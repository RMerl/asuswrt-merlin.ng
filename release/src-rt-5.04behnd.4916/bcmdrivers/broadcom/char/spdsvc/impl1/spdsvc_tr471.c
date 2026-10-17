/*
<:copyright-BRCM:2022:DUAL/GPL:standard

   Copyright (c) 2022 Broadcom 
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
* File Name  : spdsvc_tr471.c
*
* Description: This file contains Linux specific interface implementation
*              of TR-471 in the speed service acceleration support driver.
*
*******************************************************************************
*/

#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_ipv6.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/ip6_checksum.h>
#include <linux/module.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <linux/skbuff.h>
#include <linux/nbuff.h>
#include <linux/bcm_log.h>
#include <linux/kthread.h>
#include <linux/gbpm.h>
#include "spdsvc.h"
#include "spdsvc_tr471.h"
#if defined(CONFIG_BCM_RDPA) || defined(CONFIG_BCM_RDPA_MODULE)
#include "rdpa_drv.h"
#endif
#include "bcmnet.h"
#include "spdsvc_procfs.h"

#if defined(CONFIG_BCM_RDPA) || defined(CONFIG_BCM_RDPA_MODULE)
#if defined(CC_SPDSVC_TR471_FLOW_BASED_HW_SUPPORT)
#if !IS_ENABLED(CONFIG_BCM_SPDTEST)
#error "CONFIG_BCM_SPDTEST must be enabled on certain platforms when enabling SPDSVC + TR471"
#endif
#endif
#endif

spdsvc_tr471_t _spdsvc_tr471_state_g;

/* Generator thread data elements */
typedef struct {
    struct task_struct *thread;
    wait_queue_head_t threadWqh;
    int workAvail;
    _tr471_thread_mode_t thread_mode;
    int rx_recycle_thread_data;
} _spdsvc_tr471_gen_and_recycle_thread_t;

static _spdsvc_tr471_gen_and_recycle_thread_t _spdsvc_tr471_gen_and_recycle_thread_g;

/***************************************************************************
 * TR-471 Tx/Rx Checksum Support functions
 **************************************************************************/

static inline uint16_t spdsvc_tr471_loadhdr_checksum_no_mangle (struct loadHdr *lhdr)
{
    uint16_t csum16_calc = 0;

    _compute_icsum_buffer((uint16_t*)lhdr, sizeof(struct loadHdr), &csum16_calc);
    return (csum16_calc);
}

/*******************************************************************************
 *
 * TR-471 Receive Recycle Queue
 *
 *******************************************************************************/

static int spdsvc_tr471_rx_recycle_queue_construct(void)
{
    int ret = 0;

    int rx_recycle_queue_entry_size = sizeof(spdsvc_tr471_rx_recycle_queue_entry_t);

    rx_recycle_queue_entry_size = (rx_recycle_queue_entry_size + 0x3) & ~3; /* Make entry 4-byte multiple */
    if ((ret = bcm_async_queue_init(&_spdsvc_tr471_state_g.spdsvc_tr471_rx_recycle_queue, SPDSVC_TR471_RX_RECYCLE_QUEUE_SIZE,
                                    rx_recycle_queue_entry_size)) != 0)
    {
        __logError("\n\tSPDSVC Rx Recycle Queue Init Failed Q Size- %d\n", SPDSVC_TR471_RX_RECYCLE_QUEUE_SIZE);
    }
    else {
        SPDSVCLOG_DEBUG("\n\tSPDSVC Rx Recycle Queue Initialized Q Size- %d\n", SPDSVC_TR471_RX_RECYCLE_QUEUE_SIZE);
    }

    return ret;
}

static int spdsvc_tr471_rx_recycle_queue_write(spdsvc_tr471_rx_recycle_queue_entry_t *entry_p)
{
    bcm_async_queue_t *queue_p = &_spdsvc_tr471_state_g.spdsvc_tr471_rx_recycle_queue;

    if (bcm_async_queue_not_full(queue_p))
    {
        spdsvc_tr471_rx_recycle_queue_entry_t *queue_entry_p = (spdsvc_tr471_rx_recycle_queue_entry_t *)
                                                               bcm_async_queue_entry_write(queue_p);

        *queue_entry_p = *entry_p;
        bcm_async_queue_entry_enqueue(queue_p);
        queue_p->stats.writes++;
        return 1;
    }
    else
    {
        queue_p->stats.discards++;
        return 0;
    }
}

inline spdsvc_tr471_rx_recycle_queue_entry_t* spdsvc_tr471_rx_recycle_queue_read(void)
{
    bcm_async_queue_t *queue_p = &_spdsvc_tr471_state_g.spdsvc_tr471_rx_recycle_queue;

    if (bcm_async_queue_not_empty(queue_p))
    {
        return (spdsvc_tr471_rx_recycle_queue_entry_t *)
               bcm_async_queue_entry_read(queue_p);
    }

    return NULL;
}

static inline void spdsvc_tr471_rx_recycle_queue_post(void)
{
    bcm_async_queue_t *queue_p = &_spdsvc_tr471_state_g.spdsvc_tr471_rx_recycle_queue;
    bcm_async_queue_entry_dequeue(queue_p);
    queue_p->stats.reads++;
}

static void spdsvc_tr471_rx_recycle_queue_destruct(void)
{
    bcm_async_queue_uninit(&_spdsvc_tr471_state_g.spdsvc_tr471_rx_recycle_queue);
}

/***************************************************************************
 * Utility functions
 **************************************************************************/
/* Function to calaculate the UDP checksum for bytes upto TR471-load-header,
   assuming remaining bytes are zeros */
static __sum16 spdsvc_tr471_calc_csum(uint8_t *data_p, int l4_len, int l3_offset, 
                                      int l4_offset, int is_v6)
{
    struct udphdr *uh;
    __sum16 save_csum;
    __sum16 calc_csum;

    uh = (struct udphdr *)(data_p + l4_offset);

    /* Ideally, we could use below function but it will be unnecessary
       to do checksum of zeros in payload */
    /* skb_checksum_help(skb); */

    save_csum = uh->check;
    uh->check = 0;

    if (!is_v6)
    {
        struct iphdr *ipv4 = (struct iphdr *)(data_p + l3_offset);
        calc_csum = csum_tcpudp_magic(ipv4->saddr, ipv4->daddr, 
                                      l4_len, IPPROTO_UDP, 
                                      csum_partial(uh, sizeof(BlogUdpHdr_t) + sizeof(struct loadHdr), 0));
    }
    else
    {
        struct ipv6hdr *ipv6 = (struct ipv6hdr *)(data_p + l3_offset);
        calc_csum = csum_ipv6_magic(&ipv6->saddr, &ipv6->daddr, 
                                    l4_len, IPPROTO_UDP, 
                                    csum_partial(uh, sizeof(BlogUdpHdr_t) + sizeof(struct loadHdr), 0));
    }

    uh->check = save_csum;

    if (!calc_csum)
        calc_csum = CSUM_MANGLED_0;

    return calc_csum;
}

