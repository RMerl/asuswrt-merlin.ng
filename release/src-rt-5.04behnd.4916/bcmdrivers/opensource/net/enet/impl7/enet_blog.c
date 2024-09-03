/*
   <:copyright-BRCM:2015:DUAL/GPL:standard
   
      Copyright (c) 2015 Broadcom 
      All Rights Reserved
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2, as published by
   the Free Software Foundation (the "GPL").
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   
   A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
   writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
   
   :>
*/

#include "enet.h"
#include "bcmenet_common.h"
#include <linux/bcm_version_compat.h>
#include <linux/kthread.h>

enetx_port_t *blog_chnl_to_port[BLOG_EGPHY_CHNL_MAX];

void blog_chnl_unit_port_set(enetx_port_t *p)
{
    int unit = PORT_ON_ROOT_SW(p) ? 0 : 1;
    int port = p->port_info.port;
    int blog_idx = PHYSICAL_PORT_TO_LOGICAL_PORT(port, unit);

    blog_chnl_set(BLOG_ENETPHY, blog_idx, blog_idx, p);
}

void blog_chnl_unit_port_unset(enetx_port_t *p)
{
    int unit = PORT_ON_ROOT_SW(p) ? 0 : 1;
    int port = p->port_info.port;
    int blog_idx = PHYSICAL_PORT_TO_LOGICAL_PORT(port, unit);

    blog_chnl_set(BLOG_ENETPHY, blog_idx, blog_idx, NULL);
}

void blog_chnl_with_mark_set(int blog_phy, enetx_port_t *p)
{
   p->n.blog_phy = blog_phy;
   p->n.set_channel_in_mark = 1;
}

static void _blog_chnl_set(int blog_phy, int blog_channel, int blog_channel_tx, enetx_port_t *p)
{
    p->n.blog_phy = blog_phy;
    p->n.set_channel_in_mark = 0;
    p->n.blog_chnl_rx = blog_channel;
    p->n.blog_chnl = blog_channel_tx;
}

void blog_chnl_set(int blog_phy, int blog_channel, int blog_channel_tx, enetx_port_t *p)
{
    if (blog_phy != BLOG_ENETPHY)
    {
        _blog_chnl_set(blog_phy, blog_channel, blog_channel_tx, p);
        return;
    }

    if (blog_channel >= BLOG_EGPHY_CHNL_MAX)
    {
        printk("Blog channel %d larger than BLOG_EGPHY_CHNL_MAX %d for port %s\n", blog_channel, BLOG_EGPHY_CHNL_MAX, p->obj_name);
        BUG();
    }

    if (p)
    {
        if (blog_chnl_to_port[blog_channel])
        {
            printk("Failed to register Blog channel %d for port %s, used by port %s\n", blog_channel, p->obj_name, blog_chnl_to_port[blog_channel]->obj_name);
            BUG();
        }

        _blog_chnl_set(blog_phy, blog_channel, blog_channel_tx, p);
    }
    enet_dbgv("blog_phy=%d, blog_chnl=%x\n", blog_phy, blog_channel);

    blog_chnl_to_port[blog_channel] = p;
}

/*
 *------------------------------------------------------------------------------
 * enet Rx blog/fcache Thread implementation
 */
#if defined(CONFIG_ENET_RX_BLOG_THREAD)

#define ENET_RX_BLOG_RING_SIZE      32         // ring size need to be power of 2
#define ENET_RX_BLOG_RING_THRES_HI  ((ENET_RX_BLOG_RING_SIZE>>2)*2)     // 1/2
#define ENET_RX_BLOG_RING_THRES_LO  ((ENET_RX_BLOG_RING_SIZE>>2)*1)     // 1/4

typedef struct {
    enetx_port_t   *port;
    struct fkbuff  *fkb_p;
    enetx_rx_info_t rx_info;
} enet_rx_blog_evt_t;

struct {
    enet_rx_blog_evt_t ring[ENET_RX_BLOG_RING_SIZE];
    volatile uint32_t       wr;
    volatile uint32_t       rd;
    atomic_t                cnt;
    struct task_struct     *thread;
    int                     use_rx_thread;

    uint32_t dbg_rd_cnt;
    uint32_t dbg_bypass_cnt;
    uint32_t dbg_ring_full_cnt;
    uint32_t dbg_ring_full_cnt2;
    uint32_t dbg_enq_to_wake_cnt;
    uint32_t dbg_enq_thres_wake_cnt;
    uint32_t dbg_sched_cnt;
} enet_rx_blog_g;

