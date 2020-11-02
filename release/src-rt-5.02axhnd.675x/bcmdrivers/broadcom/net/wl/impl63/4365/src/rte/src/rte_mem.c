/** @file hnd_mem.c
 *
 * HND memory/image layout.
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
 * $Id: rte_mem.c 627697 2016-03-28 06:15:02Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <hnd_pktpool.h>
#include <rte_mem.h>
#include "rte_mem_priv.h"
#include <rte_cons.h>
#include "rte_heap_priv.h"

/* debug */
#ifdef BCMDBG
#define HND_MSG(x) printf x
#else
#define HND_MSG(x)
#endif // endif

/** This function must be forced into RAM since it uses RAM specific linker symbols.  */
void
BCMRAMFN(hnd_image_info)(hnd_image_info_t *i)
{
	memset(i, 0, sizeof(*i));

	i->_text_start = text_start;
	i->_text_end = text_end;
	i->_rodata_start = rodata_start;
	i->_rodata_end = rodata_end;
	i->_data_start = data_start;
	i->_data_end = data_end;
	i->_bss_start = bss_start;
	i->_bss_end = bss_end;

#ifdef BCMRECLAIM
	{
		extern char _rstart1[], _rend1[];
		i->_reclaim1_start = _rstart1;
		i->_reclaim1_end = _rend1;
	}
#endif // endif

#ifdef DONGLEBUILD
	{
		extern char _rstart2[], _rend2[];
		extern char _rstart3[], _rend3[];

		i->_reclaim2_start = _rstart2;
		i->_reclaim2_end = _rend2;
		i->_reclaim3_start = _rstart3;
		i->_reclaim3_end = _rend3;
	}

#if defined(BCMROMOFFLOAD)
	{
		extern char _rstart4[], _rend4[];
		extern char bootloader_patch_start[], bootloader_patch_end[];

		i->_reclaim4_start   = _rstart4;
		i->_reclaim4_end     = _rend4;
		i->_boot_patch_start = bootloader_patch_start;
		i->_boot_patch_end   = bootloader_patch_end;
	}
#endif /* BCMROMOFFLOAD */

	{
		extern char _rstart5[], _rend5[];
		extern char ucodes_start[], ucodes_end[];
		i->_reclaim5_start = _rstart5;
		i->_reclaim5_end = _rend5;
		i->_ucodes_start = ucodes_start;
		i->_ucodes_end = ucodes_end;
	}
#endif /* DONGLEBUILD */
}

#ifdef _RTE_SIM_
uint32 _memsize = RAMSZ;
#endif // endif

/* ROM accessor. If '_memsize' is used directly, the tools think the assembly symbol '_memsize' is
 * a function, which will result in an unexpected entry in the RAM jump table.
 */
uint32
BCMRAMFN(hnd_get_memsize)(void)
{
	return (_memsize);
}

/* ROM accessor.
 */
uint32
BCMRAMFN(hnd_get_rambase)(void)
{
#if defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7A__)
	/* CA7 also use the same variable though there is no tcm;
	 * revisit later
	 */
	return (_atcmrambase);
#endif // endif
	return 0;
}

uint32
BCMRAMFN(hnd_get_rambottom)(void)
{
#if defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7A__)
	/* CA7 also use the same variable though there is no tcm;
	 * revisit later
	 */
	return (_rambottom);
#else
	return (_memsize);
#endif // endif
}

/* ========================= reclaim ========================= */

#ifdef DONGLEBUILD

#if defined(BCMRECLAIM) && defined(CONFIG_XIP)
#error "Both XIP and RECLAIM defined"
#endif // endif

bool bcmreclaimed = FALSE;

#ifdef BCMRECLAIM
bool init_part_reclaimed = FALSE;
#endif // endif
bool attach_part_reclaimed = FALSE;
bool preattach_part_reclaimed = FALSE;
bool postattach_part_reclaimed = FALSE;
bool ucodes_reclaimed = FALSE;

