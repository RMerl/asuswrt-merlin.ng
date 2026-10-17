/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2018-2022 Marvell International Ltd.
 */

#ifndef __CVMX_RANGE_H__
#define __CVMX_RANGE_H__

int cvmx_range_init(u64 range_addr, int size);
int cvmx_range_alloc(u64 range_addr, uint64_t owner, uint64_t cnt, int align);
int cvmx_range_alloc_ordered(u64 range_addr, uint64_t owner, u64 cnt, int align,
			     int reverse);
int cvmx_range_alloc_non_contiguos(u64 range_addr, uint64_t owner, u64 cnt,
				   int elements[]);
int cvmx_range_reserve(u64 range_addr, uint64_t owner, u64 base, uint64_t cnt);
int cvmx_range_free_with_base(u64 range_addr, int base, int cnt);
int cvmx_range_free_with_owner(u64 range_addr, uint64_t owner);
u64 cvmx_range_get_owner(u64 range_addr, uint64_t base);
void cvmx_range_show(uint64_t range_addr);
int cvmx_range_memory_size(int nelements);
int cvmx_range_free_mutiple(u64 range_addr, int bases[], int count);

#endif // __CVMX_RANGE_H__
