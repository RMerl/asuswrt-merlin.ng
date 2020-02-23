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

/*
 *  Created on: Apr/2017
 *      Author: ido@broadcom.com
 */
#include <linux/module.h>
#include <linux/etherdevice.h>
#include "bcm_assert_locks.h"
#include "enet.h"
#include "enet_dbg.h"
#include "rdpa_api.h"
#include <linux/kthread.h>
#include <proc_cmd.h>
#include <linux/rtnetlink.h>

static enum
{
    FUNC_NONE,
    FUNC_INJECT,
    FUNC_PROXY_EGRESS,
    FUNC_PROXY_INGRESS,
} g_func = FUNC_NONE;

int enetx_weight_budget = 0;
static enetx_channel *enetx_channels;
typedef struct enetx_port_t *p;
struct net_device *inject_dev;

static inline void dumpHexData1(uint8_t *pHead, uint32_t len)
{
    uint32_t i;
    uint8_t *c = pHead;
    for (i = 0; i < len; ++i) {
        if (i % 16 == 0)
            printk("\n");
        printk("0x%02X, ", *c++);
    }
    printk("\n");
}

/* Called from platform ISR implementation */
inline int enetx_rx_isr(enetx_channel *chan)
{
    int i;

    enet_dbg_rx("rx_isr/priv %px\n", chan);

    for (i = 0; i < chan->rx_q_count; i++)
        enetxapi_queue_int_disable(chan, i);

    chan->rxq_cond = 1;
    wake_up_interruptible(&chan->rxq_wqh);

    return 0;
}

netdev_tx_t enet_xmit_ingress(pNBuff_t pNBuff, uint32_t port_id, uint32_t flow_id)
{
    rdpa_cpu_tx_info_t info = {};

    info.method = rdpa_cpu_tx_ingress;
    info.port = port_id;
    info.cpu_port = rdpa_cpu_host;
    info.drop_precedence = 0;
    info.flags = 0;

    if (rdpa_if_is_wan(port_id))
        info.x.wan.flow = flow_id;

    return rdpa_cpu_send_sysb(pNBuff, &info);
}

static inline void _free_fkb(FkBuff_t *fkb)
{
    fkb_flush(fkb, fkb->data, fkb->len, FKB_CACHE_FLUSH);
    enetxapi_fkb_databuf_recycle(fkb, (void *)(fkb->recycle_context));
}

extern struct sk_buff *skb_header_alloc(void);
static inline struct sk_buff *rx_skb(FkBuff_t *fkb, enetx_rx_info_t *rx_info)
{
    struct sk_buff *skb;

    skb = skb_header_alloc();
    if (unlikely(!skb))
    {
        enet_err("SKB allocation failure\n");
        _free_fkb(fkb);
        return NULL;
    }
    skb_headerinit((BCM_PKT_HEADROOM + rx_info->data_offset), 
#if defined(CC_NBUFF_FLUSH_OPTIMIZATION)
            SKB_DATA_ALIGN(fkb->len + BCM_SKB_TAILROOM + rx_info->data_offset),
#else
            BCM_MAX_PKT_LEN - rx_info->data_offset,
#endif
            skb, (uint8_t *)fkb->data, (RecycleFuncP)enetxapi_buf_recycle,(unsigned long) 0, fkb->blog_p);

    skb_trim(skb,fkb->len);

    skb->recycle_flags &= SKB_NO_RECYCLE; /* no skb recycle,just do data recyle */
    skb->recycle_flags |= rx_info->extra_skb_flags;

    skb->priority = fkb->priority;

    return skb;
}

static netdev_tx_t enet_xmit_egress(pNBuff_t pNBuff, uint32_t port_id, uint32_t flow_id, int egress_queue)
{
    rdpa_cpu_tx_info_t info = {};

    info.method = rdpa_cpu_tx_port;
    info.port = port_id;
    info.cpu_port = rdpa_cpu_host;
    info.drop_precedence = rdpa_discard_prty_low;
    info.flags = 0;

    if (rdpa_if_is_wan(port_id))
    {
        info.x.wan.flow = flow_id;
        info.x.wan.queue_id = egress_queue;
    }
    else
    {
        info.x.lan.queue_id = egress_queue;
    }

    return rdpa_cpu_send_sysb(FKBUFF_2_PNBUFF(pNBuff), &info);
}

