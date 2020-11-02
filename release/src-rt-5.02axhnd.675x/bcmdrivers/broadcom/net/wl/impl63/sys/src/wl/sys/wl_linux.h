/*
 * wl_linux.c exported functions and definitions
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
 * $Id: wl_linux.h 789025 2020-07-16 17:54:34Z $
 */

#ifndef _wl_linux_h_
#define _wl_linux_h_

#include <wlc_types.h>
#include <wlc_objregistry.h>
#include <wlc_pub.h>

#ifdef USE_IW
#include <wl_iw.h>
#endif // endif

#ifdef AVS
#include <wl_avs.h>
#endif // endif

#define WL_LOCK_TAKEN    (TRUE)
#define WL_LOCK_NOTTAKEN (FALSE)

struct d3fwd_wlif;

typedef struct wl_timer {
	struct timer_list	timer;
	struct wl_info		*wl;
	void	(*fn)(void *);
	void	*arg; /**< argument to fn */
	uint	ms;
	bool	periodic;
	bool	set;
	struct wl_timer		*next;
#ifdef BCMDBG
	char*	name; /**< Description of the timer */
	uint32	ticks;	/**< how many timer timer fired */
#endif // endif
#ifdef WL_ALL_PASSIVE
	atomic_t sched; /**< task is scheduled for this timer */
#endif /* WL_ALL_PASSIVE */
} wl_timer_t;

/* contortion to call functions at safe time */
/* In 2.6.20 kernels work functions get passed a pointer to the struct work, so things
 * will continue to work as long as the work structure is the first component of the task structure.
 */
typedef struct wl_task {
	struct work_struct work;
	void *context;
} wl_task_t;

/* This becomes netdev->priv and is the link between netdev and wlif struct */
typedef struct priv_link {
	wl_if_t *wlif;
} priv_link_t;

#define WL_DEV_IF(dev)		((wl_if_t*)((priv_link_t*)DEV_PRIV(dev))->wlif)

#define WL_INFO_GET(dev)	((wl_info_t*)(WL_DEV_IF(dev)->wl))	/* dev to wl_info_t */

#define TXQ_LOCK(_wl) spin_lock_bh(&(_wl)->txq_lock)
#define TXQ_UNLOCK(_wl) spin_unlock_bh(&(_wl)->txq_lock)

#ifdef CONFIG_SMP
#define WL_CONFIG_SMP()		TRUE
#else
#define WL_CONFIG_SMP()		FALSE
#endif /* CONFIG_SMP */

/* With WFD max interfaces supported should not exceed WIFI_MW_MAX_NUM_IF */
#define WL_MAX_IFS (16U)

#define WL_IFTYPE_BSS	1 /**< iftype subunit for BSS */
#define WL_IFTYPE_WDS	2 /**< iftype subunit for WDS */
#define WL_IFTYPE_MON	3 /**< iftype subunit for MONITOR */

