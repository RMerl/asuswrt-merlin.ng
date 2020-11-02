/*
 * Minimal debug/trace/assert driver definitions for
 * Broadcom 802.11 Networking Adapter.
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
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
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: wl_dbg.h 787038 2020-05-14 15:01:45Z $
 */

/* XXX, some old "wl msglevel" for PHY module has been moved to phy on 6/10/2009 "wl phymsglevel"
 * They are spare in TOT and can be reused if needed. see src/wl/phy/wlc_phy_int.h
 */

#ifndef _wl_dbg_h_
#define _wl_dbg_h_

#if defined(EVENT_LOG_COMPILE)
#include <event_log.h>
#endif // endif

#if defined(PKTC_TBL) && !defined(BCMDONGLEHOST)
#include <wlc_pub.h>
#include <wlc_dump.h>
#endif // endif

/* wl_msg_level is a bit vector with defs in wlioctl.h */
extern uint32 wl_msg_level;
extern uint32 wl_msg_level2;

#if !defined(BCMDONGLEHOST) && !defined(BCMDBG_EXCLUDE_HW_TIMESTAMP)
extern char* wlc_dbg_get_hw_timestamp(void);
#define WL_TIMESTAMP() do { if (wl_msg_level2 & WL_TIMESTAMP_VAL) {\
				printf(wlc_dbg_get_hw_timestamp()); }\
				} while (0)
#else
#define WL_TIMESTAMP()
#endif /* !BCMDONGLEHOST && !BCMDBG_EXCLUDE_HW_TIMESTAMP */

#ifdef ENABLE_CORECAPTURE
#define MAX_BACKTRACE_DEPTH 32
extern int wl_print_backtrace(const char * prefix, void * i_backtrace, int i_backtrace_depth);
#else
#define wl_print_backtrace(a, b, c)
#endif /* ENABLE_CORECAPTURE */

#define WIFICC_CAPTURE(_reason)
#define WIFICC_LOGDEBUGIF(_flags, _args)
#define WIFICC_LOGDEBUG(_args)

#define WL_PRINT(args)		do { if (wl_msg_level & WL_ERROR_VAL) { WL_TIMESTAMP(); printf args;} } while (0)

#if defined(EVENT_LOG_COMPILE) && defined(WLMSG_SRSCAN)
#define _WL_SRSCAN(fmt, ...)	EVENT_LOG(EVENT_LOG_TAG_SRSCAN, fmt, ##__VA_ARGS__)
#define WL_SRSCAN(args)		_WL_SRSCAN args
#else
#define WL_SRSCAN(args)
#endif // endif

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

#define	WL_ERROR(args)		do {if (wl_msg_level & WL_ERROR_VAL) WL_PRINT(args); \
					else WIFICC_LOGDEBUG(args);} while (0)

#define WL_IE_ERROR(args)	do {if (wl_msg_level & WL_ERROR_VAL) WL_PRINT(args); \
					else WIFICC_LOGDEBUG(args); } while (0)

#define WL_AMSDU_ERROR(args)	do {if (wl_msg_level & WL_ERROR_VAL) WL_PRINT(args); \
					else WIFICC_LOGDEBUG(args); } while (0)

#define WL_ASSOC_ERROR(args)	do {if (wl_msg_level & WL_ERROR_VAL) WL_PRINT(args); \
					else WIFICC_LOGDEBUG(args); } while (0)

#define WL_SCAN_ERROR(args)	do {if (wl_msg_level & WL_ERROR_VAL) WL_PRINT(args); \
					else WIFICC_LOGDEBUG(args); } while (0)
#define KM_ERR(args)		do {if (wl_msg_level & WL_ERROR_VAL) WL_PRINT(args); \
					else WIFICC_LOGDEBUG(args); } while (0)

#define WL_MBO_ERR(args)	do {if (wl_msg_level & WL_ERROR_VAL) WL_PRINT(args); \
					else WIFICC_LOGDEBUG(args); } while (0)

#define	WL_TRACE(args)		do {if (wl_msg_level & WL_TRACE_VAL) WL_PRINT(args);} while (0)

#define WL_PFN_ERROR(args)	do {if (wl_msg_level & WL_ERROR_VAL) WL_PRINT(args); \
					else WIFICC_LOGDEBUG(args); } while (0)

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
#define WL_ASSOC(args)		do {if (wl_msg_level & WL_ASSOC_VAL) { WL_TIMESTAMP(); printf args; } \
					else WIFICC_LOGDEBUG(args);} while (0)
