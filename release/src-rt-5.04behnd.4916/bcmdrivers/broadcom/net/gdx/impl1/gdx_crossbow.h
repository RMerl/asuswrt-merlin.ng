
/*
<:copyright-BRCM:2023:DUAL/GPL:standard

   Copyright (c) 2023 Broadcom 
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

#ifndef __GDX_CROSSBOW__H__
#define __GDX_CROSSBOW__H__

#define GDX_NUM_QUEUES_PER_GDX_INST 1
#define GDX_NUM_QUEUE_SUPPORTED (GDX_NUM_QUEUES_PER_GDX_INST)

typedef struct {
    uint32_t initialized;
    uint32_t rx_skb_count_flow_miss;
    uint32_t rx_skb_count_acq_drop;
    uint32_t rx_skb_count_tx;
    uint32_t fwq_count_pkt;
    uint32_t queue_mask;
    gdx_queue_stats_t queue_stats[GDX_NUM_QUEUES_PER_GDX_INST];
    HOOKP hwacc_send_func;   
} gdx_xbow_priv_info_t;

gdx_xbow_priv_info_t gdx_hwacc_priv = {.initialized = 0};

#define dest_ifid             gdx_pd_data
#define GDX_GET_QUEUE_STAT(qid,stat_name)    gdx_hwacc_priv.queue_stats[qid].stat_name
#define GDX_INCR_QUEUE_STATS(qid,stat_name)  gdx_hwacc_priv.queue_stats[qid].stat_name++
#define GDX_GET_QUEUE_STATS_PTR(qid)         &gdx_hwacc_priv.queue_stats[qid]
#define GDX_GET_QUEUE_MASK                   gdx_hwacc_priv.queue_mask
static inline void gdx_get_pkt_info_from_nbuff(pNBuff_t pNBuff, gdx_hwacc_rx_info_t *pinfo)
{
    struct sk_buff *skb;
    FkBuff_t *fkb;
    if (IS_SKBUFF_PTR(pNBuff))
    {
        skb = PNBUFF_2_SKBUFF(pNBuff);
        pinfo->data = skb->data;
        pinfo->size = skb->len;  /* Needs to check the length here */
        pinfo->rx_csum_verified = (skb->ip_summed == CHECKSUM_NONE)?0:1;
    }
    else
    {
        fkb = PNBUFF_2_FKBUFF(pNBuff);
        pinfo->data = fkb->data;
        pinfo->size = fkb->len;  /* Needs to check the length here */
        pinfo->rx_csum_verified = fkb->rx_csum_verified;
    }
}

static int gdx_miss_packet_handler(void *void_p)
{
    pNBuff_t pNBuff;
    int group_idx;
    int dev_idx;
    gdx_dev_group_t *dev_group_p;
    gdx_dev_stats_t *dev_stats_p;
    gdx_hwacc_rx_info_t info = {};
    struct sk_buff *skb = NULL;
    pNBuff = (pNBuff_t)void_p;
    /* Loopback packet, here pNBuff has to be SKB */
    GDX_ASSERT(IS_SKBUFF_PTR(pNBuff));
    gdx_get_pkt_info_from_nbuff(pNBuff, &info);
    skb = PNBUFF_2_SKBUFF(pNBuff);
    GDX_PRINT_DBG("dev<%s> skb<0x%px>", skb->dev->name, skb);
    if (gdx_get_group_idx_and_dev_idx(skb->dev, &group_idx, &dev_idx) != GDX_SUCCESS)
    {
        /* Need to recheck how we get the group_idx/dev_idx here for status
         * updates */
        GDX_PRINT_ERROR("no matching GDX intf skb<0x%px> dev<%s>", skb, skb->dev->name);
        dev_kfree_skb_thread(skb);
        return GDX_FAILURE;
    }
    dev_group_p = &gdx_dev_groups[group_idx];
    dev_stats_p = &(gdx_gendev_info_pp[dev_idx]->dev_stats);

    /* Mark the packet as exception packet */
    info.is_exception = 1;
    dev_group_p->cpu_rxq_lpbk_pkts++;
    dev_stats_p->cpu_rxq_lpbk_pkts++;
    /* Packet is sent to linux for further processing */
    gdx_exception_packet_handle(skb, &info);
    return GDX_SUCCESS;
}

