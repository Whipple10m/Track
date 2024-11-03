#include "track_form.c"

#define extern
#include "track.h"
#undef extern

#include "controllers.c"
#include "helptips.c"
#include "10meter.c"



int main(int argc, char *argv[])
{

  /*   extern Info_struct az_info,el_info;*/

   fl_initialize(&argc, argv, 0, 0, 0);
   tooltips_initialize();


   fd_track_form = create_form_track_form();
   fd_browser_form = create_form_browser_form();
   fd_correct_form = create_form_correct_form();
   fd_moon_form = create_form_moon_form();
   fd_burst_form = create_form_burst_form();
   fd_scan_form = create_form_scan_form();
   fd_message_form = create_form_message_form();
   fd_notice_form = create_form_notice_form();
   /* fill-in form initialization code */
   initialize_variables();       

   menu_setup();
   help_tip_setup();
   /* set message font and size */
   fl_set_goodies_font(FL_EMBOSSED_STYLE,FL_MEDIUM_SIZE);

   /* Set up default object values ...POLARIS*/
   strcpy( id, "POLARIS");
   strcpy( id_code, "STAR");
   ftohms( ra, rastr );
   ftohms( dec, decstr );
   strcpy(mode,"STOPPED"); 
   mode_int = STANDBY;

   /* find sidereal time and set time constants */
   setconb(gettime());
   strcpy(ststr,"");
   st = 0.0;
   gettime();
   tzset();

   /* show the first form */

   fl_show_form(fd_track_form->track_form,FL_PLACE_CENTERFREE,FL_FULLBORDER,
                VERSION);
   fl_set_form_position(fd_track_form->track_form,90,20);

   /* needed for burst detection */
   system("rm /home/observer/tracking_sources/burst/notice_reply");

   sec=-1;    /* be sure update gets information */

   init_info_structure(); /* initialize motor control information */
   SERIAL_PORT=tty_set("/dev/ttyS1");  /* open RS232 port to controllers */
#ifdef DEBUGMOTORS
      printf("START of init commands\n");
#endif

   setup_motors(&az_info,1,0); /* initialize controllers */
   setup_motors(&el_info,1,0);

   print_status(); /* prints to terminal the motor controller status*/



#ifdef DEBUGMOTORS
      printf("END of init commands\n");
#endif

   initialize_cio(); /* initialize the i/o card */
   update();  /*get telescope position first*/
   fl_check_only_forms();
   azimuth_direction();

   /* Note to observer to check system time */
   fl_show_alert("","DID YOU CHECK THE UT TIME","AND DATE WITH THE GPS?",1);

/*for VVV correction routine testing*/

  if( (fpkh = fopen( "/home/observer/kh/star_track", "a" )) != NULL){

  fprintf(fpkh,"   RA      DEC  SIDEREAL  OBJAZ   OBJEL   d_phi  d_theta    X       Y     AZ_off   EL_off\n");
	  }
  fclose(fpkh);
/*END for VVV correction routine testing*/

   /* Main event loop to handle button events */
   while(FOREVER) {
     update();
     fl_check_only_forms();
   }
  printf ("Finished!\n");

  close(CIO_1);
  close(CIO_2);

   return 0;
}

/**************************SUBROUTINES****************************************/
/*-------------------------------------------------------------------------*/
/* UPDATE  Updates the screen with new tracking information and time*/
void update(void)
{
    int i;

    wait_msecs(INTERVAL);
    gettime();
    datain();    /* for 10meter ENCODER reads */

    /* SJF 2004-09-13 */
    if((telel<-2.0)&&
      ((track)||(flags.drift_flag)||(flags.track_moon)||(flags.scan_mode_run)))
      {
        fl_set_object_label(fd_track_form->mode,"STANDBY");
	track = no; /* TELLS THE MAIN LOOP TO STOP TRACKING */
	flags.drift_flag = 0; /* TELLS DRIFT LOOP TO STOP */
	flags.track_moon = 0;
	flags.scan_mode_run = 0;
	stoptel();

	fl_show_alert("",
		      "Encoder returned negative elevation.",
		      "Stopping telescope for safety. Sorry!",0);
      }

    /* if tracking the moon...update RA and DEC every minute or so */
    if(flags.track_moon){
      if(ut >= moon_time)
        moon_time=update_moon(moon_position);/*update every five minutes*/
      else if(flags.moon_switch){
        moon_time=update_moon(moon_position);/*update every five minutes*/
	flags.moon_switch=0;
      }
      if(flags.scan_mode_run)
        continue_scan();
    }
    else if((ut_sec % 5) == 0)    /* Check for a burst every 5 seconds */
      burst_input();
    if(flags.scan_mode_run)
      continue_scan();
    if(!flags.drift_flag)
      i=azandel(ra,dec,&objel,&objaz,st);
    if( track && !flags.drift_flag) {
	telstatus();
	dataout();
    }
    else if(flags.drift_flag)
	dataout();
   
    if(send_error_message){
      /* system("DISPLAY=taurus:0 /home/observer/bin/track_error_notice_main &"); */
       system("/home/observer/bin/track_error_notice_main &");
       send_error_message=0;
    }
    /* Only update screen once a second */
    if(ut_sec == sec)
      return;
    sec = ut_sec;
    place_data();

    /* SJF 2004-09-15 -- send_to_VHEGRO now in update() */
    send_to_VHEGRO();

    return;
}
/*-------------------------------------------------------------------------*/
/* AZANDEL Calculates the object AZ and EL from its R.A. & DEC */
/* Modified to take el and az as pointers to the real locations */
/* Returns 0 if object is below horizon...1 otherwise */
int  azandel(double ra,double dec,double *el,double *az,double siderial_time)
{
  double hrang, rdec, rst, rra;

  /* CONVERT FROM DEGREES TO RADIANS */
  rdec = dec / RTOD;
  rst = (siderial_time * 15.) / RTOD;
  rra  = (ra * 15.)  / RTOD;

  hrang = (rst - rra); /* CALCULATE HOUR ANGLE */

  /* CALCULATE AZIMUTH AND ELEVATION */
  /* METHOD TAKEN FROM DUFFET-SMITH "ASTRONOMY WITH YOUR PERSONEL COMPUTER" */
  *el = asin( sin(rdec)*sin(LAT) + cos(rdec)*cos(LAT)*cos(hrang) );
  *az = atan2(  -cos(rdec)*cos(LAT)*sin(hrang)
	       ,  (sin(rdec)-sin(LAT)*sin(*el)) );

  /* CONVERT BACK TO DEGREES */
  *az *= RTOD;
  *el *= RTOD;
  *az = RANGE( *az, 360. );

  if(*el < 0)   /* If elevation is below horizon return zero */
    return 0;
  return 1;
}
/*------------------------------------------------------------------------*/
/* GETTIME  Gets the UT time from the computer clock */
int gettime(void)
{
  int utyy=1999, utmo=1, utdd=1, doy=1;
  int uthh=0, utmm=0, utss=0;

  time_t ltime;
  struct tm *uttime,*local;

  time( &ltime );
  uttime = gmtime( &ltime ); /* this should allow computer to be local time */
  utss = uttime->tm_sec;
  utmm = uttime->tm_min;
  uthh = uttime->tm_hour;
  utdd = uttime->tm_mday;
  utmo = uttime->tm_mon + 1; /* ADD 1 TO MONTH BECAUSE COMPUTER GIVES MONTH */
                             /* NUMBER AS [0 TO 11] NOT [1 TO 12] */


  utyy = 1900+uttime->tm_year;   /* returns number of years since 1900 */
  setconb(utyy);                 /*get siderial conversion number */

  doy  = uttime->tm_yday+1; /* ADD ONE TO DAY OF YEAR BECAUSE COMPUTER */
                              /* GIVES DAYS SINCE FIRST OF YEAR */


  local = localtime( &ltime ); /*local time for moon position calculations */
  local_year=local->tm_year;
  local_month=local->tm_mon+1;
  local_day=utdd;            /* use ut day for moon tracking */
  local_hour=local->tm_hour;
  sprintf( datstr, "%04i-%02i-%02i", utyy, utmo, utdd );
  ut_day=utdd;
  ut_month=utmo;
  ut_year=utyy;
  ut_sec=utss;
  ut_min=utmm;
  ut_hour=uthh;


  ut = uthh + (utmm + utss/60.) / 60.;

  /* If tracking the moon and it is OFF-SOURCE then adjust the ut time */
  if(off_moon && flags.track_moon){
    ut=ut-(offset/60.);     /* if in off mode adjust ra offset */
    if(ut < 0){
      ut += 24.;
      ut_day -= 1;
      doy -= 1;
    }
    if(ut > 24){
      ut -= 24.;
      ut_day += 1;
      doy += 1;
    }
  }

  ftohms( ut, utstr );

  /* Find Sidereal Time from UT */
  /* NOTE THAT THIS ROUTINE TAKEN FROM THE OLD APPLE TRACKING SYSTEM */
  st = ut + gmsto - LONGI + TCONA*((ut / 24.) + doy) ;
  st = RANGE( st, 24. );
  ftohms( st, ststr );

  return (utyy);
}

/*------------------------------------------------------------------------*/
/* Converts decimal hours to hours, minutes, seconds string  */
/* This also converts decimal degrees to degrees, mins, secs */
int ftohms( double x, char *string)
{
  int h, m, s, sign;
  strcpy( string, "BAD DATA" );
  sign = (x>0.) ? 1: -1;
  x *= sign;
  h = (int) x;
  x = (x - h) * 60.;
  m = (int) x;
  if (m > 59) return 0;
  x = (x - m) * 60.;
  s = (int) floor (x + 0.5);
  if (s == 60)
  {
    s = 0;
    if (++m == 60)
    {
      m = 0;
      h = (h > 0) ? h++: h--;
    }
  }
  if (s > 59) return 0;
  sprintf( string, (sign==1) ? " " : "-" );
  sprintf( &string[1], "%02i:%02i:%02i", h, m, s );
  return (sign);
}
/*------------------------------------------------------------------------*/
/* function to send telescope AZ and EL to VHEGRO                             */
void    send_to_VHEGRO(void)
{
   if ( send_time <= 8)
    {
     send_time+=1;
     return;
    }

    send_time=0;
    write_cycles+=1;
    if( (fpc = fopen( "/home/observer/status.temp", "wt" )) != NULL)
    {
     /* Telescope ID (1=10meter)*/
     fprintf(fpc,"ID                     1\n");
     fprintf(fpc,"CLEAR\n");
     fprintf(fpc,"TIME/UTC               %d %d %d %f\n",ut_year,ut_month,
	     ut_day,ut); /* UT time */
     fprintf(fpc,"TIME/SID               %d %d %d %f\n",ut_year,ut_month,
	     ut_day,st); /* Sidereal time*/
          /*What is the telescope doing?*/
     fprintf(fpc,"MODE			%d\n",mode_int);
          /* Is all OK??*/
     fprintf(fpc,"STATUS		        %d\n",status_code);
          /*source RA in radians*/
     fprintf(fpc,"SOURCE/RA		%f\n",((ra*15)/RTOD));
          /*source DEC in radians*/
     fprintf(fpc,"SOURCE/DEC		%f\n",dec/RTOD);
          /*source azimuth in radians*/
     fprintf(fpc,"SOURCE/AZ		%f\n",objaz/RTOD);
          /*source elevation in radians*/
     fprintf(fpc,"SOURCE/EL		%f\n",objel/RTOD);
          /*telescope azimuth in radians*/
     fprintf(fpc,"TELESCOPE/AZ           %f\n",telaz/RTOD);
          /*telescope elevation in radians*/
     fprintf(fpc,"TELESCOPE/EL           %f\n",telel/RTOD);
          /*convert from degrees to radians*/
     fprintf(fpc,"TELESCOPE/DEV          %f\n",tdist/RTOD);
          /*ON/OFF RA offset (convert from minutes to radians)*/
     fprintf(fpc,"OFFSET/RA              %f\n",((offset/4.0)/RTOD));
          /*ON/OFF DEC offset in radians*/
     fprintf(fpc,"OFFSET/DEC		0.0\n");
          /*Source ID*/
     fprintf(fpc,"SOURCE/NAME            %s\n",id);
          /*Indicator of new info writen to file*/
     fprintf(fpc,"CYCLE			%d\n",write_cycles);
     fclose(fpc);
    }
    system("cp /home/observer/status.temp /home/observer/status.out");
}

/*-------------------------------------------------------------------------*/
void clear_message(void)  /* Clear and remove the info window */
{
    strcpy(current_message[0],"");
    strcpy(current_message[1],"");
    strcpy(current_message[2],"");
    fl_set_object_label(fd_notice_form->message_1,current_message[0]);
    fl_set_object_label(fd_notice_form->message_2,current_message[1]);
    fl_set_object_label(fd_notice_form->message_3,current_message[2]);
    fl_hide_form(fd_notice_form->notice_form);
    return;
}
/*-------------------------------------------------------------------------*/
void place_message(void) /* Send a message to lower part of track form */
{

   fl_show_form(fd_notice_form->notice_form,FL_PLACE_CENTERFREE,FL_FULLBORDER,
                "Messages");

    fl_set_object_label(fd_notice_form->message_1,current_message[0]);
    fl_set_object_label(fd_notice_form->message_2,current_message[1]);
    fl_set_object_label(fd_notice_form->message_3,current_message[2]);
    return;
}
/*-------------------------------------------------------------------------*/
/* PRINT LIMIT CONDITION & STOP TELESCOPE */
void limit_print(char buffer[80],int stop_telescope)
{
  char dir_buf[40];
  print_status();
  if(stop_telescope){
    track = no;
    printf("limit_print: -%s- stoptel() called\n",buffer);
    stoptel();
   /* system("DISPLAY=taurus:0 /home/observer/bin/track_error_notice_main &"); */
    system("/home/observer/bin/track_error_notice_main &");

  }
  fl_set_object_label(fd_message_form->l1,check_status_message[0]);
  fl_set_object_label(fd_message_form->l2,check_status_message[1]);
  fl_set_object_label(fd_message_form->l3,check_status_message[2]);
  fl_set_object_label(fd_message_form->l4,check_status_message[3]);
  fl_set_object_label(fd_message_form->l5,check_status_message[4]);
  fl_set_object_label(fd_message_form->l6,check_status_message[5]);
  fl_set_object_label(fd_message_form->l7,check_status_message[6]);
  fl_set_object_label(fd_message_form->l8,check_status_message[7]);
  fl_set_object_label(fd_message_form->l9,check_status_message[8]);
  fl_set_object_label(fd_message_form->l10,check_status_message[9]);
  fl_set_object_label(fd_message_form->l11,check_status_message[10]);
  fl_set_object_label(fd_message_form->l12,check_status_message[11]);
  fl_set_object_label(fd_message_form->l13,check_status_message[12]);
  fl_set_object_label(fd_message_form->l14,check_status_message[13]);
  fl_set_object_label(fd_message_form->l15,check_status_message[14]);
  if(az_direction==CW)
    strcpy(dir_buf,"Starting Azimuth direction was CW");
  else if(az_direction==CCW)
    strcpy(dir_buf,"Starting Azimuth direction was CCW");
  else if(az_direction==-1)
    strcpy(dir_buf,"Starting Azimuth direction in unknown");
  fl_set_object_label(fd_message_form->l16,dir_buf);

  fl_show_form(fd_message_form->message_form,FL_PLACE_CENTERFREE,FL_FULLBORDER,
                "STATUS MESSAGES");

  fl_show_alert(buffer,"See STATUS messages:","Press Button to Continue",1);
  fl_hide_form(fd_message_form->message_form);

  return;
}
/*-------------------------------------------------------------------------*/
/* SETCONB  Sets the siderial time constant acording to the year */
/* These constants are found on pageB6 n the astronomical almanac */
void setconb( int utyy ) /* Done ! */
{
    switch(utyy){
    case 1996:
	gmsto = 6.5967564;
	break;
    case 1997:
	gmsto = 6.6465521;
	break;
    case 1998:
	gmsto = 6.6306380;
	break;
    case 1999:
	gmsto = 6.6147239;
	break;
    case 99:
	gmsto = 6.6147239;
	break;
    case 2000:
        gmsto = 6.5988098;
	break;
    case 00:
        gmsto = 6.5988098;
	break;
    case 01:
	gmsto = 6.6486056;
	break;
    case 2001:
	gmsto = 6.6486056;
	break;
    case 2002:
	gmsto = 6.6326915;
        break;
    case 2003:
	gmsto = 6.6167774;
        break;
    case 2004:
        gmsto = 6.6008633;
        break;
    default:
	fprintf(stderr,"%s: unable to get siderial time constant for %d\n",
		progname,utyy);
	break;
    }
  
  return;
}
/*-------------------------------------------------------------------------*/
/* Wait for specified number of milliseconds */
int wait_msecs( int msecs )
{
  struct timeb tnow;
  unsigned long wait_time;

  /* code changed to release CPU time to the system*/
  wait_time=(unsigned long)(msecs * 1000);  /* time in usecs to system */
  usleep(wait_time);
  ftime( &tnow );
  return (tnow.millitm);
}
/*-------------------------------------------------------------------------*/
int ftod(double x, double *y)
/* Converts hhmmss.ss to decimal hours             */
/* This also converts ddmmss.ss to decimal degrees */
{
  int h, m, sign;

  sign = (x > 0.) ? 1: -1;
  x *= sign;
  h = (int) (x/10000.);
  x -= 10000. * (double) h;
  m = (int) (x/100.);
  if (m > 59) return 0;
  x -= 100. * (double) m;
  if (x > 60.) return 0;
  *y = (double)sign * ((double)h + ((double)m + x/60.)/60.);

  return (sign);
}		       
/*------------------------------------------------------------------------*/
void place_data(void) /* Done ! */
{
    char string[80];

    /* Data in display window */
     fl_set_object_label(fd_track_form->sid_time,ststr);
     fl_set_object_label(fd_track_form->ut_time,utstr);
     fl_set_object_label(fd_track_form->raobj,rastr);
     fl_set_object_label(fd_track_form->decobj,decstr);
     sprintf(string,"%7.2f  ",((long)(RANGE(objaz,360.)*100.)/100.));
     fl_set_object_label(fd_track_form->azobj,string);
     sprintf(string,"%7.2f  ",((long)(objel*100.)/100.));
     fl_set_object_label(fd_track_form->elobj,string);
     sprintf(string,"%8.3f  ",((long)(RANGE(telaz,360.)*1000.)/1000.));
     fl_set_object_label(fd_track_form->aztel,string);
     sprintf(string,"%8.3f  ",((long)(telel*1000.)/1000.));
     fl_set_object_label(fd_track_form->eltel,string);
     /*Change colors of difference display if > .1 degrees*/
     if (track || flags.drift_mode){
       if (tdist >.05) {
         fl_set_object_lcol(fd_track_form->diff,FL_WHITE);
         fl_set_object_color(fd_track_form->diff,FL_RED,FL_RED);
       }
       else {
         fl_set_object_lcol(fd_track_form->diff,FL_RED);
         fl_set_object_color(fd_track_form->diff,FL_TOP_BCOL,FL_MCOL);
       }
       sprintf(string,"%8.3f  ",((long)(tdist*1000.)/1000.));
     }
     else {
       sprintf(string,"**.***");
       fl_set_object_lcol(fd_track_form->diff,FL_WHITE);
       fl_set_object_color(fd_track_form->diff,FL_DARKGOLD,FL_DARKGOLD);

     }
     fl_set_object_label(fd_track_form->diff,string);
     sprintf(string,"%7.2f  ",((long)(offset*100.)/100.));
     fl_set_object_label(fd_track_form->offset,string);
     if(offset >= 0)
       fl_set_object_label(fd_track_form->first_source,"ON-OFF");
     else
       fl_set_object_label(fd_track_form->first_source,"OFF-ON");
     fl_set_object_label(fd_track_form->code,id_code);
     fl_set_object_label(fd_track_form->objid,id);
     fl_set_object_label(fd_track_form->ut_date,datstr);
     fl_set_object_label(fd_track_form->slew,mode);

    
    /*finally make sure that the program did not stop tracking and not reset
      the mode button*/
    if(!track && fl_get_button(fd_track_form->mode)){
      fl_set_button(fd_track_form->mode,0); /* release button */
      fl_set_object_label(fd_track_form->mode,"STANDBY");
    }
    return;
}
/*-------------------------------------------------------------------------*/
/*--     FROM2000
*---------------------------------------------------------------------
*-- GV @ FLWO   ! Oct 14, 1988 !
*---------------------------------------------------------------------
*-- Precesses equatorial coordinates from J2000.0 to a given EPOCH.
*-- RA is the Right Ascension in hours;
*-- DEC is the declination in degrees;
*-- EPOCH is the time to which the coordinates are to be precessed.
*-- RA and DEC are replaced by the precessed coordinates.
*-- All calculations are carried on in double precision.
*-- References:
*--     Astronomical Almanac 1989, page B19.
*--     R.M. Green "Spherical Astronomy" Cambridge UP
*---------------------------------------------------------------------*/
void from2000(double epoch, double *dumra, double *dumdec)
{
  double alpha,delta,alpha0,delta0;
  double t_,m_,n_, alpham, deltam;

/*-- Get coordinates in radians --*/
  alpha0 = (*dumra * 15.)/RTOD;
  delta0 = *dumdec/RTOD;

/*-- Compute precessional parameters for the given time --*/
  t_ = ((double) epoch - 2000.0 ) / 100.0;
  m_ = raval(t_);
  n_ = decval(t_);

/*-- Precess coordinates to the mid-time point --*/
  alpham = alpha0 + 0.5*(m_+n_*sin(alpha0)*tan(delta0));
  deltam = delta0 + 0.5*n_*cos(alpha0);

/*-- Precess coordinates to EPOCH --*/
  alpha = alpha0 + m_ + n_*sin(alpham)*tan(deltam);
  delta = delta0 + n_*cos(alpham);

/*-- Replace coordinates --*/
  *dumra = alpha*RTOD/15.;
  *dumdec= delta*RTOD;

  return;
}
/*-------------------------------------------------------------------------*/
/*--     RAVAL & DECVAL
*---------------------------------------------------------------------
*-- GV @ FLWO   ! 14 Oct, 1988 !
*---------------------------------------------------------------------
*-- Return  precessional parameters in radians for both Right
*-- Ascension and Declinaton.
*-- t_ is the time in centuries from J2000.0.
*-- Ex:
*--     if t (years) is the time of interest t_=(t-2000.0)/100
*-- Reference: Astronamical Almanac 1989, page B19.
*---------------------------------------------------------------------*/
double raval(double t_)
{
 double a = 2.236172e-2, b = 6.770132e-6, c = 1.762783e-7;
 return t_*(a+t_*(b+t_*c));
}