void
hnd_reclaim(void)
{
	uint reclaim_size = 0;
	hnd_image_info_t info;
	const char *r_fmt_str = "reclaim section %s: Returned %d bytes to the heap\n";

	hnd_image_info(&info);

#ifdef BCMRECLAIM
	if (!bcmreclaimed && !init_part_reclaimed) {
		reclaim_size = (uint)(info._reclaim1_end - info._reclaim1_start);
		if (reclaim_size) {
			/* blow away the reclaim region */
			bzero(info._reclaim1_start, reclaim_size);
			hnd_arena_add((uint32)info._reclaim1_start, reclaim_size);
		}

		/* Nightly dongle test searches output for "Returned (.*) bytes to the heap" */
		printf(r_fmt_str, "2", reclaim_size);
		bcmreclaimed = TRUE;
		init_part_reclaimed = TRUE;
		goto exit;
	}
#endif /* BCMRECLAIM */

	if (!attach_part_reclaimed) {
		reclaim_size = (uint)(info._reclaim2_end - info._reclaim2_start);

		if (reclaim_size) {
			/* blow away the reclaim region */
			bzero(info._reclaim2_start, reclaim_size);
			hnd_arena_add((uint32)info._reclaim2_start, reclaim_size);
		}

		/* Nightly dongle test searches output for "Returned (.*) bytes to the heap" */
		printf(r_fmt_str, "1", reclaim_size);

		/* Reclaim space reserved for TCAM bootloader patching. Once the bootloader hands
		 * execution off to the firmware, the bootloader patch table is no longer required
		 * and can be reclaimed.
		 */
#if defined(BOOTLOADER_PATCH_RECLAIM)
		reclaim_size = (uint)(info._boot_patch_end - info._boot_patch_start);
		if (reclaim_size) {
			/* blow away the reclaim region */
			bzero(info._boot_patch_start, reclaim_size);
			hnd_arena_add((uint32)info._boot_patch_start, reclaim_size);
			printf(r_fmt_str, "boot-patch", reclaim_size);
		}
		else {
			/* For non-USB builds, the explicit bootloader patch section may not exist.
			 * However, there may still be a block of unused memory that can be
			 * reclaimed since parts of the memory map are fixed in order to provide
			 * compatibility with builds that require the bootloader patch section.
			 */
			reclaim_size = (uint)(info._reclaim4_end - info._reclaim4_start);
			if (reclaim_size) {
				/* blow away the reclaim region */
				bzero(info._reclaim4_start, reclaim_size);
				hnd_arena_add((uint32)info._reclaim4_start, reclaim_size);
				printf(r_fmt_str, "4", reclaim_size);
			}
		}
#endif /* BOOTLOADER_PATCH_RECLAIM */

		bcmreclaimed = FALSE;
		attach_part_reclaimed = TRUE;

#ifdef HND_PT_GIANT
		hnd_append_ptblk();
#endif // endif

#ifdef HNDLBUFCOMPACT
		/* Fix up 2M boundary before pktpool_fill */
		hnd_lbuf_fixup_2M_tcm();
#endif // endif

#ifdef BCMPKTPOOL
		{
#if defined(WLC_LOW) && !defined(WLC_HIGH)
		/* for bmac, populate all the configured buffers */
		bool minimal = FALSE;
#else
		bool minimal = TRUE;
#endif // endif
		hnd_pktpool_refill(minimal);
		}
#endif /* BCMPKTPOOL */

		goto exit;
	}

	if (!preattach_part_reclaimed) {
		reclaim_size = (uint)(info._reclaim3_end - info._reclaim3_start);

		if (reclaim_size) {
			/* blow away the reclaim region */
			bzero(info._reclaim3_start, reclaim_size);
			hnd_arena_add((uint32)info._reclaim3_start, reclaim_size);
		}

		/* Nightly dongle test searches output for "Returned (.*) bytes to the heap" */
		printf(r_fmt_str, "0", reclaim_size);

		attach_part_reclaimed = FALSE;
		preattach_part_reclaimed = TRUE;
	}

	if (!postattach_part_reclaimed) {
		reclaim_size = (uint)(info._reclaim5_end - info._reclaim5_start);

		if (reclaim_size) {
			/* blow away the reclaim region */
			bzero(info._reclaim5_start, reclaim_size);
			hnd_arena_add((uint32)info._reclaim5_start, reclaim_size);
		}

		/* Nightly dongle test searches output for "Returned (.*) bytes to the heap" */
		printf(r_fmt_str, "postattach", reclaim_size);

		postattach_part_reclaimed = TRUE;
	}

	if (!ucodes_reclaimed) {
		reclaim_size = (uint)(info._ucodes_end - info._ucodes_start);

		if (reclaim_size) {
			/* blow away the reclaim region */
			bzero(info._ucodes_start, reclaim_size);
			hnd_arena_add((uint32)info._ucodes_start, reclaim_size);
		}

		/* Nightly dongle test searches output for "Returned (.*) bytes to the heap" */
		printf(r_fmt_str, "ucodes", reclaim_size);

		ucodes_reclaimed = TRUE;
	}
exit:
#ifdef HNDLBUFCOMPACT
	hnd_lbuf_fixup_2M_tcm();
#endif // endif
	return;		/* return is needed to avoid compilation error
			   error: label at end of compound statement
			*/
}
#endif /* DONGLEBUILD */

