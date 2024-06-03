/*
 * Minimal debug/trace/assert driver definitions for
 * Broadcom 802.11 Networking Adapter.
 *
 * Copyright (C) 2023, Broadcom. All Rights Reserved.
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
 * $Id: wl_dbg.h 830093 2023-09-14 03:55:39Z $
 */

#ifndef _wl_dbg_h_
#define _wl_dbg_h_

#if defined(PKTC_TBL) && !defined(BCMDONGLEHOST)
#include <wlc_pub.h>
#include <wlc_dump.h>
#endif

#if defined(EVENT_LOG_COMPILE)
#include <event_log.h>
#endif /* EVENT_LOG_COMPILE */

/* wl_msg_level is a bit vector with defs in wlioctl.h */
extern uint32 wl_msg_level;
extern uint32 wl_msg_level2;
#ifdef WL_EAP_EMSGLVL
/* Enterprise-specific debug trace flags. */
extern uint32 wl_emsg_level;
#endif /* WL_EAP_EMSGLVL */
#define MSG_MACFLTR_MAX		5	/* Limit 5 SCB mac filter */
#if defined(BCMDBG)
extern int wl_msg_macfltr_idx;
#endif /* BCMDBG */

#if !defined(BCMDONGLEHOST) && !defined(BCMDBG_EXCLUDE_HW_TIMESTAMP)
extern char* wlc_dbg_get_hw_timestamp(void);
#define WL_TIMESTAMP() do { if (wl_msg_level2 & WL_TIMESTAMP_VAL) {\
				printf("%s", wlc_dbg_get_hw_timestamp()); }\
				} while (0)
#else
#define WL_TIMESTAMP()
#endif /* !BCMDONGLEHOST && !BCMDBG_EXCLUDE_HW_TIMESTAMP */

#define WL_PRINT(args)		do { WL_TIMESTAMP(); printf args; } while (0)

#define WL_SRSCAN(args)

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

#define WL_IE_ERROR(args)	do {if (wl_msg_level & WL_ERROR_VAL) WL_PRINT(args);} while (0)

#define WL_AMSDU_ERROR(args)	do {if (wl_msg_level & WL_ERROR_VAL) WL_PRINT(args);} while (0)

/* example code to covert current WL_XXX(x) to EVENT_LOG(x)
 * need to find proper EVENT_LOG_TAG_XXXX in event_log_tag.h
 *
 * #if defined(EVENT_LOG_COMPILE) && defined(ERR_USE_EVENT_LOG) && defined(EVENT_LOG_NIC)
 * #if defined(ERR_USE_EVENT_LOG_RA)
 * #define WL_ASSOC_ERROR(args)		do {if (wl_msg_level & WL_ERROR_VAL)	\
 *	EVENT_LOG_RA(EVENT_LOG_TAG_ASSOC_ERROR, args);} while (0)
 * #else
 * #define WL_ASSOC_ERROR(args)		do {if (wl_msg_level & WL_ERROR_VAL)	\
 *	EVENT_LOG_CAST_PAREN_ARGS(EVENT_LOG_TAG_ASSOC_ERROR, args);} while (0)
 * #endif
 * #else
 * #define WL_ASSOC_ERROR(args)	do {if (wl_msg_level & WL_ERROR_VAL) WL_PRINT(args);} while (0)
 * #endif
 */
#define WL_ASSOC_ERROR(args)	do {if (wl_msg_level & WL_ERROR_VAL) WL_PRINT(args);} while (0)

#define WL_SCAN_ERROR(args)	do {if (wl_msg_level & WL_ERROR_VAL) WL_PRINT(args);} while (0)

#define KM_ERR(args)		do {if (wl_msg_level & WL_ERROR_VAL) WL_PRINT(args);} while (0)

#define WL_MBO_ERR(args)	do {if (wl_msg_level & WL_ERROR_VAL) WL_PRINT(args);} while (0)

#define WL_RANDMAC_ERR(args)	do {if (wl_msg_level & WL_ERROR_VAL) WL_PRINT(args);} while (0)

#define	WL_TRACE(args)		do {if (wl_msg_level & WL_TRACE_VAL) WL_PRINT(args);} while (0)

#define WL_PFN_ERROR(args)	do {if (wl_msg_level & WL_ERROR_VAL) WL_PRINT(args);} while (0)

#ifndef LINUX_POSTMOGRIFY_REMOVAL
#define	WL_PRHDRS_MSG(args)	do {if (wl_msg_level & WL_PRHDRS_VAL) WL_PRINT(args);} while (0)
#define WL_PRHDRS(i, p, f, t, wr, r, l)	do { \
						if (wl_msg_level & WL_PRHDRS_VAL) \
							wlc_print_hdrs(i, p, f, t, wr, r, l); \
					} while (0)
