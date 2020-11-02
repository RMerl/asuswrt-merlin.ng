/*
 * OS independent ISR functions for ISRs or DPCs - Private to RTE.
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
 * $Id: rte_isr_priv.h 483472 2014-06-09 22:49:49Z $
 */

#ifndef	_hnd_isr_priv_
#define	_hnd_isr_priv_

#include <typedefs.h>
#include <osl_ext.h>
#include <rte_isr.h>

typedef struct hnd_isr_action hnd_isr_action_t;

struct hnd_isr_action {
	hnd_isr_action_t *next;
	uint coreid;
	uint32 event;
	hnd_isr_t isr;
	void *cbdata;
	uint32 sbtpsflag;
	osl_ext_task_t *thread;	/* thread context */
};

typedef struct hnd_isr_instance hnd_isr_instance_t;

struct hnd_isr_instance {
	hnd_isr_action_t *hnd_isr_action_list;
	uint32 hnd_action_flags;
};

/* get ISR instance */
extern hnd_isr_instance_t *hnd_isr;
#define hnd_isr_get_inst() hnd_isr

/* get DPC instance */
extern hnd_isr_instance_t *hnd_dpc;
#define hnd_dpc_get_inst() hnd_dpc

/* run isr based on sbflagst, optional callbacks invoked pre and post isr run */
void hnd_isr_proc_sbflagst(hnd_isr_instance_t *instance, uint32 sbflagst,
	void (*pre_cb)(hnd_isr_action_t *action),
	void (*post_cb)(hnd_isr_action_t *action));

/* run isr for specified event */
bool hnd_dpc_proc_event(hnd_isr_instance_t *instance, uint32 event);

/* Note: Each OS must implement this interface - query the event # an isr will post
 * in order to trigger its corresponding dpc.
 */
/* used by thread running dpc triggerred off an event */
uint hnd_isr_get_dpc_event(osl_ext_task_t *thread, uint32 coreid);

/* initialize registries */
void hnd_isr_module_init(osl_t *osh);

#endif /* _hnd_isr_priv_ */
