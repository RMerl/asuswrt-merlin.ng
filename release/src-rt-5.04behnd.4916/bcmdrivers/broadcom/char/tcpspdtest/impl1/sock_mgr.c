/*
<:copyright-BRCM:2018:DUAL/GPL:standard

   Copyright (c) 2018 Broadcom 
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
*******************************************************************************
* File Name  : sock_mgr.c
*
* Description: This file contains the Broadcom Tcp Speed Test Socket Manager Implementation.
*
*  Created on: Dec 6, 2016
*      Author: yonatani, ilanb
*******************************************************************************
*/

#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_ipv6.h>
#include <linux/bcm_log.h>
#include <linux/percpu.h>
#include "tcpspdtest.h"
#include "tcp_engine_api.h"
#include "tcpspdtest_defs.h"
#include "sock_mgr.h"
#include "rnr_flow.h"
#ifdef CONFIG_BCM_XRDP
#include "rdpa_udpspdtest.h"
#include "rdpa_ag_udpspdtest.h"
#include <net/udp.h>
#include <net/ip6_checksum.h>
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
#include <linux/net_namespace.h>
#endif
#include "spdsvc_defs.h"
#include "rdpa_drv.h"

#define WAIT_LOOP 100
#define WAIT_MSEC 50

typedef int (*hijack_tcp_in)(uint8_t stream_idx, void *, uint32_t txwscale);
typedef int (*hijack_tcp_out)(uint8_t stream_idx, void *, uint8_t);

/******************************************** Defines ********************************************/
#define TCP_MAX_MSS 1460

#define OOB_MSG_LEN sizeof(int)
#define OOB_IPV4_PKT_LEN  (sizeof(struct iphdr) + sizeof(struct udphdr) + OOB_MSG_LEN)
#define OOB_IPV6_PKT_LEN  (sizeof(struct ipv6hdr) + sizeof(struct udphdr) + OOB_MSG_LEN)

/**************************************** Global / Static ****************************************/
static struct tcp_info sock_tcpinfo[SPDT_NUM_OF_STREAMS];
static atomic_t nf_hook_prtcl[SPDT_NUM_OF_STREAMS] = {ATOMIC_INIT(0)};
static int sock_learned[SPDT_NUM_OF_STREAMS] = {0};
atomic_t connected_streams = ATOMIC_INIT(0);

extern uint8_t g_stream_alloc[];

/**************************************** Implementation *****************************************/
/* TODO:REPLACE with HASH table!!! */
static int8_t get_stream_idx(struct sk_buff *skb)
{
    int8_t i;

    for (i = 0; i < SPDT_NUM_OF_STREAMS; i++)
    {
        if (g_tcpspd[i].srv_socket && skb->sk && skb->sk == g_tcpspd[i].srv_socket->sk)
            return i;
    }

    /* not found */
    return -1;
}

/* Check valid packet before hijacked */
static int8_t nf_hook_check_valid(struct sk_buff *skb, int is_tcp)
{
    int8_t stream_idx = -1;

    if (!skb || !skb->sk || !skb->sk->sk_socket) 
        goto out;

    if (is_tcp)
    {
        /* Handle our socket only, and only TCP packets (our socket might send ARPs as well) */
        stream_idx = get_stream_idx(skb);
        if (stream_idx < 0)
            goto out;
    }
#ifdef CONFIG_BCM_XRDP
    else
    {
        int i;
        uint16_t ethertype = (((struct ipv6hdr *)skb->data)->version == 6) ? ETH_P_IPV6 : ETH_P_IP;

        if (skb->len > ((ethertype == ETH_P_IPV6) ? OOB_IPV6_PKT_LEN : OOB_IPV4_PKT_LEN))
        {
            for(i = 0; i < SPDT_NUM_OF_STREAMS; i++)
            {
                if (g_stream_alloc[i] && g_tcpspd[i].sk == skb->sk)
                {
                    stream_idx = i;
                    break;
                }
            }
        }
    }
#endif

    tc_debug("match stream_idx=%d\n", stream_idx);

out:
    return stream_idx;
}

extern int tcpspd_engine_packet_rcv(void *date, uint32_t len);

int _tcp_recv(pNBuff_t nbuff, BlogFcArgs_t *fc_args)
{
    tcpspd_engine_packet_rcv((uint8_t *)nbuff_get_data(nbuff) + ETH_HLEN, nbuff_get_len(nbuff) - ETH_HLEN);

    /* FKB->data needs to be cache invalidated before getting
     * freed, or else it would cause recycled buffer issue */
    nbuff_flushfree(nbuff);

    return 0;
}

static const struct net_device_ops _spdtst_netdev_ops = {
    .ndo_open               = NULL,
    .ndo_stop               = NULL,
    .ndo_start_xmit         = (HardStartXmitFuncP)_tcp_recv,
    .ndo_set_mac_address    = NULL,
    .ndo_do_ioctl           = NULL,
    .ndo_tx_timeout         = NULL,
    .ndo_get_stats          = NULL,
    .ndo_change_mtu         = NULL 
};

