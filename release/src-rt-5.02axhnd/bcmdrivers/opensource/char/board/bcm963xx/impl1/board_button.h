/*
* <:copyright-BRCM:2016:DUAL/GPL:standard
* 
*    Copyright (c) 2016 Broadcom 
*    All Rights Reserved
* 
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
* 
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
* 
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
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