#define	WL_PRUSR(m, b, n)	do {if (wl_msg_level & WL_PRUSR_VAL) prhex(m, b, n);} while (0)
#define WL_PS(args)		do {if (wl_msg_level & WL_PS_VAL) WL_PRINT(args);} while (0)
#define WL_SPARE1(args)		do {if (wl_msg_level & WL_TXPWR_VAL) WL_PRINT(args);} while (0)
// spare: WL_PORT was here
#define WL_DUAL(args)		do {if (wl_msg_level & WL_DUAL_VAL) WL_PRINT(args);} while (0)
#define WL_WSEC(args)		do {if (wl_msg_level & WL_WSEC_VAL) WL_PRINT(args);} while (0)
#define WL_WSEC_DUMP(args)	do {if (wl_msg_level & WL_WSEC_DUMP_VAL) WL_PRINT(args);} while (0)
#define WL_SPARE3(args)		do {if (wl_msg_level & WL_LOFT_VAL) WL_PRINT(args);} while (0)
#define WL_REGULATORY(args)	do {if (wl_msg_level & WL_REGULATORY_VAL) WL_PRINT(args);} while (0)
#define WL_SPARE4(args)		do {if (wl_msg_level & WL_PHYCAL_VAL) WL_PRINT(args);} while (0)
#define WL_ULO(args)		do {if (wl_msg_level & WL_ULMU_VAL) WL_PRINT(args);} while (0)
#define WL_MPC(args)		do {if (wl_msg_level & WL_MPC_VAL) WL_PRINT(args);} while (0)
#define WL_APSTA(args)		do {if (wl_msg_level & WL_APSTA_VAL) WL_PRINT(args);} while (0)
#define WL_DFS(args)		do {if (wl_msg_level & WL_DFS_VAL) WL_PRINT(args);} while (0)
#define WL_MUMIMO(args)		do {if (wl_msg_level & WL_MUMIMO_VAL) WL_PRINT(args);} while (0)
#define WL_MUTX(args)		do {if (wl_msg_level & WL_MUMIMO_VAL) WL_PRINT(args);} while (0)
#define WL_MODE_SWITCH(args) do {if (wl_msg_level & WL_MODE_SWITCH_VAL) WL_PRINT(args);} while (0)
#define WL_BCNTRIM_DBG(args) do {if (wl_msg_level & WL_BCNTRIM_VAL) WL_PRINT(args);} while (0)
#define WL_MBSS(args)		do {if (wl_msg_level & WL_MBSS_VAL) WL_PRINT(args);} while (0)
#define WL_CAC(args)		do {if (wl_msg_level & WL_CAC_VAL) WL_PRINT(args);} while (0)
#define WL_AMSDU(args)		do {if (wl_msg_level & WL_AMSDU_VAL) WL_PRINT(args);} while (0)
#define WL_AMPDU(args)		do {if (wl_msg_level & WL_AMPDU_VAL) WL_PRINT(args);} while (0)
#define WL_FFPLD(args)		do {if (wl_msg_level & WL_FFPLD_VAL) WL_PRINT(args);} while (0)
#define WL_PFN(args)		do {if (wl_msg_level & WL_PFN_VAL) WL_PRINT(args);} while (0)
#define WL_TWT(args)		do {if (wl_msg_level & WL_TWT_VAL) WL_PRINT(args);} while (0)
/* wl_msg_level is full. Use wl_msg_level_2 now */
#define WL_WOWL(args)		do {if (wl_msg_level2 & WL_WOWL_VAL) WL_PRINT(args);} while (0)
#define WL_SCAN(args)		do {if (wl_msg_level2 & WL_SCAN_VAL) WL_PRINT(args);} while (0)
#define	WL_COEX(args)		do {if (wl_msg_level2 & WL_COEX_VAL) WL_PRINT(args);} while (0)
#define WL_RTDC(w,s,i,j)	do {if (wl_msg_level2 & WL_RTDC_VAL) wlc_log(w,s,i,j);} while (0)
#define	WL_PROTO(args)		do {if (wl_msg_level2 & WL_PROTO_VAL) WL_PRINT(args);} while (0)
#define	WL_AIRIQ(args)		do {if (wl_msg_level2 & WL_AIRIQ_VAL) WL_PRINT(args);} while (0)
#define WL_RTDC2(w,s,i,j)	do {if (wl_msg_level2 & 0) wlc_log(w,s,i,j);} while (0)
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
#define WL_RRM(args)            do {if (wl_msg_level2 & WL_RRM_VAL) WL_PRINT(args);} while (0)
#define WL_RRM_HEX(m, b, n)     do {if (wl_msg_level2 & WL_RRM_VAL) prhex(m, b, n);} while (0)
#define WL_TRF_MGMT(args)	do {if (wl_msg_level2 & WL_TRF_MGMT_VAL) WL_PRINT(args);} while (0)
#define WL_MAC(args)		do {if (wl_msg_level2 & WL_MAC_VAL) WL_PRINT(args);} while (0)
#define WL_L2FILTER(args) do {if (wl_msg_level2 & WL_L2FILTER_VAL) WL_PRINT(args);} while (0)
#define WL_TSO(args)		do {if (wl_msg_level2 & WL_TSO_VAL) WL_PRINT(args);} while (0)
#define WL_MQ(args)		do {if (wl_msg_level2 & WL_MQ_VAL) WL_PRINT(args);} while (0)
#define WL_P2PO(args)		do {if (wl_msg_level2 & WL_P2PO_VAL) WL_PRINT(args);} while (0)
#define WL_WNM(args)		do {if (wl_msg_level2 & WL_WNM_VAL) WL_PRINT(args);} while (0)
#define WL_TXBF(args)           do {if (wl_msg_level2 & WL_TXBF_VAL) WL_PRINT(args);} while (0)
#define	WL_PCIE(args)		do {if (wl_msg_level2 & WL_PCIE_VAL) WL_PRINT(args);} while (0)
#define WL_TSLOG(w, s, i, j)	\
    do {if (wl_msg_level2 & WL_TIMESTAMP_VAL) wlc_log(w, s, i, j);} while (0)
#define WL_NAT(args)           do {if (wl_msg_level2 & WL_NATOE_VAL) WL_PRINT(args);} while (0)
/* not using WL_ROAM for BCMDBG at the moment */
#define WL_ROAM(args)
#define WL_PRMAC(args)	do {if (wl_msg_level & WL_PRMAC_VAL) WL_PRINT(args);} while (0)
#define WL_FBT(args)	do {if (wl_msg_level2 & WL_FBT_VAL) WL_PRINT(args);} while (0)
#define WL_SWDIV(args)	do {if (wl_msg_level2 & WL_SWDIV_VAL) WL_PRINT(args);} while (0)
#define WL_MBO_DBG(args) do {if (wl_msg_level2 & WL_MBO_VAL) WL_PRINT(args);} while (0)
#define WL_BAM_ERR(args)	do {if (wl_msg_level2 & WL_ERROR_VAL)	WL_PRINT(args);} while (0)
#define WL_ADPS(args)	do {if (wl_msg_level2 & WL_ADPS_VAL) WL_PRINT(args);} while (0)
#define WL_OCE_DBG(args) do {if (wl_msg_level2 & WL_OCE_VAL) WL_PRINT(args);} while (0)
#define WL_OCE_ERR(args) do {if (wl_msg_level2 & WL_OCE_VAL) WL_PRINT(args);} while (0)
#define WL_OCE_INFO(args) do {if (wl_msg_level2 & WL_OCE_VAL) WL_PRINT(args);} while (0)
#define WL_TAF(args)		do {if (wl_msg_level & WL_TAF_VAL) WL_PRINT(args);} while (0)