static DEFINE_PER_CPU(int, cpu_refcnt);

struct net_device *_spdtst_netdev = 0;

static void __sock_mgr_netdev_setup(struct net_device *dev)
{
    dev->netdev_ops     = &_spdtst_netdev_ops;
    dev->mtu            = 1500;
    dev->pcpu_refcnt    = &cpu_refcnt;
}

static void tcpspeedtest_on_set(uint8_t enabled, uint8_t stream_idx)
{
    tcpspd_engine_set_test_on_off(enabled, stream_idx);
}

static int _nf_hook_out_tcp(uint8_t stream_idx, struct sk_buff *skb, const struct nf_hook_state *state, uint8_t spdtst)
{
    BlogAction_t action;
    BlogFcArgs_t fcArgs={};

    if (tcpspd_engine_hijack_hook_conn_learn(stream_idx, skb->data, 1))
    {
        skb_pull(skb, ETH_HLEN);
        return -1;
    }

    if (SS_CONNECTING == g_tcpspd[stream_idx].srv_socket->state)
    {
        sock_learned[stream_idx] = 1;
        action = blog_sinit(skb, _spdtst_netdev, TYPE_ETH, 0, BLOG_SPDTST, &fcArgs);
        tc_debug("blog_sinit() action=%d blog=%p\n", action, skb->blog_p);
    }

    if (skb->blog_p)
    {
        skb->blog_p->spdtst = spdtst;
        skb->blog_p->iq_prio = BLOG_IQ_PRIO_HIGH;
    }

    return 0;
}


/* Netfilter IPv4/6 In/Out hooks for TCP */
static unsigned int _nf_hook_in_out_tcp(uint32_t hooknum, struct sk_buff *skb, const struct nf_hook_state *state,
    hijack_tcp_in hijack_tcp_in, hijack_tcp_out hijack_tcp_out)
{
    int rc;
    int8_t stream_idx;
    union {
        spdtst_bits_t bits;
        uint8_t val;
    } spdtst = {0};

    /* Discard not valid packets */
    stream_idx = nf_hook_check_valid(skb, 1);
    if (stream_idx < 0)
        return NF_ACCEPT;

    /* Delete blog created by PKT_TCP4_LOCAL during http head request.
       Created blog sent the received packets directly to linux tcp layer skipping ip layer and netfilter hooks */
    if (!g_tcpspd[stream_idx].add_flow && NF_INET_LOCAL_IN == hooknum && skb->blog_p)
    {
        tc_debug("[%hhu] blog_skip %px\n", stream_idx, skb->blog_p);
        blog_skip(skb, blog_skip_reason_ct_tcp_state_ignore);
    }

    spdtst.bits.is_tcp = 1; /* Used to distinguish TCP SpeedService from other flavours */
    spdtst.bits.is_hw = STREAM_IS_HWACCEL(stream_idx); /* Used by fcachehw */
    spdtst.bits.is_dir_upload = (g_tcpspd[stream_idx].action == SPDT_DIR_TX); 
    spdtst.bits.stream_idx = stream_idx; 

    if (g_tcpspd[stream_idx].srv_socket &&
        (SS_CONNECTING == g_tcpspd[stream_idx].srv_socket->state ||
        SS_UNCONNECTED == g_tcpspd[stream_idx].srv_socket->state))
    {
        skb_push(skb, ETH_HLEN);

        if (NF_INET_LOCAL_OUT == hooknum)
        {
            if (_nf_hook_out_tcp(stream_idx, skb, state, spdtst.val))
                return NF_ACCEPT;
        }
        else
        {
            bdmf_mac_t host_mac = {};
            if (!tcpspd_rnr_flow_get_host_mac(&host_mac, skb->dev))
                tcpspd_engine_hijack_hook_conn_learn_host_mac(stream_idx, &host_mac);
        }

        skb_pull(skb, ETH_HLEN);
    }

    if (!tcpspd_engine_is_sock_learned(stream_idx))
        return NF_ACCEPT;

    tc_debug("[%hhu] state:%d, hooknum:%d\n", stream_idx, g_tcpspd[stream_idx].srv_socket->state, hooknum);

    /* Ignore hijacked packet during engine data transfer (except for for FIN) */
    if (tcpspd_engine_is_state_up(stream_idx))
    {
        tcpspd_engine_done_data(stream_idx, skb_network_header(skb));
        return NF_ACCEPT;
    }

    /* Hijack packet for prtcl processing */
    if (atomic_read(&nf_hook_prtcl[stream_idx]))
    {
        /* HTTP prtcl processing, head response - learn content length */
        rc = tcpspd_prtcl_nf_hook(stream_idx, skb, hooknum);
        if (NF_DROP == rc)
            return NF_DROP;
    }

    /* Server to Linux and Linux to Server conversion during TCP connected state */
    if (sock_learned[stream_idx] || SS_CONNECTED == g_tcpspd[stream_idx].srv_socket->state)
    {
        if (NF_INET_LOCAL_OUT == hooknum)
        {
            if (hijack_tcp_out(stream_idx, skb_network_header(skb), skb->ip_summed))
                return NF_DROP;
        }
        else if (NF_INET_LOCAL_IN == hooknum)
        {
            if (SS_CONNECTED == g_tcpspd[stream_idx].srv_socket->state)
                hijack_tcp_in(stream_idx, skb_network_header(skb), sock_tcpinfo[stream_idx].tcpi_snd_wscale);
        }
    }

    if (!sock_learned[stream_idx])
        return NF_ACCEPT;

    if (NF_INET_LOCAL_IN == hooknum && SS_CONNECTED == g_tcpspd[stream_idx].srv_socket->state)
    {
        BlogAction_t action;
        struct net_device *tmpdev;

        if (g_tcpspd[stream_idx].add_flow && skb->blog_p)
        {
            uint8_t l2_hdr[ETH_HLEN] = {};
            uint16_t ethertype = (((struct ipv6hdr *)skb->data)->version == 6) ? ETH_P_IPV6 : ETH_P_IP;

            skb->blog_p->spdtst = spdtst.val;
            skb->blog_p->iq_prio = BLOG_IQ_PRIO_HIGH;
            tc_debug("[%hhu] blog_emit() skb=%px blog=%px\n", stream_idx, skb, skb->blog_p);
            skb_push(skb, ETH_HLEN);
            *(uint16_t *)(l2_hdr + (ETH_ALEN << 1)) = htons(ethertype);
            memcpy(skb->data, l2_hdr, ETH_HLEN);

            tmpdev = skb->dev;
            skb->dev = _spdtst_netdev;
            action = blog_emit(skb, skb->dev, TYPE_ETH, 0, BLOG_SPDTST);
            skb->dev = tmpdev;

            skb_pull(skb, ETH_HLEN);
            g_tcpspd[stream_idx].add_flow = 0;
        }
    }

    return NF_ACCEPT;
}

