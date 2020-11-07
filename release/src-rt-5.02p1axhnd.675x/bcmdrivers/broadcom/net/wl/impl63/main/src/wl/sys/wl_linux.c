/*
 * Linux-specific portion of
 * Broadcom 802.11abg Networking Device Driver
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
 * $Id: wl_linux.c 789026 2020-07-16 18:08:38Z $
 */

/**
 * XXX Twiki [LinuxHybridDriver]
 */

#define LINUX_PORT

#define __UNDEF_NO_VERSION__

#include <typedefs.h>
#include <linuxver.h>
#include <osl.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 14)
#include <linux/module.h>
#endif // endif

#include <linux/types.h>
#include <linux/errno.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/ethtool.h>
#include <linux/completion.h>
#include <linux/random.h>
#include <linux/usb.h>
#include <bcmdevs.h>

#if defined(BCM_WFD)
#include <linux/if_vlan.h>
#endif // endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 4, 0)
#include <asm/switch_to.h>
#else
#include <asm/system.h>
#endif // endif
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/pgtable.h>
#include <asm/uaccess.h>
#include <asm/unaligned.h>

#if !defined(BCMDONGLEHOST)
/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */
#include <wlc_cfg.h>
#endif // endif

#include <802.1d.h>

#include <epivers.h>
#include <bcmendian.h>
#include <ethernet.h>
#include <bcmutils.h>
#include <pcicfg.h>
#include <pcie_core.h>
#include <wlioctl.h>
#include <wlc_keymgmt.h>
#include <wldev_common.h>
#include <hndpmu.h>

#ifndef LINUX_POSTMOGRIFY_REMOVAL
#include <wlc_channel.h>
#else
typedef const struct si_pub si_t;
#endif // endif
#include <wlc_pub.h>
#include <wlc.h>
#ifndef LINUX_POSTMOGRIFY_REMOVAL
#include <wlc_bsscfg.h>
#endif // endif

#ifdef CONFIG_COMPAT
#include <linux/compat.h>
#endif // endif

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 4, 5)
#error "No support for Kernel Rev <= 2.4.5, As the older kernel revs doesn't support Tasklets"
#endif // endif
#if defined(USE_CFG80211)
#include <net/rtnetlink.h>
#endif /* USE_CFG80211 */

#include <wl_dbg.h>
#include <monitor.h>
#ifdef WL_MONITOR
#include <bcmwifi_radiotap.h>
#include <wlc_ethereal.h>
#endif // endif
#ifdef WL_STA_MONITOR
#include <wlc_stamon.h>
#endif /* WL_STA_MONITOR */
#ifdef BCMJTAG
#include <bcmjtag.h>
#endif // endif

#include <wl_iw.h>
#ifdef USE_IW
struct iw_statistics *wl_get_wireless_stats(struct net_device *dev);
#endif // endif

#include <wl_export.h>
#ifdef TOE
#include <wl_toe.h>
#endif // endif
#ifdef ARPOE
#include <wl_arpoe.h>
#endif // endif

#ifndef LINUX_POSTMOGRIFY_REMOVAL
#if defined(WL_EVENTQ) || (defined(WL_MBO) && defined(MBO_AP))
#include <wl_eventq.h>
#endif // endif
#if defined(P2PO) || defined(ANQPO) || (defined(WL_MBO) && defined(MBO_AP))
#include <wl_gas.h>
#endif // endif
#ifdef P2PO
#include <wl_p2po_disc.h>
#include <wl_p2po.h>
#endif // endif
#ifdef ANQPO
#include <wl_anqpo.h>
#endif // endif
#ifdef BDO
#include <wl_bdo.h>
#endif // endif
#ifdef ICMP
#include <wl_icmp.h>
#endif // endif
#endif /* LINUX_POSTMOGRIFY_REMOVAL */
#if defined(WL_MBO) && defined(MBO_AP)
#include <wlc_mbo.h>
#endif /* WL_MBO && MBO_AP */
#ifdef AVS
#include <wl_avs.h>
#endif // endif

#ifdef HNDCTF
#include <ctf/hndctf.h>
#endif // endif

#if defined(PLATFORM_INTEGRATED_WIFI) && defined(CONFIG_OF)
#include <linux/platform_device.h>
#include <linux/of.h>
#endif /* PLATFORM_INTEGRATED_WIFI && CONFIG_OF */

#include <wl_linux.h>

#if defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE) || \
	defined(WLBIN_COMPAT)
#include <bcm_spdsvc.h>
#endif /* CONFIG_BCM_SPDSVC || CONFIG_BCM_SPDSVC_MODULE || WLBIN_COMPAT */

#ifdef STB_SOC_WIFI
#include <wl_stbsoc.h>
#endif /* STB_SOC_WIFI */

#ifdef WL_THREAD
#include <linux/kthread.h>
#endif /* WL_THREAD */

#if defined(USE_CFG80211)
#include <wl_cfg80211.h>
#include <wldev_common.h>
#endif /* CFG80211 */
#include <wl_core.h>

#ifdef DPSTA
#include <dpsta.h>
#ifdef PSTA
#include <wlc_psta.h>
#endif /* PSTA */
#ifdef WET
#include <wlc_wet.h>
#endif /* WET */
#endif /* DPSTA */
#if defined(WDS)
#include <wlc_wds.h>
#endif /* WDS */

#ifdef WLCSO
#include <wlc_tso.h>
#endif /* WL_CSO */

#if defined(BCM_GMAC3)
#include <hndfwd.h>
#endif // endif

#if defined(PKTC_TBL)
#include <wl_pktc.h>
#endif // endif
#if defined(BCM_PKTFWD)
#include <wl_pktfwd.h>
#endif // endif
#if defined(BCM_AWL)
#include <wl_awl.h>
#endif /* BCM_AWL */
#include <wlc_scb.h>

#if defined(BCM_BLOG)
#include <wl_blog.h>
#endif // endif

#include <wlc_objregistry.h>
#include <wlc_perf_utils.h>
#include <wlc_dump.h>
#include <wlc_iocv.h>
#include <wlc_frmutil.h>
#include <wlc_rx.h>

#ifdef WLRSDB
#include <wlc_rsdb.h>
#endif /* WLRSDB */

#if defined(BCM_WFD)
#include <wl_wfd.h>
#include <wl_thread.h>
#endif // endif

#include <wlc_macdbg.h>

#if (defined(BCA_CPEROUTER) || defined(BCA_HNDROUTER)) && (defined(CONFIG_BCM963178) || \
	defined(CONFIG_BCM947622))
#define IS_BCA_2x2AX_BUILD /**< 2x2 ax core(s) embedded in a BCA DSL or AP chip */
#include <bcm_map_part.h>
#include <bcm_intr.h>
#endif /* (BCA_CPEROUTER || BCA_HNDROUTER) && (CONFIG_BCM963178 || CONFIG_BCM947622) */

#if defined(BCM_EAPFWD)
#include <bcm_eapfwd.h>
#endif /* BCM_EAPFWD */

#if defined(CONFIG_BCM_WLAN_DGASP)
/* For dying gasp */
#include <board.h>
void wl_shutdown_handler(wl_info_t *wl);
#endif /* CONFIG_BCM_WLAN_DGASP */

#ifdef IS_BCA_2x2AX_BUILD
#if defined(CONFIG_BCM963178)
#define ARMGIC_CORE_A_M2M_IRQV _2MAP(INTERRUPT_UBUS2AXI_WLAN0_M2MDMA)
#define ARMGIC_CORE_A_D11_IRQV _2MAP(INTERRUPT_UBUS2AXI_WLAN0_D11MAC)
#define CORE_A_PHYS_ADDR   WLAN0_PHYS_BASE /** from host CPU perspective */
#define GET_2x2AX_D11_IRQV(regs) ARMGIC_CORE_A_D11_IRQV
#define GET_2x2AX_M2M_IRQV(regs) ARMGIC_CORE_A_M2M_IRQV
#elif defined(CONFIG_BCM947622)
#define ARMGIC_CORE_A_M2M_IRQV INTERRUPT_ID_WLAN0_M2MDMA
#define ARMGIC_CORE_A_D11_IRQV INTERRUPT_ID_WLAN0_D11MAC
#define ARMGIC_CORE_B_M2M_IRQV INTERRUPT_ID_WLAN1_M2MDMA
#define ARMGIC_CORE_B_D11_IRQV INTERRUPT_ID_WLAN1_D11MAC
#define CORE_A_PHYS_ADDR   WLAN0_PHYS_BASE /** from host CPU perspective */
#define CORE_B_PHYS_ADDR   WLAN1_PHYS_BASE /** from host CPU perspective */
#define GET_2x2AX_D11_IRQV(regs) \
	(((regs) == CORE_B_PHYS_ADDR) ? ARMGIC_CORE_B_D11_IRQV : ARMGIC_CORE_A_D11_IRQV)
#define GET_2x2AX_M2M_IRQV(regs) \
	(((regs) == CORE_B_PHYS_ADDR) ? ARMGIC_CORE_B_M2M_IRQV : ARMGIC_CORE_A_M2M_IRQV)
#endif /* CONFIG_BCM947622 */
#endif /* IS_BCA_2x2AX_BUILD */

/* With WFD max interfaces supported should not exceed WIFI_MW_MAX_NUM_IF */
#define WL_MAX_IFACE	(16U)

/* Bit Map if the maximum interfaces created */
#define WL_MAX_IFS_BMP (((1ULL << WL_MAX_IFACE) - 1) & (UINT32_MAX))

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
static void wl_timer(ulong data);
#else
static void wl_timer(struct timer_list *tmr);
#endif /* KERNEL_VERSION < 4.15 */
static void _wl_timer(wl_timer_t *t);
static struct net_device *wl_alloc_linux_if(wl_if_t *wlif);
#if defined(STB) && !defined(STBAP)
#ifdef LINUXSTA_PS
static void wl_bus_set_wowl(void *drvinfo, int state);
#endif /* LINUXSTA_PS */
#endif /* STB && STBAP */
#if defined(WL_CFG80211) && defined(MEDIA_CFG)
static int wl_preinit_ioctls(struct net_device *ndev);
#endif // endif
#if defined(WL_CFG80211)
static int8_t wl_get_bsscfg_role(wlc_info_t *wlc, wlc_bsscfg_t *cfg, uint8_t iftype, int8_t *role);
#endif /* WL_CFG80211 */

#ifdef WL_MONITOR
static int wl_monitor_start(struct sk_buff *skb, struct net_device *dev);
#endif // endif

void wl_start_txqwork(wl_task_t *task);
static void wl_txq_free(wl_info_t *wl);

#ifdef WL_ALL_PASSIVE
static void wl_set_multicast_list_workitem(struct work_struct *work);

static void wl_timer_task(wl_task_t *task);
void wl_dpc_rxwork(struct wl_task *task);
void wl_free_timer_freelist(wl_info_t *wl, bool force);
#endif /* WL_ALL_PASSIVE */

#if defined(USE_CFG80211)
static int wl_cfg80211_enabled(void);
#endif /* #if defined(USE_CFG80211) */

static void wl_linux_watchdog(void *ctx);
static int wl_found = 0;
static int wl_get_next_instance(void);
#if defined(CONFIG_WL_MODULE) && defined(CONFIG_BCM47XX) && defined(WLRSDB)
static int rsdb_found = 0;
#endif /* CONFIG_WL_MODULE && CONFIG_BCM47XX && WLRSDB */
static int wl_get_free_ifidx(wl_info_t * wl, int iftype, int index);
static int wl_free_ifidx(wl_info_t *wl, int ifidx);

#ifdef LINUX_CRYPTO
struct ieee80211_tkip_data {
#define TKIP_KEY_LEN 32
	u8 key[TKIP_KEY_LEN];
	int key_set;

	u32 tx_iv32;
	u16 tx_iv16;
	u16 tx_ttak[5];
	int tx_phase1_done;

	u32 rx_iv32;
	u16 rx_iv16;
	u16 rx_ttak[5];
	int rx_phase1_done;
	u32 rx_iv32_new;
	u16 rx_iv16_new;

	u32 dot11RSNAStatsTKIPReplays;
	u32 dot11RSNAStatsTKIPICVErrors;
	u32 dot11RSNAStatsTKIPLocalMICFailures;

	int key_idx;

	struct crypto_tfm *tfm_arc4;
	struct crypto_tfm *tfm_michael;

	/* scratch buffers for virt_to_page() (crypto API) */
	u8 rx_hdr[16], tx_hdr[16];
};
#endif /* LINUX_CRYPTO */

/* local prototypes */
static int wl_open(struct net_device *dev);
static int wl_close(struct net_device *dev);

#ifdef PKTC_TBL
static void wl_uninit(struct net_device *dev);
#endif /* PKTC_TBL */

#ifdef WL_THREAD
static int wl_start_wlthread(struct sk_buff *skb, struct net_device *dev);
#else
static int BCMFASTPATH wl_start(struct sk_buff *skb, struct net_device *dev);
#endif /* WL_THREAD */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
static STATS64_RETURN_TYPE wl_get_stats64(struct net_device *dev,
		struct rtnl_link_stats64 *stats);
#else
static struct net_device_stats *wl_get_stats(struct net_device *dev);
#endif /* KERNEL_VERSION >= 2.6.36 */
#if defined(USE_CFG80211)
static int
wl_cfg80211_set_mac_address(struct net_device *dev, void *addr);
#endif // endif
static int wl_set_mac_address(struct net_device *dev, void *addr);
static void wl_set_multicast_list(struct net_device *dev);
static void _wl_set_multicast_list(struct net_device *dev);
static int wl_ethtool(wl_info_t *wl, void *uaddr, wl_if_t *wlif);
#ifdef NAPI_POLL
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 30)
static int wl_poll(struct napi_struct *napi, int budget);
#else
static int wl_poll(struct net_device *dev, int *budget);
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 30) */
#else /* NAPI_POLL */
static void wl_dpc(ulong data);
#endif /* NAPI_POLL */
#if !defined(WL_USE_L34_THREAD)
static void wl_tx_tasklet(ulong data);
#endif /* !WL_USE_L34_THREAD */
static void wl_link_up(wl_info_t *wl, char * ifname);
static void wl_link_down(wl_info_t *wl, char *ifname);
#if defined(BCMSUP_PSK) && defined(STA)
static void wl_mic_error(wl_info_t *wl, wlc_bsscfg_t *cfg,
	struct ether_addr *ea, bool group, bool flush_txq);
#endif // endif
#if defined(AP) || defined(WL_MONITOR)
static int wl_schedule_task(wl_info_t *wl, void (*fn)(struct wl_task *), void *context);
#endif // endif
#ifdef WL_THREAD
static int wl_start_enqueue_wlthread(wl_info_t *wl, struct sk_buff *skb);
#endif // endif
#if defined(CONFIG_PROC_FS)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
static int wl_read_proc(struct seq_file *m, void *v);
#else /* Kernel >= 3.10.0 */
static int wl_read_proc(char *buffer, char **start, off_t offset, int length, int *eof, void *data);
#endif /* Kernel >= 3.10.0 */
#endif /* #if defined(CONFIG_PROC_FS) */
#if defined(BCMDBG)
static int wl_dump(wl_info_t *wl, struct bcmstrbuf *b);
#endif // endif
static struct wl_if *wl_alloc_if(wl_info_t *wl, int iftype, uint index, struct wlc_if* wlc_if);
static void wl_free_if(wl_info_t *wl, wl_if_t *wlif, bool rtnl_is_needed);
static void wl_get_driver_info(struct net_device *dev, struct ethtool_drvinfo *info);
#ifdef WLCSO
static int wl_set_tx_csum(struct net_device *dev, uint32 on_off);
#endif // endif

#if defined(WL_CONFIG_RFKILL)
#include <linux/rfkill.h>
static int wl_init_rfkill(wl_info_t *wl);
static void wl_uninit_rfkill(wl_info_t *wl);
static int wl_set_radio_block(void *data, bool blocked);
static void wl_report_radio_state(wl_info_t *wl);
#endif // endif

#ifdef BCMJTAG
static void *wl_jtag_probe(uint16 venid, uint16 devid, void *regsva, void *param);
static void wl_jtag_detach(void *wl);
static void wl_jtag_poll(void *wl);
#endif // endif

#if defined(PLATFORM_INTEGRATED_WIFI) && defined(CONFIG_OF)
/*
 * Platform driver type used here for integrated WIFI connected with an internal bus.
 */
static int wl_plat_drv_probe(struct platform_device *pdev);
static int wl_plat_drv_remove(struct platform_device *pdev);
static void wl_plat_drv_shutdown(struct platform_device *pdev);
static int wl_plat_drv_suspend(struct platform_device *pdev, pm_message_t state);
static int wl_plat_drv_resume(struct platform_device *pdev);
#endif /* PLATFORM_INTEGRATED_WIFI && CONFIG_OF */

MODULE_LICENSE("Proprietary");

#if defined(CONFIG_PCI) && !defined(BCMJTAG)
/**
 * struct wl_cmn_data is used for multiple NIC support (including RSDB operation). XXX RB:18960
 */
struct wl_cmn_data {
	void *objrptr;		/**< Object registry is being used to share data across 2 wlc's */
	void *oshcmnptr;	/**< Used to track memory allocations in both wlc's */
	si_t *sih;
	uint16 device;
};

static struct wl_cmn_data wlcmndata;
/* recognized PCI IDs */
static struct pci_device_id wl_id_table[] =
{
	{ PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID,
	PCI_CLASS_NETWORK_OTHER << 8, 0xffff00, (unsigned long)&wlcmndata},
	{ 0, 0, 0, 0, 0, 0, 0}
};

MODULE_DEVICE_TABLE(pci, wl_id_table);
#endif // endif

#ifdef BCMDBG
static int msglevel = 0xdeadbeef;
module_param(msglevel, int, 0);
static int msglevel2 = 0xdeadbeef;
module_param(msglevel2, int, 0);
static int phymsglevel = 0xdeadbeef;
module_param(phymsglevel, int, 0);
#endif /* BCMDBG */

#if defined(WL_ALL_PASSIVE)
/* WL Passive Mode: Enable(1)/Disable(0) */
#ifdef WLP2P
static int passivemode = 1;
module_param(passivemode, int, 0);
#else /* WLP2P */
#if defined(WL_USE_L34_THREAD)
static int passivemode = 1; /* enabled by default */
#else
static int passivemode = 0;
#endif /* WL_USE_L34_THREAD */
module_param(passivemode, int, 0);
#endif /* WLP2P */
#else
static int passivemode = 0;
module_param(passivemode, int, 0);
#endif /* defined(WL_ALL_PASSIVE) */

#ifdef CONFIG_BCM947189
/* 47189 is single core processor. So, txworkq is must to avoid soft-lockups */
static int txworkq = 1;
#else
static int txworkq = 0;
#endif // endif
module_param(txworkq, int, 0);

#define WL_TXQ_THRESH	0
int wl_txq_thresh = WL_TXQ_THRESH;
module_param(wl_txq_thresh, int, 0);

#define WL_TXQ_BOUND	256
int wl_txq_bound = WL_TXQ_BOUND;
module_param(wl_txq_bound, int, 0);

static int oneonly = 0;
module_param(oneonly, int, 0);

static int piomode = 0;
module_param(piomode, int, 0);

int instance_base = 0; /* Starting instance number */
module_param(instance_base, int, 0);

int passive_channel_skip = 0;
module_param(passive_channel_skip, int, (S_IRUSR|S_IWUSR));

#if defined(BCM_GMAC3)

/** WL_FWDER_UNIT(): Fetch the assigned fwder_unit for this radio. */
#define WL_FWDER_UNIT(wl)        ((wl)->fwder_unit)

/** Wl forwarding bypass handler attached to GMAC forwarder. */
static int wl_forward(struct fwder * fwder, struct sk_buff *skb, int skb_cnt,
		struct net_device *dev);
static int BCMFASTPATH wl_start_try(struct sk_buff *skb, struct net_device *dev,
	bool in_lock);
#endif /* BCM_GMAC3 */

static int BCMFASTPATH wl_start_int_try(wl_info_t *wl, wl_if_t *wlif, struct sk_buff *skb,
	bool lock_taken);

#ifdef DPSTA
#if defined(STA) && defined(DWDS)
static void wl_dpsta_dwds_register(wl_info_t *wl, wlc_bsscfg_t *cfg);
#endif /* STA && DWDS */
#ifdef PSTA
static void wl_dpsta_psta_register(wl_info_t *wl, wlc_bsscfg_t *cfg);
#endif // endif
#endif /* DPSTA */

#if defined(BCMQT) && defined(WLC_OFFLOADS_TXSTS)
static irqreturn_t BCMFASTPATH wl_veloce_isr(int irq, void *dev_id);
#endif /* BCMQT && WLC_OFFLOADS_TXSTS */

#if defined(BCMDBG)
static struct ether_addr local_ea;
static char *macaddr = NULL;
module_param(macaddr, charp, S_IRUGO);
#endif // endif

#if defined(BCMJTAG) || defined(BCMSLTGT)
static int nompc = 1;
#else
static int nompc = 0;
#endif // endif
module_param(nompc, int, 0);

#ifdef quote_str
#undef quote_str
#endif /* quote_str */
#ifdef to_str
#undef to_str
#endif /* quote_str */
#define to_str(s) #s
#define quote_str(s) to_str(s)

#ifndef BRCM_WLAN_IFNAME
#define BRCM_WLAN_IFNAME eth%d
#endif /* BRCM_WLAN_IFNAME */

#define SCHEDULE_WORK(wl, work)		schedule_work(work)
#define SCHEDULE_WORK_ON(wl, cpu, work)	schedule_work_on((cpu), (work))
static char intf_name[IFNAMSIZ] = quote_str(BRCM_WLAN_IFNAME);

/* allow override of default wlan interface name at insmod time */
module_param_string(intf_name, intf_name, IFNAMSIZ, 0);

/* BCMSLTGT: slow target */
#if defined(BCMJTAG) || defined(BCMSLTGT)
/* host and target have different clock speeds */
uint htclkratio = 2000;
module_param(htclkratio, int, 0);
#endif // endif

/*
 * Before CT DMA is mature enough, here ctdma is off by default, but you can pass
 * module parameter "ctdma=1" during  driver installation to turn on it.
 */
#if defined(BCM_DMA_CT) && !defined(BCM_DMA_CT_DISABLED)
static int ctdma = 0;
module_param(ctdma, int, 0);
#endif // endif

#ifndef SRCBASE
#define SRCBASE "."
#endif // endif

/* decrement struct wl_info::callbacks and assert if it drops below zero */
#define CALLBACK_DEC_AND_ASSERT(_wl) \
	do { \
		int callbacks = atomic_dec_return(&(_wl)->callbacks);		\
		if (callbacks < 0) {						\
			WL_ERROR(("wl%d: %s: callbacks dropped below zero\n",	\
				(_wl)->unit, __FUNCTION__));			\
			ASSERT(0);						\
		}								\
	} while (0)

#define FOREACH_WL(wl_cmn, idx, current_wl) \
	for ((idx) = 0; (int) (idx) < MAX_RSDB_MAC_NUM; (idx)++) \
		if ((((current_wl) = wl_cmn->wl[(idx)]) != NULL))

#if WIRELESS_EXT >= 19 || LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 29)
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 19)
static struct ethtool_ops wl_ethtool_ops =
#else
static const struct ethtool_ops wl_ethtool_ops =
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 19) */
{
	.get_drvinfo = wl_get_driver_info,
#ifdef WLCSO
	.set_tx_csum = wl_set_tx_csum
#endif // endif
};
#endif /* WIRELESS_EXT >= 19 || LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 29) */

#ifdef WL_THREAD
static int
wl_thread_dpc_wlthread(void *data)
{
	wl_info_t *wl = (wl_info_t *) data;

	current->flags |= PF_NOFREEZE;

#ifdef WL_THREADNICE
	set_user_nice(current, WL_THREADNICE);
#endif // endif

	while (1) {
		wait_event_interruptible_timeout(wl->thread_wqh,
			skb_queue_len(&wl->tx_queue),
			1);

		if (kthread_should_stop())
			break;

		wl_start_txqwork_wlthread(wl);
	}

	skb_queue_purge(&wl->tx_queue);

	return 0;
}
#endif /* WL_THREAD */

#if defined(WL_USE_NETDEV_OPS)
/* Physical interface netdev ops */
static const struct net_device_ops wl_netdev_ops =
{
	.ndo_open = wl_open,
	.ndo_stop = wl_close,
#ifdef PKTC_TBL
	.ndo_uninit = wl_uninit,
#endif // endif
#ifdef WL_THREAD
	.ndo_start_xmit = wl_start_wlthread,
#else
	.ndo_start_xmit = wl_start,
#endif // endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
	.ndo_get_stats64 = wl_get_stats64,
#else
	.ndo_get_stats = wl_get_stats,
#endif /* KERNEL_VERSION >= 2.6.36 */
#if defined(USE_CFG80211)
	.ndo_set_mac_address = wl_cfg80211_set_mac_address,
#else
	.ndo_set_mac_address = wl_set_mac_address,
#endif // endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 2, 0)
	.ndo_set_rx_mode = wl_set_multicast_list,
#else
	.ndo_set_multicast_list = wl_set_multicast_list,
#endif // endif
	.ndo_do_ioctl = wl_ioctl
};

#ifdef WL_MONITOR
static const struct net_device_ops wl_netdev_monitor_ops =
{
	.ndo_start_xmit = wl_monitor_start,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
	.ndo_get_stats64 = wl_get_stats64,
#else
	.ndo_get_stats = wl_get_stats,
#endif /* KERNEL_VERSION >= 2.6.36 */
	.ndo_do_ioctl = wl_ioctl
};
#endif /* WL_MONITOR */
#endif /* WL_USE_NETDEV_OPS */

#if defined(PLATFORM_WITH_RUNNER) && defined(BCM_BLOG) && defined(CONFIG_BCM_KF_BLOG)
static int
wl_client_get_info(struct net_device *net, char *mac,
	int priority, wlan_client_info_t *info_p);
#endif /* PLATFORM_WITH_RUNNER && BCM_BLOG && CONFIG_BCM_KF_BLOG */

static void
wl_if_setup(struct net_device *dev)
{
#if defined(WL_USE_NETDEV_OPS)
	dev->netdev_ops = &wl_netdev_ops;
#else
	dev->open = wl_open;
	dev->stop = wl_close;
#ifdef WL_THREAD
	dev->hard_start_xmit = wl_start_wlthread;
#else
	dev->hard_start_xmit = wl_start;
#endif // endif
	dev->get_stats = wl_get_stats;
#if defined(USE_CFG80211)
	dev->set_mac_address = wl_cfg80211_set_mac_address;
#else
	dev->set_mac_address = wl_set_mac_address;
#endif // endif
	dev->set_multicast_list = wl_set_multicast_list;
	dev->do_ioctl = wl_ioctl;
#endif /* WL_USE_NETDEV_OPS */

#if defined(BCM_BLOG) && defined(CONFIG_BCM_KF_BLOG)
#if defined(PLATFORM_WITH_RUNNER)
	dev->wlan_client_get_info = wl_client_get_info;
#else
	dev->wlan_client_get_info = NULL;
#endif /* PLATFORM_WITH_RUNNER */
#endif /* BCM_BLOG && CONFIG_BCM_KF_BLOG */

#ifdef NAPI_POLL
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 30)
	{
		struct wl_info *wl = WL_INFO_GET(dev);

		netif_napi_add(dev, &wl->napi, wl_poll, 64);
		napi_enable(&wl->napi);
	}
#else
	dev->poll = wl_poll;
	dev->weight = 64;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 21)
	netif_poll_enable(dev);
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 21) */
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 30) */
#endif /* NAPI_POLL */

#ifdef USE_IW
#if WIRELESS_EXT < 19
	dev->get_wireless_stats = wl_get_wireless_stats;
#endif // endif
#if WIRELESS_EXT > 12
	dev->wireless_handlers = (const struct iw_handler_def *) &wl_iw_handler_def;
#endif // endif
#endif /* USE_IW */

#if WIRELESS_EXT >= 19 || LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 29)
	dev->ethtool_ops = &wl_ethtool_ops;
#endif // endif

#if defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE) || \
	defined(WLBIN_COMPAT)
	wl_spdsvc_init();
#endif /* CONFIG_BCM_SPDSVC || CONFIG_BCM_SPDSVC_MODULE || WLBIN_COMPAT */

} /* wl_if_setup */

#ifdef HNDCTF

static void
wl_ctf_detach(ctf_t *ci, void *arg)
{
	wl_info_t *wl = (wl_info_t *)arg;

	wl->cih = NULL;

#ifdef CTFPOOL
	/* free the buffers in fast pool */
	osl_ctfpool_cleanup(wl->osh);
#endif /* CTFPOOL */

	return;
}

#if defined(BCMDBG)
static int
wl_dump_ctf(wl_info_t *wl, struct bcmstrbuf *b)
{
	ctf_dump(wl->cih, b);
	return 0;
}
#endif // endif
#endif /* HNDCTF */

#if defined(CONFIG_PROC_FS)
/* create_proc_read_entry() removed in linux 3.10.0, use proc_create_data() instead. */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
#include <linux/seq_file.h>

static int wl_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, wl_read_proc, PDE_DATA(inode));
}

static const struct file_operations wl_proc_fops = {
	.open           = wl_proc_open,
	.read           = seq_read,
	.llseek         = seq_lseek,
	.release        = seq_release,
};
#endif /* #if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0)) */
#endif /* #if defined(CONFIG_PROC_FS) */

/* Below define is only for router platform */
#if defined(CONFIG_WL_MODULE) && defined(CONFIG_BCM47XX)
#ifdef WLRSDB
/**
 * Check if a register_netdev should be blocked or not.
 *
 * This function is currently enabled only for router platform.
 * In case of RSDB chips, if the current operating mode is not RSDB, then the second interface
 * should not be registered. If the second interface is created in MIMO, other router applications,
 * which are dependent on the interfaces created will behave erratically.
 *
 * This function is kept generic, so that it can be extended if required.
 *
 * Returns 0 if the interface should NOT be blocked
 * Returns 1 if the interface should be blocked
 */
static int
wl_rsdb_block_netdev(struct wl_cmn_data *cmndata, uint unit)
{
	const char *var;

	// Always allow first unit to register
	if (unit == 0) {
		return 0;
	}

	// Allow Non-RSDB Chips to register always
	if ((!cmndata->sih) || (si_numd11coreunits(cmndata->sih) <= 1)) {
		return 0;
	}

	var = getvar(NULL, "rsdb_mode");

	/* If "rsdb_mode" not defined, default is mimo mode. mimo mode combines
	 * both d11 cores so do not create a network interface for the second d11.
	 */
	if (!var) {
		return 1;
	}

	/* If "rsdb_mode" is mimo or 80p80, it combines both d11 cores so do not create
	   a network interface for the second d11.
	 */
	if ((bcm_atoi(var) == WLC_RSDB_MODE_2X2) ||
			(bcm_atoi(var) == WLC_RSDB_MODE_80P80)) {
		return 1;
	}

	return 0;
}
#endif /* WLRSDB */
#endif /* CONFIG_WL_MODULE && CONFIG_BCM47XX */

/**
 * Register read.
 *
 * Allow register operations from external context, synchronized using perimeter lock.
 *
 * @param[in] context	wl context as opaque pointer
 * @param[in] address	Physical register address
 * @return		Register value
 */

uint32
wl_backplane_read(void *context, uint32 address)
{
	wl_info_t *wl = (wl_info_t*)context;
	uint32 value = 0;

	ASSERT(wl);

	WL_LOCK(wl);
	si_backplane_access(wl->pub->sih, address, sizeof(uint32), (uint*)&value, TRUE);
	WL_UNLOCK(wl);

	return value;
}

/**
 * Register write.
 *
 * Allow register operations from external context, synchronized using perimeter lock.
 *
 * @param[in] context	wl context as opaque pointer
 * @param[in] address	Physical register address
 * @param[in] value	Register value
 */

void
wl_backplane_write(void *context, uint32 address, uint32 value)
{
	wl_info_t *wl = (wl_info_t*)context;

	ASSERT(wl);

	WL_LOCK(wl);
	si_backplane_access(wl->pub->sih, address, sizeof(uint32), (uint*)&value, FALSE);
	WL_UNLOCK(wl);
}

/**
 * PMU register read/write.
 *
 * Allow register operations from external context, synchronized using perimeter lock.
 *
 * @param[in] context	wl context as opaque pointer
 * @param[in] address	PMU register address
 * @param[in] mask	Mask.
 * @param[in] value	Register value
 * @return		Read value for read operations (mask == value == 0)
 */

uint32
wl_pmu_access(void *context, uint32 address, uint32 mask, uint32 value)
{
	wl_info_t *wl = (wl_info_t*)context;
	uint32 ret;

	ASSERT(wl);

	WL_LOCK(wl);
	ret = si_pmu_chipcontrol(wl->pub->sih, address, mask, value);
	WL_UNLOCK(wl);

	return ret;
}

#if defined(PLATFORM_WITH_RUNNER) && defined(BCM_BLOG) && defined(CONFIG_BCM_KF_BLOG)
static int
wl_client_get_info(struct net_device *net, char *mac, int priority, wlan_client_info_t *info_p)
{
#if defined(BCM_WFD)
	wl_if_t *wlif = *(wl_if_t **)netdev_priv(net);
	wl_info_t *wl = wlif->wl;
	info_p->type = WLAN_CLIENT_TYPE_WFD;
	info_p->wfd.mcast.is_tx_hw_acc_en = 1;
	info_p->wfd.mcast.is_wfd = 1;
	info_p->wfd.mcast.is_chain = 0;
	info_p->wfd.mcast.wfd_idx = wl->wfd_idx;
	info_p->wfd.mcast.wfd_prio = 1 ; /* put multicast onto high priority queue. */
	info_p->wfd.mcast.ssid = wlif->subunit;
#else /* ! BCM_WFD */
	info_p->type = WLAN_CLIENT_TYPE_CPU;
#endif /* ! BCM_WFD */
	return WLAN_CLIENT_INFO_OK;
}
#endif /* PLATFORM_WITH_RUNNER && BCM_BLOG && CONFIG_BCM_KF_BLOG */

