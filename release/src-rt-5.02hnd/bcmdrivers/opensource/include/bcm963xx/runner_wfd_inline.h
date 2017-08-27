#ifndef __RUNNER_WFD_INLINE_H_INCLUDED__
#define __RUNNER_WFD_INLINE_H_INCLUDED__

/*
<:copyright-BRCM:2014:DUAL/GPL:standard 

   Copyright (c) 2014 Broadcom 
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
/******************* Other software units include files *********************/
/****************************************************************************/
#include "rdpa_api.h"
#include "rdpa_mw_blog_parse.h"
#include "rdp_cpu_ring_defs.h"
#include "rdp_mm.h"
#include "rdpa_cpu_helper.h"
#include "linux/prefetch.h"
/****************************************************************************/
/***************************** Definitions  *********************************/
/****************************************************************************/
typedef struct
{
    uint32_t ring_size;
    uint32_t descriptor_size;
    CPU_RX_DESCRIPTOR *head;
    CPU_RX_DESCRIPTOR *base;
    CPU_RX_DESCRIPTOR *end;
    uint32_t buff_cache_cnt;
    void **buff_cache;
} WFD_RING_S;

/* wlan0 if configuration params */
#define INIT_FILTERS_ARRY_SIZE 5
#define INIT_FILTER_EAP_FILTER_VAL 0x888E
#define WFD_RING_MAX_BUFF_IN_CACHE 32
static bdmf_object_handle rdpa_cpu_obj;

static int wifi_prefix_len;

#define WFD_WLAN_QUEUE_MAX_SIZE (RDPA_CPU_WLAN_QUEUE_MAX_SIZE)

static WFD_RING_S   wfd_rings[WFD_NUM_QUEUE_SUPPORTED];

#if defined(CONFIG_BCM96838) || defined(CONFIG_BCM96848) || defined(CONFIG_BCM96858)
static int wifi_netdev_event(struct notifier_block *n, unsigned long event, void *v);

static struct notifier_block wifi_netdev_notifer = {
    .notifier_call = wifi_netdev_event,
};

void replace_upper_layer_packet_destination(void *cb, void *napi_cb)
{
    send_packet_to_upper_layer = cb;
    send_packet_to_upper_layer_napi = napi_cb;
    inject_to_fastpath = 1;
}

void unreplace_upper_layer_packet_destination(void)
{
    send_packet_to_upper_layer = netif_rx;
    send_packet_to_upper_layer_napi = netif_receive_skb;
    inject_to_fastpath = 0;
}
#endif

/*****************************************************************************/
/****************** Wlan Accelerator Device implementation *******************/
/*****************************************************************************/

static inline void map_ssid_vector_to_ssid_index(uint16_t *bridge_port_ssid_vector, uint32_t *wifi_drv_ssid_index)
{
   *wifi_drv_ssid_index = __ffs(*bridge_port_ssid_vector);
}

#if defined(CONFIG_BCM96838) || defined(CONFIG_BCM96848) || defined(CONFIG_BCM96858)
static void rdpa_port_ssid_update(int index, int create)
{
    BDMF_MATTR(rdpa_port_attrs, rdpa_port_drv());
    bdmf_object_handle rdpa_port_obj;
    bdmf_object_handle tc_to_q_obj = NULL;
    rdpa_tc_to_queue_key_t key = {0, rdpa_dir_ds};

    int rc;
    if (!index)
    {
        rc = rdpa_tc_to_queue_get(&key, &tc_to_q_obj);
        if (rc)
        {
            printk("%s %s Failed to get tc_to_queue table rc(%d)\n", __FILE__, __FUNCTION__, rc);
            return;
        }
    }

    if (create)
    {
        rdpa_port_index_set(rdpa_port_attrs, rdpa_if_ssid0 + index);
        rc = bdmf_new_and_set(rdpa_port_drv(), NULL, rdpa_port_attrs, &rdpa_port_obj);
        if (rc)
        {
            printk("%s %s Failed to create rdpa port ssid object rc(%d)\n", __FILE__, __FUNCTION__, rc);
            return;
        }

        if (!index)
        {
            rc = bdmf_link(rdpa_port_obj, tc_to_q_obj, NULL);
            if (rc)
                printk("%s %s Failed to get tc_to_queue table rc(%d)\n", __FILE__, __FUNCTION__, rc);

            bdmf_put(tc_to_q_obj);
        }
    }
    else
    {
        rc = rdpa_port_get(rdpa_if_ssid0 + index, &rdpa_port_obj);
        if (!rc)
        {
            if (!index)
            {
                bdmf_unlink(rdpa_port_obj, tc_to_q_obj);
                bdmf_put(tc_to_q_obj);
            }
            bdmf_put(rdpa_port_obj);
            bdmf_destroy(rdpa_port_obj);
        }
    }
}