/* Netfilter IPv4 In/Out hooks for TCP */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0))
static unsigned int nf_hook_in_out_tcp(const struct nf_hook_ops *ops, struct sk_buff *skb, const struct nf_hook_state *state)
{
    return _nf_hook_in_out_tcp(ops->hooknum, skb, state, tcpspd_engine_hijack_tcp_in, tcpspd_engine_hijack_tcp_out);
}

/* Netfilter IPv6 In/Out hooks for TCP */
static unsigned int nf_hook_in_out6_tcp(const struct nf_hook_ops *ops, struct sk_buff *skb, const struct nf_hook_state *state)
{
    return _nf_hook_in_out_tcp(ops->hooknum, skb, state, tcpspd_engine_hijack_tcp_in6, tcpspd_engine_hijack_tcp_out6);
}
#else
static unsigned int nf_hook_in_out_tcp(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    return _nf_hook_in_out_tcp(*(uint32_t *)priv, skb, state, tcpspd_engine_hijack_tcp_in, tcpspd_engine_hijack_tcp_out);
}

/* Netfilter IPv6 In/Out hooks for TCP */
static unsigned int nf_hook_in_out6_tcp(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    return _nf_hook_in_out_tcp(*(uint32_t *)priv, skb, state, tcpspd_engine_hijack_tcp_in6, tcpspd_engine_hijack_tcp_out6);
}
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
static uint32_t nf_hks_tcp_priv[] =  {
    NF_INET_LOCAL_OUT,
    NF_INET_LOCAL_IN,
    NF_INET_LOCAL_OUT,
    NF_INET_LOCAL_IN
};
#endif

/* Netfilter declare hooks for TCP */
static struct nf_hook_ops nf_hks_tcp[] = {
    {
        .hook = nf_hook_in_out_tcp,
        .hooknum = NF_INET_LOCAL_OUT,
        .pf = NFPROTO_IPV4,
        .priority = NF_IP_PRI_FIRST,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0))
        .owner = THIS_MODULE
#else
        .priv = (void *)&nf_hks_tcp_priv[0]
#endif
    }, /* net filter OUT hook options */
    {
        .hook = nf_hook_in_out_tcp,
        .hooknum = NF_INET_LOCAL_IN,
        .pf = NFPROTO_IPV4,
        .priority = NF_IP_PRI_FILTER - 1,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0))
        .owner = THIS_MODULE
#else
        .priv = (void *)&nf_hks_tcp_priv[1]
#endif
    }, /* net filter IN hook options */
    {
        .hook = nf_hook_in_out6_tcp,
        .hooknum = NF_INET_LOCAL_OUT,
        .pf = NFPROTO_IPV6,
        .priority = NF_IP6_PRI_FIRST,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0))
        .owner = THIS_MODULE
#else
        .priv = (void *)&nf_hks_tcp_priv[2]
