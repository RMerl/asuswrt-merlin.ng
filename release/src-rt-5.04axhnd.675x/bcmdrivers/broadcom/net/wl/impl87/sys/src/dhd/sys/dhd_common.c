/*
 * Broadcom Dongle Host Driver (DHD), common DHD core.
 *
 * Copyright (C) 2022, Broadcom. All Rights Reserved.
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
 * $Id: dhd_common.c 808279 2022-02-15 05:56:45Z $
 */
#include <typedefs.h>
#include <osl.h>

#include <epivers.h>
#include <bcmutils.h>

#include <bcmendian.h>
#include <dngl_stats.h>
#include <dhd.h>
#include <dhd_ip.h>
#include <bcmevent.h>
#include <dhd_macdbg.h>
#include <wl_core.h>
#include <dhd_linux.h>

#ifdef BCMPCIE
#include <dhd_flowring.h>
#endif

#ifdef BCMDBUS
#include <dbus.h>
#else
#include <dhd_bus.h>
#endif /* BCMDBUS */
#include <dhd_proto.h>
#include <dhd_dbg.h>
#include <msgtrace.h>

#ifdef WL_CFG80211
#include <wl_cfg80211.h>
#endif
#ifdef WLBTAMP
#include <bt_amp_hci.h>
#include <dhd_bta.h>
#endif
#ifdef PNO_SUPPORT
#include <dhd_pno.h>
#endif

#ifdef IL_BIGENDIAN
#include <bcmendian.h>
#define htod32(i) (bcmswap32(i))
#define htod16(i) (bcmswap16(i))
#define dtoh32(i) (bcmswap32(i))
#define dtoh16(i) (bcmswap16(i))
#define htodchanspec(i) htod16(i)
#define dtohchanspec(i) dtoh16(i)
#else
#define htod32(i) (i)
#define htod16(i) (i)
#define dtoh32(i) (i)
#define dtoh16(i) (i)
#define htodchanspec(i) (i)
#define dtohchanspec(i) (i)
#endif /* IL_BIGENDIAN */

#ifdef PROP_TXSTATUS
#include <wlfc_proto.h>
#include <dhd_wlfc.h>
#endif

#ifdef DHD_WMF
#include <dhd_wmf_linux.h>
#endif /* DHD_WMF */

#ifdef DHD_L2_FILTER
#include <dhd_l2_filter.h>
#endif /* DHD_L2_FILTER */

#ifdef DHD_PSTA
#include <dhd_psta.h>
#endif /* DHD_PSTA */

#ifdef DHD_WET
#include <dhd_wet.h>
#endif /* DHD_WET */

#if defined(BCM_DHD_RUNNER)
#include <dhd_runner.h>
#endif /* BCM_DHD_RUNNER */

#ifdef BCM_PKTFWD
#include <dhd_pktfwd.h>
#endif /* BCM_PKTFWD */
#if defined(BCM_AWL)
#include <dhd_awl.h>
#endif /* BCM_AWL */
#ifdef DHD_IFE
#include <dhd_ife.h>
#endif /* DHD_IFE */

#ifdef WLMEDIA_HTSF
extern void htsf_update(struct dhd_info *dhd, void *data);
#endif
/*
* XXX WAR to unblock DVT tests for the issue SWWLAN-4684
* This change will be removed once the issue is completely fixed
*/
#ifndef OEM_ANDROID
#ifdef DHD_8021X_DUMP
int dhd_msg_level = DHD_ERROR_VAL | DHD_DATA_VAL;
#else /* DHD_8021X_DUMP */
int dhd_msg_level = DHD_ERROR_VAL;
#endif /* DHD_8021X_DUMP */
#else /* OEM_ANDROID */
int dhd_msg_level = DHD_ERROR_VAL | DHD_MSGTRACE_VAL;
#endif /* OEM_ANDROID */

#if defined(WL_WLC_SHIM)
#include <wl_shim.h>
#endif /* WL_WLC_SHIM */

#if (defined PNO_SUPPORT) && (!defined WL_SCHED_SCAN)
#include <wl_iw.h>
#endif /* (defined PNO_SUPPORT) && (!defined WL_SCHED_SCAN) */

#ifdef SOFTAP
char fw_path2[MOD_PARAM_PATHLEN];
extern bool softap_enabled;
#endif

#if defined(BCM_CPE_PKTC)
#include <wl_pktc.h>
#endif /* BCM_CPE_PKTC */
#ifdef BCM_WFD
#include <dhd_wfd.h>
#endif

#ifdef BCM_BLOG
#include <dhd_blog.h>
#endif

/* Last connection success/failure status */
uint32 dhd_conn_event;
uint32 dhd_conn_status;
uint32 dhd_conn_reason;

extern int dhd_iscan_request(void * dhdp, uint16 action);
extern void dhd_ind_scan_confirm(void *h, bool status);
extern int dhd_iscan_in_progress(void *h);
void dhd_iscan_lock(void);
void dhd_iscan_unlock(void);
extern int dhd_change_mtu(dhd_pub_t *dhd, int new_mtu, int ifidx);
#if defined(OEM_ANDROID) && !defined(AP) && defined(WLP2P)
extern uint32 dhd_get_concurrent_capabilites(dhd_pub_t *dhd);
#endif

extern int dhd_socram_dump(struct dhd_bus *bus);

#if defined(OEM_ANDROID)
bool ap_cfg_running = FALSE;
bool ap_fw_loaded = FALSE;
#endif /* defined(OEM_ANDROID) && defined(SOFTAP) */

/* Version string to report */
#ifdef DHD_DEBUG
#ifndef SRCBASE
#define SRCBASE        "drivers/net/wireless/bcmdhd"
#endif
#define DHD_COMPILED "\nCompiled in " SRCBASE
#endif /* DHD_DEBUG */

#if defined(BCMDBG)
const char dhd_version[] = "\nDongle Host Driver, version " EPI_VERSION_STR "\nCompiled from "
	__FILE__;
#elif defined(DHD_DEBUG)
const char dhd_version[] = "Dongle Host Driver, version " EPI_VERSION_STR
	DHD_COMPILED;
#else
const char dhd_version[] = "\nDongle Host Driver, version " EPI_VERSION_STR "\nCompiled from ";
#endif /* BCMDBG */

void dhd_set_timer(void *bus, uint wdtick);

#if defined(BCM_ROUTER_DHD) || defined(TRAFFIC_MGMT_DWM)
static int traffic_mgmt_add_dwm_filter(dhd_pub_t *dhd,
	trf_mgmt_filter_list_t * trf_mgmt_filter_list, int len);
#endif

static char* ioctl2str(uint32 ioctl);
static int dhd_hmembytes(dhd_pub_t *dhd, void *haddr, uint8 *data, uint size);

/* IOVar table */
enum {
	IOV_VERSION = 1,
	IOV_MSGLEVEL,
	IOV_BCMERRORSTR,
	IOV_BCMERROR,
	IOV_WDTICK,
	IOV_DUMP,
	IOV_CLEARCOUNTS,
	IOV_LOGDUMP,
	IOV_LOGCAL,
	IOV_LOGSTAMP,
	IOV_GPIOOB,
	IOV_IOCTLTIMEOUT,
#ifdef WLBTAMP
	IOV_HCI_CMD,		/* HCI command */
	IOV_HCI_ACL_DATA,	/* HCI data packet */
#endif
#if defined(DHD_DEBUG)
	IOV_CONS,
	IOV_DCONSOLE_POLL,
	IOV_DHD_JOIN_TIMEOUT_DBG,
	IOV_SCAN_TIMEOUT,
#endif /* defined(DHD_DEBUG) */
#ifdef PROP_TXSTATUS
	IOV_PROPTXSTATUS_ENABLE,
	IOV_PROPTXSTATUS_MODE,
	IOV_PROPTXSTATUS_OPT,
#ifdef QMONITOR
	IOV_QMON_TIME_THRES,
	IOV_QMON_TIME_PERCENT,
#endif /* QMONITOR */
	IOV_PROPTXSTATUS_MODULE_IGNORE,
	IOV_PROPTXSTATUS_CREDIT_IGNORE,
	IOV_PROPTXSTATUS_TXSTATUS_IGNORE,
	IOV_PROPTXSTATUS_RXPKT_CHK,
#endif /* PROP_TXSTATUS */
	IOV_BUS_TYPE,
#ifdef WLMEDIA_HTSF
	IOV_WLPKTDLYSTAT_SZ,
#endif
	IOV_CHANGEMTU,
	IOV_HOSTREORDER_FLOWS,
#ifdef DHDTCPACK_SUPPRESS
	IOV_TCPACK_SUPPRESS,
#endif /* DHDTCPACK_SUPPRESS */
#ifdef DHD_WMF
	IOV_WMF_BSS_ENAB,
	IOV_WMF_UCAST_IGMP,
	IOV_WMF_MCAST_DATA_SENDUP,
#ifdef DHD_IGMP_UCQUERY
	IOV_WMF_UCAST_IGMP_QUERY,
#endif /* DHD_IGMP_UCQUERY */
#ifdef DHD_UCAST_UPNP
	IOV_WMF_UCAST_UPNP,
#endif /* DHD_UCAST_UPNP */
	IOV_WMF_PSTA_DISABLE,
#endif /* DHD_WMF */
#if defined(BCM_ROUTER_DHD) || defined(TRAFFIC_MGMT_DWM)
	IOV_TRAFFIC_MGMT_DWM,
#endif /* BCM_ROUTER_DHD || TRAFFIC_MGMT_DWM */
	IOV_AP_ISOLATE,
#ifdef DHD_L2_FILTER
	IOV_DHCP_UNICAST,
	IOV_BLOCK_PING,
	IOV_PROXY_ARP,
	IOV_GRAT_ARP,
	IOV_BLOCK_TDLS,
#endif /* DHD_L2_FILTER */
	IOV_DHD_IE,
#ifdef DHD_PSTA
	IOV_PSTA,
#endif /* DHD_PSTA */
#ifdef DHD_WET
	IOV_WET,
	IOV_WET_HOST_IPV4,
	IOV_WET_HOST_MAC,
#endif /* DHD_WET */
	IOV_CFG80211_OPMODE,
#if (defined(BCM_ROUTER_DHD) && defined(BCM_GMAC3))
	IOV_DEV_DEF,
#endif
	IOV_ASSERT_TYPE,
#if !defined(BCM_ROUTER_DHD)
	IOV_LMTEST,
#endif
#ifdef DHD_MCAST_REGEN
	IOV_MCAST_REGEN_BSS_ENABLE,
#endif
	IOV_MACDBG_PD11REGS,
	IOV_MACDBG_PSVMPMEMS,
#if defined(DHD_LBR_AGGR_BCM_ROUTER)
	IOV_LBR_AGGR_LEN,
	IOV_LBR_AGGR_EN_MASK,
	IOV_LBR_AGGR_RELEASE_TIMEOUT,
#endif /* DHD_LBR_AGGR_BCM_ROUTER */
#if defined(BCM_DHD_RUNNER)
	IOV_RNR_PROFILE,
	IOV_RNR_POLICY,
	IOV_RNR_RXOFFL,
#endif /* BCM_DHD_RUNNER */
	IOV_DUMP_DONGLE, /**< dumps core registers and d11 memories */
	IOV_MACDBG_DUMP,
	IOV_1905_AL_UCAST,
	IOV_1905_AL_MCAST,
	IOV_MACDBG_DUMP_LEVEL,
#if defined(BCM_CPE_PKTC)
	IOV_PKTC,
	IOV_PKTCBND,
#endif /* BCM_CPE_PKTC */
	IOV_HMEMBYTES,
#ifdef WL_CFG80211
	IOV_EXTENDED_OPS_SUPPORT,
#endif /* WL_CFG80211 */
#ifdef DHD_IFE
	IOV_IFE_TIMEOUT,
	IOV_IFE_BUDGET,
#endif /* DHD_IFE */
	IOV_LAST
};

const bcm_iovar_t dhd_iovars[] = {
	/* name         varid                   flags   flags2 type     minlen */
	{"version",	IOV_VERSION,	0,  0,	IOVT_BUFFER,	sizeof(dhd_version) },
	{"msglevel",	IOV_MSGLEVEL,	0,	0,  IOVT_UINT32,	0 },
	{"bcmerrorstr", IOV_BCMERRORSTR, 0, 0,  IOVT_BUFFER,	BCME_STRLEN },
	{"bcmerror",	IOV_BCMERROR,	0,	0,  IOVT_INT8,	0 },
	{"wdtick",	IOV_WDTICK, 0,	0,  IOVT_UINT32,	0 },
	{"dump",	IOV_DUMP,	0,	0,  IOVT_BUFFER,	DHD_IOCTL_MAXLEN_HIGH },
#ifdef DHD_DEBUG
	{"cons",	IOV_CONS,	0,	0,  IOVT_BUFFER,	0 },
	{"dconpoll",	IOV_DCONSOLE_POLL, 0,	0,  IOVT_UINT32,	0 },
#endif
	{"clearcounts", IOV_CLEARCOUNTS, 0, 0,  IOVT_VOID,	0 },
#ifdef BCMPERFSTATS
	{"logdump", IOV_LOGDUMP,	0,	0,  IOVT_BUFFER,	DHD_IOCTL_MAXLEN },
	{"logcal",	IOV_LOGCAL, 0,	0,  IOVT_UINT32,	0 },
	{"logstamp",	IOV_LOGSTAMP,	0,	0,  IOVT_BUFFER,	0 },
#endif
	{"gpioob",	IOV_GPIOOB,	0,	0,  IOVT_UINT32,	0 },
	{"ioctl_timeout",	IOV_IOCTLTIMEOUT,	0,	0,  IOVT_UINT32,	0 },
#ifdef WLBTAMP
	{"HCI_cmd",	IOV_HCI_CMD,	0,	0,  IOVT_BUFFER,	0},
	{"HCI_ACL_data", IOV_HCI_ACL_DATA, 0,	0,  IOVT_BUFFER,	0},
#endif
#ifdef PROP_TXSTATUS
	{"proptx",	IOV_PROPTXSTATUS_ENABLE,	0,	0,  IOVT_BOOL,	0 },
	/*
	set the proptxtstatus operation mode:
	0 - Do not do any proptxtstatus flow control
	1 - Use implied credit from a packet status
	2 - Use explicit credit
	*/
	{"ptxmode",	IOV_PROPTXSTATUS_MODE,	0,	0,  IOVT_UINT32,	0 },
	{"proptx_opt", IOV_PROPTXSTATUS_OPT,	0,	0,  IOVT_UINT32,	0 },
#ifdef QMONITOR
	{"qtime_thres",	IOV_QMON_TIME_THRES,	0,	0,  IOVT_UINT32,	0 },
	{"qtime_percent", IOV_QMON_TIME_PERCENT, 0,	0,  IOVT_UINT32,	0 },
#endif /* QMONITOR */
	{"pmodule_ignore", IOV_PROPTXSTATUS_MODULE_IGNORE, 0, 0,    IOVT_BOOL, 0 },
	{"pcredit_ignore", IOV_PROPTXSTATUS_CREDIT_IGNORE, 0, 0,    IOVT_BOOL, 0 },
	{"ptxstatus_ignore", IOV_PROPTXSTATUS_TXSTATUS_IGNORE, 0, 0,    IOVT_BOOL, 0 },
	{"rxpkt_chk", IOV_PROPTXSTATUS_RXPKT_CHK, 0, 0, IOVT_BOOL, 0 },
#endif /* PROP_TXSTATUS */
	{"bustype", IOV_BUS_TYPE, 0, 0, IOVT_UINT32, 0},
#ifdef WLMEDIA_HTSF
	{"pktdlystatsz", IOV_WLPKTDLYSTAT_SZ, 0, 0, IOVT_UINT8, 0 },
#endif
	{"changemtu", IOV_CHANGEMTU, 0, 0,  IOVT_UINT32, 0 },
	{"host_reorder_flows", IOV_HOSTREORDER_FLOWS, 0, 0, IOVT_BUFFER,
	(WLHOST_REORDERDATA_MAXFLOWS + 1) },
#ifdef DHDTCPACK_SUPPRESS
	{"tcpack_suppress",	IOV_TCPACK_SUPPRESS,	0,	0,  IOVT_UINT8,	0 },
#endif /* DHDTCPACK_SUPPRESS */
#ifdef DHD_WMF
	{"wmf_bss_enable", IOV_WMF_BSS_ENAB,	0,	0,  IOVT_BOOL,	0 },
	{"wmf_ucast_igmp", IOV_WMF_UCAST_IGMP,	0,	0,  IOVT_BOOL,	0 },
	{"wmf_mcast_data_sendup", IOV_WMF_MCAST_DATA_SENDUP,	0,	0,  IOVT_BOOL,	0 },
#ifdef DHD_IGMP_UCQUERY
	{"wmf_ucast_igmp_query", IOV_WMF_UCAST_IGMP_QUERY, (0), 0,  IOVT_BOOL, 0 },
#endif /* DHD_IGMP_UCQUERY */
#ifdef DHD_UCAST_UPNP
	{"wmf_ucast_upnp", IOV_WMF_UCAST_UPNP, (0), 0,  IOVT_BOOL, 0 },
#endif /* DHD_UCAST_UPNP */
	{"wmf_psta_disable", IOV_WMF_PSTA_DISABLE, (0), 0,  IOVT_BOOL, 0 },
#endif /* DHD_WMF */
#if defined(BCM_ROUTER_DHD) || defined(TRAFFIC_MGMT_DWM)
	{"trf_mgmt_filters_add", IOV_TRAFFIC_MGMT_DWM, (0), 0,  IOVT_BUFFER, 0},
#endif /* BCM_ROUTER_DHD || TRAFFIC_MGMT_DWM */
#ifdef DHD_L2_FILTER
	{"dhcp_unicast", IOV_DHCP_UNICAST, (0), 0,  IOVT_BOOL, 0 },
#endif /* DHD_L2_FILTER */
	{"ap_isolate", IOV_AP_ISOLATE, (0), 0,  IOVT_BOOL, 0},
#ifdef DHD_L2_FILTER
	{"block_ping", IOV_BLOCK_PING, (0), 0,  IOVT_BOOL, 0},
	{"proxy_arp", IOV_PROXY_ARP, (0), 0,    IOVT_BOOL, 0},
	{"grat_arp", IOV_GRAT_ARP, (0), 0,  IOVT_BOOL, 0},
	{"block_tdls", IOV_BLOCK_TDLS, (0), 0,  IOVT_BOOL, 0},
#endif /* DHD_L2_FILTER */
	{"dhd_ie", IOV_DHD_IE, (0), 0,  IOVT_BUFFER, 0},
#ifdef DHD_PSTA
	/* PSTA/PSR Mode configuration. 0: DIABLED 1: PSTA 2: PSR */
	{"psta", IOV_PSTA, 0, 0,    IOVT_UINT32, 0},
#endif /* DHD PSTA */
#ifdef DHD_WET
	/* WET Mode configuration. 0: DIABLED 1: WET */
	{"wet", IOV_WET, 0, 0,  IOVT_UINT32, 0},
	{"wet_host_ipv4", IOV_WET_HOST_IPV4, 0, 0,  IOVT_BUFFER, sizeof(wet_host_t)},
	{"wet_host_mac", IOV_WET_HOST_MAC, 0, 0,    IOVT_BUFFER, sizeof(wet_host_t)},
#endif /* DHD WET */
	{"op_mode",	IOV_CFG80211_OPMODE,	0,	0,  IOVT_UINT32,	0 },
#if (defined(BCM_ROUTER_DHD) && defined(BCM_GMAC3))
	{"dev_def", IOV_DEV_DEF, (0), 0,    IOVT_BOOL, 0},
#endif
	{"assert_type", IOV_ASSERT_TYPE, (0), 0,    IOVT_UINT32, 0},
#if !defined(BCM_ROUTER_DHD)
	{"lmtest", IOV_LMTEST,	0,	0,  IOVT_UINT32,	0 },
#endif
#ifdef DHD_MCAST_REGEN
	{"mcast_regen_bss_enable", IOV_MCAST_REGEN_BSS_ENABLE, 0, 0,    IOVT_BOOL, 0},
#endif
	{"pd11regs", IOV_MACDBG_PD11REGS, 0, 0, IOVT_BUFFER, 0},
	{"psvmpmems", IOV_MACDBG_PSVMPMEMS, 0, 0,   IOVT_BUFFER, 0},
#ifdef DHD_LBR_AGGR_BCM_ROUTER
	{"lbr_aggr_len", IOV_LBR_AGGR_LEN, 0, 0,    IOVT_UINT32, 0 },
	{"lbr_aggr_en_mask", IOV_LBR_AGGR_EN_MASK, 0, 0,    IOVT_UINT32, 0 },
	{"lbr_aggr_release_timeout", IOV_LBR_AGGR_RELEASE_TIMEOUT, 0,	0,  IOVT_UINT32, 0 },
#endif /* DHD_LBR_AGGR_BCM_ROUTER */
#if defined(BCM_DHD_RUNNER)
	{"rnr_flowring_profile", IOV_RNR_PROFILE, 0, 0, IOVT_BUFFER, 1 },
	{"rnr_flowring_policy",  IOV_RNR_POLICY, 0, 0,  IOVT_BUFFER, 1 },
	{"rnr_rxoffload", IOV_RNR_RXOFFL, 0, 0, IOVT_BUFFER, 1 },
#endif /* BCM_DHD_RUNNER */
	{"dump_dongle", IOV_DUMP_DONGLE, 0, 0,  IOVT_BUFFER,
	MAX(sizeof(dump_dongle_in_t), sizeof(dump_dongle_out_t)) },
	{"macdbg_dump", IOV_MACDBG_DUMP, 0, 0, IOVT_BUFFER, 0},
#ifdef BCM_GMAC3
	{"1905_al_ucast", IOV_1905_AL_UCAST, 0, 0, IOVT_BUFFER, ETHER_ADDR_LEN},
	{"1905_al_mcast", IOV_1905_AL_MCAST, 0, 0, IOVT_BUFFER, ETHER_ADDR_LEN},
#endif /* BCM_GMAC3 */
	{"macdbg_dump_level", IOV_MACDBG_DUMP_LEVEL, 0, 0, IOVT_UINT32, 0},
#if defined(BCM_CPE_PKTC)
	{"pktc",    IOV_PKTC, 0, 0, IOVT_BOOL, 0 },
	{"pktcbnd", IOV_PKTCBND, 0, 0, IOVT_UINT32, 0 },
#endif /* BCM_CPE_PKTC */
	{"hmembytes",	IOV_HMEMBYTES, 0, 0,  IOVT_BUFFER,	3 * sizeof(uint) },
#ifdef WL_CFG80211
	{"extended_ops_support",	IOV_EXTENDED_OPS_SUPPORT,	0,	0,  IOVT_BOOL,	0 },
#endif /* WL_CFG80211 */
#ifdef DHD_IFE
	{"ife_timeout",	IOV_IFE_TIMEOUT,	0,	0,  IOVT_UINT32,	0 },
	{"ife_budget",	IOV_IFE_BUDGET,	0,	0,  IOVT_UINT32,	0 },
#endif /* DHD_IFE */
	{NULL, 0, 0, 0, 0, 0 }
};

