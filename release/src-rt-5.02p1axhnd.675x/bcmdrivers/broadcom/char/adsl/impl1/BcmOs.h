/* 
* <:copyright-BRCM:2002:proprietary:standard
* 
*    Copyright (c) 2002 Broadcom 
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
* :>
*/

//**************************************************************************
// File Name  : BcmOs.c
//
// Description: This is the header file for Linux OS calls
//
//**************************************************************************
#if !defined(_BCMOS_H_)
#define _BCMOS_H_

#if defined(_WIN32_WCE)

typedef	unsigned char	Bool;
typedef ULONG (*FN_ISR) (ULONG);
#ifdef DEBUG
    #define DBG_MSG		1
#else
    #define DBG_MSG		0
#endif

#elif defined(_CFE_)

#include "lib_types.h"
#include "cfe_timer.h"
#include "lib_printf.h"

#elif defined(__ECOS)

#elif defined(_NOOS)

#include "stdarg.h"
#include "bcmtypes.h"
#ifdef __ARM_CHIP_SIM_ENVIRONMENT	/* define this when build in CHIP simulation environment */
#include "armPrint.h"
#endif

#else

#include <asm/param.h>
#ifndef _SYS_TYPES_H
#include <linux/types.h>
#define	_SYS_TYPES_H
#include <linux/version.h>
#endif

#endif /* _WIN32_WCE || CFE */

#define LOCAL static 

typedef unsigned long   OS_SEMID;       /* Linux semaphone ID type */
typedef unsigned long   OS_TIME;        /* Linux time type */
typedef unsigned long   OS_TICKS;       /* Linux tick type */
typedef unsigned long   OS_TASKID;      /* Linux task ID type */
typedef unsigned long   OS_TASKARG;     /* Linux task start argument */
typedef unsigned long   OS_STATUS;      /* Linux return status type */

#define OS_STATUS_OK  1
#define OS_STATUS_FAIL  0
#define OS_STATUS_TIMEOUT -1

extern int	gConsoleOutputEnable;

#ifdef _WIN32_WCE
void NKDbgPrintfW(void *pwStr, ...);
#ifndef TEXT
#define TEXT(quote)			L##quote
#endif
#define AdslDrvPrintf		NKDbgPrintfW
#define HZ					1000
#define BCMOS_EVENT_LOG(x)	NKDbgPrintfW x
#define KERN_CRIT
#define BCMOS_DECLARE_IRQFLAGS(f)
#define BCMOS_SPIN_LOCK_IRQ(l, f)
#define BCMOS_SPIN_UNLOCK_IRQ(l, f)
#elif defined(_CFE_)
#define HZ					CFE_HZ
#define TEXT(__str__)		__str__
#define BCMOS_EVENT_LOG(x)	printf x
#define KERN_CRIT
#define printk					printf
#define AdslDrvPrintf		printf
#define BCMOS_DECLARE_IRQFLAGS(f)
#define BCMOS_SPIN_LOCK_IRQ(l, f)
#define BCMOS_SPIN_UNLOCK_IRQ(l, f)
#elif defined(__ECOS)
#define HZ					1000
#define TEXT(__str__)		__str__
#define BCMOS_EVENT_LOG(x)	diag_printf x
#define KERN_CRIT
#define printk					diag_printf
#define AdslDrvPrintf		diag_printf
#define BCMOS_DECLARE_IRQFLAGS(f)
#define BCMOS_SPIN_LOCK_IRQ(l, f)
#define BCMOS_SPIN_UNLOCK_IRQ(l, f)
#elif defined(_NOOS)
#define HZ					1000	/* Re-adjust depending on target chip speed */
#define KERN_CRIT
#define printk(x)
#define	TEXT(__str__)		__str__
#define	AdslDrvPrintf	print_log
#define BCMOS_EVENT_LOG(x)
#define BCMOS_DECLARE_IRQFLAGS(f)
#define BCMOS_SPIN_LOCK_IRQ(l, f)
#define BCMOS_SPIN_UNLOCK_IRQ(l, f)
#elif defined(TARG_OS_RTEMS)
#define TEXT(__str__)		__str__
#define AdslDrvPrintf		BcmOsEventLog
#elif defined(VXWORKS)
#define	TEXT(__str__)		__str__
#define	AdslDrvPrintf		printf
#else	/* __KERNEL__ */
#undef TEXT
#define	TEXT(__str__)		__str__
#define	AdslDrvPrintf		printk
#define	PHY_PROFILE_SUPPORT
#define BCMOS_EVENT_LOG(x)	if(gConsoleOutputEnable) printk x
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29))
#define BCMOS_DECLARE_IRQFLAGS(f)        unsigned long (f)
#define BCMOS_SPIN_LOCK_IRQ(l, f)        spin_lock_irqsave(l, f)
#define BCMOS_SPIN_UNLOCK_IRQ(l, f)      spin_unlock_irqrestore(l, f)
#else
#define BCMOS_DECLARE_IRQFLAGS(f)
#define BCMOS_SPIN_LOCK_IRQ(l, f)
#define BCMOS_SPIN_UNLOCK_IRQ(l, f)
#endif
#endif /* _WIN32_WCE */

