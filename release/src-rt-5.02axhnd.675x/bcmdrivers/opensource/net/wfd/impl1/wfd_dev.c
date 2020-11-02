/*
   <:copyright-BRCM:2015:DUAL/GPL:standard
   
      Copyright (c) 2015 Broadcom 
      All Rights Reserved
   
   Unless you and Broadcom execute a separate written software license
   agreement governing use of this software, this software is licensed
   to you under the terms of the GNU General Public License version 2
   (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
   with the following added to such license:
   
      As a special exception, the copyright holders of this software give
      you permission to link this software with independent modules, and
      to copy and distribute the resulting executable under terms of your
      choice, provided that you also meet, for each linked independent
      module, the terms and conditions of the license of that module.
      An independent module is a module which is not derived from this
      software.  The special exception does not apply to any modifications
      of the software.
   
   Not withstanding the above, under no circumstances may you combine
   this software in any way with any other Broadcom software provided
   under a license other than the GPL, without Broadcom's express prior
   written consent.
   
   :>
 */

/****************************************************************************/
/**                                                                        **/
/** Software unit Wlan accelerator dev                                     **/
/**                                                                        **/
/** Title:                                                                 **/
/**                                                                        **/
/**   Wlan accelerator interface.                                          **/
/**                                                                        **/
/** Abstract:                                                              **/
/**                                                                        **/
/**   Mediation layer between the wifi / PCI interface and the Accelerator **/
/**  (Runner/FAP)                                                          **/
/**                                                                        **/
/** Allocated requirements:                                                **/
/**                                                                        **/
/** Allocated resources:                                                   **/
/**                                                                        **/
/**   A thread.                                                            **/
/**   An interrupt.                                                        **/
/**                                                                        **/
/**                                                                        **/
/****************************************************************************/


/****************************************************************************/
/******************** Operating system include files ************************/
/****************************************************************************/
#include <linux/types.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <net/route.h>
#include <linux/moduleparam.h>
#include <linux/netdevice.h>
#include <linux/if_ether.h>
#include <linux/etherdevice.h>
#include <linux/kthread.h>
#include "bcm_pktfwd.h" /* BCM_PKTFWD && BCM_PKTLIST && BCM_PKTQUEUE */
#include "wfd_dev.h"
#include "wfd_dev_priv.h"
#include "bpm.h"
#ifdef CONFIG_BCM_WFD_RATE_LIMITER
#include "rate_limiter.h"
#endif

#if defined(BCM_PKTFWD) && defined(BCM_PKTQUEUE)
#include "wl_pktc.h" /* for PKTC_CHAIN_IDX_MASK */
#endif

#include <linux/bcm_skb_defines.h>

/****************************************************************************/
/***************************** Module Version *******************************/
/****************************************************************************/
static const char *version = "Wifi Forwarding Driver";

#define WFD_QUEUE_TO_WFD_IDX_MASK 0x1
#define WIFI_IF_NAME_STR_LEN  ( IFNAMSIZ )
#define WLAN_CHAINID_OFFSET 8

#if defined(CONFIG_BCM94908)
#define WFD_INTERRUPT_COALESCING_TIMEOUT_US 1000
#define WFD_INTERRUPT_COALESCING_MAX_PKT_CNT 63
#else
#define WFD_INTERRUPT_COALESCING_TIMEOUT_US 500
#define WFD_INTERRUPT_COALESCING_MAX_PKT_CNT 32
#endif
#define WFD_BRIDGED_OBJECT_IDX 2
#define WFD_BRIDGED_QUEUE_IDX 2

#if defined(BCM_PKTFWD)
#define WFD_FLCTL /* BPM PktPool Availability based WFD Ingress throttle */
#define WFD_FLCTL_PKT_PRIO_FAVOR         4  /* Favor Pkt Prio >= 4 : VI,VO */
#define WFD_FLCTL_DROP_CREDITS           32
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
#define WFD_FLCTL_SKB_EXHAUSTION_LO_PCNT 25 /* Favored Pkt threshold */
#define WFD_FLCTL_SKB_EXHAUSTION_HI_PCNT 10
#endif /* CONFIG_BCM_BPM || CONFIG_BCM_BPM_MODULE */
#endif

/****************************************************************************/
/*********************** Multiple SSID FUNCTIONALITY ************************/
/****************************************************************************/
static struct net_device __read_mostly *wifi_net_devices[WIFI_MW_MAX_NUM_IF]={NULL, } ;

static struct proc_dir_entry *proc_wfd_dir;          /* /proc/wfd */
static struct proc_dir_entry *proc_wfd_stats_file;   /* /proc/wfd/stats */
#if defined(WFD_FLCTL)
static struct proc_dir_entry *proc_wfd_flctl_file;   /* /proc/wfd/flctl */
#endif

extern void replace_upper_layer_packet_destination( void * cb, void * napi_cb );
extern void unreplace_upper_layer_packet_destination( void );

#if (defined(CONFIG_BCM_MCAST) || defined(CONFIG_BCM_MCAST_MODULE))
extern void   bcm_mcast_handle_dev_down(void *net);
#endif
static int wfd_tasklet_handler(void  *context);

/****************************************************************************/
/***************************** Module parameters*****************************/
/****************************************************************************/
/* Number of packets to read in each tasklet iteration */
#define NUM_PACKETS_TO_READ_MAX 128
static int num_packets_to_read = NUM_PACKETS_TO_READ_MAX;
module_param (num_packets_to_read, int, 0);
/* wifi Broadcom prefix */
static char wifi_prefix [WIFI_IF_NAME_STR_LEN] = "wl";
module_param_string (wifi_prefix, wifi_prefix, WIFI_IF_NAME_STR_LEN, 0);

#if (defined(CONFIG_BCM_RDPA)||defined(CONFIG_BCM_RDPA_MODULE)) 
#if defined(CONFIG_BCM96838) || defined(CONFIG_BCM96848)

#define WFD_NUM_QUEUES_PER_WFD_INST 1
#else
/* 2 queues(Priorities low & high) per WFD Instance. Even-Low, Odd-High
   WFD 0 - Queue 8 low, 9 high
   WFD 1 - Queue 10 low, 11 high 
   WFD 2 - Queue 12 low, 13 high */
#define WFD_NUM_QUEUES_PER_WFD_INST 2
#endif
#define WFD_NUM_QUEUE_SUPPORTED (WFD_MAX_OBJECTS * WFD_NUM_QUEUES_PER_WFD_INST)
#elif (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
#include "fap_dqm.h"
#elif defined(CONFIG_BCM_ARCHER) || defined(CONFIG_BCM_ARCHER_MODULE)
#define WFD_NUM_QUEUES_PER_WFD_INST 2
#define WFD_NUM_QUEUE_SUPPORTED (WFD_MAX_OBJECTS * WFD_NUM_QUEUES_PER_WFD_INST)
#else //Fcache based WFD
#define WFD_NUM_QUEUES_PER_WFD_INST 2
#define WFD_NUM_QUEUE_SUPPORTED (WFD_MAX_OBJECTS * WFD_NUM_QUEUES_PER_WFD_INST)
#endif

/* Counters */
static unsigned int gs_count_rx_bridged_packets [WIFI_MW_MAX_NUM_IF] = {0, } ;
static unsigned int gs_count_tx_packets [WIFI_MW_MAX_NUM_IF] = {0, } ;
static unsigned int gs_count_no_buffers [WFD_NUM_QUEUE_SUPPORTED] = { } ;
static unsigned int gs_count_no_skbs [WFD_NUM_QUEUE_SUPPORTED] = { } ;
static unsigned int gs_count_rx_pkt [WFD_NUM_QUEUE_SUPPORTED] = { } ;
static unsigned int gs_max_rx_pkt [WFD_NUM_QUEUE_SUPPORTED] = { } ;
static unsigned int gs_count_rx_invalid_ssid_vector[WFD_NUM_QUEUE_SUPPORTED] = {};
static unsigned int gs_count_rx_no_wifi_interface[WFD_NUM_QUEUE_SUPPORTED] = {} ;
static unsigned int gs_count_rx_error[WFD_NUM_QUEUE_SUPPORTED] = {} ;
#if defined(WFD_FLCTL)
static unsigned int gs_count_flctl_pkts[WFD_NUM_QUEUE_SUPPORTED] = {} ;
#endif

/* Forward declaration */
struct wfd_object;
typedef struct wfd_object wfd_object_t;

/* Note: keep budget <= NUM_PACKETS_TO_READ_MAX */
typedef uint32_t (*wfd_bulk_get_t)(unsigned long qid, unsigned long budget, void *wfd_p);

#if defined(BCM_PKTFWD) && defined(BCM_PKTQUEUE) && !defined(CONFIG_BCM_PON)
#define WFD_SWQUEUE
#endif

#if defined(WFD_SWQUEUE)

/**
 * =============================================================================
 * Section: WFD_SWQUEUE
 * =============================================================================
*/

/** WFD SW queue drop threshold: same as WFD HW Rx queue size */
#define WFD_SWQUEUE_MIN_SIZE        (64)
#define WFD_SWQUEUE_MAX_SIZE        WFD_WLAN_QUEUE_MAX_SIZE

/** System global lock macros mutual exclusive access */
#if defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)
#define WFD_SWQUEUE_LOCK(swq)       spin_lock_bh(&((swq)->lock))
#define WFD_SWQUEUE_UNLK(swq)       spin_unlock_bh(&((swq)->lock))
#else
#define WFD_SWQUEUE_LOCK(swq)       local_irq_disable()
#define WFD_SWQUEUE_UNLK(swq)       local_irq_enable()
#endif  /* ! (CONFIG_SMP || CONFIG_PREEMPT) */


/** WFD_SWQUEUE subsystem construction & destruction */
static  int     wfd_swqueue_init(wfd_object_t * wfd_p, uint32_t swq_size);
static  void    wfd_swqueue_fini(uint32_t wfd_idx);

/** Callbacks registered with bcm_pktfwd */
static  bool    wfd_swqueue_flush_pkts(void * driver, pktqueue_t * pktqueue);
static  void    wfd_swqueue_flush_complete(void * driver);

/* WFD_SW_QUEUE functions to transmit packets from SW queue */
static  uint32_t  wfd_swqueue_skb_xmit(wfd_object_t * wfd_p, uint32_t budget);
static  uint32_t  wfd_swqueue_fkb_xmit(wfd_object_t * wfd_p, uint32_t budget);

/* Note: keep budget <= NUM_PACKETS_TO_READ_MAX */
typedef uint32_t  (* wfd_swqueue_xmit_fn_t)(wfd_object_t * wfd_p,
                                            uint32_t budget);

