// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018-2022 Marvell International Ltd.
 *
 * PKI Support.
 */

#include <time.h>
#include <log.h>
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

#include <mach/cvmx-pki-defs.h>
#include <mach/cvmx-pko-defs.h>

#include <mach/cvmx-pki.h>
#include <mach/cvmx-helper.h>
#include <mach/cvmx-helper-board.h>
#include <mach/cvmx-helper-cfg.h>

static s32 cvmx_pki_style_refcnt[CVMX_MAX_NODES][CVMX_PKI_NUM_INTERNAL_STYLE];

/**
 * This function allocates/reserves a style from pool of global styles per node.
 * @param node	node to allocate style from.
 * @param style	style to allocate, if -1 it will be allocated
 *		first available style from style resource. If index is positive
 *		number and in range, it will try to allocate specified style.
 * @return	style number on success,
 *		-1 on alloc failure.
 *		-2 on resource already reserved.
 */
int cvmx_pki_style_alloc(int node, int style)
{
	int rs;

	if (cvmx_create_global_resource_range(CVMX_GR_TAG_STYLE(node),
					      CVMX_PKI_NUM_INTERNAL_STYLE)) {
		printf("ERROR: Failed to create styles global resource\n");
		return -1;
	}
	if (style >= 0) {
		/* Reserving specific style, use refcnt for sharing */
		rs = cvmx_atomic_fetch_and_add32(
			&cvmx_pki_style_refcnt[node][style], 1);
		if (rs > 0)
			return CVMX_RESOURCE_ALREADY_RESERVED;

		rs = cvmx_reserve_global_resource_range(CVMX_GR_TAG_STYLE(node),
							style, style, 1);
		if (rs == -1) {
			/* This means the style is taken by another app */
			printf("ERROR: style %d is reserved by another app\n",
			       style);
			cvmx_atomic_fetch_and_add32(
				&cvmx_pki_style_refcnt[node][style], -1);
			return CVMX_RESOURCE_ALLOC_FAILED;
		}
	} else {
		/* Allocate first available style */
		rs = cvmx_allocate_global_resource_range(
			CVMX_GR_TAG_STYLE(node), style, 1, 1);
		if (rs < 0) {
			printf("ERROR: Failed to allocate style, none available\n");
			return CVMX_RESOURCE_ALLOC_FAILED;
		}
		style = rs;
		/* Increment refcnt for newly created style */
		cvmx_atomic_fetch_and_add32(&cvmx_pki_style_refcnt[node][style],
					    1);
	}
	return style;
}

/**
 * This function frees a style from pool of global styles per node.
 * @param node	 node to free style from.
 * @param style	 style to free
 * @return	 0 on success, -1 on failure or
 * if the style is shared a positive count of remaining users for this style.
 */
int cvmx_pki_style_free(int node, int style)
{
	int rs;

	rs = cvmx_atomic_fetch_and_add32(&cvmx_pki_style_refcnt[node][style],
					 -1);
	if (rs > 1)
		return rs - 1;

	if (cvmx_free_global_resource_range_with_base(CVMX_GR_TAG_STYLE(node),
						      style, 1) == -1) {
		printf("ERROR Failed to release style %d\n", (int)style);
		return -1;
	}
	return 0;
}

/**
 * This function allocates/reserves a cluster group from per node
   cluster group resources.
 * @param node		node to allocate cluster group from.
   @param cl_grp	cluster group to allocate/reserve, if -1 ,
 *			allocate any available cluster group.
 * @return		cluster group number
 *			-1 on alloc failure.
 *			-2 on resource already reserved.
 */
int cvmx_pki_cluster_grp_alloc(int node, int cl_grp)
{
	int rs;

	if (node >= CVMX_MAX_NODES) {
		printf("ERROR: Invalid node number %d\n", node);
		return -1;
	}
	if (cvmx_create_global_resource_range(CVMX_GR_TAG_CLUSTER_GRP(node),
					      CVMX_PKI_NUM_CLUSTER_GROUP)) {
		printf("ERROR: Failed to create Cluster group global resource\n");
		return -1;
	}
	if (cl_grp >= 0) {
		rs = cvmx_reserve_global_resource_range(
			CVMX_GR_TAG_CLUSTER_GRP(node), 0, cl_grp, 1);
		if (rs == -1) {
			debug("INFO: cl_grp %d is already reserved\n",
			      (int)cl_grp);
			return CVMX_RESOURCE_ALREADY_RESERVED;
		}
	} else {
		rs = cvmx_allocate_global_resource_range(
			CVMX_GR_TAG_CLUSTER_GRP(node), 0, 1, 1);
		if (rs == -1) {
			debug("Warning: Failed to alloc cluster grp\n");
			return CVMX_RESOURCE_ALLOC_FAILED;
		}
	}
	cl_grp = rs;
	return cl_grp;
}

