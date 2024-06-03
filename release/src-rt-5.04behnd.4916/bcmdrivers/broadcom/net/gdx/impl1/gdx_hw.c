/*
   <:copyright-BRCM:2021:DUAL/GPL:standard
   
      Copyright (c) 2021 Broadcom 
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
/** Generic Device Accelerator (GDX) Driver                                **/
/**                                                                        **/
/** Title:                                                                 **/
/**                                                                        **/
/**   Generic device accelerator interface.                                **/
/**                                                                        **/
/** Abstract:                                                              **/
/**                                                                        **/
/**   Mediation layer between the Generic driver and the Accelerator       **/
/**  (Runner, Archer, etc.)                                                **/
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
#include <linux/sched.h>
#include <uapi/linux/sched/types.h>
#include <linux/hashtable.h>
#include <linux/bcm_log.h>
#include <linux/bcm_realtime.h>
#include <linux/bcm_skb_defines.h>
#include <linux/nbuff.h>
#include "linux/blog.h"
#include "gdx.h"
#include "gdx_hw.h"
#include "bpm.h"

/****************************************************************************/
/***************************** Module Version *******************************/
/****************************************************************************/
static const char *version = "Generic Device Accelerator (GDX) Driver";

#define GDX_IF_NAME_STR_LEN  (IFNAMSIZ)

#define GDX_INTERRUPT_COALESCING_TIMEOUT_US 500
#define GDX_INTERRUPT_COALESCING_MAX_PKT_CNT 32

/****************************************************************************/
static struct proc_dir_entry *proc_gdx_dir;          /* /proc/gdx */

static int gdx_add_gendev(int group_idx, struct net_device *dev);
static int gdx_delete_gendev(struct net_device *dev);
static int gdx_add_dev_group(int group_idx);
static int gdx_delete_dev_group(int group_idx);
static int gdx_gendev_notifier(struct net_device *dev, int event, int group_idx);
static int gdx_gendev_get_blog_info(struct sk_buff *skb_p);
static inline void gdx_hwacc_recycle(void *nbuff_p, unsigned long context, uint32_t flags);
static inline FkBuff_t *gdx_fkb_alloc(gdx_hwacc_rx_info_t *info, int prepend_size);
static inline void gdx_initialize_skb(struct sk_buff *skb, gdx_hwacc_rx_info_t *info, int prepend_size);
static inline struct sk_buff *gdx_skb_alloc(gdx_hwacc_rx_info_t *info, int prepend_size);

extern int gdx_print_lvl;

extern void bcm_fro_tcp_complete(void);
extern void gdx_dev_xmit(pNBuff_t nbuff, void *dev, BlogFcArgs_t *args);

/****************************************************************************/
/***************************** Module parameters*****************************/
/****************************************************************************/
/* Number of packets to read in each tasklet iteration */
#define GDX_NUM_PKTS_TO_READ_MAX 64
static int gdx_num_pkts_to_read = GDX_NUM_PKTS_TO_READ_MAX;
module_param (gdx_num_pkts_to_read, int, 0);

/* gdx Broadcom prefix */
static char gdx_prefix [GDX_IF_NAME_STR_LEN] = "gdx";
module_param_string (gdx_prefix, gdx_prefix, GDX_IF_NAME_STR_LEN, 0);

/* Forward declaration */
struct gdx_dev_group;
typedef struct gdx_dev_group gdx_dev_group_t;
static GDX_Prepend_FillInfo_t g_fill_info;  

/* CPU_RX queue stats */
struct gdx_queue_stats
{
    unsigned int cpu_rxq_rx_pkts;
    unsigned int cpu_rxq_max_rx_pkts;
    unsigned int cpu_rxq_rx_no_dev;
    unsigned int cpu_rxq_no_skbs;
};
typedef struct gdx_queue_stats gdx_queue_stats_t;

/* Generic Device stats */
struct gdx_dev_stats
{
    unsigned int dev_rx_pkts;
    unsigned int dev_tx_pkts;               /* fwd/accelerated pkts received from CPU-RX */
    unsigned int cpu_rxq_lpbk_pkts;         /* exception pkts received from CPU-RX */
    unsigned int cpu_rxq_lpbk_no_dev_pkts;  /* no matching dev found for exception pkts*/
};
typedef struct gdx_dev_stats gdx_dev_stats_t;

struct gdx_gendev_info
{
    uint16_t            gdx_group_id;  /* Used to designate this devinfo to a specific gdx group */
    uint16_t            gdx_dev_id;    /* used for CPU_RX queue, Non zero value is valid and zero means unused/invalid */
    gdx_dev_stats_t     dev_stats;
};
typedef struct gdx_gendev_info gdx_gendev_info_t;

struct gdx_dev_group
{
    int                 is_valid;
    unsigned int        group_idx;

    int16_t             gendev_count;

    unsigned int        dev_rx_total_pkts;  /* # of RX pkts callback from Linux stack */
    unsigned int        dev_rx_pkts;        /* # of RX pkts received by dev (and TX to CPU-TX) */
    unsigned int        dev_rx_no_dev;      /* # of RX pkts dropped because of no matching dev */
    unsigned int        dev_rx_error;       /* # of RX pkts dropped because of rx error */

    unsigned int        dev_tx_pkts;        /* # of TX pkts sent by the dev */

    unsigned int        cpu_rxq_total_pkts;       /* # pkts read from the CPU-RX queue */
    unsigned int        cpu_rxq_valid_pkts;       /* # valid pkts finally received from CPU-RX queue */
    unsigned int        cpu_rxq_lpbk_pkts;        /* exception pkts received from CPU-RX queue */
    unsigned int        cpu_rxq_lpbk_no_dev;      /* no matching dev for exception pkts*/
    unsigned int        cpu_rxq_tx_pkts;          /* valid tx pkts received from CPU-RX queue */
    unsigned int        cpu_rxq_tx_no_dev;        /* no matching dev for tx pkts */

    struct mutex        group_lock;
    char                dev_name[IFNAMSIZ];
} ____cacheline_aligned;

static gdx_dev_group_t gdx_dev_groups[GDX_MAX_DEV_GROUPS];
static int gdx_dev_groups_count = 0; /* total #of dev groups */
static int gdx_gendev_count = 0;    /* total #of active gendev */
static gdx_gendev_info_t **gdx_gendev_info_pp;

#if defined(GDX_PKT_DUMP)
/*
 *------------------------------------------------------------------------------
 * Function     : gdx_pkt_dump
 * Description  : Dump a buffer data
 *------------------------------------------------------------------------------
 */