int spdsvc_tr471_store_ref_pkt_info(int connindex, void *skb_, spdsvc_tr471_ref_pkt_info_t *ref_info_p)
{
    struct sk_buff *skb = skb_;
    FkBuff_t *fkb_p;

    if (ref_info_p->fkb_p)
    {
        __logError("TR471Err: ref_pkt not NULL, possible memory/SKB leak, freeing fkb ... conn %d", connindex);
        nbuff_free(FKBUFF_2_PNBUFF(ref_info_p->fkb_p));
        ref_info_p->fkb_p = NULL;
        TR471_PROCFS_INC(connindex, error_cnt);
    }

    if (unlikely(skb_is_gso(skb) || skb_shinfo(skb)->nr_frags || skb_cloned(skb)))
    {
        __logError("ERROR - GSO/Frags/cloned not supported ... conn %d", connindex);
        /* Error count incremented by caller */
        return -1;
    }

    if (unlikely(skb_is_nonlinear(skb)))
    {
        if (__skb_linearize(skb))
        {
            __logError("ERROR - failed to linearize ... conn %d", connindex);
            /* Error count incremented by caller */
            return -1;
        }
    }

    if (skb->ip_summed == CHECKSUM_PARTIAL)
    {
        struct udphdr *uh;
        uh = (struct udphdr *)(skb->data + skb_transport_offset(skb));
        uh->check = spdsvc_tr471_calc_csum(skb->data, skb->len - skb_transport_offset(skb), 
                                             skb_network_offset(skb), 
                                             skb_transport_offset(skb),
                                             _spdsvc_tr471_state_g.is_v6);
    }

    fkb_p = spdsvc_buff_alloc(connindex);
    if (fkb_p)
    {
        /* populate fields from skb into tr471 struct */
        ref_info_p->fkb_p = fkb_p;
        ref_info_p->data = fkb_p->data;
        /* Assumption : SKB is linear */
        memcpy(ref_info_p->data, skb->data, skb->len);
        memset(ref_info_p->data + skb->len, 0, BCM_MAX_PKT_LEN - skb->len);
        ref_info_p->len = skb->len;
        ref_info_p->l3_offset = skb_network_offset(skb);
        ref_info_p->l4_offset = skb_transport_offset(skb);
        ref_info_p->lHdr_offset = ref_info_p->l4_offset + sizeof(BlogUdpHdr_t);
        TR471_PROCFS_INC(connindex, pkt_store_cnt);
        return 0;
    }
    return -1;
}

char* spdsvc_tr471_get_state(int connindex)
{
    switch (_spdsvc_tr471_state_g.flowinfo[connindex].flow_state)
    {
        case TR471_FLOW_STATE_INACTIVE:
            return "INACTIVE";

        case TR471_FLOW_STATE_ACTIVE_SW:
            return "SW_FLOW";

        case TR471_FLOW_STATE_ACTIVE_HW:
            return "HW_FLOW";

        default:
            return "INVALID";
    }
}
/***************************************************************************
 * Speed Service TR471 helper function
 * Used to transfer the load header to TR471 application.
 **************************************************************************/
static inline void _spdsvc_tr471_fwd_loadHdr_RecycleBuff(int connindex, pNBuff_t pNBuf, int recycle_action, int isFkb)
{
    unsigned char *data_p;
    BlogIpv4Hdr_t *ipv4_p;
    BlogIpv6Hdr_t *ipv6_p;
    BlogUdpHdr_t *udp_p = NULL;
    spdsvc_tr471_rx_recycle_queue_entry_t tr471_rx_recycle_q_entry;
    int          local_rel = 0;
    unsigned long irq_flags;
    int cksum_verified;

    if (likely(isFkb)) /* Optimize the condition for FKB path, which is the most occurring case */
    {
        FkBuff_t *fkb = PNBUFF_2_FKBUFF(pNBuf);
        data_p = fkb->data;
        cksum_verified = fkb->rx_csum_verified;
    }
    else /* skb */
    {
        struct sk_buff *skb = PNBUFF_2_SKBUFF(pNBuf);
        data_p = skb->data;
        cksum_verified = skb->ip_summed ? 1 : 0;
    }

    ipv4_p = (BlogIpv4Hdr_t *)data_p;
    ipv6_p = (BlogIpv6Hdr_t *)data_p;

    /*
     * We could check the IP version field or just use the TR471 state info 
     */
    if (!_spdsvc_tr471_state_g.is_v6)
    { /* IPV4  - Optimize the check for IPV4 which will mostly be true since we are in IPV4 world */

        int ihl = ipv4_p->ihl << 2;
        udp_p = (BlogUdpHdr_t *)((uint8_t *)(ipv4_p)+ihl);
    }
    else
    { /* IPV6 */

        /* TODO: We should find the UDP header by skipping other headers
         * We may want to use xmit args to get the L3/L4 offsets
         * Another way would be to store the L4 offset during learning */
        if (ipv6_p->nextHdr == BLOG_IPPROTO_UDP)
        {
            /* Unfragmented UDP packet */
            udp_p = (BlogUdpHdr_t *)(ipv6_p + 1);
        }
        else if (ipv6_p->nextHdr == BLOG_IPPROTO_FRAGMENT)
        {
            /* Fragmented packet */
            BlogIpv6ExtHdr_t *ipv6_ext_hdr_p = (BlogIpv6ExtHdr_t *)(ipv6_p + 1);
            if (ipv6_ext_hdr_p->nextHdr == BLOG_IPPROTO_UDP)
            {
                /* First fragment of UDP packet */
                udp_p = (BlogUdpHdr_t *)(ipv6_ext_hdr_p + 1);
            }
        }
    } /* IpV6 */

    if (udp_p != NULL)
    {
        /* Verify LoadHdr checksum for integrity, as required on the received network PDU. */
        struct loadHdr *lHdr_p = (struct loadHdr *)(udp_p + 1);

#if defined(CONFIG_BCM_TR471_MFLOW)
        {
            uint16_t csum16_calc;

            if (lHdr_p->checkSumOrRsvd !=0) { /* This check is forward/backward compatible */
                if (!cksum_verified)
                {
                    /* Re-verify if bad or not verified */
                    csum16_calc = spdsvc_tr471_loadhdr_checksum_no_mangle(lHdr_p);
                    if (csum16_calc) {
                        __logNotice("TR471Err: Loadheader checksum error. Conn-%d, CalcCsum 0x%x, PacketCsum 0x%x \n",
                           connindex, csum16_calc, lHdr_p->checkSumOrRsvd);
                        TR471_PROCFS_INC(connindex, error_pkt_cnt);
                        local_rel = 1;
                        goto _End;
                    }
                }
            } /* if LoadHdr checksum, required. */
        }
#endif

        tr471_rx_recycle_q_entry.lhdr_p = lHdr_p;
        tr471_rx_recycle_q_entry.pRxNBuf = pNBuf;
        tr471_rx_recycle_q_entry.action = recycle_action;
        tr471_rx_recycle_q_entry.connindex = connindex;

        /* Lock critical section to access the 
         * 1) Loadhdr forward.
         * 2) post buffer free after forward.
         */

        spin_lock_irqsave(&_spdsvc_tr471_state_g.spdsvc_lock, irq_flags);
        if (spdsvc_tr471_rx_recycle_queue_write(&tr471_rx_recycle_q_entry))
        {
            if ((++_spdsvc_tr471_state_g.rx_recycle_budget >= _spdsvc_tr471_state_g.rx_conf_recycle_budget) && (!_spdsvc_tr471_gen_and_recycle_thread_g.workAvail))
            {
                _spdsvc_tr471_gen_and_recycle_thread_g.thread_mode = TR471_THREAD_MODE_FWD_N_RECYCLE_RX_BUFF;
                _spdsvc_tr471_gen_and_recycle_thread_g.rx_recycle_thread_data = SPDSVC_EVENT_SIGNAL_BATCH_MODE;
                _spdsvc_tr471_gen_and_recycle_thread_g.workAvail = 1;
                wake_up_interruptible(&(_spdsvc_tr471_gen_and_recycle_thread_g.threadWqh));
                _spdsvc_tr471_state_g.rx_recycle_budget = 0;
            }
        }
        else
        {
            /* if we cannot forward, we drop the data (data loss) */
            local_rel = 1;
        }

        spin_unlock_irqrestore(&_spdsvc_tr471_state_g.spdsvc_lock, irq_flags);

        if (_spdsvc_tr471_state_g.rx_conf_recycle_budget == SPDSVC_RX_MIN_RECYCLE_BUDGET) {

            bcm_async_queue_t *queue_p = &_spdsvc_tr471_state_g.spdsvc_tr471_rx_recycle_queue;

            if (bcm_async_queue_avail_entries(queue_p) > SPDSVC_RX_MIN_RECYCLE_BUDGET) {
                _spdsvc_tr471_state_g.rx_conf_recycle_budget = SPDSVC_RX_MAX_RECYCLE_BUDGET;
            }
        }
    } /* udp not null */
    else
    {
        SPDSVCLOG_INFO("TR471Err: UDP null conn %d\n", connindex);
        TR471_PROCFS_INC(connindex, error_pkt_cnt);
        local_rel = 1;
    }

#if defined(CONFIG_BCM_TR471_MFLOW)
_End:
#endif
    if (local_rel) /* error condition, unexpected */
    {
        TR471_PROCFS_INC(connindex, rx_buf_local_recycle);
        if (recycle_action != SPDSVC_RELEASE)
            nbuff_invalidatefree(pNBuf);
        else
            nbuff_free(pNBuf);
    }
}