static int gdx_forward_packet_handler(void *void_p, uint32_t user_value)
{
    gdx_hwacc_rx_info_t info = {};
    gdx_dev_group_t *dev_group_p;
    gdx_dev_stats_t *dev_stats_p;
    bcmFun_t *gdx_prepend_fill_info_fn = bcmFun_get(BCM_FUN_ID_PREPEND_FILL_INFO_FROM_BUF);
    pNBuff_t pNBuff = (pNBuff_t)void_p;
    uint32_t prepend_size = 0;
    FkBuff_t *fkb = NULL;
    struct sk_buff *skb = NULL;
    pNBuff_t pNBuff_tx = NULL;

    GDX_ASSERT((pNBuff != NULL));
    GDX_INCR_QUEUE_STATS(0, cpu_rxq_rx_pkts);
    /* Here group_idx is always 0 */
    dev_group_p = &gdx_dev_groups[0];
    gdx_get_pkt_info_from_nbuff(pNBuff, &info);
    info.gdx_pd_data  = (int)user_value;
    GDX_PRINT_INFO("gdx_pd_data:%d size:%d\n",info.gdx_pd_data, info.size);
    info.dev_idx = gdx_get_dev_idx_from_gdx_dev_id(info.gdx_pd_data);
    if (info.dev_idx < 0)
    {
        GDX_PRINT_ERROR("dev_idx is not valid \n");
        goto gdx_fwd_pkt_err_handle;
    }

    dev_stats_p = &(gdx_gendev_info_pp[info.dev_idx]->dev_stats);
    info.tx_dev = bcm_get_netdev_by_id_nohold(info.gdx_pd_data);
    if (!gdx_is_dev_valid(info.tx_dev))
    {
        GDX_PRINT_ERROR("tx_dev is not valid \n");
        goto gdx_fwd_pkt_err_handle;
    }
    GDX_PRINT_INFO("TX DEV:%s",bcm_get_netdev_name_by_id(info.gdx_pd_data));

    if (gdx_prepend_fill_info_fn != NULL)
    {
        g_fill_info.prepend_data = info.data + info.data_offset;
        prepend_size = gdx_prepend_fill_info_fn(&g_fill_info);
        if (prepend_size == (uint32_t)-1)
        {
            GDX_PRINT_ERROR("prepend_size is invalid \n");
            goto gdx_fwd_pkt_err_handle;
        }
    }
    else
    {
        GDX_PRINT_ERROR("gdx_prepend_fill_info_fn NULL\n");
        goto gdx_fwd_pkt_err_handle;
    }

    GDX_PRINT_INFO("prepend_size:%u",prepend_size);

    if (is_netdev_accel_tx_fkb(info.tx_dev))
    {
        GDX_PRINT_DBG("TX DEV supports FKB");
        /* For both fkb/skb the following will initialize the fkb accordingly */
        if (IS_FKBUFF_PTR(pNBuff))
        {
            fkb = PNBUFF_2_FKBUFF(pNBuff);
            if (unlikely(!fkb))
            {
                goto gdx_fwd_pkt_initialize_err;
            }
            fkb_pull(fkb, (info.data_offset + prepend_size));
            pNBuff_tx = FKBUFF_2_PNBUFF(fkb);
        }
        else
        {
            skb = PNBUFF_2_SKBUFF(pNBuff);
            if (unlikely(!skb))
            {
                goto gdx_fwd_pkt_initialize_err;
            }
            skb_pull(skb, (info.data_offset + prepend_size));
            pNBuff_tx = SKBUFF_2_PNBUFF(skb);
        }
    }
    else
    {
        GDX_PRINT_INFO("TX DEV supports SKB");
        if (IS_FKBUFF_PTR(pNBuff))
        {
            /* Convert FKB to SKB */
            GDX_PRINT_INFO("Converting FKB to SKB");
            fkb = PNBUFF_2_FKBUFF(pNBuff);
            fkb_pull(fkb, (info.data_offset + prepend_size));
            skb = fkb_xlate(fkb);
            if (unlikely(!skb))
            {
                goto gdx_fwd_pkt_initialize_err;
            }
        }
        else
        {
            GDX_PRINT_INFO("Its SKB already just need to reinitialize again");
            skb = PNBUFF_2_SKBUFF(pNBuff);
            skb_pull(skb, (info.data_offset + prepend_size));
        }
        pNBuff_tx = SKBUFF_2_PNBUFF(skb);
    }
    GDX_PRINT_INFO("mark %lu priority %d", g_fill_info.prep_info.mark, g_fill_info.prep_info.priority);
    nbuff_set_mark(pNBuff_tx, g_fill_info.prep_info.mark);
    nbuff_set_priority(pNBuff_tx, g_fill_info.prep_info.priority);
    /* forward packets to gdx TX device. */
    gdx_forward_packet_handle(dev_group_p, &info, pNBuff_tx);
    return GDX_SUCCESS;
gdx_fwd_pkt_initialize_err:
    GDX_INCR_QUEUE_STATS(0, cpu_rxq_no_skbs);
gdx_fwd_pkt_err_handle:
    GDX_INCR_QUEUE_STATS(0, cpu_rxq_rx_no_dev);
    dev_group_p->cpu_rxq_tx_no_dev++;
    nbuff_free(pNBuff);
    return GDX_FAILURE;
}

