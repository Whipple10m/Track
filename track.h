#ifndef TRACK_H
#define TRACK_H

#define VERSION "Track V1.91"
/* Defines */
/* for program debugging 1=DEBUG MODE ... No output to controllers*/
/*#define DEBUG 1*/             /* Defined does general debug */
/*#define DIRECTION_BIT*/   /* CW/CCW switch test */
/*#define DEBUGSCAN 1*/     /* Defined shows scaning info */
/*#define DEBUGINPUT*/      /* Defined shows input debug info */
/*#define DEBUGMOTORS*/
#define COMMANDOUT    /* Defined allows commands to motors */
/*#define DEBUGCOMMANDS*/ 
/*#define DEBUGDRIFT*/
/*#define DEBUG_DIGITAL_INPUT*/ /* digital inputs from CIODIO card */
/*#define DEBUGTELSTATUS*/
/*#define DEBUG_MOVEMENT*/
/*#define BACKLASH*/

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h> 
#include <asm/io.h>
#include <time.h>
#include <sys/timeb.h>
#include <sys/time.h>
#include <unistd.h>
#include <math.h>
#include <sys/ioctl.h>
#include <strings.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <string.h>
#include <sys/stat.h>
#include <dio48.h>
#include "/usr/local/include/forms.h"

/* General Defines */
#define BYTE unsigned char
#define WORD unsigned short
#define LONG unsigned long


/* Encoder values and telescope offsets */
#define ENCODER_MAX 32767    /* Max count of 15 bit encoder */
/*changing offsets 990411 using MRK421 with NO aperture! high current was
  in tube 6 ...old values AZ=-269.131 EL=-10.5*/
/* EL higher number is lower elevation.  AZ lower number is clockwise */
#define AZOFFSET -269.150 /* Encoder offsets */
#define ELOFFSET -10.300 /*-10.380*/ 
#define LOW_EL 16.0 

/*new to tracking program */
#define EL_GAIN 30
#define EL2_GAIN 10
#define AZ_GAIN 30
#define AZ2_GAIN 10

        
#define ENCODER_TO_DEG 360/ENCODER_MAX

#define SLIDER_RANGE_MIN -2.0    /* Slider used in pointing corrections */
#define SLIDER_RANGE_MAX 2.0
#define FOREVER     1
#define TRUE    1
#define RTOD    57.295779        /* Radians to degrees */
#define TCONA   6.57098244e-2    /* Used to calculate siderial time */
#define LONGI   7.39183332       /* Site longitude */
#define LAT     0.55301242       /* Site latitude  */
#define INTERVAL 100             /* number of milliseconds between updates */
/* Short function defines */
#define RANGE(v1,v2) ((v1)-(v2)*floor((v1)/(v2)))

/* defines for checking status and limit bits from the mount */
#define EL_DOWN 1        /*limit_bits*/
#define EL_UP 2          /*limit_bits*/
#define EL_15 4          /*limit_bits*/
#define AZ_CCW 8         /*limit_bits*/
#define AZ_CW 16         /*limit_bits*/
#define WINDOW 32        /*limit_bits*/
#define EL_OK 1          /*status_bits*/
#define EL_AT_SPEED 2    /*status_bits*/
#define AZ_OK 4          /*status_bits*/
#define AZ_AT_SPEED 8    /*status_bits*/
#define CW_CCW 16        /*status_bits*/
#define HT_ON 32         /*status_bits*/
#define EL_AUTO 64       /*status_bits*/
#define AZ_AUTO 128      /*status_bits*/



/* scan default parameters */
#define MOON_SCAN_RADIUS 38.0        /* 38 mrads from moon is default */
#define SCAN_RADIUS    5.0           /* 5 degree circle to scan inside */
#define SCAN_WIDTH     1.0           /* 1 degree increments in the sweep */
#define TIMER          400.0         /* 400 second scan time through circle */
#define MOON_TIMER     600.0         /* 600 second scan time through circle */
#define X_CENTER       430/2         /* center of x on x-y plot */
#define Y_CENTER       340/2         /* center of y on x-y plot */
#define STEPS_PER_SECOND 3           /* adjust scan x-y 3 times per second */

/*Burst tracking defines */
#define BURST_ERROR_5SCAN   2.0
#define BURST_ERROR_MAX     15.0
#define BURST_MIN_INTENSITY 500


/****************************************************************/
/*DEFINES FOR TPOINT CORRECTION ROUTINE START HERE */
/****************************************************************/
#define IA    +304.09905     /*+664.9905*/
#define IE    -255.803       /*+32.1971*/
#define CA    +0.0
#define AN    -242.1436
#define AW    -23.5030
#define ACEC  +127.3120
#define ACES  +309.8698
#define TX    -171.8020
#define ECEC  -272.6916
#define ECES  +0.0
#define NPAE  +0.0