#define	WL_PRPKT(m, b, n)	do {if (wl_msg_level & WL_PRPKT_VAL) prhex(m, b, n);} while (0)
#define	WL_INFORM(args)		do {if (wl_msg_level & WL_INFORM_VAL) WL_PRINT(args);} while (0)
#define	WL_TMP(args)		do {if (wl_msg_level & WL_TMP_VAL) WL_PRINT(args);} while (0)
#define	WL_OID(args)		do {if (wl_msg_level & WL_OID_VAL) WL_PRINT(args);} while (0)
#define	WL_RATE(args)		do {if (wl_msg_level & WL_RATE_VAL) WL_PRINT(args);} while (0)
#define WL_ASSOC(args)		do {if (wl_msg_level & WL_ASSOC_VAL) WL_PRINT(args);} while (0)
#define	WL_PRUSR(m, b, n)	do {if (wl_msg_level & WL_PRUSR_VAL) prhex(m, b, n);} while (0)
#define WL_PS(args)		do {if (wl_msg_level & WL_PS_VAL) WL_PRINT(args);} while (0)
/* spare: WL_PORT was here */
#define WL_DUAL(args)		do {if (wl_msg_level & WL_DUAL_VAL) WL_PRINT(args);} while (0)
#define WL_WSEC(args)		do {if (wl_msg_level & WL_WSEC_VAL) WL_PRINT(args);} while (0)
#define WL_WSEC_DUMP(args)	do {if (wl_msg_level & WL_WSEC_DUMP_VAL) WL_PRINT(args);} while (0)
#define WL_REGULATORY(args)	do {if (wl_msg_level & WL_REGULATORY_VAL) WL_PRINT(args);} while (0)
#define WL_ULO(args)		do {if (wl_msg_level & WL_ULMU_VAL) WL_PRINT(args);} while (0)
#define WL_MPC(args)		do {if (wl_msg_level & WL_MPC_VAL) WL_PRINT(args);} while (0)
#define WL_APSTA(args)		do {if (wl_msg_level & WL_APSTA_VAL) WL_PRINT(args);} while (0)
#define WL_DFS(args)		do {if (wl_msg_level & WL_DFS_VAL) WL_PRINT(args);} while (0)
#define WL_MUMIMO(args)		do {if (wl_msg_level & WL_MUMIMO_VAL) WL_PRINT(args);} while (0)
#define WL_MUTX(args)		do {if (wl_msg_level & WL_MUMIMO_VAL) WL_PRINT(args);} while (0)
#define WL_MODE_SWITCH(args) do {if (wl_msg_level & WL_MODE_SWITCH_VAL) WL_PRINT(args);} while (0)
#define WL_BCNTRIM_DBG(args)	do {if (wl_msg_level & WL_BCNTRIM_VAL) WL_PRINT(args);} while (0)
#define WL_MBSS(args)		do {if (wl_msg_level & WL_MBSS_VAL) WL_PRINT(args);} while (0)
#define WL_CAC(args)		do {if (wl_msg_level & WL_CAC_VAL) WL_PRINT(args);} while (0)
#define WL_AMSDU(args)		do {if (wl_msg_level & WL_AMSDU_VAL) WL_PRINT(args);} while (0)
#define WL_AMPDU(args)		do {if (wl_msg_level & WL_AMPDU_VAL) WL_PRINT(args);} while (0)
#define WL_FFPLD(args)		do {if (wl_msg_level & WL_FFPLD_VAL) WL_PRINT(args);} while (0)
#define WL_PFN(args)		do {if (wl_msg_level & WL_PFN_VAL) WL_PRINT(args);} while (0)
#define WL_TWT(args)		do {if (wl_msg_level & WL_TWT_VAL) WL_PRINT(args);} while (0)
/* wl_msg_level is full. Use wl_msg_level_2 now */
#define WL_QOSMGMT_ERR(args)	do {if (wl_msg_level2 & WL_QOSMGMT_VAL) WL_PRINT(args);} while (0)
#define WL_SCAN(args)		do {if (wl_msg_level2 & WL_SCAN_VAL) WL_PRINT(args);} while (0)
#define	WL_CUBBY(args)		do {if (wl_msg_level2 & WL_CUBBY_VAL) WL_PRINT(args);} while (0)
#define	WL_COEX(args)		do {if (wl_msg_level2 & WL_COEX_VAL) WL_PRINT(args);} while (0)
#define WL_RTDC(w,s,i,j)	do {if (wl_msg_level2 & WL_RTDC_VAL) wlc_log(w,s,i,j);} while (0)
#define	WL_PROTO(args)		do {if (wl_msg_level2 & WL_PROTO_VAL) WL_PRINT(args);} while (0)
#define	WL_AIRIQ(args)		do {if (wl_msg_level2 & WL_AIRIQ_VAL) WL_PRINT(args);} while (0)
#define WL_RTDC2(w,s,i,j)	do {if (wl_msg_level2 & 0) wlc_log(w,s,i,j);} while (0)
#define WL_CHANINT(args)	do {if (wl_msg_level2 & WL_CHANINT_VAL) WL_PRINT(args);} while (0)
#define WL_WMF(args)		do {if (wl_msg_level2 & WL_WMF_VAL) WL_PRINT(args);} while (0)
#define WL_ITFR(args)		do {if (wl_msg_level2 & WL_ITFR_VAL) WL_PRINT(args);} while (0)
#define WL_PROT(args)		do {if (wl_msg_level2 & WL_PROT_VAL) WL_PRINT(args);} while (0)
#define WL_TBTT(args)		do {if (wl_msg_level2 & WL_TBTT_VAL) WL_PRINT(args);} while (0)
#define WL_RRM(args)		do {if (wl_msg_level2 & WL_RRM_VAL) WL_PRINT(args);} while (0)
#define WL_RRM_HEX(m, b, n)	do {if (wl_msg_level2 & WL_RRM_VAL) prhex(m, b, n);} while (0)
#define WL_TRF_MGMT(args)	do {if (wl_msg_level2 & WL_TRF_MGMT_VAL) WL_PRINT(args);} while (0)
#define WL_MAC(args)		do {if (wl_msg_level2 & WL_MAC_VAL) WL_PRINT(args);} while (0)
#define WL_L2FILTER(args)	do {if (wl_msg_level2 & WL_L2FILTER_VAL) WL_PRINT(args);} while (0)
#define WL_TSO(args)		do {if (wl_msg_level2 & WL_TSO_VAL) WL_PRINT(args);} while (0)
#define WL_MQ(args)		do {if (wl_msg_level2 & WL_MQ_VAL) WL_PRINT(args);} while (0)
#define WL_P2PO(args)		do {if (wl_msg_level2 & WL_P2PO_VAL) WL_PRINT(args);} while (0)
#define WL_WNM(args)		do {if (wl_msg_level2 & WL_WNM_VAL) WL_PRINT(args);} while (0)
#define WL_TXBF(args)		do {if (wl_msg_level2 & WL_TXBF_VAL) WL_PRINT(args);} while (0)
#define WL_PCIE(args)		do {if (wl_msg_level2 & WL_PCIE_VAL) WL_PRINT(args);} while (0)
#define WL_MLO_PRINT(args)	do {if (wl_msg_level2 & WL_MLO_VAL) WL_PRINT(args);} while (0)
#define WL_PRMLO(m, b, n)	do {if (wl_msg_level2 & WL_PRMLO_VAL) prhex(m, b, n);} while (0)
#define WL_PUQ_PRINT(args)	do {if (wl_msg_level2 & WL_PUQ_VAL) WL_PRINT(args);} while (0)

#define WL_TSLOG(w, s, i, j)	\
	do {if (wl_msg_level2 & WL_TIMESTAMP_VAL) wlc_log(w, s, i, j);} while (0)
