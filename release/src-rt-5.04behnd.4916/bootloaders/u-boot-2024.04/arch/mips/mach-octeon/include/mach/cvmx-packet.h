/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Packet buffer defines.
 */

#ifndef __CVMX_PACKET_H__
#define __CVMX_PACKET_H__

union cvmx_buf_ptr_pki {
	u64 u64;
	struct {
		u64 size : 16;
		u64 packet_outside_wqe : 1;
		u64 rsvd0 : 5;
		u64 addr : 42;
	};
};

typedef union cvmx_buf_ptr_pki cvmx_buf_ptr_pki_t;

/**
 * This structure defines a buffer pointer on Octeon
 */
union cvmx_buf_ptr {
	void *ptr;
	u64 u64;
	struct {
		u64 i : 1;
		u64 back : 4;
		u64 pool : 3;
		u64 size : 16;
		u64 addr : 40;
	} s;
};

typedef union cvmx_buf_ptr cvmx_buf_ptr_t;

#endif /*  __CVMX_PACKET_H__ */