static int wifi_netdev_event(struct notifier_block *n, unsigned long event, void *v)
{
    int ret;
    uint32_t wifi_dev_index;
    struct net_device *dev = NETDEV_NOTIFIER_GET_DEV(v);

    ret = NOTIFY_DONE;

    /*Check for wifi net device*/
    if (!strncmp(wifi_prefix, dev->name, wifi_prefix_len))
    {
        wifi_dev_index = netdev_path_get_hw_port(dev);

        switch (event)
        {
           case NETDEV_REGISTER:
               if (!wifi_net_devices[wifi_dev_index])
               {
                   wifi_net_devices[wifi_dev_index] = dev;
                   dev_hold(dev);
                   rdpa_port_ssid_update(wifi_dev_index, 1);
               }
               ret = NOTIFY_OK;
               break;
            case NETDEV_UNREGISTER:
               if (wifi_net_devices[wifi_dev_index])
               {
                   dev_put(wifi_net_devices[wifi_dev_index]);
                   wifi_net_devices[wifi_dev_index] = NULL;
                   rdpa_port_ssid_update(wifi_dev_index, 0);
               }
               ret = NOTIFY_OK;
               break;
        }
    }

    return ret;
}
#endif


static inline int wfd_get_wfd_idx_from_qidx(int qidx)
{
    if (qidx >= WFD_NUM_QUEUE_SUPPORTED)
    {
        printk("qidx %d out of bounds %d\n", qidx, WFD_NUM_QUEUE_SUPPORTED);
    }
    return (qidx / WFD_NUM_QUEUES_PER_WFD_INST);
}

/****************************************************************************/
/**                                                                        **/
/** Name:                                                                  **/
/**                                                                        **/
/**   wfd_dev_rx_isr_callback.                                             **/
/**                                                                        **/
/** Title:                                                                 **/
/**                                                                        **/
/**   Wlan accelerator - ISR callback                                      **/
/**                                                                        **/
/** Abstract:                                                              **/
/**                                                                        **/
/**   ISR callback for the PCI queues handler                              **/
/**                                                                        **/
/** Input:                                                                 **/
/**                                                                        **/
/** Output:                                                                **/
/**                                                                        **/
/**                                                                        **/
/****************************************************************************/
static inline void wfd_dev_rx_isr_callback(long qidx)
{
    int wfdIdx = wfd_get_wfd_idx_from_qidx(qidx);

    /* Disable PCI interrupt */
    rdpa_cpu_int_disable(rdpa_cpu_wlan0, qidx);
    rdpa_cpu_int_clear(rdpa_cpu_wlan0, qidx);

    /*Atomically set the queue bit on*/
    set_bit(qidx, &wfd_objects[wfdIdx].wfd_rx_work_avail);

    /* Call the RDPA receiving packets handler (thread or tasklet) */
    WFD_WAKEUP_RXWORKER(wfdIdx);
}

static inline void *wfd_databuf_alloc(WFD_RING_S *pDescriptor)
{
   if (likely(pDescriptor->buff_cache_cnt))
   {
      return pDescriptor->buff_cache[--pDescriptor->buff_cache_cnt];
   }
   else
   {
      uint32_t alloc_cnt;
      /* refill the local cache from global pool */
      alloc_cnt = bdmf_sysb_databuf_alloc(pDescriptor->buff_cache, WFD_RING_MAX_BUFF_IN_CACHE, 0);
      if (alloc_cnt)
      {
         pDescriptor->buff_cache_cnt = alloc_cnt;
         return pDescriptor->buff_cache[--pDescriptor->buff_cache_cnt];
      }
   }
   return NULL;
}

static inline int wfd_delete_runner_ring(int ring_id)
{
    WFD_RING_S *pDescriptor;
    uint32_t entry;
    volatile CPU_RX_DESCRIPTOR *pTravel;

    pDescriptor = &wfd_rings[ring_id];
    if (!pDescriptor->ring_size)
    {
        printk("ERROR:deleting ring_id %d which does not exists!", ring_id);
        return -1;
    }

    /* free the data buffers in ring */
    if (pDescriptor->base)
    {
        pTravel = (volatile CPU_RX_DESCRIPTOR *)pDescriptor->base;
        for (entry = 0; entry < pDescriptor->ring_size; entry++)
        {
            if (pTravel->word2)
            {
#ifdef _BYTE_ORDER_LITTLE_ENDIAN_
                /* little-endian ownership is MSb of LSB */
                pTravel->word2 = swap4bytes(pTravel->word2 | 0x80);
#else
                /* big-endian ownership is MSb of MSB */
                pTravel->ownership = OWNERSHIP_HOST;
#endif
                bdmf_sysb_databuf_free((void *)phys_to_virt((phys_addr_t)pTravel->host_buffer_data_pointer), 0);
                pTravel->word2 = 0;
            }
            pTravel++;
        }
    }

    /* free any buffers in buff_cache */
    while (pDescriptor->buff_cache_cnt)
    {
        bdmf_sysb_databuf_free(pDescriptor->buff_cache[--pDescriptor->buff_cache_cnt], 0);
    }

    /* free buff_cache */
    if (pDescriptor->buff_cache)
        kfree(pDescriptor->buff_cache);

    /* delete the ring of descriptors */
    if (pDescriptor->base)
        rdp_mm_aligned_free((void *)NONCACHE_TO_CACHE(pDescriptor->base),
                            pDescriptor->ring_size * sizeof(CPU_RX_DESCRIPTOR));

    pDescriptor->ring_size = 0;

    return 0;
}