#define WL_ROAM(args)
#define WL_PRMAC(args)		do {if (wl_msg_level & WL_PRMAC_VAL) WL_PRINT(args);} while (0)
#define WL_FBT(args)		do {if (wl_msg_level2 & WL_FBT_VAL) WL_PRINT(args);} while (0)
#define WL_MBO_DBG(args)	do {if (wl_msg_level2 & WL_MBO_VAL) WL_PRINT(args);} while (0)
#define WL_RANDMAC_DBG(args)	do {if (wl_msg_level2 & WL_RANDMAC_VAL) WL_PRINT(args);} while (0)
#define WL_RANDMAC_INFO(args)	do {if (wl_msg_level2 & WL_RANDMAC_VAL) WL_PRINT(args);} while (0)
#define WL_BAM_ERR(args)	do {if (wl_msg_level2 & WL_ERROR_VAL)	WL_PRINT(args);} while (0)
#define WL_OCE_DBG(args)	do {if (wl_msg_level2 & WL_OCE_VAL) WL_PRINT(args);} while (0)
#define WL_OCE_ERR(args)	do {if (wl_msg_level2 & WL_OCE_VAL) WL_PRINT(args);} while (0)
#define WL_OCE_INFO(args)	do {if (wl_msg_level2 & WL_OCE_VAL) WL_PRINT(args);} while (0)
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
#endif
#define WL_MBSS_ON()		(wl_msg_level & WL_MBSS_VAL)
#define WL_AMPDU_ON()		(wl_msg_level & WL_AMPDU_VAL)
#define WL_PFN_ON()		(wl_msg_level & WL_PFN_VAL)
#define WL_TWT_ON()		(wl_msg_level & WL_TWT_VAL)
#define	WL_AIRIQ_ON()		(wl_msg_level2 & WL_AIRIQ_VAL)
#define WL_SCAN_ON()		(wl_msg_level2 & WL_SCAN_VAL)
#define WL_WMF_ON()		(wl_msg_level2 & WL_WMF_VAL)
#define WL_ITFR_ON()		(wl_msg_level2 & WL_ITFR_VAL)
#define WL_PROT_ON()		(wl_msg_level2 & WL_PROT_VAL)
#define WL_TBTT_ON()		(wl_msg_level2 & WL_TBTT_VAL)
#define WL_TRF_MGMT_ON()	(wl_msg_level2 & WL_TRF_MGMT)
#define WL_MAC_ON()		(wl_msg_level2 & WL_MAC_VAL)
#define WL_PWRSEL_ON()		(wl_msg_level2 & WL_PWRSEL_VAL)
#define WL_L2FILTER_ON()	(wl_msg_level2 & WL_L2FILTER_VAL)
#define WL_MQ_ON()		(wl_msg_level2 & WL_MQ_VAL)
#define WL_P2PO_ON()		(wl_msg_level2 & WL_P2PO_VAL)
#define WL_WNM_ON()		(wl_msg_level2 & WL_WNM_VAL)
#define WL_TXBF_ON()		(wl_msg_level2 & WL_TXBF_VAL)
#define WL_PCIE_ON()		(wl_msg_level2 & WL_PCIE_VAL)
#define WL_TSLOG_ON()		(wl_msg_level2 & WL_TIMESTAMP_VAL)
#define WL_MBO_DBG_ON()		(wl_msg_level2 & WL_MBO_VAL)
#define WL_RANDMAC_DBG_ON()	(wl_msg_level2 & WL_RANDMAC_VAL)
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
#define	WL_APSTA_WME_VAL	0x00000040 /* WME dyn */

extern uint32 wl_apsta_dbg;

#define WL_APSTA_UPDN(args) do {if (wl_apsta_dbg & WL_APSTA_UPDN_VAL) {WL_APSTA(args);}} while (0)
#define WL_APSTA_BCN(args) do {if (wl_apsta_dbg & WL_APSTA_BCN_VAL) {WL_APSTA(args);}} while (0)
#define WL_APSTA_TX(args) do {if (wl_apsta_dbg & WL_APSTA_TX_VAL) {WL_APSTA(args);}} while (0)
#define WL_APSTA_RX(args) do {if (wl_apsta_dbg & WL_APSTA_RX_VAL) {WL_APSTA(args);}} while (0)
#define WL_APSTA_TSF(args) do {if (wl_apsta_dbg & WL_APSTA_TSF_VAL) {WL_APSTA(args);}} while (0)
#define WL_APSTA_BSSID(args) do {if (wl_apsta_dbg & WL_APSTA_BSSID_VAL) {WL_APSTA(args);}} while (0)
#define WL_APSTA_WME(args) do {if (wl_apsta_dbg & WL_APSTA_WME_VAL) {WL_APSTA(args);}} while (0)
#define WL_APSTA_RX_ON() (wl_apsta_dbg & WL_APSTA_RX_VAL)

/* Extra message control for AMPDU debugging */
#define WL_AMPDU_UPDN_VAL	0x00000001 /* Config up/down related  */
#define WL_AMPDU_ERR_VAL	0x00000002 /* Calls to beaocn update  */
#define WL_AMPDU_TX_VAL		0x00000004 /* Transmit data path */
#define WL_AMPDU_RX_VAL		0x00000008 /* Receive data path  */
#define WL_AMPDU_CTL_VAL	0x00000010 /* TSF-related items  */
#define WL_AMPDU_HW_VAL		0x00000020 /* AMPDU_HW */
#define WL_AMPDU_HWTXS_VAL	0x00000040 /* AMPDU_HWTXS */
#define WL_AMPDU_HWDBG_VAL	0x00000080 /* AMPDU_DBG */
#define WL_AMPDU_STAT_VAL	0x00000100 /* statistics */

extern uint32 wl_ampdu_dbg;

#define WL_AMPDU_UPDN(args) do {if (wl_ampdu_dbg & WL_AMPDU_UPDN_VAL) {WL_AMPDU(args);}} while (0)
#define WL_AMPDU_ERR(args) do {if (wl_ampdu_dbg & WL_AMPDU_ERR_VAL) {WL_AMPDU(args);}} while (0)
#define WL_AMPDU_TX(args) do {if (wl_ampdu_dbg & WL_AMPDU_TX_VAL) {WL_AMPDU(args);}} while (0)
#define WL_AMPDU_CTL(args) do {if (wl_ampdu_dbg & WL_AMPDU_CTL_VAL) {WL_AMPDU(args);}} while (0)
#define WL_AMPDU_HW(args) do {if (wl_ampdu_dbg & WL_AMPDU_HW_VAL) {WL_AMPDU(args);}} while (0)
#define WL_AMPDU_HWTXS(args) do {if (wl_ampdu_dbg & WL_AMPDU_HWTXS_VAL) {WL_AMPDU(args);}} while (0)
#define WL_AMPDU_HWDBG(args) do {if (wl_ampdu_dbg & WL_AMPDU_HWDBG_VAL) {WL_AMPDU(args);}} while (0)
#define WL_AMPDU_ERR_ON() (wl_ampdu_dbg & WL_AMPDU_ERR_VAL)
#define WL_AMPDU_HW_ON() (wl_ampdu_dbg & WL_AMPDU_HW_VAL)
#define WL_AMPDU_HWTXS_ON() (wl_ampdu_dbg & WL_AMPDU_HWTXS_VAL)
#define WL_AMPDU_TX_ON() (wl_ampdu_dbg & WL_AMPDU_TX_VAL)

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
#define WL_ERROR(args)		do {if (wl_msg_level & WL_ERROR_VAL) WL_PRINT(args);} while (0)
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
#define WL_ASSOC(args)		do {if (wl_msg_level & WL_ASSOC_VAL) WL_PRINT(args);} while (0)
#define WL_PRUSR(m, b, n)
#define WL_PS(args)		do {if (wl_msg_level & WL_PS_VAL) WL_PRINT(args);} while (0)
//#define WL_PORT(args)
#define WL_DUAL(args)
#define WL_REGULATORY(args)	do {if (wl_msg_level & WL_REGULATORY_VAL) WL_PRINT(args);} while (0)