#endif
    }, /* net filter OUT hook options */
    {
        .hook = nf_hook_in_out6_tcp,
        .hooknum = NF_INET_LOCAL_IN,
        .pf = NFPROTO_IPV6,
        .priority = NF_IP6_PRI_FILTER - 1,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0))
        .owner = THIS_MODULE
#else
        .priv = (void *)&nf_hks_tcp_priv[3]
#endif
    }, /* net filter IN hook options */
};

#ifdef CONFIG_BCM_XRDP
static int _nf_hook_in_out_udp_set_ref_pkt(struct sk_buff *skb, uint8_t stream_idx, int is_v6)
{
    rdpa_spdtest_ref_pkt_t ref_pkt;
    bdmf_object_handle bdmf_udpspdtest_obj_h;
    BlogAction_t action;
    struct udphdr *uh = udp_hdr(skb);
    uint16_t payload_offset = sizeof(struct udphdr) + ETH_HLEN;
    int udp_len;
    __wsum csum;
    int rc;
    BlogFcArgs_t fc_args={};

    /* Propagate the UDP reference packet from skb->data */
    rc = rdpa_udpspdtest_get(&bdmf_udpspdtest_obj_h);
    if (rc)
    {
        tc_err("Could not retrieve udpspdtest object, rc = %d\n", rc);
        return -1;
    }
    
    /* Recalculate UDP checksum */
    skb->ip_summed = CHECKSUM_NONE;
    if (is_v6)
    {
        struct ipv6hdr *ipv6h = ipv6_hdr(skb);
        int cs_base_buf_len;

        payload_offset += sizeof(struct ipv6hdr);
        udp_len = skb->len - ETH_HLEN - sizeof(struct ipv6hdr);
        uh->check = 0;
        cs_base_buf_len = udp_len - ETH_HLEN - sizeof(struct ipv6hdr);
        if (cs_base_buf_len < 0)
            cs_base_buf_len = udp_len;
        csum = csum_partial(skb_transport_header(skb), cs_base_buf_len, 0);

        uh->check = csum_ipv6_magic(&ipv6h->saddr, &ipv6h->daddr, udp_len, IPPROTO_UDP, csum);
        
        tc_debug("Calculated checksum for len %d = 0x%x, src_addr = %pI6, dst_addr = %pI6, skb_len = %d\n",
            udp_len, htons(uh->check), &(ipv6h->saddr), &(ipv6h->daddr), skb->len);
    }
    else
    {
        struct iphdr *iph = ip_hdr(skb);

        payload_offset += sizeof(struct iphdr);
        uh->check = 0;
        udp_len = skb->len - ETH_HLEN - sizeof(struct iphdr);
        csum = csum_partial(skb_network_header(skb), skb->len - ETH_HLEN, 0);
        uh->check = csum_tcpudp_magic(iph->saddr, iph->daddr, udp_len, IPPROTO_UDP, csum);

        tc_debug("Calculated checksum for len %d = 0x%x, src_addr = %pI4, dst_addr = %pI4, skb->len = %d\n",
            udp_len, htons(uh->check), &(iph->saddr), &(iph->daddr), skb->len);
    }
    if (uh->check == 0)
        uh->check = CSUM_MANGLED_0;
    /* For PPPoE connections, need to mark the checksum as completed */
    skb->ip_summed = CHECKSUM_COMPLETE;

    ref_pkt.data = skb->data;
    ref_pkt.size = skb->len;
    ref_pkt.udp.payload_offset = payload_offset;

    action = blog_sinit(skb, _spdtst_netdev, TYPE_ETH, 0, BLOG_SPDTST, &fc_args);
    tc_debug("blog_sinit() action=%d blog=%px\n", action, skb->blog_p);
    if (skb->blog_p)
    {
        skb->blog_p->iq_prio = BLOG_IQ_PRIO_HIGH;
    }

    rc = rdpa_udpspdtest_ref_pkt_set(bdmf_udpspdtest_obj_h, stream_idx, &ref_pkt);
    bdmf_put(bdmf_udpspdtest_obj_h);
    if (rc)
    {
        print_hex_dump(KERN_INFO, "Could not set reference packet: ", DUMP_PREFIX_NONE, 16, 1, ref_pkt.data, ref_pkt.size, true);
        return -1;
    }
    return 0;
}
#endif