static inline int wfd_create_runner_ring(int ring_id, uint32_t size, uint32_t **ring_base)
{
    WFD_RING_S *pDescriptor;
    volatile CPU_RX_DESCRIPTOR *pTravel;
    uint32_t entry;
    void *dataPtr = NULL;
    bdmf_phys_addr_t phy_addr;

    if (ring_id >= WFD_NUM_QUEUE_SUPPORTED)
    {
        printk("ERROR:wfd ring_id %d out of range(%d)", ring_id,
                        (int)(sizeof(wfd_rings)/sizeof(WFD_RING_S)));
        return -1;
    }

    pDescriptor = &wfd_rings[ring_id];
    if (pDescriptor->ring_size)
    {
        printk("ERROR: ring_id %d already exists! must be deleted first", ring_id);
        return -1;
    }

    printk("Creating CPU ring for queue number %d with %d packets descriptor=0x%p\n ", ring_id, size, pDescriptor);

    /* set ring parameters */
    pDescriptor->ring_size = size;
    pDescriptor->descriptor_size = sizeof(CPU_RX_DESCRIPTOR);
    pDescriptor->buff_cache_cnt = 0;


    /* TODO:update the comment  allocate buff_cache which helps to reduce the overhead of when
     * allocating data buffers to ring descriptor */
    pDescriptor->buff_cache = (void **)(kmalloc(sizeof(void *) * WFD_RING_MAX_BUFF_IN_CACHE, GFP_ATOMIC));
    if (pDescriptor->buff_cache == NULL)
    {
        printk("failed to allocate memory for cache of data buffers \n");
        return -1;
    }

    /* allocate ring descriptors - must be non-cacheable memory */
    pDescriptor->base = (CPU_RX_DESCRIPTOR *)rdp_mm_aligned_alloc(sizeof(CPU_RX_DESCRIPTOR) * size, &phy_addr);
    if (pDescriptor->base == NULL)
    {
        printk("failed to allocate memory for ring descriptor\n");
        wfd_delete_runner_ring(ring_id);
        return -1;
    }


    /*initialize descriptors*/
    for (pTravel = (volatile CPU_RX_DESCRIPTOR *)pDescriptor->base, entry = 0; entry < size; pTravel++ , entry++)
    {
        memset((void *)pTravel, 0, sizeof(*pTravel));

        /* allocate actual packet in DDR */
        dataPtr = wfd_databuf_alloc(pDescriptor);
        if (dataPtr)
        {
#if defined(CONFIG_BCM963138) || defined(_BCM963138_) || defined(CONFIG_BCM963148) || defined(_BCM963148_) || defined(CONFIG_BCM94908) || defined(_BCM94908_)
            /* since ARM is little edian and runner is big endian we need to
             * byte swap the dataPtr.
             * this statementchange sthe owebrship bit to runner and swaps the bytes
             * and assigns to runner
             */
            pTravel->word2 = swap4bytes((uint32_t)virt_to_phys(dataPtr)) & ~0x80;
#else
            pTravel->host_buffer_data_pointer    = VIRT_TO_PHYS(dataPtr);
            pTravel->ownership                   = OWNERSHIP_RUNNER;
#endif
        }
        else
        {
            pTravel->host_buffer_data_pointer = 0;
            printk("failed to allocate packet map entry=%d\n", entry);
            wfd_delete_runner_ring(ring_id);
            return -1;
        }
    }

    /*set the ring header to the first entry*/
    pDescriptor->head = pDescriptor->base;

    /*using pointer arithmetics calculate the end of the ring*/
    pDescriptor->end = pDescriptor->base + size;

    *ring_base = (uint32_t *)(uintptr_t)phy_addr;

    return 0;
}

static inline int wfd_ring_get_queued(uint32_t ring_id)
{
    WFD_RING_S *pDescriptor = &wfd_rings[ring_id];
    volatile CPU_RX_DESCRIPTOR *pTravel = pDescriptor->base;
    volatile CPU_RX_DESCRIPTOR *pEnd = pDescriptor->end;
    uint32_t packets     = 0;

    while (pTravel != pEnd)
    {
        uint32_t ownership = (swap4bytes(pTravel->word2) & 0x80000000) ? 1 : 0;
        packets += (ownership == OWNERSHIP_HOST) ? 1 : 0;
        pTravel++;
    }

    return packets;
}

static void wfd_rxq_stat(int qid, extern_rxq_stat_t *stat, bdmf_boolean clear)
{
    int qidx = qid - first_pci_queue;

    if (stat)
    {
        memset(stat, 0, sizeof(extern_rxq_stat_t));

        stat->received += gs_count_rx_pkt[qidx];
        stat->dropped += gs_count_no_buffers[qidx] + gs_count_rx_invalid_ssid_vector[qidx] +
            gs_count_rx_no_wifi_interface[qidx];
        if (clear)
        {
            gs_count_rx_pkt[qidx] = 0;
            gs_count_no_buffers[qidx] = 0;
            gs_count_rx_invalid_ssid_vector[qidx] = 0;
            gs_count_rx_no_wifi_interface[qidx] = 0;
        }

        stat->queued = wfd_ring_get_queued(qidx);
    }
}