/* ==================== stack ==================== */

#ifdef GLOBAL_STACK
#define	STACK_MAGIC	0x5354414b	/* Magic # for stack protection: 'STAK' */

static	uint32	*bos = NULL;		/* Bottom of the stack */
static	uint32	*tos = NULL;		/* Top of the stack */

#ifndef BCM_BOOTLOADER
#if defined(RTE_CONS) || defined(BCM_OL_DEV)
static void
hnd_print_stkuse(void *arg, int argc, char *argv[])
{
	if (*bos != STACK_MAGIC)
		printf("\tStack bottom has been overwritten\n");
	else {
		uint32 *p;
		uint32 cbos = 0;

		for (p = bos; p < &cbos; p ++)
			if (*p != STACK_MAGIC)
				break;
		printf("\tStack: %u Stack bottom: 0x%p, lwm: 0x%p, curr: 0x%p, top: 0x%p\n",
		       (uintptr)tos - (uintptr)bos, bos, p, &p, tos);
		printf("\tFree stack: 0x%x(%d) lwm: 0x%x(%d)\n",
		       ((uintptr)(&p) - (uintptr)bos),
		       ((uintptr)(&p) - (uintptr)bos),
		       ((uintptr)p - (uintptr)bos),
		       ((uintptr)p - (uintptr)bos));
		printf("\tIn use stack: 0x%x(%d) hwm: 0x%x(%d)\n",
		       ((uintptr)tos - (uintptr)(&p)),
		       ((uintptr)tos - (uintptr)(&p)),
		       ((uintptr)tos - (uintptr)p),
		       ((uintptr)tos - (uintptr)p));
	}
}
#endif /* RTE_CONS || BCM_OL_DEV */
#endif /* !BCM_BOOTLOADER */
#endif /* GLOBAL_STACK */

#ifndef BCM_BOOTLOADER
#if defined(RTE_CONS) || defined(BCM_OL_DEV)
static void
hnd_print_memuse(void *arg, int argc, char *argv[])
{
	process_ccmd("hu", 2);
	process_ccmd("su", 2);
	process_ccmd("pu", 2);
}
#endif /* RTE_CONS || BCM_OL_DEV */
#endif /* !BCM_BOOTLOADER */

#ifdef GLOBAL_STACK
void
hnd_stack_init(uint32 *stackbottom, uint32 *stacktop)
{
	uint32 *p;
	uint32 cbos = 0;

	bos = stackbottom;
	tos = stacktop;

	/*
	 * Mark the stack with STACK_MAGIC here before using any other
	 * automatic variables! The positions of these variables are
	 * compiler + target + optimization dependant, and they can be
	 * overwritten by the code below if they are allocated before
	 * 'p' towards the stack bottom.
	 */
	for (p = bos; p < &cbos - 32; p ++)
		*p = STACK_MAGIC;
}

void
hnd_stack_check(void)
{
	if (*bos != STACK_MAGIC)
		HND_MSG(("Stack bottom has been overwritten\n"));
	ASSERT(*bos == STACK_MAGIC);
}
#endif /* GLOBAL_STACK */

/* ==================== cli ==================== */

void
hnd_mem_cli_init(void)
{
#ifndef BCM_BOOTLOADER
#if defined(RTE_CONS) || defined(BCM_OL_DEV)
#ifdef GLOBAL_STACK
	hnd_cons_add_cmd("su", hnd_print_stkuse, 0);
#endif // endif
	hnd_cons_add_cmd("mu", hnd_print_memuse, 0);
#endif /* !RTE_CONS) && !BCM_OL_DEV */
#endif /* BCM_BOOTLOADER */
}
