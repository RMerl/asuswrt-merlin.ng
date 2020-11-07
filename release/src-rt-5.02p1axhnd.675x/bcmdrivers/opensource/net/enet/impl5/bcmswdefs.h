/*
<:copyright-BRCM:2004:DUAL/GPL:standard

   Copyright (c) 2004 Broadcom 
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

#ifndef _BCMSWDEFS_H_
#define _BCMSWDEFS_H_

#define MAX_VLANS 4096

#define DMA_CFG ((volatile uint32 * const) SWITCH_DMA_CONFIG)
#define DMA_STATE ((volatile uint32 * const) SWITCH_DMA_STATE)
/* Advertise 100BaseTxFD/HD and 10BaseTFD/HD */
#define AN_ADV_ALL 0x1E1
/* Advertise 1000BaseTFD/HD */
#define AN_1000BASET_CTRL_ADV_ALL 0x300

/* For USB loopback, enable rx and tx of swpktbus and set the rx_id different
   from tx_id */
#define USB_SWPKTBUS_LOOPBACK_VAL 0x70031
#define LINKDOWN_OVERRIDE_VAL 0x4B

#if defined(CONFIG_BCM_SWITCH_SCHED_WRR)
#define DEFAULT_HQ_PREEMPT_EN    0 
#else //if defined(CONFIG_BCM_SWITCH_SCHED_SP)
#define DEFAULT_HQ_PREEMPT_EN    1
#endif

/* 6829 Queue Thresholds */
#define BCM6829_PRIQ_HYST        0x220
#define BCM6829_PRIQ_PAUSE       0x2E0
#define BCM6829_PRIQ_DROP        0x2F0
#define BCM6829_PRIQ_LOWDROP     0x40
#define BCM6829_TOTAL_HYST       0x230
#define BCM6829_TOTAL_PAUSE      0x2F0
#define BCM6829_TOTAL_DROP       0x300  

/*
 * NOTE : These default buffer thresholds are duplicated in SWMDK as well. Check files bcm6xxx_a0_bmd_init.c 
*/
#if !defined(CONFIG_BCM_SWMDK)
#if defined(CONFIG_BCM963268)
/* These FC thresholds are based on 0x200 buffers available in the switch */
#define DEFAULT_TOTAL_DROP_THRESHOLD           0x1FF
#define DEFAULT_TOTAL_PAUSE_THRESHOLD          0x1FF
#define DEFAULT_TOTAL_HYSTERESIS_THRESHOLD     0x1F0 
#define DEFAULT_TXQHI_DROP_THRESHOLD           0x78
#define DEFAULT_TXQHI_PAUSE_THRESHOLD          0x70
#define DEFAULT_TXQHI_HYSTERESIS_THRESHOLD     0x68

#else /* 6328 and 6318 */

/* These FC thresholds are based on 0x100 buffers available in the switch */
#define DEFAULT_TOTAL_DROP_THRESHOLD           0xFF
#define DEFAULT_TOTAL_PAUSE_THRESHOLD          0xD0
#define DEFAULT_TOTAL_HYSTERESIS_THRESHOLD     0xA0
#define DEFAULT_TXQHI_DROP_THRESHOLD           0x3D
#define DEFAULT_TXQHI_PAUSE_THRESHOLD          0x2D
#define DEFAULT_TXQHI_HYSTERESIS_THRESHOLD     0x1D
#endif
#endif /* !CONFIG_BCM_SWMDK */

#endif /* _BCMSWDEFS_H_ */