static inline int wfd_get_minQIdx(int wfd_idx)
{
    if (wfd_idx >= WFD_MAX_OBJECTS)
    {
        printk("%s wfd_idx %d out of bounds(%d)\n", __FUNCTION__, wfd_idx, WFD_MAX_OBJECTS);
        return -1;
    }

    /* 6838 yet to support multiple prio queues.
       So both min and max are same */
    /* For DSL Runner platforms,
       2 queues(low, high) per WFD. Even-Low, Odd-High
       WFD_IDX 0 - qidx0-lowprio, qidx1-hiprio
       WFD_IDX 1 - qidx2-lowprio, qidx3-hiprio */
    return (wfd_idx * WFD_NUM_QUEUES_PER_WFD_INST);
}

static inline int wfd_get_maxQIdx(int wfd_idx)
{
    if (wfd_idx >= WFD_MAX_OBJECTS)
    {
        printk("%s wfd_idx %d out of bounds(%d)\n", __FUNCTION__, wfd_idx, WFD_MAX_OBJECTS);
        return -1;
    }
    /* 6838 yet to support multiple prio queues.
       So both min and max are same */
    /* For DSL Runner platforms,
       2 queues(low, high) per WFD. Even-Low, Odd-High
       WFD_IDX 0 - qidx0-lowprio, qidx1-hiprio
       WFD_IDX 1 - qidx2-lowprio, qidx3-hiprio */
    return ((wfd_idx * WFD_NUM_QUEUES_PER_WFD_INST) + (WFD_NUM_QUEUES_PER_WFD_INST - 1));
}

static inline int wfd_config_rx_queue(int wfd_idx, int qid, uint32_t qsize,
                                      enumWFD_WlFwdHookType eFwdHookType)
{
    rdpa_cpu_rxq_cfg_t rxq_cfg;
    uint32_t *ring_base = NULL;
    int rc = 0;
    bdmf_sysb_type qsysb_type = bdmf_sysb_skb;
    int qidx = qid - first_pci_queue;

    if (eFwdHookType == WFD_WL_FWD_HOOKTYPE_FKB)
    {
        qsysb_type = bdmf_sysb_fkb;
    }

    /* Read current configuration, set new drop threshold and ISR and write back. */
    bdmf_lock();
    rc = rdpa_cpu_rxq_cfg_get(rdpa_cpu_obj, qid, &rxq_cfg);
    if (rc)
        goto unlock_exit;

    if (qsize) {
        rc = wfd_create_runner_ring(qidx, qsize, &ring_base);
        if (rc)
            goto unlock_exit;
    } else {
        wfd_delete_runner_ring(qidx);
    }

    rxq_cfg.size = qsize;
    rxq_cfg.isr_priv = qidx;
    rxq_cfg.rx_isr = qsize ? wfd_dev_rx_isr_callback : 0;
    rxq_cfg.ring_head = ring_base;
    rxq_cfg.ic_cfg.ic_enable = qsize ? true : false;
    rxq_cfg.ic_cfg.ic_timeout_us = WFD_INTERRUPT_COALESCING_TIMEOUT_US;
    rxq_cfg.ic_cfg.ic_max_pktcnt = WFD_INTERRUPT_COALESCING_MAX_PKT_CNT;
    rxq_cfg.rxq_stat = wfd_rxq_stat;
    rc = rdpa_cpu_rxq_cfg_set(rdpa_cpu_obj, qid, &rxq_cfg);

unlock_exit:
    bdmf_unlock();
    return rc;
}

#if defined(CONFIG_BCM96838) || defined(CONFIG_BCM96848) || defined(CONFIG_BCM96858)
static void release_wfd_interfaces(void)
{
    int wifi_index;
    bdmf_object_handle tc_to_q_obj = NULL;
    rdpa_tc_to_queue_key_t key = {0, rdpa_dir_ds};

    for (wifi_index = 0; wifi_index < WIFI_MW_MAX_NUM_IF; wifi_index++)
    {
        if (wifi_net_devices[wifi_index])
        {
            rdpa_port_ssid_update(wifi_index, 0);
            dev_put(wifi_net_devices[wifi_index]);
            wifi_net_devices[wifi_index] = NULL;
        }
    }

    if (!rdpa_tc_to_queue_get(&key, &tc_to_q_obj))
    {
        bdmf_put(tc_to_q_obj);
        bdmf_destroy(tc_to_q_obj);
    }

    bdmf_destroy(rdpa_cpu_obj);

    /* Unregister for NETDEV_REGISTER and NETDEV_UNREGISTER for wifi driver */
    unregister_netdevice_notifier(&wifi_netdev_notifer);
}