#define DHD_IOVAR_BUF_SIZE	128

#ifdef DHD_FW_COREDUMP
void
dhd_save_fwdump(dhd_pub_t *dhd_pub, void * buffer, uint32 length)
{
	if (dhd_pub->soc_ram) {
#if defined(CONFIG_DHD_USE_STATIC_BUF) && defined(DHD_USE_STATIC_MEMDUMP)
		DHD_OS_PREFREE(dhd_pub, dhd_pub->soc_ram, dhd_pub->soc_ram_length);
#else
		MFREE(dhd_pub->osh, dhd_pub->soc_ram, dhd_pub->soc_ram_length);
#endif /* CONFIG_DHD_USE_STATIC_BUF && DHD_USE_STATIC_MEMDUMP */
		dhd_pub->soc_ram = NULL;
		dhd_pub->soc_ram_length = 0;
	}

#if defined(CONFIG_DHD_USE_STATIC_BUF) && defined(DHD_USE_STATIC_MEMDUMP)
	dhd_pub->soc_ram = (uint8*)DHD_OS_PREALLOC(dhd_pub,
		DHD_PREALLOC_MEMDUMP_RAM, length);
	memset(dhd_pub->soc_ram, 0, length);
#else
	dhd_pub->soc_ram = (uint8*) MALLOCZ(dhd_pub->osh, length);
#endif /* CONFIG_DHD_USE_STATIC_BUF && DHD_USE_STATIC_MEMDUMP */
	if (dhd_pub->soc_ram == NULL) {
		DHD_ERROR(("%s: Failed to allocate memory for fw crash snap shot.\n",
			__FUNCTION__));
		return;
	}

	dhd_pub->soc_ram_length = length;
	memcpy(dhd_pub->soc_ram, buffer, length);
}
#endif /* DHD_FW_COREDUMP */

static int
dhd_dump(dhd_pub_t *dhdp, char *buf, int buflen)
{
	char eabuf[ETHER_ADDR_STR_LEN];

	struct bcmstrbuf b;
	struct bcmstrbuf *strbuf = &b;

	bcm_binit(strbuf, buf, buflen);

	/* Base DHD info */
	bcm_bprintf(strbuf, "%s\n", dhd_version);
	bcm_bprintf(strbuf, "\n");
	bcm_bprintf(strbuf, "pub.up %d pub.txoff %d pub.busstate %d\n",
	            dhdp->up, dhdp->txoff, dhdp->busstate);
	bcm_bprintf(strbuf, "pub.hdrlen %u pub.maxctl %u pub.rxsz %u\n",
	            dhdp->hdrlen, dhdp->maxctl, dhdp->rxsz);
	bcm_bprintf(strbuf, "pub.iswl %d pub.drv_version %ld pub.mac %s\n",
	            dhdp->iswl, dhdp->drv_version, bcm_ether_ntoa(&dhdp->mac, eabuf));
	bcm_bprintf(strbuf, "pub.bcmerror %d tickcnt %u\n", dhdp->bcmerror, dhdp->tickcnt);

	bcm_bprintf(strbuf, "dongle stats:\n");
	bcm_bprintf(strbuf, "tx_packets %lu tx_bytes %lu tx_errors %lu tx_dropped %lu\n",
	            dhdp->dstats.tx_packets, dhdp->dstats.tx_bytes,
	            dhdp->dstats.tx_errors, dhdp->dstats.tx_dropped);
	bcm_bprintf(strbuf, "rx_packets %lu rx_bytes %lu rx_errors %lu rx_dropped %lu\n",
	            dhdp->dstats.rx_packets, dhdp->dstats.rx_bytes,
	            dhdp->dstats.rx_errors, dhdp->dstats.rx_dropped);
	bcm_bprintf(strbuf, "multicast %lu\n", dhdp->dstats.multicast);

	bcm_bprintf(strbuf, "bus stats:\n");
	bcm_bprintf(strbuf, "tx_packets %lu  tx_dropped %lu tx_multicast %lu tx_errors %lu\n",
	            dhdp->tx_packets, dhdp->tx_dropped, dhdp->tx_multicast, dhdp->tx_errors);
	bcm_bprintf(strbuf, "tx_ctlpkts %lu tx_ctlerrs %lu\n",
	            dhdp->tx_ctlpkts, dhdp->tx_ctlerrs);
	bcm_bprintf(strbuf, "rx_packets %lu rx_multicast %lu rx_errors %lu \n",
	            dhdp->rx_packets, dhdp->rx_multicast, dhdp->rx_errors);
	bcm_bprintf(strbuf, "rx_ctlpkts %lu rx_ctlerrs %lu rx_dropped %lu\n",
	            dhdp->rx_ctlpkts, dhdp->rx_ctlerrs, dhdp->rx_dropped);
	bcm_bprintf(strbuf, "rx_readahead_cnt %lu tx_realloc %lu\n",
	            dhdp->rx_readahead_cnt, dhdp->tx_realloc);
	bcm_bprintf(strbuf, "tx_pktgetfail %lu rx_pktgetfail %lu rx_pktidfail %lu\n",
	            dhdp->tx_pktgetfail, dhdp->rx_pktgetfail, dhdp->rx_pktidfail);

#ifdef BCM_WFD
	dhd_wfd_dump(dhdp, strbuf);
#endif

#ifdef BCM_NBUFF_WLMCAST
	dhd_nbuff_dump(dhdp, strbuf);
#endif

	bcm_bprintf(strbuf, "\n");

	/* Add any prot info */
	dhd_prot_dump(dhdp, strbuf);
	bcm_bprintf(strbuf, "\n");

	/* Add any bus info */
	dhd_bus_dump(dhdp, strbuf);

#if defined(DHD_LB_STATS)
	dhd_lb_stats_dump(dhdp, strbuf);
#endif /* DHD_LB_STATS */
#ifdef DHD_WET
	if (dhd_get_wet_mode(dhdp))
		dhd_wet_dump(dhdp, strbuf);
#endif /* DHD_WET */
#ifdef BCM_PKTFWD
	dhd_pktfwd_radio_dump(dhdp, strbuf);
#endif /* BCM_PKTFWD */
#if defined(BCM_CPE_PKTC)
	dhd_pktc_dump((void *)dhdp, (void *)strbuf);
#endif /* BCM_CPE_PKTC */
#if defined(BCM_AWL)
	dhd_awl_dump(dhdp, strbuf);
#endif /* BCM_AWL */

	return (!strbuf->size ? BCME_BUFTOOSHORT : 0);
}

void
dhd_dump_to_kernelog(dhd_pub_t *dhdp)
{
	char buf[512];

	bcm_bprintf_bypass = TRUE;
	dhd_dump(dhdp, buf, sizeof(buf));
	bcm_bprintf_bypass = FALSE;
}

int
dhd_wl_ioctl_cmd(dhd_pub_t *dhd_pub, int cmd, void *arg, int len, uint8 set, int ifidx)
{
	wl_ioctl_t ioc;

	ioc.cmd = cmd;
	ioc.buf = arg;
	ioc.len = len;
	ioc.set = set;

	return dhd_wl_ioctl(dhd_pub, ifidx, &ioc, arg, len);
}

int
dhd_wl_ioctl_get_intiovar(dhd_pub_t *dhd_pub, char *name, uint *pval,
	int cmd, uint8 set, int ifidx)
{
	char iovbuf[WLC_IOCTL_SMLEN];
	int ret = -1;

	memset(iovbuf, 0, sizeof(iovbuf));
	if (bcm_mkiovar(name, NULL, 0, iovbuf, sizeof(iovbuf))) {
		ret = dhd_wl_ioctl_cmd(dhd_pub, cmd, iovbuf, sizeof(iovbuf), set, ifidx);
		if (!ret) {
			*pval = ltoh32(*((uint*)iovbuf));
		} else {
			DHD_ERROR(("%s: get int iovar %s failed, ERR %d\n",
				__FUNCTION__, name, ret));
		}
	} else {
		DHD_ERROR(("%s: mkiovar %s failed\n",
			__FUNCTION__, name));
	}

	return ret;
}

int
dhd_wl_ioctl_set_intiovar(dhd_pub_t *dhd_pub, char *name, uint val,
	int cmd, uint8 set, int ifidx)
{
	char iovbuf[WLC_IOCTL_SMLEN];
	int ret = -1;
	int lval = htol32(val);
	uint len;

	len = bcm_mkiovar(name, (char*)&lval, sizeof(lval), iovbuf, sizeof(iovbuf));

	if (len) {
		ret = dhd_wl_ioctl_cmd(dhd_pub, cmd, iovbuf, len, set, ifidx);
		if (ret) {
			DHD_ERROR(("%s: set int iovar %s failed, ERR %d\n",
				__FUNCTION__, name, ret));
		}
	} else {
		DHD_ERROR(("%s: mkiovar %s failed\n",
			__FUNCTION__, name));
	}

	return ret;
}

static struct ioctl2str_s {
	uint32 ioctl;
	char *name;
} ioctl2str_array[] = {
	{WLC_GET_MAGIC, "GET_MAGIC"},
	{WLC_GET_VERSION, "GET_VERSION"},
	{WLC_UP, "UP"},
	{WLC_DOWN, "DOWN"},
	{WLC_SET_PROMISC, "SET_PROMISC"},
	{WLC_SET_INFRA, "SET_INFRA"},
	{WLC_SET_AUTH, "SET_AUTH"},
	{WLC_GET_SSID, "GET_SSID"},
	{WLC_SET_SSID, "SET_SSID"},
	{WLC_RESTART, "RESTART"},
	{WLC_GET_CHANNEL, "GET_CHAN"},
	{WLC_SET_CHANNEL, "SET_CHAN"},
	{WLC_SET_RATE_PARAMS, "SET_RATE_PARS"},
	{WLC_SET_KEY, "SET_KEY"},
	{WLC_SCAN, "SCAN"},
	{WLC_DISASSOC, "DISASSOC"},
	{WLC_REASSOC, "REASSOC"},
	{WLC_SET_COUNTRY, "SET_COUNTRY"},
	{WLC_SET_WAKE, "SET_WAKE"},
	{WLC_SET_SCANSUPPRESS, "SET_SCANSUPP"},
	{WLC_SCB_AUTHORIZE, "SCB_AUTH"},
	{WLC_SCB_DEAUTHORIZE, "SCB_DEAUTH"},
	{WLC_SET_WSEC, "SET_WSEC"},
	{WLC_SET_INTERFERENCE_MODE, "SET_INTERF_MODE"},
	{WLC_SET_RADAR, "SET_RADAR"},
	{WLC_SET_ROAM_SCAN_PERIOD, "SET_ROAM_SNPER"},
	{WLC_SET_ANTDIV, "SET_ANTDIV"},
	{WLC_GET_RSSI, "GET_RSSI"},
	{0, NULL}
};

static char *
ioctl2str(uint32 ioctl)
{
	struct ioctl2str_s *p = ioctl2str_array;

	while (p->name != NULL) {
		if (p->ioctl == ioctl) {
			return p->name;
		}
		p++;
	}

	return "";
}

/**
 * @param ioc          IO control struct, members are partially used by this function.
 * @param buf [inout]  Contains parameters to send to dongle, contains dongle response on return.
 * @param len          Maximum number of bytes that dongle is allowed to write into 'buf'.
 *
 * Note that 'buf' and 'ioc->buf' point at different buffers.
 */
int
dhd_wl_ioctl(dhd_pub_t *dhd_pub, int ifidx, wl_ioctl_t *ioc, void *buf, int len)
{
	int ret = BCME_ERROR;

	if (dhd_os_proto_block(dhd_pub))
	{
		/* logging of iovars that are send to the dongle, ./dhd msglevel +s_iov */
		bool log_ioctl = ((dhd_msg_level & DHD_DNGL_IOVAR_GET_VAL) && ioc->set == FALSE) ||
				((dhd_msg_level & DHD_DNGL_IOVAR_SET_VAL) && ioc->set == TRUE);
		char *pars = (char *)ioc->buf; // points at user buffer

		/* blocking version and magic ioctl */
		if (ioc->cmd == WLC_GET_VERSION && dhd_pub->wl_ioctl_version != 0 && buf) {
			memcpy(buf, &(dhd_pub->wl_ioctl_version), sizeof(int));
			dhd_os_proto_unblock(dhd_pub);
			return BCME_OK;
		} else if (ioc->cmd == WLC_GET_MAGIC && dhd_pub->wl_ioctl_magic != 0 && buf) {
			memcpy(buf, &(dhd_pub->wl_ioctl_magic), sizeof(int));
			dhd_os_proto_unblock(dhd_pub);
			return BCME_OK;
		}

		if (log_ioctl) {
			char *getset = (ioc->set == TRUE ? "set": "get");

			if (ioc->cmd == WLC_GET_VAR || ioc->cmd == WLC_SET_VAR) {
				printf("%s: iovar:%d: %s %s \n",
					dhd_ifname(dhd_pub, ifidx), ifidx, getset, pars);
				if (ioc->len > 1 + sizeof(uint32)) {
					// skip iovar name:
					if (pars != NULL) {
						pars += strnlen(pars,
							ioc->len - 1 - sizeof(uint32));
						pars++;               // skip NULL character
					}
				}
			} else {
				printf("%s: ioctl:%d: %s %d %s \n", dhd_ifname(dhd_pub, ifidx),
					ifidx, getset, ioc->cmd, ioctl2str(ioc->cmd));
			}

			if (ioc->set == TRUE) {
				if (pars != NULL) {
					printf(" 0x%x\n", *(uint32*)pars);
				} else {
					printf(" NULL\n");
				}
			}
		}

#if defined(WL_WLC_SHIM)
		struct wl_shim_node *shim = dhd_pub_shim(dhd_pub);

		wl_io_pport_t io_pport;
		io_pport.dhd_pub = dhd_pub;
		io_pport.ifidx = ifidx;

		ret = wl_shim_ioctl(shim, ioc, len, &io_pport);
		if (ret != BCME_OK) {
			DHD_TRACE(("%s: wl_shim_ioctl(%d) ERR %d\n", __FUNCTION__, ioc->cmd, ret));
		}
#else
		ret = dhd_prot_ioctl(dhd_pub, ifidx, ioc, buf, len); /* does not r/w 'ioc->buf' */
#endif /* defined(WL_WLC_SHIM) */

#if defined(OEM_ANDROID)
		if (ret && dhd_pub->up) {
			/* Send hang event only if dhd_open() was success */
			dhd_os_check_hang(dhd_pub, ifidx, ret);
		}

		if (ret == -ETIMEDOUT && !dhd_pub->up) {
			DHD_ERROR(("%s: 'resumed on timeout' error is "
				"occurred before the interface does not"
				" bring up\n", __FUNCTION__));
			dhd_pub->busstate = DHD_BUS_DOWN;
		}
#endif /* defined(OEM_ANDROID) */

		if (log_ioctl && ioc->set == FALSE) {
			if (buf != NULL) {
				printf(" 0x%x", *(uint32*)buf);
			}
			printf("\n");
		}

		/* only cache the version and magic ioctl value at first time */
		if (ioc->cmd == WLC_GET_VERSION && ret == BCME_OK && buf) {
			memcpy(&(dhd_pub->wl_ioctl_version), buf, sizeof(int));
		} else if (ioc->cmd == WLC_GET_MAGIC && ret == BCME_OK && buf) {
			memcpy(&(dhd_pub->wl_ioctl_magic), buf, sizeof(int));
		}

		dhd_os_proto_unblock(dhd_pub);

#ifdef DETAIL_DEBUG_LOG_FOR_IOCTL
		if (ret < 0) {
			if (ioc->cmd == WLC_GET_VAR || ioc->cmd == WLC_SET_VAR) {
				if (ret == BCME_UNSUPPORTED || ret == BCME_NOTASSOCIATED) {
					DHD_ERROR(("%s: %s: %s, %s\n",
						__FUNCTION__, ioc->cmd == WLC_GET_VAR ?
						"WLC_GET_VAR" : "WLC_SET_VAR",
						(char *)ioc->buf,
						ret == BCME_UNSUPPORTED ? "UNSUPPORTED"
						: "NOT ASSOCIATED"));
				} else {
					DHD_ERROR(("%s: %s: %s, ret = %d\n",
						__FUNCTION__, ioc->cmd == WLC_GET_VAR ?
						"WLC_GET_VAR" : "WLC_SET_VAR",
						(char *)ioc->buf, ret));
				}
			} else {
				if (ret == BCME_UNSUPPORTED || ret == BCME_NOTASSOCIATED) {
					DHD_ERROR(("%s: WLC_IOCTL: cmd: %d, %s\n",
						__FUNCTION__, ioc->cmd,
						ret == BCME_UNSUPPORTED ? "UNSUPPORTED" :
						"NOT ASSOCIATED"));
				} else {
					DHD_ERROR(("%s: WLC_IOCTL: cmd: %d, ret = %d\n",
						__FUNCTION__, ioc->cmd, ret));
				}
			}
		}
#endif /* DETAIL_DEBUG_LOG_FOR_IOCTL */
	}

	return ret;
}

uint
wl_get_port_num(wl_io_pport_t *io_pport)
{
	return 0;
}

/* Get bssidx from iovar params
 * Input:   dhd_pub - pointer to dhd_pub_t
 *	    params  - IOVAR params
 * Output:  idx	    - BSS index
 *	    val	    - ponter to the IOVAR arguments
 */
static int
dhd_iovar_parse_bssidx(dhd_pub_t *dhd_pub, const char *params, int *idx, const char **val)
{
	char *prefix = "bsscfg:";
	uint32	bssidx;

	if (!(strncmp(params, prefix, strlen(prefix)))) {
		/* per bss setting should be prefixed with 'bsscfg:' */
		const char *p = params + strlen(prefix);

		/* Skip Name */
		while (*p != '\0')
			p++;
		/* consider null */
		p = p + 1;
		bcopy(p, &bssidx, sizeof(uint32));
		/* Get corresponding dhd index */
		bssidx = dhd_bssidx2idx(dhd_pub, htod32(bssidx));

		if (bssidx >= DHD_MAX_IFS) {
			DHD_ERROR(("%s Wrong bssidx provided\n", __FUNCTION__));
			return BCME_ERROR;
		}

		/* skip bss idx */
		p += sizeof(uint32);
		*val = p;
		*idx = bssidx;
	} else {
		DHD_ERROR(("%s: bad parameter for per bss iovar\n", __FUNCTION__));
		return BCME_ERROR;
	}

	return BCME_OK;
}

