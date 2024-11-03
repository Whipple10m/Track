#define extern
#include "controllers.h"
#undef extern

/**************************************************************************/


/**************************************************************************/
char readchar(int port_in)
{  char last; 
   int ret,count=0;

   errno = 0;
   while((ret = read(port_in, &last, 1)) != 1 && count++ < 50) 
   { 
     if(errno != EAGAIN && ret != 0){
         fprintf(stderr, "BAD READ port_in = %d  ret=%d\n\r",
		 port_in,ret); usleep(1000); }  
      else
      {  
         usleep(100);}
   }

#ifdef DEBUGINPUT
   if(last == 0x3A)
     fprintf(stderr, " \nstart  ");
   else if(last == 0x0D)
     fprintf(stderr, " end\n");
   else
     fprintf(stderr, "-%c-", last);
   fflush(stdout);
#endif
   /*   if(count>20)
     printf("counts of .1ms in readchar: %d\n",count);
   */
   if(count>=49){
     printf("\nserial input character read: timed out\n");
     fflush(stdout);
     return(0x1a);  /*timed out...return error code ^Z */
   }
   return(last); 
}
/**************************************************************************/

int tty_set(char *portname)
{  
   int fd;
   struct termios port;

   if((fd = open(portname, O_RDWR | O_NOCTTY | O_NONBLOCK, 0)) < 0)
   {  fprintf(stderr, "ttyset can't open %s\n\r", portname);
      perror(":");
      return(-1);
   }
#ifdef DEBUGINPUT
   fprintf(stderr, "ttyset opened %s\n\r", portname);
#endif
   port.c_iflag = IGNBRK;   
   /*   port.c_oflag = ;*/
   port.c_cflag = B9600 | CS8 | CLOCAL | CREAD | !CRTSCTS;
   port.c_lflag = NOFLSH | !ISIG;  
   if(ioctl(fd, TCSETS, &port) < 0)
   {  perror("ttyset can't ioctl");
      close(fd);
      return(-1);
   }
#ifdef DEBUGINPUT
  fprintf(stderr, "Port %s now set to 9600 baud\n\r", portname);  
#endif
  return(fd);
}

/**************************************************************************/

int  get_response(char *buf,int port_in)
{
 int i=0,j=0,test=1;


 while(test){
   buf[0]=readchar(port_in);
   if(buf[0] == 0x3a) /* ':' */
     test=0;
   if(buf[0] == 0x1a) /* error code from readchar */
     return(-1);
   }



 test=1;
 i=0;
 while(test){
   buf[i]=readchar(port_in);
   if(buf[i] == 0x0d)  /* '\r' */
     test=0;
   if(buf[i] == 0x1a) /* error code from readchar */
     return(-1);
   i++;
 }

 buf[i-1]='\0';
 if((checksum(buf,(i-1))) != 0){
    printf("\nerror on checksum\n");
    return(-1);
 }

 /* Now strip command and checksum from string */
#ifdef DEBUGMOTORS
printf("return:\n---%s---\n",buf);
#endif

 for(j=0;j<(i-8);j++){
   buf[j]=buf[j+5];
 }
   buf[j]='\0';
 return(j);

}

/**************************************************************************/
/**************************************************************************/

int   checksum(char *buf,int size)
{
  int count,sum=0;
    for(count=0;count<(size-2);count++){
      sum += (int)buf[count];
    }
    if((int)buf[size-2]>64)
      sum+=(((int)buf[size-2]-55)*16);
    else
      sum+=(((int)buf[size-2]-48)*16);
    if((int)buf[size-1]>64)
      sum+=(((int)buf[size-1]-55));
    else
      sum+=(((int)buf[size-1]-48));

return(sum%256);

}
/**************************************************************************/

int   find_checksum(char *buf,int size)
     /* The controller requires a checksum added to the command string */
{
  int count,sum=0,mod=0,one,two;

    for(count=0;count<size;count++){
      sum += (int)buf[count];
    }

    mod=((sum/256)*256)+256-sum;

    one=(int)(mod/16);
    two=mod-(one*16);
    
    if(one>15)    /* Correct for a roll over (nothing higher than F)*/
      one-=16;

    if(one>9)
      one+=55;
    else
      one+=48;
    if(two>9)
      two+=55;
    else
      two+=48;

    buf[size]=one;
    buf[size+1]=two;
    buf[size+2]='\0';

return(size+2);

}


/**************************************************************************/

