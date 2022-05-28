#if defined(CONFIG_BCM_KF_BOUNCE) || !defined(CONFIG_BCM_IN_KERNEL)

#ifndef __BOUNCE_H_INCLUDED__
#define __BOUNCE_H_INCLUDED__

#if defined(CONFIG_BRCM_BOUNCE) || defined(CONFIG_BOUNCE)
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
/*
 *******************************************************************************
 * File Name   : bounce.h
 * Description : Tracing function call entry and exit, using compiler support
 *				 for instrumenting function entry and exit.
 *				 The GCC -finstrument-functions compiler option enables this.
 *
 *				 Files that need to be instrumented may be compiled with the
 *				 compiler option -finstrument-functions via the Makefile.
 *
 *				 To disable instrumentation of specific functions in a file
 *				 that is compiled with the option -finstrument-functions, you
 *				 may append __attribute__ ((no_instrument_function)) to it's
 *				 definition, e.g.
 *				 	void hello( void ) __attribute__ ((no_instrument_function));
 *
 *				 You may enable tracing by invoking bounce_up().
 *
 *				 Two modes of tracing are defined:
 *				 - Continuous tracing with an EXPLICIT bounce_dn() to stop.
 *				 - Auto stop, when a limited number of functions are logged.
 *                 bounce_dn() may also be invoked to stop in this mode.
 *
 *				 The collected trace is retained until the next start.
 *******************************************************************************
 */
#ifndef __ASSEMBLY__

#if defined(__cplusplus)
extern "C" {
#endif

#if defined( __KERNEL__ )
#include <linux/types.h>        /* ISO C99 7.18 Integer types */
#else
#include <stdint.h>             /* ISO C99 7.18 Integer types */
#endif

#define BOUNCE_ERROR				(-1)

#define BOUNCE_NOINSTR __attribute__((no_instrument_function))

#if defined(CONFIG_BOUNCE_EXIT)
#define BOUNCE_SIZE					(32*1024)
#define BOUNCE_PANIC				(32*1024)
#else
#define BOUNCE_SIZE					(256*1024)
#define BOUNCE_PANIC				20000
#endif

#define BOUNCE_COLOR

//#define BOUNCE_DEBUG
#ifdef BOUNCE_DEBUG
#define BDBG(code)      			code
#else
#define BDBG(code)					do {} while(0)
#endif

#define BOUNCE_VERSION(a,b,c)		(((a) << 16) + ((b) << 8) + ((c) << 0))
#define BOUNCE_VER_A(version)		((version >>16) & 0xff)
#define BOUNCE_VER_B(version)		((version >> 8) & 0xff)
#define BOUNCE_VER_C(version)		((version >> 0) & 0xff)

#define BOUNCE_DEV_VERSION			(BOUNCE_VERSION(01,00,00))
#define BOUNCE_CTL_VERSION			(BOUNCE_VERSION(01,00,00))

    /* Device name in : /proc/devices */
#define BOUNCE_DEV_NAME				"bounce"
#define BOUNCE_DEV_PATH          	"/dev/" BOUNCE_DEV_NAME
#define BOUNCE_DEV_MAJ           	213

#undef  BOUNCE_DECL
#define BOUNCE_DECL(x)				x,

typedef enum bounceMode
{
	BOUNCE_DECL(BOUNCE_MODE_DISABLED)
	BOUNCE_DECL(BOUNCE_MODE_CONTINUOUS)	/* explicit disable via bounce_dn() */
	BOUNCE_DECL(BOUNCE_MODE_LIMITED)	/* auto disable when count goes to 0 */
    BOUNCE_DECL(BOUNCE_MODE_MAXIMUM)
} BounceMode_t;

typedef enum bounceIoctl
{
	BOUNCE_DECL(BOUNCE_START_IOCTL)
	BOUNCE_DECL(BOUNCE_STOP_IOCTL)
	BOUNCE_DECL(BOUNCE_DUMP_IOCTL)
	BOUNCE_DECL(BOUNCE_INVLD_IOCTL)
} BounceIoctl_t;


#ifdef __KERNEL__

#define BOUNCE_ADDR_MASK			(0xFFFFFFFC)
#define BOUNCE_ARGS_MASK            (0xF0000000)
#define BOUNCE_GET_FUNCP(u32)       (void*)((u32) & BOUNCE_ADDR_MASK)

#define BOUNCE_IS_ARGS_LOG(u32)     (((u32) & BOUNCE_ARGS_MASK) == 0)
#define BOUNCE_IS_FUNC_LOG(u32)     (((u32) & BOUNCE_ARGS_MASK) != 0)

#define BOUNCE_MAX_EVENTS           1024
#define BOUNCE_FMT_LENGTH           126     /* Bytes in format string */
typedef struct bounceLog
{
	union {
        uint32_t u32;
		void * func;

        struct {
            uint32_t evid   : 16;
            uint32_t args   : 14;
            uint32_t cpu0   :  1;
            uint32_t type   :  1;
        } event;

		struct {
			uint32_t addr	: 30;
            uint32_t cpu0   :  1;   /* CPU0 or CPU1 */
			uint32_t type	:  1;	/* entry=1 or exit=0 */
		} site;

	} word0;						/* called function */

    union {
	    uint32_t pid; 				/* task context */
        uint32_t arg1;
    };
    uint32_t arg2;
    uint32_t arg3;
} BounceLog_t;


extern void	bounce_up(BounceMode_t mode, unsigned int limit);
extern void bounce_dn(void);
extern void bounce_panic(void);
extern void	bounce_dump(unsigned int last);

extern void __cyg_profile_func_enter(void *ced, void *cer) BOUNCE_NOINSTR;
extern void __cyg_profile_func_exit( void *ced, void *cer) BOUNCE_NOINSTR;

extern void bounce_reg(uint32_t event, char * eventName)        BOUNCE_NOINSTR;
extern void bounce0(uint32_t event)                             BOUNCE_NOINSTR;
extern void bounce1(uint32_t event, uint32_t arg1)              BOUNCE_NOINSTR;
extern void bounce2(uint32_t event, uint32_t arg1, uint32_t arg2)
                                                                BOUNCE_NOINSTR;
extern void bounce3(uint32_t event, uint32_t arg1, uint32_t arg2, uint32_t arg3)
                                                                BOUNCE_NOINSTR;

#define			BOUNCE_LOGK(func)	__cyg_profile_func_enter((void*)func,	\
								 			__builtin_return_address(0))
#endif	/* defined __KERNEL__ */

#if defined(__cplusplus)
}
#endif

#endif  /* defined __ASSEMBLY__ */


#else	/* !defined(CONFIG_BRCM_BOUNCE) */

#define			bounce_up(m,l)		do {} while(0)
#define			bounce_dn()			do {} while(0)
#define			bounce_dump(l)		do {} while(0)
#define			bounce_panic()		do {} while(0)
#define         bounce0(e)          do {} while(0)
#define         bounce1(e,a)        do {} while(0)
#define         bounce2(e,a,b)      do {} while(0)
#define         bounce3(e,a,b,c)    do {} while(0)

#define			BOUNCE_LOGK(f)		do {} while(0)

#endif	/* #defined(CONFIG_BRCM_BOUNCE) */

#endif	/* !defined(__BOUNCE_H_INCLUDED__) */

#endif	/* defined(CONFIG_BCM_KF_BOUNCE) || !defined(CONFIG_BCM_IN_KERNEL) */