#define ENET_RX_BLOG_RING_NDX(ndx)      ((ndx) & (ENET_RX_BLOG_RING_SIZE-1))
#define IS_ENET_RX_BLOG_RINGFULL(cnt)   ((cnt) == (ENET_RX_BLOG_RING_SIZE-1))

#define ENET_RX_BLOG_WAKE_THREAD(counter) { counter++; wake_up_process(enet_rx_blog_g.thread);}

int enet_rx_blog_is_full(void)
{
    int count;
    if (enet_rx_blog_g.use_rx_thread)
    {
        count = atomic_read(&enet_rx_blog_g.cnt);
        if (unlikely(IS_ENET_RX_BLOG_RINGFULL(count)))
        {
            if (!(enet_rx_blog_g.dbg_ring_full_cnt2++ & 0xffff) && 
                (enet_rx_blog_g.thread->__state != TASK_RUNNING))  // reading thread state is expensive only do it once awhile
                ENET_RX_BLOG_WAKE_THREAD(enet_rx_blog_g.dbg_ring_full_cnt);
            return 1;
        }
    }
    return 0;
}

/* return 0 - if enqueued
 *        1 - ring_full
 *       -1 - bypass
 */
int enet_rx_blog_enqueue(struct fkbuff * fkb_p, enetx_port_t  *port, enetx_rx_info_t *rx_info_p, int *count_p)
{
    enet_rx_blog_evt_t *evt_p;
    int count;
    static int thres_fired;

    // perform use_rx_thread check
    if (!enet_rx_blog_g.use_rx_thread)
    {
        enet_rx_blog_g.dbg_bypass_cnt++;
        return -1;
    }

    rcu_read_unlock();
    evt_p = &(enet_rx_blog_g.ring[ENET_RX_BLOG_RING_NDX(enet_rx_blog_g.wr)]);
    evt_p->port = port;
    evt_p->fkb_p = fkb_p;
    evt_p->rx_info = *rx_info_p;

    barrier();
    enet_rx_blog_g.wr++;
    barrier();
    count = atomic_inc_return(&enet_rx_blog_g.cnt);

    if (count==1 && enet_rx_blog_g.thread->__state != TASK_RUNNING)  // reading thread state is expensive only do it for 1st enqueue
        ENET_RX_BLOG_WAKE_THREAD(enet_rx_blog_g.dbg_enq_to_wake_cnt)
    else if (count >= ENET_RX_BLOG_RING_THRES_HI) {
        if (thres_fired == 0) {
            ENET_RX_BLOG_WAKE_THREAD(enet_rx_blog_g.dbg_enq_thres_wake_cnt)
            thres_fired ++;
        }
    } else if (count < ENET_RX_BLOG_RING_THRES_LO)
        thres_fired = 0;
    (*count_p)++;
    return 0;
}

extern int enetx_weight_budget;

static int enet_rx_blog_func(void *thread_data)
{
    BlogFcArgs_t fc_args;
    int budget;

    memset(&fc_args, 0, sizeof(BlogFcArgs_t));

    while (!kthread_should_stop()) {
        if (atomic_read(&enet_rx_blog_g.cnt)) {
            budget = enetx_weight_budget;
            do {
                enet_rx_blog_evt_t *evt_p;
                enetx_rx_info_t    *rx_info_p;
                BlogAction_t        blog_action;

                if (unlikely(budget-- <= 0))
                {
                    yield();
                    budget = enetx_weight_budget;
                }
                evt_p = &(enet_rx_blog_g.ring[ENET_RX_BLOG_RING_NDX(enet_rx_blog_g.rd)]);
                rx_info_p = &(evt_p->rx_info);
                fc_args.group_fwd_exception = rx_info_p->is_group_fwd_exception;
                fc_args.fc_ctxt = rx_info_p->fc_ctxt;
                blog_action = blog_finit(evt_p->fkb_p, evt_p->port->dev, TYPE_ETH, 
                                evt_p->port->n.set_channel_in_mark ? rx_info_p->flow_id : evt_p->port->n.blog_chnl_rx,
                                    evt_p->port->n.blog_phy, &fc_args);
                if (unlikely(blog_action == PKT_DROP))
                {
                    _free_fkb(evt_p->fkb_p);
                    INC_STAT_RX_DROP(evt_p->port,rx_dropped_blog_drop);
                    goto DONE;
                }

                /* packet consumed, proceed to next packet*/
                if (likely(blog_action == PKT_DONE))
                {
                    INC_STAT_DBG(evt_p->port,rx_packets_blog_done);
                    goto DONE;
                }
                
                rx_skb(evt_p->fkb_p, evt_p->port, rx_info_p);
DONE:
                enet_rx_blog_g.rd++;
                barrier();
            } while (atomic_dec_return(&enet_rx_blog_g.cnt)); 
        }
        enet_rx_blog_g.dbg_sched_cnt++;
        set_current_state(TASK_INTERRUPTIBLE);
        schedule();
    }
    return 0;
}

