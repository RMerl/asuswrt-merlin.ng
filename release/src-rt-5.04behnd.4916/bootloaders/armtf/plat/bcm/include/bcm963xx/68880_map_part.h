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

#ifndef __BCM68880_MAP_PART_H
#define __BCM68880_MAP_PART_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bcmtypes.h"

#define TIMER_PHYS_BASE		0x84213000
#define TIMR_OFFSET		0x0700   /* 64 bit timer registers */
#define WDTIMR0_OFFSET		0x0728

#define GIC_PHYS_BASE		0x81000000
#define GIC_SIZE		0x10000
#define GIC_OFFSET		0x0000
#define GICD_OFFSET		0x1000
#define GICC_OFFSET		0x2000

#define SMC_BASE		0x84100000
#define SMC_SIZE		0x20000
#define DQM_STATUS_OFFSET	0x2800
#define DQM_MIB_OFFSET		0x3000
#define DQM_DATA_OFFSET		0x5000
#define DQM_Q_COUNT		32

#define WDTIMR0_BASE		(TIMER_PHYS_BASE + WDTIMR0_OFFSET)

#define WDT_HZ			50000000
#define WDT_START_1		(0xff00)
#define WDT_START_2		(0x00ff)
#define MICROSECS_TO_WDOG_TICKS(x) ((uint32_t)((x) * (WDT_HZ/1000000)))

#ifndef __ASSEMBLER__

/*
 * Timer
 */
#define TIMER_64BIT
typedef struct Timer {
    uint64        TimerCtl0; /* 0x00 */
    uint64        TimerCtl1; /* 0x08 */
    uint64        TimerCtl2; /* 0x10 */
    uint64        TimerCtl3; /* 0x18 */
#define TIMERENABLE     (1ULL << 63)
#define RSTCNTCLR       (1ULL << 62)
    uint64        TimerCnt0; /* 0x20 */
    uint64        TimerCnt1; /* 0x28 */
    uint64        TimerCnt2; /* 0x30 */
    uint64        TimerCnt3; /* 0x38 */
#define TIMER_COUNT_MASK   0x3FFFFFFFFFFFFFFFULL
    uint32        TimerMask; /* 0x40 */
#define TIMER0EN        0x01
#define TIMER1EN        0x02
#define TIMER2EN        0x04
#define TIMER3EN        0x08
    uint32        TimerInts; /* 0x44 */
#define TIMER0          0x01
#define TIMER1          0x02
#define TIMER2          0x04
#define TIMER3          0x08
    uint32        ResetReason;/* 0x4c */
    uint32        spare[3];   /* 0x50 - 0x5b */
    uint32        reserved1[9]; /* 0x5c - 0x7f */
} Timer;

#define TIMER ((volatile Timer * const) TIMR_BASE)


typedef struct WDTimer {
    uint32        WatchDogDefCount;
    /* Write 0xff00 0x00ff to Start timer
     * Write 0xee00 0x00ee to Stop and re-load default count
     * Read from this register returns current watch dog count
     */
    uint32        WatchDogCtl;

    /* Number of 50-MHz ticks for WD Reset pulse to last */
    uint32        WDResetCount;

    uint32        SoftRst;
#define SOFT_RESET              0x00000001 
    uint32        WDAccessCtl;
} WDTimer;

#define WDTIMER0 ((volatile WDTimer * const) WDTIMR0_BASE)

#endif /* __ASSEMBLER__ */

#ifdef __cplusplus
}
#endif

#endif /* __BCM68880_MAP_PART_H */