int   command_out(char *buf,int size,int port_out,char *unit,char *response)
{
  int count=0,test,size_in;
  int retry=0; 

#ifdef COMMANDOUT
      /* New command out software...try to send command 5 times bfore
	 giving up */
  if(strlen(buf)!=size)
    printf("command_out size error strlen=%d size=%d\n",strlen(buf),size);


  do{
    test=0;
    for(count=0;count<strlen(buf);count++)
      test=write(port_out,&buf[count],1);
    usleep(100);
    if((size_in = get_response(response,SERIAL_PORT)) <0)
      retry++;
    else
      retry=10;
  }while(retry<5);
  /* If too many failures occured tell operator*/
  if(retry<10){
    controller_response_error(buf,unit);
    return(0);
  }
#endif


  return(1);
}

/**************************************************************************/

int  build_command(char *out_command,char *data,char *buf,int drive)
{
  int size=0,i;
  char temp[80];

  if(drive==AZIMUTH)
    sprintf(temp,"00");
  else if(drive==ELEVATION)
    sprintf(temp,"01");
  else
    return(-1);
  strcat(temp,out_command);
  strcat(temp,data);

  /* convert all characters to capitals */
  for(i=0;i<strlen(temp);i++)
    temp[i]=(char)toupper((int)temp[i]);

  size=find_checksum(temp,strlen(temp));
  sprintf(buf,":%s\r",temp);
#ifdef DEBUGCOMMANDS
printf("build_command: %s \n",temp);
#endif
  return(size+2);
}

/**************************************************************************/

void stop_motors(void) /* Stop and disable the motors */
{

 extern Info_struct az_info,el_info;


 az_info.velocity=0.0;
 el_info.velocity=0.0;
 az_info.direction=FORWARD;
 el_info.direction=FORWARD;

 move_motor(&az_info);
 move_motor(&el_info);
 sleep(1); /* Wait for motors to stop */
 /* May not need this:
   az_info.enable=DISABLE;
   el_info.enable=DISABLE;
 */
 /* az_info.set_point_mode=OFF;
    el_info.set_point_mode=OFF;
 */
 setup_motors(&az_info,0,0);
 setup_motors(&el_info,0,0);
}

/**************************************************************************/

void move_motor(Info_struct *drive)
{
 char dataout[20],command[20],unit[20],response[80];
 int size_out;

#ifdef DEBUG
 return;
#endif
/* Set the direction of the motor */
 strcpy(dataout,"");
 if(drive->drive==AZIMUTH)
   strcpy(unit,"AZIMUTH");
 else 
   strcpy(unit,"ELEVATION");

 /* handle directions by positive or negative velocity */

 /* if changing directions...set to zero then start again */
    if(drive->last_direction != drive->direction){
#ifdef DEBUGMOTORS
printf("move_motors: changing directions: %d\n",drive->drive);
#endif
      dpm_to_hex(0.0,dataout,drive->drive);
      size_out=build_command(VELOCITY_SETPOINT_WRITE,
			     dataout,command,drive->drive);

      if(!command_out(command,size_out,SERIAL_PORT,unit,response))
        return;

     if(drive->velocity > 20)
       sleep(1);  /* allow 1 second for high speed wind down */
    

    if(drive->direction == REVERSE)     
      dpm_to_hex(((-1)*drive->velocity),dataout,drive->drive);
    else
      dpm_to_hex(drive->velocity,dataout,drive->drive);

    size_out=build_command(VELOCITY_SETPOINT_WRITE,
			   dataout,command,drive->drive);


    if(!command_out(command,size_out,SERIAL_PORT,unit,response))
       return;
  }
  else if(fabs(drive->velocity-drive->last_velocity)>.001){
    /* Only change the speed if the new speed is more than .01 deg/min 
       different. */
    strcpy(dataout,"");

    if(drive->direction == REVERSE)     
      dpm_to_hex(((-1)*drive->velocity),dataout,drive->drive);
    else
      dpm_to_hex(drive->velocity,dataout,drive->drive);

    size_out=build_command(VELOCITY_SETPOINT_WRITE,
			   dataout,command,drive->drive);

    if(!command_out(command,size_out,SERIAL_PORT,unit,response))
      return;
 
    
  }
    drive->last_velocity = drive->velocity;
    drive->last_direction= drive->direction;
}
/**************************************************************************/