#define WL_MPC(args)
#define WL_APSTA(args)
#define WL_APSTA_BCN(args)
#define WL_APSTA_TX(args)
#define WL_APSTA_TSF(args)
#define WL_APSTA_BSSID(args)
#define WL_APSTA_WME(args)
#define WL_MBSS(args)
#define WL_MODE_SWITCH(args)
#define WL_PROTO(args)

#define	WL_CAC(args)		do {if (wl_msg_level & WL_CAC_VAL) WL_PRINT(args);} while (0)
#define WL_AMSDU(args)
#define WL_AMPDU(args)
#define WL_FFPLD(args)
#define WL_TWT(args)

#define WL_DFS(args)
#define WL_ASSOC_OR_DPT(args)
#define WL_QOSMGMT_ERR(args)	do {if (wl_msg_level2 & WL_QOSMGMT_VAL) WL_PRINT(args);} while (0)
#define WL_SCAN(args)		do {if (wl_msg_level2 & WL_SCAN_VAL) WL_PRINT(args);} while (0)
#define WL_CUBBY(args)
#define WL_COEX(args)
#define WL_RTDC(w, s, i, j)
#define WL_RTDC2(w, s, i, j)
#define WL_CHANINT(args)
#define WL_BTA(args)
#define WL_ITFR(args)
#define WL_PROT(args)
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
#define WL_ULO(args)
#define WL_MLO_PRINT(args)
#define WL_PRMLO(m, b, n)
#define WL_PUQ_PRINT(args)	do {if (wl_msg_level2 & WL_PUQ_VAL) WL_PRINT(args);} while (0)

#ifdef BCMDBG_ERR
#define WL_PFN_ERROR(args)	WL_PRINT(args)
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
#define WL_AMPDU_ERR_ON()	0
#define WL_AMPDU_HW_ON()	0
#define WL_AMPDU_HWTXS_ON()	0
#define WL_AMPDU_TX_ON()	0

#define WL_APSTA_UPDN(args)
#define WL_APSTA_RX(args)
#define WL_APSTA_RX_ON()	0
#define WL_WSEC(args)
#define WL_WSEC_DUMP(args)
#define WL_PCIE(args)
#define WL_MLO(args)
#define	WL_AIRIQ(args)
#define WL_TSLOG(w, s, i, j)
#define WL_FBT(args)
#define WL_MBO_DBG(args)
#define WL_RANDMAC_DBG(args)
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
#define WL_SCAN_ON()		(wl_msg_level2 & WL_SCAN_VAL)
#define WL_BTA_ON()		0
#define WL_ITFR_ON()		0
#define WL_PROT_ON()		0
#define WL_TRF_MGMT_ON()	0
#define WL_MAC_ON()		0
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
#define WL_RANDMAC_DBG_ON()	0
#define WL_ADPS_ON()            0
#define WL_OCE_DBG_ON()		0

#else /* !BCMDBG && !BCMCONDITIONAL_LOGGING */

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
#define WL_ERROR(args)		WL_PRINT(args)
#else
#define	WL_ERROR(args)
#endif /* BCMDBG_ERR */

#ifdef BCMDBG_ERR
/* ROM and ROML optimized builds */
#define  KM_ERR(args)		WL_PRINT(args)
#else
#define  KM_ERR(args)
#endif /* BCMDBG_ERR */

#ifdef BCMDBG_ERR
/* ROM and ROML optimized builds */
#define WL_AMPDU_ERR(args)	WL_PRINT(args)
#else
#define	WL_AMPDU_ERR(args)
#endif /* BCMDBG_ERR */

#define	WL_TRACE(args)
#ifndef LINUX_POSTMOGRIFY_REMOVAL
#ifdef WLMSG_PRHDRS
#define	WL_PRHDRS_MSG(args)		WL_PRINT(args)
#define WL_PRHDRS(i, p, f, t, wr, r, l)	wlc_print_hdrs(i, p, f, t, wr, r, l)
#else
#define	WL_PRHDRS_MSG(args)
#define	WL_PRHDRS(i, p, f, t, wr, r, l)
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

#ifdef BCMDBG_ERR
#define   WL_IE_ERROR(args)	WL_PRINT(args)
#else
#define	  WL_IE_ERROR(args)
#endif /* BCMDBG_ERR */

#ifdef WLMSG_ASSOC
#define WL_ASSOC(args)		WL_PRINT(args)
#else
#define	WL_ASSOC(args)
#endif /* WLMSG_ASSOC */

#ifdef BCMDBG_ERR
/* example code for dongle event log
 * to move any other module definitions, add entry here and delete the subsequent definition
 * #if defined(EVENT_LOG_COMPILE) && defined(ERR_USE_EVENT_LOG)
 * #if defined(ERR_USE_EVENT_LOG_RA)
 * #define WL_ASSOC_ERROR(args)       EVENT_LOG_RA(EVENT_LOG_TAG_ASSOC_ERROR, args)
 * #define WL_SCAN_ERROR(args)      EVENT_LOG_RA(EVENT_LOG_TAG_SCAN_ERR, args)
 * #define WL_AMSDU_ERROR(args)       EVENT_LOG_RA(EVENT_LOG_TAG_AMSDU_ERROR, args)
 * #else
 * #define WL_ASSOC_ERROR(args)  EVENT_LOG_COMPACT_CAST_PAREN_ARGS(EVENT_LOG_TAG_ASSOC_ERROR, args)
 * #define WL_SCAN_ERROR(args)   EVENT_LOG_COMPACT_CAST_PAREN_ARGS(EVENT_LOG_TAG_SCAN_ERR, args)
 * #define WL_AMSDU_ERROR(args)  EVENT_LOG_COMPACT_CAST_PAREN_ARGS(EVENT_LOG_TAG_AMSDU_ERROR, args)
 * #endif
 * #else
 * #define  WL_ASSOC_ERROR(args)      WL_PRINT(args)
 * #define  WL_SCAN_ERROR(args)    WL_PRINT(args)
 * #define   WL_AMSDU_ERROR(args)          WL_PRINT(args)
 * #endif
 */