/* Netfilter IPv4/6 In/Out hooks for UDP */
static unsigned int _nf_hook_in_out_udp(uint32_t hooknum, struct sk_buff *skb, const struct nf_hook_state *state, int is_v6)
{
    int8_t stream_idx;
    uint32_t mark = 0;

    /* Discard not valid packets */
    stream_idx = nf_hook_check_valid(skb, 0);
    if (stream_idx < 0)
        return NF_ACCEPT;

    mark |= ((rnr_engine) << 0); 
    mark |= ((rnr_engine) << 1);
    mark |= (1 << 2); 

    if (hooknum == NF_INET_LOCAL_OUT)
    {
        skb_push(skb, ETH_HLEN);
        /* Partially re-use TCP learning code */
        if (tcpspd_engine_hijack_hook_conn_learn(stream_idx, skb->data, 0))
        {
            skb_pull(skb, ETH_HLEN);
            return NF_ACCEPT;
        }

#ifdef CONFIG_BCM_XRDP
        /* Set reference packet and invoke blog_sinit */
        if (_nf_hook_in_out_udp_set_ref_pkt(skb, stream_idx, is_v6))
        {
            return NF_ACCEPT;
        }
#endif

        skb_pull(skb, ETH_HLEN);
        if (skb->blog_p)
            skb->blog_p->spdtst = mark;
        if (skb->sk)
        {
#ifdef CONFIG_BCM_XRDP
            uint32_t so_mark;
            int rc;
            bdmf_object_handle udpspdt_obj;
            rdpa_udpspdtest_cfg_t udpspdtest_cfg = {};

            rc = rdpa_udpspdtest_get(&udpspdt_obj);
            if (rc)
                return NF_ACCEPT;
        
            rc = rdpa_udpspdtest_cfg_get(udpspdt_obj, stream_idx, &udpspdtest_cfg);
            bdmf_put(udpspdt_obj);
            if (rc)
                return NF_ACCEPT;

            switch (udpspdtest_cfg.proto)
            {
            case rdpa_udpspdtest_proto_iperf3:
                skb->recycle_and_rnr_flags |= SKB_RNR_UDPSPDT_IPERF3;
                so_mark = RDPA_UDPSPDTEST_SO_MARK_IPERF3;
                break;
            case rdpa_udpspdtest_proto_basic:
                skb->recycle_and_rnr_flags |= SKB_RNR_UDPSPDT_BASIC;
                so_mark = RDPA_UDPSPDTEST_SO_MARK_BASIC;
                break;
            default:
                return NF_ACCEPT;
            }

            so_mark += stream_idx;
            skbuff_bcm_ext_spdt_set(skb, so_mark, so_mark);
#endif
        }
    }

    return NF_ACCEPT;
}

/* Netfilter IPv4 In/Out hooks for UDP */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0))
static unsigned int nf_hook_in_out_udp(const struct nf_hook_ops *ops, struct sk_buff *skb, const struct nf_hook_state *state)
{
    return _nf_hook_in_out_udp(ops->hooknum, skb, state, 0);
}
/* Netfilter IPv6 In/Out hooks for UDP */
static unsigned int nf_hook_in_out6_udp(const struct nf_hook_ops *ops, struct sk_buff *skb, const struct nf_hook_state *state)
{
    return _nf_hook_in_out_udp(ops->hooknum, skb, state, 1);
}
#else
static unsigned int nf_hook_in_out_udp(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    return _nf_hook_in_out_udp(*(uint32_t *)priv, skb, state, 0);
}
/* Netfilter IPv6 In/Out hooks for UDP */
static unsigned int nf_hook_in_out6_udp(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    return _nf_hook_in_out_udp(*(uint32_t *)priv, skb, state, 1);
}
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
static uint32_t nf_hks_udp_priv[] =  {
    NF_INET_LOCAL_OUT,
    NF_INET_LOCAL_IN,
    NF_INET_LOCAL_OUT,
    NF_INET_LOCAL_IN
};
#endif

/* Netfilter declare hooks for UDP */
static struct nf_hook_ops nf_hks_udp[] = {
    {
        .hook = nf_hook_in_out_udp,
        .hooknum = NF_INET_LOCAL_OUT,
        .pf = NFPROTO_IPV4,
        .priority = NF_IP_PRI_LAST,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0))
        .owner = THIS_MODULE
#else
        .priv = (void *)&nf_hks_udp_priv[0]
#endif
    }, /* net filter OUT hook options */
    {
        .hook = nf_hook_in_out_udp,
        .hooknum = NF_INET_LOCAL_IN,
        .pf = NFPROTO_IPV4,
        .priority = NF_IP_PRI_LAST,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0))
        .owner = THIS_MODULE
#else
        .priv = (void *)&nf_hks_udp_priv[1]
#endif
    }, /* net filter IN hook options */
    {
        .hook = nf_hook_in_out6_udp,
        .hooknum = NF_INET_LOCAL_OUT,
        .pf = NFPROTO_IPV6,
        .priority = NF_IP6_PRI_LAST,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0))
        .owner = THIS_MODULE
#else
        .priv = (void *)&nf_hks_udp_priv[2]
#endif
    }, /* net filter OUT hook options */
    {
        .hook = nf_hook_in_out6_udp,
        .hooknum = NF_INET_LOCAL_IN,
        .pf = NFPROTO_IPV6,
        .priority = NF_IP6_PRI_LAST,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0))
        .owner = THIS_MODULE
#else
        .priv = (void *)&nf_hks_udp_priv[3]
#endif
    }, /* net filter IN hook options */
};

