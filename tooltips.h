/****************************************************************************
 * tooltips.h                                                               *
 *                                                                          *
 * Contains prototypes and declarations for tooltips.c.                     *
 *                                                                          *
 * Copyright (C) 1997  Michael Chu                                          *
 * This file is part of the tooltips system for XForms.                     *
 *                                                                          *
 * This program is free software; you can redistribute it and/or modify     *
 * it under the terms of the GNU General Public License as published by     *
 * the Free Software Foundation; either version 2 of the License, or        *
 * (at your option) any later version.                                      *
 *                                                                          *
 * This program is distributed in the hope that it will be useful,          *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 * GNU General Public License for more details.                             *
 *                                                                          *
 * You should have received a copy of the GNU General Public License        *
 * along with this program; if not, write to the Free Software              *
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.                *
 *                                                                          *
 * Contact information:      Michael Chu                                    *
 *                           mmchu@pobox.com                                *
 ****************************************************************************/

#ifndef _TOOLTIPS_H_
#define _TOOLTIPS_H_

#include <time.h>
#include "forms.h"
#include "tooltips_forms.h"

/* These are the possible errors that can be returned by the functions. */
typedef enum {
  TOOLTIPS_ERROR_NOERROR,            /* There is no error. */
  TOOLTIPS_ERROR_NOTINITIALIZED,     /* The tooltips system is not
					initialized. */
  TOOLTIPS_ERROR_ALREADYINITIALIZED, /* The tooltips system is already
					initialized. */
  TOOLTIPS_ERROR_ALREADYSUSPENDED,   /* The tooltips system is already
					suspended. */
  TOOLTIPS_ERROR_NOTSUSPENDED,       /* The tooltips system is not currently 
					suspended. */
  TOOLTIPS_ERROR_ALREADYREGISTERED,  /* The XForms object is already 
					registered with the tooltips system. */
  TOOLTIPS_ERROR_NOTREGISTERED,      /* The XForms object is not registered 
					with the tooltips system. */
  TOOLTIPS_ERROR_NONEFROMFORM,       /* No XForms objects from this form are
					registered with the tooltips system. */
  TOOLTIPS_ERROR_BADJUSTIFICATION,   /* This justification combination is 
					invalid. */
  TOOLTIPS_ERROR_NOTENOUGHMEMORY,    /* There was a problem with memory 
					allocation. */
  TOOLTIPS_ERROR_NULLADDRESS,        /* The pointer passed was a NULL
					pointer. */
  TOOLTIPS_ERROR_UNKNOWNERROR        /* This is an unknown error. */
} TOOLTIPS_ERROR;

/* Defines a structure for tooltips status return. */
struct struct_TOOLTIPS_STATUS {
  int initialized; /* Whether or not system is currently initialized. */
  int suspendmode; /* Whether or not system is in suspend mode currently. */
  int showing;     /* Whether or not system is currently showing a tooltip. */
  int numtips;     /* Number of tips currently registered with tooltips
		      system. */
};

/* Will typedef TOOLTIPS_STATUS so do not need to use struct 
   struct_TOOLTIPS_STATUS every single time. */
typedef struct struct_TOOLTIPS_STATUS TOOLTIPS_STATUS;

/* NOTE: The following two enumerations determine how the tooltip will be 
         displayed relative to the object. However, some combinations will 
         result in the tooltip overlapping the object. Those combinations, 
         when specified, will have no effect. In addition, in the cases when 
         the set justification would result in the tooltip being displayed off
	 the screen, the system will automatically adjust the location of the 
	 tooltip to ensure that it remains on the screen in full. */
/* Defines the justification of the tooltips. */
/* This is the place on the object from which we calculate the location
   of the tooltip. */