void setup_motors(Info_struct *drive,int init_motors,int reset_motors)
     /* Use the information in control_info structure to set up the
	motor controller. The drive to be set up is in "drive" struct */
{
 char dataout[20],command[20],unit[20],response[80];
 int size_out;

 strcpy(dataout,"");
#ifdef DEBUG
return;
#endif 
  if(drive->drive==AZIMUTH)
   strcpy(unit,"AZIMUTH");
 else 
   strcpy(unit,"ELEVATION");

 
 if(init_motors){
   /* Set the proportional gain */
  /* Set the controllers to monitor the direction enable limit switches*/
   if(read_digital_inputs()==1){ /* read default status of controller inputs */
     set_digital_inputs(); /* change if not in operating mode*/
     read_digital_inputs(); /* read default status of controller inputs */
   }
   sprintf(dataout,"%x",drive->proportional_gain);
   size(dataout,4);
   size_out=build_command(VEL_PROP_GAIN_WRITE,dataout,command,drive->drive);
 

    if(!command_out(command,size_out,SERIAL_PORT,unit,response))
      return;

   /* Set the derivative gain */

   sprintf(dataout,"%x",drive->derivative_gain);
   size(dataout,4);
   size_out=build_command(VEL_DERIV_GAIN_WRITE,dataout,command,drive->drive);
 

    if(!command_out(command,size_out,SERIAL_PORT,unit,response))
      return;


   /* Set the integral gain */

   sprintf(dataout,"%x",drive->integral_gain);
   size(dataout,4);
   size_out=build_command(VEL_INTEG_GAIN_WRITE,dataout,command,drive->drive);
 
    if(!command_out(command,size_out,SERIAL_PORT,unit,response))
      return;

   /* Set set point on/off */
   strcpy(dataout,"");
   if(drive->set_point_mode==ON){
     size_out=build_command(SETPOINT_ON,dataout,command,drive->drive);
   }
   else{
     size_out=build_command(SETPOINT_OFF,dataout,command,drive->drive);
   }

   if(!command_out(command,size_out,SERIAL_PORT,unit,response))
     return;

/* Set the direction of the motor */
 strcpy(dataout,"");
 
 if(drive->direction==FORWARD)
  size_out=build_command(DIRECTION_WRITE_FORWARD,dataout,command,drive->drive);
 else
  size_out=build_command(DIRECTION_WRITE_REVERSE,dataout,command,drive->drive);


  if(!command_out(command,size_out,SERIAL_PORT,unit,response))
    return;


 }/* end if(init_motors) */


 if(reset_motors){
   size_out=build_command(DRIVE_ENABLE,dataout,command,drive->drive);
   if(!command_out(command,size_out,SERIAL_PORT,unit,response))
      return;
   size_out=build_command(RESET_DRIVE,dataout,command,drive->drive);
   if(!command_out(command,size_out,SERIAL_PORT,unit,response))
      return;
 }

 strcpy(dataout,"");
 
   /* start with enable/disable */
 if(drive->enable==ENABLE)
   size_out=build_command(DRIVE_ENABLE,dataout,command,drive->drive);
 else
   size_out=build_command(DRIVE_DISABLE,dataout,command,drive->drive);
 
 
 if(!command_out(command,size_out,SERIAL_PORT,unit,response))
   return;


 
 /* Set the velocity */
 strcpy(dataout,"");
 dpm_to_hex(drive->velocity,dataout,drive->drive);
 size_out=build_command(VELOCITY_SETPOINT_WRITE,dataout,command,drive->drive);
 
  if(!command_out(command,size_out,SERIAL_PORT,unit,response))
    return;

}

/**************************************************************************/
void  dpm_to_hex(float dpm_in, char *buff_in,int drive_in)
     /* convert the Degrees per minute value to an eight digit hex number */
{
  
  unsigned int rpm_int;
  float rpm_in;

#ifdef DEBUGMOTORS
  printf("dpm_to_hex: DPM in:%f  ",dpm_in);
#endif

  if(drive_in==AZIMUTH)
    rpm_in=dpm_in*AZ_RPD;    /* Speed in terms of motor rotations per minute*/
  else
    rpm_in=dpm_in*EL_RPD;    /* Speed in terms of motor rotations per minute*/
    
      /* Limit speed to maximum motor speed */
  if(drive_in==ELEVATION && (fabs(rpm_in)>MAX_RPM_EL)){
    if(rpm_in > 0) 
      rpm_in=MAX_RPM_EL;
    else
      rpm_in=(-1)*MAX_RPM_EL;
  }
  else if((fabs(rpm_in) > MAX_RPM_AZ) && (drive_in==AZIMUTH)){ 
    if(rpm_in > 0) 
      rpm_in=MAX_RPM_AZ;
    else
      rpm_in=(-1)*MAX_RPM_AZ;
  }

#ifdef DEBUGMOTORS
  printf("RPM out: %f drive: -%d-",rpm_in,drive_in);
#endif

  rpm_in*=ONE_RPM;    /* ONE_RPM is the number to get 1 RPM at motor */

  rpm_int=(int)rpm_in;
  sprintf(buff_in,"%x",rpm_int);
#ifdef DEBUGMOTORS
  printf("HEX out: %s \n",buff_in);
#endif

  size(buff_in,8);
  return;
}

