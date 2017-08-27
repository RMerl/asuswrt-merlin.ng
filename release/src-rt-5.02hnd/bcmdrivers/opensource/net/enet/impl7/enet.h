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
 *  Created on: Nov/2015
 *      Author: ido@broadcom.com
 */

#ifndef _ENET_H_
#define _ENET_H_

#include "enet_types.h"
#include "port.h"
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#ifdef DT
#include <linux/platform_device.h>
#endif

/* TODO: Allocate number of queues dynamically */
#define RXQ_MAX 8
typedef struct _enetx_channel {
    /* Mapping to HW queue ids */
    int rx_q[RXQ_MAX];
    /* Number of queues in channel */
    int rx_q_count;
    /* Number of Linux interfaces using this channel */
    int open_count;
    /* Linked list to next channel */
    struct _enetx_channel *next;
    /*event queue for thread*/
    wait_queue_head_t rxq_wqh;
    int rxq_cond;
    /*thread handler*/
    struct task_struct *rx_thread;

} enetx_channel;

typedef struct
{
    /* Backwards ptr to port object */
    enetx_port_t *port;
} enetx_netdev;

/* Called from platform ISR implementation */
int enetx_rx_isr(enetx_channel *chan);
/* Called from port init */
int enet_create_netdevice(enetx_port_t *p);
void enet_remove_netdevice(enetx_port_t *p);
int enet_dev_mac_set(enetx_port_t *p, int set);
void enet_dev_flags_update(enetx_port_t *p);

/* External per-platform APIs */

/* read a linked list of fkbs */
int enetxapi_rx_pkt(int queue_id, FkBuff_t **fkb, enetx_rx_info_t *rx_info);
/* Initialize HW queues */
int enetxapi_queues_init(enetx_channel **chan);
/* Uninitialize HW queues */
int enetxapi_queues_uninit(enetx_channel **chan);
/* Disable/clear interrupts on queue */
void enetxapi_queue_int_disable(enetx_channel *chan, int q_id);
/* Enable interrupts on queue */
void enetxapi_queue_int_enable(enetx_channel *chan, int q_id);
/* Return if queue interrupt handler need to be rescheduled */
int enetxapi_queue_need_reschedule(enetx_channel *chan, int q_id);
/* FKB recycle callback */
void enetxapi_fkb_databuf_recycle(FkBuff_t *fkb, void *context);
/* SKB recycle callback */
void enetxapi_buf_recycle(struct sk_buff *skb, uint32_t context, uint32_t flags);
#ifdef DT
/* Open Firmware driver initialization function */
int enetxapi_of_platform_enet_probe(struct platform_device *ofdev);
/* Open Firmware driver cleanup function */
int enetxapi_of_platform_enet_remove(struct platform_device *ofdev);
#endif

#endif

