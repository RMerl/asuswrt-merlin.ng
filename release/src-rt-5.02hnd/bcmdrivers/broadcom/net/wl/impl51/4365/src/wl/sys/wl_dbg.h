/*
 * Minimal debug/trace/assert driver definitions for
 * Broadcom 802.11 Networking Adapter.
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
 * $Id: wl_dbg.h 664112 2016-10-10 13:27:40Z $
 */


#ifndef _wl_dbg_h_
#define _wl_dbg_h_

#include <event_log.h>

/* wl_msg_level is a bit vector with defs in wlioctl.h */
extern uint32 wl_msg_level;
extern uint32 wl_msg_level2;

/* Need ATTACH_ERROR() defintion only for dongle builds.
* So define it to WL_ERROR for other builds
*/
#define WL_ATTACH_ERROR(args)		WL_ERROR(args)

#ifdef MACOSX
/* "libkern/version.h" defines VERSION_MAJOR */
#include <libkern/version.h>
#endif
#if defined(BCMDBG) && defined(WLC_LOW) && !defined(BCMDBG_EXCLUDE_HW_TIMESTAMP)
extern char* wlc_dbg_get_hw_timestamp(void);

#define WL_TIMESTAMP() 		do { if (wl_msg_level2 & WL_TIMESTAMP_VAL) {\
	                            printf(wlc_dbg_get_hw_timestamp()); }\
	                        } while (0)
#else
#define WL_TIMESTAMP()
#endif 

#if defined(MACOSX) && (VERSION_MAJOR > 9)
extern int osl_printf(const char *fmt, ...);
#include <IOKit/apple80211/IO8Log.h>
#define WL_PRINT(args)		do { osl_printf args; } while (0)
#define RELEASE_PRINT(args)	do { WL_PRINT(args); IO8Log args; } while (0)
#else
#define WL_PRINT(args)		do { WL_TIMESTAMP(); printf args; } while (0)
#endif /* defined(MACOSX) && (VERSION_MAJOR > 9) */

#if defined(EVENT_LOG_COMPILE) && defined(WLMSG_SRSCAN)
#define _WL_SRSCAN(fmt, ...)	EVENT_LOG(EVENT_LOG_TAG_SRSCAN, fmt, ##__VA_ARGS__)
#define WL_SRSCAN(args)		_WL_SRSCAN args
#else
#define WL_SRSCAN(args)
#endif

#if defined(BCMDBG)

/* DBGONLY() macro to reduce ifdefs in code for statements that are only needed when
 * BCMDBG is defined.
 * Ex.
 * myfn() {
 *     int i;
 *     DBGONLY(int dbg; )
 */
#define DBGONLY(x) x

/* To disable a message completely ... until you need it again */
#define	WL_NONE(args)		do {if (wl_msg_level & 0) WL_PRINT(args);} while (0)

#define	WL_ERROR(args)		do {if (wl_msg_level & WL_ERROR_VAL) WL_PRINT(args);} while (0)
#define	WL_TRACE(args)		do {if (wl_msg_level & WL_TRACE_VAL) WL_PRINT(args);} while (0)

#ifndef LINUX_POSTMOGRIFY_REMOVAL
#define	WL_PRHDRS_MSG(args)	do {if (wl_msg_level & WL_PRHDRS_VAL) WL_PRINT(args);} while (0)
#define WL_PRHDRS(i, p, f, t, r, l) 	do { \
						if (wl_msg_level & WL_PRHDRS_VAL) \
							wlc_print_hdrs(i, p, f, t, r, l); \
					} while (0)
