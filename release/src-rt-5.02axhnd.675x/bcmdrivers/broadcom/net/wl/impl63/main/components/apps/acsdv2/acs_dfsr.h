/*
 *      acs_dfsr.h
 *
 *	Header file for the ACSD DFS Re-entry module.
 *
 *	Copyright 2020 Broadcom
 *
 *	This program is the proprietary software of Broadcom and/or
 *	its licensors, and may only be used, duplicated, modified or distributed
 *	pursuant to the terms and conditions of a separate, written license
 *	agreement executed between you and Broadcom (an "Authorized License").
 *	Except as set forth in an Authorized License, Broadcom grants no license
 *	(express or implied), right to use, or waiver of any kind with respect to
 *	the Software, and Broadcom expressly reserves all rights in and to the
 *	Software and all intellectual property rights therein.  IF YOU HAVE NO
 *	AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 *	WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 *	THE SOFTWARE.
 *
 *	Except as expressly set forth in the Authorized License,
 *
 *	1. This program, including its structure, sequence and organization,
 *	constitutes the valuable trade secrets of Broadcom, and you shall use
 *	all reasonable efforts to protect the confidentiality thereof, and to
 *	use this information only in connection with your use of Broadcom
 *	integrated circuit products.
 *
 *	2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *	"AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 *	REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 *	OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 *	DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 *	NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 *	ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 *	CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 *	OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *	3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 *	BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 *	SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 *	IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 *	IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 *	ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 *	OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 *	NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *	$Id$
 */
#ifndef __acs_dfsr_h__
#define __acs_dfsr_h__

typedef struct dfsr_context dfsr_context_t;

typedef enum {
	DFS_REENTRY_NONE = 0,
	DFS_REENTRY_DEFERRED,
	DFS_REENTRY_IMMEDIATE
} dfsr_reentry_type_t;

extern dfsr_context_t *acs_dfsr_init(char *prefix, bool enable, acs_bgdfs_info_t *acs_bgdfs);
extern void acs_dfsr_exit(dfsr_context_t *);
extern dfsr_reentry_type_t acs_dfsr_chanspec_update(dfsr_context_t *, chanspec_t,
	const char *caller, char *if_name);
extern int acs_dfsr_set_reentry_type(dfsr_context_t *ctx, int type);
extern dfsr_reentry_type_t acs_dfsr_set(dfsr_context_t *ctx, chanspec_t channel,
        const char *caller);

extern dfsr_reentry_type_t acs_dfsr_activity_update(dfsr_context_t *, char *if_name);
extern dfsr_reentry_type_t acs_dfsr_reentry_type(dfsr_context_t *);
extern void acs_dfsr_reentry_done(dfsr_context_t *);
extern bool acs_dfsr_enabled(dfsr_context_t *ctx);
extern bool acs_dfsr_enable(dfsr_context_t *ctx, bool enable);
extern int acs_dfsr_dump(dfsr_context_t *ctx, char *buf, unsigned buflen);
extern void acs_bgdfs_sw_add(dfsr_context_t *ctx, time_t now, uint32_t frame_count);
extern unsigned acs_bgdfs_sw_sum(dfsr_context_t *ctx);

#endif /* __acs_dfsr_h__ */