double decval(double t_)
{
  double a = 9.717173e-3, b = -2.068215e-6, c = -2.024582e-7;
  return t_*(a+t_*(b+t_*c));
}

/*--     TO2000
*---------------------------------------------------------------------
*-- GV @ FLWO   ! Oct 14, 1988 !
*---------------------------------------------------------------------
*-- Precesses equatorial coordinates from a given EPOCH to J2000.0.
*-- RA is the Right Ascension in hours;
*-- DEC is the declination in degrees;
*-- EPOCH is the time from which the coordinates are to be precessed.
*-- RA and DEC are replaced by the precessed coordinates.
*-- All calculations are carried on in double precision.
*-- References:
*--     The Astronomical Almanac 1989, page B19.
*--     R.M. Green "Spherical Astronomy" Cambridge UP
*---------------------------------------------------------------------*/
void to2000(double epoch, double *dumra, double *dumdec)
{
  double alpha,delta,alpha0,delta0;
  double t_,m_,n_, alpham, deltam;

/*-- Get coordinates in radians --*/
  alpha = (*dumra * 15.)/RTOD;
  delta = *dumdec/RTOD;

/*-- Compute precessional parameters for the given time --*/
  t_ = (epoch - 2000.0 ) / 100.0;
  m_ = raval(t_);
  n_ = decval(t_);

/*-- Precess coordinates to the mid-time point --*/
  alpham = alpha - 0.5*(m_+n_*sin(alpha)*tan(delta));
  deltam = delta - 0.5*n_*cos(alpha);

/*-- Precess coordinates to J2000.0 --*/
  alpha0 = alpha - m_ - n_*sin(alpham)*tan(deltam);
  delta0 = delta - n_*cos(alpham);

/*-- Replace coordinates --*/
  *dumra = alpha0*RTOD/15.;
  *dumdec= delta0*RTOD;

  return;
}
/*-------------------------------------------------------------------------*/
/* Get object list from file SOURCES.10m and pick object */
void getobj(void)
{
    const char *filename;
    int  ct=0,loop;
    char oname[15], ocode[5], temp[80];
    double ora, odec, opra, opdec,epch_o,epch_p;
    double dra, ddec, diff;
    FILE *fpgo;

    if(flags.freeze && !flags.doing_corrections){
      flag_alert();
      return;
    }

    ct = 0;
    filename=fl_show_fselector("Choose a file...(Press enter for default)",
			       "/home/observer/tracking_sources","*.trk*",
			     "sources.trk");
    if((fpgo = fopen( filename,"rt")) != NULL){
	if( fgets(temp,80,fpgo) == NULL){  /* Read comment line in file */
	    fprintf(stderr,"%s: error reading %s\n",progname,filename);
	    return;
	}
	do {
	    strncpy(oname,temp,11); /*read in source name */
	    oname[11] = '\0';
	    if(sscanf(&temp[12],"%lf %lf %lf %lf %lf %lf %s", 
	       &ora, &odec, &epch_o, &opra, &opdec, &epch_p, &ocode[0]) == 7){
	       ocode[4]='\0';
               ftod(ora, &dra);
	       diff = dra - st;
	       diff -= 24. * floor( diff/24. + 0.5 );
               /* create more memory only if needed */
               if(source_list[ct]==NULL)
                  source_list[ct]=
	             (Source *)malloc(sizeof(Source));
               ftod(odec, &ddec);
   	       if(diff < 6. && diff >= 0.0)
		  source_list[ct]->visible=1;
   	       else if(diff > -6. && diff < 0.0)
		  source_list[ct]->visible=2;
	       else
		  source_list[ct]->visible=0;
	       strcpy(source_list[ct]->name, oname);
	       strcpy(source_list[ct]->source_code, ocode);
               source_list[ct]->rao = dra;
      	       source_list[ct]->deco = ddec;
               source_list[ct]->epocho = epch_o;
               source_list[ct]->epochp = epch_p;

	       ftod(opra, &dra);
	       ftod(opdec, &ddec);
	       source_list[ct]->rap = dra;
	       source_list[ct]->decp = ddec;
	       ct++;
	 
	    }

        } while(fgets(temp,80,fpgo) != NULL && ct < 150);

	fclose(fpgo);       /* Now CT = #elements */
    }
    else{
	fprintf(stderr,"%s: can not find /home/observer/sources.10m\n",
		progname);
	return;
    }
    
    /* Now sort the objects by R.A. */
    st_diff=RANGE( (st-6.), 24. ); /* RA of objects setting */
    qsort(&source_list[0], ct, sizeof(source_list[0]), compare);
    /* Put POLARIS as last object on the list */
    
    if(source_list[ct]==NULL)
       source_list[ct]=(Source *)malloc(sizeof(Source));
    strcpy(source_list[ct]->name, "POLARIS");
    strcpy(source_list[ct]->source_code, "STAR");
    source_list[ct]->rao =2.52091667;     /*old value 2.474722;*/
    source_list[ct]->deco =89.2619444;   /*old value 89.250944;*/
    source_list[ct]->epocho = 2000;
    source_list[ct]->epochp = 2000;
    source_list[ct]->rap = 2.52091667;  /*2.474722;*/
    source_list[ct]->decp = 89.2619444;  /*89.250944;*/
    source_list[ct]->visible=1;


    for(loop=0;loop<=ct;loop++)
       if(source_list[loop]->visible)
           azandel(source_list[loop]->rao,source_list[loop]->deco,
		   &source_list[loop]->el,&source_list[loop]->az,st);
    sources=ct;
    return;
}
/*-------------------------------------------------------------------------*/
void getburst(void)
{
    const char *filename;
    int  loop=0,track_choice=0;
    char temp[6][80];
    float ra_c,dec_c,ra_t,dec_t,ra_a,dec_a,ra_b,dec_b,ra_be,dec_be,epoch_b;
    FILE *fpgo;


    if(flags.freeze){
      flag_alert();
      return;
    }

    filename=fl_show_fselector("Choose a burst file",
		      "/home/observer/tracking_sources/burst","*.burst","");
    if(filename == NULL)
      return;
    if((fpgo = fopen( filename,"rt")) != NULL){
	if( fgets(temp[0],80,fpgo) == NULL){  /* Read comment line in file */
	    fprintf(stderr,"%s: error reading %s\n",progname,filename);
	    return;
	}
       for(loop=0;loop<5;loop++)
	  fgets(temp[loop],80,fpgo);
       sscanf(temp[0],"%f %f %f",&ra_c,&dec_c,&epoch_b);
       sscanf(temp[1],"%f %f",&ra_t,&dec_t); 
       sscanf(temp[2],"%f %f",&ra_a,&dec_a);
       sscanf(temp[3],"%f %f",&ra_b,&dec_b); 
       sscanf(temp[4],"%f %f",&ra_be,&dec_be); 
       /* create memory */
       burst_number=0; /* treat this as the first burst */
       if(burst_list[burst_number]==NULL)
         burst_list[burst_number]=
	          (struct source_burst *)malloc(sizeof(struct source_burst));
       burst_list[burst_number]->epoch = (double)epoch_b;
       ftod((double)ra_c, &burst_list[burst_number]->ra_center);
       ftod((double)dec_c, &burst_list[burst_number]->dec_center);
       to2000(burst_list[burst_number]->epoch , 
       	    &burst_list[burst_number]->ra_center,
            &burst_list[burst_number]->dec_center );
       from2000((double) (gettime()), 
      	    &burst_list[burst_number]->ra_center,
	    &burst_list[burst_number]->dec_center );
       ftod((double)ra_a, &burst_list[burst_number]->ra_ahead);
       ftod((double)dec_a, &burst_list[burst_number]->dec_ahead);
       to2000(burst_list[burst_number]->epoch , 
            &burst_list[burst_number]->ra_ahead,
       	    &burst_list[burst_number]->dec_ahead );
       from2000((double) (gettime()),
       	    &burst_list[burst_number]->ra_ahead,
       	    &burst_list[burst_number]->dec_ahead );
       ftod((double)ra_t, &burst_list[burst_number]->ra_top);
       ftod((double)dec_t, &burst_list[burst_number]->dec_top);
       to2000(burst_list[burst_number]->epoch ,
       	    &burst_list[burst_number]->ra_top,
       	    &burst_list[burst_number]->dec_top );
       from2000((double) (gettime()),
       	    &burst_list[burst_number]->ra_top,
       	    &burst_list[burst_number]->dec_top );
       ftod((double)ra_be, &burst_list[burst_number]->ra_behind);
       ftod((double)dec_be, &burst_list[burst_number]->dec_behind);
       to2000(burst_list[burst_number]->epoch ,
       	    &burst_list[burst_number]->ra_behind,
       	    &burst_list[burst_number]->dec_behind );
       from2000((double) (gettime()),
       	    &burst_list[burst_number]->ra_behind,
       	    &burst_list[burst_number]->dec_behind );
       ftod((double)ra_b, &burst_list[burst_number]->ra_bottom);
       ftod((double)dec_b, &burst_list[burst_number]->dec_bottom);
       to2000(burst_list[burst_number]->epoch ,
       	    &burst_list[burst_number]->ra_bottom,
       	    &burst_list[burst_number]->dec_bottom );
       from2000((double) (gettime()),
       	    &burst_list[burst_number]->ra_bottom,
       	    &burst_list[burst_number]->dec_bottom );
      }

       /* save current settings for later replacement */
       temp_ra=ra;
       temp_dec=dec;
       temp_sra=sra;
       temp_sdec=sdec;
       temp_pra=pra;
       temp_pdec=pdec;
       strcpy(temp_id,id);
       strcpy(temp_code,id_code);

       /* Default position is center...set it up */
       strcpy(id_code,"BURST");
       ra=burst_list[0]->ra_center;
       dec=burst_list[0]->dec_center;
       sra  = ra;
       sdec = dec;
       pra = ra;
       pdec = dec;
       ftohms(ra, rastr);
       ftohms(dec, decstr);
       flags.freeze=1;


#if 0
    track_choice=fl_show_choice("Choose between the scanning method and",
		     "the five position method (FIVE)",
		     "",2,"SCAN","FIVE","",1);
#else
    track_choice=0;
#endif

    if(track_choice==1){
       scan_setup();
       fl_show_form(fd_scan_form->scan_form,FL_PLACE_CENTERFREE,
		FL_FULLBORDER,"Scan Tracking");
       flags.scan_mode=1;
    }
    else{
       fl_show_form(fd_burst_form->burst_form,FL_PLACE_CENTERFREE,
		FL_FULLBORDER,"Burst Tracking");
       flags.burst_mode=1;

       burst_position=CENTER;

       strcpy(id,"BURST CENTER");
       fl_show_object(fd_burst_form->burst_center);
       fl_set_object_color(fd_burst_form->burst_center,FL_WHITE,FL_WHITE);
       fl_set_object_color(fd_burst_form->burst_1,FL_COL1,FL_COL1);
       fl_set_object_color(fd_burst_form->burst_2,FL_COL1,FL_COL1);
       fl_set_object_color(fd_burst_form->burst_3,FL_COL1,FL_COL1);
       fl_set_object_color(fd_burst_form->burst_4,FL_COL1,FL_COL1);
    }
    return;
}
/*-------------------------------------------------------------------------*/
/* Compare for quicksort of objects */
/* A change was done here to sort according to objects setting first */
/* ie. a siderial time (-6 hours) subtraction is done */

int compare( Source **x, Source **y)
{
  if( RANGE((*x)->rao - st_diff,24.) > RANGE((*y)->rao - st_diff,24.) )
    return 1;
  else if ( RANGE((*x)->rao - st_diff,24.) < RANGE((*y)->rao - st_diff,24.) )
    return -1;
  return 0;
}

/****************************************************************/
/*Subroutines FOR TPOINT CORRECTION ROUTINE START HERE */
/****************************************************************/
void tpoint_correction(double *d_el, double *d_az, double el, double az)
{ 
  double az_rad, el_rad , correction_az=0.0,correction_el=0.0;

      az_rad=pi-(az/RTOD);
      if(az_rad<0)
	 az_rad+=2*pi;
      el_rad=el/RTOD;
      /*Initially all corrections are in arc seconds but converted to degrees
	at the end and subtracked from the telescope position. */
      /*IE*/
      correction_el=IE;
      /*IA*/
      correction_az=IA;
      /*AN*/
       correction_az+=AN*sin(az_rad)*tan(el_rad);
       correction_el+=AN*cos(az_rad);
      /*AW*/ 
        correction_az-=AW*cos(az_rad)*tan(el_rad);
        correction_el+=AW*sin(az_rad);
      /*CA*/
      correction_az+=CA*(1/cos(el_rad));
      /*NPAE*/
      correction_az+=NPAE*tan(el_rad);
      /*TX*/
      correction_el-=TX*(1/tan(el_rad));
      /*ECEC*/
      correction_el+=ECEC*cos(el_rad);
      /*ECES*/
      correction_el+=ECES*sin(el_rad);
      /*ACEC*/
      correction_az+=ACEC*cos(az_rad);
      /*ACES*/
      correction_az+=ACES*sin(az_rad);

      /*convert from arc seconds to degrees */
      correction_az=correction_az/3600;
      correction_el=correction_el/3600;

      /* return corrections such that they are in degrees and ready to
	 be added to the telescope position */
      *d_az=-1*correction_az;
      *d_el=correction_el;
/*printf("correction_az= %7.3f  correction_el= %7.3f \n",correction_az,
       correction_el);*/

}
/****************************************************************/
/*Subroutines FOR  TPOINT CORRECTION ROUTINE END HERE */
/****************************************************************/

/****************************************************************/
/*Subroutines FOR VVV CORRECTION ROUTINE START HERE */
/****************************************************************/
void correction(double *d_theta, double *d_phi, 
                double theta, double phi, double X, double Z, int i_pmt)
/* 
 * This routine calculates pointing corrections to telescope
 * azimuth and elevation due to the most apparent distortions 
 * in the telescope pointing, such as tilt of the rotation axes 
 * or deflection of the telescope from the vertical.
 *
 *  d_theta   -elevation correction           [deg]
 *  d_phi     -azimuth correction             [deg]
 *  theta     -elevation                      [deg]
 *  phi       -azimuth                        [deg]
 *  X         -focal plane x coordinate       [deg]
 *  Z         -focal plane z coordinate       [deg]
 *
 * Non-zero (X,Z) allow an object to be traced at an arbitrary (X,Z) 
 * point of the focal plane r.f.. For tracing an object in the central 
 * pmt set (X,Z)=(0,0).
 *
 * vvv 11/05/99
 */
{
  /*vector e_x={1.,0.,0.};*/
  vector e_y={0.,1.,0.};
  /*  vector e_z={0.,0.,1.};*/

  vector w,v,q;
  double sxi,cxi,sq;
  double theta_,phi_;
  pmt  aux;


  if (i_pmt > 0){
    /* get pmt position */
    aux=pmt_data(i_pmt);
    X=aux.x;
    Z=aux.z;
  }

  /* phi must be in the range [-270, +270] deg    */
  if(phi > 270.) phi-=360.;
  if(phi <-270.) phi+=360.;

  /* transform focal plane r.f. to telescope r.f. */
  focalplane_to_telescope(X, Z, &v);

  /* pointing vector in the telescope r.f.        */
  q=Rx(-theta_o,
       Rz(phi-phi_o,
          Rx(theta,e_y)
       )
    );

  /* find elevation                               */
  cxi=cos(conversion*xi_o);
  sxi=sin(conversion*xi_o);
  sq=sqrt(v.y*v.y+v.z*v.z);
  theta_=asin((q.z-v.x*sxi)/cxi/sq)-asin(v.z/sq);
  if(theta_>pi/2.) theta_=pi-theta_;
  theta_/=conversion;

  /* unrotate elevation                           */
  w=Ry(xi_o, 
	  Rx(theta_,v)
	);

  /* find azimuth                                 */
  phi_=atan2(-q.y,q.x)+atan2(w.y,w.x);
  if(fabs(phi_-phi)>fabs(phi_-phi-2*pi)) phi_-=2.*pi;
  if(fabs(phi_-phi)>fabs(phi_-phi+2*pi)) phi_+=2.*pi;
  if(phi_>= 3.*pi/2.) phi_=phi_-2.*pi;
  if(phi_<=-3.*pi/2.) phi_=phi_+2.*pi;
  phi_/=conversion;

  /* unrotate azimuth                             */
  v=Rz(phi_,w);

  /* consistency check */
  /*
  printf("small numbers: %le %le %le\n",q.x-v.x, q.y-v.y, q.z-v.z);
  printf("vector       : %le %le %le\n",    v.x,     v.y,     v.z);
  */

  /* find azimuth and elevation corrections       */
  *d_theta=theta-theta_;
  *d_phi  =phi-phi_;

}
/*-------------------------------------------------------------------------*/
/* x-rotation, e_x=(1,0,0) is eigen vector */ 
vector Rx(double theta, vector v)
{
   static double c=1.,s=0.,theta_=0.;
   vector w;

   if(theta_ != theta) {
      c=cos(conversion*theta);
      s=sin(conversion*theta);
      theta_=theta;
   }
   w.x= v.x;
   w.y= v.y*c - v.z*s;
   w.z= v.y*s + v.z*c;
   return (w);
}
/* z-rotation, e_z=(0,0,1) is eigen vector */ 
vector Rz(double phi, vector v)
{
   static double c=1.,s=0.,phi_=0.;
   vector w;

   if(phi_ != phi) {
      c=cos(conversion*phi);
      s=sin(conversion*phi);
      phi_=phi;
   }
   w.x= v.x*c + v.y*s;
   w.y=-v.x*s + v.y*c;
   w.z= v.z;
   return (w);
}
/* y-rotation, e_y=(0,1,0) is eigen vector */ 
vector Ry(double xi, vector v)
{
   static double c=1.,s=0.,xi_=0.;
   vector w;

   if(xi_ != xi) {
      c=cos(conversion*xi);
      s=sin(conversion*xi);
      xi_=xi;
   }
   w.x= v.x*c - v.z*s;
   w.y= v.y;
   w.z= v.x*s + v.z*c;
   return (w);
}
/*-------------------------------------------------------------------------*/
void focalplane_to_spherical(
          double theta_t, double phi_t, double x_t, double z_t,
          double *theta  , double *phi  , double x  , double z)
/* 
 * The routine focalplane_to_spherical calculates spherical coordinates 
 * (theta,phi) correspondent to a focal plane position (x,z) if telescope 
 * tracks source with spherical coordinates (theta_t,phi_t) in the point 
 * (x_t,z_t) on the focal plane. This is the inverse function to
 *  spherical_to_focalplane routine.
 *
 *  theta_t   -tracking elevation                [deg]
 *  phi_t     -tracking azimuth                  [deg]
 *  x_t       -tracking focal plane x coordinate [deg]
 *  z_t       -tracking focal plane z coordinate [deg]
 *  x         -focal plane x=x(theta,phi)        [deg]
 *  z         -focal plane z=z(theta,phi)        [deg]
 *  theta     -elevation                         [deg]
 *  phi       -azimuth                           [deg]
 *
 * vvv 11/07/99
 */
{
  vector v,e;
  double d_theta, d_phi;

  /* find azimuth and elevation corrections       */
  correction(&d_theta, &d_phi, theta_t, phi_t, x_t, z_t, 0);

  /* determine directional vector                 */
  focalplane_to_telescope(x, z, &v);

  /* find directional vector in spherical r.f.    */              
  e=Rz(phi_o,
       Rx(theta_o,
          Rz(phi_t-d_phi,   
             Ry(xi_o,
                Rx(theta_t-d_theta,v)
             )
          )
       )
    );

  /* determine azimuth and elevation              */
  *theta=asin(e.z);
  (*theta)/=conversion;
  *phi=atan2(e.x,e.y);
  (*phi)/=conversion;
}
/*-------------------------------------------------------------------------*/
void spherical_to_focalplane(
          double theta_t, double phi_t, double x_t, double z_t,
          double theta  , double phi  , double *x  , double *z)