struct wl_if {
#ifdef USE_IW
	wl_iw_t		iw;		/**< wireless extensions state (must be first) */
#else
#ifdef BCM_NBUFF_WLMCAST_IPV6
	/* must be the first entry when USE_IW is not defined */
	void *(*nic_hook_fn)(int cmd, void *p, void *p2);
#endif // endif
#endif /* USE_IW */
	struct wl_if *next;
	struct wl_info *wl;		/**< back pointer to main wl_info_t */
	struct net_device *dev;		/**< virtual netdevice */
#if defined(BCM_GMAC3)
	struct fwder *fwdh;     /**< pointer to forwarder handle */
#endif /* BCM_GMAC3 */
	struct wlc_if *wlcif;		/**< wlc interface handle */
	uint subunit;			/**< assigned in wl_add_if(), index of the wlif if any,
					* not necessarily corresponding to bsscfg._idx or
					* AID2PVBMAP(scb).
					*/
	bool dev_registered;	/**< netdev registed done */
	int  if_type;			/**< WL_IFTYPE */
	char name[IFNAMSIZ];		/**< netdev may not be alloced yet, so store the name
					   here temporarily until the netdev comes online
					 */
#ifdef ARPOE
	wl_arp_info_t	*arpi;		/**< pointer to arp agent offload info */
#endif /* ARPOE */
	struct net_device_stats stats;  /**< stat counter reporting structure */
	uint    stats_id;               /**< the current set of stats */
	struct net_device_stats stats_watchdog[2]; /**< ping-pong stats counters updated */
	                                           /* by Linux watchdog */
#ifdef USE_IW
	struct iw_statistics wstats_watchdog[2];
	struct iw_statistics wstats;
	int             phy_noise;
#endif /* USE_IW */

#if defined(PKTC_TBL)
    union {
	    struct d3fwd_wlif   *d3fwd_wlif;
	    struct pktc_info	*pktci;
    };
#endif // endif
	int wds_index; /* wds interface indexing */
};

struct rfkill_stuff {
	struct rfkill *rfkill;
	char rfkill_name[32];
	char registered;
};

/* convenience struct for work queue scheduling */
struct swq_struct {
	struct list_head	work_list;
	spinlock_t		lock;
	wait_queue_head_t       thread_waitq;
	struct task_struct      *thread;
};

/* handle forward declaration */
typedef struct wl_cmn_info wl_cmn_info_t;
typedef struct wl_pktc_tbl wl_pktc_tbl_t;

struct wl_info {
	uint		unit;		/**< device instance number */
	wlc_pub_t	*pub;		/**< pointer to public wlc state */
	wlc_info_t	*wlc;		/**< pointer to private common os-independent data */
	osl_t		*osh;		/**< pointer to os handler */
	wl_cmn_info_t	*cmn;		/* pointer to common part of two wl structure variable */
#if defined(BCM_GMAC3)
	struct fwder *fwdh;		/**< pointer to forwarder handle */
	int			fwder_unit; /**< assigned fwder unit (modulo-FWDER_MAX_UNIT) */
#endif /* BCM_GMAC3 */
#ifdef HNDCTF
	ctf_t		*cih;		/**< ctf instance handle */
#endif /* HNDCTF */
	struct net_device *dev;		/**< backpoint to device */

	struct semaphore sem;		/**< use semaphore to allow sleep */
	spinlock_t	lock;		/**< per-device perimeter lock */
	spinlock_t	isr_lock;	/**< per-device ISR synchronization lock */

	uint		bcm_bustype;	/**< bus type */
	bool		piomode;	/**< set from insmod argument */
	void *regsva;			/**< host virtual address to the CC regs (via bar0) */
	wl_if_t *if_list;		/**< list of all interfaces */
	atomic_t callbacks;		/**< # outstanding callback functions */
	struct wl_timer *timers;	/**< timer cleanup queue */
#ifndef NAPI_POLL
	struct tasklet_struct tasklet;	/**< dpc tasklet */
	struct tasklet_struct tx_tasklet; /**< tx tasklet */
#endif /* NAPI_POLL */

#if defined(NAPI_POLL) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 30))
	struct napi_struct napi;
#endif /* defined(NAPI_POLL) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 30)) */

	struct net_device *monitor_dev;	/**< monitor pseudo device */
	uint		monitor_type;	/**< monitor pseudo device */
	bool		resched;	/**< dpc needs to be and is rescheduled */
	uint		max_cpu_id;	/**< upper ID of CPU, i.e. nr of CPUs - 1 */
#ifdef TOE
	wl_toe_info_t	*toei;		/**< pointer to toe specific information */
#endif // endif
#ifdef ARPOE
	wl_arp_info_t	*arpi;		/**< pointer to arp agent offload info */
#endif // endif
#ifdef LINUXSTA_PS
	uint32		pci_psstate[16];	/**< pci ps-state save/restore */
#endif // endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 14)
#define NUM_GROUP_KEYS 4
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 29)
	struct lib80211_crypto_ops *tkipmodops;