#define WL_ERROR_ON()		(wl_msg_level & WL_ERROR_VAL)
#define WL_TRACE_ON()		(wl_msg_level & WL_TRACE_VAL)
#define WL_PRHDRS_ON()		(wl_msg_level & WL_PRHDRS_VAL)
#define WL_PRPKT_ON()		(wl_msg_level & WL_PRPKT_VAL)
#define WL_INFORM_ON()		(wl_msg_level & WL_INFORM_VAL)
#define WL_TMP_ON()		(wl_msg_level & WL_TMP_VAL)
#define WL_OID_ON()		(wl_msg_level & WL_OID_VAL)
#define WL_RATE_ON()		(wl_msg_level & WL_RATE_VAL)
#define WL_ASSOC_ON()		(wl_msg_level & WL_ASSOC_VAL)
#define WL_WSEC_ON()		(wl_msg_level & WL_WSEC_VAL)
#define WL_WSEC_DUMP_ON()	(wl_msg_level & WL_WSEC_DUMP_VAL)
#define WL_MPC_ON()		(wl_msg_level & WL_MPC_VAL)
#define WL_REGULATORY_ON()	(wl_msg_level & WL_REGULATORY_VAL)
#define WL_APSTA_ON()		(wl_msg_level & WL_APSTA_VAL)
#define WL_DFS_ON()		(wl_msg_level & WL_DFS_VAL)
#define WL_MUMIMO_ON()		(wl_msg_level & WL_MUMIMO_VAL)
#define WL_MUTX_ON()		(wl_msg_level & WL_MUMIMO_VAL)
#ifdef WL11AC
#define WL_MODE_SWITCH_ON()	(wl_msg_level & WL_MODE_SWITCH_VAL)
#endif // endif
#define WL_MBSS_ON()		(wl_msg_level & WL_MBSS_VAL)
#define WL_AMPDU_ON()		(wl_msg_level & WL_AMPDU_VAL)
#define WL_PFN_ON()		(wl_msg_level & WL_PFN_VAL)
#define WL_TWT_ON()		(wl_msg_level & WL_TWT_VAL)
#define	WL_AIRIQ_ON()		(wl_msg_level2 & WL_ACI_VAL)
#define WL_WOWL_ON()		(wl_msg_level2 & WL_WOWL_VAL)
#define WL_SCAN_ON()		(wl_msg_level2 & WL_SCAN_VAL)
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
#define WL_MAC_ON()		(wl_msg_level2 & WL_MAC_VAL)
#define WL_PWRSEL_ON()		(wl_msg_level2 & WL_PWRSEL_VAL)
#define WL_L2FILTER_ON()	(wl_msg_level2 & WL_L2FILTER_VAL)
#define WL_MQ_ON()		(wl_msg_level2 & WL_MQ_VAL)
#define WL_P2PO_ON()		(wl_msg_level2 & WL_P2PO_VAL)
#define WL_WNM_ON()		(wl_msg_level2 & WL_WNM_VAL)
#define WL_TXBF_ON()	        (wl_msg_level2 & WL_TXBF_VAL)
#define WL_PCIE_ON()		(wl_msg_level2 & WL_PCIE_VAL)
#define WL_TSLOG_ON()		(wl_msg_level2 & WL_TIMESTAMP_VAL)
#define WL_SWDIV_ON()		(wl_msg_level2 & WL_SWDIV_VAL)
#define WL_MBO_DBG_ON()		(wl_msg_level2 & WL_MBO_VAL)
#define WL_ADPS_ON()            (wl_msg_level2 & WL_ADPS_VAL)
#define WL_OCE_DBG_ON()		(wl_msg_level2 & WL_OCE_VAL)
#define WL_OCE_ERR_ON()		(wl_msg_level2 & WL_OCE_VAL)
#define WL_OCE_INFO_ON()	(wl_msg_level2 & WL_OCE_VAL)
#define WL_ULMU_ON()		(wl_msg_level & WL_ULMU_VAL)

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
#define WL_APSTA_RX_ON() (wl_apsta_dbg & WL_APSTA_RX_VAL)

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
#define DBGONLY(x) x

/* To disable a message completely ... until you need it again */
#define WL_NONE(args)
#define WL_ERROR(args)		do {if (wl_msg_level & WL_ERROR_VAL) WL_PRINT(args); \
					else WIFICC_LOGDEBUG(args); } while (0)
#define	WL_SCAN_ERROR(args)
#define	WL_IE_ERROR(args)
#define	WL_AMSDU_ERROR(args)
#define	WL_ASSOC_ERROR(args)
#define	KM_ERR(args)

#define WL_TRACE(args)
#define WL_PRHDRS_MSG(args)
#define WL_PRHDRS(i, p, f, t, r, l)
#define WL_PRPKT(m, b, n)
#define WL_INFORM(args)
#define WL_TMP(args)
#define WL_OID(args)
#define WL_RATE(args)		do {if (wl_msg_level & WL_RATE_VAL) WL_PRINT(args);} while (0)
#define WL_ASSOC(args)		do {if (wl_msg_level & WL_ASSOC_VAL) WL_PRINT(args); \
					else WIFICC_LOGDEBUG(args);} while (0)
#define WL_PRUSR(m, b, n)
#define WL_PS(args)		do {if (wl_msg_level & WL_PS_VAL) WL_PRINT(args);} while (0)

//#define WL_PORT(args)
#define WL_DUAL(args)
#define WL_REGULATORY(args)	do {if (wl_msg_level & WL_REGULATORY_VAL) WL_PRINT(args); \
					else WIFICC_LOGDEBUG(args);} while (0)

#define WL_MPC(args)
#define WL_APSTA(args)
#define WL_APSTA_BCN(args)
#define WL_APSTA_TX(args)
#define WL_APSTA_TSF(args)
#define WL_APSTA_BSSID(args)
#define WL_MBSS(args)
#define WL_MODE_SWITCH(args)
#define WL_PROTO(args)

