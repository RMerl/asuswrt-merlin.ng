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
