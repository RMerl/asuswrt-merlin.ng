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

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/io.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/dma-attrs.h>
#include <linux/dma-mapping.h>
#include <linux/vmalloc.h>
#include <asm/cacheflush.h>
#include <linux/irqreturn.h>
#include <linux/interrupt.h>

#include "fpm.h"
#include "fpm_dev.h"
#include "fpm_priv.h"
#include "fpm_dt.h"
#include "fpm_proc.h"

#include "pmc_fpm.h"

#ifndef CONFIG_BCM_FPM_DT
#include "board.h"
#include "bcm_rsvmem.h"
#include "bcm_map_part.h"
#include "bcm_intr.h"
#endif

#define FPM_POOL_SIZE_FOR_512B_TOKEN	SZ_32M
#define FPM_POOL_SIZE_FOR_256B_TOKEN	SZ_16M
#ifdef CONFIG_BCM_JUMBO_FRAME
#define FPM_POOL_SIZE_REQ	FPM_POOL_SIZE_FOR_512B_TOKEN
#else
#define FPM_POOL_SIZE_REQ	FPM_POOL_SIZE_FOR_256B_TOKEN
#endif

/*
 * Definitions & Clarifications
 *
 * The FPM hardware register definitions seem to overload the meaning of
 * "pool" to the point of complete confusion. "Pool" is used to refer to:
 * a) all of the memory allotted to the FPM hardware, b) a portion of that
 * memory used to allocate buffers of a particular size (256, 512, 1024,
 * 2048, 4096) even though buffers of a particular size may be located
 * anywhere within the FPM memory, and c) beginning with the 3390, the
 * portion of the memory allotted to the FPM that is within a particular
 * DDR bank, even though the memory may actually be in the same DDR bank.
 * In an attempt to eliminate this confusion I have deviated from the
 * hardware definitions of "pool" and use the following terms throughout
 * the code instead:
 *
 * pool:
 * A contiguous area of DDR from which the FPM hardware may allocate.
 * Chips prior to the 3390 had 1 pool, while the 3390 introduced 2 pools
 * which may be in the same or different DDR hardware interfaces.
 *
 * chunk:
 * The smallest allocation size available from the pool. Currently this
 * can be either 256 or 512 bytes.
 *
 * buffer:
 * Memory is allocated by the FPM as one or more contiguous chunks within
 * a given pool, hereafter called a buffer. Currently a buffer consists
 * of 2^n chunks where n = 0, 1, 2, or 3. With the introduction of multiple
 * pools the FPM hardware allows allocations to be made from a specific pool
 * or from any pool. If a pool is not specified then an internal algorithm
 * is used to pick the pool.
 *
 * token size:
 * Confusion also seems to arise when referring to the size field in the
 * token. While the size of a buffer is always a multiple of chunks, the
 * token size field may or may not be the full size of the buffer. When the
 * buffer is first allocated the token size field will equal the size of the
 * buffer (NOTE: There is a bug in all chips prior to the 3390B0 that results
 * in the size field of the token returned upon allocation to indicate the
 * size of the buffer as a multiple of 256 byte chunks regardless of whether
 * the FPM has been configured for a 256 or 512 byte chunk size). When
 * interfacing with hardware the token size field indicates the size of
 * the data in the buffer.
 *
 */

/* Global lock for all register accesses. */
spinlock_t fpm_reg_lock;

struct fpmdev *fpm;
unsigned long isr_timer_period = FPM_ISR_TIMER_PERIOD;

u32 fpm_alloc_token(int size)
{
	u32 token;

	pr_debug("-->\n");

	token = __fpm_alloc_token(size);
	fpm_check_token(token);
	fpm_track_token_op(fpm_alloc_token, token, 0);
	pr_debug("%s(0x%08x)<-%pf\n", __func__, token,
		 __builtin_return_address(0));

	pr_debug("<--\n");
	return token;
}
EXPORT_SYMBOL(fpm_alloc_token);

u8 *fpm_alloc_buffer(int size)
{
	u8 *buf = NULL;
	u32 token;

	pr_debug("-->\n");
	token = __fpm_alloc_token(size);
	fpm_check_token(token);
	fpm_track_token_op(fpm_alloc_buffer, token, 0);
	pr_debug("%s(0x%08x)<-%pf\n", __func__, token,
		 __builtin_return_address(0));
	if (token)
		buf = __fpm_token_to_buffer(token);

	pr_debug("<--\n");
	return buf;
}
EXPORT_SYMBOL(fpm_alloc_buffer);

u32 fpm_alloc_token_pool(int pool, int size)
{
	u32 token;

	token = __fpm_alloc_token_pool(pool, size);
	fpm_check_token(token);
	fpm_track_token_op(fpm_alloc_token_pool, token, 0);
	pr_debug("%s(0x%08x)<-%pf\n", __func__, token,
		 __builtin_return_address(0));

	return token;
}
EXPORT_SYMBOL(fpm_alloc_token_pool);

u8 *fpm_alloc_buffer_pool(int pool, int size)
{
	u8 *buf = NULL;
	u32 token;

	token = __fpm_alloc_token_pool(pool, size);
	fpm_check_token(token);
	fpm_track_token_op(fpm_alloc_buffer_pool, token, 0);
	pr_debug("%s(0x%08x)<-%pf\n", __func__, token,
		 __builtin_return_address(0));
	if (token)
		buf = __fpm_token_to_buffer(token);

	return buf;
}
EXPORT_SYMBOL(fpm_alloc_buffer_pool);

u32 fpm_alloc_max_size_token_pool(int pool)
{
	u32 token;

	token = __fpm_alloc_token_pool(pool, (fpm->chunk_size << 3));
	fpm_check_token(token);
	fpm_track_token_op(fpm_alloc_max_size_token_pool, token, 0);
	pr_debug("%s(0x%08x)<-%pf\n", __func__, token,
		 __builtin_return_address(0));

	return token;
}
EXPORT_SYMBOL(fpm_alloc_max_size_token_pool);