/***************************************************************************
 * Speed Service Netdevice
 * Used for GDX (RX) and blog_init (TX) calls.
 **************************************************************************/

static int _spdsvc_tr471_netdev_recv(pNBuff_t pNBuf, BlogFcArgs_t *fc_args)
{
    int connindex;
    spdsvc_conn_t *conn_p;
    int isFkb = 1; /* optimized for fkb path */

    if (IS_FKBUFF_PTR(pNBuf))
    {
        FkBuff_t *fkb = PNBUFF_2_FKBUFF(pNBuf);
        connindex = fkb->mark;
    }
    else //skb.
    {
        struct sk_buff *skb = PNBUFF_2_SKBUFF(pNBuf);
        connindex = skb->mark;
        isFkb = 0 ;
    }

    if ((connindex < SPDSVC_DEF_STD_CONN) || (connindex >= SPDSVC_MAX_CONN))
    {
        SPDSVCLOG_INFO("TR471Err: Invalid Conn(%d) recvd pkt\n");
        nbuff_free(pNBuf);
        TR471_PROCFS_INC(SPDSVC_DEF_STD_CONN, error_cnt); /* ToDo. Make it global */
        return 0;
    }

    conn_p = (spdsvc_conn_t *)&spdsvc_g.connctx[connindex];

    TR471_PROCFS_INC(connindex, netdev_rx_cnt);
    if (conn_p->state != SPDSVC_STATE_ANALYZER)
    {
        SPDSVCLOG_INFO("Conn(%d)state(%d) recvd pkt\n", connindex, conn_p->state);
        nbuff_free(pNBuf);
        TR471_PROCFS_INC(connindex, error_cnt);
        return 0;
    }

    _spdsvc_tr471_fwd_loadHdr_RecycleBuff(connindex, pNBuf, SPDSVC_RELEASE_INVALIDATE, isFkb);

    return 0;
}

static const struct net_device_ops _spdsvc_tr471_netdev_ops = {
    .ndo_open               = NULL,
    .ndo_stop               = NULL,
    .ndo_start_xmit         = (HardStartXmitFuncP)_spdsvc_tr471_netdev_recv,
    .ndo_set_mac_address    = NULL,
    .ndo_do_ioctl           = NULL,
    .ndo_tx_timeout         = NULL,
    .ndo_get_stats          = NULL,
    .ndo_change_mtu         = NULL
};

static DEFINE_PER_CPU(int, _spdsvc_tr471_cpu_refcnt);

static struct net_device *_spdsvc_tr471_netdev = NULL;

static void _spdsvc_tr471_netdev_setup(struct net_device *dev)
{
    dev->netdev_ops     = &_spdsvc_tr471_netdev_ops;
    dev->mtu            = BCM_MAX_MTU_PAYLOAD_SIZE;
    dev->pcpu_refcnt    = &_spdsvc_tr471_cpu_refcnt;
    dev->type = ARPHRD_RAWIP;
}

/* Delay in ms */
static void spdsvc_tr471_delay(UINT32 ulTimeout)
{
    wait_queue_head_t wait;

    /* Convert ms to jiffies.  If the timeout is less than the granularity of
     * the system clock, wait one jiffy.
     */

    if ((ulTimeout = (ulTimeout * HZ) / 1000) == 0)
        ulTimeout = 1;

    init_waitqueue_head(&wait);
    wait_event_interruptible_timeout(wait, FALSE, ulTimeout);  //A condition that always evaluated to FALSE so it sleeps until the timeout is elapsed.
}

/***************************************************************************
 * TR-471 TX Generator and Recycle Thread
 **************************************************************************/

