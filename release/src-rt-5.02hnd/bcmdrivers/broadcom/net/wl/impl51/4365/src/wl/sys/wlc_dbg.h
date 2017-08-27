/*
 * driver debug and print functions
 *
 * Copyright (C) 2017, Broadcom. All Rights Reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * $Id: wlc_dbg.h 467328 2014-04-03 01:23:40Z $
 */

#ifndef _wlc_dbg_h_
#define _wlc_dbg_h_

#include <typedefs.h>

#if defined(EVENT_LOG_COMPILE) && defined(WLMSG_TRACECH)
#define _WL_CHANSW(fmt, ...)	EVENT_LOG(EVENT_LOG_TAG_TRACE_CHANSW, fmt, ##__VA_ARGS__)
#define WL_CHANSW(args)		_WL_CHANSW args
#else
#define WL_CHANSW(args)
#endif

enum {
	CHANSW_UNKNOWN = 0,
	CHANSW_SCAN = 1,
	CHANSW_PHYCAL = 2,
	CHANSW_INIT = 3,
	CHANSW_ASSOC = 4,
	CHANSW_ROAM = 5,
	CHANSW_MCHAN = 6,
	CHANSW_IOVAR = 7,
	CHANSW_CSA_DFS = 8,
	CHANSW_APCS = 9,
	CHANSW_AWDL = 10,
	CHANSW_FBT = 11,
	CHANSW_MISC = 12,
	CHANSW_LAST = 13
};

#if defined(BCMDBG) || defined(WLMSG_PRPKT) || defined(WLMSG_ASSOC)
extern void
wlc_print_bcn_prb(uint8 *frame, int len);
#endif

#if defined(BCMDBG) || defined(WLMSG_PRHDRS) || defined(WLMSG_PRPKT) || \
	defined(WLMSG_ASSOC)
void
wlc_print_dot11_mac_hdr(uint8* buf, int len);

#endif

#if defined(BCMDBG) || defined(WLMSG_PRHDRS)
void
wlc_print_txdesc_ac(wlc_info_t *wlc, void* hdrsBegin);
void
wlc_print_txdesc(wlc_info_t *wlc, wlc_txd_t *txd);
void
wlc_recv_print_rxh(wlc_info_t *wlc, wlc_d11rxhdr_t *wrxh);
void
wlc_print_hdrs(wlc_info_t *wlc, const char *prefix, uint8 *frame,
               wlc_txd_t *txd, wlc_d11rxhdr_t *wrxh, uint len);

#endif

#if defined(WLTINYDUMP) || defined(BCMDBG) || defined(WLMSG_ASSOC) || \
	defined(WLMSG_PRPKT) || defined(WLMSG_OID) || defined(BCMDBG_DUMP) || \
	defined(WLMSG_INFORM) || defined(WLMSG_WSEC) || defined(WLEXTLOG) || defined(WLTEST)
int
wlc_format_ssid(char* buf, const uchar ssid[], uint ssid_len);
#endif
#if defined(BCMDBG) || defined(WLMSG_PRPKT)
void
wlc_print_assoc(wlc_info_t *wlc, struct dot11_management_header *mng, int len);
#endif

#endif /* !_wlc_dbg_h_ */
