/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Resource management for PKI resources.
 */

#ifndef __CVMX_PKI_RESOURCES_H__
#define __CVMX_PKI_RESOURCES_H__

/**
 * This function allocates/reserves a style from pool of global styles per node.
 * @param node	 node to allocate style from.
 * @param style	 style to allocate, if -1 it will be allocated
		 first available style from style resource. If index is positive
		 number and in range, it will try to allocate specified style.
 * Return:	 style number on success, -1 on failure.
 */
int cvmx_pki_style_alloc(int node, int style);

/**
 * This function allocates/reserves a cluster group from per node
   cluster group resources.
 * @param node		node to allocate cluster group from.
   @param cl_grp	cluster group to allocate/reserve, if -1 ,
			allocate any available cluster group.
 * Return:		cluster group number or -1 on failure
 */
int cvmx_pki_cluster_grp_alloc(int node, int cl_grp);

/**
 * This function allocates/reserves a cluster from per node
   cluster resources.
 * @param node		node to allocate cluster group from.
   @param cluster_mask	mask of clusters  to allocate/reserve, if -1 ,
			allocate any available clusters.
 * @param num_clusters	number of clusters that will be allocated
 */
int cvmx_pki_cluster_alloc(int node, int num_clusters, u64 *cluster_mask);

/**
 * This function allocates/reserves a pcam entry from node
 * @param node		node to allocate pcam entry from.
   @param index	index of pacm entry (0-191), if -1 ,
			allocate any available pcam entry.
 * @param bank		pcam bank where to allocate/reserve pcan entry from
 * @param cluster_mask  mask of clusters from which pcam entry is needed.
 * Return:		pcam entry of -1 on failure
 */
int cvmx_pki_pcam_entry_alloc(int node, int index, int bank, u64 cluster_mask);

/**
 * This function allocates/reserves QPG table entries per node.
 * @param node		node number.
 * @param base_offset	base_offset in qpg table. If -1, first available
			qpg base_offset will be allocated. If base_offset is positive
			number and in range, it will try to allocate specified base_offset.
   @param count		number of consecutive qpg entries to allocate. They will be consecutive
			from base offset.
 * Return:		qpg table base offset number on success, -1 on failure.
 */
int cvmx_pki_qpg_entry_alloc(int node, int base_offset, int count);

/**
 * This function frees a style from pool of global styles per node.
 * @param node	 node to free style from.
 * @param style	 style to free
 * Return:	 0 on success, -1 on failure.
 */
int cvmx_pki_style_free(int node, int style);

/**
 * This function frees a cluster group from per node
   cluster group resources.
 * @param node		node to free cluster group from.
   @param cl_grp	cluster group to free
 * Return:		0 on success or -1 on failure
 */
int cvmx_pki_cluster_grp_free(int node, int cl_grp);

/**
 * This function frees QPG table entries per node.
 * @param node		node number.
 * @param base_offset	base_offset in qpg table. If -1, first available
 *			qpg base_offset will be allocated. If base_offset is positive
 *			number and in range, it will try to allocate specified base_offset.
 * @param count		number of consecutive qpg entries to allocate. They will be consecutive
 *			from base offset.
 * Return:		qpg table base offset number on success, -1 on failure.
 */
int cvmx_pki_qpg_entry_free(int node, int base_offset, int count);

/**
 * This function frees  clusters  from per node
   clusters resources.
 * @param node		node to free clusters from.
 * @param cluster_mask  mask of clusters need freeing
 * Return:		0 on success or -1 on failure
 */
int cvmx_pki_cluster_free(int node, u64 cluster_mask);

/**
 * This function frees a pcam entry from node
 * @param node		node to allocate pcam entry from.
   @param index	index of pacm entry (0-191) needs to be freed.
 * @param bank		pcam bank where to free pcam entry from
 * @param cluster_mask  mask of clusters from which pcam entry is freed.
 * Return:		0 on success OR -1 on failure
 */
int cvmx_pki_pcam_entry_free(int node, int index, int bank, u64 cluster_mask);

/**
 * This function allocates/reserves a bpid from pool of global bpid per node.
 * @param node	node to allocate bpid from.
 * @param bpid	bpid  to allocate, if -1 it will be allocated
 *		first available boid from bpid resource. If index is positive
 *		number and in range, it will try to allocate specified bpid.
 * Return:	bpid number on success,
 *		-1 on alloc failure.
 *		-2 on resource already reserved.
 */
int cvmx_pki_bpid_alloc(int node, int bpid);

/**
 * This function frees a bpid from pool of global bpid per node.
 * @param node	 node to free bpid from.
 * @param bpid	 bpid to free
 * Return:	 0 on success, -1 on failure or
 */
int cvmx_pki_bpid_free(int node, int bpid);

/**
 * This function frees all the PKI software resources
 * (clusters, styles, qpg_entry, pcam_entry etc) for the specified node
 */

/**
 * This function allocates/reserves an index from pool of global MTAG-IDX per node.
 * @param node	node to allocate index from.
 * @param idx	index  to allocate, if -1 it will be allocated
 * Return:	MTAG index number on success,
 *		-1 on alloc failure.
 *		-2 on resource already reserved.
 */
int cvmx_pki_mtag_idx_alloc(int node, int idx);

/**
 * This function frees an index from pool of global MTAG-IDX per node.
 * @param node	 node to free bpid from.
 * @param bpid	 bpid to free
 * Return:	 0 on success, -1 on failure or
 */
int cvmx_pki_mtag_idx_free(int node, int idx);

void __cvmx_pki_global_rsrc_free(int node);

#endif /*  __CVM_PKI_RESOURCES_H__ */