#define	WL_CAC(args)		do {if (wl_msg_level & WL_CAC_VAL) WL_PRINT(args);} while (0)
#define WL_AMSDU(args)
#define WL_AMPDU(args)
#define WL_FFPLD(args)
#define WL_TWT(args)
#define WL_MCHAN(args)

#define WL_DFS(args)
#define WL_WOWL(args)
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
#define WL_MAC(args)
#define WL_L2FILTER(args)
#define WL_MQ(args)
#define WL_TXBF(args)
#define WL_MUMIMO(args)
#define WL_MUTX(args)
#define WL_P2PO(args)
#define WL_ROAM(args)
#define WL_WNM(args)
#define WL_NAT(args)
#define WL_ULO(args)

#ifdef WLMSG_NATOE
#define WL_NAT(args)           do {if (wl_msg_level2 & WL_NATOE_VAL) WL_PRINT(args);} while (0)
#else
#define WL_NAT(args)
#endif // endif

#ifdef BCMDBG_ERR
#if defined(EVENT_LOG_COMPILE) && defined(ERR_USE_EVENT_LOG)
#if defined(ERR_USE_EVENT_LOG_RA)
#define   WL_PFN_ERROR(args)   EVENT_LOG_RA(EVENT_LOG_TAG_SCAN_ERR, args)
#else
#define   WL_PFN_ERROR(args)   EVENT_LOG_COMPACT_CAST_PAREN_ARGS(EVENT_LOG_TAG_SCAN_ERR, args)
#endif /* ERR_USE_EVENT_LOG_RA */
#else
#define   WL_PFN_ERROR(args)    WL_PRINT(args)
#endif /* ERR_USE_EVENT_LOG */
#else
#define WL_PFN_ERROR(args)
#endif /* BCMDBG_ERR */

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
#define WL_APSTA_RX_ON()        0
#define WL_WSEC(args)
#define WL_WSEC_DUMP(args)
#define WL_PCIE(args)
#define	WL_AIRIQ(args)
#define WL_TSLOG(w, s, i, j)
#define WL_FBT(args)
#define WL_MBO_DBG(args)
#define WL_BAM_ERR(args)
#define WL_ADPS(args)
#define WL_OCE_DBG(args)
#define WL_OCE_ERR(args)
#define WL_OCE_INFO(args)

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
#define WL_WSEC_ON()		0
#define WL_WSEC_DUMP_ON()	0
#define WL_MPC_ON()		0
#define WL_REGULATORY_ON()	(wl_msg_level & WL_REGULATORY_VAL)
#define WL_APSTA_ON()		0
#define WL_DFS_ON()		0
#define WL_MBSS_ON()		0
#define WL_CAC_ON()		(wl_msg_level & WL_CAC_VAL)
#define WL_AMPDU_ON()		0
#define WL_TWT_ON()		0
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
#define WL_MAC_ON()		0
#define WL_LPC_ON()		0
#define WL_L2FILTER_ON()	0
#define WL_TXBF_ON()		0
#define WL_P2PO_ON()		0
#define WL_TSLOG_ON()		0
#define WL_WNM_ON()		0
#define WL_PCIE_ON()		0
#define WL_MUMIMO_ON()		0
#define WL_MUTX_ON()		0
#define WL_AIRIQ_ON()		0
#define WL_MBO_DBG_ON()		0
#define WL_ADPS_ON()            0
#define WL_OCE_DBG_ON()		0
#define WL_ULMU_ON()		0

#else /* !BCMDBG */

/* DBGONLY() macro to reduce ifdefs in code for statements that are only needed when
 * BCMDBG is defined.
 */
#define DBGONLY(x)

/* To disable a message completely ... until you need it again */
#define WL_NONE(args)

#define	WL_AIRIQ(args)
#define WL_AIRIQ_ON()		0

#ifdef BCMDBG_ERR
/* ROM and ROML optimized builds */
#if defined(EVENT_LOG_COMPILE) && defined(ERR_USE_EVENT_LOG)

#if defined(ERR_USE_EVENT_LOG_RA)
#define   WL_ERROR(args)	EVENT_LOG_RA(EVENT_LOG_TAG_WL_ERROR, args)
#else
#define   WL_ERROR(args)        EVENT_LOG_COMPACT_CAST_PAREN_ARGS(EVENT_LOG_TAG_WL_ERROR, args)
#endif /* ERR_USE_EVENT_LOG_RA */

#else
#define   WL_ERROR(args)          WL_PRINT(args)
#endif /* ERR_USE_EVENT_LOG */
#else
#define	WL_ERROR(args)
#endif /* BCMDBG_ERR */

#ifdef BCMDBG_ERR
/* ROM and ROML optimized builds */
#if defined(EVENT_LOG_COMPILE)&& defined(ERR_USE_EVENT_LOG)
#if defined(ERR_USE_EVENT_LOG_RA)
#define  KM_ERR(args)	EVENT_LOG_RA(EVENT_LOG_TAG_KM_ERROR, args)
#else
#define  KM_ERR(args)	EVENT_LOG_COMPACT_CAST_PAREN_ARGS(EVENT_LOG_TAG_KM_ERROR, args)
#endif /* ERR_USE_EVENT_LOG_RA */
#else
#define  KM_ERR(args)     WL_PRINT(args)
#endif /* ERR_USE_EVENT_LOG */
#else
#define  KM_ERR(args)
#endif /* BCMDBG_ERR */

#ifdef BCMDBG_ERR
/* ROM and ROML optimized builds */
#if defined(EVENT_LOG_COMPILE) && defined(ERR_USE_EVENT_LOG)
#if defined(ERR_USE_EVENT_LOG_RA)
#define WL_AMPDU_ERR(args)	EVENT_LOG_RA(EVENT_LOG_TAG_AMPDU_ERROR, args)
#else
#define WL_AMPDU_ERR(args)    EVENT_LOG_COMPACT_CAST_PAREN_ARGS(EVENT_LOG_TAG_AMPDU_ERROR, args)
#endif /* ERR_USE_EVENT_LOG_RA */
#else
#define WL_AMPDU_ERR(args)    WL_PRINT(args)
#endif /* ERR_USE_EVENT_LOG */
#else
#define	WL_AMPDU_ERR(args)
#endif /* BCMDBG_ERR */