/**
 * Get the next instance number used as unit#
 *
 * skip all powered down wlan interfaces during calculation of next instance
 * if wlan deep power down feature is enabled. Called only once per radio
 *
 * @params none.
 *
 * return next unit# to be used
 */
static int
wl_get_next_instance(void)
{
	int inst = wl_found + instance_base;

#if defined(CONFIG_BCM_WLAN_DPDCTL)
	static int inst_skipped = 0;
	char varstr[32];
	char *var;
	int pwrdn;

	var = getvar(NULL, "wl_dpdctl_enable");
	if (var && bcm_strtoul(var, NULL, 0) == 1) {
	    inst += inst_skipped;
	    do {
	        pwrdn = 0;
	        snprintf(varstr, sizeof(varstr), "wl%d_dpd", inst);
	        var = getvar(NULL, varstr);
	        if (var) {
	            /* Get the nvram setting */
	            pwrdn = (uint32)bcm_strtoul(var, NULL, 0);
	        }

	        if (pwrdn == 1) {
	            /* Skip this instance */
	            inst++;
	            inst_skipped++;
	        }
	    } while (pwrdn == 1);
	}
#endif /* CONFIG_BCM_WLAN_DPDCTL */

	return inst;
}

/**
 * attach to the WL device.
 *
 * Attach to the WL device identified by vendor and device parameters.
 * regs is a host accessible memory address pointing to WL device registers.
 *
 * wl_attach is not defined as static because in the case where no bus
 * is defined, wl_attach will never be called, and thus, gcc will issue
 * a warning that this function is defined but not used if we declare
 * it as static.
 *
 * @param[in] device  E.g. the PCIe device id
 * @param[in] regs    Physical host address to chipcommon core (using the bar0 window)
 * @param[in] btparam Opaque pointer, e.g. to Linux 'struct pci_dev'
 */
static wl_info_t *
wl_attach(uint16 vendor, uint16 device, ulong regs,
	uint bustype, void *btparam, uint irq, uchar* bar1_addr, uint32 bar1_size,
	uchar* bar2_addr, uint32 bar2_size, void *cmndata)
{
	struct net_device *dev;
	wl_if_t *wlif;
	wl_info_t *wl;
#if defined(CONFIG_PROC_FS)
	char tmp[128];
#endif // endif
	osl_t *osh;
	int unit, err;
#ifdef WLRSDB
	unsigned int pci_barwin_sz = PCI_BAR0_WINSZ * 2;
#else
	unsigned int pci_barwin_sz = PCIE2_BAR0_WINSZ;
#endif // endif
#ifdef HNDCTF
	char ctf_name[IFNAMSIZ];
#endif /* HNDCTF */
#ifdef WL_OBJ_REGISTRY
	uint numcores = 0;
	int wl_unit = 0;
#endif /* WL_OBJ_REGISTRY */
#if defined(USE_CFG80211)
	struct device *parentdev;
#endif // endif
#ifdef CTFPOOL
	int32 ctfpoolsz;
#endif // endif
	struct wl_cmn_data *commmondata = (struct wl_cmn_data *)cmndata; /**< for multiple NIC */
	int primary_idx = 0;
	uint online_cpus, iomode = 0;
	unit = wl_get_next_instance();
	err = 0;

	if (device == EMBEDDED_2x2AX_ID) { /**< PCIe device id for 63178 802.11ax dualband device */
		/* The d11 ratelink memory lives at a backplane address offset of about 9MB from
		 * the d11 core. 16MB BAR0 window to be able to address that.
		 */
		pci_barwin_sz = 16 * 1024 * 1024;
	}

	if (unit < 0) {
		WL_ERROR(("wl%d: unit number overflow, exiting\n", unit));
		return NULL;
	}

	if (oneonly && (unit != instance_base)) {
		WL_ERROR(("wl%d: wl_attach: oneonly is set, exiting\n", unit));
		return NULL;
	}

	/* Requires pkttag feature */
#ifdef SHARED_OSL_CMN
	/* Use single osh->cmn to keep track of memory usage and allocations. */
	osh = osl_attach(btparam, bustype, TRUE, &commmondata->oshcmnptr);
#else
	BCM_REFERENCE(commmondata);
	osh = osl_attach(btparam, bustype, TRUE);
#endif /* SHARED_OSL_CMN */

	ASSERT(osh);

	/* Set ACP coherence flag */
	if (OSL_ACP_WAR_ENAB() || OSL_ARCH_IS_COHERENT())
		osl_flag_set(osh, OSL_ACP_COHERENCE);

	/* allocate private info */
	if ((wl = (wl_info_t*) MALLOCZ(osh, sizeof(wl_info_t))) == NULL) {
		WL_ERROR(("wl%d: malloc wl_info_t, out of memory, malloced %d bytes\n", unit,
			MALLOCED(osh)));
		osl_detach(osh);
		return NULL;
	}

	wl->osh = osh;
	wl->unit = unit;
	wl->dev_id = device;
	atomic_set(&wl->callbacks, 0);
#ifdef BCM_WFD
	wl->wfd_idx = -1;
#endif /* BCM_WFD */

#if defined(BCM_GMAC3)
	/* Assign a fwder_unit based on probe unit and fwder cpumap nvram. */
	WL_FWDER_UNIT(wl) = fwder_assign(FWDER_NIC_MODE, unit);
	if (WL_FWDER_UNIT(wl) == FWDER_FAILURE) {
		WL_ERROR(("wl%d: %s: fwder_assign<%d> failed!\n",
		          unit, __FUNCTION__, unit));
		goto fail;
	}

	/*
	 * Primary interface:
	 * Attach my transmit handler to DNSTREAM fwder instance on core=unit
	 *      et::GMAC# -> et_sendup/chain -> wl_forward -> wl# MAC
	 * and get the UPSTREAM direction transmit handler for use in sendup.
	 *      wl# MAC -> wl_sendup -> wl->fwdh->bypass_func=et_forward-> et_start
	 * NOTE: wl->fwdh is the UPSTREAM direction forwarder!
	 */
	wl->fwdh = fwder_attach(FWDER_DNSTREAM, WL_FWDER_UNIT(wl), FWDER_NIC_MODE,
	                        wl_forward, (struct net_device *)NULL, wl->osh);
#endif /* BCM_GMAC3 */

#ifdef CONFIG_SMP
	/* initialize number of online cpus */
	online_cpus = num_online_cpus();
#if defined(BCM47XX_CA9) || defined(BCA_HNDROUTER)
	if (online_cpus > 1) {
		if (wl_txq_thresh == 0)
			wl_txq_thresh = 512;
	}
	if (wl_txq_bound == 0)
	{
		wl_txq_bound = WL_TXQ_BOUND;
		WL_PRINT(("%s: wl_txq_bound set to 0, overriding to 0x%x\n",
			__FUNCTION__, wl_txq_bound));
	}
#endif /* BCM47XX_CA9 */
#else
	online_cpus = 1;
#endif /* CONFIG_SMP */
	wl->max_cpu_id = online_cpus - 1;

	WL_ERROR(("wl%d: online cpus %d\n", unit, online_cpus));

#ifdef WL_ALL_PASSIVE
	wl->all_dispatch_mode = (passivemode == 0) ? TRUE : FALSE;
	if (WL_ALL_PASSIVE_ENAB(wl)) {
#if !defined(WL_USE_L34_THREAD)
		/* init tx work queue for wl_start/send pkt; no need to destroy workitem  */
		MY_INIT_WORK(&wl->txq_task.work, (work_func_t)wl_start_txqwork);
#endif /* !WL_USE_L34_THREAD */

		wl->txq_task.context = wl;

		/* init work queue for wl_set_multicast_list(); no need to destroy workitem  */
		MY_INIT_WORK(&wl->multicast_task.work, (work_func_t)wl_set_multicast_list_workitem);

#if !defined(WL_USE_L34_THREAD)
		MY_INIT_WORK(&wl->wl_dpc_task.work, (work_func_t)wl_dpc_rxwork);
#endif /* !WL_USE_L34_THREAD */
		wl->wl_dpc_task.context = wl;
	} else if (txworkq) {
#if !defined(WL_USE_L34_THREAD)
		/* init tx work queue for wl_start/send pkt; no need to destroy workitem  */
		MY_INIT_WORK(&wl->txq_task.work, (work_func_t)wl_start_txqwork);
#endif /* !WL_USE_L34_THREAD */

		wl->txq_task.context = wl;
	}
#endif /* WL_ALL_PASSIVE */

#if defined(WL_USE_L34_THREAD)
	if (wl_thread_attach(wl) != 0) {
		WL_ERROR(("wl%d: %s: wl_thread_attach failed\n", unit, __FUNCTION__));
		goto fail;
	}
#endif /* WL_USE_L34_THREAD */

	wl->txq_dispatched = FALSE;
	wl->txq_head = wl->txq_tail = NULL;
	wl->txq_cnt = 0;

	WL_TRACE(("wl%d: Bus: ", unit));
	if (bustype == PCMCIA_BUS) {
		/* Disregard command overwrite */
		wl->piomode = TRUE;
		WL_TRACE(("PCMCIA\n"));
	} else if (bustype == PCI_BUS) {
		/* piomode can be overwritten by command argument */
		wl->piomode = piomode;
		WL_TRACE(("PCI/%s\n", wl->piomode ? "PIO" : "DMA"));
	}
#ifdef BCMJTAG
	else if (bustype == JTAG_BUS) {
		/* Disregard command option overwrite */
		wl->piomode = TRUE;
		WL_TRACE(("JTAG\n"));
	}
#endif /* BCMJTAG */
	else if (bustype == SI_BUS) {
		/* Do nothing */
	}
	else {
		bustype = PCI_BUS;
		WL_TRACE(("force to PCI\n"));
	}
	wl->bcm_bustype = bustype;

#ifdef BCMJTAG
	if (wl->bcm_bustype == JTAG_BUS)
		wl->regsva = (void *)regs;
	else
#endif // endif
	if ((wl->regsva = ioremap_nocache(regs, pci_barwin_sz)) == NULL) {
		WL_ERROR(("wl%d: ioremap() failed\n", unit));
		goto fail;
	}

#if defined(WLVASIP)
	wl->bar1_addr = bar1_addr;
	wl->bar1_size = bar1_size;
	wl->bar2_addr = bar2_addr;
	wl->bar2_size = bar2_size;
#endif // endif

	spin_lock_init(&wl->lock);
	spin_lock_init(&wl->isr_lock);

	if (WL_ALL_PASSIVE_ENAB(wl))
		sema_init(&wl->sem, 1);

	spin_lock_init(&wl->txq_lock);

#ifdef WL_OBJ_REGISTRY
	if (commmondata->sih) {
		numcores = si_numd11coreunits(commmondata->sih);
	}

	/* Note : For RSDB chips like 53573, the wl_attach is called two times for each core.
	   During the first call, commondata->sih will be null, and numcores will be 0.
	   For non RSDB chips also numcores will be zero at this point.
	   So, the following condition is satisfied for
	    - Non RSDB chips
	    - RSDB chips first instance
	   The else case will be satisfied for RSDB chip 2nd instance, where it has to share the
	   objregistry from the first core. This check also ensures that obj registry is not shared
	   between 2 different chips. Ex: 4360+4335 in router platform.
	*/
	if ((numcores <= 1) || (commmondata->device != device)) {
		wl->objr = obj_registry_alloc(osh, OBJR_MAX_KEYS);
		commmondata->objrptr = wl->objr;
		obj_registry_set(wl->objr, OBJR_SELF, wl->objr);
	} else {
		wl->objr = (wlc_obj_registry_t *)commmondata->objrptr;
	}

	obj_registry_ref(wl->objr, OBJR_SELF);

	wl->cmn = (wl_cmn_info_t *)obj_registry_get(wl->objr, OBJR_WL_CMN_INFO);
	if (wl->cmn == NULL) {
		if ((wl->cmn = (wl_cmn_info_t *)MALLOCZ(osh, sizeof(wl_cmn_info_t))) == NULL) {
			goto fail;
		}
		obj_registry_set(wl->objr, OBJR_WL_CMN_INFO, wl->cmn);
	}
	obj_registry_ref(wl->objr, OBJR_WL_CMN_INFO);
	wl_unit = 0;

	/* Find the wl index by identifying the first non-null location of the cmn->wl array. */
	while (wl->cmn->wl[wl_unit]) {
		wl_unit++;
	}

	/* Keep a copy of all wl pointers in the wl->cmn structure */
	wl->cmn->wl[wl_unit] = wl;

#endif /* WL_OBJ_REGISTRY */

#ifdef HNDCTF
	(void)snprintf(ctf_name, sizeof(ctf_name), "wl%d", unit);
	wl->cih = ctf_attach(osh, ctf_name, &wl_msg_level, wl_ctf_detach, wl);
#endif /* HNDCTF */

	if (wl->piomode)
		iomode = IOMODE_TYPE_PIO;
#if defined(BCM_DMA_CT) && !defined(BCM_DMA_CT_DISABLED)
	else if (ctdma)
		iomode = IOMODE_TYPE_CTDMA;
#endif // endif
#if (defined(STB) || defined(STBAP))
	OSL_PCIE_ASPM_DISABLE(osh, PCIECFGREG_LINK_STATUS_CTRL);
#endif	/* defined(STB) || defined(STBAP) */
	/* common load-time initialization */
	if (!(wl->wlc = wlc_attach((void *) wl, vendor, device, unit, iomode,
		osh, wl->regsva, wl->bcm_bustype, btparam, wl->objr, &err))) {
		printf("wl driver %s failed with code %d\n", EPI_VERSION_STR, err);
		goto fail;
	}

	wl->pub = wlc_pub(wl->wlc);

	/* Allocate a wlif */
	wlif = wl_alloc_if(wl, WL_IFTYPE_BSS, 0, NULL);

	if (!wlif) {
		WL_ERROR(("wl%d: %s: wl_alloc_if failed\n", unit, __FUNCTION__));
		goto fail;
	}

	/* Allocate netdev and sets wlif->dev & dev->local->wlif */
	if (wl_alloc_linux_if(wlif) == 0) {
		WL_ERROR(("wl%d: %s: wl_alloc_linux_if failed\n", unit, __FUNCTION__));
		goto fail;
	}

	dev = wlif->dev;
	wl->dev = dev;
	wl_if_setup(dev);

	/* map chip registers (47xx: and sprom) */
	dev->base_addr = regs;

	if (BCM4365_CHIP(wl->pub->sih->chip)) {
		if (wl->pub->sih->buscoretype == PCIE2_CORE_ID)
			OSL_PCIE_MPS_LIMIT(osh, PCIECFGREG_DEVCONTROL, 256);
		else if (wl->pub->sih->buscoretype == PCIE_CORE_ID)
			OSL_PCIE_MPS_LIMIT(osh, PCI_CFG_DEVCTRL, 256);
	}

	/* Some platforms do not pass commondata. Need to check before we access it. */
	if (commmondata != NULL) {
		commmondata->device = device;
		commmondata->sih =  wl->pub->sih;
	}

	/* Populate wlcif of the primary interface in wlif */
	primary_idx = WLC_BSSCFG_IDX(wlc_bsscfg_primary(wl->wlc));
	wlif->wlcif = wlc_wlcif_get_by_index(wl->wlc, primary_idx);

	if (nompc) {
		if (wlc_iovar_setint(wl->wlc, "mpc", 0)) {
			WL_ERROR(("wl%d: Error setting MPC variable to 0\n", unit));
		}
	}

#if defined(CONFIG_PROC_FS)
	/* create /proc/net/wl<unit> */
	(void)snprintf(tmp, sizeof(tmp), "net/wl%d", wl->pub->unit);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
	/* create_proc_read_entry removed in linux 3.10.0, use proc_create_data() instead. */
	wl->proc_entry = proc_create_data(tmp, S_IRUGO, NULL, &wl_proc_fops, (void*)wl);
#else
	wl->proc_entry = create_proc_read_entry(tmp, 0, 0, wl_read_proc, (void*)wl);
#endif /* #if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0)) */
	if (wl->proc_entry == NULL) {
		WL_ERROR(("wl%d: Create proc entry '%s' failed\n", unit, tmp));
		goto fail;
	}
	WL_INFORM(("wl%d: Created the proc entry %s \n", unit, tmp));
#endif /* #if defined(CONFIG_PROC_FS) */

#ifdef BCMDBG
	if (macaddr != NULL) { /* user command line override */
		int dbg_err;

		WL_ERROR(("wl%d: setting MAC ADDRESS %s\n", unit, macaddr));
		bcm_ether_atoe(macaddr, &local_ea);

		dbg_err = wlc_iovar_op(wl->wlc, "cur_etheraddr", NULL, 0, &local_ea,
			ETHER_ADDR_LEN, IOV_SET, NULL);
		if (dbg_err)
			WL_ERROR(("wl%d: Error setting MAC ADDRESS\n", unit));
	}
#endif /* BCMDBG */
	bcopy(&wl->pub->cur_etheraddr, dev->dev_addr, ETHER_ADDR_LEN);

#ifndef NAPI_POLL
#if !defined(WL_USE_L34_THREAD)
	/* setup the bottom half handler */
	tasklet_init(&wl->tasklet, wl_dpc, (ulong)wl);
#endif /* !WL_USE_L34_THREAD */
#endif /* NAPI_POLL */

#if !defined(WL_USE_L34_THREAD)
	tasklet_init(&wl->tx_tasklet, wl_tx_tasklet, (ulong)wl);
#endif /* !WL_USE_L34_THREAD */

#if defined(PKTC_TBL)
	/* BCM_PKTFWD: wl_pktfwd_radio_ins(wl) to insert wl radio into pktfwd */
	if ((wl->pktc_tbl = wl_pktc_attach(wl, NULL)) == NULL) {
		WL_ERROR(("wl%d: wl_pktc_attach failed\n", unit));
		goto fail;
	}
	wl->pub->pktc_tbl = wl->pktc_tbl;
#endif /* PKTC_TBL */

#if defined(BCM_AWL)
	if ((wl->pub->awl_cb = wl_awl_attach(wl, unit)) == NULL) {
		WL_ERROR(("wl%d: wl_awl_attach failed\n", unit));
		goto fail;
	}
#endif /* BCM_AWL */

#ifdef TOE
	/* allocate the toe info struct */
	if ((wl->toei = wl_toe_attach(wl->wlc)) == NULL) {
		WL_ERROR(("wl%d: wl_toe_attach failed\n", unit));
		goto fail;
	}
#endif // endif

#ifdef ARPOE
	/* allocate the arp info struct */
	if ((wl->arpi = wl_arp_attach(wl->wlc)) == NULL) {
		WL_ERROR(("wl%d: wl_arp_attach failed\n", unit));
		goto fail;
	}
#endif // endif

#ifndef LINUX_POSTMOGRIFY_REMOVAL
#if defined(WL_EVENTQ) || (defined(WL_MBO) && defined(MBO_AP))
	/* allocate wl_eventq info struct */
	if ((wl->wlevtq = wl_eventq_attach(wl->wlc)) == NULL) {
		WL_ERROR(("wl%d: wl_eventq_attach failed\n", unit));
		goto fail;
	}
#endif /* WL_EVENTQ && WL_MBO && MBO_AP */

#if defined(P2PO) || defined(ANQPO) || (defined(WL_MBO) && defined(MBO_AP))
	/* allocate gas info struct */
	if ((wl->gas = wl_gas_attach(wl->wlc, wl->wlevtq)) == NULL) {
		WL_ERROR(("wl%d: wl_gas_attach failed\n", unit));
		goto fail;
	}
#endif /* P2PO || ANQPO || (WL_MBO && MBO_AP) */

#if defined(P2PO)
	/* allocate the disc info struct */
	if ((wl->disc = wl_disc_attach(wl->wlc)) == NULL) {
		WL_ERROR(("wl%d: wl_disc_attach failed\n", unit));
		goto fail;
	}

	/* allocate the p2po info struct */
	if ((wl->p2po = wl_p2po_attach(wl->wlc, wl->wlevtq, wl->gas, wl->disc)) == NULL) {
		WL_ERROR(("wl%d: wl_p2po_attach failed\n", unit));
		goto fail;
	}
#endif /* P2PO */

#if defined(ANQPO)
	/* allocate the anqpo info struct */
	if ((wl->anqpo = wl_anqpo_attach(wl->wlc, wl->gas)) == NULL) {
		WL_ERROR(("wl%d: wl_anqpo_attach failed\n", unit));
		goto fail;
	}
#endif /* ANQPO */

#if defined(BDO)
	/* allocate the bdo info struct */
	if ((wl->bdo = wl_bdo_attach(wl->wlc)) == NULL) {
		WL_ERROR(("wl%d: wl_bdo_attach failed\n", unit));
		goto fail;
	}
#endif /* defined(BDO) */
#endif /* LINUX_POSTMOGRIFY_REMOVAL */

#ifdef HNDCTF
#if defined(BCMDBG)
	wlc_dump_register(wl->pub, "ctf", (dump_fn_t)wl_dump_ctf, (void *)wl);
#endif // endif
#endif /* HNDCTF */

#ifdef CTFPOOL
	/* create packet pool with specified number of buffers */
	ctfpoolsz = (IS_AC2_DEV(device) ||
		IS_DEV_AC3X3(device) || IS_DEV_AC2X2(device)) ? CTFPOOLSZ_AC3X3 : CTFPOOLSZ;
	if (CTF_ENAB(wl->cih) && (num_physpages >= 8192) &&
	    (osl_ctfpool_init(osh, ctfpoolsz, PKTBUFSZ+BCMEXTRAHDROOM) < 0))
	{
		WL_ERROR(("wl%d: wlc_attach: osl_ctfpool_init failed\n", unit));
		goto fail;
	}
#endif /* CTFPOOL */

#if defined(BCM_WFD)
#if defined(BCM_PKTFWD)
	wl->wfd_idx = wl_wfd_bind(wl);
#else
	wl->wfd_idx = wl_wfd_bind(dev, wl->unit);
#endif // endif
	if (wl->wfd_idx < 0)
		goto fail;
#endif /* BCM_WFD */

#if defined(BCM_EAPFWD) && defined(BCM_PKTFWD)
	wl_eap_bind(
		wl, wl->dev, wl->unit,
		wl_pktfwd_lut(),
		(pktlist_context_t *)wl_pktfwd_request(wl_pktfwd_req_pktlist_e, wl->unit, 0, 0),
		(HOOKP)wl_pktfwd_xfer_callback);
#endif /* BCM_EAPFWD && BCM_PKTFWD */

	/* register our interrupt handler */
#ifdef BCMJTAG
	if (wl->bcm_bustype != JTAG_BUS)
#endif /* BCMJTAG */
	{
		int r = -1; /** Linux return value */
		char *irqname = dev->name;

#if defined(CONFIG_BCM_WLAN_DPDCTL)
		/* bustype = PCI, even embedded 2x2AX devices have virtual pci underneeth */
		snprintf(wl->pciname, sizeof(wl->pciname), "wlpcie:%s, wl%d",
			pci_name(btparam), wl->unit);
		irqname = wl->pciname;
#endif /* !CONFIG_BCM_WLAN_DPDCTL */

		if (device == EMBEDDED_2x2AX_ID) {
#ifdef IS_BCA_2x2AX_BUILD
			/* request two non-shared interrupt lines */
			irq = GET_2x2AX_D11_IRQV(regs);
			r = request_irq(irq, wl_isr, 0, irqname, wl);
#ifdef WLC_OFFLOADS_TXSTS /* M2M/BME core transfers txstatus from d11 core into memory \
	*/
			if (r == 0) {
			    r = request_irq(GET_2x2AX_M2M_IRQV(regs), wl_isr, 0, irqname, wl);
			}
#endif /* WLC_OFFLOADS_TXSTS */
#elif defined(BCMQT) && defined(WLC_OFFLOADS_TXSTS)
			r = request_irq(irq, wl_veloce_isr, IRQF_SHARED, irqname, wl);
#endif /* BCMQT && WLC_OFFLOADS_TXSTS */
		} else	{
			r = request_irq(irq, wl_isr, IRQF_SHARED, irqname, wl);
		}

		if (r != 0) {
			WL_ERROR(("wl%d: request_irq() failed\n", unit));
			goto fail;
		}

		dev->irq = irq;

#if defined(BCM_GMAC3)
		/* Setup radios irq smp affinity: Interrupt is in UPSTREAM direction. */
		fwder_affinity(FWDER_UPSTREAM, WL_FWDER_UNIT(wl), irq);
#endif /* BCM_GMAC3 */
	}

#if defined(USE_IW)
	WL_ERROR(("Using Wireless Extension\n"));
#endif // endif

#if defined(WL_CFG80211)
#if defined(MEDIA_CFG)
	wl_preinit_ioctls(dev);
#endif /* MEDIA_CFG */
	parentdev = NULL;
	if (wl->bcm_bustype == PCI_BUS) {
		parentdev = &((struct pci_dev *)btparam)->dev;
	}
	if (parentdev) {
		wl_cfg80211_set_parent_dev(parentdev);
		err = wl_cfg80211_enabled();
		if (err == BCME_OK) {
			if (wl_cfg80211_attach(dev, wl)) {
				goto fail;
			}
			wl->wiphy = dev->ieee80211_ptr->wiphy;
		}
		else {
			SET_NETDEV_DEV(dev, parentdev);
			err = 0;
		}
	} else {
		WL_ERROR(("unsupported bus type\n"));
		goto fail;
	}
#else

	if (wl->bcm_bustype == PCI_BUS) {
		struct pci_dev *pci_dev = (struct pci_dev *)btparam;
		ASSERT(pci_dev);
		SET_NETDEV_DEV(dev, &pci_dev->dev);
	}
#endif /* defined(USE_CFG80211) */
	 wl_core_init((void *)wl);
#ifdef WLCSO
	if (wlc_tso_support(wl->wlc))
		dev->features |= NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM;
#endif /* WLCSO */

#if defined(CONFIG_WL_MODULE) && defined(CONFIG_BCM47XX)
#if defined(__ARM_ARCH_7A__) && defined(CONFIG_INET_GRO)
	dev->features |= NETIF_F_GRO;
#endif	/* __ARM_ARCH_7A__ && CONFIG_INET_GRO */
#endif /* CONFIG_WL_MODULE && CONFIG_BCM47XX */

#if defined(CONFIG_WL_MODULE) && defined(CONFIG_BCM47XX)
	/* Only for router platform */
	/* Check if the netdev should be allowed to register or not */
#ifdef WLRSDB
	if (wl_rsdb_block_netdev(commmondata, rsdb_found)) {
		goto success;
	}
	if (si_numd11coreunits(commmondata->sih) > 1) {
		/* This is a RSDB chip */
		rsdb_found++;
	}
#endif /* WLRSDB */
#endif /* CONFIG_WL_MODULE && CONFIG_BCM47XX */

	if (register_netdev(dev)) {
		WL_ERROR(("wl%d: register_netdev() failed\n", unit));
		goto fail;
	}
	wlif->dev_registered = TRUE;

#ifdef BCM_NBUFF_WLMCAST_IPV6
	/* Primary bsscfg has no referece to wlcif, this make retrieving netif from
	 * bsscfg difficult, Primary bsscfg should work the same to have the same reference
	 * pointer to MBSS, then it will be unfied handling without extra code to
	 * differentiate, IV6 wlmcast change to have all igs instance name the same
	 * as wireless interface name, will need to get primary interface name from wlcif
	 * as well. so here the linke is correctly created.
	 */
	if (wlif->wlcif) {
		wlif->wlcif->wlif = wlif;
		wl->wlc->cfg->wlcif = wlif->wlcif;
		wlif->wlcif->u.bsscfg = wl->wlc->cfg;
	}
	snprintf(wlif->name, sizeof(wlif->name), "%s", wlif->dev->name);
#endif /* BCM_NBUFF_WLMCAST_IPV6 */

#if defined(BCM_WFD)
	if (wl_wfd_registerdevice(wl->wfd_idx, dev) != 0)
	{
		WL_ERROR(("wl_wfd_registerdevice failed [%s]\n", dev->name));
		goto fail;
	}
#endif /* BCM_WFD */

#if defined(BCM_GMAC3)
	wlif->fwdh = fwder_bind(wl->fwdh, WL_FWDER_UNIT(wl), wlif->subunit,
	                        wlif->dev, TRUE);
	fwder_register(wlif->fwdh, wlif->dev);
#endif /* BCM_GMAC3 */

#if defined(HNDCTF)
	if ((ctf_dev_register(wl->cih, dev, FALSE) != BCME_OK) ||
	    (ctf_enable(wl->cih, dev, TRUE, &wl->pub->brc_hot) != BCME_OK)) {
		WL_ERROR(("wl%d: ctf_dev_register() failed\n", unit));
		goto fail;
	}

#endif /* HNDCTF */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 14)
#ifdef LINUX_CRYPTO
	/* load the tkip module */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 29)
	wl->tkipmodops = lib80211_get_crypto_ops("TKIP");
	if (wl->tkipmodops == NULL) {
		request_module("lib80211_crypt_tkip");
		wl->tkipmodops = lib80211_get_crypto_ops("TKIP");
	}
#else
	wl->tkipmodops = ieee80211_get_crypto_ops("TKIP");
	if (wl->tkipmodops == NULL) {
		request_module("ieee80211_crypt_tkip");
		wl->tkipmodops = ieee80211_get_crypto_ops("TKIP");
	}
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 29) */
#endif /* LINUX_CRYPTO */
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 14) */
#ifdef USE_IW
	wlif->iw.wlinfo = (void *)wl;
#endif // endif

#if defined(WL_CONFIG_RFKILL)
	if (wl_init_rfkill(wl) < 0)
		WL_ERROR(("%s: init_rfkill_failure\n", __FUNCTION__));
#endif // endif

#ifdef DISABLE_HT_RATE_FOR_WEP_TKIP
	/* disallow HT rate for WEP/TKIP */
	if (wlc_iovar_setint(wl->wlc, "ht_wsec_restrict", 0x3)) {
		WL_ERROR(("wl%d: Error setting ht_wsec_restrict \n", unit));
	}
#endif /* DISABLE_HT_RATE_FOR_WEP_TKIP */

#ifdef DEFAULT_EAPVER_AP
	/* use EAPOL version from AP */
	if (wlc_iovar_setint(wl->wlc, "sup_wpa2_eapver", -1)) {
		WL_ERROR(("wl%d: Error setting sup_wpa2_eapver \n", unit));
	}
	if (wlc_iovar_setint(wl->wlc, "sup_m3sec_ok", 1)) {
		WL_ERROR(("wl%d: Error setting sup_m3sec_ok \n", unit));
	}
#endif /* DEFAULT_EAPVER_AP */

	/* register module */
	if (wlc_module_register(wl->pub, NULL, "linux", wl, NULL, wl_linux_watchdog, NULL, NULL)) {
		WL_ERROR(("wl%d: %s wlc_module_register() failed\n",
		          wl->pub->unit, __FUNCTION__));
		goto fail;
	}
#if defined(WL11K_AP)&& defined(WL_MBO) && defined(MBO_AP)
	wlc_mbo_update_gasi(wl->wlc, (void*)(wl->gas));
#endif /* WL11K_AP && WL_MBO && MBO_AP */
#ifdef BCMDBG
	wlc_dump_register(wl->pub, "wl", (dump_fn_t)wl_dump, (void *)wl);
#endif // endif

#ifdef AVS
	/* Register Adaptive Voltage Scaling module */
	if ((wl->avs = wl_avs_attach(wl->wlc)) == NULL) {
		WL_ERROR(("wl%d: AVS disabled\n", wl->pub->unit));
	}
#endif /* AVS */

	/* Get dump file signature */
	wl->dump_signature = (uint16)OSL_RAND();

	if (device > 0x9999)
		printf("%s: Broadcom BCM%d 802.11 Wireless Controller " EPI_VERSION_STR,
			dev->name, device);
	else
		printf("%s: Broadcom BCM%04x 802.11 Wireless Controller " EPI_VERSION_STR,
			dev->name, device);

#ifdef BCMDBG
#if defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 9))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdate-time"
#endif // endif
	printf(" (Compiled in " SRCBASE " at " __TIME__ " on " __DATE__ ")");
#if defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 9))
#pragma GCC diagnostic pop
#endif // endif
#endif /* BCMDBG */
	printf("\n");

#if defined(CONFIG_WL_MODULE) && defined(CONFIG_BCM47XX)
#ifdef WLRSDB
success:
#endif // endif
#endif /* CONFIG_WL_MODULE && CONFIG_BCM47XX */

	wl_found++;
	return wl;

fail:
	wl_free(wl);
	return NULL;
} /* wl_attach */

#if defined(CONFIG_PROC_FS)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
static int
wl_read_proc(struct seq_file *m, void *v)
{
	wl_info_t *wl;
	char buffer[1016] = {0};

	wl = (wl_info_t *)v;

	WL_LOCK(wl);
#if defined(BCMDBG)
	/* pass space delimited variables for dumping */
	wlc_iovar_op(wl->wlc, "dump", NULL, 0, buffer, sizeof(buffer), IOV_GET, NULL);
#endif // endif
	WL_UNLOCK(wl);

	seq_puts(m, buffer);

	return 0;
}
#else /* #if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0)) */
static int
wl_read_proc(char *buffer, char **start, off_t offset, int length, int *eof, void *data)
{
	int len;
	off_t pos;
	off_t begin;

	len = pos = begin = 0;

#if defined(BCMDBG)
	{
	wl_info_t *wl;

	wl = (wl_info_t*) data;

	WL_LOCK(wl);
	/* pass space delimited variables for dumping */
	wlc_iovar_op(wl->wlc, "dump", NULL, 0, buffer, PAGE_SIZE, IOV_GET, NULL);
	len = strlen(buffer);
	WL_UNLOCK(wl);
	}
#endif // endif
	pos = begin + len;

	if (pos < offset) {
		len = 0;
		begin = pos;
	}

	*eof = 1;

	*start = buffer + (offset - begin);
	len -= (offset - begin);

	if (len > length)
		len = length;

	return (len);
} /* wl_read_proc */
#endif /* #if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0)) */
#endif /* #if defined(CONFIG_PROC_FS) */

