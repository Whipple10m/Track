
/* This include file has the subroutines that apply to the 10 meter tracking */

/*-------------------------------------------------------------------------*/
/* DATAIN Reads the telescopes az. & el. from the mount interface. */
void datain(void)
{
  /* To change az and el offsets (pc readout in relation to telescope)
     change AZOFFSET or ELOFFSET in the track.h file. A higher number here 
     is a higher readout on the screen. A change of +1 here is an increase of
     .010986 in real telescope position*/
  double azt=0., azo=0., elt=0., elo=0.;

  lastaz = telaz;   /* store value of AZ for CW/CCW decision */

  /* READ IN THE NEW AZIMUTH AND ELEVATION FROM ENCODERS*/
  gettel();
#ifdef DEBUG
  telel=objel;
  telaz=objaz;
#endif


#ifdef DEBUG_DIGITAL_INPUT
  printf("\nAZ raw: %f  EL raw: %f\n",telaz,telel);
#endif  
 /* Make sure they are in range */
 /* Adjust for encoder offsets and give degrees value in the proper range */
  telel = RANGE((telel+ELOFFSET),360.0);
  telaz = RANGE((telaz+AZOFFSET),360.0);
#ifdef DEBUG_DIGITAL_INPUT
  printf("AZOFFSET: %f  ELOFFSET: %f\n",AZOFFSET,ELOFFSET);
  printf("AZ   : %f  EL   : %f\n",telaz,telel);
#endif  

 /* Make elevation value go negative when 180-360 range */
  if (telel>180.0)
    telel-=360.0;
 
 /* CORRECT FOR TILT IN THE MOUNT */

  if((telel>0.1) && (telel < 89.999)){

    el_deg_error=0.0;
    az_deg_error=0.0;
    /* TPOINT model corrections installed 012500 KRH */
    /* Do not do corrections if telescope is below observing range in EL
       because they let the telescope go too low when stowing */
    if (telel>LOW_EL){
      tpoint_correction(&el_deg_error,&az_deg_error,telel,telaz);
      telel+=el_deg_error;
      telaz+=az_deg_error;
   

      /* VVV correction calls */
      el_deg_error=0.0;
      az_deg_error=0.0;
      correction(&el_deg_error, &az_deg_error, objel, 
	       objaz, fb_X, fb_Z, pmt_to_track);
      telel+=el_deg_error;
      telaz+=az_deg_error;
    }

  
    if(flags.doing_corrections)  
    {                      /* correction checks on stars */
      telaz+=az_cor_offset;
      telel+=el_cor_offset;
    }

    /* END newly added VVV correction calls */



  }

  /* GET DISTANCE FROM TEL TO OBJECT */
  azt = telaz / RTOD;  /* Convert telescope & object az & el to radians */
  elt = telel / RTOD;
  azo = objaz / RTOD;
  elo = objel / RTOD;
  tdist = acos( sin(elt)*sin(elo) + cos(azt-azo)*cos(elt)*cos(elo)) * RTOD;

#ifdef DEBUG_MOVEMENT
  if(track){
    printf("telaz=%8.4f  telel=%8.4f  \ndiff_az=%8.4f  diff_el=%8.4f\n",telaz,
	   telel,telaz-objaz,telel-objel);
  }
#endif

#ifdef DEBUG_DIGITAL_INPUT
  printf("AZ   : %f  EL   : %f\n",telaz,telel);
#endif  


  /*Check for change in azimuth direction about zero*/
  if(check_status() && !el_info.enable && !az_info.enable 
     && (track || flags.drift_mode))    
    limit_print("Tracking is off due to a limit being hit\n",1);

  if(RANGE(telaz,360.) < 10.0 || RANGE(telaz,360.) > 350.0 || 
     az_direction == -1)
     azimuth_direction();
  
}
/*-------------------------------------------------------------------------*/
void stoptel(void)       /* Bring telescope to a full halt */
{
  extern FD_track_form *fd_track_form;

  stop_motors();

  if (slewsta == off)
  {
    ra = sra;
    ftohms( ra, rastr );
    slewsta = on;
    /* set on/off buttons*/
    fl_set_button(fd_track_form->onsource,1); /* release button */
    fl_deactivate_object(fd_track_form->onsource);
    fl_set_button(fd_track_form->offsource,0); /* release button */
    fl_activate_object(fd_track_form->offsource);
  }

  if(!flags.drift_mode)        /* do not change mode if drifting */
    strcpy(mode,"STOPPED");
  mode_int=STANDBY;
  send_time+=500;   /* Make program send to vax immediatly */
  send_to_VHEGRO();
}
/*-------------------------------------------------------------------------*/
/* TELSTATUS Checks to see if the telescope position needs to
	     be adjusted to stay on target. */