#define	WL_TRACE(args)
#ifndef LINUX_POSTMOGRIFY_REMOVAL
#ifdef WLMSG_PRHDRS
#define	WL_PRHDRS_MSG(args)		WL_PRINT(args)
#define WL_PRHDRS(i, p, f, t, r, l)	wlc_print_hdrs(i, p, f, t, r, l)
#else
#define	WL_PRHDRS_MSG(args)
#define	WL_PRHDRS(i, p, f, t, r, l)
#endif // endif
#ifdef WLMSG_PRPKT
#define	WL_PRPKT(m, b, n)	prhex(m, b, n)
#else
#define	WL_PRPKT(m, b, n)
#endif // endif
#ifdef WLMSG_INFORM
#define	WL_INFORM(args)		WL_PRINT(args)
#else
#define	WL_INFORM(args)
#endif // endif
#define	WL_TMP(args)
#ifdef WLMSG_OID
#define WL_OID(args)		WL_PRINT(args)
#else
#define WL_OID(args)
#endif // endif
#define	WL_RATE(args)

#ifdef BCMDBG_ERR
#if defined(EVENT_LOG_COMPILE) && defined(ERR_USE_EVENT_LOG)
#if defined(ERR_USE_EVENT_LOG_RA)
#define   WL_IE_ERROR(args)	EVENT_LOG_RA(EVENT_LOG_TAG_IE_ERROR, args)
#else
#define   WL_IE_ERROR(args)	EVENT_LOG_COMPACT_CAST_PAREN_ARGS(EVENT_LOG_TAG_IE_ERROR, args)
#endif /* ERR_USE_EVENT_LOG_RA */
#else
#define   WL_IE_ERROR(args)	WL_PRINT(args)
#endif /* ERR_USE_EVENT_LOG */
#else
#define	  WL_IE_ERROR(args)
#endif /* BCMDBG_ERR */

#ifdef WLMSG_ASSOC
#if defined(EVENT_LOG_COMPILE) && defined(EVENT_LOG_COMPILE)
#if defined(USE_EVENT_LOG_RA)
#define WL_ASSOC(args)   EVENT_LOG_RA(EVENT_LOG_TAG_WL_ASSOC_LOG, args)
#else
#define WL_ASSOC(args)   EVENT_LOG_FAST_CAST_PAREN_ARGS(EVENT_LOG_TAG_WL_ASSOC_LOG, args)
#endif /* USE_EVENT_LOG_RA */
#else
#define WL_ASSOC(args)   WL_PRINT(args)
#endif /* EVENT_LOG_COMPILE */
#else
#define	WL_ASSOC(args)
#endif /* WLMSG_ASSOC */

#ifdef BCMDBG_ERR
/* ROM and ROML optimized builds */
#if defined(EVENT_LOG_COMPILE) && defined(ERR_USE_EVENT_LOG)
#if defined(ERR_USE_EVENT_LOG_RA)
#define WL_ASSOC_ERROR(args)	EVENT_LOG_RA(EVENT_LOG_TAG_ASSOC_ERROR, args)
#else
#define WL_ASSOC_ERROR(args)  EVENT_LOG_COMPACT_CAST_PAREN_ARGS(EVENT_LOG_TAG_ASSOC_ERROR, args)
#endif /* ERR_USE_EVENT_LOG_RA */
#else
#define  WL_ASSOC_ERROR(args)      WL_PRINT(args)
#endif /* ERR_USE_EVENT_LOG */
#else
#define	 WL_ASSOC_ERROR(args)
#endif /* BCMDBG_ERR */

#ifdef BCMDBG_ERR
/* ROM and ROML optimized builds */
#if defined(EVENT_LOG_COMPILE) && defined(ERR_USE_EVENT_LOG)
#if defined(ERR_USE_EVENT_LOG_RA)
#define   WL_SCAN_ERROR(args)	EVENT_LOG_RA(EVENT_LOG_TAG_SCAN_ERR, args)
#else
#define   WL_SCAN_ERROR(args)   EVENT_LOG_COMPACT_CAST_PAREN_ARGS(EVENT_LOG_TAG_SCAN_ERR, args)
#endif /* ERR_USE_EVENT_LOG_RA */
#else
#define   WL_SCAN_ERROR(args)    WL_PRINT(args)
#endif /* ERR_USE_EVENT_LOG */
#else
#define	WL_SCAN_ERROR(args)
#endif /* BCMDBG_ERR */

#define	WL_PRUSR(m, b, n)

#ifdef WLMSG_PS
#if defined(EVENT_LOG_COMPILE) && defined(EVENT_LOG_COMPILE)
#if defined(USE_EVENT_LOG_RA)
#define WL_PS(args)   EVENT_LOG_RA(EVENT_LOG_TAG_WL_PS_LOG, args)
#else
#define WL_PS(args)   EVENT_LOG_FAST_CAST_PAREN_ARGS(EVENT_LOG_TAG_WL_PS_LOG, args)
#endif /* USE_EVENT_LOG_RA */
#else
#define WL_PS(args)   WL_PRINT(args)
#endif /* EVENT_LOG_COMPILE */
#else
#define WL_PS(args)
#endif /* WLMSG_PS */