/**************************************************************************/
void size(char *buffin,int length)
/* adjusts lenth of hex string for commands */
{
  int i,count;
  char buf[80];
  count=strlen(buffin);
  strcpy(buf,buffin);
  strcpy(buffin,"");
  for(i=0;i<(length-count);i++){
    buffin[i]='0';
    buffin[i+1]='\0';
  }
  strcat(buffin,buf);
  return;
}
/**************************************************************************/
void controller_response_error(char *command,char *drive_unit)
{

  printf("\nERROR READING CONTROLLER RETURN --%s-- \n",drive_unit);
  printf("   Command sent out was: %s\n",command);
  send_error_message=1;
}
/**************************************************************************/
int read_digital_inputs()
{
  char dataout[20],command[20],unit[20],temp[20];
  int  size_out,test_limits[10],i,j;


    strcpy(dataout,"00");
    strcpy(unit,"AZIMUTH");
    size_out=build_command(READ_DIGITAL_INPUT,dataout,command,AZIMUTH);
    if(!command_out(command,size_out,SERIAL_PORT,unit,temp))
      return(-1);
    /*printf("\n\nreturned from read_digital_inputs 00 AZ : -%s-\n",temp);*/
    if(strcmp(temp,"0008") == 0){
      test_limits[0]=0;
    }
    else{
      test_limits[0]=1;
    }
    strcpy(dataout,"01");
    strcpy(unit,"AZIMUTH");
    size_out=build_command(READ_DIGITAL_INPUT,dataout,command,AZIMUTH);
    if(!command_out(command,size_out,SERIAL_PORT,unit,temp))
      return(-1);
    /*printf("returned from read_digital_inputs 01 AZ : -%s-\n",temp);*/
    if(strcmp(temp,"0010") == 0)
      test_limits[1]=0;
    else
      test_limits[1]=1;
    
    /*    if(test_limits[0]==0 && test_limits[1]==0)
       printf("Azimuth limit setup is OK for tracking program.\n");
     else
       printf("Azimuth limit setup is NOT OK for tracking program!!!\n");
    */

    strcpy(dataout,"02");
    strcpy(unit,"AZIMUTH");
    size_out=build_command(READ_DIGITAL_INPUT,dataout,command,AZIMUTH);
    if(!command_out(command,size_out,SERIAL_PORT,unit,temp))
      return(-1);
    if(strcmp(temp,"0000") == 0)
      test_limits[2]=0;
    else
      test_limits[2]=1;

    strcpy(dataout,"03");
    strcpy(unit,"AZIMUTH");
    size_out=build_command(READ_DIGITAL_INPUT,dataout,command,AZIMUTH);
    if(!command_out(command,size_out,SERIAL_PORT,unit,temp))
      return(-1);
    if(strcmp(temp,"0000") == 0)
      test_limits[3]=0;
    else
      test_limits[3]=1;


    strcpy(dataout,"00");
    strcpy(unit,"ELEVATION");
    size_out=build_command(READ_DIGITAL_INPUT,dataout,command,ELEVATION);
    if(!command_out(command,size_out,SERIAL_PORT,unit,temp))
      return(-1);
    /*printf("returned from read_digital_inputs 00 EL : -%s-\n",temp);*/
    if(strcmp(temp,"0008") == 0)
      test_limits[4]=0;
    else
      test_limits[4]=1;

    strcpy(dataout,"01");
    strcpy(unit,"ELEVATION");
    size_out=build_command(READ_DIGITAL_INPUT,dataout,command,ELEVATION);
    if(!command_out(command,size_out,SERIAL_PORT,unit,temp))
      return(-1);
    /*printf("returned from read_digital_inputs 01 EL : -%s-\n",temp);*/
    if(strcmp(temp,"0010") == 0)
      test_limits[5]=0;
    else
      test_limits[5]=1;

    /*    if(test_limits[4]==0 && test_limits[5]==0)
       printf("Elevation limit setup is OK for tracking program.\n\n\n");
     else
       printf("Elevation limit setup is NOT OK for tracking program!!!\n\n\n");
    */
    strcpy(dataout,"02");
    strcpy(unit,"ELEVATION");
    size_out=build_command(READ_DIGITAL_INPUT,dataout,command,ELEVATION);
    if(!command_out(command,size_out,SERIAL_PORT,unit,temp))
      return(-1);
    if(strcmp(temp,"0000") == 0)
      test_limits[6]=0;
    else
      test_limits[6]=1;

    strcpy(dataout,"03");
    strcpy(unit,"ELEVATION");
    size_out=build_command(READ_DIGITAL_INPUT,dataout,command,ELEVATION);
    if(!command_out(command,size_out,SERIAL_PORT,unit,temp))
      return(-1);
    if(strcmp(temp,"0000") == 0)
      test_limits[7]=0;
    else
      test_limits[7]=1;

  j=0;
  for(i=0;i<8;i++)
    j+=test_limits[i];
  if(j>0)      
    return(1);
  else
    return(0);
}
/**************************************************************************/
void set_digital_inputs()
{
  char dataout[20],command[20],unit[20],temp[20];
  int  size_out;


 
    /*first disable the drive */
    strcpy(dataout,"");
    size_out=build_command(DRIVE_DISABLE,dataout,command,AZIMUTH);
    if(!command_out(command,size_out,SERIAL_PORT,unit,temp))
      return;

      strcpy(dataout,"000008");
      strcpy(unit,"AZIMUTH");
      size_out=build_command(WRITE_DIGITAL_INPUT,dataout,command,AZIMUTH);
      if(!command_out(command,size_out,SERIAL_PORT,unit,temp))
        printf("failed command WRITE_DIGITAL_INPUT - AZIMUTH - RE_ENABLE\n");

      strcpy(dataout,"010010");
      strcpy(unit,"AZIMUTH");
      size_out=build_command(WRITE_DIGITAL_INPUT,dataout,command,AZIMUTH);
      if(!command_out(command,size_out,SERIAL_PORT,unit,temp))
        printf("failed command WRITE_DIGITAL_INPUT - AZIMUTH - RE_ENABLE\n");
     printf("Set Azimuth limit switch monitoring to normal operating mode.\n");
 
        /*now re-enable the drive */
    strcpy(dataout,"");
    size_out=build_command(DRIVE_ENABLE,dataout,command,AZIMUTH);
    if(!command_out(command,size_out,SERIAL_PORT,unit,temp))
      return;

    /*first disable the drive */
    strcpy(dataout,"");
    size_out=build_command(DRIVE_DISABLE,dataout,command,ELEVATION);
    if(!command_out(command,size_out,SERIAL_PORT,unit,temp))
      return;

      strcpy(dataout,"000008");
      strcpy(unit,"ELEVATION");
      size_out=build_command(WRITE_DIGITAL_INPUT,dataout,command,ELEVATION);
      if(!command_out(command,size_out,SERIAL_PORT,unit,temp))
        printf("failed command WRITE_DIGITAL_INPUT - ELEVATION - RE_ENABLE\n");

      strcpy(dataout,"010010");
      strcpy(unit,"ELEVATION");
      size_out=build_command(WRITE_DIGITAL_INPUT,dataout,command,ELEVATION);
      if(!command_out(command,size_out,SERIAL_PORT,unit,temp))
        printf("failed command WRITE_DIGITAL_INPUT - ELEVATION - RE_ENABLE\n");

   printf("Set Elevation limit switch monitoring to normal operating mode.\n");


        /*now re-enable the drive */
    strcpy(dataout,"");
    size_out=build_command(DRIVE_ENABLE,dataout,command,ELEVATION);
    if(!command_out(command,size_out,SERIAL_PORT,unit,temp))
      return;

}
/**************************************************************************/
void  get_status()         /* read the drive parameters */
{
  char dataout[20],command[20],unit[20];
  int  size_out;

  /*get azimuth status first */
  strcpy(dataout,"");
  strcpy(unit,"AZIMUTH");
  size_out=build_command(POWERUP_STATUS,dataout,command,AZIMUTH);
  if(!command_out(command,size_out,SERIAL_PORT,unit,az_info.powerup_status))
    return;
  size_out=build_command(READ_FAULT_STATUS,dataout,command,AZIMUTH);
  if(!command_out(command,size_out,SERIAL_PORT,unit,az_info.fault_status))
    return;
 
  size_out=build_command(RUN_STATE,dataout,command,AZIMUTH);
  if(!command_out(command,size_out,SERIAL_PORT,unit,az_info.run_state))
    return;

  /*now get elevation status */
  strcpy(unit,"ELEVATION");
  size_out=build_command(POWERUP_STATUS,dataout,command,ELEVATION);
  if(!command_out(command,size_out,SERIAL_PORT,unit,el_info.powerup_status))
    return;
  size_out=build_command(READ_FAULT_STATUS,dataout,command,ELEVATION);
  if(!command_out(command,size_out,SERIAL_PORT,unit,el_info.fault_status))
    return;

  size_out=build_command(RUN_STATE,dataout,command,ELEVATION);
  if(!command_out(command,size_out,SERIAL_PORT,unit,el_info.run_state))
    return;
 
  /*
  printf("AZIMUTH-READ_FAULT_STATUS: -%s-\n",az_info.fault_status);
  printf("AZIMUTH-POWERUP_STATUS: -%s-\n",az_info.powerup_status);
  printf("AZIMUTH-RUN_STATE: -%s-\n",az_info.run_state);
  printf("ELEVATION-POWERUP_STATUS: -%s-\n",el_info.powerup_status);
  printf("ELEVATION-READ_FAULT_STATUS: -%s-\n",el_info.fault_status);
  printf("ELEVATION-RUN_STATE: -%s-\n",el_info.run_state);
  */
}
/**************************************************************************/
void print_status()
{
  get_status();
  /*first test for azimuth problems */

  test_fault(&az_info);
  printf("- AZIMUTH\n");
  test_powerup(&az_info);
  printf("- AZIMUTH\n");
  test_runstate(&az_info);  
  printf("- AZIMUTH\n");
  /*now test for elevation problems*/
  test_fault(&el_info);
  printf("- ELEVATION\n");
  test_powerup(&el_info);
  printf("- ELEVATION\n");
  test_runstate(&el_info);  
  printf("- ELEVATION\n");
  printf("\n\n");
}
/**************************************************************************/
void test_fault(Info_struct *drive)
{
  unsigned long int test=0,big;
  
  test=strtoul(drive->fault_status,NULL,16);
/*get around too big unsigned compile warning*/
  big=test/2;
  if(test==0){
     printf("No Faults  ");
     return;
  }
  if(test&1) /*bit 0*/
    printf("+24VDC Fuse Blown,  ");
  if(test&2) /*bit 1*/
    printf("+5VDC Fuse Blown,  ");
  if(test&4) /*bit 2*/
    printf("Encoder Power Fuse Blown,  ");
  if(test&8) /*bit 3*/
    printf("Motor Overtemperature Thermostat,  ");
  if(test&16) /*bit 4*/
    printf("IPM fault (overtemp/overcurrent,short circuit),  ");
  if(test&32) /*bit 5*/
    printf("Channel IM line Break,  ");
  if(test&64) /*bit 6*/
    printf("Channel BM line Break,  ");
  if(test&128) /*bit 7*/
    printf("Channel AM line Break,  ");
  if(test&256) /*bit 8*/
    printf("Bus Undervoltage,  ");
  if(test&512) /*bit 9*/
    printf("Bus Overvoltage,  ");
  if(test&1024) /*bit 10*/
    printf("Illegal Halt State,  ");
  if(test&2048) /*bit 11*/
    printf("Sub processor Unused Interrupt,  ");
  if(test&4096) /*bit 12*/
    printf("Main processor Unused Interrupt,  ");
  if(test&65536) /*bit 16*/
    printf("Excessive Average Current,  ");
  if(test&131072) /*bit 17*/
    printf("Motor Overspeed,  ");
  if(test&262144) /*bit 18*/
    printf("Excessive Following Error,  ");
  if(test&524288) /*bit 19*/
    printf("Motor Encoder State Error,  ");
  if(test&1048576) /*bit 20*/
    printf("Master Encoder State Error,  ");
  if(test&2097152) /*bit 21*/
    printf("Motor Thermal Protection,  ");
  if(test&4194304) /*bit 22*/
    printf("IPM Thermal Protection,  ");
  if(test&134217728) /*bit 27*/
    printf("Enabled with no Motor Selected,  ");
  if(test&268435456) /*bit 28*/
    printf("Motor Selection Not in Table,  ");
  if(test&536870912) /*bit 29*/
    printf("Personality Write Error,  ");
  if(test&1073741824) /*bit 30*/
    printf("Service Write Error,  ");
  if(big&1073741824) /*bit 31*/
    printf("CPU Communications Error,  ");

}
/**************************************************************************/

