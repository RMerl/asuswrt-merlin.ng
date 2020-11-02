/*
 * HND Run Time Environment Assert Facility.
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id: rte_assert.h 697624 2017-05-04 09:54:45Z $
 */

#ifndef _rte_assert_h_
#define _rte_assert_h_

#include <typedefs.h>

extern uint32 g_assert_type;

/* accessor functions */
extern uint32 get_g_assert_type(void);
extern void set_g_assert_type(uint32 val);

void hnd_assert(const char *file, int line);

/* assertion */
#if defined(BCMDBG_ASSERT)
/* for cortex-Mx, cortex-Rx and cortex-Ax processors */
#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7A__)

#include <hnd_armtrap.h>

/* Use the system service call (SVC) instruction to generate a software
 * interrupt for a failed assert. This will use a 2-byte instruction to
 * generate the trap. It is more memory efficient than the alternate C
 * implementation which may use more than 2-bytes to generate the trap.
 * It also allows the trap handler to uniquely identify the trap as an
 * assert (since this is the only use of software interrupts in the system).
 */
#define ASSERT_WITH_TRAP(exp) do { \
		if (!(exp)) { \
		asm volatile(\
		"SVC #"STR(ASSERT_TRAP_SVC_NUMBER)\
		:\
		:\
		: "memory"); \
		} \
	} while (0)
#else /* !__ARM_ARCH_7M__ */
#define ASSERT_WITH_TRAP(exp) do { \
		if (!(exp)) { \
			int *null = NULL; \
			*null = 0; \
		} \
	} while (0)
#endif /* __ARM_ARCH_7M__ */
#if defined(BCMDBG_ASSERT_TRAP)
/* DBG_ASSERT_TRAP causes a trap/exception when an ASSERT fails, instead of calling
 * an assert handler to log the file and line number. This is a memory optimization
 * that eliminates the strings associated with the file/line and the function call
 * overhead associated with invoking the assert handler. The assert location can be
 * determined based upon the program counter displayed by the trap handler.
 */
#define ASSERT(exp) ASSERT_WITH_TRAP(exp)
#else /* !BCMDBG_ASSERT_TRAP */
#ifndef _FILENAME_
#define _FILENAME_ "_FILENAME_ is not defined"
#endif // endif
#define ASSERT(exp) do { \
		if (!(exp)) { \
			hnd_assert(_FILENAME_, __LINE__); \
		} \
	} while (0)
#endif /* BCMDBG_ASSERT_TRAP */
#else
#define	ASSERT(exp)	do {} while (0)
#endif // endif

#ifdef DONGLEBUILD
#define ROMMABLE_ASSERT(exp) do { \
		if (!(exp)) { \
			int *null = NULL; \
			*null = 0; \
		} \
	} while (0)
#endif /* DONGLEBUILD */

#endif /* _rte_assert_h_ */