/****************************************************************/
/*DEFINES FOR TPOINT CORRECTION ROUTINE END HERE */
/****************************************************************/


/****************************************************************/
/*DEFINES FOR VVV CORRECTION ROUTINE START HERE */
/****************************************************************/

/* DEFINE STATEMENTS */
#define  XI     0.0 /*0.192*/  /* tilt of the azimuth and elevation axes [deg] */
#define  THETA  0.0 /*0.037*/  /* tilt of the azimuth rotation axis      [deg] */
#define  PHI   0.0 /*-0.283*/  /* telescope azimuth offset               [deg] */
#define  PSI   -7.402  /* focal plane r.f. rotation angle        [deg] */
#define  DX    0.0 /* 0.191*/  /* reflector alignment X-offset           [deg] */
#define  DZ    0.0 /* 0.013*/  /* reflector alignment Z-offset           [deg] */


/* STRUCTURE DECLARATIONS */
typedef struct pmt{
        double x;         /* pmt x coordinate on the focal plane    [deg] */
        double z;         /* pmt z coordinate on the focal plane    [deg] */
        double r;         /* pmt photocathode radius                [deg] */
        double c;         /* pmt cone size (side to side hexagon)   [deg] */
        double g;         /* pmt conversion factor                [dc/pe] */
    } pmt;

typedef struct vector {
    double x;
	double y;
	double z;
    } vector;

#define pi  3.1415926535897932384626433832795
#define conversion pi/180.

int pmt_to_track=1;

/* FUNCTION DECLARATIONS:  Private */
static vector Rx(double theta, vector v);
static vector Rz(double phi,   vector v);
static vector Ry(double xi,    vector v);
static void focalplane_to_telescope(double X, double Z, vector *v);
static void telescope_to_focalplane(double *x, double *z, vector v);



/* GLOBAL VARIABLES */
extern double xi_o   =XI;
extern double theta_o=THETA;
extern double phi_o  =PHI;
extern double psi_o  =PSI;
extern double dx_o   =DX;
extern double dz_o   =DZ;



double fb_X=0.0,fb_Z=0.0;

double az_deg_error=0.0,el_deg_error=0.0;

FILE *fpkh;

#define NPMT 490          /* number of pmts in the camera             [1] */




/* FUNCTION DECLARATIONS:  Public */
extern void correction(double *d_theta, double *d_phi, 
        double theta_t, double phi_t, double x_t, double z_t, int i_pmt);

extern void spherical_to_focalplane(
        double theta_t, double phi_t, double x_t, double z_t,
        double theta  , double phi  , double *x  , double *z);

extern void focalplane_to_spherical(
        double theta_t, double phi_t, double x_t, double z_t,
        double *theta  , double *phi  , double x  , double z);

extern double pointing_error(
        double theta_t, double phi_t, int i_pmt, 
        double d_theta, double d_phi);

extern pmt pmt_data(int i);



/****************************************************************/
/*DEFINES FOR VVV CORRECTION ROUTINE END HERE */
/****************************************************************/


/* Command menu list */
extern char menustr[200];

extern char current_message[4][80];
extern char error_win_messages[4][80];
extern int which_instr;
extern int ct, azi, eli;
extern char *progname;


struct source_stuff
{
  char   name[30];
  char   source_code[10];
  double rao;
  double deco;
  double epocho;
  double epochp;
  double rap;
  double decp;
  double az;
  double el;
  int    visible;
  double set_time;
  double set_az;
};

typedef struct source_stuff Source;
extern Source *source_list[150];  /* Define pointer array to structures */
extern int sources;                    /* number of sources read in */
extern int position[100];                /* keep track of displayed sources */

struct source_moon
{
  float  moon_time;
  double ra_center;
  double dec_center;
  double ra_proton;
  double dec_proton;
  double ra_top;
  double dec_top;
  double ra_anti;
  double dec_anti;
  double ra_bottom;
  double dec_bottom;
};

extern struct source_moon *moon_list[100];  /* Define pointer array to structures */

/* parameters dealing with scanning across an object */
struct scan           
{
  float  radius;
  float  scan_width;
  float  total_time;
  float  start_time;
  float  elapsed_time;
  float  last_elapsed_time;
  int    odd;
  int    passes;
  int    step_count;
  int    total_steps;
  float  corners[200];
};

extern struct scan scan_data;

extern float plot_x[5000];
extern float plot_y[5000];
extern float circle_x[101];
extern float circle_y[101];

struct source_burst
{
  char   name[80];
  double epoch;
  double ra_center;
  double dec_center;
  double ra_ahead;
  double dec_ahead;
  double ra_top;
  double dec_top;
  double ra_behind;
  double dec_behind;
  double ra_bottom;
  double dec_bottom;
};

