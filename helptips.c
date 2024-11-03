/* Help is provided using pop up notes that are activated by holding
   the mouse over a button or form function */

#include "tooltips.h"

void help_tip_setup(void)
{


  /*start with tips for the main form buttons */
  tooltips_addtip(fd_track_form->ut_time, 
		  "Universal time is set based on system time and date\nTo change: Log in as root (password in red manual) and type...\ndate MMDDhhmmYYYY.ss\nWhere the date and time are local");
  tooltips_addtip(fd_track_form->ut_date, 
		  "Date is set based on system time and date\nTo change: Log in as root (password in red manual) and type...\ndate MMDDhhmmYYYY.ss\nWhere the date and time are local");
  tooltips_addtip(fd_track_form->eltel, 
  "Send the telescope to a specific AZIMUTH and ELEVATION.\nThen place telescope in position mode");
  tooltips_addtip(fd_track_form->aztel, 
  "Send the telescope to a specific AZIMUTH and ELEVATION.\nThen place telescope in position mode");
  tooltips_addtip(fd_track_form->raobj, 
		  "Assign a new object R.A.");
  tooltips_addtip(fd_track_form->decobj, 
		  "Assign a new object Declination");
  tooltips_addtip(fd_track_form->objid, 
		  "CHOOSE a new source from a list,\nASSIGN a new object identification,\nor track a previous BURST");
  tooltips_addtip(fd_track_form->mode, 
		  "Switches system between TRACK and STANDBY modes");
  tooltips_addtip(fd_track_form->offset, 
      "Changes the ON/OFF distance in R.A. minutes.\nEnterpositive values values of time (ie. 30 is thirty minutes");
  tooltips_addtip(fd_track_form->first_source, 
		  "Switches the observing mode between ON/OFF and OFF/ON\nUsed to switch off source from in front of\nthe object to behind the object.");
  tooltips_addtip(fd_track_form->offsource, 
      "Track a source at a point offset from object.\nOffset amount is in R.A. minutes\nPress 's' to switch between ON and OFF source\nSee OFFSET button above.");
  tooltips_addtip(fd_track_form->onsource, 
                  "Track ON the entered object\nPress 's' to switch between ON and OFF source");
  tooltips_addtip(fd_track_form->pointcheck, 
                  "Track on a faint star nearby to check pointing");
  tooltips_addtip(fd_track_form->slew, 
                  "Switch tracking between ON source and OFF source");
  tooltips_addtip(fd_track_form->commands, 
"Select from a group of listed commands:\nMOVE HOME parks the telescope\nZENITH puts the telescope at 89 degrees and the current azimuth\nPRECESS adjustes object coordinates for current epoch\nRE-READ CORRECTIONS is only used for tracking correction modes\nDO CORRECTIONS places tracking program into a correction routine\nTRACK MOON does as it says\nSTATUS displays tracking status information...good for finding errors\nEXIT gets you out of the tracking program");

  /*tips for the correcting form buttons */

  /*tips for the moon tracking form buttons */

  /*tips for the burst tracking form buttons */

  /*tips for the scanning form buttons */


}





/*
FD_track_form *fd_track_form;
FD_browser_form *fd_browser_form;
FD_correct_form *fd_correct_form;
FD_moon_form *fd_moon_form;
FD_burst_form *fd_burst_form;
FD_scan_form *fd_scan_form;
*/
