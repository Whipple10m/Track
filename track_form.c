/* Form definition file generated with fdesign. */

#include "forms.h"
#include <stdlib.h>
#include "track_form.h"

FD_track_form *create_form_track_form(void)
{
  FL_OBJECT *obj;
  FD_track_form *fdui = (FD_track_form *) fl_calloc(1, sizeof(*fdui));

  fdui->track_form = fl_bgn_form(FL_NO_BOX, 202, 980);
  obj = fl_add_box(FL_UP_BOX,0,0,202,980,"");
  obj = fl_add_text(FL_NORMAL_TEXT,10,87,50,30,"SID");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_EMBOSSED_STYLE);
  fdui->sid_time = obj = fl_add_text(FL_NORMAL_TEXT,60,87,120,30,"00:00:00");
    fl_set_object_boxtype(obj,FL_ROUNDED3D_DOWNBOX);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,10,50,50,30,"U.T.");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_EMBOSSED_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT,4,237,50,30,"RA");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_EMBOSSED_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT,4,318,50,30,"AZ");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_EMBOSSED_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT,4,277,50,30,"DEC");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_EMBOSSED_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT,4,358,50,30,"EL");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_EMBOSSED_STYLE);
  fdui->elobj = obj = fl_add_text(FL_NORMAL_TEXT,54,358,120,30,"32.05");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,34,403,130,20,"TELESCOPE");
    fl_set_object_lcolor(obj,FL_DARKVIOLET);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_EMBOSSED_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT,3,440,50,30,"AZ");
    fl_set_object_lcolor(obj,FL_DARKVIOLET);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_EMBOSSED_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT,4,483,50,30,"EL");
    fl_set_object_lcolor(obj,FL_DARKVIOLET);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_EMBOSSED_STYLE);
  fdui->raobj = obj = fl_add_button(FL_NORMAL_BUTTON,54,237,120,30,"00:00:00");
    fl_set_button_shortcut(obj,"r",1);
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_callback(obj,change_ra,0);
  fdui->decobj = obj = fl_add_button(FL_NORMAL_BUTTON,54,277,120,30,"00:00:00");
    fl_set_button_shortcut(obj,"d",1);
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_callback(obj,change_dec,0);
  fdui->objid = obj = fl_add_button(FL_NORMAL_BUTTON,21,156,160,30,"POLARIS");
    fl_set_button_shortcut(obj,"g",1);
    fl_set_object_boxtype(obj,FL_ROUNDED3D_DOWNBOX);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_EMBOSSED_STYLE);
    fl_set_object_callback(obj,get_object,0);
  obj = fl_add_text(FL_NORMAL_TEXT,72,536,80,30,"ERROR");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_EMBOSSED_STYLE);
  fdui->diff = obj = fl_add_text(FL_NORMAL_TEXT,54,566,120,30,"0.00");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_color(obj,FL_TOP_BCOL,FL_MCOL);
    fl_set_object_lcolor(obj,FL_RED);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,71,609,80,30,"OFFSET");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_EMBOSSED_STYLE);
  fdui->mode = obj = fl_add_lightbutton(FL_PUSH_BUTTON,29,822,140,30,"STANDBY");
    fl_set_button_shortcut(obj,"t",1);
    fl_set_object_color(obj,FL_RED,FL_GREEN);
    fl_set_object_lcolor(obj,FL_YELLOW);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE+FL_ENGRAVED_STYLE);
    fl_set_object_callback(obj,change_mode,0);
  fdui->commands = obj = fl_add_menu(FL_PUSH_MENU,29,942,140,30,"COMMANDS");
    fl_set_object_shortcut(obj,"c",1);
    fl_set_object_boxtype(obj,FL_UP_BOX);
    fl_set_object_color(obj,FL_COL1,FL_TOP_BCOL);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
    fl_set_object_callback(obj,commands,0);
  fdui->offset = obj = fl_add_button(FL_NORMAL_BUTTON,54,639,120,30,"30.00");
    fl_set_button_shortcut(obj,"o",1);
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_callback(obj,change_offset,0);
  fdui->first_source = obj = fl_add_button(FL_NORMAL_BUTTON,29,902,140,30,"ON-OFF");
    fl_set_object_color(obj,FL_COL1,FL_TOP_BCOL);
    fl_set_object_lcolor(obj,FL_BLUE);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_EMBOSSED_STYLE);
    fl_set_object_callback(obj,change_first_source,0);
  fdui->azobj = obj = fl_add_text(FL_NORMAL_TEXT,54,318,120,30,"359.89");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  fdui->aztel = obj = fl_add_button(FL_NORMAL_BUTTON,53,440,120,30,"000.00");
    fl_set_button_shortcut(obj,"d",1);
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_color(obj,FL_COL1,FL_TOP_BCOL);
    fl_set_object_lcolor(obj,FL_DARKVIOLET);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_callback(obj,change_drift_mode,0);
  fdui->eltel = obj = fl_add_button(FL_NORMAL_BUTTON,54,483,120,30,"00.00");
    fl_set_button_shortcut(obj,"d",1);
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_color(obj,FL_COL1,FL_TOP_BCOL);
    fl_set_object_lcolor(obj,FL_DARKVIOLET);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_callback(obj,change_drift_mode,0);
  fdui->offsource = obj = fl_add_roundbutton(FL_PUSH_BUTTON,39,727,40,40,"OFF Source");
    fl_set_button_shortcut(obj,"s",1);
    fl_set_object_color(obj,FL_MCOL,FL_MAGENTA);
    fl_set_object_lcolor(obj,FL_BLUE);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
    fl_set_object_callback(obj,change_slew,0);
  fdui->onsource = obj = fl_add_roundbutton(FL_PUSH_BUTTON,39,687,40,40,"ON Source");
    fl_set_object_color(obj,FL_MCOL,FL_MAGENTA);
    fl_set_object_lcolor(obj,FL_BLUE);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
    fl_set_object_callback(obj,change_slew,1);
  fdui->pointcheck = obj = fl_add_roundbutton(FL_PUSH_BUTTON,39,767,40,40,"POINT CHECK");
    fl_set_button_shortcut(obj,"p",1);
    fl_set_object_lcolor(obj,FL_BLUE);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
    fl_set_object_callback(obj,point_check,0);
  fdui->slew = obj = fl_add_text(FL_NORMAL_TEXT,29,862,140,30,"STOPPED");
    fl_set_object_boxtype(obj,FL_ROUNDED3D_DOWNBOX);
    fl_set_object_lcolor(obj,FL_GREEN);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
  fdui->code = obj = fl_add_text(FL_NORMAL_TEXT,59,197,80,30,"STAR");
    fl_set_object_boxtype(obj,FL_ROUNDED3D_DOWNBOX);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_EMBOSSED_STYLE);
  fdui->ut_time = obj = fl_add_button(FL_NORMAL_BUTTON,59,50,120,30,"00:00:00");
    fl_set_object_boxtype(obj,FL_ROUNDED3D_DOWNBOX);
    fl_set_object_color(obj,FL_TOP_BCOL,FL_COL1);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
  fdui->ut_date = obj = fl_add_button(FL_NORMAL_BUTTON,31,7,142,31,"2000-01-01");
    fl_set_object_boxtype(obj,FL_ROUNDED3D_UPBOX);
    fl_set_object_color(obj,FL_TOP_BCOL,FL_COL1);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
  fl_end_form();

  fdui->track_form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_browser_form *create_form_browser_form(void)
{
  FL_OBJECT *obj;
  FD_browser_form *fdui = (FD_browser_form *) fl_calloc(1, sizeof(*fdui));

  fdui->browser_form = fl_bgn_form(FL_NO_BOX, 580, 310);
  obj = fl_add_box(FL_UP_BOX,0,0,580,310,"");
  fdui->get_object = obj = fl_add_browser(FL_SELECT_BROWSER,20,20,540,280,"");
    fl_set_object_callback(obj,choose_object,0);
  fl_end_form();

  fdui->browser_form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_correct_form *create_form_correct_form(void)
{
  FL_OBJECT *obj;
  FD_correct_form *fdui = (FD_correct_form *) fl_calloc(1, sizeof(*fdui));

  fdui->correct_form = fl_bgn_form(FL_NO_BOX, 620, 303);
  obj = fl_add_box(FL_UP_BOX,0,0,620,303,"");
  fdui->write_offset_button = obj = fl_add_button(FL_NORMAL_BUTTON,326,203,143,28,"WRITE TO FILE");
    fl_set_button_shortcut(obj,"w",1);
    fl_set_object_color(obj,FL_COL1,FL_RED);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_EMBOSSED_STYLE);
    fl_set_object_callback(obj,write_offset,0);
  fdui->correction_exit_button = obj = fl_add_roundbutton(FL_NORMAL_BUTTON,525,245,40,40,"EXIT");
    fl_set_object_color(obj,FL_WHEAT,FL_RED);
    fl_set_object_lcolor(obj,FL_BLUE);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_EMBOSSED_STYLE);
    fl_set_object_callback(obj,correction_exit,0);
  obj = fl_add_text(FL_NORMAL_TEXT,18,248,154,42,"# OF POINT CHECKS:");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_EMBOSSED_STYLE);
  fdui->correction_number = obj = fl_add_text(FL_NORMAL_TEXT,172,253,78,40,"0");
    fl_set_object_boxtype(obj,FL_OVAL3D_DOWNBOX);
    fl_set_object_lcolor(obj,FL_YELLOW);
    fl_set_object_lsize(obj,FL_HUGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_EMBOSSED_STYLE);
  fdui->az_slider = obj = fl_add_counter(FL_NORMAL_COUNTER,19,15,270,40,"AZIMUTH");
    fl_set_object_color(obj,FL_TOP_BCOL,FL_BLUE);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_EMBOSSED_STYLE);
    fl_set_object_callback(obj,az_adjust,0);
  fdui->el_slider = obj = fl_add_counter(FL_NORMAL_COUNTER,329,16,270,40,"ELEVATION");
    fl_set_object_color(obj,FL_TOP_BCOL,FL_BLUE);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_EMBOSSED_STYLE);
    fl_set_object_callback(obj,el_adjust,0);
  fdui->Z_slider = obj = fl_add_counter(FL_NORMAL_COUNTER,17,176,270,40,"Z (EL in tube plane)");
    fl_set_object_color(obj,FL_TOP_BCOL,FL_BLUE);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_EMBOSSED_STYLE);
    fl_set_object_callback(obj,Z_adjust,0);
  fdui->X_slider = obj = fl_add_counter(FL_NORMAL_COUNTER,20,95,270,40,"X (AZ in tube plane)");
    fl_set_object_color(obj,FL_TOP_BCOL,FL_BLUE);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_EMBOSSED_STYLE);
    fl_set_object_callback(obj,X_adjust,0);
  obj = fl_add_button(FL_NORMAL_BUTTON,330,112,143,28,"CHANGE TUBE");
    fl_set_object_color(obj,FL_COL1,FL_RED);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_EMBOSSED_STYLE);
    fl_set_object_callback(obj,new_tube,0);
  fdui->pmt_tube = obj = fl_add_text(FL_NORMAL_TEXT,489,150,75,44,"1");
    fl_set_object_boxtype(obj,FL_OVAL3D_DOWNBOX);
    fl_set_object_lcolor(obj,FL_RED);
    fl_set_object_lsize(obj,FL_HUGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_EMBOSSED_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT,331,156,155,31,"TRACKING IN TUBE:");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_EMBOSSED_STYLE);
  fl_end_form();

  fdui->correct_form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_moon_form *create_form_moon_form(void)
{
  FL_OBJECT *obj;
  FD_moon_form *fdui = (FD_moon_form *) fl_calloc(1, sizeof(*fdui));

  fdui->moon_form = fl_bgn_form(FL_NO_BOX, 450, 430);
  obj = fl_add_box(FL_UP_BOX,0,0,450,430,"");
  obj = fl_add_roundbutton(FL_HIDDEN_BUTTON,170,0,200,160,"");
    fl_set_object_color(obj,FL_RED,FL_WHITE);
  fdui->center_moon = obj = fl_add_button(FL_PUSH_BUTTON,170,150,110,110,"+");
    fl_set_object_boxtype(obj,FL_OVAL_BOX);
    fl_set_object_color(obj,FL_WHITE,FL_COL1);
    fl_set_object_lsize(obj,FL_HUGE_SIZE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_EMBOSSED_STYLE);
  fdui->top_moon = obj = fl_add_button(FL_PUSH_BUTTON,170,25,110,110,"TOP");
    fl_set_object_boxtype(obj,FL_OVAL_BOX);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_EMBOSSED_STYLE);
    fl_set_object_callback(obj,move_moon,TOP);
  fdui->bottom_moon = obj = fl_add_button(FL_PUSH_BUTTON,170,275,110,110,"BOTTOM");
    fl_set_object_boxtype(obj,FL_OVAL_BOX);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_EMBOSSED_STYLE);
    fl_set_object_callback(obj,move_moon,BOTTOM);
  fdui->right_moon = obj = fl_add_button(FL_PUSH_BUTTON,300,150,110,110,"PROTON");
    fl_set_object_boxtype(obj,FL_OVAL_BOX);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_EMBOSSED_STYLE);
    fl_set_object_callback(obj,move_moon,PROTON);
  fdui->left_moon = obj = fl_add_button(FL_RADIO_BUTTON,40,150,110,110,"ANTI-PROTON");
    fl_set_object_boxtype(obj,FL_OVAL_BOX);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_EMBOSSED_STYLE);
    fl_set_object_callback(obj,move_moon,ANTI_PROTON);
  obj = fl_add_button(FL_NORMAL_BUTTON,360,390,80,30,"EXIT");
    fl_set_object_lcolor(obj,FL_YELLOW);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_EMBOSSED_STYLE);
    fl_set_object_callback(obj,exit_moon,0);
  obj = fl_add_button(FL_NORMAL_BUTTON,10,390,150,30,"MOON CENTER");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_EMBOSSED_STYLE);
    fl_set_object_callback(obj,move_moon,CENTER);
  obj = fl_add_text(FL_NORMAL_TEXT,213,190,30,30,"+");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_lsize(obj,FL_HUGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_EMBOSSED_STYLE);
  fdui->moon_offset = obj = fl_add_button(FL_PUSH_BUTTON,10,10,150,30,"ON-MOON");
    fl_set_object_color(obj,FL_COL1,FL_RED);
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_EMBOSSED_STYLE);
    fl_set_object_callback(obj,move_moon,OFFSET);
  fl_end_form();

  fdui->moon_form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_burst_form *create_form_burst_form(void)
{
  FL_OBJECT *obj;
  FD_burst_form *fdui = (FD_burst_form *) fl_calloc(1, sizeof(*fdui));

  fdui->burst_form = fl_bgn_form(FL_NO_BOX, 460, 430);
  obj = fl_add_box(FL_UP_BOX,0,0,460,430,"");
  fdui->burst_center = obj = fl_add_button(FL_NORMAL_BUTTON,160,140,120,120,"Burst");
    fl_set_object_boxtype(obj,FL_OVAL_BOX);
    fl_set_object_lsize(obj,FL_HUGE_SIZE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_EMBOSSED_STYLE);
    fl_set_object_callback(obj,burst_move,CENTER);
  fdui->burst_1 = obj = fl_add_button(FL_NORMAL_BUTTON,160,10,120,120,"#1");
    fl_set_object_boxtype(obj,FL_OVAL_BOX);
    fl_set_object_lsize(obj,FL_HUGE_SIZE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_EMBOSSED_STYLE);
    fl_set_object_callback(obj,burst_move,TOP);
  fdui->burst_2 = obj = fl_add_button(FL_NORMAL_BUTTON,290,140,120,120,"#2");
    fl_set_object_boxtype(obj,FL_OVAL_BOX);
    fl_set_object_lsize(obj,FL_HUGE_SIZE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_EMBOSSED_STYLE);
    fl_set_object_callback(obj,burst_move,AHEAD);
  fdui->burst_3 = obj = fl_add_button(FL_NORMAL_BUTTON,160,270,120,120,"#3");
    fl_set_object_boxtype(obj,FL_OVAL_BOX);
    fl_set_object_lsize(obj,FL_HUGE_SIZE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_EMBOSSED_STYLE);
    fl_set_object_callback(obj,burst_move,BOTTOM);
  fdui->burst_4 = obj = fl_add_button(FL_NORMAL_BUTTON,30,140,120,120,"#4");
    fl_set_object_boxtype(obj,FL_OVAL_BOX);
    fl_set_object_lsize(obj,FL_HUGE_SIZE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_EMBOSSED_STYLE);
    fl_set_object_callback(obj,burst_move,BEHIND);
  fdui->burst_exit_button = obj = fl_add_button(FL_NORMAL_BUTTON,340,380,90,30,"EXIT");
    fl_set_object_color(obj,FL_COL1,FL_YELLOW);
    fl_set_object_lcolor(obj,FL_BLUE);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_EMBOSSED_STYLE);
    fl_set_object_callback(obj,burst_exit,0);
  fl_end_form();

  fdui->burst_form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_scan_form *create_form_scan_form(void)
{
  FL_OBJECT *obj;
  FD_scan_form *fdui = (FD_scan_form *) fl_calloc(1, sizeof(*fdui));

  fdui->scan_form = fl_bgn_form(FL_NO_BOX, 530, 440);
  obj = fl_add_box(FL_UP_BOX,0,0,530,440,"");
  fdui->timer = obj = fl_add_timer(FL_VALUE_TIMER,410,20,100,30,"");
    fl_set_object_color(obj,FL_WHITE,FL_RED);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lstyle(obj,FL_FIXED_STYLE+FL_EMBOSSED_STYLE);
  obj = fl_add_button(FL_NORMAL_BUTTON,130,20,90,30,"STOP");
    fl_set_object_lcolor(obj,FL_RED);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_EMBOSSED_STYLE);
    fl_set_object_callback(obj,stop_scan,0);
  obj = fl_add_button(FL_NORMAL_BUTTON,12,20,90,30,"START");
    fl_set_object_lcolor(obj,FL_SPRINGGREEN);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_EMBOSSED_STYLE);
    fl_set_object_callback(obj,start_scan,0);
  fdui->xyplot = obj = fl_add_xyplot(FL_NORMAL_XYPLOT,70,60,380,340,"Scan status");
    fl_set_object_boxtype(obj,FL_UP_BOX);
    fl_set_object_color(obj,FL_LEFT_BCOL,FL_BLACK);
    fl_set_object_lcolor(obj,FL_BLUE);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_FIXEDBOLDITALIC_STYLE);
  obj = fl_add_button(FL_NORMAL_BUTTON,250,20,90,30,"EXIT");
    fl_set_object_lcolor(obj,FL_YELLOW);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_EMBOSSED_STYLE);
    fl_set_object_callback(obj,exit_scan,0);
  fl_end_form();

  fdui->scan_form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_message_form *create_form_message_form(void)
{
  FL_OBJECT *obj;
  FD_message_form *fdui = (FD_message_form *) fl_calloc(1, sizeof(*fdui));

  fdui->message_form = fl_bgn_form(FL_NO_BOX, 580, 690);
  obj = fl_add_box(FL_UP_BOX,0,0,580,690,"");
  fdui->l1 = obj = fl_add_text(FL_NORMAL_TEXT,20,20,530,30,"");
    fl_set_object_color(obj,FL_TOP_BCOL,FL_MCOL);
    fl_set_object_lcolor(obj,FL_BLUE);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  fdui->l2 = obj = fl_add_text(FL_NORMAL_TEXT,20,60,530,30,"");
    fl_set_object_color(obj,FL_TOP_BCOL,FL_MCOL);
    fl_set_object_lcolor(obj,FL_BLUE);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  fdui->l3 = obj = fl_add_text(FL_NORMAL_TEXT,20,100,530,30,"");
    fl_set_object_color(obj,FL_TOP_BCOL,FL_MCOL);
    fl_set_object_lcolor(obj,FL_BLUE);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  fdui->l4 = obj = fl_add_text(FL_NORMAL_TEXT,20,140,530,30,"");
    fl_set_object_color(obj,FL_TOP_BCOL,FL_MCOL);
    fl_set_object_lcolor(obj,FL_BLUE);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  fdui->l5 = obj = fl_add_text(FL_NORMAL_TEXT,20,180,530,30,"");
    fl_set_object_color(obj,FL_TOP_BCOL,FL_MCOL);
    fl_set_object_lcolor(obj,FL_BLUE);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  fdui->l6 = obj = fl_add_text(FL_NORMAL_TEXT,20,220,530,30,"");
    fl_set_object_color(obj,FL_TOP_BCOL,FL_MCOL);
    fl_set_object_lcolor(obj,FL_BLUE);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  fdui->l7 = obj = fl_add_text(FL_NORMAL_TEXT,20,260,530,30,"");
    fl_set_object_color(obj,FL_TOP_BCOL,FL_MCOL);
    fl_set_object_lcolor(obj,FL_BLUE);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  fdui->l8 = obj = fl_add_text(FL_NORMAL_TEXT,20,300,530,30,"");
    fl_set_object_color(obj,FL_TOP_BCOL,FL_MCOL);
    fl_set_object_lcolor(obj,FL_BLUE);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  fdui->l9 = obj = fl_add_text(FL_NORMAL_TEXT,20,340,530,30,"");
    fl_set_object_color(obj,FL_TOP_BCOL,FL_MCOL);
    fl_set_object_lcolor(obj,FL_BLUE);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  fdui->l10 = obj = fl_add_text(FL_NORMAL_TEXT,20,380,530,30,"");
    fl_set_object_color(obj,FL_TOP_BCOL,FL_MCOL);
    fl_set_object_lcolor(obj,FL_BLUE);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  fdui->l11 = obj = fl_add_text(FL_NORMAL_TEXT,20,420,530,30,"");
    fl_set_object_color(obj,FL_TOP_BCOL,FL_MCOL);
    fl_set_object_lcolor(obj,FL_BLUE);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  fdui->l12 = obj = fl_add_text(FL_NORMAL_TEXT,20,460,530,30,"");
    fl_set_object_color(obj,FL_TOP_BCOL,FL_MCOL);
    fl_set_object_lcolor(obj,FL_BLUE);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  fdui->l13 = obj = fl_add_text(FL_NORMAL_TEXT,20,500,530,30,"");
    fl_set_object_color(obj,FL_TOP_BCOL,FL_MCOL);
    fl_set_object_lcolor(obj,FL_BLUE);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  fdui->l14 = obj = fl_add_text(FL_NORMAL_TEXT,20,540,530,30,"");
    fl_set_object_color(obj,FL_TOP_BCOL,FL_MCOL);
    fl_set_object_lcolor(obj,FL_BLUE);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  fdui->l15 = obj = fl_add_text(FL_NORMAL_TEXT,20,580,530,30,"");
    fl_set_object_color(obj,FL_TOP_BCOL,FL_MCOL);
    fl_set_object_lcolor(obj,FL_BLUE);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  fdui->l16 = obj = fl_add_text(FL_NORMAL_TEXT,20,620,530,30,"");
    fl_set_object_color(obj,FL_TOP_BCOL,FL_MCOL);
    fl_set_object_lcolor(obj,FL_BLUE);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  fl_end_form();

  fdui->message_form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_notice_form *create_form_notice_form(void)
{
  FL_OBJECT *obj;
  FD_notice_form *fdui = (FD_notice_form *) fl_calloc(1, sizeof(*fdui));

  fdui->notice_form = fl_bgn_form(FL_NO_BOX, 520, 110);
  obj = fl_add_box(FL_UP_BOX,0,0,520,110,"");
  fdui->message_1 = obj = fl_add_text(FL_NORMAL_TEXT,10,10,500,30,"");
    fl_set_object_lcolor(obj,FL_WHITE);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_EMBOSSED_STYLE);
  fdui->message_2 = obj = fl_add_text(FL_NORMAL_TEXT,10,40,500,30,"");
    fl_set_object_lcolor(obj,FL_WHITE);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_EMBOSSED_STYLE);
  fdui->message_3 = obj = fl_add_text(FL_NORMAL_TEXT,10,70,500,30,"");
    fl_set_object_lcolor(obj,FL_WHITE);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_EMBOSSED_STYLE);
  fl_end_form();

  fdui->notice_form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