#define	WL_PRPKT(m, b, n)	do {if (wl_msg_level & WL_PRPKT_VAL) prhex(m, b, n);} while (0)
#define	WL_INFORM(args)		do {if (wl_msg_level & WL_INFORM_VAL) WL_PRINT(args);} while (0)
#define	WL_TMP(args)		do {if (wl_msg_level & WL_TMP_VAL) WL_PRINT(args);} while (0)
#define	WL_OID(args)		do {if (wl_msg_level & WL_OID_VAL) WL_PRINT(args);} while (0)
#define	WL_RATE(args)		do {if (wl_msg_level & WL_RATE_VAL) WL_PRINT(args);} while (0)
#define WL_ASSOC(args)		do {if (wl_msg_level & WL_ASSOC_VAL) WL_PRINT(args);} while (0)
#define	WL_PRUSR(m, b, n)	do {if (wl_msg_level & WL_PRUSR_VAL) prhex(m, b, n);} while (0)
#define WL_PS(args)		do {if (wl_msg_level & WL_PS_VAL) WL_PRINT(args);} while (0)
#define WL_SPARE1(args)		do {if (wl_msg_level & WL_TXPWR_VAL) WL_PRINT(args);} while (0)
#define WL_PORT(args)		do {if (wl_msg_level & WL_PORT_VAL) WL_PRINT(args);} while (0)
#define WL_DUAL(args)		do {if (wl_msg_level & WL_DUAL_VAL) WL_PRINT(args);} while (0)
#define WL_WSEC(args)		do {if (wl_msg_level & WL_WSEC_VAL) WL_PRINT(args);} while (0)
#define WL_WSEC_DUMP(args)	do {if (wl_msg_level & WL_WSEC_DUMP_VAL) WL_PRINT(args);} while (0)
#define WL_SPARE2(args)		do {if (wl_msg_level & WL_NRSSI_VAL) WL_PRINT(args);} while (0)
#define WL_SPARE3(args)		do {if (wl_msg_level & WL_LOFT_VAL) WL_PRINT(args);} while (0)
#define WL_REGULATORY(args)	do {if (wl_msg_level & WL_REGULATORY_VAL) WL_PRINT(args);} while (0)
#define WL_SPARE4(args)		do {if (wl_msg_level & WL_PHYCAL_VAL) WL_PRINT(args);} while (0)
#define WL_TAF(args)		do {if (wl_msg_level & WL_TAF_VAL) WL_PRINT(args);} while (0)
#define WL_SPARE5(args)		do {if (wl_msg_level & WL_RADAR_VAL) WL_PRINT(args);} while (0)
#define WL_MPC(args)		do {if (wl_msg_level & WL_MPC_VAL) WL_PRINT(args);} while (0)
#define WL_APSTA(args)		do {if (wl_msg_level & WL_APSTA_VAL) WL_PRINT(args);} while (0)
#define WL_DFS(args)		do {if (wl_msg_level & WL_DFS_VAL) WL_PRINT(args);} while (0)
#define WL_MUMIMO(args)		do {if (wl_msg_level & WL_MUMIMO_VAL) WL_PRINT(args);} while (0)
#define WL_MODE_SWITCH(args) do {if (wl_msg_level & WL_MODE_SWITCH_VAL) WL_PRINT(args);} while (0)
#define WL_BCNTRIM_DBG(args) do {if (wl_msg_level & WL_BCNTRIM_VAL) WL_PRINT(args);} while (0)
#define WL_MBSS(args)		do {if (wl_msg_level & WL_MBSS_VAL) WL_PRINT(args);} while (0)
#define WL_CAC(args)		do {if (wl_msg_level & WL_CAC_VAL) WL_PRINT(args);} while (0)
#define WL_AMSDU(args)		do {if (wl_msg_level & WL_AMSDU_VAL) WL_PRINT(args);} while (0)
#define WL_AMPDU(args)		do {if (wl_msg_level & WL_AMPDU_VAL) WL_PRINT(args);} while (0)
#define WL_FFPLD(args)		do {if (wl_msg_level & WL_FFPLD_VAL) WL_PRINT(args);} while (0)
/* wl_msg_level is full. Use wl_msg_level_2 now */
#define WL_WOWL(args)		do {if (wl_msg_level2 & WL_WOWL_VAL) WL_PRINT(args);} while (0)
#define WL_SCAN(args)		do {if (wl_msg_level2 & WL_SCAN_VAL) WL_PRINT(args);} while (0)
#define	WL_COEX(args)		do {if (wl_msg_level2 & WL_COEX_VAL) WL_PRINT(args);} while (0)
#define WL_RTDC(w,s,i,j)	do {if (wl_msg_level2 & WL_RTDC_VAL) wlc_log(w,s,i,j);} while (0)
#define	WL_PROTO(args)		do {if (wl_msg_level2 & WL_PROTO_VAL) WL_PRINT(args);} while (0)
#define	WL_SPARE6(args)		do {if (wl_msg_level2 & WL_ACI_VAL) WL_PRINT(args);} while (0)
#define WL_RTDC2(w,s,i,j)	do {if (wl_msg_level2 & 0) wlc_log(w,s,i,j);} while (0)
#define WL_BTA(args)		do {if (wl_msg_level2 & WL_BTA_VAL) WL_PRINT(args);} while (0)
#define WL_CHANINT(args)	do {if (wl_msg_level2 & WL_CHANINT_VAL) WL_PRINT(args);} while (0)
#define WL_WMF(args)		do {if (wl_msg_level2 & WL_WMF_VAL) WL_PRINT(args);} while (0)
#define WL_P2P(args)		do {if (wl_msg_level2 & WL_P2P_VAL) WL_PRINT(args);} while (0)
#define WL_ITFR(args)		do {if (wl_msg_level2 & WL_ITFR_VAL) WL_PRINT(args);} while (0)
#define WL_MCHAN(args)		do {if (wl_msg_level2 & WL_MCHAN_VAL) WL_PRINT(args);} while (0)
#define WL_TDLS(args)		do {if (wl_msg_level2 & WL_TDLS_VAL) WL_PRINT(args);} while (0)
#define WL_MCNX(args)		do {if (wl_msg_level2 & WL_MCNX_VAL) WL_PRINT(args);} while (0)
#define WL_PROT(args)		do {if (wl_msg_level2 & WL_PROT_VAL) WL_PRINT(args);} while (0)
#define WL_PSTA(args)		do {if (wl_msg_level2 & WL_PSTA_VAL) WL_PRINT(args);} while (0)
#define WL_TBTT(args)		do {if (wl_msg_level2 & WL_TBTT_VAL) WL_PRINT(args);} while (0)
#define WL_TRF_MGMT(args)	do {if (wl_msg_level2 & WL_TRF_MGMT_VAL) WL_PRINT(args);} while (0)
#define WL_L2FILTER(args) do {if (wl_msg_level2 & WL_L2FILTER_VAL) WL_PRINT(args);} while (0)
#define WL_TSO(args)		do {if (wl_msg_level2 & WL_TSO_VAL) WL_PRINT(args);} while (0)
#define WL_MQ(args)		do {if (wl_msg_level2 & WL_MQ_VAL) WL_PRINT(args);} while (0)
#define WL_P2PO(args)		do {if (wl_msg_level2 & WL_P2PO_VAL) WL_PRINT(args);} while (0)
#define WL_AWDL(args)		do {if (wl_msg_level2 & WL_AWDL_VAL) WL_PRINT(args);} while (0)
#define WL_WNM(args)		do {if (wl_msg_level2 & WL_WNM_VAL) WL_PRINT(args);} while (0)
#define WL_TXBF(args)           do {if (wl_msg_level2 & WL_TXBF_VAL) WL_PRINT(args);} while (0)
#define	WL_PCIE(args)		do {if (wl_msg_level2 & WL_PCIE_VAL) WL_PRINT(args);} while (0)
#define WL_CHANLOG(w, s, i, j)	\
    do {if (wl_msg_level2 & WL_TIMESTAMP_VAL) wlc_log(w, s, i, j);} while (0)