static inline int _gdx_hwacc_crossbow_tx(struct sk_buff *skb, bool l3_packet)
{
    int group_idx;
    int dev_idx;
    gdx_dev_group_t *dev_group_p;
    gdx_dev_stats_t *dev_stats_p;
    pNBuff_t pNBuff;
    if (skb == NULL)
    {
        GDX_PRINT_ERROR("NULL SKB pointer\n");
        return GDX_FAILURE;
    }
    GDX_PRINT_DBG("dev<%s> skb<0x%px>", skb->dev->name, skb);
    if (gdx_get_group_idx_and_dev_idx(skb->dev, &group_idx, &dev_idx) != GDX_SUCCESS)
    {
        dev_group_p = &gdx_dev_groups[group_idx];
        dev_group_p->dev_rx_no_dev++;
        GDX_PRINT_ERROR("no matching GDX intf skb<0x%px> dev<%s>", skb, skb->dev->name);
        return GDX_FAILURE;
    }

    dev_group_p = &gdx_dev_groups[group_idx];
    dev_stats_p = &(gdx_gendev_info_pp[dev_idx]->dev_stats);
    pNBuff = SKBUFF_2_PNBUFF(skb);

    dev_group_p->dev_rx_total_pkts++;

    dev_stats_p->dev_rx_pkts++;
    GDX_PRINT_INFO("CPU_TX: l3_packet<%d>, dev<%s> skb<0x%px>", 
                            l3_packet, skb->dev->name, skb);
    gdx_pkt_dump("gdx_crossbow_tx: ", pNBuff);
    if (gdx_hwacc_priv.hwacc_send_func(pNBuff))
    {
        /* packet dropped, no space in ACQ 
         * update drop stats here for this device
         * drop the packet */
        dev_group_p->dev_rx_error++;
        GDX_PRINT_DBG("CPU_TX: error");
        nbuff_free(pNBuff);
    }

    /* Packet taken by crossbow for further processing 
     * Update stats here */
    gdx_hwacc_priv.rx_skb_count_tx++;
    return GDX_SUCCESS;
}

static int _gdx_hwacc_crossbow_init(int group_idx, const char *gdx_dev_name)
{
    gdx_acc_bind_arg_t bind_arg;
    int qidx;
    int ret = GDX_FAILURE;
    bcmFun_t *gdx_hwacc_bind_func = bcmFun_get(BCM_FUN_ID_HWACC_GDX_BIND);

    if (group_idx > GDX_MAX_DEV_GROUPS)
    {
        GDX_PRINT_ERROR("Invalid group_idx:%d\n",group_idx);
        return GDX_FAILURE; 
    }

    if (!gdx_hwacc_priv.initialized)
    {
        if (gdx_hwacc_bind_func)
        {
            bind_arg.gdx_miss_pkt_handler_cb = gdx_miss_packet_handler;
            bind_arg.gdx_hit_pkt_handler_cb = gdx_forward_packet_handler;
            bind_arg.gdx_acc_send_pkt = NULL;
            bind_arg.initialized      = 0;
            ret = gdx_hwacc_bind_func(&bind_arg);
        }
        if ((ret != GDX_SUCCESS) || (bind_arg.initialized == 0))
        {
            GDX_PRINT_ERROR("Failed to attach to GDX Crossbow socket ", group_idx);
            return GDX_FAILURE;
        }
        else
        {
            gdx_hwacc_priv.hwacc_send_func = bind_arg.gdx_acc_send_pkt;
            for (qidx = 0; qidx < GDX_NUM_QUEUES_PER_GDX_INST; qidx++)
                gdx_hwacc_priv.queue_mask   |= (1 << qidx);
        }
        gdx_hwacc_priv.initialized = 1;
    }

    GDX_PRINT_INFO("Done connecting GDX with Crossbow");
    return GDX_SUCCESS;
}

static void _gdx_hwacc_crossbow_uninit(int group_idx)
{
    gdx_acc_bind_arg_t bind_arg;
    int ret = GDX_FAILURE;
    bcmFun_t *gdx_hwacc_unbind_func = bcmFun_get(BCM_FUN_ID_HWACC_GDX_UNBIND);

    if (gdx_hwacc_unbind_func)
    {
        ret = gdx_hwacc_unbind_func(&bind_arg);
        gdx_hwacc_priv.hwacc_send_func = bind_arg.gdx_acc_send_pkt;
        gdx_hwacc_priv.queue_mask      = 0;
    }

    if (ret)
        GDX_PRINT_ERROR("Error detaching from socket(Invalid socket)");
    
}

