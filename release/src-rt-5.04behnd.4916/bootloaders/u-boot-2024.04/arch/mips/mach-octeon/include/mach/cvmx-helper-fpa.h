/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Helper functions for FPA setup.
 */

#ifndef __CVMX_HELPER_H_FPA__
#define __CVMX_HELPER_H_FPA__

/**
 * Allocate memory and initialize the FPA pools using memory
 * from cvmx-bootmem. Sizes of each element in the pools is
 * controlled by the cvmx-config.h header file. Specifying
 * zero for any parameter will cause that FPA pool to not be
 * setup. This is useful if you aren't using some of the
 * hardware and want to save memory.
 *
 * @param packet_buffers
 *               Number of packet buffers to allocate
 * @param work_queue_entries
 *               Number of work queue entries
 * @param pko_buffers
 *               PKO Command buffers. You should at minimum have two per
 *               each PKO queue.
 * @param tim_buffers
 *               TIM ring buffer command queues. At least two per timer bucket
 *               is recommended.
 * @param dfa_buffers
 *               DFA command buffer. A relatively small (32 for example)
 *               number should work.
 * Return: Zero on success, non-zero if out of memory
 */
int cvmx_helper_initialize_fpa(int packet_buffers, int work_queue_entries, int pko_buffers,
			       int tim_buffers, int dfa_buffers);

int __cvmx_helper_initialize_fpa_pool(int pool, u64 buffer_size, u64 buffers, const char *name);

int cvmx_helper_shutdown_fpa_pools(int node);

void cvmx_helper_fpa_dump(int node);

#endif /* __CVMX_HELPER_H__ */