#define WL_NET_DETECT(args)	\
    do {if (wl_msg_level2 & WL_NET_DETECT_VAL) WL_PRINT(args);} while (0)
/* not using WL_ROAM for BCMDBG at the moment */
#define WL_ROAM(args)
#define WL_FBT(args)	do {if (wl_msg_level2 & WL_FBT_VAL) WL_PRINT(args);} while (0)
//#define WL_FBT(args)
#define WL_PRMAC(args)	do {if (wl_msg_level & WL_PRMAC_VAL) WL_PRINT(args);} while (0)

#define WL_ERROR_ON()		(wl_msg_level & WL_ERROR_VAL)
#define WL_TRACE_ON()		(wl_msg_level & WL_TRACE_VAL)
#define WL_PRHDRS_ON()		(wl_msg_level & WL_PRHDRS_VAL)
#define WL_PRPKT_ON()		(wl_msg_level & WL_PRPKT_VAL)
#define WL_INFORM_ON()		(wl_msg_level & WL_INFORM_VAL)
#define WL_TMP_ON()		(wl_msg_level & WL_TMP_VAL)
#define WL_OID_ON()		(wl_msg_level & WL_OID_VAL)
#define WL_RATE_ON()		(wl_msg_level & WL_RATE_VAL)
#define WL_ASSOC_ON()		(wl_msg_level & WL_ASSOC_VAL)
#define WL_PORT_ON()		(wl_msg_level & WL_PORT_VAL)
#define WL_WSEC_ON()		(wl_msg_level & WL_WSEC_VAL)
#define WL_WSEC_DUMP_ON()	(wl_msg_level & WL_WSEC_DUMP_VAL)
#define WL_MPC_ON()		(wl_msg_level & WL_MPC_VAL)
#define WL_REGULATORY_ON()	(wl_msg_level & WL_REGULATORY_VAL)
#define WL_APSTA_ON()		(wl_msg_level & WL_APSTA_VAL)
#define WL_DFS_ON()		(wl_msg_level & WL_DFS_VAL)
#define WL_MUMIMO_ON()		(wl_msg_level & WL_MUMIMO_VAL)
#define WL_TAF_ON()		(wl_msg_level & WL_TAF_VAL)
#ifdef WL11AC
#define WL_MODE_SWITCH_ON()	(wl_msg_level & WL_MODE_SWITCH_VAL)
#endif
#define WL_MBSS_ON()		(wl_msg_level & WL_MBSS_VAL)
#define WL_AMPDU_ON()		(wl_msg_level & WL_AMPDU_VAL)
#define WL_WOWL_ON()		(wl_msg_level2 & WL_WOWL_VAL)
#define WL_SCAN_ON()		(wl_msg_level2 & WL_SCAN_VAL)
#define WL_BTA_ON()		(wl_msg_level2 & WL_BTA_VAL)
#define WL_WMF_ON()	        (wl_msg_level2 & WL_WMF_VAL)
#define WL_P2P_ON()		(wl_msg_level2 & WL_P2P_VAL)
#define WL_ITFR_ON()		(wl_msg_level2 & WL_ITFR_VAL)
#define WL_MCHAN_ON()		(wl_msg_level2 & WL_MCHAN_VAL)
#define WL_TDLS_ON()		(wl_msg_level2 & WL_TDLS_VAL)
#define WL_MCNX_ON()		(wl_msg_level2 & WL_MCNX_VAL)
#define WL_PROT_ON()		(wl_msg_level2 & WL_PROT_VAL)
#define WL_PSTA_ON()		(wl_msg_level2 & WL_PSTA_VAL)
#define WL_TBTT_ON()		(wl_msg_level2 & WL_TBTT_VAL)
#define WL_TRF_MGMT_ON()	(wl_msg_level2 & WL_TRF_MGMT)
#define WL_PWRSEL_ON()		(wl_msg_level2 & WL_PWRSEL_VAL)
#define WL_L2FILTER_ON()	(wl_msg_level2 & WL_L2FILTER_VAL)
#define WL_MQ_ON()		(wl_msg_level2 & WL_MQ_VAL)
#define WL_P2PO_ON()		(wl_msg_level2 & WL_P2PO_VAL)
#define WL_AWDL_ON()		(wl_msg_level2 & WL_AWDL_VAL)
#define WL_WNM_ON()		(wl_msg_level2 & WL_WNM_VAL)
#define WL_TXBF_ON()	        (wl_msg_level2 & WL_TXBF_VAL)
#define WL_PCIE_ON()		(wl_msg_level2 & WL_PCIE_VAL)
#define WL_CHANLOG_ON()		(wl_msg_level2 & WL_TIMESTAMP_VAL)
#define WL_NET_DETECT_ON()	(wl_msg_level2 & WL_NET_DETECT)
#define WL_FBT_ON()            (wl_msg_level2 & WL_FBT_VAL)