void gdx_pkt_dump(char *str, pNBuff_t pNBuff)
{
    int len;
    uint8_t *data; 
    uint32_t length;

    if (IS_SKBUFF_PTR(pNBuff))
    {
        struct sk_buff *skb = PNBUFF_2_SKBUFF(pNBuff);
        data = skb->data;
        length = skb->len;
    }
    else
    {
        struct fkbuff *fkb = PNBUFF_2_FKBUFF(pNBuff);
        data = fkb->data;
        length = fkb->len;
    }

    if (gdx_print_lvl >= GDX_PRINT_LVL_DBG)
    {
        len = (length > 64) ? 64 : length;
        printk(CLRg " %s<%px> data<%px> len<%d>\n\t",
                str, pNBuff, data, length);
        bcm_pr_hex_dump_offset(16, 1, data, len, 0);
        printk(CLRnl);
//        cache_flush_data_len((void*)data, len);
    }
}
#else
void gdx_pkt_dump(char *str, pNBuff_t pNBuff) {};
#endif

static bool gdx_is_dev_valid(struct net_device *dev_p)
{
    return (dev_p != NULL);
}

static inline bool gdx_is_devid_valid(uint16_t devid)
{
    return bcm_is_devid_valid(devid);
}

/****************************************************************************/
/** Name: gdx_get_dev_idx_from_gdx_dev_id                                  **/
/**                                                                        **/
/** Description: Finds the matching dev_idx for the gdx_dev_id.            **/
/**                                                                        **/
/** Input: gdx_dev_id       - matched gdx_dev_id                           **/
/**                                                                        **/
/** Output: dev_idx       - match dev_idx for gdx_dev_id                   **/
/****************************************************************************/
int gdx_get_dev_idx_from_gdx_dev_id(int gdx_dev_id)
{
    int dev_idx;

    GDX_PRINT_DBG("search gdx_dev_id<%d>", gdx_dev_id);
    dev_idx = bcm_get_idx_from_id(gdx_dev_id);
    if ((dev_idx != -1) && (gdx_gendev_info_pp[dev_idx] != NULL))
    {
        if (gdx_is_devid_valid(gdx_dev_id))
        {
            GDX_PRINT_DBG("match found: dev_idx<%d>", dev_idx);
            GDX_PRINT_DBG1("gendev[%d]<%s><0x%px> gdx_dev_id<%d>",
                  dev_idx,
                  bcm_get_netdev_name_by_id(gdx_dev_id), 
                  bcm_get_netdev_by_id_nohold(gdx_dev_id),
                  gdx_dev_id);
            return dev_idx;
        }
    }

    GDX_PRINT_DBG("no match found!!!");
    return GDX_FAILURE;
}

/****************************************************************************/
/**                                                                        **/
/** Name:                                                                  **/
/**                                                                        **/
/**   gdx_get_group_idx_and_dev_idx                                        **/
/**                                                                        **/
/** Title:                                                                 **/
/**                                                                        **/
/** Abstract:                                                              **/
/**                                                                        **/
/**   Finds the group_idx and dev_idx matching for the device.             **/
/**                                                                        **/
/** Input:                                                                 **/
/**   dev_p         - device to be matched                                 **/
/**   group_idx     - matched dev group                                    **/
/**   dev_idx       - matched device's dev_idx                             **/
/**                                                                        **/
/** Output:                                                                **/
/**                                                                        **/
/**                                                                        **/
/****************************************************************************/
inline int gdx_get_group_idx_and_dev_idx(struct net_device *dev_p, int *group_idx,
        int *dev_idx)
{
    int grp_idx;
    gdx_dev_group_t *dev_group_p;

    GDX_PRINT_DBG("search dev<%s><0x%px>", dev_p->name, dev_p);
    for (grp_idx = 0; grp_idx < GDX_MAX_DEV_GROUPS; grp_idx++)
    {
        dev_group_p = &gdx_dev_groups[grp_idx];

        GDX_PRINT_DBG1("group_idx<%d>", grp_idx);
        *dev_idx = gdx_get_dev_idx_from_gdx_dev_id(bcm_netdev_ext_field_get(dev_p, devid));
        if (*dev_idx != GDX_FAILURE)
        {
            *group_idx = grp_idx;
            GDX_PRINT_DBG("match found: group_idx<%d> dev_idx<%d>",
                    *group_idx, *dev_idx); 
            return GDX_SUCCESS;
        }
    }

    *group_idx = -1;
    *dev_idx = -1;
    GDX_PRINT_DBG("no match found !!!");
    return GDX_FAILURE;
}

/****************************************************************************/
/**                                                                        **/
/** Name:                                                                  **/
/**                                                                        **/
/**   gdx_get_dev_idx_from_dev                                             **/
/**                                                                        **/
/** Title:                                                                 **/
/**                                                                        **/
/** Abstract:                                                              **/
/**                                                                        **/
/**   Finds the matching dev_idx for the dev.                              **/
/**                                                                        **/
/** Input:                                                                 **/
/**   dev_group_p   - device group to be searched                          **/
/**   dev_p         - matched dev_p                                        **/
/**                                                                        **/
/** Output:                                                                **/
/**   dev_idx       - match dev_idx for dev_p                              **/
/**                                                                        **/
/****************************************************************************/
static inline int gdx_get_dev_idx_from_dev(gdx_dev_group_t *dev_group_p, struct net_device *dev_p)
{
    int dev_idx;

    GDX_PRINT_DBG("search group_idx<%d> dev<%s>", dev_group_p->group_idx, dev_p->name);
    dev_idx = gdx_get_dev_idx_from_gdx_dev_id(bcm_netdev_ext_field_get(dev_p, devid));
    if (dev_idx != GDX_FAILURE)
    {
        GDX_PRINT_DBG("match found: dev_idx<%d>", dev_idx);
        return dev_idx;
    }

    GDX_PRINT_DBG("no match found!!!");
    return GDX_FAILURE;
}

static inline void gdx_set_nbuff_dev(pNBuff_t nbuff, void *dev)
{
    if (IS_SKBUFF_PTR(nbuff))
    {
        struct sk_buff *skb = PNBUFF_2_SKBUFF(nbuff);
        skb->dev = dev;
    }
    /* fkb does not have a dev pointer. Do nothing for fkb */
}

static inline int gdx_exception_packet_handle(struct sk_buff *skb, gdx_hwacc_rx_info_t *info)
{
    skb->bcm_ext.gdx_loopbk = 1;
#if defined(CONFIG_BCM_CSO)        
    skb->ip_summed |= info->rx_csum_verified;
#endif

    GDX_PRINT_INFO("skb<0x%px>, dev<0x%px> gdx_loopbk<%d>",
            skb, skb->dev, skb->bcm_ext.gdx_loopbk);
    gdx_pkt_dump("exception_packet_handle: ", SKBUFF_2_PNBUFF(skb));

    local_bh_disable();
    netif_receive_skb(skb);
    local_bh_enable();

    GDX_PRINT_DBG1("Done.\n");
    return GDX_SUCCESS;
}