/* ****** Mandatory Functions - Continued *******
 * These are the functions that a hardware accelerator MUST
 * define in order to integrate with GDX*/

static inline int gdx_hwacc_flush_queues(int group_idx)
{
   /* Fixme: Need to implement flushing of FWQ, STQ, ACQ of XBow */
   return GDX_SUCCESS;
}

static inline int gdx_hwacc_proc_init(void)
{
   /* Fixme: Need to implement displaying any debug stats here */
   return GDX_SUCCESS;
}

static inline void gdx_hwacc_proc_uninit(struct proc_dir_entry *dir)
{

}

/****************************************************************************/
/** Name: gdx_hwacc_dev_group_init                                         **/
/**                                                                        **/
/** Description: Hwaccelerator specific device group initialization        **/
/**                                                                        **/
/** Input: dev_group_p - device group pointer                              **/
/**        gdx_dev_name - GDX device name                                  **/
/**                                                                        **/
/** Output: GDX_SUCCESS - if flushing and freeing was done                 **/
/**         GDX_FAILURE - if there was some issue in flushing and freeing  **/
/****************************************************************************/
static inline int gdx_hwacc_dev_group_init(gdx_dev_group_t *dev_group_p, 
                                           const char *gdx_dev_name)
{
    int rc;
    int group_idx = dev_group_p->group_idx;

    if ((rc = _gdx_hwacc_crossbow_init(group_idx, gdx_dev_name)) != GDX_SUCCESS)
        return rc;


    GDX_PRINT("\033[1m\033[34m GDX: group_idx %d"
            " configured GDX<-->Crossbow",
            group_idx);

    return GDX_SUCCESS;

}

/****************************************************************************/
/** Name: gdx_hwacc_dev_group_uninit                                       **/
/**                                                                        **/
/** Description: Hwaccelerator specific device group uninitialization      **/
/**                                                                        **/
/** Input: dev_group_p - device group pointer                              **/
/**                                                                        **/
/** Output: None                                                           **/
/****************************************************************************/
static inline void gdx_hwacc_dev_group_uninit(gdx_dev_group_t *dev_group_p)
{
    _gdx_hwacc_crossbow_uninit(dev_group_p->group_idx);
}

/****************************************************************************/
/** Name: gdx_hwacc_tx                                                     **/
/**                                                                        **/
/** Description: This is the hook to be called by Linux when sending to    **/
/**              Runner Loopback                                           **/
/**                                                                        **/
/** Input: skb - skb to be transmitted                                     **/
/**              l3_packet - flag that indicates if it is a l3 packet      **/
/**                                                                        **/
/** Output:                                                                **/
/****************************************************************************/
int gdx_hwacc_tx(struct sk_buff *skb, bool l3_packet)
{
    gdx_pkt_dump("gdx_accelerator_tx: ", SKBUFF_2_PNBUFF(skb));
    return _gdx_hwacc_crossbow_tx(skb, l3_packet);
}

/****************************************************************************/
/** Name: gdx_hwacc_bind                                                   **/
/**                                                                        **/
/** Description: Bind any hardware accelerator specific callbacks          **/
/**                                                                        **/
/** Input: None                                                            **/
/**                                                                        **/
/** Output: None                                                           **/
/****************************************************************************/
static inline void gdx_hwacc_bind(void)
{

}

/****************************************************************************/
/** Name: gdx_hwacc_bind                                                   **/
/**                                                                        **/
/** Description: Unbind any hardware accelerator specific callbacks        **/
/**                                                                        **/
/** Input: None                                                            **/
/**                                                                        **/
/** Output: None                                                           **/
/****************************************************************************/
static inline void gdx_hwacc_unbind(void)
{

}

/****************************************************************************/
/** Name: gdx_hwacc_recycle                                                **/
/**                                                                        **/
/** Description: Recycle the nbuff                                         **/
/**                                                                        **/
/** Input: nbuff_p - Data buffer                                           **/
/**        context - unused,for future use                                 **/
/**        flags   - indicates what to recyle                              **/
/**                                                                        **/
/** Output: None                                                           **/
/****************************************************************************/
static inline void gdx_hwacc_recycle(void *nbuff_p, unsigned long context, uint32_t flags)
{
    nbuff_free(nbuff_p);
}

/* ******Mandatory Functions - END*******/
 
#endif /* __GDX_CROSSBOW__H__ */