/* For now, JTAG, SDIO, and PCI are mutually exclusive.  When this changes, remove
 * #if !defined(BCMJTAG) && !defined(BCMSDIO) ... #endif conditionals.
 */
#if !defined(BCMJTAG)
#ifdef CONFIG_PCI
static void __devexit wl_remove(struct pci_dev *pdev);

/**
 * determines if a device is a WL device, and if so, attaches it.
 *
 * This function determines if a device pointed to by pdev is a WL device,
 * and if so, performs a wl_attach() on it.
 *
 */
int __devinit
wl_pci_probe(struct pci_dev *pdev, const struct pci_device_id *ent)
{
	int rc;
	wl_info_t *wl;
#ifdef LINUXSTA_PS
	uint32 val;
#endif // endif
	uint32 bar1_size = 0, bar2_size = 0;
	void *bar1_addr = NULL;
	void *bar2_addr = NULL;

	if (!pdev->irq) {
		WL_TRACE(("%s: Rejecting device 0x%x with irq 0 on bus %d slot %d\n",
			__FUNCTION__, pdev->device, pdev->bus->number, PCI_SLOT(pdev->devfn)));
		return (-ENODEV);
	}

	WL_TRACE(("%s: bus %d slot %d func %d irq %d\n", __FUNCTION__,
		pdev->bus->number, PCI_SLOT(pdev->devfn), PCI_FUNC(pdev->devfn), pdev->irq));

	if (pdev->vendor == PCI_VENDOR_ID_BROADCOM && (
			pdev->device == BCM43684_CHIP_ID ||
			pdev->device == BCM43684_D11AX_ID ||
			pdev->device == BCM43684_D11AX2G_ID ||
			pdev->device == BCM43684_D11AX5G_ID)) {
		int bcm43684_nic_enabled = getintvar(NULL, "bcm43684_nic");
		if (bcm43684_nic_enabled == 0) {
			WL_ERROR(("%s: bcm43684 nic mode is not enabled\n", __FUNCTION__));
			return (-ENODEV);
		}
	}

	if (!wlc_chipmatch(pdev->vendor, pdev->device))
		return (-ENODEV);

#ifdef DISABLE_SECOND_INTERNAL_RADIO
#define IS_47622_SLAVE_SLICE(devid, enum_base) \
	(devid == EMBEDDED_2x2AX_ID && enum_base == BCM47622_DEV_B_PHYS_ADDR)
	if (IS_47622_SLAVE_SLICE(pdev->device, pci_resource_start(pdev, 0))) {
		WL_TRACE(("%s: Rejecting device 0x%x on bus %d\n",
			__FUNCTION__, pdev->device, pdev->bus->number));
		return (-ENODEV);
	}
#endif /* DISABLE_SECOND_INTERNAL_RADIO */

#if defined(WLRSDB) && defined(WLRSDB_DISABLED)
	if (BCM53573_DEVICE(pdev->device) && wl_found) {
		WL_TRACE(("%s: Blocking second wl for RSDB_DISABLED device %x\n",
				__FUNCTION__, pdev->device));
		return (-ENODEV);
	}
#endif /* WLRSDB && WLRSDB_DISABLED */

	rc = pci_enable_device(pdev);
	if (rc) {
		WL_ERROR(("%s: Cannot enable device %d-%d_%d\n", __FUNCTION__,
			pdev->bus->number, PCI_SLOT(pdev->devfn), PCI_FUNC(pdev->devfn)));
		return (-ENODEV);
	}
	pci_set_master(pdev);

#ifdef LINUXSTA_PS
	/*
	 * Disable the RETRY_TIMEOUT register (0x41) to keep
	 * PCI Tx retries from interfering with C3 CPU state.
	 * Code taken from ipw2100 driver
	 */
	pci_read_config_dword(pdev, 0x40, &val);
	if ((val & 0x0000ff00) != 0)
		pci_write_config_dword(pdev, 0x40, val & 0xffff00ff);
#endif /* LINUXSTA_PS */

#if defined(WLVASIP)
	bar1_size = pci_resource_len(pdev, 2);
	bar1_addr = (uchar *)ioremap_nocache(pci_resource_start(pdev, 2),
		bar1_size);
	bar2_size = pci_resource_len(pdev, 4);
	if (bar2_size) {
		bar2_addr = (uchar *)ioremap_nocache(pci_resource_start(pdev, 4), bar2_size);
	}
#endif // endif

	wl = wl_attach(pdev->vendor, pdev->device, pci_resource_start(pdev, 0), PCI_BUS, pdev,
		pdev->irq, bar1_addr, bar1_size, bar2_addr, bar2_size, (void *)ent->driver_data);

	if (!wl)
		return -ENODEV;

	pci_set_drvdata(pdev, wl);

#if defined(CONFIG_BCM_WLAN_DGASP)
	kerSysRegisterDyingGaspHandler(wl_netdev_get(wl)->name, &wl_shutdown_handler, wl);
#endif /* CONFIG_BCM_WLAN_DGASP */

	return 0;
} /* wl_pci_probe */

#ifdef IS_BCA_2x2AX_BUILD
static void
wl_pci_shutdown(struct pci_dev *pdev)
{
	wl_info_t *wl = (wl_info_t *) pci_get_drvdata(pdev);

	if (wl != NULL) {
		wl_remove(pdev);
	}
}
#endif /* IS_BCA_2x2AX_BUILD */

#if defined(STB) && !defined(STBAP)
#ifdef LINUXSTA_PS
/** LINUXSTA_PS specific function */
static int
wl_suspend(struct pci_dev *pdev, DRV_SUSPEND_STATE_TYPE state)
{
	wl_info_t *wl = (wl_info_t *) pci_get_drvdata(pdev);

	WL_TRACE(("wl: wl_suspend\n"));

	wl = (wl_info_t *) pci_get_drvdata(pdev);
	if (!wl) {
		WL_ERROR(("wl: wl_suspend: pci_get_drvdata failed\n"));
		return -ENODEV;
	}

	wl_bus_set_wowl(wl, true);

	WL_LOCK(wl);
	WL_APSTA_UPDN(("wl%d (%s): wl_suspend() -> wl_down()\n", wl->pub->unit, wl->dev->name));
	si_pcie_prep_D3(wl->pub->sih, TRUE);
	wlc_radio_monitor_stop(wl->wlc);
	wl_down(wl);
	wl->pub->hw_up = FALSE;
	si_pci_sleep(wl->pub->sih);
	wl->pub->hw_off = TRUE;
	WL_UNLOCK(wl);
	PCI_SAVE_STATE(pdev, wl->pci_psstate);
	pci_disable_device(pdev);
	return pci_set_power_state(pdev, PCI_D3hot);
}

/** LINUXSTA_PS specific function */
static int
wl_resume(struct pci_dev *pdev)
{
	wl_info_t *wl = (wl_info_t *) pci_get_drvdata(pdev);
	int err = 0;
	uint32 val;

	WL_TRACE(("wl: wl_resume\n"));

	if (!wl) {
		WL_ERROR(("wl: wl_resume: pci_get_drvdata failed\n"));
		return -ENODEV;
	}

	err = pci_set_power_state(pdev, PCI_D0);
	if (err)
		return err;

	PCI_RESTORE_STATE(pdev, wl->pci_psstate);

	err = pci_enable_device(pdev);
	if (err)
		return err;

	pci_set_master(pdev);
	/*
	 * Suspend/Resume resets the PCI configuration space, so we have to
	 * re-disable the RETRY_TIMEOUT register (0x41) to keep
	 * PCI Tx retries from interfering with C3 CPU state
	 * Code taken from ipw2100 driver
	 */
	pci_read_config_dword(pdev, 0x40, &val);
	if ((val & 0x0000ff00) != 0)
		pci_write_config_dword(pdev, 0x40, val & 0xffff00ff);

	wl_bus_set_wowl(wl, false);

	WL_LOCK(wl);
	WL_APSTA_UPDN(("wl%d: (%s): wl_resume() -> wl_up()\n", wl->pub->unit, wl->dev->name));

	wl->pub->hw_off = FALSE;
	err = wl_up(wl); /* re-inits e.g. PMU registers before 'up' */
	WL_UNLOCK(wl);

	return (err);
} /* wl_resume */

/* Enable or disable Wake-on-wireless LAN  for all buses */
static void wl_clear_wowl(wl_info_t *wl)
{
	int value = 0, err = BCME_OK;
	/* Reset the wowl triggers */
	err = wlc_iovar_setint(wl->wlc, "wowl", value);
	if (err < 0) {
		WL_ERROR(("%s: wowl_set error, result=%d\n", __FUNCTION__, err));
	}
}

static int wl_set_wowl_trigger(wl_info_t *wl, struct net_device *ndev)
{
	int err = 0;
	int wowl_trigger = 0;
#if defined(WL_CFG80211)
	int wowl_cfg_trigger = 0;
#endif /* WL_CFG80211 */
	err = wlc_iovar_getint(wl->wlc, "wowl", &wowl_trigger);
	if (err < 0) {
		WL_ERROR(("%s: error in get wowl_enable, result=%d\n",
			__FUNCTION__, err));
	}
#if defined(WL_CFG80211)
	wowl_cfg_trigger = wl_cfg80211_get_wowlan_triggers(ndev);

	/* No action required if already the trigger setby wl utility */
	if (((wowl_cfg_trigger & wowl_trigger) != wowl_cfg_trigger)) {
		wowl_trigger |= wowl_cfg_trigger;
		err = wl_cfg80211_set_wowlan_triggers(ndev, wowl_trigger);
		if (err < 0) {
			WL_ERROR(("%s: error in get wowl_cfg_set,result=%d\n",
				__FUNCTION__, wowl_cfg_trigger));
		}
	}
#endif /* WL_CFG80211 */
	return wowl_trigger;
}

static void wl_set_wowl(wl_info_t *wl, int state)
{
	int err = 0;
	int value = 0;
	int wowl_trigger = 0;
	struct net_device *ndev = wl->dev;

	/* Read the wowlan trigger, set by either wl utility or wpa_supplicant
	 * And activate the wowl
	 */
	if (state) {
		wowl_trigger = wl_set_wowl_trigger(wl, ndev);
		WL_ERROR(("%s : Wowl Trigger (%08x)\n", __FUNCTION__, wowl_trigger));
		if (wowl_trigger > 0) {
			value = 1;
			/* wowl_force calls wl_down which need LOCK */
			WL_LOCK(wl);
			err = wlc_iovar_setint(wl->wlc, "wowl_force", value);
			WL_UNLOCK(wl);
			if (err < 0) {
				WL_ERROR(("%s: error wowl_force, err=%d\n", __FUNCTION__, err));
			}
		}
	} else {
		value = 0;
#if defined(WL_CFG80211)
		wl_cfg80211_update_wowl_wakeind(ndev);
#endif /* WL_CFG80211 */
		wl_clear_wowl(wl);
	}
}

static void wl_bus_set_wowl(void *drvinfo, int state)
{
	wl_info_t *wl = (wl_info_t *)drvinfo;
	if (wl) {
		wl_set_wowl(wl, state);
	}
}
#endif /* LINUXSTA_PS */
#endif /* STB && STBAP */

static void __devexit
wl_remove(struct pci_dev *pdev)
{
	wl_info_t *wl = (wl_info_t *) pci_get_drvdata(pdev);
	uint16 vendorid, deviceid;

	if (!wl) {
		WL_ERROR(("wl: wl_remove: pci_get_drvdata failed\n"));
		return;
	}

	/* Get the the actual vendor/device id used in the driver */
	wlc_get_override_vendor_dev_id(wl->wlc, &vendorid, &deviceid);

	if (!wlc_chipmatch(vendorid, deviceid)) {
		WL_ERROR(("wl: wl_remove: wlc_chipmatch failed\n"));
		return;
	}

	/* wl_set_monitor(wl, 0); */

	WL_LOCK(wl);
	WL_APSTA_UPDN(("wl%d (%s): wl_remove() -> wl_down()\n", wl->pub->unit, wl->dev->name));
	wl_down(wl);
	WL_UNLOCK(wl);

#if defined(CONFIG_BCM_WLAN_DGASP)
	kerSysDeregisterDyingGaspHandler(wl_netdev_get(wl)->name);
#endif /* CONFIG_BCM_WLAN_DGASP */

	wl_free(wl);
	pci_disable_device(pdev);
	pci_set_drvdata(pdev, NULL);
}

static struct pci_driver wl_pci_driver = {
	name:		"wl",
	probe:		wl_pci_probe,
#if defined(STB) && !defined(STBAP)
#ifdef LINUXSTA_PS
	suspend:	wl_suspend,
	resume:		wl_resume,
#endif /* LINUXSTA_PS */
#endif /* STB && STBAP */
	remove:		__devexit_p(wl_remove),
#ifdef IS_BCA_2x2AX_BUILD
	shutdown:       wl_pci_shutdown,
#endif /* IS_BCA_2x2AX_BUILD */
	id_table:	wl_id_table,
	};
#endif /* CONFIG_PCI */
#endif // endif

#ifdef BCMJTAG
static bcmjtag_driver_t wl_jtag_driver = {
	wl_jtag_probe,
	wl_jtag_detach,
	wl_jtag_poll,
	};
#endif /* BCMJTAG */

#if defined(PLATFORM_INTEGRATED_WIFI) && defined(CONFIG_OF)
static const struct of_device_id plat_devices_of_match[] = {
#ifdef STB_SOC_WIFI
	{ .compatible = "brcm,bcm7271-wlan", },
#endif /* STB_SOC_WIFI */
	{ .compatible = "", } /* Empty terminated list */
};
MODULE_DEVICE_TABLE(of, plat_devices_of_match);

static struct platform_driver wl_plat_drv = {
	.probe	=		wl_plat_drv_probe,
	.remove =		wl_plat_drv_remove,
	.shutdown =		wl_plat_drv_shutdown,
	.suspend =		wl_plat_drv_suspend,
	.resume =		wl_plat_drv_resume,
	.driver =		{
		.name = "wl",
		.owner = THIS_MODULE,
		.of_match_table = plat_devices_of_match,
		},
};
#endif /* PLATFORM_INTEGRATED_WIFI && CONFIG_OF */

/**
 * This is the main entry point for the WL driver.
 *
 * This function determines if a device pointed to by pdev is a WL device,
 * and if so, performs a wl_attach() on it.
 */
static int __init
wl_module_init(void)
{
	int error = -ENODEV;

#ifdef BCMDBG
	if (msglevel != 0xdeadbeef)
		wl_msg_level = msglevel;
	else {
		const char *var = getvar(NULL, "wl_msglevel");
		if (var)
			wl_msg_level = bcm_strtoul(var, NULL, 0);
	}
	printf("%s: msglevel set to 0x%x\n", __FUNCTION__, wl_msg_level);
	if (msglevel2 != 0xdeadbeef)
		wl_msg_level2 = msglevel2;
	else {
		const char *var = getvar(NULL, "wl_msglevel2");
		if (var)
			wl_msg_level2 = bcm_strtoul(var, NULL, 0);
	}
	printf("%s: msglevel2 set to 0x%x\n", __FUNCTION__, wl_msg_level2);
	{
		extern uint32 phyhal_msg_level;

		if (phymsglevel != 0xdeadbeef)
			phyhal_msg_level = phymsglevel;
		else {
			const char *var = getvar(NULL, "phy_msglevel");
			if (var)
				phyhal_msg_level = bcm_strtoul(var, NULL, 0);
		}
		printf("%s: phymsglevel set to 0x%x\n", __FUNCTION__, phyhal_msg_level);
	}
#endif /* BCMDBG */

#if defined(WL_ALL_PASSIVE)
	{
		const char *var = getvar(NULL, "wl_dispatch_mode");
		if (var)
			passivemode = bcm_strtoul(var, NULL, 0);
		printf("%s: passivemode set to 0x%x\n", __FUNCTION__, passivemode);
		var = getvar(NULL, "txworkq");
		if (var)
			txworkq = bcm_strtoul(var, NULL, 0);
		printf("%s: txworkq set to 0x%x\n", __FUNCTION__, txworkq);
	}
#endif /* defined(WL_ALL_PASSIVE) */

#if defined(CONFIG_WL_ALL_PASSIVE_RUNTIME) || defined(WL_ALL_PASSIVE)
	{
		char *var = getvar(NULL, "wl_txq_thresh");
		if (var)
			wl_txq_thresh = bcm_strtoul(var, NULL, 0);
#ifdef BCMDBG
		WL_PRINT(("%s: wl_txq_thresh set to 0x%x\n",
			__FUNCTION__, wl_txq_thresh));
#endif // endif
		var = getvar(NULL, "wl_txq_bound");
		if (var)
			wl_txq_bound = bcm_strtoul(var, NULL, 0);
#ifdef BCMDBG
		WL_PRINT(("%s: wl_txq_bound set to 0x%x\n",
			__FUNCTION__, wl_txq_bound));
#endif // endif
	}
#endif /* CONFIG_WL_ALL_PASSIVE_RUNTIME || WL_ALL_PASSIVE */

#if defined(USE_CFG80211)
	wl_cfg80211_register_notifier();
#endif /* USE_CFG80211 */

#if defined(BCM_PKTFWD)
	wl_pktfwd_sys_init(); /* Instantiate the singleton wl_pktfwd global */
#endif /* BCM_PKTFWD */

#ifdef BCMJTAG
	if (!(error = bcmjtag_register(&wl_jtag_driver)))
		return (0);
#endif /* BCMJTAG */

#if !defined(BCMJTAG)
#ifdef CONFIG_PCI
	{
		uint16 devid = 0x0;

#ifdef STB_SOC_WIFI
		devid = wl_stbsoc_get_devid();
#else /* STB_SOC_WIFI */
		const char *var = getvar(NULL, "devid");
		if (var)
			devid = bcm_strtoul(var, NULL, 0);
#endif /* STB_SOC_WIFI */

		switch (devid) {
			case BCM7271_D11AC_ID:
#if defined(PLATFORM_INTEGRATED_WIFI) && defined(CONFIG_OF)
				if (!(error = platform_driver_register(&wl_plat_drv)))
					return 0;
#else /* PLATFORM_INTEGRATED_WIFI && CONFIG_OF */
				WL_ERROR(("%s: PLATFORM_INTEGRATED_WIFI needed.\n",
					__FUNCTION__));
#endif /* PLATFORM_INTEGRATED_WIFI && CONFIG_OF */
				break;
			default:
				if (!(error = pci_module_init(&wl_pci_driver)))
					return (0);
				break;
		}
	}
#endif /* CONFIG_PCI */
#endif // endif

#if defined(USE_CFG80211)
	wl_cfg80211_unregister_notifier();
#endif // endif
	return (error);
} /* wl_module_init */

/**
 * This function unloads the WL driver from the system.
 *
 * This function unconditionally unloads the WL driver module from the
 * system.
 */
static void __exit
wl_module_exit(void)
{
#ifdef BCMJTAG
	bcmjtag_unregister();
#endif /* BCMJTAG */

#if !defined(BCMJTAG)
#ifdef CONFIG_PCI
	{
		uint16 devid = 0x0;

#ifdef STB_SOC_WIFI
		devid = wl_stbsoc_get_devid();
#else /* STB_SOC_WIFI */
		const char *var = getvar(NULL, "devid");
		if (var)
			devid = bcm_strtoul(var, NULL, 0);
#endif /* STB_SOC_WIFI */

		switch (devid) {
			case BCM7271_D11AC_ID:
#if defined(PLATFORM_INTEGRATED_WIFI) && defined(CONFIG_OF)
				platform_driver_unregister(&wl_plat_drv);
#else /* PLATFORM_INTEGRATED_WIFI && CONFIG_OF */
				WL_ERROR(("%s: PLATFORM_INTEGRATED_WIFI needed.\n",
					__FUNCTION__));
#endif /* PLATFORM_INTEGRATED_WIFI && CONFIG_OF */
				break;
			default:
				pci_unregister_driver(&wl_pci_driver);
				break;
		}
	}

#endif /* CONFIG_PCI */
#endif // endif
#if defined(BCM_PKTFWD)
	wl_pktfwd_sys_fini(); /* Destruct the singleton wl_pktfwd global */
#endif // endif

#if defined(USE_CFG80211)
	wl_cfg80211_unregister_notifier();
#endif /* USE_CFG80211 */
} /* wl_module_exit */

module_init(wl_module_init);
module_exit(wl_module_exit);

/**
 * This function frees the WL per-device resources.
 *
 * This function frees resources owned by the WL device pointed to
 * by the wl parameter.
 */
void
wl_free(wl_info_t *wl)
{
	wl_timer_t *t, *next;
	osl_t *osh;
#if defined(USE_CFG80211)
	struct bcm_cfg80211 *cfg = NULL;
#endif // endif
	unsigned long wait_callback_timeout;

	WL_TRACE(("wl: wl_free\n"));
#ifdef SAVERESTORE
	/* need to disable SR before unload the driver
	 * the HW/CLK may be down
	 */
	if (wl->wlc)
		wlc_iovar_setint(wl->wlc, "sr_enable", 0);
#endif /* SAVERESTORE */

#ifdef BCM_WFD
	/* only unbind if bound */
	if (wl->wfd_idx >= 0)
#if defined(BCM_PKTFWD)
		wl_wfd_unbind(wl);
#else
		wl_wfd_unbind(wl->wfd_idx);
#endif // endif
#endif /* BCM_WFD */

#ifdef AVS
	if (wl->avs != NULL) {
		wl_avs_detach(wl->avs);
		wl->avs = NULL;
	}
#endif /* AVS */

#ifdef BCMJTAG
	if (wl->bcm_bustype != JTAG_BUS)
#endif /* BCMJTAG */
	{
		if (wl->dev && wl->dev->irq) {
#if defined(IS_BCA_2x2AX_BUILD) && defined(WLC_OFFLOADS_TXSTS) && !defined(BCMQT)
			if (wl->dev_id == EMBEDDED_2x2AX_ID) {
				free_irq(GET_2x2AX_M2M_IRQV(wl->dev->base_addr), wl);
			}
#endif /* IS_BCA_2x2AX_BUILD && WLC_OFFLOADS_TXSTS && !BCMQT */
			free_irq(wl->dev->irq, wl);
		}
	}
#if defined(WL_CONFIG_RFKILL)
	wl_uninit_rfkill(wl);
#endif // endif

#ifdef NAPI_POLL
	clear_bit(__LINK_STATE_START, &wl->dev->state);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 30)
	if (wl->napi.poll == wl_poll)
		napi_disable(&wl->napi);
#elif  LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 21)
	if (dev->poll == wl_poll)
		netif_poll_disable(wl->dev);
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 30) */
#endif /* NAPI_POLL */

	wl_core_deinit((void *)wl);

	if (wl->dev) {
#if defined(USE_CFG80211)
		cfg = wl_get_cfg(wl->dev);
		wl_terminate_event_handler(wl->dev);
#endif // endif
		wl_free_if(wl, WL_DEV_IF(wl->dev), TRUE);
		wl->dev = NULL;
	}
	/* free monitor */
	if (wl->monitor_dev) {
		wl_free_if(wl, WL_DEV_IF(wl->monitor_dev), TRUE);
		wl->monitor_dev = NULL;
	}

#ifdef TOE
	wl_toe_detach(wl->toei);
#endif // endif

#ifndef LINUX_POSTMOGRIFY_REMOVAL
#if defined(P2PO)
	if (wl->p2po)
		wl_p2po_detach(wl->p2po);
	if (wl->disc)
		wl_disc_detach(wl->disc);
#endif /* P2PO */
#if defined(ANQPO)
	if (wl->anqpo)
		wl_anqpo_detach(wl->anqpo);
#endif /* ANQPO */
#if defined(P2PO) || defined(ANQPO) || (defined(WL_MBO) && defined(MBO_AP))
	if (wl->gas) {
		wlc_mbo_update_gasi(wl->wlc, NULL); /* avoids references to freed gas pointer */
		wl_gas_detach(wl->gas);
	}
#endif	/* P2PO || ANQPO || (WL_MBO && MBO_AP) */
#ifdef WL_EVENTQ
	if (wl->wlevtq)
		wl_eventq_detach(wl->wlevtq);
#endif /* WL_EVENTQ */
#if defined(BDO) && !defined(BDO_DISABLED)
	if (wl->bdo)
		wl_bdo_detach(wl->bdo);
#endif /* defined(BDO) && !defined(BDO_DISABLED) */
#endif /* LINUX_POSTMOGRIFY_REMOVAL */

#ifdef ARPOE
	wl_arp_detach(wl->arpi);
#endif // endif

#if defined(BCM_GMAC3)
	if (wl->fwdh != FWDER_NULL)
		wl->fwdh = fwder_dettach(wl->fwdh, FWDER_DNSTREAM, WL_FWDER_UNIT(wl));
#endif /* BCM_GMAC3 */

#if defined(BCM_AWL)
	if (wl->pub && wl->pub->awl_cb)
		wl_awl_detach(wl, wl->pub->awl_cb);
#endif /* BCM_AWL */

#if defined(PKTC_TBL) && !defined(BCM_PKTFWD)
	if (wl->pktc_tbl)
		wl_pktc_detach(wl);
#endif /* PKTC_TBL && !BCM_PKTFWD */

#if defined(DPSTA) && ((defined(STA) && defined(DWDS)) || defined(PSTA))
	if (wl->pub)
		dpsta_unregister(wl->pub->unit);
#endif

#ifndef NAPI_POLL
	/* kill dpc */
#if !defined(WL_USE_L34_THREAD)
	tasklet_kill(&wl->tasklet);
#endif /* !WL_USE_L34_THREAD */
#endif /* ifndef NAPI_POLL */

#if !defined(WL_USE_L34_THREAD)
	/* kill tx tasklet */
	tasklet_kill(&wl->tx_tasklet);
#endif /* !WL_USE_L34_THREAD */

	wlc_module_unregister(wl->pub, "linux", wl);

#if defined(BCM_EAPFWD) && defined(BCM_PKTFWD)
	if (wl->wlc)
		wl_eap_unbind(wl->wlc->pub->unit);
#endif // endif

	/* free common resources */
	if (wl->wlc) {
#if defined(CONFIG_PROC_FS)
		if ((wl->proc_entry != NULL) && (wl->pub != NULL)) {
			/* remove /proc/net/wl<unit> */
			char tmp[32];

			(void)snprintf(tmp, sizeof(tmp), "net/wl%d", wl->pub->unit);
			tmp[sizeof(tmp) - 1] = '\0';
			WL_INFORM(("wl%d: Removing the proc entry %s \n", wl->pub->unit, tmp));
			remove_proc_entry(tmp, 0);
		}
#endif /* defined(CONFIG_PROC_FS) */
		wlc_detach(wl->wlc);
		wl->wlc = NULL;
		wl->pub = NULL;
	}

	/* virtual interface deletion is deferred so we cannot spinwait */

	/* wait for all pending callbacks to complete */
	wait_callback_timeout = jiffies + msecs_to_jiffies(5000);
	while ((atomic_read(&wl->callbacks) > 0) && time_after(wait_callback_timeout, jiffies))
		schedule();
	if (atomic_read(&wl->callbacks)) {
		WL_ERROR(("wl%d: %s: callbacks is %d after 5 seconds timeout!\n",
			wl->unit, __FUNCTION__, atomic_read(&wl->callbacks)));
		ASSERT(0);
	}

#if defined(PKTC_TBL) && defined(BCM_PKTFWD)
	/* BCM_PKTFWD: wl_pktfwd_radio_del(wl) to delete wl radio from pktfwd */
	if (wl->pktc_tbl)
		wl_pktc_detach(wl);
#endif /* PKTC_TBL && BCM_PKTFWD */

#if defined(USE_CFG80211)
	wl_cfg80211_detach(cfg);
#endif // endif

	/* free timers */
	for (t = wl->timers; t; t = next) {
		next = t->next;
#ifdef BCMDBG
		if (t->name)
			MFREE(wl->osh, t->name, strlen(t->name) + 1);
#endif // endif
		MFREE(wl->osh, t, sizeof(wl_timer_t));
	}

#ifdef WL_ALL_PASSIVE
	wl_free_timer_freelist(wl, TRUE);
#endif /* WL_ALL_PASSIVE */
	osh = wl->osh;
	/*
	 * unregister_netdev() calls get_stats() which may read chip registers
	 * so we cannot unmap the chip registers until after calling unregister_netdev() .
	 */
	if (wl->regsva && BUSTYPE(wl->bcm_bustype) != SDIO_BUS &&
	    BUSTYPE(wl->bcm_bustype) != JTAG_BUS) {
		iounmap((void*)wl->regsva);
	}
	wl->regsva = NULL;
	/* move following code under bustype */
#if defined(WLVASIP)
	if (wl->bar1_addr) {
		iounmap(wl->bar1_addr);
		wl->bar1_addr = NULL;
	}

	if (wl->bar2_addr) {
		iounmap(wl->bar2_addr);
		wl->bar2_addr = NULL;
	}
#endif // endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 14)
#ifdef LINUX_CRYPTO
	/* un register the TKIP module...if any */
	if (wl->tkipmodops != NULL) {
		int idx;
		if (wl->tkip_ucast_data) {
			wl->tkipmodops->deinit(wl->tkip_ucast_data);
			wl->tkip_ucast_data = NULL;
		}
		for (idx = 0; idx < NUM_GROUP_KEYS; idx++) {
			if (wl->tkip_bcast_data[idx]) {
				wl->tkipmodops->deinit(wl->tkip_bcast_data[idx]);
				wl->tkip_bcast_data[idx] = NULL;
			}
		}
	}
#endif /* LINUX_CRYPTO */
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 14) */

#if defined(WL_USE_L34_THREAD)
	if (!IS_ERR_OR_NULL(wl->kthread))
		wl_thread_detach(wl);
#endif /* WL_USE_L34_THREAD */

	wl_txq_free(wl);

#ifdef HNDCTF
	/* free ctf resources */
	ctf_detach(wl->cih);
#endif /* HNDCTF */

#ifdef WL_OBJ_REGISTRY
	if (wl->cmn && (obj_registry_unref(wl->objr, OBJR_WL_CMN_INFO) == 0)) {
		obj_registry_set(wl->objr, OBJR_WL_CMN_INFO, NULL);
		MFREE(osh, wl->cmn, sizeof(*(wl->cmn)));
		wl->cmn = NULL;
	}
	if (wl->objr && (obj_registry_unref(wl->objr, OBJR_SELF) == 0)) {
		obj_registry_set(wl->objr, OBJR_SELF, NULL);
		obj_registry_free(wl->objr, osh);
		wl->objr = NULL;
	}
#endif // endif

	MFREE(osh, wl, sizeof(wl_info_t));

#ifdef BCMDBG_CTRACE
	PKT_CTRACE_DUMP(osh, NULL);
#endif // endif
	if (MEMORY_LEFTOVER(osh)) {
		printf("Memory leak of bytes %d\n", MEMORY_LEFTOVER(osh));
		ASSERT(0);
	}

	osl_detach(osh);
} /* wl_free */

/** Called by the Linux kernel */
static int
wl_open(struct net_device *dev)
{
	wl_info_t *wl;
	int error = 0;

	if (!dev)
		return -ENETDOWN;

	wl = WL_INFO_GET(dev);

	WL_TRACE(("wl%d: wl_open\n", wl->pub->unit));

	WL_LOCK(wl);
	WL_APSTA_UPDN(("wl%d: (%s): wl_open() -> wl_up()\n",
		wl->pub->unit, wl->dev->name));
	/* Since this is resume, reset hw to known state */
	error = wl_up(wl);
	if (!error) {
		error = wlc_set(wl->wlc, WLC_SET_PROMISC, (dev->flags & IFF_PROMISC));
	}
	if (dev != wl->dev) {
		netif_wake_queue(dev);
	}
	WL_UNLOCK(wl);

	if (!error)
		OLD_MOD_INC_USE_COUNT;

#if defined(USE_CFG80211)
	if (wl_cfg80211_up(dev)) {
		WL_ERROR(("%s: failed to bring up cfg80211\n", __func__));
		return -1;
	}
#endif // endif
#if defined(BCM_PKTFWD)
{
	wl_if_t *wlif;
	wlc_bsscfg_t *bsscfg;

	wlif = WL_DEV_IF(dev);
	bsscfg = wl_bsscfg_find(wlif);
	if (BSSCFG_STA(bsscfg) && bsscfg->BSS) {
		netdev_wlan_set_dwds_client(wlif->d3fwd_wlif);
	}
}
#endif /* BCM_PKTFWD */

	return (error? -ENODEV : 0);
} /* wl_open */

/** Called by the Linux kernel */
static int
wl_close(struct net_device *dev)
{
	wl_info_t *wl;

	if (!dev)
		return -ENETDOWN;

	WL_ERROR(("wl%s: wl_close\n", dev->name));
	wl = WL_INFO_GET(dev);
#if defined(USE_CFG80211)
	if (dev == wl->dev)
		wl_cfg80211_down(dev);
#endif // endif

	WL_LOCK(wl);
	WL_APSTA_UPDN(("wl(%s): wl_close() -> wl_down()\n", dev->name));
	if (dev == wl->dev) {
		wl_down(wl);
	} else {
		netif_down(dev);
		netif_stop_queue(dev);
	}
	WL_UNLOCK(wl);

	OLD_MOD_DEC_USE_COUNT;

	return (0);
} /* wl_close */

#ifdef PKTC_TBL
static void
wl_uninit(struct net_device *dev)
{
	/* wipe out entire pktc table */
	wl_pktc_req(PKTC_TBL_FLUSH, (unsigned long)dev, 0, 0);

	return;
}
#endif // endif

#ifdef ARPOE
/**
 * Return the proper arpi pointer for either corr to an IF or
 *	default. For IF case, Check if arpi is present. It is possible that, upon a
 *	down->arpoe_en->up scenario, interfaces are not reallocated, and
 *	so, wl->arpi could be NULL. If so, allocate it and use.
 */
