/*
   <:copyright-BRCM:2023:DUAL/GPL:standard
   
      Copyright (c) 2023 Broadcom 
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

/*
 *  Created on: Jan/2023
 *      Author: lode.lathouwers@broadcom.com
 */


#ifndef _ENET_XDP_H_
#define _ENET_XDP_H_

#include <linux/netdevice.h>
#include "port.h"

struct enet_xdp_stats {
    u64 xdp_packets;
    u64 xdp_bytes;
    u64 xdp_pass;
    u64 xdp_redirect;
    u64 xdp_drops;
    u64 xdp_tx;
    u64 xdp_tx_err;
    u64 xdp_xmit;
    u64 xdp_xmit_err;
};

int enet_xdp_xmit(struct net_device *dev, int n,
                         struct xdp_frame **frames, u32 flags);

int enet_xdp(struct net_device *dev, struct netdev_bpf *xdp);

typedef enum {
	ENET_XDP_INGRESS_REDIRECT,
	ENET_XDP_INGRESS_PASS,
	ENET_XDP_INGRESS_DROP,
} enet_xdp_ingress_result_t ;

enet_xdp_ingress_result_t enet_xdp_handle_ingress(struct net_device *dev, FkBuff_t *fkb, struct enet_xdp_stats * stats);

struct bpf_prog *enet_xdp_get_progs_by_dev(struct net_device *dev);

#endif