void test_runstate(Info_struct *drive)
{
  if(strcmp(drive->run_state,"FF")==0){
    printf("Drive Enabled and ready  ");
    return;
  }
  if(strcmp(drive->run_state,"00")==0){
    printf("Drive ready...NOT ENABLED  ");
    return;
  }     
  if(strcmp(drive->run_state,"01")==0)
    printf("+24VDC Fuse Blown,  ");
  if(strcmp(drive->run_state,"02")==0)
    printf("+5VDC Fuse Blown,  ");
  if(strcmp(drive->run_state,"03")==0)
    printf("Encoder Power Fuse Blown,  ");
  if(strcmp(drive->run_state,"04")==0)
    printf("Motor Overtemperature Thermostat,  ");
  if(strcmp(drive->run_state,"05")==0)
    printf("IPM fault (overtemp/overcurrent,short circuit),  ");
  if(strcmp(drive->run_state,"06")==0)
    printf("Channel IM line Break,  ");
  if(strcmp(drive->run_state,"07")==0)
    printf("Channel BM line Break,  ");
  if(strcmp(drive->run_state,"08")==0)
    printf("Channel AM line Break,  ");
  if(strcmp(drive->run_state,"09")==0)
    printf("Bus Undervoltage,  ");
  if(strcmp(drive->run_state,"0A")==0)
    printf("Bus Overvoltage,  ");
  if(strcmp(drive->run_state,"0B")==0)
    printf("Illegal Halt State,  ");
  if(strcmp(drive->run_state,"0C")==0)
    printf("Sub processor Unused Interrupt,  ");
  if(strcmp(drive->run_state,"0D")==0)
    printf("Main processor Unused Interrupt,  ");
  if(strcmp(drive->run_state,"11")==0)
    printf("Excessive Average Current,  ");
  if(strcmp(drive->run_state,"12")==0)
    printf("Motor Overspeed,  ");
  if(strcmp(drive->run_state,"13")==0)
    printf("Excessive Following Error,  ");
  if(strcmp(drive->run_state,"14")==0)
    printf("Motor Encoder State Error,  ");
  if(strcmp(drive->run_state,"15")==0)
    printf("Master Encoder State Error,  ");
  if(strcmp(drive->run_state,"16")==0)
    printf("Motor Thermal Protection,  ");
  if(strcmp(drive->run_state,"17")==0)
    printf("IPM Thermal Protection,  ");
  if(strcmp(drive->run_state,"1C")==0)
    printf("Enabled with no Motor Selected,  ");
  if(strcmp(drive->run_state,"1D")==0)
    printf("Motor Selection Not in Table,  ");
  if(strcmp(drive->run_state,"1E")==0)
    printf("Personality Write Error,  ");
  if(strcmp(drive->run_state,"1F")==0)
    printf("Service Write Error,  ");
  if(strcmp(drive->run_state,"20")==0)
    printf("CPU Communications Error,  ");

}
/**************************************************************************/