static inline void gdx_forward_packet_handle(gdx_dev_group_t *dev_group_p, gdx_hwacc_rx_info_t *info, pNBuff_t nbuff_p)
{
    gdx_dev_stats_t *dev_stats_p;
    gdx_gendev_info_t *gendev_info_p;
    gdx_pkt_dump("GDX_TX: ", nbuff_p);

    dev_group_p->cpu_rxq_tx_pkts++;
    GDX_PRINT_INFO("GDX_TX: nbuff<0x%px>, dev<0x%px: %s> dev_idx<%d>",
           nbuff_p, info->tx_dev, info->tx_dev->name, info->dev_idx);

    gendev_info_p = gdx_gendev_info_pp[info->dev_idx];
    dev_stats_p = &gendev_info_p->dev_stats;
    gdx_set_nbuff_dev(nbuff_p, info->tx_dev);

    gdx_pkt_dump("GDX_TX: ", nbuff_p);

    if (g_fill_info.prep_info.fc_args.use_xmit_args)
    {
        GDX_PRINT_INFO("%s is_ipv4 %d use_tcplocal_xmit %d tx_flags 0x%x\n", __func__, g_fill_info.prep_info.fc_args.tx_is_ipv4, 
                      g_fill_info.prep_info.fc_args.use_tcplocal_xmit_enq_fn, g_fill_info.prep_info.fc_args.tx_flags);
        gdx_dev_xmit(nbuff_p, info->tx_dev, &g_fill_info.prep_info.fc_args);
    }
    else
    {
        gdx_dev_xmit(nbuff_p, info->tx_dev, NULL);
    }
    dev_group_p->dev_tx_pkts++;
    dev_stats_p->dev_tx_pkts++;
    GDX_PRINT_DBG1("GDX_TX: Done. =========\n");
}

static inline void gdx_initialize_skb(struct sk_buff *skb, gdx_hwacc_rx_info_t *info, int prepend_size)
{
    if (likely(skb))
    {

        skb_headerinit(BCM_PKT_HEADROOM + info->data_offset + prepend_size,
#if defined(CC_NBUFF_FLUSH_OPTIMIZATION)
            SKB_DATA_ALIGN(info->size - prepend_size + BCM_SKB_TAILROOM + info->data_offset),
#else
            BCM_MAX_PKT_LEN - info->data_offset,
#endif
            skb, info->data + info->data_offset + prepend_size, gdx_hwacc_recycle, 0, NULL);

        skb_trim(skb, info->size - prepend_size);
        skb->recycle_flags &= SKB_NO_RECYCLE; /* no skb recycle,just do data recyle */

#if defined(CONFIG_BCM_CSO)        
        skb->ip_summed = info->rx_csum_verified;
#endif

#if defined(CC_NBUFF_FLUSH_OPTIMIZATION)
        skb_shinfo((struct sk_buff *)(skb))->dirty_p =
            skb->data + BCM_DCACHE_LINE_LEN;
#endif
    }
}

/****************************************************************************/
/******************* Other software units include files *********************/
/****************************************************************************/
#if (defined(CONFIG_BCM_RDPA)||defined(CONFIG_BCM_RDPA_MODULE))
#include "gdx_runner.h"
#endif

#if (defined(CONFIG_BCM_CROSSBOW))
#include "gdx_crossbow.h"
#endif
/****************************************************************************/
/**                                                                        **/
/** Name:                                                                  **/
/**                                                                        **/
/**   gdx_add_gendev                                                       **/
/**                                                                        **/
/** Title:                                                                 **/
/**                                                                        **/
/** Abstract:                                                              **/
/**                                                                        **/
/**   Makes sure the the new device is not already existing in the dev     **/
/**   group. Finds an empty slot for the device and then adds it.          **/
/**   Increments the device ref count                                      **/
/**                                                                        **/
/** Input:                                                                 **/
/**                                                                        **/
/** Output:                                                                **/
/**                                                                        **/
/**                                                                        **/
/****************************************************************************/
int gdx_add_gendev(int group_idx, struct net_device *dev_p)
{
    int dev_idx;
    gdx_gendev_info_t *gendev_info_p;
    gdx_dev_group_t *dev_group_p;
    uint16_t udevid;
    uint16_t allocation_size;

    if (dev_p == NULL)
    {
        GDX_PRINT_ERROR("ERROR. dev_p(0x%px)", dev_p);
        goto gdx_add_gendev_failure;
    }

    if (group_idx >= GDX_MAX_DEV_GROUPS)
    {
        GDX_PRINT_ERROR("Incorrect group_idx %d passed", group_idx);
        goto gdx_add_gendev_failure;
    }
    
    dev_group_p = &gdx_dev_groups[group_idx];

    if (dev_group_p->gendev_count >= GDX_GENDEV_MAX_NUM_DEV)
    {
        GDX_PRINT_ERROR("ERROR. GDX_GENDEV_MAX_NUM_DEV(%d) limit reached",
                GDX_GENDEV_MAX_NUM_DEV);
        goto gdx_add_gendev_failure;
    }

    dev_idx = bcm_get_idx_from_id(bcm_netdev_ext_field_get(dev_p, devid));
    if (dev_idx == GDX_FAILURE)
    {
        GDX_PRINT_ERROR("Error Invalid  @dev_idx %d", dev_idx);
        goto gdx_add_gendev_failure;
    }

    if (gdx_gendev_info_pp[dev_idx] != NULL)
    {
        GDX_PRINT_ERROR("Error already added @dev_idx %d", dev_idx);
        goto gdx_add_gendev_failure;
    }

    allocation_size = sizeof(gdx_gendev_info_t);
    gendev_info_p = (gdx_gendev_info_t *)kmalloc(allocation_size, GFP_KERNEL);
    if (gendev_info_p == NULL)
    {
        GDX_PRINT_ERROR("Error failed to allocate gendev_info_t");
        goto gdx_add_gendev_failure;
    }
    memset(gendev_info_p, 0, allocation_size);

    /* Caution!!! reach here only if a free entry was found */
    mutex_lock(&dev_group_p->group_lock);
    udevid = bcm_netdev_ext_field_get(dev_p, devid);
    if (bcm_get_netdev_by_id(udevid) == dev_p)
    {
        /* bcm_get_netdev_by_id will do a dev_hold */
        gendev_info_p->gdx_dev_id = udevid;
    }
    else
    {
        GDX_PRINT_ERROR("devid[%u] and netdev[%s] doesn't match\n",
                                            udevid,
                                            dev_p->name);
        /* bcm_put_netdev_by_id will do a dev_put */
        bcm_put_netdev_by_id(udevid);
        mutex_unlock(&dev_group_p->group_lock);
        kfree(gendev_info_p);
        goto gdx_add_gendev_failure;
    }
    gendev_info_p->gdx_group_id = group_idx;
    gdx_gendev_info_pp[dev_idx] = gendev_info_p;
    dev_group_p->gendev_count++;
    gdx_gendev_count++;
    if (gdx_gendev_count == 1)
        try_module_get(THIS_MODULE); /* increment module ref count */

    GDX_PRINT("\033[1m\033[34m GDX: group_idx %d: Dev %s added at dev_idx %d gdx_dev_id %d\033[0m",
        group_idx, dev_p->name, dev_idx, gendev_info_p->gdx_dev_id);

    mutex_unlock(&dev_group_p->group_lock);
    return (int)dev_idx;

gdx_add_gendev_failure:
    return GDX_FAILURE;
}