#define BCMOS_MSEC_PER_TICK (1000/HZ)
#define OS_WAIT_FOREVER  (OS_TIME) -1

OS_STATUS BcmOsInitialize( void );
OS_STATUS BcmOsUninitialize( void );
OS_STATUS bcmOsTaskCreate( char* name, int stackSize, int priority,
                       void* taskEntry, OS_TASKARG argument, OS_TASKID *taskId );
OS_STATUS bcmOsTaskDelete( OS_TASKID taskId );
OS_STATUS bcmOsSemCreate(char *semName, OS_SEMID *semId);
OS_STATUS bcmOsSemDelete( OS_SEMID semId );
OS_STATUS bcmOsSemTake(OS_SEMID semId, OS_TIME timeout);
OS_STATUS bcmOsSemGive( OS_SEMID semId );
OS_STATUS bcmOsGetTime(OS_TICKS *osTime);
OS_STATUS bcmOsSleep( OS_TIME timeout );

void * bcmOsDpcCreate(void* dpcEntry, void * arg);
void bcmOsDpcEnqueue(void* dpcHandle);
void * bcmOsTimerCreate(void* timerEntry, void * arg);
void bcmOsTimerStart(void* timerHandle, int timeout);
void bcmOsTimerStop(void* timerHandle);
void bcmOsDelay(unsigned long timeMs);
void bcmOsWakeupMonitorTask(void);

#ifdef _WIN32_WCE
void BcmHalInterruptEnable (int intrId);
void BcmHalInterruptDisable (int intrId);
void BcmHalMapInterrupt(FN_ISR pIntrHandler, void* param, int intrId);
#elif defined(_NOOS)
// /tools/arm/RVDS_4.1_sp1_build894/RVCT/Data/4.1/561/include/unix/stdlib.h provides free()/calloc()
extern unsigned int get_cpuid(void);
extern void stop_TEST(unsigned int core_id);
extern void reset_TEST_timeout(unsigned int core_id);
#ifndef __ARM_CHIP_SIM_ENVIRONMENT
void print_log(char *fmt, ...);
void *calloc( unsigned long num, unsigned long size );
void free( void *memblock );
void *memset(void *str, int c, long n);
void *memcpy(void *str1, const void *str2, long n);
int sprintf(char *str, const char *format, ...);
#endif

#else
void *calloc( unsigned long num, unsigned long size );
void free( void *memblock );
#endif

#if defined(__ECOS)
#define vmalloc(sz) calloc(1,sz)
#define vfree(p)    free(p)
#endif

#endif // _BCMOS_H_