static int g_proxy_egress_port_id, g_proxy_egress_flow_id, g_proxy_egress_egress_queue;
static int g_inject_src_port, g_inject_flow_id;

static inline int proxy_egress(FkBuff_t *fkb, enetx_rx_info_t *rx_info)
{
    return enet_xmit_egress(fkb, g_proxy_egress_port_id, g_proxy_egress_flow_id, g_proxy_egress_egress_queue);
}

/* Read up to budget packets from queue.
 * Return number of packets received on queue */
static inline int rx_pkt_from_q(int hw_q_id, int budget)
{
    int rc, count = 0;
    FkBuff_t *fkb;
    struct sk_buff *skb;
    enetx_rx_info_t rx_info;

    do
    {
        rc = enetxapi_rx_pkt(hw_q_id, &fkb, &rx_info);
        if (unlikely(rc))
            continue;

        switch (g_func)
        {
            case FUNC_PROXY_INGRESS:
                skb = rx_skb(fkb, &rx_info);
                if (skb)
                {
                    /* Do work here !
                     * i.e learn MAC; etc
                     */

                    enet_xmit_ingress(skb, rx_info.src_port, rx_info.flow_id);
                }
                break;
            case FUNC_INJECT:
                skb = rx_skb(fkb, &rx_info);
                if (skb)
                {
                    skb->dev = inject_dev;
                    /* TODO: Copy metadata to sock options? */

                    local_bh_disable();
                    netif_receive_skb(skb);
                    local_bh_enable();
                }
                break;
            case FUNC_PROXY_EGRESS:
                rc = proxy_egress(fkb, &rx_info);
                break;
            default:
                printk("enet_ingress: %s: function undefined\n", __FUNCTION__);
                _free_fkb(fkb);
        }

        count++; 
    }
    while (count < budget && likely(!rc));

    return count;
}

static inline int rx_pkt(enetx_channel *chan, int budget)
{
    int i, rc , count;

    /* Receive up to budget packets while Iterating over queues in channel by priority */
    for (count = 0, i = 0; i < chan->rx_q_count && count < budget; i++)
    {
        rc = rx_pkt_from_q(chan->rx_q[i], budget - count);
        count += rc;

        /*do not continue process an empty queue*/
        if(rc == 0)
            continue;
    }

    return count;
}

int chan_thread_handler(void *data)
{
    int work = 0;
    int reschedule;
    int i;
    enetx_channel *chan = (enetx_channel *) data;

    while (1)
    {
        wait_event_interruptible(chan->rxq_wqh, chan->rxq_cond || kthread_should_stop());
        if (kthread_should_stop())
            break;

        /*read budget from all queues of the channel*/
        work += rx_pkt(chan, enetx_weight_budget);
        reschedule = 0;

        /*if budget was not consumed then check if one of the
         * queues is full so thread will be reschedule - NAPI */
        if (work < enetx_weight_budget)
        {
            for (i = 0; i < chan->rx_q_count; i++)
            {
                if (enetxapi_queue_need_reschedule(chan, i))
                {
                    reschedule = 1;
                    break;
                }
            }
            /*enable interrupts again*/
            if (!reschedule)
            {
                work = 0;
                chan->rxq_cond = 0;
                for (i = 0; i < chan->rx_q_count; i++)
                {
                    enetxapi_queue_int_enable(chan, i);
                }
            }
        }
        else
        {
            work = 0;
            yield();
        }
    }

    return 0;
}

static int enet_open(void)
{
    enetx_channel *chan = enetx_channels;

    while (chan)
    {
        int i;
        for (i = 0; i < chan->rx_q_count; i++)
            enetxapi_queue_int_enable(chan, i);

        chan = chan->next;
    }

    return 0;
}

static int enet_close(void)
{
    enetx_channel *chan = enetx_channels;

    while (chan)
    {
        int i;
        for (i = 0; i < chan->rx_q_count; i++)
            enetxapi_queue_int_disable(chan, i);

        chan = chan->next;
    }

    return 0;
}