/* By default: TOOLTIPS_JUSTIFY_BOTTOM. */
typedef enum {
  TOOLTIPS_JUSTIFY_TOPLEFT,    /* Justify to top left of object. */
  TOOLTIPS_JUSTIFY_TOP,        /* Justify to top of object. */
  TOOLTIPS_JUSTIFY_TOPRIGHT,   /* Justify to top right of object. */
  TOOLTIPS_JUSTIFY_LEFT,       /* Justify to left of object. */
  TOOLTIPS_JUSTIFY_RIGHT,      /* Justify to right of object. */
  TOOLTIPS_JUSTIFY_BOTTOMLEFT, /* Justify to bottom left of object. */
  TOOLTIPS_JUSTIFY_BOTTOM,     /* Justify to bottom of object. */
  TOOLTIPS_JUSTIFY_BOTTOMRIGHT /* Justify to bottom right of object. */
} TOOLTIPS_JUSTIFICATION;

/* Defines the justification points on the tooltips message. */
/* This is the place on the tooltip that gets justified to the object. */
/* By default: TOOLTIPS_JUSTIFYPOINT_TOPLEFT. */
typedef enum {
  TOOLTIPS_JUSTIFYPOINT_TOPLEFT,    /* Justify to top left point of tip. */
  TOOLTIPS_JUSTIFYPOINT_TOP,        /* Justify to top of tip. */
  TOOLTIPS_JUSTIFYPOINT_TOPRIGHT,   /* Justify to top right point of tip. */
  TOOLTIPS_JUSTIFYPOINT_LEFT,       /* Justify to left point of tip. */
  TOOLTIPS_JUSTIFYPOINT_RIGHT,      /* Justify to right point of tip. */
  TOOLTIPS_JUSTIFYPOINT_BOTTOMLEFT, /* Justify to bottom left point of tip. */
  TOOLTIPS_JUSTIFYPOINT_BOTTOM,     /* Justify to bottom of tip. */
  TOOLTIPS_JUSTIFYPOINT_BOTTOMRIGHT /* Justify to bottom right of tip. */
} TOOLTIPS_JUSTIFICATIONPOINT;

/* Initializes the tooltips system. Should be called before any other tooltips
   functions. */
#ifndef __cplusplus
TOOLTIPS_ERROR tooltips_initialize();
#else
extern "C" TOOLTIPS_ERROR tooltips_initialize();
#endif

/* Shuts down the tooltips system. Should be called after tooltips functions 
   are no longer desired. No tooltips functions should be called after 
   tooltips_shutdown. All tooltips are discarded. */
#ifndef __cplusplus
TOOLTIPS_ERROR tooltips_shutdown();
#else
extern "C" TOOLTIPS_ERROR tooltips_shutdown();
#endif

/* Suspends the tooltips system. Not tooltips will appear after this function 
   call. However, no tooltips are discarded. */
#ifndef __cplusplus
TOOLTIPS_ERROR tooltips_suspend();
#else
extern "C" TOOLTIPS_ERROR tooltips_suspend();
#endif

/* Resumes the tooltips system after being suspended. If the system was not 
   suspended previously, then this call has no effect. */
#ifndef __cplusplus
TOOLTIPS_ERROR tooltips_resume();
#else
extern "C" TOOLTIPS_ERROR tooltips_resume();
#endif

/* Returns the status of the tooltips system. */
#ifndef __cplusplus
TOOLTIPS_ERROR tooltips_getstatus(TOOLTIPS_STATUS *theStatusPtr);
#else
extern "C" TOOLTIPS_ERROR tooltips_getstatus(TOOLTIPS_STATUS *theStatusPtr);
#endif

/* Adds a tooltip for an XForms object. */
#ifndef __cplusplus
TOOLTIPS_ERROR tooltips_addtip(FL_OBJECT *whichObjectPtr, char *theTipPtr);
#else
extern "C" TOOLTIPS_ERROR tooltips_addtip(FL_OBJECT *whichObjectPtr, 
					  char *theTipPtr);
#endif

/* Removes a tooltip for an XForms object. */
#ifndef __cplusplus
TOOLTIPS_ERROR tooltips_removetip(FL_OBJECT *whichObjectPtr);
#else
extern "C" TOOLTIPS_ERROR tooltips_removetip(FL_OBJECT *whichObjectPtr);
#endif

