// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018-2022 Marvell International Ltd.
 *
 * PKO resources.
 */

#include <errno.h>
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
#include <mach/cvmx-range.h>
#include <mach/cvmx-global-resources.h>

#include <mach/cvmx-agl-defs.h>
#include <mach/cvmx-bgxx-defs.h>
#include <mach/cvmx-ciu-defs.h>
#include <mach/cvmx-gmxx-defs.h>
#include <mach/cvmx-gserx-defs.h>
#include <mach/cvmx-ilk-defs.h>
#include <mach/cvmx-ipd-defs.h>
#include <mach/cvmx-pcsx-defs.h>
#include <mach/cvmx-pcsxx-defs.h>
#include <mach/cvmx-pki-defs.h>
#include <mach/cvmx-pko-defs.h>
#include <mach/cvmx-xcv-defs.h>

#include <mach/cvmx-hwpko.h>
#include <mach/cvmx-ilk.h>
#include <mach/cvmx-pki.h>
#include <mach/cvmx-pko3.h>
#include <mach/cvmx-pko3-queue.h>
#include <mach/cvmx-pko3-resources.h>

#include <mach/cvmx-helper.h>
#include <mach/cvmx-helper-board.h>
#include <mach/cvmx-helper-cfg.h>

#include <mach/cvmx-helper-bgx.h>
#include <mach/cvmx-helper-cfg.h>
#include <mach/cvmx-helper-util.h>
#include <mach/cvmx-helper-pki.h>

#define CVMX_GR_TAG_PKO_PORT_QUEUES(x)                                         \
	cvmx_get_gr_tag('c', 'v', 'm', '_', 'p', 'k', 'o', 'p', 'o', 'q', '_', \
			((x) + '0'), '.', '.', '.', '.')
#define CVMX_GR_TAG_PKO_L2_QUEUES(x)                                           \
	cvmx_get_gr_tag('c', 'v', 'm', '_', 'p', 'k', 'o', 'l', '2', 'q', '_', \
			((x) + '0'), '.', '.', '.', '.')
#define CVMX_GR_TAG_PKO_L3_QUEUES(x)                                           \
	cvmx_get_gr_tag('c', 'v', 'm', '_', 'p', 'k', 'o', 'l', '3', 'q', '_', \
			((x) + '0'), '.', '.', '.', '.')
#define CVMX_GR_TAG_PKO_L4_QUEUES(x)                                           \
	cvmx_get_gr_tag('c', 'v', 'm', '_', 'p', 'k', 'o', 'l', '4', 'q', '_', \
			((x) + '0'), '.', '.', '.', '.')
#define CVMX_GR_TAG_PKO_L5_QUEUES(x)                                           \
	cvmx_get_gr_tag('c', 'v', 'm', '_', 'p', 'k', 'o', 'l', '5', 'q', '_', \
			((x) + '0'), '.', '.', '.', '.')
#define CVMX_GR_TAG_PKO_DESCR_QUEUES(x)                                        \
	cvmx_get_gr_tag('c', 'v', 'm', '_', 'p', 'k', 'o', 'd', 'e', 'q', '_', \
			((x) + '0'), '.', '.', '.', '.')
#define CVMX_GR_TAG_PKO_PORT_INDEX(x)                                          \
	cvmx_get_gr_tag('c', 'v', 'm', '_', 'p', 'k', 'o', 'p', 'i', 'd', '_', \
			((x) + '0'), '.', '.', '.', '.')

/*
 * @INRWENAL
 * Per-DQ parameters, current and maximum queue depth counters
 */
cvmx_pko3_dq_params_t *__cvmx_pko3_dq_params[CVMX_MAX_NODES];

static const short cvmx_pko_num_queues_78XX[256] = {
	[CVMX_PKO_PORT_QUEUES] = 32, [CVMX_PKO_L2_QUEUES] = 512,
	[CVMX_PKO_L3_QUEUES] = 512,  [CVMX_PKO_L4_QUEUES] = 1024,
	[CVMX_PKO_L5_QUEUES] = 1024, [CVMX_PKO_DESCR_QUEUES] = 1024
};

static const short cvmx_pko_num_queues_73XX[256] = {
	[CVMX_PKO_PORT_QUEUES] = 16, [CVMX_PKO_L2_QUEUES] = 256,
	[CVMX_PKO_L3_QUEUES] = 256,  [CVMX_PKO_L4_QUEUES] = 0,
	[CVMX_PKO_L5_QUEUES] = 0,    [CVMX_PKO_DESCR_QUEUES] = 256
};