static int filter_cfg(void)
{
    rdpa_filter_global_cfg_t global_cfg;
    bdmf_object_handle filter;

    if (rdpa_filter_get(&filter))
        return -1;

    rdpa_filter_global_cfg_get(filter, &global_cfg);
    global_cfg.cpu_bypass = 1;
    rdpa_filter_global_cfg_set(filter, &global_cfg);

    bdmf_put(filter);

    return 0;
}

static netdev_tx_t inject_xmit(struct sk_buff *skb, struct net_device *dev)
{
    int rc;
    
    if (g_func != FUNC_INJECT)
    {
        kfree_skb(skb);
        return NETDEV_TX_OK;
    }

    rc = enet_xmit_ingress(skb, g_inject_src_port, g_inject_flow_id);
    if (rc)
        return NETDEV_TX_BUSY;

    return NETDEV_TX_OK;
}

static const struct net_device_ops inject_netdev_ops = {
    .ndo_start_xmit = inject_xmit,
};

static int inject_init(void)
{
    int ret;

    inject_dev = alloc_etherdev(0);
    if (!inject_dev)
        return -ENOMEM;

    strcpy(inject_dev->name, "inject");

    inject_dev->netdev_ops = &inject_netdev_ops;
    netif_carrier_on(inject_dev);

    ret = register_netdev(inject_dev);
    if (ret)
    {
        printk("failed to register net_device\n");
        free_netdev(inject_dev);
    }
    
    rtnl_lock();
    dev_open(inject_dev);
    rtnl_unlock();

    return ret;
}

static int inject_uninit(void)
{
    unregister_netdev(inject_dev);
    return 0;
}

static struct proc_dir_entry *proc_dir;
static struct proc_dir_entry *cmd_proc_file;
#define PROC_DIR           "driver/enet_ingress"
#define CMD_PROC_FILE      "cmd"

static int proc_cmd_help(int argc, char *argv[])
{
#define P(x) printk(x"\n")
    P("proxy_egress: Forward packets RXQ[1] -> TX egress <dest_rdpa_if> <flow_id> <queue_id>");
    P("--------------------------------------------------------------");
    P("Example for proxy mirroring rx of LAN1 to LAN0:");
    P("  echo proxy_egress 3 0 0 > /proc/driver/enet_ingress/cmd");
    P("  brctl delif br0 eth0.0");
    P("  bs /b/c system cpu_reason_to_tc[cpu_mirroring]=1");
    P("  bs /b/c system cfg={options=0x8}");
    P("  bs /b/c port/index=lan1 mirror_cfg={rx_dst_port={port/index=cpu0}}");
    P("");
    P("");
    P("proxy_ingress: Forward packets RXQ[1] -> TX ingress (from packet src port)");
    P("-------------------------------------------------------------------------");
    P("echo proxy_ingress > /proc/driver/enet_ingress/cmd");
    P("");
    P("");
    P("inject_ingress: Linux interface ingress -> TX ingress <rdpa_if> <flow_id> <redirect>");
    P("proxy_inject: Forward RXQ[1] -> Linux interface ingress");
    P("--------------------------------------------------------------------");
    P("Example for injecting packets to from LAN0:");
    P("  echo inject 3 0 > /proc/driver/enet_ingress/cmd");
    P("  tcpdump -nei inject &");
    P("  sendpackets -i inject -t 100000 -c1 -p aabbccddeeff000102030405060708090a");
    P("");
    P("");

    return 0;
}

static int proc_cmd_inject(int argc, char *argv[])
{
    int port_id, flow_id;

    if (argc != 3)
        goto Error;

    if (kstrtos32(argv[1], 10, &port_id))
        goto Error;

    if (kstrtos32(argv[2], 10, &flow_id))
        goto Error;

    g_inject_src_port = port_id;
    g_inject_flow_id = flow_id;
    g_func = FUNC_INJECT;

    return 0;

Error:
    printk("Usage: inject <src_rdpa_if> <flow_id>\n");
    return 0;
}