/* Extra message control for APSTA debugging */
#define	WL_APSTA_UPDN_VAL	0x00000001 /* Config up/down related  */
#define	WL_APSTA_BCN_VAL	0x00000002 /* Calls to beacon update  */
#define	WL_APSTA_TX_VAL		0x00000004 /* Transmit data path */
#define	WL_APSTA_RX_VAL		0x00000008 /* Receive data path  */
#define	WL_APSTA_TSF_VAL	0x00000010 /* TSF-related items  */
#define	WL_APSTA_BSSID_VAL	0x00000020 /* Calls to set bssid */

extern uint32 wl_apsta_dbg;

#define WL_APSTA_UPDN(args) do {if (wl_apsta_dbg & WL_APSTA_UPDN_VAL) {WL_APSTA(args);}} while (0)
#define WL_APSTA_BCN(args) do {if (wl_apsta_dbg & WL_APSTA_BCN_VAL) {WL_APSTA(args);}} while (0)
#define WL_APSTA_TX(args) do {if (wl_apsta_dbg & WL_APSTA_TX_VAL) {WL_APSTA(args);}} while (0)
#define WL_APSTA_RX(args) do {if (wl_apsta_dbg & WL_APSTA_RX_VAL) {WL_APSTA(args);}} while (0)
#define WL_APSTA_TSF(args) do {if (wl_apsta_dbg & WL_APSTA_TSF_VAL) {WL_APSTA(args);}} while (0)
#define WL_APSTA_BSSID(args) do {if (wl_apsta_dbg & WL_APSTA_BSSID_VAL) {WL_APSTA(args);}} while (0)