int cvmx_pko3_num_level_queues(enum cvmx_pko3_level_e level)
{
	unsigned int nq = 0, ne = 0;

	if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		ne = NUM_ELEMENTS(cvmx_pko_num_queues_78XX);
		nq = cvmx_pko_num_queues_78XX[level];
	}
	if (OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)) {
		ne = NUM_ELEMENTS(cvmx_pko_num_queues_73XX);
		nq = cvmx_pko_num_queues_73XX[level];
	}

	if (nq == 0 || level >= ne) {
		printf("ERROR: %s: queue level %#x invalid\n", __func__, level);
		return -1;
	}

	return nq;
}

static inline struct global_resource_tag
__cvmx_pko_get_queues_resource_tag(int node, enum cvmx_pko3_level_e queue_level)
{
	if (cvmx_pko3_num_level_queues(queue_level) == 0) {
		printf("ERROR: %s: queue level %#x invalid\n", __func__,
		       queue_level);
		return CVMX_GR_TAG_INVALID;
	}

	switch (queue_level) {
	case CVMX_PKO_PORT_QUEUES:
		return CVMX_GR_TAG_PKO_PORT_QUEUES(node);
	case CVMX_PKO_L2_QUEUES:
		return CVMX_GR_TAG_PKO_L2_QUEUES(node);
	case CVMX_PKO_L3_QUEUES:
		return CVMX_GR_TAG_PKO_L3_QUEUES(node);
	case CVMX_PKO_L4_QUEUES:
		return CVMX_GR_TAG_PKO_L4_QUEUES(node);
	case CVMX_PKO_L5_QUEUES:
		return CVMX_GR_TAG_PKO_L5_QUEUES(node);
	case CVMX_PKO_DESCR_QUEUES:
		return CVMX_GR_TAG_PKO_DESCR_QUEUES(node);
	default:
		printf("ERROR: %s: queue level %#x invalid\n", __func__,
		       queue_level);
		return CVMX_GR_TAG_INVALID;
	}
}

/**
 * Allocate or reserve a pko resource - called by wrapper functions
 * @param tag processed global resource tag
 * @param base_queue if specified the queue to reserve
 * @param owner to be specified for resource
 * @param num_queues to allocate
 * @param max_num_queues for global resource
 */
int cvmx_pko_alloc_global_resource(struct global_resource_tag tag,
				   int base_queue, int owner, int num_queues,
				   int max_num_queues)
{
	int res;

	if (cvmx_create_global_resource_range(tag, max_num_queues)) {
		debug("ERROR: Failed to create PKO3 resource: %lx-%lx\n",
		      (unsigned long)tag.hi, (unsigned long)tag.lo);
		return -1;
	}
	if (base_queue >= 0) {
		res = cvmx_reserve_global_resource_range(tag, owner, base_queue,
							 num_queues);
	} else {
		res = cvmx_allocate_global_resource_range(tag, owner,
							  num_queues, 1);
	}
	if (res < 0) {
		debug("ERROR: Failed to %s PKO3 tag %lx:%lx, %i %i %i %i.\n",
		      ((base_queue < 0) ? "allocate" : "reserve"),
		      (unsigned long)tag.hi, (unsigned long)tag.lo, base_queue,
		      owner, num_queues, max_num_queues);
		return -1;
	}

	return res;
}

/**
 * Allocate or reserve PKO queues - wrapper for cvmx_pko_alloc_global_resource
 *
 * @param node on which to allocate/reserve PKO queues
 * @param level of PKO queue
 * @param owner of reserved/allocated resources
 * @param base_queue to start reservation/allocatation
 * @param num_queues number of queues to be allocated
 * @return 0 on success, -1 on failure
 */
int cvmx_pko_alloc_queues(int node, int level, int owner, int base_queue,
			  int num_queues)
{
	struct global_resource_tag tag =
		__cvmx_pko_get_queues_resource_tag(node, level);
	int max_num_queues = cvmx_pko3_num_level_queues(level);

	return cvmx_pko_alloc_global_resource(tag, base_queue, owner,
					      num_queues, max_num_queues);
}

/**
 * @INTERNAL
 *
 * Initialize the pointer to the descriptor queue parameter table.
 * The table is one named block per node, and may be shared between
 * applications.
 */
int __cvmx_pko3_dq_param_setup(unsigned int node)
{
	return 0;
}