int udpspd_sock_mgr_init(void)
{
    int i;

    /* Register nf hooks */
    for (i = 0; i < sizeof(nf_hks_udp) / sizeof(struct nf_hook_ops); i++)
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0))
        nf_register_hook(&nf_hks_udp[i]);
#else
        /* Inherit the network namespace from the userspace app. */
        nf_register_net_hook(current->nsproxy->net_ns, &nf_hks_udp[i]);
#endif
    return 0;
}

int udpspd_sock_mgr_uninit(void)
{
    int i;

    /* Register nf hooks */
    for (i = 0; i < sizeof(nf_hks_udp) / sizeof(struct nf_hook_ops); i++)
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0))
        nf_unregister_hook(&nf_hks_udp[i]);
#else
        /* Inherit the network namespace from the userspace app. */
        nf_unregister_net_hook(current->nsproxy->net_ns, &nf_hks_udp[i]);
#endif
    return 0;
}

/* Socket Connect to Server */
int tcpspd_sock_mgr_connect(uint8_t stream_idx, struct socket **sock, spdt_conn_params_t *params)
{
    int i, option, rc;
    int streams;

    streams = atomic_inc_return(&connected_streams);
    if (streams == 1)
    {
        tcpspeedtest_on_set(1, stream_idx);

        /* Register nf hooks */
        for (i = 0; i < sizeof(nf_hks_tcp) / sizeof(struct nf_hook_ops); i++)
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0))
            nf_register_hook(&nf_hks_tcp[i]);
#else
            /* Inherit the network namespace from the userspace app. */
            nf_register_net_hook(current->nsproxy->net_ns, &nf_hks_tcp[i]);
#endif
    }

    /* Create Kernel Socket */
    rc = sock_create(params->server_addr.ss_family, SOCK_STREAM, IPPROTO_TCP, sock);
    if (rc)
    {
        tc_err("[%hhu] Failed to create kernel socket\n", stream_idx);
        goto rel_sock;
    }

    /* Configure Socket options */
    option = 1;
    rc = kernel_setsockopt(*sock, SOL_SOCKET, SO_REUSEADDR, (char *)&option, sizeof(option));
    if (rc < 0)
    {
        tc_err("[%hhu] kernel_setsockopt() SO_REUSEADDR Failure, rc:%d\n", stream_idx, rc);
        goto rel_sock;
    }

    option = 1;
    rc = kernel_setsockopt(*sock, SOL_TCP, TCP_NODELAY, (char *) &option, sizeof(option));
    if (rc < 0)
    {
        tc_err("[%hhu] kernel_setsockopt() TCP_NODELAY Failure, rc:%d\n", stream_idx, rc);
        goto rel_sock;
    }

    option = 0;
    rc = kernel_setsockopt(*sock, SOL_SOCKET, SO_TIMESTAMP, (char *) &option, sizeof(option));
    if (rc < 0)
    {
        tc_err("[%hhu] kernel_setsockopt() SO_TIMESTAMP Failure, rc:%d\n", stream_idx, rc);
        goto rel_sock;
    }

    option = (params->server_addr.ss_family == AF_INET ? TCP_MAX_MSS : TCP_MAX_MSS - 20);
    rc = kernel_setsockopt(*sock, IPPROTO_TCP, TCP_MAXSEG, (char *)&option, sizeof(option));
    if (rc < 0)
    {
        tc_err("[%hhu] kernel_setsockopt() TCP_MAXSEG Failure, rc:%d\n", stream_idx, rc);
        goto rel_sock;
    }

    if (params->tos)
    {
        option = params->tos;
        if (params->server_addr.ss_family == AF_INET)
        {
            rc = kernel_setsockopt(*sock, IPPROTO_IP, IP_TOS, (char *)&option, sizeof(option));
            if (rc < 0)
            {
                tc_err("[%hhu] kernel_setsockopt() IP_TOS Failure tos:%u, rc:%d\n", stream_idx, params->tos, rc);
                goto rel_sock;
            }
        }
        else
        {
            rc = kernel_setsockopt(*sock, IPPROTO_IPV6, IPV6_TCLASS, (char *)&option, sizeof(option));
            if (rc < 0)
            {
                tc_err("[%hhu] kernel_setsockopt() IPV6_TCLASS Failure tos:%u, rc:%d\n", stream_idx, params->tos, rc);
                goto rel_sock;
            }
        }
    }
 
#if 0
    option = 0;
    rc = kernel_setsockopt(*sock, SOL_TCP, SO_KEEPALIVE, (char *) &option, sizeof(option));
    if (rc < 0)
    {
        tc_err("kernel_setsockopt() SO_KEEPALIVE Failure, rc:%d\n", rc);
        goto rel_sock;
    }