extern struct source_burst *burst_list[5]; /* Define pointers to structures */
extern int burst_track,burst_number;

/* Define a structure for doing telescope corrections */
struct star_corrections
{
  char name[30];
  int list_number;
  double ra;
  double dec;
  double az;
  double el;
  double offset_az; /* amount az and el needed to be changed to put the */
  double offset_el; /* star on the center tube */
  double siderial;
};

extern struct star_corrections *correction_list[50];
extern int corrections_done; /* signal that corrections are finished */

struct flags_list
{
  int  freeze;
  int  track_moon;       /* indicator of moon tracking routine */
  int  moon_switch;      /* indicator of ON-OFF moon switch */
  int  pointing_check;   /* variable to interrupt pointing check*/
  int  drift_mode;       /* variable to interrupt drift and home modes */
  int  drift_flag;       /* flag to indicate object position does not need to
			    be updated. */
  int  on_source;        /* on or off source ? */
  int  burst_mode;
  int  scan_mode;
  int  scan_mode_run;
  int  doing_corrections;
  int  more_motor_steps;
};

extern struct flags_list flags;

/* Limit switch and telescope parameter structure */
struct limit_switches
{
  int  cw_enable;
  int  ccw_enable;
  int  up_enable;
  int  down_enable;       
  int  high_voltage_on;   /* HT status indicator */
  int  cw_rotation;       /* CW/CCW indicator switch */
  int  az_auto;           /* auto/manual mode */
  int  el_auto;           /* auto/manual mode */
  int  az_window;         /* 2.5>AZ<357.5 */
  int  el_15;             /* elevation less than 15 degrees */
  int  el_drive_ready;    /* indicates all is OK with controller */
  int  az_drive_ready;    /* indicates all is OK with controller */
  int  el_at_speed;       /* motor speed is as set */
  int  az_at_speed;       /* motor speed is as set */
  int  az_enable;         /* if both directions are disabled and el>20 then
			     the azimuth disable switch has been set */
  int  el_enable;         /* if both directions are disabled then the elevation
			     disable switch has been set */
  int  too_low;           /* Elevation below 15 degrees outside window */
};

extern struct limit_switches status;

extern char check_status_message[30][160];

/* Time variables */
extern double ut, lastut, st, gmsto,st_diff;
extern char ststr[10], utstr[10], datstr[10];
extern int ut_day,utdd;
extern int ut_month,utmo;
extern int ut_year,utyy;
extern int ut_hour;
extern int ut_sec;
extern int ut_min;
extern int local_year,local_month,local_day,local_hour;

/* used to time RA and DEC renewals while tracking moon */
extern float moon_time;
extern int sec;           /* used to time the display update*/

/* Object Position default settings are POLARIS*/
extern double ra, dec, objel, objaz, temp_ra,temp_dec,temp_sra;
extern double sra, sdec, pra, pdec;
extern char rastr[10], decstr[10];
extern double temp_sdec,temp_pra,temp_pdec,tempel,tempaz;

enum moon_pos
{
  center=CENTER, top=TOP, bottom=BOTTOM, proton=PROTON, anti_proton=ANTI_PROTON
};

typedef enum moon_pos Moon_Pos;
extern enum moon_pos moon_position;


extern int  off_moon;     /* indicates off source tracking of moon */

enum burst_pos
{
  center_b=CENTER, top_b=TOP, bottom_b=BOTTOM, ahead_b=AHEAD, behind_b=BEHIND
};

typedef enum burst_pos Burst_Pos;
extern enum burst_pos burst_position;

enum slewtrack
{
  off=0, no=0, on=1, yes=1
};

extern enum slewtrack slewsta;
extern enum slewtrack track;

extern int offset,wagg;
extern char id[20], id_code[10], mode[80];
extern char temp_id[20], temp_code[10];

/* Telescope control */
extern int limit_set, az_direction;
extern double telel, telaz, lastaz, lastel;
extern double old_telaz,old_telel;
extern int direction_alert_sent;
extern float az_speed[11],el_speed[11];
extern int burst_multiplier;

/* Telescope position */
extern double  tdist, teldisp, azzero, elzero;
extern int     tdist_error,CIO_1,CIO_2;
extern BYTE  status_bits,limit_bits;

/* Error signal variables (1=ERROR) */
extern int el_error,az_error,RA_error,DEC_error;
extern int send_error_message;
/* ERROR messages to be sent out if limits are hit */
extern char az_limit_message_cw[80];
extern char az_limit_message_ccw[80];
extern char el_upper_limit_message[80];
extern char el_lower_limit_message[80];
extern char too_low_limit_message[80];

/* Global variables for sending info to VHEGRO */
extern int send_time;  /* Do not write the file EVERY cycle...every 10 cycles*/
extern int write_cycles;/* How many times file writen. Is a new file?*/