/**
 * -----------------------------------------------------------------------------
 *  wfd_swqueue
 *
 *  State
 *  - Queue holding packets from ingress network devices
 *  - Queue Size
 *  - Handler to xmit packets from SWq
 *  - pktqueue_context registered with bcm_pktfwd
 *  - SWQ thread state
 *
 * -----------------------------------------------------------------------------
 */

struct wfd_swqueue                  /* WFD SW queue state */
{
    spinlock_t          lock;               /* spinlock: Not used in xmit_fn */

    unsigned int        domain;             /* pktqueue_context domain */
    pktqueue_t          pktqueue;           /* WFD SW Queue */
    uint32_t            swq_size;           /* WFD SW Queue size */

    wfd_swqueue_xmit_fn_t  swq_xmit_fn;     /* handler to xmit pkts from SWq */
    struct pktqueue_context * pktqueue_context_p;

    uint8_t             swq_schedule;       /* WFD SWq schedule state */
    uint32_t            schedule_cnt;       /* SWq thread: SWq xmit requests */
    uint32_t            complete_cnt;       /* SWq thread: scheduled cnt */
    uint32_t            dispatches;         /* total xmit handler invocations */
    uint32_t            pkts_count;         /* pkts counts - xmited from SWq */
    uint32_t            pkts_dropped;       /* dropped packets */
};

typedef struct wfd_swqueue wfd_swqueue_t;

#define WFD_SWQUEUE_NULL        ((wfd_swqueue_t *) NULL)

#endif /* WFD_SWQUEUE */

struct wfd_object
{
    unsigned long       wfd_rx_work_avail;
    unsigned int        wfd_idx;
    int                 wfd_queue_mask;
    int                 wl_radio_idx;
    unsigned int        count_rx_queue_packets;
    unsigned int        wl_chained_packets;
    unsigned int        wl_mcast_packets;
#if defined(WFD_FLCTL)  /* BPM Skb availability based Rx flowcontrol */
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    unsigned int        skb_exhaustion_lo; /* avail < lo: admit ONLY favored */
    unsigned int        skb_exhaustion_hi; /* avail < hi: admit none */
#endif
    unsigned short      pkt_prio_favor;    /* favor pkt prio >= 4 */
#endif /* WFD_FLCTL */
    struct pktlist_context *pktlist_context_p; /* BCM_PKTFWD && BCM_PKTLIST */
    struct net_device  *wl_dev_p;
    wfd_bulk_get_t      wfd_bulk_get;
    HOOK32              wfd_completeHook; /* pktlist_context_xfer_fn_t */
    HOOK4PARM           wfd_fwdHook;
    HOOK3PARM           wfd_mcastHook;
    wait_queue_head_t   wfd_rx_thread_wqh;
    struct task_struct *wfd_rx_thread;
    void               *wfd_acc_info_p;
    struct net_device  *wl_if_dev[WIFI_MW_MAX_NUM_IF];
    int                 isValid;
    enumWFD_WlFwdHookType eFwdHookType; 
    bool isTxChainingReqd;
#if defined(XRDP) && !defined(BCM_PKTFWD)
    /* defined here due to stack size limitation, 
     * used inside wfd_bulk_skb_get, wfd_bulk_fkb_get */
    void                *mcast_packets[NUM_PACKETS_TO_READ_MAX];
#endif

#if defined(WFD_SWQUEUE)
    wfd_swqueue_t       * wfd_swqueue;  /* WFD SW queue */
#endif /* WFD_SWQUEUE */

} ____cacheline_aligned;


static wfd_object_t wfd_objects[WFD_MAX_OBJECTS];
static int wfd_objects_num=0;
static spinlock_t wfd_irq_lock[WFD_MAX_OBJECTS];
/* first Cpu ring queue - Currently pci CPU ring queues must be sequential */
static const int first_pci_queue = 8;

#define WFD_IRQ_LOCK(wfdLockIdx, flags) spin_lock_irqsave(&wfd_irq_lock[wfdLockIdx], flags)
#define WFD_IRQ_UNLOCK(wfdLockIdx, flags) spin_unlock_irqrestore(&wfd_irq_lock[wfdLockIdx], flags)

#define WFD_WAKEUP_RXWORKER(wfdIdx) do { \
            wake_up_interruptible(&wfd_objects[wfdIdx].wfd_rx_thread_wqh); \
          } while (0)

int (*send_packet_to_upper_layer)(struct sk_buff *skb) = netif_rx;
EXPORT_SYMBOL(send_packet_to_upper_layer); 
int (*send_packet_to_upper_layer_napi)(struct sk_buff *skb) = netif_receive_skb;
EXPORT_SYMBOL(send_packet_to_upper_layer_napi);
int inject_to_fastpath = 0;
EXPORT_SYMBOL(inject_to_fastpath);

static void wfd_dump(void);

/****************************************************************************/
/******************* Other software units include files *********************/
/****************************************************************************/
#if (defined(CONFIG_BCM_RDPA)||defined(CONFIG_BCM_RDPA_MODULE)) 
#if defined(XRDP)
#include "xrdp_wfd_inline.h"
#else
#include "runner_wfd_inline.h"
#endif
void (*wfd_dump_fn)(void) = 0;
#elif (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
#include "fap_wfd_inline.h"
extern void (*wfd_dump_fn)(void);
#elif defined(CONFIG_BCM_ARCHER) || defined(CONFIG_BCM_ARCHER_MODULE)
#include "archer_wfd_inline.h"
void (*wfd_dump_fn)(void) = NULL;
#else //Flow Cache only ...?!
#include "fcache_wfd_inline.h"
void (*wfd_dump_fn)(void) = 0; // not support dump yet ..
#endif

#ifndef WFD_PLATFORM_PROC
#define wfd_plat_proc_init() (0)
#define wfd_plat_proc_uninit(p) 
#endif

/****************************************************************************/
/**                                                                        **/
/** Name:                                                                  **/
/**                                                                        **/
/**   wfd_tasklet_handler.                                                 **/
/**                                                                        **/
/** Title:                                                                 **/
/**                                                                        **/
/**   wlan accelerator - tasklet handler                                   **/
/**                                                                        **/
/** Abstract:                                                              **/
/**                                                                        **/
/**   Reads all the packets from the Rx queue and send it to the wifi      **/
/**   interface.                                                           **/
/**                                                                        **/
/** Input:                                                                 **/
/**                                                                        **/
/** Output:                                                                **/
/**                                                                        **/
/**                                                                        **/
/****************************************************************************/
static int wfd_tasklet_handler(void *context)
{
    int wfdIdx = (int)(long)context;
    int rx_pktcnt = 0;
    int qid, qidx = 0;
    wfd_object_t * wfd_p = &wfd_objects[wfdIdx];
    uint32_t qMask = 0;

#if defined(WFD_SWQUEUE)
    wfd_swqueue_t * wfd_swqueue = wfd_p->wfd_swqueue;
#endif /* WFD_SWQUEUE */

    printk("Instantiating WFD %d thread\n", wfdIdx);
    while (1)
    {
        wait_event_interruptible(wfd_p->wfd_rx_thread_wqh,
                                 wfd_p->wfd_rx_work_avail ||
#if defined(WFD_SWQUEUE)
                                 wfd_swqueue->swq_schedule ||
#endif /* WFD_SWQUEUE */
                                 kthread_should_stop());

        if (kthread_should_stop())
        {
            printk(KERN_INFO "kthread_should_stop detected in wfd\n");
            break;
        }

        if (wfd_p->wfd_rx_work_avail)
        {
            qMask = wfd_p->wfd_rx_work_avail;
            /* Read from High priority queues first if it's bit is on
               Odd bits correspond to high priority queues
               Even bits correspond to low priority queues
               Hence get the last bit set */
            qidx = __fls(qMask);
            while (qMask)
            {
                qid = wfd_get_qid(qidx);

                /* Note: keep num_packets_to_read <= NUM_PACKETS_TO_READ_MAX */
                rx_pktcnt = wfd_p->wfd_bulk_get(qid, num_packets_to_read, wfd_p);

                gs_count_rx_pkt[qidx] += rx_pktcnt;
                gs_max_rx_pkt[qidx] = gs_max_rx_pkt[qidx] > rx_pktcnt ?
                                                gs_max_rx_pkt[qidx] : rx_pktcnt;

                /* NOTE: wfd_queue_not_empty() function clears the hw dqm
                 * interrupt for FAP platforms and must be always executed on
                 * every iteration
                 */
                if (wfd_queue_not_empty(wfd_p->wl_radio_idx, qid, qidx))
                {
                    if (rx_pktcnt >= num_packets_to_read)
                    {
                        schedule();
                    }
                    /* else do nothing. Queue is not empty. Do not clear
                     * work avail flag. Let the thread complete processing the
                     * rest of the packets.
                     */
                }
                else
                {
                    /* Queue is empty: no more packets,
                     * clear bit atomically and enable interrupts
                     */
                    clear_bit(qidx, &wfd_p->wfd_rx_work_avail);
                    wfd_int_enable(wfd_p->wl_radio_idx, qid, qidx);
                }

                qMask &= ~(1 << qidx);
                qidx--;
            } /*for pci queue*/
        } /* wfd_p->wfd_rx_work_avail */

#if defined(WFD_SWQUEUE)
        /* Budget has doubled due to SWq */
        if (wfd_swqueue->swq_schedule)
        {
            wfd_swqueue_xmit_fn_t  swq_xmit;
            swq_xmit = wfd_swqueue->swq_xmit_fn;

            /* Transmit packets from SW queue */
            wfd_swqueue->pkts_count += swq_xmit(wfd_p, num_packets_to_read);

            wfd_swqueue->dispatches++;
            wfd_swqueue->complete_cnt++;
            if (wfd_swqueue->pktqueue.len == 0U)
            {
                /* SW Queue is empty, clear swq_schedule state */
                wfd_swqueue->swq_schedule = 0U;
            }
        }
#endif /* WFD_SWQUEUE */

#if 0
        WFD_IRQ_LOCK(flags);
        if (wfd_rx_work_avail == 0)
        {
            for (qidx = 0; qidx < number_of_queues; qidx++)
            {
               qid = wfd_get_qid(qidx);

               //Enable interrupts
               wfd_int_enable(qid);
            }
        }
#endif
    }

    return 0;
}

struct net_device *wfd_dev_by_id_get(uint32_t radio_id, uint32_t if_id)
{
    return wfd_objects[radio_id].wl_if_dev[if_id];
}
EXPORT_SYMBOL(wfd_dev_by_id_get);

#ifdef CONFIG_BCM_WFD_RATE_LIMITER
int wfd_is_radio_valid(uint32_t radio_id)
{
     return wfd_objects[radio_id].isValid;
}

