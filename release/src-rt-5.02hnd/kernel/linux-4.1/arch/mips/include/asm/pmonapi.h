#if defined(CONFIG_BCM_KF_PMON) || !defined(CONFIG_BCM_IN_KERNEL)

#ifndef __PMONAPI_H_INCLUDED_
#define __PMONAPI_H_INCLUDED_
/*
<:copyright-BRCM:2007:DUAL/GPL:standard

   Copyright (c) 2007 Broadcom 
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

#if defined( CONFIG_PMON )

#if ! defined( __ASSEMBLY__ )

#if defined(__cplusplus)
extern "C" {
#endif

#if defined( __KERNEL__ )
#include <linux/types.h>            /* ISO C99 7.18 Integer types */
#include <asm/mipsregs.h>
#include <bcm_cpu.h>
#else
#include <stdint.h>                 /* ISO C99 7.18 Integer types */
#endif /* __KERNEL__ */

#define PMON_ERROR                  (-1)
#define PMON_COLOR
// #define PMON_DEBUG

#if defined( PMON_DEBUG )
#define PMONDBG(code)           code
#else
#define PMONDBG(code)           do {} while(0)
#endif  /* PMON_DEBUG */

#define PMON_VERSION(a,b,c)     (((a) << 16) + ((b) << 8) + ((c) << 0))
#define PMON_VER_A(version)     ((version >>16) & 0xff)
#define PMON_VER_B(version)     ((version >> 8) & 0xff)
#define PMON_VER_C(version)     ((version >> 0) & 0xff)

#define PMON_DEV_VERSION        (PMON_VERSION(01,00,00))
#define PMON_CTL_VERSION        (PMON_VERSION(01,00,00))

/* Device name in : /dev */
#define PMON_DEV_NAME           "pmon"
#define PMON_DEV_PATH           "/dev/" PMON_DEV_NAME
#define PMON_DEV_MAJ            214

// #define PMON_RAC_METRIC

#undef  PMON_DECL
#define PMON_DECL(x)            x,

typedef enum PmonIoctl
{
    PMON_DECL(PMON_CPU_START_IOCTL)
    PMON_DECL(PMON_ALL_START_IOCTL)
    PMON_DECL(PMON_REPORT_IOCTL)
    PMON_DECL(PMON_INVLD_IOCTL)
} PmonIoctl_t;

#if defined( __KERNEL__ )
/*
 * Enable PMON with configuration:
 *    skip: Delayed enabling after a number of iterations
 *    iter: Average over number of iteration per metric
 *    metric: Compute all=1 or only cyclecount=0 metrics
 *
 * An iteration could be a single packet processing path.
 */
#define PMON_MAX_EVENTS         32
#define PMON_DEF_UNREGEVT       "Unregistered Event"

/*
 * MIPS3300 and MIPS4350 performance Counting Module configuration
 * Only Performance counter #0 is used.
 */
#define __read_cycles()         __read_32bit_c0_register($9, 0)

#if defined( CONFIG_BCM96362 ) ||      defined( CONFIG_BCM96328 ) ||                defined(CONFIG_BCM963268) ||                                                    defined(CONFIG_BCM96838)

/* MIPS4350 Performance Counters */
#define PF_GBLCTL               0xBFA20000u
#define PF_CTL_0                0xBFA20004u
#define PF_CTL_1                0xBFA20008u
#define PF_CTR_0                0xBFA20010u
#define PF_CTR_1                0xBFA20014u
#define PF_CTR_2                0xBFA20018u
#define PF_CTR_3                0xBFA2001Cu

#define MIPS4350_ADDR(reg)      (((reg-MIPS_BASE_BOOT) + MIPS_BASE))

#define MIPS4350_RD(reg)        (*(volatile uint32_t *)(MIPS4350_ADDR(reg)))
#define MIPS4350_WR(reg,v)      (*(volatile uint32_t *)(MIPS4350_ADDR(reg)))=(v)

#define __read_pfgblctl()       MIPS4350_RD( PF_GBLCTL )
#define __read_pfctl_0()        MIPS4350_RD( PF_CTL_0 )
#define __read_pfctr_0()        MIPS4350_RD( PF_CTR_0 )
#define __write_pfgblctl(val)   MIPS4350_WR( PF_GBLCTL, val )
#define __write_pfctl_0(val)    MIPS4350_WR( PF_CTL_0, val )
#define __write_pfctl_1(val)    MIPS4350_WR( PF_CTL_1, val )
#define __write_pfctr_0(val)    MIPS4350_WR( PF_CTR_0, val )

#else /* MIPS ???? */

#define __read_pfgblctl()       0
#define __read_pfctl_0()        0
#define __read_pfctr_0()        0
#define __write_pfgblctl(val)   FUNC_NULL
#define __write_pfctl_0(val)    FUNC_NULL
#define __write_pfctr_0(val)    FUNC_NULL

#endif  /* MIPS: */

#define PMON_OVFLW_EVENTS       1024

extern uint32_t pfCtr[PMON_OVFLW_EVENTS];
extern void pmon_bgn(void);             /* Begin an iteration */
extern void pmon_end(uint32_t event);   /* End of an iteration */
static inline void pmon_log(uint32_t event) { pfCtr[event] = __read_pfctr_0(); }
static inline void pmon_clr(void) { pfCtr[0] = ~0U; }
extern int  pmon_enable(uint32_t skip, uint32_t iter, uint32_t metric);
extern void pmon_reg(uint32_t event, char * name);

#endif  /* __KERNEL__ */

#if defined(__cplusplus)
}
#endif

#endif  /* __ASSEMBLY__ */

#else   /* ! CONFIG_PMON */

#undef  FUNC_NULL
#define FUNC_NULL               do {} while(0)

#define pmon_log(e)             FUNC_NULL
#define pmon_clr()              FUNC_NULL
#define pmon_bgn()              FUNC_NULL
#define pmon_end(e)             FUNC_NULL
#define pmon_enable(s,i,m)      0
#define pmon_reg(e,n)           0

#endif  /* ! CONFIG_PMON */

#endif  /* __PMONAPI_H_INCLUDED_ */

#endif