#if defined(DHD_DEBUG) && defined(BCMDHDUSB)
/* USB Device console input function */
int
dhd_bus_console_in(dhd_pub_t *dhd, uchar *msg, uint msglen)
{
	DHD_TRACE(("%s \n", __FUNCTION__));

	return dhd_iovar(dhd, 0, "cons", msg, msglen, NULL, 0, TRUE);

}
#endif /* DHD_DEBUG && BCMDHDUSB  */

static int
dhd_doiovar(dhd_pub_t *dhd_pub, const bcm_iovar_t *vi, uint32 actionid, const char *name,
            void *params, int plen, void *arg, int len, int val_size)
{
	int bcmerror = 0;
	int32 int_val = 0;

	DHD_TRACE(("%s: Enter\n", __FUNCTION__));
	DHD_TRACE(("%s: actionid = %d; name %s\n", __FUNCTION__, actionid, name));

	if ((bcmerror = bcm_iovar_lencheck(vi, arg, len, IOV_ISSET(actionid))) != 0)
		goto exit;

	if (plen >= (int)sizeof(int_val))
		bcopy(params, &int_val, sizeof(int_val));

	switch (actionid) {
	case IOV_GVAL(IOV_VERSION):
		/* Need to have checked buffer length */
		bcm_strncpy_s((char*)arg, len, dhd_version, len);
		break;

	case IOV_GVAL(IOV_MSGLEVEL):
		int_val = (int32)dhd_msg_level;
		bcopy(&int_val, arg, val_size);
		break;

	case IOV_SVAL(IOV_MSGLEVEL):
#ifdef DHD_DEBUG
#ifdef WL_CFG80211
		/* XXX Usage of this enhanced IOV
		 * dhd -i wlan0 msglevel 0x8001
		 * change dhd msg level to DHD_REORDER_VAL & DHD_ERROR_VAL
		 * dhd -i wlan0 msglevel 0x18001
		 * change dhd msg level to DHD_REORDER_VAL & DHD_ERROR_VAL
		 * and enable WL_DBG_DBG
		 * dhd -i wlan0 msglevel 0x200ff
		 * change wl debug msg level only ,
		 * do not change dhd msg level enable whole WL msg level
		 */
		/* Enable DHD and WL logs in oneshot */
		if (int_val & DHD_WL_VAL2)
			wl_cfg80211_enable_trace(TRUE, int_val & (~DHD_WL_VAL2));
		else if (int_val & DHD_WL_VAL)
			wl_cfg80211_enable_trace(FALSE, WL_DBG_DBG);
		if (!(int_val & DHD_WL_VAL2))
#endif /* WL_CFG80211 */
#endif /* DHD_DEBUG */
		dhd_msg_level = int_val;
		break;

	case IOV_GVAL(IOV_BCMERRORSTR):
		bcm_strncpy_s((char *)arg, len, bcmerrorstr(dhd_pub->bcmerror), BCME_STRLEN);
		((char *)arg)[BCME_STRLEN - 1] = 0x00;
		break;

	case IOV_GVAL(IOV_BCMERROR):
		int_val = (int32)dhd_pub->bcmerror;
		bcopy(&int_val, arg, val_size);
		break;

#ifndef BCMDBUS
	case IOV_GVAL(IOV_WDTICK):
		int_val = (int32)dhd_watchdog_ms;
		bcopy(&int_val, arg, val_size);
		break;
#endif

	case IOV_SVAL(IOV_WDTICK):
		if (!dhd_pub->up) {
			bcmerror = BCME_NOTUP;
			break;
		}

		if (CUSTOM_DHD_WATCHDOG_MS == 0 && int_val == 0) {
			dhd_watchdog_ms = (uint)int_val;
		}

		dhd_os_wd_timer(dhd_pub, (uint)int_val);
		break;

	case IOV_GVAL(IOV_DUMP):
		bcmerror = dhd_dump(dhd_pub, arg, len);
		break;

#ifndef BCMDBUS
#ifdef DHD_DEBUG
	case IOV_GVAL(IOV_DCONSOLE_POLL):
		int_val = (int32)dhd_pub->dhd_console_ms;
		bcopy(&int_val, arg, val_size);
		break;

	case IOV_SVAL(IOV_DCONSOLE_POLL):
		dhd_pub->dhd_console_ms = (uint)int_val;
		break;

	case IOV_SVAL(IOV_CONS):
		if (len > 0)
			bcmerror = dhd_bus_console_in(dhd_pub, arg, len - 1);
		break;
#endif /* DHD_DEBUG */
#endif /* BCMDBUS */

	case IOV_SVAL(IOV_CLEARCOUNTS):
		dhd_pub->tx_packets = dhd_pub->rx_packets = 0;
		dhd_pub->tx_errors = dhd_pub->rx_errors = 0;
		dhd_pub->tx_ctlpkts = dhd_pub->rx_ctlpkts = 0;
		dhd_pub->tx_ctlerrs = dhd_pub->rx_ctlerrs = 0;
		dhd_pub->tx_dropped = 0;
		dhd_pub->rx_dropped = 0;
		dhd_pub->tx_pktgetfail = 0;
		dhd_pub->rx_pktgetfail = 0;
		dhd_pub->rx_readahead_cnt = 0;
		dhd_pub->tx_realloc = 0;
		dhd_pub->wd_dpc_sched = 0;
		dhd_pub->tx_multicast = 0;
		dhd_pub->rx_multicast = 0;
		dhd_pub->rx_flushed = 0;
		dhd_pub->rx_pktidfail = 0;
		dhd_pub->fc_packets = 0;
#ifdef BCM_WFD
		dhd_wfd_clear_dump(dhd_pub);
#endif
		memset(&dhd_pub->dstats, 0, sizeof(dhd_pub->dstats));
		dhd_bus_clearcounts(dhd_pub);
#ifdef PROP_TXSTATUS
		/* clear proptxstatus related counters */
		dhd_wlfc_clear_counts(dhd_pub);
#endif /* PROP_TXSTATUS */
		DHD_LB_STATS_RESET(dhd_pub);
		break;

#ifdef BCMPERFSTATS
	case IOV_GVAL(IOV_LOGDUMP): {
		bcmdumplog((char*)arg, len);
		break;
	}

	case IOV_SVAL(IOV_LOGCAL): {
		bcmlog("Starting OSL_DELAY (%d usecs)", (uint)int_val, 0);
		OSL_DELAY((uint)int_val);
		bcmlog("Finished OSL_DELAY (%d usecs)", (uint)int_val, 0);
		break;
	}

	case IOV_SVAL(IOV_LOGSTAMP): {
		int int_val2;

		if (plen >= 2 * sizeof(int)) {
			bcopy((char *)params + sizeof(int_val), &int_val2, sizeof(int_val2));
			bcmlog("User message %d %d", (uint)int_val, (uint)int_val2);
		} else if (plen >= sizeof(int)) {
			bcmlog("User message %d", (uint)int_val, 0);
		} else {
			bcmlog("User message", 0, 0);
		}
		break;
	}
#endif /* BCMPERFSTATS */

	case IOV_GVAL(IOV_IOCTLTIMEOUT): {
		int_val = (int32)dhd_os_get_ioctl_resp_timeout();
		bcopy(&int_val, arg, sizeof(int_val));
		break;
	}

	case IOV_SVAL(IOV_IOCTLTIMEOUT): {
		if (int_val <= 0)
			bcmerror = BCME_BADARG;
		else
			dhd_os_set_ioctl_resp_timeout((unsigned int)int_val);
		break;
	}

#ifdef WLBTAMP
	case IOV_SVAL(IOV_HCI_CMD): {
		amp_hci_cmd_t *cmd = (amp_hci_cmd_t *)arg;

		/* sanity check: command preamble present */
		if (len < HCI_CMD_PREAMBLE_SIZE)
			return BCME_BUFTOOSHORT;

		/* sanity check: command parameters are present */
		if (len < (int)(HCI_CMD_PREAMBLE_SIZE + cmd->plen))
			return BCME_BUFTOOSHORT;

		dhd_bta_docmd(dhd_pub, cmd, len);
		break;
	}

	case IOV_SVAL(IOV_HCI_ACL_DATA): {
		amp_hci_ACL_data_t *ACL_data = (amp_hci_ACL_data_t *)arg;

		/* sanity check: HCI header present */
		if (len < HCI_ACL_DATA_PREAMBLE_SIZE)
			return BCME_BUFTOOSHORT;

		/* sanity check: ACL data is present */
		if (len < (int)(HCI_ACL_DATA_PREAMBLE_SIZE + ACL_data->dlen))
			return BCME_BUFTOOSHORT;

		dhd_bta_tx_hcidata(dhd_pub, ACL_data, len);
		break;
	}
#endif /* WLBTAMP */

#ifdef PROP_TXSTATUS
	case IOV_GVAL(IOV_PROPTXSTATUS_ENABLE): {
		bool wlfc_enab = FALSE;
		bcmerror = dhd_wlfc_get_enable(dhd_pub, &wlfc_enab);
		if (bcmerror != BCME_OK)
			goto exit;
		int_val = wlfc_enab ? 1 : 0;
		bcopy(&int_val, arg, val_size);
		break;
	}
	case IOV_SVAL(IOV_PROPTXSTATUS_ENABLE): {
		bool wlfc_enab = FALSE;
		bcmerror = dhd_wlfc_get_enable(dhd_pub, &wlfc_enab);
		if (bcmerror != BCME_OK)
			goto exit;

		/* wlfc is already set as desired */
		if (wlfc_enab == (int_val == 0 ? FALSE : TRUE))
			goto exit;

		if (int_val == TRUE)
			bcmerror = dhd_wlfc_init(dhd_pub);
		else
			bcmerror = dhd_wlfc_deinit(dhd_pub);

		break;
	}
	case IOV_GVAL(IOV_PROPTXSTATUS_MODE):
		bcmerror = dhd_wlfc_get_mode(dhd_pub, &int_val);
		if (bcmerror != BCME_OK)
			goto exit;
		bcopy(&int_val, arg, val_size);
		break;

	case IOV_SVAL(IOV_PROPTXSTATUS_MODE):
		dhd_wlfc_set_mode(dhd_pub, int_val);
		break;
#ifdef QMONITOR
	case IOV_GVAL(IOV_QMON_TIME_THRES): {
		int_val = dhd_qmon_thres(dhd_pub, FALSE, 0);
		bcopy(&int_val, arg, val_size);
		break;
	}

	case IOV_SVAL(IOV_QMON_TIME_THRES): {
		dhd_qmon_thres(dhd_pub, TRUE, int_val);
		break;
	}

	case IOV_GVAL(IOV_QMON_TIME_PERCENT): {
		int_val = dhd_qmon_getpercent(dhd_pub);
		bcopy(&int_val, arg, val_size);
		break;
	}
#endif /* QMONITOR */

	case IOV_GVAL(IOV_PROPTXSTATUS_MODULE_IGNORE):
		bcmerror = dhd_wlfc_get_module_ignore(dhd_pub, &int_val);
		if (bcmerror != BCME_OK)
			goto exit;
		bcopy(&int_val, arg, val_size);
		break;

	case IOV_SVAL(IOV_PROPTXSTATUS_MODULE_IGNORE):
		dhd_wlfc_set_module_ignore(dhd_pub, int_val);
		break;

	case IOV_GVAL(IOV_PROPTXSTATUS_CREDIT_IGNORE):
		bcmerror = dhd_wlfc_get_credit_ignore(dhd_pub, &int_val);
		if (bcmerror != BCME_OK)
			goto exit;
		bcopy(&int_val, arg, val_size);
		break;

	case IOV_SVAL(IOV_PROPTXSTATUS_CREDIT_IGNORE):
		dhd_wlfc_set_credit_ignore(dhd_pub, int_val);
		break;

	case IOV_GVAL(IOV_PROPTXSTATUS_TXSTATUS_IGNORE):
		bcmerror = dhd_wlfc_get_txstatus_ignore(dhd_pub, &int_val);
		if (bcmerror != BCME_OK)
			goto exit;
		bcopy(&int_val, arg, val_size);
		break;

	case IOV_SVAL(IOV_PROPTXSTATUS_TXSTATUS_IGNORE):
		dhd_wlfc_set_txstatus_ignore(dhd_pub, int_val);
		break;

	case IOV_GVAL(IOV_PROPTXSTATUS_RXPKT_CHK):
		bcmerror = dhd_wlfc_get_rxpkt_chk(dhd_pub, &int_val);
		if (bcmerror != BCME_OK)
			goto exit;
		bcopy(&int_val, arg, val_size);
		break;

	case IOV_SVAL(IOV_PROPTXSTATUS_RXPKT_CHK):
		dhd_wlfc_set_rxpkt_chk(dhd_pub, int_val);
		break;

#endif /* PROP_TXSTATUS */

	case IOV_GVAL(IOV_BUS_TYPE):
		/* The dhd application queries the driver to check if its usb or sdio.  */
#ifdef BCMDHDUSB
		int_val = BUS_TYPE_USB;
#endif
#ifdef PCIE_FULL_DONGLE
		int_val = BUS_TYPE_PCIE;
#endif
		bcopy(&int_val, arg, val_size);
		break;

#ifdef WLMEDIA_HTSF
	case IOV_GVAL(IOV_WLPKTDLYSTAT_SZ):
		int_val = dhd_pub->htsfdlystat_sz;
		bcopy(&int_val, arg, val_size);
		break;

	case IOV_SVAL(IOV_WLPKTDLYSTAT_SZ):
		dhd_pub->htsfdlystat_sz = int_val & 0xff;
		printf("Setting tsfdlystat_sz:%d\n", dhd_pub->htsfdlystat_sz);
		break;
#endif
	case IOV_SVAL(IOV_CHANGEMTU):
		int_val &= 0xffff;
		bcmerror = dhd_change_mtu(dhd_pub, int_val, 0);
		break;

	case IOV_GVAL(IOV_HOSTREORDER_FLOWS):
	{
		uint i = 0;
		uint8 *ptr = (uint8 *)arg;
		uint8 count = 0;

		ptr++;
		for (i = 0; i < WLHOST_REORDERDATA_MAXFLOWS; i++) {
			if (dhd_pub->reorder_bufs[i] != NULL) {
				*ptr = dhd_pub->reorder_bufs[i]->flow_id;
				ptr++;
				count++;
			}
		}
		ptr = (uint8 *)arg;
		*ptr = count;
		break;
	}
#ifdef DHDTCPACK_SUPPRESS
	case IOV_GVAL(IOV_TCPACK_SUPPRESS): {
		int_val = (uint32)dhd_pub->tcpack_sup_mode;
		bcopy(&int_val, arg, val_size);
		break;
	}
	case IOV_SVAL(IOV_TCPACK_SUPPRESS): {
		bcmerror = dhd_tcpack_suppress_set(dhd_pub, (uint8)int_val);
		break;
	}
#endif /* DHDTCPACK_SUPPRESS */
#ifdef DHD_WMF
	case IOV_GVAL(IOV_WMF_BSS_ENAB): {
		uint32	bssidx;
		dhd_wmf_t *wmf;
		const char *val;

		if (dhd_iovar_parse_bssidx(dhd_pub, name, &bssidx, &val) != BCME_OK) {
			DHD_ERROR(("%s: wmf_bss_enable: bad parameter\n", __FUNCTION__));
			bcmerror = BCME_BADARG;
			break;
		}

		wmf = dhd_wmf_conf(dhd_pub, bssidx);
		int_val = wmf->wmf_enable ? 1 :0;
		bcopy(&int_val, arg, val_size);
		break;
	}
	case IOV_SVAL(IOV_WMF_BSS_ENAB): {
		/* Enable/Disable WMF */
		uint32	bssidx;
		dhd_wmf_t *wmf;
		const char *val;

		if (dhd_iovar_parse_bssidx(dhd_pub, name, &bssidx, &val) != BCME_OK) {
			DHD_ERROR(("%s: wmf_bss_enable: bad parameter\n", __FUNCTION__));
			bcmerror = BCME_BADARG;
			break;
		}

		ASSERT(val);
		bcopy(val, &int_val, sizeof(uint32));
		wmf = dhd_wmf_conf(dhd_pub, bssidx);
		wmf->wmf_bss_enab_val = int_val;
		bcmerror = dhd_wmf_bss_enable(dhd_pub, bssidx);
		break;
	}
	case IOV_GVAL(IOV_WMF_UCAST_IGMP):
		int_val = dhd_pub->wmf_ucast_igmp ? 1 : 0;
		bcopy(&int_val, arg, val_size);
		break;
	case IOV_SVAL(IOV_WMF_UCAST_IGMP):
		if (dhd_pub->wmf_ucast_igmp == int_val)
			break;

		if (int_val >= OFF && int_val <= ON)
			dhd_pub->wmf_ucast_igmp = int_val;
		else
			bcmerror = BCME_RANGE;
		break;
	case IOV_GVAL(IOV_WMF_MCAST_DATA_SENDUP): {
		uint32	bssidx;
		const char *val;

		if (dhd_iovar_parse_bssidx(dhd_pub, (char *)name, &bssidx, &val) != BCME_OK) {
			DHD_ERROR(("%s: wmf_mcast_data_sendup: bad parameter\n", __FUNCTION__));
			bcmerror = BCME_BADARG;
			break;
		}

		int_val = dhd_wmf_mcast_data_sendup(dhd_pub, bssidx, FALSE, FALSE);
		bcopy(&int_val, arg, val_size);
		break;
	}
	case IOV_SVAL(IOV_WMF_MCAST_DATA_SENDUP): {
		uint32	bssidx;
		const char *val;

		if (dhd_iovar_parse_bssidx(dhd_pub, (char *)name, &bssidx, &val) != BCME_OK) {
			DHD_ERROR(("%s: wmf_mcast_data_sendup: bad parameter\n", __FUNCTION__));
			bcmerror = BCME_BADARG;
			break;
		}

		ASSERT(val);
		bcopy(val, &int_val, sizeof(uint32));
		bcmerror = dhd_wmf_mcast_data_sendup(dhd_pub, bssidx, TRUE, int_val);
		break;
	}
#ifdef DHD_IGMP_UCQUERY
	case IOV_GVAL(IOV_WMF_UCAST_IGMP_QUERY):
		int_val = dhd_pub->wmf_ucast_igmp_query ? 1 : 0;
		bcopy(&int_val, arg, val_size);
		break;
	case IOV_SVAL(IOV_WMF_UCAST_IGMP_QUERY):
		if (dhd_pub->wmf_ucast_igmp_query == int_val)
			break;

		if (int_val >= OFF && int_val <= ON)
			dhd_pub->wmf_ucast_igmp_query = int_val;
		else
			bcmerror = BCME_RANGE;
		break;
#endif /* DHD_IGMP_UCQUERY */
#ifdef DHD_UCAST_UPNP
	case IOV_GVAL(IOV_WMF_UCAST_UPNP):
		int_val = dhd_pub->wmf_ucast_upnp ? 1 : 0;
		bcopy(&int_val, arg, val_size);
		break;
	case IOV_SVAL(IOV_WMF_UCAST_UPNP):
		if (dhd_pub->wmf_ucast_upnp == int_val)
			break;

		if (int_val >= OFF && int_val <= ON)
			dhd_pub->wmf_ucast_upnp = int_val;
		else
			bcmerror = BCME_RANGE;
		break;
#endif /* DHD_UCAST_UPNP */

	case IOV_GVAL(IOV_WMF_PSTA_DISABLE): {
		uint32	bssidx;
		const char *val;

		if (dhd_iovar_parse_bssidx(dhd_pub, name, &bssidx, &val) != BCME_OK) {
			DHD_ERROR(("%s: wmf_psta_disable: bad parameter\n", __FUNCTION__));
			bcmerror = BCME_BADARG;
			break;
		}

		int_val = dhd_get_wmf_psta_disable(dhd_pub, bssidx);
		bcopy(&int_val, arg, val_size);
		break;
	}

	case IOV_SVAL(IOV_WMF_PSTA_DISABLE): {
		uint32	bssidx;
		const char *val;

		if (dhd_iovar_parse_bssidx(dhd_pub, name, &bssidx, &val) != BCME_OK) {
			DHD_ERROR(("%s: wmf_psta_disable: bad parameter\n", __FUNCTION__));
			bcmerror = BCME_BADARG;
			break;
		}

		ASSERT(val);
		bcopy(val, &int_val, sizeof(uint32));
		bcmerror = dhd_set_wmf_psta_disable(dhd_pub, bssidx, int_val);
		break;
	}
#endif /* DHD_WMF */

#if defined(BCM_ROUTER_DHD) || defined(TRAFFIC_MGMT_DWM)
	case IOV_SVAL(IOV_TRAFFIC_MGMT_DWM): {
			trf_mgmt_filter_list_t   *trf_mgmt_filter_list =
				(trf_mgmt_filter_list_t *)(arg);
			bcmerror = traffic_mgmt_add_dwm_filter(dhd_pub, trf_mgmt_filter_list, len);
		}
		break;
#endif /* BCM_ROUTER_DHD || TRAFFIC_MGMT_DWM */

#ifdef DHD_L2_FILTER
	case IOV_GVAL(IOV_DHCP_UNICAST): {
		uint32 bssidx;
		const char *val;
		if (dhd_iovar_parse_bssidx(dhd_pub, name, &bssidx, &val) != BCME_OK) {
			DHD_ERROR(("%s: IOV_DHCP_UNICAST: bad parameterand name = %s\n",
				__FUNCTION__, name));
			bcmerror = BCME_BADARG;
			break;
		}
		int_val = dhd_get_dhcp_unicast_status(dhd_pub, bssidx);
		memcpy(arg, &int_val, val_size);
		break;
	}
	case IOV_SVAL(IOV_DHCP_UNICAST): {
		uint32	bssidx;
		const char *val;
		if (dhd_iovar_parse_bssidx(dhd_pub, name, &bssidx, &val) != BCME_OK) {
			DHD_ERROR(("%s: IOV_DHCP_UNICAST: bad parameterand name = %s\n",
				__FUNCTION__, name));
			bcmerror = BCME_BADARG;
			break;
		}
		memcpy(&int_val, val, sizeof(int_val));
		bcmerror = dhd_set_dhcp_unicast_status(dhd_pub, bssidx, int_val ? 1 : 0);
		break;
	}
	case IOV_GVAL(IOV_BLOCK_PING): {
		uint32 bssidx;
		const char *val;

		if (dhd_iovar_parse_bssidx(dhd_pub, name, &bssidx, &val) != BCME_OK) {
			DHD_ERROR(("%s: IOV_BLOCK_PING: bad parameter\n", __FUNCTION__));
			bcmerror = BCME_BADARG;
			break;
		}
		int_val = dhd_get_block_ping_status(dhd_pub, bssidx);
		memcpy(arg, &int_val, val_size);
		break;
	}
	case IOV_SVAL(IOV_BLOCK_PING): {
		uint32	bssidx;
		const char *val;

		if (dhd_iovar_parse_bssidx(dhd_pub, name, &bssidx, &val) != BCME_OK) {
			DHD_ERROR(("%s: IOV_BLOCK_PING: bad parameter\n", __FUNCTION__));
			bcmerror = BCME_BADARG;
			break;
		}
		memcpy(&int_val, val, sizeof(int_val));
		bcmerror = dhd_set_block_ping_status(dhd_pub, bssidx, int_val ? 1 : 0);
		break;
	}
	case IOV_GVAL(IOV_PROXY_ARP): {
		uint32	bssidx;
		const char *val;

		if (dhd_iovar_parse_bssidx(dhd_pub, name, &bssidx, &val) != BCME_OK) {
			DHD_ERROR(("%s: IOV_PROXY_ARP: bad parameter\n", __FUNCTION__));
			bcmerror = BCME_BADARG;
			break;
		}
		int_val = dhd_get_parp_status(dhd_pub, bssidx);
		bcopy(&int_val, arg, val_size);
		break;
	}
	case IOV_SVAL(IOV_PROXY_ARP): {
		uint32	bssidx;
		const char *val;

		if (dhd_iovar_parse_bssidx(dhd_pub, name, &bssidx, &val) != BCME_OK) {
			DHD_ERROR(("%s: IOV_PROXY_ARP: bad parameter\n", __FUNCTION__));
			bcmerror = BCME_BADARG;
			break;
		}
		bcopy(val, &int_val, sizeof(int_val));

		/* Issue a iovar request to WL to update the proxy arp capability bit
		 * in the Extended Capability IE of beacons/probe responses.
		 */
		bcmerror = dhd_iovar(dhd_pub, bssidx, "proxy_arp_advertise", (char *)val,
			sizeof(int_val), NULL, 0, TRUE);
		if (bcmerror == BCME_OK) {
			dhd_set_parp_status(dhd_pub, bssidx, int_val ? 1 : 0);
		}
		break;
	}
	case IOV_GVAL(IOV_GRAT_ARP): {
		uint32 bssidx;
		const char *val;

		if (dhd_iovar_parse_bssidx(dhd_pub, name, &bssidx, &val) != BCME_OK) {
			DHD_ERROR(("%s: IOV_GRAT_ARP: bad parameter\n", __FUNCTION__));
			bcmerror = BCME_BADARG;
			break;
		}
		int_val = dhd_get_grat_arp_status(dhd_pub, bssidx);
		memcpy(arg, &int_val, val_size);
		break;
	}
	case IOV_SVAL(IOV_GRAT_ARP): {
		uint32	bssidx;
		const char *val;

		if (dhd_iovar_parse_bssidx(dhd_pub, name, &bssidx, &val) != BCME_OK) {
			DHD_ERROR(("%s: IOV_GRAT_ARP: bad parameter\n", __FUNCTION__));
			bcmerror = BCME_BADARG;
			break;
		}
		memcpy(&int_val, val, sizeof(int_val));
		bcmerror = dhd_set_grat_arp_status(dhd_pub, bssidx, int_val ? 1 : 0);
		break;
	}
	case IOV_GVAL(IOV_BLOCK_TDLS): {
		uint32 bssidx;
		const char *val;

		if (dhd_iovar_parse_bssidx(dhd_pub, name, &bssidx, &val) != BCME_OK) {
			DHD_ERROR(("%s: IOV_BLOCK_TDLS: bad parameter\n", __FUNCTION__));
			bcmerror = BCME_BADARG;
			break;
		}
		int_val = dhd_get_block_tdls_status(dhd_pub, bssidx);
		memcpy(arg, &int_val, val_size);
		break;
	}
	case IOV_SVAL(IOV_BLOCK_TDLS): {
		uint32	bssidx;
		const char *val;

		if (dhd_iovar_parse_bssidx(dhd_pub, name, &bssidx, &val) != BCME_OK) {
			DHD_ERROR(("%s: IOV_BLOCK_TDLS: bad parameter\n", __FUNCTION__));
			bcmerror = BCME_BADARG;
			break;
		}
		memcpy(&int_val, val, sizeof(int_val));
		bcmerror = dhd_set_block_tdls_status(dhd_pub, bssidx, int_val ? 1 : 0);
		break;
	}
#endif /* DHD_L2_FILTER */
	case IOV_SVAL(IOV_DHD_IE): {
		uint32	bssidx;
		const char *val;
#if (defined(BCM_ROUTER_DHD) && defined(QOS_MAP_SET))
		uint8 ie_type;
		bcm_tlv_t *qos_map_ie = NULL;
		ie_setbuf_t *ie_getbufp = (ie_setbuf_t *)(arg+4);
		ie_type = ie_getbufp->ie_buffer.ie_list[0].ie_data.id;
#endif /* BCM_ROUTER_DHD && QOS_MAP_SET */

		if (dhd_iovar_parse_bssidx(dhd_pub, name, &bssidx, &val) != BCME_OK) {
			DHD_ERROR(("%s: dhd ie: bad parameter\n", __FUNCTION__));
			bcmerror = BCME_BADARG;
			break;
		}

#if (defined(BCM_ROUTER_DHD) && defined(QOS_MAP_SET))
		qos_map_ie = (bcm_tlv_t *)(&(ie_getbufp->ie_buffer.ie_list[0].ie_data));
		if (qos_map_ie != NULL && (ie_type == DOT11_MNG_QOS_MAP_ID)) {
				bcmerror = dhd_set_qosmap_up_table(dhd_pub, bssidx, qos_map_ie);
		}
#endif /* BCM_ROUTER_DHD && QOS_MAP_SET */
		break;
	}
	case IOV_GVAL(IOV_AP_ISOLATE): {
		uint32	bssidx;
		const char *val;

		if (dhd_iovar_parse_bssidx(dhd_pub, name, &bssidx, &val) != BCME_OK) {
			DHD_ERROR(("%s: ap isoalate: bad parameter\n", __FUNCTION__));
			bcmerror = BCME_BADARG;
			break;
		}

		int_val = dhd_get_ap_isolate(dhd_pub, bssidx);
		bcopy(&int_val, arg, val_size);
		break;
	}
	case IOV_SVAL(IOV_AP_ISOLATE): {
		uint32	bssidx;
		const char *val;

		if (dhd_iovar_parse_bssidx(dhd_pub, name, &bssidx, &val) != BCME_OK) {
			DHD_ERROR(("%s: ap isolate: bad parameter\n", __FUNCTION__));
			bcmerror = BCME_BADARG;
			break;
		}

		ASSERT(val);
		bcopy(val, &int_val, sizeof(uint32));
		dhd_set_ap_isolate(dhd_pub, bssidx, int_val);
		break;
	}
#ifdef DHD_PSTA
	case IOV_GVAL(IOV_PSTA): {
		int_val = dhd_get_psta_mode(dhd_pub);
		bcopy(&int_val, arg, val_size);
		break;
		}
	case IOV_SVAL(IOV_PSTA): {
		if (int_val >= DHD_MODE_PSTA_DISABLED && int_val <= DHD_MODE_PSR) {
			dhd_set_psta_mode(dhd_pub, int_val);
		} else {
			bcmerror = BCME_RANGE;
		}
		break;
		}
#endif /* DHD_PSTA */

#ifdef DHD_MCAST_REGEN
	case IOV_GVAL(IOV_MCAST_REGEN_BSS_ENABLE): {
		uint32	bssidx;
		const char *val;

		if (dhd_iovar_parse_bssidx(dhd_pub, name, &bssidx, &val) != BCME_OK) {
			DHD_ERROR(("%s: mcast_regen_bss_enable: bad parameter\n", __FUNCTION__));
			bcmerror = BCME_BADARG;
			break;
		}

		int_val = dhd_get_mcast_regen_bss_enable(dhd_pub, bssidx);
		bcopy(&int_val, arg, val_size);
		break;
	}

	case IOV_SVAL(IOV_MCAST_REGEN_BSS_ENABLE): {
		uint32	bssidx;
		const char *val;

		if (dhd_iovar_parse_bssidx(dhd_pub, name, &bssidx, &val) != BCME_OK) {
			DHD_ERROR(("%s: mcast_regen_bss_enable: bad parameter\n", __FUNCTION__));
			bcmerror = BCME_BADARG;
			break;
		}

		ASSERT(val);
		bcopy(val, &int_val, sizeof(uint32));
		dhd_set_mcast_regen_bss_enable(dhd_pub, bssidx, int_val);
		break;
	}
#endif /* DHD_MCAST_REGEN */

#ifdef DHD_WET
	case IOV_GVAL(IOV_WET):
		int_val = dhd_get_wet_mode(dhd_pub);
		bcopy(&int_val, arg, val_size);
		break;

	case IOV_SVAL(IOV_WET):
		if (int_val == 0 || int_val == 1) {
			dhd_set_wet_mode(dhd_pub, int_val);
			/* Delete the WET DB when disabled */
			if (!int_val) {
			    dhd_wet_sta_delete_list(dhd_pub);
			}
		} else {
			bcmerror = BCME_RANGE;
		}
		break;
	case IOV_SVAL(IOV_WET_HOST_IPV4):
		bcmerror = dhd_set_wet_host_ipv4(dhd_pub, params, IPV4_ADDR_LEN);
		break;
	case IOV_SVAL(IOV_WET_HOST_MAC):
		bcmerror = dhd_set_wet_host_mac(dhd_pub, params, ETHER_ADDR_LEN);
		break;
#endif /* DHD_WET */

	case IOV_GVAL(IOV_CFG80211_OPMODE): {
		int_val = (int32)dhd_pub->wlcore->op_mode;
		bcopy(&int_val, arg, sizeof(int_val));
		break;
		}
	case IOV_SVAL(IOV_CFG80211_OPMODE): {
		if (int_val <= 0)
			bcmerror = BCME_BADARG;
		else
			dhd_pub->wlcore->op_mode = int_val;
		break;
	}

	case IOV_GVAL(IOV_ASSERT_TYPE):
		int_val = g_assert_type;
		bcopy(&int_val, arg, val_size);
		break;

	case IOV_SVAL(IOV_ASSERT_TYPE):
		g_assert_type = (uint32)int_val;
		break;

#if !defined(BCM_ROUTER_DHD)
	case IOV_GVAL(IOV_LMTEST): {
		*(uint32 *)arg = (uint32)lmtest;
		break;
	}

	case IOV_SVAL(IOV_LMTEST): {
		uint32 val = *(uint32 *)arg;
		if (val > 50)
			bcmerror = BCME_BADARG;
		else {
			lmtest = (uint)val;
			DHD_ERROR(("%s: lmtest %s\n",
				__FUNCTION__, (lmtest == FALSE)? "OFF" : "ON"));
		}
		break;
	}
#endif /* !BCM_ROUTER_DHD */
	case IOV_GVAL(IOV_MACDBG_PD11REGS):
		bcmerror = dhd_macdbg_pd11regs(dhd_pub, params, plen, arg, len);
		break;
	case IOV_GVAL(IOV_MACDBG_PSVMPMEMS):
		bcmerror = dhd_macdbg_psvmpmems(dhd_pub, params, plen, arg, len);
		break;
#ifdef DHD_LBR_AGGR_BCM_ROUTER

	case IOV_SVAL(IOV_LBR_AGGR_LEN):
		dhd_lbr_aggr_len(dhd_pub, TRUE, int_val);
		break;
	case IOV_GVAL(IOV_LBR_AGGR_LEN):
		int_val = dhd_lbr_aggr_len(dhd_pub, FALSE, int_val);
		memcpy(arg, &int_val, val_size);
		break;

	case IOV_SVAL(IOV_LBR_AGGR_EN_MASK):
		dhd_lbr_aggr_en_mask(dhd_pub, TRUE, int_val);
		break;
	case IOV_GVAL(IOV_LBR_AGGR_EN_MASK):
		int_val = dhd_lbr_aggr_en_mask(dhd_pub, FALSE, int_val);
		memcpy(arg, &int_val, val_size);
		break;

	case IOV_SVAL(IOV_LBR_AGGR_RELEASE_TIMEOUT):
		dhd_lbr_aggr_release_timeout(dhd_pub, TRUE, int_val);
		break;
	case IOV_GVAL(IOV_LBR_AGGR_RELEASE_TIMEOUT):
		int_val = dhd_lbr_aggr_release_timeout(dhd_pub, FALSE, int_val);
		memcpy(arg, &int_val, val_size);
		break;

#endif /* DHD_LBR_AGGR_BCM_ROUTER */

#if defined(BCM_DHD_RUNNER)
	case IOV_SVAL(IOV_RNR_PROFILE):
		bcmerror = dhd_runner_do_iovar(dhd_pub->runner_hlp,
			DHD_RNR_IOVAR_PROFILE, TRUE, arg, len);
		break;
	case IOV_GVAL(IOV_RNR_PROFILE):
		bcmerror = dhd_runner_do_iovar(dhd_pub->runner_hlp,
			DHD_RNR_IOVAR_PROFILE, FALSE, arg, len);
		break;
	case IOV_SVAL(IOV_RNR_POLICY):
		bcmerror = dhd_runner_do_iovar(dhd_pub->runner_hlp,
			DHD_RNR_IOVAR_POLICY, TRUE, arg, len);
		break;
	case IOV_GVAL(IOV_RNR_POLICY):
		bcmerror = dhd_runner_do_iovar(dhd_pub->runner_hlp,
			DHD_RNR_IOVAR_POLICY, FALSE, arg, len);
		break;
	case IOV_SVAL(IOV_RNR_RXOFFL):
		bcmerror = dhd_runner_do_iovar(dhd_pub->runner_hlp,
			DHD_RNR_IOVAR_RXOFFL, TRUE, arg, len);
		break;
	case IOV_GVAL(IOV_RNR_RXOFFL):
		bcmerror = dhd_runner_do_iovar(dhd_pub->runner_hlp,
			DHD_RNR_IOVAR_RXOFFL, FALSE, arg, len);
		break;
#endif /* BCM_DHD_RUNNER */
	case IOV_GVAL(IOV_MACDBG_DUMP):
		bcmerror = dhd_macdbg_dumpiov(dhd_pub, params, plen, arg, len);
		break;
#ifdef BCM_GMAC3
	case IOV_SVAL(IOV_1905_AL_UCAST): {
		uint32	bssidx;
		const char *val;
		uint8 ea[6] = {0};

		if (dhd_iovar_parse_bssidx(dhd_pub, (char *)name, &bssidx, &val) != BCME_OK) {
			DHD_ERROR(("%s: 1905_al_ucast: bad parameter\n", __FUNCTION__));
			bcmerror = BCME_BADARG;
			break;
		}

		bcopy(val, ea, ETHER_ADDR_LEN);
		bcmerror = dhd_set_1905_almac(dhd_pub, bssidx, ea, FALSE);
		break;
	}
	case IOV_GVAL(IOV_1905_AL_UCAST): {
		uint32	bssidx;
		const char *val;

		if (dhd_iovar_parse_bssidx(dhd_pub, (char *)name, &bssidx, &val) != BCME_OK) {
			DHD_ERROR(("%s: 1905_al_ucast: bad parameter\n", __FUNCTION__));
			bcmerror = BCME_BADARG;
			break;
		}

		bcmerror = dhd_get_1905_almac(dhd_pub, bssidx, arg, FALSE);
		break;
	}
	case IOV_SVAL(IOV_1905_AL_MCAST): {
		uint32	bssidx;
		const char *val;
		uint8 ea[6] = {0};

		if (dhd_iovar_parse_bssidx(dhd_pub, (char *)name, &bssidx, &val) != BCME_OK) {
			DHD_ERROR(("%s: 1905_al_mcast: bad parameter\n", __FUNCTION__));
			bcmerror = BCME_BADARG;
			break;
		}

		bcopy(val, ea, ETHER_ADDR_LEN);
		bcmerror = dhd_set_1905_almac(dhd_pub, bssidx, ea, TRUE);
		break;
	}
	case IOV_GVAL(IOV_1905_AL_MCAST): {
		uint32	bssidx;
		const char *val;

		if (dhd_iovar_parse_bssidx(dhd_pub, (char *)name, &bssidx, &val) != BCME_OK) {
			DHD_ERROR(("%s: 1905_al_mcast: bad parameter\n", __FUNCTION__));
			bcmerror = BCME_BADARG;
			break;
		}

		bcmerror = dhd_get_1905_almac(dhd_pub, bssidx, arg, TRUE);
		break;
	}
#endif /* BCM_GMAC3 */
	case IOV_GVAL(IOV_MACDBG_DUMP_LEVEL): {
		int_val = dhd_get_macdbg_dump_level(dhd_pub->info);
		memcpy(arg, &int_val, val_size);
		break;
	}
	case IOV_SVAL(IOV_MACDBG_DUMP_LEVEL): {
		dhd_set_macdbg_dump_level(dhd_pub->info, (uint32)int_val);
		break;
	}
#if defined(BCM_CPE_PKTC)
	case IOV_SVAL(IOV_PKTC):
		dhd_pub->pktc = int_val;
		break;
	case IOV_GVAL(IOV_PKTC):
		int_val = (int32)dhd_pub->pktc;
		bcopy(&int_val, arg, val_size);
		break;
	case IOV_SVAL(IOV_PKTCBND):
		dhd_pub->pktcbnd = int_val;
		break;
	case IOV_GVAL(IOV_PKTCBND):
		int_val = (int32)dhd_pub->pktcbnd;
		bcopy(&int_val, arg, val_size);
		break;
#endif /* BCM_CPE_PKTC */

	case IOV_GVAL(IOV_HMEMBYTES):
	{
		uint val32, *ptr32, size;
		uint64 val64;
		uint8 *data;
		void *haddr;

		ASSERT(plen >= 3*sizeof(uint));

		ptr32 = (uint *)params;

		// haddr
		bcopy(ptr32, &val32, sizeof(uint));
		val64 = val32;
		val64 = val64 << 32;
		bcopy(ptr32+1, &val32, sizeof(uint));
		val64 |= val32;
		haddr = (void *)((uintptr)val64);

		// size
		bcopy(ptr32+2, &val32, sizeof(uint));
		size = val32;

		/* Generate the actual data pointer */
		data = (uint8*)arg;

		/* Call to do the transfer */
		bcmerror = dhd_hmembytes(dhd_pub, haddr, data, size);
		break;
	}

#ifdef WL_CFG80211
	case IOV_SVAL(IOV_EXTENDED_OPS_SUPPORT):
	{
		struct bcm_cfg80211 *cfg = NULL;
		struct net_device *dev;
		dev = dhd_linux_get_primary_netdev(dhd_pub);
		cfg = wl_get_cfg(dev);
		dhd_pub->extended_ops_support = int_val;
		wl_cfg80211_set_extended_ops_support(cfg, int_val);
		break;
	}
	case IOV_GVAL(IOV_EXTENDED_OPS_SUPPORT):
		int_val = (int32)dhd_pub->extended_ops_support;
		bcopy(&int_val, arg, val_size);
		break;
#endif /* WL_CFG80211 */
#ifdef DHD_IFE
	case IOV_GVAL(IOV_IFE_TIMEOUT):
		int_val = (int32)dhd_ife_get_timer_interval(dhd_pub);
		bcopy(&int_val, arg, val_size);
		break;
	case IOV_SVAL(IOV_IFE_TIMEOUT):
		dhd_ife_set_timer_interval(dhd_pub, int_val);
		break;
	case IOV_GVAL(IOV_IFE_BUDGET):
		int_val = (int32)dhd_ife_get_budget(dhd_pub);
		bcopy(&int_val, arg, val_size);
		break;
	case IOV_SVAL(IOV_IFE_BUDGET):
		dhd_ife_set_budget(dhd_pub, int_val);
		break;
#endif /* DHD_IFE */
	default:
		bcmerror = BCME_UNSUPPORTED;
		break;
	}

exit:
	DHD_TRACE(("%s: actionid %d, bcmerror %d\n", __FUNCTION__, actionid, bcmerror));
	return bcmerror;
}

