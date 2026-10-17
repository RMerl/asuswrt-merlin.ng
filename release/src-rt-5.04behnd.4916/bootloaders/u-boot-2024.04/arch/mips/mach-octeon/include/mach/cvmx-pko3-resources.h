// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018-2022 Marvell International Ltd.
 */

#ifndef __CVMX_PKO3_RESOURCES_H__
#define __CVMX_PKO3_RESOURCES_H__

/*
 * Allocate or reserve contiguous list of PKO queues.
 *
 * @param node is the node number for PKO queues.
 * @param level is the PKO queue level.
 * @param owner is the owner of PKO queue resources.
 * @param base_queue is the PKO queue base number(specify -1 to allocate).
 * @param num_queues is the number of PKO queues that have to be reserved or allocated.
 * @return returns queue_base if successful or -1 on failure.
 */
int cvmx_pko_alloc_queues(int node, int level, int owner, int base_queue,
			  int num_queues);

/**
 * Free an allocated/reserved PKO queues for a certain level and owner
 *
 * @param node on which to allocate/reserve PKO queues
 * @param level of PKO queue
 * @param owner of reserved/allocated resources
 * @return 0 on success, -1 on failure
 */
int cvmx_pko_free_queues(int node, int level, int owner);

int __cvmx_pko3_dq_param_setup(unsigned node);

int cvmx_pko3_num_level_queues(enum cvmx_pko3_level_e level);

#endif /* __CVMX_PKO3_RESOURCES_H__ */
