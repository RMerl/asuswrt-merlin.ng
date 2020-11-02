/*
 * Scan Data Base
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
 * $Id: wlc_scandb.h 708017 2017-06-29 14:11:45Z $
 */
#ifndef _WLC_SCANDB_H_
#define _WLC_SCANDB_H_

#include "typedefs.h"
#include "osl.h"
#include "proto/ethernet.h"
#include "bcmutils.h"
#include "wlioctl.h"

#ifndef WLC_SCANDB_DEFAULT_TIMEOUT
#define WLC_SCANDB_DEFAULT_TIMEOUT	64	/* default timeout in seconds */
#endif // endif

typedef struct wlc_scandb wlc_scandb_t;

typedef void (*scandb_iter_fn_t)(void *arg1, void *arg2, uint timestamp,
                                 struct ether_addr *BSSID, wlc_ssid_t *SSID,
                                 int BSS_type, chanspec_t chanspec,
                                 void *data, uint datalen);

#ifdef WLSCANCACHE

extern wlc_scandb_t* wlc_scandb_create(osl_t *osh, uint unit);
extern void wlc_scandb_free(wlc_scandb_t *sdb);

#else /* WLSCANCACHE */

#define wlc_scandb_free(sdb) do {(void)(sdb);} while (0)

#endif /* WLSCANCACHE */

extern void wlc_scandb_clear(wlc_scandb_t *sdb);
extern void wlc_scandb_ageout(wlc_scandb_t *sdb, uint timestamp);

/* accessors for Scan DB timeout in ms */
extern uint wlc_scandb_timeout_get(wlc_scandb_t *sdb);
extern void wlc_scandb_timeout_set(wlc_scandb_t *sdb, uint timeout);

extern void wlc_scandb_add(wlc_scandb_t *sdb,
                           const struct ether_addr *BSSID, const wlc_ssid_t *SSID,
                           uint8 BSS_type, chanspec_t chanspec, uint timestamp,
                           void *data, uint datalen);

extern int wlc_scandb_iterate(wlc_scandb_t *sdb,
                              const struct ether_addr *BSSID, int nssid, const wlc_ssid_t *SSID,
                              int BSS_type, const chanspec_t *chanspec_list, uint chanspec_num,
                              scandb_iter_fn_t fn, void *fn_arg1, void *fn_arg2);

#if (defined(BCMDBG) || defined(BCMDBG_DUMP)) && !defined(SCANOL)
extern int wlc_scandb_dump(void *handle, struct bcmstrbuf *b);
#endif // endif

#endif /* _WLC_SCANDB_H_ */