static wl_arp_info_t *
wl_get_arpi(wl_info_t *wl, struct wl_if *wlif)
{
	if (wlif != NULL) {
		if (wlif->arpi == NULL)
			wlif->arpi = wl_arp_alloc_ifarpi(wl->arpi, wlif->wlcif);
		/* note: this could be null if the above wl_arp_alloc_ifarpi fails */
		return wlif->arpi;
	} else
		return wl->arpi;
}
#endif /* ARPOE */

/** used by the ARPOE module to get the ARPI context */
void * BCMFASTPATH
wl_get_ifctx(struct wl_info *wl, int ctx_id, wl_if_t *wlif)
{
	void *ifctx = NULL;

	switch (ctx_id) {
#ifdef ARPOE
	case IFCTX_ARPI:
		ifctx = (void *)wlif->arpi;
		break;
#endif // endif
	case IFCTX_NETDEV:
		ifctx = (void *)((wlif == NULL) ? wl->dev : wlif->dev);
		break;

	default:
		break;
	}

	return ifctx;
}

/** forwards one or more packets to transmit to the WLC layer */
int BCMFASTPATH
wl_start_int(wl_info_t *wl, wl_if_t *wlif, struct sk_buff *skb)
{
	return wl_start_int_try(wl, wlif, skb, WL_LOCK_NOTTAKEN);
}

/* Caller should hold WL_LOCK and responsible for osh accounting */
int
wl_pktc_tx(wl_info_t *wl, wl_if_t *wlif, void *pkt)
{

	ASSERT(pkt != NULL);

#ifdef ARPOE
	/* Arp agent */
	if (ARPOE_ENAB(wl->pub)) {
		wl_arp_info_t *arpi = wl_get_arpi(wl, wlif);
		if (arpi && wl_arp_components_enab()) {
			if (wl_arp_send_proc(arpi, pkt) ==
				ARP_REPLY_HOST) {
				PKTFREE(wl->osh, pkt, TRUE);
				return 0;
			}
		}
	}
#endif /* ARPOE */

#ifdef TOE
	/* Apply TOE */
	if (TOE_ENAB(wl->pub))
		wl_toe_send_proc(wl->toei, pkt);
#endif // endif

	/* Fix the priority if WME is enabled */
	if (WME_ENAB(wl->pub) && (PKTPRIO(pkt) == 0))
		pktsetprio(pkt, FALSE);

#ifndef LINUX_POSTMOGRIFY_REMOVAL
	/* Mark this pkt as coming from host/bridge. */
	WLPKTTAG(pkt)->flags |= WLF_HOST_PKT;
#endif // endif

	wlc_sendpkt(wl->wlc, pkt, wlif->wlcif);

	return (0);
} /* wl_pktc_tx */

/* Used by GMAC3 forwarder for Intra BSS path, since lock is already taken in RX path. */
static int BCMFASTPATH
wl_start_int_try(wl_info_t *wl, wl_if_t *wlif, struct sk_buff *skb, bool lock_taken)
{
	void *pkt;

	WL_TRACE(("wl%d: wl_start: len %d data_len %d summed %d csum: 0x%x\n",
		wl->pub->unit, skb->len, skb->data_len, skb->ip_summed, (uint32)skb->csum));

	WL_LOCK_TRY(wl, lock_taken);

	/* Convert the packet. Mainly attach a pkttag */
	pkt = PKTFRMNATIVE(wl->osh, skb);
	ASSERT(pkt != NULL);

	wl_pktc_tx(wl, wlif, pkt);

	WL_UNLOCK_TRY(wl, lock_taken);

	return (0);
} /* wl_start_int */

/**
 * XXX If Linux flow control is enabled in WME mode, then for some
 * reason Video is dropped at effectively the same priority level as
 * Background.
 */
void
wl_txflowcontrol(wl_info_t *wl, struct wl_if *wlif, bool state, int prio)
{
	struct net_device *dev;

	ASSERT(prio == ALLPRIO);

	if (wlif == NULL)
		dev = wl->dev;
	else if (!wlif->dev_registered)
		return;
	else
		dev = wlif->dev;

	if (dev == NULL)
		return;

	if (state == ON)
		netif_stop_queue(dev);
	else
		netif_wake_queue(dev);
}

#if defined(AP) || defined(WL_ALL_PASSIVE) || defined(WL_MONITOR)
/** Schedule a completion handler to run at safe time */
int
wl_schedule_task(wl_info_t *wl, void (*fn)(struct wl_task *task), void *context)
{
	wl_task_t *task;

	WL_TRACE(("wl%d: wl_schedule_task\n", wl->pub->unit));

	if (!(task = MALLOC(wl->osh, sizeof(wl_task_t)))) {
		WL_ERROR(("wl%d: wl_schedule_task: out of memory, malloced %d bytes\n",
			wl->pub->unit, MALLOCED(wl->osh)));
		return -ENOMEM;
	}

	MY_INIT_WORK(&task->work, (work_func_t)fn);
	task->context = context;

	atomic_inc(&wl->callbacks);

	if (!SCHEDULE_WORK(wl, &task->work)) {
		WL_ERROR(("wl%d: schedule_work() failed\n", wl->pub->unit));
		MFREE(wl->osh, task, sizeof(wl_task_t));
		CALLBACK_DEC_AND_ASSERT(wl);
		return -ENOMEM;
	}

	return 0;
}
#endif /* defined(AP) || defined(WL_ALL_PASSIVE) || defined(WL_MONITOR) */

#if defined(BCM_NBUFF_WLMCAST_IPV6) && defined(WMF)

extern struct scb * BCMFASTPATH wlc_scbfind(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,
		const struct ether_addr *ea);
extern void *wl_wmf_get_igsc(wlc_bsscfg_t *bsscfg);

static void *wl_nic_hook_fn(int cmd, void *p, void *p2)
{
	struct net_device *device = p;
	wl_if_t *wlif = WL_DEV_IF(device);
	wlc_bsscfg_t *bsscfg = wl_bsscfg_find(wlif);
	if (!bsscfg) return NULL;
	switch (cmd) {
	    case WLEMF_CMD_GETIGSC:  /* get igsc instance */
	        if ((bsscfg->wmf_enable))
	            return wl_wmf_get_igsc(bsscfg);
	        break;
	    case WLEMF_CMD_SCBFIND:
	        /* find scb from associated device */
	        return wlc_scbfind(wlif->wl->wlc, bsscfg, p2);
	    default:
	        break;
	}
	return NULL;
}

void *wl_wmf_hooks_get(int cmd, void *p, void *p1, void *p2)
{
	switch (cmd)
	{
		case WLEMF_CMD_GETDEV:
			{
				wlc_bsscfg_t *bsscfg = ((wlc_bsscfg_t *)p);
				if (bsscfg->wlcif && bsscfg->wlcif->wlif) {
					return bsscfg->wlcif->wlif->dev;
				}
				return bsscfg->wlc->wl->dev;
			}
		case WLEMF_CMD_PKTDUP:
			{
				osl_t *osh = (osl_t *)p;
				return	PKTDUP(osh, p1);
			}
		case WLEMF_CMD_PKTFREE:
			{
				osl_t *osh = (osl_t *)p;
				bool *send = (bool *)p2;
				PKTFREE(osh, p1, *send);
				break;
			}
	}
	return NULL;
}
#endif /* BCM_NBUFF_WLMCAST_IPV6 */

/****************
priv_link is the private struct that we tell netdev about.  This in turn point to a wlif.

Newer kernels require that we call alloc_netdev to alloc the netdev and associated private space
from outside of our lock, which means we need to run it asynchronously in a thread but at
the same time common code wants us to return a pointer synchronously.

Answer is to add a layer of indirection so we MALLOC and return a wlif immediatly (with
wlif->dev = NULL and dev_registered = FALSE) and also spawn a thread to alloc a netdev
and priv_link for private space.  When the netdev_alloc() eventually completes and we hook it
all up.  netdev.priv contains (or points to) priv_link.  priv_link points to wlif.
wlif.dev points back to netdev.

The old way of having netdev.priv contain (or point to) wlif cannot work on newer kernels
since that was called from within our WL_LOCK perimeter lock and we would get a
'could sleep from atomic context' warning from the kernel.
*/

static struct wl_if *
wl_alloc_if(wl_info_t *wl, int iftype, uint index, struct wlc_if *wlcif)
{
	wl_if_t *wlif;
	wl_if_t *p;

	/* All kernels get a syncronous wl_if_t.  Older kernels get it populated
	   now, newer kernels get it populated async later
	 */
	if (!(wlif = MALLOCZ(wl->osh, sizeof(wl_if_t)))) {
		WL_ERROR(("wl%d: wl_alloc_if: out of memory, malloced %d bytes\n",
			(wl->pub)?wl->pub->unit:index, MALLOCED(wl->osh)));
		return NULL;
	}
	wlif->wl = wl;
	wlif->wlcif = wlcif;
	wlif->subunit = wl_get_free_ifidx(wl, iftype, index);
	if ((int)wlif->subunit < 0) {
		MFREE(wl->osh, wlif, sizeof(wl_if_t));
		WL_ERROR(("wl%d: wl_alloc_if: ifindex (%d) out of range\n",
			(wl->pub)?wl->pub->unit:index, wlif->subunit));
		return NULL;
	}

	if (wlcif)
		wlcif->index = wlif->subunit;

	wlif->if_type = iftype;

#ifdef BCM_NBUFF_WLMCAST_IPV6
	/* linkup wlcif->wlif to wlif */
	if (wlcif)
		wlcif->wlif = wlif;
#endif // endif
	/* add the interface to the interface linked list */
	if (wl->if_list == NULL)
		wl->if_list = wlif;
	else {
		p = wl->if_list;
		while (p->next != NULL)
			p = p->next;
		p->next = wlif;
	}

#ifdef ARPOE
	/* create and populate arpi for this IF */
	if (ARPOE_ENAB(wl->pub))
		wlif->arpi = wl_arp_alloc_ifarpi(wl->arpi, wlcif);
#endif /* ARPOE */
#ifdef BCM_NBUFF_WLMCAST_IPV6
#ifdef WMF
	if (iftype != WL_IFTYPE_MON)
		wlif->nic_hook_fn = wl_nic_hook_fn;
#else
	wlif->nic_hook_fn = NULL;
#endif // endif
#endif /* BCM_NBUFF_WLMCAST_IPV6 */
	return wlif;
} /* wl_alloc_if */

static void
wl_free_if(wl_info_t *wl, wl_if_t *wlif, bool rtnl_is_needed)
{
	wl_if_t *p;
	ASSERT(wlif);
	ASSERT(wl);

	WL_TRACE(("%s\n", __FUNCTION__));

#if defined(BCM_WFD)
	wl_wfd_unregisterdevice(wl->wfd_idx, wlif->dev);
#endif // endif
	/* check if register_netdev was successful */
	if (wlif->dev_registered) {
		ASSERT(wlif->dev);

#if defined(BCM_GMAC3)
		fwder_register(wlif->fwdh, NULL);
		wlif->fwdh = fwder_bind(wlif->fwdh, WL_FWDER_UNIT(wl), wlif->subunit,
		                        wlif->dev, FALSE);
#endif /* BCM_GMAC3 */

#ifdef HNDCTF
		if (wl->cih)
			ctf_dev_unregister(wl->cih, wlif->dev);
#endif /* HNDCTF */

		WL_TRACE(("%s: unregistering netdev %s\n", __FUNCTION__, wlif->dev->name));
		if (rtnl_is_needed)
			unregister_netdev(wlif->dev);
		else
			unregister_netdevice(wlif->dev);
		wlif->dev_registered = FALSE;

	}

	WL_LOCK(wl);
	/* remove the interface from the interface linked list */
	p = wl->if_list;
	if (p == wlif)
		wl->if_list = p->next;
	else {
		while (p != NULL && p->next != wlif)
			p = p->next;
		if (p != NULL)
			p->next = p->next->next;
	}
	WL_UNLOCK(wl);

#ifdef ARPOE
	/* free arpi for this IF */
	wl_arp_free_ifarpi(wlif->arpi);
#endif /* ARPOE */

#if defined(PKTC_TBL)
	/* BCM_PKTFWD: wl_pktfwd_wlif_del(wlif) to delete wlif into pktfwd */
	wl_pktc_free(wlif);
#endif // endif
	wl_free_ifidx(wl, wlif->subunit);
	MFREE(wl->osh, wlif, sizeof(wl_if_t));
} /* wl_free_if */

/**
 * Create a virtual interface. Call only from safe time!
 * can't call register_netdev with WL_LOCK
 *
 * Netdev allocator.  Only newer kernels need this to be async
 * but we'll run it async for all kernels for ease of maintenance.
 *
 * Sets:  wlif->dev & dev->priv_link->wlif
 */
static struct net_device *
wl_alloc_linux_if(wl_if_t *wlif)
{
	wl_info_t *wl = wlif->wl;
	struct net_device *dev;
	priv_link_t *priv_link;
	u32 priv_size = sizeof(priv_link_t);

	WL_TRACE(("%s\n", __FUNCTION__));
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24))
	dev = MALLOCZ(wl->osh, sizeof(struct net_device));
	if (!dev) {
		WL_ERROR(("wl%d: %s: malloc of net_device failed\n",
			(wl->pub)?wl->pub->unit:wlif->subunit, __FUNCTION__));
		return NULL;
	}
	ether_setup(dev);
	strncpy(dev->name, intf_name, IFNAMSIZ-1);
	dev->name[IFNAMSIZ-1] = '\0';

	priv_link = MALLOC(wl->osh, sizeof(priv_link_t));
	if (!priv_link) {
		WL_ERROR(("wl%d: %s: malloc of priv_link failed\n",
			(wl->pub)?wl->pub->unit:wlif->subunit, __FUNCTION__));
		MFREE(wl->osh, dev, sizeof(struct net_device));
		return NULL;
	}
	dev->priv = priv_link;
#else
	/* KERNEL >= 2.6.24 */
	/*
	 * Use osl_malloc for our own wlif priv area wl_if and use the netdev->priv area only
	 * as a pointer to our wl_if *.
	 */

	/* Allocate net device, including space for private structure */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 17, 0))
	dev = alloc_netdev(priv_size, intf_name, NET_NAME_UNKNOWN, ether_setup);
#else
	dev = alloc_netdev(priv_size, intf_name, ether_setup);
#endif // endif
	if (!dev) {
		WL_ERROR(("wl%d: %s: alloc_netdev failed\n",
			(wl->pub)?wl->pub->unit:wlif->subunit, __FUNCTION__));
		return NULL;
	}
	priv_link = netdev_priv(dev);
	if (!priv_link) {
		WL_ERROR(("wl%d: %s: cannot get netdev_priv\n",
			(wl->pub)?wl->pub->unit:wlif->subunit, __FUNCTION__));
		return NULL;
	}
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24) */

/* IFF_BCM_WLANDEV is added in netdevice.h in linux-4.1 under CONFIG_BCM_xxx */
#ifdef BCA_HNDROUTER
#if defined(CONFIG_BCM_KF_WL)
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0))
	dev->priv_flags |= IFF_BCM_WLANDEV;
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0) */
#endif /* CONFIG_BCM_KF_WL */
#endif /* BCA_HNDROUTER */
	/* Connect wlif and netdev */
	priv_link->wlif = wlif;
	wlif->dev = dev;

	/* match current flow control state */
	if (wlif->if_type != WL_IFTYPE_MON && wl->dev && netif_queue_stopped(wl->dev))
		netif_stop_queue(dev);

#if defined(BCM_BLOG)
	{
	    uint hwport = 0;
#if defined(BCM_WFD)
	    hwport = WLAN_NETDEVPATH_HWPORT(wl->unit, wlif->subunit);
#endif /* BCM_WFD */
	    netdev_path_set_hw_port(dev, hwport, BLOG_WLANPHY);
	}
#endif /* BCM_BLOG */

#if defined(PKTC_TBL)
	/* BCM_PKTFWD: wl_pktfwd_wlif_ins(wlif) to insert wlif into pktfwd */
	if (wl_pktc_init(wlif, dev) != 0)
		return NULL;

#if defined(BCM_PKTFWD)
	if (wlif->if_type == WL_IFTYPE_WDS) {
		netdev_wlan_set_dwds_ap(wlif->d3fwd_wlif);
	}
#endif /* BCM_PKTFWD */
#endif /* PKTC_TBL */

	return dev;
} /* wl_alloc_linux_if */

wlc_bsscfg_t *
wl_bsscfg_find(wl_if_t *wlif)
{
	wl_info_t *wl = wlif->wl;
	return wlc_bsscfg_find_by_wlcif(wl->wlc, wlif->wlcif);
}

/** !LINUXSIM specific */
static void
_wl_add_if(wl_task_t *task)
{
	wl_if_t *wlif = (wl_if_t *)task->context;
	wl_info_t *wl = wlif->wl;
	struct net_device *dev;
	wlc_bsscfg_t *cfg;
	int ifidx = 0;
#if defined(WL_CFG80211) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 0))
	uint8 role = WL_INTERFACE_TYPE_STA;
	int bssidx = 0;
	struct wl_if_event_info evnt_info;
	int err = 0;
#endif /* WL_CFG80211 && LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 0) */

	WL_TRACE(("%s\n", __FUNCTION__));

	/* During bsscfg allocation we schedule this function.
	 * After bsscfg allocation, if something bad happens
	 * (like no memory) we do not have any mechanism
	 * implemented to flush this function during bsscfg_init().
	 * Most of the flush API can sleep so we can not really use
	 * them from IOVAR context.
	 *
	 * If bsscfg_init() fails, we call bsscfg_free() which
	 * frees cfg and corresponding wlcif structure as well.
	 * So accessing cfg in the failuire case will result crash.
	 *
	 * wl_bsscfg_find() returns bsscfg based on wlcif and if wlcif
	 * is NULL it will return the primary bsscfg. So can not build
	 * a logic based on wl_bsscfg_find() return value.
	 */
	if (!wlif->wlcif)
		goto done;

	/* alloc_netdev and populate priv_link */
	if ((dev = wl_alloc_linux_if(wlif)) == NULL) {
		WL_ERROR(("%s: Call to  wl_alloc_linux_if failed\n", __FUNCTION__));
		goto done;
	}

	/* Copy temp to real name */
	ASSERT(strlen(wlif->name) > 0);
	strncpy(wlif->dev->name, wlif->name, strlen(wlif->name) + 1);

#if defined(WL_USE_NETDEV_OPS)
	dev->netdev_ops = &wl_netdev_ops;
#else /* WL_USE_NETDEV_OPS */
#ifdef WL_THREAD
	dev->hard_start_xmit = wl_start_wlthread;
#else
	dev->hard_start_xmit = wl_start;
#endif // endif
	dev->do_ioctl = wl_ioctl;
#if defined(USE_CFG80211)
	dev->set_mac_address = wl_cfg80211_set_mac_address;
#else
	dev->set_mac_address = wl_set_mac_address;
#endif // endif
	dev->get_stats = wl_get_stats;
#endif /* WL_USE_NETDEV_OPS */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
	dev->ethtool_ops = &wl_ethtool_ops;
#if defined(BCM_BLOG) && defined(CONFIG_BCM_KF_BLOG)
#if defined(PLATFORM_WITH_RUNNER)
	dev->wlan_client_get_info = wl_client_get_info;
#else
	dev->wlan_client_get_info = NULL;
#endif /* PLATFORM_WITH_RUNNER */
#endif /* BCM_BLOG && CONFIG_BCM_KF_BLOG */
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36) */


	cfg = wl_bsscfg_find(wlif);
	ASSERT(cfg != NULL);

	bcopy(&cfg->cur_etheraddr, dev->dev_addr, ETHER_ADDR_LEN);
	ifidx = wlif->subunit;
#if defined(WL_CFG80211)
	bssidx = WLC_BSSCFG_IDX(cfg);
	BCM_REFERENCE(bssidx);

	err = wl_cfg80211_enabled();
	if ((err != BCME_OK) ||
		((err == BCME_OK) && (strstr(wlif->name, "wds") != NULL))) {
		wl_register_interface((void *)wl, ifidx, dev, true);
	}
	else {
		if (wl_cfg80211_setup_ndev(wl->dev, dev, cfg->ID, ifidx) != 0) {
			WL_ERROR(("%s: Setup cfg80211 netdev AP failed. name=%s\n",
					__FUNCTION__, wlif->name));
			goto done;
		}
		if (wl_get_bsscfg_role(wl->wlc, cfg, wlif->wlcif->type, &role) == BCME_ERROR) {
			WL_ERROR(("%s: Failed to get the if role. name=%s\n",
					__FUNCTION__, wlif->name));
			goto done;
		}
		if (ifidx != 0) {
			if (wl_cfg80211_notify_ifadd(wl->dev, ifidx,
				wlif->dev->name, dev->dev_addr, ifidx, role) != BCME_OK) {
	/* interface create with wl utility needs this part of
	 * code to call the wl_cfg80211_post_ifcreate, to create wdev.
	 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 0)
				bzero(&evnt_info, sizeof(evnt_info));
				evnt_info.ifidx = ifidx;
				evnt_info.bssidx = ifidx;
				evnt_info.role = role;
				strncpy(evnt_info.name, wlif->dev->name, IFNAMSIZ);
				if (wl_cfg80211_post_ifcreate(wl->dev,
						&evnt_info, dev->dev_addr, NULL, true) != NULL) {
					/* Do the post interface create ops */
					WL_ERROR(("Post ifcreate ops done. Returning \n"));
					goto done;
				}
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 0) */
			}
		}
	}
#else
	wl_register_interface((void *)wl, ifidx, dev, true);
#endif /* WL_CFG80211 */
done:
	MFREE(wl->osh, task, sizeof(wl_task_t));
	CALLBACK_DEC_AND_ASSERT(wl);
} /* _wl_add_if */

/** Schedule _wl_add_if() to be run at safe time. */
struct wl_if *
wl_add_if(wl_info_t *wl, struct wlc_if *wlcif, uint index, struct ether_addr *remote)
{
	wl_if_t *wlif;
	int iftype;
	const char *devname;
	WL_TRACE(("%s\n", __FUNCTION__));
	if (remote) {
		iftype = WL_IFTYPE_WDS;
		devname = "wds";
	} else {
		iftype = WL_IFTYPE_BSS;
		devname = "wl";
	}

	wlif = wl_alloc_if(wl, iftype, index, wlcif);

	if (!wlif) {
		WL_ERROR(("wl%d: wl_add_if: failed to create %s interface %d\n",
				wl->pub->unit, (remote)?"WDS":"BSS", index));
		return NULL;
	}
#if defined(USE_CFG80211)
	if (wl_cfg80211_get_the_vif_name(wl->dev, wlif->name) != BCME_OK)
#endif // endif
	{
			wlc_bsscfg_t *cfg = wlc_bsscfg_find_by_wlcif(wl->wlc, wlcif);
			ASSERT(cfg != NULL);
		/* netdev isn't ready yet so stash name here for now and
		   copy into netdev when it becomes ready
		   */
		if (remote) {
			/* assigning the ifidx by default */
			wlif->wds_index = wlif->subunit;
#if defined(WDS)
			wlif->wds_index = wlc_update_wds_index(cfg, wlif, TRUE);
			if (wlif->wds_index < 0) {
				WL_ERROR(("%s wds count(%d) update Failed\n",
						__FUNCTION__, wlif->wds_index));
				return NULL;
			}
#endif /* WDS */
			(void)snprintf(wlif->name, sizeof(wlif->name), "%s%d.%d.%d",
				devname, wl->pub->unit, WLC_BSSCFG_IDX(cfg), wlif->wds_index);
		} else {
			(void)snprintf(wlif->name, sizeof(wlif->name),
					"%s%d.%d", devname, wl->pub->unit, WLC_BSSCFG_IDX(cfg));
		}
	}
	if (wl_schedule_task(wl, _wl_add_if, wlif)) {
		MFREE(wl->osh, wlif, sizeof(wl_if_t) + sizeof(struct net_device));
		return NULL;
	}

	return wlif;
} /* wl_add_if */

/** Remove a virtual interface. Call only from safe time! */
static void
_wl_del_if(wl_task_t *task)
{
	wl_if_t *wlif = (wl_if_t *) task->context;
	wl_info_t *wl = wlif->wl;
	unsigned long wait_dev_reg_timeout = jiffies + (HZ * 5);
#if defined(USE_CFG80211)
	struct bcm_cfg80211 *cfg = wiphy_priv(wl->wiphy);
	bool expected = FALSE;
	int ifidx;
	uint8 bssidx;
#endif

	printf("wl%d: set timeout 5 secs to wait dev reg finish\n", wl->unit);
	while (!wlif->dev_registered &&	time_after(wait_dev_reg_timeout, jiffies))
		schedule();

	if (!wlif->dev_registered)
		printf("wl%d: dev %p reg failed\n", wl->unit, wlif->dev);

#if defined(USE_CFG80211)
	ASSERT(cfg);

	if (wlif->dev->ieee80211_ptr) {
		struct net_info *ni;

		ni = wl_get_netinfo_by_wdev(cfg, wlif->dev->ieee80211_ptr);
		ifidx = ni->ifidx;
		bssidx = ni->bssidx;

		expected = wl_cfg80211_ifdel_expected(cfg, ifidx, bssidx);
	}

	/*
	 * if nl80211 interface delete is pending we do not need
	 * locking and just notify the waiting process delete is done.
	 */
	if (expected) {
		wl_free_if(wl, wlif, FALSE);
		wl_cfg80211_notify_ifdel(cfg, ifidx, bssidx);
	} else {
		rtnl_lock();
		mutex_lock(&cfg->if_sync);
		wl_free_if(wl, wlif, FALSE);
		mutex_unlock(&cfg->if_sync);
		rtnl_unlock();
	}
#else
	wl_free_if(wl, wlif, TRUE);
#endif /* USE_CFG80211 */

	MFREE(wl->osh, task, sizeof(wl_task_t));
	CALLBACK_DEC_AND_ASSERT(wl);
}

#ifdef WLRSDB
/** RSDB specific function. Update the wl pointer for RSDB bsscfg Move */
void
wl_update_if(struct wl_info *from_wl, struct wl_info *to_wl, wl_if_t *from_wlif,
	struct wlc_if *to_wlcif)
{
	ASSERT(to_wl != NULL);
	ASSERT(to_wlcif != NULL);
	if (from_wlif) {
		from_wlif->wl = to_wl;
		from_wlif->wlcif = to_wlcif;
	}
#ifdef WL_DUALNIC_RSDB
	else {
		struct wlc_if *from_wlcif;
		struct net_device *dev;
		/* XXX Swap the interface netdev structure
		 * Is there any other better way to handle this?
		 */
		from_wlif = ((priv_link_t*)netdev_priv(from_wl->dev))->wlif;
		from_wlif->wl = to_wl;
		from_wlcif = from_wlif->wlcif;
		from_wlif->wlcif = to_wlcif;

		from_wlif = ((priv_link_t*)netdev_priv(to_wl->dev))->wlif;
		from_wlif->wl = from_wl;
		from_wlif->wlcif = from_wlcif;

		dev = from_wl->dev;
		from_wl->dev = to_wl->dev;
		to_wl->dev = dev;
	}
#endif /* WL_DUALNIC_RSDB */
} /* wl_update_if */
#endif /* WLRSDB */

/** Schedule _wl_del_if() to be run at safe time. */
void
wl_del_if(wl_info_t *wl, wl_if_t *wlif)
{
	ASSERT(wlif != NULL);
	ASSERT(wlif->wl == wl);

	wlif->wlcif = NULL;

	if (wl_schedule_task(wl, _wl_del_if, wlif)) {
		WL_ERROR(("wl%d: wl_del_if: schedule_task() failed\n", wl->pub->unit));
		return;
	}
}

bool
wl_max_if_reached(wl_info_t *wl)
{
	return FALSE;
}

/** Return pointer to interface name */
char *
wl_ifname(wl_info_t *wl, wl_if_t *wlif)
{
	if (wlif) {
		return wlif->name;
	} else {
		return wl->dev->name;
	}
}

void
wl_init(wl_info_t *wl)
{
	WL_TRACE(("wl%d: wl_init\n", wl->pub->unit));

	wl_reset(wl);

	wlc_init(wl->wlc);
}

uint
wl_reset(wl_info_t *wl)
{
	uint32 macintmask;

	WL_TRACE(("wl%d: wl_reset\n", wl->pub->unit));

	/* disable interrupts */
	macintmask = wl_intrsoff(wl);

	wlc_reset(wl->wlc);

	/* restore macintmask */
	wl_intrsrestore(wl, macintmask);

	/* dpc will not be rescheduled */
	wl->resched = 0;

	return (0);
}

/**
 * These are interrupt on/off entry points. Disable interrupts
 * during interrupt state transition.
 */
void BCMFASTPATH
wl_intrson(wl_info_t *wl)
{
	unsigned long flags = 0;

	INT_LOCK(wl, flags);
	wlc_intrson(wl->wlc);
#ifdef STB_SOC_WIFI
	wl_stbsoc_enable_intrs(wl->plat_info);
#endif /* STB_SOC_WIFI */
	INT_UNLOCK(wl, flags);
}

bool
wl_alloc_dma_resources(wl_info_t *wl, uint addrwidth)
{
	return TRUE;
}

uint32 BCMFASTPATH
wl_intrsoff(wl_info_t *wl)
{
	unsigned long flags = 0;
	uint32 status;

	INT_LOCK(wl, flags);
	status = wlc_intrsoff(wl->wlc);
#ifdef STB_SOC_WIFI
	wl_stbsoc_disable_intrs(wl->plat_info);
#endif /* STB_SOC_WIFI */
	INT_UNLOCK(wl, flags);
	return status;
}

void
wl_intrsrestore(wl_info_t *wl, uint32 macintmask)
{
	unsigned long flags = 0;

	INT_LOCK(wl, flags);
	wlc_intrsrestore(wl->wlc, macintmask);
	INT_UNLOCK(wl, flags);
}

int
wl_up(wl_info_t *wl)
{
	int error = 0;
	wl_if_t *wlif;

	WL_TRACE(("wl%d: wl_up\n", wl->pub->unit));

	if (wl->pub->up)
		return (0);

	error = wlc_up(wl->wlc);

	/* wake (not just start) all interfaces */
	if (!error) {
		for (wlif = wl->if_list; wlif != NULL; wlif = wlif->next) {
			wl_txflowcontrol(wl, wlif, OFF, ALLPRIO);
		}
	}

#ifdef NAPI_POLL
	set_bit(__LINK_STATE_START, &wl->dev->state);
#endif // endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0)
	/* use d11 h/w rng to add some entropy to linux. */
#define WL_LINUX_RBUF_SZ 16
	if (!error) {
		uint8 rbuf[WL_LINUX_RBUF_SZ];
		if (wlc_getrand(wl->wlc, rbuf, sizeof(rbuf)) == BCME_OK) {
			WL_WSEC(("wl%d: updating linux rng w/ wlc random data\n", wl->pub->unit));
			add_device_randomness(rbuf, sizeof(rbuf));
		}
	}
#endif /* linuxver >= 3.10.0 */

	return (error);
} /* wl_up */

void
wl_down(wl_info_t *wl)
{
	wl_if_t *wlif;
	int monitor = 0;
	uint ret_val = 0;
	int callbacks;

	WL_TRACE(("wl%d: wl_down\n", wl->pub->unit));

	for (wlif = wl->if_list; wlif != NULL; wlif = wlif->next) {
#if defined(BCM_GMAC3)
		/* Flush all stations to interface bindings. */
		if (wlif->fwdh != FWDER_NULL) {
			fwder_flush(wlif->fwdh, (uintptr_t)wlif->dev);
		}
#endif /* BCM_GMAC3 */
		if (wlif->dev) {
			netif_down(wlif->dev);
			netif_stop_queue(wlif->dev);
		}
	}

	if (wl->monitor_dev) {
		ret_val = wlc_ioctl(wl->wlc, WLC_SET_MONITOR, &monitor, sizeof(int), NULL);
		if (ret_val != BCME_OK) {
			WL_ERROR(("%s: Disabling MONITOR failed %d\n", __FUNCTION__, ret_val));
		}
	}

	/* call common down function */
	if (wl->wlc)
		ret_val = wlc_down(wl->wlc);

	callbacks = atomic_read(&wl->callbacks) - ret_val;
	BCM_REFERENCE(callbacks);

	WL_UNLOCK(wl);
	/* wait for down callbacks to complete */

#ifdef WL_ALL_PASSIVE
	if (WL_ALL_PASSIVE_ENAB(wl)) {
		int i = 0;
		for (i = 0; (atomic_read(&wl->callbacks) > callbacks) && i < 10000; i++) {
			schedule();
			flush_scheduled_work();
		}
	} else
#endif /* WL_ALL_PASIVE */
	{
		/* For HIGH_only driver, it's important to actually schedule other work,
		 * not just spin wait since everything runs at schedule level
		 */
		SPINWAIT((atomic_read(&wl->callbacks) > callbacks), 100 * 1000);
	}

	WL_LOCK(wl);
} /* wl_down */

static int
write_file(const char * file_name, uint32 flags, uint8 *buf, int size)
{
	int ret = 0;
	struct file *fp = NULL;
	mm_segment_t old_fs;
	loff_t pos = 0;
	/* change to KERNEL_DS address limit */
	old_fs = get_fs();
	set_fs(KERNEL_DS);

	/* open file to write */
	fp = filp_open(file_name, flags, 0664);
	if (IS_ERR(fp)) {
		printf("%s: open file error, err = %ld\n", __FUNCTION__, PTR_ERR(fp));
		ret = -1;
		goto exit;
	}

	/* Write buf to file */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
	ret = kernel_write(fp, buf, size, &pos);
	if (ret < 0) {
		WL_ERROR(("write file error, err = %d\n", ret));
		goto exit;
	}
	ret = BCME_OK;
#else
	fp->f_op->write(fp, buf, size, &pos);
#endif // endif

exit:
	/* close file before return */
	if (!IS_ERR(fp))
		filp_close(fp, current->files);

	/* restore previous address limit */
	set_fs(old_fs);

	return ret;
}