struct net_device *wfd_dev_by_name_get(char *name, uint32_t *radio_idx, uint32_t *if_idx)
{
    uint32_t radio_id;

    for (radio_id = 0; radio_id < WFD_MAX_OBJECTS; radio_id++)
    {
        int if_id;

        if (!wfd_objects[radio_id].isValid)
            continue;

         for (if_id = 0; if_id < WIFI_MW_MAX_NUM_IF; if_id++)
         {
             if (wfd_objects[radio_id].wl_if_dev[if_id] == NULL)
                 continue;

             if (!strcmp(wfd_objects[radio_id].wl_if_dev[if_id]->name, name))
             {
                 if (radio_idx)
                     *radio_idx = radio_id;
                 if (if_idx)
                     *if_idx = if_id;
                 return wfd_objects[radio_id].wl_if_dev[if_id];
             }
         }
    }

    return NULL;
}
#endif

/****************************************************************************/
/**                                                                        **/
/** Name:                                                                  **/
/**                                                                        **/
/**   wfd_stats_file_read_proc                                             **/
/**                                                                        **/
/** Title:                                                                 **/
/**                                                                        **/
/**   wfd stats - proc file read handler                                   **/
/**                                                                        **/
/** Abstract:                                                              **/
/**                                                                        **/
/**   Procfs callback method.                                              **/
/**      Called when someone reads proc command                            **/
/**   using: cat /proc/wfd/stats                                           **/
/**                                                                        **/
/** Input:                                                                 **/
/**                                                                        **/
/**   page  -  Buffer where we should write                                **/
/**   start -  Never used in the kernel.                                   **/
/**   off   -  Where we should start to write                              **/
/**   count -  How many character we could write.                          **/
/**   eof   -  Used to signal the end of file.                             **/
/**   data  -  Only used if we have defined our own buffer                 **/
/**                                                                        **/
/** Output:                                                                **/
/**                                                                        **/
/**                                                                        **/
/****************************************************************************/
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0)
static int wfd_stats_file_read_proc(char* page, char** start, off_t off, int count, int* eof, void* data)
#else
static ssize_t wfd_stats_file_read_proc(struct file *filep, char __user *page, size_t count, loff_t *data)
#endif
{
    int wifi_index ;
    unsigned int count_rx_queue_packets_total=0 ;
    unsigned int count_tx_bridge_packets_total=0 ;
    int len = 0 ;

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0)
    page+=off;
    page[0]=0;
#else
	if( *data != 0)
		return 0; //indicate EOF
#endif

    for(wifi_index=0;wifi_index < WIFI_MW_MAX_NUM_IF;wifi_index++)
    {
        if( wifi_net_devices[wifi_index] != NULL )
        {
            len += sprintf((page+len),"WFD Registered Interface %d:%s\n",
                            wifi_index,wifi_net_devices[wifi_index]->name);
        }
    }

    /*RX-MW from WiFi queues*/
    for (wifi_index=0; wifi_index<WIFI_MW_MAX_NUM_IF; wifi_index++)
    {
        int head = 0;
        if (gs_count_rx_bridged_packets[wifi_index]!=0)
        {
            count_rx_queue_packets_total += gs_count_rx_bridged_packets[wifi_index];
            if (head == 0)
            {
                len += sprintf((page+len), "RX Bridged traffic from WiFi queues:\n");
                head = 1;
            }

            len += sprintf((page +len), "                            [WiFi %d] briged = %d\n", 
                wifi_index, gs_count_rx_bridged_packets[wifi_index]) ;
        }
    }

    /*TX-MW to bridge*/
    for (wifi_index=0; wifi_index<WIFI_MW_MAX_NUM_IF; wifi_index++)
    {
        int head = 0;
        if ( gs_count_tx_packets[wifi_index]!=0)
        {
            count_tx_bridge_packets_total += gs_count_tx_packets[wifi_index] ;
            if (head == 0)
            {
                len += sprintf((page+len), "TX to bridge:\n");
                head = 1;
            }
            len += sprintf((page+len ), "                            [WiFi %d] = %d\n",      
                wifi_index, gs_count_tx_packets[wifi_index]) ;
        }
    }

    for (wifi_index = 0 ; wifi_index < WFD_MAX_OBJECTS ;wifi_index++ )
    {
        if (wfd_objects[wifi_index].wfd_bulk_get)
        {
            len += sprintf((page+len),"\nWFD Object %d",wifi_index);
            len += sprintf((page+len), "\nwl_chained_counters       =%d", wfd_objects[wifi_index].wl_chained_packets) ;
            len += sprintf((page+len), "\nwl_mcast_counters         =%d", wfd_objects[wifi_index].wl_mcast_packets) ;
            len += sprintf((page+len), "\ncount_rx_queue_packets    =%d", wfd_objects[wifi_index].count_rx_queue_packets) ;

           {/* per queue status */          
                unsigned int tmp_mask = 0x00;
                unsigned int qidx = 0x00;

                tmp_mask = wfd_objects[wifi_index].wfd_queue_mask;
                
                for(qidx =0x00 ; tmp_mask!=0x00 ; qidx++)
                {
                    if(tmp_mask & (1<<qidx))
                    {
                        len += sprintf((page+len), "\nQueue[%d]:",qidx);
                        len += sprintf((page+len), "\ncount_rx_pkt              =%d", gs_count_rx_pkt[qidx]) ;
                        len += sprintf((page+len), "\nmax_rx_pkt                =%d", gs_max_rx_pkt[qidx]) ;
                        count_rx_queue_packets_total += gs_count_rx_pkt[qidx];
#if defined(WFD_FLCTL)
                        len += sprintf((page+len), "\nflctl_pkts                =%d", gs_count_flctl_pkts[qidx]) ;
                        gs_count_flctl_pkts[qidx] = 0;
#endif
                        len += sprintf((page+len), "\nno_bpm_buffers_error      =%d", gs_count_no_buffers[qidx]) ;
                        len += sprintf((page+len), "\nno_skb_error              =%d", gs_count_no_skbs[qidx]) ;
                        len += sprintf((page+len), "\nInvalid SSID vector       =%d",
                            gs_count_rx_invalid_ssid_vector[qidx]);
                        len += sprintf((page+len), "\nNo WIFI interface         =%d", gs_count_rx_no_wifi_interface[qidx]);
                        len += sprintf((page+len), "\nRx Packet Error           =%d", gs_count_rx_error[qidx]);
#if (defined(CONFIG_BCM_RDPA)||defined(CONFIG_BCM_RDPA_MODULE))
                        len += sprintf((page+len), "\nRing Information:\n");
#if !defined(XRDP)
                        len += sprintf((page+len), "\tRing Base address = 0x%pK\n",wfd_rings[qidx].base );
                        len += sprintf((page+len), "\tRing Head address = 0x%pK\n",wfd_rings[qidx].head );
                        len += sprintf((page+len), "\tRing Head position = %ld\n",(long)(wfd_rings[qidx].head - wfd_rings[qidx].base));
#endif
#endif
                    }

                    tmp_mask &= ~(1<<qidx);

                }
            }

            wfd_objects[wifi_index].wl_chained_packets = 0;
            wfd_objects[wifi_index].wl_mcast_packets = 0;
            wfd_objects[wifi_index].count_rx_queue_packets = 0;

#if defined(WFD_SWQUEUE)
            {
                wfd_swqueue_t * wfd_swqueue = wfd_objects[wifi_index].wfd_swqueue;

                /* Dump wfd_swqueue stats */
                len += sprintf((page+len), "\nSW Queue Statistics:");
                len += sprintf((page+len), "\nDomain        = %d", wfd_swqueue->domain);
                len += sprintf((page+len), "\nDispatch      = %d", wfd_swqueue->dispatches);
                len += sprintf((page+len), "\npkts_count    = %d", wfd_swqueue->pkts_count);
                len += sprintf((page+len), "\npkts_dropped  = %d", wfd_swqueue->pkts_dropped);
                len += sprintf((page+len), "\nschedule_cnt  = %d", wfd_swqueue->schedule_cnt);
                len += sprintf((page+len), "\ncomplete_cnt  = %d", wfd_swqueue->complete_cnt);

                wfd_swqueue->dispatches     = 0;
                wfd_swqueue->pkts_count     = 0;
                wfd_swqueue->pkts_dropped   = 0;
                wfd_swqueue->schedule_cnt   = 0;
                wfd_swqueue->complete_cnt   = 0;

                if (wfd_swqueue->pktqueue_context_p != PKTQUEUE_CONTEXT_NULL)
                {
                    pktqueue_context_dump(wfd_swqueue->pktqueue_context_p);
                }
            }
#endif /* WFD_SWQUEUE */

#if defined(BCM_PKTFWD)
            if (wfd_objects[wifi_index].pktlist_context_p != PKTLIST_CONTEXT_NULL)
            {
                pktlist_context_dump(wfd_objects[wifi_index].pktlist_context_p,
                                     true, true);
            }
#endif /* BCM_PKTFWD */

            len += sprintf((page+len), "\n===========\n");

        }
    }

    len += sprintf((page+len), "\nRX from WiFi queues      [SUM] = %d\n", count_rx_queue_packets_total) ;
    len += sprintf((page+len),   "TX to bridge             [SUM] = %d\n", count_tx_bridge_packets_total) ;

    memset(gs_count_rx_bridged_packets, 0, sizeof(gs_count_rx_bridged_packets));
    memset(gs_count_tx_packets, 0, sizeof(gs_count_tx_packets));
    memset(gs_count_no_buffers, 0, sizeof(gs_count_no_buffers));
    memset(gs_count_no_skbs, 0, sizeof(gs_count_no_skbs));
    memset(gs_count_rx_pkt, 0, sizeof(gs_count_rx_pkt));
    memset(gs_max_rx_pkt, 0, sizeof(gs_max_rx_pkt));
    memset(gs_count_rx_invalid_ssid_vector, 0, sizeof(gs_count_rx_invalid_ssid_vector));
    memset(gs_count_rx_no_wifi_interface, 0, sizeof(gs_count_rx_no_wifi_interface));
    memset(gs_count_rx_error, 0, sizeof(gs_count_rx_error));

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0)
    *eof = 1;
#else
    *data = len; 
#endif
    return len;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0)
static const struct file_operations stats_fops = {
       .owner  = THIS_MODULE,
       .read   = wfd_stats_file_read_proc,
};
#endif