/****************************************************************************/
/**                                                                        **/
/** Name:                                                                  **/
/**                                                                        **/
/**   gdx_delete_gendev                                                    **/
/**                                                                        **/
/** Title:                                                                 **/
/**                                                                        **/
/** Abstract:                                                              **/
/**                                                                        **/
/**   Deletes an existing device from the dev group.                       **/
/**   Decrements the device ref count                                      **/
/**                                                                        **/
/** Input:                                                                 **/
/**                                                                        **/
/** Output:                                                                **/
/**                                                                        **/
/**                                                                        **/
/****************************************************************************/
int gdx_delete_gendev(struct net_device *dev_p)
{
    int group_idx;
    int dev_idx;
    gdx_gendev_info_t *gendev_info_p;
    gdx_dev_group_t *dev_group_p;

    if (dev_p == NULL)
    {
        GDX_PRINT_ERROR("dev<0x%px>", dev_p);
        return GDX_FAILURE;
    }

    if (gdx_get_group_idx_and_dev_idx(dev_p, &group_idx, &dev_idx) != GDX_SUCCESS)
    {
        GDX_PRINT_ERROR("dev not found <0x%px>", dev_p);
        return GDX_FAILURE;
    }

    dev_group_p = &gdx_dev_groups[group_idx];
    gendev_info_p = gdx_gendev_info_pp[dev_idx];

    GDX_PRINT("\033[1m\033[34m GDX: group_idx %d: Dev %s deleted @dev_idx %d\033[0m",
        group_idx, bcm_get_netdev_name_by_id(gendev_info_p->gdx_dev_id), dev_idx);

    mutex_lock(&dev_group_p->group_lock);
    /* bcm_put_netdev_by_id will do a dev_put */
    bcm_put_netdev_by_id(gendev_info_p->gdx_dev_id);
    kfree(gendev_info_p);
    gdx_gendev_info_pp[dev_idx] = NULL;
    dev_group_p->gendev_count--;
    gdx_gendev_count--;

    if (gdx_gendev_count == 0)
    {
        for (group_idx = 0; group_idx < GDX_MAX_DEV_GROUPS; group_idx++)
        {
            if (gdx_hwacc_flush_queues(group_idx) == GDX_FAILURE)
                goto gdx_delete_gendev_exit;
        }
        module_put(THIS_MODULE); /* decrement module ref count */
    }

gdx_delete_gendev_exit:
    mutex_unlock(&dev_group_p->group_lock);

    return dev_idx;
}

static inline FkBuff_t *gdx_fkb_alloc(gdx_hwacc_rx_info_t *info, int prepend_size)
{
    FkBuff_t *fkb_p;

    /* Convert descriptor to FkBuff */
    fkb_p = fkb_init(info->data, BCM_PKT_HEADROOM, 
                     info->data + info->data_offset + prepend_size, info->size - prepend_size);
#if defined(CC_NBUFF_FLUSH_OPTIMIZATION)
    fkb_p->dirty_p = NULL;
#endif
    fkb_p->recycle_hook = gdx_hwacc_recycle;
    fkb_p->recycle_context = 0;
#if defined(CONFIG_BCM_CSO)        
    fkb_p->rx_csum_verified = info->rx_csum_verified;
#endif
    return fkb_p;
}

/* Allocate a sk_buff kmem skbuff_head_cache */
static inline struct sk_buff *gdx_skb_alloc(gdx_hwacc_rx_info_t *info, int prepend_size)
{
    struct sk_buff *skb;

    skb = skb_header_alloc();
    gdx_initialize_skb(skb, info, prepend_size);
    return skb;
}

/****************************************************************************/
/** Name: gdx_add_dev_group                                                **/
/**                                                                        **/
/** Description: Adds a new device group. Initialize the dev group,        **/
/**   creates the CPU RX queues, allocates the HW status DMA memory.       **/
/**   A GDX thread is created to handle the packets from CPU_RX queue.     **/
/**   The GDX thread is per CPU RX port. Adds multiple dev group locks.    **/
/**                                                                        **/
/** Input: group_idx - index of new dev group to add                       **/
/**                                                                        **/
/** Output: Returns group_idx on success, GDX_FAILURE otherwise            **/
/****************************************************************************/
int gdx_add_dev_group(int group_idx)
{
    int rc = 0;
    int gdx_max_dev_groups;
    gdx_dev_group_t *dev_group_p;
    char gdx_dev_name[IFNAMSIZ]={0};

    gdx_max_dev_groups = GDX_MAX_DEV_GROUPS;
    if (gdx_dev_groups_count >= gdx_max_dev_groups)
    {
        GDX_PRINT_ERROR("ERROR. GDX_MAX_DEV_GROUPS(%d) limit reached", GDX_MAX_DEV_GROUPS);
        goto gdx_add_dev_group_failure;
    }

    dev_group_p = &gdx_dev_groups[group_idx];

    if (dev_group_p->is_valid)
    {
        GDX_PRINT_ERROR("ERROR. GDX_DEV_GROUP(%d) in Use", group_idx);
        goto gdx_add_dev_group_failure;
    }

    memset(dev_group_p, 0, sizeof(gdx_dev_group_t));
    dev_group_p->group_idx = group_idx;

    sprintf(gdx_dev_name,"gdx%d", group_idx);
    memcpy(dev_group_p->dev_name, gdx_dev_name, IFNAMSIZ);

    if ((rc = gdx_hwacc_dev_group_init(dev_group_p, gdx_dev_name)))
        goto gdx_add_dev_group_failure;

    dev_group_p->is_valid = 1;
    gdx_dev_groups_count++;
    return (int)group_idx;

gdx_add_dev_group_failure:
    return GDX_FAILURE;
}

