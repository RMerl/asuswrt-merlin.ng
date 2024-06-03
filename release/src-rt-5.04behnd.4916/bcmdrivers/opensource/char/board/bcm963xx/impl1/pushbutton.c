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


/***************************************************************************
* File Name  : pushbutton.h
*
* Description: This file contains support for registering callbacks to
* pushbuttons.
*
***************************************************************************/

#include <stddef.h>
#include <pushbutton.h>
#include <linux/module.h>
#include <linux/kernel.h>

typedef struct {
    pushButtonNotifyHook_t        hook;
    unsigned long                 timeout; // in ms;
    void*                         param;
} pushButtonHookInfo_t;



pushButtonHookInfo_t btnPressedInfo[PB_BUTTON_MAX][MAX_BTN_HOOKS_PER_TRIG] = {};
/* Note: the following arrays must be ordered by timeout */
pushButtonHookInfo_t btnHeldInfo[PB_BUTTON_MAX][MAX_BTN_HOOKS_PER_TRIG] = {};
pushButtonHookInfo_t btnReleasedInfo[PB_BUTTON_MAX][MAX_BTN_HOOKS_PER_TRIG] = {};
int btnPressJiffies[PB_BUTTON_MAX] = {};
int btnHeldIdx[PB_BUTTON_MAX] = {};
static DEFINE_SPINLOCK(lock);


// returns the idx of the item removed, or negative number on failure
static int 
deleteFromOrderedArray( 
    pushButtonNotifyHook_t          hook, 
    unsigned long                   timeInMs,
    pushButtonHookInfo_t           *pHookInfoArray )
{
    int idx;
    int idx2;
    pushButtonHookInfo_t *pInfo;

    for (idx = 0; idx < MAX_BTN_HOOKS_PER_TRIG; idx++) {
        pInfo = &pHookInfoArray[idx];
        if (pInfo->hook == hook && pInfo->timeout == timeInMs) {
            for (idx2 = idx; idx2 < MAX_BTN_HOOKS_PER_TRIG-1; idx2++) {
                pHookInfoArray[idx2] = pHookInfoArray[idx2+1];
            }
            pHookInfoArray[MAX_BTN_HOOKS_PER_TRIG-1].hook = NULL;
            pHookInfoArray[MAX_BTN_HOOKS_PER_TRIG-1].timeout = 0;
            pHookInfoArray[MAX_BTN_HOOKS_PER_TRIG-1].param = NULL;
            return idx;
        }
    }
    printk(KERN_ERR "%s: could not delete entry at %lu ms\n", __func__, timeInMs);
    return -1;       
}


#if 0
// debug code.  Keeping it around as it's useful
static void dumpArray(pushButtonHookInfo_t           *pHookArray) {
    int idx; 
    printk("---- pushbutton array [%p]\n", pHookArray);
    for (idx = 0; idx < MAX_BTN_HOOKS_PER_TRIG; idx++) {
        printk("  [%d]: hook: %pF, timeout: %lu, param:%p\n", 
            idx, pHookArray[idx].hook, pHookArray[idx].timeout, pHookArray[idx].param);
    }
    printk("--------\n");
}
#endif

// returns insert idx, or negative number on failure
static int 
insertToOrderedArray( 
    pushButtonNotifyHook_t          hook, 
    unsigned long                   timeInMs,
    void*                           param,
    pushButtonHookInfo_t            *pHookArray )
{
    int idx;
    int idx2;
    pushButtonHookInfo_t *pInfo;

    if (pHookArray[MAX_BTN_HOOKS_PER_TRIG-1].hook != NULL) {
        printk(KERN_ERR "%s: to many entries\n", __func__);
        return -1;
    }
    for (idx = 0; idx < MAX_BTN_HOOKS_PER_TRIG; idx++) {
        pInfo = &pHookArray[idx];
        if (pInfo->hook == NULL) {           // inserting at end
            pInfo->hook = hook;
            pInfo->timeout = timeInMs;
            pInfo->param = param;
            return idx;
        }
        if (pInfo->timeout > timeInMs) {     // inserting in middle
            for (idx2 = MAX_BTN_HOOKS_PER_TRIG-1; idx2 > idx; idx2--) {
                pHookArray[idx2] = pHookArray[idx2-1];
            }
            pInfo->hook = hook;
            pInfo->timeout = timeInMs;
            pInfo->param = param;
            return idx;
        }
    }
    return -1;
}



int 
registerPushButtonPressNotifyHook(
    PB_BUTTON_ID                        btn, 
    pushButtonNotifyHook_t              hook,
    void*                               param)
{
    // Note: this is used from ISR's, so different locking mechansim required here....

    unsigned long flags;
    int idx;

    if (unlikely(btn >= PB_BUTTON_MAX)) {
        printk(KERN_ERR "%s: unrecognized button id (%d)\n", __func__, btn);
        return -1;
    }
    if (unlikely(hook == NULL)) {        
        printk(KERN_ERR "%s: cannot register NULL hook\n", __func__);
        return -1;
    }
    
    spin_lock_irqsave(&lock,flags);
    idx = insertToOrderedArray(hook,0,param,btnPressedInfo[btn]);
    spin_unlock_irqrestore(&lock,flags); 
    if (unlikely(idx < 0)) {
        printk(KERN_ERR "%s: Could not insert notify hook %pF (out of room)\n", __func__, hook);
        return -1;
    }
    
    return 0;
}