#define DUMPMAC_BUF_SZ (384 * 1024)
#define DUMPMAC_FILENAME_SZ 48
#define DUMPMAC_ARGS_SZ 128

static int
_wl_macdbg_write_file(wl_info_t *wl, char* dumpname, char *dumpbuf)
{
	char dumpfilename[DUMPMAC_FILENAME_SZ] = {0, };
	int res = BCME_OK;

	snprintf(dumpfilename, DUMPMAC_FILENAME_SZ,
		"/tmp/dump_%s_%04x.txt", dumpname, wl->dump_signature);
	WL_PRINT(("%s: wl dump %s to %s len %d bytes\n", __FUNCTION__,
		dumpname, dumpfilename, (int)strlen(dumpbuf)));
	/* Write to a file */
	if (write_file(dumpfilename, (O_CREAT | O_WRONLY | O_SYNC),
		dumpbuf, (int)strlen(dumpbuf))) {
		WL_ERROR(("%s: writing dump %s to the file failed\n",
			__FUNCTION__, dumpname));
		res = BCME_ERROR;
	}

	return res;
}

static int
_wl_macdbg_dump_name(wl_info_t *wl, char* dumpname, char* args, char *dumpbuf)
{
	char dumpargs[DUMPMAC_ARGS_SZ] = {0, };
	int res;

	if (args) {
		snprintf(dumpargs, DUMPMAC_ARGS_SZ, "%s %s", dumpname, args);
	} else {
		snprintf(dumpargs, DUMPMAC_ARGS_SZ, "%s", dumpname);
	}

	WL_LOCK(wl);
	res = wlc_iovar_op(wl->wlc, "dump", dumpargs, (int)strlen(dumpargs),
		dumpbuf, DUMPMAC_BUF_SZ, IOV_GET, NULL);
	WL_UNLOCK(wl);

	if (res == BCME_OK) {
		_wl_macdbg_write_file(wl, dumpname, dumpbuf);

		memset(dumpbuf, 0, DUMPMAC_BUF_SZ);
	} else {
		WL_ERROR(("%s: dump %s fails %d\n", __FUNCTION__, dumpname, res));
	}
	return res;
}

static void
_wl_sched_macdbg_dump(wl_task_t *task)
{
	wl_info_t *wl = (wl_info_t *)task->context;
	char *p, *dumpbuf = NULL;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36))
	struct task_struct *tsk;
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36) */

	int dumpnum = 0;

	dumpbuf = (char *)MALLOCZ(wl->osh, DUMPMAC_BUF_SZ);
	if (dumpbuf == NULL) {
		WL_ERROR(("%s: fail to malloc dumpbuf for %d bytes\n",
			__FUNCTION__, DUMPMAC_BUF_SZ));
		goto exit;
	}

	/* Marked under psmwd state */
	wlc_set_psm_watchdog_debug(wl->wlc, TRUE);

	/* PSMr dump */
	if (_wl_macdbg_dump_name(wl, "mac", NULL /* args */, dumpbuf) == BCME_OK) {
		dumpnum++;
	}

#if defined(WL_PSMR1)
	if (PSMR1_ENAB(wl->pub)) {
		/* PSMr1 dump */
		if (_wl_macdbg_dump_name(wl, "mac1", NULL, dumpbuf) == BCME_OK) {
			dumpnum++;
		}
	}
#endif /* WL_PSMR1 */

	/* PSMx dump */
	if (_wl_macdbg_dump_name(wl, "macx", NULL, dumpbuf) == BCME_OK) {
		dumpnum++;
	}

	/* ratelinkmem dump */
	if (_wl_macdbg_dump_name(wl, "ratelinkmem", "-I0,255 -r -p", dumpbuf) == BCME_OK) {
		dumpnum++;
	}

	/* sctpl dump */
	if (_wl_macdbg_dump_name(wl, "sctpl", NULL, dumpbuf) == BCME_OK) {
		dumpnum++;
	}

	if (_wl_macdbg_dump_name(wl, "bmc", NULL, dumpbuf) == BCME_OK) {
		dumpnum++;
	}

	/* vasip_counters dump */
	if (_wl_macdbg_dump_name(wl, "vasip_counters", NULL, dumpbuf) == BCME_OK) {
		dumpnum++;
	}

	/* physvmp dump */
	if (_wl_macdbg_dump_name(wl, "physvmp", NULL, dumpbuf) == BCME_OK) {
		dumpnum++;
	}

	/* peruser dump */
	if (_wl_macdbg_dump_name(wl, "peruser", NULL, dumpbuf) == BCME_OK) {
		dumpnum++;
	}

	/* pkt_hist dump */
	if (_wl_macdbg_dump_name(wl, "pkt_hist", NULL, dumpbuf) == BCME_OK) {
		dumpnum++;
	}

	/* saved dma dump result */
	p = wlc_macdbg_get_dmadump_buf(wl->wlc);
	if (p && strlen(p) > 0) {
		_wl_macdbg_write_file(wl, "dma", p);
	}

	/* Clear under psmwd state */
	wlc_set_psm_watchdog_debug(wl->wlc, FALSE);

	WL_PRINT(("%s: dump signature: %04x, dumpnum: %d\n",
		__FUNCTION__, wl->dump_signature, dumpnum));
	MFREE(wl->osh, dumpbuf, DUMPMAC_BUF_SZ);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36))
	/* check if debug_monitor deamon is up and running */
	for_each_process(tsk) {
		if (strncmp("debug_monitor", tsk->comm, 11) == 0) {
			WL_PRINT(("%s: send signal to debug_monitor\n", __FUNCTION__));
			send_sig_info(SIGUSR1, (void *)1L, tsk);
			break;
		}
	}
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36) */
exit:
	MFREE(wl->osh, task, sizeof(wl_task_t));
	CALLBACK_DEC_AND_ASSERT(wl);
}

void
wl_sched_macdbg_dump(wl_info_t *wl)
{
	if (wl_schedule_task(wl, _wl_sched_macdbg_dump, wl)) {
		return;
	}
}

#ifdef BCMDBG
#define DTRACE_MAX_FILESIZE	(512 * 1024)	/* 512KB */
#define DTRACE_MAX_FILENUM	2		/* keep it power of 2 */

typedef struct _dtrace_task_t {
	wl_info_t *wl;
	uint16 datalen;
	uint8 event_data[1];
} dtrace_task_t;

static void
_wl_sched_dtrace(wl_task_t *task)
{
	dtrace_task_t *dtrace_task = task->context;
	wl_info_t *wl = dtrace_task->wl;
	char *dumpbuf = NULL;
	int res, dumplen, datalen = 0, i = 0, j;
	struct bcmstrbuf *b = NULL;
	struct bcmstrbuf bcmstrbuf;
	uint32 fifleflags = (O_CREAT | O_WRONLY | O_SYNC | O_APPEND);
	char dumpfilename[DUMPMAC_FILENAME_SZ] = {0};
	bcm_xtlv_t *dtrace_rec;

	dumpbuf = (char *)MALLOCZ(wl->osh, DUMPMAC_BUF_SZ);
	if (dumpbuf == NULL) {
		WL_ERROR(("%s: dumpbuf malloc failed for 0x%x bytes\n",
			__FUNCTION__, DUMPMAC_BUF_SZ));
		goto exit;
	}

	bcm_binit(&bcmstrbuf, dumpbuf, DUMPMAC_BUF_SZ);
	b = &bcmstrbuf;

	while (datalen < dtrace_task->datalen) {
		dtrace_rec = (bcm_xtlv_t *)(dtrace_task->event_data + datalen);
		bcm_bprintf(b, "[%02d] %01X ", i, dtrace_rec->id);
		for (j = 0; j < dtrace_rec->len; j++) {
			bcm_bprintf(b, "%02X", dtrace_rec->data[j]);
		}
		bcm_bprintf(b, "\n");
		i++;
		datalen += (dtrace_rec->len + BCM_XTLV_HDR_SIZE);
	}
	ASSERT(datalen == dtrace_task->datalen);
	dumplen = strlen(dumpbuf);

	if (wl->dtrace_cur_fsz >= DTRACE_MAX_FILESIZE) {
		/* if current dtrace dump file is too big, stop growing it and create another */
		fifleflags = (O_CREAT | O_WRONLY | O_SYNC | O_TRUNC);
		wl->dtrace_cur_fidx = (wl->dtrace_cur_fidx + 1) & (DTRACE_MAX_FILENUM - 1);
		wl->dtrace_cur_fsz = 0;
		WL_INFORM(("%s: switch to new dtrace file idx %d\n", __FUNCTION__,
			wl->dtrace_cur_fidx));
	}

	snprintf(dumpfilename, DUMPMAC_FILENAME_SZ, "/tmp/dump_dtrace%1x_%04x.txt",
		wl->dtrace_cur_fidx, wl->dump_signature);

	if ((res = write_file(dumpfilename, fifleflags, dumpbuf, dumplen))) {
		WL_ERROR(("%s: dtrace dump failed %d\n",
			__FUNCTION__, res));
	} else {
		wl->dtrace_cur_fsz += dumplen;
		WL_INFORM(("%s: dumplen %d cur_fsz %d\n", __FUNCTION__,
			dumplen, wl->dtrace_cur_fsz));
	}
	MFREE(wl->osh, dumpbuf, DUMPMAC_BUF_SZ);
exit:
	MFREE(wl->osh, dtrace_task, OFFSETOF(dtrace_task_t, event_data) + datalen);
	MFREE(wl->osh, task, sizeof(wl_task_t));
	CALLBACK_DEC_AND_ASSERT(wl);
}

void
wl_sched_dtrace(wl_info_t *wl, uint8 *event_data, uint16 datalen)
{
	int res;
	dtrace_task_t *dtrace_task = MALLOCZ(wl->osh,
		OFFSETOF(dtrace_task_t, event_data) + datalen);
	if (dtrace_task == NULL) {
		WL_ERROR(("%s: malloc fail for dtrace_task\n", __FUNCTION__));
		return;
	}
	dtrace_task->wl = wl;
	dtrace_task->datalen = (uint16)datalen;
	memcpy(&dtrace_task->event_data, event_data, datalen);

	if ((res = wl_schedule_task(wl, _wl_sched_dtrace, dtrace_task))) {
		WL_ERROR(("%s: task schedule failes %d\n", __FUNCTION__, res));
		MFREE(wl->osh, dtrace_task, OFFSETOF(dtrace_task_t, event_data) + datalen);
		return;
	}
}
#endif /* BCMDBG */

/* Retrieve current toe component enables, which are kept as a bitmap in toe_ol iovar */
static int
wl_toe_get(wl_info_t *wl, uint32 *toe_ol)
{
	if (wlc_iovar_getint(wl->wlc, "toe_ol", toe_ol) != 0)
		return -EOPNOTSUPP;

	return 0;
}

/* Set current toe component enables in toe_ol iovar, and set toe global enable iovar */
static int
wl_toe_set(wl_info_t *wl, uint32 toe_ol)
{
	if (wlc_iovar_setint(wl->wlc, "toe_ol", toe_ol) != 0)
		return -EOPNOTSUPP;

	/* Enable toe globally only if any components are enabled. */

	if (wlc_iovar_setint(wl->wlc, "toe", (toe_ol != 0)) != 0)
		return -EOPNOTSUPP;

	return 0;
}

static void
wl_get_driver_info(struct net_device *dev, struct ethtool_drvinfo *info)
{
	wl_info_t *wl = WL_INFO_GET(dev);

#if WIRELESS_EXT >= 19 || LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 29)
	if (!wl || !wl->pub || !wl->wlc || !wl->dev)
		return;
#endif // endif
	bzero(info, sizeof(struct ethtool_drvinfo));
	(void)snprintf(info->driver, sizeof(info->driver), "wl%d", wl->pub->unit);
	strncpy(info->version, EPI_VERSION_STR, sizeof(info->version));
	info->version[(sizeof(info->version))-1] = '\0';
}

#ifdef WLCSO
static int
wl_set_tx_csum(struct net_device *dev, uint32 on_off)
{
	wl_info_t *wl = WL_INFO_GET(dev);

	wlc_set_tx_csum(wl->wlc, on_off);
	if (on_off)
		dev->features |= NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM;
	else
		dev->features &= ~(NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM);

	return 0;
}
#endif // endif

static int
wl_ethtool(wl_info_t *wl, void *uaddr, wl_if_t *wlif)
{
	struct ethtool_drvinfo info;
	struct ethtool_value edata;
	uint32 cmd;
	uint32 toe_cmpnt = 0, csum_dir;
	int ret;

	if (!wl || !wl->pub || !wl->wlc)
		return -ENODEV;

#ifndef BCMQT
	/* skip this trace in emulator builds since it happens every second */
	WL_TRACE(("wl%d: %s\n", wl->pub->unit, __FUNCTION__));
#endif // endif

	if (copy_from_user(&cmd, uaddr, sizeof(uint32)))
		return (-EFAULT);

	switch (cmd) {
	case ETHTOOL_GDRVINFO:
		if (!wl->dev)
			return -ENETDOWN;

		wl_get_driver_info(wl->dev, &info);
		info.cmd = cmd;
		if (copy_to_user(uaddr, &info, sizeof(info)))
			return (-EFAULT);
		break;

	/* Get toe offload components */
	case ETHTOOL_GRXCSUM:
	case ETHTOOL_GTXCSUM:
		if ((ret = wl_toe_get(wl, &toe_cmpnt)) < 0)
			return ret;

		csum_dir = (cmd == ETHTOOL_GTXCSUM) ? TOE_TX_CSUM_OL : TOE_RX_CSUM_OL;

		edata.cmd = cmd;
		edata.data = (toe_cmpnt & csum_dir) ? 1 : 0;

		if (copy_to_user(uaddr, &edata, sizeof(edata)))
			return (-EFAULT);
		break;

	/* Set toe offload components */
	case ETHTOOL_SRXCSUM:
	case ETHTOOL_STXCSUM:
		if (copy_from_user(&edata, uaddr, sizeof(edata)))
			return (-EFAULT);

		/* Read the current settings, update and write back */
		if ((ret = wl_toe_get(wl, &toe_cmpnt)) < 0)
			return ret;

		csum_dir = (cmd == ETHTOOL_STXCSUM) ? TOE_TX_CSUM_OL : TOE_RX_CSUM_OL;

		if (edata.data != 0)
			toe_cmpnt |= csum_dir;
		else
			toe_cmpnt &= ~csum_dir;

		if ((ret = wl_toe_set(wl, toe_cmpnt)) < 0)
			return ret;

		/* If setting TX checksum mode, tell Linux the new mode */
		if (cmd == ETHTOOL_STXCSUM) {
			if (!wl->dev)
				return -ENETDOWN;
			if (edata.data)
				wl->dev->features |= NETIF_F_IP_CSUM;
			else
				wl->dev->features &= ~NETIF_F_IP_CSUM;
		}

		break;

	default:
		return (-EOPNOTSUPP);

	}

	return (0);
} /* wl_ethtool */

int
wl_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	int ret, oslerr = 0;
	ret = wl_ioctl_entry(dev, ifr, cmd);
	if (
#if defined(USE_IW)
		((cmd >= SIOCIWFIRST) && (cmd <= SIOCIWLAST)) ||
#endif /* USE_IW */
		(cmd == SIOCETHTOOL)) {
		oslerr = ret;
	} else {
		oslerr = OSL_ERROR(ret);
	}
	return oslerr;
}

int
wl_ioctl_entry(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	wl_info_t *wl = NULL;
	wl_if_t *wlif = NULL;
	void *buf = NULL;
	wl_ioctl_t ioc;
	int bcmerror;
#ifdef WLRSDB
	int lock_all_wl = 0;
	int idx;
	wl_info_t *wl_iter;
#endif /* WLRSDB */
	bool is_kernel_ds = FALSE;

	if (dev) {
		wl = WL_INFO_GET(dev);
		wlif = WL_DEV_IF(dev);
	}
	if (!dev || wlif == NULL || wl == NULL || wl->dev == NULL) {
		/* return OSL_ERROR for ethtool and iw tool commands */
		if (
#if defined(USE_IW)
			((cmd >= SIOCIWFIRST) && (cmd <= SIOCIWLAST)) ||
#endif /* USE_IW */
			(cmd == SIOCETHTOOL)) {
			return -ENETDOWN;
		} else {
			return BCME_NODEVICE;
		}
	}

	bcmerror = 0;

#ifdef BCMQT
	if (cmd != SIOCETHTOOL) {
		/* skip this trace in emulator builds since it happens every second */
		WL_TRACE(("wl%d: wl_ioctl: cmd 0x%x\n", wl->pub->unit, cmd));
	}
#else
	WL_TRACE(("wl%d: wl_ioctl: cmd 0x%x\n", wl->pub->unit, cmd));
#endif // endif

#ifdef CONFIG_PREEMPT
	if (preempt_count())
		WL_ERROR(("wl%d: wl_ioctl: cmd = 0x%x, preempt_count=%d\n",
			wl->pub->unit, cmd, preempt_count()));
#endif // endif

#ifdef USE_IW
	/* linux wireless extensions */
	if ((cmd >= SIOCIWFIRST) && (cmd <= SIOCIWLAST)) {
		/* may recurse, do NOT lock */
		return wl_iw_ioctl(dev, ifr, cmd);
	}
#endif /* USE_IW */

	if (cmd == SIOCETHTOOL)
		return (wl_ethtool(wl, (void*)ifr->ifr_data, wlif));

	switch (cmd) {
		case SIOCDEVPRIVATE :
			break;
		default:
			bcmerror = BCME_UNSUPPORTED;
			goto done2;
	}

	is_kernel_ds = segment_eq(get_fs(), KERNEL_DS);
#ifdef CONFIG_COMPAT
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 6, 0))
	if (!is_kernel_ds && in_compat_syscall()) {
#else
	if (!is_kernel_ds && is_compat_task()) {
#endif /* LINUX_VER >= 4.6 */
		compat_wl_ioctl_t compat_ioc;
		if (copy_from_user(&compat_ioc, ifr->ifr_data, sizeof(compat_wl_ioctl_t))) {
			bcmerror = BCME_BADADDR;
			goto done2;
		}
		ioc.cmd = compat_ioc.cmd;
		ioc.buf = compat_ptr(compat_ioc.buf);
		ioc.len = compat_ioc.len;
		ioc.set = compat_ioc.set;
		ioc.used = compat_ioc.used;
		ioc.needed = compat_ioc.needed;
	} else
#endif /* CONFIG_COMPAT */
	if (is_kernel_ds) {
		memcpy(&ioc, ifr->ifr_data, sizeof(wl_ioctl_t));
	}
	else if (copy_from_user(&ioc, ifr->ifr_data, sizeof(wl_ioctl_t))) {
		bcmerror = BCME_BADADDR;
		goto done2;
	}

	/* optimization for direct ioctl calls from kernel */
	if (segment_eq(get_fs(), KERNEL_DS))
		buf = ioc.buf;

	else if (ioc.buf) {
		if (!(buf = (void *) MALLOC(wl->osh, MAX(ioc.len, WLC_IOCTL_MAXLEN)))) {
			bcmerror = BCME_NORESOURCE;
			goto done2;
		}

		if (copy_from_user(buf, ioc.buf, ioc.len)) {
			bcmerror = BCME_BADADDR;
			goto done1;
		}
	}

	WL_LOCK(wl);
#ifdef WLRSDB
	/*
	 * oper_mode iovar require both the perimeter locks to be taken. This iovar checks the
	 * possibility of upgrading to MIMO and if that happens will bring down the second wlc.
	 * So, locking the second wl is also required.
	 */

	if (wl->cmn && wl->cmn->wl[1] && (wl == wl->cmn->wl[0])) {
		if ((buf && !strcmp((char *)buf, "oper_mode")) ||
			((ioc.cmd == WLC_UP) || (ioc.cmd == WLC_DOWN) ||
			(ioc.cmd == WLC_OUT))) {
			lock_all_wl = 1;
		}
	}
	if (lock_all_wl) {
		FOREACH_WL(wl->cmn, idx, wl_iter) {
			if (wl_iter != wl) {
				WL_LOCK(wl_iter);
			}
		}
	}
#endif /* WLRSDB */
	if (!capable(CAP_NET_ADMIN)) {
		bcmerror = BCME_EPERM;
	} else {
		bcmerror = wlc_ioctl(wl->wlc, ioc.cmd, buf, ioc.len, wlif->wlcif);
	}
#ifdef WLRSDB
	if (lock_all_wl) {
		FOREACH_WL(wl->cmn, idx, wl_iter) {
			if (wl_iter != wl) {
				WL_UNLOCK(wl_iter);
			}
		}
	}
#endif /* WLRSDB */
	WL_UNLOCK(wl);

done1:
	if (ioc.buf && (ioc.buf != buf)) {
		if (copy_to_user(ioc.buf, buf, ioc.len))
			bcmerror = BCME_BADADDR;
		MFREE(wl->osh, buf, MAX(ioc.len, WLC_IOCTL_MAXLEN));
	}

done2:
	ASSERT(VALID_BCMERROR(bcmerror));
	if (bcmerror != 0)
		wl->pub->bcmerror = bcmerror;

	return bcmerror;
} /* wl_ioctl */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
static STATS64_RETURN_TYPE
wl_get_stats64(struct net_device *dev, struct rtnl_link_stats64 *stats)
{
	wl_info_t *wl;
	wl_if_t *wlif;
	wl_if_stats_t wlif_stats;

	/* In case of linux 2.6.36, we should never return NULL
	 * even in case of failure. Otherwise it would result NULL
	 * pointer dereference within kernel.
	 * Recent kernel version handles this although
	 */
	if (!dev)
	    goto fail;

	if ((wl = WL_INFO_GET(dev)) == NULL)
	    goto fail;

	if ((wlif = WL_DEV_IF(dev)) == NULL)
	    goto fail;

	if (wlif->wlcif == NULL)
	    goto fail;

	memset(&wlif_stats, 0, sizeof(wl_if_stats_t));
	wlc_wlcif_stats_get(wl->wlc, wlif->wlcif, &wlif_stats);

	stats->rx_packets = WLCNTVAL(wlif_stats.rxframe);
	stats->tx_packets = WLCNTVAL(wlif_stats.txframe);
	stats->rx_bytes = WLCNTVAL(wlif_stats.rxbyte);
	stats->tx_bytes = WLCNTVAL(wlif_stats.txbyte);
	stats->rx_errors = WLCNTVAL(wlif_stats.rxerror);
	stats->tx_errors = WLCNTVAL(wlif_stats.txerror);
	stats->collisions = 0;
	stats->rx_length_errors = 0;
	/*
	 * Stats which are not kept per interface
	 * come from per radio stats
	 */
	stats->rx_over_errors = WLCNTVAL(wl->pub->_cnt->rxoflo);
	stats->rx_crc_errors = WLCNTVAL(wl->pub->_cnt->rxcrc);
	stats->rx_frame_errors = 0;
	stats->rx_fifo_errors = WLCNTVAL(wl->pub->_cnt->rxoflo);
	stats->rx_missed_errors = 0;
	stats->tx_fifo_errors = 0;

fail:
	RETURN_STATS64(stats);
}
#else
static struct net_device_stats*
wl_get_stats(struct net_device *dev)
{
	struct net_device_stats *stats_watchdog = NULL;
	struct net_device_stats *stats = NULL;
	wl_info_t *wl;
	wl_if_t *wlif;

	if (!dev)
		return NULL;

	if ((wl = WL_INFO_GET(dev)) == NULL)
		return NULL;

	if ((wlif = WL_DEV_IF(dev)) == NULL)
		return NULL;

	if ((stats = &wlif->stats) == NULL)
		return NULL;

#ifndef BCMQT
	/* skip this trace in emulator builds since it happens every second */
	WL_TRACE(("wl%d: wl_get_stats\n", wl->pub->unit));
#endif // endif

	ASSERT(wlif->stats_id < 2);
	stats_watchdog = &wlif->stats_watchdog[wlif->stats_id];
	memcpy(stats, stats_watchdog, sizeof(struct net_device_stats));
	return (stats);
}
#endif /* KERNEL_VERSION >= 2.6.36 */

#ifdef USE_IW
struct iw_statistics *
wl_get_wireless_stats(struct net_device *dev)
{
	int res = 0;
	wl_info_t *wl;
	wl_if_t *wlif;
	struct iw_statistics *wstats = NULL;
	struct iw_statistics *wstats_watchdog = NULL;
	int phy_noise, rssi;

	if (!dev)
		return NULL;

	if ((wl = WL_INFO_GET(dev)) == NULL)
		return NULL;

	if ((wlif = WL_DEV_IF(dev)) == NULL)
		return NULL;

	if ((wstats = &wlif->wstats) == NULL)
		return NULL;

	WL_TRACE(("wl%d: wl_get_wireless_stats\n", wl->pub->unit));

	ASSERT(wlif->stats_id < 2);
	wstats_watchdog = &wlif->wstats_watchdog[wlif->stats_id];

	phy_noise = wlif->phy_noise;
#if WIRELESS_EXT > 11
	wstats->discard.nwid = 0;
	wstats->discard.code = wstats_watchdog->discard.code;
	wstats->discard.fragment = wstats_watchdog->discard.fragment;
	wstats->discard.retries = wstats_watchdog->discard.retries;
	wstats->discard.misc = wstats_watchdog->discard.misc;

	wstats->miss.beacon = 0;
#endif /* WIRELESS_EXT > 11 */

	/* RSSI measurement is somewhat meaningless for AP in this context */
	if (AP_ENAB(wl->pub)) {
		rssi = 0;
	} else {
		scb_val_t scb;

		WL_LOCK(wl);
		res = wlc_ioctl(wl->wlc, WLC_GET_RSSI, &scb, sizeof(int), wlif->wlcif);
		WL_UNLOCK(wl);

		if (res) {
			WL_ERROR(("wl%d: %s: WLC_GET_RSSI failed (%d)\n",
				wl->pub->unit, __FUNCTION__, res));
			return NULL;
		}
		rssi = scb.val;
	}

	if (rssi <= WLC_RSSI_NO_SIGNAL)
		wstats->qual.qual = 0;
	else if (rssi <= WLC_RSSI_VERY_LOW)
		wstats->qual.qual = 1;
	else if (rssi <= WLC_RSSI_LOW)
		wstats->qual.qual = 2;
	else if (rssi <= WLC_RSSI_GOOD)
		wstats->qual.qual = 3;
	else if (rssi <= WLC_RSSI_VERY_GOOD)
		wstats->qual.qual = 4;
	else
		wstats->qual.qual = 5;

	/* Wraps to 0 if RSSI is 0 */
	wstats->qual.level = 0x100 + rssi;
	wstats->qual.noise = 0x100 + phy_noise;
#if WIRELESS_EXT > 18
	wstats->qual.updated |= (IW_QUAL_ALL_UPDATED | IW_QUAL_DBM);
#else
	wstats->qual.updated |= 7;
#endif /* WIRELESS_EXT > 18 */

	return wstats;
} /* wl_get_wireless_stats */
#endif /* USE_IW */

static int
wl_set_mac_address(struct net_device *dev, void *addr)
{
	int err = 0;
	wl_info_t *wl;
	struct sockaddr *sa = (struct sockaddr *) addr;

	if (!dev)
		return -ENETDOWN;

	wl = WL_INFO_GET(dev);

	WL_TRACE(("wl%d: wl_set_mac_address\n", wl->pub->unit));

	WL_LOCK(wl);

	bcopy(sa->sa_data, dev->dev_addr, ETHER_ADDR_LEN);
	err = wlc_iovar_op(wl->wlc, "cur_etheraddr", NULL, 0, sa->sa_data, ETHER_ADDR_LEN,
		IOV_SET, (WL_DEV_IF(dev))->wlcif);
	WL_UNLOCK(wl);
	if (err)
		WL_ERROR(("wl%d: wl_set_mac_address: error setting MAC addr override\n",
			wl->pub->unit));
	return err;
}

#if defined(USE_CFG80211)
static int
wl_cfg80211_set_mac_address(struct net_device *dev, void *addr)
{
	struct bcm_cfg80211 *cfg = wl_get_cfg(dev);
	s32 err = 0;
	s32 bssidx;
	wl_info_t *wl;
	struct sockaddr *sa = (struct sockaddr *) addr;

	if (!dev)
		return -ENETDOWN;

	WL_DBG((" %s \n", __FUNCTION__));

	wl = WL_INFO_GET(dev);

	WL_TRACE(("wl%d: wl_cfg80211_set_mac_address\n", wl->pub->unit));

	bcopy(sa->sa_data, dev->dev_addr, ETHER_ADDR_LEN);

	if (dev->ieee80211_ptr != NULL) {
		if ((bssidx = wl_get_bssidx_by_wdev(cfg, dev->ieee80211_ptr)) < 0) {
			WL_ERR(("Find bss index from wdev(%p) failed\n", dev->ieee80211_ptr));
			return BCME_ERROR;
		}
		err = wldev_iovar_setbuf_bsscfg(dev, "cur_etheraddr", sa->sa_data, ETHER_ADDR_LEN,
			cfg->ioctl_buf, WLC_IOCTL_MAXLEN, bssidx, &cfg->ioctl_buf_sync);

		if (unlikely(err))
			WL_ERROR(("wl%d: wl_set_mac_address: error setting MAC addr override %d\n",
				wl->pub->unit, err));
	}
	else {
		wl_set_mac_address(dev, addr);
	}

	return err;
}
#endif /* USE_CFG80211 */

static void
wl_set_multicast_list(struct net_device *dev)
{
	if (!WL_ALL_PASSIVE_ENAB((wl_info_t *)WL_INFO_GET(dev)))
		_wl_set_multicast_list(dev);
#ifdef WL_ALL_PASSIVE
	else {
		wl_info_t *wl = WL_INFO_GET(dev);
		wl->multicast_task.context = dev;

		atomic_inc(&wl->callbacks);
		if (!SCHEDULE_WORK(wl, &wl->multicast_task.work)) {
			/* work item may already be on the work queue, so only inc callbacks if
			 * we actually schedule a new item
			 */
			CALLBACK_DEC_AND_ASSERT(wl);
		}
	}
#endif /* WL_ALL_PASSIVE */
}

static void
_wl_set_multicast_list(struct net_device *dev)
{
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 34)
	struct dev_mc_list *mclist;
#else
	struct netdev_hw_addr *ha;
#endif // endif
	wl_info_t *wl;
	int i, buflen;
	struct maclist *maclist;
	int allmulti;

	if (!dev)
		return;
	wl = WL_INFO_GET(dev);

	WL_TRACE(("wl%d: wl_set_multicast_list\n", wl->pub->unit));

	if (wl->pub->up) {
		allmulti = (dev->flags & IFF_ALLMULTI)? TRUE: FALSE;

		buflen = sizeof(struct maclist) + (MAXMULTILIST * ETHER_ADDR_LEN);

		if ((maclist = MALLOC(wl->pub->osh, buflen)) == NULL) {
			return;
		}

		/* copy the list of multicasts into our private table */
		i = 0;
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 34)
		for (mclist = dev->mc_list; mclist && (i < dev->mc_count); mclist = mclist->next) {
			if (i >= MAXMULTILIST) {
				allmulti = TRUE;
				i = 0;
				break;
			}
			bcopy(mclist->dmi_addr, &maclist->ea[i++], ETHER_ADDR_LEN);
		}
#else
#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == \
	4 && __GNUC_MINOR__ >= 6))
_Pragma("GCC diagnostic push")
_Pragma("GCC diagnostic ignored \"-Wcast-qual\"")
#endif // endif
		netdev_for_each_mc_addr(ha, dev)
		{
			if (i >= MAXMULTILIST) {
				allmulti = TRUE;
				i = 0;
				break;
			}
			bcopy(ha->addr, &maclist->ea[i++], ETHER_ADDR_LEN);
		}
#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == \
	4 && __GNUC_MINOR__ >= 6))
_Pragma("GCC diagnostic pop")
#endif // endif
#endif /* LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 34) */
		maclist->count = i;

		WL_LOCK(wl);

		/* update ALL_MULTICAST common code flag */
		wlc_iovar_op(wl->wlc, "allmulti", NULL, 0, &allmulti, sizeof(allmulti), IOV_SET,
			(WL_DEV_IF(dev))->wlcif);
		wlc_set(wl->wlc, WLC_SET_PROMISC, (dev->flags & IFF_PROMISC));

		/* set up address filter for multicasting */
		wlc_iovar_op(wl->wlc, "mcast_list", NULL, 0, maclist, buflen, IOV_SET,
			(WL_DEV_IF(dev))->wlcif);

		WL_UNLOCK(wl);
		MFREE(wl->pub->osh, maclist, buflen);
	}
} /* _wl_set_multicast_list */

#if defined(BCMQT) && defined(WLC_OFFLOADS_TXSTS)
/**
 * The 2x2 ax core in the 63178 has two irq lines towards the ARM host processor. This could not
 * be easily modeled in the non-full-chip Veloce testbench. Therefore, that testbench implemented a
 * shortcut: the two irq lines are OR'ed together and result in one interrupt on the (emulated)
 * PCIe endpoint. Note that there is also a full-chip testbench that doesn't have this problem. XXX.
 */