void test_powerup(Info_struct *drive)
{

  if(strcmp(drive->powerup_status,"00")==0){
    printf("Successful Power-Up  ");
    return;
  }
  if(strcmp(drive->powerup_status,"33")==0)
    printf("Program Memory Boot Block Error,  ");
  if(strcmp(drive->powerup_status,"34")==0)
    printf("Program Memory Main Block Error,  ");
  if(strcmp(drive->powerup_status,"35")==0)
    printf("Uninitialized Personality EEPROM Error,  ");
  if(strcmp(drive->powerup_status,"36")==0)
    printf("Personality EEPROM Read Error,  ");
  if(strcmp(drive->powerup_status,"37")==0)
    printf("Personality EEPROM Data Corruption Error,  ");
  if(strcmp(drive->powerup_status,"38")==0)
    printf("Main processor Watchdog Error,  ");
  if(strcmp(drive->powerup_status,"39")==0)
    printf("Sub processor Watchdog Error,  ");
  if(strcmp(drive->powerup_status,"3A")==0)
    printf("Main processor RAM Error,  ");
  if(strcmp(drive->powerup_status,"3B")==0)
    printf("Sub processor RAM Error,  ");
  if(strcmp(drive->powerup_status,"3C")==0)
    printf("Uninitialized Service EEPROM Error,  ");
  if(strcmp(drive->powerup_status,"3D")==0)
    printf("Service EEPROM Read Error,  ");
  if(strcmp(drive->powerup_status,"3E")==0)
    printf("Service EEPROM Data Corruption Error,  ");
  if(strcmp(drive->powerup_status,"3F")==0)
    printf("Main processor A/D Converter Error,  ");
  if(strcmp(drive->powerup_status,"40")==0)
    printf("Sub processor A/D Converter Error,  ");
  if(strcmp(drive->powerup_status,"41")==0)
    printf("ANALOG1 Output Error,  ");
  if(strcmp(drive->powerup_status,"42")==0)
    printf("Gate Array Error,  ");
  if(strcmp(drive->powerup_status,"43")==0)
    printf("ANALOG2 Output Error,  ");
  if(strcmp(drive->powerup_status,"44")==0)
    printf("Inter-processor Communication Error,  ");
  if(strcmp(drive->powerup_status,"45")==0)
    printf("Sub processor Initialization Error,  ");
  if(strcmp(drive->powerup_status,"46")==0)
    printf("Sub processor SRAM Error,  ");
  if(strcmp(drive->powerup_status,"47")==0)
    printf("Sub processor Code Loading Error,  ");
  if(strcmp(drive->powerup_status,"48")==0)
    printf("Sub processor Startup Error,  ");
  if(strcmp(drive->powerup_status,"49")==0)
    printf("Sub processor Checksum Error,  ");
  if(strcmp(drive->powerup_status,"4A")==0)
    printf("Personality EEPROM Write Error,  ");
  if(strcmp(drive->powerup_status,"4B")==0)
    printf("Service EEPROM Write Error,  ");
  if(strcmp(drive->powerup_status,"4C")==0)
    printf("Software Clock Error,  ");
  if(strcmp(drive->powerup_status,"4D")==0)
    printf("Sub processor Communication Checksum Error,  ");
  if(strcmp(drive->powerup_status,"4E")==0)
    printf("Sine Table Generation Error,  ");
  if(strcmp(drive->powerup_status,"4F")==0)
    printf("Personality Data Out Of Range Error,  ");
  if(strcmp(drive->powerup_status,"50")==0)
    printf("Service Data Out Of Range Error,  ");
  if(strcmp(drive->powerup_status,"51")==0)
    printf("Motor Block Checksum Error,  ");
  
}