#if defined(WFD_FLCTL)
static ssize_t wfd_flctl_file_read_proc(struct file *file, char *buff, size_t len, loff_t *offset)
{
    uint32_t radio_id;
    char *header, *format;

    if (*offset)
        return 0;

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    *offset += sprintf(buff + *offset,
        "BPM system total<%u> avail<%u>\n", gbpm_total_skb(), gbpm_avail_skb());
    header = "WFD ExhaustionLO ExhaustionHI PktPrioFavor\n";
    format = "%2u. %12u %12u %12u\n";
#else
    header = "WFD PktPrioFavor\n";
    format = "%2u. %12u\n";
#endif
    *offset += sprintf(buff + *offset, header);
    for (radio_id = 0; radio_id < WFD_MAX_OBJECTS; radio_id++)
    {
       *offset += sprintf(buff + *offset, format, radio_id,
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
           wfd_objects[radio_id].skb_exhaustion_lo,
           wfd_objects[radio_id].skb_exhaustion_hi,
#endif
           wfd_objects[radio_id].pkt_prio_favor);
    }

    return *offset;
}

#define WFD_FLCTL_PROC_CMD_MAX_LEN    64
static ssize_t wfd_flctl_file_write_proc(struct file *file, const char *buff, size_t len, loff_t *offset)
{
    int ret;
    char input[WFD_FLCTL_PROC_CMD_MAX_LEN];
    uint32_t radio_id, pkt_prio_favor;
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    uint32_t skb_exhaustion_lo, skb_exhaustion_hi;
#endif

    if (copy_from_user(input, buff, len) != 0)
        return -EFAULT;

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    ret = sscanf(input, "%u %u %u %u", &radio_id, &skb_exhaustion_lo, &skb_exhaustion_hi, &pkt_prio_favor);
    if (ret < 4)
        goto Usage;
#else
    ret = sscanf(input, "%u %u", &radio_id, &pkt_prio_favor);
    if (ret < 2)
        goto Usage;
#endif

    if (radio_id >= WFD_MAX_OBJECTS)
    {
        printk("Invalid radio_id %u, must be less than %u\n", radio_id, WFD_MAX_OBJECTS);
        goto Usage;
    }

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    if (skb_exhaustion_lo < skb_exhaustion_hi)
    {
        printk("Invalid exhaustion level lo<%u> hi<%u>\n", skb_exhaustion_lo, skb_exhaustion_hi);
        goto Usage;
    }
#endif

    if (pkt_prio_favor > 7)
    { /* prio 0 .. 7 */
        printk("Invalid pkt priority <%u>\n", pkt_prio_favor);
        goto Usage;
    }

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    wfd_objects[radio_id].skb_exhaustion_lo = skb_exhaustion_lo;
    wfd_objects[radio_id].skb_exhaustion_hi = skb_exhaustion_hi;
#endif
    wfd_objects[radio_id].pkt_prio_favor    = pkt_prio_favor;

#if defined(BCM_PKTFWD_FLCTL)
    if (wfd_objects[radio_id].pktlist_context_p->fctable != PKTLIST_FCTABLE_NULL)
        wfd_objects[radio_id].pktlist_context_p->fctable->pkt_prio_favor = pkt_prio_favor;
#endif /* BCM_PKTFWD_FLCTL */

    printk("Radio<%u> "
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
        "exhaustion level lo<%u> hi<%u>, "
#endif
        "favor pkt prio >= %u\n",
        radio_id, 
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
        skb_exhaustion_lo, skb_exhaustion_hi,
#endif
        pkt_prio_favor);
    goto Exit;

Usage:
    printk("\nUsage: <radio_id> "
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
        "<skb_exhaustion_lo> <skb_exhaustion_hi> "
#endif
        "<pkt_prio_favor>\n");

Exit:
    return len;
}

static const struct file_operations flctl_fops = {
    .owner  = THIS_MODULE,
    .read   = wfd_flctl_file_read_proc,
    .write  = wfd_flctl_file_write_proc,
};
#endif


/****************************************************************************/
/**                                                                        **/
/** Name:                                                                  **/
/**                                                                        **/
/**   wifi_proc_init.                                                      **/
/**                                                                        **/
/** Title:                                                                 **/
/**                                                                        **/
/**   wifi mw - proc init                                                  **/
/**                                                                        **/
/** Abstract:                                                              **/
/**                                                                        **/
/**   The function initialize the proc entry                               **/
/**                                                                        **/
/** Input:                                                                 **/
/**                                                                        **/
/** Output:                                                                **/
/**                                                                        **/
/**                                                                        **/
/****************************************************************************/
static int wifi_proc_init(void)
{
    if (!(proc_wfd_dir = proc_mkdir("wfd", NULL))) 
        goto fail;

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0)
    /* /proc/wfd/stats file */
    if (!(proc_wfd_stats_file = create_proc_entry("stats", 0444, proc_wfd_dir))) 
        goto fail;

    /* set callback handler for "stats" file */
    proc_wfd_stats_file->read_proc = wfd_stats_file_read_proc;
#else
    if (!(proc_wfd_stats_file = proc_create("wfd/stats", 0644, NULL, &stats_fops))) 
        goto fail;
#endif

#if defined(WFD_FLCTL)
    if (!(proc_wfd_flctl_file = proc_create("wfd/flctl", 0644, NULL, &flctl_fops)))
        goto fail;
#endif
    if (wfd_plat_proc_init()) /* platform specific procs */
        goto fail;
 
    return 0;

fail:
    printk("%s %s: Failed to create proc /wfd\n", __FILE__, __FUNCTION__);
    remove_proc_entry("wfd" ,NULL);
    return (-1);
}



/****************************************************************************/
/**                                                                        **/
/** Name:                                                                  **/
/**                                                                        **/
/**   release_wfd_os_resources                                             **/
/**                                                                        **/
/** Title:                                                                 **/
/**                                                                        **/
/** Abstract:                                                              **/
/**                                                                        **/
/**   The function releases the OS resources                               **/
/**                                                                        **/
/** Input:                                                                 **/
/**                                                                        **/
/** Output:                                                                **/
/**                                                                        **/
/**   bool - Error code:                                                   **/
/**             true - No error                                            **/
/**             false - Error                                              **/
/**                                                                        **/
/****************************************************************************/
static int release_wfd_os_resources(void)
{
#if (defined(CONFIG_BCM_RDPA) || defined(CONFIG_BCM_RDPA_MODULE)) && !defined(XRDP)
    bdmf_destroy(rdpa_cpu_obj);
#endif

#if defined(FC_WFD)
    fc_wfd_accelerator_deinit();
#endif

    return (0) ;
}



/****************************************************************************/
/**                                                                        **/
/** Name:                                                                  **/
/**                                                                        **/
/**   wfd_dev_close                                                        **/
/**                                                                        **/
/** Title:                                                                 **/
/**                                                                        **/
/**   wifi accelerator - close                                             **/
/**                                                                        **/
/** Abstract:                                                              **/
/**                                                                        **/
/**   The function closes all the driver resources.                        **/
/**                                                                        **/
/** Input:                                                                 **/
/**                                                                        **/
/** Output:                                                                **/
/**                                                                        **/
/**                                                                        **/
/****************************************************************************/
static void wfd_dev_close(void)
{
    int wfdIdx;

    /* Disable the interrupt */
    for (wfdIdx = 0; wfdIdx < WFD_MAX_OBJECTS; wfdIdx++)
    {
        wfd_unbind(wfdIdx, wfd_objects[wfdIdx].eFwdHookType);
    }
     
    /* Release the OS driver resources */
    release_wfd_os_resources();

#ifdef CONFIG_BCM_WFD_RATE_LIMITER
    remove_proc_entry("rate_limit", proc_wfd_dir);
#endif
#if defined(WFD_FLCTL)
    remove_proc_entry("flctl", proc_wfd_dir);
#endif
    remove_proc_entry("stats", proc_wfd_dir);
    wfd_plat_proc_uninit(proc_wfd_dir); /* remove platform-specific procs */
    remove_proc_entry("wfd", NULL);

#if defined(CONFIG_BCM_PON) || defined(CONFIG_BCM963158)

    /*Free PCI resources*/
    release_wfd_interfaces();
#endif
}


/****************************************************************************/
/**                                                                        **/
/** Name:                                                                  **/
/**                                                                        **/
/**   wfd_dev_init                                                         **/
/**                                                                        **/
/** Title:                                                                 **/
/**                                                                        **/
/**   wifi accelerator - init                                              **/
/**                                                                        **/
/** Abstract:                                                              **/
/**                                                                        **/
/**   The function initialize all the driver resources.                    **/
/**                                                                        **/
/** Input:                                                                 **/
/**                                                                        **/
/** Output:                                                                **/
/**                                                                        **/
/**                                                                        **/
/****************************************************************************/
static int wfd_dev_init(void)
{
    int idx;
    if (num_packets_to_read > NUM_PACKETS_TO_READ_MAX)
    {
        printk("%s %s Invalid num_packets_to_read %d\n",__FILE__, __FUNCTION__, num_packets_to_read);    
        return -1;
    }

    /* Initialize WFD objects */
    memset((uint8_t *)wfd_objects, 0, sizeof(wfd_objects));

    for (idx = 0; idx < WFD_MAX_OBJECTS; idx++)
    {
       spin_lock_init(&wfd_irq_lock[idx]);
    }

    /* Initialize the proc interface for debugging information */
    if (wifi_proc_init()!=0)
    {
        printk("\n%s %s: wifi_proc_init() failed\n", __FILE__, __FUNCTION__) ;
        goto proc_release;
    }

#ifdef CONFIG_BCM_WFD_RATE_LIMITER
    rate_limiter_init();
#endif

    /* Initialize accelerator(Runner/FAP) specific data structures, Queues */
    if (wfd_accelerator_init() != 0)
    {
        printk("%s %s: wfd_platform_init() failed\n", __FILE__, __FUNCTION__) ;
        goto proc_release;    
    }

    wfd_dump_fn = wfd_dump;
#if defined(CONFIG_BCM96838) ||defined(CONFIG_BCM96848)
    /*Bind instance for bridged traffic */
    if (wfd_bind(NULL, NULL, WFD_WL_FWD_HOOKTYPE_SKB, false, NULL, 0, NULL, -1) == -1)
    {
        printk("%s %s: wfd_bind() failed to bind bridged queue\n", __FILE__, __FUNCTION__) ;
        goto cleanup;
    }

#endif

    printk("\033[1m\033[34m%s is initialized!\033[0m\n", version);
        
    return 0;

#if defined(CONFIG_BCM96838) ||defined(CONFIG_BCM96848)
cleanup:
    wfd_dump_fn = NULL;
#endif

proc_release:
#ifdef CONFIG_BCM_WFD_RATE_LIMITER
    remove_proc_entry("rate_limit", proc_wfd_dir);
#endif
    remove_proc_entry("stats", proc_wfd_dir);
    remove_proc_entry("wfd", NULL);

    return -1;
}