/* Extra message control for AMPDU debugging */
#define   WL_AMPDU_UPDN_VAL	0x00000001 /* Config up/down related  */
#define   WL_AMPDU_ERR_VAL	0x00000002 /* Calls to beaocn update  */
#define   WL_AMPDU_TX_VAL	0x00000004 /* Transmit data path */
#define   WL_AMPDU_RX_VAL	0x00000008 /* Receive data path  */
#define   WL_AMPDU_CTL_VAL	0x00000010 /* TSF-related items  */
#define   WL_AMPDU_HW_VAL       0x00000020 /* AMPDU_HW */
#define   WL_AMPDU_HWTXS_VAL    0x00000040 /* AMPDU_HWTXS */
#define   WL_AMPDU_HWDBG_VAL    0x00000080 /* AMPDU_DBG */
#define   WL_AMPDU_STAT_VAL     0x00000100 /* statistics */

extern uint32 wl_ampdu_dbg;

#define WL_AMPDU_UPDN(args) do {if (wl_ampdu_dbg & WL_AMPDU_UPDN_VAL) {WL_AMPDU(args);}} while (0)
#define WL_AMPDU_RX(args) do {if (wl_ampdu_dbg & WL_AMPDU_RX_VAL) {WL_AMPDU(args);}} while (0)
#define WL_AMPDU_ERR(args) do {if (wl_ampdu_dbg & WL_AMPDU_ERR_VAL) {WL_AMPDU(args);}} while (0)
#define WL_AMPDU_TX(args) do {if (wl_ampdu_dbg & WL_AMPDU_TX_VAL) {WL_AMPDU(args);}} while (0)
#define WL_AMPDU_CTL(args) do {if (wl_ampdu_dbg & WL_AMPDU_CTL_VAL) {WL_AMPDU(args);}} while (0)
#define WL_AMPDU_HW(args) do {if (wl_ampdu_dbg & WL_AMPDU_HW_VAL) {WL_AMPDU(args);}} while (0)
#define WL_AMPDU_HWTXS(args) do {if (wl_ampdu_dbg & WL_AMPDU_HWTXS_VAL) {WL_AMPDU(args);}} while (0)
#define WL_AMPDU_HWDBG(args) do {if (wl_ampdu_dbg & WL_AMPDU_HWDBG_VAL) {WL_AMPDU(args);}} while (0)
#define WL_AMPDU_STAT(args) do {if (wl_ampdu_dbg & WL_AMPDU_STAT_VAL) {WL_AMPDU(args);}} while (0)
#define WL_AMPDU_ERR_ON() (wl_ampdu_dbg & WL_AMPDU_ERR_VAL)
#define WL_AMPDU_HW_ON() (wl_ampdu_dbg & WL_AMPDU_HW_VAL)
#define WL_AMPDU_HWTXS_ON() (wl_ampdu_dbg & WL_AMPDU_HWTXS_VAL)

#endif /* LINUX_POSTMOGRIFY_REMOVAL */

/* BCMDBG */
#elif defined(BCMCONDITIONAL_LOGGING)

/* Ideally this should be some include file that vendors can include to conditionalize logging */

/* DBGONLY() macro to reduce ifdefs in code for statements that are only needed when
 * BCMDBG is defined.
 */
#define DBGONLY(x)

/* To disable a message completely ... until you need it again */
#define WL_NONE(args)
#define WL_ERROR(args)		do {if (wl_msg_level & WL_ERROR_VAL) WL_PRINT(args);} while (0)
#define WL_TRACE(args)
#define WL_PRHDRS_MSG(args)
#define WL_PRHDRS(i, p, f, t, r, l)
#define WL_PRPKT(m, b, n)
#define WL_INFORM(args)
#define WL_TMP(args)
#define WL_OID(args)
#define WL_RATE(args)		do {if (wl_msg_level & WL_RATE_VAL) WL_PRINT(args);} while (0)
#define WL_ASSOC(args)		do {if (wl_msg_level & WL_ASSOC_VAL) WL_PRINT(args);} while (0)
#define WL_PRUSR(m, b, n)
#define WL_PS(args)		do {if (wl_msg_level & WL_PS_VAL) WL_PRINT(args);} while (0)

