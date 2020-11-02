/*
 * 802.11h Quiet module header file
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
 * $Id: wlc_quiet.h 708017 2017-06-29 14:11:45Z $
*/

#ifndef _wlc_quiet_h_
#define _wlc_quiet_h_

#ifdef WLQUIET
#define BSS_QUIET_STATE(qm, cfg) (qm != NULL ? wlc_quiet_get_quiet_state(qm, cfg) : 0)
#else
#define BSS_QUIET_STATE(qm, cfg) 0
#endif // endif

/* quiet state */
#define SILENCE		(1<<0)

/* APIs */
#ifdef WLQUIET

/* module */
extern wlc_quiet_info_t *wlc_quiet_attach(wlc_info_t *wlc);
extern void wlc_quiet_detach(wlc_quiet_info_t *qm);

/* actions */
extern void wlc_quiet_reset_all(wlc_quiet_info_t *qm, wlc_bsscfg_t *cfg);
extern void wlc_quiet_do_quiet(wlc_quiet_info_t *qm, wlc_bsscfg_t *cfg, dot11_quiet_t *qie);

extern void wlc_quiet_txfifo_suspend_complete(wlc_quiet_info_t *qm);

/* accessors */
extern uint wlc_quiet_get_quiet_state(wlc_quiet_info_t *qm, wlc_bsscfg_t *cfg);
extern uint wlc_quiet_get_quiet_count(wlc_quiet_info_t *qm, wlc_bsscfg_t *cfg);
extern void wlc_quiet_count_down(wlc_quiet_info_t *qm, wlc_bsscfg_t *cfg);

#else /* !WLQUIET */

#define wlc_quiet_attach(wlc) NULL
#define wlc_quiet_detach(qm) do {} while (0)

/* actions */
#define wlc_quiet_reset_all(qm, cfg) do {} while (0)
#define wlc_quiet_do_quiet(qm, cfg, qie) do {} while (0)

#define wlc_quiet_txfifo_suspend_complete(qm) do {} while (0)

/* accessors */
#define wlc_quiet_get_quiet_state(qm, cfg) 0
#define wlc_quiet_get_quiet_count(qm, cfg) 0

#endif /* !WLQUIET */

#endif /* _wlc_quiet_h_ */