void spdsvc_tr471_gen_start(int connindex)
{
    spdsvc_conn_t *conn_p = (spdsvc_conn_t *)&spdsvc_g.connctx[connindex];

    /* Software generation can only be started in following states:
    SPDSVC_STATE_GENERATOR_FCWAIT = First burst, when waiting for flow to be learnt
    SPDSVC_STATE_GENERATOR_DONE = Subsequent burst, when last burst was done i.e. completed (successfully/failure)*/
    if (!(conn_p->state == SPDSVC_STATE_GENERATOR_DONE || conn_p->state == SPDSVC_STATE_GENERATOR_FCWAIT))
    {
        TR471_PROCFS_INC(connindex, error_cnt);
        __logError("TR471 software generator (connindex = %d) called in invalid state = %d \n", connindex, conn_p->state);
        return;
    }

    /* Record start of sw burst gen */
    spdsvc_record_time(tr471_burst_gen_start_time_ns);

    _spdsvc_tr471_gen_and_recycle_thread_g.thread_mode = TR471_THREAD_MODE_SW_GEN;
    conn_p->state = SPDSVC_STATE_GENERATOR_RUNNING_SW;
    _spdsvc_tr471_gen_and_recycle_thread_g.workAvail = 1;
    wake_up_interruptible(&(_spdsvc_tr471_gen_and_recycle_thread_g.threadWqh));
}

void spdsvc_hw_tr471_tx_complete_cb(int connindex)
{
    spdsvc_conn_t *conn_p = (spdsvc_conn_t *)&spdsvc_g.connctx[connindex];

    /* Record cmpl of hw burst gen */
    spdsvc_record_time(tr471_burst_gen_cmpl_time_ns);
    /* Record time taken by hw to generate burst */
    tr471_record_pktgen_time(tr471_burst_gen_start_time_ns, tr471_burst_gen_cmpl_time_ns);

    if ((connindex == spdsvc_g.active_us_connidx) && conn_p->state == (SPDSVC_STATE_GENERATOR_RUNNING_HW))
    {
        TR471_PROCFS_INC(connindex, hw_gen_cmpl_cnt);
        _spdsvc_tr471_gen_and_recycle_thread_g.thread_mode = TR471_THREAD_MODE_HW_DONE;
        _spdsvc_tr471_gen_and_recycle_thread_g.workAvail = 1;
        wake_up_interruptible(&(_spdsvc_tr471_gen_and_recycle_thread_g.threadWqh));
    }
    else
    {
        TR471_PROCFS_INC(connindex, hw_gen_cmpl_cnt_dupl);
        SPDSVCLOG_INFO("TR471-Invalid/duplicate hw_gen_cmpl event. conn %d, state %d, active_us_conn %d \n", connindex,
                  conn_p->state, spdsvc_g.active_us_connidx);
    }
}

static int _spdsvc_tr471_gen_and_recycle_thread(void *data)
{
    volatile int connindex;
    spdsvc_t *spdsvc_p = (spdsvc_t *)data;
    spdsvc_conn_t *conn_p;
    int ret = 0, fwd_data = 0;

    while (1)
    {
        wait_event_interruptible(_spdsvc_tr471_gen_and_recycle_thread_g.threadWqh,
                                 _spdsvc_tr471_gen_and_recycle_thread_g.workAvail);

        if (kthread_should_stop())
        {
            SPDSVCLOG_INFO("kthread_should_stop detected on _spdsvc_tr471_gen_and_recycle_thread\n");
            break;
        }

        /* Record task wkup time (to start sw-gen or handle hw-gen-cmpl) */
        spdsvc_record_time(tr471_task_wkup_time_ns);

        if (_spdsvc_tr471_gen_and_recycle_thread_g.thread_mode == TR471_THREAD_MODE_FWD_N_RECYCLE_RX_BUFF)
        {
            spdsvc_tr471_rx_recycle_queue_entry_t *rx_recycle_p;

            while ((rx_recycle_p = spdsvc_tr471_rx_recycle_queue_read()))
            {
                struct loadHdr *lHdr = rx_recycle_p->lhdr_p;
                spdsvc_tr471_rx_queue_if_entry_t tr471_rx_q_if_entry;

                // Forward Action.

                // Sanity check for forward.
                if (LOAD_ID == ntohs(lHdr->loadId))
                {
                    // pass the user relevant info.
                    tr471_rx_q_if_entry.connindex   = spdsvc_g.connctx[rx_recycle_p->connindex].u_connindex;
                    tr471_rx_q_if_entry.rxq_loadHdr_p = lHdr;

                    if (spdsvc_tr471_rx_queue_write(&tr471_rx_q_if_entry,
                                _spdsvc_tr471_gen_and_recycle_thread_g.rx_recycle_thread_data)) {
                        TR471_PROCFS_INC(rx_recycle_p->connindex, fwd_lhdr);
                        fwd_data = 1;
                    }
                }
                else
                {
                    SPDSVCLOG_INFO("TR471Err: LoadId mismatch conn %d\n", connindex);
                    TR471_PROCFS_INC(rx_recycle_p->connindex, error_pkt_cnt);
                }

                // Release Action.
                /* For FKBs, the data is not invalidated at the time of free. This is
                   because FKBs are normally used in the forwarding path unlike Speedsvc
                   scenario which is using the FKBs in the terminating path. So, 
                   invalidate the cache lines to prevent corruption */


                if (rx_recycle_p->action != SPDSVC_RELEASE)
                    nbuff_invalidatefree(rx_recycle_p->pRxNBuf);
                else
                    nbuff_free(rx_recycle_p->pRxNBuf);

                TR471_PROCFS_INC(rx_recycle_p->connindex, rx_buf_thr_recycle);
                spdsvc_tr471_rx_recycle_queue_post();
            } /* while (rx_recycle_p) */
            
            if (!fwd_data)
                spdsvc_tr471_rx_queue_write_event(); /* Post the pending data event for previously queued pending packets in rx_queue. */
            else
                fwd_data = 0;
        }
        else if (_spdsvc_tr471_gen_and_recycle_thread_g.thread_mode == TR471_THREAD_MODE_SW_GEN)
        {
            connindex = spdsvc_p->active_us_connidx;
            /* Record task wkup time for sw-gen */
            tr471_record_tsk_wkup_time(tr471_burst_gen_start_time_ns, tr471_task_wkup_time_ns);
            TR471_PROCFS_INC(connindex, soft_gen_cnt);
            ret = tr471_send_tx_pkts(connindex, spdsvc_p);
            /* Record time taken for sw-gen */
            spdsvc_record_time(tr471_burst_gen_cmpl_time_ns);
            tr471_record_pktgen_time(tr471_task_wkup_time_ns, tr471_burst_gen_cmpl_time_ns);
        }
        else if (_spdsvc_tr471_gen_and_recycle_thread_g.thread_mode == TR471_THREAD_MODE_HW_DONE)
        {
            connindex = spdsvc_p->active_us_connidx;
            /* Record task wkup time for hw-gen */
            tr471_record_tsk_wkup_time(tr471_burst_gen_cmpl_time_ns, tr471_task_wkup_time_ns);
            TR471_PROCFS_INC(connindex, hw_gen_chk_status);
        }
        else
        {
            connindex = spdsvc_p->active_us_connidx;
            TR471_PROCFS_INC(connindex, error_cnt);
            __logError("TR471Err: invalid generator mode. conn %d", connindex);
        }

        if (_spdsvc_tr471_gen_and_recycle_thread_g.thread_mode != TR471_THREAD_MODE_FWD_N_RECYCLE_RX_BUFF)
        {
            SPDSVCLOG_INFO("471 BC-%d\n", connindex);
            conn_p = (spdsvc_conn_t *)&spdsvc_g.connctx[connindex];
            _spdsvc_tr471_gen_and_recycle_thread_g.thread_mode = TR471_THREAD_MODE_INVALID;
            _spdsvc_tr471_gen_and_recycle_thread_g.workAvail = 0;
            if ((!ret) && (!conn_p->is_wait_and_disable_active) && (conn_p->state != SPDSVC_STATE_OFF))
                conn_p->state = SPDSVC_STATE_GENERATOR_DONE;
            spdsvc_tr471_generator_burst_cmpl(connindex);
        }
        else
        {
            _spdsvc_tr471_gen_and_recycle_thread_g.thread_mode = TR471_THREAD_MODE_INVALID;
            _spdsvc_tr471_gen_and_recycle_thread_g.workAvail = 0;
        }
    } /* while (1) */

    return 0;
}

