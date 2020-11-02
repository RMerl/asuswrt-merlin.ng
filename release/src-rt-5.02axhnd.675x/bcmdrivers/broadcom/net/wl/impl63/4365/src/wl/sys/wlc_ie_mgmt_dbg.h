/*
 * IE management module debugging facilities
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
 * $Id: wlc_ie_mgmt_dbg.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef _wlc_ie_mgmt_dbg_h_
#define _wlc_ie_mgmt_dbg_h_

#ifdef BCMDBG
/* message bitvec */
#define IEM_DBG_ATTACH	(1U<<0)
#define IEM_DBG_TRACE	(1U<<1)
#define IEM_DBG_INFO	(1U<<2)
#define IEM_DBG_TIME	(1U<<3)
#define IEM_DBG_DUMP	(1U<<31)
/* debug/trace macros */
extern uint iem_msg_level;
#define IEM_ATTACH(x) do {if (iem_msg_level & IEM_DBG_ATTACH) printf x;} while (FALSE)
#define IEM_TRACE(x) do {if (iem_msg_level & IEM_DBG_TRACE) printf x;} while (FALSE)
#define IEM_INFO(x) do {if (iem_msg_level & IEM_DBG_INFO) printf x;} while (FALSE)
#define IEM_T32D(wlc, prev) ((iem_msg_level & IEM_DBG_TIME) && \
			     (wlc) != NULL && (wlc)->clk ?		\
			     R_REG((wlc)->osh, &(wlc)->regs->tsf_timerlow) - (prev) : \
			     0)
#define IEM_DUMP_ON() (iem_msg_level & IEM_DBG_DUMP) ? TRUE : FALSE
#else
#define IEM_ATTACH(x)
#define IEM_TRACE(x)
#define IEM_INFO(x)
#define IEM_T32D(wlc, prev) ((void)(prev), 0)
#define IEM_DUMP_ON() FALSE
#endif /* BCMDBG */

#endif /* _wlc_ie_mgmt_dbg_h_ */