/****************************************************************************/
/**                                                                        **/
/** Name:                                                                  **/
/**                                                                        **/
/**   wfd_bind                                                             **/
/**                                                                        **/
/** Title:                                                                 **/
/**                                                                        **/
/** Abstract:                                                              **/
/**                                                                        **/
/**   Bind the function hooks and other attributes that are needed by      **/
/**   wfd to forward packets to WLAN.                                      **/
/**                                                                        **/
/** Input:                                                                 **/
/**                                                                        **/
/** Output:                                                                **/
/**                                                                        **/
/**                                                                        **/
/****************************************************************************/
int wfd_bind(struct net_device *wl_dev_p, 
             struct pktlist_context *wl_pktlist_context,
             enumWFD_WlFwdHookType eFwdHookType, 
             bool isTxChainingReqd,
             HOOK4PARM wfd_fwdHook, 
             HOOK32 wfd_completeHook,
             HOOK3PARM wfd_mcastHook,
             int wl_radio_idx)
{
    int rc = 0;
    int qid;
    int qidx;
    char threadname[15]={0};
    int minQIdx;
    int maxQIdx;
    int wfd_max_objects;
    unsigned long tmp_idx;
    struct task_struct *rx_thread;

#if defined(CONFIG_BCM_PON_RDP)
    wfd_max_objects = WFD_MAX_OBJECTS - 1;
#else
    wfd_max_objects = WFD_MAX_OBJECTS;
#endif
    if (wfd_objects_num > wfd_max_objects)
    {
        WFD_ERROR("ERROR. WFD_MAX_OBJECTS(%d) limit reached\n", WFD_MAX_OBJECTS);
        rc = WFD_FAILURE;
        goto wfd_bind_failure;
    }

#if defined(CONFIG_BCM_PON_RDP)
    if (!wl_dev_p)
    {
        /*This bind is for bridged traffic use the dummy wfd instance */
        tmp_idx = WFD_BRIDGED_OBJECT_IDX;
        minQIdx = maxQIdx = WFD_BRIDGED_QUEUE_IDX;
    }
    else
#endif
    {

#if defined(CONFIG_BCM_WLAN_DPDCTL)
        /* Use radio_idx as wfd idx as well */
        tmp_idx = wl_radio_idx;
        if (tmp_idx >= wfd_max_objects) {
            WFD_ERROR("%s ERROR. WFD_OBJECT(%d) > MAX(%d)\n", __FUNCTION__, wl_radio_idx, wfd_max_objects);
            rc = WFD_FAILURE;
            goto wfd_bind_failure;
        }

        if (wfd_objects[tmp_idx].wfd_bulk_get) {
            WFD_ERROR("%s ERROR. WFD_OBJECT(%d) in Use\n", __FUNCTION__, wl_radio_idx);
            rc = WFD_FAILURE;
            goto wfd_bind_failure;
        }
#else  /* !CONFIG_BCM_WLAN_DPDCTL */
        /* Find available slot. */
        for (tmp_idx = 0; tmp_idx < wfd_max_objects; tmp_idx++)
        {
            if (!wfd_objects[tmp_idx].wfd_bulk_get) /* This callback must be set for registered object */
                break;
        }
#endif /* !CONFIG_BCM_WLAN_DPDCTL */

        /* Get Min & Max Q Indices for this WFD Instance */
        minQIdx = wfd_get_minQIdx(tmp_idx);
        maxQIdx = wfd_get_maxQIdx(tmp_idx);
    }

#if (defined(CONFIG_BCM_RDPA)||defined(CONFIG_BCM_RDPA_MODULE)) && defined(XRDP)
    if ((rc = wfd_rdpa_init(wl_radio_idx)))
        goto wfd_bind_failure;
#endif

    memset(&wfd_objects[tmp_idx], 0, sizeof(wfd_objects[tmp_idx]));

    wfd_objects[tmp_idx].isValid = 1;
    wfd_objects[tmp_idx].wl_dev_p         = wl_dev_p;
    wfd_objects[tmp_idx].eFwdHookType     = eFwdHookType;
    wfd_objects[tmp_idx].isTxChainingReqd = isTxChainingReqd;

    if (eFwdHookType == WFD_WL_FWD_HOOKTYPE_SKB)
        wfd_objects[tmp_idx].wfd_bulk_get = wfd_bulk_skb_get;
    else
        wfd_objects[tmp_idx].wfd_bulk_get = wfd_bulk_fkb_get;

    wfd_objects[tmp_idx].wfd_fwdHook      = wfd_fwdHook;
    wfd_objects[tmp_idx].wfd_completeHook = wfd_completeHook;
    wfd_objects[tmp_idx].wl_chained_packets = 0;
    wfd_objects[tmp_idx].wl_mcast_packets = 0;
    wfd_objects[tmp_idx].wfd_acc_info_p  = wfd_acc_info_get(wl_radio_idx);
    wfd_objects[tmp_idx].wfd_idx  = tmp_idx;
    wfd_objects[tmp_idx].wfd_rx_work_avail  = 0;
    wfd_objects[tmp_idx].wfd_mcastHook = wfd_mcastHook;
    wfd_objects[tmp_idx].wl_radio_idx = wl_radio_idx;

    sprintf(threadname,"wfd%lu-thrd", tmp_idx);

#if defined(BCM_PKTFWD)
#if defined(WFD_FLCTL)
    /* Apply default BPM exhaustion level thresholds */
    if ((wl_pktlist_context != PKTLIST_CONTEXT_NULL) &&
        (eFwdHookType == WFD_WL_FWD_HOOKTYPE_SKB))
    {
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
        uint32_t bpm_total_skb = gbpm_total_skb();
        wfd_objects[tmp_idx].skb_exhaustion_lo =
            ((bpm_total_skb * WFD_FLCTL_SKB_EXHAUSTION_LO_PCNT) / 100);
        wfd_objects[tmp_idx].skb_exhaustion_hi =
            ((bpm_total_skb * WFD_FLCTL_SKB_EXHAUSTION_HI_PCNT) / 100);
#endif
        wfd_objects[tmp_idx].pkt_prio_favor = WFD_FLCTL_PKT_PRIO_FAVOR;
        printk("%s WL %u FLowControl "
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
            "total<%u> lo<%u> hi<%u> "
#endif
            "favor prio<%u>\n",
               threadname, wl_radio_idx,
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
               bpm_total_skb, wfd_objects[tmp_idx].skb_exhaustion_lo, wfd_objects[tmp_idx].skb_exhaustion_hi,
#endif
               wfd_objects[tmp_idx].pkt_prio_favor);
    }
#endif

    PKTLIST_TRACE(" radio %d wl_pktlist_context %p %pS",
                  wl_radio_idx, wl_pktlist_context, wfd_completeHook);

    if ((wl_pktlist_context != PKTLIST_CONTEXT_NULL) && (wfd_completeHook))
    {
        wfd_objects[tmp_idx].pktlist_context_p =
            pktlist_context_init(wl_pktlist_context,
                (pktlist_context_xfer_fn_t)wfd_completeHook,
                PKTLIST_CONTEXT_KEYMAP_NULL,
                &wfd_objects[tmp_idx], threadname, wl_radio_idx);

        if (wfd_objects[tmp_idx].pktlist_context_p == PKTLIST_CONTEXT_NULL)
        {
            WFD_ERROR("pktlist_context_init %s failed\n", threadname);
            rc = WFD_FAILURE;
            goto wfd_bind_failure;
        }

#if defined(BCM_PKTFWD_FLCTL)
        if ((eFwdHookType == WFD_WL_FWD_HOOKTYPE_SKB) &&
            (wl_pktlist_context->fctable != PKTLIST_FCTABLE_NULL))
        {
            /* Point WFD pktlist_fctable to WLAN pktlist_fctable */
            wfd_objects[tmp_idx].pktlist_context_p->fctable =
                wl_pktlist_context->fctable;

#if defined(WFD_FLCTL)
            /* Set pktlist_context pkt_prio_favor*/
            wl_pktlist_context->fctable->pkt_prio_favor =
                wfd_objects[tmp_idx].pkt_prio_favor;
#else /* ! WFD_FLCTL */
            wl_pktlist_context->fctable->pkt_prio_favor = PKTFWD_PRIO_MAX;
#endif /* WFD_FLCTL */

        }
        else
        {
            wfd_objects[tmp_idx].pktlist_context_p->fctable =
               PKTLIST_FCTABLE_NULL;
        }
#endif /* BCM_PKTFWD_FLCTL */

        WFD_ERROR("%s initialized pktlists: radio %u nodes %u xfer %pS\n",
            threadname, wl_radio_idx, PKTLIST_NODES_MAX, wfd_completeHook);
    }
#endif /* BCM_PKTFWD */

    /* Configure WFD RX queue */
    if (maxQIdx < WFD_NUM_QUEUE_SUPPORTED)
    {
        for (qidx = minQIdx; qidx <= maxQIdx; qidx++)
        {
           qid = wfd_get_qid(qidx);

           if ((rc = wfd_config_rx_queue(tmp_idx, qid, WFD_WLAN_QUEUE_MAX_SIZE, eFwdHookType)) != 0)
           {
               WFD_ERROR("Cannot configure WFD CPU Rx queue (%d), status (%d)\n", qid, rc);
               goto wfd_bind_failure;
           }

           wfd_int_enable (wl_radio_idx, qid, qidx); 

           wfd_objects[tmp_idx].wfd_queue_mask |= (1 << qidx);
        }

#if defined(WFD_SWQUEUE)
        /* Construct WFD SW Queue */
        rc = wfd_swqueue_init(&wfd_objects[tmp_idx], WFD_SWQUEUE_MAX_SIZE);
        if (rc != 0)
        {
            WFD_ERROR("WFD [%d] SW Queue construction failed status (%d)\n",
                      (int)tmp_idx, rc);
            goto wfd_bind_failure;
        }
#endif /* WFD_SWQUEUE */

        /* Create WFD Thread */
        init_waitqueue_head(&wfd_objects[tmp_idx].wfd_rx_thread_wqh);
        rx_thread = kthread_create(wfd_tasklet_handler, (void *)tmp_idx, threadname);
        if (IS_ERR(rx_thread)) 
        {
            return (int)PTR_ERR(rx_thread);
        }

        wfd_objects[tmp_idx].wfd_rx_thread = rx_thread;

        /* wlmngr manages the logic to bind the WFD threads to specific CPUs depending on platform
           Look at function wlmngr_setupTPs() for more details */
        //kthread_bind(wfd_objects[tmp_idx].wfd_rx_thread, tmp_idx);
        wake_up_process(wfd_objects[tmp_idx].wfd_rx_thread);

        printk("\033[1m\033[34m %s: Dev %s wfd_idx %lu wl_radio_idx %d"
            " Type %s:%s configured WFD thread %s "
            "minQId/maxQId (%d/%d), status (%d) qmask 0x%x\033[0m\n",
            __FUNCTION__, wl_dev_p->name, tmp_idx, wl_radio_idx,
            ((eFwdHookType == WFD_WL_FWD_HOOKTYPE_SKB) ? "skb" : "fkb"),
            (wfd_objects[tmp_idx].pktlist_context_p == NULL) ? "def" : "sll",
            threadname,
            wfd_get_qid(minQIdx), wfd_get_qid(maxQIdx), rc,
            wfd_objects[tmp_idx].wfd_queue_mask);
    }
    else
    {
        WFD_ERROR("ERROR qidx %d maxq %d\n", (int)maxQIdx,
                  (int)WFD_NUM_QUEUE_SUPPORTED);
    }

#if defined(CONFIG_BCM_PON)
    if (tmp_idx != WFD_BRIDGED_OBJECT_IDX)
#endif
        wfd_objects_num++;
    return (int)tmp_idx;

wfd_bind_failure:
    return rc;
}
EXPORT_SYMBOL(wfd_bind);