#ifdef BCMDONGLEHOST
/* Store the status of a connection attempt for later retrieval by an iovar */
void
dhd_store_conn_status(uint32 event, uint32 status, uint32 reason)
{
	/* Do not overwrite a WLC_E_PRUNE with a WLC_E_SET_SSID
	 * because an encryption/rsn mismatch results in both events, and
	 * the important information is in the WLC_E_PRUNE.
	 */
	if (!(event == WLC_E_SET_SSID && status == WLC_E_STATUS_FAIL &&
	      dhd_conn_event == WLC_E_PRUNE)) {
		dhd_conn_event = event;
		dhd_conn_status = status;
		dhd_conn_reason = reason;
	}
}
#else
#error "BCMDONGLEHOST not defined"
#endif /* BCMDONGLEHOST */

bool
dhd_prec_enq(dhd_pub_t *dhdp, struct pktq *q, void *pkt, int prec)
{
	void *p;
	int eprec = -1;		/* precedence to evict from */
	bool discard_oldest;

	/* Fast case, precedence queue is not full and we are also not
	 * exceeding total queue length
	 */
	if (!pktqprec_full(q, prec) && !pktq_full(q)) {
		pktq_penq(q, prec, pkt);
		return TRUE;
	}

	/* Determine precedence from which to evict packet, if any */
	if (pktqprec_full(q, prec))
		eprec = prec;
	else if (pktq_full(q)) {
		p = pktq_peek_tail(q, &eprec);
		ASSERT(p);
		if (eprec > prec || eprec < 0)
			return FALSE;
	}

	/* Evict if needed */
	if (eprec >= 0) {
		/* Detect queueing to unconfigured precedence */
		ASSERT(!pktqprec_empty(q, eprec));
		discard_oldest = AC_BITMAP_TST(dhdp->wme_dp, eprec);
		if (eprec == prec && !discard_oldest)
			return FALSE;		/* refuse newer (incoming) packet */
		/* Evict packet according to discard policy */
		p = discard_oldest ? pktq_pdeq(q, eprec) : pktq_pdeq_tail(q, eprec);
		ASSERT(p);
#ifdef DHDTCPACK_SUPPRESS
		if (dhd_tcpack_check_xmit(dhdp, p) == BCME_ERROR) {
			DHD_ERROR(("%s %d: tcpack_suppress ERROR!!! Stop using it\n",
				__FUNCTION__, __LINE__));
			dhd_tcpack_suppress_set(dhdp, TCPACK_SUP_OFF);
		}
#endif /* DHDTCPACK_SUPPRESS */
		PKTFREE(dhdp->osh, p, TRUE);
	}

	/* Enqueue */
	p = pktq_penq(q, prec, pkt);
	ASSERT(p);

	return TRUE;
}

