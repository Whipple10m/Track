#ifndef CONTROLLER_H
#define CONTROLLER_H


#define DISABLE 0
#define ENABLE  1
#define START   1
#define STOP    0
#define FORWARD 0
#define REVERSE 1
#define AZIMUTH 0
#define ELEVATION 1
#define ONE_RPM 65359.48      /* Number that makes motors move one RPM */
#define MOTOR_ENCODER 5000    /* 5000 line encoders */
#define VELOCITY_RESOLUTION   1.525878e-05  /*Controller resolution- velocity*/
#define ON      1
#define OFF     0

/* Setup control for 10 meter tracking */
#define AZGEARS      15098.0  /* Motor rotations per telescope rotation */
#define ELGEARS      25435.0  /* Changed EL motor gear 40506.0 */  
#define AZ_RPD     AZGEARS/360.0 /* Motor rotations per degree of movement*/
#define EL_RPD     ELGEARS/360.0 /* Motor rotations per degree of movement*/
#define ZERO         0.0
#define MAXSLEW      70.0     /* MAX telescope speed Degrees per minute */
#define MAX_RPM_EL      2800.0   /* MAX motor RPM ELEVATION*/
#define MAX_RPM_AZ	2500.0   /* MAX motor RPM AZIMUTH */
#define CW      0
#define CCW     1
#define UP      1
#define DOWN    0
#define TRACKGAIN 4
/* define motor velocity gains */
#define AZ_D_GAIN  0
#define AZ_I_GAIN  5
#define AZ_P_GAIN  60
#define EL_D_GAIN  0
#define EL_I_GAIN  10
#define EL_P_GAIN  60


/* controller commands */
#define DRIVE_ENABLE		"12201"
#define DRIVE_DISABLE		"12200"
#define CHANGE_DIRECTION_READ	"1AC"
#define DIRECTION_WRITE_FORWARD	"1AD00"
#define DIRECTION_WRITE_REVERSE	"1AD01"
#define DRIVE_MODE_WRITE	"0F5"
#define VEL_DERIV_GAIN_READ	"04E"
#define VEL_DERIV_GAIN_WRITE	"04F"
#define VEL_INTEG_GAIN_READ	"04C"
#define VEL_INTEG_GAIN_WRITE	"04D"
#define VEL_PROP_GAIN_READ	"04A"
#define VEL_PROP_GAIN_WRITE	"04B"
#define VELOCITY_SETPOINT_READ	"125"
#define VELOCITY_SETPOINT_WRITE	"126"
#define SETPOINT_ON             "12A01"
#define SETPOINT_OFF            "12A00"
#define RESET_DRIVE             "120"
#define RESET_FAULTS            "12B"
#define RESET_COMMAND_SOURCE    "0F303"  /* sets the command back to manual*/
#define COMMAND_SOURCE_ENCODER  "0F302"
#define SET_ENCODER_LINES_5000  "0931388"
#define READ_ENCODER_LINES      "092"
#define SET_INDEX_OFFSET        "0B1"
#define READ_INDEX_OFFSET       "0B0"
#define READ_GEAR_RATIO         "042"
#define WRITE_GEAR_RATIO        "043"
#define POSITION_ERROR          "148"
#define READ_DIGITAL_INPUT      "0C0"   /*Used to set limit switch monitoring*/
#define WRITE_DIGITAL_INPUT     "0C1"

/* data commands */
#define TRIGGER_MODE_NOW        "16900"
#define TRIGGER_MODE_POS        "16901"
#define TRIGGER_MODE_NEG        "16902"
#define TRIGGER_THRESHOLD       "16B"      /*needs 4 digits following */
#define TIMEBASE                "167"      /*needs 4 digits following */
#define TRIGGER_MOTOR_VELOCITY  "16506"
#define TRIGGER_COMMAND_VELOCITY  "16507"
#define DATA_SOURCE_1_VELOCITY  "16106"
#define DATA_SOURCE_2_VELOCITY  "16307"    /* command velocity */
#define ARM_DATA                "16C"
#define TRIGGER_STATUS          "16D"
#define COLLECT_DATA            "16E"      /* needs 2 digits for arrays */
#define MANUAL_TUNE_VELOCITY    "110"
#define MANUAL_TUNE_PERIOD      "10E"
#define OPERATING_MODE_NORMAL   "10300"
#define OPERATING_MODE_MANUAL   "10302"
#define READ_FAULT_STATUS       "135"
#define RUN_STATE               "136"
#define POWERUP_STATUS          "001"
#define MOTOR_POSITION          "145"
#define MOTOR_INDEX_POSITION    "116"
#define MASTER_POSITION         "146"
#define MASTER_INDEX_POSITION   "117"
#define READ_POSITION_ERROR     "148"

extern int SERIAL_PORT;  /* This is the serial port used for the controllers */
extern int TOO_MANY_ERRORS;

struct info
{
  char   name[30];          /* Name of drive */
  float  velocity;          /* Current velocity setting */
  float  last_velocity;     /* only send command when a change in velocity */
  int    drive;             /* Drive number 0-255 */
  int    enable;            /* ENABLE or DISABLE */
  int    direction;         /* FORWARD or REVERSE (UP,DOWN,CW,CCW)*/
  int    last_direction;    /* only send direction command when changed */
  int    set_point_mode;    /* movement mode of controllers */
  int    derivative_gain;   /* Gains for velocity control */
  int    integral_gain;
  int    proportional_gain;
  char   run_state[20];      /* bits indicate controller and motor condition */
  char   fault_status[20];   /* bits indicate controller faults */
  char   powerup_status[20]; /* bits indicate controller powerup status */
  int    motor_position;     /* motor encoder counts */
  int    motor_encoder_zero; /* encoder value at relative zero for alignment*/
                /*motor encoder counts from last absolute encoder bit change */
  int    mirror_alignment_offset;
                /* absolute encoder value for mirror alignment position */
  double mirror_alignment_position;
  int    digital_input[4];   /* digital input setup bits 0-3 */
};

typedef struct info Info_struct;
extern struct info az_info;
extern struct info el_info;

struct controller_info
{
  /* controller status information in here */

};

extern struct controller_info az_controller_info;
extern struct controller_info el_controller_info;

/* subroutine declarations */
extern void init_info_structure(); /* Initializes drive info structures */
extern void setup_motors();   /* Uses info structure -sets up controllers*/
extern int   tty_set();           /* Sets up serial port */
extern char  readchar();          /* gets character from serial port */
extern int   get_response();      /* gets a string from the serial port */
extern int   checksum();          /* determines if checksum was OK */
extern int   find_checksum();     /* determines the checksum for a command */
extern int   command_out();       /* sends a command to a controller */
extern int   build_command();     /* builds the controller command */
 /* Converts Degrees per minute to hex value used in the command */
extern void  dpm_to_hex(float, char *,int);
extern void  stop_motors();       /* Sets both motors to zero velocity */
extern void  move_motor();          /* turn the motor on at the set RPM */
extern void  size();
extern void  get_status();         /* read the drive parameters */
extern void  controller_response_error(); /*print error message*/
extern int   get_position();       /* read motor encoder & index positions*/
int   read_digital_inputs(); 
void  set_digital_inputs();  /* used to set limit switch ignore */
void print_status();  /* print status of motor controllers */
void test_fault();
void test_runstate(); 
void test_powerup();

#endif