#else
	struct ieee80211_crypto_ops *tkipmodops;	/**< external tkip module ops */
#endif // endif
	struct ieee80211_tkip_data  *tkip_ucast_data;
	struct ieee80211_tkip_data  *tkip_bcast_data[NUM_GROUP_KEYS];
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 14) */
	/* handle, lock, txq, workitem */
	bool		txq_dispatched;	/**< Avoid scheduling multiple tasks */
	spinlock_t	txq_lock;	/**< Lock for the queue */
	struct sk_buff	*txq_head;	/**< TX Q */
	struct sk_buff	*txq_tail;	/**< Points to the last buf */
	int		txq_cnt;	/**< the txq length */

#ifdef WL_ALL_PASSIVE
	wl_task_t	txq_task;	/**< work queue for wl_start() */
	wl_task_t	multicast_task;	/**< work queue for wl_set_multicast_list() */

	wl_task_t	wl_dpc_task;	/**< work queue for wl_dpc() */
	bool		all_dispatch_mode;
	struct wl_timer *timers_freelist;	/**< timer list waiting to be free */
#endif /* WL_ALL_PASSIVE */

#ifdef WL_THREAD
    struct task_struct      *thread;
    wait_queue_head_t       thread_wqh;
    struct sk_buff_head     tx_queue;
#endif /* WL_THREAD */
#if defined(WL_CONFIG_RFKILL)
	struct rfkill_stuff wl_rfkill;
	mbool last_phyind;
#endif /* defined(WL_CONFIG_RFKILL) */

	uint processed;		/**< Number of rx frames processed */
	struct proc_dir_entry *proc_entry;
#if defined(WLVASIP)
	uchar* bar1_addr;
	uint32 bar1_size;
	uchar* bar2_addr;
	uint32 bar2_size;
#endif // endif
	wlc_obj_registry_t	*objr;		/**< Common shared object registry for RSDB */
#ifndef LINUX_POSTMOGRIFY_REMOVAL
#ifdef P2PO
	wl_p2po_info_t	*p2po;		/**< pointer to p2p offload info */
	wl_disc_info_t	*disc;		/**< pointer to disc info */
#endif /* P2PO */
#ifdef ANQPO
	wl_anqpo_info_t *anqpo;	/**< pointer to anqp offload info */
#endif /* ANQPO */
#if defined(P2PO) || defined(ANQPO) || (defined(WL_MBO) && defined(MBO_AP))
	wl_gas_info_t	*gas;		/**< pointer to gas info */
#endif /* P2PO || ANQPO || MBO_AP */
#if defined(WL_EVENTQ) || (defined(WL_MBO) && defined(MBO_AP))
	wl_eventq_info_t	*wlevtq;	/**< pointer to wl_eventq info */
#endif /* WL_EVENTQ */
#ifdef BDO
	wl_bdo_info_t *bdo;	/* pointer to bonjour dongle offload info */
#endif /* BDO */
#ifdef ICMP
	wl_icmp_info_t	*icmp;	/* pointer to icmp info */
#endif	/* ICMP */
#endif /* LINUX_POSTMOGRIFY_REMOVAL */
#if defined(PLATFORM_INTEGRATED_WIFI) && defined(CONFIG_OF)
	struct wl_platform_info *plat_info;	/* platform device handler */
#endif /* PLATFORM_INTEGRATED_WIFI && CONFIG_OF */

#ifdef WL_ALL_PASSIVE
#if defined(PKTC_TBL)
	bool	txq_txchain_dispatched; /* dispatched flag for wl thread */
#endif // endif
#if defined(BCM_AWL)
	bool	awl_sp_rxq_dispatched;  /* dispatch flag for slow path rx packets */