/*
 * Functions to drop proper pkts from queue:
 *	If one pkt in queue is non-fragmented, drop first non-fragmented pkt only
 *	If all pkts in queue are all fragmented, find and drop one whole set fragmented pkts
 *	If can't find pkts matching upper 2 cases, drop first pkt anyway
 */
bool
dhd_prec_drop_pkts(dhd_pub_t *dhdp, struct pktq *pq, int prec, f_droppkt_t fn)
{
	struct pktq_prec *q = NULL;
	void *p, *prev = NULL, *next = NULL, *first = NULL, *last = NULL, *prev_first = NULL;
	pkt_frag_t frag_info;

	ASSERT(dhdp && pq);
	ASSERT(prec >= 0 && prec < pq->num_prec);

	q = &pq->q[prec];
	p = q->head;

	if (p == NULL)
		return FALSE;

	while (p) {
		frag_info = pkt_frag_info(dhdp->osh, p);
		if (frag_info == DHD_PKT_FRAG_NONE) {
			break;
		} else if (frag_info == DHD_PKT_FRAG_FIRST) {
			if (first) {
				/* No last frag pkt, use prev as last */
				last = prev;
				break;
			} else {
				first = p;
				prev_first = prev;
			}
		} else if (frag_info == DHD_PKT_FRAG_LAST) {
			if (first) {
				last = p;
				break;
			}
		}

		prev = p;
		p = PKTLINK(p);
	}

	if ((p == NULL) || ((frag_info != DHD_PKT_FRAG_NONE) && !(first && last))) {
		/* Not found matching pkts, use oldest */
		prev = NULL;
		p = q->head;
		frag_info = 0;
	}

	if (frag_info == DHD_PKT_FRAG_NONE) {
		first = last = p;
		prev_first = prev;
	}

	p = first;
	while (p) {
		next = PKTLINK(p);
		q->n_pkts--;
		pq->n_pkts_tot--;

		PKTSETLINK(p, NULL);

		if (fn)
			fn(dhdp, prec, p, TRUE);

		if (p == last)
			break;

		p = next;
	}

	if (prev_first == NULL) {
		if ((q->head = next) == NULL)
			q->tail = NULL;
	} else {
		PKTSETLINK(prev_first, next);
		if (!next)
			q->tail = prev_first;
	}

	return TRUE;
}

static int
dhd_iovar_op(dhd_pub_t *dhd_pub, const char *name,
	void *params, int plen, void *arg, int len, bool set)
{
	int bcmerror = 0;
	int val_size;
	const bcm_iovar_t *vi = NULL;
	uint32 actionid;

	DHD_TRACE(("%s: Enter\n", __FUNCTION__));

	ASSERT(name);
	ASSERT(len >= 0);

	/* Get MUST have return space */
	ASSERT(set || (arg && len));

	/* Set does NOT take qualifiers */
	ASSERT(!set || (!params && !plen));

	if ((vi = bcm_iovar_lookup(dhd_iovars, name)) == NULL) {
		bcmerror = BCME_UNSUPPORTED;
		goto exit;
	}

	DHD_CTL(("%s: %s %s, len %d plen %d\n", __FUNCTION__,
		name, (set ? "set" : "get"), len, plen));

	/* set up 'params' pointer in case this is a set command so that
	 * the convenience int and bool code can be common to set and get
	 */
	if (params == NULL) {
		params = arg;
		plen = len;
	}

	if (vi->type == IOVT_VOID)
		val_size = 0;
	else if (vi->type == IOVT_BUFFER)
		val_size = len;
	else
		/* all other types are integer sized */
		val_size = sizeof(int);

	actionid = set ? IOV_SVAL(vi->varid) : IOV_GVAL(vi->varid);

	bcmerror = dhd_doiovar(dhd_pub, vi, actionid, name, params, plen, arg, len, val_size);

exit:
	return bcmerror;
}

int
dhd_ioctl(dhd_pub_t * dhd_pub, dhd_ioctl_t *ioc, void *buf, uint buflen)
{
	int bcmerror = 0;

	DHD_TRACE(("%s: Enter\n", __FUNCTION__));

	if (!buf) {
		return BCME_BADARG;
	}

	switch (ioc->cmd) {
	case DHD_GET_MAGIC:
		if (buflen < sizeof(int))
			bcmerror = BCME_BUFTOOSHORT;
		else
			*(int*)buf = DHD_IOCTL_MAGIC;
		break;

	case DHD_GET_VERSION:
		if (buflen < sizeof(int))
			bcmerror = BCME_BUFTOOSHORT;
		else
			*(int*)buf = DHD_IOCTL_VERSION;
		break;

	case DHD_GET_VAR:
	case DHD_SET_VAR: {
		char *arg;
		uint arglen;

		/* scan past the name to any arguments */
		for (arg = buf, arglen = buflen; *arg && arglen; arg++, arglen--)
			;

		if (*arg) {
			bcmerror = BCME_BUFTOOSHORT;
			break;
		}

		/* account for the NUL terminator */
		arg++, arglen--;

		/* call with the appropriate arguments */
		if (ioc->cmd == DHD_GET_VAR)
			bcmerror = dhd_iovar_op(dhd_pub, buf, arg, arglen,
			buf, buflen, IOV_GET);
		else
			bcmerror = dhd_iovar_op(dhd_pub, buf, NULL, 0, arg, arglen, IOV_SET);
		if (bcmerror != BCME_UNSUPPORTED)
			break;

		/* not in generic table, try protocol module */
		if (ioc->cmd == DHD_GET_VAR)
			bcmerror = dhd_prot_iovar_op(dhd_pub, buf, arg,
				arglen, buf, buflen, IOV_GET);
		else
			bcmerror = dhd_prot_iovar_op(dhd_pub, buf,
				NULL, 0, arg, arglen, IOV_SET);
		if (bcmerror != BCME_UNSUPPORTED)
			break;

		/* if still not found, try bus module */
		if (ioc->cmd == DHD_GET_VAR) {
			bcmerror = BUS_IOVAR_OP(dhd_pub, buf,
				arg, arglen, buf, buflen, IOV_GET);

		} else {
			bcmerror = BUS_IOVAR_OP(dhd_pub, buf,
				NULL, 0, arg, arglen, IOV_SET);
		}

		break;
	}

	default:
		bcmerror = BCME_UNSUPPORTED;
	}

	return bcmerror;
}

#ifdef SHOW_EVENTS
#if defined(BCMDBG) || defined(DHD_DEBUG)

static const char *dfs_cacstate_str[WL_DFS_CACSTATES] = {
	"IDLE",
	"PRE-ISM Channel Availability Check(CAC)",
	"In-Service Monitoring(ISM)",
	"Channel Switching Announcement(CSA)",
	"POST-ISM Channel Availability Check",
	"PRE-ISM Ouf Of Channels(OOC)",
	"POST-ISM Out Of Channels(OOC)"
};

/* prints dfs status from one subset */
static int
wl_show_dfs_sub_status(wl_dfs_sub_status_t *sub)
{
	char chanspec_str[CHANSPEC_STR_LEN];

	(void) chanspec_str;
	(void) dfs_cacstate_str;

	sub->state = dtoh32(sub->state);
	sub->duration = dtoh32(sub->duration);
	sub->chanspec = dtoh16(sub->chanspec);
	sub->chanspec_last_cleared = dtoh16(sub->chanspec_last_cleared);
	sub->sub_type = dtoh16(sub->sub_type);

	// state
	if (sub->state >= WL_DFS_CACSTATES) {
		DHD_EVENT(("Unknown dfs state %d.\n", sub->state));
		return -1;
	}
	DHD_EVENT(("state: %s, time elapsed: %dms, ",
			dfs_cacstate_str[sub->state], sub->duration));
	// chanspec
	if (sub->chanspec) {
		DHD_EVENT(("chanspec: %s (0x%04X), ",
				wf_chspec_ntoa(sub->chanspec, chanspec_str),
				sub->chanspec));
	} else {
		DHD_EVENT(("chanspec: none, "));
	}
	// chanspec last cleared
	if (sub->chanspec_last_cleared) {
		DHD_EVENT(("chanspec last cleared: %s (0x%04X), ",
				wf_chspec_ntoa(sub->chanspec_last_cleared, chanspec_str),
				sub->chanspec_last_cleared));
	} else {
		DHD_EVENT(("chanspec last cleared: none, "));
	}
	// sub type
	DHD_EVENT(("sub type: 0x%02x\n", sub->sub_type));

	return 0;
}

/* prints dfs status of all subsets received */
static int
wl_show_dfs_status_all(wl_dfs_status_all_t *dfs_status_all)
{
	int count;
	int err;

	if (dfs_status_all == NULL) {
		return BCME_ERROR;
	}

	dfs_status_all->version = dtoh16(dfs_status_all->version);
	dfs_status_all->num_sub_status = dtoh16(dfs_status_all->num_sub_status);

	if (dfs_status_all->version != WL_DFS_STATUS_ALL_VERSION) {
		err = BCME_UNSUPPORTED;
		DHD_EVENT(("MACEVENT\terr=%d version=%d\n", err, dfs_status_all->version));
		return err;
	}

	DHD_EVENT(("MACEVENT\tversion: %d, num_sub_status: %d\n",
			dfs_status_all->version, dfs_status_all->num_sub_status));

	for (count = 0; count < dfs_status_all->num_sub_status; ++count) {
		DHD_EVENT(("MACEVENT\t@%d: ", count));
		if ((err = wl_show_dfs_sub_status(&dfs_status_all->dfs_sub_status[count]))
				!= BCME_OK) {
			return err;
		}
	}

	return BCME_OK;
}

static int
wl_show_cac_event(void *event_data, uint datalen)
{
	wlc_cac_event_t *ce = NULL;

	if (!event_data || datalen < sizeof(*ce)) {
		DHD_EVENT(("MACEVENT\tevent_data=%p datalen=%u (expected>=%zu)\n",
				event_data, datalen, sizeof(*ce)));
		return BCME_ERROR;
	}

	ce = (wlc_cac_event_t *) event_data;

	if (dtoh16(ce->version) != WLC_E_CAC_STATE_VERSION || dtoh16(ce->length) < sizeof(*ce) ||
			dtoh16(ce->type) != WLC_E_CAC_STATE_TYPE_DFS_STATUS_ALL) {
		DHD_EVENT(("MACEVENT\tunsupported cac event ver=%d or len=%d or type=%d\n",
				dtoh16(ce->version), dtoh16(ce->length), dtoh16(ce->type)));
		return BCME_UNSUPPORTED;
	}

	return wl_show_dfs_status_all(&ce->scan_status);
}
#endif /* defined(BCMDBG) || defined(DHD_DEBUG) */

static void
wl_show_host_event(dhd_pub_t *dhd_pub, wl_event_msg_t *event, void *event_data,
	void *raw_event_ptr, char *eventmask)
{
	uint i, status, reason;
	bool group = FALSE, flush_txq = FALSE, link = FALSE;
	bool host_data = FALSE; /* prints  event data after the case  when set */
	const char *auth_str;
	const char *event_name;
	uchar *buf;
	char err_msg[256], eabuf[ETHER_ADDR_STR_LEN];
	uint event_type, flags, auth_type, datalen;

	event_type = ntoh32(event->event_type);
	flags = ntoh16(event->flags);
	status = ntoh32(event->status);
	reason = ntoh32(event->reason);
	BCM_REFERENCE(reason);
	auth_type = ntoh32(event->auth_type);
	datalen = ntoh32(event->datalen);

	/* debug dump of event messages */
	snprintf(eabuf, sizeof(eabuf), "%02x:%02x:%02x:%02x:%02x:%02x",
	        (uchar)event->addr.octet[0]&0xff,
	        (uchar)event->addr.octet[1]&0xff,
	        (uchar)event->addr.octet[2]&0xff,
	        (uchar)event->addr.octet[3]&0xff,
	        (uchar)event->addr.octet[4]&0xff,
	        (uchar)event->addr.octet[5]&0xff);

	event_name = bcmevent_get_name(event_type);
	BCM_REFERENCE(event_name);

	if (flags & WLC_EVENT_MSG_LINK)
		link = TRUE;
	if (flags & WLC_EVENT_MSG_GROUP)
		group = TRUE;
	if (flags & WLC_EVENT_MSG_FLUSHTXQ)
		flush_txq = TRUE;

	switch (event_type) {
	case WLC_E_START:
	case WLC_E_DEAUTH:
	case WLC_E_DISASSOC:
		DHD_EVENT(("MACEVENT: %s, MAC %s\n", event_name, eabuf));
		break;

	case WLC_E_ASSOC_IND:
	case WLC_E_REASSOC_IND:

		DHD_EVENT(("MACEVENT: %s, MAC %s\n", event_name, eabuf));
		break;

	case WLC_E_ASSOC:
	case WLC_E_REASSOC:
		if (status == WLC_E_STATUS_SUCCESS) {
			DHD_EVENT(("MACEVENT: %s, MAC %s, SUCCESS\n", event_name, eabuf));
		} else if (status == WLC_E_STATUS_TIMEOUT) {
			DHD_EVENT(("MACEVENT: %s, MAC %s, TIMEOUT\n", event_name, eabuf));
		} else if (status == WLC_E_STATUS_FAIL) {
			DHD_EVENT(("MACEVENT: %s, MAC %s, FAILURE, reason %d\n",
			       event_name, eabuf, (int)reason));
		} else {
			DHD_EVENT(("MACEVENT: %s, MAC %s, unexpected status %d\n",
			       event_name, eabuf, (int)status));
		}
		break;

	case WLC_E_DEAUTH_IND:
	case WLC_E_DISASSOC_IND:
		DHD_EVENT(("MACEVENT: %s, MAC %s, reason %d\n", event_name, eabuf, (int)reason));
		break;

	case WLC_E_AUTH:
	case WLC_E_AUTH_IND:
		if (auth_type == DOT11_OPEN_SYSTEM)
			auth_str = "Open System";
		else if (auth_type == DOT11_SHARED_KEY)
			auth_str = "Shared Key";
		else {
			snprintf(err_msg, sizeof(err_msg), "AUTH unknown: %d", (int)auth_type);
			auth_str = err_msg;
		}
		if (event_type == WLC_E_AUTH_IND) {
			DHD_EVENT(("MACEVENT: %s, MAC %s, %s\n", event_name, eabuf, auth_str));
		} else if (status == WLC_E_STATUS_SUCCESS) {
			DHD_EVENT(("MACEVENT: %s, MAC %s, %s, SUCCESS\n",
				event_name, eabuf, auth_str));
		} else if (status == WLC_E_STATUS_TIMEOUT) {
			DHD_EVENT(("MACEVENT: %s, MAC %s, %s, TIMEOUT\n",
				event_name, eabuf, auth_str));
		} else if (status == WLC_E_STATUS_FAIL) {
			DHD_EVENT(("MACEVENT: %s, MAC %s, %s, FAILURE, reason %d\n",
			       event_name, eabuf, auth_str, (int)reason));
		}
		BCM_REFERENCE(auth_str);

		break;

	case WLC_E_JOIN:
	case WLC_E_ROAM:
	case WLC_E_SET_SSID:
		if (status == WLC_E_STATUS_SUCCESS) {
			DHD_EVENT(("MACEVENT: %s, MAC %s\n", event_name, eabuf));
		} else if (status == WLC_E_STATUS_FAIL) {
			DHD_EVENT(("MACEVENT: %s, failed\n", event_name));
		} else if (status == WLC_E_STATUS_NO_NETWORKS) {
			DHD_EVENT(("MACEVENT: %s, no networks found\n", event_name));
		} else {
			DHD_EVENT(("MACEVENT: %s, unexpected status %d\n",
				event_name, (int)status));
		}
		break;

	case WLC_E_BEACON_RX:
		if (status == WLC_E_STATUS_SUCCESS) {
			DHD_EVENT(("MACEVENT: %s, SUCCESS\n", event_name));
		} else if (status == WLC_E_STATUS_FAIL) {
			DHD_EVENT(("MACEVENT: %s, FAIL\n", event_name));
		} else {
			DHD_EVENT(("MACEVENT: %s, status %d\n", event_name, status));
		}
		break;

	case WLC_E_LINK:
		DHD_EVENT(("MACEVENT: %s %s\n", event_name, link?"UP":"DOWN"));
		BCM_REFERENCE(link);
		break;

	case WLC_E_MIC_ERROR:
		DHD_EVENT(("MACEVENT: %s, MAC %s, Group %d, Flush %d\n",
		       event_name, eabuf, group, flush_txq));
		BCM_REFERENCE(group);
		BCM_REFERENCE(flush_txq);
		break;

	case WLC_E_ICV_ERROR:
	case WLC_E_UNICAST_DECODE_ERROR:
	case WLC_E_MULTICAST_DECODE_ERROR:
		DHD_EVENT(("MACEVENT: %s, MAC %s\n",
		       event_name, eabuf));
		break;

	case WLC_E_TXFAIL:
		DHD_EVENT(("MACEVENT: %s, RA %s status %d\n", event_name, eabuf, status));
		break;

	case WLC_E_ASSOC_REQ_IE:
	case WLC_E_ASSOC_RESP_IE:
	case WLC_E_PMKID_CACHE:
	case WLC_E_SCAN_COMPLETE:
		DHD_EVENT(("MACEVENT: %s\n", event_name));
		break;

	case WLC_E_PFN_NET_FOUND:
	case WLC_E_PFN_NET_LOST:
	case WLC_E_PFN_SCAN_NONE:
	case WLC_E_PFN_SCAN_ALLGONE:
		DHD_EVENT(("PNOEVENT: %s\n", event_name));
		break;

	case WLC_E_PSK_SUP:
	case WLC_E_PRUNE:
		DHD_EVENT(("MACEVENT: %s, status %d, reason %d\n",
		           event_name, (int)status, (int)reason));
		break;

#ifdef WIFI_ACT_FRAME
	case WLC_E_ACTION_FRAME:
		DHD_TRACE(("MACEVENT: %s Bssid %s\n", event_name, eabuf));
		break;
#endif /* WIFI_ACT_FRAME */

#if defined(WLBTAMP) && defined(BCMDBG)
	case WLC_E_BTA_HCI_EVENT:
		DHD_EVENT(("MACEVENT: %s %d\n", event_name, ntoh32(*((int *)event_data))));
		if (DHD_BTA_ON())
			dhd_bta_hcidump_evt(NULL, event_data);
		break;
#endif   /* WLBTAMP && BCMDBG */

	case WLC_E_RSSI:
		DHD_EVENT(("MACEVENT: %s %d\n", event_name, ntoh32(*((int *)event_data))));
		break;

	case WLC_E_SERVICE_FOUND:
	case WLC_E_P2PO_ADD_DEVICE:
	case WLC_E_P2PO_DEL_DEVICE:
		DHD_EVENT(("MACEVENT: %s, MAC %s\n", event_name, eabuf));
		break;

#ifdef BT_WIFI_HANDOBER
	case WLC_E_BT_WIFI_HANDOVER_REQ:
		DHD_EVENT(("MACEVENT: %s, MAC %s\n", event_name, eabuf));
		break;
#endif

	case WLC_E_CCA_CHAN_QUAL:
		if (datalen) {
			buf = (uchar *) event_data;
			DHD_EVENT(("MACEVENT: %s %d, MAC %s, status %d, reason %d, auth %d, "
				"channel 0x%02x \n", event_name, event_type, eabuf, (int)status,
				(int)reason, (int)auth_type, *(buf + 4)));
		}
		break;
	case WLC_E_ESCAN_RESULT:
	{
		DHD_EVENT(("MACEVENT: %s %d, MAC %s, status %d \n",
		       event_name, event_type, eabuf, (int)status));
	}
		break;
	case WLC_E_CAC_STATE_CHANGE:
		{
#if defined(BCMDBG) || defined(DHD_DEBUG)
			DHD_EVENT(("MACEVENT: %s %d, MAC %s, status %d \n",
				event_name, event_type, eabuf, (int)status));
			(void) wl_show_cac_event(event_data, datalen);
#endif /* defined(BCMDBG) || defined(DHD_DEBUG) */
		}
		break;
	default:
		DHD_EVENT(("MACEVENT: %s %d, MAC %s, status %d, reason %d, auth %d\n",
		       event_name, event_type, eabuf, (int)status, (int)reason,
		       (int)auth_type));
		break;
	}

	/* show any appended data if message level is set to bytes or host_data is set */
	if ((DHD_BYTES_ON() || (host_data == TRUE)) && DHD_EVENT_ON() && datalen) {
		buf = (uchar *) event_data;
		BCM_REFERENCE(buf);
		DHD_EVENT((" data (%d) : ", datalen));
		for (i = 0; i < datalen; i++)
			DHD_EVENT((" 0x%02x ", *buf++));
		DHD_EVENT(("\n"));
	}
}
#endif /* SHOW_EVENTS */

