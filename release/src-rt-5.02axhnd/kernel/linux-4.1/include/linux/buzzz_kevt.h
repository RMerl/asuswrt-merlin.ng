#if defined(CONFIG_BCM_KF_ARM_BCM963XX)
#ifndef __buzzz_kevt_h_included__
#define __buzzz_kevt_h_included__

#if defined(CONFIG_BUZZZ_KEVT) || defined(CONFIG_BUZZZ_FUNC)
/*
 * +----------------------------------------------------------------------------
 *
 * BCM BUZZZ ARM Cortex A9 Router Kernel events
 *
 * $Copyright Open Broadcom Corporation$
 * $Id$
 *
 * vim: set ts=4 noet sw=4 tw=80:
 * -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*-
 *
 * +----------------------------------------------------------------------------
 */

#include <asm/buzzz.h>

#undef BUZZZ_KEVT
#define BUZZZ_KEVT(event)       BUZZZ_KEVT__ ## event,

#undef  _B_
#undef  _H_
#undef  _N_
#undef  _FAIL_
#define _B_                     "\e[0;34m"
#define _H_                     "\e[0;31m;40m"
#define _N_                     "\e[0m"
#define _FAIL_                  _H_ " === FAILURE ===" _N_

/* Expected events : Font = Normal */
#define BUZZZ_KEVTN(event, format) \
    buzzz_klog_reg(BUZZZ_KEVT__## event, "\t\t" format);

/* Unexpected events: Font = bold2 highlighted */
#define BUZZZ_KEVTH(event, format) \
    buzzz_klog_reg(BUZZZ_KEVT__## event, _H_ "\t\t" format _N_);

typedef
enum buzzz_rtr_dpid
{
    BUZZZ_KEVT__DATAPATH_START = 100,

	BUZZZ_KEVT(SAMPLE)

	/* Enet */
	BUZZZ_KEVT(ENET_RX_THREAD)
	BUZZZ_KEVT(ENET_RX_BUDGET)
	BUZZZ_KEVT(BCMEAPI_RX_PKT_BREAK)
	BUZZZ_KEVT(BCMEAPI_RX_PKT_CONT)
	BUZZZ_KEVT(BCMEAPI_RX_PKT_SKIP)
	BUZZZ_KEVT(BCMEAPI_RX_PKT_CHAIN)
	BUZZZ_KEVT(BCMEAPI_RX_PKT_FCACHE)
	BUZZZ_KEVT(BCMEAPI_RX_PKT_SKB)
	BUZZZ_KEVT(BCMEAPI_RX_PKT_NETIF_RX)
	BUZZZ_KEVT(BCMEAPI_RX_PKT_NEXTRX)

	/* Flow Cache */
	BUZZZ_KEVT(FC_RECEIVE)
	BUZZZ_KEVT(FC_STACK)
	BUZZZ_KEVT(FC_PKT_DONE)

	/* DHD */
	BUZZZ_KEVT(DHD_START_XMIT)
	BUZZZ_KEVT(DHD_PROT_TXDATA_BGN)
	BUZZZ_KEVT(DHD_PROT_TXDATA_END)
	BUZZZ_KEVT(DHD_PROT_CREDIT_DROP)
	BUZZZ_KEVT(DHD_PROT_TXDESC_DROP)
	BUZZZ_KEVT(DHD_PROT_PROCESS_BGN)
	BUZZZ_KEVT(DHD_PROT_PROCESS_END)
	BUZZZ_KEVT(DHD_PROCESS_TXSTATUS)
	
	BUZZZ_KEVT(CIRCULARBUF_WRITE_COMPLETE_BGN)
	BUZZZ_KEVT(CIRCULARBUF_WRITE_COMPLETE_END)

    /* NBUFF */
	BUZZZ_KEVT(FKB_FLUSH)

    /* WFD */
	BUZZZ_KEVT(WFD_PKT_GET_BGN)
    BUZZZ_KEVT(WFD_PKT_GET_PROG)
    BUZZZ_KEVT(WFD_PKT_GET_END)
	BUZZZ_KEVT(WFD_TX_HOOK_BGN)
    BUZZZ_KEVT(WFD_TX_HOOK_END)

} buzzz_rtr_dpid_t;


/* Invoke this once in a datapath module's init */
static inline int
buzzz_dp_init(void)
{
	BUZZZ_KEVTN(SAMPLE,                "sample pkt<%p>")

	/* Enet */
	BUZZZ_KEVTN(ENET_RX_THREAD,        "bcm63xx_enet_rx_thread loop")
	BUZZZ_KEVTN(ENET_RX_BUDGET,        "bcm63xx_rx budget<%d>")
	BUZZZ_KEVTN(BCMEAPI_RX_PKT_BREAK,  "bcmeapi_rx_pkt break")
	BUZZZ_KEVTN(BCMEAPI_RX_PKT_CONT,   "bcmeapi_rx_pkt cont")
	BUZZZ_KEVTN(BCMEAPI_RX_PKT_SKIP,   "bcmeapi_rx_pkt skip")
	BUZZZ_KEVTN(BCMEAPI_RX_PKT_CHAIN,  "bcm63xx_rx tx chain")
	BUZZZ_KEVTN(BCMEAPI_RX_PKT_FCACHE, "bcm63xx_rx finit")
	BUZZZ_KEVTN(BCMEAPI_RX_PKT_SKB,    "bcm63xx_rx alloc skb")
	BUZZZ_KEVTN(BCMEAPI_RX_PKT_NETIF_RX, "bcm63xx_rx netif_receive_skb")
	BUZZZ_KEVTN(BCMEAPI_RX_PKT_NEXTRX, "bcm63xx_rx next_rx")

	/* Flow Cache */
	BUZZZ_KEVTN(FC_RECEIVE,            "fc_receive")
	BUZZZ_KEVTN(FC_STACK,              "fc_stack")
	BUZZZ_KEVTN(FC_PKT_DONE,           "fc_stack PKT_DONE")

	/* DHD */
	BUZZZ_KEVTN(DHD_START_XMIT,        "dhd_start_xmit")
	BUZZZ_KEVTN(DHD_PROT_TXDATA_BGN,   "dhd_prot_txdata bgn credit<%d>")
	BUZZZ_KEVTN(DHD_PROT_TXDATA_END,   "dhd_prot_txdata end pktlen<%d>")
	BUZZZ_KEVTH(DHD_PROT_CREDIT_DROP,  "dhd_prot_txdata credit DROP")
	BUZZZ_KEVTH(DHD_PROT_TXDESC_DROP,  "dhd_prot_txdata txdesc DROP")
	BUZZZ_KEVTN(DHD_PROT_PROCESS_BGN,  ">>> dhd_prot_process_msgbuf")
	BUZZZ_KEVTN(DHD_PROT_PROCESS_END,  "<<< dhd_prot_process_msgbuf")
	BUZZZ_KEVTN(DHD_PROCESS_TXSTATUS,  "dhd_prot_txstatus_process")
	
	BUZZZ_KEVTN(CIRCULARBUF_WRITE_COMPLETE_BGN, ">> circularbuf_write_complete")
	BUZZZ_KEVTN(CIRCULARBUF_WRITE_COMPLETE_END, "<< circularbuf_write_complete")

    /* NBUFF */
	BUZZZ_KEVTN(FKB_FLUSH,  "_fkb_flush cache_op<%d> data<%p> dirty<%p> flush_len<%d>")

    /* WFD */
	BUZZZ_KEVTN(WFD_PKT_GET_BGN,  "rdpa_cpu_wfd_packet_get BGN")	
    BUZZZ_KEVTN(WFD_PKT_GET_PROG, "rdpa_cpu_wfd_packet_get cnt<%u> PROG")
	BUZZZ_KEVTN(WFD_PKT_GET_END,  "rdpa_cpu_wfd_packet_get cnt<%u> END")
    BUZZZ_KEVTN(WFD_TX_HOOK_BGN,  "WFD Tx Hook BGN")
    BUZZZ_KEVTN(WFD_TX_HOOK_END,  "WFD Tx Hook END")

	return 0;
}
#else  /* ! CONFIG_BUZZZ */
#define BUZZZ_DPL1(N, ID, ARG...)   do {} while (0)
#define BUZZZ_DPL2(N, ID, ARG...)   do {} while (0)
#define BUZZZ_DPL3(N, ID, ARG...)   do {} while (0)
#define BUZZZ_DPL4(N, ID, ARG...)   do {} while (0)
#define BUZZZ_DPL5(N, ID, ARG...)   do {} while (0)
#endif /* ! CONFIG_BUZZZ */

#endif /* __buzzz_kevt_h_included__ */
#endif
