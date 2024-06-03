/*
<:copyright-BRCM:2013:GPL/GPL:standard

   Copyright (c) 2013 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/

/***************************************************************************
* File Name  : pushbutton.h
*
* Description: This file contains support for registering callbacks to
* pushbuttons.
*
***************************************************************************/

#ifndef _PUSHBUTTON_H_
#define _PUSHBUTTON_H_

#define MAX_BTN_HOOKS_PER_TRIG 5
#define MAX_BTN_HOOKS_PER_BTN  (MAX_BTN_HOOKS_PER_TRIG * 3)

typedef enum {
    PB_BUTTON_0,
    PB_BUTTON_1,
    PB_BUTTON_2,
    PB_BUTTON_MAX
} PB_BUTTON_ID;

typedef void (* pushButtonNotifyHook_t)(unsigned long timeInMs, void* param);



/* The following will allow callbacks to be registered against various button events.

   press event:  occurs when the button is first pressed (usually caught in ISR context).  If multiple
                 hooks are registered against the press event, then all hooks are invoked.

   hold event:   occurs after the button has been held for a specific period of time (does not require
                 a button release).

   release event: occurs when the button is released (usually in Timer Soft-IRQ context).  A button release
                 hook will only be called if the button was held down more than the specified time.  Also, 
                 only the most recent hook will be invoked.  In the event that multiple hooks are registered 
                 with the exact same time as the most recent time, then all events with this time will be invoked
*/


extern int registerPushButtonPressNotifyHook(PB_BUTTON_ID btn, pushButtonNotifyHook_t hook, void* param);
extern int registerPushButtonHoldNotifyHook(PB_BUTTON_ID btn, pushButtonNotifyHook_t hook, unsigned long timeInMs, void* param);
extern int registerPushButtonReleaseNotifyHook(PB_BUTTON_ID btn, pushButtonNotifyHook_t hook, unsigned long timeInMs, void* param);

extern int deregisterPushButtonPressNotifyHook(PB_BUTTON_ID btn, pushButtonNotifyHook_t hook);
extern int deregisterPushButtonHoldNotifyHook(PB_BUTTON_ID btn, pushButtonNotifyHook_t hook, unsigned long timeInMs);
extern int deregisterPushButtonReleaseNotifyHook(PB_BUTTON_ID btn, pushButtonNotifyHook_t hook, unsigned long timeInMs);





/* The following should only be called from board.c */
extern void doPushButtonPress(PB_BUTTON_ID btn, unsigned long currJiffie); 
extern void doPushButtonHold(PB_BUTTON_ID btn, unsigned long currJiffie);
extern void doPushButtonRelease(PB_BUTTON_ID btn, unsigned long currJiffie);

#endif

