/*
   <:copyright-BRCM:2023:DUAL/GPL:standard
   
      Copyright (c) 2023 Broadcom 
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