enum trackmode
{
  ONSOURCE=1,OFFSOURCE=2, SLEWING=3, STANDBY=4, ZENITH=5, POINTCHECK=6,
  STOW=7, DRIFTSCAN=8, BURSTMODE=9, MOONTRACK=10
};

typedef enum trackmode Trackmode;
extern enum trackmode mode_int;

extern int status_code;      /* anything other than zero is an error */


/* Global variables for receiving commands from VHEGRO */
extern int auto_mode;   /* Is program in AUTO (REMOTE) command mode? 1=yes */
extern int receive_time; /* Limits whenthe file is read...#'s of loop.*/

/* Global variables for corrections */
extern int    doing_corrections;  /* set to 1 by correction routine */
extern double cos_alpha, sin_alpha;
extern double cos_beta, sin_beta;
extern double cos_gamma, tan_gamma;
extern double delta;
extern double az_cor_offset, el_cor_offset; /* pointing correction routine */
extern int    total_correction_entries;

/* Misc variables */
extern int  write_data;
extern char datstat[10];
extern FILE *df,*fpc,*fpscan;

/*main globals for tracking forms*/
extern FD_track_form *fd_track_form;
extern FD_browser_form *fd_browser_form;
extern FD_correct_form *fd_correct_form;
extern FD_moon_form *fd_moon_form;
extern FD_burst_form *fd_burst_form;
extern FD_scan_form *fd_scan_form;
extern FD_message_form *fd_message_form;
extern FD_notice_form *fd_notice_form;



/* Function declarations */
extern void place_display();        /* Place the information in the main window */
extern void clear_message();        /* Clear message from screen */
extern void place_message();        /* Write message to screen */
extern void place_data();           /* Writes data to display window */
extern void send_to_VHEGRO();       /* Write tracking information to file for VHEGRO*/
extern void setconb();              /* Sets the siderial constant acording to year*/
extern int  gettime();              /* Gets the siderial time */
extern void azimuth_direction();    /* Checks the direction of initial AZ motion */
extern int  azandel();              /* CALCULATES OBJECT'S AZ. AND EL. returns 0 if
                                object is below horizon*/
extern void datain();               /* GETS THE TELESCOPE POSITION DATA FROM MOUNT
                                INTERFACE HARDWARE */
extern void telstatus();       /* DECIDES WHERE TO MOVE THE TELESCOPE */
extern void update();          /* UPDATES TELESCOPE POSITION ON SCREEN */
extern void stoptel();         /* STOPS THE TELESCOPE */
extern int  ftod();            /* CONVERT DDMMSS.S TO DECIMAL DD */
extern int  ftohms();          /* CONVERT DECIMAL DD TO STRING */
extern void getobj();          /* PRINT LIST OF OBJECTS AND GET CHOICE */
extern void gettel();          /* GET CURRENT AZ. & EL. OF TELESCOPE */
extern void waggel();          /* Toggle Waggle mode tracking */
extern void drift_tel();       /* Drift Scan mode */
extern int  compare();         /* FOR QUICKSORT */
extern void to2000() ;         /* Precess ra & dec from J2000 */
extern void from2000() ;       /* Precess ra & dec to J2000 */
extern double raval();         /* For precession */
extern double decval();        /* For precession */
extern int  wait_msecs();      /* Wait for given # milliseconds */
extern void limit_print();     /* Print limit condition & stop tel */
extern void menu_setup();      /* Set up the menu commands that are listed */
extern void star_corrections();/* pointing checks to create new corrections */
extern float update_moon();    /* Gets new RA and DEC of moon while tracking */
extern void moon_rotate();     /* Move telescope to next moon position*/
extern int  get_moon_file();   /* Read in file for moon positions */
extern void burst_input();     /* Check for burst and respond if there */
extern void save_burst();      /* Save burst values to a file for later */
extern void getburst();        /* Get an existing burst from a file */
extern void flag_alert();      /* Notify operator of errors (flags)*/
extern void scale_form();      /* change size of tracking window */
extern void scan_setup();      /* set up variables for the scan mode*/
extern void continue_scan();   /* called to update info while in scan mode */
extern void moon_scan_setup(); /* set up variables for the moon scan mode*/
extern void initialize_cio();  /* initialize the i/o card */
extern int  check_status();    /* check status of limit switches + others */
extern int  gb();              /* Convert grey code to binary */
extern void printbin();        /* print a binary number (trouble shooting) */
extern void split();           /* part of the data input routines */
extern void dataout();         /* chacks and sends data to motors */
extern void initialize_variables(); /*AS it says*/
extern void tpoint_correction(); /* corrections using TPOINT values */
extern void find_backlash();     /*routine to find azimuth backlash */
#endif