static int proc_cmd_proxy_egress(int argc, char *argv[])
{
    int port_id, flow_id, queue_id;

    if (argc != 4)
        goto Error;

    if (kstrtos32(argv[1], 10, &port_id))
        goto Error;

    if (kstrtos32(argv[2], 10, &flow_id))
        goto Error;
    
    if (kstrtos32(argv[3], 10, &queue_id))
        goto Error;

    g_proxy_egress_port_id = port_id;
    g_proxy_egress_flow_id = flow_id;
    g_proxy_egress_egress_queue = queue_id;
    g_func = FUNC_PROXY_EGRESS;

    return 0;

Error:
    printk("Usage: proxy_egress <dest_rdpa_if> <flow_id> <queue_id>\n");
    return 0;
}

static int proc_cmd_proxy_ingress(int argc, char *argv[])
{
    g_func = FUNC_PROXY_INGRESS;

    return 0;
}

static struct proc_cmd_ops command_entries[] = {
    { .name = "help", .do_command = proc_cmd_help},
    { .name = "inject", .do_command = proc_cmd_inject},
    { .name = "proxy_egress", .do_command = proc_cmd_proxy_egress},
    { .name = "proxy_ingress", .do_command = proc_cmd_proxy_ingress},
};

static struct proc_cmd_table command_table = {
    .module_name = "ENET_INGRESS",
    .size = ARRAY_SIZE(command_entries),
    .ops = command_entries,
};

int __init proc_init(void)
{
    proc_dir = proc_mkdir(PROC_DIR, NULL);
    if (!proc_dir)
    {
        pr_err("Failed to create PROC directory %s.\n", PROC_DIR);
        return -1;
    }

    cmd_proc_file = proc_create_cmd(CMD_PROC_FILE, proc_dir, &command_table);
    if (!cmd_proc_file)
    {
        pr_err("Failed to create %s\n", CMD_PROC_FILE);
        return -1;
    }

    printk("enet_ingress: For help type: # echo help > /proc/driver/enet_ingress/cmd\n");

    return 0;
}

int proc_uninit(void)
{
    if (cmd_proc_file)
    {
        remove_proc_entry(CMD_PROC_FILE, proc_dir);
        cmd_proc_file = NULL;
    }

    if (proc_dir)
    {
        remove_proc_entry(PROC_DIR, NULL);
        proc_dir = NULL;
    }

    return 0;
}

static void __ref bcm_enet_exit(void)
{
    proc_uninit();
    enet_close();
    inject_uninit();
    enetxapi_queues_uninit(&enetx_channels);
}
module_exit(bcm_enet_exit);

int __init bcm_enet_init(void)
{
    int rc = -1;

    if (BCM_SKB_ALIGNED_SIZE != skb_aligned_size())
    {
        enet_err("skb_aligned_size mismatch. Need to recompile enet module\n");
        return -ENOMEM;
    }

    if (proc_init())
        goto exit;

    if (filter_cfg())
        goto exit;

    if (enetxapi_queues_init(&enetx_channels))
        goto exit;

    enet_open();

    rc = inject_init();

exit:
    if (rc)
    {
        enet_err("Failed to inititialize, exiting\n");
        bcm_enet_exit();
    }

    return rc;
}

module_init(bcm_enet_init);

MODULE_DESCRIPTION("BCM internal enet testing network driver");
MODULE_LICENSE("GPL");

void phy_link_change_cb(void *ctx)
{
}

void enet_remove_netdevice(enetx_port_t *p)
{
}

int enet_create_netdevice(enetx_port_t *p)
{
    return 0;
}

void enet_dev_role_update(enetx_port_t *self)
{
}

void dynamic_meters_init(bdmf_object_handle cpu_obj, int watch_qid)
{
}

void dynamic_meters_uninit(bdmf_object_handle cpu_obj)
{
}

void ptp_1588_uninit(void)
{
}

int ptp_1588_init(void)
{
    return 0;
}

int is_pkt_ptp_1588(pNBuff_t pNBuff, rdpa_cpu_tx_info_t *info, char **ptp_offset)
{
    return 0;
}

int ptp_1588_cpu_send_sysb(bdmf_sysb sysb, rdpa_cpu_tx_info_t *info, char *ptp_header)
{
    return 0;
}