/* ROM and ROML optimized builds */
#define  WL_ASSOC_ERROR(args)      WL_PRINT(args)
#define  WL_SCAN_ERROR(args)    WL_PRINT(args)
#define   WL_AMSDU_ERROR(args)          WL_PRINT(args)
#else
#define        WL_ASSOC_ERROR(args)
#define        WL_SCAN_ERROR(args)
#define       WL_AMSDU_ERROR(args)
#endif /* BCMDBG_ERR */

#define	WL_PRUSR(m, b, n)

#ifdef WLMSG_PS
#define WL_PS(args)		WL_PRINT(args)
#else
#define WL_PS(args)
#endif /* WLMSG_PS */

#ifdef WLMSG_ROAM
#define WL_ROAM(args)		WL_PRINT(args)
#else
#define WL_ROAM(args)
#endif /* WLMSG_ROAM */

//#define WL_PORT(args)
#define WL_DUAL(args)
#define WL_REGULATORY(args)
#define WL_TAF(args)

#ifdef WLMSG_MPC
#define WL_MPC(args)		WL_PRINT(args)
#else
#define WL_MPC(args)
#endif /* WLMSG_MPC */

#define WL_APSTA(args)
#define WL_APSTA_BCN(args)
#define WL_APSTA_TX(args)
#define WL_APSTA_TSF(args)
#define WL_APSTA_BSSID(args)
#define WL_APSTA_WME(args)
#define WL_MBSS(args)
#define WL_MODE_SWITCH(args)
#define	WL_PROTO(args)

#define	WL_CAC(args)
#define WL_AMSDU(args)
#define WL_AMPDU(args)
#define WL_FFPLD(args)
#define WL_TWT(args)
#define WL_BCNTRIM_DBG(args)

/* Define WLMSG_DFS automatically for WLTEST builds */
#if defined(WLTEST)
#ifndef WLMSG_DFS
#define WLMSG_DFS
#endif
#endif /* WLTEST */

#ifdef WLMSG_DFS
#define WL_DFS(args)		do {if (wl_msg_level & WL_DFS_VAL) WL_PRINT(args);} while (0)
#else /* WLMSG_DFS */
#define WL_DFS(args)
#endif /* WLMSG_DFS */

#define WL_QOSMGMT_ERR(args)

#ifdef WLMSG_SCAN
#define WL_SCAN(args)		WL_PRINT(args)
#else
#define WL_SCAN(args)
#endif /* WLMSG_SCAN */

#define	WL_CUBBY(args)
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
#define WL_ITFR(args)

#define WL_PROT(args)
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
#define WL_ADPS(args)
#define WL_ULO(args)
#define WL_MLO_PRINT(args)
#define WL_PRMLO(m, b, n)
#define WL_PUQ_PRINT(args)	do {if (wl_msg_level2 & WL_PUQ_VAL) WL_PRINT(args);} while (0)

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
#define WL_SCAN_ON()		1
#else
#define WL_SCAN_ON()		0
#endif

#ifdef WLMSG_BTA
#define WL_BTA_ON()		1
#else
#define WL_BTA_ON()		0
#endif

#define WL_WMF_ON()		0
#define WL_PROT_ON()		0
#define WL_TBTT_ON()		0
#define WL_PWRSEL_ON()		0
#define WL_L2FILTER_ON()	0
#define WL_MAC_ON()		0
#define WL_MQ_ON()		0
#define WL_P2PO_ON()		0
#define WL_TXBF_ON()		0
#define WL_TSLOG_ON()		0
#define WL_MUMIMO_ON()		0
#define WL_MUTX_ON()		0
#define WL_ULMU_ON()		0

#define WL_AMPDU_UPDN(args)
#define WL_AMPDU_RX(args)
#define WL_AMPDU_TX(args)
#define WL_AMPDU_CTL(args)
#define WL_AMPDU_HW(args)
#define WL_AMPDU_HWTXS(args)
#define WL_AMPDU_HWDBG(args)
#define WL_AMPDU_STAT(args)
#define WL_AMPDU_ERR_ON()	0
#define WL_AMPDU_HW_ON()	0
#define WL_AMPDU_HWTXS_ON()	0
#define WL_AMPDU_TX_ON()	0

#define WL_WNM_ON()		0

#ifdef WLMSG_MBO
#define WL_MBO_DBG_ON()		1
#else
#define WL_MBO_DBG_ON()		0
#endif /* WLMSG_MBO */
#ifdef WLMSG_RANDMAC
#define WL_RANDMAC_DBG_ON()        1
#else
#define WL_RANDMAC_DBG_ON()        0
#endif /* WLMSG_RANDMAC */
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
#define WL_WSEC(args)		WL_PRINT(args)
#define WL_WSEC_DUMP(args)	WL_PRINT(args)
#else
#define WL_WSEC(args)
#define WL_WSEC_DUMP(args)
#endif /* WLMSG_WSEC */

#ifdef WLMSG_MBO
#define WL_MBO_DBG(args)	WL_PRINT(args)
#define WL_MBO_INFO(args)	WL_PRINT(args)
#else
#define	WL_MBO_DBG(args)
#define	WL_MBO_INFO(args)
#endif /* WLMSG_MBO */

#ifdef BCMDBG_ERR
#define WL_MBO_ERR(args)	WL_PRINT(args)
#else
#define WL_MBO_ERR(args)
#endif /* BCMDBG_ERR */

#ifdef WLMSG_RANDMAC
#if defined(EVENT_LOG_COMPILE) && defined(ERR_USE_EVENT_LOG)
#if defined(USE_EVENT_LOG_RA)
#define   WL_RANDMAC_DBG(args)              EVENT_LOG_RA(EVENT_LOG_TAG_RANDMAC_DBG, args)
#define   WL_RANDMAC_INFO(args)     EVENT_LOG_RA(EVENT_LOG_TAG_RANDMAC_INFO, args)
#else
#define   WL_RANDMAC_DBG(args)       \
			EVENT_LOG_COMPACT_CAST_PAREN_ARGS(EVENT_LOG_TAG_RANDMAC_DBG, args)
#define   WL_RANDMAC_INFO(args)      \
			EVENT_LOG_COMPACT_CAST_PAREN_ARGS(EVENT_LOG_TAG_RANDMAC_INFO, args)
#endif /* USE_EVENT_LOG_RA */
#else
#define   WL_RANDMAC_DBG(args)                 WL_PRINT(args)
#define   WL_RANDMAC_INFO(args)             WL_PRINT(args)
#endif /* EVENT_LOG_COMPILE && ERR_USE_EVENT_LOG */
#else
#define   WL_RANDMAC_DBG(args)
#define   WL_RANDMAC_INFO(args)
#endif /* WLMSG_RANDMAC */