u32 fpm_incr_multicount(u32 token)
{
#ifdef CONFIG_BCM_FPM_TOKEN_HIST
	struct fpmdev *fdev = fpm;
	u32 tok_idx;
#endif
	int incr = 0;
	u32 fpm_local_multi;

	pr_debug("-->\n");

	fpm_check_token(token);
	fpm_track_token_op(fpm_incr_multicount, token, 0);
	pr_debug("%s(0x%08x)<-%pf\n", __func__, token,
		 __builtin_return_address(0));
	fpm_local_multi = (token & ~FPM_TOKEN_MULTIVAL_MASK);
	fpm_local_multi |= (1 << FPM_TOKEN_MULTIINCR_SHIFT);
	fpm_local_multi++;

	if (fpm_local_multi & FPM_TOKEN_VALID_MASK) {
		fpm_reg_write(FPM_POOL_MULTI, fpm_local_multi);
		incr = 1;
	}

#ifdef CONFIG_BCM_FPM_TOKEN_HIST
	if (fdev->track_tokens) {
		tok_idx = FPM_TOKEN_INDEX(token);
		tok_idx |= (FPM_TOKEN_POOL(token) <<
			   (FPM_TOKEN_INDEX_SHIFT + 1));
#ifdef CONFIG_BCM_FPM_TOKEN_HIST_CHECKING
		if (!fdev->tok_ref_count[tok_idx]) {
			pr_err("Multicount increment of token (0x%08x)", token);
			pr_cont(" with zero ref count (%d).\n",
				fdev->tok_ref_count[tok_idx]);
			fdev->track_tokens = fdev->track_on_err;
		}
#endif
		fdev->tok_ref_count[tok_idx] += incr;
	}
#endif

	pr_debug("<--\n");
	return incr;
}
EXPORT_SYMBOL(fpm_incr_multicount);

void fpm_free_token(u32 token)
{
	pr_debug("-->\n");

	fpm_check_token(token);
	fpm_track_token_op(fpm_free_token, token, 0);
	pr_debug("%s(0x%08x)<-%pf\n", __func__, token,
		 __builtin_return_address(0));
	__fpm_free_token(token);

	pr_debug("<--\n");
}
EXPORT_SYMBOL(fpm_free_token);

void fpm_free_buffer(u8 *buf)
{
	u32 token;

	pr_debug("-->\n");

	token = __fpm_buffer_to_token(buf, 0);
	fpm_check_token(token);
	fpm_track_token_op(fpm_free_buffer, token, 0);
	pr_debug("%s(0x%08x)<-%pf\n", __func__, token,
		 __builtin_return_address(0));
	__fpm_free_token(token);

	pr_debug("<--\n");
}
EXPORT_SYMBOL(fpm_free_buffer);

u8 *fpm_token_to_buffer(u32 token)
{
	pr_debug("-->\n");

	fpm_check_token(token);
	fpm_track_token_op(fpm_token_to_buffer, token, 0);
	pr_debug("%s(0x%08x)<-%pf\n", __func__, token,
		 __builtin_return_address(0));

	pr_debug("<--\n");
	return __fpm_token_to_buffer(token);
}
EXPORT_SYMBOL(fpm_token_to_buffer);

u32 fpm_buffer_to_token(u8 *buf, u32 size)
{
	u32 token;

	pr_debug("-->\n");

	token = __fpm_buffer_to_token(buf, size);
	fpm_check_token(token);
	fpm_track_token_op(fpm_buffer_to_token, token, 0);
	pr_debug("%s(0x%08x)<-%pf\n", __func__, token,
		 __builtin_return_address(0));

	pr_debug("<--\n");
	return token;
}
EXPORT_SYMBOL(fpm_buffer_to_token);

u32 fpm_get_token_size(u32 token)
{
	pr_debug("-->\n");

	fpm_check_token(token);
	fpm_track_token_op(fpm_get_token_size, token, 0);
	pr_debug("%s(0x%08x)<-%pf\n", __func__, token,
		 __builtin_return_address(0));

	pr_debug("<--\n");
	return __fpm_get_token_size(token);
}
EXPORT_SYMBOL(fpm_get_token_size);

int fpm_set_token_size(u32 *token, u32 size)
{
	int status = 0;

	pr_debug("-->\n");

	if (!token) {
		pr_err("%s: NULL token ptr\n", __func__);
		dump_stack();
		status = -EINVAL;
		goto done;
	}
	fpm_check_token(*token);
	fpm_track_token_op(fpm_set_token_size, *token, 0);
	*token &= ~FPM_TOKEN_SIZE_MASK;
	*token |= (size & FPM_TOKEN_SIZE_MASK);
	pr_debug("%s(0x%08x)<-%pf\n", __func__, *token,
		 __builtin_return_address(0));

done:
	pr_debug("<--\n");
	return status;
}
EXPORT_SYMBOL(fpm_set_token_size);

bool fpm_is_valid_token(u32 token)
{
	return (token & FPM_TOKEN_VALID_MASK);
}
EXPORT_SYMBOL(fpm_is_valid_token);


/* this convert the token terminology from fpm to rdp */
u32 fpm_convert_fpm_token_to_rdp_token(u32 token)
{
	return (FPM_TOKEN_INDEX(token) |
		(FPM_TOKEN_POOL(token) << FPM_TOKEN_INDEX_WIDTH));
}
EXPORT_SYMBOL(fpm_convert_fpm_token_to_rdp_token);

u32 fpm_convert_rdp_token_to_fpm_token(u32 token)
{
	u32 pool = token >> FPM_TOKEN_INDEX_WIDTH;
	u32 fpm_token = (token << FPM_TOKEN_INDEX_SHIFT) & FPM_TOKEN_INDEX_MASK;
	fpm_token |= FPM_TOKEN_VALID_MASK | (pool << FPM_TOKEN_POOL_SHIFT);

	return fpm_token;
}
EXPORT_SYMBOL(fpm_convert_rdp_token_to_fpm_token);

