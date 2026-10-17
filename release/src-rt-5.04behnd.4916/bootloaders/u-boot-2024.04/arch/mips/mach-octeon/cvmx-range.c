// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018-2022 Marvell International Ltd.
 */

#include <log.h>
#include <time.h>
#include <linux/delay.h>

#include <mach/cvmx-regs.h>
#include <mach/cvmx-csr.h>
#include <mach/cvmx-bootmem.h>
#include <mach/octeon-model.h>
#include <mach/cvmx-fuse.h>
#include <mach/octeon-feature.h>
#include <mach/cvmx-qlm.h>
#include <mach/octeon_qlm.h>
#include <mach/cvmx-pcie.h>
#include <mach/cvmx-coremask.h>

#include <mach/cvmx-global-resources.h>

#include <mach/cvmx-pki.h>
#include <mach/cvmx-helper.h>
#include <mach/cvmx-helper-board.h>
#include <mach/cvmx-helper-cfg.h>

#include <mach/cvmx-range.h>

#define CVMX_RANGE_AVAILABLE ((u64)-88)
#define addr_of_element(base, index)					\
	(1ull << 63 | ((base) + sizeof(u64) + (index) * sizeof(u64)))
#define addr_of_size(base) (1ull << 63 | (base))

static const int debug;

int cvmx_range_memory_size(int nelements)
{
	return sizeof(u64) * (nelements + 1);
}

int cvmx_range_init(u64 range_addr, int size)
{
	u64 lsize = size;
	u64 i;

	cvmx_write64_uint64(addr_of_size(range_addr), lsize);
	for (i = 0; i < lsize; i++) {
		cvmx_write64_uint64(addr_of_element(range_addr, i),
				    CVMX_RANGE_AVAILABLE);
	}
	return 0;
}

static int64_t cvmx_range_find_next_available(u64 range_addr, u64 index,
					      int align)
{
	u64 size = cvmx_read64_uint64(addr_of_size(range_addr));
	u64 i;

	while ((index % align) != 0)
		index++;

	for (i = index; i < size; i += align) {
		u64 r_owner = cvmx_read64_uint64(addr_of_element(range_addr, i));

		if (debug)
			debug("%s: index=%d owner=%llx\n", __func__, (int)i,
			      (unsigned long long)r_owner);
		if (r_owner == CVMX_RANGE_AVAILABLE)
			return i;
	}
	return -1;
}

static int64_t cvmx_range_find_last_available(u64 range_addr, u64 index,
					      u64 align)
{
	u64 size = cvmx_read64_uint64(addr_of_size(range_addr));
	u64 i;

	if (index == 0)
		index = size - 1;

	while ((index % align) != 0)
		index++;

	for (i = index; i > align; i -= align) {
		u64 r_owner = cvmx_read64_uint64(addr_of_element(range_addr, i));

		if (debug)
			debug("%s: index=%d owner=%llx\n", __func__, (int)i,
			      (unsigned long long)r_owner);
		if (r_owner == CVMX_RANGE_AVAILABLE)
			return i;
	}
	return -1;
}

int cvmx_range_alloc_ordered(u64 range_addr, u64 owner, u64 cnt,
			     int align, int reverse)
{
	u64 i = 0, size;
	s64 first_available;

	if (debug)
		debug("%s: range_addr=%llx  owner=%llx cnt=%d\n", __func__,
		      (unsigned long long)range_addr,
		      (unsigned long long)owner, (int)cnt);

	size = cvmx_read64_uint64(addr_of_size(range_addr));
	while (i < size) {
		u64 available_cnt = 0;

		if (reverse)
			first_available = cvmx_range_find_last_available(range_addr, i, align);
		else
			first_available = cvmx_range_find_next_available(range_addr, i, align);
		if (first_available == -1)
			return -1;
		i = first_available;

		if (debug)
			debug("%s: first_available=%d\n", __func__, (int)first_available);
		while ((available_cnt != cnt) && (i < size)) {
			u64 r_owner = cvmx_read64_uint64(addr_of_element(range_addr, i));

			if (r_owner == CVMX_RANGE_AVAILABLE)
				available_cnt++;
			i++;
		}
		if (available_cnt == cnt) {
			u64 j;

			if (debug)
				debug("%s: first_available=%d available=%d\n",
				      __func__,
				      (int)first_available, (int)available_cnt);

			for (j = first_available; j < first_available + cnt;
			     j++) {
				u64 a = addr_of_element(range_addr, j);

				cvmx_write64_uint64(a, owner);
			}
			return first_available;
		}
	}

	if (debug) {
		debug("ERROR: %s: failed to allocate range cnt=%d\n",
		      __func__, (int)cnt);
		cvmx_range_show(range_addr);
	}

	return -1;
}