int enet_rx_blog_thread_create(void)
{
    struct task_struct *tsk;
    
    tsk = kthread_create(enet_rx_blog_func, NULL, "enet_rx_blog");
    if (IS_ERR(tsk))
    {
        enet_err("enet_rx_blog creation failed\n");
        return -1;
    }

    enet_rx_blog_g.use_rx_thread = 1;
    enet_rx_blog_g.wr = enet_rx_blog_g.rd = 0;
    atomic_set(&enet_rx_blog_g.cnt, 0);
    enet_rx_blog_g.thread = tsk;
    
    wake_up_process(enet_rx_blog_g.thread);
    printk("enet_rx_blog created successfully\n");
    return 0;
}

void enet_rx_blog_thread_destory(void)
{
    kthread_stop(enet_rx_blog_g.thread);
}

#define RD_PKTS_PER_WAKE(x)     (enet_rx_blog_g.rd-enet_rx_blog_g.dbg_rd_cnt)/(x)
int enet_rx_blog_dump(int argc, char *argv[])
{
    printk("usage: rx_blog [clr]\n\n");

    printk("info:  use_rx_thread=%d/%u wrCnt=%u rdCnt=%u ringSz=%d filled=%d\n", enet_rx_blog_g.use_rx_thread, enet_rx_blog_g.dbg_sched_cnt,
        enet_rx_blog_g.wr, enet_rx_blog_g.rd, ENET_RX_BLOG_RING_SIZE, atomic_read(&enet_rx_blog_g.cnt));
    printk("debug: full=%u/%u bypass=%u enqToWake=%u(%u) enqThresWake(>=%d..%d)=%u(%u)\n",
        enet_rx_blog_g.dbg_ring_full_cnt, enet_rx_blog_g.dbg_ring_full_cnt2, enet_rx_blog_g.dbg_bypass_cnt,
        enet_rx_blog_g.dbg_enq_to_wake_cnt, RD_PKTS_PER_WAKE(enet_rx_blog_g.dbg_enq_to_wake_cnt),
        ENET_RX_BLOG_RING_THRES_HI, ENET_RX_BLOG_RING_THRES_LO, enet_rx_blog_g.dbg_enq_thres_wake_cnt, RD_PKTS_PER_WAKE(enet_rx_blog_g.dbg_enq_thres_wake_cnt));

    if (argc >=2 && strcmp(argv[1], "clr") == 0)
    {
        enet_rx_blog_g.dbg_sched_cnt=0;
        enet_rx_blog_g.dbg_bypass_cnt=0;
        enet_rx_blog_g.dbg_ring_full_cnt=0;
        enet_rx_blog_g.dbg_ring_full_cnt2=0;
        enet_rx_blog_g.dbg_enq_to_wake_cnt=0;
        enet_rx_blog_g.dbg_enq_thres_wake_cnt=0;
        enet_rx_blog_g.dbg_rd_cnt = enet_rx_blog_g.rd;
    }

    return 0;
}

int enet_rx_blog_en(int argc, char *argv[])
{
    printk("usage: rx_blog_en [enable|disable]\n\n");
    
    if (argc >=2)
    {
        if (strcmp(argv[1], "enable") == 0)
            enet_rx_blog_g.use_rx_thread = 1;
        else if (strcmp(argv[1], "disable") == 0)
            enet_rx_blog_g.use_rx_thread = 0;
    }
    printk(" use_rx_thread=%d\n", enet_rx_blog_g.use_rx_thread);
        
    return 0;
}

#endif // CONFIG_ENET_RX_BLOG_THREAD
