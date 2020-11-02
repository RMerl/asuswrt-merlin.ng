/*
 * Common (OS-independent) portion of
 * Broadcom 802.11bang Networking Device Driver
 *
 * BMAC portion of common driver. The external functions should match wlc_bmac_stubs.c
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
 * $Id: wlc_bmac.c 778298 2019-08-29 00:43:36Z $
 */

/**
 * @file
 * @brief
 * In contrast to the traditional NIC driver architecture, dongle devices are limited by a slow(er)
 * host-client BUS. To cope with this bus latency(significantly slower R_REG, W_REG), some host
 * driver blocks have to be moved to run on dongle on-chip memory with simple CPU(like ARM7,
 * cortexM3). Dongle driver normally requires less load on host CPU due to the offloading.
 */

/**
 * @file
 * @brief
 * XXX Twiki: [WlBmacDesign]
 */

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */
#include <wlc_cfg.h>

#ifndef WLC_LOW
#error "This file needs WLC_LOW"
#endif // endif

/* On a split driver, wlc_bmac_recv() runs in the low driver. When PKTC is defined,
 * wlc_bmac_recv() calls directly to wlc_rxframe_chainable() and wlc_sendup_chain(),
 * which run in the high driver.
 */
#if (defined(PKTC) || defined(PKTC_DONGLE)) && defined(WLC_SPLIT)
#error "PKTC cannot be defined when WLC_SPLIT is defined"
#endif // endif

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <proto/802.11.h>
#include <bcmwifi_channels.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <wlioctl.h>
#include <sbconfig.h>
#include <sbchipc.h>
#include <pcicfg.h>
#include <sbhndpio.h>
#include <sbhnddma.h>
#include <hnddma.h>
#include <hndpmu.h>
#include <bcmdevs.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_mbss.h>
#include <wlc_channel.h>
#include <wlc_pio.h>
#include <bcmsrom.h>
#include <wlc_rm.h>
#include <wlc_macdbg.h>
#ifdef WLC_LOW
#include <bcmnvram.h>
#endif // endif
#ifdef WLMCNX
#include <wlc_mcnx.h>
#endif // endif
#ifdef WLC_HIGH
#ifdef WLMCHAN
#include <wlc_mchan.h>
#endif // endif
#endif /* WLC_HIGH */
#ifdef PROP_TXSTATUS
#include <wlfc_proto.h>
#include <wl_wlfc.h>
#endif /* PROP_TAXSTATUS */
/* BMAC_NOTE: a WLC_HIGH compile include of wlc.h adds in more structures and type
 * dependencies. Need to include these to files to allow a clean include of wlc.h
 * with WLC_HIGH defined.
 * At some point we may be able to skip the include of wlc.h and instead just
 * define a stub wlc_info and band struct to allow rpc calls to get the rpc handle.
 */
#include <wlc.h>
#include <wlc_hw.h>
#include <wlc_hw_priv.h>
#include <wlc_bmac.h>
#include <wlc_led.h>
#include <wl_export.h>
#include "d11ucode.h"
#ifdef BCMSDIO
#include <bcmsdh.h>
#include <hndpmu.h>
#endif // endif
#include <bcmotp.h>
#include  <wlc_stf.h>
#include <wlc_rsdb.h>
/* BMAC_NOTE: With WLC_HIGH defined, some fns in this file make calls to high level
 * functions defined in the headers below. We should be eliminating those calls and
 * will be able to delete these include lines.
 */
#ifdef WLC_HIGH
#include <wlc_antsel.h>
#endif /* WLC_HIGH */
#ifdef WLDIAG
#include <wlc_diag.h>
#endif // endif
#include <pcie_core.h>
#ifdef ROUTER_COMA
#include <hndchipc.h>
#include <hndjtagdefs.h>
#endif // endif
#if defined(BCMDBG) || defined(WLTEST)
#include <bcmsrom.h>
#endif // endif
#if defined(DSLCPE)
#include <board.h>
#include <boardparms.h>
#include <wl_linux_dslcpe.h>
#endif	/* DSLCPE */
#ifdef AP
#include <wlc_apps.h>
#endif // endif
#include <wlc_extlog.h>
#include <wlc_alloc.h>
#if defined(SR_ESSENTIALS)
#include "saverestore.h"
#endif /* SR_ESSENTIALS */
#include <phy_api.h>
#include <wlc_iocv.h>
#include <wlc_bmac_iocv.h>
#include <wlc_dump_reg.h>
#include <phyioctl.h>
#ifdef DONGLEBUILD
#include <rte.h>
#endif // endif

/* ******************************************************** */
#include <phy_api.h>
#include <phy_ana_api.h>
#include <phy_radio_api.h>
#include <phy_rssi_api.h>
#include <phy_wd_api.h>
#include <phy_dbg_api.h>
#include <phy_utils_api.h>
/* ******************************************************** */

#ifdef BCM_OL_DEV
#include <bcm_ol_msg.h>
#endif // endif

#ifdef WLOFFLD
#include <wlc_offloads.h>
#endif // endif

#ifdef WLAWDL
#include <wlc_awdl.h>
#endif // endif

#ifdef BCM_OL_DEV
#include <wlc_keymgmt.h>
#include <wlc_dngl_ol.h>
#ifdef L2KEEPALIVEOL
#include <wlc_l2keepaliveol.h>
#endif /* L2KEEPALIVEOL */
#endif /* BCM_OL_DEV */

#include <wlc_vasip.h>
#ifdef WLC_LOW_ONLY
#include <bcm_rpc_tp.h>
#endif // endif

#ifdef WLDURATION
#include <wlc_duration.h>
#endif // endif

#ifdef DONGLEBUILD
#include <hndcpu.h>
#endif // endif

#include <wlc_tx.h>

#include <wlc_btcx.h>

#ifdef BCMLTECOEX
#include <wlc_ltecx.h>
#endif /* BCMLTECOEX */

#include <wlc_rx.h>

#ifdef WL11ULB
#ifdef WLAMPDU
#include <wlc_ampdu_rx.h>
#endif /* WLAMPDU */
#endif /* WL11ULB */

#ifdef WL_MU_TX
#include <wlc_mutx.h>
#endif // endif
#ifdef HOST_HDR_FETCH
#include <rte_fetch.h>
#endif /* HOST_HDR_FETCH */
#ifndef EFI
#ifdef ENABLE_PANIC_CHECK_CLK
#define PANIC_CHECK_CLK(clk, format, ...)			\
	do {							\
		if (!clk) {					\
			osl_panic(format, __VA_ARGS__);		\
		}						\
	} while (0);
#else
#define PANIC_CHECK_CLK(clk, format, ...)
#endif // endif
#endif /* !EFI */

#define	TIMER_INTERVAL_WATCHDOG_BMAC	1000	/* watchdog timer, in unit of ms */
#define TIMER_INTERVAL_RPC_AGG_WATCHDOG_BMAC	5 /* rpc agg watchdog timer, in unit of ms */

/* QT PHY */
#define	SYNTHPU_DLY_PHY_US_QT		100	/* QT(no radio) synthpu_dly time in us */

/* real PHYs */
/*
 * XXX - In ucode BOM 820.1 and later SYNTHPU and PRETBTT are independant.
 */
#define	SYNTHPU_DLY_APHY_US		3580	/* a phy synthpu_dly time in us */
#define	SYNTHPU_DLY_BPHY_US		800	/* b/g phy synthpu_dly time in us, def */
#define	SYNTHPU_DLY_LPPHY_US		200	/* lpphy synthpu_dly time in us */
#define SYNTHPU_DLY_LCNPHY_US   	500  	/* lcnphy synthpu_dly time in us */
#ifdef SRFAST
#define SYNTHPU_DLY_LCN40PHY_US		300	/* lcn40phy synthpu_dly time in us */
#else
#define SYNTHPU_DLY_LCN40PHY_US		500	/* lcn40phy synthpu_dly time in us */
#endif /* SRFAST */
#define	SYNTHPU_DLY_NPHY_US		1536	/* n phy REV3 synthpu_dly time in us, def */
#define	SYNTHPU_DLY_HTPHY_US		2288	/* HT phy REV0 synthpu_dly time in us, def */
#define	SYNTHPU_DLY_ACPHY_US		512
#define	SYNTHPU_DLY_ACPHY2_US		1200	/* AC phy synthpu_dly time in us, def */
#define	SYNTHPU_DLY_SSLPNPHY_US		50	/* sslpnphy synthpu_dly time in us */

/* chip specific */
#define SYNTHPU_DLY_LCNPHY_4336_US	400 	/* lcnphy 4336 synthpu_dly time in us */
#if defined(PMU_OPT_REV6)
#define SYNTHPU_DLY_ACPHY_4339_US	310 	/* acphy 4339 synthpu_dly time in us */
#else
#define SYNTHPU_DLY_ACPHY_4339_US	400 	/* acphy 4339 synthpu_dly time in us */
#endif // endif
#define SYNTHPU_DLY_ACPHY_4335_US	400 	/* acphy 4335 synthpu_dly time in us */
#define SYNTHPU_DLY_ACPHY_4349_US	2000 	/* acphy 4349 synthpu_dly time in us */
#define SYNTHPU_DLY_ACPHY_4365_US	1024	/* acphy 4365, 4366 synthpu_dly time in us */

typedef struct _btc_flags_ucode {
	uint8	idx;
	uint16	mask;
} btc_flags_ucode_t;

#define BTC_FLAGS_SIZE 9
#define BTC_FLAGS_MHF3_START 1
#define BTC_FLAGS_MHF3_END   6

const btc_flags_ucode_t btc_ucode_flags[BTC_FLAGS_SIZE] = {
	{MHF2, MHF2_BTCPREMPT},
	{MHF3, MHF3_BTCX_DEF_BT},
	{MHF3, MHF3_BTCX_ACTIVE_PROT},
	{MHF3, MHF3_BTCX_SIM_RSP},
	{MHF3, MHF3_BTCX_PS_PROTECT},
	{MHF3, MHF3_BTCX_SIM_TX_LP},
	{MHF3, MHF3_BTCX_ECI},
	{MHF5, MHF5_BTCX_LIGHT},
	{MHF5, MHF5_BTCX_PARALLEL}
};

#ifndef BMAC_DUP_TO_REMOVE
#define WLC_RM_WAIT_TX_SUSPEND		4 /* Wait Tx Suspend */
#define	ANTCNT			10		/* vanilla M_MAX_ANTCNT value */
#endif	/* BMAC_DUP_TO_REMOVE */

#define DMAREG(wlc_hw, direction, fifonum)	(D11REV_LT(wlc_hw->corerev, 11) ? \
	((direction == DMA_TX) ? \
		(void*)(uintptr)&(wlc_hw->regs->fifo.f32regs.dmaregs[fifonum].xmt) : \
		(void*)(uintptr)&(wlc_hw->regs->fifo.f32regs.dmaregs[fifonum].rcv)) : \
	((direction == DMA_TX) ? \
		(BCM_DMA_CT_ENAB(wlc_hw->wlc) ? (void*)(uintptr)&(wlc_hw->regs->inddma.dma) : \
		(void*)(uintptr)&(wlc_hw->regs->fifo.f64regs[fifonum].dmaxmt)) : \
		(void*)(uintptr)&(wlc_hw->regs->fifo.f64regs[fifonum].dmarcv)))

/*
 * The following table lists the buffer memory allocated to xmt fifos in HW.
 * the size is in units of 256bytes(one block), total size is HW dependent
 * ucode has default fifo partition, sw can overwrite if necessary
 *
 * This is documented in twiki under the topic UcodeTxFifo. Please ensure
 * the twiki is updated before making changes.
 */

#define XMTFIFOTBL_STARTREV	4	/* Starting corerev for the fifo size table */

static uint16 xmtfifo_sz[][NFIFO] = {
	{ 14, 14, 14, 14, 14, 2 }, 	/* corerev 4: 3584, 3584, 3584, 3584, 3584, 512 */
	{ 9, 13, 10, 8, 13, 1 }, 	/* corerev 5: 2304, 3328, 2560, 2048, 3328, 256 */
	{ 9, 13, 10, 8, 13, 1 }, 	/* corerev 6: 2304, 3328, 2560, 2048, 3328, 256 */
	{ 9, 13, 10, 8, 13, 1 }, 	/* corerev 7: 2304, 3328, 2560, 2048, 3328, 256 */
	{ 9, 13, 10, 8, 13, 1 }, 	/* corerev 8: 2304, 3328, 2560, 2048, 3328, 256 */
#if defined(WLNINTENDO_ENABLED) || (defined(MBSS) && !defined(MBSS_DISABLED))
	/* Fifo sizes are different for ucode with this support */
	{ 9, 14, 10, 9, 14, 6 }, 	/* corerev 9: 2304, 3584, 2560, 2304, 3584, 1536 */
#else
	{ 10, 14, 11, 9, 14, 2 }, 	/* corerev 9: 2560, 3584, 2816, 2304, 3584, 512 */
#endif // endif
	{ 10, 14, 11, 9, 14, 2 }, 	/* corerev 10: 2560, 3584, 2816, 2304, 3584, 512 */
#ifdef MACOSX
	/* Give more bandwidth to BK traffic as Apple wants to Aggregate */
	{ 30, 47, 22, 14, 8, 1 }, 	/* corerev 11: 5632, 12032, 5632, 3584, 2048, 256 */
	{ 30, 47, 22, 14, 8, 1 }, 	/* corerev 12: 5632, 12032, 5632, 3584, 2048, 256 */
#else
	{ 9, 58, 22, 14, 14, 5 }, 	/* corerev 11: 2304, 14848, 5632, 3584, 3584, 1280 */
	{ 9, 58, 22, 14, 14, 5 }, 	/* corerev 12: 2304, 14848, 5632, 3584, 3584, 1280 */
#endif // endif
	{ 10, 14, 11, 9, 14, 4 }, 	/* corerev 13: 2560, 3584, 2816, 2304, 3584, 1280 */
	{ 10, 14, 11, 9, 14, 2 }, 	/* corerev 14: 2560, 3584, 2816, 2304, 3584, 512 */
	{ 10, 14, 11, 9, 14, 2 }, 	/* corerev 15: 2560, 3584, 2816, 2304, 3584, 512 */
#ifdef MACOSX
	/* Give more bandwidth to BK traffic as Apple wants to Aggregate */
	{ 98, 159, 160, 21, 8, 1 },	/* corerev 16: 25088, 40704, 40960, 5376, 2048, 256 */
#else /* MACOSX */
#ifdef WLLPRS
	{ 20, 176, 192, 21, 17, 5 },	/* corerev 16: 5120, 45056, 49152, 5376, 4352, 1280 */
#else /* WLLPRS */
	{ 20, 192, 192, 21, 17, 5 },	/* corerev 16: 5120, 49152, 49152, 5376, 4352, 1280 */
#endif /* WLLPRS */
#endif /* MACOSX */
#ifdef WLLPRS
	{ 20, 176, 192, 21, 17, 5 },	/* corerev 17: 5120, 45056, 49152, 5376, 4352, 1280 */
#else /* WLLPRS */
	{ 20, 192, 192, 21, 17, 5 },	/* corerev 17: 5120, 49152, 49152, 5376, 4352, 1280 */
#endif /* WLLPRS */
	{ 20, 192, 192, 21, 17, 5 },	/* corerev 18: 5120, 49152, 49152, 5376, 4352, 1280 */
	{ 20, 192, 192, 21, 17, 5 },	/* corerev 19: 5120, 49152, 49152, 5376, 4352, 1280 */
	{ 20, 192, 192, 21, 17, 5 },	/* corerev 20: 5120, 49152, 49152, 5376, 4352, 1280 */
	{ 9, 58, 22, 14, 14, 5 },	/* corerev 21: 2304, 14848, 5632, 3584, 3584, 1280 */
#ifdef WLLPRS
	{ 9, 42, 22, 14, 14, 5 }, 	/* corerev 22: 2304, 10752, 5632, 3584, 3584, 1280 */
#else /* WLLPRS */
	{ 9, 58, 22, 14, 14, 5 },	/* corerev 22: 2304, 14848, 5632, 3584, 3584, 1280 */
#endif /* WLLPRS */
#ifdef MACOSX
	/* Give more bandwidth to BK traffic as Apple wants to Aggregate */
	{ 98, 159, 160, 21, 8, 1 },	/* corerev 23: 25088, 40704, 40960, 5376, 2048, 256 */
#else
	{ 20, 192, 192, 21, 17, 5 },    /* corerev 23: 5120, 49152, 49152, 5376, 4352, 1280 */
#endif // endif
	{ 9, 58, 22, 14, 14, 5 },	/* corerev 24: 2304, 14848, 5632, 3584, 3584, 1280 */
	{ 9, 58, 22, 14, 14, 5 },	/* corerev 25: 2304, 14848, 5632, 3584, 3584, 1280 */
	{ 150, 223, 223, 21, 17, 5 },	/* corerev 26: 38400, 57088, 57088, 5376, 4352, 1280 */
	{ 20, 192, 192, 21, 17, 5 },	/* corerev 27: 5120, 49152, 49152, 5376, 4352, 1280 */
	{ 9, 58, 22, 14, 14, 5 },	/* corerev 28: 2304, 14848, 5632, 3584, 3584, 1280 */
	{ 9, 58, 22, 14, 14, 5 },	/* corerev 29: 2304, 14848, 5632, 3584, 3584, 1280 */
	{ 9, 98, 22, 14, 14, 5 },       /* corerev 30: 2304, 25088, 5632, 3584, 3584, 1280 */
	{ 9, 58, 22, 14, 14, 5 },	/* corerev 31: 2304, 14848, 5632, 3584, 3584, 1280 */
	{ 12, 183, 25, 17, 17, 8 },	/* corerev 32: 3072, 46848, 6400, 4352, 4352, 2048 */
	{ 9, 58, 22, 14, 14, 5 },	/* corerev 33: 2304, 14848, 5632, 3584, 3584, 1280 */
	{ 9, 183, 25, 17, 17, 8 },	/* corerev 34: 2304, 46848, 6400, 4352, 4352, 2048 */
	{ 9, 183, 25, 17, 17, 8 },	/* corerev 35: 2304, 46848, 6400, 4352, 4352, 2048 */
	{ 9, 183, 25, 17, 17, 8 },	/* corerev 36: 2304, 46848, 6400, 4352, 4352, 2048 */
	{ 9, 58, 22, 14, 14, 5 },	/* corerev 37: 2304, 14848, 5632, 3584, 3584, 1280 */
	{ 9, 183, 25, 17, 17, 8 },	/* corerev 38: 2304, 46848, 6400, 4352, 4352, 2048 */
	{ 9, 183, 25, 17, 17, 8 },	/* corerev 39: 2304, 46848, 6400, 4352, 4352, 2048 */
	{ 9, 183, 25, 17, 17, 8 },	/* corerev >=40: 2304, 46848, 6400, 4352, 4352, 2048 */
};

/* corerev 26 host agg fifo size: 38400, 57088, 57088, 5376, 4352, 1280 */
static uint16 xmtfifo_sz_hostagg[] = { 150, 223, 223, 21, 17, 5 };
/* corerev 26 hw agg fifo size: 25088, 65280, 62208, 5120, 4352, 1280 */
static uint16 xmtfifo_sz_hwagg[] = { 98, 255, 243, 21, 17, 5 };

static uint16 xmtfifo_sz_dummy[] = { 98, 255, 243, 21, 17, 5 };

/* WLP2P Support */
#ifdef WLP2P
#ifndef WLP2P_UCODE
#error "WLP2P_UCODE is not defined"
#endif // endif
#endif /* WLP2P */

/* PIO Mode Support */
#ifdef WLPIO
#define PIO_ENAB_HW(wlc_hw) ((wlc_hw)->_piomode)
#else
#define PIO_ENAB_HW(wlc_hw) 0
#endif /* WLPIO */

/* P2P ucode Support */
#ifdef WLP2P_UCODE
	#if defined(WL_ENAB_RUNTIME_CHECK)
		#define DL_P2P_UC(wlc_hw)	((wlc_hw)->_p2p)
	#elif defined(WLP2P_UCODE_ONLY)
		#define DL_P2P_UC(wlc_hw)	1
	#elif defined(WLMCNX_DISABLED)
		#define DL_P2P_UC(wlc_hw)	0
	#else
		#define DL_P2P_UC(wlc_hw)	((wlc_hw)->_p2p)
	#endif /* WLP2P_UCODE_ONLY */
#else /* !WLP2P_UCODE */
	#define DL_P2P_UC(wlc_hw)	0
#endif /* !WLP2P_UCODE */

typedef struct bmac_pmq_entry {
	struct ether_addr ea;		/* station address */
	uint8 switches;
	uint8 ps_on;
	struct bmac_pmq_entry *next;
} bmac_pmq_entry_t;

#define BMAC_PMQ_SIZE 16
#define BMAC_PMQ_MIN_ROOM 5

/* bitmap of auxpmq index */
typedef uint8 auxpmq_idx_bitmap_t[CEIL(AUXPMQ_ENTRIES, NBBY)];

typedef struct bmac_auxpmq_entry {
	struct ether_addr ea;		/* station address */
} bmac_auxpmq_entry_t;

struct bmac_pmq {
	bmac_pmq_entry_t *entry;
	int active_entries; /* number of entries still to receive akcs from high driver  */
	uint8 tx_draining; /* total number of entries */
	uint8 pmq_read_count; /* how many entries have been read since the last clear */
	uint8 pmq_size;
	/* The following is used for aux pmq */
	bmac_auxpmq_entry_t *auxpmq_list;
	auxpmq_idx_bitmap_t auxpmq_used;
	uint32 auxpmq_full_cnt;
	uint16 auxpmq_entry_cnt;
	uint8 auxpmq_full;
};

typedef struct bmac_dmactl {
	uint16 txmr;		/* no. of outstanding reads */
	uint16 txpfc;		/* tx prefetch control */
	uint16 txpft;		/* tx prefetch threshold */
	uint16 txblen;		/* tx burst len */
	uint16 rxpfc;		/* rx prefetch threshold */
	uint16 rxpft;		/* rx prefetch threshold */
	uint16 rxblen;		/* rx burst len */
} bmac_dmactl_t;

#define D11MAC_BMC_STARTADDR	0	/* Specified in units of 256B */
#define D11MAC_BMC_MAXBUFS		1024
#define D11MAC_BMC_BUFSIZE_512BLOCK	1	/* 1 = 512B */
#define D11MAC_BMC_BUFSIZE_256BLOCK	0	/* 0 = 256B */
#define D11MAC_BMC_MAXFIFOS		9
#define D11MAC_BMC_BUFS_512(sz)	((sz) / (1 << (8 + D11MAC_BMC_BUFSIZE_512BLOCK)))
#define D11MAC_BMC_BUFS_256(sz)	((sz) / (1 << (8 + D11MAC_BMC_BUFSIZE_256BLOCK)))

#define D11AC_MAX_RX_FIFO_NUM	2

#define D11MAC_BMC_TPL_IDX		7	/* Template FIFO#7 */
#define D11MAC_BMC_TPL_BYTES	(21 * 1024)	/* 21K bytes default */
#define D11MAC_BMC_TPL_NUMBUFS	D11MAC_BMC_BUFS_512(D11MAC_BMC_TPL_BYTES) /* Note: only for 512 */

/* For corerev 64, 65 actually the HW can accept 4095 blocks [4095 * 512 byte = ~2M] maximum.
 * Here we limit it to 1M which we think it should be enough for NIC mode.
 */
#define D11MAC_BMC_SYSMEM_MAX_BYTES	(1024 * 1024)	/* 1M bytes maximum */

/* D11MAC rev48, rev49 layout: BM uses 128KB sized banks
 *  bmc_startaddr = 80KB
 *  TPL FIFO#7
 *      TPL BUFs  = 24KB
 *      Deadspace = 24KB*  (deadspace to align SR ASM at start of bank1)
 *      SR ASM    = 4KB*   (allocated at start of bank1)
 * Total FIFO#7 sizing:
 *      SR disabled: 24KB
 *      SR enabled*: 52KB
 */
#define D11MAC_BMC_STARTADDR_SRASM	320 /* units of 256B => 80KB */
#define D11MAC_BMC_TPL_BUFS_BYTES	(24 * 1024)	/* Min TPL FIFO#7 size */
#define D11MAC_BMC_SRASM_OFFSET		(128 * 1024)	/* bank1 */
#define D11MAC_BMC_SRASM_BYTES		(28 * 1024)	/* deadspace + 4KB */
#define D11MAC_BMC_SRASM_NUMBUFS	D11MAC_BMC_BUFS_512(D11MAC_BMC_SRASM_BYTES)

/* For 4349 core revision 50 */
#define D11CORE_TEMPLATE_REG_OFFSET OFFSETOF(d11regs_t, tplatewrptr)

#define D11MAC_BMC_TPL_BYTES_PERCORE	4096	/* 4K Template bytes */
#define D11MAC_BMC_TPL_NUMBUFS_PERCORE	\
	D11MAC_BMC_BUFS_256(D11MAC_BMC_TPL_BYTES_PERCORE)

#define D11CORE_TEMPLATE_REGION_START D11MAC_BMC_TPL_BYTES_PERCORE

#define D11MAC_BMC_SR_BYTES				6144	/* 6K SR bytes */
#define D11MAC_BMC_SR_NUMBUFS			\
	D11MAC_BMC_BUFS_256(D11MAC_BMC_SR_BYTES)

/* For 4365 core revision 64 */
/* 4365 ca7 use first 1792KB space, 114688 units of 16B(128bits) */
#ifdef RAMSIZE
#define D11MAC_SYSM_STARTADDR_H		(RAMSIZE_ADJ / 16) >> 16
#define D11MAC_SYSM_STARTADDR_L		(RAMSIZE_ADJ / 16) & 0xFFFF
#else
#define D11MAC_SYSM_STARTADDR_H		0
#define D11MAC_SYSM_STARTADDR_L		0
#endif // endif

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
#define DUMP_BMC_ARGV_MAX		64
#endif // endif

static void wlc_clkctl_clk(wlc_hw_info_t *wlc, uint mode);
#if !defined(BCM_OL_DEV) || (defined(BCMDBG) || defined(MACOSX)) || defined(__NetBSD__)
static void wlc_bmac_rcvlazy_update(wlc_hw_info_t *wlc_hw, uint32 intrcvlazy);
#endif /* !BCM_OL_DEV || BCMDBG || MACOSX */
static void wlc_coreinit(wlc_hw_info_t *wlc_hw);
#ifndef BCM_OL_DEV
static void wlc_bmac_reset_amt(wlc_hw_info_t *wlc_hw);
static void wlc_bmac_reset_auxpmq(wlc_hw_info_t *wlc_hw);
#endif /* BCM_OL_DEV */
/* used by wlc_bmac_wakeucode_init() */
static void wlc_write_inits(wlc_hw_info_t *wlc_hw, const d11init_t *inits);

#ifdef WLRSDB
static void wlc_bmac_rsdb_write_inits(wlc_hw_info_t *wlc_hw, const d11init_t *common_inits,
	const d11init_t *core1_inits);
#endif /* WLRSDB */

static void _wlc_ucode_write(wlc_hw_info_t *wlc_hw, const uint32 ucode[], const uint nbytes,
	bool psmx);
static void wlc_ucode_write(wlc_hw_info_t *wlc_hw, const uint32 ucode[], const uint nbytes);
static void wlc_ucodex_write(wlc_hw_info_t *wlc_hw, const uint32 ucode[], const uint nbytes);
static void wlc_ucode_write_byte(wlc_hw_info_t *wlc_hw, const uint8 ucode[], const uint nbytes);
static int wlc_bmac_bt_regs_read(wlc_hw_info_t *wlc_hw, uint32 stAdd, uint32 dump_size, uint32 *b);

#ifndef BCMUCDOWNLOAD
static void wlc_ucode_download(wlc_hw_info_t *wlc_hw);
#else
#define wlc_ucode_download(wlc_hw) do {} while (0)
#endif // endif
#ifdef BCMUCDOWNLOAD
int wlc_process_ucodeparts(wlc_info_t *wlc, uint8 *buf_to_process);
int wlc_handle_ucodefw(wlc_info_t *wlc, wl_ucode_info_t *ucode_buf);
int wlc_handle_initvals(wlc_info_t *wlc, wl_ucode_info_t *ucode_buf);

int BCMINITDATA(cumulative_len) = 0;
#endif // endif

static int wlc_reset_accum_pmdur(wlc_info_t *wlc);

d11init_t *BCMINITDATA(initvals_ptr) = NULL;
uint32 BCMINITDATA(initvals_len) = 0;
/* uCode download chunk varies depending on whether it is for
* it for lcn & sslpn or for other chips
*/
#if LCNCONF || SSLPNCONF
#define DL_MAX_CHUNK_LEN 1456  /* 8 * 7 * 26 */
#else
#define DL_MAX_CHUNK_LEN 1408 /* 8 * 8 * 22 */
#endif // endif

static void wlc_ucode_txant_set(wlc_hw_info_t *wlc_hw);

/* The following variable used for dongle images which have
ucode download feature. Since ucode is downloaded in chunks &
written to ucode memory it is necessary to identify the
first chunk, hence the variable which gets reclaimed in
attach phase.
*/
uint32 ucode_chunk = 0;

/* used by wlc_dpc() */
static bool wlc_bmac_dotxstatus(wlc_hw_info_t *wlc, tx_status_t *txs, uint32 s2);
static bool wlc_bmac_txstatus_corerev4(wlc_hw_info_t *wlc);
#if defined(STA) && defined(BCMDBG)
static void wlc_bmac_dma_lpbk(wlc_hw_info_t *wlc_hw, bool enable);
#endif // endif

#ifdef WLLED
static void wlc_bmac_led_hw_init(wlc_hw_info_t *wlc_hw);
#endif // endif

/* used by wlc_down() */
static void wlc_flushqueues(wlc_hw_info_t *wlc_hw);

static void wlc_write_mhf(wlc_hw_info_t *wlc_hw, uint16 *mhfs);
#if defined(WL_PSMX)
static void wlc_mctrlx_reset(wlc_hw_info_t *wlc_hw);
static void wlc_bmac_mctrlx(wlc_hw_info_t *wlc_hw, uint32 mask, uint32 val);
static int wlc_bmac_wowlucodex_start(wlc_hw_info_t *wlc_hw);
#else
#define wlc_mctrlx_reset(a) do {} while (0)
#define wlc_bmac_mctrlx(a, b, c) do {} while (0)
#define wlc_bmac_wowlucodex_start(a) 0
#endif /* WL_PSMX */
static void wlc_bmac_btc_btcflag2ucflag(wlc_hw_info_t *wlc_hw);
static bool wlc_bmac_btc_param_to_shmem(wlc_hw_info_t *wlc_hw, uint32 *pval);
static bool wlc_bmac_btc_flags_ucode(uint8 val, uint8 *idx, uint16 *mask);
static void wlc_bmac_btc_flags_upd(wlc_hw_info_t *wlc_hw, bool set_clear, uint16, uint8, uint16);
static void wlc_bmac_btc_gpio_enable(wlc_hw_info_t *wlc_hw);
static void wlc_bmac_btc_gpio_disable(wlc_hw_info_t *wlc_hw);
static void wlc_bmac_btc_gpio_configure(wlc_hw_info_t *wlc_hw);
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static void wlc_bmac_btc_dump(wlc_hw_info_t *wlc_hw, struct bcmstrbuf *b);
#endif // endif

typedef wlc_dump_reg_fn_t bmac_dump_fn_t;

#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(WLTEST) || \
	defined(BCMDBG_PHYDUMP)
static int wlc_bmac_add_dump_fn(wlc_hw_info_t *wlc_hw, char *name, bmac_dump_fn_t fn,
	const void *ctx);
#endif // endif
#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(BCMDBG_PHYDUMP)
static int wlc_bmac_dbg_dump(wlc_hw_info_t *wlc_hw, char *name, struct bcmstrbuf *b);
static int wlc_bmac_dump_bmac(wlc_hw_info_t *wlc_hw, char *name, struct bcmstrbuf *b);
static int wlc_bmac_dump_phy(wlc_hw_info_t *wlc_hw, char *name, struct bcmstrbuf *b);
static void wlc_bmac_suspend_dump(wlc_hw_info_t *wlc_hw, struct bcmstrbuf *b);
#endif /* BCMDBG || BCMDBG_DUMP || BCMDBG_PHYDUMP */
static int wlc_bmac_register_dumps(wlc_hw_info_t *wlc_hw);

/* Low Level Prototypes */
#ifdef AP
#ifdef WLC_LOW_ONLY
static void wlc_bmac_pmq_remove(wlc_hw_info_t *wlc, bmac_pmq_entry_t *pmq_entry);
static bmac_pmq_entry_t * wlc_bmac_pmq_find(wlc_hw_info_t *wlc, struct ether_addr *ea);
#ifndef TXQ_MUX
static bmac_pmq_entry_t * wlc_bmac_pmq_add(wlc_hw_info_t *wlc, struct ether_addr *ea);
#endif /* TXQ_MUX */
static void wlc_bmac_pa_war_set(wlc_hw_info_t *wlc_hw, bool enable);
#endif /* WLC_LOW_ONLY */
#ifdef WL_AUXPMQ
#define AUXPMQ_ADD_ENTRY	0x01
#define AUXPMQ_REMOVE_ENTRY	0x02
#define AUXPMQ_CLEAR_ALL	0x04

static uint16 wlc_bmac_auxpmq_add(wlc_hw_info_t * wlc_hw, struct ether_addr * ea);
static void wlc_bmac_auxpmq_remove(wlc_hw_info_t * wlc_hw, uint16 idx);
static void wlc_bmac_clear_auxpmq(wlc_hw_info_t *wlc_hw);
static void wlc_bmac_process_auxpmq(wlc_hw_info_t *wlc_hw, struct ether_addr *ea,
	uint8 operation, uint16 *auxpmq_idx);
#endif /* WL_AUXPMQ */
static void wlc_bmac_pmq_delete(wlc_hw_info_t *wlc_hw);
static int wlc_bmac_pmq_init(wlc_hw_info_t *wlc);
static void wlc_bmac_clearpmq(wlc_hw_info_t *wlc);
#endif /* AP */
static uint16 wlc_bmac_read_objmem16(wlc_hw_info_t *wlc_hw, uint offset, uint32 sel);
static uint32 wlc_bmac_read_objmem32(wlc_hw_info_t *wlc_hw, uint offset, uint32 sel);
static void wlc_bmac_write_objmem16(wlc_hw_info_t *wlc_hw, uint offset, uint16 v, uint32 sel);
static void wlc_bmac_write_objmem32(wlc_hw_info_t *wlc_hw, uint offset, uint32 v, uint32 sel);
static void wlc_bmac_update_objmem16(wlc_hw_info_t *wlc_hw, uint offset,
	uint16 v, uint16 mask, uint32 objsel);
static bool wlc_bmac_attach_dmapio(wlc_hw_info_t *wlc_hw, bool wme);
static void wlc_bmac_detach_dmapio(wlc_hw_info_t *wlc_hw);
static void wlc_ucode_bsinit(wlc_hw_info_t *wlc_hw);
static bool wlc_validboardtype(wlc_hw_info_t *wlc);
static bool wlc_isgoodchip(wlc_hw_info_t* wlc_hw);
static char* wlc_get_macaddr(wlc_hw_info_t *wlc_hw);
static void wlc_mhfdef(wlc_hw_info_t *wlc_hw, uint16 *mhfs);
static void wlc_mctrl_write(wlc_hw_info_t *wlc_hw);
static void wlc_ucode_mute_override_set(wlc_hw_info_t *wlc_hw);
static void wlc_ucode_mute_override_clear(wlc_hw_info_t *wlc_hw);
static void wlc_bmac_ifsctl1_regshm(wlc_hw_info_t *wlc_hw, uint32 mask, uint32 val);
#if defined(STA) && defined(WLRM)
static uint16 wlc_bmac_read_ihr(wlc_hw_info_t *wlc_hw, uint offset);
#endif // endif
static uint32 wlc_wlintrsoff(wlc_hw_info_t *wlc_hw);
static void wlc_wlintrsrestore(wlc_hw_info_t *wlc_hw, uint32 macintmask);
#ifdef BCMDBG
static bool wlc_intrs_enabled(wlc_hw_info_t *wlc_hw);
#endif /* BCMDBG */
#ifndef BCM_OL_DEV
static int wlc_bmac_btc_param_attach(wlc_info_t *wlc);
static void wlc_bmac_btc_param_init(wlc_hw_info_t *wlc_hw);
static void wlc_corerev_fifofixup(wlc_hw_info_t *wlc_hw);
static void wlc_ucode_pcm_write(wlc_hw_info_t *wlc_hw, const uint32 pcm[], const uint nbytes);
static void wlc_gpio_init(wlc_hw_info_t *wlc_hw);
static int wlc_corerev_fifosz_validate(wlc_hw_info_t *wlc_hw, uint16 *buf);
static int wlc_bmac_bmc_init(wlc_hw_info_t *wlc_hw);
static bool wlc_bmac_txfifo_sz_chk(wlc_hw_info_t *wlc_hw);
#endif /* BCM_OL_DEV */
static void wlc_write_hw_bcntemplate0(wlc_hw_info_t *wlc_hw, void *bcn, int len);
static void wlc_write_hw_bcntemplate1(wlc_hw_info_t *wlc_hw, void *bcn, int len);
static void wlc_bmac_bsinit(wlc_hw_info_t *wlc_hw, chanspec_t chanspec, bool chanswitch_path);
static uint32 wlc_setband_inact(wlc_hw_info_t *wlc_hw, uint bandunit);
static void wlc_bmac_setband(wlc_hw_info_t *wlc_hw, uint bandunit, chanspec_t chanspec);
static void wlc_bmac_update_slot_timing(wlc_hw_info_t *wlc_hw, bool shortslot);
#ifdef WL11N
static void wlc_upd_ofdm_pctl1_table(wlc_hw_info_t *wlc_hw);
static uint16 wlc_bmac_ofdm_ratetable_offset(wlc_hw_info_t *wlc_hw, uint8 rate);
#endif // endif
#if (defined(BCMNVRAMR) || (defined(BCMNVRAMW) && !defined(WLTEST_DISABLED))) && \
	(defined(WLTEST) && !defined(WLTEST_DISABLED))
static int wlc_bmac_cissource(wlc_hw_info_t *wlc_hw);
#endif // endif
#if defined(DSLCPE)
#if defined(DSLCPE_WOMBO)
extern int read_sromfile(void *swmap, void *buf, uint offset, uint nbytes);
#endif /* DSLCPE_WOMBO */
extern int sprom_update_params(si_t *sbh, uint16 *buf);
#endif /* DSLCPE */
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int wlc_bmac_bmc_dump(wlc_hw_info_t *wlc_hw, struct bcmstrbuf *b);
#endif // endif

#ifdef NOT_RIGGED_UP_YET
/* switch phymode supported on RSDB family of chips */
static int wlc_bmac_switch_phymode(wlc_hw_info_t *wlc_hw, uint16 requested_phymode);
#endif /* NOT_RIGGED_UP_YET */

#if defined(BCMPCIEDEV)
static int wlc_bmac_process_split_fifo_pkt(wlc_hw_info_t *wlc_hw, uint fifo, void* p);
#endif /* BCMPCIEDEV */
static uint8 wlc_bmac_rxfifo_enab(uint fifo);

#ifdef WL11ULB
static void wlc_bmac_set_ulbparams(wlc_hw_info_t *wlc_hw, chanspec_t chanspec);
#endif /* WL11ULB */

#ifdef BCM_DMA_CT
static void wlc_bmac_enable_ct_access(wlc_hw_info_t *wlc_hw, bool enabled);
#endif // endif
#ifdef HOST_HDR_FETCH
static void wlc_txs_fetch_rqst_detach(wlc_info_t * wlc);
static void wlc_txs_fetch_rqst_attach(wlc_info_t * wlc);
static void wlc_bmac_process_fetched_hdr_txstatus(struct fetch_rqst *fr, bool cancelled);

static txhdr_fetch_rqst_t* wlc_txs_fetch_rqst_get(wlc_info_t * wlc);
static void wlc_txs_fetch_rqst_release(wlc_info_t * wlc, txhdr_fetch_rqst_t* fetch_rqst);
#endif /* HOST_HDR_FETCH */

uint32 wlc_bmac_txfifo_flush_status(wlc_hw_info_t *wlc_hw, uint fifo);
uint32 wlc_bmac_txfifo_suspend_status(wlc_hw_info_t *wlc_hw, uint fifo);

void wlc_bmac_init_core_reset_disable_fn(wlc_hw_info_t *wlc_hw);
void wlc_bmac_core_reset(wlc_hw_info_t *wlc_hw, uint32 flags, uint32 resetflags);
void wlc_bmac_core_disable(wlc_hw_info_t *wlc_hw, uint32 bits);
bool wlc_bmac_islast_core(wlc_hw_info_t *wlc_hw);
static void wlc_bmac_4349_btcx_prisel_war(wlc_hw_info_t *wlc_hw);
void wlc_bmac_4349_core1_hwreqoff(wlc_hw_info_t *wlc_hw, bool mode);
#ifndef BCM_OL_DEV
static void wlc_bmac_bmc_template_allocstatus(wlc_hw_info_t *wlc_hw,
uint32 mac_core_unit, int tplbuf);
#endif /* BCM_OL_DEV */
#ifdef WLC_LOW_ONLY
/* debug/trace */
#ifdef BCMDBG
uint wl_msg_level = WL_ERROR_VAL;
#ifndef BCMDBG_EXCLUDE_HW_TIMESTAMP
wlc_info_t *wlc_info_time_dbg = (wlc_info_t *)(NULL);
#endif /* !BCMDBG_EXCLUDE_HW_TIMESTAMP */
#else /* BCMDBG */
uint wl_msg_level = 0;
#endif /* BCMDBG */
uint wl_msg_level2 = 0;
#endif /* WLC_LOW_ONLY */

#ifdef BCM_HOST_MEM_UCODE
uint32 *host_d11ucode = NULL, *host_d11ucodex = NULL;
uint32 *host_d11ucode_mu = NULL, *host_d11ucodex_mu = NULL;
uint32 host_d11ucodesz = 0, host_d11ucodexsz = 0;
uint32 host_d11ucode_musz = 0, host_d11ucodex_musz = 0;
#endif /* BCM_HOST_MEM_UCODE */

static const char BCMATTACHDATA(rstr_devid)[] = "devid";
static const char BCMINITDATA(rstr_wlD)[] = "wl%d:dma%d";
#ifdef BCM_DMA_CT
static const char BCMINITDATA(rstr_wl_aqmD)[] = "wl%d:aqm_dma%d";
#endif // endif
#if defined(__mips__) || defined(BCM47XX_CA9)
static const char BCMATTACHDATA(rstr_wl_tlclk)[] = "wl_tlclk";
#endif // endif
static const char BCMATTACHDATA(rstr_vendid)[] = "vendid";
static const char BCMATTACHDATA(rstr_boardrev)[] = "boardrev";
static const char BCMATTACHDATA(rstr_sromrev)[] = "sromrev";
static const char BCMATTACHDATA(rstr_boardflags)[] = "boardflags";
static const char BCMATTACHDATA(rstr_boardflags2)[] = "boardflags2";
static const char BCMATTACHDATA(rstr_boardflags4)[] = "boardflags4";
static const char BCMATTACHDATA(rstr_antswctl2g)[] = "antswctl2g";
static const char BCMATTACHDATA(rstr_antswctl5g)[] = "antswctl5g";
#ifdef PKTC
static const char BCMATTACHDATA(rstr_pktc_disable)[] = "pktc_disable";
#endif // endif
static const char BCMATTACHDATA(rstr_aa2g)[] = "aa2g";
static const char BCMATTACHDATA(rstr_macaddr)[] = "macaddr";
static const char BCMATTACHDATA(rstr_il0macaddr)[] = "il0macaddr";
static const char BCMATTACHDATA(rstr_et1macaddr)[] = "et1macaddr";
#ifdef WLLED
static const char BCMATTACHDATA(rstr_ledbhD)[] = "ledbh%d";
static const char BCMATTACHDATA(rstr_wl0gpioD)[] = "wl0gpio%d";
static const char BCMATTACHDATA(rstr_bmac_led_attach_out_of_mem_malloced_D_bytes)[] =
		"wlc_bmac_led_attach: out of memory, malloced %d bytes";
static const char BCMATTACHDATA(rstr_wlD_led_attach_wl_init_timer_for_led_blink_timer_failed)[] =
		"wl%d: wlc_led_attach: wl_init_timer for led_blink_timer failed\n";
#endif /* WLLED */
static const char BCMATTACHDATA(rstr_btc_paramsD)[] = "btc_params%d";
static const char BCMATTACHDATA(rstr_btc_flags)[] = "btc_flags";
static const char BCMATTACHDATA(rstr_btc_mode)[] = "btc_mode";
static const char BCMATTACHDATA(rstr_wowl_gpio)[] = "wowl_gpio";
static const char BCMATTACHDATA(rstr_wowl_gpiopol)[] = "wowl_gpiopol";

/* === Low Level functions === */

#ifdef WLC_LOW_ONLY
wlc_pub_t *
wlc_pub(void *wlc)
{
	return ((wlc_info_t *)wlc)->pub;
}

#ifdef BCM_OL_DEV /* offload firmware specific */

/* Peterson's algorithm
 * P1: flag[1] = true;
 *   turn = 0;
 *   while (flag[0] == true && turn == 0)
 *   {
 *       / / busy wait
 *   }
 *   / / critical section
 *   ...
 *   / / end of critical section
 *   flag[1] = false;
 */

void
tcm_sem_enter(wlc_info_t *wlc)
{
	int i = 0;
	ppcie_shared->flag[1] = TRUE;
	ppcie_shared->turn = 0;
	while (ppcie_shared->flag[0] == TRUE && ppcie_shared->turn == 0)
	{
		i++;
		WL_ERROR(("################## %s : loop %d ##############\n", __FUNCTION__, i));
	/* busy wait */
	}
}

void
tcm_sem_exit(wlc_info_t *wlc)
{
	/* end of critical section */
	ppcie_shared->flag[1] = FALSE;
}

static void
wlc_bmac_txstatus_init_shm(wlc_hw_info_t *wlc_hw)
{
	uint16 txs_wptr;

	wlc_hw->pso_blk = wlc_bmac_read_shm(wlc_hw, M_ARM_PSO_BLK_PTR) * 2;

	wlc_hw->txs_rptr = wlc_bmac_read_shm(wlc_hw, wlc_hw->pso_blk + M_TXS_FIFO_RPTR);
	txs_wptr = wlc_bmac_read_shm(wlc_hw, wlc_hw->pso_blk + M_TXS_FIFO_WPTR);
	wlc_hw->txs_addr_blk = (wlc_hw->pso_blk + M_TXS_FIFO_BLK)/2;
	wlc_hw->txs_addr_end = wlc_hw->txs_addr_blk + M_TXS_FIFO_BLK_SIZE/2;

	WL_ERROR(("%s : rptr:0x%x wptr:0x%x block start:0x%x\n",
		__FUNCTION__, wlc_hw->txs_rptr, txs_wptr, wlc_hw->txs_addr_blk));

	ASSERT(txs_wptr >= wlc_hw->txs_addr_blk);
	ASSERT(txs_wptr <= wlc_hw->txs_addr_end);

	ASSERT(wlc_hw->txs_rptr >= wlc_hw->txs_addr_blk);
	ASSERT(wlc_hw->txs_rptr <= wlc_hw->txs_addr_end);

	/* Make sure we start from a fresh page(rptr == wptr) in case ARM
	 * firmware got reloaded in the middle, without reloading
	 * ucode. Write wptr to rptr location.
	 */
	wlc_bmac_write_shm(wlc_hw, (wlc_hw->pso_blk + M_TXS_FIFO_RPTR), txs_wptr);

}

void
wlc_bmac_sync_sw_state(wlc_hw_info_t *wlc_hw)
{
	uint8 idx;
	const uint16 addr[] = {M_HOST_FLAGS1, M_HOST_FLAGS2, M_HOST_FLAGS3,
		M_HOST_FLAGS4, M_HOST_FLAGS5};
#if defined(WL_PSMX)
	const uint16 addrx[] = {MX_HOST_FLAGS0};
#endif /* WL_PSMX */

	/* Sync maccontrol */
	wlc_hw->maccontrol = R_REG(wlc_hw->osh, &wlc_hw->regs->maccontrol);
	ASSERT(ARRAYSIZE(addr) == MHFMAX);
#if defined(WL_PSMX)
	wlc_hw->maccontrol_x = R_REG(wlc_hw->osh, &wlc_hw->regs->maccontrol_x);
	ASSERT(ARRAYSIZE(addrx) == MXHFMAX);
#endif /* WL_PSMX */

	/* Sync host flags */
	for (idx = 0; idx < MHFMAX; idx++) {
		wlc_bmac_mhf(wlc_hw, idx, 0xFFFF,
			wlc_bmac_read_shm(wlc_hw, addr[idx]),
			WLC_BAND_ALL);
	}
#if defined(WL_PSMX)
	if (D11REV_GE(wlc_hw->corerev, 64)) {
		for (idx = 0; idx < MXHFMAX; idx++) {
			wlc_bmac_mhf(wlc_hw, MXHF0+idx, 0xFFFF,
				wlc_bmac_read_shmx(wlc_hw, addrx[idx]),
				WLC_BAND_ALL);
		}
	}
#endif /* WL_PSMX */
	ASSERT(wlc_hw->clk);
#ifndef EFI
	PANIC_CHECK_CLK(wlc_hw->clk, "wl%d: %s: NO CLK\n", wlc_hw->unit, __FUNCTION__);
#endif // endif

	/* When transistion to into WoWL, clk is force FAST, put back to DYNAMIC */
	if (R_REG(wlc_hw->osh, &wlc_hw->regs->clk_ctl_st) & CCS_FORCEHT)
		wlc_clkctl_clk(wlc_hw, CLK_DYNAMIC);

	wlc_phy_hw_clk_state_upd(wlc_hw->band->pi, TRUE);
}

/* offload firmware specific: make interface operational */
int
BCMINITFN(wlc_up)(wlc_info_t *wlc)
{
	wlc_hw_info_t *wlc_hw = wlc->hw;
	int ret;

	WL_TRACE(("wl%d: %s:\n", wlc_hw->unit, __FUNCTION__));

	if (wlc_hw->up) {
		WL_ERROR(("wl%d: %s: Already up.\n", wlc_hw->unit, __FUNCTION__));
		return BCME_NOTDOWN;
	}

	/* Sync the software state with hardware */
	wlc_bmac_sync_sw_state(wlc_hw);

	wlc_coredisable(wlc_hw);

	wlc_bmac_hw_up(wlc_hw);

	ret = wlc_bmac_up_prep(wlc_hw);

	wlc_bmac_reset(wlc_hw);

	wlc_bmac_init(wlc_hw, wlc_hw->chanspec, FALSE, 0);

	/* Clear the assumption that PSM is suspended */
	wlc_hw->mac_suspend_depth = 0;
#if defined(WL_PSMX)
	wlc_hw->macx_suspend_depth = 0;
#endif // endif
	mboolclr(wlc_hw->wake_override, WLC_WAKE_OVERRIDE_MACSUSPEND);

	wlc_bmac_up_finish(wlc_hw);

	/* Initialize the shmem blk to txstatus on ATIM fofo */
	wlc_bmac_txstatus_init_shm(wlc_hw);

#ifdef BCM_OL_DEV
	RXOEINCCNTR(wlc->wlc_dngl_ol);
#endif // endif

	return ret;
}

void
wlc_bmac_set_wake_ctrl(wlc_hw_info_t *wlc_hw, bool wake)
{
	uint32 new_mc;
	bool awake_before;
	volatile uint32 mc;

	mc = R_REG(wlc_hw->osh, &wlc_hw->regs->maccontrol);

	new_mc = wake ? MCTL_WAKE : 0;

#if defined(BCMDBG) || defined(WLMSG_PS)
	if ((mc & MCTL_WAKE) && !wake)
		WL_ERROR(("PS mode: clear WAKE (sleep if permitted) 0x%x\n",
			wlc_hw->wlc->wlc_dngl_ol->stay_awake));
	if (!(mc & MCTL_WAKE) && wake)
		WL_ERROR(("PS mode: set WAKE (stay awake) 0x%x\n",
			wlc_hw->wlc->wlc_dngl_ol->stay_awake));
#endif	/* BCMDBG || WLMSG_PS */

	wlc_bmac_mctrl(wlc_hw, MCTL_WAKE, new_mc);

	awake_before = !!(mc & MCTL_WAKE);

	if (wake && !awake_before)
		wlc_bmac_wait_for_wake(wlc_hw);
}

#endif /* BCM_OL_DEV */

/**
 * For a BMAC firmware build, there is no wlc contained in the build (just a WLC high stub). For
 * that case, the BMAC contains its own wlc_attach().
 */
void *
BCMATTACHFN(wlc_attach)(void *wl, uint16 vendor, uint16 device, uint unit, uint iomode,
                      osl_t *osh, void *regsva, uint bustype, void *btparam, void *objr, uint *perr)
{
	wlc_info_t *wlc = NULL;
	uint err = 0;
	si_t *sih = NULL;
	char *vars = NULL;
	uint vars_size = 0;

	WL_TRACE(("wl%d: wlc_attach: vendor 0x%x device 0x%x\n", unit, vendor, device));

	sih = wlc_bmac_si_attach((uint)device, osh, regsva, bustype, btparam,
		&vars, &vars_size);
	if (sih == NULL) {
		WL_ERROR(("wl%d: %s: si_attach failed\n", unit, __FUNCTION__));
		err = 1;
		goto fail;
	}
	if (vars) {
		char *var;
		if ((var = getvar(vars, rstr_devid))) {
			uint16 devid = (uint16)bcm_strtoul(var, NULL, 0);

			WL_ERROR(("wl%d: %s: Overriding device id = 0x%x with 0x%x\n",
				unit, __FUNCTION__, device, devid));
			device = devid;
		}
	}

	/* allocate wlc_info_t state and its substructures */
	if ((wlc = (wlc_info_t*) wlc_attach_malloc(osh, unit, &err, device, objr)) == NULL)
		goto fail;
	wlc->osh = osh;
	wlc->objr = objr;

	/* stash sih, vars and vars_size in pub now */
	wlc->pub->sih = sih;
	sih = NULL;
	wlc->pub->vars = vars;
	vars = NULL;
	wlc->pub->vars_size = vars_size;
	vars_size = 0;

	wlc->core = wlc->corestate;
	wlc->wl = wl;
	wlc->pub->_piomode = (iomode == IOMODE_TYPE_PIO);
#if defined(BCM_DMA_CT) && !defined(BCM_DMA_CT_DISABLED)
	wlc->_dma_ct = (iomode == IOMODE_TYPE_CTDMA);
#endif // endif

	/* do the low level hw attach steps */
	err = wlc_bmac_attach(wlc, vendor, device, unit, (iomode == IOMODE_TYPE_PIO),
		osh, regsva, bustype, btparam);
	if (err != 0)
		goto fail;

#if defined(BCMDBG) && !defined(BCMDBG_EXCLUDE_HW_TIMESTAMP)
	if (wlc_info_time_dbg == NULL) {
	    wlc_info_time_dbg = wlc;
	}
#endif /* BCMDBG && !BCMDBG_EXCLUDE_HW_TIMESTAMP */

	if (!(wlc->hw->wdtimer = wl_init_timer(wlc->wl, wlc_bmac_watchdog, wlc, "watchdog"))) {
		WL_ERROR(("wl%d: %s: wl_init_timer for wdtimer failed\n", unit, __FUNCTION__));
		err = 39;
		goto fail;
	}

#ifndef BCM_OL_DEV

	if (!(wlc->hw->rpc_agg_wdtimer = wl_init_timer(wlc->wl, wlc_bmac_rpc_agg_watchdog,
		wlc, "rpc_agg_watchdog"))) {
		WL_ERROR(("wl%d: %s: wl_init_timer for rpc_agg_wdtimer failed\n", unit,
			__FUNCTION__));
		err = 39;
		goto fail;
	}
#endif /* BCM_OL_DEV */

	if (perr)
		*perr = 0;
#ifdef BCM_OL_DEV
	if ((wlc->wlc_dngl_ol = wlc_dngl_ol_attach(wlc)) == NULL) {
		WL_ERROR(("wlc%d: %s: wlc_dngl_ol_attach() failed.\n", unit, __FUNCTION__));
		goto fail;
	}

	wlc->keymgmt = wlc_keymgmt_attach(wlc);
	if (wlc->keymgmt == NULL) {
		WL_ERROR(("wlc%d: %s: wlc_keymgmt_attach() failed.\n", unit, __FUNCTION__));
		goto fail;
	}
#endif // endif
	return ((void*)wlc);

fail:
	WL_ERROR(("wl%d: wlc_attach: failed with err %d\n", unit, err));
	if (sih) {
		wlc_bmac_si_detach(osh, sih);
		sih = NULL;
	}
	if (vars) {
		MFREE(osh, vars, vars_size);
		vars = NULL;
		vars_size = 0;
	}

#ifndef BCMNODOWN
	if (wlc)
		wlc_detach(wlc);
#endif /* BCMNODOWN */

	if (perr)
		*perr = err;
	return (NULL);
}

/*
 * XXX Do we need a separate low implementation for this, or just wlc_bmac_detach()
 * that is called from high level wlc_detach()?
 */
/** BMAC specific detach function */
uint
BCMATTACHFN(wlc_detach)(wlc_info_t *wlc)
{
	wlc_hw_info_t *wlc_hw;

	if (wlc == NULL)
		return 0;

	wlc_hw = wlc->hw;

#if defined(BCMDBG) && !defined(BCMDBG_EXCLUDE_HW_TIMESTAMP)
	if (wlc == wlc_info_time_dbg) {
	    wlc_info_time_dbg = NULL;
	}
#endif /* BCMDBG && !BCMDBG_EXCLUDE_HW_TIMESTAMP */

	/* free timer state */
	if (wlc_hw->wdtimer) {
		wl_free_timer(wlc->wl, wlc_hw->wdtimer);
		wlc_hw->wdtimer = NULL;
	}

	/* free timer state */
	if (wlc_hw->rpc_agg_wdtimer) {
		wl_free_timer(wlc->wl, wlc_hw->rpc_agg_wdtimer);
		wlc_hw->rpc_agg_wdtimer = NULL;
	}

	/* XXX:do not change the order of the functions below, sih is
	 * still used inside wlc_bmac_detach()
	 */
	wlc_bmac_detach(wlc);

	/* free the sih now */
	if (wlc->pub->sih) {
		wlc_bmac_si_detach(wlc->osh, wlc->pub->sih);
		wlc->pub->sih = NULL;
	}

	if (wlc->pub->vars) {
		MFREE(wlc->osh, wlc->pub->vars, wlc->pub->vars_size);
		wlc->pub->vars = NULL;
	}

	wlc_detach_mfree(wlc, wlc->osh);
	return 0;
}

/** BMAC specific */
void
BCMINITFN(wlc_reset)(wlc_info_t *wlc)
{
	wlc_hw_info_t *wlc_hw = wlc->hw;
	WL_TRACE(("wl%d: wlc_reset\n", wlc_hw->unit));
	wlc_bmac_reset(wlc_hw);
}

int
wlc_ioctl(wlc_info_t *wlc, int cmd, void *arg, int len, struct wlc_if *wlcif)
{
	return 0;
}

int
wlc_iovar_op(wlc_info_t *wlc, const char *name,
	void *params, int p_len, void *arg, int len,
             bool set, struct wlc_if *wlcif)
{
	return 0;
}
#endif /* WLC_LOW_ONLY */

/** 11b/g has longer slot duration than 11g */
void
wlc_bmac_set_shortslot(wlc_hw_info_t *wlc_hw, bool shortslot)
{
	wlc_hw->shortslot = shortslot;

	if (BAND_2G(wlc_hw->band->bandtype) && wlc_hw->up) {
		wlc_bmac_suspend_mac_and_wait(wlc_hw);
		wlc_bmac_update_slot_timing(wlc_hw, shortslot);
		wlc_bmac_enable_mac(wlc_hw);
	}
}

/**
 * Update the slot timing for standard 11b/g (20us slots)
 * or shortslot 11g (9us slots)
 * The PSM needs to be suspended for this call.
 */
static void
wlc_bmac_update_slot_timing(wlc_hw_info_t *wlc_hw, bool shortslot)
{
	osl_t *osh = wlc_hw->osh;
	d11regs_t *regs = wlc_hw->regs;

	if (shortslot) {
		/* 11g short slot: 11a timing */
		W_REG(osh, &regs->u.d11regs.ifs_slot, 0x0207);	/* APHY_SLOT_TIME */
		wlc_bmac_write_shm(wlc_hw, M_DOT11_SLOT, APHY_SLOT_TIME);
#ifdef WL11ULB
		if (ULB_ENAB(wlc_hw->wlc->pub)) {
			if (CHSPEC_IS10(wlc_hw->chanspec)) {
				W_REG(osh, &regs->u.d11regs.ifs_slot, IFS_SLOT_10M);
				wlc_bmac_write_shm(wlc_hw, M_DOT11_SLOT, 2*APHY_SLOT_TIME);
			} else if (CHSPEC_IS5(wlc_hw->chanspec)) {
				W_REG(osh, &regs->u.d11regs.ifs_slot, IFS_SLOT_5M);
				wlc_bmac_write_shm(wlc_hw, M_DOT11_SLOT, 4*APHY_SLOT_TIME);
			}
		}
#endif /* WL11ULB */
	} else {
		/* 11g long slot: 11b timing */
		W_REG(osh, &regs->u.d11regs.ifs_slot, 0x0212);	/* BPHY_SLOT_TIME */
		wlc_bmac_write_shm(wlc_hw, M_DOT11_SLOT, BPHY_SLOT_TIME);
#ifdef WL11ULB
		if (ULB_ENAB(wlc_hw->wlc->pub)) {
			if (CHSPEC_IS10(wlc_hw->chanspec)) {
				W_REG(osh, &regs->u.d11regs.ifs_slot, IFS_SLOT_BPHY_10M);
				wlc_bmac_write_shm(wlc_hw, M_DOT11_SLOT, 2*BPHY_SLOT_TIME);
			} else if (CHSPEC_IS5(wlc_hw->chanspec)) {
				W_REG(osh, &regs->u.d11regs.ifs_slot, IFS_SLOT_BPHY_5M);
				wlc_bmac_write_shm(wlc_hw, M_DOT11_SLOT, 4*BPHY_SLOT_TIME);
			}
		}
#endif /* WL11ULB */
	}
}

/* Helper functions for full ROM chips */

#if !defined(BCMUCDOWNLOAD) && !defined(BCM_OL_DEV)
static CONST d11init_t*
WLBANDINITFN(wlc_get_n19initvals34_addr)(void)
{
	return (d11n19initvals34);
}

static CONST d11init_t*
WLBANDINITFN(wlc_get_n18initvals32_addr)(void)
{
	return (d11n18initvals32);
}
#endif /* !BCMUCDOWNLOAD && !BCM_OL_DEV */

static CONST d11init_t*
WLBANDINITFN(wlc_get_n19bsinitvals34_addr)(void)
{
	return (d11n19bsinitvals34);
}

static CONST d11init_t*
WLBANDINITFN(wlc_get_n18bsinitvals32_addr)(void)
{
	return (d11n18bsinitvals32);
}

static CONST d11init_t*
WLBANDINITFN(wlc_get_lcn0bsinitvals24_addr)(void)
{
	return (d11lcn0bsinitvals24);
}

static CONST d11init_t*
WLBANDINITFN(wlc_get_lcn0bsinitvals25_addr)(void)
{
	return (d11lcn0bsinitvals25);
}

static CONST d11init_t*
WLBANDINITFN(wlc_get_d11lcn400bsinitvals33_addr)(void)
{
	return (d11lcn400bsinitvals33);
}

static CONST d11init_t*
WLBANDINITFN(wlc_get_n22bsinitvals31_addr)(void)
{
	return (d11n22bsinitvals31);
}

static CONST d11init_t*
WLBANDINITFN(wlc_get_d11lcn406bsinitvals37_addr)(void)
{
	return (d11lcn406bsinitvals37);
}

static CONST d11init_t*
WLBANDINITFN(wlc_get_d11lcn407bsinitvals38_addr)(void)
{
	return (d11lcn407bsinitvals38);
}

#if !defined(BCMUCDOWNLOAD) && !defined(BCM_OL_DEV)
static CONST d11init_t*
WLBANDINITFN(wlc_get_lcn0initvals24_addr)(void)
{
	return (d11lcn0initvals24);
}

static CONST d11init_t*
WLBANDINITFN(wlc_get_lcn0initvals25_addr)(void)
{
	return (d11lcn0initvals25);
}

static CONST d11init_t*
WLBANDINITFN(wlc_get_d11lcn400initvals33_addr)(void)
{
	return (d11lcn400initvals33);
}

static CONST d11init_t*
WLBANDINITFN(wlc_get_n22initvals31_addr)(void)
{
	return (d11n22initvals31);
}

static CONST d11init_t*
WLBANDINITFN(wlc_get_d11lcn406initvals37_addr)(void)
{
	return (d11lcn406initvals37);
}

static CONST d11init_t*
WLBANDINITFN(wlc_get_d11lcn407initvals38_addr)(void)
{
	return (d11lcn407initvals38);
}

#endif /* BCMUCDOWNLOAD */

/**
 * Band can change as a result of a 'wl up', band request from a higher layer or VSDB related. In
 * such a case, the ucode has to be provided with new band initialization values.
 */
static void
WLBANDINITFN(wlc_ucode_bsinit)(wlc_hw_info_t *wlc_hw)
{
#if defined(MBSS)
	bool ucode9 = TRUE;
	(void)ucode9;
#endif // endif

	/* init microcode host flags */
	wlc_write_mhf(wlc_hw, wlc_hw->band->mhfs);

	/* do band-specific ucode IHR, SHM, and SCR inits */
#ifdef WLRSDB
	if (wlc_bmac_rsdb_cap(wlc_hw))  {

	   wlc_bmac_rsdb_write_inits(wlc_hw, d11ac12bsinitvals50, d11ac12bsinitvals50core1);

	} else
#endif /* WLRSDB */
	if (D11REV_IS(wlc_hw->corerev, 65)) {
		if (WLCISACPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, d11ac33bsinitvals65);
			wlc_write_inits(wlc_hw, d11ac33bsinitvalsx65);
		} else {
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
				__FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
		}
	} else if (D11REV_IS(wlc_hw->corerev, 64)) {
		if (WLCISACPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, d11ac32bsinitvals64);
			wlc_write_inits(wlc_hw, d11ac32bsinitvalsx64);
		} else {
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
				__FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
		}
	} else if (D11REV_IS(wlc_hw->corerev, 49)) {
		if (WLCISACPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, d11ac9bsinitvals49);
		} else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
				__FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
	} else if (D11REV_IS(wlc_hw->corerev, 48)) {
		if (WLCISACPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, d11ac8bsinitvals48);
		} else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
				__FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
	} else if (D11REV_IS(wlc_hw->corerev, 45) || D11REV_IS(wlc_hw->corerev, 47) ||
	           D11REV_IS(wlc_hw->corerev, 51) || D11REV_IS(wlc_hw->corerev, 52)) {
		if (WLCISACPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, d11ac7bsinitvals47);
		} else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
				__FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
	} else if (D11REV_IS(wlc_hw->corerev, 46)) {
		if (WLCISACPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, d11ac6bsinitvals46);
		} else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
				__FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
	} else if (D11REV_IS(wlc_hw->corerev, 43)) {
		if (WLCISACPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, d11ac3bsinitvals43);
		} else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
				__FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
	} else if (D11REV_IS(wlc_hw->corerev, 42)) {
		if (WLCISACPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, d11ac1bsinitvals42);
		} else {
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
				__FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
		}
	} else if (D11REV_IS(wlc_hw->corerev, 41) || D11REV_IS(wlc_hw->corerev, 44)) {
		if (WLCISACPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, d11ac2bsinitvals41);
		} else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
				__FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
	} else  if (D11REV_IS(wlc_hw->corerev, 40)) {
		if (WLCISACPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, d11ac0bsinitvals40);
		} else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
				__FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
	} else if (D11REV_IS(wlc_hw->corerev, 38)) {
		if (WLCISLCN40PHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, wlc_get_d11lcn407bsinitvals38_addr());
		} else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
				__FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
	} else if (D11REV_IS(wlc_hw->corerev, 37)) {
		if (WLCISLCN40PHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, wlc_get_d11lcn406bsinitvals37_addr());
		} else if (WLCISNPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, wlc_get_n22bsinitvals31_addr());
		} else {
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
				__FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
		}
	} else if (D11REV_IS(wlc_hw->corerev, 34)) {
		if (WLCISNPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, wlc_get_n19bsinitvals34_addr());
		} else {
			WL_ERROR(("%s: wl%d: unsupported phy in corerev 34\n",
				__FUNCTION__, wlc_hw->unit));
		}
	} else if (D11REV_IS(wlc_hw->corerev, 33)) {
		if (WLCISLCN40PHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, wlc_get_d11lcn400bsinitvals33_addr());
		} else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
				__FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
	} else if (D11REV_IS(wlc_hw->corerev, 32)) {
		if (WLCISNPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, wlc_get_n18bsinitvals32_addr());
		} else {
			WL_ERROR(("%s: wl%d: unsupported phy in corerev 32\n",
				__FUNCTION__, wlc_hw->unit));
		}
	} else if (D11REV_IS(wlc_hw->corerev, 31)) {
		if (WLCISNPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, d11ht0bsinitvals29);
		} else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
				__FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
	} else if (D11REV_IS(wlc_hw->corerev, 30)) {
		if (WLCISNPHY(wlc_hw->band))
			wlc_write_inits(wlc_hw, d11n16bsinitvals30);
	} else if (D11REV_IS(wlc_hw->corerev, 29)) {
		if (WLCISHTPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, d11ht0bsinitvals29);
		} else {
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
				__FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
		}
	} else if (D11REV_IS(wlc_hw->corerev, 26)) {
		if (WLCISHTPHY(wlc_hw->band)) {
			if (D11REV_IS(wlc_hw->corerev, 26))
				wlc_write_inits(wlc_hw, d11ht0bsinitvals26);
			else if (D11REV_IS(wlc_hw->corerev, 29))
				wlc_write_inits(wlc_hw, d11ht0bsinitvals29);
		} else {
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
				__FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
		}
	} else if (D11REV_IS(wlc_hw->corerev, 25) || D11REV_IS(wlc_hw->corerev, 28)) {
		if (WLCISNPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, d11n0bsinitvals25);
		} else if (WLCISLCNPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, wlc_get_lcn0bsinitvals25_addr());
		} else {
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
				__FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
		}
	} else if (D11REV_IS(wlc_hw->corerev, 24)) {
		if (WLCISNPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, d11n0bsinitvals24);
		} else if (WLCISLCNPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, wlc_get_lcn0bsinitvals24_addr());
		} else {
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
				__FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
		}
	} else if (D11REV_GE(wlc_hw->corerev, 22)) {
		if (WLCISNPHY(wlc_hw->band)) {
			/* ucode only supports rev23(43224b0) with rev16 ucode */
			if (D11REV_IS(wlc_hw->corerev, 23))
				wlc_write_inits(wlc_hw, d11n0bsinitvals16);
			else
				wlc_write_inits(wlc_hw, d11n0bsinitvals22);
		} else if (WLCISSSLPNPHY(wlc_hw->band)) {
			if (D11REV_IS(wlc_hw->corerev, 22))
				wlc_write_inits(wlc_hw, d11sslpn4bsinitvals22);
			else
				WL_ERROR(("wl%d: unsupported phy in corerev 16\n", wlc_hw->unit));
		}
		else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
				__FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
	} else if (D11REV_IS(wlc_hw->corerev, 21)) {
		if (WLCISSSLPNPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, d11sslpn3bsinitvals21);
		}
		else
			WL_ERROR(("wl%d: unsupported phy in corerev 21\n", wlc_hw->unit));
	} else if (D11REV_IS(wlc_hw->corerev, 20) && WLCISSSLPNPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, d11sslpn1bsinitvals20);
	} else if (D11REV_GE(wlc_hw->corerev, 16)) {
		if (WLCISNPHY(wlc_hw->band))
			wlc_write_inits(wlc_hw, d11n0bsinitvals16);
		else if (WLCISSSLPNPHY(wlc_hw->band))
			wlc_write_inits(wlc_hw, d11sslpn0bsinitvals16);
		else if (WLCISLPPHY(wlc_hw->band))
			wlc_write_inits(wlc_hw, d11lp0bsinitvals16);
		else
			WL_ERROR(("wl%d: unsupported phy in corerev 16\n", wlc_hw->unit));
	} else if (D11REV_IS(wlc_hw->corerev, 15)) {
		if (WLCISLPPHY(wlc_hw->band))
			wlc_write_inits(wlc_hw, d11lp0bsinitvals15);
		else
			WL_ERROR(("wl%d: unsupported phy in corerev 15\n", wlc_hw->unit));
	} else if (D11REV_IS(wlc_hw->corerev, 14)) {
		if (WLCISLPPHY(wlc_hw->band))
			wlc_write_inits(wlc_hw, d11lp0bsinitvals14);
		else
			WL_ERROR(("wl%d: unsupported phy in corerev 14\n", wlc_hw->unit));
	} else if (D11REV_IS(wlc_hw->corerev, 13)) {
		if (WLCISLPPHY(wlc_hw->band))
			wlc_write_inits(wlc_hw, d11lp0bsinitvals13);
		else if (WLCISGPHY(wlc_hw->band))
			wlc_write_inits(wlc_hw, d11b0g0bsinitvals13);
		else if (WLCISAPHY(wlc_hw->band) &&
			(si_core_sflags(wlc_hw->sih, 0, 0) & SISF_2G_PHY))
			wlc_write_inits(wlc_hw, d11a0g1bsinitvals13);
		else
			WL_ERROR(("wl%d: unsupported phy in corerev 13\n", wlc_hw->unit));
	} else if (D11REV_GE(wlc_hw->corerev, 11)) {
		if (WLCISNPHY(wlc_hw->band))
			wlc_write_inits(wlc_hw, d11n0bsinitvals11);
		else
			WL_ERROR(("wl%d: corerev >= 11 && ! NPHY\n", wlc_hw->unit));
#if defined(MBSS)
	} else if (D11REV_IS(wlc_hw->corerev, 9) && ucode9) {
		/* Only ucode for corerev 9 has this support */
		if (WLCISAPHY(wlc_hw->band)) {
			if (si_core_sflags(wlc_hw->sih, 0, 0) & SISF_2G_PHY)
				wlc_write_inits(wlc_hw, d11a0g1bsinitvals9);
			else
				wlc_write_inits(wlc_hw, d11a0g0bsinitvals9);
		} else
			wlc_write_inits(wlc_hw, d11b0g0bsinitvals9);
#endif // endif
	} else if (D11REV_GE(wlc_hw->corerev, 5)) {
		if (WLCISAPHY(wlc_hw->band)) {
			if (si_core_sflags(wlc_hw->sih, 0, 0) & SISF_2G_PHY)
				wlc_write_inits(wlc_hw, d11a0g1bsinitvals5);
			else
				wlc_write_inits(wlc_hw, d11a0g0bsinitvals5);
		} else
			wlc_write_inits(wlc_hw, d11b0g0bsinitvals5);
	} else if (D11REV_IS(wlc_hw->corerev, 4)) {
		if (WLCISAPHY(wlc_hw->band))
			wlc_write_inits(wlc_hw, d11a0g0bsinitvals4);
		else
			wlc_write_inits(wlc_hw, d11b0g0bsinitvals4);
	} else
		WL_ERROR(("wl%d: %s: corerev %d is invalid", wlc_hw->unit,
			__FUNCTION__, wlc_hw->corerev));
}

/** switch to new band but leave it inactive */
static uint32
WLBANDINITFN(wlc_setband_inact)(wlc_hw_info_t *wlc_hw, uint bandunit)
{
	wlc_info_t *wlc = wlc_hw->wlc;
	uint32 macintmask;
	uint32 tmp;

	WL_TRACE(("wl%d: wlc_setband_inact\n", wlc_hw->unit));

	ASSERT(bandunit != wlc_hw->band->bandunit);
	ASSERT(si_iscoreup(wlc_hw->sih));
	ASSERT((R_REG(wlc_hw->osh, &wlc_hw->regs->maccontrol) & MCTL_EN_MAC) == 0);

	/* disable interrupts */
	macintmask = wl_intrsoff(wlc->wl);

	/* radio off -- NPHY radios don't require to be turned off and on on a band switch */
	phy_radio_xband((phy_info_t *)wlc_hw->band->pi);

#if defined(WLC_HIGH) && defined(BCMNODOWN)
	/* This is not necessary in BCMNODOWN for either
	 * single- or dual-band, but just set it anyway
	 * to sync up with radio HW state.
	 */
	wlc->pub->radio_active = OFF;
#endif // endif

	ASSERT(wlc_hw->clk);
#ifndef EFI
	PANIC_CHECK_CLK(wlc_hw->clk, "wl%d: %s: NO CLK\n", wlc_hw->unit, __FUNCTION__);
#endif // endif

	if (D11REV_LT(wlc_hw->corerev, 17)) {
		tmp = R_REG(wlc_hw->osh, &wlc_hw->regs->maccontrol);
		BCM_REFERENCE(tmp);
	}

	if (!(WLCISACPHY(wlc_hw->band)))
		wlc_bmac_core_phy_clk(wlc_hw, OFF);

	wlc_setxband(wlc_hw, bandunit);

	return (macintmask);
}

/** related to older (obsolete?) PHY's */
static void
WLBANDINITFN(wlc_bmac_core_phyclk_abg_switch)(wlc_hw_info_t *wlc_hw)
{
	uint cnt = 0;
	uint32 fsbit = WLCISAPHY(wlc_hw->band) ? SICF_FREF : 0;

	/* toggle SICF_FREF */

	ASSERT(si_core_sflags(wlc_hw->sih, 0, 0) & SISF_FCLKA);
	while (cnt++ < 5) {
		/* PR47223 : WAR for PCMCIA corerev 9 :
		 * readback after write operation on control flags
		 * is needed to ensure write completion.
		 * But immediate read after flipping SICF_FREF bit
		 * causes the backplane timeout; hence the following sequence of write
		 * -> delay -> readback.
		 */
		if ((BUSTYPE(wlc_hw->sih->bustype) == PCMCIA_BUS) &&
			(D11REV_IS(wlc_hw->corerev, 9))) {
			si_core_cflags_wo(wlc_hw->sih, SICF_FREF, fsbit);
			OSL_DELAY(2 * MIN_SLOW_CLK); /* 2 slow clock tick (worst case 32khz) */
			/* readback to ensure write completion */
			si_core_cflags(wlc_hw->sih, 0, 0);
		} else {
			si_core_cflags(wlc_hw->sih, SICF_FREF, fsbit);
			OSL_DELAY(2 * MIN_SLOW_CLK); /* 2 slow clock tick (worst case 32khz) */
		}
		SPINWAIT(!(si_core_sflags(wlc_hw->sih, 0, 0) & SISF_FCLKA), 2 * FREF_DELAY);

		if (D11REV_GE(wlc_hw->corerev, 8) ||
			(si_core_sflags(wlc_hw->sih, 0, 0) & SISF_FCLKA))
			break;

		si_core_cflags(wlc_hw->sih, SICF_FREF, WLCISAPHY(wlc_hw->band) ? 0 : SICF_FREF);
		OSL_DELAY(FREF_DELAY);
	}
	ASSERT(si_core_sflags(wlc_hw->sih, 0, 0) & SISF_FCLKA);
}

#ifdef WL_MU_RX
/* hacking Signal Tail field in VHT-SIG-A2 to save nssand mcs,
 * nss = bit 18:19 (for 11ac 2 bits to indicate maximum 4 nss)
 * mcs = 20:23
 */
void BCMFASTPATH
wlc_bmac_upd_murate(wlc_info_t *wlc, d11rxhdr_t *rxhdr, uchar *plcp)
{
	uint32 plcp0;
	uint8 gid;

	/* Check AC rate */
	if ((rxhdr->PhyRxStatus_0 & PRXS0_FT_MASK) != PRXS0_STDN)
		return;

	if ((plcp[0] | plcp[1] | plcp[2])) {
		plcp0 = plcp[0] | (plcp[1] << 8);
		gid = (plcp0 & VHT_SIGA1_GID_MASK) >> VHT_SIGA1_GID_SHIFT;
		if ((gid > VHT_SIGA1_GID_TO_AP) &&
			(gid < VHT_SIGA1_GID_NOT_TO_AP)) {
			ASSERT(((rxhdr->MuRate >> 4) & 0x7) >= 1);
			ASSERT(((rxhdr->MuRate >> 4) & 0x7) <= 4);
			plcp[5] &= ~0xFC;
			plcp[5] |= (((rxhdr->MuRate >> 4) & 0x3) - 1) << 2;
			plcp[5] |= (rxhdr->MuRate & 0xf) << 4;
		}
	}
}

static void BCMFASTPATH
wlc_bmac_save_murate2plcp(wlc_info_t *wlc, d11rxhdr_t *rxhdr, void *p)
{
	uchar *plcp;

	/* Skip amsdu until deaggration completed */
	if (rxhdr->dma_flags & RXS_SHORT_MASK)
		return;

	if ((rxhdr->RxStatus2 & RXS_AMSDU_MASK) != 0)
		return;

	/* Hack murx rate */
	plcp = (uchar *)(PKTDATA(wlc->osh, p) + wlc->hwrxoff);
	if (rxhdr->RxStatus1 & RXS_PBPRES)
		plcp += 2;

	wlc_bmac_upd_murate(wlc, rxhdr, plcp);
}
#endif /* WL_MU_RX */

/**
 * Called as a result of a hardware event: when the D11 core signals one or more received frames
 * on its RX FIFO(s). The received frames are then processed by firmware/driver.
 *
 * Return TRUE if more frames need to be processed. FALSE otherwise.
 * Param 'bound' indicates max. # frames to process before break out.
 */
bool BCMFASTPATH
wlc_bmac_recv(wlc_hw_info_t *wlc_hw, uint fifo, bool bound, wlc_dpc_info_t *dpc)
{
	void *p;
	void *head = NULL;
	void *tail = NULL;
	uint n = 0;
	uint32 tsf_l;
	wlc_d11rxhdr_t *wlc_rxhdr = NULL;
#if defined(PKTC) || defined(PKTC_DONGLE)
	uint index = 0;
	void *head0 = NULL;
	bool one_chain = PKTC_ENAB(wlc_hw->wlc->pub);
	uint bound_limit = bound ? wlc_hw->wlc->pub->tunables->pktcbnd : -1;
#else
	uint bound_limit = bound ? wlc_hw->wlc->pub->tunables->rxbnd : -1;
#endif // endif
#if defined(IL_BIGENDIAN) && !defined(WLC_SPLIT)
	uint rxh_len;
#endif // endif
	uint16 rxchan;        /* receive channel in host byte order */
	uint16 phy_rx_stat0;   /* PhyRxStatus_0 in host byte order */

#ifdef RXDMA_STUCK_WAR
	wlc_info_t *wlc = wlc_hw->wlc;
#endif // endif

#if defined(WLC_LOW_ONLY)
	if (!POOL_ENAB(wlc_hw->wlc->pub->pktpool)) {
		uint32 mem_avail;
		/* always bounded for bmac */
		bound_limit = wlc_hw->wlc->pub->tunables->rxbnd;
		/* check how many rxbufs we can afford to refill */
		mem_avail = OSL_MEM_AVAIL();
		if (mem_avail < wlc_hw->mem_required_def) {
			if (mem_avail > wlc_hw->mem_required_lower) {
				bound_limit = bound_limit - (bound_limit >> 2);
			}
			else if (mem_avail > wlc_hw->mem_required_least) {
				bound_limit = (bound_limit >> 1);
			} else { /* memory pool is stresed,  come back later */
				return TRUE;
			}
		}
	} else {
		uint pktpool_len;
		/* always bounded for bmac */
		bound_limit = wlc_hw->wlc->pub->tunables->rxbnd;

		pktpool_len = (uint) pktpool_avail(wlc_hw->wlc->pub->pktpool);
		if (bound_limit > pktpool_len)
		{
			bound_limit = (pktpool_len - (pktpool_len >> 2));

			/* If bound_limit comes out as 0, set it to atlesat 1 so
			 * that return value (n >= bound_limit ) is not always true.
			 */
			if (bound_limit == 0) {
				bound_limit = 1;
			}
		}

	}

#endif /* defined(WLC_LOW_ONLY) */

	ASSERT(bound_limit > 0);

	WL_TRACE(("wl%d: %s\n", wlc_hw->unit, __FUNCTION__));
	/* gather received frames */
	while (1) {
#ifdef	WL_RXEARLYRC
		if (wlc_hw->rc_pkt_head != NULL) {
			p = wlc_hw->rc_pkt_head;
			wlc_hw->rc_pkt_head = PKTLINK(p);
			PKTSETLINK(p, NULL);
		} else
#endif // endif
		if ((p = (PIO_ENAB_HW(wlc_hw) ?
			wlc_pio_rx(wlc_hw->pio[fifo]) : dma_rx(wlc_hw->di[fifo]))) == NULL)
			break;
#if defined(BCMPCIEDEV)
		/* For fifo-split rx , fifo-0/1 has to be synced up */
		if (BCMPCIEDEV_ENAB() && RXFIFO_SPLIT() &&
		    PKTISRXFRAG(wlc_hw->osh, p)) {
			if (!wlc_bmac_process_split_fifo_pkt(wlc_hw, fifo, p))
				continue;
			}
#endif /*  defined(BCMPCIEDEV) */

#if defined(BCM47XX_CA9)
		DMA_MAP(wlc_hw->osh, PKTDATA(wlc_hw->osh, p),
			PKTLEN(wlc_hw->osh, p), DMA_RX, p, NULL);
#endif /* defined(BCM47XX_CA9) */

		/* record the rxfifo in wlc_rxd11hdr */
		wlc_rxhdr = (wlc_d11rxhdr_t *)PKTDATA(wlc_hw->osh, p);
		wlc_rxhdr->rxhdr.fifo = (uint8)fifo;

#if defined(IL_BIGENDIAN) && !defined(WLC_SPLIT)
		/* If driver is not split, this code runs on the same processor as the rest
		 * of the driver. Convert receive status from little endian to host endian.
		 * On split driver, this code runs on the on chip processor and we don't know
		 * here what type of processor is on the host. On split driver, endian conversion
		 * is done in wlc_recv().
		 */
		if ((D11REV_GE(wlc_hw->wlc->pub->corerev, 64)) &&
		    (wlc_rxhdr->rxhdr.dma_flags & RXS_SHORT_MASK)) {
			/* short rx status received */
			rxh_len = SRXHDR_LEN;
		} else {
			/* long rx status received */
			rxh_len = RXHDR_LEN;
		}
		BCM_REFERENCE(rxh_len);  /* needed when next line turns out to be no-op */
		ltoh16_buf((void*)&wlc_rxhdr->rxhdr, rxh_len);

		/* Keep 8-bit fifo and dma_flags fields in original order */
		ltoh16_buf((void*) &wlc_rxhdr->rxhdr.dma_flags, 2);
#endif /* IL_BIGENDIAN && !WLC_SPLIT */

#if defined(WL_MU_RX)
		if (MU_RX_ENAB(wlc_hw->wlc))
			wlc_bmac_save_murate2plcp(wlc_hw->wlc, &wlc_rxhdr->rxhdr, p);
#endif /* WL_MU_RX */

#if defined(PKTC) || defined(PKTC_DONGLE)
		/* if management pkt  or data in fifo-1 skip chainbale checks */
		if (BCMSPLITRX_ENAB() && (fifo == PKT_CLASSIFY_FIFO)) {
			one_chain = FALSE;
		}
		/* Chaining is intentionally disabled in monitor mode */
		if (MONITOR_ENAB(wlc_hw->wlc)) {
			one_chain = FALSE;
		}

		ASSERT(PKTCLINK(p) == NULL);
		/* if current frame hits the hot bridge cache entry, and if it
		 * belongs to the burst received from same source and going to
		 * same destination then it is a candidate for chained sendup.
		 */

		if (one_chain && !wlc_rxframe_chainable(wlc_hw->wlc, &p, index)) {
			one_chain = FALSE;
			/* breaking chain from here, first half of burst can
			 * be sent up as one. frames in the other half are
			 * sent up individually.
			 */
			if (tail != NULL) {
				head0 = head;
				tail = NULL;
			}
		}

		if (p != NULL) {
			index++;
			PKTCENQTAIL(head, tail, p);
		}
#else /* PKTC || PKTC_DONGLE */
		if (!tail)
			head = tail = p;
		else {
			PKTSETLINK(tail, p);
			tail = p;
		}
#endif /* PKTC || PKTC_DONGLE */

#ifdef BCMDBG_POOL
		if (p != NULL)
			PKTPOOLSETSTATE(p, POOL_RXD11);
#endif // endif

		/* !give others some time to run! */
		if (++n >= bound_limit)
			break;
	}

#ifdef RXDMA_STUCK_WAR
	if (fifo == RX_FIFO2) {
		wlc->pub->rxfifo2_cnt += n;
	}
#endif // endif

	/* post more rbufs */
	if (!PIO_ENAB_HW(wlc_hw)) {
#ifdef DBG_RXDMA_STUCK_WAR
		if (fifo == RX_FIFO) {
			dma_rxfill(wlc_hw->di[fifo]);
		} else if (!wlc->pub->trig_rxstuck) {
			dma_rxfill(wlc_hw->di[fifo]);
		}
#else
		dma_rxfill(wlc_hw->di[fifo]);
#endif // endif
	}
#if defined(PKTC) || defined(PKTC_DONGLE)
	/* see if the chain is broken */
	if (head0 != NULL) {
		WL_TRACE(("%s: partial chain %p\n", __FUNCTION__, head0));
		wlc_sendup_chain(wlc_hw->wlc, head0);
	} else {
		if (one_chain && (head != NULL)) {
			/* send up burst in one shot */
			WL_TRACE(("%s: full chain %p sz %d\n", __FUNCTION__, head, n));
			wlc_sendup_chain(wlc_hw->wlc, head);
			dpc->processed += n;
			return (n >= bound_limit);
		}
	}
#endif // endif

	/* prefetch the headers */
	if (head != NULL) {
		WLPREFHDRS(PKTDATA(wlc_hw->osh, head), PREFSZ);
	}

	/* get the TSF REG reading */
	wlc_bmac_read_tsf(wlc_hw, &tsf_l, NULL);

	/* process each frame */
	while ((p = head) != NULL) {
#if defined(PKTC) || defined(PKTC_DONGLE)
		head = PKTCLINK(head);
		PKTSETCLINK(p, NULL);
		WLCNTINCR(wlc_hw->wlc->pub->_cnt->unchained);
#else
		head = PKTLINK(head);
		PKTSETLINK(p, NULL);
#endif // endif

		/* prefetch the headers */
		if (head != NULL) {
			WLPREFHDRS(PKTDATA(wlc_hw->osh, head), PREFSZ);
		}

		/* record the tsf_l in wlc_rxd11hdr */
		wlc_rxhdr = (wlc_d11rxhdr_t *)PKTDATA(wlc_hw->osh, p);
#ifdef WLC_SPLIT
		/* On split driver, write tsf in little endian. Host processor
		 * will convert if necessary. rx status is still in little endian.
		 */
		wlc_rxhdr->tsf_l = htol32(tsf_l);
		rxchan = ltoh16(wlc_rxhdr->rxhdr.RxChan);
		phy_rx_stat0 = ltoh16(wlc_rxhdr->rxhdr.PhyRxStatus_0);
#else
		/* On monolithic driver, write tsf in host byte order. rx status already
		 * in host byte order.
		 */
		wlc_rxhdr->tsf_l = tsf_l;
		rxchan = wlc_rxhdr->rxhdr.RxChan;
		phy_rx_stat0 = wlc_rxhdr->rxhdr.PhyRxStatus_0;
#endif // endif

		/* compute the RSSI from d11rxhdr and record it in wlc_rxd11hr */
		phy_rssi_compute_rssi((phy_info_t *)wlc_hw->band->pi, wlc_rxhdr);

		/* Convert the RxChan to a chanspec for pre-rev40 devices
		 * The chanspec will not have sideband info on this conversion.
		 */
		if (D11REV_LT(wlc_hw->corerev, 40)) {
			wlc_rxhdr->rxhdr.RxChan = (
				/* channel */
				((rxchan & RXS_CHAN_ID_MASK) >> RXS_CHAN_ID_SHIFT) |
				/* band */
				((rxchan & RXS_CHAN_5G) ? WL_CHANSPEC_BAND_5G :
					WL_CHANSPEC_BAND_2G) |
				/* bw */
				((rxchan & RXS_CHAN_40) ? WL_CHANSPEC_BW_40 :
					WL_CHANSPEC_BW_20) |
				/* bogus sideband */
				WL_CHANSPEC_CTL_SB_L);
#ifdef WLC_SPLIT
			/* If split driver, receive status stays in little endian here */
			wlc_rxhdr->rxhdr.RxChan = htol16(wlc_rxhdr->rxhdr.RxChan);
#endif // endif
		}

		/* Set the FT value for pre-11n phys */
		if (WLCISAPHY(wlc_hw->band) ||
			WLCISGPHY(wlc_hw->band) ||
			WLCISLPPHY(wlc_hw->band)) {
			uint16 phy_ft;
			uint16 ft_mask;

			/* The GPHY and LPPHY only set bit 0 of the phyrxstatus0 word to indicate
			 * OFDM or CCK. The APHY does not set a bit.
			 * For this set of phys, CCK and OFDM are the only options, so
			 * check the RxChan band for 5G and set OFDM, or if 2G, check
			 * bit 0, PRXS0_OFDM, to differentiate CCK/OFDM
			 */
			if (CHSPEC_IS5G(rxchan) || (phy_rx_stat0 & PRXS0_OFDM)) {
				phy_ft = PRXS0_OFDM;
			} else {
				phy_ft = PRXS0_CCK;
			}

			/* clear the FT field and update with new value */
			ft_mask = PRXS0_FT_MASK;
#ifdef WLC_SPLIT
			/* rx status still in little endian */
			ft_mask = htol16(ft_mask);
			phy_ft = htol16(phy_ft);
#endif // endif
			wlc_rxhdr->rxhdr.PhyRxStatus_0 &= ~ft_mask;
			wlc_rxhdr->rxhdr.PhyRxStatus_0 |= phy_ft;
		}

		wlc_recv(wlc_hw->wlc, p);
	}

	dpc->processed += n;

	return (n >= bound_limit);
}

#ifdef WLP2P_UCODE
/** ucode generates p2p specific interrupts. Low level p2p interrupt processing */
void
wlc_p2p_bmac_int_proc(wlc_hw_info_t *wlc_hw)
{
	uint b, i;
	uint8 p2p_interrupts[M_P2P_BSS_MAX];
	uint32 tsf_l, tsf_h;

	ASSERT(DL_P2P_UC(wlc_hw));
	ASSERT(wlc_hw->p2p_shm_base != (uint)~0);

	memset(p2p_interrupts, 0, sizeof(uint8) * M_P2P_BSS_MAX);

	/* collect and clear p2p interrupts */
	for (b = 0; b < M_P2P_BSS_MAX; b ++) {

		for (i = 0; i < M_P2P_I_BLK_SZ; i ++) {
			uint loc = wlc_hw->p2p_shm_base + M_P2P_I(b, i);

			/* any P2P event/interrupt? */
			if (wlc_bmac_read_shm(wlc_hw, loc) == 0)
				continue;

			/* ACK */
			wlc_bmac_write_shm(wlc_hw, loc, 0);

			/* store */
			p2p_interrupts[b] |= (1 << i);
#ifdef BCMDBG
			/* Update p2p Interrupt stats. */
			wlc_hw->wlc->perf_stats.isr_stats.n_p2p[i]++;
#endif // endif
		}
	}

	wlc_bmac_read_tsf(wlc_hw, &tsf_l, &tsf_h);
#ifdef WLAWDL
	if (AWDL_ENAB(wlc_hw->wlc->pub))
		wlc_awdl_int_proc(wlc_hw->wlc, p2p_interrupts, tsf_l, tsf_h);
#endif // endif
	wlc_p2p_int_proc(wlc_hw->wlc, p2p_interrupts, tsf_l, tsf_h);
}
#endif /* WLP2P_UCODE */

#ifdef AP	/** PMQ (power management queue) stuff */
#if defined(WLC_LOW_ONLY)
/**
 * tries to find an entry for ea. If none and "add" is true,
 * add and initialize the new
 */
static  bmac_pmq_entry_t *
wlc_bmac_pmq_find(wlc_hw_info_t *wlc_hw, struct ether_addr *ea)
{
	bmac_pmq_entry_t * res = wlc_hw->bmac_pmq->entry;

	/* There should not be THAT many stations in PS ON mode which needs
	   tx pending drain at the same time.
	   do a simple search for now. We will see if we need to optimize it later.
	*/
	if (res == NULL)
		return res;
	do {
		if (!memcmp(&res->ea, ea, sizeof(struct ether_addr))) {
			/* move the head to the entry we found. For high
			   traffic, this is the most likely to comme next.
			 */
			wlc_hw->bmac_pmq->entry = res;
			return res;
		}
		res = res->next;
	} while (res && res != wlc_hw->bmac_pmq->entry);

	/* If we are here, we didn't find it */
	ASSERT(res);

	return NULL;
}

#ifndef TXQ_MUX
static  bmac_pmq_entry_t *
wlc_bmac_pmq_add(wlc_hw_info_t *wlc_hw, struct ether_addr *ea)
{
	bmac_pmq_entry_t * res;

	res = (bmac_pmq_entry_t *)MALLOC(wlc_hw->osh, sizeof(bmac_pmq_t));
	if (!res) {
		WL_ERROR(("wl%d: BPMQ error. Out of memory !!\n",
			wlc_hw->unit));
		return NULL;
	}
	if (! wlc_hw->bmac_pmq->entry) {
		wlc_hw->bmac_pmq->entry = res;
		res->next = res;
	}
	else {
		res->next = wlc_hw->bmac_pmq->entry->next;
		wlc_hw->bmac_pmq->entry->next = res;
		wlc_hw->bmac_pmq->entry = res;
	}
	memcpy(&res->ea, ea, sizeof(struct ether_addr));
	res->switches = 0;
	res->ps_on = 0;
	return res;
}
#endif /* TXQ_MUX */

static void
wlc_bmac_pmq_remove(wlc_hw_info_t *wlc_hw, bmac_pmq_entry_t *pmq_entry)
{
	bmac_pmq_entry_t * res = wlc_hw->bmac_pmq->entry;
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN];
#endif // endif
	/* There should not be THAT many stations in PS ON mode which needs
	 *  tx pending drain at the same time.
	 * do a simple search for now. We will see if we need to optimize it later.
	*/

	ASSERT(res);

	while (res && res->next != pmq_entry && res->next != wlc_hw->bmac_pmq->entry) {
		res = res->next;
	}

	ASSERT(res->next == pmq_entry);
	/* last element */
	if (res == res->next)
		wlc_hw->bmac_pmq->entry = NULL;
	else {
		res->next = pmq_entry->next;
		if (pmq_entry == wlc_hw->bmac_pmq->entry)
			wlc_hw->bmac_pmq->entry = res;
	}

	WL_PS((" BPMQ removed %s from table \n", bcm_ether_ntoa(&pmq_entry->ea, eabuf)));
	MFREE(wlc_hw->osh, pmq_entry, sizeof(bmac_pmq_entry_t));

}

#endif /* WLC_LOW_ONLY */

#ifdef WL_AUXPMQ
static uint16 BCMFASTPATH
wlc_bmac_auxpmq_add(wlc_hw_info_t *wlc_hw, struct ether_addr *ea)
{
	bmac_pmq_t *bmac_pmq = wlc_hw->bmac_pmq;
	bmac_auxpmq_entry_t *add;
	uint16 idx;

	for (idx = 0; idx < AUXPMQ_ENTRIES; ++idx) {
		if (!isset(bmac_pmq->auxpmq_used, idx))
			break;
	}

	if (idx == AUXPMQ_ENTRIES) {
		WL_NONE(("wl%d: Add AuxPMQ is full !!\n", wlc_hw->unit));
		idx = AUXPMQ_INVALID_IDX;
		bmac_pmq->auxpmq_full_cnt++;
		bmac_pmq->auxpmq_full = 1;
		goto done;
	}

	add = &(bmac_pmq->auxpmq_list[idx]);

	setbit(bmac_pmq->auxpmq_used, idx);

	bmac_pmq->auxpmq_entry_cnt++;

	memcpy(&add->ea, ea, sizeof(struct ether_addr));

	WL_PS(("Add "MACF" to AuxPMQ \n", ETHER_TO_MACF(add->ea)));

	/* For PMQ Data, Set BIT1:PSMode(PM) to indicate the STA in PS */
	wlc_bmac_write_auxpmq(wlc_hw, idx, ea, (PMQH_PMON >> 16));
done:
	return idx;
}

static void BCMFASTPATH
wlc_bmac_auxpmq_remove(wlc_hw_info_t *wlc_hw, uint16 idx)
{
	bmac_pmq_t *bmac_pmq = wlc_hw->bmac_pmq;
	bmac_auxpmq_entry_t *remove;

	ASSERT(isset(bmac_pmq->auxpmq_used, idx));

	remove = &(bmac_pmq->auxpmq_list[idx]);

	WL_PS(("Removed "MACF" from AuxPMQ \n", ETHER_TO_MACF(remove->ea)));

	wlc_bmac_write_auxpmq(wlc_hw, idx, &ether_null, 0);

	clrbit(bmac_pmq->auxpmq_used, idx);

	bmac_pmq->auxpmq_entry_cnt--;

	memset(remove, 0, sizeof(bmac_auxpmq_entry_t));

	if (bmac_pmq->auxpmq_full)
		bmac_pmq->auxpmq_full = 0;
}

static void BCMFASTPATH
wlc_bmac_clear_auxpmq(wlc_hw_info_t *wlc_hw)
{
	bmac_pmq_t *bmac_pmq = wlc_hw->bmac_pmq;
	bmac_auxpmq_entry_t *remove;
	uint16 idx;

	WL_PS(("PS : clearing ucode AuxPMQ\n"));

	for (idx = 0; idx < AUXPMQ_ENTRIES; ++idx) {
		if (isset(bmac_pmq->auxpmq_used, idx)) {
			remove = &(bmac_pmq->auxpmq_list[idx]);
			WL_PS(("Removed "MACF" from AuxPMQ \n", ETHER_TO_MACF(remove->ea)));
			wlc_bmac_write_auxpmq(wlc_hw, idx, &ether_null, 0);
			clrbit(bmac_pmq->auxpmq_used, idx);
			memset(remove, 0, sizeof(bmac_auxpmq_entry_t));
		}
	}

	if (bmac_pmq->auxpmq_full)
		bmac_pmq->auxpmq_full = 0;

	bmac_pmq->auxpmq_entry_cnt = 0;
}

static void BCMFASTPATH
wlc_bmac_process_auxpmq(wlc_hw_info_t *wlc_hw, struct ether_addr *ea,
	uint8 operation, uint16 *auxpmq_idx)
{
	uint16 index;

	switch (operation) {
	case AUXPMQ_ADD_ENTRY:
		index = *auxpmq_idx;
		if (index != AUXPMQ_INVALID_IDX) {
			WL_PS(("wl%d: "MACF" already add to auxpmq, index %d !\n",
				wlc_hw->unit, ETHERP_TO_MACF(ea), index));
			break;
		}
		*auxpmq_idx = wlc_bmac_auxpmq_add(wlc_hw, ea);
		break;
	case AUXPMQ_REMOVE_ENTRY:
		index = *auxpmq_idx;
		if (index < AUXPMQ_ENTRIES) {
			wlc_bmac_auxpmq_remove(wlc_hw, index);
		} else {
			WL_PS(("wl%d: "MACF" already remove from auxpmq, index %d !\n",
				wlc_hw->unit, ETHERP_TO_MACF(ea), index));
		}
		*auxpmq_idx = AUXPMQ_INVALID_IDX;
		break;
	case AUXPMQ_CLEAR_ALL:
		wlc_bmac_clear_auxpmq(wlc_hw);
		wlc_apps_clear_auxpmq(wlc_hw->wlc);
		break;
	default:
		break;
	}
}

/**
 * Preallocate the all entries of AuxPMQ.
 */
static int
wlc_bmac_auxpmq_init(wlc_hw_info_t *wlc_hw)
{
	bmac_auxpmq_entry_t *res;

	res = (bmac_auxpmq_entry_t *)MALLOCZ(wlc_hw->osh,
		AUXPMQ_ENTRIES * sizeof(bmac_auxpmq_entry_t));
	if (res == NULL) {
		WL_ERROR(("wl%d: Init AuxPMQ error. Out of memory !!\n", wlc_hw->unit));
		return BCME_NOMEM;
	}

	wlc_hw->bmac_pmq->auxpmq_list = res;
	return BCME_OK;
}

static void
wlc_bmac_auxpmq_delete(wlc_hw_info_t *wlc_hw)
{
	if (wlc_hw->bmac_pmq->auxpmq_list) {
		MFREE(wlc_hw->osh, wlc_hw->bmac_pmq->auxpmq_list,
			AUXPMQ_ENTRIES * sizeof(bmac_auxpmq_entry_t));
	}
}
#endif /* WL_AUXPMQ */

/**
 * AP specific. Called by high driver when it detects a switch which is normally already in the bmac
 * queue.
 * For full driver, this function will be called directly as a result of a call to
 * wlc_apps_process_ps_switch,
 *  with no delay, before the end of the wlc_bmac_processpmq loop. No state is necessary.
 *  In case of bmac-high split, it will be called after the high received the rpc
 *  call to wlc_apps_process_ps_switch, AFTER
 *  the end of  the wlc_bmac_processpmq loop.
 *  We need to keep state of what has been received by the high driver.
 */
void BCMFASTPATH
wlc_bmac_process_ps_switch(wlc_hw_info_t *wlc_hw, struct ether_addr *ea, int8 ps_flags,
	uint16 *auxpmq_idx)
{
	bmac_pmq_t *pmq = wlc_hw->bmac_pmq;

	/*
	   ps_on's highest bits are used like this :
	   - PS_SWITCH_FIFO_FLUSHED : there is no more packets pending
	   - PS_SWITCH_MAC_INVALID  this is not really a switch, just a tx fifo
	   empty  indication. Mac address
	   is not present in the message.
	   - PS_SWITCH_STA_REMOVED : the scb for this mac has been removed by the high driver or
	   is not associated.
	*/
#ifdef WLC_LOW_ONLY
	/*
	   for bmac-high split, manage a local pmq state to detect when the high driver
	   has received all relevant information and the ucode pmq can be cleared.
	   NOTE : it seems unlikely that we will ever get the PMQ full because the high
	   driver doesn't report switches fast enough to keep up with bmac.
	   However, if it happens, we would need to either stop reading the PMQ
	   waiting to be in sync, or have a bmac suppression level.
	   To keep in mind.
	*/
	bmac_pmq_entry_t *entry;
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN];
#endif // endif
	if (!(ps_flags & PS_SWITCH_MAC_INVALID)) {

		if ((entry = wlc_bmac_pmq_find(wlc_hw, ea))) {
			WL_PS((" HIGH %s switched to PS state %d switches %d active %d \n",
			       bcm_ether_ntoa(ea, eabuf), ps_flags,
			       entry->switches, pmq->active_entries));

			/* if the STA is removed, remove forcefully and decrement active entries
			 * if switches are not already 0
			 */
			if (ps_flags & PS_SWITCH_STA_REMOVED) {
				if (entry->switches)
					pmq->active_entries--;
				wlc_bmac_pmq_remove(wlc_hw, entry);
			}
			else {
				/* if getting to 0, decrement the number of active entries.
				   Ideally, this should take the 0x80 bit into account for each
				   STA.
				*/
				entry->switches--;
				if (entry->switches == 0) {
					pmq->active_entries--;
					/* if we are in ps off state, or the scb has been removed,
					   this entry can be re-used
					*/
					if ((!entry->ps_on)) {
						wlc_bmac_pmq_remove(wlc_hw, entry);
					}
				}
			}
		}
	}
#endif /* WLC_LOW_ONLY */

#ifdef WL_AUXPMQ
	if (AUXPMQ_ENAB(wlc_hw)) {
		if ((ps_flags & PS_SWITCH_STA_REMOVED) && ea && auxpmq_idx) {
			/* Remove AuxPMQ entry if SCB finished transitioning. */
			wlc_bmac_process_auxpmq(wlc_hw, ea, AUXPMQ_REMOVE_ENTRY, auxpmq_idx);
		} else if (ps_flags & PS_SWITCH_FIFO_FLUSHED) {
			/* Clear all entries if no data in the Tx FIFO. */
			if (pmq->auxpmq_entry_cnt)
				wlc_bmac_process_auxpmq(wlc_hw, ea, AUXPMQ_CLEAR_ALL, auxpmq_idx);
		} else if ((ps_flags & (PS_SWITCH_PMQ_ENTRY | PS_SWITCH_PMQ_SUPPR_PKT |
			PS_SWITCH_PMQ_PSPRETEND)) && ea && auxpmq_idx) {
			/* Add AuxPMQ Entry when SCB is transitioning to PS mode
			 * and has data in the Tx FIFO
			 */
			wlc_bmac_process_auxpmq(wlc_hw, ea, AUXPMQ_ADD_ENTRY, auxpmq_idx);
		}
	}
#endif /* WL_AUXPMQ */

	/* no more packet pending and no more non-acked switches ... clear the PMQ */
	if (ps_flags & PS_SWITCH_FIFO_FLUSHED)
	{
		pmq->tx_draining = 0;
		if (pmq->active_entries == 0 &&
		    pmq->pmq_read_count) {
			wlc_bmac_clearpmq(wlc_hw);
		}
	}
}

static int
BCMATTACHFN(wlc_bmac_pmq_init)(wlc_hw_info_t *wlc_hw)
{
	int ret = BCME_OK;

	wlc_hw->bmac_pmq = (bmac_pmq_t *)MALLOC(wlc_hw->osh, sizeof(bmac_pmq_t));
	if (!wlc_hw->bmac_pmq) {
		WL_ERROR(("BPMQ error. Out of memory !!\n"));
		return BCME_NOMEM;
	}
	memset(wlc_hw->bmac_pmq, 0, sizeof(bmac_pmq_t));
	wlc_hw->bmac_pmq->pmq_size = BMAC_PMQ_SIZE;

#ifdef WL_AUXPMQ
	ret = wlc_bmac_auxpmq_init(wlc_hw);
#endif // endif
	return ret;
}

static void
BCMATTACHFN(wlc_bmac_pmq_delete)(wlc_hw_info_t *wlc_hw)
{
#ifdef WLC_LOW_ONLY
	bmac_pmq_entry_t * entry;
	bmac_pmq_entry_t * nxt;
#endif // endif
	if (!wlc_hw->bmac_pmq)
		return;

#ifdef WL_AUXPMQ
	wlc_bmac_auxpmq_delete(wlc_hw);
#endif // endif

#ifdef WLC_LOW_ONLY
	entry = wlc_hw->bmac_pmq->entry;
	while (entry) {
		nxt = entry->next;
		MFREE(wlc_hw->osh, entry, sizeof(bmac_pmq_entry_t));
		if (nxt == wlc_hw->bmac_pmq->entry)
			break;
		entry = nxt;
	}
#endif // endif
	MFREE(wlc_hw->osh, wlc_hw->bmac_pmq, sizeof(bmac_pmq_t));
	wlc_hw->bmac_pmq = NULL;
}

#ifdef TXQ_MUX
/* XXX Turn off PS processing in the AP, disabled
 * until the development of the TXQ_MUX PS handling code completes.
 */
bool BCMFASTPATH
wlc_bmac_processpmq(wlc_hw_info_t *wlc_hw, bool bounded)
{
	volatile uint16 *pmqctrlstatus;

	BCM_REFERENCE(bounded);

	WL_ERROR(("-------------------------------->PS : clearing ucode PMQ\n"));

	pmqctrlstatus = (volatile uint16 *)&wlc_hw->regs->pmqreg.w.pmqctrlstatus;
	/* Clear the PMQ entry unless we are letting the data fifo drain
	 * when txstatus indicates unlocks the data fifo we clear
	 * the PMQ of any processed entries
	 */

	W_REG(wlc_hw->osh, pmqctrlstatus, PMQH_DEL_MULT);
	wlc_hw->bmac_pmq->pmq_read_count = 0;

	return FALSE;
}
#else
bool BCMFASTPATH
wlc_bmac_processpmq(wlc_hw_info_t *wlc_hw, bool bounded)
{
	volatile uint32 *pmqhostdata;
	uint32 pmqdata;
	d11regs_t *regs = wlc_hw->regs;
	uint32 pat_hi, pat_lo;
	struct ether_addr eaddr;
	bmac_pmq_t *pmq = wlc_hw->bmac_pmq;
	int8 ps_on, ps_pretend, read_count = 0;
	bool pmq_need_resched = FALSE;
	uint32 macintstatus;

#ifdef WLC_LOW_ONLY
	uint8 new_switch = 0;
	bmac_pmq_entry_t *pmq_entry = NULL;
#endif /* WLC_LOW_ONLY */
#if defined(BCMDBG) || defined(BCMDBG_ERR)
	char eabuf[ETHER_ADDR_STR_LEN];
	BCM_REFERENCE(eabuf);
#endif // endif
	/* clear pmqerr */
	if (D11REV_IS(wlc_hw->corerev, 64))
		SET_MACINTSTATUS(wlc_hw->osh, regs, MI_PMQERR);
	pmqhostdata = (volatile uint32 *)&regs->pmqreg.pmqhostdata;
	/* read entries until empty or pmq exceeding limit */
	while (1) {
		pmqdata = R_REG(wlc_hw->osh, pmqhostdata);
		if (!(pmqdata & PMQH_NOT_EMPTY)) {
			if (!D11REV_IS(wlc_hw->corerev, 64)) {
				break;
			} else {
				 /* if it's empty, need to check pmqerr to know
				  * this read itself is reading valid entry
				  */
				macintstatus = GET_MACINTSTATUS(wlc_hw->osh, regs);
				if (macintstatus & MI_PMQERR) {
					/* already empty before this read */
					SET_MACINTSTATUS(wlc_hw->osh, regs, MI_PMQERR);
					break;
				}
			}
		}

		pat_lo = R_REG(wlc_hw->osh, &regs->pmqpatl);
		pat_hi = R_REG(wlc_hw->osh, &regs->pmqpath);
		eaddr.octet[5] = (pat_hi >> 8)  & 0xff;
		eaddr.octet[4] =  pat_hi	& 0xff;
		eaddr.octet[3] = (pat_lo >> 24) & 0xff;
		eaddr.octet[2] = (pat_lo >> 16) & 0xff;
		eaddr.octet[1] = (pat_lo >> 8)  & 0xff;
		eaddr.octet[0] =  pat_lo	& 0xff;

		read_count++;
		pmq->pmq_read_count++;

		if (ETHER_ISMULTI(eaddr.octet)) {
			WL_INFORM(("wl%d: wlc_bmac_processpmq:"
				" skip entry with mc/bc address %s\n",
				wlc_hw->unit, bcm_ether_ntoa(&eaddr, eabuf)));
			continue;
		}

		ps_on = (pmqdata & PMQH_PMON) ? PS_SWITCH_PMQ_ENTRY : PS_SWITCH_OFF;
		ps_pretend = (pmqdata & PMQH_PMPS) ? PS_SWITCH_PMQ_PSPRETEND : PS_SWITCH_OFF;

#ifdef WLC_LOW_ONLY
		/* in case of a split driver, evaluate if this is a real switch
		   before calling the high
		*/
		new_switch = 0;
		if ((pmq_entry = wlc_bmac_pmq_find(wlc_hw, &eaddr))) {
			/* is it a switch ? */
			if (pmq_entry->ps_on != ps_on) {
				pmq_entry->switches++;
				if (pmq_entry->switches == 1)
					pmq->active_entries++;
				pmq_entry->ps_on = ps_on;
				new_switch = 1;
				WL_PS(("BPMQ : sta %s switched to PS %d\n",
				       bcm_ether_ntoa(&eaddr, eabuf), ps_on));
			}
		}
		/* add a new entry only if it is in the ON state */
		else if (ps_on == PS_SWITCH_PMQ_ENTRY) {
			if ((pmq_entry = wlc_bmac_pmq_add(wlc_hw, &eaddr))) {
				WL_PS(("BPMQ : sta %s added with PS %d\n",
				       bcm_ether_ntoa(&eaddr, eabuf), ps_on));
				pmq_entry->ps_on = ps_on;
				pmq_entry->switches = 1;
				pmq->active_entries++;
				new_switch = 1;
			}
		}
		if (new_switch) {
			if (ps_on == PS_SWITCH_PMQ_ENTRY)
				pmq->tx_draining = 1;
			wlc_apps_process_ps_switch(wlc_hw->wlc, &eaddr, ps_on);
		}
#else
		wlc_apps_process_ps_switch(wlc_hw->wlc, &eaddr, ps_on | ps_pretend);
#endif /* WLC_LOW_ONLY */
		/* if we exceed the per invocation pmq entry processing limit,
		 * reschedule again (only if bounded) to process the remaining pmq entries at a
		 * later time.
		 */
		if (bounded &&
		    (read_count >= pmq->pmq_size - BMAC_PMQ_MIN_ROOM)) {
			pmq_need_resched = TRUE;
			break;
		}
	}

	/* After Reading all the entries in the PMQ fifo,
	 * and after writing the CAM with the corresponding entries,
	 * host can delete all entries in the PMQ fifo.
	 * If Aux PMQ is full, using the PMQ for those entries which are not in Aux PMQ.
	 */
	if ((AUXPMQ_ENAB(wlc_hw) && !pmq->auxpmq_full) ||
#ifdef WLC_LOW_ONLY
		(!pmq->active_entries && !pmq->tx_draining) ||
#endif /* WLC_LOW_ONLY */
		FALSE) {
		wlc_bmac_clearpmq(wlc_hw);
	}

	return pmq_need_resched;
}
#endif /* TXQ_MUX */

/**
 * AP specific. Read and drain all the PMQ entries while not EMPTY.
 * When PMQ handling is enabled (MCTL_DISCARD_PMQ in maccontrol is clear),
 * one PMQ entry per packet received from a STA is created with corresponding 'ea' as key.
 * AP reads the entry and handles the PowerSave mode transitions of a STA by
 * comparing the PMQ entry with current PS-state of the STA. If PMQ entry is same as the
 * driver state, it's ignored, else transition is handled.
 *
 * With MBSS code, ON PMQ entries are also added for BSS configs; they are
 * ignored by the SW.
 *
 * Note that PMQ entries remain in the queue for the ucode to search until
 * an explicit delete of the entries is done with PMQH_DEL_MULT (or DEL_ENTRY).
 */

static void
wlc_bmac_clearpmq(wlc_hw_info_t *wlc_hw)
{
	volatile uint16 *pmqctrlstatus;
	d11regs_t *regs = wlc_hw->regs;

	if (!wlc_hw->bmac_pmq->pmq_read_count)
		return;

	WL_PS(("PS : clearing ucode PMQ\n"));

	pmqctrlstatus = (volatile uint16 *)&regs->pmqreg.w.pmqctrlstatus;
	/* Clear the PMQ entry unless we are letting the data fifo drain
	 * when txstatus indicates unlocks the data fifo we clear
	 * the PMQ of any processed entries
	 */

	W_REG(wlc_hw->osh, pmqctrlstatus, PMQH_DEL_MULT);
	wlc_hw->bmac_pmq->pmq_read_count = 0;
}
#endif /* AP */

/**
 * Used for test functionality (packet engine / diagnostics), or for BMAC and offload firmware
 * builds.
 */
void
wlc_bmac_txfifo(wlc_hw_info_t *wlc_hw, uint fifo, void *p,
	bool commit, uint16 frameid, uint8 txpktpend)
{
	ASSERT(p);

	/* bump up pending count */
	if (commit) {
		TXPKTPENDINC(wlc_hw->wlc, fifo, txpktpend);
#if defined(WLC_HIGH) && defined(NEW_TXQ)
		wlc_low_txq_buffered_inc(wlc_hw->wlc->active_queue->low_txq,
			fifo, txpktpend);
#endif /* WLC_HIGH && NEW_TXQ */

		WL_TRACE(("wlc_bmac_txfifo, pktpend inc %d to %d\n", txpktpend,
			TXPKTPENDGET(wlc_hw->wlc, fifo)));
	}

	if (!PIO_ENAB_HW(wlc_hw)) {
		/* Commit BCMC sequence number in the SHM frame ID location */
		if (frameid != INVALIDFID) {
			wlc_bmac_write_shm(wlc_hw, M_BCMC_FID, frameid);
		}

		if (wlc_bmac_dma_txfast(wlc_hw->wlc, fifo, p, commit) < 0) {
			PKTFREE(wlc_hw->osh, p, TRUE);
			WL_ERROR(("wlc_bmac_txfifo: fatal, toss frames !!!\n"));
			if (commit) {
				TXPKTPENDDEC(wlc_hw->wlc, fifo, txpktpend);
#if defined(WLC_HIGH) && defined(NEW_TXQ)
				wlc_low_txq_buffered_dec(wlc_hw->wlc->active_queue->low_txq,
					fifo, txpktpend);
#endif /* WLC_HIGH && NEW_TXQ */
			}
		}
#if defined(BCM_OL_DEV) && defined(L2KEEPALIVEOL)
		else {
			wlc_pkttag_t *pkttag;
			pkttag = WLPKTTAG(p);
			if (!(pkttag->flags & WLF_NULLPKT)) {
				wlc_l2_keepalive_timer_start(wlc_hw->wlc->wlc_dngl_ol, TRUE);
			}
		}
#endif // endif
	} else {
		wlc_pio_tx(wlc_hw->pio[fifo], p);
	}
}

/** Periodic tasks are carried out by a watchdog timer that is called once every second */
void
wlc_bmac_watchdog(void *arg)
{
	wlc_info_t *wlc = (wlc_info_t*)arg;
	wlc_hw_info_t *wlc_hw = wlc->hw;
	uint i = 0;
	WL_TRACE(("wl%d: wlc_bmac_watchdog\n", wlc_hw->unit));

	if (!wlc_hw->up)
		return;

	ASSERT(!wlc_hw->sbclk || !si_taclear(wlc_hw->sih, TRUE));

	/* increment second count */
	wlc_hw->now++;

	/* Check for FIFO error interrupts */
	wlc_bmac_fifoerrors(wlc_hw);

#ifdef WLC_HIGH
	/* Check whether PSMx watchdog triggered */
	if (D11REV_GE(wlc->pub->corerev, 64))
		wlc_bmac_psmx_errors(wlc);
#endif /* WLC_HIGH */

	/* make sure RX dma has buffers */
	if (!PIO_ENAB_HW(wlc_hw)) {
		/* DMA RXFILL */
		for (i = 0; i < MAX_RX_FIFO; i++) {
			if ((wlc_hw->di[i] != NULL) && wlc_bmac_rxfifo_enab(i)) {
#ifdef DBG_RXDMA_STUCK_WAR
				if (i == RX_FIFO) {
					dma_rxfill(wlc_hw->di[i]);
				} else if (!wlc->pub->trig_rxstuck) {
					dma_rxfill(wlc_hw->di[i]);
				}
#else
				dma_rxfill(wlc_hw->di[i]);
#endif // endif
			}
		}
		if (D11REV_IS(wlc_hw->corerev, 4)) {
			dma_rxfill(wlc_hw->di[RX_TXSTATUS_FIFO]);
		}
	}

#ifndef BCM_OL_DEV
#ifdef LPAS
	/* In LPAS mode phy_wdog is invoked from tbtt only. It can be upto
	 * 2 secs between two phy_wdog invokations.
	 */
	if (!wlc->lpas)
#endif /* LPAS */
	{
		phy_watchdog((phy_info_t *)wlc_hw->band->pi);
	}
#else
	wl_watchdog(wlc_hw->wlc->wl);
#endif /* BCM_OL_DEV */

	ASSERT(!wlc_hw->sbclk || !si_taclear(wlc_hw->sih, TRUE));
}

#ifndef BCM_OL_DEV
/** bmac rpc agg watchdog code, called once every couple of milliseconds */
void
wlc_bmac_rpc_agg_watchdog(void *arg)
{
	wlc_info_t *wlc = (wlc_info_t*)arg;
	wlc_hw_info_t *wlc_hw = wlc->hw;

	WL_TRACE(("wl%d: wlc_bmac_rpc_agg_watchdog\n", wlc_hw->unit));

	if (!wlc_hw->up)
		return;
#ifdef WLC_LOW_ONLY
	/* maintenance */
	wlc_bmac_rpc_watchdog(wlc);
#endif // endif
}
#endif /* BCM_OL_DEV */

/** Higher MAC requests to activate SRVSDB operation for the currently selected band */
int wlc_bmac_activate_srvsdb(wlc_hw_info_t *wlc_hw, chanspec_t chan0, chanspec_t chan1)
{
	int err = BCME_ERROR;

	wlc_hw->sr_vsdb_active = FALSE;
#ifdef SRHWVSDB
	if (SRHWVSDB_ENAB(wlc_hw->wlc->pub) &&
	    wlc_phy_attach_srvsdb_module(wlc_hw->band->pi, chan0, chan1)) {
		wlc_hw->sr_vsdb_active = TRUE;
		err = BCME_OK;
	}
#endif /* SRHWVSDB */
	return err;
}

/** Higher MAC requests to deactivate SRVSDB operation for the currently selected band */
void wlc_bmac_deactivate_srvsdb(wlc_hw_info_t *wlc_hw)
{
	wlc_hw->sr_vsdb_active = FALSE;
#ifdef SRHWVSDB
	if (SRHWVSDB_ENAB(wlc_hw->wlc->pub)) {
		wlc_phy_detach_srvsdb_module(wlc_hw->band->pi);
	}
#endif /* SRHWVSDB */
}

/** All SW inits needed for band change */
static void
wlc_bmac_srvsdb_set_band(wlc_hw_info_t *wlc_hw, uint bandunit, chanspec_t chanspec)
{
	uint32 macintmask;
	uint32 mc;
	wlc_info_t *wlc = wlc_hw->wlc;

	ASSERT(NBANDS_HW(wlc_hw) > 1);
	ASSERT(bandunit != wlc_hw->band->bandunit);
	ASSERT(si_iscoreup(wlc_hw->sih));
	ASSERT((R_REG(wlc_hw->osh, &wlc_hw->regs->maccontrol) & MCTL_EN_MAC) == 0);

	/* disable interrupts */
	macintmask = wl_intrsoff(wlc->wl);

	wlc_setxband(wlc_hw, bandunit);

	/* bsinit */
	wlc_ucode_bsinit(wlc_hw);

	mc = R_REG(wlc_hw->osh, &wlc_hw->regs->maccontrol);
	if ((mc & MCTL_EN_MAC) != 0) {
		if (mc == 0xffffffff)
			WL_ERROR(("wl%d: wlc_phy_init: chip is dead !!!\n", wlc_hw->unit));
		else
			WL_ERROR(("wl%d: wlc_phy_init:MAC running! mc=0x%x\n",
				wlc_hw->unit, mc));

		ASSERT((const char*)"wlc_phy_init: Called with the MAC running!" == NULL);
	}

	/* check D11 is running on Fast Clock */
	if (D11REV_GE(wlc_hw->corerev, 5))
		ASSERT(si_core_sflags(wlc_hw->sih, 0, 0) & SISF_FCLKA);

	/* cwmin is band-specific, update hardware with value for current band */
	wlc_bmac_set_cwmin(wlc_hw, wlc_hw->band->CWmin);
	wlc_bmac_set_cwmax(wlc_hw, wlc_hw->band->CWmax);

	wlc_bmac_update_slot_timing(wlc_hw,
	        BAND_5G(wlc_hw->band->bandtype) ? TRUE : wlc_hw->shortslot);

#ifdef WL11N
	/* initialize the txphyctl1 rate table since shmem is shared between bands */
	wlc_upd_ofdm_pctl1_table(wlc_hw);
#endif // endif
	/* Configure BTC GPIOs as bands change */
	if (BAND_5G(wlc_hw->band->bandtype))
		wlc_bmac_mhf(wlc_hw, MHF5, MHF5_BTCX_DEFANT, MHF5_BTCX_DEFANT, WLC_BAND_ALL);
	else
		wlc_bmac_mhf(wlc_hw, MHF5, MHF5_BTCX_DEFANT, 0, WLC_BAND_ALL);

	/*
	 * If there are any pending software interrupt bits,
	 * then replace these with a harmless nonzero value
	 * so wlc_dpc() will re-enable interrupts when done.
	 */
	if (wlc_hw->macintstatus)
		wlc_hw->macintstatus = MI_DMAINT;

	/* restore macintmask */
	wl_intrsrestore(wlc->wl, macintmask);
}

/** optionally switches band */
static uint8
wlc_bmac_srvsdb_set_chanspec(wlc_hw_info_t *wlc_hw, chanspec_t chanspec, bool mute,
	bool fastclk, uint bandunit, bool band_change)
{
	uint8 switched;
	uint8 last_chan_saved = FALSE;

	/* if excursion is active (scan), switch to normal switch */
	/* mode, else allow sr switching when scan periodically */
	/* returns to home channel */
	if (wlc_hw->wlc->excursion_active)
		return FALSE;

	/* calls the PHY to switch */
	switched = wlc_set_chanspec_sr_vsdb(wlc_hw->band->pi, chanspec, &last_chan_saved);

	/*
	 * note: for bmac firmware this is an asynchronous call. Given caller supplied flags,
	 * optionally saves / restores PPR (power per rate, tx power control related) context.
	 */
	wlc_srvsdb_switch_ppr(wlc_hw->wlc, chanspec, last_chan_saved, switched);

	/* If phy context changed from SRVSDB, continue, otherwise return */
	if (!switched)
		return FALSE;

	/* SW init required after SRVSDB switch */
	if (band_change) {
		wlc_bmac_srvsdb_set_band(wlc_hw, bandunit, chanspec);

		if (CHSPEC_IS2G(chanspec)) {
			wlc_hw->band->bandtype = 2;
			wlc_hw->band->bandunit = 0;
		} else {
			wlc_hw->band->bandunit = 1;
			wlc_hw->band->bandtype = 1;
		}
	}

	/* come out of mute */
	wlc_bmac_mute(wlc_hw, mute, 0);

	if (!fastclk)
		wlc_clkctl_clk(wlc_hw, CLK_DYNAMIC);

	/* Successfull SRVSDB switch */
	return TRUE;
}

#ifdef WL11ULB
static void
wlc_bmac_set_ulbparams(wlc_hw_info_t *wlc_hw, chanspec_t chanspec)
{
	wlc_info_t *wlc = wlc_hw->wlc;
	uint16 ulb_active = 0;
	uint16 sifs_val, navctl_val, mhf3_ulbbw_val;
	if (CHSPEC_IS10(chanspec)) {
		ulb_active = MHF5_ULB_PROTECT_EN;
		sifs_val = SIFS_RX_TX_TX_10M;
		navctl_val = NAV_CTL_10M;
		mhf3_ulbbw_val = MHF3_ULB_BW;
	} else if (CHSPEC_IS5(chanspec)) {
		ulb_active = MHF5_ULB_PROTECT_EN;
		sifs_val = SIFS_RX_TX_TX_5M;
		navctl_val = NAV_CTL_5M;
		mhf3_ulbbw_val = MHF3_ULB_BW;
	} else {
		sifs_val = SIFS_RX_TX_TX_20_40_80M;
		navctl_val = NAV_CTL_20_40_80M;
		mhf3_ulbbw_val = 0;
	}
	wlc_bmac_write_ihr(wlc->hw, SIFS_RX_TX_TX, sifs_val);
	wlc_bmac_write_ihr(wlc->hw, NAV_CTL, navctl_val);
	wlc_mhf(wlc, MHF3, MHF3_ULB_BW, mhf3_ulbbw_val, WLC_BAND_ALL);
	wlc_mhf(wlc, MHF5, MHF5_ULB_PROTECT_EN, ulb_active, WLC_BAND_ALL);
#ifdef WLAMPDU
	/* call wlc_ampdu_shm_upd on channel change as the shm's needs
	 *  to be updated with different values for ulb bw's
	 */
	wlc_ampdu_shm_upd(wlc->ampdu_rx);
#endif /* WLAMPDU */
}
#endif /* WL11ULB */

/** higher MAC requested a channel change, optionally to a different band */
void
wlc_bmac_set_chanspec(wlc_hw_info_t *wlc_hw, chanspec_t chanspec, bool mute, ppr_t *txpwr)
{
	bool fastclk;
	uint bandunit = 0;
	uint8 vsdb_switch = 0;
	uint8 vsdb_status = 0;

	if (SRHWVSDB_ENAB(wlc_hw->pub)) {
		vsdb_switch =  wlc_hw->sr_vsdb_force;
#if defined(WLMCHAN) && defined(SR_ESSENTIALS)
		vsdb_switch |= (wlc_hw->sr_vsdb_active &&
			sr_engine_enable(wlc_hw->sih, IOV_GET, FALSE) > 0);
#endif /* WLMCHAN SR_ESSENTIALS */
	}

	WL_TRACE(("wl%d: wlc_bmac_set_chanspec 0x%x\n", wlc_hw->unit, chanspec));

	/* request FAST clock if not on */
	if (!(fastclk = wlc_hw->forcefastclk))
		wlc_clkctl_clk(wlc_hw, CLK_FAST);

	wlc_hw->chanspec = chanspec;

	WL_CHANLOG(wlc_hw->wlc, __FUNCTION__, TS_ENTER, 0);
	/* Switch bands if necessary */
	if (NBANDS_HW(wlc_hw) > 1) {
		bandunit = CHSPEC_WLCBANDUNIT(chanspec);
		if (wlc_hw->band->bandunit != bandunit) {
			/* wlc_bmac_setband disables other bandunit,
			 *  use light band switch if not up yet
			 */
			if (wlc_hw->up)
			{
				if (SRHWVSDB_ENAB(wlc_hw->pub) && vsdb_switch) {
					vsdb_status = wlc_bmac_srvsdb_set_chanspec(wlc_hw,
						chanspec, mute, fastclk, bandunit, TRUE);
					if (vsdb_status) {
						WL_INFORM(("SRVSDB Switch DONE successfully \n"));
						WL_CHANLOG(wlc_hw->wlc, __FUNCTION__, TS_EXIT, 0);
						return;
					}
				}
			}

			if (wlc_hw->up) {
				wlc_phy_chanspec_radio_set(wlc_hw->bandstate[bandunit]->pi,
					chanspec);
				wlc_bmac_setband(wlc_hw, bandunit, chanspec);
			} else {
				wlc_setxband(wlc_hw, bandunit);
			}
		}
	}

	wlc_phy_initcal_enable(wlc_hw->band->pi, !mute);

	if (!wlc_hw->up) {
		if (wlc_hw->clk)
			wlc_phy_txpower_limit_set(wlc_hw->band->pi, txpwr, chanspec);
		wlc_phy_chanspec_radio_set(wlc_hw->band->pi, chanspec);
	} else {
		/* Update muting of the channel.
		 * wlc_phy_chanspec_set may start a wlc_phy_cal_perical, to prevent emitting energy
		 * on a muted channel, muting of the channel is updated before hand.
		 */
		wlc_bmac_mute(wlc_hw, mute, 0);

		if ((wlc_hw->deviceid == BCM4360_D11AC_ID) ||
			(wlc_hw->deviceid == BCM4335_D11AC_ID) ||
			(wlc_hw->deviceid == BCM4345_D11AC_ID) ||
			(wlc_hw->deviceid == BCM43602_D11AC_ID) ||
			(wlc_hw->deviceid == BCM4350_D11AC_ID) ||
			(wlc_hw->deviceid == BCM43556_D11AC_ID) ||
			(wlc_hw->deviceid == BCM43558_D11AC_ID) ||
			(wlc_hw->deviceid == BCM43566_D11AC_ID) ||
			(wlc_hw->deviceid == BCM43568_D11AC_ID) ||
			(wlc_hw->deviceid == BCM43569_D11AC_ID) ||
			(wlc_hw->deviceid == BCM4354_D11AC_ID) ||
			(wlc_hw->deviceid == BCM4356_D11AC_ID) ||
			(wlc_hw->deviceid == BCM4358_D11AC_ID) ||
			(wlc_hw->deviceid == BCM4352_D11AC_ID) ||
			(wlc_hw->deviceid == BCM4349_D11AC_ID) ||
			(wlc_hw->deviceid == BCM4355_D11AC_ID) ||
			(wlc_hw->deviceid == BCM4359_D11AC_ID) ||
			(wlc_hw->deviceid == BCM4365_D11AC_ID) ||
			(wlc_hw->deviceid == BCM4366_D11AC_ID) ||
			0) {
				/* phymode switch requires phyinit */
				if (phy_init_pending((phy_info_t *)wlc_hw->band->pi))
					phy_init((phy_info_t *)wlc_hw->band->pi, chanspec);
				else
					wlc_phy_chanspec_set(wlc_hw->band->pi, chanspec);
		} else {
			/* Bandswitch above may end up changing the channel so avoid repetition */
			if (chanspec != phy_utils_get_chanspec((phy_info_t *)wlc_hw->band->pi)) {
				if (SRHWVSDB_ENAB(wlc_hw->pub) && vsdb_switch) {
					vsdb_status = wlc_bmac_srvsdb_set_chanspec(wlc_hw,
						chanspec, mute, fastclk, bandunit, FALSE);
					if (vsdb_status) {
						WL_INFORM(("SRVSDB Switch DONE successfully \n"));
						WL_CHANLOG(wlc_hw->wlc, __FUNCTION__, TS_EXIT, 0);
						return;
					}
				}
				wlc_phy_chanspec_set(wlc_hw->band->pi, chanspec);
			}
		}
		wlc_phy_txpower_limit_set(wlc_hw->band->pi, txpwr, chanspec);
	}

#ifdef WL11ULB
	if (ULB_ENAB(wlc_hw->wlc->pub)) {
		if (wlc_hw->up)
			wlc_bmac_set_ulbparams(wlc_hw, chanspec);
	}
#endif /* WL11ULB */

	if (!fastclk)
		wlc_clkctl_clk(wlc_hw, CLK_DYNAMIC);
	WL_CHANLOG(wlc_hw->wlc, __FUNCTION__, TS_EXIT, 0);
} /* wlc_bmac_set_chanspec */

/**
 * Higher MAC (e.g. as contained in the wl host driver) has to query hardware+firmware
 * attributes to use the same settings as bmac, and has to query capabilities before enabling them.
 */
int
wlc_bmac_revinfo_get(wlc_hw_info_t *wlc_hw, wlc_bmac_revinfo_t *revinfo)
{
	si_t *sih = wlc_hw->sih;
	uint idx;

	revinfo->vendorid = wlc_hw->vendorid;
	revinfo->deviceid = wlc_hw->deviceid;

	revinfo->boardrev = wlc_hw->boardrev;
	revinfo->corerev = wlc_hw->corerev;
	revinfo->sromrev = wlc_hw->sromrev;
	/* srom9 introduced ppr, which requires corerev >= 24 */
	if (wlc_hw->sromrev >= 9) {
		WL_ERROR(("wlc_bmac_attach: srom9 ppr requires corerev >=24"));
		ASSERT(D11REV_GE(wlc_hw->corerev, 24));
	}
	revinfo->chiprev = sih->chiprev;
	revinfo->chip = SI_CHIPID(sih);
	revinfo->chippkg = sih->chippkg;
	revinfo->boardtype = sih->boardtype;
	revinfo->boardvendor = sih->boardvendor;
	revinfo->bustype = sih->bustype;
	revinfo->buscoretype = sih->buscoretype;
	revinfo->buscorerev = sih->buscorerev;
	revinfo->issim = sih->issim;
	revinfo->boardflags = wlc_hw->boardflags;
	revinfo->boardflags2 = wlc_hw->boardflags2;
	if (wlc_hw->sromrev >= 12)
		revinfo->boardflags4 = wlc_hw->boardflags4;

	revinfo->nbands = NBANDS_HW(wlc_hw);

	for (idx = 0; idx < NBANDS_HW(wlc_hw); idx++) {
		wlc_hwband_t *band;

		/* So if this is a single band 11a card, use band 1 */
		if (IS_SINGLEBAND_5G(wlc_hw->deviceid))
			idx = BAND_5G_INDEX;

		band = wlc_hw->bandstate[idx];
		revinfo->band[idx].bandunit = band->bandunit;
		revinfo->band[idx].bandtype = band->bandtype;
		revinfo->band[idx].phytype = band->phytype;
		revinfo->band[idx].phyrev = band->phyrev;
		revinfo->band[idx].phy_minor_rev = band->phy_minor_rev;
		revinfo->band[idx].radioid = band->radioid;
		revinfo->band[idx].radiorev = band->radiorev;
		revinfo->band[idx].anarev = 0;

	}

	revinfo->_wlsrvsdb = wlc_hw->wlc->pub->_wlsrvsdb;

#ifdef WLC_LOW_ONLY
	revinfo->host_rpc_agg_size = BCM_RPC_TP_HOST_AGG_DEFAULT;
	revinfo->ampdu_mpdu = BCM_AMPDU_MPDU;
#ifdef AMPDU_BA_RX_WSIZE
	revinfo->ampdu_ba_rx_wsize = AMPDU_BA_RX_WSIZE;
#else
	revinfo->ampdu_ba_rx_wsize = 0;
#endif /* AMPDU_BA_RX_WSIZE */
	revinfo->host_rpc_agg_size = BCM_RPC_TP_HOST_AGG_DEFAULT;
	revinfo->ampdu_mpdu = BCM_AMPDU_MPDU;
	revinfo->is_ss = wl_dngl_is_ss(wlc_hw->wlc->wl);
#ifdef BCM_AMPDU_MPDU_SS
	if (revinfo->is_ss) {
		revinfo->host_rpc_agg_size = BCM_RPC_TP_HOST_AGG_DEFAULT_SS;
		revinfo->ampdu_mpdu = BCM_AMPDU_MPDU_SS;
	}
#endif // endif
	revinfo->wowl_gpio = wlc_hw->wowl_gpio;
	revinfo->wowl_gpiopol = wlc_hw->wowl_gpiopol;
#endif /* WLC_LOW_ONLY */

	return BCME_OK;
} /* wlc_bmac_revinfo_get */

int
wlc_bmac_state_get(wlc_hw_info_t *wlc_hw, wlc_bmac_state_t *state)
{
	state->machwcap = wlc_hw->machwcap;
	state->preamble_ovr = (uint32)wlc_phy_preamble_override_get(wlc_hw->band->pi);

	return 0;
}

#ifdef DMATXRC
/*
 * improves # of free packets by recycling Tx packets faster: on D11 DMA IRQ instead of on D11 Tx
 * Complete IRQ.
 */

static void
wlc_phdr_handle(wlc_hw_info_t *wlc_hw, void *p)
{
	bool found, processed;
	void *pdata;
	osl_t *osh = wlc_hw->osh;
	txrc_ctxt_t *rctxt;

	processed = TRUE;
	found = (WLPKTTAG(p)->flags & (WLF_PHDR)) ? TRUE : FALSE;
	if (found) {
		pdata = PKTNEXT(osh, p);
		if (pdata != NULL) {
#ifndef PKTC_TX_DONGLE
			/* Only apply if not using pkt chaining */
			ASSERT(PKTNEXT(osh, pdata) == NULL);
#endif // endif
			rctxt = TXRC_PKTTAIL(osh, p);
			ASSERT(TXRC_ISMARKER(rctxt));

#ifdef PROP_TXSTATUS
			/*
			send credit update only if this packet came from the host
			and this was not sent to a vFIFO (i.e. for a psq)
			*/
			if (PROP_TXSTATUS_ENAB(wlc_hw->wlc->pub)) {
				uint32 whinfo = WLPKTTAG(p)->wl_hdr_information;
				void *p2;
#ifdef PKTC_TX_DONGLE
				int i = 0;
#endif // endif

			   /* XXX: Should chainable condition avoid
			    * XXX: having different flags?
			    */
			   if ((WL_TXSTATUS_GET_FLAGS(whinfo) & WLFC_PKTFLAG_PKTFROMHOST) &&
			      !(WL_TXSTATUS_GET_FLAGS(whinfo) & WLFC_PKTFLAG_PKT_REQUESTED))
					processed = TRUE;
				else
					processed = FALSE;

				for (p2 = p; processed && p2; p2 = PKTNEXT(osh, p2)) {
				whinfo = WLPKTTAG(p2)->wl_hdr_information;

				/* whinfo is zero if prepended with phdr so only
				 * processed pkts with valid wl_hdr_information
				 */
				if (whinfo) {
					if (WLFC_CONTROL_SIGNALS_TO_HOST_ENAB(wlc_hw->wlc->pub))
						wlfc_push_credit_data(wlc_hw->wlc->wl, p2);

#ifdef PKTC_TX_DONGLE
					/* Save starting with second pkt since phdr
					 * already has wl_hdr_information for first pkt
					 */
					if (PKTC_ENAB(wlc_hw->wlc->pub) && (i > 0)) {
						/* Clear so PKTFREE() callback does not
						 * process wlhdr info
						 */
						WLPKTTAG(p2)->wl_hdr_information = 0;

						ASSERT(rctxt->n < ARRAYSIZE(rctxt->wlhdr));
						rctxt->wlhdr[rctxt->n] = whinfo;
						rctxt->seq[rctxt->n] = WLPKTTAG(p2)->seq;
						rctxt->n++;
					}
					i++; /* Next */
#endif /* PKTC_TX_DONGLE */
				}
			     }

			     if (processed) {
#ifdef PKTC_TX_DONGLE
				if (PKTC_ENAB(wlc_hw->wlc->pub) && (rctxt->n))
					TXRC_SETWLHDR(rctxt);
#endif // endif
			     }
			}
#endif /* PROP_TXSTATUS */

			/* For proptxstatus, don't free if not processed so we can return
			 * info back to host
			 */
			if (processed) {
				PKTSETNEXT(osh, p, NULL);
				TXRC_SETRECLAIMED(rctxt);
#ifdef BCMDBG_POOL
				ASSERT(PKTPOOLSTATE(pdata) == POOL_TXD11);
#endif // endif
				PKTFREE(osh, pdata, TRUE);
			}
		}
	}
}

static void
wlc_dmatx_peekall(wlc_hw_info_t *wlc_hw, hnddma_t *di)
{
	void *phdr;
	int i, n;

	ASSERT(wlc_hw->wlc->phdr_list);

	n = wlc_hw->wlc->phdr_len;
	if (dma_peekntxp(di, &n, wlc_hw->wlc->phdr_list, HNDDMA_RANGE_TRANSFERED))
		return;

	for (i = 0; i < n; i++) {
		phdr = wlc_hw->wlc->phdr_list[i];
		ASSERT(phdr);
		if (phdr != NULL) {
			wlc_phdr_handle(wlc_hw, phdr);
			wlc_hw->wlc->phdr_list[i] = NULL;
		}
	}

}

/** early tx packet reclaim to improve memory usage */
void
wlc_dmatx_reclaim(wlc_hw_info_t *wlc_hw)
{
	int i;
	hnddma_t *di;

	/*
	 * A couple of issues impacting tput with early reclaim:
	 * 1.  Doing pktpool avail callbacks has tput impact;
	 *     Temporarily disable callbacks while processing tx reclaim.
	 * 2.  Wasting cycles on fifos not enabled for early reclaim
	 */
	pktpool_emptycb_disable(wlc_hw->wlc->pub->pktpool, TRUE);

	for (i = 0; i < NFIFO; i++) {
		if (!(wlc_hw->wlc->txrc_fifo_mask & (1 << i)))
			continue;

		di = wlc_hw->di[i];
		if (di)
			wlc_dmatx_peekall(wlc_hw, di);
	}

	/* Re-enable pktpool avail callbacks */
	pktpool_emptycb_disable(wlc_hw->wlc->pub->pktpool, FALSE);
}
#endif /* DMATXRC */

void * wlc_bmac_dmatx_peeknexttxp(wlc_info_t *wlc, int fifo)
{
	return dma_peeknexttxp(WLC_HW_DI(wlc, fifo));
}

#ifdef BCM_DMA_CT
/*
 * XXX: WAR for CRWLDOT11M-2082
*/
#define	XXF(x, n)	((x) & ((n) - 1))
void BCMFASTPATH
wlc_bmac_sched_aqm_fifo(wlc_hw_info_t *wlc_hw)
{
	int fifo, i;

	if (wlc_hw->aqm_dma_sched_fifo_active) {
		return;
	}

	wlc_hw->aqm_dma_sched_fifo_active = TRUE;
	fifo = aqm_dma_get_current_fifo(wlc_hw->dmacommon);
	if (aqm_dma_prefetch_active(wlc_hw->aqm_di[fifo], fifo)) {
		wlc_hw->aqm_dma_sched_fifo_active = FALSE;
		return;
	}

	for (i = 0; i <  wlc_hw->nfifo_inuse; i++) {
		fifo = XXF((fifo + 1), wlc_hw->nfifo_inuse);
		if (fifo == TX_FIFO_6 || fifo == TX_FIFO_7) {
			fifo++;
			continue;
		}
		if (wlc_hw->aqm_di[fifo] && aqm_dma_sched_fifo(wlc_hw->aqm_di[fifo], fifo)) {
			aqm_dma_commit(wlc_hw->aqm_di[fifo], fifo);
			break;
		}
	}
	wlc_hw->aqm_dma_sched_fifo_active = FALSE;
}
#endif /* BCM_DMA_CT */
void
wlc_dma_map_pkts(wlc_hw_info_t *wlc_hw, map_pkts_cb_fn cb, void *ctx)
{
	uint i;
	hnddma_t *di;
	for (i = 0; i < wlc_hw->nfifo_inuse; i++) {
		di = wlc_hw->di[i];
		if (di) {
			dmatx_map_pkts(di, cb, ctx);
		}
	}
}

#ifdef BCMDBG_POOL
/*
 * Packet pool separates memory allocation for packets from other allocations, making the system
 * more robust to low memory conditions, and prevents DMA error conditions by reusing recently freed
 * packets in a faster manner.
 */
static void
wlc_pktpool_dbg_cb(pktpool_t *pool, void *arg)
{
	wlc_hw_info_t *wlc_hw = arg;
	pktpool_stats_t pstats;

	if (wlc_hw == NULL)
		return;

	if (wlc_hw->up == FALSE)
		return;

	WL_ERROR(("wl: post=%d rxactive=%d txactive=%d txpend=%d\n",
		NRXBUFPOST,
		dma_rxactive(wlc_hw->di[RX_FIFO]),
		dma_txactive(wlc_hw->di[1]),
		dma_txpending(wlc_hw->di[1])));

	pktpool_stats_dump(pool, &pstats);
	WL_ERROR(("pool len=%d\n", pktpool_len(pool)));
	WL_ERROR(("txdh:%d txd11:%d enq:%d rxdh:%d rxd11:%d rxfill:%d idle:%d\n",
		pstats.txdh, pstats.txd11, pstats.enq,
		pstats.rxdh, pstats.rxd11, pstats.rxfill, pstats.idle));
}
#endif /* BCMDBG_POOL */

static void
wlc_pktpool_empty_cb(pktpool_t *pool, void *arg)
{
	wlc_hw_info_t *wlc_hw = arg;

	if (wlc_hw == NULL)
		return;

	if (wlc_hw->up == FALSE)
		return;

#ifdef DMATXRC
	if (DMATXRC_ENAB(wlc_hw->wlc->pub) && !(PROP_TXSTATUS_ENAB(wlc_hw->wlc->pub)))
		wlc_dmatx_reclaim(wlc_hw);
#endif // endif
}

static void
wlc_pktpool_avail_cb(pktpool_t *pool, void *arg)
{
	wlc_hw_info_t *wlc_hw = arg;

	if (wlc_hw == NULL)
		return;

	if (wlc_hw->up == FALSE || wlc_hw->reinit)
		return;

#ifdef	WL_RXEARLYRC
	if (WL_RXEARLYRC_ENAB(wlc_hw->wlc->pub)) {
		if ((wlc_hw->rc_pkt_head == NULL)) {
			if ((dma_activerxbuf(wlc_hw->di[RX_FIFO]) < 4)) {
				void *prev = NULL;
				void *p;
				while ((p = dma_rx(wlc_hw->di[RX_FIFO])) != NULL) {
					if (wlc_hw->rc_pkt_head == NULL) {
						wlc_hw->rc_pkt_head = p;
					} else {
						PKTSETLINK(prev, p);
					}
					prev = p;
				}
			}
		}
	}
#endif /* WL_RXEARLYRC */
	if (!PIO_ENAB_HW(wlc_hw) && wlc_hw->di[RX_FIFO])
		dma_rxfill(wlc_hw->di[RX_FIFO]);
}

#ifdef WLRXOV

/* Throttle tx on rx fifo overflow: flow control related */

void
wlc_rxov_timer(void *arg)
{
	wlc_info_t *wlc = (wlc_info_t*)arg;

	ASSERT(wlc->rxov_active == TRUE);
	if (wlc->rxov_delay > RXOV_TIMEOUT_MIN) {
		/* Gradually back off rxfifo overflow */
		wlc->rxov_delay -= RXOV_TIMEOUT_BACKOFF;

		wl_add_timer(wlc->wl, wlc->rxov_timer, wlc->rxov_delay, FALSE);
		wlc->rxov_active = TRUE;
	} else {
		/* Restore tx params */
		if (N_ENAB(wlc->pub) && AMPDU_MAC_ENAB(wlc->pub))
			wlc_set_txmaxpkts(wlc, MAXTXPKTS_AMPDUMAC);

		wlc->rxov_delay = RXOV_TIMEOUT_MIN;
		wlc->rxov_active = FALSE;

		if (POOL_ENAB(wlc->pub->pktpool))
			pktpool_avail_notify_normal(wlc->osh, SHARED_POOL);
	}
}

void
wlc_rxov_int(wlc_info_t *wlc)
{
	if (wlc->rxov_active == FALSE) {
		if (POOL_ENAB(wlc->pub->pktpool))
			pktpool_avail_notify_exclusive(wlc->osh, SHARED_POOL, wlc_pktpool_avail_cb);
		/*
		 * Throttle tx when hitting rxfifo overflow
		 * Increase rx post??
		 */
		if (N_ENAB(wlc->pub) && AMPDU_MAC_ENAB(wlc->pub))
			wlc_set_txmaxpkts(wlc, wlc->rxov_txmaxpkts);

		wl_add_timer(wlc->wl, wlc->rxov_timer, wlc->rxov_delay, FALSE);
		wlc->rxov_active = TRUE;
	} else {
		/* Re-arm it */
		wlc->rxov_delay = MIN(wlc->rxov_delay*2, RXOV_TIMEOUT_MAX);
		wl_add_timer(wlc->wl, wlc->rxov_timer, wlc->rxov_delay, FALSE);
	}

	if (!PIO_ENAB_HW(wlc->hw) && wlc->hw->di[RX_FIFO])
		dma_rxfill(wlc->hw->di[RX_FIFO]);
}
#endif /* WLRXOV */

#ifdef BCMPCIEDEV
static void
BCMINITFN(wlc_bmac_set_dma_burstlen_pcie)(wlc_hw_info_t *wlc_hw, hnddma_t *di)
{
	uint32 devctl;
	uint32 mrrs;
	si_t *sih = wlc_hw->sih;

	si_corereg(sih, sih->buscoreidx, OFFSETOF(sbpcieregs_t, configaddr), ~0, 0xB4);
	devctl = si_corereg(sih, sih->buscoreidx, OFFSETOF(sbpcieregs_t, configdata), 0, 0);
	mrrs = (devctl & PCIE_CAP_DEVCTRL_MRRS_MASK) >> PCIE_CAP_DEVCTRL_MRRS_SHIFT;
	switch (mrrs)
	{
		case PCIE_CAP_DEVCTRL_MRRS_128B:
			dma_param_set(di, HNDDMA_PID_TX_BURSTLEN, DMA_BL_128);
			break;
		case PCIE_CAP_DEVCTRL_MRRS_256B:
			dma_param_set(di, HNDDMA_PID_TX_BURSTLEN, DMA_BL_256);
			break;
		case PCIE_CAP_DEVCTRL_MRRS_512B:
			dma_param_set(di, HNDDMA_PID_TX_BURSTLEN, DMA_BL_512);
			break;
		case PCIE_CAP_DEVCTRL_MRRS_1024B:
			dma_param_set(di, HNDDMA_PID_TX_BURSTLEN, DMA_BL_1024);
			break;
		default:
			break;
	}

	WL_INFORM(("MRRS Read from config reg %x \n", mrrs));

	if ((wlc_hw->sih->buscoretype == PCIE2_CORE_ID) && (wlc_hw->sih->buscorerev ==  5))
		dma_param_set(di, HNDDMA_PID_TX_BURSTLEN, DMA_BL_128);

}
#endif /* BCMPCIEDEV */

static uint16
BCMINITFN(wlc_bmac_dma_max_outstread)(wlc_hw_info_t *wlc_hw)
{
	uint16 txmr = (TXMR == 2) ? DMA_MR_2 : DMA_MR_1;
	if ((CHIPID(wlc_hw->sih->chip) == BCM4345_CHIP_ID) && (wlc_hw->sih->chiprev >= 2)) {
		txmr = DMA_MR_2;
#ifdef UNRELEASEDCHIP
	} else if (BCM4349_CHIP(wlc_hw->sih->chip)) {
		txmr = DMA_MR_1;
#endif /* UNRELEASEDCHIP */
	}
	//#endif /* UNRELEASEDCHIP */
	else if (((CHIPID(wlc_hw->sih->chip) == BCM4350_CHIP_ID) && (wlc_hw->sih->chiprev >= 3)) ||
		(CHIPID(wlc_hw->sih->chip) == BCM4354_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM4356_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM4358_CHIP_ID))
		txmr = DMA_MR_12;
	else if ((CHIPID(wlc_hw->sih->chip) == BCM43602_CHIP_ID) ||
		BCM4365_CHIP(wlc_hw->sih->chip) ||
		0)
		txmr = DMA_MR_12;

	return txmr;
}
static void
BCMINITFN(wlc_bmac_dma_param_set)(wlc_hw_info_t *wlc_hw, uint bustype, hnddma_t *di,
	bmac_dmactl_t *dmactl)
{
	if (bustype == PCI_BUS) {
		if (D11REV_GE(wlc_hw->corerev, 32)) {
			if ((CHIPID(wlc_hw->sih->chip) == BCM4360_CHIP_ID) ||
				(CHIPID(wlc_hw->sih->chip) == BCM43460_CHIP_ID) ||
				BCM4350_CHIP(wlc_hw->sih->chip) ||
				(CHIPID(wlc_hw->sih->chip) == BCM4352_CHIP_ID) ||
				(CHIPID(wlc_hw->sih->chip) == BCM43602_CHIP_ID) ||
				(CHIPID(wlc_hw->sih->chip) == BCM4335_CHIP_ID) ||
				BCM4365_CHIP(wlc_hw->sih->chip) ||
				0) {
				uint16 dma_mr = dmactl->txmr;
#ifdef DMA_MRENAB
				dma_mr = wlc_bmac_dma_max_outstread(wlc_hw);
#endif // endif
				dma_param_set(di, HNDDMA_PID_TX_MULTI_OUTSTD_RD, dma_mr);
				dma_param_set(di, HNDDMA_PID_TX_PREFETCH_CTL, dmactl->txpfc);
				dma_param_set(di, HNDDMA_PID_TX_PREFETCH_THRESH, dmactl->txpft);
				dma_param_set(di, HNDDMA_PID_TX_BURSTLEN, dmactl->txblen);
				dma_param_set(di, HNDDMA_PID_RX_PREFETCH_CTL, dmactl->rxpfc);
				dma_param_set(di, HNDDMA_PID_RX_PREFETCH_THRESH, dmactl->rxpft);
				dma_param_set(di, HNDDMA_PID_RX_BURSTLEN, dmactl->rxblen);
			} else {
				dma_burstlen_set(di, DMA_BL_128, DMA_BL_128);
			}
		}
	} else if (bustype == SI_BUS) {
			if (D11REV_GE(wlc_hw->corerev, 32)) {
				if ((CHIPID(wlc_hw->sih->chip) == BCM4360_CHIP_ID) ||
					(CHIPID(wlc_hw->sih->chip) == BCM4352_CHIP_ID) ||
					(CHIPID(wlc_hw->sih->chip) == BCM43460_CHIP_ID) ||
					(CHIPID(wlc_hw->sih->chip) == BCM4345_CHIP_ID) ||
					(CHIPID(wlc_hw->sih->chip) == BCM43602_CHIP_ID) ||
					BCM4350_CHIP(wlc_hw->sih->chip) ||
					(CHIPID(wlc_hw->sih->chip) == BCM43526_CHIP_ID) ||
					(CHIPID(wlc_hw->sih->chip) == BCM43460_CHIP_ID) ||
					(CHIPID(wlc_hw->sih->chip) == BCM4345_CHIP_ID) ||
#ifdef UNRELEASEDCHIP
					BCM4349_CHIP(wlc_hw->sih->chip) ||
#endif /* UNRELEASEDCHIP */
					(CHIPID(wlc_hw->sih->chip) == BCM4335_CHIP_ID)) {
					dma_param_set(di, HNDDMA_PID_TX_MULTI_OUTSTD_RD,
						wlc_bmac_dma_max_outstread(wlc_hw));

					dma_param_set(di, HNDDMA_PID_TX_PREFETCH_CTL,
						DMA_PC_0);
					dma_param_set(di, HNDDMA_PID_TX_PREFETCH_THRESH,
						DMA_PT_1);
#if defined(DONGLEBUILD) && defined(BCMSDIODEV_ENABLED)
					/* SWWLAN-27667 WAR */
					/* SWWLAN-38880 WAR for SDIO FIFO overrun or underrun issue.
					   MAC DMA burst length is reduced from 64 to 32 bytes
					   while maintaining SDIO DMA burst length to 64 bytes,
					   which effectively makes SDIO DMA faster than MAC DMA.
					*/
					dma_param_set(di, HNDDMA_PID_TX_BURSTLEN,
						DMA_BL_32);
					dma_param_set(di, HNDDMA_PID_RX_BURSTLEN,
						DMA_BL_32);
#else
					dma_param_set(di, HNDDMA_PID_TX_BURSTLEN,
						DMA_BL_64);
					dma_param_set(di, HNDDMA_PID_RX_BURSTLEN,
						DMA_BL_64);
#endif /* defined(DONGLEBUILD) && defined(BCMSDIODEV_ENABLED) */
					dma_param_set(di, HNDDMA_PID_RX_PREFETCH_CTL,
						DMA_PC_0);
					dma_param_set(di, HNDDMA_PID_RX_PREFETCH_THRESH,
						DMA_PT_1);

#ifdef BCMPCIEDEV

					/* addresses going across the bus */
					if (BCMPCIEDEV_ENAB() &&
					    (wlc_hw->sih->buscoretype == PCIE2_CORE_ID)) {
						wlc_bmac_set_dma_burstlen_pcie(wlc_hw, di);

#ifdef PCIE_PHANTOM_DEV
						dma_param_set(di, HNDDMA_PID_BURSTLEN_WAR, 1);
#else
						dma_param_set(di, HNDDMA_PID_BURSTLEN_CAP, 1);
#endif /* PCIE_PHANTOM_DEV */
					}
#endif /* BCMPCIEDEV */
				} else if (IS_AC2_DEV(wlc_hw->deviceid)) {
					dma_param_set(di, HNDDMA_PID_TX_MULTI_OUTSTD_RD,
						dmactl->txmr);
					dma_param_set(di, HNDDMA_PID_TX_PREFETCH_CTL,
						dmactl->txpfc);
					dma_param_set(di, HNDDMA_PID_TX_PREFETCH_THRESH,
						dmactl->txpft);
					dma_param_set(di, HNDDMA_PID_TX_BURSTLEN,
						dmactl->txblen);
					dma_param_set(di, HNDDMA_PID_RX_PREFETCH_CTL,
						dmactl->rxpfc);
					dma_param_set(di, HNDDMA_PID_RX_PREFETCH_THRESH,
						dmactl->rxpft);
					dma_param_set(di, HNDDMA_PID_RX_BURSTLEN,
						dmactl->rxblen);
#ifdef BCMPCIEDEV
					if (BCMPCIEDEV_ENAB() &&
					    (wlc_hw->sih->buscoretype == PCIE2_CORE_ID)) {
						wlc_bmac_set_dma_burstlen_pcie(wlc_hw, di);
						dma_param_set(di, HNDDMA_PID_BURSTLEN_CAP, 1);
					}
#endif /* BCMPCIEDEV */
				}
			}
	}
}

#if defined(WLC_HIGH)
static void
BCMINITFN(wlc_bmac_construct_dmactl)(wlc_tunables_t *tune, bmac_dmactl_t *dmactl)
{
	dmactl->txmr = (tune->txmr == 32 ? DMA_MR_32 :
			tune->txmr == 20 ? DMA_MR_20 :
			tune->txmr == 16 ? DMA_MR_16 :
			tune->txmr == 12 ? DMA_MR_12 :
			tune->txmr == 8 ? DMA_MR_8 :
			tune->txmr == 4 ? DMA_MR_4 :
			tune->txmr == 2 ? DMA_MR_2 : DMA_MR_1);
	dmactl->txpft = (tune->txpft == 8 ? DMA_PT_8 :
			tune->txpft == 4 ? DMA_PT_4 :
			tune->txpft == 2 ? DMA_PT_2 : DMA_PT_1);
	dmactl->txpfc = (tune->txpfc == 16 ? DMA_PC_16 :
			tune->txpfc == 8 ? DMA_PC_8 :
			tune->txpfc == 4 ? DMA_PC_4 : DMA_PC_0);
	dmactl->txblen = (tune->txblen == 1024 ? DMA_BL_1024 :
			tune->txblen == 512 ? DMA_BL_512 :
			tune->txblen == 256 ? DMA_BL_256 :
			tune->txblen == 128 ? DMA_BL_128 :
			tune->txblen == 64 ? DMA_BL_64 :
			tune->txblen == 32 ? DMA_BL_32 : DMA_BL_16);

	dmactl->rxpft =  (tune->rxpft == 8 ? DMA_PT_8 :
			tune->rxpft == 4 ? DMA_PT_4 :
			tune->rxpft == 2 ? DMA_PT_2 : DMA_PT_1);
	dmactl->rxpfc = (tune->rxpfc == 16 ? DMA_PC_16 :
			tune->rxpfc == 8 ? DMA_PC_8 :
			tune->rxpfc == 4 ? DMA_PC_4 : DMA_PC_0);
	dmactl->rxblen = (tune->rxblen == 1024 ? DMA_BL_1024 :
			tune->rxblen == 512 ? DMA_BL_512 :
			tune->rxblen == 256 ? DMA_BL_256 :
			tune->rxblen == 128 ? DMA_BL_128 :
			tune->rxblen == 64 ? DMA_BL_64 :
			tune->rxblen == 32 ? DMA_BL_32 : DMA_BL_16);
}
#endif /* WLC_HIGH */

/**
 * D11 core contains several TX FIFO's and one or two RX FIFO's. These FIFO's are fed by either a
 * DMA engine or programmatically (PIO).
 */
static bool
BCMINITFN(wlc_bmac_attach_dmapio)(wlc_hw_info_t *wlc_hw, bool wme)
{
	uint i;
	char name[8];
	/* ucode host flag 2 needed for pio mode, independent of band and fifo */
	wlc_info_t *wlc = wlc_hw->wlc;
	uint unit = wlc_hw->unit;
	wlc_tunables_t *tune = wlc->pub->tunables;
#ifndef BCM_OL_DEV
	/* For split rx case, we dont want any extra head room */
	/* pkt coming from d11 dma will be used only in PKT RX path */
	/* For RX path, we dont need to grow the packet at head */
	/* Pkt loopback within a dongle case may require some changes with this logic */
	int extraheadroom = (BCMSPLITRX_ENAB()) ? 0 : WLRXEXTHDROOM;
	uint8 splittx_hdr = (BCMLFRAG_ENAB() ? 1 : 0);
	uint nrxd_fifo1 = 0;
	uint rxbufpost_fifo1 = 0;
	bool fifo1_rxen;
#ifdef WME
	bool fifo2_rxen;
#endif /* WME */
#endif /* BCM_OL_DEV */

	dma_common_t *dmacommon = NULL;
	uint32 *indqsel_reg = NULL;
	uint32 *suspreq_reg = NULL;
	uint32 *flushreq_reg = NULL;

	/* name and offsets for dma_attach */
	snprintf(name, sizeof(name), rstr_wlD, unit, 0);

	/* init core's pio or dma channels */
	if (PIO_ENAB_HW(wlc_hw)) {
		/* skip if already initialized, for dual band, but single core
		 * MHF2 update by wlc_pio_attach is for PR38778 WAR. The WAR is
		 * independent of band and fifo.
		 */
		if (wlc_hw->pio[0] == 0) {
			pio_t *pio;

			for (i = 0; i < NFIFO; i++) {
				pio = wlc_pio_attach(wlc->pub, wlc, i);
				if (pio == NULL) {
					WL_ERROR(("wlc_attach: pio_attach failed\n"));
					return FALSE;
				}
				wlc_hw_set_pio(wlc_hw, i, pio);
			}
		}
	} else if (wlc_hw->di[RX_FIFO] == 0) {	/* Init FIFOs only if they are not initialized */

		uint addrwidth;
		osl_t *osh = wlc_hw->osh;
		hnddma_t *di;
		static bmac_dmactl_t dmactl = { DMA_MR_2, DMA_PC_16,
			DMA_PT_8, DMA_BL_1024, DMA_PC_16, DMA_PT_8, DMA_BL_128};

		/* Use the *_large tunable values for cores that support the larger DMA ring size,
		 * 4k descriptors.
		 */
		uint ntxd = (D11REV_GE(wlc_hw->corerev, 42)) ? tune->ntxd_large : tune->ntxd;
		uint nrxd = (D11REV_GE(wlc_hw->corerev, 42)) ? tune->nrxd_large : tune->nrxd;
		/* All data channels are indirect when CT is enabled. */
		uint32 dma_attach_flags = BCM_DMA_CT_ENAB(wlc) ? BCM_DMA_IND_INTF_FLAG : 0;

#ifdef BULK_PKTLIST
		dma_attach_flags |= BCM_DMA_BULK_PROCESSING;
#endif // endif
		if (D11REV_GE(wlc_hw->corerev, 64)) {
			dma_attach_flags |= BCM_DMA_CHAN_SWITCH_EN;
		}

		if (!wlc_hw->dmacommon) {
			/* init the dma_common instance */
			if (D11REV_GE(wlc_hw->corerev, 64)) {
				indqsel_reg = (uint32 *)(&(wlc_hw->regs->indqsel));
				suspreq_reg = (uint32 *)(&(wlc_hw->regs->suspreq));
				flushreq_reg = (uint32 *)(&(wlc_hw->regs->flushreq));
			}
			dmacommon = dma_common_attach(wlc_hw->osh, indqsel_reg, suspreq_reg,
				flushreq_reg);

			if (dmacommon == NULL) {
				WL_ERROR(("wl%d: wlc_attach: dma_common_attach failed\n", unit));
				return FALSE;
			}
			wlc_hw_set_dmacommon(wlc_hw, dmacommon);
		}

		/* Find out the DMA addressing capability and let OS know
		 * All the channels within one DMA core have 'common-minimum' same
		 * capability
		 */
		addrwidth = dma_addrwidth(wlc_hw->sih, DMAREG(wlc_hw, DMA_TX, 0));
		OSL_DMADDRWIDTH(osh, addrwidth);

		if (!wl_alloc_dma_resources(wlc->wl, addrwidth)) {
			WL_ERROR(("wl%d: wlc_attach: alloc_dma_resources failed\n", unit));
			return FALSE;
		}

#if !defined(BCM_GMAC3)
		STATIC_ASSERT(BCMEXTRAHDROOM >= TXOFF);
#endif // endif

#ifndef BCM_OL_DEV
		/*
		 * FIFO 0
		 * TX: TX_AC_BK_FIFO (TX AC Background data packets)
		 * RX: RX_FIFO (RX data packets)
		 */
		STATIC_ASSERT(TX_AC_BK_FIFO == 0);
		STATIC_ASSERT(RX_FIFO == 0);
		snprintf(name, sizeof(name), rstr_wlD, unit, 0);
		di = dma_attach_ext(wlc_hw->dmacommon, osh, name, wlc_hw->sih,
			(wme ? DMAREG(wlc_hw, DMA_TX, 0) : NULL), DMAREG(wlc_hw, DMA_RX, 0),
			dma_attach_flags, 0, (wme ? ntxd : 0), nrxd, tune->rxbufsz,
			extraheadroom, tune->nrxbufpost, wlc->hwrxoff, &wl_msg_level);
		if (di == NULL)
			goto dma_attach_fail;

		/* Set separate rx hdr flag only for fifo-0 */
		if (SPLIT_RXMODE1() || SPLIT_RXMODE2())
			dma_param_set(di, HNDDMA_SEP_RX_HDR, 1);

		if (RXFIFO_SPLIT())
			dma_param_set(di, HNDDMA_SPLIT_FIFO, SPLIT_FIFO_0);

#if defined(WLC_HIGH)
		if ((BUSTYPE(wlc_hw->sih->bustype) == PCI_BUS) ||
			IS_AC2_DEV(wlc_hw->deviceid)) {
			wlc_bmac_construct_dmactl(tune, &dmactl);
		}

		if (BUSTYPE(wlc_hw->sih->bustype) == PCI_BUS ||
			BUSTYPE(wlc_hw->sih->bustype) == SI_BUS) {
			wlc_bmac_dma_param_set(wlc_hw, BUSTYPE(wlc_hw->sih->bustype), di, &dmactl);
		}
#elif defined(WLC_LOW_ONLY)
		if (BUSTYPE(wlc_hw->sih->bustype) == SI_BUS) {
			wlc_bmac_dma_param_set(wlc_hw, SI_BUS, di, &dmactl);
		}
#endif	/* defined(WLC_HIGH) */
		wlc_hw_set_di(wlc_hw, 0, di);

#if defined(WLC_LOW_ONLY)
		/* calculate memory required to replenish  rx buffer */
		wlc_hw->mem_required_def = (wlc->pub->tunables->rxbufsz + BCMEXTRAHDROOM) *
			(wlc->pub->tunables->rxbnd);
		wlc_hw->mem_required_lower = wlc_hw->mem_required_def -
			(wlc_hw->mem_required_def >> 2) +
			+ wlc->pub->tunables->dngl_mem_restrict_rxdma;
		wlc_hw->mem_required_least = (wlc_hw->mem_required_def >> 1) +
			wlc->pub->tunables->dngl_mem_restrict_rxdma;
		wlc_hw->mem_required_def += wlc->pub->tunables->dngl_mem_restrict_rxdma;
#endif /* defined(WLC_LOW_ONLY) */

		/*
		 * FIFO 1
		 * TX: TX_AC_BE_FIFO (TX AC Best-Effort data packets)
		 *   (legacy) TX_DATA_FIFO (TX data packets)
		 * RX: UNUSED
		 */
		STATIC_ASSERT(TX_AC_BE_FIFO == 1);
		STATIC_ASSERT(TX_DATA_FIFO == 1);
		ASSERT(wlc_hw->di[1] == 0);
		/* if fifo-1 is used for classification, use classiifcation specific tunables */
		fifo1_rxen = (PKT_CLASSIFY_EN(RX_FIFO1) || RXFIFO_SPLIT());
		nrxd_fifo1 = (fifo1_rxen ?
			(PKT_CLASSIFY_EN(RX_FIFO1) ? tune->nrxd_classified_fifo : nrxd) : 0);
		rxbufpost_fifo1 = (PKT_CLASSIFY_EN(RX_FIFO1) ? tune->bufpost_classified_fifo :
			tune->nrxbufpost);
		extraheadroom = (PKT_CLASSIFY_EN(RX_FIFO1) ? WLRXEXTHDROOM : 0);
#ifdef	FORCE_RX_FIFO1
		/* in 4349a0, fifo-2 classification will work only if fifo-1 is enabled */
		/* Enable fifo-1, but dont do any posting */
		fifo1_rxen = TRUE;
		nrxd_fifo1 = 1;
		rxbufpost_fifo1 = 0;
#endif /* FORCE_RX_FIFO1 */
		/* Since we are splitting up TCM buffers, increase no of descriptors */
		/* if splitrx is enabled, fifo-1 needs to be inited for rx too */
		snprintf(name, sizeof(name), rstr_wlD, unit, 1);
		di = dma_attach_ext(wlc_hw->dmacommon, osh, name, wlc_hw->sih,
			(wme ? DMAREG(wlc_hw, DMA_TX, 1) : NULL),
			(fifo1_rxen ? DMAREG(wlc_hw, DMA_RX, 1) : NULL),
			dma_attach_flags, 1,
			(splittx_hdr ? tune->ntxd_lfrag : ntxd),
			nrxd_fifo1, tune->rxbufsz, extraheadroom,
			rxbufpost_fifo1, wlc->hwrxoff, &wl_msg_level);

		if (di == NULL)
			goto dma_attach_fail;

		wlc_bmac_dma_param_set(wlc_hw, BUSTYPE(wlc_hw->sih->bustype), di, &dmactl);
		wlc_hw_set_di(wlc_hw, 1, di);

		if (RXFIFO_SPLIT())
			dma_param_set(di, HNDDMA_SPLIT_FIFO, SPLIT_FIFO_1);

#else
		/*
		 * FIFO 1
		 * TX: TX_AC_BE_FIFO (TX AC Best-Effort data packets)
		 *	 (legacy) TX_DATA_FIFO (TX data packets)
		 * RX: UNUSED
		 */

		STATIC_ASSERT(RX_FIFO == 1);
		ASSERT(wlc_hw->di[1] == 0);

		di = dma_attach_ext(wlc_hw->dmacommon, osh, name, wlc_hw->sih,
			NULL, DMAREG(wlc_hw, DMA_RX, RX_FIFO), dma_attach_flags, 1,
			0, nrxd, tune->rxbufsz, -1, tune->nrxbufpost,
			wlc->hwrxoff, &wl_msg_level);
		if (di == NULL)
			goto dma_attach_fail;

		wlc_bmac_dma_param_set(wlc_hw, BUSTYPE(wlc_hw->sih->bustype), di, &dmactl);
		wlc_hw_set_di(wlc_hw, RX_FIFO, di);

#endif /* BCM_OL_DEV */

#ifndef BCM_OL_DEV
#ifdef WME
		/*
		 * FIFO 2
		 * TX: TX_AC_VI_FIFO (TX AC Video data packets)
		 * RX: UNUSED
		 */
		STATIC_ASSERT(TX_AC_VI_FIFO == 2);
		fifo2_rxen = (PKT_CLASSIFY_EN(RX_FIFO2));
		/* if splitrx mode 3 is enabled, fifo-2 needs to be inited for rx too */
		if (wme || fifo2_rxen) {
			snprintf(name, sizeof(name), rstr_wlD, unit, 2);
			di = dma_attach_ext(wlc_hw->dmacommon, osh, name, wlc_hw->sih,
				(wme ? DMAREG(wlc_hw, DMA_TX, 2):NULL),
				(fifo2_rxen ? DMAREG(wlc_hw, DMA_RX, 2) : NULL),
				dma_attach_flags, 2, ntxd,
				(fifo2_rxen ? tune->nrxd_classified_fifo : 0),
				tune->rxbufsz,
				WLRXEXTHDROOM, tune->bufpost_classified_fifo,
				wlc->hwrxoff, &wl_msg_level);

			if (di == NULL) {
				goto dma_attach_fail;
			}

			wlc_bmac_dma_param_set(wlc_hw, BUSTYPE(wlc_hw->sih->bustype), di, &dmactl);

			wlc_hw_set_di(wlc_hw, 2, di);
		}
#endif /* WME */

		/*
		 * FIFO 3
		 * TX: TX_AC_VO_FIFO (TX AC Voice data packets)
		 *   (legacy) TX_CTL_FIFO (TX control & mgmt packets)
		 * RX: RX_TXSTATUS_FIFO (transmit-status packets)
		 *	for corerev < 5 only
		 */
		STATIC_ASSERT(TX_AC_VO_FIFO == 3);
		STATIC_ASSERT(TX_CTL_FIFO == 3);
		if (D11REV_IS(wlc_hw->corerev, 4)) {
			STATIC_ASSERT(RX_TXSTATUS_FIFO == 3);
			di = dma_attach_ext(wlc_hw->dmacommon, osh, name, wlc_hw->sih,
				DMAREG(wlc_hw, DMA_TX, 3), DMAREG(wlc_hw, DMA_RX, 3),
				dma_attach_flags, 3, ntxd,
				nrxd, sizeof(tx_status_t), -1, tune->nrxbufpost, 0,
				&wl_msg_level);
		} else {
			di = dma_attach_ext(wlc_hw->dmacommon, osh, name, wlc_hw->sih,
				DMAREG(wlc_hw, DMA_TX, 3), NULL, dma_attach_flags, 3,
				ntxd, 0, 0, -1, 0, 0, &wl_msg_level);
		}
		if (di == NULL)
			goto dma_attach_fail;

		wlc_bmac_dma_param_set(wlc_hw, BUSTYPE(wlc_hw->sih->bustype), di, &dmactl);

		wlc_hw_set_di(wlc_hw, 3, di);

#ifdef AP
		/*
		 * FIFO 4
		 * TX: TX_BCMC_FIFO (TX broadcast & multicast packets)
		 * RX: UNUSED
		 */

		STATIC_ASSERT(TX_BCMC_FIFO == 4);
		di = dma_attach_ext(wlc_hw->dmacommon, osh, name, wlc_hw->sih,
			DMAREG(wlc_hw, DMA_TX, 4), NULL, dma_attach_flags, 4, ntxd,
			0, 0, -1, 0, 0, &wl_msg_level);
		if (di == NULL)
			goto dma_attach_fail;

		wlc_bmac_dma_param_set(wlc_hw, BUSTYPE(wlc_hw->sih->bustype), di, &dmactl);

		wlc_hw_set_di(wlc_hw, 4, di);
#endif /* AP */

#endif /* BCM_OL_DEV */

#if defined(MBSS) || defined(BCM_OL_DEV) || defined(WLAIBSS)
		{
			bool attach_dma = FALSE;

#if defined(MBSS) || defined(BCM_OL_DEV) || defined(WLAIBSS)
			attach_dma = TRUE;
#else
			if (WLNINTENDO_ENAB(wlc)) {
				attach_dma = TRUE;
			}
#endif /* BCM_OL_DEV */
#if defined(BCM_DMA_CT) && !defined(BCM_DMA_CT_DISABLED)
			if (BCM_DMA_CT_ENAB(wlc_hw->wlc) && (D11REV_IS(wlc_hw->corerev, 64))) {
				/* XXX: CRWLDOT11M-2050 : attach all DMAs so that ChannelSwitch
				 * gets latched.
				 */
				attach_dma = TRUE;
			}
#endif // endif

			if (attach_dma) {
				/*
				 * FIFO 5: TX_ATIM_FIFO
				 * TX: MBSS: but used for beacon/probe resp pkts
				 * TX: WLNINTENDO: TX Nintendo Nitro protocol packets
				 * RX: UNUSED
				 */
				di = dma_attach_ext(wlc_hw->dmacommon, osh, name, wlc_hw->sih,
					DMAREG(wlc_hw, DMA_TX, 5), NULL, dma_attach_flags, 5,
					ntxd, 0, 0, -1, 0, 0, &wl_msg_level);
				if (di == NULL)
					goto dma_attach_fail;

				wlc_bmac_dma_param_set(wlc_hw,
					BUSTYPE(wlc_hw->sih->bustype), di, &dmactl);

				wlc_hw_set_di(wlc_hw, 5, di);
			}
		}
#endif // endif

		if (D11REV_GE(wlc_hw->corerev, 64)) {
#if defined(BCM_DMA_CT) && !defined(BCM_DMA_CT_DISABLED)
			if (BCM_DMA_CT_ENAB(wlc)) {
				uint ntxd_aqm = ntxd;
				uint ntxd_adj;

#if !defined(WL_MU_TX) || defined(WL_MU_TX_DISABLED)
				uint nfifo_local = NFIFO;
#else
				uint nfifo_local;
				if (D11REV_IS(wlc->pub->corerev, 64)) {
					/* For 4366B1 we use high fifos starting from 16, and two
					 * fifos for each client.
					 */
					nfifo_local = wlc->pub->_mutx_max ?
						(TX_FIFO_16 + wlc->pub->_mutx_max * 2) : NFIFO;
				} else {
					/* NFIFO + 2 (reserved) + 3 * mutxmax */
					nfifo_local = wlc->pub->_mutx_max ?
						(NFIFO + 2 + wlc->pub->_mutx_max * 3) : NFIFO;
				}

				/* Cannnot use MU_TX_ENAB here as the MU_TX and Beamforming modules
				 * are attached after dma_attach
				 */
				/* attach all the remaining FIFOs needed for MU TX */
				for (i = TX_FIFO_MU_START; i < nfifo_local; i++) {
					/* Check and adjust this DATA FIFO */
					ntxd_adj = ntxd;
					if (wlc_mutx_ntxd_adj(wlc, i, &ntxd_adj, NULL) != BCME_OK)
						continue;

					snprintf(name, sizeof(name), rstr_wlD, unit, i);
					di = dma_attach_ext(wlc_hw->dmacommon, osh, name,
						wlc_hw->sih,
						(void*)(uintptr)(&(wlc_hw->regs->inddma.dma)),
						NULL,  dma_attach_flags, (uint8)i, ntxd_adj,
						0, 0, -1, 0, 0, &wl_msg_level);
					if (di == NULL) {
						WL_ERROR(("wl%d: dma_attach inddma failed"
							"NFIFO_EXT=%d\n", unit, i));
						goto dma_attach_fail;
					}
					wlc_bmac_dma_param_set(wlc_hw,
						BUSTYPE(wlc_hw->sih->bustype), di, &dmactl);
					wlc_hw_set_di(wlc_hw, i, di);
				}
#endif /* !WL_MU_TX || WL_MU_TX_DISABLED */

				/* for AQM DMAs set the BCM_DMA_DESC_ONLY_FLAG also */
				dma_attach_flags = BCM_DMA_IND_INTF_FLAG | BCM_DMA_DESC_ONLY_FLAG |
					BCM_DMA_CHAN_SWITCH_EN;

				/* SWMUMIMO-106: WAR for AQM DMA's CD not being updated correctly
				 * by HW in CT mode.
				 * HW JIRA - CRWLDOT11M-1925, Relevant information in RB 39395.
				 */
				if (D11REV_IS(wlc_hw->corerev, 64)) {
					dma_attach_flags |= BCM_DMA_DESC_CD_WAR_FLAG;
				}

				ntxd_aqm = MIN(256, ntxd);
				/* as needed attach all the AQM DMAs */
				for (i = 0; i < nfifo_local; i++) {
					ntxd_adj = ntxd_aqm;

#if defined(WL_MU_TX) && !defined(WL_MU_TX_DISABLED)
					/* Check and adjust this AQM FIFO */
					if (wlc_mutx_ntxd_adj(wlc, i, NULL, &ntxd_adj) != BCME_OK)
						continue;
#endif /* WL_MU_TX && !WL_MU_TX_DISABLED */

					snprintf(name, sizeof(name), rstr_wl_aqmD, unit, i);
					di = dma_attach_ext(wlc_hw->dmacommon, osh, name,
						wlc_hw->sih,
						(void*)(uintptr)(&(wlc_hw->regs->indaqm.dma)),
						NULL,  dma_attach_flags, (uint8)i, ntxd_adj,
						0, 0, -1, 0, 0, &wl_msg_level);
					if (di == NULL) {
						WL_ERROR(("wl%d: dma_attach indaqm failed "
							"NFIFO_EXT=%d\n", unit, i));
						goto dma_attach_fail;
					}
					wlc_bmac_dma_param_set(wlc_hw,
						BUSTYPE(wlc_hw->sih->bustype), di, &dmactl);
					wlc_hw_set_aqm_di(wlc_hw, i, di);

				} /* for */
			}
#endif /* defined(BCM_DMA_CT) && !defined(BCM_DMA_CT_DISABLED) */
		} /* if (D11REV_GE(wlc_hw->corerev, 64)) */

		/* get pointer to dma engine tx flow control variable */
		for (i = 0; i < NFIFO_EXT; i++)
			if (wlc_hw->di[i]) {
				wlc_hw->txavail[i] =
					(uint*)dma_getvar(wlc_hw->di[i], "&txavail");
#if defined(BCM_DMA_CT) && !defined(BCM_DMA_CT_DISABLED)
				if (wlc_hw->aqm_di[i]) {
					wlc_hw->txavail_aqm[i] =
						(uint*)dma_getvar(wlc_hw->aqm_di[i],
						"&txavail");
				}
#endif /* defined(BCM_DMA_CT) && !defined(BCM_DMA_CT_DISABLED) */
#ifdef PCIE_PHANTOM_DEV
				/* Extra dma allocs required for devices which require bl war  */
				/* need to wait till dma params are set */
				if (dma_blwar_alloc(wlc_hw->di[i]))
					goto dma_attach_fail;
#endif // endif
				/* FIXIT, shouldnt be restting to 64 bytes un conditionally */
#ifdef BCMPCIEDEV
				if (!BCMPCIEDEV_ENAB())
#endif /* BCMPCIEDEV */
				{
					/* XXX:
					 * XXX: Default burstlen of zero caused txfifo overrun
					 * XXX: when using ucagg; caused PSM watchdog/random
					 * XXX: asserts in txstatus path
					 * XXX: JIRA:SWWLAN-32986
					 */
					if (CHIPID(wlc_hw->sih->chip) == BCM43143_CHIP_ID) {
						dma_param_set(wlc_hw->di[i], HNDDMA_PID_TX_BURSTLEN,
							DMA_BL_64);
						dma_param_set(wlc_hw->di[i], HNDDMA_PID_RX_BURSTLEN,
							DMA_BL_64);
					}
				}
			}
	}

	if (BCMSPLITRX_ENAB()) {
		/* enable host flags to do ucode frame classification */
		wlc_bmac_enable_rx_hostmem_access(wlc_hw, TRUE);
	}

	if (PKT_CLASSIFY()) {
		/* enable host flags to do ucode frame classification */
		wlc_bmac_enable_rx_hostmem_access(wlc_hw, TRUE);
	}

	if (RXFIFO_SPLIT())
		dma_link_handle(wlc_hw->di[RX_FIFO1], wlc_hw->di[RX_FIFO]);

	return TRUE;

dma_attach_fail:
	WL_ERROR(("wl%d: wlc_attach: dma_attach failed\n", unit));
	return FALSE;
}

static void
BCMINITFN(wlc_bmac_detach_dmapio)(wlc_hw_info_t *wlc_hw)
{
	uint j;

	for (j = 0; j < NFIFO_EXT; j++) {
		if (!PIO_ENAB_HW(wlc_hw)) {
			if (wlc_hw->di[j]) {
				dma_detach(wlc_hw->di[j]);
				wlc_hw_set_di(wlc_hw, j, NULL);
			}
		} else {
			if ((j < NFIFO) && (wlc_hw->pio[j])) {
				wlc_pio_detach(wlc_hw->pio[j]);
				wlc_hw_set_pio(wlc_hw, j, NULL);
			}
		}
	}

#if defined(BCM_DMA_CT) && !defined(BCM_DMA_CT_DISABLED)
	/* Detach all the AQM DMAs */
	for (j = 0; j < NFIFO_EXT; j++) {
		if (wlc_hw->aqm_di[j]) {
			dma_detach(wlc_hw->aqm_di[j]);
			wlc_hw_set_aqm_di(wlc_hw, j, NULL);
		}
	}
#endif /* defined(BCM_DMA_CT) && !defined(BCM_DMA_CT_DISABLED) */

	if (wlc_hw->dmacommon) {
		/* detach dma_common */
		dma_common_detach(wlc_hw->dmacommon);
		wlc_hw_set_dmacommon(wlc_hw, NULL);
	}
}

/** DMA segment list related */
static uint
wlc_bmac_dma_avoidance_cnt(wlc_hw_info_t *wlc_hw)
{
	uint i, total = 0;
	/* get total DMA avoidance counts */
	for (i = 0; i < NFIFO; i++)
		if (wlc_hw->di[i])
			total += dma_avoidance_cnt(wlc_hw->di[i]);

	return (total);
}

#define GPIO_4_BTSWITCH          (1 << 4)
#define GPIO_4_GPIOOUT_DEFAULT    0
#define GPIO_4_GPIOOUTEN_DEFAULT  0

/* Bluetooth switch drives multiple outputs */
int
wlc_bmac_set_btswitch(wlc_hw_info_t *wlc_hw, int8 state)
{
	if (((CHIPID(wlc_hw->sih->chip) == BCM4331_CHIP_ID) ||
	     (CHIPID(wlc_hw->sih->chip) == BCM43431_CHIP_ID)) &&
	    ((wlc_hw->sih->boardtype == BCM94331X28) ||
	     (wlc_hw->sih->boardtype == BCM94331X28B) ||
	     (wlc_hw->sih->boardtype == BCM94331CS_SSID) ||
	     (wlc_hw->sih->boardtype == BCM94331X29B) ||
	     (wlc_hw->sih->boardtype == BCM94331X29D))) {
		if (state == AUTO) {
			/* default */
			if (wlc_hw->up) {
				wlc_bmac_set_ctrl_bt_shd0(wlc_hw, TRUE);
			}
			si_gpioout(wlc_hw->sih, GPIO_4_BTSWITCH, GPIO_4_GPIOOUT_DEFAULT,
			           GPIO_DRV_PRIORITY);
			si_gpioouten(wlc_hw->sih, GPIO_4_BTSWITCH, GPIO_4_GPIOOUTEN_DEFAULT,
			             GPIO_DRV_PRIORITY);
		} else {
			uint32 val = 0;
			if (state == ON) {
				val = GPIO_4_BTSWITCH;
			}
			wlc_bmac_set_ctrl_bt_shd0(wlc_hw, FALSE);

			si_gpioout(wlc_hw->sih, GPIO_4_BTSWITCH, val, GPIO_DRV_PRIORITY);
			si_gpioouten(wlc_hw->sih, GPIO_4_BTSWITCH, GPIO_4_BTSWITCH,
			             GPIO_DRV_PRIORITY);
		}
		/* Save switch state */
		wlc_hw->btswitch_ovrd_state = state;
		return BCME_OK;
	} else if (WLCISACPHY(wlc_hw->band)) {
		if (wlc_hw->up) {
			wlc_phy_set_femctrl_bt_wlan_ovrd(wlc_hw->band->pi, state);
			/* Save switch state */
			wlc_hw->btswitch_ovrd_state = state;
			return BCME_OK;
		} else {
			return BCME_NOTUP;
		}
	} else {
		return BCME_UNSUPPORTED;
	}
}

#ifdef WLC_LOW_ONLY
bool
BCMATTACHFN(wlc_chipmatch)(uint16 vendor, uint16 device)
{
	if (vendor != VENDOR_BROADCOM) {
		WL_ERROR(("wlc_chipmatch: unknown vendor id %04x\n", vendor));
		return (FALSE);
	}

	if (device == BCM4306_D11G_ID)
		return (TRUE);
	if (device == BCM4306_D11G_ID2)
		return (TRUE);
	if (device == BCM4303_D11B_ID)
		return (TRUE);
	if (device == BCM4306_D11A_ID)
		return (TRUE);
	if (device == BCM4306_D11DUAL_ID)
		return (TRUE);
	if (device == BCM4318_D11G_ID)
		return (TRUE);
	if (device == BCM4318_D11DUAL_ID)
		return (TRUE);
	if (device == BCM4318_D11A_ID)
		return (TRUE);
	if (device == BCM4311_D11G_ID)
		return (TRUE);
	if (device == BCM4311_D11A_ID)
		return (TRUE);
	if (device == BCM4311_D11DUAL_ID)
		return (TRUE);
	if (device == BCM4328_D11DUAL_ID)
		return (TRUE);
	if (device == BCM4328_D11G_ID)
		return (TRUE);
	if (device == BCM4328_D11A_ID)
		return (TRUE);
	if (device == BCM4325_D11DUAL_ID)
		return (TRUE);
	if (device == BCM4325_D11G_ID)
		return (TRUE);
	if (device == BCM4325_D11A_ID)
		return (TRUE);
	if ((device == BCM4321_D11N_ID) || (device == BCM4321_D11N2G_ID) ||
		(device == BCM4321_D11N5G_ID))
		return (TRUE);
	if ((device == BCM4322_D11N_ID) || (device == BCM4322_D11N2G_ID) ||
		(device == BCM4322_D11N5G_ID))
		return (TRUE);
	if (device == BCM4322_CHIP_ID)	/* 4322/43221 without OTP/SROM */
		return (TRUE);
	if (device == BCM43221_D11N2G_ID)
		return (TRUE);
	if (device == BCM43231_D11N2G_ID)
		return (TRUE);
	if ((device == BCM43222_D11N_ID) || (device == BCM43222_D11N2G_ID) ||
		(device == BCM43222_D11N5G_ID))
		return (TRUE);
	if ((device == BCM43224_D11N_ID) || (device == BCM43225_D11N2G_ID) ||
		(device == BCM43421_D11N_ID) || (device == BCM43224_D11N_ID_VEN1))
		return (TRUE);
	if (device == BCM43226_D11N_ID)
		return (TRUE);
	if ((device == BCM6362_D11N_ID) || (device == BCM6362_D11N2G_ID) ||
		(device == BCM6362_D11N5G_ID))
		return (TRUE);

	if (device == BCM4329_D11N2G_ID)
		return (TRUE);
	if (device == BCM4315_D11DUAL_ID)
		return (TRUE);
	if (device == BCM4315_D11G_ID)
		return (TRUE);
	if (device == BCM4315_D11A_ID)
		return (TRUE);
	if ((device == BCM4319_D11N_ID) || (device == BCM4319_D11N2G_ID) ||
		(device == BCM4319_D11N5G_ID))
		return (TRUE);
	if (device == BCM4716_CHIP_ID)
		return (TRUE);
	if (device == BCM4748_CHIP_ID)
		return (TRUE);
	if (device == BCM4313_D11N2G_ID)
		return (TRUE);
	if (device == BCM4336_D11N_ID)
		return (TRUE);
	if (device == BCM4330_D11N_ID)
		return (TRUE);
	if ((device == BCM43236_D11N_ID) || (device == BCM43236_D11N2G_ID) ||
		(device == BCM43236_D11N5G_ID))
		return (TRUE);
	if ((device == BCM4331_D11N_ID) || (device == BCM4331_D11N2G_ID) ||
		(device == BCM4331_D11N5G_ID))
		return (TRUE);
	if (device == BCM43131_D11N2G_ID)
		return (TRUE);
	if ((device == BCM43227_D11N2G_ID) || (device == BCM43228_D11N_ID) ||
		(device == BCM43228_D11N5G_ID) || (device == BCM43217_D11N2G_ID))
		return (TRUE);
	if ((device == BCM43237_D11N_ID) || (device == BCM43237_D11N5G_ID))
		return (TRUE);
	if (device == BCM43362_D11N_ID)
		return (TRUE);
	if ((device == BCM4334_D11N_ID) || (device == BCM4334_D11N2G_ID) ||
		(device == BCM4334_D11N5G_ID))
		return (TRUE);
	if (device == BCM4314_D11N2G_ID)
		return (TRUE);
	if (device == BCM4324_D11N_ID)
		return (TRUE);
	if ((device == BCM43242_D11N_ID) || (device == BCM43242_D11N2G_ID) ||
		(device == BCM43242_D11N5G_ID))
		return (TRUE);
	if ((device == BCM4360_D11AC_ID) || (device == BCM4360_D11AC2G_ID) ||
		(device == BCM4360_D11AC5G_ID))
		return (TRUE);
	if ((device == BCM4335_D11AC_ID) || (device == BCM4335_D11AC2G_ID) ||
		(device == BCM4335_D11AC5G_ID))
		return (TRUE);
	if ((device == BCM43602_D11AC_ID) || (device == BCM43602_D11AC2G_ID) ||
		(device == BCM43602_D11AC5G_ID))
		return (TRUE);
#ifdef UNRELEASEDCHIP
	if ((device == BCM4349_D11AC_ID) || (device == BCM4349_D11AC2G_ID) ||
		(device == BCM4349_D11AC5G_ID))
		return (TRUE);
	if ((device == BCM4355_D11AC_ID) || (device == BCM4355_D11AC2G_ID) ||
		(device == BCM4355_D11AC5G_ID))
		return (TRUE);
	if ((device == BCM4359_D11AC_ID) || (device == BCM4359_D11AC2G_ID) ||
		(device == BCM4359_D11AC5G_ID))
		return (TRUE);
#endif /* UNRELEASEDCHIP */
	if ((device == BCM4365_D11AC_ID) || (device == BCM4365_D11AC2G_ID) ||
		(device == BCM4365_D11AC5G_ID))
		return (TRUE);
	if ((device == BCM4366_D11AC_ID) || (device == BCM4366_D11AC2G_ID) ||
		(device == BCM4366_D11AC5G_ID))
		return (TRUE);
	if ((device == BCM4345_D11AC_ID) || (device == BCM4345_D11AC2G_ID) ||
		(device == BCM4345_D11AC5G_ID))
		return (TRUE);
	if ((device == BCM4352_D11AC_ID) || (device == BCM4352_D11AC2G_ID) ||
		(device == BCM4352_D11AC5G_ID))
		return (TRUE);
	if (device == BCM43143_D11N2G_ID)
		return (TRUE);
	if ((device == BCM43341_D11N_ID) || (device == BCM43341_D11N2G_ID) ||
		(device == BCM43341_D11N5G_ID))
		return (TRUE);
	if ((device == BCM4350_D11AC_ID) || (device == BCM4350_D11AC2G_ID) ||
		(device == BCM4350_D11AC5G_ID))
		return (TRUE);
	if ((device == BCM43556_D11AC_ID) || (device == BCM43556_D11AC2G_ID) ||
		(device == BCM43556_D11AC5G_ID))
		return (TRUE);
	if ((device == BCM43558_D11AC_ID) || (device == BCM43558_D11AC2G_ID) ||
		(device == BCM43558_D11AC5G_ID))
		return (TRUE);
	if ((device == BCM43566_D11AC_ID) || (device == BCM43566_D11AC2G_ID) ||
		(device == BCM43566_D11AC5G_ID))
		return (TRUE);
	if ((device == BCM43568_D11AC_ID) || (device == BCM43568_D11AC2G_ID) ||
		(device == BCM43568_D11AC5G_ID))
		return (TRUE);
	if ((device == BCM43569_D11AC_ID) || (device == BCM43569_D11AC2G_ID) ||
		(device == BCM43569_D11AC5G_ID))
		return (TRUE);
	if ((device == BCM4354_D11AC_ID) || (device == BCM4354_D11AC2G_ID) ||
		(device == BCM4354_D11AC5G_ID))
		return (TRUE);
	if ((device == BCM4356_D11AC_ID) || (device == BCM4356_D11AC2G_ID) ||
		(device == BCM4356_D11AC5G_ID))
		return (TRUE);
	if ((device == BCM4358_D11AC_ID) || (device == BCM4358_D11AC2G_ID) ||
		(device == BCM4358_D11AC5G_ID))
		return (TRUE);
	WL_ERROR(("wlc_chipmatch: unknown device id %04x\n", device));
	return (FALSE);
}
#endif /* WLC_LOW_ONLY */

/** Switch between host and ucode AMPDU aggregation */
void
wlc_bmac_ampdu_set(wlc_hw_info_t *wlc_hw, uint8 mode)
{
	if ((D11REV_IS(wlc_hw->corerev, 26) || D11REV_IS(wlc_hw->corerev, 29))) {
		if (mode == AMPDU_AGG_HW)
			memcpy(wlc_hw->xmtfifo_sz, xmtfifo_sz_hwagg, sizeof(xmtfifo_sz_hwagg));
		else
			memcpy(wlc_hw->xmtfifo_sz, xmtfifo_sz_hostagg, sizeof(xmtfifo_sz_hostagg));
	}
}

#if defined(SAVERESTORE)
/* conserves power by powering off parts of the chip when idle */

static CONST uint32 *
BCMPREATTACHFN(wlc_bmac_sr_params_get)(wlc_hw_info_t *wlc_hw, uint32 *offset, uint32 *srfwsz)
{
	CONST uint32 *srfw = sr_get_sr_params(wlc_hw->sih, srfwsz, offset);

	if (D11REV_IS(wlc_hw->corerev, 48) || D11REV_IS(wlc_hw->corerev, 49))
		*offset += D11MAC_BMC_SRASM_OFFSET - (D11MAC_BMC_STARTADDR_SRASM << 8);
	else if (D11REV_IS(wlc_hw->corerev, 50))
			*offset <<= 1;
	return srfw;
}

#ifdef BCMDBG_SR
/*
 * SR sanity check
 * - ASM code is expected to be constant so compare original with txfifo
 */
static int
wlc_bmac_sr_verify(wlc_hw_info_t *wlc_hw, struct bcmstrbuf *b)
{
	int i;
	uint32 offset = 0;
	uint32 srfwsz = 0;
	CONST uint32 *srfw;
	uint32 c1, c2;
	bool asm_pass = TRUE;

	bcm_bprintf(b, "SR ASM:\n");
	if (!wlc_hw->wlc->clk) {
		bcm_bprintf(b, "No clk\n");
		return BCME_NOCLK;
	}

	srfw = wlc_bmac_sr_params_get(wlc_hw, &offset, &srfwsz);

	/* The template region starts where the BMC_STARTADDR starts.
	 * This shouldn't use a #defined value but some parameter in a
	 * global struct.
	 */
	if (D11REV_IS(wlc_hw->corerev, 48) || D11REV_IS(wlc_hw->corerev, 49))
		offset += (D11MAC_BMC_STARTADDR_SRASM << 8);
	wlc_bmac_templateptr_wreg(wlc_hw, offset);
	bcm_bprintf(b, "len: %d offset: 0x%x ", srfwsz, wlc_bmac_templateptr_rreg(wlc_hw));

	for (i = 0; i < (srfwsz/4); i++) {
		c1 = *srfw++;
		c2 = wlc_bmac_templatedata_rreg(wlc_hw);

		if (c1 != c2) {
			bcm_bprintf(b, "\ncmp failed: %d - 0x%x exp: 0x%x got: 0x%x\n", i,
			wlc_bmac_templateptr_rreg(wlc_hw), c1, c2);
			asm_pass = FALSE;
			break;
		}
	}

	bcm_bprintf(b, "\ncmp: %s", asm_pass ? "PASS" : "FAIL");
	bcm_bprintf(b, "\n");
	return 0;
}
#endif /* BCMDBG_SR */

/** S/R binary code is written into the D11 TX FIFO */
static int
BCMPREATTACHFN(wlc_bmac_sr_asm_download)(wlc_hw_info_t *wlc_hw)
{
	uint32 offset = 0;
	uint32 srfwsz = 0;
	CONST uint32 *srfw = wlc_bmac_sr_params_get(wlc_hw, &offset, &srfwsz);

	wlc_bmac_write_template_ram(wlc_hw, offset, srfwsz, (void *)srfw);
	return BCME_OK;
}

static int
BCMPREATTACHFN(wlc_bmac_sr_enable)(wlc_hw_info_t *wlc_hw)
{
	sr_engine_enable_post_dnld(wlc_hw->sih, TRUE);

	/*
	 * After enabling SR engine, update PMU min res mask
	 * This is done before si_clkctl_fast_pwrup_delay().
	 */
	si_update_masks(wlc_hw->sih);

	return BCME_OK;
}

static int
BCMPREATTACHFN(wlc_bmac_sr_init)(wlc_hw_info_t *wlc_hw)
{
	if (sr_cap(wlc_hw->sih) == FALSE) {
		WL_ERROR(("%s: sr not supported\n", __FUNCTION__));
		return BCME_ERROR;
	}

	if (wlc_bmac_sr_asm_download(wlc_hw) == BCME_OK)
		wlc_bmac_sr_enable(wlc_hw);
	else
		WL_ERROR(("%s: sr download failed\n", __FUNCTION__));

	return BCME_OK;
}
#endif /* SAVERESTORE */

/** Only called for firmware builds. Saves RAM by freeing ucode and SR arrays in an early stadium */
int
BCMPREATTACHFN(wlc_bmac_process_ucode_sr)(uint16 device, osl_t *osh, void *
				regsva, uint bustype, void *btparam)
{
	int err;
	wlc_hw_info_t *wlc_hw;

	/* allocate wlc_hw_info_t state structure */
	if ((wlc_hw = (wlc_hw_info_t*) MALLOC(osh, sizeof(wlc_hw_info_t))) == NULL) {
		WL_ERROR(("%s: no mem for wlc_hw, malloced %d bytes\n", __FUNCTION__,
			MALLOCED(osh)));
		err = 30;
		return err;
	}

	bzero((char *)wlc_hw, sizeof(wlc_hw_info_t));

	wlc_hw->sih = si_attach((uint)device, osh, regsva, bustype, btparam,
		&wlc_hw->vars, &wlc_hw->vars_size);
	if (wlc_hw->sih == NULL) {
		WL_ERROR(("%s: si_attach failed\n", __FUNCTION__));
		err = 11;
		/* return error below, after memory free */
	}
	else {
		uint32 flags = SICF_PRST;
		uint32 resetflags = 0;

		/*
		 * corerev <= 11, PR 37608 WAR assert PHY_CLK_EN bit whenever asserting sbtm reset
		 */
		if (D11REV_LE(wlc_hw->corerev, 11))
			resetflags |= SICF_PCLKE;

		/*
		 * corerev >= 18, mac no longer enables phyclk automatically when driver accesses
		 * phyreg throughput mac. This can be skipped since only mac reg is accessed below
		 */
		if (D11REV_GE(wlc_hw->corerev, 18))
			flags |= SICF_PCLKE;

#ifdef WLP2P_UCODE_ONLY
		/*
		 * DL_P2P_UC() evaluates to wlc_hw->_p2p in the ROM, so must set this field
		 * appropriately in order to decide which ucode to load.
		 */
		wlc_hw->_p2p = TRUE;	/* P2P ucode must be loaded in this case */
#endif // endif
		wlc_hw->regs = (d11regs_t *)si_setcore(wlc_hw->sih, D11_CORE_ID, 0);
		ASSERT(wlc_hw->regs != NULL);
		wlc_hw->num_mac_chains = si_numcoreunits(wlc_hw->sih, D11_CORE_ID);
		wlc_bmac_init_core_reset_disable_fn(wlc_hw);

		wlc_bmac_core_reset(wlc_hw, flags, resetflags);
		wlc_ucode_download(wlc_hw);

#if defined(SAVERESTORE)
		/* Download SR code and reclaim: ~3.5K for 4350, ~2.2K for 4335 */
		if (SR_ENAB())
			wlc_bmac_sr_init(wlc_hw);
#endif // endif
		err = 0;

		si_detach(wlc_hw->sih);
	}
	/* Always free wlc_hw ptr here prior to return */
	MFREE(osh, wlc_hw, sizeof(wlc_hw_info_t));
	return err;
}

#if defined(AP) && defined(WLC_LOW_ONLY)
/**
 * Workaround to mask off PA until the driver is ready to control PA VREF. The workaround is keyed
 * off of nvram pa_mask_low|high being set.
 */
static void
wlc_bmac_pa_war_set(wlc_hw_info_t *wlc_hw, bool enable)
{
	int polarity = 0;
	int pa_gpio_pin = GPIO_PIN_NOTDEFINED;
	static bool war_on = 0;

	if (enable && war_on) {
		return;
	}
	war_on = enable;

	/*
	 * Locate GPIO pin used for workaround by trying both polarities.
	 * If workaround not configured, pa_gpio_pin will have GPIO_PIN_NOTDEFINED.
	 */
	if ((pa_gpio_pin = getgpiopin(NULL, "pa_mask_low", GPIO_PIN_NOTDEFINED)) !=
	    GPIO_PIN_NOTDEFINED) {
		polarity = enable ? 0 : 1;
	} else if ((pa_gpio_pin = getgpiopin(NULL, "pa_mask_high", GPIO_PIN_NOTDEFINED)) !=
		GPIO_PIN_NOTDEFINED) {
		polarity = enable ? 1 : 0;
	}

	if (pa_gpio_pin != GPIO_PIN_NOTDEFINED) {
		int pa_enable = 1 << pa_gpio_pin;

		if (wlc_hw->boardflags2 & (BFL2_ANAPACTRL_2G | BFL2_ANAPACTRL_5G)) {
			/* External PA is controlled using ANALOG PA ctrl lines.
			 * Enable PARLDO_pwrup (bit 12).
			 */
			si_pmu_regcontrol(wlc_hw->sih, 0, 0x3000, 0x1000);
		} else {
			/* External PA is controlled using DIGITAL PA ctrl lines */
			si_corereg(wlc_hw->sih, SI_CC_IDX, OFFSETOF(chipcregs_t, chipcontrol),
				0x44, 0x44);
		}
		si_gpioout(wlc_hw->sih, pa_enable, polarity, GPIO_DRV_PRIORITY);
		si_gpioouten(wlc_hw->sih, pa_enable, pa_enable, GPIO_DRV_PRIORITY);
	}
}
#endif /* AP && WLC_LOW_ONLY */

void wlc_bmac_4360_pcie2_war(wlc_hw_info_t* wlc_hw, uint32 vcofreq);

/*
 * BMAC level function to allocate si handle.
 * devid - pci device id (used to determine chip#)
 * osh - opaque OS handle
 * regs - virtual address of initial core registers
 * bustype - pci/pcmcia/sb/sdio/etc
 * vars - pointer to a pointer area for "environment" variables
 * varsz - pointer to int to return the size of the vars
 */
si_t *
BCMATTACHFN(wlc_bmac_si_attach)(uint device, osl_t *osh, void *regsva, uint bustype,
	void *btparam, char **vars, uint *varsz)
{
	return si_attach(device, osh, regsva, bustype, btparam, vars, varsz);
}

/** may be called with core in reset */
void
BCMATTACHFN(wlc_bmac_si_detach)(osl_t *osh, si_t *sih)
{
	if (sih) {
		si_detach(sih);
	}
}

/** register iovar table/handlers to the system */
static int
BCMATTACHFN(wlc_bmac_register_iovt_all)(wlc_hw_info_t *hw, wlc_iocv_info_t *ii)
{
	phy_info_t *pi, *prev_pi = NULL;
	int err;
	uint i;

	if ((err = wlc_bmac_register_iovt(hw, ii)) != BCME_OK) {
		WL_ERROR(("%s: wlc_bmac_register_iovt failed\n", __FUNCTION__));
		goto fail;
	}

	for (i = 0; i < MAXBANDS; i++) {
		wlc_hwband_t *band = hw->bandstate[i];

		if (band == NULL)
			continue;

		pi = (phy_info_t *)band->pi;
		if (pi == NULL)
			continue;

		if (pi == prev_pi)
			continue;

		if ((err = phy_register_iovt_all(pi, ii)) != BCME_OK) {
			WL_ERROR(("%s: phy_register_iovt_all failed\n", __FUNCTION__));
			goto fail;
		}

		prev_pi = pi;
	}

	return BCME_OK;

fail:
	return err;
}

static int
BCMATTACHFN(wlc_bmac_register_ioct_all)(wlc_hw_info_t *hw, wlc_iocv_info_t *ii)
{
	phy_info_t *pi, *prev_pi = NULL;
	int err;
	uint i;

	for (i = 0; i < MAXBANDS; i++) {
		wlc_hwband_t *band = hw->bandstate[i];

		if (band == NULL)
			continue;

		pi = (phy_info_t *)band->pi;
		if (pi == NULL)
			continue;

		if (pi == prev_pi)
			continue;

		if ((err = phy_register_ioct_all(pi, ii)) != BCME_OK) {
			WL_ERROR(("%s: phy_register_ioct_all failed\n", __FUNCTION__));
			goto fail;
		}

		prev_pi = pi;
	}

	return BCME_OK;

fail:
	return err;
}

/**
 * low level attach
 *    run backplane attach, init nvram
 *    run phy attach
 *    initialize software state for each core and band
 *    put the whole chip in reset(driver down state), no clock
 */
int
BCMATTACHFN(wlc_bmac_attach)(wlc_info_t *wlc, uint16 vendor, uint16 device, uint unit,
	bool piomode, osl_t *osh, void *regsva, uint bustype, void *btparam)
{
	wlc_hw_info_t *wlc_hw;
	d11regs_t *regs;
	char *macaddr = NULL;
	char *vars;
	uint err = 0;
	uint j;
	bool wme = FALSE;
	shared_phy_params_t sha_params;
#ifdef WL_MU_TX
	int max_muclients;
#endif // endif

	WL_TRACE(("wl%d: %s: vendor 0x%x device 0x%x\n", unit, __FUNCTION__, vendor, device));

	STATIC_ASSERT(sizeof(wlc_d11rxhdr_t) <= WL_HWRXOFF);

	if ((wlc_hw = wlc_hw_attach(wlc, osh, unit, &err)) == NULL)
		goto fail;

	wlc->hw = wlc_hw;

#ifdef HOST_HDR_FETCH
	if (HOST_HDR_FETCH_ENAB()) {
		/* Initialize txstatus fetch request pool */
		wlc_txs_fetch_rqst_attach(wlc);
	}
#endif // endif

#ifdef WLC_HIGH
	wlc->cmn->num_d11_cores =
	si_numcoreunits(wlc->pub->sih, D11_CORE_ID);
#endif // endif
	wlc_hw->num_mac_chains = si_numcoreunits(wlc->pub->sih, D11_CORE_ID);
#ifdef WLRSDB
	/* Update the pub state saying we are an RSDB capable chip. */
#ifdef WL_DUALNIC_RSDB
		wlc->pub->cmn->_isrsdb = TRUE;
#else
	if (wlc->cmn->num_d11_cores > 1) {
		wlc->pub->cmn->_isrsdb = TRUE;
	}
#endif /* WL_DUALNIC_RSDB */
#endif /* WLRSDB */
#ifdef WME
	wme = TRUE;
#endif /* WME */

#ifdef WLC_SPLIT
	wlc->rpc = btparam;
	wlc_hw->rpc = btparam;
#endif // endif
	wlc_hw->_piomode = piomode;
#ifdef BCMSDIO
	wlc_hw->sdh = btparam;
#endif // endif

#ifdef BCM_OL_DEV
	wlc_hw->noreset = TRUE;
	wlc_hw->clk = TRUE;
#endif // endif

#if defined(SRHWVSDB) && !defined(SRHWVSDB_DISABLED)
	wlc->pub->_wlsrvsdb = TRUE; /* from this point on, macro SRHWVSDB_ENAB() may be used */
#endif /* SRHWVSDB SRHWVSDB_DISABLED */

	/* si_attach is done much more earlier in the attach path and we dont
	 * expect it to be null.
	 */
	wlc_hw->sih = wlc->pub->sih;
	wlc_hw->vars = wlc->pub->vars;
	wlc_hw->vars_size = wlc->pub->vars_size;
	ASSERT(wlc_hw->sih);
	vars = wlc_hw->vars;

	/* set bar0 window to point at D11 core */
	wlc_hw->regs = (d11regs_t *)si_setcore(wlc_hw->sih, D11_CORE_ID, wlc_hw->macunit);
	ASSERT(wlc_hw->regs != NULL);
	regs = wlc_hw->regs;

	wlc->regs = wlc_hw->regs;

	/* Save the corerev */
	wlc_hw->corerev = si_corerev(wlc_hw->sih);

	/* populate wlc_hw_info_t with default values  */
	wlc_bmac_info_init(wlc_hw);

#if defined(__mips__) || defined(BCM47XX_CA9)
	if ((CHIPID(wlc_hw->sih->chip) == BCM4360_CHIP_ID) ||
	    (CHIPID(wlc_hw->sih->chip) == BCM43460_CHIP_ID) ||
	    (CHIPID(wlc_hw->sih->chip) == BCM43526_CHIP_ID) ||
	    (CHIPID(wlc_hw->sih->chip) == BCM4352_CHIP_ID)) {
		extern int do_4360_pcie2_war;
		char *var;
		uint8 tlclkwar = 0;
		/* changing the avb vcoFreq as 510M (from default: 500M) */
		/* Tl clk 127.5Mhz */
		if ((var = getvar(NULL, rstr_wl_tlclk)))
			tlclkwar = (uint8)bcm_strtoul(var, NULL, 16);

		if (tlclkwar == 1)
			wlc_bmac_4360_pcie2_war(wlc_hw, 510);
		else if (tlclkwar == 2)
			do_4360_pcie2_war = 1;
	}
#endif /* defined(__mips__) || defined(BCM47XX_CA9) */

#if defined(BCMSDIO) && !defined(BCMDBUS)
	/* register for device being removed */
	bcmsdh_devremove_reg(btparam, wlc_device_removed, wlc);
#endif /* BCMSDIO */

	/*
	 * Get vendid/devid nvram overwrites, which could be different
	 * than those the BIOS recognizes for devices on PCMCIA_BUS,
	 * SDIO_BUS, and SROMless devices on PCI_BUS.
	 */
#ifdef BCMBUSTYPE
	bustype = BCMBUSTYPE;
#endif // endif
#ifndef BCMSDIODEV
	if (bustype != SI_BUS)
#else
	BCM_REFERENCE(bustype);
#endif // endif
	{
	char *var;

	if ((var = getvar(vars, rstr_vendid))) {
		vendor = (uint16)bcm_strtoul(var, NULL, 0);
		WL_ERROR(("Overriding vendor id = 0x%x\n", vendor));
	}
	if ((var = getvar(vars, rstr_devid))) {
		uint16 devid = (uint16)bcm_strtoul(var, NULL, 0);
		if (devid != 0xffff) {
			device = devid;
			WL_ERROR(("Overriding device id = 0x%x\n", device));
		}
	}

	if (CHIPID(wlc_hw->sih->chip) == BCM43602_CHIP_ID && device == BCM43602_CHIP_ID) {
		device = BCM43602_D11AC_ID;
	}

	/* verify again the device is supported */
	if (!wlc_chipmatch(vendor, device)) {
		WL_ERROR(("wl%d: %s: Unsupported vendor/device (0x%x/0x%x)\n",
		          unit, __FUNCTION__, vendor, device));
		err = 12;
		goto fail;
	}
	}

	wlc_hw->vendorid = vendor;
	wlc_hw->deviceid = device;

	/* XXX 4360:11AC:WES unprogrammed BCM4360 has a dev id 0x4360 which is actually
	 * BCM4330_D11N_ID, a single band dev id
	 * Remove this when we get QT to have OTP to set proper DevID
	 */
	if ((ISSIM_ENAB(wlc_hw->sih)) &&
	    ((CHIPID(wlc_hw->sih->chip) == BCM4360_CHIP_ID) ||
	     (CHIPID(wlc_hw->sih->chip) == BCM43460_CHIP_ID) ||
	     (CHIPID(wlc_hw->sih->chip) == BCM43526_CHIP_ID) ||
	     (CHIPID(wlc_hw->sih->chip) == BCM4352_CHIP_ID))) {
		wlc_hw->deviceid = (CHIPID(wlc_hw->sih->chip) == BCM4352_CHIP_ID) ?
			BCM4352_D11AC_ID : BCM4360_D11AC_ID;
	}

	wlc_hw->band = wlc_hw->bandstate[IS_SINGLEBAND_5G(wlc_hw->deviceid) ?
		BAND_5G_INDEX : BAND_2G_INDEX];
#ifdef WLC_HIGH
	/* Monolithic driver gets wlc->band and band members initialized in wlc_bmac_attach() */
	wlc->band = wlc->bandstate[IS_SINGLEBAND_5G(wlc_hw->deviceid) ?
		BAND_5G_INDEX : BAND_2G_INDEX];
#endif // endif

	wlc->regs = wlc_hw->regs;

	/* validate chip, chiprev and corerev */
	if (!wlc_isgoodchip(wlc_hw)) {
		err = 13;
		goto fail;
	}
	wlc_bmac_init_core_reset_disable_fn(wlc_hw);
	/* initialize power control registers */
	si_clkctl_init(wlc_hw->sih);

#ifdef WLC_HIGH
	si_pcie_ltr_war(wlc_hw->sih);

	/* 'ltr' advertizes to the PCIe host how long the device takes to power up or down */
	if ((BCM4350_CHIP(wlc_hw->sih->chip) &&
	     CST4350_IFC_MODE(wlc_hw->sih->chipst) == CST4350_IFC_MODE_PCIE) ||
	     CHIPID(wlc_hw->sih->chip) == BCM43602_CHIP_ID) { /* 43602 is PCIe only */
		si_pcieltrenable(wlc_hw->sih, 1, 1);
	}

	if ((BUSTYPE(wlc_hw->sih->bustype) == PCI_BUS) &&
	    (wlc_hw->sih->buscoretype == PCIE2_CORE_ID) &&
	    (wlc_hw->sih->buscorerev <= 4))
		si_pcieobffenable(wlc_hw->sih, 1, 0);
#endif /* WLC_HIGH */

	/* request fastclock and force fastclock for the rest of attach
	 * bring the d11 core out of reset.
	 *   For PMU chips, the first wlc_clkctl_clk is no-op since core-clk is still FALSE;
	 *   But it will be called again inside wlc_corereset, after d11 is out of reset.
	 */
	wlc_clkctl_clk(wlc_hw, CLK_FAST);
	wlc_bmac_corereset(wlc_hw, WLC_USE_COREFLAGS);

	if (!wlc_bmac_validate_chip_access(wlc_hw)) {
		WL_ERROR(("wl%d: %s: validate_chip_access failed\n", unit, __FUNCTION__));
		err = 14;
		goto fail;
	}

	/* get the board rev, used just below */
	j = getintvar(vars, rstr_boardrev);
	/* promote srom boardrev of 0xFF to 1 */
	if (j == BOARDREV_PROMOTABLE)
		j = BOARDREV_PROMOTED;
	wlc_hw->boardrev = (uint16)j;
	if (!wlc_validboardtype(wlc_hw)) {
		WL_ERROR(("wl%d: %s: Unsupported Broadcom board type (0x%x)"
			" or revision level (0x%x)\n",
			unit, __FUNCTION__, wlc_hw->sih->boardtype, wlc_hw->boardrev));
		err = 15;
		goto fail;
	}
	wlc_hw->sromrev = (uint8)getintvar(vars, rstr_sromrev);
	wlc_hw->boardflags = (uint32)getintvar(vars, rstr_boardflags);
	wlc_hw->boardflags2 = (uint32)getintvar(vars, rstr_boardflags2);
	if (wlc_hw->sromrev >= 12)
		wlc_hw->boardflags4 = (uint32)getintvar(vars, rstr_boardflags4);
	wlc_hw->antswctl2g = (uint8)getintvar(vars, rstr_antswctl2g);
	wlc_hw->antswctl5g = (uint8)getintvar(vars, rstr_antswctl5g);

	/* some branded-boards boardflags srom programmed incorrectly */
	if (wlc_hw->sih->boardvendor == VENDOR_APPLE) {
		if ((wlc_hw->sih->boardtype == 0x4e) && (wlc_hw->boardrev >= 0x41))
			wlc_hw->boardflags |= BFL_PACTRL;
		else if ((wlc_hw->sih->boardtype == BCM94331X28) &&
		         (wlc_hw->boardrev < 0x1501)) {
			wlc_hw->boardflags |= BFL_FEM_BT;
			wlc_hw->boardflags2 = 0;
		} else if ((wlc_hw->sih->boardtype == BCM94331X29B) &&
		           (wlc_hw->boardrev < 0x1202)) {
			wlc_hw->boardflags |= BFL_FEM_BT;
			wlc_hw->boardflags2 = 0;
		}
	}

	if (D11REV_LE(wlc_hw->corerev, 4) || (wlc_hw->boardflags & BFL_NOPLLDOWN))
		wlc_bmac_pllreq(wlc_hw, TRUE, WLC_PLLREQ_SHARED);

	if ((BUSTYPE(wlc_hw->sih->bustype) == PCI_BUS) && (si_pci_war16165(wlc_hw->sih)))
		wlc->war16165 = TRUE;

#if defined(DBAND)
	/* check device id(srom, nvram etc.) to set bands */
	if ((wlc_hw->deviceid == BCM4306_D11DUAL_ID) ||
		(wlc_hw->deviceid == BCM4318_D11DUAL_ID) ||
		(wlc_hw->deviceid == BCM4311_D11DUAL_ID) ||
		(wlc_hw->deviceid == BCM4321_D11N_ID) ||
		(wlc_hw->deviceid == BCM4322_D11N_ID) ||
		(wlc_hw->deviceid == BCM4328_D11DUAL_ID) ||
		(wlc_hw->deviceid == BCM4325_D11DUAL_ID) ||
		(wlc_hw->deviceid == BCM43222_D11N_ID) ||
		(wlc_hw->deviceid == BCM43224_D11N_ID) ||
		(wlc_hw->deviceid == BCM43224_D11N_ID_VEN1) ||
		(wlc_hw->deviceid == BCM43421_D11N_ID) ||
		(wlc_hw->deviceid == BCM43226_D11N_ID) ||
		(wlc_hw->deviceid == BCM43236_D11N_ID) ||
		(wlc_hw->deviceid == BCM6362_D11N_ID) ||
		(wlc_hw->deviceid == BCM4331_D11N_ID) ||
		(wlc_hw->deviceid == BCM4315_D11DUAL_ID) ||
		(wlc_hw->deviceid == BCM43228_D11N_ID) ||
		(wlc_hw->deviceid == BCM4324_D11N_ID) ||
		(wlc_hw->deviceid == BCM43242_D11N_ID) ||
		(wlc_hw->deviceid == BCM4334_D11N_ID) ||
		(wlc_hw->deviceid == BCM4360_D11AC_ID) ||
		(wlc_hw->deviceid == BCM4352_D11AC_ID) ||
		(wlc_hw->deviceid == BCM43341_D11N_ID) ||
		(wlc_hw->deviceid == BCM4335_D11AC_ID) ||
		(wlc_hw->deviceid == BCM4345_D11AC_ID) ||
		(wlc_hw->deviceid == BCM43602_D11AC_ID) ||
		(wlc_hw->deviceid == BCM4352_D11AC_ID) ||
		(wlc_hw->deviceid == BCM4350_D11AC_ID) ||
		(wlc_hw->deviceid == BCM43556_D11AC_ID) ||
		(wlc_hw->deviceid == BCM43558_D11AC_ID) ||
		(wlc_hw->deviceid == BCM43566_D11AC_ID) ||
		(wlc_hw->deviceid == BCM43568_D11AC_ID) ||
		(wlc_hw->deviceid == BCM43569_D11AC_ID) ||
		(wlc_hw->deviceid == BCM4354_D11AC_ID) ||
		(wlc_hw->deviceid == BCM4356_D11AC_ID) ||
		(wlc_hw->deviceid == BCM4358_D11AC_ID) ||
#ifdef UNRELEASEDCHIP
		(wlc_hw->deviceid == BCM4349_D11AC_ID) ||
		(wlc_hw->deviceid == BCM4355_D11AC_ID) ||
		(wlc_hw->deviceid == BCM4359_D11AC_ID) ||
		(wlc_hw->deviceid == BCM43909_D11AC_ID) ||
#endif /* UNRELEASEDCHIP */
		(wlc_hw->deviceid == BCM4365_D11AC_ID) ||
		(wlc_hw->deviceid == BCM4366_D11AC_ID) ||
		0) {
		/* Dualband boards */
		wlc_hw->_nbands = 2;
	} else
#endif /* DBAND */
		wlc_hw->_nbands = 1;

#if NCONF
	if (CHIPID(wlc_hw->sih->chip) == BCM43221_CHIP_ID ||
	    CHIPID(wlc_hw->sih->chip) == BCM43231_CHIP_ID ||
	    CHIPID(wlc_hw->sih->chip) == BCM43225_CHIP_ID ||
	    CHIPID(wlc_hw->sih->chip) == BCM43235_CHIP_ID ||
	    CHIPID(wlc_hw->sih->chip) == BCM43131_CHIP_ID ||
	    CHIPID(wlc_hw->sih->chip) == BCM43217_CHIP_ID ||
	    CHIPID(wlc_hw->sih->chip) == BCM43227_CHIP_ID) {
		wlc_hw->_nbands = 1;
	}
#endif /* NCONF */

#ifdef WLC_HIGH
	/* BMAC_NOTE: remove init of pub values when wlc_attach() unconditionally does the
	 * init of these values
	 */
	wlc->vendorid = wlc_hw->vendorid;
	wlc->deviceid = wlc_hw->deviceid;
	wlc->pub->sih = wlc_hw->sih;
	wlc->pub->corerev = wlc_hw->corerev;
	wlc->pub->sromrev = wlc_hw->sromrev;
	wlc->pub->boardrev = wlc_hw->boardrev;
	wlc->pub->boardflags = wlc_hw->boardflags;
	wlc->pub->boardflags2 = wlc_hw->boardflags2;
	if (wlc_hw->sromrev >= 12)
		wlc->pub->boardflags4 = wlc_hw->boardflags4;
	wlc->pub->_nbands = wlc_hw->_nbands;
#endif // endif

	WL_ERROR(("wlc_bmac_attach, deviceid 0x%x nbands %d\n", wlc_hw->deviceid, wlc_hw->_nbands));

#ifdef PKTC
	wlc->pub->_pktc = (getintvar(vars, "pktc_disable") == 0) &&
		(getintvar(vars, "ctf_disable") == 0);
#endif // endif
#if defined(PKTC_DONGLE)
	wlc->pub->_pktc = TRUE;
#endif // endif

	/* init mutxmax */
	wlc->pub->_mutx_max = MUTX_MAX_0;
#ifdef WL_MU_TX
	wlc->pub->_mutx_max = (getvar(vars, "mutxmax") == NULL) ?
		MUTX_MAX_AUTO : getintvar(vars, "mutxmax");
	switch (wlc->pub->_mutx_max) {
	case MUTX_MAX_0:
	case MUTX_MAX_2:
	case MUTX_MAX_4:
	case MUTX_MAX_8:
		WL_ERROR(("%s: mutxmax is configured to %d\n", __FUNCTION__, wlc->pub->_mutx_max));
		break;
	default:
		wlc->pub->_mutx_max = MUTX_MAX_AUTO;
		WL_ERROR(("%s: mutxmax is configured to auto\n", __FUNCTION__));
		break;
	}

	max_muclients = MUCLIENT_NUM;
	/* For 4366B1, we can only support MU-0 and MU-2 */
	if (D11REV_IS(wlc->pub->corerev, 64)) {
		max_muclients = MUCLIENT_NUM_MIN;
	}

	if (wlc->pub->_mutx_max == MUTX_MAX_AUTO) {
		wlc->pub->_mutx_max = max_muclients;
	} else {
		wlc->pub->_mutx_max = MIN(max_muclients, wlc->pub->_mutx_max);
	}
	WL_ERROR(("%s: mutxmax is %d after adjustment\n", __FUNCTION__,
		wlc->pub->_mutx_max));
#endif /* WL_MU_TX */

	wlc_hw->physhim = wlc_phy_shim_attach(wlc_hw, wlc->wl, wlc);

	if (wlc_hw->physhim == NULL) {
		WL_ERROR(("wl%d: %s: wlc_phy_shim_attach failed\n", unit, __FUNCTION__));
		err = 25;
		goto fail;
	}

	/* pass all the parameters to wlc_phy_shared_attach in one struct */
	sha_params.osh = osh;
	sha_params.sih = wlc_hw->sih;
	sha_params.physhim = wlc_hw->physhim;
	sha_params.unit = unit;
	sha_params.corerev = wlc_hw->corerev;
	sha_params.vars = vars;
	sha_params.vid = wlc_hw->vendorid;
	sha_params.did = wlc_hw->deviceid;
	sha_params.chip = wlc_hw->sih->chip;
	sha_params.chiprev = wlc_hw->sih->chiprev;
	sha_params.chippkg = wlc_hw->sih->chippkg;
	sha_params.sromrev = wlc_hw->sromrev;
	sha_params.boardtype = wlc_hw->sih->boardtype;
	sha_params.boardrev = wlc_hw->boardrev;
	sha_params.boardvendor = wlc_hw->sih->boardvendor;
	sha_params.boardflags = wlc_hw->boardflags;
	sha_params.boardflags2 = wlc_hw->boardflags2;
	sha_params.boardflags4 = wlc_hw->boardflags4;
	sha_params.bustype = wlc_hw->sih->bustype;
	sha_params.buscorerev = wlc_hw->sih->buscorerev;

	/* alloc and save pointer to shared phy state area */
	wlc_hw->phy_sh = wlc_phy_shared_attach(&sha_params);
	if (!wlc_hw->phy_sh) {
		err = 16;
		goto fail;
	}

	/* use different hw rx offset for AC cores, must be done before dma_attach */
	wlc->hwrxoff = (D11REV_GE(wlc_hw->corerev, 40)) ? WL_HWRXOFF_AC : WL_HWRXOFF;
	wlc->hwrxoff_pktget = (wlc->hwrxoff % 4) ?  wlc->hwrxoff : (wlc->hwrxoff + 2);

#if defined(BCM_DMA_CT) && !defined(BCM_DMA_CT_DISABLED)
	/* Check var if we need to enable CT */
	if (!wlc->_dma_ct) {
		wlc->_dma_ct = (getintvar(vars, "ctdma") == 1);

		if ((BUSTYPE(wlc_hw->sih->bustype) == SI_BUS) && D11REV_IS(wlc_hw->corerev, 65) &&
				(getvar(NULL, "ctdma") == NULL)) {
			wlc->_dma_ct = TRUE;
		}
	}

	/* init of cut through mode.  must be done before dma_attach */
	if (wlc->_dma_ct) {
		wlc->_dma_ct = WLC_CT_HW_SUPPORTED(wlc);
	}

	/* Disabling CTDMA if mutxmax is 0 */
	if (wlc->_dma_ct && wlc->pub->_mutx_max == MUTX_MAX_0)
		wlc->_dma_ct = FALSE;

	/* preconfigured ctdma with NVRAM or param */
	wlc->_dma_ct_flags = wlc->_dma_ct? DMA_CT_PRECONFIGURED : 0;
#ifdef DMATXRC
	if (BCM_DMA_CT_ENAB(wlc) && DMATXRC_ENAB(wlc->pub)) {
		wlc->pub->_dmatxrc = FALSE;
	}
#endif // endif
#endif /* BCM_DMA_CT && !BCM_DMA_CT_DISABLED */

	wlc_hw->vcoFreq_4360_pcie2_war = 510; /* Default Value */

	if (D11REV_GE(wlc_hw->corerev, 14)) {
		wlc_hw->machwcap1 = R_REG(wlc_hw->osh, &wlc_hw->regs->machwcap1);
		wlc_hw->num_mac_chains =
			1 + ((wlc_hw->machwcap1 & MCAP1_NUMMACCHAINS) >> MCAP1_NUMMACCHAINS_SHIFT);
	} else {
		wlc_hw->machwcap1 = 0;
		wlc_hw->num_mac_chains = si_numcoreunits(wlc_hw->sih, D11_CORE_ID);
	}

	/* initialize software state for each core and band */
	for (j = 0; j < NBANDS_HW(wlc_hw); j++) {
		/*
		 * band0 is always 2.4Ghz
		 * band1, if present, is 5Ghz
		 */

		/* So if this is a single band 11a card, use band 1 */
		if (IS_SINGLEBAND_5G(wlc_hw->deviceid))
			j = BAND_5G_INDEX;

		wlc_setxband(wlc_hw, j);

		wlc_hw->band->bandunit = j;
		wlc_hw->band->bandtype = j ? WLC_BAND_5G : WLC_BAND_2G;
#ifdef WLC_HIGH
		/* Monolithic driver gets wlc->band and band members
		 * initialized in wlc_bmac_attach()
		 */
		wlc->band->bandunit = j;
		wlc->band->bandtype = j ? WLC_BAND_5G : WLC_BAND_2G;
#endif // endif
		wlc->core->coreidx = si_coreidx(wlc_hw->sih);

		if (D11REV_GE(wlc_hw->corerev, 13)) {
			wlc_hw->machwcap = R_REG(osh, &regs->machwcap);
			/* PR78990 WAR for broken HW TKIP in 4331 A0 and A1
			 * XXX: This PR also impacts: 4334A0 rev33, 43143. Was fixed in rev 40.
			 * XXX 4360A0: HW TKIP also broken, 4360B0 not sure
			 */
			if ((D11REV_IS(wlc_hw->corerev, 26) &&
				CHIPREV(wlc_hw->sih->chiprev) == 0) ||
			    (D11REV_IS(wlc_hw->corerev, 29)) || (D11REV_IS(wlc_hw->corerev, 33)) ||
			    (D11REV_IS(wlc_hw->corerev, 34)) || (D11REV_IS(wlc_hw->corerev, 35)) ||
			    (D11REV_IS(wlc_hw->corerev, 37)) || (D11REV_IS(wlc_hw->corerev, 30)) ||
			    (D11REV_IS(wlc_hw->corerev, 40)) || (D11REV_IS(wlc_hw->corerev, 41)) ||
#if !defined(WLOFFLD) && !defined(BCM_OL_DEV)
			    (D11REV_IS(wlc_hw->corerev, 42)) ||
#endif // endif
			    (D11REV_IS(wlc_hw->corerev, 43)) || (D11REV_IS(wlc_hw->corerev, 44)) ||
			    (D11REV_IS(wlc_hw->corerev, 48)) ||
			    (D11REV_IS(wlc_hw->corerev, 49))) {
			     WL_ERROR(("%s: Disabling HW TKIP!\n", __FUNCTION__));
				 wlc_hw->machwcap &= ~MCAP_TKIPMIC;
			}
		}

		/* init tx fifo size */

		if (D11REV_GE(wlc_hw->corerev, 48) && D11REV_LT(wlc_hw->corerev, 64)) {
			/* this is just a way to reduce memory footprint */
			wlc_hw->xmtfifo_sz = xmtfifo_sz_dummy;
		} else if (D11REV_LT(wlc_hw->corerev, 40)) {
		/* init tx fifo size */
			ASSERT((wlc_hw->corerev - XMTFIFOTBL_STARTREV) < ARRAYSIZE(xmtfifo_sz));
			wlc_hw->xmtfifo_sz = xmtfifo_sz[(wlc_hw->corerev - XMTFIFOTBL_STARTREV)];
		} else {
			wlc_hw->xmtfifo_sz = xmtfifo_sz[40 - XMTFIFOTBL_STARTREV];
		}

		wlc_bmac_ampdu_set(wlc_hw, AMPDU_AGGMODE_HOST);

		/* Get a phy for this band */
		WL_NONE(("wl%d: %s: bandunit %d bandtype %d coreidx %d\n", unit,
		         __FUNCTION__, wlc_hw->band->bandunit, wlc_hw->band->bandtype,
		         wlc->core->coreidx));
		if ((wlc_hw->band->pi = (wlc_phy_t *)
		     phy_module_attach(wlc_hw->phy_sh, (void *)(uintptr)regs,
		                wlc_hw->band->bandtype, vars)) == NULL) {
			WL_ERROR(("wl%d: %s: phy_module_attach failed\n", unit, __FUNCTION__));
			err = 17;
			goto fail;
		}

#ifndef BCM_OL_DEV
		/* No need to call this for ACPHY chips */
		if (!WLCISACPHY(wlc_hw->band))
			wlc_bmac_set_btswitch(wlc_hw, AUTO);
#endif // endif

		wlc_phy_machwcap_set(wlc_hw->band->pi, wlc_hw->machwcap);

		phy_utils_get_phyversion((phy_info_t *)wlc_hw->band->pi, &wlc_hw->band->phytype,
			&wlc_hw->band->phyrev, &wlc_hw->band->radioid, &wlc_hw->band->radiorev,
			&wlc_hw->band->phy_minor_rev);

		wlc_hw->band->core_flags = phy_utils_get_coreflags((phy_info_t *)wlc_hw->band->pi);

		/* verify good phy_type & supported phy revision */
		if (WLCISAPHY(wlc_hw->band)) {
			if (ACONF_HAS(wlc_hw->band->phyrev))
				goto good_phy;
			else
				goto bad_phy;
		} else if (WLCISGPHY(wlc_hw->band)) {
			if (GCONF_HAS(wlc_hw->band->phyrev))
				goto good_phy;
			else
				goto bad_phy;
		} else if (WLCISNPHY(wlc_hw->band)) {
			if (NCONF_HAS(wlc_hw->band->phyrev))
				goto good_phy;
			else
				goto bad_phy;
		} else if (WLCISLPPHY(wlc_hw->band)) {
			if (LPCONF_HAS(wlc_hw->band->phyrev))
				goto good_phy;
			else
				goto bad_phy;
		} else if (WLCISSSLPNPHY(wlc_hw->band)) {
			if (SSLPNCONF_HAS(wlc_hw->band->phyrev))
				goto good_phy;
			else
				goto bad_phy;
		} else if (WLCISLCNPHY(wlc_hw->band)) {
			if (LCNCONF_HAS(wlc_hw->band->phyrev))
				goto good_phy;
			else
				goto bad_phy;
		} else if (WLCISHTPHY(wlc_hw->band)) {
			if (HTCONF_HAS(wlc_hw->band->phyrev))
				goto good_phy;
			else
				goto bad_phy;
		} else if (WLCISLCN40PHY(wlc_hw->band)) {
			if (LCN40CONF_HAS(wlc_hw->band->phyrev))
				goto good_phy;
			else
				goto bad_phy;
		} else if (WLCISACPHY(wlc_hw->band)) {
			if (ACCONF_HAS(wlc_hw->band->phyrev))
				goto good_phy;
			else
				goto bad_phy;
		} else {
bad_phy:
			WL_ERROR(("wl%d: %s: unsupported phy type/rev (%d/%d)\n",
				unit, __FUNCTION__, wlc_hw->band->phytype, wlc_hw->band->phyrev));
			err = 18;
			goto fail;
		}

good_phy:
		WL_ERROR(("wl%d: %s: chiprev %d corerev %d "
			"cccap 0x%x maccap 0x%x band %sG, phy_type %d phy_rev %d\n",
			unit, __FUNCTION__, CHIPREV(wlc_hw->sih->chiprev),
			wlc_hw->corerev, wlc_hw->sih->cccaps, wlc_hw->machwcap,
			BAND_2G(wlc_hw->band->bandtype) ? "2.4" : "5",
			wlc_hw->band->phytype, wlc_hw->band->phyrev));

#ifdef WLC_HIGH
		/* Monolithic driver gets wlc->band and band members
		 * initialized in wlc_bmac_attach()
		 */
		/* Initialize both wlc->pi and wlc->bandinst->pi */
		wlc->pi = wlc->bandinst[wlc->band->bandunit]->pi = wlc_hw->band->pi;
		wlc->band->phytype = wlc_hw->band->phytype;
		wlc->band->phyrev = wlc_hw->band->phyrev;
		wlc->band->radioid = wlc_hw->band->radioid;
		wlc->band->radiorev = wlc_hw->band->radiorev;
		wlc->band->phy_minor_rev = wlc_hw->band->phy_minor_rev;
#endif /* WLC_HIGH */

		/* default contention windows size limits */
		wlc_hw->band->CWmin = APHY_CWMIN;
		wlc_hw->band->CWmax = PHY_CWMAX;

		if (!wlc_bmac_attach_dmapio(wlc_hw, wme)) {
		err = 19;
		goto fail;
		}

		/* initial ucode host flags */
		wlc_mhfdef(wlc_hw, wlc_hw->band->mhfs);
	}

	if (!PIO_ENAB_HW(wlc_hw) &&
	    (BCM4331_CHIP_ID == CHIPID(wlc_hw->sih->chip)) &&
	    (si_pcie_get_request_size(wlc_hw->sih) > 128)) {
		uint i;
		for (i = 0; i < NFIFO; i++) {
			if (wlc_hw->di[i])
				dma_ctrlflags(wlc_hw->di[i], DMA_CTRL_DMA_AVOIDANCE_WAR,
					DMA_CTRL_DMA_AVOIDANCE_WAR);
		}
		wlc->dma_avoidance_war = TRUE;
	}

	/* set default 2-wire or 3-wire setting */
	wlc_bmac_btc_wire_set(wlc_hw, WL_BTC_DEFWIRE);

	wlc_hw->btc->btcx_aa = (uint8)getintvar(vars, rstr_aa2g);
	wlc_hw->btc->mode = (uint8)getintvar(vars, rstr_btc_mode);
	/* set BT Coexistence default mode */
	if (WLCISSSLPNPHY(wlc_hw->band))
		wlc_bmac_btc_mode_set(wlc_hw, WL_BTC_ENABLE);
	else if (getvar(vars, rstr_btc_mode))
		wlc_bmac_btc_mode_set(wlc_hw, (uint8)getintvar(vars, rstr_btc_mode));
	else
		wlc_bmac_btc_mode_set(wlc_hw, WL_BTC_DEFAULT);

	/* attach/register iovar/ioctl handlers */
	if ((wlc_hw->iocvi = wlc_iocv_attach(wlc_hw)) == NULL) {
		WL_ERROR(("wl%d: %s: wlc_iocv_attach failed\n", unit, __FUNCTION__));
		err = 11;
		goto fail;
	}
	if (wlc_bmac_register_iovt_all(wlc_hw, wlc_hw->iocvi) != BCME_OK) {
		err = 181;
		goto fail;
	}
	if (wlc_bmac_register_ioct_all(wlc_hw, wlc_hw->iocvi) != BCME_OK) {
		err = 182;
		goto fail;
	}

#ifdef PREATTACH_NORECLAIM
#if defined(DONGLEBUILD) && !defined(BCM_OL_DEV)
	/* 2 stage reclaim
	 *  download the ucode during attach, reclaim the ucode after attach
	 *    along with other rattach stuff, unconditionally on dongle
	 *  the second stage reclaim happens after up conditioning on reclaim flag
	 * for dual d11 core chips, ucode is downloaded only once
	 * and will be thru core-0
	 */
	if (wlc_hw->macunit == 0)
		wlc_ucode_download(wlc_hw);

#endif /* DONGLEBUILD */
#endif /* PREATTACH_NORECLAIM */

#ifdef DONGLEBUILD
	if (CHIPID(wlc_hw->sih->chip) == BCM43602_CHIP_ID) {
		/* Double CR4 clock frequency */
		hnd_cpu_clockratio(wlc_hw->sih, 2);
	}
#endif /* DONGLEBUILD */

#ifdef ATE_BUILD

	wlc_hw->dump = wlc_dump_reg_create(wlc_hw->osh, 10);
	if (!wlc_hw->dump) {
		WL_ERROR(("wl%d: %s: wlc_dump_reg_create() failed.\n", unit, __FUNCTION__));
		err = BCME_NOMEM;
		return err;
	}

	wlc_bmac_register_dumps(wlc_hw);

	return err;
#endif // endif

#ifdef SAVERESTORE
	if (SR_ENAB() && sr_cap(wlc_hw->sih)) {
#ifndef DONGLEBUILD
		/* Download SR code */
		wlc_bmac_sr_init(wlc_hw);
#endif /* !DONGLEBUILD */
#ifdef BCM_OL_DEV
		/* Offloads just enable SR don't need to download */
		wlc_bmac_sr_enable(wlc_hw);
#endif /* BCM_OL_DEV */
	}
#endif /* SAVERESTORE */

	/* disable core to match driver "down" state */
	wlc_coredisable(wlc_hw);

	if ((CHIPID(wlc_hw->sih->chip) == BCM4360_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM43602_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM4352_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM43526_CHIP_ID))
		si_pmu_rfldo(wlc_hw->sih, 0);

	/* Match driver "down" state */
	if (BUSTYPE(wlc_hw->sih->bustype) == PCI_BUS)
		si_pci_down(wlc_hw->sih);

	/* register sb interrupt callback functions */
#ifdef BCMDBG
	si_register_intr_callback(wlc_hw->sih, (void *)wlc_wlintrsoff,
		(void *)wlc_wlintrsrestore, (void *)wlc_intrs_enabled, wlc_hw);
#else
	si_register_intr_callback(wlc_hw->sih, (void *)wlc_wlintrsoff,
		(void *)wlc_wlintrsrestore, NULL, wlc_hw);
#endif /* BCMDBG */

	ASSERT(!wlc_hw->sbclk || !si_taclear(wlc_hw->sih, TRUE));

#ifndef BCM_OL_DEV
	/* turn off pll and xtal to match driver "down" state */
	wlc_bmac_xtal(wlc_hw, OFF);
#endif // endif

	/* *********************************************************************
	 * The hardware is in the DOWN state at this point. D11 core
	 * or cores are in reset with clocks off, and the board PLLs
	 * are off if possible.
	 *
	 * Beyond this point, wlc->sbclk == FALSE and chip registers
	 * should not be touched.
	 *********************************************************************
	 */

	/* init etheraddr state variables */
	if ((macaddr = wlc_get_macaddr(wlc_hw)) == NULL) {
		WL_ERROR(("wl%d: %s: macaddr not found\n", unit, __FUNCTION__));
		err = 21;
		goto fail;
	}
#ifdef WL_DUALNIC_RSDB
	if (wlc->pub->unit == 1) {
		bcopy(&wlc->cmn->wlc[0]->hw->etheraddr, &wlc_hw->etheraddr, ETHER_ADDR_LEN);
	} else
#endif // endif
	{
		bcm_ether_atoe(macaddr, &wlc_hw->etheraddr);
	}

	if (ETHER_ISBCAST((char*)&wlc_hw->etheraddr) ||
		ETHER_ISNULLADDR((char*)&wlc_hw->etheraddr)) {
#ifdef DSLCPE
		unsigned long ulId = (unsigned long)('w'<<24) + (unsigned long)('l'<<16) + unit;
		unsigned char macAddr_b[ETH_ALEN];
		int i;
		macAddr_b[0] = 0xff;
		kerSysGetMacAddress(macAddr_b, ulId);
		WL_INFORM(("wl%d: wlc_attach: use mac addr from the system pool by id: 0x%4x\n",
		           unit, (int)ulId));

		if (ETHER_ISBCAST(macAddr_b) || ETHER_ISNULLADDR(macAddr_b)) {
			WL_ERROR(("wlc_attach: wl%d: MAC address has not been "
			          "initialized in NVRAM.\n", unit));
			goto fail;
		}
		memcpy(&wlc_hw->etheraddr, macAddr_b, ETHER_ADDR_LEN);

		WL_INFORM(("wl%d: MAC Address: ", unit));
		for (i = 0; i < ETH_ALEN; i++) {
			WL_INFORM(("%2.2X:", ((unsigned char *)(&wlc_hw->etheraddr))[i]));
		}
		WL_INFORM(("\n"));

		if (getvar(wlc_hw->vars, rstr_macaddr) != NULL) {
			bcm_ether_ntoa((struct ether_addr *)&macAddr_b,
			getvar(wlc_hw->vars, rstr_macaddr));
		}

#else
		WL_ERROR(("wl%d: %s: bad macaddr %s\n", unit, __FUNCTION__, macaddr));
		err = 22;
		goto fail;
#endif   /* DSLCPE */
	}

#ifdef WLC_LOW_ONLY
	bcopy(&wlc_hw->etheraddr, &wlc_hw->orig_etheraddr, ETHER_ADDR_LEN);
#endif // endif

	WL_INFORM(("wl%d: %s: board 0x%x macaddr: %s\n", unit, __FUNCTION__,
		wlc_hw->sih->boardtype, macaddr));

#ifdef WLLED
	if ((wlc_hw->ledh = wlc_bmac_led_attach(wlc_hw)) == NULL) {
		WL_ERROR(("wl%d: %s: wlc_bmac_led_attach() failed.\n", unit, __FUNCTION__));
		err = 23;
		goto fail;
	}
#endif // endif

#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(BCMDBG_PHYDUMP)
	wlc_hw->suspend_stats = (bmac_suspend_stats_t*) MALLOC(wlc_hw->osh,
	                                                       sizeof(*wlc_hw->suspend_stats));
	if (wlc_hw->suspend_stats == NULL) {
		WL_ERROR(("wl%d: wlc_bmac_attach: suspend_stats alloc failed.\n", unit));
		err = 26;
		goto fail;
	}
#endif /* BCMDBG || BCMDBG_DUMP || BCMDBG_PHYDUMP */

#ifdef AP
	if (wlc_bmac_pmq_init(wlc_hw) != BCME_OK) {
		err = 27;
		goto fail;
	}
#endif // endif

#if defined(AP) && defined(WLC_LOW_ONLY)
	wlc_bmac_pa_war_set(wlc_hw, FALSE);
#endif // endif

	/* Initialize template config variables */
	wlc_template_cfg_init(wlc, wlc_hw->corerev);

#ifdef WLEXTLOG
#ifdef WLC_LOW_ONLY
	if ((wlc_hw->extlog = wlc_extlog_attach(osh, NULL)) == NULL) {
		WL_ERROR(("wl%d: %s: wlc_extlog_attach() failed.\n", unit, __FUNCTION__));
		err = 24;
		goto fail;
	}
#endif // endif
#endif /* WLEXTLOG */

	/* Register to be notified when pktpool is available which can
	 * happen outside this scope from bus side.
	 */
	if (BCMSPLITRX_ENAB()) {
		if (POOL_ENAB(wlc->pub->pktpool_rxlfrag)) {
			/* if rx frag pool is enabled, use fragmented rx pool for registration */
#ifdef BCMDBG_POOL
			pktpool_dbg_register(wlc->pub->pktpool_rxlfrag, wlc_pktpool_dbg_cb, wlc_hw);
#endif // endif

			/* Pass down pool info to dma layer */
			if (wlc_hw->di[RX_FIFO])
				dma_pktpool_set(wlc_hw->di[RX_FIFO], wlc->pub->pktpool_rxlfrag);

			/* if second fifo is set, set pktpool for that */
			if (wlc_hw->di[RX_FIFO1])
				dma_pktpool_set(wlc_hw->di[RX_FIFO1],
				(RXFIFO_SPLIT()?wlc->pub->pktpool_rxlfrag :wlc->pub->pktpool));

			/* FIFO- 2 */
			if (wlc_hw->di[RX_FIFO2])
				dma_pktpool_set(wlc_hw->di[RX_FIFO2], wlc->pub->pktpool);
			/* register a callback to be invoked when hostaddress is posted */
			if (wlc_hw->di[RX_FIFO])
				pkpool_haddr_avail_register_cb(wlc->pub->pktpool_rxlfrag,
					wlc_pktpool_avail_cb, wlc_hw);

		} else {
			WL_ERROR(("%s: RXLFRAG Pool not available split RX mode \n", __FUNCTION__));
		}
	} else if (POOL_ENAB(wlc->pub->pktpool)) {
		pktpool_avail_register(wlc->pub->pktpool,
			wlc_pktpool_avail_cb, wlc_hw);
		pktpool_empty_register(wlc->pub->pktpool,
			wlc_pktpool_empty_cb, wlc_hw);
#ifdef BCMDBG_POOL
		pktpool_dbg_register(wlc->pub->pktpool, wlc_pktpool_dbg_cb, wlc_hw);
#endif // endif

#ifdef DMATXRC
		if (DMATXRC_ENAB(wlc->pub))
			wlc_phdr_attach(wlc);
#endif // endif

		/* set pool for rx dma */
		if (wlc_hw->di[RX_FIFO])
			dma_pktpool_set(wlc_hw->di[RX_FIFO], wlc->pub->pktpool);
	}

#ifndef BCM_OL_DEV
	/* Initialize btc param information from NVRAM */
	if (wlc_bmac_btc_param_attach(wlc) != BCME_OK) {
		err = 27;
		goto fail;
	}
#endif /* BCM_OL_DEV */

#ifdef WLC_LOW_ONLY
	if ((wlc_hw->wdtimer =
	     wl_init_timer(wlc->wl, wlc_bmac_watchdog, wlc, "watchdog")) == NULL) {
		WL_ERROR(("wl%d: %s: wl_init_timer() failed.\n", unit, __FUNCTION__));
		err = 30;
		goto fail;
	}
#ifdef MACOL
	wlc_macol_attach(wlc_hw, (int *)&err);
	if (err != BCME_OK) {
		WL_ERROR(("%s: mac offload attach FAILED (err %d)\n", __FUNCTION__, err));
		goto fail;
	}
#endif /* MACOL */
#ifdef SCANOL
	wlc_scanol_attach(wlc_hw, wlc->wl, (int *)&err);
	if (err != BCME_OK) {
		WL_ERROR(("%s: scan offload attach FAILED (err %d)\n", __FUNCTION__, err));
		goto fail;
	}
	wlc_hw->band->bw_cap = WLC_BW_CAP_20MHZ;	/* SCAN Offload only use BW 20 */
#endif /* SCANOL */

#if (defined(WOWL_GPIO) && defined(WOWL_GPIO_POLARITY))
	wlc_hw->wowl_gpio = WOWL_GPIO;
	wlc_hw->wowl_gpiopol = WOWL_GPIO_POLARITY;
#else
	wlc_hw->wowl_gpio = WOWL_GPIO_INVALID_VALUE;
	wlc_hw->wowl_gpiopol = 1;
#endif // endif
	{
		/* override wowl gpio if defined in nvram */
		char *var;
		if ((var = getvar(wlc_hw->vars, rstr_wowl_gpio)) != NULL)
			wlc_hw->wowl_gpio =  (uint8)bcm_strtoul(var, NULL, 0);
		if ((var = getvar(wlc_hw->vars, rstr_wowl_gpiopol)) != NULL)
			wlc_hw->wowl_gpiopol =  (bool)bcm_strtoul(var, NULL, 0);
	}
#endif /* WLC_LOW_ONLY */

	wlc_hw->dump = wlc_dump_reg_create(wlc_hw->osh, 10);
	if (!wlc_hw->dump) {
		WL_ERROR(("wl%d: %s: wlc_dump_reg_create() failed.\n", unit, __FUNCTION__));
		err = 32;
		goto fail;
	}
	wlc_bmac_register_dumps(wlc_hw);

	wlc_hw->templatebase =
	D11MAC_BMC_TPL_BYTES_PERCORE * wlc_hw->macunit;

	return BCME_OK;

fail:
	WL_ERROR(("wl%d: %s: failed with err %d\n", unit, __FUNCTION__, err));
	return err;
}

/**
 * Initialize wlc_info default values ... may get overrides later in this function.
 * BMAC_NOTES, move low out and resolve the dangling ones.
 */
void
#ifdef WLC_LOW_ONLY
wlc_bmac_info_init(wlc_hw_info_t *wlc_hw)
#else
BCMATTACHFN(wlc_bmac_info_init)(wlc_hw_info_t *wlc_hw)
#endif /* WLC_LOW_ONLY */
{
	wlc_info_t *wlc = wlc_hw->wlc;

	(void)wlc;

	/* set default sw macintmask value */
	wlc_hw->defmacintmask = DEF_MACINTMASK;

	/* set default delayedintmask value */
	wlc_hw->delayedintmask = DELAYEDINTMASK;

	ASSERT(wlc_hw->corerev);

#ifdef WL_PRE_AC_DELAY_NOISE_INT
	/* delay noise interrupt for non ac chips */
	if (D11REV_LT(wlc_hw->corerev, 40)) {
		/* stop BG_NOISE from interrupting host */
		wlc_bmac_set_defmacintmask(wlc_hw, MI_BG_NOISE, ~MI_BG_NOISE);
		/* instead handle BG_NOISE when already interrupted */
		wlc_hw->delayedintmask |= MI_BG_NOISE;
	}
#endif /* WL_PRE_AC_DELAY_NOISE_INT */

	wlc_bmac_set_defmacintmask(wlc_hw, MI_HWACI_NOTIFY, MI_HWACI_NOTIFY);
#ifdef DMATXRC
	/* For D11 >= 40, use I_XI */
	if (DMATXRC_ENAB(wlc->pub) && D11REV_LT(wlc_hw->corerev, 40)) {
		wlc_hw->defmacintmask |= MI_DMATX;

		/* Default to process all fifos */
		wlc->txrc_fifo_mask = (1 << NFIFO) - 1;
	}
#endif // endif

	/* inital value for receive interrupt lazy control */
	wlc_hw->intrcvlazy = WLC_INTRCVLAZY_DEFAULT;

	/* various 802.11g modes */
	wlc_hw->shortslot = FALSE;

	wlc_hw->SFBL = RETRY_SHORT_FB;
	wlc_hw->LFBL = RETRY_LONG_FB;

	/* default mac retry limits */
	wlc_hw->SRL = RETRY_SHORT_DEF;
	wlc_hw->LRL = RETRY_LONG_DEF;
	wlc_hw->chanspec = CH20MHZ_CHSPEC(1);

#ifdef WLRXOV
	wlc->rxov_delay = RXOV_TIMEOUT_MIN;
	wlc->rxov_txmaxpkts = MAXTXPKTS;

	if (WLRXOV_ENAB(wlc->pub))
		wlc_hw->defmacintmask |= MI_RXOV;
#endif // endif

	wlc_hw->btswitch_ovrd_state = AUTO;

#ifdef WLP2P_UCODE
	/* default p2p to enabled */
#ifdef WLP2P_UCODE_ONLY
	wlc_hw->_p2p = TRUE;
#endif // endif
#endif /* WLP2P_UCODE */

#ifdef WL_BCNTRIM
	if (WLC_BCNTRIM_ENAB(wlc->pub))
		wlc_hw->defmacintmask |= MI_BCNTRIM_RX;
#endif // endif

}

/** low level detach */
int
BCMATTACHFN(wlc_bmac_detach)(wlc_info_t *wlc)
{
	uint i;
	wlc_hwband_t *band;
	wlc_hw_info_t *wlc_hw = wlc->hw;
	int callbacks;

	callbacks = 0;

	if (wlc_hw == NULL) {
		return callbacks;
	}

	if (wlc_hw->dump) {
		wlc_dump_reg_destroy(wlc_hw->osh, wlc_hw->dump);
	}

	if (wlc_hw->sih) {
		/* detach interrupt sync mechanism since interrupt is disabled and per-port
		 * interrupt object may has been freed. this must be done before sb core switch
		 */
		si_deregister_intr_callback(wlc_hw->sih);

		if (BUSTYPE(wlc_hw->sih->bustype) == PCI_BUS)
			si_pci_sleep(wlc_hw->sih);
	}

	wlc_bmac_detach_dmapio(wlc_hw);

	band = wlc_hw->band;
	for (i = 0; i < NBANDS_HW(wlc_hw); i++) {
		/* So if this is a single band 11a card, use band 1 */
		if (IS_SINGLEBAND_5G(wlc_hw->deviceid))
			i = BAND_5G_INDEX;

		if (band->pi) {
			/* Detach this band's phy */
			phy_module_detach((phy_info_t *)band->pi);
			band->pi = NULL;
		}
		band = wlc_hw->bandstate[OTHERBANDUNIT(wlc_hw)];
	}

	/* Free shared phy state */
	wlc_phy_shared_detach(wlc_hw->phy_sh);

	wlc_phy_shim_detach(wlc_hw->physhim);

	/* free vars */
	/*
	 * we are done with vars now, let wlc_detach take care of freeing it.
	 */
	wlc_hw->vars = NULL;

	/*
	 * we are done with sih now, let wlc_detach take care of freeing it.
	 */
	wlc_hw->sih = NULL;

#ifdef WLLED
	if (wlc_hw->ledh) {
		callbacks += wlc_bmac_led_detach(wlc_hw);
		wlc_hw->ledh = NULL;
	}
#endif // endif
#ifdef AP
	wlc_bmac_pmq_delete(wlc_hw);
#endif // endif

#ifdef WLC_LOW_ONLY
	wlc_macol_detach(wlc_hw);
	wlc_scanol_detach(wlc_hw);
#endif /* WLC_LOW_ONLY */

#ifdef BCM_OL_DEV
	wlc_keymgmt_detach(wlc->keymgmt);
#endif /* BCM_OL_DEV */

#ifdef WLEXTLOG
#ifdef WLC_LOW_ONLY
	if (wlc_hw->extlog) {
		callbacks += wlc_extlog_detach(wlc_hw->extlog);
		wlc_hw->extlog = NULL;
	}
#endif // endif
#endif /* WLEXTLOG */

#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(BCMDBG_PHYDUMP)
	if (wlc_hw->suspend_stats) {
		MFREE(wlc_hw->osh, wlc_hw->suspend_stats, sizeof(*wlc_hw->suspend_stats));
		wlc_hw->suspend_stats = NULL;
	}
#endif /* defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(BCMDBG_PHYDUMP) */

#ifdef DMATXRC
	if (DMATXRC_ENAB(wlc->pub))
		wlc_phdr_detach(wlc);
#endif // endif

	if (wlc->btc_param_vars) {
		MFREE(wlc_hw->osh, wlc->btc_param_vars,
			sizeof(struct wlc_btc_param_vars_info) + wlc->btc_param_vars->num_entries
				* sizeof(struct wlc_btc_param_vars_entry));
		wlc->btc_param_vars = NULL;
	}
#if !defined(WLTEST)
	/* Free btc_params state */
	if (wlc_hw->btc->wlc_btc_params) {
		MFREE(wlc->osh, wlc_hw->btc->wlc_btc_params,
			M_BTCX_BACKUP_SIZE*sizeof(uint16));
		wlc_hw->btc->wlc_btc_params = NULL;
	}
#endif /* !defined(WLTEST) */

	if (wlc_hw->btc->wlc_btc_params_fw) {
		MFREE(wlc->osh, wlc_hw->btc->wlc_btc_params_fw,
			BTC_FW_MAX_INDICES*sizeof(uint16));
		wlc_hw->btc->wlc_btc_params_fw = NULL;
	}

#ifdef WLC_LOW_ONLY
	/* free timer state */
	if (wlc_hw->wdtimer) {
		wl_free_timer(wlc->wl, wlc_hw->wdtimer);
		wlc_hw->wdtimer = NULL;
	}
#endif /* WLC_LOW_ONLY */

	if (wlc_hw->iocvi != NULL)
		wlc_iocv_detach(wlc_hw->iocvi);

	wlc_hw_detach(wlc_hw);
	wlc->hw = NULL;

#ifdef HOST_HDR_FETCH
	if (HOST_HDR_FETCH_ENAB()) {
		/* Detach txstatus fetch request pool */
		wlc_txs_fetch_rqst_detach(wlc);
	}
#endif // endif
	return callbacks;

}

/** d11 core needs to be reset during a 'wl up' or 'wl down' */
void
BCMINITFN(wlc_bmac_reset)(wlc_hw_info_t *wlc_hw)
{
	WL_TRACE(("wl%d: wlc_bmac_reset\n", wlc_hw->unit));

	WLCNTINCR(wlc_hw->wlc->pub->_cnt->reset);

	/* reset the core */
	if (!DEVICEREMOVED(wlc_hw->wlc))
		wlc_bmac_corereset(wlc_hw, WLC_USE_COREFLAGS);

	/* purge the pio queues or dma rings */
	wlc_hw->reinit = TRUE;
	wlc_flushqueues(wlc_hw);
#if defined(BTCX_ENABLED)
	/* save a copy of the btc params before going down */
	wlc_bmac_btc_params_save(wlc_hw);
#endif /* BTCX_ENABLED */
#ifndef BCM_OL_DEV
	wlc_reset_bmac_done(wlc_hw->wlc);
#endif // endif
}

/** d11 core needs to be initialized during a 'wl up' */
void
BCMINITFN(wlc_bmac_init)(wlc_hw_info_t *wlc_hw, chanspec_t chanspec, bool mute,
	uint32 defmacintmask)
{
	uint32 macintmask;
	bool fastclk;
	wlc_info_t *wlc = wlc_hw->wlc;

	WL_TRACE(("wl%d: wlc_bmac_init\n", wlc_hw->unit));

#if defined(WLC_LOW_ONLY) && defined(MBSS)
	wlc_hw->defmacintmask |= defmacintmask;
#else
	UNUSED_PARAMETER(defmacintmask);
#endif // endif
	/* request FAST clock if not on */
	if (!(fastclk = wlc_hw->forcefastclk))
		wlc_clkctl_clk(wlc_hw, CLK_FAST);

	/* disable interrupts */
	macintmask = wl_intrsoff(wlc->wl);

	if ((CHIPID(wlc_hw->sih->chip) == BCM4360_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM43602_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM4352_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM43526_CHIP_ID))
		si_pmu_rfldo(wlc_hw->sih, 1);

	/* set up the specified band and chanspec */
	wlc_setxband(wlc_hw, CHSPEC_WLCBANDUNIT(chanspec));
	wlc_phy_chanspec_radio_set(wlc_hw->band->pi, chanspec);
	wlc_hw->chanspec = chanspec;

	/* do one-time phy inits and calibration */
	wlc_phy_cal_init(wlc_hw->band->pi);

	/* core-specific initialization. E.g. load and initialize ucode. */
	wlc_coreinit(wlc_hw);

	/* CRWLDOT11M-2187 WAR: Always force backplane HT clock with CTDMA in 4365 */
	if (D11REV_IS(wlc_hw->corerev, 64) || D11REV_IS(wlc_hw->corerev, 65)) {
		if (BCM_DMA_CT_ENAB(wlc_hw->wlc))
			fastclk = TRUE;
		else
			fastclk = FALSE;
	}

	/*
	 * initialize mac_suspend_depth to 1 to match ucode initial suspended state
	 */
	wlc_hw->mac_suspend_depth = 1;
#if defined(WL_PSMX)
	wlc_hw->macx_suspend_depth = 1;
#endif // endif

	/* suspend the tx fifos and mute the phy for preism cac time */
	if (mute)
		wlc_bmac_mute(wlc_hw, ON, PHY_MUTE_FOR_PREISM);

	/* XXX:
	 * For HTPHY, must switch radio off to allow pi->radio_is_on to be cleared.
	 * HTPHY wlc_phy_switch_radio_htphy() checks for pi->radio_is_on.
	 * if pi->radio_is_on is set, wlc_phy_radio2059_init and wlc_phy_chanspec_set
	 * will not get called when switching radio back on.
	 * In cases when "wl reinit" or big hammer is triggered, not calling
	 * wlc_phy_radio2059_init and/or wlc_phy_chanspec_set in wlc_phy_switch_radio_htphy()
	 * when switching radio on in wlc_bmac_bsinit()
	 * will cause the driver to lose tx/rx capabilities and lose connection to AP.
	 */
	phy_radio_init((phy_info_t *)wlc_hw->band->pi);

	if ((CHIPID(wlc_hw->sih->chip) == BCM4360_CHIP_ID) ||
	    (CHIPID(wlc_hw->sih->chip) == BCM43460_CHIP_ID) ||
	    (CHIPID(wlc_hw->sih->chip) == BCM43602_CHIP_ID) ||
	    (CHIPID(wlc_hw->sih->chip) == BCM43526_CHIP_ID) ||
	    (CHIPID(wlc_hw->sih->chip) == BCM4352_CHIP_ID) ||
	    BCM4350_CHIP(wlc_hw->sih->chip)) {
		/**
		 * JIRA:SWWLAN-26291. Whenever driver changes BBPLL frequency it needs to adjust
		 * the TSF clock as well.
		 */
		wlc_bmac_switch_macfreq(wlc_hw, 0);
	}

#ifndef BCM_OL_DEV
	/* band-specific inits */
	wlc_bmac_bsinit(wlc_hw, chanspec, FALSE);

#ifdef WLC_LOW
#if !defined(PCIE_PHANTOM_DEV) && !defined(WL_PROXDETECT)
	/* Phantom devices use sdio & usb core dma to do message transfer */
	/* Low power modes will switch off cores other than host bus */
	/* TOF AVB timer CLK won't work when si_lowpwr_opt is called */
	si_lowpwr_opt(wlc_hw->sih);
#endif // endif
#endif /* WLC_LOW */
#endif /* BCM_OL_DEV */

	/* vasip intialization */
	/* TBD: Move VASIP initialization to PHY init */
	wlc_vasip_init(wlc_hw);

	/* restore macintmask */
	wl_intrsrestore(wlc->wl, macintmask);

#if defined(WL_PSMX) && defined(WL_AIR_IQ)
	if (D11REV_GE(wlc_hw->corerev, 65)) {
		/* setup PSMX interrupt mask */
		SET_MACINTMASK_X(wlc_hw->osh, wlc_hw->regs, DEF_MACINTMASK_X);
	}
#endif /* WL_PSMX && WL_AIR_IQ */

	/* seed wake_override with WLC_WAKE_OVERRIDE_MACSUSPEND since the mac is suspended
	 * and wlc_bmac_enable_mac() will clear this override bit.
	 */
	mboolset(wlc_hw->wake_override, WLC_WAKE_OVERRIDE_MACSUSPEND);

	/* restore the clk */
	if (!fastclk)
		wlc_clkctl_clk(wlc_hw, CLK_DYNAMIC);

	/* XXX 4314/43142 (pcie rev 21, 22) L0s is unstable. Enabling L0s might
	 * cause hang with some RC. This has been observed with Dell 6410.
	 */
	if (CHIPID(wlc_hw->sih->chip) == BCM4314_CHIP_ID ||
		CHIPID(wlc_hw->sih->chip) == BCM43142_CHIP_ID) {
		uint32 tmp;
		tmp = si_pcielcreg(wlc_hw->sih, 0, 0);
		tmp &= ~0x1;				/* disable L0s */
		si_pcielcreg(wlc_hw->sih, 3, tmp);
	}

	wlc_hw->reinit = FALSE;
}

int
BCMINITFN(wlc_bmac_4331_epa_init)(wlc_hw_info_t *wlc_hw)
{
#define GPIO_5	(1<<5)
	bool is_4331_12x9 = FALSE;

	WL_TRACE(("wl%d: %s\n", wlc_hw->unit, __FUNCTION__));

	if ((BUSTYPE(wlc_hw->sih->bustype) == PCI_BUS) &&
	    ((CHIPID(wlc_hw->sih->chip) == BCM4331_CHIP_ID) ||
	     (CHIPID(wlc_hw->sih->chip) == BCM43431_CHIP_ID)))
		is_4331_12x9 = ((wlc_hw->sih->chippkg == 9 || wlc_hw->sih->chippkg == 0xb));

	if (!is_4331_12x9)
		return (-1);

	si_gpiopull(wlc_hw->sih, GPIO_PULLUP, GPIO_5, 0);
	si_gpiopull(wlc_hw->sih, GPIO_PULLDN, GPIO_5, GPIO_5);

	/* give the control to chip common */
	si_gpiocontrol(wlc_hw->sih, GPIO_2_PA_CTRL_5G_0, 0, GPIO_DRV_PRIORITY);
	/* drive the output to 0 */
	si_gpioout(wlc_hw->sih, GPIO_2_PA_CTRL_5G_0, 0, GPIO_DRV_PRIORITY);
	/* set output disable */
	si_gpioouten(wlc_hw->sih, GPIO_2_PA_CTRL_5G_0, 0, GPIO_DRV_PRIORITY);
	return 0;
}

static void
BCMINITFN(wlc_bmac_config_4331_5GePA)(wlc_hw_info_t *wlc_hw)
{
	bool is_4331_12x9 = FALSE;
	if ((BUSTYPE(wlc_hw->sih->bustype) == PCI_BUS) &&
	    ((CHIPID(wlc_hw->sih->chip) == BCM4331_CHIP_ID) ||
	     (CHIPID(wlc_hw->sih->chip) == BCM43431_CHIP_ID)))
		is_4331_12x9 = ((wlc_hw->sih->chippkg == 9 || wlc_hw->sih->chippkg == 0xb));

	if (!is_4331_12x9)
		return;
	/* XXX WAR for pin mux between ePA & SROM for 4331 12x9 package
	 * always clear the 5G ePA WAR
	 */
	wlc_hw->band->mhfs[MHF1] &= ~MHF1_4331EPA_WAR;
	wlc_write_mhf(wlc_hw, wlc_hw->band->mhfs);
}

/** called during 'wl up' (after wlc_bmac_init), or on a 'big hammer' event */
int
BCMINITFN(wlc_bmac_up_prep)(wlc_hw_info_t *wlc_hw)
{
	uint coremask;
	wlc_tunables_t *tune = wlc_hw->wlc->pub->tunables;

	WL_TRACE(("wl%d: %s:\n", wlc_hw->unit, __FUNCTION__));

#ifdef ATE_BUILD
	if (wlc_hw->macintmask != 0) {
		wl_intrsoff(wlc_hw->wlc->wl);
	}
#endif // endif
	ASSERT(wlc_hw->wlc->pub->hw_up && wlc_hw->macintmask == 0);

	if (BCM4350_CHIP(wlc_hw->sih->chip) &&
	    (CHIPREV(wlc_hw->sih->chiprev) == 0)) {
		si_pmu_chipcontrol(wlc_hw->sih, PMU_CHIPCTL2,
			PMU_CC2_FORCE_PHY_PWR_SWITCH_ON,
			PMU_CC2_FORCE_PHY_PWR_SWITCH_ON);
	}

	/*
	 * Enable pll and xtal, initialize the power control registers,
	 * and force fastclock for the remainder of wlc_up().
	 */
	wlc_bmac_xtal(wlc_hw, ON);
	si_clkctl_init(wlc_hw->sih);
	wlc_clkctl_clk(wlc_hw, CLK_FAST);

	/*
	 * Configure pci/pcmcia here instead of in wlc_attach()
	 * to allow mfg hotswap:  down, hotswap (chip power cycle), up.
	 */
	coremask = (1 << wlc_hw->wlc->core->coreidx);

	if (BUSTYPE(wlc_hw->sih->bustype) == PCI_BUS)
		si_pci_setup(wlc_hw->sih, coremask);
	else if (BUSTYPE(wlc_hw->sih->bustype) == PCMCIA_BUS) {
		wlc_hw->regs = (d11regs_t*)si_setcore(wlc_hw->sih, D11_CORE_ID, wlc_hw->macunit);
		ASSERT(wlc_hw->regs != NULL);
		wlc_hw->wlc->regs = wlc_hw->regs;
		si_pcmcia_init(wlc_hw->sih);
	}
#ifdef BCMSDIO
	else if (BUSTYPE(wlc_hw->sih->bustype) == SDIO_BUS) {
		wlc_hw->regs = (d11regs_t*)si_setcore(wlc_hw->sih, D11_CORE_ID, wlc_hw->macunit);
		ASSERT(wlc_hw->regs != NULL);
		wlc_hw->wlc->regs = wlc_hw->regs;
		si_sdio_init(wlc_hw->sih);
	}
#endif /* BCMSDIO */
	ASSERT(si_coreid(wlc_hw->sih) == D11_CORE_ID);

	/*
	 * Need to read the hwradio status here to cover the case where the system
	 * is loaded with the hw radio disabled. We do not want to bring the driver up in this case.
	 */
	if (wlc_bmac_radio_read_hwdisabled(wlc_hw)) {
		/* put SB PCI in down state again */
		if (BUSTYPE(wlc_hw->sih->bustype) == PCI_BUS)
			si_pci_down(wlc_hw->sih);
		wlc_bmac_xtal(wlc_hw, OFF);
		return BCME_RADIOOFF;
	}

	if (BUSTYPE(wlc_hw->sih->bustype) == PCI_BUS) {
		if (tune->mrrs != AUTO) {
			si_pcie_set_request_size(wlc_hw->sih, (uint16)tune->mrrs);
			si_pcie_set_maxpayload_size(wlc_hw->sih, (uint16)tune->mrrs);
		}

		si_pci_up(wlc_hw->sih);
	}

	/* Jira: SWWLAN-47716: In the down path, the FEM control has been overridden.
	 * Restore FEM control back to its default.
	 */
	if (CHIPID(wlc_hw->sih->chip) == BCM43602_CHIP_ID) {
		si_pmu_chipcontrol(wlc_hw->sih, CHIPCTRLREG1, PMU43602_CC1_GPIO12_OVRD, 0);
	}

	/* reset the d11 core */
	wlc_bmac_corereset(wlc_hw, WLC_USE_COREFLAGS);

	return 0;
} /* wlc_bmac_up_prep */

/** called during 'wl up', after the chanspec has been set */
int
BCMINITFN(wlc_bmac_up_finish)(wlc_hw_info_t *wlc_hw)
{
	bool disable_dynamic_clock = FALSE;
	WL_TRACE(("wl%d: %s:\n", wlc_hw->unit, __FUNCTION__));

#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(BCMDBG_PHYDUMP)
	bzero(wlc_hw->suspend_stats, sizeof(*wlc_hw->suspend_stats));
	wlc_hw->suspend_stats->suspend_start = (uint32)-1;
	wlc_hw->suspend_stats->suspend_end = (uint32)-1;
#endif // endif
	wlc_hw->up = TRUE;

#ifndef BCM_OL_DEV
	wlc_phy_hw_state_upd(wlc_hw->band->pi, TRUE);
#endif // endif

	/* CRWLDOT11M-2187 WAR: Always force backplane HT clock in 4365 */
	if (BCM_DMA_CT_ENAB(wlc_hw->wlc) && (D11REV_IS(wlc_hw->corerev, 64) ||
		D11REV_IS(wlc_hw->corerev, 65))) {
		disable_dynamic_clock = TRUE;
	}
	/* FULLY enable dynamic power control and d11 core interrupt */
	if (!disable_dynamic_clock)
		wlc_clkctl_clk(wlc_hw, CLK_DYNAMIC);
	ASSERT(wlc_hw->macintmask == 0);
	ASSERT(!wlc_hw->sbclk || !si_taclear(wlc_hw->sih, TRUE));
	wl_intrson(wlc_hw->wlc->wl);

#ifdef WLC_LOW_ONLY
	/* start one second watchdog timer */
	wl_add_timer(wlc_hw->wlc->wl, wlc_hw->wdtimer, TIMER_INTERVAL_WATCHDOG_BMAC, TRUE);
#ifndef BCM_OL_DEV
	wl_add_timer(wlc_hw->wlc->wl, wlc_hw->rpc_agg_wdtimer,
		TIMER_INTERVAL_RPC_AGG_WATCHDOG_BMAC, TRUE);
#endif /* BCM_OL_DEV */
#endif /* WLC_LOW_ONLY */
#if NCONF || HTCONF || ACCONF || ACCONF2
	wlc_bmac_ifsctl_edcrs_set(wlc_hw, WLCISHTPHY(wlc_hw->band));
#endif /* NCONF */
	return 0;
}

/** On some chips, pins are multiplexed and serve either an SROM or WLAN specific function */
int
BCMINITFN(wlc_bmac_set_ctrl_SROM)(wlc_hw_info_t *wlc_hw)
{
	WL_TRACE(("wl%d: %s:\n", wlc_hw->unit, __FUNCTION__));

	if (BUSTYPE(wlc_hw->sih->bustype) == PCI_BUS) {
		if ((CHIPID(wlc_hw->sih->chip) == BCM4331_CHIP_ID) ||
			(CHIPID(wlc_hw->sih->chip) == BCM43431_CHIP_ID)) {
			WL_INFORM(("wl%d: %s: set mux pin to SROM\n", wlc_hw->unit, __FUNCTION__));
			/* force muxed pin to control SROM */
			si_chipcontrl_epa4331(wlc_hw->sih, FALSE);
		} else if (((CHIPID(wlc_hw->sih->chip) == BCM4360_CHIP_ID) ||
			(CHIPID(wlc_hw->sih->chip) == BCM43460_CHIP_ID) ||
			(CHIPID(wlc_hw->sih->chip) == BCM43602_CHIP_ID) ||
			(CHIPID(wlc_hw->sih->chip) == BCM4352_CHIP_ID)) &&
			(CHIPREV(wlc_hw->sih->chiprev) <= 2)) {
			si_chipcontrl_srom4360(wlc_hw->sih, TRUE);
		}
	}

	return 0;
}

int
BCMINITFN(wlc_bmac_set_ctrl_ePA)(wlc_hw_info_t *wlc_hw)
{
	WL_TRACE(("wl%d: %s:\n", wlc_hw->unit, __FUNCTION__));

	if (!wlc_hw->clk) {
		WL_ERROR(("wl%d: %s: NO CLK\n", wlc_hw->unit, __FUNCTION__));
		return -1;
	}
	if (BUSTYPE(wlc_hw->sih->bustype) == PCI_BUS) {
		if ((CHIPID(wlc_hw->sih->chip) == BCM4331_CHIP_ID) ||
		    (CHIPID(wlc_hw->sih->chip) == BCM43431_CHIP_ID)) {
			WL_INFORM(("wl%d: %s: set mux pin to ePA\n", wlc_hw->unit, __FUNCTION__));
			/* force muxed pin to control ePA */
			si_chipcontrl_epa4331(wlc_hw->sih, TRUE);
		}
	}

	return 0;
}

int
BCMINITFN(wlc_bmac_set_ctrl_bt_shd0)(wlc_hw_info_t *wlc_hw, bool enable)
{
	WL_TRACE(("wl%d: %s:\n", wlc_hw->unit, __FUNCTION__));

	if (BUSTYPE(wlc_hw->sih->bustype) == PCI_BUS) {
		if (((CHIPID(wlc_hw->sih->chip) == BCM4331_CHIP_ID) ||
		     (CHIPID(wlc_hw->sih->chip) == BCM43431_CHIP_ID)) &&
		    ((wlc_hw->sih->boardtype == BCM94331X28) ||
		     (wlc_hw->sih->boardtype == BCM94331X28B) ||
		     (wlc_hw->sih->boardtype == BCM94331CS_SSID) ||
		     (wlc_hw->sih->boardtype == BCM94331X29B) ||
		     (wlc_hw->sih->boardtype == BCM94331X29D))) {
			if (enable) {
				/* force muxed pin to bt_shd0 */
				WL_INFORM(("wl%d: %s: set mux pin to bt_shd0\n",
				           wlc_hw->unit, __FUNCTION__));
				si_chipcontrl_btshd0_4331(wlc_hw->sih, TRUE);
			} else {
				/* restore muxed pin to default state */
				WL_INFORM(("wl%d: %s: set mux pin to default (gpio4) \n",
				           wlc_hw->unit, __FUNCTION__));
				si_chipcontrl_btshd0_4331(wlc_hw->sih, FALSE);
			}
		}
	}

	return 0;
}

#ifndef BCMNODOWN
/** tear down d11 interrupts, cancel BMAC software timers, tear down PHY operation */
int
BCMUNINITFN(wlc_bmac_down_prep)(wlc_hw_info_t *wlc_hw)
{
	bool dev_gone;
	uint callbacks = 0;

	WL_TRACE(("wl%d: %s:\n", wlc_hw->unit, __FUNCTION__));

	if (!wlc_hw->up)
		return callbacks;

	/* Forcing it here.
	 * As after core 0 down complete, core 1 MCTL_IHR_EN bit is reset
	 */
	if (wlc_hw->macunit == 1) {
		OR_REG(wlc_hw->osh, &wlc_hw->regs->maccontrol, MCTL_IHR_EN);
	}
	dev_gone = DEVICEREMOVED(wlc_hw->wlc);

	/* disable interrupts */
	if (dev_gone)
		wlc_hw->macintmask = 0;
	else {
		/* now disable interrupts */
		wl_intrsoff(wlc_hw->wlc->wl);

		/* ensure we're running on the pll clock again */
		wlc_clkctl_clk(wlc_hw, CLK_FAST);

		/* Disable GPIOs related to BTC returning the control to chipcommon */
		if (!wlc_hw->noreset)
			wlc_bmac_btc_gpio_disable(wlc_hw);
	}

	if (CHIPID(wlc_hw->sih->chip) == BCM4331_CHIP_ID) {
		wlc_bmac_write_shm(wlc_hw, M_EXTLNA_PWRSAVE, 0x480);
	}
#ifdef WLC_LOW_ONLY
	/* cancel the watchdog timer */
	if (!wl_del_timer(wlc_hw->wlc->wl, wlc_hw->wdtimer))
		callbacks++;
#ifndef BCM_OL_DEV
	/* cancel the rpc agg watchdog timer */
	if (!wl_del_timer(wlc_hw->wlc->wl, wlc_hw->rpc_agg_wdtimer))
		callbacks++;
#endif /* BCM_OL_DEV */
#endif /* WLC_LOW_ONLY */
#if defined(BTCX_ENABLED)
	/* save a copy of the btc params before going down */
	wlc_bmac_btc_params_save(wlc_hw);
#endif /* BTCX_ENABLED */
	/* down phy at the last of this stage */
	callbacks += phy_down((phy_info_t *)wlc_hw->band->pi);

	return callbacks;
}

void
BCMUNINITFN(wlc_bmac_hw_down)(wlc_hw_info_t *wlc_hw)
{
	if (BUSTYPE(wlc_hw->sih->bustype) == PCI_BUS) {
		wlc_bmac_set_ctrl_SROM(wlc_hw);

#ifdef BCMECICOEX
		/* seci down */
		if (BCMSECICOEX_ENAB_BMAC(wlc_hw))
			si_seci_down(wlc_hw->sih);
#endif /* BCMECICOEX */
		wlc_bmac_set_ctrl_bt_shd0(wlc_hw, FALSE);
#ifdef WLOFFLD
		if (WLOFFLD_CAP(wlc_hw->wlc)) {
			/* clear survive perst and enable srom download */
			si_survive_perst_war(wlc_hw->sih, FALSE, (PCIE_SPERST|PCIE_DISSPROMLD), 0);
			/* clear any pending timer interrupt from the ARM */
			si_pmu_res_req_timer_clr(wlc_hw->sih);
		}
#endif // endif
		si_pci_down(wlc_hw->sih);
	}

	/* Jira: SWWLAN-47716: override the FEM control to GPIO (High-Z) so that in down state
	 * the pin is not driven low which causes excess current draw.
	 */
	if (CHIPID(wlc_hw->sih->chip) == BCM43602_CHIP_ID) {
		si_pmu_chipcontrol(wlc_hw->sih, CHIPCTRLREG1,
			PMU43602_CC1_GPIO12_OVRD, PMU43602_CC1_GPIO12_OVRD);
	}

	if ((CHIPID(wlc_hw->sih->chip) == BCM4360_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM43602_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM4352_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM43526_CHIP_ID))
		si_pmu_rfldo(wlc_hw->sih, 0);

	wlc_bmac_xtal(wlc_hw, OFF);

	if (BCM4350_CHIP(wlc_hw->sih->chip) &&
	    (CHIPREV(wlc_hw->sih->chiprev) == 0)) {
		si_pmu_chipcontrol(wlc_hw->sih, PMU_CHIPCTL2, PMU_CC2_FORCE_PHY_PWR_SWITCH_ON, 0);
	}
}

int
BCMUNINITFN(wlc_bmac_down_finish)(wlc_hw_info_t *wlc_hw)
{
	uint callbacks = 0;
	bool dev_gone;

	WL_TRACE(("wl%d: %s:\n", wlc_hw->unit, __FUNCTION__));

	if (!wlc_hw->up)
		return callbacks;

	wlc_hw->up = FALSE;
	wlc_phy_hw_state_upd(wlc_hw->band->pi, FALSE);

	dev_gone = DEVICEREMOVED(wlc_hw->wlc);

	if (dev_gone) {
		wlc_hw->sbclk = FALSE;
		wlc_hw->clk = FALSE;
		wlc_phy_hw_clk_state_upd(wlc_hw->band->pi, FALSE);

		/* reclaim any posted packets */
		wlc_flushqueues(wlc_hw);
	} else if (!wlc_hw->wlc->psm_watchdog_debug) {

		/* Reset and disable the core */
		if (si_iscoreup(wlc_hw->sih)) {
			if (R_REG(wlc_hw->osh, &wlc_hw->regs->maccontrol) & MCTL_EN_MAC) {
				wlc_bmac_suspend_mac_and_wait(wlc_hw);
			}
			if (D11REV_GE(wlc_hw->corerev, 64) &&
				(R_REG(wlc_hw->osh, &wlc_hw->regs->maccontrol_x) & MCTL_EN_MAC)) {
				wlc_bmac_suspend_macx_and_wait(wlc_hw);
			}
			callbacks += wl_reset(wlc_hw->wlc->wl);
			wlc_coredisable(wlc_hw);
		}

		/* turn off primary xtal and pll */
		if (!wlc_hw->noreset)
			wlc_bmac_hw_down(wlc_hw);
	}

	ASSERT(!wlc_hw->sbclk || !si_taclear(wlc_hw->sih, TRUE));

	return callbacks;
}
#endif	/* BCMNODOWN */

/** 802.11 Power State (PS) related */
void
wlc_bmac_wait_for_wake(wlc_hw_info_t *wlc_hw)
{
	if (D11REV_IS(wlc_hw->corerev, 4)) /* no slowclock */
		OSL_DELAY(5);
	else {
		/* delay before first read of ucode state */
		if (!(wlc_hw->band)) {
			WL_ERROR(("wl%d: %s:Active per-band state not set. \n",
				wlc_hw->unit, __FUNCTION__));
			return;
		}
		if ((WLCISGPHY(wlc_hw->band)) && (D11REV_IS(wlc_hw->corerev, 5))) {
			OSL_DELAY(2000);
		} else {
			OSL_DELAY(40);
		}

		/* wait until ucode is no longer asleep */
		SPINWAIT((wlc_bmac_read_shm(wlc_hw, M_UCODE_DBGST) == DBGST_ASLEEP),
		         wlc_hw->fastpwrup_dly);
	}
#ifdef WAR4360_UCODE
	if (wlc_bmac_read_shm(wlc_hw, M_UCODE_DBGST) == DBGST_ASLEEP) {
		WL_PRINT(("wl%d:%s: Hammering due to M_UCODE_DBGST==DBGST_ASLEEP\n",
			wlc_hw->unit, __FUNCTION__));
		wlc_hw->need_reinit = 3;
#ifdef WLOFFLD
		if (WLOFFLD_ENAB(wlc_hw->wlc->pub))
			wlc_ol_down(wlc_hw->wlc->ol);
#endif // endif
		return;
	}
#endif /* WAR4360_UCODE */
	ASSERT(wlc_bmac_read_shm(wlc_hw, M_UCODE_DBGST) != DBGST_ASLEEP);
}

void
wlc_bmac_hw_etheraddr(wlc_hw_info_t *wlc_hw, struct ether_addr *ea)
{
	bcopy(&wlc_hw->etheraddr, ea, ETHER_ADDR_LEN);
}

void
wlc_bmac_set_hw_etheraddr(wlc_hw_info_t *wlc_hw, struct ether_addr *ea)
{
	bcopy(ea, &wlc_hw->etheraddr, ETHER_ADDR_LEN);
}

int
wlc_bmac_bandtype(wlc_hw_info_t *wlc_hw)
{
	return (wlc_hw->band->bandtype);
}

void *
wlc_cur_phy(wlc_info_t *wlc)
{
	wlc_hw_info_t *wlc_hw = wlc->hw;
	return ((void *)wlc_hw->band->pi);
}

/** control chip clock to save power, enable dynamic clock or force fast clock */
static void
wlc_clkctl_clk(wlc_hw_info_t *wlc_hw, uint mode)
{
	if (PMUCTL_ENAB(wlc_hw->sih)) {
		/* new chips with PMU, CCS_FORCEHT will distribute the HT clock on backplane,
		 *  but mac core will still run on ALP(not HT) when it enters powersave mode,
		 *      which means the FCA bit may not be set.
		 *      should wakeup mac if driver wants it to run on HT.
		 */

		if (wlc_hw->clk) {
			if (mode == CLK_FAST) {
				OR_REG(wlc_hw->osh, &wlc_hw->regs->clk_ctl_st, CCS_FORCEHT);

				/* PR53224 PR53908: PMU could be in ILP clock while we reset
				 * d11 core. Thus, if we do forceht after a d11 reset, it
				 * might take 1 ILP clock cycle (32us) + 2 ALP clocks (~100ns)
				 * to get through all the transitions before having HT clock
				 * ready. Put 64us instead of the minimum 33us here for some
				 * margin.
				 */
				OSL_DELAY(64);

				if (wlc_hw->sih->pmurev >= 0x1c)
					/* Need one more ILP clock cycle do forceht after a
					 * d11 reset.
					 */
					OSL_DELAY(32);

				SPINWAIT(((R_REG(wlc_hw->osh, &wlc_hw->regs->clk_ctl_st) &
				           CCS_HTAVAIL) == 0), PMU_MAX_TRANSITION_DLY);
				ASSERT(R_REG(wlc_hw->osh, &wlc_hw->regs->clk_ctl_st) &
					CCS_HTAVAIL);
			} else {
				if ((wlc_hw->sih->pmurev == 0) &&
				    (R_REG(wlc_hw->osh, &wlc_hw->regs->clk_ctl_st) &
				     (CCS_FORCEHT | CCS_HTAREQ)))
					SPINWAIT(((R_REG(wlc_hw->osh, &wlc_hw->regs->clk_ctl_st) &
					           CCS_HTAVAIL) == 0), PMU_MAX_TRANSITION_DLY);
				AND_REG(wlc_hw->osh, &wlc_hw->regs->clk_ctl_st, ~CCS_FORCEHT);
			}
		}
		wlc_hw->forcefastclk = (mode == CLK_FAST);
	} else {
		bool wakeup_ucode;

		/* old chips w/o PMU, force HT through cc,
		 * then use FCA to verify mac is running fast clock
		 */

		/*
		 * PR18373: ucode works around some d11 chip bugs by sometimes
		 * doing a spinwait for the slowclk signal.  On d11 core rev 5-9,
		 * if we ever force the fast clk and don't do this workaround,
		 * the ucode will wedge spinning on a signal that never comes.
		 */
		wakeup_ucode = D11REV_LT(wlc_hw->corerev, 9);

		if (wlc_hw->up && wakeup_ucode)
			wlc_ucode_wake_override_set(wlc_hw, WLC_WAKE_OVERRIDE_CLKCTL);

		wlc_hw->forcefastclk = si_clkctl_cc(wlc_hw->sih, mode);

		if (D11REV_LT(wlc_hw->corerev, 11)) {
			/* ucode WAR for old chips */
			if (wlc_hw->forcefastclk)
				wlc_bmac_mhf(wlc_hw, MHF1, MHF1_FORCEFASTCLK, MHF1_FORCEFASTCLK,
				        WLC_BAND_ALL);
			else
				wlc_bmac_mhf(wlc_hw, MHF1, MHF1_FORCEFASTCLK, 0, WLC_BAND_ALL);
		}

		/* check fast clock is available (if core is not in reset) */
		if (D11REV_GT(wlc_hw->corerev, 4) && wlc_hw->forcefastclk && wlc_hw->clk)
			ASSERT(si_core_sflags(wlc_hw->sih, 0, 0) & SISF_FCLKA);

		/* keep the ucode wake bit on if forcefastclk is on
		 * since we do not want ucode to put us back to slow clock
		 * when it dozes for PM mode.
		 * Code below matches the wake override bit with current forcefastclk state
		 * Only setting bit in wake_override instead of waking ucode immediately
		 * since old code (wlc.c 1.4499) had this behavior. Older code set
		 * wlc->forcefastclk but only had the wake happen if the wakup_ucode work
		 * (protected by an up check) was executed just below.
		 */
		if (wlc_hw->forcefastclk)
			mboolset(wlc_hw->wake_override, WLC_WAKE_OVERRIDE_FORCEFAST);
		else
			mboolclr(wlc_hw->wake_override, WLC_WAKE_OVERRIDE_FORCEFAST);

		/* ok to clear the wakeup now */
		if (wlc_hw->up && wakeup_ucode)
			wlc_ucode_wake_override_clear(wlc_hw, WLC_WAKE_OVERRIDE_CLKCTL);
	}
}

/* Forcing Core1's HW request Off bit in PM Mode for MIMO and 80P80 */
void
wlc_bmac_4349_core1_hwreqoff(wlc_hw_info_t *wlc_hw, bool mode)
{
	int sicoreunit = 0;

	ASSERT(wlc_hw != NULL);
	sicoreunit = si_coreunit(wlc_hw->sih);

	/* Apply the setting to D11 core unit one always */
		if ((wlc_rsdb_mode(wlc_hw->wlc) == PHYMODE_MIMO) ||
		(wlc_rsdb_mode(wlc_hw->wlc) == PHYMODE_80P80)) {
			d11regs_t *regs = si_d11_switch_addrbase(wlc_hw->sih, 1);
			if (mode == TRUE)
				OR_REG(wlc_hw->osh, &regs->clk_ctl_st, CCS_FORCEHWREQOFF);
			else
				AND_REG(wlc_hw->osh, &regs->clk_ctl_st, ~CCS_FORCEHWREQOFF);
		}

	si_d11_switch_addrbase(wlc_hw->sih, sicoreunit);
}

#if !defined(BCM_OL_DEV) || (defined(BCMDBG) || defined(MACOSX)) || defined(__NetBSD__)
/**
 * Update the hardware for rcvlazy (interrupt mitigation) setting changes
 */
static void
wlc_bmac_rcvlazy_update(wlc_hw_info_t *wlc_hw, uint32 intrcvlazy)
{
	W_REG(wlc_hw->osh, &wlc_hw->regs->intrcvlazy[0], intrcvlazy);

	/* interrupt for second fifo - 1 */
	if (PKT_CLASSIFY_EN(RX_FIFO1) || RXFIFO_SPLIT()) {
		W_REG(wlc_hw->osh, &wlc_hw->regs->intrcvlazy[RX_FIFO1], intrcvlazy);
	}
	/* interrupt for second fifo - 2 */
	if (PKT_CLASSIFY_EN(RX_FIFO2)) {
		W_REG(wlc_hw->osh, &wlc_hw->regs->intrcvlazy[RX_FIFO2], intrcvlazy);
	}
}
#endif /* !BCM_OL_DEV || BCMDBG || MACOSX */

/** set initial host flags value. Ucode interprets these host flags. */
static void
BCMINITFN(wlc_mhfdef)(wlc_hw_info_t *wlc_hw, uint16 *mhfs)
{
	bzero(mhfs, sizeof(uint16) * (MXHF0+MXHFMAX));

	if (WLCISGPHY(wlc_hw->band) && GREV_IS(wlc_hw->band->phyrev, 1))
		mhfs[MHF1] |= MHF1_DCFILTWAR;

	if (WLCISGPHY(wlc_hw->band) && (wlc_hw->boardflags & BFL_PACTRL) &&
		(wlc_hw->ucode_dbgsel != 0))
		mhfs[MHF1] |= MHF1_PACTL;

	/* enable CCK power boost in ucode but not for 4318/20 */
	if (WLCISGPHY(wlc_hw->band) && (wlc_hw->band->phyrev < 3)) {
		if (mhfs[MHF1] & MHF1_PACTL)
			WL_ERROR(("wl%d: Cannot support simultaneous MHF1_OFDMPWR & MHF1_CCKPWR\n",
				wlc_hw->unit));
		else
			mhfs[MHF1] |= MHF1_CCKPWR;
	} else {
		/* WAR for pin mux between ePA & SROM for 4331 12x9 package */
		bool is_4331_12x9 = FALSE;
		is_4331_12x9 = (CHIPID(wlc_hw->sih->chip) == BCM4331_CHIP_ID) ||
			(CHIPID(wlc_hw->sih->chip) == BCM43431_CHIP_ID);
		is_4331_12x9 &= ((wlc_hw->sih->chippkg == 9 || wlc_hw->sih->chippkg == 0xb));
		if (is_4331_12x9)
			mhfs[MHF1] |= MHF1_4331EPA_WAR;
	}

	/* prohibit use of slowclock on multifunction boards */
	if (wlc_hw->boardflags & BFL_NOPLLDOWN)
		mhfs[MHF1] |= MHF1_FORCEFASTCLK;

#ifdef BCM_DMA_CT
	/* For 4365B1, we enabled preloading with CTDMA but no MUTX.
	 * For 4365C0, we enabled preloading with CTDMA.
	 */
	if (BCM_DMA_CT_ENAB(wlc_hw->wlc)) {
		if ((D11REV_IS(wlc_hw->corerev, 64) && !MU_TX_ENAB(wlc_hw->wlc)) ||
			D11REV_IS(wlc_hw->corerev, 65)) {
			/* Only enable preloading with CTDMA but no MU TX */
			mhfs[MHF2] |= MHF2_PRELD_GE64;
		}
	}
#endif /* BCM_DMA_CT */

	if ((wlc_hw->band->radioid == BCM2050_ID) && (wlc_hw->band->radiorev < 6))
		mhfs[MHF2] |= MHF2_SYNTHPUWAR;

	if (WLCISNPHY(wlc_hw->band) && NREV_LT(wlc_hw->band->phyrev, 2)) {
		mhfs[MHF2] |= MHF2_NPHY40MHZ_WAR;
		mhfs[MHF1] |= MHF1_IQSWAP_WAR;
	}

#ifdef WLFCTS
	if (WLFCTS_ENAB(wlc_hw->wlc->pub)) {
		ASSERT(D11REV_GE(wlc_hw->corerev, 26));
		mhfs[MHF2] |= MHF2_TX_TMSTMP;
	}
#endif /* WLFCTS */

	/* set host flag to enable ucode for srom9: tx power offset based on txpwrctrl word */
	if (WLCISNPHY(wlc_hw->band) && (wlc_hw->sromrev >= 9)) {
		mhfs[MHF2] |= MHF2_PPR_HWPWRCTL;
	}

	if (CHIPID(wlc_hw->sih->chip) == BCM43526_CHIP_ID) {
		/* hostflag to tell the ucode that the interface is USB.
		ucode doesn't pull the HT request from the backplane.
		*/
		mhfs[MHF3] |= MHF3_USB_OLD_NPHYMLADVWAR;
	}
}

/**
 * set or clear ucode host flag bits
 * it has an optimization for no-change write
 * it only writes through shared memory when the core has clock;
 * pre-CLK changes should use wlc_write_mhf to get around the optimization
 *
 * bands values are: WLC_BAND_AUTO <--- Current band only
 *                   WLC_BAND_5G   <--- 5G band only
 *                   WLC_BAND_2G   <--- 2G band only
 *                   WLC_BAND_ALL  <--- All bands
 */
void
wlc_bmac_mhf(wlc_hw_info_t *wlc_hw, uint8 idx, uint16 mask, uint16 val, int bands)
{
	uint16 save;
	const uint16 addr[] = {M_HOST_FLAGS1, M_HOST_FLAGS2, M_HOST_FLAGS3,
		M_HOST_FLAGS4, M_HOST_FLAGS5};
#if defined(WL_PSMX)
	const uint16 addrx[] = {MX_HOST_FLAGS0};
#endif /* WL_PSMX */
	wlc_hwband_t *band;

	ASSERT((val & ~mask) == 0);
#if defined(WL_PSMX)
	ASSERT(idx < MXHF0+MXHFMAX);
	ASSERT(ARRAYSIZE(addrx) == MXHFMAX);
#else
	ASSERT(idx < MHFMAX);
#endif /* WL_PSMX */
	ASSERT(ARRAYSIZE(addr) == MHFMAX);

	switch (bands) {
		/* Current band only or all bands,
		 * then set the band to current band
		 */
	case WLC_BAND_AUTO:
	case WLC_BAND_ALL:
		band = wlc_hw->band;
		break;
	case WLC_BAND_5G:
		band = wlc_hw->bandstate[BAND_5G_INDEX];
		break;
	case WLC_BAND_2G:
		band = wlc_hw->bandstate[BAND_2G_INDEX];
		break;
	default:
		ASSERT(0);
		band = NULL;
	}

	if (band) {
		save = band->mhfs[idx];
		band->mhfs[idx] = (band->mhfs[idx] & ~mask) | val;

		/* optimization: only write through if changed, and
		 * changed band is the current band
		 */
		if (wlc_hw->clk && (band->mhfs[idx] != save) && (band == wlc_hw->band)) {
			if (idx < MXHF0)
				wlc_bmac_write_shm(wlc_hw, addr[idx], (uint16)band->mhfs[idx]);
#if defined(WL_PSMX)
			else if (D11REV_GE(wlc_hw->corerev, 64)) {
				wlc_bmac_write_shmx(wlc_hw, addrx[idx-MXHF0],
					(uint16)band->mhfs[idx]);
			}
#endif /* WL_PSMX */
		}
	}

	if (bands == WLC_BAND_ALL) {
		wlc_hw->bandstate[0]->mhfs[idx] = (wlc_hw->bandstate[0]->mhfs[idx] & ~mask) | val;
		wlc_hw->bandstate[1]->mhfs[idx] = (wlc_hw->bandstate[1]->mhfs[idx] & ~mask) | val;
	}
}

uint16
wlc_bmac_mhf_get(wlc_hw_info_t *wlc_hw, uint8 idx, int bands)
{
	wlc_hwband_t *band;
#if defined(WL_PSMX)
	ASSERT(idx < MXHF0+MXHFMAX);
#else
	ASSERT(idx < MHFMAX);
#endif /* WL_PSMX */

	switch (bands) {
	case WLC_BAND_AUTO:
		band = wlc_hw->band;
		break;
	case WLC_BAND_5G:
		band = wlc_hw->bandstate[BAND_5G_INDEX];
		break;
	case WLC_BAND_2G:
		band = wlc_hw->bandstate[BAND_2G_INDEX];
		break;
	default:
		ASSERT(0);
		band = NULL;
	}

	if (!band)
		return 0;

	return band->mhfs[idx];
}

static void
wlc_write_mhf(wlc_hw_info_t *wlc_hw, uint16 *mhfs)
{
	uint8 idx;
	const uint16 addr[] = {M_HOST_FLAGS1, M_HOST_FLAGS2, M_HOST_FLAGS3,
		M_HOST_FLAGS4, M_HOST_FLAGS5};
#if defined(WL_PSMX)
	const uint16 addrx[] = {MX_HOST_FLAGS0};
#endif /* WL_PSMX */

	ASSERT(ARRAYSIZE(addr) == MHFMAX);
#if defined(WL_PSMX)
	ASSERT(ARRAYSIZE(addrx) == MXHFMAX);
#endif /* WL_PSMX */

	for (idx = 0; idx < MHFMAX; idx++) {
		wlc_bmac_write_shm(wlc_hw, addr[idx], mhfs[idx]);
	}
#if defined(WL_PSMX)
	if (D11REV_GE(wlc_hw->corerev, 64)) {
		for (idx = 0; idx < MXHFMAX; idx++) {
			wlc_bmac_write_shmx(wlc_hw, addrx[idx], mhfs[MXHF0+idx]);
		}
	}
#endif /* WL_PSMX */
}

static void
wlc_bmac_ifsctl1_regshm(wlc_hw_info_t *wlc_hw, uint32 mask, uint32 val)
{
	osl_t *osh;
	d11regs_t *regs;
	uint32 w;
	volatile uint16 *ifsctl_reg;

	if (D11REV_GE(wlc_hw->corerev, 40))
		return;

	osh = wlc_hw->osh;
	regs = wlc_hw->regs;

	ifsctl_reg = (volatile uint16 *) &regs->u.d11regs.ifs_ctl1;

	w = (R_REG(osh, ifsctl_reg) & ~mask) | val;
	W_REG(osh, ifsctl_reg, w);

	wlc_bmac_write_shm(wlc_hw, M_IFSCTL1, (uint16)w);
}

#ifdef WL_PROXDETECT
/**
 * Proximity detection service - enables a way for mobile devices to pair based on relative
 * proximity.
 */
void wlc_enable_avb_timer(wlc_hw_info_t *wlc_hw, bool enable)
{
	osl_t *osh;
	d11regs_t *regs;

	osh = wlc_hw->osh;
	regs = wlc_hw->regs;

	if (enable) {
	OR_REG(osh, &regs->clk_ctl_st, CCS_AVBCLKREQ);
	OR_REG(osh, &regs->maccontrol1, MCTL1_AVB_ENABLE);
	}
	else {
		AND_REG(osh, &regs->clk_ctl_st, ~CCS_AVBCLKREQ);
		AND_REG(osh, &regs->maccontrol1, ~MCTL1_AVB_ENABLE);
	}

	/* enable/disable the avb timer */
	si_pmu_avb_clk_set(wlc_hw->sih, osh, enable);
}

void wlc_get_avb_timer_reg(wlc_hw_info_t *wlc_hw, uint32 *clkst, uint32 *maccontrol1)
{
	osl_t *osh;
	d11regs_t *regs;

	osh = wlc_hw->osh;
	regs = wlc_hw->regs;

	if (clkst)
		*clkst = R_REG(osh, &regs->clk_ctl_st);
	if (maccontrol1)
		*maccontrol1 = R_REG(osh, &regs->maccontrol1);
}

void wlc_get_avb_timestamp(wlc_hw_info_t *wlc_hw, uint32* ptx, uint32* prx)
{
	osl_t *osh;
	d11regs_t *regs;

	osh = wlc_hw->osh;
	regs = wlc_hw->regs;

	*ptx = R_REG(osh, &regs->avbtx_timestamp);
	*prx = R_REG(osh, &regs->avbrx_timestamp);
}
#endif /* WL_PROXDETECT */

/**
 * set the maccontrol register to desired reset state and
 * initialize the sw cache of the register
 */
void
wlc_mctrl_reset(wlc_hw_info_t *wlc_hw)
{
	/* IHR accesses are always enabled, PSM disabled, HPS off and WAKE on */
	wlc_hw->maccontrol = 0;
	wlc_hw->suspended_fifos = 0;
	wlc_hw->wake_override = 0;
	wlc_hw->mute_override = 0;
	wlc_bmac_mctrl(wlc_hw, ~0, MCTL_IHR_EN | MCTL_WAKE);
}

/** set or clear maccontrol bits */
void
wlc_bmac_mctrl(wlc_hw_info_t *wlc_hw, uint32 mask, uint32 val)
{
	uint32 maccontrol;
	uint32 new_maccontrol;

	ASSERT((val & ~mask) == 0);

	maccontrol = wlc_hw->maccontrol;
	new_maccontrol = (maccontrol & ~mask) | val;

	/* if the new maccontrol value is the same as the old, nothing to do */
	if (new_maccontrol == maccontrol)
		return;

	/* something changed, cache the new value */
	wlc_hw->maccontrol = new_maccontrol;

	/* write the new values with overrides applied */
	wlc_mctrl_write(wlc_hw);
}

#if defined(WL_PSMX)
static void
wlc_mctrlx_reset(wlc_hw_info_t *wlc_hw)
{
	ASSERT(D11REV_GE(wlc_hw->corerev, 64));

	/* IHR accesses are always enabled, PSM disabled, HPS off and WAKE on */
	wlc_hw->maccontrol_x = 0;
	wlc_bmac_mctrlx(wlc_hw, ~0, MCTL_IHR_EN);
}

static void
wlc_bmac_mctrlx(wlc_hw_info_t *wlc_hw, uint32 mask, uint32 val)
{
	uint32 maccontrol;
	uint32 new_maccontrol;

	ASSERT((val & ~mask) == 0);
	ASSERT((D11REV_GE(wlc_hw->corerev, 64)));

	maccontrol = wlc_hw->maccontrol_x;
	new_maccontrol = (maccontrol & ~mask) | val;

	/* if the new maccontrol value is the same as the old, nothing to do */
	if (new_maccontrol == maccontrol)
		return;

	/* something changed, cache the new value */
	wlc_hw->maccontrol_x = new_maccontrol;

	/* write the new values with overrides applied */
	W_REG(wlc_hw->osh, &wlc_hw->regs->maccontrol_x, new_maccontrol);
}
#endif /*  WL_PSMX */

/** write the software state of maccontrol and overrides to the maccontrol register */
static void
wlc_mctrl_write(wlc_hw_info_t *wlc_hw)
{
	uint32 maccontrol = wlc_hw->maccontrol;

	/* OR in the wake bit if overridden */
	if (wlc_hw->wake_override)
		maccontrol |= MCTL_WAKE;

	/* set AP and INFRA bits for mute if needed */
	if (wlc_hw->mute_override) {
		maccontrol &= ~(MCTL_AP);
		maccontrol |= MCTL_INFRA;
	}

	W_REG(wlc_hw->osh, &wlc_hw->regs->maccontrol, maccontrol);
}

void
wlc_ucode_wake_override_set(wlc_hw_info_t *wlc_hw, uint32 override_bit)
{
	ASSERT((wlc_hw->wake_override & override_bit) == 0);

	if (wlc_hw->wake_override || (wlc_hw->maccontrol & MCTL_WAKE)) {
		mboolset(wlc_hw->wake_override, override_bit);
		return;
	}

	mboolset(wlc_hw->wake_override, override_bit);

	wlc_mctrl_write(wlc_hw);
	wlc_bmac_wait_for_wake(wlc_hw);

	return;
}

void
wlc_ucode_wake_override_clear(wlc_hw_info_t *wlc_hw, uint32 override_bit)
{
	ASSERT(wlc_hw->wake_override & override_bit);

	mboolclr(wlc_hw->wake_override, override_bit);

	if (wlc_hw->wake_override || (wlc_hw->maccontrol & MCTL_WAKE))
		return;

	wlc_mctrl_write(wlc_hw);

	return;
}

/**
 * When driver needs ucode to stop beaconing, it has to make sure that
 * MCTL_AP is clear and MCTL_INFRA is set
 * Mode           MCTL_AP        MCTL_INFRA
 * AP                1              1
 * STA               0              1 <--- This will ensure no beacons
 * IBSS              0              0
 */
static void
wlc_ucode_mute_override_set(wlc_hw_info_t *wlc_hw)
{
	wlc_hw->mute_override = 1;

	/* if maccontrol already has AP == 0 and INFRA == 1 without this
	 * override, then there is no change to write
	 */
	if ((wlc_hw->maccontrol & (MCTL_AP | MCTL_INFRA)) == MCTL_INFRA)
		return;

	wlc_mctrl_write(wlc_hw);

	return;
}

/** Clear the override on AP and INFRA bits */
static void
wlc_ucode_mute_override_clear(wlc_hw_info_t *wlc_hw)
{
	if (wlc_hw->mute_override == 0)
		return;

	wlc_hw->mute_override = 0;

	/* if maccontrol already has AP == 0 and INFRA == 1 without this
	 * override, then there is no change to write
	 */
	if ((wlc_hw->maccontrol & (MCTL_AP | MCTL_INFRA)) == MCTL_INFRA)
		return;

	wlc_mctrl_write(wlc_hw);
}

/**
 * Updates suspended_fifos admin when suspending a txfifo
 * and may set the ucode wake override bit
 */
void
wlc_upd_suspended_fifos_set(wlc_hw_info_t *wlc_hw, uint txfifo)
{
	if (wlc_hw->suspended_fifos == 0) {
		wlc_ucode_wake_override_set(wlc_hw, WLC_WAKE_OVERRIDE_TXFIFO);
	}
	wlc_hw->suspended_fifos |= (1 << txfifo);
}

/**
 * Updates suspended_fifos admin when resuming a txfifo
 * and may clear the ucode wake override bit
 */
void
wlc_upd_suspended_fifos_clear(wlc_hw_info_t *wlc_hw, uint txfifo)
{
	if (!wlc_hw->suspended_fifos == 0) {
		wlc_hw->suspended_fifos &= ~(1 << txfifo);
		if (wlc_hw->suspended_fifos == 0) {
			wlc_ucode_wake_override_clear(wlc_hw, WLC_WAKE_OVERRIDE_TXFIFO);
		}
	}
}

#ifdef WLC_LOW_ONLY
/* Low Level API functions for reg read/write called by a high level driver */
uint32
wlc_reg_read(wlc_info_t *wlc, void *r, uint size)
{
	wlc_hw_info_t *wlc_hw = wlc->hw;
	void* addr = (int8*)wlc_hw->regs + ((int8*)r - (int8*)0);
	uint32 v;

	if (size == 1)
		v = R_REG(wlc_hw->osh, (uint8*)addr);
	else if (size == 2)
		v = R_REG(wlc_hw->osh, (uint16*)addr);
	else
		v = R_REG(wlc_hw->osh, (uint32*)addr);

	return v;
}

void
wlc_reg_write(wlc_info_t *wlc, void *r, uint32 v, uint size)
{
	wlc_hw_info_t *wlc_hw = wlc->hw;
	void* addr = (int8*)wlc_hw->regs + ((int8*)r - (int8*)0);

	if (size == 1)
		W_REG(wlc_hw->osh, (uint8*)addr, v);
	else if (size == 2)
		W_REG(wlc_hw->osh, (uint16*)addr, v);
	else
		W_REG(wlc_hw->osh, (uint32*)addr, v);
}
#endif /* WLC_LOW_ONLY */

/**
 * Add a MAC address to the rcmta memory in the D11 core. This is an associative memory used to
 * quickly compare a received address with a preloaded set of addresses.
 */
void
wlc_bmac_set_rcmta(wlc_hw_info_t *wlc_hw, int idx, const struct ether_addr *addr)
{
	WL_TRACE(("wl%d: %s\n", wlc_hw->unit, __FUNCTION__));

	ASSERT(idx >= 0);	/* This routine only for non primary interfaces */

	wlc_bmac_copyto_objmem(wlc_hw, (idx * 2) << 2, addr->octet,
		ETHER_ADDR_LEN, OBJADDR_RCMTA_SEL);
}

void
wlc_bmac_get_rcmta(wlc_hw_info_t *wlc_hw, int idx, struct ether_addr *addr)
{
	ASSERT(idx >= 0);	/* This routine only for non primary interfaces */

	wlc_bmac_copyfrom_objmem(wlc_hw, (idx * 2) << 2, addr->octet,
		ETHER_ADDR_LEN, OBJADDR_RCMTA_SEL);
}

/** for d11 rev >= 40, RCMTA was replaced with AMT (Address Match Table) */
void
wlc_bmac_read_amt(wlc_hw_info_t *wlc_hw, int idx, struct ether_addr *addr, uint16 *attr)
{
	uint32 word[2];

	WL_TRACE(("wl%d: %s: idx %d\n", wlc_hw->unit, __FUNCTION__, idx));
	ASSERT(wlc_hw->corerev >= 40);

	wlc_bmac_copyfrom_objmem(wlc_hw, (idx * 2) << 2, word,
		sizeof(word), OBJADDR_AMT_SEL);

	addr->octet[0] = (uint8)word[0];
	addr->octet[1] = (uint8)(word[0] >> 8);
	addr->octet[2] = (uint8)(word[0] >> 16);
	addr->octet[3] = (uint8)(word[0] >> 24);
	addr->octet[4] = (uint8)word[1];
	addr->octet[5] = (uint8)(word[1] >> 8);
	*attr = (word[1] >> 16);
}

void
wlc_bmac_amt_dump(wlc_hw_info_t *wlc_hw)
{
	struct ether_addr addr;
	uint16 attr = 0;

	wlc_bmac_read_amt(wlc_hw, AMT_IDX_MAC, &addr, &attr);
	WL_ERROR(("%s: mac etheraddr %02x:%02x:%02x:%02x:%02x:%02x  attr %x\n", __FUNCTION__,
		addr.octet[0], addr.octet[1],
		addr.octet[2], addr.octet[3],
		addr.octet[4], addr.octet[5], attr));

	attr = 0;
	wlc_bmac_read_amt(wlc_hw, AMT_IDX_BSSID, &addr, &attr);
	WL_ERROR(("%s: bssid etheraddr %02x:%02x:%02x:%02x:%02x:%02x  attr %x\n", __FUNCTION__,
		addr.octet[0], addr.octet[1], addr.octet[2], addr.octet[3],
		addr.octet[4], addr.octet[5], attr));
}

/** Write a MAC address to the AMT (Address Match Table) */
uint16
wlc_bmac_write_amt(wlc_hw_info_t *wlc_hw, int idx, const struct ether_addr *addr, uint16 attr)
{
	uint32 word[2];
	struct ether_addr prev_addr;
	uint16 prev_attr  = 0;

	WL_TRACE(("wl%d: %s: idx %d\n", wlc_hw->unit, __FUNCTION__, idx));
	ASSERT(wlc_hw->corerev >= 40);

	/* Read/Modify/Write unless entry is being disabled */
	wlc_bmac_read_amt(wlc_hw, idx, &prev_addr, &prev_attr);
	if (attr & AMT_ATTR_VALID) {
		attr |= prev_attr;
	}
	else {
		attr = 0;
	}

	word[0] = (addr->octet[3] << 24) |
	        (addr->octet[2] << 16) |
	        (addr->octet[1] << 8) |
	        addr->octet[0];
	word[1] = (attr << 16) |
	        (addr->octet[5] << 8) |
	        addr->octet[4];

	wlc_bmac_copyto_objmem(wlc_hw, (idx * 2) << 2, word, sizeof(word), OBJADDR_AMT_SEL);

	return prev_attr;
}

#if defined(BCMDBG)
/**
 * Read from HW Aux PMQ according to the index of bmac_auxpmq_entry_t.
 * Ecah entry is stored in Aux PMQ memory.
 * addr is the mac address of the mapping STA.
 * data is the PMQ data for the mapping STA.
 */
void
wlc_bmac_read_auxpmq(wlc_hw_info_t *wlc_hw, int idx, struct ether_addr *addr, uint16 *data)
{
	uint32 word[2];

	WL_TRACE(("wl%d: %s: idx %d\n", wlc_hw->unit, __FUNCTION__, idx));
	ASSERT(wlc_hw->corerev >= 64);

	/* ObjAddr[6:0] of objAddr refer to the Index*2 of the AuxPMQ
	 * ObjAddr[23:16] = 7, it will select for AuxPmq.
	 * 2 Consecutive reads of ObjData will be 64 bit entry of the AuxPMQ.
	 */
	wlc_bmac_copyfrom_objmem(wlc_hw, (idx * 2) << 2, word,
		sizeof(word), OBJADDR_AUXPMQ_SEL);

	addr->octet[0] = (uint8)word[0];
	addr->octet[1] = (uint8)(word[0] >> 8);
	addr->octet[2] = (uint8)(word[0] >> 16);
	addr->octet[3] = (uint8)(word[0] >> 24);
	addr->octet[4] = (uint8)word[1];
	addr->octet[5] = (uint8)(word[1] >> 8);
	*data = (word[1] >> 16);
}
#endif /* BCMDBG */

/**
 * Write data to HW Aux PMQ. AuxPMQ (APMQ) (width = 64 bits, 48 addr bits + 16 attribute bits)
 * Data is composed of 6 bytes Mac address + 2 bytes PMQ data.
 * idx is mapping from 0 to 63. Each entry is 8 bytes.
 */
void
wlc_bmac_write_auxpmq(wlc_hw_info_t *wlc_hw, int idx, const struct ether_addr *addr, uint16 data)
{
	uint32 word[2];

	WL_TRACE(("wl%d: %s: idx %d\n", wlc_hw->unit, __FUNCTION__, idx));
	ASSERT(wlc_hw->corerev >= 64);

	/* lower 4 bytes of Mac address */
	word[0] = (addr->octet[3] << 24) |
	        (addr->octet[2] << 16) |
	        (addr->octet[1] << 8) |
	        addr->octet[0];
	/* higher 2 bytes of Mac address and 2 bytes PMQ data */
	word[1] = (data << 16) |
	        (addr->octet[5] << 8) |
	        addr->octet[4];

	/* ObjAddr[6:0] of objAddr refer to the Index*2 of the AuxPMQ
	 * ObjAddr[23:16] = 7, it will select for AuxPmq.
	 * 2 Consecutive writes of ObjData will be 64 bit entry of the AuxPMQ.
	 */
	wlc_bmac_copyto_objmem(wlc_hw, (idx * 2) << 2, word, sizeof(word), OBJADDR_AUXPMQ_SEL);
}

/** Write a MAC address to the given match reg offset in the RXE match engine. */
void
wlc_bmac_set_rxe_addrmatch(wlc_hw_info_t *wlc_hw, int match_reg_offset,
	const struct ether_addr *addr)
{
	d11regs_t *regs;
	uint16 mac_l;
	uint16 mac_m;
	uint16 mac_h;
	osl_t *osh;

	WL_TRACE(("wl%d: %s: offset %d\n", wlc_hw->unit, __FUNCTION__,
	          match_reg_offset));

	ASSERT(wlc_hw->corerev < 40);
	ASSERT((match_reg_offset < RCM_SIZE) || (wlc_hw->corerev == 4));

	/* RCM addrmatch is replaced by AMT in d11 rev40 */
	if (D11REV_GE(wlc_hw->corerev, 40)) {
		WL_ERROR(("wl%d: %s: RCM addrmatch not available on corerev >= 40\n",
		          wlc_hw->unit, __FUNCTION__));
		return;
	}

	regs = wlc_hw->regs;
	mac_l = addr->octet[0] | (addr->octet[1] << 8);
	mac_m = addr->octet[2] | (addr->octet[3] << 8);
	mac_h = addr->octet[4] | (addr->octet[5] << 8);

	osh = wlc_hw->osh;

	/* enter the MAC addr into the RXE match registers */
	W_REG(osh, &regs->u_rcv.d11regs.rcm_ctl, RCM_INC_DATA | match_reg_offset);
	W_REG(osh, &regs->u_rcv.d11regs.rcm_mat_data, mac_l);
	W_REG(osh, &regs->u_rcv.d11regs.rcm_mat_data, mac_m);
	W_REG(osh, &regs->u_rcv.d11regs.rcm_mat_data, mac_h);

}

static void
wlc_bmac_set_match_mac(wlc_hw_info_t *wlc_hw, const struct ether_addr *addr)
{
	if (D11REV_LT(wlc_hw->corerev, 40)) {
		wlc_bmac_set_rxe_addrmatch(wlc_hw, RCM_MAC_OFFSET, addr);
	} else {
		wlc_bmac_write_amt(wlc_hw, AMT_IDX_MAC, addr, (AMT_ATTR_VALID | AMT_ATTR_A1));
	}
}

static void
wlc_bmac_clear_match_mac(wlc_hw_info_t *wlc_hw)
{
	if (D11REV_LT(wlc_hw->corerev, 40)) {
		wlc_bmac_set_rxe_addrmatch(wlc_hw, RCM_MAC_OFFSET, &ether_null);
	} else {
		wlc_bmac_write_amt(wlc_hw, AMT_IDX_MAC, &ether_null, 0);
	}
}

#ifndef BCM_OL_DEV
static void
wlc_bmac_reset_amt(wlc_hw_info_t *wlc_hw)
{
	int i;

	for (i = 0; i < (int)wlc_hw->wlc->pub->max_addrma_idx; i++)
		wlc_bmac_write_amt(wlc_hw, i, &ether_null, 0);
}

static void
wlc_bmac_reset_auxpmq(wlc_hw_info_t *wlc_hw)
{
	int auxpmq_len = AUXPMQ_ENTRIES * AUXPMQ_ENTRY_SIZE;
	wlc_bmac_set_objmem32(wlc_hw, 0, 0, auxpmq_len, OBJADDR_AUXPMQ_SEL);
}
#endif /* BCM_OL_DEV */

/**
 * Template memory is located in the d11 core, and is used by ucode to transmit frames based on a
 * preloaded template.
 */
void
wlc_bmac_templateptr_wreg(wlc_hw_info_t *wlc_hw, int offset)
{
	ASSERT(ISALIGNED(offset, sizeof(uint32)));

	/* Correct the template read pointer according to mac core using templatebase */
	offset = offset + wlc_hw->templatebase;
	W_REG(wlc_hw->osh, &wlc_hw->regs->tplatewrptr, offset);
}

uint32
wlc_bmac_templateptr_rreg(wlc_hw_info_t *wlc_hw)
{
	return R_REG(wlc_hw->osh, &wlc_hw->regs->tplatewrptr);
}

void
wlc_bmac_templatedata_wreg(wlc_hw_info_t *wlc_hw, uint32 word)
{
	W_REG(wlc_hw->osh, &wlc_hw->regs->tplatewrdata, word);
}

uint32
wlc_bmac_templatedata_rreg(wlc_hw_info_t *wlc_hw)
{
	return R_REG(wlc_hw->osh, &wlc_hw->regs->tplatewrdata);
}

void
wlc_bmac_write_template_ram(wlc_hw_info_t *wlc_hw, int offset, int len, void *buf)
{
	d11regs_t *regs = wlc_hw->regs;
	uint32 word;
	bool be_bit;
#ifdef IL_BIGENDIAN
	volatile uint16 *dptr = NULL;
#endif /* IL_BIGENDIAN */
	osl_t *osh = wlc_hw->osh;

	WL_TRACE(("wl%d: wlc_bmac_write_template_ram\n", wlc_hw->unit));

	ASSERT(ISALIGNED(offset, sizeof(uint32)));
	ASSERT(ISALIGNED(len, sizeof(uint32)));
	ASSERT((offset & ~0xffff) == 0);

	/* The template region starts where the BMC_STARTADDR starts.
	 * This shouldn't use a #defined value but some parameter in a
	 * global struct.
	 */
	if (D11REV_IS(wlc_hw->corerev, 48) || D11REV_IS(wlc_hw->corerev, 49))
		offset += (D11MAC_BMC_STARTADDR_SRASM << 8);
	wlc_bmac_templateptr_wreg(wlc_hw, offset);

#ifdef IL_BIGENDIAN
	if (BUSTYPE(wlc_hw->sih->bustype) == PCMCIA_BUS)
		dptr = (volatile uint16*)&regs->tplatewrdata;
#endif /* IL_BIGENDIAN */

	/* if MCTL_BIGEND bit set in mac control register,
	 * the chip swaps data in fifo, as well as data in
	 * template ram
	 */
	be_bit = (R_REG(osh, &regs->maccontrol) & MCTL_BIGEND) != 0;

	while (len > 0) {
		bcopy((uint8*)buf, &word, sizeof(uint32));

		if (be_bit)
			word = hton32(word);
		else
			word = htol32(word);

#ifdef IL_BIGENDIAN
		if (BUSTYPE(wlc_hw->sih->bustype) == PCMCIA_BUS &&
		    D11REV_IS(wlc_hw->corerev, 4)) {
			W_REG(osh, (dptr + 1), (uint16)((word >> NBITS(uint16)) & 0xffff));
			W_REG(osh, dptr, (uint16)(word & 0xffff));
		} else
#endif /* IL_BIGENDIAN */
			wlc_bmac_templatedata_wreg(wlc_hw, word);

		buf = (uint8*)buf + sizeof(uint32);
		len -= sizeof(uint32);
	}
}

/** contention window related */
void
wlc_bmac_set_cwmin(wlc_hw_info_t *wlc_hw, uint16 newmin)
{
	wlc_hw->band->CWmin = newmin;

	wlc_bmac_copyto_objmem(wlc_hw, S_DOT11_CWMIN << 2, &newmin,
		sizeof(newmin), OBJADDR_SCR_SEL);
}

void
wlc_bmac_set_cwmax(wlc_hw_info_t *wlc_hw, uint16 newmax)
{
	wlc_hw->band->CWmax = newmax;

	wlc_bmac_copyto_objmem(wlc_hw, S_DOT11_CWMAX << 2, &newmax,
		sizeof(newmax), OBJADDR_SCR_SEL);
}

void
wlc_bmac_bw_set(wlc_hw_info_t *wlc_hw, uint16 bw)
{
	uint32 tmp;
	phy_info_t *pi = (phy_info_t *)wlc_hw->band->pi;
	chanspec_t chspec = phy_utils_get_chanspec(pi);

	phy_utils_set_bwstate(pi, bw);

	ASSERT(wlc_hw->clk);
#ifndef EFI
	PANIC_CHECK_CLK(wlc_hw->clk, "wl%d: %s: NO CLK\n", wlc_hw->unit, __FUNCTION__);
#endif // endif

	if (D11REV_LT(wlc_hw->corerev, 17)) {
		tmp = R_REG(wlc_hw->osh, &wlc_hw->regs->maccontrol);
		BCM_REFERENCE(tmp);
	}
	if (WLCISACPHY(wlc_hw->band) && (BW_RESET == 1))
	  wlc_bmac_bw_reset(wlc_hw);
	else
	  wlc_bmac_phy_reset(wlc_hw);

	/* No need to issue init for acphy on bw change */
	phy_bwinit(pi, chspec);

	/* restore the clk */
}

static void
wlc_write_hw_bcntemplate0(wlc_hw_info_t *wlc_hw, void *bcn, int len)
{
	d11regs_t *regs = wlc_hw->regs;
	uint shm_bcn_tpl0_base;

	if (D11REV_GE(wlc_hw->corerev, 40))
		shm_bcn_tpl0_base = D11AC_T_BCN0_TPL_BASE;
	else
		shm_bcn_tpl0_base = D11_T_BCN0_TPL_BASE;

	wlc_bmac_write_template_ram(wlc_hw, shm_bcn_tpl0_base, (len + 3) & ~3, bcn);
	/* write beacon length to SCR */
	ASSERT(len < 65536);
	wlc_bmac_write_shm(wlc_hw, M_BCN0_FRM_BYTESZ, (uint16)len);
	/* mark beacon0 valid */
	OR_REG(wlc_hw->osh, &regs->maccommand, MCMD_BCN0VLD);
}

static void
wlc_write_hw_bcntemplate1(wlc_hw_info_t *wlc_hw, void *bcn, int len)
{
	d11regs_t *regs = wlc_hw->regs;
	uint shm_bcn_tpl1_base;

	if (D11REV_GE(wlc_hw->corerev, 40))
		shm_bcn_tpl1_base = D11AC_T_BCN1_TPL_BASE;
	else
		shm_bcn_tpl1_base = D11_T_BCN1_TPL_BASE;

	wlc_bmac_write_template_ram(wlc_hw, shm_bcn_tpl1_base, (len + 3) & ~3, bcn);
	/* write beacon length to SCR */
	ASSERT(len < 65536);
	wlc_bmac_write_shm(wlc_hw, M_BCN1_FRM_BYTESZ, (uint16)len);
	/* mark beacon1 valid */
	OR_REG(wlc_hw->osh, &regs->maccommand, MCMD_BCN1VLD);
}

/** mac is assumed to be suspended at this point */
void
wlc_bmac_write_hw_bcntemplates(wlc_hw_info_t *wlc_hw, void *bcn, int len, bool both)
{
	d11regs_t *regs = wlc_hw->regs;

	if (both) {
		wlc_write_hw_bcntemplate0(wlc_hw, bcn, len);
		wlc_write_hw_bcntemplate1(wlc_hw, bcn, len);
	} else {
		/* bcn 0 */
		if (!(R_REG(wlc_hw->osh, &regs->maccommand) & MCMD_BCN0VLD))
			wlc_write_hw_bcntemplate0(wlc_hw, bcn, len);
		/* bcn 1 */
		else if (!(R_REG(wlc_hw->osh, &regs->maccommand) & MCMD_BCN1VLD))
			wlc_write_hw_bcntemplate1(wlc_hw, bcn, len);
		else	/* one template should always have been available */
			ASSERT(0);
	}
}

/** returns the time it takes to power up the synthesizer */
static uint16
WLBANDINITFN(wlc_bmac_synthpu_dly)(wlc_hw_info_t *wlc_hw)
{
	uint16 v;

	/* return SYNTHPU_DLY */

	/* BMAC_NOTE: changing this code to always return phy specific synthpu delays
	 * instead of checking STA/AP mode and setting a constant 500us delay.
	 * need to check if values should be conditionalize differently, maybe by
	 * radio type, not phy. Original code from wlc_bsinit().
	 * Filed PR53142 to see if STA/AP and infra checks are needed.
	 */

	if (ISSIM_ENAB(wlc_hw->sih)) {
		v = SYNTHPU_DLY_PHY_US_QT;
	} else {

		/* for LPPHY synthpu delay is very small as PMU handles xtal/pll */
		if (WLCISLPPHY(wlc_hw->band)) {
			v = LPREV_GE(wlc_hw->band->phyrev, 2) ?
				SYNTHPU_DLY_LPPHY_US : SYNTHPU_DLY_BPHY_US;
		} else if (WLCISSSLPNPHY(wlc_hw->band)) {
			v = SYNTHPU_DLY_SSLPNPHY_US;
		} else if (WLCISLCNPHY(wlc_hw->band)) {
			v = SYNTHPU_DLY_LPPHY_US;
		} else if (WLCISAPHY(wlc_hw->band)) {
			v = SYNTHPU_DLY_APHY_US;
		} else if (WLCISNPHY(wlc_hw->band)) {
			v = NREV_GE(wlc_hw->band->phyrev, 3) ?
				SYNTHPU_DLY_NPHY_US : SYNTHPU_DLY_BPHY_US;
		} else if (WLCISHTPHY(wlc_hw->band)) {
			v = SYNTHPU_DLY_HTPHY_US;
		} else if (WLCISLCNPHY(wlc_hw->band)) {
			v = SYNTHPU_DLY_LCNPHY_US;
			if (CHIPID(wlc_hw->sih->chip) == BCM4336_CHIP_ID)
				v = SYNTHPU_DLY_LCNPHY_4336_US;
		} else if (WLCISLCN40PHY(wlc_hw->band)) {
			v = SYNTHPU_DLY_LCN40PHY_US;
		} else if (WLCISACPHY(wlc_hw->band)) {
			if (BCM4350_CHIP(wlc_hw->sih->chip))
				v = SYNTHPU_DLY_ACPHY2_US;
			else if ((CHIPID(wlc_hw->sih->chip) == BCM4365_CHIP_ID) ||
				(CHIPID(wlc_hw->sih->chip) == BCM4366_CHIP_ID))
				v = SYNTHPU_DLY_ACPHY_4365_US;
			else if (CHIPID(wlc_hw->sih->chip) == BCM4335_CHIP_ID &&
				(CHIPREV(wlc_hw->sih->chiprev) < 2))
				v = SYNTHPU_DLY_ACPHY_4335_US;
			else if (CHIPID(wlc_hw->sih->chip) == BCM4335_CHIP_ID &&
				(CHIPREV(wlc_hw->sih->chiprev) >= 2))
				v = SYNTHPU_DLY_ACPHY_4339_US;

			else if (BCM4349_CHIP(wlc_hw->sih->chip))
				v = SYNTHPU_DLY_ACPHY_4349_US;

			else
				v = SYNTHPU_DLY_ACPHY_US;
		} else {
			v = SYNTHPU_DLY_BPHY_US;
		}
	}

	if ((wlc_hw->band->radioid == BCM2050_ID) && (wlc_hw->band->radiorev == 8)) {
		if (v < 2400)
			v = 2400;
	}

	return v;
}

static void
WLBANDINITFN(wlc_bmac_upd_synthpu)(wlc_hw_info_t *wlc_hw)
{
	uint16 v = wlc_bmac_synthpu_dly(wlc_hw);
	wlc_bmac_write_shm(wlc_hw, M_SYNTHPU_DLY, v);
}

void
wlc_bmac_set_extlna_pwrsave_shmem(wlc_hw_info_t *wlc_hw)
{
	uint16 extlna_pwrctl = 0x480;

	if ((CHIPID(wlc_hw->sih->chip) == BCM4331_CHIP_ID) &&
	    ((wlc_hw->sih->boardtype == BCM94331X29B) ||
	     ((wlc_hw->boardflags2 & BFL2_EXTLNA_PWRSAVE) &&
	      (wlc_hw->antswctl2g >= 3 && wlc_hw->antswctl5g >= 3)))) {
		extlna_pwrctl = 0x4c0;
	}
	wlc_bmac_write_shm(wlc_hw, M_EXTLNA_PWRSAVE, extlna_pwrctl);
}

/** band-specific init */
static void
WLBANDINITFN(wlc_bmac_bsinit)(wlc_hw_info_t *wlc_hw, chanspec_t chanspec, bool chanswitch_path)
{
	wlc_info_t *wlc = wlc_hw->wlc;

	(void)wlc;

	WL_TRACE(("wl%d: wlc_bmac_bsinit: bandunit %d\n", wlc_hw->unit, wlc_hw->band->bandunit));
	/* we need to do this before phy_init.  5G PA shares the same pin as SECI */
	if (((CHIPID(wlc_hw->sih->chip) == BCM4331_CHIP_ID) ||
	     (CHIPID(wlc_hw->sih->chip) == BCM43431_CHIP_ID)) &&
	    (wlc_hw->sih->boardtype != BCM94331X19)) {
		si_seci_upd(wlc_hw->sih, CHSPEC_IS2G(chanspec));
	}
	/* sanity check */
	if (PHY_TYPE(R_REG(wlc_hw->osh, &wlc_hw->regs->phyversion)) != PHY_TYPE_LCNXN)
		ASSERT((uint)PHY_TYPE(R_REG(wlc_hw->osh, &wlc_hw->regs->phyversion)) ==
		       wlc_hw->band->phytype);

	wlc_ucode_bsinit(wlc_hw);

	/* phymode switch requires phyinit */
	phy_bsinit((phy_info_t *)wlc_hw->band->pi, chanspec,
		(!chanswitch_path) || phy_init_pending((phy_info_t *)wlc_hw->band->pi));

#if defined(WLC_HIGH) && defined(BCMNODOWN)
	/* Radio is active after phy_init() */
	wlc->pub->radio_active = ON;
#endif // endif

	wlc_ucode_txant_set(wlc_hw);

	/* cwmin is band-specific, update hardware with value for current band */
	wlc_bmac_set_cwmin(wlc_hw, wlc_hw->band->CWmin);
	wlc_bmac_set_cwmax(wlc_hw, wlc_hw->band->CWmax);

	wlc_bmac_update_slot_timing(wlc_hw,
		BAND_5G(wlc_hw->band->bandtype) ? TRUE : wlc_hw->shortslot);

	/* write phytype and phyvers */
	wlc_bmac_write_shm(wlc_hw, M_PHYTYPE, (uint16)wlc_hw->band->phytype);
	wlc_bmac_write_shm(wlc_hw, M_PHYVER, (uint16)wlc_hw->band->phyrev);

#ifdef WL11N
	/* initialize the txphyctl1 rate table since shmem is shared between bands */
	wlc_upd_ofdm_pctl1_table(wlc_hw);
#endif // endif

	if (D11REV_IS(wlc_hw->corerev, 4) && WLCISAPHY(wlc_hw->band))
		wlc_bmac_write_shm(wlc_hw, M_SEC_DEFIVLOC, 0x1d);

	wlc_bmac_upd_synthpu(wlc_hw);

	/* Configure BTC GPIOs as bands change */
	if (BAND_5G(wlc_hw->band->bandtype))
		wlc_bmac_mhf(wlc_hw, MHF5, MHF5_BTCX_DEFANT, MHF5_BTCX_DEFANT, WLC_BAND_ALL);
	else
		wlc_bmac_mhf(wlc_hw, MHF5, MHF5_BTCX_DEFANT, 0, WLC_BAND_ALL);
	wlc_bmac_btc_gpio_enable(wlc_hw);

#if defined(WLRSDB) && defined(BCMECICOEX) && defined(WLC_LOW)
	/* Update coex_io_mask on band switch for corerev 50 only */
	if (wlc_bmac_rsdb_cap(wlc_hw)) {
		wlc_cmn_info_t* wlc_cmn = wlc->cmn;
		wlc_info_t *wlc_iter;
		int idx;
		/* update coex_io_mask for all the cores */
		FOREACH_WLC(wlc_cmn, idx, wlc_iter) {
			wlc_btcx_update_coex_iomask(wlc_iter);
#ifdef BCMLTECOEX
			if (BCMLTECOEX_ENAB(wlc->pub))	{
				wlc_ltecx_update_coex_iomask(wlc_iter->ltecx);
			}
#endif /* BCMLTECOEX */
		}
	}
#endif /* WLRSDB && BCMECICOEX && WLC_LOW */

#ifdef WL11ULB
	if (ULB_ENAB(wlc->pub))
		wlc_bmac_set_ulbparams(wlc_hw, chanspec);
#endif /* WL11ULB */

	if (BAND_5G(wlc_hw->band->bandtype))
		wlc_bmac_config_4331_5GePA(wlc_hw);

	wlc_bmac_set_extlna_pwrsave_shmem(wlc_hw);
}

/* Helper API to apply the phy reset on d11 core unit one for RSDB chip.
 * This is required while there is no core one init happens for cases like MIMO,
 * 80p80 mode.
 */
static void
wlc_bmac_4349_btcx_prisel_war(wlc_hw_info_t *wlc_hw)
{
	int sicoreunit = 0;

	ASSERT(wlc_hw != NULL);
	sicoreunit = si_coreunit(wlc_hw->sih);

	/* Apply the setting to D11 core unit one always */
	if ((wlc_rsdb_mode(wlc_hw->wlc) == PHYMODE_MIMO) ||
	(wlc_rsdb_mode(wlc_hw->wlc) == PHYMODE_80P80)) {
		WL_INFORM(("MIMO: PRISEL ISSUE WORKAROUND\n"));
		si_d11_switch_addrbase(wlc_hw->sih, 1);
		si_core_cflags(wlc_hw->sih, SICF_PRST, 0x7);
	}

	si_d11_switch_addrbase(wlc_hw->sih, sicoreunit);
}

void
wlc_bmac_core_phy_clk(wlc_hw_info_t *wlc_hw, bool clk)
{
	WL_TRACE(("wl%d: wlc_bmac_core_phy_clk: clk %d\n", wlc_hw->unit, clk));

	wlc_hw->phyclk = clk;

	if (OFF == clk) {
		/* CLEAR GMODE BIT, PUT PHY INTO RESET */

		si_core_cflags(wlc_hw->sih,
			(SICF_PRST | SICF_FGC | SICF_GMODE),
			(SICF_PRST | SICF_FGC));
		OSL_DELAY(1);

		si_core_cflags(wlc_hw->sih, (SICF_PRST | SICF_FGC), SICF_PRST);
		OSL_DELAY(1);
	} else {
		/* TAKE PHY OUT OF RESET */

		/* High Speed DAC Configuration */
		if (D11REV_GE(wlc_hw->corerev, 40)) {
			si_core_cflags(wlc_hw->sih, SICF_DAC, 0x100);
		}

		/* Special PHY RESET Sequence for ACPHY to ensure correct Clock Alignment */
		if (WLCISACPHY(wlc_hw->band)) {
			/* turn off phy clocks and bring out of reset */
			si_core_cflags(wlc_hw->sih, (SICF_PRST | SICF_FGC | SICF_PCLKE), 0);
			OSL_DELAY(1);

			/*
			 * CRWLDOT11M-1403: Enabling Core 1 PHY reset bit
			 * in Core 1 ioctrl for dot11macphy_prisel to PHY
			 */
			if (wlc_bmac_rsdb_cap(wlc_hw)) {
				if (!RSDB_ENAB(wlc_hw->wlc->pub) &&
				(wlc_hw->core1_mimo_reset == 0)) {
					wlc_bmac_4349_btcx_prisel_war(wlc_hw);
					wlc_hw->core1_mimo_reset = 1;
				}
			}

			/* reenable phy clocks to resync to mac mac clock */
			si_core_cflags(wlc_hw->sih, SICF_PCLKE, SICF_PCLKE);
			OSL_DELAY(1);
		} else {
			/* turn off phy clocks */
			si_core_cflags(wlc_hw->sih,
				(SICF_PRST | SICF_FGC | SICF_PCLKE),
				SICF_FGC);
			OSL_DELAY(1);

			/* reenable phy clocks to resync to mac mac clock */
			si_core_cflags(wlc_hw->sih, (SICF_FGC | SICF_PCLKE), SICF_PCLKE);
			OSL_DELAY(1);
		}
	}
}

/** Perform a soft reset of the PHY PLL */
void
wlc_bmac_core_phypll_reset(wlc_hw_info_t *wlc_hw)
{
	WL_TRACE(("wl%d: wlc_bmac_core_phypll_reset\n", wlc_hw->unit));

	if (WLCISNPHY(wlc_hw->band) || WLCISHTPHY(wlc_hw->band)) {

		si_corereg(wlc_hw->sih, SI_CC_IDX, OFFSETOF(chipcregs_t, chipcontrol_addr), ~0, 0);
		OSL_DELAY(1);
		si_corereg(wlc_hw->sih, SI_CC_IDX, OFFSETOF(chipcregs_t, chipcontrol_data), 0x4, 0);
		OSL_DELAY(1);
		si_corereg(wlc_hw->sih, SI_CC_IDX, OFFSETOF(chipcregs_t, chipcontrol_data), 0x4, 4);
		OSL_DELAY(1);
		si_corereg(wlc_hw->sih, SI_CC_IDX, OFFSETOF(chipcregs_t, chipcontrol_data), 0x4, 0);
		OSL_DELAY(1);
	}
}

/**
 * light way to turn on phy clock without reset for NPHY and HTPHY only
 *  refer to wlc_bmac_core_phy_clk for full version
 */
void
wlc_bmac_phyclk_fgc(wlc_hw_info_t *wlc_hw, bool clk)
{
	/* support(necessary for NPHY and HTPHY) only */
	if (!WLCISNPHY(wlc_hw->band) && !WLCISHTPHY(wlc_hw->band) && !WLCISACPHY(wlc_hw->band))
		return;

	if (ON == clk)
		si_core_cflags(wlc_hw->sih, SICF_FGC, SICF_FGC);
	else
		si_core_cflags(wlc_hw->sih, SICF_FGC, 0);

}

void
wlc_bmac_macphyclk_set(wlc_hw_info_t *wlc_hw, bool clk)
{
	if (ON == clk)
		si_core_cflags(wlc_hw->sih, SICF_MPCLKE, SICF_MPCLKE);
	else
		si_core_cflags(wlc_hw->sih, SICF_MPCLKE, 0);
}

static uint32
wlc_bmac_clk_bwbits(wlc_hw_info_t *wlc_hw)
{
	uint32 phy_bw_clkbits = 0;

	/* select the phy speed according to selected channel b/w */
	switch (CHSPEC_BW(wlc_hw->chanspec)) {
#ifdef WL11ULB
	case WL_CHANSPEC_BW_2P5:
	case WL_CHANSPEC_BW_5:
	case WL_CHANSPEC_BW_10:
#else /* WL11ULB */
	case WL_CHANSPEC_BW_10:
		phy_bw_clkbits = SICF_BW10;
		break;
#endif /* WL11ULB */
	case WL_CHANSPEC_BW_20:
		phy_bw_clkbits = SICF_BW20;
		break;
	case WL_CHANSPEC_BW_40:
		phy_bw_clkbits = SICF_BW40;
		break;
	case WL_CHANSPEC_BW_80:
	case WL_CHANSPEC_BW_8080:
	case WL_CHANSPEC_BW_160:
		phy_bw_clkbits = SICF_BW80;
		break;
	default:
		ASSERT(0);	/* should never get here */
	}

	return phy_bw_clkbits;
}

void
wlc_bmac_phy_reset(wlc_hw_info_t *wlc_hw)
{
	uint32 phy_bw_clkbits;

	WL_TRACE(("wl%d: %s\n", wlc_hw->unit, __FUNCTION__));

	ASSERT(wlc_hw->band != NULL);

	phy_bw_clkbits = wlc_bmac_clk_bwbits(wlc_hw);

	if (WLCISNPHY(wlc_hw->band) && NREV_IS(wlc_hw->band->phyrev, 18)) {
		if (si_read_pmu_autopll(wlc_hw->sih))
		{
			if (phy_bw_clkbits != SICF_BW40) {
				/* Set the PHY bandwidth */
				si_core_cflags(wlc_hw->sih, SICF_BWMASK, SICF_BW40);
			}
			/* Turn on Auto reset for PLL phy clock */
			si_pmu_chipcontrol(wlc_hw->sih, PMU1_PLL0_CHIPCTL0, 2, 0);
		}
	}

	/* Specfic reset sequence required for NPHY rev 3 and 4 */
	if (WLCISNPHY(wlc_hw->band) && NREV_GE(wlc_hw->band->phyrev, 3) &&
		NREV_LE(wlc_hw->band->phyrev, 4)) {
		/* Set the PHY bandwidth */
		si_core_cflags(wlc_hw->sih, SICF_BWMASK, phy_bw_clkbits);

		OSL_DELAY(1);

		/* Perform a soft reset of the PHY PLL */
		wlc_bmac_core_phypll_reset(wlc_hw);

		/* reset the PHY */
		si_core_cflags(wlc_hw->sih, (SICF_PRST | SICF_PCLKE),
			(SICF_PRST | SICF_PCLKE));
	} else if (WLCISSSLPNPHY(wlc_hw->band)) {
		uint32 pll_div_mask;
		uint32 pll_div_val;

		if ((SSLPNREV_IS(wlc_hw->band->phyrev, 2)) ||
			(SSLPNREV_IS(wlc_hw->band->phyrev, 4))) {

			/* For sslpnphy 40MHz bw program the pll */
			si_core_cflags(wlc_hw->sih, SICF_BWMASK, phy_bw_clkbits);

			if (phy_bw_clkbits == SICF_BW40) {
				pll_div_mask = PMU1_PLL0_PC1_M4DIV_MASK;
				pll_div_val = PMU1_PLL0_PC1_M4DIV_BY_9 << PMU1_PLL0_PC1_M4DIV_SHIFT;
				si_pmu_pllcontrol(wlc_hw->sih, PMU1_PLL0_PLLCTL1,
					pll_div_mask, pll_div_val);
				OSL_DELAY(5);
				pll_div_mask = PMU1_PLL0_PC2_M5DIV_MASK | PMU1_PLL0_PC2_M6DIV_MASK;
				pll_div_val = ((PMU1_PLL0_PC2_M5DIV_BY_12 <<
							PMU1_PLL0_PC2_M5DIV_SHIFT) |
							(PMU1_PLL0_PC2_M6DIV_BY_18 <<
							 PMU1_PLL0_PC2_M6DIV_SHIFT));
				si_pmu_pllcontrol(wlc_hw->sih, PMU1_PLL0_PLLCTL2,
					pll_div_mask, pll_div_val);
				OSL_DELAY(5);
			} else if (phy_bw_clkbits == SICF_BW20) {
				pll_div_mask = PMU1_PLL0_PC1_M4DIV_MASK;
				pll_div_val = PMU1_PLL0_PC1_M4DIV_BY_18 <<
							PMU1_PLL0_PC1_M4DIV_SHIFT;
				si_pmu_pllcontrol(wlc_hw->sih, PMU1_PLL0_PLLCTL1,
					pll_div_mask, pll_div_val);
				OSL_DELAY(5);
				pll_div_mask = PMU1_PLL0_PC2_M5DIV_MASK | PMU1_PLL0_PC2_M6DIV_MASK;
				pll_div_val = ((PMU1_PLL0_PC2_M5DIV_BY_18 <<
							PMU1_PLL0_PC2_M5DIV_SHIFT) |
							(PMU1_PLL0_PC2_M6DIV_BY_18 <<
							 PMU1_PLL0_PC2_M6DIV_SHIFT));
				si_pmu_pllcontrol(wlc_hw->sih, PMU1_PLL0_PLLCTL2,
					pll_div_mask, pll_div_val);
				OSL_DELAY(5);
			} else if (phy_bw_clkbits == SICF_BW10) {
				pll_div_mask = PMU1_PLL0_PC1_M4DIV_MASK;
				pll_div_val = PMU1_PLL0_PC1_M4DIV_BY_36 <<
							PMU1_PLL0_PC1_M4DIV_SHIFT;
				si_pmu_pllcontrol(wlc_hw->sih, PMU1_PLL0_PLLCTL1,
					pll_div_mask, pll_div_val);
				OSL_DELAY(5);
				pll_div_mask = PMU1_PLL0_PC2_M5DIV_MASK | PMU1_PLL0_PC2_M6DIV_MASK;
				pll_div_val = ((PMU1_PLL0_PC2_M5DIV_BY_36 <<
							PMU1_PLL0_PC2_M5DIV_SHIFT) |
							(PMU1_PLL0_PC2_M6DIV_BY_36 <<
							 PMU1_PLL0_PC2_M6DIV_SHIFT));
				si_pmu_pllcontrol(wlc_hw->sih, PMU1_PLL0_PLLCTL2,
					pll_div_mask, pll_div_val);
				OSL_DELAY(5);
			}

			/* update the pll settings now */
			si_pmu_pllupd(wlc_hw->sih);
			OSL_DELAY(5);

			si_pmu_chipcontrol(wlc_hw->sih, PMU1_PLL0_CHIPCTL0, 0x40000000, 0x40000000);
			OSL_DELAY(5);
			si_pmu_chipcontrol(wlc_hw->sih, PMU1_PLL0_CHIPCTL0, 0x40000000, 0);
		} else if (SSLPNREV_IS(wlc_hw->band->phyrev, 3)) {
			si_corereg(wlc_hw->sih, SI_CC_IDX,
				OFFSETOF(chipcregs_t, min_res_mask), (1<<6), (0<< 6));
				OSL_DELAY(100);
			si_corereg(wlc_hw->sih, SI_CC_IDX,
				OFFSETOF(chipcregs_t, max_res_mask), (1<<6), (0<< 6));
				OSL_DELAY(100);

			if (phy_bw_clkbits == SICF_BW40) {
				pll_div_mask = PMU7_PLL_CTL7_M4DIV_MASK;
				pll_div_val = PMU7_PLL_CTL7_M4DIV_BY_6 << PMU7_PLL_CTL7_M4DIV_SHIFT;
				si_pmu_pllcontrol(wlc_hw->sih,
					PMU7_PLL_PLLCTL7, pll_div_mask, pll_div_val);
				OSL_DELAY(100);
				pll_div_mask = PMU7_PLL_CTL8_M5DIV_MASK | PMU7_PLL_CTL8_M6DIV_MASK;
				pll_div_val = ((PMU7_PLL_CTL8_M5DIV_BY_8 <<
							PMU7_PLL_CTL8_M5DIV_SHIFT) |
							(PMU7_PLL_CTL8_M6DIV_BY_12 <<
							 PMU7_PLL_CTL8_M6DIV_SHIFT));
				si_pmu_pllcontrol(wlc_hw->sih, PMU7_PLL_PLLCTL8,
					pll_div_mask, pll_div_val);
				OSL_DELAY(100);
			} else if (phy_bw_clkbits == SICF_BW20) {
				pll_div_mask = PMU7_PLL_CTL7_M4DIV_MASK;
				pll_div_val = PMU7_PLL_CTL7_M4DIV_BY_12 <<
							PMU7_PLL_CTL7_M4DIV_SHIFT;
				si_pmu_pllcontrol(wlc_hw->sih,
					PMU7_PLL_PLLCTL7, pll_div_mask, pll_div_val);
				OSL_DELAY(100);
				pll_div_mask = PMU7_PLL_CTL8_M5DIV_MASK | PMU7_PLL_CTL8_M6DIV_MASK;
				pll_div_val = ((PMU7_PLL_CTL8_M5DIV_BY_12 <<
							PMU7_PLL_CTL8_M5DIV_SHIFT) |
							(PMU7_PLL_CTL8_M6DIV_BY_12 <<
							 PMU7_PLL_CTL8_M6DIV_SHIFT));
				si_pmu_pllcontrol(wlc_hw->sih, PMU7_PLL_PLLCTL8,
					pll_div_mask, pll_div_val);
				OSL_DELAY(100);
			} else if (phy_bw_clkbits == SICF_BW10) {
				pll_div_mask = PMU7_PLL_CTL7_M4DIV_MASK;
				pll_div_val = PMU7_PLL_CTL7_M4DIV_BY_24 <<
							PMU7_PLL_CTL7_M4DIV_SHIFT;
				si_pmu_pllcontrol(wlc_hw->sih,
					PMU7_PLL_PLLCTL7, pll_div_mask, pll_div_val);
				OSL_DELAY(100);
				pll_div_mask = PMU7_PLL_CTL8_M5DIV_MASK | PMU7_PLL_CTL8_M6DIV_MASK;
				pll_div_val = ((PMU7_PLL_CTL8_M5DIV_BY_24 <<
							PMU7_PLL_CTL8_M5DIV_SHIFT) |
							(PMU7_PLL_CTL8_M6DIV_BY_24 <<
							 PMU7_PLL_CTL8_M6DIV_SHIFT));
				si_pmu_pllcontrol(wlc_hw->sih, PMU7_PLL_PLLCTL8,
					pll_div_mask, pll_div_val);
				OSL_DELAY(100);
			}

			si_pmu_pllcontrol(wlc_hw->sih,
				PMU7_PLL_PLLCTL11, PMU7_PLL_PLLCTL11_MASK, PMU7_PLL_PLLCTL11_VAL);
			OSL_DELAY(100);

			/* update the pll settings now */
			si_pmu_pllupd(wlc_hw->sih);
			OSL_DELAY(100);

			si_corereg(wlc_hw->sih, SI_CC_IDX,
				OFFSETOF(chipcregs_t, min_res_mask), (1<<6), (1<< 6));
				OSL_DELAY(100);
			si_corereg(wlc_hw->sih, SI_CC_IDX,
				OFFSETOF(chipcregs_t, max_res_mask), (1<<6), (1<< 6));
				OSL_DELAY(100);
		}

		si_core_cflags(wlc_hw->sih, (SICF_PRST | SICF_PCLKE | SICF_BWMASK),
			(SICF_PRST | SICF_PCLKE | phy_bw_clkbits));
			OSL_DELAY(100);
	} else if (WLCISACPHY(wlc_hw->band)) {

		si_core_cflags(wlc_hw->sih, (SICF_PRST | SICF_PCLKE | SICF_BWMASK| SICF_FGC),
		               (SICF_PRST | SICF_PCLKE | phy_bw_clkbits| SICF_FGC));
	} else {

		si_core_cflags(wlc_hw->sih, (SICF_PRST | SICF_PCLKE | SICF_BWMASK),
			(SICF_PRST | SICF_PCLKE | phy_bw_clkbits));
	}

	OSL_DELAY(2);
	wlc_bmac_core_phy_clk(wlc_hw, ON);

	/* XXX: ??? moving this to phy init caused a calibration failure, even after
	 *	I added 10 ms delay after turning on the analog core tx/rx
	 */
	if (wlc_hw->band->pi != NULL)
		phy_ana_reset((phy_info_t *)wlc_hw->band->pi);
}

void
wlc_bmac_bw_reset(wlc_hw_info_t *wlc_hw)
{
	WL_TRACE(("wl%d: %s\n", wlc_hw->unit, __FUNCTION__));

	si_core_cflags(wlc_hw->sih, SICF_BWMASK, wlc_bmac_clk_bwbits(wlc_hw));
}

/** switch to and initialize d11 + PHY for operation on caller supplied band */
static void
WLBANDINITFN(wlc_bmac_setband)(wlc_hw_info_t *wlc_hw, uint bandunit, chanspec_t chanspec)
{
	wlc_info_t *wlc = wlc_hw->wlc;
	uint32 macintmask;

	ASSERT(NBANDS_HW(wlc_hw) > 1);
	ASSERT(bandunit != wlc_hw->band->bandunit);

	WL_CHANLOG(wlc_hw->wlc, __FUNCTION__, TS_ENTER, 0);

	/* XXX WES: Why does setband need to check for a disabled core?
	 * Should it always be called when up?
	 * Looks like down a few lines there is an up check, but if down there should be no
	 * touching of the core in setband_inact(), so there should be no iscoreup() check
	 * here.
	 */
	/* Enable the d11 core before accessing it */
	if (!si_iscoreup(wlc_hw->sih)) {
		wlc_bmac_core_reset(wlc_hw, 0, 0);
		ASSERT(si_iscoreup(wlc_hw->sih));
		wlc_mctrl_reset(wlc_hw);
	}

	macintmask = wlc_setband_inact(wlc_hw, bandunit);

	if (!wlc_hw->up) {
		WL_CHANLOG(wlc_hw->wlc, __FUNCTION__, TS_EXIT, 0);
		return;
	}

	/* FREF: switch the pll frequency reference for abg phy */
	if ((WLCISAPHY(wlc_hw->band) || WLCISGPHY(wlc_hw->band)) &&
		D11REV_GT(wlc_hw->corerev, 4)) {
		wlc_bmac_core_phyclk_abg_switch(wlc_hw);
	}

	if (!(WLCISACPHY(wlc_hw->band)))
		wlc_bmac_core_phy_clk(wlc_hw, ON);

	/* band-specific initializations */
	wlc_bmac_bsinit(wlc_hw, chanspec, TRUE);

	/*
	 * If there are any pending software interrupt bits,
	 * then replace these with a harmless nonzero value
	 * so wlc_dpc() will re-enable interrupts when done.
	 */
	if (wlc_hw->macintstatus)
		wlc_hw->macintstatus = MI_DMAINT;

	/* restore macintmask */
	wl_intrsrestore(wlc->wl, macintmask);

	/* ucode should still be suspended.. */
	ASSERT((R_REG(wlc_hw->osh, &wlc_hw->regs->maccontrol) & MCTL_EN_MAC) == 0);
	WL_CHANLOG(wlc_hw->wlc, __FUNCTION__, TS_EXIT, 0);
}

/** low-level band switch utility routine */
void
WLBANDINITFN(wlc_setxband)(wlc_hw_info_t *wlc_hw, uint bandunit)
{
	WL_TRACE(("wl%d: wlc_setxband: bandunit %d\n", wlc_hw->unit, bandunit));

	wlc_hw->band = wlc_hw->bandstate[bandunit];

#ifdef WLC_HIGH
	/* Update the wlc->band pointer for monolithic driver */
	wlc_pi_band_update(wlc_hw->wlc, bandunit);
#endif // endif

	/* set gmode core flag */
	if (wlc_hw->sbclk && !wlc_hw->noreset) {
		si_core_cflags(wlc_hw->sih, SICF_GMODE, ((bandunit == 0) ? SICF_GMODE : 0));
	}
}

static bool
BCMATTACHFN(wlc_isgoodchip)(wlc_hw_info_t *wlc_hw)
{
	/* reject some 4306 package/device combinations */
	if ((CHIPID(wlc_hw->sih->chip) == BCM4306_CHIP_ID) &&
	    (CHIPREV(wlc_hw->sih->chiprev) > 2)) {
		/* 4309 is recognized by a pkg option */
		if (((wlc_hw->deviceid == BCM4306_D11A_ID) ||
		     (wlc_hw->deviceid == BCM4306_D11DUAL_ID)) &&
		    (wlc_hw->sih->chippkg != BCM4309_PKG_ID))
			return FALSE;
	}
	/* reject 4311 A0 device */
	if ((CHIPID(wlc_hw->sih->chip) == BCM4311_CHIP_ID) &&
	    (CHIPREV(wlc_hw->sih->chiprev) == 0)) {
		WL_ERROR(("4311A0 is not supported\n"));
		return FALSE;
	}

	/* rejest 4365 A0 device */
	if (BCM4365_CHIP(wlc_hw->sih->chip) && CHIPREV(wlc_hw->sih->chiprev) == 0) {
		WL_ERROR(("4365A0 is not supported\n"));
		return FALSE;
	}

	/* reject unsupported corerev */
	if (!VALID_COREREV((int)wlc_hw->corerev)) {
		WL_ERROR(("unsupported core rev %d\n", wlc_hw->corerev));
		return FALSE;
	}

	return TRUE;
}

static bool
BCMATTACHFN(wlc_validboardtype)(wlc_hw_info_t *wlc_hw)
{
	bool goodboard = TRUE;
	uint boardtype = wlc_hw->sih->boardtype;
	uint boardrev = wlc_hw->boardrev;

	if (boardrev == 0)
		goodboard = FALSE;
	else if (boardrev > 0xff) {
		uint brt = (boardrev & 0xf000) >> 12;
		uint b0 = (boardrev & 0xf00) >> 8;
		uint b1 = (boardrev & 0xf0) >> 4;
		uint b2 = boardrev & 0xf;

		if ((brt > 2) || (brt == 0) || (b0 > 9) || (b0 == 0) || (b1 > 9) || (b2 > 9))
			goodboard = FALSE;
	}

	if (wlc_hw->sih->boardvendor != VENDOR_BROADCOM)
		return goodboard;

	if ((boardtype == BCM94306MP_BOARD) || (boardtype == BCM94306CB_BOARD)) {
		if (boardrev < 0x40)
			goodboard = FALSE;
	} else if (boardtype == BCM94309MP_BOARD) {
		goodboard = FALSE;
	} else if (boardtype == BCM94309G_BOARD) {
		if (boardrev < 0x51)
			goodboard = FALSE;
	}
	return goodboard;
}

static char *
BCMATTACHFN(wlc_get_macaddr)(wlc_hw_info_t *wlc_hw)
{
	const char *varname = rstr_macaddr;
	char *macaddr;

	/* If macaddr exists, use it (Sromrev4, CIS, ...). */
	if ((macaddr = getvar(wlc_hw->vars, varname)) != NULL)
		return macaddr;

#ifndef BCMSMALL
	/*
	 * Take care of our legacy: MAC addresses can not change
	 * during sw upgrades!
	 * 4309B0 dualband:  il0macaddr
	 * other  dualband:  et1macaddr
	 * uniband-A cards:  et1macaddr
	 * else:             il0macaddr
	 */
	if ((CHIPID(wlc_hw->sih->chip) == BCM4306_CHIP_ID) &&
	    (CHIPREV(wlc_hw->sih->chiprev) == 2) && (NBANDS_HW(wlc_hw) > 1))
		varname = rstr_il0macaddr;
	else if (NBANDS_HW(wlc_hw) > 1)
		varname = rstr_et1macaddr;
	else if (WLCISAPHY(wlc_hw->band))
		varname = rstr_et1macaddr;
	else
		varname = rstr_il0macaddr;

	if ((macaddr = getvar(wlc_hw->vars, varname)) == NULL) {
		WL_ERROR(("wl%d: %s: macaddr getvar(%s) not found\n",
			wlc_hw->unit, __FUNCTION__, varname));
	}
#endif /* !BCMSMALL */

	return macaddr;
}

/**
 * Return TRUE if radio is disabled, otherwise FALSE.
 * hw radio disable signal is an external pin, users activate it asynchronously
 * this function could be called when driver is down and w/o clock
 * it operates on different registers depending on corerev and boardflag.
 */
bool
wlc_bmac_radio_read_hwdisabled(wlc_hw_info_t* wlc_hw)
{
	bool v, clk, xtal;
	uint32 resetbits = 0, flags = 0;

	xtal = wlc_hw->sbclk;
	if (!xtal)
		wlc_bmac_xtal(wlc_hw, ON);

	/* may need to take core out of reset first */
	clk = wlc_hw->clk;
	if (!clk) {
		/*
		 * corerev <= 11, PR 37608 WAR assert PHY_CLK_EN bit whenever asserting sbtm reset
		 */
		if (D11REV_LE(wlc_hw->corerev, 11))
			resetbits |= SICF_PCLKE;

		/*
		 * corerev >= 18, mac no longer enables phyclk automatically when driver accesses
		 * phyreg throughput mac. This can be skipped since only mac reg is accessed below
		 */
		if (D11REV_GE(wlc_hw->corerev, 18))
			flags |= SICF_PCLKE;

		/* AI chip doesn't restore bar0win2 on hibernation/resume, need sw fixup */
		if ((CHIPID(wlc_hw->sih->chip) == BCM43224_CHIP_ID) ||
		    (CHIPID(wlc_hw->sih->chip) == BCM43225_CHIP_ID) ||
		    (CHIPID(wlc_hw->sih->chip) == BCM43421_CHIP_ID)) {
			wlc_hw->regs = (d11regs_t *)si_setcore(
					wlc_hw->sih, D11_CORE_ID, wlc_hw->macunit);
			ASSERT(wlc_hw->regs != NULL);
		}
		wlc_bmac_core_reset(wlc_hw, flags, resetbits);
		wlc_mctrl_reset(wlc_hw);
	}

	v = ((R_REG(wlc_hw->osh, &wlc_hw->regs->phydebug) & PDBG_RFD) != 0);

	/* put core back into reset */
	if (!clk && v) {
		wlc_bmac_core_disable(wlc_hw, 0);
	}

	if (!xtal)
		wlc_bmac_xtal(wlc_hw, OFF);

	return (v);
}

void
wlc_bmac_4360_pcie2_war(wlc_hw_info_t* wlc_hw, uint32 vcofreq)
{
	extern int do_4360_pcie2_war;
	uint32 xtalfreqi;
	uint32 p1div;
	uint32 xtalfreq1;
	uint32 ndiv_int;
	uint32 is_frac;
	uint32 ndiv_mode;
	uint32 val;
	uint32 data;
	int linkspeed;

	if (((CHIPID(wlc_hw->sih->chip) != BCM4360_CHIP_ID) &&
	     (CHIPID(wlc_hw->sih->chip) != BCM43460_CHIP_ID) &&
	     (CHIPID(wlc_hw->sih->chip) != BCM4352_CHIP_ID)) ||
	    (wlc_hw->sih->chiprev > 2) ||
	    (BUSTYPE(wlc_hw->sih->bustype) != PCI_BUS))
		return;

#if !defined(__mips__) && !defined(BCM47XX_CA9)
	if (wl_osl_pcie_rc(wlc_hw->wlc->wl, 0, 0) == 1)	/* pcie gen 1 */
		return;
#endif /* !defined(__mips__) && !defined(BCM47XX_CA9) */

	if (do_4360_pcie2_war != 0)
		return;

	do_4360_pcie2_war = 1;

	si_corereg(wlc_hw->sih, 3, 0x120, ~0, 0xBC);
	data = si_corereg(wlc_hw->sih, 3, 0x124, 0, 0);
	linkspeed = (data >> 16) & 0xf;

	/* don't need the WAR if linkspeed is already gen2 */
	if (linkspeed == 2)
		return;

	/* Save PCI cfg space. (cfg offsets 0x0 - 0x3f) */
	si_pcie_configspace_cache((si_t *)(uintptr)(wlc_hw->sih));

	xtalfreqi = 40;
	p1div = 2;
	xtalfreq1 = xtalfreqi / p1div;
	ndiv_int = vcofreq / xtalfreq1;
	is_frac = (vcofreq % xtalfreq1) > 0 ? 1 : 0;
	ndiv_mode = is_frac ? 3 : 0;
	val = (ndiv_int << 7) | (ndiv_mode << 4) | (p1div << 0);

	si_pmu_pllcontrol(wlc_hw->sih, 10, ~0, val);

	if (is_frac) {
		uint32 frac = (vcofreq % xtalfreq1) * (1 << 24) / xtalfreq1;
		si_pmu_pllcontrol(wlc_hw->sih, 11, ~0, frac);
	}

	/* update pll */
	si_pmu_pllupd(wlc_hw->sih);

	/* Issuing Watchdog Reset */
	si_watchdog(wlc_hw->sih, 2);
	OSL_DELAY(2000);

	/* hot reset */
#if !defined(__mips__) && !defined(BCM47XX_CA9)
	wl_osl_pcie_rc(wlc_hw->wlc->wl, 1, 0);
#endif /* !defined(__mips__) && !defined(BCM47XX_CA9) */
	OSL_DELAY(50 * 1000);

	si_pcie_configspace_restore((si_t *)(uintptr)(wlc_hw->sih));

	/* set pcie gen2 capability */
	si_corereg(wlc_hw->sih, 3, 0x120, ~0, 0x4DC);
	data = si_corereg(wlc_hw->sih, 3, 0x124, 0, 0);

	si_corereg(wlc_hw->sih, 3, 0x120, ~0, 0x4DC);
	si_corereg(wlc_hw->sih, 3, 0x124, ~0, (data & 0xfffffff0) | 2);

	si_corereg(wlc_hw->sih, 3, 0x120, ~0, 0x1800);
	data = si_corereg(wlc_hw->sih, 3, 0x124, 0, 0);

	si_corereg(wlc_hw->sih, 3, 0x120, ~0, 0x1800);
	si_corereg(wlc_hw->sih, 3, 0x124, ~0, (data & 0xfffffff0) | 2);

	si_corereg(wlc_hw->sih, 3, 0x120, ~0, 0x1800);
	si_corereg(wlc_hw->sih, 3, 0x124, ~0, data & 0xfffffff0);

	OSL_DELAY(1000);

	si_corereg(wlc_hw->sih, 3, 0x120, ~0, 0xBC);
	data = si_corereg(wlc_hw->sih, 3, 0x124, 0, 0);
	linkspeed = (data >> 16) & 0xf;

	WL_INFORM(("wl%d: pcie gen2 link speed: %d\n", wlc_hw->unit, linkspeed));
}

/** Initialize just the hardware when coming out of POR or S3/S5 system states */
void
BCMINITFN(wlc_bmac_hw_up)(wlc_hw_info_t *wlc_hw)
{
	if (wlc_hw->wlc->pub->hw_up)
		return;

#if defined(BCM_BACKPLANE_TIMEOUT)
	si_slave_wrapper_add(wlc_hw->sih);
#endif // endif

	WL_TRACE(("wl%d: %s:\n", wlc_hw->unit, __FUNCTION__));
#ifdef WLOFFLD
	if (WLOFFLD_CAP(wlc_hw->wlc)) {
		/* issue wdreset which will clear survive perst and enable srom download */
		si_survive_perst_war(wlc_hw->sih, TRUE, 0, 0);
	}
#endif /* WLOFFLD */

	if (BUSTYPE(wlc_hw->sih->bustype) == PCI_BUS)
		si_ldo_war(wlc_hw->sih, CHIPID(wlc_hw->sih->chip));

	if (CHIPID(wlc_hw->sih->chip) == BCM43142_CHIP_ID)
		si_pmu_res_init(wlc_hw->sih, wlc_hw->osh);

#ifdef WLC_HIGH
	if ((BUSTYPE(wlc_hw->sih->bustype) == PCI_BUS) &&
	    (D11REV_GE(wlc_hw->corerev, 40)))
		si_pmu_res_init(wlc_hw->sih, wlc_hw->osh);

	/* config GPIO1 as HW radio on/off pin for 43162 */
	if ((BUSTYPE(wlc_hw->sih->bustype) == PCI_BUS) &&
		(CHIPID(wlc_hw->sih->chip) == BCM4335_CHIP_ID))
		si_gci_set_functionsel(wlc_hw->sih, CC4335_PIN_GPIO_01, CC4335_FNSEL_MISC0);

	/**
	 * JIRA: SWWLAN-27305 shut the bbpll off in sleep as well as improve the efficiency of
	 * some internal regulator.
	 */
	if (BUSTYPE(wlc_hw->sih->bustype) == PCI_BUS &&
	    (BCM4350_CHIP(wlc_hw->sih->chip) || CHIPID(wlc_hw->sih->chip) == BCM43602_CHIP_ID)) {
		si_pmu_chip_init(wlc_hw->sih, wlc_hw->osh);
		si_pmu_slow_clk_reinit(wlc_hw->sih, wlc_hw->osh);
	}
#endif /* WLC_HIGH */

	/*
	 * Enable pll and xtal, initialize the power control registers,
	 * and force fastclock for the remainder of wlc_up().
	 */
	wlc_bmac_xtal(wlc_hw, ON);
	si_clkctl_init(wlc_hw->sih);
	wlc_clkctl_clk(wlc_hw, CLK_FAST);

#ifdef WLC_HIGH
	if (BUSTYPE(wlc_hw->sih->bustype) == PCI_BUS) {
		si_pcie_hw_LTR_war(wlc_hw->sih);
		si_pciedev_crwlpciegen2(wlc_hw->sih);
		si_pciedev_reg_pm_clk_period(wlc_hw->sih);
	}
#ifndef WL_LTR
	si_pcie_ltr_war(wlc_hw->sih);
#endif // endif
#endif /* WLC_HIGH */

	/* Init BTC related GPIOs to clean state on power up as well. This must
	 * be done here as even if radio is disabled, driver needs to
	 * make sure that output GPIO is lowered
	 */
	wlc_bmac_btc_gpio_disable(wlc_hw);

	if (BUSTYPE(wlc_hw->sih->bustype) == PCI_BUS) {
		/* HW up(initial load, post hibernation resume), core init/fixup */

#ifdef WLC_HIGH
		if ((CHIPID(wlc_hw->sih->chip) == BCM4360_CHIP_ID) ||
		    (CHIPID(wlc_hw->sih->chip) == BCM43460_CHIP_ID) ||
		    (CHIPID(wlc_hw->sih->chip) == BCM4352_CHIP_ID)) {
			/* changing the avb vcoFreq as 510M (from default: 500M) */
			/* Tl clk 127.5Mhz */
				WL_INFORM(("wl%d: %s: settng clock to %d\n",
				wlc_hw->unit, __FUNCTION__,	wlc_hw->vcoFreq_4360_pcie2_war));

				wlc_bmac_4360_pcie2_war(wlc_hw, wlc_hw->vcoFreq_4360_pcie2_war);
			}
#endif /* WLC_HIGH */
		si_pci_fixcfg(wlc_hw->sih);

		/* AI chip doesn't restore bar0win2 on hibernation/resume, need sw fixup */
		if ((CHIPID(wlc_hw->sih->chip) == BCM43224_CHIP_ID) ||
		    (CHIPID(wlc_hw->sih->chip) == BCM43225_CHIP_ID) ||
		    (CHIPID(wlc_hw->sih->chip) == BCM43421_CHIP_ID)) {
			wlc_hw->regs = (d11regs_t *)si_setcore(
					wlc_hw->sih, D11_CORE_ID, wlc_hw->macunit);
			ASSERT(wlc_hw->regs != NULL);
		}

		if (CHIPID(wlc_hw->sih->chip) == BCM4313_CHIP_ID) {
			/* PR84286: 4313 WAR:
			 * RTL sets HT_AVAIL resource by default in min_res_mask, causing high power
			 * Driver clears HT_AVAIL during init and hibernation resume
			 */
			si_clk_pmu_htavail_set(wlc_hw->sih, FALSE);

			/* PR114137: 4313 WAR:
			* Synth_pwrsw resource bit is cleared in min_res_mask after a system
			* sleep/wakeup sequence. Re-enable the resource.
			*/
			si_pmu_synth_pwrsw_4313_war(wlc_hw->sih);
		}
	}

#ifdef WLLED
	wlc_bmac_led_hw_init(wlc_hw);
#endif // endif

#ifdef DMATXRC
	if (DMATXRC_ENAB(wlc_hw->wlc->pub) && PHDR_ENAB(wlc_hw->wlc))
		wlc_phdr_fill(wlc_hw->wlc);
#endif // endif

	/* Inform phy that a POR reset has occurred so it does a complete phy init */
	wlc_phy_por_inform(wlc_hw->band->pi);

#ifdef BCM_OL_DEV
	wlc_hw->ucode_loaded = TRUE;
#else
	wlc_hw->ucode_loaded = FALSE;
#endif // endif
	wlc_hw->wlc->pub->hw_up = TRUE;
	/* 4313 EPA fix */
	if ((wlc_hw->boardflags & BFL_FEM) && (CHIPID(wlc_hw->sih->chip) == BCM4313_CHIP_ID)) {
		if (!(wlc_hw->boardrev >= 0x1250 && (wlc_hw->boardflags & BFL_FEM_BT)))
			si_epa_4313war(wlc_hw->sih);
		else
			si_btcombo_p250_4313_war(wlc_hw->sih);
	}
	if (((CHIPID(wlc_hw->sih->chip) == BCM43228_CHIP_ID)) &&
		(wlc_hw->boardflags & BFL_FEM_BT)) {
		si_btcombo_43228_war(wlc_hw->sih);
		si_pmu_chipcontrol(wlc_hw->sih, PMU1_PLL0_CHIPCTL1, 0x20, 0x20);
	}

#if defined(AP) && defined(WLC_LOW_ONLY)
	wlc_bmac_pa_war_set(wlc_hw, TRUE);
#endif // endif

}

static bool
wlc_dma_rxreset(wlc_hw_info_t *wlc_hw, uint fifo)
{
	hnddma_t *di = wlc_hw->di[fifo];

	if (D11REV_LT(wlc_hw->corerev, 12)) {
		bool rxidle = TRUE;
		uint16 rcv_frm_cnt = 0;
		osl_t *osh = wlc_hw->osh;

		W_REG(osh, &wlc_hw->regs->rcv_fifo_ctl, fifo << 8);
		SPINWAIT((!(rxidle = dma_rxidle(di))) &&
		         ((rcv_frm_cnt = R_REG(osh, &wlc_hw->regs->rcv_frm_cnt)) != 0), 50000);

		if (!rxidle && (rcv_frm_cnt != 0))
			WL_ERROR(("wl%d: %s: rxdma[%d] not idle && rcv_frm_cnt(%d) not zero\n",
			          wlc_hw->unit, __FUNCTION__, fifo, rcv_frm_cnt));
		OSL_DELAY(2000);
	}

	return (dma_rxreset(di));
}

/**
 * d11 core reset
 *   ensure fask clock during reset
 *   reset dma
 *   reset d11(out of reset)
 *   reset phy(out of reset)
 *   clear software macintstatus for fresh new start
 * one testing hack wlc_hw->noreset will bypass the d11/phy reset
 */
void
BCMINITFN(wlc_bmac_corereset)(wlc_hw_info_t *wlc_hw, uint32 flags)
{
	uint i, nfifo = NFIFO;
	bool fastclk;
	uint32 resetbits = 0;
	uint32 macintmask = 0;

	if (flags == WLC_USE_COREFLAGS)
		flags = (wlc_hw->band->pi ? wlc_hw->band->core_flags : 0);

	WL_TRACE(("wl%d: %s\n", wlc_hw->unit, __FUNCTION__));

	/* request FAST clock if not on  */
	if (!(fastclk = wlc_hw->forcefastclk))
		wlc_clkctl_clk(wlc_hw, CLK_FAST);

	/* reset the dma engines except if core is in reset (first time thru or bigger hammer) */
	if (si_iscoreup(wlc_hw->sih)) {
		if (!PIO_ENAB_HW(wlc_hw)) {
#ifndef BCM_OL_DEV
			hnddma_t *di;

			/* XXX: use CTdma macro here to get nfifo; too early
			 * to use wlc_hw->nfifo_inuse here.
			 */
			if (BCM_DMA_CT_ENAB(wlc_hw->wlc))
				nfifo = NFIFO_EXT;
			for (i = 0; i < nfifo; i++) {
				di = wlc_hw->di[i];
#ifdef BCM_DMA_CT
				if (di && BCM_DMA_CT_ENAB(wlc_hw->wlc)) {
					/* Reset Data DMA channel */
					if (!dma_txreset(di)) {
						WL_ERROR(("wl%d: %s: dma_txreset[%d]: cannot stop "
							"dma\n", wlc_hw->unit, __FUNCTION__, i));
					}

					/* Reset AQM DMA channel */
					di = wlc_hw->aqm_di[i];
				}
#endif /* BCM_DMA_CT */
				if (di) {
					if (!dma_txreset(di)) {
						WL_ERROR(("wl%d: %s: dma_txreset[%d]: cannot stop "
							"dma\n", wlc_hw->unit, __FUNCTION__, i));
						WL_HEALTH_LOG(wlc_hw->wlc, DMATX_ERROR);
					}
					wlc_upd_suspended_fifos_clear(wlc_hw, i);
				}
			}
#endif /* BCM_OL_DEV */
			if ((wlc_hw->di[RX_FIFO]) && (!wlc_dma_rxreset(wlc_hw, RX_FIFO))) {
				WL_ERROR(("wl%d: %s: dma_rxreset[%d]: cannot stop dma\n",
				          wlc_hw->unit, __FUNCTION__, RX_FIFO));
				WL_HEALTH_LOG(wlc_hw->wlc, DMARX_ERROR);
			}
			if (D11REV_IS(wlc_hw->corerev, 4) && wlc_hw->di[RX_TXSTATUS_FIFO] &&
			    (!wlc_dma_rxreset(wlc_hw, RX_TXSTATUS_FIFO))) {
				WL_ERROR(("wl%d: %s: dma_rxreset[%d]: cannot stop dma\n",
				          wlc_hw->unit, __FUNCTION__, RX_TXSTATUS_FIFO));
			}
		} else {
			for (i = 0; i < nfifo; i++)
				if (wlc_hw->pio[i])
					wlc_pio_reset(wlc_hw->pio[i]);
		}
	}
	/* if noreset, just stop the psm and return */
	if (wlc_hw->noreset) {
		wlc_hw->macintstatus = 0;	/* skip wl_dpc after down */
#ifndef BCM_OL_DEV
		wlc_bmac_mctrl(wlc_hw, MCTL_PSM_RUN | MCTL_EN_MAC, 0);
#endif /* BCM_OL_DEV */
		return;
	}

	/*
	 * corerev <= 11, PR 37608 WAR assert PHY_CLK_EN bit whenever asserting sbtm reset
	 */
	if (D11REV_LE(wlc_hw->corerev, 11))
		resetbits |= SICF_PCLKE;

	/*
	 * corerev >= 18, mac no longer enables phyclk automatically when driver accesses phyreg
	 * throughput mac, AND phy_reset is skipped at early stage when band->pi is invalid
	 * need to enable PHY CLK
	 */
	if (D11REV_GE(wlc_hw->corerev, 18))
		flags |= SICF_PCLKE;

	/* reset the core
	 * In chips with PMU, the fastclk request goes through d11 core reg 0x1e0, which
	 *  is cleared by the core_reset. have to re-request it.
	 *  This adds some delay and we can optimize it by also requesting fastclk through
	 *  chipcommon during this period if necessary. But that has to work coordinate
	 *  with other driver like mips/arm since they may touch chipcommon as well.
	 *  RSDB chips handle core reset programming of both cores from core 0
	 *  context only.
	 *  RSDB chip does D11 core reset only in Core 0 context.
	 *  In case of SISO 1 mode core reset sequence is required in CORE 1
	 *  context as well.
	 *
	 */
	/* disable interrupts */
	if (wlc_hw->macintmask != 0)
		macintmask = wl_intrsoff(wlc_hw->wlc->wl);

	wlc_hw->clk = FALSE;
	if (si_coreunit(wlc_hw->sih) == 0x0 ||
		wlc_rsdb_mode(wlc_hw->wlc) == PHYMODE_RSDB_SISO_1) {
		wlc_bmac_core_reset(wlc_hw, flags, resetbits);
	}
	wlc_hw->clk = TRUE;

	if (macintmask != 0)
		wl_intrsrestore(wlc_hw->wlc->wl, macintmask);

	/* PHY Mode has to be written only in Core 0 cflags.
	 * For Core 1 override, switch to core-0 and write it.
	 */
	if (wlc_bmac_rsdb_cap(wlc_hw)) {
		wlc_rsdb_set_phymode(wlc_hw->wlc, (wlc_rsdb_mode(wlc_hw->wlc)));
	}
	/*
	 * If band->phytype & band->phyrev are not yet known, get them from the d11 registers.
	 * The phytype and phyrev are used in WLCISXXX() and XXXREV_XX() macros.
	 */
	ASSERT(wlc_hw->regs != NULL);
	if (wlc_hw->band && wlc_hw->band->phytype == 0 && wlc_hw->band->phyrev == 0) {
		uint16 phyversion = R_REG(wlc_hw->osh, &wlc_hw->regs->phyversion);

		wlc_hw->band->phytype = PHY_TYPE(phyversion);
		wlc_hw->band->phyrev = phyversion & PV_PV_MASK;
	}

	if (wlc_hw->band && wlc_hw->band->pi)
		wlc_phy_hw_clk_state_upd(wlc_hw->band->pi, TRUE);

	if (D11REV_IS(wlc_hw->corerev, 33)) {
		/* CRLCNPHY-668: WAR for phy reg access hang in 4334/4314/43142 chips.
		 * A restore pulse to the phy unwedges the reg access
		 */
		wlc_bmac_write_ihr(wlc_hw, PHY_CTRL, PHY_CTRL_RESTORESTART | PHY_CTRL_MC);
		wlc_bmac_write_ihr(wlc_hw, PHY_CTRL, PHY_CTRL_MC);
	}

	if (wlc_hw->band && WLCISACPHY(wlc_hw->band)) {
		/* set up highspeed DAC mode to 1 by default
		 * (see default value 0 is undefined mode)
		 */
		si_core_cflags(wlc_hw->sih, SICF_DAC, 0x100);

		/* turn off phy clocks */
		si_core_cflags(wlc_hw->sih, (SICF_FGC | SICF_PCLKE), 0);

		/* re-enable phy clocks to resync to macphy clock */
		si_core_cflags(wlc_hw->sih, SICF_PCLKE, SICF_PCLKE);
	}

	wlc_mctrl_reset(wlc_hw);
	if (D11REV_GE(wlc_hw->corerev, 64)) {
		wlc_mctrlx_reset(wlc_hw);
	}

	if (PMUCTL_ENAB(wlc_hw->sih))
		wlc_clkctl_clk(wlc_hw, CLK_FAST);

	if (wlc_hw->band && wlc_hw->band->pi) {
		wlc_bmac_phy_reset(wlc_hw);
	}

	/* turn on PHY_PLL */
	wlc_bmac_core_phypll_ctl(wlc_hw, TRUE);

	/* clear sw intstatus */
	wlc_hw->macintstatus = 0;

	/* restore the clk setting */
	if (!fastclk)
		wlc_clkctl_clk(wlc_hw, CLK_DYNAMIC);

#ifdef WLP2P_UCODE
	wlc_hw->p2p_shm_base = (uint)~0;
#endif // endif
	wlc_hw->cca_shm_base = (uint)~0;
}

/* Search mem rw utilities */

#ifdef MBSS
bool
wlc_bmac_ucodembss_hwcap(wlc_hw_info_t *wlc_hw)
{
	/* add up template space here */
	int templ_ram_sz, fifo_mem_used, i, stat;
	uint blocks = 0;
	wlc_info_t *wlc = wlc_hw->wlc;

	for (fifo_mem_used = 0, i = 0; i < NFIFO; i++) {
		stat = wlc_bmac_xmtfifo_sz_get(wlc_hw, i, &blocks);
		if (stat != 0) return FALSE;
		fifo_mem_used += blocks;
	}

	templ_ram_sz = ((wlc_hw->machwcap & MCAP_TXFSZ_MASK) >> MCAP_TXFSZ_SHIFT) * 2;

	if ((templ_ram_sz - fifo_mem_used) < (int)MBSS_TPLBLKS(WLC_MAX_AP_BSS(wlc_hw->corerev))) {
		WL_ERROR(("wl%d: %s: Insuff mem for MBSS: templ memblks %d fifo memblks %d\n",
			wlc_hw->unit, __FUNCTION__, templ_ram_sz, fifo_mem_used));
		return FALSE;
	}

	return TRUE;
}
#endif /* MBSS */

#ifndef BCM_OL_DEV
/**
 * If the ucode that supports corerev 5 is used for corerev 9 and above, txfifo sizes needs to be
 * modified (increased) since the newer cores have more memory.
 */
static void
BCMINITFN(wlc_corerev_fifofixup)(wlc_hw_info_t *wlc_hw)
{
	d11regs_t *regs = wlc_hw->regs;
	uint16 fifo_nu;
	uint16 txfifo_startblk = TXFIFO_START_BLK, txfifo_endblk;
	uint16 txfifo_def, txfifo_def1;
	uint16 txfifo_cmd;
	osl_t *osh;

	if (D11REV_LT(wlc_hw->corerev, 9))
		goto exit;

	/* Re-assign the space for tx fifos to allow BK aggregation */
	if (D11REV_IS(wlc_hw->corerev, 28)) {
		uint16 xmtsz[] = { 30, 47, 22, 14, 8, 1 };

		memcpy(xmtfifo_sz[(wlc_hw->corerev - XMTFIFOTBL_STARTREV)],
		       xmtsz, sizeof(xmtsz));
	} else if (D11REV_IS(wlc_hw->corerev, 16) || D11REV_IS(wlc_hw->corerev, 17)) {
		uint16 xmtsz[] = { 98, 159, 160, 21, 8, 1 };

		memcpy(xmtfifo_sz[(wlc_hw->corerev - XMTFIFOTBL_STARTREV)],
		       xmtsz, sizeof(xmtsz));
	}

	if ((CHIPID(wlc_hw->sih->chip) == BCM43242_CHIP_ID)) {
		uint16 xmtsz[] = { 18, 254, 25, 17, 17, 8 };
		memcpy(xmtfifo_sz[(wlc_hw->corerev - XMTFIFOTBL_STARTREV)],
		       xmtsz, sizeof(xmtsz));
	}

	/* tx fifos start at TXFIFO_START_BLK from the Base address */
#ifdef MBSS
	if (D11REV_ISMBSS16(wlc_hw->corerev)) {
		/* 4313 has total fifo space of 128 blocks. if we enable
		 * all 16 MBSSs we will not be left with enough fifo space to
		 * support max thru'put. so we only allow configuring/enabling
		 * max of 4 BSSs. Rest of the space is distributed acorss
		 * the tx fifos.
		 */
		if (D11REV_IS(wlc_hw->corerev, 24)) {
#ifdef WLLPRS
			uint16 xmtsz[] = { 9, 39, 22, 14, 14, 5 };
#else
			uint16 xmtsz[] = { 9, 47, 22, 14, 14, 5 };
#endif // endif
			memcpy(xmtfifo_sz[(wlc_hw->corerev - XMTFIFOTBL_STARTREV)],
			       xmtsz, sizeof(xmtsz));
		}
#ifdef WLLPRS
		/* tell ucode the lprs size is 0x80 * 4bytes. */
		wlc_write_shm(wlc, SHM_MBSS_BC_FID2, 0x80);
#endif /* WLLPRS */
		if (D11REV_IS(wlc_hw->corerev, 25)) {
			uint16 xmtsz[] = { 9, 47, 22, 14, 14, 5 };
			memcpy(xmtfifo_sz[(wlc_hw->corerev - XMTFIFOTBL_STARTREV)],
				xmtsz, sizeof(xmtsz));
		}

#ifdef WLC_HIGH
		if (MBSS_ENAB(wlc_hw->wlc->pub)) {
#endif /* WLC_HIGH */
			if (wlc_bmac_ucodembss_hwcap(wlc_hw)) {
				wlc_info_t *wlc = wlc_hw->wlc;

				ASSERT(WLC_MAX_AP_BSS(wlc_hw->corerev) > 0);
				txfifo_startblk =
					MBSS_TXFIFO_START_BLK(WLC_MAX_AP_BSS(wlc_hw->corerev));
			}
#ifdef WLC_HIGH
		}
#endif /* WLC_HIGH */
	} else
#endif /* MBSS */
	txfifo_startblk = TXFIFO_START_BLK;

	/* NEW */

	osh = wlc_hw->osh;

	/* sequence of operations:  reset fifo, set fifo size, reset fifo */
	for (fifo_nu = 0; fifo_nu < NFIFO; fifo_nu++) {

		txfifo_endblk = txfifo_startblk + wlc_hw->xmtfifo_sz[fifo_nu];
		txfifo_def = (txfifo_startblk & 0xff) |
			(((txfifo_endblk - 1) & 0xff) << TXFIFO_FIFOTOP_SHIFT);
		txfifo_def1 = ((txfifo_startblk >> 8) & 0x3) |
			((((txfifo_endblk - 1) >> 8) & 0x3) << TXFIFO_FIFOTOP_SHIFT);
		txfifo_cmd = TXFIFOCMD_RESET_MASK | (fifo_nu << TXFIFOCMD_FIFOSEL_SHIFT);

		W_REG(osh, &regs->u.d11regs.xmtfifocmd, txfifo_cmd);
		W_REG(osh, &regs->u.d11regs.xmtfifodef, txfifo_def);
		if (D11REV_GE(wlc_hw->corerev, 16))
			W_REG(osh, &regs->u.d11regs.xmtfifodef1, txfifo_def1);

		W_REG(osh, &regs->u.d11regs.xmtfifocmd, txfifo_cmd);

		txfifo_startblk += wlc_hw->xmtfifo_sz[fifo_nu];
	}
exit:
	/* need to propagate to shm location to be in sync since ucode/hw won't do this */
	wlc_bmac_write_shm(wlc_hw, M_FIFOSIZE0, wlc_hw->xmtfifo_sz[TX_AC_BE_FIFO]);
	wlc_bmac_write_shm(wlc_hw, M_FIFOSIZE1, wlc_hw->xmtfifo_sz[TX_AC_VI_FIFO]);
	wlc_bmac_write_shm(wlc_hw, M_FIFOSIZE2, ((wlc_hw->xmtfifo_sz[TX_AC_VO_FIFO] << 8) |
		wlc_hw->xmtfifo_sz[TX_AC_BK_FIFO]));
	wlc_bmac_write_shm(wlc_hw, M_FIFOSIZE3, ((wlc_hw->xmtfifo_sz[TX_ATIM_FIFO] << 8) |
		wlc_hw->xmtfifo_sz[TX_BCMC_FIFO]));
	/* Check if TXFIFO HW config is proper */
	wlc_bmac_txfifo_sz_chk(wlc_hw);
}

#endif /* BCM_OL_DEV */

#ifndef BCM_OL_DEV
static void
BCMINITFN(wlc_bmac_btc_init)(wlc_hw_info_t *wlc_hw)
{

	/* make sure 2-wire or 3-wire decision has been made */
	ASSERT((wlc_hw->btc->wire >= WL_BTC_2WIRE) || (wlc_hw->btc->wire <= WL_BTC_4WIRE));

	/* Configure selected BTC mode */
	wlc_bmac_btc_mode_set(wlc_hw, wlc_hw->btc->mode);

	if (wlc_hw->boardflags2 & BFL2_BTCLEGACY) {
		if ((CHIPID(wlc_hw->sih->chip) == BCM4313_CHIP_ID) ||
		    (CHIPID(wlc_hw->sih->chip) == BCM4331_CHIP_ID) ||
		    (CHIPID(wlc_hw->sih->chip) == BCM43431_CHIP_ID))
			si_btc_enable_chipcontrol(wlc_hw->sih);
		/* Pin muxing changes for BT coex operation in LCNXNPHY */
		if ((CHIPID(wlc_hw->sih->chip) == BCM43131_CHIP_ID) ||
			(CHIPID(wlc_hw->sih->chip) == BCM43217_CHIP_ID) ||
			(CHIPID(wlc_hw->sih->chip) == BCM43227_CHIP_ID) ||
			(CHIPID(wlc_hw->sih->chip) == BCM43228_CHIP_ID) ||
			(CHIPID(wlc_hw->sih->chip) == BCM43428_CHIP_ID)) {
			si_btc_enable_chipcontrol(wlc_hw->sih);
			si_pmu_chipcontrol(wlc_hw->sih, PMU1_PLL0_CHIPCTL1, 0x10, 0x10);
		}

		if (CHIPID(wlc_hw->sih->chip) == BCM4313_CHIP_ID) {
			if (wlc_bmac_btc_mode_get(wlc_hw))
				wlc_phy_btclock_war(wlc_hw->band->pi, wlc_hw->btclock_tune_war);
		}
	}

	/* starting from ccrev 35, seci, 3/4 wire can be controlled by newly
	 * constructed SECI block.
	 * exception: X19 (4331) does not utilize this new feature
	 */
	if (wlc_hw->boardflags & BFL_BTCOEX) {
		if (wlc_hw->boardflags2 & BFL2_BTCLEGACY) {
			/* X19 has its special 4 wire which is not using new SECI block */
			if (CHIPID(wlc_hw->sih->chip) != BCM4331_CHIP_ID)
				si_seci_init(wlc_hw->sih, SECI_MODE_LEGACY_3WIRE_WLAN);
		}
		else if (BCMECICOEX_ENAB_BMAC(wlc_hw))
			si_eci_init(wlc_hw->sih);
		else if (BCMSECICOEX_ENAB_BMAC(wlc_hw))
			si_seci_init(wlc_hw->sih, SECI_MODE_SECI);
		else if (BCMGCICOEX_ENAB_BMAC(wlc_hw))
			si_gci_init(wlc_hw->sih);
	}
}

#ifdef WLCX_ATLAS
static void
BCMINITFN(wlc_bmac_wlcx_init)(wlc_hw_info_t *wlc_hw)
{
	/* GCI/SECI initialization for WLCX protocol on FL-ATLAS platform. */
	if ((wlc_hw->boardflags2 & BFL2_WLCX_ATLAS) && si_gci(wlc_hw->sih)) {
		si_gci_seci_init(wlc_hw->sih);
	}
}

#endif /* WLCX_ATLAS */
#endif /* BCM_OL_DEV */

/**
 * d11 core init
 *   reset PSM
 *   download ucode/PCM
 *   let ucode run to suspended
 *   download ucode inits
 *   config other core registers
 *   init dma/pio
 *   init VASIP
 */
static void
BCMINITFN(wlc_coreinit)(wlc_hw_info_t *wlc_hw)
{
	wlc_info_t *wlc = wlc_hw->wlc;
	d11regs_t *regs = wlc_hw->regs;
	wlc_tunables_t *tune = wlc->pub->tunables;
	uint16 rcv_fifo_ctl = 0;
#ifndef BCM_OL_DEV
#ifndef BCMUCDOWNLOAD
	uint32 sflags;
#endif // endif
	bool fifosz_fixup = FALSE;
	uint16 buf[NFIFO] = {0, 0, 0, 0, 0, 0};
#ifdef STA
	uint32 seqnum = 0;
#endif // endif
#endif /* BCM_OL_DEV */
	uint bcnint_us;
	uint i = 0;
	osl_t *osh = wlc_hw->osh;
#if defined(MBSS)
	bool ucode9 = TRUE;
	(void)ucode9;
#endif // endif

	WL_TRACE(("wl%d: wlc_coreinit\n", wlc_hw->unit));

#if defined(BCM_DMA_CT) && !defined(BCM_DMA_CT_DISABLED)
	/* If ctdma is not preconfigured, we need to reattach the DMA so that
	 * DMA is initialized properly with CDTMA on or off.
	 * DMA would not be reattached if user preconfigures ctdma=1 where CTDMA
	 * would always be enabled regardless of MU or SU.
	 */
	if (!(wlc->_dma_ct_flags & DMA_CT_PRECONFIGURED) &&
	     (wlc->_dma_ct_flags & DMA_CT_IOCTL_OVERRIDE)) {
		bool wme = FALSE;
#ifdef WME
		wme = TRUE;
#endif /* WME */
		wlc_bmac_detach_dmapio(wlc_hw);

		wlc_hw->wlc->_dma_ct = (wlc_hw->wlc->_dma_ct_flags & DMA_CT_IOCTL_CONFIG)?
			TRUE : FALSE;
		wlc_hw->wlc->_dma_ct_flags &= ~DMA_CT_IOCTL_OVERRIDE;
#if defined(DMATXRC) && !defined(DMATXRC_DISABLED)
		wlc_hw->wlc->pub->_dmatxrc = TRUE;
#else
		wlc_hw->wlc->pub->_dmatxrc = FALSE;
#endif /* defined(DMATXRC) && !defined(DMATXRC_DISABLED */

#ifdef DMATXRC
		if (BCM_DMA_CT_ENAB(wlc) && DMATXRC_ENAB(wlc->pub)) {
			wlc->pub->_dmatxrc = FALSE;
		}
#endif // endif
		if (!wlc_bmac_attach_dmapio(wlc_hw, wme)) {
			WL_ERROR(("%s: DMA attach failed!\n", __FUNCTION__));
		}

		if (BCMSPLITRX_ENAB()) {
			/* Pass down pool info to dma layer */
			if (wlc_hw->di[RX_FIFO])
				dma_pktpool_set(wlc_hw->di[RX_FIFO], wlc->pub->pktpool_rxlfrag);

			/* if second fifo is set, set pktpool for that */
			if (wlc_hw->di[RX_FIFO1])
				dma_pktpool_set(wlc_hw->di[RX_FIFO1],
				(RXFIFO_SPLIT()?wlc->pub->pktpool_rxlfrag:wlc->pub->pktpool));

			/* FIFO- 2 */
			if (wlc_hw->di[RX_FIFO2])
				dma_pktpool_set(wlc_hw->di[RX_FIFO2], wlc->pub->pktpool);
		} else if (POOL_ENAB(wlc->pub->pktpool)) {
			/* set pool for rx dma */
			if (wlc_hw->di[RX_FIFO])
				dma_pktpool_set(wlc_hw->di[RX_FIFO], wlc->pub->pktpool);
		}
	}
#endif /* defined(BCM_DMA_CT) && !defined(BCM_DMA_CT_DISABLED) */

#ifndef BCM_OL_DEV
	/* reset PSM */
	wlc_bmac_mctrl(wlc_hw, ~0, (MCTL_IHR_EN | MCTL_PSM_JMP_0 | MCTL_WAKE));

	wlc_bmac_btc_init(wlc_hw);

#ifdef WLCX_ATLAS
	/* Initializate WLCX SECI/GCI configuration */
	wlc_bmac_wlcx_init(wlc_hw);
#endif // endif

#if (defined(BCM_DMA_CT) && !defined(BCM_DMA_CT_DISABLED)) || !defined(DONGLEBUILD)
	/*
	 * for dual d11 core chips, ucode is downloaded only once
	 * and will be thru core-0
	 */
	if (wlc_hw->macunit == 0) {
		wlc_ucode_download(wlc_hw);
	}
#endif // endif
#if defined(SAVERESTORE) && !defined(DONGLEBUILD)
#ifdef SR_ESSENTIALS
	/* Only needs to be done once.
	 * Needs this before si_pmu_res_init() to use sr_isenab()
	 */
	if (SR_ESSENTIALS_ENAB())
		sr_save_restore_init(wlc_hw->sih);
#endif /* SR_ESSENTIALS */
	if (SR_ENAB() && sr_cap(wlc_hw->sih)) {
		/* Download SR code */
		wlc_bmac_sr_init(wlc_hw);
	}
#endif /* SAVERESTORE && !DONGLEBUILD */
	/*
	 * FIFOSZ fixup
	 * 1) core5-9 use ucode 5 to save space since the PSM is the same
	 * 2) newer chips, driver wants to controls the fifo allocation
	 */
	if (D11REV_GE(wlc_hw->corerev, 4))
		fifosz_fixup = TRUE;

	/* write the PCM ucode for cores supporting AES (via the PCM) */
	if (D11REV_IS(wlc_hw->corerev, 4))
		wlc_ucode_pcm_write(wlc_hw, d11pcm4, d11pcm4sz);
	else if (D11REV_LT(wlc_hw->corerev, 11))
		wlc_ucode_pcm_write(wlc_hw, d11pcm5, d11pcm5sz);

	(void) wlc_bmac_wowlucode_start(wlc_hw);

	wlc_gpio_init(wlc_hw);

	if (D11REV_GE(wlc_hw->corerev, 40)) {
		wlc_bmac_reset_amt(wlc_hw);
	}

	if (D11REV_GE(wlc_hw->corerev, 64)) {
		wlc_bmac_reset_auxpmq(wlc_hw);
	}

#ifdef WL11N
	/* REV8+: mux out 2o3 control lines when 3 antennas are available */
	if (wlc_hw->antsel_avail) {
		if (CHIPID(wlc_hw->sih->chip) == BCM43234_CHIP_ID ||
		    CHIPID(wlc_hw->sih->chip) == BCM43235_CHIP_ID ||
		    CHIPID(wlc_hw->sih->chip) == BCM43236_CHIP_ID ||
		    CHIPID(wlc_hw->sih->chip) == BCM43238_CHIP_ID) {
			si_corereg(wlc_hw->sih, SI_CC_IDX, OFFSETOF(chipcregs_t, chipcontrol),
				CCTRL43236_ANT_MUX_2o3, CCTRL43236_ANT_MUX_2o3);

		} else if (((CHIPID(wlc_hw->sih->chip)) == BCM5357_CHIP_ID) ||
		           ((CHIPID(wlc_hw->sih->chip)) == BCM4749_CHIP_ID) ||
		           ((CHIPID(wlc_hw->sih->chip)) == BCM53572_CHIP_ID)) {
			si_pmu_chipcontrol(wlc_hw->sih, 1, CCTRL5357_ANT_MUX_2o3,
				CCTRL5357_ANT_MUX_2o3);
		}
	}
#endif	/* WL11N */

#ifdef STA
	/* store the previous sequence number */
	wlc_bmac_copyfrom_objmem(wlc->hw, S_SEQ_NUM << 2, &seqnum, sizeof(seqnum), OBJADDR_SCR_SEL);

#endif /* STA */

#ifdef BCMUCDOWNLOAD
	if (initvals_ptr) {
		wlc_write_inits(wlc_hw, initvals_ptr);
#ifdef BCMRECLAIM
		MFREE(wlc->osh, initvals_ptr, initvals_len);
		initvals_ptr = NULL;
		initvals_len = 0;
#endif // endif
	}
	else
		printf("initvals_ptr is NULL, error in inivals download\n");
#else
	sflags = si_core_sflags(wlc_hw->sih, 0, 0);

#ifdef WLRSDB
	/* init IHR, SHM, and SCR */
	if (wlc_bmac_rsdb_cap(wlc_hw))  {

		wlc_bmac_rsdb_write_inits(wlc_hw, d11ac12initvals50, d11ac12initvals50core1);

	} else
#endif /* WLRSDB */
	if (D11REV_IS(wlc_hw->corerev, 65)) {
		if (WLCISACPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, d11ac33initvals65);
			wlc_write_inits(wlc_hw, d11ac33initvalsx65);
		} else {
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
				__FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
		}
	} else if (D11REV_IS(wlc_hw->corerev, 64)) {
		if (WLCISACPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, d11ac32initvals64);
			wlc_write_inits(wlc_hw, d11ac32initvalsx64);
		} else {
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
				__FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
		}
	} else if (D11REV_IS(wlc_hw->corerev, 49)) {
		if (WLCISACPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, d11ac9initvals49);
		} else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev 40\n",
				__FUNCTION__, wlc_hw->unit));
	} else if (D11REV_IS(wlc_hw->corerev, 48)) {
		if (WLCISACPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, d11ac8initvals48);
		} else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev 40\n",
				__FUNCTION__, wlc_hw->unit));
	} else if (D11REV_IS(wlc_hw->corerev, 45) || D11REV_IS(wlc_hw->corerev, 47) ||
	           D11REV_IS(wlc_hw->corerev, 51) || D11REV_IS(wlc_hw->corerev, 52)) {
		if (WLCISACPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, d11ac7initvals47);
		} else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev 40\n",
				__FUNCTION__, wlc_hw->unit));
	} else if (D11REV_IS(wlc_hw->corerev, 46)) {
		if (WLCISACPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, d11ac6initvals46);
		} else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev 40\n",
				__FUNCTION__, wlc_hw->unit));
	} else if (D11REV_IS(wlc_hw->corerev, 43)) {
		if (WLCISACPHY(wlc_hw->band)) {
		    wlc_write_inits(wlc_hw, d11ac3initvals43);
		} else {
			WL_ERROR(("%s: wl%d: unsupported phy in corerev 40\n",
				__FUNCTION__, wlc_hw->unit));
		}
	} else if (D11REV_IS(wlc_hw->corerev, 42)) {
		if (WLCISACPHY(wlc_hw->band)) {
		    wlc_write_inits(wlc_hw, d11ac1initvals42);
		} else {
			WL_ERROR(("%s: wl%d: unsupported phy in corerev 40\n",
				__FUNCTION__, wlc_hw->unit));
		}
	} else if (D11REV_IS(wlc_hw->corerev, 41) || D11REV_IS(wlc_hw->corerev, 44) ||
		D11REV_IS(wlc_hw->corerev, 45)) {
		if (WLCISACPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, d11ac2initvals41);
		} else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev 40\n",
				__FUNCTION__, wlc_hw->unit));
	} else if (D11REV_IS(wlc_hw->corerev, 40)) {
		if (WLCISACPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, d11ac0initvals40);
		} else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev 40\n",
				__FUNCTION__, wlc_hw->unit));
	} else if (D11REV_IS(wlc_hw->corerev, 38)) {
		if (WLCISLCN40PHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, wlc_get_d11lcn407initvals38_addr());
		} else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
				__FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
	} else if (D11REV_IS(wlc_hw->corerev, 37)) {
		if (WLCISLCN40PHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, wlc_get_d11lcn406initvals37_addr());
		} else if (WLCISNPHY(wlc_hw->band)) {
			fifosz_fixup = TRUE;
			wlc_write_inits(wlc_hw, wlc_get_n22initvals31_addr());
		} else {
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
				__FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
		}
	} else if (D11REV_IS(wlc_hw->corerev, 34)) {
		if (WLCISNPHY(wlc_hw->band)) {
			fifosz_fixup = TRUE;
			wlc_write_inits(wlc_hw, wlc_get_n19initvals34_addr());
		} else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev 34\n",
				__FUNCTION__, wlc_hw->unit));
	} else if (D11REV_IS(wlc_hw->corerev, 33)) {
		if (WLCISLCN40PHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, wlc_get_d11lcn400initvals33_addr());

			/* XXX 43142/4334a2 has a bug that PHY might got stuck when coming
			 * back from slow clock.
			 */
			wlc_bmac_mhf(wlc_hw, MHF5, MHF5_SPIN_AT_SLEEP,
				MHF5_SPIN_AT_SLEEP, WLC_BAND_2G);
		} else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
				__FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
	} else if (D11REV_IS(wlc_hw->corerev, 32)) {
		if (WLCISNPHY(wlc_hw->band)) {
			fifosz_fixup = TRUE;
			wlc_write_inits(wlc_hw, wlc_get_n18initvals32_addr());
		} else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev 32\n",
				__FUNCTION__, wlc_hw->unit));
	} else if (D11REV_IS(wlc_hw->corerev, 31)) {
		if (WLCISNPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, d11ht0initvals29);
		} else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
				__FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
	} else if (D11REV_IS(wlc_hw->corerev, 30)) {
		if (WLCISNPHY(wlc_hw->band))
			wlc_write_inits(wlc_hw, d11n16initvals30);
	} else if (D11REV_IS(wlc_hw->corerev, 29)) {
		if (WLCISHTPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, d11ht0initvals29);
		} else
			WL_ERROR(("wl%d: unsupported phy in corerev 26 \n", wlc_hw->unit));
	} else if (D11REV_IS(wlc_hw->corerev, 26)) {
		if (WLCISHTPHY(wlc_hw->band)) {
			if (D11REV_IS(wlc_hw->corerev, 26))
				wlc_write_inits(wlc_hw, d11ht0initvals26);
			else if (D11REV_IS(wlc_hw->corerev, 29))
				wlc_write_inits(wlc_hw, d11ht0initvals29);
		} else
			WL_ERROR(("wl%d: unsupported phy in corerev 26 \n", wlc_hw->unit));
	} else if (D11REV_IS(wlc_hw->corerev, 25) || D11REV_IS(wlc_hw->corerev, 28)) {
		if (WLCISNPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, d11n0initvals25);
		} else if (WLCISLCNPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, wlc_get_lcn0initvals25_addr());
			if (MBSS_ENAB(wlc->pub)) {
				fifosz_fixup = TRUE;
			}
		} else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
				__FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
	} else if (D11REV_IS(wlc_hw->corerev, 24)) {
		if (WLCISNPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, d11n0initvals24);
		} else if (WLCISLCNPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, wlc_get_lcn0initvals24_addr());
		} else
			WL_ERROR(("wl%d: unsupported phy in corerev 24 \n", wlc_hw->unit));
	} else if (D11REV_GE(wlc_hw->corerev, 22)) {
		if (WLCISNPHY(wlc_hw->band)) {
			/* ucode only supports rev23(43224b0) with rev16 ucode */
			if (D11REV_IS(wlc_hw->corerev, 23))
				wlc_write_inits(wlc_hw, d11n0initvals16);
			else
				wlc_write_inits(wlc_hw, d11n0initvals22);
		} else if (WLCISSSLPNPHY(wlc_hw->band)) {
			if (D11REV_IS(wlc_hw->corerev, 22))
				wlc_write_inits(wlc_hw, d11sslpn4initvals22);
			else
				WL_ERROR(("wl%d: unsupported phy in corerev 16\n", wlc_hw->unit));
		} else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
				__FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
	} else if (D11REV_IS(wlc_hw->corerev, 21)) {
		if (WLCISSSLPNPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, d11sslpn3initvals21);
		}
		else
			WL_ERROR(("wl%d: unsupported phy in corerev 16\n", wlc_hw->unit));
	} else if (D11REV_IS(wlc_hw->corerev, 20)) {
		if (WLCISSSLPNPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, d11sslpn1initvals20);
			WL_ERROR(("wl%d: supported phy in corerev 20\n", wlc_hw->unit));
		}
		else
			WL_ERROR(("wl%d: unsupported phy in corerev 20\n", wlc_hw->unit));
	} else if (D11REV_IS(wlc_hw->corerev, 19)) {
		if (WLCISSSLPNPHY(wlc_hw->band))
			wlc_write_inits(wlc_hw, d11sslpn2initvals19);
		else
			WL_ERROR(("wl%d: unsupported phy in corerev 19\n", wlc_hw->unit));
	} else if (D11REV_GE(wlc_hw->corerev, 16)) {
		if (WLCISNPHY(wlc_hw->band))
			wlc_write_inits(wlc_hw, d11n0initvals16);
		else if (WLCISSSLPNPHY(wlc_hw->band))
			wlc_write_inits(wlc_hw, d11sslpn0initvals16);
		else if (WLCISLPPHY(wlc_hw->band))
			wlc_write_inits(wlc_hw, d11lp0initvals16);
		else
			WL_ERROR(("wl%d: unsupported phy in corerev 16\n", wlc_hw->unit));
	} else if (D11REV_IS(wlc_hw->corerev, 15)) {
		if (WLCISLPPHY(wlc_hw->band))
			wlc_write_inits(wlc_hw, d11lp0initvals15);
		else
			WL_ERROR(("wl%d: unsupported phy in corerev 15\n", wlc_hw->unit));
	} else if (D11REV_IS(wlc_hw->corerev, 14)) {
		if (WLCISLPPHY(wlc_hw->band))
			wlc_write_inits(wlc_hw, d11lp0initvals14);
		else
			WL_ERROR(("wl%d: unsupported phy in corerev 14\n", wlc_hw->unit));
	} else if (D11REV_IS(wlc_hw->corerev, 13)) {
		if (WLCISLPPHY(wlc_hw->band))
			wlc_write_inits(wlc_hw, d11lp0initvals13);
		else if (WLCISGPHY(wlc_hw->band))
			wlc_write_inits(wlc_hw, d11b0g0initvals13);
		else if (WLCISAPHY(wlc_hw->band) && (sflags & SISF_2G_PHY))
			wlc_write_inits(wlc_hw, d11a0g1initvals13);
		else
			WL_ERROR(("wl%d: unsupported phy in corerev 13\n", wlc_hw->unit));
	} else if (D11REV_GE(wlc_hw->corerev, 11)) {
		if (WLCISNPHY(wlc_hw->band))
			wlc_write_inits(wlc_hw, d11n0initvals11);
		else
			WL_ERROR(("wl%d: corerev 11 or 12 && ! NPHY\n", wlc_hw->unit));
#if defined(MBSS)
	} else if (D11REV_IS(wlc_hw->corerev, 9) && ucode9) {
		if (WLCISAPHY(wlc_hw->band)) {
			if (sflags & SISF_2G_PHY)
				wlc_write_inits(wlc_hw, d11a0g1initvals9);
			else
				wlc_write_inits(wlc_hw, d11a0g0initvals9);
		} else
			wlc_write_inits(wlc_hw, d11b0g0initvals9);
#endif // endif
	} else if (D11REV_IS(wlc_hw->corerev, 4)) {
		if (WLCISAPHY(wlc_hw->band))
			wlc_write_inits(wlc_hw, d11a0g0initvals4);
		else
			wlc_write_inits(wlc_hw, d11b0g0initvals4);
	} else {
		if (WLCISAPHY(wlc_hw->band)) {
			if (sflags & SISF_2G_PHY)
				wlc_write_inits(wlc_hw, d11a0g1initvals5);
			else
				wlc_write_inits(wlc_hw, d11a0g0initvals5);
		} else
			wlc_write_inits(wlc_hw, d11b0g0initvals5);
	}
#endif /* BCMUCDOWNLOAD */
	/* For old ucode, txfifo sizes needs to be modified(increased) for Corerev >= 9 */
	if (D11REV_GE(wlc_hw->corerev, 40)) {
		if (wlc_hw->macunit == 0) {
			wlc_bmac_bmc_init(wlc_hw);
		}
	}
	else if (D11REV_LT(wlc_hw->corerev, 40)) {
		if (fifosz_fixup == TRUE) {
			wlc_corerev_fifofixup(wlc_hw);
		}
		wlc_corerev_fifosz_validate(wlc_hw, buf);
	}
	else {
		printf("add support for fifo inits for corerev %d......\n", wlc_hw->corerev);
		ASSERT(0);
	}

	/* make sure we can still talk to the mac */
	ASSERT(R_REG(osh, &regs->maccontrol) != 0xffffffff);

	/* band-specific inits done by wlc_bsinit() */

#ifdef MBSS
	if (MBSS_ENAB(wlc->pub)) {
		/* Set search engine ssid lengths to zero */
		if (D11REV_ISMBSS16(wlc_hw->corerev) &&
		    wlc_bmac_ucodembss_hwcap(wlc_hw) == TRUE) {
			uint32 start, swplen, idx;

			swplen = 0;
			for (idx = 0; idx < (uint) wlc->pub->tunables->maxucodebss; idx++) {

				start = SHM_MBSS_SSIDSE_BASE_ADDR + (idx * SHM_MBSS_SSIDSE_BLKSZ);

				wlc_bmac_copyto_objmem(wlc_hw, start, &swplen,
					SHM_MBSS_SSIDLEN_BLKSZ, OBJADDR_SRCHM_SEL);
			}
		}
	}
#endif /* MBSS */

	/* Set up frame burst size and antenna swap threshold init values */
	wlc_bmac_write_shm(wlc_hw, M_MBURST_SIZE, MAXTXFRAMEBURST);
	wlc_bmac_write_shm(wlc_hw, M_MAX_ANTCNT, ANTCNT);

	/* set intrecvlazy to configured value */
	wlc_bmac_rcvlazy_update(wlc_hw, wlc_hw->intrcvlazy);
	if (D11REV_IS(wlc_hw->corerev, 4))
		W_REG(osh, &regs->intrcvlazy[3], (1 << IRL_FC_SHIFT));

#endif /* BCM_OL_DEV */

	/* set the station mode (BSS STA) */
	wlc_bmac_mctrl(wlc_hw,
	          (MCTL_INFRA | MCTL_DISCARD_PMQ | MCTL_AP),
	          (MCTL_INFRA | MCTL_DISCARD_PMQ));

	if (PIO_ENAB_HW(wlc_hw)) {
		/* set fifo mode for each VALID rx fifo */
		wlc_rxfifo_setpio(wlc_hw);

		for (i = 0; i < NFIFO; i++)
			if (wlc_hw->pio[i])
				wlc_pio_init(wlc_hw->pio[i]);

		/*
		 * For D11 corerev less than 8, the h/w does not store the pad bytes in the
		 * Rx Data FIFO
		*/
		if (D11REV_LT(wlc_hw->corerev, 8))
			wlc_bmac_write_shm(wlc_hw, M_RX_PAD_DATA_OFFSET, 0);

#ifdef IL_BIGENDIAN
		/* enable byte swapping */
		wlc_bmac_mctrl(wlc_hw, MCTL_BIGEND, MCTL_BIGEND);
#endif /* IL_BIGENDIAN */
	}

	/* BMAC_NOTE: Could this just be a ucode init? */
	if (D11REV_ISMBSS4(wlc_hw->corerev)) {
		uint offset = SHM_MBSS_BC_FID0;
		int idx = 0;
		for (idx = 0; idx < wlc->pub->tunables->maxucodebss4; idx++) {
			wlc_bmac_write_shm(wlc_hw, offset, INVALIDFID);
			offset += 2;
		}
	}

	/* set up Beacon interval */
	bcnint_us = 0x8000 << 10;
	W_REG(osh, &regs->tsf_cfprep, (bcnint_us << CFPREP_CBI_SHIFT));
	W_REG(osh, &regs->tsf_cfpstart, bcnint_us);
	SET_MACINTSTATUS(osh, regs, MI_GP1);

	if (D11REV_GE(wlc_hw->corerev, 64)) {
		int idx;
		for (idx = 0; idx < 6; idx++) {
			W_REG(osh, &regs->intctrlregs[idx].intmask, 0);
			W_REG(osh, &regs->altintmask[idx], 0);

			/* When CT mode is enabled, the above writes to intmasks are not effective
			 * since only the indirect registers are active.
			 * Instead use the indirect AQM DMA register access that are used for CT
			 * mode.
			 * Note: do not program inddma.indintstatus and mask for corerev64
			 */
			if (BCM_DMA_CT_ENAB(wlc_hw->wlc) && wlc_hw->aqm_di[idx]) {
				dma_set_indqsel(wlc_hw->aqm_di[idx], FALSE);
				W_REG(osh, &regs->indaqm.indintmask, 0);
			}
		}
	}

	/* write interrupt mask */
#ifdef BCM_OL_DEV
	W_REG(osh, &regs->altintmask[RX_FIFO], DEF_RXINTMASK);
#else
	W_REG(osh, &regs->intctrlregs[RX_FIFO].intmask, DEF_RXINTMASK);
	/* interrupt for second fifo */
	if (PKT_CLASSIFY_EN(RX_FIFO1) || RXFIFO_SPLIT())
		W_REG(osh, &regs->intctrlregs[RX_FIFO1].intmask, DEF_RXINTMASK);

	/* interrupt for third fifo in MODE 3 and MODE 4 */
	if (PKT_CLASSIFY_EN(RX_FIFO2)) {
		W_REG(osh, &regs->intctrlregs[RX_FIFO2].intmask, DEF_RXINTMASK);
	}

	if (D11REV_IS(wlc_hw->corerev, 4))
		W_REG(osh, &regs->intctrlregs[RX_TXSTATUS_FIFO].intmask, DEF_RXINTMASK);
#endif // endif
#ifdef DMATXRC
	if (DMATXRC_ENAB(wlc->pub) && D11REV_GE(wlc_hw->corerev, 40)) {
		W_REG(osh, &regs->intctrlregs[TX_DATA_FIFO].intmask, I_XI);
		wlc->txrc_fifo_mask |= (1 << TX_DATA_FIFO);
	}
#endif // endif

	/* allow the MAC to control the PHY clock (dynamic on/off) */
	wlc_bmac_macphyclk_set(wlc_hw, ON);

	/* program dynamic clock control fast powerup delay register */
	if (D11REV_GT(wlc_hw->corerev, 4)) {
		wlc_hw->fastpwrup_dly = si_clkctl_fast_pwrup_delay(wlc_hw->sih);
		W_REG(osh, &regs->u.d11regs.scc_fastpwrup_dly, wlc_hw->fastpwrup_dly);
		if (D11REV_GT(wlc_hw->corerev, 40)) {
			/* For corerev >= 40, M_UCODE_DBGST is set after
			 * the synthesizer is powered up in wake sequence.
			 * So add the synthpu delay to wait for wake functionality.
			 */
			wlc_hw->fastpwrup_dly += wlc_bmac_synthpu_dly(wlc_hw);
		}
	}

#ifndef BCM_OL_DEV

	/* tell the ucode the corerev */
	wlc_bmac_write_shm(wlc_hw, M_MACHW_VER, (uint16)wlc_hw->corerev);

	/* tell the ucode MAC capabilities */
	if (D11REV_GE(wlc_hw->corerev, 13)) {
		wlc_bmac_write_shm(wlc_hw, M_MACHW_CAP_L, (uint16)(wlc_hw->machwcap & 0xffff));
		wlc_bmac_write_shm(wlc_hw, M_MACHW_CAP_H,
			(uint16)((wlc_hw->machwcap >> 16) & 0xffff));
	}

	/* write retry limits to SCR, this done after PSM init */
	wlc_bmac_copyto_objmem(wlc_hw, S_DOT11_SRC_LMT << 2, &(wlc_hw->SRL),
		sizeof(wlc_hw->SRL), OBJADDR_SCR_SEL);

	wlc_bmac_copyto_objmem(wlc_hw, S_DOT11_LRC_LMT << 2, &(wlc_hw->LRL),
		sizeof(wlc_hw->LRL), OBJADDR_SCR_SEL);

#ifdef STA
	if (wlc->seq_reset) {
		wlc->seq_reset = FALSE;
	} else {
		/* write the previous sequence number, this done after PSM init */
		wlc_bmac_copyto_objmem(wlc->hw, S_SEQ_NUM << 2, &seqnum,
			sizeof(seqnum), OBJADDR_SCR_SEL);
	}
#endif /* STA */

	/* write rate fallback retry limits */
	wlc_bmac_write_shm(wlc_hw, M_SFRMTXCNTFBRTHSD, wlc_hw->SFBL);
	wlc_bmac_write_shm(wlc_hw, M_LFRMTXCNTFBRTHSD, wlc_hw->LFBL);

	/*
	FIXME :Hardware EDCF function is broken now, so we force the AC0 AIFS
	to EDCF_AIFSN_MIN. This can be removed once ucode does it.
	*/
	if (D11REV_GE(wlc_hw->corerev, 16)) {
		AND_REG(osh, &regs->u.d11regs.ifs_ctl, 0x0FFF);
		W_REG(osh, &regs->u.d11regs.ifs_aifsn, EDCF_AIFSN_MIN);
	}
#endif /* BCM_OL_DEV  */

	/* dma or pio initializations */
	if (!PIO_ENAB_HW(wlc_hw)) {
		wlc->txpend16165war = 0;

		/* find out number of FIFOs needed. WL_MU_TX can be dynamically
		 * enabled/disabled. However the resources for all the possible
		 * DMA channels that were attached are always available unless the
		 * driver is unloaded. So instead of checking if the dma handle is null,
		 * the number of fifos in use should be used.
		 */
		if (BCM_DMA_CT_ENAB(wlc_hw->wlc)) {
			wlc_hw_set_nfifo_inuse(wlc_hw, NFIFO_EXT);
		} else {
			wlc_hw_set_nfifo_inuse(wlc_hw, NFIFO);
		}
		WL_ERROR(("wl%d: CORE INIT : nfifo %d mu_tx_enab %d\n",
			wlc_hw->unit, WLC_HW_NFIFO_INUSE(wlc_hw->wlc), MU_TX_ENAB(wlc)));

		/* init the tx dma engines */
		for (i = 0; i < wlc_hw->nfifo_inuse; i++) {
			if (wlc_hw->di[i])
				dma_txinit(wlc_hw->di[i]);
		}

#ifdef BCM_DMA_CT
		if (BCM_DMA_CT_ENAB(wlc_hw->wlc)) {
			/* set M_MU_TESTMODE and TXE_ctmode */
			wlc_bmac_enable_ct_access(wlc_hw, TRUE);
			/* init ct_dma(aqmdma) */
			for (i = 0; i < wlc_hw->nfifo_inuse; i++) {
				if (wlc_hw->aqm_di[i]) {
					dma_txinit(wlc_hw->aqm_di[i]);
					if (wlc->cpbusy_war)
						aqm_dma_init_fifo_stats(wlc_hw->aqm_di[i], i);
				}
			}
		} else {
			wlc_bmac_enable_ct_access(wlc_hw, FALSE);
		}
#endif // endif
		for (i = 0; i < MAX_RX_FIFO; i++) {
			if ((wlc_hw->di[i] != NULL) && wlc_bmac_rxfifo_enab(i)) {
				dma_rxinit(wlc_hw->di[i]);
				dma_rxfill(wlc_hw->di[i]);
			}

		}
		/* Enable Header conversion */
		if (HDR_CONV()) {
			/* enable bit */
			OR_REG(wlc_hw->osh, &regs->u.d11acregs.RCVHdrConvStats, 0x1);

			/* Read current fifo-sel for backup */
			rcv_fifo_ctl = R_REG(wlc_hw->osh, &regs->rcv_fifo_ctl);

			/* select fifo-0 */
			AND_REG(wlc_hw->osh, &regs->rcv_fifo_ctl,
				(uint16)(~(RXFIFO_CTL_FIFOSEL_MASK)));

			/* update fifo-0 status len to be 0x4 */
			W_REG(wlc_hw->osh, &regs->rcv_status_len, 0x4);

			/* Put back fifo ctrl value */
			W_REG(wlc_hw->osh, &regs->rcv_fifo_ctl, rcv_fifo_ctl);
		}

		if (RXFIFO_SPLIT()) {
			/* copy count value */
			W_REG(wlc_hw->osh, &regs->u_rcv.d11acregs.rcv_copcnt_q1, tune->copycount);
#ifdef DONGLEBUILD
			wl_set_copycount_bytes(wlc_hw->wlc->wl, tune->copycount,
				WL_HWRXOFF);
#endif // endif
		}
		if (D11REV_IS(wlc_hw->corerev, 4)) {
			dma_rxinit(wlc_hw->di[RX_TXSTATUS_FIFO]);
			dma_rxfill(wlc_hw->di[RX_TXSTATUS_FIFO]);
		}
	} else {
		for (i = 0; i < NFIFO; i++) {
			uint tmp = 0;
			if (wlc_pio_txdepthget(wlc_hw->pio[i]) == 0) {
				wlc_pio_txdepthset(wlc_hw->pio[i], (buf[i] << 8));

				tmp = wlc_pio_txdepthget(wlc_hw->pio[i]);
				if ((D11REV_LE(wlc_hw->corerev, 7) ||
				     D11REV_GE(wlc_hw->corerev, 11)) && tmp)
					wlc_pio_txdepthset(wlc_hw->pio[i], tmp - 4);
			}
		}
	}

#ifndef BCM_OL_DEV

	if ((CHIPID(wlc_hw->sih->chip) == BCM4716_CHIP_ID) ||
	    (CHIPID(wlc_hw->sih->chip) == BCM4748_CHIP_ID) ||
	    (CHIPID(wlc_hw->sih->chip) == BCM47162_CHIP_ID)) {
		/* The value to be written into these registers is (2^26)/(freq)MHz */
		/* MAC clock frequency for 4716 is 125MHz */
		W_REG(osh, &regs->u.d11regs.tsf_clk_frac_l, 0x3127);
		W_REG(osh, &regs->u.d11regs.tsf_clk_frac_h, 0x8);
	} else if (
		(CHIPID(wlc_hw->sih->chip) == BCM4334_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM4314_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM43142_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM43143_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM43340_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM43341_CHIP_ID) ||
		0) {
		/* The value to be written into these registers is (2^26)/(freq)MHz */
		/* Ex. MAC clock frequency for 4334 is 96MHz = 0xaaaab */
		uint32 val;

		val = (2 << 25)/(si_clock(wlc_hw->sih)/1000000);
		W_REG(osh, &regs->u.d11regs.tsf_clk_frac_l, val & 0xffff);
		W_REG(osh, &regs->u.d11regs.tsf_clk_frac_h, val >> 16);
	} else if (BCM4365_CHIP(wlc_hw->sih->chip)) {
		/* The value to be written into these registers is (2^26)/(freq)MHz */
		/* MAC clock frequency for 4366 is 192.599998MHz */
		W_REG(osh, &regs->u.d11regs.tsf_clk_frac_l, 0x5114);
		W_REG(osh, &regs->u.d11regs.tsf_clk_frac_h, 0x5);
	} else if ((CHIPID(wlc_hw->sih->chip) == BCM4335_CHIP_ID) ||
#ifdef UNRELEASEDCHIP
		BCM4349_CHIP(wlc_hw->sih->chip) ||
#endif /* UNRELEASEDCHIP */
		0)
	{
		wlc_bmac_switch_macfreq(wlc_hw, 0);
	}

	/*
	Set the TA at the appropriate SHM location for CTS2SELF frames
	generated by ucode for AC only
	*/
	if (D11REV_GE(wlc_hw->corerev, 40))
		wlc_bmac_set_myaddr(wlc_hw, &(wlc_hw->etheraddr));

	/* initialize btc_params and btc_flags */
	wlc_bmac_btc_param_init(wlc_hw);
#endif /* BCM_OL_DEV */

#ifdef BCMLTECOEX
	/* config ltecx interface */
	if (BCMLTECOEX_ENAB(wlc->pub))	{
		wlc_ltecx_init(wlc->ltecx);
	}
#endif /* BCMLTECOEX */

#ifdef WLP2P_UCODE
	if (DL_P2P_UC(wlc_hw)) {
		/* enable P2P mode */
		wlc_bmac_mhf(wlc_hw, MHF5, MHF5_P2P_MODE,
		             wlc_hw->_p2p ? MHF5_P2P_MODE : 0, WLC_BAND_ALL);
		/* cache p2p SHM location */
		wlc_hw->p2p_shm_base = wlc_bmac_read_shm(wlc_hw, M_P2P_BLK_PTR) << 1;
	}
#endif // endif
	if (D11REV_LT(wlc_hw->corerev, 40)) {
		wlc_hw->cca_shm_base = M_CCA_STATS_BLK_PRE40;
	} else {
		wlc_hw->cca_shm_base = (wlc_bmac_read_shm(wlc_hw, M_CCASTATS_PTR) << 1);
		wlc_hw->macstat1_shm_base = (wlc_bmac_read_shm(wlc_hw, M_UCODE_MACSTAT1_PTR) << 1);
	}

	/* Shmem pm_dur is reset by ucode as part of auto-init, hence call wlc_reset_accum_pmdur */
	wlc_reset_accum_pmdur(wlc);
	WL_ERROR(("wl%d: CORE INIT : mode %d pktclassify %d rxsplit %d  hdr conve %d DMA_CT %s\n",
		wlc_hw->unit, BCMSPLITRX_MODE(), PKT_CLASSIFY(), RXFIFO_SPLIT(), HDR_CONV(),
		wlc->_dma_ct ? "Enabled":"Disabled"));
}
/** Reset the PM duration accumulator maintained by SW */
static int
wlc_reset_accum_pmdur(wlc_info_t *wlc)
{
	wlc->pm_dur_clear_timeout = TIMEOUT_TO_READ_PM_DUR;
	wlc->wlc_pm_dur_last_sample =
		wlc_bmac_cca_read_counter(wlc->hw, M_MAC_DOZE_L, M_MAC_DOZE_H);
	return BCME_OK;
}

/**
 * On changing the MAC clock frequency, the tsf frac register must be adjusted accordingly.
 * If spur avoidance mode is off, the mac freq will be 80/120/160Mhz
 * If spur avoidance mode is on1, the mac freq will be 82/123/164Mhz
 * If spur avoidance mode is on2, the mac freq will be 84/126/168Mhz
 * Formula is 2^26/freq(MHz)
 */
void
wlc_bmac_switch_macfreq(wlc_hw_info_t *wlc_hw, uint8 spurmode)
{
	d11regs_t *regs;
	osl_t *osh;

	/* this function is called only by AC, N, LCN and HT PHYs */
	ASSERT(WLCISNPHY(wlc_hw->band) || WLCISLCNPHY(wlc_hw->band) ||
		WLCISHTPHY(wlc_hw->band) || WLCISACPHY(wlc_hw->band));

	regs = wlc_hw->regs;
	osh = wlc_hw->osh;

	/* ??? better keying, corerev, phyrev ??? */
	if ((CHIPID(wlc_hw->sih->chip) == BCM4331_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM43431_CHIP_ID)) {
		if (spurmode == WL_SPURAVOID_ON2) { /* 168MHz */
			W_REG(osh, &regs->u.d11regs.tsf_clk_frac_l, 0x1862);
			W_REG(osh, &regs->u.d11regs.tsf_clk_frac_h, 0x6);
		} else if (spurmode == WL_SPURAVOID_ON1) { /* 164MHz */
			W_REG(osh, &regs->u.d11regs.tsf_clk_frac_l, 0x3E70);
			W_REG(osh, &regs->u.d11regs.tsf_clk_frac_h, 0x6);
		} else { /* 160MHz */
			W_REG(osh, &regs->u.d11regs.tsf_clk_frac_l, 0x6666);
			W_REG(osh, &regs->u.d11regs.tsf_clk_frac_h, 0x6);
		}
	} else if ((CHIPID(wlc_hw->sih->chip) == BCM43222_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM43420_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM43111_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM43112_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM43224_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM43225_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM43421_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM43226_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM43131_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM43217_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM43227_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM43228_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM43428_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM43242_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM43243_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM43234_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM43235_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM43236_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM43238_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM43237_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM6362_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM5357_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM4345_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM4749_CHIP_ID)) {
		if (spurmode == WL_SPURAVOID_ON2) {	/* 126Mhz */
			W_REG(osh, &regs->u.d11regs.tsf_clk_frac_l, 0x2082);
			W_REG(osh, &regs->u.d11regs.tsf_clk_frac_h, 0x8);
		} else if (spurmode == WL_SPURAVOID_ON1) {	/* 123Mhz */
			W_REG(osh, &regs->u.d11regs.tsf_clk_frac_l, 0x5341);
			W_REG(osh, &regs->u.d11regs.tsf_clk_frac_h, 0x8);
		} else {	/* 120Mhz */
			W_REG(osh, &regs->u.d11regs.tsf_clk_frac_l, 0x8889);
			W_REG(osh, &regs->u.d11regs.tsf_clk_frac_h, 0x8);
		}
	} else if (CHIPID(wlc_hw->sih->chip) == BCM4335_CHIP_ID ||
#ifdef UNRELEASEDCHIP
		BCM4349_CHIP(wlc_hw->sih->chip) ||
#endif /* UNRELEASEDCHIP */
		0)
	{
		uint32 mac_clk;
		uint32 clk_frac;
		uint16 frac_l, frac_h;
		uint32 r_high, r_low;

		mac_clk = si_mac_clk(wlc_hw->sih, wlc_hw->osh);
#ifdef WL11ULB
		/* MAC Clock is adjusted based on Multiplication factor before configuring TSF
		 * Clock Frac Registers. (incase MAC has been down-clocked for ULB Operation)
		 */
		mac_clk <<= si_get_macclk_mul_fact(wlc_hw->sih);
#endif /* WL11ULB */

		/* the mac_clk is scaled by 1000 */
		/* so, multiplier for numerator will be 1 / (mac_clk / 1000): 1000 */
		bcm_uint64_multiple_add(&r_high, &r_low, (1 << 26), 1000, (mac_clk >> 1));
		bcm_uint64_divide(&clk_frac, r_high, r_low, mac_clk);

		frac_l =  (uint16)(clk_frac & 0xffff);
		frac_h =  (uint16)((clk_frac >> 16) & 0xffff);

		W_REG(osh, &regs->u.d11regs.tsf_clk_frac_l, frac_l);
		W_REG(osh, &regs->u.d11regs.tsf_clk_frac_h, frac_h);

	} else if ((CHIPID(wlc_hw->sih->chip) == BCM4360_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM43460_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM43526_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM4352_CHIP_ID) ||
		(CHIPID(wlc_hw->sih->chip) == BCM43602_CHIP_ID) ||
		BCM4350_CHIP(wlc_hw->sih->chip) ||
		0) {
		/*
		 * PR115835: Whenever driver changes BBPLL frequency it needs to change MAC clock
		 * frequency as well.
		 * mac_freq = bbpll_freq / 6.0
		 * clk_frac = (8.0/mac_freq) * 2^23
		 */
		uint32 bbpll_freq, clk_frac;

		/*
		 * XXX: This assumes xtalfreq is 40Mhz; Need fix for 37.4Mhz
		 * XXX: For 4350 and dongle, use ucode default for now
		 */
		if (BCM4350_CHIP(wlc_hw->sih->chip) &&
			(BUSTYPE(wlc_hw->sih->bustype) == SI_BUS)) {
			WL_ERROR(("%s: 4350 need fix for 37.4Mhz\n", __FUNCTION__));
			return;
		}

		bbpll_freq = si_pmu_get_bb_vcofreq(wlc_hw->sih, osh, 40); /* in [100Hz] units */

		/* 6 * 8 * 10000 * 2^23 = 0x3A980000000 */
		bcm_uint64_divide(&clk_frac, 0x3A9, 0x80000000, bbpll_freq);

		W_REG(osh, &regs->u.d11regs.tsf_clk_frac_l, clk_frac & 0xffff);
		W_REG(osh, &regs->u.d11regs.tsf_clk_frac_h, (clk_frac >> 16) & 0xffff);

	} else if (WLCISLCNPHY(wlc_hw->band)) {
		if (spurmode == WL_SPURAVOID_ON1) {	/* 82Mhz */
			W_REG(osh, &regs->u.d11regs.tsf_clk_frac_l, 0x7CE0);
			W_REG(osh, &regs->u.d11regs.tsf_clk_frac_h, 0xC);
		} else {	/* 80Mhz */
			W_REG(osh, &regs->u.d11regs.tsf_clk_frac_l, 0xCCCD);
			W_REG(osh, &regs->u.d11regs.tsf_clk_frac_h, 0xC);
		}
	}
} /* wlc_bmac_switch_macfreq */

#ifndef BCM_OL_DEV
/** Initialize GPIOs that are controlled by D11 core */
static void
BCMINITFN(wlc_gpio_init)(wlc_hw_info_t *wlc_hw)
{
	d11regs_t *regs = wlc_hw->regs;
	uint32 gc, gm;
	osl_t *osh = wlc_hw->osh;
	/* use GPIO select 0 to get all gpio signals from the gpio out reg */
	wlc_bmac_mctrl(wlc_hw, MCTL_GPOUT_SEL_MASK, 0);

	/*
	 * Common GPIO setup:
	 *	G0 = LED 0 = WLAN Activity
	 *	G1 = LED 1 = WLAN 2.4 GHz Radio State
	 *	G2 = LED 2 = WLAN 5 GHz Radio State
	 *	G4 = radio disable input (HI enabled, LO disabled)
	 * Boards that support BT Coexistence:
	 *	G7 = BTC
	 *	G8 = BTC
	 * Boards with chips that have fewer gpios and support BT Coexistence:
	 *	G4 = BTC
	 *	G5 = BTC
	 */

	gc = gm = 0;

	/* Set/clear GPIOs for BTC */
	if (wlc_hw->btc->gpio_out != 0)
		wlc_bmac_btc_gpio_enable(wlc_hw);

#ifdef WL11N
	/* Allocate GPIOs for mimo antenna diversity feature */
	if (WLANTSEL_ENAB(wlc)) {
		if (wlc_hw->antsel_type == ANTSEL_2x3 || wlc_hw->antsel_type == ANTSEL_1x2_CORE1 ||
			wlc_hw->antsel_type == ANTSEL_1x2_CORE0) {
			/* Enable antenna diversity, use 2x3 mode */
			wlc_bmac_mhf(wlc_hw, MHF3, MHF3_ANTSEL_EN, MHF3_ANTSEL_EN, WLC_BAND_ALL);
			wlc_bmac_mhf(wlc_hw, MHF3, MHF3_ANTSEL_MODE, MHF3_ANTSEL_MODE,
				WLC_BAND_ALL);

			/* init superswitch control */
			wlc_phy_antsel_init(wlc_hw->band->pi, FALSE);

		} else if (wlc_hw->antsel_type == ANTSEL_2x4) {
			/* XXX GPIO 8 is also defined for BTC_OUT.
			* Just make sure that we don't conflict
			*/
			ASSERT((gm & BOARD_GPIO_12) == 0);
			gm |= gc |= (BOARD_GPIO_12 | BOARD_GPIO_13);
			/* The board itself is powered by these GPIOs (when not sending pattern)
			* So set them high
			*/
			OR_REG(osh, &regs->psm_gpio_oe, (BOARD_GPIO_12 | BOARD_GPIO_13));
			OR_REG(osh, &regs->psm_gpio_out, (BOARD_GPIO_12 | BOARD_GPIO_13));

			/* Enable antenna diversity, use 2x4 mode */
			wlc_bmac_mhf(wlc_hw, MHF3, MHF3_ANTSEL_EN, MHF3_ANTSEL_EN, WLC_BAND_ALL);
			wlc_bmac_mhf(wlc_hw, MHF3, MHF3_ANTSEL_MODE, 0, WLC_BAND_ALL);

			/* Configure the desired clock to be 4Mhz */
			wlc_bmac_write_shm(wlc_hw, M_ANTSEL_CLKDIV, ANTSEL_CLKDIV_4MHZ);
		}
	}
#endif /* WL11N */
	/* gpio 9 controls the PA.  ucode is responsible for wiggling out and oe */
	if (wlc_hw->boardflags & BFL_PACTRL)
		gm |= gc |= BOARD_GPIO_PACTRL;

	if (((wlc_hw->sih->boardtype == BCM94322MC_SSID) ||
	     (wlc_hw->sih->boardtype == BCM94322HM_SSID)) &&
	    ((wlc_hw->boardrev & 0xfff) >= 0x200) &&
	    ((CHIPID(wlc_hw->sih->chip) == BCM4322_CHIP_ID) &&
	     (CHIPREV(wlc_hw->sih->chiprev) == 0))) {

		gm |= gc |= BOARD_GPIO_12;
		OR_REG(osh, &regs->psm_gpio_oe, (BOARD_GPIO_12));
		AND_REG(osh, &regs->psm_gpio_out, ~BOARD_GPIO_12);
	}

	/* gpio 14(Xtal_up) and gpio 15(PLL_powerdown) are controlled in PCI config space */

	/* config dual wlan radio coex function in bmac driver and monolithic driver */
#ifdef WLC_LOW_ONLY
	/* wlancoex slave gpio init */
	if (getvar(wlc_hw->vars, "wlancoex_s") != NULL) {
		char *var = getvar(wlc_hw->vars, "wlancoex_s");
		uint val = (1 << bcm_strtoul(var, NULL, 0));
		gm |= gc |= val;
		/* setup gpio as input */
		AND_REG(osh, &regs->psm_gpio_oe, ~val);
	}
#endif /* WLC_LOW_ONLY */
#ifdef USBAP
	/* wlancoex master gpio init */
	if (getvar(wlc_hw->vars, "wlancoex_m") != NULL) {
		char *var = getvar(wlc_hw->vars, "wlancoex_m");
		uint val = (1 << bcm_strtoul(var, NULL, 0));
		gm |= gc |= val;
		/* setup output_enable */
		OR_REG(osh, &regs->psm_gpio_oe, val);
	}
#endif /* USBAP */

	WL_INFORM(("wl%d: gpiocontrol mask 0x%x value 0x%x\n", wlc_hw->unit, gm, gc));

	/* apply to gpiocontrol register */
	si_gpiocontrol(wlc_hw->sih, gm, gc, GPIO_DRV_PRIORITY);
}

#endif /* BCM_OL_DEV */

#ifdef BCM_HOST_MEM_UCODE
void
wlc_bmac_ucode_copy2host(wlc_info_t *wlc, uint32 *hostmem_addr, uint32 hostmem_len)
{
	uint32 ucode_totsize = 0;
	uint32 ucode_size = 0, ucodex_size = 0, ucode_mu_size = 0, ucodex_mu_size = 0;
	const uint32 *d11ucode, *d11ucodex, *d11ucode_mu, *d11ucodex_mu;
	uint32 *host_addr = hostmem_addr;
	wlc_hw_info_t *wlc_hw = wlc->hw;

	ASSERT(hostmem_len > 0);

	/* Do not proceed if ucodes are already uploaded */
	if (host_d11ucode) {
		return;
	}

	/* Only support corerev 64 and 65 for now */
	if (D11REV_IS(wlc_hw->corerev, 65)) {
#if defined(BTCX_ENABLED)
		d11ucode = d11ucode_btcx65;
		ucode_size = d11ucode_btcx65sz;
		d11ucodex = d11ucodex_btcx65;
		ucodex_size = d11ucodex_btcx65sz;
		d11ucode_mu = d11ucode_btcx_mu65;
		ucode_mu_size = d11ucode_btcx_mu65sz;
		d11ucodex_mu = d11ucodex_btcx_mu65;
		ucodex_mu_size = d11ucodex_btcx_mu65sz;
		WL_ERROR(("%s: wl%d: btcx ucode configured\n", __FUNCTION__, wlc_hw->unit));
#else
		d11ucode = d11ucode65;
		ucode_size = d11ucode65sz;
		d11ucodex = d11ucodex65;
		ucodex_size = d11ucodex65sz;
		d11ucode_mu = d11ucode_mu65;
		ucode_mu_size = d11ucode_mu65sz;
		d11ucodex_mu = d11ucodex_mu65;
		ucodex_mu_size = d11ucodex_mu65sz;
		WL_ERROR(("%s: wl%d: Regular ucode configured\n", __FUNCTION__, wlc_hw->unit));
#endif /* BTCX_ENABLED */
	}
	else if (D11REV_IS(wlc_hw->corerev, 64)) {
#if defined(BTCX_ENABLED)
#ifdef WLCX_ATLAS
			d11ucode = d11ucode_wlcx64;
			ucode_size = d11ucode_wlcx64sz;
			d11ucodex = d11ucodex64;
			ucodex_size = d11ucodex64sz;
#else
			d11ucode = d11ucode_btcx64;
			ucode_size = d11ucode_btcx64sz;
			d11ucodex = d11ucodex_btcx64;
			ucodex_size = d11ucodex_btcx64sz;
#endif /* WLCX_ATLAS */
			d11ucode_mu = d11ucode_btcx_mu64;
			ucode_mu_size = d11ucode_btcx_mu64sz;
			d11ucodex_mu = d11ucodex_btcx_mu64;
			ucodex_mu_size = d11ucodex_btcx_mu64sz;
			WL_ERROR(("%s: wl%d: btcx ucode configured\n", __FUNCTION__, wlc_hw->unit));
#else
#ifdef WLCX_ATLAS
			d11ucode = d11ucode_wlcx64;
			ucode_size = d11ucode_wlcx64sz;
#else
			d11ucode = d11ucode64;
			ucode_size = d11ucode64sz;
#endif /* WLCX_ATLAS */
			d11ucodex = d11ucodex64;
			ucodex_size = d11ucodex64sz;
			d11ucode_mu = d11ucode_mu64;
			ucode_mu_size = d11ucode_mu64sz;
			d11ucodex_mu = d11ucodex_mu64;
			ucodex_mu_size = d11ucodex_mu64sz;
			WL_ERROR(("%s: wl%d: Regular ucode configured\n",
				__FUNCTION__, wlc_hw->unit));
#endif /* BTCX_ENABLED */
	}

	ucode_totsize = ROUNDUP(ucode_size, 16) + ROUNDUP(ucodex_size, 16) +
		ROUNDUP(ucode_mu_size, 16) + ROUNDUP(ucodex_mu_size, 16);

	/* Do not proceed if corerev is not supported or not enough host memory */
	if (ucode_totsize == 0 || hostmem_len < ucode_totsize) {
		return;
	}

	memcpy(host_addr, d11ucode, ucode_size);
	host_d11ucode = host_addr;
	host_d11ucodesz = ucode_size;
	*((uint32 *)&host_addr) += ROUNDUP(ucode_size, 16);

	memcpy(host_addr, d11ucodex, ucodex_size);
	host_d11ucodex = host_addr;
	host_d11ucodexsz = ucodex_size;
	*((uint32 *)&host_addr) += ROUNDUP(ucodex_size, 16);

	memcpy(host_addr, d11ucode_mu, ucode_mu_size);
	host_d11ucode_mu = host_addr;
	host_d11ucode_musz = ucode_mu_size;
	*((uint32 *)&host_addr) += ROUNDUP(ucode_mu_size, 16);

	memcpy(host_addr, d11ucodex_mu, ucodex_mu_size);
	host_d11ucodex_mu = host_addr;
	host_d11ucodex_musz = ucodex_mu_size;

	WL_INFORM(("%s: wl%d: host SU ucode addr %p len %u\n",
		__FUNCTION__, wlc_hw->unit, host_d11ucode, host_d11ucodesz));
	WL_INFORM(("%s: wl%d: host SU ucodex addr %p len %u\n",
		__FUNCTION__, wlc_hw->unit, host_d11ucodex, host_d11ucodexsz));
	WL_INFORM(("%s: wl%d: host MU ucode addr %p len %u\n",
		__FUNCTION__, wlc_hw->unit, host_d11ucode_mu, host_d11ucode_musz));
	WL_INFORM(("%s: wl%d: host MU ucodex addr %p len %u\n",
		__FUNCTION__, wlc_hw->unit, host_d11ucodex_mu, host_d11ucodex_musz));
}
#endif /* BCM_HOST_MEM_UCODE */

#ifndef BCMUCDOWNLOAD
static void
BCMINITFN(wlc_ucode_download)(wlc_hw_info_t *wlc_hw)
{
#if defined(BCM_DMA_CT) && !defined(BCM_DMA_CT_DISABLED)
	int load_mu_ucode = 0;
	int ctdma = 0;

	if (wlc_hw->wlc) {
		ctdma = BCM_DMA_CT_ENAB(wlc_hw->wlc);
	} else {
		if (D11REV_GE(wlc_hw->corerev, 64)) {
			ctdma = (getintvar(NULL, "ctdma") == 1);

			if ((BUSTYPE(wlc_hw->sih->bustype) == SI_BUS) &&
				D11REV_IS(wlc_hw->corerev, 65) &&
				(getvar(NULL, "ctdma") == NULL))
				ctdma = TRUE;
		}
	}
	if (wlc_hw->wlc && (wlc_hw->wlc->pub->mu_features & MU_FEATURES_MUTX) && ctdma) {
		load_mu_ucode = 1;
	}
#else
	if (wlc_hw->ucode_loaded) {
		WL_ERROR(("%s: wl%d: Ucode already loaded \n", __FUNCTION__, wlc_hw->unit));
		return;
	}

#endif /* defined(BCM_DMA_CT) && !defined(BCM_DMA_CT_DISABLED) */

#if defined(WLP2P_UCODE)
	if (DL_P2P_UC(wlc_hw)) {
		const uint32 *ucode32 = NULL;
		const uint8 *ucode8 = NULL;
		uint nbytes = 0;
#if defined(WL_PSMX)
		const uint32 *ucodex32 = NULL;
		uint nbytes_x = 0;
#endif // endif

		if (WLCISACPHY(wlc_hw->band)) {
#ifdef UNRELEASEDCHIP
			if (D11REV_IS(wlc_hw->corerev, 50)) {
				ucode32 = d11ucode_p2p50;
				nbytes = d11ucode_p2p50sz;
			} else
#endif /* UNRELEASEDCHIP */
			if (D11REV_IS(wlc_hw->corerev, 65)) {
#if defined(BTCX_ENABLED)
				ucode32 = d11ucode_btcx_p2p65;
				nbytes = d11ucode_btcx_p2p65sz;
				WL_ERROR(("%s: wl%d: downloading d11ucode_btcx_p2p65\n",
					__FUNCTION__, wlc_hw->unit));
#else
				ucode32 = d11ucode_p2p65;
				nbytes = d11ucode_p2p65sz;
				WL_ERROR(("%s: wl%d:  downloading d11ucode_p2p65\n",
					__FUNCTION__, wlc_hw->unit));
#endif /* BTCX_ENABLED */
#if defined(WL_PSMX)
#if defined(BTCX_ENABLED)
				ucodex32 = d11ucodex_btcx65;
				nbytes_x = d11ucodex_btcx65sz;
				WL_ERROR(("%s: wl%d: downloading d11ucodex_btcx65\n",
					__FUNCTION__, wlc_hw->unit));
#else
				ucodex32 = d11ucodex65;
				nbytes_x = d11ucodex65sz;
				WL_ERROR(("%s: wl%d: downloading d11ucodex_p2p65\n",
					__FUNCTION__, wlc_hw->unit));
#endif /* BTCX_ENABLED */
#endif /* WL_PSMX */
			} else if (D11REV_IS(wlc_hw->corerev, 64)) {
#if defined(BTCX_ENABLED)
				ucode32 = d11ucode_btcx_p2p64;
				nbytes = d11ucode_btcx_p2p64sz;
				WL_ERROR(("%s: wl%d: downloading d11ucode_btcx_p2p64\n",
					__FUNCTION__, wlc_hw->unit));
#else
				ucode32 = d11ucode_p2p64;
				nbytes = d11ucode_p2p64sz;
				WL_ERROR(("%s: wl%d:  downloading d11ucode_p2p64\n",
					__FUNCTION__, wlc_hw->unit));
#endif /* BTCX_ENABLED */
#if defined(WL_PSMX)
#if defined(BTCX_ENABLED)
				ucodex32 = d11ucodex_btcx64;
				nbytes_x = d11ucodex_btcx64sz;
				WL_ERROR(("%s: wl%d: downloading d11ucodex_btcx64\n",
					__FUNCTION__, wlc_hw->unit));
#else
				ucodex32 = d11ucodex64;
				nbytes_x = d11ucodex64sz;
				WL_ERROR(("%s: wl%d: downloading d11ucodex_p2p64\n",
					__FUNCTION__, wlc_hw->unit));
#endif /* BTCX_ENABLED */
#endif /* WL_PSMX */
			} else if (D11REV_IS(wlc_hw->corerev, 49)) {
				ucode32 = d11ucode_p2p49;
				nbytes = d11ucode_p2p49sz;
			} else if (D11REV_IS(wlc_hw->corerev, 48)) {
				ucode32 = d11ucode_p2p48;
				nbytes = d11ucode_p2p48sz;
			} else if (D11REV_IS(wlc_hw->corerev, 45) ||
			           D11REV_IS(wlc_hw->corerev, 47) ||
			           D11REV_IS(wlc_hw->corerev, 51) ||
			           D11REV_IS(wlc_hw->corerev, 52)) {
				ucode32 = d11ucode_p2p47;
				nbytes = d11ucode_p2p47sz;
			} else if (D11REV_IS(wlc_hw->corerev, 46)) {
				ucode32 = d11ucode_p2p46;
				nbytes = d11ucode_p2p46sz;
			} else if (D11REV_IS(wlc_hw->corerev, 43)) {
				ucode32 = d11ucode_p2p43;
				nbytes = d11ucode_p2p43sz;
			} else if (D11REV_IS(wlc_hw->corerev, 42)) {
				ucode32 = d11ucode_p2p42;
				nbytes = d11ucode_p2p42sz;
			} else if (D11REV_IS(wlc_hw->corerev, 41) ||
			           D11REV_IS(wlc_hw->corerev, 44)) {
				ucode32 = d11ucode_p2p41;
				nbytes = d11ucode_p2p41sz;
			} else if (D11REV_IS(wlc_hw->corerev, 40)) {
				ucode32 = d11ucode_p2p40;
				nbytes = d11ucode_p2p40sz;
			} else {
				/* not supported yet */
				WL_ERROR(("no p2p ucode for rev %d\n", wlc_hw->corerev));
				ASSERT(0);
				return;
			}
		} else if (WLCISHTPHY(wlc_hw->band)) {
			if (D11REV_IS(wlc_hw->corerev, 26)) {
				ucode32 = d11ucode_p2p26_mimo;
				nbytes = d11ucode_p2p26_mimosz;
			} else if (D11REV_IS(wlc_hw->corerev, 29)) {
				ucode32 = d11ucode_p2p29_mimo;
				nbytes = d11ucode_p2p29_mimosz;
			}
		} else if (WLCISNPHY(wlc_hw->band)) {
			if (D11REV_IS(wlc_hw->corerev, 37)) {
#if defined WLP2P_DISABLED
				/* Temporary hack to use non p2p ucode */
				ucode32 = d11ucode31_mimo;
				nbytes = d11ucode31_mimosz;
#else
				ucode32 = d11ucode_p2p31_mimo;
				nbytes = d11ucode_p2p31_mimosz;
#endif // endif
			} else if (D11REV_IS(wlc_hw->corerev, 36)) {
#if defined WLP2P_DISABLED
				/* Temporary hack to use non p2p ucode */
				ucode32 = d11ucode36_mimo;
				nbytes = d11ucode36_mimosz;
#else
				ucode32 = d11ucode_p2p36_mimo;
				nbytes = d11ucode_p2p36_mimosz;
#endif // endif
			} else if (D11REV_IS(wlc_hw->corerev, 34)) {
				ucode32 = d11ucode_p2p34_mimo;
				nbytes = d11ucode_p2p34_mimosz;
			} else if (D11REV_IS(wlc_hw->corerev, 32)) {
				ucode32 = d11ucode_p2p32_mimo;
				nbytes = d11ucode_p2p32_mimosz;
			} else if (D11REV_IS(wlc_hw->corerev, 31)) {
				ucode32 = d11ucode_p2p29_mimo;
				nbytes = d11ucode_p2p29_mimosz;
			} else if (D11REV_IS(wlc_hw->corerev, 30)) {
				ucode32 = d11ucode_p2p30_mimo;
				nbytes = d11ucode_p2p30_mimosz;
			} else if (D11REV_IS(wlc_hw->corerev, 25) ||
				D11REV_IS(wlc_hw->corerev, 28)) {
				ucode32 = d11ucode_p2p25_mimo;
				nbytes = d11ucode_p2p25_mimosz;
			} else if (D11REV_IS(wlc_hw->corerev, 24)) {
				ucode32 = d11ucode_p2p24_mimo;
				nbytes = d11ucode_p2p24_mimosz;
			} else if (D11REV_IS(wlc_hw->corerev, 22)) {
				ucode32 = d11ucode_p2p22_mimo;
				nbytes = d11ucode_p2p22_mimosz;
			} else if (D11REV_GE(wlc_hw->corerev, 16)) {
				/* ucode only supports rev23(43224b0) with rev16 ucode */
				ucode32 = d11ucode_p2p16_mimo;
				nbytes = d11ucode_p2p16_mimosz;
			}
		} else if (WLCISLCNPHY(wlc_hw->band)) {
			if (D11REV_IS(wlc_hw->corerev, 25)) {
				ucode8 = d11ucode_p2p25_lcn;
				nbytes = d11ucode_p2p25_lcnsz;
			} else if (D11REV_IS(wlc_hw->corerev, 24)) {
				ucode8 = d11ucode_p2p24_lcn;
				nbytes = d11ucode_p2p24_lcnsz;
			}
		} else if (WLCISSSLPNPHY(wlc_hw->band)) {
			if (D11REV_GE(wlc_hw->corerev, 24)) {
				ucode8 = d11ucode_p2p20_sslpn;
				nbytes = d11ucode_p2p20_sslpnsz;
			} else if (D11REV_GE(wlc_hw->corerev, 16)) {
				ucode8 = d11ucode_p2p16_sslpn;
				nbytes = d11ucode_p2p16_sslpnsz;
			}
		} else if (WLCISLPPHY(wlc_hw->band)) {
			if (D11REV_GE(wlc_hw->corerev, 16)) {
				ucode32 = d11ucode_p2p16_lp;
				nbytes = d11ucode_p2p16_lpsz;
			}
			else if (D11REV_IS(wlc_hw->corerev, 15)) {
				ucode32 = d11ucode_p2p15;
				nbytes = d11ucode_p2p15sz;
			}
		} else if (WLCISLCN40PHY(wlc_hw->band)) {
			if (D11REV_IS(wlc_hw->corerev, 38)) {
#ifdef WLTEST
				ucode8 = d11ucode38_lcn40;
				nbytes = d11ucode38_lcn40sz;
#else
				ucode8 = d11ucode_p2p38_lcn40;
				nbytes = d11ucode_p2p38_lcn40sz;
#endif // endif
			} else if (D11REV_IS(wlc_hw->corerev, 37)) {
#ifdef WLTEST
				ucode8 = d11ucode37_lcn40;
				nbytes = d11ucode37_lcn40sz;
#else
				ucode8 = d11ucode_p2p37_lcn40;
				nbytes = d11ucode_p2p37_lcn40sz;
#endif // endif
			} else if (D11REV_IS(wlc_hw->corerev, 33)) {
				ucode8 = d11ucode_p2p33_lcn40;
				nbytes = d11ucode_p2p33_lcn40sz;
			}
		}
		if (ucode32 != NULL) {
			wlc_ucode_write(wlc_hw, ucode32, nbytes);
#if defined(WL_PSMX)
		if (ucodex32 != NULL) {
			wlc_ucodex_write(wlc_hw, ucodex32, nbytes_x);
		}
#endif // endif
		}
		else if (ucode8 != NULL)
			wlc_ucode_write_byte(wlc_hw, ucode8, nbytes);
		else {
			WL_ERROR(("%s: wl%d: unsupported phy %d in corerev %d for P2P\n",
			          __FUNCTION__, wlc_hw->unit, wlc_hw->band->phytype,
			          wlc_hw->corerev));
			return;
		}
	}
	else
#endif /* WLP2P_UCODE */
	if (D11REV_IS(wlc_hw->corerev, 65)) {
		if (WLCISACPHY(wlc_hw->band)) {
#if defined(BCM_DMA_CT) && !defined(BCM_DMA_CT_DISABLED)
			if (load_mu_ucode) {
				WL_ERROR(("%s: wl%d: Loading MU ucode\n",
					__FUNCTION__, wlc_hw->unit));
#if defined(BCM_HOST_MEM_UCODE)
				if (host_d11ucode_mu) {
					wlc_ucode_write(wlc_hw, host_d11ucode_mu,
						host_d11ucode_musz);
					wlc_ucodex_write(wlc_hw, host_d11ucodex_mu,
						host_d11ucodex_musz);
				} else
#endif /* BCM_HOST_MEM_UCODE */
				{
#if defined(BTCX_ENABLED)
					wlc_ucode_write(wlc_hw, d11ucode_btcx_mu65,
							d11ucode_btcx_mu65sz);
					wlc_ucodex_write(wlc_hw, d11ucodex_btcx_mu65,
							d11ucodex_btcx_mu65sz);
					WL_ERROR(("%s: wl%d: downloading d11ucode_btcx_mu65\n",
						__FUNCTION__, wlc_hw->unit));
#else
					wlc_ucode_write(wlc_hw, d11ucode_mu65, d11ucode_mu65sz);
					wlc_ucodex_write(wlc_hw, d11ucodex_mu65, d11ucodex_mu65sz);
					WL_ERROR(("%s: wl%d: downloading d11ucode_mu65\n",
						__FUNCTION__, wlc_hw->unit));
#endif /* BTCX_ENABLED */
				}
			} else
#endif /* BCM_DMA_CT && !BCM_DMA_CT_DISABLED */
			{
				WL_ERROR(("%s: wl%d: Loading non-MU ucode\n",
					__FUNCTION__, wlc_hw->unit));
#if defined(BCM_HOST_MEM_UCODE)
				if (host_d11ucode) {
					wlc_ucode_write(wlc_hw, host_d11ucode, host_d11ucodesz);
					wlc_ucodex_write(wlc_hw, host_d11ucodex, host_d11ucodexsz);
				} else
#endif /* BCM_HOST_MEM_UCODE */
				{
#if defined(BTCX_ENABLED)
					wlc_ucode_write(wlc_hw, d11ucode_btcx65, d11ucode_btcx65sz);
					wlc_ucodex_write(wlc_hw, d11ucodex_btcx65,
							d11ucodex_btcx65sz);
					WL_ERROR(("%s: wl%d: downloading d11ucode_btcx65\n",
						__FUNCTION__, wlc_hw->unit));
#else
					wlc_ucode_write(wlc_hw, d11ucode65, d11ucode65sz);
					wlc_ucodex_write(wlc_hw, d11ucodex65, d11ucodex65sz);
					WL_ERROR(("%s: wl%d: downloading d11ucode65\n",
						__FUNCTION__, wlc_hw->unit));
#endif /* BTCX_ENABLED */
				}
			}
		} else {
			WL_ERROR(("%s: wl%d: unsupported phy in corerev 65\n",
				__FUNCTION__, wlc_hw->unit));
		}
	} else if (D11REV_IS(wlc_hw->corerev, 64)) {
		if (WLCISACPHY(wlc_hw->band)) {
#if defined(BCM_DMA_CT) && !defined(BCM_DMA_CT_DISABLED)
			if (load_mu_ucode) {
				WL_ERROR(("%s: wl%d: Loading MU ucode\n",
					__FUNCTION__, wlc_hw->unit));
#if defined(BCM_HOST_MEM_UCODE)
				if (host_d11ucode_mu) {
					wlc_ucode_write(wlc_hw, host_d11ucode_mu,
						host_d11ucode_musz);
					wlc_ucodex_write(wlc_hw, host_d11ucodex_mu,
						host_d11ucodex_musz);
				} else
#endif /* BCM_HOST_MEM_UCODE */
				{
#if defined(BTCX_ENABLED)
					wlc_ucode_write(wlc_hw, d11ucode_btcx_mu64,
							d11ucode_btcx_mu64sz);
					wlc_ucodex_write(wlc_hw, d11ucodex_btcx_mu64,
							d11ucodex_btcx_mu64sz);
					WL_ERROR(("%s: wl%d: downloading d11ucode_btcx_mu64\n",
						__FUNCTION__, wlc_hw->unit));
#else
					wlc_ucode_write(wlc_hw, d11ucode_mu64, d11ucode_mu64sz);
					wlc_ucodex_write(wlc_hw, d11ucodex_mu64, d11ucodex_mu64sz);
					WL_ERROR(("%s: wl%d: downloading d11ucode_mu64\n",
						__FUNCTION__, wlc_hw->unit));
#endif /* BTCX_ENABLED */
				}
			} else
#endif /* BCM_DMA_CT && !BCM_DMA_CT_DISABLED */
			{
				WL_ERROR(("%s: wl%d: Loading non-MU ucode\n",
					__FUNCTION__, wlc_hw->unit));
#if defined(BCM_HOST_MEM_UCODE)
				if (host_d11ucode) {
					wlc_ucode_write(wlc_hw, host_d11ucode, host_d11ucodesz);
					wlc_ucodex_write(wlc_hw, host_d11ucodex, host_d11ucodexsz);
				} else
#endif /* BCM_HOST_MEM_UCODE */
				{
#ifdef WLCX_ATLAS
#if defined(BTCX_ENABLED)
					WL_ERROR(("%s: wl%d: BTCX and WLCX can't coexist\n",
						__FUNCTION__, wlc_hw->unit));
#endif /* BTCX_ENABLED */
					wlc_ucode_write(wlc_hw, d11ucode_wlcx64, d11ucode_wlcx64sz);
					wlc_ucodex_write(wlc_hw, d11ucodex64, d11ucodex64sz);
					WL_ERROR(("%s: wl%d: downloading d11ucode_wlcx64\n",
						__FUNCTION__, wlc_hw->unit));
#else
#if defined(BTCX_ENABLED)
					wlc_ucode_write(wlc_hw, d11ucode_btcx64, d11ucode_btcx64sz);
					wlc_ucodex_write(wlc_hw, d11ucodex_btcx64,
							d11ucodex_btcx64sz);
					WL_ERROR(("%s: wl%d: downloading d11ucode_btcx64\n",
						__FUNCTION__, wlc_hw->unit));
#else
					wlc_ucode_write(wlc_hw, d11ucode64, d11ucode64sz);
					wlc_ucodex_write(wlc_hw, d11ucodex64, d11ucodex64sz);
					WL_ERROR(("%s: wl%d: downloading d11ucode64\n",
						__FUNCTION__, wlc_hw->unit));
#endif /* BTCX_ENABLED */
#endif /* WLCX_ATLAS */
				}
			}
		} else {
			WL_ERROR(("%s: wl%d: unsupported phy in corerev 64\n",
				__FUNCTION__, wlc_hw->unit));
		}
	} else
#ifdef UNRELEASEDCHIP
	if (D11REV_IS(wlc_hw->corerev, 50)) {
		if (WLCISACPHY(wlc_hw->band)) {
			wlc_ucode_write(wlc_hw, d11ucode50, d11ucode50sz);
		} else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev 50\n",
				__FUNCTION__, wlc_hw->unit));
	} else
#endif /* UNRELEASEDCHIP */
	if (D11REV_IS(wlc_hw->corerev, 49)) {
		if (WLCISACPHY(wlc_hw->band)) {
			wlc_ucode_write(wlc_hw, d11ucode49, d11ucode49sz);
		} else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev 49\n",
				__FUNCTION__, wlc_hw->unit));
	} else if (D11REV_IS(wlc_hw->corerev, 48)) {
		if (WLCISACPHY(wlc_hw->band))
			wlc_ucode_write(wlc_hw, d11ucode48, d11ucode48sz);
		else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev 48\n",
				__FUNCTION__, wlc_hw->unit));
	} else if (D11REV_IS(wlc_hw->corerev, 45) ||
	           D11REV_IS(wlc_hw->corerev, 47) ||
	           D11REV_IS(wlc_hw->corerev, 51) ||
	           D11REV_IS(wlc_hw->corerev, 52)) {
		if (WLCISACPHY(wlc_hw->band))
			wlc_ucode_write(wlc_hw, d11ucode47, d11ucode47sz);
		else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev 46\n",
				__FUNCTION__, wlc_hw->unit));
	} else if (D11REV_IS(wlc_hw->corerev, 46)) {
		if (WLCISACPHY(wlc_hw->band))
			wlc_ucode_write(wlc_hw, d11ucode46, d11ucode46sz);
		else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev 46\n",
				__FUNCTION__, wlc_hw->unit));
	} else if (D11REV_IS(wlc_hw->corerev, 43)) {
		if (WLCISACPHY(wlc_hw->band))
			wlc_ucode_write(wlc_hw, d11ucode43, d11ucode43sz);
		else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev 42\n",
				__FUNCTION__, wlc_hw->unit));
	} else if (D11REV_IS(wlc_hw->corerev, 42)) {
		if (WLCISACPHY(wlc_hw->band))
			wlc_ucode_write(wlc_hw, d11ucode42, d11ucode42sz);
		else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev 42\n",
				__FUNCTION__, wlc_hw->unit));
	} else if (D11REV_IS(wlc_hw->corerev, 41) || D11REV_IS(wlc_hw->corerev, 44)) {
		if (WLCISACPHY(wlc_hw->band))
			wlc_ucode_write(wlc_hw, d11ucode41, d11ucode41sz);
		else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev 40\n",
				__FUNCTION__, wlc_hw->unit));
	} else if (D11REV_IS(wlc_hw->corerev, 40)) {
		if (WLCISACPHY(wlc_hw->band))
			wlc_ucode_write(wlc_hw, d11ucode40, d11ucode40sz);
		else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev 40\n",
				__FUNCTION__, wlc_hw->unit));
	} else if (D11REV_IS(wlc_hw->corerev, 38)) {
		if (WLCISLCN40PHY(wlc_hw->band)) {
			wlc_ucode_write_byte(wlc_hw, d11ucode38_lcn40, d11ucode38_lcn40sz);
		} else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
			          __FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
	} else if (D11REV_IS(wlc_hw->corerev, 37)) {
		if (WLCISLCN40PHY(wlc_hw->band)) {
			wlc_ucode_write_byte(wlc_hw, d11ucode37_lcn40, d11ucode37_lcn40sz);
		} else if (WLCISNPHY(wlc_hw->band)) {
			wlc_ucode_write(wlc_hw, d11ucode31_mimo,
				d11ucode31_mimosz);
		} else {
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
			          __FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
		}
	} else if (D11REV_IS(wlc_hw->corerev, 34)) {
		if (WLCISNPHY(wlc_hw->band))
			wlc_ucode_write(wlc_hw, d11ucode34_mimo, d11ucode34_mimosz);
		else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev 34d\n",
				__FUNCTION__, wlc_hw->unit));
	} else if (D11REV_IS(wlc_hw->corerev, 33)) {
		if (WLCISLCN40PHY(wlc_hw->band)) {
			wlc_ucode_write_byte(wlc_hw, d11ucode33_lcn40, d11ucode33_lcn40sz);
		} else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
			          __FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
	} else if (D11REV_IS(wlc_hw->corerev, 32)) {
		if (WLCISNPHY(wlc_hw->band))
			wlc_ucode_write(wlc_hw, d11ucode32_mimo, d11ucode32_mimosz);
		else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev 32d\n",
				__FUNCTION__, wlc_hw->unit));
	} else if (D11REV_IS(wlc_hw->corerev, 31)) {
		if (WLCISNPHY(wlc_hw->band))
			wlc_ucode_write(wlc_hw, d11ucode29_mimo, d11ucode29_mimosz);
		else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
				__FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
	} else if (D11REV_IS(wlc_hw->corerev, 30)) {
		if (WLCISNPHY(wlc_hw->band))
			wlc_ucode_write(wlc_hw, d11ucode30_mimo, d11ucode30_mimosz);
		else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
			          __FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
	} else if (D11REV_IS(wlc_hw->corerev, 29)) {
		if (WLCISHTPHY(wlc_hw->band))
			wlc_ucode_write(wlc_hw, d11ucode29_mimo, d11ucode29_mimosz);
		else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
			          __FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
	} else if (D11REV_IS(wlc_hw->corerev, 26)) {
		if (WLCISHTPHY(wlc_hw->band))
			wlc_ucode_write(wlc_hw, d11ucode26_mimo, d11ucode26_mimosz);
		else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
			          __FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
	} else if (D11REV_IS(wlc_hw->corerev, 25) || D11REV_IS(wlc_hw->corerev, 28)) {
		if (WLCISNPHY(wlc_hw->band))
			wlc_ucode_write(wlc_hw, d11ucode25_mimo, d11ucode25_mimosz);
		else if (WLCISLCNPHY(wlc_hw->band))
			wlc_ucode_write_byte(wlc_hw, d11ucode25_lcn, d11ucode25_lcnsz);
		else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
			          __FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
	} else if (D11REV_IS(wlc_hw->corerev, 24)) {
		if (WLCISLCNPHY(wlc_hw->band))
			wlc_ucode_write_byte(wlc_hw, d11ucode24_lcn,
			                     d11ucode24_lcnsz);
		else if (WLCISNPHY(wlc_hw->band))
			wlc_ucode_write(wlc_hw, d11ucode24_mimo, d11ucode24_mimosz);
		else if (WLCISSSLPNPHY(wlc_hw->band))
			wlc_ucode_write_byte(wlc_hw, d11ucode20_sslpn,
			                     d11ucode20_sslpnsz);
		else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
			          __FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
	} else if (D11REV_IS(wlc_hw->corerev, 23)) {
		/* ucode only supports rev23(43224b0) with rev16 ucode */
		if (WLCISNPHY(wlc_hw->band))
			wlc_ucode_write(wlc_hw, d11ucode16_mimo, d11ucode16_mimosz);
		else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
			          __FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
	} else if (D11REV_IS(wlc_hw->corerev, 22)) {
		if (WLCISNPHY(wlc_hw->band))
			wlc_ucode_write(wlc_hw, d11ucode22_mimo, d11ucode22_mimosz);
		else if (WLCISSSLPNPHY(wlc_hw->band))
			wlc_ucode_write_byte(wlc_hw, d11ucode22_sslpn, d11ucode22_sslpnsz);
		else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
			          __FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
	} else if (D11REV_IS(wlc_hw->corerev, 21)) {
		if (WLCISSSLPNPHY(wlc_hw->band)) {
			wlc_ucode_write_byte(wlc_hw, d11ucode21_sslpn,
				d11ucode21_sslpnsz);
		}
		else
			WL_ERROR(("wl%d: unsupported phy in corerev 21\n", wlc_hw->unit));
	} else if (D11REV_GE(wlc_hw->corerev, 20) && WLCISSSLPNPHY(wlc_hw->band))
		wlc_ucode_write_byte(wlc_hw, d11ucode20_sslpn, d11ucode20_sslpnsz);
	else if (D11REV_IS(wlc_hw->corerev, 19) && WLCISSSLPNPHY(wlc_hw->band)) {
#ifdef BCMECICOEX
		wlc_ucode_write_byte(wlc_hw, d11ucode19_sslpn, d11ucode19_sslpnsz);
#else
		wlc_ucode_write_byte(wlc_hw, d11ucode19_sslpn_nobt, d11ucode19_sslpn_nobtsz);
#endif // endif
	}
	else if (D11REV_GE(wlc_hw->corerev, 16)) {
		if (WLCISNPHY(wlc_hw->band))
			wlc_ucode_write(wlc_hw, d11ucode16_mimo, d11ucode16_mimosz);
		else if (WLCISSSLPNPHY(wlc_hw->band))
			wlc_ucode_write_byte(wlc_hw, d11ucode16_sslpn, d11ucode16_sslpnsz);
		else if (WLCISLPPHY(wlc_hw->band))
			wlc_ucode_write(wlc_hw, d11ucode16_lp, d11ucode16_lpsz);
		else
			WL_ERROR(("wl%d: unsupported phy in corerev 16\n", wlc_hw->unit));
	}
#ifdef BTC2WIRE
	else if (D11REV_IS(wlc_hw->corerev, 15) && (wlc_hw->btc->wire == WL_BTC_2WIRE))
		wlc_ucode_write(wlc_hw, d11ucode_2w15, d11ucode_2w15sz);
#endif /* BTC2WIRE */
	else if (D11REV_IS(wlc_hw->corerev, 15))
		wlc_ucode_write(wlc_hw, d11ucode15, d11ucode15sz);
	else if (D11REV_IS(wlc_hw->corerev, 14))
		wlc_ucode_write(wlc_hw, d11ucode14, d11ucode14sz);
#ifdef BTC2WIRE
	else if (D11REV_IS(wlc_hw->corerev, 13) && (wlc_hw->btc->wire == WL_BTC_2WIRE))
		wlc_ucode_write(wlc_hw, d11ucode_2w13, d11ucode_2w13sz);
#endif /* BTC2WIRE */
	else if (D11REV_IS(wlc_hw->corerev, 13))
		wlc_ucode_write(wlc_hw, d11ucode13, d11ucode13sz);
#ifdef BTC2WIRE
	else if (D11REV_GE(wlc_hw->corerev, 11) && (wlc_hw->btc->wire == WL_BTC_2WIRE))
		wlc_ucode_write(wlc_hw, d11ucode_2w11, d11ucode_2w11sz);
#endif /* BTC2WIRE */
	else if (D11REV_GE(wlc_hw->corerev, 11))
		wlc_ucode_write(wlc_hw, d11ucode11, d11ucode11sz);
#if defined(WLNINTENDO_ENABLED) || defined(MBSS)
	/* ucode for corerev 9 has this support */
	else if (D11REV_IS(wlc_hw->corerev, 9))
		wlc_ucode_write(wlc_hw, d11ucode9, d11ucode9sz);
#endif /* defined(WLNINTENDO_ENABLED) || defined(MBSS) */
	else if (D11REV_GE(wlc_hw->corerev, 5))
		wlc_ucode_write(wlc_hw, d11ucode5, d11ucode5sz);
	else if (D11REV_IS(wlc_hw->corerev, 4))
		wlc_ucode_write(wlc_hw, d11ucode4, d11ucode4sz);
	else
		WL_ERROR(("wl%d: %s: corerev %d is invalid\n", wlc_hw->unit,
			__FUNCTION__, wlc_hw->corerev));

	wlc_hw->ucode_loaded = TRUE;
}
#endif /* BCMUCDOWNLOAD */

static void
_wlc_ucode_write(wlc_hw_info_t *wlc_hw, const uint32 ucode[], const uint nbytes, bool psmx)
{
	osl_t *osh = wlc_hw->osh;
	d11regs_t *regs = wlc_hw->regs;
	uint i;
	uint count;

	ASSERT(ISALIGNED(nbytes, sizeof(uint32)));

	count = (nbytes/sizeof(uint32));

	if (ucode_chunk == 0) {
		if (psmx) {
			W_REG(osh, &regs->objaddr, (OBJADDR_AUTO_INC | OBJADDR_UCMX_SEL));
		} else {
			W_REG(osh, &regs->objaddr, (OBJADDR_AUTO_INC | OBJADDR_UCM_SEL));
		}
		(void)R_REG(osh, &regs->objaddr);
	}
	for (i = 0; i < count; i++)
		W_REG(osh, &regs->objdata, ucode[i]);
#ifdef BCMUCDOWNLOAD
	ucode_chunk++;
#endif // endif
}

static void
wlc_ucode_write(wlc_hw_info_t *wlc_hw, const uint32 ucode[], const uint nbytes)
{
	WL_TRACE(("wl%d: wlc_ucode_write\n", wlc_hw->unit));
	_wlc_ucode_write(wlc_hw, ucode, nbytes, FALSE);
}

static void
wlc_ucodex_write(wlc_hw_info_t *wlc_hw, const uint32 ucode[], const uint nbytes)
{
	WL_TRACE(("wl%d: wlc_ucodex_write\n", wlc_hw->unit));
	_wlc_ucode_write(wlc_hw, ucode, nbytes, TRUE);
}

static void
BCMINITFN(wlc_ucode_write_byte)(wlc_hw_info_t *wlc_hw, const uint8 ucode[], const uint nbytes)
{
	osl_t *osh = wlc_hw->osh;
	d11regs_t *regs = wlc_hw->regs;
	uint i;
	uint32 ucode_word;

	WL_TRACE(("wl%d: wlc_ucode_write\n", wlc_hw->unit));

	if (ucode_chunk == 0)
		W_REG(osh, &regs->objaddr, (OBJADDR_AUTO_INC | OBJADDR_UCM_SEL));
	for (i = 0; i < nbytes; i += 7) {
		ucode_word = ucode[i+3] << 24;
		ucode_word = ucode_word | (ucode[i+4] << 16);
		ucode_word = ucode_word | (ucode[i+5] << 8);
		ucode_word = ucode_word | (ucode[i+6] << 0);
		W_REG(osh, &regs->objdata, ucode_word);

		ucode_word = ucode[i+0] << 16;
		ucode_word = ucode_word | (ucode[i+1] << 8);
		ucode_word = ucode_word | (ucode[i+2] << 0);
		W_REG(osh, &regs->objdata, ucode_word);
	}
#ifdef BCMUCDOWNLOAD
	ucode_chunk++;
#endif // endif
}

#ifndef BCM_OL_DEV
static void
BCMINITFN(wlc_ucode_pcm_write)(wlc_hw_info_t *wlc_hw, const uint32 pcm[], const uint nbytes)
{
	uint i;
	osl_t *osh = wlc_hw->osh;
	d11regs_t *regs = wlc_hw->regs;

	WL_TRACE(("wl%d: wlc_ucode_pcm_write\n", wlc_hw->unit));

	ASSERT(ISALIGNED(nbytes, sizeof(uint32)));

	W_REG(osh, &regs->objaddr,
	      (OBJADDR_IHR_SEL | ((WEP_PCMADDR - PIHR_BASE)/sizeof(uint16))));
	(void)R_REG(osh, &regs->objaddr);
	W_REG(osh, &regs->objdata, (PCMADDR_INC | PCMADDR_UCM_SEL));
	W_REG(osh, &regs->objaddr,
	      (OBJADDR_IHR_SEL | ((WEP_PCMDATA - PIHR_BASE)/sizeof(uint16))));
	(void)R_REG(osh, &regs->objaddr);
	for (i = 0; i < (nbytes/sizeof(uint32)); i++)
		W_REG(osh, &regs->objdata, pcm[i]);
}

#endif /* BCM_OL_DEV */

#ifdef WLRSDB
static void
wlc_bmac_rsdb_write_inits(wlc_hw_info_t *wlc_hw, const d11init_t *common_inits,
	const d11init_t *core1_inits)
{
	/* For RSDB chips, download common initvals d11ac12bsinitvals50
	 * for both cores. Later download the core-1 specific initvals
	 * d11ac12bsinitvals50core1 if macunit is 1 which will overwrite
	 * the initvals d11ac12bsinitvals50 in some places.
	 */

	if (D11REV_IS(wlc_hw->corerev, 50)) {
		if (WLCISACPHY(wlc_hw->band)) {
			wlc_write_inits(wlc_hw, common_inits);

			/* If it is core-1, write core-1 inits */
			if (wlc_hw->macunit)
				wlc_write_inits(wlc_hw, core1_inits);
		} else
			WL_ERROR(("%s: wl%d: unsupported phy in corerev %d\n",
				__FUNCTION__, wlc_hw->unit, wlc_hw->corerev));
	}
}
#endif /* WLRSDB */

static void
wlc_write_inits(wlc_hw_info_t *wlc_hw, const d11init_t *inits)
{
	int i;
	osl_t *osh = wlc_hw->osh;
	volatile uint8 *base;

	WL_TRACE(("wl%d: wlc_write_inits\n", wlc_hw->unit));

	base = (volatile uint8*)wlc_hw->regs;

	for (i = 0; inits[i].addr != 0xffff; i++) {
		uint offset_val = 0;
		ASSERT((inits[i].size == 2) || (inits[i].size == 4));

		if (inits[i].addr == D11CORE_TEMPLATE_REG_OFFSET) {
			/* wlc_hw->templatebase is the template base address for core 1/0
			 * For core-0 it is zero and for core 1 it contains the core-1
			 * template offset.
			 */
			offset_val = wlc_hw->templatebase;
		}
		if (inits[i].size == 2)
			W_REG(osh, (uint16*)(uintptr)(base+inits[i].addr), inits[i].value +
			offset_val);
		else if (inits[i].size == 4)
			W_REG(osh, (uint32*)(uintptr)(base+inits[i].addr), inits[i].value +
			offset_val);
	}
}

#if defined(WL_PSMX)
static int
wlc_bmac_wowlucodex_start(wlc_hw_info_t *wlc_hw)
{
	d11regs_t *regs;
	regs = wlc_hw->regs;

	/* let the PSM run to the suspended state, set mode to BSS STA */
	SET_MACINTSTATUS_X(wlc_hw->osh, regs, -1);
	wlc_bmac_mctrlx(wlc_hw, ~0, (MCTL_IHR_EN | MCTL_PSM_RUN));

	/* wait for ucode to self-suspend after auto-init */
	SPINWAIT(((GET_MACINTSTATUS_X(wlc_hw->osh, regs) & MI_MACSSPNDD) == 0), 1000 * 1000);

	if ((GET_MACINTSTATUS_X(wlc_hw->osh, regs) & MI_MACSSPNDD) == 0) {
		WL_ERROR(("wl%d: wlc_coreinit: ucode psmx did not self-suspend!\n", wlc_hw->unit));
		WL_HEALTH_LOG(wlc_hw->wlc, MACSPEND_WOWL_TIMOUT);
#ifdef WLC_HIGH
		wlc_dump_psmx_fatal(wlc_hw->wlc, PSMX_FATAL_SUSP);
#endif // endif
		return BCME_ERROR;
	}
	return BCME_OK;
}
#endif /* WL_PSMX */

int
wlc_bmac_wowlucode_start(wlc_hw_info_t *wlc_hw)
{
	d11regs_t *regs = wlc_hw->regs;
	int err = BCME_OK;

	/* let the PSM run to the suspended state, set mode to BSS STA */
	SET_MACINTSTATUS(wlc_hw->osh, regs, -1);
	wlc_bmac_mctrl(wlc_hw, ~0, (MCTL_IHR_EN | MCTL_INFRA | MCTL_PSM_RUN | MCTL_WAKE));

	/* wait for ucode to self-suspend after auto-init */
	SPINWAIT(((GET_MACINTSTATUS(wlc_hw->osh, regs) & MI_MACSSPNDD) == 0), 1000 * 1000);

	if ((GET_MACINTSTATUS(wlc_hw->osh, regs) & MI_MACSSPNDD) == 0) {
		WL_ERROR(("wl%d: wlc_coreinit: ucode did not self-suspend!\n", wlc_hw->unit));
		WL_HEALTH_LOG(wlc_hw->wlc, MACSPEND_WOWL_TIMOUT);
		err = BCME_ERROR;
#ifdef WLC_HIGH
		wlc_dump_ucode_fatal(wlc_hw->wlc, PSM_FATAL_SUSP);
#endif // endif
		goto exit;
	}

	if (D11REV_GE(wlc_hw->corerev, 64)) {
		err = wlc_bmac_wowlucodex_start(wlc_hw);
	}
exit:
	return err;
}
#ifdef WOWL
void
wlc_bmac_wowl_config_4331_5GePA(wlc_hw_info_t *wlc_hw, bool is_5G, bool is_4331_12x9)
{
	si_chipcontrl_epa4331(wlc_hw->sih, FALSE);

	if (!is_4331_12x9) {
		si_chipcontrl_epa4331(wlc_hw->sih, TRUE);
		return;
	}

	si_chipcontrl_epa4331_wowl(wlc_hw->sih, TRUE);

	if (is_5G) {
		wlc_hw->band->mhfs[MHF1] |= MHF1_4331EPA_WAR;
		wlc_write_mhf(wlc_hw, wlc_hw->band->mhfs);

		/* give the control to ucode */
		si_gpiocontrol(wlc_hw->sih, GPIO_2_PA_CTRL_5G_0, GPIO_2_PA_CTRL_5G_0,
			GPIO_DRV_PRIORITY);
		/* drive the output to 0 and ucode will drive to 1 */
		si_gpioout(wlc_hw->sih, GPIO_2_PA_CTRL_5G_0, 0, GPIO_DRV_PRIORITY);
		/* set default PA disable.  Ucode will toggle this at start of tx */
		si_gpioouten(wlc_hw->sih, GPIO_2_PA_CTRL_5G_0, GPIO_2_PA_CTRL_5G_0,
			GPIO_DRV_PRIORITY);
	}
}

/* External API to write the ucode to avoid exposing the details */

#define BOARD_GPIO_3_WOWL 0x8 /* bit mask of 3rd pin */

#ifdef WLC_LOW_ONLY
static bool
wlc_bmac_wowl_config_hw(wlc_hw_info_t *wlc_hw)
{
	/* configure the gpio etc to inform host to wake up etc */

	WL_TRACE(("wl: %s: corerev = 0x%x boardtype = 0x%x\n",  __FUNCTION__,
		wlc_hw->corerev, wlc_hw->sih->boardtype));

	if (!wlc_hw->clk) {
		WL_ERROR(("wl: %s: No hw clk \n",  __FUNCTION__));
		return FALSE;
	}

#if defined(WOWL)
	if (wlc_hw->wowl_gpio != WOWL_GPIO_INVALID_VALUE) {
		si_gpiocontrol(wlc_hw->sih, 1 << wlc_hw->wowl_gpio, 0, GPIO_DRV_PRIORITY);

		si_gpioout(wlc_hw->sih, 1 << wlc_hw->wowl_gpio,
			wlc_hw->wowl_gpiopol << wlc_hw->wowl_gpio, GPIO_DRV_PRIORITY);

		si_gpioouten(wlc_hw->sih, 1 << wlc_hw->wowl_gpio,
			1 << wlc_hw->wowl_gpio, GPIO_DRV_PRIORITY);

		OR_REG(wlc_hw->osh, &wlc_hw->regs->psm_gpio_oe, 1 << wlc_hw->wowl_gpio);
		OR_REG(wlc_hw->osh, &wlc_hw->regs->psm_gpio_out, 1 << wlc_hw->wowl_gpio);

		/* give the control to ucode */
		si_gpiocontrol(wlc_hw->sih, 1 << wlc_hw->wowl_gpio, 1 << wlc_hw->wowl_gpio,
			GPIO_DRV_PRIORITY);
	}
#endif	/* WOWL */

	return TRUE;
}

#else

static bool
wlc_bmac_wowl_config_hw(wlc_hw_info_t *wlc_hw)
{
	/* configure the gpio etc to inform host to wake up etc */

	WL_TRACE(("wl: %s: corerev = 0x%x boardtype = 0x%x\n",  __FUNCTION__,
		wlc_hw->corerev, wlc_hw->sih->boardtype));

	if (!wlc_hw->clk) {
		WL_ERROR(("wl: %s: No hw clk \n",  __FUNCTION__));
		return FALSE;
	}

	if (BUSTYPE(wlc_hw->sih->bustype) == PCI_BUS) {
		if ((CHIPID(wlc_hw->sih->chip) == BCM4331_CHIP_ID) ||
			(CHIPID(wlc_hw->sih->chip) == BCM43431_CHIP_ID)) {
				WL_INFORM(("wl%d: %s: set mux pin to SROM\n",
				           wlc_hw->unit, __FUNCTION__));
				/* force muxed pin to control ePA */
				si_chipcontrl_epa4331(wlc_hw->sih, FALSE);
				/* Apply WAR to enable 2G ePA and force muxed pin to SROM */
				si_chipcontrl_epa4331_wowl(wlc_hw->sih, TRUE);
		} else if (((CHIPID(wlc_hw->sih->chip) == BCM4360_CHIP_ID) ||
			(CHIPID(wlc_hw->sih->chip) == BCM43460_CHIP_ID) ||
			(CHIPID(wlc_hw->sih->chip) == BCM43602_CHIP_ID) ||
			(CHIPID(wlc_hw->sih->chip) == BCM4352_CHIP_ID)) &&
			(CHIPREV(wlc_hw->sih->chiprev) <= 2)) {
			si_chipcontrl_srom4360(wlc_hw->sih, TRUE);
		}
	}

	return TRUE;
}
#endif /* WLC_LOW_ONLY */

int
wlc_bmac_wowlucode_init(wlc_hw_info_t *wlc_hw)
{
#ifndef WLC_LOW_ONLY
	wlc_bmac_wowl_config_hw(wlc_hw);
#endif // endif

	if (!wlc_hw->clk) {
		WL_ERROR(("wl: %s: No hw clk \n",  __FUNCTION__));
		return BCME_ERROR;
	}

	/* Reset ucode. PSM_RUN is needed because current PC is not going to be 0 */
	wlc_bmac_mctrl(wlc_hw, ~0, (MCTL_IHR_EN | MCTL_PSM_JMP_0 | MCTL_PSM_RUN));

	return BCME_OK;
}
int
wlc_bmac_write_inits(wlc_hw_info_t *wlc_hw, void *inits, int len)
{

	wlc_write_inits(wlc_hw, inits);

	return BCME_OK;
}

int
wlc_bmac_wakeucode_dnlddone(wlc_hw_info_t *wlc_hw)
{
	d11regs_t *regs;
	regs = wlc_hw->regs;

	/* tell the ucode the corerev */
	wlc_bmac_write_shm(wlc_hw, M_MACHW_VER, (uint16)wlc_hw->corerev);

	/* overwrite default long slot timing */
	if (wlc_hw->shortslot)
		wlc_bmac_update_slot_timing(wlc_hw, wlc_hw->shortslot);

	/* write rate fallback retry limits */
	wlc_bmac_write_shm(wlc_hw, M_SFRMTXCNTFBRTHSD, wlc_hw->SFBL);
	wlc_bmac_write_shm(wlc_hw, M_LFRMTXCNTFBRTHSD, wlc_hw->LFBL);

	/* Restore the hostflags */
	wlc_write_mhf(wlc_hw, wlc_hw->band->mhfs);

	/* make sure we can still talk to the mac */
	ASSERT(R_REG(wlc_hw->osh, &regs->maccontrol) != 0xffffffff);

	wlc_bmac_mctrl(wlc_hw, MCTL_DISCARD_PMQ, MCTL_DISCARD_PMQ);

	wlc_clkctl_clk(wlc_hw, CLK_DYNAMIC);

	wlc_bmac_upd_synthpu(wlc_hw);

#ifdef WLC_LOW_ONLY
	wlc_bmac_wowl_config_hw(wlc_hw);
#endif // endif

	return BCME_OK;
}
#endif /* WOWL */

#ifdef SAMPLE_COLLECT
/**
 * Load sample collect ucode
 * Ucode inits the SHM and all MAC regs
 * can support all PHY types, implement NPHY for now.
 */
static void
wlc_ucode_sample_init_rev(wlc_hw_info_t *wlc_hw, const uint32 ucode[], const uint nbytes)
{
	if (WLCISNPHY(wlc_hw->band) && NREV_GE(wlc_hw->band->phyrev, 7)) {
	  /* Restart the ucode (recover from wl out) */
		wlc_bmac_mctrl(wlc_hw, ~0, (MCTL_IHR_EN | MCTL_PSM_RUN | MCTL_EN_MAC));
		return;
	}

	/* Reset ucode. PSM_RUN is needed because current PC is not going to be 0 */
	wlc_bmac_mctrl(wlc_hw, ~0, (MCTL_IHR_EN | MCTL_PSM_JMP_0 | MCTL_PSM_RUN));

	/* Load new d11ucode */
	wlc_ucode_write(wlc_hw, ucode, nbytes);

	(void) wlc_bmac_wowlucode_start(wlc_hw);

	/* make sure we can still talk to the mac */
	ASSERT(R_REG(wlc_hw->osh, &wlc_hw->regs->maccontrol) != 0xffffffff);
}

void
wlc_ucode_sample_init(wlc_hw_info_t *wlc_hw)
{
	if (D11REV_LT(wlc_hw->corerev, 16)) {
		WL_ERROR(("wlc_ucode_sample_init: this corerev is not support\n"));
	} else {
		wlc_ucode_sample_init_rev(wlc_hw, d11sampleucode16, d11sampleucode16sz);
	}
}
#endif	/* SAMPLE_COLLECT */

static void
wlc_ucode_txant_set(wlc_hw_info_t *wlc_hw)
{
	uint16 phyctl;
	uint16 phytxant = wlc_hw->bmac_phytxant;
	uint16 mask = PHY_TXC_ANT_MASK;

	if (D11REV_GE(wlc_hw->corerev, 40)) {
		WL_INFORM(("wl%d: %s: need rev40 update\n", wlc_hw->unit, __FUNCTION__));
		return;
	}

	/* XXX HT FIXME, pub is not available in BMAC driver
	 * if (HT_ENAB(wlc_hw->wlc->pub))
	 *	  mask = PHY_TXC_HTANT_MASK;
	 */

	/* set the Probe Response frame phy control word */
	phyctl = wlc_bmac_read_shm(wlc_hw, M_CTXPRS_BLK + C_CTX_PCTLWD_POS);
	phyctl = (phyctl & ~mask) | phytxant;
	wlc_bmac_write_shm(wlc_hw, M_CTXPRS_BLK + C_CTX_PCTLWD_POS, phyctl);

	/* set the Response (ACK/CTS) frame phy control word */
	phyctl = wlc_bmac_read_shm(wlc_hw, M_RSP_PCTLWD);
	phyctl = (phyctl & ~mask) | phytxant;
	wlc_bmac_write_shm(wlc_hw, M_RSP_PCTLWD, phyctl);
}

void
wlc_bmac_txant_set(wlc_hw_info_t *wlc_hw, uint16 phytxant)
{
	/* update sw state */
	wlc_hw->bmac_phytxant = phytxant;

	/* push to ucode if up */
	if (!wlc_hw->up)
		return;
	wlc_ucode_txant_set(wlc_hw);

}

uint16
wlc_bmac_get_txant(wlc_hw_info_t *wlc_hw)
{
#ifdef WLC_HIGH
	return (uint16)wlc_hw->wlc->stf->txant;
#else
	return 0;
#endif // endif
}

void
wlc_bmac_antsel_type_set(wlc_hw_info_t *wlc_hw, uint8 antsel_type)
{
	wlc_hw->antsel_type = antsel_type;

	/* Update the antsel type for phy module to use */
	wlc_phy_antsel_type_set(wlc_hw->band->pi, antsel_type);
}

void
wlc_bmac_fifoerrors(wlc_hw_info_t *wlc_hw)
{
	bool fatal = FALSE;
	uint unit;
	uint intstatus, idx;
	d11regs_t *regs = wlc_hw->regs;

	unit = wlc_hw->unit;
	BCM_REFERENCE(unit);

	for (idx = 0; idx < wlc_hw->nfifo_inuse; idx++) {
#if defined(WL_MU_TX) && !defined(WL_MU_TX_DISABLED)
		/* skip FIFO #6 and #7 as they are not used */
		if ((idx == TX_FIFO_6)||(idx == TX_FIFO_7)) {
			continue;
		}
#endif /* #if defined(WL_MU_TX) && !defined(WL_MU_TX_DISABLED) */

		/* read intstatus register and ignore any non-error bits */
		if (BCM_DMA_CT_ENAB(wlc_hw->wlc)) {
			if (wlc_hw->aqm_di[idx] == NULL)
				continue;
			dma_set_indqsel(wlc_hw->aqm_di[idx], FALSE);
			intstatus = R_REG(wlc_hw->osh, &regs->indaqm.indintstatus) & I_ERRORS;
		}
		else {
			intstatus = R_REG(wlc_hw->osh,
				&regs->intctrlregs[idx].intstatus) & I_ERRORS;
		}

		if (!intstatus)
			continue;

		WL_TRACE(("wl%d: wlc_bmac_fifoerrors: intstatus%d 0x%x\n", unit, idx, intstatus));

		if (intstatus & I_RO) {
			WL_ERROR(("wl%d: fifo %d: receive fifo overflow\n", unit, idx));
			WLCNTINCR(wlc_hw->wlc->pub->_cnt->rxoflo);
			fatal = TRUE;
		}

		if (intstatus & I_PC) {
			WL_ERROR(("wl%d: fifo %d: descriptor error\n", unit, idx));
			WLCNTINCR(wlc_hw->wlc->pub->_cnt->dmade);
			fatal = TRUE;
		}

		if (intstatus & I_PD) {
#if defined(MACOSX)
			printf("wl%d: fifo %d: data error\n", unit, idx);
#else
			WL_ERROR(("wl%d: fifo %d: data error\n", unit, idx));
#endif // endif
			WLCNTINCR(wlc_hw->wlc->pub->_cnt->dmada);
			fatal = TRUE;
		}

		if (intstatus & I_DE) {
			WL_ERROR(("wl%d: fifo %d: descriptor protocol error\n", unit, idx));
			WLCNTINCR(wlc_hw->wlc->pub->_cnt->dmape);
			fatal = TRUE;
		}

		if (intstatus & I_RU) {
			WL_ERROR(("wl%d: fifo %d: receive descriptor underflow\n", unit, idx));
			WLCNTINCR(wlc_hw->wlc->pub->_cnt->rxuflo[idx]);
		}

		if (intstatus & I_XU) {
			WL_ERROR(("wl%d: fifo %d: transmit fifo underflow\n", idx, unit));
			WLCNTINCR(wlc_hw->wlc->pub->_cnt->txuflo);
			fatal = TRUE;
		}

#ifdef BCMDBG
		{
			/* dump dma rings to console */
			const int FIFOERROR_DUMP_SIZE = 16384;
			char *tmp;
			struct bcmstrbuf b;
			if (fatal && !PIO_ENAB_HW(wlc_hw) && wlc_hw->di[idx] &&
			    (tmp = MALLOC(wlc_hw->osh, FIFOERROR_DUMP_SIZE))) {
				bcm_binit(&b, tmp, FIFOERROR_DUMP_SIZE);
				dma_dump(wlc_hw->di[idx], &b, TRUE);
				printbig(tmp);
				MFREE(wlc_hw->osh, tmp, FIFOERROR_DUMP_SIZE);
			}
		}

#endif /* BCMDBG */

		if (fatal) {
			WLC_EXTLOG(wlc_hw->wlc, LOG_MODULE_COMMON, FMTSTR_FATAL_ERROR_ID,
				WL_LOG_LEVEL_ERR, 0, intstatus, NULL);
			WL_HEALTH_LOG(wlc_hw->wlc, DESCRIPTOR_ERROR);
			wlc_fatal_error(wlc_hw->wlc);	/* big hammer */
			break;
		}
		else {
			if (BCM_DMA_CT_ENAB(wlc_hw->wlc)) {
				if (wlc_hw->aqm_di[idx] == NULL)
					continue;
				dma_set_indqsel(wlc_hw->aqm_di[idx], FALSE);
				W_REG(wlc_hw->osh, &regs->indaqm.indintstatus, intstatus);
			}
			else {
				W_REG(wlc_hw->osh, &regs->intctrlregs[idx].intstatus, intstatus);
			}
		}
	}
}

/**
 * callback for siutils.c, which has only wlc handler, no wl
 * they both check up, not only because there is no need to off/restore d11 interrupt
 *  but also because per-port code may require sync with valid interrupt.
 */

static uint32
wlc_wlintrsoff(wlc_hw_info_t *wlc_hw)
{
	if (!wlc_hw->up)
		return 0;

	return wl_intrsoff(wlc_hw->wlc->wl);
}

static void
wlc_wlintrsrestore(wlc_hw_info_t *wlc_hw, uint32 macintmask)
{
	if (!wlc_hw->up)
		return;

	wl_intrsrestore(wlc_hw->wlc->wl, macintmask);
}

#ifdef BCMDBG
static bool
wlc_intrs_enabled(wlc_hw_info_t *wlc_hw)
{
	return (wlc_hw->macintmask != 0);
}
#endif /* BCMDBG */

void
wlc_bmac_mute(wlc_hw_info_t *wlc_hw, bool on, mbool flags)
{
#ifdef BCM_OL_DEV
#define MUTE_DATA_FIFO	TX_ATIM_FIFO	/* for low mac, only fifo 5 allowed */
#else
#define MUTE_DATA_FIFO	TX_DATA_FIFO
#endif // endif
	if (on) {
		/* suspend tx fifos */
		wlc_bmac_tx_fifo_suspend(wlc_hw, MUTE_DATA_FIFO);
		wlc_bmac_tx_fifo_suspend(wlc_hw, TX_CTL_FIFO);
		wlc_bmac_tx_fifo_suspend(wlc_hw, TX_AC_BK_FIFO);
#ifdef WME
		wlc_bmac_tx_fifo_suspend(wlc_hw, TX_AC_VI_FIFO);
#endif /* WME */
#ifdef AP
		wlc_bmac_tx_fifo_suspend(wlc_hw, TX_BCMC_FIFO);
#endif /* AP */
#if defined(MBSS) || defined(BCM_OL_DEV) || defined(WLAIBSS)
		wlc_bmac_tx_fifo_suspend(wlc_hw, TX_ATIM_FIFO);
#endif // endif

		/* clear the address match register so we do not send ACKs */
		wlc_bmac_clear_match_mac(wlc_hw);
	} else {
		/* resume tx fifos */
		if (!wlc_hw->wlc->tx_suspended) {
			wlc_bmac_tx_fifo_resume(wlc_hw, MUTE_DATA_FIFO);
		}
		wlc_bmac_tx_fifo_resume(wlc_hw, TX_CTL_FIFO);
		wlc_bmac_tx_fifo_resume(wlc_hw, TX_AC_BK_FIFO);
#ifdef WME
		wlc_bmac_tx_fifo_resume(wlc_hw, TX_AC_VI_FIFO);
#endif /* WME */
#ifdef AP
		wlc_bmac_tx_fifo_resume(wlc_hw, TX_BCMC_FIFO);
#endif /* AP */
#if defined(MBSS) || defined(BCM_OL_DEV) || defined(WLAIBSS)
		wlc_bmac_tx_fifo_resume(wlc_hw, TX_ATIM_FIFO);
#endif // endif

		/* Restore address */
		wlc_bmac_set_match_mac(wlc_hw, &wlc_hw->etheraddr);
	}

	wlc_phy_mute_upd(wlc_hw->band->pi, on, flags);

	if (on)
		wlc_ucode_mute_override_set(wlc_hw);
	else
		wlc_ucode_mute_override_clear(wlc_hw);
}

void
wlc_bmac_set_deaf(wlc_hw_info_t *wlc_hw, bool user_flag)
{
	wlc_phy_set_deaf(wlc_hw->band->pi, user_flag);
}

#if defined(WLTEST)
void
wlc_bmac_clear_deaf(wlc_hw_info_t *wlc_hw, bool user_flag)
{
	wlc_phy_clear_deaf(wlc_hw->band->pi, user_flag);
}
#endif // endif

void
wlc_bmac_filter_war_upd(wlc_hw_info_t *wlc_hw, bool set)
{
	wlc_phy_set_filt_war(wlc_hw->band->pi, set);
}

int
wlc_bmac_xmtfifo_sz_get(wlc_hw_info_t *wlc_hw, uint fifo, uint *blocks)
{
	if (fifo >= NFIFO)
		return BCME_RANGE;

	*blocks = wlc_hw->xmtfifo_sz[fifo];

	return 0;
}

int
wlc_bmac_xmtfifo_sz_set(wlc_hw_info_t *wlc_hw, uint fifo, uint16 blocks)
{
	if (fifo >= NFIFO || blocks > 299)
		return BCME_RANGE;

	wlc_hw->xmtfifo_sz[fifo] = blocks;

#ifdef WLAMPDU_HW
	if (fifo < AC_COUNT) {
		wlc_hw->xmtfifo_frmmax[fifo] =
			(wlc_hw->xmtfifo_sz[fifo] * 256 - 1300)	/ MAX_MPDU_SPACE;
		WL_INFORM(("%s: fifo sz blk %d entries %d\n",
			__FUNCTION__, wlc_hw->xmtfifo_sz[fifo], wlc_hw->xmtfifo_frmmax[fifo]));
	}
#endif // endif

	return 0;
}

#if defined(WLTEST)
static void
wlc_bmac_suspend_timeout(void *arg)
{
	wlc_info_t *wlc = (wlc_info_t*)arg;

	ASSERT(wlc);

	if (wlc->bmac_suspend_timer) {
		wl_del_timer(wlc->wl, wlc->bmac_suspend_timer);
		wlc->is_bmac_suspend_timer_active = FALSE;
	}

	wlc_bmac_enable_mac(wlc->hw);
}
#endif // endif

/* Get the TX FIFO FLUSH status
 *
 * Get the specified TX FIFO flush status
 * Use a function to accomandate the different flush register address
 * where indirect DMA feature is insdie.
 *
 * return the corresponding channel status bit
 * caller can check if the return value is 0 or not to judge the status
 */

uint32
wlc_bmac_txfifo_flush_status(wlc_hw_info_t *wlc_hw, uint fifo)
{
	volatile uint32 *chnflushstatus;
	uint32 chnmask;

	chnflushstatus = &wlc_hw->regs->chnstatus;

#ifdef BCM_DMA_CT
	if (BCM_DMA_CT_ENAB(wlc_hw->wlc)) {
		chnflushstatus = &wlc_hw->regs->chnflushstatus;
		chnmask = (0x1 << fifo);
	}
	else
#endif /* BCM_DMA_CT */
	{
		chnmask = (0x100 << fifo);
	}

	return (R_REG(wlc_hw->osh, chnflushstatus) & chnmask);
}

/* Get the TX FIFO SUSPEND status
 *
 * Get the specified TX FIFO suspend status
 * Use a function to accomandate the different flush register address
 * where indirect DMA feature is insdie
 */

uint32
wlc_bmac_txfifo_suspend_status(wlc_hw_info_t *wlc_hw, uint fifo)
{
	volatile uint32 *chnsuspstatus;
	uint32 chnmask;

	chnsuspstatus = &wlc_hw->regs->chnstatus;
	chnmask = (0x1 << fifo);

#ifdef BCM_DMA_CT
	if (BCM_DMA_CT_ENAB(wlc_hw->wlc)) {
		chnsuspstatus = &wlc_hw->regs->chnsuspstatus;
	}
#endif /* BCM_DMA_CT */

	return (R_REG(wlc_hw->osh, chnsuspstatus) & chnmask);
}

/**
 * Check the MAC's tx suspend status for a tx fifo.
 *
 * When the MAC acknowledges a tx suspend, it indicates that no more packets will be transmitted out
 * the radio. This is independent of DMA channel suspension---the DMA may have finished suspending,
 * or may still be pulling data into a tx fifo, by the time the MAC acks the suspend request.
 */
bool
wlc_bmac_tx_fifo_suspended(wlc_hw_info_t *wlc_hw, uint tx_fifo)
{
	volatile uint32 *suspstatus;

	/* check that a suspend has been requested and is no longer pending */
	if (!PIO_ENAB_HW(wlc_hw)) {
		/*
		 * for DMA mode, the suspend request is set in xmtcontrol of the DMA engine,
		 * and the tx fifo suspend at the lower end of the MAC is acknowledged in the
		 * chnstatus register.
		 * for indirect DMA mode , the suspend request is set in common register, SuspReq
		 * and suspend status is reflected in SuspStatus register.
		 * The tx fifo suspend completion is independent of the DMA suspend completion and
		 *   may be acked before or after the DMA is suspended.
		 */

#ifdef BCM_DMA_CT
		if (BCM_DMA_CT_ENAB(wlc_hw->wlc)) {
			suspstatus = &wlc_hw->regs->chnsuspstatus;
		}
		else
#endif /* BCM_DMA_CT */
		{
			suspstatus = &wlc_hw->regs->chnstatus;
		}

		if (dma_txsuspended(wlc_hw->di[tx_fifo]) &&
		    (R_REG(wlc_hw->osh, suspstatus) & (1<<tx_fifo)) == 0)
			return TRUE;
	} else {
		if (wlc_pio_txsuspended(wlc_hw->pio[tx_fifo]))
			return TRUE;
	}

	return FALSE;
}

void
wlc_bmac_tx_fifo_suspend(wlc_hw_info_t *wlc_hw, uint tx_fifo)
{
	uint8 fifo = 1 << tx_fifo;

	/* Two clients of this code, 11h Quiet period and scanning. */

	/* only suspend if not already suspended */
	if ((wlc_hw->suspended_fifos & fifo) == fifo)
		return;

	/* force the core awake only if not already */
	wlc_upd_suspended_fifos_set(wlc_hw, tx_fifo);

	if (!PIO_ENAB_HW(wlc_hw)) {
		if (wlc_hw->di[tx_fifo]) {
			bool suspend;

			/* Suspending AMPDU transmissions in the middle can cause underflow
			 * which may result in mismatch between ucode and driver
			 * so suspend the mac before suspending the FIFO
			 */
			suspend = !(R_REG(wlc_hw->osh, &wlc_hw->regs->maccontrol) & MCTL_EN_MAC);

			if (WLC_PHY_11N_CAP(wlc_hw->band) && !suspend)
				wlc_bmac_suspend_mac_and_wait(wlc_hw);

			dma_txsuspend(wlc_hw->di[tx_fifo]);

			if (WLC_PHY_11N_CAP(wlc_hw->band) && !suspend)
				wlc_bmac_enable_mac(wlc_hw);
		}
	} else {
		wlc_pio_txsuspend(wlc_hw->pio[tx_fifo]);
	}
}

void
wlc_bmac_tx_fifo_resume(wlc_hw_info_t *wlc_hw, uint tx_fifo)
{
	/* BMAC_NOTE: WLC_TX_FIFO_ENAB is done in wlc_dpc() for DMA case but need to be done
	 * here for PIO otherwise the watchdog will catch the inconsistency and fire
	 */
	/* Two clients of this code, 11h Quiet period and scanning. */
	if (!PIO_ENAB_HW(wlc_hw)) {
		if (wlc_hw->di[tx_fifo])
			dma_txresume(wlc_hw->di[tx_fifo]);
	} else {
		wlc_pio_txresume(wlc_hw->pio[tx_fifo]);
		/* BMAC_NOTE: XXX This macro uses high level state, we need to do something
		 * about it. PIO fifo needs to be enabled when tx fifo got resumed
		 */
#if !defined(NEW_TXQ)
/* ifdef out because low TxQ implementation does not use the wlc->prec_map */
#ifdef WLC_HIGH
		WLC_TX_FIFO_ENAB(wlc_hw->wlc, tx_fifo);
#endif // endif
#endif /* NEW_TXQ */
	}

	/* allow core to sleep again */
	wlc_upd_suspended_fifos_clear(wlc_hw, tx_fifo);
}

#ifdef WL_MULTIQUEUE
static void wlc_bmac_service_txstatus(wlc_hw_info_t *wlc_hw);
static void wlc_bmac_flush_tx_fifos(wlc_hw_info_t *wlc_hw, uint fifo_bitmap);
static void wlc_bmac_uflush_tx_fifos(wlc_hw_info_t *wlc_hw, uint fifo_bitmap);
static void wlc_bmac_enable_tx_fifos(wlc_hw_info_t *wlc_hw, uint fifo_bitmap);
#ifdef WLC_LOW_ONLY
static void wlc_bmac_clear_tx_fifos(wlc_hw_info_t *wlc_hw, uint fifo_bitmap);
#endif // endif

/* Enable new method of suspend and flush.
 * Requires minimum ucode BOM 622.1.
 */
#define NEW_SUSPEND_FLUSH_UCODE 1
void
wlc_bmac_tx_fifo_sync(wlc_hw_info_t *wlc_hw, uint fifo_bitmap, uint8 flag)
{
#ifdef NEW_SUSPEND_FLUSH_UCODE

	/* halt any tx processing by ucode */
	wlc_bmac_suspend_mac_and_wait(wlc_hw);

	/* PR95192 WAR: For 5357/53572 chips.
	 * For 5357/53572 chips after halting the microcode and before clear the
	 * hardware fifos the microcode resumed again from halted state in order
	 * to detect the flush_tx_fifos command to flash the fifos. After fifos
	 * are flashed the microcode will be halted again to return it to the
	 * previous state.
	 */
	if (((CHIPID(wlc_hw->sih->chip)) == BCM5357_CHIP_ID) ||
	    ((CHIPID(wlc_hw->sih->chip)) == BCM53572_CHIP_ID)) {
		wlc_bmac_mctrl(wlc_hw, MCTL_PSM_RUN, MCTL_PSM_RUN);
	}

	/* clear the hardware fifos */
	wlc_bmac_flush_tx_fifos(wlc_hw, fifo_bitmap);

	/* PR95192 WAR: For 5357/53572 chips.
	 * After hardware fifos are flashed put the microcode back into the halted
	 * state again to return it to the previous state.
	 */
	if (((CHIPID(wlc_hw->sih->chip)) == BCM5357_CHIP_ID) ||
	    ((CHIPID(wlc_hw->sih->chip)) == BCM53572_CHIP_ID))
	    wlc_bmac_mctrl(wlc_hw, MCTL_PSM_RUN, 0);

	/* process any frames that made it out before the suspend */
	wlc_bmac_service_txstatus(wlc_hw);

#ifdef HOST_HDR_FETCH
	if (HOST_HDR_FETCH_ENAB()) {
		wlc_txs_fetch_rqst_queue_flush(wlc_hw->wlc, fifo_bitmap);
	}
#endif // endif

	/* allow ucode to run again */
	wlc_bmac_enable_mac(wlc_hw);
#else
	bool suspend;

	/* enable MAC only if currently suspended */
	suspend = !(R_REG(wlc_hw->osh, &wlc_hw->regs->maccontrol) & MCTL_EN_MAC);
	if (suspend)
		wlc_bmac_enable_mac(wlc_hw);

	/* clear the hardware fifos */
	wlc_bmac_flush_tx_fifos(wlc_hw, fifo_bitmap);

	/* put MAC back into suspended state if required */
	if (suspend)
		wlc_bmac_suspend_mac_and_wait(wlc_hw);

	/* process any frames that made it out before the suspend */
	wlc_bmac_service_txstatus(wlc_hw);

#endif /* NEW_SUSPEND_FLUSH_UCODE */

#ifdef WLC_LOW_ONLY
	/* clear the hardware fifos for split driver */
	wlc_bmac_clear_tx_fifos(wlc_hw, fifo_bitmap);
#endif /* WLC_LOW_ONLY */

	/* signal to the upper layer that the fifos are flushed
	 * and any tx packet statuses have been returned
	 */
	wlc_tx_fifo_sync_complete(wlc_hw->wlc, fifo_bitmap, flag);

	/* reenable the fifos once the completion has been signaled */
	wlc_bmac_enable_tx_fifos(wlc_hw, fifo_bitmap);

	/* Check the opportunity to open block_datafifo */
	wlc_tx_open_datafifo(wlc_hw->wlc);
}

static void
wlc_bmac_service_txstatus(wlc_hw_info_t *wlc_hw)
{
	bool fatal = FALSE;

	if (wlc_bmac_txstatus(wlc_hw, FALSE, &fatal)) {
		WL_ERROR(("%s: there is still txstatus is pending to process()\n", __FUNCTION__));
	}
}

#define BMC_IN_PROG_CHK_ENAB 0x8000

static void
wlc_bmac_uflush_tx_fifos(wlc_hw_info_t *wlc_hw, uint fifo_bitmap)
{
	uint i;
	uint chnstatus, status;
	uint count;
	osl_t *osh = wlc_hw->osh;
	uint fbmp;
	d11regs_t *regs;
	dma64regs_t *d64regs;

	regs = wlc_hw->regs;

	/* step 1. request dma suspend fifos */
	/* Do this one DMA at a time, suspending all in a loop causes
	   trouble for dongle drivers...
	*/
	for (i = 0, fbmp = fifo_bitmap; fbmp; i++, fbmp = fbmp >> 1) {
		if ((fbmp & 0x01) == 0)
			continue;

		//no need to update wlc_hw->suspended_fifos cause dma_txsuspend is called hereafter
		dma_txsuspend(wlc_hw->di[i]);
		wlc_bmac_write_shm(wlc_hw, M_TXFL_BMAP, (1 << i));

		d64regs = &regs->fifo.f64regs[i].dmaxmt;
		count = 0;
		while (count < (80 * 1000)) {
			chnstatus = R_REG(osh, &regs->chnstatus);
			status = R_REG(osh, &d64regs->status0) & D64_XS0_XS_MASK;
			if (chnstatus == 0 && status == D64_XS0_XS_IDLE)
				break;
			OSL_DELAY(10);
			count += 10;
		}
		if (chnstatus || (status != D64_XS0_XS_IDLE)) {
			WL_ERROR(("MQ ERROR %s: suspend dma %d not done after %d us: "
				 "chnstatus 0x%04x dma_status 0x%x txefs 0x%04x\n"
				 "BMCReadStatus: 0x%04x AQMFifoReady: 0x%04x\n",
				 __FUNCTION__, i, count, chnstatus, status,
				 R_REG(osh, &regs->u.d11acregs.XmtSuspFlush),
				 R_REG(osh, &regs->u.d11acregs.BMCReadStatus),
				 R_REG(osh, &regs->u.d11acregs.u0.lt64.AQMFifoReady)));
		}
	}

	/* step 4. re-wind dma last ptr to the first desc with EOF from current active index  */
	for (i = 0, fbmp = fifo_bitmap; fbmp; i++, fbmp = fbmp >> 1) {
		if ((fbmp & 0x01) == 0)
			continue;
		dma_txrewind(wlc_hw->di[i]);
	}

	/* step 5. un-suspend dma...and do another ucode flush for "partial" frames */
	/* Again, do this one DMA at a time and not all in a loop */
	for (i = 0, fbmp = fifo_bitmap; fbmp; i++, fbmp = fbmp >> 1) {
		if ((fbmp & 0x01) == 0)
			continue;
		dma_txresume(wlc_hw->di[i]);
		//no need to update wlc_hw->suspended_fifos cause dma_txsuspend is called hereafter
		/* (| BMC_IN_PROG_CHK_ENAB) enables BMC in_prog check in ucode */
		wlc_bmac_write_shm(wlc_hw, M_TXFL_BMAP, ((1 << i) | BMC_IN_PROG_CHK_ENAB));

		count = 0;
		while (count < (80 * 1000)) {
			chnstatus = wlc_bmac_read_shm(wlc_hw, M_TXFL_BMAP);
			chnstatus &= ~BMC_IN_PROG_CHK_ENAB;
			if (chnstatus == 0)
				break;
			OSL_DELAY(10);
			count += 10;
		}
		if (chnstatus) {
			WL_ERROR(("MQ ERROR %s: ucode flush 0x%02x not done after %d us: "
				  "M_TXFL_BMAP 0x%04x txefs 0x%04x\n"
				  "BMCReadStatus: 0x%04x AQMFifoReady: 0x%04x DMABusy %04x\n",
				  __FUNCTION__, i, count, chnstatus,
				  R_REG(osh, &regs->u.d11acregs.XmtSuspFlush),
				  R_REG(osh, &regs->u.d11acregs.BMCReadStatus),
				  R_REG(osh, &regs->u.d11acregs.u0.lt64.AQMFifoReady),
				  R_REG(osh, &regs->u.d11acregs.XmtDMABusy)));
		}
	}

	/* step 6: have to suspend dma again. otherwise, frame doesn't show up fifordy */
	for (i = 0, fbmp = fifo_bitmap; fbmp; i++, fbmp = fbmp >> 1) {
		if ((fbmp & 0x01) == 0)
			continue;

		wlc_upd_suspended_fifos_set(wlc_hw, i);
		dma_txsuspend(wlc_hw->di[i]);
		d64regs = &regs->fifo.f64regs[i].dmaxmt;
		count = 0;
		while (count < (80 * 1000)) {
			chnstatus = R_REG(osh, &regs->chnstatus);
			status = R_REG(osh, &d64regs->status0) & D64_XS0_XS_MASK;
			if (chnstatus == 0 && status == D64_XS0_XS_IDLE)
				break;
			OSL_DELAY(10);
			count += 10;
		}
		if (chnstatus || (status != D64_XS0_XS_IDLE)) {
			WL_PRINT(("MQ ERROR %s: final suspend dma %d not done after %d us: "
				 "chnstatus 0x%04x dma_status 0x%x txefs 0x%04x\n"
				 "BMCReadStatus: 0x%04x AQMFifoReady: 0x%04x\n",
				 __FUNCTION__, i, count, chnstatus, status,
				 R_REG(osh, &regs->u.d11acregs.XmtSuspFlush),
				 R_REG(osh, &regs->u.d11acregs.BMCReadStatus),
				 R_REG(osh, &regs->u.d11acregs.u0.lt64.AQMFifoReady)));
		}
	}
}

static void
wlc_bmac_flush_tx_fifos(wlc_hw_info_t *wlc_hw, uint fifo_bitmap)
{
	uint i;
	uint32 chnstatus;
	uint count;
	osl_t *osh = wlc_hw->osh;
	uint fbmp;
	d11regs_t *regs = wlc_hw->regs;
	bool mq_err = FALSE, pre64_sfwar = TRUE;
	uint status = 0;

	/* filter out un-initalized txfifo */
	for (i = 0, fbmp = fifo_bitmap; fbmp; i++, fbmp = fbmp >> 1) {
		if ((fbmp & 0x01) == 0)
			continue;
		if ((!PIO_ENAB_HW(wlc_hw) && wlc_hw->di[i] == NULL))
			fifo_bitmap &= ~(1 << i);
	}

	if (D11REV_IS(wlc_hw->corerev, 48) || D11REV_IS(wlc_hw->corerev, 49) ||
		D11REV_IS(wlc_hw->corerev, 50)) {
		wlc_bmac_uflush_tx_fifos(wlc_hw, fifo_bitmap);
		return;
	}

	// define variable pre64_sfwar for making sure DMA go into idle state after suspend
#ifdef BCM_DMA_CT
	if (BCM_DMA_CT_ENAB(wlc_hw->wlc))
		pre64_sfwar = FALSE;
#endif // endif
	/* WAR 104924:
	 * HW WAR 4360A0/B0, 4335A0, 4350, 43602A0 (d11 core rev ge 40) DMA engine cannot take a
	 * simultaneous suspend and flush request. Software WAR is to set suspend request,
	 * wait for DMA idle indication, then set the flush request
	 * (eg continue w/ regular processing).
	 * XXX: fixed since rev64 (at least we hope)
	 */
	if (D11REV_GE(wlc_hw->corerev, 40)) {

		/* set suspend to the requested fifos */
		for (i = 0, fbmp = fifo_bitmap; fbmp; i++, fbmp = fbmp >> 1) {
			if ((fbmp & 0x01) == 0)
				continue;

			wlc_upd_suspended_fifos_set(wlc_hw, i);
			dma_txsuspend(wlc_hw->di[i]);
			if (D11REV_LT(wlc_hw->corerev, 64)) {
				/* request ucode flush */
				wlc_bmac_write_shm(wlc_hw, M_TXFL_BMAP, (uint16)(1 << i));
			}

			/* check chnstatus and ucode flush status */
			count = 0;
			while (count < (80 * 1000)) {
				chnstatus = wlc_bmac_txfifo_suspend_status(wlc_hw, i);
				if (D11REV_LT(wlc_hw->corerev, 64)) {
					status = wlc_bmac_read_shm(wlc_hw, M_TXFL_BMAP);
					if (chnstatus == 0 && status == 0)
						break;
				} else if (chnstatus == 0) {
					break;
				}
				OSL_DELAY(10);
				count += 10;
			}
			if (chnstatus || status) {
				mq_err = TRUE;
				WL_ERROR(("MQ ERROR %s: suspend dma %d not done after %d us: "
					  "chnstatus 0x%04x txfl_bmap 0x%04x txefs 0x%04x\n",
					  __FUNCTION__, i, count, chnstatus, status,
					  R_REG(osh, &regs->u.d11acregs.XmtSuspFlush)));
			}
		}

		if (pre64_sfwar) {
			for (i = 0, fbmp = fifo_bitmap; fbmp; i++, fbmp = fbmp >> 1) {
				dma64regs_t *d64regs;
				uint status_local;

				/* skip uninterested and empty fifo */
				if ((fbmp & 0x01) == 0)
					continue;

				d64regs = &regs->fifo.f64regs[i].dmaxmt;

				/* need to make sure dma has become idle (finish any pending tx) */
				count = 0;
				while (count < (80 * 1000)) {
					status_local = R_REG(osh, &d64regs->status0)
									& D64_XS0_XS_MASK;
					if (status_local == D64_XS0_XS_IDLE)
						break;
					OSL_DELAY(10);
					count += 10;
				}
				if (status_local != D64_XS0_XS_IDLE) {
					mq_err = TRUE;
					if (D11REV_LT(wlc_hw->corerev, 64)) {
						WL_ERROR(("ERROR: dma %d status 0x%x %x "
							"doesn't return idle "
							"after %d us. shm_bmap 0x%04x\n",
							i, status_local,
							R_REG(osh, &d64regs->status1), count,
							wlc_bmac_read_shm(wlc_hw, M_TXFL_BMAP)));
					} else {
						WL_ERROR(("ERROR: dma %d status 0x%x %x "
							"doesn't return idle "
							"after %d us.\n", i, status_local,
							R_REG(osh, &d64regs->status1), count));
					}
				}
			}
		}
	}
	/* end WAR 104924 */

	if (!PIO_ENAB_HW(wlc_hw)) {
		for (i = 0, fbmp = fifo_bitmap; fbmp; i++, fbmp = fbmp >> 1) {
			if ((fbmp & 0x01) == 0) /* not the right fifo to process */
				continue;

			wlc_upd_suspended_fifos_set(wlc_hw, i);
			dma_txflush(wlc_hw->di[i]);

			/* wait for flush complete */
			count = 0;
			while (count < (80 * 1000)) {
				chnstatus = wlc_bmac_txfifo_flush_status(wlc_hw, i);
				if (chnstatus == 0)
					break;
				OSL_DELAY(10);
				count += 10;
			}
			if (chnstatus != 0) {
				WL_ERROR(("MQ ERROR: %s: flush fifo %d timeout after %d us. "
					  "chnstatus 0x%x\n", __FUNCTION__, i, count, chnstatus));
			} else {
				WL_MQ(("MQ: %s: fifo %d waited %d us for success chanstatus 0x%x\n",
				       __FUNCTION__, i, count, chnstatus));
			}
		}

#ifdef WL_MULTIQUEUE_DBG
		/* DBG print */
		volatile uint32 *regaddr;
		dma64regs_t *d64regs;
#ifdef BCM_DMA_CT
		if (BCM_DMA_CT_ENAB(wlc_hw->wlc)) {
			regaddr = &regs->chnflushstatus;
		}
		else
#endif /* BCM_DMA_CT */
		{
			regaddr = &regs->chnstatus;
		}
		chnstatus = R_REG(osh, regaddr);
		WL_MQ(("MQ: %s: post flush req chnstatus 0x%x\n", __FUNCTION__,
		       chnstatus));

		for (i = 0, fbmp = fifo_bitmap; fbmp; i++, fbmp = fbmp >> 1) {
			if ((fbmp & 0x01) == 0) /* not the right fifo to process */
				continue;

			if (D11REV_LT(wlc_hw->corerev, 11)) {
				dma32regs_t *d32regs = &regs->fifo.f32regs.dmaregs[i].xmt;
				status = ((R_REG(osh, &d32regs->status) & XS_XS_MASK) >>
					  XS_XS_SHIFT);
			} else {
#ifdef BCM_DMA_CT
				if (BCM_DMA_CT_ENAB(wlc_hw->wlc)) {
					d64regs = &regs->indaqm.dma;
					dma_set_indqsel(wlc_hw->aqm_di[i], FALSE);
				}
				else
#endif // endif
				{
					d64regs = &regs->fifo.f64regs[i].dmaxmt;
				}
				status = ((R_REG(osh, &d64regs->status0) & D64_XS0_XS_MASK) >>
					D64_XS0_XS_SHIFT);
			}
			WL_MQ(("MQ: %s: post flush req dma %d status %u\n", __FUNCTION__,
			       i, status));
		}
#endif /* WL_MULTIQUEUE_DBG */

		/* Clear the dma flush command */
		for (i = 0, fbmp = fifo_bitmap; fbmp; i++, fbmp = fbmp >> 1) {
			if ((fbmp & 0x01) == 0) /* not the right fifo to process */
				continue;

			dma_txflush_clear(wlc_hw->di[i]);
		}

#ifdef WL_MULTIQUEUE_DBG
		/* DBG print */
		for (i = 0, fbmp = fifo_bitmap; fbmp; i++, fbmp = fbmp >> 1) {

			if ((fbmp & 0x01) == 0) /* not the right fifo to process */
				continue;

			if (D11REV_LT(wlc_hw->corerev, 11)) {
				dma32regs_t *d32regs = &regs->fifo.f32regs.dmaregs[i].xmt;
				status = ((R_REG(osh, &d32regs->status) & XS_XS_MASK) >>
				          XS_XS_SHIFT);
			} else {
#ifdef BCM_DMA_CT
				if (BCM_DMA_CT_ENAB(wlc_hw->wlc)) {
					d64regs = &regs->indaqm.dma;
					dma_set_indqsel(wlc_hw->aqm_di[i], FALSE);
				}
				else
#endif // endif
				{
					d64regs = &regs->fifo.f64regs[i].dmaxmt;
				}
				status = ((R_REG(osh, &d64regs->status0) & D64_XS0_XS_MASK) >>
			          D64_XS0_XS_SHIFT);
			}
			WL_MQ(("MQ: %s: post flush wait dma %d status %u\n", __FUNCTION__,
			       i, status));
		} /* for */
#endif /* WL_MULTIQUEUE_DBG */
	} else {
		for (i = 0, fbmp = fifo_bitmap; fbmp; i++, fbmp = fbmp >> 1) {
			if ((fbmp & 0x01) == 0) /* not the right fifo to process */
				continue;

			if (wlc_hw->pio[i])
				wlc_pio_reset(wlc_hw->pio[i]);
		} /* for */
	} /* else */

	BCM_REFERENCE(mq_err);
}

static void
wlc_bmac_enable_tx_fifos(wlc_hw_info_t *wlc_hw, uint fifo_bitmap)
{
	uint i;
	uint fbmp;

	if (!PIO_ENAB_HW(wlc_hw)) {
		for (i = 0, fbmp = fifo_bitmap; fbmp; i++, fbmp = fbmp >> 1) {
			if ((fbmp & 0x01) == 0) /* not the right fifo to process */
				continue;

			if (wlc_hw->di[i] == NULL)
				continue;

#if BCM_DMA_CT
			if (BCM_DMA_CT_ENAB(wlc_hw->wlc)) {
				dma_txreset(wlc_hw->di[i]);
				dma_txreset(wlc_hw->aqm_di[i]);

				/* clear suspend req */
				AND_REG(wlc_hw->osh, &wlc_hw->regs->suspreq, ~(1 << i));
			}
			else
#endif /* BCM_DMA_CT */
			{
				dma_txreset(wlc_hw->di[i]);
			}
			wlc_upd_suspended_fifos_clear(wlc_hw, i);
			dma_txinit(wlc_hw->di[i]);
#if BCM_DMA_CT
			if (BCM_DMA_CT_ENAB(wlc_hw->wlc)) {
				/* init ct_dma(aqmdma) */
				dma_txinit(wlc_hw->aqm_di[i]);
				if (wlc_hw->wlc->cpbusy_war) {
					aqm_dma_init_fifo_stats(wlc_hw->aqm_di[i], i);
				}
			}
#endif // endif

		} /* for */
	} else {
		for (i = 0, fbmp = fifo_bitmap; fbmp; i++, fbmp = fbmp >> 1) {
			if ((fbmp & 0x01) == 0) /* not the right fifo to process */
				continue;

			if (wlc_hw->pio[i])
				wlc_pio_reset(wlc_hw->pio[i]);
		} /* for */
	} /* else */
}

#ifdef WLC_LOW_ONLY
static void
wlc_bmac_clear_tx_fifos(wlc_hw_info_t *wlc_hw, uint fifo_bitmap)
{
	uint i;
	uint fbmp;

	/* clear dma fifo */
	for (i = 0, fbmp = fifo_bitmap; fbmp; i++, fbmp = fbmp >> 1) {
		if ((fbmp & 0x01) == 0) /* not the right fifo to process */
			continue;

		if (!PIO_ENAB_HW(wlc_hw)) {
			if (wlc_hw->di[i] == NULL)
				continue;
#ifdef BCM_DMA_CT
			if (wlc_hw->aqm_di[i])
				dma_txreclaim(wlc_hw->aqm_di[i], HNDDMA_RANGE_ALL);
#endif // endif
			dma_txreclaim(wlc_hw->di[i], HNDDMA_RANGE_ALL);
			TXPKTPENDCLR(wlc_hw->wlc, i);
#if defined(DMA_TX_FREE)
			wlc_hw->txstatus_ampdu_flags[i].head = 0;
			wlc_hw->txstatus_ampdu_flags[i].tail = 0;
#endif // endif
		} else {
			if (wlc_hw->pio[i] == NULL)
				continue;

			/* include reset the counter */
			wlc_pio_txreclaim(wlc_hw->pio[i]);
		}
	}
}
#endif /* WL_LOW_ONLY */
#endif /* WL_MULTIQUEUE */

/** process tx completion events for corerev < 5 */
static bool
wlc_bmac_txstatus_corerev4(wlc_hw_info_t *wlc_hw)
{
	void *status_p;
	tx_status_cr4_t *txscr4;
	tx_status_t txs;
	osl_t *osh = wlc_hw->osh;
	bool fatal = FALSE;

	WL_TRACE(("wl%d: wlc_txstatusrecv\n", wlc_hw->unit));

	while (!fatal && (PIO_ENAB_HW(wlc_hw) ?
	                  (status_p = wlc_pio_rx(wlc_hw->pio[RX_TXSTATUS_FIFO])) :
	                  (status_p = dma_rx(wlc_hw->di[RX_TXSTATUS_FIFO])))) {

		txscr4 = (tx_status_cr4_t *)PKTDATA(osh, status_p);
		/* MAC uses little endian only */
		ltoh16_buf((void*)txscr4, sizeof(tx_status_cr4_t));

		/* shift low bits for tx_status_t status compatibility */
		txscr4->status = (txscr4->status & ~TXS_COMPAT_MASK)
			| (((txscr4->status & TXS_COMPAT_MASK) << TXS_COMPAT_SHIFT));
		txs.status.raw_bits = txscr4->status;

		fatal = wlc_bmac_dotxstatus(wlc_hw, &txs, 0);

		PKTFREE(osh, status_p, FALSE);
	}

	if (fatal)
		return TRUE;

	/* post more rbufs */
	if (!PIO_ENAB_HW(wlc_hw))
		dma_rxfill(wlc_hw->di[RX_TXSTATUS_FIFO]);

	return FALSE;
}
#ifdef BCM_OL_DEV

bool
wlc_dotxstatus(wlc_info_t *wlc, tx_status_t *txs, uint32 frm_tx2)
{
	int ncons = 0;
	void *p = NULL;
	uint queue;

	queue = WLC_TXFID_GET_QUEUE(txs->frameid);

	/* PCIDEV always operates on TX_ATIM_FIFO */
	ASSERT(queue == TX_ATIM_FIFO);

	ncons = ((txs->status.raw_bits & TX_STATUS40_NCONS)
		>> TX_STATUS40_NCONS_SHIFT);
#ifndef DMA_TX_FREE
	if (!(txs->status.raw_bits & TXS_V)) {
		ncons = 0;
		WL_ERROR(("%s: invalid txstatus 0x%x\n", __FUNCTION__,
			txs->status.raw_bits));
	}
	if ((txs->status.suppr_ind == TX_STATUS_SUPR_FLUSH) ||
	    (txs->status.suppr_ind == TX_STATUS_SUPR_BADCH)) {
		ncons  = 0;
		WL_ERROR(("%s: suppressed due to flush\n", __FUNCTION__));
	}

	while (ncons > 0) {
		p = wlc_bmac_dma_getnexttxp(wlc, queue, HNDDMA_RANGE_TRANSMITTED);
		ASSERT(p);
		if (p) {
			PKTFREE(wlc->hw->osh, p, TRUE);
		}
		ncons--;
	}
#endif /* !DMA_TX_FREE */
	return FALSE;
}

#endif /* BCM_OL_DEV */

/**
 * BMAC portion of wlc_dotxstatus,
 * XXX need to move all DMA/HW dependent preprocessing from high wlc_dotxstatus to here
 */
static bool BCMFASTPATH
wlc_bmac_dotxstatus(wlc_hw_info_t *wlc_hw, tx_status_t *txs, uint32 s2)
{
#ifdef BCMDBG
	if (wlc_hw->wlc->txfifo_detach_pending)
		WL_MQ(("MQ: %s: sync processing of txstatus\n", __FUNCTION__));
#endif /* BCMDBG */

	/* discard intermediate indications for ucode with one legitimate case:
	 *   e.g. if "useRTS" is set. ucode did a successful rts/cts exchange, but the subsequent
	 *   tx of DATA failed. so it will start rts/cts from the beginning (resetting the rts
	 *   transmission count)
	 */
	if (D11REV_LT(wlc_hw->corerev, 40) &&
		!(txs->status.raw_bits & TX_STATUS_AMPDU) &&
		(txs->status.raw_bits & TX_STATUS_INTERMEDIATE)) {
		WL_TRACE(("%s: discard status\n", __FUNCTION__));
		return FALSE;
	}

	return wlc_dotxstatus(wlc_hw->wlc, txs, s2);
}

#ifdef BCMDBG
void wlc_bmac_print_txstatus(wlc_hw_info_t *wlc_hw, tx_status_t* txs);

void wlc_bmac_print_txstatus(wlc_hw_info_t *wlc_hw, tx_status_t* txs)
{

	uint16 s = txs->status.raw_bits;
	uint16 status_bits = txs->status.raw_bits;

	static const char *supr_reason[] = {
		"None", "PMQ Entry", "Flush request",
		"Previous frag failure", "Channel mismatch",
		"Lifetime Expiry", "Underflow", "AB NACK or TX SUPR"
	};

	WL_ERROR(("\ntxpkt (MPDU) Complete\n"));

	WL_ERROR(("FrameID: 0x%04x   ", txs->frameid));
	WL_ERROR(("Seq: 0x%04x   ", txs->sequence));
	WL_ERROR(("TxStatus: 0x%04x", s));
	WL_ERROR(("\n"));

	WL_ERROR(("ACK %d IM %d PM %d Suppr %d (%s)",
	       txs->status.was_acked, txs->status.is_intermediate,
	       txs->status.pm_indicated, txs->status.suppr_ind,
	       (txs->status.suppr_ind < ARRAYSIZE(supr_reason) ?
	        supr_reason[txs->status.suppr_ind] : "Unkn supr")));

	WL_ERROR(("PHYTxErr:   0x%04x ", txs->phyerr));
	WL_ERROR(("\n"));

	WL_ERROR(("Raw\n[0]	%d Valid\n", ((status_bits & TX_STATUS_VALID) != 0)));
	WL_ERROR(("[2]    %d IM\n", ((status_bits & TX_STATUS40_INTERMEDIATE) != 0)));
	WL_ERROR(("[3]    %d PM\n", ((status_bits & TX_STATUS40_PMINDCTD) != 0)));
	WL_ERROR(("[7-4]  %d Suppr\n",
		((status_bits & TX_STATUS40_SUPR) >> TX_STATUS40_SUPR_SHIFT)));
	WL_ERROR(("[14:8] %d Ncons\n",
		((status_bits & TX_STATUS40_NCONS) >> TX_STATUS40_NCONS_SHIFT)));
	WL_ERROR(("[15]   %d Acked\n", (status_bits & TX_STATUS40_ACK_RCV) != 0));
}
#endif /* BCMDBG */

#ifdef BCM_OL_DEV

/* update txiv upon ucode suppress indication for tkip */
void wlc_bmac_update_iv(wlc_hw_info_t *wlc_hw, tx_status_t *txs)
{
	wlc_info_t *wlc;
	wlc_dngl_ol_info_t *wlc_dngl_ol;
	wlc_key_t *key;
	wlc_key_info_t key_info;
	uchar iv_rc[WOWL_TSCPN_SIZE];
	int retval;

	wlc = wlc_hw->wlc;
	wlc_dngl_ol = wlc->wlc_dngl_ol;

	key = wlc_keymgmt_get_scb_key(wlc->keymgmt, &wlc_dngl_ol->scb[0],
		WLC_KEY_ID_PAIRWISE,  WLC_KEY_FLAG_NONE, &key_info);
	if (key_info.algo != CRYPTO_ALGO_TKIP)
		return;

	wlc_bmac_copyfrom_shm(wlc_hw, M_REPCNT_TID, iv_rc, sizeof(iv_rc));

	retval = wlc_key_advance_seq(key, iv_rc, sizeof(iv_rc), 0, TRUE);
	if (retval != BCME_OK) {
		WL_ERROR(("wl%d: %s: err %d from wlc_key_advance_seq\n",
			WLCWLUNIT(wlc), __FUNCTION__, retval));
	}

	/* temporarily enable h/w updates - as phase1 key needs updating */
	key_info.flags &= ~WLC_KEY_FLAG_NO_HW_UPDATE;
	retval = wlc_key_set_flags(key, key_info.flags);
	if (retval != BCME_OK) {
		WL_ERROR(("wl%d: %s: err %d clearing key idx %d hw upd flag\n",
			WLCWLUNIT(wlc), __FUNCTION__, retval, key_info.key_idx));
	}

	key_info.flags |= WLC_KEY_FLAG_NO_HW_UPDATE;
	retval = wlc_key_set_flags(key, key_info.flags);
	if (retval != BCME_OK) {
		WL_ERROR(("wl%d: %s: err %d setting key idx %d hw update flag\n",
			WLCWLUNIT(wlc), __FUNCTION__, retval, key_info.key_idx));
	}
}

bool BCMFASTPATH
wlc_bmac_txstatus_shm(wlc_hw_info_t *wlc_hw, bool bound, bool *fatal)
{
	uint16 s1, s2, s3, s4, s5;
	uint16 status_bits;
	uint16 ncons;
	tx_status_t txs;
	uint16 base;
	uint16 wptr;
	base = wlc_hw->txs_rptr * 2;
	wptr = wlc_bmac_read_shm(wlc_hw, (wlc_hw->pso_blk + M_TXS_FIFO_WPTR));

	if ((wptr < wlc_hw->txs_addr_blk) || (wptr > wlc_hw->txs_addr_end)) {

		WL_ERROR(("%s : Error : Invalid wptr. Out of range.\n",
			__FUNCTION__));

		WL_ERROR(("%s : rptr:0x%x wptr:0x%x block start:0x%x\n",
			__FUNCTION__, wlc_hw->txs_rptr, wptr, wlc_hw->txs_addr_blk));
		ASSERT(wptr >= wlc_hw->txs_addr_blk);
		ASSERT(wptr <= wlc_hw->txs_addr_end);

		return FALSE;
	}

	while (!(*fatal) &&
		(wlc_hw->txs_rptr != wptr) &&
		((s1 = wlc_bmac_read_shm(wlc_hw, base)) & TXS_V)) {
		s2 = wlc_bmac_read_shm(wlc_hw, base + 2);
		s3 = wlc_bmac_read_shm(wlc_hw, base + 4);
		s4 = wlc_bmac_read_shm(wlc_hw, base + 6);
		s5 = wlc_bmac_read_shm(wlc_hw, base + 8);
		wlc_hw->txs_rptr += M_TXS_SIZE/2;
		WL_TRACE(("%s: updated rptr:0x%x\n", __FUNCTION__, wlc_hw->txs_rptr));
		if (s1 == 0xffffffff) {
				WL_ERROR(("%s: dead chip\n", __FUNCTION__));
				ASSERT(s1 != 0xffffffff);
		}

		txs.frameid = s2;
		txs.sequence = s3;
		txs.phyerr = s4;
		status_bits = s1;
		txs.status.raw_bits = status_bits;
		txs.status.is_intermediate = (status_bits & TX_STATUS40_INTERMEDIATE) != 0;
		txs.status.pm_indicated = (status_bits & TX_STATUS40_PMINDCTD) != 0;

		ncons = ((status_bits & TX_STATUS40_NCONS) >> TX_STATUS40_NCONS_SHIFT);
		txs.status.was_acked = ((ncons <= 1) ?
			((status_bits & TX_STATUS40_ACK_RCV) != 0) : TRUE);
		txs.status.suppr_ind =
			(status_bits & TX_STATUS40_SUPR) >> TX_STATUS40_SUPR_SHIFT;
		/* handle roll over scenario on the dongle
		* suppressed status  is 9 for suppressed frames
		*/
		if (txs.status.suppr_ind == TX_STATUS_SUPR_PHASE1_KEY) {
			wlc_bmac_update_iv(wlc_hw, &txs);
		}

		if (txs.status.suppr_ind != TX_STATUS_SUPR_NONE)
			wlc_dngl_cntinc(wlc_hw->wlc->wlc_dngl_ol, TXSUPPRESS);
		else if (txs.status.was_acked)
			wlc_dngl_cntinc(wlc_hw->wlc->wlc_dngl_ol, TXACKED);

		if (wlc_macol_frameid_match(wlc_hw, txs.frameid))
			wlc_macol_dequeue_dma_pkt(wlc_hw, &txs);
		else
			*fatal = wlc_bmac_dotxstatus(wlc_hw, &txs, s2);

		if (wlc_hw->txs_rptr >= wlc_hw->txs_addr_end) {
			wlc_hw->txs_rptr = wlc_hw->txs_addr_blk;
			WL_INFORM(("wrap_up:rptr:0x%x wrptr: 0x%x\n", wlc_hw->txs_rptr, wptr));
		}
		wlc_bmac_write_shm(wlc_hw, (wlc_hw->pso_blk + M_TXS_FIFO_RPTR), wlc_hw->txs_rptr);
		base = wlc_hw->txs_rptr * 2;

#ifdef BCMDBG
		if (*fatal) {
			WL_ERROR(("wl%d: %s:: bad txstatus %08X %08X %08X %08X %08X\n",
				wlc_hw->unit, __FUNCTION__,
				s1, s2, s3, s4, s5));
			break;
		}
#endif // endif
	}
	return FALSE;
}

#endif /* BCM_OL_DEV */

/**
 * process tx completion events in BMAC
 * Return TRUE if more tx status need to be processed. FALSE otherwise.
 */
bool BCMFASTPATH
wlc_bmac_txstatus(wlc_hw_info_t *wlc_hw, bool bound, bool *fatal)
{
	bool morepending = FALSE;
	wlc_info_t *wlc = wlc_hw->wlc;

	WL_TRACE(("wl%d: wlc_bmac_txstatus\n", wlc_hw->unit));

	if (D11REV_IS(wlc_hw->corerev, 4)) {
		/* to retire soon */
		*fatal = wlc_bmac_txstatus_corerev4(wlc->hw);

		if (*fatal)
			return 0;
	} else if (D11REV_LT(wlc_hw->corerev, 40)) {
		/* corerev >= 5 && < 40 */
		d11regs_t *regs = wlc_hw->regs;
		osl_t *osh = wlc_hw->osh;
		tx_status_t txs;
		uint32 s1, s2;
		uint16 status_bits;
		uint n = 0;
		/* Param 'max_tx_num' indicates max. # tx status to process before break out. */
		uint max_tx_num = bound ? wlc->pub->tunables->txsbnd : -1;
		uint32 tsf_time = 0;
#ifdef WLFCTS
		uint8 status_delay;
		if (WLFCTS_ENAB(wlc->pub)) {
			ASSERT(D11REV_GE(wlc_hw->corerev, 26));
			ASSERT(wlc_bmac_mhf_get(wlc_hw, MHF2, WLC_BAND_AUTO) & MHF2_TX_TMSTMP);
		}
#endif /* WLFCTS */

		WL_TRACE(("wl%d: %s: ltrev40\n", wlc_hw->unit, __FUNCTION__));

		/* To avoid overhead time is read only once for the whole while loop
		 * since time accuracy is not a concern for now.
		 */
#ifdef WLFCTS
		if (!WLFCTS_ENAB(wlc->pub))
#endif /* !WLFCTS */
		{
			tsf_time = R_REG(osh, &regs->tsf_timerlow);
			txs.dequeuetime = 0;
		}

		while (!(*fatal) && (s1 = R_REG(osh, &regs->frmtxstatus)) & TXS_V) {
			if (s1 == 0xffffffff) {
				WL_ERROR(("wl%d: %s: dead chip\n", wlc_hw->unit, __FUNCTION__));
				ASSERT(s1 != 0xffffffff);
				WL_HEALTH_LOG(wlc_hw->wlc, DEADCHIP_ERROR);
				return morepending;
			}

			s2 = R_REG(osh, &regs->frmtxstatus2);

#ifdef WLFCTS
			if (WLFCTS_ENAB(wlc->pub)) {
				/* For corerevs >= 26, the first txstatus package contains
				 * 32-bit timestamps for dequeue_time and last_tx_time
				 */
				txs.dequeuetime = s1;
				tsf_time = s2;

				/* wait till the next 8 bytes of txstatus is available */
				status_delay = 0;
				while (((s1 = R_REG(osh, &regs->frmtxstatus)) & TXS_V) == 0) {
					OSL_DELAY(1);
					status_delay++;
					if (status_delay > 10) {
						ASSERT(0);
						return 0;
					}
				}
				s2 = R_REG(osh, &regs->frmtxstatus2);
			}
#endif /* WLFCTS */

			WL_PRHDRS_MSG(("wl%d: %s: Raw txstatus s1 0x%0X s2 0x%0X\n",
				wlc_hw->unit, __FUNCTION__, s1, s2));

			status_bits = (s1 & TXS_STATUS_MASK);
			txs.status.raw_bits = status_bits;
			txs.status.was_acked = (status_bits & TX_STATUS_ACK_RCV) != 0;
			txs.status.is_intermediate = (status_bits & TX_STATUS_INTERMEDIATE) != 0;
			txs.status.pm_indicated = (status_bits & TX_STATUS_PMINDCTD) != 0;
			txs.status.suppr_ind =
			        (status_bits & TX_STATUS_SUPR_MASK) >> TX_STATUS_SUPR_SHIFT;
			txs.status.rts_tx_cnt =
			        ((s1 & TX_STATUS_RTS_RTX_MASK) >> TX_STATUS_RTS_RTX_SHIFT);
			txs.status.frag_tx_cnt =
			        ((s1 & TX_STATUS_FRM_RTX_MASK) >> TX_STATUS_FRM_RTX_SHIFT);
			txs.frameid = (s1 & TXS_FID_MASK) >> TXS_FID_SHIFT;
			txs.sequence = s2 & TXS_SEQ_MASK;
			txs.phyerr = (s2 & TXS_PTX_MASK) >> TXS_PTX_SHIFT;
			txs.lasttxtime = tsf_time;

			*fatal = wlc_bmac_dotxstatus(wlc_hw, &txs, s2);

			/* !give others some time to run! */
			if (++n >= max_tx_num)
				break;
		}

		if (*fatal)
			return 0;

		if (n >= max_tx_num)
			morepending = TRUE;
	} else {
		/* corerev >= 40 */
		d11regs_t *regs = wlc_hw->regs;
		osl_t *osh = wlc_hw->osh;
		tx_status_t txs;
		/* pkg 1 */
		uint32 v_s1, v_s2, v_s3, v_s4;
		/* pkg 2 */
		uint32 v_s5, v_s6, v_s7, v_s8;
		uint16 status_bits;
		uint n = 0;
		uint16 ncons;

		/* Param 'max_tx_num' indicates max. # tx status to process before break out. */
		uint max_tx_num = bound ? wlc->pub->tunables->txsbnd : -1;
		uint32 tsf_time;
#ifdef HOST_HDR_FETCH
		/* Check if d11 buffers are available to hold txheaders */
		/* If not, delay txstatus processing till more buffers become available */
		if (bound && (HOST_HDR_FETCH_ENAB()) &&
			(lfbufpool_avail(D11_EXT_LFRAG_BUF_POOL)) == 0) {
			/* For the bounded processing, wlc_bmac_txstatus can return
			 * without processing all pending txstatus from MAC
			 */
			return TRUE;
		}
#endif /* HOST_HDR_FETCH */
#ifdef WLFCTS
		if (WLFCTS_ENAB(wlc->pub)) {
			ASSERT(D11REV_GE(wlc_hw->corerev, 26));
			ASSERT(wlc_bmac_mhf_get(wlc_hw, MHF2, WLC_BAND_AUTO) & MHF2_TX_TMSTMP);
		}
#endif /* WLFCTS */

		/* To avoid overhead time is read only once for the whole while loop
		 * since time accuracy is not a concern for now.
		 */
		tsf_time = R_REG(osh, &regs->tsf_timerlow);
		WL_TRACE(("wl%d: %s: rev40\n", wlc_hw->unit, __FUNCTION__));

		while (!(*fatal) && (v_s1 = R_REG(osh, &regs->frmtxstatus)) & TXS_V) {
			if (v_s1 == 0xffffffff) {
				WL_ERROR(("wl%d: %s: dead chip\n", wlc_hw->unit, __FUNCTION__));
				ASSERT(v_s1 != 0xffffffff);
				return morepending;
			}

			v_s2 = R_REG(osh, &regs->frmtxstatus2);
			v_s3 = R_REG(osh, &regs->frmtxstatus3);
			v_s4 = R_REG(osh, &regs->frmtxstatus4);
			WL_TRACE(("%s: s1=%0x ampdu=%d\n", __FUNCTION__, v_s1,
				((v_s1 & 0x4) != 0)));
			txs.frameid = (v_s1 & TXS_FID_MASK) >> TXS_FID_SHIFT;
			txs.sequence = v_s2 & TXS_SEQ_MASK;
			txs.phyerr = (v_s2 & TXS_PTX_MASK) >> TXS_PTX_SHIFT;
			txs.lasttxtime = tsf_time;
			status_bits = v_s1 & TXS_STATUS_MASK;
			txs.status.raw_bits = status_bits;
			txs.status.is_intermediate = (status_bits & TX_STATUS40_INTERMEDIATE) != 0;
			txs.status.pm_indicated = (status_bits & TX_STATUS40_PMINDCTD) != 0;

			ncons = ((status_bits & TX_STATUS40_NCONS) >> TX_STATUS40_NCONS_SHIFT);
			txs.status.was_acked = ((ncons <= 1) ?
				((status_bits & TX_STATUS40_ACK_RCV) != 0) : TRUE);
			txs.status.suppr_ind =
			        (status_bits & TX_STATUS40_SUPR) >> TX_STATUS40_SUPR_SHIFT;

			/* pkg 2 comes always */
			v_s5 = R_REG(osh, &regs->frmtxstatus);
			v_s6 = R_REG(osh, &regs->frmtxstatus2);
			v_s7 = R_REG(osh, &regs->frmtxstatus3);
			v_s8 = R_REG(osh, &regs->frmtxstatus4);
			WL_TRACE(("wl%d: %s calls dotxstatus\n", wlc_hw->unit, __FUNCTION__));

			WL_PRHDRS_MSG(("wl%d: %s %08X %08X %08X %08X || "
				"%08X %08X %08X %08X\n",
				wlc_hw->unit, __FUNCTION__,
				v_s1, v_s2, v_s3, v_s4, v_s5, v_s6, v_s7, v_s8));

			/* store saved extras (check valid pkg ) */
			if ((v_s5 & TXS_V) == 0) {
				/* if not a valid package, assert and bail */
				WL_ERROR(("wl%d: %s: package read not valid\n",
				          wlc_hw->unit, __FUNCTION__));
				ASSERT(v_s5 != 0xffffffff);
				return morepending;
			}
			txs.status.s3 = v_s3;
			txs.status.s4 = v_s4;
			txs.status.s5 = v_s5;
			txs.status.ack_map1 = v_s6;
			txs.status.ack_map2 = v_s7;
			txs.status.s8 = v_s8;

			txs.status.rts_tx_cnt =
			        ((v_s5 & TX_STATUS40_RTS_RTX_MASK) >> TX_STATUS40_RTS_RTX_SHIFT);
			txs.status.cts_rx_cnt =
			        ((v_s5 & TX_STATUS40_CTS_RRX_MASK) >> TX_STATUS40_CTS_RRX_SHIFT);

			if ((v_s5 & TX_STATUS64_MUTX)) {
				/* Only RT0 entry is used for frag_tx_cnt in ucode */
				txs.status.frag_tx_cnt = TX_STATUS40_TXCNT_RT0(v_s3);
			} else {
				/* XXX: Need to be recalculated to "txs->status.s3 & 0xffff"
				 * if this tx was fixed rate.
				 * The recalculation is done in wlc_dotxstatus() as we need
				 * TX descriptor from pkt ptr to know if it was fixed rate or not.
				 */
				txs.status.frag_tx_cnt = TX_STATUS40_TXCNT(v_s3, v_s4);
			}

#ifdef WLFCTS
			if (WLFCTS_ENAB(wlc->pub)) {
				uint32 tsf_time_lo16 = txs.lasttxtime & 0x0000ffff;
				uint32 lasttxtime_lo16 = (v_s8 >> 16) & 0x0000ffff;
				uint32 dequeuetime_lo16 = v_s8 & 0x0000ffff;

				txs.lasttxtime &= 0xffff0000;
				txs.dequeuetime = txs.lasttxtime | dequeuetime_lo16;
				txs.lasttxtime = txs.lasttxtime | lasttxtime_lo16;
				if (dequeuetime_lo16 > tsf_time_lo16)
					txs.dequeuetime -= 0x10000;
				if (lasttxtime_lo16 > tsf_time_lo16)
					txs.lasttxtime -= 0x10000;
			}
#endif /* WLFCTS */
#ifdef HOST_HDR_FETCH
			/* Initialize head_pkt to NULL in the first go.
			 * Will be initialilzed if a fetch of txhdr is required
			 */
			txs.head_pkt = NULL;

			/* Tag bounded info in the txs.
			 * Bounded request is from MAC interrupt
			 * Unbounded request is form flush routine
			 */
			if (HOST_HDR_FETCH_ENAB()) {
				txs.flush_rqst = (bound) ? FALSE : TRUE;
			}
#endif // endif
			if (wlc_macol_frameid_match(wlc_hw, txs.frameid))
				wlc_macol_dequeue_dma_pkt(wlc_hw, &txs);
			else
				*fatal = wlc_bmac_dotxstatus(wlc_hw, &txs, v_s2);

			/* !give others some time to run! */
#ifdef PROP_TXSTATUS
			/* We must drain out in case of suppress, to avoid Out of Orders */
			if (txs.status.suppr_ind == TX_STATUS_SUPR_NONE)
#endif // endif
				if (++n >= max_tx_num)
					break;
#ifdef HOST_HDR_FETCH
			/* Break the tx status processing if no more d11 buffers */
			if (bound && (HOST_HDR_FETCH_ENAB()) &&
				(lfbufpool_avail(D11_EXT_LFRAG_BUF_POOL)) == 0) {
				/* For the bounded processing, wlc_bmac_txstatus can return
				 * without processing all pending txstatus from MAC
				 */
				morepending = TRUE;
				break;
			}
#endif /* HOST_HDR_FETCH */
		}

		if (*fatal) {
			WL_ERROR(("error %d caught in %s\n", *fatal, __FUNCTION__));
			return 0;
		}

		if (n >= max_tx_num)
			morepending = TRUE;
	}

#ifdef WLC_HIGH
	if (wlc->active_queue != NULL && WLC_TXQ_OCCUPIED(wlc)) {
		WLDURATION_ENTER(wlc, DUR_DPC_TXSTATUS_SENDQ);
		wlc_send_q(wlc, wlc->active_queue);
		WLDURATION_EXIT(wlc, DUR_DPC_TXSTATUS_SENDQ);
	}
#endif // endif

	return morepending;
}

#if defined(STA) && defined(WLRM)
static uint16
wlc_bmac_read_ihr(wlc_hw_info_t *wlc_hw, uint offset)
{
	uint16 v;
	wlc_bmac_copyfrom_objmem(wlc_hw, offset << 2, &v,
		sizeof(v), OBJADDR_IHR_SEL);
	return v;
}
#endif  /* STA && WLRM */

void
wlc_bmac_write_ihr(wlc_hw_info_t *wlc_hw, uint offset, uint16 v)
{
	wlc_bmac_copyto_objmem(wlc_hw, offset<<2, &v, sizeof(v), OBJADDR_IHR_SEL);
}

void
wlc_bmac_suspend_mac_and_wait(wlc_hw_info_t *wlc_hw)
{
	wlc_info_t *wlc = wlc_hw->wlc;
	d11regs_t *regs = wlc_hw->regs;
	osl_t *osh = wlc_hw->osh;
	uint32 mc, mi;

	WL_TRACE(("wl%d: wlc_bmac_suspend_mac_and_wait: bandunit %d\n", wlc_hw->unit,
		wlc_hw->band->bandunit));

	/*
	 * Track overlapping suspend requests
	 */
	wlc_hw->mac_suspend_depth++;
	if (wlc_hw->mac_suspend_depth > 1) {
		mc = R_REG(osh, &regs->maccontrol);
		if (mc & MCTL_EN_MAC) {
			WL_PRINT(("%s ERROR: suspend_depth %d maccontrol 0x%x\n",
				__FUNCTION__, wlc_hw->mac_suspend_depth, mc));
#ifdef WLC_HIGH
			wlc_dump_ucode_fatal(wlc, PSM_FATAL_SUSP);
#endif // endif
			ASSERT_PSM_UCODE(!(mc & MCTL_EN_MAC), PSM_FATAL_SUSP, wlc);
		}
		WL_TRACE(("wl%d: %s: bail: mac_suspend_depth=%d\n", wlc_hw->unit,
			__FUNCTION__, wlc_hw->mac_suspend_depth));
		return;
	}

#ifdef STA
	/* force the core awake */
	wlc_ucode_wake_override_set(wlc_hw, WLC_WAKE_OVERRIDE_MACSUSPEND);

	if ((wlc_hw->btc->wire >= WL_BTC_3WIRE) &&
	    D11REV_LT(wlc_hw->corerev, 13)) {
		si_gpiocontrol(wlc_hw->sih, wlc_hw->btc->gpio_mask, 0, GPIO_DRV_PRIORITY);
	}
#endif /* STA */

	mc = R_REG(osh, &regs->maccontrol);

#ifdef WLC_HIGH	/* BMAC: skip DEVICEREMOVED, not needed in low level driver. */
	if (mc == 0xffffffff) {
		WL_ERROR(("wl%d: %s: dead chip\n", wlc_hw->unit, __FUNCTION__));
		WL_HEALTH_LOG(wlc, DEADCHIP_ERROR);
		wl_down(wlc->wl);
		return;
	}
#endif // endif
	ASSERT_PSM_UCODE(!(mc & MCTL_PSM_JMP_0), PSM_FATAL_SUSP, wlc);
	ASSERT_PSM_UCODE(mc & MCTL_PSM_RUN, PSM_FATAL_SUSP, wlc);
	ASSERT_PSM_UCODE(mc & MCTL_EN_MAC, PSM_FATAL_SUSP, wlc);

	mi = R_REG(osh, &regs->macintstatus);
#ifdef WLC_HIGH	/* BMAC: skip DEVICEREMOVED, not needed in low level driver. */
	if (mi == 0xffffffff) {
		WL_ERROR(("wl%d: %s: dead chip\n", wlc_hw->unit, __FUNCTION__));
		WL_HEALTH_LOG(wlc, DEADCHIP_ERROR);
		wl_down(wlc->wl);
		return;
	}
#endif // endif
#ifdef WAR4360_UCODE
	if (mi & MI_MACSSPNDD) {
		WL_ERROR(("wl%d:%s: Hammering due to (mc & MI_MACSPNDD)\n",
			wlc_hw->unit, __FUNCTION__));
		wlc_hw->need_reinit = 4;
#ifdef WLOFFLD
		if (WLOFFLD_ENAB(wlc_hw->wlc->pub))
			wlc_ol_down(wlc_hw->wlc->ol);
#endif // endif
		return;
	}
#endif /* WAR4360_UCODE */
	ASSERT_PSM_UCODE(!(mi & MI_MACSSPNDD), PSM_FATAL_SUSP, wlc);

	wlc_bmac_mctrl(wlc_hw, MCTL_EN_MAC, 0);

	SPINWAIT(!(R_REG(osh, &regs->macintstatus) & MI_MACSSPNDD), WLC_MAX_MAC_SUSPEND);

#ifdef WL11ULB
	if (ULB_ENAB(wlc_hw->wlc->pub)) {
		if (CHSPEC_IS10(wlc_hw->chanspec)) {
			SPINWAIT(!(R_REG(osh, &regs->macintstatus) & MI_MACSSPNDD),
				WLC_MAX_MAC_SUSPEND);
		} else if (CHSPEC_IS5(wlc_hw->chanspec)) {
			SPINWAIT(!(R_REG(osh, &regs->macintstatus) & MI_MACSSPNDD),
				WLC_MAX_MAC_SUSPEND);
			SPINWAIT(!(R_REG(osh, &regs->macintstatus) & MI_MACSSPNDD),
				WLC_MAX_MAC_SUSPEND);
			SPINWAIT(!(R_REG(osh, &regs->macintstatus) & MI_MACSSPNDD),
				WLC_MAX_MAC_SUSPEND);
		}
	}
#endif /* WL11ULB */

	if (!(R_REG(osh, &regs->macintstatus) & MI_MACSSPNDD)) {
		WLC_EXTLOG(wlc, LOG_MODULE_COMMON, FMTSTR_SUSPEND_MAC_FAIL_ID,
			WL_LOG_LEVEL_ERR, 0, R_REG(osh, &regs->psmdebug), NULL);
		WLC_EXTLOG(wlc, LOG_MODULE_COMMON, FMTSTR_REG_PRINT_ID, WL_LOG_LEVEL_ERR,
			0, R_REG(osh, &regs->phydebug), "phydebug");
		WLC_EXTLOG(wlc, LOG_MODULE_COMMON, FMTSTR_REG_PRINT_ID, WL_LOG_LEVEL_ERR,
			0, R_REG(osh, &regs->psm_brc), "psm_brc");
		WL_PRINT(("wl%d: wlc_bmac_suspend_mac_and_wait: waited %d uS and "
			 "MI_MACSSPNDD is still not on.\n",
			 wlc_hw->unit, WLC_MAX_MAC_SUSPEND));
#ifdef WLC_HIGH
		wlc_dump_ucode_fatal(wlc, PSM_FATAL_SUSP);
#endif // endif
		WL_HEALTH_LOG(wlc, MACSPEND_TIMOUT);
#ifdef WAR4360_UCODE
		WL_PRINT(("wl%d:%s: Hammering due to SPINWAIT timeout\n",
			wlc_hw->unit, __FUNCTION__));
		wlc_hw->need_reinit = 5;
#ifdef WLOFFLD
		if (WLOFFLD_ENAB(wlc->pub))
			wlc_ol_down(wlc->ol);
#endif // endif
		return;
#endif /* WAR4360_UCODE */
	}

	mc = R_REG(osh, &regs->maccontrol);
#ifdef WLC_HIGH	/* BMAC: skip DEVICEREMOVED, not needed in low level driver. */
	if (mc == 0xffffffff) {
		WL_ERROR(("wl%d: %s: dead chip\n", wlc_hw->unit, __FUNCTION__));
		WL_HEALTH_LOG(wlc, DEADCHIP_ERROR);
		wl_down(wlc->wl);
		return;
	}
#endif // endif
	ASSERT_PSM_UCODE(!(mc & MCTL_PSM_JMP_0), PSM_FATAL_SUSP, wlc);
	ASSERT_PSM_UCODE(mc & MCTL_PSM_RUN, PSM_FATAL_SUSP, wlc);
	ASSERT_PSM_UCODE(!(mc & MCTL_EN_MAC), PSM_FATAL_SUSP, wlc);
	/* PR95192, WAR for NTGR problem where we stopped the PSM after suspending the ucode
	 * (the problem happens while ucode is suspended).  Only for 5357 and 53572 chips.
	 */
	if (((CHIPID(wlc_hw->sih->chip)) == BCM5357_CHIP_ID) ||
	    ((CHIPID(wlc_hw->sih->chip)) == BCM53572_CHIP_ID)) {
	    wlc_bmac_mctrl(wlc_hw, MCTL_PSM_RUN, 0);
	}

#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(BCMDBG_PHYDUMP)
	{
	    bmac_suspend_stats_t* stats = wlc_hw->suspend_stats;

	    stats->suspend_start = R_REG(osh, &regs->tsf_timerlow);
	    stats->suspend_count++;

	    if (stats->suspend_start > stats->suspend_end) {
			uint32 unsuspend_time = (stats->suspend_start - stats->suspend_end)/100;
			stats->unsuspended += unsuspend_time;
			WL_TRACE(("wl%d: bmac now suspended; time spent active was %d ms\n",
			           wlc_hw->unit, (unsuspend_time + 5)/10));
	    }
	}
#endif /* BCMDBG || BCMDBG_DUMP || BCMDBG_PHYDUMP */
} /* wlc_bmac_suspend_mac_and_wait */

void
wlc_bmac_enable_mac(wlc_hw_info_t *wlc_hw)
{
	d11regs_t *regs = wlc_hw->regs;
	uint32 mc, mi, dbgst;
	osl_t *osh;

	WL_TRACE(("wl%d: wlc_bmac_enable_mac: bandunit %d\n",
		wlc_hw->unit, wlc_hw->band->bandunit));
#ifdef WAR4360_UCODE
	if (wlc_hw->need_reinit)
		return;
#endif // endif
	/*
	 * Track overlapping suspend requests
	 */
	ASSERT(wlc_hw->mac_suspend_depth > 0);
	wlc_hw->mac_suspend_depth--;
	if (wlc_hw->mac_suspend_depth > 0)
		return;

	osh = wlc_hw->osh;

	mc = R_REG(osh, &regs->maccontrol);
	ASSERT(!(mc & MCTL_PSM_JMP_0));
	ASSERT(!(mc & MCTL_EN_MAC));
	/* PR95192, WAR for NTGR problem where we stopped the PSM after suspending the ucode
	 * (the problem happens while ucode is suspended).  Only for 5357 and 53572 chips.
	 */
	if (((CHIPID(wlc_hw->sih->chip)) != BCM5357_CHIP_ID) &&
	    ((CHIPID(wlc_hw->sih->chip)) != BCM53572_CHIP_ID)) {
		ASSERT(mc & MCTL_PSM_RUN);
	}

	/* FIXME: The following should be a valid assert, except that we bail out
	 * of the spin loop in wlc_bmac_suspend_mac_and_wait.
	 *
	 * mi = R_REG(osh, &regs->macintstatus);
	 * ASSERT(mi & MI_MACSSPNDD);
	 */

	/* PR95192, WAR for NTGR problem where we stopped the PSM after suspending the ucode
	 * (the problem happens while ucode is suspended).  Only for 5357 and 53572 chips.
	 */
	if (((CHIPID(wlc_hw->sih->chip)) == BCM5357_CHIP_ID) ||
	    ((CHIPID(wlc_hw->sih->chip)) == BCM53572_CHIP_ID)) {
		wlc_bmac_mctrl(wlc_hw, (MCTL_EN_MAC | MCTL_PSM_RUN), (MCTL_EN_MAC | MCTL_PSM_RUN));
	} else {
		wlc_bmac_mctrl(wlc_hw, MCTL_EN_MAC, MCTL_EN_MAC);
	}

	W_REG(osh, &regs->macintstatus, MI_MACSSPNDD);

	mc = R_REG(osh, &regs->maccontrol);
	ASSERT(!(mc & MCTL_PSM_JMP_0));
	ASSERT(mc & MCTL_EN_MAC);
	ASSERT(mc & MCTL_PSM_RUN);
	BCM_REFERENCE(mc);

	mi = R_REG(osh, &regs->macintstatus);
	ASSERT(!(mi & MI_MACSSPNDD));
	BCM_REFERENCE(mi);

	SPINWAIT(((dbgst = wlc_bmac_read_shm(wlc_hw, M_UCODE_DBGST)) == DBGST_SUSPENDED),
			WLC_MAX_MAC_ENABLE);
	if (dbgst == DBGST_SUSPENDED) {
		WL_PRINT(("wl%d: %s: waited %d uS and M_UCODE_DBGST is still DBGST_SUSPENDED.\n",
			 wlc_hw->unit, __FUNCTION__, WLC_MAX_MAC_ENABLE));
#ifdef WLC_HIGH
		wlc_dump_ucode_fatal(wlc_hw->wlc, PSM_FATAL_WAKE);
#endif // endif
		return;
	}

#ifdef STA
	wlc_ucode_wake_override_clear(wlc_hw, WLC_WAKE_OVERRIDE_MACSUSPEND);

	if ((wlc_hw->btc->wire >= WL_BTC_3WIRE) &&
	    D11REV_LT(wlc_hw->corerev, 13)) {
		si_gpiocontrol(wlc_hw->sih, wlc_hw->btc->gpio_mask, wlc_hw->btc->gpio_mask,
		               GPIO_DRV_PRIORITY);
	}
#endif /* STA */

#if defined(WLC_HIGH) && defined(WLC_LOW)
	if (MBSS_SUPPORT(wlc_hw->wlc->pub)) {
		wlc_mbss_info_t *mbss = wlc_hw->wlc->mbss;

		/* The PRQ fifo is reset on a mac suspend/resume; reset the SW read ptr */
		mbss->prq_rd_ptr = mbss->prq_base;
	}
#endif /* WLC_HIGH && WLC_LOW */

#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(BCMDBG_PHYDUMP)
	{
	    bmac_suspend_stats_t* stats = wlc_hw->suspend_stats;

	    stats->suspend_end = R_REG(osh, &regs->tsf_timerlow);

	    if (stats->suspend_end > stats->suspend_start) {
			uint32 suspend_time = (stats->suspend_end - stats->suspend_start)/100;

			if (suspend_time > stats->suspend_max) {
				stats->suspend_max = suspend_time;
			}
			stats->suspended += suspend_time;
			WL_TRACE(("wl%d: bmac now active; time spent suspended was %d ms\n",
			           wlc_hw->unit, (suspend_time + 5)/10));
	    }
	}
#endif /* BCMDBG || BCMDBG_DUMP || BCMDBG_PHYDUMP */
}

#if defined(WL_PSMX)
void
wlc_bmac_suspend_macx_and_wait(wlc_hw_info_t *wlc_hw)
{
	d11regs_t *regs = wlc_hw->regs;
	osl_t *osh = wlc_hw->osh;
	uint32 mcx, mix;
#ifdef WLC_HIGH
	wlc_info_t *wlc = wlc_hw->wlc;
#endif /* WLC_HIGH */

	WL_TRACE(("wl%d: %s: bandunit %d\n", wlc_hw->unit, __FUNCTION__,
		wlc_hw->band->bandunit));

	/*
	 * Track overlapping suspend requests
	 */
	wlc_hw->macx_suspend_depth++;
	if (wlc_hw->macx_suspend_depth > 1) {
		mcx = R_REG(osh, &regs->maccontrol_x);
		if (mcx & MCTL_EN_MAC) {
			WL_PRINT(("%s ERROR: suspend_depth %d maccontrol_x 0x%x\n",
				__FUNCTION__, wlc_hw->macx_suspend_depth, mcx));
#ifdef WLC_HIGH
			wlc_dump_psmx_fatal(wlc, PSMX_FATAL_SUSP);
#endif // endif
			ASSERT_PSMX_UCODE(!(mcx & MCTL_EN_MAC), PSMX_FATAL_SUSP, wlc);
		}
		WL_TRACE(("wl%d: %s: bail: macx_suspend_depth=%d\n", wlc_hw->unit,
			__FUNCTION__, wlc_hw->macx_suspend_depth));
		return;
	}

	mcx = R_REG(osh, &regs->maccontrol_x);

#ifdef WLC_HIGH	/* BMAC: skip DEVICEREMOVED, not needed in low level driver. */
	if (mcx == 0xffffffff) {
		WL_ERROR(("wl%d: %s: dead chip\n", wlc_hw->unit, __FUNCTION__));
		WL_HEALTH_LOG(wlc, DEADCHIP_ERROR);
		wl_down(wlc->wl);
		return;
	}
#endif // endif
	ASSERT_PSMX_UCODE(!(mcx & MCTL_PSM_JMP_0), PSMX_FATAL_SUSP, wlc);
	ASSERT_PSMX_UCODE(mcx & MCTL_PSM_RUN, PSMX_FATAL_SUSP, wlc);
	ASSERT_PSMX_UCODE(mcx & MCTL_EN_MAC, PSMX_FATAL_SUSP, wlc);

	mix = R_REG(osh, &regs->macintstatus_x);
#ifdef WLC_HIGH	/* BMAC: skip DEVICEREMOVED, not needed in low level driver. */
	if (mix == 0xffffffff) {
		WL_ERROR(("wl%d: %s: dead chip\n", wlc_hw->unit, __FUNCTION__));
		WL_HEALTH_LOG(wlc, DEADCHIP_ERROR);
		wl_down(wlc->wl);
		return;
	}
#endif // endif
	ASSERT_PSMX_UCODE(!(mix & MI_MACSSPNDD), PSMX_FATAL_SUSP, wlc);

	wlc_bmac_mctrlx(wlc_hw, MCTL_EN_MAC, 0);

	SPINWAIT(!(R_REG(osh, &regs->macintstatus_x) & MI_MACSSPNDD),
		WLC_MAX_MAC_SUSPEND);

	if (!(R_REG(osh, &regs->macintstatus_x) & MI_MACSSPNDD)) {
		WL_PRINT(("wl%d: %s: waited %d uS and "
			 "MI_MACSSPNDD is still not on.\n",
			 wlc_hw->unit, __FUNCTION__, WLC_MAX_MAC_SUSPEND));
#ifdef WLC_HIGH
		wlc_dump_psmx_fatal(wlc, PSMX_FATAL_SUSP);
#endif // endif
		WL_HEALTH_LOG(wlc, MACSPEND_TIMOUT);
	}

	mcx = R_REG(osh, &regs->maccontrol_x);
#ifdef WLC_HIGH	/* BMAC: skip DEVICEREMOVED, not needed in low level driver. */
	if (mcx == 0xffffffff) {
		WL_ERROR(("wl%d: %s: dead chip\n", wlc_hw->unit, __FUNCTION__));
		WL_HEALTH_LOG(wlc, DEADCHIP_ERROR);
		wl_down(wlc->wl);
		return;
	}
#endif // endif
	ASSERT_PSMX_UCODE(!(mcx & MCTL_PSM_JMP_0), PSMX_FATAL_SUSP, wlc);
	ASSERT_PSMX_UCODE(mcx & MCTL_PSM_RUN, PSMX_FATAL_SUSP, wlc);
	ASSERT_PSMX_UCODE(!(mcx & MCTL_EN_MAC), PSMX_FATAL_SUSP, wlc);
} /* wlc_bmac_suspend_macx_and_wait */

void
wlc_bmac_enable_macx(wlc_hw_info_t *wlc_hw)
{
	d11regs_t *regs = wlc_hw->regs;
	uint32 mcx, mix;
	osl_t *osh;

	ASSERT(D11REV_GE(wlc_hw->corerev, 64));
	WL_TRACE(("wl%d: %s: bandunit %d\n",
		wlc_hw->unit, __FUNCTION__, wlc_hw->band->bandunit));
	/*
	 * Track overlapping suspend requests
	 */
	ASSERT(wlc_hw->macx_suspend_depth > 0);
	wlc_hw->macx_suspend_depth--;
	if (wlc_hw->macx_suspend_depth > 0) {
		return;
	}

	osh = wlc_hw->osh;
	mcx = R_REG(osh, &regs->maccontrol_x);
	ASSERT(!(mcx & MCTL_PSM_JMP_0));
	ASSERT(!(mcx & MCTL_EN_MAC));

	wlc_bmac_mctrlx(wlc_hw, MCTL_EN_MAC, MCTL_EN_MAC);

	W_REG(osh, &regs->macintstatus_x, MI_MACSSPNDD);

	mcx = R_REG(osh, &regs->maccontrol_x);

	ASSERT(!(mcx & MCTL_PSM_JMP_0));
	ASSERT(mcx & MCTL_EN_MAC);
	ASSERT(mcx & MCTL_PSM_RUN);

	BCM_REFERENCE(mcx);

	mix = R_REG(osh, &regs->macintstatus_x);

	ASSERT(!(mix & MI_MACSSPNDD));
	BCM_REFERENCE(mix);
}
#endif /* WL_PSMX */

void
wlc_bmac_sync_macstate(wlc_hw_info_t *wlc_hw)
{
	bool wake_override = ((wlc_hw->wake_override & WLC_WAKE_OVERRIDE_MACSUSPEND) != 0);
	if (wake_override && wlc_hw->mac_suspend_depth == 1)
		wlc_bmac_enable_mac(wlc_hw);
}

void
wlc_bmac_ifsctl_vht_set(wlc_hw_info_t *wlc_hw, int ed_sel)
{
	uint16 mask, val;
	uint16 val_mask1, val_mask2;
	bool sb_ctrl, enable, err = FALSE;
	volatile uint16 *ifsctl_reg;
	osl_t *osh;
	d11regs_t *regs;
	uint16 chanspec;

	ASSERT(D11REV_GE(wlc_hw->corerev, 40));
	if (!WLCISACPHY(wlc_hw->band))
		return;

	osh = wlc_hw->osh;
	regs = wlc_hw->regs;
	ifsctl_reg = (volatile uint16 *)&regs->u.d11regs.ifs_ctl_sel_pricrs;
	mask = IFS_CTL_CRS_SEL_MASK|IFS_CTL_ED_SEL_MASK;

	if (ed_sel == AUTO) {
		val = (uint16)wlc_bmac_read_shm(wlc_hw, M_IFSCTL1);
		enable = (val & IFS_CTL_ED_SEL_MASK) ? TRUE:FALSE;
	} else {
		enable = (ed_sel == ON) ? TRUE : FALSE;
	}

	val_mask1 = enable ? 0xffff : 0x00ff;
	val_mask2 = 0xf;

	chanspec = wlc_hw->chanspec;

	switch (CHSPEC_BW(chanspec)) {
#ifdef WL11ULB
	case WL_CHANSPEC_BW_2P5:
	case WL_CHANSPEC_BW_5:
	case WL_CHANSPEC_BW_10:
#endif /* WL11ULB */
	case WL_CHANSPEC_BW_20:
		val = mask & val_mask1;
		break;

	case WL_CHANSPEC_BW_40:
		/* Secondary first */
		sb_ctrl = (chanspec & WL_CHANSPEC_CTL_SB_MASK) ==  WL_CHANSPEC_CTL_SB_L;
		val = (sb_ctrl ? 0x0202 : 0x0101) & val_mask2;
		/* Primary */
		val = ((wlc_hw->band->mhfs[MHF1] & MHF1_D11AC_DYNBW) ?
			(val ^ 0x303) : 0x303) & val_mask1;

		break;

	case WL_CHANSPEC_BW_80:
		/* Secondary first */
		sb_ctrl =
			(chanspec & WL_CHANSPEC_CTL_SB_MASK) == WL_CHANSPEC_CTL_SB_LL ||
			(chanspec & WL_CHANSPEC_CTL_SB_MASK) == WL_CHANSPEC_CTL_SB_LU;
		val = (sb_ctrl ? 0x0c0c : 0x0303) & val_mask2;
		/* Primary */
		val = ((wlc_hw->band->mhfs[MHF1] & MHF1_D11AC_DYNBW) ?
			(val ^ 0xf0f) : 0xf0f) & val_mask1;

		break;

	case WL_CHANSPEC_BW_160:
	case WL_CHANSPEC_BW_8080:
		val = 0xffff & val_mask1; /* enable all 8 sub-bands */
		if (wlc_hw->band->mhfs[MHF1] & MHF1_D11AC_DYNBW) {
			WL_ERROR(("%s: cannot enable dynbw fow bw160 for now!\n",
				__FUNCTION__));
		}
		break;

	default:
		err = TRUE;
		WL_ERROR(("Unsupported bandwidth - chanspec: 0x%04x\n",
			wlc_hw->chanspec));
		ASSERT(!"Invalid bandwidth in chanspec");
	}

	if (!err) {
		wlc_bmac_write_shm(wlc_hw, M_IFSCTL1, val);
		W_REG(osh, ifsctl_reg, val);

		/* update phyreg NsyncscramInit1:scramb_dyn_bw_en */
		wlc_acphy_set_scramb_dyn_bw_en(wlc_hw->band->pi, enable);
	}
}

void
wlc_bmac_ifsctl_edcrs_set(wlc_hw_info_t *wlc_hw, bool isht)
{
	if (!(WLCISNPHY(wlc_hw->band) && (D11REV_GE(wlc_hw->corerev, 16))) &&
	    !WLCISHTPHY(wlc_hw->band) && !WLCISACPHY(wlc_hw->band))
		return;

	if (isht) {
		if (WLCISNPHY(wlc_hw->band) &&
			NREV_LT(wlc_hw->band->phyrev, 3)) {
			wlc_bmac_ifsctl1_regshm(wlc_hw, IFS_CTL1_EDCRS, 0);
		}
	} else {
		/* enable EDCRS for non-11n association */
		wlc_bmac_ifsctl1_regshm(wlc_hw, IFS_CTL1_EDCRS, IFS_CTL1_EDCRS);
	}
	if (WLCISHTPHY(wlc_hw->band) ||
	    (WLCISNPHY(wlc_hw->band) && NREV_GE(wlc_hw->band->phyrev, 3))) {
		if (CHSPEC_IS20(wlc_hw->chanspec)) {
			/* 20 mhz, use 20U ED only */
			wlc_bmac_ifsctl1_regshm(wlc_hw,
				(IFS_CTL1_EDCRS | IFS_CTL1_EDCRS_20L | IFS_CTL1_EDCRS_40),
				IFS_CTL1_EDCRS);
		} else {
			/* 40 mhz, use 20U 20L and 40 ED */
			wlc_bmac_ifsctl1_regshm(wlc_hw,
				(IFS_CTL1_EDCRS | IFS_CTL1_EDCRS_20L | IFS_CTL1_EDCRS_40),
				(IFS_CTL1_EDCRS | IFS_CTL1_EDCRS_20L | IFS_CTL1_EDCRS_40));
		}
	} else if (WLCISACPHY(wlc_hw->band)) {
		wlc_bmac_ifsctl_vht_set(wlc_hw, ON);
	}

#ifdef WLC_LOW_ONLY
	/* XXX 4323/43231USB dongle boards are very sensitive to noise
	 *  need to increase the detection threshold to avoid false detection
	 *  and block other path to change it before the edcrs is disabled
	 */
	if (CHIPID(wlc_hw->sih->chip) == BCM4322_CHIP_ID ||
		CHIPID(wlc_hw->sih->chip) == BCM43231_CHIP_ID) {

		wlc_phy_edcrs_lock(wlc_hw->band->pi, !isht);
	}
#endif // endif
}

#ifdef WL11N
/**
 * update the txphyctl in shm table for BA/ACK/CTS
 * XXX: This does not update the single stream mimo rates as yet. OK for now
 *      since ucode never sends any response at those rates
 */
static void
wlc_upd_ofdm_pctl1_table(wlc_hw_info_t *wlc_hw)
{
	uint8 rate;
	const uint8 rates[8] = {
		WLC_RATE_6M, WLC_RATE_9M, WLC_RATE_12M, WLC_RATE_18M,
		WLC_RATE_24M, WLC_RATE_36M, WLC_RATE_48M, WLC_RATE_54M
	};

	uint16 rate_phyctl1[8] = {0x0002, 0x0202, 0x0802, 0x0a02, 0x1002, 0x1202, 0x1902, 0x1a02};

	uint16 entry_ptr;
	uint16 pctl1, phyctl;
	uint i;

	if (!WLC_PHY_11N_CAP(wlc_hw->band))
		return;

	/* walk the phy rate table and update the entries */
	for (i = 0; i < ARRAYSIZE(rates); i++) {
		rate = rates[i];

		entry_ptr = wlc_bmac_ofdm_ratetable_offset(wlc_hw, rate);

		/* read the SHM Rate Table entry OFDM PCTL1 values */
		pctl1 = wlc_bmac_read_shm(wlc_hw, entry_ptr + M_RT_OFDM_PCTL1_POS);

		/* modify the MODE & code_rate value */
		if (D11REV_IS(wlc_hw->corerev, 31) && WLCISNPHY(wlc_hw->band)) {
			/* corerev31 uses corerev29 ucode, where PHY_CTL_1 inits is for HTPHY
			 * fix it to OFDM rate
			 */
			pctl1 &= (PHY_TXC1_MODE_MASK | PHY_TXC1_BW_MASK);
			pctl1 |= (rate_phyctl1[i] & 0xFFC0);
		}

		if (D11REV_IS(wlc_hw->corerev, 29) &&
			WLCISHTPHY(wlc_hw->band) &&
			AMPDU_HW_ENAB(wlc_hw->wlc->pub)) {
			pctl1 &= ~PHY_TXC1_BW_MASK;
			if (CHSPEC_WLC_BW(wlc_hw->chanspec) == WLC_40_MHZ)
				pctl1 |= PHY_TXC1_BW_40MHZ_DUP;
			else
				pctl1 |= PHY_TXC1_BW_20MHZ;
		}

		/* modify the STF value */
		if ((WLCISNPHY(wlc_hw->band)) || (WLCISLCNPHY(wlc_hw->band))) {
			pctl1 &= ~PHY_TXC1_MODE_MASK;
			if (wlc_bmac_btc_mode_get(wlc_hw))
				pctl1 |= (PHY_TXC1_MODE_SISO << PHY_TXC1_MODE_SHIFT);
			else
				pctl1 |= (wlc_hw->hw_stf_ss_opmode << PHY_TXC1_MODE_SHIFT);
		}

		/* Update the SHM Rate Table entry OFDM PCTL1 values */
		wlc_bmac_write_shm(wlc_hw, entry_ptr + M_RT_OFDM_PCTL1_POS, pctl1);
	}
	/* only works for nphy */
	if (wlc_bmac_btc_mode_get(wlc_hw))
	{
		uint16 ant_ctl = ((wlc_hw->boardflags2 & BFL2_BT_SHARE_ANT0) == BFL2_BT_SHARE_ANT0)
			? PHY_TXC_ANT_1 : PHY_TXC_ANT_0;
		/* set the Response (ACK/CTS) frame phy control word */
		phyctl = wlc_bmac_read_shm(wlc_hw, M_RSP_PCTLWD);
		phyctl = (phyctl & ~PHY_TXC_ANT_MASK) | ant_ctl;
		wlc_bmac_write_shm(wlc_hw, M_RSP_PCTLWD, phyctl);
	}
}

static uint16
wlc_bmac_ofdm_ratetable_offset(wlc_hw_info_t *wlc_hw, uint8 rate)
{
	uint i;
	uint8 plcp_rate = 0;
	struct plcp_signal_rate_lookup {
		uint8 rate;
		uint8 signal_rate;
	};
	/* OFDM RATE sub-field of PLCP SIGNAL field, per 802.11 sec 17.3.4.1 */
	const struct plcp_signal_rate_lookup rate_lookup[] = {
		{WLC_RATE_6M,  0xB},
		{WLC_RATE_9M,  0xF},
		{WLC_RATE_12M, 0xA},
		{WLC_RATE_18M, 0xE},
		{WLC_RATE_24M, 0x9},
		{WLC_RATE_36M, 0xD},
		{WLC_RATE_48M, 0x8},
		{WLC_RATE_54M, 0xC}
	};

	for (i = 0; i < ARRAYSIZE(rate_lookup); i++) {
		if (rate == rate_lookup[i].rate) {
			plcp_rate = rate_lookup[i].signal_rate;
			break;
		}
	}

	/* Find the SHM pointer to the rate table entry by looking in the
	 * Direct-map Table
	 */
	return (2*wlc_bmac_read_shm(wlc_hw, M_RT_DIRMAP_A + (plcp_rate * 2)));
}

void
wlc_bmac_band_stf_ss_set(wlc_hw_info_t *wlc_hw, uint8 stf_mode)
{
	wlc_hw->hw_stf_ss_opmode = stf_mode;

	if (wlc_hw->clk)
		wlc_upd_ofdm_pctl1_table(wlc_hw);
}

void
wlc_bmac_txbw_update(wlc_hw_info_t *wlc_hw)
{
	if (wlc_hw->clk)
		wlc_upd_ofdm_pctl1_table(wlc_hw);

}
#endif /* WL11N */

void BCMFASTPATH
wlc_bmac_read_tsf(wlc_hw_info_t* wlc_hw, uint32* tsf_l_ptr, uint32* tsf_h_ptr)
{
	d11regs_t *regs = wlc_hw->regs;
	uint32 tsf_l;

	/* read the tsf timer low, then high to get an atomic read */
	tsf_l = R_REG(wlc_hw->osh, &regs->tsf_timerlow);

	if (tsf_l_ptr)
		*tsf_l_ptr = tsf_l;

	if (tsf_h_ptr)
		*tsf_h_ptr = R_REG(wlc_hw->osh, &regs->tsf_timerhigh);

	return;
}

bool
#ifdef WLDIAG
wlc_bmac_validate_chip_access(wlc_hw_info_t *wlc_hw)
#else
BCMATTACHFN(wlc_bmac_validate_chip_access)(wlc_hw_info_t *wlc_hw)
#endif // endif
{
	d11regs_t *regs = wlc_hw->regs;
	uint32 w, valw, valr;
	volatile uint16 *reg16;
	osl_t *osh = wlc_hw->osh;

	WL_TRACE(("wl%d: %s\n", wlc_hw->unit, __FUNCTION__));

	/* Validate dchip register access */
	wlc_bmac_copyfrom_shm(wlc_hw, 0, &w, sizeof(w));

	/* Can we write and read back a 32bit register? */
	valw = 0xaa5555aa;
	wlc_bmac_copyto_shm(wlc_hw, 0, &valw, sizeof(valw));

	wlc_bmac_copyfrom_shm(wlc_hw, 0, &valr, sizeof(valr));
	if (valr != valw) {
		WL_ERROR(("wl%d: %s: SHM = 0x%x, expected 0x%x\n",
			wlc_hw->unit, __FUNCTION__, valr, valw));
		return (FALSE);
	}

	valw = 0x55aaaa55;
	wlc_bmac_copyto_shm(wlc_hw, 0, &valw, sizeof(valw));

	wlc_bmac_copyfrom_shm(wlc_hw, 0, &valr, sizeof(valr));
	if (valr != valw) {
		WL_ERROR(("wl%d: %s: SHM = 0x%x, expected 0x%x\n",
			wlc_hw->unit, __FUNCTION__, valr, valw));
		return (FALSE);
	}

	wlc_bmac_copyto_shm(wlc_hw, 0, &w, sizeof(w));

	if (D11REV_LT(wlc_hw->corerev, 11)) {
		/* if 32 bit writes are split into 16 bit writes, are they in the correct order
		 * for our interface, low to high
		 */
		reg16 = (volatile uint16*)(uintptr)&regs->tsf_cfpstart;

		/* write the CFPStart register low half explicitly, starting a buffered write */
		W_REG(osh, reg16, 0xAAAA);

		/* Write a 32 bit value to CFPStart to test the 16 bit split order.
		 * If the low 16 bits are written first, followed by the high 16 bits then the
		 * 32 bit value 0xCCCCBBBB should end up in the register.
		 * If the order is reversed, then the write to the high half will trigger a buffered
		 * write of 0xCCCCAAAA.
		 * If the bus is 32 bits, then this is not much of a test, and the reg should
		 * have the correct value 0xCCCCBBBB.
		 */
		W_REG(osh, &regs->tsf_cfpstart, 0xCCCCBBBB);

		/* verify with the 16 bit registers that have no side effects */
		w = R_REG(osh, &regs->u.d11regs.tsf_cfpstrt_l);
		if (w != (uint)0xBBBB) {
			WL_ERROR(("wl%d: %s: tsf_cfpstrt_l = 0x%x, expected"
				" 0x%x\n",
				wlc_hw->unit, __FUNCTION__, w, 0xBBBB));
			return (FALSE);
		}
		w = R_REG(osh, &regs->u.d11regs.tsf_cfpstrt_h);
		if (w != (uint)0xCCCC) {
			WL_ERROR(("wl%d: %s: tsf_cfpstrt_h = 0x%x, expected"
				" 0x%x\n",
				wlc_hw->unit, __FUNCTION__, w, 0xCCCC));
			return (FALSE);
		}

	}

	/* clear CFPStart */
	W_REG(osh, &regs->tsf_cfpstart, 0);

#ifndef BCM_OL_DEV
	w = R_REG(osh, &regs->maccontrol);
	if ((w != (MCTL_IHR_EN | MCTL_WAKE)) &&
	    (w != (MCTL_IHR_EN | MCTL_GMODE | MCTL_WAKE))) {
		WL_ERROR(("wl%d: %s: maccontrol = 0x%x, expected 0x%x or 0x%x\n",
		          wlc_hw->unit, __FUNCTION__, w, (MCTL_IHR_EN | MCTL_WAKE),
		          (MCTL_IHR_EN | MCTL_GMODE | MCTL_WAKE)));
		return (FALSE);
	}
#endif /* BCM_OL_DEV */

	return (TRUE);
}

#define PHYPLL_WAIT_US	100000

void
wlc_bmac_core_phypll_ctl(wlc_hw_info_t* wlc_hw, bool on)
{
	d11regs_t *regs;
	osl_t *osh;
	uint32 req_bits, avail_bits, tmp;

	WL_TRACE(("wl%d: wlc_bmac_core_phypll_ctl\n", wlc_hw->unit));

	if (D11REV_LE(wlc_hw->corerev, 16) ||
	    D11REV_IS(wlc_hw->corerev, 20) ||
	    D11REV_IS(wlc_hw->corerev, 27))
		return;

	regs = wlc_hw->regs;
	osh = wlc_hw->osh;

	/* Do not access registers if core is not up */
	if (wlc_bmac_si_iscoreup(wlc_hw) == FALSE)
		return;

	if (on) {
		if (D11REV_GE(wlc_hw->corerev, 24) &&
			!(D11REV_IS(wlc_hw->corerev, 29) || D11REV_GE(wlc_hw->corerev, 40))) {
			req_bits = PSM_CORE_CTL_PPAR;
			avail_bits = PSM_CORE_CTL_PPAS;

			if (CHIPID(wlc_hw->sih->chip) == BCM4313_CHIP_ID) {
				req_bits = PSM_CORE_CTL_PPAR | PSM_CORE_CTL_HAR;
				avail_bits = PSM_CORE_CTL_HAS;
			}

			OR_REG(osh, &regs->psm_corectlsts, req_bits);
			SPINWAIT((R_REG(osh, &regs->psm_corectlsts) & avail_bits) != avail_bits,
				PHYPLL_WAIT_US);

			tmp = R_REG(osh, &regs->psm_corectlsts);
		} else {
			req_bits = CCS_ERSRC_REQ_D11PLL | CCS_ERSRC_REQ_PHYPLL;
			avail_bits = CCS_ERSRC_AVAIL_D11PLL | CCS_ERSRC_AVAIL_PHYPLL;

			if (CHIPID(wlc_hw->sih->chip) == BCM4313_CHIP_ID) {
				req_bits = CCS_ERSRC_REQ_D11PLL | CCS_ERSRC_REQ_PHYPLL |
					CCS_ERSRC_REQ_HT;
				avail_bits = CCS_ERSRC_AVAIL_HT;
			}

			OR_REG(osh, &regs->clk_ctl_st, req_bits);
			SPINWAIT((R_REG(osh, &regs->clk_ctl_st) & avail_bits) != avail_bits,
				PHYPLL_WAIT_US);

			tmp = R_REG(osh, &regs->clk_ctl_st);
		}

		if ((tmp & avail_bits) != avail_bits) {
			WL_ERROR(("%s: turn on PHY PLL failed\n", __FUNCTION__));
			WL_HEALTH_LOG(wlc_hw->wlc, PHY_PLL_ERROR);
			ASSERT(0);
		}
	} else {
		/* Since the PLL may be shared, other cores can still be requesting it;
		 * so we'll deassert the request but not wait for status to comply.
		 */
		if (D11REV_GE(wlc_hw->corerev, 24) &&
		!(D11REV_IS(wlc_hw->corerev, 29) || D11REV_GE(wlc_hw->corerev, 40))) {
			req_bits = PSM_CORE_CTL_PPAR;

			if (CHIPID(wlc_hw->sih->chip) == BCM4313_CHIP_ID)
				req_bits = PSM_CORE_CTL_PPAR | PSM_CORE_CTL_HAR;

			AND_REG(osh, &regs->psm_corectlsts, ~req_bits);
			tmp = R_REG(osh, &regs->psm_corectlsts);
		} else {
			req_bits = CCS_ERSRC_REQ_D11PLL | CCS_ERSRC_REQ_PHYPLL;

			if (CHIPID(wlc_hw->sih->chip) == BCM4313_CHIP_ID)
				req_bits = CCS_ERSRC_REQ_D11PLL | CCS_ERSRC_REQ_PHYPLL |
					CCS_ERSRC_REQ_HT;

			AND_REG(osh, &regs->clk_ctl_st, ~req_bits);
			tmp = R_REG(osh, &regs->clk_ctl_st);
		}
	}

	if (wlc_bmac_rsdb_cap(wlc_hw))
		wlc_bmac_4349_core1_hwreqoff(wlc_hw, (on == 0)? TRUE:FALSE);

	WL_TRACE(("%s: clk_ctl_st after phypll(%d) request 0x%x\n",
		__FUNCTION__, on, tmp));
}

void
wlc_coredisable(wlc_hw_info_t* wlc_hw)
{
	bool dev_gone;

	WL_TRACE(("wl%d: %s\n", wlc_hw->unit, __FUNCTION__));

	ASSERT(!wlc_hw->up);

	dev_gone = DEVICEREMOVED(wlc_hw->wlc);

	if (dev_gone)
		return;

	if (wlc_hw->noreset)
		return;

	/* radio off */
	phy_radio_switch((phy_info_t *)wlc_hw->band->pi, OFF);

	/* turn off analog core */
	phy_ana_switch((phy_info_t *)wlc_hw->band->pi, OFF);

	/* turn off PHYPLL to save power */
	wlc_bmac_core_phypll_ctl(wlc_hw, FALSE);

	/* No need to set wlc->pub->radio_active = OFF
	 * because this function needs down capability and
	 * radio_active is designed for BCMNODOWN.
	 */

	/* remove gpio controls */
	if (wlc_hw->ucode_dbgsel)
		si_gpiocontrol(wlc_hw->sih, ~0, 0, GPIO_DRV_PRIORITY);

	wlc_hw->clk = FALSE;
	if (wlc_bmac_islast_core(wlc_hw)) {
		wlc_bmac_core_disable(wlc_hw, 0);
	}
	wlc_phy_hw_clk_state_upd(wlc_hw->band->pi, FALSE);
}

/** power both the pll and external oscillator on/off */
void
wlc_bmac_xtal(wlc_hw_info_t* wlc_hw, bool want)
{
	WL_TRACE(("wl%d: wlc_bmac_xtal: want %d\n", wlc_hw->unit, want));

	/* dont power down if plldown is false or we must poll hw radio disable */
	if (!want && wlc_hw->pllreq)
		return;

	if (wlc_hw->sih)
		si_clkctl_xtal(wlc_hw->sih, XTAL|PLL, want);

	wlc_hw->sbclk = want;
	if (!wlc_hw->sbclk) {
		wlc_hw->clk = FALSE;
		if (wlc_hw->band && wlc_hw->band->pi)
			wlc_phy_hw_clk_state_upd(wlc_hw->band->pi, FALSE);
	}
}

static void
wlc_flushqueues(wlc_hw_info_t *wlc_hw)
{
	wlc_info_t *wlc = wlc_hw->wlc;
	uint i, nfifo = NFIFO;
#ifdef HOST_HDR_FETCH
	struct spktq bus_pkt_list;
#endif // endif
	int pktcnt;

	if (!PIO_ENAB_HW(wlc_hw)) {
		wlc->txpend16165war = 0;

		/* XXX: use CTdma macro here to get nfifo; too early
		 * to use wlc_hw->nfifo_inuse here.
		 */
		if (BCM_DMA_CT_ENAB(wlc))
			nfifo = NFIFO_EXT;

		/* free any posted tx packets */
		for (i = 0; i < nfifo; i++) {
#ifdef HOST_HDR_FETCH
			if (HOST_HDR_FETCH_ENAB()) {
				/* Reclaim pkts from pciedev queue */
				pktqinit(&bus_pkt_list, -1);
				wl_reclaim_bus_txpkts(wlc->wl, &bus_pkt_list, i, FALSE);

				/* flush the pkt list from bus layer */
				pktcnt = pktqplen(&bus_pkt_list);
				if (pktcnt > 0) {
					pktqflush(wlc->osh, &bus_pkt_list);

					/* Do the accounting */
					WLC_TXFIFO_COMPLETE(wlc, i, pktcnt, 0);
					WL_ERROR(("wlc_flushqueues: reclaim_bus fifo %d pkts %d\n",
						i, pktcnt));
				}

				TXDMAPEND_CLR(wlc, i);
				AQMDMA_PENDCLR(wlc, i);
				/* With HOST_HDR_FETCH , a set of packets could be
				 * stuck inside the fetch request queues.
				 * Walk the list to cancel callback functions for the same.
				 * Can not release the packets since PCIE M2M DMA
				 * could be underway for them
				 */
				wlc_txs_fetch_rqst_queue_flush(wlc, (1 << i));

			}
#endif /* HOST_HDR_FETCH */
			if (wlc_hw->di[i]) {
#ifdef BCM_DMA_CT
				if (wlc_hw->aqm_di[i]) {
					dma_txreclaim(wlc_hw->aqm_di[i], HNDDMA_RANGE_ALL);
				}
#endif // endif
				pktcnt = dma_txreclaim(wlc_hw->di[i], HNDDMA_RANGE_ALL);
#ifdef WLC_HIGH
				if (pktcnt > 0) {
					WLC_TXFIFO_COMPLETE(wlc, i, pktcnt, 0);
					WL_ERROR(("wlc_flushqueues: reclaim fifo %d pkts %d\n",
						i, pktcnt));
				}
#endif /* WLC_HIGH */

				pktcnt = TXPKTPENDGET(wlc, i);
				if (pktcnt > 0) {
					WL_ERROR(("wlc_flushqueues: fifo %d REMAINS %d pkts\n",
						i, pktcnt));
				}
#if defined(DMA_TX_FREE)
				WL_TRACE(("wlc_flushqueues: ampdu_flags cleared, head %d tail %d\n",
				          wlc_hw->txstatus_ampdu_flags[i].head,
				          wlc_hw->txstatus_ampdu_flags[i].tail));
				wlc_hw->txstatus_ampdu_flags[i].head = 0;
				wlc_hw->txstatus_ampdu_flags[i].tail = 0;
#endif // endif
			}
		}

		/* Free the packets which is early reclaimed */
#ifdef	WL_RXEARLYRC
		while (wlc_hw->rc_pkt_head) {
			void *p = wlc_hw->rc_pkt_head;
			wlc_hw->rc_pkt_head = PKTLINK(p);
			PKTSETLINK(p, NULL);
			PKTFREE(wlc_hw->osh, p, FALSE);
		}
#endif // endif

#ifdef AP
		wlc_tx_fifo_sync_bcmc_reset(wlc);
#endif // endif

		/* free any posted rx packets */
		for (i = 0; i < MAX_RX_FIFO; i++) {
			if ((wlc_hw->di[i] != NULL) && wlc_bmac_rxfifo_enab(i)) {
				dma_rxreclaim(wlc_hw->di[i]);
			}
		}

		if (D11REV_IS(wlc_hw->corerev, 4))
			dma_rxreclaim(wlc_hw->di[RX_TXSTATUS_FIFO]);
	} else {
		for (i = 0; i < nfifo; i++) {
			if (wlc_hw->pio[i]) {
				/* include reset the counter */
				wlc_pio_txreclaim(wlc_hw->pio[i]);
			}
		}
		/* For PIO, no rx sw queue to reclaim */
	}
}

#ifdef STA
#if defined(WLRM)
/** start a CCA measurement for the given number of microseconds */
void
wlc_bmac_rm_cca_measure(wlc_hw_info_t *wlc_hw, uint32 us)
{
	uint32 gpt_ticks;

	/* convert dur in TUs to 1/8 us units for GPT */
	gpt_ticks = us << 3;

	/* config GPT 2 to decrement by TSF ticks */
	wlc_bmac_write_ihr(wlc_hw, TSF_GPT_2_STAT, TSF_GPT_USETSF);
	/* set GPT 2 to the measurement duration */
	wlc_bmac_write_ihr(wlc_hw, TSF_GPT_2_CTR_L, (gpt_ticks & 0xffff));
	wlc_bmac_write_ihr(wlc_hw, TSF_GPT_2_CTR_H, (gpt_ticks >> 16));
	/* tell ucode to start the CCA measurement */
	OR_REG(wlc_hw->osh, &wlc_hw->regs->maccommand, MCMD_CCA);

	return;
}

void
wlc_bmac_rm_cca_int(wlc_hw_info_t *wlc_hw)
{
	uint32 cca_idle;
	uint32 cca_idle_us;
	uint32 gpt2_h, gpt2_l;

	gpt2_l = wlc_bmac_read_ihr(wlc_hw, TSF_GPT_2_VAL_L);
	gpt2_h = wlc_bmac_read_ihr(wlc_hw, TSF_GPT_2_VAL_H);
	cca_idle = (gpt2_h << 16) | gpt2_l;

	/* convert GTP 1/8 us units to us */
	cca_idle_us = (cca_idle >> 3);

	wlc_rm_cca_complete(wlc_hw->wlc, cca_idle_us);
}
#endif /* WLRM */
#endif /* STA */

/** set the PIO mode bit in the control register for the rxfifo */
void
wlc_rxfifo_setpio(wlc_hw_info_t *wlc_hw)
{
	if (D11REV_LT(wlc_hw->corerev, 11)) {
		fifo32_t *fiforegs;

		fiforegs = &wlc_hw->regs->fifo.f32regs;
		W_REG(wlc_hw->osh, &fiforegs->dmaregs[RX_FIFO].rcv.control, RC_FM);
		if (D11REV_IS(wlc_hw->corerev, 4))
			W_REG(wlc_hw->osh,
				&fiforegs->dmaregs[RX_TXSTATUS_FIFO].rcv.control, RC_FM);
	} else {
		fifo64_t *fiforegs;

		fiforegs = &wlc_hw->regs->fifo.f64regs[RX_FIFO];
		W_REG(wlc_hw->osh, &fiforegs->dmarcv.control, D64_RC_FM);
	}
}

/* Set the range of objmem memory that is organized as 32bit words to a value.
 * 'offset' needs to be multiple of 4 address and
 * Buffer length 'len' must be an multiple of 4 bytes
 * 'sel' selects the type of memory
 */
void
wlc_bmac_set_objmem32(wlc_hw_info_t *wlc_hw, uint offset, uint32 val, int len, uint32 sel)
{
	d11regs_t *regs = wlc_hw->regs;
	int i;

	ASSERT(wlc_hw->clk);
#ifndef EFI
	PANIC_CHECK_CLK(wlc_hw->clk, "wl%d: %s: NO CLK\n", wlc_hw->unit, __FUNCTION__);
#endif // endif

	ASSERT((offset & 3) == 0);
	ASSERT((len & 3) == 0);

	ASSERT(regs != NULL);

#if defined(BCM_OL_DEV) || defined(WLOFFLD)
	tcm_sem_enter(wlc_hw->wlc);
#endif // endif

	W_REG(wlc_hw->osh, &regs->objaddr, sel | OBJADDR_AUTO_INC | (offset >> 2));
	(void)R_REG(wlc_hw->osh, &regs->objaddr);

	for (i = 0; i < len; i += 4) {
		W_REG(wlc_hw->osh, &regs->objdata, val);
	}

#if defined(BCM_OL_DEV) || defined(WLOFFLD)
	tcm_sem_exit(wlc_hw->wlc);
#endif // endif
}

/* Copy a buffer to an objmem memory that is organized as 32bit words.
 * 'offset' needs to be multiple of 4 address and
 * Buffer length 'len' must be an multiple of 4 bytes
 * 'sel' selects the type of memory
 */
void
wlc_bmac_copyto_objmem32(wlc_hw_info_t *wlc_hw, uint offset, const uint8 *buf, int len, uint32 sel)
{
	d11regs_t *regs = wlc_hw->regs;
	const uint8* p = buf;
	int i;
	uint32 val;

	ASSERT(wlc_hw->clk);
#ifndef EFI
	PANIC_CHECK_CLK(wlc_hw->clk, "wl%d: %s: NO CLK\n", wlc_hw->unit, __FUNCTION__);
#endif // endif

	ASSERT((offset & 3) == 0);
	ASSERT((len & 3) == 0);

	ASSERT(regs != NULL);

#if defined(BCM_OL_DEV) || defined(WLOFFLD)
	tcm_sem_enter(wlc_hw->wlc);
#endif // endif

	W_REG(wlc_hw->osh, &regs->objaddr, sel | OBJADDR_AUTO_INC | (offset >> 2));
	(void)R_REG(wlc_hw->osh, &regs->objaddr);

	for (i = 0; i < len; i += 4) {
		val = p[i] | (p[i+1] << 8) | (p[i+2] << 16) | (p[i+3] << 24);
		val = htol32(val);
		W_REG(wlc_hw->osh, &regs->objdata, val);
	}

#if defined(BCM_OL_DEV) || defined(WLOFFLD)
	tcm_sem_exit(wlc_hw->wlc);
#endif // endif
}

/* Copy objmem memory that is organized as 32bit words to a buffer.
 * 'offset' needs to be multiple of 4 address and
 * Buffer length 'len' must be an multiple of 4 bytes
 * 'sel' selects the type of memory
 */
void
wlc_bmac_copyfrom_objmem32(wlc_hw_info_t *wlc_hw, uint offset, uint8 *buf, int len, uint32 sel)
{
	d11regs_t *regs = wlc_hw->regs;
	uint8* p = buf;
	int i, len32 = (len/4)*4;
	uint32 val;

	ASSERT(wlc_hw->clk);
#ifndef EFI
	PANIC_CHECK_CLK(wlc_hw->clk, "wl%d: %s: NO CLK\n", wlc_hw->unit, __FUNCTION__);
#endif // endif

	ASSERT((offset & 3) == 0);
	ASSERT((len & 3) == 0);

#if defined(BCM_OL_DEV) || defined(WLOFFLD)
	tcm_sem_enter(wlc_hw->wlc);
#endif // endif

	W_REG(wlc_hw->osh, &regs->objaddr, sel | OBJADDR_AUTO_INC | (offset >> 2));
	(void)R_REG(wlc_hw->osh, &regs->objaddr);
	for (i = 0; i < len32; i += 4) {
		val = R_REG(wlc_hw->osh, &regs->objdata);
		val = ltoh32(val);
		p[i] = val & 0xFF;
		p[i+1] = (val >> 8) & 0xFF;
		p[i+2] = (val >> 16) & 0xFF;
		p[i+3] = (val >> 24) & 0xFF;
	}

#if defined(BCM_OL_DEV) || defined(WLOFFLD)
	tcm_sem_exit(wlc_hw->wlc);
#endif // endif
}

uint16
wlc_bmac_read_shm(wlc_hw_info_t *wlc_hw, uint offset)
{
	return  wlc_bmac_read_objmem16(wlc_hw, offset, OBJADDR_SHM_SEL);
}

void
wlc_bmac_write_shm(wlc_hw_info_t *wlc_hw, uint offset, uint16 v)
{
	wlc_bmac_write_objmem16(wlc_hw, offset, v, OBJADDR_SHM_SEL);
}

void
wlc_bmac_update_shm(wlc_hw_info_t *wlc_hw, uint offset, uint16 v, uint16 mask)
{
	wlc_bmac_update_objmem16(wlc_hw, offset, v, mask, OBJADDR_SHM_SEL);
}

/**
 * Set a range of shared memory to a value.
 * SHM 'offset' needs to be an even address and
 * Buffer length 'len' must be an even number of bytes
 */
void
wlc_bmac_set_shm(wlc_hw_info_t *wlc_hw, uint offset, uint16 v, int len)
{
	int i;

	/* offset and len need to be even */
	ASSERT((offset & 1) == 0);
	ASSERT((len & 1) == 0);

	if (len <= 0)
		return;

	for (i = 0; i < len; i += 2) {
		wlc_bmac_write_objmem16(wlc_hw, offset + i, v, OBJADDR_SHM_SEL);
	}
}

#if defined(WL_PSMX)
uint16
wlc_bmac_read_shmx(wlc_hw_info_t *wlc_hw, uint offset)
{
	return  wlc_bmac_read_objmem16(wlc_hw, offset, OBJADDR_SHMX_SEL);
}

void
wlc_bmac_write_shmx(wlc_hw_info_t *wlc_hw, uint offset, uint16 v)
{
	wlc_bmac_write_objmem16(wlc_hw, offset, v, OBJADDR_SHMX_SEL);
}
#endif /* WL_PSMX */

#if defined(WL_PSMX) && defined(NOT_YET)
static uint16
wlc_bmac_read_macregx(wlc_hw_info_t *wlc_hw, uint offset)
{
	ASSERT(offset >= D11REG_IHR_BASE);
	offset = (offset - D11REG_IHR_BASE) << 1;
	return wlc_bmac_read_objmem16(wlc_hw, offset, OBJADDR_IHRX_SEL);
}

static void
wlc_bmac_write_macregx(wlc_hw_info_t *wlc_hw, uint offset, uint16 v)
{
	ASSERT(offset >= D11REG_IHR_BASE);
	offset = (offset - D11REG_IHR_BASE) << 1;
	wlc_bmac_write_objmem16(wlc_hw, offset, v, OBJADDR_IHRX_SEL);
}
#endif /* WL_PSMX */

static uint16
wlc_bmac_read_objmem16(wlc_hw_info_t *wlc_hw, uint offset, uint32 sel)
{
	d11regs_t *regs = wlc_hw->regs;
	volatile uint16* objdata_lo = (volatile uint16*)(uintptr)&regs->objdata;
	volatile uint16* objdata_hi = objdata_lo + 1;
	uint16 v;

	ASSERT(wlc_hw->clk);
#ifndef EFI
	PANIC_CHECK_CLK(wlc_hw->clk, "wl%d: %s: NO CLK\n", wlc_hw->unit, __FUNCTION__);
#endif // endif

	ASSERT((offset & 1) == 0);

#if defined(BCM_OL_DEV) || defined(WLOFFLD)
	tcm_sem_enter(wlc_hw->wlc);
#endif // endif

	W_REG(wlc_hw->osh, &regs->objaddr, sel | (offset >> 2));
	(void)R_REG(wlc_hw->osh, &regs->objaddr);
	if (offset & 2) {
		v = R_REG(wlc_hw->osh, objdata_hi);
	} else {
		v = R_REG(wlc_hw->osh, objdata_lo);
	}

#if defined(BCM_OL_DEV) || defined(WLOFFLD)
	tcm_sem_exit(wlc_hw->wlc);
#endif // endif
	return v;
}

static uint32
wlc_bmac_read_objmem32(wlc_hw_info_t *wlc_hw, uint offset, uint32 sel)
{
	d11regs_t *regs = wlc_hw->regs;
	uint32 v;

	ASSERT(wlc_hw->clk);
#ifndef EFI
	PANIC_CHECK_CLK(wlc_hw->clk, "wl%d: %s: NO CLK\n", wlc_hw->unit, __FUNCTION__);
#endif // endif

	ASSERT((offset & 3) == 0);

#if defined(BCM_OL_DEV) || defined(WLOFFLD)
	tcm_sem_enter(wlc_hw->wlc);
#endif // endif

	W_REG(wlc_hw->osh, &regs->objaddr, sel | (offset >> 2));
	(void)R_REG(wlc_hw->osh, &regs->objaddr);
	v = R_REG(wlc_hw->osh, &regs->objdata);

#if defined(BCM_OL_DEV) || defined(WLOFFLD)
	tcm_sem_exit(wlc_hw->wlc);
#endif // endif
	return v;
}

static void
wlc_bmac_write_objmem16(wlc_hw_info_t *wlc_hw, uint offset, uint16 v, uint32 sel)
{
	d11regs_t *regs = wlc_hw->regs;
	volatile uint16* objdata_lo = (volatile uint16*)(uintptr)&regs->objdata;
	volatile uint16* objdata_hi = objdata_lo + 1;

	ASSERT(wlc_hw->clk);
#ifndef EFI
	PANIC_CHECK_CLK(wlc_hw->clk, "wl%d: %s: NO CLK\n", wlc_hw->unit, __FUNCTION__);
#endif // endif

	ASSERT(regs != NULL);

	ASSERT((offset & 1) == 0);

#if defined(BCM_OL_DEV) || defined(WLOFFLD)
	tcm_sem_enter(wlc_hw->wlc);
#endif // endif

	W_REG(wlc_hw->osh, &regs->objaddr, sel | (offset >> 2));
	(void)R_REG(wlc_hw->osh, &regs->objaddr);
	if (offset & 2) {
		W_REG(wlc_hw->osh, objdata_hi, v);
	} else {
		W_REG(wlc_hw->osh, objdata_lo, v);
	}

#if defined(BCM_OL_DEV) || defined(WLOFFLD)
	tcm_sem_exit(wlc_hw->wlc);
#endif // endif
}

static void
wlc_bmac_write_objmem32(wlc_hw_info_t *wlc_hw, uint offset, uint32 v, uint32 sel)
{
	d11regs_t *regs = wlc_hw->regs;

	ASSERT(wlc_hw->clk);
#ifndef EFI
	PANIC_CHECK_CLK(wlc_hw->clk, "wl%d: %s: NO CLK\n", wlc_hw->unit, __FUNCTION__);
#endif // endif

	ASSERT(regs != NULL);

	ASSERT((offset & 3) == 0);

#if defined(BCM_OL_DEV) || defined(WLOFFLD)
	tcm_sem_enter(wlc_hw->wlc);
#endif // endif

	W_REG(wlc_hw->osh, &regs->objaddr, sel | (offset >> 2));
	(void)R_REG(wlc_hw->osh, &regs->objaddr);

	W_REG(wlc_hw->osh, &regs->objdata, v);

#if defined(BCM_OL_DEV) || defined(WLOFFLD)
	tcm_sem_exit(wlc_hw->wlc);
#endif // endif
}

static void
wlc_bmac_update_objmem16(wlc_hw_info_t *wlc_hw, uint offset, uint16 v, uint16 mask, uint32 objsel)
{
	uint16 objval;

	ASSERT((v & ~mask) == 0);

	objval = wlc_bmac_read_objmem16(wlc_hw, offset, objsel);
	objval = (objval & ~mask) | v;
	wlc_bmac_write_objmem16(wlc_hw, offset, objval, objsel);
}

/**
 * Copy a buffer to shared memory of specified type .
 * SHM 'offset' needs to be an even address and
 * Buffer length 'len' must be an even number of bytes
 * 'sel' selects the type of memory
 */
void
wlc_bmac_copyto_objmem(wlc_hw_info_t *wlc_hw, uint offset, const void* buf, int len, uint32 sel)
{
	const uint8* p = (const uint8*)buf;
	int i;
	uint16 v16;
	uint32 v32;

	/* offset and len need to be even */
	ASSERT((offset & 1) == 0);
	ASSERT((len & 1) == 0);

	if (len <= 0)
		return;

	/* Some of the OBJADDR memories can be accessed as 4 byte
	 * and some as 2 byte
	 */
	if (OBJADDR_2BYTES_ACCESS(sel)) {
		for (i = 0; i < len; i += 2) {
			v16 = htol16(p[i] | (p[i+1] << 8));
			wlc_bmac_write_objmem16(wlc_hw, offset + i, v16, sel);
		}
	} else {
		int len16 = (len%4);
		int len32 = (len/4)*4;

		/* offset needs to be multiple of 4 here */
		ASSERT((offset & 3) == 0);

		/* Write all the 32bit words */
		for (i = 0; i < len32; i += 4) {
			v32 = htol32(p[i] | (p[i+1] << 8) | (p[i+2] << 16) | (p[i+3] << 24));
			wlc_bmac_write_objmem32(wlc_hw, offset + i, v32, sel);
		}

		/* Write the last 16bit if any */
		if (len16) {
			v16 = htol16(p[i] | (p[i+1] << 8));
			wlc_bmac_write_objmem16(wlc_hw, offset + i, v16, sel);
		}
	}
}

/**
 * Copy a piece of shared memory of specified type to a buffer .
 * SHM 'offset' needs to be an even address and
 * Buffer length 'len' must be an even number of bytes
 * 'sel' selects the type of memory
 */
void
wlc_bmac_copyfrom_objmem(wlc_hw_info_t *wlc_hw, uint offset, void* buf, int len, uint32 sel)
{
	uint8* p = (uint8*)buf;
	int i;
	uint16 v16;
	uint32 v32;

	/* offset and len need to be even */
	ASSERT((offset & 1) == 0);
	ASSERT((len & 1) == 0);

	if (len <= 0)
		return;

	/* Some of the OBJADDR memories can be accessed as 4 byte
	 * and some as 2 byte
	 */
	if (OBJADDR_2BYTES_ACCESS(sel)) {
		for (i = 0; i < len; i += 2) {
			v16 = ltoh16(wlc_bmac_read_objmem16(wlc_hw, offset + i, sel));
			p[i] = v16 & 0xFF;
			p[i+1] = (v16 >> 8) & 0xFF;
		}
	} else {
		int len16 = (len%4);
		int len32 = (len/4)*4;

		/* offset needs to be multiple of 4 here */
		ASSERT((offset & 3) == 0);

		/* Read all the 32bit words */
		for (i = 0; i < len32; i += 4) {
			v32 = ltoh32(wlc_bmac_read_objmem32(wlc_hw, offset + i, sel));
			p[i] = v32 & 0xFF;
			p[i+1] = (v32 >> 8) & 0xFF;
			p[i+2] = (v32 >> 16) & 0xFF;
			p[i+3] = (v32 >> 24) & 0xFF;
		}

		/* Read the last 16bit if any */
		if (len16) {
			v16 = ltoh16(wlc_bmac_read_objmem16(wlc_hw, offset + i, sel));
			p[i] = v16 & 0xFF;
			p[i+1] = (v16 >> 8) & 0xFF;
		}
	}
}

void
wlc_bmac_copyfrom_vars(wlc_hw_info_t *wlc_hw, char ** buf, uint *len)
{
	/* XXX these vars can be very large, need to make sure
	 *  it fits into RPC data buffer. And with headers from different lower layers,
	 * the total len must fit into the bus MPS/MTU. At host side, the DBUS rx buffer may
	 * be limited to DBUS_RX_BUFFER_SIZE_RPC
	 *
	 * Can not truncate this buffer even the tail vars may not be needed in high driver
	 * because windows driver verifier will complain.
	 *
	 * Breaking into multiple pieces if that's a problem.
	 */
	WL_TRACE(("wlc_bmac_copyfrom_vars, nvram vars totlen=%d\n", wlc_hw->vars_size));

	if (wlc_hw->vars) {
		*buf = wlc_hw->vars;
		*len = wlc_hw->vars_size;
	}
#ifdef WLC_LOW_ONLY
	else {
		/* no per device vars, return the global one */
		nvram_get_global_vars(buf, len);
	}
#endif // endif
}

void
wlc_bmac_retrylimit_upd(wlc_hw_info_t *wlc_hw, uint16 SRL, uint16 LRL)
{
	wlc_hw->SRL = SRL;
	wlc_hw->LRL = LRL;

	/* write retry limit to SCR, shouldn't need to suspend */
	if (wlc_hw->up) {
		wlc_bmac_copyto_objmem(wlc_hw, S_DOT11_SRC_LMT << 2, &(wlc_hw->SRL),
			sizeof(wlc_hw->SRL), OBJADDR_SCR_SEL);

		wlc_bmac_copyto_objmem(wlc_hw, S_DOT11_LRC_LMT << 2, &(wlc_hw->LRL),
			sizeof(wlc_hw->LRL), OBJADDR_SCR_SEL);
	}
}

void
wlc_bmac_set_noreset(wlc_hw_info_t *wlc_hw, bool noreset_flag)
{
	wlc_hw->noreset = noreset_flag;
}

bool
wlc_bmac_get_noreset(wlc_hw_info_t *wlc_hw)
{
	return wlc_hw->noreset;
}

bool
wlc_bmac_p2p_cap(wlc_hw_info_t *wlc_hw)
{
#ifdef WLP2P_UCODE
	return wlc_hw->corerev >= 15;
#else
	return FALSE;
#endif // endif
}

int
wlc_bmac_p2p_set(wlc_hw_info_t *wlc_hw, bool enable)
{
	if (wlc_hw->_p2p == enable)
		return BCME_OK;
	if (enable &&
	    !wlc_bmac_p2p_cap(wlc_hw))
		return BCME_ERROR;
#ifdef WLP2P_UCODE
#ifdef WLP2P_UCODE_ONLY
	if (!enable)
		return BCME_ERROR;
#endif // endif
	wlc_hw->ucode_loaded = FALSE;
	wlc_hw->_p2p = enable;
#endif /* WLP2P_UCODE */
	return BCME_OK;
}

void
wlc_bmac_pllreq(wlc_hw_info_t *wlc_hw, bool set, mbool req_bit)
{
	ASSERT(req_bit);

	if (set) {
		if (mboolisset(wlc_hw->pllreq, req_bit))
			return;

		mboolset(wlc_hw->pllreq, req_bit);

		if (mboolisset(wlc_hw->pllreq, WLC_PLLREQ_FLIP)) {
			if (!wlc_hw->sbclk) {
				wlc_bmac_xtal(wlc_hw, ON);
			}
		}
	}
	else {
		if (!mboolisset(wlc_hw->pllreq, req_bit))
			return;

		mboolclr(wlc_hw->pllreq, req_bit);

		if (mboolisset(wlc_hw->pllreq, WLC_PLLREQ_FLIP)) {
			if (wlc_hw->sbclk) {
				wlc_bmac_xtal(wlc_hw, OFF);
			}
		}
	}

	return;
}

void
wlc_bmac_set_clk(wlc_hw_info_t *wlc_hw, bool on)
{
	if (on) {
		/* power up pll and oscillator */
		wlc_bmac_xtal(wlc_hw, ON);

		/* enable core(s), ignore bandlocked
		 * Leave with the same band selected as we entered
		 */
		wlc_bmac_corereset(wlc_hw, WLC_USE_COREFLAGS);
	} else {
		/* if already down, must skip the core disable */
		if (wlc_hw->clk) {
			/* disable core(s), ignore bandlocked */
			wlc_coredisable(wlc_hw);
		}
			/* power down pll and oscillator */
		wlc_bmac_xtal(wlc_hw, OFF);
	}
}

#ifdef BCMASSERT_SUPPORT
bool
wlc_bmac_taclear(wlc_hw_info_t *wlc_hw, bool ta_ok)
{
	return (!wlc_hw->sbclk || !si_taclear(wlc_hw->sih, !ta_ok));
}
#endif // endif

#ifdef WLLED
/** may touch sb register inside */
void
wlc_bmac_led_hw_deinit(wlc_hw_info_t *wlc_hw, uint32 gpiomask_cache)
{
	/* BMAC_NOTE: split mac should not worry about pci cfg access to disable GPIOs. */
	bool xtal_set = FALSE;

	if (!wlc_hw->sbclk) {
		wlc_bmac_xtal(wlc_hw, ON);
		xtal_set = TRUE;
	}

	/* opposite sequence of wlc_led_init */
	if (wlc_hw->sih) {
		si_gpioout(wlc_hw->sih, gpiomask_cache, 0, GPIO_DRV_PRIORITY);
		si_gpioouten(wlc_hw->sih, gpiomask_cache, 0, GPIO_DRV_PRIORITY);
		si_gpioled(wlc_hw->sih, gpiomask_cache, 0);
	}

	if (xtal_set)
		wlc_bmac_xtal(wlc_hw, OFF);
}

void
wlc_bmac_led_hw_mask_init(wlc_hw_info_t *wlc_hw, uint32 mask)
{
	wlc_hw->led_gpio_mask = mask;
}

static void
wlc_bmac_led_hw_init(wlc_hw_info_t *wlc_hw)
{
	uint32 mask = wlc_hw->led_gpio_mask, val = 0;
	struct bmac_led *led;
	bmac_led_info_t *li = wlc_hw->ledh;

	if (!wlc_hw->sbclk)
		return;

	/* designate gpios driving LEDs . Make sure that we have the control */
	si_gpiocontrol(wlc_hw->sih, mask, 0, GPIO_DRV_PRIORITY);
	si_gpioled(wlc_hw->sih, mask, mask);

	/* Begin with LEDs off */
	for (led = &li->led[0]; led < &li->led[WL_LED_NUMGPIO]; led++) {
		if (!led->activehi)
			val |= (1 << led->pin);
	}
	val = val & mask;

	if (!(wlc_hw->boardflags2 & BFL2_TRISTATE_LED)) {
		li->gpioout_cache = si_gpioout(wlc_hw->sih, mask, val, GPIO_DRV_PRIORITY);
		si_gpioouten(wlc_hw->sih, mask, mask, GPIO_DRV_PRIORITY);
	} else {
		si_gpioout(wlc_hw->sih, mask, ~val & mask, GPIO_DRV_PRIORITY);
		li->gpioout_cache = si_gpioouten(wlc_hw->sih, mask, 0, GPIO_DRV_PRIORITY);
		/* for tristate leds, clear gpiopullup/gpiopulldown registers to
		 * allow the tristated gpio to float
		 */
		if (wlc_hw->sih->ccrev >= 20) {
			si_gpiopull(wlc_hw->sih, GPIO_PULLDN, mask, 0);
			si_gpiopull(wlc_hw->sih, GPIO_PULLUP, mask, 0);
		}
	}

	li->gpiomask_cache = mask;
#ifndef ATE_BUILD
	/* set override bit for the GPIO line controlling the LED */
	val = 0;
	for (led = &li->led[0]; led < &li->led[WL_LED_NUMGPIO]; led++) {
		if (led->pin_ledbh) {
			if (val == 0) {
				val = si_pmu_chipcontrol(wlc_hw->sih, PMU_CHIPCTL1, 0, 0);
			}

			val |= (1 << (PMU_CCA1_OVERRIDE_BIT_GPIO0 + led->pin));
		}
	}

	if (val) {
		si_pmu_chipcontrol(wlc_hw->sih, PMU_CHIPCTL1, 0xFFFFFFFF, val);
	}
#endif /*!ATE_BUILD */
}

/** called by the led_blink_timer at every li->led_blink_time interval */
static void
wlc_bmac_led_blink_timer(bmac_led_info_t *li)
{
	struct bmac_led *led;
#if OSL_SYSUPTIME_SUPPORT
	uint32 now = OSL_SYSUPTIME();
	/* Timer event can come early, and the LED on/off state change will be missed until the
	 * next li->led_blink_time cycle. Thus, the LED on/off state could be extended. To adjust
	 * for this situation, LED time may need to restart at the end of the current
	 * li->led_blink_time cycle
	 */
	wlc_hw_info_t *wlc_hw = (wlc_hw_info_t *)li->wlc_hw;
	uint time_togo;
	uint restart_time = 0;
	uint time_passed;

	/* blink each pin at its respective blinkrate */
	for (led = &li->led[0]; led < &li->led[WL_LED_NUMGPIO]; led++) {
		if (led->msec_on || led->msec_off) {
			bool change_state = FALSE;
			uint factor;

			time_passed = now - led->timestamp;

			/* Currently off */
			if ((led->next_state) || (led->restart)) {
				if (time_passed > led->msec_off)
					change_state = TRUE;
				else {
					time_togo = led->msec_off - time_passed;
					factor = (led->msec_off > 1000) ? 20 : 10;
					if (time_togo < li->led_blink_time) {
						if (time_togo < led->msec_off/factor ||
							time_togo < LED_BLINK_TIME) {
							if (li->led_blink_time - time_togo >
								li->led_blink_time/10)
								change_state = TRUE;
						}
						else {
							if (!restart_time)
								restart_time = time_togo;
							else if (time_togo < restart_time)
								restart_time = time_togo;
						}
					}
				}

				/* Blink on */
				if (led->restart || change_state) {
					wlc_bmac_led((wlc_hw_info_t*)li->wlc_hw,
					             (1<<led->pin), (1<<led->pin), led->activehi);
					led->next_state = OFF;
					led->timestamp = now;
					led->restart = FALSE;
				}
			}
			/* Currently on */
			else {
				if (time_passed > led->msec_on)
					change_state = TRUE;
				else {
							time_togo = led->msec_on - time_passed;
					if (time_togo < li->led_blink_time) {
						factor = (led->msec_on > 1000) ? 20 : 10;
						if (time_togo < led->msec_on/factor ||
							time_togo < LED_BLINK_TIME) {
							if (li->led_blink_time - time_togo >
								li->led_blink_time/10)
								change_state = TRUE;
						}
						else {
							if (!restart_time)
								restart_time = time_togo;
							else if (time_togo < restart_time)
								restart_time = time_togo;
						}
					}
				}

				/* Blink off  */
				if (change_state) {
					wlc_bmac_led((wlc_hw_info_t*)li->wlc_hw,
					             (1<<led->pin), 0, led->activehi);
					led->next_state = ON;
					led->timestamp = now;
				}
			}
		}
	}

	if (restart_time) {
#ifdef BCMDBG
		WL_TRACE(("restart led blink timer in %dms\n", restart_time));
#endif // endif
		wl_del_timer(wlc_hw->wlc->wl, li->led_blink_timer);
		wl_add_timer(wlc_hw->wlc->wl, li->led_blink_timer, restart_time, 0);
		li->blink_start = TRUE;
		li->blink_adjust = TRUE;
		}
	else if (li->blink_adjust) {
#ifdef BCMDBG
		WL_TRACE(("restore led_blink_time to %d\n", li->led_blink_time));
#endif // endif
		wlc_bmac_led_blink_event(wlc_hw, TRUE);
		li->blink_start = TRUE;
		li->blink_adjust = FALSE;
	}
#else
	for (led = &li->led[0]; led < &li->led[WL_LED_NUMGPIO]; led++) {
		if (led->blinkmsec) {
			if (led->blinkmsec > (int32) led->msec_on) {
				wlc_bmac_led((wlc_hw_info_t*)li->wlc_hw,
				             (1<<led->pin), 0, led->activehi);
			} else {
				wlc_bmac_led((wlc_hw_info_t*)li->wlc_hw,
				             (1<<led->pin), (1<<led->pin), led->activehi);
			}
			led->blinkmsec -= LED_BLINK_TIME;
			if (led->blinkmsec <= 0)
				led->blinkmsec = led->msec_on + led->msec_off;
		}
	}
#endif /* (OSL_SYSUPTIME_SUPPORT) */
}

static void
wlc_bmac_timer_led_blink(void *arg)
{
	wlc_info_t *wlc = (wlc_info_t*)arg;
	wlc_hw_info_t *wlc_hw = wlc->hw;

	if (DEVICEREMOVED(wlc)) {
		WL_ERROR(("wl%d: %s: dead chip\n", wlc_hw->unit, __FUNCTION__));
#ifdef WLC_HIGH
		wl_down(wlc->wl);
#endif // endif
		return;
	}

	wlc_bmac_led_blink_timer(wlc_hw->ledh);
}

bmac_led_info_t *
BCMATTACHFN(wlc_bmac_led_attach)(wlc_hw_info_t *wlc_hw)
{
	bmac_led_info_t *bmac_li;
	bmac_led_t *led;
	int i;
	char name[32];
	char *var;
	uint val;

	if ((bmac_li = (bmac_led_info_t *)MALLOC
			(wlc_hw->osh, sizeof(bmac_led_info_t))) == NULL) {
		printf(rstr_bmac_led_attach_out_of_mem_malloced_D_bytes,
			MALLOCED(wlc_hw->osh));
		goto fail;
	}
	bzero((char *)bmac_li, sizeof(bmac_led_info_t));

	led = &bmac_li->led[0];
	for (i = 0; i < WL_LED_NUMGPIO; i ++) {
		led->pin = i;
		led->activehi = TRUE;
#if OSL_SYSUPTIME_SUPPORT
		/* current time, in ms, for computing LED blink duration */
		led->timestamp = OSL_SYSUPTIME();
		led->next_state = ON; /* default to turning on */
#endif // endif
		led ++;
	}

	/* look for led gpio/behavior nvram overrides */
	for (i = 0; i < WL_LED_NUMGPIO; i++) {
		led = &bmac_li->led[i];

		snprintf(name, sizeof(name), rstr_ledbhD, i);

		if ((var = getvar(wlc_hw->vars, name)) == NULL) {
			snprintf(name, sizeof(name), rstr_wl0gpioD, i);
			if ((var = getvar(wlc_hw->vars, name)) == NULL) {
				continue;
			}
		}

		val = bcm_strtoul(var, NULL, 0);

		/* silently ignore old card srom garbage */
		if ((val & WL_LED_BEH_MASK) >= WL_LED_NUMBEHAVIOR)
			continue;

		led->pin = i;	/* gpio pin# == led index# */
		if (val & WL_LED_PMU_OVERRIDE) {
			led->pin_ledbh = TRUE;
		}
		led->activehi = (val & WL_LED_AL_MASK)? FALSE : TRUE;
	}

	bmac_li->wlc_hw = wlc_hw;
	if (!(bmac_li->led_blink_timer = wl_init_timer
			(wlc_hw->wlc->wl, wlc_bmac_timer_led_blink, wlc_hw->wlc,
	                                          "led_blink"))) {
		printf(rstr_wlD_led_attach_wl_init_timer_for_led_blink_timer_failed,
			wlc_hw->unit);
		goto fail;
	}

#if !OSL_SYSUPTIME_SUPPORT
	bmac_li->led_blink_time = LED_BLINK_TIME;
#endif // endif

	return bmac_li;

fail:
	if (bmac_li) {
		MFREE(wlc_hw->osh, bmac_li, sizeof(bmac_led_info_t));
	}
	return NULL;

}

int
BCMATTACHFN(wlc_bmac_led_detach)(wlc_hw_info_t *wlc_hw)
{
	bmac_led_info_t *li = wlc_hw->ledh;
	int callbacks = 0;

	if (li) {
		if (li->led_blink_timer) {
			if (!wl_del_timer(wlc_hw->wlc->wl, li->led_blink_timer))
				callbacks++;
			wl_free_timer(wlc_hw->wlc->wl, li->led_blink_timer);
			li->led_blink_timer = NULL;
		}

		MFREE(wlc_hw->osh, li, sizeof(bmac_led_info_t));
	}

	return callbacks;
}

static void
wlc_bmac_led_blink_off(bmac_led_info_t *li)
{
	struct bmac_led *led;

	/* blink each pin at its respective blinkrate */
	for (led = &li->led[0]; led < &li->led[WL_LED_NUMGPIO]; led++) {
		if (led->msec_on || led->msec_off) {
			wlc_bmac_led((wlc_hw_info_t*)li->wlc_hw,
				(1<<led->pin), 0, led->activehi);
#if OSL_SYSUPTIME_SUPPORT
			led->restart = TRUE;
#endif // endif
		}
	}
}

int
wlc_bmac_led_blink_event(wlc_hw_info_t *wlc_hw, bool blink)
{
	bmac_led_info_t *li = (bmac_led_info_t *)(wlc_hw->ledh);

	if (blink) {
		wl_del_timer(wlc_hw->wlc->wl, li->led_blink_timer);
		wl_add_timer(wlc_hw->wlc->wl, li->led_blink_timer, li->led_blink_time, 1);
		li->blink_start = TRUE;
	} else {
		if (!wl_del_timer(wlc_hw->wlc->wl, li->led_blink_timer))
			return 1;
		li->blink_start = FALSE;
		wlc_bmac_led_blink_off(li);
	}
	return 0;
}

void
wlc_bmac_led_set(wlc_hw_info_t *wlc_hw, int indx, uint8 activehi)
{
	bmac_led_t *led = &wlc_hw->ledh->led[indx];

	led->activehi = activehi;

	return;
}

void
wlc_bmac_led_blink(wlc_hw_info_t *wlc_hw, int indx, uint16 msec_on, uint16 msec_off)
{
	bmac_led_t *led = &wlc_hw->ledh->led[indx];
#if OSL_SYSUPTIME_SUPPORT
	bmac_led_info_t *li = (bmac_led_info_t *)(wlc_hw->ledh);
	uint num_leds_set = 0;
	uint led_blink_rates[WL_LED_NUMGPIO];
	uint tmp, a, b, i;
	led_blink_rates[0] = 1000; /* 1 sec, default timer */
#endif // endif

	led->msec_on = msec_on;
	led->msec_off = msec_off;

#if !OSL_SYSUPTIME_SUPPORT
	led->blinkmsec = msec_on + msec_off;
#else
	if ((led->msec_on != msec_on) || (led->msec_off != msec_off)) {
		led->restart = TRUE;
	}

	/* recompute to an optimized blink rate timer interval */
	for (led = &li->led[0]; led < &li->led[WL_LED_NUMGPIO]; led++) {
		if (!(led->msec_on || led->msec_off)) {
			led->restart = TRUE;
			continue;
		}

		/* compute the GCF of this particular LED's on+off rates */
		b = led->msec_off;
		a = led->msec_on;
		while (b != 0) {
			tmp = b;
			b = a % b;
			a = tmp;
		}

		led_blink_rates[num_leds_set++] = a;
	}

	/* compute the GCF across all LEDs, if more than one */
	a = led_blink_rates[0];

	for (i = 1; i < num_leds_set; i++) {
		b = led_blink_rates[i];
		while (b != 0) {
			tmp = b;
			b = a % b;
			a = tmp; /* A is the running GCF */
		}
	}

	li->led_blink_time = MAX(a, LED_BLINK_TIME);

	if (num_leds_set) {
		if ((li->blink_start) && !li->blink_adjust) {
			wlc_bmac_led_blink_event(wlc_hw, FALSE);
			wlc_bmac_led_blink_event(wlc_hw, TRUE);
		}
	}

#endif /* !(OSL_SYSUPTIME_SUPPORT) */
	return;
}

void
wlc_bmac_blink_sync(wlc_hw_info_t *wlc_hw, uint32 led_pins)
{
#if OSL_SYSUPTIME_SUPPORT
	bmac_led_info_t *li = wlc_hw->ledh;
	int i;

	for (i = 0; i < WL_LED_NUMGPIO; i++) {
		if (led_pins & (0x1 << i)) {
			li->led[i].restart = TRUE;
		}
	}
#endif // endif

	return;
}

/** turn gpio bits on or off */
void
wlc_bmac_led(wlc_hw_info_t *wlc_hw, uint32 mask, uint32 val, bool activehi)
{
	bmac_led_info_t *li = wlc_hw->ledh;
	bool off = (val != mask);

	ASSERT((val & ~mask) == 0);

	if (!wlc_hw->sbclk)
		return;

	if (!activehi)
		val = ((~val) & mask);

	/* Tri-state the GPIO if the board flag is set */
	if (wlc_hw->boardflags2 & BFL2_TRISTATE_LED) {
		if ((!activehi && ((val & mask) == (li->gpioout_cache & mask))) ||
		    (activehi && ((val & mask) != (li->gpioout_cache & mask))))
			li->gpioout_cache = si_gpioouten(wlc_hw->sih, mask, off ? 0 : mask,
			                                 GPIO_DRV_PRIORITY);
	}
	else
		/* prevent the unnecessary writes to the gpio */
		if ((val & mask) != (li->gpioout_cache & mask))
			/* Traditional GPIO behavior */
			li->gpioout_cache = si_gpioout(wlc_hw->sih, mask, val,
			                               GPIO_DRV_PRIORITY);
}
#endif /* WLLED */

int
wlc_bmac_iovar_dispatch(wlc_hw_info_t *wlc_hw, uint32 actionid,
	void *params, uint p_len, void *arg, int len, int val_size)
{
	int err = 0;
	int32 int_val = 0;
	int32 int_val2 = 0;
	int32 *ret_int_ptr;
	bool bool_val;
	bool bool_val2;

	/* convenience int and bool vals for first 8 bytes of buffer */
	if (p_len >= (int)sizeof(int_val))
		bcopy(params, &int_val, sizeof(int_val));

	if (p_len >= (int)sizeof(int_val) * 2)
		bcopy((void*)((uintptr)params + sizeof(int_val)), &int_val2, sizeof(int_val));

	/* convenience int ptr for 4-byte gets (requires int aligned arg) */
	ret_int_ptr = (int32 *)arg;

	bool_val = (int_val != 0) ? TRUE : FALSE;
	bool_val2 = (int_val2 != 0) ? TRUE : FALSE;
	BCM_REFERENCE(bool_val2);

	WL_TRACE(("%s(): actionid=%d, p_len=%d, len=%d\n", __FUNCTION__, actionid, p_len, len));

	switch (actionid) {
#ifdef WLDIAG
	case IOV_GVAL(IOV_BMAC_DIAG): {
		uint32 result;
		uint32 diagtype;

		/* recover diagtype to run */
		bcopy((char *)params, (char *)(&diagtype), sizeof(diagtype));
		err = wlc_diag(wlc_hw->wlc, diagtype, &result);
		bcopy((char *)(&result), arg, sizeof(diagtype)); /* copy result to be buffer */
		break;
	}
#endif /* WLDIAG */

#ifdef WLLED
	case IOV_GVAL(IOV_BMAC_SBGPIOTIMERVAL):
	case IOV_GVAL(IOV_BMAC_LEDDC):
		*ret_int_ptr = si_gpiotimerval(wlc_hw->sih, 0, 0);
		break;
	case IOV_SVAL(IOV_BMAC_SBGPIOTIMERVAL):
	case IOV_SVAL(IOV_BMAC_LEDDC):
		si_gpiotimerval(wlc_hw->sih, ~0, int_val);
		break;
#endif /* WLLED */

#if (defined(WLTEST) && !defined(WLTEST_DISABLED))
	case IOV_SVAL(IOV_BMAC_SBGPIOOUT): {
		uint8 gpio;
		uint32 mask; /* GPIO pin mask */
		uint32 val;  /* GPIO value to program */
		mask = ((uint32*)params)[0];
		val = ((uint32*)params)[1];

		/* WARNING: This is unconditionally assigning the GPIOs to Chipcommon */
		/* Make it override all other priorities */
		if (CHIPID(wlc_hw->sih->chip) == BCM4345_CHIP_ID) {
			/* First get the GPIO pin */
			for (gpio = 0; gpio < CC4345_PIN_GPIO_15; gpio ++) {
				if ((mask >> gpio) & 0x1)
					break;
			}
			si_gci_enable_gpio(wlc_hw->sih, gpio, mask, val);
		}
		else {
			si_gpiocontrol(wlc_hw->sih, mask, 0, GPIO_HI_PRIORITY);
			si_gpioouten(wlc_hw->sih, mask, mask, GPIO_HI_PRIORITY);
			si_gpioout(wlc_hw->sih, mask, val, GPIO_HI_PRIORITY);
		}
		break;
	}

	case IOV_GVAL(IOV_BMAC_SBGPIOOUT): {
		uint32 gpio_cntrl;
		uint32 gpio_out;
		uint32 gpio_outen;

		if (len < (int) (sizeof(uint32) * 3))
			return BCME_BUFTOOSHORT;

		gpio_cntrl = si_gpiocontrol(wlc_hw->sih, 0, 0, GPIO_HI_PRIORITY);
		gpio_out = si_gpioout(wlc_hw->sih, 0, 0, GPIO_HI_PRIORITY);
		gpio_outen = si_gpioouten(wlc_hw->sih, 0, 0, GPIO_HI_PRIORITY);

		((uint32*)arg)[0] = gpio_cntrl;
		((uint32*)arg)[1] = gpio_out;
		((uint32*)arg)[2] = gpio_outen;
		break;
	}

	case IOV_SVAL(IOV_BMAC_CCGPIOCTRL):
		si_gpiocontrol(wlc_hw->sih, ~0, int_val, GPIO_HI_PRIORITY);
		break;
	case IOV_GVAL(IOV_BMAC_CCGPIOCTRL):
		*ret_int_ptr = si_gpiocontrol(wlc_hw->sih, 0, 0, GPIO_HI_PRIORITY);
		break;

	case IOV_SVAL(IOV_BMAC_CCGPIOOUT):
		si_gpioout(wlc_hw->sih, ~0, int_val, GPIO_HI_PRIORITY);
		break;
	case IOV_GVAL(IOV_BMAC_CCGPIOOUT):
		*ret_int_ptr = si_gpioout(wlc_hw->sih, 0, 0, GPIO_HI_PRIORITY);
		break;
	case IOV_SVAL(IOV_BMAC_CCGPIOOUTEN):
		si_gpioouten(wlc_hw->sih, ~0, int_val, GPIO_HI_PRIORITY);
		break;
	case IOV_GVAL(IOV_BMAC_CCGPIOOUTEN):
		*ret_int_ptr = si_gpioouten(wlc_hw->sih, 0, 0, GPIO_HI_PRIORITY);
		break;

#ifdef WLTEST
	case IOV_SVAL(IOV_BMAC_BOARDFLAGS): {
		/* XXX This iovar is designed for 4323 only because we don't support nvm
		 * file, other chips should use generic nvm file for setting different
		 * boardflags
		 */
		if (CHIPID(wlc_hw->sih->chip) == BCM4322_CHIP_ID ||
			CHIPID(wlc_hw->sih->chip) == BCM43231_CHIP_ID) {
			wlc_hw->boardflags = int_val;

			/* wlc_hw->sih->boardflags is not updated because it's in
			 * read-only region and not used by 4323/43231
			 */
			/* wlc_hw->sih->boardflags = int_val; */

			/* some branded-boards boardflags srom programmed incorrectly */
			if ((wlc_hw->sih->boardvendor == VENDOR_APPLE) &&
			    (wlc_hw->sih->boardtype == 0x4e) && (wlc_hw->boardrev >=
			                                         0x41))
				wlc_hw->boardflags |= BFL_PACTRL;

			/* PR12527: don't power down pll for pre-4306C0 chips or boards
			 * w/GPRS
			 */
			if (D11REV_LE(wlc_hw->corerev, 4) || (wlc_hw->boardflags &
			                                      BFL_NOPLLDOWN))
				wlc_bmac_pllreq(wlc_hw, TRUE, WLC_PLLREQ_SHARED);
		}
		break;
	}

	case IOV_SVAL(IOV_BMAC_BOARDFLAGS2): {
		/* XXX This iovar is designed for 4323 only because we don't support nvm
		 * file, other chips should use generic nvm file for setting different
		 * boardflags
		 */
		if (CHIPID(wlc_hw->sih->chip) == BCM4322_CHIP_ID ||
			CHIPID(wlc_hw->sih->chip) == BCM43231_CHIP_ID) {
			int orig_band = wlc_hw->band->bandunit;

			wlc_hw->boardflags2 = int_val;

			/* Update A-band spur WAR */
			wlc_setxband(wlc_hw, WLC_BAND_5G);
			wlc_phy_boardflag_upd(wlc_hw->band->pi);
			wlc_setxband(wlc_hw, orig_band);
		}
		break;
	}
#endif /* WLTEST */
#endif // endif

	case IOV_GVAL(IOV_BMAC_CCGPIOIN):
		*ret_int_ptr = si_gpioin(wlc_hw->sih);
		break;

	case IOV_GVAL(IOV_BMAC_WPSGPIO): {
		char *var;

		if ((var = getvar(wlc_hw->vars, "wpsgpio")))
			*ret_int_ptr = (uint32)bcm_strtoul(var, NULL, 0);
		else {
			*ret_int_ptr = -1;
			err = BCME_NOTFOUND;
		}

		break;
	}

	case IOV_GVAL(IOV_BMAC_WPSLED): {
		char *var;

		if ((var = getvar(wlc_hw->vars, "wpsled")))
			*ret_int_ptr = (uint32)bcm_strtoul(var, NULL, 0);
		else {
			*ret_int_ptr = -1;
			err = BCME_NOTFOUND;
		}

		break;
	}

	case IOV_GVAL(IOV_BMAC_BTCLOCK_TUNE_WAR):
		*ret_int_ptr = wlc_hw->btclock_tune_war;
		break;

	case IOV_SVAL(IOV_BMAC_BTCLOCK_TUNE_WAR):
		wlc_hw->btclock_tune_war = bool_val;
		break;

	case IOV_GVAL(IOV_BMAC_BT_REGS_READ): {
		/* the size of output dump can not be larger than the buffer size */
		if (int_val2 > len)
			err = BCME_BUFTOOSHORT;
		else
			err = wlc_bmac_bt_regs_read(wlc_hw, int_val, int_val2, (uint32*)arg);
		break;
	}

#if (defined(BCMNVRAMR) || defined(BCMNVRAMW)) && (defined(WLTEST) && \
	!defined(WLTEST_DISABLED))
	case IOV_GVAL(IOV_BMAC_OTPDUMP): {
		void *oh;
		uint32 macintmask;
		bool wasup;
		uint32 min_res_mask = 0;

		/* intrs off */
		macintmask = wl_intrsoff(wlc_hw->wlc->wl);

		if (!(wasup = si_is_otp_powered(wlc_hw->sih)))
			si_otp_power(wlc_hw->sih, TRUE, &min_res_mask);

		if ((oh = otp_init(wlc_hw->sih)) == NULL) {
			err = BCME_NOTFOUND;
		} else if (otp_dump(oh, int_val, (char *)arg, len) <= 0) {
			err = BCME_BUFTOOSHORT;
		}

		if (!wasup)
			si_otp_power(wlc_hw->sih, FALSE, &min_res_mask);

		/* restore intrs */
		wl_intrsrestore(wlc_hw->wlc->wl, macintmask);

		break;
	}

	case IOV_GVAL(IOV_BMAC_OTPSTAT): {
		void *oh;
		uint32 macintmask;
		bool wasup;
		uint32 min_res_mask = 0;

		/* intrs off */
		macintmask = wl_intrsoff(wlc_hw->wlc->wl);

		if (!(wasup = si_is_otp_powered(wlc_hw->sih)))
			si_otp_power(wlc_hw->sih, TRUE, &min_res_mask);

		if ((oh = otp_init(wlc_hw->sih)) == NULL) {
			err = BCME_NOTFOUND;
		} else if (otp_dumpstats(oh, int_val, (char *)arg, len) <= 0) {
			err = BCME_BUFTOOSHORT;
		}

		if (!wasup)
			si_otp_power(wlc_hw->sih, FALSE, &min_res_mask);

		/* restore intrs */
		wl_intrsrestore(wlc_hw->wlc->wl, macintmask);

		break;
	}

#endif // endif

	case IOV_GVAL(IOV_BMAC_PCIEADVCORRMASK):
			if ((BUSTYPE(wlc_hw->sih->bustype) != PCI_BUS) ||
			    (wlc_hw->sih->buscoretype != PCIE_CORE_ID)) {
			err = BCME_UNSUPPORTED;
			break;
		}

#ifdef WLC_HIGH
		*ret_int_ptr = si_pciereg(wlc_hw->sih, PCIE_ADV_CORR_ERR_MASK,
			0, 0, PCIE_CONFIGREGS);
#endif // endif
		break;

	case IOV_SVAL(IOV_BMAC_PCIEADVCORRMASK):
	        if ((BUSTYPE(wlc_hw->sih->bustype) != PCI_BUS) ||
	            (wlc_hw->sih->buscoretype != PCIE_CORE_ID)) {
			err = BCME_UNSUPPORTED;
			break;
		}

		/* Set all errors if -1 or else mask off undefined bits */
		if (int_val == -1)
			int_val = ALL_CORR_ERRORS;

		int_val &= ALL_CORR_ERRORS;
#ifdef WLC_HIGH
		si_pciereg(wlc_hw->sih, PCIE_ADV_CORR_ERR_MASK, 1, int_val,
			PCIE_CONFIGREGS);
#endif // endif
		break;

	case IOV_GVAL(IOV_BMAC_PCIEASPM): {
		/* this command is to hide the details, but match the lcreg
		   #define PCIE_CLKREQ_ENAB		0x100
		   #define PCIE_ASPM_L1_ENAB        	2
		   #define PCIE_ASPM_L0s_ENAB       	1
		*/
		uint8 clkreq = 0;
		uint32 aspm = 0;
#ifdef WLC_HIGH
		clkreq = si_pcieclkreq(wlc_hw->sih, 0, 0);
		aspm = si_pcielcreg(wlc_hw->sih, 0, 0);
#endif // endif
		*ret_int_ptr = ((clkreq & 0x1) << 8) | (aspm & PCIE_ASPM_ENAB);
		break;
	}

	case IOV_SVAL(IOV_BMAC_PCIEASPM): {
#ifdef WLC_HIGH
		uint32 tmp;
		tmp = si_pcielcreg(wlc_hw->sih, 0, 0);
		si_pcielcreg(wlc_hw->sih, PCIE_ASPM_ENAB,
			(tmp & ~PCIE_ASPM_ENAB) | (int_val & PCIE_ASPM_ENAB));

		si_pcieclkreq(wlc_hw->sih, 1, ((int_val & 0x100) >> 8));
#endif // endif
		break;
	}
#ifdef BCMDBG
	case IOV_GVAL(IOV_BMAC_PCIECLKREQ):
		*ret_int_ptr = si_pcieclkreq(wlc_hw->sih, 0, 0);
		break;

	case IOV_SVAL(IOV_BMAC_PCIECLKREQ):
		if (int_val < AUTO || int_val > ON) {
			err = BCME_RANGE;
			break;
		}

		/* For AUTO, disable clkreq and then rest of the
		 * state machine will take care of it
		 */
		if (int_val == AUTO)
			si_pcieclkreq(wlc_hw->sih, 1, 0);
		else
			si_pcieclkreq(wlc_hw->sih, 1, (uint)int_val);
		break;

	case IOV_GVAL(IOV_BMAC_PCIELCREG):
		*ret_int_ptr = si_pcielcreg(wlc_hw->sih, 0, 0);
		break;

	case IOV_SVAL(IOV_BMAC_PCIELCREG):
		si_pcielcreg(wlc_hw->sih, 3, (uint)int_val);
		break;

#ifdef STA
	case IOV_SVAL(IOV_BMAC_DMALPBK):
		if (BUSTYPE(wlc_hw->sih->bustype) == PCI_BUS &&
		    !PIO_ENAB_HW(wlc_hw)) {
			if (wlc_hw->dma_lpbk == bool_val)
				break;
			wlc_bmac_dma_lpbk(wlc_hw, bool_val);
			wlc_hw->dma_lpbk = bool_val;
		} else
			err = BCME_UNSUPPORTED;

		break;
#endif /* STA */
#endif /* BCMDBG */

	case IOV_SVAL(IOV_BMAC_PCIEREG):
		if (p_len < (int)sizeof(int_val) * 2) {
			err = BCME_BUFTOOSHORT;
			break;
		}
		if (int_val < 0) {
			err = BCME_BADARG;
			break;
		}
		si_pciereg(wlc_hw->sih, int_val, 1, int_val2, PCIE_PCIEREGS);
		break;

	case IOV_GVAL(IOV_BMAC_PCIEREG):
		if (p_len < (int)sizeof(int_val)) {
			err = BCME_BUFTOOSHORT;
			break;
		}
		if (int_val < 0) {
			err = BCME_BADARG;
			break;
		}
		*ret_int_ptr = si_pciereg(wlc_hw->sih, int_val, 0, 0, PCIE_PCIEREGS);
		break;

	case IOV_SVAL(IOV_BMAC_EDCRS):
		if (!(WLCISNPHY(wlc_hw->band) && (D11REV_GE(wlc_hw->corerev, 16))) &&
			!WLCISHTPHY(wlc_hw->band) && !WLCISACPHY(wlc_hw->band)) {
			err = BCME_UNSUPPORTED;
			break;
		}
		if (bool_val) {
			wlc_bmac_ifsctl_edcrs_set(wlc_hw, WLCISHTPHY(wlc_hw->band));
		} else {
			if (WLCISACPHY(wlc_hw->band))
				wlc_bmac_ifsctl_vht_set(wlc_hw, OFF);
			else
				wlc_bmac_ifsctl1_regshm(wlc_hw, (IFS_CTL1_EDCRS |
					IFS_CTL1_EDCRS_20L | IFS_CTL1_EDCRS_40), 0);
		}
		break;

	case IOV_GVAL(IOV_BMAC_EDCRS):
		if (!(WLCISNPHY(wlc_hw->band) && (D11REV_GE(wlc_hw->corerev, 16))) &&
			!WLCISHTPHY(wlc_hw->band) && !WLCISACPHY(wlc_hw->band)) {
			err = BCME_UNSUPPORTED;
			break;
		}
		{
			uint16 val;
			val = wlc_bmac_read_shm(wlc_hw, M_IFSCTL1);

			if (WLCISACPHY(wlc_hw->band))
				*ret_int_ptr = (val & IFS_CTL_ED_SEL_MASK) ? TRUE:FALSE;
			else if (WLCISHTPHY(wlc_hw->band))
				*ret_int_ptr = (val & IFS_EDCRS_MASK) ? TRUE:FALSE;
			else
				*ret_int_ptr = (val & IFS_CTL1_EDCRS) ? TRUE:FALSE;
		}
		break;

	case IOV_SVAL(IOV_BMAC_PCIESERDESREG): {
		int32 int_val3;
		if (p_len < (int)sizeof(int_val) * 3) {
			err = BCME_BUFTOOSHORT;
			break;
		}
		if (int_val < 0 || int_val2 < 0) {
			err = BCME_BADARG;
			break;
		}
		if (BUSTYPE(wlc_hw->sih->bustype) != PCI_BUS) {
			err = BCME_UNSUPPORTED;
			break;
		}

		bcopy((void*)((uintptr)params + 2 * sizeof(int_val)), &int_val3, sizeof(int_val));
		/* write dev/offset/val to serdes */
		si_pcieserdesreg(wlc_hw->sih, int_val, int_val2, 1, int_val3);
		break;
	}

	case IOV_GVAL(IOV_BMAC_PCIESERDESREG): {
		if (p_len < (int)sizeof(int_val) * 2) {
			err = BCME_BUFTOOSHORT;
			break;
		}
		if (int_val < 0 || int_val2 < 0) {
			err = BCME_BADARG;
			break;
		}

		*ret_int_ptr = si_pcieserdesreg(wlc_hw->sih, int_val, int_val2, 0, 0);
		break;
	}

#ifdef BCMSDIO
	case IOV_GVAL(IOV_BMAC_SDCIS): {
		*(char *)arg = 0;
		bcmstrncat(arg, "\nFunc 0\n", len - 1);
		bcmsdh_cis_read(wlc_hw->sdh, 0x10, (char *)arg + strlen(arg), 49 * 32);
		bcmstrncat(arg, "\nFunc 1\n", len - strlen(arg) - 1);
		bcmsdh_cis_read(wlc_hw->sdh, 0x11, (char *)arg+strlen(arg), 49 * 32);
		break;
	}
	case IOV_GVAL(IOV_BMAC_SDIO_DRIVE):
		*ret_int_ptr = wlc_hw->sdiod_drive_strength;
		break;
	case IOV_SVAL(IOV_BMAC_SDIO_DRIVE): {
		if ((int_val >= 2) && (int_val <= 12)) {
			wlc_hw->sdiod_drive_strength = int_val;
			si_sdiod_drive_strength_init(wlc_hw->sih, wlc_hw->osh,
			                             wlc_hw->sdiod_drive_strength);
		}
		break;
	}
#endif /* BCMSDIO */

#if (defined(WLTEST) && !defined(WLTEST_DISABLED))
	case IOV_SVAL(IOV_BMAC_PLLRESET): {
		uint32 macintmask;
		wlc_info_t *wlc = wlc_hw->wlc;
		if (wlc_hw->up)
		{
			/* disable interrupts */
			macintmask = wl_intrsoff(wlc->wl);
			err = si_pll_reset(wlc_hw->sih);
			/* restore macintmask */
			wl_intrsrestore(wlc->wl, macintmask);
			wlc_phy_resetcntrl_regwrite(wlc_hw->band->pi);
		}
		else
		{
			err = BCME_UNSUPPORTED;
		}
		break;
	}
#ifdef BCMNVRAMW
	case IOV_SVAL(IOV_BMAC_OTPW):
	case IOV_SVAL(IOV_BMAC_NVOTPW): {
		void *oh;
		uint32 macintmask;
		uint32 min_res_mask = 0;

		/* intrs off */
		macintmask = wl_intrsoff(wlc_hw->wlc->wl);

		if (actionid == IOV_SVAL(IOV_BMAC_OTPW)) {
			err = otp_write_region(wlc_hw->sih, OTP_HW_RGN,
			                       (uint16 *)params, p_len / 2, 0);
		} else {
			bool wasup;

			if (!(wasup = si_is_otp_powered(wlc_hw->sih)))
				si_otp_power(wlc_hw->sih, TRUE, &min_res_mask);

			oh = otp_init(wlc_hw->sih);
			if (oh != NULL)
				err = otp_nvwrite(oh, (uint16 *)params, p_len / 2);
			else
				err = BCME_NOTFOUND;

			if (!wasup)
				si_otp_power(wlc_hw->sih, FALSE, &min_res_mask);
		}

		/* restore intrs */
		wl_intrsrestore(wlc_hw->wlc->wl, macintmask);

		break;
	}

	case IOV_SVAL(IOV_BMAC_CISVAR): {
		/* note: for SDIO, this IOVAR will fail on an unprogrammed OTP. */
		uint32 macintmask = wl_intrsoff(wlc_hw->wlc->wl);
		bool wasup;
		uint32 min_res_mask = 0;
		if (!(wasup = si_is_otp_powered(wlc_hw->sih))) {
			si_otp_power(wlc_hw->sih, TRUE, &min_res_mask);
			if (!si_is_otp_powered(wlc_hw->sih)) {
				err = BCME_NOTFOUND;
				break;
			}
		}

		/* for OTP wrvar */
		if (CHIPID(wlc_hw->sih->chip) == BCM43237_CHIP_ID ||
		    BCM4365_CHIP(wlc_hw->sih->chip) ||
		    (CHIPID(wlc_hw->sih->chip) == BCM43143_CHIP_ID &&
		    CST43143_CHIPMODE_SDIOD(wlc_hw->sih->chipst)) ||
		    (CHIPID(wlc_hw->sih->chip) == BCM4345_CHIP_ID &&
		    CST4345_CHIPMODE_SDIOD(wlc_hw->sih->chipst)) ||
		    (CHIPID(wlc_hw->sih->chip) == BCM4335_CHIP_ID &&
		    CST4335_CHIPMODE_SDIOD(wlc_hw->sih->chipst)) ||
		    (BCM4350_CHIP(wlc_hw->sih->chip) &&
		    CST4350_CHIPMODE_SDIOD(wlc_hw->sih->chipst))) {
			err = otp_cis_append_region(wlc_hw->sih, OTP_HW_RGN, (char*)params, p_len);
		} else {
			err = otp_cis_append_region(wlc_hw->sih, OTP_SW_RGN, (char*)params, p_len);
		}
		if (!wasup)
			si_otp_power(wlc_hw->sih, FALSE, &min_res_mask);

		wl_intrsrestore(wlc_hw->wlc->wl, macintmask);

		break;
	}

	case IOV_GVAL(IOV_BMAC_OTPLOCK): {
		uint32 macintmask;
		macintmask = wl_intrsoff(wlc_hw->wlc->wl);

		*ret_int_ptr = otp_lock(wlc_hw->sih);

		wl_intrsrestore(wlc_hw->wlc->wl, macintmask);

		break;
	}
#endif /* BCMNVRAMW */

#if defined(BCMNVRAMR) || defined(BCMNVRAMW)
	case IOV_GVAL(IOV_BMAC_OTP_RAW_READ):
	{
		uint32 macintmask;
		uint32 min_res_mask = 0;
		macintmask = wl_intrsoff(wlc_hw->wlc->wl);
		if (si_is_otp_disabled(wlc_hw->sih)) {
			WL_INFORM(("OTP do not exist\n"));
			err = BCME_NOTFOUND;
		} else {
			bool wasup;
			uint32 i, offset, data = 0;
			uint16 tmp;
			void * oh;
			if (!(wasup = si_is_otp_powered(wlc_hw->sih)))
				si_otp_power(wlc_hw->sih, TRUE, &min_res_mask);

			oh = otp_init(wlc_hw->sih);
			if (oh == NULL)
				err = BCME_NOTFOUND;
			else  {
				offset = (*(uint32 *)params);
				offset *= 16;
				for (i = 0; i < 16; i++) {
					tmp = otp_read_bit(oh, i + offset);
					data |= (tmp << i);
				}
				*ret_int_ptr = data;
				WL_TRACE(("OTP_RAW_READ, offset %x:%x\n", offset, data));
			}
			if (!wasup)
				si_otp_power(wlc_hw->sih, FALSE, &min_res_mask);
		}
		wl_intrsrestore(wlc_hw->wlc->wl, macintmask);
		break;
	}

	case IOV_GVAL(IOV_BMAC_CIS_SOURCE): {
		if ((*ret_int_ptr = wlc_bmac_cissource(wlc_hw)) == BCME_ERROR)
			err = BCME_ERROR;
		break;
	}

	case IOV_GVAL(IOV_BMAC_OTP_RAW):
	{
		uint32 macintmask;
		uint32 min_res_mask = 0;
		macintmask = wl_intrsoff(wlc_hw->wlc->wl);
		if (si_is_otp_disabled(wlc_hw->sih)) {
			WL_INFORM(("OTP do not exist\n"));
			err = BCME_NOTFOUND;
		} else {
			bool wasup;
			void * oh;

			if (!(wasup = si_is_otp_powered(wlc_hw->sih)))
				si_otp_power(wlc_hw->sih, TRUE, &min_res_mask);

			oh = otp_init(wlc_hw->sih);
			if (oh == NULL)
				err = BCME_NOTFOUND;
			else  {
				uint32 i, j, offset, bits;
				uint8 tmp, data, *ptr;

				offset = int_val;
				bits = int_val2;

				ptr = (uint8 *)arg;
				for (i = 0; i < bits; ) {
					data = 0;
					for (j = 0; j < 8; j++, i++) {
						if (i >= bits)
							break;
						tmp = (uint8)otp_read_bit(oh, i + offset);
						data |= (tmp << j);
					}
					*ptr++ = data;
				}
			}
			if (!wasup)
				si_otp_power(wlc_hw->sih, FALSE, &min_res_mask);
		}
		wl_intrsrestore(wlc_hw->wlc->wl, macintmask);
		break;
	}
#endif  /* defined(BCMNVRAMR) || defined (BCMNVRAMW) */

#if defined(BCMNVRAMW)
	case IOV_SVAL(IOV_BMAC_OTP_RAW):
	{
		uint32 macintmask;
		uint32 min_res_mask = 0;
		macintmask = wl_intrsoff(wlc_hw->wlc->wl);
		if (si_is_otp_disabled(wlc_hw->sih)) {
			WL_INFORM(("OTP do not exist\n"));
			err = BCME_NOTFOUND;
		} else {
			bool wasup;
			void * oh;

			if (!(wasup = si_is_otp_powered(wlc_hw->sih)))
				si_otp_power(wlc_hw->sih, TRUE, &min_res_mask);

			oh = otp_init(wlc_hw->sih);
			if (oh == NULL)
				err = BCME_NOTFOUND;
			else  {
				uint32 offset, bits;
				uint8 *ptr;

				offset = int_val;
				bits = int_val2;
				ptr = (uint8 *)params + 2 * sizeof(int_val);

				err = otp_write_bits(oh, offset, bits, ptr);
			}
			if (!wasup)
				si_otp_power(wlc_hw->sih, FALSE, &min_res_mask);
		}
		wl_intrsrestore(wlc_hw->wlc->wl, macintmask);
		break;
	}
#endif /* BCMNVRAMW */

	case IOV_GVAL(IOV_BMAC_DEVPATH): {
		char devpath[SI_DEVPATH_BUFSZ];
		char devpath_pcie[SI_DEVPATH_BUFSZ];
		int devpath_length, pcie_devpath_length;
		int i;
		char *nvram_value;

		si_devpath(wlc_hw->sih, (char *)arg, SI_DEVPATH_BUFSZ);

		devpath_length = strlen((char *)arg);

		if (devpath_length && ((char *)arg)[devpath_length-1] == '/')
			devpath_length--;

		if (BUSTYPE(wlc_hw->sih->bustype) == PCI_BUS) {
			si_devpath_pcie(wlc_hw->sih, devpath_pcie, SI_DEVPATH_BUFSZ);
			pcie_devpath_length = strlen(devpath_pcie);
			if (pcie_devpath_length && devpath_pcie[pcie_devpath_length-1] == '/')
				pcie_devpath_length--;
		} else
			pcie_devpath_length = 0;

		for (i = 0; i < 10; i++) {
			snprintf(devpath, sizeof(devpath), "devpath%d", i);
			nvram_value = nvram_get(devpath);
			if (nvram_value &&
				(memcmp((char *)arg, nvram_value, devpath_length) == 0 ||
				(pcie_devpath_length &&
				memcmp(devpath_pcie, nvram_value, pcie_devpath_length) == 0))) {
				snprintf((char *)arg, SI_DEVPATH_BUFSZ, "%d:", i);
				break;
			}
		}

		break;
	}
#endif // endif

	case IOV_GVAL(IOV_BMAC_SROM): {
		srom_rw_t *s = (srom_rw_t *)arg;
		bool was_enabled;
		uint32 macintmask;

		/* intrs off */
		macintmask = wl_intrsoff(wlc_hw->wlc->wl);

		if (si_is_sprom_available(wlc_hw->sih)) {
			if (!(was_enabled = si_is_sprom_enabled(wlc_hw->sih)))
				si_sprom_enable(wlc_hw->sih, TRUE);
			if (srom_read(wlc_hw->sih, wlc_hw->sih->bustype,
			              (void *)(uintptr)wlc_hw->regs, wlc_hw->osh,
			              s->byteoff, s->nbytes, s->buf, FALSE))
				err = BCME_ERROR;
			if (!was_enabled)
				si_sprom_enable(wlc_hw->sih, FALSE);
#if defined(BCMNVRAMR) || defined(BCMNVRAMW)
		} else if (!si_is_otp_disabled(wlc_hw->sih)) {
#if (defined(WLTEST) && !defined(WLTEST_DISABLED))
			err = otp_read_region(wlc_hw->sih, OTP_HW_RGN, s->buf,
			                      &s->nbytes);
#else
			err = BCME_UNSUPPORTED;
#endif // endif
#endif /* BCMNVRAMR || defined (BCMNVRAMW) && !defined(WLTEST_DISABLED) */
		} else
			err = BCME_NOTFOUND;

#if defined(DSLCPE)
#if defined(DSLCPE_WOMBO)
		if (err) {
			read_sromfile((void *)wlc_hw->sih->wl_srom_sw_map,
				s->buf, s->byteoff, s->nbytes);
			err = 0;
		}
#endif /* DSLCPE_WOMBO */
		/* Updated srom by user's change */
		sprom_update_params(wlc_hw->sih, s->buf);
#endif /* DSLCPE */

		/* restore intrs */
		wl_intrsrestore(wlc_hw->wlc->wl, macintmask);
		break;
	}

#if (defined(BCMDBG) || (defined(WLTEST)&& !defined(WLTEST_DISABLED)))
	case IOV_SVAL(IOV_BMAC_SROM): {
		srom_rw_t *s = (srom_rw_t *)params;
		bool was_enabled;
		uint32 macintmask;

		/* intrs off */
		macintmask = wl_intrsoff(wlc_hw->wlc->wl);

		if (si_is_sprom_available(wlc_hw->sih)) {
			if (!(was_enabled = si_is_sprom_enabled(wlc_hw->sih)))
				si_sprom_enable(wlc_hw->sih, TRUE);
			if (srom_write(wlc_hw->sih, wlc_hw->sih->bustype,
			               (void *)(uintptr)wlc_hw->regs, wlc_hw->osh,
			               s->byteoff, s->nbytes, s->buf))
				err = BCME_ERROR;
			if (!was_enabled)
				si_sprom_enable(wlc_hw->sih, FALSE);
		} else if (!si_is_otp_disabled(wlc_hw->sih)) {
			/* srwrite to SROM format OTP */
			err = srom_otp_write_region_crc(wlc_hw->sih, s->nbytes, s->buf,
			                                TRUE);
		} else
			err = BCME_NOTFOUND;

		/* restore intrs */
		wl_intrsrestore(wlc_hw->wlc->wl, macintmask);
		break;
	}

	case IOV_GVAL(IOV_BMAC_SRCRC): {
		srom_rw_t *s = (srom_rw_t *)params;

		*ret_int_ptr = (uint8)srom_otp_write_region_crc(wlc_hw->sih, s->nbytes,
		                                                s->buf, FALSE);
		break;
	}

	case IOV_GVAL(IOV_BMAC_NVRAM_SOURCE): {
		uint32 macintmask;
		uint32 was_enabled;
		uint16 buffer[32];
		int i;

		/* intrs off */
		macintmask = wl_intrsoff(wlc_hw->wlc->wl);
		/* 0 for SROM; 1 for OTP; 2 for NVRAM */

		if (si_is_sprom_available(wlc_hw->sih)) {
			if (!(was_enabled = si_is_sprom_enabled(wlc_hw->sih)))
				si_sprom_enable(wlc_hw->sih, TRUE);

			err = srom_read(wlc_hw->sih, wlc_hw->sih->bustype,
			                (void *)(uintptr)wlc_hw->regs, wlc_hw->osh,
			                0, sizeof(buffer), buffer, FALSE);

			*ret_int_ptr = 2; /* NVRAM */

			if (!err)
				for (i = 0; i < (int)sizeof(buffer)/2; i++) {
					if ((buffer[i] != 0) && (buffer[i] != 0xffff)) {
						*ret_int_ptr = 0; /* SROM */
						break;
					}
				}
#ifdef BCMPCIEDEV
			if ((BUSTYPE(wlc_hw->sih->bustype) == SI_BUS) &&
			    ((CHIPID(wlc_hw->sih->chip) == BCM43602_CHIP_ID) ||
			     (CHIPID(wlc_hw->sih->chip) == BCM43462_CHIP_ID))) {
#else
			if (BUSTYPE(wlc_hw->sih->bustype) == PCI_BUS) {
#endif /* BCMPCIEDEV */
				/* If we still think its nvram try a test write */
				if (*ret_int_ptr == 2) {
					uint16 buffer1[32];
					int err1;

					err1 = srom_read(wlc_hw->sih, wlc_hw->sih->bustype,
					                (void *)(uintptr)wlc_hw->regs, wlc_hw->osh,
					                8, sizeof(unsigned short), buffer1, FALSE);
					srom_write_short(wlc_hw->sih, wlc_hw->sih->bustype,
					                 (void *)(uintptr)wlc_hw->regs, wlc_hw->osh,
					                 8, 0x1234);
					err = srom_read(wlc_hw->sih, wlc_hw->sih->bustype,
					                (void *)(uintptr)wlc_hw->regs, wlc_hw->osh,
					                8, sizeof(unsigned short), buffer, FALSE);
					if (!err1 && !err && buffer[0] == 0x1234) {
						*ret_int_ptr = 0; /* SROM */
						srom_write_short(wlc_hw->sih, wlc_hw->sih->bustype,
							(void *)(uintptr)wlc_hw->regs, wlc_hw->osh,
							8, buffer1[0]);
					}
				}
			}
			if (!was_enabled)
				si_sprom_enable(wlc_hw->sih, FALSE);
		} else
			*ret_int_ptr = 1; /* OTP */

		/* restore intrs */
		wl_intrsrestore(wlc_hw->wlc->wl, macintmask);
		break;
	}
#endif // endif

	/* XXX This feature should not be exposed to mfg but BMAC images doesn't have
	 * BCMINTERNAL defined. Need to build a special driver for 43231 chipid programmed
	 * with this feature compiled in if we want to program chipid via driver.
	 */

	case IOV_GVAL(IOV_BMAC_CUSTOMVAR1): {
		char *var;

		if ((var = getvar(wlc_hw->vars, "customvar1")))
			*ret_int_ptr = (uint32)bcm_strtoul(var, NULL, 0);
		else
			*ret_int_ptr = 0;

		break;
	}
	case IOV_SVAL(IOV_BMAC_GENERIC_DLOAD): {
		wl_dload_data_t *dload_ptr, dload_data;
		uint8 *bufptr;
		uint32 total_len;
		uint actual_data_offset;
		actual_data_offset = OFFSETOF(wl_dload_data_t, data);
		memcpy(&dload_data, (wl_dload_data_t *)arg, sizeof(wl_dload_data_t));
		total_len = dload_data.len + actual_data_offset;
		if ((bufptr = MALLOC(wlc_hw->osh, total_len)) == NULL) {
			err = BCME_NOMEM;
			break;
		}
		memcpy(bufptr, (uint8 *)arg, total_len);
		dload_ptr = (wl_dload_data_t *)bufptr;
		if (((dload_ptr->flag & DLOAD_FLAG_VER_MASK) >> DLOAD_FLAG_VER_SHIFT)
		    != DLOAD_HANDLER_VER) {
			err =  BCME_ERROR;
			MFREE(wlc_hw->osh, bufptr, total_len);
			break;
		}
		switch (dload_ptr->dload_type)	{
#ifdef BCMUCDOWNLOAD
		case DL_TYPE_UCODE:
			if (wlc_hw->wlc->is_initvalsdloaded != TRUE)
				wlc_process_ucodeparts(wlc_hw->wlc, dload_ptr->data);
			break;
#endif /* BCMUCDOWNLOAD */
		default:
			err = BCME_UNSUPPORTED;
			break;
		}
		MFREE(wlc_hw->osh, bufptr, total_len);
		break;
	}
	case IOV_GVAL(IOV_BMAC_UCDLOAD_STATUS):
		*ret_int_ptr = (int32) wlc_hw->wlc->is_initvalsdloaded;
		break;
	case IOV_GVAL(IOV_BMAC_UC_CHUNK_LEN):
		*ret_int_ptr = DL_MAX_CHUNK_LEN;
		break;

	case IOV_GVAL(IOV_BMAC_NOISE_METRIC):
		*ret_int_ptr = (int32)wlc_hw->noise_metric;
		break;
	case IOV_SVAL(IOV_BMAC_NOISE_METRIC):

		if ((uint16)int_val > NOISE_MEASURE_KNOISE) {
			err = BCME_UNSUPPORTED;
			break;
		}

		wlc_hw->noise_metric = (uint16)int_val;

		if ((wlc_hw->noise_metric & NOISE_MEASURE_KNOISE) == NOISE_MEASURE_KNOISE)
			wlc_bmac_mhf(wlc_hw, MHF3, MHF3_KNOISE, MHF3_KNOISE, WLC_BAND_ALL);
		else
			wlc_bmac_mhf(wlc_hw, MHF3, MHF3_KNOISE, 0, WLC_BAND_ALL);

		break;

	case IOV_GVAL(IOV_BMAC_AVIODCNT):
		*ret_int_ptr = wlc_bmac_dma_avoidance_cnt(wlc_hw);
		break;

#ifdef BCMDBG
	case IOV_SVAL(IOV_BMAC_FILT_WAR):
		wlc_phy_set_filt_war(wlc_hw->band->pi, bool_val);
		break;

	case IOV_GVAL(IOV_BMAC_FILT_WAR):
		*ret_int_ptr = wlc_phy_get_filt_war(wlc_hw->band->pi);
		break;
#endif /* BCMDBG */

#if defined(WLTEST)
	case IOV_SVAL(IOV_BMAC_SUSPEND_MAC):
	{
		wlc_info_t *wlc = wlc_hw->wlc;
		bool suspend = TRUE;

		err = BCME_OK;

		if ((wlc->bmac_suspend_timer) && (wlc->is_bmac_suspend_timer_active)) {
			/* stop timer in case it's already active */
			wl_del_timer(wlc->wl, wlc->bmac_suspend_timer);
			wlc->is_bmac_suspend_timer_active = FALSE;

			if (int_val == 0) {
				/* MAC already suspended and timer will be cancelled immediately */
				wlc_bmac_enable_mac(wlc_hw);
			}
			else {
				/* MAC already suspended - timer will restart with new value */
				suspend = FALSE;
			}
		}
		if (int_val > 0) {
			if (wlc->bmac_suspend_timer == NULL) {
				wlc->bmac_suspend_timer = wl_init_timer(wlc->wl,
				     wlc_bmac_suspend_timeout, wlc, "bmac_suspend");
			}

			if (wlc->bmac_suspend_timer != NULL) {
				if (suspend) {
					wlc_bmac_suspend_mac_and_wait(wlc_hw);
				}

				wl_add_timer(wlc->wl, wlc->bmac_suspend_timer, int_val, FALSE);

				wlc->is_bmac_suspend_timer_active = TRUE;
			}
			else {
				err = BCME_ERROR;
			}
		}
		break;
	}
#endif // endif
#if defined(BCMDBG) || defined(__NetBSD__) || defined(MACOSX)
	case IOV_SVAL(IOV_BMAC_RCVLAZY):
		/* Fix up the disable value if needed */
		if (int_val == 0) {
			int_val = IRL_DISABLE;
		}
		wlc_hw->intrcvlazy = (uint)int_val;
		if (wlc_hw->up) {
			wlc_bmac_rcvlazy_update(wlc_hw, wlc_hw->intrcvlazy);
		}
		break;

	case IOV_GVAL(IOV_BMAC_RCVLAZY):
		*ret_int_ptr = (int)wlc_hw->intrcvlazy;
		break;
#endif /* BCMDBG || __NetBSD__ || MACOSX */

	case IOV_SVAL(IOV_BMAC_BTSWITCH):
		if ((int_val != OFF) && (int_val != ON) && (int_val != AUTO)) {
			return BCME_RANGE;
		}

		if (WLCISACPHY(wlc_hw->band) && !(wlc_hw->up)) {
			err = BCME_NOTUP;
		} else {
			err = wlc_bmac_set_btswitch(wlc_hw, (int8)int_val);
		}

		break;

	case IOV_GVAL(IOV_BMAC_BTSWITCH):
		if (!((((CHIPID(wlc_hw->sih->chip) == BCM4331_CHIP_ID) ||
		       (CHIPID(wlc_hw->sih->chip) == BCM43431_CHIP_ID)) &&
		      ((wlc_hw->sih->boardtype == BCM94331X28) ||
		       (wlc_hw->sih->boardtype == BCM94331X28B) ||
		       (wlc_hw->sih->boardtype == BCM94331CS_SSID) ||
		       (wlc_hw->sih->boardtype == BCM94331X29B) ||
		       (wlc_hw->sih->boardtype == BCM94331X29D))) ||
		      WLCISACPHY(wlc_hw->band))) {
			err = BCME_UNSUPPORTED;
			break;
		}

		if (WLCISACPHY(wlc_hw->band)) {
			if (!(wlc_hw->up)) {
				err = BCME_NOTUP;
				break;
			}

			/* read the phyreg to find the state of bt/wlan ovrd */
			wlc_hw->btswitch_ovrd_state =
			        wlc_phy_get_femctrl_bt_wlan_ovrd(wlc_hw->band->pi);
		}

		*ret_int_ptr = 	wlc_hw->btswitch_ovrd_state;
		break;

#ifdef BCMDBG
	case IOV_GVAL(IOV_BMAC_PCIESSID):
		*ret_int_ptr = si_pcie_get_ssid(wlc_hw->sih);
		break;

	case IOV_GVAL(IOV_BMAC_PCIEBAR0):
		*ret_int_ptr = si_pcie_get_bar0(wlc_hw->sih);
		break;
#endif /* BCMDBG */

	case IOV_SVAL(IOV_BMAC_4360_PCIE2_WAR):
		wlc_hw->vcoFreq_4360_pcie2_war = (uint)int_val;
		break;

	case IOV_GVAL(IOV_BMAC_4360_PCIE2_WAR):
		*ret_int_ptr = (int)wlc_hw->vcoFreq_4360_pcie2_war;
		break;

#if defined(SAVERESTORE) && defined(BCMDBG_SR)
	case IOV_GVAL(IOV_BMAC_SR_VERIFY): {
		struct bcmstrbuf b;
		bcm_binit(&b, (char *)arg, len);

		if (SR_ENAB())
			wlc_bmac_sr_verify(wlc_hw, &b);
		break;
	}
#endif // endif
	default:
		WL_ERROR(("%s(): undefined BMAC IOVAR: %d\n", __FUNCTION__, actionid));
		err = BCME_NOTFOUND;
		break;

	}

	return err;

}

int
wlc_bmac_dispatch_iov(wlc_hw_info_t *wlc_hw, uint16 tid, uint32 aid, uint16 type,
	void *p, uint plen, void *a, int alen, int vsize)
{
	return wlc_iocv_dispatch_iov(wlc_hw->iocvi, tid, aid, p, plen, a, alen, vsize);
}

int
wlc_bmac_dispatch_ioc(wlc_hw_info_t *wlc_hw, uint16 tid, uint16 cid, uint16 type,
	void *a, uint alen, bool *ta)
{
	return wlc_iocv_dispatch_ioc(wlc_hw->iocvi, tid, cid, a, alen, ta);
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(BCMDBG_PHYDUMP)
#endif /* BCMDBG || BCMDBG_DUMP || BCMDBG_PHYDUMP */

#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(WLTEST) || \
	defined(BCMDBG_PHYDUMP)
/* register a dump name/callback in bmac */
static int
wlc_bmac_add_dump_fn(wlc_hw_info_t *wlc_hw, char *name, bmac_dump_fn_t fn, const void *ctx)
{
	return wlc_dump_reg_add_fn(wlc_hw->dump, name, (wlc_dump_reg_fn_t)fn, ctx);
}
#endif // endif

#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(BCMDBG_PHYDUMP)
/* invoke callback for a dump name */
static int
wlc_bmac_dbg_dump(wlc_hw_info_t *wlc_hw, char *name, struct bcmstrbuf *b)
{
	return wlc_dump_reg_invoke_fn(wlc_hw->dump, name, b);
}

/* lookup a dump name in bmac and execute it if found */
static int
wlc_bmac_dump_bmac(wlc_hw_info_t *wlc_hw, char *name, struct bcmstrbuf *b)
{
	int ret;
	bool ta_ok = FALSE;

	ret = wlc_bmac_dbg_dump(wlc_hw, name, b);
	ASSERT(wlc_bmac_taclear(wlc_hw, ta_ok) || !ta_ok);

	BCM_REFERENCE(ta_ok);

	return ret;
}

/* lookup a dump name in phy and execute it if found */
static int
wlc_bmac_dump_phy(wlc_hw_info_t *wlc_hw, char *name, struct bcmstrbuf *b)
{
	int ret = BCME_UNSUPPORTED;
	bool ta_ok = FALSE;

#if defined(DBG_PHY_IOV)
	if (!strcmp(name, "radioreg") || !strcmp(name, "phyreg") ||
	    !strcmp(name, "phytbl") || !strcmp(name, "phytbl2")) {
		ret = phy_dbg_dump((phy_info_t *)wlc_hw->band->pi, name, b);
		ta_ok = TRUE;
	}
	else
#endif // endif
	{
		bool single_phy, a_only;
		single_phy = (wlc_hw->bandstate[0]->pi == wlc_hw->bandstate[1]->pi) ||
			(wlc_hw->bandstate[1]->pi == NULL);

		a_only = (wlc_hw->bandstate[0]->pi == NULL);

		if (wlc_hw->bandstate[0]->pi)
			ret = phy_dbg_dump((phy_info_t *)wlc_hw->bandstate[0]->pi, name, b);
		if (!single_phy || a_only)
			ret = phy_dbg_dump((phy_info_t *)wlc_hw->bandstate[1]->pi, name, b);
	}

	ASSERT(wlc_bmac_taclear(wlc_hw, ta_ok) || !ta_ok);
	BCM_REFERENCE(ta_ok);
	return ret;
}
#endif /* BCMDBG || BCMDBG_DUMP || defined(BCMDBG_PHYDUMP) */

/* register bmac/si dump names */
static int
wlc_bmac_register_dumps(wlc_hw_info_t *wlc_hw)
{
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	wlc_bmac_add_dump_fn(wlc_hw, "btc", (bmac_dump_fn_t)wlc_bmac_btc_dump, (void *)wlc_hw);
	wlc_bmac_add_dump_fn(wlc_hw, "bmc", (bmac_dump_fn_t)wlc_bmac_bmc_dump, (void *)wlc_hw);
#endif /* BCMDBG || BCMDBG_DUMP */

#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(BCMDBG_PHYDUMP)
	wlc_bmac_add_dump_fn(wlc_hw, "macsuspend", (bmac_dump_fn_t)wlc_bmac_suspend_dump,
		(void *)wlc_hw);
#endif /* BCMDBG || BCMDBG_DUMP || BCMDBG_PHYDUMP */

#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(WLTEST)
	wlc_bmac_add_dump_fn(wlc_hw, "pcieinfo", (bmac_dump_fn_t)si_dump_pcieinfo,
		(const void *)wlc_hw->sih);
#endif /* BCMDBG || BCMDBG_DUMP || WLTEST */

	return BCME_OK;
}

int
wlc_bmac_dump(wlc_hw_info_t *wlc_hw, char *name, struct bcmstrbuf *b)
{
	int ret = BCME_UNSUPPORTED;

#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(BCMDBG_PHYDUMP)
	/* dump if 'name' is a bmac/si dump */
	ret = wlc_bmac_dump_bmac(wlc_hw, name, b);
	if (ret == BCME_NOTFOUND)
		/* dump if 'name' is a phy dump */
		ret = wlc_bmac_dump_phy(wlc_hw, name, b);
#endif /* BCMDBG || BCMDBG_DUMP || defined(BCMDBG_PHYDUMP) */

	return ret;
}

#if (defined(BCMNVRAMR) || defined(BCMNVRAMW)) && (defined(WLTEST) && \
	!defined(WLTEST_DISABLED))
static int
wlc_bmac_cissource(wlc_hw_info_t *wlc_hw)
{
	int ret = 0;

	switch (si_cis_source(wlc_hw->sih)) {
	case CIS_OTP:
		ret = WLC_CIS_OTP;
		break;
	case CIS_SROM:
		ret = WLC_CIS_SROM;
		break;
	case CIS_DEFAULT:
		ret = WLC_CIS_DEFAULT;
		break;
	default:
		ret = BCME_ERROR;
		break;
	}

	return ret;
}

#endif // endif

#if (defined(BCMNVRAMR) || defined(BCMNVRAMW)) && defined(WLTEST)
#ifdef BCMNVRAMW
int
wlc_bmac_ciswrite(wlc_hw_info_t *wlc_hw, cis_rw_t *cis, uint16 *tbuf, int len)
{
	int err = 0;

	WL_TRACE(("%s\n", __FUNCTION__));

	if (len < (int)cis->nbytes)
		return BCME_BUFTOOSHORT;

	switch (si_cis_source(wlc_hw->sih)) {
	case CIS_OTP: {
		uint32 macintmask;
		int region;
		macintmask = wl_intrsoff(wlc_hw->wlc->wl);

		if ((CHIPID(wlc_hw->sih->chip) == BCM4319_CHIP_ID) ||
			(CHIPID(wlc_hw->sih->chip) == BCM4322_CHIP_ID) ||
			(CHIPID(wlc_hw->sih->chip) == BCM43231_CHIP_ID) ||
			(CHIPID(wlc_hw->sih->chip) == BCM43234_CHIP_ID) ||
			(CHIPID(wlc_hw->sih->chip) == BCM43235_CHIP_ID) ||
			(CHIPID(wlc_hw->sih->chip) == BCM43236_CHIP_ID) ||
			(CHIPID(wlc_hw->sih->chip) == BCM43242_CHIP_ID) ||
			(CHIPID(wlc_hw->sih->chip) == BCM43243_CHIP_ID) ||
			((CHIPID(wlc_hw->sih->chip) == BCM43143_CHIP_ID) &&
			(!CST43143_CHIPMODE_SDIOD(wlc_hw->sih->chipst))) ||
			(CHIPID(wlc_hw->sih->chip) == BCM43238_CHIP_ID) ||
			(CHIPID(wlc_hw->sih->chip) == BCM4360_CHIP_ID) ||
			(CHIPID(wlc_hw->sih->chip) == BCM43460_CHIP_ID) ||
			CHIPID(wlc_hw->sih->chip) == BCM43602_CHIP_ID ||
			(CHIPID(wlc_hw->sih->chip) == BCM43526_CHIP_ID) ||
			(CHIPID(wlc_hw->sih->chip) == BCM4352_CHIP_ID) ||
			((CHIPID(wlc_hw->sih->chip) == BCM43340_CHIP_ID) &&
			(CST4334_CHIPMODE_HSIC(wlc_hw->sih->chipst))) ||
			((CHIPID(wlc_hw->sih->chip) == BCM43341_CHIP_ID) &&
			(CST4334_CHIPMODE_HSIC(wlc_hw->sih->chipst))) ||
			(BCM4350_CHIP(wlc_hw->sih->chip) &&
			(wlc_hw->sih->chipst &
			(CST4350_HSIC20D_MODE | CST4350_USB20D_MODE | CST4350_USB30D_MODE))) ||
			((wlc_hw->sih->chip == BCM4345_CHIP_ID) &&
			((CST4345_CHIPMODE_HSIC(wlc_hw->sih->chipst)) ||
			CST4345_CHIPMODE_USB20D(wlc_hw->sih->chipst))) ||
			0)
			region = OTP_SW_RGN;
		else
			region = OTP_HW_RGN;

		err = otp_write_region(wlc_hw->sih, region, tbuf, cis->nbytes / 2,
			cis->flags);

		wl_intrsrestore(wlc_hw->wlc->wl, macintmask);
		break;

	}

	case CIS_SROM: {
		bool was_enabled;

		if (!(was_enabled = si_is_sprom_enabled(wlc_hw->sih)))
			si_sprom_enable(wlc_hw->sih, TRUE);
		if (srom_write(wlc_hw->sih, wlc_hw->sih->bustype,
			(void *)(uintptr)wlc_hw->regs, wlc_hw->osh,
			cis->byteoff, cis->nbytes, tbuf))
			err = BCME_ERROR;
		if (!was_enabled)
			si_sprom_enable(wlc_hw->sih, FALSE);
		break;
	}

	case CIS_DEFAULT:
	default:
		err = BCME_NOTFOUND;
		break;
	}

	return err;
}
#endif /* def BCMNVRAMW */

int
wlc_bmac_cisdump(wlc_hw_info_t *wlc_hw, cis_rw_t *cis, uint16 *tbuf, int len)
{
	int err = 0;
	uint32 macintmask;

	WL_TRACE(("%s\n", __FUNCTION__));

	macintmask = wl_intrsoff(wlc_hw->wlc->wl);
	cis->source = WLC_CIS_OTP;
	cis->byteoff = 0;

	switch (si_cis_source(wlc_hw->sih)) {

	case CIS_SROM: {
		bool was_enabled;

		cis->source = WLC_CIS_SROM;
		cis->byteoff = 0;
		cis->nbytes = cis->nbytes ? ROUNDUP(cis->nbytes, 2) : SROM_MAX;
		if (len < (int)cis->nbytes) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		if (!(was_enabled = si_is_sprom_enabled(wlc_hw->sih)))
			si_sprom_enable(wlc_hw->sih, TRUE);
		if (srom_read(wlc_hw->sih, wlc_hw->sih->bustype,
			(void *)(uintptr)wlc_hw->regs, wlc_hw->osh,
			0, cis->nbytes, tbuf, FALSE)) {
			err = BCME_ERROR;
		}
		if (!was_enabled)
			si_sprom_enable(wlc_hw->sih, FALSE);

		break;
	}

	case CIS_OTP: {
		int region;

		cis->nbytes = len;
		if ((CHIPID(wlc_hw->sih->chip) == BCM4319_CHIP_ID) ||
			(CHIPID(wlc_hw->sih->chip) == BCM4322_CHIP_ID) ||
			(CHIPID(wlc_hw->sih->chip) == BCM43231_CHIP_ID) ||
			(CHIPID(wlc_hw->sih->chip) == BCM43234_CHIP_ID) ||
			(CHIPID(wlc_hw->sih->chip) == BCM43235_CHIP_ID) ||
			((CHIPID(wlc_hw->sih->chip) == BCM43143_CHIP_ID) &&
			(!CST43143_CHIPMODE_SDIOD(wlc_hw->sih->chipst))) ||
			(CHIPID(wlc_hw->sih->chip) == BCM43242_CHIP_ID) ||
			(CHIPID(wlc_hw->sih->chip) == BCM43243_CHIP_ID) ||
			((CHIPID(wlc_hw->sih->chip) == BCM43340_CHIP_ID) &&
			(CST4334_CHIPMODE_HSIC(wlc_hw->sih->chipst))) ||
			((CHIPID(wlc_hw->sih->chip) == BCM43341_CHIP_ID) &&
			(CST4334_CHIPMODE_HSIC(wlc_hw->sih->chipst))) ||
			(CHIPID(wlc_hw->sih->chip) == BCM43236_CHIP_ID) ||
			(CHIPID(wlc_hw->sih->chip) == BCM43238_CHIP_ID) ||
			(CHIPID(wlc_hw->sih->chip) == BCM4360_CHIP_ID) ||
			(CHIPID(wlc_hw->sih->chip) == BCM43460_CHIP_ID) ||
			CHIPID(wlc_hw->sih->chip) == BCM43602_CHIP_ID ||
			(CHIPID(wlc_hw->sih->chip) == BCM4352_CHIP_ID) ||
			(CHIPID(wlc_hw->sih->chip) == BCM43526_CHIP_ID) ||
			((wlc_hw->sih->chip == BCM4345_CHIP_ID) &&
			((CST4345_CHIPMODE_HSIC(wlc_hw->sih->chipst)) ||
			CST4345_CHIPMODE_USB20D(wlc_hw->sih->chipst))) ||
			(BCM4350_CHIP(wlc_hw->sih->chip) &&
			(wlc_hw->sih->chipst &
			(CST4350_HSIC20D_MODE | CST4350_USB20D_MODE | CST4350_USB30D_MODE))) ||
			0)
			region = OTP_SW_RGN;
		else
			region = OTP_HW_RGN;

		err = otp_read_region(wlc_hw->sih, region, tbuf, &cis->nbytes);
		cis->nbytes *= 2;

		/* Not programmed is ok */
		if (err == BCME_NOTFOUND)
			err = 0;

		break;
	}

	case CIS_DEFAULT:
	case BCME_NOTFOUND:
	default:
		err = BCME_NOTFOUND;
		cis->source = 0;
		cis->byteoff = 0;
		cis->nbytes = 0;
		break;
	}

	wl_intrsrestore(wlc_hw->wlc->wl, macintmask);

	return err;
}
#endif // endif

#if (defined(WLTEST) || defined(WLPKTENG))
int
wlc_bmac_pkteng(wlc_hw_info_t *wlc_hw, wl_pkteng_t *pkteng, void* p)
{
	wlc_phy_t *pi = wlc_hw->band->pi;
	uint32 cmd;
	bool is_sync;
	uint16 pkteng_mode;
	uint err = BCME_OK;

#if defined(WLP2P_UCODE)
	if (DL_P2P_UC(wlc_hw) && (CHIPID(wlc_hw->sih->chip) != BCM4360_CHIP_ID) &&
		(CHIPID(wlc_hw->sih->chip) != BCM4335_CHIP_ID) &&
		(CHIPID(wlc_hw->sih->chip) != BCM4345_CHIP_ID) &&
		(CHIPID(wlc_hw->sih->chip) != BCM43602_CHIP_ID) &&
#ifdef UNRELEASEDCHIP
		!BCM4349_CHIP(wlc_hw->sih->chip) &&
#endif /* UNRELEASEDCHIP */
		!BCM4350_CHIP(wlc_hw->sih->chip) &&
		!BCM4365_CHIP(wlc_hw->sih->chip) &&
		1) {
		WL_ERROR(("p2p-ucode does not support pkteng\n"));
		if (p) PKTFREE(wlc_hw->osh, p, TRUE);
		return BCME_UNSUPPORTED;
	}
#endif /* WLP2P_UCODE */

	cmd = pkteng->flags & WL_PKTENG_PER_MASK;
	is_sync = (pkteng->flags & WL_PKTENG_SYNCHRONOUS) ? TRUE : FALSE;

	switch (cmd) {
	case WL_PKTENG_PER_RX_START:
	case WL_PKTENG_PER_RX_WITH_ACK_START:
	{
#if defined(WLC_HIGH) && defined(WLCNT)
		uint32 pktengrxducast_start = 0;
		wlc_pub_t	*pub = wlc_hw->wlc->pub;
#endif /* WLC_HIGH */
		/* Reset the counters */
		wlc_bmac_write_shm(wlc_hw, M_PKTENG_FRMCNT_LO, 0);
		wlc_bmac_write_shm(wlc_hw, M_PKTENG_FRMCNT_HI, 0);
		wlc_phy_hold_upd(pi, PHY_HOLD_FOR_PKT_ENG, TRUE);
		wlc_bmac_mhf(wlc_hw, MHF3, MHF3_PKTENG_PROMISC,
			MHF3_PKTENG_PROMISC, WLC_BAND_ALL);

		if (is_sync) {
#if defined(WLC_HIGH) && defined(WLCNT)
			/* get counter value before start of pkt engine */
			wlc_ctrupd(wlc_hw->wlc, MCSTOFF_RXGOODUCAST);
			pktengrxducast_start = WLCNTVAL(MCSTVAR(pub, pktengrxducast));
#else
			/* BMAC_NOTE: need to split wlc_ctrupd before supporting this in bmac */
			ASSERT(0);
#endif /* WLC_HIGH */
		}

		pkteng_mode = (cmd == WL_PKTENG_PER_RX_START) ?
			M_PKTENG_MODE_RX: M_PKTENG_MODE_RX_WITH_ACK;

		wlc_bmac_write_shm(wlc_hw, M_PKTENG_CTRL, pkteng_mode);

		/* set RA match reg with dest addr */
		wlc_bmac_set_match_mac(wlc_hw, &pkteng->dest);

#if defined(WLC_HIGH) && defined(WLCNT)
		/* wait for counter for synchronous receive with a maximum total delay */
		if (is_sync) {
			/* loop delay in msec */
			uint32 delay_msec = 1;
			/* avoid calculation in loop */
			uint32 delay_usec = delay_msec * 1000;
			uint32 total_delay = 0;
			uint32 delta;
			do {
				OSL_DELAY(delay_usec);
				total_delay += delay_msec;
				wlc_ctrupd(wlc_hw->wlc, MCSTOFF_RXGOODUCAST);
				if (WLCNTVAL(MCSTVAR(pub, pktengrxducast))
					> pktengrxducast_start) {
					delta = WLCNTVAL(MCSTVAR(pub, pktengrxducast)) -
						pktengrxducast_start;
				} else {
					/* counter overflow */
					delta = (~pktengrxducast_start + 1) +
						WLCNTVAL(MCSTVAR(pub, pktengrxducast));
				}
			} while (delta < pkteng->nframes && total_delay < pkteng->delay);

			wlc_phy_hold_upd(pi, PHY_HOLD_FOR_PKT_ENG, FALSE);
			/* implicit rx stop after synchronous receive */
			wlc_bmac_write_shm(wlc_hw, M_PKTENG_CTRL, 0);
			wlc_bmac_mhf(wlc_hw, MHF3, MHF3_PKTENG_PROMISC,
			0, WLC_BAND_ALL);
			wlc_bmac_set_match_mac(wlc_hw, &wlc_hw->etheraddr);
		}
#endif /* WLC_HIGH */

		break;
	}

	case WL_PKTENG_PER_RX_STOP:
		WL_INFORM(("Pkteng RX Stop Called\n"));
		wlc_bmac_write_shm(wlc_hw, M_PKTENG_CTRL, 0);
		wlc_bmac_mhf(wlc_hw, MHF3, MHF3_PKTENG_PROMISC,
			0, WLC_BAND_ALL);
		wlc_phy_hold_upd(pi, PHY_HOLD_FOR_PKT_ENG, FALSE);
		/* Restore match address register */
		wlc_bmac_set_match_mac(wlc_hw, &wlc_hw->etheraddr);

		break;

	case WL_PKTENG_PER_TX_START:
	case WL_PKTENG_PER_TX_WITH_ACK_START:
	case WL_MUPKTENG_PER_TX_START:
	{
		uint16 val = M_PKTENG_MODE_TX;

		WL_ERROR(("Pkteng TX Start Called\n"));

		if (cmd == WL_MUPKTENG_PER_TX_START)
			ASSERT(p == NULL);
		else
			ASSERT(p != NULL);

		if ((pkteng->delay < 15) || (pkteng->delay > 1000)) {
			WL_ERROR(("delay out of range, freeing the packet\n"));
			if (cmd != WL_MUPKTENG_PER_TX_START)
				PKTFREE(wlc_hw->osh, p, TRUE);
			err = BCME_RANGE;
			break;
		}

		wlc_phy_hold_upd(pi, PHY_HOLD_FOR_PKT_ENG, TRUE);
		wlc_bmac_suspend_mac_and_wait(wlc_hw);

		if (WLCISSSLPNPHY(wlc_hw->band) || WLCISLCNPHY(wlc_hw->band)) {
			wlc_phy_set_deaf(pi, TRUE);
		} else {
			/*
			 * mute the rx side for the regular TX.
			 * tx_with_ack mode makes the ucode update rxdfrmucastmbss count
			 */
			if ((cmd == WL_PKTENG_PER_TX_START) || (cmd == WL_MUPKTENG_PER_TX_START)) {
				wlc_phy_set_deaf(pi, TRUE);
			} else {
				wlc_phy_clear_deaf(pi, TRUE);
			}
		}

		/* set nframes */
		if (pkteng->nframes) {
			wlc_bmac_write_shm(wlc_hw, M_PKTENG_FRMCNT_LO,
				(pkteng->nframes & 0xffff));
			wlc_bmac_write_shm(wlc_hw, M_PKTENG_FRMCNT_HI,
				((pkteng->nframes>>16) & 0xffff));
			val |= M_PKTENG_FRMCNT_VLD;
		}

		wlc_bmac_write_shm(wlc_hw, M_PKTENG_CTRL, val);

		/* we write to M_MFGTEST_IFS the IFS required in 1/8us factor */
		/* 10 : for factoring difference b/w Tx.crs and energy in air */
		/* 44 : amount of time spent after TX_RRSP to frame start */
		/* IFS */
		wlc_bmac_write_shm(wlc_hw, M_PKTENG_IFS, (pkteng->delay - 10)*8 - 44);
		if (is_sync)
		  wlc_bmac_mctrl(wlc_hw, MCTL_DISCARD_TXSTATUS, 1 << 29);
		wlc_bmac_enable_mac(wlc_hw);

		if (p != NULL) {
			/* Do the low part of wlc_txfifo() */
			wlc_bmac_txfifo(wlc_hw, TX_DATA_FIFO, p, TRUE, INVALIDFID, 1);
		}
		/* wait for counter for synchronous transmit */
		if (is_sync) {
			int i;
			do {
				OSL_DELAY(1000);
				i = wlc_bmac_read_shm(wlc_hw, M_PKTENG_CTRL);
			} while (i & M_PKTENG_MODE_TX);

			wlc_bmac_suspend_mac_and_wait(wlc_hw);
			wlc_bmac_mctrl(wlc_hw, MCTL_DISCARD_TXSTATUS, 0);
			wlc_bmac_enable_mac(wlc_hw);
			/* implicit tx stop after synchronous transmit */
			wlc_phy_clear_deaf(pi, (bool)1);
			wlc_phy_hold_upd(pi, PHY_HOLD_FOR_PKT_ENG, FALSE);
			p = wlc_bmac_dma_getnexttxp(wlc_hw->wlc, TX_DATA_FIFO,
				HNDDMA_RANGE_TRANSMITTED);
			ASSERT(p != NULL);
			PKTFREE(wlc_hw->osh, p, TRUE);
		}

		break;
	}

	case WL_PKTENG_PER_TX_STOP:
	case WL_MUPKTENG_PER_TX_STOP:
	{
		int status;

		ASSERT(p == NULL);

		WL_INFORM(("Pkteng TX Stop Called\n"));

		/* Check pkteng state */
		status = wlc_bmac_read_shm(wlc_hw, M_PKTENG_CTRL);
		if (status & M_PKTENG_MODE_TX) {
			uint16 val = M_PKTENG_MODE_TX;

			/* Still running
			 * Stop cleanly by setting frame count
			 */
			wlc_bmac_suspend_mac_and_wait(wlc_hw);
			wlc_bmac_write_shm(wlc_hw, M_PKTENG_FRMCNT_LO, 1);
			wlc_bmac_write_shm(wlc_hw, M_PKTENG_FRMCNT_HI, 0);
			val |= M_PKTENG_FRMCNT_VLD;
			wlc_bmac_write_shm(wlc_hw, M_PKTENG_CTRL, val);
			wlc_bmac_enable_mac(wlc_hw);

			/* Wait for the pkteng to stop */
			do {
				OSL_DELAY(1000);
				status = wlc_bmac_read_shm(wlc_hw, M_PKTENG_CTRL);
			} while (status & M_PKTENG_MODE_TX);
		}

		/* Clean up */
		wlc_phy_clear_deaf(pi, (bool)1);
		wlc_phy_hold_upd(pi, PHY_HOLD_FOR_PKT_ENG, FALSE);
		break;
	}

	default:
		err = BCME_UNSUPPORTED;
	}

	return err;
}
#endif // endif

#ifdef WLC_HIGH
/**
 * Lower down relevant GPIOs like LED/BTC when going down w/o
 * doing PCI config cycles or touching interrupts
 */
void
wlc_gpio_fast_deinit(wlc_hw_info_t *wlc_hw)
{
	if ((wlc_hw == NULL) || (wlc_hw->sih == NULL))
		return;

	/* Only chips with internal bus or PCIE cores or certain PCI cores
	 * are able to switch cores w/o disabling interrupts
	 */
	if (!((BUSTYPE(wlc_hw->sih->bustype) == SI_BUS) ||
	      ((BUSTYPE(wlc_hw->sih->bustype) == PCI_BUS) &&
	       ((wlc_hw->sih->buscoretype == PCIE_CORE_ID) ||
	        (wlc_hw->sih->buscoretype == PCIE2_CORE_ID) ||
	        (wlc_hw->sih->buscorerev >= 13)))))
		return;

	WL_TRACE(("wl%d: %s\n", wlc_hw->unit, __FUNCTION__));

#ifdef WLLED
	if (wlc_hw->wlc->ledh)
		wlc_led_deinit(wlc_hw->wlc->ledh);
#endif // endif

	wlc_bmac_btc_gpio_disable(wlc_hw);

	return;
}
#endif /* WLC_HIGH */

#if defined(STA) && defined(BCMDBG)
static void
wlc_bmac_dma_lpbk(wlc_hw_info_t *wlc_hw, bool enable)
{
	if (BUSTYPE(wlc_hw->sih->bustype) != PCI_BUS ||
	    PIO_ENAB_HW(wlc_hw))
		return;

	if (enable) {
		wlc_bmac_suspend_mac_and_wait(wlc_hw);
		dma_fifoloopbackenable(wlc_hw->di[TX_DATA_FIFO]);
	} else {
		dma_txreset(wlc_hw->di[TX_DATA_FIFO]);
		wlc_upd_suspended_fifos_clear(wlc_hw, TX_DATA_FIFO);
		wlc_bmac_enable_mac(wlc_hw);
	}
}
#endif /* defined(STA) && defined(BCMDBG) */

bool
wlc_bmac_radio_hw(wlc_hw_info_t *wlc_hw, bool enable, bool skip_anacore)
{
	/* Do not access Phy registers if core is not up */
	if (si_iscoreup(wlc_hw->sih) == FALSE)
		return FALSE;

	if (enable) {
		if (PMUCTL_ENAB(wlc_hw->sih)) {
			AND_REG(wlc_hw->osh, &wlc_hw->regs->clk_ctl_st, ~CCS_FORCEHWREQOFF);
			si_pmu_radio_enable(wlc_hw->sih, TRUE);
		}

		/* need to skip for 5356 in case of radio_pwrsave feature. */
		if (!skip_anacore)
			phy_ana_switch((phy_info_t *)wlc_hw->band->pi, ON);
		phy_radio_switch((phy_info_t *)wlc_hw->band->pi, ON);

		/* resume d11 core */
		wlc_bmac_enable_mac(wlc_hw);
	}
	else {
		/* suspend d11 core */
		wlc_bmac_suspend_mac_and_wait(wlc_hw);

		phy_radio_switch((phy_info_t *)wlc_hw->band->pi, OFF);
		/* need to skip for 5356 in case of radio_pwrsave feature. */
		if (!skip_anacore)
			phy_ana_switch((phy_info_t *)wlc_hw->band->pi, OFF);

		if (PMUCTL_ENAB(wlc_hw->sih)) {
			si_pmu_radio_enable(wlc_hw->sih, FALSE);
			OR_REG(wlc_hw->osh, &wlc_hw->regs->clk_ctl_st, CCS_FORCEHWREQOFF);
		}
	}

	return TRUE;
}

void
wlc_bmac_minimal_radio_hw(wlc_hw_info_t *wlc_hw, bool enable)
{
	wlc_info_t *wlc = wlc_hw->wlc;
	if (D11REV_GE(wlc_hw->corerev, 13) && PMUCTL_ENAB(wlc_hw->sih)) {

		if (enable == TRUE) {
			AND_REG(wlc->osh, &wlc->regs->clk_ctl_st, ~CCS_FORCEHWREQOFF);
			si_pmu_radio_enable(wlc_hw->sih, TRUE);
		} else {
			si_pmu_radio_enable(wlc_hw->sih, FALSE);
			OR_REG(wlc->osh, &wlc->regs->clk_ctl_st, CCS_FORCEHWREQOFF);
		}
	}
}

bool
wlc_bmac_si_iscoreup(wlc_hw_info_t *wlc_hw)
{
	return si_iscoreup(wlc_hw->sih);
}

uint16
wlc_bmac_rate_shm_offset(wlc_hw_info_t *wlc_hw, uint8 rate)
{
	uint16 table_ptr;
	uint8 phy_rate, indx;

	/* get the phy specific rate encoding for the PLCP SIGNAL field */
	/* XXX4321 fixup needed ? */
	if (IS_OFDM(rate))
		table_ptr = M_RT_DIRMAP_A;
	else
		table_ptr = M_RT_DIRMAP_B;

	/* for a given rate, the LS-nibble of the PLCP SIGNAL field is
	 * the index into the rate table.
	 */
	phy_rate = rate_info[rate] & RATE_MASK;
	indx = phy_rate & 0xf;

	/* Find the SHM pointer to the rate table entry by looking in the
	 * Direct-map Table
	 */
	return (2*wlc_bmac_read_shm(wlc_hw, table_ptr + (indx * 2)));
}

void
wlc_bmac_stf_set_rateset_shm_offset(wlc_hw_info_t *wlc_hw, uint count, uint16 pos, uint16 mask,
wlc_stf_rs_shm_offset_t *stf_rs)
{
	uint16 idx;
	uint16 entry_ptr;
	uint16 val;
	uint8 rate;

	for (idx = 0; idx < count; idx++) {
		rate = stf_rs->rate[idx] & RATE_MASK;
		entry_ptr = wlc_bmac_rate_shm_offset(wlc_hw, rate);
		val = stf_rs->val[idx];
		if (D11REV_GE(wlc_hw->corerev, 40)) {
			val |= (wlc_bmac_read_shm(wlc_hw, (entry_ptr + pos)) & ~mask);
		}
		wlc_bmac_write_shm(wlc_hw, (entry_ptr + pos), val);
	}
}

#ifdef WLEXTLOG
#ifdef WLC_LOW_ONLY
void
wlc_bmac_extlog_cfg_set(wlc_hw_info_t *wlc_hw, wlc_extlog_cfg_t *cfg)
{
	wlc_extlog_info_t *extlog = (wlc_extlog_info_t *)wlc_hw->extlog;

	extlog->cfg.module = cfg->module;
	extlog->cfg.level = cfg->level;
	extlog->cfg.flag = cfg->flag;

	return;
}
#endif /* WLC_LOW_ONLY */
#endif /* WLEXTLOG */

#ifdef PHYCAL_CACHING
void
wlc_bmac_set_phycal_cache_flag(wlc_hw_info_t *wlc_hw, bool state)
{
	wlc_phy_cal_cache_set(wlc_hw->band->pi, state);
}

bool
wlc_bmac_get_phycal_cache_flag(wlc_hw_info_t *wlc_hw)
{
	return wlc_phy_cal_cache_get(wlc_hw->band->pi);
}
#endif /* PHYCAL_CACHING */

void
wlc_bmac_set_txpwr_percent(wlc_hw_info_t *wlc_hw, uint8 val)
{
	wlc_phy_txpwr_percent_set(wlc_hw->band->pi, val);
}

void
wlc_bmac_set_txpwr_degrade(wlc_hw_info_t *wlc_hw, uint8 val)
{
	wlc_phy_txpwr_degrade(wlc_hw->band->pi, val);
}

uint32
wlc_bmac_read_counter(wlc_hw_info_t* wlc_hw, uint baseaddr, int lo_off, int hi_off)
{
	uint16 high, tmp_high, low;

	ASSERT(baseaddr != (uint)~0);

	tmp_high = wlc_bmac_read_shm(wlc_hw, baseaddr + hi_off);
	low = wlc_bmac_read_shm(wlc_hw, baseaddr + lo_off);
	high = wlc_bmac_read_shm(wlc_hw, baseaddr + hi_off);
	if (high != tmp_high) {
		low = 0;	/* assume it zero */
	}
	return (high << 16) | low;
}

uint32
wlc_bmac_cca_read_counter(wlc_hw_info_t* wlc_hw, int lo_off, int hi_off)
{
	return wlc_bmac_read_counter(wlc_hw, wlc_hw->cca_shm_base, lo_off, hi_off);
}

int
wlc_bmac_cca_stats_read(wlc_hw_info_t *wlc_hw, cca_ucode_counts_t *cca_counts)
{
	uint32 tsf_h;
	int i;

	/* Read shmem */
	cca_counts->txdur = wlc_bmac_cca_read_counter(wlc_hw, M_CCA_TXDUR_L, M_CCA_TXDUR_H);
	cca_counts->ibss = wlc_bmac_cca_read_counter(wlc_hw, M_CCA_INBSS_L, M_CCA_INBSS_H);
	cca_counts->obss = wlc_bmac_cca_read_counter(wlc_hw, M_CCA_OBSS_L, M_CCA_OBSS_H);
	cca_counts->noctg = wlc_bmac_cca_read_counter(wlc_hw, M_CCA_NOCTG_L, M_CCA_NOCTG_H);
	cca_counts->nopkt = wlc_bmac_cca_read_counter(wlc_hw, M_CCA_NOPKT_L, M_CCA_NOPKT_H);
	cca_counts->PM = wlc_bmac_cca_read_counter(wlc_hw, M_MAC_DOZE_L, M_MAC_DOZE_H);
#ifdef ISID_STATS
	cca_counts->crsglitch = wlc_bmac_read_shm(wlc_hw, MACSTAT_ADDR(MCSTOFF_RXCRSGLITCH));
	cca_counts->badplcp = wlc_bmac_read_shm(wlc_hw, MACSTAT_ADDR(MCSTOFF_RXBADPLCP));
	cca_counts->bphy_crsglitch = wlc_bmac_read_shm(wlc_hw, MACSTAT_ADDR(MCSTOFF_BPHYGLITCH));
	cca_counts->bphy_badplcp = wlc_bmac_read_shm(wlc_hw, MACSTAT_ADDR(MCSTOFF_BPHY_BADPLCP));
#endif /* ISID_STATS */
	wlc_bmac_read_tsf(wlc_hw, &cca_counts->usecs, &tsf_h);

	for (i = 0; i < 8; i++) {
		int j = i*4;
		cca_counts->txnode[i] = wlc_bmac_cca_read_counter(wlc_hw,
			shm_addr(M_CCA_TXNODE_L, j), shm_addr(M_CCA_TXNODE_H, j));
		cca_counts->rxnode[i] = wlc_bmac_cca_read_counter(wlc_hw,
			shm_addr(M_CCA_RXNODE_L, j), shm_addr(M_CCA_RXNODE_H, j));
	}
	cca_counts->xxobss = wlc_bmac_cca_read_counter(wlc_hw,
		shm_addr(M_CCA_XXOBSS_L, 0), shm_addr(M_CCA_XXOBSS_H, 0));

	return 0;
}

int
wlc_bmac_obss_stats_read(wlc_hw_info_t *wlc_hw, wlc_bmac_obss_counts_t *obss_counts)
{
	uint32 tsf_h;
	int i;

#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(BCMDBG_PHYDUMP)
	bmac_suspend_stats_t* susp_stats = wlc_hw->suspend_stats;
	uint32 suspend_time = susp_stats->suspended;
	uint32 timenow = R_REG(wlc_hw->osh, &wlc_hw->regs->tsf_timerlow);

	if (susp_stats->suspend_start > susp_stats->suspend_end &&
	    timenow > susp_stats->suspend_start) {
		suspend_time += (timenow - susp_stats->suspend_start) / 100;
	}

	obss_counts->suspend = suspend_time;
	obss_counts->suspend_cnt = susp_stats->suspend_count;
#endif // endif

	/* Read shmem */
	obss_counts->txdur = wlc_bmac_cca_read_counter(wlc_hw, M_CCA_TXDUR_L, M_CCA_TXDUR_H);
	obss_counts->ibss = wlc_bmac_cca_read_counter(wlc_hw, M_CCA_INBSS_L, M_CCA_INBSS_H);
	obss_counts->obss = wlc_bmac_cca_read_counter(wlc_hw, M_CCA_OBSS_L, M_CCA_OBSS_H);
	obss_counts->noctg = wlc_bmac_cca_read_counter(wlc_hw, M_CCA_NOCTG_L, M_CCA_NOCTG_H);
	obss_counts->nopkt = wlc_bmac_cca_read_counter(wlc_hw, M_CCA_NOPKT_L, M_CCA_NOPKT_H);
	obss_counts->PM = wlc_bmac_cca_read_counter(wlc_hw, M_MAC_DOZE_L, M_MAC_DOZE_H);
	obss_counts->txopp = wlc_bmac_cca_read_counter(wlc_hw, M_CCA_TXOP_L, M_CCA_TXOP_H);
	obss_counts->slot_time_txop = R_REG(wlc_hw->osh, &wlc_hw->regs->u.d11regs.ifs_slot);
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	obss_counts->gdtxdur = wlc_bmac_cca_read_counter(wlc_hw, M_CCA_GDTXDUR_L, M_CCA_GDTXDUR_H);
	obss_counts->bdtxdur = wlc_bmac_cca_read_counter(wlc_hw, M_CCA_BDTXDUR_L, M_CCA_BDTXDUR_H);
#endif // endif
#ifdef ISID_STATS
	obss_counts->crsglitch = wlc_bmac_read_shm(wlc_hw, MACSTAT_ADDR(MCSTOFF_RXCRSGLITCH));
	obss_counts->badplcp = wlc_bmac_read_shm(wlc_hw, MACSTAT_ADDR(MCSTOFF_RXBADPLCP));
	obss_counts->bphy_crsglitch = wlc_bmac_read_shm(wlc_hw, MACSTAT_ADDR(MCSTOFF_BPHYGLITCH));
	obss_counts->bphy_badplcp = wlc_bmac_read_shm(wlc_hw, MACSTAT_ADDR(MCSTOFF_BPHY_BADPLCP));
#endif /* ISID_STATS */
	for (i = 0; i < 8; i++) {
		int j = i*4;
		obss_counts->txnode[i] = wlc_bmac_cca_read_counter(wlc_hw,
			shm_addr(M_CCA_TXNODE_L, j), shm_addr(M_CCA_TXNODE_H, j));
		obss_counts->rxnode[i] = wlc_bmac_cca_read_counter(wlc_hw,
			shm_addr(M_CCA_RXNODE_L, j), shm_addr(M_CCA_RXNODE_H, j));
	}
	obss_counts->xxobss = wlc_bmac_cca_read_counter(wlc_hw,
		shm_addr(M_CCA_XXOBSS_L, 0), shm_addr(M_CCA_XXOBSS_H, 0));

#if (defined(WL_PROT_OBSS) || defined(WL_OBSS_DYNBW))

	if (D11REV_GE(wlc_hw->corerev, 40)) {
		obss_counts->rxcrs_sec20 = wlc_bmac_read_counter(wlc_hw, wlc_hw->macstat1_shm_base,
			OFFSETOF(macstat1_t, rxsec20_durl), OFFSETOF(macstat1_t, rxsec20_durh));
		obss_counts->rxcrs_sec40 = wlc_bmac_read_counter(wlc_hw, wlc_hw->macstat1_shm_base,
			OFFSETOF(macstat1_t, rxsec40_durl), OFFSETOF(macstat1_t, rxsec40_durh));
		obss_counts->rxcrs_pri = wlc_bmac_read_counter(wlc_hw, wlc_hw->macstat1_shm_base,
			OFFSETOF(macstat1_t, rxpri_durl), OFFSETOF(macstat1_t, rxpri_durh));

		obss_counts->sec_rssi_hist_hi = wlc_bmac_read_shm(wlc_hw,
			wlc_hw->macstat1_shm_base + OFFSETOF(macstat1_t, rxsecrssi0));
		obss_counts->sec_rssi_hist_med = wlc_bmac_read_shm(wlc_hw,
			wlc_hw->macstat1_shm_base + OFFSETOF(macstat1_t, rxsecrssi1));
		obss_counts->sec_rssi_hist_low = wlc_bmac_read_shm(wlc_hw,
			wlc_hw->macstat1_shm_base + OFFSETOF(macstat1_t, rxsecrssi2));

		obss_counts->rxdrop20s = wlc_bmac_read_shm(wlc_hw,
			MACSTAT_ADDR(MCSTOFF_RXDROP20S));
		obss_counts->rx20s = wlc_bmac_read_shm(wlc_hw,
			wlc_hw->macstat1_shm_base + OFFSETOF(macstat1_t, rx20s));
	}
#endif /* (defined(WL_PROT_OBSS) || defined(WL_OBSS_DYNBW)) */
	wlc_bmac_read_tsf(wlc_hw, &obss_counts->usecs, &tsf_h);

	obss_counts->txfrm = wlc_bmac_read_shm(wlc_hw, MACSTAT_ADDR(MCSTOFF_TXFRAME));
	obss_counts->rxstrt = wlc_bmac_read_shm(wlc_hw, MACSTAT_ADDR(MCSTOFF_RXSTRT));
	obss_counts->rxglitch = wlc_bmac_read_shm(wlc_hw, MACSTAT_ADDR(MCSTOFF_RXCRSGLITCH));
	obss_counts->rxwifi = wlc_bmac_cca_read_counter(wlc_hw, M_CCA_WIFI_L, M_CCA_WIFI_H);
	obss_counts->edcrs = wlc_bmac_cca_read_counter(wlc_hw, M_CCA_EDCRS_L, M_CCA_EDCRS_H);
	return 0;
}

#ifdef WLCHANIM_US
int  wlc_bmaq_lq_stats_read(wlc_hw_info_t *wlc_hw, chanim_cnt_us_t *chanim_cnt_us)
{
	if (D11REV_GE(wlc_hw->corerev, 40)) {
		chanim_cnt_us->rxcrs_pri20 = wlc_bmac_read_counter(wlc_hw,
				wlc_hw->macstat1_shm_base, OFFSETOF(macstat1_t, rxpri_durl),
				OFFSETOF(macstat1_t, rxpri_durh));
		chanim_cnt_us->rxcrs_sec20 = wlc_bmac_read_counter(wlc_hw,
				wlc_hw->macstat1_shm_base, OFFSETOF(macstat1_t, rxsec20_durl),
				OFFSETOF(macstat1_t, rxsec20_durh));
		chanim_cnt_us->rxcrs_sec40 = wlc_bmac_read_counter(wlc_hw,
				wlc_hw->macstat1_shm_base, OFFSETOF(macstat1_t, rxsec40_durl),
				OFFSETOF(macstat1_t, rxsec40_durh));
	}
	return 0;
}
#endif /* WLCHANIM_US */

void
wlc_bmac_antsel_set(wlc_hw_info_t *wlc_hw, uint32 antsel_avail)
{
	wlc_hw->antsel_avail = antsel_avail;
}

#ifndef BCM_OL_DEV
/* BTC stuff BEGIN */
/**
 * Copy btc_paramXX and btc_flag information from NVRAM to wlc_info_t structure, so that it can be
 * used during INIT, when NVRAM has already been released (reclaimed).
 */
static int
BCMATTACHFN(wlc_bmac_btc_param_attach)(wlc_info_t *wlc)
{
	wlc_hw_info_t *wlc_hw = wlc->hw;
	uint16 indx;
	char   buf[15];
	uint16 num_params = 0;

#if !defined(WLTEST)
	if (D11REV_GE(wlc_hw->corerev, 15)) {
		if ((wlc_hw->btc->wlc_btc_params = (uint16 *)
			MALLOC(wlc->osh, M_BTCX_BACKUP_SIZE*sizeof(uint16))) == NULL) {
			WL_ERROR(
			("wlc_bmac_attach: no mem for wlc_btc_params, malloced %d bytes\n",
				MALLOCED(wlc->osh)));
			return BCME_NOMEM;
		}
	}
#endif /* !defined(WLTEST) */

	/* Allocate space for Host-based COEX variables */
	if ((wlc_hw->btc->wlc_btc_params_fw = (uint16 *)
		MALLOC(wlc->osh, (BTC_FW_MAX_INDICES*sizeof(uint16)))) == NULL) {
		WL_ERROR(
		("wlc_bmac_attach: no mem for wlc_btc_params_fw, malloced %d bytes\n",
			MALLOCED(wlc->osh)));
		return BCME_NOMEM;
	}

	/* first count nr of present btc_params in NVRAM */
	for (indx = 0; indx <= M_BTCX_MAX_INDEX; indx++) {
		snprintf(buf, sizeof(buf), rstr_btc_paramsD, indx);
		if (getvar(wlc_hw->vars, buf) != NULL) {
			num_params++;
		}
	}
	if ((num_params > 0) || (getvar(wlc_hw->vars, rstr_btc_flags) != NULL)) {
		wlc->btc_param_vars = (struct wlc_btc_param_vars_info*) MALLOC(wlc->osh,
			sizeof(struct wlc_btc_param_vars_info) +
			num_params * sizeof(struct wlc_btc_param_vars_entry));
		if (wlc->btc_param_vars == NULL) {
			WL_ERROR(("wlc_btc_param_attach: no mem for btc_param_vars, malloc: %db\n",
				MALLOCED(wlc->osh)));
			return BCME_NOMEM;
		}
		wlc->btc_param_vars->num_entries = 0;
		/* go through all btc_params, and if exist in nvram copy to wlc->btc_param_vars */
		for (indx = 0; indx <= M_BTCX_MAX_INDEX; indx++) {
			snprintf(buf, sizeof(buf), rstr_btc_paramsD, indx);
			if (getvar(wlc_hw->vars, buf) != NULL) {
				wlc->btc_param_vars->param_list[
					wlc->btc_param_vars->num_entries].value =
					(uint16)getintvar(wlc_hw->vars, buf);
				wlc->btc_param_vars->param_list[
					wlc->btc_param_vars->num_entries++].index = indx;
			}
		}
		ASSERT(wlc->btc_param_vars->num_entries == num_params);
		/* check if btc_flags exist in nvram and if so copy to wlc->btc_param_vars */
		if (getvar(wlc_hw->vars, rstr_btc_flags) != NULL) {
			wlc->btc_param_vars->flags = (uint16)getintvar(wlc_hw->vars,
				rstr_btc_flags);
			wlc->btc_param_vars->flags_present = TRUE;
		} else {
			wlc->btc_param_vars->flags_present = FALSE;
		}
	}
	/* Initializing the host-based Bt-coex parameters */
	if (wlc_hw->btc->wlc_btc_params_fw) {
		wlc_hw->btc->wlc_btc_params_fw[BTC_FW_SISO_ACK_TX_PWR] = 0;
	}

	return BCME_OK;
}

static void
BCMINITFN(wlc_bmac_btc_param_init)(wlc_hw_info_t *wlc_hw)
{
	wlc_info_t *wlc = wlc_hw->wlc;
	uint16 indx;

	/* cache the pointer to the BTCX shm block, which won't change after coreinit */
	wlc_hw->btc->bt_shm_addr = 2 * wlc_bmac_read_shm(wlc_hw, M_BTCX_BLK_PTR);

	if (wlc_hw->btc->bt_shm_addr == 0)
		return;
	if (wlc_hw->btc->wlc_btc_params && wlc_hw->btc->btc_init_flag) {
		for (indx = 0; indx < M_BTCX_BACKUP_SIZE; indx++) {
			wlc_bmac_write_shm(wlc_hw, wlc_hw->btc->bt_shm_addr+indx*2,
				(uint16)(wlc_hw->btc->wlc_btc_params[indx]));
		}
	}
	/* First time initialization from nvram or ucode */
	/* or if wlc_btc_params is not malloced */
	else {
		wlc_hw->btc->btc_init_flag = TRUE;

		if (wlc->btc_param_vars == NULL) {
			/* wlc_btc_param_init: wlc->btc_param_vars unavailable */
			return;
		}

		/* go through all btc_params, if they existed in nvram, overwrite shared memory */
		for (indx = 0; indx < wlc->btc_param_vars->num_entries; indx++)
			wlc_bmac_write_shm(wlc_hw, wlc_hw->btc->bt_shm_addr +
				wlc->btc_param_vars->param_list[indx].index * 2,
				wlc->btc_param_vars->param_list[indx].value);
		/* go through btc_flags list as copied from nvram and initialize them */
		if (wlc->btc_param_vars->flags_present) {
			wlc_hw->btc->flags = wlc->btc_param_vars->flags;
		}
	}

	wlc_bmac_btc_btcflag2ucflag(wlc_hw);
}
#endif /* BCM_OL_DEV */

static void
wlc_bmac_btc_btcflag2ucflag(wlc_hw_info_t *wlc_hw)
{
	int indx;
	int btc_flags = wlc_hw->btc->flags;
	uint16 btc_mhf = (btc_flags & WL_BTC_FLAG_PREMPT) ? MHF2_BTCPREMPT : 0;

	wlc_bmac_mhf(wlc_hw, MHF2, MHF2_BTCPREMPT, btc_mhf, WLC_BAND_2G);
	btc_mhf = 0;
	for (indx = BTC_FLAGS_MHF3_START; indx <= BTC_FLAGS_MHF3_END; indx++)
		if (btc_flags & (1 << indx))
			btc_mhf |= btc_ucode_flags[indx].mask;

	btc_mhf &= ~(MHF3_BTCX_ACTIVE_PROT | MHF3_BTCX_PS_PROTECT);
	wlc_bmac_mhf(wlc_hw, MHF3, MHF3_BTCX_DEF_BT | MHF3_BTCX_SIM_RSP |
		MHF3_BTCX_ECI | MHF3_BTCX_SIM_TX_LP, btc_mhf, WLC_BAND_2G);

	/* Ucode needs ECI indication in all bands */
	if ((btc_mhf & ~MHF3_BTCX_ECI) == 0)
		wlc_bmac_mhf(wlc_hw, MHF3, MHF3_BTCX_ECI, btc_mhf & MHF3_BTCX_ECI, WLC_BAND_AUTO);
	btc_mhf = 0;
	for (indx = BTC_FLAGS_MHF3_END + 1; indx < BTC_FLAGS_SIZE; indx++)
		if (btc_flags & (1 << indx))
			btc_mhf |= btc_ucode_flags[indx].mask;

	wlc_bmac_mhf(wlc_hw, MHF5, MHF5_BTCX_LIGHT | MHF5_BTCX_PARALLEL,
		btc_mhf, WLC_BAND_2G);

	/* Need to specify when platform has low shared antenna isolation */
	if ((wlc_hw->sih->boardvendor == VENDOR_APPLE) &&
	    ((wlc_hw->sih->boardtype == BCM94331X29B) ||
	     (wlc_hw->sih->boardtype == BCM94331X29D) ||
	     (wlc_hw->sih->boardtype == BCM94331X33) ||
	     (wlc_hw->sih->boardtype == BCM94331X28B) ||
	     (CHIPID(wlc_hw->sih->chip) == BCM4360_CHIP_ID))) {
		wlc_bmac_mhf(wlc_hw, MHF5, MHF5_4331_BTCX_LOWISOLATION,
			MHF5_4331_BTCX_LOWISOLATION, WLC_BAND_2G);
	}
}

#ifdef STA
void
wlc_bmac_btc_update_predictor(wlc_hw_info_t *wlc_hw)
{
	uint32 tsf;
	uint16 bt_period, bt_last_l, bt_last_h, bt_shm_addr;
	uint32 bt_last, bt_next;
	d11regs_t *regs = wlc_hw->regs;

	bt_shm_addr = wlc_hw->btc->bt_shm_addr;
	if (bt_shm_addr == 0)
		return;

	/* Make sure period is known */
	bt_period = wlc_bmac_read_shm(wlc_hw, wlc_hw->btc->bt_shm_addr + M_BTCX_PRED_PER);

	if (bt_period == 0)
		return;

	tsf = R_REG(wlc_hw->osh, &regs->tsf_timerlow);

	/* Avoid partial read */
	do {
		bt_last_l = wlc_bmac_read_shm(wlc_hw, bt_shm_addr + M_BTCX_LAST_SCO);
		bt_last_h = wlc_bmac_read_shm(wlc_hw, bt_shm_addr + M_BTCX_LAST_SCO_H);
	} while (bt_last_l != wlc_bmac_read_shm(wlc_hw, bt_shm_addr + M_BTCX_LAST_SCO));
	bt_last = ((uint32)bt_last_h << 16) | bt_last_l;

	/* Calculate next expected BT slot time */
	bt_next = bt_last + ((((tsf - bt_last) / bt_period) + 1) * bt_period);
	wlc_bmac_write_shm(wlc_hw, bt_shm_addr + M_BTCX_NEXT_SCO, (uint16)(bt_next & 0xffff));
}
#endif /* STA */

/**
 * Bluetooth/WLAN coexistence parameters are exposed for some customers.
 * Rather than exposing all of shared memory, an index that is range-checked
 * is translated to an address.
 */
static bool
wlc_bmac_btc_param_to_shmem(wlc_hw_info_t *wlc_hw, uint32 *pval)
{
	if (*pval > M_BTCX_MAX_INDEX)
		return FALSE;

	if (wlc_hw->btc->bt_shm_addr == 0)
		return FALSE;

	*pval = wlc_hw->btc->bt_shm_addr + (2 * (*pval));
	return TRUE;
}

static bool
wlc_bmac_btc_flags_ucode(uint8 val, uint8 *idx, uint16 *mask)
{
	/* Check that the index is valid */
	if (val >= ARRAYSIZE(btc_ucode_flags))
		return FALSE;

	*idx = btc_ucode_flags[val].idx;
	*mask = btc_ucode_flags[val].mask;

	return TRUE;
}

int
wlc_bmac_btc_period_get(wlc_hw_info_t *wlc_hw, uint16 *btperiod, bool *btactive,
	uint16 *agg_off_bm)
{
	uint16 bt_period = 0, bt_shm_addr, bt_per_count = 0;
	uint32 tmp;
	d11regs_t *regs = wlc_hw->regs;

	bt_shm_addr = wlc_hw->btc->bt_shm_addr;

#define BTCX_PER_THRESHOLD 4
#define BTCX_BT_ACTIVE_THRESHOLD 5
#define BTCX_PER_OUT_OF_SYNC_CNT 15

	if (bt_shm_addr == 0)
		tmp = 0;

	else if ((bt_period = wlc_bmac_read_shm(wlc_hw, bt_shm_addr + M_BTCX_PRED_PER)) == 0)

		tmp = 0;

	else {
		/*
		Read PRED_PER_COUNT only for non-ECI chips. For ECI, PRED_PER gets
		cleared as soon as periodic activity ends so there is no need to
		monitor PRED_PER_COUNT.
		*/
		if (!BCMCOEX_ENAB_BMAC(wlc_hw)) {
			if ((bt_per_count = wlc_bmac_read_shm(wlc_hw,
				bt_shm_addr + M_BTCX_PRED_PER_COUNT)) <= BTCX_PER_THRESHOLD)
				tmp = 0;
			else
				tmp = bt_period;
		}
		else {
			tmp = bt_period;
		}
	}
	/*
	This code has been added to filter out any spurious reads of PRED_PER
	being '0' (this may happen if the value is read right after a mac
	suspend/resume because ucode clears out this value after resumption).
	*/
	if (!tmp) {
		wlc_hw->btc->bt_period_out_of_sync_cnt++;
		if (wlc_hw->btc->bt_period_out_of_sync_cnt <= BTCX_PER_OUT_OF_SYNC_CNT) {
			tmp = wlc_hw->btc->bt_period;
		} else {
			wlc_hw->btc->bt_period_out_of_sync_cnt = BTCX_PER_OUT_OF_SYNC_CNT;
		}
	}
	else
		wlc_hw->btc->bt_period_out_of_sync_cnt = 0;
	*btperiod = wlc_hw->btc->bt_period = (uint16)tmp;

	*agg_off_bm = wlc_bmac_read_shm(wlc_hw, bt_shm_addr + M_BTCX_AGG_OFF_BM);

	if (R_REG(wlc_hw->osh, &regs->maccontrol) & MCTL_PSM_RUN) {
		tmp = R_REG(wlc_hw->osh, &regs->u.d11regs.btcx_cur_rfact_timer);
		/* code below can be optimized for speed; however, we choose not
		 * to do that to achieve better readability
		 */
		if (wlc_hw->btc->bt_active) {
			/* active state : switch to inactive when reading 0xffff */
			if (tmp == 0xffff) {
				wlc_hw->btc->bt_active = FALSE;
				wlc_hw->btc->bt_active_asserted_cnt = 0;
			}
		} else {
			/* inactive state : switch to active when bt_active asserted for
			 * more than a certain times
			 */
			if (tmp == 0xffff)
				wlc_hw->btc->bt_active_asserted_cnt = 0;
			/* concecutive asserts, now declare bt is active */
			else if (++wlc_hw->btc->bt_active_asserted_cnt >= BTCX_BT_ACTIVE_THRESHOLD)
				wlc_hw->btc->bt_active = TRUE;
		}
	}

	*btactive = wlc_hw->btc->bt_active;

	return BCME_OK;
}

int
wlc_bmac_btc_mode_set(wlc_hw_info_t *wlc_hw, int btc_mode)
{
	uint16 btc_mhfs[MHFMAX];
	bool ucode_up = FALSE;

	if (btc_mode > WL_BTC_DEFAULT)
		return BCME_BADARG;

	/* Make sure 2-wire or 3-wire decision has been made */
	ASSERT((wlc_hw->btc->wire >= WL_BTC_2WIRE) || (wlc_hw->btc->wire <= WL_BTC_4WIRE));

	 /* Determine the default mode for the device */
	if (btc_mode == WL_BTC_DEFAULT) {
		if (BCMCOEX_ENAB_BMAC(wlc_hw) || (wlc_hw->boardflags2 & BFL2_BTCLEGACY)) {
			btc_mode = WL_BTC_FULLTDM;
			/* default to hybrid mode for combo boards with 2 or more antennas */
			if (wlc_hw->btc->btcx_aa > 2) {
				if (CHIPID(wlc_hw->sih->chip) == BCM43242_CHIP_ID ||
					(CHIPID(wlc_hw->sih->chip) == BCM4354_CHIP_ID) ||
					(CHIPID(wlc_hw->sih->chip) == BCM4356_CHIP_ID))
					btc_mode = WL_BTC_FULLTDM;
				else if (CHIPID(wlc_hw->sih->chip) == BCM43142_CHIP_ID)
					btc_mode = WL_BTC_LITE;
				else
					btc_mode = WL_BTC_HYBRID;
			}
		}
		else
			btc_mode = WL_BTC_DISABLE;
	}

	/* Do not allow an enable without hw support */
	if (btc_mode != WL_BTC_DISABLE) {
		if ((wlc_hw->btc->wire >= WL_BTC_3WIRE) && D11REV_GE(wlc_hw->corerev, 13) &&
			!(wlc_hw->machwcap & MCAP_BTCX))
			return BCME_BADOPTION;
	}

	/* Initialize ucode flags */
	bzero(btc_mhfs, sizeof(btc_mhfs));
	wlc_hw->btc->flags = 0;

	if (wlc_hw->up)
		ucode_up = (R_REG(wlc_hw->osh, &wlc_hw->regs->maccontrol) & MCTL_EN_MAC);

	if (btc_mode != WL_BTC_DISABLE) {
		btc_mhfs[MHF1] |= MHF1_BTCOEXIST;
		if (wlc_hw->btc->wire == WL_BTC_2WIRE) {
			/* BMAC_NOTES: sync the state with HIGH driver ??? */
			/* Make sure 3-wire coex is off */
			if (wlc_hw->boardflags & BFL_BTC2WIRE_ALTGPIO) {
				btc_mhfs[MHF2] |= MHF2_BTC2WIRE_ALTGPIO;
				wlc_hw->btc->gpio_mask =
					BOARD_GPIO_BTCMOD_OUT | BOARD_GPIO_BTCMOD_IN;
				wlc_hw->btc->gpio_out = BOARD_GPIO_BTCMOD_OUT;
			} else {
				btc_mhfs[MHF2] &= ~MHF2_BTC2WIRE_ALTGPIO;
			}
		} else {
			if (D11REV_GE(wlc_hw->corerev, 13)) {
				/* by default we use PS protection unless overriden. */
				if (btc_mode == WL_BTC_HYBRID)
					wlc_hw->btc->flags |= WL_BTC_FLAG_SIM_RSP;
				else if (btc_mode == WL_BTC_LITE) {
					/* for X28, parallel mode used given 30+ isolation */
					if (CHIPID(wlc_hw->sih->chip) == BCM4331_CHIP_ID &&
						(wlc_hw->boardflags & BFL_FEM_BT))
						wlc_hw->btc->flags |= WL_BTC_FLAG_PARALLEL;
					else
						wlc_hw->btc->flags |= WL_BTC_FLAG_LIGHT;
					wlc_hw->btc->flags |= WL_BTC_FLAG_SIM_RSP;
				} else if (btc_mode == WL_BTC_PARALLEL) {
					wlc_hw->btc->flags |= WL_BTC_FLAG_PARALLEL;
					wlc_hw->btc->flags |= WL_BTC_FLAG_SIM_RSP;
				}
				else
					wlc_hw->btc->flags |=
						(WL_BTC_FLAG_PS_PROTECT | WL_BTC_FLAG_ACTIVE_PROT);

				if (BCMCOEX_ENAB_BMAC(wlc_hw)) {
					wlc_hw->btc->flags |= WL_BTC_FLAG_ECI;
				} else {
					if (wlc_hw->btc->wire == WL_BTC_4WIRE)
						btc_mhfs[MHF3] |= MHF3_BTCX_EXTRA_PRI;
					else
						wlc_hw->btc->flags |= WL_BTC_FLAG_PREMPT;
				}
			} else { /* 3-wire over GPIO */
				wlc_hw->btc->flags |= (WL_BTC_FLAG_ACTIVE_PROT |
					WL_BTC_FLAG_SIM_RSP);
				wlc_hw->btc->gpio_mask = BOARD_GPIO_BTC3W_OUT | BOARD_GPIO_BTC3W_IN;
				wlc_hw->btc->gpio_out = BOARD_GPIO_BTC3W_OUT;
			}
		}

	} else {
		btc_mhfs[MHF1] &= ~MHF1_BTCOEXIST;
	}

#if defined(BCMECICOEX) && defined(WLC_HIGH)
	if (CHIPID(wlc_hw->sih->chip) == BCM4350_CHIP_ID) {
#ifdef MACOSX
		if (btc_mode == WL_BTC_HYBRID || btc_mode == WL_BTC_FULLTDM) {
#else
		if (btc_mode == WL_BTC_HYBRID) {
#endif // endif
			wlc_btc_siso_ack_set(wlc_hw->wlc, AUTO, FALSE);
		} else {
			wlc_btc_siso_ack_set(wlc_hw->wlc, 0, FALSE);
		}
	}
#endif /* BCMECICOEX */

	wlc_hw->btc->mode = btc_mode;

	/* Set the MHFs only in 2G band
	 * If we are on the other band, update the sw cache for the
	 * 2G band.
	 */
	if (wlc_hw->up && ucode_up)
		wlc_bmac_suspend_mac_and_wait(wlc_hw);

	wlc_bmac_mhf(wlc_hw, MHF1, MHF1_BTCOEXIST, btc_mhfs[MHF1], WLC_BAND_2G);
	wlc_bmac_mhf(wlc_hw, MHF2, MHF2_BTC2WIRE_ALTGPIO, btc_mhfs[MHF2],
		WLC_BAND_2G);
	wlc_bmac_mhf(wlc_hw, MHF3, MHF3_BTCX_EXTRA_PRI, btc_mhfs[MHF3], WLC_BAND_2G);
	wlc_bmac_btc_btcflag2ucflag(wlc_hw);

	if (wlc_hw->up && ucode_up) {
		wlc_bmac_enable_mac(wlc_hw);
	}

	return BCME_OK;
}

int
wlc_bmac_btc_mode_get(wlc_hw_info_t *wlc_hw)
{
	return wlc_hw->btc->mode;
}

int
wlc_bmac_btc_wire_get(wlc_hw_info_t *wlc_hw)
{
	return wlc_hw->btc->wire;
}

int
wlc_bmac_btc_wire_set(wlc_hw_info_t *wlc_hw, int btc_wire)
{
	/* Has to be down. Enforced through iovar flag */
	ASSERT(!wlc_hw->up);

	if (btc_wire > WL_BTC_4WIRE)
		return BCME_BADARG;

	/* default to 4-wire ucode if 3-wire boardflag is set or
	 * - M93 or ECI is enabled
	 * else default to 2-wire
	 */
	if (btc_wire == WL_BTC_DEFWIRE) {
		/* Use the boardflags to finally fix the setting for
		 * boards with correct flags
		 */
		if (BCMCOEX_ENAB_BMAC(wlc_hw))
			wlc_hw->btc->wire = WL_BTC_3WIRE;
		else if (wlc_hw->boardflags2 & BFL2_BTCLEGACY) {
			if (wlc_hw->boardflags2 & BFL2_BTC3WIREONLY)
				wlc_hw->btc->wire = WL_BTC_3WIRE;
			else
				wlc_hw->btc->wire = WL_BTC_4WIRE;
		} else
			wlc_hw->btc->wire = WL_BTC_2WIRE;

		/* some boards may not have the 3-wire boardflag */
		if (D11REV_IS(wlc_hw->corerev, 12) &&
		    ((wlc_hw->sih->boardvendor == VENDOR_APPLE) &&
		     ((wlc_hw->sih->boardtype == BCM943224M93) ||
		      (wlc_hw->sih->boardtype == BCM943224M93A))))
			wlc_hw->btc->wire = WL_BTC_3WIRE;
	}
	else
		wlc_hw->btc->wire = btc_wire;
	/* flush ucode_loaded so the ucode download will happen again to pickup the right ucode */
	wlc_hw->ucode_loaded = FALSE;

	wlc_bmac_btc_gpio_configure(wlc_hw);

	return BCME_OK;
}

int
wlc_bmac_btc_flags_get(wlc_hw_info_t *wlc_hw)
{
	return wlc_hw->btc->flags;
}

static void
wlc_bmac_btc_flags_upd(wlc_hw_info_t *wlc_hw, bool set_clear, uint16 val, uint8 idx, uint16 mask)
{
	if (set_clear) {
		wlc_hw->btc->flags |= val;
		wlc_bmac_mhf(wlc_hw, idx, mask, mask, WLC_BAND_2G);
	} else {
		wlc_hw->btc->flags &= ~val;
		wlc_bmac_mhf(wlc_hw, idx, mask, 0, WLC_BAND_2G);
	}
}

int
wlc_bmac_btc_flags_idx_get(wlc_hw_info_t *wlc_hw, int int_val)
{
	uint8 idx = 0;
	uint16 mask = 0;

	if (!wlc_bmac_btc_flags_ucode((uint8)int_val, &idx, &mask))
		return 0xbad;

	return (wlc_bmac_mhf_get(wlc_hw, idx, WLC_BAND_2G) & mask) ? 1 : 0;
}

int
wlc_bmac_btc_flags_idx_set(wlc_hw_info_t *wlc_hw, int int_val, int int_val2)
{
	uint8 idx = 0;
	uint16 mask = 0;

	if (!wlc_bmac_btc_flags_ucode((uint8)int_val, &idx, &mask))
		return BCME_BADARG;

	if (int_val2)
		wlc_bmac_btc_flags_upd(wlc_hw, TRUE, (uint16)(int_val2 << int_val), idx, mask);
	else
		wlc_bmac_btc_flags_upd(wlc_hw, FALSE, (uint16)(1 << int_val), idx, mask);

	return BCME_OK;
}

void
wlc_bmac_btc_stuck_war50943(wlc_hw_info_t *wlc_hw, bool enable)
{
	if (enable) {
		wlc_hw->btc->stuck_detected = FALSE;
		wlc_hw->btc->stuck_war50943 = TRUE;
		wlc_bmac_mhf(wlc_hw, MHF3, MHF3_BTCX_DELL_WAR, MHF3_BTCX_DELL_WAR, WLC_BAND_ALL);
	} else {
		wlc_hw->btc->stuck_war50943 = FALSE;
		wlc_bmac_mhf(wlc_hw, MHF3, MHF3_BTCX_DELL_WAR, 0, WLC_BAND_ALL);
	}
}

int
wlc_bmac_btc_params_set(wlc_hw_info_t *wlc_hw, int int_val, int int_val2)
{
	/* btc_params with indices > 1000 are stored in FW.
	 * First check to see whether this is a FW btc_param.
	 */
	if (int_val >= BTC_PARAMS_FW_START_IDX) {
		if (!(wlc_hw->btc->wlc_btc_params_fw)) return BCME_ERROR;
		int_val -= BTC_PARAMS_FW_START_IDX; /* Normalize to a 0-based index */
		if (int_val < BTC_FW_MAX_INDICES) {
			wlc_hw->btc->wlc_btc_params_fw[int_val] = (uint16)int_val2;
			return BCME_OK;
		} else {
			return BCME_BADADDR;
		}
	} else {
		/* If shmem is powered down & wlc_btc_params cached values exist,
		 * then update the relevant cached value based on the int_val index
		 */
		if (!(wlc_hw->up)) {
			if (!(wlc_hw->btc->wlc_btc_params))
				return BCME_ERROR;
			if (int_val < M_BTCX_BACKUP_SIZE) {
				wlc_hw->btc->wlc_btc_params[int_val] = (uint16)int_val2;
				return BCME_OK;
			} else {
				return BCME_BADADDR;
			}
		} else {
			if (!wlc_bmac_btc_param_to_shmem(wlc_hw, (uint32*)&int_val))
				return BCME_BADARG;
			wlc_bmac_write_shm(wlc_hw, (uint16)int_val, (uint16)int_val2);
			return BCME_OK;
		}
	}
}

int
wlc_bmac_btc_params_get(wlc_hw_info_t *wlc_hw, int int_val)
{
	/* btc_params with indices > 1000 are stored in FW.
	 * First check to see whether this is a FW btc_param.
	 */
	if (int_val >= BTC_PARAMS_FW_START_IDX) {
		if (!(wlc_hw->btc->wlc_btc_params_fw))
			return BCME_ERROR;
		int_val -= BTC_PARAMS_FW_START_IDX; /* Normalize to a 0-based index */
		if (int_val < BTC_FW_MAX_INDICES) {
			return wlc_hw->btc->wlc_btc_params_fw[int_val];
		} else {
			return 0xbad;
		}
	} else {
		/* If shmem is powered down & wlc_btc_params cached values exist,
		 * then read from the relevant cached value based on the int_val index
		 */
		if (!(wlc_hw->up)) {
			if (!(wlc_hw->btc->wlc_btc_params))
				return 0xbad;
			if (int_val < M_BTCX_BACKUP_SIZE) {
				return wlc_hw->btc->wlc_btc_params[int_val];
			} else {
				return 0xbad;
			}
		} else {
			if (!wlc_bmac_btc_param_to_shmem(wlc_hw, (uint32*)&int_val))
				return 0xbad;
			return wlc_bmac_read_shm(wlc_hw, (uint16)int_val);
		}
	}
}

#if defined(BTCX_ENABLED)
void
BCMUNINITFN(wlc_bmac_btc_params_save)(wlc_hw_info_t *wlc_hw)
{
	uint16 bt_shm_addr = wlc_hw->btc->bt_shm_addr;
	uint16* wlc_btc_params = wlc_hw->btc->wlc_btc_params;

	/* Save btc shmem values before going down */
	if (wlc_hw->btc->wlc_btc_params) {
	/* Get pointer to the BTCX shm block */
		if (0 != (bt_shm_addr = 2 * wlc_bmac_read_shm(wlc_hw, M_BTCX_BLK_PTR))) {
			wlc_bmac_copyfrom_shm(wlc_hw, bt_shm_addr, wlc_btc_params,
					M_BTCX_BACKUP_SIZE);
		}
	}
}
#endif /* BTCX_ENABLED */

void
wlc_bmac_btc_rssi_threshold_get(wlc_hw_info_t *wlc_hw,
	uint8 *prot, uint8 *high_thresh, uint8 *low_thresh)
{
	uint16 bt_shm_addr = wlc_hw->btc->bt_shm_addr;

	if (bt_shm_addr == 0)
		return;

	*prot =	(uint8)wlc_bmac_read_shm(wlc_hw, bt_shm_addr + M_BTCX_PROT_RSSI_THRESH);
	*high_thresh = (uint8)wlc_bmac_read_shm(wlc_hw, bt_shm_addr + M_BTCX_HIGH_THRESH);
	*low_thresh = (uint8)wlc_bmac_read_shm(wlc_hw, bt_shm_addr + M_BTCX_LOW_THRESH);
}

/** configure 3/4 wire coex gpio for newer chips */
void
wlc_bmac_btc_gpio_configure(wlc_hw_info_t *wlc_hw)
{

	if (wlc_hw->btc->wire >= WL_BTC_3WIRE) {
		uint32 gm = 0;
		switch ((CHIPID(wlc_hw->sih->chip))) {
		case BCM43224_CHIP_ID:
		case BCM43421_CHIP_ID:
			if (wlc_hw->boardflags & BFL_FEM_BT)
				gm = GPIO_BTC4W_OUT_43224_SHARED;
			else
				gm = GPIO_BTC4W_OUT_43224;
			break;
		case BCM43225_CHIP_ID:
			gm = GPIO_BTC4W_OUT_43225;
			break;
		case BCM4312_CHIP_ID:
			gm = GPIO_BTC4W_OUT_4312;
			break;
		case BCM4313_CHIP_ID:
			gm = GPIO_BTC4W_OUT_4313;
			break;
		};

		wlc_hw->btc->gpio_mask = wlc_hw->btc->gpio_out = gm;
	}
}

/** Lower BTC GPIO through ChipCommon when BTC is OFF or D11 MAC is in reset or on powerup */
void
wlc_bmac_btc_gpio_disable(wlc_hw_info_t *wlc_hw)
{
	uint32 gm, go;
	si_t *sih;
	bool xtal_set = FALSE;

	if (!wlc_hw->sbclk) {
		wlc_bmac_xtal(wlc_hw, ON);
		xtal_set = TRUE;
	}

	/* Proceed only if BTC GPIOs had been configured */
	if (wlc_hw->btc->gpio_mask == 0)
		return;

	sih = wlc_hw->sih;

	gm = wlc_hw->btc->gpio_mask;
	go = wlc_hw->btc->gpio_out;

	/* Set the control of GPIO back and lower only GPIO OUT pins and not the ones that
	 * are supposed to be IN
	 */
	si_gpiocontrol(sih, gm, 0, GPIO_DRV_PRIORITY);
	/* configure gpio to input to float pad */
	si_gpioouten(sih, gm, 0, GPIO_DRV_PRIORITY);
	/* a HACK to enable internal pulldown for 4313 */
	if (CHIPID(wlc_hw->sih->chip) == BCM4313_CHIP_ID)
		si_gpiopull(wlc_hw->sih, GPIO_PULLDN, gm, 0x40);

	si_gpioout(sih, go, 0, GPIO_DRV_PRIORITY);

	if (wlc_hw->clk)
		AND_REG(wlc_hw->osh, &wlc_hw->regs->psm_gpio_oe, ~wlc_hw->btc->gpio_out);

	/* BMAC_NOTE: PCI_BUS check here is actually not relevant; there is nothing PCI
	 * bus specific here it was only meant to be compile time optimization. Now it's
	 * true that it may not anyway be applicable to 4323, but need to see if there are
	 * any more places like this
	 */
	/* On someboards, which give GPIOs to UART via strapping,
	 * GPIO_BTC_OUT is not directly controlled by gpioout on CC
	 */
	if ((BUSTYPE(sih->bustype) == PCI_BUS) && (gm & BOARD_GPIO_BTC_OUT))
		si_btcgpiowar(sih);

	if (xtal_set)
		wlc_bmac_xtal(wlc_hw, OFF);

}

/** Set BTC GPIO through ChipCommon when BTC is ON */
static void
wlc_bmac_btc_gpio_enable(wlc_hw_info_t *wlc_hw)
{
	uint32 gm, gi;
	si_t *sih;

	ASSERT(wlc_hw->clk);

	/* Proceed only if GPIO-based BTC is configured */
	if (wlc_hw->btc->gpio_mask == 0)
		return;

	sih = wlc_hw->sih;

	gm = wlc_hw->btc->gpio_mask;
	gi = (~wlc_hw->btc->gpio_out) & wlc_hw->btc->gpio_mask;

	OR_REG(wlc_hw->osh, &wlc_hw->regs->psm_gpio_oe, wlc_hw->btc->gpio_out);
	/* Clear OUT enable from GPIOs that the driver expects to be IN */
	si_gpioouten(sih, gi, 0, GPIO_DRV_PRIORITY);

	if (CHIPID(wlc_hw->sih->chip) == BCM4313_CHIP_ID)
		si_gpiopull(wlc_hw->sih, GPIO_PULLDN, gm, 0);
	si_gpiocontrol(sih, gm, gm, GPIO_DRV_PRIORITY);
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static void
wlc_bmac_btc_dump(wlc_hw_info_t *wlc_hw, struct bcmstrbuf *b)
{
	bcm_bprintf(b, "BTC---\n");
	bcm_bprintf(b, "btc_mode %d btc_wire %d btc_flags %d "
		"btc_gpio_mask %d btc_gpio_out %d btc_stuck_detected %d btc_stuck_war50943 %d "
		"bt_shm_add %d bt_period %d bt_active %d\n",
		wlc_hw->btc->mode, wlc_hw->btc->wire, wlc_hw->btc->flags, wlc_hw->btc->gpio_mask,
		wlc_hw->btc->gpio_out, wlc_hw->btc->stuck_detected, wlc_hw->btc->stuck_war50943,
		wlc_hw->btc->bt_shm_addr, wlc_hw->btc->bt_period, wlc_hw->btc->bt_active);
}
#endif	/* BCMDBG || BCMDBG_DUMP */

#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(BCMDBG_PHYDUMP)
static void
wlc_bmac_suspend_dump(wlc_hw_info_t *wlc_hw, struct bcmstrbuf *b)
{
	bmac_suspend_stats_t* stats = wlc_hw->suspend_stats;
	uint32 suspend_time = stats->suspended;
	uint32 unsuspend_time = stats->unsuspended;
	uint32 ratio = 0;
	uint32 timenow = R_REG(wlc_hw->osh, &wlc_hw->regs->tsf_timerlow);
	bool   suspend_active = stats->suspend_start > stats->suspend_end;

	bcm_bprintf(b, "bmac suspend stats---\n");
	bcm_bprintf(b, "Suspend count: %d%s\n", stats->suspend_count,
	            suspend_active ? " ACTIVE" : "");

	if (suspend_active) {
		if (timenow > stats->suspend_start) {
			suspend_time += (timenow - stats->suspend_start) / 100;
			stats->suspend_start = timenow;
		}
	}
	else {
		if (timenow > stats->suspend_end) {
			unsuspend_time += (timenow - stats->suspend_end) / 100;
			stats->suspend_end = timenow;
		}
	}

	bcm_bprintf(b, "    Suspended: %9d millisecs\n", (suspend_time + 5)/10);
	bcm_bprintf(b, "  Unsuspended: %9d millisecs\n", (unsuspend_time + 5)/10);
	bcm_bprintf(b, "  Max suspend: %9d millisecs\n", (stats->suspend_max + 5)/10);
	bcm_bprintf(b, " Mean suspend: %9d millisecs\n",
	           (suspend_time / (stats->suspend_count ? stats->suspend_count : 1) + 5)/10);

	/* avoid problems with arithmetric overflow */
	while ((suspend_time > (1 << 26)) || (unsuspend_time > (1 << 26))) {
		suspend_time >>= 1;
		unsuspend_time >>= 1;
	}

	if (suspend_time && unsuspend_time) {
		ratio = (suspend_time + unsuspend_time) * 10;
		ratio /= suspend_time;

		if (ratio > 0) {
			ratio = 100000 / ratio;
		}
		ratio = (ratio + 5)/10;
	}

	bcm_bprintf(b, "Suspend ratio: %3d / 1000\n", ratio);

	stats->suspend_count = 0;
	stats->unsuspended = 0;
	stats->suspended = 0;
	stats->suspend_max = 0;
}
#endif	/* defined(BCMDBG) || defined(BCMDBG_DUMP) || || defined(BCMDBG_PHYDUMP) */

/* BTC stuff END */

#ifdef STA
/** Change PCIE War override for some platforms */
void
wlc_bmac_pcie_war_ovr_update(wlc_hw_info_t *wlc_hw, uint8 aspm)
{
	if ((BUSTYPE(wlc_hw->sih->bustype) == PCI_BUS) &&
		(wlc_hw->sih->buscoretype == PCIE_CORE_ID))
		si_pcie_war_ovr_update(wlc_hw->sih, aspm);
}

void
wlc_bmac_pcie_power_save_enable(wlc_hw_info_t *wlc_hw, bool enable)
{
	if ((BUSTYPE(wlc_hw->sih->bustype) == PCI_BUS) &&
		(wlc_hw->sih->buscoretype == PCIE_CORE_ID))
		si_pcie_power_save_enable(wlc_hw->sih, enable);
}
#endif /* STA */

#ifdef BCMUCDOWNLOAD
/** function to write ucode to ucode memory */
int
wlc_handle_ucodefw(wlc_info_t *wlc, wl_ucode_info_t *ucode_buf)
{
	/* for first chunk turn on the clock & do core reset */
	if (ucode_buf->chunk_num == 1) {
		wlc_bmac_xtal(wlc->hw, ON);
		wlc_bmac_corereset(wlc->hw, WLC_USE_COREFLAGS);
	}
	/* write ucode chunk to ucode memory */
	if (WLCISLCNPHY(wlc->hw->band) || WLCISSSLPNPHY(wlc->hw->band))
		wlc_ucode_write_byte(wlc->hw, &ucode_buf->data_chunk[0], ucode_buf->chunk_len);
	else
		wlc_ucode_write(wlc->hw,  (uint32 *)(&ucode_buf->data_chunk[0]),
			ucode_buf->chunk_len);
	return 0;
}

/**
 * function to handle initvals & bsinitvals. Initvals chunks are accumulated
 * in the driver & kept allocated till 'wl up'. During 'wl up' initvals
 * are written to the memory & then buffer is freed. Even though bsinitvals
 * implementation is also present it is not being downloaded from the host
 * since the size is small & will not be reclaimed if it is dual band image
 */
int
wlc_handle_initvals(wlc_info_t *wlc, wl_ucode_info_t *ucode_buf)
{
	if (ucode_buf->chunk_num == 1) {
		initvals_len = ucode_buf->num_chunks * ucode_buf->chunk_len * sizeof(uint8);
		initvals_ptr = (d11init_t *)MALLOC(wlc->osh, initvals_len);
	}

	bcopy(ucode_buf->data_chunk, (uint8*)initvals_ptr + cumulative_len, ucode_buf->chunk_len);
	cumulative_len += ucode_buf->chunk_len;

	/* when last chunk is received call the write function  */
	if (ucode_buf->chunk_num == ucode_buf->num_chunks)
		wlc->is_initvalsdloaded = TRUE;
	return 0;
}

/**
 * Generic function to handle different downloadable parts like ucode fw
 * & initvals & bsinitvals
 */
int
wlc_process_ucodeparts(wlc_info_t *wlc, uint8 *buf_to_process)
{
	wl_ucode_info_t *ucode_buf = (wl_ucode_info_t *)buf_to_process;
	if (ucode_buf->ucode_type == INIT_VALS)
		wlc_handle_initvals(wlc, ucode_buf);
	else
		wlc_handle_ucodefw(wlc, ucode_buf);
	return 0;
}
#endif /* BCMUCDOWNLOAD */

#ifdef WLC_LOW_ONLY
void
wlc_bmac_reload_mac(wlc_hw_info_t *wlc_hw)
{
	bcopy(&wlc_hw->orig_etheraddr, &wlc_hw->etheraddr, ETHER_ADDR_LEN);
	ASSERT(!ETHER_ISBCAST((char*)&wlc_hw->etheraddr));
	ASSERT(!ETHER_ISNULLADDR((char*)&wlc_hw->etheraddr));
}
#endif // endif

/* The function is supposed to enable/disable MI_TBTT or M_P2P_I_PRE_TBTT.
 * But since there is no control over M_P2P_I_PRE_TBTT interrupt ,
 * this is achieved by enabling/disabling MI_P2P interrupt as a whole, though
 * that is not the actual intention. The assumption here is if
 * M_P2P_I_PRE_TBTT is no required, no other P2P interrupt will be required.
 * Do not use this function to enable/disable MI_P2P in other conditions.
 * Smply use wlc_bmac_set_defmacintmask() if required.
 */

void
wlc_bmac_enable_tbtt(wlc_hw_info_t *wlc_hw, uint32 mask, uint32 val)
{
	wlc_hw->tbttenablemask = (wlc_hw->tbttenablemask & ~mask) | (val & mask);

	if (wlc_hw->tbttenablemask)
		wlc_bmac_set_defmacintmask(wlc_hw, MI_P2P|MI_TBTT, MI_P2P|MI_TBTT);
	else
		wlc_bmac_set_defmacintmask(wlc_hw, MI_P2P|MI_TBTT, ~(MI_P2P|MI_TBTT));
}

void
wlc_bmac_set_defmacintmask(wlc_hw_info_t *wlc_hw, uint32 mask, uint32 val)
{
	wlc_hw->defmacintmask = (wlc_hw->defmacintmask & ~mask) | (val & mask);
}

#ifdef BPRESET
#include <wlc_scb.h>
void
wlc_full_reset(wlc_hw_info_t *wlc_hw, uint32 val)
{
	osl_t *osh;
	uint32 bar0win;
	uint32 bar0win_after;
	int i;
#ifdef BCMDBG
	uint32 start = OSL_SYSUPTIME();
#endif // endif
	uint tmp_bcn_li_dtim;
	uint32 mac_intmask;
	wlc_info_t *wlc = wlc_hw->wlc;
	int ac;

	if (!BPRESET_ENAB(wlc->pub)) {
		WL_ERROR(("wl%d: BPRESET not enabled, do nothing!\n", wlc->pub->unit));
		return;
	}

	/*
	 * 0:	Just show we are alive
	 * 1:	Basic big hammer
	 * 2:	Bigger hammer, big hammer plus backplane reset
	 * 4:	Extra debugging after wl_init
	 * 8:	Issue wl_down() & wl_up() after wl_init
	 */
	WL_ERROR(("wl%d: %s(0x%x): starting backplane reset\n",
	           wlc_hw->unit, __FUNCTION__, val));

	osh = wlc_hw->osh;

	if (val == 0)
		return;

	/* stop DMA */
	if (!PIO_ENAB(wlc_hw->wlc->pub)) {
		for (i = 0; i < wlc_hw->nfifo_inuse; i++) {
			if (wlc_hw->di[i]) {
				if (!dma_txreset(wlc_hw->di[i])) {
					WL_ERROR(("wl%d: %s: dma_txreset[%d]: cannot stop dma\n",
					          wlc_hw->unit, __FUNCTION__, i));
					WL_HEALTH_LOG(wlc_hw->wlc, DMATX_ERROR);
				}
				wlc_upd_suspended_fifos_clear(wlc_hw, i);
			}
		}
		if ((wlc_hw->di[RX_FIFO]) && (!wlc_dma_rxreset(wlc_hw, RX_FIFO))) {
			WL_ERROR(("wl%d: %s: dma_rxreset[%d]: cannot stop dma\n",
			          wlc_hw->unit, __FUNCTION__, RX_FIFO));
			WL_HEALTH_LOG(wlc_hw->wlc, DMARX_ERROR);
		}
	} else {
		for (i = 0; i < NFIFO; i++)
			if (wlc_hw->pio[i])
				wlc_pio_reset(wlc_hw->pio[i]);
	}

	WL_NONE(("wl%d: %s: up %d, hw->up %d, sbclk %d, clk %d, hw->clk %d, fastclk %d\n",
	         wlc_hw->unit, __FUNCTION__, wlc_hw->wlc->pub->up, wlc_hw->up,
	         wlc_hw->sbclk, wlc_hw->wlc->clk, wlc_hw->clk, wlc_hw->forcefastclk));

	if (val & 2) {
		/* cause chipc watchdog */
		WL_INFORM(("wl%d: %s: starting chipc watchdog\n",
		           wlc_hw->unit, __FUNCTION__));

		bar0win = OSL_PCI_READ_CONFIG(osh, PCI_BAR0_WIN, sizeof(uint32));

		/* Stop interrupt handling */
		wlc_hw->macintmask = 0;

		wlc_bmac_set_ctrl_SROM(wlc_hw);
		if (R_REG(wlc_hw->osh, &wlc_hw->regs->maccontrol) & MCTL_EN_MAC) {
			wlc_bmac_suspend_mac_and_wait(wlc_hw);
		}

		if (CHIPID(wlc_hw->sih->chip) == BCM4331_CHIP_ID &&
		    ((D11REV_IS(wlc_hw->corerev, 26) && CHIPREV(wlc_hw->sih->chiprev) == 0) ||
		     D11REV_IS(wlc_hw->corerev, 29))) {
			si_corereg(wlc_hw->sih, SI_CC_IDX, OFFSETOF(chipcregs_t, chipcontrol),
				(CCTRL4331_EXTPA_EN | CCTRL4331_EXTPA_EN2 |
				CCTRL4331_EXTPA_ON_GPIO2_5), 0);
		}

		/* Write the watchdog */
		si_corereg(wlc_hw->sih, SI_CC_IDX, OFFSETOF(chipcregs_t, watchdog), ~0, 100);

		/* Srom read takes ~12mS */
		OSL_DELAY(20000);

		bar0win_after = OSL_PCI_READ_CONFIG(osh, PCI_BAR0_WIN, sizeof(uint32));

		if (bar0win_after != bar0win) {
			WL_ERROR(("wl%d: %s: bar0win before %08x, bar0win after %08x\n",
			          wlc_hw->unit, __FUNCTION__, bar0win, bar0win_after));
			OSL_PCI_WRITE_CONFIG(osh, PCI_BAR0_WIN, sizeof(uint32), bar0win);
		}

		/* If the core is up, the watchdog did not take effect */
		if (si_iscoreup(wlc_hw->sih))
			WL_ERROR(("wl%d: %s: Core still up after WD\n",
			          wlc_hw->unit, __FUNCTION__));

		/* Fixup the state to say the chip (or at least d11) is down */
		wlc_hw->clk = FALSE;

		/* restore hardware related stuff */
		wlc_bmac_up_prep(wlc_hw);
	}

	WL_INFORM(("wl%d: %s: about to wl_init()\n", wlc_hw->unit, __FUNCTION__));

	tmp_bcn_li_dtim = wlc_hw->wlc->bcn_li_dtim;
	wlc_hw->wlc->bcn_li_dtim = 0;
	wlc_fatal_error(wlc_hw->wlc);	/* big hammer */

	/* Propagate rfaware_lifetime setting to ucode */
	wlc_rfaware_lifetime_set(wlc, wlc->rfaware_lifetime);

	/* for full backplane reset, need to reenable interrupt */
	if (val & 2) {
		/* FULLY enable dynamic power control and d11 core interrupt */
		wlc_clkctl_clk(wlc_hw, CLK_DYNAMIC);
		ASSERT(wlc_hw->macintmask == 0);
		ASSERT(!wlc_hw->sbclk || !si_taclear(wlc_hw->sih, TRUE));
		wl_intrson(wlc_hw->wlc->wl);
	}

	mac_intmask = wlc_intrsoff(wlc_hw->wlc);
	wlc_bmac_set_ctrl_ePA(wlc_hw);
	if (!WLCISACPHY(wlc_hw->band))
		wlc_bmac_set_btswitch(wlc_hw, wlc_hw->btswitch_ovrd_state);
	wlc_intrsrestore(wlc_hw->wlc, mac_intmask);

	/* Write WME tunable parameters for retransmit/max rate from wlc struct to ucode */
	for (ac = 0; ac < AC_COUNT; ac++) {
		wlc_bmac_write_shm(wlc_hw, M_AC_TXLMT_ADDR(ac), wlc_hw->wlc->wme_retries[ac]);
	}
	/* sanitize any existing scb rates */
	wlc_scblist_validaterates(wlc);
	/* ensure antenna config is up to date */
	wlc_stf_phy_txant_upd(wlc);

	wlc_hw->wlc->bcn_li_dtim = tmp_bcn_li_dtim;

	WL_INFORM(("wl%d: %s: back from wl_init()\n", wlc_hw->unit, __FUNCTION__));
	WL_NONE(("wl%d: %s: up %d, hw->up %d, sbclk %d, clk %d, hw->clk %d, fastclk %d\n",
	         wlc_hw->unit, __FUNCTION__, wlc_hw->wlc->pub->up, wlc_hw->up,
	         wlc_hw->sbclk, wlc_hw->wlc->clk, wlc_hw->clk, wlc_hw->forcefastclk));

	if (val & 8) {
		WL_INFORM(("wl%d: %s: calling wl_down()\n", wlc_hw->unit, __FUNCTION__));
		wl_down(wlc_hw->wlc->wl);

		WL_INFORM(("wl%d: %s: calling wl_up()\n", wlc_hw->unit, __FUNCTION__));
		wl_up(wlc_hw->wlc->wl);
	}
	WL_INFORM(("wl%d: %s(0x%x): done in %dmS\n", wlc_hw->unit, __FUNCTION__, val,
	           OSL_SYSUPTIME() - start));
}
#endif	/* BPRESET */

#ifndef BCM_OL_DEV
/** Returns 1 if any error is detected in TXFIFO configuration */
static bool
BCMINITFN(wlc_bmac_txfifo_sz_chk)(wlc_hw_info_t *wlc_hw)
{
	d11regs_t *regs = wlc_hw->regs;
	osl_t *osh;
	uint16 fifo_nu = 0;
	uint16 txfifo_cmd_org = 0;
	uint16 txfifo_cmd = 0;

	uint16 txfifo_def = 0;
	uint16 txfifo_def1 = 0;

	/* Index of "256 byte" block where this FIFO starts */
	uint16 txfifo_start = 0;
	/* Index of "256 byte" block where this FIFO ends */
	uint16 txfifo_end = 0;
	/* Number of "256 byte" blocks used so far */
	uint16 txfifo_used = 0;
	/* Total number of "256 byte" blocks available in chip */
	uint16 txfifo_total;
	bool err = 0;

	/* If MACHWCAP is not implemented this function cannot work */
	if (D11REV_LT(wlc_hw->corerev, 13)) {
		return 0;
	}

	osh = wlc_hw->osh;

	/* Adjust size as MACHWCAP gives size in "512 blocks" */
	txfifo_total = ((wlc_hw->machwcap & MCAP_TXFSZ_MASK) >> MCAP_TXFSZ_SHIFT) * 2;

	/* Store current value of xmtfifocmd for restoring later */
	txfifo_cmd_org = R_REG(osh, &regs->u.d11regs.xmtfifocmd);

	/* Read all configured FIFO size entries and check if they are valid */
	for (fifo_nu = 0; fifo_nu < NFIFO; fifo_nu++) {
		/* Select the FIFO */
		txfifo_cmd = ((txfifo_cmd_org & ~TXFIFOCMD_FIFOSEL_SET(-1)) |
			TXFIFOCMD_FIFOSEL_SET(fifo_nu));
		W_REG(osh, &regs->u.d11regs.xmtfifocmd, txfifo_cmd);

		/* Read the current configured size */
		txfifo_def = R_REG(osh, &regs->u.d11regs.xmtfifodef);
		if (D11REV_GE(wlc_hw->corerev, 16))
			txfifo_def1 = R_REG(osh, &regs->u.d11regs.xmtfifodef1);
		else
			txfifo_def1 = 0;

		/* Validate the size of the template fifo too */
		if (fifo_nu == 0) {
			if (TXFIFO_FIFO_START(txfifo_def, txfifo_def1) == 0) {
				WL_ERROR(("wl%d: %s: Template FIFO size is zero\n",
				          wlc_hw->unit, __FUNCTION__));
				ASSERT(0);
				err = 1;
				break;
			}

			/* End of template FIFO is just before start of fifo0 */
			txfifo_end = (TXFIFO_FIFO_START(txfifo_def, txfifo_def1) - 1);
			txfifo_used += ((txfifo_end - txfifo_start) + 1);
		}

		txfifo_start = TXFIFO_FIFO_START(txfifo_def, txfifo_def1);
		/* Check FIFO overlap with previous FIFO */
		if (txfifo_start < txfifo_end) {
			WL_ERROR(("wl%d: %s: FIFO %d overlaps with FIFO %d\n",
				wlc_hw->unit, __FUNCTION__, fifo_nu,
				((fifo_nu == 0) ? -1 : (fifo_nu-1))));
			ASSERT(0);
			err = 1;
			break;

		/* If consecutive blocks are not contiguous, this function cannot check overlap */
		} else if (txfifo_start != (txfifo_end + 1)) {
			WL_ERROR(("wl%d: %s: FIFO %d not contiguous with previous FIFO."
			"Cannot check overlap. (start=%d prev_end=%d)\n",
				wlc_hw->unit, __FUNCTION__, fifo_nu,
				txfifo_start, txfifo_end));
			ASSERT(0);
			err = 1;
			break;
		}
		txfifo_end = TXFIFO_FIFO_END(txfifo_def, txfifo_def1);
		/* Fifo should be configured to atleast 1 block */
		if (txfifo_end < txfifo_start) {
			WL_ERROR(("wl%d: %s: FIFO %d config invalid. start=%d and end=%d\n",
				wlc_hw->unit, __FUNCTION__, fifo_nu,
				txfifo_start, txfifo_end));
			ASSERT(0);
			err = 1;
			break;
		}
		txfifo_used += ((txfifo_end - txfifo_start) + 1);
		/* At any point, FIFO size used should not exceed capacity */
		if (txfifo_used > txfifo_total) {
			WL_ERROR(("wl%d: %s: FIFO %d config causes memblk usage %d"
			"to exceed chip capacity %d\n",
				wlc_hw->unit, __FUNCTION__, fifo_nu,
				txfifo_used, txfifo_total));
			ASSERT(0);
			err = 1;
			break;
		}
		WL_INFORM(("wl%d: %s: FIFO %d block config, "
		"start=%d end=%d sz=%d used=%d avail=%d\n",
			wlc_hw->unit, __FUNCTION__, fifo_nu,
			txfifo_start, txfifo_end,
			((txfifo_end - txfifo_start) + 1),
			txfifo_used, (txfifo_total - txfifo_used)));
	}
	/* Restore xmtfifocmd configuration */
	W_REG(osh, &regs->u.d11regs.xmtfifocmd, txfifo_cmd_org);

	return err;
}
#endif /* BCM_OL_DEV */

#if defined(BCMDBG) && !defined(BCMDBG_EXCLUDE_HW_TIMESTAMP)
char* wlc_dbg_get_hw_timestamp(void)
{
	static char timestamp[20];
	static uint32 nestcount = 0;

	if (nestcount == 0 && wlc_info_time_dbg)
	{
		struct bcmstrbuf b;
		uint32 t;
		uint32 mins;
		uint32 secs;
		uint32 fraction;
		bool   use_usec_timer = FALSE;

		nestcount++;

		/* use usec timer for revisions 26, 29 and revision 31 onwards */
		if (D11REV_GE(wlc_info_time_dbg->hw->corerev, 31) ||
			D11REV_IS(wlc_info_time_dbg->hw->corerev, 26) ||
			D11REV_IS(wlc_info_time_dbg->hw->corerev, 29))
		{
			use_usec_timer = TRUE;
		}

		if (use_usec_timer) {
			t = (R_REG(wlc_info_time_dbg->osh, &wlc_info_time_dbg->regs->usectimer));
		}
		else {
			t = (R_REG(wlc_info_time_dbg->osh, &wlc_info_time_dbg->regs->tsf_timerlow));
		}

		secs = t / 1000000;
		fraction = (t - secs*1000000 + 5) / 10;
		mins = secs / 60;
		secs -= mins * 60;

		bcm_binit(&b, timestamp, sizeof(timestamp));
		bcm_bprintf(&b, "[%d:%02d.%05d]:", mins, secs, fraction);

		nestcount--;
		return timestamp;
	}
	return "";
}
#endif /* BCMDBG && !BCMDBG_EXCLUDE_HW_TIMESTAMP */

#ifndef BCM_OL_DEV
static int
BCMINITFN(wlc_corerev_fifosz_validate)(wlc_hw_info_t *wlc_hw, uint16 *buf)
{
	int i = 0, err = 0;

	/* check txfifo allocations match between ucode and driver */
	buf[TX_AC_BE_FIFO] = wlc_bmac_read_shm(wlc_hw, M_FIFOSIZE0);
	if (buf[TX_AC_BE_FIFO] != wlc_hw->xmtfifo_sz[TX_AC_BE_FIFO]) {
		i = TX_AC_BE_FIFO;
		err = -1;
	}
	buf[TX_AC_VI_FIFO] = wlc_bmac_read_shm(wlc_hw, M_FIFOSIZE1);
	if (buf[TX_AC_VI_FIFO] != wlc_hw->xmtfifo_sz[TX_AC_VI_FIFO]) {
		i = TX_AC_VI_FIFO;
	        err = -1;
	}
	buf[TX_AC_BK_FIFO] = wlc_bmac_read_shm(wlc_hw, M_FIFOSIZE2);
	buf[TX_AC_VO_FIFO] = (buf[TX_AC_BK_FIFO] >> 8) & 0xff;
	buf[TX_AC_BK_FIFO] &= 0xff;
	if (buf[TX_AC_BK_FIFO] != wlc_hw->xmtfifo_sz[TX_AC_BK_FIFO]) {
		i = TX_AC_BK_FIFO;
	        err = -1;
	}
	if (buf[TX_AC_VO_FIFO] != wlc_hw->xmtfifo_sz[TX_AC_VO_FIFO]) {
		i = TX_AC_VO_FIFO;
		err = -1;
	}
	buf[TX_BCMC_FIFO] = wlc_bmac_read_shm(wlc_hw, M_FIFOSIZE3);
	buf[TX_ATIM_FIFO] = (buf[TX_BCMC_FIFO] >> 8) & 0xff;
	buf[TX_BCMC_FIFO] &= 0xff;
	if (buf[TX_BCMC_FIFO] != wlc_hw->xmtfifo_sz[TX_BCMC_FIFO]) {
		i = TX_BCMC_FIFO;
		err = -1;
	}
	if (buf[TX_ATIM_FIFO] != wlc_hw->xmtfifo_sz[TX_ATIM_FIFO]) {
		i = TX_ATIM_FIFO;
		err = -1;
	}
	if (err != 0) {
		WL_ERROR(("wlc_coreinit: txfifo mismatch: ucode size %d driver size %d index %d\n",
			buf[i], wlc_hw->xmtfifo_sz[i], i));
		/* DO NOT ASSERT corerev < 4 even there is a mismatch
		 * shmem, since driver don't overwrite those chip and
		 * ucode initialize data will be used.
		 */
		if (D11REV_GE(wlc_hw->corerev, 4))
			ASSERT(0);
	}

#ifdef WLAMPDU_HW
	for (i = 0; i < AC_COUNT; i++) {
		wlc_hw->xmtfifo_frmmax[i] =
		        (wlc_hw->xmtfifo_sz[i] * 256 - 1300) / MAX_MPDU_SPACE;
		WL_INFORM(("%s: fifo sz blk %d entries %d\n",
			__FUNCTION__, wlc_hw->xmtfifo_sz[i], wlc_hw->xmtfifo_frmmax[i]));
	}
#else
	BCM_REFERENCE(i);
#endif	/* WLAMPDU_HW */
	return err;
}

typedef struct _bmc_params {
	uint8	rxq_in_bm;	    /* 1: rx queues are allocated in BMC, 0: not */
	uint16	rxq0_buf;	    /* number of buffers (in 512 bytes) for rx queue 0
				     * if rxbmmap_is_en == 1, this number indicates
				     * the fifo boundary
				     */
	uint16	rxq1_buf;	    /* number of buffers for rx queue 1 */
	uint8	rxbmmap_is_en;	    /* 1: rx queues are managed as fifo, 0: not */
	uint8	tx_flowctrl_scheme; /* 1: new tx flow control scheme,
				     *	  don't preallocate as many buffers,
				     * 0: old scheme, preallocate
				     */
	uint16	full_thresh;	    /* used when tx_flowctrl_scheme == 0 */
	uint16	minbufs[];
} bmc_params_t;

static const bmc_params_t bmc_params_40 = {0, 0, 0, 0, 0, 11, {32, 32, 32, 32, 32, 8}};
static const bmc_params_t bmc_params_41 = {0, 0, 0, 0, 0, 6, {32, 32, 32, 32, 32, 0}};
static const bmc_params_t bmc_params_42 = {0, 0, 0, 0, 0, 11, {32, 32, 32, 32, 32, 32}};
static const bmc_params_t bmc_params_43 = {1, 40, 40, 0, 0, 11, {32, 32, 32, 32, 32, 32}};
static const bmc_params_t bmc_params_44 = {0, 0, 0, 0, 0, 6, {32, 32, 32, 32, 32, 0}};
static const bmc_params_t bmc_params_45 = {1, 20, 20, 0, 0, 6, {32, 32, 32, 32, 32, 0}};
static const bmc_params_t bmc_params_46 = {0, 0, 0, 0, 0, 6, {32, 32, 32, 32, 32, 0}};
/* corerev 47 uses bmc_params_45 */
static const bmc_params_t bmc_params_48 =
	{1, 128, 128, 1, 1, 0, {32, 32, 32, 32, 32, 0, 20, 0, 10}};
static const bmc_params_t bmc_params_49 =
	{1, 192, 192, 1, 1, 0, {32, 32, 32, 32, 10, 0, 92, 0, 10}};
static const bmc_params_t bmc_params_50 =
	{1, 128, 128, 1, 1, 0, {32, 32, 32, 32, 32, 0, 40, 0, 40}};
/* corerev 51 uses bmc_params_45 */
static const bmc_params_t bmc_params_54 =
	{1, 128, 128, 1, 1, 0, {16, 32, 32, 16, 8, 0, 96, 0, 10}};
/* corerev 64 uses bmc_params_64 */
static const bmc_params_t bmc_params_64 =
	{1, 128, 128, 1, 1, 0, {32, 32, 32, 32, 32, 0, 64, 0, 32}};

static const bmc_params_t *bmc_params = NULL;

static uint16 bmc_maxbufs;
static uint16 bmc_nbufs = D11MAC_BMC_MAXBUFS;

static void
wlc_bmac_bmc_template_allocstatus(wlc_hw_info_t *wlc_hw, uint32 mac_core_unit, int tplbuf)
{
	volatile uint16 *alloc_status;

	ASSERT(wlc_hw != NULL);
	ASSERT(wlc_hw->regs != NULL);

	if (mac_core_unit == MAC_CORE_UNIT_0) {
		alloc_status = (volatile uint16 *)
			&wlc_hw->regs->u.d11acregs.Core0BMCAllocStatusTID7;
	} else {
		alloc_status = (volatile uint16 *)
			&wlc_hw->regs->u.d11acregs.Core1BMCAllocStatusTID7;
	}

	SPINWAIT((R_REG(wlc_hw->osh, alloc_status) != tplbuf), 10);

	if (R_REG(wlc_hw->osh, alloc_status) != tplbuf) {
		WL_ERROR(("Error BMC buffer allocation: TID 7 of Core unit %d reg 0x%p val 0x%x",
		mac_core_unit, alloc_status, R_REG(wlc_hw->osh, alloc_status)));
	}
}

/** buffer manager initialisation */
static int
BCMINITFN(wlc_bmac_bmc_init)(wlc_hw_info_t *wlc_hw)
{
	osl_t *osh;
	d11regs_t *regs;
	uint32 bmc_ctl;
	uint16 maxbufs, minbufs, alloc_cnt, alloc_thresh, full_thresh, buf_desclen;
	int bmc_fifo_list[D11MAC_BMC_MAXFIFOS] = {7, 0, 1, 2, 3, 4, 5, 6, 8};
	int num_of_fifo, rxmapfifosz = 0;

	/* used for BMCCTL */
	uint8 bufsize = D11MAC_BMC_BUFSIZE_512BLOCK;
	uint8 loopback = 0;
	uint8 reset_stats = 0;
	uint8 init = 1;

	int i, fifo;
	uint32 fifo_sz;
	int tplbuf = D11MAC_BMC_TPL_NUMBUFS;
	uint32 bmc_startaddr = D11MAC_BMC_STARTADDR;
	uint8 doublebufsize = 0;

	int rxq0buf, rxq1buf;
	int rxq0_more_bufs = 0;
	uint8 clkgateen = 1;

	osh = wlc_hw->osh;
	regs = wlc_hw->regs;

	if (D11REV_IS(wlc_hw->corerev, 52) ||
	    D11REV_IS(wlc_hw->corerev, 51) ||
	    D11REV_IS(wlc_hw->corerev, 47) ||
	    D11REV_IS(wlc_hw->corerev, 45))
		bmc_params = &bmc_params_45;
	else if (D11REV_IS(wlc_hw->corerev, 54))
		bmc_params = &bmc_params_54;
	else if (D11REV_IS(wlc_hw->corerev, 64) ||
		D11REV_IS(wlc_hw->corerev, 65))
		bmc_params = &bmc_params_64;
	else if (D11REV_IS(wlc_hw->corerev, 50))
		bmc_params = &bmc_params_50;
	else if (D11REV_IS(wlc_hw->corerev, 49))
		bmc_params = &bmc_params_49;
	else if (D11REV_IS(wlc_hw->corerev, 48))
		bmc_params = &bmc_params_48;
	else if (D11REV_IS(wlc_hw->corerev, 46))
		bmc_params = &bmc_params_46;
	else if (D11REV_IS(wlc_hw->corerev, 44))
		bmc_params = &bmc_params_44;
	else if (D11REV_IS(wlc_hw->corerev, 43))
		bmc_params = &bmc_params_43;
	else if (D11REV_IS(wlc_hw->corerev, 42))
		bmc_params = &bmc_params_42;
	else if (D11REV_IS(wlc_hw->corerev, 41))
		bmc_params = &bmc_params_41;
	else if (D11REV_IS(wlc_hw->corerev, 40))
		bmc_params = &bmc_params_40;
	else {
		WL_ERROR(("corerev %d not supported\n", wlc_hw->corerev));
		ASSERT(0);
		return BCME_ERROR;
	}

	/* CRWLDOT11M-1160, impacts both revs 48, 49 */
	if (D11REV_IS(wlc_hw->corerev, 48) || D11REV_IS(wlc_hw->corerev, 49)) {

		/* Minimum region of TPL FIFO#7 */
		int tpl_fifo_sz = D11MAC_BMC_TPL_BUFS_BYTES;

		/* Need it to be 512B/buffer */
		bufsize = D11MAC_BMC_BUFSIZE_512BLOCK;

		/* start at 80KB, there are fewer buffers available for BMC use */
		bmc_startaddr = D11MAC_BMC_STARTADDR_SRASM;

#if defined(SAVERESTORE) && defined(SR_ESSENTIALS)
		/* When SR is disabled, allot the unused SR ASM space to RXQ0 FIFO */
		if (SR_ESSENTIALS_ENAB()) {
			/* Allot space in TPL FIFO#7 for 4KB aligned SR ASM */
			tpl_fifo_sz += D11MAC_BMC_SRASM_BYTES;
		} else
#endif /* (SAVERESTORE && SR_ESSENTIALS) */
		{
			/* Increase RXQ0 FIFO#6 by SR ASM unused space */
			rxq0_more_bufs = D11MAC_BMC_SRASM_NUMBUFS;
		}

		/* Number of 512 Bytes buffers for TPL FIFO#7 */
		tplbuf = D11MAC_BMC_BUFS_512(tpl_fifo_sz);
	} else if (D11REV_IS(wlc_hw->corerev, 50) || D11REV_IS(wlc_hw->corerev, 54) ||
	           wlc_bmac_rsdb_cap(wlc_hw)) {
		bufsize = D11MAC_BMC_BUFSIZE_256BLOCK;
	}

	if (bufsize == D11MAC_BMC_BUFSIZE_256BLOCK) {
		doublebufsize = 1;
		if (wlc_bmac_rsdb_cap(wlc_hw)) {
			/* Consider max/min to both core templates and sr array area */
			tplbuf = (si_numcoreunits(wlc_hw->sih, D11_CORE_ID) *
				D11MAC_BMC_TPL_NUMBUFS_PERCORE) + D11MAC_BMC_SR_NUMBUFS;
		} else {
			tplbuf = D11MAC_BMC_BUFS_256(D11MAC_BMC_TPL_BYTES);
		}
	}

	/* Derive from machwcap registers */
	fifo_sz = ((R_REG(osh, &regs->machwcap) & MCAP_TXFSZ_MASK) >> MCAP_TXFSZ_SHIFT) * 2048;

	/* Derive BMC memory size from configuration */
	if (D11REV_IS(wlc_hw->corerev, 64) || D11REV_IS(wlc_hw->corerev, 65)) {
		uint32 sysmem_size = si_sysmem_size(wlc_hw->sih);

		if (BUSTYPE(wlc_hw->sih->bustype) == SI_BUS) {
			/* Derive from RAMSIZE in dongle mode */
#ifdef RAMSIZE
			ASSERT(sysmem_size > RAMSIZE_ADJ);
			fifo_sz = sysmem_size - RAMSIZE_ADJ;
#endif // endif
		} else {
			/* Up to D11MAC_BMC_SYSMEM_MAX_BYTES for NIC mode */
			if (sysmem_size > D11MAC_BMC_SYSMEM_MAX_BYTES)
				fifo_sz = D11MAC_BMC_SYSMEM_MAX_BYTES;
			else
				fifo_sz = sysmem_size;
		}
	}

	/* Account for bmc_startaddr which is specified in units of 256B */
	bmc_maxbufs = (fifo_sz - (bmc_startaddr << 8)) >> (8 + bufsize);

	WL_INFORM(("wl%d: %s bmc_size 0x%x bufsize %d maxbufs %d start_addr 0x%04x\n",
		wlc_hw->unit, __FUNCTION__,
		fifo_sz, 1 << (8 + bufsize), bmc_maxbufs, (bmc_startaddr << 8)));

	if (bmc_params->rxq_in_bm) {
		rxq0buf = bmc_params->rxq0_buf;
		rxq1buf = bmc_params->rxq1_buf;

		WL_INFORM(("wl%d: %s rxq_in_bm ON. rxbmmap is %s. "
			"RXQ size/ptr below are in 32-bit DW.\n",
			wlc_hw->unit, __FUNCTION__,
			bmc_params->rxbmmap_is_en ? "enabled" : "disabled"));

		if (bmc_params->rxbmmap_is_en) {  /* RXBMMAP is enabled */
			/* Convert to word addresses, num of buffer * 512 / 4 */
			W_REG(osh, &regs->u_rcv.d11acregs.rcv_bm_sp_q0, 0);
			W_REG(osh, &regs->u_rcv.d11acregs.rcv_bm_ep_q0, (rxq0buf << 7) - 1);

			W_REG(osh, &regs->u_rcv.d11acregs.rcv_bm_sp_q1, rxq0buf << 7);
			W_REG(osh, &regs->u_rcv.d11acregs.rcv_bm_ep_q1,
				((rxq0buf + rxq1buf) << 7) - 1);

			WL_INFORM(("wl%d: RXQ-0 size 0x%04x start_ptr 0x0000 end_ptr 0x%04x\n",
				wlc_hw->unit, (rxq0buf << 7), ((rxq0buf << 7) - 1)));
			WL_INFORM(("wl%d: RXQ-1 size 0x%04x start_ptr 0x%04x end_ptr 0x%04x\n",
				wlc_hw->unit, (rxq1buf << 7),
				(rxq0buf << 7), (((rxq0buf + rxq1buf) << 7) - 1)));
		} else {
			/* This corresponds to the case where rxbmmap
			 * is not present/disabled/passthru
			 * Convert to word addresses, num of buffer * 512 / 4
			 */
			W_REG(osh, &regs->u_rcv.d11acregs.rcv_bm_sp_q0, tplbuf << 7);
			W_REG(osh, &regs->u_rcv.d11acregs.rcv_bm_ep_q0,
				((tplbuf + rxq0buf) << 7) - 1);

			W_REG(osh, &regs->u_rcv.d11acregs.rcv_bm_sp_q1, (tplbuf + rxq0buf) << 7);
			W_REG(osh, &regs->u_rcv.d11acregs.rcv_bm_ep_q1,
				((tplbuf + rxq0buf + rxq1buf) << 7) - 1);

			tplbuf += rxq0buf + rxq1buf;

			WL_INFORM(("wl%d: RXQ-0 size 0x%04x start_ptr 0x%04x end_ptr 0x%04x\n",
				wlc_hw->unit, (rxq0buf << 7),
				(tplbuf << 7), (((tplbuf +rxq0buf) << 7) - 1)));
			WL_INFORM(("wl%d: RXQ-1 size 0x%04x start_ptr 0x%0x4 end_ptr 0x%04x\n",
				wlc_hw->unit, (rxq1buf << 7), ((tplbuf + rxq0buf) << 7),
				(((tplbuf + rxq0buf + rxq1buf) << 7) - 1)));
		}

		/* Reset the RXQs to have the pointers take effect;resets are self-clearing */
		W_REG(osh, &regs->rcv_fifo_ctl, 0x101);	/* sel and reset q1 */
		W_REG(osh, &regs->rcv_fifo_ctl, 0x001);	/* sel and reset q0 */
	}

	/* init the total number for now */
	bmc_nbufs = bmc_maxbufs;
	W_REG(osh, &regs->u.d11acregs.BMCConfig, bmc_nbufs);
	bmc_ctl = (loopback << BMCCTL_LOOPBACK_SHIFT)	|
	        (bufsize << BMCCTL_TXBUFSIZE_SHIFT)	|
	        (reset_stats << BMCCTL_RESETSTATS_SHIFT)|
	        (init << BMCCTL_INITREQ_SHIFT);

	/*
	 * Enable hardware clock gating for BM memories, only for 4350A0/B0/B1.
	 * The memory wrapper in BM don't have the chip select clock gate feature, increasing
	 * Tx/Rx currents -- setting it to 1 to enable hardware clock gating. Code is conditioned
	 * to MAC rev 43 only, as 4350C0 will handle this in different manner.
	 */
	if (D11REV_IS(wlc_hw->corerev, 43))
		bmc_ctl |= (clkgateen << BMCCTL_CLKGATEEN_SHIFT);

	W_REG(osh, &regs->u.d11acregs.BMCCTL, bmc_ctl);
#ifndef BCMQT
	SPINWAIT((R_REG(wlc_hw->osh, &regs->u.d11acregs.BMCCTL) & BMC_CTL_DONE), 200);
#else
	SPINWAIT((R_REG(wlc_hw->osh, &regs->u.d11acregs.BMCCTL) & BMC_CTL_DONE), 200000);
#endif /* BCMQT */
	if (R_REG(wlc_hw->osh, &regs->u.d11acregs.BMCCTL) & BMC_CTL_DONE) {
		WL_ERROR(("wl%d: bmc init not done yet :-(\n", wlc_hw->unit));
	}

	buf_desclen = ((D11AC_TXH_LEN - DOT11_FCS_LEN - AMPDU_DELIMITER_LEN)
		       << BMCDescrLen_LongLen_SHIFT)
		| (D11AC_TXH_SHORT_LEN - DOT11_FCS_LEN - AMPDU_DELIMITER_LEN);

	if (bmc_params->rxbmmap_is_en) {
		num_of_fifo = 9;
		WL_INFORM(("wl%d: fifo 0-5: tx-fifos, fifo 7: template; "
			"fifo 6/8: rx-fifos\n", wlc_hw->unit));
	} else {
		num_of_fifo = 7;
		WL_INFORM(("wl%d: fifo 0-5: tx-fifos, fifo 7: template; ", wlc_hw->unit));
	}

	WL_INFORM(("wl%d: \t maxbuf\t minbuf\t fullthr alloccnt allocthr\n", wlc_hw->unit));

	for (i = 0; i < num_of_fifo; i++) {
		fifo = bmc_fifo_list[i];
		/* configure per-fifo parameters and enable them one fifo by fifo
		 * always init template first to guarantee template start from first buffer
		 */
		if (fifo == D11MAC_BMC_TPL_IDX) {
			maxbufs = (uint16)tplbuf;
			minbufs = maxbufs;
			full_thresh = maxbufs;
			alloc_cnt = minbufs;
			alloc_thresh = alloc_cnt - 4;
		} else {
			if (fifo == 6 || fifo == 8) {	/* rx fifo */
				int rxq0_bufs = bmc_params->minbufs[fifo] << doublebufsize;

				if (fifo == 6)
					rxq0_bufs += rxq0_more_bufs; /* any unused SR ASM space */

				maxbufs = rxq0_bufs + 3;
				minbufs = rxq0_bufs + 3;

				rxmapfifosz = rxq0_bufs;
				W_REG(osh, &regs->u.d11acregs.RXMapFifoSize, rxmapfifosz);
			} else {
				maxbufs = bmc_nbufs - tplbuf;
				minbufs = bmc_params->minbufs[fifo] << doublebufsize;
			}

			if (bmc_params->tx_flowctrl_scheme == 0) {
				full_thresh = bmc_params->full_thresh;
				alloc_cnt = 2 * full_thresh;
				alloc_thresh = alloc_cnt - 4;
			} else {
				full_thresh = 1;
				alloc_cnt = 2;
				alloc_thresh = 2;
			}
		}

		if (fifo == 6 || fifo == 8)
			WL_INFORM(("fifo %d:  %d \t %d \t %d \t %d \t %d\t "
				"rx-fifo buffer cnt: %d\n",
				fifo, maxbufs, minbufs, full_thresh, alloc_cnt, alloc_thresh,
				rxmapfifosz));
		else
			WL_INFORM(("fifo %d:  %d \t %d \t %d \t %d \t %d\n",
				fifo, maxbufs, minbufs, full_thresh, alloc_cnt, alloc_thresh));

		W_REG(osh, &regs->u.d11acregs.BMCMaxBuffers, maxbufs);
		W_REG(osh, &regs->u.d11acregs.BMCMinBuffers, minbufs);
		W_REG(osh, &regs->u.d11acregs.XmtFIFOFullThreshold, full_thresh);
		if (D11REV_IS(wlc_hw->corerev, 50) || D11REV_GT(wlc_hw->corerev, 52)) {
			/* XXX: Rev50 || Rev>52;
			* Refer	http://hwnbu-twiki.sj.broadcom.com/bin/view/Mwgroup/Dot11macRevMap
			*	BMCAllocCtl.AllocCount [0:10]
			*	BMCAllocCtl.AllocThreshold [11:14]
			*/
			W_REG(osh, &regs->u.d11acregs.BMCAllocCtl,
			(alloc_thresh << BMCAllocCtl_AllocThreshold_SHIFT_Rev50) | alloc_cnt);

			/* If the MSDUINDEXFIFO for a given TID,
			 * has fewer entries then the buffer arbiter doesn't grant requests
			 */
			W_REG(osh, &regs->u.d11acregs.MsduThreshold, 0x8);
		} else {
			W_REG(osh, &regs->u.d11acregs.BMCAllocCtl,
			(alloc_thresh << BMCAllocCtl_AllocThreshold_SHIFT) | alloc_cnt);
		}
		W_REG(osh, &regs->u.d11acregs.BMCDescrLen, buf_desclen);

		/* Enable this fifo */
		W_REG(osh, &regs->u.d11acregs.BMCCmd, fifo | (1 << BMCCmd_Enable_SHIFT));

		if (D11REV_IS(wlc_hw->corerev, 50) || D11REV_GE(wlc_hw->corerev, 54)) {
			if (fifo == D11MAC_BMC_TPL_IDX) {
				wlc_bmac_bmc_template_allocstatus(wlc_hw, MAC_CORE_UNIT_0, tplbuf);
			}
		}

		if (RSDB_ENAB(wlc_hw->wlc->pub)) {
			/* 4349 . Set maccore_sel to 1 for Core 1 */
			if (wlc_bmac_rsdb_cap(wlc_hw) &&
				(si_coreunit(wlc_hw->sih) == 0) &&
				(wlc_rsdb_mode(wlc_hw->wlc) == PHYMODE_RSDB)) {
				W_REG(osh, &regs->u.d11acregs.BMCCmd,
					fifo | (1 << 10) | (1 <<  BMCCmd_Enable_SHIFT));
				if (fifo == D11MAC_BMC_TPL_IDX) {
					wlc_bmac_bmc_template_allocstatus(wlc_hw,
					MAC_CORE_UNIT_1, tplbuf);
				}
			}
		}
	}

	if (wlc_bmac_rsdb_cap(wlc_hw)) {
		tplbuf =  D11MAC_BMC_TPL_NUMBUFS_PERCORE;
	}

	/* init template */
	for (i = 0; i < tplbuf; i ++) {
		int end_idx = i + 2 + doublebufsize;

		if (end_idx >= tplbuf)
			end_idx = tplbuf - 1;
		W_REG(osh, &regs->u.d11acregs.MSDUEntryStartIdx, i);
		W_REG(osh, &regs->u.d11acregs.MSDUEntryEndIdx, end_idx);
		W_REG(osh, &regs->u.d11acregs.MSDUEntryBufCnt, end_idx - i + 1);
		W_REG(osh, &regs->u.d11acregs.PsmMSDUAccess,
		      ((1 << PsmMSDUAccess_WriteBusy_SHIFT) |
		       (i << PsmMSDUAccess_MSDUIdx_SHIFT) |
		       (D11MAC_BMC_TPL_IDX << PsmMSDUAccess_TIDSel_SHIFT)));

		SPINWAIT((R_REG(wlc_hw->osh, &regs->u.d11acregs.PsmMSDUAccess) &
			(1 << PsmMSDUAccess_WriteBusy_SHIFT)), 200);
		if (R_REG(wlc_hw->osh, &regs->u.d11acregs.PsmMSDUAccess) &
		    (1 << PsmMSDUAccess_WriteBusy_SHIFT))
			{
				WL_ERROR(("wl%d: PSM MSDU init not done yet :-(\n", wlc_hw->unit));
			}
	}

	wlc_rsdb_bmc_smac_template(wlc_hw->wlc, tplbuf, doublebufsize);

	WL_INFORM(("wl%d: bmc_init done\n", wlc_hw->unit));
	return 0;
}

#endif /* BMCPCIDEV */

#if defined(BCM_OL_DEV) && defined(BCMDBG)
#define DUMP_BUFFER_SIZE	2048
void
wlc_bmac_dump_dma(wlc_hw_info_t *wlc_hw)
{
	char *buf;
	struct bcmstrbuf b;

	buf = MALLOC(wlc_hw->osh, DUMP_BUFFER_SIZE);
	if (buf != NULL) {
		printf("dump RxFIFO %d DMA\n", RX_FIFO);
		bcm_binit(&b, buf, DUMP_BUFFER_SIZE);
		dma_dumprx(wlc_hw->di[RX_FIFO], &b, TRUE);
		printbig(buf);
		printf("dump TxFIFO %d DMA\n", TX_ATIM_FIFO);
		bcm_binit(&b, buf, DUMP_BUFFER_SIZE);
		dma_dumptx(wlc_hw->di[TX_ATIM_FIFO], &b, TRUE);
		printbig(buf);
		MFREE(wlc_hw->osh, buf, DUMP_BUFFER_SIZE);
	}
}
#endif /* BCM_OL_DEV && BCMDBG */

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int
wlc_bmac_bmc_dump_parse_args(wlc_info_t *wlc, bool *init)
{
	int err = BCME_OK;
	char *args = wlc->dump_args;
	char *p, **argv = NULL;
	uint argc = 0;
	char opt;

	if (args == NULL || init == NULL) {
		err = BCME_BADARG;
		goto exit;
	}

	/* allocate argv */
	if ((argv = MALLOC(wlc->osh, sizeof(*argv) * DUMP_BMC_ARGV_MAX)) == NULL) {
		WL_ERROR(("wl%d: %s: failed to allocate the argv buffer\n",
		          wlc->pub->unit, __FUNCTION__));
		goto exit;
	}

	/* get each token */
	p = bcmstrtok(&args, " ", 0);
	while (p && argc < DUMP_BMC_ARGV_MAX-1) {
		argv[argc++] = p;
		p = bcmstrtok(&args, " ", 0);
	}
	argv[argc] = NULL;

	/* initial default */
	*init = FALSE;

	/* parse argv */
	argc = 0;
	while ((p = argv[argc++])) {
		if (!strncmp(p, "-", 1)) {
			if (strlen(p) > 2) {
				err = BCME_BADARG;
				goto exit;
			}
			opt = p[1];

			switch (opt) {
				case 'i':
					if (D11REV_GE(wlc->hw->corerev, 47))
						*init = TRUE;
					else
						err = BCME_UNSUPPORTED;
					break;
				default:
					err = BCME_BADARG;
					goto exit;
			}
		} else {
			err = BCME_BADARG;
			goto exit;
		}
	}

exit:
	if (argv) {
		MFREE(wlc->osh, argv, sizeof(*argv) * DUMP_BMC_ARGV_MAX);
	}

	return err;
}

static int
wlc_bmac_bmc_dump(wlc_hw_info_t *wlc_hw, struct bcmstrbuf *b)
{
	osl_t *osh;
	d11regs_t *regs;
	uint16 nbuf[12]; /* 12 -> number of BMCStatCtl sel options */
	int i, j, fifonum;
	bool	init = FALSE;
	uint16 tmp0, tmp1, bmccmd1;
	int fifo;
	int bmc_fifo_list[D11MAC_BMC_MAXFIFOS] = {7, 0, 1, 2, 3, 4, 5, 6, 8};
	int err = BCME_OK;

	osh = wlc_hw->osh;
	regs = wlc_hw->regs;

	if (!wlc_hw->clk)
		return BCME_NOCLK;

	if (D11REV_LT(wlc_hw->corerev, 40)) {
		return BCME_UNSUPPORTED;
	}

	fifonum = (D11REV_GE(wlc_hw->corerev, 48) && !D11REV_IS(wlc_hw->corerev, 51)) ? 9 : 6;
	if (D11REV_GE(wlc_hw->corerev, 64)) {
		bcm_bprintf(b, "BcmReadStatus 0x%04x rqPrio 0x%x BmcCmd 0x%x "
			"psm_reg_mux 0x%x AQMQMAP 0x%x AQMFifo_Status 0x%x\n",
			R_REG(osh, &regs->u.d11acregs.BMCReadStatus),
			R_REG(osh, &regs->u.d11acregs.XmtFifoRqPrio),
			R_REG(osh, &regs->u.d11acregs.BMCCmd),
			R_REG(osh, &regs->u.d11acregs.psm_reg_mux),
			R_REG(osh, &regs->u.d11acregs.AQMQMAP),
			R_REG(osh, &regs->u.d11acregs.AQMFifo_Status));
	} else {
		bcm_bprintf(b, "BcmReadStatus 0x%04x rqPrio 0x%x BmcCmd 0x%x "
			"FifoRdy 0x%x FrmCnt 0x%x\n",
			R_REG(osh, &regs->u.d11acregs.BMCReadStatus),
			R_REG(osh, &regs->u.d11acregs.XmtFifoRqPrio),
			R_REG(osh, &regs->u.d11acregs.BMCCmd),
			R_REG(osh, &regs->u.d11acregs.u0.lt64.AQMFifoReady),
			R_REG(osh, &regs->u.d11acregs.XmtFifoFrameCnt));
	}

	bcm_bprintf(b, "BMC stats: 0-nfrm 1-nbufRecvd 2-nbuf2DMA 3-nbufMax 4-nbufUse 5-nbufMin\n"
		       "       6-nFree 7-nDrqPush 8-nDrqPop 9-nDdqPush 10-nDdqPop 11-nOccupied\n");
	bcm_bprintf(b, "---------------------------------------------------------------------\n");
	bcm_bprintf(b, "          [0]  [1]  [2]  [3]  [4]  [5]  [6]  [7]  [8]   [9] [10] [11]\n");

	for (i = 0; i < fifonum; i++) {
		/* skip template */
		if (i == 7) {
			continue;
		}
		for (j = 0; j < 12; j++) {
			W_REG(osh, &regs->u.d11acregs.BMCStatCtl, ((j << 4) | i));
			nbuf[j] = R_REG(osh, &regs->u.d11acregs.BMCStatData);
		}
		bcm_bprintf(b, "fifo-%d :", i);
		for (j = 0; j < 12; j++) {
			bcm_bprintf(b, " %4x", nbuf[j]);
		}
		bcm_bprintf(b, "\n");
	}

	/* Parse args if needed */
	if (wlc_hw->wlc->dump_args) {
		err = wlc_bmac_bmc_dump_parse_args(wlc_hw->wlc, &init);
		if (err != BCME_OK)
			return err;
	}

	if (init) {
		tmp0 = R_REG(osh, &regs->u.d11acregs.BMCConfig) & BMCCONFIG_BUFCNT_MASK;
		tmp1 = R_REG(osh, &regs->u.d11acregs.BMCCTL) & (1 << BMCCTL_TXBUFSIZE_SHIFT);
		bcm_bprintf(b, "\nBMC bufsize %d maxbufs %d start_addr 0x%04x\n",
			(1 << (8 + (tmp1 >> BMCCTL_TXBUFSIZE_SHIFT))), tmp0,
			(R_REG(osh, &regs->u.d11acregs.BMCStartAddr) & BMCSTARTADDR_STRTADDR_MASK));

		/* dump RX queue init data */
		if (bmc_params->rxq_in_bm) {
			bcm_bprintf(b, "rxq_in_bm ON. rxbmmap is %s. "
				"RXQ size/ptr below are in 32-bit DW.\n",
				bmc_params->rxbmmap_is_en ? "enabled" : "disabled");

			tmp0 = R_REG(osh, &regs->u_rcv.d11acregs.rcv_bm_sp_q0);
			tmp1 = R_REG(osh, &regs->u_rcv.d11acregs.rcv_bm_ep_q0);
			bcm_bprintf(b, "RXQ-0 size 0x%04x start_ptr 0x%04x end_ptr 0x%04x\n",
				(tmp1 - tmp0 +1), tmp0, tmp1);
			tmp0 = R_REG(osh, &regs->u_rcv.d11acregs.rcv_bm_sp_q1);
			tmp1 = R_REG(osh, &regs->u_rcv.d11acregs.rcv_bm_ep_q1);
			bcm_bprintf(b, "RXQ-1 size 0x%04x start_ptr 0x%04x end_ptr 0x%04x\n",
				(tmp1 - tmp0 +1), tmp0, tmp1);
		}

		/* dump all fifos init data */
		if (fifonum == 9) {
			bcm_bprintf(b, "fifo 0-5: tx-fifos, fifo 7: template; "
				"fifo 6/8: rx-fifos\n");
		} else {
			bcm_bprintf(b, "fifo 0-5: tx-fifos, fifo 7: template;\n");
		}

		tmp1 = R_REG(osh, &regs->u.d11acregs.BMCAllocCtl);
		if (D11REV_IS(wlc_hw->corerev, 50) || D11REV_GT(wlc_hw->corerev, 52)) {
			tmp0 = tmp1 & ((1 <<BMCAllocCtl_AllocThreshold_SHIFT_Rev50)-1);
			tmp1 = tmp1 >> BMCAllocCtl_AllocThreshold_SHIFT_Rev50;
		} else {
			tmp0 = tmp1 & ((1 << BMCAllocCtl_AllocThreshold_SHIFT)-1);
			tmp1 = tmp1 >> BMCAllocCtl_AllocThreshold_SHIFT;
		}
		bcm_bprintf(b, "\t xmt_fifo_full_thr %d alloc_cnt %d alloc_thr %d\n",
			R_REG(osh, &regs->u.d11acregs.XmtFIFOFullThreshold), tmp0, tmp1);

		wlc_bmac_suspend_mac_and_wait(wlc_hw);
		bmccmd1 = R_REG(osh, &regs->u.d11acregs.BMCCmd1);

		bcm_bprintf(b, "\t maxbuf\t minbuf  \n");
		for (i = 0; i < fifonum; i++) {
			fifo = bmc_fifo_list[i];

			W_REG(osh, &regs->u.d11acregs.BMCCmd1, ((bmccmd1 &
				~(0xf << BMCCMD1_TIDSEL_SHIFT)) | (fifo << BMCCMD1_TIDSEL_SHIFT)
				| (0x3 << BMCCMD1_RDSRC_SHIFT)));

			if (fifo == 6 || fifo == 8)
				bcm_bprintf(b, "fifo %d:  %d \t %d \trx-fifo buffer cnt: %d\n",
					fifo, R_REG(osh, &regs->u.d11acregs.BMCMaxBuffers),
					R_REG(osh, &regs->u.d11acregs.BMCMinBuffers),
					R_REG(osh, &regs->u.d11acregs.RXMapFifoSize));
			else
				bcm_bprintf(b, "fifo %d:  %d \t %d\n",
					fifo, R_REG(osh, &regs->u.d11acregs.BMCMaxBuffers),
					R_REG(osh, &regs->u.d11acregs.BMCMinBuffers));
		}

		W_REG(osh, &regs->u.d11acregs.BMCCmd1, bmccmd1);
		wlc_bmac_enable_mac(wlc_hw);
	}

	return BCME_OK;
}
#endif /* BCMDBG || BCMDBG_DUMP || BCMDBG_PHYDUMP */

#if defined(BCMDBG) || defined(WLMCHAN) || defined(SRHWVSDB)
void
wlc_bmac_tsf_adjust(wlc_hw_info_t *wlc_hw, uint32 delta)
{
	wlc_info_t *wlc = wlc_hw->wlc;
	wlc_tsf_adjust(wlc, delta);
}
#endif /* BCMDBG || WLMCHAN || SRHWVSDB */

void
wlc_bmac_update_bt_chanspec(wlc_hw_info_t *wlc_hw,
	chanspec_t chanspec, bool scan_in_progress, bool roam_in_progress)
{
#ifdef BCMECICOEX
	si_t *sih = wlc_hw->sih;

	if (BCMCOEX_ENAB_BMAC(wlc_hw) && !scan_in_progress && !roam_in_progress) {
		/* Inform BT about the channel change if we are operating in 2Ghz band */
		if (chanspec && CHSPEC_IS2G(chanspec)) {
			NOTIFY_BT_CHL(sih, CHSPEC_CHANNEL(chanspec));
			if (CHSPEC_IS40(chanspec))
				NOTIFY_BT_BW_40(sih);
			else
				NOTIFY_BT_BW_20(sih);
		} else if (chanspec && CHSPEC_IS5G(chanspec))
			NOTIFY_BT_CHL(sih, 0xf);
		else
			NOTIFY_BT_CHL(sih, 0);
	}
#endif /* BCMECICOEX */
}

int wlc_bmac_is_singleband_5g(unsigned int device)
{
	return (_IS_SINGLEBAND_5G(device));
}

int wlc_bmac_srvsdb_force_set(wlc_hw_info_t *wlc_hw, uint8 force)
{
	wlc_hw->sr_vsdb_force = force;
	return BCME_OK;
}

#ifndef BCM_OL_DEV
/*
Function to set input mac address in SHM for ucode generated CTS2SELF. The
Mac addresses are written out 2 bytes at a time at the specific SHM location.
For non-AC chips this mac address was retrieved from the RCMTA by ucode
directly. For AC chips there is a bug that prevents access to the search
engine by ucode. For CTS packets (normal and CTS2SELF), the mac address is
bit-substituted before transmission. So we use the address set in this SHM
location for CTS2SELF packets.
* GE40 only
*/
void
wlc_bmac_set_myaddr(wlc_hw_info_t *wlc_hw, struct ether_addr *mac_addr)
{
	unsigned short mac;

	mac = ((mac_addr->octet[1]) << 8) | mac_addr->octet[0];
	wlc_bmac_write_shm(wlc_hw, M_MYMAC_ADDR_L, mac);
	mac = ((mac_addr->octet[3]) << 8) | mac_addr->octet[2];
	wlc_bmac_write_shm(wlc_hw, M_MYMAC_ADDR_M, mac);
	mac = ((mac_addr->octet[5]) << 8) | mac_addr->octet[4];
	wlc_bmac_write_shm(wlc_hw, M_MYMAC_ADDR_H, mac);
}
#endif /* BCM_OL_DEV */

/* This function attempts to drain A2DP buffers in BT before granting the antenna to Wl for
various calibrations, etc. This can only be done for ECI supported chips (including GCI) since
task and buffer count information is needed. It is also assumed that the mac is suspended when
this function is called. This function does the following:
	- Grant the antenna to BT (ANTSEL and TXCONF set to 0)
	- If the BT task type is A2DP and the buffer count is non-zero wait for up to 50 ms
	until the buffer count becomes zero.
	- If the task type is not A2DP or the buffer count is zero, exit the wait loop
	- If BT RF Active is asserted, wait for up to 5 ms for it to de-assert after setting
	TXCONF to 1 (don't grant to BT).
This functionality has been moved out of common PHY code since it is mac-related.
*/
#define BTCX_FLUSH_WAIT_MAX_MS  50
void
wlc_bmac_coex_flush_a2dp_buffers(wlc_hw_info_t *wlc_hw)
{
#if defined(BCMECICOEX) && defined(WLC_LOW)
	int delay_val;
	uint16 eci_m = 0;
	uint16 a2dp_buffer = 0;
	uint16 bt_task = 0;

	if (BCMCOEX_ENAB_BMAC(wlc_hw)) {
		/* Ucode better be suspended when we mess with BTCX regs directly */
		ASSERT(!(R_REG(wlc_hw->osh, &wlc_hw->regs->maccontrol) & MCTL_EN_MAC));

		OR_REG(wlc_hw->osh, &wlc_hw->regs->u.d11regs.btcx_ctrl,
			BTCX_CTRL_EN | BTCX_CTRL_SW);
		/* Set BT priority and antenna to allow A2DP to catch up
		* TXCONF is active low, granting BT to TX/RX, and ANTSEL=0 for
		* BT, so clear both bits to 0.
		* microcode is already suspended, so no one can change these bits
		*/
		AND_REG(wlc_hw->osh, &wlc_hw->regs->u.d11regs.btcx_trans_ctrl,
			~(BTCX_TRANS_TXCONF | BTCX_TRANS_ANTSEL));
		/* Wait for A2DP to flush all pending data.
		* Since some of these bits are over-loaded it is best to ensure that the task
		* type is A2DP.
		* In GCI, A2DP buffer count is at ECI[27:24], BT Task Type is in
		* ECI[21:16] (both are in word 1)
		* Non-GCI has A2DP buffer count in ECI[23:20] (word 1), BT Task type is in
		* ECI[8:3] (word 0),
		*/
		for (delay_val = 0; delay_val < BTCX_FLUSH_WAIT_MAX_MS * 10; delay_val++) {
			if (BCMGCICOEX_ENAB_BMAC(wlc_hw)) {
				/* In GCI, both task type and buffer count are available
				   in the same word
				*/
				W_REG(wlc_hw->osh, &wlc_hw->regs->u.d11regs.btcx_eci_addr, 1);
				eci_m = R_REG(wlc_hw->osh, &wlc_hw->regs->u.d11regs.btcx_eci_data);
				a2dp_buffer = ((eci_m >> 8) & 0xf);
				bt_task = eci_m  & 0x3f;
			} else {
				W_REG(wlc_hw->osh, &wlc_hw->regs->u.d11regs.btcx_eci_addr, 0);
				eci_m = R_REG(wlc_hw->osh, &wlc_hw->regs->u.d11regs.btcx_eci_data);
				bt_task = (eci_m >> 4)  & 0x3f;
				W_REG(wlc_hw->osh, &wlc_hw->regs->u.d11regs.btcx_eci_addr, 1);
				eci_m = R_REG(wlc_hw->osh, &wlc_hw->regs->u.d11regs.btcx_eci_data);
				a2dp_buffer = ((eci_m >> 4) & 0xf);
			}
			if (((bt_task == 4) && (a2dp_buffer == 0)) ||
			    (bt_task != 4)) {
				/* All A2DP data is flushed  or not A2DP */
				goto pri_wlan;
			}
			OSL_DELAY(100);
		}
		if (delay_val == (BTCX_FLUSH_WAIT_MAX_MS * 10)) {
			WL_ERROR(("wl%d: %s: A2DP flush failed, eci_m: 0x%x\n",
				wlc_hw->unit, __FUNCTION__, eci_m));
		}

pri_wlan:
		/* Reenable WLAN priority, and then wait for BT to finish */
		OR_REG(wlc_hw->osh, &wlc_hw->regs->u.d11regs.btcx_trans_ctrl, BTCX_TRANS_TXCONF);
		delay_val = 0;
		/* While RF_ACTIVE is asserted... */
		while (R_REG(wlc_hw->osh, &wlc_hw->regs->u.d11regs.btcx_stat) & BTCX_STAT_RA) {
			if (delay_val++ > BTCX_FLUSH_WAIT_MAX_MS) {
				WL_ERROR(("wl%d: %s: BT still active\n",
					wlc_hw->unit, __FUNCTION__));
				break;
			}
			OSL_DELAY(100);
		}
	}
#endif /* BCMECICOEX && WLC_LOW */
}

#ifdef NOT_RIGGED_UP_YET
/* Macmode / Phymode / Opmode are used interchangebly some-
 * times even though they are one and the same. Going ahead
 * with HW signal name - using phymode here on (even though
 * its a misnomer).
 */
static int
wlc_bmac_switch_phymode(wlc_hw_info_t *wlc_hw, uint16 requested_phymode)
{
	uint32 phymode = 0x0;

	/* No need to do anything if requested and current phymode are same */
	if (requested_phymode == phy_get_phymode((phy_info_t *)wlc_hw->band->pi))
		return BCME_OK;
	else if ((requested_phymode != PHYMODE_RSDB) ||
		(requested_phymode != PHYMODE_RSDB_SISO_0) ||
		(requested_phymode != PHYMODE_RSDB_SISO_1) ||
		(requested_phymode != PHYMODE_MIMO) ||
		(requested_phymode != PHYMODE_80P80))
		return BCME_UNSUPPORTED;

	phymode = (uint32)requested_phymode << SICF_PHYMODE_SHIFT;

	/* put phy in reset */
	wlc_bmac_core_phy_clk(wlc_hw, OFF);

	/* doing phymode switch */
	si_core_cflags(wlc_hw->sih, SICF_PHYMODE, phymode);

	/* bring phy out of reset */
	wlc_bmac_core_phy_clk(wlc_hw, ON);

	phy_set_phymode((phy_info_t *)wlc_hw->band->pi, requested_phymode);

	return BCME_OK;
}
#endif /* WLRSDB */

/* In MIMO mode, some registers that need to path registers are still common registers in 4349A0.
 * However, due to the RSDB support, there are two copies of the common registers, one for
 * each core. This can be exploited to write to the shadow copy of the common register for each
 * of the cores. The workaround to do that is by enabling a bit in PHYMODE register, and this
 * results in a write to common register only writing the core0 copy. The following function
 * implements this workaround of writing to the specific bit in the PHYMODE register.
 */
void
wlc_bmac_exclusive_reg_access_core0(wlc_hw_info_t *wlc_hw, bool set)
{
	uint32 phymode = (si_core_cflags(wlc_hw->sih, 0, 0) & SICF_PHYMODE) >> SICF_PHYMODE_SHIFT;

	ASSERT(phy_get_phymode((phy_info_t *)wlc_hw->band->pi) == PHYMODE_MIMO);

	if (set)
		phymode |= SUPPORT_EXCLUSIVE_REG_ACCESS_CORE0;
	else
		phymode &= ~SUPPORT_EXCLUSIVE_REG_ACCESS_CORE0;

	si_core_cflags(wlc_hw->sih, SICF_PHYMODE, phymode << SICF_PHYMODE_SHIFT);
}

/*
This routine, dumps the contents of the BT registers and memory.
To access BT register, we use interconnect registers.
These registers  are at offset 0xd0, 0xd4, 0xe0 and 0xd8.
Backplane addresses low and high are at offset 0xd0 and 0xd4 and
contain the lower and higher 32 bits of a 64-bit address used for
indirect accesses respectively. Backplane indirect access
register is at offset 0xe0. Bits 3:0 of this register contain
the byte enables supplied to the system backplane on indirect
backplane accesses. So they should be set to 1. Bit 9 (StartBusy bit)
is set to 1 to start an indirect backplane access.
The hardware clears this bit to 0 when the transfer completes.
*/
#define  STARTBUSY_BIT_POLL_MAX_TIME 50
#define  INCREMENT_ADDRESS 4
static int
wlc_bmac_bt_regs_read(wlc_hw_info_t *wlc_hw, uint32 stAdd, uint32 dump_size, uint32 *b)
{
	uint32 regval1;
	uint32 cur_val = stAdd;
	uint32 endAddress = stAdd + dump_size;
	int counter = 0;
	int delay_val;
	int err;
	while (cur_val < endAddress) {
		si_ccreg(wlc_hw->sih, 0xd0, ~0, cur_val);
		si_ccreg(wlc_hw->sih, 0xd4, ~0, 0);
		si_ccreg(wlc_hw->sih, 0xe0, ~0, 0x20f);
		/*
		The StartBusy bit is set to 1 to start an indirect backplane access.
		The hardware clears this field to 0 when the transfer completes.
		*/
		for (delay_val = 0; delay_val < STARTBUSY_BIT_POLL_MAX_TIME; delay_val++) {
			if (si_ccreg(wlc_hw->sih, 0xe0, 0, 0) == 0x0000000F)
				break;
			OSL_DELAY(100);
		}
		if (delay_val == (STARTBUSY_BIT_POLL_MAX_TIME)) {
			err = BCME_ERROR;
			return err;
		}
		regval1 = si_ccreg(wlc_hw->sih, 0xd8, 0, 0);
		b[counter] = regval1;
		counter++;
		cur_val += INCREMENT_ADDRESS;
	}
	return 0;
}

void
wlc_bmac_enable_rx_hostmem_access(wlc_hw_info_t *wlc_hw, bool enabled)
{
	wlc_info_t *wlc = wlc_hw->wlc;

	/* device power state changed in D3 device can;t access host memory any more */
	/* switch off the classification in the ucode and let the packet come to fifo1 only */
	if (BCMSPLITRX_ENAB()) {
		if (!wlc_hw->up) {
			return;
		}
		WL_ERROR(("enable %d: q0 frmcnt %d, wrdcnt %d, q1 frmcnt %d, wrdcnt %d\n",
		enabled, R_REG(wlc->osh, &wlc->regs->u_rcv.d11acregs.rcv_frm_cnt_q0),
		R_REG(wlc->osh, &wlc->regs->u_rcv.d11acregs.rcv_wrd_cnt_q0),
		R_REG(wlc->osh, &wlc->regs->u_rcv.d11acregs.rcv_frm_cnt_q1),
		R_REG(wlc->osh, &wlc->regs->u_rcv.d11acregs.rcv_wrd_cnt_q1)));

		dma_rxfill_suspend(wlc_hw->di[RX_FIFO], !enabled);
		if (enabled)  {
			wlc_mhf(wlc, MHF1, MHF1_RXFIFO1, 0, WLC_BAND_ALL);
			wlc_mhf(wlc, MHF3, MHF3_SELECT_RXF1, MHF3_SELECT_RXF1, WLC_BAND_ALL);
		}
		else {
			wlc_bmac_suspend_mac_and_wait(wlc_hw);
			if (R_REG(wlc->osh, &wlc->regs->u_rcv.d11acregs.rcv_frm_cnt_q0)) {
				uint32 pend_cnt;
				SPINWAIT((R_REG(wlc_hw->osh,
					&wlc_hw->regs->u_rcv.d11acregs.rcv_frm_cnt_q0) != 0),
					10000);
				pend_cnt = R_REG(wlc->osh,
					&wlc->regs->u_rcv.d11acregs.rcv_frm_cnt_q0);
				if (pend_cnt) {
					WL_ERROR(("TO pkts to be drained fifo0 %d, dma pend %d\n",
						pend_cnt, dma_rxactive(wlc_hw->di[RX_FIFO])));
				}
			}
			wlc_mhf(wlc, MHF1, MHF1_RXFIFO1, MHF1_RXFIFO1, WLC_BAND_ALL);
			wlc_bmac_enable_mac(wlc->hw);
		}
	}
}

#if defined WLTXPWR_CACHE
void wlc_bmac_clear_band_pwr_offset(ppr_t *txpwr_offsets, wlc_hw_info_t *wlc_hw)
{
	if (NBANDS_HW(wlc_hw) > 1) {
		wlc_phy_clear_match_tx_offset(wlc_hw->bandstate[BAND_2G_INDEX]->pi, txpwr_offsets);
		wlc_phy_clear_match_tx_offset(wlc_hw->bandstate[BAND_5G_INDEX]->pi, txpwr_offsets);
	} else {
		wlc_phy_clear_match_tx_offset(wlc_hw->band->pi, txpwr_offsets);
	}
}
#endif // endif

bool
wlc_bmac_rsdb_cap(wlc_hw_info_t *wlc_hw)
{
	bool hwcap = FALSE;

	ASSERT(wlc_hw != NULL);
	hwcap = (wlc_hw->num_mac_chains > 1) ? TRUE : FALSE;
	return hwcap;
}

void
wlc_bmac_init_core_reset_disable_fn(wlc_hw_info_t *wlc_hw)
{
	ASSERT(wlc_hw != NULL);

#ifdef WLRSDB
	if (wlc_bmac_rsdb_cap(wlc_hw)) {
		wlc_hw->mac_core_reset_fn = si_d11rsdb_core_reset;
		wlc_hw->mac_core_disable_fn = si_d11rsdb_core_disable;

	} else

#endif // endif
	{
		wlc_hw->mac_core_reset_fn = si_core_reset;
		wlc_hw->mac_core_disable_fn = si_core_disable;
	}
}

void
wlc_bmac_core_reset(wlc_hw_info_t *wlc_hw, uint32 flags, uint32 resetflags)
{
	if (!wlc_hw || !wlc_hw->mac_core_reset_fn)
		return;

	if (D11REV_GE(wlc_hw->corerev, 64)) {
		int idx;

		/* New MAC uses sysmem as the buffers;
		 * need to make sure the sysmem core is up before use
		 */
		idx = si_coreidx(wlc_hw->sih);
		if (si_setcore(wlc_hw->sih, SYSMEM_CORE_ID, 0)) {
			if (!si_iscoreup(wlc_hw->sih))
				si_core_reset(wlc_hw->sih, 0, 0);
			si_setcoreidx(wlc_hw->sih, idx);
		}
	}

	(wlc_hw->mac_core_reset_fn)(wlc_hw->sih, flags, resetflags);

	/* Program the location in Sysmem RAM where the MAC Buffer Memory region begins. */
	if (D11REV_GE(wlc_hw->corerev, 64) && (BUSTYPE(wlc_hw->sih->bustype) == SI_BUS)) {
		/* Enable IHR for programming below regs. */
		wlc_bmac_mctrl(wlc_hw, ~0, MCTL_IHR_EN);
		W_REG(wlc_hw->osh, &wlc_hw->regs->u.d11acregs.SysMStartAddrHi,
			D11MAC_SYSM_STARTADDR_H);
		W_REG(wlc_hw->osh, &wlc_hw->regs->u.d11acregs.SysMStartAddrLo,
			D11MAC_SYSM_STARTADDR_L);
	}
}

void
wlc_bmac_core_disable(wlc_hw_info_t *wlc_hw, uint32 bits)
{
	if (!wlc_hw || !wlc_hw->mac_core_disable_fn)
		return;

	(wlc_hw->mac_core_disable_fn)(wlc_hw->sih, bits);

}

/* This returns the context of last D11 core
 * units using wlc_hw->macunit identity.
 */
bool
wlc_bmac_islast_core(wlc_hw_info_t *wlc_hw)
{
	ASSERT(wlc_hw != NULL);
	ASSERT(wlc_hw->sih != NULL);
	return (wlc_hw->macunit ==
		(si_numd11coreunits(wlc_hw->sih) - 1));
}

/*
FIFO- interrupt state machine is explained below
FIFO0-INT	FIFO1-INT	DECODE AS
0		0		Idle state; no interrupt recieved on this pkt
0		1		fifo-1 interrupt recieved; waiting for fifo-0 int
1		0		fifo-0 interrupt recieved; waiting for fifo-1 int
*/
#if defined(BCMPCIEDEV)
static int
wlc_bmac_process_split_fifo_pkt(wlc_hw_info_t *wlc_hw, uint fifo, void* p)
{
/*
	uint16 fifo1len = 0;
	uint16 convstatus = 0;
*/
	wlc_info_t *wlc = wlc_hw->wlc;
	wlc_tunables_t *tune = wlc->pub->tunables;

	WL_TRACE(("wl%d: %s BMAC rev pkt on fifo %d \n\n", wlc_hw->unit, __FUNCTION__, fifo));

	if (fifo == RX_FIFO) {
		if (PKTISFIFO0INT(wlc_hw->osh, p)) {
			/* FIFO-0 cant be set while processing fifo-0 int */
			WL_ERROR(("Error:FIFO-0 allready set for pkt %p  \n", p));
		} else {
			/* fifo-0 is the first int */
			PKTSETFIFO0INT(wlc_hw->osh, p);
		}
	}
	if (fifo == RX_FIFO1) {
		if (PKTISFIFO1INT(wlc_hw->osh, p)) {
			/* fifo-1 int cant be set during F-1 processing */
			WL_ERROR(("Error: fifo-1 allready set for %p \n", p));
		} else {
			/* Set F1 int */
			PKTSETFIFO1INT(wlc_hw->osh, p);
		}

	}

	if (!(PKTISFIFO0INT(wlc_hw->osh, p) && PKTISFIFO1INT(wlc_hw->osh, p))) {
		/* both fifos not set, return */
		return 0;
	} else {
		/* Recieved both interrupts, Proceed with rx processing */
		/* retrieve fifo-0 len */
		uint16 fifo0len = 0;
		wlc_d11rxhdr_t * wlc_rxhdr = NULL;
		wlc_rxhdr = (wlc_d11rxhdr_t *)PKTDATA(wlc_hw->osh, p);

		/* reset interrupt bits */
		PKTRESETFIFO0INT(wlc_hw->osh, p);
		PKTRESETFIFO1INT(wlc_hw->osh, p);

		/* length & conv status */
		fifo0len = ltoh16(wlc_rxhdr->rxhdr.RxFameSize_0);

		/*
		fifo1len = ltoh16(wlc_rxhdr->rxhdr.RxFrameSize);
		convstatus = ltoh16(wlc_rxhdr->rxhdr.HdrConvSt);
		*/

		if (!HDR_CONV()) {
			if (fifo0len <= ((uint16)(tune->copycount * 4))) {
				PKTSETFRAGUSEDLEN(wlc_hw->osh, p, 0);
			} else {
				PKTSETFRAGUSEDLEN(wlc_hw->osh, p,
					(fifo0len - (tune->copycount * 4)));
			}
		} else {
			PKTSETFRAGUSEDLEN(wlc_hw->osh, p, fifo0len);
		}
		return 1;
	}
}
#endif /* BCMPCIEDEV */

/* Check if given rx fifo is valid */
static uint8
wlc_bmac_rxfifo_enab(uint fifo)
{
	switch (fifo) {
		case RX_FIFO :
			return 1;
			break;
#ifndef BCM_OL_DEV
		case RX_FIFO1:
#ifdef FORCE_RX_FIFO1
			/* in 4349a0, fifo-2 classification will work only if fifo-1 is enabled */
			return 1;
#endif /* FORCE_RX_FIFO1 */
			return ((uint8)(PKT_CLASSIFY_EN(RX_FIFO1) || RXFIFO_SPLIT()));
			break;
		case RX_FIFO2 :
			return ((uint8)(PKT_CLASSIFY_EN(RX_FIFO2)));
			break;
#endif	/* BCM_OL_DEV */
		default :
			return 0;
	}
}

/**
 * This function encapsulates the call to dma_txfast.
 * In case cut through DMA is enabled, this function will also format necessary information
 * and call the required hnddma api to post the AQM dma descriptor.
 */
int BCMFASTPATH
wlc_bmac_dma_txfast(wlc_info_t *wlc, uint fifo, void *p, bool commit)
{
	hnddma_t *tx_di = NULL;
	int err = 0;

	WL_TRACE(("wlc%d: %s fifo=%d commit=%d \n", wlc->pub->unit, __FUNCTION__, fifo, commit));

#ifdef BCM_DMA_CT
	if (BCM_DMA_CT_ENAB(wlc)) {
		wlc_hw_info_t *wlc_hw = wlc->hw;
		uint pre_txout = 0;
		uint numd = 0;
		hnddma_t *aqm_di = NULL;
		uint32 ctrl1 = 0;
		uint32 ctrl2 = 0;
		uint8 ac = 0;
		uint8 *txd;
		d11actxh_t *txh;
		struct dot11_header *d11_hdr;
		dma64dd_t dd;
		uintptr data_dd_addr;
		uint mpdulen, tsoHdrSize = 0;

		tx_di = WLC_HW_DI(wlc, fifo);

		aqm_di = WLC_HW_AQM_DI(wlc, fifo);

		ASSERT(aqm_di);

		/* Before proceeding ahead first check if an AQM descriptor is available */
		if (!(*wlc_hw->txavail_aqm[fifo])) {
			WL_INFORM(("wlc%d %s: NO AQM descriptors available, fifo=%d \n",
				wlc->pub->unit, __FUNCTION__, fifo));
			return BCME_BUSY;
		}

		pre_txout = dma_get_next_txd_idx(tx_di, TRUE);

		/* post the data descriptor */
		err = dma_txfast(tx_di, p, commit);
		if (err < 0) {
			WL_ERROR(("wlc%d %s: dma_txfast failed, fifo=%d \n", wlc->pub->unit,
				__FUNCTION__, fifo));
			return BCME_BUSY;
		}

		/* tx dma call was successfull. Post the AQM descriptor */
		numd = dma_get_txd_count(tx_di, pre_txout, TRUE);

		/* get the txd header */
		wlc_txprep_pkt_get_hdrs(wlc, p, &txd, &txh, &d11_hdr);

		/* Start preparting AQM descriptor fields */

		/* ctrl1 */
		/* SOFPTR */
		data_dd_addr = dma_get_txd_addr(tx_di, pre_txout);
		ctrl1 |= (D64_AQM_CTRL1_SOFPTR & data_dd_addr);

		/* epoch */
		if (txd[2] & TOE_F2_EPOCH) {
			ctrl1 |= D64_AQM_CTRL1_EPOCH;
		}

		/* numd */
		ctrl1 |= ((numd << D64_AQM_CTRL1_NUMD_SHIFT) & D64_AQM_CTRL1_NUMD_MASK);

		/* AC */
#ifdef WME
		ac =  prio2fifo[PKTPRIO(p)];
#else
		ac =  (uint8)PKTPRIO(p);
#endif // endif
		ctrl1 |= ((ac << D64_AQM_CTRL1_AC_SHIFT) & D64_AQM_CTRL1_AC_MASK);
		ctrl1 |= (D64_CTRL1_EOF | D64_CTRL1_SOF | D64_CTRL1_IOC);

		/* ctrl2 */
		tsoHdrSize = (uint8*)txh - txd;
		mpdulen = pkttotlen(wlc_hw->osh, p) - ((uint8*)d11_hdr - txd);
		ctrl2 |= (mpdulen & D64_AQM_CTRL2_MPDULEN_MASK);
		if (!(txh->PktInfo.MacTxControlLow & D11AC_TXC_HDR_FMT_SHORT)) {
			ctrl2 |= D64_AQM_CTRL2_TXDTYPE;
		}

		dd.ctrl1 = ctrl1;
		dd.ctrl2 = ctrl2;

		/* get the mem address of the data buffer pointed by the SOF Tx dd */
		dma_get_txd_memaddr(tx_di, (uint32*)&dd.addrlow, (uint32*)&dd.addrhigh, pre_txout);
		dd.addrlow += tsoHdrSize;

		WL_TRACE(("wlc%d %s: pre dma_txdesc dd.ctrl1 = 0x%x, dd.ctrl2 = 0x%x, "
			"dd.addrlow = 0x%x, dd.addrhigh = 0x%x \n",
			wlc->pub->unit, __FUNCTION__, dd.ctrl1, dd.ctrl2,
			dd.addrlow, dd.addrhigh));
		WL_TRACE(("wlc%d %s: pre-dma_txdesc numd = %d, pre_txout = %d  \n", wlc->pub->unit,
			__FUNCTION__, numd, pre_txout));

		/* post the AQM descriptor */
		err = dma_txdesc(aqm_di, &dd, commit);
		if (err < 0) {
			WL_ERROR(("wlc%d %s: dma_txdesc failed fifo=%d \n", wlc->pub->unit,
				__FUNCTION__, fifo));
			return BCME_BUSY;
		}
	}
	else
#endif /* BCM_DMA_CT */
	{
		tx_di = WLC_HW_DI(wlc, fifo);
		err = dma_txfast(tx_di, p, commit);
		if (err < 0) {
			WL_ERROR(("wlc%d %s: dma_txfast failed, fifo=%d \n", wlc->pub->unit,
				__FUNCTION__, fifo));
			return BCME_BUSY;
		}
	}
	return err;
}

#ifdef BULK_PKTLIST
#ifdef BCM_DMA_CT
static uint16 BCMFASTPATH
wlc_bmac_dma_bulk_aqm_txcomplete(wlc_info_t *wlc, uint fifo,
		txd_range_t range, uint16 ncons)
{
	uint32 npkt;

	for (npkt = 0; npkt < ncons; npkt++)
	{
		if (dma_getnexttxdd(WLC_HW_AQM_DI(wlc, fifo), range, NULL) != BCME_OK)
		{
			/* Scan or other flush always set ncons to 1 and range to HNDDMA_RANGE_ALL.
			 * Expecting a zero return to confirm all packets in fifo has completed.
			 * No need to print the well-known behavior
			 */
			if (range == HNDDMA_RANGE_ALL)
				break;

			WL_INFORM(("wlc%d: %s: could not reclaim AQM-fifo %d descriptor\n",
				wlc->pub->unit, __FUNCTION__, fifo));
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
			{
				int npkts;
				uint i;
				for (i = 0; i < WLC_HW_NFIFO_INUSE(wlc); i++) {
					hnddma_t *di;
					npkts = TXPKTPENDGET(wlc, i);
					if (npkts == 0) continue;
					di = WLC_HW_DI(wlc, i);
					WL_ERROR(("FIFO-%d TXPEND = %d TX-DMA%d =>\n",
							i, npkts, i));
					dma_dumptx(di, NULL, FALSE);
					WL_ERROR(("CT-DMA%d =>\n", i));
					di = WLC_HW_AQM_DI(wlc, i);
					dma_dumptx(di, NULL, FALSE);
				}
			}
#endif	/* BCMDBG || BCMDBG_DUMP */
			break;
		} /* if (dma_getnexttxdd()..) */
	} /* for (npkt=0 ...) */
	return npkt;
}
#define DMA_CT_RANGE(r) HNDDMA_RANGE_ALL
#else
#define DMA_CT_RANGE(r)	r
#endif /* BCM_DMA_CT */

uint16 BCMFASTPATH
wlc_bmac_dma_bulk_txcomplete(wlc_info_t *wlc, uint fifo, uint16 ncons,
		void **list_head, void **list_tail, txd_range_t range)
{

/* Routine performs the BMAC bulk txcompletion processing.
 * Returns the number of packets processed.
 * This number may be different from ncons, however the number of AQM descriptors
 * reclaimed must match ncons otherwise the return value is zero.
 * It is left up to the caller to decide on the next course of action.
 *
 * Note : The extended error codes from the DMA library are discarded and not passed
 * up to the callers.
 */
	uint16 nproc = 0;

	/* Assert for non-zero ncons, avoid additional check where ncons == nproc == 0 */
	ASSERT(ncons);

	/* This routine minimizes the logical comparisons. nproc will not be updated
	 * if the AQM or data desc cannot be reclaimed, this is normally an error,
	 * it is left up to the caller to take corrective action or ASSERT()
	 */
#ifdef BCM_DMA_CT
	if (BCM_DMA_CT_ENAB(wlc)) {
		/* Reclaim AQM descriptor and corresponding data descriptor */
		if (wlc_bmac_dma_bulk_aqm_txcomplete(wlc, fifo,
			DMA_CT_RANGE(range), ncons) == ncons) {
			 /* In the case of data desc error nproc will not be updated. */
			dma64_bulk_txcomplete(WLC_HW_DI(wlc, fifo), ncons, &nproc,
				list_head, list_tail, DMA_CT_RANGE(range));
		}
		/* This value will be zero if either the AQM desc or
		 * the data descriptor cannot be reclaimed
		 */
		return nproc;
	}
#endif // endif

	/* In the case of error nproc will not be updated */
	dma64_bulk_txcomplete(WLC_HW_DI(wlc, fifo), ncons, &nproc,
		list_head, list_tail, DMA_CT_RANGE(range));
	return nproc;
}

/**
 * This function encapsulates the call to dma_getnexttxp.
 * In case cut through DMA is enabled, this function will also issue the call to get
 * the next AQM descriptor.
 *
 * wlc_bmac_dma_getnexttxp() returns a single descriptor
 */

void * BCMFASTPATH
wlc_bmac_dma_getnexttxp(wlc_info_t *wlc, uint fifo, txd_range_t range)
{
	void *txp = NULL;

	if (wlc_bmac_dma_bulk_txcomplete(wlc, fifo, 1, &txp, NULL, range)) {
		return txp;
	} else {
		/* If the AQM ring is empty, the function returns a zero pkt count
		 * Check to see if the corresponding data fifo ring is also empty.
		 * This situation can arise if we are detaching a queue
		 * at the end of scan.
		 */
		ASSERT(dma_txcommitted(WLC_HW_DI(wlc, fifo)) == 0);
		return NULL;
	}
}
#else
/**
 * This function encapsulates the call to dma_getnexttxp.
 * In case cut through DMA is enabled, this function will also issue the call to get
 * the next AQM descriptor.
 */
void * BCMFASTPATH
wlc_bmac_dma_getnexttxp(wlc_info_t *wlc, uint fifo, txd_range_t range)
{
	void *txp = NULL;

	WL_TRACE(("wlc%d: %s fifo %d \n", wlc->pub->unit, __FUNCTION__, fifo));
#ifdef BCM_DMA_CT
	if (BCM_DMA_CT_ENAB(wlc)) {
		uint32 txdma_flags = 0;

		if ((wlc->cpbusy_war) && (range == HNDDMA_RANGE_ALL))
			aqm_dma_init_fifo_stats(WLC_HW_AQM_DI(wlc, fifo), fifo);
		/* reclaim AQM descriptor */
		if (dma_getnexttxdd(WLC_HW_AQM_DI(wlc, fifo), range, &txdma_flags) != BCME_OK) {
			if (range == HNDDMA_RANGE_ALL)
				return NULL;

			WL_ERROR(("wlc%d: %s: could not reclaim AQM-fifo %d descriptor,"
				" range %d\n", wlc->pub->unit, __FUNCTION__, fifo, range));

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
			{
			int i, npkts;

			for (i = 0; i < WLC_HW_NFIFO_INUSE(wlc); i++) {
				hnddma_t *di;
				npkts = TXPKTPENDGET(wlc, i);
				if (npkts == 0) continue;
				di = WLC_HW_DI(wlc, i);
				WL_ERROR(("FIFO-%d TXPEND = %d TX-DMA%d =>\n", i, npkts, i));
				dma_dumptx(di, NULL, FALSE);
				WL_ERROR(("CT-DMA%d =>\n", i));
				di = WLC_HW_AQM_DI(wlc, i);
				dma_dumptx(di, NULL, FALSE);
			}
			}
#endif /* defined(BCMDBG) || defined(BCMDBG_DUMP) */
			/* If the AQM ring is empty check to see if the
			 * corresponding data fifo ring is also empty.
			 * This situation can arise if we are detaching a queue
			 * in the case of a scan end.
			 *
			 * dma_txcommitted() counts all packets in the ring including
			 * completed packets not yet processed
			 */
			ASSERT(dma_txcommitted(WLC_HW_DI(wlc, fifo)) == 0);
			return NULL;
		}

		/* get next data packet and reclaim TXDMA descriptor(s) */
		txp = dma_getnexttxp(WLC_HW_DI(wlc, fifo), HNDDMA_RANGE_ALL);
		if (txp == NULL) {
			WL_ERROR(("wlc%d: %s Could not reclaim data dd fifo=%d\n",
				wlc->pub->unit, __FUNCTION__, fifo));
			return NULL;
		}

		return txp;
	}
#endif /* #ifdef BCM_DMA_CT */

	/* get next data packet and reclaim TXDMA descriptor(s) */
	txp = dma_getnexttxp(WLC_HW_DI(wlc, fifo), range);
	if (txp == NULL) {
		WL_INFORM(("wlc%d: %s Could not reclaim data dd fifo=%d\n",
			wlc->pub->unit, __FUNCTION__, fifo));
		return NULL;
	}
	return txp;
}
#endif /* BULK_PKTLIST */

#ifdef BCM_DMA_CT
static void
wlc_bmac_enable_ct_access(wlc_hw_info_t *wlc_hw, bool enabled)
{
	if (D11REV_GE(wlc_hw->corerev, 64) && wlc_hw->clk) {
		if (enabled)
			W_REG(wlc_hw->osh, &wlc_hw->regs->u.d11acregs.u0.ge64.TXE_ctmode, 0x1);
		else
			W_REG(wlc_hw->osh, &wlc_hw->regs->u.d11acregs.u0.ge64.TXE_ctmode, 0x0);
	}
}

void
wlc_bmac_ctdma_update(wlc_hw_info_t *wlc_hw, bool enabled)
{
	bool user_config;

	if (wlc_hw->wlc->_dma_ct_flags & DMA_CT_IOCTL_OVERRIDE) {
		/* User already changed CTDMA on/off previously, so we need to
		 * check against previous changed value.
		 */
		user_config = (wlc_hw->wlc->_dma_ct_flags & DMA_CT_IOCTL_CONFIG) ? TRUE : FALSE;
		if (user_config != enabled)
			wlc_hw->wlc->_dma_ct_flags &= ~DMA_CT_IOCTL_OVERRIDE;
	} else if (wlc_hw->wlc->_dma_ct != enabled) {
		/* Otherwise check against current CTDMA state. If different, then it implies
		 * a state change.
		 */
		wlc_hw->wlc->_dma_ct_flags |= DMA_CT_IOCTL_OVERRIDE;
		wlc_hw->wlc->_dma_ct_flags &= ~DMA_CT_IOCTL_CONFIG;
		if (enabled)
			wlc_hw->wlc->_dma_ct_flags |= DMA_CT_IOCTL_CONFIG;
	}
}
#endif /* BCM_DMA_CT */

#ifdef HOST_HDR_FETCH
/**
 * Allocate a pool of tx status fetch requests
 */
static void
wlc_txs_fetch_rqst_attach(wlc_info_t * wlc)
{
	txhdr_fetch_rqst_t * fetch_rqst;
	txhdr_fetch_rqst_t * free_list;
	uint16 size;
	uint8 idx;

	/* Initialize */
	free_list = NULL;
	size = TXS_MAX_FETCH_RQST * sizeof(txhdr_fetch_rqst_t);

	/* Preallocate the fetch requests */
	fetch_rqst = MALLOCZ(wlc->osh, size);

	if (fetch_rqst == NULL) {
		WL_ERROR(("%s : Mem alloc failed for size %d \n", __FUNCTION__, size));
		return;
	}

	/* Store the mem blob */
	wlc->txs_fetch_rqst = fetch_rqst;

	/* Link all free fetch requests */
	for (idx = 0; idx < TXS_MAX_FETCH_RQST; idx++) {
		/* assign the callback function during init time */
		fetch_rqst->rqst.cb = wlc_bmac_process_fetched_hdr_txstatus;
		fetch_rqst->rqst.ctx = fetch_rqst;

		fetch_rqst->next = free_list;
		free_list =  fetch_rqst++;
	}

	/* Free pool of fetch request */
	wlc->txs_fetch_free_list = free_list;

}
/** Detach the pool of fetch requests */
static void
wlc_txs_fetch_rqst_detach(wlc_info_t * wlc)
{

	/* release the pool */
	if (wlc->txs_fetch_rqst)
		MFREE(wlc->osh, wlc->txs_fetch_rqst,
			TXS_MAX_FETCH_RQST * sizeof(txhdr_fetch_rqst_t));

	/* Reset the free list */
	wlc->txs_fetch_free_list = NULL;
}

/** Return a free fetch request handle */
static txhdr_fetch_rqst_t*
wlc_txs_fetch_rqst_get(wlc_info_t * wlc)
{
	txhdr_fetch_rqst_t* fetch_rqst;

	/* List empty */
	if (wlc->txs_fetch_free_list == NULL)
		return NULL;

	fetch_rqst = wlc->txs_fetch_free_list;

	/* Update free list */
	wlc->txs_fetch_free_list = fetch_rqst->next;
	fetch_rqst->next = NULL;

	return fetch_rqst;
}
/** Free a fetch request handle */
static void
wlc_txs_fetch_rqst_release(wlc_info_t * wlc, txhdr_fetch_rqst_t* fetch_rqst)
{
	ASSERT(fetch_rqst);
	ASSERT(fetch_rqst->next == NULL);

	fetch_rqst->next = wlc->txs_fetch_free_list;

	wlc->txs_fetch_free_list = fetch_rqst;
}
/*
 * 	TXS Fetch Queue APIs
 * Enqueue a fetch request into an inuse list
 * Used for canceling the request in case of a flush
 */
static void
wlc_txs_fetch_rqst_enque(wlc_info_t * wlc, txhdr_fetch_rqst_t *fr)
{
	txhdr_fetch_rqst_q_t *frq;

	/* Initialize */
	frq = &wlc->txs_fetch_queue;

	/* Enqueue at the tail */
	if (frq->head == NULL)
		frq->head = frq->tail = fr;
	else {
		frq->tail->next = fr;
		frq->tail = fr;
	}
	frq->count++;
	frq->tail->next = NULL;
}
/*
 * Remove a fetch request from saved list
 * Used for canceling the request in case of a flush
 */
static txhdr_fetch_rqst_t*
wlc_txs_fetch_rqst_remqueue(wlc_info_t * wlc, txhdr_fetch_rqst_t *fr)
{
	txhdr_fetch_rqst_q_t *frq;
	txhdr_fetch_rqst_t *cur_fr, *prev_fr;

	/* Initialize */
	frq = &wlc->txs_fetch_queue;

	/* Dequeue from the head */
	if (frq->head == NULL) {
		return NULL;
	} else if (frq->head == frq->tail) {
		if (frq->head == fr) {
			ASSERT(frq->count > 0);
			frq->count--;
			frq->head = frq->tail = NULL;
			return fr;
		} else
			return NULL;
	} else {
		for (cur_fr = frq->head, prev_fr = NULL;
			cur_fr != NULL; prev_fr = cur_fr, cur_fr = cur_fr->next)
		{
			if (cur_fr == fr) {
				ASSERT(frq->count > 0);
				frq->count--;
				if (prev_fr == NULL)
					frq->head = cur_fr->next;
				else if (cur_fr == frq->tail) {
					prev_fr->next = NULL;
					frq->tail = prev_fr;
				} else
					prev_fr->next = cur_fr->next;

				fr->next = NULL;
				return fr;
			}
		}
	}

	return NULL;
}

/*
 * Cancel all pending tx header fetch requests
 * 1. request could be in fetch requst wait queue at rte_fethc layer
 * 2. or could be dispatched to bus layer for DMA processing
 * This routine will call the user callback fn with "cancelled" flag set
 */
void
wlc_txs_fetch_rqst_queue_flush(wlc_info_t * wlc, uint fifo_bitmap)
{
	uint fifo_idx;
	txhdr_fetch_rqst_t *fr, *next_fr;

	fr = wlc->txs_fetch_queue.head;
	while (fr) {
		next_fr = fr->next;

		/* Find the queue we want to flush */
		fifo_idx = WLC_TXFID_GET_QUEUE(fr->txs.frameid);
		if (isset(&fifo_bitmap, fifo_idx)) {
			if (FETCH_RQST_FLAG_GET(&fr->rqst, FETCH_RQST_CANCELLED)) {
				WL_ERROR(("%s : fifo %d fr->rqst has been cancelled\n",
					__FUNCTION__, fifo_idx));
			} else {
				void *p;
				uint16 pktcnt;

				pktcnt = 0;
				p = fr->txs.head_pkt;
				ASSERT(p);
				while (p) {
					pktcnt++;
					p = PKTLINK(p);
				}

				WL_ERROR(("%s : fifo %d complete %d mpdus\n",
					__FUNCTION__, fifo_idx, pktcnt));

				WLC_TXFIFO_COMPLETE(wlc, fifo_idx, pktcnt, 0);

				/* Cancel The request */
				hnd_cancel_fetch_rqst(&fr->rqst);
			}
		}

		fr = next_fr;
	}
}

/** Schedule a fetch request to pull down Host tx header */
int
wlc_bmac_tx_hdr_fetch(wlc_info_t * wlc, void* p, tx_status_t* txs, uint32 s2)
{
	txhdr_fetch_rqst_t * fetch_rqst;
	uint16 ncons, tot_mpdu;
	uint queue;
	void *p0, *p1;
	uint16 txhdrlen;	/* header length */
	void* d11buf;		/* New extension buffer */

	ASSERT(PKTISTXFRAG(wlc->osh, p));

	/* Initialize */
	tot_mpdu = 0;
	queue = WLC_TXFID_GET_QUEUE(txs->frameid);
	ncons = (txs->status.raw_bits & TX_STATUS40_NCONS) >> TX_STATUS40_NCONS_SHIFT;
	txhdrlen = PKTLEN(wlc->osh, p);

	/* Make space for txheader: allocate a header buffer */
	d11buf = lfbufpool_get(D11_EXT_LFRAG_BUF_POOL);

	/* Should not fail here: No recovery available */
	if (d11buf == NULL) {
		WL_ERROR(("%s : D11 buffer not available \n", __FUNCTION__));
		ASSERT(0);
		return BCME_NOMEM;
	}

	/* Remap extended buffer to lbuf */
	PKTSETBUF(wlc->osh, p, d11buf, txhdrlen);

	/* Update head pkt info in the txs */
	txs->head_pkt = p;

	/* Allocate a fetch request entity */
	fetch_rqst = wlc_txs_fetch_rqst_get(wlc);

	if (fetch_rqst == NULL) {
		WL_ERROR(("%s: Fetch request for tx hdr failed \n", __FUNCTION__));
		ASSERT(0);
		return BCME_NOMEM;
	}

	/* Fillup fetch request info */
	fetch_rqst->wlc = wlc;

	/* Retrieve ncons pkts from DMA TXq
	 * Link all the pkts to head pkt
	 * Use this link info to process other MPDUs marked in this txstatus
	 */
	p0 = p;

	while (TRUE) {
		tot_mpdu++;

		 /* only loop ncons times */
		if (tot_mpdu >= ncons) {
			break;
		}

		/* get next pkt -- check and handle NULL */
		p1 = GETNEXTTXP(wlc, queue);

		ASSERT(WLPKTFLAG_AMPDU(WLPKTTAG(p1)));

		/* if tot_mpdu < ncons, should have pkt in queue */
		if (p1 == NULL) {
			WL_ERROR(("%s: p is NULL. tot_mpdu: %d, ncons: %d\n",
			        __FUNCTION__, tot_mpdu, ncons));
			ASSERT(p1);
			break;
		}

		ASSERT(PKTISTXFRAG(wlc->osh, p1));

		/* Reset Metadata section
		 * All chained packets will never go through header fetch;
		 * so reset their txheader section in host
		 */
	//	PKTRESETHDRINHOST(wlc->osh, p1);

		/* Link all pkts to head pkt */
		PKTSETLINK(p0, p1);
		p0 = p1;
	}

	/* Copy Tx status bits */
	bcopy(txs, &fetch_rqst->txs, sizeof(tx_status_t));
	fetch_rqst->s2 = s2;

	 /* Fillup  the fetch request */
	fetch_rqst->rqst.flags = 0;
	fetch_rqst->rqst.size = txhdrlen;
	fetch_rqst->rqst.dest = PKTDATA(wlc->osh, p);

	/* Fetch Address */

	/* Get the extended memory region */
	lbuf_dhdhdr_memory_extension(p, &fetch_rqst->rqst.haddr);

	/* Reset host header flag */
	ASSERT(__PKTISHDRINHOST(wlc->osh, p));
	PKTRESETHDRINHOST(wlc->osh, p);

	/* Submit the request now */
	hnd_fetch_rqst(&fetch_rqst->rqst);

	/* En-queue the request to a inuse list */
	wlc_txs_fetch_rqst_enque(wlc, fetch_rqst);

	return BCME_OK;

}

/* Callback function for txstatus processing on fetch completion */
static void
wlc_bmac_process_fetched_hdr_txstatus(struct fetch_rqst *fr, bool cancelled)
{
	wlc_info_t * wlc;
	txhdr_fetch_rqst_t * fetch_rqst;
	tx_status_t *txs;
	txhdr_fetch_rqst_t *fr_deq;

	BCM_REFERENCE(fr_deq);

	/* Initialize */
	fetch_rqst = (txhdr_fetch_rqst_t*)fr->ctx;
	txs = &fetch_rqst->txs;

	wlc = (wlc_info_t*)fetch_rqst->wlc;

	/* Dequeue the request from inuse list */
	fr_deq = wlc_txs_fetch_rqst_remqueue(wlc, fetch_rqst);

	/* Sanitize the fetch request */
	ASSERT(fetch_rqst == fr_deq);

	/* If wl was brought down, release all pkts now
	 * TXQ and FIFO accounting wasreset as part of wl down
	 * Dont proceed with do_txstatus
	 */
	if (cancelled || (wlc->pub->hw_up == FALSE)) {
		void *p0, *p;
		uint16 pktcnt = 0;

		p = txs->head_pkt;
		while (p) {

			/* get the LInked pkt */
			p0 = PKTLINK(p);

			/* reset the link */
			PKTSETLINK(p, NULL);

			/* Re use the same header buffer for rest of the packets */
			if (p0) {
				PKTREUSED11BUF(wlc->osh, p, p0);

				/* reset header in host flag */
				ASSERT(__PKTISHDRINHOST(wlc->osh, p0));
				PKTRESETHDRINHOST(wlc->osh, p0);
			}

			pktcnt++;
			PKTFREE(wlc->osh, p, TRUE);

			/* Switch to next p */
			p = p0;
		}
		WL_ERROR(("%s : Device RESET/Fetch Cancelled : fifo %d free UP %d mpdus \n",
			__FUNCTION__, WLC_TXFID_GET_QUEUE(txs->frameid), pktcnt));

		/* Free the fetch request now back to free pool */
		wlc_txs_fetch_rqst_release(wlc, fetch_rqst);

		return;
	}
	/* Put the length back to hdr length */
	/* Continue dotxstatus with fetched header */
	wlc_dotxstatus(wlc, &fetch_rqst->txs, fetch_rqst->s2);

	/* Free the fetch request now back to free pool */
	wlc_txs_fetch_rqst_release(wlc, fetch_rqst);

	/* Flush accumulated txstatus to host */
	wl_busioctl(wlc->wl, BUS_FLUSH_CHAINED_PKTS, NULL,
		0, NULL, NULL, FALSE);
}

/* For wlc_ampdu_tx_map_pkts() to walk through the txs_fetch_queue */
void
wlc_bmac_tx_hdr_fetch_map_pkts(wlc_info_t *wlc, map_pkts_cb_fn cb, void *ctx)
{
	txhdr_fetch_rqst_t *fr, *next_fr;
	void *p;

	/* Iterate over all linked fetch requests */
	fr = wlc->txs_fetch_queue.head;
	while (fr) {
		next_fr = fr->next;

		p = fr->txs.head_pkt;
		ASSERT(p);

		/* Map the packets */
		while (p) {
			(void)cb(ctx, p);
			p = PKTLINK(p);
		}

		fr = next_fr;
	}
}
#endif /* HOST_HDR_FETCH */