/**************************************************************************/

int  get_position(Info_struct *drive) /* read the drive motor positions*/
{
  char dataout[20],command[20],unit[20],response[20];
  int  size_out,number_in;

  /*get azimuth position first */
  strcpy(dataout,"");
  if(drive->drive==AZIMUTH)
   strcpy(unit,"AZIMUTH");
  else 
   strcpy(unit,"ELEVATION");
  
  size_out=build_command(MOTOR_POSITION,dataout,command,drive->drive);
  if(!command_out(command,size_out,SERIAL_PORT,unit,response))
    return(0);
  
  sscanf(response,"%x",&number_in);
  drive->motor_position=number_in;
  /*
   printf("%s MOTOR_POSITION=%s -> %d",unit,response,number_in);
  */
  return(number_in);
  

}
/**************************************************************************/
void init_info_structure(void) /* Initializes drive info structures */
{

  extern Info_struct az_info,el_info;

  strcpy(az_info.name,"AZIMUTH");
  strcpy(el_info.name,"ELEVATION");
  az_info.velocity=0.0;
  el_info.velocity=0.0;
  az_info.last_velocity=0.0;
  el_info.last_velocity=0.0;
  az_info.drive=AZIMUTH;
  el_info.drive=ELEVATION;
  az_info.enable=ENABLE;
  el_info.enable=ENABLE;
  az_info.direction=FORWARD;
  el_info.direction=FORWARD;
  az_info.last_direction=CW;
  el_info.last_direction=UP;
  az_info.set_point_mode=ON;
  el_info.set_point_mode=ON;
  az_info.derivative_gain=AZ_D_GAIN;
  az_info.integral_gain=AZ_I_GAIN;
  az_info.proportional_gain=AZ_P_GAIN;
  el_info.derivative_gain=EL_D_GAIN;
  el_info.integral_gain=EL_I_GAIN;
  el_info.proportional_gain=EL_P_GAIN;

  strcpy(el_info.run_state,"");     
  strcpy(az_info.run_state,"");     
  strcpy(el_info.fault_status,"");   
  strcpy(az_info.fault_status,"");   
  strcpy(el_info.powerup_status,"");
  strcpy(az_info.powerup_status,"");

  az_info.motor_position=0;
  el_info.motor_position=0;

  TOO_MANY_ERRORS=0;
}