static int _spdsvc_tr471_gen_and_recycle_thread_construct(void)
{
    memset(&_spdsvc_tr471_gen_and_recycle_thread_g, 0, sizeof(_spdsvc_tr471_gen_and_recycle_thread_t));

    _spdsvc_tr471_gen_and_recycle_thread_g.workAvail = 0;

    init_waitqueue_head(&_spdsvc_tr471_gen_and_recycle_thread_g.threadWqh);

    _spdsvc_tr471_gen_and_recycle_thread_g.thread = kthread_create(_spdsvc_tr471_gen_and_recycle_thread, (void *)&spdsvc_g, "spdsvc_tr471_th");
    if (_spdsvc_tr471_gen_and_recycle_thread_g.thread == NULL)
    {
        __logError("Could not kthread_create: _spdsvc_tr471_gen_and_recycle_thread");
        return -1;
    }

    wake_up_process(_spdsvc_tr471_gen_and_recycle_thread_g.thread);

    return 0;
}

void _spdsvc_tr471_gen_and_recycle_thread_destruct(void)
{
    if (kthread_stop(_spdsvc_tr471_gen_and_recycle_thread_g.thread))
        __logError("TR-471 Could not kthread_stop\n");
}

/***************************************************************************
 * TR-471 netfilter hooks -- LOCAL_OUT hooks
 **************************************************************************/

static inline unsigned int _spdsvc_tr471_nf_hook_out_udp(int connindex, struct sk_buff *skb, const struct nf_hook_state *state, int is_v6)
{
    int ret;
    spdsvc_conn_t *conn_p = (spdsvc_conn_t *)&spdsvc_g.connctx[connindex];

    TR471_PROCFS_INC(connindex, nf_out_hk_cnt);
    if (conn_p->state != SPDSVC_STATE_GENERATOR_SETUP)
    {
        TR471_PROCFS_INC(connindex, nf_invalid_state);
        return NF_ACCEPT;
    }

    if (spdsvc_g.mode != SPDSVC_MODE_TR471)
    {
        TR471_PROCFS_INC(connindex, nf_invalid_mode);
        return NF_ACCEPT;
    }

    if (!skb || !skb->sk || !skb->sk->sk_socket)
    {
        TR471_PROCFS_INC(connindex, nf_invalid_socket);
        return NF_ACCEPT;
    }

    ret = spdsvc_tr471_process_out_udp(connindex, skb, skb->data, &skb->blog_p, is_v6);

    if (ret == TR471_NF_PKT_PASS)
    {
        return NF_ACCEPT;
    }
    else if (ret == TR471_NF_PKT_DROP)
    {
        nbuff_free(SKBUFF_2_PNBUFF(skb));
        return NF_STOLEN;
    }
    return NF_STOLEN;  /* if return value == TR471_NF_PKT_FWDED */
}

/* Netfilter IPv4 In/Out hooks for UDP */
static unsigned int _spdsvc_tr471_nf_hook_out4_udp(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    return _spdsvc_tr471_nf_hook_out_udp(spdsvc_g.active_us_connidx, skb, state, 0);
}
/* Netfilter IPv6 In/Out hooks for UDP */
static unsigned int _spdsvc_tr471_nf_hook_out6_udp(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    return _spdsvc_tr471_nf_hook_out_udp(spdsvc_g.active_us_connidx, skb, state, 1);
}

/* Netfilter declare hooks for UDP */
static struct nf_hook_ops nf_hks_tr471_out[] = {
    {
        .hook = _spdsvc_tr471_nf_hook_out4_udp,
        .hooknum = NF_INET_LOCAL_OUT,
        .pf = NFPROTO_IPV4,
        .priority = NF_IP_PRI_FIRST,
        .priv = (void *)NULL
    }, /* net filter OUT hook options */
    {
        .hook = _spdsvc_tr471_nf_hook_out6_udp,
        .hooknum = NF_INET_LOCAL_OUT,
        .pf = NFPROTO_IPV6,
        .priority = NF_IP6_PRI_FIRST,
        .priv = (void *)NULL
    }, /* net filter OUT hook options */
};

static int _spdsvc_tr471_netfilter_hook_out_init(int connindex)
{
    int i;

    TR471_PROCFS_INC(connindex, nf_out_hk_reg_cnt);

    if (spdsvc_g.active_conns == SPDSVC_ONLY_CONN)
    {
        /* Register nf hooks */
        for (i = 0; i < sizeof(nf_hks_tr471_out) / sizeof(struct nf_hook_ops); i++) nf_register_net_hook(&init_net, &nf_hks_tr471_out[i]);
    }

    return 0;
}

static int _spdsvc_tr471_netfilter_hook_out_uninit(int connindex)
{
    int i;

    TR471_PROCFS_INC(connindex, nf_out_hk_unreg_cnt);

    if (spdsvc_g.active_conns == SPDSVC_ONLY_CONN)
    {
        /* Unregister nf hooks */
        for (i = 0; i < sizeof(nf_hks_tr471_out) / sizeof(struct nf_hook_ops); i++) nf_unregister_net_hook(&init_net, &nf_hks_tr471_out[i]);
    }

    return 0;
}

