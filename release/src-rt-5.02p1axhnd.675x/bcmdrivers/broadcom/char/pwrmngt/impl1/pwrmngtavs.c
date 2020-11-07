/*
* <:copyright-BRCM:2008:proprietary:standard
* 
*    Copyright (c) 2008 Broadcom 
*    All Rights Reserved
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
:>
*/

#include <linux/kthread.h>
#include "boardparms.h"
#include "bcm_map_part.h"

/* A generic kernel thread for board related maintenance activities */
/* Only used if AVS is configured, but feel free to change */


#if defined(RUN_AVS_65)
struct task_struct * avs65_task_struct = NULL;
extern int Avs65_Task(void *data);
#endif

#if defined(RUN_AVS_40)
struct task_struct * avs40_task_struct = NULL;
extern int Avs40_Task(void *data);
extern void Avs_BootRun(uint32_t vmin_avs_mv);
#endif

int AvsEnabled = -1;
int AvsLogPeriod = 0;

int BcmPwrMngtAvsEnabled(void)
{
    return AvsEnabled;
}

void BcmPwrMngtEnableAvs(int enable)
{
    unsigned short vregVsel1P2;

    /* Set target 1V2 level */
    if (BpGetVregSel1P2(&vregVsel1P2) == BP_SUCCESS ) {
        AvsEnabled = -1;
        printk("Adaptive Voltage Scaling is disabled because 1V2 is forced to a specific level by design\n");
    }
#if defined(CONFIG_BCM963268) && !defined(CONFIG_BCM_6802_MoCA)
    else if (MISC->miscVregCtrl0 & MISC_VREG_CONTROL0_POWER_DOWN_1) {
        AvsEnabled = -1;
        printk("Adaptive Voltage Scaling is disabled because the internal regulator is powered down\n");    
    }
#endif
    else {
        AvsEnabled = enable;
        printk("Adaptive Voltage Scaling is now %s\n", (enable==1?"enabled":
                                                       (enable==0?"disabled":
                                                       (enable==-1?"stopped":"deep"))));

        if (enable == 1) {
#if defined(RUN_AVS_65)
            if (avs65_task_struct == NULL) {
                avs65_task_struct = kthread_create(Avs65_Task, NULL, "Avs65_Task");
                printk("%s: AVS_START, %p\n", __FUNCTION__, avs65_task_struct);
                kthread_bind(avs65_task_struct, 0);
                wake_up_process(avs65_task_struct);
            }
#endif 
#if defined(RUN_AVS_40)
            if (avs40_task_struct == NULL) {
                unsigned short vregAvsMin;
                if (BpGetVregAvsMin(&vregAvsMin) == BP_SUCCESS) {
                    printk("Adaptive Voltage Scaling minimum voltage set to %d mVolts\n", vregAvsMin);
                }
                else {
                    vregAvsMin = 0;
                }
                Avs_BootRun(vregAvsMin);
                avs40_task_struct = kthread_create(Avs40_Task, NULL, "Avs40_Task");
                printk("%s: AVS_START, %p\n", __FUNCTION__, avs40_task_struct);
                kthread_bind(avs40_task_struct, 0);
                wake_up_process(avs40_task_struct);
            }
#endif
        }        
#if defined(RUN_AVS_40)
        else if ((enable == 0) && (avs40_task_struct)) {
            printk("%s: AVS_STOP, %p\n", __FUNCTION__, avs40_task_struct);
            kthread_stop(avs40_task_struct);
            avs40_task_struct = NULL;
        }
#endif
    }
}


int BcmPwrMngtAvsLogGet(void)
{
	return (AvsLogPeriod);
}


void BcmPwrMngtAvsLogSet(int period)
{
	AvsLogPeriod = period;
	printk("AvsLogPeriod set to %d\n", period);
}
 