#ifdef BCMDBG_ERR
/* ROM and ROML optimized builds */
#if defined(EVENT_LOG_COMPILE) && defined(ERR_USE_EVENT_LOG)
#if defined(ERR_USE_EVENT_LOG_RA)
#define WL_AMSDU_ERROR(args)	EVENT_LOG_RA(EVENT_LOG_TAG_AMSDU_ERROR, args)
#else
#define WL_AMSDU_ERROR(args)    EVENT_LOG_COMPACT_CAST_PAREN_ARGS(EVENT_LOG_TAG_AMSDU_ERROR, args)
#endif /* ERR_USE_EVENT_LOG_RA */
#else
#define   WL_AMSDU_ERROR(args)          WL_PRINT(args)
#endif /* ERR_USE_EVENT_LOG */
#else
#define	WL_AMSDU_ERROR(args)
#endif /* BCMDBG_ERR */

#ifdef WLMSG_ROAM
#if defined(EVENT_LOG_COMPILE) && defined(EVENT_LOG_COMPILE)
#if defined(USE_EVENT_LOG_RA)
#define WL_ROAM(args)	EVENT_LOG_RA(EVENT_LOG_TAG_WL_ROAM_LOG, args)
#else
#define WL_ROAM(args)   EVENT_LOG_FAST_CAST_PAREN_ARGS(EVENT_LOG_TAG_WL_ROAM_LOG, args)
#endif /* USE_EVENT_LOG_RA */
#else
#define WL_ROAM(args)   WL_PRINT(args)
#endif /* EVENT_LOG_COMPILE */
#else
#define WL_ROAM(args)
#endif /* WLMSG_ROAM */

//#define WL_PORT(args)
#define WL_DUAL(args)
#define WL_REGULATORY(args)
#define WL_TAF(args)

#ifdef WLMSG_MPC
#if defined(EVENT_LOG_COMPILE) && defined(EVENT_LOG_COMPILE)
#if defined(USE_EVENT_LOG_RA)
#define WL_MPC(args)   EVENT_LOG_RA(EVENT_LOG_TAG_WL_MPC_LOG, args)
#else
#define WL_MPC(args)   EVENT_LOG_FAST_CAST_PAREN_ARGS(EVENT_LOG_TAG_WL_MPC_LOG, args)
#endif /* USE_EVENT_LOG_RA */
#else
#define WL_MPC(args)   WL_PRINT(args)
#endif /* EVENT_LOG_COMPILE */
#else
#define WL_MPC(args)
#endif /* WLMSG_MPC */

#define WL_APSTA(args)
#define WL_APSTA_BCN(args)
#define WL_APSTA_TX(args)
#define WL_APSTA_TSF(args)
#define WL_APSTA_BSSID(args)
#define WL_MBSS(args)
#define WL_MODE_SWITCH(args)
#define	WL_PROTO(args)

#define	WL_CAC(args)
#define WL_AMSDU(args)
#define WL_AMPDU(args)
#define WL_FFPLD(args)
#define WL_TWT(args)
#define WL_MCHAN(args)
#define WL_BCNTRIM_DBG(args)

/* Define WLMSG_DFS automatically for WLTEST builds */
#if defined(WLTEST)
#ifndef WLMSG_DFS
#define WLMSG_DFS
#endif // endif
#endif /* WLTEST */

#ifdef WLMSG_DFS
#define WL_DFS(args)		do {if (wl_msg_level & WL_DFS_VAL) WL_PRINT(args);} while (0)
#else /* WLMSG_DFS */
#define WL_DFS(args)
#endif /* WLMSG_DFS */
#define WL_WOWL(args)

#ifdef WLMSG_SCAN
#if defined(EVENT_LOG_COMPILE) && defined(EVENT_LOG_COMPILE)
#if defined(USE_EVENT_LOG_RA)
#define   WL_SCAN(args)		EVENT_LOG_RA(EVENT_LOG_TAG_SCAN_TRACE_LOW, args)
#else
#define   WL_SCAN(args)		EVENT_LOG_FAST_CAST_PAREN_ARGS(EVENT_LOG_TAG_SCAN_TRACE_LOW, args)
#endif /* USE_EVENT_LOG_RA */
#else
#define   WL_SCAN(args)          WL_PRINT(args)
#endif /* EVENT_LOG_COMPILE */
#else
#define WL_SCAN(args)
#endif /* WLMSG_SCAN */

#define	WL_COEX(args)
#define WL_RTDC(w, s, i, j)
#define WL_RTDC2(w, s, i, j)
#define WL_CHANINT(args)
#ifdef WLMSG_BTA
#define WL_BTA(args)		WL_PRINT(args)
#else
#define WL_BTA(args)
#endif // endif
#define WL_WMF(args)
#define WL_P2P(args)
#define WL_ITFR(args)
#define WL_TDLS(args)

#ifdef WLMSG_MCNX
#if defined(EVENT_LOG_COMPILE) && defined(EVENT_LOG_COMPILE)
#if defined(USE_EVENT_LOG_RA)
#define WL_MCNX(args)	EVENT_LOG_RA(EVENT_LOG_TAG_WL_MCNX_LOG, args)
#else
#define WL_MCNX(args)   EVENT_LOG_FAST_CAST_PAREN_ARGS(EVENT_LOG_TAG_WL_MCNX_LOG, args)
#endif /* USE_EVENT_LOG_RA */
#else
#define WL_MCNX(args)   WL_PRINT(args)
#endif /* EVENT_LOG_COMPILE */
#else
#define WL_MCNX(args)
#endif /* WLMSG_MCNX */

#define WL_PROT(args)
#define WL_PSTA(args)
#define WL_TBTT(args)
#define WL_TRF_MGMT(args)
#define WL_MAC(args)
#define WL_L2FILTER(args)
#define WL_MQ(args)
#define WL_P2PO(args)
#define WL_WNM(args)
#define WL_TXBF(args)
#define WL_TSLOG(w, s, i, j)
#define WL_FBT(args)
#define WL_MUMIMO(args)
#define WL_MUTX(args)
#define WL_RRM(args)
#define WL_RRM_HEX(m, b, n)
#define WL_SWDIV(args)
#define WL_ADPS(args)
#define WL_ULO(args)