void fpm_sync_buffer_for_cpu(u32 token, u32 head, u32 tail, u32 flags)
{
	u32 size;
	u8 *start;
	dma_addr_t paddr;

	pr_debug("-->\n");

	fpm_check_token(token);
	fpm_track_token_op(fpm_sync_buffer_for_cpu, token, 0);
	pr_debug("%s(0x%08x)<-%pf\n", __func__, token,
		 __builtin_return_address(0));
	fpm_get_sync_start_size(token, head, tail, flags, &start, &size);
	/* TODO! this can probably be improved by using cache op defined
	 * in nbuff.h */
	paddr = BcmMemReserveVirtToPhys(fpm->pool_vbase[FPM_TOKEN_POOL(token)], 
		fpm->pool_pbase[FPM_TOKEN_POOL(token)], start);

	dma_sync_single_for_cpu(NULL, paddr, size, DMA_FROM_DEVICE);

	pr_debug("<--\n");
}
EXPORT_SYMBOL(fpm_sync_buffer_for_cpu);

void fpm_sync_buffer_for_device(u32 token, u32 head, u32 tail, u32 flags)
{
	u32 size;
	u8 *start;
	dma_addr_t paddr;

	pr_debug("-->\n");

	fpm_check_token(token);
	fpm_track_token_op(fpm_sync_buffer_for_device, token, 0);
	pr_debug("%s(0x%08x)<-%pf\n", __func__, token,
		 __builtin_return_address(0));
	fpm_get_sync_start_size(token, head, tail, flags, &start, &size);
	/* TODO! this can probably be improved by using cache op defined
	 * in nbuff.h */
	paddr = BcmMemReserveVirtToPhys(fpm->pool_vbase[FPM_TOKEN_POOL(token)], 
		fpm->pool_pbase[FPM_TOKEN_POOL(token)], start);

	dma_sync_single_for_device(NULL, paddr, size, DMA_TO_DEVICE);

	pr_debug("<--\n");
}
EXPORT_SYMBOL(fpm_sync_buffer_for_device);

void fpm_get_hw_info(struct fpm_hw_info *hw_info)
{
	pr_debug("-->\n");

	hw_info->pool_base[0] = (u32)fpm->pool_pbase[0];
#if FPM_POOL_NUM > 1
	hw_info->pool_base[1] = (u32)fpm->pool_pbase[1];
#endif
	hw_info->alloc_dealloc[0] = (u32)FPM_ALLOC_8_CHUNKS_PHYS;
	hw_info->alloc_dealloc[1] = (u32)FPM_ALLOC_4_CHUNKS_PHYS;
	hw_info->alloc_dealloc[2] = (u32)FPM_ALLOC_2_CHUNKS_PHYS;
	hw_info->alloc_dealloc[3] = (u32)FPM_ALLOC_1_CHUNKS_PHYS;
	hw_info->chunk_size = fpm->chunk_size;
	hw_info->net_buf_head_pad = FPM_NET_BUF_HEAD_PAD;
	hw_info->net_buf_tail_pad = FPM_NET_BUF_TAIL_PAD;

	pr_debug("<--\n");
}
EXPORT_SYMBOL(fpm_get_hw_info);

int fpm_is_fpm_buf(void *buf)
{
	int is_fpm = 1;

	pr_debug("-->\n");

	if (fpm_buffer_to_pool(buf) < 0)
		is_fpm = 0;

	pr_debug("<--\n");
	return is_fpm;
}
EXPORT_SYMBOL(fpm_is_fpm_buf);

int fpm_get_pool_stats(int pool, struct fpm_pool_stats *stats)
{
	pr_debug("-->\n");

	stats->underflow_count		= FPM_GET_UNDERFLOW(pool);
	stats->overflow_count		= FPM_GET_OVERFLOW(pool);
	stats->tok_avail		= FPM_GET_TOK_AVAIL(pool);
	stats->alloc_fifo_empty		= FPM_GET_ALLOC_FIFO_EMPTY(pool);
	stats->alloc_fifo_full		= FPM_GET_ALLOC_FIFO_FULL(pool);
	stats->free_fifo_empty		= FPM_GET_FREE_FIFO_EMPTY(pool);
	stats->free_fifo_full		= FPM_GET_FREE_FIFO_FULL(pool);
	stats->pool_full		= FPM_GET_POOL_FULL(pool);
	stats->invalid_tok_frees	= FPM_GET_INVAL_TOK_FREES(pool);
	stats->invalid_tok_multi	= FPM_GET_INVAL_TOK_MULTI(pool);
	stats->mem_corrupt_tok		= FPM_GET_MEM_CORRUPT_TOK(pool);
	stats->mem_corrupt_tok_valid	=
		FPM_GET_MEM_CORRUPT_TOK_VALID(pool) == 1;
	stats->invalid_free_tok		= FPM_GET_INVALID_FREE_TOK(pool);
	stats->invalid_free_tok_valid	=
		FPM_GET_INVALID_FREE_TOK_VALID(pool) == 1;
	stats->invalid_mcast_tok	= FPM_GET_INVALID_MCAST_TOK(pool);
	stats->invalid_mcast_tok_valid	=
		FPM_GET_INVALID_MCAST_TOK_VALID(pool) == 1;
	stats->tok_avail_low_wtmk	= FPM_GET_TOK_AVAIL_LOW_WTMK(pool);

	pr_debug("<--\n");
	return 0;
}
EXPORT_SYMBOL(fpm_get_pool_stats);

void fpm_reset_bb(bool reset)
{
	pr_debug("-->\n");

	FPM_SET_BB_RESET(reset ? 1 : 0);

	pr_debug("<--\n");
}
EXPORT_SYMBOL(fpm_reset_bb);