/****************************************************************************/
/**                                                                        **/
/** Name:                                                                  **/
/**                                                                        **/
/**   send_packet_to_bridge                                                **/
/**                                                                        **/
/** Title:                                                                 **/
/**                                                                        **/
/**   wlan accelerator - Rx PCI path                                       **/
/**                                                                        **/
/** Abstract:                                                              **/
/**                                                                        **/
/**   Sends packet to bridge and free skb buffer                           **/
/**                                                                        **/
/** Input:                                                                 **/
/**                                                                        **/
/** Output:                                                                **/
/**                                                                        **/
/**                                                                        **/
/****************************************************************************/
void send_packet_to_bridge(struct sk_buff *skb)
{
    rdpa_cpu_tx_info_t cpu_tx_info = {};
    size_t offset = (uint32_t)&PKTCLINK(skb) - (uint32_t)skb;
    /* Send the packet to the RDPA bridge for fast path forwarding */
    cpu_tx_info.port = netdev_path_get_hw_port(skb->dev) + rdpa_if_ssid0;

    gs_count_tx_packets[cpu_tx_info.port - rdpa_if_ssid0] += rdpa_cpu_send_wfd_to_bridge(skb, &cpu_tx_info, offset);
}

static inline void wfd_bulk_skb_fwd_bridged(unsigned int rx_pktcnt, void **rx_pkts)
{
    uint32_t wifi_drv_if_index;
    uint16_t wifi_ssid_vector;
    int loopcnt;
    struct sk_buff *skb_p;

    for (loopcnt = 0; loopcnt < rx_pktcnt; loopcnt++)
    {
        skb_p = (struct sk_buff *)rx_pkts[loopcnt];

        wifi_ssid_vector = skb_p->wl.ucast.nic.ssid_dst;
        map_ssid_vector_to_ssid_index(&wifi_ssid_vector, &wifi_drv_if_index);

        /* Check if device was initialized */
        if (unlikely(!wifi_net_devices[wifi_drv_if_index]))
        {
            nbuff_free(skb_p);
            printk("%s wifi_net_devices[%d] returned NULL\n", __FUNCTION__, wifi_drv_if_index);
            continue;
        }

        gs_count_rx_bridged_packets[wifi_drv_if_index]++;

        wifi_net_devices[wifi_drv_if_index]->netdev_ops->ndo_start_xmit(skb_p,
            wifi_net_devices[wifi_drv_if_index]);
    }
}
#endif

static inline void reset_current_descriptor(WFD_RING_S *pDescriptor, void *p_data)
{
    rdpa_cpu_ring_rest_desc(pDescriptor->head, p_data);

    if (++pDescriptor->head == pDescriptor->end)
        pDescriptor->head = pDescriptor->base;
}

static inline uint32_t
_wfd_bulk_fkb_get(uint32_t qid, uint32_t budget, wfd_object_t *wfd_p, void **rx_pkts)
{
    WFD_RING_S *pDescriptor = &wfd_rings[qid - first_pci_queue];
    volatile CPU_RX_DESCRIPTOR *pTravel = NULL;
    CPU_RX_DESCRIPTOR rxDesc;
    FkBuff_t *fkb_p;
    uint8_t *data;
    uint32_t len;
    void *pNewBuf;
    uint32_t rx_pktcnt = 0;

    while (budget)
    {
        pTravel = pDescriptor->head;

        rxDesc.word2 = pTravel->word2;

        rxDesc.word2 = swap4bytes(rxDesc.word2);

        if ((rxDesc.ownership == OWNERSHIP_HOST))
        {
            /* Initiate Uncached read of the descriptor words to save CPU cycles */
            rxDesc.word3 = pTravel->word3;
            rxDesc.word0 = pTravel->word0;

            rxDesc.ownership = 0; /*clear the ownership bit */
            data = (uint8_t *)phys_to_virt((phys_addr_t)rxDesc.word2);

            rxDesc.word3 = swap4bytes(rxDesc.word3);

            pNewBuf = wfd_databuf_alloc(pDescriptor);
            if (unlikely(!pNewBuf))
            {
                /* Push back the rxDesc */
                reset_current_descriptor(pDescriptor, data);
                break;
            }

            rxDesc.word0 = swap4bytes(rxDesc.word0);
            len = (uint32_t)rxDesc.packet_length;

#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM9408)
            /* cache_invalidate_len_outer_first(data, 32); */
            cache_invalidate_len_outer_first(data, len);
#endif
            bcm_prefetch(data, 1);

            /* Convert descriptor to FkBuff */
            fkb_p = fkb_init(data, BCM_PKT_HEADROOM, data, len);
#if defined(CC_NBUFF_FLUSH_OPTIMIZATION)
            fkb_p->dirty_p = _to_dptr_from_kptr_(data + ETH_HLEN);
#endif
            fkb_p->recycle_hook = bdmf_sysb_recycle;
            fkb_p->recycle_context = 0;

            /* Both ucast and mcast share the same wl_prio bits position */
            fkb_p->wl.ucast.dhd.is_ucast = rxDesc.is_ucast;
            fkb_p->wl.ucast.dhd.wl_prio = rxDesc.wl_tx_prio;

            if (fkb_p->wl.ucast.dhd.is_ucast)
            {
                fkb_p->wl.ucast.dhd.ssid = rxDesc.ssid;
                fkb_p->wl.ucast.dhd.flowring_idx = rxDesc.flow_ring_idx;
            }
            else
            {
                fkb_p->wl.mcast.ssid_vector = rxDesc.ssid_vector;
            }

            rx_pkts[rx_pktcnt] = (void *)fkb_p;

            rx_pktcnt++;

            reset_current_descriptor(pDescriptor, pNewBuf);
        }
        else
        {
            /* No more packets to read. Ring is empty */
            break;
        }
        budget--;
    }

    return rx_pktcnt;
}