#ifdef BCMDBG_ERR
#define WL_ERROR_ON()		1
#else
#define WL_ERROR_ON()		0
#endif // endif
#define WL_TRACE_ON()		0
#ifdef WLMSG_PRHDRS
#define WL_PRHDRS_ON()		1
#else
#define WL_PRHDRS_ON()		0
#endif // endif
#ifdef WLMSG_PRPKT
#define WL_PRPKT_ON()		1
#else
#define WL_PRPKT_ON()		0
#endif // endif
#ifdef WLMSG_INFORM
#define WL_INFORM_ON()		1
#else
#define WL_INFORM_ON()		0
#endif // endif
#ifdef WLMSG_OID
#define WL_OID_ON()		1
#else
#define WL_OID_ON()		0
#endif // endif
#define WL_TMP_ON()		0
#define WL_RATE_ON()		0
#ifdef WLMSG_ASSOC
#define WL_ASSOC_ON()		1
#else
#define WL_ASSOC_ON()		0
#endif // endif

#ifdef WLMSG_WSEC
#define WL_WSEC_ON()		1
#define WL_WSEC_DUMP_ON()	1
#else
#define WL_WSEC_ON()		0
#define WL_WSEC_DUMP_ON()	0
#endif // endif
#ifdef WLMSG_MPC
#define WL_MPC_ON()		1
#else
#define WL_MPC_ON()		0
#endif // endif
#define WL_REGULATORY_ON()	0

#define WL_APSTA_ON()		0
#define WL_MBSS_ON()		0
#define WL_MODE_SWITCH_ON()	0
#ifdef WLMSG_DFS
#define WL_DFS_ON()		1
#else /* WLMSG_DFS */
#define WL_DFS_ON()		0
#endif /* WLMSG_DFS */
#define WL_TWT_ON()		0
#ifdef WLMSG_SCAN
#define WL_SCAN_ON()            1
#else
#define WL_SCAN_ON()            0
#endif // endif
#ifdef WLMSG_BTA
#define WL_BTA_ON()		1
#else
#define WL_BTA_ON()		0
#endif // endif
#define WL_WMF_ON()		0
#define WL_P2P_ON()		0
#define WL_MCHAN_ON()		0
#define WL_TDLS_ON()		0
#define WL_MCNX_ON()		0
#define WL_PROT_ON()		0
#define WL_TBTT_ON()		0
#define WL_PWRSEL_ON()		0
#define WL_L2FILTER_ON()	0
#define WL_MAC_ON()		0
#define WL_MQ_ON()		0
#define WL_P2PO_ON()		0
#define WL_TXBF_ON()            0
#define WL_TSLOG_ON()		0
#define WL_MUMIMO_ON()		0
#define WL_MUTX_ON()		0
#define WL_SWDIV_ON()		0
#define WL_ULMU_ON()            0

#define WL_AMPDU_UPDN(args)
#define WL_AMPDU_RX(args)
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
#ifdef WLMSG_MBO
#define WL_MBO_DBG_ON()		1
#else
#define WL_MBO_DBG_ON()		0
#endif /* WLMSG_MBO */
#define WL_ADPS_ON()            0
#ifdef WLMSG_OCE
#define WL_OCE_DBG_ON()		1
#else
#define WL_OCE_DBG_ON()		0
#endif /* WLMSG_OCE */
#endif /* LINUX_POSTMOGRIFY_REMOVAL */
#define WL_APSTA_UPDN(args)
#define WL_APSTA_RX(args)
#define WL_APSTA_RX_ON()	0

#ifdef WLMSG_WSEC
#if defined(EVENT_LOG_COMPILE) && defined(EVENT_LOG_COMPILE)
#if defined(USE_EVENT_LOG_RA)
#define   WL_WSEC(args)	EVENT_LOG_RA(EVENT_LOG_TAG_WL_WSEC_LOG, args)
#define   WL_WSEC_DUMP(args)	EVENT_LOG_RA(EVENT_LOG_TAG_WL_WSEC_DUMP, args)
#else
#define   WL_WSEC(args)   EVENT_LOG_FAST_CAST_PAREN_ARGS(EVENT_LOG_TAG_WL_WSEC_LOG, args)
#define   WL_WSEC_DUMP(args)   EVENT_LOG_FAST_CAST_PAREN_ARGS(EVENT_LOG_TAG_WL_WSEC_DUMP, args)
#endif /* USE_EVENT_LOG_RA */
#else
#define WL_WSEC(args)		WL_PRINT(args)
#define WL_WSEC_DUMP(args)	WL_PRINT(args)
#endif /* EVENT_LOG_COMPILE */
#else
#define WL_WSEC(args)
#define WL_WSEC_DUMP(args)
#endif /* WLMSG_WSEC */

#ifdef WLMSG_MBO
#if defined(EVENT_LOG_COMPILE) && defined(EVENT_LOG_COMPILE)
#if defined(USE_EVENT_LOG_RA)
#define   WL_MBO_DBG(args)		EVENT_LOG_RA(EVENT_LOG_TAG_MBO_DBG, args)
#define   WL_MBO_INFO(args)	EVENT_LOG_RA(EVENT_LOG_TAG_MBO_INFO, args)
#else
#define   WL_MBO_DBG(args)	 \
			EVENT_LOG_COMPACT_CAST_PAREN_ARGS(EVENT_LOG_TAG_MBO_DBG, args)
#define   WL_MBO_INFO(args)	 \
			EVENT_LOG_COMPACT_CAST_PAREN_ARGS(EVENT_LOG_TAG_MBO_INFO, args)
#endif /* USE_EVENT_LOG_RA */
#else
#define   WL_MBO_DBG(args)		   WL_PRINT(args)
#define   WL_MBO_INFO(args)		WL_PRINT(args)
#endif /* EVENT_LOG_COMPILE */
#else
#define	  WL_MBO_DBG(args)
#define	  WL_MBO_INFO(args)
#endif /* WLMSG_MBO */