void wfd_unbind(int wfdIdx, enumWFD_WlFwdHookType hook_type)
{
    int qidx, qid;
    int minQIdx;
    int maxQIdx;

    // simple reclaim iff idx of last bind
    if ((wfdIdx < 0) || (wfdIdx >= WFD_MAX_OBJECTS))
    {
        WFD_ERROR("wfd_idx %d out of bounds %d\n", wfdIdx, WFD_MAX_OBJECTS);
        return;
    }

    if (wfd_objects[wfdIdx].isValid == 0)
    {
        WFD_ERROR("wfd_idx %d is not initialized\n", wfdIdx);
        return;
    }

#if defined(CONFIG_BCM96838) ||defined(CONFIG_BCM96848)
    if (wfdIdx == WFD_BRIDGED_OBJECT_IDX)
    {
        /*This unbind is for bridged traffic use the bridged qidx */
        minQIdx = maxQIdx = WFD_BRIDGED_QUEUE_IDX;
    }
    else
#endif
    {
        /* Get Min & Max Q Indices for this WFD Instance */
        minQIdx = wfd_get_minQIdx(wfdIdx);
        maxQIdx = wfd_get_maxQIdx(wfdIdx);
    }


    /* free the pci rx queue(s); disable the interrupt(s) */
    for (qidx = minQIdx; qidx <= maxQIdx; qidx++) 
    {
        /* Deconfigure PCI RX queue(s) */
        qid = wfd_get_qid(qidx);
        wfd_config_rx_queue(wfdIdx, qid, 0, hook_type);
        wfd_objects[wfdIdx].wfd_queue_mask &= ~(1 << qidx);
        wfd_int_disable(wfd_objects[wfdIdx].wl_radio_idx, qid, qidx);
    }

#if (defined(CONFIG_BCM_RDPA)||defined(CONFIG_BCM_RDPA_MODULE)) && defined(XRDP)
   wfd_rdpa_uninit(wfd_objects[wfdIdx].wl_radio_idx);
#endif

   if (wfd_objects[wfdIdx].wfd_rx_thread)
            kthread_stop(wfd_objects[wfdIdx].wfd_rx_thread);

#if defined(BCM_PKTFWD)
    if (wfd_objects[wfdIdx].pktlist_context_p)
    {
        pktlist_context_t *pktlist_context_p;
        pktlist_context_p = wfd_objects[wfdIdx].pktlist_context_p;

        /* Debug dump using pktlist_context_dump() */
        pktlist_context_dump(pktlist_context_p, true, true);
        /* Free pktlist_context resources using pktlist_context_fini() */
        pktlist_context_fini(pktlist_context_p);
    }
#endif /* BCM_PKTFWD */

#if defined(WFD_SWQUEUE)
        /* Destruct WFD SW Queue */
        wfd_swqueue_fini(wfdIdx);
#endif /* WFD_SWQUEUE */

   memset(&wfd_objects[wfdIdx], 0, sizeof wfd_objects[wfdIdx]);
   wfd_objects_num--;
}
EXPORT_SYMBOL(wfd_unbind);

int wfd_registerdevice(uint32_t wfd_idx, int ifidx, struct net_device *dev)
{
    if (wfd_idx >= WFD_MAX_OBJECTS)
    {
        printk("%s Error Incorrect wfd_idx %d passed\n", __FUNCTION__, wfd_idx);
        return -1;
    }
   
    WFD_ASSERT(ifidx < WIFI_MW_MAX_NUM_IF);
    if (ifidx >= WIFI_MW_MAX_NUM_IF)
    {
        printk("%s Error ifidx %d out of bounds(%d)\n", 
            __FUNCTION__, ifidx, WIFI_MW_MAX_NUM_IF);
        return -1;
    }

    WFD_ASSERT(wfd_objects[wfd_idx].wl_if_dev[ifidx] == NULL);
    if (wfd_objects[wfd_idx].wl_if_dev[ifidx] != NULL)
    {
        printk("%s Device already registered for wfd_idx %d ifidx %d\n", 
            __FUNCTION__, wfd_idx, ifidx);
        return -1;
    }

    wfd_objects[wfd_idx].wl_if_dev[ifidx] = dev;

    printk("%s Successfully registered dev %s ifidx %d wfd_idx %d\n", 
        __FUNCTION__, dev->name, ifidx, wfd_idx);

    return 0;
}
EXPORT_SYMBOL(wfd_registerdevice);


int wfd_unregisterdevice(uint32_t wfd_idx, int ifidx)
{
    if (wfd_idx >= WFD_MAX_OBJECTS)
    {
        printk("%s Error Incorrect wfd_idx %d passed\n", __FUNCTION__, wfd_idx);
        return -1;
    }
   
    WFD_ASSERT(ifidx < WIFI_MW_MAX_NUM_IF);
    if (ifidx >= WIFI_MW_MAX_NUM_IF)
    {
        printk("%s Error ifidx %d out of bounds(%d)\n", 
            __FUNCTION__, ifidx, WIFI_MW_MAX_NUM_IF);
        return -1;
    }

#if (defined(CONFIG_BCM_MCAST) || defined(CONFIG_BCM_MCAST_MODULE))
    bcm_mcast_handle_dev_down(wfd_objects[wfd_idx].wl_if_dev[ifidx]);
#endif
    wfd_objects[wfd_idx].wl_if_dev[ifidx] = NULL;

    printk("%s Successfully unregistered ifidx %d wfd_idx %d\n", 
        __FUNCTION__, ifidx, wfd_idx);

    return 0;
}
EXPORT_SYMBOL(wfd_unregisterdevice);


static void wfd_dump(void)
{
    unsigned long flags;
    int idx;

    for (idx = 0; idx < WFD_MAX_OBJECTS; idx++)
    {
       WFD_IRQ_LOCK(idx, flags);
       printk("wfd_rx_work_avail 0x%lx qmask 0x%x\n",
              wfd_objects[idx].wfd_rx_work_avail, wfd_objects[idx].wfd_queue_mask);
       WFD_IRQ_UNLOCK(idx, flags);
    }
}

#if defined(WFD_SWQUEUE)

/**
 * =============================================================================
 * Section: WFD SW QUEUE Functional Interface
 * =============================================================================
 */

/**
 * -----------------------------------------------------------------------------
 *
 * Function   : wfd_swqueue_init
 * Description: Construct all WFD SW queue subsystem.
 *              Invoked in wfd_bind per WFD instance.
 *
 *              Initialize SW queue and swq_xmit_fn handler to transmit SWq
 *              packets based on eFwdHookType (FKB | SKB).
 *              Register "flush" and "flush complete" handler with bcm_pktfwd.
 *              These handlers are used by ingress network devices to enqueue
 *              packtes to SW queue and inform egress network device (WFD) for
 *              arrival of new packets in SW queue.
 *
 * -----------------------------------------------------------------------------
 */

static int
wfd_swqueue_init(wfd_object_t * wfd_p, uint32_t swq_size)
{
    int                 mem_bytes;
    wfd_swqueue_t     * wfd_swqueue;

    mem_bytes = sizeof(wfd_swqueue_t);

    wfd_swqueue = (wfd_swqueue_t *) kmalloc(mem_bytes, GFP_ATOMIC);
    if (wfd_swqueue == WFD_SWQUEUE_NULL)
    {
        WFD_ERROR("wfd_swqueue kmalloc %d failure", mem_bytes);
        return WFD_FAILURE;
    }

    memset(wfd_swqueue, 0, mem_bytes);

    spin_lock_init(&wfd_swqueue->lock); /* Initialize SWq lock */

    wfd_swqueue->domain         = wfd_p->wfd_idx;
    wfd_swqueue->swq_schedule   = 0; /* Schedule WFD SW queue dispatch */

    /* Validate and Set Queue size */
    if (swq_size < WFD_SWQUEUE_MIN_SIZE)
        swq_size = WFD_SWQUEUE_MIN_SIZE;
    else if (swq_size > WFD_SWQUEUE_MAX_SIZE)
        swq_size = WFD_SWQUEUE_MAX_SIZE;

    wfd_swqueue->swq_size = swq_size;

    if (wfd_p->eFwdHookType == WFD_WL_FWD_HOOKTYPE_SKB)
    {
        wfd_swqueue->swq_xmit_fn = wfd_swqueue_skb_xmit;
        wfd_swqueue->pktqueue.NBuffPtrType = SKBUFF_PTR; /* never reset */
    }
    else
    {
        wfd_swqueue->swq_xmit_fn = wfd_swqueue_fkb_xmit;
        wfd_swqueue->pktqueue.NBuffPtrType = FKBUFF_PTR; /* never reset */
    }

    PKTQUEUE_RESET(&wfd_swqueue->pktqueue);  /* head,tail, not reset */

    /* Sanity check */
    WFD_ASSERT(wfd_swqueue->domain < PKTFWD_DOMAINS_WLAN);

    /* Register "flush" & "flush complete" handlers with bcm_pktfwd. */
    wfd_swqueue->pktqueue_context_p =
        pktqueue_context_register(wfd_swqueue_flush_pkts,
                wfd_swqueue_flush_complete,
                wfd_p, wfd_swqueue->domain);

    wfd_p->wfd_swqueue = wfd_swqueue;

    PKTQUEUE_TRACE("WFD SW queue initialized swq_size-%d", swq_size);

    return WFD_SUCCESS;
}   /* wfd_swqueue_init() */