/****************************************************************************/
/**                                                                        **/
/** Name:                                                                  **/
/**                                                                        **/
/**   gdx_delete_dev_group                                                 **/
/**                                                                        **/
/** Title:                                                                 **/
/**                                                                        **/
/** Abstract:                                                              **/
/**                                                                        **/
/**   Delete an existing device group. Just do reverse of gdx_add_dev_group**/
/**   A dev group will not be deleted if there exists a device bound to    **/
/**   dev group.                                                           **/
/**                                                                        **/
/** Input:                                                                 **/
/**   group_idx - index of dev group to delete                             **/
/**                                                                        **/
/** Output:                                                                **/
/**                                                                        **/
/**                                                                        **/
/****************************************************************************/
int gdx_delete_dev_group(int group_idx)
{
    int dev_idx;
    gdx_gendev_info_t *gendev_info_p;
    gdx_dev_group_t *dev_group_p;

    if ((group_idx < 0) || (group_idx >= GDX_MAX_DEV_GROUPS))
    {
        GDX_PRINT_ERROR("group_idx %d out of bounds %d", group_idx, GDX_MAX_DEV_GROUPS);
        return GDX_FAILURE;
    }

    if (gdx_dev_groups_count <= 0)
    {
        GDX_PRINT_ERROR("ERROR. No dev group created");
        return GDX_FAILURE;
    }
 
    dev_group_p = &gdx_dev_groups[group_idx];
    if (dev_group_p->is_valid == 0)
    {
        GDX_PRINT_ERROR("group_idx %d is not initialized", group_idx);
        return GDX_FAILURE;
    }

    /* If any valid device exist, then dev group cannot be deleted */
    for (dev_idx = 0; dev_idx < GDX_GENDEV_MAX_NUM_DEV; dev_idx++)
    {
        gendev_info_p = gdx_gendev_info_pp[dev_idx];
        if (gendev_info_p->gdx_group_id == group_idx)
        {
            GDX_PRINT_ERROR("one or more valid devices exist @dev_idx %d", dev_idx);
            return GDX_FAILURE;
        }
    }

    gdx_hwacc_dev_group_uninit(dev_group_p);

    memset(dev_group_p, 0, sizeof(gdx_dev_group_t));
    gdx_dev_groups_count--;
    return GDX_SUCCESS;
}

int gdx_gendev_get_blog_info(struct sk_buff *skb_p)
{
    int group_idx;
    int dev_idx;
    gdx_dev_group_t *dev_group_p;
    gdx_gendev_info_t *gendev_info_p;
    Blog_t *blog_p;
    struct net_device *dev_p;

    GDX_ASSERT((skb_p != NULL));

    if (skb_p == NULL)
    {
        GDX_PRINT_ERROR("NULL skb pointer");
        goto gdx_gendev_get_blog_info_error;
    }

    dev_p = skb_p->dev; 
    if (dev_p == NULL)
    {
        GDX_PRINT_ERROR("NULL dev pointer");
        goto gdx_gendev_get_blog_info_error;
    }

    blog_p = skb_p->blog_p;
    if (blog_p == NULL)
    {
        GDX_PRINT_ERROR("NULL blog_p pointer");
        goto gdx_gendev_get_blog_info_error;
    }

    GDX_PRINT_DBG("skb<0x%px> dev<%s> blog_p<0x%px>", skb_p, dev_p->name, blog_p);

    /* find the GDX gendev entry matching the dev */
    for (group_idx = 0; group_idx < GDX_MAX_DEV_GROUPS; group_idx++)
    {
        dev_group_p = &gdx_dev_groups[group_idx];
        if (dev_group_p->is_valid)
        {
            dev_idx = gdx_get_dev_idx_from_dev(dev_group_p, dev_p);
            if (dev_idx >= 0)
            {
                gendev_info_p = gdx_gendev_info_pp[dev_idx];
                if ((gendev_info_p != NULL) && (gendev_info_p->gdx_group_id == dev_group_p->group_idx))
                    goto gdx_gendev_get_blog_info_dev_found;
            }
        }
    }

    GDX_PRINT_ERROR("dev<0x%px> not found!!!", dev_p);

gdx_gendev_get_blog_info_error:
    return GDX_FAILURE;

gdx_gendev_get_blog_info_dev_found:
    GDX_PRINT_DBG("found match group_idx<%d> dev_idx<%d>", group_idx, dev_idx);

    blog_p->gdx.is_gdx_tx = 1;
    blog_p->gdx.gdx_idx = group_idx;
    blog_p->gdx.gdx_prio = 0;
    blog_p->gdx.gdx_ifid = gendev_info_p->gdx_dev_id;

    GDX_PRINT_DBG1("blog: gdx<0x%08x> is_gdx_tx<%d> gdx_idx<%d> gdx_prio<%d> gdx_ifid<%d>",
        blog_p->gdx.u32, blog_p->gdx.is_gdx_tx, blog_p->gdx.gdx_idx,
        blog_p->gdx.gdx_prio, blog_p->gdx.gdx_ifid);

    return GDX_SUCCESS;
}

int gdx_gendev_notifier(struct net_device *dev, int event, int group_idx)
{
    int dev_idx;
    gdx_dev_group_t *dev_group_p;
    int rc = GDX_FAILURE;

    GDX_PRINT_INFO("dev %s event %d group_idx %d", dev->name, event, group_idx);

    if (group_idx >= GDX_MAX_DEV_GROUPS)
    {
        GDX_PRINT_ERROR("Incorrect group_idx %d passed", group_idx);
        return GDX_FAILURE;
    }

    dev_group_p = &gdx_dev_groups[group_idx];

    switch (event)
    {
        case GDX_GENDEV_NOTIFY_EVENT_ADD:
        {
            dev_idx = gdx_add_gendev(group_idx, dev);

            if (dev_idx < 0) {
                GDX_PRINT_ERROR("Error in adding gendev %s to group_idx %d",
                        dev->name, group_idx);
                rc = GDX_FAILURE;
            } else {
                GDX_PRINT("GDX: gendev %s added @dev_idx %d to group_idx %d",
                        dev->name, dev_idx, group_idx);
                rc = GDX_SUCCESS;
            }

            break;
        }

        case GDX_GENDEV_NOTIFY_EVENT_DEL:
        {
            dev_idx = gdx_delete_gendev(dev);

            if (dev_idx < 0) {
                GDX_PRINT_ERROR("Error in deleting gendev %s from group_idx %d",
                        dev->name, group_idx);
                rc = GDX_FAILURE;
            } else {
                GDX_PRINT("GDX: gendev %s deleted @dev_idx %d from group_idx %d",
                        dev->name, dev_idx, group_idx);
                rc = GDX_SUCCESS;
            }

            break;
        }

        default:
            GDX_PRINT_ERROR("Unknown event: dev %s event %d group_idx %d",
                    dev->name, event, group_idx);
            rc = GDX_FAILURE;
    }

    return rc;
}