#define WL_PORT(args)
#define WL_DUAL(args)
#define WL_REGULATORY(args)	do {if (wl_msg_level & WL_REGULATORY_VAL) WL_PRINT(args);} while (0)

#define WL_MPC(args)
#define WL_APSTA(args)
#define WL_APSTA_BCN(args)
#define WL_APSTA_TX(args)
#define WL_APSTA_TSF(args)
#define WL_APSTA_BSSID(args)
#define WL_BA(args)
#define WL_MBSS(args)
#define WL_PROTO(args)

#define	WL_CAC(args)		do {if (wl_msg_level & WL_CAC_VAL) WL_PRINT(args);} while (0)
#define WL_AMSDU(args)
#define WL_AMPDU(args)
#define WL_FFPLD(args)
#define WL_MCHAN(args)

#define WL_DFS(args)
#define WL_WOWL(args)
#define WL_DPT(args)
#define WL_ASSOC_OR_DPT(args)
#define WL_SCAN(args)		do {if (wl_msg_level2 & WL_SCAN_VAL) WL_PRINT(args);} while (0)
#define WL_COEX(args)
#define WL_RTDC(w, s, i, j)
#define WL_RTDC2(w, s, i, j)
#define WL_CHANINT(args)
#define WL_BTA(args)
#define WL_P2P(args)
#define WL_ITFR(args)
#define WL_TDLS(args)
#define WL_MCNX(args)
#define WL_PROT(args)
#define WL_PSTA(args)
#define WL_WFDS(m, b, n)
#define WL_TRF_MGMT(args)
#define WL_L2FILTER(args)
#define WL_MQ(args)
#define WL_TXBF(args)
#define WL_MUMIMO(args)
#define WL_P2PO(args)
#define WL_NET_DETECT(args)
#define WL_ROAM(args)
#define WL_WNM(args)


#define WL_AMPDU_UPDN(args)
#define WL_AMPDU_RX(args)
#define WL_AMPDU_ERR(args)
#define WL_AMPDU_TX(args)
#define WL_AMPDU_CTL(args)
#define WL_AMPDU_HW(args)
#define WL_AMPDU_HWTXS(args)
#define WL_AMPDU_HWDBG(args)
#define WL_AMPDU_STAT(args)
#define WL_AMPDU_ERR_ON()       0
#define WL_AMPDU_HW_ON()        0
#define WL_AMPDU_HWTXS_ON()     0

#define WL_APSTA_UPDN(args)
#define WL_APSTA_RX(args)
#define WL_WSEC(args)
#define WL_WSEC_DUMP(args)
#define WL_PCIE(args)
#define WL_CHANLOG(w, s, i, j)

#define WL_ERROR_ON()		(wl_msg_level & WL_ERROR_VAL)
#define WL_TRACE_ON()		0
#define WL_PRHDRS_ON()		0
#define WL_PRPKT_ON()		0
#define WL_INFORM_ON()		0
#define WL_TMP_ON()		0
#define WL_OID_ON()		0
#define WL_RATE_ON()		(wl_msg_level & WL_RATE_VAL)
#define WL_ASSOC_ON()		(wl_msg_level & WL_ASSOC_VAL)
#define WL_PRUSR_ON()		0
#define WL_PS_ON()		(wl_msg_level & WL_PS_VAL)
#define WL_PORT_ON()		0
#define WL_WSEC_ON()		0
#define WL_WSEC_DUMP_ON()	0
#define WL_MPC_ON()		0
#define WL_REGULATORY_ON()	(wl_msg_level & WL_REGULATORY_VAL)
#define WL_APSTA_ON()		0
#define WL_DFS_ON()		0
#define WL_MBSS_ON()		0
#define WL_CAC_ON()		(wl_msg_level & WL_CAC_VAL)
#define WL_AMPDU_ON()		0
#define WL_DPT_ON()		0
#define WL_WOWL_ON()		0
#define WL_SCAN_ON()		(wl_msg_level2 & WL_SCAN_VAL)
#define WL_BTA_ON()		0
#define WL_P2P_ON()		0
#define WL_ITFR_ON()		0
#define WL_MCHAN_ON()		0
#define WL_TDLS_ON()		0
#define WL_MCNX_ON()		0
#define WL_PROT_ON()		0
#define WL_PSTA_ON()		0
#define WL_TRF_MGMT_ON()	0
#define WL_LPC_ON()		0
#define WL_L2FILTER_ON()	0
#define WL_TXBF_ON()		0
#define WL_P2PO_ON()		0
#define WL_CHANLOG_ON()		0
#define WL_NET_DETECT_ON()	0
#define WL_WNM_ON()		0
#define WL_PCIE_ON()		0
#define WL_MUMIMO_ON()		0

