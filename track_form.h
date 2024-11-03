/** Header file generated with fdesign on Wed Mar 29 14:44:47 2000.**/

#ifndef FD_track_form_h_
#define FD_track_form_h_

#define CENTER  0
#define TOP     1
#define PROTON  2
#define AHEAD   2
#define BOTTOM  3
#define ANTI_PROTON    4
#define BEHIND  4
#define OFFSET  5

/** Callbacks, globals and object handlers **/
extern void change_ra(FL_OBJECT *, long);
extern void change_dec(FL_OBJECT *, long);
extern void get_object(FL_OBJECT *, long);
extern void change_mode(FL_OBJECT *, long);
extern void commands(FL_OBJECT *, long);
extern void change_offset(FL_OBJECT *, long);
extern void change_first_source(FL_OBJECT *, long);
extern void change_drift_mode(FL_OBJECT *, long);
extern void change_slew(FL_OBJECT *, long);
extern void point_check(FL_OBJECT *, long);

extern void choose_object(FL_OBJECT *, long);

extern void write_offset(FL_OBJECT *, long);
extern void correction_exit(FL_OBJECT *, long);
extern void az_adjust(FL_OBJECT *, long);
extern void el_adjust(FL_OBJECT *, long);
extern void Z_adjust(FL_OBJECT *, long);
extern void X_adjust(FL_OBJECT *, long);
extern void new_tube(FL_OBJECT *, long);

extern void move_moon(FL_OBJECT *, long);
extern void exit_moon(FL_OBJECT *, long);

extern void burst_move(FL_OBJECT *, long);
extern void burst_exit(FL_OBJECT *, long);

extern void stop_scan(FL_OBJECT *, long);
extern void start_scan(FL_OBJECT *, long);
extern void exit_scan(FL_OBJECT *, long);




/**** Forms and Objects ****/
typedef struct {
	FL_FORM *track_form;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *sid_time;
	FL_OBJECT *elobj;
	FL_OBJECT *raobj;
	FL_OBJECT *decobj;
	FL_OBJECT *objid;
	FL_OBJECT *diff;
	FL_OBJECT *mode;
	FL_OBJECT *commands;
	FL_OBJECT *offset;
	FL_OBJECT *first_source;
	FL_OBJECT *azobj;
	FL_OBJECT *aztel;
	FL_OBJECT *eltel;
	FL_OBJECT *offsource;
	FL_OBJECT *onsource;
	FL_OBJECT *pointcheck;
	FL_OBJECT *slew;
	FL_OBJECT *code;
	FL_OBJECT *ut_time;
	FL_OBJECT *ut_date;
} FD_track_form;

extern FD_track_form * create_form_track_form(void);
typedef struct {
	FL_FORM *browser_form;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *get_object;
} FD_browser_form;

extern FD_browser_form * create_form_browser_form(void);
typedef struct {
	FL_FORM *correct_form;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *write_offset_button;
	FL_OBJECT *correction_exit_button;
	FL_OBJECT *correction_number;
	FL_OBJECT *az_slider;
	FL_OBJECT *el_slider;
	FL_OBJECT *Z_slider;
	FL_OBJECT *X_slider;
	FL_OBJECT *pmt_tube;
} FD_correct_form;

extern FD_correct_form * create_form_correct_form(void);
typedef struct {
	FL_FORM *moon_form;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *center_moon;
	FL_OBJECT *top_moon;
	FL_OBJECT *bottom_moon;
	FL_OBJECT *right_moon;
	FL_OBJECT *left_moon;
	FL_OBJECT *moon_offset;
} FD_moon_form;

extern FD_moon_form * create_form_moon_form(void);
typedef struct {
	FL_FORM *burst_form;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *burst_center;
	FL_OBJECT *burst_1;
	FL_OBJECT *burst_2;
	FL_OBJECT *burst_3;
	FL_OBJECT *burst_4;
	FL_OBJECT *burst_exit_button;
} FD_burst_form;

extern FD_burst_form * create_form_burst_form(void);
typedef struct {
	FL_FORM *scan_form;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *timer;
	FL_OBJECT *xyplot;
} FD_scan_form;

extern FD_scan_form * create_form_scan_form(void);
typedef struct {
	FL_FORM *message_form;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *l1;
	FL_OBJECT *l2;
	FL_OBJECT *l3;
	FL_OBJECT *l4;
	FL_OBJECT *l5;
	FL_OBJECT *l6;
	FL_OBJECT *l7;
	FL_OBJECT *l8;
	FL_OBJECT *l9;
	FL_OBJECT *l10;
	FL_OBJECT *l11;
	FL_OBJECT *l12;
	FL_OBJECT *l13;
	FL_OBJECT *l14;
	FL_OBJECT *l15;
	FL_OBJECT *l16;
} FD_message_form;

extern FD_message_form * create_form_message_form(void);
typedef struct {
	FL_FORM *notice_form;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *message_1;
	FL_OBJECT *message_2;
	FL_OBJECT *message_3;
} FD_notice_form;

extern FD_notice_form * create_form_notice_form(void);

#endif /* FD_track_form_h_ */