void fpm_set_pool_sel_both(void)
{
#if FPM_POOL_NUM > 1
	if (FPM_GET_POOL_INIT(1))
		FPM_SET_BB_POOL_SEL(FPM_BB_POOL_SEL_POOL_BOTH);
#endif
}
EXPORT_SYMBOL(fpm_set_pool_sel_both);

#if STOP_UBUS_CAPTURE
static void fpm_stop_ubus_capture(void)
{
	int i;
	u32 *r;

	pr_err("Stopping UBUS capture\n");
	for (i = 0, r = fpm->cap_vbase[0] + (0x408 >> 2);
		i < 8;
		i++, r += (0x100 >> 2)) {
		fpm_reg_write(r, 1 << 24);
	}
	for (i = 0, r = fpm->cap_vbase[1] + (0x408 >> 2);
		i < 12;
		i++, r += (0x100 >> 2)) {
		fpm_reg_write(r, 1 << 24);
	}
}
#endif

static void fpm_isr_timer_timeout(unsigned long data)
{
#if FPM_POOL_NUM > 1
	struct fpmdev *fdev = (struct fpmdev *)data;
#endif

	FPM_SET_IRQ_MASK(0, FPM_DEV_IRQ_MASK);
#if FPM_POOL_NUM > 1
	if (fdev->pool_pbase[1])
		FPM_SET_IRQ_MASK(1, FPM_DEV_IRQ_MASK);
#endif
	return;
}

static irqreturn_t fpm_isr(int irq, void *dev)
{
	struct fpmdev *fdev = fpm;
	int pool;
	u32 status;
	u32 mask;

	pr_debug("IRQ -->\n");

	for (pool = 0; pool < 2; pool++) {
		mask = FPM_GET_IRQ_MASK(pool);
		status = mask & FPM_GET_IRQ_STATUS(pool);
		if (!status)
			continue;
		if (status & FPM_ALLOC_FIFO_FULL_MASK) {
			pr_err("FPM Pool %d: alloc fifo full\n", pool);
			fdev->track_tokens = fdev->track_on_err;
		}
		if (status & FPM_FREE_FIFO_FULL_MASK) {
			pr_err("FPM Pool %d: free fifo full\n", pool);
			fdev->track_tokens = fdev->track_on_err;
		}
		if (status & FPM_POOL_FULL_MASK) {
			pr_err("FPM Pool %d: pool full\n", pool);
			fdev->track_tokens = fdev->track_on_err;
		}
		if (status & FPM_FREE_TOKEN_NO_VALID_MASK) {
			if (FPM_GET_INVALID_FREE_TOK_VALID(pool)) {
				pr_err("FPM Pool %d: invalid token 0x%08x freed\n",
				       pool, FPM_GET_INVALID_FREE_TOK(pool));
				FPM_CLEAR_INVALID_FREE_TOK(pool);
			} else {
				pr_err("FPM Pool %d: invalid token freed\n",
				       pool);
			}
			fdev->track_tokens = fdev->track_on_err;
		}
		if (status & FPM_FREE_TOKEN_INDEX_OUT_OF_RANGE_MASK) {
			if (FPM_GET_INVALID_FREE_TOK_VALID(pool)) {
				pr_err("FPM Pool %d: token 0x%08x ", pool,
				       FPM_GET_INVALID_FREE_TOK(pool));
				pr_err("freed with out of range index\n");
				FPM_CLEAR_INVALID_FREE_TOK(pool);
			} else {
				pr_err("FPM Pool %d: token ", pool);
				pr_err("freed with out of range index\n");
			}
			fdev->track_tokens = fdev->track_on_err;
		}
		if (status & FPM_MULTI_TOKEN_NO_VALID_MASK) {
			if (FPM_GET_INVALID_MCAST_TOK_VALID(pool)) {
				pr_err("FPM Pool %d: multicount update on ",
				       pool);
				pr_err("invalid token %08x\n",
				       FPM_GET_INVALID_MCAST_TOK(pool));
				FPM_CLEAR_INVALID_MCAST_TOK(pool);
			} else {
				pr_err("FPM Pool %d: multicount update on ",
				       pool);
				pr_err("invalid token\n");
			}
			fdev->track_tokens = fdev->track_on_err;
		}
		if (status & FPM_MULTI_TOKEN_INDEX_OUT_OF_RANGE_MASK) {
			if (FPM_GET_INVALID_MCAST_TOK_VALID(pool)) {
				pr_err("FPM Pool %d: multicount update with ",
				       pool);
				pr_err("out of range token index 0x%08x\n",
				       FPM_GET_INVALID_MCAST_TOK(pool));
				FPM_CLEAR_INVALID_MCAST_TOK(pool);
			} else {
				pr_err("FPM Pool %d: multicount update with ",
				       pool);
				pr_err("out of range token index\n");
			}
			fdev->track_tokens = fdev->track_on_err;
		}
		if (status & FPM_POOL_DIS_FREE_MULTI_MASK) {
			pr_err("FPM Pool %d: free or multicount ", pool);
			pr_err("update on disabled pool\n");
#if STOP_UBUS_CAPTURE
			fpm_stop_ubus_capture();
#endif
			fdev->track_tokens = fdev->track_on_err;
		}
		if (status & FPM_MEMORY_CORRUPT_MASK) {
			pr_err("FPM Pool %d: index memory corrupt\n", pool);
			fdev->track_tokens = fdev->track_on_err;
		}
		if (status & FPM_XOFF_MASK)
			pr_err("FPM Pool %d: XOFF state\n", pool);
		if (status & FPM_XON_MASK)
			pr_err("FPM Pool %d: XON state\n", pool);
		if (status & FPM_ILLEGAL_ADDRESS_ACCESS_MASK) {
			pr_err("FPM Pool %d: illegal address access attempt\n",
			       pool);
			fdev->track_tokens = fdev->track_on_err;
		}
		if (status & FPM_ILLEGAL_ALLOC_REQUEST_MASK) {
			pr_err("FPM Pool %d: illegal alloc request\n", pool);
			fdev->track_tokens = fdev->track_on_err;
		}

		if (isr_timer_period != 0) {
			pr_info("FPM Pool %d: ISR timer is enabled. There "
			        "could be multiple occurrences of the "
				"reported issue\n", pool);
			mask &= ~status;
			FPM_SET_IRQ_MASK(pool, mask);
			if (!timer_pending(&fdev->isr_timer))
				mod_timer(&fdev->isr_timer, jiffies +
					  msecs_to_jiffies(isr_timer_period));
		}

		FPM_CLEAR_IRQ_STATUS(pool, status);
	}

	pr_debug("IRQ <--\n");
	return IRQ_HANDLED;
}