/***************************************************************************
 * TR-471 netfilter hooks -- LOCAL_IN hooks
 **************************************************************************/
static inline unsigned int _spdsvc_tr471_nf_hook_in_udp(struct sk_buff *skb, const struct nf_hook_state *state, int is_v6)
{
    int connindex, lkup_result, ret = TR471_NF_PKT_PASS;
    spdsvc_conn_t *conn_p;

    lkup_result = spdsvc_conn_lookup(skb->data, is_v6, &connindex);

    conn_p = (spdsvc_conn_t *)&spdsvc_g.connctx[connindex];

    TR471_PROCFS_INC(connindex, nf_in_hk_cnt);
    if (conn_p->state != SPDSVC_STATE_ANALYZER)
    {
        TR471_PROCFS_INC(connindex, nf_invalid_state);
        return NF_ACCEPT;
    }

    if (spdsvc_g.mode != SPDSVC_MODE_TR471)
    {
        TR471_PROCFS_INC(connindex, nf_invalid_mode);
        return NF_ACCEPT;
    }
    /* Not checking socket for local-in because hook is bound to pre-routing */

    ret = spdsvc_tr471_process_in_udp(connindex, skb, skb->data, (void **)&skb->dev,
                                      &skb->blog_p, is_v6, lkup_result, conn_p->frag_id);

    if (ret == TR471_NF_PKT_PASS)
    {
        return NF_ACCEPT;
    }
    else if (ret == TR471_NF_PKT_DROP)
    {
        nbuff_free(SKBUFF_2_PNBUFF(skb));
        return NF_STOLEN;
    }
    else if (ret == TR471_NF_PKT_LHDR)
    {
        _spdsvc_tr471_fwd_loadHdr_RecycleBuff(connindex, SKBUFF_2_PNBUFF(skb), SPDSVC_RELEASE, 0); /* No Fkb. */
        return NF_STOLEN;
    }
    return NF_STOLEN;  /* ret == TR471_NF_PKT_FWDED */
}

/* Netfilter IPv4 In/Out hooks for UDP */
static unsigned int _spdsvc_tr471_nf_hook_in4_udp(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    return _spdsvc_tr471_nf_hook_in_udp(skb, state, 0);
}
/* Netfilter IPv6 In/Out hooks for UDP */
static unsigned int _spdsvc_tr471_nf_hook_in6_udp(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    return _spdsvc_tr471_nf_hook_in_udp(skb, state, 1);
}

/* Netfilter declare hooks for UDP */
static struct nf_hook_ops nf_hks_tr471_in[] = {
    {
        .hook = _spdsvc_tr471_nf_hook_in4_udp,
        .hooknum = NF_INET_LOCAL_IN,
        .pf = NFPROTO_IPV4,
        .priority = NF_IP_PRI_LAST,
        .priv = (void *)NULL
    }, /* net filter IN hook options */
    {
        .hook = _spdsvc_tr471_nf_hook_in6_udp,
        .hooknum = NF_INET_LOCAL_IN,
        .pf = NFPROTO_IPV6,
        .priority = NF_IP6_PRI_LAST,
        .priv = (void *)NULL
    }, /* net filter OUT hook options */
};
static int _spdsvc_tr471_netfilter_hook_in_init(int connindex)
{
    int i;
    TR471_PROCFS_INC(connindex, nf_in_hk_reg_cnt);

    if (spdsvc_g.active_conns == SPDSVC_ONLY_CONN)
    {
        /* Register nf hooks */
        for (i = 0; i < sizeof(nf_hks_tr471_in) / sizeof(struct nf_hook_ops); i++) nf_register_net_hook(&init_net, &nf_hks_tr471_in[i]);
    }
    return 0;
}

static int _spdsvc_tr471_netfilter_hook_in_uninit(int connindex)
{
    int i;
    TR471_PROCFS_INC(connindex, nf_in_hk_unreg_cnt);

    if (spdsvc_g.active_conns == SPDSVC_ONLY_CONN)
    {
        /* Unregister nf hooks */
        for (i = 0; i < sizeof(nf_hks_tr471_in) / sizeof(struct nf_hook_ops); i++) nf_unregister_net_hook(&init_net, &nf_hks_tr471_in[i]);
    }
    return 0;
}

/***************************************************************************
 * Blog notifier hooks
 **************************************************************************/
static int _spdsvc_tr471_blog_flowevent(struct notifier_block *nb, unsigned long event, void *info)
{
    int connindex = 0;
    BlogFlowEventInfo_t *einfo = info;
    if (einfo->is_tr471_flow)
    {
        if (spdsvc_g.dir == SPDSVC_DIRECTION_US)
        {
            /* For upstream; SPDSVC driver is the source/RX of the flow */
            connindex = einfo->rx_channel;
        }
        else /* if (spdsvc_g.dir == SPDSVC_DIRECTION_DS) */
        {
            /* For downstream; SPDSVC driver is the sink/TX of the flow */
            connindex = einfo->tx_channel;
        }
        if (connindex >= 0 && connindex < SPDSVC_MAX_CONN)
        {
            TR471_PROCFS_INC(connindex, blog_event_cnt);
            if (event == FLOW_EVENT_ACTIVATE && einfo->flow_event_type == FLOW_EVENT_TYPE_HW 
                && spdsvc_g.dir == SPDSVC_DIRECTION_DS)
            {
                spdsvc_conn_t *conn_p = (spdsvc_conn_t *)&spdsvc_g.connctx[connindex];
                if (spdsvc_hw_tr471_is_hw_support(connindex))
                    spdsvc_hw_tr471_analyzer_enable(&conn_p->rx_tuple, connindex);
            }

            spdsvc_fc_blog_flowevent(connindex, event, einfo->flow_event_type, einfo->rx_max_pktlen);
        }
        else
        {
            /* Use default connindex to notify error */
            __logError("TR471Err: Invalid conn %d in blog flowevent", connindex);
            TR471_PROCFS_INC(0, error_cnt);
        }
    }
    return NOTIFY_OK;
}

static struct notifier_block _spdsvc_tr471_blog_notifier = {
    .notifier_call = _spdsvc_tr471_blog_flowevent,
};

void spdsvc_tr471_generator_burst_cmpl(int connindex)
{
    spdsvc_conn_t *conn_p = (spdsvc_conn_t *)&spdsvc_g.connctx[connindex];

    /* Notify application about burst completion */
    spdsvc_record_time(tr471_burst_cmpl_time_ns);
    tr471_record_total_burst_time(tr471_burst_enable_time_ns, tr471_burst_cmpl_time_ns);

    spdsvc_tr471_burst_cmpl_event(conn_p->rx_max_pktlen);
    _spdsvc_tr471_state_g.flowinfo[connindex].tx_count = 0;
}

