/*
 * Broadcom Dongle Host Driver (DHD), Linux-specific network interface
 * Basically selected code segments from usb-cdc.c and usb-rndis.c
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
 * $Id: dhd_linux.c 808017 2022-02-08 04:16:00Z $
 */

#include <typedefs.h>
#include <linuxver.h>
#include <osl.h>

#ifdef WL_MONITOR
#include <hndd11.h>
#endif /* WL_MONITOR */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#include <linux/rtnetlink.h>
#include <linux/etherdevice.h>
#include <linux/random.h>
#include <linux/spinlock.h>
#include <linux/ethtool.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <linux/ip.h>
#include <linux/reboot.h>
#include <linux/notifier.h>
#include <net/addrconf.h>
#include <net/sch_generic.h>
#ifdef ENABLE_ADAPTIVE_SCHED
#include <linux/cpufreq.h>
#endif /* ENABLE_ADAPTIVE_SCHED */

#include <asm/uaccess.h>
#include <asm/unaligned.h>

#include <epivers.h>
#include <bcmutils.h>
#include <siutils.h>
#include <hndsoc.h>
#include <bcmendian.h>
#include <bcmdevs.h>

#include <dngl_stats.h>
#include <dhd_linux_wq.h>
#include <dhd.h>
#include <dhd_linux.h>

#ifdef BCM_WFD
#include <dhd_wfd.h>
#endif
#if defined(BCM_PKTFWD)
#include <dhd_pktfwd.h>
#endif /* BCM_PKTFWD */
#if defined(BCM_AWL)
#include <dhd_awl.h>
#endif /* BCM_AWL */
#include <ethernet.h>
#include <bcmevent.h>
#include <vlan.h>
#include <802.3.h>

#ifdef DHD_WET
#include <dhd_wet.h>
#endif /* DHD_WET */
#include <dhd_flowring.h>
#include <dhd_bus.h>
#include <dhd_proto.h>
#include <dhd_dbg.h>
#ifdef CONFIG_HAS_WAKELOCK
#include <linux/wakelock.h>
#endif
#ifdef WL_CFG80211
#include <wl_cfg80211.h>
#endif
#include <wl_core.h>
#ifdef PNO_SUPPORT
#include <dhd_pno.h>
#endif
#ifdef WLBTAMP
#include <802.11_bta.h>
#include <bt_amp_hci.h>
#include <dhd_bta.h>
#endif

#ifdef CONFIG_COMPAT
#include <linux/compat.h>
#endif

#ifdef DHD_WMF
#include <dhd_wmf_linux.h>
#endif /* DHD_WMF */

#ifdef DHD_L2_FILTER
#include <bcmicmp.h>
#include <bcm_l2_filter.h>
#include <dhd_l2_filter.h>
#endif /* DHD_L2_FILTER */

#ifdef DHD_PSTA
#include <dhd_psta.h>
#endif /* DHD_PSTA */

#ifdef DHD_DPSTA
#include <dpsta.h>
#endif /* DHD_DPSTA */

#ifdef AMPDU_VO_ENABLE
#include <802.1d.h>
#endif /* AMPDU_VO_ENABLE */

#ifdef DHDTCPACK_SUPPRESS
#include <dhd_ip.h>
#endif /* DHDTCPACK_SUPPRESS */

#ifdef BCM_NBUFF
#include <dhd_nbuff.h>
#endif
#ifdef BCM_BLOG
#include <dhd_blog.h>
#endif

#if defined(BCM_CPE_PKTC)
#include <wl_pktc.h>
#endif /* BCM_CPE_PKTC */

#if (defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE))
#include <bcm_spdsvc.h>
#endif /* CONFIG_BCM_SPDSVC || CONFIG_BCM_SPDSVC_MODULE */

#if defined(BCM_DHD_RUNNER)
#include <dhd_runner.h>
#endif /* BCM_DHD_RUNNER */

#ifdef DHD_IFE
#include <dhd_ife.h>
#endif /* DHD_IFE */

#ifdef DHD_DEBUG_PAGEALLOC
typedef void (*page_corrupt_cb_t)(void *handle, void *addr_corrupt, size_t len);
void dhd_page_corrupt_cb(void *handle, void *addr_corrupt, size_t len);
extern void register_page_corrupt_cb(page_corrupt_cb_t cb, void* handle);
#endif /* DHD_DEBUG_PAGEALLOC */
/* debug/trace */
uint wl_msg_level = WL_ERROR_VAL;
#if defined(DHD_TCP_WINSIZE_ADJUST)
#include <linux/tcp.h>
#include <net/tcp.h>
#endif /* DHD_TCP_WINSIZE_ADJUST */

#if defined(DHD_LB)
/* Dynamic CPU selection for load balancing */
#include <linux/cpu.h>
#include <linux/cpumask.h>
#include <linux/notifier.h>
#include <linux/workqueue.h>
#include <asm/atomic.h>

#if !defined(DHD_LB_PRIMARY_CPUS)
#define DHD_LB_PRIMARY_CPUS     0x0 /* Big CPU coreids mask */
#endif

#if !defined(DHD_LB_SECONDARY_CPUS)
#define DHD_LB_SECONDARY_CPUS   0xFE /* Little CPU coreids mask */
#endif

#define HIST_BIN_SIZE	8

#if defined(DHD_LB_RXP)
static void dhd_rx_napi_dispatcher_fn(struct work_struct * work);
#endif /* DHD_LB_RXP */

#endif /* DHD_LB */

#ifdef WLMEDIA_HTSF
#include <linux/time.h>
#include <htsf.h>

#define HTSF_MINLEN 200    /* min. packet length to timestamp */
#define HTSF_BUS_DELAY 150 /* assume a fix propagation in us  */
#define TSMAX  1000        /* max no. of timing record kept   */
#define NUMBIN 34

static uint32 tsidx = 0;
static uint32 htsf_seqnum = 0;
uint32 tsfsync;
struct timeval tsync;
static uint32 tsport = 5010;

typedef struct histo_ {
	uint32 bin[NUMBIN];
} histo_t;

#if !ISPOWEROF2(DHD_SDALIGN)
#error DHD_SDALIGN is not a power of 2!
#endif

static histo_t vi_d1, vi_d2, vi_d3, vi_d4;
#endif /* WLMEDIA_HTSF */

#ifdef WL_MONITOR
#include <bcmmsgbuf.h>
#include <bcmwifi_monitor.h>
#endif /* WL_MONITOR */

#if defined(DHD_TCP_WINSIZE_ADJUST)
#define MIN_TCP_WIN_SIZE 18000
#define WIN_SIZE_SCALE_FACTOR 2
#define MAX_TARGET_PORTS 5

static uint target_ports[MAX_TARGET_PORTS] = {20, 0, 0, 0, 0};
static uint dhd_use_tcp_window_size_adjust = FALSE;
static void dhd_adjust_tcp_winsize(int op_mode, struct sk_buff *skb);
#endif /* DHD_TCP_WINSIZE_ADJUST */

#if defined(BLOCK_IPV6_PACKET) && defined(CUSTOMER_HW4)
#define HEX_PREF_STR	"0x"
#define UNI_FILTER_STR	"010000000000"
#define ZERO_ADDR_STR	"000000000000"
#define ETHER_TYPE_STR	"0000"
#define IPV6_FILTER_STR	"20"
#define ZERO_TYPE_STR	"00"
#endif /* BLOCK_IPV6_PACKET && CUSTOMER_HW4 */

#if defined(OEM_ANDROID) && defined(SOFTAP)
extern bool ap_cfg_running;
extern bool ap_fw_loaded;
#endif

#ifdef CUSTOMER_HW4
#ifdef FIX_CPU_MIN_CLOCK
#include <linux/pm_qos.h>
#endif /* FIX_CPU_MIN_CLOCK */
#endif /* CUSTOMER_HW4 */

#ifdef ENABLE_ADAPTIVE_SCHED
#define DEFAULT_CPUFREQ_THRESH		1000000	/* threshold frequency : 1000000 = 1GHz */
#ifndef CUSTOM_CPUFREQ_THRESH
#define CUSTOM_CPUFREQ_THRESH	DEFAULT_CPUFREQ_THRESH
#endif /* CUSTOM_CPUFREQ_THRESH */
#endif /* ENABLE_ADAPTIVE_SCHED */

/* enable HOSTIP cache update from the host side when an eth0:N is up */
#define AOE_IP_ALIAS_SUPPORT 1

#ifdef BCM_FD_AGGR
#include <bcm_rpc.h>
#include <bcm_rpc_tp.h>
#endif
#ifdef PROP_TXSTATUS
#include <wlfc_proto.h>
#include <dhd_wlfc.h>
#endif

#if defined(OEM_ANDROID)
#include <wl_android.h>
#endif

#if defined(DHD_DPSTA) && defined(DHD_PSTA)
void dhd_dpsta_psta_register(dhd_pub_t *dhd);
void dhd_dpsta_dwds_register(dhd_pub_t *dhd);
#endif /* DHD_DPSTA && DHD_PSTA */

#include <dhd_macdbg.h>

/*
 * No need to update this for new dumps. The pre-processor expansion
 * will produce code as defined by MACDBGDUMP_ENUMDEF for each dump
 * as listed in MACDBGDUMP_ENUMDEF_LIST.
 *
 * MACDBGDUMP_ENUMDEF is defined below to produce an enumeration
 * value for each dump list in MACDBGDUMP_ENUMDEF_LIST.
 */
#define MACDBGDUMP_ENUMDEF(_id, _str)	MACDBG_DUMP_ ## _id,

enum macdbgdumptype {
	MACDBGDUMP_ENUMDEF_LIST

	MACDBG_DUMP_MAX
};

#undef MACDBGDUMP_ENUMDEF

/*
 * No need to update this for new dumps. The pre-processor expansion
 * will produce code as defined by MACDBGDUMP_ENUMDEF for each dump
 * as listed in MACDBGDUMP_ENUMDEF_LIST.
 *
 * MACDBGDUMP_ENUMDEF is defined below to produce an initializer
 * for each dump entry.
 */
#define MACDBGDUMP_ENUMDEF(_id, _str)	[MACDBG_DUMP_ ## _id] = #_str,

static const char *macdbg_dump_names[] = {
	MACDBGDUMP_ENUMDEF_LIST
};

#undef MACDBGDUMP_ENUMDEF

#ifdef DHD_LBR_AGGR_BCM_ROUTER
#include <dhd_aggr.h>
#endif

#ifdef DHD_DPSTA
#define IFIDX_PSTA_STAINTF (0)
#define IFIDX_PSTA_APINTF (1)
#define IFIDX_PSTA_FIRST_PROXYINTF (IFIDX_PSTA_APINTF + 1)
#endif /* DHD_DPSTA */

#define DUMP_FILENAME_SZ	(48)
#ifdef CUSTOMER_HW4
#define SOCRAM_DUMP_FILE_NAME  "/data/log/mem_dump"
#define SOCRAM_DUMP_FILE_FLAGS (O_CREAT | O_WRONLY)
#elif defined(OEM_ANDROID)
#define SOCRAM_DUMP_FILE_NAME "/installmedia/mem_dump"
/* Extra flags O_DIRECT and O_SYNC are required for Brix Android, as we are
 * calling BUG_ON immediately after collecting the socram dump.
 * So the file write operation should directly write the contents into the
 * file instead of caching it. O_TRUNC flag ensures that file will be re-written
 * instead of appending.
 */
#define SOCRAM_DUMP_FILE_FLAGS (O_CREAT | O_WRONLY | O_DIRECT | O_SYNC | O_TRUNC)
#else
#define SOCRAM_DUMP_FILE_NAME "/tmp/mem_dump"
#define SOCRAM_DUMP_FILE_FLAGS (O_CREAT | O_WRONLY)
#endif /* CUSTOMER_HW4 */

#define MEMBLOCK (64 * 1024) /**< Block size used for uploading of dongle socram */

#if defined(BCM_ROUTER_DHD)
/*
 * Queue budget: Minimum number of packets that a queue must be allowed to hold
 * to prevent starvation.
 */
#define DHD_QUEUE_BUDGET_DEFAULT    (256)
int dhd_queue_budget = DHD_QUEUE_BUDGET_DEFAULT;

module_param(dhd_queue_budget, int, 0);

/*
 * Per station pkt threshold: Sum total of all packets in the backup queues of
 * flowrings belonging to the station, not including packets already admitted
 * to flowrings.
 */
#define DHD_STA_THRESHOLD_DEFAULT   (2048)
int dhd_sta_threshold = DHD_STA_THRESHOLD_DEFAULT;
module_param(dhd_sta_threshold, int, 0);

/*
 * Per interface pkt threshold: Sum total of all packets in the backup queues of
 * flowrings belonging to the interface, not including packets already admitted
 * to flowrings.
 */
#define DHD_IF_THRESHOLD_DEFAULT   (2048 * 32)
int dhd_if_threshold = DHD_IF_THRESHOLD_DEFAULT;
module_param(dhd_if_threshold, int, 0);
#endif /* BCM_ROUTER_DHD */

const uint8 wme_fifo2ac[] = { 0, 1, 2, 3, 1, 1 };
const uint8 prio2fifo[8] = { 1, 0, 0, 1, 2, 2, 3, 3 };
#define WME_PRIO2AC(prio)  wme_fifo2ac[prio2fifo[(prio)]]

#ifdef ARP_OFFLOAD_SUPPORT
void aoe_update_host_ipv4_table(dhd_pub_t *dhd_pub, u32 ipa, bool add, int idx);
static int dhd_inetaddr_notifier_call(struct notifier_block *this,
	unsigned long event, void *ptr);
static struct notifier_block dhd_inetaddr_notifier = {
	.notifier_call = dhd_inetaddr_notifier_call
};
/* to make sure we won't register the same notifier twice, otherwise a loop is likely to be
 * created in kernel notifier link list (with 'next' pointing to itself)
 */
static bool dhd_inetaddr_notifier_registered = FALSE;
#endif /* ARP_OFFLOAD_SUPPORT */

#if defined(OEM_ANDROID) && defined(CONFIG_IPV6)
static int dhd_inet6addr_notifier_call(struct notifier_block *this,
	unsigned long event, void *ptr);
static struct notifier_block dhd_inet6addr_notifier = {
	.notifier_call = dhd_inet6addr_notifier_call
};
/* to make sure we won't register the same notifier twice, otherwise a loop is likely to be
 * created in kernel notifier link list (with 'next' pointing to itself)
 */
static bool dhd_inet6addr_notifier_registered = FALSE;
#endif /* OEM_ANDROID && CONFIG_IPV6 */

#if defined(STB) && !defined(STBAP)
static int dhd_set_wowl_trigger(dhd_pub_t *dhdp, struct net_device *ndev);
static void dhd_clear_wowl(dhd_pub_t *dhdp);
#endif /* STB && SBTAP */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)) && defined(CONFIG_PM_SLEEP)
#include <linux/suspend.h>
volatile bool dhd_mmc_suspend = FALSE;
DECLARE_WAIT_QUEUE_HEAD(dhd_dpc_wait);
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)) && defined(CONFIG_PM_SLEEP) */

#if defined(OOB_INTR_ONLY) || defined(BCMSPI_ANDROID)
extern void dhd_enable_oob_intr(struct dhd_bus *bus, bool enable);
#endif /* defined(OOB_INTR_ONLY) || defined(BCMSPI_ANDROID) */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)) && (defined(OEM_ANDROID))
static void dhd_hang_process(void *dhd_info, void *event_data, enum dhd_wq_event event);
#endif /* #if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)) && (defined(OEM_ANDROID)) */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
MODULE_LICENSE("GPL and additional rights");
#endif /* LinuxVer */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 32)) && 0
static struct device_type wlan_type = {
	.name   = "wlan",
};
#endif

#ifdef BCMDBUS
#include <dbus.h>
atomic_t	in_disconnect;
extern int dhd_bus_init(dhd_pub_t *dhdp, bool enforce_mutex);
extern void dhd_bus_stop(struct dhd_bus *bus, bool enforce_mutex);
extern void dhd_bus_unregister(void);
#else
#include <dhd_bus.h>
#endif /* BCMDBUS */

#include <msf.h> // support one file containing dongle fw image, event logging strings, etc.

#ifdef BCM_FD_AGGR
#define DBUS_RX_BUFFER_SIZE_DHD(net)	(BCM_RPC_TP_DNGL_AGG_MAX_BYTE)
#else
#ifndef PROP_TXSTATUS
#define DBUS_RX_BUFFER_SIZE_DHD(net)	(net->mtu + net->hard_header_len + dhd->pub.hdrlen)
#else
#define DBUS_RX_BUFFER_SIZE_DHD(net)	(net->mtu + net->hard_header_len + dhd->pub.hdrlen + 128)
#endif
#endif /* BCM_FD_AGGR */

#ifdef PROP_TXSTATUS
extern bool dhd_wlfc_skip_fc(void * dhdp, uint8 idx);
extern void dhd_wlfc_plat_init(void *dhd);
extern void dhd_wlfc_plat_deinit(void *dhd);
#endif /* PROP_TXSTATUS */
#if defined(CUSTOMER_HW4) && defined(USE_DYNAMIC_F2_BLKSIZE)
extern uint sd_f2_blocksize;
extern int dhdsdio_func_blocksize(dhd_pub_t *dhd, int function_num, int block_size);
#endif /* CUSTOMER_HW4 && USE_DYNAMIC_F2_BLKSIZE */

#if LINUX_VERSION_CODE == KERNEL_VERSION(2, 6, 15)
const char *
print_tainted()
{
	return "";
}
#endif	/* LINUX_VERSION_CODE == KERNEL_VERSION(2, 6, 15) */

/* Linux wireless extension support */
#if defined(WL_WIRELESS_EXT)
#include <wl_iw.h>
extern wl_iw_extra_params_t  g_wl_iw_params;
#endif /* defined(WL_WIRELESS_EXT) */

#if defined(CUSTOMER_HW4) && defined(CONFIG_PARTIALSUSPEND_SLP)
/* XXX SLP use defferent earlysuspend header file and some functions
 * But most of meaning is same as Android
 */
#include <linux/partialsuspend_slp.h>
#define CONFIG_HAS_EARLYSUSPEND
#define DHD_USE_EARLYSUSPEND
#define register_early_suspend		register_pre_suspend
#define unregister_early_suspend	unregister_pre_suspend
#define early_suspend				pre_suspend
#define EARLY_SUSPEND_LEVEL_BLANK_SCREEN		50
#else
#if defined(CONFIG_HAS_EARLYSUSPEND) && defined(DHD_USE_EARLYSUSPEND)
#include <linux/earlysuspend.h>
#endif /* defined(CONFIG_HAS_EARLYSUSPEND) && defined(DHD_USE_EARLYSUSPEND) */
#endif /* CUSTOMER_HW4 && CONFIG_PARTIALSUSPEND_SLP */

extern int dhd_get_suspend_bcn_li_dtim(dhd_pub_t *dhd);

#ifdef PKT_FILTER_SUPPORT
extern void dhd_pktfilter_offload_set(dhd_pub_t * dhd, char *arg);
extern void dhd_pktfilter_offload_enable(dhd_pub_t * dhd, char *arg, int enable, int master_mode);
extern void dhd_pktfilter_offload_delete(dhd_pub_t *dhd, int id);
#endif

#ifdef CUSTOMER_HW4
#ifdef READ_MACADDR
extern int dhd_read_macaddr(struct dhd_info *dhd, struct ether_addr *mac);
#endif
#ifdef WRITE_MACADDR
extern int dhd_write_macaddr(struct ether_addr *mac);
#endif
#ifdef USE_CID_CHECK
extern int dhd_check_module_cid(dhd_pub_t *dhd);
#endif
#ifdef GET_MAC_FROM_OTP
extern int dhd_check_module_mac(dhd_pub_t *dhd, struct ether_addr *mac);
#endif
#ifdef MIMO_ANT_SETTING
extern int dhd_sel_ant_from_file(dhd_pub_t *dhd);
#endif
#ifdef WRITE_WLANINFO
extern uint32 sec_save_wlinfo(char *firm_ver, char *dhd_ver, char *nvram_p);
#endif
#ifdef DHD_OF_SUPPORT
extern void interrupt_set_cpucore(int set);
#endif

#else

#ifdef READ_MACADDR
extern int dhd_read_macaddr(struct dhd_info *dhd);
#else
static inline int dhd_read_macaddr(struct dhd_info *dhd) { return 0; }
#endif
#ifdef WRITE_MACADDR
extern int dhd_write_macaddr(struct ether_addr *mac);
#else
static inline int dhd_write_macaddr(struct ether_addr *mac) { return 0; }
#endif
#endif /* CUSTOMER_HW4 */

#if defined(CUSTOMER_HW4) && defined(ARGOS_CPU_SCHEDULER)
extern int argos_task_affinity_setup_label(struct task_struct *p, const char *label,
	struct cpumask * affinity_cpu_mask, struct cpumask * default_cpu_mask);
extern struct cpumask hmp_slow_cpu_mask;
extern struct cpumask hmp_fast_cpu_mask;
extern void set_cpucore_for_interrupt(cpumask_var_t default_cpu_mask,
	cpumask_var_t affinity_cpu_mask);
#endif /* CUSTOMER_HW4 && ARGOS_CPU_SCHEDULER */

#if defined(BCA_SROMMAP)
extern void reinit_loaded_srommap(void);
#endif

/* XXX WAR: Sometimes Exynos 5433 can't schedule Rx traffic after push packet to stack queue
*  if activated rps_cpus kernel feature. To avoid this problem, Need to disable rps_cpus
*  if current tput is low than the TPUT_THRESHOLD
*/
#if defined(ARGOS_CPU_SCHEDULER) && defined(ARGOS_RPS_CPU_CTL)
int argos_register_notifier_init(struct net_device *net);
int argos_register_notifier_deinit(void);

extern int sec_argos_register_notifier(struct notifier_block *n, char *label);
extern int sec_argos_unregister_notifier(struct notifier_block *n, char *label);

static int argos_status_notifier_wifi_cb(struct notifier_block *notifier,
	unsigned long speed, void *v);

static struct notifier_block argos_wifi = {
	.notifier_call = argos_status_notifier_wifi_cb,
};

typedef struct {
	struct net_device *wlan_primary_netdev;
	int argos_rps_cpus_enabled;
} argos_rps_ctrl;

argos_rps_ctrl argos_rps_ctrl_data;
#define RPS_TPUT_THRESHOLD		300
#define DELAY_TO_CLEAR_RPS_CPUS 300
#endif /* ARGOS_RPS_CPU_CTL && ARGOS_CPU_SCHEDULER */

#if defined(SOFTAP_TPUT_ENHANCE)
extern void dhd_bus_setidletime(dhd_pub_t *dhdp, int idle_time);
extern void dhd_bus_getidletime(dhd_pub_t *dhdp, int* idle_time);
#endif /* SOFTAP_TPUT_ENHANCE */

#if defined(BCM_ROUTER_DHD) || defined(TRAFFIC_MGMT_DWM)
void traffic_mgmt_pkt_set_prio(dhd_pub_t *dhdp, void * pktbuf);
#endif /* BCM_ROUTER_DHD || TRAFFIC_MGMT_DWM */

#ifdef DHD_FW_COREDUMP
static void dhd_mem_dump(void *dhd_info, void *event_info, enum dhd_wq_event event);
#endif /* DHD_FW_COREDUMP */

static int dhd_reboot_callback(struct notifier_block *this, unsigned long code, void *unused);
static struct notifier_block dhd_reboot_notifier = {
	.notifier_call = dhd_reboot_callback,
	.priority = 1,
};

#if defined(OEM_ANDROID) || defined(BCA_HNDROUTER)
#ifdef BCMPCIE
#ifdef BCA_HNDROUTER
int is_reboot = 0;
#else
static int is_reboot = 0;
#endif
#endif /* BCMPCIE */
#endif /* OEM_ANDROID */

typedef struct dhd_if_event {
	struct list_head	list;
	wl_event_data_if_t	event;
	char			name[IFNAMSIZ+1];
	uint8			mac[ETHER_ADDR_LEN];
} dhd_if_event_t;

/* Interface control information */
typedef struct dhd_if {
	struct dhd_info *info;			/* back pointer to dhd_info */
	/* OS/stack specifics */
	struct net_device *net;
	int				idx;			/* iface idx in dongle */
	uint			subunit;		/* subunit */
	uint8			mac_addr[ETHER_ADDR_LEN];	/* assigned MAC address */
	bool			set_macaddress;
	uint8			bssidx;			/* bsscfg index for the interface */
	bool			attached;		/* Delayed attachment when unset */
	bool			txflowcontrol;	/* Per interface flow control indicator */
	char			name[IFNAMSIZ+1]; /* linux interface name */
	char			dngl_name[IFNAMSIZ+1]; /* corresponding dongle interface name */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36))
	struct rtnl_link_stats64 stats;
#else
	struct net_device_stats stats;
#endif /* KERNEL_VERSION >= 2.6.36 */
#if defined(BCM_DHD_RUNNER)
	/* m_stats is used to maintain current ssid stats for mutlicast in runner */
	struct net_device_stats m_stats;
	struct net_device_stats c_stats;
	BlogStats_t	b_stats; /* blog stats */
#endif /* BCM_DHD_RUNNER */

#ifdef DHD_WMF
	dhd_wmf_t		wmf;		/* per bsscfg wmf setting */
	bool	wmf_psta_disable;		/* enable/disable MC pkt to each mac
						 * of MC group behind PSTA
						 */
#endif /* DHD_WMF */
#ifdef PCIE_FULL_DONGLE
	struct list_head sta_list;		/* sll of associated stations */
#if !defined(BCM_GMAC3)
	spinlock_t	sta_list_lock;		/* lock for manipulating sll */
#endif /* ! BCM_GMAC3 */
#endif /* PCIE_FULL_DONGLE */
	uint32  ap_isolate;			/* ap-isolation settings */
#ifdef DHD_L2_FILTER
	bool parp_enable;
	bool parp_discard;
	bool parp_allnode;
	arp_table_t *phnd_arp_table;
/* for Per BSS modification */
	bool dhcp_unicast;
	bool block_ping;
	bool grat_arp;
	bool block_tdls;
#endif /* DHD_L2_FILTER */
#if (defined(BCM_ROUTER_DHD) && defined(QOS_MAP_SET))
	uint8	 *qosmap_up_table;		/* user priority table, size is UP_TABLE_MAX */
	bool qosmap_up_table_enable;	/* flag set only when app want to set additional UP */
#endif /* BCM_ROUTER_DHD && QOS_MAP_SET */
#ifdef DHD_MCAST_REGEN
	bool mcast_regen_bss_enable;
#endif
	bool rx_pkt_chainable;		/* set all rx packet to chainable config by default */
	cumm_ctr_t cumm_ctr;		/* cummulative queue length of child flowrings */
#ifdef BCM_ROUTER_DHD
	bool	primsta_dwds;		/* DWDS status of primary sta interface */
	bool	bss_up;			/* bsscfg up in dongle */
#endif /* BCM_ROUTER_DHD */
#ifdef BCM_GMAC3
	uint8   _1905_al_ucast[ETHER_ADDR_LEN];
	uint8   _1905_al_mcast[ETHER_ADDR_LEN];
#endif /* BCM_GMAC3 */
#if defined(BCM_PKTFWD)
	struct d3fwd_wlif   *d3fwd_wlif;
#endif /* BCM_PKTFWD */
	uint8	peer_wds_mac_addr[ETHER_ADDR_LEN];	/* peer WDS MAC address */
	uint8	wds_role;		/* WDS interface role; Authenticator(AP)/Supplicant(STA) */
	uint8	wdsidx;			/* WDS index for the interface */
} dhd_if_t;

#ifdef WLMEDIA_HTSF
typedef struct {
	uint32 low;
	uint32 high;
} tsf_t;

typedef struct {
	uint32 last_cycle;
	uint32 last_sec;
	uint32 last_tsf;
	uint32 coef;     /* scaling factor */
	uint32 coefdec1; /* first decimal  */
	uint32 coefdec2; /* second decimal */
} htsf_t;

typedef struct {
	uint32 t1;
	uint32 t2;
	uint32 t3;
	uint32 t4;
} tstamp_t;

static tstamp_t ts[TSMAX];
static tstamp_t maxdelayts;
static uint32 maxdelay = 0, tspktcnt = 0, maxdelaypktno = 0;

#endif  /* WLMEDIA_HTSF */

struct ipv6_work_info_t {
	uint8			if_idx;
	char			ipv6_addr[16];
	unsigned long		event;
};

#ifdef DHD_DEBUG
typedef struct dhd_dump {
	uint8 *buf;
	int bufsize;
} dhd_dump_t;
#endif /* DHD_DEBUG */

/* When Perimeter locks are deployed, any blocking calls must be preceeded
 * with a PERIM UNLOCK and followed by a PERIM LOCK.
 * Examples of blocking calls are: schedule_timeout(), down_interruptible(),
 * wait_event_timeout().
 */

/* Local private structure (extension of pub) */
typedef struct dhd_info {
#if defined(WL_WIRELESS_EXT)
	wl_iw_t		iw;		/* wireless extensions state (must be first) */
#else
#ifdef BCM_NBUFF_WLMCAST_IPV6
	void *(*nic_hook_fn)(int cmd, void *p, void *p2);
#endif
#endif /* defined(WL_WIRELESS_EXT) */
#if (defined(BCM_ROUTER_DHD) && defined(BCM_GMAC3))
	int fwder_unit; /* assigned fwder instance (modulo-FWDER_MAX_UNIT) */
#endif /* BCM_ROUTER_DHD && BCM_GMAC3 */
	dhd_pub_t pub;
	dhd_if_t *iflist[DHD_MAX_IFS]; /* for supporting multiple interfaces */

	void *adapter;			/* adapter information, interrupt, fw path etc. */
	char fw_path[PATH_MAX];		/* path to firmware image */
	char nv_path[PATH_MAX];		/* path to nvram vars file */

	struct semaphore proto_sem;
#ifdef PROP_TXSTATUS
	spinlock_t	wlfc_spinlock;

#ifdef BCMDBUS
	ulong		wlfc_lock_flags;
	ulong		wlfc_pub_lock_flags;
#endif
#endif /* PROP_TXSTATUS */
#ifdef WLMEDIA_HTSF
	htsf_t  htsf;
#endif
	wait_queue_head_t ioctl_resp_wait;
	wait_queue_head_t d3ack_wait;
	uint32	default_wd_interval;

	struct timer_list timer;
	bool wd_timer_valid;
	struct tasklet_struct tasklet;
	spinlock_t	sdlock;
	spinlock_t	txqlock;
	spinlock_t	dhd_lock;
#ifdef BCMDBUS
	ulong		txqlock_flags;
#else

	struct semaphore sdsem;
	tsk_ctl_t	thr_dpc_ctl;
	tsk_ctl_t	thr_wdt_ctl;
#endif /* BCMDBUS */

	tsk_ctl_t	thr_rxf_ctl;
	spinlock_t	rxf_lock;
	bool		rxthread_enabled;

	/* Wakelocks */
#if defined(CONFIG_HAS_WAKELOCK) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27))
	struct wake_lock wl_wifi;   /* Wifi wakelock */
	struct wake_lock wl_rxwake; /* Wifi rx wakelock */
	struct wake_lock wl_ctrlwake; /* Wifi ctrl wakelock */
	struct wake_lock wl_wdwake; /* Wifi wd wakelock */
#ifdef BCMPCIE_OOB_HOST_WAKE
	struct wake_lock wl_intrwake; /* Host wakeup wakelock */
#endif /* BCMPCIE_OOB_HOST_WAKE */
#endif /* CONFIG_HAS_WAKELOCK && LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27) */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 25)) && defined(OEM_ANDROID)
	/* net_device interface lock, prevent race conditions among net_dev interface
	 * calls and wifi_on or wifi_off
	 */
	struct mutex dhd_net_if_mutex;
	struct mutex dhd_suspend_mutex;
#endif
	spinlock_t wakelock_spinlock;
	uint32 wakelock_counter;
	int wakelock_wd_counter;
	int wakelock_rx_timeout_enable;
	int wakelock_ctrl_timeout_enable;
	bool waive_wakelock;
	uint32 wakelock_before_waive;

	/* Thread to issue ioctl for multicast */
	wait_queue_head_t ctrl_wait;
	atomic_t pend_8021x_cnt;
	dhd_attach_states_t dhd_state;

#if defined(CONFIG_HAS_EARLYSUSPEND) && defined(DHD_USE_EARLYSUSPEND)
	struct early_suspend early_suspend;
#endif /* CONFIG_HAS_EARLYSUSPEND && DHD_USE_EARLYSUSPEND */

#ifdef ARP_OFFLOAD_SUPPORT
	u32 pend_ipaddr;
#endif /* ARP_OFFLOAD_SUPPORT */
#ifdef BCM_FD_AGGR
	void *rpc_th;
	void *rpc_osh;
	struct timer_list rpcth_timer;
	bool rpcth_timer_active;
	uint8 fdaggr;
#endif
#ifdef DHDTCPACK_SUPPRESS
	spinlock_t	tcpack_lock;
#endif /* DHDTCPACK_SUPPRESS */
#ifdef CUSTOMER_HW4
#ifdef FIX_CPU_MIN_CLOCK
	bool cpufreq_fix_status;
	struct mutex cpufreq_fix;
	struct pm_qos_request dhd_cpu_qos;
#ifdef FIX_BUS_MIN_CLOCK
	struct pm_qos_request dhd_bus_qos;
#endif /* FIX_BUS_MIN_CLOCK */
#endif /* FIX_CPU_MIN_CLOCK */
#endif /* CUSTOMER_HW4 */
	void			*dhd_deferred_wq;
#if defined(BCM_CPE_PKTC)
	struct wl_pktc_tbl *pktc_tbl;
#endif /* BCM_CPE_PKTC */
#ifdef DEBUG_CPU_FREQ
	struct notifier_block freq_trans;
	int __percpu *new_freq;
#endif
#if defined(CONFIG_PM_SLEEP)
	struct notifier_block pm_notifier;
#endif /* CONFIG_PM_SLEEP */
#ifdef DHD_PSTA
	uint32	psta_mode;	/* PSTA or PSR */
#endif /* DHD_PSTA */
#ifdef DHD_WET
	uint32	wet_mode;
#endif /* DHD_PSTA */
#ifdef DHD_DEBUG
	dhd_dump_t *dump;
	struct timer_list join_timer;
	u32 join_timeout_val;
	bool join_timer_active;
	uint scan_time_count;
	struct timer_list scan_timer;
	bool scan_timer_active;
#endif

#if defined(DHD_LB)
	/* CPU Load Balance dynamic CPU selection */

	/* Variable that tracks the currect CPUs available for candidacy */
	cpumask_var_t cpumask_curr_avail;

	/* Primary and secondary CPU mask */
	cpumask_var_t cpumask_primary, cpumask_secondary; /* configuration */
	cpumask_var_t cpumask_primary_new, cpumask_secondary_new; /* temp */

	struct notifier_block cpu_notifier;

	/* Tasklet to handle Tx Completion packet freeing */
	struct tasklet_struct tx_compl_tasklet;
	atomic_t	tx_compl_cpu;

	/* Tasklet to handle RxBuf Post during Rx completion */
	struct tasklet_struct rx_compl_tasklet;
	atomic_t	rx_compl_cpu;

	/* Napi struct for handling rx packet sendup. Packets are removed from
	 * H2D RxCompl ring and placed into rx_pend_queue. rx_pend_queue is then
	 * appended to rx_napi_queue (w/ lock) and the rx_napi_struct is scheduled
	 * to run to rx_napi_cpu.
	 */
	struct sk_buff_head   rx_pend_queue  ____cacheline_aligned;
	struct sk_buff_head   rx_napi_queue  ____cacheline_aligned;
	struct napi_struct    rx_napi_struct ____cacheline_aligned;
	atomic_t	rx_napi_cpu; /* cpu on which the napi is dispatched */
	struct net_device    *rx_napi_netdev; /* netdev of primary interface */

	struct work_struct    rx_napi_dispatcher_work;
	struct work_struct    tx_compl_dispatcher_work;
	struct work_struct    rx_compl_dispatcher_work;
	/* Number of times DPC Tasklet ran */
	uint32	dhd_dpc_cnt;

	/* Number of times NAPI processing got scheduled */
	uint32	napi_sched_cnt;

	/* Number of times NAPI processing ran on each available core */
	uint32	napi_percpu_run_cnt[NR_CPUS];

	/* Number of times RX Completions got scheduled */
	uint32	rxc_sched_cnt;
	/* Number of times RX Completion ran on each available core */
	uint32	rxc_percpu_run_cnt[NR_CPUS];

	/* Number of times TX Completions got scheduled */
	uint32	txc_sched_cnt;
	/* Number of times TX Completions ran on each available core */
	uint32	txc_percpu_run_cnt[NR_CPUS];

	/* CPU status */
	/* Number of times each CPU came online */
	uint32	cpu_online_cnt[NR_CPUS];

	/* Number of times each CPU went offline */
	uint32	cpu_offline_cnt[NR_CPUS];

	/*
	 * Consumer Histogram - NAPI RX Packet processing
	 * -----------------------------------------------
	 * On Each CPU, when the NAPI RX Packet processing call back was invoked
	 * how many packets were processed is captured in this data structure.
	 * Now its difficult to capture the "exact" number of packets processed.
	 * So considering the packet counter to be a 32 bit one, we have a
	 * bucket with 8 bins (2^1, 2^2 ... 2^8). The "number" of packets
	 * processed is rounded off to the next power of 2 and put in the
	 * approriate "bin" the value in the bin gets incremented.
	 * For example, assume that in CPU 1 if NAPI Rx runs 3 times
	 * and the packet count processed is as follows (assume the bin counters are 0)
	 * iteration 1 - 10 (the bin counter 2^4 increments to 1)
	 * iteration 2 - 30 (the bin counter 2^5 increments to 1)
	 * iteration 3 - 15 (the bin counter 2^4 increments by 1 to become 2)
	 */
	uint32 napi_rx_hist[NR_CPUS][HIST_BIN_SIZE];
	uint32 txc_hist[NR_CPUS][HIST_BIN_SIZE];
	uint32 rxc_hist[NR_CPUS][HIST_BIN_SIZE];
#endif /* DHD_LB */

#if defined(BCM_DNGL_EMBEDIMAGE) || defined(BCM_REQUEST_FW)
#if defined(BCMDBUS)
	struct task_struct *fw_download_task;
	struct semaphore fw_download_lock;
#endif /* BCMDBUS */
#endif /* defined(BCM_DNGL_EMBEDIMAGE) || defined(BCM_REQUEST_FW) */
	mm_segment_t fs;     /**< allows file seeking/reading from kernel module */
#ifdef WL_MONITOR
	struct net_device *monitor_dev; /* monitor pseudo device */
	uint monitor_type;   /* monitor pseudo device */
	struct sk_buff *monitor_skb;
	monitor_info_t *monitor_info;
	uint32          corerev;
#endif /* WL_MONITOR */
	uint16 dump_signature;		/* dump filename signature */
	uint8 dtrace_cur_fidx;		/* dtrace current file index */
	uint8 dtrace_max_fidx;		/* max file index of created dtrace files */
	uint32 dtrace_cur_fsz;		/* dtrace durrent file byte size */
	uint32 macdbg_dump_level;	/* list bitmap of macdbg dump allowed */
} dhd_info_t;

#ifdef BCM_NBUFF_WLMCAST_IPV6
static void *dhd_nic_hook_fn(int cmd, void *p, void *p2);
#endif

/* Flag to indicate if we should download firmware on driver load */
uint dhd_download_fw_on_driverload = TRUE;

/* Definitions to provide path to the firmware and nvram
 * example nvram_path[MOD_PARAM_PATHLEN]="/projects/wlan/nvram.txt"
 */
char firmware_path[MOD_PARAM_PATHLEN];
char nvram_path[MOD_PARAM_PATHLEN];

/* backup buffer for firmware and nvram path */
char fw_bak_path[MOD_PARAM_PATHLEN];
char nv_bak_path[MOD_PARAM_PATHLEN];

#if defined(BCM_DHD_RUNNER) && !defined(BCM_COUNTER_EXTSTATS)
static void dhd_update_fp_stats(struct net_device * dev_p, BlogStats_t * blogStats_p);
static void dhd_clear_stats(struct net_device *net);
static void *dhd_get_stats_pointer(struct net_device *dev_p, char type);
#endif

/* information string to keep firmware, chio, cheip version info visiable from log */
char info_string[MOD_PARAM_INFOLEN];
module_param_string(info_string, info_string, MOD_PARAM_INFOLEN, 0444);
int op_mode = 0;
int disable_proptx = 0;
module_param(op_mode, int, 0644);

#if defined(DHD_LB_RXP)
static int dhd_napi_weight = 32;
module_param(dhd_napi_weight, int, 0644);
#endif /* DHD_LB_RXP */

#if defined(OEM_ANDROID)
extern int wl_control_wl_start(struct net_device *dev);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)) && defined(BCMLXSDMMC)
struct semaphore dhd_registration_sem;
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)) */
#endif /* defined(OEM_ANDROID) */

/* deferred handlers */
static void dhd_ifadd_event_handler(void *handle, void *event_info, enum dhd_wq_event event);
static void dhd_ifdel_event_handler(void *handle, void *event_info, enum dhd_wq_event event);
static void dhd_set_mac_addr_handler(void *handle, void *event_info, enum dhd_wq_event event);
static void dhd_set_mcast_list_handler(void *handle, void *event_info, enum dhd_wq_event event);
#ifdef DHD_WMF
static void dhd_set_wmf_handler(void *handle, void *event_info, enum dhd_wq_event event);
#endif /* DHD_WMF */
static void dhd_fwtrap_handler(void *handle, void *event_info, enum dhd_wq_event event);
static void dhd_dtrace_handler(void *handle, void *event_info, enum dhd_wq_event event);
#if defined(OEM_ANDROID) && defined(CONFIG_IPV6)
static void dhd_inet6_work_handler(void *dhd_info, void *event_data, enum dhd_wq_event event);
#endif /* OEM_ANDROID && CONFIG_IPV6 */

#if (defined(DHD_WET) || defined(DHD_MCAST_REGEN) || defined(DHD_L2_FILTER))
/* update rx_pkt_chainable state of dhd interface */
static void dhd_update_rx_pkt_chainable_state(dhd_pub_t* dhdp, uint32 idx);
#endif /* DHD_WET || DHD_MCAST_REGEN || DHD_L2_FILTER */

/* Error bits */
module_param(dhd_msg_level, int, 0);

#ifdef ARP_OFFLOAD_SUPPORT
/* ARP offload enable */
uint dhd_arp_enable = TRUE;
module_param(dhd_arp_enable, uint, 0);

/* ARP offload agent mode : Enable ARP Host Auto-Reply and ARP Peer Auto-Reply */

#if defined(CUSTOMER_HW4)
uint dhd_arp_mode = ARP_OL_AGENT | ARP_OL_PEER_AUTO_REPLY | ARP_OL_SNOOP;
#else
uint dhd_arp_mode = ARP_OL_AGENT | ARP_OL_PEER_AUTO_REPLY;
#endif

module_param(dhd_arp_mode, uint, 0);
#endif /* ARP_OFFLOAD_SUPPORT */

#if !defined(BCMDBUS)||defined(OEM_ANDROID)
/* Disable Prop tx */
module_param(disable_proptx, int, 0644);
/* load firmware and/or nvram values from the filesystem */
module_param_string(firmware_path, firmware_path, MOD_PARAM_PATHLEN, 0660);
module_param_string(nvram_path, nvram_path, MOD_PARAM_PATHLEN, 0660);

/* Watchdog interval */

/* extend watchdog expiration to 2 seconds when DPC is running */
#define WATCHDOG_EXTEND_INTERVAL (2000)

uint dhd_watchdog_ms = CUSTOM_DHD_WATCHDOG_MS;
module_param(dhd_watchdog_ms, uint, 0);

/* Console poll interval */
#if defined(OEM_ANDROID)
uint dhd_console_ms = 0;
#else
uint dhd_console_ms = 250;
#endif
module_param(dhd_console_ms, uint, 0644);

uint dhd_slpauto = TRUE;
module_param(dhd_slpauto, uint, 0);

#ifdef PKT_FILTER_SUPPORT
/* Global Pkt filter enable control */
uint dhd_pkt_filter_enable = TRUE;
module_param(dhd_pkt_filter_enable, uint, 0);
#endif

/* Pkt filter init setup */
uint dhd_pkt_filter_init = 0;
module_param(dhd_pkt_filter_init, uint, 0);

/* Pkt filter mode control */
#if defined(CUSTOMER_HW4) && defined(GAN_LITE_NAT_KEEPALIVE_FILTER)
uint dhd_master_mode = FALSE;
#else
uint dhd_master_mode = TRUE;
#endif /* CUSTOMER_HW4 && GAN_LITE_NAT_KEEPALIVE_FILTER */
module_param(dhd_master_mode, uint, 0);

int dhd_watchdog_prio = 0;
module_param(dhd_watchdog_prio, int, 0);

/* DPC thread priority */
int dhd_dpc_prio = CUSTOM_DPC_PRIO_SETTING;
module_param(dhd_dpc_prio, int, 0);

/* RX frame thread priority */
int dhd_rxf_prio = CUSTOM_RXF_PRIO_SETTING;
module_param(dhd_rxf_prio, int, 0);

#if !defined(BCMDHDUSB)
extern int dhd_dongle_ramsize;
module_param(dhd_dongle_ramsize, int, 0);
#endif /* BCMDHDUSB */
#endif /* BCMDBUS */

#ifdef WL_CFG80211
int passive_channel_skip = 0;
module_param(passive_channel_skip, int, (S_IRUSR|S_IWUSR));
#endif /* WL_CFG80211 */

/* Keep track of number of instances */
static int dhd_found = 0;
static int instance_base = 0; /* Starting instance number */
module_param(instance_base, int, 0644);

/* instance order override */
extern uint16 dhdpcie_get_deviceid(struct dhd_bus *);

#if defined(CMWIFI_RDKB)
static int nvram_ifid_override = 1; /* enabled on cmwifi platforms */
#else
static int nvram_ifid_override = 0;
#endif
module_param(nvram_ifid_override, int, 0644);

#if defined(BCM_ROUTER_DHD)
/*
 * nvram based ifid assignment: allocates wl<ifid> based on device id
 */
static uint32 nvram_ifid_map = 0xFFFFFFFF;
#define DHD_NVRAM_MAX_IFID			(0xF)
#define DHD_NVRAM_INVALID_IFID			(0xF)

#define DHD_GET_NVRAM_IFID(radio)		((nvram_ifid_map >> (radio * 4)) & 0xF)
#define DHD_SET_NVRAM_IFID(radio, ifid)		                     \
		(nvram_ifid_map = (nvram_ifid_map & ~(0xF << (radio * 4))) |  \
		 (ifid << (radio * 4)))

#endif /* BCM_ROUTER_DHD */

#if defined(BCMDBG) || defined(DHD_DEBUG)
struct bootp_fmt {
	struct iphdr ip_header;
	struct udphdr udp_header;
	uint8 op;
	uint8 htype;
	uint8 hlen;
	uint8 hops;
	uint32 transaction_id;
	uint16 secs;
	uint16 flags;
	uint32 client_ip;
	uint32 assigned_ip;
	uint32 server_ip;
	uint32 relay_ip;
	uint8 hw_address[16];
	uint8 server_name[64];
	uint8 file_name[128];
	uint8 options[312];
};

static const uint8 bootp_magic_cookie[4] = { 99, 130, 83, 99 };
static const char dhcp_ops[][10] = {
	"NA", "REQUEST", "REPLY"
};
static const char dhcp_types[][10] = {
	"NA", "DISCOVER", "OFFER", "REQUEST", "DECLINE", "ACK", "NAK", "RELEASE", "INFORM"
};
static void dhd_dhcp_dump(uint8 *pktdata, bool tx);
#endif /* defined(BCMDBG) || defined(DHD_DEBUG) */

#ifdef WL_CFG80211
static s32 dhd_cfg80211_enabled(void);
#endif /* WL_CFG80211 */

#if defined(DHD_LB)

static void
dhd_lb_set_default_cpus(dhd_info_t *dhd)
{
	/* Default CPU allocation for the jobs */
	atomic_set(&dhd->rx_napi_cpu, 1);
	atomic_set(&dhd->rx_compl_cpu, 2);
	atomic_set(&dhd->tx_compl_cpu, 2);
}

static int
dhd_cpumasks_init(dhd_info_t *dhd)
{
	int id;
	uint32 cpus;
	int ret = 0;

	if (!alloc_cpumask_var(&dhd->cpumask_curr_avail, GFP_KERNEL) ||
		!alloc_cpumask_var(&dhd->cpumask_primary, GFP_KERNEL) ||
		!alloc_cpumask_var(&dhd->cpumask_primary_new, GFP_KERNEL) ||
		!alloc_cpumask_var(&dhd->cpumask_secondary, GFP_KERNEL) ||
		!alloc_cpumask_var(&dhd->cpumask_secondary_new, GFP_KERNEL)) {
		DHD_ERROR(("%s Failed to init cpumasks\n", __FUNCTION__));
		ret = -ENOMEM;
		goto fail;
	}

	cpumask_copy(dhd->cpumask_curr_avail, cpu_online_mask);
	cpumask_clear(dhd->cpumask_primary);
	cpumask_clear(dhd->cpumask_secondary);

	cpus = DHD_LB_PRIMARY_CPUS;
	for (id = 0; id < NR_CPUS; id++) {
		if (isset(&cpus, id))
			cpumask_set_cpu(id, dhd->cpumask_primary);
	}

	cpus = DHD_LB_SECONDARY_CPUS;
	for (id = 0; id < NR_CPUS; id++) {
		if (isset(&cpus, id))
			cpumask_set_cpu(id, dhd->cpumask_secondary);
	}

	return ret;

fail:
	if (dhd->cpumask_curr_avail) {
		free_cpumask_var(dhd->cpumask_curr_avail);
	}

	if (dhd->cpumask_primary) {
		free_cpumask_var(dhd->cpumask_primary);
	}

	if (dhd->cpumask_primary_new) {
		free_cpumask_var(dhd->cpumask_primary_new);
	}

	if (dhd->cpumask_secondary) {
		free_cpumask_var(dhd->cpumask_secondary);
	}

	if (dhd->cpumask_secondary_new) {
		free_cpumask_var(dhd->cpumask_secondary_new);
	}

	return ret;
}

static void
dhd_cpumasks_deinit(dhd_info_t *dhd)
{
	if (dhd->cpumask_curr_avail) {
		free_cpumask_var(dhd->cpumask_curr_avail);
	}
	if (dhd->cpumask_primary) {
		free_cpumask_var(dhd->cpumask_primary);
	}
	if (dhd->cpumask_primary_new) {
		free_cpumask_var(dhd->cpumask_primary_new);
	}
	if (dhd->cpumask_secondary) {
		free_cpumask_var(dhd->cpumask_secondary);
	}
	if (dhd->cpumask_secondary_new) {
		free_cpumask_var(dhd->cpumask_secondary_new);
	}
}

/*
 * The CPU Candidacy Algorithm
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * The available CPUs for selection are divided into two groups
 *  Primary Set - A CPU mask that carries the First Choice CPUs
 *  Secondary Set - A CPU mask that carries the Second Choice CPUs.
 *
 * There are two types of Job, that needs to be assigned to
 * the CPUs, from one of the above mentioned CPU group. The Jobs are
 * 1) Rx Packet Processing - napi_cpu
 * 2) Completion Processiong (Tx, RX) - compl_cpu
 *
 * To begin with both napi_cpu and compl_cpu are on CPU0. Whenever a CPU goes
 * on-line/off-line the CPU candidacy algorithm is triggerd. The candidacy
 * algo tries to pickup the first available non boot CPU (CPU0) for napi_cpu.
 * If there are more processors free, it assigns one to compl_cpu.
 * It also tries to ensure that both napi_cpu and compl_cpu are not on the same
 * CPU, as much as possible.
 *
 * By design, both Tx and Rx completion jobs are run on the same CPU core, as it
 * would allow Tx completion skb's to be released into a local free pool from
 * which the rx buffer posts could have been serviced. it is important to note
 * that a Tx packet may not have a large enough buffer for rx posting.
 */
static void
dhd_select_cpu_candidacy(dhd_info_t *dhd)
{
	uint32 available_cpus; /* count of available cpus */
	uint32 napi_cpu = 0; /* cpu selected for napi rx processing */
	uint32 compl_cpu = 0; /* cpu selected for completion jobs */

	cpumask_clear(dhd->cpumask_primary_new);
	cpumask_clear(dhd->cpumask_secondary_new);

	/*
	 * Now select from the primary mask. Even if a Job is
	 * already running on a CPU in secondary group, we still move
	 * to primary CPU. So no conditional checks.
	 */
	cpumask_and(dhd->cpumask_primary_new, dhd->cpumask_primary,
		dhd->cpumask_curr_avail);

	cpumask_and(dhd->cpumask_secondary_new, dhd->cpumask_secondary,
		dhd->cpumask_curr_avail);

	available_cpus = cpumask_weight(dhd->cpumask_primary_new);

	if (available_cpus > 0) {
		napi_cpu = cpumask_first(dhd->cpumask_primary_new);

		/* If no further CPU is available,
		 * cpumask_next returns >= nr_cpu_ids
		 */
		compl_cpu = cpumask_next(napi_cpu, dhd->cpumask_primary_new);
		if (compl_cpu >= nr_cpu_ids)
			compl_cpu = 0;
	}

	DHD_INFO(("%s After primary CPU check napi_cpu %d compl_cpu %d\n",
		__FUNCTION__, napi_cpu, compl_cpu));

	/* -- Now check for the CPUs from the secondary mask -- */
	available_cpus = cpumask_weight(dhd->cpumask_secondary_new);

	DHD_INFO(("%s Available secondary cpus %d nr_cpu_ids %d\n",
		__FUNCTION__, available_cpus, nr_cpu_ids));

	if (available_cpus > 0) {
		/* At this point if napi_cpu is unassigned it means no CPU
		 * is online from Primary Group
		 */
		if (napi_cpu == 0) {
			napi_cpu = cpumask_first(dhd->cpumask_secondary_new);
			compl_cpu = cpumask_next(napi_cpu, dhd->cpumask_secondary_new);
		} else if (compl_cpu == 0) {
			compl_cpu = cpumask_first(dhd->cpumask_secondary_new);
		}

		/* If no CPU was available for completion, choose CPU 0 */
		if (compl_cpu >= nr_cpu_ids)
			compl_cpu = 0;
	} else {
		/* No CPUs available from primary or secondary mask */
		napi_cpu = 0;
		compl_cpu = 0;
	}

	DHD_INFO(("%s After secondary CPU check napi_cpu %d compl_cpu %d\n",
		__FUNCTION__, napi_cpu, compl_cpu));
	ASSERT(napi_cpu < nr_cpu_ids);
	ASSERT(compl_cpu < nr_cpu_ids);

	atomic_set(&dhd->rx_napi_cpu, napi_cpu);
	atomic_set(&dhd->tx_compl_cpu, compl_cpu);
	atomic_set(&dhd->rx_compl_cpu, compl_cpu);

	return;
}

/*
 * Function to handle CPU Hotplug notifications.
 * One of the task it does is to trigger the CPU Candidacy algorithm
 * for load balancing.
 */
static int
dhd_cpu_callback(struct notifier_block *nfb, unsigned long action, void *hcpu)
{
	unsigned int cpu = (unsigned int)(long)hcpu;

	dhd_info_t *dhd = container_of(nfb, dhd_info_t, cpu_notifier);

	switch (action)
	{
		case CPU_ONLINE:
			DHD_LB_STATS_INCR(dhd->cpu_online_cnt[cpu]);
			cpumask_set_cpu(cpu, dhd->cpumask_curr_avail);
			dhd_select_cpu_candidacy(dhd);
			break;

		case CPU_DOWN_PREPARE:
		case CPU_DOWN_PREPARE_FROZEN:
			DHD_LB_STATS_INCR(dhd->cpu_offline_cnt[cpu]);
			cpumask_clear_cpu(cpu, dhd->cpumask_curr_avail);
			dhd_select_cpu_candidacy(dhd);
			break;
		default:
			break;
	}

	return NOTIFY_OK;
}

#if defined(DHD_LB_STATS)
void
dhd_lb_stats_init(dhd_pub_t *dhdp)
{
	dhd_info_t *dhd;
	int i, j;

	if (dhdp == NULL) {
		DHD_ERROR(("%s(): Invalid argument dhdp is NULL \n",
			__FUNCTION__));
		return;
	}

	dhd = dhdp->info;
	if (dhd == NULL) {
		DHD_ERROR(("%s(): DHD pointer is NULL \n", __FUNCTION__));
		return;
	}

	DHD_LB_STATS_CLR(dhd->dhd_dpc_cnt);
	DHD_LB_STATS_CLR(dhd->napi_sched_cnt);
	DHD_LB_STATS_CLR(dhd->rxc_sched_cnt);
	DHD_LB_STATS_CLR(dhd->txc_sched_cnt);

	for (i = 0; i < NR_CPUS; i++) {
		DHD_LB_STATS_CLR(dhd->napi_percpu_run_cnt[i]);
		DHD_LB_STATS_CLR(dhd->rxc_percpu_run_cnt[i]);
		DHD_LB_STATS_CLR(dhd->txc_percpu_run_cnt[i]);

		DHD_LB_STATS_CLR(dhd->cpu_online_cnt[i]);
		DHD_LB_STATS_CLR(dhd->cpu_offline_cnt[i]);
	}

	for (i = 0; i < NR_CPUS; i++) {
		for (j = 0; j < HIST_BIN_SIZE; j++) {
			DHD_LB_STATS_CLR(dhd->napi_rx_hist[i][j]);
			DHD_LB_STATS_CLR(dhd->txc_hist[i][j]);
			DHD_LB_STATS_CLR(dhd->rxc_hist[i][j]);
		}
	}

	return;
}

static void
dhd_lb_stats_dump_histo(
	struct bcmstrbuf *strbuf, uint32 (*hist)[HIST_BIN_SIZE])
{
	int i, j;
	uint32 per_cpu_total[NR_CPUS] = {0};
	uint32 total = 0;

	bcm_bprintf(strbuf, "CPU: \t\t");
	for (i = 0; i < num_possible_cpus(); i++)
		bcm_bprintf(strbuf, "%d\t", i);
	bcm_bprintf(strbuf, "\nBin\n");

	for (i = 0; i < HIST_BIN_SIZE; i++) {
		bcm_bprintf(strbuf, "%d:\t\t", 1<<(i+1));
		for (j = 0; j < num_possible_cpus(); j++) {
			bcm_bprintf(strbuf, "%d\t", hist[j][i]);
		}
		bcm_bprintf(strbuf, "\n");
	}
	bcm_bprintf(strbuf, "Per CPU Total \t");
	total = 0;
	for (i = 0; i < num_possible_cpus(); i++) {
		for (j = 0; j < HIST_BIN_SIZE; j++) {
			per_cpu_total[i] += (hist[i][j] * (1<<(j+1)));
		}
		bcm_bprintf(strbuf, "%d\t", per_cpu_total[i]);
		total += per_cpu_total[i];
	}
	bcm_bprintf(strbuf, "\nTotal\t\t%d \n", total);

	return;
}

static inline void
dhd_lb_stats_dump_cpu_array(struct bcmstrbuf *strbuf, uint32 *p)
{
	int i;

	bcm_bprintf(strbuf, "CPU: \t");
	for (i = 0; i < num_possible_cpus(); i++)
		bcm_bprintf(strbuf, "%d\t", i);
	bcm_bprintf(strbuf, "\n");

	bcm_bprintf(strbuf, "Val: \t");
	for (i = 0; i < num_possible_cpus(); i++)
		bcm_bprintf(strbuf, "%u\t", *(p+i));
	bcm_bprintf(strbuf, "\n");

	return;
}

void
dhd_lb_stats_dump(dhd_pub_t *dhdp, struct bcmstrbuf *strbuf)
{
	dhd_info_t *dhd;

	if (dhdp == NULL || strbuf == NULL) {
		DHD_ERROR(("%s(): Invalid argument dhdp %p strbuf %p \n",
			__FUNCTION__, dhdp, strbuf));
		return;
	}

	dhd = dhdp->info;
	if (dhd == NULL) {
		DHD_ERROR(("%s(): DHD pointer is NULL \n", __FUNCTION__));
		return;
	}

	bcm_bprintf(strbuf, "\ncpu_online_cnt:\n");
	dhd_lb_stats_dump_cpu_array(strbuf, dhd->cpu_online_cnt);

	bcm_bprintf(strbuf, "cpu_offline_cnt:\n");
	dhd_lb_stats_dump_cpu_array(strbuf, dhd->cpu_offline_cnt);

	bcm_bprintf(strbuf, "\nsched_cnt: dhd_dpc %u napi %u rxc %u txc %u\n",
		dhd->dhd_dpc_cnt, dhd->napi_sched_cnt, dhd->rxc_sched_cnt,
		dhd->txc_sched_cnt);
#ifdef DHD_LB_RXP
	bcm_bprintf(strbuf, "napi_percpu_run_cnt:\n");
	dhd_lb_stats_dump_cpu_array(strbuf, dhd->napi_percpu_run_cnt);
	bcm_bprintf(strbuf, "\nNAPI Packets Received Histogram:\n");
	dhd_lb_stats_dump_histo(strbuf, dhd->napi_rx_hist);
#endif /* DHD_LB_RXP */

#ifdef DHD_LB_RXC
	bcm_bprintf(strbuf, "rxc_percpu_run_cnt:\n");
	dhd_lb_stats_dump_cpu_array(strbuf, dhd->rxc_percpu_run_cnt);
	bcm_bprintf(strbuf, "\nRX Completions (Buffer Post) Histogram:\n");
	dhd_lb_stats_dump_histo(strbuf, dhd->rxc_hist);
#endif /* DHD_LB_RXC */

#ifdef DHD_LB_TXC
	bcm_bprintf(strbuf, "txc_percpu_run_cnt:\n");
	dhd_lb_stats_dump_cpu_array(strbuf, dhd->txc_percpu_run_cnt);
	bcm_bprintf(strbuf, "\nTX Completions (Buffer Free) Histogram:\n");
	dhd_lb_stats_dump_histo(strbuf, dhd->txc_hist);
#endif /* DHD_LB_TXC */
}

static void
dhd_lb_stats_update_histo(uint32 *bin, uint32 count)
{
	uint32 bin_power;
	uint32 *p = NULL;

	bin_power = next_larger_power2(count);

	switch (bin_power) {
		case   0: break;
		case   1: /* Fall through intentionally */
		case   2: p = bin + 0; break;
		case   4: p = bin + 1; break;
		case   8: p = bin + 2; break;
		case  16: p = bin + 3; break;
		case  32: p = bin + 4; break;
		case  64: p = bin + 5; break;
		case 128: p = bin + 6; break;
		default : p = bin + 7; break;
	}

	if (p)
		*p = *p + 1;

	return;
}

extern void
dhd_lb_stats_update_napi_histo(dhd_pub_t *dhdp, uint32 count)
{
	int cpu;
	dhd_info_t *dhd = dhdp->info;

	cpu = get_cpu();
	put_cpu();
	dhd_lb_stats_update_histo(&dhd->napi_rx_hist[cpu][0], count);

	return;
}

extern void
dhd_lb_stats_update_txc_histo(dhd_pub_t *dhdp, uint32 count)
{
	int cpu;
	dhd_info_t *dhd = dhdp->info;

	cpu = get_cpu();
	put_cpu();
	dhd_lb_stats_update_histo(&dhd->txc_hist[cpu][0], count);

	return;
}

extern void
dhd_lb_stats_update_rxc_histo(dhd_pub_t *dhdp, uint32 count)
{
	int cpu;
	dhd_info_t *dhd = dhdp->info;

	cpu = get_cpu();
	put_cpu();
	dhd_lb_stats_update_histo(&dhd->rxc_hist[cpu][0], count);

	return;
}

extern void
dhd_lb_stats_txc_percpu_cnt_incr(dhd_pub_t *dhdp)
{
	dhd_info_t *dhd = dhdp->info;
	DHD_LB_STATS_PERCPU_ARR_INCR(dhd->txc_percpu_run_cnt);
}

extern void
dhd_lb_stats_rxc_percpu_cnt_incr(dhd_pub_t *dhdp)
{
	dhd_info_t *dhd = dhdp->info;
	DHD_LB_STATS_PERCPU_ARR_INCR(dhd->rxc_percpu_run_cnt);
}

#endif /* DHD_LB_STATS */
#endif /* DHD_LB */

#if defined(CUSTOMER_HW4) && defined(DISABLE_FRAMEBURST_VSDB) && \
	defined(USE_WFA_CERT_CONF)
int g_frameburst = 1;
#endif /* CUSTOMER_HW4 && DISABLE_FRAMEBURST_VSDB && USE_WFA_CERT_CONF */

static int dhd_get_pend_8021x_cnt(dhd_info_t *dhd);

#if (defined(BCM_ROUTER_DHD) && defined(BCM_GMAC3))

char dhd_dpc_thr_name[FWDER_MAX_RADIO][10];

/** DHD_FWDER_UNIT(): Fetch the assigned fwder_unit for this radio. */
#define DHD_FWDER_UNIT(dhd)     ((dhd)->fwder_unit)

/** Perimeter lock for dhd in 3 GMAC mode.
 * In 3 GMAC mode, the forwarding between ET <---> DHD, occurs on the same CPU
 * core with bottom halves disabled. Forwarding is via dhd_forward() bypass
 * path. dhd_forward() will grab all dhd unit's perimeter locks that it is
 * managing. dhd_forward() may invoke dhd_start_xmit_try()* (see note below).
 * dhd_start_xmit() may be invoked by the network stack. dhd_start_xmit() will
 * invoke dhd_start_xmit_try().
 *
 * As dhd_start_xmit_try() is reentrant. When invoked by dhd_forward(), an
 * indication that the lock has already been acquired is provided, so a nested
 * lock is not taken.
 *
 * All other locks in dhd are disabled when a bypass 3 GMAC is compiled and only
 * the perimeter lock is taken with bottom halves disabled.
 *
 */
#if defined(PCIE_ISR_THREAD)
#error "PCIE_ISR_THREAD not compatible with PERIM bottom half spin locks"
#endif

typedef struct dhd_radio_unit {
	dhd_pub_t * dhdp; /* dhd_pub object associated with radio */
#if defined(CONFIG_SMP)
	spinlock_t  perim_lock; /* perimeter spinlock for unit */
	int         processor_id; /* processor on which the dhd unit is mapped */
#endif /* CONFIG_SMP */
} dhd_radio_unit_t;

/* GMAC3 supports a max of 3 radio units, fwder_unit = [0 .. FWDER_MAX_RADIO).
 *
 * One of the fwder_unit entires may not be used, (see hndfwd.h) e.g.
 * Config #1: "d:l:5:163:0 d:x:2:163:0 d:u:5:169:1" 5G_LO=<0> 2G=<2> 5G_HI=<1>
 * Config #2: "d:u:5:163:0 d:x:2:169:1 d:l:5:169:1" 5G_HI=<0> 2G=<1> 5G_LO=<3>
 * Config #3: "d:x:2:163:0 d:l:5:163:0 d:u:5:169:1" 2G=<0> 5G_LO=<2> 5G_HI=<1>
 * Config #4: "d:u:5:163:0 d:l:5:169:1 d:x:2:169:1" 5G_HI=<0> 5G_LO=<1> 2G=<3>
 * Config #5: "d:x:2:169:1 d:l:5:169:1 d:u:5:163:0" 2G=<1> 5G_LO=<3> 5G_HI=<0>
 *
 * BCM4709acdcrh: uses config #1
 */
/* XXX
 * BCM  software on R8000: Config #2
 * FXCN software on R8000: Config #5 [PCIE bus 0,1 probe sequence swapped]
 */
#define DHD_MAX_RADIO    (FWDER_MAX_RADIO)

#define DHD_PERIM_LOCK_TAKEN    (TRUE)
#define DHD_PERIM_LOCK_NOTTAKEN (FALSE)

typedef struct dhd_radio {
	dhd_radio_unit_t fwder_unit[FWDER_MAX_RADIO];
	unsigned long irq_flags; /* UniProcessor irq save/restore flags */
} dhd_radio_t;

/* Global dhd bypass lock accessible outside the dhd_info instance. */
dhd_radio_t dhd_radio_g;

/* Forward declarations */
static void dhd_perim_radio_init(void);

/* Register a radio with the perimeter lock and radio management subsystem */
static void dhd_perim_radio_reg(int fwder_unit, dhd_pub_t * dhdp);

/** Perimeter Lock/Unlock a radio, with a try option */
static inline void dhd_perim_lock_try(int fwder_unit, const bool lock_taken);
static inline void dhd_perim_unlock_try(int fwder_unit, const bool lock_taken);

/** Initialize the bypass locking system */
static void
dhd_perim_radio_init(void)
{
	int fwder_unit;
	for (fwder_unit = 0; fwder_unit < DHD_MAX_RADIO; fwder_unit++) {
		dhd_radio_g.fwder_unit[fwder_unit].dhdp = (dhd_pub_t *)NULL;
#if defined(CONFIG_SMP)
		spin_lock_init(&dhd_radio_g.fwder_unit[fwder_unit].perim_lock);
		dhd_radio_g.fwder_unit[fwder_unit].processor_id = fwder_unit % FWDER_MAX_UNIT;
#endif /* CONFIG_SMP */
	}
	dhd_radio_g.irq_flags = 0;
}

static void
dhd_perim_radio_reg(int fwder_unit, dhd_pub_t * dhdp)
{
	dhd_radio_g.fwder_unit[fwder_unit].dhdp = dhdp;
}

/* Invoke a dhd_bus callback handler for all busses managed by CPU core */
typedef void (*dhd_perim_bus_cb_t)(struct dhd_bus *dhd_bus);

static inline void
dhd_perim_invoke_all(int processor_id, dhd_perim_bus_cb_t bus_cb)
{
	int fwder_unit;
#if defined(CONFIG_SMP)
	for (fwder_unit = 0; fwder_unit < DHD_MAX_RADIO; fwder_unit++) {
		if ((dhd_radio_g.fwder_unit[fwder_unit].dhdp != NULL) & /* bitwise */
		    (dhd_radio_g.fwder_unit[fwder_unit].processor_id == processor_id)) {
			bus_cb(dhd_radio_g.fwder_unit[fwder_unit].dhdp->bus);
		}
	}
#else  /* ! CONFIG_SMP */
	for (fwder_unit = 0; fwder_unit < DHD_MAX_RADIO; fwder_unit++) {
		if (dhd_radio_g.fwder_unit[fwder_unit].dhdp) {
			bus_cb(dhd_radio_g.fwder_unit[fwder_unit].dhdp->bus);
		}
	}
#endif /* ! CONFIG_SMP */
}

static inline void
dhd_perim_lock_try(int fwder_unit, const bool lock_taken)
{
	if (lock_taken == DHD_PERIM_LOCK_NOTTAKEN) {
#if defined(CONFIG_SMP)
		spin_lock_bh(&dhd_radio_g.fwder_unit[fwder_unit].perim_lock);
#else  /* ! CONFIG_SMP */
		local_irq_save(dhd_radio_g.irq_flags);
#endif /* ! CONFIG_SMP */
	}
}

void BCMFASTPATH
dhd_perim_lock(dhd_pub_t * dhdp)
{
	dhd_perim_lock_try(DHD_FWDER_UNIT(dhdp->info), DHD_PERIM_LOCK_NOTTAKEN);
}

static inline void
dhd_perim_unlock_try(int fwder_unit, const bool lock_taken)
{
	if (lock_taken == DHD_PERIM_LOCK_NOTTAKEN) {
#if defined(CONFIG_SMP)
		spin_unlock_bh(&dhd_radio_g.fwder_unit[fwder_unit].perim_lock);
#else  /* ! CONFIG_SMP */
		local_irq_restore(dhd_radio_g.irq_flags);
#endif /* ! CONFIG_SMP */
	}
}

void BCMFASTPATH
dhd_perim_unlock(dhd_pub_t * dhdp)
{
	dhd_perim_unlock_try(DHD_FWDER_UNIT(dhdp->info), DHD_PERIM_LOCK_NOTTAKEN);
}

void BCMFASTPATH
dhd_perim_lock_all(int processor_id)
{
#if defined(CONFIG_SMP)
	int fwder_unit;
	for (fwder_unit = 0; fwder_unit < DHD_MAX_RADIO; fwder_unit++) {
		if ((dhd_radio_g.fwder_unit[fwder_unit].processor_id == processor_id) &&
			(dhd_radio_g.fwder_unit[fwder_unit].dhdp != NULL)) {
			dhd_perim_lock_try(fwder_unit, DHD_PERIM_LOCK_NOTTAKEN);
		}
	}
#else
	local_irq_save(dhd_radio_g.irq_flags);
#endif /* ! CONFIG_SMP */
}

void BCMFASTPATH
dhd_perim_unlock_all(int processor_id)
{
#if defined(CONFIG_SMP)
	int fwder_unit;
	for (fwder_unit = DHD_MAX_RADIO-1; fwder_unit >= 0; fwder_unit--) {
		if ((dhd_radio_g.fwder_unit[fwder_unit].processor_id == processor_id) &&
			(dhd_radio_g.fwder_unit[fwder_unit].dhdp != NULL)) {
			dhd_perim_unlock_try(fwder_unit, DHD_PERIM_LOCK_NOTTAKEN);
		}
	}
#else
	local_irq_restore(dhd_radio_g.irq_flags);
#endif /* ! CONFIG_SMP */
}

#define DHD_PERIM_RADIO_INIT()              dhd_perim_radio_init()
#define DHD_PERIM_LOCK_TRY(fwder_unit, lock_taken)                             \
	dhd_perim_lock_try((fwder_unit), (lock_taken))
#define DHD_PERIM_UNLOCK_TRY(fwder_unit, lock_taken)                           \
	dhd_perim_unlock_try((fwder_unit), (lock_taken))

#else  /* !(BCM_ROUTER_DHD && BCM_GMAC3) */

/* DHD Perimiter lock only used in router with bypass forwarding. */
#define DHD_PERIM_RADIO_INIT()              do { /* noop */ } while (0)
#define DHD_PERIM_LOCK_TRY(unit, flag)      do { /* noop */ } while (0)
#define DHD_PERIM_UNLOCK_TRY(unit, flag)    do { /* noop */ } while (0)

#endif /* ! (BCM_ROUTER_DHD && BCM_GMAC3) */

#ifdef PCIE_FULL_DONGLE
#if defined(BCM_GMAC3)
#define DHD_IF_STA_LIST_LOCK_INIT(ifp)      do { /* noop */ } while (0)
#define DHD_IF_STA_LIST_LOCK(ifp, flags)    ({ BCM_REFERENCE(flags); })
#define DHD_IF_STA_LIST_UNLOCK(ifp, flags)  ({ BCM_REFERENCE(flags); })

#if defined(DHD_IGMP_UCQUERY) || defined(DHD_UCAST_UPNP)
#define DHD_IF_WMF_UCFORWARD_LOCK(dhd, ifp, slist) ({ BCM_REFERENCE(slist); &(ifp)->sta_list; })
#define DHD_IF_WMF_UCFORWARD_UNLOCK(dhd, slist) ({ BCM_REFERENCE(slist); })
#endif /* DHD_IGMP_UCQUERY || DHD_UCAST_UPNP */

#else /* !BCM_GMAC3 */

#define DHD_IF_STA_LIST_LOCK_INIT(ifp) spin_lock_init(&(ifp)->sta_list_lock)
#define DHD_IF_STA_LIST_LOCK(ifp, flags) \
	spin_lock_irqsave(&(ifp)->sta_list_lock, (flags))
#define DHD_IF_STA_LIST_UNLOCK(ifp, flags) \
	spin_unlock_irqrestore(&(ifp)->sta_list_lock, (flags))

#if defined(DHD_IGMP_UCQUERY) || defined(DHD_UCAST_UPNP)
static struct list_head * dhd_sta_list_snapshot(dhd_info_t *dhd, dhd_if_t *ifp,
	struct list_head *snapshot_list);
static void dhd_sta_list_snapshot_free(dhd_info_t *dhd, struct list_head *snapshot_list);
#define DHD_IF_WMF_UCFORWARD_LOCK(dhd, ifp, slist) ({ dhd_sta_list_snapshot(dhd, ifp, slist); })
#define DHD_IF_WMF_UCFORWARD_UNLOCK(dhd, slist) ({ dhd_sta_list_snapshot_free(dhd, slist); })
#endif /* DHD_IGMP_UCQUERY || DHD_UCAST_UPNP */

#endif /* ! BCM_GMAC3 */
#endif /* PCIE_FULL_DONGLE */

/* Control fw roaming */
#ifdef BCMCCX
uint dhd_roam_disable = 0;
#else
#ifdef OEM_ANDROID
uint dhd_roam_disable = 0;
#else
uint dhd_roam_disable = 1;
#endif
#endif /* BCMCCX */

#ifdef DHD_DEBUG
bool dhd_fw_hang_sendup = FALSE;
#else
bool dhd_fw_hang_sendup = TRUE;
#endif

#ifdef DHD_DEBUGFS
extern int dhd_dbg_init(dhd_pub_t *dhdp);
extern void dhd_dbg_remove(void);
#endif /* DHD_DEBUGFS */

/* Control radio state */
uint dhd_radio_up = 1;

/* Network inteface name */
char iface_name[IFNAMSIZ] = {'\0'};
module_param_string(iface_name, iface_name, IFNAMSIZ, 0);

/* The following are specific to the SDIO dongle */

/* IOCTL response timeout */
int dhd_ioctl_timeout_msec = IOCTL_RESP_TIMEOUT;

/* Idle timeout for backplane clock */
int dhd_idletime = DHD_IDLETIME_TICKS;
module_param(dhd_idletime, int, 0);

/* Use polling */
uint dhd_poll = FALSE;
module_param(dhd_poll, uint, 0);

/* Use interrupts */
uint dhd_intr = TRUE;
module_param(dhd_intr, uint, 0);

/* SDIO Drive Strength (in milliamps) */
uint dhd_sdiod_drive_strength = 6;
module_param(dhd_sdiod_drive_strength, uint, 0);

#ifdef BCMSLTGT
uint htclkratio = 10;
module_param(htclkratio, uint, 0);
#endif

#ifdef SDTEST
/* Echo packet generator (pkts/s) */
uint dhd_pktgen = 0;
module_param(dhd_pktgen, uint, 0);

/* Echo packet len (0 => sawtooth, max 2040) */
uint dhd_pktgen_len = 0;
module_param(dhd_pktgen_len, uint, 0);
#endif /* SDTEST */

#ifdef CUSTOM_DSCP_TO_PRIO_MAPPING
uint dhd_dscpmap_enable = 1;
module_param(dhd_dscpmap_enable, uint, 0644);
module_param(dhd_dscpmap_enable, uint, 0644);
#endif /* CUSTOM_DSCP_TO_PRIO_MAPPING */

#if defined(BCMSUP_4WAY_HANDSHAKE)
/* Use in dongle supplicant for 4-way handshake */
uint dhd_use_idsup = 0;
module_param(dhd_use_idsup, uint, 0);
#endif /* BCMSUP_4WAY_HANDSHAKE */

#ifndef BCMDBUS
#if defined(OEM_ANDROID) || defined(BCM_ROUTER_DHD)
/* Allow delayed firmware download for debug purpose */
int allow_delay_fwdl = FALSE;
#else
int allow_delay_fwdl = TRUE;
#endif /* OEM_ANDROID || BCM_ROUTER_DHD */
module_param(allow_delay_fwdl, int, 0);
#endif /* !BCMDBUS */

extern char dhd_version[];

int dhd_net_bus_devreset(struct net_device *dev, uint8 flag);
static void dhd_net_if_lock_local(dhd_info_t *dhd);
static void dhd_net_if_unlock_local(dhd_info_t *dhd);
static void dhd_suspend_lock(dhd_pub_t *dhdp);
static void dhd_suspend_unlock(dhd_pub_t *dhdp);

#ifdef WLMEDIA_HTSF
void htsf_update(dhd_info_t *dhd, void *data);
tsf_t prev_tsf, cur_tsf;

uint32 dhd_get_htsf(dhd_info_t *dhd, int ifidx);
static int dhd_ioctl_htsf_get(dhd_info_t *dhd, int ifidx);
static void dhd_dump_latency(void);
static void dhd_htsf_addtxts(dhd_pub_t *dhdp, void *pktbuf);
static void dhd_htsf_addrxts(dhd_pub_t *dhdp, void *pktbuf);
static void dhd_dump_htsfhisto(histo_t *his, char *s);
#endif /* WLMEDIA_HTSF */

/* Monitor interface */
int wl_monitor_init(void *dhd_pub);
int wl_monitor_uninit(void);

#if defined(CUSTOMER_HW4) && defined(CONFIG_CONTROL_PM)
bool g_pm_control;
void sec_control_pm(dhd_pub_t *dhd, uint *);
#endif /* CUSTOMER_HW4 & CONFIG_CONTROL_PM */

#if defined(WL_WIRELESS_EXT)
struct iw_statistics *dhd_get_wireless_stats(struct net_device *dev);
#endif /* defined(WL_WIRELESS_EXT) */

#ifndef BCMDBUS
static void dhd_dpc(ulong data);
#endif
/* forward decl */
extern int dhd_wait_pend8021x(struct net_device *dev);
void dhd_os_wd_timer_extend(void *bus, bool extend);

#ifdef TOE
#ifndef BDC
#error TOE requires BDC
#endif /* !BDC */
static int dhd_toe_get(dhd_info_t *dhd, int idx, uint32 *toe_ol);
static int dhd_toe_set(dhd_info_t *dhd, int idx, uint32 toe_ol);
#endif /* TOE */
#ifdef BCMDBUS
int dhd_dbus_txdata(dhd_pub_t *dhdp, void *pktbuf);
#endif

static int dhd_wl_host_event(dhd_info_t *dhd, int *ifidx, void *pktdata, uint16 pktlen,
                             wl_event_msg_t *event_ptr, void **data_ptr);

#if defined(CONFIG_PM_SLEEP)
static int
dhd_pm_callback(struct notifier_block *nfb, unsigned long action, void *ignored)
{
	int ret = NOTIFY_DONE;
	bool suspend = FALSE;
	dhd_info_t *dhdinfo = container_of(nfb, dhd_info_t, pm_notifier);

	BCM_REFERENCE(dhdinfo);
	BCM_REFERENCE(suspend);

	switch (action) {
	case PM_HIBERNATION_PREPARE:
	case PM_SUSPEND_PREPARE:
		suspend = TRUE;
		break;

	case PM_POST_HIBERNATION:
	case PM_POST_SUSPEND:
		suspend = FALSE;
		break;
	}

#if defined(SUPPORT_P2P_GO_PS)
#ifdef PROP_TXSTATUS
	if (suspend) {
		DHD_OS_WAKE_LOCK_WAIVE(&dhdinfo->pub);
		dhd_wlfc_suspend(&dhdinfo->pub);
		DHD_OS_WAKE_LOCK_RESTORE(&dhdinfo->pub);
	} else
		dhd_wlfc_resume(&dhdinfo->pub);
#endif
#endif /* defined(SUPPORT_P2P_GO_PS) */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)) && (LINUX_VERSION_CODE <= \
	KERNEL_VERSION(2, 6, 39))
	if (dhd_mmc_suspend != suspend) {
		dhd_mmc_suspend = suspend;
		smp_mb();
	}
#endif

	return ret;
}

extern int register_pm_notifier(struct notifier_block *nb);     /**< is a Linux function */
extern int unregister_pm_notifier(struct notifier_block *nb);   /**< is a Linux function */
#endif /* CONFIG_PM_SLEEP */

/* Request scheduling of the bus rx frame */
static void dhd_sched_rxf(dhd_pub_t *dhdp, void *skb);
static void dhd_os_rxflock(dhd_pub_t *pub);
static void dhd_os_rxfunlock(dhd_pub_t *pub);

/** priv_link is the link between netdev and the dhdif and dhd_info structs. */
typedef struct dhd_dev_priv {
	dhd_info_t * dhd; /* cached pointer to dhd_info in netdevice priv */
	dhd_if_t   * ifp; /* cached pointer to dhd_if in netdevice priv */
	int          ifidx; /* interface index */
	void       * lkup;
#ifdef BCM_PKTFWD
	dhd_pktfwd_priv_t pktfwd_priv;
#endif /* BCM_PKTFWD */
} dhd_dev_priv_t;

#define DHD_DEV_PRIV_SIZE       (sizeof(dhd_dev_priv_t))
#define DHD_DEV_PRIV(dev)       ((dhd_dev_priv_t *)DEV_PRIV(dev))
#define DHD_DEV_INFO(dev)       (((dhd_dev_priv_t *)DEV_PRIV(dev))->dhd)
#define DHD_DEV_IFP(dev)        (((dhd_dev_priv_t *)DEV_PRIV(dev))->ifp)
#define DHD_DEV_IFIDX(dev)      (((dhd_dev_priv_t *)DEV_PRIV(dev))->ifidx)
#define DHD_DEV_LKUP(dev)	(((dhd_dev_priv_t *)DEV_PRIV(dev))->lkup)

#ifdef BCM_PKTFWD
#define DHD_DEV_PKTFWD_PRIV(dev) (&(((dhd_dev_priv_t *)DEV_PRIV(dev))->pktfwd_priv))
#endif /* BCM_PKTFWD */

/** Fetch the dhd_if object, given the interface index in the dhd. */
static inline dhd_if_t *dhd_get_ifp(dhd_pub_t *dhdp, uint32 ifidx);

#ifdef BCM_PKTFWD
/* Returns pointer to the pktfwd priv structure */
dhd_pktfwd_priv_t *
dhd_pktfwd_get_priv(struct net_device * dev)
{
	return DHD_DEV_PKTFWD_PRIV(dev);
} /* dhd_pktfwd_get_priv */

#endif /* BCM_PKTFWD */

/** Clear the dhd net_device's private structure. */
static inline void
dhd_dev_priv_clear(struct net_device * dev)
{
	dhd_dev_priv_t * dev_priv;
#ifdef BCM_PKTFWD
	dhd_pktfwd_priv_t *pktfwd_priv;
#endif /* BCM_PKTFWD */

	ASSERT(dev != (struct net_device *)NULL);
	dev_priv = DHD_DEV_PRIV(dev);
	dev_priv->dhd = (dhd_info_t *)NULL;
	dev_priv->ifp = (dhd_if_t *)NULL;
	dev_priv->ifidx = DHD_BAD_IF;
	dev_priv->lkup = (void *)NULL;

#ifdef BCM_PKTFWD
	pktfwd_priv = DHD_DEV_PKTFWD_PRIV(dev);
	pktfwd_priv->radio_unit = DHD_BAD_IF;
	pktfwd_priv->ifidx = DHD_BAD_IF;
	pktfwd_priv->d3fwd_wlif = NULL;
	pktfwd_priv->ifp = NULL;
#endif /* BCM_PKTFWD */

}

/** Setup the dhd net_device's private structure. */
static inline void
dhd_dev_priv_save(struct net_device * dev, dhd_info_t * dhd, dhd_if_t * ifp,
                  int ifidx)
{
	dhd_dev_priv_t * dev_priv;
#ifdef BCM_PKTFWD
	dhd_pktfwd_priv_t *pktfwd_priv;
#endif /* BCM_PKTFWD */

	ASSERT(dev != (struct net_device *)NULL);
	dev_priv = DHD_DEV_PRIV(dev);
	dev_priv->dhd = dhd;
	dev_priv->ifp = ifp;
	dev_priv->ifidx = ifidx;

#ifdef BCM_PKTFWD
	pktfwd_priv = DHD_DEV_PKTFWD_PRIV(dev);
	pktfwd_priv->radio_unit = dhd->pub.unit;
	pktfwd_priv->ifidx = ifidx;
	pktfwd_priv->d3fwd_wlif = ifp->d3fwd_wlif;
	pktfwd_priv->ifp = ifp;
#endif /* BCM_PKTFWD */
}

/* Return interface pointer */
static inline dhd_if_t *
dhd_get_ifp(dhd_pub_t *dhdp, uint32 ifidx)
{
	ASSERT(ifidx < DHD_MAX_IFS);

	if (ifidx >= DHD_MAX_IFS)
		return NULL;

	return dhdp->info->iflist[ifidx];
}
#ifdef PCIE_FULL_DONGLE

/** Dummy objects are defined with state representing bad|down.
 * Performance gains from reducing branch conditionals, instruction parallelism,
 * dual issue, reducing load shadows, avail of larger pipelines.
 * Use DHD_XXX_NULL instead of (dhd_xxx_t *)NULL, whenever an object pointer
 * is accessed via the dhd_sta_t.
 */

/* Dummy dhd_info object */
dhd_info_t dhd_info_null = {
	.pub = {
	         .info = &dhd_info_null,
#ifdef DHDTCPACK_SUPPRESS
	         .tcpack_sup_mode = TCPACK_SUP_REPLACE,
#endif /* DHDTCPACK_SUPPRESS */
#if defined(BCM_ROUTER_DHD) || defined(TRAFFIC_MGMT_DWM)
	         .dhd_tm_dwm_tbl = { .dhd_dwm_enabled = TRUE },
#endif
	         .up = FALSE,
	         .busstate = DHD_BUS_DOWN
	}
};

#define DHD_INFO_NULL (&dhd_info_null)
#define DHD_PUB_NULL  (&dhd_info_null.pub)

/* Dummy netdevice object */
struct net_device dhd_net_dev_null = {
	.reg_state = NETREG_UNREGISTERED
};
#define DHD_NET_DEV_NULL (&dhd_net_dev_null)

/* Dummy dhd_if object */
dhd_if_t dhd_if_null = {
#ifdef WMF
	.wmf = { .wmf_enable = TRUE, .wmf_bss_enab_val = TRUE },
#endif
	.info = DHD_INFO_NULL,
	.net = DHD_NET_DEV_NULL,
	.idx = DHD_BAD_IF,
	.wdsidx = ID8_INVALID
};
#define DHD_IF_NULL  (&dhd_if_null)

#define DHD_STA_NULL ((dhd_sta_t *)NULL)

/** Interface STA list management. */

/** Alloc/Free a dhd_sta object from the dhd instances' sta_pool. */
static void dhd_sta_free(dhd_pub_t *pub, dhd_sta_t *sta);
static dhd_sta_t * dhd_sta_alloc(dhd_pub_t * dhdp);

/* Delete a dhd_sta or flush all dhd_sta in an interface's sta_list. */
static void dhd_if_del_sta_list(dhd_if_t * ifp);

/* Construct/Destruct a sta pool. */
static int dhd_sta_pool_init(dhd_pub_t *dhdp, int max_sta);
static void dhd_sta_pool_fini(dhd_pub_t *dhdp, int max_sta);
/* Clear the pool of dhd_sta_t objects for built-in type driver */
static void dhd_sta_pool_clear(dhd_pub_t *dhdp, int max_sta);

#ifdef DHD_PSTA
dhd_if_t *
dhd_get_ifp_by_mac(dhd_pub_t *dhdp, uint8 *mac)
{
	int i;

	for (i = 0; i < DHD_MAX_IFS; i++)
	{
		if (dhdp->info->iflist[i] &&
			!memcmp(dhdp->info->iflist[i]->mac_addr, mac, ETHER_ADDR_LEN)) {
			return dhdp->info->iflist[i];
		}
	}

	return NULL;
}
#endif /* DHD_PSTA */

/* Given a WDS peer MAC addr, get interface pointer of corresponding WDS interface */
static dhd_if_t *
dhd_get_wds_ifp_by_mac(dhd_pub_t *dhdp, uint8 *mac)
{
	int i;
	dhd_if_t *ifp;

	for (i = 0; i < DHD_MAX_IFS; i++)
	{
		ifp = dhdp->info->iflist[i];
		if (ifp && ifp->wds_role == WLC_E_IF_FLAGS_WDS_AP &&
			!memcmp(ifp->peer_wds_mac_addr, mac, ETHER_ADDR_LEN)) {
			return ifp;
		}
	}

	return NULL;
}

/** Reset a dhd_sta object and free into the dhd pool. */
static void
dhd_sta_free(dhd_pub_t * dhdp, dhd_sta_t * sta)
{
	int prio;

	ASSERT((sta != DHD_STA_NULL) && (sta->idx != ID16_INVALID));

	ASSERT((dhdp->staid_allocator != NULL) && (dhdp->sta_pool != NULL));

	/*
	 * Flush and free all packets in all flowring's queues belonging to sta.
	 * Packets in flow ring will be flushed later.
	 */
	for (prio = 0; prio < (int)NUMPRIO; prio++) {
		uint16 flowid = sta->flowid[prio];

		if (flowid != FLOWID_INVALID) {
			unsigned long flags;
			flow_queue_t * queue = dhd_flow_queue(dhdp, flowid);
			flow_ring_node_t * flow_ring_node;

#ifdef DHDTCPACK_SUPPRESS
			/* Clean tcp_ack_info_tbl in order to prevent access to flushed pkt,
			 * when there is a newly coming packet from network stack.
			 */
			dhd_tcpack_info_tbl_clean(dhdp);
#endif /* DHDTCPACK_SUPPRESS */

			flow_ring_node = dhd_flow_ring_node(dhdp, flowid);
			DHD_FLOWRING_LOCK(flow_ring_node->lock, flags);
			flow_ring_node->status = FLOW_RING_STATUS_STA_FREEING;

			if (!DHD_FLOW_QUEUE_EMPTY(queue)) {
				void * pkt;
				while ((pkt = dhd_flow_queue_dequeue(dhdp, queue)) != NULL) {
					PKTFREE(dhdp->osh, pkt, TRUE);
				}
			}

			DHD_FLOWRING_UNLOCK(flow_ring_node->lock, flags);
			ASSERT(DHD_FLOW_QUEUE_EMPTY(queue));
		}

		sta->flowid[prio] = FLOWID_INVALID;
	}

	id16_map_free(dhdp->staid_allocator, sta->idx);
	DHD_CUMM_CTR_INIT(&sta->cumm_ctr);
	sta->ifp = DHD_IF_NULL; /* dummy dhd_if object */
	sta->ifidx = DHD_BAD_IF;
	bzero(sta->ea.octet, ETHER_ADDR_LEN);
	INIT_LIST_HEAD(&sta->list);
	sta->idx = ID16_INVALID; /* implying free */
}

/** Allocate a dhd_sta object from the dhd pool. */
static dhd_sta_t *
dhd_sta_alloc(dhd_pub_t * dhdp)
{
	uint16 idx;
	dhd_sta_t * sta;
	dhd_sta_pool_t * sta_pool;

	ASSERT((dhdp->staid_allocator != NULL) && (dhdp->sta_pool != NULL));

	idx = id16_map_alloc(dhdp->staid_allocator);
	if (idx == ID16_INVALID) {
		DHD_ERROR(("%s: cannot get free staid\n", __FUNCTION__));
		return DHD_STA_NULL;
	}

	sta_pool = (dhd_sta_pool_t *)(dhdp->sta_pool);
	sta = &sta_pool[idx];

	ASSERT((sta->idx == ID16_INVALID) &&
	       (sta->ifp == DHD_IF_NULL) && (sta->ifidx == DHD_BAD_IF));

	DHD_CUMM_CTR_INIT(&sta->cumm_ctr);

	sta->idx = idx; /* implying allocated */

	return sta;
}

/** Delete all STAs in an interface's STA list. */
static void
dhd_if_del_sta_list(dhd_if_t *ifp)
{
	dhd_sta_t *sta, *next;
	unsigned long flags;

	DHD_IF_STA_LIST_LOCK(ifp, flags);

	list_for_each_entry_safe(sta, next, &ifp->sta_list, list) {
		list_del(&sta->list);
		dhd_sta_free(&ifp->info->pub, sta);
	}

	DHD_IF_STA_LIST_UNLOCK(ifp, flags);

	return;
}

/** Construct a pool of dhd_sta_t objects to be used by interfaces. */
static int
dhd_sta_pool_init(dhd_pub_t *dhdp, int max_sta)
{
	int idx, prio, sta_pool_memsz;
	dhd_sta_t * sta;
	dhd_sta_pool_t * sta_pool;
	void * staid_allocator;

	ASSERT(dhdp != (dhd_pub_t *)NULL);
	ASSERT((dhdp->staid_allocator == NULL) && (dhdp->sta_pool == NULL));

	/* dhd_sta objects per radio are managed in a table. id#0 reserved. */
	staid_allocator = id16_map_init(dhdp->osh, max_sta, 1);
	if (staid_allocator == NULL) {
		DHD_ERROR(("%s: sta id allocator init failure\n", __FUNCTION__));
		return BCME_ERROR;
	}

	/* Pre allocate a pool of dhd_sta objects (one extra). */
	sta_pool_memsz = ((max_sta + 1) * sizeof(dhd_sta_t)); /* skip idx 0 */
	sta_pool = (dhd_sta_pool_t *)MALLOC(dhdp->osh, sta_pool_memsz);
	if (sta_pool == NULL) {
		DHD_ERROR(("%s: sta table alloc failure\n", __FUNCTION__));
		id16_map_fini(dhdp->osh, staid_allocator);
		return BCME_ERROR;
	}

	dhdp->sta_pool = sta_pool;
	dhdp->staid_allocator = staid_allocator;

	/* Initialize all sta(s) for the pre-allocated free pool. */
	bzero((uchar *)sta_pool, sta_pool_memsz);
	for (idx = max_sta; idx >= 1; idx--) { /* skip sta_pool[0] */
		sta = &sta_pool[idx];
		sta->idx = id16_map_alloc(staid_allocator);
		ASSERT(sta->idx <= max_sta);
	}

	/* Now place them into the pre-allocated free pool. */
	for (idx = 1; idx <= max_sta; idx++) {
		sta = &sta_pool[idx];
		for (prio = 0; prio < (int)NUMPRIO; prio++) {
			sta->flowid[prio] = FLOWID_INVALID; /* Flow rings do not exist */
		}
		dhd_sta_free(dhdp, sta);
	}

	return BCME_OK;
}

/** Destruct the pool of dhd_sta_t objects.
 * Caller must ensure that no STA objects are currently associated with an if.
 */
static void
dhd_sta_pool_fini(dhd_pub_t *dhdp, int max_sta)
{
	dhd_sta_pool_t * sta_pool = (dhd_sta_pool_t *)dhdp->sta_pool;

	if (sta_pool) {
		int idx;
		int sta_pool_memsz = ((max_sta + 1) * sizeof(dhd_sta_t));
		for (idx = 1; idx <= max_sta; idx++) {
			ASSERT(sta_pool[idx].ifp == DHD_IF_NULL);
			ASSERT(sta_pool[idx].idx == ID16_INVALID);
		}
		MFREE(dhdp->osh, dhdp->sta_pool, sta_pool_memsz);
		dhdp->sta_pool = NULL;
	}

	id16_map_fini(dhdp->osh, dhdp->staid_allocator);
	dhdp->staid_allocator = NULL;
}

/* Clear the pool of dhd_sta_t objects for built-in type driver */
static void
dhd_sta_pool_clear(dhd_pub_t *dhdp, int max_sta)
{
	int idx, prio, sta_pool_memsz;
	dhd_sta_t * sta;
	dhd_sta_pool_t * sta_pool;
	void *staid_allocator;

	if (!dhdp) {
		DHD_ERROR(("%s: dhdp is NULL\n", __FUNCTION__));
		return;
	}

	sta_pool = (dhd_sta_pool_t *)dhdp->sta_pool;
	staid_allocator = dhdp->staid_allocator;

	if (!sta_pool) {
		DHD_ERROR(("%s: sta_pool is NULL\n", __FUNCTION__));
		return;
	}

	if (!staid_allocator) {
		DHD_ERROR(("%s: staid_allocator is NULL\n", __FUNCTION__));
		return;
	}

	/* clear free pool */
	sta_pool_memsz = ((max_sta + 1) * sizeof(dhd_sta_t));
	bzero((uchar *)sta_pool, sta_pool_memsz);

	/* dhd_sta objects per radio are managed in a table. id#0 reserved. */
	id16_map_clear(staid_allocator, max_sta, 1);

	/* Initialize all sta(s) for the pre-allocated free pool. */
	for (idx = max_sta; idx >= 1; idx--) { /* skip sta_pool[0] */
		sta = &sta_pool[idx];
		sta->idx = id16_map_alloc(staid_allocator);
		ASSERT(sta->idx <= max_sta);
	}
	/* Now place them into the pre-allocated free pool. */
	for (idx = 1; idx <= max_sta; idx++) {
		sta = &sta_pool[idx];
		for (prio = 0; prio < (int)NUMPRIO; prio++) {
			sta->flowid[prio] = FLOWID_INVALID; /* Flow rings do not exist */
		}
		dhd_sta_free(dhdp, sta);
	}
}

/** Find STA with MAC address ea in an interface's STA list. */
dhd_sta_t *
dhd_find_sta(void *pub, int ifidx, void *ea)
{
	dhd_sta_t *sta;
	dhd_if_t *ifp;
	unsigned long flags;

	ASSERT(ea != NULL);
	ifp = dhd_get_ifp((dhd_pub_t *)pub, ifidx);
	if (ifp == NULL)
		return DHD_STA_NULL;

	DHD_IF_STA_LIST_LOCK(ifp, flags);

	list_for_each_entry(sta, &ifp->sta_list, list) {
		if (!memcmp(sta->ea.octet, ea, ETHER_ADDR_LEN)) {
			DHD_IF_STA_LIST_UNLOCK(ifp, flags);
			return sta;
		}
	}

	DHD_IF_STA_LIST_UNLOCK(ifp, flags);

	return DHD_STA_NULL;
}

/** Add STA into the interface's STA list. */
dhd_sta_t *
dhd_add_sta(void *pub, int ifidx, void *ea)
{
	dhd_sta_t *sta;
	dhd_if_t *ifp;
	unsigned long flags;

	ASSERT(ea != NULL);
	ifp = dhd_get_ifp((dhd_pub_t *)pub, ifidx);
	if (ifp == NULL)
		return DHD_STA_NULL;

	if (!memcmp(ifp->net->dev_addr, ea, ETHER_ADDR_LEN)) {
		DHD_ERROR(("%s: %s: Serious FAILURE, receive own MAC %pM !!\n",
			ifp->name, __FUNCTION__, ea));
		return DHD_STA_NULL;
	}

	sta = dhd_sta_alloc((dhd_pub_t *)pub);
	if (sta == DHD_STA_NULL) {
		DHD_ERROR(("%s: %s: Alloc failed\n", ifp->name, __FUNCTION__));
		return DHD_STA_NULL;
	}

	memcpy(sta->ea.octet, ea, ETHER_ADDR_LEN);

	/* link the sta and the dhd interface */
	sta->ifp = ifp;
	sta->ifidx = ifidx;
#ifdef DHD_WMF
	sta->psta_prim = NULL;
#endif

	INIT_LIST_HEAD(&sta->list);

	DHD_IF_STA_LIST_LOCK(ifp, flags);
	list_add_tail(&sta->list, &ifp->sta_list);
	DHD_IF_STA_LIST_UNLOCK(ifp, flags);

	return sta;
}

/*
 * This function will do following:
 * 1. Remove DWDS station's DA from AP's sta list.
 * 2. Remove all flowring created earlier. New flowrings will be created in the interface
 *	index associated with wds0.0.1. All future communication to DWDS repeater will be done
 *	through wds0.0.1 interface.
 * 3. Destroy all flows associated with DWDS STA in Flowcache/PKTFWD.
 */
inline static void
dhd_update_dwds_sta_record(dhd_pub_t *dhd_pub, uint8 ifidx)
{
	int idx, ret;
	dhd_sta_t *sta = NULL;
	struct ether_addr ea;
#ifdef BCM_BLOG
	wl_event_msg_t msg;
#endif /* BCM_BLOG */

	ret = dhd_wl_ioctl_cmd(dhd_pub, WLC_WDS_GET_REMOTE_HWADDR, (void *)&ea,
			ETHER_ADDR_LEN, FALSE, ifidx);
	if (ret < 0) {
		DHD_ERROR(("%s: can't get WDS remote MAC address , error=%d\n",	__FUNCTION__, ret));
		return;
	}

	for (idx = 0; idx < DHD_MAX_IFS; idx++) {
		if ((sta = dhd_find_sta(dhd_pub, idx, &ea))) {
			dhd_flow_rings_delete_for_peer(dhd_pub, idx, (char *)&ea);
			dhd_del_sta(dhd_pub, idx, &ea);

#ifdef BCM_BLOG
			bzero(&msg, sizeof(wl_event_msg_t));
			msg.event_type = -1;	/* Dummy event */
			memcpy(&msg.addr, &ea, sizeof(struct ether_addr));
			memcpy(msg.ifname, dhd_ifname(dhd_pub, idx), BCM_MSG_IFNAME_MAX);
			dhd_handle_blog_disconnect_event(dhd_pub, &msg);
#endif /* BCM_BLOG */
			break;
		}
	}
} /* dhd_update_dwds_sta_record() */

#ifdef BCM_PKTFWD_DWDS
/*
 * To save wds peer sta idx when wds interface is created.
 */
void
dhd_alloc_dwds_idx(void *dhdp, int ifidx)
{
	uint16 idx;
	dhd_if_t *ifp;

	ASSERT(((dhd_pub_t *)dhdp)->staid_allocator != NULL);

	/* save sta idx of wds peer to ifp for wds use */
	ifp = dhd_get_ifp((dhd_pub_t *)dhdp, ifidx);
	if ((ifp != NULL) && (ifp->wdsidx == ID8_INVALID)) {
		/* steal one idx from common allocator for wds use */
		idx = id16_map_alloc(((dhd_pub_t *)dhdp)->staid_allocator);
		if (idx == ID16_INVALID) {
			DHD_ERROR(("%s: %s: cannot get free staid\n",
				ifp->name, __FUNCTION__));
			return;
		}
		ifp->wdsidx = idx;
	}
}

/*
 * To free wds idx when wds interface is disconnected.
 */
void
dhd_free_dwds_idx(void *dhdp, int ifidx)
{
	dhd_if_t *ifp;

	ASSERT(((dhd_pub_t *)dhdp)->staid_allocator != NULL);

	/* free idx used by wds */
	ifp = dhd_get_ifp((dhd_pub_t *)dhdp, ifidx);
	if ((ifp != NULL) && (ifp->wdsidx != ID8_INVALID)) {
		id16_map_free(((dhd_pub_t *)dhdp)->staid_allocator, ifp->wdsidx);
		ifp->wdsidx = ID8_INVALID;
	}
}
#endif /* BCM_PKTFWD_DWDS */

/** Delete STA from the interface's STA list. */
void
dhd_del_sta(void *pub, int ifidx, void *ea)
{
	dhd_sta_t *sta, *next;
	dhd_if_t *ifp;
	unsigned long flags;

	ASSERT(ea != NULL);
	ifp = dhd_get_ifp((dhd_pub_t *)pub, ifidx);
	if (ifp == NULL)
		return;

	DHD_IF_STA_LIST_LOCK(ifp, flags);

	list_for_each_entry_safe(sta, next, &ifp->sta_list, list) {
		if (!memcmp(sta->ea.octet, ea, ETHER_ADDR_LEN)) {
			list_del(&sta->list);
#if defined(BCM_NBUFF_WLMCAST) && defined(DHD_WMF)
			{
			/* if WMF enbaled, also need to notify EMF module, sta is leaving in case
			 * igmp snooping leave is not sent, EMF module also has a timer, but it is
			 * bigger than four mintes, it is too long when there is continous mutlicast
			 * traffic on
			 */
			if (ifp->wmf.wmf_enable && dhd_wmf_sta_del(&(ifp->wmf), sta) != BCME_OK)
					DHD_ERROR((" EMFC remove STA failure\r\n"));
			}
#endif

			dhd_sta_free(&ifp->info->pub, sta);
		}
	}

	DHD_IF_STA_LIST_UNLOCK(ifp, flags);

#ifdef DHD_L2_FILTER
	if (ifp->parp_enable) {
		/* clear Proxy ARP cache of specific Ethernet Address */
		bcm_l2_filter_arp_table_update(((dhd_pub_t*)pub)->osh, ifp->phnd_arp_table, FALSE,
			ea, FALSE, ((dhd_pub_t*)pub)->tickcnt);
	}
#endif /* DHD_L2_FILTER */

	return;
}

/* Add STA if it doesn't exist. Not reentrant.
 * Flush the old entry when 'flush' is passed as TRUE
 */
dhd_sta_t*
dhd_findadd_sta(void *pub, int ifidx, void *ea, bool flush)
{
	dhd_sta_t *sta = NULL;

	if ((sta = dhd_find_sta(pub, ifidx, ea))) {
		/* STA entry already exists in this interface and can NOT
		 * exist in other interfaces. So we shouldn't Delete/add
		 * this entry. This is to achieve ZPL.
		 */
		return sta;
	}

	BCM_REFERENCE(flush);
	/* Add entry */
	sta = dhd_add_sta(pub, ifidx, ea);

	return sta;
}

/* Returns idx from dhd_pub::sta_pool[] for a given mac */
uint32
dhd_if_get_staidx(void *pub, int ifidx, void *ea)
{
	dhd_sta_t *sta;
	dhd_if_t *ifp;
	unsigned long flags;
	uint32 staidx = ID16_INVALID;

	ASSERT(ea != NULL);

	ifp = dhd_get_ifp((dhd_pub_t *)pub, ifidx);
	if (ifp == NULL)
	    return ID16_INVALID;

#ifdef BCM_PKTFWD_DWDS
	if (DHD_IF_ROLE_WDS((dhd_pub_t *)pub, ifidx) || DHD_IF_ROLE_STA((dhd_pub_t *)pub, ifidx)) {
		/* always return the WDS peer for the sta idx */
		return (ifp->wdsidx != ID8_INVALID ? ifp->wdsidx : ID16_INVALID);
	}
#endif /* BCM_PKTFWD_DWDS */

	DHD_IF_STA_LIST_LOCK(ifp, flags);

	list_for_each_entry(sta, &ifp->sta_list, list) {
	    if (!memcmp(sta->ea.octet, ea, ETHER_ADDR_LEN)) {
		staidx = sta->idx;
		break;
	    }
	}

	DHD_IF_STA_LIST_UNLOCK(ifp, flags);

	return staidx;
}

#if defined(DHD_IGMP_UCQUERY) || defined(DHD_UCAST_UPNP)
#if !defined(BCM_GMAC3)
static struct list_head *
dhd_sta_list_snapshot(dhd_info_t *dhd, dhd_if_t *ifp, struct list_head *snapshot_list)
{
	unsigned long flags;
	dhd_sta_t *sta, *snapshot;

	INIT_LIST_HEAD(snapshot_list);

	DHD_IF_STA_LIST_LOCK(ifp, flags);

	list_for_each_entry(sta, &ifp->sta_list, list) {
		/* allocate one and add to snapshot */
		snapshot = (dhd_sta_t *)MALLOC(dhd->pub.osh, sizeof(dhd_sta_t));
		if (snapshot == NULL) {
			DHD_ERROR(("%s: Cannot allocate memory\n", __FUNCTION__));
			continue;
		}

		memcpy(snapshot->ea.octet, sta->ea.octet, ETHER_ADDR_LEN);

		INIT_LIST_HEAD(&snapshot->list);
		list_add_tail(&snapshot->list, snapshot_list);
	}

	DHD_IF_STA_LIST_UNLOCK(ifp, flags);

	return snapshot_list;
}

static void
dhd_sta_list_snapshot_free(dhd_info_t *dhd, struct list_head *snapshot_list)
{
	dhd_sta_t *sta, *next;

	list_for_each_entry_safe(sta, next, snapshot_list, list) {
		list_del(&sta->list);
		MFREE(dhd->pub.osh, sta, sizeof(dhd_sta_t));
	}
}

#endif /* !BCM_GMAC3 */
#endif /* DHD_IGMP_UCQUERY || DHD_UCAST_UPNP */

#else

static inline void dhd_if_del_sta_list(dhd_if_t *ifp) {}
static inline int dhd_sta_pool_init(dhd_pub_t *dhdp, int max_sta) { return BCME_OK; }
static inline void dhd_sta_pool_fini(dhd_pub_t *dhdp, int max_sta) {}
static inline void dhd_sta_pool_clear(dhd_pub_t *dhdp, int max_sta) {}
dhd_sta_t *dhd_findadd_sta(void *pub, int ifidx, void *ea, bool flush) { return NULL; }
dhd_sta_t *dhd_find_sta(void *pub, int ifidx, void *ea) { return NULL; }
uint32 dhd_if_get_staidx(void *pub, int ifidx, void *ea) { return ID16_INVALID; }
void dhd_del_sta(void *pub, int ifidx, void *ea) {}

#endif /* PCIE_FULL_DONGLE */

#ifdef BCM_ROUTER_DHD
/** Bind a flowid to the dhd_sta's flowid table. */
void
dhd_add_flowid(dhd_pub_t * dhdp, int ifidx, uint8 ac_prio, void * ea,
                uint16 flowid)
{
	int prio;
	dhd_if_t * ifp;
	dhd_sta_t * sta;
	flow_queue_t * queue;

	ASSERT((dhdp != (dhd_pub_t *)NULL) && (ea != NULL));

	/* Fetch the dhd_if object given the if index */
	ifp = dhd_get_ifp(dhdp, ifidx);
	if (ifp == (dhd_if_t *)NULL) /* ifp fetched from dhdp iflist[] */
		return;

	if (DHD_IF_ROLE_WDS(dhdp, ifidx) ||
#ifdef DHD_WET
		WET_ENABLED(dhdp) ||
#endif /* DHD_WET */
		(DHD_IF_ROLE_STA(dhdp, ifidx) &&
#ifdef DHD_PSTA
		(DHD_MODE_PSTA_DISABLED == dhd_get_psta_mode(dhdp)) &&
#endif /* DHD_PSTA */
		TRUE)) {
		queue = dhd_flow_queue(dhdp, flowid);
		dhd_flow_ring_config_thresholds(dhdp, flowid,
			dhd_queue_budget, queue->max, DHD_FLOW_QUEUE_CLEN_PTR(queue),
			dhd_if_threshold, (void *)&ifp->cumm_ctr);
		return;
	} else if ((sta = dhd_find_sta(dhdp, ifidx, ea)) == DHD_STA_NULL) {
		/* Fetch the station with a matching Mac address. */
		/* Update queue's grandparent cummulative length threshold */
		if (ETHER_ISMULTI((char *)ea)) {
			queue = dhd_flow_queue(dhdp, flowid);
			if (ifidx != 0 && DHD_IF_ROLE_STA(dhdp, ifidx)) {
				/* Use default dhdp->cumm_ctr and dhdp->l2cumm_ctr,
				 * in PSTA mode the ifp will be deleted but we don't delete
				 * the PSTA flowring.
				 */
				dhd_flow_ring_config_thresholds(dhdp, flowid,
					queue->max, queue->max, DHD_FLOW_QUEUE_CLEN_PTR(queue),
					dhd_if_threshold, DHD_FLOW_QUEUE_L2CLEN_PTR(queue));
			}
			else if (DHD_FLOW_QUEUE_L2CLEN_PTR(queue) != (void *)&ifp->cumm_ctr) {
				dhd_flow_ring_config_thresholds(dhdp, flowid,
					queue->max, queue->max, DHD_FLOW_QUEUE_CLEN_PTR(queue),
					dhd_if_threshold, (void *)&ifp->cumm_ctr);
			}
		}

		return;
	}

	/* Set queue's min budget and queue's parent cummulative length threshold */
	dhd_flow_ring_config_thresholds(dhdp, flowid, dhd_queue_budget,
	                                dhd_sta_threshold, (void *)&sta->cumm_ctr,
	                                dhd_if_threshold, (void *)&ifp->cumm_ctr);

	/* Populate the flowid into the stations flowid table, for all packet
	 * priorities that would match the given flow's ac priority.
	 */
	for (prio = 0; prio < (int)NUMPRIO; prio++) {
		if (dhdp->flow_prio_map[prio] == ac_prio) {
			/* flowring shared for all these pkt prio */
			sta->flowid[prio] = flowid;
		}
	}
}

/** Unbind a flowid to the sta's flowid table. */
void
dhd_del_flowid(dhd_pub_t * dhdp, int ifidx, uint16 flowid)
{
	int prio;
	dhd_if_t * ifp;
	dhd_sta_t * sta;
	unsigned long flags;

#ifdef DHD_LBR_AGGR_BCM_ROUTER
	dhd_aggr_del_aggregator(dhdp, DHD_AGGR_TYPE_LBR, flowid);
#endif

	/* Fetch the dhd_if object given the if index */
	ifp = dhd_get_ifp(dhdp, ifidx);
	if (ifp == (dhd_if_t *)NULL) /* ifp fetched from dhdp iflist[] */
		return;

	/* Walk all stations and delete clear any station's reference to flowid */
	DHD_IF_STA_LIST_LOCK(ifp, flags);

	list_for_each_entry(sta, &ifp->sta_list, list) {
		for (prio = 0; prio < (int)NUMPRIO; prio++) {
			if (sta->flowid[prio] == flowid) {
				sta->flowid[prio] = FLOWID_INVALID;
#ifdef BCM_PKTFWD
				if (PKTFWD_ENABLED(dhdp)) {
					/* Reset pktfwd mapping */
					dhd_pktfwd_reset_keymap(dhdp->unit,
						DHD_STAIDX2LUTID(sta->idx),
						flowid, prio);
				}
#endif /* BCM_PKTFWD */
			}
		}
	}

	DHD_IF_STA_LIST_UNLOCK(ifp, flags);
}
#endif /* BCM_ROUTER_DHD */

#if defined(DHD_LB)

#if defined(DHD_LB_TXC) || defined(DHD_LB_RXC)
/**
 * dhd_tasklet_schedule - Function that runs in IPI context of the destination
 * CPU and schedules a tasklet.
 * @tasklet: opaque pointer to the tasklet
 */
static INLINE void
dhd_tasklet_schedule(void *tasklet)
{
	tasklet_schedule((struct tasklet_struct *)tasklet);
}

/**
 * dhd_tasklet_schedule_on - Executes the passed takslet in a given CPU
 * @tasklet: tasklet to be scheduled
 * @on_cpu: cpu core id
 *
 * If the requested cpu is online, then an IPI is sent to this cpu via the
 * smp_call_function_single with no wait and the tasklet_schedule function
 * will be invoked to schedule the specified tasklet on the requested CPU.
 */
static void
dhd_tasklet_schedule_on(struct tasklet_struct *tasklet, int on_cpu)
{
	const int wait = 0;
	smp_call_function_single(on_cpu,
		dhd_tasklet_schedule, (void *)tasklet, wait);
}
#endif /* DHD_LB_TXC || DHD_LB_RXC */

#if defined(DHD_LB_TXC)
/**
 * dhd_lb_tx_compl_dispatch - load balance by dispatching the tx_compl_tasklet
 * on another cpu. The tx_compl_tasklet will take care of DMA unmapping and
 * freeing the packets placed in the tx_compl workq
 */
void
dhd_lb_tx_compl_dispatch(dhd_pub_t *dhdp)
{
	dhd_info_t *dhd = dhdp->info;
	int curr_cpu, on_cpu;

	DHD_LB_STATS_INCR(dhd->txc_sched_cnt);
	/*
	 * If the destination CPU is NOT online or is same as current CPU
	 * no need to schedule the work
	 */
	curr_cpu = get_cpu();
	put_cpu();

	on_cpu = atomic_read(&dhd->tx_compl_cpu);

	if ((on_cpu == curr_cpu) || (!cpu_online(on_cpu))) {
		dhd_tasklet_schedule(&dhd->tx_compl_tasklet);
	} else {
		schedule_work(&dhd->tx_compl_dispatcher_work);
	}
}

static void
dhd_tx_compl_dispatcher_fn(struct work_struct * work)
{
	dhd_info_t *dhd = container_of(work, dhd_info_t, tx_compl_dispatcher_work);
	int cpu;

	get_online_cpus();
	cpu = atomic_read(&dhd->tx_compl_cpu);
	if (!cpu_online(cpu))
		dhd_tasklet_schedule(&dhd->tx_compl_tasklet);
	else
		dhd_tasklet_schedule_on(&dhd->tx_compl_tasklet, cpu);
	put_online_cpus();
}

#endif /* DHD_LB_TXC */

#if defined(DHD_LB_RXC)
/**
 * dhd_lb_rx_compl_dispatch - load balance by dispatching the rx_compl_tasklet
 * on another cpu. The rx_compl_tasklet will take care of reposting rx buffers
 * in the H2D RxBuffer Post common ring, by using the recycled pktids that were
 * placed in the rx_compl workq.
 *
 * @dhdp: pointer to dhd_pub object
 */
void
dhd_lb_rx_compl_dispatch(dhd_pub_t *dhdp)
{
	dhd_info_t *dhd = dhdp->info;
	int curr_cpu, on_cpu;

	DHD_LB_STATS_INCR(dhd->rxc_sched_cnt);
	/*
	 * If the destination CPU is NOT online or is same as current CPU
	 * no need to schedule the work
	 */
	curr_cpu = get_cpu();
	put_cpu();

	on_cpu = atomic_read(&dhd->rx_compl_cpu);

	if ((on_cpu == curr_cpu) || (!cpu_online(on_cpu))) {
		dhd_tasklet_schedule(&dhd->rx_compl_tasklet);
	} else {
		schedule_work(&dhd->rx_compl_dispatcher_work);
	}
}

static void
dhd_rx_compl_dispatcher_fn(struct work_struct * work)
{
	dhd_info_t *dhd = container_of(work, dhd_info_t, rx_compl_dispatcher_work);
	int cpu;

	get_online_cpus();
	cpu = atomic_read(&dhd->tx_compl_cpu);
	if (!cpu_online(cpu))
		dhd_tasklet_schedule(&dhd->rx_compl_tasklet);
	else
		dhd_tasklet_schedule_on(&dhd->rx_compl_tasklet, cpu);
	put_online_cpus();
}

#endif /* DHD_LB_RXC */

#if defined(DHD_LB_RXP)
/**
 * dhd_napi_poll - Load balance napi poll function to process received
 * packets and send up the network stack using netif_receive_skb()
 *
 * @napi: napi object in which context this poll function is invoked
 * @budget: number of packets to be processed.
 *
 * Fetch the dhd_info given the rx_napi_struct. Move all packets from the
 * rx_napi_queue into a local rx_process_queue (lock and queue move and unlock).
 * Dequeue each packet from head of rx_process_queue, fetch the ifid from the
 * packet tag and sendup.
 */
static int
dhd_napi_poll(struct napi_struct *napi, int budget)
{
	int ifid;
	const int pkt_count = 1;
	const int chan = 0;
	struct sk_buff * skb;
	unsigned long flags;
	dhd_info_t *dhd;
	int processed = 0;
	struct sk_buff_head rx_process_queue;

	dhd = container_of(napi, dhd_info_t, rx_napi_struct);
	DHD_INFO(("%s napi_queue<%d> budget<%d>\n",
		__FUNCTION__, skb_queue_len(&dhd->rx_napi_queue), budget));

	__skb_queue_head_init(&rx_process_queue);

	/* extract the entire rx_napi_queue into local rx_process_queue */
	spin_lock_irqsave(&dhd->rx_napi_queue.lock, flags);
	skb_queue_splice_tail_init(&dhd->rx_napi_queue, &rx_process_queue);
	spin_unlock_irqrestore(&dhd->rx_napi_queue.lock, flags);

	while ((skb = __skb_dequeue(&rx_process_queue)) != NULL) {
		OSL_PREFETCH(skb->data);

		ifid = DHD_PKTTAG_IFID((dhd_pkttag_fr_t *)PKTTAG(skb));

		DHD_INFO(("%s dhd_rx_frame pkt<%p> ifid<%d>\n",
			__FUNCTION__, skb, ifid));

		dhd_rx_frame(&dhd->pub, ifid, skb, pkt_count, chan);
		processed++;
	}

	DHD_LB_STATS_UPDATE_NAPI_HISTO(&dhd->pub, processed);

	DHD_INFO(("%s processed %d\n", __FUNCTION__, processed));
	napi_complete(napi);

	return budget - 1;
}

/**
 * dhd_napi_schedule - Place the napi struct into the current cpus softnet napi
 * poll list. This function may be invoked via the smp_call_function_single
 * from a remote CPU.
 *
 * This function will essentially invoke __raise_softirq_irqoff(NET_RX_SOFTIRQ)
 * after the napi_struct is added to the softnet data's poll_list
 *
 * @info: pointer to a dhd_info struct
 */
static void
dhd_napi_schedule(void *info)
{
	dhd_info_t *dhd = (dhd_info_t *)info;

	DHD_INFO(("%s rx_napi_struct<%p> on cpu<%d>\n",
		__FUNCTION__, &dhd->rx_napi_struct, atomic_read(&dhd->rx_napi_cpu)));

	/* add napi_struct to softnet data poll list and raise NET_RX_SOFTIRQ */
	if (napi_schedule_prep(&dhd->rx_napi_struct)) {
		__napi_schedule(&dhd->rx_napi_struct);
		DHD_LB_STATS_PERCPU_ARR_INCR(dhd->napi_percpu_run_cnt);
	}

	/*
	 * If the rx_napi_struct was already running, then we let it complete
	 * processing all its packets. The rx_napi_struct may only run on one
	 * core at a time, to avoid out-of-order handling.
	 */
}

/**
 * dhd_napi_schedule_on - API to schedule on a desired CPU core a NET_RX_SOFTIRQ
 * action after placing the dhd's rx_process napi object in the the remote CPU's
 * softnet data's poll_list.
 *
 * @dhd: dhd_info which has the rx_process napi object
 * @on_cpu: desired remote CPU id
 */
static INLINE int
dhd_napi_schedule_on(dhd_info_t *dhd, int on_cpu)
{
	int wait = 0; /* asynchronous IPI */

	DHD_INFO(("%s dhd<%p> napi<%p> on_cpu<%d>\n",
		__FUNCTION__, dhd, &dhd->rx_napi_struct, on_cpu));

	if (smp_call_function_single(on_cpu, dhd_napi_schedule, dhd, wait)) {
		DHD_ERROR(("%s smp_call_function_single on_cpu<%d> failed\n",
			__FUNCTION__, on_cpu));
	}

	DHD_LB_STATS_INCR(dhd->napi_sched_cnt);

	return 0;
}

/*
 * Call get_online_cpus/put_online_cpus around dhd_napi_schedule_on
 * Why should we do this?
 * The candidacy algorithm is run from the call back function
 * registered to CPU hotplug notifier. This call back happens from Worker
 * context. The dhd_napi_schedule_on is also from worker context.
 * Note that both of this can run on two different CPUs at the same time.
 * So we can possibly have a window where a given CPUn is being brought
 * down from CPUm while we try to run a function on CPUn.
 * To prevent this its better have the whole code to execute an SMP
 * function under get_online_cpus.
 * This function call ensures that hotplug mechanism does not kick-in
 * until we are done dealing with online CPUs
 * If the hotplug worker is already running, no worries because the
 * candidacy algo would then reflect the same in dhd->rx_napi_cpu.
 *
 * The below mentioned code structure is proposed in
 * https://www.kernel.org/doc/Documentation/cpu-hotplug.txt
 * for the question
 * Q: I need to ensure that a particular cpu is not removed when there is some
 *    work specific to this cpu is in progress
 *
 * According to the documentation calling get_online_cpus is NOT required, if
 * we are running from tasklet context. Since dhd_rx_napi_dispatcher_fn can
 * run from Work Queue context we have to call these functions
 */
static void
dhd_rx_napi_dispatcher_fn(struct work_struct * work)
{
	dhd_info_t *dhd = container_of(work, dhd_info_t, rx_napi_dispatcher_work);
	int cpu;

	get_online_cpus();
	cpu = atomic_read(&dhd->rx_napi_cpu);
	if (!cpu_online(cpu))
		dhd_napi_schedule(dhd);
	else
		dhd_napi_schedule_on(dhd, cpu);
	put_online_cpus();
}

/**
 * dhd_lb_rx_napi_dispatch - load balance by dispatching the rx_napi_struct
 * to run on another CPU. The rx_napi_struct's poll function will retrieve all
 * the packets enqueued into the rx_napi_queue and sendup.
 * The producer's rx packet queue is appended to the rx_napi_queue before
 * dispatching the rx_napi_struct.
 */
void
dhd_lb_rx_napi_dispatch(dhd_pub_t *dhdp)
{
	unsigned long flags;
	dhd_info_t *dhd = dhdp->info;
	int curr_cpu;
	int on_cpu;

	DHD_INFO(("%s append napi_queue<%d> pend_queue<%d>\n", __FUNCTION__,
		skb_queue_len(&dhd->rx_napi_queue), skb_queue_len(&dhd->rx_pend_queue)));

	/* append the producer's queue of packets to the napi's rx process queue */
	spin_lock_irqsave(&dhd->rx_napi_queue.lock, flags);
	skb_queue_splice_tail_init(&dhd->rx_pend_queue, &dhd->rx_napi_queue);
	spin_unlock_irqrestore(&dhd->rx_napi_queue.lock, flags);

	/*
	 * If the destination CPU is NOT online or is same as current CPU
	 * no need to schedule the work
	 */
	curr_cpu = get_cpu();
	put_cpu();

	on_cpu = atomic_read(&dhd->rx_napi_cpu);

	if ((on_cpu == curr_cpu) || (!cpu_online(on_cpu))) {
		dhd_napi_schedule(dhd);
	} else {
		schedule_work(&dhd->rx_napi_dispatcher_work);
	}
}

/**
 * dhd_lb_rx_pkt_enqueue - Enqueue the packet into the producer's queue
 */
void
dhd_lb_rx_pkt_enqueue(dhd_pub_t *dhdp, void *pkt, int ifidx)
{
	dhd_info_t *dhd = dhdp->info;

	DHD_INFO(("%s: %s enqueue pkt<%p> ifidx<%d> pend_queue<%d>\n",
		dhd_ifname(dhdp, ifidx), __FUNCTION__, pkt, ifidx,
		skb_queue_len(&dhd->rx_pend_queue)));
	DHD_PKTTAG_SET_IFID((dhd_pkttag_fr_t *)PKTTAG(pkt), ifidx);
	__skb_queue_tail(&dhd->rx_pend_queue, pkt);
}
#endif /* DHD_LB_RXP */

#endif /* DHD_LB */

/** Returns dhd iflist index corresponding the the bssidx provided by apps */
int
dhd_bssidx2idx(dhd_pub_t *dhdp, uint32 bssidx)
{
	dhd_if_t *ifp;
	dhd_info_t *dhd = dhdp->info;
	int i;

	ASSERT(bssidx < DHD_MAX_IFS);
	ASSERT(dhdp);

	for (i = 0; i < DHD_MAX_IFS; i++) {
		ifp = dhd->iflist[i];
		if (ifp && (ifp->bssidx == bssidx)) {
			DHD_TRACE(("Index manipulated for %s from %d to %d\n",
				ifp->name, bssidx, i));
			break;
		}
	}

	return i;
}

static inline int
dhd_rxf_enqueue(dhd_pub_t *dhdp, void* skb)
{
	uint32 store_idx;
	uint32 sent_idx;

	if (!skb) {
		DHD_ERROR(("dhd_rxf_enqueue: NULL skb!!!\n"));
		return BCME_ERROR;
	}

	dhd_os_rxflock(dhdp);
	store_idx = dhdp->store_idx;
	sent_idx = dhdp->sent_idx;
	if (dhdp->skbbuf[store_idx] != NULL) {
		/* Make sure the previous packets are processed */
		dhd_os_rxfunlock(dhdp);
#ifdef RXF_DEQUEUE_ON_BUSY
		DHD_TRACE(("dhd_rxf_enqueue: pktbuf not consumed %p, store idx %d sent idx %d\n",
			skb, store_idx, sent_idx));
		return BCME_BUSY;
#else /* RXF_DEQUEUE_ON_BUSY */
		DHD_ERROR(("dhd_rxf_enqueue: pktbuf not consumed %p, store idx %d sent idx %d\n",
			skb, store_idx, sent_idx));
		/* removed msleep here, should use wait_event_timeout if we
		 * want to give rx frame thread a chance to run
		 */
#if defined(WAIT_DEQUEUE)
		OSL_SLEEP(1);
#endif
		return BCME_ERROR;
#endif /* RXF_DEQUEUE_ON_BUSY */
	}
	DHD_TRACE(("dhd_rxf_enqueue: Store SKB %p. idx %d -> %d\n",
		skb, store_idx, (store_idx + 1) & (MAXSKBPEND - 1)));
	dhdp->skbbuf[store_idx] = skb;
	dhdp->store_idx = (store_idx + 1) & (MAXSKBPEND - 1);
	dhd_os_rxfunlock(dhdp);

	return BCME_OK;
}

static inline void *
dhd_rxf_dequeue(dhd_pub_t *dhdp)
{
	uint32 store_idx;
	uint32 sent_idx;
	void *skb;

	dhd_os_rxflock(dhdp);

	store_idx = dhdp->store_idx;
	sent_idx = dhdp->sent_idx;
	skb = dhdp->skbbuf[sent_idx];

	if (skb == NULL) {
		dhd_os_rxfunlock(dhdp);
		DHD_ERROR(("dhd_rxf_dequeue: Dequeued packet is NULL, store idx %d sent idx %d\n",
			store_idx, sent_idx));
		return NULL;
	}

	dhdp->skbbuf[sent_idx] = NULL;
	dhdp->sent_idx = (sent_idx + 1) & (MAXSKBPEND - 1);

	DHD_TRACE(("dhd_rxf_dequeue: netif_rx_ni(%p), sent idx %d\n",
		skb, sent_idx));

	dhd_os_rxfunlock(dhdp);

	return skb;
}

#ifdef OEM_ANDROID
int
dhd_process_cid_mac(dhd_pub_t *dhdp, bool prepost)
{
#ifndef CUSTOMER_HW10
	dhd_info_t *dhd = (dhd_info_t *)dhdp->info;
#endif /* !CUSTOMER_HW10 */

	if (prepost) { /* pre process */
#ifdef CUSTOMER_HW4
#ifdef USE_CID_CHECK
		dhd_check_module_cid(dhdp);
#endif
#ifdef READ_MACADDR
		dhd_read_macaddr(dhd, &dhd->pub.mac);
#endif
#else
		dhd_read_macaddr(dhd);
#endif /* CUSTOMER_HW4 */
	} else { /* post process */
#ifdef CUSTOMER_HW4
#ifdef GET_MAC_FROM_OTP
		dhd_check_module_mac(dhdp, &dhd->pub.mac);
#endif
#ifdef WRITE_MACADDR
		dhd_write_macaddr(&dhd->pub.mac);
#endif
#else
		dhd_write_macaddr(&dhd->pub.mac);
#endif /* CUSTOMER_HW4 */
	}

	return 0;
}
#endif /* OEM_ANDROID */

void
dhd_set_packet_filter(dhd_pub_t *dhd)
{
#ifdef PKT_FILTER_SUPPORT
	int i;

	DHD_TRACE(("%s: enter\n", __FUNCTION__));
	if (dhd_pkt_filter_enable) {
		for (i = 0; i < dhd->wlcore->pktfilter_count; i++) {
			dhd_pktfilter_offload_set(dhd, dhd->wlcore->pktfilter[i]);
		}
	}
#endif /* PKT_FILTER_SUPPORT */
}

void
dhd_enable_packet_filter(int value, dhd_pub_t *dhd)
{
#ifdef PKT_FILTER_SUPPORT
	int i;

	DHD_TRACE(("%s: enter, value = %d\n", __FUNCTION__, value));
	/* 1 - Enable packet filter, only allow unicast packet to send up */
	/* 0 - Disable packet filter */
	if (dhd_pkt_filter_enable && (!value ||
	    (dhd_support_sta_mode(dhd) && !dhd->wlcore->dhcp_in_progress)))
	    {
		for (i = 0; i < dhd->wlcore->pktfilter_count; i++) {
#ifndef GAN_LITE_NAT_KEEPALIVE_FILTER
			if (value && (i == WL_ARP_FILTER_NUM) &&
					!_turn_on_arp_filter(dhd->wlcore, dhd->wlcore->op_mode)) {
				DHD_TRACE(("Do not turn on ARP white list pkt filter:"
							"val %d, cnt %d, op_mode 0x%x\n",
							value, i, dhd->wlcore->op_mode));
				continue;
			}
#endif /* !GAN_LITE_NAT_KEEPALIVE_FILTER */
			dhd_pktfilter_offload_enable(dhd, dhd->wlcore->pktfilter[i],
					value, dhd_master_mode);
		}
	}
#endif /* PKT_FILTER_SUPPORT */
}

static int
dhd_set_suspend(int value, dhd_pub_t *dhd)
{
#ifndef SUPPORT_PM2_ONLY
	int power_mode = PM_MAX;
#endif /* SUPPORT_PM2_ONLY */
	/* wl_pkt_filter_enable_t	enable_parm; */
	int bcn_li_dtim = 0; /* Default bcn_li_dtim in resume mode is 0 */
#ifdef OEM_ANDROID
#ifdef CUSTOM_BCN_TIMEOUT_IN_SUSPEND
	int bcn_timeout = 0;
#endif /* CUSTOM_BCN_TIMEOUT_IN_SUSPEND */
#ifdef CUSTOM_ROAM_TIME_THRESH_IN_SUSPEND
	int roam_time_thresh = 0;	/* (ms) */
#endif /* CUSTOM_ROAM_TIME_THRESH_IN_SUSPEND */
#ifndef ENABLE_FW_ROAM_SUSPEND
	uint roamvar = 1;
#endif /* ENABLE_FW_ROAM_SUSPEND */
#if defined(CUSTOMER_HW4) && defined(ENABLE_BCN_LI_BCN_WAKEUP)
	int bcn_li_bcn;
#endif /* CUSTOMER_HW4 && ENABLE_BCN_LI_BCN_WAKEUP */
	uint nd_ra_filter = 0;
#endif /* OEM_ANDROID */
	int ret = BCME_OK;
#if defined(PASS_ALL_MCAST_PKTS) && defined(CUSTOMER_HW4)
	struct dhd_info *dhdinfo;
	uint32 allmulti;
	uint i;
#endif /* PASS_ALL_MCAST_PKTS && CUSTOMER_HW4 */

#ifdef DYNAMIC_SWOOB_DURATION
#ifndef CUSTOM_INTR_WIDTH
#define CUSTOM_INTR_WIDTH 100
	int intr_width = 0;
#endif /* CUSTOM_INTR_WIDTH */
#endif /* DYNAMIC_SWOOB_DURATION */
	if (!dhd)
		return -ENODEV;

#if defined(PASS_ALL_MCAST_PKTS) && defined(CUSTOMER_HW4)
	dhdinfo = dhd->info;
#endif /* PASS_ALL_MCAST_PKTS && CUSTOMER_HW4 */

	DHD_TRACE(("%s: enter, value = %d in_suspend=%d\n",
		__FUNCTION__, value, dhd->in_suspend));

	dhd_suspend_lock(dhd);

#ifdef CUSTOM_SET_CPUCORE
	DHD_TRACE(("%s set cpucore(suspend%d)\n", __FUNCTION__, value));
	/* set specific cpucore */
	dhd_set_cpucore(dhd, TRUE);
#endif /* CUSTOM_SET_CPUCORE */
	if (dhd->up) {
		if (value && dhd->in_suspend) {
#ifdef PKT_FILTER_SUPPORT
				dhd->wlcore->early_suspended = 1;
#endif
				/* Kernel suspended */
				DHD_ERROR(("%s: force extra Suspend setting \n", __FUNCTION__));

#ifndef SUPPORT_PM2_ONLY
				ret = dhd_wl_ioctl_cmd(dhd, WLC_SET_PM, (char *)&power_mode,
				sizeof(power_mode), TRUE, 0);
				if (ret < 0) {
					DHD_ERROR(("%s: set WLC_SET_PM failed (%d)\n",
						__FUNCTION__, ret));
					goto exit;
				}
#endif /* SUPPORT_PM2_ONLY */

#ifdef PKT_FILTER_SUPPORT
				/* Enable packet filter, only allow unicast packet to send up */
				dhd_enable_packet_filter(1, dhd);
#endif
#if defined(PASS_ALL_MCAST_PKTS) && defined(CUSTOMER_HW4)
				allmulti = 0;
				for (i = 0; i < DHD_MAX_IFS; i++) {
					if (dhdinfo->iflist[i] && dhdinfo->iflist[i]->net)
						ret = dhd_iovar(dhd, i, "allmulti",
						(char *)&allmulti, sizeof(allmulti), NULL, 0, TRUE);
				}

				if (ret < 0) {
					DHD_ERROR(("%s: set allmulti failed (%d)\n",
						__FUNCTION__, ret));
					goto exit;
				}
#endif /* PASS_ALL_MCAST_PKTS && CUSTOMER_HW4 */

				/* If DTIM skip is set up as default, force it to wake
				 * each third DTIM for better power savings.  Note that
				 * one side effect is a chance to miss BC/MC packet.
				 */
#ifdef WLTDLS
				/* Do not set bcn_li_ditm on WFD mode */
				if (dhd->wlcore->tdls_mode) {
					bcn_li_dtim = 0;
				} else
#endif /* WLTDLS */
				bcn_li_dtim = dhd_get_suspend_bcn_li_dtim(dhd);
				ret = dhd_iovar(dhd, 0, "bcn_li_dtim", (char *)&bcn_li_dtim,
						sizeof(bcn_li_dtim), NULL, 0, TRUE);
				if (ret < 0) {
					DHD_ERROR(("%s: set dtim failed(%d)\n",
						__FUNCTION__, ret));
					goto exit;
				}

#ifdef OEM_ANDROID
#ifdef CUSTOM_BCN_TIMEOUT_IN_SUSPEND
				bcn_timeout = CUSTOM_BCN_TIMEOUT_IN_SUSPEND;
				ret = dhd_iovar(dhd, 0, "bcn_timeout", (char *)&bcn_timeout,
						sizeof(bcn_timeout), NULL, 0, TRUE);
				if (ret < 0) {
					DHD_ERROR(("%s: set bcn_timeout failed(%d)\n",
					__FUNCTION__, ret));
					goto exit;
				}
#endif /* CUSTOM_BCN_TIMEOUT_IN_SUSPEND */
#ifdef CUSTOM_ROAM_TIME_THRESH_IN_SUSPEND
				roam_time_thresh = CUSTOM_ROAM_TIME_THRESH_IN_SUSPEND;
				ret = dhd_iovar(dhd, 0, "roam_time_thresh",
					(char *)&roam_time_thresh, sizeof(roam_time_thresh),
					NULL, 0, TRUE);

				if (ret < 0) {
					DHD_ERROR(("%s: set roam_time_thresh failed (%d)\n",
						__FUNCTION__, ret));
					goto exit;
				}
#endif /* CUSTOM_ROAM_TIME_THRESH_IN_SUSPEND */
#ifndef ENABLE_FW_ROAM_SUSPEND
				/* Disable firmware roaming during suspend */
				ret = dhd_iovar(dhd, 0, "roam_off", (char *)&roamvar,
					sizeof(roamvar), NULL, 0, TRUE);
				if (ret < 0) {
					DHD_ERROR(("%s: set roam_off failed (%d)\n",
						__FUNCTION__, ret));
					goto exit;
				}
#endif /* ENABLE_FW_ROAM_SUSPEND */
#if defined(CUSTOMER_HW4) && defined(ENABLE_BCN_LI_BCN_WAKEUP)
				bcn_li_bcn = 0;
				ret = dhd_iovar(dhd, 0, "bcn_li_bcn", (char *)&bcn_li_bcn,
						sizeof(bcn_li_bcn), NULL, 0, TRUE);
				if (ret < 0) {
					DHD_ERROR(("%s: set bcn_li_bcn failed (%d)\n",
						__FUNCTION__, ret));
					goto exit;
				}
#endif /* CUSTOMER_HW4 && ENABLE_BCN_LI_BCN_WAKEUP */
				if (FW_SUPPORTED(dhd->wlcore, ndoe)) {
					/* enable IPv6 RA filter in  firmware during suspend */
					nd_ra_filter = 1;
					ret = dhd_iovar(dhd, 0, "nd_ra_filter_enable",
							(char *)&nd_ra_filter, sizeof(nd_ra_filter),
							NULL, 0, TRUE);
					if (ret < 0) {
						DHD_ERROR(("%s: failed to set nd_ra_filter (%d)\n",
							__FUNCTION__, ret));
						goto exit;
					}
				}
#ifdef DYNAMIC_SWOOB_DURATION
				intr_width = CUSTOM_INTR_WIDTH;
				ret = dhd_iovar(dhd, 0, "bus:intr_width", (char *)&intr_width,
						sizeof(intr_width), NULL, 0, TRUE);
				if (ret < 0) {
					DHD_ERROR(("%s: failed to set intr_width (%d)\n",
						__FUNCTION__, ret));
					goto exit;
				}
#endif /* DYNAMIC_SWOOB_DURATION */
#endif /* OEM_ANDROID */
			} else {
#ifdef PKT_FILTER_SUPPORT
				dhd->wlcore->early_suspended = 0;
#endif
				/* Kernel resumed  */
				DHD_ERROR(("%s: Remove extra suspend setting \n", __FUNCTION__));
#ifdef DYNAMIC_SWOOB_DURATION
				intr_width = 0;
				ret = dhd_iovar(dhd, 0, "bus:intr_width", (char *)&intr_width,
						sizeof(intr_width), NULL, 0, TRUE);
				if (ret < 0) {
					DHD_ERROR(("%s: failed to set intr_width (%d)\n",
						__FUNCTION__, ret));
					goto exit;
				}
#endif /* DYNAMIC_SWOOB_DURATION */
#ifndef SUPPORT_PM2_ONLY
				power_mode = PM_FAST;
				ret = dhd_wl_ioctl_cmd(dhd, WLC_SET_PM, (char *)&power_mode,
				                 sizeof(power_mode), TRUE, 0);
				if (ret < 0) {
					DHD_ERROR(("%s: failed to set WLC_SET_PM (%d)\n",
						__FUNCTION__, ret));
					goto exit;
				}
#endif /* SUPPORT_PM2_ONLY */
#ifdef PKT_FILTER_SUPPORT
				/* disable pkt filter */
				dhd_enable_packet_filter(0, dhd);
#endif /* PKT_FILTER_SUPPORT */
#if defined(PASS_ALL_MCAST_PKTS) && defined(CUSTOMER_HW4)
				allmulti = 1;
				for (i = 0; i < DHD_MAX_IFS; i++) {
					if (dhdinfo->iflist[i] && dhdinfo->iflist[i]->net)
						ret = dhd_iovar(dhd, i "allmulti",
							(char *)&allmulti, sizeof(allmulti),
							NULL, 0, TRUE);
				}

				if (ret < 0) {
					DHD_ERROR(("%s: failed to set bcn_li_dtim (%d)\n",
						__FUNCTION__, ret));
					goto exit;
				}

#endif /* PASS_ALL_MCAST_PKTS && CUSTOMER_HW4 */

				/* restore pre-suspend setting for dtim_skip */
				ret = dhd_iovar(dhd, 0, "bcn_li_dtim", (char *)&bcn_li_dtim,
						sizeof(bcn_li_dtim), NULL, 0, TRUE);
				if (ret < 0) {
					DHD_ERROR(("%s: set bcn_li_dtim failed (%d)\n",
						__FUNCTION__, ret));
					goto exit;
				}
#ifdef OEM_ANDROID
#ifdef CUSTOM_BCN_TIMEOUT_IN_SUSPEND
				bcn_timeout = CUSTOM_BCN_TIMEOUT;
				ret = dhd_iovar(dhd, 0, "bcn_timeout", (char *)&bcn_timeout,
						sizeof(bcn_timeout), NULL, 0, TRUE);
				if (ret < 0) {
					DHD_ERROR(("%s: set bcn_timeout failed (%d)\n",
						__FUNCTION__, ret));
					goto exit;
				}
#endif /* CUSTOM_BCN_TIMEOUT_IN_SUSPEND */
#ifdef CUSTOM_ROAM_TIME_THRESH_IN_SUSPEND
				roam_time_thresh = 2000;
				ret = dhd_iovar(dhd, 0, "roam_time_thresh",
					(char *)&roam_time_thresh,
					sizeof(roam_time_thresh), NULL, 0, TRUE);
				if (ret < 0) {
					DHD_ERROR(("%s: set roam_time_thresh failed (%d)\n",
						__FUNCTION__, ret));
					goto exit;
				}
#endif /* CUSTOM_ROAM_TIME_THRESH_IN_SUSPEND */
#ifndef ENABLE_FW_ROAM_SUSPEND
				roamvar = dhd_roam_disable;
				ret = dhd_iovar(dhd, 0, "roam_off", (char *)&roamvar,
					sizeof(roamvar), NULL, 0, TRUE);
				if (ret < 0) {
					DHD_ERROR(("%s: set roam_off failed (%d)\n",
						__FUNCTION__, ret));
					goto exit;
				}
#endif /* ENABLE_FW_ROAM_SUSPEND */
#if defined(CUSTOMER_HW4) && defined(ENABLE_BCN_LI_BCN_WAKEUP)
				bcn_li_bcn = 1;
				ret = dhd_iovar(dhd, 0, "bcn_li_bcn", (char *)&bcn_li_bcn,
						sizeof(bcn_li_bcn), NULL, 0, TRUE);
				if (ret < 0) {
					DHD_ERROR(("%s set nd_ra_filter failed (%d)\n",
						__FUNCTION__, ret));
					goto exit;
				}
#endif /* CUSTOMER_HW4 && ENABLE_BCN_LI_BCN_WAKEUP */
				if (FW_SUPPORTED(dhd->wlcore, ndoe)) {
					/* disable IPv6 RA filter in  firmware during suspend */
					nd_ra_filter = 0;
					ret = dhd_iovar(dhd, 0, "nd_ra_filter_enable",
							(char *)&nd_ra_filter, sizeof(nd_ra_filter),
							NULL, 0, TRUE);
					if (ret < 0) {
						DHD_ERROR(("%s: failed to set nd_ra_filter (%d)\n",
							__FUNCTION__, ret));
						goto exit;
					}
				}
#endif /* OEM_ANDROID */
			}
	}
	dhd_suspend_unlock(dhd);

exit:
	return ret;
}

static int
dhd_suspend_resume_helper(struct dhd_info *dhd, int val, int force)
{
	dhd_pub_t *dhdp = &dhd->pub;
	int ret = 0;

	DHD_OS_WAKE_LOCK(dhdp);
	DHD_LOCK(dhdp);

	/* Set flag when early suspend was called */
	dhdp->in_suspend = val;
	if ((force || !dhdp->suspend_disable_flag) &&
		dhd_support_sta_mode(dhdp))
	{
		ret = dhd_set_suspend(val, dhdp);
	}

	DHD_UNLOCK(dhdp);
	DHD_OS_WAKE_UNLOCK(dhdp);

	return ret;
}

#if defined(CONFIG_HAS_EARLYSUSPEND) && defined(DHD_USE_EARLYSUSPEND)
static void
dhd_early_suspend(struct early_suspend *h)
{
	dhd_info_t *dhd = container_of(h, dhd_info_t, early_suspend);
	DHD_TRACE_HW4(("%s: enter\n", __FUNCTION__));

	if (dhd)
		dhd_suspend_resume_helper(dhd, 1, 0);
}

static void
dhd_late_resume(struct early_suspend *h)
{
	dhd_info_t *dhd = container_of(h, dhd_info_t, early_suspend);
	DHD_TRACE_HW4(("%s: enter\n", __FUNCTION__));

	if (dhd)
		dhd_suspend_resume_helper(dhd, 0, 0);
}
#endif /* CONFIG_HAS_EARLYSUSPEND && DHD_USE_EARLYSUSPEND */

/*
 * Generalized timeout mechanism.  Uses spin sleep with exponential back-off until
 * the sleep time reaches one jiffy, then switches over to task delay.  Usage:
 *
 *      dhd_timeout_start(&tmo, usec);
 *      while (!dhd_timeout_expired(&tmo))
 *              if (poll_something())
 *                      break;
 *      if (dhd_timeout_expired(&tmo))
 *              fatal();
 */

void
dhd_timeout_start(dhd_timeout_t *tmo, uint usec)
{
	tmo->limit = usec;
	tmo->increment = 0;
	tmo->elapsed = 0;
	tmo->tick = jiffies_to_usecs(1);
}

int
dhd_timeout_expired(dhd_timeout_t *tmo)
{
	/* Does nothing the first call */
	if (tmo->increment == 0) {
		tmo->increment = 1;
		return 0;
	}

	if (tmo->elapsed >= tmo->limit)
		return 1;

	/* Add the delay that's about to take place */
	tmo->elapsed += tmo->increment;

	if ((!CAN_SLEEP()) || tmo->increment < tmo->tick) {
		OSL_DELAY(tmo->increment);
		tmo->increment *= 2;
		if (tmo->increment > tmo->tick)
			tmo->increment = tmo->tick;
	} else {
		wait_queue_head_t delay_wait;
		DECLARE_WAITQUEUE(wait, current);
		init_waitqueue_head(&delay_wait);
		add_wait_queue(&delay_wait, &wait);
		set_current_state(TASK_INTERRUPTIBLE);
		(void)schedule_timeout(1);
		remove_wait_queue(&delay_wait, &wait);
		set_current_state(TASK_RUNNING);
	}

	return 0;
}

int
dhd_net2idx(dhd_info_t *dhd, struct net_device *net)
{
	int i = 0;

	if (!dhd) {
		DHD_ERROR(("%s : DHD_BAD_IF return\n", __FUNCTION__));
		return DHD_BAD_IF;
	}

	while (i < DHD_MAX_IFS) {
		if (dhd->iflist[i] && dhd->iflist[i]->net && (dhd->iflist[i]->net == net))
			return i;
		i++;
	}

	return DHD_BAD_IF;
}

struct net_device *
dhd_idx2net(void *pub, int ifidx)
{
	struct dhd_pub *dhd_pub = (struct dhd_pub *)pub;
	struct dhd_info *dhd_info;

	if (!dhd_pub || ifidx < 0 || ifidx >= DHD_MAX_IFS)
		return NULL;

	dhd_info = dhd_pub->info;
	if (dhd_info && dhd_info->iflist[ifidx])
		return dhd_info->iflist[ifidx]->net;

	return NULL;
}

int
dhd_ifname2idx(dhd_info_t *dhd, char *name)
{
	int i = DHD_MAX_IFS;

	ASSERT(dhd);

	if (name == NULL || *name == '\0')
		return 0;

	while (--i > 0)
		if (dhd->iflist[i] && !strncmp(dhd->iflist[i]->dngl_name, name, IFNAMSIZ))
				break;

	DHD_TRACE(("%s: return idx %d for \"%s\"\n", __FUNCTION__, i, name));

	return i;	/* default - the primary interface */
}

#if defined(PCIE_FULL_DONGLE)
/* Update WDS interface info */
static void
dhd_update_wds_interface_info(dhd_pub_t *dhdp, int ifidx, wl_event_data_if_t *ifevent)
{
	dhd_info_t *dhd = (dhd_info_t *)dhdp->info;
	dhd_if_t *ifp;

	ASSERT(ifidx < DHD_MAX_IFS);

	if (!DHD_IF_ROLE_WDS(dhdp, ifidx)) {
		return;
	}

	ifp = dhd->iflist[ifidx];

	if (ifevent->reserved & WLC_E_IF_FLAGS_WDS_STA) {
		ifp->wds_role = WLC_E_IF_FLAGS_WDS_STA;
#if defined(BCM_PKTFWD) && defined(netdev_wlan_set_dwds_client)
		{
			dhd_pktfwd_priv_t *pktfwd_priv = dhd_pktfwd_get_priv(ifp->net);
			if (pktfwd_priv != NULL)
				netdev_wlan_set_dwds_client(pktfwd_priv->d3fwd_wlif);
		}
#endif /* BCM_PKTFWD && netdev_wlan_set_dwds_client */
	} else if (ifevent->reserved & WLC_E_IF_FLAGS_WDS_AP) {
		ifp->wds_role = WLC_E_IF_FLAGS_WDS_AP;
		memcpy(ifp->peer_wds_mac_addr, ifevent->peer_addr.octet, ETHER_ADDR_LEN);
#if defined(BCM_PKTFWD) && defined(netdev_wlan_set_dwds_ap)
		{
			dhd_pktfwd_priv_t *pktfwd_priv = dhd_pktfwd_get_priv(ifp->net);
			if (pktfwd_priv != NULL)
				netdev_wlan_set_dwds_ap(pktfwd_priv->d3fwd_wlif);
		}
#endif /* BCM_PKTFWD && netdev_wlan_set_dwds_ap */
	}

	DHD_INFO(("%s: %s: ifidx %d wds_role %d Peer MAC["MACDBG"]\n",
		dhd_ifname(dhdp, ifidx), __FUNCTION__, ifidx, ifp->wds_role,
		MAC2STRDBG(ifevent->peer_addr.octet)));
	return;
}
#endif /* PCIE_FULL_DONGLE */

#ifdef BCM_ROUTER_DHD
/* Set bss_up for a particular ifp
 * pass state as TRUE to mark this bsscfg enabled
 */
void
dhd_update_bsscfg_state(dhd_pub_t *dhdp, int ifindex, bool state)
{
	dhd_if_t *ifp;

	if (ifindex >= DHD_MAX_IFS)
		return;

	ifp = dhdp->info->iflist[ifindex];

	if (ifp == NULL) {
		return;
	}

	ifp->bss_up = state;

	/* Clean up ifp->primsta_dwds value */
	if (state == FALSE)
		ifp->primsta_dwds = FALSE;

	return;
}
#endif /* BCM_ROUTER_DHD */

char *
dhd_ifname(dhd_pub_t *dhdp, int ifidx)
{
	dhd_info_t *dhd = (dhd_info_t *)dhdp->info;

	ASSERT(dhd);

	if (ifidx < 0 || ifidx >= DHD_MAX_IFS) {
		DHD_ERROR(("%s: ifidx %d out of range\n", __FUNCTION__, ifidx));
		return "<if_bad>";
	}

	if (dhd->iflist[ifidx] == NULL) {
		DHD_ERROR(("%s: null i/f %d\n", __FUNCTION__, ifidx));
		return "<if_null>";
	}

	if (dhd->iflist[ifidx]->net)
		return dhd->iflist[ifidx]->net->name;

	return "<if_none>";
}

uint8 *
dhd_bssidx2bssid(dhd_pub_t *dhdp, int idx)
{
	int i;
	dhd_info_t *dhd = (dhd_info_t *)dhdp;

	ASSERT(dhd);
	for (i = 0; i < DHD_MAX_IFS; i++)
	if (dhd->iflist[i] && dhd->iflist[i]->bssidx == idx)
		return dhd->iflist[i]->mac_addr;

	return NULL;
}

#ifdef BCMDBUS
#define DBUS_NRXQ	50
#define DBUS_NTXQ	100

static void
dhd_dbus_send_complete(void *handle, void *info, int status)
{
	dhd_info_t *dhd = (dhd_info_t *)handle;
	void *pkt = info;

	if ((dhd == NULL) || (pkt == NULL))
		return;

	if (status == DBUS_OK) {
		dhd->pub.dstats.tx_packets++;
	} else {
		DHD_ERROR(("TX error=%d\n", status));
		dhd->pub.dstats.tx_errors++;
	}
#ifdef PROP_TXSTATUS
	if (DHD_PKTTAG_WLFCPKT(PKTTAG(pkt)) &&
		(dhd_wlfc_txcomplete(&dhd->pub, pkt, status == 0) != WLFC_UNSUPPORTED)) {
		return;
	}
#endif /* PROP_TXSTATUS */
	PKTFREE(dhd->pub.osh, pkt, TRUE);
}

static void
dhd_dbus_recv_pkt(void *handle, void *pkt)
{
	uchar reorder_info_buf[WLHOST_REORDERDATA_TOTLEN];
	uint reorder_info_len;
	uint pkt_count;
	dhd_info_t *dhd = (dhd_info_t *)handle;
	int ifidx = 0;

	if (dhd == NULL)
		return;

	/* If the protocol uses a data header, check and remove it */
	if (dhd_prot_hdrpull(&dhd->pub, &ifidx, pkt, reorder_info_buf,
		&reorder_info_len) != 0) {
		DHD_ERROR(("rx protocol error\n"));
		PKTFREE(dhd->pub.osh, pkt, FALSE);
		dhd->pub.rx_errors++;
		return;
	}

	if (reorder_info_len) {
		/* Reordering info from the firmware */
		dhd_process_pkt_reorder_info(&dhd->pub, reorder_info_buf, reorder_info_len,
			&pkt, &pkt_count);
		if (pkt_count == 0)
			return;
	}
	else {
		pkt_count = 1;
	}
	dhd_rx_frame(&dhd->pub, ifidx, pkt, pkt_count, 0);
}

static void
dhd_dbus_recv_buf(void *handle, uint8 *buf, int len)
{
	dhd_info_t *dhd = (dhd_info_t *)handle;
	void *pkt;

	if (dhd == NULL)
		return;

	if ((pkt = PKTGET(dhd->pub.osh, len, FALSE)) == NULL) {
		DHD_ERROR(("PKTGET (rx) failed=%d\n", len));
		return;
	}

	bcopy(buf, PKTDATA(dhd->pub.osh, pkt), len);
	dhd_dbus_recv_pkt(dhd, pkt);
}

static void
dhd_dbus_txflowcontrol(void *handle, bool onoff)
{
	dhd_info_t *dhd = (dhd_info_t *)handle;
	bool wlfc_enabled = FALSE;

	if (dhd == NULL)
		return;

#ifdef PROP_TXSTATUS
	wlfc_enabled = (dhd_wlfc_flowcontrol(&dhd->pub, onoff, !onoff) != WLFC_UNSUPPORTED);
#endif

	if (!wlfc_enabled) {
		dhd_txflowcontrol(&dhd->pub, ALL_INTERFACES, onoff);
	}
}

static void
dhd_dbus_errhandler(void *handle, int err)
{
}

static void
dhd_dbus_ctl_complete(void *handle, int type, int status)
{
	dhd_info_t *dhd = (dhd_info_t *)handle;

	if (dhd == NULL)
		return;

	if (type == DBUS_CBCTL_READ) {
		if (status == DBUS_OK)
			dhd->pub.rx_ctlpkts++;
		else
			dhd->pub.rx_ctlerrs++;
	} else if (type == DBUS_CBCTL_WRITE) {
		if (status == DBUS_OK)
			dhd->pub.tx_ctlpkts++;
		else
			dhd->pub.tx_ctlerrs++;
	}

	dhd_prot_ctl_complete(&dhd->pub);
}

static void
dhd_dbus_state_change(void *handle, int state)
{
	dhd_info_t *dhd = (dhd_info_t *)handle;

	if (dhd == NULL)
		return;

	switch (state) {

		case DBUS_STATE_DL_NEEDED:
#if defined(BCM_DNGL_EMBEDIMAGE) || defined(BCM_REQUEST_FW)
#if defined(BCMDBUS)
			DHD_TRACE(("%s: firmware request\n", __FUNCTION__));
			up(&dhd->fw_download_lock);
#endif /* BCMDBUS */
#else
			DHD_ERROR(("%s: firmware request cannot be handled\n", __FUNCTION__));
#endif /* defined(BCM_DNGL_EMBEDIMAGE) || defined(BCM_REQUEST_FW) */
			break;
		case DBUS_STATE_DOWN:
			DHD_TRACE(("%s: DBUS is down\n", __FUNCTION__));
			dhd->pub.busstate = DHD_BUS_DOWN;
			break;
		case DBUS_STATE_UP:
			DHD_TRACE(("%s: DBUS is up\n", __FUNCTION__));
#if defined(STB) && !defined(STBAP)
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 39))
			if (dhd->pub.busstate == DHD_BUS_SUSPEND) {
				dhd->pub.busstate = DHD_BUS_DATA;
				dhd_bus_set_wowl(dhd, FALSE);
				DHD_TRACE(("%s: DBUS is Resuming\n", __FUNCTION__));
			} else
#endif /* KERNEL_VERSION(2, 6, 39) */
#endif /* STB && STBAP */
			dhd->pub.busstate = DHD_BUS_DATA;
			break;
#if defined(STB) && !defined(STBAP)
		case DBUS_STATE_SLEEP:
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 39))
			dhd_bus_set_wowl(dhd, TRUE);
#endif /* KERNEL_VERSION(2, 6, 39) */
			dhd->pub.busstate = DHD_BUS_SUSPEND;
			break;
#endif /* STB && STBAP */
		default:
			break;
	}

	DHD_TRACE(("%s: DBUS current state=%d\n", __FUNCTION__, state));
}

static void *
dhd_dbus_pktget(void *handle, uint len, bool send)
{
	dhd_info_t *dhd = (dhd_info_t *)handle;
	void *p = NULL;

	if (dhd == NULL)
		return NULL;

	if (send == TRUE) {
		dhd_os_sdlock_txq(&dhd->pub);
		p = PKTGET(dhd->pub.osh, len, TRUE);
		dhd_os_sdunlock_txq(&dhd->pub);
	} else {
		dhd_os_sdlock_rxq(&dhd->pub);
		p = PKTGET(dhd->pub.osh, len, FALSE);
		dhd_os_sdunlock_rxq(&dhd->pub);
	}

	return p;
}

static void
dhd_dbus_pktfree(void *handle, void *p, bool send)
{
	dhd_info_t *dhd = (dhd_info_t *)handle;

	if (dhd == NULL)
		return;

	if (send == TRUE) {
#ifdef PROP_TXSTATUS
		if (DHD_PKTTAG_WLFCPKT(PKTTAG(p)) &&
			(dhd_wlfc_txcomplete(&dhd->pub, p, FALSE) != WLFC_UNSUPPORTED)) {
			return;
		}
#endif /* PROP_TXSTATUS */

		dhd_os_sdlock_txq(&dhd->pub);
		PKTFREE(dhd->pub.osh, p, TRUE);
		dhd_os_sdunlock_txq(&dhd->pub);
	} else {
		dhd_os_sdlock_rxq(&dhd->pub);
		PKTFREE(dhd->pub.osh, p, FALSE);
		dhd_os_sdunlock_rxq(&dhd->pub);
	}
}

#ifdef BCM_FD_AGGR

static void
dbus_rpcth_tx_complete(void *ctx, void *pktbuf, int status)
{
	dhd_info_t *dhd = (dhd_info_t *)ctx;
	void *tmp;

	while (pktbuf && dhd) {
		tmp = PKTNEXT(dhd->pub.osh, pktbuf);
		PKTSETNEXT(dhd->pub.osh, pktbuf, NULL);
		dhd_dbus_send_complete(ctx, pktbuf, status);
		pktbuf = tmp;
	}
}

static void
dbus_rpcth_rx_pkt(void *context, rpc_buf_t *rpc_buf)
{
	dhd_dbus_recv_pkt(context, rpc_buf);
}

static void
dbus_rpcth_rx_aggrpkt(void *context, void *rpc_buf)
{
	dhd_info_t *dhd = (dhd_info_t *)context;

	if (dhd == NULL)
		return;

	/* all the de-aggregated packets are delivered back to function dbus_rpcth_rx_pkt()
	* as cloned packets
	*/
	bcm_rpc_dbus_recv_aggrpkt(dhd->rpc_th, rpc_buf,
		bcm_rpc_buf_len_get(dhd->rpc_th, rpc_buf));

	/* free the original packet */
	dhd_dbus_pktfree(context, rpc_buf, FALSE);
}

static void
dbus_rpcth_rx_aggrbuf(void *context, uint8 *buf, int len)
{
	dhd_info_t *dhd = (dhd_info_t *)context;

	if (dhd == NULL)
		return;

	if (dhd->fdaggr & BCM_FDAGGR_D2H_ENABLED) {
		bcm_rpc_dbus_recv_aggrbuf(dhd->rpc_th, buf, len);
	}
	else {
		dhd_dbus_recv_buf(context, buf, len);
	}

}

static void
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
dhd_rpcth_watchdog(ulong data)
{
	dhd_info_t *dhd = (dhd_info_t *)data;
#else
dhd_rpcth_watchdog(struct timer_list *t)
{
	dhd_info_t *dhd = from_timer(dhd, t, rpcth_timer);
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0) */

	if (dhd->pub.dongle_reset) {
		return;
	}

	dhd->rpcth_timer_active = FALSE;
	/* release packets in the aggregation queue */
	bcm_rpc_tp_watchdog(dhd->rpc_th);
}

static int
dhd_fdaggr_ioctl(dhd_pub_t *dhd_pub, int ifindex, wl_ioctl_t *ioc, void *buf, int len)
{
	int bcmerror = 0;
	void *rpc_th;

	rpc_th = dhd_pub->info->rpc_th;

	if (!strcmp("rpc_agg", ioc->buf)) {
		uint32 rpc_agg;
		uint32 rpc_agg_host;
		uint32 rpc_agg_dngl;

		if (ioc->set) {
			memcpy(&rpc_agg, ioc->buf + strlen("rpc_agg") + 1, sizeof(uint32));
			rpc_agg_host = rpc_agg & BCM_RPC_TP_HOST_AGG_MASK;
			if (rpc_agg_host)
				bcm_rpc_tp_agg_set(rpc_th, rpc_agg_host, TRUE);
			else
				bcm_rpc_tp_agg_set(rpc_th, BCM_RPC_TP_HOST_AGG_MASK, FALSE);
			bcmerror = dhd_wl_ioctl(dhd_pub, ifindex, ioc, buf, len);
			if (bcmerror < 0) {
				DHD_ERROR(("usb aggregation not supported\n"));
			} else {
				dhd_pub->info->fdaggr = 0;
				if (rpc_agg & BCM_RPC_TP_HOST_AGG_MASK)
					dhd_pub->info->fdaggr |= BCM_FDAGGR_H2D_ENABLED;
				if (rpc_agg & BCM_RPC_TP_DNGL_AGG_MASK)
					dhd_pub->info->fdaggr |= BCM_FDAGGR_D2H_ENABLED;
			}
		} else {
			rpc_agg_host = bcm_rpc_tp_agg_get(rpc_th);
			bcmerror = dhd_wl_ioctl(dhd_pub, ifindex, ioc, buf, len);
			if (!bcmerror) {
				memcpy(&rpc_agg_dngl, buf, sizeof(uint32));
				rpc_agg = (rpc_agg_host & BCM_RPC_TP_HOST_AGG_MASK) |
					(rpc_agg_dngl & BCM_RPC_TP_DNGL_AGG_MASK);
				memcpy(buf, &rpc_agg, sizeof(uint32));
			}
		}
	} else if (!strcmp("rpc_host_agglimit", ioc->buf)) {
		uint8 sf;
		uint16 bytes;
		uint32 agglimit;

		if (ioc->set) {
			memcpy(&agglimit, ioc->buf + strlen("rpc_host_agglimit") + 1,
				sizeof(uint32));
			sf = agglimit >> 16;
			bytes = agglimit & 0xFFFF;
			bcm_rpc_tp_agg_limit_set(rpc_th, sf, bytes);
		} else {
			bcm_rpc_tp_agg_limit_get(rpc_th, &sf, &bytes);
			agglimit = (uint32)((sf << 16) + bytes);
			memcpy(buf, &agglimit, sizeof(uint32));
		}

	} else {
		bcmerror = dhd_wl_ioctl(dhd_pub, ifindex, ioc, buf, len);
	}

	return bcmerror;
}
#endif /* BCM_FD_AGGR */

static dbus_callbacks_t dhd_dbus_cbs = {
#ifdef BCM_FD_AGGR
	dbus_rpcth_tx_complete,
	dbus_rpcth_rx_aggrbuf,
	dbus_rpcth_rx_aggrpkt,
#else
	dhd_dbus_send_complete,
	dhd_dbus_recv_buf,
	dhd_dbus_recv_pkt,
#endif
	dhd_dbus_txflowcontrol,
	dhd_dbus_errhandler,
	dhd_dbus_ctl_complete,
	dhd_dbus_state_change,
	dhd_dbus_pktget,
	dhd_dbus_pktfree
};

void
dhd_bus_dump(dhd_pub_t *dhdp, struct bcmstrbuf *strbuf)
{
	bcm_bprintf(strbuf, "Bus USB\n");
}

void
dhd_bus_clearcounts(dhd_pub_t *dhdp)
{
}

bool
dhd_bus_dpc(struct dhd_bus *bus)
{
	return FALSE;
}

int
dhd_dbus_txdata(dhd_pub_t *dhdp, void *pktbuf)
{

	if (dhdp->txoff)
		return BCME_EPERM;
#ifdef BCM_FD_AGGR
	if (((dhd_info_t *)(dhdp->info))->fdaggr & BCM_FDAGGR_H2D_ENABLED)

	{
		dhd_info_t *dhd;
		int ret;
		dhd = (dhd_info_t *)(dhdp->info);
		ret = bcm_rpc_tp_buf_send(dhd->rpc_th, pktbuf);
		if (dhd->rpcth_timer_active == FALSE) {
			dhd->rpcth_timer_active = TRUE;
			mod_timer(&dhd->rpcth_timer, jiffies + BCM_RPC_TP_HOST_TMOUT * HZ / 1000);
		}
		return ret;
	} else
#endif /* BCM_FD_AGGR */

	return dbus_send_txdata(dhdp->dbus, pktbuf);
}

#endif /* BCMDBUS */

static void
_dhd_set_multicast_list(dhd_info_t *dhd, int ifidx)
{
	struct net_device *dev;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35)
	struct netdev_hw_addr *ha;
#else
	struct dev_mc_list *mclist;
#endif
	uint32 allmulti, cnt;

	wl_ioctl_t ioc;
	char *buf, *bufp;
	uint buflen;
	int ret;

#ifdef MCAST_LIST_ACCUMULATION
	int i;
	uint32 cnt_iface[DHD_MAX_IFS];
	cnt = 0;
	allmulti = 0;

	for (i = 0; i < DHD_MAX_IFS; i++) {
		if (dhd->iflist[i]) {
			dev = dhd->iflist[i]->net;
			if (!dev)
				continue;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)
			netif_addr_lock_bh(dev);
#endif /* LINUX >= 2.6.27 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35)
			cnt_iface[i] = netdev_mc_count(dev);
			cnt += cnt_iface[i];
#else
			cnt += dev->mc_count;
#endif /* LINUX >= 2.6.35 */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)
			netif_addr_unlock_bh(dev);
#endif /* LINUX >= 2.6.27 */

			/* Determine initial value of allmulti flag */
			allmulti |= (dev->flags & IFF_ALLMULTI) ? TRUE : FALSE;
		}
	}
#else /* !MCAST_LIST_ACCUMULATION */
	if (!dhd->iflist[ifidx]) {
		DHD_ERROR(("%s: %s : dhd->iflist[%d] was NULL\n",
			dhd_ifname(&dhd->pub, ifidx), __FUNCTION__, ifidx));
		return;
	}
	dev = dhd->iflist[ifidx]->net;
	if (!dev)
		return;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)
	netif_addr_lock_bh(dev);
#endif /* LINUX >= 2.6.27 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35)
	cnt = netdev_mc_count(dev);
#else
	cnt = dev->mc_count;
#endif /* LINUX >= 2.6.35 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)
	netif_addr_unlock_bh(dev);
#endif /* LINUX >= 2.6.27 */

	/* Determine initial value of allmulti flag */
	allmulti = (dev->flags & IFF_ALLMULTI) ? TRUE : FALSE;
#endif /* MCAST_LIST_ACCUMULATION */

#if defined(PASS_ALL_MCAST_PKTS) && defined(CUSTOMER_HW4)
#ifdef PKT_FILTER_SUPPORT
	if (!dhd->pub.wlcore->early_suspended)
#endif /* PKT_FILTER_SUPPORT */
		allmulti = TRUE;
#endif /* PASS_ALL_MCAST_PKTS && CUSTOMER_HW4 */

	/* Send down the multicast list first. */

	/* XXX Not using MAXMULTILIST to avoid including wlc_pub.h; but
	 * maybe we should?  (Or should that be in wlioctl.h instead?)
	 */

	buflen = sizeof("mcast_list") + sizeof(cnt) + (cnt * ETHER_ADDR_LEN);
	if (!(bufp = buf = MALLOC(dhd->pub.osh, buflen))) {
		DHD_ERROR(("%s: out of memory for mcast_list, cnt %d\n",
		           dhd_ifname(&dhd->pub, ifidx), cnt));
		return;
	}

	strncpy(bufp, "mcast_list", buflen - 1);
	bufp[buflen - 1] = '\0';
	bufp += strlen("mcast_list") + 1;

	cnt = htol32(cnt);
	memcpy(bufp, &cnt, sizeof(cnt));
	bufp += sizeof(cnt);

#ifdef MCAST_LIST_ACCUMULATION
	for (i = 0; i < DHD_MAX_IFS; i++) {
		if (dhd->iflist[i]) {
			DHD_TRACE(("_dhd_set_multicast_list: ifidx %d\n", i));
			dev = dhd->iflist[i]->net;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)
			netif_addr_lock_bh(dev);
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35)
			netdev_for_each_mc_addr(ha, dev) {
				if (!cnt_iface[i])
					break;
				memcpy(bufp, ha->addr, ETHER_ADDR_LEN);
				bufp += ETHER_ADDR_LEN;
				DHD_TRACE(("_dhd_set_multicast_list: cnt "
					"%d " MACDBG "\n",
					cnt_iface[i], MAC2STRDBG(ha->addr)));
				cnt_iface[i]--;
			}
#else /* LINUX < 2.6.35 */
			for (mclist = dev->mc_list; (mclist && (cnt_iface[i] > 0));
				cnt_iface[i]--, mclist = mclist->next) {
				memcpy(bufp, (void *)mclist->dmi_addr, ETHER_ADDR_LEN);
				bufp += ETHER_ADDR_LEN;
			}
#endif /* LINUX >= 2.6.35 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)
			netif_addr_unlock_bh(dev);
#endif /* LINUX >= 2.6.27 */
		}
	}
#else /* !MCAST_LIST_ACCUMULATION */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)
	netif_addr_lock_bh(dev);
#endif /* LINUX >= 2.6.27 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35)
	netdev_for_each_mc_addr(ha, dev) {
		if (!cnt)
			break;
		memcpy(bufp, ha->addr, ETHER_ADDR_LEN);
		bufp += ETHER_ADDR_LEN;
		cnt--;
	}
#else /* LINUX < 2.6.35 */
	for (mclist = dev->mc_list; (mclist && (cnt > 0));
		cnt--, mclist = mclist->next) {
		memcpy(bufp, (void *)mclist->dmi_addr, ETHER_ADDR_LEN);
		bufp += ETHER_ADDR_LEN;
	}
#endif /* LINUX >= 2.6.35 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)
	netif_addr_unlock_bh(dev);
#endif /* LINUX >= 2.6.27 */
#endif /* MCAST_LIST_ACCUMULATION */

	memset(&ioc, 0, sizeof(ioc));
	ioc.cmd = WLC_SET_VAR;
	ioc.buf = buf;
	ioc.len = buflen;
	ioc.set = TRUE;

	ret = dhd_wl_ioctl(&dhd->pub, ifidx, &ioc, ioc.buf, ioc.len);
	if (ret < 0) {
		DHD_ERROR(("%s: set mcast_list failed, cnt %d\n",
			dhd_ifname(&dhd->pub, ifidx), cnt));
		allmulti = cnt ? TRUE : allmulti;
	}

	MFREE(dhd->pub.osh, buf, buflen);

	/* Now send the allmulti setting.  This is based on the setting in the
	 * net_device flags, but might be modified above to be turned on if we
	 * were trying to set some addresses and dongle rejected it...
	 */

	allmulti = htol32(allmulti);

	ret = dhd_iovar(&dhd->pub, ifidx, "allmulti", (char *)&allmulti,
		sizeof(allmulti), NULL, 0, TRUE);
	if (ret < 0) {
		DHD_ERROR(("%s: set allmulti %d failed\n",
		           dhd_ifname(&dhd->pub, ifidx), ltoh32(allmulti)));
	}

	/* Finally, pick up the PROMISC flag as well, like the NIC driver does */

#ifdef MCAST_LIST_ACCUMULATION
	allmulti = 0;
	for (i = 0; i < DHD_MAX_IFS; i++) {
		if (dhd->iflist[i]) {
			dev = dhd->iflist[i]->net;
			allmulti |= (dev->flags & IFF_PROMISC) ? TRUE : FALSE;
		}
	}
#else
	allmulti = (dev->flags & IFF_PROMISC) ? TRUE : FALSE;
#endif /* MCAST_LIST_ACCUMULATION */

	allmulti = htol32(allmulti);

	memset(&ioc, 0, sizeof(ioc));
	ioc.cmd = WLC_SET_PROMISC;
	ioc.buf = &allmulti;
	ioc.len = sizeof(allmulti);
	ioc.set = TRUE;

	ret = dhd_wl_ioctl(&dhd->pub, ifidx, &ioc, ioc.buf, ioc.len);
	if (ret < 0) {
		DHD_ERROR(("%s: set promisc %d failed\n",
		           dhd_ifname(&dhd->pub, ifidx), ltoh32(allmulti)));
	}
}

int
_dhd_set_mac_address(dhd_info_t *dhd, int ifidx, uint8 *addr)
{
	int ret;

	ret = dhd_iovar(&dhd->pub, ifidx, "cur_etheraddr", (char *)addr,
			ETHER_ADDR_LEN, NULL, 0, TRUE);
	if (ret < 0) {
		DHD_ERROR(("%s: set cur_etheraddr failed\n", dhd_ifname(&dhd->pub, ifidx)));
	} else {
		memcpy(dhd->iflist[ifidx]->net->dev_addr, addr, ETHER_ADDR_LEN);
		if (ifidx == 0)
			memcpy(dhd->pub.mac.octet, addr, ETHER_ADDR_LEN);
	}

	return ret;
}

#ifdef SOFTAP
extern struct net_device *ap_net_dev;
extern tsk_ctl_t ap_eth_ctl; /* ap netdev heper thread ctl */
#endif

#ifdef BCM_ROUTER_DHD
void
dhd_update_dpsta_interface_for_sta(dhd_pub_t* dhdp, int ifidx, void* event_data)
{
	struct wl_dpsta_intf_event *dpsta_prim_event = (struct wl_dpsta_intf_event *)event_data;
	dhd_if_t *ifp = dhdp->info->iflist[ifidx];

	if (dpsta_prim_event->intf_type == WL_INTF_DWDS) {
		ifp->primsta_dwds = TRUE;
	} else {
		ifp->primsta_dwds = FALSE;
	}
#if defined(DHD_DPSTA) && defined(DHD_PSTA)
	if (dpsta_prim_event->intf_type == WL_INTF_PSTA) {
		dhd_dpsta_psta_register(dhdp);
	}
	else if (dpsta_prim_event->intf_type == WL_INTF_DWDS) {
		dhd_dpsta_dwds_register(dhdp);
	}
#endif /* DHD_DPSTA && DHD_PSTA */
}
#endif /* BCM_ROUTER_DHD */

#ifdef DHD_WMF
void
dhd_update_psta_interface_for_sta(dhd_pub_t* dhdp, char* ifname, void* ea,
	void* event_data)
{
	struct wl_psta_primary_intf_event *psta_prim_event =
		(struct wl_psta_primary_intf_event*)event_data;
	dhd_sta_t *psta_interface =  NULL;
	dhd_sta_t *sta = NULL;
	uint8 ifindex;
	ASSERT(ifname);
	ASSERT(psta_prim_event);
	ASSERT(ea);

	ifindex = (uint8)dhd_ifname2idx(dhdp->info, ifname);
	sta = dhd_find_sta(dhdp, ifindex, ea);
	if (sta != NULL) {
		psta_interface = dhd_find_sta(dhdp, ifindex,
			(void *)(psta_prim_event->prim_ea.octet));
		if (psta_interface != NULL) {
			sta->psta_prim = psta_interface;
		}
	}
}

/* Get wmf_psta_disable configuration configuration */
int
dhd_get_wmf_psta_disable(dhd_pub_t *dhdp, uint32 idx)
{
	dhd_info_t *dhd = dhdp->info;
	dhd_if_t *ifp;
	ASSERT(idx < DHD_MAX_IFS);
	ifp = dhd->iflist[idx];

	return ifp->wmf_psta_disable;
}

/* Set wmf_psta_disable configuration configuration */
int
dhd_set_wmf_psta_disable(dhd_pub_t *dhdp, uint32 idx, int val)
{
	dhd_info_t *dhd = dhdp->info;
	dhd_if_t *ifp;
	ASSERT(idx < DHD_MAX_IFS);
	ifp = dhd->iflist[idx];
	ifp->wmf_psta_disable = val;

	return 0;
}
#endif /* DHD_WMF */

#ifdef DHD_PSTA

/* Get psta/psr configuration configuration */
int
dhd_get_psta_mode(dhd_pub_t *dhdp)
{
	dhd_info_t *dhd = dhdp->info;

	return (int)dhd->psta_mode;
}

/* Set psta/psr configuration configuration */
int dhd_set_psta_mode(dhd_pub_t *dhdp, uint32 val)
{
	dhd_info_t *dhd = dhdp->info;
	dhd->psta_mode = val;

	return 0;
}

#endif /* DHD_PSTA */

#if (defined(DHD_WET) || defined(DHD_MCAST_REGEN) || defined(DHD_L2_FILTER))
static void
dhd_update_rx_pkt_chainable_state(dhd_pub_t* dhdp, uint32 idx)
{
	dhd_info_t *dhd = dhdp->info;
	dhd_if_t *ifp;

	ASSERT(idx < DHD_MAX_IFS);

	ifp = dhd->iflist[idx];

	if (
#ifdef DHD_L2_FILTER
		(ifp->block_ping) ||
#endif
#ifdef DHD_WET
		(dhd->wet_mode) ||
#endif
#ifdef DHD_MCAST_REGEN
		(ifp->mcast_regen_bss_enable) ||
#endif
		FALSE) {
		ifp->rx_pkt_chainable = FALSE;
	}
}
#endif /* DHD_WET || DHD_MCAST_REGEN || DHD_L2_FILTER */

#ifdef DHD_WET
/* Get wet configuration configuration */
int
dhd_get_wet_mode(dhd_pub_t *dhdp)
{
	dhd_info_t *dhd = dhdp->info;

	return (int)dhd->wet_mode;
}

/* Set wet configuration configuration */
int
dhd_set_wet_mode(dhd_pub_t *dhdp, uint32 val)
{
	dhd_info_t *dhd = dhdp->info;
	dhd->wet_mode = val;

	/* disable rx_pkt_chainable stae for dhd interface, if WET is enabled */
	dhd_update_rx_pkt_chainable_state(dhdp, 0);

#if defined(DHD_DPSTA) && defined(DHD_WET)
	if (WET_ENABLED(dhdp)) {
		dhd_dpsta_wet_register(dhdp);
	}
#endif /* DHD_DPSTA && DHD_WET */

	return 0;
}
#endif /* DHD_WET */

#if defined(BCM_PKTFWD)
void dhd_wlan_set_dwds_client(dhd_pub_t *dhdp, uint8 ifidx, bool set)
{
	dhd_info_t *dhd = dhdp->info;
	dhd_if_t *ifp = dhd->iflist[ifidx];
	dhd_pktfwd_priv_t  *pktfwd_priv;

	pktfwd_priv = dhd_pktfwd_get_priv(ifp->net);

	if (pktfwd_priv != NULL) {
		if (set == TRUE) {
			/* mark net device as dwds client */
			BCM_REFERENCE(pktfwd_priv);
#if defined(netdev_wlan_set_dwds_client)
			netdev_wlan_set_dwds_client(pktfwd_priv->d3fwd_wlif);
#endif /* netdev_wlan_set_dwds_client */
		} else {
			/* unset dwds client */
			BCM_REFERENCE(pktfwd_priv);
#if defined(netdev_wlan_unset_dwds_client)
			netdev_wlan_unset_dwds_client(pktfwd_priv->d3fwd_wlif);
#endif /* netdev_wlan_unset_dwds_client */
		}
	}
}
#endif /* BCM_PKTFWD */

static void
dhd_ifadd_event_handler(void *handle, void *event_info, enum dhd_wq_event event)
{
	dhd_info_t *dhd = handle;
	dhd_if_event_t *if_event = event_info;
	struct net_device *ndev;
	dhd_if_t *ifp;
	int ifidx, bssidx;
	int ret;
#if defined(WL_CFG80211) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 0))
	struct wl_if_event_info info;
	char *if_name = NULL;
#endif /* WL_CFG80211 && LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 0) */
#if defined(WL_CFG80211) && defined(WL_HAPD_WDS)
	bool cfg_iface = FALSE;
#endif /* WL_CFG80211 && WL_HAPD_WDS */

	BCM_REFERENCE(ifp);

	if (event != DHD_WQ_WORK_IF_ADD) {
		DHD_ERROR(("%s: unexpected event \n", __FUNCTION__));
		return;
	}

	if (!dhd) {
		DHD_ERROR(("%s: dhd info not available \n", __FUNCTION__));
		return;
	}

	if (!if_event) {
		DHD_ERROR(("%s: event data is null \n", __FUNCTION__));
		return;
	}

	dhd_net_if_lock_local(dhd);
	DHD_OS_WAKE_LOCK(&dhd->pub);
	DHD_LOCK(&dhd->pub);

	ifidx = if_event->event.ifidx;
	bssidx = if_event->event.bssidx;

#if defined(WL_CFG80211) && defined(WL_HAPD_WDS)
	if (if_event->event.role == WLC_E_IF_ROLE_WDS) {
		if_name = if_event->name;
		if (if_event->event.reserved & WLC_E_IF_FLAGS_WDS_STA) {
			cfg_iface = TRUE;
		}
	}
	else
		cfg_iface = TRUE;
#endif /* WL_CFG80211 && WL_HAPD_WDS */

	DHD_TRACE(("%s: registering if with ifidx %d\n", __FUNCTION__, ifidx));

#if defined(PCIE_FULL_DONGLE)
	if (DHD_IF_ROLE_WDS(&dhd->pub, ifidx)) {
		dhd_update_dwds_sta_record(&dhd->pub, ifidx);
	}
#endif /* PCIE_FULL_DONGLE */

#if defined(WL_CFG80211) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 0))
	if ((dhd->dhd_state & DHD_ATTACH_STATE_CFG80211) &&
#if defined(WL_HAPD_WDS)
		cfg_iface &&
#endif /* WL_HAPD_WDS */
		(TRUE)) {

		if (ifidx > 0) {
			bzero(&info, sizeof(info));
			info.ifidx = ifidx;
			info.bssidx = bssidx;
			info.role = if_event->event.role;
			strncpy(info.name, if_event->name, IFNAMSIZ);
			DHD_UNLOCK(&dhd->pub);
			ndev = wl_cfg80211_post_ifcreate(dhd->pub.info->iflist[0]->net,
				&info, if_event->mac, if_name, true);
			DHD_LOCK(&dhd->pub);
			if (ndev != NULL) {
				/* Do the post interface create ops */
				DHD_ERROR(("%s: ifindex %u Post ifcreate ops done. Returning \n",
					if_name, ifidx));

#if defined(PCIE_FULL_DONGLE)
				dhd_update_wds_interface_info(&dhd->pub, ifidx, &if_event->event);

#if defined(BCM_PKTFWD)
				if (DHD_IF_ROLE_WDS(&dhd->pub, ifidx) ||
				    DHD_IF_ROLE_STA(&dhd->pub, ifidx)) {
#if defined(BCM_PKTFWD_DWDS)
					dhd_alloc_dwds_idx(&dhd->pub, ifidx);
#endif /* BCM_PKTFWD_DWDS */
				}
#endif /* BCM_PKTFWD */
#endif /* PCIE_FULL_DONGLE */
				goto done;
			}
		}
	}
#endif /* WL_CFG80211 && LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 0) */
	/* This path is for non-android case */
	/* The interface name in host and in event msg are same */
	/* if name in event msg is used to create dongle if list on host */

	ndev = dhd_allocate_if(&dhd->pub, ifidx, if_event->name,
		if_event->mac, bssidx, TRUE, if_event->name);

	if (!ndev) {
		DHD_ERROR(("%s: %s: net device alloc failed\n",
			if_event->name, __FUNCTION__));
		goto done;
	}

#if defined(PCIE_FULL_DONGLE)
	dhd_update_wds_interface_info(&dhd->pub, ifidx, &if_event->event);
#endif /* PCIE_FULL_DONGLE */

	DHD_UNLOCK(&dhd->pub);
	ret = dhd_register_if(&dhd->pub, ifidx, TRUE);
	DHD_LOCK(&dhd->pub);
	if (ret != BCME_OK) {
		DHD_ERROR(("%s: %s: ifindex %u dhd_register_if failed\n",
			if_event->name, __FUNCTION__, ifidx));
		dhd_remove_if(&dhd->pub, ifidx, TRUE);
		goto done;
	}

#if defined(PCIE_FULL_DONGLE) && defined(BCM_PKTFWD)
	/* update dwds info has to be after dhd_allocate_if to get ifp */
	if (DHD_IF_ROLE_WDS(&dhd->pub, ifidx) || DHD_IF_ROLE_STA(&dhd->pub, ifidx)) {
#if defined(BCM_PKTFWD_DWDS)
		dhd_alloc_dwds_idx(&dhd->pub, ifidx);
#endif /* BCM_PKTFWD_DWDS */
	}
#endif /* PCIE_FULL_DONGLE && BCM_PKTFWD */

#ifdef PCIE_FULL_DONGLE
	/* Turn on AP isolation in the firmware for interfaces operating in AP mode */
	if (FW_SUPPORTED((&dhd->pub)->wlcore, ap) && (if_event->event.role != WLC_E_IF_ROLE_STA)) {
		uint32 var_int =  1;

		ret = dhd_iovar(&dhd->pub, ifidx, "ap_isolate", (char *)&var_int, sizeof(var_int),
				NULL, 0, TRUE);

		if (ret != BCME_OK) {
			DHD_ERROR(("%s: %s: Failed to set ap_isolate to dongle\n",
				dhd_ifname(&dhd->pub, ifidx), __FUNCTION__));
			dhd_remove_if(&dhd->pub, ifidx, TRUE);
		}
	}
#endif /* PCIE_FULL_DONGLE */

done:
	MFREE(dhd->pub.osh, if_event, sizeof(dhd_if_event_t));

	DHD_UNLOCK(&dhd->pub);
	DHD_OS_WAKE_UNLOCK(&dhd->pub);
	dhd_net_if_unlock_local(dhd);
}

static void
dhd_ifdel_event_handler(void *handle, void *event_info, enum dhd_wq_event event)
{
	dhd_info_t *dhd = handle;
	int ifidx;
	dhd_if_event_t *if_event = event_info;

	if (event != DHD_WQ_WORK_IF_DEL) {
		DHD_ERROR(("%s: unexpected event \n", __FUNCTION__));
		return;
	}

	if (!dhd) {
		DHD_ERROR(("%s: dhd info not available \n", __FUNCTION__));
		return;
	}

	if (!if_event) {
		DHD_ERROR(("%s: event data is null \n", __FUNCTION__));
		return;
	}

	dhd_net_if_lock_local(dhd);
	DHD_OS_WAKE_LOCK(&dhd->pub);
	DHD_LOCK(&dhd->pub);

	ifidx = if_event->event.ifidx;
	if (dhd->pub.info->iflist[ifidx] == NULL) {
		/* i/f event is no longer relevant */
		goto done;
	}
	DHD_TRACE(("Removing interface with idx %d\n", ifidx));

#if defined(BCM_PKTFWD)
	if (DHD_IF_ROLE_WDS(&dhd->pub, ifidx) || DHD_IF_ROLE_STA(&dhd->pub, ifidx)) {
#if defined(BCM_PKTFWD_DWDS)
		dhd_free_dwds_idx(&dhd->pub, ifidx);
#endif /* BCM_PKTFWD_DWDS */
	}
#endif /* BCM_PKTFWD */

#if defined(WL_CFG80211) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 0))
	if (if_event->event.ifidx > 0) {
		struct net_device *ndev = dhd->pub.info->iflist[ifidx]->net;
		DHD_UNLOCK(&dhd->pub);

		rtnl_lock();

		if (ndev->reg_state != NETREG_UNINITIALIZED) {
			netif_tx_disable(ndev);
			dev_deactivate(ndev);
		}

		/* Do the post interface del ops */
		if (wl_cfg80211_post_ifdel(ndev) == 0) {
			DHD_TRACE(("Post ifdel ops done. Returning \n"));
			rtnl_unlock();
			DHD_LOCK(&dhd->pub);
			goto done;
		}
		rtnl_unlock();
		DHD_LOCK(&dhd->pub);
	}
#endif /* WL_CFG80211 && LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 0) */
	dhd_remove_if(&dhd->pub, ifidx, TRUE);
done:
	MFREE(dhd->pub.osh, if_event, sizeof(dhd_if_event_t));

	DHD_UNLOCK(&dhd->pub);
	DHD_OS_WAKE_UNLOCK(&dhd->pub);
	dhd_net_if_unlock_local(dhd);
}

static void
dhd_set_mac_addr_handler(void *handle, void *event_info, enum dhd_wq_event event)
{
	dhd_info_t *dhd = handle;
	int ifidx = (int) (long) event_info;
	dhd_if_t *ifp;

	if (event != DHD_WQ_WORK_SET_MAC) {
		DHD_ERROR(("%s: unexpected event \n", __FUNCTION__));
		return;
	}

	if (ifidx < 0 || ifidx >= DHD_MAX_IFS) {
		DHD_ERROR(("%s: ifidx %d out of range\n", __FUNCTION__, ifidx));
		return;
	}

	if (!dhd) {
		DHD_ERROR(("%s: dhd info not available \n", __FUNCTION__));
		return;
	}

	dhd_net_if_lock_local(dhd);
	DHD_OS_WAKE_LOCK(&dhd->pub);
	DHD_LOCK(&dhd->pub);

	ifp = dhd->iflist[ifidx];

	if (ifp == NULL) {
		DHD_ERROR(("%s: interface info not available/down \n", __FUNCTION__));
		goto done;
	}

#ifdef SOFTAP
	{
		unsigned long flags;
		bool in_ap = FALSE;
		DHD_GENERAL_LOCK(&dhd->pub, flags);
		in_ap = (ap_net_dev != NULL);
		DHD_GENERAL_UNLOCK(&dhd->pub, flags);

		if (in_ap)  {
			DHD_ERROR(("attempt to set MAC for %s in AP Mode, blocked. \n",
			           ifp->net->name));
			goto done;
		}
	}
#endif /* SOFTAP */

	if (!ifp->set_macaddress) {
		DHD_ERROR(("%s: interface flag set_macaddress not set\n", __FUNCTION__));
		goto done;
	}

	DHD_ERROR(("%s: MACID is overwritten\n", __FUNCTION__));
	ifp->set_macaddress = FALSE;
	if (_dhd_set_mac_address(dhd, ifp->idx, ifp->mac_addr) == 0)
		DHD_INFO(("%s: MACID is overwritten\n",	__FUNCTION__));
	else
		DHD_ERROR(("%s: _dhd_set_mac_address() failed\n", __FUNCTION__));

done:
	DHD_UNLOCK(&dhd->pub);
	DHD_OS_WAKE_UNLOCK(&dhd->pub);
	dhd_net_if_unlock_local(dhd);
}

static void
dhd_set_mcast_list_handler(void *handle, void *event_info, enum dhd_wq_event event)
{
	dhd_info_t *dhd = handle;
	struct net_device *dev = (struct net_device *)event_info;
	int ifidx;
	dhd_if_t *ifp;

	if (event != DHD_WQ_WORK_SET_MCAST_LIST) {
		DHD_ERROR(("%s: unexpected event \n", __FUNCTION__));
		return;
	}

	if (!dev) {
		DHD_ERROR(("%s: net_device not available\n", __FUNCTION__));
		return;
	}

	if (!dhd) {
		DHD_ERROR(("%s: dhd info not available \n", __FUNCTION__));
		return;
	}

	dhd_net_if_lock_local(dhd);
	DHD_OS_WAKE_LOCK(&dhd->pub);
	DHD_LOCK(&dhd->pub);

	ifidx = dhd_net2idx(dhd, dev);
	if (ifidx == DHD_BAD_IF) {
		DHD_ERROR(("%s: BAD_IF\n", __FUNCTION__));
		goto done;
	}

	ifp = dhd->iflist[ifidx];

	if (ifp == NULL) {
		DHD_ERROR(("%s: interface info not available/down \n", __FUNCTION__));
		goto done;
	}

#ifdef SOFTAP
	{
		bool in_ap = FALSE;
		unsigned long flags;
		DHD_GENERAL_LOCK(&dhd->pub, flags);
		in_ap = (ap_net_dev != NULL);
		DHD_GENERAL_UNLOCK(&dhd->pub, flags);

		if (in_ap)  {
			DHD_ERROR(("set MULTICAST list for %s in AP Mode, blocked. \n",
			           ifp->net->name));
			goto done;
		}
	}
#endif /* SOFTAP */

#ifdef MCAST_LIST_ACCUMULATION
	ifidx = 0;
#endif /* MCAST_LIST_ACCUMULATION */

	_dhd_set_multicast_list(dhd, ifidx);
	DHD_INFO(("%s: %s: set multicast list for if %d\n", ifp->name, __FUNCTION__, ifidx));

done:
	DHD_UNLOCK(&dhd->pub);
	DHD_OS_WAKE_UNLOCK(&dhd->pub);
	dhd_net_if_unlock_local(dhd);
}

static int
dhd_set_mac_address(struct net_device *dev, void *addr)
{
	int ret = 0;

	dhd_info_t *dhd = DHD_DEV_INFO(dev);
	struct sockaddr *sa = (struct sockaddr *)addr;
	int ifidx;
	dhd_if_t *dhdif;
#if defined(SUPPORT_RANDOM_MAC)
	bool addr_chngd = TRUE;
	BCM_REFERENCE(addr_chngd);
#endif /* SUPPORT_RANDOM_MAC */

	if (!is_valid_ether_addr(sa->sa_data)) {
		DHD_ERROR(("%s: mac address %pM is invalid\n", __FUNCTION__, sa->sa_data));
		return -1;
	}

	dhd_net_if_lock_local(dhd);
	DHD_OS_WAKE_LOCK(&dhd->pub);
	DHD_LOCK(&dhd->pub);
	ifidx = dhd_net2idx(dhd, dev);
	if (ifidx == DHD_BAD_IF) {
		DHD_UNLOCK(&dhd->pub);
		DHD_OS_WAKE_UNLOCK(&dhd->pub);
		dhd_net_if_unlock_local(dhd);
		return -1;
	}

	dhdif = dhd->iflist[ifidx];
#if defined(SUPPORT_RANDOM_MAC)
	if (memcmp(dhdif->mac_addr, sa->sa_data, ETHER_ADDR_LEN) == 0) {
		addr_chngd = FALSE;
	}
#endif /* SUPPORT_RANDOM_MAC */
	memcpy(dhdif->mac_addr, sa->sa_data, ETHER_ADDR_LEN);
	dhdif->set_macaddress = TRUE;
	DHD_UNLOCK(&dhd->pub);
	DHD_OS_WAKE_UNLOCK(&dhd->pub);
	dhd_net_if_unlock_local(dhd);

#if defined(WL_CFG80211) && defined(SUPPORT_RANDOM_MAC)
	/* Check wdev->iftype for the role */
	if (wl_cfg80211_macaddr_sync_reqd(dev)) {
		/* Supplicant and certain user layer applications expect macaddress to be
		 * set once the context returns. so set it from the same context
		 */
		DHD_ERROR(("%s: iftype = %d macaddr = "MACDBG"\n",
			__FUNCTION__, dev->ieee80211_ptr->iftype, MAC2STRDBG(&dhdif->mac_addr)));

		wl_cfg80211_handle_macaddr_change(dev, dhdif->mac_addr);
		ret = _dhd_set_mac_address(dhd, ifidx, dhdif->mac_addr);
		if ((ret == BCME_OK) && (addr_chngd == TRUE)) {
			/* Notify Dev/Address change to upperlayer */
			netdev_state_change(dev);
		}
		return ret;
	}
#endif /* WL_CFG80211 && SUPPORT_RANDOM_MAC */

	dhd_deferred_schedule_work(dhd->dhd_deferred_wq, (void *)(long)ifidx,
		DHD_WQ_WORK_SET_MAC, dhd_set_mac_addr_handler, DHD_WORK_PRIORITY_LOW);

	return ret;
}

static void
dhd_set_multicast_list(struct net_device *dev)
{
	dhd_info_t *dhd = DHD_DEV_INFO(dev);

	dhd_deferred_schedule_work(dhd->dhd_deferred_wq, dev,
		DHD_WQ_WORK_SET_MCAST_LIST, dhd_set_mcast_list_handler, DHD_WORK_PRIORITY_LOW);
}

#ifdef PROP_TXSTATUS
int
dhd_os_wlfc_block(dhd_pub_t *pub)
{
	dhd_info_t *di = (dhd_info_t *)(pub->info);
	ASSERT(di != NULL);
#ifdef BCMDBUS
	spin_lock_irqsave(&di->wlfc_spinlock, di->wlfc_lock_flags);
#else
	spin_lock_bh(&di->wlfc_spinlock);
#endif

	return 1;
}

int
dhd_os_wlfc_unblock(dhd_pub_t *pub)
{
	dhd_info_t *di = (dhd_info_t *)(pub->info);

	ASSERT(di != NULL);
#ifdef BCMDBUS
	spin_unlock_irqrestore(&di->wlfc_spinlock, di->wlfc_lock_flags);
#else
	spin_unlock_bh(&di->wlfc_spinlock);
#endif

	return 1;
}

#endif /* PROP_TXSTATUS */

int
dhd_os_get_image_size(void *image)
{
	struct file *fp = (struct file *)image;
	int size;

	if (!image) {
		return 0;
	}
	size = i_size_read(file_inode(fp));

	return size;
}

#if defined(DHD_8021X_DUMP)
void
dhd_tx_dump(osl_t *osh, void *pkt)
{
	uint8 *dump_data;
	uint16 protocol;

	dump_data = PKTDATA(osh, pkt);
	protocol = (dump_data[12] << 8) | dump_data[13];

	if (protocol == ETHER_TYPE_802_1X) {
		DHD_DATA(("ETHER_TYPE_802_1X [TX]: ver %d, type %d, replay %d\n",
			dump_data[14], dump_data[15], dump_data[30]));
	}
}
#endif /* DHD_8021X_DUMP */

/*  This routine do not support Packet chain feature, Currently tested for
 *  proxy arp feature
 */
int
dhd_sendup(dhd_pub_t *dhdp, int ifidx, void *p)
{
	struct sk_buff *skb = NULL;
	void *skbhead = NULL;
	void *skbprev = NULL;
	dhd_if_t *ifp;
	ASSERT(!PKTISCHAINED(p));
	skb = PKTTONATIVE(dhdp->osh, p);

	ifp = dhdp->info->iflist[ifidx];
	skb->dev = ifp->net;

#ifdef DHD_DPSTA
	if (PSTA_ENABLED(dhdp) || WET_ENABLED(dhdp)) {
		if (dpsta_recv(skb) != BCME_OK) {
			PKTFRMNATIVE(dhdp->osh, skb);
			PKTFREE(dhdp->osh, skb, FALSE);
			return BCME_OK;
		}
	}
#endif /* DHD_DPSTA */

	skb->protocol = eth_type_trans(skb, skb->dev);

	if (in_interrupt()) {
		bcm_object_trace_opr(skb, BCM_OBJDBG_REMOVE,
			__FUNCTION__, __LINE__);
		netif_rx(skb);
	} else {
		if (dhdp->info->rxthread_enabled) {
			if (!skbhead) {
				skbhead = skb;
			} else {
				PKTSETNEXT(dhdp->osh, skbprev, skb);
			}
			skbprev = skb;
		} else {
			/* If the receive is not processed inside an ISR,
			 * the softirqd must be woken explicitly to service
			 * the NET_RX_SOFTIRQ.	In 2.6 kernels, this is handled
			 * by netif_rx_ni(), but in earlier kernels, we need
			 * to do it manually.
			 */
			bcm_object_trace_opr(skb, BCM_OBJDBG_REMOVE,
				__FUNCTION__, __LINE__);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0)
			netif_rx_ni(skb);
#else
			ulong flags;
			netif_rx(skb);
			local_irq_save(flags);
			RAISE_RX_SOFTIRQ();
			local_irq_restore(flags);
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0) */
		}
	}

	if (dhdp->info->rxthread_enabled && skbhead)
		dhd_sched_rxf(dhdp, skbhead);

	return BCME_OK;
}

/**
 * zero pad runt (short) ethernet packets
 *
 * return the padded bytes
 */
int
dhd_shortpktpad(dhd_pub_t *dhdp, void* pktbuf)
{
	int    pad_size = 0;

#ifdef BCM_ROUTER_DHD
	int    pktlen;
	int    minlen;

	/* Noting to do if disabled by nvram */
	if (dhdp->shortpktpad == FALSE) {
		return 0;
	}

	minlen = (ETHER_MIN_LEN - ETHER_CRC_LEN);
	if (DHDHDR_SUPPORT(dhdp)) {
		/* Later LLCSNAP header will be added to the packet */
		minlen -= DOT11_LLC_SNAP_HDR_LEN;
	}

	pktlen = PKTLEN(dhdp->osh, pktbuf);
	if (pktlen < minlen) {
		/* Short packet, Pad and Adjust the length */
		uint8 *pktdata = (uint8 *) PKTDATA(dhdp->osh, pktbuf);
		uint8 *pad_ptr = pktdata + pktlen;
		uint16 hdrlen  = ntoh16(((struct ether_header *)pktdata)->ether_type);

		/* zero out the padded bytes */
		pad_size = minlen - pktlen;
		bzero((void *)pad_ptr, pad_size);

		/* Adjust the packet length */
		PKTSETLEN(dhdp->osh, pktbuf, minlen);

		/* Update length in the sub frame */
		if (hdrlen <= ETHER_TYPE_MIN) {
			hdrlen += pad_size;
			((struct ether_header *)pktdata)->ether_type = hton16(hdrlen);
		}

		/* Cache flush */
		DMA_MAP(dhdp->osh, pktdata, minlen, DMA_TX, NULL, NULL);
	}
#endif /* BCM_ROUTER_DHD */

	return pad_size;
}

int BCMFASTPATH
dhd_sendpkt(dhd_pub_t *dhdp, int ifidx, void *pktbuf)
{
	int ret = BCME_OK;
	dhd_info_t *dhd = (dhd_info_t *)(dhdp->info);
	struct ether_header *eh = NULL;
#if defined(BCM_BLOG)
	/* for wmf unicast, we want to skip blog operation to prevent flow */
	int b_wmf_unicast = DHD_PKT_GET_WMF_UCAST(pktbuf);
#endif
#if (defined(DHD_L2_FILTER) || (defined(BCM_ROUTER_DHD) && defined(QOS_MAP_SET)))
	dhd_if_t *ifp = dhd_get_ifp(dhdp, ifidx);

	/* Reject if interface is not found */
	if (ifp == NULL) {
		DHD_ERROR(("%s: ifp is NULL\n", __FUNCTION__));
		/* free the packet here since the caller won't */
		PKTCFREE(dhdp->osh, pktbuf, TRUE);
		return -ENODEV;
	}
#endif /* DHD_L2_FILTER || (BCM_ROUTER_DHD && QOS_MAP_SET) */

#if defined(BCM_BLOG)
	/* clear the wmf_ucast tag */
	DHD_PKT_CLR_WMF_UCAST(pktbuf);
#endif

	/* Reject if down */
	if (!dhdp->up || (dhdp->busstate == DHD_BUS_DOWN)) {
		/* free the packet here since the caller won't */
		PKTCFREE(dhdp->osh, pktbuf, TRUE);
		return -ENODEV;
	}
#ifdef BCM_ROUTER_DHD
	/* Reject if bsscfg is not up */
	if (((dhd_flow_rings_ifindex2role(dhdp, ifidx) == WLC_E_IF_ROLE_AP) ||
		(dhd_flow_rings_ifindex2role(dhdp, ifidx) == WLC_E_IF_ROLE_STA)) &&
		!ifp->bss_up) {
		/* free the packet here since the caller won't */
		PKTCFREE(dhdp->osh, pktbuf, TRUE);
		return -ENODEV;
	}
#endif /* BCM_ROUTER_DHD */

#ifdef PCIE_FULL_DONGLE
	if (dhdp->busstate == DHD_BUS_SUSPENDING || dhdp->busstate == DHD_BUS_SUSPENDED) {
		DHD_ERROR(("%s : pcie is still in suspend state!!\n", __FUNCTION__));
		PKTFREE(dhdp->osh, pktbuf, TRUE);
		return -EBUSY;
	}
#endif /* PCIE_FULL_DONGLE */

#ifdef DHD_L2_FILTER
	/* if dhcp_unicast is enabled, we need to convert the */
	/* broadcast DHCP ACK/REPLY packets to Unicast. */
	if (ifp->dhcp_unicast) {
	    uint8* mac_addr;
	    uint8* ehptr = NULL;
	    int ret;
	    ret = bcm_l2_filter_get_mac_addr_dhcp_pkt(dhdp->osh, pktbuf, ifidx, &mac_addr);
	    if (ret == BCME_OK) {
		/*  if given mac address having valid entry in sta list
		 *  copy the given mac address, and return with BCME_OK
		*/
		if (dhd_find_sta(dhdp, ifidx, mac_addr)) {
		    ehptr = PKTDATA(dhdp->osh, pktbuf);
		    bcopy(mac_addr, ehptr + ETHER_DEST_OFFSET, ETHER_ADDR_LEN);
		}
	    }
	}

	if (ifp->grat_arp && DHD_IF_ROLE_AP(dhdp, ifidx)) {
	    if (bcm_l2_filter_gratuitous_arp(dhdp->osh, pktbuf) == BCME_OK) {
			PKTCFREE(dhdp->osh, pktbuf, TRUE);
			return BCME_ERROR;
	    }
	}

	if (ifp->parp_enable && DHD_IF_ROLE_AP(dhdp, ifidx)) {
		ret = dhd_l2_filter_pkt_handle(dhdp, ifidx, pktbuf, TRUE);

		/* Drop the packets if l2 filter has processed it already
		 * otherwise continue with the normal path
		 */
		if (ret == BCME_OK) {
			PKTCFREE(dhdp->osh, pktbuf, TRUE);
			return BCME_ERROR;
		}
	}
#endif /* DHD_L2_FILTER */
	/* Update multicast statistic */
	if (PKTLEN(dhdp->osh, pktbuf) >= ETHER_HDR_LEN) {
		uint8 *pktdata = (uint8 *)PKTDATA(dhdp->osh, pktbuf);
		eh = (struct ether_header *)pktdata;

		if (ETHER_ISMULTI(eh->ether_dhost))
			dhdp->tx_multicast++;
		if (ntoh16(eh->ether_type) == ETHER_TYPE_802_1X) {
			PKTSETPRIO(pktbuf, PRIO_8021D_VI);
			atomic_inc(&dhd->pend_8021x_cnt);
		}
#if defined(BCMDBG) || defined(DHD_DEBUG)
		if (ntoh16(eh->ether_type) == ETHER_TYPE_IP &&
			DHD_DHCP_DUMP_ON()) {
			dhd_dhcp_dump(pktdata, TRUE);
		}
#endif /* defined(BCMDBG) || defined(DHD_DEBUG) */
	} else {
			PKTCFREE(dhdp->osh, pktbuf, TRUE);
			return BCME_ERROR;
	}

#ifdef BCM_NBUFF_WLMCAST
	if (!b_wmf_unicast && IS_SKBUFF_PTR(pktbuf)) {
#endif /* BCM_NBUFF_WLMCAST */

#if defined(QOS_MAP_SET) && defined(BCM_ROUTER_DHD)
	if (ifp->qosmap_up_table_enable) {
		pktsetprio_qms(pktbuf, ifp->qosmap_up_table, FALSE);
	}
	else
#endif
	{
		/* Look into the packet and update the packet priority */
#ifndef PKTPRIO_OVERRIDE
		if (PKTPRIO(pktbuf) == 0)
#endif /* !CUSTOMER_HW4 */
		{
			pktsetprio(pktbuf, FALSE);
		}
	}
#ifdef BCM_NBUFF_WLMCAST
	}
#endif /* BCM_NBUFF_WLMCAST */

#if defined(BCM_ROUTER_DHD) || defined(TRAFFIC_MGMT_DWM)
	traffic_mgmt_pkt_set_prio(dhdp, pktbuf);

#ifdef BCM_GMAC3
	DHD_PKT_SET_DATAOFF(pktbuf, 0);
#endif /* BCM_GMAC3 */
#endif /* BCM_ROUTER_DHD || TRAFFIC_MGMT_DWM */

#ifdef PCIE_FULL_DONGLE
	/*
	 * Lkup the per interface hash table, for a matching flowring. If one is not
	 * available, allocate a unique flowid and add a flowring entry.
	 * The found or newly created flowid is placed into the pktbuf's tag.
	 */

#if defined(BCM_WFD)
	ret = dhd_handle_wfd_blog(dhdp, ifp->net, ifidx, pktbuf, b_wmf_unicast);
	if (ret != BCME_OK)
		return ret;
#else
	ret = dhd_flowid_update(dhdp, ifidx, dhdp->flow_prio_map[(PKTPRIO(pktbuf))], pktbuf);
	if (ret != BCME_OK) {
#if defined(BCM_BLOG_UNKNOWN_UCAST)
		if (DHD_IF_ROLE_AP(dhdp, ifidx) && (ret == BCME_NOTASSOCIATED)) {
			/* Notify blog of dropping unknown UCAST packets in acceleration */
			dhd_handle_blog_emit_drop(dhdp, ifp->net, ifidx, pktbuf);
		}
#endif /* BCM_BLOG_UNKNOWN_UCAST */
		PKTCFREE(dhd->pub.osh, pktbuf, TRUE);
		return ret;
	}
#if defined(BCM_BLOG)
	ret = dhd_handle_blog_emit(dhdp, ifp->net, ifidx, pktbuf, b_wmf_unicast);
	if (ret != BCME_OK)
		return ret;
#endif /* BCM_BLOG */
#endif /* BCM_WFD */

#endif /* PCIE_FULL_DONGLE */

#if defined(DHD_LBR_AGGR_BCM_ROUTER)
	if (dhdp->lbr_aggr_en_mask && dhd_sendpkt_lbr_aggr_intercept(dhdp, ifidx, pktbuf)) {
		/*
		 * if packet is intercepted by dhd_sendpkt_lbr_aggr_intercept
		 * dhd_sendpkt_resume_intercept is called to resume/complete packet send
		 */
		return BCME_OK;
	}
#endif  /* DHD_LBR_AGGR_BCM_ROUTER */

#ifdef PROP_TXSTATUS
	if (dhd_wlfc_is_supported(dhdp)) {
		/* store the interface ID */
		DHD_PKTTAG_SETIF(PKTTAG(pktbuf), ifidx);

		/* store destination MAC in the tag as well */
		DHD_PKTTAG_SETDSTN(PKTTAG(pktbuf), eh->ether_dhost);

		/* decide which FIFO this packet belongs to */
		if (ETHER_ISMULTI(eh->ether_dhost))
			/* one additional queue index (highest AC + 1) is used for bc/mc queue */
			DHD_PKTTAG_SETFIFO(PKTTAG(pktbuf), AC_COUNT);
		else
			DHD_PKTTAG_SETFIFO(PKTTAG(pktbuf), WME_PRIO2AC(PKTPRIO(pktbuf)));
	} else
#endif /* PROP_TXSTATUS */
	{
		/* If the protocol uses a data header, apply it */
		dhd_prot_hdrpush(dhdp, ifidx, pktbuf);
	}

	/* Use bus module to send data frame */
#ifdef WLMEDIA_HTSF
	dhd_htsf_addtxts(dhdp, pktbuf);
#endif
#if defined(DHD_8021X_DUMP)
	dhd_tx_dump(dhdp->osh, pktbuf);
#endif /* DHD_8021X_DUMP */
#if defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE)
	ret = dhd_spdsvc_tx(dhdp, &pktbuf);
	if ((ret == BCME_ERROR) || (ret == BCME_OK))
		return ret;
#endif

#ifdef BCMDBUS
#ifdef PROP_TXSTATUS
	if (dhd_wlfc_commit_packets(dhdp, (f_commitpkt_t)dhd_dbus_txdata,
		dhdp, pktbuf, TRUE) == WLFC_UNSUPPORTED) {
		/* non-proptxstatus way */
		ret = dhd_dbus_txdata(dhdp, pktbuf);
	}
#else
	ret = dhd_dbus_txdata(dhdp, pktbuf);
#endif /* PROP_TXSTATUS */
	if (ret)
		PKTCFREE(dhdp->osh, pktbuf, TRUE);
#else
#ifdef PROP_TXSTATUS
	{
		if (dhd_wlfc_commit_packets(dhdp, (f_commitpkt_t)dhd_bus_txdata,
			dhdp->bus, pktbuf, TRUE) == WLFC_UNSUPPORTED) {
			/* non-proptxstatus way */
#ifdef BCMPCIE
			ret = dhd_bus_txdata(dhdp->bus, pktbuf, (uint8)ifidx);
#else
			ret = dhd_bus_txdata(dhdp->bus, pktbuf);
#endif /* BCMPCIE */
		}
	}
#else
#ifdef BCMPCIE
	ret = dhd_bus_txdata(dhdp->bus, pktbuf, (uint8)ifidx);
#else
	ret = dhd_bus_txdata(dhdp->bus, pktbuf);
#endif /* BCMPCIE */
#endif /* PROP_TXSTATUS */

#endif /* BCMDBUS */

	return ret;
}

#if defined(PCIE_FULL_DONGLE)
/* With HOSTAPD, 802.1X (EAPOL) frames of WDS/DWDS client received on AP interface will be
 * directed to corresponding WDS interface.
 */
static int
dhd_wl_check_wds_destn(dhd_info_t *dhd, int *wds_ifidx,
	struct net_device **wds_net, void *pktbuf)
{
	struct ether_header *eh;
	dhd_if_t *ifp;

	eh = (struct ether_header *)PKTDATA(dhd->pub.osh, pktbuf);

	if ((eh->ether_type == hton16(ETHER_TYPE_802_1X)) &&
		!ETHER_ISMULTI(eh->ether_dhost)) {

		ifp = dhd_get_wds_ifp_by_mac(&dhd->pub, eh->ether_dhost);

		if ((ifp) && (ifp->wds_role == WLC_E_IF_FLAGS_WDS_AP)) {
			*wds_net = ifp->net;
			*wds_ifidx = DHD_DEV_IFIDX(ifp->net);
			DHD_INFO(("%s: Switched interface index %d for MAC["MACDBG"]\n",
				__FUNCTION__, *wds_ifidx, MAC2STRDBG(eh->ether_dhost)));
			return BCME_OK;
		}
	}

	return BCME_ERROR;
}
#endif /* PCIE_FULL_DONGLE */

#if defined(DHD_IGMP_UCQUERY) || defined(DHD_UCAST_UPNP)
static bool BCMFASTPATH
dhd_need_wmf_ucast_convert(dhd_pub_t *dhdp, struct ether_header *eh)
{
	bool ucast_convert = FALSE;
	uint8 *iph = (uint8 *)eh + ETHER_HDR_LEN;
#ifdef DHD_UCAST_UPNP
	uint32 dest_ip;

#ifdef BCM_NBUFF_WLMCAST_IPV6
	if (ntoh16(eh->ether_type) == ETHER_TYPE_IP)
#endif
	{
		dest_ip = ntoh32(*((uint32 *)(iph + IPV4_DEST_IP_OFFSET)));
		ucast_convert = dhdp->wmf_ucast_upnp && MCAST_ADDR_UPNP_SSDP(dest_ip);
	}
#endif /* DHD_UCAST_UPNP */

#ifdef DHD_IGMP_UCQUERY
	ucast_convert |= dhdp->wmf_ucast_igmp_query &&
#ifdef BCM_NBUFF_WLMCAST_IPV6
		(IP_VER(iph) == IP_VER_4) &&
#endif
		(IPV4_PROT(iph) == IP_PROT_IGMP) &&
		(*(iph + IPV4_HLEN(iph)) == IGMPV2_HOST_MEMBERSHIP_QUERY);
#endif /* DHD_IGMP_UCQUERY */

	return ucast_convert;
}
#endif /* DHD_IGMP_UCQUERY || DHD_UCAST_UPNP */

#if defined(BCM_ROUTER_DHD) && defined(BCM_GMAC3)
static int BCMFASTPATH
dhd_start_xmit_try(struct sk_buff *skb, struct net_device *net, bool lock_taken)
#else /* ! (BCM_ROUTER_DHD && BCM_GMAC3) */
int BCMFASTPATH
dhd_start_xmit(struct sk_buff *skb, struct net_device *net)
#endif /* BCM_ROUTER_DHD && BCM_GMAC3 */
{
	int ret = 0;
	uint datalen;
	void *pktbuf;
	dhd_info_t *dhd = DHD_DEV_INFO(net);
	dhd_if_t *ifp = NULL;
	int ifidx;
	unsigned long flags;
#ifdef WLMEDIA_HTSF
	uint8 htsfdlystat_sz = dhd->pub.htsfdlystat_sz;
#elif !defined(BCM_ROUTER_DHD)
	uint8 htsfdlystat_sz = 0;
#endif /* ! WLMEDIA_HTSF && ! BCM_ROUTER_DHD */
#ifdef DHD_WMF
	struct ether_header *eh;
	uint8 *iph;
#endif /* DHD_WMF */
#ifdef BCM_CPEROUTER_EXTSTATS
	uint8 is_multicast_pkt;
	uint8 is_broadcast_pkt;
#endif
#ifdef DHD_WMF
	uint16 ether_type;
#endif

	DHD_TRACE(("%s: Enter\n", __FUNCTION__));

#ifdef PCIE_FULL_DONGLE
	DHD_GENERAL_LOCK(&dhd->pub, flags);
	dhd->pub.tx_in_progress = 1;
#ifdef BCM_NBUFF_WLMCAST
	if (dhd->pub.busstate == DHD_BUS_SUSPENDING || dhd->pub.busstate == DHD_BUS_SUSPENDED ||
		PKTATTACHTAG(dhd->pub.osh, skb)) {
#else
	if (dhd->pub.busstate == DHD_BUS_SUSPENDING || dhd->pub.busstate == DHD_BUS_SUSPENDED) {
#endif
		DHD_ERROR(("%s : pcie is still in suspend state!!\n", __FUNCTION__));
		dev_kfree_skb(skb);
		ifp = DHD_DEV_IFP(net);
		ifp->stats.tx_dropped++;
		dhd->pub.tx_dropped++;
		dhd->pub.tx_in_progress = 0;
		DHD_GENERAL_UNLOCK(&dhd->pub, flags);
		return -EBUSY;
	}
	DHD_GENERAL_UNLOCK(&dhd->pub, flags);
#endif /* PCIE_FULL_DONGLE */

	DHD_OS_WAKE_LOCK(&dhd->pub);
	DHD_PERIM_LOCK_TRY(DHD_FWDER_UNIT(dhd), lock_taken);

	DHD_GENERAL_LOCK(&dhd->pub, flags);
	/* Reject if down */
	/* XXX kernel panic issue when first bootup time,
	 *	 rmmod without interface down make unnecessary hang event.
	 */
	if (dhd->pub.busstate == DHD_BUS_DOWN || dhd->pub.hang_was_sent) {
		DHD_INFO(("%s: xmit rejected pub.up=%d busstate=%d \n",
			__FUNCTION__, dhd->pub.up, dhd->pub.busstate));
		netif_stop_queue(net);
#if defined(OEM_ANDROID)
		/* Send Event when bus down detected during data session */
		if (dhd->pub.up) {
			DHD_ERROR(("%s: Event HANG sent up\n", __FUNCTION__));
			net_os_send_hang_message(net);
		}
#endif /* OEM_ANDROID */
#ifdef PCIE_FULL_DONGLE
		dhd->pub.tx_in_progress = 0;
#endif /* PCIE_FULL_DONGLE */
		DHD_GENERAL_UNLOCK(&dhd->pub, flags);
		DHD_PERIM_UNLOCK_TRY(DHD_FWDER_UNIT(dhd), lock_taken);
		DHD_OS_WAKE_UNLOCK(&dhd->pub);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 20))
		return -ENODEV;
#else
		return NETDEV_TX_BUSY;
#endif
	}
	DHD_GENERAL_UNLOCK(&dhd->pub, flags);

	ifp = DHD_DEV_IFP(net);
	ifidx = DHD_DEV_IFIDX(net);
	BUZZZ_LOG(START_XMIT_BGN, 2, (uint32)ifidx, (uintptr)skb);

	if (ifidx == DHD_BAD_IF) {
		DHD_ERROR(("%s: bad ifidx %d\n", __FUNCTION__, ifidx));
		netif_stop_queue(net);
#ifdef PCIE_FULL_DONGLE
		DHD_GENERAL_LOCK(&dhd->pub, flags);
		dhd->pub.tx_in_progress = 0;
		DHD_GENERAL_UNLOCK(&dhd->pub, flags);
#endif /* PCIE_FULL_DONGLE */
		DHD_PERIM_UNLOCK_TRY(DHD_FWDER_UNIT(dhd), lock_taken);
		DHD_OS_WAKE_UNLOCK(&dhd->pub);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 20))
		return -ENODEV;
#else
		return NETDEV_TX_BUSY;
#endif
	}

	ASSERT(ifidx == dhd_net2idx(dhd, net));
	ASSERT((ifp != NULL) && ((ifidx < DHD_MAX_IFS) && (ifp == dhd->iflist[ifidx])));
#ifdef BCM_CPEROUTER_EXTSTATS
	is_multicast_pkt = ETHER_ISMULTI((struct ether_header *)PKTDATA(dhd->pub.osh, skb));
	is_broadcast_pkt = ETHER_ISBCAST((struct ether_header *)PKTDATA(dhd->pub.osh, skb));
#endif
#if defined(CUSTOMER_HW4)
	if (!((dhd->pub.op_mode & DHD_FLAG_HOSTAP_MODE) ||
		(dhd->pub.op_mode & DHD_FLAG_IBSS_MODE) ||
		(dhd->pub.op_mode & DHD_FLAG_MFG_MODE))) {
		if (wl_cfg80211_is_primary_device(ifp->net) &&
			wl_cfg80211_is_connected(ifp->net) == 0) {
			ret = -ENODEV;
			goto done;
		}
	}
#endif /* CUSTOMER_HW4 */

	bcm_object_trace_opr(skb, BCM_OBJDBG_ADD_PKT, __FUNCTION__, __LINE__);

#if defined(BCM_DHDHDR) && defined(PCIE_FULL_DONGLE)
#if defined(BCM_NBUFF)
	if (IS_SKBUFF_PTR(skb))
#endif /* BCM_NBUFF */
	if ((DHDHDR_SUPPORT(&dhd->pub)) && (skb_headroom(skb) < DOT11_LLC_SNAP_HDR_LEN)) {
		struct sk_buff *nskb;

		DHD_INFO(("DHDHDR %s: insufficient headroom\n",
		          dhd_ifname(&dhd->pub, ifidx)));

		dhd->pub.tx_realloc++;
		nskb = skb_realloc_headroom(skb, DOT11_LLC_SNAP_HDR_LEN);
		dev_kfree_skb(skb);

		if ((skb = nskb) == NULL) {
			DHD_ERROR(("%s: skb_realloc_headroom failed\n",
			           dhd_ifname(&dhd->pub, ifidx)));
			ret = -ENOMEM;
			goto done;
		}
	}
#endif /* BCM_DHDHDR && PCIE_FULL_DONGLE */

#if !defined(BCM_NBUFF)
	/* re-align socket buffer if "skb->data" is odd address */
	if (((unsigned long)(skb->data)) & 0x1) {
		unsigned char *data = skb->data;
		uint32 length = skb->len;
		PKTPUSH(dhd->pub.osh, skb, 1);
		memmove(skb->data, data, length);
		PKTSETLEN(dhd->pub.osh, skb, length);
	}
#endif

	datalen  = PKTLEN(dhd->pub.osh, skb);

	/* Make sure there's enough room for any header */
#if !defined(BCM_ROUTER_DHD)
	if (skb_headroom(skb) < dhd->pub.hdrlen + htsfdlystat_sz) {
		struct sk_buff *skb2;

		DHD_INFO(("%s: insufficient headroom\n",
		          dhd_ifname(&dhd->pub, ifidx)));
		dhd->pub.tx_realloc++;

		bcm_object_trace_opr(skb, BCM_OBJDBG_REMOVE, __FUNCTION__, __LINE__);
		skb2 = skb_realloc_headroom(skb, dhd->pub.hdrlen + htsfdlystat_sz);

		dev_kfree_skb(skb);
		if ((skb = skb2) == NULL) {
			DHD_ERROR(("%s: skb_realloc_headroom failed\n",
			           dhd_ifname(&dhd->pub, ifidx)));
			ret = -ENOMEM;
			goto done;
		}
		bcm_object_trace_opr(skb, BCM_OBJDBG_ADD_PKT, __FUNCTION__, __LINE__);
	}
#endif /* !BCM_ROUTER_DHD */

#ifdef BCM_NBUFF
#ifdef BCM_NBUFF_WLMCAST
	pktbuf = PKTFRMNATIVE(dhd->pub.osh, skb);
	DHD_PKT_CLR_DATA_DHDHDR(pktbuf);
	DHD_PKT_CLR_FKBPOOL(pktbuf);
#ifdef BCM_WLAN_PER_CLIENT_FLOW_LEARNING
	if (IS_FKBUFF_PTR(pktbuf))
		DHD_PKT_CLR_FKB_FLOW_UNHANDLED(pktbuf);
#endif
#else
	/* For multicast packets we may do pktdup or pktcpy
	 * which is not supported by fkb. Convert fkb to skb
	 */
	{
		struct sk_buff *nskb;
		struct ether_header *eh;

		pktbuf = PKTFRMNATIVE(dhd->pub.osh, skb);
		eh = (struct ether_header *)PKTDATA(dhd->pub.osh, pktbuf);
		if (ETHER_ISMULTI(eh->ether_dhost)) {
			/* if fkb, convert to skb */
			nskb = nbuff_xlate((pNBuff_t)pktbuf);
			if (nskb == NULL) {
				PKTFREE(dhd->pub.osh, pktbuf, TRUE);
				ret =  -ENOMEM;
				DHD_ERROR(("%s: nbuff_xlate failed for a multicast packet\n",
					dhd_ifname(&dhd->pub, ifidx)));
				goto done;
			}
			pktbuf = nskb;
		}
	}
#endif /* BCM_NBUFF_WLMCAST */
#else
	/* Convert to packet */
	if (!(pktbuf = PKTFRMNATIVE(dhd->pub.osh, skb))) {
		DHD_ERROR(("%s: PKTFRMNATIVE failed\n",
		           dhd_ifname(&dhd->pub, ifidx)));
		bcm_object_trace_opr(skb, BCM_OBJDBG_REMOVE, __FUNCTION__, __LINE__);
		dev_kfree_skb_any(skb);
		ret = -ENOMEM;
		goto done;
	}
#endif /* BCM_NBUFF */

#if defined(PCIE_FULL_DONGLE)
	/* Check EAPOL, role WDS */
#if defined(BCM_NBUFF)
	if (IS_SKBUFF_PTR(pktbuf))
#endif /* BCM_NBUFF */
	{
		ret = dhd_wl_check_wds_destn(dhd, &ifidx, &net, pktbuf);
	}
#endif /* PCIE_FULL_DONGLE */

#if defined(BCM_DHDHDR) && defined(PCIE_FULL_DONGLE)
	/* Multicast packets from CPU may share the same data, we need to make a copy
	 * for it if DHDHDR_SUPPORT is enabled.  In GMAC3 DHDAP lock_taken implies the packet
	 * is from CPU
	 */
	if (DHDHDR_SUPPORT(&dhd->pub) &&
#if defined(BCM_ROUTER_DHD) && defined(BCM_GMAC3)
		(lock_taken == DHD_PERIM_LOCK_NOTTAKEN) &&
#endif
		TRUE) {
		void *npkt;
		struct ether_header *eh;
		bool need_pktcopy = FALSE;

		eh = (struct ether_header *)PKTDATA(dhd->pub.osh, pktbuf);
		need_pktcopy = ETHER_ISMULTI(eh->ether_dhost);
#ifdef BCM_NBUFF
		if (IS_SKBUFF_PTR(pktbuf))
#endif /* BCM_NBUFF */
		{
			/* XXX, Non router platforms ignore the need_pktcopy for PKTSHARED
			 * packets.  TCP packets from BUPC to AP has this bit set and need_pktcopy
			 * cause throughput drop
			*/
			need_pktcopy |= (PKTSHARED(pktbuf) &&
#if !defined(BCM_ROUTER_DHD)
				(ntoh16(eh->ether_type) == ETHER_TYPE_802_1X) &&
#endif /* BCM_ROUTER_DHD */
				TRUE);
		}

		if (need_pktcopy) {
			npkt = PKTDUP_CPY(dhd->pub.osh, pktbuf);
			if (npkt == NULL) {
				PKTFREE(dhd->pub.osh, pktbuf, TRUE);
				ret =  -ENOMEM;
				DHD_ERROR(("%s: PKTDUP_CPY failed for a CPU multicast packet\n",
					dhd_ifname(&dhd->pub, ifidx)));
				goto done;
			}
#ifdef BCM_NBUFF
			/* if incoming FKB and out is skb, then fkb already freed, not
			 * need to free again, causing crash otherwize for double free
			 */
			if (!(IS_FKBUFF_PTR(pktbuf) && IS_SKBUFF_PTR(npkt)))
#endif
			{
				/* Free original one */
				PKTFREE(dhd->pub.osh, pktbuf, TRUE);
			}
			pktbuf = npkt;
		}
	}
#endif /* BCM_DHDHDR && PCIE_FULL_DONGLE */

#if defined(WLMEDIA_HTSF) && !defined(BCM_ROUTER_DHD)
	if (htsfdlystat_sz && PKTLEN(dhd->pub.osh, pktbuf) >= ETHER_ADDR_LEN) {
		uint8 *pktdata = (uint8 *)PKTDATA(dhd->pub.osh, pktbuf);
		struct ether_header *eh = (struct ether_header *)pktdata;

		if (!ETHER_ISMULTI(eh->ether_dhost) &&
			(ntoh16(eh->ether_type) == ETHER_TYPE_IP)) {
			eh->ether_type = hton16(ETHER_TYPE_BRCM_PKTDLYSTATS);
		}
	}
#endif /* WLMEDIA_HTSF && ! BCM_ROUTER_DHD */
#ifdef DHD_WET
	/* wet related packet proto manipulation should be done in DHD
	 * since dongle doesn't have complete payload
	 */
	if (WET_ENABLED(&dhd->pub) &&
#ifdef BCM_ROUTER_DHD
		(ifp->primsta_dwds == FALSE) &&
#endif /* BCM_ROUTER_DHD */
		DHD_IF_ROLE_STA(&dhd->pub, ifidx)) {
		if (dhd_wet_send_proc(dhd->pub.wet_info, ifidx,
				pktbuf, &pktbuf) < 0) {
			DHD_INFO(("%s:%s: wet send proc failed\n",
				__FUNCTION__, dhd_ifname(&dhd->pub, ifidx)));
			PKTFREE(dhd->pub.osh, pktbuf, FALSE);
			ret =  -EFAULT;
			goto done;
		}
	}
#endif /* DHD_WET */

#ifdef BCM_NBUFF
	/* Look into and update the pkt prio. Must be done before dhd_handle_wfd_blog() */
	if (IS_SKBUFF_PTR(pktbuf)) {
#ifndef PKTPRIO_OVERRIDE
		if (PKTPRIO(pktbuf) == 0)
#endif
			pktsetprio(pktbuf, FALSE);
	}
#endif /* BCM_NBUFF */

#ifdef DHD_WMF
	eh = (struct ether_header *)PKTDATA(dhd->pub.osh, pktbuf);
	iph = (uint8 *)eh + ETHER_HDR_LEN;
	ether_type = ntoh16(eh->ether_type);

	/* WMF processing for multicast packets
	 * Only IPv4 packets are handled
	 */
	if (ifp->wmf.wmf_enable && (((ether_type == ETHER_TYPE_IP) &&
		(IP_VER(iph) == IP_VER_4) && (ETHER_ISMULTI(eh->ether_dhost) ||
		((IPV4_PROT(iph) == IP_PROT_IGMP) && dhd->pub.wmf_ucast_igmp))) ||
#ifdef BCM_NBUFF_WLMCAST_IPV6
		(ether_type == ETHER_TYPE_IPV6 && IP_VER(iph) == IP_VER_6 &&
		(ETHER_ISMULTI(eh->ether_dhost))) ||
#endif
		FALSE)) {

#if defined(DHD_IGMP_UCQUERY) || defined(DHD_UCAST_UPNP)
		void *sdu_clone;
		bool ucast_convert = dhd_need_wmf_ucast_convert(&(dhd->pub), eh);
#ifdef BCM_WFD
		/* Handle BLOG before WMF is handled to enable FC entry be generated
		 * by mutlicast stream
		 */
#ifndef BCM_WLAN_PER_CLIENT_FLOW_LEARNING
		if (dhd_handle_wfd_blog(&(dhd->pub), net, ifidx, pktbuf, 0)) {
			ret = -EFAULT;
			goto done;
		}
#ifdef BCM_NBUFF
		if (IS_SKBUFF_PTR(pktbuf)) {
			DHD_PKT_SET_SKB_FLOW_HANDLED(pktbuf);
		}
#endif /* BCM_NBUFF */
#endif /* BCM_WLAN_PER_CLIENT_FLOW_LEARNING */
#endif /* BCM_WFD */

		if (ucast_convert) {
			dhd_sta_t *sta;
#ifdef PCIE_FULL_DONGLE
			unsigned long flags;
#endif
			struct list_head snapshot_list;
			struct list_head *wmf_ucforward_list;

			ret = NETDEV_TX_OK;

			/* For non BCM_GMAC3 platform we need a snapshot sta_list to
			 * resolve double DHD_IF_STA_LIST_LOCK call deadlock issue.
			 */
			wmf_ucforward_list = DHD_IF_WMF_UCFORWARD_LOCK(dhd, ifp, &snapshot_list);

			/* Convert upnp/igmp query to unicast for each assoc STA */
			list_for_each_entry(sta, wmf_ucforward_list, list) {
				struct ether_header *pktbuf_eh;
				/* Skip sending to
				 * - Proxy interfaces of proxySTA
				 * - Clients with WDS interface
				 */
				if (sta->psta_prim != NULL && !ifp->wmf_psta_disable)
					continue;

				/* Skip sending the frame back to the source */
				pktbuf_eh = (struct ether_header *) PKTDATA(dhd->pub.osh, pktbuf);
				if (eacmp(sta->ea.octet, pktbuf_eh->ether_shost) == 0) {
					DHD_INFO(("%s: Skipping unicast conversion to "MACDBG"\n",
							__FUNCTION__, MAC2STRDBG(&sta->ea)));
					continue;
				}

				sdu_clone = PKTDUP_CPY(dhd->pub.osh, pktbuf);
				if (sdu_clone == NULL) {
					ret = WMF_NOP;
					break;
				}
				dhd_wmf_forward(ifp->wmf.wmfh, sdu_clone, 0, sta, 1);
			}
			DHD_IF_WMF_UCFORWARD_UNLOCK(dhd, wmf_ucforward_list);

#ifdef PCIE_FULL_DONGLE
			DHD_GENERAL_LOCK(&dhd->pub, flags);
			dhd->pub.tx_in_progress = 0;
			DHD_GENERAL_UNLOCK(&dhd->pub, flags);
#endif /* PCIE_FULL_DONGLE */
			DHD_PERIM_UNLOCK_TRY(DHD_FWDER_UNIT(dhd), lock_taken);
			DHD_OS_WAKE_UNLOCK(&dhd->pub);

			if (ret == NETDEV_TX_OK)
				PKTFREE(dhd->pub.osh, pktbuf, TRUE);

			return ret;
		} else
#endif /* defined(DHD_IGMP_UCQUERY) || defined(DHD_UCAST_UPNP) */
		{
			/* There will be no STA info if the packet is coming from LAN host
			 * Pass as NULL
			 */
			ret = dhd_wmf_packets_handle(&dhd->pub, pktbuf, NULL, ifidx, 0);
			switch (ret) {
			case WMF_DROP:
				/* Packet DROP decision by WMF. Toss it */
				DHD_INFO(("%s: WMF decides to drop packet 0\n", __FUNCTION__));
				PKTFREE(dhd->pub.osh, pktbuf, TRUE);
				/* Fall through */
			case WMF_TAKEN:
				/* Either taken by WMF or we should drop it.
				 * Exiting send path
				 */
#ifdef PCIE_FULL_DONGLE
				DHD_GENERAL_LOCK(&dhd->pub, flags);
				dhd->pub.tx_in_progress = 0;
				DHD_GENERAL_UNLOCK(&dhd->pub, flags);
#endif /* PCIE_FULL_DONGLE */
				DHD_PERIM_UNLOCK_TRY(DHD_FWDER_UNIT(dhd), lock_taken);
				DHD_OS_WAKE_UNLOCK(&dhd->pub);
				return NETDEV_TX_OK;
			default:
				/* Continue the transmit path */
				break;
			}
		}
	}
#endif /* DHD_WMF */
#ifdef DHD_PSTA
	/* PSR related packet proto manipulation should be done in DHD
	 * since dongle doesn't have complete payload
	 */
	if (PSR_ENABLED(&dhd->pub) &&
#ifdef BCM_ROUTER_DHD
		!(ifp->primsta_dwds) &&
#endif /* BCM_ROUTER_DHD */
		1) {
#ifdef BCM_BLOG
#ifdef BCM_NBUFF
		if (IS_SKBUFF_PTR(pktbuf))
#endif /* BCM_NBUFF */
		DHD_PKT_CLR_SKB_SKIP_BLOG(pktbuf);
#endif /* BCM_BLOG */

		if (dhd_psta_proc(&dhd->pub, ifidx, &pktbuf, TRUE) < 0) {

			DHD_ERROR(("%s:%s: psta send proc failed\n", __FUNCTION__,
				dhd_ifname(&dhd->pub, ifidx)));
		}
#ifdef BCM_BLOG
		else {
#ifdef BCM_NBUFF
			if (IS_SKBUFF_PTR(pktbuf))
#endif /* BCM_NBUFF */
			if (DHD_PKT_GET_SKB_SKIP_BLOG(pktbuf)) {
				DHD_BLOG_SKIP(pktbuf, 0);
				DHD_PKT_CLR_SKB_SKIP_BLOG(pktbuf);
			}
		}
#endif /* BCM_BLOG */
	}
#endif /* DHD_PSTA */

#ifdef DHDTCPACK_SUPPRESS
	if (dhd->pub.tcpack_sup_mode == TCPACK_SUP_HOLD) {
		/* If this packet has been hold or got freed, just return */
		if (dhd_tcpack_hold(&dhd->pub, pktbuf, ifidx)) {
			ret = 0;
			goto done;
		}
	} else {
		/* If this packet has replaced another packet and got freed, just return */
		if (dhd_tcpack_suppress(&dhd->pub, pktbuf)) {
			ret = 0;
			goto done;
		}
	}
#endif /* DHDTCPACK_SUPPRESS */

	ret = dhd_sendpkt(&dhd->pub, ifidx, pktbuf);

done:
	if (ret) {
		ifp->stats.tx_dropped++;
		dhd->pub.tx_dropped++;
	} else {

#ifdef PROP_TXSTATUS
		/* tx_packets counter can counted only when wlfc is disabled */
		if (!dhd_wlfc_is_supported(&dhd->pub))
#endif
		{
			dhd->pub.tx_packets++;
			ifp->stats.tx_packets++;
			ifp->stats.tx_bytes += datalen;
		}
#ifdef BCM_CPEROUTER_EXTSTATS
		if (is_multicast_pkt) {
			ifp->stats.tx_multicast_packets++;
			ifp->stats.tx_multicast_bytes += datalen;
			if (is_broadcast_pkt)
				ifp->stats.tx_broadcast_packets++;
		}
#endif /* BCM_CPEROUTER_EXTSTATS */
	}

#ifdef PCIE_FULL_DONGLE
	DHD_GENERAL_LOCK(&dhd->pub, flags);
	dhd->pub.tx_in_progress = 0;
	DHD_GENERAL_UNLOCK(&dhd->pub, flags);
#endif /* PCIE_FULL_DONGLE */
	DHD_PERIM_UNLOCK_TRY(DHD_FWDER_UNIT(dhd), lock_taken);
	DHD_OS_WAKE_UNLOCK(&dhd->pub);

	BUZZZ_LOG(START_XMIT_END, 0);

	/* Return ok: we always eat the packet */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 20))
	return 0;
#else
	return NETDEV_TX_OK;
#endif
}

#if (defined(BCM_ROUTER_DHD) && defined(BCM_GMAC3))
int BCMFASTPATH
dhd_start_xmit(struct sk_buff *skb, struct net_device *net)
{
	return dhd_start_xmit_try(skb, net, DHD_PERIM_LOCK_NOTTAKEN);
}
#endif /* BCM_ROUTER_DHD && BCM_GMAC3 */

void
dhd_txflowcontrol(dhd_pub_t *dhdp, int ifidx, bool state)
{
	struct net_device *net;
	dhd_info_t *dhd = dhdp->info;
	int i;

	DHD_TRACE(("%s: Enter\n", __FUNCTION__));

	ASSERT(dhd);

	if (ifidx == ALL_INTERFACES) {
		/* Flow control on all active interfaces */
		dhdp->txoff = state;
		for (i = 0; i < DHD_MAX_IFS; i++) {
			if (dhd->iflist[i]) {
				net = dhd->iflist[i]->net;
				if (state == ON)
					netif_stop_queue(net);
				else
					netif_wake_queue(net);
			}
		}
	} else {
		if (dhd->iflist[ifidx]) {
			net = dhd->iflist[ifidx]->net;
			if (state == ON)
				netif_stop_queue(net);
			else
				netif_wake_queue(net);
		}
	}
}

#ifdef DHD_RX_DUMP
typedef struct {
	uint16 type;
	const char *str;
} PKTTYPE_INFO;

static const PKTTYPE_INFO packet_type_info[] =
{
	{ ETHER_TYPE_IP, "IP" },
	{ ETHER_TYPE_ARP, "ARP" },
	{ ETHER_TYPE_BRCM, "BRCM" },
	{ ETHER_TYPE_802_1X, "802.1X" },
	{ ETHER_TYPE_WAI, "WAPI" },
	{ 0, ""}
};

static const char *
_get_packet_type_str(uint16 type)
{
	int i;
	int n = ARRAYSIZE(packet_type_info) - 1;

	for (i = 0; i < n; i++) {
		if (packet_type_info[i].type == type)
			return packet_type_info[i].str;
	}

	return packet_type_info[n].str;
}
#endif /* DHD_RX_DUMP */

#if defined(BCM_CPE_PKTC)
/* Returns TRUE if hot bridge entry for this da is present */
bool BCMFASTPATH
dhd_pktc_tbl_check(dhd_pub_t *dhdp, uint8 *eh, int ifidx)
{
	dhd_info_t *dhd = dhdp->info;
	dhd_if_t *ifp = dhd->iflist[ifidx];

	ASSERT(ifp != NULL);

	if (!dhd->pktc_tbl)
		return FALSE;

	return PKTC_TBL_FN_CMP(dhd->pktc_tbl, (eh), (void *)(ifp->net));
}

bool BCMFASTPATH
dhd_rx_pktc_tbl_chainable(dhd_pub_t *dhdp, int ifidx)
{
	dhd_info_t *dhd = dhdp->info;
	dhd_if_t *ifp = dhd->iflist[ifidx];

	return ifp->rx_pkt_chainable;
}
#endif /* BCM_CPE_PKTC */

#ifdef DHD_WMF
bool
dhd_is_rxthread_enabled(dhd_pub_t *dhdp)
{
	dhd_info_t *dhd = dhdp->info;

	return dhd->rxthread_enabled;
}
#endif /* DHD_WMF */

#ifdef DHD_MCAST_REGEN
/*
 * Description: This function is called to do the reverse translation
 *
 * Input    eh - pointer to the ethernet header
 */
int32
dhd_mcast_reverse_translation(struct ether_header *eh)
{
	uint8 *iph;
	uint32 dest_ip;

	iph = (uint8 *)eh + ETHER_HDR_LEN;
	dest_ip = ntoh32(*((uint32 *)(iph + IPV4_DEST_IP_OFFSET)));

	/* Only IP packets are handled */
	if (eh->ether_type != hton16(ETHER_TYPE_IP))
		return BCME_ERROR;

	/* Non-IPv4 multicast packets are not handled */
	if (IP_VER(iph) != IP_VER_4)
		return BCME_ERROR;

	/*
	 * The packet has a multicast IP and unicast MAC. That means
	 * we have to do the reverse translation
	 */
	if (IPV4_ISMULTI(dest_ip) && !ETHER_ISMULTI(&eh->ether_dhost)) {
		ETHER_FILL_MCAST_ADDR_FROM_IP(eh->ether_dhost, dest_ip);
		return BCME_OK;
	}

	return BCME_ERROR;
}
#endif /* MCAST_REGEN */

/** Called when a frame is received by the dongle on interface 'ifidx' */
void
dhd_rx_frame(dhd_pub_t *dhdp, int ifidx, void *pktbuf, int numpkt, uint8 chan)
{
	dhd_info_t *dhd = (dhd_info_t *)dhdp->info;
	struct sk_buff *skb = NULL;
	uchar *eth;
	uint len = 0;
	void *data, *pnext = NULL;
	int i;
	dhd_if_t *ifp;
	wl_event_msg_t event;
#if (defined(OEM_ANDROID) || defined(OEM_EMBEDDED_LINUX))
	int tout_rx = 0;
	int tout_ctrl = 0;
#endif /* OEM_ANDROID || OEM_EMBEDDED_LINUX */
	void *skbhead = NULL;
	void *skbprev = NULL;
#if defined(DHD_RX_DUMP) || defined(DHD_8021X_DUMP) || defined(BCMDBG) || \
	defined(DHD_DEBUG)
	char *dump_data;
	uint16 protocol;
#endif /* DHD_RX_DUMP || DHD_8021X_DUMP || BCMDBG || DHD_DEBUG */
#ifdef DHD_MCAST_REGEN
	uint8 interface_role;
	if_flow_lkup_t *if_flow_lkup;
	unsigned long flags;
#endif
	int ret = BCME_OK;

	BCM_REFERENCE(ret);
	DHD_TRACE(("%s: Enter\n", __FUNCTION__));

#if defined(BCM_NBUFF_PKT_BPM)
	/* DHD_RX_CHAINING is not supported for FKB buffers. So numpkt is
	 * always passed as 1 from dhd_msgbuf.c
	 */
	ASSERT(!IS_FKBUFF_PTR(pktbuf) || (numpkt == 1));
#endif /* BCM_NBUFF_PKT_BPM */

	for (i = 0; pktbuf && i < numpkt; i++, pktbuf = pnext) {
		struct ether_header *eh;
#ifdef WLBTAMP
		struct dot11_llc_snap_header *lsh;
#endif

#if defined(BCM_NBUFF_PKT_BPM)
		if (IS_SKBUFF_PTR(pktbuf))
#endif /* BCM_NBUFF_PKT_BPM */
		{
			pnext = PKTNEXT(dhdp->osh, pktbuf);
			PKTSETNEXT(dhdp->osh, pktbuf, NULL);
		}

		ifp = dhd->iflist[ifidx];
		if (ifp == NULL) {
			DHD_ERROR(("%s: ifp is NULL. drop packet\n",
				__FUNCTION__));
			PKTCFREE(dhdp->osh, pktbuf, FALSE);
			continue;
		}

		eh = (struct ether_header *)PKTDATA(dhdp->osh, pktbuf);
		len = PKTLEN(dhdp->osh, pktbuf);

#if defined(BCM_NBUFF_PKT_BPM)
		if (PSTA_ENABLED(dhdp) || WET_ENABLED(dhdp) || ETHER_ISMULTI(eh->ether_dhost)) {
			/* if fkb, convert to skb */
			if (IS_FKBUFF_PTR(pktbuf)) {
				skb = dhd_xlate_to_skb(dhdp, pktbuf);
				if (skb == NULL) {
					DHD_ERROR(("%s: %s: xlate to SKB failed for recv packet\n",
						__FUNCTION__, dhd_ifname(&dhd->pub, ifidx)));
					PKTFREE(dhdp->osh, pktbuf, FALSE);
					continue;
				}
				pktbuf = skb;
			}
		}
#endif /* BCM_NBUFF_PKT_BPM */

		if (ETHER_ISMULTI(eh->ether_dhost)) {
			dhd->pub.rx_multicast++;
			ifp->stats.multicast++;
#ifdef BCM_CPEROUTER_EXTSTATS
			ifp->stats.rx_multicast_bytes += len;
			if (ETHER_ISBCAST(eh->ether_dhost))
				ifp->stats.rx_broadcast_packets++;
#endif /* BCM_CPEROUTER_EXTSTATS */
		}

		if (ntoh16(eh->ether_type) != ETHER_TYPE_BRCM) {
			dhdp->dstats.rx_bytes += len;
			dhdp->rx_packets++; /* Local count */
			ifp->stats.rx_bytes += len;
			ifp->stats.rx_packets++;
		}

		/* Dropping packets before registering net device to avoid kernel panic */
#ifndef PROP_TXSTATUS_VSDB
		if (!ifp->net || ifp->net->reg_state != NETREG_REGISTERED) {
#else
		if (!ifp->net || ifp->net->reg_state != NETREG_REGISTERED || !dhd->pub.up) {
#endif /* PROP_TXSTATUS_VSDB */
			DHD_ERROR(("%s: net device is NOT registered yet. drop packet\n",
			__FUNCTION__));
			PKTCFREE(dhdp->osh, pktbuf, FALSE);
			continue;
		}

#ifdef WLBTAMP
		lsh = (struct dot11_llc_snap_header *)&eh[1];

		if ((ntoh16(eh->ether_type) < ETHER_TYPE_MIN) &&
		    (PKTLEN(dhdp->osh, pktbuf) >= RFC1042_HDR_LEN) &&
		    bcmp(lsh, BT_SIG_SNAP_MPROT, DOT11_LLC_SNAP_HDR_LEN - 2) == 0 &&
		    lsh->type == HTON16(BTA_PROT_L2CAP)) {
			amp_hci_ACL_data_t *ACL_data = (amp_hci_ACL_data_t *)
			        ((uint8 *)eh + RFC1042_HDR_LEN);
#ifdef BCMDBG
			if (DHD_BTA_ON())
				dhd_bta_hcidump_ACL_data(dhdp, ACL_data, FALSE);
#endif
			ACL_data = NULL;
		}
#endif /* WLBTAMP */

#ifdef PROP_TXSTATUS
		if (dhd_wlfc_is_header_only_pkt(dhdp, pktbuf)) {
			/* WLFC may send header only packet when
			there is an urgent message but no packet to
			piggy-back on
			*/
			PKTCFREE(dhdp->osh, pktbuf, FALSE);
			continue;
		}
#endif
#ifdef DHD_L2_FILTER
		/* If block_ping is enabled drop the ping packet */
		if (ifp->block_ping) {
			if (bcm_l2_filter_block_ping(dhdp->osh, pktbuf) == BCME_OK) {
				PKTCFREE(dhdp->osh, pktbuf, FALSE);
				continue;
			}
		}
		if (ifp->grat_arp && DHD_IF_ROLE_STA(dhdp, ifidx)) {
		    if (bcm_l2_filter_gratuitous_arp(dhdp->osh, pktbuf) == BCME_OK) {
				PKTCFREE(dhdp->osh, pktbuf, FALSE);
				continue;
		    }
		}
		if (ifp->parp_enable && DHD_IF_ROLE_AP(dhdp, ifidx)) {
			ret = dhd_l2_filter_pkt_handle(dhdp, ifidx, pktbuf, FALSE);

			/* Drop the packets if l2 filter has processed it already
			 * otherwise continue with the normal path
			 */
			if (ret == BCME_OK) {
				PKTCFREE(dhdp->osh, pktbuf, TRUE);
				continue;
			}
		}
		if (ifp->block_tdls) {
			if (bcm_l2_filter_block_tdls(dhdp->osh, pktbuf) == BCME_OK) {
				PKTCFREE(dhdp->osh, pktbuf, FALSE);
				continue;
			}
		}
#endif /* DHD_L2_FILTER */

#ifdef DHD_MCAST_REGEN
		DHD_FLOWID_LOCK(dhdp->flowid_lock, flags);
		if_flow_lkup = (if_flow_lkup_t *)dhdp->if_flow_lkup;
		ASSERT(if_flow_lkup);

		interface_role = if_flow_lkup[ifidx].role;
		DHD_FLOWID_UNLOCK(dhdp->flowid_lock, flags);

		if (ifp->mcast_regen_bss_enable && (interface_role != WLC_E_IF_ROLE_WDS) &&
			!DHD_IF_ROLE_AP(dhdp, ifidx) &&
			ETHER_ISUCAST(eh->ether_dhost)) {
			if (dhd_mcast_reverse_translation(eh) ==  BCME_OK) {
#ifdef DHD_PSTA
				/* Change bsscfg to primary bsscfg for unicast-multicast packets */
				if ((dhd_get_psta_mode(dhdp) == DHD_MODE_PSTA) ||
					(dhd_get_psta_mode(dhdp) == DHD_MODE_PSR)) {
					if (ifidx != 0) {
						/* Let the primary in PSTA interface handle this
						 * frame after unicast to Multicast conversion
						 */
						ifp = dhd_get_ifp(dhdp, 0);
						ASSERT(ifp);
					}
				}
			}
#endif /* PSTA */
		}
#endif /* MCAST_REGEN */

#ifdef DHD_WMF
		/* WMF processing for multicast packets */
		if (ifp->wmf.wmf_enable && (ETHER_ISMULTI(eh->ether_dhost))) {
			dhd_sta_t *sta;

			sta = dhd_find_sta(dhdp, ifidx, (void *)eh->ether_shost);
			ret = dhd_wmf_packets_handle(dhdp, pktbuf, sta, ifidx, 1);
			switch (ret) {
				case WMF_TAKEN:
					/* The packet is taken by WMF. Continue to next iteration */
					continue;
				case WMF_DROP:
					/* Packet DROP decision by WMF. Toss it */
					DHD_INFO(("%s: WMF decides to drop packet\n",
						__FUNCTION__));
					PKTCFREE(dhdp->osh, pktbuf, FALSE);
					continue;
				default:
					/* Continue the transmit path */
					break;
			}
		}
#endif /* DHD_WMF */

#ifdef DHDTCPACK_SUPPRESS
		dhd_tcpdata_info_get(dhdp, pktbuf);
#endif
		ASSERT(ifp);
#if defined(BCM_NBUFF_PKT_BPM)
		if (IS_SKBUFF_PTR(pktbuf))
#endif /* BCM_NBUFF_PKT_BPM */
		{
			skb = PKTTONATIVE(dhdp->osh, pktbuf);
			skb->dev = ifp->net;
		}

#ifdef DHD_WET
		/* wet related packet proto manipulation should be done in DHD
		 * since dongle doesn't have complete payload
		 */
		if (WET_ENABLED(&dhd->pub) &&
#ifdef BCM_ROUTER_DHD
			(ifp->primsta_dwds == FALSE) &&
#endif /* BCM_ROUTER_DHD */
			DHD_IF_ROLE_STA(dhdp, ifidx)) {

#ifdef BCM_BLOG
			/* WET: Pass packet to a Blog client application
			 * before packet manipulation
			 */
			/* XXX: TODO - Find a common place to pass packet to Blog client
			 * application before packet manipulation for modes
			 * like WET, PSTA, L2FILTER etc.
			 */
			if (!DHD_PKT_GET_SKB_SKIP_BLOG(pktbuf)) {

#if defined(BCM_NBUFF_PKT_BPM)
				/* In WET mode, expects only SKB frames */
				ASSERT(IS_SKBUFF_PTR(pktbuf));
#endif /* BCM_NBUFF_PKT_BPM */

				if (PKT_DONE == dhd_handle_blog_sinit(dhdp, ifidx, pktbuf)) {
					continue;
				}

				/* Skip any further blog_sint processing in DHD. */
				DHD_PKT_SET_SKB_SKIP_BLOG(pktbuf);
			}
#endif /* BCM_BLOG */

			if (dhd_wet_recv_proc(dhd->pub.wet_info, ifidx,
					pktbuf) < 0) {
				DHD_INFO(("%s:%s: wet recv proc failed\n",
					__FUNCTION__, dhd_ifname(dhdp, ifidx)));
			}
		}
#endif /* DHD_WET */

#ifdef DHD_DPSTA
		if (PSTA_ENABLED(dhdp) || WET_ENABLED(dhdp)) {
			if (dpsta_recv(skb) != BCME_OK) {
				PKTFRMNATIVE(dhdp->osh, skb);
				PKTCFREE(dhdp->osh, skb, FALSE);
				return;
			}
		}
#endif /* DHD_DPSTA */

#ifdef DHD_PSTA
		if (PSR_ENABLED(dhdp) &&
#ifdef BCM_ROUTER_DHD
			!(ifp->primsta_dwds) &&
#endif /* BCM_ROUTER_DHD */
			(dhd_psta_proc(dhdp, ifidx, &pktbuf, FALSE) < 0)) {
			DHD_ERROR(("%s:%s: psta recv proc failed\n", __FUNCTION__,
				dhd_ifname(dhdp, ifidx)));
		}
#endif /* DHD_PSTA */

#if defined(BCM_ROUTER_DHD)
		if (DHD_IF_ROLE_AP(dhdp, ifidx) && (!ifp->ap_isolate)) {
			eh = (struct ether_header *)PKTDATA(dhdp->osh, pktbuf);
			if (ETHER_ISUCAST(eh->ether_dhost)) {
				if (dhd_find_sta(dhdp, ifidx, (void *)eh->ether_dhost)) {
#if defined(BCM_BLOG)
					if (DHD_PKT_GET_SKB_SKIP_BLOG(pktbuf)) {
						DHD_PKT_CLR_SKB_SKIP_BLOG(pktbuf);
					}
					else
					{

#if defined(BCM_NBUFF_PKT_BPM)
						if (IS_FKBUFF_PTR(pktbuf)) {
							ret = dhd_handle_blog_finit(dhdp, ifidx,
								PNBUFF_2_FKBUFF(pktbuf), ifp->net);
						} else
#endif /* BCM_NBUFF_PKT_BPM */
						{
							ret = dhd_handle_blog_sinit(dhdp, ifidx,
								pktbuf);
						}

						if (ret == PKT_DONE)
							continue;
					}
#endif /* BCM_BLOG */

					DHD_PKT_SET_FKB_FLOW_UNHANDLED(pktbuf);
#if defined(BCM_NBUFF_PKT_BPM)
					/* Converts to skb since blog_emit expects skb.
					 * Otherwise blog will never be attached to this
					 * frame.
					 *
					 * TODO: Even without this change, packets would have been
					 * forwarded through legacy dhd_sendpkt when blog_entry is
					 * not present. But it seems blog_sinit() decrements
					 * fkb->users even when it does not handle the packets
					 * through FC. This causes non zero fkb->users count
					 * during pktfree after dhd_sendpkt and fkb_free does not
					 * call the dhd recycle handler which recycles packets
					 * to BPM. Hence BPM buffer gets exhausted after few mins
					 */
					if (IS_FKBUFF_PTR(pktbuf)) {
						skb = dhd_xlate_to_skb(dhdp, pktbuf);
						if (skb == NULL) {
							DHD_ERROR(("%s: xlate to skb failed\n",
							    __FUNCTION__));
							dhdp->rx_dropped++;
							PKTFREE(dhdp->osh, pktbuf, FALSE);
							continue;
						}
						skb->dev = ifp->net;
						pktbuf = skb;
					}
#endif /* BCM_NBUFF_PKT_BPM */
					dhd_sendpkt(dhdp, ifidx, pktbuf);
					continue;
				}
			} else {
				void *npkt;

				if ((ntoh16(eh->ether_type) != ETHER_TYPE_IAPP_L2_UPDATE)) {
					if (DHDHDR_SUPPORT(&dhd->pub))
						npkt = PKTDUP_CPY(dhdp->osh, pktbuf);
					else
						npkt = PKTDUP(dhdp->osh, pktbuf);

					if (npkt != NULL) {
#if defined(BCM_ROUTER_DHD) && defined(BCM_GMAC3)
#if defined(DHD_IGMP_UCQUERY) || defined(DHD_UCAST_UPNP)
						bool ucast_convert =
							dhd_need_wmf_ucast_convert(dhdp, eh);

						if (ucast_convert) {
							PKTTONATIVE(dhdp->osh, npkt);
							dhd_start_xmit_try(npkt, ifp->net,
									DHD_PERIM_LOCK_TAKEN);
						} else
#endif /* DHD_IGMP_UCQUERY || DHD_UCAST_UPNP */
#endif /* BCM_ROUTER_DHD && BCM_GMAC3 */
						{
							dhd_sendpkt(dhdp, ifidx, npkt);
						}
					}
				}
			}
		}

#if defined(BCM_CPE_PKTC)
		DHD_UNLOCK(dhdp);
		if (dhd_rxchainhandler(dhdp, skb) == BCME_OK) {
			dhdp->dstats.rx_bytes += PKTLEN(dhdp->osh, skb);
			dhdp->rx_packets ++;
			ifp->stats.rx_bytes += PKTLEN(dhdp->osh, skb);
			ifp->stats.rx_packets ++;
			DHD_LOCK(dhdp);
			continue;
		}
		DHD_LOCK(dhdp);
#endif /* BCM_CPE_PKTC */

#else /* !BCM_ROUTER_DHD */
#ifdef PCIE_FULL_DONGLE
		if ((DHD_IF_ROLE_AP(dhdp, ifidx) || DHD_IF_ROLE_P2PGO(dhdp, ifidx)) &&
			(!ifp->ap_isolate)) {
			eh = (struct ether_header *)PKTDATA(dhdp->osh, pktbuf);
			if (ETHER_ISUCAST(eh->ether_dhost)) {
				if (dhd_find_sta(dhdp, ifidx, (void *)eh->ether_dhost)) {
					dhd_sendpkt(dhdp, ifidx, pktbuf);
					continue;
				}
			} else {
				void *npktbuf = NULL;
				if ((ntoh16(eh->ether_type) != ETHER_TYPE_IAPP_L2_UPDATE)) {
					if (DHDHDR_SUPPORT(&dhd->pub))
						npktbuf = PKTDUP_CPY(dhdp->osh, pktbuf);
					else
						npktbuf = PKTDUP(dhd->pub.osh, pktbuf);
					if (npktbuf != NULL)
						dhd_sendpkt(dhdp, ifidx, npktbuf);
				}
			}
		}
#endif /* PCIE_FULL_DONGLE */
#endif /* BCM_ROUTER_DHD */

#ifdef BCM_BLOG
		if (DHD_PKT_GET_SKB_SKIP_BLOG(pktbuf)) {
			DHD_PKT_CLR_SKB_SKIP_BLOG(pktbuf);
		} else {

#if defined(BCM_PKTFWD)
			if (true &&
#if defined(BCM_DHD_RUNNER)
				(!DHD_RNR_OFFL_RXCMPL(dhdp)) &&
#endif /* BCM_DHD_RUNNER */
				(PKTFWD_ENABLED(dhdp)))
			{
				DHD_UNLOCK(dhdp);

				if (IS_FKBUFF_PTR(pktbuf)) {
					/* Set PKT priority */
					pktsetprio(pktbuf, FALSE);
					/* PktQueue based Handoff to Egress network device */
					ret = dhd_pktfwd_pktqueue_add_pkt(dhdp, ifp->net, pktbuf);
				} else {
					/* Rx through pktfwd/d3lut system */
					ret = dhd_pktfwd_upstream(dhdp, pktbuf);
				}

				DHD_LOCK(dhdp);

				if (ret == BCME_OK) {
					continue;
				}
			}
#endif /* BCM_PKTFWD */

#if defined(BCM_NBUFF_PKT_BPM)
			if (IS_FKBUFF_PTR(pktbuf)) {
				FkBuff_t *fkb = PNBUFF_2_FKBUFF(pktbuf);
				uint32 pkt_len = fkb->len;
				void * pkt_data = fkb->data;

				ret = dhd_handle_blog_finit(dhdp, ifidx, fkb, ifp->net);

				if ((ret == PKT_NORM)) {
					/* for PKT_NORM, fkb is released already
					 * so initialize it again for xlate to work
					 */
					fkb = fkb_init(PFKBUFF_TO_PDATA(fkb, BCM_PKT_HEADROOM),
						BCM_PKT_HEADROOM, pkt_data, pkt_len);

#ifdef BCM_NBUFF_DHD_RECYCLE_HOOK
					fkb->recycle_hook = dhd_nbuff_recycle;
#else
					fkb->recycle_hook = gbpm_recycle_pNBuff;
#endif
					pktbuf = FKBUFF_2_PNBUFF(fkb);
				}
			} else
#endif /* BCM_NBUFF_PKT_BPM */
			{
				ret = dhd_handle_blog_sinit(dhdp, ifidx, pktbuf);
			}

			if (ret == PKT_DONE)
				continue;
		}
#endif /* BCM_BLOG */

#if defined(BCM_NBUFF_PKT_BPM)
		/* Legacy slow path through stack which accepts skb
		 * Convert fkb to skb here
		 */
		if (IS_FKBUFF_PTR(pktbuf)) {
			skb = dhd_xlate_to_skb(dhdp, pktbuf);
			if (skb == NULL) {
				DHD_ERROR(("%s: xlate to skb failed\n", __FUNCTION__));
				dhdp->rx_dropped++;
				PKTFREE(dhdp->osh, pktbuf, FALSE);
				continue;
			}
			skb->dev = ifp->net;
			pktbuf = skb;
		}

		ASSERT(IS_SKBUFF_PTR(pktbuf));
#endif /* BCM_NBUFF_PKT_BPM */

#if defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE)
		/* skb has to be converted from fkb as BCM_NBUFF_PKT_BPM is enabled */
		if (dhd_spdsvc_rx(skb) == BCME_OK)
			continue;
#endif

		/* Get the protocol, maintain skb around eth_type_trans()
		 * The main reason for this hack is for the limitation of
		 * Linux 2.4 where 'eth_type_trans' uses the 'net->hard_header_len'
		 * to perform skb_pull inside vs ETH_HLEN. Since to avoid
		 * coping of the packet coming from the network stack to add
		 * BDC, Hardware header etc, during network interface registration
		 * we set the 'net->hard_header_len' to ETH_HLEN + extra space required
		 * for BDC, Hardware header etc. and not just the ETH_HLEN
		 */
		eth = skb->data;
		len = skb->len;

#if defined(DHD_RX_DUMP) || defined(DHD_8021X_DUMP) || defined(BCMDBG) || \
	defined(DHD_DEBUG)
		dump_data = skb->data;
		protocol = (dump_data[12] << 8) | dump_data[13];
#endif /* DHD_RX_DUMP || DHD_8021X_DUMP || BCMDBG || DHD_DEBUG */
#ifdef DHD_8021X_DUMP
		if (protocol == ETHER_TYPE_802_1X) {
			DHD_DATA(("ETHER_TYPE_802_1X [RX]: "
				"ver %d, type %d, replay %d\n",
				dump_data[14], dump_data[15],
				dump_data[30]));
		}
#endif /* DHD_8021X_DUMP */
#if defined(BCMDBG) || defined(DHD_DEBUG)
		if (protocol != ETHER_TYPE_BRCM && protocol == ETHER_TYPE_IP &&
			DHD_DHCP_DUMP_ON()) {
			dhd_dhcp_dump(dump_data, FALSE);
		}
#endif /* defined(BCMDBG) || defined(DHD_DEBUG) */
#if defined(DHD_RX_DUMP)
		DHD_ERROR(("RX DUMP - %s\n", _get_packet_type_str(protocol)));
		if (protocol != ETHER_TYPE_BRCM) {
			if (dump_data[0] == 0xFF) {
				DHD_ERROR(("%s: BROADCAST\n", __FUNCTION__));

				if ((dump_data[12] == 8) &&
					(dump_data[13] == 6)) {
					DHD_ERROR(("%s: ARP %d\n",
						__FUNCTION__, dump_data[0x15]));
				}
			} else if (dump_data[0] & 1) {
				DHD_ERROR(("%s: MULTICAST: " MACDBG "\n",
					__FUNCTION__, MAC2STRDBG(dump_data)));
			}
#ifdef DHD_RX_FULL_DUMP
			{
				int k;
				for (k = 0; k < skb->len; k++) {
					DHD_ERROR(("%02X ", dump_data[k]));
					if ((k & 15) == 15)
						DHD_ERROR(("\n"));
				}
				DHD_ERROR(("\n"));
			}
#endif /* DHD_RX_FULL_DUMP */
		}
#endif /* DHD_RX_DUMP */

		skb->protocol = eth_type_trans(skb, skb->dev);
		skb->data = eth;
		skb->len = len;

#ifdef WLMEDIA_HTSF
		dhd_htsf_addrxts(dhdp, pktbuf);
#endif
		/* Strip header, count, deliver upward */
		skb_pull(skb, ETH_HLEN);

		/* Process special event packets and then discard them */
		memset(&event, 0, sizeof(event));
		if (ntoh16(skb->protocol) == ETHER_TYPE_BRCM) {
			dhd_wl_host_event(dhd, &ifidx,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 22)
			skb_mac_header(skb),
#else
			skb->mac.raw,
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 22) */
			len - ETHER_TYPE_LEN,
			&event,
			&data);

			wl_event_to_host_order(&event);
#if (defined(OEM_ANDROID) || defined(OEM_EMBEDDED_LINUX))
			if (!tout_ctrl)
				tout_ctrl = DHD_PACKET_TIMEOUT_MS;
#endif /* (defined(OEM_ANDROID) || defined(OEM_EMBEDDED_LINUX)) */
#ifdef WLBTAMP
			if (event.event_type == WLC_E_BTA_HCI_EVENT) {
				dhd_bta_doevt(dhdp, data, event.datalen);
			}
#endif /* WLBTAMP */

#if (defined(OEM_ANDROID) && defined(PNO_SUPPORT))
			if (event.event_type == WLC_E_PFN_NET_FOUND) {
				/* enforce custom wake lock to garantee that Kernel not suspended */
				tout_ctrl = CUSTOM_PNO_EVENT_LOCK_xTIME * DHD_PACKET_TIMEOUT_MS;
			}
#endif /* PNO_SUPPORT */
			if (numpkt != 1) {
				DHD_ERROR(("%s:  %s: Got BRCM event packet in a chained packet.\n",
				dhd_ifname(dhdp, ifidx), __FUNCTION__));
			}
#ifdef DHD_DONOT_FORWARD_BCMEVENT_AS_NETWORK_PKT
			PKTFREE(dhdp->osh, pktbuf, FALSE);
			continue;
#else
			/*
			 * For the event packets, there is a possibility
			 * of ifidx getting modifed.Thus update the ifp
			 * once again.
			 */
			ASSERT(ifidx < DHD_MAX_IFS && dhd->iflist[ifidx]);
			ifp = dhd->iflist[ifidx];
#ifndef PROP_TXSTATUS_VSDB
			if (!(ifp && ifp->net && (ifp->net->reg_state == NETREG_REGISTERED))) {
#else
			if (!(ifp && ifp->net && (ifp->net->reg_state == NETREG_REGISTERED) &&
				dhd->pub.up)) {
#endif /* PROP_TXSTATUS_VSDB */
				DHD_ERROR(("%s: net device is NOT registered. drop event packet\n",
				__FUNCTION__));
				PKTFREE(dhdp->osh, pktbuf, FALSE);
				continue;
			}
#endif /* DHD_DONOT_FORWARD_BCMEVENT_AS_NETWORK_PKT */
		} else {
#if (defined(OEM_ANDROID) || defined(OEM_EMBEDDED_LINUX))
			tout_rx = DHD_PACKET_TIMEOUT_MS;
#endif /* OEM_ANDROID || OEM_EMBEDDED_LINUX */

#ifdef PROP_TXSTATUS
			dhd_wlfc_save_rxpath_ac_time(dhdp, (uint8)PKTPRIO(skb));
#endif /* PROP_TXSTATUS */
		}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 11, 0)
		if (ifp->net)
			ifp->net->last_rx = jiffies;
#endif

		if (event.event_type == WLC_E_AIRIQ_EVENT) {
			skb->protocol = hton16(ETHER_TYPE_BRCM_AIRIQ);
		}

#if defined(DHD_TCP_WINSIZE_ADJUST)
		if (dhd_use_tcp_window_size_adjust) {
			if (ifidx == 0 && ntoh16(skb->protocol) == ETHER_TYPE_IP) {
				dhd_adjust_tcp_winsize(dhdp->wlcore->op_mode, skb);
			}
		}
#endif /* DHD_TCP_WINSIZE_ADJUST */

		if (in_interrupt()) {
			bcm_object_trace_opr(skb, BCM_OBJDBG_REMOVE,
				__FUNCTION__, __LINE__);
			DHD_UNLOCK(dhdp);
#if defined(DHD_LB) && defined(DHD_LB_RXP)
			netif_receive_skb(skb);
#else
			netif_rx(skb);
#endif /* !defined(DHD_LB) && !defined(DHD_LB_RXP) */
			DHD_LOCK(dhdp);
		} else {
			if (dhd->rxthread_enabled) {
				if (!skbhead)
					skbhead = skb;
				else
					PKTSETNEXT(dhdp->osh, skbprev, skb);
				skbprev = skb;
			} else {

				/* If the receive is not processed inside an ISR,
				 * the softirqd must be woken explicitly to service
				 * the NET_RX_SOFTIRQ.	In 2.6 kernels, this is handled
				 * by netif_rx_ni(), but in earlier kernels, we need
				 * to do it manually.
				 */
				bcm_object_trace_opr(skb, BCM_OBJDBG_REMOVE,
					__FUNCTION__, __LINE__);

#if defined(DHD_LB) && defined(DHD_LB_RXP)
				DHD_UNLOCK(dhdp);
				netif_receive_skb(skb);
				DHD_LOCK(dhdp);
#else
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0)
				DHD_UNLOCK(dhdp);
				netif_rx_ni(skb);
				DHD_LOCK(dhdp);
#else
				ulong flags;
				DHD_UNLOCK(dhdp);
				netif_rx(skb);
				DHD_LOCK(dhdp);
				local_irq_save(flags);
				RAISE_RX_SOFTIRQ();
				local_irq_restore(flags);
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0) */
#endif /* !defined(DHD_LB) && !defined(DHD_LB_RXP) */
			}
		}

	}

	if (dhd->rxthread_enabled && skbhead)
		dhd_sched_rxf(dhdp, skbhead);

#if (defined(OEM_ANDROID) || defined(OEM_EMBEDDED_LINUX))
	DHD_OS_WAKE_LOCK_RX_TIMEOUT_ENABLE(dhdp, tout_rx);
	DHD_OS_WAKE_LOCK_CTRL_TIMEOUT_ENABLE(dhdp, tout_ctrl);
#endif /* OEM_ANDROID || OEM_EMBEDDED_LINUX */
}

void
dhd_event(struct dhd_info *dhd, char *evpkt, int evlen, int ifidx)
{
	/* Linux version has nothing to do */
	return;
}

void
dhd_txcomplete(dhd_pub_t *dhdp, void *txp, bool success)
{
	dhd_info_t *dhd = (dhd_info_t *)(dhdp->info);
	struct ether_header *eh;
	uint16 type;
#ifdef WLBTAMP
	uint len;
#endif

	dhd_prot_hdrpull(dhdp, NULL, txp, NULL, NULL);

	eh = (struct ether_header *)PKTDATA(dhdp->osh, txp);
	type  = ntoh16(eh->ether_type);

	if ((type == ETHER_TYPE_802_1X) && (dhd_get_pend_8021x_cnt(dhd) > 0))
		atomic_dec(&dhd->pend_8021x_cnt);

#ifdef WLBTAMP
	/* Crack open the packet and check to see if it is BT HCI ACL data packet.
	 * If yes generate packet completion event.
	 */
	len = PKTLEN(dhdp->osh, txp);

	/* Generate ACL data tx completion event locally to avoid SDIO bus transaction */
	if ((type < ETHER_TYPE_MIN) && (len >= RFC1042_HDR_LEN)) {
		struct dot11_llc_snap_header *lsh = (struct dot11_llc_snap_header *)&eh[1];

		if (bcmp(lsh, BT_SIG_SNAP_MPROT, DOT11_LLC_SNAP_HDR_LEN - 2) == 0 &&
		    ntoh16(lsh->type) == BTA_PROT_L2CAP) {

			dhd_bta_tx_hcidata_complete(dhdp, txp, success);
		}
	}
#endif /* WLBTAMP */
#ifdef PROP_TXSTATUS
	if (dhdp->wlfc_state && (dhdp->proptxstatus_mode != WLFC_FCMODE_NONE)) {
		dhd_if_t *ifp = dhd->iflist[DHD_PKTTAG_IF(PKTTAG(txp))];
		uint datalen  = PKTLEN(dhd->pub.osh, txp);
		if (ifp != NULL) {
			if (success) {
				dhd->pub.tx_packets++;
				ifp->stats.tx_packets++;
				ifp->stats.tx_bytes += datalen;
			} else {
				ifp->stats.tx_dropped++;
			}
		}
	}
#endif
}

#if defined(BCM_DHD_RUNNER) && !defined(BCM_WLAN_PER_CLIENT_FLOW_LEARNING)
static struct rtnl_link_stats64*
_dhd_stats_runner_update(struct net_device *net, dhd_if_t *ifp, int ifidx,
	struct rtnl_link_stats64 *stats)
{
	dhd_info_t *dhd = DHD_DEV_INFO(net);
	dhd_helper_rnr_stats_t	rnr_stats;
	rnr_stats.ifidx = ifidx;
	if (dhd_runner_do_iovar(dhd->pub.runner_hlp, DHD_RNR_IOVAR_RNR_STATS,
	    FALSE, (char*)&rnr_stats, sizeof(rnr_stats)) == BCME_OK) {

	/* These counters are not clear on read. */
		if (rnr_stats.mcast_pkts != ifp->m_stats.tx_packets) {
#ifdef BCM_CPEROUTER_EXTSTATS
			BlogStats_t *bStats_p;
			bStats_p = &(ifp->b_stats);
			bStats_p->tx_multicast_packets +=
				DELTA(rnr_stats.mcast_pkts, ifp->m_stats.tx_packets);
			bStats_p->tx_multicast_bytes +=
				DELTA(rnr_stats.mcast_bytes, ifp->m_stats.tx_bytes);
#endif
			ifp->stats.tx_packets +=
				DELTA(rnr_stats.mcast_pkts, ifp->m_stats.tx_packets);
			ifp->stats.tx_bytes += DELTA(rnr_stats.mcast_bytes, ifp->m_stats.tx_bytes);
			ifp->m_stats.tx_packets = rnr_stats.mcast_pkts;
			ifp->m_stats.tx_bytes = rnr_stats.mcast_bytes;
		}

	/* This counter is clear on read. */
	ifp->stats.tx_dropped += rnr_stats.dropped_pkts;
	} else {
		DHD_ERROR(("%s: dhd_runner_do_iovar returned Error\n", __FUNCTION__));
		memset(stats, 0, sizeof(struct rtnl_link_stats64));
		return NULL;
	}
#ifdef BCM_COUNTER_EXTSTATS
	return &ifp->stats;
#else
	return net_dev_collect_stats64(net, stats);
#endif
}
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36))
static STATS64_RETURN_TYPE
dhd_get_stats(struct net_device *net, struct rtnl_link_stats64 *stats)
#else
static struct net_device_stats *
dhd_get_stats(struct net_device *net)
#endif /* KERNEL_VERSION >= 2.6.36 */
{
	dhd_info_t *dhd = DHD_DEV_INFO(net);
	dhd_if_t *ifp;
	int ifidx;
	DHD_TRACE(("%s: Enter\n", __FUNCTION__));

	ASSERT(dhd != NULL);

	ifidx = dhd_net2idx(dhd, net);
	if (ifidx == DHD_BAD_IF) {
		DHD_ERROR(("%s: BAD_IF\n", __FUNCTION__));

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
		RETURN_STATS64(stats);
#else
		return &net->stats;
#endif /* KERNEL_VERSION >= 2.6.36 */
	}

#ifdef BCA_HNDROUTER
	DHD_LOCK(&dhd->pub);
#endif
	ifp = dhd->iflist[ifidx];
#ifndef BCA_HNDROUTER
	ASSERT(ifp != NULL);
#else
	if (!ifp) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36))
		memset(stats, 0, sizeof(struct rtnl_link_stats64));
#endif
		goto out;
	}
#endif /* defined(BCA_HNDROUTER) */

	if (dhd->pub.up) {
		/* Use the protocol to get dongle stats */
		dhd_prot_dstats(&dhd->pub);
#if defined(BCA_HNDROUTER) && defined(BCM_DHD_RUNNER) && \
	!defined(BCM_WLAN_PER_CLIENT_FLOW_LEARNING)
		if (!_dhd_stats_runner_update(net, ifp, ifidx, stats)) {
			goto out;
		}
#endif
	}

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 36))
	/* since 2.6.37 the return pointer is ignored so need to memcpy */
	memcpy(stats, &ifp->stats, sizeof(struct rtnl_link_stats64));
#endif

#ifdef BCA_HNDROUTER
out:
	DHD_UNLOCK(&dhd->pub);
	/* for kernel>2.6.36, the return pointer is useless and not
	 * referredd to, so outof lock is ok
	 */
#endif /* BCA_HNDROUTER */
	RETURN_STATS64(&ifp->stats);
}

#ifndef BCMDBUS
static int
dhd_watchdog_thread(void *data)
{
	tsk_ctl_t *tsk = (tsk_ctl_t *)data;
	dhd_info_t *dhd = (dhd_info_t *)tsk->parent;
	int csimon_ret = BCME_OK;

	/* This thread doesn't need any user-level access,
	 * so get rid of all our resources
	 */
	if (dhd_watchdog_prio > 0) {
		struct sched_param param;
		param.sched_priority = (dhd_watchdog_prio < MAX_RT_PRIO)?
			dhd_watchdog_prio:(MAX_RT_PRIO-1);
		setScheduler(current, SCHED_FIFO, &param);
	}

	while (1) {
		if (down_interruptible (&tsk->sema) == 0) {
			unsigned long flags;
			unsigned long jiffies_at_start = jiffies;
			unsigned long time_lapse;

			DHD_OS_WD_WAKE_LOCK(&dhd->pub);
			if (tsk->terminated) {
				break;
			}

			if (dhd->pub.dongle_reset == FALSE) {
				DHD_TIMER(("%s:\n", __FUNCTION__));

				/* Call the bus module watchdog */
				dhd_bus_watchdog(&dhd->pub);

				DHD_GENERAL_LOCK(&dhd->pub, flags);
				/* Count the tick for reference */
				dhd->pub.tickcnt++;
#ifdef DHD_L2_FILTER
				dhd_l2_filter_watchdog(&dhd->pub);
#endif /* DHD_L2_FILTER */

				csimon_ret = dhd_csimon_watchdog(&dhd->pub);

				time_lapse = jiffies - jiffies_at_start;

				/* Reschedule the watchdog */
				if (dhd->wd_timer_valid &&
				    (csimon_ret != BCME_NODEVICE)) {
					mod_timer(&dhd->timer,
					    jiffies +
					    msecs_to_jiffies(dhd_watchdog_ms) -
					    min(msecs_to_jiffies(dhd_watchdog_ms), time_lapse));
				}
				DHD_GENERAL_UNLOCK(&dhd->pub, flags);
			}
			DHD_OS_WD_WAKE_UNLOCK(&dhd->pub);
		} else {
			break;
		}
	}

	complete_and_exit(&tsk->completed, 0);
}

static void
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
dhd_watchdog(ulong data)
{
	dhd_info_t *dhd = (dhd_info_t *)data;
#else
dhd_watchdog(struct timer_list *t)
{
	dhd_info_t *dhd = from_timer(dhd, t, timer);
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0) */
	unsigned long flags;

	if (dhd->pub.dongle_reset) {
		return;
	}

	if (dhd->thr_wdt_ctl.thr_pid >= 0) {
		up(&dhd->thr_wdt_ctl.sema);
		return;
	}

	DHD_OS_WD_WAKE_LOCK(&dhd->pub);
	/* Call the bus module watchdog */
	dhd_bus_watchdog(&dhd->pub);

	DHD_GENERAL_LOCK(&dhd->pub, flags);
	/* Count the tick for reference */
	dhd->pub.tickcnt++;

#ifdef DHD_L2_FILTER
	dhd_l2_filter_watchdog(&dhd->pub);
#endif /* DHD_L2_FILTER */
	/* Reschedule the watchdog */
	if (dhd->wd_timer_valid)
		mod_timer(&dhd->timer, jiffies + msecs_to_jiffies(dhd_watchdog_ms));
	DHD_GENERAL_UNLOCK(&dhd->pub, flags);
	DHD_OS_WD_WAKE_UNLOCK(&dhd->pub);
}

#ifdef ENABLE_ADAPTIVE_SCHED
static void
dhd_sched_policy(int prio)
{
	struct sched_param param;
	if (cpufreq_quick_get(0) <= CUSTOM_CPUFREQ_THRESH) {
		param.sched_priority = 0;
		setScheduler(current, SCHED_NORMAL, &param);
	} else {
		if (get_scheduler_policy(current) != SCHED_FIFO) {
			param.sched_priority = (prio < MAX_RT_PRIO)? prio : (MAX_RT_PRIO-1);
			setScheduler(current, SCHED_FIFO, &param);
		}
	}
}
#endif /* ENABLE_ADAPTIVE_SCHED */

#ifdef DEBUG_CPU_FREQ
static int
dhd_cpufreq_notifier(struct notifier_block *nb, unsigned long val, void *data)
{
	dhd_info_t *dhd = container_of(nb, dhd_info_t, freq_trans);
	struct cpufreq_freqs *freq = data;
	if (dhd) {
		if (!dhd->new_freq)
			goto exit;
		if (val == CPUFREQ_POSTCHANGE) {
			DHD_ERROR(("cpu freq is changed to %u kHZ on CPU %d\n",
				freq->new, freq->cpu));
			*per_cpu_ptr(dhd->new_freq, freq->cpu) = freq->new;
		}
	}
exit:
	return 0;
}
#endif /* DEBUG_CPU_FREQ */

static int
dhd_dpc_thread(void *data)
{
#if defined(CUSTOMER_HW4) && defined(ARGOS_CPU_SCHEDULER)
	int ret = 0;
	unsigned long flags;
#endif /* CUSTOMER_HW4 && ARGOS_CPU_SCHEDULER */
	tsk_ctl_t *tsk = (tsk_ctl_t *)data;
	dhd_info_t *dhd = (dhd_info_t *)tsk->parent;

	/* This thread doesn't need any user-level access,
	 * so get rid of all our resources
	 */
	if (dhd_dpc_prio > 0)
	{
		struct sched_param param;
		param.sched_priority = (dhd_dpc_prio < MAX_RT_PRIO)?dhd_dpc_prio:(MAX_RT_PRIO-1);
		setScheduler(current, SCHED_FIFO, &param);
	}

#if defined(CUSTOMER_HW4) && defined(ARGOS_CPU_SCHEDULER)
	if (!zalloc_cpumask_var(&dhd->pub.default_cpu_mask, GFP_KERNEL)) {
		DHD_ERROR(("dpc_thread, zalloc_cpumask_var error\n"));
		dhd->pub.affinity_isdpc = FALSE;
	} else {
		if (!zalloc_cpumask_var(&dhd->pub.dpc_affinity_cpu_mask, GFP_KERNEL)) {
			DHD_ERROR(("dpc_thread, dpc_affinity_cpu_mask  error\n"));
			free_cpumask_var(dhd->pub.default_cpu_mask);
			dhd->pub.affinity_isdpc = FALSE;
		} else {
			cpumask_copy(dhd->pub.default_cpu_mask, &hmp_slow_cpu_mask);
			cpumask_or(dhd->pub.dpc_affinity_cpu_mask,
				dhd->pub.dpc_affinity_cpu_mask, cpumask_of(DPC_CPUCORE));

			DHD_GENERAL_LOCK(&dhd->pub, flags);
			if ((ret = argos_task_affinity_setup_label(current, "WIFI",
				dhd->pub.dpc_affinity_cpu_mask,
				dhd->pub.default_cpu_mask)) < 0) {
				DHD_ERROR(("Failed to add CPU affinity(dpc) error=%d\n",
					ret));
				free_cpumask_var(dhd->pub.default_cpu_mask);
				free_cpumask_var(dhd->pub.dpc_affinity_cpu_mask);
				dhd->pub.affinity_isdpc = FALSE;
			} else {
				DHD_ERROR(("Argos set Completed : dpcthread\n"));
				set_cpucore_for_interrupt(dhd->pub.default_cpu_mask,
					dhd->pub.dpc_affinity_cpu_mask);
				dhd->pub.affinity_isdpc = TRUE;
			}
			DHD_GENERAL_UNLOCK(&dhd->pub, flags);
		}
	}
#else /* CUSTOMER_HW4 && ARGOS_CPU_SCHEDULER */
#ifdef CUSTOM_DPC_CPUCORE
	set_cpus_allowed_ptr(current, cpumask_of(CUSTOM_DPC_CPUCORE));
#endif
#ifdef CUSTOM_SET_CPUCORE
	dhd->pub.current_dpc = current;
#endif /* CUSTOM_SET_CPUCORE */
#endif /* CUSTOMER_HW4 && ARGOS_CPU_SCHEDULER */
	/* Run until signal received */
	while (1) {
		if (!binary_sema_down(tsk)) {
#ifdef ENABLE_ADAPTIVE_SCHED
			dhd_sched_policy(dhd_dpc_prio);
#endif /* ENABLE_ADAPTIVE_SCHED */
			if (tsk->terminated) {
				break;
			}

			/* Call bus dpc unless it indicated down (then clean stop) */
			if (dhd->pub.busstate != DHD_BUS_DOWN) {
#if defined(CUSTOMER_HW4) || defined(CUSTOMER_HW5)
				int resched_cnt = 0;
#endif /* CUSTOMER_HW4 || CUSTOMER_HW5 */
				dhd_os_wd_timer_extend(&dhd->pub, TRUE);
				while (dhd_bus_dpc(dhd->pub.bus)) {
					/* process all data */
#if defined(CUSTOMER_HW4) || defined(CUSTOMER_HW5)
					resched_cnt++;
					if (resched_cnt > MAX_RESCHED_CNT) {
						DHD_INFO(("%s Calling msleep to"
							"let other processes run. \n",
							__FUNCTION__));
						dhd->pub.dhd_bug_on = true;
						resched_cnt = 0;
						OSL_SLEEP(1);
					}
#endif /* CUSTOMER_HW4 || CUSTOMER_HW5 */
				}
				dhd_os_wd_timer_extend(&dhd->pub, FALSE);
				DHD_OS_WAKE_UNLOCK(&dhd->pub);

			} else {
				if (dhd->pub.up)
					dhd_bus_stop(dhd->pub.bus, TRUE);
				DHD_OS_WAKE_UNLOCK(&dhd->pub);
			}
		} else {
			break;
		}
	}
#if defined(CUSTOMER_HW4) && defined(ARGOS_CPU_SCHEDULER)
	if (dhd->pub.affinity_isdpc == TRUE) {
		free_cpumask_var(dhd->pub.default_cpu_mask);
		free_cpumask_var(dhd->pub.dpc_affinity_cpu_mask);
		dhd->pub.affinity_isdpc = FALSE;
	}
#endif /* CUSTOMER_HW4 && ARGOS_CPU_SCHEDULER */
	complete_and_exit(&tsk->completed, 0);
}

static int
dhd_rxf_thread(void *data)
{
	tsk_ctl_t *tsk = (tsk_ctl_t *)data;
	dhd_info_t *dhd = (dhd_info_t *)tsk->parent;
#if defined(CUSTOMER_HW4) && defined(ARGOS_CPU_SCHEDULER)
	int ret = 0;
	unsigned long flags;
#endif /* CUSTOMER_HW4 && ARGOS_CPU_SCHEDULER */
#if defined(WAIT_DEQUEUE)
#define RXF_WATCHDOG_TIME 250 /* BARK_TIME(1000) /  */
	ulong watchdogTime = OSL_SYSUPTIME(); /* msec */
#endif
	dhd_pub_t *pub = &dhd->pub;

	/* This thread doesn't need any user-level access,
	 * so get rid of all our resources
	 */
	if (dhd_rxf_prio > 0)
	{
		struct sched_param param;
		param.sched_priority = (dhd_rxf_prio < MAX_RT_PRIO)?dhd_rxf_prio:(MAX_RT_PRIO-1);
		setScheduler(current, SCHED_FIFO, &param);
	}

	DAEMONIZE("dhd_rxf");
	/* DHD_OS_WAKE_LOCK is called in dhd_sched_dpc[dhd_linux.c] down below  */

	/*  signal: thread has started */
	complete(&tsk->completed);
#if defined(CUSTOMER_HW4) && defined(ARGOS_CPU_SCHEDULER)
	if (!zalloc_cpumask_var(&dhd->pub.rxf_affinity_cpu_mask, GFP_KERNEL)) {
		DHD_ERROR(("rxthread zalloc_cpumask_var error\n"));
		dhd->pub.affinity_isrxf = FALSE;
	} else {
		cpumask_or(dhd->pub.rxf_affinity_cpu_mask, dhd->pub.rxf_affinity_cpu_mask,
			cpumask_of(RXF_CPUCORE));

		DHD_GENERAL_LOCK(&dhd->pub, flags);
		if ((ret = argos_task_affinity_setup_label(current, "WIFI",
			dhd->pub.rxf_affinity_cpu_mask, dhd->pub.default_cpu_mask)) < 0) {
			DHD_ERROR(("Failed to add CPU affinity(rxf) error=%d\n", ret));
			dhd->pub.affinity_isrxf = FALSE;
			free_cpumask_var(dhd->pub.rxf_affinity_cpu_mask);
		} else {
			DHD_ERROR(("RXthread affinity completed\n"));
			dhd->pub.affinity_isrxf = TRUE;
		}
		DHD_GENERAL_UNLOCK(&dhd->pub, flags);
	}
#else /* CUSTOMER_HW4 && ARGOS_CPU_SCHEDULER */
#ifdef CUSTOM_SET_CPUCORE
	dhd->pub.current_rxf = current;
#endif /* CUSTOM_SET_CPUCORE */
#endif /* CUSTOMER_HW4 && ARGOS_CPU_SCHEDULER */
	/* Run until signal received */
	while (1) {
		if (down_interruptible(&tsk->sema) == 0) {
			void *skb;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0)
			ulong flags;
#endif
#ifdef ENABLE_ADAPTIVE_SCHED
			dhd_sched_policy(dhd_rxf_prio);
#endif /* ENABLE_ADAPTIVE_SCHED */
			if (tsk->terminated) {
				break;
			}
			skb = dhd_rxf_dequeue(pub);

			if (skb == NULL) {
				continue;
			}
			while (skb) {
				void *skbnext = PKTNEXT(pub->osh, skb);
				PKTSETNEXT(pub->osh, skb, NULL);
				bcm_object_trace_opr(skb, BCM_OBJDBG_REMOVE,
					__FUNCTION__, __LINE__);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0)
				netif_rx_ni(skb);
#else
				netif_rx(skb);
				local_irq_save(flags);
				RAISE_RX_SOFTIRQ();
				local_irq_restore(flags);

#endif
				skb = skbnext;
			}
#if defined(WAIT_DEQUEUE)
			if (OSL_SYSUPTIME() - watchdogTime > RXF_WATCHDOG_TIME) {
				OSL_SLEEP(1);
				watchdogTime = OSL_SYSUPTIME();
			}
#endif

			DHD_OS_WAKE_UNLOCK(pub);
		} else {
			break;
		}
	}
#if defined(CUSTOMER_HW4) && defined(ARGOS_CPU_SCHEDULER)
	if (dhd->pub.affinity_isrxf == TRUE) {
		free_cpumask_var(dhd->pub.rxf_affinity_cpu_mask);
		dhd->pub.affinity_isrxf = FALSE;
	}
#endif /* CUSTOMER_HW4 && ARGOS_CPU_SCHEDULER */
	complete_and_exit(&tsk->completed, 0);
}

#ifdef BCMPCIE
void
dhd_dpc_enable(dhd_pub_t *dhdp)
{
	dhd_info_t *dhd;

	if (!dhdp || !dhdp->info)
		return;

	dhd = dhdp->info;

#ifdef DHD_LB
#ifdef DHD_LB_RXP
	__skb_queue_head_init(&dhd->rx_pend_queue);
#endif /* DHD_LB_RXP */
#ifdef DHD_LB_TXC
	if (atomic_read(&dhd->tx_compl_tasklet.count) == 1)
		tasklet_enable(&dhd->tx_compl_tasklet);
#endif /* DHD_LB_TXC */
#ifdef DHD_LB_RXC
	if (atomic_read(&dhd->rx_compl_tasklet.count) == 1)
		tasklet_enable(&dhd->rx_compl_tasklet);
#endif /* DHD_LB_RXC */
#endif /* DHD_LB */
	if (atomic_read(&dhd->tasklet.count) ==  1)
		tasklet_enable(&dhd->tasklet);
}
#endif /* BCMPCIE */

#ifdef OEM_ANDROID
#ifdef BCMPCIE
void
dhd_dpc_kill(dhd_pub_t *dhdp)
{
	dhd_info_t *dhd;

	if (!dhdp) {
		return;
	}

	dhd = dhdp->info;

	if (!dhd) {
		return;
	}

	if (dhd->thr_dpc_ctl.thr_pid < 0) {
		tasklet_disable(&dhd->tasklet);
		tasklet_kill(&dhd->tasklet);
		DHD_ERROR(("%s: tasklet disabled\n", __FUNCTION__));
	}
#if defined(DHD_LB)
#ifdef DHD_LB_RXP
	skb_queue_purge(&dhd->rx_pend_queue);
#endif /* DHD_LB_RXP */
	/* Kill the Load Balancing Tasklets */
#if defined(DHD_LB_TXC)
	tasklet_disable(&dhd->tx_compl_tasklet);
	tasklet_kill(&dhd->tx_compl_tasklet);
#endif /* DHD_LB_TXC */
#if defined(DHD_LB_RXC)
	tasklet_disable(&dhd->rx_compl_tasklet);
	tasklet_kill(&dhd->rx_compl_tasklet);
#endif /* DHD_LB_RXC */
#endif /* DHD_LB */
}
#endif /* BCMPCIE */
#endif /* OEM_ANDROID */

static void
dhd_dpc(ulong data)
{
	dhd_info_t *dhd;

	dhd = (dhd_info_t *)data;
	DHD_OS_WAKE_LOCK(&dhd->pub);

	/* this (tasklet) can be scheduled in dhd_sched_dpc[dhd_linux.c]
	 * down below , wake lock is set,
	 * the tasklet is initialized in dhd_attach()
	 */
	/* Call bus dpc unless it indicated down (then clean stop) */
	if (dhd->pub.busstate != DHD_BUS_DOWN) {
		if (dhd_bus_dpc(dhd->pub.bus)) {
			DHD_LB_STATS_INCR(dhd->dhd_dpc_cnt);
			tasklet_schedule(&dhd->tasklet);
		}
	} else {
		dhd_bus_stop(dhd->pub.bus, TRUE);
	}
	DHD_OS_WAKE_UNLOCK(&dhd->pub);
}

void
dhd_sched_dpc(dhd_pub_t *dhdp)
{
	dhd_info_t *dhd = (dhd_info_t *)dhdp->info;

	if (dhd->thr_dpc_ctl.thr_pid >= 0) {
		DHD_OS_WAKE_LOCK(dhdp);
		/* If the semaphore does not get up,
		* wake unlock should be done here
		*/
		if (!binary_sema_up(&dhd->thr_dpc_ctl)) {
			DHD_OS_WAKE_UNLOCK(dhdp);
		}

		return;
	} else {
		if (!test_bit(TASKLET_STATE_SCHED, &dhd->tasklet.state)) {
			DHD_OS_WAKE_LOCK(dhdp);
		}

		tasklet_schedule(&dhd->tasklet);
		DHD_OS_WAKE_UNLOCK(dhdp);
	}
}
#endif /* BCMDBUS */

static void
dhd_sched_rxf(dhd_pub_t *dhdp, void *skb)
{
	dhd_info_t *dhd = (dhd_info_t *)dhdp->info;
#ifdef RXF_DEQUEUE_ON_BUSY
	int ret = BCME_OK;
	int retry = 2;
#endif /* RXF_DEQUEUE_ON_BUSY */

	DHD_OS_WAKE_LOCK(dhdp);

	DHD_TRACE(("dhd_sched_rxf: Enter\n"));
#ifdef RXF_DEQUEUE_ON_BUSY
	do {
		ret = dhd_rxf_enqueue(dhdp, skb);
		if (ret == BCME_OK || ret == BCME_ERROR)
			break;
		else
			OSL_SLEEP(50); /* waiting for dequeueing */
	} while (retry-- > 0);

	if (retry <= 0 && ret == BCME_BUSY) {
		void *skbp = skb;

		while (skbp) {
			void *skbnext = PKTNEXT(dhdp->osh, skbp);
			PKTSETNEXT(dhdp->osh, skbp, NULL);
			bcm_object_trace_opr(skb, BCM_OBJDBG_REMOVE,
				__FUNCTION__, __LINE__);
			netif_rx_ni(skbp);
			skbp = skbnext;
		}
		DHD_ERROR(("send skb to kernel backlog without rxf_thread\n"));
	} else {
		if (dhd->thr_rxf_ctl.thr_pid >= 0) {
			up(&dhd->thr_rxf_ctl.sema);
		}
	}
#else /* RXF_DEQUEUE_ON_BUSY */
	do {
		if (dhd_rxf_enqueue(dhdp, skb) == BCME_OK)
			break;
	} while (1);
	if (dhd->thr_rxf_ctl.thr_pid >= 0) {
		up(&dhd->thr_rxf_ctl.sema);
	}

	return;
#endif /* RXF_DEQUEUE_ON_BUSY */
}

#if defined(BCM_DNGL_EMBEDIMAGE) || defined(BCM_REQUEST_FW)
#if defined(BCMDBUS)
static int
fw_download_thread_func(void *data)
{
	dhd_info_t *dhd = (dhd_info_t *)data;
	int ret;

	while (1) {
		/* Wait for start trigger */
		if (down_interruptible(&dhd->fw_download_lock) != 0)
			return -ERESTARTSYS;

		if (kthread_should_stop())
			break;

		DHD_TRACE(("%s: initiating firmware check and download\n", __FUNCTION__));
		if (dbus_download_firmware(dhd->pub.dbus) == DBUS_OK) {
			if ((ret = dbus_up(dhd->pub.dbus)) == 0) {
#ifdef PROP_TXSTATUS
				/* Need to deinitialise WLFC to allow re-initialisation later. */
				dhd_wlfc_deinit(&dhd->pub);
#endif /* PROP_TXSTATUS */
				/* Resynchronise with the dongle. This also re-initialises WLFC. */
				if ((ret = dhd_sync_with_dongle(&dhd->pub)) < 0) {
					DHD_ERROR(("%s: failed with code %d\n", __FUNCTION__, ret));
				}
			}
		}
	}

	return 0;
}
#endif /* BCMDBUS */
#endif /* defined(BCM_DNGL_EMBEDIMAGE) || defined(BCM_REQUEST_FW) */

#ifdef TOE
/* Retrieve current toe component enables, which are kept as a bitmap in toe_ol iovar */
static int
dhd_toe_get(dhd_info_t *dhd, int ifidx, uint32 *toe_ol)
{
	char buf[32];
	int ret;

	ret = dhd_iovar(&dhd->pub, ifidx, "toe_ol", NULL, 0, (char *)&buf, sizeof(buf), FALSE);

	if (ret < 0) {
		if (ret == -EIO) {
			DHD_ERROR(("%s: toe not supported by device\n", dhd_ifname(&dhd->pub,
				ifidx)));
			return -EOPNOTSUPP;
		}

		DHD_INFO(("%s: could not get toe_ol: ret=%d\n",
			dhd_ifname(&dhd->pub, ifidx), ret));
		return ret;
	}

	memcpy(toe_ol, buf, sizeof(uint32));
	return 0;
}

/* Set current toe component enables in toe_ol iovar, and set toe global enable iovar */
static int
dhd_toe_set(dhd_info_t *dhd, int ifidx, uint32 toe_ol)
{
	int toe, ret;

	/* Set toe_ol as requested */
	ret = dhd_iovar(&dhd->pub, ifidx, "toe_ol", (char *)&toe_ol, sizeof(toe_ol), NULL, 0, TRUE);
	if (ret < 0) {
		DHD_ERROR(("%s: could not set toe_ol: ret=%d\n",
			dhd_ifname(&dhd->pub, ifidx), ret));
		return ret;
	}

	/* Enable toe globally only if any components are enabled. */

	toe = (toe_ol != 0);

	ret = dhd_iovar(&dhd->pub, ifidx, "toe", (char *)&toe, sizeof(toe), NULL, 0, TRUE);
	if (ret < 0) {
		DHD_ERROR(("%s: could not set toe: ret=%d\n", dhd_ifname(&dhd->pub, ifidx), ret));
		return ret;
	}

	return 0;
}
#endif /* TOE */

#if defined(WL_CFG80211) && defined(NUM_SCB_MAX_PROBE)
void
dhd_set_scb_probe(dhd_pub_t *dhd)
{
	int ret = 0;
	wl_scb_probe_t scb_probe;
	char iovbuf[WL_EVENTING_MASK_LEN + sizeof(wl_scb_probe_t)];

	if (dhd->wlcore->op_mode & DHD_FLAG_HOSTAP_MODE) {
		return;
	}

	ret = dhd_iovar(dhd, 0, "scb_probe", NULL, 0, iovbuf, sizeof(iovbuf), FALSE);
	if (ret < 0) {
		DHD_ERROR(("%s: GET max_scb_probe failed\n", __FUNCTION__));
	}

	memcpy(&scb_probe, iovbuf, sizeof(wl_scb_probe_t));

	scb_probe.scb_max_probe = NUM_SCB_MAX_PROBE;

	ret = dhd_iovar(dhd, 0, "scb_probe", (char *)&scb_probe, sizeof(wl_scb_probe_t), NULL, 0,
			TRUE);
	if (ret < 0) {
		DHD_ERROR(("%s: max_scb_probe setting failed\n", __FUNCTION__));
		return;
	}
}
#endif /* WL_CFG80211 && NUM_SCB_MAX_PROBE */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24)
static void
dhd_ethtool_get_drvinfo(struct net_device *net, struct ethtool_drvinfo *info)
{
	dhd_info_t *dhd = DHD_DEV_INFO(net);

	snprintf(info->driver, sizeof(info->driver), "wl");
	snprintf(info->version, sizeof(info->version), "%lu", dhd->pub.drv_version);
}

struct ethtool_ops dhd_ethtool_ops = {
	.get_drvinfo = dhd_ethtool_get_drvinfo
};
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24) */

#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 4, 2)
static int
dhd_ethtool(dhd_info_t *dhd, void *uaddr)
{
	struct ethtool_drvinfo info;
	char drvname[sizeof(info.driver)] = { 0 };
	uint32 cmd;
#ifdef TOE
	struct ethtool_value edata;
	uint32 toe_cmpnt, csum_dir;
	int ret;
#endif

	DHD_TRACE(("%s: Enter\n", __FUNCTION__));

	/* all ethtool calls start with a cmd word */
	if (copy_from_user(&cmd, uaddr, sizeof (uint32)))
		return -EFAULT;

	switch (cmd) {
	case ETHTOOL_GDRVINFO:
		/* Copy out any request driver name */
		if (copy_from_user(&info, uaddr, sizeof(info)))
			return -EFAULT;
		strncpy(drvname, info.driver, sizeof(drvname));
		drvname[sizeof(info.driver)-1] = '\0';

		/* clear struct for return */
		memset(&info, 0, sizeof(info));
		info.cmd = cmd;

		/* if dhd requested, identify ourselves */
		if (strcmp(drvname, "?dhd") == 0) {
			snprintf(info.driver, sizeof(info.driver), "dhd");
			strncpy(info.version, EPI_VERSION_STR, sizeof(info.version) - 1);
			info.version[sizeof(info.version) - 1] = '\0';
		}

		/* otherwise, require dongle to be up */
		else if (!dhd->pub.up) {
			DHD_ERROR(("%s: dongle is not up\n", __FUNCTION__));
			return -ENODEV;
		}

		/* finally, report dongle driver type */
		else if (dhd->pub.iswl)
			snprintf(info.driver, sizeof(info.driver), "wl");
		else
			snprintf(info.driver, sizeof(info.driver), "xx");

		snprintf(info.version, sizeof(info.version), "%lu", dhd->pub.drv_version);
		if (copy_to_user(uaddr, &info, sizeof(info)))
			return -EFAULT;
		DHD_CTL(("%s: given %*s, returning %s\n", __FUNCTION__,
		         (int)sizeof(drvname), drvname, info.driver));
		break;

#ifdef TOE
	/* Get toe offload components from dongle */
	case ETHTOOL_GRXCSUM:
	case ETHTOOL_GTXCSUM:
		if ((ret = dhd_toe_get(dhd, 0, &toe_cmpnt)) < 0)
			return ret;

		csum_dir = (cmd == ETHTOOL_GTXCSUM) ? TOE_TX_CSUM_OL : TOE_RX_CSUM_OL;

		edata.cmd = cmd;
		edata.data = (toe_cmpnt & csum_dir) ? 1 : 0;

		if (copy_to_user(uaddr, &edata, sizeof(edata)))
			return -EFAULT;
		break;

	/* Set toe offload components in dongle */
	case ETHTOOL_SRXCSUM:
	case ETHTOOL_STXCSUM:
		if (copy_from_user(&edata, uaddr, sizeof(edata)))
			return -EFAULT;

		/* Read the current settings, update and write back */
		if ((ret = dhd_toe_get(dhd, 0, &toe_cmpnt)) < 0)
			return ret;

		csum_dir = (cmd == ETHTOOL_STXCSUM) ? TOE_TX_CSUM_OL : TOE_RX_CSUM_OL;

		if (edata.data != 0)
			toe_cmpnt |= csum_dir;
		else
			toe_cmpnt &= ~csum_dir;

		if ((ret = dhd_toe_set(dhd, 0, toe_cmpnt)) < 0)
			return ret;

		/* If setting TX checksum mode, tell Linux the new mode */
		if (cmd == ETHTOOL_STXCSUM) {
			if (edata.data)
				dhd->iflist[0]->net->features |= NETIF_F_IP_CSUM;
			else
				dhd->iflist[0]->net->features &= ~NETIF_F_IP_CSUM;
		}

		break;
#endif /* TOE */

	default:
		return -EOPNOTSUPP;
	}

	return 0;
}
#endif /* LINUX_VERSION_CODE > KERNEL_VERSION(2, 4, 2) */

static bool
dhd_check_hang(struct net_device *net, dhd_pub_t *dhdp, int error)
{
#if defined(OEM_ANDROID)
	dhd_info_t *dhd;

	if (!dhdp) {
		DHD_ERROR(("%s: dhdp is NULL\n", __FUNCTION__));
		return FALSE;
	}

	if (!dhdp->up)
		return FALSE;

	dhd = (dhd_info_t *)dhdp->info;
#if (!defined(BCMDBUS) && !defined(BCMPCIE))
	if (dhd->thr_dpc_ctl.thr_pid < 0) {
		DHD_ERROR(("%s : skipped due to negative pid - unloading?\n", __FUNCTION__));
		return FALSE;
	}
#endif /* BCMDBUS */

	if ((error == -ETIMEDOUT) || (error == -EREMOTEIO) ||
		((dhdp->busstate == DHD_BUS_DOWN) && (!dhdp->dongle_reset))) {
#ifdef BCMPCIE
		DHD_ERROR(("%s: Event HANG send up due to  re=%d te=%d d3acke=%d e=%d s=%d\n",
			__FUNCTION__, dhdp->rxcnt_timeout, dhdp->txcnt_timeout,
			dhdp->d3ackcnt_timeout, error, dhdp->busstate));
		/* This is required only for Brix Android platform in Debug builds */
#if defined(DHD_FW_COREDUMP) && !defined(CUSTOMER_HW4) && defined(DHD_DEBUG)
		/* For dongle TRAP fw mem_dump is called from dhdpcie_checkdied.
		 * Avoid race condition by checking if the TRAP has occurred.
		 */
		if (!dhdp->dongle_trap_occured) {
#if defined(STB) || defined(STBAP)
			dhdp->memdump_enabled = DUMP_MEMFILE;
#else /* STB */
			/* Load the dongle side dump to a file and then BUG_ON() */
			dhdp->memdump_enabled = DUMP_MEMFILE_BUGON;
#endif /* STB */
			dhd_bus_mem_dump(dhdp);
		}
#endif /* DHD_FW_COREDUMP && !CUSTOMER_HW4 && DHD_DEBUG */
#else
		DHD_ERROR(("%s: Event HANG send up due to  re=%d te=%d e=%d s=%d\n", __FUNCTION__,
			dhdp->rxcnt_timeout, dhdp->txcnt_timeout, error, dhdp->busstate));
#endif /* BCMPCIE */
		net_os_send_hang_message(net);
		return TRUE;
	}
#endif /* OEM_ANDROID */

	return FALSE;
}

int
dhd_ioctl_process(dhd_pub_t *pub, int ifidx, dhd_ioctl_t *ioc, void *data_buf)
{
	int bcmerror = BCME_OK;
	int buflen = 0;
	struct net_device *net;

	net = dhd_idx2net(pub, ifidx);
	if (!net) {
		bcmerror = BCME_BADARG;
		goto done;
	}

	/* check for local dhd ioctl and handle it */
	if (ioc->driver == DHD_IOCTL_MAGIC) {
		if (data_buf)
			buflen = MIN(ioc->len, DHD_IOCTL_MAXLEN_HIGH);
		bcmerror = dhd_ioctl((void *)pub, ioc, data_buf, buflen);
		if (bcmerror)
			pub->bcmerror = bcmerror;
		goto done;
	}

	/* This is a WL IOVAR, truncate buflen to WLC_IOCTL_MAXLEN */
	if (data_buf)
		buflen = MIN(ioc->len, WLC_IOCTL_MAXLEN);
#ifndef BCMDBUS
	/* send to dongle (must be up, and wl). */
	if (pub->busstate != DHD_BUS_DATA) {
		if (allow_delay_fwdl) {
			int ret = dhd_bus_start(pub);
			if (ret != 0) {
				DHD_ERROR(("%s: failed with code %d\n", __FUNCTION__, ret));
				bcmerror = BCME_DONGLE_DOWN;
				goto done;
			}
		} else {
			bcmerror = BCME_DONGLE_DOWN;
			goto done;
		}
	}

	if (!pub->iswl) {
		bcmerror = BCME_DONGLE_DOWN;
		goto done;
	}
#endif /* BCMDBUS */

	/*
	 * Flush the TX queue if required for proper message serialization:
	 * Intercept WLC_SET_KEY IOCTL - serialize M4 send and set key IOCTL to
	 * prevent M4 encryption and
	 * intercept WLC_DISASSOC IOCTL - serialize WPS-DONE and WLC_DISASSOC IOCTL to
	 * prevent disassoc frame being sent before WPS-DONE frame.
	 */
	if (ioc->cmd == WLC_SET_KEY ||
	    (ioc->cmd == WLC_SET_VAR && data_buf != NULL &&
	     strncmp("wsec_key", data_buf, 9) == 0) ||
	    (ioc->cmd == WLC_SET_VAR && data_buf != NULL &&
	     strncmp("bsscfg:wsec_key", data_buf, 15) == 0) ||
	    ioc->cmd == WLC_DISASSOC)
		dhd_wait_pend8021x(net);

#ifdef WLMEDIA_HTSF
	if (data_buf) {
		/*  short cut wl ioctl calls here  */
		if (strcmp("htsf", data_buf) == 0) {
			dhd_ioctl_htsf_get(dhd, 0);
			return BCME_OK;
		}

		if (strcmp("htsflate", data_buf) == 0) {
			if (ioc->set) {
				memset(ts, 0, sizeof(tstamp_t)*TSMAX);
				memset(&maxdelayts, 0, sizeof(tstamp_t));
				maxdelay = 0;
				tspktcnt = 0;
				maxdelaypktno = 0;
				memset(&vi_d1.bin, 0, sizeof(uint32)*NUMBIN);
				memset(&vi_d2.bin, 0, sizeof(uint32)*NUMBIN);
				memset(&vi_d3.bin, 0, sizeof(uint32)*NUMBIN);
				memset(&vi_d4.bin, 0, sizeof(uint32)*NUMBIN);
			} else {
				dhd_dump_latency();
			}

			return BCME_OK;
		}
		if (strcmp("htsfclear", data_buf) == 0) {
			memset(&vi_d1.bin, 0, sizeof(uint32)*NUMBIN);
			memset(&vi_d2.bin, 0, sizeof(uint32)*NUMBIN);
			memset(&vi_d3.bin, 0, sizeof(uint32)*NUMBIN);
			memset(&vi_d4.bin, 0, sizeof(uint32)*NUMBIN);
			htsf_seqnum = 0;
			return BCME_OK;
		}
		if (strcmp("htsfhis", data_buf) == 0) {
			dhd_dump_htsfhisto(&vi_d1, "H to D");
			dhd_dump_htsfhisto(&vi_d2, "D to D");
			dhd_dump_htsfhisto(&vi_d3, "D to H");
			dhd_dump_htsfhisto(&vi_d4, "H to H");
			return BCME_OK;
		}
		if (strcmp("tsport", data_buf) == 0) {
			if (ioc->set) {
				memcpy(&tsport, data_buf + 7, 4);
			} else {
				DHD_ERROR(("current timestamp port: %d \n", tsport));
			}
			return BCME_OK;
		}
	}
#endif /* WLMEDIA_HTSF */

	if ((ioc->cmd == WLC_SET_VAR || ioc->cmd == WLC_GET_VAR) &&
		data_buf != NULL && strncmp("rpc_", data_buf, 4) == 0) {
#ifdef BCM_FD_AGGR
		bcmerror = dhd_fdaggr_ioctl(pub, ifidx, (wl_ioctl_t *)ioc, data_buf, buflen);
#else
		bcmerror = BCME_UNSUPPORTED;
#endif
		goto done;
	}
	bcmerror = dhd_wl_ioctl(pub, ifidx, (wl_ioctl_t *)ioc, data_buf, buflen);

	/* Intercept monitor ioctl here, add/del monitor if */
#ifdef WL_MONITOR
	if (bcmerror == BCME_OK && ioc->cmd == WLC_SET_MONITOR) {
		dhd_set_monitor(pub, ifidx, *(int32*)data_buf);
	}
#endif /* WL_MONITOR */

done:
#if defined(OEM_ANDROID)
	dhd_check_hang(net, pub, bcmerror);
#endif /* OEM_ANDROID */

	return bcmerror;
}

/**
 * Called by the OS (optionally via a wrapper function).
 * @param net  Linux per dongle instance
 * @param ifr  Linux request structure
 * @param cmd  e.g. SIOCETHTOOL
 */
static int
dhd_ioctl_entry(struct net_device *net, struct ifreq *ifr, int cmd)
{
	dhd_info_t *dhd = DHD_DEV_INFO(net);
	dhd_ioctl_t ioc;
	int bcmerror = 0;
	int ifidx;
	int ret;
	void *local_buf = NULL;           /**< buffer in kernel space */
	u16 buflen = 0;

	memset(&ioc, 0, sizeof(ioc));

#ifdef CONFIG_COMPAT
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 6, 0))
	if (in_compat_syscall()) {
#else
	if (is_compat_task()) {
#endif /* LINUX_VER >= 4.6 */
		compat_wl_ioctl_t compat_ioc;
		if (copy_from_user(&compat_ioc, ifr->ifr_data, sizeof(compat_wl_ioctl_t))) {
			bcmerror = BCME_BADADDR;
			goto done;
		}
		ioc.cmd = compat_ioc.cmd;
		ioc.buf = compat_ptr(compat_ioc.buf);
		ioc.len = compat_ioc.len;
		ioc.set = compat_ioc.set;
		ioc.used = compat_ioc.used;
		ioc.needed = compat_ioc.needed;
		/* To differentiate between wl and dhd read 4 more byes */
		if ((copy_from_user(&ioc.driver, (char *)ifr->ifr_data + sizeof(compat_wl_ioctl_t),
			sizeof(uint)) != 0)) {
			bcmerror = BCME_BADADDR;
			goto done;
		}
	} else
#endif /* CONFIG_COMPAT */
	{
		/* Copy the ioc control structure part of ioctl request */
		if (copy_from_user(&ioc, ifr->ifr_data, sizeof(wl_ioctl_t))) {
			bcmerror = BCME_BADADDR;
			goto done;
		}

		/* To differentiate between wl and dhd read 4 more byes */
		if ((copy_from_user(&ioc.driver, (char *)ifr->ifr_data + sizeof(wl_ioctl_t),
			sizeof(uint)) != 0)) {
			bcmerror = BCME_BADADDR;
			goto done;
		}
	}

	if (!capable(CAP_NET_ADMIN)) {
		bcmerror = BCME_EPERM;
		goto done;
	}

	if (ioc.driver == WLC_IOCTL_MAGIC && dhd->pub.busstate != DHD_BUS_DATA &&
#ifndef BCMDBUS
	    allow_delay_fwdl == FALSE &&
#endif /* !BCMDBUS */
	    TRUE) {
		bcmerror = BCME_NODEVICE;
		goto done;
	}

	if (ioc.len > 0) {
		buflen = MIN(ioc.len, DHD_IOCTL_MAXLEN_HIGH);

		if (!(local_buf = IOCTL_MALLOC(dhd->pub.osh, ioc.set, buflen+1))) {
			bcmerror = BCME_NOMEM;
			goto done;
		}

		if (copy_from_user(local_buf, ioc.buf, buflen)) {
			bcmerror = BCME_BADADDR;
			goto done;
		}

		*((char *)local_buf + buflen) = '\0';
	}

	DHD_OS_WAKE_LOCK(&dhd->pub);
	DHD_LOCK(&dhd->pub);

#if defined(OEM_ANDROID)
	/* Interface up check for built-in type */
	if (!dhd_download_fw_on_driverload && dhd->pub.up == FALSE) {
		DHD_TRACE(("%s: Interface is down \n", __FUNCTION__));
		DHD_UNLOCK(&dhd->pub);
		DHD_OS_WAKE_UNLOCK(&dhd->pub);
		bcmerror = BCME_NOTUP;
		goto done;
	}

	/* send to dongle only if we are not waiting for reload already */
	if (dhd->pub.hang_was_sent) {
		DHD_TRACE(("%s: HANG was sent up earlier\n", __FUNCTION__));
		DHD_OS_WAKE_LOCK_CTRL_TIMEOUT_ENABLE(&dhd->pub, DHD_EVENT_TIMEOUT_MS);
		DHD_OS_WAKE_UNLOCK(&dhd->pub);
		bcmerror = BCME_DONGLE_DOWN;
		goto done;
	}
#endif /* (OEM_ANDROID) */

	ifidx = dhd_net2idx(dhd, net);
	DHD_TRACE(("%s: ifidx %d, cmd 0x%04x\n", __FUNCTION__, ifidx, cmd));

	if (ifidx == DHD_BAD_IF) {
		DHD_ERROR(("%s: BAD IF\n", __FUNCTION__));
		DHD_UNLOCK(&dhd->pub);
		DHD_OS_WAKE_UNLOCK(&dhd->pub);
		IOCTL_MFREE(dhd->pub.osh, ioc.set, local_buf, buflen+1);
		return -1;
	}

#if defined(WL_WIRELESS_EXT)
	/* linux wireless extensions */
	if ((cmd >= SIOCIWFIRST) && (cmd <= SIOCIWLAST)) {
		/* may recurse, do NOT lock */
		ret = wl_iw_ioctl(net, ifr, cmd);
		DHD_UNLOCK(&dhd->pub);
		DHD_OS_WAKE_UNLOCK(&dhd->pub);
		IOCTL_MFREE(dhd->pub.osh, ioc.set, local_buf, buflen+1);
		return ret;
	}
#endif /* defined(WL_WIRELESS_EXT) */

#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 4, 2)
	if (cmd == SIOCETHTOOL) {
		ret = dhd_ethtool(dhd, (void*)ifr->ifr_data);
		DHD_UNLOCK(&dhd->pub);
		DHD_OS_WAKE_UNLOCK(&dhd->pub);
		IOCTL_MFREE(dhd->pub.osh, ioc.set, local_buf, buflen+1);
		return ret;
	}
#endif /* LINUX_VERSION_CODE > KERNEL_VERSION(2, 4, 2) */

#if defined(OEM_ANDROID)
	if (cmd == SIOCDEVPRIVATE+1) {
		ret = wl_android_priv_cmd(net, ifr, cmd);
		dhd_check_hang(net, &dhd->pub, ret);
		DHD_UNLOCK(&dhd->pub);
		DHD_OS_WAKE_UNLOCK(&dhd->pub);
		IOCTL_MFREE(dhd->pub.osh, ioc.set, local_buf, buflen+1);
		return ret;
	}
#endif /* OEM_ANDROID */

	if (cmd != SIOCDEVPRIVATE) {
		DHD_UNLOCK(&dhd->pub);
		DHD_OS_WAKE_UNLOCK(&dhd->pub);
		IOCTL_MFREE(dhd->pub.osh, ioc.set, local_buf, buflen+1);
		return -EOPNOTSUPP;
	}

	bcmerror = dhd_ioctl_process(&dhd->pub, ifidx, &ioc, local_buf);

	DHD_UNLOCK(&dhd->pub);
	DHD_OS_WAKE_UNLOCK(&dhd->pub);

	if (!bcmerror && buflen && local_buf && ioc.buf) {
		if (copy_to_user(ioc.buf, local_buf, buflen))
			bcmerror = -EFAULT;
	}

done:
	if (local_buf)
		IOCTL_MFREE(dhd->pub.osh, ioc.set, local_buf, buflen+1);

	return OSL_ERROR(bcmerror);
}

#if defined(WL_CFG80211) && defined(SUPPORT_DEEP_SLEEP)
/* Flags to indicate if we distingish power off policy when
 * user set the memu "Keep Wi-Fi on during sleep" to "Never"
 */
int trigger_deep_sleep = 0;
#endif /* WL_CFG80211 && SUPPORT_DEEP_SLEEP */

#ifdef CUSTOMER_HW4
#ifdef FIX_CPU_MIN_CLOCK
static int
dhd_init_cpufreq_fix(dhd_info_t *dhd)
{
	if (dhd) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 25)) && defined(OEM_ANDROID)
		mutex_init(&dhd->cpufreq_fix);
#endif
		dhd->cpufreq_fix_status = FALSE;
	}

	return 0;
}

static void
dhd_fix_cpu_freq(dhd_info_t *dhd)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 25)) && defined(OEM_ANDROID)
	mutex_lock(&dhd->cpufreq_fix);
#endif
	if (dhd && !dhd->cpufreq_fix_status) {
		pm_qos_add_request(&dhd->dhd_cpu_qos, PM_QOS_CPU_FREQ_MIN, 300000);
#ifdef FIX_BUS_MIN_CLOCK
		pm_qos_add_request(&dhd->dhd_bus_qos, PM_QOS_BUS_THROUGHPUT, 400000);
#endif /* FIX_BUS_MIN_CLOCK */
		DHD_ERROR(("pm_qos_add_requests called\n"));

		dhd->cpufreq_fix_status = TRUE;
	}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 25)) && defined(OEM_ANDROID)
	mutex_unlock(&dhd->cpufreq_fix);
#endif
}

static void
dhd_rollback_cpu_freq(dhd_info_t *dhd)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 25)) && defined(OEM_ANDROID)
	mutex_lock(&dhd ->cpufreq_fix);
#endif
	if (dhd && dhd->cpufreq_fix_status != TRUE) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 25)) && defined(OEM_ANDROID)
		mutex_unlock(&dhd->cpufreq_fix);
#endif
		return;
	}

	pm_qos_remove_request(&dhd->dhd_cpu_qos);
#ifdef FIX_BUS_MIN_CLOCK
	pm_qos_remove_request(&dhd->dhd_bus_qos);
#endif /* FIX_BUS_MIN_CLOCK */
	DHD_ERROR(("pm_qos_add_requests called\n"));

	dhd->cpufreq_fix_status = FALSE;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 25)) && defined(OEM_ANDROID)
	mutex_unlock(&dhd->cpufreq_fix);
#endif
}
#endif /* FIX_CPU_MIN_CLOCK */
#endif /* CUSTOMER_HW4 */

static int
dhd_stop(struct net_device *net)
{
	int ifidx = 0;
	dhd_info_t *dhd = DHD_DEV_INFO(net);
	DHD_OS_WAKE_LOCK(&dhd->pub);
	DHD_LOCK(&dhd->pub);
	DHD_TRACE(("%s: Enter %p\n", __FUNCTION__, net));
	dhd->pub.rxcnt_timeout = 0;
	dhd->pub.txcnt_timeout = 0;

#ifdef BCMPCIE
	dhd->pub.d3ackcnt_timeout = 0;
#endif /* BCMPCIE */

	if (dhd->pub.up == 0) {
		goto exit;
	}

#if defined(CUSTOMER_HW4) && defined(FIX_CPU_MIN_CLOCK)
	if (dhd_get_fw_mode(dhd) == DHD_FLAG_HOSTAP_MODE)
		dhd_rollback_cpu_freq(dhd);
#endif /* defined(CUSTOMER_HW4) && defined(FIX_CPU_MIN_CLOCK) */

	ifidx = dhd_net2idx(dhd, net);
	BCM_REFERENCE(ifidx);

	/* Set state and stop OS transmissions */
	netif_stop_queue(net);
	dhd->pub.up = 0;

#ifdef WL_CFG80211
	if ((dhd->dhd_state & DHD_ATTACH_STATE_CFG80211) && (ifidx == 0)) {
		dhd_if_t *ifp;
		/* Unlock the locks as we take the lock again in subsequent function call. */
		DHD_UNLOCK(&dhd->pub);
		DHD_OS_WAKE_UNLOCK(&dhd->pub);
		wl_cfg80211_down(net);

		DHD_OS_WAKE_LOCK(&dhd->pub);
		DHD_LOCK(&dhd->pub);
		ifp = dhd->iflist[0];
		ASSERT(ifp && ifp->net);
		/*
		 * For CFG80211: Clean up all the left over virtual interfaces
		 * when the primary Interface is brought down. [ifconfig wlan0 down]
		 */
		if (!dhd_download_fw_on_driverload) {
			if ((dhd->dhd_state & DHD_ATTACH_STATE_ADD_IF)) {
				int i;

#if defined(CUSTOMER_HW4) && defined(WL_CFG80211_P2P_DEV_IF)
				wl_cfg80211_del_p2p_wdev(net);
#endif /* CUSTOMER_HW4 && WL_CFG80211_P2P_DEV_IF */

				dhd_net_if_lock_local(dhd);
				for (i = 1; i < DHD_MAX_IFS; i++)
					dhd_remove_if(&dhd->pub, i, FALSE);

				if (ifp && ifp->net) {
					dhd_if_del_sta_list(ifp);
				}

#ifdef ARP_OFFLOAD_SUPPORT
				if (dhd_inetaddr_notifier_registered) {
					dhd_inetaddr_notifier_registered = FALSE;
					unregister_inetaddr_notifier(&dhd_inetaddr_notifier);
				}
#endif /* ARP_OFFLOAD_SUPPORT */
#if defined(OEM_ANDROID) && defined(CONFIG_IPV6)
				if (dhd_inet6addr_notifier_registered) {
					dhd_inet6addr_notifier_registered = FALSE;
					unregister_inet6addr_notifier(&dhd_inet6addr_notifier);
				}
#endif /* OEM_ANDROID && CONFIG_IPV6 */
				dhd_net_if_unlock_local(dhd);
			}
			cancel_work_sync(dhd->dhd_deferred_wq);
#if defined(DHD_LB) && defined(DHD_LB_RXP)
			skb_queue_purge(&dhd->rx_pend_queue);
#endif /* DHD_LB && DHD_LB_RXP */
		}

#if defined(DHD_LB) && defined(DHD_LB_RXP)
		if (ifp->net == dhd->rx_napi_netdev) {
			DHD_INFO(("%s napi<%p> disabled ifp->net<%p, %s>\n",
				__FUNCTION__, &dhd->rx_napi_struct, net, net->name));
			skb_queue_purge(&dhd->rx_napi_queue);
			napi_disable(&dhd->rx_napi_struct);
			netif_napi_del(&dhd->rx_napi_struct);
			dhd->rx_napi_netdev = NULL;
		}
#endif /* DHD_LB && DHD_LB_RXP */

	}
#endif /* WL_CFG80211 */

#ifdef PROP_TXSTATUS
	dhd_wlfc_cleanup(&dhd->pub, NULL, 0);
#endif
	/* Stop the protocol module */
	dhd_prot_stop(&dhd->pub);

	OLD_MOD_DEC_USE_COUNT;
exit:
#if defined(WL_CFG80211) && defined(OEM_ANDROID)
	if (ifidx == 0 && !dhd_download_fw_on_driverload)
		wl_android_wifi_off(net, TRUE);
#ifdef SUPPORT_DEEP_SLEEP
	else {
		/* CSP#505233: Flags to indicate if we distingish
		 * power off policy when user set the memu
		 * "Keep Wi-Fi on during sleep" to "Never"
		 */
		if (trigger_deep_sleep) {
			dhd_deepsleep(net, 1);
			trigger_deep_sleep = 0;
		}
	}
#endif /* SUPPORT_DEEP_SLEEP */
#endif /* defined(WL_CFG80211) && defined(OEM_ANDROID) */
	dhd->pub.hang_was_sent = 0;

	/* Clear country spec for for built-in type driver */
	if (!dhd_download_fw_on_driverload) {
		dhd->pub.dhd_cspec.country_abbrev[0] = 0x00;
		dhd->pub.dhd_cspec.rev = 0;
		dhd->pub.dhd_cspec.ccode[0] = 0x00;
	}

#ifdef DHD_DEBUGFS
	dhd_dbg_remove();
#endif /* DHD_DEBUGFS */

	DHD_UNLOCK(&dhd->pub);
	DHD_OS_WAKE_UNLOCK(&dhd->pub);

	return 0;
}

static void
dhd_uninit(struct net_device *dev)
{
#if defined(BCM_CPE_PKTC)
	/* wipe out entire hot brc table */
	dhd_pktc_req(PKTC_TBL_FLUSH, 0, 0, 0);
#endif /* BCM_CPE_PKTC */

	return;
}
#if defined(OEM_ANDROID) && defined(WL_CFG80211) && (defined(USE_INITIAL_2G_SCAN) || \
	defined(USE_INITIAL_SHORT_DWELL_TIME))
extern bool g_first_broadcast_scan;
#endif /* OEM_ANDROID && WL_CFG80211 && (USE_INITIAL_2G_SCAN || USE_INITIAL_SHORT_DWELL_TIME) */

#ifdef WL11U
static int
dhd_interworking_enable(dhd_pub_t *dhd)
{
	uint32 enable = true;
	int ret = BCME_OK;

	ret = dhd_iovar(dhd, 0, "interworking", (char *)&enable, sizeof(enable), NULL, 0, TRUE);
	if (ret < 0) {
		DHD_ERROR(("%s: enableing interworking failed, ret=%d\n", __FUNCTION__, ret));
	}

	if (ret == BCME_OK) {
		/* basic capabilities for HS20 REL2 */
		uint32 cap = WL_WNM_BSSTRANS | WL_WNM_NOTIF;
		ret = dhd_iovar(dhd, 0, "wnm", (char *)&cap, sizeof(cap), NULL, 0, TRUE);
		if (ret < 0) {
			DHD_ERROR(("%s: failed to set WNM info, ret=%d\n", __FUNCTION__, ret));
		}
	}

	return ret;
}
#endif /* WL11u */

static int
dhd_open(struct net_device *net)
{
	dhd_info_t *dhd = DHD_DEV_INFO(net);
#ifdef TOE
	uint32 toe_ol;
#endif
#ifdef BCM_FD_AGGR
	char iovbuf[WLC_IOCTL_SMLEN];
	dbus_config_t config;
	uint32 agglimit = 0;
	uint32 rpc_agg = BCM_RPC_TP_DNGL_AGG_DPC; /* host aggr not enabled yet */
#endif /* BCM_FD_AGGR */
	int ifidx;
	int32 ret = 0;

#ifdef CUSTOMER_HW4
	/* WAR : to prevent calling dhd_open abnormally in quick succession after hang event */
	if (dhd->pub.hang_was_sent == 1) {
		DHD_ERROR(("%s: HANG was sent up earlier\n", __FUNCTION__));
		/* Force to bring down WLAN interface in case dhd_stop() is not called
		 * from the upper layer when HANG event is triggered.
		 */
		if (!dhd_download_fw_on_driverload && dhd->pub.up == 1) {
			DHD_ERROR(("%s: WLAN interface is not brought down\n", __FUNCTION__));
			dhd_stop(net);
		} else {
			return -1;
		}
	}
#endif /* CUSTOMER_HW4 */

#if defined(MULTIPLE_SUPPLICANT)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 25)) && defined(OEM_ANDROID) && 0
	if (mutex_is_locked(&_dhd_sdio_mutex_lock_) != 0) {
		DHD_ERROR(("%s : dhd_open: call dev open before insmod complete!\n", __FUNCTION__));
	}
	mutex_lock(&_dhd_sdio_mutex_lock_);
#endif
#endif /* MULTIPLE_SUPPLICANT */

	DHD_OS_WAKE_LOCK(&dhd->pub);
	DHD_LOCK(&dhd->pub);
	dhd->pub.dongle_trap_occured = 0;
	dhd->pub.hang_was_sent = 0;

#if defined(OEM_ANDROID) && !defined(WL_CFG80211)
	/*
	 * Force start if ifconfig_up gets called before START command
	 *  We keep WEXT's wl_control_wl_start to provide backward compatibility
	 *  This should be removed in the future
	 */
	ret = wl_control_wl_start(net);
	if (ret != 0) {
		DHD_ERROR(("%s: failed with code %d\n", __FUNCTION__, ret));
		ret = -1;
		goto exit;
	}

#endif /* defined(OEM_ANDROID) && !defined(WL_CFG80211) */

	ifidx = dhd_net2idx(dhd, net);
	DHD_TRACE(("%s: ifidx %d\n", __FUNCTION__, ifidx));

	if (ifidx < 0) {
		DHD_ERROR(("%s: Error: called with invalid IF\n", __FUNCTION__));
		ret = -1;
		goto exit;
	}

	if (!dhd->iflist[ifidx]) {
		DHD_ERROR(("%s: Error: called when IF already deleted\n", __FUNCTION__));
		ret = -1;
		goto exit;
	}

	if (ifidx == 0) {
		atomic_set(&dhd->pend_8021x_cnt, 0);
#if defined(WL_CFG80211) && defined(OEM_ANDROID)
		if (!dhd_download_fw_on_driverload) {
			DHD_ERROR(("\n%s\n", dhd_version));
#if defined(USE_INITIAL_2G_SCAN) || defined(USE_INITIAL_SHORT_DWELL_TIME)
			g_first_broadcast_scan = TRUE;
#endif /* USE_INITIAL_2G_SCAN || USE_INITIAL_SHORT_DWELL_TIME */
			ret = wl_android_wifi_on(net);
			if (ret != 0) {
				DHD_ERROR(("%s : wl_android_wifi_on failed (%d)\n",
					__FUNCTION__, ret));
				ret = -1;
				goto exit;
			}
		}
#ifdef SUPPORT_DEEP_SLEEP
		else {
			/* Flags to indicate if we distingish
			 * power off policy when user set the memu
			 * "Keep Wi-Fi on during sleep" to "Never"
			 */
			if (trigger_deep_sleep) {
#if defined(USE_INITIAL_2G_SCAN) || defined(USE_INITIAL_SHORT_DWELL_TIME)
				g_first_broadcast_scan = TRUE;
#endif /* USE_INITIAL_2G_SCAN || USE_INITIAL_SHORT_DWELL_TIME */
				dhd_deepsleep(net, 0);
				trigger_deep_sleep = 0;
			}
		}
#endif /* SUPPORT_DEEP_SLEEP */
#if defined(CUSTOMER_HW4) && defined(FIX_CPU_MIN_CLOCK)
		if (dhd_get_fw_mode(dhd) == DHD_FLAG_HOSTAP_MODE) {
			dhd_init_cpufreq_fix(dhd);
			dhd_fix_cpu_freq(dhd);
		}
#endif /* defined(CUSTOMER_HW4) && defined(FIX_CPU_MIN_CLOCK) */
#endif /* defined(WL_CFG80211) && defined(OEM_ANDROID) */

		if (dhd->pub.busstate != DHD_BUS_DATA) {

#ifndef BCMDBUS
			/* try to bring up bus */
			DHD_UNLOCK(&dhd->pub);
			ret = dhd_bus_start(&dhd->pub);
			DHD_LOCK(&dhd->pub);
			if (ret) {
				DHD_ERROR(("%s: failed with code %d\n", __FUNCTION__, ret));
				ret = -1;
				goto exit;
			}
#else /* BCMDBUS */
			if ((ret = dbus_up(dhd->pub.dbus)) != 0) {
				goto exit;
			} else {
				dhd->pub.busstate = DHD_BUS_DATA;
			}

			/*
			 * Bus is ready, query any dongle information
			 *
			 * XXX: Since dhd_sync_with_dongle can sleep, should
			 * module count surround it?
			 */
			if ((ret = dhd_sync_with_dongle(&dhd->pub)) < 0) {
				DHD_ERROR(("%s: failed with code %d\n", __FUNCTION__, ret));
				goto exit;
			}
#endif /* BCMDBUS */

		}

#ifdef BCM_FD_AGGR
		config.config_id = DBUS_CONFIG_ID_AGGR_LIMIT;

		memset(iovbuf, 0, sizeof(iovbuf));
		bcm_mkiovar("rpc_dngl_agglimit", (char *)&agglimit, 4,
			iovbuf, sizeof(iovbuf));

		if (!dhd_wl_ioctl_cmd(&dhd->pub, WLC_GET_VAR, iovbuf, sizeof(iovbuf), FALSE, 0)) {
			agglimit = *(uint32 *)iovbuf;
			config.aggr_param.maxrxsf = agglimit >> BCM_RPC_TP_AGG_SF_SHIFT;
			config.aggr_param.maxrxsize = agglimit & BCM_RPC_TP_AGG_BYTES_MASK;
			DHD_ERROR(("rpc_dngl_agglimit %x : sf_limit %d bytes_limit %d\n",
				agglimit, config.aggr_param.maxrxsf, config.aggr_param.maxrxsize));
			if (bcm_rpc_tp_set_config(dhd->pub.info->rpc_th, &config)) {
				DHD_ERROR(("set tx/rx queue size and buffersize failed\n"));
			}
		} else {
			DHD_ERROR(("get rpc_dngl_agglimit failed\n"));
			rpc_agg &= ~BCM_RPC_TP_DNGL_AGG_DPC;
		}

		/* Set aggregation for TX */
		bcm_rpc_tp_agg_set(dhd->pub.info->rpc_th, BCM_RPC_TP_HOST_AGG_MASK,
			rpc_agg & BCM_RPC_TP_HOST_AGG_MASK);

		/* Set aggregation for RX */
		memset(iovbuf, 0, sizeof(iovbuf));
		bcm_mkiovar("rpc_agg", (char *)&rpc_agg, sizeof(rpc_agg), iovbuf, sizeof(iovbuf));
		if (!dhd_wl_ioctl_cmd(&dhd->pub, WLC_SET_VAR, iovbuf, sizeof(iovbuf), TRUE, 0)) {
			dhd->pub.info->fdaggr = 0;
			if (rpc_agg & BCM_RPC_TP_HOST_AGG_MASK)
				dhd->pub.info->fdaggr |= BCM_FDAGGR_H2D_ENABLED;
			if (rpc_agg & BCM_RPC_TP_DNGL_AGG_MASK)
				dhd->pub.info->fdaggr |= BCM_FDAGGR_D2H_ENABLED;
		} else {
			DHD_ERROR(("%s(): Setting RX aggregation failed %d\n", __FUNCTION__, ret));
		}
#endif /* BCM_FD_AGGR */

		/* dhd_sync_with_dongle has been called in dhd_bus_start or wl_android_wifi_on */
		memcpy(net->dev_addr, dhd->pub.mac.octet, ETHER_ADDR_LEN);

#ifdef TOE
		/* Get current TOE mode from dongle */
		if (dhd_toe_get(dhd, ifidx, &toe_ol) >= 0 && (toe_ol & TOE_TX_CSUM_OL) != 0) {
			dhd->iflist[ifidx]->net->features |= NETIF_F_IP_CSUM;
		} else {
			dhd->iflist[ifidx]->net->features &= ~NETIF_F_IP_CSUM;
		}
#endif /* TOE */

#if defined(WL_CFG80211)
		/* Unlock the locks as we take the lock again in subsequent function call. */
		DHD_UNLOCK(&dhd->pub);
		DHD_OS_WAKE_UNLOCK(&dhd->pub);
		if (unlikely(wl_cfg80211_up(net))) {
			DHD_ERROR(("%s: failed to bring up cfg80211\n", __FUNCTION__));
			ret = -1;
			DHD_OS_WAKE_LOCK(&dhd->pub);
			DHD_LOCK(&dhd->pub);
			goto exit;
		}
		DHD_OS_WAKE_LOCK(&dhd->pub);
		DHD_LOCK(&dhd->pub);
		if (!dhd_download_fw_on_driverload) {
#ifdef ARP_OFFLOAD_SUPPORT
			dhd->pend_ipaddr = 0;
			if (!dhd_inetaddr_notifier_registered) {
				dhd_inetaddr_notifier_registered = TRUE;
				register_inetaddr_notifier(&dhd_inetaddr_notifier);
			}
#endif /* ARP_OFFLOAD_SUPPORT */
#if defined(OEM_ANDROID) && defined(CONFIG_IPV6)
			if (!dhd_inet6addr_notifier_registered) {
				dhd_inet6addr_notifier_registered = TRUE;
				register_inet6addr_notifier(&dhd_inet6addr_notifier);
			}
#endif /* OEM_ANDROID && CONFIG_IPV6 */
#ifdef DHD_LB
			DHD_LB_STATS_INIT(&dhd->pub);
#ifdef DHD_LB_RXP
			__skb_queue_head_init(&dhd->rx_pend_queue);
#endif /* DHD_LB_RXP */
#endif /* DHD_LB */
		}
#if defined(DHD_LB) && defined(DHD_LB_RXP)
		if (dhd->rx_napi_netdev == NULL) {
			dhd->rx_napi_netdev = dhd->iflist[ifidx]->net;
			memset(&dhd->rx_napi_struct, 0, sizeof(struct napi_struct));
			netif_napi_add(dhd->rx_napi_netdev, &dhd->rx_napi_struct,
				dhd_napi_poll, dhd_napi_weight);
			DHD_INFO(("%s napi<%p> enabled ifp->net<%p, %s>\n",
				__FUNCTION__, &dhd->rx_napi_struct, net, net->name));
			napi_enable(&dhd->rx_napi_struct);
			DHD_INFO(("%s load balance init rx_napi_struct\n", __FUNCTION__));
			skb_queue_head_init(&dhd->rx_napi_queue);
		}
#endif /* DHD_LB && DHD_LB_RXP */
#if defined(NUM_SCB_MAX_PROBE)
		dhd_set_scb_probe(&dhd->pub);
#endif /* NUM_SCB_MAX_PROBE */
#endif /* WL_CFG80211 */
	}

	/* Allow transmit calls */
	netif_start_queue(net);
	dhd->pub.up = 1;

	OLD_MOD_INC_USE_COUNT;

#ifdef DHD_DEBUGFS
	(void) dhd_dbg_init(&dhd->pub);
#endif /* DHD_DEBUGFS */
exit:
	DHD_UNLOCK(&dhd->pub);
	DHD_OS_WAKE_UNLOCK(&dhd->pub);

#if defined(MULTIPLE_SUPPLICANT)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 25)) && defined(OEM_ANDROID) && 0
	mutex_unlock(&_dhd_sdio_mutex_lock_);
#endif
#endif /* MULTIPLE_SUPPLICANT */

	if (ret) {
		dhd_stop(net);
	}

	return ret;
}

int
dhd_do_driver_init(struct net_device *net)
{
	dhd_info_t *dhd = NULL;

	if (!net) {
		DHD_ERROR(("Primary Interface not initialized \n"));
		return -EINVAL;
	}

#ifdef MULTIPLE_SUPPLICANT
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 25)) && defined(OEM_ANDROID) && 0
	if (mutex_is_locked(&_dhd_sdio_mutex_lock_) != 0) {
		DHD_ERROR(("%s : dhdsdio_probe is already running!\n", __FUNCTION__));
		return 0;
	}
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 25)) */
#endif /* MULTIPLE_SUPPLICANT */

	/*  && defined(OEM_ANDROID) && defined(BCMSDIO) */
	dhd = DHD_DEV_INFO(net);

	/* If driver is already initialized, do nothing
	 */
	if (dhd->pub.busstate == DHD_BUS_DATA) {
		DHD_TRACE(("Driver already Inititalized. Nothing to do"));
		return 0;
	}

	if (dhd_open(net) < 0) {
		DHD_ERROR(("Driver Init Failed \n"));
		return -1;
	}

	return 0;
}

#ifdef WL_CFG80211
static struct bcm_cfg80211 *
dhd_get_cfg(dhd_info_t *dhd)
{
	dhd_if_t *ifp = dhd->iflist[0];
	return wl_get_cfg(ifp->net);
}
#endif /* WL_CFG80211 */

int
dhd_event_ifadd(dhd_info_t *dhdinfo, wl_event_data_if_t *ifevent, char *name, uint8 *mac)
{

#ifdef WL_CFG80211
	if ((dhdinfo->dhd_state & DHD_ATTACH_STATE_CFG80211) &&
#if defined(WL_CFG80211) && defined(WL_HAPD_WDS)
		(strstr(name, "wds") == NULL) &&
#endif
	(TRUE)) {
		if (wl_cfg80211_notify_ifadd(dhd_linux_get_primary_netdev(&dhdinfo->pub),
			ifevent->ifidx, name, mac, ifevent->bssidx, ifevent->role) == BCME_OK)
		{
			return BCME_OK;
		}
	}
#endif /* WL_CFG80211 */

	/* handle IF event caused by wl commands, SoftAP, WEXT and
	 * anything else. This has to be done asynchronously otherwise
	 * DPC will be blocked (and iovars will timeout as DPC has no chance
	 * to read the response back)
	 */
	if (ifevent->ifidx > 0) {
		dhd_if_event_t *if_event = MALLOC(dhdinfo->pub.osh, sizeof(dhd_if_event_t));
		if (if_event == NULL) {
			DHD_ERROR(("dhd_event_ifadd: Failed MALLOC, malloced %d bytes",
				MALLOCED(dhdinfo->pub.osh)));
			return BCME_NOMEM;
		}

		memcpy(&if_event->event, ifevent, sizeof(if_event->event));
		memcpy(if_event->mac, mac, ETHER_ADDR_LEN);
		strncpy(if_event->name, name, IFNAMSIZ);
		if_event->name[IFNAMSIZ - 1] = '\0';
		if (dhd_deferred_schedule_work(dhdinfo->dhd_deferred_wq,
			if_event, DHD_WQ_WORK_IF_ADD,
			dhd_ifadd_event_handler, DHD_WORK_PRIORITY_LOW) != DHD_WQ_STS_OK) {
			MFREE(dhdinfo->pub.osh, if_event, sizeof(dhd_if_event_t));
			return BCME_ERROR;
		}
	}

	return BCME_OK;
}

int
dhd_event_ifdel(dhd_info_t *dhdinfo, wl_event_data_if_t *ifevent, char *name, uint8 *mac)
{
	dhd_if_event_t *if_event;

#ifdef WL_CFG80211
	if ((dhdinfo->dhd_state & DHD_ATTACH_STATE_CFG80211) &&
		(strstr(name, "wds") == NULL)) {
		if (wl_cfg80211_notify_ifdel(dhd_get_cfg(dhdinfo),
			ifevent->ifidx, ifevent->bssidx) == BCME_OK)
		{
			return BCME_OK;
		}
	}
#endif /* WL_CFG80211 */

	/* handle IF event caused by wl commands, SoftAP, WEXT and
	 * anything else
	 */
	if_event = MALLOC(dhdinfo->pub.osh, sizeof(dhd_if_event_t));
	if (if_event == NULL) {
		DHD_ERROR(("dhd_event_ifdel: malloc failed for if_event, malloced %d bytes",
			MALLOCED(dhdinfo->pub.osh)));
		return BCME_NOMEM;
	}
	memcpy(&if_event->event, ifevent, sizeof(if_event->event));
	memcpy(if_event->mac, mac, ETHER_ADDR_LEN);
	strncpy(if_event->name, name, IFNAMSIZ);
	if_event->name[IFNAMSIZ - 1] = '\0';

	if (dhd_deferred_schedule_work(dhdinfo->dhd_deferred_wq,
		if_event, DHD_WQ_WORK_IF_DEL,
		dhd_ifdel_event_handler, DHD_WORK_PRIORITY_LOW) != DHD_WQ_STS_OK) {
		MFREE(dhdinfo->pub.osh, if_event, sizeof(dhd_if_event_t));
		return BCME_ERROR;
	}

	return BCME_OK;
}

int
dhd_event_ifchange(dhd_info_t *dhdinfo, wl_event_data_if_t *ifevent, char *name, uint8 *mac)
{
#ifdef WL_CFG80211
	wl_cfg80211_notify_ifchange(dhd_get_cfg(dhdinfo));
#endif /* WL_CFG80211 */
	return BCME_OK;
}

/* unregister and free the existing net_device interface (if any) in iflist and
 * allocate a new one. the slot is reused. this function does NOT register the
 * new interface to linux kernel. dhd_register_if does the job
 */
struct net_device*
dhd_allocate_if(dhd_pub_t *dhdpub, int ifidx, const char *name,
	uint8 *mac, uint8 bssidx, bool need_rtnl_lock, const char *dngl_name)
{
	dhd_info_t *dhdinfo = (dhd_info_t *)dhdpub->info;
	dhd_if_t *ifp;
	struct net_device *net = NULL;

	ASSERT(dhdinfo && (ifidx < DHD_MAX_IFS));
	ifp = dhdinfo->iflist[ifidx];

	if (ifp != NULL) {
		if (ifp->net != NULL) {
			DHD_ERROR(("%s: free existing IF %s\n", __FUNCTION__, ifp->net->name));
			net = ifp->net;
			ifp->net = NULL;

			dhd_dev_priv_clear(net); /* clear net_device private */
			DHD_UNLOCK(dhdpub);
			/* in unregister_netdev case, the interface gets freed by net->destructor
			 * (which is set to free_netdev)
			 */
			if (net->reg_state == NETREG_UNINITIALIZED) {
				free_netdev(net);
			} else {
#ifdef BCA_HNDROUTER
				if (is_reboot != SYS_RESTART) {
#endif

				netif_stop_queue(net);
				if (need_rtnl_lock)
					unregister_netdev(net);
				else
					unregister_netdevice(net);
#ifdef BCA_HNDROUTER
				}
#endif
			}
			DHD_LOCK(dhdpub);
		}
	} else {
		ifp = MALLOC(dhdinfo->pub.osh, sizeof(dhd_if_t));
		if (ifp == NULL) {
			DHD_ERROR(("%s: OOM - dhd_if_t(%zu)\n", __FUNCTION__, sizeof(dhd_if_t)));
			return NULL;
		}
	}

	memset(ifp, 0, sizeof(dhd_if_t));
	ifp->info = dhdinfo;
	ifp->idx = ifidx;
	ifp->bssidx = bssidx;
#ifdef DHD_MCAST_REGEN
	ifp->mcast_regen_bss_enable = FALSE;
#endif
#ifdef BCA_HNDROUTER
	ifp->subunit = ifidx;
#endif
	/* set to TRUE rx_pkt_chainable at alloc time */
	ifp->rx_pkt_chainable = TRUE;
#ifdef BCM_ROUTER_DHD
	ifp->bss_up = FALSE;
#endif
	ifp->wdsidx = ID8_INVALID;

	if (mac != NULL)
		memcpy(&ifp->mac_addr, mac, ETHER_ADDR_LEN);

	DHD_UNLOCK(dhdpub);
	/* Allocate etherdev, including space for private structure */
	net = alloc_etherdev(DHD_DEV_PRIV_SIZE);
	DHD_LOCK(dhdpub);
	ifp->net = net;
	if (ifp->net == NULL) {
		DHD_ERROR(("%s: OOM - alloc_etherdev(%zu)\n", __FUNCTION__, sizeof(dhdinfo)));
		goto fail;
	}

#ifdef BCA_HNDROUTER
	/* tag netdev as wlan device */
	netdev_wlan_set(ifp->net);
#endif /* BCA_HNDROUTER */
#if defined(BCM_PKTFWD)
	/* BCM_PKTFWD: insert ifp into pktfwd */
	ifp->d3fwd_wlif = dhd_pktfwd_wlif_ins(ifp, dhdpub->osh, ifp->net, dhdpub->unit);

	if (!ifp->d3fwd_wlif)
		DHD_ERROR(("%s: PKTFWD disabled for wl%d interface\n", __FUNCTION__, dhdpub->unit));
#endif /* BCM_PKTFWD */

	/* Setup the dhd interface's netdevice private structure. */
	dhd_dev_priv_save(ifp->net, dhdinfo, ifp, ifidx);

	if (name && name[0]) {
		strncpy(ifp->net->name, name, IFNAMSIZ);
		ifp->net->name[IFNAMSIZ - 1] = '\0';
	}

#ifdef WL_CFG80211
	if (ifidx == 0)
		wl_netdev_set_free_netdev(ifp->net);
	else
		wl_netdev_set_destructor(ifp->net, wl_netdev_free);
#else
	wl_netdev_set_free_netdev(ifp->net);
#endif /* WL_CFG80211 */
	strncpy(ifp->name, ifp->net->name, IFNAMSIZ);
	ifp->name[IFNAMSIZ - 1] = '\0';
	dhdinfo->iflist[ifidx] = ifp;

	/* initialize the dongle provided if name */
	if (dngl_name)
		strncpy(ifp->dngl_name, dngl_name, IFNAMSIZ);
	else if (name)
		strncpy(ifp->dngl_name, name, IFNAMSIZ);
	else
		ifp->dngl_name[0] = '\0';

#ifdef PCIE_FULL_DONGLE
	/* Initialize STA info list */
	INIT_LIST_HEAD(&ifp->sta_list);
	DHD_IF_STA_LIST_LOCK_INIT(ifp);
#endif /* PCIE_FULL_DONGLE */

#ifdef DHD_L2_FILTER
	ifp->phnd_arp_table = init_l2_filter_arp_table(dhdpub->osh);
	ifp->parp_allnode = TRUE;
#endif /* DHD_L2_FILTER */

#if (defined(BCM_ROUTER_DHD) && defined(QOS_MAP_SET))
	ifp->qosmap_up_table = ((uint8*)MALLOCZ(dhdpub->osh, UP_TABLE_MAX));
	ifp->qosmap_up_table_enable = FALSE;
#endif /* BCM_ROUTER_DHD && QOS_MAP_SET */

	DHD_CUMM_CTR_INIT(&ifp->cumm_ctr);

#ifdef DHD_WMF
	dhd_wmf_bss_enable(dhdpub, ifidx);
#endif /* DHD_EMF */

#if defined(BCM_BLOG)
	{
	    uint hwport = 0;
#if defined(BCM_WFD)
	    hwport = WLAN_NETDEVPATH_HWPORT(dhdpub->unit, ifp->subunit);
#endif /* BCM_WFD */
	    DHD_INFO(("%s: ifidx=%d, bssidx=%d, unit=%d, hwport=%d\n", __FUNCTION__,
	        ifidx, bssidx, dhdpub->unit, hwport));
	    netdev_path_set_hw_port(ifp->net, hwport, BLOG_WLANPHY);
	}
#endif /* BCM_BLOG */

#ifdef BCM_WLAN_PER_CLIENT_FLOW_LEARNING
#ifdef BCM_CPEROUTER_EXTSTATS
	/* Indicate we're supporting extended statistics */
	net->features |= NETIF_F_EXTSTATS;
#endif /* BCM_CPEROUTER_EXTSTATS */

#if defined(BCM_COUNTER_EXTSTATS) && defined(CONFIG_BLOG)
	/* new stats implementation */
	bcm_netdev_blog_stats_flags(net) |= BLOG_DEV_STAT_FLAG_INCLUDE_HW_UC;
	bcm_netdev_blog_stats_flags(net) |= BLOG_DEV_STAT_FLAG_INCLUDE_HW_MC;
#endif /* defined(BCM_COUNTER_EXTSTATS) && defined(CONFIG_BLOG) */
#endif /* defined(BCM_WLAN_PER_CLIENT_FLOW_LEARNING) */

	return ifp->net;

fail:

	if (ifp != NULL) {
		if (ifp->net != NULL) {
			dhd_dev_priv_clear(ifp->net);
			DHD_UNLOCK(dhdpub);
			free_netdev(ifp->net);
			DHD_LOCK(dhdpub);
			ifp->net = NULL;
		}
		MFREE(dhdinfo->pub.osh, ifp, sizeof(*ifp));
		ifp = NULL;
	}

	dhdinfo->iflist[ifidx] = NULL;
	return NULL;
}

/* unregister and free the the net_device interface associated with the indexed
 * slot, also free the slot memory and set the slot pointer to NULL
 */
int
dhd_remove_if(dhd_pub_t *dhdpub, int ifidx, bool need_rtnl_lock)
{
	dhd_info_t *dhdinfo = (dhd_info_t *)dhdpub->info;
	dhd_if_t *ifp;
#ifdef PCIE_FULL_DONGLE
	if_flow_lkup_t *if_flow_lkup = (if_flow_lkup_t *)dhdpub->if_flow_lkup;
#endif /* PCIE_FULL_DONGLE */
	struct net_device *net = NULL;

	ifp = dhdinfo->iflist[ifidx];
	if (ifp == NULL)
		return BCME_OK;

	if (ifp->net != NULL) {
		DHD_ERROR(("deleting interface '%s' idx %d\n", ifp->net->name, ifp->idx));
		net = ifp->net;
		ifp->net = NULL;
		DHD_DEV_IFP(net) = NULL;
		DHD_DEV_IFIDX(net) = DHD_BAD_IF;

		/* in unregister_netdev case, the interface gets freed by net->destructor
		 * (which is set to free_netdev)
		 */
		if (net->reg_state != NETREG_UNINITIALIZED) {
#if defined(ARGOS_RPS_CPU_CTL) && defined(ARGOS_CPU_SCHEDULER)
			if (ifidx == 0) {
				argos_register_notifier_deinit();
			}
#endif /* ARGOS_RPS_CPU_CTL && ARGOS_CPU_SCHEDULER  */

#if defined(SET_RPS_CPUS)
			custom_rps_map_clear(net->_rx);
#endif /* SET_RPS_CPUS */
#if (defined(SET_RPS_CPUS) || defined(ARGOS_RPS_CPU_CTL))
#if (defined(DHDTCPACK_SUPPRESS) && defined(BCMPCIE))
			dhd_tcpack_suppress_set(dhdpub, TCPACK_SUP_OFF);
#endif /* DHDTCPACK_SUPPRESS && BCMPCIE */
#endif /* SET_RPS_CPUS || ARGOS_RPS_CPU_CTL */

#if defined(BCM_WFD)
			if (dhd_wfd_unregisterdevice(dhdinfo->pub.wfd_idx, net) != 0)
				DHD_ERROR(("%s: %s: dhd_wfd_unregisterdevice failed "
					"wfd_idx %d ifidx %d\n", ifp->net->name,
					__FUNCTION__, dhdpub->wfd_idx, ifidx));
			else
				DHD_INFO(("%s: %s: dhd_wfd_unregisterdevice successful "
					"wfd_idx %d ifidx %d\n", ifp->net->name,
					__FUNCTION__, dhdpub->wfd_idx, ifidx));
#endif /* BCM_WFD */
		}
	}
#ifdef DHD_WMF
	dhd_wmf_cleanup(dhdpub, ifidx);
#endif /* DHD_WMF */
#ifdef DHD_L2_FILTER
	bcm_l2_filter_arp_table_update(dhdpub->osh, ifp->phnd_arp_table, TRUE,
		NULL, FALSE, dhdpub->tickcnt);
	deinit_l2_filter_arp_table(dhdpub->osh, ifp->phnd_arp_table);
	ifp->phnd_arp_table = NULL;
	ifp->parp_enable = FALSE;
#endif /* DHD_L2_FILTER */

#if (defined(BCM_ROUTER_DHD) && defined(QOS_MAP_SET))
	MFREE(dhdpub->osh, ifp->qosmap_up_table, UP_TABLE_MAX);
	ifp->qosmap_up_table = NULL;
	ifp->qosmap_up_table_enable = FALSE;
#endif /* BCM_ROUTER_DHD && QOS_MAP_SET */

	dhd_if_del_sta_list(ifp);
#ifdef PCIE_FULL_DONGLE
	/* Delete flowrings of WDS interface */
	if ((ifidx != 0) && (if_flow_lkup[ifidx].role != WLC_E_IF_ROLE_AP)) {
		dhd_flow_rings_delete(dhdpub, ifidx);
	}
#endif /* PCIE_FULL_DONGLE */
	DHD_CUMM_CTR_INIT(&ifp->cumm_ctr);
#if defined(BCM_PKTFWD)
	/* BCM_PKTFWD: delete ifp from pktfwd */
	if (PKTFWD_ENABLED(dhdpub) && ifp->d3fwd_wlif) {
		dhd_pktfwd_wlif_del(ifp->d3fwd_wlif);
		ifp->d3fwd_wlif = NULL;
	}
#endif /* BCM_PKTFWD */

	dhdinfo->iflist[ifidx] = NULL;
	MFREE(dhdinfo->pub.osh, ifp, sizeof(*ifp));

	if (net) {
		DHD_UNLOCK(dhdpub);
		if (net->reg_state == NETREG_UNINITIALIZED)
			free_netdev(net);
		else {
			if (need_rtnl_lock)
				unregister_netdev(net);
			else
				unregister_netdevice(net);
		}
		DHD_LOCK(dhdpub);
	}

	return BCME_OK;
}

#if (defined(BCM_ROUTER_DHD) && defined(QOS_MAP_SET))
int
dhd_set_qosmap_up_table(dhd_pub_t *dhdp, uint32 idx, bcm_tlv_t *qos_map_ie)
{
	dhd_info_t *dhd = dhdp->info;
	dhd_if_t *ifp;

	ASSERT(idx < DHD_MAX_IFS);
	ifp = dhd->iflist[idx];

	if (!ifp)
	    return BCME_ERROR;

	wl_set_up_table(ifp->qosmap_up_table, qos_map_ie);
	ifp->qosmap_up_table_enable = TRUE;

	return BCME_OK;
}
#endif /* BCM_ROUTER_DHD && QOS_MAP_SET */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 31))
static struct net_device_ops dhd_ops_pri = {
	.ndo_open = dhd_open,
	.ndo_stop = dhd_stop,
	.ndo_uninit = dhd_uninit,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36))
	.ndo_get_stats64 = dhd_get_stats,
#else
	.ndo_get_stats = dhd_get_stats,
#endif /* KERNEL_VERSION  >= 2.6.36 */
	.ndo_do_ioctl = dhd_ioctl_entry,
	.ndo_start_xmit = dhd_start_xmit,
	.ndo_set_mac_address = dhd_set_mac_address,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 2, 0))
	.ndo_set_rx_mode = dhd_set_multicast_list,
#else
	.ndo_set_multicast_list = dhd_set_multicast_list,
#endif
};

static struct net_device_ops dhd_ops_virt = {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36))
	.ndo_get_stats64 = dhd_get_stats,
#else
	.ndo_get_stats = dhd_get_stats,
#endif /* KERNEL_VERSION  >= 2.6.36 */
	.ndo_do_ioctl = dhd_ioctl_entry,
	.ndo_start_xmit = dhd_start_xmit,
	.ndo_set_mac_address = dhd_set_mac_address,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 2, 0))
	.ndo_set_rx_mode = dhd_set_multicast_list,
#else
	.ndo_set_multicast_list = dhd_set_multicast_list,
#endif
};
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 31)) */

/** sizeof(BEA file header) excluding per-segment information */
#define SIZEOF_BEA_HDR (sizeof(wl_segment_info_t) - sizeof(wl_segment_t))

/**
 * reads the .bea file header, including information on number of segments, and including per
 * segment information.
 *
 * @param filep  pointer to binary file opened for reading
 * @param bea_h  caller allocated memory containing header at function exit
 * @param bea_file_path
 *
 * @return pointer to newly allocated memory on success. Callers responsibility to free it.
 */
static wl_segment_info_t *
dhd_bea_read_header(struct file *filep)
{
	wl_segment_info_t *ret = NULL;
	int error;
	wl_segment_info_t *msf_hdr;
	int n_header_bytes = SIZEOF_BEA_HDR;
	int i;
	int n_bytes;

	for (i = 0; i < 2; i++) {
		/* first pass reads part of header, second pass reads whole header */
		error = generic_file_llseek(filep, 0, SEEK_SET);
		msf_hdr = (wl_segment_info_t*) kmalloc(n_header_bytes, KMALLOC_FLAG);

		if (error < 0 || msf_hdr == NULL) {
			DHD_ERROR(("%s: .bea llseek failed %d or malloc err\n", __FUNCTION__,
				error));
			goto exit;
		}

		n_bytes = kernel_read(filep, msf_hdr, n_header_bytes, &filep->f_pos);
		if (n_bytes != n_header_bytes) {
			DHD_INFO(("%s: Failed to read .bea file (%d)\n", __FUNCTION__, n_bytes));
			goto exit;
		}

		if (msf_hdr->magic[0] != 'B' || msf_hdr->magic[1] != 'L' ||
			msf_hdr->magic[2] != 'O' || msf_hdr->magic[3] != 'B') {
			DHD_INFO(("%s: Not a .bea header\n", __FUNCTION__));
			goto exit;
		}

		if (i == 0) {
			n_header_bytes = msf_hdr->hdr_len; /* now we know the whole header size */
			kfree(msf_hdr);
		}
	}

	/* Header crc32 check.  It starts with the field after the crc32, i.e. file type */
	{
		uint32 hdr_crc32;
		uint32 crc32_start_offset =
			OFFSETOF(wl_segment_info_t, crc32) + sizeof(hdr_crc32);
		char *data = (char*) msf_hdr;

		hdr_crc32 = hndcrc32(data + crc32_start_offset,
			msf_hdr->hdr_len - crc32_start_offset,
			0xffffffff);
		hdr_crc32 = hdr_crc32 ^ 0xffffffff;

		if (msf_hdr->crc32 != hdr_crc32) {
			DHD_ERROR(("Invalid header crc - expected %08x computed %08x\n",
				msf_hdr->crc32,
				hdr_crc32));
			goto exit;
		}
	}

	ret = msf_hdr;

exit:
	if (ret == NULL && msf_hdr != NULL) {
		kfree(msf_hdr);
	}

	return ret;
} /* dhd_bea_read_header */

/**
 * If a .bea file is to be loaded, the .bea file is first checked for validity. If that check
 * passes, then the rest of the code doesn't have to check for parse errors.
 */
static bool
dhd_bea_is_valid_file(char *_bea_file_path)
{
	struct file *filep;
	bool ret = FALSE;
	wl_segment_info_t *msf_hdr = NULL;
	wl_segment_t *msf_seg;
	int segment_present_bitmap = 0;
	int i_segment;

	if (_bea_file_path == NULL || strlen(_bea_file_path) == 0) {
		return FALSE;
	}

	filep = filp_open(_bea_file_path, O_RDONLY, 0);
	if (IS_ERR(filep)) {
		goto exit;
	}

	msf_hdr = dhd_bea_read_header(filep);
	if (msf_hdr == NULL) {
		goto exit;
	}

	for (i_segment = 0; i_segment < msf_hdr->num_segments; i_segment++) {
		msf_seg = &msf_hdr->segments[i_segment];
		segment_present_bitmap |= (1 << msf_seg->type);
	}

	if (segment_present_bitmap != (
		(1 << MSF_SEG_TYP_RTECDC_BIN) | (1 << MSF_SEG_TYP_LOGSTRS_BIN) |
		(1 << MSF_SEG_TYP_FW_SYMBOLS) | (1 << MSF_SEG_TYP_ROML_BIN))) {
		DHD_ERROR(("%s: .bea not all required partitions present\n", __FUNCTION__));
		goto exit;
	}

	ret = TRUE;

exit:
	if (msf_hdr != NULL) {
		kfree(msf_hdr);
	}

	if (!IS_ERR(filep)) {
		filp_close(filep, NULL);
	}

	return ret;
}

/**
 * One BEA file can contain multiple segments, like one for firmware, one for event logging, etc.
 * @return: length of segment (excluding header), or -1 on failure.
 */
static int
dhd_bea_seek_segment(struct file *filep, enum bea_seg_type_e bea_typ)
{
	int error;
	int segment_len = -1;
	wl_segment_info_t *msf_hdr;
	wl_segment_t *msf_seg;
	int i_segment;

	msf_hdr = dhd_bea_read_header(filep);
	if (msf_hdr == NULL) {
		goto exit;
	}

	for (i_segment = 0; i_segment < msf_hdr->num_segments; i_segment++) {
		msf_seg = &msf_hdr->segments[i_segment];
		if (msf_seg->type == bea_typ) {
			error = generic_file_llseek(filep, msf_seg->offset, SEEK_SET);
			if (error < 0) {
				DHD_ERROR(("%s: .bea llseek failed %d\n", __FUNCTION__, error));
				goto exit;
			}
			segment_len = msf_seg->length;
			break;
		}
	}

exit:
	if (msf_hdr != NULL) {
		kfree(msf_hdr);
	}

	return segment_len;
} /* dhd_bea_seek_segment */

/**
 * Get the interface id based on nvram define if configured.
 *
 *
 * @params none.
 *
 * return next ifid# to be used
 */

static int
dhd_get_ifid_override(struct dhd_bus *bus, int inst)
{
#ifdef BCM_ROUTER_DHD
	char *var;
	char varstr[32];
	uint16 devid;
	uint16 nvdevid;
	int ifid;
	int i;
	int j;

	int radio_idx = inst - instance_base;

	if (!nvram_ifid_override) {
		return inst;
	}

	/* use user defined radio/wlx map based on pcie enumeration order */
	if ((var = getvar(NULL, "wl_radio_to_ifid_map")) != NULL) {
		nvram_ifid_map = (uint32)bcm_strtoul(var, NULL, 0);
		/* use inst as idx for pcie enumeration order */
		ifid = DHD_GET_NVRAM_IFID(inst);
		goto done;
	}

	/* override instance id with nvram assigned ifid for radio */
	ifid = DHD_GET_NVRAM_IFID(radio_idx);
	devid = dhdpcie_get_deviceid(bus);

	for (i = 0; i < DHD_NVRAM_MAX_IFID && ifid == DHD_NVRAM_INVALID_IFID; i++) {
		snprintf(varstr, sizeof(varstr), "wl%d_deviceid", i);
		var = getvar(NULL, varstr);

		if (!var)
			continue;

		nvdevid = (uint16)bcm_strtoul(var, NULL, 0);

		if (nvdevid && (nvdevid == devid)) {
			bool ifid_used = FALSE;
			for (j = 0; j <= i; j++) {
				/* handle multiple radio's with same devid */
				if (DHD_GET_NVRAM_IFID(j) == i) {
					ifid_used = TRUE;
					break;
				}
			}

			if (!ifid_used) {
				ifid = i;
				DHD_SET_NVRAM_IFID(radio_idx, ifid);
				break;
			}
		}
	}
done:
	if (ifid != DHD_NVRAM_INVALID_IFID) {
		DHD_ERROR(("Using nvram assigned ifid (%s%d) for device(0x%x)\n",
			iface_name, ifid, dhdpcie_get_deviceid(bus)));
		return ifid;
	}
#endif /* BCM_ROUTER_HND */

	return inst;
}

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
int
dhd_get_next_instance(void)
{
	int inst = dhd_found + instance_base;

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

/** Called once for each hardware (dongle) instance that this DHD manages */
dhd_pub_t *
dhd_attach(osl_t *osh, struct dhd_bus *bus, uint bus_hdrlen)
{
	dhd_info_t *dhd = NULL;
	struct net_device *net = NULL;
	char if_name[IFNAMSIZ] = {'\0'};
	uint32 bus_type = -1;
	uint32 bus_num = -1;
	uint32 slot_num = -1;
	wifi_adapter_info_t *adapter = NULL;
#if defined(BCM_ROUTER_DHD)
	char *var;
	char varstr[32];
#endif /* BCM_ROUTER_DHD */
	dhd_attach_states_t dhd_state = DHD_ATTACH_STATE_INIT;
	DHD_TRACE(("%s: Enter\n", __FUNCTION__));

	/* will implement get_ids for DBUS later */
	adapter = dhd_wifi_platform_get_adapter(bus_type, bus_num, slot_num);

	/* Allocate primary dhd_info */
	dhd = wifi_platform_prealloc(adapter, DHD_PREALLOC_DHD_INFO, sizeof(dhd_info_t));
	if (dhd == NULL) {
		dhd = MALLOC(osh, sizeof(dhd_info_t));
		if (dhd == NULL) {
			DHD_ERROR(("%s: OOM - alloc dhd_info\n", __FUNCTION__));
			goto fail;
		}
	}
	memset(dhd, 0, sizeof(dhd_info_t));
	dhd_state |= DHD_ATTACH_STATE_DHD_ALLOC;

	dhd->pub.unit = dhd_get_next_instance();

	/* over ride unit assigned if enabled */
	if (nvram_ifid_override) {
		dhd->pub.unit = dhd_get_ifid_override(bus, dhd->pub.unit);
	}

	dhd->pub.osh = osh;
#if defined(BCM_WFD)
	dhd->pub.wfd_idx = -1;
#endif /* BCM_WFD */

#if !defined(BCMDBUS) /* console not supported for USB (uses DBUS) */
	dhd->pub.dhd_console_ms = dhd_console_ms; /* assigns default value */
#endif /* BCMDBUS */
	dhd->adapter = adapter;

#ifdef GET_CUSTOM_MAC_ENABLE
	wifi_platform_get_mac_addr(dhd->adapter, dhd->pub.mac.octet);
#endif /* GET_CUSTOM_MAC_ENABLE */
#ifndef BCMDBUS
	dhd->thr_dpc_ctl.thr_pid = DHD_PID_KT_TL_INVALID;
	dhd->thr_wdt_ctl.thr_pid = DHD_PID_KT_INVALID;

#ifdef DHD_WET
	dhd->pub.wet_info = dhd_get_wet_info(&dhd->pub);
#endif

	/* Initialize thread based operation and lock */
	sema_init(&dhd->sdsem, 1);

	/* Some DHD modules (e.g. cfg80211) configures operation mode based on firmware name.
	 * This is indeed a hack but we have to make it work properly before we have a better
	 * solution
	 */
	dhd_update_fw_nv_path(dhd);
#endif /* BCMDBUS */

	/* Link to info module */
	dhd->pub.info = dhd;

#if (defined(BCM_ROUTER_DHD) && defined(BCM_GMAC3))
	/* DHD_FWDER_UNIT(dhd) is not initialized yet */
	dhd_perim_radio_reg(dhd->pub.unit, &dhd->pub);
#endif /* BCM_ROUTER_DHD && BCM_GMAC3 */

	/* Link to bus module */
	dhd->pub.bus = bus;
	dhd->pub.hdrlen = bus_hdrlen;

	/* Set network interface name if it was provided as module parameter */
	if (iface_name[0]) {
		int len;
		char ch;
		strncpy(if_name, iface_name, IFNAMSIZ);
		if_name[IFNAMSIZ - 1] = 0;
		len = strlen(if_name);
		ch = if_name[len - 1];
		if ((ch > '9' || ch < '0') && (len < IFNAMSIZ - 2)) {
#ifdef BCM_NBUFF_WLMCAST_IPV6
			/* dhd->pub.unit has been adjust to clearly indciate
			 * how many wl interfaces are there, no need to use %d
			 */
			snprintf(if_name+len, 2, "%d", dhd->pub.unit);
#else
			if (nvram_ifid_override) {
				snprintf(if_name+len, 2, "%d", dhd->pub.unit);
			}
			else
				strcat(if_name, "%d");
#endif
		}
#ifdef BCM_NBUFF_WLMCAST_IPV6
		else
			snprintf(if_name, IFNAMSIZ, "wl%d", dhd->pub.unit);
#endif
	}
	DHD_LOCK(&dhd->pub);
	/* Passing NULL to dngl_name to ensure host gets if_name in dngl_name member */
	net = dhd_allocate_if(&dhd->pub, 0, if_name, NULL, 0, TRUE, NULL);
	DHD_UNLOCK(&dhd->pub);
	if (net == NULL)
		goto fail;
	dhd_state |= DHD_ATTACH_STATE_ADD_IF;
#ifdef DHD_L2_FILTER
	/* initialize the l2_filter_cnt */
	dhd->pub.l2_filter_cnt = 0;
#endif
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 31))
	net->open = NULL;
#else
	net->netdev_ops = NULL;
#endif
#ifndef BCM_WLAN_PER_CLIENT_FLOW_LEARNING
#ifdef BCM_CPEROUTER_EXTSTATS
	/* Indicate we're supporting extended statistics */
	net->features |= NETIF_F_EXTSTATS;
#endif /* BCM_CPEROUTER_EXTSTATS */

#if defined(BCM_COUNTER_EXTSTATS) && defined(CONFIG_BLOG)
	/* new stats implementation */
	bcm_netdev_blog_stats_flags(net) |= BLOG_DEV_STAT_FLAG_INCLUDE_HW_UC;
#endif /* defined(BCM_COUNTER_EXTSTATS) && defined(CONFIG_BLOG) */
#endif /* !defined(BCM_WLAN_PER_CLIENT_FLOW_LEARNING) */
	sema_init(&dhd->proto_sem, 1);

#ifdef PROP_TXSTATUS
	spin_lock_init(&dhd->wlfc_spinlock);

	dhd->pub.skip_fc = dhd_wlfc_skip_fc;
	dhd->pub.plat_init = dhd_wlfc_plat_init;
	dhd->pub.plat_deinit = dhd_wlfc_plat_deinit;

#ifdef DHD_WLFC_THREAD
	init_waitqueue_head(&dhd->pub.wlfc_wqhead);
	dhd->pub.wlfc_thread = kthread_create(dhd_wlfc_transfer_packets, &dhd->pub, "wlfc-thread");
	if (IS_ERR(dhd->pub.wlfc_thread)) {
		DHD_ERROR(("create wlfc thread failed\n"));
		goto fail;
	} else {
		wake_up_process(dhd->pub.wlfc_thread);
	}
#endif /* DHD_WLFC_THREAD */
#endif /* PROP_TXSTATUS */

	/* Initialize other structure content */
	init_waitqueue_head(&dhd->ioctl_resp_wait);
	init_waitqueue_head(&dhd->d3ack_wait);
	init_waitqueue_head(&dhd->ctrl_wait);

	/* Initialize the spinlocks */
	spin_lock_init(&dhd->sdlock);
	spin_lock_init(&dhd->txqlock);
	spin_lock_init(&dhd->dhd_lock);
	spin_lock_init(&dhd->rxf_lock);
#if defined(RXFRAME_THREAD)
	dhd->rxthread_enabled = TRUE;
#endif /* defined(RXFRAME_THREAD) */

#ifdef DHDTCPACK_SUPPRESS
	spin_lock_init(&dhd->tcpack_lock);
#endif /* DHDTCPACK_SUPPRESS */

	/* Initialize Wakelock stuff */
	spin_lock_init(&dhd->wakelock_spinlock);
	dhd->wakelock_counter = 0;
	dhd->wakelock_wd_counter = 0;
	dhd->wakelock_rx_timeout_enable = 0;
	dhd->wakelock_ctrl_timeout_enable = 0;
#ifdef CONFIG_HAS_WAKELOCK /* wakelocks prevent a system from going into a low power \
	state */
	wake_lock_init(&dhd->wl_wifi, WAKE_LOCK_SUSPEND, "wlan_wake");
	wake_lock_init(&dhd->wl_rxwake, WAKE_LOCK_SUSPEND, "wlan_rx_wake");
	wake_lock_init(&dhd->wl_ctrlwake, WAKE_LOCK_SUSPEND, "wlan_ctrl_wake");
	wake_lock_init(&dhd->wl_wdwake, WAKE_LOCK_SUSPEND, "wlan_wd_wake");
#ifdef BCMPCIE_OOB_HOST_WAKE
	wake_lock_init(&dhd->wl_intrwake, WAKE_LOCK_SUSPEND, "wlan_oob_irq_wake");
#endif /* BCMPCIE_OOB_HOST_WAKE */
#endif /* CONFIG_HAS_WAKELOCK */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 25)) && defined(OEM_ANDROID)
	mutex_init(&dhd->dhd_net_if_mutex);
	mutex_init(&dhd->dhd_suspend_mutex);
#endif
	dhd_state |= DHD_ATTACH_STATE_WAKELOCKS_INIT;

	/* Attach and link in the protocol */
	if (dhd_prot_attach(&dhd->pub) != 0) {
		DHD_ERROR(("dhd_prot_attach failed\n"));
		goto fail;
	}
	dhd_state |= DHD_ATTACH_STATE_PROT_ATTACH;

#ifdef WL_CFG80211
	/* Attach and link in the cfg80211 */
	if (dhd_cfg80211_enabled() == BCME_OK) {
#if defined(BCMPCIE)
		if (dhd->pub.osh) {
			struct pci_dev *pdev = NULL;
			pdev = osl_pci_device(dhd->pub.osh);
			if (pdev) {
				wl_cfg80211_set_parent_dev(&(pdev->dev));
			}
		}
#endif /* BCMPCIE */
		if (unlikely(wl_cfg80211_attach(net, &dhd->pub))) {
			DHD_ERROR(("wl_cfg80211_attach failed\n"));
			goto fail;
		}
	}

	wl_monitor_init(&dhd->pub);
	dhd_state |= DHD_ATTACH_STATE_CFG80211;
#endif /* WL_CFG80211 */
#if defined(WL_WIRELESS_EXT)
	/* Attach and link in the iw */
	if (!(dhd_state &  DHD_ATTACH_STATE_CFG80211)) {
		if (wl_iw_attach(net, (void *)&dhd->pub) != 0) {
		DHD_ERROR(("wl_iw_attach failed\n"));
		goto fail;
	}
	dhd_state |= DHD_ATTACH_STATE_WL_ATTACH;
	}
#endif /* defined(WL_WIRELESS_EXT) */
	wl_core_init((void *)&dhd->pub);

	if (dhd_sta_pool_init(&dhd->pub, DHD_MAX_STA) != BCME_OK) {
		DHD_ERROR(("%s: Initializing %u sta\n", __FUNCTION__, DHD_MAX_STA));
		goto fail;
	}

#if defined(BCM_CPE_PKTC)
	dhd->pktc_tbl = (wl_pktc_tbl_t *)dhd_pktc_attach((void *)(&dhd->pub));
	if (dhd->pktc_tbl == NULL) {
		DHD_ERROR(("%s: dhd_pktc_attach() failed\n", __FUNCTION__));
		goto fail;
	}
#endif /* BCM_CPE_PKTC */

#if defined(BCM_AWL)
	/* awl_attach might change the dhd_dpc_prio based on the mode, so dhd can
	 * run as thread instead of tasklet
	 */
	if ((dhd->pub.awl_cb = dhd_awl_attach(&dhd->pub, dhd->pub.unit)) == NULL) {
	    DHD_ERROR(("wl%d: dhd_awl_attach failed\n", dhd->pub.unit));
	    goto fail;
	}
#endif /* BCM_AWL */

#ifndef BCMDBUS
	/* Set up the watchdog timer */
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
	init_timer(&dhd->timer);
	dhd->timer.data = (ulong)dhd;
	dhd->timer.function = dhd_watchdog;
#else
	timer_setup(&dhd->timer, dhd_watchdog, 0);
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0) */

	dhd->default_wd_interval = dhd_watchdog_ms;

	if (dhd_watchdog_prio >= 0) {
		/* Initialize watchdog thread */
		PROC_START(dhd_watchdog_thread, dhd, &dhd->thr_wdt_ctl, 0, "dhd_watchdog_thread");

	} else {
		dhd->thr_wdt_ctl.thr_pid = -1;
	}

	/* Set up the bottom half handler */
	if (dhd_dpc_prio >= 0) {
		/* Initialize DPC thread */
#if (defined(BCM_ROUTER_DHD) && defined(BCM_GMAC3))
		sprintf(dhd_dpc_thr_name[dhd->pub.unit], "dhd%d_dpc", dhd->pub.unit);
		PROC_START(dhd_dpc_thread, dhd, &dhd->thr_dpc_ctl,
			0, dhd_dpc_thr_name[dhd->pub.unit]);
#else
		PROC_START(dhd_dpc_thread, dhd, &dhd->thr_dpc_ctl, 0, "dhd_dpc");
#endif
	} else {
#if defined(CUSTOMER_HW4) && defined(ARGOS_CPU_SCHEDULER)
		if (!zalloc_cpumask_var(&dhd->pub.default_cpu_mask, GFP_KERNEL)) {
			DHD_ERROR(("dpc tasklet, zalloc_cpumask_var error\n"));
			dhd->pub.affinity_isdpc = FALSE;
		} else {
			if (!zalloc_cpumask_var(&dhd->pub.dpc_affinity_cpu_mask, GFP_KERNEL)) {
				DHD_ERROR(("dpc thread, dpc_affinity_cpu_mask  error\n"));
				free_cpumask_var(dhd->pub.default_cpu_mask);
				dhd->pub.affinity_isdpc = FALSE;
			} else {
				cpumask_copy(dhd->pub.default_cpu_mask, &hmp_slow_cpu_mask);
				cpumask_or(dhd->pub.dpc_affinity_cpu_mask,
					dhd->pub.dpc_affinity_cpu_mask,
					cpumask_of(TASKLET_CPUCORE));

				set_cpucore_for_interrupt(dhd->pub.default_cpu_mask,
					dhd->pub.dpc_affinity_cpu_mask);
				dhd->pub.affinity_isdpc = TRUE;
			}
		}
#endif /* CUSTOMER_HW4 && ARGOS_CPU_SCHEDULER */
		/*  use tasklet for dpc */
		tasklet_init(&dhd->tasklet, dhd_dpc, (ulong)dhd);
		dhd->thr_dpc_ctl.thr_pid = -1;
	}

	if (dhd->rxthread_enabled) {
		bzero(&dhd->pub.skbbuf[0], sizeof(void *) * MAXSKBPEND);
		/* Initialize RXF thread */
		PROC_START(dhd_rxf_thread, dhd, &dhd->thr_rxf_ctl, 0, "dhd_rxf");
	}
#endif /* BCMDBUS */

	dhd_state |= DHD_ATTACH_STATE_THREADS_CREATED;

#ifdef BCM_NBUFF_WLMCAST_IPV6
	dhd->nic_hook_fn = dhd_nic_hook_fn;
#endif

#if defined(CONFIG_PM_SLEEP)
	dhd->pm_notifier.notifier_call = dhd_pm_callback;
	dhd->pm_notifier.priority = 10;
	register_pm_notifier(&dhd->pm_notifier);
#endif /* CONFIG_PM_SLEEP */

#if defined(CONFIG_HAS_EARLYSUSPEND) && defined(DHD_USE_EARLYSUSPEND)
	dhd->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 20;
	dhd->early_suspend.suspend = dhd_early_suspend;
	dhd->early_suspend.resume = dhd_late_resume;
	register_early_suspend(&dhd->early_suspend);
	dhd_state |= DHD_ATTACH_STATE_EARLYSUSPEND_DONE;
#endif /* CONFIG_HAS_EARLYSUSPEND && DHD_USE_EARLYSUSPEND */

#ifdef ARP_OFFLOAD_SUPPORT
	dhd->pend_ipaddr = 0;
	if (!dhd_inetaddr_notifier_registered) {
		dhd_inetaddr_notifier_registered = TRUE;
		register_inetaddr_notifier(&dhd_inetaddr_notifier);
	}
#endif /* ARP_OFFLOAD_SUPPORT */

#if defined(OEM_ANDROID) && defined(CONFIG_IPV6)
	if (!dhd_inet6addr_notifier_registered) {
		dhd_inet6addr_notifier_registered = TRUE;
		register_inet6addr_notifier(&dhd_inet6addr_notifier);
	}
#endif /* OEM_ANDROID && CONFIG_IPV6 */
	dhd->dhd_deferred_wq = dhd_deferred_work_init((void *)dhd);
#ifdef DEBUG_CPU_FREQ
	dhd->new_freq = alloc_percpu(int);
	dhd->freq_trans.notifier_call = dhd_cpufreq_notifier;
	cpufreq_register_notifier(&dhd->freq_trans, CPUFREQ_TRANSITION_NOTIFIER);
#endif
#ifdef DHDTCPACK_SUPPRESS
#if defined(BCMPCIE)
	/* xxx : In case of PCIe based Samsung Android project, enable TCP ACK Suppress
	 *       when throughput is higher than threshold, following rps_cpus setting.
	 */
#if (defined(SET_RPS_CPUS) || defined(ARGOS_RPS_CPU_CTL))
	dhd_tcpack_suppress_set(&dhd->pub, TCPACK_SUP_OFF);
#else
	dhd_tcpack_suppress_set(&dhd->pub, TCPACK_SUP_HOLD);
#endif /* (SET_RPS_CPUS || ARGOS_RPS_CPU_CTL) */
#else
	dhd_tcpack_suppress_set(&dhd->pub, TCPACK_SUP_OFF);
#endif
#endif /* DHDTCPACK_SUPPRESS */

#if defined(BCM_DNGL_EMBEDIMAGE) || defined(BCM_REQUEST_FW)
#if defined(BCMDBUS)
	sema_init(&dhd->fw_download_lock, 0);
	dhd->fw_download_task = kthread_run(fw_download_thread_func, dhd, "fwdl-thread");
#endif /* BCMDBUS */
#endif /* defined(BCM_DNGL_EMBEDIMAGE) || defined(BCM_REQUEST_FW) */

	dhd_state |= DHD_ATTACH_STATE_DONE;
	dhd->dhd_state = dhd_state;

	dhd_found++;

#ifdef DHD_DEBUG_PAGEALLOC
	register_page_corrupt_cb(dhd_page_corrupt_cb, &dhd->pub);
#endif /* DHD_DEBUG_PAGEALLOC */

#if defined(DHD_LB)
	DHD_ERROR(("DHD LOAD BALANCING Enabled\n"));

	dhd_lb_set_default_cpus(dhd);

	/* Initialize the CPU Masks */
	if (dhd_cpumasks_init(dhd) ==  0) {
		/* Now we have the current CPU maps, run through candidacy */
		dhd_select_cpu_candidacy(dhd);

		/*
		 * If we are able to initialize CPU masks, lets register to the
		 * CPU Hotplug framework to change the CPU for each job dynamically
		 * using candidacy algorithm.
		 */
		dhd->cpu_notifier.notifier_call = dhd_cpu_callback;
		register_cpu_notifier(&dhd->cpu_notifier); /* Register a callback */
	} else {
		/*
		 * We are unable to initialize CPU masks, so candidacy algorithm
		 * won't run, but still Load Balancing will be honoured based
		 * on the CPUs allocated for a given job statically during init
		 */
		dhd->cpu_notifier.notifier_call = NULL;
		DHD_ERROR(("%s(): dhd_cpumasks_init failed CPUs for JOB would be static\n",
			__FUNCTION__));
	}

	DHD_LB_STATS_INIT(&dhd->pub);

	/* Initialize the Load Balancing Tasklets and Napi object */
#if defined(DHD_LB_TXC)
	tasklet_init(&dhd->tx_compl_tasklet,
		dhd_lb_tx_compl_handler, (ulong)(&dhd->pub));
	INIT_WORK(&dhd->tx_compl_dispatcher_work, dhd_tx_compl_dispatcher_fn);
	DHD_INFO(("%s load balance init tx_compl_tasklet\n", __FUNCTION__));
#endif /* DHD_LB_TXC */

#if defined(DHD_LB_RXC)
	tasklet_init(&dhd->rx_compl_tasklet,
		dhd_lb_rx_compl_handler, (ulong)(&dhd->pub));
	INIT_WORK(&dhd->rx_compl_dispatcher_work, dhd_rx_compl_dispatcher_fn);
	DHD_INFO(("%s load balance init rx_compl_tasklet\n", __FUNCTION__));
#endif /* DHD_LB_RXC */

#if defined(DHD_LB_RXP)
	 __skb_queue_head_init(&dhd->rx_pend_queue);
	skb_queue_head_init(&dhd->rx_napi_queue);

	/* Initialize the work that dispatches NAPI job to a given core */
	INIT_WORK(&dhd->rx_napi_dispatcher_work, dhd_rx_napi_dispatcher_fn);
	DHD_INFO(("%s load balance init rx_napi_queue\n", __FUNCTION__));
#endif /* DHD_LB_RXP */

#endif /* DHD_LB */

	if (dhd_macdbg_attach(&dhd->pub) != BCME_OK) {
		DHD_ERROR(("%s: dhd_macdbg_attach fail\n", __FUNCTION__));
		goto fail;
	}

	/* Get dump file signature */
	dhd->dump_signature = (uint16)OSL_RAND();

	/* Allow all macdbg dumps by default */
	dhd->macdbg_dump_level = MAXBITVAL(MACDBG_DUMP_MAX);
#ifdef BCM_ROUTER_DHD
	snprintf(varstr, sizeof(varstr), "macdbg_dump_level%d", dhd->pub.unit);
	var = getvar(NULL, varstr);
	if (var) {
		/* override nvram setting */
		dhd->macdbg_dump_level = (uint32)bcm_strtoul(var, NULL, 0);
	}

	/* Default short packet padding enabled */
	dhd->pub.shortpktpad = TRUE;
	var = getvar(NULL, "dhd_shortpktpad");
	if (var) {
		/* override nvram setting */
		dhd->pub.shortpktpad = (bool)bcm_strtoul(var, NULL, 0);
	}
#endif /* BCM_ROUTER_DHD */
#if defined(BCM_PKTFWD)
	/* BCM_PKTFWD: insert radio into pktfwd */
	if ((dhd->pub.pktfwd_tbl = dhd_pktfwd_radio_ins(&dhd->pub, dhd->pub.unit, net)) == NULL) {
		DHD_ERROR(("wl%d: PKTFWD acceleration disabled\n", dhd->pub.unit));
	} else {
		DHD_ERROR(("wl%d: pktfwd_tbl 0x%px\n", dhd->pub.unit, dhd->pub.pktfwd_tbl));
	}
#endif /* BCM_PKTFWD */

#if defined(BCM_WFD)
	dhd->pub.wfd_idx = dhd_wfd_bind(net, dhd->pub.unit);
#if defined(WLAN_WFD_INVALID_IDX)
	if (dhd->pub.wfd_idx == WLAN_WFD_INVALID_IDX)
#else
	if (dhd->pub.wfd_idx < 0)
#endif
	{
		DHD_ERROR(("wl%d: dhd_wfd_bind failed\n", dhd->pub.unit));
		goto fail;
	}
#endif /* BCM_WFD */

#if defined(BCM_WFD)
	dhd->pub.fwder_unit = dhd->fwder_unit = dhd->pub.unit;
#endif /* BCM_WFD */

#if defined(BCM_WFD) || defined(BCM_PKTFWD)
	g_dhd_info[dhd->pub.unit] = &(dhd->pub);
#endif /* BCM_WFD || BCM_PKTFWD */

#ifdef DHD_LBR_AGGR_BCM_ROUTER
	dhd_aggr_init(&dhd->pub);
	dhd_lbr_aggr_init(&dhd->pub);
#endif /* DHD_LBR_AGGR_BCM_ROUTER */
#if defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE)
	dhd_spdsvc_init();
#endif

#if defined(BCM_NBUFF_PKT_BPM)
	if (dhd_nbuff_bpm_init(&dhd->pub) != BCME_OK)
		goto fail;
#endif /* BCM_NBUFF_PKT_BPM */

	dhd->pub.wl_ioctl_version = 0;
	dhd->pub.wl_ioctl_magic = 0;

#ifdef DHD_IFE
	dhd_ife_init(&dhd->pub);
#endif /* DHD_IFE */

	return &dhd->pub;

fail:
	if (dhd_state >= DHD_ATTACH_STATE_DHD_ALLOC) {
		DHD_TRACE(("%s: Calling dhd_detach dhd_state 0x%x &dhd->pub %p\n",
			__FUNCTION__, dhd_state, &dhd->pub));
		dhd->dhd_state = dhd_state;
		dhd_detach(&dhd->pub);
		dhd_free(&dhd->pub);
	}

	return NULL;
}

int dhd_get_fw_mode(dhd_info_t *dhdinfo)
{
	if (strstr(dhdinfo->fw_path, "_apsta") != NULL)
		return DHD_FLAG_HOSTAP_MODE;
	if (strstr(dhdinfo->fw_path, "_p2p") != NULL)
		return DHD_FLAG_P2P_MODE;
	if (strstr(dhdinfo->fw_path, "_ibss") != NULL)
		return DHD_FLAG_IBSS_MODE;
	if (strstr(dhdinfo->fw_path, "_mfg") != NULL)
		return DHD_FLAG_MFG_MODE;

	return DHD_FLAG_STA_MODE;
}

#ifndef BCMDBUS
bool
dhd_update_fw_nv_path(dhd_info_t *dhdinfo)
{
	int fw_len;
	int nv_len;
	const char *fw = NULL;
	const char *nv = NULL;
	wifi_adapter_info_t *adapter = dhdinfo->adapter;

	/* Update firmware and nvram path. The path may be from adapter info or module parameter
	 * The path from adapter info is used for initialization only (as it won't change).
	 *
	 * The firmware_path/nvram_path module parameter may be changed by the system at run
	 * time. When it changes we need to copy it to dhdinfo->fw_path. Also Android private
	 * command may change dhdinfo->fw_path. As such we need to clear the path info in
	 * module parameter after it is copied. We won't update the path until the module parameter
	 * is changed again (first character is not '\0')
	 */

	/* set default firmware and nvram path for built-in type driver */
	if (!dhd_download_fw_on_driverload) {
#ifdef CONFIG_BCMDHD_FW_PATH
		fw = CONFIG_BCMDHD_FW_PATH;
#endif /* CONFIG_BCMDHD_FW_PATH */
#ifdef CONFIG_BCMDHD_NVRAM_PATH
		nv = CONFIG_BCMDHD_NVRAM_PATH;
#endif /* CONFIG_BCMDHD_NVRAM_PATH */
	}

	/* check if we need to initialize the path */
	if (dhdinfo->fw_path[0] == '\0') {
		if (adapter && adapter->fw_path && adapter->fw_path[0] != '\0')
			fw = adapter->fw_path;

	}
	if (dhdinfo->nv_path[0] == '\0') {
		if (adapter && adapter->nv_path && adapter->nv_path[0] != '\0')
			nv = adapter->nv_path;
	}

	/* Use module parameter if it is valid, EVEN IF the path has not been initialized
	 *
	 * TODO: need a solution for multi-chip, can't use the same firmware for all chips
	 */
	if (firmware_path[0] != '\0')
		fw = firmware_path;
	if (nvram_path[0] != '\0')
		nv = nvram_path;

#ifdef BCM_ROUTER_DHD
	if (!fw) {
		char var[32];

		snprintf(var, sizeof(var), "firmware_path%d", dhdinfo->pub.unit);
		fw = getvar(NULL, var);
	}

#if !defined(BCMEMBEDIMAGE) && defined(EXTFDIMGPATH)
	if ((!fw) || (fw[0] == '\0')) {
		/* Use default specified path */
		/* Order: dhd_info, firmware_path, nvram, default */
		fw = EXTFDIMGPATH;
	}
#endif /* !defined(BCMEMBEDIMAGE) && defined(EXTFDIMGPATH) */

	if (!nv) {
		char var[32];

		snprintf(var, sizeof(var), "nvram_path%d", dhdinfo->pub.unit);
		nv = getvar(NULL, var);
	}
#endif /* BCM_ROUTER_DHD */
	DHD_ERROR(("dhd:%d: fw path:%s nv path:%s\n", dhdinfo->pub.unit, fw, nv));

	if (fw && fw[0] != '\0') {
		fw_len = strlen(fw);
		if (fw_len >= sizeof(dhdinfo->fw_path)) {
			DHD_ERROR(("fw path len exceeds max len of dhdinfo->fw_path\n"));
			return FALSE;
		}
		strncpy(dhdinfo->fw_path, fw, sizeof(dhdinfo->fw_path));
		if (dhdinfo->fw_path[fw_len-1] == '\n')
		       dhdinfo->fw_path[fw_len-1] = '\0';
	}
	if (nv && nv[0] != '\0') {
		nv_len = strlen(nv);
		if (nv_len >= sizeof(dhdinfo->nv_path)) {
			DHD_ERROR(("nvram path len exceeds max len of dhdinfo->nv_path\n"));
			return FALSE;
		}
		strncpy(dhdinfo->nv_path, nv, sizeof(dhdinfo->nv_path));
		if (dhdinfo->nv_path[nv_len-1] == '\n')
		       dhdinfo->nv_path[nv_len-1] = '\0';
	}

	/* clear the path in module parameter */
	/* exclude routers as firmware_path is needed for other enumerated dongles */
#if !defined(BCM_ROUTER_DHD) && !defined(EXTFDIMGPATH)
	firmware_path[0] = '\0';
	nvram_path[0] = '\0';
#endif /* !defined(BCM_ROUTER_DHD) && !defined(EXTFDIMGPATH) */

#ifndef BCMEMBEDIMAGE
	/* fw_path and nv_path are not mandatory for BCMEMBEDIMAGE */
	if ((dhdinfo->fw_path[0] == '\0') &&
#if defined(BCM_REQUEST_FW) && defined(OEM_ANDROID)
		(!dhd_download_fw_on_driverload) &&
#endif
		(TRUE)) {
		DHD_ERROR(("firmware path not found\n"));
		return FALSE;
	}
#ifndef STBLINUX
#if !defined(BCM_ROUTER_DHD) && !defined(EXTFDIMGPATH)
	if (dhdinfo->nv_path[0] == '\0') {
		DHD_ERROR(("nvram path not found\n"));
		return FALSE;
	}
#endif /* !defined(BCM_ROUTER_DHD) && !defined(EXTFDIMGPATH) */
#endif /* STBLINUX */
#endif /* BCMEMBEDIMAGE */

	return TRUE;
}

#ifdef CUSTOMER_HW4
bool
dhd_validate_chipid(dhd_pub_t *dhdp)
{
	uint chipid = dhd_bus_chip_id(dhdp);
	uint config_chipid;

#ifdef BCM4359_CHIP
	config_chipid = BCM4359_CHIP_ID;
#elif defined(BCM4358_CHIP)
	config_chipid = BCM4358_CHIP_ID;
#elif defined(BCM4354_CHIP)
	config_chipid = BCM4354_CHIP_ID;
#elif defined(BCM4356_CHIP)
	config_chipid = BCM4356_CHIP_ID;
#elif defined(BCM4339_CHIP)
	config_chipid = BCM4339_CHIP_ID;
#elif defined(BCM43349_CHIP)
	config_chipid = BCM43349_CHIP_ID;
#elif defined(BCM4335_CHIP)
	config_chipid = BCM4335_CHIP_ID;
#elif defined(BCM43241_CHIP)
	config_chipid = BCM4324_CHIP_ID;
#elif defined(BCM4334_CHIP)
	config_chipid = BCM4334_CHIP_ID;
#elif defined(BCM4330_CHIP)
	config_chipid = BCM4330_CHIP_ID;
#elif defined(BCM43430_CHIP)
	config_chipid = BCM43430_CHIP_ID;
#elif defined(BCM4334W_CHIP)
	config_chipid = BCM43342_CHIP_ID;
#elif defined(BCM43455_CHIP)
	config_chipid = BCM4345_CHIP_ID;
#else
	DHD_ERROR(("%s: Unknown chip id, if you use new chipset, "
		" please add CONFIG_BCMXXXX into the Kernel and"
		" BCMXXXX_CHIP definition into the DHD driver\n",
		__FUNCTION__));
	config_chipid = 0;

	return FALSE;
#endif /* BCM4354_CHIP */

#if defined(BCM4354_CHIP) && defined(SUPPORT_MULTIPLE_REVISION)
	if (chipid == BCM4350_CHIP_ID && config_chipid == BCM4354_CHIP_ID) {
		return TRUE;
	}
#endif /* BCM4354_CHIP && SUPPORT_MULTIPLE_REVISION */
#if defined(BCM4358_CHIP) && defined(SUPPORT_MULTIPLE_REVISION)
	if (chipid == BCM43569_CHIP_ID && config_chipid == BCM4358_CHIP_ID) {
		return TRUE;
	}
#endif /* BCM4358_CHIP && SUPPORT_MULTIPLE_REVISION */
#if defined(BCM4359_CHIP)
	if (chipid == BCM4355_CHIP_ID && config_chipid == BCM4359_CHIP_ID) {
		return TRUE;
	}
#endif /* BCM4359_CHIP */

	return config_chipid == chipid;
}
#endif /* CUSTOMER_HW4 */

int
dhd_bus_start(dhd_pub_t *dhdp)
{
	int ret = -1;
	dhd_info_t *dhd = (dhd_info_t*)dhdp->info;
	unsigned long flags;
#ifdef WL_CFG80211
	struct net_device *dev;
#endif /* WL_CFG80211 */

	ASSERT(dhd);

	DHD_TRACE(("Enter %s:\n", __FUNCTION__));

	/* try to download image and nvram to the dongle */
	if  (((dhd->pub.busstate == DHD_BUS_DOWN) || (dhd->pub.busstate == DHD_BUS_SUSPENDED)) &&
		dhd_update_fw_nv_path(dhd)) {
		DHD_ERROR(("%s download fw %s, nv %s\n", __FUNCTION__, dhd->fw_path, dhd->nv_path));
		ret = dhd_bus_download_firmware(dhd->pub.bus, dhd->pub.osh,
		                                dhd->fw_path, dhd->nv_path);
		if (ret < 0) {
			DHD_ERROR(("%s: failed to download firmware %s\n",
			          __FUNCTION__, dhd->fw_path));
			return ret;
		}
	}
	if (dhd->pub.busstate != DHD_BUS_LOAD) {
		return -ENETDOWN;
	}

	dhd_os_sdlock(dhdp);

	/* Start the watchdog timer */
	dhd->pub.tickcnt = 0;
	dhd_os_wd_timer(&dhd->pub, dhd_watchdog_ms);

	/* Bring up the bus */
	if ((ret = dhd_bus_init(&dhd->pub, FALSE)) != 0) {

		DHD_ERROR(("%s, dhd_bus_init failed %d\n", __FUNCTION__, ret));
		dhd_os_sdunlock(dhdp);
		return ret;
	}
#if defined(OOB_INTR_ONLY) || defined(BCMSPI_ANDROID) || defined(BCMPCIE_OOB_HOST_WAKE)
#if defined(BCMPCIE_OOB_HOST_WAKE)
	dhd_os_sdunlock(dhdp);
#endif /* BCMPCIE_OOB_HOST_WAKE */
	/* Host registration for OOB interrupt */
	if (dhd_bus_oob_intr_register(dhdp)) {
		/* deactivate timer and wait for the handler to finish */
#if !defined(BCMPCIE_OOB_HOST_WAKE)
		DHD_GENERAL_LOCK(&dhd->pub, flags);
		dhd->wd_timer_valid = FALSE;
		DHD_GENERAL_UNLOCK(&dhd->pub, flags);
		del_timer_sync(&dhd->timer);

		dhd_os_sdunlock(dhdp);
#endif /* !BCMPCIE_OOB_HOST_WAKE */
		DHD_OS_WD_WAKE_UNLOCK(&dhd->pub);
		DHD_ERROR(("%s Host failed to register for OOB\n", __FUNCTION__));
		return -ENODEV;
	}

#if defined(BCMPCIE_OOB_HOST_WAKE)
	dhd_os_sdlock(dhdp);
	dhd_bus_oob_intr_set(dhdp, TRUE);
#else
	/* Enable oob at firmware */
	dhd_enable_oob_intr(dhd->pub.bus, TRUE);
#endif /* BCMPCIE_OOB_HOST_WAKE */
#endif /* OOB_INTR_ONLY || BCMSPI_ANDROID || BCMPCIE_OOB_HOST_WAKE */
#ifdef PCIE_FULL_DONGLE
	{
		/* max_h2d_rings includes H2D common rings */
		uint32 max_h2d_rings = dhd_bus_max_h2d_rings(dhd->pub.bus);

		DHD_ERROR(("%s: Initializing %u h2drings\n", __FUNCTION__,
			max_h2d_rings));
		if ((ret = dhd_flow_rings_init(&dhd->pub, max_h2d_rings)) != BCME_OK) {
			dhd_os_sdunlock(dhdp);
			return ret;
		}

#ifdef DHD_IFE
		dhd_ife_evict_timer_start(&dhd->pub);
#endif /* DHD_IFE */
	}
#endif /* PCIE_FULL_DONGLE */

	/* Do protocol initialization necessary for IOCTL/IOVAR */
#ifdef PCIE_FULL_DONGLE
	dhd_os_sdunlock(dhdp);
#endif /* PCIE_FULL_DONGLE */
	ret = dhd_prot_init(&dhd->pub);
	if (ret != BCME_OK) {
		DHD_OS_WD_WAKE_UNLOCK(&dhd->pub);
		return ret;
	}
#ifdef PCIE_FULL_DONGLE
	dhd_os_sdlock(dhdp);
#endif /* PCIE_FULL_DONGLE */

	/* If bus is not ready, can't come up */
	if (dhd->pub.busstate != DHD_BUS_DATA) {
		DHD_GENERAL_LOCK(&dhd->pub, flags);
		dhd->wd_timer_valid = FALSE;
		DHD_GENERAL_UNLOCK(&dhd->pub, flags);
		del_timer_sync(&dhd->timer);
		DHD_ERROR(("%s failed bus is not ready\n", __FUNCTION__));
		dhd_os_sdunlock(dhdp);
		DHD_OS_WD_WAKE_UNLOCK(&dhd->pub);
		return -ENODEV;
	}

	dhd_os_sdunlock(dhdp);

	/* start the lock when doing dongle IOCTL at starup stage */
	DHD_LOCK(dhdp);

	/* Bus is ready, query any dongle information */
	if ((ret = dhd_sync_with_dongle(&dhd->pub)) < 0) {
		DHD_UNLOCK(dhdp);
		return ret;
	}

#ifdef WL_CFG80211
	/* update wiphy mac address after sync */
	dev = dhd_idx2net(dhdp, 0);
	if (dev->ieee80211_ptr) {
		memcpy(dhd_idx2net(dhdp, 0)->ieee80211_ptr->wiphy->perm_addr,
			dhdp->mac.octet, ETHER_ADDR_LEN);
	}
#endif /* WL_CFG80211 */
#ifdef ARP_OFFLOAD_SUPPORT
	if (dhd->pend_ipaddr) {
#ifdef AOE_IP_ALIAS_SUPPORT
		aoe_update_host_ipv4_table(&dhd->pub, dhd->pend_ipaddr, TRUE, 0);
#endif /* AOE_IP_ALIAS_SUPPORT */
		dhd->pend_ipaddr = 0;
	}
#endif /* ARP_OFFLOAD_SUPPORT */

#if defined(BCM_ROUTER_DHD) || defined(TRAFFIC_MGMT_DWM)
	bzero(&dhd->pub.dhd_tm_dwm_tbl, sizeof(dhd_trf_mgmt_dwm_tbl_t));
#endif /* BCM_ROUTER_DHD || TRAFFIC_MGMT_DWM */
	DHD_UNLOCK(dhdp);

	return 0;
}
#endif /* BCMDBUS */

#ifdef WLTDLS
int
_dhd_tdls_enable(dhd_pub_t *dhd, bool tdls_on, bool auto_on, struct ether_addr *mac)
{
	struct net_device *ndev;
	ndev  = dhd_linux_get_primary_netdev(dhd);

	return _wl_tdls_enable(ndev, dhd->wlcore, tdls_on, auto_on, mac);
}

int
dhd_tdls_enable(struct net_device *dev, bool tdls_on, bool auto_on, struct ether_addr *mac)
{
	dhd_info_t *dhd = DHD_DEV_INFO(dev);
	int ret = 0;
	if (dhd)
		ret = _dhd_tdls_enable(&dhd->pub, tdls_on, auto_on, mac);
	else
		ret = BCME_ERROR;

	return ret;
}

int
dhd_tdls_set_mode(dhd_pub_t *dhd, bool wfd_mode)
{
	int ret = 0;
	bool auto_on = false;
	uint32 mode =  wfd_mode;

#ifdef CUSTOMER_HW4
	if (wfd_mode) {
		auto_on = false;
	} else {
		auto_on = true;
	}
#else
	auto_on = false;
#endif
	ret = _dhd_tdls_enable(dhd, false, auto_on, NULL);
	if (ret < 0) {
		DHD_ERROR(("Disable tdls_auto_op failed. %d\n", ret));
		return ret;
	}

	ret = dhd_iovar(dhd, 0, "tdls_wfd_mode", (char *)&mode, sizeof(mode), NULL, 0, TRUE);
	if ((ret < 0) && (ret != BCME_UNSUPPORTED)) {
		DHD_ERROR(("%s: tdls_wfd_mode faile_wfd_mode %d\n", __FUNCTION__, ret));
		return ret;
	}

	ret = _dhd_tdls_enable(dhd, true, auto_on, NULL);
	if (ret < 0) {
		DHD_ERROR(("enable tdls_auto_op failed. %d\n", ret));
		return ret;
	}

	dhd->wlcore->tdls_mode = mode;

	return ret;
}

#ifdef PCIE_FULL_DONGLE
void
dhd_tdls_update_peer_info(struct net_device *dev, bool connect, uint8 *da)
{
	dhd_info_t *dhd = DHD_DEV_INFO(dev);
	dhd_pub_t *dhdp =  (dhd_pub_t *)&dhd->pub;
	tdls_peer_node_t *cur = dhdp->peer_tbl.node;
	tdls_peer_node_t *new = NULL, *prev = NULL;
	dhd_if_t *dhdif;
	uint8 sa[ETHER_ADDR_LEN];
	int ifidx = dhd_net2idx(dhd, dev);

	if (ifidx == DHD_BAD_IF)
		return;

	dhdif = dhd->iflist[ifidx];
	memcpy(sa, dhdif->mac_addr, ETHER_ADDR_LEN);

	if (connect) {
		while (cur != NULL) {
			if (!memcmp(da, cur->addr, ETHER_ADDR_LEN)) {
				DHD_ERROR(("%s: TDLS Peer exist already %d\n",
					__FUNCTION__, __LINE__));
				return;
			}
			cur = cur->next;
		}

		new = MALLOC(dhdp->osh, sizeof(tdls_peer_node_t));
		if (new == NULL) {
			DHD_ERROR(("%s: Failed to allocate memory\n", __FUNCTION__));
			return;
		}
		memcpy(new->addr, da, ETHER_ADDR_LEN);
		new->next = dhdp->peer_tbl.node;
		dhdp->peer_tbl.node = new;
		dhdp->peer_tbl.tdls_peer_count++;

	} else {
		while (cur != NULL) {
			if (!memcmp(da, cur->addr, ETHER_ADDR_LEN)) {
				dhd_flow_rings_delete_for_peer(dhdp, ifidx, da);
				if (prev)
					prev->next = cur->next;
				else
					dhdp->peer_tbl.node = cur->next;
				MFREE(dhdp->osh, cur, sizeof(tdls_peer_node_t));
				dhdp->peer_tbl.tdls_peer_count--;
				return;
			}
			prev = cur;
			cur = cur->next;
		}
		DHD_ERROR(("%s: TDLS Peer Entry Not found\n", __FUNCTION__));
	}
}
#endif /* PCIE_FULL_DONGLE */
#endif /* BCMDBUS */

bool
dhd_is_concurrent_mode(dhd_pub_t *dhd)
{
	if (!dhd)
		return FALSE;

	if (dhd->wlcore->op_mode & DHD_FLAG_CONCURR_MULTI_CHAN_MODE)
		return TRUE;
	else if ((dhd->wlcore->op_mode & DHD_FLAG_CONCURR_SINGLE_CHAN_MODE) ==
		DHD_FLAG_CONCURR_SINGLE_CHAN_MODE)
		return TRUE;
	else
		return FALSE;
}

#if defined(OEM_ANDROID) && !defined(AP) && defined(WLP2P)
/* From Android JerryBean release, the concurrent mode is enabled by default and the firmware
 * name would be fw_bcmdhd.bin. So we need to determine whether P2P is enabled in the STA
 * firmware and accordingly enable concurrent mode (Apply P2P settings). SoftAP firmware
 * would still be named as fw_bcmdhd_apsta.
 */
uint32
dhd_get_concurrent_capabilites(dhd_pub_t *dhd)
{
	int32 ret = 0;
	char buf[WLC_IOCTL_SMLEN];
	bool mchan_supported = FALSE;
	/* if dhd->wlcore->op_mode is already set for HOSTAP and Manufacturing
	 * test mode, that means we only will use the mode as it is
	 */
	if (dhd->wlcore->op_mode & (DHD_FLAG_HOSTAP_MODE | DHD_FLAG_MFG_MODE))
		return 0;
	if (FW_SUPPORTED(dhd->wlcore, vsdb)) {
		mchan_supported = TRUE;
	}
	if (!FW_SUPPORTED(dhd->wlcore, p2p)) {
		DHD_TRACE(("Chip does not support p2p\n"));
		return 0;
	} else {
		/* Chip supports p2p but ensure that p2p is really implemented in firmware or not */
		memset(buf, 0, sizeof(buf));
		ret = dhd_iovar(dhd, 0, "p2p", NULL, 0, (char *)&buf,
				sizeof(buf), FALSE);
		if (ret < 0) {
			DHD_ERROR(("%s: Get P2P failed (error=%d)\n", __FUNCTION__, ret));
			return 0;
		} else {
			if (buf[0] == 1) {
				/* By default, chip supports single chan concurrency,
				* now lets check for mchan
				*/
				ret = DHD_FLAG_CONCURR_SINGLE_CHAN_MODE;
				if (mchan_supported)
					ret |= DHD_FLAG_CONCURR_MULTI_CHAN_MODE;
				if (FW_SUPPORTED(dhd->wlcore, mp2p)) {
					ret |= DHD_FLAG_MP2P_MODE;
				}
#if defined(WL_ENABLE_P2P_IF) || defined(CUSTOMER_HW4) || \
	defined(WL_CFG80211_P2P_DEV_IF)
				/* For customer_hw4, although ICS,
				* we still support concurrent mode
				*/
				return ret;
#else
				return 0;
#endif /* WL_ENABLE_P2P_IF || CUSTOMER_HW4 || WL_CFG80211_P2P_DEV_IF */
			}
		}
	}

	return 0;
}
#endif /* defined(OEM_ANDROID) && !defined(AP) && defined(WLP2P) */

#ifdef SUPPORT_AP_POWERSAVE
#define RXCHAIN_PWRSAVE_PPS			10
#define RXCHAIN_PWRSAVE_QUIET_TIME		10
#define RXCHAIN_PWRSAVE_STAS_ASSOC_CHECK	0
int
dhd_set_ap_powersave(dhd_pub_t *dhdp, int ifidx, int enable)
{
	char iovbuf[128];
	int32 pps = RXCHAIN_PWRSAVE_PPS;
	int32 quiet_time = RXCHAIN_PWRSAVE_QUIET_TIME;
	int32 stas_assoc_check = RXCHAIN_PWRSAVE_STAS_ASSOC_CHECK;
	int ret;

	if (enable) {
		ret = dhd_iovar(dhdp, 0, "rxchain_pwrsave_enable", (char *)&enable, sizeof(enable),
				NULL, 0, TRUE);
		if (ret != BCME_OK) {
			DHD_ERROR(("Failed to enable AP power save"));
		}
		ret = dhd_iovar(dhdp, 0, "rxchain_pwrsave_pps", (char *)&pps, sizeof(pps), NULL, 0,
				TRUE);
		if (ret != BCME_OK) {
			DHD_ERROR(("Failed to set pps"));
		}
		ret = dhd_iovar(dhdp, 0, "rxchain_pwrsave_quiet_time", (char *)&quiet_time,
				sizeof(quiet_time), NULL, 0, TRUE);
		if (ret != BCME_OK) {
			DHD_ERROR(("Failed to set quiet time"));
		}
		ret = dhd_iovar(dhdp., 0, "rxchain_pwrsave_stas_assoc_check",
				(char *)&stas_assoc_check, sizeof(stas_assoc_check), NULL, 0, TRUE);
		if (ret != BCME_OK) {
			DHD_ERROR(("Failed to set stas assoc check"));
		}
	} else {
		ret = dhd_iovar(dhdp, 0, "rxchain_pwrsave_enable", (char *)&enable, sizeof(enable),
				NULL, 0, TRUE);
		if (ret != BCME_OK) {
			DHD_ERROR(("Failed to disable AP power save"));
		}
	}

	return 0;
}
#endif /* SUPPORT_AP_POWERSAVE */

#if defined(READ_CONFIG_FROM_FILE)
#include <linux/fs.h>
#include <linux/ctype.h>

#define strtoul(nptr, endptr, base) bcm_strtoul((nptr), (endptr), (base))
bool PM_control = TRUE;

static int
dhd_preinit_proc(dhd_pub_t *dhd, int ifidx, char *name, char *value)
{
	int var_int;
	wl_country_t cspec = {{0}, -1, {0}};
	char *revstr;
	char *endptr = NULL;
#ifdef ROAM_AP_ENV_DETECTION
	int roam_env_mode = AP_ENV_INDETERMINATE;
#endif /* ROAM_AP_ENV_DETECTION */

	if (!strcmp(name, "country")) {
		revstr = strchr(value, '/');
		if (revstr) {
			cspec.rev = strtoul(revstr + 1, &endptr, 10);
			memcpy(cspec.country_abbrev, value, WLC_CNTRY_BUF_SZ);
			cspec.country_abbrev[2] = '\0';
			memcpy(cspec.ccode, cspec.country_abbrev, WLC_CNTRY_BUF_SZ);
		} else {
			cspec.rev = -1;
			memcpy(cspec.country_abbrev, value, WLC_CNTRY_BUF_SZ);
			memcpy(cspec.ccode, value, WLC_CNTRY_BUF_SZ);
			get_customized_country_code(dhd->info->adapter,
				(char *)&cspec.country_abbrev, &cspec);
		}
		DHD_ERROR(("config country code is country : %s, rev : %d !!\n",
			cspec.country_abbrev, cspec.rev));
		return dhd_iovar(dhd, 0, "country", (char*)&cspec, sizeof(cspec), NULL, 0, TRUE);
	} else if (!strcmp(name, "roam_scan_period")) {
		var_int = (int)simple_strtol(value, NULL, 0);
		return dhd_wl_ioctl_cmd(dhd, WLC_SET_ROAM_SCAN_PERIOD,
			&var_int, sizeof(var_int), TRUE, 0);
	} else if (!strcmp(name, "roam_delta")) {
		struct {
			int val;
			int band;
		} x;
		x.val = (int)simple_strtol(value, NULL, 0);
		/* x.band = WLC_BAND_AUTO; */
		x.band = WLC_BAND_ALL;
		return dhd_wl_ioctl_cmd(dhd, WLC_SET_ROAM_DELTA, &x, sizeof(x), TRUE, 0);
	} else if (!strcmp(name, "roam_trigger")) {
		int ret = 0;

		roam_trigger[0] = (int)simple_strtol(value, NULL, 0);
		roam_trigger[1] = WLC_BAND_ALL;
		ret = dhd_wl_ioctl_cmd(dhd, WLC_SET_ROAM_TRIGGER, &roam_trigger,
			sizeof(roam_trigger), TRUE, 0);

#ifdef ROAM_AP_ENV_DETECTION
		if (roam_trigger[0] == WL_AUTO_ROAM_TRIGGER) {
			if (dhd_iovar(dhd, 0, "roam_env_detection",
			    (char *)&roam_env_mode, sizeof(roam_env_mode), NULL,
			    0, TRUE) == BCME_OK) {
				dhd->roam_env_detection = TRUE;
			} else {
				dhd->roam_env_detection = FALSE;
			}
		}
#endif /* ROAM_AP_ENV_DETECTION */
		return ret;
	} else if (!strcmp(name, "PM")) {
		int ret = 0;
		var_int = (int)simple_strtol(value, NULL, 0);

		ret =  dhd_wl_ioctl_cmd(dhd, WLC_SET_PM,
			&var_int, sizeof(var_int), TRUE, 0);

#if defined(CONFIG_CONTROL_PM) || defined(CONFIG_PM_LOCK)
		if (var_int == 0) {
			g_pm_control = TRUE;
			printk("%s var_int=%d don't control PM\n", __func__, var_int);
		} else {
			g_pm_control = FALSE;
			printk("%s var_int=%d do control PM\n", __func__, var_int);
		}
#endif

		return ret;
	}
#ifdef WLBTAMP
	else if (!strcmp(name, "btamp_chan")) {
		int btamp_chan;
		int iov_len = 0;
		int ret;

		btamp_chan = (int)simple_strtol(value, NULL, 0);
		ret = dhd_iovar(dhd, 0, "btamp_chan", (char*)&btamp_chan, sizeof(btamp_chan),
			NULL, 0, TRUE);
		if (ret < 0)
			DHD_ERROR(("%s btamp_chan=%d set failed code %d\n",
				__FUNCTION__, btamp_chan, ret));
		else
			DHD_ERROR(("%s btamp_chan %d set success\n",
				__FUNCTION__, btamp_chan));
	}
#endif /* WLBTAMP */
	else if (!strcmp(name, "band")) {
		int ret;
		if (!strcmp(value, "auto"))
			var_int = WLC_BAND_AUTO;
		else if (!strcmp(value, "a"))
			var_int = WLC_BAND_5G;
		else if (!strcmp(value, "b"))
			var_int = WLC_BAND_2G;
		else if (!strcmp(value, "all"))
			var_int = WLC_BAND_ALL;
		else {
			printk(" set band value should be one of the a or b or all\n");
			var_int = WLC_BAND_AUTO;
		}
		if ((ret = dhd_wl_ioctl_cmd(dhd, WLC_SET_BAND, &var_int,
			sizeof(var_int), TRUE, 0)) < 0)
			printk(" set band err=%d\n", ret);
		return ret;
	} else if (!strcmp(name, "cur_etheraddr")) {
		struct ether_addr ea;
		char buf[32];
		uint iovlen;
		int ret;

		bcm_ether_atoe(value, &ea);

		ret = memcmp(&ea.octet, dhd->mac.octet, ETHER_ADDR_LEN);
		if (ret == 0) {
			DHD_ERROR(("%s: Same Macaddr\n", __FUNCTION__));
			return 0;
		}

		DHD_ERROR(("%s: Change Macaddr = %02X:%02X:%02X:%02X:%02X:%02X\n", __FUNCTION__,
			ea.octet[0], ea.octet[1], ea.octet[2],
			ea.octet[3], ea.octet[4], ea.octet[5]));

		ret = dhd_iovar(dhd, 0, "cur_etheraddr", (char*)&ea, ETHER_ADDR_LEN, NULL, 0, TRUE);
		if (ret < 0) {
			DHD_ERROR(("%s: can't set MAC address , error=%d\n", __FUNCTION__, ret));
			return ret;
		} else {
			memcpy(dhd->mac.octet, (void *)&ea, ETHER_ADDR_LEN);
			return ret;
		}
	} else if (!strcmp(name, "lpc")) {
		int ret = 0;
		var_int = (int)simple_strtol(value, NULL, 0);
		if (dhd_wl_ioctl_cmd(dhd, WLC_DOWN, NULL, 0, TRUE, 0) < 0) {
			DHD_ERROR(("%s: wl down failed\n", __FUNCTION__));
		}
		ret = dhd_iovar(dhd, 0, "lpc", (char *)&var_int, sizeof(var_int), NULL, 0, TRUE);
		if (ret < 0) {
			DHD_ERROR(("%s Set lpc failed  %d\n", __FUNCTION__, ret));
		}
		if (dhd_wl_ioctl_cmd(dhd, WLC_UP, NULL, 0, TRUE, 0) < 0) {
			DHD_ERROR(("%s: wl up failed\n", __FUNCTION__));
		}
		return ret;
	} else if (!strcmp(name, "vht_features")) {
		int ret = 0;
		var_int = (int)simple_strtol(value, NULL, 0);

		if (dhd_wl_ioctl_cmd(dhd, WLC_DOWN, NULL, 0, TRUE, 0) < 0) {
			DHD_ERROR(("%s: wl down failed\n", __FUNCTION__));
		}
		ret = dhd_iovar(dhd, 0, "vht_features", (char *)&var_int, sizeof(var_int), NULL, 0,
				TRUE);
		if (ret < 0) {
			DHD_ERROR(("%s Set vht_features failed  %d\n", __FUNCTION__, ret));
		}
		if (dhd_wl_ioctl_cmd(dhd, WLC_UP, NULL, 0, TRUE, 0) < 0) {
			DHD_ERROR(("%s: wl up failed\n", __FUNCTION__));
		}
		return ret;
	} else {
		/* wlu_iovar_setint */
		var_int = (int)simple_strtol(value, NULL, 0);

		/* Setup timeout bcn_timeout from dhd driver 4.217.48 */
		if (!strcmp(name, "roam_off")) {
			/* Setup timeout if Beacons are lost to report link down */
			if (var_int) {
				uint bcn_timeout = 2;
				dhd_iovar(dhd, 0, "bcn_timeout", (char *)&bcn_timeout,
						sizeof(bcn_timeout), NULL, 0, TRUE);
			}
		}
		/* Setup timeout bcm_timeout from dhd driver 4.217.48 */

		DHD_INFO(("%s:[%s]=[%d]\n", __FUNCTION__, name, var_int));

		return dhd_iovar(dhd, 0, name, (char *)&var_int,
				sizeof(var_int), NULL, 0, TRUE);
	}

	return 0;
}

static int
dhd_preinit_config(dhd_pub_t *dhd, int ifidx)
{
	mm_segment_t old_fs;
	struct kstat stat;
	struct file *fp = NULL;
	unsigned int len;
	char *buf = NULL, *p, *name, *value;
	int ret = 0;
	char *config_path;

	config_path = CONFIG_BCMDHD_CONFIG_PATH;

	if (!config_path)
	{
		printk(KERN_ERR "config_path can't read. \n");
		return 0;
	}

	old_fs = get_fs();
	set_fs(get_ds());
	if ((ret = vfs_stat(config_path, &stat))) {
		set_fs(old_fs);
		printk(KERN_ERR "%s: Failed to get information (%d)\n",
			config_path, ret);
		return ret;
	}
	set_fs(old_fs);

	if (!(buf = MALLOC(dhd->osh, stat.size + 1))) {
		printk(KERN_ERR "Failed to allocate memory %llu bytes\n", stat.size);
		return -ENOMEM;
	}

	printk("dhd_preinit_config : config path : %s \n", config_path);

	if (!(fp = dhd_os_open_image(config_path)) ||
		(len = dhd_os_get_image_block(buf, stat.size, fp)) < 0)
		goto err;

	buf[stat.size] = '\0';
	for (p = buf; *p; p++) {
		if (isspace(*p))
			continue;
		for (name = p++; *p && !isspace(*p); p++) {
			if (*p == '=') {
				*p = '\0';
				p++;
				for (value = p; *p && !isspace(*p); p++);
				*p = '\0';
				if ((ret = dhd_preinit_proc(dhd, ifidx, name, value)) < 0) {
					printk(KERN_ERR "%s: %s=%s\n",
						bcmerrorstr(ret), name, value);
				}
				break;
			}
		}
	}
	ret = 0;

out:
	if (fp)
		dhd_os_close_image(fp);
	if (buf)
		MFREE(dhd->osh, buf, stat.size+1);
	return ret;

err:
	ret = -1;
	goto out;
}
#endif /* READ_CONFIG_FROM_FILE */

int
dhd_preinit_ioctls(dhd_pub_t *dhd)
{
	int ret = 0;
	char eventmask[WL_EVENTING_MASK_LEN];
	char iovbuf[WL_EVENTING_MASK_LEN + 12];	/*  Room for "event_msgs" + '\0' + bitvec  */
	uint32 buf_key_b4_m4 = 1;
	uint8 msglen;
	eventmsgs_ext_t *eventmask_msg = NULL;
	char* iov_buf = NULL;
	/* XXX: Use ret2 for return check of IOVARS that might return BCME_UNSUPPORTED,
	*	based on FW build tag.
	*/
	int ret2 = 0;
#if defined(BCMSUP_4WAY_HANDSHAKE) && defined(WLAN_AKM_SUITE_FT_8021X)
	uint32 sup_wpa = 0;
#endif
#if defined(CUSTOM_AMPDU_BA_WSIZE)
	uint32 ampdu_ba_wsize = 0;
#endif /* CUSTOM_AMPDU_BA_WSIZE */
#if defined(CUSTOM_AMPDU_MPDU)
	int32 ampdu_mpdu = 0;
#endif
#if defined(CUSTOM_AMPDU_RELEASE)
	int32 ampdu_release = 0;
#endif
#if defined(CUSTOM_AMSDU_AGGSF)
	int32 amsdu_aggsf = 0;
#endif

#if defined(BCMDBUS)
#ifdef PROP_TXSTATUS
	int wlfc_enable = TRUE;
#ifndef DISABLE_11N
	uint32 hostreorder = 1;
#endif /* DISABLE_11N */
#endif /* PROP_TXSTATUS */
#endif
#ifdef PCIE_FULL_DONGLE
	uint32 wl_ap_isolate;
#endif /* PCIE_FULL_DONGLE */

	uint32 frameburst = 1;
#ifdef OEM_ANDROID
#ifdef DHD_ENABLE_LPC
	uint32 lpc = 1;
#endif /* DHD_ENABLE_LPC */
	uint power_mode = PM_FAST;
#if defined(CUSTOMER_HW2) && defined(USE_WL_CREDALL)
	uint32 credall = 1;
#endif
#if defined(VSDB) || defined(ROAM_ENABLE)
	uint bcn_timeout = CUSTOM_BCN_TIMEOUT;
#else
	uint bcn_timeout = 4;
#endif /* CUSTOMER_HW4 && (VSDB || ROAM_ENABLE) */
#if defined(CUSTOMER_HW4) && defined(ENABLE_BCN_LI_BCN_WAKEUP)
	uint32 bcn_li_bcn = 1;
#endif /* CUSTOMER_HW4 && ENABLE_BCN_LI_BCN_WAKEUP */
	uint retry_max = CUSTOM_ASSOC_RETRY_MAX;
#if defined(ARP_OFFLOAD_SUPPORT)
	int arpoe = 1;
#endif
	int scan_assoc_time = DHD_SCAN_ASSOC_ACTIVE_TIME;
	int scan_unassoc_time = DHD_SCAN_UNASSOC_ACTIVE_TIME;
	int scan_passive_time = DHD_SCAN_PASSIVE_TIME;
	char buf[WLC_IOCTL_SMLEN];
	char *ptr;
	uint32 listen_interval = CUSTOM_LISTEN_INTERVAL; /* Default Listen Interval in Beacons */
#ifdef ROAM_ENABLE
	uint roamvar = 0;
	int roam_trigger[2] = {CUSTOM_ROAM_TRIGGER_SETTING, WLC_BAND_ALL};
	int roam_scan_period[2] = {10, WLC_BAND_ALL};
	int roam_delta[2] = {CUSTOM_ROAM_DELTA_SETTING, WLC_BAND_ALL};
#ifdef ROAM_AP_ENV_DETECTION
	int roam_env_mode = AP_ENV_INDETERMINATE;
#endif /* ROAM_AP_ENV_DETECTION */
#ifdef FULL_ROAMING_SCAN_PERIOD_60_SEC
	int roam_fullscan_period = 60;
#else /* FULL_ROAMING_SCAN_PERIOD_60_SEC */
	int roam_fullscan_period = 120;
#endif /* FULL_ROAMING_SCAN_PERIOD_60_SEC */
#else
#ifdef DISABLE_BUILTIN_ROAM
	uint roamvar = 1;
#endif /* DISABLE_BUILTIN_ROAM */
#endif /* ROAM_ENABLE */

#if defined(SOFTAP)
	uint dtim = 1;
#endif
#if (defined(AP) && !defined(WLP2P)) || (!defined(AP) && defined(WL_CFG80211))
	uint32 mpc = 0; /* Turn MPC off for AP/APSTA mode */
	struct ether_addr p2p_ea;
#endif
#ifdef BCMCCX
	uint32 ccx = 1;
#endif
#ifdef SOFTAP_UAPSD_OFF
	uint32 wme_apsd = 0;
#endif /* SOFTAP_UAPSD_OFF */
#if (defined(AP) || defined(WLP2P)) && !defined(SOFTAP_AND_GC)
	uint32 apsta = 1; /* Enable APSTA mode */
#elif defined(SOFTAP_AND_GC)
	uint32 apsta = 0;
	int ap_mode = 1;
#endif /* (defined(AP) || defined(WLP2P)) && !defined(SOFTAP_AND_GC) */
#ifdef GET_CUSTOM_MAC_ENABLE
	struct ether_addr ea_addr;
#endif /* GET_CUSTOM_MAC_ENABLE */
#ifdef OKC_SUPPORT
	uint32 okc = 1;
#endif

#ifdef DISABLE_11N
	uint32 nmode = 0;
#endif /* DISABLE_11N */

#if defined(DISABLE_11AC)
	uint32 vhtmode = 0;
#endif /* DISABLE_11AC */
#ifdef USE_WL_TXBF
	uint32 txbf = 1;
#endif /* USE_WL_TXBF */
#ifdef AMPDU_VO_ENABLE
	struct ampdu_tid_control tid;
#endif
#ifdef DHD_SET_FW_HIGHSPEED
	uint32 ack_ratio = 250;
	uint32 ack_ratio_depth = 64;
#endif /* DHD_SET_FW_HIGHSPEED */
#ifdef SUPPORT_2G_VHT
	uint32 vht_features = 0x3; /* 2G enable | rates all */
#endif /* SUPPORT_2G_VHT */
#ifdef CUSTOM_PSPRETEND_THR
	uint32 pspretend_thr = CUSTOM_PSPRETEND_THR;
#endif
#ifdef PKT_FILTER_SUPPORT
	dhd_pkt_filter_enable = TRUE;
#endif /* PKT_FILTER_SUPPORT */
#ifdef WLTDLS
	dhd->wlcore->tdls_enable = FALSE;
	dhd_tdls_set_mode(dhd, false);
#endif /* WLTDLS */
	dhd->suspend_bcn_li_dtim = CUSTOM_SUSPEND_BCN_LI_DTIM;
	DHD_TRACE(("Enter %s\n", __FUNCTION__));
	dhd->wlcore->op_mode = 0;
#ifdef CUSTOMER_HW4
	if (!dhd_validate_chipid(dhd)) {
		DHD_ERROR(("%s: CONFIG_BCMXXX and CHIP ID(%x) is mismatched\n",
			__FUNCTION__, dhd_bus_chip_id(dhd)));
#ifndef SUPPORT_MULTIPLE_CHIPS
		ret = BCME_BADARG;
		goto done;
#endif /* !SUPPORT_MULTIPLE_CHIPS */
	}
#endif /* CUSTOMER_HW4 */
	if ((!op_mode && dhd_get_fw_mode(dhd->info) == DHD_FLAG_MFG_MODE) ||
		(op_mode == DHD_FLAG_MFG_MODE)) {
		/* Check and adjust IOCTL response timeout for Manufactring firmware */
		dhd_os_set_ioctl_resp_timeout(MFG_IOCTL_RESP_TIMEOUT);
		DHD_ERROR(("%s : Set IOCTL response time for Manufactring Firmware\n",
			__FUNCTION__));
	} else {
		dhd_os_set_ioctl_resp_timeout(IOCTL_RESP_TIMEOUT);
		DHD_INFO(("%s : Set IOCTL response time.\n", __FUNCTION__));
	}
#ifdef GET_CUSTOM_MAC_ENABLE
	ret = wifi_platform_get_mac_addr(dhd->info->adapter, ea_addr.octet);
	if (!ret) {
		ret = dhd_iovar(dhd, 0, "cur_etheraddr", (char *)&ea_addr, ETHER_ADDR_LEN, NULL, 0,
				TRUE);
		if (ret < 0) {
			DHD_ERROR(("%s: can't set MAC address , error=%d\n", __FUNCTION__, ret));
			ret = BCME_NOTUP;
			goto done;
		}
		memcpy(dhd->mac.octet, ea_addr.octet, ETHER_ADDR_LEN);
	} else {
#endif /* GET_CUSTOM_MAC_ENABLE */
		/* Get the default device MAC address directly from firmware */
		ret = dhd_iovar(dhd, 0, "cur_etheraddr", NULL, 0, (char *)&buf, sizeof(buf), FALSE);
		if (ret < 0) {
			DHD_ERROR(("%s: can't get MAC address , error=%d\n", __FUNCTION__, ret));
			ret = BCME_NOTUP;
			goto done;
		}
		/* Update public MAC address after reading from Firmware */
		memcpy(dhd->mac.octet, buf, ETHER_ADDR_LEN);

#ifdef GET_CUSTOM_MAC_ENABLE
	}
#endif /* GET_CUSTOM_MAC_ENABLE */

	/* get a capabilities from firmware */
	{
		uint32 cap_buf_size = sizeof(dhd->wlcore->fw_capabilities);
		memset(dhd->wlcore->fw_capabilities, 0, cap_buf_size);
		ret = dhd_iovar(dhd, 0, "cap", NULL, 0, dhd->wlcore->fw_capabilities,
			(cap_buf_size - 1), FALSE);
		if (ret < 0) {
			DHD_ERROR(("%s: Get Capability failed (error=%d)\n",
				__FUNCTION__, ret));
			return 0;
		}

		memmove(&dhd->wlcore->fw_capabilities[1],
			dhd->wlcore->fw_capabilities, (cap_buf_size - 1));
		dhd->wlcore->fw_capabilities[0] = ' ';
		dhd->wlcore->fw_capabilities[cap_buf_size - 2] = ' ';
		dhd->wlcore->fw_capabilities[cap_buf_size - 1] = '\0';
	}

	if ((!op_mode && dhd_get_fw_mode(dhd->info) == DHD_FLAG_HOSTAP_MODE) ||
		(op_mode == DHD_FLAG_HOSTAP_MODE)) {
#ifdef SET_RANDOM_MAC_SOFTAP
		uint rand_mac;
#endif
		dhd->wlcore->op_mode = DHD_FLAG_HOSTAP_MODE;
#if defined(ARP_OFFLOAD_SUPPORT)
			arpoe = 0;
#endif
#ifdef PKT_FILTER_SUPPORT
			dhd_pkt_filter_enable = FALSE;
#endif
#ifdef SET_RANDOM_MAC_SOFTAP
		SRANDOM32((uint)jiffies);
		rand_mac = RANDOM32();
		iovbuf[0] = 0x02;			   /* locally administered bit */
		iovbuf[1] = 0x1A;
		iovbuf[2] = 0x11;
		iovbuf[3] = (unsigned char)(rand_mac & 0x0F) | 0xF0;
		iovbuf[4] = (unsigned char)(rand_mac >> 8);
		iovbuf[5] = (unsigned char)(rand_mac >> 16);

		ret = dhd_iovar(dhd, 0, "cur_etheraddr", (char *)&iovbuf, ETHER_ADDR_LEN, NULL, 0,
				TRUE);
		if (ret < 0) {
			DHD_ERROR(("%s: can't set MAC address , error=%d\n", __FUNCTION__, ret));
		} else
			memcpy(dhd->mac.octet, iovbuf, ETHER_ADDR_LEN);
#endif /* SET_RANDOM_MAC_SOFTAP */
#if defined(OEM_ANDROID) && !defined(AP) && defined(WL_CFG80211)
		/* Turn off MPC in AP mode */
		ret = dhd_iovar(dhd, 0, "mpc", (char *)&mpc, sizeof(mpc), NULL, 0,
				TRUE);
		if (ret < 0) {
			DHD_ERROR(("%s mpc for HostAPD failed  %d\n", __FUNCTION__, ret));
		}
#endif
#if defined(CUSTOMER_HW4) && defined(USE_DYNAMIC_F2_BLKSIZE)
		dhdsdio_func_blocksize(dhd, 2, DYNAMIC_F2_BLKSIZE_FOR_NONLEGACY);
#endif /* CUSTOMER_HW4 && USE_DYNAMIC_F2_BLKSIZE */
#ifdef SUPPORT_AP_POWERSAVE
		dhd_set_ap_powersave(dhd, 0, TRUE);
#endif /* SUPPORT_AP_POWERSAVE */
#ifdef SOFTAP_UAPSD_OFF
		ret = dhd_iovar(dhd, 0, "wme_apsd", (char *)&wme_apsd, sizeof(wme_apsd), NULL, 0,
				TRUE);
		if (ret < 0) {
			DHD_ERROR(("%s: set wme_apsd 0 fail (error=%d)\n",
				__FUNCTION__, ret));
		}
#endif /* SOFTAP_UAPSD_OFF */
	} else if ((!op_mode && dhd_get_fw_mode(dhd->info) == DHD_FLAG_MFG_MODE) ||
		(op_mode == DHD_FLAG_MFG_MODE)) {
#if defined(ARP_OFFLOAD_SUPPORT)
		arpoe = 0;
#endif /* ARP_OFFLOAD_SUPPORT */
#ifdef PKT_FILTER_SUPPORT
		dhd_pkt_filter_enable = FALSE;
#endif /* PKT_FILTER_SUPPORT */
		dhd->wlcore->op_mode = DHD_FLAG_MFG_MODE;
#if defined(CUSTOMER_HW4) && defined(USE_DYNAMIC_F2_BLKSIZE)
		/* XXX The 'wl counters' command triggers SDIO bus error
		 * if F2 block size is greater than 128 bytes using 4354A1
		 * manufacturing firmware. To avoid this problem, F2 block
		 * size is set to 128 bytes only for DHD_FLAG_MFG_MODE.
		 * There is no problem for other chipset since big data
		 * transcation through SDIO bus is not happened during
		 * manufacturing test.
		 */
		dhdsdio_func_blocksize(dhd, 2, DYNAMIC_F2_BLKSIZE_FOR_NONLEGACY);
#endif /* CUSTOMER_HW4 && USE_DYNAMIC_F2_BLKSIZE */
	} else {
		uint32 concurrent_mode = 0;
		if ((!op_mode && dhd_get_fw_mode(dhd->info) == DHD_FLAG_P2P_MODE) ||
			(op_mode == DHD_FLAG_P2P_MODE)) {
#if defined(ARP_OFFLOAD_SUPPORT)
			arpoe = 0;
#endif
#ifdef PKT_FILTER_SUPPORT
			dhd_pkt_filter_enable = FALSE;
#endif
			dhd->wlcore->op_mode = DHD_FLAG_P2P_MODE;
		} else if ((!op_mode && dhd_get_fw_mode(dhd->info) == DHD_FLAG_IBSS_MODE) ||
			(op_mode == DHD_FLAG_IBSS_MODE)) {
			dhd->wlcore->op_mode = DHD_FLAG_IBSS_MODE;
		} else
			dhd->wlcore->op_mode = DHD_FLAG_STA_MODE;
#if defined(OEM_ANDROID) && !defined(AP) && defined(WLP2P)
		if (dhd->wlcore->op_mode != DHD_FLAG_IBSS_MODE &&
			(concurrent_mode = dhd_get_concurrent_capabilites(dhd))) {
#if defined(ARP_OFFLOAD_SUPPORT)
			arpoe = 1;
#endif
			dhd->wlcore->op_mode |= concurrent_mode;
		}

		/* Check if we are enabling p2p */
		if (dhd->wlcore->op_mode & DHD_FLAG_P2P_MODE) {
			ret = dhd_iovar(dhd, 0, "apsta", (char *)&apsta, sizeof(apsta), NULL, 0,
					TRUE);
			if (ret < 0)
				DHD_ERROR(("%s APSTA for P2P failed ret= %d\n", __FUNCTION__, ret));

#if defined(SOFTAP_AND_GC)
		if ((ret = dhd_wl_ioctl_cmd(dhd, WLC_SET_AP,
			(char *)&ap_mode, sizeof(ap_mode), TRUE, 0)) < 0) {
				DHD_ERROR(("%s WLC_SET_AP failed %d\n", __FUNCTION__, ret));
		}
#endif
			memcpy(&p2p_ea, &dhd->mac, ETHER_ADDR_LEN);
			ETHER_SET_LOCALADDR(&p2p_ea);
			ret = dhd_iovar(dhd, 0, "p2p_da_override", (char *)&p2p_ea, sizeof(p2p_ea),
					NULL, 0, TRUE);
			if (ret < 0) {
				DHD_ERROR(("%s p2p_da_override ret= %d\n", __FUNCTION__, ret));
			} else {
				DHD_INFO(("dhd_preinit_ioctls: p2p_da_override succeeded\n"));
			}
		}
#else
	(void)concurrent_mode;
#endif /* defined(OEM_ANDROID) && !defined(AP) && defined(WLP2P) */
	}

	DHD_ERROR(("Firmware up: op_mode=0x%04x, MAC="MACDBG"\n",
		dhd->wlcore->op_mode, MAC2STRDBG(dhd->mac.octet)));
#if defined(RXFRAME_THREAD) && defined(RXTHREAD_ONLYSTA)
	if (dhd->wlcore->op_mode == DHD_FLAG_HOSTAP_MODE)
		dhd->info->rxthread_enabled = FALSE;
	else
		dhd->info->rxthread_enabled = TRUE;
#endif
	/* Set Country code  */
	if (dhd->dhd_cspec.ccode[0] != 0) {
		ret = dhd_iovar(dhd, 0, "country", (char *)&dhd->dhd_cspec, sizeof(wl_country_t),
			NULL, 0, TRUE);
	if (ret < 0)
			DHD_ERROR(("%s: country code setting failed\n", __FUNCTION__));
	}

#if defined(DISABLE_11AC)
	ret = dhd_iovar(dhd, 0, "vhtmode", (char *)&vhtmode, sizeof(vhtmode), NULL, 0, TRUE);
	if (ret < 0)
		DHD_ERROR(("%s wl vhtmode 0 failed %d\n", __FUNCTION__, ret));
#endif /* DISABLE_11AC */

	/* Set Listen Interval */
	ret = dhd_iovar(dhd, 0, "assoc_listen", (char *)&listen_interval, sizeof(listen_interval),
		NULL, 0, TRUE);
	if (ret < 0)
		DHD_ERROR(("%s assoc_listen failed %d\n", __FUNCTION__, ret));

#if defined(ROAM_ENABLE) || defined(DISABLE_BUILTIN_ROAM)
#if defined(CUSTOMER_HW4) && defined(USE_WFA_CERT_CONF)
	roamvar = sec_get_param(dhd, SET_PARAM_ROAMOFF);
#endif /* CUSTOMER_HW4 && USE_WFA_CERT_CONF */
	/* Disable built-in roaming to allowed ext supplicant to take care of roaming */
	dhd_iovar(dhd, 0, "roam_off", (char *)&roamvar, sizeof(roamvar), NULL, 0, TRUE);
#endif /* ROAM_ENABLE || DISABLE_BUILTIN_ROAM */
#if defined(ROAM_ENABLE)
	if ((ret = dhd_wl_ioctl_cmd(dhd, WLC_SET_ROAM_TRIGGER, roam_trigger,
		sizeof(roam_trigger), TRUE, 0)) < 0)
		DHD_ERROR(("%s: roam trigger set failed %d\n", __FUNCTION__, ret));
	if ((ret = dhd_wl_ioctl_cmd(dhd, WLC_SET_ROAM_SCAN_PERIOD, roam_scan_period,
		sizeof(roam_scan_period), TRUE, 0)) < 0)
		DHD_ERROR(("%s: roam scan period set failed %d\n", __FUNCTION__, ret));
	if ((dhd_wl_ioctl_cmd(dhd, WLC_SET_ROAM_DELTA, roam_delta,
		sizeof(roam_delta), TRUE, 0)) < 0)
		DHD_ERROR(("%s: roam delta set failed %d\n", __FUNCTION__, ret));
	ret = dhd_iovar(dhd, 0, "fullroamperiod", (char *)&roam_fullscan_period,
			sizeof(roam_fullscan_period), NULL, 0, TRUE);
	if (ret < 0)
		DHD_ERROR(("%s: roam fullscan period set failed %d\n", __FUNCTION__, ret));
#ifdef ROAM_AP_ENV_DETECTION
	if (roam_trigger[0] == WL_AUTO_ROAM_TRIGGER) {
		if (dhd_iovar(dhd, 0, "roam_env_detection", (char *)&roam_env_mode,
				sizeof(roam_env_mode), NULL, 0, TRUE) == BCME_OK)
			dhd->roam_env_detection = TRUE;
		else {
			dhd->roam_env_detection = FALSE;
		}
	}
#endif /* ROAM_AP_ENV_DETECTION */
#endif /* ROAM_ENABLE */

#ifdef OKC_SUPPORT
	dhd_iovar(dhd, 0, "ccx_enable", (char *)&ccx, sizeof(ccx), NULL, 0, TRUE);
#endif
#ifdef BCMCCX
	bcm_mkiovar("ccx_enable", (char *)&ccx, 4, iovbuf, sizeof(iovbuf));
	dhd_wl_ioctl_cmd(dhd, WLC_SET_VAR, iovbuf, sizeof(iovbuf), TRUE, 0);
#endif /* BCMCCX */
#ifdef WLTDLS
#ifdef CUSTOMER_HW4
	/* by default TDLS on and auto mode on */
	_dhd_tdls_enable(dhd, true, true, NULL);
#else
	/* by default TDLS on and auto mode off */
	_dhd_tdls_enable(dhd, true, false, NULL);
#endif /* CUSTOMER_HW4 */
#endif /* WLTDLS */

#ifdef DHD_ENABLE_LPC
	/* Set lpc 1 */
	ret = dhd_iovar(dhd, 0, "lpc", (char *)&lpc, sizeof(lpc), NULL, 0, TRUE);
	if (ret < 0) {
		DHD_ERROR(("%s Set lpc failed  %d\n", __FUNCTION__, ret));
#ifdef CUSTOMER_HW4
		if (ret == BCME_NOTDOWN) {
			uint wl_down = 1;
			ret = dhd_wl_ioctl_cmd(dhd, WLC_DOWN,
				(char *)&wl_down, sizeof(wl_down), TRUE, 0);
			DHD_ERROR(("%s lpc fail WL_DOWN : %d, lpc = %d\n", __FUNCTION__, ret, lpc));

			ret = dhd_iovar(dhd, 0, "lpc", (char *)&lpc, sizeof(lpc), NULL, 0, TRUE);
			DHD_ERROR(("%s Set lpc ret --> %d\n", __FUNCTION__, ret));
		}
#endif /* CUSTOMER_HW4 */
	}
#endif /* DHD_ENABLE_LPC */

#if defined(CUSTOMER_HW4) && defined(CONFIG_CONTROL_PM)
	sec_control_pm(dhd, &power_mode);
#else
	/* Set PowerSave mode */
	dhd_wl_ioctl_cmd(dhd, WLC_SET_PM, (char *)&power_mode, sizeof(power_mode), TRUE, 0);
#endif /* CUSTOMER_HW4 && CONFIG_CONTROL_PM */

#if defined(STB) && !defined(STBAP)
#if defined(BCMDHDUSB) && defined(BCMDBUS)
	dhd_clear_wowl(dhd);
#endif
#endif /* STB && STBAP */

#if defined(OEM_ANDROID)
	/* Turn off MPC */
	ret = dhd_iovar(dhd, 0, "mpc", (char *)&mpc, sizeof(mpc), NULL, 0,
			TRUE);
	if (ret < 0) {
		DHD_ERROR(("%s set mpc failed  %d\n", __FUNCTION__, ret));
	}
#endif /* OEM_ANDROID */

	/* Setup timeout if Beacons are lost and roam is off to report link down */
	dhd_iovar(dhd, 0, "bcn_timeout", (char *)&bcn_timeout, sizeof(bcn_timeout), NULL, 0, TRUE);
	/* Setup assoc_retry_max count to reconnect target AP in dongle */
	dhd_iovar(dhd, 0, "assoc_retry_max", (char *)&retry_max, sizeof(retry_max), NULL, 0, TRUE);
#if defined(AP) && !defined(WLP2P)
	/* Turn off MPC in AP mode */
	dhd_iovar(dhd, 0, "mpc", (char *)&mpc, sizeof(mpc), NULL, 0, TRUE);
	dhd_iovar(dhd, 0, "apsta", (char *)&apsta, sizeof(apsta), NULL, 0, TRUE);
#endif /* defined(AP) && !defined(WLP2P) */

#if defined(CUSTOMER_HW4) && defined(MIMO_ANT_SETTING)
	dhd_sel_ant_from_file(dhd);
#endif /* defined(CUSTOMER_HW4) && defined(MIMO_ANT_SETTING) */

#if defined(OEM_ANDROID) && defined(SOFTAP)
	if (ap_fw_loaded == TRUE) {
		dhd_wl_ioctl_cmd(dhd, WLC_SET_DTIMPRD, (char *)&dtim, sizeof(dtim), TRUE, 0);
	}
#endif /* defined(OEM_ANDROID) && defined(SOFTAP) */

#if defined(KEEP_ALIVE)
	{
	/* Set Keep Alive : be sure to use FW with -keepalive */
	int res;

#if defined(OEM_ANDROID) && defined(SOFTAP)
	if (ap_fw_loaded == FALSE)
#endif /* defined(OEM_ANDROID) && defined(SOFTAP) */
		if (!(dhd->wlcore->op_mode &
			(DHD_FLAG_HOSTAP_MODE | DHD_FLAG_MFG_MODE))) {
			if ((res = dhd_keep_alive_onoff(dhd)) < 0)
				DHD_ERROR(("%s set keeplive failed %d\n",
				__FUNCTION__, res));
		}
	}
#endif /* defined(KEEP_ALIVE) */

#else
	/* get a capabilities from firmware */
	memset(dhd->wlcore->fw_capabilities, 0, sizeof(dhd->wlcore->fw_capabilities));
	ret = dhd_iovar(dhd, 0, "cap", NULL, 0, dhd->wlcore->fw_capabilities,
		sizeof(dhd->wlcore->fw_capabilities), FALSE);
	if (ret < 0) {
		DHD_ERROR(("%s: Get Capability failed (error=%d)\n",
			__FUNCTION__, ret));
		goto done;
	}
#endif  /* OEM_ANDROID */

#ifdef USE_WL_TXBF
	ret = dhd_iovar(dhd, 0, "txbf", (char *)&txbf, sizeof(txbf), NULL,
			0, TRUE);
	if (ret < 0) {
		DHD_ERROR(("%s Set txbf failed  %d\n", __FUNCTION__, ret));
	}
#endif /* USE_WL_TXBF */
#if defined(CUSTOMER_HW4) && defined(USE_WFA_CERT_CONF)
	frameburst = sec_get_param(dhd, SET_PARAM_FRAMEBURST);
#ifdef DISABLE_FRAMEBURST_VSDB
	g_frameburst = frameburst;
#endif /* DISABLE_FRAMEBURST_VSDB */
#endif /* CUSTOMER_HW4 && USE_WFA_CERT_CONF */
#ifdef DISABLE_WL_FRAMEBURST_SOFTAP
	/* Disable Framebursting for SofAP */
	if (dhd->wlcore->op_mode & DHD_FLAG_HOSTAP_MODE) {
		frameburst = 0;
	}
#endif /* DISABLE_WL_FRAMEBURST_SOFTAP */
	/* Set frameburst to value */
	if ((ret = dhd_wl_ioctl_cmd(dhd, WLC_SET_FAKEFRAG, (char *)&frameburst,
		sizeof(frameburst), TRUE, 0)) < 0) {
		DHD_INFO(("%s frameburst not supported  %d\n", __FUNCTION__, ret));
	}
#ifdef DHD_SET_FW_HIGHSPEED
	/* Set ack_ratio */
	ret = dhd_iovar(dhd, 0, "ack_ratio", (char *)&ack_ratio, sizeof(ack_ratio), NULL, 0, TRUE);
	if (ret < 0) {
		DHD_ERROR(("%s Set ack_ratio failed  %d\n", __FUNCTION__, ret));
	}

	/* Set ack_ratio_depth */
	ret = dhd_iovar(dhd, 0, "ack_ratio_depth", (char *)&ack_ratio_depth,
			sizeof(ack_ratio_depth), NULL, 0, TRUE);
	if (ret < 0) {
		DHD_ERROR(("%s Set ack_ratio_depth failed  %d\n", __FUNCTION__, ret));
	}
#endif /* DHD_SET_FW_HIGHSPEED */

#ifdef CUSTOM_AMPDU_BA_WSIZE
	ampdu_ba_wsize = CUSTOM_AMPDU_BA_WSIZE;

	if (ampdu_ba_wsize != 0) {
		ret = dhd_iovar(dhd, 0, "ampdu_ba_wsize", (char *)&ampdu_ba_wsize,
				sizeof(ampdu_ba_wsize), NULL, 0, TRUE);
		if (ret < 0) {
			DHD_ERROR(("%s Set ampdu_ba_wsize to %d failed  %d\n",
				__FUNCTION__, ampdu_ba_wsize, ret));
		}
	}
#endif /* CUSTOM_AMPDU_BA_WSIZE */

	iov_buf = (char*)kmalloc(WLC_IOCTL_SMLEN, KMALLOC_FLAG);
	if (iov_buf == NULL) {
		DHD_ERROR(("failed to allocate %d bytes for iov_buf\n", WLC_IOCTL_SMLEN));
		ret = BCME_NOMEM;
		goto done;
	}

#ifdef WLAIBSS
	/* Configure custom IBSS beacon transmission */
	if (dhd->op_mode & DHD_FLAG_IBSS_MODE)
	{
		aibss = 1;
		ret = dhd_iovar(dhd, 0, "aibss", (char *)&aibss,
				sizeof(aibss), NULL, 0, TRUE);
		if (ret < 0) {
			DHD_ERROR(("%s Set aibss to %d failed  %d\n",
				__FUNCTION__, aibss, ret));
		}
#ifdef WLAIBSS_PS
		aibss_ps = 1;
		ret = dhd_iovar(dhd, 0, "aibss_ps", (char *)&aibss_ps,
				sizeof(aibss_ps), NULL, 0, TRUE);
		if (ret < 0) {
			DHD_ERROR(("%s Set aibss PS to %d failed  %d\n",
				__FUNCTION__, aibss, ret));
		}
#endif /* WLAIBSS_PS */
	}
	memset(&bcn_config, 0, sizeof(bcn_config));
	bcn_config.initial_min_bcn_dur = AIBSS_INITIAL_MIN_BCN_DUR;
	bcn_config.min_bcn_dur = AIBSS_MIN_BCN_DUR;
	bcn_config.bcn_flood_dur = AIBSS_BCN_FLOOD_DUR;
	bcn_config.version = AIBSS_BCN_FORCE_CONFIG_VER_0;
	bcn_config.len = sizeof(bcn_config);

	ret = dhd_iovar(dhd, 0, "aibss_bcn_force_config", (char *)&bcn_config,
		sizeof(aibss_bcn_force_config_t), NULL, 0, TRUE);
	if (ret < 0) {
		DHD_ERROR(("%s Set aibss_bcn_force_config to %d, %d, %d failed %d\n",
			__FUNCTION__, AIBSS_INITIAL_MIN_BCN_DUR, AIBSS_MIN_BCN_DUR,
			AIBSS_BCN_FLOOD_DUR, ret));
	}
#endif /* WLAIBSS */

#if defined(CUSTOM_AMPDU_MPDU)
	ampdu_mpdu = CUSTOM_AMPDU_MPDU;
	if (ampdu_mpdu != 0 && (ampdu_mpdu <= ampdu_ba_wsize)) {
		ret = dhd_iovar(dhd, 0, "ampdu_mpdu", (char *)&ampdu_mpdu, sizeof(ampdu_mpdu),
				NULL, 0, TRUE);
		if (ret < 0)
			DHD_ERROR(("%s Set ampdu_mpdu to %d failed  %d\n",
				__FUNCTION__, CUSTOM_AMPDU_MPDU, ret));
	}
#endif /* CUSTOM_AMPDU_MPDU */

#if defined(CUSTOM_AMPDU_RELEASE)
	ampdu_release = CUSTOM_AMPDU_RELEASE;
	if (ampdu_release != 0 && (ampdu_release <= ampdu_ba_wsize)) {
		ret = dhd_iovar(dhd, 0, "ampdu_release", (char *)&ampdu_release,
				sizeof(ampdu_release), NULL, 0, TRUE);
		if (ret < 0)
			DHD_ERROR(("%s Set ampdu_release to %d failed  %d\n",
				__FUNCTION__, CUSTOM_AMPDU_RELEASE, ret));
	}
#endif /* CUSTOM_AMPDU_RELEASE */

#if defined(CUSTOM_AMSDU_AGGSF)
	amsdu_aggsf = CUSTOM_AMSDU_AGGSF;
	if (amsdu_aggsf != 0) {
		ret = dhd_iovar(dhd, 0, "amsdu_aggsf", (char *)&amsdu_aggsf, sizeof(amsdu_aggsf),
				NULL, 0, TRUE);
		if (ret < 0) {
			DHD_ERROR(("%s Set amsdu_aggsf to %d failed %d\n",
				__FUNCTION__, CUSTOM_AMSDU_AGGSF, ret));
		}
	}
#endif /* CUSTOM_AMSDU_AGGSF */

#if defined(BCMSUP_4WAY_HANDSHAKE) && defined(WLAN_AKM_SUITE_FT_8021X)
	/* Read 4-way handshake requirements */
	if (dhd_use_idsup == 1) {
		ret = dhd_iovar(dhd, 0, "sup_wpa", (char *)&sup_wpa, sizeof(sup_wpa),
				(char *)&iovbuf, sizeof(iovbuf), FALSE);
		/* sup_wpa iovar returns NOTREADY status on some platforms using modularized
		 * in-dongle supplicant.
		 */
		if (ret >= 0 || ret == BCME_NOTREADY)
			dhd->fw_4way_handshake = TRUE;
		DHD_TRACE(("4-way handshake mode is: %d\n", dhd->fw_4way_handshake));
	}
#endif /* BCMSUP_4WAY_HANDSHAKE && WLAN_AKM_SUITE_FT_8021X */
#ifdef SUPPORT_2G_VHT
	ret = dhd_iovar(dhd, 0, "vht_features", (char *)&vht_features, sizeof(vht_features),
		NULL, 0, TRUE);
	if (ret < 0) {
		DHD_ERROR(("%s vht_features set failed %d\n", __FUNCTION__, ret));
#ifdef CUSTOMER_HW4
		if (ret == BCME_NOTDOWN) {
			uint wl_down = 1;
			ret = dhd_wl_ioctl_cmd(dhd, WLC_DOWN,
				(char *)&wl_down, sizeof(wl_down), TRUE, 0);
			DHD_ERROR(("%s vht_features fail WL_DOWN : %d, vht_features = 0x%x\n",
				__FUNCTION__, ret, vht_features));

			ret = dhd_iovar(dhd, 0, "vht_features", (char *)&vht_features,
				sizeof(vht_features), NULL, 0, TRUE);
			DHD_ERROR(("%s vht_features set. ret --> %d\n", __FUNCTION__, ret));
		}
#endif /* CUSTOMER_HW4 */
	}
#endif /* SUPPORT_2G_VHT */
#ifdef CUSTOM_PSPRETEND_THR
	/* Turn off MPC in AP mode */
	ret = dhd_iovar(dhd, 0, "pspretend_threshold", (char *)&pspretend_thr,
			sizeof(pspretend_thr), NULL, 0, TRUE);
	if (ret < 0) {
		DHD_ERROR(("%s pspretend_threshold for HostAPD failed  %d\n",
			__FUNCTION__, ret));
	}
#endif

	ret = dhd_iovar(dhd, 0, "buf_key_b4_m4", (char *)&buf_key_b4_m4, sizeof(buf_key_b4_m4),
			NULL, 0, TRUE);
	if (ret < 0) {
		DHD_ERROR(("%s buf_key_b4_m4 set failed %d\n", __FUNCTION__, ret));
	}

	/* Read event_msgs mask */
	ret = dhd_iovar(dhd, 0, "event_msgs", eventmask, WL_EVENTING_MASK_LEN, iovbuf,
			sizeof(iovbuf), FALSE);
	if (ret < 0) {
		DHD_ERROR(("%s read Event mask failed %d\n", __FUNCTION__, ret));
		goto done;
	}
	bcopy(iovbuf, eventmask, WL_EVENTING_MASK_LEN);

	/* Setup event_msgs */
	setbit(eventmask, WLC_E_SET_SSID);
	setbit(eventmask, WLC_E_PRUNE);
	setbit(eventmask, WLC_E_AUTH);
	setbit(eventmask, WLC_E_AUTH_IND);
	setbit(eventmask, WLC_E_ASSOC);
	setbit(eventmask, WLC_E_REASSOC);
	setbit(eventmask, WLC_E_REASSOC_IND);
	if (!(dhd->wlcore->op_mode & DHD_FLAG_IBSS_MODE))
		setbit(eventmask, WLC_E_DEAUTH);
	setbit(eventmask, WLC_E_DEAUTH_IND);
	setbit(eventmask, WLC_E_DISASSOC_IND);
	setbit(eventmask, WLC_E_DISASSOC);
	setbit(eventmask, WLC_E_JOIN);
	setbit(eventmask, WLC_E_START);
	setbit(eventmask, WLC_E_ASSOC_IND);
	setbit(eventmask, WLC_E_PSK_SUP);
	setbit(eventmask, WLC_E_LINK);
	setbit(eventmask, WLC_E_MIC_ERROR);
	setbit(eventmask, WLC_E_ASSOC_REQ_IE);
	setbit(eventmask, WLC_E_ASSOC_RESP_IE);
#ifndef WL_CFG80211
	setbit(eventmask, WLC_E_PMKID_CACHE);
	setbit(eventmask, WLC_E_TXFAIL);
#endif
	setbit(eventmask, WLC_E_JOIN_START);
	setbit(eventmask, WLC_E_SCAN_COMPLETE);
#ifdef DHD_DEBUG
	setbit(eventmask, WLC_E_SCAN_CONFIRM_IND);
#endif
#ifdef WLMEDIA_HTSF
	setbit(eventmask, WLC_E_HTSFSYNC);
#endif /* WLMEDIA_HTSF */
#ifdef PNO_SUPPORT
	setbit(eventmask, WLC_E_PFN_NET_FOUND);
	setbit(eventmask, WLC_E_PFN_BEST_BATCHING);
	setbit(eventmask, WLC_E_PFN_BSSID_NET_FOUND);
	setbit(eventmask, WLC_E_PFN_BSSID_NET_LOST);
#endif /* PNO_SUPPORT */
	/* enable dongle roaming event */
#if defined(OEM_ANDROID)
	setbit(eventmask, WLC_E_ROAM);
	setbit(eventmask, WLC_E_BSSID);
#endif
#ifdef BCMCCX
	setbit(eventmask, WLC_E_ADDTS_IND);
	setbit(eventmask, WLC_E_DELTS_IND);
#endif /* BCMCCX */
#ifdef WLTDLS
	setbit(eventmask, WLC_E_TDLS_PEER_EVENT);
#endif /* WLTDLS */
#if defined(WL_CFG80211) && defined(WL_DPP)
	setbit(eventmask, WLC_E_ACTION_FRAME_RX);
#endif
#ifdef WL_CFG80211
	setbit(eventmask, WLC_E_ESCAN_RESULT);
	if (dhd->wlcore->op_mode & DHD_FLAG_P2P_MODE) {
		setbit(eventmask, WLC_E_ACTION_FRAME_RX);
		setbit(eventmask, WLC_E_P2P_DISC_LISTEN_COMPLETE);
	}
#if defined(CUSTOMER_HW4) && defined(WES_SUPPORT)
	else {
		setbit(eventmask, WLC_E_ACTION_FRAME_RX);
	}
#endif /* WES_SUPPORT */
#endif /* WL_CFG80211 */
#ifdef CUSTOMER_HW10
	clrbit(eventmask, WLC_E_TRACE);
#else
	setbit(eventmask, WLC_E_TRACE);
#endif
	setbit(eventmask, WLC_E_CSA_COMPLETE_IND);
#ifdef DHD_WMF
	setbit(eventmask, WLC_E_PSTA_PRIMARY_INTF_IND);
#endif

	/* Write updated Event mask */
	ret = dhd_iovar(dhd, 0, "event_msgs", eventmask, WL_EVENTING_MASK_LEN, NULL, 0, TRUE);
	if (ret < 0) {
		DHD_ERROR(("%s Set Event mask failed %d\n", __FUNCTION__, ret));
		goto done;
	}

	/* make up event mask ext message iovar for event larger than 128 */
	msglen = ROUNDUP(WLC_E_LAST, NBBY)/NBBY + EVENTMSGS_EXT_STRUCT_SIZE;
	eventmask_msg = (eventmsgs_ext_t*)kmalloc(msglen, KMALLOC_FLAG);
	if (eventmask_msg == NULL) {
		DHD_ERROR(("failed to allocate %d bytes for event_msg_ext\n", msglen));
		ret = BCME_NOMEM;
		goto done;
	}
	bzero(eventmask_msg, msglen);
	eventmask_msg->ver = EVENTMSGS_VER;
	eventmask_msg->len = ROUNDUP(WLC_E_LAST, NBBY)/NBBY;

	/* Read event_msgs_ext mask */
	ret2 = dhd_iovar(dhd, 0, "event_msgs_ext", (char *)eventmask_msg, msglen, iov_buf,
			WLC_IOCTL_SMLEN, FALSE);
	if (ret2 != BCME_UNSUPPORTED)
		ret = ret2;
	if (ret2 == 0) { /* event_msgs_ext must be supported */
		bcopy(iov_buf, eventmask_msg, msglen);
#ifdef GSCAN_SUPPORT
		setbit(eventmask_msg->mask, WLC_E_PFN_GSCAN_FULL_RESULT);
		setbit(eventmask_msg->mask, WLC_E_PFN_SCAN_COMPLETE);
#endif /* GSCAN_SUPPORT */
#ifdef BT_WIFI_HANDOVER
		setbit(eventmask_msg->mask, WLC_E_BT_WIFI_HANDOVER_REQ);
#endif /* BT_WIFI_HANDOVER */

		setbit(eventmask_msg->mask, WLC_E_DPSTA_INTF_IND);

		setbit(eventmask_msg->mask, WLC_E_MACDBG);
#ifdef BCMHWA
		setbit(eventmask_msg->mask, WLC_E_HWA_EVENT);
#endif
#ifdef WL_SAE
		setbit(eventmask_msg->mask, WLC_E_START_AUTH);
#endif /* WL_SAE */
		setbit(eventmask_msg->mask, WLC_E_FBT);
		setbit(eventmask_msg->mask, WLC_E_PWR_SAVE_SYNC);
#ifdef WL_CFG80211
		setbit(eventmask_msg->mask, WLC_E_BAND_CHANGE);
#endif /* WL_CFG80211 */

		setbit(eventmask_msg->mask, WLC_E_CAC_STATE_CHANGE);

		/* Write updated Event mask */
		eventmask_msg->ver = EVENTMSGS_VER;
		eventmask_msg->command = EVENTMSGS_SET_MASK;
		eventmask_msg->len = ROUNDUP(WLC_E_LAST, NBBY)/NBBY;
		ret = dhd_iovar(dhd, 0, "event_msgs_ext", (char *)eventmask_msg, msglen, NULL, 0,
				TRUE);
		if (ret < 0) {
			DHD_ERROR(("%s write event mask ext failed %d\n", __FUNCTION__, ret));
			goto done;
		}
	} else if (ret2 < 0 && ret2 != BCME_UNSUPPORTED) {
		DHD_ERROR(("%s read event mask ext failed %d\n", __FUNCTION__, ret2));
		goto done;
	} /* unsupported is ok */
#ifdef OEM_ANDROID
	dhd_wl_ioctl_cmd(dhd, WLC_SET_SCAN_CHANNEL_TIME, (char *)&scan_assoc_time,
		sizeof(scan_assoc_time), TRUE, 0);
	dhd_wl_ioctl_cmd(dhd, WLC_SET_SCAN_UNASSOC_TIME, (char *)&scan_unassoc_time,
		sizeof(scan_unassoc_time), TRUE, 0);
	dhd_wl_ioctl_cmd(dhd, WLC_SET_SCAN_PASSIVE_TIME, (char *)&scan_passive_time,
		sizeof(scan_passive_time), TRUE, 0);

#ifdef ARP_OFFLOAD_SUPPORT
	/* Set and enable ARP offload feature for STA only  */
#if defined(OEM_ANDROID) && defined(SOFTAP)
	if (arpoe && !ap_fw_loaded) {
#else
	if (arpoe) {
#endif /* defined(OEM_ANDROID) && defined(SOFTAP) */
		dhd_arp_offload_enable(dhd, TRUE);
		dhd_arp_offload_set(dhd, dhd_arp_mode);
	} else {
		/*
		 * XXX andrey in any falcon f/w, AOE feature is always present by default
		 *	so we need to explicitly disable it for SOFTAP, see PR#97072
		*/
		dhd_arp_offload_enable(dhd, FALSE);
		dhd_arp_offload_set(dhd, 0);
	}
	dhd_arp_enable = arpoe;
#endif /* ARP_OFFLOAD_SUPPORT */

#ifdef PKT_FILTER_SUPPORT
	/* Setup default defintions for pktfilter , enable in suspend */
	dhd->wlcore->pktfilter_count = 6;
	/* Setup filter to allow only unicast */
	dhd->wlcore->pktfilter[DHD_UNICAST_FILTER_NUM] = "100 0 0 0 0x01 0x00";
	dhd->wlcore->pktfilter[DHD_BROADCAST_FILTER_NUM] = NULL;
	dhd->wlcore->pktfilter[DHD_MULTICAST4_FILTER_NUM] = NULL;
	dhd->wlcore->pktfilter[DHD_MULTICAST6_FILTER_NUM] = NULL;
	/* Add filter to pass multicastDNS packet and NOT filter out as Broadcast */
	dhd->wlcore->pktfilter[DHD_MDNS_FILTER_NUM] = "104 0 0 0 0xFFFFFFFFFFFF 0x01005E0000FB";
	/* apply APP pktfilter */
	dhd->wlcore->pktfilter[WL_ARP_FILTER_NUM] = "105 0 0 12 0xFFFF 0x0806";

#ifdef CUSTOMER_HW4
#ifdef GAN_LITE_NAT_KEEPALIVE_FILTER
	dhd->wlcore->pktfilter_count = 4;
	/* Setup filter to block broadcast and NAT Keepalive packets */
	/* discard all broadcast packets */
	dhd->wlcore->pktfilter[DHD_UNICAST_FILTER_NUM] = "100 0 0 0 0xffffff 0xffffff";
	/* discard NAT Keepalive packets */
	dhd->wlcore->pktfilter[DHD_BROADCAST_FILTER_NUM] = "102 0 0 36 0xffffffff 0x11940009";
	/* discard NAT Keepalive packets */
	dhd->wlcore->pktfilter[DHD_MULTICAST4_FILTER_NUM] = "104 0 0 38 0xffffffff 0x11940009";
	dhd->wlcore->pktfilter[DHD_MULTICAST6_FILTER_NUM] = NULL;
#else
#ifdef BLOCK_IPV6_PACKET
	/* Setup filter to allow only IPv4 unicast frames */
	dhd->wlcore->pktfilter[DHD_UNICAST_FILTER_NUM] = "100 0 0 0 "
		HEX_PREF_STR UNI_FILTER_STR ZERO_ADDR_STR ETHER_TYPE_STR IPV6_FILTER_STR
		" "
		HEX_PREF_STR ZERO_ADDR_STR ZERO_ADDR_STR ETHER_TYPE_STR ZERO_TYPE_STR;
#endif /* BLOCK_IPV6_PACKET */
#ifdef PASS_IPV4_SUSPEND
	dhd->wlcore->pktfilter[DHD_MDNS_FILTER_NUM] = "104 0 0 0 0xFFFFFF 0x01005E";
#endif /* PASS_IPV4_SUSPEND */
#endif /* GAN_LITE_NAT_KEEPALIVE_FILTER */
#endif /* CUSTOMER_HW4 */

#if defined(SOFTAP)
	if (ap_fw_loaded) {
		dhd_enable_packet_filter(0, dhd);
	}
#endif /* defined(SOFTAP) */
	dhd_set_packet_filter(dhd);
#endif /* PKT_FILTER_SUPPORT */
#ifdef DISABLE_11N
	ret = dhd_iovar(dhd, 0, "nmode", (char *)&nmode, sizeof(nmode), NULL, 0, TRUE);
	if (ret < 0)
		DHD_ERROR(("%s wl nmode 0 failed %d\n", __FUNCTION__, ret));
#endif /* DISABLE_11N */

#if defined(CUSTOMER_HW4) && defined(ENABLE_BCN_LI_BCN_WAKEUP)
	dhd_iovar(dhd, 0, "bcn_li_bcn", (char *)&bcn_li_bcn, sizeof(bcn_li_bcn), NULL, 0, TRUE);
#endif /* CUSTOMER_HW4 && ENABLE_BCN_LI_BCN_WAKEUP */
#ifdef AMPDU_VO_ENABLE
	tid.tid = PRIO_8021D_VO; /* Enable TID(6) for voice */
	tid.enable = TRUE;
	dhd_iovar(dhd, 0, "ampdu_tid", (char *)&tid, sizeof(tid), NULL, 0, TRUE);
	tid.tid = PRIO_8021D_NC; /* Enable TID(7) for voice */
	tid.enable = TRUE;
	dhd_iovar(dhd, 0, "ampdu_tid", (char *)&tid, sizeof(tid), NULL, 0, TRUE);
#endif
#if defined(SOFTAP_TPUT_ENHANCE)
	if (dhd->wlcore->op_mode & DHD_FLAG_HOSTAP_MODE) {
		dhd_bus_setidletime(dhd, (int)100);
#ifdef DHDTCPACK_SUPPRESS
		dhd->tcpack_sup_enabled = FALSE;
#endif
#if defined(DHD_TCP_WINSIZE_ADJUST)
		dhd_use_tcp_window_size_adjust = TRUE;
#endif

		memset(buf, 0, sizeof(buf));
		ret = dhd_iovar(dhd, 0, "bus:txglom_auto_control", NULL, 0, buf, sizeof(buf),
				FALSE);
		if (ret < 0) {
			glom = 0;
			dhd_iovar(dhd, 0, "bus:txglom", (char *)&glom, sizeof(glom), NULL, 0, TRUE);
		} else {
			if (buf[0] == 0) {
				glom = 1;
				dhd_iovar(dhd, 0, "bus:txglom_auto_control", (char *)&glom,
						sizeof(glom), NULL, 0, TRUE);
			}
		}
	}
#endif /* SOFTAP_TPUT_ENHANCE */
	/* query for 'ver' to get version info from firmware */
	memset(buf, 0, sizeof(buf));
	ptr = buf;
	ret = dhd_iovar(dhd, 0, "ver", NULL, 0, (char *)&buf, sizeof(buf), FALSE);
	if (ret < 0)
		DHD_ERROR(("%s failed %d\n", __FUNCTION__, ret));
	else {
		bcmstrtok(&ptr, "\n", 0);
		/* Print fw version info */
		DHD_ERROR(("Firmware version = %s\n", buf));
#if defined(CUSTOMER_HW4) && defined(WRITE_WLANINFO)
		sec_save_wlinfo(buf, EPI_VERSION_STR, dhd->info->nv_path);
#endif /* CUSTOMER_HW4 && WRITE_WLANINFO */
	}
#endif /* defined(OEM_ANDROID) */

#if defined(BCMDBUS)
#ifdef PROP_TXSTATUS
	if (disable_proptx ||
#ifdef PROP_TXSTATUS_VSDB
		/* enable WLFC only if the firmware is VSDB when it is in STA mode */
		(dhd->wlcore->op_mode != DHD_FLAG_HOSTAP_MODE &&
		 dhd->wlcore->op_mode != DHD_FLAG_IBSS_MODE) ||
#endif /* PROP_TXSTATUS_VSDB */
		FALSE) {
		wlfc_enable = FALSE;
	}

#ifndef DISABLE_11N
	ret2 = dhd_iovar(dhd, 0, "ampdu_hostreorder", (char *)&hostreorder, sizeof(hostreorder),
			NULL, 0, TRUE);
	if (ret2 < 0) {
		DHD_ERROR(("%s wl ampdu_hostreorder failed %d\n", __FUNCTION__, ret2));
		if (ret2 != BCME_UNSUPPORTED)
			ret = ret2;
#ifdef CUSTOMER_HW4
		if (ret == BCME_NOTDOWN) {
			uint wl_down = 1;
			ret2 = dhd_wl_ioctl_cmd(dhd, WLC_DOWN, (char *)&wl_down,
				sizeof(wl_down), TRUE, 0);
			DHD_ERROR(("%s ampdu_hostreorder fail WL_DOWN : %d, hostreorder :%d\n",
				__FUNCTION__, ret2, hostreorder));

			ret2 = dhd_iovar(dhd, 0, "ampdu_hostreorder", (char *)&hostreorder,
					sizeof(hostreorder), NULL, 0, TRUE);
			DHD_ERROR(("%s wl ampdu_hostreorder. ret --> %d\n", __FUNCTION__, ret2));
			if (ret2 != BCME_UNSUPPORTED)
					ret = ret2;
		}
#endif /* CUSTOMER_HW4 */
		if (ret2 != BCME_OK)
			hostreorder = 0;
	}
#endif /* DISABLE_11N */

#ifdef READ_CONFIG_FROM_FILE
	dhd_preinit_config(dhd, 0);
#endif /* READ_CONFIG_FROM_FILE */

	if (wlfc_enable)
		dhd_wlfc_init(dhd);
#ifndef DISABLE_11N
	else if (hostreorder)
		dhd_wlfc_hostreorder_init(dhd);
#endif /* DISABLE_11N */

#endif /* PROP_TXSTATUS */
#endif
#ifdef PCIE_FULL_DONGLE
	/* For FD we need all the packets at DHD to handle intra-BSS forwarding */
	if (FW_SUPPORTED(dhd->wlcore, ap)) {
		wl_ap_isolate = AP_ISOLATE_SENDUP_ALL;
		ret = dhd_iovar(dhd, 0, "ap_isolate", (char *)&wl_ap_isolate, sizeof(wl_ap_isolate),
				NULL, 0, TRUE);
		if (ret < 0)
			DHD_ERROR(("%s failed %d\n", __FUNCTION__, ret));
	}
#endif /* PCIE_FULL_DONGLE */
#ifdef PNO_SUPPORT
	if (!dhd->pno_state) {
		dhd_pno_init(dhd);
	}
#endif
#ifdef WL11U
	dhd_interworking_enable(dhd);
#endif /* WL11U */

done:

	if (eventmask_msg)
		kfree(eventmask_msg);
	if (iov_buf)
		kfree(iov_buf);

	return ret;
}

int
dhd_iovar(dhd_pub_t *pub, int ifidx, char *name, char *param_buf, uint param_len, char *res_buf,
		uint res_len, int set)
{
	char *buf = NULL;
	int input_len;
	wl_ioctl_t ioc;
	int ret;

	if (res_len > WLC_IOCTL_MAXLEN || param_len > WLC_IOCTL_MAXLEN)
		return BCME_BADARG;

	input_len = strlen(name) + 1 + param_len;
	if (input_len > WLC_IOCTL_MAXLEN)
		return BCME_BADARG;

	buf = NULL;
	if (set) {
		if (res_buf || res_len != 0) {
			DHD_ERROR(("%s: SET wrong arguemnet\n", __FUNCTION__));
			ret = BCME_BADARG;
			goto exit;
		}
		buf = kzalloc(input_len, KMALLOC_FLAG);
		if (!buf) {
			DHD_ERROR(("%s: mem alloc failed\n", __FUNCTION__));
			ret = BCME_NOMEM;
			goto exit;
		}
		ret = bcm_mkiovar(name, param_buf, param_len, buf, input_len);
		if (!ret) {
			ret = BCME_NOMEM;
			goto exit;
		}

		ioc.cmd = WLC_SET_VAR;
		ioc.buf = buf;
		ioc.len = input_len;
		ioc.set = set;

		ret = dhd_wl_ioctl(pub, ifidx, &ioc, ioc.buf, ioc.len);
	} else {
		if (!res_buf || !res_len) {
			DHD_ERROR(("%s: GET failed. resp_buf NULL or length 0.\n", __FUNCTION__));
			ret = BCME_BADARG;
			goto exit;
		}

		if (res_len < input_len) {
			DHD_INFO(("%s: res_len(%d) < input_len(%d)\n", __FUNCTION__,
					res_len, input_len));
			buf = kzalloc(input_len, KMALLOC_FLAG);
			if (!buf) {
				DHD_ERROR(("%s: mem alloc failed\n", __FUNCTION__));
				ret = BCME_NOMEM;
				goto exit;
			}
			ret = bcm_mkiovar(name, param_buf, param_len, buf, input_len);
			if (!ret) {
				ret = BCME_NOMEM;
				goto exit;
			}

			ioc.cmd = WLC_GET_VAR;
			ioc.buf = buf;
			ioc.len = input_len;
			ioc.set = set;

			ret = dhd_wl_ioctl(pub, ifidx, &ioc, ioc.buf, ioc.len);

			if (ret == BCME_OK) {
				memcpy(res_buf, buf, res_len);
			}
		} else {
			memset(res_buf, 0, res_len);
			ret = bcm_mkiovar(name, param_buf, param_len, res_buf, res_len);
			if (!ret) {
				ret = BCME_NOMEM;
				goto exit;
			}

			ioc.cmd = WLC_GET_VAR;
			ioc.buf = res_buf;
			ioc.len = res_len;
			ioc.set = set;

			ret = dhd_wl_ioctl(pub, ifidx, &ioc, ioc.buf, ioc.len);
		}
	}

exit:
	kfree(buf);

	return ret;
}

int
dhd_change_mtu(dhd_pub_t *dhdp, int new_mtu, int ifidx)
{
	struct dhd_info *dhd = dhdp->info;
	struct net_device *dev = NULL;

	ASSERT(dhd && dhd->iflist[ifidx]);
	dev = dhd->iflist[ifidx]->net;
	ASSERT(dev);

	if (netif_running(dev)) {
		DHD_ERROR(("%s: Must be down to change its MTU", dev->name));
		return BCME_NOTDOWN;
	}

#define DHD_MIN_MTU 1500
#define DHD_MAX_MTU 1752

	if ((new_mtu < DHD_MIN_MTU) || (new_mtu > DHD_MAX_MTU)) {
		DHD_ERROR(("%s: MTU size %d is invalid.\n", __FUNCTION__, new_mtu));
		return BCME_BADARG;
	}

	dev->mtu = new_mtu;

	return 0;
}

#ifdef ARP_OFFLOAD_SUPPORT
/* add or remove AOE host ip(s) (up to 8 IPs on the interface)  */
void
aoe_update_host_ipv4_table(dhd_pub_t *dhd_pub, u32 ipa, bool add, int idx)
{
	u32 ipv4_buf[MAX_IPV4_ENTRIES]; /* temp save for AOE host_ip table */
	int i;
	int ret;

	bzero(ipv4_buf, sizeof(ipv4_buf));

	/* display what we've got */
	ret = dhd_arp_get_arp_hostip_table(dhd_pub, ipv4_buf, sizeof(ipv4_buf), idx);
	DHD_ARPOE(("%s: hostip table read from Dongle:\n", __FUNCTION__));
#ifdef AOE_DBG
	dhd_print_buf(ipv4_buf, 32, 4); /* max 8 IPs 4b each */
#endif
	/* now we saved hoste_ip table, clr it in the dongle AOE */
	dhd_aoe_hostip_clr(dhd_pub, idx);

	if (ret) {
		DHD_ERROR(("%s failed\n", __FUNCTION__));
		return;
	}

	for (i = 0; i < MAX_IPV4_ENTRIES; i++) {
		if (add && (ipv4_buf[i] == 0)) {
				ipv4_buf[i] = ipa;
				add = FALSE; /* added ipa to local table  */
				DHD_ARPOE(("%s: Saved new IP in temp arp_hostip[%d]\n",
				__FUNCTION__, i));
		} else if (ipv4_buf[i] == ipa) {
			ipv4_buf[i]	= 0;
			DHD_ARPOE(("%s: removed IP:%x from temp table %d\n",
				__FUNCTION__, ipa, i));
		}

		if (ipv4_buf[i] != 0) {
			/* add back host_ip entries from our local cache */
			dhd_arp_offload_add_ip(dhd_pub, ipv4_buf[i], idx);
			DHD_ARPOE(("%s: added IP:%x to dongle arp_hostip[%d]\n\n",
				__FUNCTION__, ipv4_buf[i], i));
		}
	}
#ifdef AOE_DBG
	/* see the resulting hostip table */
	dhd_arp_get_arp_hostip_table(dhd_pub, ipv4_buf, sizeof(ipv4_buf), idx);
	DHD_ARPOE(("%s: read back arp_hostip table:\n", __FUNCTION__));
	dhd_print_buf(ipv4_buf, 32, 4); /* max 8 IPs 4b each */
#endif
}

/*
 * Notification mechanism from kernel to our driver. This function is called by the Linux kernel
 * whenever there is an event related to an IP address.
 * ptr : kernel provided pointer to IP address that has changed
 */
static int
dhd_inetaddr_notifier_call(struct notifier_block *this,
	unsigned long event,
	void *ptr)
{
	struct in_ifaddr *ifa = (struct in_ifaddr *)ptr;

	dhd_info_t *dhd;
	dhd_pub_t *dhd_pub;
	int idx;

	if (!dhd_arp_enable)
		return NOTIFY_DONE;
	if (!ifa || !(ifa->ifa_dev->dev))
		return NOTIFY_DONE;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 31))
	/* Filter notifications meant for non Broadcom devices */
	if ((ifa->ifa_dev->dev->netdev_ops != &dhd_ops_pri) &&
	    (ifa->ifa_dev->dev->netdev_ops != &dhd_ops_virt)) {
#if defined(WL_ENABLE_P2P_IF)
		if (!wl_cfgp2p_is_ifops(ifa->ifa_dev->dev->netdev_ops))
#endif /* WL_ENABLE_P2P_IF */
			return NOTIFY_DONE;
	}
#endif /* LINUX_VERSION_CODE */

	dhd = DHD_DEV_INFO(ifa->ifa_dev->dev);
	if (!dhd)
		return NOTIFY_DONE;

	dhd_pub = &dhd->pub;

	if (dhd_pub->wlcore->arp_version == 1) {
		idx = 0;
	} else {
		for (idx = 0; idx < DHD_MAX_IFS; idx++) {
			if (dhd->iflist[idx] && dhd->iflist[idx]->net == ifa->ifa_dev->dev)
			break;
		}
		if (idx < DHD_MAX_IFS) {
			DHD_TRACE(("ifidx : %p %s %d\n", dhd->iflist[idx]->net,
				dhd->iflist[idx]->name, dhd->iflist[idx]->idx));
		} else {
			DHD_ERROR(("Cannot find ifidx for(%s) set to 0\n", ifa->ifa_label));
			idx = 0;
		}
	}

	switch (event) {
		case NETDEV_UP:
			DHD_ARPOE(("%s: [%s] Up IP: 0x%x\n",
				__FUNCTION__, ifa->ifa_label, ifa->ifa_address));

			if (dhd->pub.busstate != DHD_BUS_DATA) {
				DHD_ERROR(("%s: bus not ready, exit\n", __FUNCTION__));
				if (dhd->pend_ipaddr) {
					DHD_ERROR(("%s: overwrite pending ipaddr: 0x%x\n",
						__FUNCTION__, dhd->pend_ipaddr));
				}
				dhd->pend_ipaddr = ifa->ifa_address;
				break;
			}

#ifdef AOE_IP_ALIAS_SUPPORT
			DHD_ARPOE(("%s:add aliased IP to AOE hostip cache\n",
				__FUNCTION__));
			aoe_update_host_ipv4_table(dhd_pub, ifa->ifa_address, TRUE, idx);
#endif /* AOE_IP_ALIAS_SUPPORT */
			break;

		case NETDEV_DOWN:
			DHD_ARPOE(("%s: [%s] Down IP: 0x%x\n",
				__FUNCTION__, ifa->ifa_label, ifa->ifa_address));
			dhd->pend_ipaddr = 0;
#ifdef AOE_IP_ALIAS_SUPPORT
			DHD_ARPOE(("%s:interface is down, AOE clr all for this if\n",
				__FUNCTION__));
			aoe_update_host_ipv4_table(dhd_pub, ifa->ifa_address, FALSE, idx);
#else
			dhd_aoe_hostip_clr(&dhd->pub, idx);
			dhd_aoe_arp_clr(&dhd->pub, idx);
#endif /* AOE_IP_ALIAS_SUPPORT */
			break;

		default:
			DHD_ARPOE(("%s: do noting for [%s] Event: %lu\n",
				__func__, ifa->ifa_label, event));
			break;
	}

	return NOTIFY_DONE;
}
#endif /* ARP_OFFLOAD_SUPPORT */

#if defined(OEM_ANDROID) && defined(CONFIG_IPV6)
/* Neighbor Discovery Offload: defered handler */
static void
dhd_inet6_work_handler(void *dhd_info, void *event_data, enum dhd_wq_event event)
{
	struct ipv6_work_info_t *ndo_work = (struct ipv6_work_info_t *)event_data;
	dhd_pub_t	*pub = &((dhd_info_t *)dhd_info)->pub;
	int		ret;

	if (event != DHD_WQ_WORK_IPV6_NDO) {
		DHD_ERROR(("%s: unexpected event \n", __FUNCTION__));
		return;
	}

	if (!ndo_work) {
		DHD_ERROR(("%s: ipv6 work info is not initialized \n", __FUNCTION__));
		return;
	}

	if (!pub) {
		DHD_ERROR(("%s: dhd pub is not initialized \n", __FUNCTION__));
		return;
	}

	if (ndo_work->if_idx) {
		DHD_ERROR(("%s: idx %d \n", __FUNCTION__, ndo_work->if_idx));
		return;
	}

	switch (ndo_work->event) {
		case NETDEV_UP:
			DHD_TRACE(("%s: Enable NDO and add ipv6 into table \n ", __FUNCTION__));
			ret = dhd_ndo_enable(pub, TRUE);
			if (ret < 0) {
				DHD_ERROR(("%s: Enabling NDO Failed %d\n", __FUNCTION__, ret));
			}

			ret = dhd_ndo_add_ip(pub, &ndo_work->ipv6_addr[0], ndo_work->if_idx);
			if (ret < 0) {
				DHD_ERROR(("%s: Adding host ip for NDO failed %d\n",
					__FUNCTION__, ret));
			}
			break;
		case NETDEV_DOWN:
			DHD_TRACE(("%s: clear ipv6 table \n", __FUNCTION__));
			ret = dhd_ndo_remove_ip(pub, ndo_work->if_idx);
			if (ret < 0) {
				DHD_ERROR(("%s: Removing host ip for NDO failed %d\n",
					__FUNCTION__, ret));
				goto done;
			}

			ret = dhd_ndo_enable(pub, FALSE);
			if (ret < 0) {
				DHD_ERROR(("%s: disabling NDO Failed %d\n", __FUNCTION__, ret));
				goto done;
			}
			break;
		default:
			DHD_ERROR(("%s: unknown notifier event \n", __FUNCTION__));
			break;
	}
done:
	/* free ndo_work. alloced while scheduling the work */
	kfree(ndo_work);

	return;
}

/*
 * Neighbor Discovery Offload: Called when an interface
 * is assigned with ipv6 address.
 * Handles only primary interface
 */
static int
dhd_inet6addr_notifier_call(struct notifier_block *this,
	unsigned long event,
	void *ptr)
{
	dhd_info_t *dhd;
	dhd_pub_t *dhd_pub;
	struct inet6_ifaddr *inet6_ifa = ptr;
	struct in6_addr *ipv6_addr = &inet6_ifa->addr;
	struct ipv6_work_info_t *ndo_info;
	int idx = 0; /* REVISIT */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 31))
	/* Filter notifications meant for non Broadcom devices */
	if (inet6_ifa->idev->dev->netdev_ops != &dhd_ops_pri) {
			return NOTIFY_DONE;
	}
#endif /* LINUX_VERSION_CODE */

	dhd = DHD_DEV_INFO(inet6_ifa->idev->dev);
	if (!dhd)
		return NOTIFY_DONE;

	if (dhd->iflist[idx] && dhd->iflist[idx]->net != inet6_ifa->idev->dev)
		return NOTIFY_DONE;
	dhd_pub = &dhd->pub;

	if (!FW_SUPPORTED(dhd_pub->wlcore, ndoe))
		return NOTIFY_DONE;

	ndo_info = MALLOCZ(dhd_pub->osh, sizeof(struct ipv6_work_info_t));
	if (!ndo_info) {
		DHD_ERROR(("%s: ipv6 work alloc failed\n", __FUNCTION__));
		return NOTIFY_DONE;
	}

	ndo_info->event = event;
	ndo_info->if_idx = idx;
	memcpy(&ndo_info->ipv6_addr[0], ipv6_addr, IPV6_ADDR_LEN);

	/* defer the work to thread as it may block kernel */
	if (dhd_deferred_schedule_work(dhd->dhd_deferred_wq,
		ndo_info, DHD_WQ_WORK_IPV6_NDO,
		dhd_inet6_work_handler, DHD_WORK_PRIORITY_LOW) != DHD_WQ_STS_OK) {
		MFREE(dhd_pub->osh, ndo_info, sizeof(struct ipv6_work_info_t));
	}

	return NOTIFY_DONE;
}
#endif /* OEM_ANDROID && CONFIG_IPV6 */

int
dhd_register_if(dhd_pub_t *dhdp, int ifidx, bool need_rtnl_lock)
{
	dhd_info_t *dhd = (dhd_info_t *)dhdp->info;
	dhd_if_t *ifp;
	struct net_device *net = NULL;
	int err = 0;
	uint8 temp_addr[ETHER_ADDR_LEN] = { 0x00, 0x90, 0x4c, 0x11, 0x22, 0x33 };

	DHD_TRACE(("%s: ifidx %d\n", __FUNCTION__, ifidx));

	ASSERT(dhd && dhd->iflist[ifidx]);
	ifp = dhd->iflist[ifidx];
	net = ifp->net;
	ASSERT(net && (ifp->idx == ifidx));

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 31))
	ASSERT(!net->open);
	net->get_stats = dhd_get_stats;
	net->do_ioctl = dhd_ioctl_entry;
	net->hard_start_xmit = dhd_start_xmit;
	net->set_mac_address = dhd_set_mac_address;
	net->set_multicast_list = dhd_set_multicast_list;
	net->open = net->stop = NULL;
#else
	ASSERT(!net->netdev_ops);
	net->netdev_ops = &dhd_ops_virt;
#if defined(BCM_DHD_RUNNER)
#if !defined(BCM_COUNTER_EXTSTATS)
	net->put_stats = dhd_update_fp_stats;
	net->clr_stats = dhd_clear_stats;
	net->get_stats_pointer = dhd_get_stats_pointer;
#endif
#endif /* BCM_DHD_RUNNER */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
	net->max_mtu = DHD_MAX_MTU;
#endif

#ifdef BCM_NBUFF_WLMCAST
#if defined(BCM_DHD_RUNNER) || defined(BCM_AWL)
	netdev_wlan_client_get_info(net) = dhd_client_get_info;
#else
	netdev_wlan_client_get_info(net) = NULL;
#endif /* BCM_DHD_RUNNER || BCM_AWL */
#endif /* BCM_NBUFF_WLMCAST */

#if defined(BCM_DHDHDR) && defined(PCIE_FULL_DONGLE)
	net->needed_headroom += DOT11_LLC_SNAP_HDR_LEN;
#endif
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 31) */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 32)) && 0
	{
		struct device *parent_dev;

		if (ifidx == 0) {
			struct pci_dev *pdev = osl_pci_device(dhd->pub.osh);

			/* Primary interface, set parent to be the pci_dev */
			parent_dev = &pdev->dev;
		} else {
			/* Virtual interface, set parent to be the primary device */
			parent_dev = &dhd->pub.info->iflist[0]->net->dev;
		}
		SET_NETDEV_DEV(net, parent_dev);
		SET_NETDEV_DEVTYPE(net, &wlan_type);
	}
#endif

	/* Ok, link into the network layer... */
	if (ifidx == 0) {
		/*
		 * device functions for the primary interface only
		 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 31))
		net->open = dhd_open;
		net->stop = dhd_stop;
#else
		net->netdev_ops = &dhd_ops_pri;
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 31) */
		if (!ETHER_ISNULLADDR(dhd->pub.mac.octet))
			memcpy(temp_addr, dhd->pub.mac.octet, ETHER_ADDR_LEN);
	} else {
		/*
		 * We have to use the primary MAC for virtual interfaces
		 */
		memcpy(temp_addr, ifp->mac_addr, ETHER_ADDR_LEN);
#if defined(OEM_ANDROID)
		/*
		 * Android sets the locally administered bit to indicate that this is a
		 * portable hotspot.  This will not work in simultaneous AP/STA mode,
		 * nor with P2P.  Need to set the Donlge's MAC address, and then use that.
		 */
		if (!memcmp(temp_addr, dhd->iflist[0]->mac_addr,
			ETHER_ADDR_LEN)) {
			DHD_ERROR(("%s interface [%s]: set locally administered bit in MAC\n",
			__func__, net->name));
			temp_addr[0] |= 0x02;
		}
#endif /* defined(OEM_ANDROID) */
	}

	net->hard_header_len = ETH_HLEN + dhd->pub.hdrlen;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24)
	net->ethtool_ops = &dhd_ethtool_ops;
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24) */

#if defined(WL_WIRELESS_EXT)
#if WIRELESS_EXT < 19
	net->get_wireless_stats = dhd_get_wireless_stats;
#endif /* WIRELESS_EXT < 19 */
#if WIRELESS_EXT > 12
	net->wireless_handlers = &wl_iw_handler_def;
#endif /* WIRELESS_EXT > 12 */
#endif /* defined(WL_WIRELESS_EXT) */

	dhd->pub.rxsz = DBUS_RX_BUFFER_SIZE_DHD(net);

	/*
	 * XXX Linux 2.6.25 does not like a blank MAC address, so use a
	 * dummy address until the interface is brought up.
	 */
	memcpy(net->dev_addr, temp_addr, ETHER_ADDR_LEN);

	if (ifidx == 0) {
		printf("%s\n", dhd_version);
#ifdef WL_CFG80211
		/* assure wiphy is registered before primary netdev */
		if (net->ieee80211_ptr) {
			wl_cfg80211_register_wiphy(net);
		}
#endif
	}

	if (need_rtnl_lock)
		err = register_netdev(net);
	else
		err = register_netdevice(net);

	if (err != 0) {
		DHD_ERROR(("couldn't register the net device [%s], err %d\n", net->name, err));
		goto fail;
	}

#if defined(BCM_WFD)
	if (dhd_wfd_registerdevice(dhd->pub.wfd_idx, net) != 0)
	{
	   DHD_ERROR(("dhd_wfd_registerdevice failed [%s]\n", net->name));
	   goto fail;
	}
#endif /* BCM_WFD */

#if defined(ARGOS_CPU_SCHEDULER) && defined(ARGOS_RPS_CPU_CTL)
	if (ifidx == 0) {
		argos_register_notifier_init(net);
	}
#endif /* ARGOS_CPU_SCHEDULER && ARGOS_RPS_CPU_CTL */
	printf("Register interface [%s]  MAC: "MACDBG"\n\n", net->name,
#if defined(CUSTOMER_HW4)
		MAC2STRDBG(dhd->pub.mac.octet));
#else
		MAC2STRDBG(net->dev_addr));
#endif /* CUSTOMER_HW4 */

#if defined(OEM_ANDROID) && (defined(BCMPCIE) || (defined(BCMLXSDMMC) && \
	(LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27))))
	if (ifidx == 0) {
#ifdef BCMLXSDMMC
		up(&dhd_registration_sem);
#endif /* BCMLXSDMMC */
		if (!dhd_download_fw_on_driverload) {
#ifdef WL_CFG80211
			wl_terminate_event_handler(net);
#endif /* WL_CFG80211 */
#if defined(DHD_LB) && defined(DHD_LB_RXP)
			skb_queue_purge(&dhd->rx_pend_queue);
#endif /* DHD_LB && DHD_LB_RXP */
			dhd_net_bus_devreset(net, TRUE);
#ifdef BCMLXSDMMC
			dhd_net_bus_suspend(net);
#endif /* BCMLXSDMMC */
			wifi_platform_set_power(dhdp->info->adapter, FALSE, WIFI_TURNOFF_DELAY);
		}
	}
#endif /* OEM_ANDROID && (BCMPCIE || (BCMLXSDMMC && KERNEL_VERSION >= 2.6.27)) */

	return 0;

fail:
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 31)
	net->open = NULL;
#else
	net->netdev_ops = NULL;
#endif
	return err;
}

void
dhd_bus_detach(dhd_pub_t *dhdp)
{
	dhd_info_t *dhd;

	DHD_TRACE(("%s: Enter\n", __FUNCTION__));

	if (dhdp) {
		dhd = (dhd_info_t *)dhdp->info;
		if (dhd) {

			/*
			 * In case of Android cfg80211 driver, the bus is down in dhd_stop,
			 *  calling stop again will cuase SD read/write errors.
			 */
			if (dhd->pub.busstate != DHD_BUS_DOWN) {
				/* Stop the protocol module */
				dhd_prot_stop(&dhd->pub);

				/* Stop the bus module */
#ifdef BCMDBUS
				/* Force Dongle terminated */
				if (dhd_wl_ioctl_cmd(dhdp, WLC_TERMINATED, NULL, 0, TRUE, 0) < 0)
					DHD_ERROR(("%s Setting WLC_TERMINATED failed\n",
						__FUNCTION__));
				dbus_stop(dhd->pub.dbus);
				dhd->pub.busstate = DHD_BUS_DOWN;
#else
				dhd_bus_stop(dhd->pub.bus, TRUE);
#endif /* BCMDBUS */
			}

#if defined(OOB_INTR_ONLY) || defined(BCMSPI_ANDROID) || defined(BCMPCIE_OOB_HOST_WAKE)
			dhd_bus_oob_intr_unregister(dhdp);
#endif /* OOB_INTR_ONLY || BCMSPI_ANDROID || BCMPCIE_OOB_HOST_WAKE */
		}
	}
}

void
dhd_detach(dhd_pub_t *dhdp)
{
	dhd_info_t *dhd;
	unsigned long flags;
	int timer_valid = FALSE;
	struct net_device *dev;
#ifdef WL_CFG80211
	struct bcm_cfg80211 *cfg = NULL;
#endif
#if defined(BCA_SROMMAP)
	reinit_loaded_srommap();
#endif /* DSLCPE_DONGLEHOST_WOMBO */

	if (!dhdp)
		return;

	dhd = (dhd_info_t *)dhdp->info;
	if (!dhd)
		return;
	dev = dhd->iflist[0]->net;

	if (dev) {
		rtnl_lock();
		if (dev->flags & IFF_UP) {
		/* If IFF_UP is still up, it indicates that
		 * "ifconfig wlan0 down" hasn't been called.
		 * So invoke dev_close explicitly here to
		 * bring down the interface.
		 */
			dev_close(dev);
		}
		rtnl_unlock();
	}

	DHD_TRACE(("%s: Enter state 0x%x\n", __FUNCTION__, dhd->dhd_state));

	/* XXX	kernel panic issue when first bootup time,
	 *	 rmmod without interface down make unnecessary hang event.
	 */
	dhd->pub.up = 0;
	if (dhd->dhd_state & DHD_ATTACH_STATE_DONE) {
		if (dhd_found > 0)
			dhd_found--;
	}
	else {
		/* Give sufficient time for threads to start running in case
		 * dhd_attach() has failed
		 */
		OSL_SLEEP(100);
	}
#ifdef DHD_WET
	dhd_free_wet_info(&dhd->pub, dhd->pub.wet_info);
#endif

#if defined(DHD_DPSTA) && (defined(DHD_PSTA) || defined(DHD_WET))
	dpsta_unregister(dhd_get_instance(&dhd->pub));
#endif

#if defined(BCM_DNGL_EMBEDIMAGE) || defined(BCM_REQUEST_FW)
#if defined(BCMDBUS)
	if (dhd->fw_download_task) {
		up(&dhd->fw_download_lock);
		kthread_stop(dhd->fw_download_task);
		dhd->fw_download_task = NULL;
	}
#endif /* BCMDBUS */
#endif /* defined(BCM_DNGL_EMBEDIMAGE) || defined(BCM_REQUEST_FW) */

#ifdef PROP_TXSTATUS
#ifdef DHD_WLFC_THREAD
	if (dhd->pub.wlfc_thread) {
		kthread_stop(dhd->pub.wlfc_thread);
		dhdp->wlfc_thread_go = TRUE;
		wake_up_interruptible(&dhdp->wlfc_wqhead);
	}
	dhd->pub.wlfc_thread = NULL;
#endif /* DHD_WLFC_THREAD */
#endif /* PROP_TXSTATUS */
#ifdef WL_CFG80211
	if (dev)
		wl_cfg80211_down(dev);
#endif /* WL_CFG80211 */

	if (dhd->dhd_state & DHD_ATTACH_STATE_PROT_ATTACH) {

		dhd_bus_detach(dhdp);
#ifdef OEM_ANDROID
#ifdef BCMPCIE
		if (is_reboot == SYS_RESTART) {
			extern bcmdhd_wifi_platdata_t *dhd_wifi_platdata;
			if (dhd_wifi_platdata && !dhdp->dongle_reset) {
				dhdpcie_bus_clock_stop(dhdp->bus);
				wifi_platform_set_power(dhd_wifi_platdata->adapters,
					FALSE, WIFI_TURNOFF_DELAY);
			}
		}
#endif /* BCMPCIE */
#endif /* OEM_ANDROID */
#ifndef PCIE_FULL_DONGLE
		if (dhdp->prot)
			dhd_prot_detach(dhdp);
#endif
	}

#ifdef ARP_OFFLOAD_SUPPORT
	if (dhd_inetaddr_notifier_registered) {
		dhd_inetaddr_notifier_registered = FALSE;
		unregister_inetaddr_notifier(&dhd_inetaddr_notifier);
	}
#endif /* ARP_OFFLOAD_SUPPORT */
#if defined(OEM_ANDROID) && defined(CONFIG_IPV6)
	if (dhd_inet6addr_notifier_registered) {
		dhd_inet6addr_notifier_registered = FALSE;
		unregister_inet6addr_notifier(&dhd_inet6addr_notifier);
	}
#endif /* OEM_ANDROID && CONFIG_IPV6 */
#if defined(CONFIG_HAS_EARLYSUSPEND) && defined(DHD_USE_EARLYSUSPEND)
	if (dhd->dhd_state & DHD_ATTACH_STATE_EARLYSUSPEND_DONE) {
		if (dhd->early_suspend.suspend)
			unregister_early_suspend(&dhd->early_suspend);
	}
#endif /* CONFIG_HAS_EARLYSUSPEND && DHD_USE_EARLYSUSPEND */

#if defined(WL_WIRELESS_EXT)
	if (dhd->dhd_state & DHD_ATTACH_STATE_WL_ATTACH) {
		/* Detatch and unlink in the iw */
		wl_iw_detach();
	}
#endif /* defined(WL_WIRELESS_EXT) */

	dhd_deferred_work_disable_all(dhd->dhd_deferred_wq);

	/* delete all interfaces, start with virtual  */
	if (dhd->dhd_state & DHD_ATTACH_STATE_ADD_IF) {
		int i = 1;
		dhd_if_t *ifp;

		/* Cleanup virtual interfaces */
		DHD_LOCK(dhdp);
		dhd_net_if_lock_local(dhd);
		for (i = 1; i < DHD_MAX_IFS; i++) {
			if (dhd->iflist[i]) {
				dhd_remove_if(&dhd->pub, i, TRUE);
			}
		}
		dhd_net_if_unlock_local(dhd);
		DHD_UNLOCK(dhdp);

		/*  delete primary interface 0 */
		ifp = dhd->iflist[0];
		ASSERT(ifp);
		ASSERT(ifp->net);
		if (ifp && ifp->net) {
#ifdef WL_CFG80211
			cfg = wl_get_cfg(ifp->net);
#endif

			/* in unregister_netdev case, the interface gets freed by net->destructor
			 * (which is set to free_netdev)
			 */
			if (ifp->net->reg_state == NETREG_UNINITIALIZED) {
				free_netdev(ifp->net);
			} else {
#if defined(ARGOS_CPU_SCHEDULER) && defined(ARGOS_RPS_CPU_CTL)
				argos_register_notifier_deinit();
#endif /*  ARGOS_CPU_SCHEDULER && ARGOS_RPS_CPU_CTL */
#ifdef SET_RPS_CPUS
				custom_rps_map_clear(ifp->net->_rx);
#endif /* SET_RPS_CPUS */
				netif_tx_disable(ifp->net);
				unregister_netdev(ifp->net);
			}
#ifdef PCIE_FULL_DONGLE
			ifp->net = DHD_NET_DEV_NULL;
#else
			ifp->net = NULL;
#endif /* PCIE_FULL_DONGLE */
#ifdef DHD_WMF
			dhd_wmf_cleanup(dhdp, 0);
#endif /* DHD_WMF */
#ifdef DHD_L2_FILTER
			bcm_l2_filter_arp_table_update(dhdp->osh, ifp->phnd_arp_table, TRUE,
				NULL, FALSE, dhdp->tickcnt);
			deinit_l2_filter_arp_table(dhdp->osh, ifp->phnd_arp_table);
			ifp->phnd_arp_table = NULL;
			ifp->parp_enable = FALSE;
#endif /* DHD_L2_FILTER */

#if (defined(BCM_ROUTER_DHD) && defined(QOS_MAP_SET))
			MFREE(dhdp->osh, ifp->qosmap_up_table, UP_TABLE_MAX);
			ifp->qosmap_up_table = NULL;
			ifp->qosmap_up_table_enable = FALSE;
#endif /* BCM_ROUTER_DHD && QOS_MAP_SET */

			dhd_if_del_sta_list(ifp);

			MFREE(dhd->pub.osh, ifp, sizeof(*ifp));
			dhd->iflist[0] = NULL;
		}
	}

	/* Clear the watchdog timer */
	DHD_GENERAL_LOCK(&dhd->pub, flags);
	timer_valid = dhd->wd_timer_valid;
	dhd->wd_timer_valid = FALSE;
	DHD_GENERAL_UNLOCK(&dhd->pub, flags);
	if (timer_valid)
		del_timer_sync(&dhd->timer);

#ifdef BCMDBUS
	tasklet_kill(&dhd->tasklet);
#else
	if (dhd->dhd_state & DHD_ATTACH_STATE_THREADS_CREATED) {
		if (dhd->thr_wdt_ctl.thr_pid >= 0) {
			PROC_STOP(&dhd->thr_wdt_ctl);
		}

		if (dhd->rxthread_enabled && dhd->thr_rxf_ctl.thr_pid >= 0) {
			PROC_STOP(&dhd->thr_rxf_ctl);
		}

		if (dhd->thr_dpc_ctl.thr_pid >= 0) {
			PROC_STOP(&dhd->thr_dpc_ctl);
		} else {
			tasklet_kill(&dhd->tasklet);
#ifdef DHD_LB_RXP
			skb_queue_purge(&dhd->rx_pend_queue);
#endif /* DHD_LB_RXP */
		}
	}
#endif /* BCMDBUS */

#if defined(DHD_LB)
	/* Kill the Load Balancing Tasklets */
#if defined(DHD_LB_TXC)
	tasklet_disable(&dhd->tx_compl_tasklet);
	tasklet_kill(&dhd->tx_compl_tasklet);
#endif /* DHD_LB_TXC */
#if defined(DHD_LB_RXC)
	tasklet_disable(&dhd->rx_compl_tasklet);
	tasklet_kill(&dhd->rx_compl_tasklet);
#endif /* DHD_LB_RXC */
	if (dhd->cpu_notifier.notifier_call != NULL)
		unregister_cpu_notifier(&dhd->cpu_notifier);
	dhd_cpumasks_deinit(dhd);
#endif /* DHD_LB */

#ifdef WL_CFG80211
	if (dhd->dhd_state & DHD_ATTACH_STATE_CFG80211) {
		wl_cfg80211_detach(cfg);
		wl_monitor_uninit();
	}
#endif
	wl_core_deinit(dhdp);
	/* free deferred work queue */
	dhd_deferred_work_deinit(dhd->dhd_deferred_wq);
	dhd->dhd_deferred_wq = NULL;

#ifdef BCMDBUS
	if (dhdp->dbus) {
		dbus_detach(dhdp->dbus);
		dhdp->dbus = NULL;
	}
#endif /* BCMDBUS */

#ifdef PNO_SUPPORT
	if (dhdp->pno_state)
		dhd_pno_deinit(dhdp);
#endif
#if defined(CONFIG_PM_SLEEP)
	if (dhd->pm_notifier.notifier_call != NULL) {
		unregister_pm_notifier(&dhd->pm_notifier);
		dhd->pm_notifier.notifier_call = NULL;
	}
#endif /* CONFIG_PM_SLEEP */

#ifdef DEBUG_CPU_FREQ
		if (dhd->new_freq)
			free_percpu(dhd->new_freq);
		dhd->new_freq = NULL;
		cpufreq_unregister_notifier(&dhd->freq_trans, CPUFREQ_TRANSITION_NOTIFIER);
#endif
	if (dhd->dhd_state & DHD_ATTACH_STATE_WAKELOCKS_INIT) {
		DHD_TRACE(("wd wakelock count:%d\n", dhd->wakelock_wd_counter));
#ifdef CONFIG_HAS_WAKELOCK
		dhd->wakelock_counter = 0;
		dhd->wakelock_wd_counter = 0;
		dhd->wakelock_rx_timeout_enable = 0;
		dhd->wakelock_ctrl_timeout_enable = 0;
		wake_lock_destroy(&dhd->wl_wifi);
		wake_lock_destroy(&dhd->wl_rxwake);
		wake_lock_destroy(&dhd->wl_ctrlwake);
		wake_lock_destroy(&dhd->wl_wdwake);
#ifdef BCMPCIE_OOB_HOST_WAKE
		wake_lock_destroy(&dhd->wl_intrwake);
#endif /* BCMPCIE_OOB_HOST_WAKE */
#endif /* CONFIG_HAS_WAKELOCK */
	}

#if defined(CUSTOMER_HW4) && defined(ARGOS_CPU_SCHEDULER)
	if (dhd->pub.affinity_isdpc == TRUE) {
		free_cpumask_var(dhd->pub.default_cpu_mask);
		free_cpumask_var(dhd->pub.dpc_affinity_cpu_mask);
		dhd->pub.affinity_isdpc = FALSE;
	}
#endif /* CUSTOMER_HW4 && ARGOS_CPU_SCHEDULER */

#if (defined(BCM_ROUTER_DHD) && defined(BCM_GMAC3))
	dhd_perim_radio_reg(DHD_FWDER_UNIT(dhd), (dhd_pub_t *)NULL);
#endif /* BCM_ROUTER_DHD && BCM_GMAC3 */

#ifdef DHDTCPACK_SUPPRESS
	/* This will free all MEM allocated for TCPACK SUPPRESS */
	dhd_tcpack_suppress_set(&dhd->pub, TCPACK_SUP_OFF);
#endif /* DHDTCPACK_SUPPRESS */

#ifdef PCIE_FULL_DONGLE
		dhd_flow_rings_deinit(dhdp);
		if (dhdp->prot)
			dhd_prot_detach(dhdp);
#endif

#if defined(BCM_CPE_PKTC)
	dhd_pktc_detach(dhdp);
#endif

#if defined(BCM_NBUFF_PKT_BPM)
	dhd_nbuff_bpm_deinit(&dhd->pub);
#endif /* BCM_NBUFF_PKT_BPM */

	if (dhdp->macdbg_info)
		dhd_macdbg_detach(dhdp);

#ifdef DHD_LBR_AGGR_BCM_ROUTER
	dhd_lbr_aggr_deinit(dhdp);
	dhd_aggr_deinit(dhdp);
#endif /* DHD_LBR_AGGR_BCM_ROUTER */

#ifdef DHD_IFE
	dhd_ife_deinit(dhdp);
#endif /* DHD_IFE */

}

void
dhd_free(dhd_pub_t *dhdp)
{
	dhd_info_t *dhd;
	DHD_TRACE(("%s: Enter\n", __FUNCTION__));

	if (dhdp) {
		int i;
		for (i = 0; i < ARRAYSIZE(dhdp->reorder_bufs); i++) {
			if (dhdp->reorder_bufs[i]) {
				reorder_info_t *ptr;
				uint32 buf_size = sizeof(struct reorder_info);

				ptr = dhdp->reorder_bufs[i];

				buf_size += ((ptr->max_idx + 1) * sizeof(void*));
				DHD_REORDER(("free flow id buf %d, maxidx is %d, buf_size %d\n",
					i, ptr->max_idx, buf_size));

				MFREE(dhdp->osh, dhdp->reorder_bufs[i], buf_size);
				dhdp->reorder_bufs[i] = NULL;
			}
		}

		dhd_sta_pool_fini(dhdp, DHD_MAX_STA);

		dhd = (dhd_info_t *)dhdp->info;
		if (dhdp->soc_ram) {
#if defined(CONFIG_DHD_USE_STATIC_BUF) && defined(DHD_USE_STATIC_MEMDUMP)
			DHD_OS_PREFREE(dhdp, dhdp->soc_ram, dhdp->soc_ram_length);
#else
			MFREE(dhdp->osh, dhdp->soc_ram, dhdp->soc_ram_length);
#endif /* CONFIG_DHD_USE_STATIC_BUF && DHD_USE_STATIC_MEMDUMP */
			dhdp->soc_ram = NULL;
		}
#ifdef CACHE_FW_IMAGES
		if (dhdp->cached_fw) {
			MFREE(dhdp->osh, dhdp->cached_fw, dhdp->bus->ramsize);
			dhdp->cached_fw = NULL;
		}

		if (dhdp->cached_nvram) {
			MFREE(dhdp->osh, dhdp->cached_nvram, MAX_NVRAMBUF_SIZE);
			dhdp->cached_nvram = NULL;
		}
#endif

#ifdef BCM_WFD
		if (WFD_ENABLED(dhdp))
#ifndef BCM_PKTFWD
			dhd_wfd_unbind(dhd->pub.wfd_idx);
#else
			dhd_wfd_unbind(dhd->pub.wfd_idx, dhdp->unit);
#endif /* BCM_PKTFWD */
#endif /* BCM_WFD */

#if defined(BCM_AWL)
	    if (dhdp && dhdp->awl_cb)
	        dhd_awl_detach(dhdp, dhdp->awl_cb);
#endif /* BCM_AWL */

#ifdef BCM_PKTFWD
		/* BCM_PKTFWD: delete radio from pktfwd */
		if (dhdp && PKTFWD_ENABLED(dhdp))
			dhd_pktfwd_radio_del(dhdp->unit);
#endif /* BCM_PKTFWD */

		/* If pointer is allocated by dhd_os_prealloc then avoid MFREE */
		if (dhd &&
			dhd != (dhd_info_t *)dhd_os_prealloc(dhdp, DHD_PREALLOC_DHD_INFO, 0, FALSE))
			MFREE(dhdp->osh, dhd, sizeof(*dhd));
		dhd = NULL;
	}
}

void
dhd_clear(dhd_pub_t *dhdp)
{
	DHD_TRACE(("%s: Enter\n", __FUNCTION__));

	if (dhdp) {
		int i;
#ifdef DHDTCPACK_SUPPRESS
		/* Clean up timer/data structure for any remaining/pending packet or timer. */
		dhd_tcpack_info_tbl_clean(dhdp);
#endif /* DHDTCPACK_SUPPRESS */
		for (i = 0; i < ARRAYSIZE(dhdp->reorder_bufs); i++) {
			if (dhdp->reorder_bufs[i]) {
				reorder_info_t *ptr;
				uint32 buf_size = sizeof(struct reorder_info);

				ptr = dhdp->reorder_bufs[i];

				buf_size += ((ptr->max_idx + 1) * sizeof(void*));
				DHD_REORDER(("free flow id buf %d, maxidx is %d, buf_size %d\n",
					i, ptr->max_idx, buf_size));

				MFREE(dhdp->osh, dhdp->reorder_bufs[i], buf_size);
				dhdp->reorder_bufs[i] = NULL;
			}
		}

		dhd_sta_pool_clear(dhdp, DHD_MAX_STA);

		if (dhdp->soc_ram) {
#if defined(CONFIG_DHD_USE_STATIC_BUF) && defined(DHD_USE_STATIC_MEMDUMP)
			DHD_OS_PREFREE(dhdp, dhdp->soc_ram, dhdp->soc_ram_length);
#else
			MFREE(dhdp->osh, dhdp->soc_ram, dhdp->soc_ram_length);
#endif /* CONFIG_DHD_USE_STATIC_BUF && DHD_USE_STATIC_MEMDUMP */
			dhdp->soc_ram = NULL;
		}
	}
}

static void
dhd_module_cleanup(void)
{
	DHD_TRACE(("%s: Enter\n", __FUNCTION__));

#ifdef BCMDBUS
	dbus_deregister();
#else
	dhd_bus_unregister();
#endif /* BCMDBUS */

#if defined(OEM_ANDROID)
	wl_android_exit();
#endif /* OEM_ANDROID */

	dhd_wifi_platform_unregister_drv();
}

static void
dhd_platform_cleanup(void)
{
	DHD_TRACE(("%s: Enter\n", __FUNCTION__));

#if defined(BCM_PKTFWD)
	dhd_pktfwd_sys_fini(); /* Destruct the singleton wl_pktfwd global */
#else /* !BCM_PKTFWD */
#if defined(BCM_BLOG)
	fdb_check_expired_dhd_hook = NULL;
#endif /* BCM_BLOG */
#endif /* !BCM_PKTFWD */
#if defined(BCM_NBUFF_WLMCAST)
	dhd_nbuff_detach();
#endif /* BCM_NBUFF_WLMCAST */

#if defined(BCM_BLOG) && defined(CONFIG_BCM_DPI_WLAN_QOS)
	blog_dhd_flow_update_fn = NULL;
#endif

}

static void __exit
dhd_module_exit(void)
{
	dhd_buzzz_detach();
#if defined(WL_CFG80211)
	wl_cfg80211_unregister_notifier();
#endif /* WL_CFG80211 */
	dhd_module_cleanup();
	dhd_platform_cleanup();

	unregister_reboot_notifier(&dhd_reboot_notifier);
}

#if defined(BCM_BLOG) && defined(CONFIG_BCM_DPI_WLAN_QOS)
static int
update_flowring_idx(void * device, char* eth_sa, char* eth_da, int priority)
{
	struct net_device * net = (struct net_device*)device;
	dhd_info_t *dhd = DHD_DEV_INFO(net);
	int ifidx = DHD_DEV_IFIDX(net);
	uint16 flowid = 0;

	if (dhd_flowid_lookup(&dhd->pub, ifidx, priority, eth_sa, eth_da, &flowid) == BCME_OK)
		return flowid;

	return -1;
}
#endif

static int __init
dhd_module_init(void)
{
	int err;
	int retry = POWERUP_MAX_RETRY;

	DHD_ERROR(("%s in\n", __FUNCTION__));

#if defined(BCM_BLOG) && defined(CONFIG_BCM_DPI_WLAN_QOS)
	blog_dhd_flow_update_fn = update_flowring_idx;
#endif

#ifdef BCM_NBUFF_WLMCAST
	if (dhd_nbuff_attach())
		return 0;
#endif
	dhd_buzzz_attach();

	DHD_PERIM_RADIO_INIT();
#if defined(BCM_PKTFWD)
	dhd_pktfwd_sys_init(); /* Instantiate the singleton wl_pktfwd global */
#else /* !BCM_PKTFWD */
#if defined(BCM_BLOG)
	fdb_check_expired_dhd_hook = fdb_check_expired_dhd;
#endif /* BCM_BLOG */
#endif /* !BCM_PKTFWD */

#if defined(BCM_ROUTER_DHD)
	{
		char * var;
		if ((var = getvar(NULL, "dhd_queue_budget")) != NULL) {
			dhd_queue_budget = bcm_strtoul(var, NULL, 0);
		}
		DHD_ERROR(("dhd_queue_budget = %d\n", dhd_queue_budget));

		if ((var = getvar(NULL, "dhd_sta_threshold")) != NULL) {
			dhd_sta_threshold = bcm_strtoul(var, NULL, 0);
		}
		DHD_ERROR(("dhd_sta_threshold = %d\n", dhd_sta_threshold));

		if ((var = getvar(NULL, "dhd_if_threshold")) != NULL) {
			dhd_if_threshold = bcm_strtoul(var, NULL, 0);
		}
		DHD_ERROR(("dhd_if_threshold = %d\n", dhd_if_threshold));
	}
#endif /* BCM_ROUTER_DHD */

	if (firmware_path[0] != '\0') {
		strncpy(fw_bak_path, firmware_path, MOD_PARAM_PATHLEN);
		fw_bak_path[MOD_PARAM_PATHLEN-1] = '\0';
	}

	if (nvram_path[0] != '\0') {
		strncpy(nv_bak_path, nvram_path, MOD_PARAM_PATHLEN);
		nv_bak_path[MOD_PARAM_PATHLEN-1] = '\0';
	}

	do {
		err = dhd_wifi_platform_register_drv();
		if (!err) {
			register_reboot_notifier(&dhd_reboot_notifier);
			break;
		}
		else {
			DHD_ERROR(("%s: Failed to load the driver, try cnt %d\n",
				__FUNCTION__, retry));
			strncpy(firmware_path, fw_bak_path, MOD_PARAM_PATHLEN);
			firmware_path[MOD_PARAM_PATHLEN-1] = '\0';
			strncpy(nvram_path, nv_bak_path, MOD_PARAM_PATHLEN);
			nvram_path[MOD_PARAM_PATHLEN-1] = '\0';
		}
	} while (retry--);

	if (err)
		DHD_ERROR(("%s: Failed to load driver max retry reached**\n", __FUNCTION__));

	DHD_ERROR(("%s out\n", __FUNCTION__));

	if (err)
		DHD_ERROR(("%s: Failed to load driver (err=%d)\n", __FUNCTION__, err));
#ifdef WL_CFG80211
	else
		wl_cfg80211_register_notifier();
#endif /* WL_CFG80211 */

	if ((!dhd_found) || (err)) {
		dhd_platform_cleanup();
	}

	return err;
}

static int
dhd_reboot_callback(struct notifier_block *this, unsigned long code, void *unused)
{
	DHD_TRACE(("%s: code = %ld\n", __FUNCTION__, code));
	if (code == SYS_RESTART) {
#ifdef OEM_ANDROID
#ifdef BCMPCIE
		is_reboot = code;
#endif /* BCMPCIE */
#else
#ifdef BCA_HNDROUTER
		is_reboot = code;
#endif

		dhd_module_cleanup();
#endif /* OEM_ANDROID */
	}

	return NOTIFY_DONE;
}

#ifdef BCMDBUS

/*
 * hdrlen is space to reserve in pkt headroom for DBUS
 */
void *
dhd_dbus_probe_cb(void *arg, const char *desc, uint32 bustype, uint32 hdrlen)
{
	osl_t *osh;
	int ret = 0;
	dbus_attrib_t attrib;
	dhd_pub_t *pub = NULL;

	DHD_TRACE(("%s: Enter\n", __FUNCTION__));

	atomic_set(&in_disconnect, 0);

	/* Ask the OS interface part for an OSL handle */
	if (!(osh = osl_attach(NULL, bustype, TRUE))) {
		DHD_ERROR(("%s: OSL attach failed\n", __FUNCTION__));
		ret = -ENOMEM;
		goto fail;
	}

	/* Attach to the dhd/OS interface */
	if (!(pub = dhd_attach(osh, NULL /* bus */, hdrlen))) {
		DHD_ERROR(("%s: dhd_attach failed\n", __FUNCTION__));
		ret = -ENXIO;
		goto fail;
	}

	/* Ok, finish the attach to the OS network interface */
	if (dhd_register_if(pub, 0, TRUE) != 0) {
		DHD_ERROR(("%s: dhd_register_if failed\n", __FUNCTION__));
		ret = -ENXIO;
		goto fail;
	}

	pub->dbus = dbus_attach(osh, pub->rxsz, DBUS_NRXQ, DBUS_NTXQ,
		pub->info, &dhd_dbus_cbs, NULL, NULL);
	if (pub->dbus) {
		dbus_get_attrib(pub->dbus, &attrib);
		DHD_ERROR(("DBUS: vid=0x%x pid=0x%x devid=0x%x bustype=0x%x mtu=%d\n",
			attrib.vid, attrib.pid, attrib.devid, attrib.bustype, attrib.mtu));
	} else {
		ret = -ENXIO;
		goto fail;
	}
#ifdef BCM_FD_AGGR
	pub->info->rpc_th = bcm_rpc_tp_attach(osh, (void *)pub->dbus);
	if (!pub->info->rpc_th) {
		DHD_ERROR(("%s: bcm_rpc_tp_attach failed\n", __FUNCTION__));
		ret = -ENXIO;
		goto fail;
	}

	pub->info->rpc_osh = rpc_osl_attach(osh);
	if (!pub->info->rpc_osh) {
		DHD_ERROR(("%s: rpc_osl_attach failed\n", __FUNCTION__));
		bcm_rpc_tp_detach(pub->info->rpc_th);
		pub->info->rpc_th = NULL;
		ret = -ENXIO;
		goto fail;
	}
	/* Set up the aggregation release timer */
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
	init_timer(&pub->info->rpcth_timer);
	pub->info->rpcth_timer.data = (ulong)pub->info;
	pub->info->rpcth_timer.function = dhd_rpcth_watchdog;
#else
	timer_setup(&pub->info->rpcth_timer), dhd_rpcth_watchdog, 0);
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0) */
	pub->info->rpcth_timer_active = FALSE;

	bcm_rpc_tp_register_cb(pub->info->rpc_th, NULL, pub->info,
		dbus_rpcth_rx_pkt, pub->info, pub->info->rpc_osh);
#endif /* BCM_FD_AGGR */

	/* This is passed to dhd_dbus_disconnect_cb */
	return pub->info;
fail:
	/* Release resources in reverse order */
	if (osh) {
		if (pub) {
			dhd_detach(pub);
			dhd_free(pub);
		}
		osl_detach(osh);
	}

	BCM_REFERENCE(ret);

	return NULL;
}

void
dhd_dbus_disconnect_cb(void *arg)
{
	dhd_info_t *dhd = (dhd_info_t *)arg;
	dhd_pub_t *pub;
	osl_t *osh;

	if (dhd == NULL)
		return;

	if (atomic_cmpxchg(&in_disconnect, 0, 1)) {
		DHD_ERROR(("%s: disconnect in progress\n", __FUNCTION__));
		return;
	}

	pub = &dhd->pub;
	osh = pub->osh;
#ifdef BCM_FD_AGGR
	del_timer_sync(&dhd->rpcth_timer);
	bcm_rpc_tp_deregister_cb(dhd->rpc_th);
	rpc_osl_detach(dhd->rpc_osh);
	bcm_rpc_tp_detach(dhd->rpc_th);
#endif
	dhd_detach(pub);
	dhd_free(pub);

	if (MALLOCED(osh)) {
		DHD_ERROR(("%s: MEMORY LEAK %d bytes\n", __FUNCTION__, MALLOCED(osh)));
	}
	osl_detach(osh);
}
#endif /* BCMDBUS */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0)
#if defined(CONFIG_DEFERRED_INITCALLS)
/* XXX To decrease the device boot time, deferred_module_init() macro can be
 * used. The detailed principle and implemenation of deferred_module_init()
 * is found at http://elinux.org/Deferred_Initcalls
 * To enable this feature for module build, it needs to add another
 * deferred_module_init() definition to include/linux/init.h in Linux Kernel.
 * #define deferred_module_init(fn)	module_init(fn)
 */
#if defined(CONFIG_MACH_UNIVERSAL7420)
deferred_module_init_sync(dhd_module_init);
#else
deferred_module_init(dhd_module_init);
#endif /* CONFIG_MACH_UNIVERSAL7420 */
#elif defined(USE_LATE_INITCALL_SYNC)
late_initcall_sync(dhd_module_init);
#else
late_initcall(dhd_module_init);
#endif /* USE_LATE_INITCALL_SYNC */
#else
module_init(dhd_module_init);
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0) */

module_exit(dhd_module_exit);

/*
 * OS specific functions required to implement DHD driver in OS independent way
 */
int
dhd_os_proto_block(dhd_pub_t *pub)
{
	dhd_info_t * dhd = (dhd_info_t *)(pub->info);

	if (dhd) {
		DHD_UNLOCK(pub);

		down(&dhd->proto_sem);

		DHD_LOCK(pub);
		return 1;
	}

	return 0;
}

int
dhd_os_proto_unblock(dhd_pub_t *pub)
{
	dhd_info_t * dhd = (dhd_info_t *)(pub->info);

	if (dhd) {
		up(&dhd->proto_sem);
		return 1;
	}

	return 0;
}

unsigned int
dhd_os_get_ioctl_resp_timeout(void)
{
	return ((unsigned int)dhd_ioctl_timeout_msec);
}

void
dhd_os_set_ioctl_resp_timeout(unsigned int timeout_msec)
{
	dhd_ioctl_timeout_msec = (int)timeout_msec;
}

int
dhd_os_ioctl_resp_wait(dhd_pub_t *pub, uint *condition)
{
	dhd_info_t * dhd = (dhd_info_t *)(pub->info);
	int timeout;

	/* Convert timeout in millsecond to jiffies */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27))
	timeout = msecs_to_jiffies(dhd_ioctl_timeout_msec);
#else
	timeout = dhd_ioctl_timeout_msec * HZ / 1000;
#endif
#ifdef BCMSLTGT
	timeout *= htclkratio;
#endif /* BCMSLTGT */

	DHD_UNLOCK(pub);

	timeout = wait_event_timeout(dhd->ioctl_resp_wait, (*condition), timeout);

	DHD_LOCK(pub);

	return timeout;
}

int
dhd_os_ioctl_resp_wake(dhd_pub_t *pub)
{
	dhd_info_t *dhd = (dhd_info_t *)(pub->info);

	wake_up(&dhd->ioctl_resp_wait);

	return 0;
}

int
dhd_os_d3ack_wait(dhd_pub_t *pub, uint *condition)
{
	dhd_info_t * dhd = (dhd_info_t *)(pub->info);
	int timeout;

	/* Convert timeout in millsecond to jiffies */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27))
	timeout = msecs_to_jiffies(dhd_ioctl_timeout_msec);
#else
	timeout = dhd_ioctl_timeout_msec * HZ / 1000;
#endif
#ifdef BCMSLTGT
	timeout *= htclkratio;
#endif /* BCMSLTGT */

	DHD_UNLOCK(pub);

	timeout = wait_event_timeout(dhd->d3ack_wait, (*condition), timeout);

	DHD_LOCK(pub);

	return timeout;
}

int
dhd_os_d3ack_wake(dhd_pub_t *pub)
{
	dhd_info_t *dhd = (dhd_info_t *)(pub->info);

	wake_up(&dhd->d3ack_wait);

	return 0;
}

void
dhd_os_wd_timer_extend(void *bus, bool extend)
{
#ifndef BCMDBUS
	dhd_pub_t *pub = bus;
	dhd_info_t *dhd = (dhd_info_t *)pub->info;

	if (extend)
		dhd_os_wd_timer(bus, WATCHDOG_EXTEND_INTERVAL);
	else
		dhd_os_wd_timer(bus, dhd->default_wd_interval);
#endif /* !BCMDBUS */
}

void
dhd_os_stop_wd_thread(void *bus)
{
#ifndef BCMDBUS
	dhd_pub_t *pub = bus;
	dhd_info_t *dhd = (dhd_info_t *)pub->info;

	if (!dhd) {
		DHD_ERROR(("%s: dhd NULL\n", __FUNCTION__));
		return;
	}

	if (dhd->thr_wdt_ctl.thr_pid >= 0) {
		PROC_STOP(&dhd->thr_wdt_ctl);
	}
#endif /* !BCMDBUS */
}

void
dhd_os_wd_timer(void *bus, uint wdtick)
{
#ifndef BCMDBUS
	dhd_pub_t *pub = bus;
	dhd_info_t *dhd = (dhd_info_t *)pub->info;
	unsigned long flags;

	DHD_TRACE(("%s: Enter\n", __FUNCTION__));

	if (!dhd) {
		DHD_ERROR(("%s: dhd NULL\n", __FUNCTION__));
		return;
	}

#if !defined(DHD_USE_IDLECOUNT) && defined(BCMPCIE)
	DHD_OS_WD_WAKE_LOCK(pub);
#endif /* !DHD_USE_IDLECOUNT && BCMPCIE */
	DHD_GENERAL_LOCK(pub, flags);

	/* don't start the wd until fw is loaded */
	if (pub->busstate == DHD_BUS_DOWN) {
		DHD_GENERAL_UNLOCK(pub, flags);
		if (!wdtick)
			DHD_OS_WD_WAKE_UNLOCK(pub);
		return;
	}

	/* Totally stop the timer */
	if (!wdtick && dhd->wd_timer_valid == TRUE) {
		dhd->wd_timer_valid = FALSE;
		DHD_GENERAL_UNLOCK(pub, flags);
		del_timer_sync(&dhd->timer);
		DHD_OS_WD_WAKE_UNLOCK(pub);
		return;
	}

	if (wdtick) {
		DHD_OS_WD_WAKE_LOCK(pub);
		dhd_watchdog_ms = (uint)wdtick;
		/* Re arm the timer, at last watchdog period */
		mod_timer(&dhd->timer, jiffies + msecs_to_jiffies(dhd_watchdog_ms));
		dhd->wd_timer_valid = TRUE;
	}
	DHD_GENERAL_UNLOCK(pub, flags);
#if !defined(DHD_USE_IDLECOUNT) && defined(BCMPCIE)
	DHD_OS_WD_WAKE_UNLOCK(pub);
#endif /* !DHD_USE_IDLECOUNT && BCMPCIE */
#endif /* BCMDBUS */
}

/**
 * Opens a .bin or .bea file containing firmware, in case of a .bea file seeks to the segment
 * containing the ROM offload image within the file. Various function calls in this functions are
 * 'blocking' because they perform file i/o.
 */
void *
dhd_os_open_image(dhd_pub_t *pub, char *filename)
{
	struct file *fp;
	bool bea;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0))
	pub->info->fs = get_fs(); /** restore fs on dhd_os_close_image */
	set_fs(KERNEL_DS);
#endif

	bea = dhd_bea_is_valid_file(filename);

	fp = filp_open(filename, O_RDONLY, 0); /* caution: can be a 'blocking' function call */
	/*
	 * 2.6.11 (FC4) supports filp_open() but later revs don't?
	 * Alternative:
	 * fp = open_namei(AT_FDCWD, filename, O_RD, 0);
	 * ???
	 */
	 if (IS_ERR(fp))
		 fp = NULL;

	if (bea == TRUE) {
		if (fp != NULL) {
			dhd_bea_seek_segment(fp, MSF_SEG_TYP_RTECDC_BIN);
		}
	}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0))
	if (fp == NULL) {
		set_fs(pub->info->fs);
	}
#endif
	return fp;
}

int
dhd_os_get_image_block(char *buf, int len, void *image)
{
	struct file *fp = (struct file *)image;

	if (!image)
		return 0;

	return kernel_read(fp, buf, len, &fp->f_pos);
}

void
dhd_os_close_image(dhd_pub_t *pub, void *image)
{
	if (image) {
		filp_close((struct file *)image, NULL);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0))
		set_fs(pub->info->fs);
#endif
	}
}

void
dhd_os_sdlock(dhd_pub_t *pub)
{
	dhd_info_t *dhd;

	dhd = (dhd_info_t *)(pub->info);

#ifndef BCMDBUS
	if (dhd_dpc_prio >= 0)
		down(&dhd->sdsem);
	else
		spin_lock_bh(&dhd->sdlock);
#else
	spin_lock_bh(&dhd->sdlock);
#endif /* BCMDBUS */
}

void
dhd_os_sdunlock(dhd_pub_t *pub)
{
	dhd_info_t *dhd;

	dhd = (dhd_info_t *)(pub->info);

#ifndef BCMDBUS
	if (dhd_dpc_prio >= 0)
		up(&dhd->sdsem);
	else
		spin_unlock_bh(&dhd->sdlock);
#else
	spin_unlock_bh(&dhd->sdlock);
#endif /* BCMDBUS */
}

void
dhd_os_sdlock_txq(dhd_pub_t *pub)
{
	dhd_info_t *dhd;

	dhd = (dhd_info_t *)(pub->info);
#ifdef BCMDBUS
	spin_lock_irqsave(&dhd->txqlock, dhd->txqlock_flags);
#else
	spin_lock_bh(&dhd->txqlock);
#endif
}

void
dhd_os_sdunlock_txq(dhd_pub_t *pub)
{
	dhd_info_t *dhd;

	dhd = (dhd_info_t *)(pub->info);
#ifdef BCMDBUS
	spin_unlock_irqrestore(&dhd->txqlock, dhd->txqlock_flags);
#else
	spin_unlock_bh(&dhd->txqlock);
#endif
}

void
dhd_os_sdlock_rxq(dhd_pub_t *pub)
{
}

void
dhd_os_sdunlock_rxq(dhd_pub_t *pub)
{
}

static void
dhd_os_rxflock(dhd_pub_t *pub)
{
	dhd_info_t *dhd;

	dhd = (dhd_info_t *)(pub->info);
	spin_lock_bh(&dhd->rxf_lock);

}

static void
dhd_os_rxfunlock(dhd_pub_t *pub)
{
	dhd_info_t *dhd;

	dhd = (dhd_info_t *)(pub->info);
	spin_unlock_bh(&dhd->rxf_lock);
}

#ifdef DHDTCPACK_SUPPRESS
unsigned long
dhd_os_tcpacklock(dhd_pub_t *pub)
{
	dhd_info_t *dhd;
	unsigned long flags = 0;

	dhd = (dhd_info_t *)(pub->info);

	if (dhd) {
		spin_lock_irqsave(&dhd->tcpack_lock, flags);
	}

	return flags;
}

void
dhd_os_tcpackunlock(dhd_pub_t *pub, unsigned long flags)
{
	dhd_info_t *dhd;

	dhd = (dhd_info_t *)(pub->info);

	if (dhd) {
		spin_unlock_irqrestore(&dhd->tcpack_lock, flags);
	}
}
#endif /* DHDTCPACK_SUPPRESS */

uint8*
dhd_os_prealloc(dhd_pub_t *dhdpub, int section, uint size, bool kmalloc_if_fail)
{
	uint8* buf;
	gfp_t flags = CAN_SLEEP() ? GFP_KERNEL: GFP_ATOMIC;

	buf = (uint8*)wifi_platform_prealloc(dhdpub->info->adapter, section, size);
	if (buf == NULL && kmalloc_if_fail)
		buf = kmalloc(size, flags);

	return buf;
}

void
dhd_os_prefree(dhd_pub_t *dhdpub, void *addr, uint size)
{
}

#if defined(WL_WIRELESS_EXT)
struct iw_statistics *
dhd_get_wireless_stats(struct net_device *dev)
{
	int res = 0;
	dhd_info_t *dhd = DHD_DEV_INFO(dev);

	if (!dhd->pub.up) {
		return NULL;
	}

	res = wl_iw_get_wireless_stats(dev, &dhd->iw.wstats);

	if (res == 0)
		return &dhd->iw.wstats;
	else
		return NULL;
}
#endif /* defined(WL_WIRELESS_EXT) */

static int
dhd_wl_host_event(dhd_info_t *dhd, int *ifidx, void *pktdata, uint16 pktlen,
	wl_event_msg_t *event, void **data)
{
	int bcmerror = 0;

	ASSERT(dhd != NULL);

	bcmerror = wl_host_event(&dhd->pub, ifidx, pktdata, pktlen, event, data, NULL);

	if (bcmerror != BCME_OK)
		return (bcmerror);

#if defined(WL_WIRELESS_EXT)
	if (event->bsscfgidx == 0) {
		/*
		 * Wireless ext is on primary interface only
		 */

	ASSERT(dhd->iflist[*ifidx] != NULL);
	ASSERT(dhd->iflist[*ifidx]->net != NULL);

		if (dhd->iflist[*ifidx]->net) {
		wl_iw_event(dhd->iflist[*ifidx]->net, event, *data);
		}
	}
#endif /* defined(WL_WIRELESS_EXT)  */

#ifdef WL_CFG80211
	ASSERT(dhd->iflist[*ifidx] != NULL);
	ASSERT(dhd->iflist[*ifidx]->net != NULL);
#ifdef WL_HAPD_WDS
	if (dhd->iflist[*ifidx]->net &&
		dhd->iflist[*ifidx]->wds_role == WLC_E_IF_FLAGS_WDS_AP) {
			wl_cfg80211_event(dhd_linux_get_primary_netdev(&dhd->pub), event, *data);
		}
		else
#endif /* WL_HAPD_WDS */
		if (dhd->iflist[*ifidx]->net)
			wl_cfg80211_event(dhd->iflist[*ifidx]->net, event, *data);
#endif /* defined(WL_CFG80211) */

	return (bcmerror);
}

/* send up locally generated event */
void
dhd_sendup_event(dhd_pub_t *dhdp, wl_event_msg_t *event, void *data)
{
	switch (ntoh32(event->event_type)) {
#ifdef WLBTAMP
	/* Send up locally generated AMP HCI Events */
	case WLC_E_BTA_HCI_EVENT: {
		struct sk_buff *p, *skb;
		bcm_event_t *msg;
		wl_event_msg_t *p_bcm_event;
		char *ptr;
		uint32 len;
		uint32 pktlen;
		dhd_if_t *ifp;
		dhd_info_t *dhd;
		uchar *eth;
		int ifidx;

		len = ntoh32(event->datalen);
		pktlen = sizeof(bcm_event_t) + len + 2;
		dhd = dhdp->info;
		ifidx = dhd_ifname2idx(dhd, event->ifname);

		if ((p = PKTGET(dhdp->osh, pktlen, FALSE))) {
			ASSERT(ISALIGNED((uintptr)PKTDATA(dhdp->osh, p), sizeof(uint32)));

			msg = (bcm_event_t *) PKTDATA(dhdp->osh, p);

			bcopy(&dhdp->mac, &msg->eth.ether_dhost, ETHER_ADDR_LEN);
			bcopy(&dhdp->mac, &msg->eth.ether_shost, ETHER_ADDR_LEN);
			ETHER_TOGGLE_LOCALADDR(&msg->eth.ether_shost);

			msg->eth.ether_type = hton16(ETHER_TYPE_BRCM);

			/* BCM Vendor specific header... */
			msg->bcm_hdr.subtype = hton16(BCMILCP_SUBTYPE_VENDOR_LONG);
			msg->bcm_hdr.version = BCMILCP_BCM_SUBTYPEHDR_VERSION;
			bcopy(BRCM_OUI, &msg->bcm_hdr.oui[0], DOT11_OUI_LEN);

			/* vendor spec header length + pvt data length (private indication
			 *  hdr + actual message itself)
			 */
			msg->bcm_hdr.length = hton16(BCMILCP_BCM_SUBTYPEHDR_MINLENGTH +
				BCM_MSG_LEN + sizeof(wl_event_msg_t) + (uint16)len);
			msg->bcm_hdr.usr_subtype = hton16(BCMILCP_BCM_SUBTYPE_EVENT);

			PKTSETLEN(dhdp->osh, p, (sizeof(bcm_event_t) + len + 2));

			/* copy  wl_event_msg_t into sk_buf */

			/* pointer to wl_event_msg_t in sk_buf */
			p_bcm_event = &msg->event;
			bcopy(event, p_bcm_event, sizeof(wl_event_msg_t));

			/* copy hci event into sk_buf */
			bcopy(data, (p_bcm_event + 1), len);

			msg->bcm_hdr.length  = hton16(sizeof(wl_event_msg_t) +
				ntoh16(msg->bcm_hdr.length));
			PKTSETLEN(dhdp->osh, p, (sizeof(bcm_event_t) + len + 2));

			ptr = (char *)(msg + 1);
			/* Last 2 bytes of the message are 0x00 0x00 to signal that there
			 * are no ethertypes which are following this
			 */
			ptr[len+0] = 0x00;
			ptr[len+1] = 0x00;

			skb = PKTTONATIVE(dhdp->osh, p);
			eth = skb->data;
			len = skb->len;

			ifp = dhd->iflist[ifidx];
			if (ifp == NULL)
			     ifp = dhd->iflist[0];

			ASSERT(ifp);
			skb->dev = ifp->net;
			skb->protocol = eth_type_trans(skb, skb->dev);

			skb->data = eth;
			skb->len = len;

			/* Strip header, count, deliver upward */
			skb_pull(skb, ETH_HLEN);

			/* Send the packet */
			bcm_object_trace_opr(skb, BCM_OBJDBG_REMOVE,
				__FUNCTION__, __LINE__);
			if (in_interrupt()) {
				netif_rx(skb);
			} else {
				netif_rx_ni(skb);
			}
		}
		else {
			/* Could not allocate a sk_buf */
			DHD_ERROR(("%s: unable to alloc sk_buf", __FUNCTION__));
		}
		break;
	} /* case WLC_E_BTA_HCI_EVENT */
#endif /* WLBTAMP */

	default:
		break;
	}
}

#ifdef LOG_INTO_TCPDUMP
void
dhd_sendup_log(dhd_pub_t *dhdp, void *data, int data_len)
{
	struct sk_buff *p, *skb;
	uint32 pktlen;
	int len;
	dhd_if_t *ifp;
	dhd_info_t *dhd;
	uchar *skb_data;
	int ifidx = 0;
	struct ether_header eth;

	pktlen = sizeof(eth) + data_len;
	dhd = dhdp->info;

	if ((p = PKTGET(dhdp->osh, pktlen, FALSE))) {
		ASSERT(ISALIGNED((uintptr)PKTDATA(dhdp->osh, p), sizeof(uint32)));

		bcopy(&dhdp->mac, &eth.ether_dhost, ETHER_ADDR_LEN);
		bcopy(&dhdp->mac, &eth.ether_shost, ETHER_ADDR_LEN);
		ETHER_TOGGLE_LOCALADDR(&eth.ether_shost);
		eth.ether_type = hton16(ETHER_TYPE_BRCM);

		bcopy((void *)&eth, PKTDATA(dhdp->osh, p), sizeof(eth));
		bcopy(data, PKTDATA(dhdp->osh, p) + sizeof(eth), data_len);
		skb = PKTTONATIVE(dhdp->osh, p);
		skb_data = skb->data;
		len = skb->len;

		ifidx = dhd_ifname2idx(dhd, "wlan0");
		ifp = dhd->iflist[ifidx];
		if (ifp == NULL)
			 ifp = dhd->iflist[0];

		ASSERT(ifp);
		skb->dev = ifp->net;
		skb->protocol = eth_type_trans(skb, skb->dev);
		skb->data = skb_data;
		skb->len = len;

		/* Strip header, count, deliver upward */
		skb_pull(skb, ETH_HLEN);

		bcm_object_trace_opr(skb, BCM_OBJDBG_REMOVE,
			__FUNCTION__, __LINE__);
		/* Send the packet */
		if (in_interrupt()) {
			netif_rx(skb);
		} else {
			netif_rx_ni(skb);
		}
	}
	else {
		/* Could not allocate a sk_buf */
		DHD_ERROR(("%s: unable to alloc sk_buf", __FUNCTION__));
	}
}
#endif /* LOG_INTO_TCPDUMP */

void
dhd_wait_for_event(dhd_pub_t *dhd, bool *lockvar)
{
#if 0 && (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
	struct dhd_info *dhdinfo =  dhd->info;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27))
	int timeout = msecs_to_jiffies(IOCTL_RESP_TIMEOUT);
#else
	int timeout = (IOCTL_RESP_TIMEOUT / 1000) * HZ;
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)) */

	dhd_os_sdunlock(dhd);
	wait_event_timeout(dhdinfo->ctrl_wait, (*lockvar == FALSE), timeout);
	dhd_os_sdlock(dhd);
#endif

	return;
}

void
dhd_wait_event_wakeup(dhd_pub_t *dhd)
{
#if 0 && (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
	struct dhd_info *dhdinfo =  dhd->info;
	if (waitqueue_active(&dhdinfo->ctrl_wait))
		wake_up(&dhdinfo->ctrl_wait);
#endif

	return;
}

#if defined(BCMPCIE)
int
dhd_net_bus_devreset(struct net_device *dev, uint8 flag)
{
	int ret;

	dhd_info_t *dhd = DHD_DEV_INFO(dev);

	if (flag == TRUE) {
		/* Issue wl down command before resetting the chip */
		if (dhd_wl_ioctl_cmd(&dhd->pub, WLC_DOWN, NULL, 0, TRUE, 0) < 0) {
			DHD_TRACE(("%s: wl down failed\n", __FUNCTION__));
		}
#ifdef PROP_TXSTATUS
		if (dhd->pub.wlfc_enabled)
			dhd_wlfc_deinit(&dhd->pub);
#endif /* PROP_TXSTATUS */
#ifdef PNO_SUPPORT
	if (dhd->pub.pno_state)
		dhd_pno_deinit(&dhd->pub);
#endif
	}

	ret = dhd_bus_devreset(&dhd->pub, flag);
	if (ret) {
		DHD_ERROR(("%s: dhd_bus_devreset: %d\n", __FUNCTION__, ret));
		return ret;
	}

	return ret;
}

#endif

int
net_os_set_suspend_disable(struct net_device *dev, int val)
{
	dhd_info_t *dhd = DHD_DEV_INFO(dev);
	int ret = 0;

	if (dhd) {
		ret = dhd->pub.suspend_disable_flag;
		dhd->pub.suspend_disable_flag = val;
	}

	return ret;
}

int
net_os_set_suspend(struct net_device *dev, int val, int force)
{
	int ret = 0;
	dhd_info_t *dhd = DHD_DEV_INFO(dev);

	if (dhd) {
#if defined(CONFIG_HAS_EARLYSUSPEND) && defined(DHD_USE_EARLYSUSPEND)
		ret = dhd_set_suspend(val, &dhd->pub);
#else
		ret = dhd_suspend_resume_helper(dhd, val, force);
#endif
#ifdef WL_CFG80211
		wl_cfg80211_update_power_mode(dev);
#endif
	}

	return ret;
}

int
net_os_set_suspend_bcn_li_dtim(struct net_device *dev, int val)
{
	dhd_info_t *dhd = DHD_DEV_INFO(dev);

	if (dhd)
		dhd->pub.suspend_bcn_li_dtim = val;

	return 0;
}

#ifdef PKT_FILTER_SUPPORT
int
net_os_rxfilter_add_remove(struct net_device *dev, int add_remove, int num)
{
#if defined(CUSTOMER_HW4) && defined(GAN_LITE_NAT_KEEPALIVE_FILTER)
	return 0;
#else
	dhd_info_t *dhd = DHD_DEV_INFO(dev);
	dhd_pub_t *dhdp = &dhd->pub;
	char *filterp = NULL;
	int filter_id = 0;
	int ret = 0;

	if (!dhd || (num == DHD_UNICAST_FILTER_NUM) ||
		(num == DHD_MDNS_FILTER_NUM))
		return ret;
	if (num >= dhdp->wlcore->pktfilter_count)
		return -EINVAL;
	switch (num) {
		case DHD_BROADCAST_FILTER_NUM:
			filterp = "101 0 0 0 0xFFFFFFFFFFFF 0xFFFFFFFFFFFF";
			filter_id = 101;
			break;
		case DHD_MULTICAST4_FILTER_NUM:
			filterp = "102 0 0 0 0xFFFFFF 0x01005E";
			filter_id = 102;
			break;
		case DHD_MULTICAST6_FILTER_NUM:
#if defined(BLOCK_IPV6_PACKET) && defined(CUSTOMER_HW4)
			/* customer want to use NO IPV6 packets only */
			return ret;
#else
			filterp = "103 0 0 0 0xFFFF 0x3333";
			filter_id = 103;
			break;
#endif /* BLOCK_IPV6_PACKET && CUSTOMER_HW4 */
		default:
			return -EINVAL;
	}

	/* Add filter */
	if (add_remove) {
		dhd->pub.wlcore->pktfilter[num] = filterp;
		dhd_pktfilter_offload_set(&dhd->pub, dhd->pub.wlcore->pktfilter[num]);
	} else { /* Delete filter */
			if (dhd->pub.wlcore->pktfilter[num] != NULL) {
				dhd_pktfilter_offload_delete(&dhd->pub, filter_id);
				dhd->pub.wlcore->pktfilter[num] = NULL;
			}
	}

	return ret;
#endif /* CUSTOMER_HW4 && GAN_LITE_NAT_KEEPALIVE_FILTER */
}

/* XXX RB:4238 Change net_os_set_packet_filter() function name to net_os_enable_packet_filter()
 * previous code do 'set' & 'enable' in one fucntion.
 * but from now on, we are going to separate 'set' and 'enable' feature.
 *  - set : net_os_rxfilter_add_remove() -> dhd_set_packet_filter() -> dhd_pktfilter_offload_set()
 *  - enable : net_os_enable_packet_filter() -> dhd_enable_packet_filter()
 *                                                              -> dhd_pktfilter_offload_enable()
 */
int
dhd_os_enable_packet_filter(dhd_pub_t *dhdp, int val)

{
	int ret = 0;

	/* Packet filtering is set only if we still in early-suspend and
	 * we need either to turn it ON or turn it OFF
	 * We can always turn it OFF in case of early-suspend, but we turn it
	 * back ON only if suspend_disable_flag was not set
	*/
	if (dhdp && dhdp->up) {
		if (dhdp->in_suspend) {
			if (!val || (val && !dhdp->suspend_disable_flag))
				dhd_enable_packet_filter(val, dhdp);
		}
	}

	return ret;
}

/* function to enable/disable packet for Network device */
int
net_os_enable_packet_filter(struct net_device *dev, int val)
{
	dhd_info_t *dhd = DHD_DEV_INFO(dev);

	return dhd_os_enable_packet_filter(&dhd->pub, val);
}
#endif /* PKT_FILTER_SUPPORT */

int
dhd_dev_init_ioctl(struct net_device *dev)
{
	dhd_info_t *dhd = DHD_DEV_INFO(dev);
	int ret;

	if ((ret = dhd_sync_with_dongle(&dhd->pub)) < 0)
		goto done;

done:
	return ret;
}

int
dhd_dev_get_feature_set(struct net_device *dev)
{
	dhd_info_t *ptr = *(dhd_info_t **)netdev_priv(dev);
	dhd_pub_t *dhd = (&ptr->pub);
	int feature_set = 0;

#ifdef DYNAMIC_SWOOB_DURATION
#ifndef CUSTOM_INTR_WIDTH
#define CUSTOM_INTR_WIDTH 100
	int intr_width = 0;
#endif /* CUSTOM_INTR_WIDTH */
#endif /* DYNAMIC_SWOOB_DURATION */
	if (!dhd)
		return feature_set;

	if (FW_SUPPORTED(dhd->wlcore, sta))
		feature_set |= WIFI_FEATURE_INFRA;
	if (FW_SUPPORTED(dhd->wlcore, dualband))
		feature_set |= WIFI_FEATURE_INFRA_5G;
	if (FW_SUPPORTED(dhd->wlcore, p2p))
		feature_set |= WIFI_FEATURE_P2P;
	if (dhd->wlcore->op_mode & DHD_FLAG_HOSTAP_MODE)
		feature_set |= WIFI_FEATURE_SOFT_AP;
	if (FW_SUPPORTED(dhd->wlcore, tdls))
		feature_set |= WIFI_FEATURE_TDLS;
	if (FW_SUPPORTED(dhd->wlcore, vsdb))
		feature_set |= WIFI_FEATURE_TDLS_OFFCHANNEL;
#ifdef RTT_SUPPORT
	feature_set |= WIFI_FEATURE_D2AP_RTT;
#endif /* RTT_SUPPORT */
#ifdef LINKSTAT_SUPPORT
	feature_set |= WIFI_FEATURE_LINKSTAT;
#endif /* LINKSTAT_SUPPORT */
	/* Supports STA + STA in certain chips */
	/* feature_set |= WIFI_FEATURE_ADDITIONAL_STA; */
#ifdef PNO_SUPPORT
	if (dhd_is_pno_supported(dhd)) {
		feature_set |= WIFI_FEATURE_PNO;
		feature_set |= WIFI_FEATURE_BATCH_SCAN;
#ifdef GSCAN_SUPPORT
		feature_set |= WIFI_FEATURE_GSCAN;
#endif /* GSCAN_SUPPORT */
	}
#endif /* PNO_SUPPORT */
#ifdef WL11U
	feature_set |= WIFI_FEATURE_HOTSPOT;
#endif /* WL11U */

	return feature_set;
}

int
dhd_dev_get_feature_set_matrix(struct net_device *dev, int num)
{
	int feature_set_full;
	int ret = 0;

	feature_set_full = dhd_dev_get_feature_set(dev);

	/* Common feature set for all interface */
	ret = (feature_set_full & WIFI_FEATURE_INFRA) |
		(feature_set_full & WIFI_FEATURE_INFRA_5G) |
		(feature_set_full & WIFI_FEATURE_RSSI_MONITOR) |
		(feature_set_full & WIFI_FEATURE_EPR);

	/* Specific feature group for each interface */
	switch (num) {
		case 0:
			ret |= (feature_set_full & WIFI_FEATURE_P2P) |
				(feature_set_full & WIFI_FEATURE_TDLS) |
				(feature_set_full & WIFI_FEATURE_PNO) |
				(feature_set_full & WIFI_FEATURE_HAL_EPNO) |
				(feature_set_full & WIFI_FEATURE_BATCH_SCAN) |
				(feature_set_full & WIFI_FEATURE_GSCAN) |
				(feature_set_full & WIFI_FEATURE_HOTSPOT) |
				(feature_set_full & WIFI_FEATURE_ADDITIONAL_STA);
			break;

		case 1:
			ret |= (feature_set_full & WIFI_FEATURE_P2P);
			break;

		case 2:
			ret |= (feature_set_full & WIFI_FEATURE_TDLS) |
				(feature_set_full & WIFI_FEATURE_TDLS_OFFCHANNEL);
			break;

		default:
			ret = WIFI_FEATURE_INVALID;
			DHD_ERROR(("%s: Out of index(%d) for get feature set\n",
				__FUNCTION__, num));
			break;
	}

	return ret;
}

#ifdef PNO_SUPPORT

/* Linux wrapper to call common dhd_pno_stop_for_ssid */
int
dhd_dev_pno_stop_for_ssid(struct net_device *dev)
{
	dhd_info_t *dhd = DHD_DEV_INFO(dev);

	return (dhd_pno_stop_for_ssid(&dhd->pub));
}

/* Linux wrapper to call common dhd_pno_set_for_ssid */
int
dhd_dev_pno_set_for_ssid(struct net_device *dev, wlc_ssid_ext_t* ssids_local, int nssid,
	uint16  scan_fr, int pno_repeat, int pno_freq_expo_max, uint16 *channel_list, int nchan)
{
	dhd_info_t *dhd = DHD_DEV_INFO(dev);

	return (dhd_pno_set_for_ssid(&dhd->pub, ssids_local, nssid, scan_fr,
		pno_repeat, pno_freq_expo_max, channel_list, nchan));
}

/* Linux wrapper to call common dhd_pno_enable */
int
dhd_dev_pno_enable(struct net_device *dev, int enable)
{
	dhd_info_t *dhd = DHD_DEV_INFO(dev);

	return (dhd_pno_enable(&dhd->pub, enable));
}

/* Linux wrapper to call common dhd_pno_set_for_hotlist */
int
dhd_dev_pno_set_for_hotlist(struct net_device *dev, wl_pfn_bssid_t *p_pfn_bssid,
	struct dhd_pno_hotlist_params *hotlist_params)
{
	dhd_info_t *dhd = DHD_DEV_INFO(dev);

	return (dhd_pno_set_for_hotlist(&dhd->pub, p_pfn_bssid, hotlist_params));
}

/* Linux wrapper to call common dhd_dev_pno_stop_for_batch */
int
dhd_dev_pno_stop_for_batch(struct net_device *dev)
{
	dhd_info_t *dhd = DHD_DEV_INFO(dev);

	return (dhd_pno_stop_for_batch(&dhd->pub));
}

/* Linux wrapper to call common dhd_dev_pno_set_for_batch */
int
dhd_dev_pno_set_for_batch(struct net_device *dev, struct dhd_pno_batch_params *batch_params)
{
	dhd_info_t *dhd = DHD_DEV_INFO(dev);

	return (dhd_pno_set_for_batch(&dhd->pub, batch_params));
}

/* Linux wrapper to call common dhd_dev_pno_get_for_batch */
int
dhd_dev_pno_get_for_batch(struct net_device *dev, char *buf, int bufsize)
{
	dhd_info_t *dhd = DHD_DEV_INFO(dev);

	return (dhd_pno_get_for_batch(&dhd->pub, buf, bufsize, PNO_STATUS_NORMAL));
}

/* Linux wrapper to call common dhd_pno_set_mac_oui */
int
dhd_dev_pno_set_mac_oui(struct net_device *dev, uint8 *oui)
{
	dhd_info_t *dhd = DHD_DEV_INFO(dev);

	return (dhd_pno_set_mac_oui(&dhd->pub, oui));
}

#endif /* PNO_SUPPORT */

#ifdef GSCAN_SUPPORT
/* Linux wrapper to call common dhd_pno_set_cfg_gscan */
int
dhd_dev_pno_set_cfg_gscan(struct net_device *dev, dhd_pno_gscan_cmd_cfg_t type,
 void *buf, uint8 flush)
{
	dhd_info_t *dhd = *(dhd_info_t **)netdev_priv(dev);

	return (dhd_pno_set_cfg_gscan(&dhd->pub, type, buf, flush));
}

/* Linux wrapper to call common dhd_pno_get_gscan */
void *
dhd_dev_pno_get_gscan(struct net_device *dev, dhd_pno_gscan_cmd_cfg_t type,
                      void *info, uint32 *len)
{
	dhd_info_t *dhd = *(dhd_info_t **)netdev_priv(dev);

	return (dhd_pno_get_gscan(&dhd->pub, type, info, len));
}

/* Linux wrapper to call common dhd_wait_batch_results_complete */
void
dhd_dev_wait_batch_results_complete(struct net_device *dev)
{
	dhd_info_t *dhd = *(dhd_info_t **)netdev_priv(dev);

	return (dhd_wait_batch_results_complete(&dhd->pub));
}

/* Linux wrapper to call common dhd_pno_lock_batch_results */
void
dhd_dev_pno_lock_access_batch_results(struct net_device *dev)
{
	dhd_info_t *dhd = *(dhd_info_t **)netdev_priv(dev);

	return (dhd_pno_lock_batch_results(&dhd->pub));
}

/* Linux wrapper to call common dhd_pno_unlock_batch_results */
void
dhd_dev_pno_unlock_access_batch_results(struct net_device *dev)
{
	dhd_info_t *dhd = *(dhd_info_t **)netdev_priv(dev);

	return (dhd_pno_unlock_batch_results(&dhd->pub));
}

/* Linux wrapper to call common dhd_pno_initiate_gscan_request */
int
dhd_dev_pno_run_gscan(struct net_device *dev, bool run, bool flush)
{
	dhd_info_t *dhd = *(dhd_info_t **)netdev_priv(dev);

	return (dhd_pno_initiate_gscan_request(&dhd->pub, run, flush));
}

/* Linux wrapper to call common dhd_pno_enable_full_scan_result */
int
dhd_dev_pno_enable_full_scan_result(struct net_device *dev, bool real_time_flag)
{
	dhd_info_t *dhd = *(dhd_info_t **)netdev_priv(dev);

	return (dhd_pno_enable_full_scan_result(&dhd->pub, real_time_flag));
}

/* Linux wrapper to call common dhd_handle_hotlist_scan_evt */
void *
dhd_dev_hotlist_scan_event(struct net_device *dev,
      const void  *data, int *send_evt_bytes, hotlist_type_t type)
{
	dhd_info_t *dhd = *(dhd_info_t **)netdev_priv(dev);

	return (dhd_handle_hotlist_scan_evt(&dhd->pub, data, send_evt_bytes, type));
}

/* Linux wrapper to call common dhd_process_full_gscan_result */
void *
dhd_dev_process_full_gscan_result(struct net_device *dev,
const void  *data, uint32 len, int *send_evt_bytes)
{
	dhd_info_t *dhd = *(dhd_info_t **)netdev_priv(dev);

	return (dhd_process_full_gscan_result(&dhd->pub, data, len, send_evt_bytes));
}

void
dhd_dev_gscan_hotlist_cache_cleanup(struct net_device *dev, hotlist_type_t type)
{
	dhd_info_t *dhd = *(dhd_info_t **)netdev_priv(dev);

	dhd_gscan_hotlist_cache_cleanup(&dhd->pub, type);

	return;
}

int
dhd_dev_gscan_batch_cache_cleanup(struct net_device *dev)
{
	dhd_info_t *dhd = *(dhd_info_t **)netdev_priv(dev);

	return (dhd_gscan_batch_cache_cleanup(&dhd->pub));
}

/* Linux wrapper to call common dhd_retreive_batch_scan_results */
int
dhd_dev_retrieve_batch_scan(struct net_device *dev)
{
	dhd_info_t *dhd = *(dhd_info_t **)netdev_priv(dev);

	return (dhd_retreive_batch_scan_results(&dhd->pub));
}

#endif /* GSCAN_SUPPORT */

int
dhd_dev_cfg_rand_mac_oui(struct net_device *dev, uint8 *oui)
{
	dhd_info_t *dhd = *(dhd_info_t **)netdev_priv(dev);
	dhd_pub_t *dhdp = &dhd->pub;

	if (!dhdp || !oui) {
		DHD_ERROR(("NULL POINTER : %s\n",
				__FUNCTION__));
		return BCME_ERROR;
	}
	if (ETHER_ISMULTI(oui)) {
		DHD_ERROR(("Expected unicast OUI\n"));
		return BCME_ERROR;
	} else {
		uint8 *rand_mac_oui = dhdp->rand_mac_oui;
		memcpy(rand_mac_oui, oui, DOT11_OUI_LEN);
		DHD_ERROR(("Random MAC OUI to be used - %02x:%02x:%02x\n", rand_mac_oui[0],
				rand_mac_oui[1], rand_mac_oui[2]));
	}

	return BCME_OK;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)) && (defined(OEM_ANDROID))
static void
dhd_hang_process(void *dhd_info, void *event_info, enum dhd_wq_event event)
{
	dhd_info_t *dhd;
	struct net_device *dev;

	dhd = (dhd_info_t *)dhd_info;
	dev = dhd->iflist[0]->net;

	if (dev) {
#if !defined(CUSTOMER_HW4)
		rtnl_lock();
		dev_close(dev);
		rtnl_unlock();
#endif /* !defined(CUSTOMER_HW4) */
#if defined(WL_WIRELESS_EXT)
		wl_iw_send_priv_event(dev, "HANG");
#endif
#if defined(WL_CFG80211)
		wl_cfg80211_hang(dev, WLAN_REASON_UNSPECIFIED);
#endif
	}
}

#ifdef CUSTOMER_HW4
#ifdef SUPPORT_LINKDOWN_RECOVERY
#if defined(CONFIG_MACH_UNIVERSAL5433) || defined(CONFIG_MACH_UNIVERSAL7420)
extern dhd_pub_t *link_recovery;
void dhd_host_recover_link(void)
{
#if defined(BCMPCIE)
	dhd_os_send_hang_message(link_recovery);
#endif /* BCMPCIE */
}
EXPORT_SYMBOL(dhd_host_recover_link);
#endif /* CONFIG_MACH_UNIVERSAL5433 || CONFIG_MACH_UNIVERSAL7420 */
#endif /* SUPPORT_LINKDOWN_RECOVERY */
#endif /* CUSTOMER_HW4 */

int
dhd_os_send_hang_message(dhd_pub_t *dhdp)
{
	int ret = 0;
	if (dhdp) {
		if (!dhdp->hang_was_sent) {
			dhdp->hang_was_sent = 1;
			dhd_deferred_schedule_work(dhdp->info->dhd_deferred_wq, dhdp,
				DHD_WQ_WORK_HANG_MSG, dhd_hang_process, DHD_WORK_PRIORITY_HIGH);
		}
	}

	return ret;
}

int
net_os_send_hang_message(struct net_device *dev)
{
	dhd_info_t *dhd = DHD_DEV_INFO(dev);
	int ret = 0;

	if (dhd) {
		/* Report FW problem when enabled */
		if (dhd->pub.hang_report && dhd_fw_hang_sendup) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27))
			ret = dhd_os_send_hang_message(&dhd->pub);
#else
			ret = wl_cfg80211_hang(dev, WLAN_REASON_UNSPECIFIED);
#endif
		} else {
			DHD_ERROR(("%s: FW HANG ignored (for testing purpose) and not sent up\n",
				__FUNCTION__));
			/* Enforce bus down to stop any future traffic */
			dhd->pub.busstate = DHD_BUS_DOWN;
		}
	}

	return ret;
}
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27) && OEM_ANDROID */

int
dhd_net_wifi_platform_set_power(struct net_device *dev, bool on, unsigned long delay_msec)
{
	dhd_info_t *dhd = DHD_DEV_INFO(dev);

	return wifi_platform_set_power(dhd->adapter, on, delay_msec);
}

void
dhd_get_customized_country_code(struct net_device *dev, char *country_iso_code,
	wl_country_t *cspec)
{
	dhd_info_t *dhd = DHD_DEV_INFO(dev);
	get_customized_country_code(dhd->adapter, country_iso_code, cspec);

#ifdef KEEP_JP_REGREV
	if (strncmp(country_iso_code, "JP", 3) == 0 && strncmp(dhd->pub.vars_ccode, "JP", 3) == 0) {
		cspec->rev = dhd->pub.vars_regrev;
	}
#endif /* KEEP_JP_REGREV */
}

void
dhd_bus_country_set(struct net_device *dev, wl_country_t *cspec, bool notify)
{
	dhd_info_t *dhd = DHD_DEV_INFO(dev);
#ifdef WL_CFG80211
	struct bcm_cfg80211 *cfg = wl_get_cfg(dev);
#endif
	if (dhd && dhd->pub.up) {
		memcpy(&dhd->pub.dhd_cspec, cspec, sizeof(wl_country_t));
#ifdef WL_CFG80211
		wl_update_wiphybands(cfg, notify);
#endif
	}
}

void
dhd_bus_band_set(struct net_device *dev, uint band)
{
	dhd_info_t *dhd = DHD_DEV_INFO(dev);
#ifdef WL_CFG80211
	struct bcm_cfg80211 *cfg = wl_get_cfg(dev);
#endif
	if (dhd && dhd->pub.up) {
#ifdef WL_CFG80211
		wl_update_wiphybands(cfg, true);
#endif
	}
}

int
dhd_net_set_fw_path(struct net_device *dev, char *fw)
{
	dhd_info_t *dhd = DHD_DEV_INFO(dev);

	if (!fw || fw[0] == '\0')
		return -EINVAL;

	strncpy(dhd->fw_path, fw, sizeof(dhd->fw_path) - 1);
	dhd->fw_path[sizeof(dhd->fw_path)-1] = '\0';

#if defined(OEM_ANDROID) && defined(SOFTAP)
	if (strstr(fw, "apsta") != NULL) {
		DHD_INFO(("GOT APSTA FIRMWARE\n"));
		ap_fw_loaded = TRUE;
	} else {
		DHD_INFO(("GOT STA FIRMWARE\n"));
		ap_fw_loaded = FALSE;
	}
#endif /* defined(OEM_ANDROID) && defined(SOFTAP) */

	return 0;
}

void dhd_net_if_lock(struct net_device *dev)
{
	dhd_info_t *dhd = DHD_DEV_INFO(dev);
	dhd_net_if_lock_local(dhd);
}

void
dhd_net_if_unlock(struct net_device *dev)
{
	dhd_info_t *dhd = DHD_DEV_INFO(dev);
	dhd_net_if_unlock_local(dhd);
}

static void
dhd_net_if_lock_local(dhd_info_t *dhd)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 25)) && defined(OEM_ANDROID)
	if (dhd)
		mutex_lock(&dhd->dhd_net_if_mutex);
#endif
}

static void
dhd_net_if_unlock_local(dhd_info_t *dhd)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 25)) && defined(OEM_ANDROID)
	if (dhd)
		mutex_unlock(&dhd->dhd_net_if_mutex);
#endif
}

static void
dhd_suspend_lock(dhd_pub_t *pub)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 25)) && defined(OEM_ANDROID)
	dhd_info_t *dhd = (dhd_info_t *)(pub->info);
	if (dhd)
		mutex_lock(&dhd->dhd_suspend_mutex);
#endif
}

static void
dhd_suspend_unlock(dhd_pub_t *pub)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 25)) && defined(OEM_ANDROID)
	dhd_info_t *dhd = (dhd_info_t *)(pub->info);
	if (dhd)
		mutex_unlock(&dhd->dhd_suspend_mutex);
#endif
}

unsigned long
dhd_os_general_spin_lock(dhd_pub_t *pub)
{
	dhd_info_t *dhd = (dhd_info_t *)(pub->info);
	unsigned long flags = 0;

	if (dhd)
		spin_lock_irqsave(&dhd->dhd_lock, flags);

	return flags;
}

void
dhd_os_general_spin_unlock(dhd_pub_t *pub, unsigned long flags)
{
	dhd_info_t *dhd = (dhd_info_t *)(pub->info);

	if (dhd)
		spin_unlock_irqrestore(&dhd->dhd_lock, flags);
}

/* Linux specific multipurpose spinlock API */

void *
dhd_os_spin_lock_init(osl_t *osh)
{
	/* Adding 4 bytes since the sizeof(spinlock_t) could be 0 */
	/* if CONFIG_SMP and CONFIG_DEBUG_SPINLOCK are not defined */
	/* and this results in kernel asserts in internal builds */
	spinlock_t * lock = osl_malloc(osh, sizeof(spinlock_t) + 4);

	if (lock)
		spin_lock_init(lock);

	return ((void *)lock);
}

void
dhd_os_spin_lock_deinit(osl_t *osh, void *lock)
{
	if (lock)
		osl_mfree(osh, lock, sizeof(spinlock_t) + 4);
}

unsigned long
dhd_os_spin_lock(void *lock)
{
	unsigned long flags = 0;

	if (lock)
		spin_lock_irqsave((spinlock_t *)lock, flags);

	return flags;
}

void
dhd_os_spin_unlock(void *lock, unsigned long flags)
{
	if (lock)
		spin_unlock_irqrestore((spinlock_t *)lock, flags);
}

static int
dhd_get_pend_8021x_cnt(dhd_info_t *dhd)
{
	return (atomic_read(&dhd->pend_8021x_cnt));
}

#define MAX_WAIT_FOR_8021X_TX	100

#if defined(BCMDONGLEHOST)
/* wrapper to invoke dhd_wait_pend8021x() with a DHD lock
 * to avoid crash.
 */
int
dhd_locked_wait_pend8021x(struct net_device *dev)
{
	int pend;
	dhd_info_t *dhd = DHD_DEV_INFO(dev);

	BCM_REFERENCE(dhd);

	DHD_LOCK(&dhd->pub);
	pend = dhd_wait_pend8021x(dev);
	DHD_UNLOCK(&dhd->pub);

	return pend;
}
#endif /* BCMDONGLEHOST */

int
dhd_wait_pend8021x(struct net_device *dev)
{
	dhd_info_t *dhd = DHD_DEV_INFO(dev);
	int timeout = msecs_to_jiffies(10);
	int ntimes = MAX_WAIT_FOR_8021X_TX;
	int pend = dhd_get_pend_8021x_cnt(dhd);

	while (ntimes && pend) {
		if (pend) {
			set_current_state(TASK_INTERRUPTIBLE);
			DHD_UNLOCK(&dhd->pub);
			schedule_timeout(timeout);
			DHD_LOCK(&dhd->pub);
			set_current_state(TASK_RUNNING);
			ntimes--;
		}
		pend = dhd_get_pend_8021x_cnt(dhd);
	}

	if (ntimes == 0)
	{
		atomic_set(&dhd->pend_8021x_cnt, 0);
		DHD_ERROR(("%s: TIMEOUT\n", __FUNCTION__));
	}

	return pend;
}

static struct file*
open_file(const char *file_name, uint32 flags)
{
	struct file *fp = NULL;

	/* open file to write */
	fp = filp_open(file_name, flags, 0664);
	if (IS_ERR(fp)) {
		printf("%s: open file error, err = %ld\n", __FUNCTION__, PTR_ERR(fp));
		fp = NULL;
	}
	return fp;
} /* open_file */

/**
 * @param[inout] pos  Position in file, updated on return.
 * @return BCME_* status code
 */
static int
append_to_file(struct file *fp, uint8 *buf, int size, loff_t *pos)
{
	int ret = BCME_OK;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
	ret = kernel_write(fp, buf, size, pos);
	if (ret < 0) {
		DHD_ERROR(("write file error, err = %d\n", ret));
	} else {
		ret = BCME_OK;
	}
#else
	fp->f_op->write(fp, buf, size, pos);
#endif

	return ret;
} /* append_to_file */

/** Invoked for e.g. socsram dumps, mac dumps */
static int
write_file(const char * file_name, uint32 flags, uint8 *buf, int size)
{
	int ret = 0;
	struct file *fp = NULL;
	loff_t pos = 0;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0))
	/* change to KERNEL_DS address limit */
	mm_segment_t old_fs = get_fs();

	set_fs(KERNEL_DS);
#endif
	/* open file to write */
	fp = open_file(file_name, flags);
	if (fp == NULL) {
		ret = -1;
		goto exit;
	}

	/* Write buf to file */
	ret = append_to_file(fp, buf, size, &pos);

	/* close file before return */
	filp_close(fp, current->files);

exit:
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0))
	/* restore previous address limit */
	set_fs(old_fs);
#endif
	return ret;
} /* write_file */

#ifdef DHD_DEBUG
/**
 * A large buffer was allocated and filled with socsram contents in interrupt context. Because file
 * operations are not permitted at interrupt context, this function is invoked at a lower execution
 * context. It writes the buffer to a file.
 */
int
socsram_write_to_file(dhd_pub_t *dhd, uint8 *buf, int size)
{
	int ret = 0;

	ret = write_file(SOCRAM_DUMP_FILE_NAME, SOCRAM_DUMP_FILE_FLAGS, buf, size);

	/* free buf before return */
#if defined(CONFIG_DHD_USE_STATIC_BUF) && defined(DHD_USE_STATIC_MEMDUMP)
	DHD_OS_PREFREE(dhd, buf, size);
#else
	MFREE(dhd->osh, buf, size);
#endif /* CONFIG_DHD_USE_STATIC_BUF && DHD_USE_STATIC_MEMDUMP */

	return ret;
} /* write_to_file */
#endif /* DHD_DEBUG */

/**
 * A large buffer failed to allocate in interrupt context. This function avoids the need of a large
 * allocation by reading from dongle and writing to file in small chunks. This function must be
 * invoked at a process/thread execution context since it performs operations on files.
 */
static void
socsram_write_to_file_no_large_buf(dhd_info_t *dhd)
{
	dhd_pub_t *pub = &dhd->pub;
	uint32 memsize;
	struct {
		uint32 bp_address;
		uint32 size;
	} m;
	struct file *fp = NULL;
	loff_t pos = 0;
	uint8 *buf = NULL;
	char dumpfilename[DUMP_FILENAME_SZ] = {0};
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0))
	mm_segment_t old_fs = get_fs(); /* change to KERNEL_DS address limit */

	set_fs(KERNEL_DS);
#endif
	if (BUS_IOVAR_OP(pub, "memsize", NULL, 0, &memsize, sizeof(memsize), FALSE) < 0) {
		goto the_end;
	}

	if (BUS_IOVAR_OP(pub, "ramstart", NULL, 0, &m.bp_address,
		sizeof(m.bp_address), FALSE) < 0) {
		goto the_end;
	}

	buf = MALLOC(pub->osh, MEMBLOCK);
	if (buf == NULL) {
		goto the_end;
	}

#if defined(CUSTOMER_HW4) || defined(OEM_ANDROID) || defined(BCM_ROUTER_DHD)
	snprintf(dumpfilename, DUMP_FILENAME_SZ, "%s", SOCRAM_DUMP_FILE_NAME);
#else
	snprintf(dumpfilename, DUMP_FILENAME_SZ, "%s_%04x", SOCRAM_DUMP_FILE_NAME,
		dhd->dump_signature);
#endif /* CUSTORMER_HW4 || OEM_ANDROID || BCM_ROUTER_DHD */

	fp = open_file(dumpfilename, SOCRAM_DUMP_FILE_FLAGS);
	if (fp == NULL) {
		goto the_end;
	}

	while (memsize)	{
		m.size = MIN(MEMBLOCK, memsize);
		if (BUS_IOVAR_OP(pub, "membytes", &m, sizeof(m), buf, MEMBLOCK, FALSE) < 0) {
			printf("%s %d membytes FAILED\n", __FUNCTION__, __LINE__);
			goto the_end;
		}
		if (append_to_file(fp, buf, m.size, &pos) != BCME_OK) {
			printf("%s %d append FAILED\n", __FUNCTION__, __LINE__);
		}

		memsize -= m.size;
		m.bp_address += m.size;
	}

the_end:
	DHD_ERROR(("%s: %ssocram dump to %s len %llu bytes\n", __FUNCTION__,
		memsize ? "incomplete ": "", SOCRAM_DUMP_FILE_NAME, pos));

	if (fp != NULL) {
		filp_close(fp, current->files);
	}
	if (buf != NULL) {
		MFREE(pub->osh, buf, MEMBLOCK);
	}
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0))
	set_fs(old_fs);  /* restore previous address limit */
#endif
} /* socsram_write_to_file_no_large_buf */

int
dhd_os_wake_lock_timeout(dhd_pub_t *pub)
{
	dhd_info_t *dhd = (dhd_info_t *)(pub->info);
	unsigned long flags;
	int ret = 0;

	if (dhd) {
		spin_lock_irqsave(&dhd->wakelock_spinlock, flags);
		ret = dhd->wakelock_rx_timeout_enable > dhd->wakelock_ctrl_timeout_enable ?
			dhd->wakelock_rx_timeout_enable : dhd->wakelock_ctrl_timeout_enable;
#ifdef CONFIG_HAS_WAKELOCK
		if (dhd->wakelock_rx_timeout_enable)
			wake_lock_timeout(&dhd->wl_rxwake,
				msecs_to_jiffies(dhd->wakelock_rx_timeout_enable));
		if (dhd->wakelock_ctrl_timeout_enable)
			wake_lock_timeout(&dhd->wl_ctrlwake,
				msecs_to_jiffies(dhd->wakelock_ctrl_timeout_enable));
#endif
		dhd->wakelock_rx_timeout_enable = 0;
		dhd->wakelock_ctrl_timeout_enable = 0;
		spin_unlock_irqrestore(&dhd->wakelock_spinlock, flags);
	}

	return ret;
}

int
net_os_wake_lock_timeout(struct net_device *dev)
{
	dhd_info_t *dhd = DHD_DEV_INFO(dev);
	int ret = 0;

	if (dhd)
		ret = dhd_os_wake_lock_timeout(&dhd->pub);

	return ret;
}

int
dhd_os_wake_lock_rx_timeout_enable(dhd_pub_t *pub, int val)
{
	dhd_info_t *dhd = (dhd_info_t *)(pub->info);
	unsigned long flags;

	if (dhd) {
		spin_lock_irqsave(&dhd->wakelock_spinlock, flags);
		if (val > dhd->wakelock_rx_timeout_enable)
			dhd->wakelock_rx_timeout_enable = val;
		spin_unlock_irqrestore(&dhd->wakelock_spinlock, flags);
	}

	return 0;
}

int
dhd_os_wake_lock_ctrl_timeout_enable(dhd_pub_t *pub, int val)
{
	dhd_info_t *dhd = (dhd_info_t *)(pub->info);
	unsigned long flags;

	if (dhd) {
		spin_lock_irqsave(&dhd->wakelock_spinlock, flags);
		if (val > dhd->wakelock_ctrl_timeout_enable)
			dhd->wakelock_ctrl_timeout_enable = val;
		spin_unlock_irqrestore(&dhd->wakelock_spinlock, flags);
	}

	return 0;
}

int
dhd_os_wake_lock_ctrl_timeout_cancel(dhd_pub_t *pub)
{
	dhd_info_t *dhd = (dhd_info_t *)(pub->info);
	unsigned long flags;

	if (dhd) {
		spin_lock_irqsave(&dhd->wakelock_spinlock, flags);
		dhd->wakelock_ctrl_timeout_enable = 0;
#ifdef CONFIG_HAS_WAKELOCK
		if (wake_lock_active(&dhd->wl_ctrlwake))
			wake_unlock(&dhd->wl_ctrlwake);
#endif
		spin_unlock_irqrestore(&dhd->wakelock_spinlock, flags);
	}

	return 0;
}

int
net_os_wake_lock_rx_timeout_enable(struct net_device *dev, int val)
{
	dhd_info_t *dhd = DHD_DEV_INFO(dev);
	int ret = 0;

	if (dhd)
		ret = dhd_os_wake_lock_rx_timeout_enable(&dhd->pub, val);

	return ret;
}

int
net_os_wake_lock_ctrl_timeout_enable(struct net_device *dev, int val)
{
	dhd_info_t *dhd = DHD_DEV_INFO(dev);
	int ret = 0;

	if (dhd)
		ret = dhd_os_wake_lock_ctrl_timeout_enable(&dhd->pub, val);

	return ret;
}

int
dhd_os_wake_lock(dhd_pub_t *pub)
{
	dhd_info_t *dhd = (dhd_info_t *)(pub->info);
	unsigned long flags;
	int ret = 0;

	if (dhd) {
		spin_lock_irqsave(&dhd->wakelock_spinlock, flags);

		if (dhd->wakelock_counter == 0 && !dhd->waive_wakelock) {
#ifdef CONFIG_HAS_WAKELOCK
			wake_lock(&dhd->wl_wifi);
#elif 0 && (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 36))
			dhd_bus_dev_pm_stay_awake(pub);
#endif
		}
		dhd->wakelock_counter++;
		ret = dhd->wakelock_counter;
		spin_unlock_irqrestore(&dhd->wakelock_spinlock, flags);
	}

	return ret;
}

int
net_os_wake_lock(struct net_device *dev)
{
	dhd_info_t *dhd = DHD_DEV_INFO(dev);
	int ret = 0;

	if (dhd)
		ret = dhd_os_wake_lock(&dhd->pub);

	return ret;
}

int
dhd_os_wake_unlock(dhd_pub_t *pub)
{
	dhd_info_t *dhd = (dhd_info_t *)(pub->info);
	unsigned long flags;
	int ret = 0;

	dhd_os_wake_lock_timeout(pub);
	if (dhd) {
		spin_lock_irqsave(&dhd->wakelock_spinlock, flags);
		if (dhd->wakelock_counter > 0) {
			dhd->wakelock_counter--;
			if (dhd->wakelock_counter == 0 && !dhd->waive_wakelock) {
#ifdef CONFIG_HAS_WAKELOCK
				wake_unlock(&dhd->wl_wifi);
#elif 0 && (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 36))
				dhd_bus_dev_pm_relax(pub);
#endif
			}
			ret = dhd->wakelock_counter;
		}
		spin_unlock_irqrestore(&dhd->wakelock_spinlock, flags);
	}

	return ret;
}

int
dhd_os_check_wakelock(dhd_pub_t *pub)
{
#if defined(CONFIG_HAS_WAKELOCK) || (0 && (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, \
	36)))
	dhd_info_t *dhd;

	if (!pub)
		return 0;
	dhd = (dhd_info_t *)(pub->info);
#endif

#ifdef CONFIG_HAS_WAKELOCK
	/* Indicate to the SD Host to avoid going to suspend if internal locks are up */
	if (dhd && (wake_lock_active(&dhd->wl_wifi) ||
		(wake_lock_active(&dhd->wl_wdwake))))
		return 1;
#elif 0 && (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 36))
	if (dhd && (dhd->wakelock_counter > 0) && dhd_bus_dev_pm_enabled(pub))
		return 1;
#endif

	return 0;
}

int
dhd_os_check_wakelock_all(dhd_pub_t *pub)
{
#if defined(CONFIG_HAS_WAKELOCK) || (0 && (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, \
	36)))
	dhd_info_t *dhd;

	if (!pub) {
		return 0;
	}
	dhd = (dhd_info_t *)(pub->info);
	if (!dhd) {
		return 0;
	}
#endif

#ifdef CONFIG_HAS_WAKELOCK
	/* Indicate to the SD Host to avoid going to suspend if internal locks are up */
	if (dhd && (wake_lock_active(&dhd->wl_wifi) ||
		wake_lock_active(&dhd->wl_wdwake) ||
		wake_lock_active(&dhd->wl_rxwake) ||
		wake_lock_active(&dhd->wl_ctrlwake))) {
		DHD_ERROR(("%s wakelock_counter %d\n", __FUNCTION__, dhd->wakelock_counter));
		return 1;
	}
#elif 0 && (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 36))
	if (dhd && (dhd->wakelock_counter > 0) && dhd_bus_dev_pm_enabled(pub)) {
		return 1;
	}
#endif

	return 0;
}

int
net_os_wake_unlock(struct net_device *dev)
{
	dhd_info_t *dhd = DHD_DEV_INFO(dev);
	int ret = 0;

	if (dhd)
		ret = dhd_os_wake_unlock(&dhd->pub);

	return ret;
}

int
dhd_os_wd_wake_lock(dhd_pub_t *pub)
{
	dhd_info_t *dhd = (dhd_info_t *)(pub->info);
	unsigned long flags;
	int ret = 0;

	if (dhd) {
		spin_lock_irqsave(&dhd->wakelock_spinlock, flags);
#ifdef CONFIG_HAS_WAKELOCK
		/* if wakelock_wd_counter was never used : lock it at once */
		if (!dhd->wakelock_wd_counter)
			wake_lock(&dhd->wl_wdwake);
#endif
		dhd->wakelock_wd_counter++;
		ret = dhd->wakelock_wd_counter;
		spin_unlock_irqrestore(&dhd->wakelock_spinlock, flags);
	}

	return ret;
}

int
dhd_os_wd_wake_unlock(dhd_pub_t *pub)
{
	dhd_info_t *dhd = (dhd_info_t *)(pub->info);
	unsigned long flags;
	int ret = 0;

	if (dhd) {
		spin_lock_irqsave(&dhd->wakelock_spinlock, flags);
		if (dhd->wakelock_wd_counter) {
			dhd->wakelock_wd_counter = 0;
#ifdef CONFIG_HAS_WAKELOCK
			wake_unlock(&dhd->wl_wdwake);
#endif
		}
		spin_unlock_irqrestore(&dhd->wakelock_spinlock, flags);
	}

	return ret;
}

#ifdef BCMPCIE_OOB_HOST_WAKE
void
dhd_os_oob_irq_wake_lock_timeout(dhd_pub_t *pub, int val)
{
#ifdef CONFIG_HAS_WAKELOCK
	dhd_info_t *dhd = (dhd_info_t *)(pub->info);

	if (dhd) {
		wake_lock_timeout(&dhd->wl_intrwake, msecs_to_jiffies(val));
	}
#endif /* CONFIG_HAS_WAKELOCK */
}

void
dhd_os_oob_irq_wake_unlock(dhd_pub_t *pub)
{
#ifdef CONFIG_HAS_WAKELOCK
	dhd_info_t *dhd = (dhd_info_t *)(pub->info);

	if (dhd) {
		/* if wl_intrwake is active, unlock it */
		if (wake_lock_active(&dhd->wl_intrwake)) {
			wake_unlock(&dhd->wl_intrwake);
		}
	}
#endif /* CONFIG_HAS_WAKELOCK */
}
#endif /* BCMPCIE_OOB_HOST_WAKE */

/* waive wakelocks for operations such as IOVARs in suspend function, must be closed
 * by a paired function call to dhd_wakelock_restore. returns current wakelock counter
 */
int
dhd_os_wake_lock_waive(dhd_pub_t *pub)
{
	dhd_info_t *dhd = (dhd_info_t *)(pub->info);
	unsigned long flags;
	int ret = 0;

	if (dhd) {
		spin_lock_irqsave(&dhd->wakelock_spinlock, flags);
		/* dhd_wakelock_waive/dhd_wakelock_restore must be paired */
		if (dhd->waive_wakelock == FALSE) {
			/* record current lock status */
			dhd->wakelock_before_waive = dhd->wakelock_counter;
			dhd->waive_wakelock = TRUE;
		}
		ret = dhd->wakelock_wd_counter;
		spin_unlock_irqrestore(&dhd->wakelock_spinlock, flags);
	}

	return ret;
}

int
dhd_os_wake_lock_restore(dhd_pub_t *pub)
{
	dhd_info_t *dhd = (dhd_info_t *)(pub->info);
	unsigned long flags;
	int ret = 0;

	if (!dhd)
		return 0;

	spin_lock_irqsave(&dhd->wakelock_spinlock, flags);
	/* dhd_wakelock_waive/dhd_wakelock_restore must be paired */
	if (!dhd->waive_wakelock)
		goto exit;

	dhd->waive_wakelock = FALSE;
	/* if somebody else acquires wakelock between dhd_wakelock_waive/dhd_wakelock_restore,
	 * we need to make it up by calling wake_lock or pm_stay_awake. or if somebody releases
	 * the lock in between, do the same by calling wake_unlock or pm_relax
	 */
	if (dhd->wakelock_before_waive == 0 && dhd->wakelock_counter > 0) {
#ifdef CONFIG_HAS_WAKELOCK
		wake_lock(&dhd->wl_wifi);
#elif 0 && (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 36))
		dhd_bus_dev_pm_stay_awake(&dhd->pub);
#endif
	} else if (dhd->wakelock_before_waive > 0 && dhd->wakelock_counter == 0) {
#ifdef CONFIG_HAS_WAKELOCK
		wake_unlock(&dhd->wl_wifi);
#elif 0 && (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 36))
		dhd_bus_dev_pm_relax(&dhd->pub);
#endif
	}

	dhd->wakelock_before_waive = 0;

exit:
	ret = dhd->wakelock_wd_counter;
	spin_unlock_irqrestore(&dhd->wakelock_spinlock, flags);

	return ret;
}

bool
dhd_os_check_if_up(dhd_pub_t *pub)
{
	if (!pub)
		return FALSE;

	return pub->up;
}

int
dhd_ioctl_entry_local(struct net_device *net, wl_ioctl_t *ioc, int cmd)
{
	int ifidx;
	int ret = 0;
	dhd_info_t *dhd = NULL;

	if (!net || !DEV_PRIV(net)) {
		DHD_ERROR(("%s invalid parameter\n", __FUNCTION__));
		return -EINVAL;
	}

	dhd = DHD_DEV_INFO(net);
	if (!dhd)
		return -EINVAL;

	if (dhd->pub.busstate != DHD_BUS_DATA)
		return -ENODEV;

	ifidx = dhd_net2idx(dhd, net);
	if (ifidx == DHD_BAD_IF) {
		DHD_ERROR(("%s bad ifidx\n", __FUNCTION__));
		return -ENODEV;
	}

	DHD_OS_WAKE_LOCK(&dhd->pub);
	DHD_LOCK(&dhd->pub);

	ret = dhd_wl_ioctl(&dhd->pub, ifidx, ioc, ioc->buf, ioc->len);
	dhd_check_hang(net, &dhd->pub, ret);

	DHD_UNLOCK(&dhd->pub);
	DHD_OS_WAKE_UNLOCK(&dhd->pub);

	return ret;
}

bool
dhd_os_check_hang(dhd_pub_t *dhdp, int ifidx, int ret)
{
	struct net_device *net;

	net = dhd_idx2net(dhdp, ifidx);
	if (!net) {
		DHD_ERROR(("%s : Invalid index : %d\n", __FUNCTION__, ifidx));
		return -EINVAL;
	}

	return dhd_check_hang(net, dhdp, ret);
}

/* Return instance */
int
dhd_get_instance(dhd_pub_t *dhdp)
{
	return dhdp->unit;
}

#if defined(WL_CFG80211) && defined(SUPPORT_DEEP_SLEEP)
#define MAX_TRY_CNT             5 /* Number of tries to disable deepsleep */
int
dhd_deepsleep(struct net_device *dev, int flag)
{
	char iovbuf[20];
	uint powervar = 0;
	dhd_info_t *dhd;
	dhd_pub_t *dhdp;
	int cnt = 0;
	int ret = 0;

	dhd = DHD_DEV_INFO(dev);
	dhdp = &dhd->pub;

	switch (flag) {
		case 1 :  /* Deepsleep on */
			DHD_ERROR(("[WiFi] Deepsleep On\n"));
			/* give some time to sysioc_work before deepsleep */
			OSL_SLEEP(200);
#ifdef PKT_FILTER_SUPPORT
		/* disable pkt filter */
		dhd_enable_packet_filter(0, dhdp);
#endif /* PKT_FILTER_SUPPORT */
			/* Disable MPC */
			powervar = 0;
			dhd_iovar(dhdp, 0, "mpc", (char *)&powervar, sizeof(powervar), NULL, 0,
					TRUE);

			/* Enable Deepsleep */
			powervar = 1;
			dhd_iovar(dhdp, 0, "deepsleep", (char *)&powervar, sizeof(powervar), NULL,
					0, TRUE);
			break;

		case 0: /* Deepsleep Off */
			DHD_ERROR(("[WiFi] Deepsleep Off\n"));

			/* Disable Deepsleep */
			for (cnt = 0; cnt < MAX_TRY_CNT; cnt++) {
				powervar = 0;
				dhd_iovar(dhdp, 0, "deepsleep", (char *)&powervar, sizeof(powervar),
						NULL, 0, TRUE);

				ret = dhd_iovar(dhdp, 0, "deepsleep", (char *)&powervar,
						sizeof(powervar), iovbuf, sizeof(iovbuf), FALSE);
				if (ret < 0) {
					DHD_ERROR(("the error of dhd deepsleep status"
						" ret value :%d\n", ret));
				} else {
					if (!(*(int *)iovbuf)) {
						DHD_ERROR(("deepsleep mode is 0, "
							" count: %d\n", cnt));
						break;
					}
				}
			}

			/* Enable MPC */
			powervar = 1;
			dhd_iovar(dhdp, 0, "mpc", (char *)&powervar, sizeof(powervar), NULL, 0,
					TRUE);
			break;
	}

	return 0;
}
#endif /* WL_CFG80211 && SUPPORT_DEEP_SLEEP */

#ifdef PROP_TXSTATUS

void
dhd_wlfc_plat_init(void *dhd)
{
#if defined(CUSTOMER_HW4) && defined(USE_DYNAMIC_F2_BLKSIZE)
	dhdsdio_func_blocksize((dhd_pub_t *)dhd, 2, DYNAMIC_F2_BLKSIZE_FOR_NONLEGACY);
#endif /* CUSTOMER_HW4 && USE_DYNAMIC_F2_BLKSIZE */

	return;
}

void
dhd_wlfc_plat_deinit(void *dhd)
{
#if defined(CUSTOMER_HW4) && defined(USE_DYNAMIC_F2_BLKSIZE)
	dhdsdio_func_blocksize((dhd_pub_t *)dhd, 2, sd_f2_blocksize);
#endif /* CUSTOMER_HW4 && USE_DYNAMIC_F2_BLKSIZE */

	return;
}

bool
dhd_wlfc_skip_fc(void * dhdp, uint8 idx)
{
#ifdef CUSTOMER_HW4

#ifdef WL_CFG80211
	struct net_device * net =  dhd_idx2net((dhd_pub_t *)dhdp, idx);
	if (net)
	/* enable flow control in vsdb mode */
		return !(wl_cfg80211_is_vsdb_mode(net));
#else
	return TRUE; /* skip flow control */
#endif /* WL_CFG80211 */

#else
	return FALSE;
#endif /* CUSTOMER_HW4 */

	return FALSE;
}
#endif /* PROP_TXSTATUS */

#ifdef DHD_DEBUGFS
#include <linux/debugfs.h>

/* XXX: Just added new debugfs forcedevclose to already existing code
* And disabled debugfs_mem with BCMDBGFS_MEM, as this was causing
* insmod failure due to bad magic number from dhd_readregl() and
* dhd_writeregl() calls.
*/

typedef struct dhd_dbgfs {
	struct dentry	*debugfs_dir;
#ifdef BCMDBGFS_MEM
	struct dentry	*debugfs_mem;
	dhd_pub_t	*dhdp;
	uint32		size;
#endif
	struct dentry	*debugfs_fw_hang_sendup;
} dhd_dbgfs_t;

dhd_dbgfs_t g_dbgfs;

#ifdef BCMDBGFS_MEM
extern uint32 dhd_readregl(void *bp, uint32 addr);
extern uint32 dhd_writeregl(void *bp, uint32 addr, uint32 data);

static int
dhd_dbg_state_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;

	return 0;
}

static ssize_t
dhd_dbg_state_read(struct file *file, char __user *ubuf,
                       size_t count, loff_t *ppos)
{
	ssize_t rval;
	uint32 tmp;
	loff_t pos = *ppos;
	size_t ret;

	if (pos < 0)
		return -EINVAL;
	if (pos >= g_dbgfs.size || !count)
		return 0;
	if (count > g_dbgfs.size - pos)
		count = g_dbgfs.size - pos;

	/* Basically enforce aligned 4 byte reads. It's up to the user to work out the details */
	tmp = dhd_readregl(g_dbgfs.dhdp->bus, file->f_pos & (~3));

	ret = copy_to_user(ubuf, &tmp, 4);
	if (ret == count)
		return -EFAULT;

	count -= ret;
	*ppos = pos + count;
	rval = count;

	return rval;
}

static ssize_t
dhd_debugfs_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos)
{
	loff_t pos = *ppos;
	size_t ret;
	uint32 buf;

	if (pos < 0)
		return -EINVAL;
	if (pos >= g_dbgfs.size || !count)
		return 0;
	if (count > g_dbgfs.size - pos)
		count = g_dbgfs.size - pos;

	ret = copy_from_user(&buf, ubuf, sizeof(uint32));
	if (ret == count)
		return -EFAULT;

	/* Basically enforce aligned 4 byte writes. It's up to the user to work out the details */
	dhd_writeregl(g_dbgfs.dhdp->bus, file->f_pos & (~3), buf);

	return count;
}

loff_t
dhd_debugfs_lseek(struct file *file, loff_t off, int whence)
{
	loff_t pos = -1;

	switch (whence) {
		case 0:
			pos = off;
			break;
		case 1:
			pos = file->f_pos + off;
			break;
		case 2:
			pos = g_dbgfs.size - off;
	}

	return (pos < 0 || pos > g_dbgfs.size) ? -EINVAL : (file->f_pos = pos);
}

static const struct file_operations dhd_dbg_state_ops = {
	.read   = dhd_dbg_state_read,
	.write	= dhd_debugfs_write,
	.open   = dhd_dbg_state_open,
	.llseek	= dhd_debugfs_lseek
};
#endif /* BCMDBGFS_MEM */

static int
dhd_dbg_fw_hang_sendup_get(void *data, u64 *val)
{
	*val = dhd_fw_hang_sendup;

	return 0;
}

static int
dhd_dbg_fw_hang_sendup_set(void *data, u64 val)
{
	dhd_fw_hang_sendup = val;

	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(dhd_dbg_fw_hang_sendup_ops, dhd_dbg_fw_hang_sendup_get,
	dhd_dbg_fw_hang_sendup_set, "%llu\n");

static void
dhd_dbg_create(void)
{
	if (g_dbgfs.debugfs_dir) {
#ifdef BCMDBGFS_MEM
		g_dbgfs.debugfs_mem = debugfs_create_file("mem", 0644, g_dbgfs.debugfs_dir,
			NULL, &dhd_dbg_state_ops);
#endif
		g_dbgfs.debugfs_fw_hang_sendup = debugfs_create_file("fwhangsendup", 0644,
			g_dbgfs.debugfs_dir, NULL, &dhd_dbg_fw_hang_sendup_ops);
	}
}

int
dhd_dbg_init(dhd_pub_t *dhdp)
{
	int err = 0;

#ifdef BCMDBGFS_MEM
	g_dbgfs.dhdp = dhdp;
	g_dbgfs.size = 0x20000000; /* Allow access to various cores regs */
#endif

	g_dbgfs.debugfs_dir = debugfs_create_dir("dhd", 0);
	if (IS_ERR(g_dbgfs.debugfs_dir)) {
		err = PTR_ERR(g_dbgfs.debugfs_dir);
		g_dbgfs.debugfs_dir = NULL;
		return err;
	}

	dhd_dbg_create();

	return err;
}

void
dhd_dbg_remove(void)
{

#ifdef BCMDBGFS_MEM
	debugfs_remove(g_dbgfs.debugfs_mem);
#endif
	debugfs_remove(g_dbgfs.debugfs_fw_hang_sendup);

	debugfs_remove(g_dbgfs.debugfs_dir);

	bzero((unsigned char *) &g_dbgfs, sizeof(g_dbgfs));
}
#endif /* DHD_DEBUGFS */

#ifdef WLMEDIA_HTSF

static void
dhd_htsf_addtxts(dhd_pub_t *dhdp, void *pktbuf)
{
	dhd_info_t *dhd = (dhd_info_t *)(dhdp->info);
	struct sk_buff *skb;
	uint32 htsf = 0;
	uint16 dport = 0, oldmagic = 0xACAC;
	char *p1;
	htsfts_t ts;

	/*  timestamp packet  */

	p1 = (char*) PKTDATA(dhdp->osh, pktbuf);

	if (PKTLEN(dhdp->osh, pktbuf) > HTSF_MINLEN) {
/*		memcpy(&proto, p1+26, 4);  	*/
		memcpy(&dport, p1+40, 2);
/* 	proto = ((ntoh32(proto))>> 16) & 0xFF;  */
		dport = ntoh16(dport);
	}

	/* timestamp only if  icmp or udb iperf with port 5555 */
/*	if (proto == 17 && dport == tsport) { */
	if (dport >= tsport && dport <= tsport + 20) {

		skb = (struct sk_buff *) pktbuf;

		htsf = dhd_get_htsf(dhd, 0);
		memset(skb->data + 44, 0, 2); /* clear checksum */
		memcpy(skb->data+82, &oldmagic, 2);
		memcpy(skb->data+84, &htsf, 4);

		memset(&ts, 0, sizeof(htsfts_t));
		ts.magic  = HTSFMAGIC;
		ts.prio   = PKTPRIO(pktbuf);
		ts.seqnum = htsf_seqnum++;
		ts.c10    = get_cycles();
		ts.t10    = htsf;
		ts.endmagic = HTSFENDMAGIC;

		memcpy(skb->data + HTSF_HOSTOFFSET, &ts, sizeof(ts));
	}
}

static void
dhd_dump_htsfhisto(histo_t *his, char *s)
{
	int pktcnt = 0, curval = 0, i;
	for (i = 0; i < (NUMBIN-2); i++) {
		curval += 500;
		printf("%d ",  his->bin[i]);
		pktcnt += his->bin[i];
	}
	printf(" max: %d TotPkt: %d neg: %d [%s]\n", his->bin[NUMBIN-2], pktcnt,
		his->bin[NUMBIN-1], s);
}

static void
sorttobin(int value, histo_t *histo)
{
	int i, binval = 0;

	if (value < 0) {
		histo->bin[NUMBIN-1]++;
		return;
	}
	if (value > histo->bin[NUMBIN-2])  /* store the max value  */
		histo->bin[NUMBIN-2] = value;

	for (i = 0; i < (NUMBIN-2); i++) {
		binval += 500; /* 500m s bins */
		if (value <= binval) {
			histo->bin[i]++;
			return;
		}
	}
	histo->bin[NUMBIN-3]++;
}

static void
dhd_htsf_addrxts(dhd_pub_t *dhdp, void *pktbuf)
{
	dhd_info_t *dhd = (dhd_info_t *)dhdp->info;
	struct sk_buff *skb;
	char *p1;
	uint16 old_magic;
	int d1, d2, d3, end2end;
	htsfts_t *htsf_ts;
	uint32 htsf;

	skb = PKTTONATIVE(dhdp->osh, pktbuf);
	p1 = (char*)PKTDATA(dhdp->osh, pktbuf);

	if (PKTLEN(osh, pktbuf) > HTSF_MINLEN) {
		memcpy(&old_magic, p1+78, 2);
		htsf_ts = (htsfts_t*) (p1 + HTSF_HOSTOFFSET - 4);
	} else {
		return;
	}
	if (htsf_ts->magic == HTSFMAGIC) {
		htsf_ts->tE0 = dhd_get_htsf(dhd, 0);
		htsf_ts->cE0 = get_cycles();
	}

	if (old_magic == 0xACAC) {

		tspktcnt++;
		htsf = dhd_get_htsf(dhd, 0);
		memcpy(skb->data+92, &htsf, sizeof(uint32));

		memcpy(&ts[tsidx].t1, skb->data+80, 16);

		d1 = ts[tsidx].t2 - ts[tsidx].t1;
		d2 = ts[tsidx].t3 - ts[tsidx].t2;
		d3 = ts[tsidx].t4 - ts[tsidx].t3;
		end2end = ts[tsidx].t4 - ts[tsidx].t1;

		sorttobin(d1, &vi_d1);
		sorttobin(d2, &vi_d2);
		sorttobin(d3, &vi_d3);
		sorttobin(end2end, &vi_d4);

		if (end2end > 0 && end2end >  maxdelay) {
			maxdelay = end2end;
			maxdelaypktno = tspktcnt;
			memcpy(&maxdelayts, &ts[tsidx], 16);
		}
		if (++tsidx >= TSMAX)
			tsidx = 0;
	}
}

uint32
dhd_get_htsf(dhd_info_t *dhd, int ifidx)
{
	uint32 htsf = 0, cur_cycle, delta, delta_us;
	uint32    factor, baseval, baseval2;
	cycles_t t;

	t = get_cycles();
	cur_cycle = t;

	if (cur_cycle >  dhd->htsf.last_cycle) {
		delta = cur_cycle -  dhd->htsf.last_cycle;
	} else {
		delta = cur_cycle + (0xFFFFFFFF -  dhd->htsf.last_cycle);
	}

	delta = delta >> 4;

	if (dhd->htsf.coef) {
		/* times ten to get the first digit */
	        factor = (dhd->htsf.coef*10 + dhd->htsf.coefdec1);
		baseval  = (delta*10)/factor;
		baseval2 = (delta*10)/(factor+1);
		delta_us  = (baseval -  (((baseval - baseval2) * dhd->htsf.coefdec2)) / 10);
		htsf = (delta_us << 4) +  dhd->htsf.last_tsf + HTSF_BUS_DELAY;
	} else {
		DHD_ERROR(("-------dhd->htsf.coef = 0 -------\n"));
	}

	return htsf;
}

static void
dhd_dump_latency(void)
{
	int i, max = 0;
	int d1, d2, d3, d4, d5;

	printf("T1       T2       T3       T4           d1  d2   t4-t1     i    \n");
	for (i = 0; i < TSMAX; i++) {
		d1 = ts[i].t2 - ts[i].t1;
		d2 = ts[i].t3 - ts[i].t2;
		d3 = ts[i].t4 - ts[i].t3;
		d4 = ts[i].t4 - ts[i].t1;
		d5 = ts[max].t4-ts[max].t1;
		if (d4 > d5 && d4 > 0)  {
			max = i;
		}
		printf("%08X %08X %08X %08X \t%d %d %d   %d i=%d\n",
			ts[i].t1, ts[i].t2, ts[i].t3, ts[i].t4,
			d1, d2, d3, d4, i);
	}

	printf("current idx = %d \n", tsidx);

	printf("Highest latency %d pkt no.%d total=%d\n", maxdelay, maxdelaypktno, tspktcnt);
	printf("%08X %08X %08X %08X \t%d %d %d   %d\n",
	maxdelayts.t1, maxdelayts.t2, maxdelayts.t3, maxdelayts.t4,
	maxdelayts.t2 - maxdelayts.t1,
	maxdelayts.t3 - maxdelayts.t2,
	maxdelayts.t4 - maxdelayts.t3,
	maxdelayts.t4 - maxdelayts.t1);
}

static int
dhd_ioctl_htsf_get(dhd_info_t *dhd, int ifidx)
{
	char buf[32];
	int ret;
	uint32 s1, s2;

	struct tsf {
		uint32 low;
		uint32 high;
	} tsf_buf;

	memset(&tsf_buf, 0, sizeof(tsf_buf));

	s1 = dhd_get_htsf(dhd, 0);
	ret = dhd_iovar(&dhd->pub, ifidx, "tsf", NULL, 0, buf, sizeof(buf), FALSE);
	if (ret < 0) {
		if (ret == -EIO) {
			DHD_ERROR(("%s: tsf is not supported by device\n",
				dhd_ifname(&dhd->pub, ifidx)));
			return -EOPNOTSUPP;
		}
		return ret;
	}
	s2 = dhd_get_htsf(dhd, 0);

	memcpy(&tsf_buf, buf, sizeof(tsf_buf));
	printf(" TSF_h=%04X lo=%08X Calc:htsf=%08X, coef=%d.%d%d delta=%d ",
		tsf_buf.high, tsf_buf.low, s2, dhd->htsf.coef, dhd->htsf.coefdec1,
		dhd->htsf.coefdec2, s2-tsf_buf.low);
	printf("lasttsf=%08X lastcycle=%08X\n", dhd->htsf.last_tsf, dhd->htsf.last_cycle);

	return 0;
}

void
htsf_update(dhd_info_t *dhd, void *data)
{
	static ulong  cur_cycle = 0, prev_cycle = 0;
	uint32 htsf, tsf_delta = 0;
	uint32 hfactor = 0, cyc_delta, dec1 = 0, dec2, dec3, tmp;
	ulong b, a;
	cycles_t t;

	/* cycles_t in inlcude/mips/timex.h */

	t = get_cycles();

	prev_cycle = cur_cycle;
	cur_cycle = t;

	if (cur_cycle > prev_cycle)
		cyc_delta = cur_cycle - prev_cycle;
	else {
		b = cur_cycle;
		a = prev_cycle;
		cyc_delta = cur_cycle + (0xFFFFFFFF - prev_cycle);
	}

	if (data == NULL)
		printf(" tsf update ata point er is null \n");

	memcpy(&prev_tsf, &cur_tsf, sizeof(tsf_t));
	memcpy(&cur_tsf, data, sizeof(tsf_t));

	if (cur_tsf.low == 0) {
		DHD_INFO((" ---- 0 TSF, do not update, return\n"));
		return;
	}

	if (cur_tsf.low > prev_tsf.low)
		tsf_delta = (cur_tsf.low - prev_tsf.low);
	else {
		DHD_INFO((" ---- tsf low is smaller cur_tsf= %08X, prev_tsf=%08X, \n",
		 cur_tsf.low, prev_tsf.low));
		if (cur_tsf.high > prev_tsf.high) {
			tsf_delta = cur_tsf.low + (0xFFFFFFFF - prev_tsf.low);
			DHD_INFO((" ---- Wrap around tsf coutner  adjusted TSF=%08X\n", tsf_delta));
		} else {
			return; /* do not update */
		}
	}

	if (tsf_delta)  {
		hfactor = cyc_delta / tsf_delta;
		tmp  = 	(cyc_delta - (hfactor * tsf_delta))*10;
		dec1 =  tmp/tsf_delta;
		dec2 =  ((tmp - dec1*tsf_delta)*10) / tsf_delta;
		tmp  = 	(tmp   - (dec1*tsf_delta))*10;
		dec3 =  ((tmp - dec2*tsf_delta)*10) / tsf_delta;

		if (dec3 > 4) {
			if (dec2 == 9) {
				dec2 = 0;
				if (dec1 == 9) {
					dec1 = 0;
					hfactor++;
				} else {
					dec1++;
				}
			} else {
				dec2++;
			}
		}
	}

	if (hfactor) {
		htsf = ((cyc_delta * 10)  / (hfactor*10+dec1)) + prev_tsf.low;
		dhd->htsf.coef = hfactor;
		dhd->htsf.last_cycle = cur_cycle;
		dhd->htsf.last_tsf = cur_tsf.low;
		dhd->htsf.coefdec1 = dec1;
		dhd->htsf.coefdec2 = dec2;
	} else {
		htsf = prev_tsf.low;
	}
}

#endif /* WLMEDIA_HTSF */

#ifdef CUSTOM_SET_CPUCORE
void
dhd_set_cpucore(dhd_pub_t *dhd, int set)
{
	int e_dpc = 0, e_rxf = 0, retry_set = 0;

	if (!(dhd->wlcore->chan_isvht80)) {
		DHD_ERROR(("%s: chan_status(%d) cpucore!!!\n", __FUNCTION__, dhd->chan_isvht80));
		return;
	}

	if (DPC_CPUCORE) {
		do {
			if (set == TRUE) {
				e_dpc = set_cpus_allowed_ptr(dhd->current_dpc,
					cpumask_of(DPC_CPUCORE));
			} else {
				e_dpc = set_cpus_allowed_ptr(dhd->current_dpc,
					cpumask_of(PRIMARY_CPUCORE));
			}
			if (retry_set++ > MAX_RETRY_SET_CPUCORE) {
				DHD_ERROR(("%s: dpc(%d) invalid cpu!\n", __FUNCTION__, e_dpc));
				return;
			}
			if (e_dpc < 0)
				OSL_SLEEP(1);
		} while (e_dpc < 0);
	}
	if (RXF_CPUCORE) {
		do {
			if (set == TRUE) {
				e_rxf = set_cpus_allowed_ptr(dhd->current_rxf,
					cpumask_of(RXF_CPUCORE));
			} else {
				e_rxf = set_cpus_allowed_ptr(dhd->current_rxf,
					cpumask_of(PRIMARY_CPUCORE));
			}
			if (retry_set++ > MAX_RETRY_SET_CPUCORE) {
				DHD_ERROR(("%s: rxf(%d) invalid cpu!\n", __FUNCTION__, e_rxf));
				return;
			}
			if (e_rxf < 0)
				OSL_SLEEP(1);
		} while (e_rxf < 0);
	}
#ifdef DHD_OF_SUPPORT
	interrupt_set_cpucore(set);
#endif /* DHD_OF_SUPPORT */
	DHD_TRACE(("%s: set(%d) cpucore success!\n", __FUNCTION__, set));

	return;
}
#endif /* CUSTOM_SET_CPUCORE */

#if defined(DHD_TCP_WINSIZE_ADJUST)

static int
dhd_port_list_match(int port)
{
	int i;
	for (i = 0; i < MAX_TARGET_PORTS; i++) {
		if (target_ports[i] == port)
			return 1;
	}

	return 0;
}

static void
dhd_adjust_tcp_winsize(int op_mode, struct sk_buff *skb)
{
	struct iphdr *ipheader;
	struct tcphdr *tcpheader;
	uint16 win_size;
	int32 incremental_checksum;

	if (!(op_mode & DHD_FLAG_HOSTAP_MODE))
		return;
	if (skb == NULL || skb->data == NULL)
		return;

	ipheader = (struct iphdr*)(skb->data);

	if (ipheader->protocol == IPPROTO_TCP) {
		tcpheader = (struct tcphdr*) skb_pull(skb, (ipheader->ihl)<<2);
		if (tcpheader) {
			win_size = ntoh16(tcpheader->window);
			if (win_size < MIN_TCP_WIN_SIZE &&
				dhd_port_list_match(ntoh16(tcpheader->dest))) {
				incremental_checksum = ntoh16(tcpheader->check);
				incremental_checksum += win_size - win_size*WIN_SIZE_SCALE_FACTOR;
				if (incremental_checksum < 0)
					--incremental_checksum;
				tcpheader->window = hton16(win_size*WIN_SIZE_SCALE_FACTOR);
				tcpheader->check = hton16((unsigned short)incremental_checksum);
			}
		}
		skb_push(skb, (ipheader->ihl)<<2);
	}
}
#endif /* DHD_TCP_WINSIZE_ADJUST */

#ifdef DHD_MCAST_REGEN
/* Get interface specific ap_isolate configuration */
int
dhd_get_mcast_regen_bss_enable(dhd_pub_t *dhdp, uint32 idx)
{
	dhd_info_t *dhd = dhdp->info;
	dhd_if_t *ifp;

	ASSERT(idx < DHD_MAX_IFS);

	ifp = dhd->iflist[idx];

	return ifp->mcast_regen_bss_enable;
}

/* Set interface specific mcast_regen configuration */
int
dhd_set_mcast_regen_bss_enable(dhd_pub_t *dhdp, uint32 idx, int val)
{
	dhd_info_t *dhd = dhdp->info;
	dhd_if_t *ifp;

	ASSERT(idx < DHD_MAX_IFS);

	ifp = dhd->iflist[idx];

	ifp->mcast_regen_bss_enable = val;

	/* Disable rx_pkt_chain feature for interface, if mcast_regen feature
	 * is enabled
	 */
	dhd_update_rx_pkt_chainable_state(dhdp, idx);

	return BCME_OK;
}
#endif	/* DHD_MCAST_REGEN */

/* Get interface specific ap_isolate configuration */
int
dhd_get_ap_isolate(dhd_pub_t *dhdp, uint32 idx)
{
	dhd_info_t *dhd = dhdp->info;
	dhd_if_t *ifp;

	ASSERT(idx < DHD_MAX_IFS);

	ifp = dhd->iflist[idx];

	return ifp->ap_isolate;
}

/* Set interface specific ap_isolate configuration */
int
dhd_set_ap_isolate(dhd_pub_t *dhdp, uint32 idx, int val)
{
	dhd_info_t *dhd = dhdp->info;
	dhd_if_t *ifp;

	ASSERT(idx < DHD_MAX_IFS);

	ifp = dhd->iflist[idx];

	ifp->ap_isolate = val;

	return 0;
}

#ifdef DHD_FW_COREDUMP
/**
 * Since writing to a file cannot be done in interrupt execution context, function dhd_mem_dump()
 * is scheduled to run at a lower execution context.
 */
void
dhd_schedule_memdump(dhd_pub_t *dhdp, uint8 *buf, uint32 size)
{
	dhd_dump_t *dump = NULL;
	dump = (dhd_dump_t *)MALLOC(dhdp->osh, sizeof(dhd_dump_t));
	if (dump == NULL) {
		DHD_ERROR(("%s: dhd dump memory allocation failed\n", __FUNCTION__));
		return;
	}
	dump->buf = buf;
	dump->bufsize = size;

#if defined(CONFIG_ARM64) || (defined(BCA_HNDROUTER) && defined(__aarch64__))
	DHD_ERROR(("%s: buf(va)=%llx, buf(pa)=%llx, bufsize=%d\n", __FUNCTION__,
		(uint64)buf, (uint64)__virt_to_phys((ulong)buf), size));
#elif defined(__ARM_ARCH_7A__)
	DHD_ERROR(("%s: buf(va)=%x, buf(pa)=%x, bufsize=%d\n", __FUNCTION__,
		(uint32)buf, (uint32)__virt_to_phys((ulong)buf), size));
#endif /* __ARM_ARCH_7A__ */
	if (dhdp->memdump_enabled == DUMP_MEMONLY) {
		BUG_ON(1);
	}

	if (dhd_deferred_schedule_work(dhdp->info->dhd_deferred_wq,
		dump, DHD_WQ_WORK_SOC_RAM_DUMP,
		dhd_mem_dump, DHD_WORK_PRIORITY_HIGH) != DHD_WQ_STS_OK) {
		MFREE(dhdp->osh, dump, sizeof(dhd_dump_t));
	}
}

/**
 * Called in scheduled work context. Since waitable (file) operations take place, no spinlock may
 * be held when calling this function.
 */
static void
dhd_mem_dump(void *handle, void *event_info, enum dhd_wq_event event)
{
	dhd_info_t *dhd = handle;
	dhd_dump_t *dump = event_info;

	if (!dhd) {
		DHD_ERROR(("%s: dhd is NULL\n", __FUNCTION__));
		return;
	}

	if (!dump) {
		DHD_ERROR(("%s: dump is NULL\n", __FUNCTION__));
		return;
	}

	if (dump->buf == NULL) {
		/* Likely a large allocation failed. Try alternative approach. */
		socsram_write_to_file_no_large_buf(dhd);
	} else if (socsram_write_to_file(&dhd->pub, dump->buf, dump->bufsize)) {
		DHD_ERROR(("%s: writing SoC_RAM dump to the file failed\n", __FUNCTION__));
	}

	if (dhd->pub.memdump_enabled == DUMP_MEMFILE_BUGON) {
		BUG_ON(1);
	}

	MFREE(dhd->pub.osh, dump, sizeof(dhd_dump_t));
}
#endif /* DHD_FW_COREDUMP */

void
dhd_schedule_trap_log_dump(dhd_pub_t *dhdp, bool skip_regdumps)
{
	dhd_deferred_schedule_work(dhdp->info->dhd_deferred_wq, (void *)skip_regdumps,
		DHD_WQ_WORK_FWDUMP, dhd_fwtrap_handler, DHD_WORK_PRIORITY_HIGH);
	dhd_deferred_work_disable_all(dhdp->info->dhd_deferred_wq);
}

typedef struct _dtrace_wq_t {
	uint16 datalen;
	uint8 event_data[1];
} dtrace_wq_t;

void
dhd_schedule_dtrace(dhd_pub_t *dhdp, uint8 *event_data, uint32 datalen)
{
	int res;
	dtrace_wq_t *dtrace_wq_data = MALLOCZ(dhdp->osh,
		(OFFSETOF(dtrace_wq_t, event_data) + datalen));

	if (dtrace_wq_data == NULL) {
		DHD_ERROR(("%s: malloc fail for dtrace_wq_data\n", __FUNCTION__));
		return;
	}
	dtrace_wq_data->datalen = (uint16)datalen;
	memcpy(dtrace_wq_data->event_data, event_data, datalen);

	if ((res = dhd_deferred_schedule_work(dhdp->info->dhd_deferred_wq, dtrace_wq_data,
		DHD_WQ_WORK_DTRACE, dhd_dtrace_handler, DHD_WORK_PRIORITY_HIGH)) != DHD_WQ_STS_OK) {
		DHD_ERROR(("%s: wq schedule failes %d\n", __FUNCTION__, res));
		MFREE(dhdp->osh, dtrace_wq_data, OFFSETOF(dtrace_wq_t, event_data) + datalen);
	}
}

#ifdef BCM_ROUTER_DHD

/* Returns the pid of a the userspace process running with the given name */
static struct task_struct *
_get_task_info(const char *pname)
{
	struct task_struct *task;
	if (!pname)
		return NULL;

	for_each_process(task) {
		if (strcmp(pname, task->comm) == 0)
			return task;
	}

	return NULL;
}

#define DEBUG_MONITOR_NS	"debug_monitor"
extern void emergency_restart(void);
#endif /* BCM_ROUTER_DHD */

void
dhd_write_file(dhd_pub_t *dhdp, const char *file_name, uint8 *dumpbuf)
{
	if (write_file(file_name, (O_CREAT | O_WRONLY | O_SYNC | O_APPEND),
		dumpbuf, (int)strlen(dumpbuf))) {
		DHD_ERROR(("%s: writing dump %s to the file failed\n",
			__FUNCTION__, file_name));
	}
}

#if !defined(BCMPCIE)
/* not implemented by usb layer */
void *
dhd_bus_sih(struct dhd_bus *bus)
{
	return NULL;
}
#endif

static int
_dhd_macdbg_dump_name(dhd_info_t *dhd, enum macdbgdumptype dump_type, char *dumpbuf)
{
	dhd_pub_t *dhdp = &dhd->pub;
	char dumpfilename[DUMPMAC_FILENAME_SZ] = {0};
	const char *dumpname;
	int res;

	DHD_OS_WAKE_LOCK(dhdp);
	DHD_LOCK(dhdp);

	dumpname = macdbg_dump_names[dump_type];

	/* validation if this dump is allowed */
	if (!isbitset(dhd->macdbg_dump_level, dump_type)) {
		res = BCME_DISABLED;
		goto failed;
	}

	snprintf(dumpfilename, DUMPMAC_FILENAME_SZ,
		"/tmp/dump_%s_%04x.txt", dumpname, dhd->dump_signature);

	if (dump_type == MACDBG_DUMP_RXBM) {
		/* iovar buffer size is too short for rxbm, so dedicated function call */
		res = dhd_macdbg_dumprxbm(dhdp, dumpbuf, DUMPMAC_BUF_SZ, dumpfilename);
		if (res != BCME_OK)
			goto failed;

		DHD_ERROR(("%s: dhd dump %s to %s\n", __FUNCTION__, dumpname, dumpfilename));
		goto done;
	}

	res = dhd_macdbg_dump(dhdp, dumpbuf, DUMPMAC_BUF_SZ, dumpname);
	if (res != BCME_OK) {
		goto failed;
	}

	DHD_UNLOCK(dhdp);
	DHD_OS_WAKE_UNLOCK(dhdp);

	DHD_ERROR(("%s: dhd dump %s to %s len %d bytes\n", __FUNCTION__,
		dumpname, dumpfilename, (int)strlen(dumpbuf)));
	/* Write to a file */
	if (write_file(dumpfilename, (O_CREAT | O_WRONLY | O_SYNC),
		dumpbuf, (int)strlen(dumpbuf))) {
		DHD_ERROR(("%s: writing dump %s to the file failed\n",
			__FUNCTION__, dumpname));
	}

	memset(dumpbuf, 0, DUMPMAC_BUF_SZ);
	return BCME_OK;

failed:
	DHD_ERROR(("%s: dump %s fails %d\n", __FUNCTION__, dumpname, res));
done:
	DHD_UNLOCK(dhdp);
	DHD_OS_WAKE_UNLOCK(dhdp);
	return res;
}

static void
dhd_fwtrap_handler(void *handle, void *event_info, enum dhd_wq_event event)
{
	dhd_info_t *dhd = handle;
	char *dumpbuf = NULL;
	dhd_pub_t *dhdp = &dhd->pub;
	int dumpnum = 0;
	bool isup = FALSE;
	bool skip_regdumps = (bool)event_info;
	si_t *sih;
	int idx;
#ifdef BCM_ROUTER_DHD
	struct task_struct *monitor_task;
	char *failed_if;
#endif /* BCM_ROUTER_DHD */

	if (!dhd) {
		DHD_ERROR(("%s: dhd is NULL\n", __FUNCTION__));
		return;
	}
#ifdef BCA_HNDROUTER
	if (is_reboot == SYS_RESTART)
		return;
#endif

	ASSERT(event == DHD_WQ_WORK_FWDUMP);

	sih = dhd_bus_sih(dhdp->bus);
	if (sih) {
		idx = si_coreidx(sih);
	        if (si_setcore(sih, D11_CORE_ID, 0) != NULL) {
			if (si_iscoreup(sih)) {
				isup = TRUE;
			}
			si_setcoreidx(sih, idx);
		}
	}

	DHD_ERROR(("%s: Dongle(wl%d) macreg dump scheduled with isup %d\n",
		__FUNCTION__, dhdp->unit, isup));

	/* Write macdump to a file */

	if (dhdp->busstate != DHD_BUS_DOWN) {
		/* Make sure dongle stops running to avoid race in reading mac registers */
		(void) dhd_wl_ioctl_set_intiovar(dhdp, "bus:disconnect", 102,
			WLC_SET_VAR, TRUE, 0);
		dhdp->busstate = DHD_BUS_DOWN;
	}

	if (!isup || skip_regdumps) {
		DHD_ERROR(("%s: don't do register dumps\n", __FUNCTION__));
		goto socram;
	}

	dumpbuf = (char *)MALLOCZ(dhdp->osh, DUMPMAC_BUF_SZ);
	if (dumpbuf == NULL) {
		DHD_ERROR(("%s: dumpbuf malloc failed for 0x%x bytes\n",
			__FUNCTION__, DUMPMAC_BUF_SZ));
		goto socram;
	}

	/*
	 * No need to update this for new dumps. The pre-processor expansion
	 * will produce code as defined by MACDBGDUMP_ENUMDEF for each dump
	 * as listed in MACDBGDUMP_ENUMDEF_LIST.
	 *
	 * MACDBGDUMP_ENUMDEF is defined below to produce the call and check
	 * the result.
	 */
#define MACDBGDUMP_ENUMDEF(_id, _str) \
	if (_dhd_macdbg_dump_name(dhd, MACDBG_DUMP_ ## _id, dumpbuf) == BCME_OK) { \
		dumpnum++; \
	}

	MACDBGDUMP_ENUMDEF_LIST

#undef MACDBGDUMP_ENUMDEF

socram:
	socsram_write_to_file_no_large_buf(dhd);

	/* print the dtrace file names for UTF scripts to pick up */
	for (idx = 0; idx <= dhd->dtrace_max_fidx; idx++) {
		DHD_ERROR(("%s: dhd dump dtrace0 to /tmp/dump_dtrace%d_%04x.txt len 8 byte\n",
			__FUNCTION__, idx, dhd->dump_signature));
	}
	DHD_ERROR(("%s: dump signature: %04x, dumpnum: %d, dtrace_cur_fidx: %1x, dumplevel: 0x%x\n",
		__FUNCTION__, dhd->dump_signature, dumpnum,
		dhd->dtrace_cur_fidx, dhd->macdbg_dump_level));

	MFREE(dhdp->osh, dumpbuf, DUMPMAC_BUF_SZ);

#ifdef BCM_ROUTER_DHD
	/* check if debug_monitor is running */
	monitor_task = _get_task_info(DEBUG_MONITOR_NS);
	if (monitor_task == NULL) {
		/* If debug_monitor is not running, handle recovery from here */
		char *val = getvar(NULL, "watchdog");

		if (val && bcm_atoi(val)) {
			/* watchdog enabled, so reboot */
			DHD_ERROR(("%s: Dongle(wl%d) trap detected. Restarting the system\n",
				__FUNCTION__, dhdp->unit));

			mdelay(1000);
			emergency_restart();
			while (1)
				cpu_relax();
		} else {
			DHD_ERROR(("%s: Dongle(wl%d) trap detected. No watchdog.\n",
			   __FUNCTION__, dhdp->unit));
		}

		return;
	}

	/* save interface name for debug_monitor */
	/* XXX: the interface name is needed for debug_monitor to run "dhd -i XXX upload" only.
	 * If the iovar is run by dhd host driver and the resulting dump file is put into /tmp/
	 * so that debug_monitor can simply copy it, we can avoid creating this failed_if.txt file.
	 */
	failed_if = dhd_ifname(&dhd->pub, 0);
	/* Write to a file */
	if (write_file("/tmp/failed_if.txt", (O_CREAT | O_WRONLY | O_SYNC),
		failed_if, strlen(failed_if))) {
		DHD_ERROR(("%s: writing interface name %s to the file failed\n",
			__FUNCTION__, failed_if));
	}

	/* If monitor daemon is running, let's signal the monitor for recovery */
	DHD_ERROR(("%s: Dongle(wl%d) trap detected. Send signal to debug_monitor from %s.\n",
		__FUNCTION__, dhdp->unit, failed_if));

	/* send SIGUSR2 so that debug_monitor thinks noclk is true and jumps out macreg dump */
	send_sig_info(SIGUSR2, (void *)1L, monitor_task);
#endif /* BCM_ROUTER_DHD */
}

#define DTRACE_MAX_FILESIZE	(512 * 1024)	/* 512KB */
#define DTRACE_MAX_FILENUM	2		/* keep it power of 2 */

static void
dhd_dtrace_handler(void *handle, void *event_info, enum dhd_wq_event event)
{
	dhd_info_t *dhd = handle;
	dhd_pub_t *dhdp = &dhd->pub;
	char *dumpbuf = NULL;
	int res, dumplen, datalen = 0, i = 0, j;
	dtrace_wq_t *dtrace_wq = event_info;
	struct bcmstrbuf *b = NULL;
	struct bcmstrbuf bcmstrbuf;
	uint32 fifleflags = (O_CREAT | O_WRONLY | O_SYNC | O_APPEND);
	char dumpfilename[DUMPMAC_FILENAME_SZ] = {0};
	bcm_xtlv_t *dtrace_rec;

	ASSERT(dhd != NULL);
	ASSERT(event == DHD_WQ_WORK_DTRACE);

	dumpbuf = (char *)MALLOCZ(dhdp->osh, DUMPMAC_BUF_SZ);
	if (dumpbuf == NULL) {
		DHD_ERROR(("%s: dumpbuf malloc failed for 0x%x bytes\n",
			__FUNCTION__, DUMPMAC_BUF_SZ));
		goto exit;
	}

	bcm_binit(&bcmstrbuf, dumpbuf, DUMPMAC_BUF_SZ);
	b = &bcmstrbuf;

	while (datalen < dtrace_wq->datalen) {
		dtrace_rec = (bcm_xtlv_t *)(dtrace_wq->event_data + datalen);
		bcm_bprintf(b, "[%02d] %01X ", i, dtrace_rec->id);
		for (j = 0; j < dtrace_rec->len; j++) {
			bcm_bprintf(b, "%02X", dtrace_rec->data[j]);
		}
		bcm_bprintf(b, "\n");
		i++;
		datalen += (dtrace_rec->len + BCM_XTLV_HDR_SIZE);
	}
	ASSERT(datalen == dtrace_wq->datalen);
	dumplen = strlen(dumpbuf);

	DHD_OS_WAKE_LOCK(dhdp);
	DHD_LOCK(dhdp);
	if (dhd->dtrace_cur_fsz >= DTRACE_MAX_FILESIZE) {
		/* if current dtrace dump file is too big, stop growing it and create another */
		fifleflags = (O_CREAT | O_WRONLY | O_SYNC | O_TRUNC);
		dhd->dtrace_cur_fidx = (dhd->dtrace_cur_fidx + 1) & (DTRACE_MAX_FILENUM - 1);
		dhd->dtrace_cur_fsz = 0;
		DHD_INFO(("%s: switch to new dtrace file idx %d\n", __FUNCTION__,
			dhd->dtrace_cur_fidx));
		dhd->dtrace_max_fidx = MAX(dhd->dtrace_max_fidx, dhd->dtrace_cur_fidx);
	}

	snprintf(dumpfilename, DUMPMAC_FILENAME_SZ, "/tmp/dump_dtrace%1x_%04x.txt",
		dhd->dtrace_cur_fidx, dhd->dump_signature);

	if ((res = write_file(dumpfilename, fifleflags, dumpbuf, dumplen))) {
		DHD_ERROR(("%s: dtrace dump failed %d\n",
			__FUNCTION__, res));
	} else {
		dhd->dtrace_cur_fsz += dumplen;
		DHD_INFO(("%s: dumplen %d cur_fsz %d\n", __FUNCTION__,
			dumplen, dhd->dtrace_cur_fsz));
	}

	DHD_UNLOCK(dhdp);
	DHD_OS_WAKE_UNLOCK(dhdp);
	MFREE(dhdp->osh, dumpbuf, DUMPMAC_BUF_SZ);
exit:
	MFREE(dhdp->osh, event_info, OFFSETOF(dtrace_wq_t, event_data) + datalen);
}

#ifdef BCMHWA
static void
dhd_hwa_event_handler(void *handle, void *event_info, enum dhd_wq_event event)
{
	dhd_info_t *dhd = handle;
	dhd_pub_t *dhdp = &dhd->pub;

	ASSERT(event == DHD_WQ_WORK_HWA);
	BCM_REFERENCE(event_info);

	DHD_ERROR(("%s: Dongle(wl%d) hwa event scheduled\n",
		__FUNCTION__, dhdp->unit));

	DHD_OS_WAKE_LOCK(dhdp);
	DHD_LOCK(dhdp);

	/* Enable hwa Rx blocks to resume receiving packets */
	(void) dhd_wl_ioctl_set_intiovar(dhdp, "hwa_rx_en", 1, WLC_SET_VAR, TRUE, 0);

	DHD_UNLOCK(dhdp);
	DHD_OS_WAKE_UNLOCK(dhdp);
	dhd_deferred_work_enable(dhd->dhd_deferred_wq, BIT(DHD_WQ_WORK_HWA));
}

void
dhd_hwa_event(dhd_pub_t *dhdp)
{
	DHD_ERROR(("%s: Dongle(wl%d) schedule hwa event\n",
		__FUNCTION__, dhdp->unit));

	dhd_deferred_schedule_work(dhdp->info->dhd_deferred_wq, NULL,
		DHD_WQ_WORK_HWA, dhd_hwa_event_handler, DHD_WORK_PRIORITY_HIGH);
	dhd_deferred_work_disable(dhdp->info->dhd_deferred_wq, BIT(DHD_WQ_WORK_HWA));
}
#endif /* BCMHWA */

#ifdef DHD_WMF
static void
dhd_set_wmf_handler(void *handle, void *event_info, enum dhd_wq_event event)
{
	dhd_info_t *dhd = handle;
	uint32 *ifidx_p = event_info;

	if (event != DHD_WQ_WORK_SET_WMF) {
		DHD_ERROR(("%s: unexpected event \n", __FUNCTION__));
		return;
	}

	if (!dhd) {
		DHD_ERROR(("%s: dhd info not available \n", __FUNCTION__));
		return;
	}

	if (!ifidx_p) {
		DHD_ERROR(("%s: event data is null \n", __FUNCTION__));
		return;
	}

	dhd_net_if_lock_local(dhd);
	DHD_OS_WAKE_LOCK(&dhd->pub);
	DHD_LOCK(&dhd->pub);

	dhd_wmf_bss_enable(&dhd->pub, *ifidx_p);
	MFREE(dhd->pub.osh, ifidx_p, sizeof(uint32));

	DHD_UNLOCK(&dhd->pub);
	DHD_OS_WAKE_UNLOCK(&dhd->pub);
	dhd_net_if_unlock_local(dhd);
}

void
dhd_schedule_wmf_bss_enable(dhd_pub_t *dhdp, uint32 ifidx)
{
	uint32 *ifidx_p = (uint32 *)MALLOC(dhdp->osh, sizeof(uint32));

	if (ifidx_p == NULL) {
		DHD_ERROR(("%s: dhd set wmf memory allocation failed\n", __FUNCTION__));
		return;
	}

	*ifidx_p = ifidx;
	if (dhd_deferred_schedule_work(dhdp->info->dhd_deferred_wq,
		ifidx_p, DHD_WQ_WORK_SET_WMF,
		dhd_set_wmf_handler, DHD_WORK_PRIORITY_LOW) != DHD_WQ_STS_OK) {
		MFREE(dhdp->osh, ifidx_p, sizeof(uint32));
	}
}

#if defined(BCM_NBUFF_WLMCAST_IPV6)
char*
dhd_wmf_ifname(dhd_pub_t *dhdp, uint32 idx)
{
	dhd_info_t *dhd = dhdp->info;
	dhd_if_t *ifp;

	ASSERT(idx < DHD_MAX_IFS);

	ifp = dhd->iflist[idx];

	return ifp->name;
}
#endif

/* Returns interface specific WMF configuration */
dhd_wmf_t*
dhd_wmf_conf(dhd_pub_t *dhdp, uint32 idx)
{
	dhd_info_t *dhd = dhdp->info;
	dhd_if_t *ifp;

	ASSERT(idx < DHD_MAX_IFS);

	ifp = dhd->iflist[idx];

	return &ifp->wmf;
}
#endif /* DHD_WMF */

#if defined(BCM_ROUTER_DHD) || defined(TRAFFIC_MGMT_DWM)
void
traffic_mgmt_pkt_set_prio(dhd_pub_t *dhdp, void * pktbuf)
{
	struct ether_header *eh;
	struct ethervlan_header *evh;
	uint8 *pktdata, *ip_body;
	uint8  dwm_filter;
	uint8 tos_tc = 0;
	uint8 dscp   = 0;
	pktdata = (uint8 *)PKTDATA(dhdp->osh, pktbuf);
	eh = (struct ether_header *) pktdata;
	ip_body = NULL;

	if (dhdp->dhd_tm_dwm_tbl.dhd_dwm_enabled) {
		if (eh->ether_type == hton16(ETHER_TYPE_8021Q)) {
			evh = (struct ethervlan_header *)eh;
			if ((evh->ether_type == hton16(ETHER_TYPE_IP)) ||
				(evh->ether_type == hton16(ETHER_TYPE_IPV6))) {
				ip_body = pktdata + sizeof(struct ethervlan_header);
			}
		} else if ((eh->ether_type == hton16(ETHER_TYPE_IP)) ||
			(eh->ether_type == hton16(ETHER_TYPE_IPV6))) {
			ip_body = pktdata + sizeof(struct ether_header);
		}
		if (ip_body) {
			tos_tc = IP_TOS46(ip_body);
			dscp = tos_tc >> IPV4_TOS_DSCP_SHIFT;
		}

		if (dscp < DHD_DWM_TBL_SIZE) {
			dwm_filter = dhdp->dhd_tm_dwm_tbl.dhd_dwm_tbl[dscp];
			if (DHD_TRF_MGMT_DWM_IS_FILTER_SET(dwm_filter)) {
				PKTSETPRIO(pktbuf, DHD_TRF_MGMT_DWM_PRIO(dwm_filter));
			}
		}
	}
}
#endif /* BCM_ROUTER_DHD || TRAFFIC_MGMT_DWM */

#if defined(DHD_L2_FILTER) || defined(BCM_ROUTER_DHD) || defined(STB)
bool
dhd_sta_associated(dhd_pub_t *dhdp, uint32 bssidx, uint8 *mac)
{
	return dhd_find_sta(dhdp, bssidx, mac) ? TRUE : FALSE;
}
#endif /* #if defined (DHD_L2_FILTER) || defined(BCM_ROUTER_DHD) */

#ifdef DHD_L2_FILTER
arp_table_t*
dhd_get_ifp_arp_table_handle(dhd_pub_t *dhdp, uint32 bssidx)
{
	dhd_info_t *dhd = dhdp->info;
	dhd_if_t *ifp;

	ASSERT(bssidx < DHD_MAX_IFS);
	ifp = dhd->iflist[bssidx];

	return ifp->phnd_arp_table;
}

int
dhd_get_parp_status(dhd_pub_t *dhdp, uint32 idx)
{
	dhd_info_t *dhd = dhdp->info;
	dhd_if_t *ifp;

	ASSERT(idx < DHD_MAX_IFS);

	ifp = dhd->iflist[idx];

	if (ifp)
		return ifp->parp_enable;
	else
		return FALSE;
}

/* Set interface specific proxy arp configuration */
int
dhd_set_parp_status(dhd_pub_t *dhdp, uint32 idx, int val)
{
	dhd_info_t *dhd = dhdp->info;
	dhd_if_t *ifp;
	ASSERT(idx < DHD_MAX_IFS);
	ifp = dhd->iflist[idx];

	if (!ifp)
	    return BCME_ERROR;

	/* At present all 3 variables are being
	 * handled at once
	 */
	ifp->parp_enable = val;
	ifp->parp_discard = val;
	ifp->parp_allnode = val;

	/* Flush ARP entries when disabled */
	if (val == FALSE) {
		bcm_l2_filter_arp_table_update(dhdp->osh, ifp->phnd_arp_table, TRUE, NULL,
			FALSE, dhdp->tickcnt);
	}

	return BCME_OK;
}

bool
dhd_parp_discard_is_enabled(dhd_pub_t *dhdp, uint32 idx)
{
	dhd_info_t *dhd = dhdp->info;
	dhd_if_t *ifp;

	ASSERT(idx < DHD_MAX_IFS);

	ifp = dhd->iflist[idx];
	ASSERT(ifp);

	return ifp->parp_discard;
}

bool
dhd_parp_allnode_is_enabled(dhd_pub_t *dhdp, uint32 idx)
{
	dhd_info_t *dhd = dhdp->info;
	dhd_if_t *ifp;

	ASSERT(idx < DHD_MAX_IFS);

	ifp = dhd->iflist[idx];

	ASSERT(ifp);

	return ifp->parp_allnode;
}

int
dhd_get_dhcp_unicast_status(dhd_pub_t *dhdp, uint32 idx)
{
	dhd_info_t *dhd = dhdp->info;
	dhd_if_t *ifp;

	ASSERT(idx < DHD_MAX_IFS);

	ifp = dhd->iflist[idx];

	ASSERT(ifp);

	return ifp->dhcp_unicast;
}

int
dhd_set_dhcp_unicast_status(dhd_pub_t *dhdp, uint32 idx, int val)
{
	dhd_info_t *dhd = dhdp->info;
	dhd_if_t *ifp;
	ASSERT(idx < DHD_MAX_IFS);
	ifp = dhd->iflist[idx];

	ASSERT(ifp);
	ifp->dhcp_unicast = val;

	return BCME_OK;
}

int
dhd_get_block_ping_status(dhd_pub_t *dhdp, uint32 idx)
{
	dhd_info_t *dhd = dhdp->info;
	dhd_if_t *ifp;

	ASSERT(idx < DHD_MAX_IFS);

	ifp = dhd->iflist[idx];
	ASSERT(ifp);

	return ifp->block_ping;
}

int
dhd_set_block_ping_status(dhd_pub_t *dhdp, uint32 idx, int val)
{
	dhd_info_t *dhd = dhdp->info;
	dhd_if_t *ifp;
	ASSERT(idx < DHD_MAX_IFS);
	ifp = dhd->iflist[idx];

	ASSERT(ifp);

	ifp->block_ping = val;
	/* Disable rx_pkt_chain feature for interface if block_ping option is
	 * enabled
	 */
	dhd_update_rx_pkt_chainable_state(dhdp, idx);

	return BCME_OK;
}

int
dhd_get_grat_arp_status(dhd_pub_t *dhdp, uint32 idx)
{
	dhd_info_t *dhd = dhdp->info;
	dhd_if_t *ifp;

	ASSERT(idx < DHD_MAX_IFS);

	ifp = dhd->iflist[idx];
	ASSERT(ifp);

	return ifp->grat_arp;
}

int
dhd_set_grat_arp_status(dhd_pub_t *dhdp, uint32 idx, int val)
{
	dhd_info_t *dhd = dhdp->info;
	dhd_if_t *ifp;
	ASSERT(idx < DHD_MAX_IFS);
	ifp = dhd->iflist[idx];

	ASSERT(ifp);

	ifp->grat_arp = val;

	return BCME_OK;
}

int
dhd_get_block_tdls_status(dhd_pub_t *dhdp, uint32 idx)
{
	dhd_info_t *dhd = dhdp->info;
	dhd_if_t *ifp;

	ASSERT(idx < DHD_MAX_IFS);

	ifp = dhd->iflist[idx];

	ASSERT(ifp);

	return ifp->block_tdls;
}

int
dhd_set_block_tdls_status(dhd_pub_t *dhdp, uint32 idx, int val)
{
	dhd_info_t *dhd = dhdp->info;
	dhd_if_t *ifp;
	ASSERT(idx < DHD_MAX_IFS);
	ifp = dhd->iflist[idx];

	ASSERT(ifp);

	ifp->block_tdls = val;

	return BCME_OK;
}

#endif /* DHD_L2_FILTER */

#if defined(SET_RPS_CPUS) || defined(ARGOS_RPS_CPU_CTL)
int
dhd_rps_cpus_enable(struct net_device *net, int enable)
{
	dhd_info_t *dhd = DHD_DEV_INFO(net);
	dhd_if_t *ifp;
	int ifidx;
	char * RPS_CPU_SETBUF;

	ifidx = dhd_net2idx(dhd, net);
	if (ifidx == DHD_BAD_IF) {
		DHD_ERROR(("%s bad ifidx\n", __FUNCTION__));
		return -ENODEV;
	}

	if (ifidx == PRIMARY_INF) {
		if (dhd->pub.op_mode == DHD_FLAG_IBSS_MODE) {
			DHD_INFO(("%s : set for IBSS.\n", __FUNCTION__));
			RPS_CPU_SETBUF = RPS_CPUS_MASK_IBSS;
		} else {
			DHD_INFO(("%s : set for BSS.\n", __FUNCTION__));
			RPS_CPU_SETBUF = RPS_CPUS_MASK;
		}
	} else if (ifidx == VIRTUAL_INF) {
		DHD_INFO(("%s : set for P2P.\n", __FUNCTION__));
		RPS_CPU_SETBUF = RPS_CPUS_MASK_P2P;
	} else {
		DHD_ERROR(("%s : Invalid index : %d.\n", __FUNCTION__, ifidx));
		return -EINVAL;
	}

	ifp = dhd->iflist[ifidx];
	if (ifp) {
		if (enable) {
			DHD_INFO(("%s : set rps_cpus as [%s]\n", __FUNCTION__, RPS_CPU_SETBUF));
			custom_rps_map_set(ifp->net->_rx, RPS_CPU_SETBUF, strlen(RPS_CPU_SETBUF));
#if (defined(DHDTCPACK_SUPPRESS) && defined(BCMPCIE))
			DHD_TRACE(("%s : set ack suppress. TCPACK_SUP_HOLD.\n", __FUNCTION__));
			dhd_tcpack_suppress_set(&dhd->pub, TCPACK_SUP_HOLD);
#endif /* DHDTCPACK_SUPPRESS && BCMPCIE */
		} else {
			custom_rps_map_clear(ifp->net->_rx);
#if (defined(DHDTCPACK_SUPPRESS) && defined(BCMPCIE))
			DHD_TRACE(("%s : clear ack suppress.\n", __FUNCTION__));
			dhd_tcpack_suppress_set(&dhd->pub, TCPACK_SUP_OFF);
#endif /* DHDTCPACK_SUPPRESS && BCMPCIE */
		}
	} else {
		DHD_ERROR(("%s : ifp is NULL!!\n", __FUNCTION__));
		return -ENODEV;
	}

	return BCME_OK;
}

int
custom_rps_map_set(struct netdev_rx_queue *queue, char *buf, size_t len)
{
	struct rps_map *old_map, *map;
	cpumask_var_t mask;
	int err, cpu, i;
	static DEFINE_SPINLOCK(rps_map_lock);

	DHD_INFO(("%s : Entered.\n", __FUNCTION__));

	if (!alloc_cpumask_var(&mask, GFP_KERNEL)) {
		DHD_ERROR(("%s : alloc_cpumask_var fail.\n", __FUNCTION__));
		return -ENOMEM;
	}

	err = bitmap_parse(buf, len, cpumask_bits(mask), nr_cpumask_bits);
	if (err) {
		free_cpumask_var(mask);
		DHD_ERROR(("%s : bitmap_parse fail.\n", __FUNCTION__));
		return err;
	}

	map = kzalloc(max_t(unsigned int,
		RPS_MAP_SIZE(cpumask_weight(mask)), L1_CACHE_BYTES),
		GFP_KERNEL);
	if (!map) {
		free_cpumask_var(mask);
		DHD_ERROR(("%s : map malloc fail.\n", __FUNCTION__));
		return -ENOMEM;
	}

	i = 0;
	for_each_cpu(cpu, mask) {
		map->cpus[i++] = cpu;
	}

	if (i) {
		map->len = i;
	} else {
		kfree(map);
		map = NULL;
		free_cpumask_var(mask);
		DHD_ERROR(("%s : mapping cpu fail.\n", __FUNCTION__));
		return -1;
	}

	spin_lock(&rps_map_lock);
	old_map = rcu_dereference_protected(queue->rps_map,
		lockdep_is_held(&rps_map_lock));
	rcu_assign_pointer(queue->rps_map, map);
	spin_unlock(&rps_map_lock);

	if (map) {
		static_key_slow_inc(&rps_needed);
	}

	if (old_map) {
		kfree_rcu(old_map, rcu);
		static_key_slow_dec(&rps_needed);
	}

	free_cpumask_var(mask);

	DHD_INFO(("%s : Done. mapping cpu nummber : %d\n", __FUNCTION__, map->len));

	return map->len;
}

void
custom_rps_map_clear(struct netdev_rx_queue *queue)
{
	struct rps_map *map;

	DHD_INFO(("%s : Entered.\n", __FUNCTION__));

	map = rcu_dereference_protected(queue->rps_map, 1);
	if (map) {
		RCU_INIT_POINTER(queue->rps_map, NULL);
		kfree_rcu(map, rcu);
		DHD_INFO(("%s : rps_cpus map clear.\n", __FUNCTION__));
	}
}

#endif /* SET_RPS_CPUS || ARGOS_RPS_CPU_CTL */

/* XXX WAR: Sometimes Exynos 5433 can't schedule Rx traffic after push packet to stack queue
*  if activated rps_cpus kernel feature. To avoid this problem, Need to disable rps_cpus
*  if current tput is low than the TPUT_THRESHOLD
*/
#if defined(ARGOS_CPU_SCHEDULER) && defined(ARGOS_RPS_CPU_CTL)
int
argos_register_notifier_init(struct net_device *net)
{
	int ret = 0;
#if (defined(DHDTCPACK_SUPPRESS) && defined(BCMPCIE))
	dhd_info_t *dhd;
#endif /* DHDTCPACK_SUPPRESS && BCMPCIE */

	DHD_INFO(("DHD: %s: \n", __FUNCTION__));
	argos_rps_ctrl_data.wlan_primary_netdev = net;
	argos_rps_ctrl_data.argos_rps_cpus_enabled = 0;

	ret = sec_argos_register_notifier(&argos_wifi, "WIFI");
	if (ret < 0) {
		DHD_ERROR(("DHD:Failed to register WIFI notifier , ret=%d\n", ret));
	}

#if (defined(DHDTCPACK_SUPPRESS) && defined(BCMPCIE))
	dhd = DHD_DEV_INFO(argos_rps_ctrl_data.wlan_primary_netdev);
	DHD_TRACE(("%s : clear ack suppress.\n", __FUNCTION__));
	dhd_tcpack_suppress_set(&dhd->pub, TCPACK_SUP_OFF);
#endif /* DHDTCPACK_SUPPRESS && BCMPCIE */

	return ret;
}

int
argos_register_notifier_deinit(void)
{
#if defined(DHDTCPACK_SUPPRESS) && defined(BCMPCIE)
	dhd_info_t *dhd;
#endif /* DHDTCPACK_SUPPRESS && BCMPCIE */
	DHD_INFO(("DHD: %s: \n", __FUNCTION__));

	if (argos_rps_ctrl_data.wlan_primary_netdev == NULL) {
		DHD_ERROR(("DHD: primary_net_dev is null %s: \n", __FUNCTION__));
		return -1;
	}
	custom_rps_map_clear(argos_rps_ctrl_data.wlan_primary_netdev->_rx);
#if defined(DHDTCPACK_SUPPRESS) && defined(BCMPCIE)
	dhd = DHD_DEV_INFO(argos_rps_ctrl_data.wlan_primary_netdev);
	DHD_TRACE(("%s : clear ack suppress.\n", __FUNCTION__));
	dhd_tcpack_suppress_set(&dhd->pub, TCPACK_SUP_OFF);
#endif /* DHDTCPACK_SUPPRESS && BCMPCIE */
	sec_argos_unregister_notifier(&argos_wifi, "WIFI");
	argos_rps_ctrl_data.wlan_primary_netdev = NULL;
	argos_rps_ctrl_data.argos_rps_cpus_enabled = 0;

	return 0;
}

int
argos_status_notifier_wifi_cb(struct notifier_block *notifier,
	unsigned long speed, void *v)
{
	int err = 0;

	DHD_INFO(("DHD: %s: , speed=%ld\n", __FUNCTION__, speed));
	if (speed > RPS_TPUT_THRESHOLD && argos_rps_ctrl_data.wlan_primary_netdev != NULL &&
		argos_rps_ctrl_data.argos_rps_cpus_enabled == 0) {
		if (cpu_online(RPS_CPUS_WLAN_CORE_ID)) {
			err = custom_rps_map_set(argos_rps_ctrl_data.wlan_primary_netdev->_rx,
			RPS_CPUS_MASK, strlen(RPS_CPUS_MASK));
			if (err < 0) {
				DHD_ERROR(("DHD: %s: Failed to RPS_CPUs. speed=%ld, error=%d\n",
					__FUNCTION__, speed, err));
			} else {
#if (defined(DHDTCPACK_SUPPRESS) && defined(BCMPCIE))
				dhd_info_t *dhd =
					DHD_DEV_INFO(argos_rps_ctrl_data.wlan_primary_netdev);
				DHD_TRACE(("%s : set ack suppress.TCPACK_SUP_HOLD \n",
					__FUNCTION__));
				dhd_tcpack_suppress_set(&dhd->pub, TCPACK_SUP_HOLD);
#endif /* DHDTCPACK_SUPPRESS && BCMPCIE */
				argos_rps_ctrl_data.argos_rps_cpus_enabled = 1;
				DHD_ERROR(("DHD: %s: Set RPS_CPUs, speed=%ld\n",
					__FUNCTION__, speed));
			}
		} else {
			DHD_ERROR(("DHD: %s: RPS_Set fail, Core=%d Offline\n", __FUNCTION__,
				RPS_CPUS_WLAN_CORE_ID));
		}
	} else if (speed <= RPS_TPUT_THRESHOLD && argos_rps_ctrl_data.wlan_primary_netdev != NULL) {
		custom_rps_map_clear(argos_rps_ctrl_data.wlan_primary_netdev->_rx);
		DHD_ERROR(("DHD: %s: Clear RPS_CPUs, speed=%ld\n", __FUNCTION__, speed));
		argos_rps_ctrl_data.argos_rps_cpus_enabled = 0;
		OSL_SLEEP(DELAY_TO_CLEAR_RPS_CPUS);
	}

	return NOTIFY_OK;
}
#endif /* ARGOS_CPU_SCHEDULER && ARGOS_RPS_CPU_CTL */

#ifdef DHD_BUZZZ_LOG_ENABLED

static int
dhd_buzzz_thread(void *data)
{
	tsk_ctl_t *tsk = (tsk_ctl_t *)data;

	DAEMONIZE("dhd_buzzz");

	/*  signal: thread has started */
	complete(&tsk->completed);

	/* Run until signal received */
	while (1) {
		if (down_interruptible(&tsk->sema) == 0) {
			if (tsk->terminated) {
				break;
			}
			printk("%s: start to dump...\n", __FUNCTION__);
			dhd_buzzz_dump();
		} else {
			break;
		}
	}
	complete_and_exit(&tsk->completed, 0);
}

void*
dhd_os_create_buzzz_thread(void)
{
	tsk_ctl_t *thr_buzzz_ctl = NULL;

	thr_buzzz_ctl = kmalloc(sizeof(tsk_ctl_t), KMALLOC_FLAG);
	if (!thr_buzzz_ctl) {
		return NULL;
	}

	PROC_START(dhd_buzzz_thread, NULL, thr_buzzz_ctl, 0, "dhd_buzzz");

	return (void *)thr_buzzz_ctl;
}

void
dhd_os_destroy_buzzz_thread(void *thr_hdl)
{
	tsk_ctl_t *thr_buzzz_ctl = (tsk_ctl_t *)thr_hdl;

	if (!thr_buzzz_ctl) {
		return;
	}

	PROC_STOP(thr_buzzz_ctl);
	kfree(thr_buzzz_ctl);
}

void
dhd_os_sched_buzzz_thread(void *thr_hdl)
{
	tsk_ctl_t *thr_buzzz_ctl = (tsk_ctl_t *)thr_hdl;

	if (!thr_buzzz_ctl) {
		return;
	}

	if (thr_buzzz_ctl->thr_pid >= 0) {
		up(&thr_buzzz_ctl->sema);
	}
}
#endif /* DHD_BUZZZ_LOG_ENABLED */

#ifdef DHD_DEBUG_PAGEALLOC
/* XXX Additional Kernel implemenation is needed to use this function at
 * the top of the check_poison_mem() function in mm/debug-pagealloc.c file.
 * Please check if below codes are implemenated your Linux Kernel first.
 *
 * - mm/debug-pagealloc.c
 *
 * // for DHD_DEBUG_PAGEALLOC
 * typedef void (*page_corrupt_cb_t)(void *handle, void *addr_corrupt, uint addr_len);
 * page_corrupt_cb_t corrupt_cb = NULL;
 * void *corrupt_cb_handle = NULL;
 *
 * void register_page_corrupt_cb(page_corrupt_cb_t cb, void *handle)
 * {
 *      corrupt_cb = cb;
 *      corrupt_cb_handle = handle;
 * }
 * EXPORT_SYMBOL(register_page_corrupt_cb);
 *
 * extern void dhd_page_corrupt_cb(void *handle, void *addr_corrupt, size_t len);
 *
 * static void check_poison_mem(unsigned char *mem, size_t bytes)
 * {
 * ......
 *
 *      if (!__ratelimit(&ratelimit))
 *               return;
 *      else if (start == end && single_bit_flip(*start, PAGE_POISON))
 *              printk(KERN_ERR "pagealloc: single bit error\n");
 *      else
 *              printk(KERN_ERR "pagealloc: memory corruption\n");
 *
 *      print_hex_dump(KERN_ERR, "", DUMP_PREFIX_ADDRESS, 16, 1, start,
 *              end - start + 1, 1);
 *
 *      // for DHD_DEBUG_PAGEALLOC
 *      dhd_page_corrupt_cb(corrupt_cb_handle, start, end - start + 1);
 *
 *      dump_stack();
 * }
 *
 */

void
dhd_page_corrupt_cb(void *handle, void *addr_corrupt, size_t len)
{
	dhd_pub_t *dhdp = (dhd_pub_t *)handle;

	DHD_ERROR(("%s: Got dhd_page_corrupt_cb 0x%p %d\n",
		__FUNCTION__, addr_corrupt, (uint32)len));

	DHD_OS_WAKE_LOCK(dhdp);
	prhex("Page Corruption:", addr_corrupt, len);
	dhd_dump_to_kernelog(dhdp);
#if defined(BCMPCIE) && defined(DHD_FW_COREDUMP)
	/* Load the dongle side dump to host memory and then BUG_ON() */
	dhdp->memdump_enabled = DUMP_MEMONLY;
	dhd_bus_mem_dump(dhdp);
#endif /* BCMPCIE && DHD_FW_COREDUMP */
	DHD_OS_WAKE_UNLOCK(dhdp);
}
EXPORT_SYMBOL(dhd_page_corrupt_cb);

#ifdef DHD_PKTID_AUDIT_ENABLED
void
dhd_pktid_audit_fail_cb(dhd_pub_t *dhdp)
{
	DHD_ERROR(("%s: Got Pkt Id Audit failure \n", __FUNCTION__));
	DHD_OS_WAKE_LOCK(dhdp);
#if defined(BCMPCIE) && defined(DHD_FW_COREDUMP)
	/* Load the dongle side dump to host memory and then BUG_ON() */
	dhdp->memdump_enabled = DUMP_MEMONLY;
	dhd_bus_mem_dump(dhdp);
#endif /* BCMPCIE && DHD_FW_COREDUMP */
	DHD_OS_WAKE_UNLOCK(dhdp);
}
#endif /* DHD_PKTID_AUDIT_ENABLED */
#endif /* DHD_DEBUG_PAGEALLOC */

/* XXX WAR: disable pm_bcnrx , scan_ps for BCM4354 WISOL module.
 * WISOL module have ANT_1 Rx sensitivity issue.
*/
#if defined(CUSTOMER_HW4)
void
dhd_force_disable_singlcore_scan(dhd_pub_t *dhd)
{
	int ret = 0;
	struct file *fp = NULL;
	char *filepath = "/data/.cid.info";
	char vender[10] = {0, };
	uint32 pm_bcnrx = 0;
	uint32 scan_ps = 0;

	if (BCM4354_CHIP_ID != dhd_bus_chip_id(dhd))
		return;

	fp = filp_open(filepath, O_RDONLY, 0);
	if (IS_ERR(fp)) {
		DHD_ERROR(("/data/.cid.info file open error\n"));
	} else {
		loff_t pos = 0;
		ret = kernel_read(fp, (char *)vender, 5, &pos);

		if (ret > 0 && NULL != strstr(vender, "wisol")) {
			DHD_ERROR(("wisol module : set pm_bcnrx=0, set scan_ps=0\n"));

			ret = dhd_iovar(dhd, 0, "pm_bcnrx", (char *)&pm_bcnrx, sizeof(pm_bcnrx),
					NULL, 0, TRUE);
			if (ret < 0)
				DHD_ERROR(("Set pm_bcnrx error (%d)\n", ret));

			ret = dhd_iovar(dhd, 0, "scan_ps", (char *)&scan_ps, sizeof(scan_ps), NULL,
					0, TRUE);
			if (ret < 0)
				DHD_ERROR(("Set scan_ps error (%d)\n", ret));
		}
		filp_close(fp, NULL);
	}
}
#endif /* CUSTOMER_HW4 */

struct net_device *
dhd_linux_get_primary_netdev(dhd_pub_t *dhdp)
{
	dhd_info_t *dhd = dhdp->info;

	if (dhd->iflist[0] && dhd->iflist[0]->net)
		return dhd->iflist[0]->net;
	else
		return NULL;
}

#if defined(STB) && !defined(STBAP)
static void
dhd_clear_wowl(dhd_pub_t *dhdp)
{
	int value = 0, err = BCME_OK;
	/* Clear the wowl functionality */
	err = dhd_iovar(dhdp, 0, "wowl_clear", NULL, 0, (char *)&value, sizeof(value), FALSE);
	if (err < 0) {
		DHD_ERROR(("%s: wowl_clear error, result=%d\n", __FUNCTION__, err));
	}
	/* Reset the wowl triggers */
	err = dhd_iovar(dhdp, 0, "wowl", NULL, 0, (char *)&value, sizeof(value), TRUE);
	if (err < 0) {
		DHD_ERROR(("%s: wowl_set error, result=%d\n", __FUNCTION__, err));
	}
}

static int
dhd_set_wowl_trigger(dhd_pub_t *dhdp, struct net_device *ndev)
{
	int err = 0;
	int value = 0;
	int wowl_trigger = 0;
#if defined(WL_CFG80211)
	int wowl_cfg_trigger = 0;
#endif /* WL_CFG80211 */

	err = dhd_iovar(dhdp, 0, "wowl", NULL, 0, (char *)&wowl_trigger, sizeof(value), FALSE);
	if (err < 0) {
		DHD_ERROR(("%s: error in get wowl_enable, result=%d\n", __FUNCTION__, err));
	}

#if defined(WL_CFG80211)
	wowl_cfg_trigger = wl_cfg80211_get_wowlan_triggers(ndev);

	/* No action required if already the triggers setby wl utility */
	if (((wowl_cfg_trigger & wowl_trigger) != wowl_cfg_trigger)) {
		wowl_trigger |= wowl_cfg_trigger;
		err = wl_cfg80211_set_wowlan_triggers(ndev, wowl_trigger);
		if (err < 0) {
			DHD_ERROR(("%s: error in get wowl_cfg_set,result=%d\n",
					__FUNCTION__, wowl_cfg_trigger));
		}
	}
#endif /* WL_CFG80211 */

	return wowl_trigger;
}

/* Enable or disable Wake-on-wireless LAN  for all buses */
void
dhd_set_wowl(dhd_pub_t *dhdp, int state)
{
	int err = 0;
	int value = 0;
	int wowl_trigger = 0;
	struct net_device *ndev;
	ndev = dhd_linux_get_primary_netdev(dhdp);
	DHD_OS_WAKE_LOCK_WAIVE(dhdp);

	/* Read the wowlan trigger, set by either wl utility or wpa_supplicant
	 * And activate the wowl
	 */
	if (state) {
		wowl_trigger = dhd_set_wowl_trigger(dhdp, ndev);
		if (wowl_trigger) {
			err = dhd_iovar(dhdp, 0, "wowl_activate", NULL, 0, (char *)&value,
					sizeof(value), FALSE);
			if (err < 0) {
				DHD_ERROR(("%s: error wowl_activate, err=%d\n", __FUNCTION__, err));
			} else
				DHD_ERROR(("%s: wowl activating with Trigger (%08x)\n",
					__FUNCTION__, wowl_trigger));
		}
	} else {
#if defined(WL_CFG80211)
		wl_cfg80211_update_wowl_wakeind(ndev);
#endif /* WL_CFG80211 */
		dhd_clear_wowl(dhdp);
	}

	DHD_OS_WAKE_LOCK_RESTORE(dhdp);
}

void
dhd_bus_set_wowl(void *drvinfo, int state)
{
	dhd_pub_t *dhdp;
	if (drvinfo) {
		dhdp = &((dhd_info_t *)drvinfo)->pub;
		dhd_set_wowl(dhdp, state);
	}
}
#endif /* STB && STBAP */

#ifdef DHD_DPSTA
#define EA_CMP(e1, e2) \
	(!((((uint16 *)(e1))[0] ^ ((uint16 *)(e2))[0]) | \
	(((uint16 *)(e1))[1] ^ ((uint16 *)(e2))[1]) | \
	(((uint16 *)(e1))[2] ^ ((uint16 *)(e2))[2])))

/**
 * Look all AP interfaces for downstream STA.
 */
bool
dhd_psta_is_ds_sta(void *dhd, dhd_pub_t *dhdp, struct ether_addr *mac)
{
	int i;

	for (i = 0; i < DHD_MAX_IFS; i++) {
		if (DHD_IF_ROLE_AP(dhdp, i) && dhd_find_sta(dhdp, i, mac)) {
			return TRUE;
		}
	}

	return FALSE;
}

static bool
dhd_psta_find_dpsta(void *dhd, dhd_pub_t *dhdp, uint8 *mac)
{
	dhd_if_t *ifp = NULL;
	int i = 0;

	/* psta_if is start from wlx.2 */
	for (i = IFIDX_PSTA_FIRST_PROXYINTF; i < DHD_MAX_IFS; i++) {
		ifp = dhd_get_ifp(dhdp, i);
		if (ifp && DHD_IF_ROLE_STA(dhdp, i)) {
			if (EA_CMP(mac, ifp->mac_addr))
				return TRUE;
		}
	}

	return FALSE;
}

bool
dhd_psta_authorized(void *dhd, dhd_pub_t *dhdp)
{
	if_flow_lkup_t *if_flow_lkup = (if_flow_lkup_t *)dhdp->if_flow_lkup;

	return (DHD_IF_ROLE_STA(dhdp, 0) && if_flow_lkup[0].status);
}

#ifdef DHD_PSTA
void
dhd_dpsta_psta_register(dhd_pub_t *dhd)
{
	psta_if_api_t api;

	DHD_ERROR(("dhd_dpsta_psta_register()\n"));

	/* Register proxy sta APIs with DPSTA module */
	api.is_ds_sta = (bool (*)(void *, void *, struct ether_addr *))dhd_psta_is_ds_sta;
	api.psta_find = (void *(*)(void *, void *, uint8 *))dhd_psta_find_dpsta;
	api.bss_auth = (bool (*)(void *, void *))dhd_psta_authorized;
	api.psta = dhd;
	api.bsscfg = dhd;
	api.mode = DPSTA_MODE_PSTA;
	(void) dpsta_register(dhd->unit, &api);

	return;
}

void
dhd_dpsta_dwds_register(dhd_pub_t *dhd)
{
	psta_if_api_t api;

	DHD_ERROR(("dhd_dpsta_dwds_register()\n"));

	/* Register proxy sta APIs with DPSTA module */
	api.is_ds_sta = (bool (*)(void *, void *, struct ether_addr *))dhd_psta_is_ds_sta;
	api.psta_find = (void *(*)(void *, void *, uint8 *))NULL;
	api.bss_auth = (bool (*)(void *, void *))dhd_psta_authorized;
	api.psta = dhd;
	api.bsscfg = dhd;
	api.mode = DPSTA_MODE_DWDS;
	(void) dpsta_register(dhd->unit, &api);

	return;
}
#endif /* DHD_PSTA */
#endif /* DHD_DPSTA */

#if defined(BCMDBG) || defined(DHD_DEBUG)
static void
dhd_dhcp_dump(uint8 *pktdata, bool tx)
{
	struct bootp_fmt *b = (struct bootp_fmt *) &pktdata[ETHER_HDR_LEN];
	struct iphdr *h = &b->ip_header;
	uint8 *ptr, *opt, *end = (uint8 *) b + ntohs(b->ip_header.tot_len);
	int dhcp_type = 0, len, opt_len;

	/* check IP header */
	if (h->ihl != 5 || h->version != 4 || h->protocol != IPPROTO_UDP) {
		return;
	}

	/* check UDP port for bootp (67, 68) */
	if (b->udp_header.source != htons(67) && b->udp_header.source != htons(68) &&
		b->udp_header.dest != htons(67) && b->udp_header.dest != htons(68)) {
		return;
	}

	/* check header length */
	if (ntohs(h->tot_len) < ntohs(b->udp_header.len) + sizeof(struct iphdr)) {
		return;
	}

	len = ntohs(b->udp_header.len) - sizeof(struct udphdr);
	opt_len = len
		- (sizeof(*b) - sizeof(struct iphdr) - sizeof(struct udphdr) - sizeof(b->options));

	/* parse bootp options */
	if (opt_len >= 4 && !memcmp(b->options, bootp_magic_cookie, 4)) {
		ptr = &b->options[4];
		while (ptr < end && *ptr != 0xff) {
			opt = ptr++;
			if (*opt == 0) {
				continue;
			}
			ptr += *ptr + 1;
			if (ptr >= end) {
				break;
			}
			/* 53 is dhcp type */
			if (*opt == 53) {
				if (opt[1]) {
					dhcp_type = opt[2];
					DHD_ERROR(("DHCP - %s [%s] [%s]\n",
						dhcp_types[dhcp_type], tx?"TX":"RX",
						dhcp_ops[b->op]));
					break;
				}
			}
		}
	}
}
#endif /* defined(BCMDBG) || defined(DHD_DEBUG) */

typedef struct dhd_mon_dev_priv {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36))
	struct rtnl_link_stats64 stats;
#else
	struct net_device_stats stats;
#endif /* KERNEL_VERSION >= 2.6.36 && !defined(BCM_DHD_RUNNER) */
} dhd_mon_dev_priv_t;

#define DHD_MON_DEV_PRIV_SIZE		(sizeof(dhd_mon_dev_priv_t))
#define DHD_MON_DEV_PRIV(dev)		((dhd_mon_dev_priv_t *)DEV_PRIV(dev))
#define DHD_MON_DEV_STATS(dev)		(((dhd_mon_dev_priv_t *)DEV_PRIV(dev))->stats)

static int
dhd_monitor_start(struct sk_buff *skb, struct net_device *dev)
{
	PKTFREE(NULL, skb, FALSE);

	return 0;
}

static int
dhd_monitor_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	return 0;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36))
static STATS64_RETURN_TYPE
dhd_monitor_get_stats(struct net_device *dev, struct rtnl_link_stats64 *stats)
{
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 36))
	/* since 2.6.37 the return pointer is ignored so need to memcpy */
	memcpy(stats, &DHD_MON_DEV_STATS(dev), sizeof(*stats));
#endif /* KERNEL_VERSION > 2.6.36 */

	RETURN_STATS64(&DHD_MON_DEV_STATS(dev));
}
#else
static struct net_device_stats*
dhd_monitor_get_stats(struct net_device *dev)
{
	return &DHD_MON_DEV_STATS(dev);
}
#endif /* KERNEL_VERSION >= 2.6.36 */

static const struct net_device_ops netdev_monitor_ops =
{
	.ndo_start_xmit = dhd_monitor_start,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36))
	.ndo_get_stats64 = dhd_monitor_get_stats,
#else
	.ndo_get_stats = dhd_monitor_get_stats,
#endif /* KERNEL_VERSION >= 2.6.36 */
	.ndo_do_ioctl = dhd_monitor_ioctl
};

#ifdef WL_MONITOR
static void
dhd_add_monitor_if(void *handle, void *event_info, enum dhd_wq_event event)
{
	dhd_info_t *dhd = handle;
	uint monitor_type = (uint) (long) event_info;
	struct net_device *dev;
	char *devname;

	if (event != DHD_WQ_WORK_IF_ADD) {
		DHD_ERROR(("%s: unexpected event \n", __FUNCTION__));
		return;
	}

	if (monitor_type < 1 || monitor_type > 3) {
		DHD_ERROR(("%s: monitor_type %d out of range\n", __FUNCTION__,
			monitor_type));
		return;
	}

	if (!dhd) {
		DHD_ERROR(("%s: dhd info not available \n", __FUNCTION__));
		return;
	}

	dev = alloc_etherdev(DHD_MON_DEV_PRIV_SIZE);
	if (!dev) {
		DHD_ERROR(("%s: alloc wlif failed\n", __FUNCTION__));
		return;
	}

	if (monitor_type == 1)
		devname = "prism";
	else
		devname = "radiotap";

	snprintf(dev->name, sizeof(dev->name), "%s%u", devname, dhd->pub.unit);

#ifndef ARPHRD_IEEE80211_PRISM  /* From Linux 2.4.18 */
#define ARPHRD_IEEE80211_PRISM 802
#endif

#ifndef ARPHRD_IEEE80211_RADIOTAP
#define ARPHRD_IEEE80211_RADIOTAP	803 /* IEEE 802.11 + radiotap header */
#endif /* ARPHRD_IEEE80211_RADIOTAP */

	if (monitor_type == 1)
		dev->type = ARPHRD_IEEE80211_PRISM;
	else
		dev->type = ARPHRD_IEEE80211_RADIOTAP;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 31))
	dev->hard_start_xmit = dhd_monitor_start;
	dev->do_ioctl = dhd_monitor_ioctl;
	dev->get_stats = dhd_monitor_get_stats;
#else
	dev->netdev_ops = &netdev_monitor_ops;
#endif

	if (register_netdev(dev)) {
		DHD_ERROR(("%s, register_netdev failed for %s\n",
			__FUNCTION__, dev->name));
		free_netdev(dev);
		return;
	}

	DHD_OS_WAKE_LOCK(&dhd->pub);
	DHD_LOCK(&dhd->pub);

	if (dhd->monitor_dev) {
		DHD_ERROR(("%s: monitor already enabled\n", __FUNCTION__));
		DHD_UNLOCK(&dhd->pub);
		DHD_OS_WAKE_UNLOCK(&dhd->pub);

		unregister_netdev(dev);
		free_netdev(dev);

		return;
	}

	bcmwifi_monitor_create(&dhd->monitor_info);
	dhd->monitor_info->corerev = dhd->corerev;
	dhd->monitor_dev = dev;
	dhd->monitor_type = monitor_type;

	DHD_UNLOCK(&dhd->pub);
	DHD_OS_WAKE_UNLOCK(&dhd->pub);
}

static void
dhd_del_monitor_if(void *handle, void *event_info, enum dhd_wq_event event)
{
	dhd_info_t *dhd = handle;
	struct net_device *monitor_dev = NULL;

	if (event != DHD_WQ_WORK_IF_DEL) {
		DHD_ERROR(("%s: unexpected event \n", __FUNCTION__));
		return;
	}

	if (!dhd) {
		DHD_ERROR(("%s: dhd info not available \n", __FUNCTION__));
		return;
	}

	DHD_OS_WAKE_LOCK(&dhd->pub);
	DHD_LOCK(&dhd->pub);

	if (dhd->monitor_dev) {
		monitor_dev = dhd->monitor_dev;
		dhd->monitor_dev = NULL;
	}

	DHD_UNLOCK(&dhd->pub);
	DHD_OS_WAKE_UNLOCK(&dhd->pub);

	if (monitor_dev) {
		unregister_netdev(monitor_dev);
		free_netdev(monitor_dev);
	}
}

void
dhd_set_monitor(dhd_pub_t *dhd, int ifidx, int val)
{
	dhd_info_t *info = dhd->info;
	wlc_rev_info_t revinfo;
	int ret;

	DHD_TRACE(("%s: val %d\n", __FUNCTION__, val));
	if ((val && info->monitor_dev) || (!val && !info->monitor_dev)) {
		DHD_ERROR(("%s: Mismatched params, return\n", __FUNCTION__));
		return;
	}

	/* Delete monitor */
	if (!val) {
		info->monitor_type = val;
		dhd_deferred_schedule_work(info->dhd_deferred_wq, NULL, DHD_WQ_WORK_IF_DEL,
			dhd_del_monitor_if, DHD_WORK_PRIORITY_LOW);
		return;
	}
	ret = dhd_wl_ioctl_cmd(dhd, WLC_GET_REVINFO, &revinfo, sizeof(revinfo), FALSE, 0);
	if (ret < 0) {
		DHD_ERROR(("%s: GET revinfo FAILED\n", __FUNCTION__));
	}
	info->corerev = revinfo.corerev;
	/* Add monitor */
	if (val >= 1 && val <= 3) {
		dhd_deferred_schedule_work(info->dhd_deferred_wq, (void *)(long)val,
			DHD_WQ_WORK_IF_ADD, dhd_add_monitor_if, DHD_WORK_PRIORITY_LOW);
	} else {
		DHD_ERROR(("monitor type %d not supported\n", val));
		ASSERT(0);
	}
}

#endif /* WL_MONITOR */

#if defined(BCM_WFD)
void
dhd_wfd_invoke_func(int processor_id, void (*func)(struct dhd_bus *bus))
{
	dhd_perim_invoke_all(processor_id, func);
}
#endif /* BCM_WFD */

#if defined(BCM_DHD_RUNNER)
/*
 * Find and return the active station count of the STA
 * with MAC address ea in an interface's STA list
 *
 */
uint16
dhd_get_sta_cnt(void *pub, int ifidx, void *ea)
{
	dhd_sta_t *sta;
	dhd_if_t *ifp;
	unsigned long flags;
	uint16 cnt = 0;

	ASSERT(ea != NULL);
	ifp = dhd_get_ifp((dhd_pub_t *)pub, ifidx);
	if (ifp == NULL)
	    return ID16_INVALID;

	DHD_IF_STA_LIST_LOCK(ifp, flags);

	list_for_each_entry(sta, &ifp->sta_list, list) {
	    cnt++;
	    if (!memcmp(sta->ea.octet, ea, ETHER_ADDR_LEN)) {
	        DHD_IF_STA_LIST_UNLOCK(ifp, flags);
	        return cnt;
	    }
	}

	DHD_IF_STA_LIST_UNLOCK(ifp, flags);

	return ID16_INVALID;
}
#if !defined(BCM_COUNTER_EXTSTATS)
/*
 * function to update network device's stats which counted in fastpath
 *
 */
static void
dhd_update_fp_stats(struct net_device * dev_p, BlogStats_t * blogStats_p)
{

	if (dev_p == (struct net_device *)NULL)
		return;
	else {
		dhd_if_t *ifp = DHD_DEV_IFP(dev_p);
		BlogStats_t *bStats_p;

		if (!ifp)
			return;

		bStats_p = &(ifp->b_stats);
		bStats_p->rx_packets += blogStats_p->rx_packets;
		bStats_p->tx_packets += blogStats_p->tx_packets;
		bStats_p->rx_bytes   += blogStats_p->rx_bytes;
		bStats_p->tx_bytes   += blogStats_p->tx_bytes;
		bStats_p->multicast  += blogStats_p->multicast;
	}

	return;
}

static void
*dhd_get_stats_pointer(struct net_device *dev_p, char type)
{
	dhd_if_t *ifp = DHD_DEV_IFP(dev_p);

	if (unlikely(!ifp))
		return NULL;

	switch (type) {
		case 'd': /* slow path stats */
			return &(ifp->stats);
		case 'c':  /* accumlate stats */
			return &(ifp->c_stats);
		case 'b': /* hw b_stats */
			return &(ifp->b_stats);
	}

	/* it should never come here */
	return NULL;
}

static void
dhd_clear_stats(struct net_device *net)
{
	net_dev_clear_stats(net);
}
#endif
#endif /* BCM_DHD_RUNNER  */

#if defined(BCM_ROUTER_DHD)
dhd_pub_t *
dhd_dev_get_dhdpub(struct net_device *dev)
{
	struct dhd_info *dhd = DHD_DEV_INFO(dev);

	return &(dhd->pub);
}

int
dhd_dev_get_ifidx(struct net_device *dev)
{
	return DHD_DEV_IFIDX(dev);
}

void
dhd_if_inc_txpkt_cnt(dhd_pub_t *dhdp, int ifidx, void *pkt)
{
	dhd_if_t *ifp = dhd_get_ifp(dhdp, ifidx);

	if (unlikely(ifp == (dhd_if_t *)NULL))
		return;

	ifp->stats.tx_packets++;
	ifp->stats.tx_bytes += PKTLEN(dhdp->osh, pkt);
}

void
dhd_if_inc_rxpkt_cnt(dhd_pub_t *dhdp, int ifidx, uint32 pktlen)
{
	dhd_if_t *ifp = dhd_get_ifp(dhdp, ifidx);

	if (unlikely(ifp == (dhd_if_t *)NULL))
		return;

	ifp->stats.rx_packets++;
	ifp->stats.rx_bytes += pktlen;
}

void
dhd_if_inc_txpkt_drop_cnt(dhd_pub_t *dhdp, int ifidx)
{
	dhd_if_t *ifp = dhd_get_ifp(dhdp, ifidx);

	if (unlikely(ifp == (dhd_if_t *)NULL))
		return;

	ifp->stats.tx_dropped++;
}

/* increase per interface drop stats by cnt */
void
dhd_if_add_txpkt_drop_cnt(dhd_pub_t *dhdp, int ifidx, int cnt)
{
	dhd_if_t *ifp = dhd_get_ifp(dhdp, ifidx);

	if (unlikely(ifp == (dhd_if_t *)NULL))
		return;

	ifp->stats.tx_dropped += cnt;
} /* dhd_if_add_txpkt_drop_cnt() */

void
dhd_if_clear_stats(dhd_pub_t *dhdp, int ifidx)
{
	dhd_if_t *ifp = dhd_get_ifp(dhdp, ifidx);

	if (unlikely(ifp == (dhd_if_t *)NULL))
		return;

	memset(&ifp->stats, 0, sizeof(struct net_device_stats));
}

#endif /* BCM_ROUTER_DHD */

uint8 *BCMFASTPATH
dhd_if_get_macaddr(dhd_pub_t *dhdp, int ifidx)
{
	dhd_if_t *ifp = dhd_get_ifp(dhdp, ifidx);

	if (unlikely(ifp == (dhd_if_t *)NULL))
		return dhdp->mac.octet;

	return ifp->mac_addr;
}

#ifdef WL_MONITOR

bool
dhd_monitor_enabled(dhd_pub_t *dhd, int ifidx)
{
	return (dhd->info->monitor_type != 0);
}

void
dhd_rx_mon_pkt(dhd_pub_t *dhdp, host_rxbuf_cmpl_t* msg, void *pkt, int ifidx)
{
	dhd_info_t *dhd = (dhd_info_t *)dhdp->info;
	uint16 len = 0;
	monitor_pkt_info_t pkt_info = {0};
	uint16 pkt_type;
	int16 offset = 0;
	uint8 dma_flags = 0xff;
	wl_phyextract_t *phyextract = NULL;
	wlc_d11rxhdr_t *wrxh;
	d11rxhdr_t *rxhdr;
	if (!dhd->monitor_info || (skb_headroom(pkt) < sizeof(wl_radiotap_vht_t))) {
		PKTFREE(dhdp->osh, pkt, FALSE);
		return;
	}
	if (!BCM43684_CHIP(dhd_bus_chip_id(dhdp))) {
		memcpy(&pkt_info.marker, &msg->rx_status_0, sizeof(msg->rx_status_0));
	}
	if (!dhd->monitor_skb) {
		dhd->monitor_skb = (struct sk_buff *)pkt;
	}
	if (skb_tailroom(dhd->monitor_skb) < DHD_RX_PKT_BUFSZ) {
		if (dhd->monitor_skb != pkt) {
			PKTFREE(dhdp->osh, dhd->monitor_skb, FALSE);
		}
		PKTFREE(dhdp->osh, pkt, FALSE);
		dhd->monitor_skb = NULL;
		dhd->monitor_info->amsdu_len = 0;
		dhd->monitor_info->amsdu_pkt = NULL;
		return;
	}

	/* Handling different wlc_d11rxhdr_t for 4366 and 43602 */
	if (BCM43602_CHIP(dhd_bus_chip_id(dhdp))) {
		dma_flags = 0;
	}
	/* currently phyextract is only added to the packet in corev >= 128
	 * TODO: for corev < 128
	 */
	if (dhd->monitor_info->corerev >= 128) {
		phyextract  = (wl_phyextract_t *)PKTDATA(dhdp->osh, pkt);
		skb_pull(pkt, sizeof(wl_phyextract_t));
	}
	wrxh = (wlc_d11rxhdr_t*)PKTDATA(dhdp->osh, pkt);

	if (BCM43602_CHIP(dhd_bus_chip_id(dhdp)) || BCM4365_CHIP(dhd_bus_chip_id(dhdp))) {
		/* wlc_d11rxhdr  is different in kudu from eagle branch. d11rxhdr_t is at
		 * the start of the wlc_d11rxhdr. wlc_d11rxhdr_t was added back to the packet
		 * in firmware
		 */
		rxhdr = (d11rxhdr_t *)wrxh;
	} else {
		rxhdr = &wrxh->rxhdr;
	}

	len = bcmwifi_monitor(dhd->monitor_info, &pkt_info, PKTDATA(dhdp->osh, pkt),
			PKTLEN(dhdp->osh, pkt),  PKTDATA(dhdp->osh, dhd->monitor_skb),
			&offset, &pkt_type, dma_flags, phyextract, rxhdr);
	if (pkt_type == MON_PKT_AMSDU_ERROR)
	{
		if (dhd->monitor_skb != pkt) {
			PKTFREE(dhdp->osh, dhd->monitor_skb, FALSE);
			dhd->monitor_skb = pkt;
			len = bcmwifi_monitor(dhd->monitor_info, &pkt_info, PKTDATA(dhdp->osh, pkt),
				PKTLEN(dhdp->osh, pkt),	PKTDATA(dhdp->osh, dhd->monitor_skb),
				&offset, &pkt_type, dma_flags, (void *)phyextract, rxhdr);
			if (pkt_type == MON_PKT_AMSDU_ERROR) {
				PKTFREE(dhdp->osh, pkt, FALSE);
				dhd->monitor_skb = NULL;
				return;
			}
		}
		else {
			PKTFREE(dhdp->osh, dhd->monitor_skb, FALSE);
			dhd->monitor_skb = NULL;
			return;
		}
	}

	if (dhd->monitor_type && dhd->monitor_dev)
		dhd->monitor_skb->dev = dhd->monitor_dev;
	else {
		PKTFREE(dhdp->osh, pkt, FALSE);
		dhd->monitor_skb = NULL;
		return;
	}
	if (pkt_type == MON_PKT_AMSDU_INTERMEDIATE || pkt_type == MON_PKT_AMSDU_LAST) {
		PKTFREE(dhdp->osh, pkt, FALSE);
		if (skb_tailroom(dhd->monitor_skb) >= len)
		{
			skb_put(dhd->monitor_skb, len);
		}
	}

	if (pkt_type == MON_PKT_AMSDU_FIRST || pkt_type == MON_PKT_AMSDU_INTERMEDIATE) {
		return;
	}

	if (offset > 0) {
		skb_push(dhd->monitor_skb, offset);
	}
	else {
		skb_pull(dhd->monitor_skb, -(offset));
	}
	dhd->monitor_skb->protocol = eth_type_trans(dhd->monitor_skb, dhd->monitor_skb->dev);

	if (in_interrupt()) {
		bcm_object_trace_opr(skb, BCM_OBJDBG_REMOVE,
			__FUNCTION__, __LINE__);
		DHD_UNLOCK(dhdp);
		netif_rx(dhd->monitor_skb);
		DHD_LOCK(dhdp);
	} else {
		/* If the receive is not processed inside an ISR,
		 * the softirqd must be woken explicitly to service
		 * the NET_RX_SOFTIRQ.	In 2.6 kernels, this is handled
		 * by netif_rx_ni(), but in earlier kernels, we need
		 * to do it manually.
		 */
		bcm_object_trace_opr(dhd->monitor_skb, BCM_OBJDBG_REMOVE,
			__FUNCTION__, __LINE__);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0)
		DHD_UNLOCK(dhdp);
		netif_rx_ni(dhd->monitor_skb);
		DHD_LOCK(dhdp);
#else
		ulong flags;
		DHD_UNLOCK(dhdp);
		netif_rx(dhd->monitor_skb);
		DHD_LOCK(dhdp);
		local_irq_save(flags);
		RAISE_RX_SOFTIRQ();
		local_irq_restore(flags);
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0) */
	}
	dhd->monitor_skb = NULL;
}

#endif /* WL_MONITOR */

#ifdef BCM_NBUFF_WLMCAST
int g_multicast_priority = -1;

#if defined(BCM_DHD_RUNNER) || defined(BCM_AWL)
/**
 * function to return dhd client information from bridge snooping
 *
 * ret:         WLAN_CLIENT_INFO_OK-	 info_p is filled
 *              WLAN_CLIENT_INFO_ERR-	 could not find the STA, action to take, delete blog etc..
 */
int
dhd_client_get_info(struct net_device *dev, char *mac, int priority, wlan_client_info_t *info_p)
{
	/* default the type to CPU */
	info_p->type = WLAN_CLIENT_TYPE_CPU;
#if defined(BCM_WFD) && defined(BCM_BLOG)
	if (dev && mac)  {
		dhd_dev_priv_t * dev_priv;
		dhd_if_t *ifp = NULL;
		dhd_pub_t *dhdp = NULL;
		dev_priv = DHD_DEV_PRIV(dev);

		if (dev_priv)
			ifp = dev_priv->ifp;
		if (!ifp) {
			DHD_TRACE(("%s: Invalid parameter [%s]\n", __FUNCTION__, dev->name));
			return WLAN_CLIENT_INFO_ERR;
		}

		dhdp = &ifp->info->pub;
		if (!WFD_ENABLED(dhdp)) {
			/* WFD is not enabled for this radio */
			return WLAN_CLIENT_INFO_ERR;
		}

		if (ifp->wmf.wmf_enable)  {
#ifdef BCM_WLAN_PER_CLIENT_FLOW_LEARNING
			uint16_t staidx = dhd_if_get_staidx(dhdp, ifp->idx, mac);
			info_p->wfd.mcast.sta_id = (staidx == ID16_INVALID) ? 0 : staidx;
			info_p->wfd.mcast.is_wmf_enabled = 1;
#else
			dhd_sta_t *sta, *next;
			unsigned long flags;
			int found = 0;
			DHD_INFO((" if:%s WMF enabled \r\n", dev->name));
			if ((priority >= NUMPRIO) || (priority < 0)) {
				DHD_INFO(("priroty input is wrong, set to 5(AC_VI)  \r\n"));
				priority = PRIO_8021D_VI;
			}
			g_multicast_priority = priority;
			/* when WMF is enabled, searching for the STA and fill the client_info_t */
			DHD_IF_STA_LIST_LOCK(ifp, flags);
			list_for_each_entry_safe(sta, next, &ifp->sta_list, list) {
				if (!memcmp(sta->ea.octet, mac, ETHER_ADDR_LEN)) {
					found = 1;
					DHD_INFO(("found: dhd_client_get_info:sta->ea:%pM \r\n",
							&(sta->ea)));
			switch (dhd_flowid_get_type(dhdp, sta, priority)) {
				case WLAN_CLIENT_TYPE_RUNNER:
					info_p->type = WLAN_CLIENT_TYPE_RUNNER;
					info_p->rnr.is_wfd = 0;
					info_p->rnr.is_tx_hw_acc_en = 1;
					info_p->rnr.radio_idx = dhdp->unit;
					info_p->rnr.priority = priority;
					info_p->rnr.ssid = sta->ifidx;
					info_p->rnr.flowring_idx = sta->flowid[priority];
					DHD_IF_STA_LIST_UNLOCK(ifp, flags);
					return WLAN_CLIENT_INFO_OK;
				case WLAN_CLIENT_TYPE_WFD:
					break;
				default:
					DHD_IF_STA_LIST_UNLOCK(ifp, flags);
					return WLAN_CLIENT_INFO_ERR;
				}
				}
			}
			DHD_IF_STA_LIST_UNLOCK(ifp, flags);
			if (!found)  {
				printk("did not found sta:%pM\r\n", mac);
				return WLAN_CLIENT_INFO_ERR;
			}
#endif /* BCM_WLAN_PER_CLIENT_FLOW_LEARNING */
		}

		/* when WMF is disabled, Or  flowringID is in M range,
		 * runner should forward the packet to WMF for forwaring, fill
		 * in BlogWfd_t information
		 */
		DHD_INFO((" if:%s WMF disabled or falls into RUNNER-M case\r\n", dev->name));
		info_p->type = WLAN_CLIENT_TYPE_WFD;

		info_p->wfd.mcast.is_tx_hw_acc_en = 1;
		DHD_INFO((" wfd.mcast.is_valid:%d \r\n", info_p->wfd.mcast.is_tx_hw_acc_en));
		info_p->wfd.mcast.is_wfd = 1;
		info_p->wfd.mcast.is_chain = 0;
		DHD_INFO(("WFD idx for this :%d  \r\n", dhdp->wfd_idx));
		info_p->wfd.mcast.wfd_idx = dhdp->wfd_idx;
		info_p->wfd.mcast.wfd_prio = 0; /* always put mutlicast onto high priority queue */
		info_p->wfd.mcast.ssid =  dhd_net2idx(ifp->info, dev);

		return WLAN_CLIENT_INFO_OK;

	} else
		return WLAN_CLIENT_INFO_ERR;
#else
	/* in case WFD is not enabled, it will return CPU type and Error */
	return WLAN_CLIENT_INFO_ERR;
#endif /* defined(BCM_WFD) && defined(BCM_BLOG) */
}
#endif  /* BCM_DHD_RUNNER || BCM_AWL */

/* After WMF handle packet, update if stats, invoked from dhd_wmf_linux.c as
 * dhd_if_t and dhd_get_ifp is static defines in this file
 */
void
dhd_wmf_update_if_stats(void *dhd_p, int ifidx, int wmf_action,
		unsigned int pktlen, bool frombss)
{
	dhd_pub_t *dhdp = (dhd_pub_t*)dhd_p;
	dhd_if_t  *ifp = dhd_get_ifp(dhdp, ifidx);

	if (wmf_action == WMF_TAKEN) {
		dhdp->tx_packets++;
		ifp->stats.tx_packets++;
		ifp->stats.tx_bytes += pktlen;
	} else if (wmf_action == WMF_DROP) {
		ifp->stats.tx_dropped++;
	}

	return;
}

int32
dhd_wmf_stall_sta_check_fn(void *wrapper, void *txif, uint32 mgrp_ip)
{
	dhd_wmf_wrapper_t *wmfh = (dhd_wmf_wrapper_t*)wrapper;
	dhd_pub_t *dhdp = wmfh->dhdp;
	flow_ring_node_t *flow_ring_node;
	flow_queue_t	*flow_queue;
	dhd_sta_t *sta = (dhd_sta_t *)txif;
	int i = 0;

	for (i = 0; i < NUMPRIO; ++i) {
		if (sta->flowid[i] != FLOWID_INVALID) {
			flow_ring_node = DHD_FLOW_RING(dhdp, sta->flowid[i]);
			flow_queue = &flow_ring_node->queue;
			if (flow_ring_node->status == FLOW_RING_STATUS_OPEN)
				dhd_bus_flow_ring_flush_request(dhdp->bus, flow_ring_node);
		}
	}

	return 0;
}

int32
dhd_wmf_forward_fn(void *wrapper, void *p, uint32 mgrp_ip, void *txif, int rt_port)
{
	dhd_wmf_wrapper_t *wmfh = (dhd_wmf_wrapper_t*)wrapper;
	dhd_pub_t *dhdp = wmfh->dhdp;
	dhd_if_t *ifp = dhd_get_ifp(dhdp, wmfh->bssidx);

	if (rt_port&0x80) {
		rt_port &= 0x7f;
		if (!list_empty(&ifp->sta_list)) {
			dhd_sta_t *sta;
			void *sdu_clone;
			list_for_each_entry(sta, &ifp->sta_list, list) {
				if (sta == txif) continue; /* don't send out to the incoming sta */
				sdu_clone = IS_SKBUFF_PTR(p)? PKTDUP_CPY(dhdp->osh, p):
					PKTDUP(dhdp->osh, p);
				if (!sdu_clone)
					break;
				dhd_wmf_forward(wrapper, sdu_clone, 0, sta, (bool)rt_port);
			}
		}
		if (rt_port) {
			PKTFREE(dhdp->osh, p, TRUE);
			return WMF_TAKEN;
		} else
			return WMF_NOP;
	} else {
		dhd_wmf_forward(wrapper, p, mgrp_ip, txif, rt_port);
		return EMF_TAKEN;
	}
}

void *
dhd_wmf_get_device(void *dhd_p, int idx)
{
	dhd_pub_t *dhdp = (dhd_pub_t*)dhd_p;
	dhd_info_t *dhd = (dhd_info_t *)dhdp->info;
	dhd_if_t *ifp;

	ASSERT(idx < DHD_MAX_IFS);
	ifp = dhd->iflist[idx];

	return ifp->net;
}

#ifdef BCM_NBUFF_WLMCAST_IPV6

void *
dhd_wmf_get_instance(void *pdev)
{
	if (pdev) {
		dhd_dev_priv_t * dev_priv;
		struct net_device *dev = pdev;
		dhd_if_t *dhdif;
		dev_priv = DHD_DEV_PRIV(dev);
		dhdif = dev_priv->ifp;

		if (dhdif)
			return dhdif->wmf.wmf_instance;
	}

	return NULL;
}

static void *
dhd_wmf_sdbfind(void *pdev, void *mac)
{
	if (pdev) {
		dhd_dev_priv_t * dev_priv;
		struct net_device *dev = pdev;
		dhd_info_t *dhd;
		dev_priv = DHD_DEV_PRIV(dev);
		dhd = dev_priv->dhd;
		if (dhd)
			return dhd_find_sta(&dhd->pub, dev_priv->ifidx, mac);
	}

	return NULL;
}

void *
dhd_wmf_get_igsc(void *pdev)
{
	dhd_wmf_instance_t *wmf_inst = dhd_wmf_get_instance(pdev);
	if (wmf_inst) return wmf_inst->igsci;

	return NULL;
}

static void *
dhd_nic_hook_fn(int cmd, void *p, void *p2)
{
	switch (cmd) {
		case WLEMF_CMD_GETIGSC:	 /* get wmf instance */
			return 	dhd_wmf_get_igsc(p);
		case WLEMF_CMD_SCBFIND:
			/* find scb from associated device */
			return dhd_wmf_sdbfind(p, p2);
		default:
			break;
	}

	return NULL;
}

#endif /* BCM_NBUFF_WLMCAST_IPV6 */

#endif  /* BCM_NBUFF_WLMCAST */

#ifdef WL_CFG80211
static s32
dhd_cfg80211_enabled(void)
{
#if defined(BCM_ROUTER_DHD) && !defined(MEDIA_CFG)
	char * var;
	int cfg_hapd_enable = 0;

	if ((var = getvar(NULL, "hapd_enable")) == NULL) {
		return BCME_OK;
	}
	cfg_hapd_enable = bcm_strtoul(var, NULL, 0);

	if (!cfg_hapd_enable)
		return BCME_UNSUPPORTED;
#endif /* BCM_ROUTER_DHD && !MEDIA_CFG */

	return BCME_OK;
}
#endif /* WL_CFG80211 */

#ifdef BCM_GMAC3

int
dhd_set_1905_almac(dhd_pub_t *dhdp, uint8 ifidx, uint8* ea, bool mcast)
{
	dhd_if_t *ifp;

	ASSERT(ea != NULL);
	ifp = dhd_get_ifp(dhdp, ifidx);
	if (ifp == NULL) {
		return BCME_ERROR;
	}

	if (mcast) {
		memcpy(ifp->_1905_al_mcast, ea, ETHER_ADDR_LEN);
	} else {
		memcpy(ifp->_1905_al_ucast, ea, ETHER_ADDR_LEN);
	}

	return BCME_OK;
}

int
dhd_get_1905_almac(dhd_pub_t *dhdp, uint8 ifidx, uint8* ea, bool mcast)
{
	dhd_if_t *ifp;

	ASSERT(ea != NULL);
	ifp = dhd_get_ifp(dhdp, ifidx);
	if (ifp == NULL) {
		return BCME_ERROR;
	}

	if (mcast) {
		memcpy(ea, ifp->_1905_al_mcast, ETHER_ADDR_LEN);
	} else {
		memcpy(ea, ifp->_1905_al_ucast, ETHER_ADDR_LEN);
	}

	return BCME_OK;
}
#endif /* BCM_GMAC3 */

uint32
dhd_get_macdbg_dump_level(struct dhd_info *dhd)
{
	return dhd->macdbg_dump_level;
}

void
dhd_set_macdbg_dump_level(struct dhd_info *dhd, uint32 macdbg_dump_level)
{
	dhd->macdbg_dump_level = macdbg_dump_level;
}