static inline void wfd_fwd_mcast(void *nkb_p, wfd_object_t *wfd_p, uint16_t ssid_vector, int is_fkb)
{
    uint32_t wl_if_index;
    void *nkbC_p = NULL;

    while (ssid_vector)
    {
        map_ssid_vector_to_ssid_index(&ssid_vector, &wl_if_index);

        /* Clear the bit we found */
        ssid_vector &= ~(1 << wl_if_index);

        /* Check if device was initialized */
        if (unlikely(!wfd_p->wl_if_dev[wl_if_index]))
        {
            nbuff_free(nkb_p);
            printk("%s wifi_net_devices[%d] returned NULL\n", __FUNCTION__, wl_if_index);
            return;
        }

        wfd_p->count_rx_queue_packets++;
        wfd_p->wl_mcast_packets++;

        if (ssid_vector) /* To prevent (last/the only) ssid copy */
        {
            if (is_fkb)
            {
                /* clone fkb */
                nkbC_p = fkb_clone(nkb_p);
            }
            else
            {
                /* skb copy */
                nkbC_p = skb_copy(nkb_p, GFP_ATOMIC);
            }

            if (!nkbC_p)
            {
                printk("%s %s: Failed to clone %ckb\n", __FILE__, __FUNCTION__, is_fkb ? 'f':'s');
                nbuff_free(nkb_p);
                return;
            }
        }
        else
        {
            nkbC_p = nkb_p;
        }

        (void)wfd_p->wfd_mcastHook(wfd_p->wl_radio_idx, (unsigned long)nkbC_p, (unsigned long)wfd_p->wl_if_dev[wl_if_index]);
    }

    /* Done with mcast fwding */
}

static inline void wfd_fwd_pkts_fkb(uint32_t rx_pktcnt, void **rx_pkts, wfd_object_t *wfd_p)
{
    int ucast_cnt = 0;
    void **ucast_pkts = rx_pkts;
    FkBuff_t *fkb_p;
    int pktidx;

    for (pktidx = 0; pktidx < rx_pktcnt; pktidx++)
    {
        fkb_p = rx_pkts[pktidx];
        if (!fkb_p->wl.ucast.dhd.is_ucast)
        {
            if (ucast_cnt)
            {
                /* Forward the accumulated ucast packets */
                (void)wfd_p->wfd_fwdHook(ucast_cnt, (unsigned long)ucast_pkts, wfd_p->wl_radio_idx, 0);
                wfd_p->wl_chained_packets += ucast_cnt;
                wfd_p->count_rx_queue_packets += ucast_cnt;
                ucast_cnt = 0;
            }

            /* Forward mcast packet */
            wfd_fwd_mcast(fkb_p, wfd_p, fkb_p->wl.mcast.ssid_vector, 1);
        }
        else
        {
            if (ucast_cnt == 0)
            {
                ucast_pkts = &rx_pkts[pktidx];
            }
            ucast_cnt++;
        }
    }

    if (ucast_cnt)
    {
        /* Forward the accumulated ucast packets */
        (void) wfd_p->wfd_fwdHook(ucast_cnt, (unsigned long)ucast_pkts, wfd_p->wl_radio_idx, 0);
        wfd_p->wl_chained_packets += ucast_cnt;
        wfd_p->count_rx_queue_packets += ucast_cnt;
    }
}

static inline void wfd_fwd_pkts_skb(uint32_t rx_pktcnt, void **rx_pkts, wfd_object_t *wfd_p)
{
    int ucast_cnt = 0;
    void **ucast_pkts = rx_pkts;
    struct sk_buff *skb_p;
    int pktidx;

    for (pktidx = 0; pktidx < rx_pktcnt; pktidx++)
    {
        skb_p = rx_pkts[pktidx];
        if (!skb_p->wl.ucast.nic.is_ucast)
        {
            if (ucast_cnt)
            {
                /* Forward the accumulated ucast packets */
                (void)wfd_p->wfd_fwdHook(ucast_cnt, (unsigned long)ucast_pkts, wfd_p->wl_radio_idx, 0);
                wfd_p->wl_chained_packets += ucast_cnt;
                wfd_p->count_rx_queue_packets += ucast_cnt;
                ucast_cnt = 0;
            }

            /* Forward mcast packet */
            wfd_fwd_mcast(skb_p, wfd_p, skb_p->wl.mcast.ssid_vector, 0);
        }
        else
        {
            if (ucast_cnt == 0)
            {
                ucast_pkts = &rx_pkts[pktidx];
            }
            ucast_cnt++;
        }
    }

    if (ucast_cnt)
    {
        /* Forward the accumulated ucast packets */
        (void)wfd_p->wfd_fwdHook(ucast_cnt, (unsigned long)ucast_pkts, wfd_p->wl_radio_idx, 0);
        wfd_p->wl_chained_packets += ucast_cnt;
        wfd_p->count_rx_queue_packets += ucast_cnt;
    }
}