#ifdef BCMDBG_ERR
#if defined(EVENT_LOG_COMPILE) && defined(ERR_USE_EVENT_LOG)
#if defined(ERR_USE_EVENT_LOG_RA)
#define   WL_RANDMAC_ERR(args)              EVENT_LOG_RA(EVENT_LOG_TAG_RANDMAC_ERR, args)
#else
#define   WL_RANDMAC_ERR(args)     \
		EVENT_LOG_COMPACT_CAST_PAREN_ARGS(EVENT_LOG_TAG_RANDMAC_ERR, args)
#endif /* ERR_USE_EVENT_LOG_RA */
#else
#define   WL_RANDMAC_ERR(args)              WL_PRINT(args)
#endif /* EVENT_LOG_COMPILE && ERR_USE_EVENT_LOG */
#else
#define   WL_RANDMAC_ERR(args)              WL_PRINT(args)
#endif /* ERR_USE_EVENT_LOG */

#ifdef WLMSG_OCE
#define WL_OCE_DBG(args)	WL_PRINT(args)
#define WL_OCE_INFO(args)	WL_PRINT(args)
#else
#define	WL_OCE_DBG(args)
#define	WL_OCE_INFO(args)
#endif /* WLMSG_OCE */

#ifdef BCMDBG_ERR
#define WL_OCE_ERR(args)	WL_PRINT(args)
#else
#define WL_OCE_ERR(args)
#endif /* BCMDBG_ERR */

#define WL_PCIE(args)		do {if (wl_msg_level2 & WL_PCIE_VAL) WL_PRINT(args);} while (0)
#define WL_PCIE_ON()		(wl_msg_level2 & WL_PCIE_VAL)
#define WL_PFN(args)		do {if (wl_msg_level & WL_PFN_VAL) WL_PRINT(args);} while (0)
#define WL_PFN_ON()		(wl_msg_level & WL_PFN_VAL)
#endif /* BCMDBG */

/**
 * Msg MacFilter related MACROs
 * Below the options for calling WL_MSG_FILTER_EX:
 * 1) both scb and func_str pointers are set:
 * 'wl0.0 00:11:22:33:44:55: function_name: ', macfilter in case of BCMDBG.
 * 2) only func_str pointer is set:
 * 'function_name: ', no macfilter.
 * 3) only scb pointer is set:
 * no prefix, but there is a macfilter in case of BCMDBG.
 * 4) both scb and func_str pointer are not set:
 * no prefix and no macfilter.
 */
#define WL_MSG_PRINT_PFX(scb, func_str) \
	do { \
		if (scb && func_str) { \
			WL_PRINT(("%s ", SCB_GENERIC_SEC_CUBBY(scb)->scb_msg_pfx_string)); \
		} \
		if (func_str) { \
			printf("%s: ", func_str); \
		} \
	} while (0)

#ifdef BCMDBG
/* When the macfilter is disabled, i.e., no MAC address is added to macfilter table,
 * WL_MSG_MACFILTER0((...)) - display messages
 * WL_MSG_FILTER_EX(scb, (...)) - display messages
 *
 * When the macfilter is enabled, i.e., at least one MAC address is added to macfilter table,
 * WL_MSG_MACFILTER0((...)) - don't display messages
 * WL_MSG_FILTER_EX(scb, (...)) - display messages when Mac filter matches
 *
 */
#define WL_MSG_MACFILTER0(msglvl, args)	\
	do { \
		if ((wl_msg_level & msglvl) && (wl_msg_macfltr_idx == 0)) \
			WL_PRINT(args); \
	} while (0)

#define WL_MSG_FILTER_EX(msglvl, scb, args, func_str) \
	do { \
		if (msg_mac_filter_ex(msglvl, scb, func_str)) { \
			printf args; \
		} \
	} while (0)
#elif defined(BCMCONDITIONAL_LOGGING)
#define WL_MSG_FILTER_EX(msglvl, scb, args, func_str) \
	do { \
		if (wl_msg_level & msglvl) { \
			msg_pfx(scb, func_str); \
			printf args; \
		} \
	} while (0)
#else /* BCMDBG */
#define WL_MSG_FILTER_EX(msglvl, scb, args, func_str) \
	do { \
		msg_pfx(scb, func_str); \
		printf args; \
	} while (0)
#endif /* BCMDBG */