/****************************************************************************/
/**                                                                        **/
/** Name:                                                                  **/
/**                                                                        **/
/**   gdx_stats_file_show_proc                                             **/
/**                                                                        **/
/** Title:                                                                 **/
/**                                                                        **/
/**   gdx stats - proc file read handler                                   **/
/**                                                                        **/
/** Abstract:                                                              **/
/**                                                                        **/
/**   Procfs callback method.                                              **/
/**      Called when someone reads proc command                            **/
/**   using: cat /proc/gdx/stats                                           **/
/**                                                                        **/
/** Input:                                                                 **/
/**                                                                        **/
/**   m     -  Sequence file handle                                        **/
/**                                                                        **/
/** Output:                                                                **/
/**                                                                        **/
/**                                                                        **/
/****************************************************************************/
static int gdx_stats_file_show_proc(struct seq_file *m, void *v)
{
    int group_idx;
    unsigned int count_rx_queue_pkts_total=0;
    gdx_dev_group_t *dev_group_p;
    gdx_gendev_info_t *gendev_info_p;
    gdx_queue_stats_t *queue_stats_p;
    gdx_dev_stats_t *dev_stats_p;

    for (group_idx = 0; group_idx < GDX_MAX_DEV_GROUPS; group_idx++)
    {
        dev_group_p = &gdx_dev_groups[group_idx];
        if (dev_group_p->is_valid)
        {
            seq_printf(m, "\nGDX Dev Stats: Group %d", group_idx);
            seq_printf(m, "\n\ndev_rx_total_pkts         = %u", dev_group_p->dev_rx_total_pkts);
            seq_printf(m, "\ndev_rx_pkts               = %u", dev_group_p->dev_rx_pkts);
            seq_printf(m, "\ndev_rx_no_dev             = %u", dev_group_p->dev_rx_no_dev);
            seq_printf(m, "\ndev_rx_error              = %u", dev_group_p->dev_rx_error);
            seq_printf(m, "\ndev_tx_pkts               = %u", dev_group_p->dev_tx_pkts);

            {
                int dev_idx;

                for (dev_idx = 0; dev_idx < GDX_GENDEV_MAX_NUM_DEV; dev_idx++)
                {
                    gendev_info_p = gdx_gendev_info_pp[dev_idx];
                    if (gendev_info_p != NULL)
                    {
                        dev_stats_p = &gendev_info_p->dev_stats;

                        if ((!gdx_is_devid_valid(gendev_info_p->gdx_dev_id)) || (gendev_info_p->gdx_group_id != dev_group_p->group_idx))
                            continue;

                        seq_printf(m, "\n\nGDX Dev[%d]: %s", dev_idx, bcm_get_netdev_name_by_id(gendev_info_p->gdx_dev_id));
                        seq_printf(m, "\ndev_rx_pkts               = %u", dev_stats_p->dev_rx_pkts);
                        seq_printf(m, "\ndev_tx_pkts               = %u", dev_stats_p->dev_tx_pkts);

                        seq_printf(m, "\n\ncpu_rxq_lpbk_pkts         = %u", dev_stats_p->cpu_rxq_lpbk_pkts);
                        seq_printf(m, "\ncpu_rxq_lpbk_no_dev_pkts  = %u", dev_stats_p->cpu_rxq_lpbk_no_dev_pkts);
                        seq_printf(m, "\n");
                    }
                }
            }

            seq_printf(m,"\n\nGDX CPU_RXQ Stats: Group %d", group_idx);
            seq_printf(m, "\n\ncpu_rxq_total_pkts        = %u", dev_group_p->cpu_rxq_total_pkts);
            seq_printf(m, "\ncpu_rxq_valid_pkts        = %u", dev_group_p->cpu_rxq_valid_pkts);
            seq_printf(m, "\ncpu_rxq_lpbk_pkts         = %u", dev_group_p->cpu_rxq_lpbk_pkts);
            seq_printf(m, "\ncpu_rxq_lpbk_no_dev       = %u", dev_group_p->cpu_rxq_lpbk_no_dev);
            seq_printf(m, "\ncpu_rxq_tx_pkts           = %u", dev_group_p->cpu_rxq_tx_pkts);
            seq_printf(m, "\ncpu_rxq_tx_no_dev         = %u", dev_group_p->cpu_rxq_tx_no_dev);

            {
               /* per queue status */          
                unsigned int tmp_mask = 0x00;
                unsigned int qidx = 0x00;

                tmp_mask = GDX_GET_QUEUE_MASK;
                
                for (qidx =0x00; tmp_mask!=0x00; qidx++)
                {
                    queue_stats_p = GDX_GET_QUEUE_STATS_PTR(qidx);
                    if (tmp_mask & (1<<qidx))
                    {
                        seq_printf(m, "\n\nQueue[%d]:", qidx);
                        seq_printf(m, "\ncpu_rxq_rx_pkts           = %u", queue_stats_p->cpu_rxq_rx_pkts);
                        seq_printf(m, "\ncpu_rxq_max_rx_pkts       = %u", queue_stats_p->cpu_rxq_max_rx_pkts);
                        count_rx_queue_pkts_total += queue_stats_p->cpu_rxq_rx_pkts;
                        seq_printf(m, "\ncpu_rxq_rx_no_dev         = %u", queue_stats_p->cpu_rxq_rx_no_dev);
                        seq_printf(m, "\ncpu_rxq_no_skb_error      = %u", queue_stats_p->cpu_rxq_no_skbs);
                    }

                    tmp_mask &= ~(1<<qidx);
                }
            }
            seq_printf(m, "\nRX from GDX queues [SUM]  = %u\n", count_rx_queue_pkts_total);

            seq_printf(m, "\n===========\n");

        }
    }

    return 0;
}