/**
 * -----------------------------------------------------------------------------
 *
 * Function   : wfd_swqueue_fini
 * Description: Destruct WFD SW queue subsystem.
 *              Invoked in wfd_unbind per WFD instance.
 *
 * -----------------------------------------------------------------------------
 */

static void
wfd_swqueue_fini(uint32_t wfd_idx)
{
    wfd_swqueue_t      * wfd_swqueue;
    pktqueue_context_t * pktqueue_context_p;

    wfd_swqueue = wfd_objects[wfd_idx].wfd_swqueue;
    wfd_objects[wfd_idx].wfd_swqueue = WFD_SWQUEUE_NULL;

    WFD_ASSERT(wfd_swqueue != WFD_SWQUEUE_NULL);

    pktqueue_context_p = wfd_swqueue->pktqueue_context_p;
    wfd_swqueue->pktqueue_context_p = PKTQUEUE_CONTEXT_NULL;

    if (pktqueue_context_p != PKTQUEUE_CONTEXT_NULL)
    {
        /* Debug dump using pktqueue_context_dump() */
        pktqueue_context_dump(pktqueue_context_p);

        /* Free pktqueue_context resources */
        pktqueue_context_unregister(pktqueue_context_p);
    }

    memset(wfd_swqueue, 0xff, sizeof(wfd_swqueue_t));  /* scribble */
    kfree(wfd_swqueue);

    PKTQUEUE_TRACE("WFD SW queue Destructed");

}   /* wfd_swqueue_fini() */


/**
 * =============================================================================
 * Section: SW Queue Packet Processing
 * =============================================================================
 */

/**
 * -----------------------------------------------------------------------------
 *
 * Callback function registered with bcm_pktfwd.
 *
 * Operation: Ingress device driver (WLAN) will bin packets into its domain
 * specific local pktqueue. Once all packets are binned, Ingress device driver
 * will invoke wfd_swqueue_flush_pkts() to "flush" packets from WLAN pktqueue to
 * corresponding peer (WFD) pktqueue.
 * Once all packets in pktqueue are flushed, ingress device driver will invoke
 * wfd_swqueue_flush_complete().
 * wfd_swqueue_flush_complete() will schedule WFD thread to move packets from
 * SW queue to pktlists in WFD pktlist_context.
 *
 * wfd_swqueue_flush_pkts() and wfd_swqueue_flush_complete() are invoked in
 * ingress device driver "WLAN" thread context.
 *
 * Helper   : Helper functions used by WFD driver for flush packets are,
 *      wfd_swqueue_xfer_pkts() : Translate network buffers in ingress pktqueue
 *                                to egress device NBuffPtrType and append it
 *                                to ENET SW queue.
 * TODO:
 *      - Use skb->wl.pktfwd instead of skb->wl.ucast.nic in downstream path
 *
 * -----------------------------------------------------------------------------
 */

/** Translate and transfer network buffers in ingress device pktqueue */
static void
wfd_swqueue_xfer_pkts(
    wfd_object_t    * wfd_p,
    pktqueue_t      * src_pktqueue,       /* producer's pktqueue */
    pktqueue_t      * dst_pktqueue)       /* consumer's pktqueue */
{
    wlFlowInf_t         wlFlowInf;
    FkBuff_t          * fkb;
    struct sk_buff    * skb;
    pktqueue_pkt_t    * pkt;

    WFD_ASSERT(src_pktqueue->len != 0U);

    if (src_pktqueue->NBuffPtrType == dst_pktqueue->NBuffPtrType)
    {
        __pktqueue_xfer_pkts(src_pktqueue, dst_pktqueue,
                             dst_pktqueue->NBuffPtrType);
    }
    else if (src_pktqueue->NBuffPtrType == FKBUFF_PTR)
    {   /* FKB ==> SKB */
        uint8_t prio4bit;

        while (src_pktqueue->len)
        {
            pkt                 = src_pktqueue->head;
            src_pktqueue->head  = PKTQUEUE_PKT_SLL(pkt, FKBUFF_PTR);
            PKTQUEUE_PKT_SET_SLL(pkt, PKTQUEUE_PKT_NULL, FKBUFF_PTR);
            src_pktqueue->len--;

            fkb = PNBUFF_2_FKBUFF(pkt);
            wlFlowInf.u32 = fkb->wl.u32;

            /* Convert to skb */
            skb = nbuff_xlate(pkt);
            if (skb == NULL)
            {
                WFD_ERROR("nbuff_xlate failed\n");
                PKTQUEUE_PKT_FREE(pkt);
                wfd_p->wfd_swqueue->pkts_dropped++;
                continue;
            }

            pkt = (pktqueue_pkt_t *) skb;

            /* Restore d3lut pktfwd_key_t */
            skb->wl.u32 = wlFlowInf.u32;

            /* FKB (DHD) uses 3 bit prio.
             * Convert it to 4 bit prio and assign it to SKB (NIC)
             * IQ_PRIO is not available, use only prio */
            prio4bit = 0;
            prio4bit = SET_WLAN_PRIORITY(prio4bit, wlFlowInf.ucast.dhd.wl_prio);

            skb->wl.pktfwd.wl_prio = prio4bit;
            DECODE_WLAN_PRIORITY_MARK(prio4bit, skb->mark);

            /* Transfer packet to WFD pktqueue */
            if (dst_pktqueue->len == 0U)
                dst_pktqueue->head = pkt;
             else
                PKTQUEUE_PKT_SET_SLL(dst_pktqueue->tail, pkt, SKBUFF_PTR);

            dst_pktqueue->tail = pkt;
            dst_pktqueue->len++;
        } /* while (src_pktqueue->len) */
    }
    else
    {   /* SKB ==> FKB */

        while (src_pktqueue->len)
        {
            pkt                 = src_pktqueue->head;
            src_pktqueue->head  = PKTQUEUE_PKT_SLL(pkt, SKBUFF_PTR);
            PKTQUEUE_PKT_SET_SLL(pkt, PKTQUEUE_PKT_NULL, SKBUFF_PTR);
            src_pktqueue->len--;

            skb             = PNBUFF_2_SKBUFF(pkt);
            wlFlowInf.u32   = skb->wl.u32;

            /* No blog should be attached */
            WFD_ASSERT(skb->blog_p == NULL);
            /* Only BPM buffers are supported */
            WFD_ASSERT(skb->recycle_flags & SKB_DATA_RECYCLE);

            /* Convert to fkb */
            fkb = fkb_init(BPM_PHEAD_TO_BUF(skb->head), BCM_PKT_HEADROOM,
                           skb->data, skb->len);

            fkb->recycle_hook       = gbpm_recycle_pNBuff;
            fkb->recycle_context    = 0;

            /* No vaild dirty_p in fkb any more, flush whole pkt .*/
            cache_flush_len(fkb->data, fkb->len);

            /* Free SKB header */
            skb_header_free(skb);

            /* Restore d3lut pktfwd_key_t */
            fkb->wl.u32 = wlFlowInf.u32;

            /* SKB (NIC) uses 4 bit prio.
             * Convert it to 3 bit prio (ignore IQ_PRIO)
             * and assign to FKB (DHD) */
            fkb->wl.pktfwd.wl_prio =
                GET_WLAN_PRIORITY(wlFlowInf.ucast.nic.wl_prio);

            pkt = (pktqueue_pkt_t *) FKBUFF_2_PNBUFF(fkb);

            /* Transfer packet to WFD pktqueue */
            if (dst_pktqueue->len == 0U)
                dst_pktqueue->head = pkt;
             else
                PKTQUEUE_PKT_SET_SLL(dst_pktqueue->tail, pkt, FKBUFF_PTR);

            dst_pktqueue->tail = pkt;
            dst_pktqueue->len++;
        } /* while (src_pktqueue->len) */
    }
}   /* wfd_swqueue_xfer_pkts() */


/** Flush pkts from Ingress pktqueue to WFD SW queue */
static bool
wfd_swqueue_flush_pkts(void * driver, pktqueue_t * pktqueue)
{
    wfd_object_t      * wfd_p = (wfd_object_t *)driver;
    wfd_swqueue_t     * wfd_swqueue = wfd_p->wfd_swqueue;
    pktqueue_t        * wfd_pktqueue;

    WFD_ASSERT(wfd_swqueue != WFD_SWQUEUE_NULL);

    WFD_SWQUEUE_LOCK(wfd_swqueue); // +++++++++++++++++++++++++++++++++++++++++

    wfd_pktqueue = &wfd_swqueue->pktqueue;

    /* Check for Queue avail */
    /* TODO: Append avail len and drop remaining packets */
    if (unlikely(wfd_pktqueue->len + pktqueue->len > wfd_swqueue->swq_size))
    {
        wfd_swqueue->pkts_dropped += pktqueue->len;
        __pktqueue_free_pkts(pktqueue, pktqueue->NBuffPtrType);
        goto wfd_swqueue_flush_pkts_done;
    }

    wfd_swqueue_xfer_pkts(wfd_p, pktqueue, wfd_pktqueue);

wfd_swqueue_flush_pkts_done:
    PKTQUEUE_RESET(pktqueue); /* head,tail, not reset */

    WFD_SWQUEUE_UNLK(wfd_swqueue); // -----------------------------------------

    return true;
}   /* wfd_swqueue_flush_pkts() */


/* Flush complete invoked by ingress driver (WLAN) */
static void
wfd_swqueue_flush_complete(void * driver)
{
    wfd_object_t      * wfd_p = (wfd_object_t *)driver;
    wfd_swqueue_t     * wfd_swqueue = wfd_p->wfd_swqueue;

    if (likely(wfd_swqueue->swq_schedule == 0))
    {
        wfd_swqueue->schedule_cnt++;
        wfd_swqueue->swq_schedule = ~0;

        /* Wake up WFD thread to xmit packets from SW queue */
        WFD_WAKEUP_RXWORKER(wfd_p->wfd_idx);
    }
}   /* wfd_swqueue_flush_complete() */


/**
 * -----------------------------------------------------------------------------
 *
 * Function   : wfd_swqueue_skb_xmit
 * Description: SW Queue transmit func (wfd_swqueue_xmit_fn_t) for SKB buffers.
 *              Extract packets from a SW queue and bin into pktlists of local
 *              pktlist_context (WFD).
 *              After all (or bounded by a budget) packets are added to packet
 *              lists, packet lists be handed off to the peer pktlist_context
 *              using the attached "xfer" handler.
 * -----------------------------------------------------------------------------
 */

