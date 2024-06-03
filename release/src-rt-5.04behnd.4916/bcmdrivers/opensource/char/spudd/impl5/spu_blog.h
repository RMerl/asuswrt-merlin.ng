/*
<:copyright-BRCM:2016:GPL/GPL:spu

   Copyright (c) 2016 Broadcom
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
#ifndef __SPU_BLOG_H__
#define __SPU_BLOG_H__

#include "cipher.h"

struct spu_offload_rx_info {
    uint32_t spi;
    uint8_t iv_size;
    uint8_t digestsize;
    uint8_t blog_chan_id;
    uint8_t esp_over_udp;
    union {
        struct sec_path *sp;
        struct dst_entry *dst_p;
	void *ptr;
    };
};

void spu_blog_ctx_add(struct iproc_ctx_s *ctx);
void spu_blog_ctx_del(struct iproc_ctx_s *ctx);
int spu_blog_emit_aead_us(struct iproc_reqctx_s *rctx, int is_esn);
void spu_blog_emit_aead_ds(struct iproc_reqctx_s *rctx);
int spu_blog_register(void);
void spu_blog_unregister(void);

#if defined(CONFIG_BCM_SPU_HW_OFFLOAD)

void spu_blog_offload_register(void);
void spu_blog_offload_deregister(void);
int spu_offload_response_callback (struct bcmspu_offload_resp *resp_info);
struct iproc_ctx_s *spu_blog_lookup_ctx_by_offload_id(uint32_t id);

int spu_update_xfrm_offload_stats(void *xfrm);
void *spu_offload_lookup_dstp_secp_by_id(uint32_t id);
int spu_offload_get_us_id(struct iproc_ctx_s *ctx, uint32_t spi, Blog_t *blog_p);
int spu_offload_insert_us_pkt(pNBuff_t pNBuf, int session_id, uint32_t digest_size, uint32_t *payloadlen);
void spu_offload_get_fixed_hdr(int session_id, uint32_t *hdr_offset, uint8_t *size, uint8_t **hdr);
void spu_offload_session_free(uint32_t session_id);
void spu_offload_handle_exception(uint32_t session_id, uint8_t data_limit, uint8_t overflow);
void spu_blog_evict (struct iproc_ctx_s *ctx);
int spu_offload_priv_info_get(struct net_device *dev, bcm_netdev_priv_info_type_t info_type, bcm_netdev_priv_info_out_t *info_out);
int spu_offload_init(void);
int spu_offload_postinit(void);
void spu_offload_deinit(void);

#define MAX_REPLAY_WIN_SIZE     512 /* limitation due to offload platform memory */


int spu_platform_offload_init(void);
void spu_platform_offload_register(void);
void spu_platform_offload_deregister(void);
int spu_platform_offload_session_us_parm(int session_id, struct xfrm_state *xfrm,
				  struct bcmspu_offload_parm *parm);

int spu_platform_offload_session_ds_parm(int session_id, struct xfrm_state *xfrm,
				  struct bcmspu_offload_parm *parm);
void spu_platform_offload_free_session(uint32_t session_id);
void spu_platform_offload_stats(uint32_t session_id, struct spu_offload_tracker *curr, uint32_t *bitmap, uint8_t long_bitmap);
int spu_offload_get_rx_info(uint32_t session_id, struct spu_offload_rx_info *info);

#endif

#endif /* __SPU_BLOG_H__ */