int 
registerPushButtonHoldNotifyHook(
    PB_BUTTON_ID                    btn, 
    pushButtonNotifyHook_t          hook, 
    unsigned long                   timeInMs,
    void*                           param)
{
    int idx;
    unsigned long flags;
    
    if (unlikely(btn >= PB_BUTTON_MAX)) {
        printk(KERN_ERR "%s: unrecognized button id (%d)\n", __func__, btn);
        return -1;
    }
    if (unlikely(hook == NULL)) {
        printk(KERN_ERR "%s: cannot register NULL hook\n", __func__);
        return -1;
    }
    
    spin_lock_irqsave(&lock,flags);
    idx = insertToOrderedArray(hook,timeInMs,param,btnHeldInfo[btn]);
    if (!hook && btnHeldIdx[btn])
        btnHeldIdx[btn]--;
    spin_unlock_irqrestore(&lock,flags);    
    if (unlikely(idx < 0)) {
        printk(KERN_ERR "%s: Could not insert notify hook %pF (out of room)\n", __func__, hook);
        return -1;
    }

    return 0;
}


int 
registerPushButtonReleaseNotifyHook(
    PB_BUTTON_ID                    btn, 
    pushButtonNotifyHook_t          hook, 
    unsigned long                   timeInMs,
    void*                           param)
{
    int idx;
    unsigned long flags;
    
    if (unlikely(btn >= PB_BUTTON_MAX)) {
        printk(KERN_ERR "%s: unrecognized button id (%d)\n", __func__, btn);
        return -1;
    }    
    if (unlikely(hook == NULL)) {
        printk(KERN_ERR "%s: cannot register NULL hook\n", __func__);
        return -1;
    }    
    spin_lock_irqsave(&lock,flags);
    idx = insertToOrderedArray(hook,timeInMs,param,btnReleasedInfo[btn]);
    spin_unlock_irqrestore(&lock,flags);  
    if (unlikely(idx < 0)) {
        printk(KERN_ERR "%s: Could not insert notify hook %pF (out of room)\n", __func__, hook);
        return -1;
    }
    return 0;
}


int 
deregisterPushButtonPressNotifyHook(
    PB_BUTTON_ID                        btn, 
    pushButtonNotifyHook_t              hook)
{
    // Note: this is used from ISR's, so different locking mechansim required here....

    unsigned long flags;
    int idx;

    if (unlikely(btn >= PB_BUTTON_MAX)) {
        printk(KERN_ERR "%s: unrecognized button id (%d)\n", __func__, btn);
        return -1;
    }
    if (unlikely(hook == NULL)) {
        printk(KERN_ERR "%s: cannot register NULL hook\n", __func__);
        return -1;
    }
    
    spin_lock_irqsave(&lock,flags);
    idx = deleteFromOrderedArray(hook,0,btnPressedInfo[btn]);
    spin_unlock_irqrestore(&lock,flags); 
    if (unlikely(idx < 0)) {
        printk(KERN_ERR "%s: Could not delete hook %pF (not found)\n", __func__, hook);
        return -1;
    }
    return 0;
}



int 
deregisterPushButtonHoldNotifyHook(
    PB_BUTTON_ID                    btn, 
    pushButtonNotifyHook_t          hook, 
    unsigned long                   timeInMs)
{
    int idx;
    unsigned long flags;
    
    if (unlikely(btn >= PB_BUTTON_MAX)) {
        printk(KERN_ERR "%s: unrecognized button id (%d)\n", __func__, btn);
        return -1;
    }
    if (unlikely(hook == NULL)) {
        printk(KERN_ERR "%s: cannot register NULL hook\n", __func__);
        return -1;
    }
    
    spin_lock_irqsave(&lock,flags);
    idx = deleteFromOrderedArray(hook,timeInMs,btnHeldInfo[btn]);
    if ((idx >= 0) && (btnHeldIdx[btn] >= idx))
        btnHeldIdx[btn]--;
    spin_unlock_irqrestore(&lock,flags);    
    if (unlikely(idx < 0)) {
        printk(KERN_ERR "%s: Could not delete notify hook %pF (not found)\n", __func__, hook);
        return -1;
    }
    return 0;
}