#endif
    tc_debug("[%hhu] tcpspdtest: local socket created\n", stream_idx);

    /* Bind to local address */
    if (params->local_addr.ss_family)
    {
        rc = kernel_bind(*sock, (struct sockaddr *)&params->local_addr, sizeof(params->local_addr));
        if (rc < 0)
        {
            tc_err("[%hhu] kernel_bind() Failure, rc:%d\n", stream_idx, rc);
            goto rel_sock;
        }
        tc_debug("[%hhu] tcpspdtest: bound local socket to %pIScp\n", stream_idx, (struct sockaddr *)&params->local_addr);
    }

    sock_learned[stream_idx] = 0;
    g_tcpspd[stream_idx].add_flow = 0;

    rc = kernel_connect(*sock, (struct sockaddr *)&params->server_addr, sizeof(params->server_addr), 0);
    if (rc < 0)
    {
        tc_err("[%hhu] Connect to server:%pISpc Failed ,rc:%d\n", stream_idx, &params->server_addr, rc);
        goto rel_sock;
    }

    rc = tcpspd_engine_is_sock_learned(stream_idx);
    if (!rc)
    {
        tc_err("[%hhu] Failed to learn connection.\n", stream_idx);
        goto rel_sock;
    }

    option = sizeof(struct tcp_info);  
    rc = kernel_getsockopt(*sock, SOL_TCP, TCP_INFO, (char *)&sock_tcpinfo[stream_idx], &option);
    if (rc < 0)
    {
        tc_err("[%hhu] kernel_getsockopt() TCP_INFO Failure, rc:%d\n", stream_idx, rc);
        goto rel_sock;
    }

    if (sock_tcpinfo[stream_idx].tcpi_options & TCPI_OPT_SACK)
    	tcpspd_engine_set_sack(stream_idx);

    tcpspd_engine_set_mss(stream_idx, sock_tcpinfo[stream_idx].tcpi_snd_mss, 
                            sock_tcpinfo[stream_idx].tcpi_advmss);
    tcpspd_engine_set_rtt(stream_idx, sock_tcpinfo[stream_idx].tcpi_rtt);
    tcpspd_engine_set_rwnd(stream_idx, g_tcpspd[stream_idx].rwnd_bytes, sock_tcpinfo[stream_idx].tcpi_rtt, sock_tcpinfo[stream_idx].tcpi_rcv_wscale, g_tcpspd[stream_idx].rate_Mbps);

    return 0;

rel_sock:
    tcpspd_sock_mgr_release(stream_idx, *sock);
    *sock = NULL;
    return -1;
}

/* Send Socket Kernel message to the Server */
int tcpspd_sock_mgr_sendmsg(uint8_t stream_idx, uint8_t *data, uint32_t len)
{
    struct msghdr message = {};
    struct kvec ioVector = {};
    int rc;

    /* Prepare the message */
    ioVector.iov_base = (void *)data;
    ioVector.iov_len = len;
    message.msg_flags = MSG_NOSIGNAL;

    /* Send the message */
    rc = kernel_sendmsg(g_tcpspd[stream_idx].srv_socket, &message, &ioVector, 1, ioVector.iov_len);
    if (rc != ioVector.iov_len)
    {
        tc_err("[%hhu] kernel_sendmsg() Failure, rc:%d\n", stream_idx, rc);
        return -1;
    }

    return 0;
}

/* Wait for send operation to be fully Acked by the Server */
int tcpspd_sock_mgr_wait_send_complete(uint8_t stream_idx, struct socket *sock)
{
    int opt_size = sizeof(struct tcp_info);
    int cnt = WAIT_LOOP;
    int rc;

    do
    {
        mdelay(WAIT_MSEC);
        rc = kernel_getsockopt(sock, SOL_TCP, TCP_INFO, (char *) &sock_tcpinfo[stream_idx], &opt_size);
        if (rc < 0)
        {
            tc_err("[%hhu] kernel_getsockopt() Failure, rc:%d\n", stream_idx, rc);
            return -1;
        }
    }
    while (cnt-- && sock_tcpinfo[stream_idx].tcpi_unacked && TCP_ESTABLISHED == sock_tcpinfo[stream_idx].tcpi_state);

    if (cnt <= 0 || TCP_ESTABLISHED != sock_tcpinfo[stream_idx].tcpi_state)
        return -1;

    return 0;
}

/* Start/Stop hijack nf_hook out/in packets for prtcl processing */
void tcpspd_sock_mgr_set_nf_hook_prtcl(uint8_t stream_idx, int accept)
{
    atomic_set(&nf_hook_prtcl[stream_idx], accept);
}

/* Socket manager disconnect, shutdown the connection, delete runner flows */
int tcpspd_sock_mgr_disconnect(uint8_t stream_idx, struct socket *sock)
{
    tc_debug("[%hhu] TCPSPDTEST: Disconnecting from server\n", stream_idx);

    if (NULL == sock)
    {
        tc_debug("[%hhu] Socket already released!\n", stream_idx);
        return 0;
    }

    if (SS_CONNECTED != g_tcpspd[stream_idx].srv_socket->state)
    {
        tc_err("[%hhu] Try to disconnect socket (state=%d) not in CONNECTED state\n", stream_idx,
            g_tcpspd[stream_idx].srv_socket->state);
        return -1;
    }

    /* Don't create RX flow */
    g_tcpspd[stream_idx].add_flow = 0;

    /* Engine down, delete runner flows */
    tcpspd_engine_down(stream_idx);

    /* Shutdown the connection */
    return kernel_sock_shutdown(sock, SHUT_RDWR); /* no data rx/tx is expected */
}

