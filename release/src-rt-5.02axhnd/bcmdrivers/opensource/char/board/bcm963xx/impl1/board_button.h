/*
* <:copyright-BRCM:2016:DUAL/GPL:standard
* 
*    Copyright (c) 2016 Broadcom 
*    All Rights Reserved
* 
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, version 2, as published by
* the Free Software Foundation (the "GPL").
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* 
* A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
* writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
* 
* :> 
*/

#ifndef _BOARD_BUTTON_H_
#define _BOARD_BUTTON_H_

#include <pushbutton.h>

#define BTN_EV_PRESSED       0x1
#define BTN_EV_HOLD          0x2
#define BTN_EV_RELEASED      0x4
#define BTN_POLLFREQ         100         /* in ms */

// Main button structure:
typedef struct _BtnInfo {
    PB_BUTTON_ID         btnId;
    int                  extIrqIdx;    // this is 0 based index
    int                  extIrqMap;    //this is the board-specific id for the irq
    int                  gpio;         //zero based index
    bool                 gpioActiveHigh;
    int                  active;       //set to 1 if button is down, 0 otherwise
    bool                 isConfigured;
    
    uint32_t             lastPressJiffies;
    uint32_t             lastHoldJiffies;
    uint32_t             lastReleaseJiffies;
    
    struct timer_list    timer;        //used for polling
    
    spinlock_t            lock;
    unsigned long         events;       //must be protected by lock
    wait_queue_head_t     waitq;
    struct task_struct *  thread;
    
    //interrupt related functions
    irqreturn_t  (* pressIsr)(int irq, void *btnInfo);
    irqreturn_t  (* releaseIsr)(int irq, void *btnInfo);
    void         (* poll)(unsigned long btnInfo);

    //functional related fuctions
    bool         (* isDown)(struct _BtnInfo *btnInfo);
    void         (* enableIrqs)(struct _BtnInfo *btnInfo);
    void         (* disableIrqs)(struct _BtnInfo *btnInfo);
} BtnInfo;

int registerBtns(void);
void btnHook_DoNothing(unsigned long timeInMs, void* param);
void btnHook_RestoreToDefault(unsigned long timeInMs, void* param);
void btnHook_Print(unsigned long timeInMs, void* param);
void btnHook_Ses(unsigned long timeInMs, void* param);
void btnHook_WlanDown(unsigned long timeInMs, void* param);
void btnHook_RandomizePlc(unsigned long timeInMs, void* param);
void btnHook_Reset(unsigned long timeInMs, void* param);
void btnHook_PlcUke(unsigned long timeInMs, void* param);

#endif
