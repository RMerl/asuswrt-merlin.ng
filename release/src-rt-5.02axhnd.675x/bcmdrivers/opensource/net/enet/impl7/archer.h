/*
   <:copyright-BRCM:2018:DUAL/GPL:standard
   
      Copyright (c) 2018 Broadcom 
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

#ifndef _ARCHER_H_
#define _ARCHER_H_

#include <linux/types.h>

#include <bcm_async_queue.h>
#include <archer_cpu_queues.h>
#include "enet.h"

#define ENET_CPU_RX_QUEUE_SIZE  2048    // chosen to match the interface queue size
#if ((ENET_CPU_RX_QUEUE_SIZE & (ENET_CPU_RX_QUEUE_SIZE-1)) != 0)
#error "ENET_CPU_RX_QUEUE_SIZE is not power of 2"
#endif

#define ENET_CPU_TX_QUEUE_SIZE  512
#if ((ENET_CPU_TX_QUEUE_SIZE & (ENET_CPU_TX_QUEUE_SIZE-1)) != 0)
#error "ENET_CPU_TX_QUEUE_SIZE is not power of 2"
#endif

#define ENET_CPU_RECYCLE_Q_SIZE 512
#if ((ENET_CPU_RECYCLE_Q_SIZE & (ENET_CPU_RECYCLE_Q_SIZE-1)) != 0)
#error "ENET_CPU_RECYCLE_Q_SIZE is not power of 2"
#endif

#define NUM_RX_QUEUES   2
#if (NUM_RX_QUEUES > CPU_RX_QUEUE_MAX)
#error "NUM_RX_QUEUES larger than supported"
#endif
// Only 1 TX queue is required since priority is not differentiated
#define NUM_TX_QUEUES   2
#if (NUM_TX_QUEUES > CPU_TX_QUEUE_MAX)
#error "NUM_TX_QUEUES larger than supported"
#endif

#define CC_ENET_CPU_QUEUE_STATS


#if defined(CC_ENET_CPU_QUEUE_STATS)
#define ENET_CPU_STATS_UPDATE(_counter) ( (_counter)++ )
#else
#define ENET_CPU_STATS_UPDATE(_counter)
#endif

typedef struct cpu_queues_t
{
    bcm_async_queue_t rxq[NUM_RX_QUEUES]; 
    bcm_async_queue_t txq[NUM_TX_QUEUES];

    spinlock_t tx_lock[NUM_TX_QUEUES];
    TX_NOTIFIER tx_notifier[NUM_TX_QUEUES];

    int rx_notify_enable;
    int rx_notify_pending_disable;

    enetx_channel * chanp;

    /* Buffer Recycling Thread */
    volatile unsigned long recycle_work_avail;
    wait_queue_head_t recycle_thread_wqh;
    struct task_struct *recycle_thread;
    bcm_async_queue_t recycleq;

} cpu_queues_t;

extern cpu_queues_t * enet_cpu_queues;
extern int cpu_queues_tx_send (int send_q, dispatch_info_t *dispatch_info);

#endif //_ARCHER_H_