static inline uint32_t
wfd_bulk_fkb_get(unsigned long qid, unsigned long budget, void *priv, void **rx_pkts)
{
    uint32_t rx_pktcnt;
    wfd_object_t *wfd_p = (wfd_object_t *)priv;

    rx_pktcnt = _wfd_bulk_fkb_get(qid, budget, wfd_p, rx_pkts);

    if (rx_pktcnt)
    {
       wfd_fwd_pkts_fkb(rx_pktcnt, rx_pkts, wfd_p);
    }
    return rx_pktcnt;
}

static inline uint32_t _wfd_bulk_skb_get(unsigned long qid, unsigned long budget, void **rx_pkts)
{
    WFD_RING_S *pDescriptor = &wfd_rings[qid - first_pci_queue];
    volatile CPU_RX_DESCRIPTOR *pTravel = NULL;
    CPU_RX_DESCRIPTOR rxDesc;
    struct sk_buff *skb_p;
    uint8_t *data;
    uint32_t len;
    void *pNewBuf;
    unsigned int rx_pktcnt = 0;

    while (budget)
    {
        pTravel = pDescriptor->head;

        rxDesc.word2 = pTravel->word2;

        rxDesc.word2 = swap4bytes(rxDesc.word2);

        if (rxDesc.ownership == OWNERSHIP_HOST)
        {
            /* Initiate Uncached read of the descriptor words to save CPU cycles */
            rxDesc.word0 = pTravel->word0;
            rxDesc.word3 = pTravel->word3;

            rxDesc.ownership = 0; /*clear the ownership bit */
            data = (uint8_t *)phys_to_virt((phys_addr_t)rxDesc.word2);

            pNewBuf = wfd_databuf_alloc(pDescriptor);
            if (unlikely(!pNewBuf))
            {
                /* Push back the rxDesc */
                gs_count_no_buffers[qid - first_pci_queue]++;
                reset_current_descriptor(pDescriptor, data);
                break;
            }

            rxDesc.word0 = swap4bytes(rxDesc.word0);

            len = (uint32_t)rxDesc.packet_length;

#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM94908)
            /* cache_invalidate_len_outer_first(data, 32); */
            cache_invalidate_len_outer_first(data, len);
#endif

            bcm_prefetch(data, 1);
            rxDesc.word3 = swap4bytes(rxDesc.word3);

            /* allocate skb structure*/
            skb_p = skb_header_alloc();
            if (!skb_p)
            {
                printk("%s : SKB Allocation failure\n", __FUNCTION__);
                reset_current_descriptor(pDescriptor, pNewBuf);
                break;
            }

            /* initialize the skb */
            skb_headerinit(BCM_PKT_HEADROOM,
#if defined(CC_NBUFF_FLUSH_OPTIMIZATION)
                           SKB_DATA_ALIGN(len + BCM_SKB_TAILROOM),
#else
                           BCM_MAX_PKT_LEN,
#endif
                           skb_p, data, bdmf_sysb_recycle, 0, NULL);

            skb_trim(skb_p, len);
            skb_p->recycle_flags &= SKB_NO_RECYCLE; /* no skb recycle,just do data recyle */

#if defined(CC_NBUFF_FLUSH_OPTIMIZATION)
            /* Set dirty pointer to optimize cache invalidate */
            skb_shinfo((struct sk_buff *)(skb_p))->dirty_p = skb_p->data + ETH_HLEN;
#endif

            skb_p->wl.ucast.nic.is_ucast = rxDesc.is_ucast;
            if (skb_p->wl.ucast.nic.is_ucast)
            {
                skb_p->wl.ucast.nic.wl_chainidx = rxDesc.wl_chain_id & 0xff;
#if defined(CONFIG_BCM96838) || defined(CONFIG_BCM96848) || defined(CONFIG_BCM96858)
                if (qid == first_pci_queue + WFD_BRIDGED_QUEUE_IDX)
                    skb_p->wl.ucast.nic.ssid_dst = rxDesc.ssid_vector; /* For bridged traffic only */
#endif
            }
            else
            {
                skb_p->wl.mcast.ssid_vector = rxDesc.ssid_vector;
            }
            DECODE_WLAN_PRIORITY_MARK(rxDesc.wl_tx_prio, skb_p->mark);

            rx_pkts[rx_pktcnt] = (void *)skb_p;

            reset_current_descriptor(pDescriptor, pNewBuf);
        }
        else
        {
            /* No more packets to read. Ring is empty */
            break;
        }
        budget--;
        rx_pktcnt++;
    }

    return rx_pktcnt;
}