#else /* !BCMDBG */

/* DBGONLY() macro to reduce ifdefs in code for statements that are only needed when
 * BCMDBG is defined.
 */
#define DBGONLY(x)

/* To disable a message completely ... until you need it again */
#define WL_NONE(args)

#ifdef BCMDBG_ERR
#define	WL_ERROR(args)		WL_PRINT(args)
#else
#define	WL_ERROR(args)
#endif /* BCMDBG_ERR */
#define	WL_TRACE(args)
#ifndef LINUX_POSTMOGRIFY_REMOVAL
#ifdef WLMSG_PRHDRS
#define	WL_PRHDRS_MSG(args)		WL_PRINT(args)
#define WL_PRHDRS(i, p, f, t, r, l)	wlc_print_hdrs(i, p, f, t, r, l)
#else
#define	WL_PRHDRS_MSG(args)
#define	WL_PRHDRS(i, p, f, t, r, l)
#endif
#ifdef WLMSG_PRPKT
#define	WL_PRPKT(m, b, n)	prhex(m, b, n)
#else
#define	WL_PRPKT(m, b, n)
#endif
#ifdef WLMSG_INFORM
#define	WL_INFORM(args)		WL_PRINT(args)
#else
#define	WL_INFORM(args)
#endif
#define	WL_TMP(args)
#ifdef WLMSG_OID
#define WL_OID(args)		WL_PRINT(args)
#else
#define WL_OID(args)
#endif
#define	WL_RATE(args)
#ifdef WLMSG_ASSOC
#if 0
#define	WL_ASSOC(args)		WL_PRINT(args)
#else
#define WL_ASSOC(args)		do {if (wl_msg_level & WL_ASSOC_VAL) WL_PRINT(args);} while (0)
#endif
#else
#define	WL_ASSOC(args)
#endif
#define	WL_PRUSR(m, b, n)
#ifdef WLMSG_PS
#define WL_PS(args)		WL_PRINT(args)
#else
#define WL_PS(args)
#endif
#ifdef WLMSG_ROAM
#define WL_ROAM(args)	WL_PRINT(args)
#else
#define WL_ROAM(args)
#endif
#define WL_PORT(args)
#define WL_DUAL(args)
#define WL_REGULATORY(args)
#define WL_TAF(args)

#ifdef WLMSG_MPC
#define WL_MPC(args)		WL_PRINT(args)
#else
#define WL_MPC(args)
#endif
#define WL_APSTA(args)
#define WL_APSTA_BCN(args)
#define WL_APSTA_TX(args)
#define WL_APSTA_TSF(args)
#define WL_APSTA_BSSID(args)
#define WL_BA(args)
#define WL_MBSS(args)
#define WL_MODE_SWITCH(args)
#define	WL_PROTO(args)

#define	WL_CAC(args)
#define WL_AMSDU(args)
#define WL_AMPDU(args)
#define WL_FFPLD(args)
#define WL_MCHAN(args)
#define WL_BCNTRIM_DBG(args)

/* Define WLMSG_DFS automatically for WLTEST builds */
#if defined(WLTEST) && !defined(BCMROMBUILD)
#ifndef WLMSG_DFS
#define WLMSG_DFS
#endif
#endif /* WLTEST */

#ifdef WLMSG_DFS
#define WL_DFS(args)		do {if (wl_msg_level & WL_DFS_VAL) WL_PRINT(args);} while (0)
#else /* WLMSG_DFS */
#define WL_DFS(args)
#endif /* WLMSG_DFS */
#define WL_WOWL(args)
#ifdef WLMSG_SCAN
#define WL_SCAN(args)		WL_PRINT(args)
#else
#define WL_SCAN(args)
#endif
#define	WL_COEX(args)
#define WL_RTDC(w, s, i, j)
#define WL_RTDC2(w, s, i, j)
#define WL_CHANINT(args)
#ifdef WLMSG_BTA
#define WL_BTA(args)		WL_PRINT(args)
#else
#define WL_BTA(args)
#endif
#define WL_WMF(args)
#define WL_P2P(args)
#define WL_ITFR(args)
#define WL_TDLS(args)
#define WL_MCNX(args)
#define WL_PROT(args)
#define WL_PSTA(args)
#define WL_TBTT(args)
#define WL_TRF_MGMT(args)
#define WL_L2FILTER(args)
#define WL_MQ(args)
#define WL_P2PO(args)
#define WL_AWDL(args)
#define WL_WNM(args)
#define WL_TXBF(args)
#define WL_MUMIMO(args)
#define WL_CHANLOG(w, s, i, j)
#define WL_NET_DETECT(args)
#define WL_FBT(args)