static uint32_t
wfd_swqueue_skb_xmit(wfd_object_t * wfd_p, uint32_t budget)
{
    uint16_t pktfwd_key;
    uint16_t pktlist_prio, pktlist_dest;
    uint32_t            rx_pktcnt;
    pktqueue_t          temp_pktqueue; /* Declared on stack */
    struct sk_buff    * skb;
    pktqueue_pkt_t    * pkt;
    pktqueue_t        * pktqueue;
    wfd_swqueue_t     * wfd_swqueue = wfd_p->wfd_swqueue;
    pktlist_context_t * pktlist_context = wfd_p->pktlist_context_p;

    pktqueue    = &wfd_swqueue->pktqueue;

    WFD_SWQUEUE_LOCK(wfd_swqueue); // +++++++++++++++++++++++++++++++++++++++++

    /* Transfer packets to a local pktqueue */
    temp_pktqueue.head   = pktqueue->head;
    temp_pktqueue.tail   = pktqueue->tail;
    temp_pktqueue.len    = pktqueue->len;

    PKTQUEUE_RESET(pktqueue); /* head,tail, not reset */

    WFD_SWQUEUE_UNLK(wfd_swqueue); // -----------------------------------------

    /* Now lock-less; transmit packets from local pktqueue */

    pktfwd_key  = 0; /* 2b-radio, 2b-incarn, 12b-dest */
    rx_pktcnt   = 0;

    while (budget)
    {
        if (temp_pktqueue.len != 0U)
        {
            pkt             = temp_pktqueue.head;
            temp_pktqueue.head  = PKTQUEUE_PKT_SLL(pkt, SKBUFF_PTR);
            PKTQUEUE_PKT_SET_SLL(pkt, PKTQUEUE_PKT_NULL, SKBUFF_PTR);
            temp_pktqueue.len--;

            skb = (struct sk_buff *)pkt;

            /* Fetch the 3b WLAN prio, by skipping the 1b IQPRIO */
            pktlist_prio = GET_WLAN_PRIORITY(skb->wl.pktfwd.wl_prio);

            if (likely(skb->wl.pktfwd.is_ucast))
            {
                pktfwd_key = skb->wl.pktfwd.pktfwd_key & PKTC_CHAIN_IDX_MASK;
                pktlist_dest = PKTLIST_DEST(pktfwd_key);

                /* Reset pktfwd FlowInf and set wl FlowInf */
                skb->wl.u32 = 0U;

                /* No keymap required for WLAN NIC mode */

                /* Set wl FlowInf */
                skb->wl.ucast.nic.is_ucast      = 1;
                skb->wl.ucast.nic.wl_chainidx   = pktfwd_key;

            }
            else
            {
                skb->wl.u32  = 0U;
                pktlist_dest = PKTLIST_MCAST_ELEM; /* last col in 2d pktlist */
            }

            skb->mark = LINUX_SET_PRIO_MARK(skb->mark, pktlist_prio);

#if defined(BCM_PKTFWD_FLCTL)
            /* FIXME: Implement a credit based flow control logic for SWq. */
#endif /* BCM_PKTFWD_FLCTL */

            WFD_ASSERT(pktlist_dest <= PKTLIST_MCAST_ELEM);
            WFD_ASSERT(pktlist_prio <  PKTLIST_PRIO_MAX);

            /* add to local pktlist */
            __pktlist_add_pkt(pktlist_context, pktlist_prio, pktlist_dest,
                              pktfwd_key, (pktlist_pkt_t *)skb, SKBUFF_PTR);

#if defined(BCM_PKTFWD_FLCTL)
            if (pktlist_context->fctable != PKTLIST_FCTABLE_NULL)
            {
                /* Decreament avail credits for pktlist */
                __pktlist_fctable_dec_credits(pktlist_context, pktlist_prio,
                        pktlist_dest);
            }
#endif /* BCM_PKTFWD_FLCTL */

            ++rx_pktcnt;

        }
        else /* temp_pktqueue.len == 0 : No more packets to read */
        {
            break;
        }

        --budget;
    } /* while (budget) */

    if (temp_pktqueue.len != 0U) {
        /* Out of budget, prepend left-over packets to ENET SWq */

        WFD_SWQUEUE_LOCK(wfd_swqueue); // +++++++++++++++++++++++++++++++++++++

        if (pktqueue->len == 0) {
            pktqueue->tail = temp_pktqueue.tail;
        } else {
            PKTQUEUE_PKT_SET_SLL(temp_pktqueue.tail, pktqueue->head, SKBUFF_PTR);
        }

        pktqueue->head = temp_pktqueue.head;
        pktqueue->len += temp_pktqueue.len;

        WFD_SWQUEUE_UNLK(wfd_swqueue); // -------------------------------------

        PKTQUEUE_RESET(&temp_pktqueue); /* head,tail, not reset */

    }

    if (likely(rx_pktcnt))
    {
        /* Dispatch all pending pktlists to peer's pktlist_context */
        wfd_pktfwd_xfer(wfd_p->pktlist_context_p, SKBUFF_PTR);
    }

    return rx_pktcnt;

}   /* wfd_swqueue_skb_xmit() */


/**
 * -----------------------------------------------------------------------------
 *
 * Function   : wfd_swqueue_fkb_xmit
 * Description: SW Queue transmit func (wfd_swqueue_xmit_fn_t) for FKB buffers.
 *              Extract packets from a SW queue and forward to egress network
 *              device.
 *
 *              TODO: Extract packets from a SW queue and bin into pktlists of
 *              local pktlist_context (WFD).
 *              After all (or bounded by a budget) packets are added to packet
 *              lists, packet lists be handed off to the peer pktlist_context
 *              using the attached "xfer" handler.
 * -----------------------------------------------------------------------------
 */

static uint32_t
wfd_swqueue_fkb_xmit(wfd_object_t * wfd_p, uint32_t budget)
{
    uint16_t pktfwd_key, flowring_idx;
    uint16_t pktlist_prio, pktlist_dest;
    uint32_t ssid;
    uint32_t            rx_pktcnt;
    pktqueue_t          temp_pktqueue; /* Declared on stack */
    FkBuff_t          * fkb;
    pktqueue_pkt_t    * pkt;
    pktqueue_t        * pktqueue;
    wfd_swqueue_t     * wfd_swqueue = wfd_p->wfd_swqueue;
    pktlist_context_t * pktlist_context = wfd_p->pktlist_context_p;
    pktlist_context_t * peer_pktlist_context = pktlist_context->peer;

    ASSERT(peer_pktlist_context->keymap_fn);

    pktqueue    = &wfd_swqueue->pktqueue;

    WFD_SWQUEUE_LOCK(wfd_swqueue); // +++++++++++++++++++++++++++++++++++++++++

    /* Transfer packets to a local pktqueue */
    temp_pktqueue.head   = pktqueue->head;
    temp_pktqueue.tail   = pktqueue->tail;
    temp_pktqueue.len    = pktqueue->len;

    PKTQUEUE_RESET(pktqueue); /* head,tail, not reset */

    WFD_SWQUEUE_UNLK(wfd_swqueue); // -----------------------------------------

    /* Now lock-less; transmit packets from local pktqueue */

    pktfwd_key  = 0; /* 2b-radio, 2b-incarn, 12b-dest */
    rx_pktcnt   = 0;

    while (budget)
    {
        if (temp_pktqueue.len != 0U)
        {
            pkt             = temp_pktqueue.head;
            temp_pktqueue.head  = PKTQUEUE_PKT_SLL(pkt, FKBUFF_PTR);
            PKTQUEUE_PKT_SET_SLL(pkt, PKTQUEUE_PKT_NULL, FKBUFF_PTR);
            temp_pktqueue.len--;

            fkb = PNBUFF_2_FKBUFF(pkt);

            pktlist_prio = fkb->wl.pktfwd.wl_prio;

            if (likely(fkb->wl.pktfwd.is_ucast))
            {
                ssid            = fkb->wl.pktfwd.ssid;
                pktfwd_key      = fkb->wl.pktfwd.pktfwd_key;
                pktlist_dest    = PKTLIST_DEST(pktfwd_key);
                flowring_idx    = (uint16_t) ~0;

                /* Reset pktfwd FlowInf and set dhd FlowInf */
                fkb->wl.u32 = 0U;

                /* Get flowring_idx from pktfwd_key */
                (peer_pktlist_context->keymap_fn)(wfd_p->wl_radio_idx,
                    &pktfwd_key, &flowring_idx, pktlist_prio, PKTFWD_KEYMAP_K2F);

                /* Set dhd FlowInf */
                fkb->wl.ucast.dhd.is_ucast      = 1;
                fkb->wl.ucast.dhd.flowring_idx  = flowring_idx;
                fkb->wl.ucast.dhd.ssid          = ssid;
            }
            else
            {
                fkb->wl.u32  = 0U;
                pktlist_dest = PKTLIST_MCAST_ELEM; /* last col in 2d pktlist */
            }

            WFD_ASSERT(pktlist_dest <= PKTLIST_MCAST_ELEM);
            WFD_ASSERT(pktlist_prio <  PKTLIST_PRIO_MAX);

            /* add to local pktlist */
            __pktlist_add_pkt(pktlist_context, pktlist_prio, pktlist_dest,
                              pktfwd_key, pkt /* fkb */, FKBUFF_PTR);
            ++rx_pktcnt;

        }
        else /* temp_pktqueue.len == 0 : No more packets to read */
        {
            break;
        }

        --budget;
    } /* while (budget) */

    if (temp_pktqueue.len != 0U) {
        /* Out of budget, prepend left-over packets to ENET SWq */

        WFD_SWQUEUE_LOCK(wfd_swqueue); // +++++++++++++++++++++++++++++++++++++

        if (pktqueue->len == 0) {
            pktqueue->tail = temp_pktqueue.tail;
        } else {
            PKTQUEUE_PKT_SET_SLL(temp_pktqueue.tail, pktqueue->head, FKBUFF_PTR);
        }

        pktqueue->head = temp_pktqueue.head;
        pktqueue->len += temp_pktqueue.len;

        WFD_SWQUEUE_UNLK(wfd_swqueue); // -------------------------------------

        PKTQUEUE_RESET(&temp_pktqueue); /* head,tail, not reset */

    }

    if (likely(rx_pktcnt))
    {
        /* Dispatch all pending pktlists to peer's pktlist_context */
        wfd_pktfwd_xfer(wfd_p->pktlist_context_p, FKBUFF_PTR);
    }

    return rx_pktcnt;

}   /* wfd_swqueue_fkb_xmit() */


#endif /* WFD_SWQUEUE */

MODULE_DESCRIPTION("WLAN Forwarding Driver");
MODULE_AUTHOR("Broadcom");
MODULE_LICENSE("GPL");

module_init(wfd_dev_init);
module_exit(wfd_dev_close);