#if defined(BCMDBG) || defined(WLMSG_PS) || defined(BCMCONDITIONAL_LOGGING)
#define WL_PS_PFX(scb, string, args...)	WL_MSG_FILTER_EX(WL_PS_VAL, (scb), \
	(string, ##args), __FUNCTION__)
#else
#define WL_PS_PFX(scb, string, args...)
#endif /* WLMSG_PS */

#ifdef BCMDBG
#define WL_PS0(args)		WL_MSG_MACFILTER0(WL_PS_VAL, args)

#define WL_AMSDU_PFX(scb, string, args...)	WL_MSG_FILTER_EX(WL_AMSDU_VAL, (scb), \
		(string, ##args), __FUNCTION__)
#define WL_AMSDU_NPFX(scb, string, args...) \
	WL_MSG_FILTER_EX(WL_AMSDU_VAL, (scb), (string, ##args), NULL)

#define WL_AMPDU0(args)		WL_MSG_MACFILTER0(WL_AMPDU_VAL, args)
#define WL_AMPDU_PFX(scb, string, args...)	WL_MSG_FILTER_EX(WL_AMPDU_VAL, (scb), \
		(string, ##args), __FUNCTION__)
#define WL_AMPDU_NPFX(scb, string, args...) WL_MSG_FILTER_EX(WL_AMPDU_VAL, (scb), \
		(string, ##args), NULL)
#define WL_AMPDU_DBG_FILTER(dbg_val, scb, args) \
	do { \
		if (wl_ampdu_dbg & dbg_val) { \
			WL_MSG_FILTER_EX(WL_AMPDU_VAL, (scb), args, __FUNCTION__); \
		} \
	} while (0)

#define WL_AMPDU_ERR_PFX(scb, string, args...) \
	WL_AMPDU_DBG_FILTER(WL_AMPDU_ERR_VAL, scb, (string, ##args))
#define WL_AMPDU_UPDN_PFX(scb, string, args...) \
	WL_AMPDU_DBG_FILTER(WL_AMPDU_UPDN_VAL, scb, (string, ##args))
#define WL_AMPDU_RX_PFX(scb, string, args...) \
	WL_AMPDU_DBG_FILTER(WL_AMPDU_RX_VAL, scb, (string, ##args))
#define WL_AMPDU_TX_PFX(scb, string, args...) \
	WL_AMPDU_DBG_FILTER(WL_AMPDU_TX_VAL, scb, (string, ##args))
#define WL_AMPDU_CTL_PFX(scb, string, args...) \
	WL_AMPDU_DBG_FILTER(WL_AMPDU_CTL_VAL, scb, (string, ##args))
#define WL_AMPDU_STAT_PFX(scb, string, args...) \
	WL_AMPDU_DBG_FILTER(WL_AMPDU_STAT_VAL, scb, (string, ##args))
#else /* BCMDBG */
#define WL_PS0(args)		WL_PS(args)

#define WL_AMSDU_PFX(scb, string, args...)
#define WL_AMSDU_NPFX(scb, string, args...)

#define WL_AMPDU0(args)		WL_AMPDU(args)
#define WL_AMPDU_PFX(scb, string, args...)
#define WL_AMPDU_NPFX(scb, string, args...)
#if defined(BCMDBG_ERR) && !defined(BCMCONDITIONAL_LOGGING)
#define WL_AMPDU_ERR_PFX(scb, string, args...) \
	WL_MSG_FILTER_EX(WL_AMPDU_VAL, (scb), (string, ##args), __FUNCTION__);
#else
#define WL_AMPDU_ERR_PFX(scb, string, args...)
#endif /* BCMDBG_ERR */
#define WL_AMPDU_UPDN_PFX(scb, string, args...)
#define WL_AMPDU_RX_PFX(scb, string, args...)
#define WL_AMPDU_TX_PFX(scb, string, args...)
#define WL_AMPDU_CTL_PFX(scb, string, args...)
#define WL_AMPDU_STAT_PFX(scb, string, args...)
#endif /* BCMDBG */

#ifdef WL_EAP_EMSGLVL
/* Always enable the WL_EAP traces regardless of whether BCMDBG is enabled. */
#define WL_EAP_TRC_ERROR(args)		\
	do {if (wl_emsg_level & WL_EAP_ERROR_VAL) printf args;} while (0)
#define WL_EAP_TRC_INFO(args)		\
	do {if (wl_emsg_level & WL_EAP_INFO_VAL) printf args;} while (0)
#define WL_EAP_TRC_SCAN(args)		\
	do {if (wl_emsg_level & WL_EAP_SCAN_VAL) printf args;} while (0)
#define WL_EAP_TRC_SCAN_DBG(args)	\
	do {if (wl_emsg_level & WL_EAP_SCAN_DBG_VAL) printf args;} while (0)
#define WL_EAP_TRC_INTR(args)		\
	do {if (wl_emsg_level & WL_EAP_INTR_VAL) printf args;} while (0)
#define WL_EAP_TRC_RM(args)		\
	do {if (wl_emsg_level & WL_EAP_RM_VAL) printf args;} while (0)
#define WL_EAP_TRC_AMPDU(args)		\
	do {if (wl_emsg_level & WL_EAP_AMPDU_VAL) printf args;} while (0)
#define WL_EAP_TRC_AMSDU(args)		\
	do {if (wl_emsg_level & WL_EAP_AMSDU_VAL) printf args;} while (0)
#define WL_EAP_TRC_RXTXUTIL_MSG(args)	\
	do {if (wl_emsg_level & WL_EAP_RXTXUTIL_VAL) printf args;} while (0)
#define WL_EAP_TRC_DTIM(args)		\
	do {if (wl_emsg_level & WL_EAP_DTIM_VAL) printf args;} while (0)
#define WL_EAP_TRC_EVENT(args)		\
	do {if (wl_emsg_level & WL_EAP_EV_EAP_VAL) printf args;} while (0)
#define WL_EAP_TRC_AP_80211RAW_MSG(args) \
	do {if (wl_emsg_level & WL_EAP_80211RAW_VAL) WL_PRINT(args);} while (0)
#define WL_EAP_TRC_PRHDRS_RAW(i, p, f, t, wr, r, l)	\
	do { \
		if (wl_emsg_level & WL_EAP_80211RAW_VAL) \
			wlc_print_hdrs(i, p, f, t, wr, r, l); \
	} while (0)
#define WL_EAP_TRC_PRPKT_RAW(m, b, n)			\
	do {if (wl_emsg_level & WL_EAP_80211RAW_VAL) prhex(m, b, n);} while (0)
#define WL_EAP_TRC_80211RAW_ON()    (wl_emsg_level & WL_EAP_80211RAW_VAL)
#define WL_EAP_TRC_PCAP(args) \
	do {if (wl_emsg_level & WL_EAP_PCAP_VAL) WL_PRINT(args);} while (0)
#else
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
#define WL_EAP_TRC_PCAP(args)
#endif /* WL_EAP_EMSGLVL */

#ifdef WLMSG_BAM
#define WL_BAM_ERR(args)	WL_PRINT(args)
#endif /* WLMSG_BAM */

#if defined(BCMDBG) || defined(BCMDBG_ERR)
#define DBGERRONLY(x) x
#else
#define DBGERRONLY(x)
#endif

extern uint32 wl_msg_level;
extern uint32 wl_msg_level2;
#ifdef BCM_IOCV_MEM_LOG
extern uint32 wl_log_level;
extern uint32 wl_log_level2;
#endif /* BCM_IOCV_MEM_LOG */

/* Customers ask for more visibility when dealing with association problems, even in 'production'
 * firmware builds. Enabling WLMSG_ASSOC consumes approx 42KB of 4366c0 dongle RAM. As a lightweight
 * alternative, WLMSG_ASSOC_LT consumes 1KB on 4366c0. It does not generate all messages that its
 * full-blown sibling features, but the essential ones.
 */
#ifndef WLMSG_ASSOC_LT
#define WL_ASSOC_LT  WL_ASSOC
#else
#define WL_ASSOC_LT(args)  do {if (wl_msg_level & WL_ASSOC_VAL) WL_PRINT(args);} while (0)
#endif /* WLMSG_ASSOC_LT */

/* even in non-BCMDBG NIC builds, logging of iovars should be available */
#ifdef BCM_IOCV_MEM_LOG
/* Log to console */
#define WL_G_IOV_ON()		(wl_msg_level & WL_G_IOV_VAL)
#define WL_S_IOV_ON()		(wl_msg_level2 & WL_S_IOV_VAL)
/* Log to memory */
#define WL_G_IOV_MEM_LOG_ON()		(wl_log_level & WL_G_IOV_VAL)
#define WL_S_IOV_MEM_LOG_ON()		(wl_log_level2 & WL_S_IOV_VAL)
#else /* For full dongle, DHD prints messages instead of firmware */
#define WL_G_IOV_ON()		0
#define WL_S_IOV_ON()		0
#define WL_G_IOV_MEM_LOG_ON()		0
#define WL_S_IOV_MEM_LOG_ON()		0
#endif /* BCM_IOCV_MEM_LOG */

#define WL_DEBUG_EXPR(condition, exec)	if (condition) {exec;}

/* WL_MSG_LEVEL_MASK / WL_MSG_LEVEL_MASK2 #defines indicate which messages are supported by the
 * current build.
 * WL_MSG_LEVEL_ALWAYS_ON / WL_MSG_LEVEL2_ALWAYS_ON #defines indicate which messages are always
 * enabled, and cannot be disabled by the user.
 */
#if defined(BCMDBG)

#ifdef LINUX_POSTMOGRIFY_REMOVAL
#define WL_MSG_LEVEL_MASK  (WL_ERROR_VAL | WL_TRACE_VAL)
#define WL_MSG_LEVEL2_MASK
#else
#define WL_MSG_LEVEL_MASK  0xFFFFFFFF
#define WL_MSG_LEVEL2_MASK 0xFFFFFFFF
#endif /* LINUX_POSTMOGRIFY_REMOVAL */
#define WL_MSG_LEVEL_ALWAYS_ON 0
#define WL_MSG_LEVEL2_ALWAYS_ON 0

#elif defined(BCMCONDITIONAL_LOGGING)

#define WL_MSG_LEVEL_MASK  (WL_ERROR_VAL | WL_RATE_VAL | WL_ASSOC_VAL | WL_PS_VAL | \
			    WL_REGULATORY_VAL | WL_CAC_VAL)
#define WL_MSG_LEVEL2_MASK (WL_SCAN_VAL)
#define WL_MSG_LEVEL_ALWAYS_ON 0
#define WL_MSG_LEVEL2_ALWAYS_ON 0

#else /* !BCMDBG && !BCMCONDITIONAL_LOGGING */

#if defined(BCMDBG_ERR)
#define _WL_MSG_LEVEL_MASK  (WL_ERROR_VAL | WL_PFN_VAL)
#else
#define _WL_MSG_LEVEL_MASK  (WL_PFN_VAL)
#endif /* BCMDBG_ERR */

#ifdef WLMSG_ASSOC_LT
#define WL_MSG_LEVEL_MASK (_WL_MSG_LEVEL_MASK | WL_ASSOC_VAL)
#else
#define WL_MSG_LEVEL_MASK (_WL_MSG_LEVEL_MASK)
#endif

#define WL_MSG_LEVEL2_MASK (WL_PCIE_VAL)

#define WL_MSG_LEVEL_ALWAYS_ON (\
	(WL_ERROR_ON() ? WL_ERROR_VAL : 0) | \
	(WL_PRHDRS_ON() ? WL_PRHDRS_VAL : 0) | \
	(WL_PRPKT_ON() ? WL_PRPKT_VAL : 0) |   \
	(WL_INFORM_ON() ? WL_INFORM_VAL : 0) | \
	(WL_OID_ON() ? WL_OID_VAL : 0) | \
	(WL_ASSOC_ON() ? WL_ASSOC_VAL : 0) | \
	(WL_WSEC_ON() ? WL_WSEC_VAL : 0) | \
	(WL_WSEC_DUMP_ON() ? WL_WSEC_DUMP_VAL : 0) | \
	(WL_MPC_ON() ? WL_MPC_VAL : 0) | \
	(WL_DFS_ON() ? WL_DFS_VAL : 0))

#define WL_MSG_LEVEL2_ALWAYS_ON (\
	(WL_SCAN_ON() ? WL_SCAN_VAL : 0) | \
	(WL_MBO_DBG_ON() ? WL_MBO_VAL : 0) | \
	(WL_OCE_DBG_ON() ? WL_OCE_VAL : 0))

#endif /* BCMDBG */

#ifdef BCM_IOCV_MEM_LOG
/* Get DLD handle */
extern void * wl_get_dld(void *ptr);
/* Get timestamp string */
extern char* wl_get_timestamp(void);

#define WL_LOG_LEVEL_MASK (WL_G_IOV_VAL)
#define WL_LOG_LEVEL2_MASK (WL_S_IOV_VAL)

#define WL_IOVAR_MEM_WRITE(args, wl) \
	do {	\
		if (WL_S_IOV_MEM_LOG_ON() || WL_G_IOV_MEM_LOG_ON()) {	\
			DLD_WRITE_TO_BUF(wl_get_dld(wl), DLD_BUF_GENERAL, \
				("[%s] ", wl_get_timestamp()));	\
			DLD_WRITE_TO_BUF(wl_get_dld(wl), DLD_BUF_GENERAL, args);	\
		}	\
	} while (0)

/* even in non-BCMDBG builds, logging of iovars should be available
 * Logging to console first, then writes logging to dld buffer
 * This buffer will be flushed to a file upon error
 */
#define WL_IOVAR_MEM(args, wl) \
	do {	\
		if (WL_G_IOV_ON() || WL_S_IOV_ON()) {	\
			printf args;	\
		}	\
		WL_IOVAR_MEM_WRITE(args, wl);	\
	} while (0)
#endif /* BCM_IOCV_MEM_LOG */

#ifdef FTM_CSI
#ifdef EVENT_LOG_COMPILE
#ifdef USE_EVENT_LOG_RA
#define FTM_HW_ERR(args)	EVENT_LOG_RA(EVENT_LOG_TAG_FTM_HW_ERR, args)
#define FTM_HW_INFO(args)	EVENT_LOG_RA(EVENT_LOG_TAG_FTM_HW_INFO, args)
#define FTM_HW_TRACE(args)	EVENT_LOG_RA(EVENT_LOG_TAG_FTM_HW_TRACE, args)
#else
#define FTM_HW_ERR(args)	EVENT_LOG_COMPACT_CAST_PAREN_ARGS(EVENT_LOG_TAG_FTM_HW_ERR, args)
#define FTM_HW_INFO(args)	EVENT_LOG_COMPACT_CAST_PAREN_ARGS(EVENT_LOG_TAG_FTM_HW_INFO, args)
#define FTM_HW_TRACE(args)	EVENT_LOG_COMPACT_CAST_PAREN_ARGS(EVENT_LOG_TAG_FTM_HW_TRACE, args)
#endif /* USE_EVENT_LOG_RA */
#else
#define FTM_HW_ERR(args)	WL_PRINT(args)
#define FTM_HW_INFO(args)	WL_PRINT(args)
#define FTM_HW_TRACE(args)	WL_PRINT(args)
#endif /* EVENT_LOG_COMPILE */
#else
#define FTM_HW_ERR(args)
#define FTM_HW_INFO(args)
#define FTM_HW_TRACE(args)
#endif /* FTM_CSI */

#endif /* _wl_dbg_h_ */
