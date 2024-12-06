/*
* <:copyright-BRCM:2015:DUAL/GPL:standard
* 
*    Copyright (c) 2015 Broadcom 
*    All Rights Reserved
* 
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
* 
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
* 
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
* 
* :> 
*/

/*****************************************************************************
 *
 * Copyright (c) 2013 Broadcom Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Author: Peter Sulc <petersu@broadcom.com>
 *	   Tim Ross <tross@broadcom.com>
 *****************************************************************************/
#ifndef __FPM_CLIENT_H_
#define __FPM_CLIENT_H_

u32 fpm_alloc_token(int size);
u8 *fpm_alloc_buffer(int size);
u32 fpm_alloc_token_pool(int pool, int size);
u8 *fpm_alloc_buffer_pool(int pool, int size);
u32 fpm_alloc_max_size_token_pool(int pool);
void fpm_free_token(u32);
void fpm_free_buffer(u8 *);
u32 fpm_incr_multicount(u32);
u8 *fpm_token_to_buffer(u32);
u32 fpm_buffer_to_token(u8 *, u32);
u32 fpm_get_token_size(u32);
int fpm_set_token_size(u32 *, u32);
bool fpm_is_valid_token(u32 token);
void fpm_sync_buffer_for_cpu(u32 token, u32 head, u32 tail, u32 flags);
void fpm_sync_buffer_for_device(u32 token, u32 head, u32 tail, u32 flags);
/* the following 2 APIs convert the token terminology from fpm <-> rdp */
u32 fpm_convert_fpm_token_to_rdp_token(u32 token);
u32 fpm_convert_rdp_token_to_fpm_token(u32 token);

struct fpm_hw_info {
	/* physical address(es) of pool(s) */
	u32 pool_base[2];
	/* physical addresses of alloc_dealloc registers */
	u32 alloc_dealloc[4];
	/* configured chunk size */
	u32 chunk_size;
	/* network buffer head padding */
	u32 net_buf_head_pad;
	/* network buffer tail padding */
	u32 net_buf_tail_pad;
};
void fpm_get_hw_info(struct fpm_hw_info *);
int fpm_is_fpm_buf(void *buf);

/* FPM sync flags */
#define	FPM_SYNC_HEAD	0x1
#define FPM_SYNC_TAIL	0x2	/* ignored if token format doen't have pool ID bits */

struct fpm_pool_stats {
	u32	underflow_count;
	u32	overflow_count;
	u32	tok_avail;
	u32	alloc_fifo_empty;
	u32	alloc_fifo_full;
	u32	free_fifo_empty;
	u32	free_fifo_full;
	u32	pool_full;
	u32	invalid_tok_frees;
	u32	invalid_tok_multi;
	u32	mem_corrupt_tok;
	u32	mem_corrupt_tok_valid;
	u32	invalid_free_tok;
	u32	invalid_free_tok_valid;
	u32	invalid_mcast_tok;
	u32	invalid_mcast_tok_valid;
	u32	tok_avail_low_wtmk;
};
int fpm_get_pool_stats(int pool, struct fpm_pool_stats *stats);
void fpm_reset_bb(bool reset);
void fpm_set_pool_sel_both(void);

/*
 * Token tracking src/dest data is context-specific, but
 * limited to a u32.
 */
union tok_src_dest {
	u32 data;		/* Generic reference to context data */
	u32 rpc_hdr;		/* RPC header when src/dest is RPC */
	struct {		/* interface ID/sub-ID when packet */
#ifdef __LITTLE_ENDIAN__	/* output is a u32 - keep order consistent */
		u8 id;
		u8 sub_id;
		u16 rsvd;
#else
		u16 rsvd;
		u8 sub_id;
		u8 id;
#endif
	} iface;
};

#ifdef CONFIG_BCM_FPM_TOKEN_HIST
void fpm_track_token_rx(u32 token);
void fpm_track_token_src(u32 token, union tok_src_dest *src);
void fpm_track_token_tx(u32 token);
void fpm_track_token_dest(u32 token, union tok_src_dest *dest);
#else
#define fpm_track_token_rx(token)
#define fpm_track_token_src(token, src)
#define fpm_track_token_tx(token)
#define fpm_track_token_dest(token, dest)
#endif
#endif