#ifndef CONFIG_BCM_FPM_DT
static int fpm_resource_alloc(struct platform_device *pdev)
{
	struct fpmdev *fdev = pdev->dev.platform_data;
	void *pool0_vbase;
	unsigned int pool0_size;
	phys_addr_t phy_addr;

	if (BcmMemReserveGetByName(BUFFER_MEMORY_BASE_ADDR_STR, &pool0_vbase, &phy_addr,
				   &pool0_size))
		return -EFAULT;
	fdev->pool_vbase[0]  = (u32 *)pool0_vbase;
	fdev->pool_pbase[0] = phy_addr;
	fdev->pool_size[0] = pool0_size;

	fdev->reg_pbase = pdev->resource[0].start;
	fdev->reg_vbase = ioremap(pdev->resource[0].start,
			pdev->resource[0].end - pdev->resource[0].start + 1);
	if (fdev->reg_vbase == NULL)
		return -EFAULT;

	fdev->irq = pdev->resource[1].start;

	/* current the following values are hardcoded based on the default
	 * 3390 device tree value */
	fdev->init = 1;		/* have FPM driver does the initialization */
	fdev->track_tokens = 1;
	fdev->track_on_err = 0;
	fdev->pool_alloc_weight[0] = 1;
	fdev->pool_free_weight[0] = 1;
#if FPM_POOL_NUM > 1
	fdev->pool_alloc_weight[1] = 1;
	fdev->pool_free_weight[1] = 1;
#endif

	return 0;
}
#endif

/* TODO!! once we implement how to trim down the number of token per
 * fpm pool, then we update the following check */
static int fpm_pool_size_check_adjust(struct platform_device *pdev)
{
	struct fpmdev *fdev = pdev->dev.platform_data;
	int remaining_size, pool_idx, pool_size;

	if (fdev->pool_size[0] < (FPM_POOL_SIZE_REQ * FPM_POOL_NUM)) {
		pr_err("FPM fails to initialize %d pool[s], because it requires "
		       "%d MB, and system has only reserved %d MB.\n  "
		       "Initialization will fail!\n", FPM_POOL_NUM, 
		       (FPM_POOL_SIZE_REQ >> 20) * FPM_POOL_NUM,
		       fdev->pool_size[0] >> 20);
		return -EFAULT;
	}

	if (fdev->pool_size[0] >= (FPM_POOL_SIZE_FOR_512B_TOKEN * FPM_POOL_NUM)) {
		pool_size = FPM_POOL_SIZE_FOR_512B_TOKEN;
		fdev->chunk_size = 512;
	} else {
		pool_size = FPM_POOL_SIZE_FOR_256B_TOKEN;
		fdev->chunk_size = 256;
	}

	remaining_size = fdev->pool_size[0] - (pool_size * FPM_POOL_NUM);
	fdev->pool_size[0] = pool_size;

	for (pool_idx = 1; pool_idx < FPM_POOL_NUM; pool_idx++) {
		fdev->pool_size[pool_idx] = pool_size;
		fdev->pool_vbase[pool_idx] = (u32 *)((uintptr_t)fdev->pool_vbase[0] +
						(pool_size * pool_idx));
		fdev->pool_pbase[pool_idx] = BcmMemReserveVirtToPhys(fdev->pool_vbase[0],
						fdev->pool_pbase[0], fdev->pool_vbase[pool_idx]);
	}

	pr_info("FPM will initialize %d pool[s] with token size set to %dB.  This requires %dMB\n",
		FPM_POOL_NUM, fdev->chunk_size, (pool_size * FPM_POOL_NUM) >> 20);

	if (remaining_size != 0)
		pr_info("FPM: there are some unused/wasted memory: %d MB\n",
			remaining_size >> 20);

	return 0;
}

