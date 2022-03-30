/*
<:copyright-BRCM:2015:DUAL/GPL:standard 

   Copyright (c) 2015 Broadcom 
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

#ifndef __BCM6855_MAP_PART_H
#define __BCM6855_MAP_PART_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bcmtypes.h"

#define MEMC_PHYS_BASE		0x80180000
#define MEMC_SIZE		0x24000

#define PMC_PHYS_BASE		0xffb20000
#define PMC_SIZE		0x1000
#define PMB_OFFSET		0x100

#define PERF_PHYS_BASE		0xff800000
#define PERF_SIZE		0x14000
#define TIMR_OFFSET		0x0400
#define WDTIMR0_OFFSET		0x0480
#define WDTIMR1_OFFSET		0x04c0

#define BIUCFG_PHYS_BASE	0x81060000
#define BIUCFG_SIZE		0x3000
#define BIUCFG_OFFSET		0x0000

#define BOOTLUT_PHYS_BASE	0xffff0000
#define BOOTLUT_SIZE		0x1000


#define GIC_PHYS_BASE           0x81000000
#define GIC_SIZE                0x10000
#define GIC_OFFSET              0x0000
#define GICD_OFFSET             0x1000
#define GICC_OFFSET             0x2000


#define PMC_BASE		(PMC_PHYS_BASE + 0)
#define PMB_BASE		(PMC_PHYS_BASE + PMB_OFFSET)

#define WDTIMR0_BASE		(PERF_PHYS_BASE + WDTIMR0_OFFSET)
#define TIMR_BASE		(PERF_PHYS_BASE + TIMR_OFFSET)
#define BIUCFG_BASE		(BIUCFG_PHYS_BASE + BIUCFG_OFFSET)

#ifndef __ASSEMBLER__

/*
 * Power Management Control
 */

typedef union
{
    struct
    {
        uint32 propagate_to_err  :  1; // [00:00] -+
        uint32 propagate_slv_err :  1; // [01:01]  | - these are potentially dangerous and MAY cause a system crash
        uint32 pmbus_reset_n     :  1; // [02:02] -+
        uint32 reserved0         :  1; // [03:03]
        uint32 maxPmbIdx         :  3; // [06:04] 0-based (0-7)
        uint32 reserved1         :  1; // [07:07]
        uint32 maxClientId       : 12; // [19:08] 0-based (theoreticaly 0-4095, but code limits this to 256 devices - 0-255)
        uint32 numRegsPerClient  : 10; // [29:20] some power of 2 - number of 32-bit registers in each client (max = 512)
        uint32 startDiscovery    :  1; // [30:30] kicks off H/W discovery of clients and fills in the map (see PMB_REGS below)
        uint32 discoveryBusy     :  1; // [31:31] whether or not H/W discovery is still busy creating the map
    } Bits;
    uint32 Reg32;
} PMB_CONFIG_REG;

typedef union
{
    struct {
        uint32 data      : 16; // [15:00]
        uint32 reserved1 : 16; // [31:16]
    } Bits;
    uint32 Reg32;
} SSBM_data_reg;

typedef union
{
    struct {
        uint32  ssb_addr    : 10; // [09:00]
        uint32  ssb_cmd     :  2; // [11:10]
        uint32  ssb_en      :  1; // [12:12]
        uint32  ssb_add_pre :  1; // [13:13]
        uint32  reserved2   :  1; // [14:14]
        uint32  ssb_start   :  1; // [15:15]
        uint32  reserved1   : 16; // [31:16]
    } Bits;
    uint32 Reg32;
} SSBM_control_reg;

typedef union
{
    struct {
        uint32 busy      :  1; // [00:00]
        uint32 reserved1 : 31; // [31:01]
    } Bits;
    uint32 Reg32;
} SSBM_status_reg;

typedef union
{
    struct {
        uint32 swreg_th_lo : 8; // [07:00]
        uint32 swreg_th_hi : 8; // [15:08]
        uint32 reserved    :16; // [31:16]
    } Bits;
    uint32 Reg32;
} SSBM_SWREG_th_hilo_reg;


typedef union
{
    struct {
        uint32 ssb_lock_addr : 10; // [09:00]
        uint32 lock_bit      :  1; // [10:10]
        uint32 lock_mode     :  1; // [11:11]
        uint32 reserved      : 20; // [31:12]
    } Bits;
    uint32 Reg32;
} SSBM_SWREG_lock_reg;

#define kSSBWrite   0x01
#define kSSBRead    0x02
#define kSSBEn      (1 << 12)
#define kSSBStart   (1 << 15)

typedef struct SSBMaster {
    SSBM_control_reg        ssbmControl;    /* 0x0060 */
    SSBM_data_reg           ssbmWrData;     /* 0x0064 */
    SSBM_data_reg           ssbmRdData;     /* 0x0068 */
    SSBM_status_reg         ssbmStatus;     /* 0x006c */
    SSBM_SWREG_th_hilo_reg  ssbmThHiLo;     /* 0x0070 */
    SSBM_SWREG_lock_reg     ssbmSwLock;     /* 0x0074 */
} SSBMaster;

typedef struct PmmReg {
    uint32 memPowerCtrl;            /* 0x0000 */
    uint32 regSecurityConfig;       /* 0x0004 */
} PmmReg;

