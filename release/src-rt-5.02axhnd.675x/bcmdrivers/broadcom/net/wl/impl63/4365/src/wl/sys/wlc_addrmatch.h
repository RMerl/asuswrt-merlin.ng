/*
 * Common (OS-independent) portion of
 * Broadcom 802.11bang Networking Device Driver
 *
 * BMAC driver - AMT/RCMTA interface
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
 * $Id: wlc_addrmatch.h 709556 2017-07-07 17:58:36Z $
 */

/* This interface provides support for manipulating address match
 * or rcmta (pre-corerev 40) entries, independent of the core, althouth
 * certain functionality - AMT attributes are not available for older
 * core revisions.
 */

#ifndef _wlc_addrmatch_h_
#define _wlc_addrmatch_h_

#include <wlc_types.h>
#include <d11.h>

enum {
	WLC_ADDRMATCH_IDX_MAC	= -1,
	WLC_ADDRMATCH_IDX_BSSID	= -2
};

/* General information corresponding to AMT entries (M_AMT_INFO_BLK) */
typedef enum {
	C_ADDR_IBSS_STA_NBIT    = 0,    // IBSS STA
	C_ADDR_WLIST_NBIT       = 1,    // WHITELIST chk enabled
	C_ADDR_BSSID_NBIT       = 2,    // BSSID only, not myAddr
	C_ADDR_CCACAP_NBIT      = 3,    // capture CCA statistics for this node
	C_ADDR_BFIDX_NBIT       = 4,    // index into M_BFI_BLK
	C_ADDR_BFIIDX_LB        = 7,    // last bit
	C_ADDR_MONITOR_NBIT     = 10,   // Monitor other STA
	C_ADDR_ACKMCAST_NBIT    = 11,   // ACK B/Mcast from this TA
	C_ADDR_BFCAP_NBIT       = 12,   // Beamforming
} eAddressBitmapDefinitions;
#define C_ADDR_BFIDX_BSZ        (C_ADDR_BFIIDX_LB - C_ADDR_BFIDX_NBIT + 1)

/* Add the address match entry. For AMT, the input attributes are OR'd
 * with the current ones, if any, if AMT_ATTR_VALID is set in input.
 * Otherwise, they are cleared. Based on idx, the entry is selected as follows
 *  WLC_ADDRMATCH_IDX_MAC - rxe MAC or AMT_IDX_MAC
 *  WLC_ADDRMATCH_IDX_BSSID - rxe BSSID or AMT_IDX_BSSID
 *  otherwise entry corresponds to the idx  - rcmta or AMT
 * Previous attributes are returned for AMT, or 0 for RCMTA
 */
uint16 wlc_set_addrmatch(wlc_info_t *wlc, int idx,
	const struct ether_addr *addr, uint16 attr);

/* Clears the address match entry - both address and attributes
 * if applicable, are cleared. Previous attributes are returned for AMT,
 * 0 for RCMTA
 */
uint16 wlc_clear_addrmatch(wlc_info_t *wlc, int idx);

#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(WL_BEAMFORMING) || \
	defined(ACKSUPR_MAC_FILTER)
/* get info for the address match entry. For AMT, both address and
 * the attributes are returned. For RCMTA, address is returned and
 * attributes are set to AMT_ATTR_VALID for non-NULL ether address
 */
void wlc_get_addrmatch(wlc_info_t *wlc, int idx, struct ether_addr *addr,
	uint16 *attr);
#endif // endif

#if defined(BCMDBG_DUMP)
/* dump the address match entry */
int wlc_dump_addrmatch(wlc_info_t *wlc, struct bcmstrbuf *b);
#endif // endif

#ifdef ACKSUPR_MAC_FILTER
int wlc_addrmatch_info_alloc(wlc_info_t *wlc, int max_entry_num);
void wlc_addrmatch_info_free(wlc_info_t *wlc, int max_entry_num);
#endif /* ACKSUPR_MAC_FILTER */
#endif /* _wlc_addrmatch_h_ */