void spdsvc_tr471_generator_disable(int connindex)
{
    /* Disable the hardware generator */
    if (spdsvc_hw_tr471_is_hw_support(connindex))
    {
        spdsvc_hw_tr471_generator_disable(connindex);
    }
}

/***************************************************************************
 * Reset TR471 state machine 
 * Burst is complete or error handling 
 * Notes :
       Following Module load time attributes of tr471_state are not reset
            installed and netdev_p
       flow_state is not reset to keep it in sync with flow-cache 
 **************************************************************************/
void spdsvc_tr471_reset_state_machine(int connindex)
{
    spdsvc_conn_t *conn_p = (spdsvc_conn_t *)&spdsvc_g.connctx[connindex];
    spdsvc_tr471_flow_info_t *flowinfo_p = (spdsvc_tr471_flow_info_t *)&_spdsvc_tr471_state_g.flowinfo[connindex];

    TR471_PROCFS_INC(connindex, reset_cnt);
    if (conn_p->state == SPDSVC_STATE_OFF)
    {
        return;
    }
    /* Unregister netfilter hooks */
    _spdsvc_tr471_netfilter_hook_out_uninit(connindex);

    conn_p->tr471.totalburst = 0;
    if (flowinfo_p->ref_pkt_info.fkb_p)
    {
        nbuff_free(FKBUFF_2_PNBUFF(flowinfo_p->ref_pkt_info.fkb_p));
        TR471_PROCFS_INC(connindex, pkt_free_cnt);
    }
    /* Ref packet is freed, clear out all ref pkt related info */
    memset(&flowinfo_p->ref_pkt_info, 0, sizeof(flowinfo_p->ref_pkt_info));
    flowinfo_p->is_hw_tx = 0;
    if (spdsvc_g.active_conns == SPDSVC_ONLY_CONN)
        _spdsvc_tr471_state_g.is_v6 = 0;
    flowinfo_p->tx_count = 0;
}

static inline int _spdsvc_tr471_generator_is_new_test(spdsvc_tuple_t *curr_tuple_p, spdsvc_tuple_t *new_tuple_p)
{
    if ((curr_tuple_p->ports == new_tuple_p->ports) &&
        (curr_tuple_p->family == new_tuple_p->family))
    {
        switch (curr_tuple_p->family)
        {
            case SPDSVC_FAMILY_IPV4:
            {
                if ((curr_tuple_p->dest_ip_addr.ipv4.u32 != new_tuple_p->dest_ip_addr.ipv4.u32) ||
                    (curr_tuple_p->source_ip_addr.ipv4.u32 != new_tuple_p->source_ip_addr.ipv4.u32))
                {
                    return 1; /* New test */
                }
            }
                return 0; /* Different burst for the same test */

            case SPDSVC_FAMILY_IPV6:
            {
                int i;
                for (i = 0; i < 4; i++)
                {
                    if ((curr_tuple_p->dest_ip_addr.ipv6.u32[i] != new_tuple_p->dest_ip_addr.ipv6.u32[i]) ||
                        (curr_tuple_p->source_ip_addr.ipv6.u32[i] != new_tuple_p->source_ip_addr.ipv6.u32[i]))
                    {
                        return 1; /* New test */
                    }
                }
            }
                return 0; /* Different burst for the same test */

            default:
                /* Error */
                return 1;
        }
    }

    return 1;
}

/* Flush/Purge flows on the device if the flow state is inactive */
/* TBD - how to purge only flows impacted and not all the flows of the device
 * */
static void _spdsvc_tr471_flush_flows(void)
{
    int i, flushDevFlag = 0;
    spdsvc_tr471_flow_info_t *flowinfo_p;

    for (i = 0; i < SPDSVC_MAX_CONN; i++)
    {
        flowinfo_p = (spdsvc_tr471_flow_info_t *)&_spdsvc_tr471_state_g.flowinfo[i];
        if (flowinfo_p->flow_state != TR471_FLOW_STATE_INACTIVE)
        {
            flushDevFlag = 1;
            if (flowinfo_p->flow_state == TR471_FLOW_STATE_ACTIVE_SW)
                TR471_PROCFS_INC(i, blog_event_fc_deact_cnt);
            else
                TR471_PROCFS_INC(i, blog_event_hw_deact_cnt);
            flowinfo_p->flow_state = TR471_FLOW_STATE_INACTIVE;
        }
    } /* for all connections */

    /* For the near term, (until we flush per connection), we set the flow
     * states for all the connections to inactive and induce FLUSH on the device
     */
    if (flushDevFlag)
    {
        BlogFlushParams_t blogFlushParams = { };

        blogFlushParams.flush_dev = 1;
        blogFlushParams.devid = _spdsvc_tr471_netdev->ifindex;
        blog_notify_async_wait(FLUSH, (void *)&blogFlushParams, (unsigned long)&blogFlushParams, 0);
    }
}

/***************************************************************************
 * Speed Service TR471 Public Interface
 **************************************************************************/
void spdsvc_tr471_generator_setup(int connindex)
{
    spdsvc_conn_t *conn_p = (spdsvc_conn_t *)&spdsvc_g.connctx[connindex];
    spdsvc_tr471_flow_info_t *flowinfo_p = &_spdsvc_tr471_state_g.flowinfo[connindex];

    if (_spdsvc_tr471_generator_is_new_test(&flowinfo_p->tr471_tx_tuple, &conn_p->tx_tuple))
    {
        /* New test */
        __logNotice("\nSpeed Service: New test C %d active_conns %d", connindex, spdsvc_g.active_conns);
        memcpy(&flowinfo_p->tr471_tx_tuple, &conn_p->tx_tuple, sizeof(spdsvc_tuple_t));

        if (spdsvc_g.active_conns == SPDSVC_ONLY_CONN)
        {
            /* Flush flows from previous tests */
            _spdsvc_tr471_flush_flows();
        }
    }

    /* Register netfilter hooks */
    _spdsvc_tr471_netfilter_hook_out_init(connindex);
}

int spdsvc_tr471_analyzer_enable(int connindex, spdsvc_socket_t *socket_p, spdsvc_config_tr471_t *tr471_p)
{
    spdsvc_conn_t *conn_p = (spdsvc_conn_t *)&spdsvc_g.connctx[connindex];

    if (SPDSVC_STATE_OFF == conn_p->state)
    {
        if (spdsvc_store_tuple(socket_p, &conn_p->rx_tuple))
        {
            __logError("Invalid RX tuple");
            return SPDSVC_RET_ERROR;
        }
        /* Flush flows from previous tests */
        _spdsvc_tr471_flush_flows();

        /* Note: Not enabling the hardware analyzer just yet;
           Enable hardware once downstream flow is pushed to hardware */

        _spdsvc_tr471_netfilter_hook_in_init(connindex);

        return SPDSVC_RET_SUCCESS;
    }
    __logError("SPDSVC already in progress (%d)", conn_p->state);
    return SPDSVC_RET_BUSY;
}