static irqreturn_t BCMFASTPATH
wl_veloce_isr(int irq, void *dev_id)
{
	wl_info_t *wl = (wl_info_t*) dev_id;
	int irq_vec;

#ifdef WLC_OFFLOADS_TXSTS
	irq_vec = wlc_int_is_m2m_irq(wl->wlc) ? ARMGIC_CORE_A_M2M_IRQV : ARMGIC_CORE_A_D11_IRQV;
#else
	BCM_REFERENCE(wl);
	irq_vec = ARMGIC_CORE_A_D11_IRQV; /* not 2 but just 1 interrupt line */
#endif /* WLC_OFFLOADS_TXSTS */

	return wl_isr(irq_vec, dev_id);
}
#endif /* BCMQT && WLC_OFFLOADS_TXSTS */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 20)
irqreturn_t BCMFASTPATH
wl_isr(int irq, void *dev_id)
#else
irqreturn_t BCMFASTPATH
wl_isr(int irq, void *dev_id, struct pt_regs *ptregs)
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 20) */
{
	wl_info_t *wl;
	bool ours, wantdpc;
	unsigned long flags;

	wl = (wl_info_t*) dev_id;
#ifdef WLC_OFFLOADS_TXSTS
	INT_LOCK(wl, flags); /* disables irqs to prevent isr preemption by other isr */
#else
	/* prevents concurrent DPC access to irq registers and variables */
	WL_ISRLOCK(wl, flags); /* prevents concurrent DPC access to irq registers and variables */
#endif /* WLC_OFFLOADS_TXSTS */

#ifdef STB_SOC_WIFI
	/* device to host interrupt handler */
	wl_stbsoc_d2h_isr(wl->plat_info);
#endif /* STB_SOC_WIFI */

#if defined(WLC_OFFLOADS_TXSTS) && (defined(IS_BCA_2x2AX_BUILD) || defined(BCMQT))
	if (irq == GET_2x2AX_M2M_IRQV(wl->dev->base_addr)) {
		ours = wlc_isr_offload_txs_ch1(wl->wlc, &wantdpc);
		ASSERT(ours != 0); // because this is not a shared irq line
	} else
#endif /* WLC_OFFLOADS_TXSTS && (IS_BCA_2x2AX_BUILD || BCMQT) */
	{
		ours = wlc_isr(wl->wlc, &wantdpc);
	}

	/* call common first level interrupt handler */
	if (ours) {
		/* if more to do... */
		if (wantdpc) {
			/* ...and call the second level interrupt handler */
			/* schedule dpc */
			ASSERT(wl->resched == FALSE);
#ifdef NAPI_POLL
			/* allow the device to be added to the cpu polling
			 * list if we are up
			 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 30)
			napi_schedule(&wl->napi);
#else
			netif_rx_schedule(wl->dev);
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 30) */
#else /* NAPI_POLL */
#ifdef WL_ALL_PASSIVE
			if (WL_ALL_PASSIVE_ENAB(wl)) {
				atomic_inc(&wl->callbacks);
#if defined(WL_USE_L34_THREAD)
				if (!wl->rxq_dispatched) {
					wl->rxq_dispatched = 1;
					wl_thread_schedule_work(wl);
				} else {
#else
				if (!SCHEDULE_WORK(wl, &wl->wl_dpc_task.work)) {
#endif /* WL_USE_L34_THREAD */
					CALLBACK_DEC_AND_ASSERT(wl);
				}
			} else
#endif /* WL_ALL_PASSIVE */
				tasklet_schedule(&wl->tasklet);
#endif /* NAPI_POLL */
		}
	}
#ifdef STB_SOC_WIFI
	/* re-enable device to host interrupt */
	wl_stbsoc_d2h_intstatus(wl->plat_info);
#endif /* STB_SOC_WIFI */

#ifdef WLC_OFFLOADS_TXSTS
	INT_UNLOCK(wl, flags); /* re-enables irqs */
#else
	WL_ISRUNLOCK(wl, flags);
#endif /* WLC_OFFLOADS_TXSTS */

	return IRQ_RETVAL(ours);
} /* wl_isr */

#ifdef NAPI_POLL
static int BCMFASTPATH
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 30)
wl_poll(struct napi_struct *napi, int budget)
#else
wl_poll(struct net_device *dev, int *budget)
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 30) */
#else /* NAPI_POLL */
static void BCMFASTPATH
wl_dpc(ulong data)
#endif /* NAPI_POLL */
{
	wl_info_t *wl;

#ifdef NAPI_POLL
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 30)
	wl = (wl_info_t *)container_of(napi, wl_info_t, napi);
	wl->pub->tunables->rxbnd = min(RXBND, budget);
#else
	wl = WL_INFO_GET(dev);
	wl->pub->tunables->rxbnd = min(RXBND, *budget);
	ASSERT(wl->pub->tunables->rxbnd <= dev->quota);
#endif /* #if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 30) */
#else /* NAPI_POLL */

	wl = (wl_info_t *)data;

	WL_LOCK(wl);
#endif /* NAPI_POLL */

	/* call the common second level interrupt handler */
	if (wl->pub->up) {
		wlc_dpc_info_t dpci = {0};

		if (wl->resched) {
			unsigned long flags = 0;
			INT_LOCK(wl, flags);
			wlc_intrsupd(wl->wlc);
			INT_UNLOCK(wl, flags);
		}
		wl->resched = wlc_dpc(wl->wlc, TRUE, &dpci);
		wl->processed = dpci.processed;
	}

	/* wlc_dpc() may bring the driver down */
	if (!wl->pub->up) {
#ifdef WL_ALL_PASSIVE
#if defined(WL_USE_L34_THREAD)
		wl->rxq_dispatched = 0;
#endif /* WL_USE_L34_THREAD */
		/* Reenable wl_dpc_task to be dispatch */
		if ((WL_ALL_PASSIVE_ENAB(wl))) {
			CALLBACK_DEC_AND_ASSERT(wl);
		}
#endif /* WL_ALL_PASSIVE */
		goto done;
	}

#ifndef NAPI_POLL
#ifdef WL_ALL_PASSIVE
	if (wl->resched) {
		if (!(WL_ALL_PASSIVE_ENAB(wl)))
			tasklet_schedule(&wl->tasklet);
		else
#if defined(WL_USE_L34_THREAD)
			/*
			 * In Linux 3.4, we have a dedicated thread for wlan processing,
			 * so as long as wl->rxq_dispatched is TRUE, it will keep running.
			 * So no need to re-schedule the thread.
			 */
			if (0)
#else
			if (!SCHEDULE_WORK(wl, &wl->wl_dpc_task.work))
#endif /* WL_USE_L34_THREAD */
			{
				/* wl_dpc_task alread in queue.
				 * Shall not reach here
				 * Removed the Assert, causing issue while running traffic
				 * by issuing reinit command with dpc pending for schedule
				 * occurs in the same context of dpc is not considerd fatal
				 */
			}
	} else {
#if defined(WL_USE_L34_THREAD)
		wl->rxq_dispatched = 0;
#endif /* WL_USE_L34_THREAD */

		/* re-enable interrupts */
		if (WL_ALL_PASSIVE_ENAB(wl)) {
			CALLBACK_DEC_AND_ASSERT(wl);
		}
		wl_intrson(wl);
	}
#else /* WL_ALL_PASSIVE */
	if (wl->resched)
		tasklet_schedule(&wl->tasklet);
	else {
		/* re-enable interrupts */
		wl_intrson(wl);
	}
#endif /* WL_ALL_PASSIVE */

done:
	WL_UNLOCK(wl);
	return;
#else /* NAPI_POLL */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 30)
	WL_TRACE(("wl%d: wl_poll: rxbnd %d budget %d processed %d\n",
		wl->pub->unit, wl->pub->rxbnd, budget, wl->processed));

	ASSERT(wl->processed <= wl->pub->tunables->rxbnd);

	/* update number of frames processed */
	/* we got packets but no budget */
	if (!wl->resched) {
		napi_complete(&wl->napi);
		/* enable interrupts now */
		wl_intrson(wl);
	}
	return wl->processed;
done:
	return 0;

#else /* LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 30) */
	WL_TRACE(("wl%d: wl_poll: rxbnd %d quota %d budget %d processed %d\n",
	          wl->pub->unit, wl->pub->rxbnd, dev->quota,
	          *budget, wl->processed));

	ASSERT(wl->processed <= wl->pub->tunables->rxbnd);

	/* update number of frames processed */
	*budget -= wl->processed;
	dev->quota -= wl->processed;

	/* we got packets but no budget */
	if (wl->resched)
		/* indicate that we are not done, don't enable
		 * interrupts yet. linux network core will call
		 * us again.
		 */
		return 1;

	netif_rx_complete(dev);

	/* enable interrupts now */
	wl_intrson(wl);
done:
	return 0;
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 30) */
#endif /* NAPI_POLL */
} /* wl_poll / wl_dpc */

#if defined(WL_ALL_PASSIVE)
void BCMFASTPATH
wl_dpc_rxwork(struct wl_task *task)
{
	wl_info_t *wl = (wl_info_t *)task->context;
	//WL_TRACE(("wl%d: %s\n", wl->pub->unit, __FUNCTION__));

	wl_dpc((unsigned long)wl);
	return;
}
#endif /* defined(WL_ALL_PASSIVE) */

#ifdef HNDCTF
static inline int32
wl_ctf_forward(wl_info_t *wl, struct sk_buff *skb)
{
	int32 ret;
	/* use slow path if ctf is disabled */
	if (!CTF_ENAB(wl->cih))
		return (BCME_ERROR);

	/* try cut thru first */
	if ((ret = ctf_forward(wl->cih, skb, skb->dev)) != BCME_ERROR) {
		if (ret == BCME_EPERM)
			PKTCFREE(wl->osh, skb, FALSE);
		return (BCME_OK);
	}

	return (BCME_ERROR);
}
#endif /* HNDCTF */

void
wl_sendup_event(wl_info_t *wl, wl_if_t *wlif, void *p)
{
	wl_sendup(wl, wlif, p, 1);
}

#if defined(WLCFP)

/**
 * -----------------------------------------------------------------------------
 * Funtion	: wl_cfp_sendup
 * Description	: Breaks AMSDU subframes and sendup
 * -----------------------------------------------------------------------------
 */

void BCMFASTPATH
wl_cfp_sendup(wl_info_t * wl, wl_if_t * wlif, void * pkt, uint16 flowid)
{
	struct ether_addr *da;
	void * next_pkt;
	struct net_device * net_device;
	struct scb * scb = WLPKTTAGSCBGET(pkt);

	WL_TRACE(("wl%d: wl_cfp_sendup: %d bytes\n", wl->pub->unit, PKTLEN(wl->osh, pkt)));

	net_device = (wlif != NULL) ? wlif->dev : wl->dev;

#if defined(STS_FIFO_RXEN) || defined(WLC_OFFLOADS_RXSTS)
	if (STS_RX_ENAB(wl->wlc->pub) || STS_RX_OFFLOAD_ENAB(wl->wlc->pub)) {
		wlc_stsbuff_free(wl->wlc, pkt);
	}
#endif /* STS_FIFO_RXEN || WLC_OFFLOADS_RXSTS */

	/* Loop through AMSDU sub frames if any */
	for (; pkt; pkt = next_pkt) {

		next_pkt = PKTNEXT(wl->osh, pkt);
		PKTSETNEXT(wl->osh, pkt, NULL);

		da = (struct ether_addr *) PKTDATA(wl->osh, pkt);

		/* AMSDU sub frames might have multicast frames received.
		 * Send via slow path if any
		 */
		if (ETHER_ISMULTI(da)) {
			struct wlc_frminfo f;

			f.p = pkt;
			f.da = da;
			f.wds = FALSE;
			wlc_recvdata_sendup_msdus(wl->wlc, scb, &f);
			continue;
		}

#if defined(BCM_PKTFWD)
		/* Convert the packet, mainly detach the pkttag */
		pkt = PKTTONATIVE(wl->osh, pkt);

		/** Intrass for CFP packets is handled in
		 * wl_pktfwd_pktqueue_add_pkt()/wl_awl_upstream_add_pkt().
		 */
		wl_pktfwd_pktqueue_add_pkt(wl, net_device, pkt, flowid);

#else /* ! BCM_PKTFWD */

		if (wl_intrabss_forward(wl, net_device, pkt) == FALSE) {
			wl_sendup(wl, wlif, pkt, 1);
		}
#endif /* ! BCM_PKTFWD */
	}

} /* wl_cfp_sendup() */

#endif /* WLCFP */

/**
 * Function	: wl_intrabss_forward()
 * Description	: Handles Intrabss fowarding.
 *		  If bsscfg->ap_isolate is set, all packets will be sent up
 *		  else forward the packet to WLAN tx if destination is associated to same BSS.
 *
 *		  If PROMISC_ENAB is set, a copy of intrabss packet will sent to Network stack.
 *
 * RETURN	: TRUE - Intrabss packet and caller should forget the packet.
 */
bool BCMFASTPATH
wl_intrabss_forward(wl_info_t *wl, struct net_device *net_device, void *pkt)
{
	bool intrabss_fwd = FALSE;
	struct wlc_if	* wlcif;
	wlc_bsscfg_t	* bsscfg;
	struct ether_addr * da;

	da = (struct ether_addr *)PKTDATA(wl->osh, pkt);

	/* Intrabss forwarding for BCMC packets is taken care in wlc_recvdata_sendup_msdus()
	 * This API is used only for unicast packets.
	 */
	ASSERT(ETHER_ISUCAST(da));

#ifdef DPSTA
#ifdef BCA_HNDROUTER
#if defined(CONFIG_BCM_KF_WL)
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0))
	/* In case of dpsta_recv, there won't be intrabss transmission */
	if (!(net_device->priv_flags & IFF_BCM_WLANDEV)) {
		return intrabss_fwd;
	}
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0) */
#endif /* CONFIG_BCM_KF_WL */
#endif /* BCA_HNDROUTER */
#endif /* DPSTA */

	wlcif = WL_DEV_IF(net_device)->wlcif;
	bsscfg = wlc_bsscfg_find_by_wlcif(wl->wlc, wlcif);

	if (wlcif->type == WLC_IFTYPE_WDS) {
		/* WDS packets has to be sent to Network stack */
		return intrabss_fwd;
	}

	/* Forward packets destined within the BSS */
	if (BSSCFG_AP(bsscfg) && !bsscfg->ap_isolate) {
		struct scb * dst;

		if ((eacmp(da, wl->pub->cur_etheraddr.octet) != 0) &&
			(dst = wlc_scbfind(wl->wlc, bsscfg, da))) {
			/* Check that the dst is associated to same BSS
			 * before forwarding within the BSS
			 */
			if (SCB_ASSOCIATED(dst)) {
				intrabss_fwd = TRUE;
			}
		}
	}

	if (intrabss_fwd) {
#if defined(WLCNT)
		if (WME_ENAB(wl->pub)) {
			uint8 ac = WME_PRIO2AC(PKTPRIO(pkt));
			wl_traffic_stats_t *forward;

			forward =  &wl->wlc->pub->_wme_cnt->forward[ac];
			WLCNTINCR(forward->packets);
			WLCNTADD(forward->bytes, PKTLEN(wl->osh, pkt));
		}
#endif /* WLCNT */

#if defined(BCM_PKTFWD)
		/* keep the accounting correct */
		PKTACCOUNT(wl->osh, 1, TRUE);
#endif /* BCM_PKTFWD */

#if !defined(BCM47XX_CA9) && !defined(BCA_HNDROUTER)
		if (PROMISC_ENAB(wl->wlc->pub)) {
			void *pktdup;

			/* both forward and send up stack */
			intrabss_fwd = FALSE;
			if ((pktdup = PKTDUP(wl->osh, pkt)) != NULL) {
				wlc_recvdata_sendpkt(wl->wlc, pktdup, wlcif);
			}
		} else
#endif /* !BCM47XX_CA9 && !BCA_HNDROUTER */
		{
			wlc_recvdata_sendpkt(wl->wlc, pkt, wlcif);
		}
	}

	return intrabss_fwd;
} /* wl_intrabss_forward() */

/** Sendup packet to Flowcache/Network stack */
void BCMFASTPATH
wl_sendup_ex(wl_info_t *wl, void *pkt)
{
	struct sk_buff * skb = (struct sk_buff *)pkt;
	bool brcm_specialpkt;

	/* Internally generated events have the special ether-type of
	 * ETHER_TYPE_BRCM
	*/
	brcm_specialpkt = (ntoh16_ua(skb->data + ETHER_TYPE_OFFSET) == ETHER_TYPE_BRCM);
	BCM_REFERENCE(brcm_specialpkt);

#if defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE) || \
	defined(WLBIN_COMPAT)
	if (wl_spdsvc_rx(skb) == BCME_OK)
		return;
#endif /* CONFIG_BCM_SPDSVC || CONFIG_BCM_SPDSVC_MODULE || WLBIN_COMPAT */

	PKTSETFCDONE(skb);
#ifdef BCM_BLOG
	if (wl_handle_blog_sinit(wl, skb) == 0)
		return;
#endif /* BCM_BLOG */

	PKTCLRFCDONE(skb);
	skb->protocol = eth_type_trans(skb, skb->dev);

	/*
	 * Problem with WAPI encrypted/decrypted packet is it uses a 18 byte IV which makes
	 * it different from all the other cryptos and ucode wouldn't know about this,
	 * So data would be always unaligned
	 * XXX: could find a way to indicate to ucode to add the pad bytes.
	 */
	/* Internally generated special ether-type ETHER_TYPE_BRCM packets for event data
	 * have no requirement for alignment, so skip the alignment check for brcm_specialpkt
	*/
	if (!brcm_specialpkt && !ISALIGNED(skb->data, 4)) {
		WL_APSTA_RX(("Unaligned assert. skb %p. skb->data %p.\n", skb, skb->data));
		if (WL_DEV_IF(skb->dev)) {
			WL_APSTA_RX(("wl_sendup: dev name is %s (wlif) \n", skb->dev->name));
			WL_APSTA_RX(("wl_sendup: hard header len  %d (wlif) \n",
				skb->dev->hard_header_len));
		}
		WL_APSTA_RX(("wl_sendup: dev name is %s (wl) \n", wl->dev->name));
		WL_APSTA_RX(("wl_sendup: hard header len %d (wl) \n", wl->dev->hard_header_len));
	}

	/* send it up */
	WL_APSTA_RX(("wl%d: wl_sendup(): pkt %p summed %d on interface %p (%s)\n",
		wl->pub->unit, pkt, skb->ip_summed, WL_DEV_IF(skb->dev), skb->dev->name));

#if defined(BCM_NBUFF_PKT) && defined(CC_BPM_SKB_POOL_BUILD)
	PKTTAINTED(wl->osh, skb);
#endif /* BCM_NBUFF_PKT && CC_BPM_SKB_POOL_BUILD */

#ifdef NAPI_POLL
	netif_receive_skb(skb);
#else /* NAPI_POLL */
	netif_rx(skb);
#endif /* NAPI_POLL */
} /* wl_sendup_ex() */

/**
 * The last parameter was added for the build. Caller of
 * this function should pass 1 for now.
 */
void BCMFASTPATH
wl_sendup(wl_info_t *wl, wl_if_t *wlif, void *p, int numpkt)
{
	struct sk_buff *skb;
#ifdef HNDCTF
	struct sk_buff *nskb;
#endif /* HNDCTF */
	bool brcm_specialpkt;
#if defined(BCM_GMAC3)
	fwder_t * fwdh; /* forwarder handler */
	int fwder_rx_port; /* fwder WOFA cached lookup assist */
#endif /* BCM_GMAC3 */
#if defined(PKTC_TBL) && defined(CONFIG_BCM_KF_WL) && defined(BCM_PKTFWD)
	struct sk_buff *xskb, *xskb1;
#endif /* PKTC_TBL && CONFIG_BCM_KF_WL && BCM_PKTFWD */
#ifdef DPSTA
	wlc_bsscfg_t *bsscfg = wlc_bsscfg_find_by_wlcif(wl->wlc, NULL);
	bool dpsta_recved = FALSE;
#endif // endif
#ifdef WL_AIR_IQ
	bcm_event_t *msg;
#endif /* WL_AIR_IQ */

	WL_TRACE(("wl%d: wl_sendup: %d bytes\n", wl->pub->unit, PKTLEN(wl->osh, p)));

#if defined(STS_FIFO_RXEN) || defined(WLC_OFFLOADS_RXSTS)
	if (STS_RX_ENAB(wl->pub) || STS_RX_OFFLOAD_ENAB(wl->pub)) {
		wlc_stsbuff_free(wl->wlc, p);
	}
#endif /* STS_FIFO_RXEN || WLC_OFFLOADS_RXSTS */

	/* Internally generated events have the special ether-type of
	 * ETHER_TYPE_BRCM
	*/
	brcm_specialpkt =
		(ntoh16_ua(PKTDATA(wl->pub->osh, p) + ETHER_TYPE_OFFSET) == ETHER_TYPE_BRCM);

	if (!brcm_specialpkt) {
#ifdef ARPOE
		/* Arp agent */
		if (ARPOE_ENAB(wl->pub)) {
			wl_arp_info_t *arpi = wl_get_arpi(wl, wlif);
			if (arpi) {
				int err = wl_arp_recv_proc(arpi, p);
				if ((err == ARP_REQ_SINK) || (err ==  ARP_REPLY_PEER)) {
					PKTFREE(wl->pub->osh, p, FALSE);
					return;
				}
			}
		}
#endif // endif

#ifdef TOE
		/* Apply TOE */
		if (TOE_ENAB(wl->pub))
			(void)wl_toe_recv_proc(wl->toei, p);
#endif // endif

#ifdef BDO
		if (BDO_SUPPORT(wl->pub) && BDO_ENAB(wl->pub)) {
			if (wl_bdo_rx(wl->bdo, PKTDATA(wl->pub->osh, p), PKTLEN(wl->pub->osh, p))) {
				PKTFREE(wl->pub->osh, p, FALSE);
				return;
			}
		}
#endif	/* BDO */
	}

	/* Convert the packet, mainly detach the pkttag */
#if defined(PKTC_TBL) && defined(CONFIG_BCM_KF_WL) && defined(BCM_PKTFWD)
	skb = (struct sk_buff *)p;
	FOREACHPKT(skb, xskb, xskb1) {
		PKTACCOUNT(wl->osh, 1, FALSE);
		WLPKTTAGCLEAR(xskb1);
	}
#else
	skb = PKTTONATIVE(wl->osh, p);
#endif /* PKTC_TBL && CONFIG_BCM_KF_WL && BCM_PKTFWD */
	/* route packet to the appropriate interface */
	if (wlif) {
		/* drop if the interface is not up yet */
		if (!wlif->dev || !netif_device_present(wlif->dev)) {
			WL_ERROR(("wl%d: wl_sendup: interface not ready\n", wl->pub->unit));
			PKTFREE(wl->osh, p, FALSE);
			return;
		}
		skb->dev = wlif->dev;
#if defined(BCM_GMAC3)
		fwdh = wlif->fwdh;
		fwder_rx_port = wlif->subunit;
#endif // endif
	} else {
		skb->dev = wl->dev;
#if defined(BCM_GMAC3)
		fwdh = wl->fwdh;
		fwder_rx_port = 0; /* use primary interface */
#endif // endif
	}

#ifdef DPSTA
	/* Forward to dpsta if primary dev */
	if (skb->dev == wl->dev) {
		BCM_REFERENCE(bsscfg);
		if (PSTA_ENAB(wl->pub) || DWDS_ENAB(bsscfg) || MAP_ENAB(bsscfg) ||
			WET_ENAB(wl->wlc)) {
			if (dpsta_recv(skb) != BCME_OK) {
				PKTFRMNATIVE(wl->osh, skb);
				PKTFREE(wl->osh, skb, FALSE);
				return;
			}
			if (skb->dev != wl->dev)
				dpsta_recved = TRUE;
		}
	}
#endif /* DPSTA */

#if defined(BCM_GMAC3)
	if (fwdh && !brcm_specialpkt) { /* upstream forwarder */
		if (fwdh->devs_cnt > 1) { /* local bridging */
			/* May we assume that the Ethernet DstAddr is 2 Byte aligned? */
			uint16 *da = (uint16*)PKTDATA(wl->osh, skb);
			ASSERT(ISALIGNED(da, 2));
			if (ETHER_ISUCAST(da)) {
				uintptr_t wofa_data;
				wofa_data = fwder_lookup(fwdh->mate, da, fwder_rx_port);
				if (wofa_data != WOFA_DATA_INVALID) {
					/* Intra|Inter BSS: forward locally */
					struct net_device * fwd_dev = (struct net_device*)wofa_data;
					wlif = WL_DEV_IF(fwd_dev);
					/* could be second radio (Inter) */
					wl = WL_INFO_GET(fwd_dev);
					skb->dev = fwd_dev;
					/* WL_LOCK is already taken in wl_dpc() for RX path. */
					wl_start_int_try(wl, wlif, skb, WL_LOCK_TAKEN);
					return;
				}
			} else {
				ASSERT(numpkt == 1);
				fwder_flood(fwdh->mate, skb, wl->osh, TRUE, wl_start_try);
				/* Need to also send to GMAC fwder */
			}
		}

		/* Now send using upstream forwarder to GMAC */
		if (fwder_transmit(fwdh, skb, numpkt, skb->dev) != FWDER_SUCCESS) {
			PKTFRMNATIVE(wl->osh, skb);
			PKTCFREE(wl->osh, skb, FALSE);
		}

		return;
	}
#endif /* ! BCM_GMAC3 */

#if !defined(BCMDONGLEHOST)
#ifdef WL_AIR_IQ
	msg = (bcm_event_t *) PKTDATA(wl->wlc->osh, p);
	if (msg->eth.ether_type == hton16(ETHER_TYPE_BRCM)) {
		if (msg->event.event_type == hton32(WLC_E_AIRIQ_EVENT)) {
			msg->eth.ether_type = hton16(ETHER_TYPE_BRCM_AIRIQ);
		}
	}
#endif /* WL_AIR_IQ */
#endif /* !defined(BCMDONGLEHOST) */

#ifdef HNDCTF
	/* try cut thru' before sending up */
	if (wl_ctf_forward(wl, skb) != BCME_ERROR)
		return;

sendup_next:
	/* clear skipct flag before sending up */
	PKTCLRSKIPCT(wl->osh, skb);

#ifdef CTFPOOL
	/* allocate and add a new skb to the pkt pool */
	if (PKTISFAST(wl->osh, skb))
		osl_ctfpool_add_by_poolptr(wl->osh, CTFPOOLPTR(wl->osh, skb));

	/* clear fast buf flag before sending up */
	PKTCLRFAST(wl->osh, skb);

	/* re-init the hijacked field */
	CTFPOOLPTR(wl->osh, skb) = NULL;
#endif /* CTFPOOL */

	nskb = (PKTISCHAINED(skb) ? PKTCLINK(skb) : NULL);
	PKTSETCLINK(skb, NULL);
	PKTCLRCHAINED(wl->osh, skb);
	PKTCCLRFLAGS(skb);

	/* map the unmapped buffer memory before sending up */
	PKTCTFMAP(wl->osh, skb);
#endif /* HNDCTF */

#if defined(PKTC_TBL)
#if !defined(WL_EAP_WLAN_ONLY_UL_PKTC)
	if (!brcm_specialpkt
#ifdef DPSTA
			&& !dpsta_recved
#endif
			) {
		if (wl_rxchainhandler(wl, skb) == BCME_OK)
			return;
	}
#else /* WL_EAP_WLAN_ONLY_UL_PKTC */

#if defined(CONFIG_BCM_KF_WL) && defined(BCM_PKTFWD)
	/* Clear remaining wl_cb */
	FOREACHPKT(skb, xskb, xskb1) {
		bzero(xskb1->wl_cb, sizeof(xskb1->wl_cb));
	}
#endif /* CONFIG_BCM_KF_WL && BCM_PKTFWD */

	/* De-chain the chained packets, clear flags, clear pkttag,
		and assign device to the skb
	*/
	if (PKTISCHAINED(skb)) {
		void *nskb = NULL;
		FOREACH_CHAINED_PKT(skb, nskb) {
			PKTCLRCHAINED(wl->osh, skb);
			PKTCCLRFLAGS(skb);
			PKTTONATIVE(wl->osh, skb);
			if (wlif) {
				skb->dev = wlif->dev;
			} else {
				skb->dev = wl->dev;
			}
			skb->protocol = eth_type_trans(skb, skb->dev);
#if defined(BCM_NBUFF_PKT) && defined(CC_BPM_SKB_POOL_BUILD)
			PKTTAINTED(wl->osh, skb);
#endif /* BCM_NBUFF_PKT && CC_BPM_SKB_POOL_BUILD */

			/* XXX: Note that we could call enet_xmit here directly after
				looking up the tx_dev in the pktc_table. But our customers
				typically would not do that so not doing that here.
			*/
#ifdef NAPI_POLL
			netif_receive_skb(skb);
#else /* NAPI_POLL */
			netif_rx(skb);
#endif /* NAPI_POLL */
		}
		return;
	}
#endif /* WL_EAP_WLAN_ONLY_UL_PKTC */
#endif /* PKTC_TBL */

#if defined(PKTC_TBL) && defined(CONFIG_BCM_KF_WL) && defined(BCM_PKTFWD)
	/* Clear remaining wl_cb */
	FOREACHPKT(skb, xskb, xskb1) {
		bzero(xskb1->wl_cb, sizeof(xskb1->wl_cb));
	}
#endif /* PKTC_TBL && CONFIG_BCM_KF_WL && BCM_PKTFWD */

#if defined(BCM_WFD) && defined(CONFIG_BCM_FC_BASED_WFD)
	/* For Fcache based WFD WLAN --> WLAN Packet Chain :
	   Need De-chain packets before send packet to linux/fcache
	   De-chain the chained packets, clear flags, clear pkttag,
	   and assign device to the skb
	*/
	if (PKTISCHAINED(skb)) {
		void *nskb = NULL;
		FOREACH_CHAINED_PKT(skb, nskb) {
			PKTCLRCHAINED(wl->osh, skb);
			PKTCCLRFLAGS(skb);
			if (wlif) {
				skb->dev = wlif->dev;
			} else {
				skb->dev = wl->dev;
			}
			wl_sendup_ex(wl, skb);
		}
		return;
	}
#endif /* defined(BCM_WFD) && defined(CONFIG_BCM_FC_BASED_WFD) */

	wl_sendup_ex(wl, skb);

#ifdef HNDCTF
	if (nskb != NULL) {
		nskb->dev = skb->dev;
		skb = nskb;
		goto sendup_next;
	}
#endif /* HNDCTF */
} /* wl_sendup */

/* I/O ports for configuration space */
#define	PCI_CFG_ADDR	0xcf8	/* Configuration address to read/write */
#define	PCI_CFG_DATA	0xcfc	/* Configuration data for conf 1 mode */
#define	PCI_CFG_DATA2	0xcfa	/* Configuration data for conf 2 mode */
#define PCI_EN 0x80000000

static uint32
read_pci_cfg(uint32 bus, uint32 slot, uint32 fun, uint32 addr)
{
	uint32 config_cmd = PCI_EN | (bus << PCICFG_BUS_SHIFT) |
		(slot << PCICFG_SLOT_SHIFT) | (fun << PCICFG_FUN_SHIFT) | (addr & 0xfffffffc);

	outl(config_cmd, PCI_CFG_ADDR);
	return inl(PCI_CFG_DATA);
}

static void
write_pci_cfg(uint32 bus, uint32 slot, uint32 fun, uint32 addr, uint32 data)
{
	uint32 config_cmd = PCI_EN | (bus << PCICFG_BUS_SHIFT) |
		(slot << PCICFG_SLOT_SHIFT) | (fun << PCICFG_FUN_SHIFT) | (addr & 0xfffffffc);

	outl(config_cmd, PCI_CFG_ADDR);
	outl(data, PCI_CFG_DATA);
}

static uint32 rc_bus = 0xffffffff, rc_dev, rc_fun;

int
wl_osl_pcie_rc(struct wl_info *wl, uint op, int param)
{
	uint32 data;

	if (op == 0) {	/* return link capability in configuration space */
		struct pci_dev *pdev, *pdev_rc;
		uint32 header_type, cap_ptr, link_cap_speed = 0;

		pdev = osl_pci_device(wl->osh);

		if (pdev == NULL)
			return -1;

		pdev_rc = pdev->bus->self;

		if (pdev_rc == NULL)
			return -2;

		rc_bus = pdev_rc->bus->number;
		rc_dev = PCI_SLOT(pdev_rc->devfn);
		rc_fun = PCI_FUNC(pdev_rc->devfn);

		/* Header Type */
		data = read_pci_cfg(rc_bus, rc_dev, rc_fun, 0xc);
		header_type = (data >> 16) & 0x7f;

		if (header_type != 1)
			return -3;

		/* Status */
		data = read_pci_cfg(rc_bus, rc_dev, rc_fun, 0x4);
		data = (data >> 16) & 0xffff;

		if (((data >> 4) & 1) == 0)
			return -4;

		/* Capabilities Pointer */
		data = read_pci_cfg(rc_bus, rc_dev, rc_fun, 0x34);
		cap_ptr = data & 0xff;

		while (cap_ptr) {
			/* PCI Express Capabilities */
			data = read_pci_cfg(rc_bus, rc_dev, rc_fun, cap_ptr + 0x0);

			/* PCI Express Cap ID */
			if ((data & 0xff) != 0x10) {
				/* next cap pointer */
				cap_ptr = (data >> 8) & 0xff;
				continue;
			}

			/* Link Capabilities */
			data = read_pci_cfg(rc_bus, rc_dev, rc_fun, cap_ptr + 0xc);
			link_cap_speed = data & 0xf;
			break;
		}

		return link_cap_speed;
	} else if (op == 1) {		/* hot reset */
		if (rc_bus == 0xffffffff)
			return -1;

		data = read_pci_cfg(rc_bus, rc_dev, rc_fun, 0x3c);
		write_pci_cfg(rc_bus, rc_dev, rc_fun, 0x3c, data | 0x400000);
		OSL_DELAY(50 * 1000);
		write_pci_cfg(rc_bus, rc_dev, rc_fun, 0x3c, data);
		return 0;
	}

	return 0;
} /* wl_osl_pcie_rc */

void
wl_dump_ver(wl_info_t *wl, struct bcmstrbuf *b)
{
#if defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 9))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdate-time"
#endif // endif
	bcm_bprintf(b, "wl%d: %s %s version %s\n", wl->pub->unit,
		__DATE__, __TIME__, EPI_VERSION_STR);
#if defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 9))
#pragma GCC diagnostic pop
#endif // endif
}