static int fpm_probe(struct platform_device *pdev)
{
	int status = 0;
	struct fpmdev *fdev;
	u32 pool_pbase;
	/* TODO! double check whether 4908 need this workaround or not */
#if 0
	int i;
#endif
#ifdef CONFIG_BCM_FPM_TOKEN_HIST
	u32 num_tokens;
#endif

	pr_debug("-->\n");

	if (fpm) {
		pr_err("%s Exceeded max FPM devices.\n", __func__);
		status = -EFAULT;
		goto done;
	}

	fdev = kmalloc(sizeof(struct fpmdev), GFP_KERNEL);
	if (!fdev) {
		status = -ENOMEM;
		goto done;
	}
	memset(fdev, 0, sizeof(struct fpmdev));

	fpm = fdev;
	pdev->dev.platform_data = fdev;
	fdev->pdev = pdev;

#ifdef CONFIG_BCM_FPM_DT
	status = fpm_parse_dt_node(pdev);
#else
	status = fpm_resource_alloc(pdev);
#endif
	if (status)
		goto err_free_fdev;

	status = fpm_pool_size_check_adjust(pdev);
	if (status)
		goto err_free_fdev;

	/* this shouldn't be the usual case, because RDP should've initialized it */
	if (fdev->init && FPM_GET_POOL_BASE_ADDR(0) == 0) {
		/*
		 * Put Broadbus into reset (if it exists on this chip). BB will
		 * be taken out of reset by the Runner drivers once they have
		 * finished initializing the BPM registers.
		 */
		fpm_reset_bb(true);

		/*
		 * Init the pool(s) in the FPM.  This creates the
		 * TOKEN_INDEX values internally
		 */
		FPM_SET_POOL_INIT(0);
#if FPM_POOL_NUM > 1
		if (fdev->pool_pbase[1])
			FPM_SET_POOL_INIT(1);
#endif

		/* Wait for the pool(s) to init properly */
		while (FPM_GET_POOL_INIT(0))
			msleep(20);
#if FPM_POOL_NUM > 1
		if (fdev->pool_pbase[1])
			while (FPM_GET_POOL_INIT(1))
				msleep(20);
#endif

		/*
		 * Enable the FPM interrupt masks
		 * TODO - we can look at this later when we decide who wants to
		 * take/service interrupts from FPM.
		 */

		/* Activate the FPM pool(s). */
		FPM_SET_POOL_ENABLE(0);
#if FPM_POOL_NUM > 1
		if (fdev->pool_pbase[1])
			FPM_SET_POOL_ENABLE(1);
#endif

		/* Set the chunk size to 512. */
		if (fdev->chunk_size == 512)
			FPM_SET_CHUNK_SIZE(FPM_CHUNK_SIZE_512);
		else /* Set the chunk size to 256. */
			FPM_SET_CHUNK_SIZE(FPM_CHUNK_SIZE_256);

		/* Store the pad values for everyone to use. */
		FPM_SET_CTL_SPARE((FPM_NET_BUF_HEAD_PAD << 16) |
				  FPM_NET_BUF_TAIL_PAD);

		/* Set the alloc & free weights. */
#if FPM_POOL_NUM > 1
		if (fdev->pool_pbase[1]) {
			FPM_SET_ALLOC_WEIGHT(0, fdev->pool_alloc_weight[0]);
			FPM_SET_FREE_WEIGHT(0, fdev->pool_free_weight[0]);
			FPM_SET_ALLOC_WEIGHT(1, fdev->pool_alloc_weight[1]);
			FPM_SET_FREE_WEIGHT(1, fdev->pool_free_weight[1]);
		}
#endif

		/*
		 * Now that FPM has been initialized, set the pool base address
		 * register(s) so that everyone else in the system (RG and CM
		 * subsystems, IOP's, etc) are aware that they can start using
		 * it.
		 */
		pool_pbase = fdev->pool_pbase[0];
		FPM_SET_POOL_BASE_ADDR(0, pool_pbase);

		/*
		 * Always write pool 1 even if it's not being used to make sure
		 * it's 0.
		 */
#if FPM_POOL_NUM > 1
		pool_pbase = fdev->pool_pbase[1];
		FPM_SET_POOL_BASE_ADDR(1, pool_pbase);
#else
		pool_pbase = 0;
		FPM_SET_POOL_BASE_ADDR(1, pool_pbase);
#endif

		if (fdev->irq > 0) {
			status = request_irq(fdev->irq, fpm_isr, IRQF_SHARED,
					     "FPM", (void *)fdev);
			if (status) {
				pr_err("Failed registering IRQ with status ");
				pr_err("%d!\n", status);
				goto err_free_fdev;
			}

			FPM_CLEAR_IRQ_STATUS(0, 0xffffffff);
			FPM_CLEAR_IRQ_STATUS(1, 0xffffffff);
			FPM_SET_IRQ_MASK(0, FPM_DEV_IRQ_MASK);
#if FPM_POOL_NUM > 1
			if (fdev->pool_pbase[1])
				FPM_SET_IRQ_MASK(1, FPM_DEV_IRQ_MASK);
#endif

			/* timer for absorbing ISR */
			init_timer(&fdev->isr_timer);
			setup_timer(&fdev->isr_timer, fpm_isr_timer_timeout,
				    (unsigned long)fdev);

#if STOP_UBUS_CAPTURE
			fdev->cap_vbase[0] = ioremap(fdev->cap_pbase[0],
						     fdev->cap_size[0]);
			fdev->cap_vbase[1] = ioremap(fdev->cap_pbase[1],
						     fdev->cap_size[1]);
			if (!fdev->cap_vbase[0]) {
				pr_err("%s Unable to ioremap UBUS ", __func__);
				pr_err("capture engines 0.\n");
				status = -EFAULT;
				goto err_free_fdev;
			}
			if (!fdev->cap_vbase[1]) {
				pr_err("%s Unable to ioremap UBUS ", __func__);
				pr_err("capture engines 1.\n");
				status = -EFAULT;
				goto err_free_fdev;
			}
#endif
		}
	}

	fdev->pool_pbase[0] = FPM_GET_POOL_BASE_ADDR(0);
#if FPM_POOL_NUM > 1
	fdev->pool_pbase[1] = FPM_GET_POOL_BASE_ADDR(1);
#endif

#ifdef CREATE_VIRTUAL_ADDRESS_MAPPING
	/* This is not the current case for 4908 implementation, which
	 * the board driver should've mapped the memory to uncached */
#if FPM_CACHED
	fdev->pool_vbase[0] = ioremap_cache(fdev->pool_pbase[0],
					    fdev->pool_size[0]);
#if FPM_POOL_NUM > 1
	if (fdev->pool_pbase[1])
		fdev->pool_vbase[1] = ioremap_cache(fdev->pool_pbase[1],
						    fdev->pool_size[1]);
	else
		fdev->pool_vbase[1] = NULL;
#endif
#else
	fdev->pool_vbase[0] = ioremap(fdev->pool_pbase[0],
				      fdev->pool_size[0]);
#if FPM_POOL_NUM > 1
	if (fdev->pool_pbase[1])
		fdev->pool_vbase[1] = ioremap(fdev->pool_pbase[1],
					      fdev->pool_size[1]);
	else
		fdev->pool_vbase[1] = NULL;
#endif
#endif
	if (!fdev->pool_vbase[0]) {
		pr_err("%s Unable to ioremap FPM pool 0 @ 0x%08x physical.\n",
		       __func__, fdev->pool_pbase[0]);
		status = -EFAULT;
		goto err_free_fdev;
	}

#if FPM_POOL_NUM > 1
	if (fdev->pool_pbase[1] && !fdev->pool_vbase[1]) {
		pr_err("%s Unable to ioremap FPM pool 1 @ 0x%08x physical.\n",
		       __func__, fdev->pool_pbase[1]);
		status = -EFAULT;
		goto err_free_fdev;
	}
#endif
#else
	/* why do we need this here?  It is already assigned in 
	   fpm_resource_alloc. Should delete this code */
#if 0
	fdev->pool_vbase[0] = phys_to_virt(fdev->pool_pbase[0]);
#if FPM_POOL_NUM > 1
	if (fdev->pool_pbase[1])
		fdev->pool_vbase[1] = phys_to_virt(fdev->pool_pbase[1]);
#endif
#endif

#endif

#if defined(FPM_TOKEN_CHUNKS) && !FPM_FORCE_NO_TOKEN_CHUNKS_BITS
	fdev->buf_sizes[0] = fdev->chunk_size << 3;
	fdev->buf_sizes[1] = fdev->chunk_size << 2;
	fdev->buf_sizes[2] = fdev->chunk_size << 1;
	fdev->buf_sizes[3] = fdev->chunk_size << 0;
#endif

	fdev->buf_size_to_alloc_reg_map[0]  =
		FPM_ALLOC_1_CHUNKS;	/*  512 ||  256 */
	fdev->buf_size_to_alloc_reg_map[1]  =
		FPM_ALLOC_2_CHUNKS;	/* 1024 ||  512 */
	fdev->buf_size_to_alloc_reg_map[2]  =
		FPM_ALLOC_4_CHUNKS;	/* 2048 || 1024 */
	fdev->buf_size_to_alloc_reg_map[3]  =
		FPM_ALLOC_4_CHUNKS;	/* 2048 || 1024 */
	fdev->buf_size_to_alloc_reg_map[4]  =
		FPM_ALLOC_8_CHUNKS;	/* 4096 || 2048 */
	fdev->buf_size_to_alloc_reg_map[5]  =
		FPM_ALLOC_8_CHUNKS;	/* 4096 || 2048 */
	fdev->buf_size_to_alloc_reg_map[6]  =
		FPM_ALLOC_8_CHUNKS;	/* 4096 || 2048 */
	fdev->buf_size_to_alloc_reg_map[7]  =
		FPM_ALLOC_8_CHUNKS;	/* 4096 || 2048 */

	fdev->buf_size_to_alloc_reg_map_pool[0][0] =
		FPM_ALLOC_1_CHUNKS_FROM_POOL_0;
	fdev->buf_size_to_alloc_reg_map_pool[0][1] =
		FPM_ALLOC_2_CHUNKS_FROM_POOL_0;
	fdev->buf_size_to_alloc_reg_map_pool[0][2] =
		FPM_ALLOC_4_CHUNKS_FROM_POOL_0;
	fdev->buf_size_to_alloc_reg_map_pool[0][3] =
		FPM_ALLOC_4_CHUNKS_FROM_POOL_0;
	fdev->buf_size_to_alloc_reg_map_pool[0][4] =
		FPM_ALLOC_8_CHUNKS_FROM_POOL_0;
	fdev->buf_size_to_alloc_reg_map_pool[0][5] =
		FPM_ALLOC_8_CHUNKS_FROM_POOL_0;
	fdev->buf_size_to_alloc_reg_map_pool[0][6] =
		FPM_ALLOC_8_CHUNKS_FROM_POOL_0;
	fdev->buf_size_to_alloc_reg_map_pool[0][7] =
		FPM_ALLOC_8_CHUNKS_FROM_POOL_0;

#if FPM_POOL_NUM > 1
	fdev->buf_size_to_alloc_reg_map_pool[1][0] =
		FPM_ALLOC_1_CHUNKS_FROM_POOL_1;
	fdev->buf_size_to_alloc_reg_map_pool[1][1] =
		FPM_ALLOC_2_CHUNKS_FROM_POOL_1;
	fdev->buf_size_to_alloc_reg_map_pool[1][2] =
		FPM_ALLOC_4_CHUNKS_FROM_POOL_1;
	fdev->buf_size_to_alloc_reg_map_pool[1][3] =
		FPM_ALLOC_4_CHUNKS_FROM_POOL_1;
	fdev->buf_size_to_alloc_reg_map_pool[1][4] =
		FPM_ALLOC_8_CHUNKS_FROM_POOL_1;
	fdev->buf_size_to_alloc_reg_map_pool[1][5] =
		FPM_ALLOC_8_CHUNKS_FROM_POOL_1;
	fdev->buf_size_to_alloc_reg_map_pool[1][6] =
		FPM_ALLOC_8_CHUNKS_FROM_POOL_1;
	fdev->buf_size_to_alloc_reg_map_pool[1][7] =
		FPM_ALLOC_8_CHUNKS_FROM_POOL_1;
#endif

	fdev->pool_alloc_weight[0] = FPM_GET_ALLOC_WEIGHT(0);
	fdev->pool_free_weight[0]  = FPM_GET_FREE_WEIGHT(0);
#if FPM_POOL_NUM > 1
	fdev->pool_alloc_weight[1] = FPM_GET_ALLOC_WEIGHT(1);
	fdev->pool_free_weight[1]  = FPM_GET_FREE_WEIGHT(1);
#endif

	/* TODO! double check whether 4908 need this workaround or not */
#if 0
	/*
	 * If we are the one initializing the FPM block then we need to
	 * allocate and never free 4 of the largest size tokens from each
	 * pool in order to work around a bug in the FPM hardware of the 3383
	 * thru 3390A chips.
	 */
	if (fdev->init) {
		for (i = 0; i < 4; i++) {
			fdev->prealloc_tok[0][i] =
				FPM_ALLOC_8_CHUNKS_FROM_POOL(0);
			pr_debug("Pre-allocated token 0x%08x\n",
				 fdev->prealloc_tok[0][i]);
#if FPM_POOL_NUM > 1
			if (fdev->pool_pbase[1]) {
				fdev->prealloc_tok[1][i] =
					FPM_ALLOC_8_CHUNKS_FROM_POOL(1);
				pr_debug("Pre-allocated token 0x%08x\n",
					 fdev->prealloc_tok[1][i]);
			}
#endif
		}
	}
#endif

#ifdef CONFIG_BCM_FPM_TOKEN_HIST
	num_tokens = FPM_TOKEN_INDEX_MASK >> FPM_TOKEN_INDEX_SHIFT;
#if FPM_POOL_NUM > 1
	if (fdev->pool_pbase[1])
		num_tokens <<= 1;
#endif
	fdev->tok_ref_count = kzalloc(num_tokens, GFP_KERNEL);
	if (!fdev->tok_ref_count) {
		pr_err("%s Unable to alloc token history array.\n",
		       __func__);
		status = -ENOMEM;
		goto err_free_fdev;
	}
	pr_info("Allocating %d bytes (%d ops) for token history buffer.\n",
		FPM_HISTORY_MEM_SIZE, FPM_NUM_HISTORY_ENTRIES);
	fdev->tok_hist_start = kzalloc(FPM_NUM_HISTORY_ENTRIES *
		sizeof(struct fpm_tok_op), GFP_KERNEL);
	if (!fdev->tok_hist_start) {
		pr_err("%s Unable to alloc token history buffer.\n",
		       __func__);
		status = -ENOMEM;
		goto err_free_hist;
	}
	fdev->tok_hist_end = fdev->tok_hist_start +
		FPM_NUM_HISTORY_ENTRIES;
	fdev->tok_hist_head = fdev->tok_hist_start;
	fdev->tok_hist_tail = fdev->tok_hist_start;
	spin_lock_init(&fdev->tok_hist_lock);
	fdev->track_on_err = false;
#endif

#if FPM_POOL_NUM > 1
	pr_info("%s: FPM with %d pool(s)\n", MODULE_NAME,
		fdev->pool_pbase[1] ? 2 : 1);
#endif

	goto done;

#ifdef CONFIG_BCM_FPM_TOKEN_HIST
err_free_hist:
	if (fdev->tok_ref_count)
		kfree(fdev->tok_ref_count);
#endif

err_free_fdev:
	fpm = NULL;
	kfree(fdev);

done:
	pr_debug("<--\n");
	return status;
}