typedef struct keyholeReg {
    uint32 ctrlSts;
    uint32 wrData;
    uint32 mutex;
    uint32 rdData;
} keyholeReg;

typedef struct PmbBus {
    PMB_CONFIG_REG config;          /* 0x0100 */
    uint32 arbiter;         /* 0x0104 */
    uint32 timeout;         /* 0x0108 */
    uint32 unused1;         /* 0x010c */
    keyholeReg keyhole[4];  /* 0x0110-0x014f */
    uint32 unused2[44];     /* 0x0150-0x01ff */
    uint32 map[64];         /* 0x0200-0x02ff */ 
}PmbBus;

typedef struct Pmc {
    PmmReg  pmm;                        /* 0x20000 */
    uint32 unused11[22];                /* 0x20008-0x2005f */
    SSBMaster ssbMasterCtrl;            /* 0x20060-0x20077 */
    uint32 unused12[34];                /* 0x20078-0x200ff */
    PmbBus pmb;                         /* 0x20100 */
} Pmc;
#define PMC ((volatile Pmc * const) PMC_BASE)

typedef struct
{
    uint32  control;
#define PMC_PMBM_START                  (1 << 31)
#define PMC_PMBM_TIMEOUT                (1 << 30)
#define PMC_PMBM_SLAVE_ERR              (1 << 29)
#define PMC_PMBM_BUSY                   (1 << 28)
#define PMC_PMBM_BUS_SHIFT              (20)
#define PMC_PMBM_Read                   (0 << 24)
#define PMC_PMBM_Write                  (1 << 24)
    uint32  wr_data;
    uint32  mutex;
    uint32  rd_data;
} PMB_keyhole_reg;

typedef struct PMBMaster {
    uint32 config;
#define PMB_NUM_REGS_SHIFT (20)
#define PMB_NUM_REGS_MASK  (0x3ff)
    uint32 arbitger;
    uint32 timeout;
    uint32 reserved;
    PMB_keyhole_reg keyhole[4];
    uint32 reserved1[44];
    uint32 map[64];
} PMBMaster;
#define PMB ((volatile PMBMaster * const) PMB_BASE)

/*
** Timer
*/
#define TIMER_64BIT
typedef struct Timer {
    uint64        TimerCtl0;
    uint64        TimerCtl1;
    uint64        TimerCtl2;
    uint64        TimerCtl3;
#define TIMERENABLE     (1ULL << 63)
#define RSTCNTCLR       (1ULL << 62)
    uint64        TimerCnt0;
    uint64        TimerCnt1;
    uint64        TimerCnt2;
    uint64        TimerCnt3;
#define TIMER_COUNT_MASK   0x3FFFFFFFFFFFFFFFULL
    uint32        TimerMask;
#define TIMER0EN        0x01
#define TIMER1EN        0x02
#define TIMER2EN        0x04
#define TIMER3EN        0x08
    uint32        TimerInts;
#define TIMER0          0x01
#define TIMER1          0x02
#define TIMER2          0x04
#define TIMER3          0x08
    uint32        ResetReason;
#define SW_INI_RESET            0x00000001
    uint32        spare[3];
} Timer;

typedef struct WDTimer {
    uint32        WatchDogDefCount;/* Write 0xff00 0x00ff to Start timer
     * Write 0xee00 0x00ee to Stop and re-load default count
     *      *      * Read from this register returns current watch dog count
     *           *           */
    uint32        WatchDogCtl;

    /* Number of 50-MHz ticks for WD Reset pulse to last */
    uint32        WDResetCount;

    uint32        WDTimerCtl;
#define SOFT_RESET              0x00000001
    uint32        WDAccessCtl;
} WDTimer;

#define TIMER ((volatile Timer * const) TIMR_BASE)
#define WDTIMER0 ((volatile WDTimer * const) WDTIMR0_BASE)

typedef struct BIUCFG_Access {
    uint32 permission;        /* 0x0 */
    uint32 revd0;             /* 0x4 */
    uint32 cpu_defeature;     /* 0x8 */
    uint32 dbg_security;      /* 0xc */
    uint32 rsvd1[36];         /* 0x10 - 0x9f */
    uint32 ts_access;         /* 0xa0 - 0xa3 */
    uint32 rsvd2[23];         /* 0xa4 - 0xff */
}BIUCFG_Access;


typedef struct BIUCFG_Cluster {
    uint32 permission;        /* 0x0 */
    uint32 config;            /* 0x4 */
    uint32 status;            /* 0x8 */
    uint32 control;           /* 0xc */
    uint32 cpucfg;            /* 0x10 */
    uint32 dbgrom;            /* 0x14 */
    uint32 rsvd1[2];          /* 0x18 - 0x1f */
    uint64 rvbar_addr[4];     /* 0x20 - 0x3f */
    uint32 rsvd2[48];         /* 0x40 - 0xff */
}BIUCFG_Cluster;

typedef struct BIUCFG {
    BIUCFG_Access access;         /* 0x0 - 0xff*/
    BIUCFG_Cluster cluster[2];    /* 0x100 - 0x2ff*/
    uint32 anonymous[2880];       /* 0x300 - 0x2fff*/
}BIUCFG;
#define BIUCFG ((volatile BIUCFG * const) BIUCFG_BASE)


#endif /* __ASSEMBLER__ */

#ifdef __cplusplus
}
#endif

#endif