int
deregisterPushButtonReleaseNotifyHook(
    PB_BUTTON_ID                    btn, 
    pushButtonNotifyHook_t          hook, 
    unsigned long                   timeInMs)
{
    int idx;    
    unsigned long flags;

    if (unlikely(btn >= PB_BUTTON_MAX)) {
        printk(KERN_ERR "%s: unrecognized button id (%d)\n", __func__, btn);
        return -1;
    }    
    if (unlikely(hook == NULL)) {        
        printk(KERN_ERR "%s: cannot register NULL hook\n", __func__);
        return -1;
    }    
    spin_lock_irqsave(&lock,flags);
    idx = deleteFromOrderedArray(hook,timeInMs,btnReleasedInfo[btn]);
    spin_unlock_irqrestore(&lock,flags);  
    if (unlikely(idx < 0)) {
        printk(KERN_ERR "%s: Could not insert notify hook %pF (out of room)\n", __func__, hook);
        return -1;
    }
    return 0;
}



void 
doPushButtonPress(PB_BUTTON_ID btn, unsigned long currentJiffies) 
{
    unsigned long flags;
    int callIdx;
    int callInfoIdx = 0;
    int idx;
    pushButtonHookInfo_t callInfo[MAX_BTN_HOOKS_PER_TRIG] = {};      
    pushButtonHookInfo_t *pInfo;

    if (unlikely(btn >= PB_BUTTON_MAX)) {
        printk(KERN_ERR "%s: unrecognized button id (%d)\n", __func__, btn);
        return;
    }
    
    spin_lock_irqsave(&lock,flags);

    btnPressJiffies[btn] = currentJiffies;
    btnHeldIdx[btn] = 0;
    
    for (idx = 0; idx < MAX_BTN_HOOKS_PER_TRIG; idx++)
    {
        pInfo = &btnPressedInfo[btn][idx];
        if (pInfo->hook) {
            callInfo[callInfoIdx] = *pInfo;
            callInfoIdx++;
        }
    }
    spin_unlock_irqrestore(&lock, flags);

    for (callIdx = 0; callIdx < callInfoIdx; callIdx++)
        callInfo[callIdx].hook(currentJiffies, callInfo[callIdx].param);
}


void
doPushButtonHold(PB_BUTTON_ID btn, unsigned long currentJiffies)
{
    unsigned long timeInMs;
    pushButtonHookInfo_t *pInfo;
    int callIdx;
    int callInfoIdx = 0;
    pushButtonHookInfo_t callInfo[MAX_BTN_HOOKS_PER_TRIG] = {};      
    unsigned long flags;
    
    if (unlikely(btn >= PB_BUTTON_MAX)) {
        printk(KERN_ERR "%s: unrecognized button id (%d)\n", __func__, btn);
        return;
    }

    timeInMs = jiffies_to_msecs(currentJiffies-btnPressJiffies[btn]);

    spin_lock_irqsave(&lock,flags);
    {
        while(btnHeldIdx[btn] < MAX_BTN_HOOKS_PER_TRIG) {
            pInfo = &btnHeldInfo[btn][btnHeldIdx[btn]];
            if (pInfo->hook == NULL) {
                btnHeldIdx[btn] = MAX_BTN_HOOKS_PER_TRIG;
                break;
            }
            if (pInfo->timeout > timeInMs)
                break;
            btnHeldIdx[btn]++;
            callInfo[callInfoIdx] = *pInfo;
            callInfoIdx++;
        }
    }
    spin_unlock_irqrestore(&lock, flags);

    for (callIdx = 0; callIdx < callInfoIdx; callIdx++) {
        callInfo[callIdx].hook(timeInMs, callInfo[callIdx].param);
    }
}


void
doPushButtonRelease(PB_BUTTON_ID btn, unsigned long currentJiffies)
{
    int idx;
    unsigned long timeInMs;
    unsigned long flags;
    int callIdx;
    int callInfoIdx = 0;
    pushButtonHookInfo_t callInfo[MAX_BTN_HOOKS_PER_TRIG] = {};      
   
    if (unlikely(btn >= PB_BUTTON_MAX)) {
        printk(KERN_ERR "%s: unrecognized button id (%d)\n", __func__, btn);
        return;
    }

    timeInMs = jiffies_to_msecs(currentJiffies-btnPressJiffies[btn]);
    
    spin_lock_irqsave(&lock,flags);
    for (idx = 0; idx < MAX_BTN_HOOKS_PER_TRIG; idx++) {
        pushButtonHookInfo_t *pNewInfo = &btnReleasedInfo[btn][idx];
        if ( (pNewInfo->hook == NULL) || (pNewInfo->timeout > timeInMs))
            continue;
        if ( callInfoIdx == 0 || pNewInfo->timeout == callInfo[0].timeout) {
            callInfo[callInfoIdx] = *pNewInfo;
            callInfoIdx++;
        } else if (pNewInfo->timeout > callInfo[0].timeout) {
            callInfo[0] = *pNewInfo;
            callInfoIdx = 1;
        }
    }
    spin_unlock_irqrestore(&lock, flags);
    
    for (callIdx = 0; callIdx < callInfoIdx; callIdx++) {
        callInfo[callIdx].hook(timeInMs, callInfo[callIdx].param);
    }
}