static int fpm_remove(struct platform_device *pdev)
{
	int status = 0;
	struct fpmdev *fdev = pdev->dev.platform_data;

	pr_debug("-->\n");

	if (!fdev) {
		pr_err("%s Release called with uninitialized ",
		       __func__);
		pr_err("platform_data.\n");
		status = -EINVAL;
		goto done;
	}

	if (timer_pending(&fdev->isr_timer))
		del_timer_sync(&fdev->isr_timer);

#ifdef CREATE_VIRTUAL_ADDRESS_MAPPING
	iounmap((void *)fdev->pool_vbase[0]);
#if FPM_POOL_NUM > 1
	if (fdev->pool_vbase[1])
		iounmap((void *)fdev->pool_vbase[1]);
#endif
	iounmap((void *)fdev->reg_vbase);
#endif

#ifdef CONFIG_BCM_FPM_TOKEN_HIST
	if (fdev->tok_ref_count)
		kfree(fdev->tok_ref_count);
	if (fdev->tok_hist_start)
		kfree(fdev->tok_hist_start);
#endif

	fpm = NULL;
	kfree(fdev);

done:
	pr_debug("<--\n");
	return status;
}

#ifdef CONFIG_BCM_FPM_DT
static const struct of_device_id fpm_of_match[] = {
	{.compatible = "brcm,fpm"},
	{}
};
MODULE_DEVICE_TABLE(of, fpm_of_match);
#else
static struct resource fpm_resources[] = {
	[0] = {
		.start	= FPM_PHYS_BASE,
		.end	= FPM_PHYS_BASE + FPM_SIZE - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= INTERRUPT_FPM_INTR,
		.end	= INTERRUPT_FPM_INTR,
		.flags	= IORESOURCE_IRQ,
	}
};