/* Stub for now. Will become real function as soon as shim
 * is being integrated to Android, Linux etc.
 */
int
wl_event_process_default(wl_event_msg_t *event, struct wl_evt_pport *evt_pport)
{
	return BCME_OK;
}

int
wl_event_process(dhd_pub_t *dhd_pub, int *ifidx, void *pktdata, void **data_ptr, void *raw_event)
{
	wl_evt_pport_t evt_pport;
	wl_event_msg_t event;

	/* make sure it is a BRCM event pkt and record event data */
	int ret = wl_host_event_get_data(pktdata, &event, data_ptr);
	if (ret != BCME_OK) {
		return ret;
	}

	/* convert event from network order to host order */
	wl_event_to_host_order(&event);

	/* record event params to evt_pport */
	evt_pport.dhd_pub = dhd_pub;
	evt_pport.ifidx = ifidx;
	evt_pport.pktdata = pktdata;
	evt_pport.data_ptr = data_ptr;
	evt_pport.raw_event = raw_event;

#if defined(WL_WLC_SHIM) && defined(WL_WLC_SHIM_EVENTS)
	{
		struct wl_shim_node *shim = dhd_pub_shim(dhd_pub);
		ASSERT(shim);
		ret = wl_shim_event_process(shim, &event, &evt_pport);
	}
#else
	ret = wl_event_process_default(&event, &evt_pport);
#endif

	return ret;
}

/* Check whether packet is a BRCM event pkt. If it is, record event data. */
int
wl_host_event_get_data(void *pktdata, wl_event_msg_t *event, void **data_ptr)
{
	bcm_event_t *pvt_data = (bcm_event_t *)pktdata;

	if (bcmp(BRCM_OUI, &pvt_data->bcm_hdr.oui[0], DOT11_OUI_LEN)) {
		DHD_ERROR(("%s: mismatched OUI, bailing\n", __FUNCTION__));
		return BCME_ERROR;
	}

	/* BRCM event pkt may be unaligned - use xxx_ua to load user_subtype. */
	if (ntoh16_ua((void *)&pvt_data->bcm_hdr.usr_subtype) != BCMILCP_BCM_SUBTYPE_EVENT) {
		DHD_ERROR(("%s: mismatched subtype, bailing\n", __FUNCTION__));
		return BCME_ERROR;
	}

	*data_ptr = &pvt_data[1];

	/* memcpy since BRCM event pkt may be unaligned. */
	memcpy(event, &pvt_data->event, sizeof(wl_event_msg_t));

	return BCME_OK;
}

int
wl_host_event(dhd_pub_t *dhd_pub, int *ifidx, void *pktdata, uint16 pktlen,
	wl_event_msg_t *event, void **data_ptr, void *raw_event)
{
	bcm_event_t *pvt_data;
	uint8 *event_data;
	uint32 type, status, datalen, reason;
	uint16 flags;
	int evlen;

	/* make sure it is a BRCM event pkt and record event data */
	int ret = wl_host_event_get_data(pktdata, event, data_ptr);
	if (ret != BCME_OK) {
		return ret;
	}

	if (pktlen < sizeof(bcm_event_t))
		return (BCME_ERROR);

	pvt_data = (bcm_event_t *)pktdata;

	if (ntoh16_ua((void *)&pvt_data->bcm_hdr.subtype) != BCMILCP_SUBTYPE_VENDOR_LONG ||
		(bcmp(BRCM_OUI, &pvt_data->bcm_hdr.oui[0], DOT11_OUI_LEN)) ||
		ntoh16_ua((void *)&pvt_data->bcm_hdr.usr_subtype) != BCMILCP_BCM_SUBTYPE_EVENT)
	{
		DHD_ERROR(("%s: mismatched bcm_event_t info, bailing out\n", __FUNCTION__));
		return (BCME_ERROR);
	}

	event_data = *data_ptr;

	type = ntoh32_ua((void *)&event->event_type);
	flags = ntoh16_ua((void *)&event->flags);
	status = ntoh32_ua((void *)&event->status);
	reason = ntoh32_ua((void *)&event->reason);
	datalen = ntoh32_ua((void *)&event->datalen);

	if (datalen > pktlen)
		return (BCME_ERROR);

	evlen = datalen + sizeof(bcm_event_t);

	if (evlen > pktlen)
		return (BCME_ERROR);

	switch (type) {
#ifdef PROP_TXSTATUS
	case WLC_E_FIFO_CREDIT_MAP:
		dhd_wlfc_enable(dhd_pub);
		dhd_wlfc_FIFOcreditmap_event(dhd_pub, event_data);
		WLFC_DBGMESG(("WLC_E_FIFO_CREDIT_MAP:(AC0,AC1,AC2,AC3),(BC_MC),(OTHER): "
			"(%d,%d,%d,%d),(%d),(%d)\n", event_data[0], event_data[1],
			event_data[2],
			event_data[3], event_data[4], event_data[5]));
		break;

	case WLC_E_BCMC_CREDIT_SUPPORT:
		dhd_wlfc_BCMCCredit_support_event(dhd_pub);
		break;
#endif

	case WLC_E_IF:
		{
		struct wl_event_data_if *ifevent = (struct wl_event_data_if *)event_data;

		/* Ignore the event if NOIF is set */
		if (ifevent->reserved & WLC_E_IF_FLAGS_BSSCFG_NOIF) {
			DHD_ERROR(("WLC_E_IF: NO_IF set, event Ignored\r\n"));
			return (BCME_UNSUPPORTED);
		}
#ifdef PCIE_FULL_DONGLE
		dhd_update_interface_flow_info(dhd_pub, ifevent->ifidx,
			ifevent->opcode, ifevent->role);
#endif
#if defined(BCM_GMAC3) && !defined(BCA_HNDROUTER)
		dhd_update_wofa_learning(dhd_pub, ifevent->ifidx);
#endif

#ifdef PROP_TXSTATUS
		{
			uint8* ea = pvt_data->eth.ether_dhost;
			WLFC_DBGMESG(("WLC_E_IF: idx:%d, action:%s, iftype:%s, "
			              "[%02x:%02x:%02x:%02x:%02x:%02x]\n",
			              ifevent->ifidx,
			              ((ifevent->opcode == WLC_E_IF_ADD) ? "ADD":"DEL"),
			              ((ifevent->role == 0) ? "STA":"AP "),
			              ea[0], ea[1], ea[2], ea[3], ea[4], ea[5]));
			(void)ea;

			if (ifevent->opcode == WLC_E_IF_CHANGE)
				dhd_wlfc_interface_event(dhd_pub,
					eWLFC_MAC_ENTRY_ACTION_UPDATE,
					ifevent->ifidx, ifevent->role, ea);
			else
				dhd_wlfc_interface_event(dhd_pub,
					((ifevent->opcode == WLC_E_IF_ADD) ?
					eWLFC_MAC_ENTRY_ACTION_ADD : eWLFC_MAC_ENTRY_ACTION_DEL),
					ifevent->ifidx, ifevent->role, ea);

			/* dhd already has created an interface by default, for 0 */
			if (ifevent->ifidx == 0)
				break;
		}
#endif /* PROP_TXSTATUS */

		if (ifevent->ifidx > 0 && ifevent->ifidx < DHD_MAX_IFS) {
			if (ifevent->opcode == WLC_E_IF_ADD) {
				if (dhd_event_ifadd(dhd_pub->info, ifevent, event->ifname,
					event->addr.octet)) {

					DHD_ERROR(("%s: %s: dhd_event_ifadd failed ifidx: %d  %s\n",
						dhd_ifname(dhd_pub, ifevent->ifidx), __FUNCTION__,
						ifevent->ifidx, event->ifname));
					return (BCME_ERROR);
				}
			} else if (ifevent->opcode == WLC_E_IF_DEL) {
#ifdef PCIE_FULL_DONGLE
				/* Delete flowrings unconditionally for i/f delete */
				dhd_flow_rings_delete(dhd_pub, (uint8)dhd_ifname2idx(dhd_pub->info,
					event->ifname));
#endif /* PCIE_FULL_DONGLE */
				dhd_event_ifdel(dhd_pub->info, ifevent, event->ifname,
					event->addr.octet);
			} else if (ifevent->opcode == WLC_E_IF_CHANGE) {
#ifdef WL_CFG80211
				dhd_event_ifchange(dhd_pub->info, ifevent, event->ifname,
					event->addr.octet);
#endif /* WL_CFG80211 */
			}
		} else {
#if !defined(PROP_TXSTATUS) && !defined(PCIE_FULL_DONGLE) && defined(WL_CFG80211)
			DHD_ERROR(("%s: %s: Invalid ifidx %d for %s\n",
				dhd_ifname(dhd_pub, ifidx), __FUNCTION__,
				ifevent->ifidx, event->ifname));
#endif /* !PROP_TXSTATUS && !PCIE_FULL_DONGLE && WL_CFG80211 */
		}
			/* send up the if event: btamp user needs it */
			*ifidx = dhd_ifname2idx(dhd_pub->info, event->ifname);
			/* push up to external supp/auth */
			dhd_event(dhd_pub->info, (char *)pvt_data, evlen, *ifidx);
		break;
	}

#ifdef WLMEDIA_HTSF
	case WLC_E_HTSFSYNC:
		htsf_update(dhd_pub->info, event_data);
		break;
#endif /* WLMEDIA_HTSF */
	case WLC_E_PFN_NET_FOUND:
	case WLC_E_PFN_SCAN_ALLGONE: /* share with WLC_E_PFN_BSSID_NET_LOST */
	case WLC_E_PFN_NET_LOST:
		break;
#if defined(OEM_ANDROID) && defined(PNO_SUPPORT)
	case WLC_E_PFN_BSSID_NET_FOUND:
	case WLC_E_PFN_BEST_BATCHING:
		dhd_pno_event_handler(dhd_pub, event, (void *)event_data);
		break;
#endif /* #if defined(OEM_ANDROID) && defined(PNO_SUPPORT) */
		/* These are what external supplicant/authenticator wants */
	case WLC_E_ASSOC_IND:
	case WLC_E_AUTH_IND:
	case WLC_E_REASSOC_IND:
		dhd_findadd_sta(dhd_pub,
			dhd_ifname2idx(dhd_pub->info, event->ifname),
			&event->addr.octet, TRUE);
		/* fall through */
#if defined(BCM_PKTFWD)
	case WLC_E_ASSOC:
		if (type != WLC_E_AUTH_IND)
			dhd_pktfwd_request(dhd_pktfwd_req_assoc_sta_e,
				(unsigned long)&event->addr.octet, 1, event->event_type);
#if defined(BCM_PKTFWD_DWDS)
		if (DHD_IF_ROLE_STA(dhd_pub, dhd_ifname2idx(dhd_pub->info, event->ifname))) {
			dhd_alloc_dwds_idx(dhd_pub, dhd_ifname2idx(dhd_pub->info, event->ifname));
		}
#endif /* BCM_PKTFWD_DWDS */
#endif /* BCM_PKTFWD */
		break;

#if !defined(BCMDBUS) && defined(DHD_FW_COREDUMP)
	case WLC_E_PSM_WATCHDOG:
		DHD_ERROR(("%s: WLC_E_PSM_WATCHDOG event received : \n", __FUNCTION__));
		if (dhd_socram_dump(dhd_pub->bus) != BCME_OK) {
			DHD_ERROR(("%s: socram dump ERROR : \n", __FUNCTION__));
		}
	break;
#endif
#ifdef DHD_WMF
	case WLC_E_PSTA_PRIMARY_INTF_IND:
		dhd_update_psta_interface_for_sta(dhd_pub, event->ifname,
			(void *)(event->addr.octet), (void*) event_data);
		break;
#endif
#ifdef BCM_ROUTER_DHD
	case WLC_E_DPSTA_INTF_IND:
		dhd_update_dpsta_interface_for_sta(dhd_pub, (uint8)dhd_ifname2idx(dhd_pub->info,
			event->ifname), (void*) event_data);
		break;
#endif /* BCM_ROUTER_DHD */
	case WLC_E_MACDBG:
		dhd_macdbg_event_handler(dhd_pub, reason, event_data, datalen);
		break;
#ifdef BCMHWA
	case WLC_E_HWA_EVENT:
		dhd_prot_process_hwa_event(dhd_pub, reason);
		break;
#endif /* BCMHWA */
	case WLC_E_LINK:
#ifdef PCIE_FULL_DONGLE
		if (dhd_update_interface_link_status(dhd_pub, (uint8)dhd_ifname2idx(dhd_pub->info,
			event->ifname), (uint8)flags) != BCME_OK)
			break;
		if (!flags) {
			dhd_flow_rings_delete(dhd_pub, (uint8)dhd_ifname2idx(dhd_pub->info,
				event->ifname));
#if defined(BCM_GMAC3) && !defined(BCA_HNDROUTER)
			/* Delete all WOFA entries for the upstream hosts */
			dhd_del_allsta(dhd_pub, dhd_ifname2idx(dhd_pub->info,
				event->ifname));
#endif /* BCM_GMAC3 && !BCA_HNDROUTER */
		} else {
#ifdef BCMHWA
			/* Update RxPost WR to dongle manually in order to trigger iDMA again. */
			dhd_prot_rxpost_upd(dhd_pub);
#endif
		}
#endif /* PCIE_FULL_DONGLE */

		/* fall through */
	case WLC_E_DEAUTH:
	case WLC_E_DEAUTH_IND:
	case WLC_E_DISASSOC:
	case WLC_E_DISASSOC_IND:
		if (type != WLC_E_LINK) {
			dhd_del_sta(dhd_pub, dhd_ifname2idx(dhd_pub->info,
				event->ifname), &event->addr.octet);
		}
		DHD_EVENT(("%s: Link event %d, flags %x, status %x\n",
		           __FUNCTION__, type, flags, status));
#ifdef PCIE_FULL_DONGLE
		if (type != WLC_E_LINK) {
			uint8 ifindex = (uint8)dhd_ifname2idx(dhd_pub->info, event->ifname);
			uint8 role = dhd_flow_rings_ifindex2role(dhd_pub, ifindex);
			if (role == WLC_E_IF_ROLE_STA) {
				dhd_flow_rings_delete(dhd_pub, ifindex);
			} else {
				dhd_flow_rings_delete_for_peer(dhd_pub, ifindex,
					&event->addr.octet[0]);
			}
		}
#endif
#ifdef BCM_BLOG
		if (type != WLC_E_LINK) {
			dhd_handle_blog_disconnect_event(dhd_pub, event);
		}
#endif /* BCM_BLOG */
		/* fall through */

	default:
		*ifidx = dhd_ifname2idx(dhd_pub->info, event->ifname);
		/* push up to external supp/auth */
		dhd_event(dhd_pub->info, (char *)pvt_data, evlen, *ifidx);
		DHD_TRACE(("%s: MAC event %d, flags %x, status %x\n",
		           __FUNCTION__, type, flags, status));
		BCM_REFERENCE(flags);
		BCM_REFERENCE(status);
		BCM_REFERENCE(reason);

		break;
	}
#if defined(BCM_ROUTER_DHD) || defined(STBAP)
	/* For routers, EAPD will be working on these events.
	 * Overwrite interface name to that event is pushed
	 * to host with its registered interface name
	 */
	memcpy(pvt_data->event.ifname, dhd_ifname(dhd_pub, *ifidx), IFNAMSIZ);
#endif

#ifdef SHOW_EVENTS
	wl_show_host_event(dhd_pub, event,
		(void *)event_data, raw_event, dhd_pub->enable_log);
#endif /* SHOW_EVENTS */

	return (BCME_OK);
}

void
dhd_print_buf(void *pbuf, int len, int bytes_per_line)
{
#ifdef DHD_DEBUG
	int i, j = 0;
	unsigned char *buf = pbuf;

	if (bytes_per_line == 0) {
		bytes_per_line = len;
	}

	for (i = 0; i < len; i++) {
		printf("%2.2x", *buf++);
		j++;
		if (j == bytes_per_line) {
			printf("\n");
			j = 0;
		} else {
			printf(":");
		}
	}
	printf("\n");
#endif /* DHD_DEBUG */
}
#ifndef strtoul
#define strtoul(nptr, endptr, base) bcm_strtoul((nptr), (endptr), (base))
#endif