/* Release Socket, unregister netfilter hooks */
int tcpspd_sock_mgr_release(uint8_t stream_idx, struct socket *sock)
{
    int i, rc = 0;
    int streams;

    tc_debug("[%hhu] TCPSPDTEST: Release socket\n", stream_idx);

    if (NULL == sock)
    {
        tc_debug("[%hhu] Socket already released!\n", stream_idx);
        return 0;
    }

    if (SS_CONNECTED != g_tcpspd[stream_idx].srv_socket->state)
    {
        tc_err("[%hhu] Try to release socket (state=%d) not in CONNECTED state! Release anyway.\n",
            stream_idx, g_tcpspd[stream_idx].srv_socket->state);
        rc = -1;
    }

    sock_learned[stream_idx] = 0;
    g_tcpspd[stream_idx].srv_socket = NULL;

    /* Kernel Socket release */
    sock_release(sock);

    streams = atomic_dec_return(&connected_streams);
    if (streams == 0)
    {
        tcpspeedtest_on_set(0, stream_idx);

        /* Unregister netfilter hooks */
        for (i = 0; i < sizeof(nf_hks_tcp) / sizeof(struct nf_hook_ops); i++)
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0))
            nf_unregister_hook(&nf_hks_tcp[i]);
#else
            /* Inherit the network namespace from the userspace app. */
            nf_unregister_net_hook(current->nsproxy->net_ns, &nf_hks_tcp[i]);
#endif
    }
    return rc;
}

static int spdt_rnr_transmit_check(void *arg_p)
{
    spdsvcHook_transmit_t *transmit_p = arg_p;
    pNBuff_t *pNBuff = (pNBuff_t)transmit_p->pNBuff;
    void *pBuf = PNBUFF_2_PBUF(pNBuff);

    if (!pBuf)
        return 0;

#ifdef CONFIG_BCM_XRDP
    if (IS_SKBUFF_PTR(pNBuff))
    {
        struct sk_buff *skb = PNBUFF_2_SKBUFF(pNBuff);

        if ((skb->recycle_and_rnr_flags & SKB_RNR_UDPSPDT_FLAGS) != 0)
        {
            transmit_p->so_mark = skbuff_bcm_ext_spdt_get(skb, so_mark);

            /* XXX: Actually, we should always use this trigger instead of the enet driver. But until the Speed Service
             * is not rewritten, we cannot completely move to it, so we temporarly use this flag. */
            if (transmit_p->flags & SPDSVC_HOOK_TRANSMIT_SPDT_NO_AUTO_TRIGGER)
            {
                /* We want to return 0, b/c blog_emit is invoked later */
                return 0;
            }
            return 1;
        }
    }
    else if (IS_FKBUFF_PTR(pNBuff))
    {
        FkBuff_t* fkb = PNBUFF_2_FKBUFF(pNBuff);

        if (fkb->spdtst)
        {
            transmit_p->so_mark = RDPA_TCPSPDTEST_SO_MARK;
            return 1;
        }
    }
#endif
    return 0;
}

/* Socket manager init */
int tcpspd_sock_mgr_init(void)
{
    int ret = 0;

    _spdtst_netdev = alloc_netdev(0, "spdt%d", NET_NAME_UNKNOWN, __sock_mgr_netdev_setup);

    if (_spdtst_netdev)
    {
        ret = register_netdev(_spdtst_netdev);
        if (ret)
            free_netdev(_spdtst_netdev);
    }
    else
    {
        ret = (-ENOMEM);
    }

    bcmFun_reg(BCM_FUN_ID_SPDT_RNR_TRANSMIT, spdt_rnr_transmit_check);

    bcm_netdev_ext_field_set(_spdtst_netdev, bcm_netdev_cb_fn, bcm_netdev_def_cpu_port_obj_get);

    return ret;
}

/* Shutdown the socket manager */
int tcpspd_sock_mgr_shutdown(uint8_t stream_idx)
{
    bcmFun_dereg(BCM_FUN_ID_SPDT_RNR_TRANSMIT);

    if (g_tcpspd[stream_idx].srv_socket)
    {
        /* Release Socket, unregister net filter hooks */
        tcpspd_sock_mgr_release(stream_idx, g_tcpspd[stream_idx].srv_socket);
        g_tcpspd[stream_idx].srv_socket = NULL;
    }

    if (_spdtst_netdev)
    {
        unregister_netdev(_spdtst_netdev);
        free_netdev(_spdtst_netdev);
        _spdtst_netdev = 0;
    }

    return 0;
}