/****************************************************************************/
/**                                                                        **/
/** Name:                                                                  **/
/**                                                                        **/
/**   gdx_dev_file_show_proc                                               **/
/**                                                                        **/
/** Title:                                                                 **/
/**                                                                        **/
/**   gdx stats - proc file read handler                                   **/
/**                                                                        **/
/** Abstract:                                                              **/
/**                                                                        **/
/**   Procfs callback method.                                              **/
/**      Called when someone reads proc command                            **/
/**   using: cat /proc/gdx/stats                                           **/
/**                                                                        **/
/** Input:                                                                 **/
/**                                                                        **/
/**   m     -  Sequence file handle                                        **/
/**                                                                        **/
/** Output:                                                                **/
/**                                                                        **/
/**                                                                        **/
/****************************************************************************/
static int gdx_dev_file_show_proc(struct seq_file *m, void *v)
{
    int group_idx;
    int dev_idx;
    gdx_dev_group_t *dev_group_p;
    gdx_gendev_info_t *gendev_info_p;

    for (group_idx = 0; group_idx < GDX_MAX_DEV_GROUPS; group_idx++)
    {
        dev_group_p = &gdx_dev_groups[group_idx];
        if (dev_group_p->is_valid)
        {
            seq_printf(m, "\nGDX Dev Group: %d", group_idx);
            seq_printf(m, "\ngroup_idx        = %d", dev_group_p->group_idx);
            seq_printf(m, "\nqueue_mask       = 0x%08x", GDX_GET_QUEUE_MASK);
            seq_printf(m, "\ngendev_count     = %d", dev_group_p->gendev_count);

            for (dev_idx = 0; dev_idx < GDX_GENDEV_MAX_NUM_DEV; dev_idx++)
            {
                gendev_info_p = gdx_gendev_info_pp[dev_idx];

                if (gendev_info_p != NULL)
                {
                    if ((!gdx_is_devid_valid(gendev_info_p->gdx_dev_id)) || (gendev_info_p->gdx_group_id != dev_group_p->group_idx))
                        continue;

                    seq_printf(m, "\n\nGDX Dev[%d]: %s", dev_idx, bcm_get_netdev_name_by_id(gendev_info_p->gdx_dev_id));
                    seq_printf(m, "\ngdx_dev_id          = %d", gendev_info_p->gdx_dev_id);
                    seq_printf(m, "\n");
                }
            }
         
            seq_printf(m, "\n===========\n");
        }
    }

    return 0;
}


/****************************************************************************/
/**                                                                        **/
/** Name:                                                                  **/
/**                                                                        **/
/**   gdx_proc_init.                                                       **/
/**                                                                        **/
/** Title:                                                                 **/
/**                                                                        **/
/**   gdx mw - proc init                                                   **/
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
static int gdx_proc_init(void)
{
    if (!(proc_gdx_dir = proc_mkdir("gdx", NULL))) 
        goto fail;

    if (!proc_create_single("dev", 0644, proc_gdx_dir, gdx_dev_file_show_proc))
        goto fail;
             
    if (!proc_create_single("stats", 0644, proc_gdx_dir, gdx_stats_file_show_proc))
        goto fail;
        
    if (gdx_hwacc_proc_init()) /* platform specific procs */
        goto fail;
 
    return 0;

fail:
    GDX_PRINT_ERROR("Failed to create proc /gdx");
    remove_proc_entry("gdx", NULL);
    return (GDX_FAILURE);
}

static inline void gdx_proc_uninit(void)
{
    remove_proc_entry("stats", proc_gdx_dir);
    remove_proc_entry("dev", proc_gdx_dir);
    gdx_hwacc_proc_uninit(proc_gdx_dir); /* remove platform-specific procs */
    remove_proc_entry("gdx", NULL);
}

extern int (*gdx_hw_accel_loopbk_fn)(struct sk_buff *skb, bool l3_packet);
extern int (*bcm_netdev_gen_hwaccel_notfier_cb)(struct net_device *dev,
       int event, int group);
extern int (*gdx_get_blog_info_fn)(struct sk_buff *skb);

/****************************************************************************/
/**                                                                        **/
/** Name:                                                                  **/
/**                                                                        **/
/**   gdx_bind                                                             **/
/**                                                                        **/
/** Title:                                                                 **/
/**                                                                        **/
/**   gdx bind the hooks                                                   **/
/**                                                                        **/
/** Abstract:                                                              **/
/**                                                                        **/
/**   The function binds to gendev hooks in Linux kernel                   **/
/**                                                                        **/
/** Input:                                                                 **/
/**                                                                        **/
/** Output:                                                                **/
/**                                                                        **/
/**                                                                        **/
/****************************************************************************/
static void gdx_bind(void)
{
    gdx_hw_accel_loopbk_fn = gdx_hwacc_tx;
    bcm_netdev_gen_hwaccel_notfier_cb = gdx_gendev_notifier;
    gdx_get_blog_info_fn = gdx_gendev_get_blog_info;
    gdx_hwacc_bind();
    GDX_PRINT("GDX: Successfully bound the hooks");    
}

/****************************************************************************/
/**                                                                        **/
/** Name:                                                                  **/
/**                                                                        **/
/**   gdx_unbind                                                           **/
/**                                                                        **/
/** Title:                                                                 **/
/**                                                                        **/
/**   gdx unbind the hooks                                                 **/
/**                                                                        **/
/** Abstract:                                                              **/
/**                                                                        **/
/**   The function unbinds to gendev hooks in Linux kernel                 **/
/**                                                                        **/
/** Input:                                                                 **/
/**                                                                        **/
/** Output:                                                                **/
/**                                                                        **/
/**                                                                        **/
/****************************************************************************/
static void gdx_unbind(void)
{
    gdx_hw_accel_loopbk_fn = NULL;
    bcm_netdev_gen_hwaccel_notfier_cb = NULL;
    gdx_get_blog_info_fn = NULL;
    gdx_hwacc_unbind();
    GDX_PRINT("GDX: Successfully unbound the hooks");    
}

static int gdx_netdev_notifier_cb(struct notifier_block *this,
                            unsigned long event, void *dev_ptr);

static struct notifier_block gdx_netdev_notifier = 
{
    .notifier_call = gdx_netdev_notifier_cb,
};

