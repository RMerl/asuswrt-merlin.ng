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

#include <bcmtypes.h>
#include <bcm_map_part.h>
#include <linux/delay.h>
#include <linux/kthread.h>

#if defined(CONFIG_BCM963268)
#define RING_OSC_UPPER_BOUND (0xA800)
#define RING_OSC_LOWER_BOUND_OFFSET (1600)
#define VREG_VSEL1P2_LOWER_BOUND (MISC_VREG_CONTROL1_VSEL1P2_DEFAULT - 0x0a)
#define VREG_STEP_SIZE 4
#define DEEP_RING_OSC_UPPER_BOUND (0xB000)
#define DEEP_VREG_VSEL1P2_LOWER_BOUND (MISC_VREG_CONTROL1_VSEL1P2_DEFAULT - 0x14)
#else
#error "Not Implemented for this chip"
#endif

#define AVSDEBUG(f, ...)
//#define AVSDEBUG(f, ...) printk(f, __VA_ARGS__)

extern int AvsEnabled;

static void brcm_adaptive_voltage_scaling(void)
{
    static int max_count = 0;
    static int is_ss_part = 0;
    int ring_osc_select = 0;
    int current_1v2 = 0;
    int next_1v2 = 0;
    uint32 RingOscCtrl1 = GPIO->RingOscCtrl1;
    int ring_osc_lower_bound, ring_osc_upper_bound, vreg_sel1p2_lower_bound;

    /* Verify is AVS is not forced off */
    if (AvsEnabled != -1) {
        static uint32 initmiscVregCtrl1 = 0;

        if (!initmiscVregCtrl1) {
            if (!(MISC->miscVregCtrl0 & MISC_VREG_CONTROL0_REG_RESET_B)) {
                MISC->miscVregCtrl2 &= ~MISC_VREG_CONTROL2_SWITCH_CLKEN;
                MISC->miscVregCtrl1 = MISC->miscVregCtrl1;
                MISC->miscVregCtrl0 |= MISC_VREG_CONTROL0_REG_RESET_B;
            }
            initmiscVregCtrl1 = 1;
        }
        /* Verify if the ring oscillator has completed a measurement */
        /* This will only fail on the very first call to this function */
        if (RingOscCtrl1 & RING_OSC_IRQ)
        {
            AVSDEBUG("Read ring osc %ld: %lx\n",
                    (RingOscCtrl1 & RING_OSC_SELECT_MASK) >> RING_OSC_SELECT_SHIFT,
                     RingOscCtrl1 & RING_OSC_COUNT_MASK);
            if ((RingOscCtrl1 & RING_OSC_COUNT_MASK) > max_count)
            {
                max_count = RingOscCtrl1 & RING_OSC_COUNT_MASK;
                AVSDEBUG("max_count: %x\n", max_count);
            }

            /* Move to the next enabled ring osc */
            ring_osc_select = (RingOscCtrl1 & RING_OSC_SELECT_MASK) >> RING_OSC_SELECT_SHIFT;
            while (++ring_osc_select < RING_OSC_MAX)
            {
                if ((((1<<ring_osc_select)<<RING_OSC_ENABLE_SHIFT) & RING_OSC_ENABLE_MASK) != 0)
                {
                    break;
                }
            }

            /* If we have read all ring osc, determine if the voltage should be changed */
            if (ring_osc_select == RING_OSC_MAX)
            {
                /* All ring osc have been read, prepare for the next round */
                /* 0 is always a valid ring osc so no need to verify if it is enabled */
                ring_osc_select = 0;

                /* Check if the voltage should be adjusted */
                if (AvsEnabled == 1) {
                    ring_osc_upper_bound = RING_OSC_UPPER_BOUND;
                    vreg_sel1p2_lower_bound = VREG_VSEL1P2_LOWER_BOUND;
                } else {
                    ring_osc_upper_bound = DEEP_RING_OSC_UPPER_BOUND;
                    vreg_sel1p2_lower_bound = DEEP_VREG_VSEL1P2_LOWER_BOUND;
                }
                ring_osc_lower_bound = ring_osc_upper_bound - RING_OSC_LOWER_BOUND_OFFSET;
                
                current_1v2 = (MISC->miscVregCtrl1 & VREG_VSEL1P2_MASK) >> VREG_VSEL1P2_SHIFT;
                next_1v2 = current_1v2;

                if ((max_count < ring_osc_lower_bound) && AvsEnabled && !is_ss_part)
                {
                    /* The ring osc is too fast, reduce the voltage if it is not too low */
                    if (current_1v2 > vreg_sel1p2_lower_bound)
                    {
                        next_1v2 -= VREG_STEP_SIZE;
                    }

                    AVSDEBUG("ring_osc is fast, can reduce voltage: %d to %d\n", current_1v2, next_1v2);
                }
                if ((max_count > ring_osc_upper_bound) || !AvsEnabled || is_ss_part || (current_1v2 <= vreg_sel1p2_lower_bound-VREG_STEP_SIZE))
                {
                    /* The ring osc is too slow, increase the voltage up to the default of 0 */
                    /* If AVS is disabled, we need to force the voltage to come back up to default */
                    if (current_1v2 < MISC_VREG_CONTROL1_VSEL1P2_DEFAULT)
                    {
                        next_1v2 += VREG_STEP_SIZE;
                    }
                    AVSDEBUG("ring_osc is slow, can increase voltage: %d to %d\n", current_1v2, next_1v2);
                }

                if (next_1v2 != current_1v2)
                {
                    MISC->miscVregCtrl1 = (MISC->miscVregCtrl1 & ~VREG_VSEL1P2_MASK) | (next_1v2 << VREG_VSEL1P2_SHIFT);
                    AVSDEBUG("Adjusted voltage: %d to %d\n", current_1v2, next_1v2);
                }
                max_count = 0;
            }
        }         

        /* Start a new ring osc count cycle by resetting the counter */
        GPIO->RingOscCtrl1 = RING_OSC_ENABLE_MASK |
                             RING_OSC_COUNT_RESET;
        GPIO->RingOscCtrl1 = RING_OSC_ENABLE_MASK |
                             (ring_osc_select << RING_OSC_SELECT_SHIFT);
        /* Writing to this register starts the count */
        GPIO->RingOscCtrl0 = RING_OSC_512_CYCLES;
    }
}

int Avs65_Task(void *data)
{
	while(!kthread_should_stop())
	{
        brcm_adaptive_voltage_scaling();

        /* Sleep for 1 second (HZ jiffies) */
        msleep(HZ);
	}

   return(0);
}