void telstatus(void)
{
  double delel=0., delaz=0.;
  double range_az=0.;
  
  /* NEW TRACKING VARIABLES HERE */
  double future_az,future_el;
  int  i=0, obj_north=0, tel_north=0;
  int  obj_east=0;

  range_az=RANGE(telaz,360);
  /* decide if object and telescope are in north or south hemisphere*/
  if((range_az>270) || (range_az<90))
    tel_north=1;
  if((objaz>270) || (objaz<90))
    obj_north=1;
  if((objaz>0) && (objaz<180))  
    obj_east=1;

  /* rotate previous 10 speeds of az and el */
  for(i=9;i>=1;i--){
    az_speed[i]=az_speed[i-1];
    el_speed[i]=el_speed[i-1];
  }
  /* Get az and el velocities of the object being tracked */
  
  azandel(ra,dec,&objel,&objaz,st);    /*current az and el*/
  azandel(ra,dec,&future_el,&future_az,st+.01667); /*future (1min) az and el*/
  az_speed[0]=(future_az-objaz);    /* az velocity in degrees/minute */

  if ( az_speed[0] > 180. )
    az_speed[0] = az_speed[0] - 360.;
  else
    if ( az_speed[0] < -180. )
      az_speed[0] = az_speed[0] + 360.;

  el_speed[0]=(future_el-objel);    /* el velocity in degrees/minute */


#ifdef DEBUGTELSTATUS
printf("telstatus:      oj_EL=%f       oj_AZ=%f\n",el_speed[0],az_speed[0]);
#endif


  /* Get the distance of AZ and EL to the object */
  delel = telel - objel;       /* elevation offset from source */

#ifdef DEBUG /*TELSTATUS*/
printf("telstatus: range_az=%f  objaz=%f\n",range_az,objaz);
#endif
  delaz = objaz - range_az;
  if ( delaz > 180. )
    delaz = delaz - 360.;
  else
    if ( delaz < -180. )
      delaz = delaz + 360.;

  /* Set directions of the drives if not in drift mode*/
  /* Allow drives to change directions only if distance between telescope
     and object is greater than .04 degrees...otherwise just change the
     speed slower or faster */

#ifdef DEBUG /*TELSTATUS*/
printf("telstatus: delaz=%f\n",delaz);
#endif


       if(delaz > 0.0)
	 az_info.direction = CW;
       else
	 az_info.direction = CCW;
       if (!tel_north && obj_north)
       {
    /* If azimuth will hit the limit switch go around the other way */
         if (az_direction == CW)
           az_info.direction = CCW;
         else
           az_info.direction = CW;
       }
       
       if( delel >= 0.0 )
         el_info.direction = DOWN;
       else
         el_info.direction = UP;

#ifdef DEBUG /*TELSTATUS*/
printf("telstatus: directions el:%d UP=%d az:%d CW=%d\n",el_info.direction,
       UP,az_info.direction,CW);
if(az_direction == CW)
 printf("telstatus: delel:%f delaz:%f az_direction:CW\n",delel,delaz);
else
 printf("telstatus: delel:%f delaz:%f az_direction:CCW\n",delel,delaz);

#endif

  /* Now add or subtract speed if the telescope is behind or ahead of source */
  /* Added speed is the speed to catch the object in 30 seconds */
/*NEW SOFTWARE-ELEVATION*/

    if(obj_east)
      el_info.velocity = fabs(fabs(el_speed[0])-(EL_GAIN*delel)+
	EL2_GAIN*pow(delel,2));
    else
      el_info.velocity = fabs(fabs(el_speed[0])+(EL_GAIN*delel)+
	EL2_GAIN*pow(delel,2));

    if((telel < LOW_EL+1.0) && (el_info.direction != UP)
         && !((range_az < 1.5) || (range_az > 358.5)))
      el_info.velocity = 0.0;

    if(el_info.velocity>MAXSLEW)
      el_info.velocity = MAXSLEW;


/*NEW SOFTWARE-AZIMUTH*/

    if(obj_north)
      az_info.velocity = fabs(fabs(az_speed[0])-(AZ_GAIN*delaz)+
	AZ2_GAIN*pow(delaz,2));
    else
      az_info.velocity = fabs(fabs(az_speed[0])+(AZ_GAIN*delaz)+
	AZ2_GAIN*pow(delaz,2));

    if(az_info.velocity>MAXSLEW)
      az_info.velocity = MAXSLEW;
    if (telel < LOW_EL)
      az_info.velocity=0.0;

  /* END of SET AZ speed */


#ifdef DEBUGTELSTATUS
printf("telstatus:  el_speed: %f  az_speed: %f\n",el_info.velocity,
       az_info.velocity);
printf("telstatus:    delel: %f     delaz: %f\n",delel,delaz);
#endif  
#ifdef DEBUG_MOVEMENT
printf("telstatus:  el_speed: %f --%f  %f  %f\n",el_info.velocity,
       fabs(el_speed[0]),EL_GAIN*delel,EL2_GAIN*pow(delel,2));  
printf("telstatus:  az_speed: %f --%f  %f  %f\n",az_info.velocity,
       fabs(az_speed[0]),AZ_GAIN*delaz,AZ2_GAIN*pow(delaz,2));  

printf("telstatus:    delel: %f     delaz: %f\n\n\n",delel,delaz);
#endif  
  if( track ){
    if( tdist <= 0.05 ){
      flags.on_source=1;           /* telescope is on source */
      if ( slewsta == on)
      {
	strcpy(mode,"ON SOURCE");
        if (mode_int!=ONSOURCE)      /* If a change in mode then...*/
	    send_time+=500;   /* Make program send to vax immediatly*/
	mode_int=ONSOURCE;
#if 0
	/* SJF 2004-09-15 -- send_to_VHEGRO now in update() */
	send_to_VHEGRO(); 
#endif
      }
      else
      {
	strcpy(mode,"OFF SOURCE");
        if (mode_int!=OFFSOURCE)    /* If a change in mode then...*/
	   send_time+=500;  /* Make program send to vax immediatly*/
	mode_int=OFFSOURCE;
#if 0
	/* SJF 2004-09-15 -- send_to_VHEGRO now in update() */
	send_to_VHEGRO(); 
#endif
      }
    }
    else
    {
        flags.on_source=0;          /* telescope is not on source yet */
        if(slewsta==on)
	  strcpy(mode,"SLEWING ON");
        else
	  strcpy(mode,"SLEWING OFF");
        if (mode_int!=SLEWING)    /* If a change in mode then...*/
	  send_time+=500;   /* Make program send to vax immediatly*/
        mode_int=SLEWING;
#if 0
	/* SJF 2004-09-15 -- send_to_VHEGRO now in update() */
        send_to_VHEGRO();
#endif
    }

#ifdef DEBUG
        flags.on_source=1;          /* telescope is not on source yet */
#endif

    /* CHECK IF THE TEL HAS HIT THE LIMIT SWITCH IN AZ, IF SO STOP */
	if ( ((int)(RANGE(telaz,360.)) == 269) && (az_direction==CW)){
	  track=no;
	  stoptel();
          printf("az_direction=CW RANGE(telaz,360.)=%f\n",RANGE(telaz,360.));
	  fl_show_alert("",az_limit_message_cw,"",0);
	}
	if ( ((int)(RANGE(telaz,360.)) == 91) &&  (az_direction==CCW)){
	  track=no;
	  stoptel();
          printf("az_direction=CW RANGE(telaz,360.)=%f\n",RANGE(telaz,360.));
	  fl_show_alert("",az_limit_message_ccw,"",0);
	}
	/* IF THE TEL IS ABOVE 90 DEG IN ELEVATION AND MOVING UP, STOP */
	if ( telel > 90. && el_info.direction == UP){
	  track=no;
	  stoptel();
	  fl_show_alert("",el_upper_limit_message,"",0);
	}
    /* IF THE ELEVATION IS BELOW 15 DEG. AND outside the window STOP*/
	if ( telel < 15. && el_info.direction == DOWN && !status.az_window){
	  track=no;
	  stoptel();
	  fl_show_alert("",el_lower_limit_message,"",0);
	}
	/* IF THE OBJECT IS BELOW 15 DEG STOP AND ALERT THE OPERATOR */
	if ( objel < 15. && fabs(RANGE(telaz,360.))>1.5){ 
	  /*if not in az slot */
	  track=no;
	  stoptel();
	  fl_show_alert("",too_low_limit_message,"",0);
	}
  }/*end if(track)*/
  if(!track && !flags.drift_mode)
    strcpy(mode,"STOPPED");
  if(flags.pointing_check)
    strcpy(mode,"POINTING");
  return;
}
/*-------------------------------------------------------------------------*/
void azimuth_direction(void)
{
  double range_az;
  int test;
  char buf[300],temp[80];
  FILE *fp;


  /* CW=0,  CCW=1 */

  if (az_direction==-1){ /* Program just starting and direction not known */
    if( (fp = fopen( "/home/observer/bin/.track_start_direction",
		     "r" )) == NULL){
      if( (fp = fopen( "/home/observer/bin/.track_start_direction",
		     "wt" )) != NULL){
        strcpy(buf,"Starting Azimuth direction in not known.
                 \nFile /home/observer/bin/.track_start_direction is missing.
                 \nWas CW the starting Azimuth direction from zero?");
        test=fl_show_question(buf,1);
        if(test){
           az_direction=CW;
           printf("Telescope direction chosen to be CW.\n");
           fprintf(fp,"%d  CW\n",CW);
        }
        else{
           az_direction=CCW;
           printf("Telescope direction chosen to be CCW.\n");
           fprintf(fp,"%d  CCW\n",CCW);
        }
        fclose(fp);
      }
    }
    else {  /* read direction from input file */
      if( fgets(temp,80,fp) == NULL){  /* Read comment line in file */
	    fprintf(stderr,"%s: error reading %s\n",progname,"direction");
	    fclose(fp);
	    return;
	}
      sscanf(temp,"%d",&test);
      if(test==CW){
	az_direction=CW;
        printf("initial direction=CW\n");
      }
      else if(test==CCW){
	az_direction=CCW;
        printf("initial direction=CCW\n");
      }
      else{ 
	az_direction=-1;
        printf("initial direction NOT KNOWN!!");
      } 
    }
    fclose(fp);
  }
  else {
    range_az=RANGE(telaz,360);

    if ((range_az >= 0.0) && (range_az < 10.0) && az_direction==CCW){
      az_direction=CW;
      if( (fp = fopen( "/home/observer/bin/.track_start_direction",
		     "wt" )) != NULL){
         fprintf(fp,"%d  CW\n",CW);
printf("writing CW az direction to /home/observer/bin/.track_direction\ntelescope az is %f\n",range_az);
         fclose(fp);

      }
    }
    else if ((range_az < 359.999) && (range_az > 350.0) && az_direction==CW){
      az_direction=CCW;
      if( (fp = fopen( "/home/observer/bin/.track_start_direction",
		     "wt" )) != NULL){
         fprintf(fp,"%d  CCW\n",CCW);
printf("writing CCW az direction to /home/observer/bin/.track_direction\ntelescope az is %f\n",range_az);
         fclose(fp);
      }
    }
  }
}

/*direction_alert_sent==*/
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
/* Get encoder values from the COMPUTOR BOARDS CARD */
void gettel(void)
{
  int enc_bin,count=0;
  unsigned long data1=0,data2=0;
  BYTE  port1[3],port2[3],temp=0;

  do{
   read(CIO_1, &data1, 3);
   split(data1,port1);
   limit_bits=port1[2];
   read(CIO_1, &data1, 3);
   split(data1,port1);
   temp=port1[2];
   count++;
   limit_bits &= 63;
   temp &= 63;
  }while((limit_bits != temp) && count < 10);
  
  if(count>=10)
    printf("Tried 10 times to get CIO_1 port.\n");
  count=0;
  temp=0;

  do{     
   read(CIO_2, &data2, 3);
   split(data2,port2);
   status_bits=port2[2];
   read(CIO_2, &data2, 3);
   split(data2,port2);
   temp=port2[2];
   count++;
  }while((status_bits != temp) && count < 10);

  if(count>=10)
    printf("Tried 10 times to get CIO_2 port.\n");

#ifdef DEBUG_DIGITAL_INPUT
  printf("CIO_1 input: ");
  printbin(data1,24);
  printf("\n");
  printf("CIO_2 input: ");
  printbin(data2,24);
  printf("\n");
  printf("STATUS: ");
  printbin(status_bits,24);
  printf("\n");
  printf("LIMITS: ");
  printbin(limit_bits,24);
  printf("\n");
#endif 

  enc_bin = (int) port1[1];
  enc_bin <<= 8;
  enc_bin += (int) port1[0];
      
  enc_bin = gb((WORD) enc_bin, 16);
  telaz = (float) enc_bin * ENCODER_TO_DEG;
       
  enc_bin = (int) port2[1];
  enc_bin <<= 8;
  enc_bin += (int) port2[0];
  enc_bin = gb((WORD) enc_bin, 16);

  telel =(float) enc_bin * ENCODER_TO_DEG;

/* The next few lines are for program test mode only */
#ifdef DEBUG
   telaz=objaz;
   telel=objel;
   return;
#endif

}
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
void drift_tel(double dumel,double dumaz)
{
    double delaz, delel;
    double range_az;
    int  obj_north=0, tel_north=0;


    flags.drift_mode=1;
    track = no;
    datain();
    stoptel();

    setup_motors(&az_info,1,0); /* initialize controllers */
    setup_motors(&el_info,1,0);

    /* SET AZ. & EL as Object AZ and EL*/

    objaz = RANGE( dumaz,360.);
    objel = dumel;

    range_az=RANGE(telaz,360);
    /* decide if object and telescope are in north or south hemisphere*/
    if((range_az>270) || (range_az<90))
      tel_north=1;
    if((objaz>270) || (objaz<90))
      obj_north=1;

    delaz = objaz - range_az;
    if ( delaz > 180. )
      delaz = delaz - 360.;
    else
      if ( delaz < -180. )
        delaz = delaz + 360.;

    /* IF THE TEL IS NOT ABOVE 16 DEG. ELEVATION AND AZIMUTH IS NOT IN WINDOW
       , MOVE IT ABOVE 20 DEG */
    /* Loop until above 20 */
    /*    flags.pointing_check=1;*/
    flags.drift_flag=1;  /*do not update object position */
    if((!status.az_window||!((objaz < 1.0)||(objaz > 359.0))) && telel<LOW_EL)
      while( telel < LOW_EL && flags.drift_mode)
	  {
	  update();
	  el_info.direction = UP;
	  el_info.velocity = 20;
	  fl_check_only_forms();
	  wait_msecs(100);
#if 0
	/* SJF 2004-09-15 -- send_to_VHEGRO now in update() */
	  send_to_VHEGRO();
#endif
      }
    
    /* Loop until at position */
      while( (fabs(delaz) > 0.01 || fabs(telel-objel) > 0.01) && 
	    flags.drift_mode)
     {
       wait_msecs(100);
       fl_check_only_forms();
       update();
#if 0
	/* SJF 2004-09-15 -- send_to_VHEGRO now in update() */
       send_to_VHEGRO();
#endif

       range_az=RANGE(telaz,360);
       delaz = objaz - range_az;
       if ( delaz > 180. )
         delaz = delaz - 360.;
       else
         if ( delaz < -180. )
           delaz = delaz + 360.;

       delel = telel - objel;


       if(delaz > 0.0)
	 az_info.direction = CW;
       else
	 az_info.direction = CCW;
       if (!tel_north && obj_north)
       {
    /* If azimuth will hit the limit switch go around the other way */
         if (az_direction == CW)
           az_info.direction = CCW;
         else
           az_info.direction = CW;
       }
       
       if( delel >= 0.0 )
         el_info.direction = DOWN;
       else
         el_info.direction = UP;



       /* Set telescope motor velocities */

       if(((telel>LOW_EL) || ((range_az < 1.3) || (range_az > 358.7)))
	  && fabs(delel)>.01 )
           el_info.velocity=.5+15*fabs(delel);
              /* Anything >5 degrees away... max*/
       /* Correct for condition where az was moved out of window while el was
	  below 15 degrees...only direction is UP */
       else if((el_info.direction == UP) && (telel<LOW_EL))
	  el_info.velocity=.5+15*fabs(delel);
       else
           el_info.velocity=0.0;    /* EL cannot move down until in window */

       if(telel > LOW_EL){
	 if((telel > LOW_EL) && (fabs(delaz) >.01)){
	   if(telel < 89)
	     az_info.velocity=.5+(10*fabs(delaz));
	   else
	     az_info.velocity=.5+((10*fabs(delaz)));
	 }
	 else {
	   az_info.velocity=0.0;    /* AZ cannot move with el < 15 degrees */
	 }
       }
       else {
       if(status.az_window && (fabs(delaz) >.01) && 
	  ((range_az < 1.3) || (range_az > 358.7)))
	 az_info.velocity=.5+((10*fabs(delaz)));	 
       else
	   az_info.velocity=0.0;    /* AZ cannot move outside the window */
       }



#ifdef DEBUGDRIFT
printf("\nobjel= %f telel= %f delel= %f  objaz= %f range_az= %f delaz=%f\n",
       objel,telel,delel,objaz,range_az,delaz);
printf("az_direction from zero= %d  CW= %d\n",az_direction,CW);
printf("el_direction= %d  DOWN= %d\n",el_info.direction,DOWN);
printf("az_direction= %d \n",az_info.direction);

printf("az_speed= %f  el_speed= %f\n\n",az_info.velocity,el_info.velocity);

#endif


     }


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

/*-------------------------------------------------------------------------*/

int check_status()
     /* Check status information from telescope and return fault indicator
	1=fault (problem) 0=all is fine */
{
  int return_set=0,i=0;
  
  for(i=0;i<20;i++)
    strcpy(check_status_message[i],"");
  i=0;
     /* test for limits...a 1 means not hit */
  if(!(limit_bits & AZ_CW)){
    status.cw_enable=0;
    sprintf(check_status_message[i++],"%d: Azimuth CW is disabled",i);
  }
  else
    status.cw_enable=1;
  if(!(limit_bits & AZ_CCW)){
    status.ccw_enable=0;
    sprintf(check_status_message[i++],"%d: Azimuth CCW is disabled",i);
  }
  else
    status.ccw_enable=1;
  if(!(limit_bits & EL_UP)){
    status.up_enable=0;
    sprintf(check_status_message[i++],"%d: Elevation UP is disabled",i);
  }
  else
    status.up_enable=1;
  if(!(limit_bits & EL_DOWN)){
    status.down_enable=0;
    sprintf(check_status_message[i++],"%d: Elevation DOWN is disabled",i);
  }
  else
    status.down_enable=1;
  if(!(limit_bits & EL_15))
    status.el_15=1;
  else{
    status.el_15=0;
    sprintf(check_status_message[i++],"%d: Elevation BELOW 15 degrees",i);
  }
  if(!(limit_bits & WINDOW)){
    status.az_window=1;
    sprintf(check_status_message[i++],"%d: Azimuth in 2.5>AZ<357.5 window",i);
  }
  else{
    status.az_window=0;
    sprintf(check_status_message[i++],"%d: Azimuth outside 2.5>AZ<357.5 window",i);
  }
  if(status_bits & EL_OK)
    status.el_drive_ready=1;
  else{
    status.el_drive_ready=0;
    sprintf(check_status_message[i++],"%d: Elevation not ready",i);
    /*printf("STATUS CHECK : Elevation not ready\n");*/
    return_set=5;
  }
  if(status_bits & AZ_OK)
    status.az_drive_ready=1;
  else{
    status.az_drive_ready=0;
    /*printf("STATUS CHECK : Azimuth not ready\n");*/
    sprintf(check_status_message[i++],"%d: Azimuth not ready",i);
    return_set=5;
  }
  if(!(status_bits & EL_AT_SPEED))
    status.el_at_speed=1;
  else
    status.el_at_speed=0;
  if(!(status_bits & AZ_AT_SPEED))
    status.az_at_speed=1;
  else
    status.az_at_speed=0;
  if(!(status_bits & CW_CCW))
    status.cw_rotation=CW;
  else
    status.cw_rotation=CCW;
  if(!(status_bits & EL_AUTO))
    status.el_auto=1;
  else{
    status.el_auto=0;
    sprintf(check_status_message[i++],"%d: Elevation in manual mode",i);
    return_set=1;
  }
  if(!(status_bits & AZ_AUTO))
    status.az_auto=1;
  else{
    status.az_auto=0;
    sprintf(check_status_message[i++],"%d: Azimuth in manual mode",i);
    return_set=1;
  }
  if(!(status_bits & HT_ON))
    status.high_voltage_on=1;
  else
    status.high_voltage_on=0;

  /* Now do some logic to decide if drives are disabled and which limits
     have actually been hit */

  if(!status.cw_enable || !status.cw_enable || !status.up_enable ||
     !status.down_enable){
    sprintf(check_status_message[i++],"%d: A direction is disabled",i);

    if(!status.up_enable && !status.down_enable){
       status.el_enable=0;
       sprintf(check_status_message[i++],"%d: Elevation drive is disabled",i);
       return_set=1;
    }
    else
      status.el_enable=1;
    if(!status.cw_enable && !status.ccw_enable){
      if(status.el_15 || status.az_window){
         sprintf(check_status_message[i++],"%d: Azimuth drive is disabled",i);
	 status.az_enable=0;
         return_set=1;
      }
      else{
         status.az_enable=1;     
         if(!status.el_15 && !status.az_window){
	   status.too_low=1;
           sprintf(check_status_message[i++],
		  "%d: Telescope below 15 degrees and not in window",i);
           printf("STATUS CHECK %d: Telescope below 15 degrees and not in window",i);
           return_set=1;
         }
      }
    }
    else{
      status.el_enable=1;
      status.az_enable=1;
    }
  }

  

#ifdef DEBUG_DIGITAL_INPUT  /* if in debug mode print status settings */
  printf("\n\n\nStatus settings after check_status() routine:\n");
  printf("LIMIT: ");
  printbin(limit_bits,8);
  printf("    STATUS: ");
  printbin(status_bits,8);
  printf("\n\n");
  if(status.el_enable)
    printf("Elevation enabled\n");
  else
    printf("Elevation disabled\n");
  if(status.az_enable)
    printf("Azimuth enabled\n");
  else
    printf("Azimuth disabled\n");
  if(status.up_enable)
    printf("UP enabled\n");
  else
    printf("UP disabled\n");
  if(status.down_enable)
    printf("DOWN enabled\n");
  else
    printf("DOWN disabled\n");
  if(status.cw_enable)
    printf("CW enabled\n");
  else
    printf("CW disabled\n");
  if(status.ccw_enable)
    printf("CCW enabled\n");
  else
    printf("CCW disabled\n");
  if(status.high_voltage_on)
    printf("High Voltage is ON\n");
  else
    printf("High Voltage is OFF\n");
  if(status.cw_rotation==CW)
    printf("Telescope has moved clockwise\n");
  else
    printf("Telescope has moved counter-clockwise\n");
  if(status.az_auto)
    printf("Azimuth is in AUTO mode\n");
  else
    printf("Azimuth is in MANUAL mode\n");
  if(status.el_auto)
    printf("Elevation is in AUTO mode\n");
  else
    printf("Elevation is in MANUAL mode\n");
  if(status.az_window)
    printf("Azimuth is in stow window\n");
  else
    printf("Azimuth is outside the stow window\n");
  if(status.el_15)
    printf("Elevation is above 15 degrees\n");
  else
    printf("Elevation is below 15 degrees\n");
  if(status.el_drive_ready)
    printf("Elevation drive is ready\n");
  else
    printf("Elevation drive is NOT ready\n");
  if(status.az_drive_ready)
    printf("Azimuth drive is ready\n");
  else
    printf("Azimuth drive is NOT ready\n");
  if(status.el_at_speed)
    printf("Elevation motor is at speed\n");
  else
    printf("Elevation motor is NOT at speed\n");
  if(status.az_at_speed)
    printf("Azimuth motor is at speed\n");
  else
    printf("Azimuth motor is NOT at speed\n");
  printf("\nMESSAGES:\n");
  i=0;
  do{
    printf("%d: %s\n",i,check_status_message[i]);
  }while((strlen(check_status_message[i++])>0) && (i<20)); 
  sleep(3);
#endif

  /*  return(return_set);*/
return(0);
}


/********************************************************/
/*  Convert m-bit graycode number n to binary           */
/*                                                      */
/*  Formula:    b(MSB) = g(MSB)                         */
/*              b(i-1) = b(i) ^ g(i-1)                  */
/*                                                      */
/********************************************************/

int gb(int n, int m)
{
  int ans=0, bits=1<<(m-1);
  for( ans=n&bits;bits!=0;bits>>=1)
    ans |= (n^(ans>>1)) & bits;
  return ans;
}


/********************************************************/
/*  Print out binary number n                           */
/*  with m bits                                         */
/*                                                      */
/********************************************************/

void printbin(LONG n, int m)
{
  int i;
  for(i=1<<(m-1);i>0;i>>=1)
    (n&i) ? printf("1") : printf("0");
  return;
}

void split(unsigned long n,BYTE *m)
     /* Split the input 24 bits into an encoder 16 bits and a status 8 bits */
{
  
  m[0]=(BYTE) ((n) & (255));
  n>>=8;
  m[1]=(BYTE) ((n) & (255));
  n>>=8;
  m[2]=(BYTE) ((n) & (255));

}   

/*-------------------------------------------------------------------------*/
void dataout(void)
{
  extern Info_struct az_info,el_info;
  int temp=0,test=0;
  char buf[30];

  if ((test=check_status())==0 || (status.el_15==0 && status.az_window==0)){
    if(status.el_drive_ready && status.el_auto){
      if(status.up_enable && (el_info.direction==UP)){
        move_motor(&el_info);
      }
      else if(status.down_enable && (el_info.direction==DOWN))
        move_motor(&el_info);
    }
    else
      temp=1;
    if(status.az_drive_ready && status.az_auto){
      if(status.cw_enable && az_info.direction==CW)
        move_motor(&az_info);
      else if(status.ccw_enable && az_info.direction==CCW)
        move_motor(&az_info);
    }
    else
      temp=1;
  }
  else{
    if(test==0){
      /*read status byte again to check for error in read */
      if(check_status()){
        strcpy(buf,"dataout() error");
        limit_print(buf,1);
      }
     }

  }

  if(temp){
    printf("Problem with check status : dataout()\n");
    if(!status.el_drive_ready)
      printf("!el_drive_ready: dataout()\n");
    if(!status.el_at_speed)
      printf("!el_at_speed: dataout()\n");
    if(!status.el_auto)
      printf("!el_auto: dataout()\n");
    if(!status.az_drive_ready)
      printf("!az_drive_ready: dataout()\n");
    if(!status.az_at_speed)
      printf("!az_at_speed: dataout()\n");
    if(!status.az_auto)
      printf("!az_auto: dataout()\n");
  }



}
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/