int cvmx_range_alloc(u64 range_addr, u64 owner, u64 cnt, int align)
{
	return cvmx_range_alloc_ordered(range_addr, owner, cnt, align, 0);
}

int cvmx_range_reserve(u64 range_addr, u64 owner, u64 base,
		       u64 cnt)
{
	u64 i, size, r_owner;
	u64 up = base + cnt;

	size = cvmx_read64_uint64(addr_of_size(range_addr));
	if (up > size) {
		debug("ERROR: %s: invalid base or cnt. range_addr=0x%llx, owner=0x%llx, size=%d base+cnt=%d\n",
		      __func__, (unsigned long long)range_addr,
		      (unsigned long long)owner,
		      (int)size, (int)up);
		return -1;
	}
	for (i = base; i < up; i++) {
		r_owner = cvmx_read64_uint64(addr_of_element(range_addr, i));
		if (debug)
			debug("%s: %d: %llx\n",
			      __func__, (int)i, (unsigned long long)r_owner);
		if (r_owner != CVMX_RANGE_AVAILABLE) {
			if (debug) {
				debug("%s: resource already reserved base+cnt=%d %llu %llu %llx %llx %llx\n",
				      __func__, (int)i, (unsigned long long)cnt,
				      (unsigned long long)base,
				      (unsigned long long)r_owner,
				      (unsigned long long)range_addr,
				      (unsigned long long)owner);
			}
			return -1;
		}
	}
	for (i = base; i < up; i++)
		cvmx_write64_uint64(addr_of_element(range_addr, i), owner);
	return base;
}

int __cvmx_range_is_allocated(u64 range_addr, int bases[], int count)
{
	u64 i, cnt, size;
	u64 r_owner;

	cnt = count;
	size = cvmx_read64_uint64(addr_of_size(range_addr));
	for (i = 0; i < cnt; i++) {
		u64 base = bases[i];

		if (base >= size) {
			debug("ERROR: %s: invalid base or cnt size=%d base=%d\n",
			      __func__, (int)size, (int)base);
			return 0;
		}
		r_owner = cvmx_read64_uint64(addr_of_element(range_addr, base));
		if (r_owner == CVMX_RANGE_AVAILABLE) {
			if (debug) {
				debug("%s: i=%d:base=%d is available\n",
				      __func__, (int)i, (int)base);
			}
			return 0;
		}
	}
	return 1;
}

int cvmx_range_free_mutiple(u64 range_addr, int bases[], int count)
{
	u64 i, cnt;

	cnt = count;
	if (__cvmx_range_is_allocated(range_addr, bases, count) != 1)
		return -1;
	for (i = 0; i < cnt; i++) {
		u64 base = bases[i];

		cvmx_write64_uint64(addr_of_element(range_addr, base),
				    CVMX_RANGE_AVAILABLE);
	}
	return 0;
}

int cvmx_range_free_with_base(u64 range_addr, int base, int cnt)
{
	u64 i, size;
	u64 up = base + cnt;

	size = cvmx_read64_uint64(addr_of_size(range_addr));
	if (up > size) {
		debug("ERROR: %s: invalid base or cnt size=%d base+cnt=%d\n",
		      __func__, (int)size, (int)up);
		return -1;
	}
	for (i = base; i < up; i++) {
		cvmx_write64_uint64(addr_of_element(range_addr, i),
				    CVMX_RANGE_AVAILABLE);
	}
	return 0;
}