/* Removes all tooltips for a particular XForms form. */
#ifndef __cplusplus
TOOLTIPS_ERROR tooltips_removeformtips(FL_FORM *whichFormPtr);
#else
extern "C" TOOLTIPS_ERROR tooltips_removeformtips(FL_FORM *whichFormPtr);
#endif

/* Removes all tooltips from the tooltips system. */
#ifndef __cplusplus
TOOLTIPS_ERROR tooltips_cleartips();
#else
extern "C" TOOLTIPS_ERROR tooltips_cleartips();
#endif

/* Returns the tooltip associated with an XForms object. */
#ifndef __cplusplus
TOOLTIPS_ERROR tooltips_gettip(FL_OBJECT *whichObjectPtr, char **theTipPtrPtr);
#else
extern "C" TOOLTIPS_ERROR tooltips_gettip(FL_OBJECT *whichObjectPtr, 
					  char **theTipPtrPtr);
#endif

/* Sets the justification for a particular object's tooltip. */
#ifndef __cplusplus
TOOLTIPS_ERROR tooltips_setjustification(FL_OBJECT *whichObjectPtr,
					 TOOLTIPS_JUSTIFICATION 
					 theJustification,
					 TOOLTIPS_JUSTIFICATIONPOINT 
					 theJustificationPoint);
#else
extern "C" TOOLTIPS_ERROR tooltips_setjustification(FL_OBJECT *whichObjectPtr,
						    TOOLTIPS_JUSTIFICATION 
						    theJustification,
						    TOOLTIPS_JUSTIFICATIONPOINT
						    theJustificationPoint);
#endif

/* Gets the justification for a particular object's tooltip. */
#ifndef __cplusplus
TOOLTIPS_ERROR tooltips_getjustification(FL_OBJECT *whichObjectPtr,
					 TOOLTIPS_JUSTIFICATION
					 *theJustificationPtr,
					 TOOLTIPS_JUSTIFICATIONPOINT
					 *theJustificationPointPtr);
#else
extern "C" TOOLTIPS_ERROR tooltips_getjustification(FL_OBJECT *whichObjectPtr,
						    TOOLTIPS_JUSTIFICATION
						    *theJustificationPtr,
						    TOOLTIPS_JUSTIFICATIONPOINT
						    *theJustificationPointPtr);
#endif

/****************************************************************************
 * The following are internal functions and declaration of the tooltips     *
 *  system and should not be called directly by the user!                   *
 ****************************************************************************/

/* This is the structure in which we store the objects and tooltips which need
   to be handled. */
struct struct_TOOLTIPS_LIST {
  FL_OBJECT *theObjectPtr;   /* A pointer to the object for which this tip 
				is registered. */
  FL_FORM *theFormPtr;       /* A pointer to the form in which this object 
				resides. */
  char *theTipPtr;           /* A pointer to the tooltip for this object. */
  FL_FORM *theTimerFormPtr;  /* A pointer to the form containing the timer
				object. */
  FL_OBJECT *theTimerObjectPtr;
                             /* A pointer to the timer object for this
				tooltip. */
  FD_ToolTip *theTipFormPtr; /* A pointer to the tooltip form for this 
				object, if one is currently up. */
  TOOLTIPS_JUSTIFICATION theJustification;
                             /* Place on object to justify tooltip to. */
  TOOLTIPS_JUSTIFICATIONPOINT theJustificationPoint;
                             /* Place on tooltip to justify from. */
  int shouldShow;            /* A boolean flagging whether or not to show this 
				tooltip. */
  int currentlyShowing;      /* A boolean flagging whether or not this tooltip
				is currently showing. */
  struct struct_TOOLTIPS_LIST *prevNodePtr;
                             /* A pointer to previous node in list. */
  struct struct_TOOLTIPS_LIST *nextNodePtr;
                             /* A pointer to next node in list. */
};

/* This allows us to address a tooltips list node more simply. */
typedef struct struct_TOOLTIPS_LIST TOOLTIPS_LIST;

/* Defines the borders and dimension of the tooltip. */
/* Defines really the width left<->right (only height since height is what
   is higher. */
#define TOOLTIPS_TIP_HEIGHT_BORDER 5
/* Defines really the height top<->bottom (only width since width is what
   is shorter. */