/* 
 * The routine spherical_to_focalplane calculates focal plane
 * position (x,z) correspondent to spherical coordinates 
 * (theta,phi) if telescope tracks source with spherical
 * coordinates (theta_t,phi_t) in the point (x_t,z_t) on the 
 * focal plane (if theta=theta_t & phi=phi_t, => x=x_t & z=z_t).   
 *
 *  theta_t   -tracking elevation                [deg]
 *  phi_t     -tracking azimuth                  [deg]
 *  x_t       -tracking focal plane x coordinate [deg]
 *  z_t       -tracking focal plane z coordinate [deg]
 *  theta     -elevation                         [deg]
 *  phi       -azimuth                           [deg]
 *  x         -focal plane x=x(theta,phi)        [deg]
 *  z         -focal plane z=z(theta,phi)        [deg]
 *
 * vvv 11/07/99
 */
{
  vector v;
  vector e_y={0.,1.,0.};
  double d_theta, d_phi;

  /* find azimuth and elevation corrections       */
  correction(&d_theta, &d_phi, theta_t, phi_t, x_t, z_t, 0);

  /* find directional vector in telescope r.f.    */ 
  v=Rx(-theta_t+d_theta,  
       Ry(-xi_o,
          Rz(-phi_t+d_phi,
             Rx(-theta_o,
                Rz(phi-phi_o,
                   Rx(theta,e_y)
                )
             )
          )
       )
    );

  /* determine coordinates in the focal plane     */
  telescope_to_focalplane(x, z, v);

}
/*-------------------------------------------------------------------------*/
void focalplane_to_telescope(double X, double Z, vector *v)
/* 
 * transform focal plane coordinates to directional vector 
 *
 *  X         -focal plane x coordinate                  [deg]
 *  Z         -focal plane z coordinate                  [deg]
 *  v         -unit directional vector in telescope r.f.   [1]
 *
 * vvv 11/05/99
 */
{
  vector w;

  w.x=-conversion*(X-dx_o);
  w.z=-conversion*(Z-dz_o);
  w.y=sqrt(1.-w.x*w.x-w.z*w.z);
  *v=Ry(psi_o,w);

}
/*-------------------------------------------------------------------------*/
void telescope_to_focalplane(double *x, double *z, vector v)
/* 
 * transform directional vector to focal plane coordinates 
 *
 *  v         -unit directional vector in telescope r.f.   [1]
 *  x         -focal plane x coordinate                  [deg]
 *  z         -focal plane z coordinate                  [deg]
 *
 * vvv 11/05/99
 */
{
  vector w;

  w=Ry(-psi_o,v);

  w.x/=conversion;
  w.z/=conversion;

  *x=-w.x+dx_o;
  *z=-w.z+dz_o;

}
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
pmt pmt_data(int i)
{
/* data for pmts of 490 camera
 *
 *     pmt.x  -x coordinate             [deg]
 *     pmt.z  -z coordinate             [deg]
 *     pmt.r  -photocathode radius      [deg]
 *     pmt.c  -hexagonal cone           [deg]
 *     pmt.g  -conversion factor (gain) [dc/pe]
 *
 * vvv 11/02/99
 */
 pmt data[490]={{0.000, -0.000,  0.043,  0.118,  1.000},
               { 0.118, -0.000,  0.043,  0.118,  1.000},
               { 0.059, -0.102,  0.043,  0.118,  1.000},
               {-0.059, -0.102,  0.043,  0.118,  1.000},
               {-0.118, -0.000,  0.043,  0.118,  1.000},
               {-0.059,  0.102,  0.043,  0.118,  1.000},
               { 0.059,  0.102,  0.043,  0.118,  1.000},
               { 0.236, -0.000,  0.043,  0.118,  1.000},
               { 0.177, -0.102,  0.043,  0.118,  1.000},
               { 0.118, -0.204,  0.043,  0.118,  1.000},
               { 0.000, -0.204,  0.043,  0.118,  1.000},
               {-0.118, -0.204,  0.043,  0.118,  1.000},
               {-0.177, -0.102,  0.043,  0.118,  1.000},
               {-0.236, -0.000,  0.043,  0.118,  1.000},
               {-0.177,  0.102,  0.043,  0.118,  1.000},
               {-0.118,  0.204,  0.043,  0.118,  1.000},
               { 0.000,  0.204,  0.043,  0.118,  1.000},
                {0.118,  0.204,  0.043,  0.118,  1.000},
               { 0.177,  0.102,  0.043,  0.118,  1.000},
               { 0.354, -0.000,  0.043,  0.118,  1.000},
               { 0.295, -0.102,  0.043,  0.118,  1.000},
               { 0.236, -0.204,  0.043,  0.118,  1.000},
               { 0.177, -0.306,  0.043,  0.118,  1.000},
               { 0.059, -0.306,  0.043,  0.118,  1.000},
              { -0.059, -0.306,  0.043,  0.118,  1.000},
              { -0.177, -0.306,  0.043,  0.118,  1.000},
              { -0.236, -0.204,  0.043,  0.118,  1.000},
              { -0.295, -0.102,  0.043,  0.118,  1.000},
              { -0.354, -0.000,  0.043,  0.118,  1.000},
              { -0.295,  0.102,  0.043,  0.118,  1.000},
              { -0.236,  0.204,  0.043,  0.118,  1.000},
              { -0.177,  0.306,  0.043,  0.118,  1.000},
              { -0.059,  0.306,  0.043,  0.118,  1.000},
              {  0.059,  0.306,  0.043,  0.118,  1.000},
              {  0.177,  0.306,  0.043,  0.118,  1.000},
              {  0.236,  0.204,  0.043,  0.118,  1.000},
              {  0.295,  0.102,  0.043,  0.118,  1.000},
              {  0.472, -0.000,  0.043,  0.118,  1.000},
              {  0.413, -0.102,  0.043,  0.118,  1.000},
              {  0.354, -0.204,  0.043,  0.118,  1.000},
              {  0.295, -0.306,  0.043,  0.118,  1.000},
              {  0.236, -0.408,  0.043,  0.118,  1.000},
              {  0.118, -0.408,  0.043,  0.118,  1.000},
              {  0.000, -0.408,  0.043,  0.118,  1.000},
              { -0.118, -0.408,  0.043,  0.118,  1.000},
              { -0.236, -0.408,  0.043,  0.118,  1.000},
              { -0.295, -0.306,  0.043,  0.118,  1.000},
              { -0.354, -0.204,  0.043,  0.118,  1.000},
              { -0.413, -0.102,  0.043,  0.118,  1.000},
              { -0.472, -0.000,  0.043,  0.118,  1.000},
              { -0.413,  0.102,  0.043,  0.118,  1.000},
              { -0.354,  0.204,  0.043,  0.118,  1.000},
              { -0.295,  0.306,  0.043,  0.118,  1.000},
              { -0.236,  0.408,  0.043,  0.118,  1.000},
              { -0.118,  0.408,  0.043,  0.118,  1.000},
              {  0.000,  0.408,  0.043,  0.118,  1.000},
              {  0.118,  0.408,  0.043,  0.118,  1.000},
              {  0.236,  0.408,  0.043,  0.118,  1.000},
              {  0.295,  0.306,  0.043,  0.118,  1.000},
              {  0.354,  0.204,  0.043,  0.118,  1.000},
              {  0.413,  0.102,  0.043,  0.118,  1.000},
              {  0.589, -0.000,  0.043,  0.118,  1.000},
              {  0.531, -0.102,  0.043,  0.118,  1.000},
              {  0.472, -0.204,  0.043,  0.118,  1.000},
              {  0.413, -0.306,  0.043,  0.118,  1.000},
              {  0.354, -0.408,  0.043,  0.118,  1.000},
              {  0.295, -0.510,  0.043,  0.118,  1.000},
              {  0.177, -0.510,  0.043,  0.118,  1.000},
              {  0.059, -0.510,  0.043,  0.118,  1.000},
              { -0.059, -0.510,  0.043,  0.118,  1.000},
              { -0.177, -0.510,  0.043,  0.118,  1.000},
              { -0.295, -0.510,  0.043,  0.118,  1.000},
              { -0.354, -0.408,  0.043,  0.118,  1.000},
              { -0.413, -0.306,  0.043,  0.118,  1.000},
              { -0.472, -0.204,  0.043,  0.118,  1.000},
              { -0.531, -0.102,  0.043,  0.118,  1.000},
              { -0.589, -0.000,  0.043,  0.118,  1.000},
              { -0.531,  0.102,  0.043,  0.118,  1.000},
              { -0.472,  0.204,  0.043,  0.118,  1.000},
              { -0.413,  0.306,  0.043,  0.118,  1.000},
              { -0.354,  0.408,  0.043,  0.118,  1.000},
              { -0.295,  0.510,  0.043,  0.118,  1.000},
              { -0.177,  0.510,  0.043,  0.118,  1.000},
              { -0.059,  0.510,  0.043,  0.118,  1.000},
              { 0.059,  0.510,  0.043,  0.118,  1.000},
              { 0.177,  0.510,  0.043,  0.118,  1.000},
              { 0.295,  0.510,  0.043,  0.118,  1.000},
              { 0.354,  0.408,  0.043,  0.118,  1.000},
              { 0.413,  0.306,  0.043,  0.118,  1.000},
              { 0.472,  0.204,  0.043,  0.118,  1.000},
              { 0.531,  0.102,  0.043,  0.118,  1.000},
              { 0.707, -0.000,  0.043,  0.118,  1.000},
              { 0.648, -0.102,  0.043,  0.118,  1.000},
              { 0.589, -0.204,  0.043,  0.118,  1.000},
              { 0.531, -0.306,  0.043,  0.118,  1.000},
              { 0.472, -0.408,  0.043,  0.118,  1.000},
              { 0.413, -0.510,  0.043,  0.118,  1.000},
              { 0.354, -0.613,  0.043,  0.118,  1.000},
              { 0.236, -0.613,  0.043,  0.118,  1.000},
              { 0.118, -0.613,  0.043,  0.118,  1.000},
              { 0.000, -0.613,  0.043,  0.118,  1.000},
              { -0.118, -0.613,  0.043,  0.118,  1.000},
              { -0.236, -0.613,  0.043,  0.118,  1.000},
              { -0.354, -0.613,  0.043,  0.118,  1.000},
              { -0.413, -0.510,  0.043,  0.118,  1.000},
              { -0.472, -0.408,  0.043,  0.118,  1.000},
              { -0.531, -0.306,  0.043,  0.118,  1.000},
              { -0.589, -0.204,  0.043,  0.118,  1.000},
              { -0.648, -0.102,  0.043,  0.118,  1.000},
              { -0.707, -0.000,  0.043,  0.118,  1.000},
              { -0.648,  0.102,  0.043,  0.118,  1.000},
              { -0.589,  0.204,  0.043,  0.118,  1.000},
              { -0.531,  0.306,  0.043,  0.118,  1.000},
              { -0.472,  0.408,  0.043,  0.118,  1.000},
              { -0.413,  0.510,  0.043,  0.118,  1.000},
              { -0.354,  0.613,  0.043,  0.118,  1.000},
              { -0.236,  0.613,  0.043,  0.118,  1.000},
              { -0.118,  0.613,  0.043,  0.118,  1.000},
              { 0.000,  0.613,  0.043,  0.118,  1.000},
              { 0.118,  0.613,  0.043,  0.118,  1.000},
              { 0.236,  0.613,  0.043,  0.118,  1.000},
              { 0.354,  0.613,  0.043,  0.118,  1.000},
              { 0.413,  0.510,  0.043,  0.118,  1.000},
              { 0.472,  0.408,  0.043,  0.118,  1.000},
              { 0.531,  0.306,  0.043,  0.118,  1.000},
              { 0.589,  0.204,  0.043,  0.118,  1.000},
              { 0.648,  0.102,  0.043,  0.118,  1.000},
              { 0.825, -0.000,  0.043,  0.118,  1.000},
              { 0.766, -0.102,  0.043,  0.118,  1.000},
              { 0.707, -0.204,  0.043,  0.118,  1.000},
              { 0.648, -0.306,  0.043,  0.118,  1.000},
              { 0.589, -0.408,  0.043,  0.118,  1.000},
              { 0.531, -0.510,  0.043,  0.118,  1.000},
              { 0.472, -0.613,  0.043,  0.118,  1.000},
              { 0.413, -0.715,  0.043,  0.118,  1.000},
              { 0.295, -0.715,  0.043,  0.118,  1.000},
              { 0.177, -0.715,  0.043,  0.118,  1.000},
              { 0.059, -0.715,  0.043,  0.118,  1.000},
              { -0.059, -0.715,  0.043,  0.118,  1.000},
              { -0.177, -0.715,  0.043,  0.118,  1.000},
              { -0.295, -0.715,  0.043,  0.118,  1.000},
              { -0.413, -0.715,  0.043,  0.118,  1.000},
              { -0.472, -0.613,  0.043,  0.118,  1.000},
              { -0.531, -0.510,  0.043,  0.118,  1.000},
              { -0.589, -0.408,  0.043,  0.118,  1.000},
              { -0.648, -0.306,  0.043,  0.118,  1.000},
              { -0.707, -0.204,  0.043,  0.118,  1.000},
              { -0.766, -0.102,  0.043,  0.118,  1.000},
              { -0.825, -0.000,  0.043,  0.118,  1.000},
              { -0.766,  0.102,  0.043,  0.118,  1.000},
              { -0.707,  0.204,  0.043,  0.118,  1.000},
              { -0.648,  0.306,  0.043,  0.118,  1.000},
              { -0.589,  0.408,  0.043,  0.118,  1.000},
              { -0.531,  0.510,  0.043,  0.118,  1.000},
              { -0.472,  0.613,  0.043,  0.118,  1.000},
              { -0.413,  0.715,  0.043,  0.118,  1.000},
              { -0.295,  0.715,  0.043,  0.118,  1.000},
              { -0.177,  0.715,  0.043,  0.118,  1.000},
              { -0.059,  0.715,  0.043,  0.118,  1.000},
              { 0.059,  0.715,  0.043,  0.118,  1.000},
              { 0.177,  0.715,  0.043,  0.118,  1.000},
              { 0.295,  0.715,  0.043,  0.118,  1.000},
              { 0.413,  0.715,  0.043,  0.118,  1.000},
              { 0.472,  0.613,  0.043,  0.118,  1.000},
              { 0.531,  0.510,  0.043,  0.118,  1.000},
              { 0.589,  0.408,  0.043,  0.118,  1.000},
              { 0.648,  0.306,  0.043,  0.118,  1.000},
              { 0.707,  0.204,  0.043,  0.118,  1.000},
              { 0.766,  0.102,  0.043,  0.118,  1.000},
              { 0.943, -0.000,  0.043,  0.118,  1.000},
              { 0.884, -0.102,  0.043,  0.118,  1.000},
              { 0.825, -0.204,  0.043,  0.118,  1.000},
              { 0.766, -0.306,  0.043,  0.118,  1.000},
              { 0.707, -0.408,  0.043,  0.118,  1.000},
              { 0.648, -0.510,  0.043,  0.118,  1.000},
              { 0.589, -0.613,  0.043,  0.118,  1.000},
              { 0.531, -0.715,  0.043,  0.118,  1.000},
              { 0.472, -0.817,  0.043,  0.118,  1.000},
              { 0.354, -0.817,  0.043,  0.118,  1.000},
              { 0.236, -0.817,  0.043,  0.118,  1.000},
              { 0.118, -0.817,  0.043,  0.118,  1.000},
              { 0.000, -0.817,  0.043,  0.118,  1.000},
              { -0.118, -0.817,  0.043,  0.118,  1.000},
              { -0.236, -0.817,  0.043,  0.118,  1.000},
              { -0.354, -0.817,  0.043,  0.118,  1.000},
              { -0.472, -0.817,  0.043,  0.118,  1.000},
              { -0.531, -0.715,  0.043,  0.118,  1.000},
              { -0.589, -0.613,  0.043,  0.118,  1.000},
              { -0.648, -0.510,  0.043,  0.118,  1.000},
              { -0.707, -0.408,  0.043,  0.118,  1.000},
              { -0.766, -0.306,  0.043,  0.118,  1.000},
              { -0.825, -0.204,  0.043,  0.118,  1.000},
              { -0.884, -0.102,  0.043,  0.118,  1.000},
              { -0.943, -0.000,  0.043,  0.118,  1.000},
              { -0.884,  0.102,  0.043,  0.118,  1.000},
              { -0.825,  0.204,  0.043,  0.118,  1.000},
              { -0.766,  0.306,  0.043,  0.118,  1.000},
              { -0.707,  0.408,  0.043,  0.118,  1.000},
              { -0.648,  0.510,  0.043,  0.118,  1.000},
              { -0.589,  0.613,  0.043,  0.118,  1.000},
              { -0.531,  0.715,  0.043,  0.118,  1.000},
              { -0.472,  0.817,  0.043,  0.118,  1.000},
              { -0.354,  0.817,  0.043,  0.118,  1.000},
              { -0.236,  0.817,  0.043,  0.118,  1.000},
              { -0.118,  0.817,  0.043,  0.118,  1.000},
              { 0.000,  0.817,  0.043,  0.118,  1.000},
              { 0.118,  0.817,  0.043,  0.118,  1.000},
              { 0.236,  0.817,  0.043,  0.118,  1.000},
              { 0.354,  0.817,  0.043,  0.118,  1.000},
              { 0.472,  0.817,  0.043,  0.118,  1.000},
              { 0.531,  0.715,  0.043,  0.118,  1.000},
              { 0.589,  0.613,  0.043,  0.118,  1.000},
              { 0.648,  0.510,  0.043,  0.118,  1.000},
              { 0.707,  0.408,  0.043,  0.118,  1.000},
              { 0.766,  0.306,  0.043,  0.118,  1.000},
              { 0.825,  0.204,  0.043,  0.118,  1.000},
              { 0.884,  0.102,  0.043,  0.118,  1.000},
              { 1.061, -0.000,  0.043,  0.118,  1.000},
              { 1.002, -0.102,  0.043,  0.118,  1.000},
              { 0.943, -0.204,  0.043,  0.118,  1.000},
              { 0.884, -0.306,  0.043,  0.118,  1.000},
              { 0.825, -0.408,  0.043,  0.118,  1.000},
              { 0.766, -0.510,  0.043,  0.118,  1.000},
              { 0.707, -0.613,  0.043,  0.118,  1.000},
              { 0.648, -0.715,  0.043,  0.118,  1.000},
              { 0.589, -0.817,  0.043,  0.118,  1.000},
              { 0.531, -0.919,  0.043,  0.118,  1.000},
              { 0.413, -0.919,  0.043,  0.118,  1.000},
              { 0.295, -0.919,  0.043,  0.118,  1.000},
              { 0.177, -0.919,  0.043,  0.118,  1.000},
              { 0.059, -0.919,  0.043,  0.118,  1.000},
              { -0.059, -0.919,  0.043,  0.118,  1.000},
              { -0.177, -0.919,  0.043,  0.118,  1.000},
              { -0.295, -0.919,  0.043,  0.118,  1.000},
              { -0.413, -0.919,  0.043,  0.118,  1.000},
              { -0.531, -0.919,  0.043,  0.118,  1.000},
              { -0.589, -0.817,  0.043,  0.118,  1.000},
              { -0.648, -0.715,  0.043,  0.118,  1.000},
              { -0.707, -0.613,  0.043,  0.118,  1.000},
              { -0.766, -0.510,  0.043,  0.118,  1.000},
              { -0.825, -0.408,  0.043,  0.118,  1.000},
              { -0.884, -0.306,  0.043,  0.118,  1.000},
              { -0.943, -0.204,  0.043,  0.118,  1.000},
              { -1.002, -0.102,  0.043,  0.118,  1.000},
              { -1.061, -0.000,  0.043,  0.118,  1.000},
              { -1.002,  0.102,  0.043,  0.118,  1.000},
              { -0.943,  0.204,  0.043,  0.118,  1.000},
              { -0.884,  0.306,  0.043,  0.118,  1.000},
              { -0.825,  0.408,  0.043,  0.118,  1.000},
              { -0.766,  0.510,  0.043,  0.118,  1.000},
              { -0.707,  0.613,  0.043,  0.118,  1.000},
              { -0.648,  0.715,  0.043,  0.118,  1.000},
              { -0.589,  0.817,  0.043,  0.118,  1.000},
              { -0.531,  0.919,  0.043,  0.118,  1.000},
              { -0.413,  0.919,  0.043,  0.118,  1.000},
              { -0.295,  0.919,  0.043,  0.118,  1.000},
              { -0.177,  0.919,  0.043,  0.118,  1.000},
              { -0.059,  0.919,  0.043,  0.118,  1.000},
              { 0.059,  0.919,  0.043,  0.118,  1.000},
              { 0.177,  0.919,  0.043,  0.118,  1.000},
              { 0.295,  0.919,  0.043,  0.118,  1.000},
              { 0.413,  0.919,  0.043,  0.118,  1.000},
              { 0.531,  0.919,  0.043,  0.118,  1.000},
              { 0.589,  0.817,  0.043,  0.118,  1.000},
              { 0.648,  0.715,  0.043,  0.118,  1.000},
              { 0.707,  0.613,  0.043,  0.118,  1.000},
              { 0.766,  0.510,  0.043,  0.118,  1.000},
              { 0.825,  0.408,  0.043,  0.118,  1.000},
              { 0.884,  0.306,  0.043,  0.118,  1.000},
              { 0.943,  0.204,  0.043,  0.118,  1.000},
              { 1.002,  0.102,  0.043,  0.118,  1.000},
              { 1.179, -0.000,  0.043,  0.118,  1.000},
              { 1.120, -0.102,  0.043,  0.118,  1.000},
              { 1.061, -0.204,  0.043,  0.118,  1.000},
              { 1.002, -0.306,  0.043,  0.118,  1.000},
              { 0.943, -0.408,  0.043,  0.118,  1.000},
              { 0.884, -0.510,  0.043,  0.118,  1.000},
              { 0.825, -0.613,  0.043,  0.118,  1.000},
              { 0.766, -0.715,  0.043,  0.118,  1.000},
              { 0.707, -0.817,  0.043,  0.118,  1.000},
              { 0.648, -0.919,  0.043,  0.118,  1.000},
              { 0.589, -1.021,  0.043,  0.118,  1.000},
              { 0.472, -1.021,  0.043,  0.118,  1.000},
              { 0.354, -1.021,  0.043,  0.118,  1.000},
              { 0.236, -1.021,  0.043,  0.118,  1.000},
              { 0.118, -1.021,  0.043,  0.118,  1.000},
              { 0.000, -1.021,  0.043,  0.118,  1.000},
              { -0.118, -1.021,  0.043,  0.118,  1.000},
              { -0.236, -1.021,  0.043,  0.118,  1.000},
              { -0.354, -1.021,  0.043,  0.118,  1.000},
              { -0.472, -1.021,  0.043,  0.118,  1.000},
              { -0.589, -1.021,  0.043,  0.118,  1.000},
              { -0.648, -0.919,  0.043,  0.118,  1.000},
              { -0.707, -0.817,  0.043,  0.118,  1.000},
              { -0.766, -0.715,  0.043,  0.118,  1.000},
              { -0.825, -0.613,  0.043,  0.118,  1.000},
              { -0.884, -0.510,  0.043,  0.118,  1.000},
              { -0.943, -0.408,  0.043,  0.118,  1.000},
              { -1.002, -0.306,  0.043,  0.118,  1.000},
              { -1.061, -0.204,  0.043,  0.118,  1.000},
              { -1.120, -0.102,  0.043,  0.118,  1.000},
              { -1.179, -0.000,  0.043,  0.118,  1.000},
              { -1.120,  0.102,  0.043,  0.118,  1.000},
              { -1.061,  0.204,  0.043,  0.118,  1.000},
              { -1.002,  0.306,  0.043,  0.118,  1.000},
              { -0.943,  0.408,  0.043,  0.118,  1.000},
              { -0.884,  0.510,  0.043,  0.118,  1.000},
              { -0.825,  0.613,  0.043,  0.118,  1.000},
              { -0.766,  0.715,  0.043,  0.118,  1.000},
              { -0.707,  0.817,  0.043,  0.118,  1.000},
              { -0.648,  0.919,  0.043,  0.118,  1.000},
              { -0.589,  1.021,  0.043,  0.118,  1.000},
              { -0.472,  1.021,  0.043,  0.118,  1.000},
              { -0.354,  1.021,  0.043,  0.118,  1.000},
              { -0.236,  1.021,  0.043,  0.118,  1.000},
              { -0.118,  1.021,  0.043,  0.118,  1.000},
              { 0.000,  1.021,  0.043,  0.118,  1.000},
              { 0.118,  1.021,  0.043,  0.118,  1.000},
              { 0.236,  1.021,  0.043,  0.118,  1.000},
              { 0.354,  1.021,  0.043,  0.118,  1.000},
              { 0.472,  1.021,  0.043,  0.118,  1.000},
              { 0.589,  1.021,  0.043,  0.118,  1.000},
              { 0.648,  0.919,  0.043,  0.118,  1.000},
              { 0.707,  0.817,  0.043,  0.118,  1.000},
              { 0.766,  0.715,  0.043,  0.118,  1.000},
              { 0.825,  0.613,  0.043,  0.118,  1.000},
              { 0.884,  0.510,  0.043,  0.118,  1.000},
              { 0.943,  0.408,  0.043,  0.118,  1.000},
              { 1.002,  0.306,  0.043,  0.118,  1.000},
              { 1.061,  0.204,  0.043,  0.118,  1.000},
              { 1.120,  0.102,  0.043,  0.118,  1.000},
              { 1.179, -0.204,  0.043,  0.118,  1.000},
              { 1.120, -0.306,  0.043,  0.118,  1.000},
              { 1.061, -0.408,  0.043,  0.118,  1.000},
              { 1.002, -0.510,  0.043,  0.118,  1.000},
              { 0.943, -0.613,  0.043,  0.118,  1.000},
              { 0.884, -0.715,  0.043,  0.118,  1.000},
              { 0.825, -0.817,  0.043,  0.118,  1.000},
              { 0.766, -0.919,  0.043,  0.118,  1.000},
              { 0.413, -1.123,  0.043,  0.118,  1.000},
              { 0.295, -1.123,  0.043,  0.118,  1.000},
              { 0.177, -1.123,  0.043,  0.118,  1.000},
              { 0.059, -1.123,  0.043,  0.118,  1.000},
              { -0.059, -1.123,  0.043,  0.118,  1.000},
              { -0.177, -1.123,  0.043,  0.118,  1.000},
              { -0.295, -1.123,  0.043,  0.118,  1.000},
              { -0.413, -1.123,  0.043,  0.118,  1.000},
              { -0.766, -0.919,  0.043,  0.118,  1.000},
              { -0.825, -0.817,  0.043,  0.118,  1.000},
              { -0.884, -0.715,  0.043,  0.118,  1.000},
              { -0.943, -0.613,  0.043,  0.118,  1.000},
              { -1.002, -0.510,  0.043,  0.118,  1.000},
              { -1.061, -0.408,  0.043,  0.118,  1.000},
              { -1.120, -0.306,  0.043,  0.118,  1.000},
              { -1.179, -0.204,  0.043,  0.118,  1.000},
              { -1.179,  0.204,  0.043,  0.118,  1.000},
              { -1.120,  0.306,  0.043,  0.118,  1.000},
              { -1.061,  0.408,  0.043,  0.118,  1.000},
              { -1.002,  0.510,  0.043,  0.118,  1.000},
              { -0.943,  0.613,  0.043,  0.118,  1.000},
              { -0.884,  0.715,  0.043,  0.118,  1.000},
              { -0.825,  0.817,  0.043,  0.118,  1.000},
              { -0.766,  0.919,  0.043,  0.118,  1.000},
              { -0.413,  1.123,  0.043,  0.118,  1.000},
              { -0.295,  1.123,  0.043,  0.118,  1.000},
              { -0.177,  1.123,  0.043,  0.118,  1.000},
              { -0.059,  1.123,  0.043,  0.118,  1.000},
              { 0.059,  1.123,  0.043,  0.118,  1.000},
              { 0.177,  1.123,  0.043,  0.118,  1.000},
              { 0.295,  1.123,  0.043,  0.118,  1.000},
              { 0.413,  1.123,  0.043,  0.118,  1.000},
              { 0.766,  0.919,  0.043,  0.118,  1.000},
              { 0.825,  0.817,  0.043,  0.118,  1.000},
              { 0.884,  0.715,  0.043,  0.118,  1.000},
              { 0.943,  0.613,  0.043,  0.118,  1.000},
              { 1.002,  0.510,  0.043,  0.118,  1.000},
              { 1.061,  0.408,  0.043,  0.118,  1.000},
              { 1.120,  0.306,  0.043,  0.118,  1.000},
              { 1.179,  0.204,  0.043,  0.118,  1.000},
              { 1.377,  0.000,  0.098,  0.000,  1.000},
              { 1.357, -0.233,  0.098,  0.000,  1.000},
              { 1.298, -0.459,  0.098,  0.000,  1.000},
              { 1.202, -0.671,  0.098,  0.000,  1.000},
              { 1.071, -0.865,  0.098,  0.000,  1.000},
              { 0.910, -1.033,  0.098,  0.000,  1.000},
              { 0.722, -1.172,  0.098,  0.000,  1.000},
              { 0.513, -1.277,  0.098,  0.000,  1.000},
              { 0.290, -1.346,  0.098,  0.000,  1.000},
              { 0.058, -1.375,  0.098,  0.000,  1.000},
              { -0.175, -1.366,  0.098,  0.000,  1.000},
              { -0.403, -1.316,  0.098,  0.000,  1.000},
              { -0.620, -1.229,  0.098,  0.000,  1.000},
              { -0.819, -1.107,  0.098,  0.000,  1.000},
              { -0.994, -0.953,  0.098,  0.000,  1.000},
              { -1.141, -0.771,  0.098,  0.000,  1.000},
              { -1.255, -0.567,  0.098,  0.000,  1.000},
              { -1.332, -0.347,  0.098,  0.000,  1.000},
              { -1.372, -0.117,  0.098,  0.000,  1.000},
              { -1.372,  0.117,  0.098,  0.000,  1.000},
              { -1.332,  0.347,  0.098,  0.000,  1.000},
              { -1.255,  0.567,  0.098,  0.000,  1.000},
              { -1.141,  0.771,  0.098,  0.000,  1.000},
              { -0.994,  0.953,  0.098,  0.000,  1.000},
              { -0.819,  1.107,  0.098,  0.000,  1.000},
              { -0.620,  1.229,  0.098,  0.000,  1.000},
              { -0.403,  1.316,  0.098,  0.000,  1.000},
              { -0.175,  1.366,  0.098,  0.000,  1.000},
              { 0.058,  1.375,  0.098,  0.000,  1.000},
              { 0.290,  1.346,  0.098,  0.000,  1.000},
              { 0.513,  1.277,  0.098,  0.000,  1.000},
              { 0.722,  1.172,  0.098,  0.000,  1.000},
              { 0.910,  1.033,  0.098,  0.000,  1.000},
              { 1.071,  0.865,  0.098,  0.000,  1.000},
              { 1.202,  0.671,  0.098,  0.000,  1.000},
              { 1.298,  0.459,  0.098,  0.000,  1.000},
              { 1.357,  0.233,  0.098,  0.000,  1.000},
              { 1.573, -0.134,  0.098,  0.000,  1.000},
              { 1.528, -0.398,  0.098,  0.000,  1.000},
              { 1.439, -0.650,  0.098,  0.000,  1.000},
              { 1.308, -0.884,  0.098,  0.000,  1.000},
              { 1.140, -1.092,  0.098,  0.000,  1.000},
              { 0.939, -1.269,  0.098,  0.000,  1.000},
              { 0.711, -1.410,  0.098,  0.000,  1.000},
              { 0.462, -1.510,  0.098,  0.000,  1.000},
              { 0.200, -1.566,  0.098,  0.000,  1.000},
              { -0.067, -1.577,  0.098,  0.000,  1.000},
              { -0.333, -1.543,  0.098,  0.000,  1.000},
              { -0.589, -1.465,  0.098,  0.000,  1.000},
              { -0.828, -1.344,  0.098,  0.000,  1.000},
              { -1.043, -1.185,  0.098,  0.000,  1.000},
              { -1.228, -0.992,  0.098,  0.000,  1.000},
              { -1.378, -0.770,  0.098,  0.000,  1.000},
              { -1.489, -0.526,  0.098,  0.000,  1.000},
              { -1.556, -0.267,  0.098,  0.000,  1.000},
              { -1.579,  0.000,  0.098,  0.000,  1.000},
              { -1.556,  0.267,  0.098,  0.000,  1.000},
              { -1.489,  0.526,  0.098,  0.000,  1.000},
              { -1.378,  0.770,  0.098,  0.000,  1.000},
              { -1.228,  0.992,  0.098,  0.000,  1.000},
              { -1.043,  1.185,  0.098,  0.000,  1.000},
              { -0.828,  1.344,  0.098,  0.000,  1.000},
              { -0.589,  1.465,  0.098,  0.000,  1.000},
              { -0.333,  1.543,  0.098,  0.000,  1.000},
              { -0.067,  1.577,  0.098,  0.000,  1.000},
              { 0.200,  1.566,  0.098,  0.000,  1.000},
              { 0.462,  1.510,  0.098,  0.000,  1.000},
              { 0.711,  1.410,  0.098,  0.000,  1.000},
              {  0.939,  1.269,  0.098,  0.000,  1.000},
              { 1.140,  1.092,  0.098,  0.000,  1.000},
              { 1.308,  0.884,  0.098,  0.000,  1.000},
              { 1.439,  0.650,  0.098,  0.000,  1.000},
              { 1.528,  0.398,  0.098,  0.000,  1.000},
              { 1.573,  0.134,  0.098,  0.000,  1.000},
              { 1.770,  0.000,  0.098,  0.000,  1.000},
               { 1.744, -0.299,  0.098,  0.000,  1.000},
               { 1.669, -0.590,  0.098,  0.000,  1.000},
               { 1.545, -0.863,  0.098,  0.000,  1.000},
               { 1.377, -1.112,  0.098,  0.000,  1.000},
               { 1.169, -1.329,  0.098,  0.000,  1.000},
               { 0.928, -1.507,  0.098,  0.000,  1.000},
               { 0.660, -1.642,  0.098,  0.000,  1.000},
               { 0.373, -1.730,  0.098,  0.000,  1.000},
               { 0.075, -1.768,  0.098,  0.000,  1.000},
              { -0.225, -1.755,  0.098,  0.000,  1.000},
              { -0.518, -1.692,  0.098,  0.000,  1.000},
              { -0.797, -1.580,  0.098,  0.000,  1.000},
              { -1.052, -1.423,  0.098,  0.000,  1.000},
              { -1.278, -1.225,  0.098,  0.000,  1.000},
              { -1.466, -0.991,  0.098,  0.000,  1.000},
              { -1.613, -0.729,  0.098,  0.000,  1.000},
              { -1.713, -0.446,  0.098,  0.000,  1.000},
              { -1.763, -0.150,  0.098,  0.000,  1.000},
              { -1.763,  0.150,  0.098,  0.000,  1.000},
              { -1.713,  0.446,  0.098,  0.000,  1.000},
              { -1.613,  0.729,  0.098,  0.000,  1.000},
              { -1.466,  0.991,  0.098,  0.000,  1.000},
              { -1.278,  1.225,  0.098,  0.000,  1.000},
              { -1.052,  1.423,  0.098,  0.000,  1.000},
              { -0.797,  1.580,  0.098,  0.000,  1.000},
              { -0.518,  1.692,  0.098,  0.000,  1.000},
              { -0.225,  1.755,  0.098,  0.000,  1.000},
               { 0.075,  1.768,  0.098,  0.000,  1.000},
               { 0.373,  1.730,  0.098,  0.000,  1.000},
               { 0.660,  1.642,  0.098,  0.000,  1.000},
               { 0.928,  1.507,  0.098,  0.000,  1.000},
               { 1.169,  1.329,  0.098,  0.000,  1.000},
               { 1.377,  1.112,  0.098,  0.000,  1.000},
               { 1.545,  0.863,  0.098,  0.000,  1.000},
               { 1.669,  0.590,  0.098,  0.000,  1.000},
               { 1.744,  0.299,  0.098,  0.000,  1.000}};

 pmt zero={0.,0.,0.,0.,0.};

 if(i > NPMT || i <= 0) return (zero);

 return (data[i-1]);

}