#endif /* BCM_AWL */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 4, 0))
	int processor_id; /* TP id for WLAN TX/RX */
	struct task_struct	*kthread;
	wait_queue_head_t	kthread_wqh;
	bool	rxq_dispatched;
#endif /* LINUX_VERSION_CODE */
#endif /* WL_ALL_PASSIVE */

#ifdef BCM_WFD
	int wfd_idx;
#endif // endif
	uint16 dev_id; /**< e.g. PCIe device ID */
	struct wl_core_priv *wlcore;

#ifdef AVS
	wl_avs_t		*avs;
#endif /* AVS */
	uint16 dump_signature;	/* dump filename signature */
	uint8 dtrace_cur_fidx;	/* dtrace current file index */
	uint32 dtrace_cur_fsz;	/* dtrace durrent file byte size */
#if defined(PKTC_TBL)
	wl_pktc_tbl_t *pktc_tbl;
#endif /* PKTC_TBL */
#ifdef WL_CFG80211
	struct wiphy *wiphy;
#endif // endif
#if defined(CONFIG_BCM_WLAN_DPDCTL)
	char pciname[32];
#endif /* CONFIG_BCM_WLAN_DPDCTL */
	int ifidx_bitmap;
};

#if (defined(NAPI_POLL) && defined(WL_ALL_PASSIVE))
#error "WL_ALL_PASSIVE cannot co-exists w/ NAPI_POLL"
#endif /* defined(NAPI_POLL) && defined(WL_ALL_PASSIVE) */

#if defined(BCM_GMAC3)
#define WLIF_FWDER(wlif)        ((wlif)->fwdh != FWDER_NULL)
#else
#define WLIF_FWDER(wlif)        (FALSE)
#endif /* BCM_GMAC3 */

#if defined(BCM_GMAC3)
#define WL_ALL_PASSIVE_ENAB(wl)	0
#elif defined(WL_ALL_PASSIVE_ON)
#define WL_ALL_PASSIVE_ENAB(wl)	1
#else
#ifdef WL_ALL_PASSIVE
#define WL_ALL_PASSIVE_ENAB(wl)	(!(wl)->all_dispatch_mode)
#else
#define WL_ALL_PASSIVE_ENAB(wl)	0
#endif /* WL_ALL_PASSIVE */
#endif /* !BCM_GMAC3 && !(WL_ALL_PASSIVE_ON ) */

/* perimeter lock */
#define WL_LOCK(wl) \
	do { \
		if (WL_ALL_PASSIVE_ENAB(wl)) \
			down(&(wl)->sem); \
		else \
			spin_lock_bh(&(wl)->lock); \
	} while (0)
#define WL_UNLOCK(wl) \
	do { \
		if (WL_ALL_PASSIVE_ENAB(wl)) \
			up(&(wl)->sem); \
		else \
			spin_unlock_bh(&(wl)->lock); \
	} while (0)

#if defined(BCM_GMAC3)
#define WL_LOCK_TRY(wl, lock_taken)	do { \
						if (lock_taken == WL_LOCK_NOTTAKEN) { \
							WL_LOCK(wl); \
						} \
					} while (0)

#define WL_UNLOCK_TRY(wl, lock_taken)	do { \
						if (lock_taken == WL_LOCK_NOTTAKEN) { \
							WL_UNLOCK(wl); \
						} \
					} while (0)
#else
#define WL_LOCK_TRY(wl, lock_taken)	WL_LOCK(wl)
#define WL_UNLOCK_TRY(wl, lock_taken)	WL_UNLOCK(wl)
#endif /* BCM_GMAC3 */

/** locking from inside wl_isr */
#define WL_ISRLOCK(wl, flags) do {spin_lock(&(wl)->isr_lock); (void)(flags);} while (0)
#define WL_ISRUNLOCK(wl, flags) do {spin_unlock(&(wl)->isr_lock); (void)(flags);} while (0)

