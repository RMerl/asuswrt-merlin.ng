/*
<:copyright-BRCM:2019:DUAL/GPL:standard

   Copyright (c) 2019 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/

#ifndef PLATFORM_SUPPORT_63138_H
#define PLATFORM_SUPPORT_63138_H

/***************************************************************************
 *GT - GOBAL TIMER
 ***************************************************************************/
#define GT_COUNTER_LOW_OFFSET          0x00000000   /* Global Timer Counter Register Low */
#define GT_COUNTER_HIGH_OFFSET         0x00000004   /* Global Timer Counter Register High */
#define GT_CONTROL_OFFSET              0x00000008   /* Global Timer Control Register */
#define GT_CVAL_LOW_OFFSET             0x00000010   /* Global Timer Comparator Value Register Low */
#define GT_CVAL_HIGH_OFFSET            0x00000014   /* Global Timer Comparator Value Register High */
#define GT_INT_STATUS_OFFSET           0x0000000c   /* Global Timer Interrupt Status Register */

#define GT_CONTROL_IRQ_ENABLE          (1 << 2)     /* Global Timer IRQ Enable */
#define GT_CONTROL_COMP_ENABLE         (1 << 1)     /* Global Timer Comparator Enable */
#define GT_CONTROL_TIMER_ENABLE        (1 << 0)     /* Global Timer Enable */
#define GT_INT_STATUS_EVENT_FLAG       (1 << 0)     /* Event Flag */

#define GT_CONTROL_DISABLE (GT_CONTROL_COMP_ENABLE) /* do NOT disable the timer. */
#define GT_CONTROL_ENABLE  (GT_CONTROL_COMP_ENABLE | GT_CONTROL_TIMER_ENABLE)
#define GT_COUNTER_FREQ  50000000
/***************************************************************************
 *PMU - Performance Monitor Unit
 ***************************************************************************/
#define PMCCNTR_ENABLE                 (1 << 31)  // enable PMCCNTR
#define PMC_RESET_PMCCNTR              (1 << 2)   // Reset PMCCNTR to zero.
#define PMC_COUNTERS_ENABLE            (1 << 0)   // All counters are enabled.
#define PRESERVE_PMCCNTR 0xFFFFFFFFU              // do not set PMCCNTR

#define SCU_BASE                       0x8001E000
#define SCU_INV_CTRL_INIT              0xFFFF
#define SCU_SAC_CTRL_INIT              0xF
#define SCU_NSAC_CTRL_INIT             0xFFF
#define SCU_INV_SEC                    0x0C
#define SCU_SAC                        0x50
#define SCU_NSAC                       0x54
#define SCU_CTRL                       0x0
#endif /* PLATFORM_SUPPORT_63138_H */
