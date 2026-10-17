/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 */

#ifndef __CVMX_HELPER_PKO3_H__
#define __CVMX_HELPER_PKO3_H__

/*
 * Initialize PKO3 unit on the current node.
 *
 * Covers the common hardware, memory and global configuration.
 * Per-interface initialization is performed separately.
 *
 * Return: 0 on success.
 *
 */
int cvmx_helper_pko3_init_global(unsigned int node);
int __cvmx_helper_pko3_init_global(unsigned int node, u16 gaura);

/**
 * Initialize a simple interface with a a given number of
 * fair or prioritized queues.
 * This function will assign one channel per sub-interface.
 */
int __cvmx_pko3_config_gen_interface(int xiface, u8 subif, u8 num_queues, bool prioritized);

/*
 * Configure and initialize PKO3 for an interface
 *
 * @param interface is the interface number to configure
 * Return: 0 on success.
 *
 */
int cvmx_helper_pko3_init_interface(int xiface);
int __cvmx_pko3_helper_dqs_activate(int xiface, int index, bool min_pad);

/**
 * Uninitialize PKO3 interface
 *
 * Release all resources held by PKO for an interface.
 * The shutdown code is the same for all supported interfaces.
 */
int cvmx_helper_pko3_shut_interface(int xiface);

/**
 * Shutdown PKO3
 *
 * Should be called after all interfaces have been shut down on the PKO3.
 *
 * Disables the PKO, frees all its buffers.
 */
int cvmx_helper_pko3_shutdown(unsigned int node);

/**
 * Show integrated PKO configuration.
 *
 * @param node	   node number
 */
int cvmx_helper_pko3_config_dump(unsigned int node);

/**
 * Show integrated PKO statistics.
 *
 * @param node	   node number
 */
int cvmx_helper_pko3_stats_dump(unsigned int node);

/**
 * Clear PKO statistics.
 *
 * @param node	   node number
 */
void cvmx_helper_pko3_stats_clear(unsigned int node);

#endif /* __CVMX_HELPER_PKO3_H__ */