static uint32_t
wfd_bulk_skb_get(unsigned long qid, unsigned long budget, void *priv, void **rx_pkts)
{
    unsigned int rx_pktcnt;
    wfd_object_t *wfd_p = (wfd_object_t *)priv;

    rx_pktcnt = _wfd_bulk_skb_get(qid, budget, rx_pkts);

    if (rx_pktcnt)
    {
#if defined(CONFIG_BCM96838) || defined(CONFIG_BCM96848) || defined(CONFIG_BCM96858)
        if (qid == first_pci_queue + WFD_BRIDGED_QUEUE_IDX) /* Briged traffic only queue */
            wfd_bulk_skb_fwd_bridged(rx_pktcnt, rx_pkts);
        else
#endif
        (void)wfd_fwd_pkts_skb(rx_pktcnt, rx_pkts, wfd_p);
    }

    return rx_pktcnt;
}

/****************************************************************************/
/**                                                                        **/
/** Name:                                                                  **/
/**                                                                        **/
/**   wfd_accelerator_init                                                 **/
/**                                                                        **/
/** Title:                                                                 **/
/**                                                                        **/
/**   wifi accelerator - init                                              **/
/**                                                                        **/
/** Abstract:                                                              **/
/**                                                                        **/
/**   The function initialize all the runner resources.                    **/
/**                                                                        **/
/** Input:                                                                 **/
/**                                                                        **/
/** Output:                                                                **/
/**                                                                        **/
/**                                                                        **/
/****************************************************************************/
static int wfd_accelerator_init(void)
{
#if defined(CONFIG_BCM96838) || defined(CONFIG_BCM96848) || defined(CONFIG_BCM96858)
    int error;
    char wifi_if_name[WIFI_IF_NAME_STR_LEN];
    int wifi_dev_index;
#endif
    BDMF_MATTR(cpu_wlan0_attrs, rdpa_cpu_drv());

    wifi_prefix_len = strlen(wifi_prefix);

    /* create cpu */
    rdpa_cpu_index_set(cpu_wlan0_attrs, rdpa_cpu_wlan0);
    if (bdmf_new_and_set(rdpa_cpu_drv(), NULL, cpu_wlan0_attrs, &rdpa_cpu_obj))
    {
        printk("%s %s Failed to create cpu wlan0 object\n", __FILE__, __FUNCTION__);
        return -1;
    }

    if (rdpa_cpu_int_connect_set(rdpa_cpu_obj, true))
    {
        printk("%s %s Failed to connect cpu interrupts\n", __FILE__, __FUNCTION__);
        return -1;
    }

    /* Init wifi driver callback */
#if defined(CONFIG_BCM96838) || defined(CONFIG_BCM96848) || defined(CONFIG_BCM96858)
    replace_upper_layer_packet_destination(send_packet_to_bridge, send_packet_to_bridge);

    for (wifi_dev_index = 0; wifi_dev_index < WIFI_MW_MAX_NUM_IF; wifi_dev_index++)
    {
        if (wifi_dev_index % 8)
            error = (wifi_dev_index > 8) ?
                snprintf(wifi_if_name, WIFI_IF_NAME_STR_LEN, "%s1.%u", wifi_prefix, wifi_dev_index-7) :
                snprintf(wifi_if_name, WIFI_IF_NAME_STR_LEN, "%s0.%u", wifi_prefix, wifi_dev_index);
        else
            error = (wifi_dev_index == 0) ?
                snprintf(wifi_if_name, WIFI_IF_NAME_STR_LEN, "%s0", wifi_prefix) :
                snprintf(wifi_if_name, WIFI_IF_NAME_STR_LEN, "%s1", wifi_prefix);

        if (error == -1)
        {
            printk("%s %s: wifi interface name retrieval failed \n", __FILE__, __FUNCTION__);
            goto error_handling;
        }

        if (!wifi_net_devices[wifi_dev_index])
        {
            wifi_net_devices[wifi_dev_index] = dev_get_by_name(&init_net, wifi_if_name);
            if (wifi_net_devices[wifi_dev_index])
                rdpa_port_ssid_update(wifi_dev_index, 1);
        }
    }

    /* Register for NETDEV_REGISTER and NETDEV_UNREGISTER for wifi driver */
    register_netdevice_notifier(&wifi_netdev_notifer);
#endif

    return 0;

#if defined(CONFIG_BCM96838) || defined(CONFIG_BCM96848) || defined(CONFIG_BCM96858)
error_handling:
    release_wfd_interfaces();
    return -1;
#endif
}

static inline int wfd_queue_not_empty(int radio, long qid, int qidx)
{
    return rdpa_cpu_ring_not_empty(wfd_rings[qidx].head);
}

static inline void wfd_int_enable(int radio, long qid, int qidx)
{
    rdpa_cpu_int_enable(rdpa_cpu_wlan0, qidx);
}

static inline void wfd_int_disable(int radio, long qid, int qidx)
{
    rdpa_cpu_int_disable(rdpa_cpu_wlan0, qidx);
}

static inline void *wfd_acc_info_get(int radio)
{
    return rdpa_cpu_data_get(rdpa_cpu_wlan0);
}

static inline int wfd_get_qid(int qidx)
{
    return first_pci_queue + qidx;
}

static inline int wfd_get_objidx(int qid, int qidx)
{
    return qidx;
}
#endif /* __RUNNER_WFD_INLINE_H_INCLUDED__ */