int spdsvc_tr471_analyzer_disable(int connindex)
{
    spdsvc_conn_t *conn_p = (spdsvc_conn_t *)&spdsvc_g.connctx[connindex];

    if (SPDSVC_STATE_ANALYZER == conn_p->state)
    {
        if (spdsvc_hw_tr471_is_hw_support(connindex))
            spdsvc_hw_tr471_analyzer_disable(connindex);

        _spdsvc_tr471_netfilter_hook_in_uninit(connindex);

        conn_p->state = SPDSVC_STATE_OFF;

        if (spdsvc_g.active_conns == SPDSVC_ONLY_CONN)
        {
           spdsvc_tr471_delay(10);
 
           /* Make sure receive data thread is idle, before scheduling it for single event processing (as opposed to normal mode of batch processing of events
            * to the application Q.
            */
            while (_spdsvc_tr471_gen_and_recycle_thread_g.workAvail)
            {
                spdsvc_tr471_delay(5);
                TR471_PROCFS_INC(connindex, rx_wait_for_completion);
            }

            _spdsvc_tr471_gen_and_recycle_thread_g.thread_mode = TR471_THREAD_MODE_FWD_N_RECYCLE_RX_BUFF;
            _spdsvc_tr471_gen_and_recycle_thread_g.rx_recycle_thread_data = SPDSVC_EVENT_SIGNAL_NORMAL_MODE;
            _spdsvc_tr471_gen_and_recycle_thread_g.workAvail = 1;
            wake_up_interruptible(&(_spdsvc_tr471_gen_and_recycle_thread_g.threadWqh));

            while (_spdsvc_tr471_gen_and_recycle_thread_g.workAvail)
            {
                spdsvc_tr471_delay(5);
                TR471_PROCFS_INC(connindex, rx_wait_for_completion);
            }

            _spdsvc_tr471_state_g.rx_recycle_budget = 0;
            _spdsvc_tr471_state_g.rx_conf_recycle_budget = SPDSVC_RX_MIN_RECYCLE_BUDGET;
            spdsvc_tr471_rx_queue_stats(&spdsvc_procfs_tr471_rx_queue_stats_g.pending,
                                        &spdsvc_procfs_tr471_rx_queue_stats_g.total,
                                        &spdsvc_procfs_tr471_rx_queue_stats_g.writes,
                                        &spdsvc_procfs_tr471_rx_queue_stats_g.reads,
                                        &spdsvc_procfs_tr471_rx_queue_stats_g.discards);
            spdsvc_tr471_rx_queue_init();
        }
    } /* If connection state is ANALYZER */

    return 0;
}

int spdsvc_tr471_get_result(int connindex, spdsvc_result_t *result_p)
{
    spdsvc_conn_t *conn_p = (spdsvc_conn_t *)&spdsvc_g.connctx[connindex];

    if (!_spdsvc_tr471_state_g.installed)
        return -1;

    conn_p->tr471_generator_is_supported = 1;
    /* If the spdsvc state is OFF AND mode is not TR471 i.e. generator not running
     * Reset state m/c function sets both state and mode, if state gets set to OFF 
     * but before mode is updated, task gets preempted, the next burst will fail */
    result_p->running = !((conn_p->state == SPDSVC_STATE_OFF) && (spdsvc_g.mode != SPDSVC_MODE_TR471));

    result_p->rx_packets = 0;
    result_p->rx_bytes = 0;
    result_p->rx_time_usec = 0;
    result_p->tx_packets = 1;
    result_p->tx_discards = 0;

    return 0;
}

extern void bcmnet_configure_gdx_accel(struct net_device *dev, bcmnet_accel_t *accel_p);

int __init spdsvc_tr471_construct(void)
{
    int ret;
    bcmnet_accel_t gdx_accel;

    memset(&gdx_accel, 0, sizeof(gdx_accel));
    memset(&_spdsvc_tr471_state_g, 0, sizeof(_spdsvc_tr471_state_g));
    _spdsvc_tr471_state_g.rx_conf_recycle_budget = SPDSVC_RX_MIN_RECYCLE_BUDGET;

    /* SpdSvc TR471 Netdevice - setup */
    _spdsvc_tr471_netdev = alloc_netdev(0, "spdsvc_tr471", NET_NAME_UNKNOWN, _spdsvc_tr471_netdev_setup);

    if (!_spdsvc_tr471_netdev)
    {
        _spdsvc_tr471_netdev = 0;
        __logError("Failed to allocate netdev\n");
        return -1;
    }

    netdev_accel_tx_fkb_set(_spdsvc_tr471_netdev);
    ret = register_netdev(_spdsvc_tr471_netdev);

    if (ret)
    {
        free_netdev(_spdsvc_tr471_netdev);
        __logError("Failed to register netdev\n");
        return ret;
    }

#if defined(CONFIG_BCM_RDPA) || defined(CONFIG_BCM_RDPA_MODULE)
    bcm_netdev_ext_field_set(_spdsvc_tr471_netdev, bcm_netdev_cb_fn, bcm_netdev_def_cpu_port_obj_get);
#endif

    /* Enable GDX-TX for TR471-RX */
    gdx_accel.gdx_tx = 1;

#if defined(CONFIG_BCM_GDX_HW) && defined(CC_SPDSVC_TR471_FLOW_BASED_HW_SUPPORT)
    gdx_accel.gdx_hw = 1;
#endif

    bcmnet_configure_gdx_accel(_spdsvc_tr471_netdev, &gdx_accel);

    _spdsvc_tr471_state_g.netdev_p = _spdsvc_tr471_netdev;

    blog_flowevent_register_notifier(&_spdsvc_tr471_blog_notifier);

    ret = spdsvc_tr471_rx_recycle_queue_construct();
    if (ret)
    {
        __logError("Cannot spdsvc_tr471_rx_recycle_queue_construct");
        return ret;
    }

    spin_lock_init(&_spdsvc_tr471_state_g.spdsvc_lock);

    /* SpdSvc TR471 TX thread - setup */
    _spdsvc_tr471_gen_and_recycle_thread_construct();

    _spdsvc_tr471_state_g.installed = 1;

    return ret;
}


void __exit spdsvc_tr471_destruct(void)
{
    // Netdev free
    if (_spdsvc_tr471_netdev)
    {
        unregister_netdev(_spdsvc_tr471_netdev);
        free_netdev(_spdsvc_tr471_netdev);
        _spdsvc_tr471_netdev = NULL;
    }

    spdsvc_tr471_rx_recycle_queue_destruct();

    /* SpdSvc TR471 TX thread - clean up */
    _spdsvc_tr471_gen_and_recycle_thread_destruct();

    blog_flowevent_unregister_notifier(&_spdsvc_tr471_blog_notifier);

}