/** locking under WL_LOCK() to synchronize with wl_isr */
#define INT_LOCK(wl, flags)	spin_lock_irqsave(&(wl)->isr_lock, flags)
#define INT_UNLOCK(wl, flags)	spin_unlock_irqrestore(&(wl)->isr_lock, flags)

/* handle forward declaration */
typedef struct wl_info wl_info_t;

struct wl_cmn_info {
	wl_info_t *wl[MAX_RSDB_MAC_NUM];
};

#ifndef PCI_D0
#define PCI_D0		0
#endif // endif

#ifndef PCI_D3hot
#define PCI_D3hot	3
#endif // endif

/* exported functions */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 20)
extern irqreturn_t wl_isr(int irq, void *dev_id);
#else
extern irqreturn_t wl_isr(int irq, void *dev_id, struct pt_regs *ptregs);
#endif // endif

extern int wl_start_int(wl_info_t *wl, wl_if_t *wlif, struct sk_buff *skb);
extern int wl_pktc_tx(wl_info_t *wl, wl_if_t *wlif, void *pkt);
#ifdef WL_ALL_PASSIVE
void wl_dpc_rxwork(struct wl_task *task);
#endif /* WL_ALL_PASSIVE */

extern int __devinit wl_pci_probe(struct pci_dev *pdev, const struct pci_device_id *ent);
extern void wl_free(wl_info_t *wl);
extern int  wl_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd);
extern int  wl_ioctl_entry(struct net_device *dev, struct ifreq *ifr, int cmd);
extern struct net_device * wl_netdev_get(wl_info_t *wl);
extern wlc_bsscfg_t * wl_bsscfg_find(wl_if_t *wlif);
extern struct net_device * wl_idx2net(wl_info_t *wl, uint subunit);

#ifdef BCM_WL_EMULATOR
extern wl_info_t *  wl_wlcreate(osl_t *osh, void *pdev);
#endif // endif

s32 get_ifidx_from_bsscfg(wlc_bsscfg_t *cfg);

extern uint32 wl_backplane_read(void *context, uint32 address);
extern void wl_backplane_write(void *context, uint32 address, uint32 value);
extern uint32 wl_pmu_access(void *context, uint32 address, uint32 mask, uint32 value);

#if defined(PLATFORM_INTEGRATED_WIFI) && defined(CONFIG_OF)
struct wl_platform_info {
	struct platform_device *pdev;
	void __iomem *regs;	/* Base ioremap address for platform device */
	int irq;
	uint16 deviceid;
	void	*plat_priv;
};
#endif /* PLATFORM_INTEGRATED_WIFI && CONFIG_OF */

#if defined(WLCFP)
extern void wl_cfp_sendup(wl_info_t *wl, wl_if_t *wlif, void *pkt, uint16 flowid);
#endif /* WLCFP */

extern bool wl_intrabss_forward(wl_info_t *wl, struct net_device *net_device, void *pkt);
extern void wl_sendup_ex(wl_info_t *wl, void *pkt);

#if defined(WDS)
int wlc_get_wlif_wdsindex(struct wl_if *wlif);
#endif // endif

#if !defined(FLAG_DWDS_AP)
#define netdev_wlan_set_dwds_ap(wlif)    ({ BCM_REFERENCE(wlif); })
#define netdev_wlan_unset_dwds_ap(wlif)  ({ BCM_REFERENCE(wlif); })
#define is_netdev_wlan_dwds_ap(wlif)     (0)
#endif // endif

#if !defined(FLAG_DWDS_CLIENT)
#define netdev_wlan_set_dwds_client(wlif)    ({ BCM_REFERENCE(wlif); })
#define netdev_wlan_unset_dwds_client(wlif)  ({ BCM_REFERENCE(wlif); })
#define is_netdev_wlan_dwds_client(wlif)     (0)
#endif // endif

#endif /* _wl_linux_h_ */