#ifdef BCMDBG_ERR
#define WL_ERROR_ON()		1
#else
#define WL_ERROR_ON()		0
#endif
#define WL_TRACE_ON()		0
#ifdef WLMSG_PRHDRS
#define WL_PRHDRS_ON()		1
#else
#define WL_PRHDRS_ON()		0
#endif
#ifdef WLMSG_PRPKT
#define WL_PRPKT_ON()		1
#else
#define WL_PRPKT_ON()		0
#endif
#ifdef WLMSG_INFORM
#define WL_INFORM_ON()		1
#else
#define WL_INFORM_ON()		0
#endif
#ifdef WLMSG_OID
#define WL_OID_ON()		1
#else
#define WL_OID_ON()		0
#endif
#define WL_TMP_ON()		0
#define WL_RATE_ON()		0
#ifdef WLMSG_ASSOC
#define WL_ASSOC_ON()		1
#else
#define WL_ASSOC_ON()		0
#endif
#define WL_PORT_ON()		0
#ifdef WLMSG_WSEC
#define WL_WSEC_ON()		1
#define WL_WSEC_DUMP_ON()	1
#else
#define WL_WSEC_ON()		0
#define WL_WSEC_DUMP_ON()	0
#endif
#ifdef WLMSG_MPC
#define WL_MPC_ON()		1
#else
#define WL_MPC_ON()		0
#endif
#define WL_REGULATORY_ON()	0
#define WL_TAF_ON()		0

#define WL_APSTA_ON()		0
#define WL_BA_ON()		0
#define WL_MBSS_ON()		0
#define WL_MODE_SWITCH_ON()		0
#ifdef WLMSG_DFS
#define WL_DFS_ON()		1
#else /* WLMSG_DFS */
#define WL_DFS_ON()		0
#endif /* WLMSG_DFS */
#ifdef WLMSG_SCAN
#define WL_SCAN_ON()            1
#else
#define WL_SCAN_ON()            0
#endif
#ifdef WLMSG_BTA
#define WL_BTA_ON()		1
#else
#define WL_BTA_ON()		0
#endif
#define WL_WMF_ON()		0
#define WL_P2P_ON()		0
#define WL_MCHAN_ON()		0
#define WL_TDLS_ON()		0
#define WL_MCNX_ON()		0
#define WL_PROT_ON()		0
#define WL_TBTT_ON()		0
#define WL_PWRSEL_ON()		0
#define WL_L2FILTER_ON()	0
#define WL_MQ_ON()		0
#define WL_P2PO_ON()		0
#define WL_AWDL_ON()		0
#define WL_TXBF_ON()            0
#define WL_CHANLOG_ON()		0
#define WL_MUMIMO_ON()		0

#define WL_AMPDU_UPDN(args)
#define WL_AMPDU_RX(args)
#define WL_AMPDU_ERR(args)
#define WL_AMPDU_TX(args)
#define WL_AMPDU_CTL(args)
#define WL_AMPDU_HW(args)
#define WL_AMPDU_HWTXS(args)
#define WL_AMPDU_HWDBG(args)
#define WL_AMPDU_STAT(args)
#define WL_AMPDU_ERR_ON()       0
#define WL_AMPDU_HW_ON()        0
#define WL_AMPDU_HWTXS_ON()     0

#define WL_WNM_ON()		0
#endif /* LINUX_POSTMOGRIFY_REMOVAL */
#define WL_APSTA_UPDN(args)
#define WL_APSTA_RX(args)
#ifdef WLMSG_WSEC
#define WL_WSEC(args)		WL_PRINT(args)
#define WL_WSEC_DUMP(args)	WL_PRINT(args)
#else
#define WL_WSEC(args)
#define WL_WSEC_DUMP(args)
#endif
#define WL_PCIE(args)		do {if (wl_msg_level2 & WL_PCIE_VAL) WL_PRINT(args);} while (0)
#define WL_PCIE_ON()		(wl_msg_level2 & WL_PCIE_VAL)
#endif /* BCMDBG */

extern uint32 wl_msg_level;
extern uint32 wl_msg_level2;
#endif /* _wl_dbg_h_ */
