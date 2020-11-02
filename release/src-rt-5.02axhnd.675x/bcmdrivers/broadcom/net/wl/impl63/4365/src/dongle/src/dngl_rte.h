/*
 * RTE dongle private definitions
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
 * $Id: dngl_rte.h 735737 2017-12-12 07:16:33Z $
 */

#ifndef	_dngl_rte_h_
#define	_dngl_rte_h_

#include <rte_dev.h>
#include <bcmcdc.h>

#ifndef MAXVSLAVEDEVS
#define MAXVSLAVEDEVS	0
#endif // endif

#ifndef BCMMSGBUF
#if MAXVSLAVEDEVS > BDC_FLAG2_IF_MASK
#error "MAXVSLAVEDEVS is too big"
#endif // endif
#endif  /* !BCMMSGBUF */

#if defined(WLRSDB)
/* Slave adjust value to accomodate new cfg during bsscfg MOVE */
#define WLRSDB_CLONE_ADJUST_SLAVES 1
#endif // endif

typedef struct dngl {
	void *bus;			/* generic bus handle */
	osl_t *osh;			/* OS abstraction layer handler */
	si_t *sih;			/* sb/si backplane handler */
	void *proto;			/* BDC/RNDIS proto handler */
	dngl_stats_t stats;
	int medium;
	hnd_dev_t *rtedev;
	hnd_dev_t *primary_slave;
	hnd_dev_t **slaves;		/* real  + virtual slave device handles for AP interfaces
					 * [Note: Total max is  BDC_FLAG2_IF_MASK ]
					 */
	int *iface2slave_map;
#ifdef BCMET
	et_cb_t cb;			/* Link event handler */
#endif // endif
	uint8 unit;			/* Device index */
	bool up;
	bool devopen;
#ifdef FLASH_UPGRADE
	int upgrade_status;		/* Upgrade return status code */
#endif // endif
	int tunable_max_slave_devs;
	int num_realslave_devs;
#ifdef BCM_FD_AGGR
	void *rpc_th;           /* handle for the bcm rpc tp module */
	dngl_timer_t *rpctimer;     /* Timer for toggling gpio output */
	uint16 rpctime;     /* max time  to push the aggregation */
	bool rpctimer_active;   /* TRUE = rpc timer is running */
	bool fdaggr;    /* 1 = aggregation enabled */
#endif // endif
	uint16	data_seq_no;
	uint16  ioctl_seq_no;
	uint16	data_seq_no_prev;
	uint16  ioctl_seq_no_prev;
	uint16	num_activeslave_devs;

} dngl_t;

#endif	/* _dngl_rte_h_ */
