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

#include "port.h"
#include "enet.h"
#include <rdpa_api.h>
#include <linux/of.h>
#include <linux/skbuff.h>
#include <linux/nbuff.h>
#include "enet_dbg.h"
inline int runner_get_pkt_from_ring(int hw_q_id, rdpa_cpu_rx_info_t *info)
{
    return rdpa_cpu_packet_get(rdpa_cpu_host, hw_q_id, info);
}

inline int enetxapi_queue_need_reschedule(enetx_channel *chan, int q_id)
{
/* If the queue got full during handling of packets, new packets will not cause
 * interrupt (they will be dropped without interrupt). In this case, no one
 * will wake up NAPI, ever. */

#ifdef XRDP
/*The solution is to schedule another NAPI round
 * if the queue is full. */
    return rdpa_cpu_queue_is_full(rdpa_cpu_host, chan->rx_q[q_id]);
#else
/*The solution is to schedule another NAPI round
 * if the queue is not empty due to ring implementation we can't get is ring is full in acceptable way. */
    return rdpa_cpu_queue_not_empty(rdpa_cpu_host, chan->rx_q[q_id]);
#endif
}

int runner_ring_create_delete(enetx_channel *chan, int q_id, int size, rdpa_cpu_rxq_cfg_t *rxq_cfg)
{
    rxq_cfg->ring_head = NULL; /* NULL required by RDPA to create AND REMOVE rdp_ring */
    return 0;
}