static struct platform_device fpm_plat_dev = {
	.name		= MODULE_NAME,
	.num_resources	= ARRAY_SIZE(fpm_resources),
	.resource	= fpm_resources,
};
#endif

static struct platform_driver fpm_driver = {
	.probe  = fpm_probe,
	.remove = fpm_remove,
	.driver = {
		.name = MODULE_NAME,
		.owner = THIS_MODULE,
#ifdef CONFIG_BCM_FPM_DT
		.of_match_table = fpm_of_match
#endif
	}
};

static int __init fpm_init(void)
{
	int status = 0;

	pr_debug("%s driver v%s", MODULE_NAME, MODULE_VER);

	spin_lock_init(&fpm_reg_lock);
	status = platform_driver_register(&fpm_driver);
	if (status)
		goto done;

	pmc_fpm_power_up();

	fpm_proc_init();

#ifndef CONFIG_BCM_FPM_DT
	/* since we are not using DeviceTree for FPM, we will just
	 * register platform device here. */
	fpm_plat_dev.resource[1].start = INTERRUPT_ID_FPM;
	fpm_plat_dev.resource[1].end = INTERRUPT_ID_FPM;
	status = platform_device_register(&fpm_plat_dev);
	if (status) {
		platform_device_unregister(&fpm_plat_dev);
		return status;
	}
#endif

done:
	return status;
}

static void __exit fpm_exit(void)
{
	platform_driver_unregister(&fpm_driver);
	fpm_proc_exit();
	pmc_fpm_power_down();
}

module_init(fpm_init);
module_exit(fpm_exit);
MODULE_LICENSE("GPL v2");