#ifdef PKT_FILTER_SUPPORT
/* Convert user's input in hex pattern to byte-size mask */
static int
wl_pattern_atoh(char *src, char *dst)
{
	int i;

	if (strncmp(src, "0x", 2) != 0 &&
	    strncmp(src, "0X", 2) != 0) {
		DHD_ERROR(("Mask invalid format. Needs to start with 0x\n"));
		return -1;
	}

	src = src + 2; /* Skip past 0x */
	if (strlen(src) % 2 != 0) {
		DHD_ERROR(("Mask invalid format. Needs to be of even length\n"));
		return -1;
	}

	for (i = 0; *src != '\0'; i++) {
		char num[3];
		bcm_strncpy_s(num, sizeof(num), src, 2);
		num[2] = '\0';
		dst[i] = (uint8)strtoul(num, NULL, 16);
		src += 2;
	}

	return i;
}

void
dhd_pktfilter_offload_enable(dhd_pub_t * dhd, char *arg, int enable, int master_mode)
{
	char				*argv[8];
	int					i = 0;
	const char			*str;
	int					buf_len;
	int					str_len;
	char				*arg_save = 0, *arg_org = 0;
	int					rc;
	char				buf[32] = {0};
	wl_pkt_filter_enable_t	enable_parm;
	wl_pkt_filter_enable_t	* pkt_filterp;

	if (!arg)
		return;

	if (!(arg_save = MALLOC(dhd->osh, strlen(arg) + 1))) {
		DHD_ERROR(("%s: malloc failed\n", __FUNCTION__));
		goto fail;
	}
	arg_org = arg_save;
	memcpy(arg_save, arg, strlen(arg) + 1);

	argv[i] = bcmstrtok(&arg_save, " ", 0);

	i = 0;
	if (argv[i] == NULL) {
		DHD_ERROR(("No args provided\n"));
		goto fail;
	}

	str = "pkt_filter_enable";
	str_len = strlen(str);
	bcm_strncpy_s(buf, sizeof(buf) - 1, str, sizeof(buf) - 1);
	buf[ sizeof(buf) - 1 ] = '\0';
	buf_len = str_len + 1;

	pkt_filterp = (wl_pkt_filter_enable_t *)(buf + str_len + 1);

	/* Parse packet filter id. */
	enable_parm.id = htod32(strtoul(argv[i], NULL, 0));

	/* Parse enable/disable value. */
	enable_parm.enable = htod32(enable);

	buf_len += sizeof(enable_parm);
	memcpy((char *)pkt_filterp,
	       &enable_parm,
	       sizeof(enable_parm));

	/* Enable/disable the specified filter. */
	rc = dhd_wl_ioctl_cmd(dhd, WLC_SET_VAR, buf, buf_len, TRUE, 0);
	rc = rc >= 0 ? 0 : rc;
	if (rc)
		DHD_TRACE(("%s: failed to add pktfilter %s, retcode = %d\n",
		__FUNCTION__, arg, rc));
	else
		DHD_TRACE(("%s: successfully added pktfilter %s\n",
		__FUNCTION__, arg));

	/* Contorl the master mode */
	rc = dhd_wl_ioctl_set_intiovar(dhd, "pkt_filter_mode",
		master_mode, WLC_SET_VAR, TRUE, 0);
	rc = rc >= 0 ? 0 : rc;
	if (rc)
		DHD_TRACE(("%s: failed to add pktfilter %s, retcode = %d\n",
		__FUNCTION__, arg, rc));

fail:
	if (arg_org)
		MFREE(dhd->osh, arg_org, strlen(arg) + 1);
}

void
dhd_pktfilter_offload_set(dhd_pub_t * dhd, char *arg)
{
	const char 			*str;
	wl_pkt_filter_t		pkt_filter;
	wl_pkt_filter_t		*pkt_filterp;
	int					buf_len;
	int					str_len;
	int 				rc;
	uint32				mask_size;
	uint32				pattern_size;
	char				*argv[MAXPKT_ARG] = {0}, * buf = 0;
	int					i = 0;
	char				*arg_save = 0, *arg_org = 0;
#define BUF_SIZE		2048

	if (!arg)
		return;

	if (!(arg_save = MALLOC(dhd->osh, strlen(arg) + 1))) {
		DHD_ERROR(("%s: malloc failed\n", __FUNCTION__));
		goto fail;
	}

	arg_org = arg_save;

	if (!(buf = MALLOC(dhd->osh, BUF_SIZE))) {
		DHD_ERROR(("%s: malloc failed\n", __FUNCTION__));
		goto fail;
	}

	memcpy(arg_save, arg, strlen(arg) + 1);

	if (strlen(arg) > BUF_SIZE) {
		DHD_ERROR(("Not enough buffer %d < %d\n", (int)strlen(arg), (int)sizeof(buf)));
		goto fail;
	}

	argv[i] = bcmstrtok(&arg_save, " ", 0);
	while (argv[i++]) {
		if (i >= MAXPKT_ARG) {
			DHD_ERROR(("Invalid args provided\n"));
			goto fail;
		}
		argv[i] = bcmstrtok(&arg_save, " ", 0);
	}

	i = 0;
	if (argv[i] == NULL) {
		DHD_ERROR(("No args provided\n"));
		goto fail;
	}

	str = "pkt_filter_add";
	str_len = strlen(str);
	bcm_strncpy_s(buf, BUF_SIZE, str, str_len);
	buf[ str_len ] = '\0';
	buf_len = str_len + 1;

	pkt_filterp = (wl_pkt_filter_t *) (buf + str_len + 1);

	/* Parse packet filter id. */
	pkt_filter.id = htod32(strtoul(argv[i], NULL, 0));

	if (argv[++i] == NULL) {
		DHD_ERROR(("Polarity not provided\n"));
		goto fail;
	}

	/* Parse filter polarity. */
	pkt_filter.negate_match = htod32(strtoul(argv[i], NULL, 0));

	if (argv[++i] == NULL) {
		DHD_ERROR(("Filter type not provided\n"));
		goto fail;
	}

	/* Parse filter type. */
	pkt_filter.type = htod32(strtoul(argv[i], NULL, 0));

	if (argv[++i] == NULL) {
		DHD_ERROR(("Offset not provided\n"));
		goto fail;
	}

	/* Parse pattern filter offset. */
	pkt_filter.u.pattern.offset = htod32(strtoul(argv[i], NULL, 0));

	if (argv[++i] == NULL) {
		DHD_ERROR(("Bitmask not provided\n"));
		goto fail;
	}

	/* Parse pattern filter mask. */
	mask_size =
		htod32(wl_pattern_atoh(argv[i], (char *) pkt_filterp->u.pattern.mask_and_pattern));

	if (argv[++i] == NULL) {
		DHD_ERROR(("Pattern not provided\n"));
		goto fail;
	}

	/* Parse pattern filter pattern. */
	pattern_size =
		htod32(wl_pattern_atoh(argv[i],
	         (char *) &pkt_filterp->u.pattern.mask_and_pattern[mask_size]));

	if (mask_size != pattern_size) {
		DHD_ERROR(("Mask and pattern not the same size\n"));
		goto fail;
	}

	pkt_filter.u.pattern.size_bytes = mask_size;
	buf_len += WL_PKT_FILTER_FIXED_LEN;
	buf_len += (WL_PKT_FILTER_PATTERN_FIXED_LEN + 2 * mask_size);

	/* Keep-alive attributes are set in local	variable (keep_alive_pkt), and
	** then memcpy'ed into buffer (keep_alive_pktp) since there is no
	** guarantee that the buffer is properly aligned.
	*/
	memcpy((char *)pkt_filterp,
	       &pkt_filter,
	       WL_PKT_FILTER_FIXED_LEN + WL_PKT_FILTER_PATTERN_FIXED_LEN);

	rc = dhd_wl_ioctl_cmd(dhd, WLC_SET_VAR, buf, buf_len, TRUE, 0);
	rc = rc >= 0 ? 0 : rc;

	if (rc)
		DHD_TRACE(("%s: failed to add pktfilter %s, retcode = %d\n",
		__FUNCTION__, arg, rc));
	else
		DHD_TRACE(("%s: successfully added pktfilter %s\n",
		__FUNCTION__, arg));

fail:
	if (arg_org)
		MFREE(dhd->osh, arg_org, strlen(arg) + 1);

	if (buf)
		MFREE(dhd->osh, buf, BUF_SIZE);
}

void
dhd_pktfilter_offload_delete(dhd_pub_t *dhd, int id)
{
	int ret;

	ret = dhd_wl_ioctl_set_intiovar(dhd, "pkt_filter_delete",
		id, WLC_SET_VAR, TRUE, 0);
	if (ret < 0) {
		DHD_ERROR(("%s: Failed to delete filter ID:%d, ret=%d\n",
			__FUNCTION__, id, ret));
	}
}
#endif /* PKT_FILTER_SUPPORT */

/* ========================== */
/* ==== ARP OFFLOAD SUPPORT = */
/* ========================== */
#ifdef ARP_OFFLOAD_SUPPORT
void
dhd_arp_offload_set(dhd_pub_t * dhd, int arp_mode)
{
	int retcode;

	retcode = dhd_wl_ioctl_set_intiovar(dhd, "arp_ol",
		arp_mode, WLC_SET_VAR, TRUE, 0);

	retcode = retcode >= 0 ? 0 : retcode;
	if (retcode)
		DHD_TRACE(("%s: failed to set ARP offload mode to 0x%x, retcode = %d\n",
			__FUNCTION__, arp_mode, retcode));
	else
		DHD_TRACE(("%s: successfully set ARP offload mode to 0x%x\n",
			__FUNCTION__, arp_mode));
}

void
dhd_arp_offload_enable(dhd_pub_t * dhd, int arp_enable)
{
	int retcode;

	retcode = dhd_wl_ioctl_set_intiovar(dhd, "arpoe",
		arp_enable, WLC_SET_VAR, TRUE, 0);

	retcode = retcode >= 0 ? 0 : retcode;
	if (retcode)
		DHD_TRACE(("%s: failed to enabe ARP offload to %d, retcode = %d\n",
			__FUNCTION__, arp_enable, retcode));
	else
		DHD_TRACE(("%s: successfully enabed ARP offload to %d\n",
			__FUNCTION__, arp_enable));
	if (arp_enable) {
		uint32 version;
		retcode = dhd_wl_ioctl_get_intiovar(dhd, "arp_version",
			&version, WLC_GET_VAR, FALSE, 0);
		if (retcode) {
			DHD_INFO(("%s: fail to get version (maybe version 1:retcode = %d\n",
				__FUNCTION__, retcode));
			dhd->wlcore->arp_version = 1;
		}
		else {
			DHD_INFO(("%s: ARP Version= %x\n", __FUNCTION__, version));
			dhd->wlcore->arp_version = version;
		}
	}
}

void
dhd_aoe_arp_clr(dhd_pub_t *dhd, int idx)
{
	int ret = 0;

	if (dhd == NULL) return;
	if (dhd->wlcore->arp_version == 1)
		idx = 0;

	ret = dhd_iovar(dhd, idx, "arp_table_clear", NULL, 0, NULL, 0, TRUE);
	if (ret < 0)
		DHD_ERROR(("%s failed code %d\n", __FUNCTION__, ret));
}

void
dhd_aoe_hostip_clr(dhd_pub_t *dhd, int idx)
{
	int ret = 0;

	if (dhd == NULL) return;
	if (dhd->wlcore->arp_version == 1)
		idx = 0;

	ret = dhd_iovar(dhd, idx, "arp_hostip_clear", NULL, 0, NULL, 0, TRUE);
	if (ret < 0)
		DHD_ERROR(("%s failed code %d\n", __FUNCTION__, ret));
}

void
dhd_arp_offload_add_ip(dhd_pub_t *dhd, uint32 ipaddr, int idx)
{
	int ret;

	if (dhd == NULL) return;
	if (dhd->wlcore->arp_version == 1)
		idx = 0;

	ret = dhd_iovar(dhd, idx, "arp_hostip", (char *)&ipaddr, sizeof(ipaddr),
			NULL, 0, TRUE);
	if (ret)
		DHD_TRACE(("%s: ARP ip addr add failed, ret = %d\n", __FUNCTION__, ret));
	else
		DHD_TRACE(("%s: sARP H ipaddr entry added \n",
		__FUNCTION__));
}

int
dhd_arp_get_arp_hostip_table(dhd_pub_t *dhd, void *buf, int buflen, int idx)
{
	int ret, i;
	uint32 *ptr32 = buf;
	bool clr_bottom = FALSE;

	if (!buf)
		return -1;
	if (dhd == NULL) return -1;
	if (dhd->wlcore->arp_version == 1)
		idx = 0;

	ret = dhd_iovar(dhd, idx, "arp_hostip", NULL, 0, (char *)buf, buflen,
			FALSE);
	if (ret) {
		DHD_TRACE(("%s: ioctl WLC_GET_VAR error %d\n",
		__FUNCTION__, ret));

		return -1;
	}

	/* clean up the buf, ascii reminder */
	for (i = 0; i < MAX_IPV4_ENTRIES; i++) {
		if (!clr_bottom) {
			if (*ptr32 == 0)
				clr_bottom = TRUE;
		} else {
			*ptr32 = 0;
		}
		ptr32++;
	}

	return 0;
}
#endif /* ARP_OFFLOAD_SUPPORT  */

/*
 * Neighbor Discovery Offload: enable NDO feature
 * Called  by ipv6 event handler when interface comes up/goes down
 */
int
dhd_ndo_enable(dhd_pub_t * dhd, int ndo_enable)
{
	int retcode;

	if (dhd == NULL)
		return -1;

	retcode = dhd_wl_ioctl_set_intiovar(dhd, "ndoe",
		ndo_enable, WLC_SET_VAR, TRUE, 0);
	if (retcode)
		DHD_ERROR(("%s: failed to enabe ndo to %d, retcode = %d\n",
			__FUNCTION__, ndo_enable, retcode));
	else
		DHD_TRACE(("%s: successfully enabed ndo offload to %d\n",
			__FUNCTION__, ndo_enable));

	return retcode;
}

/*
 * Neighbor Discover Offload: enable NDO feature
 * Called  by ipv6 event handler when interface comes up
 */
int
dhd_ndo_add_ip(dhd_pub_t *dhd, char* ipv6addr, int idx)
{
	int iov_len = 0;
	char iovbuf[DHD_IOVAR_BUF_SIZE];
	int retcode;

	if (dhd == NULL)
		return -1;

	iov_len = bcm_mkiovar("nd_hostip", (char *)ipv6addr,
		IPV6_ADDR_LEN, iovbuf, sizeof(iovbuf));
	if (!iov_len) {
		DHD_ERROR(("%s: Insufficient iovar buffer size %zu \n",
			__FUNCTION__, sizeof(iovbuf)));
		return -1;
	}
	retcode = dhd_wl_ioctl_cmd(dhd, WLC_SET_VAR, iovbuf, iov_len, TRUE, idx);

	if (retcode)
		DHD_ERROR(("%s: ndo ip addr add failed, retcode = %d\n",
		__FUNCTION__, retcode));
	else
		DHD_TRACE(("%s: ndo ipaddr entry added \n",
		__FUNCTION__));

	return retcode;
}
/*
 * Neighbor Discover Offload: enable NDO feature
 * Called  by ipv6 event handler when interface goes down
 */
int
dhd_ndo_remove_ip(dhd_pub_t *dhd, int idx)
{
	int iov_len = 0;
	char iovbuf[DHD_IOVAR_BUF_SIZE];
	int retcode;

	if (dhd == NULL)
		return -1;

	iov_len = bcm_mkiovar("nd_hostip_clear", NULL,
		0, iovbuf, sizeof(iovbuf));
	if (!iov_len) {
		DHD_ERROR(("%s: Insufficient iovar buffer size %zu \n",
			__FUNCTION__, sizeof(iovbuf)));
		return -1;
	}
	retcode = dhd_wl_ioctl_cmd(dhd, WLC_SET_VAR, iovbuf, iov_len, TRUE, idx);

	if (retcode)
		DHD_ERROR(("%s: ndo ip addr remove failed, retcode = %d\n",
		__FUNCTION__, retcode));
	else
		DHD_TRACE(("%s: ndo ipaddr entry removed \n",
		__FUNCTION__));

	return retcode;
}

#ifdef WLBTAMP

/* send up locally generated event */
void
dhd_sendup_event_common(dhd_pub_t *dhdp, wl_event_msg_t *event, void *data)
{
	switch (ntoh32(event->event_type)) {
	case WLC_E_BTA_HCI_EVENT:
#ifdef BCMDBG
		if (DHD_BTA_ON())
			dhd_bta_hcidump_evt(dhdp, data);
#endif
		break;
	default:
		break;
	}

	/* Call per-port handler. */
	dhd_sendup_event(dhdp, event, data);
}
#endif /* WLBTAMP */

#ifdef SIMPLE_ISCAN

uint iscan_thread_id = 0;
iscan_buf_t * iscan_chain = 0;

iscan_buf_t *
dhd_iscan_allocate_buf(dhd_pub_t *dhd, iscan_buf_t **iscanbuf)
{
	iscan_buf_t *iscanbuf_alloc = 0;
	iscan_buf_t *iscanbuf_head;

	DHD_ISCAN(("%s: Entered\n", __FUNCTION__));
	dhd_iscan_lock();

	iscanbuf_alloc = (iscan_buf_t*)MALLOC(dhd->osh, sizeof(iscan_buf_t));
	if (iscanbuf_alloc == NULL)
		goto fail;

	iscanbuf_alloc->next = NULL;
	iscanbuf_head = *iscanbuf;

	DHD_ISCAN(("%s: addr of allocated node = 0x%X"
		   "addr of iscanbuf_head = 0x%X dhd = 0x%X\n",
		   __FUNCTION__, iscanbuf_alloc, iscanbuf_head, dhd));

	if (iscanbuf_head == NULL) {
		*iscanbuf = iscanbuf_alloc;
		DHD_ISCAN(("%s: Head is allocated\n", __FUNCTION__));
		goto fail;
	}

	while (iscanbuf_head->next)
		iscanbuf_head = iscanbuf_head->next;

	iscanbuf_head->next = iscanbuf_alloc;

fail:
	dhd_iscan_unlock();

	return iscanbuf_alloc;
}

void
dhd_iscan_free_buf(void *dhdp, iscan_buf_t *iscan_delete)
{
	iscan_buf_t *iscanbuf_free = 0;
	iscan_buf_t *iscanbuf_prv = 0;
	iscan_buf_t *iscanbuf_cur;
	dhd_pub_t *dhd = dhd_bus_pub(dhdp);
	DHD_ISCAN(("%s: Entered\n", __FUNCTION__));

	dhd_iscan_lock();

	iscanbuf_cur = iscan_chain;

	/* If iscan_delete is null then delete the entire
	 * chain or else delete specific one provided
	 */
	if (!iscan_delete) {
		while (iscanbuf_cur) {
			iscanbuf_free = iscanbuf_cur;
			iscanbuf_cur = iscanbuf_cur->next;
			iscanbuf_free->next = 0;
			MFREE(dhd->osh, iscanbuf_free, sizeof(iscan_buf_t));
		}
		iscan_chain = 0;
	} else {
		while (iscanbuf_cur) {
			if (iscanbuf_cur == iscan_delete)
				break;
			iscanbuf_prv = iscanbuf_cur;
			iscanbuf_cur = iscanbuf_cur->next;
		}
		if (iscanbuf_prv)
			iscanbuf_prv->next = iscan_delete->next;

		iscan_delete->next = 0;
		MFREE(dhd->osh, iscan_delete, sizeof(iscan_buf_t));

		if (!iscanbuf_prv)
			iscan_chain = 0;
	}
	dhd_iscan_unlock();
}

iscan_buf_t *
dhd_iscan_result_buf(void)
{
	return iscan_chain;
}

int
dhd_iscan_issue_request(void * dhdp, wl_iscan_params_t *pParams, uint32 size)
{
	int rc = -1;
	dhd_pub_t *dhd = dhd_bus_pub(dhdp);
	char *buf;
	char iovar[] = "iscan";
	uint32 allocSize = 0;
	wl_ioctl_t ioctl;

	if (pParams) {
		allocSize = (size + strlen(iovar) + 1);
		if ((allocSize < size) || (allocSize < strlen(iovar)))
		{
			DHD_ERROR(("%s: overflow - allocation size too large %d < %d + %d!\n",
				__FUNCTION__, allocSize, size, strlen(iovar)));
			goto cleanUp;
		}
		buf = MALLOC(dhd->osh, allocSize);

		if (buf == NULL)
			{
			DHD_ERROR(("%s: malloc of size %d failed!\n", __FUNCTION__, allocSize));
			goto cleanUp;
			}
		ioctl.cmd = WLC_SET_VAR;
		bcm_mkiovar(iovar, (char *)pParams, size, buf, allocSize);
		rc = dhd_wl_ioctl(dhd, 0, &ioctl, buf, allocSize);
	}

cleanUp:
	if (buf) {
		MFREE(dhd->osh, buf, allocSize);
	}

	return rc;
}