/****************************************************************/
/*Subroutines FOR VVV CORRECTION ROUTINE END HERE */
/****************************************************************/

/*-------------------------------------------------------------------------*/

/* callbacks for form track_form */
void get_object(FL_OBJECT *ob, long data)
{
 int pole,i=0,j=2,len,spaces,exists=0,loop;
 const char *id_in;
 char  string[50],el_string[10],az_string[10];
 
 if(flags.freeze && !flags.doing_corrections){
   flag_alert();
   return;
 }
   
 pole=fl_show_choice("You can select a normal source from a list (LIST)",
		     "or select a previous burst source (BURST)",
		     "or enter change source name only (NAME)",3,"LIST",
		     "BURST","NAME",1);
 if(pole==1){

   if(source_list[0]!=NULL)
     exists=fl_show_question("Do you want use the previous object list ?\n
		       \n(Press NO to select a new list file)",0);
   
   if(!exists){ 
 
     /* get the objects from the source list */
     getobj();
     if(source_list[0]==NULL) /* getobj failed...no sources here */
     {
       printf("There were no sources in source list\n");
       return;
     }
   }
   else 
   {
      for(loop=0;loop<=sources;loop++){
         if(source_list[loop]->rao >st-6 && source_list[loop]->rao < st )
	 {
           azandel(source_list[loop]->rao,source_list[loop]->deco,
		   &source_list[loop]->el,&source_list[loop]->az,st);
           source_list[loop]->visible=2;
         }
         else if (source_list[loop]->rao < st+6 && source_list[loop]->rao > st)
	 {
           azandel(source_list[loop]->rao,source_list[loop]->deco,
		   &source_list[loop]->el,&source_list[loop]->az,st);
           source_list[loop]->visible=1;
         }
         else
           source_list[loop]->visible=0;
         
      }
   }
   /* set up browser to get the wanted object */
   fl_show_form(fd_browser_form->browser_form,FL_PLACE_CENTER,FL_FULLBORDER,
                "get_object");
   fl_clear_browser(fd_browser_form->get_object);
   fl_set_browser_fontsize(fd_browser_form->get_object,
			   FL_LARGE_SIZE);
   fl_freeze_form(fd_browser_form->browser_form);
   /*put information in the browser window first*/
   strcpy(string,"@C1@fPRESS HERE TO CANCEL");
   fl_add_browser_line(fd_browser_form->get_object,string);
   strcpy(string,"@C1@fOBJECT             elevation     azimuth");
   fl_add_browser_line(fd_browser_form->get_object,string);
   /* now add polaris as the first on the list */
       strcpy(string,"@C22@f");
       strcat(string,source_list[sources]->name);
       len = strlen(string);
       /*add spaces to put elevation at the correct spot*/
       for(spaces=0;spaces<(26-len);spaces++)
         strcat(string," ");
       sprintf(el_string,"%6.2f  ",
	       ((long)(source_list[sources]->el*100.)/100.));
       sprintf(az_string,"%6.2f  ",
	       ((long)(source_list[sources]->az*100.)/100.));
       strcat(string,el_string);
       strcat(string,"      ");
       strcat(string,az_string);
       fl_add_browser_line(fd_browser_form->get_object,string);
       position[j]=sources;
       j++;
   for(i=0;i<sources;i++){
     if (source_list[i]->visible){
       strcpy(string,"@C4@f");
       if (source_list[i]->visible == 2)
          strcpy(string,"@C7@f");
       /* 7 is white */
       strcat(string,source_list[i]->name);
       len = strlen(string);
       /*add spaces to put elevation at the correct spot*/
       for(spaces=0;spaces<(25-len);spaces++)
         strcat(string," ");
       sprintf(el_string,"%6.2f  ",((long)(source_list[i]->el*100.)/100.));
       sprintf(az_string,"%6.2f  ",
	       ((long)(source_list[i]->az*100.)/100.));
       strcat(string,el_string);
       strcat(string,"      ");
       strcat(string,az_string);
       fl_add_browser_line(fd_browser_form->get_object,string);
       position[j]=i;
       j++;
     }
   }
   for(i=0;i<sources;i++){
     if (!source_list[i]->visible){
       strcpy(string,"@f");
       strcat(string,source_list[i]->name);
       fl_add_browser_line(fd_browser_form->get_object,string);
       position[j]=i;
       j++;
     }
   }
   fl_unfreeze_form(fd_browser_form->browser_form);
 }
 else if(pole==2)
  getburst();
 else if(pole==3){
  id_in=fl_show_input("Enter the new object name",id);
  fl_set_object_label(ob,id_in);
  strcpy(id,id_in);
  id_in=fl_show_input("Enter the object code (four letters)",id_code);
  fl_set_object_label(fd_track_form->code,id_in);
  strcpy(id_code,id_in);
 }
}
void choose_object(FL_OBJECT *ob, long data)
{
  int test=0;
  double epch_o,epch_p;

  fl_hide_form(fd_browser_form->browser_form);
  test=fl_get_browser(fd_browser_form->get_object);
  if(test<=2)
    return;
  if(test>2){
    test-=1;
    test=position[test];
    strcpy(id,source_list[test]->name);
    strcpy(id_code, source_list[test]->source_code);
    ra   = source_list[test]->rao;
    dec  = source_list[test]->deco;
    pra  = source_list[test]->rap;
    pdec = source_list[test]->decp;
    epch_o = source_list[test]->epocho;
    epch_p = source_list[test]->epochp;
    	 
    /* Precess from J2000.0 to current year */
    to2000( epch_o, &ra, &dec );
    from2000((double) (gettime()), &ra, &dec );
    to2000( epch_p, &pra, &pdec );
    from2000((double) (gettime()), &pra, &pdec);
    sra  = ra;
    sdec = dec;
    slewsta = on; /* INDICATES THAT THE TELESCOPE IS ON SOURCE */
    /* setup on/off source buttins */
    fl_set_button(fd_track_form->onsource,1); /* release button */
    fl_deactivate_object(fd_track_form->onsource);
    fl_set_button(fd_track_form->offsource,0); /* release button */
    fl_activate_object(fd_track_form->offsource);
    track = no; /* TELLS THE MAIN LOOP TO STOP TRACKING */
    stoptel();
   
    ftohms(ra, rastr);
    ftohms(dec, decstr);
  }
}


void change_ra(FL_OBJECT *obj, long data)
{
  const char *input;
  char string[10];
  float hhmmss = 0.;
  int  test;

  if(flags.freeze && !flags.doing_corrections){
    flag_alert();
    return;
  }

  test=fl_show_question("\nDo you really want to change the RA ?\n",0);
  if(!test)  /*if answer is "no" then return */
    return;
  /*convert current ra string to proper format */
  strcpy( string, rastr);
  string[3]=string[4];
  string[4]=string[5];
  string[5]=string[7];
  string[6]=string[8];
  string[7]=string[9];
  while(test==1){
    input=fl_show_input("Enter the new RA (hhmmss)",string);
    hhmmss=atof(input);
    if((ftod(hhmmss, &sra) == 0) || (sra > 24.) || (sra < 0.))
      fl_show_alert("","Incorrect entry...try again","",0);
    else{
      ra = sra;
      ftohms(ra, rastr);
      test=0;
      fl_set_object_label(obj,rastr);
    }
  }
}
void change_dec(FL_OBJECT *obj, long data)
{
  const char *input;
  char string[10];
  float ddmmss = 0.;
  int  test;

  if(flags.freeze && !flags.doing_corrections){
    flag_alert();
    return;
  }

  test=fl_show_question("\nDo you really want to change the DEC ?\n",0);
  if(!test)  /*if answer is "no" then return */
    return;
  /*convert current ra string to proper format */
  strcpy( string, decstr);
  string[3]=string[4];
  string[4]=string[5];
  string[5]=string[7];
  string[6]=string[8];
  string[7]=string[9];
  while(test==1){
    input=fl_show_input("Enter the new DEC (ddmmss)",string);
    ddmmss=atof(input);
    if((ftod(ddmmss, &sdec) == 0) || (fabs(sdec) > 90.))
      fl_show_alert("","Incorrect entry...try again","",0);
    else{
      dec = sdec;
      ftohms(dec, decstr);
      test=0;
      fl_set_object_label(obj,decstr);
    }
  }
}
void change_offset(FL_OBJECT *obj, long data)
{
  const char *input;
  int  test;

  if(flags.freeze  && !flags.doing_corrections){
    flag_alert();
    return;
  }

  test=fl_show_question("\nDo you really want to change the OFFSET ?\n",0);
  if(!test)  /*if answer is "no" then return */
    return;
  while(test==1){
    input=fl_show_input("Enter OFFSET (number of RA minutes)","30");
    offset=atoi(input);
    if((offset > 720)||(offset < 0))
      fl_show_alert("","Incorrect entry...valid values 0 < offset < 720","",0);
    else{
      fl_set_object_label(obj,input);
      test=0;
    }
  }
}

void change_mode(FL_OBJECT *obj, long data)
{

  if(flags.freeze && !flags.track_moon && !flags.doing_corrections){
    flag_alert();
    fl_set_button(obj,1); /* release button */
    return;
  }

    /* IF NOT TRACKING GO INTO TRACKING MODE */
    if(!track){
      if(!status.el_drive_ready || !status.el_auto){
        fl_show_alert("",
		      "Elevation AUTO mode is not selected",
		      "Toggle the switch before tracking",0);
        return;
      }
      if(!status.az_drive_ready || !status.az_auto){
        fl_show_alert("",
		      "Azimuth AUTO mode is not selected",
		      "Toggle the switch before tracking",0);
        return;
      }
        setup_motors(&az_info,1,0); /* initialize controllers */
        setup_motors(&el_info,1,0);
        fl_set_object_label(fd_track_form->mode,"TRACKING");
	track = yes; /* VARIABLE THAT TELLS THE MAIN LOOP TO */
		     /* TRACK THE CURRENT SOURCE */
	slewsta = on; /* INDICATES THAT THE TELESCOPE IS ON SOURCE */
	ra = sra;
	ftohms(ra, rastr);
        fl_set_button(fd_track_form->onsource,1); /* release button */
        fl_deactivate_object(fd_track_form->onsource);
        fl_set_button(fd_track_form->offsource,0); /* release button */
        fl_activate_object(fd_track_form->offsource);
	datain();
    }
    else{ /* YOU ARE TRACKING ALREADY SO EXIT THE TRACK MODE */
        fl_set_object_label(fd_track_form->mode,"STANDBY");
	track = no; /* TELLS THE MAIN LOOP TO STOP TRACKING */
	stoptel();
    }
    return;
}

void change_slew(FL_OBJECT *obj, long data)
{ 
 /* first check if drift mode is being used and respond to it */
 if (flags.drift_mode){
    flags.drift_mode=0;
    flags.drift_flag=0;
    flags.freeze=0;
    track=no;
    fl_set_button(fd_track_form->mode,0); /* release button */
    fl_set_object_label(fd_track_form->mode,"STANDBY");
    fl_set_object_label(fd_track_form->slew,"STOPPED");
    objel=tempel;
    objaz=tempaz;
    stoptel();
    clear_message();
    place_data();
    update();
    return;
  }
 if (track){
   if((data==0) || ((slewsta == on)&&(data==2))){
       slewsta = off;
       ra = sra + offset/60.;
       fl_set_button(fd_track_form->offsource,1); /* release button */
       fl_deactivate_object(fd_track_form->offsource);
       fl_set_button(fd_track_form->onsource,0); /* release button */
       fl_activate_object(fd_track_form->onsource);
   }
   else {
       slewsta = on;
       ra = sra;
       fl_set_button(fd_track_form->onsource,1); /* release button */
       fl_deactivate_object(fd_track_form->onsource);
       fl_set_button(fd_track_form->offsource,0); /* release button */
       fl_activate_object(fd_track_form->offsource);
   }
 }
 else{
       fl_set_button(obj,0); /* release button */
 }   
   ftohms(ra, rastr);

 return;
}

void point_check(FL_OBJECT *obj, long data)
{

  if(flags.freeze && !flags.pointing_check && !flags.doing_corrections){
    flag_alert();
    fl_set_button(fd_track_form->pointcheck,0); /* release button */
    return;
  }
  if(!track){
    fl_show_alert("","Must have tracking turned on",
		  "before doing a pointing check",0);
    fl_set_button(fd_track_form->pointcheck,0); /* release button */
    return;
  } 
    if(!flags.pointing_check){
      flags.pointing_check=1;
      flags.freeze=1;
      strcpy( temp_id, id);
      strcpy( temp_code, id_code);
      strcpy( id, "POINTING CHECK");
      strcpy( id_code, "STAR");
      /*change forground of button to indicate a pointing check*/
      fl_set_object_lcol(fd_track_form->code,FL_MAGENTA);
      ra = pra;
      dec = pdec;
      ftohms( ra, rastr );
      ftohms( dec, decstr );
      strcpy(current_message[0],"POINTING CHECK");
      strcpy(current_message[1],"PRESS 'P' KEY TO RETURN TO THE SOURCE");
      strcpy(current_message[2],"");
      place_message();
      mode_int=POINTCHECK;
      track = yes;
      fl_set_object_label(fd_track_form->mode,"TRACKING");
      send_time+=500;   /* Make program send to vax immediatly */
      send_to_VHEGRO();
      /* turn off on/off source tracking buttons */
      fl_set_button(fd_track_form->onsource,0); /* release button */
      fl_deactivate_object(fd_track_form->onsource);
      fl_set_button(fd_track_form->offsource,0); /* release button */
      fl_deactivate_object(fd_track_form->offsource);
      fl_set_object_label(fd_track_form->slew,"POINTING");
      while(flags.pointing_check){
        update();
        fl_check_only_forms();
      }
    }
    else{
      /* Finished pointing check ... return to object */
      flags.pointing_check=0;
      flags.freeze=0;
      fl_set_object_lcol(fd_track_form->code,FL_BLACK);
      ra = sra;
      dec = sdec;
      ftohms(ra, rastr);
      ftohms(dec, decstr);
      clear_message();
      strcpy(id,temp_id);
      strcpy(id_code,temp_code);
      track = yes;
      /* turn on on/off source tracking buttons */
      if(slewsta == on){
        fl_set_button(fd_track_form->onsource,1); /* release button */
        fl_deactivate_object(fd_track_form->onsource);
        fl_activate_object(fd_track_form->offsource);
        fl_set_object_label(fd_track_form->slew,"SLEWING ON");
      }
      else{
        fl_set_button(fd_track_form->offsource,1); /* release button */
        fl_deactivate_object(fd_track_form->offsource);
        fl_activate_object(fd_track_form->onsource);
        fl_set_object_label(fd_track_form->slew,"SLEWING OFF");
      } 
      datain();
    }
    return;

}
void change_first_source(FL_OBJECT *obj, long data)
{

  if(flags.freeze){
    flag_alert();
    return;
  }

 if (offset > 0.0){
   if(!fl_show_question("\nDo you want to switch to OFF before ON runs?\n",0))
      return;
 }
 else{
   if(!fl_show_question("\nDo you want to switch to ON before OFF runs?\n",0))
      return;
 }
 offset=0.0-offset;
 if (offset > 0.0)
   fl_set_object_label(obj,"ON-OFF");
 else
   fl_set_object_label(obj,"OFF-ON");
}
void change_drift_mode(FL_OBJECT *obj, long data)
{
  const char *input;
  char  string[50],temp[10];
  double dumel=0,dumaz=0;
  int test=1,test2=1;

  if(flags.drift_mode || flags.freeze){
    flag_alert();
    return;
  }

  if(!fl_show_question("\nAre you sure you want to go into drift mode?\n",0))
    return;
  while(test2==1){
    test=1;
    while(test==1){
      input=fl_show_input("Enter the desired Azimuth","0.0");
      if(input==NULL)
	return;
      dumaz=atof(input);
      if(dumaz < 0.0 || dumaz >= 360.0)
	fl_show_alert("","Range is 0.0 to 359.99","",0);
      else
	test=0;     
    }
    test=1;
    while(test==1){
      input=fl_show_input("Enter the desired Elevation","20.0");
      if(input==NULL)
	return;
      dumel=atof(input);
      if( dumel < -0.5 || dumel >= 90.5){
	fl_show_alert("","Range is -0.5 to 90.5","",0);
        test=1;
      }
      else
	test=0;
    }
    if( (dumel < LOW_EL) && !((dumaz < 1.5) || (dumaz > 358.5))){
      fl_show_alert("","AZ must be between 358.5 and 1.5",
		    "degrees in order to go below 16 degrees",0);
      test2=1;
    }
    else{
      test=0;
      test2=0;
    }     
  }

  sprintf(temp,"%7.2f  ",((long)(RANGE(dumaz,360.)*100.)/100.));
  strcpy(string,"MOVING TO AZ=");
  strcat(string,temp);  
  strcat(string,"   EL=");
  sprintf(temp,"%7.2f  ",((long)(dumel*100.)/100.));
  strcat(string,temp);  
  strcpy(current_message[0],"TELESCOPE IS IN DRIFT MODE");
  strcpy(current_message[1],string);
  strcpy(current_message[2],"PRESS 'S' (mouse in main window) TO STOP");
  place_message();
  strcpy(mode,"DRIFT");
  mode_int=DRIFTSCAN;
  send_time+=200;   
  send_to_VHEGRO();
  flags.drift_mode=1; 
  drift_tel(dumel,dumaz); /* enter drift mode */
}
void commands(FL_OBJECT *ob, long data)
{
  int epoch = 0, test=0,choice=0;
  const char  *input;
  char  temp[80],string[50];
  FILE  *fp;
  double telaz_temp=0.0;

  test=fl_get_menu(ob);
  if(test==1){
    if(flags.freeze){
      flag_alert();
      return;
    }
    /*    if(!fl_show_question("\nAre you sure you want to move telescope home?\n",
			 0))
			 return;*/
    strcpy( temp_id, id);
    strcpy( temp_code, id_code);
    strcpy( id, "HOME");
    strcpy( id_code, "HOME");
    tempel=objel;
    tempaz=objaz;
    flags.drift_mode=1;
    flags.freeze=1;
    strcpy(mode,"HOME");
    /* NOTE mode needs to be changed for HOME */   
    mode_int=STANDBY;
    send_time+=200;  /*  Make program send to vax immediatly */
    send_to_VHEGRO();
    if (telel>LOW_EL){
      strcpy(current_message[0],"MOVING TELESCOPE HOME");
      strcpy(current_message[1],"POSITION: AZ=359.99  EL=17.0");
      strcpy(current_message[2],"PRESS 'S' (mouse in main window) TO STOP");
      place_message();

      drift_tel(LOW_EL+1.0,359.99);
    }
    strcpy(current_message[0],"MOVING TELESCOPE HOME");
    strcpy(current_message[1],"POSITION: AZ=359.99  EL=0.0");
    strcpy(current_message[2],"PRESS 'S' (mouse in main window) TO STOP");
    place_message();
    drift_tel(0.0,359.99); /* enter drift mode and go home */

    strcpy( id, temp_id);
    strcpy( id_code, temp_code);
    flags.drift_mode=0;
    flags.freeze=0;
    track=no;
    fl_set_button(fd_track_form->mode,0); /* release button */
    fl_set_object_label(fd_track_form->mode,"STANDBY");
    objel=tempel;
    objaz=tempaz;
    clear_message();
    update();
  }
  else if(test==2){
    if(flags.freeze){
      flag_alert();
      return;
    }

    if(!fl_show_question("\nAre you sure you want to move the telescope\n
			 to the ZENITH?",0))
      return;
    sprintf(temp,"%7.2f  ",((long)(RANGE(telaz,360.)*100.)/100.));
    strcpy(string,"MOVING TO: AZ=");
    strcat(string,temp);  
    /* SJF 2004-09-13   strcat(string,"   EL=89.0"); */
    strcat(string,"   EL=86.0");
    strcpy(current_message[0],"MOVING TELESCOPE TO ZENITH");
    strcpy(current_message[1],string);
    strcpy(current_message[2],"PRESS 'S' (mouse in main window) TO STOP");
    strcpy( temp_id, id);
    strcpy( temp_code, id_code);
    strcpy( id, "ZENITH");
    strcpy( id_code, "UP!!");
    place_message();
    strcpy(mode,"ZENITH");
    /* NOTE mode needs to be changed for ZENITH */   
    mode_int=ZENITH;
    send_time+=200;  /*  Make program send to vax immediatly*/ 
    send_to_VHEGRO();
    flags.drift_mode=1;
    flags.freeze=1;
    /* SJF 2004-09-13 drift_tel(89.0,telaz); */ /* enter drift mode but not go home */
    drift_tel(86.0,telaz); /* enter drift mode but not go home */
    flags.drift_mode=0;
    flags.freeze=0;
    track=no;
    strcpy( id, temp_id);
    strcpy( id_code, temp_code);
  }
  else if(test==3){
    test=1;
    while(test==1){
      input=fl_show_input("PRECESS FROM WHAT EPOCH (yyyy)","1999");
      epoch=atoi(input);
      if((epoch < 1900) || (epoch > 2500))
        fl_show_alert("","Incorrect entry...valid values 1900 < epoch < 2100",
		      "",0);
      else{
	 to2000(epoch, &sra, &sdec);
	 from2000((double)(gettime()), &sra, &sdec);
	 ra  = sra;
	 dec = sdec;
	 ftohms(ra, rastr);
	 ftohms(dec, decstr);
         test=0;
      }
    }    
  }
  else if(test==4){
  }
  else if(test==5){
    /*Request to do corrections for telescope pointing*/
    if(flags.freeze){
      flag_alert();
      return;
    }
    /*    if(!fl_show_question("Correcting the telescope pointing requires\n
			looking at a MINIMUM of three well chosen objects\n
			 Do you still want to continue?",0))
			 return;*/
    star_corrections(); /*continue on to doing the corrections*/
  }
  else if(test==6){
    if(flags.freeze){
      flag_alert();
      return;
    }

  if(get_moon_file()){  /*calculate the moon position for today*/
  choice=fl_show_choice("Choose between the scanning method and",
	          "the five position method (FIVE)",
	          "",2,"SCAN","FIVE","",1);

    if(choice==1){
       moon_scan_setup();
       fl_show_form(fd_scan_form->scan_form,FL_PLACE_CENTERFREE,
		FL_FULLBORDER,"Moon Scan Tracking");
       flags.scan_mode=1;
       flags.track_moon=1;
       flags.freeze=1;
       moon_position=CENTER;
       strcpy(id_code,"MOON");
       moon_time=update_moon(moon_position);
    }
    else{
       fl_show_form(fd_moon_form->moon_form,FL_PLACE_CENTERFREE,
	  	    FL_FULLBORDER,"Moon Tracking");
       flags.track_moon=1;
       flags.freeze=1;
       moon_position=CENTER;
       strcpy(id_code,"MOON");
       moon_time=update_moon(moon_position);
    }

  }
  }
  else if(test==7){  /* Give status information on the telescope */
       limit_print("Last Read Status and Limit info",0);
  }
  else if(test==8){
    if(flags.freeze){
      flag_alert();
      return;
    }
#ifdef BACKLASH
find_backlash();
return;
#endif

    if(!fl_show_question("\nMove telescope to mirror align position?\n",
			 0))
      return;

    /*Read alignment position from file */
    if( (fp = fopen( "/home/observer/bin/.alignment_position",
		     "r" )) != NULL){
      if( fgets(temp,80,fp) == NULL){  /* Read comment line in file */
	    fprintf(stderr,"%s: error reading %s\n",progname,"direction");
	    fclose(fp);
	    return;
	}
      /*Ignore first line and get second */
      if( fgets(temp,80,fp) == NULL){  /* Read comment line in file */
	    fprintf(stderr,"%s: error reading %s\n",progname,"direction");
	    fclose(fp);
	    return;
	}
      sscanf(temp,"%lf %d %lf %d",&el_info.mirror_alignment_position,
	     &el_info.mirror_alignment_offset,
	     &az_info.mirror_alignment_position,
	     &az_info.mirror_alignment_offset);
    }
    else { /*Choose a default position*/
      el_info.mirror_alignment_position=-.154;
      el_info.mirror_alignment_offset=500;
      az_info.mirror_alignment_position=359.956;
      az_info.mirror_alignment_offset=700;
    }
    strcpy( temp_id, id);
    strcpy( temp_code, id_code);
    strcpy( id, "MIRROR");
    strcpy( id_code, "MIRROR");
    tempel=objel;
    tempaz=objaz;
    flags.drift_mode=1;
    flags.freeze=1;
    strcpy(mode,"HOME");
    /* NOTE mode needs to be changed for HOME */   
    mode_int=STANDBY;
    send_time+=200;  /*  Make program send to vax immediatly */
    send_to_VHEGRO();
    if (telel>.5){   /* telescope not in stow position */
     if (telel>LOW_EL){
       strcpy(current_message[0],"MOVING TELESCOPE HOME FIRST");
       strcpy(current_message[1],"POSITION: AZ=359.99  EL=17.0");
       strcpy(current_message[2],"PRESS 'S' (mouse in main window) TO STOP");
       place_message();

       drift_tel(LOW_EL+1.0,359.99);
     }
     strcpy(current_message[0],"MOVING TELESCOPE HOME FIRST");
     strcpy(current_message[1],"POSITION: AZ=359.99  EL=0.0");
     strcpy(current_message[2],"PRESS 'S' (mouse in main window) TO STOP");
     place_message();
     drift_tel(0.0,359.99); /* enter drift mode and go home */
    }
    if(!flags.more_motor_steps){
      strcpy(current_message[0],"MOVING TO MIRROR ALIGNMENT");
      sprintf(current_message[1],"POSITION: AZ=%7.3f  EL=%7.3f",
	    az_info.mirror_alignment_position,
	    el_info.mirror_alignment_position);
      strcpy(current_message[2],"PRESS 'S' (mouse in main window) TO STOP");
      place_message();
      drift_tel(el_info.mirror_alignment_position-.015,
	      az_info.mirror_alignment_position-.015); /*.022= 2 encoder bits*/

      strcpy(current_message[0],"Stepping forward to exact position");
      strcpy(current_message[1],"Please wait");
      strcpy(current_message[2],"See xterm window for more info");
      place_message();
      update();
      fl_check_only_forms();


     /* start by moving telescope UP until next absolute encoder bit changes */
     el_info.direction=UP;
     el_info.velocity=0.05;
     move_motor(&el_info);

     while (telel<(el_info.mirror_alignment_position)){
       gettel();
       telel = RANGE((telel+ELOFFSET),360.0);
       if (telel>180.0)
        telel-=360.0;
     }
     stop_motors();
     printf("\nEL encoder zeroed at: %f\n",telel);
     get_position(&el_info);  
     el_info.motor_encoder_zero=el_info.motor_position;

     /* Now move telescope CW until next absolute encoder bit changes */
     az_info.direction=CW;
     az_info.velocity=0.05;
     move_motor(&az_info);

     while (telaz<az_info.mirror_alignment_position){
       while(telaz!=telaz_temp){
	 gettel();
	 telaz_temp = RANGE((telaz+AZOFFSET),360.0);
	 gettel();
	 telaz = RANGE((telaz+AZOFFSET),360.0);
       }
     }
     stop_motors();
     printf("AZ encoder zeroed at: %f\n",telaz);
     get_position(&az_info);  
     az_info.motor_encoder_zero=az_info.motor_position;
 
    }
   /* Slowly move telescope UP until at motor encoder position */

   el_info.velocity=0.01;
   move_motor(&el_info);
   while ((abs(el_info.motor_position-el_info.motor_encoder_zero))<
	  el_info.mirror_alignment_offset){
     get_position(&el_info);  
/*
printf("counts= %d\n",abs(el_info.motor_position-el_info.motor_encoder_zero));
*/
    }
   stop_motors();
   gettel();
   telel = RANGE((telel+ELOFFSET),360.0);
   if (telel>180.0)
    telel-=360.0;
   printf("\nEL position at: %7.3f degrees plus %d counts\n",telel,
	 abs(el_info.motor_position-el_info.motor_encoder_zero));

   /* Slowly move telescope CW until at motor encoder position */

   az_info.velocity=0.01;
   move_motor(&az_info);
   while ((abs(az_info.motor_position-az_info.motor_encoder_zero))<
	  az_info.mirror_alignment_offset){
     get_position(&az_info);  
/*
printf("counts= %d\n",abs(az_info.motor_position-az_info.motor_encoder_zero));
*/
    }
   stop_motors();
   gettel();
   telaz = RANGE((telaz+AZOFFSET),360.0);
   printf("\nAZ position at: %7.3f degrees plus %d counts\n",telaz,
	 abs(az_info.motor_position-az_info.motor_encoder_zero));

   flags.more_motor_steps=1;
printf("Done!!\n");

    strcpy( id, temp_id);
    strcpy( id_code, temp_code);
    flags.drift_mode=0;
    flags.freeze=0;
    track=no;
    fl_set_button(fd_track_form->mode,0); /* release button */
    fl_set_object_label(fd_track_form->mode,"STANDBY");
    objel=tempel;
    objaz=tempaz;
    clear_message();
    update();

  }
  else if(test==9){
    /*       if(fl_show_question("\nDo you really want to EXIT the tracking program?\n",                         1))*/
{
#ifdef DEBUGMOTORS
      printf("start of closing commands\n");
#endif
      stoptel();
      az_info.set_point_mode=OFF;
      el_info.set_point_mode=OFF;
      az_info.enable=ENABLE;
      el_info.enable=ENABLE;

      setup_motors(&az_info,1,1); 
      setup_motors(&el_info,1,1); 
      /* close the RS232 port */
      close(SERIAL_PORT);
      close(CIO_1);
      close(CIO_2);

      exit(0);
    }
  }
}

void find_backlash()
{
  /* a routine that is not normally used by the tracking program
     it finds the number of motor counts between absolute encoder bits
     and then finds the backlash in azimuth */
double az_start,el_start;
int az_position,el_position,loop,inloop;


printf("finding backlash...\n");
  /* Do this starting at AZ=0, EL=0 degrees */
     el_info.mirror_alignment_position=0.1;
     el_info.mirror_alignment_offset=0;
     az_info.mirror_alignment_position=0.1;
     az_info.mirror_alignment_offset=0;
    
    strcpy( temp_id, id);
    strcpy( temp_code, id_code);
    strcpy( id, "MIRROR");
    strcpy( id_code, "MIRROR");
    tempel=objel;
    tempaz=objaz;
    flags.drift_mode=1;
    flags.freeze=1;
    strcpy(mode,"HOME");
    /* NOTE mode needs to be changed for HOME */   
    mode_int=STANDBY;
    send_time+=200;  /*  Make program send to vax immediatly */
    send_to_VHEGRO();
    if (telel>.5){   /* telescope not in stow position */
     if (telel>LOW_EL){
       strcpy(current_message[0],"MOVING TELESCOPE HOME FIRST");
       strcpy(current_message[1],"POSITION: AZ=359.99  EL=17.0");
       strcpy(current_message[2],"PRESS 'S' (mouse in main window) TO STOP");
       place_message();

       drift_tel(LOW_EL+1.0,359.99);
     }
     strcpy(current_message[0],"MOVING TELESCOPE HOME FIRST");
     strcpy(current_message[1],"POSITION: AZ=359.99  EL=0.0");
     strcpy(current_message[2],"PRESS 'S' (mouse in main window) TO STOP");
     place_message();
     drift_tel(0.0,359.99); /* enter drift mode and go home */
    }

    strcpy(current_message[0],"MOVING TO ENCODER ALIGNMENT");
    sprintf(current_message[1],"POSITION: AZ=%7.3f  EL=%7.3f",
	    az_info.mirror_alignment_position,
	    el_info.mirror_alignment_position);
    strcpy(current_message[2],"PRESS 'S' (mouse in main window) TO STOP");
     place_message();
    drift_tel(el_info.mirror_alignment_position-.015,
	      az_info.mirror_alignment_position-.015); /*.022= 2 encoder bits*/

    strcpy(current_message[0],"Stepping forward to exact position");
    strcpy(current_message[1],"Please wait");
    strcpy(current_message[2],"See xterm window for more info");
    place_message();
    update();
    fl_check_only_forms();


     /* start by moving telescope UP until next absolute encoder bit changes */
   el_info.direction=UP;
   el_info.velocity=0.05;
   move_motor(&el_info);

   while (telel<(el_info.mirror_alignment_position)){
     gettel();
     telel = RANGE((telel+ELOFFSET),360.0);
     if (telel>180.0)
      telel-=360.0;
    }
   stop_motors();
   printf("\nEL encoder zeroed at: %f\n",telel);
   get_position(&el_info);  
   el_info.motor_encoder_zero=el_info.motor_position;
   el_position=el_info.motor_position;

     /* Now move telescope CW until next absolute encoder bit changes */
   az_info.direction=CW;
   az_info.velocity=0.05;
   move_motor(&az_info);

   while (telaz<(az_info.mirror_alignment_position)){
     gettel();
     telaz = RANGE((telaz+AZOFFSET),360.0);
    }
   stop_motors();
   printf("AZ encoder zeroed at: %f\n",RANGE((telaz+AZOFFSET),360.0));
   get_position(&az_info);  
   az_info.motor_encoder_zero=az_info.motor_position;
   az_position=az_info.motor_position; 

   /* Slowly move telescope UP until there is a bit change in the
      absolute encoder and read motor encoder position */
   el_info.direction=UP;
   el_info.velocity=0.05;
   move_motor(&el_info);
   for(loop=0;loop<5;loop++){
   el_start=telel+.01;
   inloop=0;
   while (telel<el_start){
     gettel();
     telel = RANGE((telel+ELOFFSET),360.0);
     if (telel>180.0)
      telel-=360.0;
     if(inloop++>100000){
       get_position(&el_info);  
       printf("elevation=%f counts=%d\n",telel,
	      abs(el_info.motor_position-el_info.motor_encoder_zero));
       inloop=0;
       update();
     }
    }
    get_position(&el_info);  
printf("elevation= %7.3f counts= %d  difference= %d\n",telel,
       abs(el_info.motor_position-el_info.motor_encoder_zero),
       abs(el_info.motor_position-el_position));
    el_position=el_info.motor_position;
   }
   stop_motors();


   /* Repeat above for azimuth */
   az_info.velocity=0.05;
   move_motor(&az_info);
   for(loop=0;loop<5;loop++){
     gettel();
     telaz = RANGE((telaz+AZOFFSET),360.0);   az_start=telaz+.01;
printf("az_start=%f\n",az_start);
   inloop=0;
     while (telaz<az_start){
     gettel();
     telaz = RANGE((telaz+AZOFFSET),360.0);
     if(inloop++>100000){
       get_position(&az_info);  
       printf("azimuth=%f counts=%d\n",telaz,
	      abs(az_info.motor_position-az_info.motor_encoder_zero));
       inloop=0;
       update();
     }
     }
   get_position(&az_info);  

printf("azimuth= %7.3f   az counts= %d  difference= %d\n",telaz,
       abs(az_info.motor_position-az_info.motor_encoder_zero),
       abs(az_info.motor_position-az_position));
    az_position=az_info.motor_position;
   }
   stop_motors();

printf("Done with motor counts between absolute encoder bits!!\n");


/* Now move az back and forth between absolute encoder bits and 
   measure backlash */

   az_info.direction=CW;
   az_info.velocity=0.05;
   move_motor(&az_info);
   for(loop=0;loop<5;loop++){
   az_start=telaz;
     while (telaz==az_start){
     gettel();
     telaz = RANGE((telaz+AZOFFSET),360.0);
     }
     get_position(&az_info);  

printf("azimuth= %7.3f   az counts= %d  difference=  %d\n",telaz,     
       abs(az_info.motor_position-az_info.motor_encoder_zero),
       abs(az_info.motor_position-az_position));
    az_position=az_info.motor_position;

   az_start=telaz;
     while (telaz==az_start){
     gettel();
     telaz = RANGE((telaz+AZOFFSET),360.0);
     }
   get_position(&az_info);  

printf("azimuth= %7.3f   az counts= %d  difference=  %d\n",telaz,     
       abs(az_info.motor_position-az_info.motor_encoder_zero),
       abs(az_info.motor_position-az_position));
    az_position=az_info.motor_position;

   stop_motors();
   if(az_info.direction==CW){
     az_info.direction=CCW;
     printf("reversing AZ motor to CCW\n");
   }
   else{
     az_info.direction=CW;
     printf("reversing AZ motor to CW\n");
   }
   az_info.velocity=0.05;
   move_motor(&az_info);
   }
   stop_motors();


printf("DONE!!!\n");

    strcpy( id, temp_id);
    strcpy( id_code, temp_code);
    flags.drift_mode=0;
    flags.freeze=0;
    track=no;
    fl_set_button(fd_track_form->mode,0); /* release button */
    fl_set_object_label(fd_track_form->mode,"STANDBY");
    objel=tempel;
    objaz=tempaz;
    clear_message();
    update();



}
void burst_input()
{
 /* Check if burst file has been sent to the tracking program. Notify
    observer if there and set up tracking if requested. 
    if request=0 then check file. if request=1 ...exit burst tracking */

 char temp[5][80];
 char trash[20],arrival_time[80],determine[80],notice[3][80];
 char countline[80];
 FILE *fpburst,*fpnotice;
 int ct=0,counts=-1,burst_number=-1,track_choice=0;
 float ra_c,dec_c,ra_t,dec_t,ra_a,dec_a,ra_b,dec_b,ra_be,dec_be,epoch_b;
 double az_b,el_b;
 double errorrad=6.0;
 enum { IGNORE_BURST=0, SLEW_TO_BURST=1 } recommendation=SLEW_TO_BURST;

   if((fpburst = fopen("/home/observer/tracking_sources/burst/burst","rt")) 
       != NULL){
     if(flags.burst_mode) /* if already looking at a burst */
       burst_number=1;  /* deal with burst #2 */
     else
       burst_number=0; /* deal with the first burst */
    }
   else
     return;  /* return if no new burst */

    for(ct=0;ct<5;ct++){
      if(fgets(temp[ct],80,fpburst) == NULL)
	printf("\nERROR READING INPUT BURST\n\n");
    }
    if(ct == 5){
       sscanf(temp[0],"%s %f %f %f",&trash[0],&ra_c,&dec_c,&epoch_b);
       sscanf(temp[1],"%s %f %f",&trash[0],&ra_t,&dec_t); 
       sscanf(temp[2],"%s %f %f",&trash[0],&ra_a,&dec_a);
       sscanf(temp[3],"%s %f %f",&trash[0],&ra_b,&dec_b); 
       sscanf(temp[4],"%s %f %f",&trash[0],&ra_be,&dec_be); 
       /* create memory */
       if(burst_list[burst_number]==NULL)
         burst_list[burst_number]=
	          (struct source_burst *)malloc(sizeof(struct source_burst));
       burst_list[burst_number]->epoch = (double)epoch_b;
       ftod((double)ra_c, &burst_list[burst_number]->ra_center);
       ftod((double)dec_c, &burst_list[burst_number]->dec_center);
       to2000(burst_list[burst_number]->epoch , 
       	    &burst_list[burst_number]->ra_center,
            &burst_list[burst_number]->dec_center );
       from2000((double) (gettime()), 
      	    &burst_list[burst_number]->ra_center,
	    &burst_list[burst_number]->dec_center );
       ftod((double)ra_a, &burst_list[burst_number]->ra_ahead);
       ftod((double)dec_a, &burst_list[burst_number]->dec_ahead);
       to2000(burst_list[burst_number]->epoch , 
            &burst_list[burst_number]->ra_ahead,
       	    &burst_list[burst_number]->dec_ahead );
       from2000((double) (gettime()),
       	    &burst_list[burst_number]->ra_ahead,
       	    &burst_list[burst_number]->dec_ahead );
       ftod((double)ra_t, &burst_list[burst_number]->ra_top);
       ftod((double)dec_t, &burst_list[burst_number]->dec_top);
       to2000(burst_list[burst_number]->epoch ,
       	    &burst_list[burst_number]->ra_top,
       	    &burst_list[burst_number]->dec_top );
       from2000((double) (gettime()),
       	    &burst_list[burst_number]->ra_top,
       	    &burst_list[burst_number]->dec_top );
       ftod((double)ra_be, &burst_list[burst_number]->ra_behind);
       ftod((double)dec_be, &burst_list[burst_number]->dec_behind);
       to2000(burst_list[burst_number]->epoch ,
       	    &burst_list[burst_number]->ra_behind,
       	    &burst_list[burst_number]->dec_behind );
       from2000((double) (gettime()),
       	    &burst_list[burst_number]->ra_behind,
       	    &burst_list[burst_number]->dec_behind );
       ftod((double)ra_b, &burst_list[burst_number]->ra_bottom);
       ftod((double)dec_b, &burst_list[burst_number]->dec_bottom);
       to2000(burst_list[burst_number]->epoch ,
       	    &burst_list[burst_number]->ra_bottom,
       	    &burst_list[burst_number]->dec_bottom );
       from2000((double) (gettime()),
       	    &burst_list[burst_number]->ra_bottom,
       	    &burst_list[burst_number]->dec_bottom );
    }
    fclose(fpburst);       /* Now CT = #elements */

    if((fpnotice = fopen("/home/observer/tracking_sources/burst/notice","rt"))
       != NULL){
      if(fgets(temp[0],80,fpnotice) != NULL)
	strcpy(arrival_time,temp[0]);
      if(fgets(temp[0],80,fpnotice) != NULL)
	sscanf(temp[0],"%s",determine);
      if(fgets(temp[0],80,fpnotice) != NULL) {
        strcpy(countline,temp[0]);
	sscanf(temp[0],"%d",&counts); }
      if(fgets(temp[0],80,fpnotice) != NULL)
	sscanf(temp[0],"%lf",&errorrad);
      fclose(fpnotice);
    } /* end if fopen(!NULL */
    else{
      printf("error opening /home/observer/tracking_sources/burst/notice \n");
      return;
    }
    /* Remove burst and notice files to allow more notices to come in */ 
    system("rm -f /home/observer/tracking_sources/burst/burst");
    system("rm -f /home/observer/tracking_sources/burst/notice");

    /* check az and el. If below horizon...save and ignore */
    if(!azandel(burst_list[burst_number]->ra_center,
		burst_list[burst_number]->dec_center,&el_b,&az_b,st)){
       /* write a notice file for vhegro to display */
       fpburst = fopen("/home/observer/tracking_sources/burst/new_burst","wt");
       fprintf(fpburst,"A new burst has arrived !!!\n
               It is below the horizon and will\n
               be saved to the burst directory");
       fclose(fpburst);
       /*       system("DISPLAY=taurus:0 /home/observer/bin/taurus_notice &");*/
       /*       system("DISPLAY=draco:0 /home/observer/bin/taurus_notice &");*/
       system("/home/observer/bin/taurus_notice &");
       save_burst(ra_c,dec_c,ra_t,dec_t,ra_a,dec_a,ra_b,
       	         dec_b,ra_be,dec_be,epoch_b,countline,arrival_time,determine);
      return;
    } /* end if !azandel */
  
    /* let operator know about burst and get an answer */
    sprintf(notice[0],"AZ=%-6.2f    Intensity: %d counts",az_b,counts);
    sprintf(notice[1],"EL=%-6.2f    Error rad: %5.2f deg",el_b,errorrad);

    if((counts < BURST_MIN_INTENSITY)||(errorrad > BURST_ERROR_MAX))
      {
	recommendation=IGNORE_BURST;
	sprintf(notice[2],"Do NOT slew to burst");
      }
    else if(errorrad > BURST_ERROR_5SCAN) 
      {
	recommendation=SLEW_TO_BURST;
	sprintf(notice[2],"SLEW to burst and do 5 positions");
      }
    else /* counts >= BURST_MIN_INTENSITY and errorrad < BURST_ERROR_5SCAN */
      {
	recommendation=SLEW_TO_BURST;
	sprintf(notice[2],"SLEW to burst and STAY on centre");
      }

    /* write a notice file for vhegro to display */
    fpburst = fopen("/home/observer/tracking_sources/burst/new_burst","wt");
    fprintf(fpburst,"A new BURST has arrived: %s\n\n",determine);
    fprintf(fpburst,"Arrival time: %s",arrival_time);
    fprintf(fpburst,"%s\n",notice[0]);
    fprintf(fpburst,"%s\n",notice[1]);
    fprintf(fpburst,"Recommendation:\n%s\n",notice[2]);
    
    fclose(fpburst);
    /* system("DISPLAY=taurus:0 /home/observer/bin/taurus_notice &");*/
    /* system("DISPLAY=draco:0 /home/observer/bin/taurus_notice &");*/

    if(recommendation==IGNORE_BURST)
      {
	system("/home/observer/bin/track10_notice 0&");
      }
    else
      {
	system("/home/observer/bin/track10_notice 1&");
      }

    /* wait here until a response is given */
    fpburst=NULL;
    while(fpburst==NULL)
    {
      fpburst=fopen("/home/observer/tracking_sources/burst/notice_reply","rt");
      update();
      fl_check_only_forms();
    }

    if(fgets(temp[0],80,fpburst) != NULL){
	if((atoi(temp[0]))==0){
          save_burst(ra_c,dec_c,ra_t,dec_t,ra_a,dec_a,ra_b,dec_b,
		 ra_be,dec_be,epoch_b,countline,arrival_time,determine);
	  return;
	}
    fclose(fpburst);
    }
    system("rm /home/observer/tracking_sources/burst/notice_reply");
    system("rm /home/observer/tracking_sources/burst/new_burst");

    save_burst(ra_c,dec_c,ra_t,dec_t,ra_a,dec_a,ra_b,dec_b,
	       ra_be,dec_be,epoch_b,countline,arrival_time,determine);


    /* If switching to a new burst..transfer data to first burst structure */
    if(burst_number > 0){
       burst_list[0]->epoch = burst_list[burst_number]->epoch;
       burst_list[0]->ra_center = burst_list[burst_number]->ra_center;
       burst_list[0]->dec_center = burst_list[burst_number]->dec_center;
       burst_list[0]->ra_top = burst_list[burst_number]->ra_top;
       burst_list[0]->dec_top = burst_list[burst_number]->dec_top;
       burst_list[0]->ra_ahead = burst_list[burst_number]->ra_ahead;
       burst_list[0]->dec_ahead = burst_list[burst_number]->dec_ahead;
       burst_list[0]->ra_bottom = burst_list[burst_number]->ra_bottom;
       burst_list[0]->dec_bottom = burst_list[burst_number]->dec_bottom;
       burst_list[0]->ra_behind = burst_list[burst_number]->ra_behind;
       burst_list[0]->dec_behind = burst_list[burst_number]->dec_behind;
    }

    burst_position=CENTER;
    /* save current settings for later replacement */
    temp_ra=ra;
    temp_dec=dec;
    temp_sra=sra;
    temp_sdec=sdec;
    temp_pra=pra;
    temp_pdec=pdec;
    strcpy(temp_id,id);
    strcpy(temp_code,id_code);

    /* Default position is center...set it up */
    strcpy(id_code,"BURST");
    burst_position=CENTER;
    ra=burst_list[0]->ra_center;
    dec=burst_list[0]->dec_center;
    sra  = ra;
    sdec = dec;
    pra = ra;
    pdec = dec;
    ftohms(ra, rastr);
    ftohms(dec, decstr);

    flags.freeze=1;  /*freeze other operations while in burst mode*/

#if 0
    track_choice=fl_show_choice("Choose between the scanning method and",
		     "the five position method (FIVE)",
		     "",2,"SCAN","FIVE","",1);
#else
    track_choice=0;
#endif

    if(track_choice==1){
       scan_setup();
       fl_show_form(fd_scan_form->scan_form,FL_PLACE_CENTERFREE,
		FL_FULLBORDER,"Scan Tracking");
       flags.scan_mode=1;
    }
    else{
       fl_show_form(fd_burst_form->burst_form,FL_PLACE_CENTERFREE,
		FL_FULLBORDER,"Burst Tracking");
       flags.burst_mode=1;

       strcpy(id,"BURST CENTER");
       fl_show_object(fd_burst_form->burst_center);
       fl_set_object_color(fd_burst_form->burst_center,FL_WHITE,FL_WHITE);
       fl_set_object_color(fd_burst_form->burst_1,FL_COL1,FL_COL1);
       fl_set_object_color(fd_burst_form->burst_2,FL_COL1,FL_COL1);
       fl_set_object_color(fd_burst_form->burst_3,FL_COL1,FL_COL1);
       fl_set_object_color(fd_burst_form->burst_4,FL_COL1,FL_COL1);
    } 
} /* end void burst_input( */

void save_burst(double ra_c,double dec_c,double ra_t,double dec_t,
		double ra_a,double dec_a, double ra_b,
		double dec_b,double ra_be,double dec_be,double epoch_b,
		char *countline,char *arrival_time,char *determine)
{
  FILE *fpburst;
  char filename[100],l_string[5][40];
  int i;

  sscanf(arrival_time,"%s %s %s %s %s",&l_string[0][0],&l_string[1][0],
	 &l_string[2][0],&l_string[3][0],&l_string[4][0]);
  for(i=0;i<5;i++)
    l_string[i][strlen(l_string[i])]='\0';
  strcpy(filename,"/home/observer/tracking_sources/burst/");
  strcat(filename,l_string[1]);
  strcat(filename,l_string[2]);
  strcat(filename,l_string[3]);
  strcat(filename,"_");
  strcat(filename,l_string[4]);
  strcat(filename,".burst");
  printf("writing file %s",filename);
  fflush(stdout);
  if((fpburst = fopen(filename,"wt"))
       != NULL){
    fprintf(fpburst,"%s",arrival_time);
    fprintf(fpburst,"%f %f %f\n",ra_c,dec_c,epoch_b);
    fprintf(fpburst,"%f %f\n",ra_t,dec_t);
    fprintf(fpburst,"%f %f\n",ra_a,dec_a);
    fprintf(fpburst,"%f %f\n",ra_b,dec_b);
    fprintf(fpburst,"%f %f\n",ra_be,dec_be);
    fprintf(fpburst,"%s",countline);
    fprintf(fpburst,"%s",determine);
  }
  fclose(fpburst);  
  printf("\ndone.\n");
  fflush(stdout);
}

void burst_exit(FL_OBJECT *obj, long data)
{
printf("burst_exit: Entering the burst mode\n");
     flags.burst_mode=0;
     flags.freeze=0;
     ra=temp_ra;
     dec=temp_dec;
     sra=temp_sra;
     sdec=temp_sdec;
     pra=temp_pra;
     pdec=temp_pdec;
     strcpy(id,temp_id);
     strcpy(id_code,temp_code);
     fl_hide_form(fd_burst_form->burst_form);
printf("burst_exit: Exiting the burst mode\n");
}
  
void burst_move(FL_OBJECT *obj, long data)
{
  /* This routine called from burst tracking to change positions */
switch(data){
 case CENTER:
   burst_position=CENTER;
   ra=burst_list[0]->ra_center;
   sra=ra;
   dec=burst_list[0]->dec_center;
   strcpy(id,"BURST CENTER");
   fl_show_object(fd_burst_form->burst_center);
   fl_set_object_color(fd_burst_form->burst_center,FL_WHITE,FL_WHITE);
   fl_set_object_color(fd_burst_form->burst_1,FL_COL1,FL_COL1);
   fl_set_object_color(fd_burst_form->burst_2,FL_COL1,FL_COL1);
   fl_set_object_color(fd_burst_form->burst_3,FL_COL1,FL_COL1);
   fl_set_object_color(fd_burst_form->burst_4,FL_COL1,FL_COL1);
   break;
 case TOP:
   ra=burst_list[0]->ra_top;
   sra=ra;
   dec=burst_list[0]->dec_top;
   strcpy(id,"POSITION 1");
   burst_position=TOP;
   fl_set_object_color(fd_burst_form->burst_1,FL_WHITE,FL_WHITE);
   fl_set_object_color(fd_burst_form->burst_2,FL_COL1,FL_COL1);
   fl_set_object_color(fd_burst_form->burst_3,FL_COL1,FL_COL1);
   fl_set_object_color(fd_burst_form->burst_4,FL_COL1,FL_COL1);
   fl_set_object_color(fd_burst_form->burst_center,FL_COL1,FL_COL1);
   break;
 case AHEAD:
   ra=burst_list[0]->ra_ahead;
   sra=ra;
   dec=burst_list[0]->dec_ahead;
   strcpy(id,"POSITION 2");
   burst_position=AHEAD;
   fl_set_object_color(fd_burst_form->burst_2,FL_WHITE,FL_WHITE);
   fl_set_object_color(fd_burst_form->burst_3,FL_COL1,FL_COL1);
   fl_set_object_color(fd_burst_form->burst_4,FL_COL1,FL_COL1);
   fl_set_object_color(fd_burst_form->burst_center,FL_COL1,FL_COL1);
   fl_set_object_color(fd_burst_form->burst_1,FL_COL1,FL_COL1);
   break;
 case BOTTOM:
   ra=burst_list[0]->ra_bottom;
   sra=ra;
   dec=burst_list[0]->dec_bottom;
   strcpy(id,"POSITION 3");
   burst_position=BOTTOM;
   fl_set_object_color(fd_burst_form->burst_3,FL_WHITE,FL_WHITE);
   fl_set_object_color(fd_burst_form->burst_4,FL_COL1,FL_COL1);
   fl_set_object_color(fd_burst_form->burst_center,FL_COL1,FL_COL1);
   fl_set_object_color(fd_burst_form->burst_1,FL_COL1,FL_COL1);
   fl_set_object_color(fd_burst_form->burst_2,FL_COL1,FL_COL1);
   break;
 case BEHIND:
   ra=burst_list[0]->ra_behind;
   sra=ra;
   dec=burst_list[0]->dec_behind;
   strcpy(id,"POSITION 4");
   burst_position=BEHIND;
   fl_set_object_color(fd_burst_form->burst_4,FL_WHITE,FL_WHITE);
   fl_set_object_color(fd_burst_form->burst_center,FL_COL1,FL_COL1);
   fl_set_object_color(fd_burst_form->burst_1,FL_COL1,FL_COL1);
   fl_set_object_color(fd_burst_form->burst_2,FL_COL1,FL_COL1);
   fl_set_object_color(fd_burst_form->burst_3,FL_COL1,FL_COL1);
   break;
 default:
   printf("NO CHANGE\n");
 }
  ftohms(ra, rastr);
  ftohms(dec, decstr);

}

void menu_setup(void)
{
  fl_setpup_fontsize(FL_MEDIUM_SIZE);
  fl_setpup_fontstyle(2);
  /*printf("menustr: %s\n",menustr);*/
  fl_set_menu(fd_track_form->commands,menustr);
  /* next command used to turn off a menu button */
/*  fl_set_menu_item_mode(fd_track_form->commands,7,FL_PUP_GRAY);*/
}
void change_time(FL_OBJECT *obj, long data)
{
printf("change time not installed\n");
}
void change_date(FL_OBJECT *obj, long data)
{
printf("change date not installed\n");
}

/* Pointing correction routines */
void star_corrections(void)
{
  int count=0;

    /* set the current corrections to zero to make things easier */
  cos_alpha=1.;
  sin_alpha=0.;
  cos_beta =1.;
  sin_beta =0.;
  cos_gamma=1.;
  tan_gamma=0.;
  delta=0.;

  if(flags.freeze){
    flag_alert();
    return;
  }

  flags.doing_corrections=1;
  flags.freeze=1;

  /* Set up correction form */
  fl_show_form(fd_correct_form->correct_form,FL_PLACE_CENTER,FL_FULLBORDER,
                "POINTING CORRECTIONS");
  fl_set_counter_value(fd_correct_form->az_slider,0.0); 
  fl_set_counter_value(fd_correct_form->el_slider,0.0); 
  fl_set_counter_bounds(fd_correct_form->az_slider,-6.0,6.0);
/*		      (int)SLIDER_RANGE_MIN/(cos(telel/RTOD)),
		      (int)SLIDER_RANGE_MAX/(cos(telel/RTOD))); */
  fl_set_counter_bounds(fd_correct_form->el_slider,SLIDER_RANGE_MIN,
		       SLIDER_RANGE_MAX); 
  fl_set_counter_value(fd_correct_form->X_slider,0.0); 
  fl_set_counter_bounds(fd_correct_form->X_slider,-2.0,2.0);
  fl_set_counter_step(fd_correct_form->X_slider,0.001,.01); 
  fl_set_counter_precision(fd_correct_form->X_slider,3); 

  fl_set_counter_value(fd_correct_form->Z_slider,0.0); 
  fl_set_counter_bounds(fd_correct_form->Z_slider,-2.0,2.0);
  fl_set_counter_step(fd_correct_form->Z_slider,0.001,.01); 
  fl_set_counter_precision(fd_correct_form->Z_slider,3); 

      /* set slider step values and precision */
  fl_set_counter_step(fd_correct_form->az_slider,0.001,.01); 
  fl_set_counter_step(fd_correct_form->el_slider,0.001,.01); 
  fl_set_counter_precision(fd_correct_form->az_slider,3); 
  fl_set_counter_precision(fd_correct_form->el_slider,3); 

  total_correction_entries=0; /*reset counter before starting input */

        /* cycle in this loop until the corrections are done */

    while(!corrections_done) {
     update();
     fl_check_only_forms();
    }

    corrections_done=0;
    fl_hide_form(fd_correct_form->correct_form);
    total_correction_entries=0;

    flags.doing_corrections=0;  /* no more offsets to telescope position */
    flags.freeze=0;

    count=0;
      /*prepare for another run through in correction routine*/
    /*    strcpy(correction_list[total_correction_entries]->name, "\0");*/
}/* end  void star_corrections(void) */

/* callbacks for form correct_form */
void correction_exit(FL_OBJECT *ob, long data)
{
  /*Exit from correction routine*/
  corrections_done=1;
  return;
}
void az_adjust(FL_OBJECT *ob, long data)
{
  az_cor_offset=fl_get_counter_value(fd_correct_form->az_slider);

}

void el_adjust(FL_OBJECT *ob, long data)
{
  el_cor_offset=fl_get_counter_value(fd_correct_form->el_slider);

}
void Z_adjust(FL_OBJECT *ob, long data)
{
  char tube_string[5];
  float dx,dz,dist=0.0,small=100.0;
  int i,pmt_now=pmt_to_track;
  pmt aux;

  /*fr VVV correction routine*/
  fb_Z=fl_get_counter_value(fd_correct_form->Z_slider);
  fb_X=fl_get_counter_value(fd_correct_form->X_slider);
  for(i=0;i<NPMT;i++){
      aux=pmt_data(i);
      dx=(fb_X-aux.x) * (fb_X-aux.x);
      dz=(fb_Z-aux.z) * (fb_Z-aux.z);
      dist=sqrt(dx+dz);
      if(dist<small){
        small=dist;
        pmt_now=i+1;
      }
  }
  pmt_to_track=0;
  sprintf(tube_string,"%d",pmt_now);
  fl_set_object_label(fd_correct_form->pmt_tube,tube_string);
}
void X_adjust(FL_OBJECT *ob, long data)
{
  char tube_string[5];
  float dx,dz,dist=0.0,small=100.0;
  int i,pmt_now=pmt_to_track;
  pmt aux;

  /*fr VVV correction routine*/
  fb_X=fl_get_counter_value(fd_correct_form->X_slider);
  fb_Z=fl_get_counter_value(fd_correct_form->Z_slider);
  for(i=0;i<NPMT;i++){
      aux=pmt_data(i);
      dx=(fb_X-aux.x) * (fb_X-aux.x);
      dz=(fb_Z-aux.z) * (fb_Z-aux.z);
      dist=sqrt(dx+dz);
      if(dist<small){
        small=dist;
        pmt_now=i+1;
      }
  }
  pmt_to_track=0;
  sprintf(tube_string,"%d",pmt_now);
  fl_set_object_label(fd_correct_form->pmt_tube,tube_string);
}
void new_tube(FL_OBJECT *ob, long data)
{
  char tube_string[5];
  const char *input;
  pmt aux;

  input=fl_show_input("Enter the PMT to center on","1");
  pmt_to_track=atoi(input);

  if((pmt_to_track<0) || (pmt_to_track>NPMT))
    pmt_to_track=1;
 
  sprintf(tube_string,"%d",pmt_to_track);
  fl_set_object_label(fd_correct_form->pmt_tube,tube_string);
  fb_X=0;
  fb_Z=0;
  aux=pmt_data(pmt_to_track);
  fl_set_counter_value(fd_correct_form->X_slider,aux.x); 
  fl_set_counter_value(fd_correct_form->Z_slider,aux.z); 

} 
void write_offset(FL_OBJECT *ob, long data)
{
  char cor_num_string[5];

    
  if(correction_list[total_correction_entries]==NULL)
    correction_list[total_correction_entries]=(struct star_corrections *)
	            malloc(sizeof(struct star_corrections ));
  strcpy(correction_list[total_correction_entries]->name, id);
  correction_list[total_correction_entries]->ra=ra;
  correction_list[total_correction_entries]->dec=dec;
  correction_list[total_correction_entries]->az=RANGE(objaz,360.);
  correction_list[total_correction_entries]->el=objel;
  correction_list[total_correction_entries]->offset_az=az_cor_offset;
  correction_list[total_correction_entries]->offset_el=el_cor_offset;
  correction_list[total_correction_entries]->siderial=st;
  total_correction_entries++;
  sprintf(cor_num_string,"%d",total_correction_entries);
  fl_set_object_label(fd_correct_form->correction_number,cor_num_string);

  if( (fpkh = fopen( "/home/observer/kh/star_track", "a" )) != NULL){
  printf("%8.3f %8.3f %8.3f %8.3f %8.3f %d %8.3f %8.3f %8.3f %8.3f\n",
	 ra,dec,st,objel,RANGE(objaz,360.),pmt_to_track,el_deg_error,
	 az_deg_error,el_cor_offset,az_cor_offset);
  fprintf(fpkh,"%8.3f %8.3f %8.3f %8.3f %8.3f %d %8.3f %8.3f %8.3f %8.3f\n",
	 ra,dec,st,objel,RANGE(objaz,360.),pmt_to_track,el_deg_error,
	 az_deg_error,el_cor_offset,az_cor_offset);

	  }
   else printf(" unable to open /home/observer/kh/star_track\n ");
   fclose(fpkh);



} /* end void write_offset(FL_ */

void bad_entry(FL_OBJECT *ob, long data)
{

} /*end void bad_entry(FL_OBJEC */
/*-------------------------------------------------------------------------*/
/* MOON tracking subroutines start here */

void move_moon(FL_OBJECT *ob, long data)
{

switch(data){
 case OFFSET:{
   if(off_moon){
     off_moon=0;
     fl_set_object_label(ob,"ON-MOON");
     break;
   }
   else{
     off_moon=1;
     flags.moon_switch=1; /*indicate a change has been made */
     fl_set_object_label(ob,"OFF-MOON");
   }
   break;
 }
 case CENTER:
   if(fl_show_question("Do you REALLY want point at the center of the moon?\n
		       \n\n",0)){
     moon_position=CENTER;
     fl_show_object(fd_moon_form->center_moon);
     fl_set_object_color(fd_moon_form->right_moon,FL_COL1,FL_COL1);
     fl_set_object_color(fd_moon_form->top_moon,FL_COL1,FL_COL1);
     fl_set_object_color(fd_moon_form->left_moon,FL_COL1,FL_COL1);
     fl_set_object_color(fd_moon_form->bottom_moon,FL_COL1,FL_COL1);

   }
   break;
 case TOP:
   switch(moon_position){
   case PROTON:
     moon_rotate(CCW);
     break;
   case BOTTOM:
     moon_rotate(CW);
     moon_rotate(CW);
     break;
   case ANTI_PROTON:
     moon_rotate(CW);
     break;
   default: /* default current position is CENTER*/
     fl_hide_object(fd_moon_form->center_moon);
     break;
   }
   moon_position=TOP;
   fl_set_object_color(fd_moon_form->top_moon,FL_WHITE,FL_WHITE);
   fl_set_object_color(fd_moon_form->right_moon,FL_COL1,FL_COL1);
   fl_set_object_color(fd_moon_form->left_moon,FL_COL1,FL_COL1);
   fl_set_object_color(fd_moon_form->bottom_moon,FL_COL1,FL_COL1);
   break;
 case PROTON:
   switch(moon_position){
   case BOTTOM:
     moon_rotate(CCW);
     break;
   case ANTI_PROTON:
     moon_rotate(CW);
     moon_rotate(CW);
     break;
   case TOP:
     moon_rotate(CW);
     break;
   default:
     fl_hide_object(fd_moon_form->center_moon);
     break;
   }
   moon_position=PROTON;
   fl_set_object_color(fd_moon_form->right_moon,FL_WHITE,FL_WHITE);
   fl_set_object_color(fd_moon_form->top_moon,FL_COL1,FL_COL1);
   fl_set_object_color(fd_moon_form->left_moon,FL_COL1,FL_COL1);
   fl_set_object_color(fd_moon_form->bottom_moon,FL_COL1,FL_COL1);
   break;
 case BOTTOM:
   switch(moon_position){
   case ANTI_PROTON:
     moon_rotate(CCW);
     break;
   case TOP:
     moon_rotate(CW);
     moon_rotate(CW);
     break;
   case PROTON:
     moon_rotate(CW);
     break;
   default:
     fl_hide_object(fd_moon_form->center_moon);
     break;
   }
   moon_position=BOTTOM;
   fl_set_object_color(fd_moon_form->bottom_moon,FL_WHITE,FL_WHITE);
   fl_set_object_color(fd_moon_form->right_moon,FL_COL1,FL_COL1);
   fl_set_object_color(fd_moon_form->left_moon,FL_COL1,FL_COL1);
   fl_set_object_color(fd_moon_form->top_moon,FL_COL1,FL_COL1);
   break;
 case ANTI_PROTON:
   switch(moon_position){
   case TOP:
     moon_rotate(CCW);
     break;
   case PROTON:
     moon_rotate(CW);
     moon_rotate(CW);
     break;
   case BOTTOM:
     moon_rotate(CW);
     break;
   default:
     fl_hide_object(fd_moon_form->center_moon);
     break;
   }
   moon_position=ANTI_PROTON;
   fl_set_object_color(fd_moon_form->left_moon,FL_WHITE,FL_WHITE);
   fl_set_object_color(fd_moon_form->right_moon,FL_COL1,FL_COL1);
   fl_set_object_color(fd_moon_form->top_moon,FL_COL1,FL_COL1);
   fl_set_object_color(fd_moon_form->bottom_moon,FL_COL1,FL_COL1);
   break;
 default:
   printf("NO CHANGE\n");
 }
 moon_time=update_moon(moon_position);
}
void moon_scan_setup(void)
{
  char string[2][60];
  const char *input;
  int test=0,ct=0,total_steps=0;
  float path_length=0.0,degrees_per_step=0.0;
  double radius=0.0;
 
  sprintf(string[0],"scan_time=%7.1f seconds ,radius=%4.1f mrads,",
	  MOON_TIMER,MOON_SCAN_RADIUS);

  test=fl_show_question("Do you want use the defaults:\nstring[0]\n",0);

  if(test==0){
    sprintf(string[0],"%7.2f  ",MOON_SCAN_RADIUS);
    input=fl_show_input("Enter the radius of the scan (mrads)",string[0]);
    scan_data.radius=atof(input);
    sprintf(string[0],"%7.2f  ",MOON_TIMER);
    input=fl_show_input("Enter the scan completion time (seconds)",string[0]);
    scan_data.total_time=atof(input);
  }
  else{
    scan_data.radius=MOON_SCAN_RADIUS;
    scan_data.total_time=MOON_TIMER;
  }
  radius=(scan_data.radius/1000)*RTOD;  /* convert from mrads to degrees */
  /* set up data for x-y plot */

  fl_set_xyplot_xbounds(fd_scan_form->xyplot,(double)(-1.1*radius),
			(double)(1.1*radius));	
  fl_set_xyplot_ybounds(fd_scan_form->xyplot,(double)(-1.1*radius),
			(double)(1.1*radius));
  fl_set_xyplot_xtics(fd_scan_form->xyplot,9,0);
  fl_set_xyplot_ytics(fd_scan_form->xyplot,9,0);
	
  /* setup circle overlay start with unit circle and expand to input radius*/
  for(ct=0;ct<=100;ct++){
    circle_x[ct]=cos(ct*.0628)*.5;  /* .5 degrees for moon radius */
    circle_y[ct]=sin(ct*.0628)*.5;
  } 
  for(ct=0;ct<5000;ct++){    
    plot_x[ct]=0.0;
    plot_y[ct]=0.0;
  }
  /* put data in xy plot */
  fl_set_xyplot_data(fd_scan_form->xyplot,plot_x,plot_y,
		     (scan_data.total_time*STEPS_PER_SECOND)+20,"","","");
  fl_add_xyplot_overlay(fd_scan_form->xyplot,1,circle_x,circle_y,101,FL_RED);

 
  /* calculate full path length of scan (in degrees)*/
  path_length=2*M_PI*radius;

  /* now create the timed path to follow and to use as a drawing file */
  degrees_per_step=path_length/(scan_data.total_time*STEPS_PER_SECOND);
  total_steps=(int)(path_length/degrees_per_step);
#ifdef DEBUGSCAN
  printf("path length is : %f\n",path_length);
  printf("degrees_per_step : %f\n",degrees_per_step);
  printf("total steps : %d\n",total_steps);
#endif 
  for(ct=0;ct<=total_steps;ct++){
    plot_x[ct]=cos(ct*6.28/total_steps)*radius;
    plot_y[ct]=sin(ct*6.28/total_steps)*radius;
  } 


  scan_data.total_steps=total_steps;

#ifdef DEBUGSCAN
    printf("count      x          y    steps=%d\n",total_steps);
  for(ct=0;ct<=total_steps;ct++)
    printf("%d       %f         %f\n",ct,plot_x[ct],plot_y[ct]);
  fl_set_xyplot_data(fd_scan_form->xyplot,plot_x,plot_y,total_steps,
		     "","","");
#endif 

  /* wait here until telescope is at the starting point of the scan */
  if(!track){
    fl_set_object_label(fd_track_form->mode,"TRACKING");
    track = yes; /* VARIABLE THAT TELLS THE MAIN LOOP TO */
	         /* TRACK THE CURRENT SOURCE */
    slewsta = on; /* INDICATES THAT THE TELESCOPE IS ON SOURCE */
    fl_set_button(fd_track_form->mode,1); /* release button */
    fl_set_button(fd_track_form->onsource,1); /* release button */
    fl_set_button(fd_track_form->offsource,0); /* release button */
    datain();
  }
  fl_deactivate_object(fd_track_form->onsource);
  fl_deactivate_object(fd_track_form->offsource);
  fl_deactivate_object(fd_track_form->pointcheck);

  while(flags.scan_mode && !flags.on_source){
     update();
     fl_check_only_forms();
  }
  fl_activate_object(fd_track_form->offsource);
  fl_activate_object(fd_track_form->pointcheck);
  scan_data.odd=1; 
}

void exit_moon(FL_OBJECT *ob, long data)
{
  flags.track_moon=0;
  strcpy(id_code,"****");
  strcpy(id,"******");

  fl_hide_form(fd_moon_form->moon_form);
}

float update_moon(int location) /* updates RA and DEC of moon for current time */
{
  int count,found=0;

/*printf("\nmoon_update  ut=%4.2f  ",ut);*/
  for(count=0;count<100;count++)
    if(moon_list[count] != NULL){
      if(moon_list[count]->moon_time <= ut){
         found=count;
       }
      else
	count=101;
    }
/*printf("\nmoon_time=%f\n",moon_list[found]->moon_time);*/
  if(count!=101)
  switch(location){
  case CENTER:
    ra=moon_list[found]->ra_center;
    sra=ra;
    dec=moon_list[found]->dec_center;
    sdec=dec;
    strcpy(id,"CENTER");
    break;
  case TOP:
    ra=moon_list[found]->ra_top;
    sra=ra;
    dec=moon_list[found]->dec_top;
    sdec=dec;
    strcpy(id,"TOP");
    break;
  case BOTTOM:
    ra=moon_list[found]->ra_bottom;
    sra=ra;
    dec=moon_list[found]->dec_bottom;
    strcpy(id,"BOTTOM");
    break;
  case PROTON:
    ra=moon_list[found]->ra_proton;
    sra=ra;
    dec=moon_list[found]->dec_proton;
    strcpy(id,"PROTON");
    break;
  case ANTI_PROTON:
    ra=moon_list[found]->ra_anti;
    sra=ra;
    dec=moon_list[found]->dec_anti;
    strcpy(id,"ANTI-PRO");
    break;
  default:
    ra=0.0;
    sra=0.0;
    dec=0.0;
    break;
  }
  else /*count ==101 ... no valid moon position*/
    printf("Cannot find a matching time in the moon data file!\n");
/*printf("time=%4.2f ra=%4.2f dec=%4.2f\n",moon_time,ra,dec);*/

  ftohms(ra, rastr);
  ftohms(dec, decstr);

  if(moon_list[found+1] != NULL && count != 101)
    return(moon_list[found+1]->moon_time);
  else
    return(0.);
}
void moon_rotate(int direction) /*move moon to a different spot...avoid PMT
				  currents getting too high */
{
if(direction==CW)
 printf("rotate CW\n");
else
 printf("rotate CCW\n");
}

int get_moon_file()  /* read in file for moon positions */
{
  /*  const char *filename;*/
    char temp[80];
    int  ct=0;
    float time_in;
    float ra_m,dec_m,ra_p,dec_p,ra_t,dec_t,ra_a,dec_a,ra_b,dec_b;
    FILE *fpgo;


    printf("moonpos 19%02i  %02i  %02i  dist=38 mrads\n",local_year, 
	   local_month,local_day);
    printf("Wait while moon positions are calculated.\n");
    if((fpgo = fopen( "/home/observer/moonpos/moon_input","w")) != NULL)
      fprintf(fpgo,"19%02i\n%02i\n%02i\n38\n",local_year, local_month,
	      local_day );
    fclose(fpgo);

    system("/home/observer/moonpos/moonpos");

/*    filename=fl_show_fselector("Choose a file...(Press enter for default)",
			       "/home/observer/tracking_sources","*.dat*",
			     "moon.dat");*/

    if((fpgo = fopen("/home/observer/moonpos/moon.dat","rt")) != NULL){
      while(fgets(temp,80,fpgo) != NULL && ct < 100){
	if(sscanf(temp,"%f %f %f %f %f %f %f %f %f %f %f", 
	       &time_in,&ra_m,&dec_m,&ra_p,&dec_p,&ra_b,&dec_b,&ra_a,&dec_a,
	       &ra_t,&dec_t) == 11){
           /* create more memory only if needed */
           if(moon_list[ct]==NULL)
             moon_list[ct]=
	              (struct source_moon *)malloc(sizeof(struct source_moon));
           moon_list[ct]->moon_time = time_in;
           moon_list[ct]->ra_center = (double)ra_m;
           moon_list[ct]->dec_center = (double)dec_m;
           moon_list[ct]->ra_proton = (double)ra_p;
           moon_list[ct]->dec_proton = (double)dec_p;
           moon_list[ct]->ra_top = (double)ra_t;
           moon_list[ct]->dec_top = (double)dec_t;
           moon_list[ct]->ra_anti = (double)ra_a;
           moon_list[ct]->dec_anti = (double)dec_a;
           moon_list[ct]->ra_bottom = (double)ra_b;
           moon_list[ct]->dec_bottom = (double)dec_b;
	   ct++;
	}
      }
      fclose(fpgo);       /* Now CT = #elements */
    }
    else{
      printf("Can not find or open /home/observer/moonpos/moon.dat\n");
      return(0);
    }
  return(1);
  }
void initialize_flags()
{
  flags.freeze=0;       /* indictor that another routine is running */
  flags.track_moon=0;   /* indicator of moon tracking routine */
  flags.moon_switch=0;  /* indicator of ON-OFF moon switch */
  flags.pointing_check=0;   /*variable to interrupt pointing check*/
  flags.drift_mode=0;   /*variable to interrupt drift and home modes */
  flags.drift_flag=0; /*flag to indicate object position does not need to
			be updated. */
  flags.on_source=0;
  flags.burst_mode=0;
  flags.scan_mode=0;
  flags.scan_mode_run=0;
  flags.doing_corrections=0;
  flags.more_motor_steps=0;
}
void flag_alert()
{
 if(flags.drift_mode)
   {
     fl_show_alert("","Exit drift (home) mode first","",0);
     return;
   }
 if(flags.pointing_check)
   {
     fl_show_alert("","Exit pointing check (p) first","",0);
     return;
   }
 if(flags.burst_mode)
   {
     fl_show_alert("","Exit burst tracking first","",0);
     return;
   }
 if(flags.scan_mode)
   {
     fl_show_alert("","Exit scan mode tracking first","",0);
     return;
   }
 if(flags.track_moon)
   {
     fl_show_alert("","Exit moon tracking first","",0);
     return;
   }
 if(flags.doing_corrections)
   {
     fl_show_alert("","Exit correction routine first","",0);
     return;
   }
 
}
void scale_form(double scale)
{
  fl_scale_form(fd_track_form->track_form,scale,scale);
}
void scan_setup(void)
{
  char string[2][100],temp_string[60];
  const char *input;
  int test=0,ct=0,ct2=0,total_steps=0;
  float path_length,degrees_per_second;

  sprintf(string[0],"Do you want use the defaults:\n");
  sprintf(temp_string,"scan_time=%7.1f seconds ,radius=%4.1f degrees,\n",
	  TIMER,SCAN_RADIUS);
  strcat(string[0],temp_string);
  sprintf(temp_string,"scan_width=%4.1f degree\n",SCAN_WIDTH);
  strcat(string[0],temp_string);
  sprintf(temp_string,"burst_multiplier=%d\n",burst_multiplier);
  strcat(string[0],temp_string);
  test=fl_show_question(string[0],0);

  if(test==0){
    sprintf(string[0],"%7.2f  ",SCAN_RADIUS);
    input=fl_show_input("Enter the radius of the scan",string[0]);
    scan_data.radius=atof(input);
    sprintf(string[0],"%7.2f  ",SCAN_WIDTH);
    input=fl_show_input("Enter the width steps for scanning",string[0]);
    scan_data.scan_width=atof(input);
    sprintf(string[0],"%7.2f  ",TIMER);
    input=fl_show_input("Enter the scan completion time (seconds)",string[0]);
    scan_data.total_time=atof(input);
    sprintf(string[0],"%d  ",burst_multiplier);
    input=fl_show_input("Enter the burst_multiplier",string[0]);
    burst_multiplier=atoi(input);
  }
  else{
    scan_data.radius=SCAN_RADIUS;
    scan_data.scan_width=SCAN_WIDTH;
    scan_data.total_time=TIMER;
  }

  /* set up data for x-y plot */

  fl_set_xyplot_xbounds(fd_scan_form->xyplot,(double)(-1.1*scan_data.radius),
			(double)(1.1*scan_data.radius));	
  fl_set_xyplot_ybounds(fd_scan_form->xyplot,(double)(-1.1*scan_data.radius),
			(double)(1.1*scan_data.radius));
  fl_set_xyplot_xtics(fd_scan_form->xyplot,9,0);
  fl_set_xyplot_ytics(fd_scan_form->xyplot,9,0);
	
  /* setup circle overlay start with unit circle and expand to input radius*/
  for(ct=0;ct<=100;ct++){
    circle_x[ct]=cos(ct*.0628)*scan_data.radius;
    circle_y[ct]=sin(ct*.0628)*scan_data.radius;
  } 
  for(ct=0;ct<5000;ct++){    
    plot_x[ct]=scan_data.radius;
    plot_y[ct]=0.0;
  }
  /* put data in xy plot */
  fl_set_xyplot_data(fd_scan_form->xyplot,plot_x,plot_y,
		     (scan_data.total_time*STEPS_PER_SECOND*2)+20,"","","");
  fl_add_xyplot_overlay(fd_scan_form->xyplot,1,circle_x,circle_y,101,FL_RED);

 
  /* now calculate the data needed */
  scan_data.passes=(int)(((2*scan_data.radius)/scan_data.scan_width)-1);
  if((scan_data.passes%2)==0)
    scan_data.passes+=1;      /* if not an odd number of passes...add 1*/
  for(ct=(scan_data.passes-3)/2;ct>=0;ct--){
     scan_data.corners[((scan_data.passes-1)/2)-ct]=pow(-1,(ct+1))*
            sqrt(pow((scan_data.radius-scan_data.scan_width),2)-
	    pow((ct*scan_data.scan_width),2));
     scan_data.corners[scan_data.passes-((scan_data.passes-1)/2)+ct]=-1*
       scan_data.corners[((scan_data.passes-1)/2)-ct];
  }
  scan_data.corners[0]=-1*scan_data.corners[1];
  scan_data.corners[scan_data.passes]=-1*scan_data.corners[scan_data.passes-1];

  /* now calculate full path length of scan */
  path_length=0;
  for(ct=0;ct<scan_data.passes+1;ct++)
    path_length+=2*fabs(scan_data.corners[ct]);    /* start with DEC segments*/
  path_length-=2*fabs(scan_data.corners[0]);       /*subtract extra at ends*/
  path_length+=(scan_data.passes-1)*scan_data.scan_width; /* add RA lengths */
#ifdef DEBUGSCAN
  printf("path length is : %f\n",path_length);
  for(ct=0;ct<(scan_data.passes+1);ct++)
    printf("x[%d]=%f\n",ct,scan_data.corners[ct]);
#endif 
  /* now create the timed path to follow and to use as a drawing file */
  degrees_per_second=path_length/(scan_data.total_time*STEPS_PER_SECOND);
  total_steps=0;
  for(ct=0;ct<=((scan_data.passes-3)/2);ct++){
    ct2=0;
    for(ct2=0;ct2<=((fabs(scan_data.corners[ct]-scan_data.corners[ct+1])-
		     degrees_per_second)/degrees_per_second);ct2++){
        plot_x[total_steps]=scan_data.scan_width*
	  (-1*(((scan_data.passes-1)/2)-ct));
        if((((scan_data.passes-1)/2)%2)==0){
          if((ct%2)==0)  /* if even (path is increasing) */
            plot_y[total_steps]=scan_data.corners[ct]+(degrees_per_second*ct2);
          else
            plot_y[total_steps]=scan_data.corners[ct]-(degrees_per_second*ct2);
	}
	else{
          if((ct%2)==0)  /* if even (path is increasing) */
            plot_y[total_steps]=scan_data.corners[ct]-(degrees_per_second*ct2);
          else
            plot_y[total_steps]=scan_data.corners[ct]+(degrees_per_second*ct2);
	}
        total_steps++;
      }
    for(ct2=0;ct2<=((scan_data.scan_width-degrees_per_second)/
      	    degrees_per_second);ct2++){
       plot_x[total_steps]=(scan_data.scan_width*
	  (-1*(((scan_data.passes-1)/2)-ct))+(degrees_per_second*ct2));
       plot_y[total_steps]=scan_data.corners[ct+1];
       total_steps++;
    }
  }
  for(ct2=0;ct2<=(fabs(scan_data.corners[ct])/degrees_per_second);ct2++){
     plot_x[total_steps]=scan_data.scan_width*
	  (-1*(((scan_data.passes-1)/2)-ct));
     plot_y[total_steps]=scan_data.corners[ct]+(degrees_per_second*ct2);
     total_steps++;
  }
  plot_x[total_steps]=0.0;
  plot_y[total_steps]=0.0;

  for(ct2=1;ct2<=total_steps;ct2++){
     plot_x[total_steps+ct2]= -1*plot_x[total_steps-ct2];
     plot_y[total_steps+ct2]= -1*plot_y[total_steps-ct2];
  }
  scan_data.total_steps=total_steps+ct2-1;
  /* now create the return path */
  for(ct2=scan_data.total_steps;ct2>=0;ct2--){
     plot_x[(2*scan_data.total_steps+1)-ct2] = plot_x[ct2];
     plot_y[(2*scan_data.total_steps+1)-ct2]=plot_y[ct2];
  }
  scan_data.total_steps=2*scan_data.total_steps+1;

#ifdef DEBUGSCAN
    printf("count      x          y    steps=%d\n",total_steps);
  for(ct=0;ct<=(total_steps+ct2);ct++)
    printf("%d       %f         %f\n",ct,plot_x[ct],plot_y[ct]);
  /*  fl_set_xyplot_data(fd_scan_form->xyplot,plot_x,plot_y,(total_steps+ct2),
		     "","","");*/
#endif 
  /* wait here until telescope is at the starting point of the scan */
   ra=burst_list[0]->ra_center-(plot_x[0]/15);
   dec=burst_list[0]->dec_center+plot_y[0];
   sra  = ra;
   sdec = dec;
   pra = ra;
   pdec = dec;
   ftohms(ra, rastr);
   ftohms(dec, decstr);

  if(!track){
    fl_set_object_label(fd_track_form->mode,"TRACKING");
    track = yes; /* VARIABLE THAT TELLS THE MAIN LOOP TO */
	         /* TRACK THE CURRENT SOURCE */
    slewsta = on; /* INDICATES THAT THE TELESCOPE IS ON SOURCE */
    fl_set_button(fd_track_form->mode,1); /* release button */
    fl_set_button(fd_track_form->onsource,1); /* release button */
    fl_set_button(fd_track_form->offsource,0); /* release button */
    datain();
  }
  fl_deactivate_object(fd_track_form->onsource);
  fl_deactivate_object(fd_track_form->offsource);
  fl_deactivate_object(fd_track_form->pointcheck);

  while(flags.scan_mode && !flags.on_source){
     update();
     fl_check_only_forms();
  }
  fl_activate_object(fd_track_form->offsource);
  fl_activate_object(fd_track_form->pointcheck);

}
void continue_scan(void)
{
  double ct=0,elapsed_time=0;

  ct=fl_get_timer(fd_scan_form->timer);
  if(ct<scan_data.elapsed_time){
#ifdef DEBUGSCAN
    printf("time=%f elapsed time= %f    int=%d    x=%f     y=%f\n",ct,
	   scan_data.elapsed_time,scan_data.step_count,
	   plot_x[scan_data.step_count],plot_y[scan_data.step_count]);
    fflush(stdout);
#endif
    if(!flags.track_moon){
      if(scan_data.step_count<=(scan_data.total_steps/2))
        fl_replace_xyplot_point(fd_scan_form->xyplot,scan_data.step_count,
			    plot_x[scan_data.step_count],
			    plot_y[scan_data.step_count]);
      else
        fl_replace_xyplot_point(fd_scan_form->xyplot,
			      scan_data.total_steps-scan_data.step_count,
			    scan_data.radius,0.0);

                                 /* Starting adjusted ra (15degrees/hour)*/
      ra=burst_list[0]->ra_center-(plot_x[scan_data.step_count]/15);
      dec=burst_list[0]->dec_center+plot_y[scan_data.step_count];
    }
    else{    /* scanning moon */
      if(scan_data.step_count>=1){
	/*        ra=ra-(plot_x[scan_data.step_count-1]/15);
        dec=dec-plot_y[scan_data.step_count-1];*/
        ra=sra+(plot_x[scan_data.step_count]/15);
        dec=sdec+plot_y[scan_data.step_count];
        if(scan_data.odd)
          fl_replace_xyplot_point(fd_scan_form->xyplot,scan_data.step_count,
			    plot_x[scan_data.step_count],
			    plot_y[scan_data.step_count]);
	else
          fl_replace_xyplot_point(fd_scan_form->xyplot,scan_data.step_count,
			    0.0,0.0);

      }
      else
          fl_replace_xyplot_point(fd_scan_form->xyplot,scan_data.step_count,
			    (scan_data.radius/1000)*RTOD,0.0);
    }
    ftohms(ra, rastr);
    ftohms(dec, decstr);

    scan_data.step_count++;
    scan_data.elapsed_time-=(float)(1.0/STEPS_PER_SECOND);
  
  }
  else{
    if(ct==0.0 && flags.scan_mode_run==1){
      start_scan(fd_scan_form->xyplot,1);
      if(scan_data.odd)
	scan_data.odd=0;
      else
	scan_data.odd=1;
      return;
    }
  }
  elapsed_time=scan_data.start_time-ct;
  if(elapsed_time > (scan_data.last_elapsed_time+.1)){
    fprintf(fpscan,"%s        %7.3f          %7.3f         %7.3f\n",utstr,
	  elapsed_time,telel,telaz);
    scan_data.last_elapsed_time=elapsed_time;
    #ifdef DEBUGSCAN
    printf("%s        %f        %f         %f\n",utstr,
	 elapsed_time,telel,telaz);
    #endif
  }
}

void stop_scan(FL_OBJECT *ob, long data)
{
  int ct=0;

  track=no;
  fl_set_timer(fd_scan_form->timer,0.0);
  fclose(fpscan);
  fpscan = NULL;
  flags.scan_mode_run=0;
  if(!flags.track_moon){
    for(ct=-1;ct<scan_data.total_steps;ct++){    
      fl_replace_xyplot_point(fd_scan_form->xyplot,
			    (scan_data.total_steps-ct-1),scan_data.radius,0.0);
    }
    ra=burst_list[0]->ra_center-(plot_x[0]/15);
    dec=burst_list[0]->dec_center+plot_y[0];
    sra  = ra;
    sdec = dec;
    pra = ra;
    pdec = dec;
    ftohms(ra, rastr);
    ftohms(dec, decstr);
  }
  else{
    for(ct=-1;ct<scan_data.total_steps;ct++){    
      fl_replace_xyplot_point(fd_scan_form->xyplot,
	   (scan_data.total_steps-ct-1),0.0,0.0);
    }
  }
  fl_add_xyplot_overlay(fd_scan_form->xyplot,1,circle_x,circle_y,101,FL_RED);
  scan_data.start_time=0;
  scan_data.elapsed_time=0;
  scan_data.step_count=0;
  while(flags.scan_mode && !flags.on_source){
     update();
     fl_check_only_forms();
  }
  fl_activate_object(fd_track_form->offsource);
  fl_activate_object(fd_track_form->pointcheck);


}
void exit_scan(FL_OBJECT *ob, long data)
{
   fl_set_timer(fd_scan_form->timer,0.0);
   if(fpscan != NULL){
     fprintf(fpscan,"\nscan ended at %s UT time\n",utstr);
     fclose(fpscan);
   }
   flags.scan_mode=0;
   flags.scan_mode_run=0;
   flags.freeze=0;
   ra=temp_ra;
   dec=temp_dec;
   sra=temp_sra;
   sdec=temp_sdec;
   pra=temp_pra;
   pdec=temp_pdec;
   ftohms(ra, rastr);
   ftohms(dec, decstr);
   strcpy(id,temp_id);
   strcpy(id_code,temp_code);
   fl_hide_form(fd_scan_form->scan_form);
   fl_set_object_label(fd_track_form->mode,"STANDBY");
   track = no; /* VARIABLE THAT TELLS THE MAIN LOOP TO */
	         /* TRACK THE CURRENT SOURCE */

   burst_multiplier=1;
   fl_activate_object(fd_track_form->offsource);
   fl_activate_object(fd_track_form->pointcheck);

}
void start_scan(FL_OBJECT *ob, long data)
{
  char string[80],rstr[30],dstr[30];

  if(!flags.on_source){  /* do not start if not in start position */
    fl_show_alert("The telescope is not at the starting point.",
		  "Please wait","You must acknowledge to continue !",1);
    return;
  }
  fl_deactivate_object(fd_track_form->onsource);
  fl_deactivate_object(fd_track_form->offsource);
  fl_deactivate_object(fd_track_form->pointcheck);
  if(data==0){  /* only open file if this is the first start call */
    if(!flags.track_moon){
      strcpy(string,"/home/observer/burst_");
      strcat(string,&utstr[1]);
      strcat(string,".scan");
      ftohms(burst_list[0]->ra_center,rstr);   /* position of burst */
      ftohms(burst_list[0]->dec_center,dstr);
    }
    else{
      strcpy(string,"/home/observer/moon_");
      strcat(string,&utstr[1]);
      strcat(string,"_");
      strcat(string,datstr);
      strcat(string,".scan");
    }
    if( (fpscan = fopen( string, "wt" )) == NULL)
    {
      fl_show_alert("Unable to open the file...",
		  string,"You must correct to continue",1);
      return;
    }
    if(!flags.track_moon){
    fprintf(fpscan,"BURST at RA%s    DEC%s\nscan started at %s UT on %s\n\n",
	    rstr,dstr,utstr,datstr);
    fprintf(fpscan,"   UT              TIMER             EL             AZ\n");
    }
    else{
    fprintf(fpscan,"MOON scan started at %s UT time. Date is %s\n\n",
	    utstr,datstr);
    fprintf(fpscan,"   UT              TIMER             EL             AZ\n");
    }
  }

  fl_set_timer(fd_scan_form->timer,
	       (float)(scan_data.total_steps/STEPS_PER_SECOND));
  scan_data.start_time=scan_data.total_steps/STEPS_PER_SECOND;
  flags.scan_mode_run=1;
  scan_data.step_count=0;
  if(!flags.track_moon){
    scan_data.elapsed_time=2*scan_data.total_time;
    ftohms(burst_list[0]->ra_center,rstr);   /* position of burst */
    ftohms(burst_list[0]->dec_center,dstr);
  }
  else
    scan_data.elapsed_time=scan_data.total_time;
  track=yes;
}

void   initialize_cio()  /* initialize the i/o card */
{
  char DevNameIO[20];

  strcpy(DevNameIO, "/dev/dio48/dio0");
  if ((CIO_1 = open(DevNameIO, O_RDWR )) < 0) {
    perror(DevNameIO);
    printf("error opening device %s\n", DevNameIO);
    exit(2);
  }

  strcpy(DevNameIO, "/dev/dio48/dio1");
  if ((CIO_2 = open(DevNameIO, O_RDWR )) < 0) {
    perror(DevNameIO);
    printf("error opening device %s\n", DevNameIO);
    exit(2);
  }

  ioctl(CIO_1, DIO_SET_MODE, CNTL_A|CNTL_B|CNTL_C);
  ioctl(CIO_2, DIO_SET_MODE, CNTL_A|CNTL_B|CNTL_C);


}
/****************************************************************************/
void initialize_variables(void)
{
  int loop=0;

  strcpy(menustr,"MOVE HOME|ZENITH|PRECESS COORDINATES|   ");
  strcat(menustr,"|DO CORRECTIONS|TRACK MOON|STATUS|MIRROR ALIGN|EXIT PROGRAM");
  strcpy(current_message[1],"");
  strcpy(current_message[2],"");
  strcpy(current_message[3],"");
  burst_track=0;burst_number=-1;
  corrections_done=0;
  ra=2.52091667 ; dec=89.2619444 ; objel=40.; objaz=20.;
  sra=2.52091667; sdec=89.2619444;   pra=2.52091667; pdec=89.2619444;
  moon_position=center;off_moon=0;moon_time=0.;
  burst_position=center_b;
  slewsta = on;track = no;
  offset = 30; wagg=0;limit_set=0; az_direction=-1;
  telel=0; telaz=0; lastaz=0; lastel=0;
  old_telaz=0;old_telel=0;direction_alert_sent=0;
  tdist=0;teldisp=0;azzero=0;elzero=0;tdist_error=0;
  el_error=0;az_error=0;RA_error=0;DEC_error=0;
  strcpy(az_limit_message_cw,"CW AZIMUTH LIMIT HIT");
  strcpy(az_limit_message_ccw,"CCW AZIMUTH LIMIT HIT");
  strcpy(el_upper_limit_message,"ELEVATION UPPER LIMIT HIT");
  strcpy(el_lower_limit_message,"ELEVATION LOWER LIMIT HIT");
  strcpy(too_low_limit_message,"OBJECT IS TOO LOW TO TRACK");
  send_time=0;write_cycles=0;
  mode_int=STANDBY;status_code=0;
  auto_mode=0;receive_time=0;
  doing_corrections=0;cos_alpha=1.; sin_alpha=0.;
  cos_beta =1.; sin_beta =0.;cos_gamma=1.; tan_gamma=0.;delta=0.;
  az_cor_offset=0. ; el_cor_offset=0.;total_correction_entries=0;
  write_data=1;
  which_instr = 0;
  sources=0;
  burst_multiplier=1; /* multiplier in burst mode */
  send_error_message=0;  /*set if tracking error occured...send notice taurus*/

  for(loop=0;loop<20;loop++)
    strcpy(check_status_message[loop],""); 
}






