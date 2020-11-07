/*
  <:copyright-BRCM:2018:proprietary:standard

  Copyright (c) 2018 Broadcom 
  All Rights Reserved

  This program is the proprietary software of Broadcom and/or its
  licensors, and may only be used, duplicated, modified or distributed pursuant
  to the terms and conditions of a separate, written license agreement executed
  between you and Broadcom (an "Authorized License").  Except as set forth in
  an Authorized License, Broadcom grants no license (express or implied), right
  to use, or waiver of any kind with respect to the Software, and Broadcom
  expressly reserves all rights in and to the Software and all intellectual
  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

  Except as expressly set forth in the Authorized License,

  1. This program, including its structure, sequence and organization,
  constitutes the valuable trade secrets of Broadcom, and you shall use
  all reasonable efforts to protect the confidentiality thereof, and to
  use this information only in connection with your use of Broadcom
  integrated circuit products.

  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
  ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
  FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
  COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
  TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
  PERFORMANCE OF THE SOFTWARE.

  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
  ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
  WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
  IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
  OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
  SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
  SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
  LIMITED REMEDY.
  :> 
*/

/*
*******************************************************************************
*
* File Name  : archer_wfd.c
*
* Description: Archer WLAN Forwarding Driver
*
*******************************************************************************
*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/bcm_log.h>
#include <linux/blog.h>
#include <linux/bcm_skb_defines.h>
#include "bcmenet.h"

#include "sysport_rsb.h"
#include "sysport_classifier.h"

#include "archer.h"
#include "archer_driver.h"
#include "archer_thread.h"
#include "archer_wfd.h"

//#define CC_ARCHER_WFD_FORWARD

archer_wfd_hooks_t archer_wfd_hooks = { };

uint32_t archer_wfd_tx_context_g = 0;

/*******************************************************************
 *
 * Local Forwarding
 *
 *******************************************************************/

#if defined(CC_ARCHER_WFD_FORWARD)

#define ARCHER_WFD_QUEUE_MAX      ( SYSPORT_FLOW_WLAN_PORTS_MAX * SYSPORT_FLOW_WLAN_QUEUES_MAX )
#define ARCHER_WFD_TASK_PRIORITY  ARCHER_TASK_PRIORITY_LOW
#define ARCHER_WFD_BUDGET         64

typedef struct {
    int rx_runs;
    int rx_packets;
    int rx_packets_max;
} archer_wfd_queue_stats_t;

typedef struct {
    archer_wfd_bulk_get_t wfd_bulk_get;
    archer_wfd_queue_stats_t stats;
} archer_wfd_queue_t;

typedef struct {
    archer_wfd_queue_t queue[ARCHER_WFD_QUEUE_MAX];
    archer_task_t task;
    unsigned long queue_work_mask;
} archer_wfd_t;

static archer_wfd_t archer_wfd_g;

static void archer_wfd_notify(int queue_index)
{
    set_bit(queue_index, &archer_wfd_g.queue_work_mask);

    archer_task_schedule(&archer_wfd_g.task, ARCHER_WFD_TASK_PRIORITY);
}

static int archer_wfd_task_handler(void *arg_p)
{
    int queue_work_mask = archer_wfd_g.queue_work_mask;
    int queue_index = __fls(queue_work_mask);
    int re_schedule = 0;

    while(queue_work_mask)
    {
        archer_wfd_queue_t *queue_p = &archer_wfd_g.queue[queue_index];
        int rx_packets;
        int work_done;

        rx_packets = queue_p->wfd_bulk_get(queue_index, ARCHER_WFD_BUDGET, &work_done);

        queue_p->stats.rx_runs++;
        queue_p->stats.rx_packets += rx_packets;
        if(rx_packets > queue_p->stats.rx_packets_max)
        {
            queue_p->stats.rx_packets_max = rx_packets;
        }

        if(work_done)
        {
            clear_bit(queue_index, &archer_wfd_g.queue_work_mask);
        }
        else
        {
            re_schedule++;
        }

        queue_work_mask &= ~(1 << queue_index);
        queue_index--;
    }

    if(re_schedule)
    {
        archer_task_schedule(&archer_wfd_g.task, ARCHER_WFD_TASK_PRIORITY);
    }

    return 0;
}

static int archer_wfd_config(void *arg_p)
{
    archer_wfd_config_t *config_p = (archer_wfd_config_t *)arg_p;
    archer_wfd_queue_t *queue_p = &archer_wfd_g.queue[config_p->queue_index];

    if(config_p->queue_index >= ARCHER_WFD_QUEUE_MAX)
    {
        __logError("Invalid WFD Queue Number: %d, max %d",
                   config_p->queue_index, ARCHER_WFD_QUEUE_MAX);
        return -1;
    }

    queue_p->wfd_bulk_get = config_p->wfd_bulk_get;

    bcm_print("Archer WFD FWD: Radio %d, Queue %d\n",
              config_p->radio_index, config_p->queue_index);

    return 0;
}

static void __init archer_wfd_forward_construct(void)
{
    memset(&archer_wfd_g, 0, sizeof(archer_wfd_t));

    ARCHER_TASK_INIT(&archer_wfd_g.task, ARCHER_THREAD_ID_US,
                     archer_wfd_task_handler, NULL);
}

#else /* !CC_ARCHER_WFD_FORWARD */

static int archer_wfd_config(void *arg_p)
{
    return 0;
}

#endif /* CC_ARCHER_WFD_FORWARD */

/*******************************************************************
 *
 * WFD Driver Binding
 *
 *******************************************************************/

void archer_wfd_stats(void)
{
    if(archer_wfd_hooks.queue_stats)
    {
#if defined(CC_ARCHER_WFD_FORWARD)
        int queue_index;

        bcm_print("Archer WFD Forwarder\n");

        for(queue_index=0; queue_index<ARCHER_WFD_QUEUE_MAX; ++queue_index)
        {
            archer_wfd_queue_t *queue_p = &archer_wfd_g.queue[queue_index];

            bcm_print("\tQueue[%d]: rx_packets %d, rx_packets_max %d, rx_runs %d, rx_packets_per_run %d\n",
                      queue_index, queue_p->stats.rx_packets, queue_p->stats.rx_packets_max,
                      queue_p->stats.rx_runs,
                      queue_p->stats.rx_runs ? queue_p->stats.rx_packets / queue_p->stats.rx_runs : 0);

            memset(&queue_p->stats, 0, sizeof(archer_wfd_queue_stats_t));
        }
#endif
        archer_wfd_hooks.queue_stats();
    }
    else
    {
        bcm_print("WFD is not available\n");
    }
}

static int archer_wfd_bind(void *arg_p)
{
    archer_wfd_hooks_t *hooks_p = (archer_wfd_hooks_t *)arg_p;

    archer_wfd_hooks = *hooks_p;

#if defined(CC_ARCHER_WFD_FORWARD)
    archer_wfd_hooks.queue_notify = archer_wfd_notify;
#endif

    bcm_print("Archer WFD Binding Successfull\n");

    return 0;
}

int __init archer_wfd_construct(void)
{
#if defined(CC_ARCHER_WFD_FORWARD)
    archer_wfd_forward_construct();
#endif
    bcmFun_reg(BCM_FUN_ID_ARCHER_WFD_BIND, archer_wfd_bind);
    bcmFun_reg(BCM_FUN_ID_ARCHER_WFD_CONFIG, archer_wfd_config);

    return 0;
}
