/*
 * Copyright (c) 2013-2015, Mellanox Technologies. All rights reserved.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the
 * OpenIB.org BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <linux/module.h>
#include <rdma/ib_umem.h>
#include <rdma/ib_umem_odp.h>
#include "mlx5_ib.h"

/* @umem: umem object to scan
 * @addr: ib virtual address requested by the user
 * @count: number of PAGE_SIZE pages covered by umem
 * @shift: page shift for the compound pages found in the region
 * @ncont: number of compund pages
 * @order: log2 of the number of compound pages
 */
void mlx5_ib_cont_pages(struct ib_umem *umem, u64 addr, int *count, int *shift,
			int *ncont, int *order)
{
	unsigned long tmp;
	unsigned long m;
	int i, k;
	u64 base = 0;
	int p = 0;
	int skip;
	int mask;
	u64 len;
	u64 pfn;
	struct scatterlist *sg;
	int entry;
	unsigned long page_shift = ilog2(umem->page_size);

	/* With ODP we must always match OS page size. */
	if (umem->odp_data) {
		*count = ib_umem_page_count(umem);
		*shift = PAGE_SHIFT;
		*ncont = *count;
		if (order)
			*order = ilog2(roundup_pow_of_two(*count));

		return;
	}

	addr = addr >> page_shift;
	tmp = (unsigned long)addr;
	m = find_first_bit(&tmp, sizeof(tmp));
	skip = 1 << m;
	mask = skip - 1;
	i = 0;
	for_each_sg(umem->sg_head.sgl, sg, umem->nmap, entry) {
		len = sg_dma_len(sg) >> page_shift;
		pfn = sg_dma_address(sg) >> page_shift;
		for (k = 0; k < len; k++) {
			if (!(i & mask)) {
				tmp = (unsigned long)pfn;
				m = min_t(unsigned long, m, find_first_bit(&tmp, sizeof(tmp)));
				skip = 1 << m;
				mask = skip - 1;
				base = pfn;
				p = 0;
			} else {
				if (base + p != pfn) {
					tmp = (unsigned long)p;
					m = find_first_bit(&tmp, sizeof(tmp));
					skip = 1 << m;
					mask = skip - 1;
					base = pfn;
					p = 0;
				}
			}
			p++;
			i++;
		}
	}

	if (i) {
		m = min_t(unsigned long, ilog2(roundup_pow_of_two(i)), m);

		if (order)
			*order = ilog2(roundup_pow_of_two(i) >> m);

		*ncont = DIV_ROUND_UP(i, (1 << m));
	} else {
		m  = 0;

		if (order)
			*order = 0;

		*ncont = 0;
	}
	*shift = page_shift + m;
	*count = i;
}

#ifdef CONFIG_INFINIBAND_ON_DEMAND_PAGING
static u64 umem_dma_to_mtt(dma_addr_t umem_dma)
{
	u64 mtt_entry = umem_dma & ODP_DMA_ADDR_MASK;

	if (umem_dma & ODP_READ_ALLOWED_BIT)
		mtt_entry |= MLX5_IB_MTT_READ;
	if (umem_dma & ODP_WRITE_ALLOWED_BIT)
		mtt_entry |= MLX5_IB_MTT_WRITE;

	return mtt_entry;
}
#endif

/*
 * Populate the given array with bus addresses from the umem.
 *
 * dev - mlx5_ib device
 * umem - umem to use to fill the pages
 * page_shift - determines the page size used in the resulting array
 * offset - offset into the umem to start from,
 *          only implemented for ODP umems
 * num_pages - total number of pages to fill
 * pas - bus addresses array to fill
 * access_flags - access flags to set on all present pages.
		  use enum mlx5_ib_mtt_access_flags for this.
 */
void __mlx5_ib_populate_pas(struct mlx5_ib_dev *dev, struct ib_umem *umem,
			    int page_shift, size_t offset, size_t num_pages,
			    __be64 *pas, int access_flags)
{
	unsigned long umem_page_shift = ilog2(umem->page_size);
	int shift = page_shift - umem_page_shift;
	int mask = (1 << shift) - 1;
	int i, k;
	u64 cur = 0;
	u64 base;
	int len;
	struct scatterlist *sg;
	int entry;
#ifdef CONFIG_INFINIBAND_ON_DEMAND_PAGING
	const bool odp = umem->odp_data != NULL;

	if (odp) {
		WARN_ON(shift != 0);
		WARN_ON(access_flags != (MLX5_IB_MTT_READ | MLX5_IB_MTT_WRITE));

		for (i = 0; i < num_pages; ++i) {
			dma_addr_t pa = umem->odp_data->dma_list[offset + i];

			pas[i] = cpu_to_be64(umem_dma_to_mtt(pa));
		}
		return;
	}
#endif

	i = 0;
	for_each_sg(umem->sg_head.sgl, sg, umem->nmap, entry) {
		len = sg_dma_len(sg) >> umem_page_shift;
		base = sg_dma_address(sg);
		for (k = 0; k < len; k++) {
			if (!(i & mask)) {
				cur = base + (k << umem_page_shift);
				cur |= access_flags;

				pas[i >> shift] = cpu_to_be64(cur);
				mlx5_ib_dbg(dev, "pas[%d] 0x%llx\n",
					    i >> shift, be64_to_cpu(pas[i >> shift]));
			}  else
				mlx5_ib_dbg(dev, "=====> 0x%llx\n",
					    base + (k << umem_page_shift));
			i++;
		}
	}
}

void mlx5_ib_populate_pas(struct mlx5_ib_dev *dev, struct ib_umem *umem,
			  int page_shift, __be64 *pas, int access_flags)
{
	return __mlx5_ib_populate_pas(dev, umem, page_shift, 0,
				      ib_umem_num_pages(umem), pas,
				      access_flags);
}
int mlx5_ib_get_buf_offset(u64 addr, int page_shift, u32 *offset)
{
	u64 page_size;
	u64 page_mask;
	u64 off_size;
	u64 off_mask;
	u64 buf_off;

	page_size = (u64)1 << page_shift;
	page_mask = page_size - 1;
	buf_off = addr & page_mask;
	off_size = page_size >> 6;
	off_mask = off_size - 1;

	if (buf_off & off_mask)
		return -EINVAL;

	*offset = buf_off >> ilog2(off_size);
	return 0;
}
