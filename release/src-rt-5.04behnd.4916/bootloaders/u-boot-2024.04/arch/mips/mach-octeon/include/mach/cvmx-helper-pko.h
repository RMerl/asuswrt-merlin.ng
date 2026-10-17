/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * PKO helper, configuration API
 */

#ifndef __CVMX_HELPER_PKO_H__
#define __CVMX_HELPER_PKO_H__

/* CSR typedefs have been moved to cvmx-pko-defs.h */

/**
 * cvmx_override_pko_queue_priority(int ipd_port, u64
 * priorities[16]) is a function pointer. It is meant to allow
 * customization of the PKO queue priorities based on the port
 * number. Users should set this pointer to a function before
 * calling any cvmx-helper operations.
 */
extern void (*cvmx_override_pko_queue_priority)(int ipd_port, u8 *priorities);

/**
 * Gets the fpa pool number of pko pool
 */
s64 cvmx_fpa_get_pko_pool(void);

/**
 * Gets the buffer size of pko pool
 */
u64 cvmx_fpa_get_pko_pool_block_size(void);

/**
 * Gets the buffer size  of pko pool
 */
u64 cvmx_fpa_get_pko_pool_buffer_count(void);

int cvmx_helper_pko_init(void);

/*
 * This function is a no-op
 * included here for backwards compatibility only.
 */
static inline int cvmx_pko_initialize_local(void)
{
	return 0;
}

int __cvmx_helper_pko_drain(void);
int __cvmx_helper_interface_setup_pko(int interface);

#endif /* __CVMX_HELPER_PKO_H__ */
