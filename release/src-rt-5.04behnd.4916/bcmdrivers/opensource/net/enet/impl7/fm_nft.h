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

/*
 *  Created on: Jan/2023
 *      Author: itai.weiss@broadcom.com
 */ 

#ifndef __FM_NFT_H__
#define __FM_NFT_H__

typedef struct fm_nft_ctx_t fm_nft_ctx_t;

typedef struct bpf_prog *(*xdp_nft_get_progs_by_dev_cb)(struct net_device *dev);

// generic functions
int fm_setup_tc(fm_nft_ctx_t *fm_nft_ctx, enum tc_setup_type type, void *type_data);

int nft_init(fm_nft_ctx_t **_ctx, struct net_device *dev, xdp_nft_get_progs_by_dev_cb get_xdp_prog_cb);

void nft_uninit(fm_nft_ctx_t *fm_nft_ctx);
int fm_nft_stats_update(fm_nft_ctx_t *fm_nft_ctx, unsigned long cookie, u64 packets, u64 bytes, u64 drops);

// xdp specific functions
u32 fm_nft_run_xdp_meta(struct net_device *dev, struct bpf_prog *xdp_prog, struct xdp_buff *xdp, unsigned long *cookie);
extern struct bpf_prog *(*xdp_nft_get_progs_by_dev)(struct net_device *dev);



#endif