/**
 * This function allocates/reserves a pcam entry from node
 * @param node		node to allocate pcam entry from.
 * @param index	index of pacm entry (0-191), if -1 ,
 *			allocate any available pcam entry.
 * @param bank		pcam bank where to allocate/reserve pcan entry from
 * @param cluster_mask  mask of clusters from which pcam entry is needed.
 * @return		pcam entry of -1 on failure
 */
int cvmx_pki_pcam_entry_alloc(int node, int index, int bank, u64 cluster_mask)
{
	int rs = 0;
	unsigned int cluster;

	for (cluster = 0; cluster < CVMX_PKI_NUM_CLUSTER; cluster++) {
		if ((cluster_mask & (1 << cluster)) == 0)
			continue;
		rs = cvmx_create_global_resource_range(
			CVMX_GR_TAG_PCAM(node, cluster, bank),
			CVMX_PKI_TOTAL_PCAM_ENTRY);
		if (rs != 0) {
			printf("ERROR: Failed to create pki pcam global resource\n");
			return -1;
		}
		if (index >= 0)
			rs = cvmx_reserve_global_resource_range(
				CVMX_GR_TAG_PCAM(node, cluster, bank), cluster,
				index, 1);
		else
			rs = cvmx_allocate_global_resource_range(
				CVMX_GR_TAG_PCAM(node, cluster, bank), cluster,
				1, 1);
		if (rs == -1) {
			printf("ERROR: PCAM :index %d not available in cluster %d bank %d",
			       (int)index, (int)cluster, bank);
			return -1;
		}
	} /* for cluster */
	index = rs;
	/* implement cluster handle for pass2, for now assume
	all clusters will have same base index*/
	return index;
}

/**
 * This function allocates/reserves QPG table entries per node.
 * @param node		node number.
 * @param base_offset	base_offset in qpg table. If -1, first available
 *			qpg base_offset will be allocated. If base_offset is positive
 *			number and in range, it will try to allocate specified base_offset.
 * @param count		number of consecutive qpg entries to allocate. They will be consecutive
 *                       from base offset.
 * @return		qpg table base offset number on success
 *			-1 on alloc failure.
 *			-2 on resource already reserved.
 */
int cvmx_pki_qpg_entry_alloc(int node, int base_offset, int count)
{
	int rs;

	if (cvmx_create_global_resource_range(CVMX_GR_TAG_QPG_ENTRY(node),
					      CVMX_PKI_NUM_QPG_ENTRY)) {
		printf("ERROR: Failed to create qpg_entry global resource\n");
		return -1;
	}
	if (base_offset >= 0) {
		rs = cvmx_reserve_global_resource_range(
			CVMX_GR_TAG_QPG_ENTRY(node), base_offset, base_offset,
			count);
		if (rs == -1) {
			debug("INFO: qpg entry %d is already reserved\n",
			      (int)base_offset);
			return CVMX_RESOURCE_ALREADY_RESERVED;
		}
	} else {
		rs = cvmx_allocate_global_resource_range(
			CVMX_GR_TAG_QPG_ENTRY(node), base_offset, count, 1);
		if (rs == -1) {
			printf("ERROR: Failed to allocate qpg entry\n");
			return CVMX_RESOURCE_ALLOC_FAILED;
		}
	}
	base_offset = rs;
	return base_offset;
}

/**
 * This function frees QPG table entries per node.
 * @param node		node number.
 * @param base_offset	base_offset in qpg table. If -1, first available
 *			qpg base_offset will be allocated. If base_offset is positive
 *			number and in range, it will try to allocate specified base_offset.
 * @param count		number of consecutive qpg entries to allocate. They will be consecutive
 *			from base offset.
 * @return		qpg table base offset number on success, -1 on failure.
 */
int cvmx_pki_qpg_entry_free(int node, int base_offset, int count)
{
	if (cvmx_free_global_resource_range_with_base(
		    CVMX_GR_TAG_QPG_ENTRY(node), base_offset, count) == -1) {
		printf("ERROR Failed to release qpg offset %d",
		       (int)base_offset);
		return -1;
	}
	return 0;
}

int cvmx_pki_mtag_idx_alloc(int node, int idx)
{
	if (cvmx_create_global_resource_range(CVMX_GR_TAG_MTAG_IDX(node),
					      CVMX_PKI_NUM_MTAG_IDX)) {
		printf("ERROR: Failed to create MTAG-IDX global resource\n");
		return -1;
	}
	if (idx >= 0) {
		idx = cvmx_reserve_global_resource_range(
			CVMX_GR_TAG_MTAG_IDX(node), idx, idx, 1);
		if (idx == -1) {
			debug("INFO: MTAG index %d is already reserved\n",
			      (int)idx);
			return CVMX_RESOURCE_ALREADY_RESERVED;
		}
	} else {
		idx = cvmx_allocate_global_resource_range(
			CVMX_GR_TAG_MTAG_IDX(node), idx, 1, 1);
		if (idx == -1) {
			printf("ERROR: Failed to allocate MTAG index\n");
			return CVMX_RESOURCE_ALLOC_FAILED;
		}
	}
	return idx;
}