static int
dhd_iscan_get_partial_result(void *dhdp, uint *scan_count)
{
	wl_iscan_results_t *list_buf;
	wl_iscan_results_t list;
	wl_scan_results_t *results;
	iscan_buf_t *iscan_cur;
	int status = -1;
	dhd_pub_t *dhd = dhd_bus_pub(dhdp);
	int rc;
	wl_ioctl_t ioctl;

	DHD_ISCAN(("%s: Enter\n", __FUNCTION__));

	iscan_cur = dhd_iscan_allocate_buf(dhd, &iscan_chain);
	if (!iscan_cur) {
		DHD_ERROR(("%s: Failed to allocate node\n", __FUNCTION__));
		dhd_iscan_free_buf(dhdp, 0);
		dhd_iscan_request(dhdp, WL_SCAN_ACTION_ABORT);
		dhd_ind_scan_confirm(dhdp, FALSE);
		goto fail;
	}

	dhd_iscan_lock();

	memset(iscan_cur->iscan_buf, 0, WLC_IW_ISCAN_MAXLEN);
	list_buf = (wl_iscan_results_t*)iscan_cur->iscan_buf;
	results = &list_buf->results;
	results->buflen = WL_ISCAN_RESULTS_FIXED_SIZE;
	results->version = 0;
	results->count = 0;

	memset(&list, 0, sizeof(list));
	list.results.buflen = htod32(WLC_IW_ISCAN_MAXLEN);
	bcm_mkiovar("iscanresults", (char *)&list, WL_ISCAN_RESULTS_FIXED_SIZE,
		iscan_cur->iscan_buf, WLC_IW_ISCAN_MAXLEN);
	ioctl.cmd = WLC_GET_VAR;
	ioctl.set = FALSE;
	rc = dhd_wl_ioctl(dhd, 0, &ioctl, iscan_cur->iscan_buf, WLC_IW_ISCAN_MAXLEN);

	results->buflen = dtoh32(results->buflen);
	results->version = dtoh32(results->version);
	*scan_count = results->count = dtoh32(results->count);
	status = dtoh32(list_buf->status);
	DHD_ISCAN(("%s: Got %d resuls status = (%x)\n", __FUNCTION__, results->count, status));

	dhd_iscan_unlock();

	if (!(*scan_count)) {
		 /* TODO: race condition when FLUSH already called */
		dhd_iscan_free_buf(dhdp, 0);
	}

fail:
	return status;
}

#endif /* SIMPLE_ISCAN */

/* Function to estimate possible DTIM_SKIP value */
int
dhd_get_suspend_bcn_li_dtim(dhd_pub_t *dhd)
{
	int bcn_li_dtim = 1; /* deafult no dtim skip setting */
	int ret = -1;
	int dtim_period = 0;
	int ap_beacon = 0;
	int allowed_skip_dtim_cnt = 0;
	/* Check if associated */
	if (wl_is_associated(dhd_linux_get_primary_netdev(dhd), NULL) == FALSE) {
		DHD_TRACE(("%s NOT assoc ret %d\n", __FUNCTION__, ret));
		goto exit;
	}

	/* read associated AP beacon interval */
	if ((ret = dhd_wl_ioctl_cmd(dhd, WLC_GET_BCNPRD,
		&ap_beacon, sizeof(ap_beacon), FALSE, 0)) < 0) {
		DHD_ERROR(("%s get beacon failed code %d\n", __FUNCTION__, ret));
		goto exit;
	}

	/* read associated ap's dtim setup */
	if ((ret = dhd_wl_ioctl_cmd(dhd, WLC_GET_DTIMPRD,
		&dtim_period, sizeof(dtim_period), FALSE, 0)) < 0) {
		DHD_ERROR(("%s failed code %d\n", __FUNCTION__, ret));
		goto exit;
	}

	/* if not assocated just eixt */
	if (dtim_period == 0) {
		goto exit;
	}

	/* attemp to use platform defined dtim skip interval */
	bcn_li_dtim = dhd->suspend_bcn_li_dtim;

	/* check if sta listen interval fits into AP dtim */
	if (dtim_period > CUSTOM_LISTEN_INTERVAL) {
		/* AP DTIM to big for our Listen Interval : no dtim skiping */
		bcn_li_dtim = NO_DTIM_SKIP;
		DHD_ERROR(("%s DTIM=%d > Listen=%d : too big ...\n",
			__FUNCTION__, dtim_period, CUSTOM_LISTEN_INTERVAL));
		goto exit;
	}

	if ((dtim_period * ap_beacon * bcn_li_dtim) > MAX_DTIM_ALLOWED_INTERVAL) {
		 allowed_skip_dtim_cnt = MAX_DTIM_ALLOWED_INTERVAL / (dtim_period * ap_beacon);
		 bcn_li_dtim = (allowed_skip_dtim_cnt != 0) ? allowed_skip_dtim_cnt : NO_DTIM_SKIP;
	}

	if ((bcn_li_dtim * dtim_period) > CUSTOM_LISTEN_INTERVAL) {
		/* Round up dtim_skip to fit into STAs Listen Interval */
		bcn_li_dtim = (int)(CUSTOM_LISTEN_INTERVAL / dtim_period);
		DHD_TRACE(("%s agjust dtim_skip as %d\n", __FUNCTION__, bcn_li_dtim));
	}

	DHD_ERROR(("%s beacon=%d bcn_li_dtim=%d DTIM=%d Listen=%d\n",
		__FUNCTION__, ap_beacon, bcn_li_dtim, dtim_period, CUSTOM_LISTEN_INTERVAL));

exit:
	return bcn_li_dtim;
}

/* Check if the mode supports STA MODE */
bool
dhd_support_sta_mode(dhd_pub_t *dhd)
{
	return wl_support_sta_mode(dhd->wlcore);
}

#if defined(KEEP_ALIVE)
int
dhd_keep_alive_onoff(dhd_pub_t *dhd)
{
	char				buf[32] = {0};
	const char			*str;
	wl_mkeep_alive_pkt_t	mkeep_alive_pkt = {0};
	wl_mkeep_alive_pkt_t	*mkeep_alive_pktp;
	int					buf_len;
	int					str_len;
	int res					= -1;

	if (!dhd_support_sta_mode(dhd))
		return res;

	DHD_TRACE(("%s execution\n", __FUNCTION__));

	str = "mkeep_alive";
	str_len = strlen(str);
	strncpy(buf, str, sizeof(buf) - 1);
	buf[ sizeof(buf) - 1 ] = '\0';
	mkeep_alive_pktp = (wl_mkeep_alive_pkt_t *) (buf + str_len + 1);
	mkeep_alive_pkt.period_msec = CUSTOM_KEEP_ALIVE_SETTING;
	buf_len = str_len + 1;
	mkeep_alive_pkt.version = htod16(WL_MKEEP_ALIVE_VERSION);
	mkeep_alive_pkt.length = htod16(WL_MKEEP_ALIVE_FIXED_LEN);
	/* Setup keep alive zero for null packet generation */
	mkeep_alive_pkt.keep_alive_id = 0;
	mkeep_alive_pkt.len_bytes = 0;
	buf_len += WL_MKEEP_ALIVE_FIXED_LEN;
	bzero(mkeep_alive_pkt.data, sizeof(mkeep_alive_pkt.data));
	/* Keep-alive attributes are set in local	variable (mkeep_alive_pkt), and
	 * then memcpy'ed into buffer (mkeep_alive_pktp) since there is no
	 * guarantee that the buffer is properly aligned.
	 */
	memcpy((char *)mkeep_alive_pktp, &mkeep_alive_pkt, WL_MKEEP_ALIVE_FIXED_LEN);

	res = dhd_wl_ioctl_cmd(dhd, WLC_SET_VAR, buf, buf_len, TRUE, 0);

	return res;
}
#endif /* defined(KEEP_ALIVE) */

#if (defined PNO_SUPPORT) && (!defined WL_SCHED_SCAN)
/* Android ComboSCAN support */

/*
 *  data parsing from ComboScan tlv list
*/
int
wl_iw_parse_data_tlv(char** list_str, void *dst, int dst_size, const char token,
                     int input_size, int *bytes_left)
{
	char* str;
	uint16 short_temp;
	uint32 int_temp;

	if ((list_str == NULL) || (*list_str == NULL) ||(bytes_left == NULL) || (*bytes_left < 0)) {
		DHD_ERROR(("%s error paramters\n", __FUNCTION__));
		return -1;
	}
	str = *list_str;

	/* Clean all dest bytes */
	memset(dst, 0, dst_size);
	while (*bytes_left > 0) {

		if (str[0] != token) {
			DHD_TRACE(("%s NOT Type=%d get=%d left_parse=%d \n",
				__FUNCTION__, token, str[0], *bytes_left));
			return -1;
		}

		*bytes_left -= 1;
		str += 1;

		if (input_size == 1) {
			memcpy(dst, str, input_size);
		}
		else if (input_size == 2) {
			memcpy(dst, (char *)htod16(memcpy(&short_temp, str, input_size)),
				input_size);
		}
		else if (input_size == 4) {
			memcpy(dst, (char *)htod32(memcpy(&int_temp, str, input_size)),
				input_size);
		}

		*bytes_left -= input_size;
		str += input_size;
		*list_str = str;
		return 1;
	}

	return 1;
}

/*
 *  channel list parsing from cscan tlv list
*/
int
wl_iw_parse_channel_list_tlv(char** list_str, uint16* channel_list,
                             int channel_num, int *bytes_left)
{
	char* str;
	int idx = 0;

	if ((list_str == NULL) || (*list_str == NULL) ||(bytes_left == NULL) || (*bytes_left < 0)) {
		DHD_ERROR(("%s error paramters\n", __FUNCTION__));
		return -1;
	}
	str = *list_str;

	while (*bytes_left > 0) {

		if (str[0] != CSCAN_TLV_TYPE_CHANNEL_IE) {
			*list_str = str;
			DHD_TRACE(("End channel=%d left_parse=%d %d\n", idx, *bytes_left, str[0]));
			return idx;
		}
		/* Get proper CSCAN_TLV_TYPE_CHANNEL_IE */
		*bytes_left -= 1;
		str += 1;

		if (str[0] == 0) {
			/* All channels */
			channel_list[idx] = 0x0;
		}
		else {
			channel_list[idx] = (uint16)str[0];
			DHD_TRACE(("%s channel=%d \n", __FUNCTION__,  channel_list[idx]));
		}
		*bytes_left -= 1;
		str += 1;

		if (idx++ > 255) {
			DHD_ERROR(("%s Too many channels \n", __FUNCTION__));
			return -1;
		}
	}

	*list_str = str;

	return idx;
}

/*
 *  SSIDs list parsing from cscan tlv list
 */
int
wl_iw_parse_ssid_list_tlv(char** list_str, wlc_ssid_ext_t* ssid, int max, int *bytes_left)
{
	char* str;
	int idx = 0;

	if ((list_str == NULL) || (*list_str == NULL) || (*bytes_left < 0)) {
		DHD_ERROR(("%s error paramters\n", __FUNCTION__));
		return BCME_BADARG;
	}
	str = *list_str;
	while (*bytes_left > 0) {

		if (str[0] != CSCAN_TLV_TYPE_SSID_IE) {
			*list_str = str;
			DHD_TRACE(("nssid=%d left_parse=%d %d\n", idx, *bytes_left, str[0]));
			return idx;
		}

		/* Get proper CSCAN_TLV_TYPE_SSID_IE */
		*bytes_left -= 1;
		str += 1;

		if (str[0] == 0) {
			/* Broadcast SSID */
			ssid[idx].SSID_len = 0;
			memset((char*)ssid[idx].SSID, 0x0, DOT11_MAX_SSID_LEN);
			*bytes_left -= 1;
			str += 1;

			DHD_TRACE(("BROADCAST SCAN  left=%d\n", *bytes_left));
		}
		else if (str[0] <= DOT11_MAX_SSID_LEN) {
			/* Get proper SSID size */
			ssid[idx].SSID_len = str[0];
			*bytes_left -= 1;
			str += 1;

			/* Get SSID */
			if (ssid[idx].SSID_len > *bytes_left) {
				DHD_ERROR(("%s out of memory range len=%d but left=%d\n",
				__FUNCTION__, ssid[idx].SSID_len, *bytes_left));
				return BCME_BADARG;
			}

			memcpy((char*)ssid[idx].SSID, str, ssid[idx].SSID_len);

			*bytes_left -= ssid[idx].SSID_len;
			str += ssid[idx].SSID_len;
			ssid[idx].hidden = TRUE;

			DHD_TRACE(("%s :size=%d left=%d\n",
				(char*)ssid[idx].SSID, ssid[idx].SSID_len, *bytes_left));
		}
		else {
			DHD_ERROR(("### SSID size more that %d\n", str[0]));
			return BCME_BADARG;
		}

		if (++idx >=  max) {
			DHD_ERROR(("%s number of SSIDs more that %d\n", __FUNCTION__, idx));
			return BCME_BADARG;
		}
	}

	*list_str = str;

	return idx;
}

/* Parse a comma-separated list from list_str into ssid array, starting
 * at index idx.  Max specifies size of the ssid array.  Parses ssids
 * and returns updated idx; if idx >= max not all fit, the excess have
 * not been copied.  Returns -1 on empty string, or on ssid too long.
 */
int
wl_iw_parse_ssid_list(char** list_str, wlc_ssid_t* ssid, int idx, int max)
{
	char* str, *ptr;

	if ((list_str == NULL) || (*list_str == NULL))
		return -1;

	for (str = *list_str; str != NULL; str = ptr) {

		/* check for next TAG */
		if (!strncmp(str, GET_CHANNEL, strlen(GET_CHANNEL))) {
			*list_str	 = str + strlen(GET_CHANNEL);
			return idx;
		}

		if ((ptr = strchr(str, ',')) != NULL) {
			*ptr++ = '\0';
		}

		if (strlen(str) > DOT11_MAX_SSID_LEN) {
			DHD_ERROR(("ssid <%s> exceeds %d\n", str, DOT11_MAX_SSID_LEN));
			return -1;
		}

		if (strlen(str) == 0)
			ssid[idx].SSID_len = 0;

		if (idx < max) {
			bzero(ssid[idx].SSID, sizeof(ssid[idx].SSID));
			strncpy((char*)ssid[idx].SSID, str, sizeof(ssid[idx].SSID) - 1);
			ssid[idx].SSID_len = strlen(str);
		}
		idx++;
	}

	return idx;
}

/*
 * Parse channel list from iwpriv CSCAN
 */
int
wl_iw_parse_channel_list(char** list_str, uint16* channel_list, int channel_num)
{
	int num;
	int val;
	char* str;
	char* endptr = NULL;

	if ((list_str == NULL)||(*list_str == NULL))
		return -1;

	str = *list_str;
	num = 0;
	while (strncmp(str, GET_NPROBE, strlen(GET_NPROBE))) {
		val = (int)strtoul(str, &endptr, 0);
		if (endptr == str) {
			printf("could not parse channel number starting at"
				" substring \"%s\" in list:\n%s\n",
				str, *list_str);
			return -1;
		}
		str = endptr + strspn(endptr, " ,");

		if (num == channel_num) {
			DHD_ERROR(("too many channels (more than %d) in channel list:\n%s\n",
				channel_num, *list_str));
			return -1;
		}

		channel_list[num++] = (uint16)val;
	}

	*list_str = str;

	return num;
}

#endif /* (defined PNO_SUPPORT) && (!defined WL_SCHED_SCAN) */

#if defined(BCM_ROUTER_DHD) || defined(TRAFFIC_MGMT_DWM)
static int
traffic_mgmt_add_dwm_filter(dhd_pub_t *dhd,
	trf_mgmt_filter_list_t * trf_mgmt_filter_list, int len)
{
	int ret = 0;
	uint32              i;
	trf_mgmt_filter_t   *trf_mgmt_filter;
	uint8               dwm_tbl_entry;
	uint32              dscp = 0;
	uint16              dwm_filter_enabled = 0;

	/* Check parameter length is adequate */
	if (len < (OFFSETOF(trf_mgmt_filter_list_t, filter) +
		trf_mgmt_filter_list->num_filters * sizeof(trf_mgmt_filter_t))) {
		ret = BCME_BUFTOOSHORT;
		return ret;
	}

	bzero(&dhd->dhd_tm_dwm_tbl, sizeof(dhd_trf_mgmt_dwm_tbl_t));

	for (i = 0; i < trf_mgmt_filter_list->num_filters; i++) {
		trf_mgmt_filter = &trf_mgmt_filter_list->filter[i];

		dwm_filter_enabled = (trf_mgmt_filter->flags & TRF_FILTER_DWM);

		if (dwm_filter_enabled) {
			dscp = trf_mgmt_filter->dscp;
			if (dscp >= DHD_DWM_TBL_SIZE) {
				ret = BCME_BADARG;
			return ret;
			}
		}

		dhd->dhd_tm_dwm_tbl.dhd_dwm_enabled = 1;
		/* set WMM AC bits */
		dwm_tbl_entry = (uint8) trf_mgmt_filter->priority;
		DHD_TRF_MGMT_DWM_SET_FILTER(dwm_tbl_entry);

		/* set favored bits */
		if (trf_mgmt_filter->flags & TRF_FILTER_FAVORED)
			DHD_TRF_MGMT_DWM_SET_FAVORED(dwm_tbl_entry);

		dhd->dhd_tm_dwm_tbl.dhd_dwm_tbl[dscp] =  dwm_tbl_entry;
	}

	return ret;
}
#endif /* BCM_ROUTER_DHD || TRAFFIC_MGMT_DWM */

/* Given filename and download type,  returns a buffer pointer and length
 * for download to f/w. Type can be FW or NVRAM.
 *
 */
int
dhd_get_download_buffer(dhd_pub_t	*dhd, char *file_path, download_type_t component,
	char ** buffer, int *length)

{
	int ret = BCME_ERROR;
	int len = 0;
	int file_len;
	void *image = NULL;
	uint8 *buf = NULL;

	/* Point to cache if available. */
#ifdef CACHE_FW_IMAGES
	if (component == download_type_FW) {
		if (dhd->cached_fw_length) {
			len = dhd->cached_fw_length;
			buf = dhd->cached_fw;
		}
	} else if (component == download_type_NVRAM) {
		if (dhd->cached_nvram_length) {
			len = dhd->cached_nvram_length;
			buf = dhd->cached_nvram;
		}
	} else {
		return ret;
	}
#endif
	/* No Valid cache found on this call */
	if (!len) {
		file_len = *length;
		*length = 0;

		if (file_path) {
			image = dhd_os_open_image(dhd, file_path);
			if (image == NULL) {
				goto err;
			}
		}

		buf = MALLOCZ(dhd->osh, file_len);
		if (buf == NULL) {
			DHD_ERROR(("%s: Failed to allocate memory %d bytes\n",
				__FUNCTION__, file_len));
			goto err;
		}

		/* Download image */
		len = dhd_os_get_image_block(buf, file_len, image);
		if ((len <= 0 || len > file_len)) {
			MFREE(dhd->osh, buf, file_len);
			goto err;
		}
	}

	ret = BCME_OK;
	*length = len;
	*buffer = buf;

	/* Cache if first call. */
#ifdef CACHE_FW_IMAGES
	if (component == download_type_FW) {
		if (!dhd->cached_fw_length) {
			dhd->cached_fw = buf;
			dhd->cached_fw_length = len;
		}
	} else if (component == download_type_NVRAM) {
		if (!dhd->cached_nvram_length) {
			dhd->cached_nvram = buf;
			dhd->cached_nvram_length = len;
		}
	}
#endif

err:
	if (image)
		dhd_os_close_image(dhd, image);

	return ret;
}

void
dhd_free_download_buffer(dhd_pub_t	*dhd, void *buffer, int length)
{
#ifdef CACHE_FW_IMAGES
	return;
#endif
	MFREE(dhd->osh, buffer, length);
}

static int
dhd_hmembytes(dhd_pub_t *dhd, void *haddr, uint8 *data, uint size)
{
	uint dsize;
	char *buf_p;

	buf_p = (char *)haddr;
	dsize = sizeof(uint32);

	OSL_CACHE_INV((void *)buf_p, size);
	while (size) {
		if (size >= sizeof(uint32)) {
			*(uint32*)data = LTOH32(*(uint32 *)buf_p);
		} else {
			dsize = sizeof(uint8);
			*(uint32*)data = LTOH32(*(uint32 *)buf_p);
		}

		/* Adjust for next transfer (if any) */
		if ((size -= dsize) > 0) {
			data += dsize;
			buf_p += dsize;
		}
	}

	return BCME_OK;
}
