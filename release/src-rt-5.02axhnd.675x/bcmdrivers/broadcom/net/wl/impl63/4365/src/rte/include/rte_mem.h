/*
 * HND image/memory layout
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
 * $Id: rte_mem.h 627697 2016-03-28 06:15:02Z $
 */

#ifndef	_RTE_MEM_H
#define	_RTE_MEM_H

#include <typedefs.h>

/* Use standard symbols for Armulator build which does not use the hndrte.lds linker script */
#if defined(_RTE_SIM_) || defined(EXT_CBALL)
#define text_start	_start
#define text_end	etext
#define data_start	__data_start
#define data_end	edata
#define rodata_start	etext
#define rodata_end	__data_start
#define bss_start	__bss_start
#define bss_end		_end
#endif // endif

extern char text_start[], text_end[];
extern char rodata_start[], rodata_end[];
extern char data_start[], data_end[];
extern char bss_start[], bss_end[], _end[];

typedef struct {
	char *_text_start, *_text_end;
	char *_rodata_start, *_rodata_end;
	char *_data_start, *_data_end;
	char *_bss_start, *_bss_end;
	char *_reclaim1_start, *_reclaim1_end;
	char *_reclaim2_start, *_reclaim2_end;
	char *_reclaim3_start, *_reclaim3_end;
	char *_reclaim4_start, *_reclaim4_end;
	char *_boot_patch_start, *_boot_patch_end;
	char *_reclaim5_start, *_reclaim5_end;
	char *_ucodes_start, *_ucodes_end;
} hnd_image_info_t;

extern void hnd_image_info(hnd_image_info_t *info);

extern uint32 _memsize;
extern uint32 _rambottom;
extern uint32 _atcmrambase;

extern uint32 hnd_get_memsize(void);
extern uint32 hnd_get_rambase(void);
extern uint32 hnd_get_rambottom(void);

#ifdef DONGLEBUILD
extern void hnd_reclaim(void);
#else
#define hnd_reclaim() do {} while (0)
#endif /* DONGLEBUILD */

#endif	/* _RTE_MEM_H */