#if defined(BCMDBG)
static int
wl_dump(wl_info_t *wl, struct bcmstrbuf *b)
{
	wl_if_t *p;
	int i;

	wl_dump_ver(wl, b);

	bcm_bprintf(b, "name %s dev %p tbusy %d callbacks %d malloced %d\n",
		wl->dev->name, wl->dev, (uint)netif_queue_stopped(wl->dev),
		atomic_read(&wl->callbacks), MALLOCED(wl->osh));

	/* list all interfaces, skipping the primary one since it is printed above */
	p = wl->if_list;
	if (p)
		p = p->next;
	for (i = 0; p != NULL; p = p->next, i++) {
		if ((i % 4) == 0) {
			if (i != 0)
				bcm_bprintf(b, "\n");
			bcm_bprintf(b, "Interfaces:");
		}
		bcm_bprintf(b, " name %s dev %p", p->dev->name, p->dev);
	}
	if (i)
		bcm_bprintf(b, "\n");

#ifdef BCMDBG_CTRACE
	PKT_CTRACE_DUMP(wl->osh, b);
#endif // endif
	return 0;
}
#endif /* BCMDBG */

static void
wl_link_up(wl_info_t *wl, char *ifname)
{
	WL_ERROR(("wl%d: link up (%s)\n", wl->pub->unit, ifname));
}

static void
wl_link_down(wl_info_t *wl, char *ifname)
{
	WL_ERROR(("wl%d: link down (%s)\n", wl->pub->unit, ifname));
}

void
wl_event(wl_info_t *wl, char *ifname, wlc_event_t *e)
{
	/* skip processing if netdev was freed */
	if (!wl->dev)
		return;
#ifdef USE_IW
	wl_iw_event(wl->dev, &(e->event), e->data);
#endif /* USE_IW */

#if defined(BCM_BLOG)
	wl_handle_blog_event(wl, e);
#endif // endif

#if defined(USE_CFG80211)
	wl_cfg80211_event(wl->dev, &(e->event), e->data);
#endif // endif
	switch (e->event.event_type) {
	case WLC_E_LINK:
		if (e->event.flags&WLC_EVENT_MSG_LINK)
			wl_link_up(wl, ifname);
		else
			wl_link_down(wl, ifname);
		break;
#if defined(BCMSUP_PSK) && defined(STA)
	case WLC_E_MIC_ERROR: {
		wlc_bsscfg_t *cfg = wlc_bsscfg_find(wl->wlc, e->event.bsscfgidx, NULL);
		if (cfg == NULL)
			break;
		wl_mic_error(wl, cfg, e->addr,
			e->event.flags&WLC_EVENT_MSG_GROUP,
			e->event.flags&WLC_EVENT_MSG_FLUSHTXQ);
		break;
	}
#endif // endif
#if defined(WL_CONFIG_RFKILL)
	case WLC_E_RADIO: {
		mbool i;
		if (wlc_get(wl->wlc, WLC_GET_RADIO, &i) < 0)
			WL_ERROR(("%s: WLC_GET_RADIO failed\n", __FUNCTION__));
		if (wl->last_phyind == (mbool)(i & WL_RADIO_HW_DISABLE))
			break;

		wl->last_phyind = (mbool)(i & WL_RADIO_HW_DISABLE);

		WL_ERROR(("wl%d: Radio hardware state changed to %d\n", wl->pub->unit, i));
		wl_report_radio_state(wl);
		break;
	}
#else
	case WLC_E_RADIO:
		break;
#endif /* WL_CONFIG_RFKILL */
#if defined(DPSTA) && ((defined(STA) && defined(DWDS)) || defined(PSTA))
	case WLC_E_DPSTA_INTF_IND: {
		wl_dpsta_intf_event_t *dpsta_if = (wl_dpsta_intf_event_t *)(e->data);
		wlc_bsscfg_t *cfg = wlc_bsscfg_find(wl->wlc, e->event.bsscfgidx, NULL);

#if defined(STA) && defined(DWDS)
		if (dpsta_if->intf_type == WL_INTF_DWDS) {
			wl_dpsta_dwds_register(wl, cfg);
			break;
		}
#endif /* STA && DWDS */
#ifdef PSTA
		if (dpsta_if->intf_type == WL_INTF_PSTA) {
			wl_dpsta_psta_register(wl, cfg);
			break;
		}
#endif /* PSTA */
	}
#endif /* DPSTA && ((STA &&DWDS) || PSTA) */
	}
} /* wl_event */

#if defined(BCM_GMAC3)
/** BCM_GMAC3 specific function */
void
wl_fwder_event(wl_info_t *wl, char *ifname, wlc_event_t *e)
{
	bool attach;
	wl_if_t * wlif;

	if ((wl->fwdh == NULL) || !(e->addr) || (e->event.status != WLC_E_STATUS_SUCCESS))
		return;

	switch (e->event.event_type) {
		case WLC_E_AUTH_IND:
		case WLC_E_ASSOC_IND:
		case WLC_E_REASSOC_IND:
		case WLC_E_PRE_ASSOC_IND:
			attach = TRUE;
			break;

		case WLC_E_DEAUTH_IND:
		case WLC_E_DISASSOC_IND:
		case WLC_E_PRE_REASSOC_IND:
			attach = FALSE;
			break;

		default:
			return;
	}

	/* Find the wl_if to which this event is directed */
	for (wlif = wl->if_list; wlif != (wl_if_t *)NULL; wlif = wlif->next) {
		if ((wlif->dev != NULL) && (strcmp(ifname, wlif->dev->name) == 0))
			break;
	}

	if ((wlif == NULL) || (wlif->fwdh == FWDER_NULL))
		return;

	/* Add|Del station (Mac Address) to interface forwarding entry in WOFA */
	if (attach) {
		fwder_reassoc(wlif->fwdh, (uint16 *)e->addr, (uintptr_t)wlif->dev);
	} else {
		fwder_deassoc(wlif->fwdh, (uint16 *)e->addr, (uintptr_t)wlif->dev);
	}
} /* wl_fwder_event */
#endif /* BCM_GMAC3 */

void
wl_event_sync(wl_info_t *wl, char *ifname, wlc_event_t *e)
{
#ifndef LINUX_POSTMOGRIFY_REMOVAL
#ifdef WL_EVENTQ
	/* duplicate event for local event q */
	wl_eventq_dup_event(wl->wlevtq, e);
#endif /* WL_EVENTQ */
#endif /* LINUX_POSTMOGRIFY_REMOVAL */

#if defined(BCM_GMAC3)
	wl_fwder_event(wl, ifname, e);
#endif /* BCM_GMAC3 */
}

#ifndef WL_THREAD
/*
 * Called in non-passive mode when we need to send frames received on other CPU.
 */
static void BCMFASTPATH
wl_sched_tx_tasklet(void *t)
{
	wl_info_t *wl = (wl_info_t *)t;
	tasklet_schedule(&wl->tx_tasklet);
}

#if defined(BCM_GMAC3)
/**
 * Bypass transmit handler that will be invoked by GMAC forwarder.
 * A NIC mode forwarder may only forward a packet chain or a packet.
 * fwder is the downstream forwarder. !WL_THREAD specific.
 */
static int BCMFASTPATH
wl_forward(fwder_t * fwder, struct sk_buff *skb, int skb_cnt, struct net_device *rx_dev)
{
	wl_if_t *wlif;
	wl_info_t *wl;
	struct net_device * dev;

	/* Locate the TX network device using default dev or by looking up WOFA */
	if (fwder->devs_cnt == 1) {
		dev = fwder->dev_def; /* use forwarder's default device */
		if (dev == NULL)
			return FWDER_FAILURE;
	} else { /* Multiple interfaces: find interface using Eth DstAddress */
		uint16 *da = (uint16 *)(skb->data);

		if (ETHER_ISUCAST(da)) { /* fetch device by looking up WOFA */
			uintptr_t wofa_data = fwder_lookup(fwder, da, FWDER_GMAC_PORT);
			if (wofa_data == WOFA_DATA_INVALID)
				return FWDER_FAILURE;
			dev = (struct net_device *)wofa_data;
		} else { /* flood to all interfaces hosted by the downstream fwder. */
			ASSERT(skb_cnt == 1);

			fwder_flood(fwder, skb, fwder->mate->osh, TRUE, wl_start_try);
			PKTFRMNATIVE(fwder->mate->osh, skb);
			PKTFREE(fwder->mate->osh, skb, TRUE);
			return FWDER_SUCCESS;
		}
	}

	wlif = WL_DEV_IF(dev);
	if ((wlif->fwdh == FWDER_NULL) || !(dev->flags & IFF_UP))
		return -ENETDOWN;

	wl = WL_INFO_GET(dev);

	wl_start_int(wl, wlif, skb);

	return FWDER_SUCCESS;
} /* wl_forward */

/** !WL_THREAD specific */
static int BCMFASTPATH
wl_start_try(struct sk_buff *skb, struct net_device *dev, bool in_lock)
{
	BCM_REFERENCE(in_lock);
	return wl_start(skb, dev);
}

#endif /* BCM_GMAC3 */

static int32 BCMFASTPATH
wl_txq_xmit(wl_info_t *wl, struct sk_buff *skb)
{
	skb->prev = NULL;

	/* Lock the queue as tasklet could be running at this time */
	TXQ_LOCK(wl);

	PKTCNTR_INC(wl->wlc, PKTCNTR_MSDU_WL_IN_TXQ_TOT, 1);

	if ((wl_txq_thresh > 0) && (wl->txq_cnt >= wl_txq_thresh)) {
		PKTFRMNATIVE(wl->osh, skb);
		PKTCFREE(wl->osh, skb, TRUE);
		TXQ_UNLOCK(wl);
		PKTCNTR_INC(wl->wlc, PKTCNTR_MSDU_WL_IN_TXQ_DROP, 1);
		return BCME_OK;
	}

	if (wl->txq_head == NULL)
		wl->txq_head = skb;
	else
		wl->txq_tail->prev = skb;
	wl->txq_tail = skb;
	wl->txq_cnt++;

	if (!wl->txq_dispatched) {
		int32 err = 0;

		atomic_inc(&wl->callbacks);
		wl->txq_dispatched = TRUE;
		/* In smp non passive mode, schedule tasklet for tx */
		if (!WL_ALL_PASSIVE_ENAB(wl) && txworkq == 0)
			wl_sched_tx_tasklet(wl);
#ifdef WL_ALL_PASSIVE
#ifdef CONFIG_SMP
		else if (txworkq && wl->max_cpu_id > 0) {
			err = (int32)(SCHEDULE_WORK_ON(wl,
				wl->max_cpu_id - raw_smp_processor_id(),
				&wl->txq_task.work) == 0);
		}
#endif /* CONFIG_SMP */
		else {
#if defined(WL_USE_L34_THREAD)
			wl_thread_schedule_work(wl);
#else
			err = (int32)(SCHEDULE_WORK(wl, &wl->txq_task.work) == 0);
#endif /* WL_USE_L34_THREAD */
		}
#endif /* WL_ALL_PASSIVE */

		if (err) {
			wl->txq_dispatched = FALSE;
			WL_ERROR(("wl%d: wl_start/schedule_work failed\n",
			          wl->pub->unit));
			CALLBACK_DEC_AND_ASSERT(wl);
		}
	}

	TXQ_UNLOCK(wl);

	return BCME_OK;
}

/**
 * Transmit pkt. In PASSIVE mode, enqueue pkt to local queue,schedule task to
 * run, return this context. In non passive mode, directly call wl_start_int()
 * to transmit pkt. !WL_THREAD specific.
 */
static int BCMFASTPATH
wl_start(struct sk_buff *skb, struct net_device *dev)
{
	wl_if_t *wlif;
	wl_info_t *wl;

	if (!dev)
		return -ENETDOWN;

	wlif = WL_DEV_IF(dev);
	wl = WL_INFO_GET(dev);

#if defined(BCM_BLOG)
	if ((skb = wl_xlate_to_skb(wl, skb)) == NULL)
	{
		WLCNTINCR(wl->pub->_cnt->txnobuf);
		return BCME_OK;
	}
	wl_handle_blog_emit(wl, wlif, skb, dev);
#endif /* BCM_BLOG */

#if defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE) || \
	defined(WLBIN_COMPAT)
	if (wl_spdsvc_tx(skb, dev) == 0)
		return 0;
#endif /* CONFIG_BCM_SPDSVC || CONFIG_BCM_SPDSVC_MODULE || WLBIN_COMPAT */

	/* Call in the same context when we are UP and non-passive is enabled */
	if (WL_ALL_PASSIVE_ENAB(wl) || (WL_RTR() && WL_CONFIG_SMP())) {
		return wl_txq_xmit(wl, skb);
	} else {
		return wl_start_int(wl, wlif, skb);
	}

	return BCME_OK;
} /* wl_start */
#endif /* WL_THREAD */

void BCMFASTPATH
wl_start_txqwork(wl_task_t *task)
{
	wl_info_t *wl = (wl_info_t *)task->context;
	struct sk_buff *skb;
	int tx_count = 0;

	WL_TRACE(("wl%d: %s txq_cnt %d\n", wl->pub->unit, __FUNCTION__, wl->txq_cnt));

#ifdef BCMDBG
	if (wl->txq_cnt >= 500)
		WL_INFORM(("wl%d: WARNING dispatching over 500 packets in txqwork(%d)\n",
			wl->pub->unit, wl->txq_cnt));
#endif // endif

	/* XXX First remove an entry then go for execution.
	 * We cannot drop packet here without knowing the priority.
	 * Packets should be dropped in wlc txq. Therefore, the loop here
	 * is unbounded to avoid the queue growing unlimitedly.
	 */
	TXQ_LOCK(wl);
	while (wl->txq_head) {
		skb = wl->txq_head;
		wl->txq_head = skb->prev;
		skb->prev = NULL;
		if (wl->txq_head == NULL)
			wl->txq_tail = NULL;
		wl->txq_cnt--;
		TXQ_UNLOCK(wl);

		/* it has WL_LOCK/WL_UNLOCK inside */
		wl_start_int(wl, WL_DEV_IF(skb->dev), skb);

		TXQ_LOCK(wl);
		if (++tx_count >= wl_txq_bound) {
#ifdef BCMDBG
			WL_INFORM(("wl%d: tx_count %d / txbound %d, txq_head %p\n",
					wl->pub->unit, tx_count, wl_txq_bound, wl->txq_head));
#endif // endif
			break;
		}
	}

#if defined(WL_USE_L34_THREAD)
	if (!(wl->txq_head))
#endif /* WL_USE_L34_THREAD */
	{
		wl->txq_dispatched = FALSE;
		CALLBACK_DEC_AND_ASSERT(wl);
	}
	TXQ_UNLOCK(wl);
} /* wl_start_txqwork */

#if !defined(WL_USE_L34_THREAD)
static void BCMFASTPATH
wl_tx_tasklet(ulong data)
{
	wl_task_t task;
	task.context = (void *)data;
	wl_start_txqwork(&task);
}
#endif // endif

static void
wl_txq_free(wl_info_t *wl)
{
	struct sk_buff *skb;

	if (wl->txq_head == NULL) {
		ASSERT(wl->txq_tail == NULL);
		return;
	}

	while (wl->txq_head) {
		skb = wl->txq_head;
		wl->txq_head = skb->prev;
		wl->txq_cnt--;
		PKTFRMNATIVE(wl->osh, skb);
		PKTCFREE(wl->osh, skb, TRUE);
	}

	wl->txq_tail = NULL;
}

#ifdef WL_ALL_PASSIVE
static void
wl_set_multicast_list_workitem(struct work_struct *work)
{
	wl_task_t *task = (wl_task_t *)work;
	struct net_device *dev = (struct net_device*)task->context;
	wl_info_t *wl;

	wl = WL_INFO_GET(dev);

	CALLBACK_DEC_AND_ASSERT(wl);

	_wl_set_multicast_list(dev);
}

static void
wl_timer_task(wl_task_t *task)
{
	wl_timer_t *t = (wl_timer_t *)task->context;
	struct wl_info *wl = t->wl;

	_wl_timer(t);
	MFREE(wl->osh, task, sizeof(wl_task_t));

	/* This dec is for the task_schedule. The timer related
	 * callback is decremented in _wl_timer
	 */
	CALLBACK_DEC_AND_ASSERT(wl);
}
#endif /* WL_ALL_PASSIVE */

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
static void wl_timer(ulong data)
{
	wl_timer_t *t = (wl_timer_t *)data;
#else
static void wl_timer(struct timer_list *tmr)
{
	wl_timer_t *t = from_timer(t, tmr, timer);
#endif /* KERNEL_VERSION < 4.15 */
	if (!WL_ALL_PASSIVE_ENAB(t->wl)) {
		_wl_timer(t);
#ifdef WL_ALL_PASSIVE
	} else {
		atomic_set(&t->sched, 1);
		if (wl_schedule_task(t->wl, wl_timer_task, t) < 0)
			atomic_set(&t->sched, 0);
#endif /* WL_ALL_PASSIVE */
	}
}

static void
_wl_timer(wl_timer_t *t)
{
	wl_info_t *wl = t->wl;
	void (*timer_fn)(void *) = t->fn;
	void *timer_arg = t->arg;

	WL_LOCK(wl);

	if (t->set && (!timer_pending(&t->timer))) {
		if (t->periodic) {
			/* Periodic timer can't be a zero delay */
			ASSERT(t->ms != 0);

#if defined(BCMJTAG) || defined(BCMSLTGT)
			t->timer.expires = jiffies + t->ms*HZ/1000*htclkratio;
#else
			/* See the comment in the similar logic in wl_add_timer in this file but
			 * note in this case of re-programming a periodic timer, there has
			 * been a conscious decision to still add the +1 adjustment.  We want
			 * to guarantee that two consecutive callbacks are always AT LEAST the
			 * requested ms delay apart, even if this means the callbacks might "drift"
			 * from even the rounded ms to jiffy HZ period.
			 */
			t->timer.expires = jiffies + (t->ms*HZ+999)/1000 + 1;
#endif // endif
			atomic_inc(&wl->callbacks);
			add_timer(&t->timer);
			t->set = TRUE;
		} else
			t->set = FALSE;

#ifdef BCMDBG
		t->ticks++;
#endif // endif
#ifdef WL_ALL_PASSIVE
		/* need to clear sched flag as timer function may
		 * free itself so after this timer variable can no
		 * longer be referenced.
		 */
		atomic_set(&t->sched, 0);
#endif /* WL_ALL_PASSIVE */

		timer_fn(timer_arg);

#ifdef BCMDBG
		wlc_update_perf_stats(wl->wlc, WLC_PERF_STATS_TMR_DPC);
#endif // endif
	}
#ifdef WL_ALL_PASSIVE
	else {
		/* no timer function called but still need to clear
		 * the sched flag here.
		 */
		atomic_set(&t->sched, 0);
	}
#endif /* WL_ALL_PASSIVE */

	CALLBACK_DEC_AND_ASSERT(wl);

	WL_UNLOCK(wl);
} /* _wl_timer */

wl_timer_t *
wl_init_timer(wl_info_t *wl, void (*fn)(void *arg), void *arg, const char *tname)
{
	wl_timer_t *t;

	t = (wl_timer_t*)MALLOCZ(wl->osh, sizeof(wl_timer_t));

	if (t == NULL) {
		WL_ERROR(("wl%d: wl_init_timer: out of memory, malloced %d bytes\n",
			wl->unit, MALLOCED(wl->osh)));
		return 0;
	}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
	init_timer(&t->timer);
	t->timer.data = (ulong) t;
	t->timer.function = wl_timer;
#else
	timer_setup(&t->timer, wl_timer, 0);
#endif /* KERNEL_VERSION < 4.15 */
	t->wl = wl;
	t->fn = fn;
	t->arg = arg;
	t->next = wl->timers;
	wl->timers = t;

#ifdef BCMDBG
	if ((t->name = MALLOCZ(wl->osh, strlen(tname) + 1)))
		strncpy(t->name, tname, strlen(tname) + 1);
#endif // endif

	return t;
}

/* BMAC_NOTE: Add timer adds only the kernel timer since it's going to be more accurate
 * as well as it's easier to make it periodic
 */
void
wl_add_timer(wl_info_t *wl, wl_timer_t *t, uint ms, int periodic)
{
#ifdef BCMDBG
	if (t->set) {
		WL_INFORM(("%s: Already set. Name: %s, per %d\n",
			__FUNCTION__, t->name, periodic));
	}
#endif // endif
	/* ASSERT(!t->set); */

	/* Delay can't be zero for a periodic timer */
	ASSERT(periodic == 0 || ms != 0);

	t->ms = ms;
	t->periodic = (bool) periodic;

	/* if timer has been added, Just return w/ updated behavior */
	if (t->set)
		return;

	t->set = TRUE;
#if defined(BCMJTAG) || defined(BCMSLTGT)
	t->timer.expires = jiffies + ms*HZ/1000*htclkratio;
#else
	/* Make sure that you meet the guarantee of ms delay before
	 * calling the function.  You must consider both rounding to
	 * HZ and the fact that the next jiffy might be imminent, e.g.
	 * the timer interrupt is only a us away.
	 */
	if (ms == 0) {
		/* Zero is special - no HZ rounding up necessary nor
		 * accounting for an imminent timer tick.  Just use
		 * the current jiffies value.
		 */
		t->timer.expires = jiffies;
	} else {
		/* In converting ms to HZ, round up. Example: with HZ=250
		 * and thus a 4 ms jiffy/tick, round a 3 ms request to
		 * 1 jiffy, i.e. 4 ms.  In addition because the timer
		 * tick might occur imminently, you must add an extra
		 * jiffy/tick to guarantee the 3 ms request.
		 */
		t->timer.expires = jiffies + (ms*HZ+999)/1000 + 1;
	}
#endif /* defined(BCMJTAG) || defined(BCMSLTGT) */

	atomic_inc(&wl->callbacks);
	add_timer(&t->timer);
} /* wl_add_timer */

/* return TRUE if timer successfully deleted, FALSE if we deleted an inactive timer. */
bool
wl_del_timer(wl_info_t *wl, wl_timer_t *t)
{
	ASSERT(t);

	if (t->set) {
		t->set = FALSE;

		if (!del_timer(&t->timer)) {
#ifdef BCMDBG
			WL_INFORM(("wl%d: Deleted inactive timer %s.\n", wl->unit, t->name));
#endif /* BCMDBG */
#ifdef WL_ALL_PASSIVE
			/*
			 * The timer was inactive - this is normal in passive mode when we
			 * try to delete a timer after it fired, but before the associated
			 * task got scheduled.
			 */
			return TRUE;
#else
			return FALSE;
#endif // endif
		}
		CALLBACK_DEC_AND_ASSERT(wl);
	}

	return TRUE;
}

#ifdef WL_ALL_PASSIVE
void
wl_free_timer_freelist(wl_info_t *wl, bool force)
{
	wl_timer_t *t, *tmp;
	tmp = wl->timers_freelist;
	wl->timers_freelist = NULL;

	while (tmp) {
		t = tmp;
		tmp = tmp->next;
		t->next = NULL;

		if (!force && atomic_read(&t->sched)) {
			t->next = wl->timers_freelist;
			wl->timers_freelist = t;
			continue;
		}

#ifdef BCMDBG
		if (t->name)
			MFREE(wl->osh, t->name, strlen(t->name) + 1);
#endif /* BCMDBG */
		MFREE(wl->osh, t, sizeof(wl_timer_t));
	}
}
#endif /* WL_ALL_PASSIVE */

void
wl_free_timer(wl_info_t *wl, wl_timer_t *t)
{
	wl_timer_t timer_head, *p_timer = &timer_head;
	timer_head.next = wl->timers;

	wl_del_timer(wl, t);

	while (p_timer->next) {
		if (p_timer->next == t) {
			p_timer->next = t->next;
			break;
		}
		p_timer = p_timer->next;
	}
	wl->timers = timer_head.next;

#ifdef WL_ALL_PASSIVE
	if (WL_ALL_PASSIVE_ENAB(wl)) {
		t->next = wl->timers_freelist;
		wl->timers_freelist = t;
		wl_free_timer_freelist(wl, FALSE);
	} else
#endif /* WL_ALL_PASSIVE */
	{
#ifdef BCMDBG
		if (t->name)
			MFREE(wl->osh, t->name, strlen(t->name) + 1);
#endif /* BCMDBG */
		MFREE(wl->osh, t, sizeof(wl_timer_t));
		return;
	}
}

#if defined(BCMSUP_PSK) && defined(STA)
static void
wl_mic_error(wl_info_t *wl, wlc_bsscfg_t *cfg, struct ether_addr *ea, bool group, bool flush_txq)
{
	WL_WSEC(("wl%d: mic error using %s key\n", wl->pub->unit,
		(group) ? "group" : "pairwise"));

	if (wlc_sup_mic_error(cfg, group))
		return;
}
#endif /* defined(BCMSUP_PSK) && defined(STA) */

void
wl_monitor(wl_info_t *wl, struct wl_rxsts *rxsts, void *p)
{
#ifdef WL_MONITOR
	struct sk_buff *oskb = (struct sk_buff *)p;
	struct sk_buff *skb = NULL;
	uint16 len = 0, olen = 0;
	int16 offset;
	void *local_oskb_data;
	uint8 *p1data;

	WL_TRACE(("wl%d: wl_monitor\n", wl->pub->unit));

	if (!wl->monitor_dev) {
		WL_ERROR(("wl%d: %s: monitor_dev is NULL\n", wl->pub->unit, __FUNCTION__));
		return;
	}

	local_oskb_data = (void *)(PKTDATA(wl->wlc->osh, p) +
		D11_PHY_RXPLCP_LEN(wl->wlc->pub->corerev));

	len = PKTLEN(wl->osh, oskb) - D11_PHY_RXPLCP_LEN(wl->wlc->pub->corerev);
	olen = len;

	if (oskb->next) {
		struct sk_buff *amsdu_p = oskb->next;
		uint amsdu_len = 0;
		while (amsdu_p) {
			amsdu_len += amsdu_p->len;
			amsdu_p = amsdu_p->next;
		}
		len += amsdu_len;
	}

	if ((skb = dev_alloc_skb(len + MAX_RADIOTAP_SIZE)) == NULL)
		return;

	skb_put(skb, len + MAX_RADIOTAP_SIZE);
	len = wl_rxsts_to_rtap(rxsts, local_oskb_data, len, skb->data, &offset);

	ASSERT(offset <= MAX_RADIOTAP_SIZE);
	/* Update actual len of skb: len = (actual rtap header len) + (data len) */

	skb_trim(skb, len);

	p1data = skb->data;
	p1data += offset; /* Copy data after rtap header */
	memcpy(p1data, local_oskb_data, olen);

	if ((rxsts->nfrmtype & WL_RXS_NFRM_AMSDU_FIRST) ||
		(rxsts->nfrmtype & WL_RXS_NFRM_AMSDU_SUB)) {
		uint8* pdata = p1data + olen;
		/* copy MSDU chain to payload portion of radiotap header */
		if (oskb->next) {
			struct sk_buff *amsdu_p = oskb->next;
			while (amsdu_p) {
				memcpy(pdata, amsdu_p->data, amsdu_p->len);
				pdata += amsdu_p->len;
				amsdu_p = amsdu_p->next;
			}
		}
	}

	skb->dev = wl->monitor_dev;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 11, 0)
	skb->dev->last_rx = jiffies;
#endif /* KERNEL_VERSION < 4.11 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 22)
	skb_reset_mac_header(skb);
#else
	skb->mac.raw = skb->data;
#endif /* KERNEL_VERSION >= 2.6.22 */
	skb->ip_summed = CHECKSUM_NONE;
	skb->pkt_type = PACKET_OTHERHOST;
	skb->protocol = htons(ETH_P_80211_RAW);
#ifdef NAPI_POLL
	netif_receive_skb(skb);
#else
	netif_rx(skb);
#endif /* NAPI_POLL */
#endif /* WL_MONITOR */
}

#ifdef WL_MONITOR

static int
wl_monitor_start(struct sk_buff *skb, struct net_device *dev)
{
	wl_info_t *wl;

	wl = WL_DEV_IF(dev)->wl;
	PKTFREE(wl->osh, skb, FALSE);
	return 0;
}

/**
 * Create a virtual interface. Call only from safe time!
 * can't call register_netdev with WL_LOCK
 *
 * Equivalent to _wl_add_if
 */
static void
_wl_add_monitor_if(wl_task_t *task)
{
	struct net_device *dev;
	wl_if_t *wlif = (wl_if_t *) task->context;
	wl_info_t *wl = wlif->wl;

	WL_TRACE(("wl%d: %s\n", wl->pub->unit, __FUNCTION__));
	ASSERT(wl);
	ASSERT(!wl->monitor_dev);

	if ((dev = wl_alloc_linux_if(wlif)) == NULL) {
		WL_ERROR(("wl%d: %s: wl_alloc_linux_if failed\n", wl->pub->unit, __FUNCTION__));
		goto done;
	}

	/* Copy temp to real name */
	ASSERT(strlen(wlif->name) > 0);
	strncpy(wlif->dev->name, wlif->name, strlen(wlif->name) + 1);

	dev->type = ARPHRD_IEEE80211_RADIOTAP;

	/* override some fields */
	bcopy(wl->dev->dev_addr, dev->dev_addr, ETHER_ADDR_LEN);

	/* initialize dev fn pointers */
#if defined(WL_USE_NETDEV_OPS)
	dev->netdev_ops = &wl_netdev_monitor_ops;
#else
	dev->hard_start_xmit = wl_monitor_start;
	dev->do_ioctl = wl_ioctl;
	dev->get_stats = wl_get_stats;
#endif /* WL_USE_NETDEV_OPS */

	if (register_netdev(dev)) {
		WL_ERROR(("wl%d: %s, register_netdev failed for %s\n",
			wl->pub->unit, __FUNCTION__, dev->name));
		wl->monitor_dev = NULL;
		goto done;
	}

	/* Move monitor_dev assignment to here avoid panic */
	wl->monitor_dev = dev;
	wlif->dev_registered = TRUE;

done:
	MFREE(wl->osh, task, sizeof(wl_task_t));
	CALLBACK_DEC_AND_ASSERT(wl);
} /* _wl_add_monitor_if */

static void
_wl_del_monitor(wl_task_t *task)
{
	wl_info_t *wl = (wl_info_t *) task->context;

	ASSERT(wl);
	ASSERT(wl->monitor_dev);

	WL_TRACE(("wl%d: _wl_del_monitor\n", wl->pub->unit));

	wl_free_if(wl, WL_DEV_IF(wl->monitor_dev), TRUE);
	wl->monitor_dev = NULL;

	MFREE(wl->osh, task, sizeof(wl_task_t));
	CALLBACK_DEC_AND_ASSERT(wl);
}

#endif /* WL_MONITOR */

/**
 * Create a dedicated monitor interface since libpcap caches the
 * packet type when it opens the device. The protocol type in the skb
 * is dropped somewhere in libpcap, and every received frame is tagged
 * with the DLT/ARPHRD type that's read by libpcap when the device is
 * opened.
 *
 * If libpcap was fixed to handle per-packet link types, we might not
 * need to create a pseudo device at all, wl_set_monitor() would be
 * unnecessary, and wlc->monitor could just get set in wlc_ioctl().
 */
/* Equivalent to wl_add_if */
void
wl_set_monitor(wl_info_t *wl, int val)
{
#ifdef WL_MONITOR
	const char *devname;
	wl_if_t *wlif;

	WL_TRACE(("wl%d: wl_set_monitor: val %d\n", wl->pub->unit, val));
	if ((val && wl->monitor_dev) || (!val && !wl->monitor_dev) ||
		(!val && !wl->monitor_type)) {
		WL_ERROR(("%s: Mismatched params, return\n", __FUNCTION__));
		return;
	}

	/* Delete monitor */
	if (!val) {
		wl->monitor_type = val;
		(void) wl_schedule_task(wl, _wl_del_monitor, wl);
		return;
	}

	/* Add monitor */
	if (val >= 1 && val <= 3) {
		wl->monitor_type = val;
	} else {
		WL_ERROR(("monitor type %d not supported\n", val));
		ASSERT(0);
	}

	wlif = wl_alloc_if(wl, WL_IFTYPE_MON, wl->pub->unit, NULL);
	if (!wlif) {
		WL_ERROR(("wl%d: %s: alloc wlif failed\n", wl->pub->unit, __FUNCTION__));
		return;
	}

	/* netdev isn't ready yet so stash name here for now and
	   copy into netdev when it becomes ready
	 */
	devname = "radiotap";
	(void)snprintf(wlif->name, sizeof(wlif->name), "%s%d", devname, wl->pub->unit);

	if (wl_schedule_task(wl, _wl_add_monitor_if, wlif)) {
		MFREE(wl->osh, wlif, sizeof(wl_if_t));
		return;
	}
#endif /* WL_MONITOR */
} /* wl_set_monitor */

#if LINUX_VERSION_CODE == KERNEL_VERSION(2, 6, 15)
/*
 * Fedora did it again, the kernel shipped in the original CD images has a flaw
 * which prevents non-gpl drivers from loading with:
 *	wl: Unknown symbol print_tainted
 *
 * This is their explanation:
 *
 *	From: Dave Jones <davej redhat com>
 *	To: For testers of Fedora Core development releases <fedora-test-list redhat com>
 *	Subject: Re: Kernel 2054 breaks nvidia.ko loading
 *	Date: Wed, 15 Mar 2006 21:20:48 -0500
 *
 *	An oversight on my part.
 *	print_tainted is used in the spinlock macros, so essentially,
 *	the macro is made GPL-only too.  I've reverted that change
 *	in cvs, but it's too late for the final FC5 image, which is
 *	already being pushed out to mirrors.
 *
 * In that thread somebody suggested just adding a print_tainted to your driver,
 * and I could not figure out a more elegant workaround, so here it is. Ideally
 * I would want to ifdef it
 *	#if UTS_RELEASE == "2.6.15-1.2054_FC5"
 * but that is not valid perprocessor syntax, so I'm stuck with just testing
 * for 2.6.15.
 */
const char *
print_tainted()
{
	return "";
}
#endif /* LINUX_VERSION_CODE == KERNEL_VERSION(2, 6, 15) */