#ifdef BCMDBG_ERR
#if defined(EVENT_LOG_COMPILE) && defined(ERR_USE_EVENT_LOG)
#if defined(ERR_USE_EVENT_LOG_RA)
#define   WL_MBO_ERR(args)		EVENT_LOG_RA(EVENT_LOG_TAG_MBO_ERR, args)
#else
#define   WL_MBO_ERR(args)	EVENT_LOG_COMPACT_CAST_PAREN_ARGS(EVENT_LOG_TAG_MBO_ERR, args)
#endif /* ERR_USE_EVENT_LOG_RA */
#else
#define   WL_MBO_ERR(args)		WL_PRINT(args)
#endif /* ERR_USE_EVENT_LOG */
#else
#define   WL_MBO_ERR(args)		WL_PRINT(args)
#endif /* ERR_USE_EVENT_LOG */

#ifdef WLMSG_OCE
#if defined(EVENT_LOG_COMPILE)
#if defined(USE_EVENT_LOG_RA)
#define   WL_OCE_DBG(args)		EVENT_LOG_RA(EVENT_LOG_TAG_OCE_DBG, args)
#define   WL_OCE_INFO(args)	EVENT_LOG_RA(EVENT_LOG_TAG_OCE_INFO, args)
#else
#define   WL_OCE_DBG(args)	 \
		EVENT_LOG_COMPACT_CAST_PAREN_ARGS(EVENT_LOG_TAG_OCE_DBG, args)
#define   WL_OCE_INFO(args)	 \
			EVENT_LOG_COMPACT_CAST_PAREN_ARGS(EVENT_LOG_TAG_OCE_INFO, args)
#endif /* USE_EVENT_LOG_RA */
#else
#define   WL_OCE_DBG(args)		WL_PRINT(args)
#define   WL_OCE_INFO(args)		WL_PRINT(args)
#endif /* EVENT_LOG_COMPILE */
#else
#define	  WL_OCE_DBG(args)
#define	  WL_OCE_INFO(args)
#endif /* WLMSG_OCE */

#ifdef BCMDBG_ERR
#if defined(ERR_USE_EVENT_LOG)
#if defined(ERR_USE_EVENT_LOG_RA)
#define   WL_OCE_ERR(args)		EVENT_LOG_RA(EVENT_LOG_TAG_OCE_ERR, args)
#else
#define   WL_OCE_ERR(args)	 \
			EVENT_LOG_COMPACT_CAST_PAREN_ARGS(EVENT_LOG_TAG_OCE_ERR, args)
#endif /* ERR_USE_EVENT_LOG_RA */
#else
#define   WL_OCE_ERR(args)		WL_PRINT(args)
#endif /* ERR_USE_EVENT_LOG */
#else
#define   WL_OCE_ERR(args)		WL_PRINT(args)
#endif /* ERR_USE_EVENT_LOG */

#define WL_PCIE(args)		do {if (wl_msg_level2 & WL_PCIE_VAL) WL_PRINT(args);} while (0)
#define WL_PCIE_ON()		(wl_msg_level2 & WL_PCIE_VAL)
#define WL_PFN(args)      do {if (wl_msg_level & WL_PFN_VAL) WL_PRINT(args);} while (0)
#define WL_PFN_ON()		(wl_msg_level & WL_PFN_VAL)
#endif /* BCMDBG */

#define WL_EAP_TRC_ERROR(args)
#define WL_EAP_TRC_INFO(args)
#define WL_EAP_TRC_SCAN(args)
#define WL_EAP_TRC_SCAN_DBG(args)
#define WL_EAP_TRC_INTR(args)
#define WL_EAP_TRC_RM(args)
#define WL_EAP_TRC_AMPDU(args)
#define WL_EAP_TRC_AMSDU(args)
#define WL_EAP_TRC_RXTXUTIL_MSG(args)
#define WL_EAP_TRC_DTIM(args)
#define WL_EAP_TRC_EVENT(args)

#ifdef WLMSG_BAM
#if defined(EVENT_LOG_COMPILE)
#ifdef USE_EVENT_LOG_RA
#define WL_BAM_ERR(args)	EVENT_LOG_RA(EVENT_LOG_TAG_BAM, args)
#else
#define WL_BAM_ERR(args)	EVENT_LOG_COMPACT_CAST_PAREN_ARGS(EVENT_LOG_TAG_BAM, args)
#endif /* USE_EVENT_LOG_RA */
#else
#define WL_BAM_ERR(args)	WL_PRINT(args)
#endif /* EVENT_LOG_COMPILE */
#endif /* WLMSG_BAM */

#if defined(BCMDBG) || defined(BCMDBG_ERR)
#define DBGERRONLY(x) x
#else
#define DBGERRONLY(x)
#endif // endif

extern uint32 wl_msg_level;
extern uint32 wl_msg_level2;

/* Customers ask for more visibility when dealing with association problems, even in 'production'
 * firmware builds. Enabling WLMSG_ASSOC consumes approx 42KB of 4366c0 dongle RAM. As a lightweight
 * alternative, WLMSG_ASSOC_LT consumes 1KB on 4366c0. It does not generate all messages that its
 * full-blown sibling features, but the essential ones.
 */
#ifndef WLMSG_ASSOC_LT
#define WL_ASSOC_LT  WL_ASSOC
#elif defined(EVENT_LOG_COMPILE)
#define WL_ASSOC_LT(args)  EVENT_LOG_COMPACT_CAST_PAREN_ARGS(EVENT_LOG_TAG_WL_ASSOC_LOG, args)
#else
#define WL_ASSOC_LT(args)  do {if (wl_msg_level & WL_ASSOC_VAL) WL_PRINT(args);} while (0)
#endif /* WLMSG_ASSOC_LT */

/* even in non-BCMDBG NIC builds, logging of iovars should be available */
#if !defined(DONGLEBUILD)
#define WL_G_IOV_ON()		(wl_msg_level & WL_G_IOV_VAL)
#define WL_S_IOV_ON()		(wl_msg_level2 & WL_S_IOV_VAL)
#else /* For full dongle, DHD prints messages instead of firmware */
#define WL_G_IOV_ON()		0
#define WL_S_IOV_ON()		0
#endif /* DONGLEBUILD */

#endif /* _wl_dbg_h_ */