/****************************************************************************/
/**                                                                        **/
/** Name:                                                                  **/
/**                                                                        **/
/**   gdx_netdev_unregister                                                **/
/**                                                                        **/
/** Title:                                                                 **/
/**                                                                        **/
/**   gdx net device unregisgter                                           **/
/**                                                                        **/
/** Abstract:                                                              **/
/**                                                                        **/
/**   The function is called whenener a net_device unregister change is    **/
/**   detected                                                             **/
/**                                                                        **/
/** Input:                                                                 **/
/**   dev   - the net devivce getting unregistered                         **/
/**                                                                        **/
/** Output:                                                                **/
/**                                                                        **/
/****************************************************************************/
static void gdx_netdev_unregister(struct net_device *dev_p)
{
    int group_idx;
    int dev_idx;
    gdx_dev_group_t *dev_group_p;
    gdx_gendev_info_t *gendev_info_p;

    if (dev_p == NULL)
    {
        GDX_PRINT_ERROR("%s:%d : Null Netdev pointer\n", __func__, __LINE__);
        return;
    }

    /* find the device and dev group */
    for (group_idx = 0; group_idx < GDX_MAX_DEV_GROUPS; group_idx++)
    {
        dev_group_p = &gdx_dev_groups[group_idx];

        if (dev_group_p->is_valid)
        {
            for (dev_idx = 0; dev_idx < GDX_GENDEV_MAX_NUM_DEV; dev_idx++)
            {
                gendev_info_p = gdx_gendev_info_pp[dev_idx];

                if (gendev_info_p != NULL)
                {
                    /* matching device */
                    if (gdx_is_devid_valid(gendev_info_p->gdx_dev_id) && (gendev_info_p->gdx_dev_id == bcm_netdev_ext_field_get(dev_p, devid)))
                        goto gdx_netdev_unregister_dev_found;
                }
            }
            continue;

gdx_netdev_unregister_dev_found:
            dev_idx = gdx_delete_gendev(dev_p);
            if (dev_idx < 0) {
                GDX_PRINT_ERROR("Error in deleting gendev %s from group_idx %d",
                        dev_p->name, group_idx);
            } else {
                GDX_PRINT("GDX: gendev %s deleted @dev_idx %d from group_idx %d",
                        dev_p->name, dev_idx, group_idx);
            }

            return; /* only one device at a time */
        }
    }
    return;
}

/****************************************************************************/
/**                                                                        **/
/** Name:                                                                  **/
/**                                                                        **/
/**   gdx_netdev_notifier_cb                                               **/
/**                                                                        **/
/** Title:                                                                 **/
/**                                                                        **/
/**   gdx net device notifier call back                                    **/
/**                                                                        **/
/** Abstract:                                                              **/
/**                                                                        **/
/**   The function is called whenener a net_device change is detected      **/
/**                                                                        **/
/** Input:                                                                 **/
/**   event - the type of net_device event detected                        **/
/**   dev   - the net devivce for which net_device event detected          **/
/**                                                                        **/
/** Output:                                                                **/
/**                                                                        **/
/****************************************************************************/
int gdx_netdev_notifier_cb(struct notifier_block *this,
                            unsigned long event, void *dev)
{
    struct net_device *dev_p = NETDEV_NOTIFIER_GET_DEV(dev);

    GDX_PRINT_DBG("dev<%s> dev_p<%px> event<%lu>\n", 
            ((struct net_device *) dev_p)->name, dev_p, event);

    switch (event) {
        case NETDEV_UNREGISTER:
            gdx_netdev_unregister(dev_p);
            break;

        default:
            break;
    }

    return NOTIFY_DONE;
}

/****************************************************************************/
/**                                                                        **/
/** Name:                                                                  **/
/**                                                                        **/
/**   gdx_hw_uninit                                                        **/
/**                                                                        **/
/** Title:                                                                 **/
/**                                                                        **/
/**   gdx accelerator - close                                              **/
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
void gdx_hw_uninit(void)
{
    int group_idx;
    gdx_dev_group_t *dev_group_p;

    /* Disable the interrupt */
    for (group_idx = 0; group_idx < GDX_MAX_DEV_GROUPS; group_idx++)
    {
        dev_group_p = &gdx_dev_groups[group_idx];

        if (dev_group_p->is_valid)
        {
            /* Has all the devices been deleted */
            if (dev_group_p->gendev_count)
                goto gdx_dev_uninit_err;

            if (gdx_delete_dev_group(group_idx) == GDX_FAILURE)
                goto gdx_dev_uninit_err;
        }
    }

    gdx_unbind();
    unregister_netdevice_notifier(&gdx_netdev_notifier);
      
    gdx_proc_uninit();
    return;

gdx_dev_uninit_err:
    return;
}

/****************************************************************************/
/**                                                                        **/
/** Name:                                                                  **/
/**                                                                        **/
/**   gdx_hw_init                                                          **/
/**                                                                        **/
/** Title:                                                                 **/
/**                                                                        **/
/**   Generic device accelerator - init                                    **/
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
int gdx_hw_init(void)
{
    int grp_idx;
    gdx_dev_group_t *dev_group_p;
    gdx_gendev_info_t **gendev_info_ptr;
    uint16_t allocation_size;

    /* Initialize GDX dev group */
    memset((uint8_t *)gdx_dev_groups, 0, sizeof(gdx_dev_groups));

    /* Initialize the proc interface for debugging information */
    if (gdx_proc_init()!=0)
    {
        GDX_PRINT_ERROR("gdx_proc_init() failed");
        goto proc_release;
    }

    /* Initialize gendev_info_ptr array */
    allocation_size = (sizeof(gdx_gendev_info_t *) * GDX_GENDEV_MAX_NUM_DEV);
    gendev_info_ptr = (gdx_gendev_info_t **)kmalloc(allocation_size, GFP_KERNEL);
    if (gendev_info_ptr == NULL)
    {
        GDX_PRINT_ERROR("Failed to allocate memory for gendev_info_t pointer array");
        goto proc_release;
    }
    memset(gendev_info_ptr, 0, allocation_size);
    gdx_gendev_info_pp = gendev_info_ptr;

    for (grp_idx = 0; grp_idx < GDX_MAX_DEV_GROUPS; grp_idx++)
    {
        dev_group_p = &gdx_dev_groups[grp_idx];

        GDX_PRINT_DBG1("group_idx<%d>", grp_idx);
        if (dev_group_p->is_valid == 0)
        {
            if (gdx_add_dev_group(grp_idx) != GDX_SUCCESS)
            {
                GDX_PRINT_ERROR("Error in adding group_idx %d", grp_idx);
                goto gendev_release;
            }
        }
    }

    gdx_bind();

    register_netdevice_notifier(&gdx_netdev_notifier);

    GDX_PRINT("\033[1m\033[34m GDX: gdx_print_lvl<0x%px>=%d\033[0m",
            &gdx_print_lvl, gdx_print_lvl);

    GDX_PRINT("\033[1m\033[34m GDX: %s is initialized!\033[0m", version);

    return 0;
gendev_release:
    kfree(gdx_gendev_info_pp);
proc_release:
    gdx_proc_uninit();

    return GDX_FAILURE;
}