#ifdef BCMJTAG
/* attach to d11 core thru jtag */
/* venid and devid are pci vendor id and pci device id */
static void *
wl_jtag_probe(uint16 venid, uint16 devid, void *regsva, void *param)
{
	wl_info_t *wl;

	if (!wlc_chipmatch(venid, devid)) {
		WL_ERROR(("wl_jtag_probe: wlc_chipmatch failed\n"));
		return NULL;
	}

	if (!(wl = wl_attach(venid, devid, (ulong)regsva, JTAG_BUS, param, 0))) {
		WL_ERROR(("wl_jtag_probe: wl_attach failed\n"));
		return NULL;
	}

	return wl;
}

/* detach from d11 core */
static void
wl_jtag_detach(void *wl)
{
	WL_LOCK((wl_info_t *)wl);
	wl_down((wl_info_t *)wl);
	WL_UNLOCK((wl_info_t *)wl);
	wl_free((wl_info_t *)wl);
}

/* poll d11 core */
static void
wl_jtag_poll(void *wl)
{
	WL_ISR(0, (wl_info_t *)wl, NULL);
}
#endif /* BCMJTAG */

#if defined(PLATFORM_INTEGRATED_WIFI) && defined(CONFIG_OF)
static int
wl_plat_drv_probe(struct platform_device *pdev)
{
	struct wl_platform_info *plat;
	struct resource *r;
	int error = 0;
	wl_info_t	*wl;

	plat = devm_kzalloc(&pdev->dev, sizeof(*plat), GFP_KERNEL);
	if (!plat) {
		return -ENOMEM;
	}
	bzero(plat, sizeof(struct wl_platform_info));

	plat->pdev = pdev;

	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	plat->regs = devm_ioremap_resource(&pdev->dev, r);
	if (IS_ERR(plat->regs)) {
		return PTR_ERR(plat->regs);
	}

	plat->irq = platform_get_irq(pdev, 0);
	if (plat->irq < 0) {
		return plat->irq;
	}

#ifdef STB_SOC_WIFI
	error = wl_stbsoc_init(plat);
	if (error != BCME_OK)
		return error;

	plat->deviceid = wl_stbsoc_get_devid();
#else
	{
		const char *var = getvar(NULL, "devid");
		if (var)
			plat->deviceid = bcm_strtoul(var, NULL, 0);
	}
#endif /* STB_SOC_WIFI */

	wl = wl_attach(VENDOR_BROADCOM, plat->deviceid, SI_ENUM_BASE_DEFAULT /* regs */,
		SI_BUS /* bus type */, &pdev->dev /* btparam or dev */, plat->irq,
		NULL /* BAR1_ADDR */, 0 /* BAR1_SIZE */, NULL /* BAR2_ADDR */,
		0 /* BAR2_SIZE */,NULL /* private data */);

	if (!wl) {
		error = -ENODEV;
	} else {
		wl->plat_info = plat;
		platform_set_drvdata(pdev, wl);
	}

	return error;
}

static int
wl_plat_drv_remove(struct platform_device *pdev)
{
	wl_info_t	*wl;

	wl = (wl_info_t *)platform_get_drvdata(pdev);

	WL_LOCK((wl_info_t *)wl);
	wl_down((wl_info_t *)wl);
	WL_UNLOCK((wl_info_t *)wl);

#ifdef STB_SOC_WIFI
	wl_stbsoc_deinit(wl->plat_info);
#endif /* STB_SOC_WIFI */

	if (wl->plat_info)
		devm_kfree(&pdev->dev, (wl_info_t *)wl->plat_info);

	wl_free((wl_info_t *)wl);

	return 0;
}

static void
wl_plat_drv_shutdown(struct platform_device *pdev)
{
	wl_info_t	*wl;

	wl = (wl_info_t *)platform_get_drvdata(pdev);

#ifdef STB_SOC_WIFI
	wl_stbsoc_deinit(wl->plat_info);
#endif /* STB_SOC_WIFI */

	if (wl->plat_info)
		devm_kfree(&pdev->dev, (wl_info_t *)wl->plat_info);

	wl_free((wl_info_t *)wl);

	return;
}

static int
wl_plat_drv_suspend(struct platform_device *pdev, pm_message_t state)
{
	BCM_REFERENCE(pdev);
	BCM_REFERENCE(state);
	return 0;
}

static int
wl_plat_drv_resume(struct platform_device *pdev)
{
	BCM_REFERENCE(pdev);
	return 0;
}
#endif /* PLATFORM_INTEGRATED_WIFI && CONFIG_OF */

struct net_device *
wl_netdev_get(wl_info_t *wl)
{
	return wl->dev;
}

#ifdef LINUX_CRYPTO

int
wl_tkip_miccheck(wl_info_t *wl, void *p, int hdr_len, bool group_key, int key_index)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 14)
	struct sk_buff *skb = (struct sk_buff *)p;
	skb->dev = wl->dev;

	if (wl->tkipmodops) {
		if (group_key && wl->tkip_bcast_data[key_index])
			return (wl->tkipmodops->decrypt_msdu(skb, key_index, hdr_len,
				wl->tkip_bcast_data[key_index]));
		else if (!group_key && wl->tkip_ucast_data)
			return (wl->tkipmodops->decrypt_msdu(skb, key_index, hdr_len,
				wl->tkip_ucast_data));
	}
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 14) */
	WL_ERROR(("%s: No tkip mod ops\n", __FUNCTION__));

	return -1;
}

int
wl_tkip_micadd(wl_info_t *wl, void *p, int hdr_len)
{
	int error = -1;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 14)
	struct sk_buff *skb = (struct sk_buff *)p;
	skb->dev = wl->dev;

	if (wl->tkipmodops) {
		if (wl->tkip_ucast_data)
			error = wl->tkipmodops->encrypt_msdu(skb, hdr_len, wl->tkip_ucast_data);
		if (error)
			WL_ERROR(("Error encrypting MSDU %d\n", error));
	} else
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 14) */
		WL_ERROR(("%s: No tkip mod ops\n", __FUNCTION__));
	return error;
}

int
wl_tkip_encrypt(wl_info_t *wl, void *p, int hdr_len)
{
	int error = -1;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 14)
	struct sk_buff *skb = (struct sk_buff *)p;
	skb->dev = wl->dev;

	if (wl->tkipmodops) {
		if (wl->tkip_ucast_data)
			error = wl->tkipmodops->encrypt_mpdu(skb, hdr_len, wl->tkip_ucast_data);
		if (error) {
			WL_ERROR(("Error encrypting MPDU %d\n", error));
		}
	} else
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 14) */
		WL_ERROR(("%s: No tkip mod ops\n", __FUNCTION__));
	return error;

}

int
wl_tkip_decrypt(wl_info_t *wl, void *p, int hdr_len, bool group_key)
{
	int err = -1;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 14)
	struct sk_buff *skb = (struct sk_buff *)p;
	uint8 *pos;
	uint8 key_idx = 0;

	if (group_key) {
		skb->dev = wl->dev;
		pos = skb->data + hdr_len;
		key_idx = pos[3];
		key_idx >>= 6;
		WL_ERROR(("%s: Invalid key_idx %d\n", __FUNCTION__, key_idx));
		ASSERT(key_idx < NUM_GROUP_KEYS);
	}

	if (wl->tkipmodops) {
		if (group_key && key_idx < NUM_GROUP_KEYS && wl->tkip_bcast_data[key_idx])
			err = wl->tkipmodops->decrypt_mpdu(skb, hdr_len,
				wl->tkip_bcast_data[key_idx]);
		else if (!group_key && wl->tkip_ucast_data)
			err = wl->tkipmodops->decrypt_mpdu(skb, hdr_len, wl->tkip_ucast_data);
	} else {
		WL_ERROR(("%s: No tkip mod ops\n", __FUNCTION__));
	}

#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 14) */

	/* Error */
	return err;
} /* wl_tkip_decrypt */

int
wl_tkip_keyset(wl_info_t *wl, const wlc_key_info_t *key_info,
	const uint8 *key_data, size_t key_len,
	const uint8 *rx_seq, size_t rx_seq_len)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 14)
	bool group_key = FALSE;
	uint8 key_buf[32];

	ASSERT(key_info != NULL);
	if (key_len != 0) {
		ASSERT(key_len == 32);
		ASSERT(rx_seq_len == 6);
		WL_WSEC(("%s: Key Length is Not zero\n", __FUNCTION__));
		if (key_info->algo != CRYPTO_ALGO_TKIP) {
			WL_WSEC(("%s: Algo is Not TKIP %d\n", __FUNCTION__, key_info->algo));
			return 0;
		}
		WL_WSEC(("%s: Trying to set a key in TKIP Mod\n", __FUNCTION__));
	} else {
		WL_WSEC(("%s: Trying to Remove a Key from TKIP Mod\n", __FUNCTION__));
	}

	if (key_info->flags & WLC_KEY_FLAG_GROUP) {
		group_key = TRUE;
		WL_WSEC(("Group Key index %d\n", key_info->key_id));
	} else {
		WL_WSEC(("Unicast Key index %d\n", key_info->key_id));
	}

	if (wl->tkipmodops) {
		if (group_key) {
			if (key_len) {
				if (!wl->tkip_bcast_data[key_info->key_id]) {
					WL_WSEC(("Init TKIP Bcast Module\n"));
					WL_UNLOCK(wl);
					wl->tkip_bcast_data[key_info->key_id] =
						wl->tkipmodops->init(key_info->key_id);
					WL_LOCK(wl);
				}
				if (wl->tkip_bcast_data[key_info->key_id]) {
					WL_WSEC(("TKIP SET BROADCAST KEY******************\n"));
					bcopy(key_data, key_buf, 16);
					bcopy(key_data+24, key_buf + 16, 8);
					bcopy(key_data+16, key_buf + 24, 8);

/* set_key defined by linux kernel so that 3 parameter is not const, so
 * must disable -Wcast-qual warning
 */
#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == \
	4 && __GNUC_MINOR__ >= 6))
_Pragma("GCC diagnostic push")
_Pragma("GCC diagnostic ignored \"-Wcast-qual\"")
#endif // endif
					wl->tkipmodops->set_key(key_buf, key_len, (uint8 *)rx_seq,
						wl->tkip_bcast_data[key_info->key_id]);
#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == \
	4 && __GNUC_MINOR__ >= 6))
_Pragma("GCC diagnostic pop")
#endif // endif
				}
			} else {
				if (wl->tkip_bcast_data[key_info->key_id]) {
					WL_WSEC(("Deinit TKIP Bcast Module\n"));
					wl->tkipmodops->deinit(
						wl->tkip_bcast_data[key_info->key_id]);
					wl->tkip_bcast_data[key_info->key_id] = NULL;
				}
			}
		} else {
			if (key_len) {
				if (!wl->tkip_ucast_data) {
					WL_WSEC(("Init TKIP Ucast Module\n"));
					WL_UNLOCK(wl);
					wl->tkip_ucast_data =
						wl->tkipmodops->init(key_info->key_id);
					WL_LOCK(wl);
				}
				if (wl->tkip_ucast_data) {
					WL_WSEC(("TKIP SET UNICAST KEY******************\n"));
					bcopy(key_data, key_buf, 16);
					bcopy(key_data+24, key_buf + 16, 8);
					bcopy(key_data+16, key_buf + 24, 8);
/* set_key defined by linux kernel so that 3 parameter is not const, so
 * must disable -Wcast-qual warning
 */
#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == \
	4 && __GNUC_MINOR__ >= 6))
_Pragma("GCC diagnostic push")
_Pragma("GCC diagnostic ignored \"-Wcast-qual\"")
#endif // endif
					wl->tkipmodops->set_key(key_buf, key_len,
						(uint8 *)rx_seq, wl->tkip_ucast_data);
#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == \
	4 && __GNUC_MINOR__ >= 6))
_Pragma("GCC diagnostic pop")
#endif // endif

				}
			} else {
				if (wl->tkip_ucast_data) {
					WL_WSEC(("Deinit TKIP Ucast Module\n"));
					wl->tkipmodops->deinit(wl->tkip_ucast_data);
					wl->tkip_ucast_data = NULL;
				}
			}
		}
	} else
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 14) */
		WL_WSEC(("%s: No tkip mod ops\n", __FUNCTION__));

	return 0;
} /* wl_tkip_keyset */

void
wl_tkip_printstats(wl_info_t *wl, bool group_key)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 14)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0)
	struct seq_file sfile;
	struct seq_file *debug_buf = &sfile;
#else
	char debug_buf[512];
#endif // endif
	int idx;
	if (wl->tkipmodops) {
		if (group_key) {
			for (idx = 0; idx < NUM_GROUP_KEYS; idx++) {
				if (wl->tkip_bcast_data[idx])
					wl->tkipmodops->print_stats(debug_buf,
						wl->tkip_bcast_data[idx]);
			}
		} else if (!group_key && wl->tkip_ucast_data)
			wl->tkipmodops->print_stats(debug_buf, wl->tkip_ucast_data);
		else
			return;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0)
		printk("%s: TKIP stats from module: %s\n",
			debug_buf->buf, group_key?"Bcast":"Ucast");
#else
		printk("%s: TKIP stats from module: %s\n",
			debug_buf, group_key?"Bcast":"Ucast");
#endif // endif
	}
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 14) */
}

#endif /* LINUX_CRYPTO */

#if defined(CONFIG_BCM_WLAN_DGASP)
void wl_shutdown_handler(wl_info_t *wl)
{
	wlc_shutdown_handler(wl->wlc);
}
#endif /* CONFIG_BCM_WLAN_DGASP */

#if defined(WL_CONFIG_RFKILL)   /* Rfkill support */

static int
wl_set_radio_block(void *data, bool blocked)
{
	wl_info_t *wl = data;
	uint32 radioval;

	WL_TRACE(("%s: kernel set blocked = %d\n", __FUNCTION__, blocked));

	radioval = WL_RADIO_SW_DISABLE << 16 | blocked;

	WL_LOCK(wl);

	if (wlc_set(wl->wlc, WLC_SET_RADIO, radioval) < 0) {
		WL_ERROR(("%s: SET_RADIO failed\n", __FUNCTION__));
		return 1;
	}

	WL_UNLOCK(wl);

	return 0;
}

static const struct rfkill_ops bcmwl_rfkill_ops = {
	.set_block = wl_set_radio_block
};

static int
wl_init_rfkill(wl_info_t *wl)
{
	int status;
	struct device *dev;

#if defined(PLATFORM_INTEGRATED_WIFI) && defined(CONFIG_OF)
	dev = &wl->plat_info->pdev->dev;
#else /* PLATFORM_INTEGRATED_WIFI && CONFIG_OF */
	dev = &wl->dev->dev;
#endif /* PLATFORM_INTEGRATED_WIFI && CONFIG_OF */

	(void)snprintf(wl->wl_rfkill.rfkill_name, sizeof(wl->wl_rfkill.rfkill_name),
	"brcmwl-%d", wl->pub->unit);

	wl->wl_rfkill.rfkill = rfkill_alloc(wl->wl_rfkill.rfkill_name, dev,
	RFKILL_TYPE_WLAN, &bcmwl_rfkill_ops, wl);

	if (!wl->wl_rfkill.rfkill) {
		WL_ERROR(("%s: RFKILL: Failed to allocate rfkill\n", __FUNCTION__));
		return -ENOMEM;
	}

	if (wlc_get(wl->wlc, WLC_GET_RADIO, &status) < 0) {
		WL_ERROR(("%s: WLC_GET_RADIO failed\n", __FUNCTION__));
		return 1;
	}

	rfkill_init_sw_state(wl->wl_rfkill.rfkill, status);

	if (rfkill_register(wl->wl_rfkill.rfkill)) {
		WL_ERROR(("%s: rfkill_register failed! \n", __FUNCTION__));
		rfkill_destroy(wl->wl_rfkill.rfkill);
		return 2;
	}

	WL_ERROR(("%s: rfkill registered\n", __FUNCTION__));
	wl->wl_rfkill.registered = TRUE;
	return 0;
}

static void
wl_uninit_rfkill(wl_info_t *wl)
{
	if (wl->wl_rfkill.registered) {
		rfkill_unregister(wl->wl_rfkill.rfkill);
		rfkill_destroy(wl->wl_rfkill.rfkill);
		wl->wl_rfkill.registered = FALSE;
		wl->wl_rfkill.rfkill = NULL;
	}
}

static void
wl_report_radio_state(wl_info_t *wl)
{
	WL_TRACE(("%s: report radio state %d\n", __FUNCTION__, wl->last_phyind));

	rfkill_set_hw_state(wl->wl_rfkill.rfkill, wl->last_phyind != 0);
}

#endif /* WL_CONFIG_RFKILL */

static void
wl_linux_watchdog(void *ctx)
{
	wl_info_t *wl = (wl_info_t *) ctx;
	struct net_device_stats *stats = NULL;
	uint id;
	wl_if_t *wlif;
	wl_if_stats_t wlif_stats;
#ifdef USE_IW
	struct iw_statistics *wstats = NULL;
	int phy_noise;
#endif // endif
	if (wl == NULL)
		return;

	if (wl->if_list) {
		for (wlif = wl->if_list; wlif != NULL; wlif = wlif->next) {
			memset(&wlif_stats, 0, sizeof(wl_if_stats_t));
			wlc_wlcif_stats_get(wl->wlc, wlif->wlcif, &wlif_stats);

			/* refresh stats */
			if (wl->pub->up) {
				ASSERT(wlif->stats_id < 2);

				id = 1 - wlif->stats_id;
				stats = &wlif->stats_watchdog[id];
				if (stats) {
					stats->rx_packets = WLCNTVAL(wlif_stats.rxframe);
					stats->tx_packets = WLCNTVAL(wlif_stats.txframe);
					stats->rx_bytes = WLCNTVAL(wlif_stats.rxbyte);
					stats->tx_bytes = WLCNTVAL(wlif_stats.txbyte);
					stats->rx_errors = WLCNTVAL(wlif_stats.rxerror);
					stats->tx_errors = WLCNTVAL(wlif_stats.txerror);
					stats->collisions = 0;
					stats->rx_length_errors = 0;
					/*
					 * Stats which are not kept per interface
					 * come from per radio stats
					 */
					stats->rx_over_errors = WLCNTVAL(wl->pub->_cnt->rxoflo);
					stats->rx_crc_errors = WLCNTVAL(wl->pub->_cnt->rxcrc);
					stats->rx_frame_errors = 0;
					stats->rx_fifo_errors = WLCNTVAL(wl->pub->_cnt->rxoflo);
					stats->rx_missed_errors = 0;
					stats->tx_fifo_errors = 0;
				}

#ifdef USE_IW
				wstats = &wlif->wstats_watchdog[id];
				if (wstats) {
#if WIRELESS_EXT > 11
					wstats->discard.nwid = 0;
					wstats->discard.code = WLCNTVAL(wl->pub->_cnt->rxundec);
					wstats->discard.fragment = WLCNTVAL(wlif_stats.rxfragerr);
					wstats->discard.retries = WLCNTVAL(wlif_stats.txfail);
					wstats->discard.misc = WLCNTVAL(wl->pub->_cnt->rxrunt) +
						WLCNTVAL(wl->pub->_cnt->rxgiant);
					wstats->miss.beacon = 0;
#endif /* WIRELESS_EXT > 11 */
				}
#endif /* USE_IW */

				wlif->stats_id = id;
			}
#ifdef USE_IW
			if (!wlc_get(wl->wlc, WLC_GET_PHY_NOISE, &phy_noise))
				wlif->phy_noise = phy_noise;
#endif /* USE_IW */

		}
	}

#ifdef CTFPOOL
	/* allocate and add a new skbs to the pkt pool */
	if (CTF_ENAB(wl->cih))
		osl_ctfpool_replenish(wl->osh, CTFPOOL_REFILL_THRESH);
#endif /* CTFPOOL */
} /* wl_linux_watchdog */

#if defined(WLVASIP)
uint32 wl_pcie_bar1(struct wl_info *wl, uchar** addr)
{
	*addr = wl->bar1_addr;
	return (wl->bar1_size);
}

uint32 wl_pcie_bar2(struct wl_info *wl, uchar** addr)
{
	*addr = wl->bar2_addr;
	return (wl->bar2_size);
}
#endif /* WLVASIP */

#ifdef DPSTA
#if defined(STA) && defined(DWDS)

static bool
wl_dwds_is_ds_sta(wl_info_t *wl, wlc_bsscfg_t *bsscfg, struct ether_addr *mac)
{
	bool ret;
	wlc_bsscfg_t *cfg = wlc_bsscfg_find_by_wlcif(wl->wlc, NULL);

	ret = wlc_dwds_is_ds_sta(cfg->wlc, mac);

	return ret;
}

static bool
wl_dwds_authorized(wl_info_t *wl, wlc_bsscfg_t *cfg)
{
	bool ret;

	ret = wlc_dwds_authorized(cfg);

	return ret;
}

static void
wl_dpsta_dwds_register(wl_info_t *wl, wlc_bsscfg_t *pcfg)
{
	psta_if_api_t api;

	/* Register dwds sta APIs with DPSTA module */
	api.is_ds_sta = (bool (*)(void *, void *, struct ether_addr *))wl_dwds_is_ds_sta;
	api.psta_find = (void *(*)(void *, void *, uint8 *)) NULL;
	api.bss_auth = (bool (*)(void *, void *))wl_dwds_authorized;
	api.wl = wl;
	api.psta = pcfg->wlc;
	api.bsscfg = pcfg;
	api.mode = DPSTA_MODE_DWDS;
	(void) dpsta_register(wl->pub->unit, &api);

	return;
}

#endif /* STA && DWDS */

#ifdef PSTA
static bool
wl_psta_is_ds_sta(wl_info_t *wl, wlc_bsscfg_t *cfg, struct ether_addr *mac)
{
	bool ret;

	ret = wlc_psta_is_ds_sta(cfg->wlc, mac);

	return ret;
}

static bool
wl_psta_find_dpsta(wl_info_t *wl, wlc_bsscfg_t *cfg, uint8 *mac)
{
	bool ret;

	ret = wlc_psta_find_dpsta(cfg->wlc, mac);

	return ret;
}

static bool
wl_psta_authorized(wl_info_t *wl, wlc_bsscfg_t *cfg)
{
	bool ret;

	ret = wlc_psta_authorized(cfg);

	return ret;
}

static void
wl_dpsta_psta_register(wl_info_t *wl, wlc_bsscfg_t *pcfg)
{
	psta_if_api_t api;

	/* Register proxy sta APIs with DPSTA module */
	api.is_ds_sta = (bool (*)(void *, void *, struct ether_addr *))wl_psta_is_ds_sta;
	api.psta_find = (void *(*)(void *, void *, uint8 *))wl_psta_find_dpsta;
	api.bss_auth = (bool (*)(void *, void *))wl_psta_authorized;
	api.wl = wl;
	api.psta = wlc_psta_get_psta(pcfg->wlc);
	api.bsscfg = pcfg;
	api.mode = DPSTA_MODE_PSTA;
	(void) dpsta_register(wl->pub->unit, &api);

	return;
}
#endif /* PSTA */
#endif /* DPSTA */

int wl_fatal_error(void * wl, int rc)
{
	return FALSE;
}

/**
 * Called from either this file, or from one of the cfg80211 specific source files
 * @param rtnl_is_needed   Should be false if the call path leading to this function already
 *                         acquired the rtnetlink mutex.
 */
int wl_register_interface(void *pub, int ifidx, struct net_device *new_dev, bool rtnl_is_needed)
{
	priv_link_t *priv_link;
	wl_if_t *wlif;
	wl_info_t *wl;
	bool dev_registered = FALSE;

	priv_link = netdev_priv(new_dev);
	wlif = priv_link->wlif;
	if (!wlif || !wlif->wlcif) {
		goto done;
	}
	wl = wlif->wl;
	BCM_REFERENCE(wl);
#ifdef WL_CFG80211
	if (ifidx == 0)
		wl_netdev_set_free_netdev(new_dev);
	else
		wl_netdev_set_destructor(new_dev, wl_netdev_free);
#else
	wl_netdev_set_free_netdev(new_dev);
#endif /* WL_CFG80211 */

#if defined(USE_CFG80211)
	/* XXX CSP626409: If NL80211_FLAG_NEED_RTNL is enabled, nl80211 layer takes rtnl_lock()
	 * first before wl_add_if() is called. When register_netdev() calls rtnl_lock() it blocks
	 * there.  Calling unlock() before call register_netdev() can reduce race condition,
	 * but this only WAR since the tiny race condition may happen during unclock() and lock().
	 */
	WL_TRACE(("%s: Start register_netdev() %s\n", __FUNCTION__, wlif->name));
	if (!rtnl_is_needed) {
		ASSERT_RTNL();
		if (register_netdevice(new_dev)) {
			WL_ERROR(("wl%d: wl_add_if: register_netdevice() failed for \"%s\"\n",
					wl->pub->unit, new_dev->name));
			goto done;
		}
	}
	else
#endif /* #if defined(USE_CFG80211) && defined(WLC_HIGH) */
	{
		if (register_netdev(new_dev)) {
			WL_ERROR(("wl%d: wl_add_if: register_netdev() failed for \"%s\"\n",
					wl->pub->unit, new_dev->name));
			goto done;
		}
	}
	dev_registered = TRUE;

#if defined(BCM_WFD)
	if (wl_wfd_registerdevice(wl->wfd_idx, new_dev) != 0) {
		WL_ERROR(("wl_wfd_registerdevice failed [%s]\n", new_dev->name));
		goto done;
	}
#endif // endif

#if defined(BCM_GMAC3)
	/* wlif->dev is now in wl%d.%d <unit.subunit> format */
	wlif->fwdh = fwder_bind(wl->fwdh, WL_FWDER_UNIT(wl), wlif->subunit,
			wlif->dev, TRUE);
	fwder_register(wlif->fwdh, wlif->dev);
#endif /* BCM_GMAC3 */

#ifdef HNDCTF
	if ((ctf_dev_register(wl->cih, new_dev, FALSE) != BCME_OK) ||
		(ctf_enable(wl->cih, new_dev, TRUE, NULL) != BCME_OK)) {
			ctf_dev_unregister(wl->cih, new_dev);
			WL_ERROR(("wl%d: ctf_dev_register() failed\n", wl->pub->unit));
			goto done;
		}
#endif /* HNDCTF */

done:
	wlif->dev_registered = dev_registered;

	return 0;
} /* wl_register_interface */

/** Fetch net_device given a interface index (subunit) */
struct net_device *
wl_idx2net(wl_info_t *wl, uint subunit)
{
	wl_if_t *if_list;

	ASSERT(wl != NULL);
	if_list = wl->if_list;

	while (if_list) {
		if (if_list->subunit == subunit) {
			return if_list->dev;
		}
		if_list = if_list->next;
	}

	return NULL;
} /*  wl_idx2net() */

#if defined(WL_CFG80211)

#if defined(MEDIA_CFG)
static int wl_preinit_ioctls(struct net_device *ndev)
{
	int ret = 0;
	char eventmask[WL_EVENTING_MASK_LEN];
	char iovbuf[WL_EVENTING_MASK_LEN + 12]; /*  Room for "event_msgs" + '\0' + bitvec  */

	/* Read event_msgs mask */
	bcm_mkiovar("event_msgs", NULL, 0, iovbuf, sizeof(iovbuf));
	ret = wldev_ioctl_get(ndev, WLC_GET_VAR, iovbuf, sizeof(iovbuf));

	if (unlikely(ret)) {
		WL_ERROR(("Get event_msgs error (%d)\n", ret));
		goto done;
	}

	memcpy(eventmask, iovbuf, WL_EVENTING_MASK_LEN);

	/* Setup event_msgs */
	setbit(eventmask, WLC_E_SET_SSID);
	setbit(eventmask, WLC_E_AUTH);
#ifdef WL_SAE
	setbit(eventmask, WLC_E_START_AUTH);
#endif /* WL_SAE */
	setbit(eventmask, WLC_E_ASSOC);
	setbit(eventmask, WLC_E_REASSOC);
	setbit(eventmask, WLC_E_REASSOC_IND);
	setbit(eventmask, WLC_E_DEAUTH);
	setbit(eventmask, WLC_E_DEAUTH_IND);
	setbit(eventmask, WLC_E_DISASSOC_IND);
	setbit(eventmask, WLC_E_DISASSOC);
	setbit(eventmask, WLC_E_JOIN);
	setbit(eventmask, WLC_E_ASSOC_IND);
	setbit(eventmask, WLC_E_LINK);
	setbit(eventmask, WLC_E_SCAN_COMPLETE);
	setbit(eventmask, WLC_E_ESCAN_RESULT);
	/* enable dongle roaming event */
#ifdef WL_CFG80211
	setbit(eventmask, WLC_E_ROAM);
	setbit(eventmask, WLC_E_BSSID);
#endif // endif

#if defined(WLP2P) && (defined(WL_ENABLE_P2P_IF) || defined(WL_CFG80211_P2P_DEV_IF))
	setbit(eventmask, WLC_E_ACTION_FRAME_RX);
	setbit(eventmask, WLC_E_ACTION_FRAME_COMPLETE);
	setbit(eventmask, WLC_E_ACTION_FRAME_OFF_CHAN_COMPLETE);
	setbit(eventmask, WLC_E_P2P_DISC_LISTEN_COMPLETE);
#endif  /* defined(WLP2P) && (defined(WL_ENABLE_P2P_IF) || defined(WL_CFG80211_P2P_DEV_IF)) */

	setbit(eventmask, WLC_E_PWR_SAVE_SYNC);
	/* Write updated Event mask */
	bcm_mkiovar("event_msgs", eventmask, WL_EVENTING_MASK_LEN, iovbuf, sizeof(iovbuf));

	ret = wldev_ioctl_set(ndev, WLC_SET_VAR, iovbuf, sizeof(iovbuf));
	if (unlikely(ret)) {
		WL_ERROR(("Set event_msgs error (%d)\n", ret));
		goto done;
	}

done:
	return ret;
} /* wl_preinit_ioctls */
#endif /* MEDIA_CFG */

/* Get the interface Role from the bsscfg */
static int8_t wl_get_bsscfg_role(wlc_info_t *wlc, wlc_bsscfg_t *cfg,
		uint8_t iftype, int8_t *if_role)
{
	int8_t role = -1;

	if (iftype == WLC_IFTYPE_BSS) {
		if (BSSCFG_AP(cfg)) {

			role =
#ifdef WLP2P
				BSS_P2P_ENAB(wlc, cfg) ? WLC_E_IF_ROLE_P2P_GO:
#endif // endif
				WLC_E_IF_ROLE_AP;
		} else {
			role =
#ifdef WLP2P
				BSS_P2P_ENAB(wlc, cfg) ? WLC_E_IF_ROLE_P2P_CLIENT:
#endif // endif
				BSSCFG_IBSS(cfg) ? WLC_E_IF_ROLE_IBSS:
				WLC_E_IF_ROLE_STA;
		}
	}
	if (!if_role || (role == -1))
		return BCME_ERROR;

	*if_role = role;

	return BCME_OK;
}

static s32 wl_cfg80211_enabled()
{
	char * var;
	int cfg_hapd_enable = 0;

	if ((var = getvar(NULL, "hapd_enable")) == NULL) {
		return BCME_OK;
	}
	cfg_hapd_enable = bcm_strtoul(var, NULL, 0);

	if (!cfg_hapd_enable)
		return BCME_UNSUPPORTED;
	return BCME_OK;
}
int wl_cfg80211_trace_set(bool enable)
{
	u32 dbg_level = enable ? (WL_DBG_ERR | WL_DBG_INFO | WL_DBG_DBG) : WL_DBG_ERR;

	wl_cfg80211_enable_trace(TRUE, dbg_level);
	return BCME_OK;
}

#endif /* WL_CFG80211 */

/* All asssigned free index are mapping to a bitmap
 * wl->ifidx_bitmap integer value.Getting new ifidx
 * is based on the index of clr bit in the variable.
 * While remove an interface clear the curresponding
 * bits from the bitmap wl->ifidx_bitmap
 */
static int wl_get_free_ifidx(wl_info_t * wl, int iftype, int index)
{
	uint8 ifidx = 0;
	struct wl_if *wlif;

	ASSERT(wl != NULL);
	/* To get the free index for each interface, checking each bits of the ifidx_bitmap. */
	if (wl->ifidx_bitmap >= (uint32) WL_MAX_IFS_BMP) {
		WL_ERROR(("%s: Max interface count Exceeded\n", __FUNCTION__));
		return BCME_RANGE;
	}
	if (iftype == WL_IFTYPE_MON) {
		ifidx = index;
		goto done;
	} else if (iftype == WL_IFTYPE_WDS) {
		while (isset((void *)&wl->ifidx_bitmap, ifidx))
			ifidx++;
	} else if ((index < WL_MAX_IFACE) &&
		!isset((void *)&wl->ifidx_bitmap, index)) {
			ifidx = index;
	} else {
		WL_ERROR(("%s: ifindex get failed\n", __FUNCTION__));
		ASSERT(0);
		return BCME_ERROR;
	}

	/* Update ifidx_bitmap */
	setbit((void *)&wl->ifidx_bitmap, ifidx);

	for (wlif = wl->if_list; wlif != (wl_if_t *)NULL; wlif = wlif->next) {
		if (wlif->subunit == ifidx) {
			WL_ERROR(("%s: Already ifindex(%d) is exists\n", __FUNCTION__, ifidx));
			ASSERT(0);
			return BCME_BADARG;
		}
	}
done:
	return (ifidx);
}

static int wl_free_ifidx(wl_info_t *wl, int ifidx)
{
	ASSERT(ifidx < WL_MAX_IFACE || wl != NULL);

	/* Update ifidx_bitmap */
	clrbit((void *)&wl->ifidx_bitmap, ifidx);
	return BCME_OK;
}

#if defined(WDS)
/* get the wds index from the wlif */
int wlc_get_wlif_wdsindex(struct wl_if *wlif)
{
	return wlif->wds_index;
}
#endif // endif

#if defined(__arm__)
int fake_main(void);
/*
 * Function fake_main will never be called by the Linux loader. It is supplied to the linker,
 * so the linker is able to create a call tree and thus determine which functions are used.
 */
int fake_main(void)
{
	wl_module_init();
	wl_module_exit();

	return 0;
}
#endif /* __arm__ */