#define TOOLTIPS_TIP_WIDTH_BORDER  4

/* Defines how far from object to place tooltip. */
#define TOOLTIPS_TIP_DISTANCE 5

/* Defines how long to wait (in seconds) after "arriving" on object before 
   displaying tooltip. */
#define TOOLTIPS_DELAY 0.5

/* Defines the color of the tooltip background. */
#define TOOLTIPS_TIP_BACKGROUND_RED   255
#define TOOLTIPS_TIP_BACKGROUND_GREEN 255
#define TOOLTIPS_TIP_BACKGROUND_BLUE  204

/* Defines this as the designated tooltips color map place. */
#define TOOLTIPS_TIP_BACKGROUND_MAPINDEX (FL_FREE_COL16)

/* Defines actions for use with internal tooltips_internal_set_initialize_state
   function. */
typedef enum {
  TOOLTIPS_INTERNAL_SET_INITIALIZED,   /* sets the system as initialized. */
  TOOLTIPS_INTERNAL_UNSET_INITIALIZED, /* sets the system as uninitialized. */
  TOOLTIPS_INTERNAL_CHECK_INITIALIZED  /* checks the system initialization
					  status. */
} TOOLTIPS_INTERNAL_INITCODE;

/* Defines actions for use with internal tooltips_internal_set_suspend_mode 
   function. */
typedef enum {
  TOOLTIPS_INTERNAL_SET_SUSPENDED,   /* sets the system as suspended. */
  TOOLTIPS_INTERNAL_UNSET_SUSPENDED, /* sets the system as unsuspended. */
  TOOLTIPS_INTERNAL_CHECK_SUSPENDED  /* checks the system suspension status. */
} TOOLTIPS_INTERNAL_SUSPENDCODE;

/* Defines actions for use with internal tooltips_internal_listhead 
   function. */
typedef enum {
  TOOLTIPS_INTERNAL_LIST_SET,   /* returns the head of the list. */
  TOOLTIPS_INTERNAL_LIST_CLEAR, /* clears the internal list. */
  TOOLTIPS_INTERNAL_LIST_RETURN /* does not clear list. */
} TOOLTIPS_INTERNAL_LISTCODE;

/* Used internally to define/determine whether or not system is initialized. */
int tooltips_internal_set_initialize_state(TOOLTIPS_INTERNAL_INITCODE
					   whichAction);

/* Used internally to define/determine if we are in suspend mode. */
int tooltips_internal_set_suspend_mode(TOOLTIPS_INTERNAL_SUSPENDCODE
				       whichAction);

/* Used internally to hide all showing tips either at shutdown, suspension,
   or clearing of tips. */
void tooltips_internal_hidealltips();

/* Used internally to get the height/width of a tooltip with a particular 
   label set. */
void tooltips_internal_gettipdimensions(FL_OBJECT *theToolTipPtr,
					FL_Coord *theHeightPtr, 
					FL_Coord *theWidthPtr);

/* Used internally to get the list node of a particular object. */
TOOLTIPS_LIST *tooltips_internal_getnode(FL_OBJECT *whichObjectPtr);

/* Used internally to remove a list node. */
void tooltips_internal_removenode(FL_OBJECT *whichObjectPtr);

/* Used internally to get/set/clear the tooltips list structure. This avoids
   having to have global variables. */
TOOLTIPS_LIST *tooltips_internal_listhead(TOOLTIPS_LIST *listHeadPtr,
					  TOOLTIPS_INTERNAL_LISTCODE
					  whichAction);

/* The pre-handler callback which is central to the tooltips system.
   All the checking for tooltips is done in this routine. This routine is
   called whenever an event occurs for a registered object. */
int tooltips_internal_pre_handler(FL_OBJECT *whichObjectPtr, int theEvent,
				  FL_Coord mouseX, FL_Coord mouseY, 
				  int pushedKey, void *xEvent);

/* This is the timer callback that allows tooltips to be displayed at a
   delay. */
void tooltips_internal_timer_handler(FL_OBJECT *timerObject, long data);

#endif


