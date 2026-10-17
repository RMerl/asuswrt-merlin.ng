// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018-2022 Marvell International Ltd.
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
#include <mach/cvmx-ipd.h>
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

static struct global_resource_tag get_fpa1_resource_tag(void)
{
	return CVMX_GR_TAG_FPA;
}

static struct global_resource_tag get_fpa3_aura_resource_tag(int node)
{
	return cvmx_get_gr_tag('c', 'v', 'm', '_', 'a', 'u', 'r', 'a', '_',
			       node + '0', '.', '.', '.', '.', '.', '.');
}

static struct global_resource_tag get_fpa3_pool_resource_tag(int node)
{
	return cvmx_get_gr_tag('c', 'v', 'm', '_', 'p', 'o', 'o', 'l', '_',
			       node + '0', '.', '.', '.', '.', '.', '.');
}

int cvmx_fpa_get_max_pools(void)
{
	if (octeon_has_feature(OCTEON_FEATURE_FPA3))
		return cvmx_fpa3_num_auras();
	else if (OCTEON_IS_MODEL(OCTEON_CN68XX))
		/* 68xx pool 8 is not available via API */
		return CVMX_FPA1_NUM_POOLS;
	else
		return CVMX_FPA1_NUM_POOLS;
}

cvmx_fpa3_gaura_t cvmx_fpa3_reserve_aura(int node, int desired_aura_num)
{
	u64 owner = cvmx_get_app_id();
	int rv = 0;
	struct global_resource_tag tag;
	cvmx_fpa3_gaura_t aura;

	if (node == -1)
		node = cvmx_get_node_num();

	tag = get_fpa3_aura_resource_tag(node);

	if (cvmx_create_global_resource_range(tag, cvmx_fpa3_num_auras()) !=
	    0) {
		printf("ERROR: %s: global resource create node=%u\n", __func__,
		       node);
		return CVMX_FPA3_INVALID_GAURA;
	}

	if (desired_aura_num >= 0)
		rv = cvmx_reserve_global_resource_range(tag, owner,
							desired_aura_num, 1);
	else
		rv = cvmx_resource_alloc_reverse(tag, owner);

	if (rv < 0) {
		printf("ERROR: %s: node=%u desired aura=%d\n", __func__, node,
		       desired_aura_num);
		return CVMX_FPA3_INVALID_GAURA;
	}

	aura = __cvmx_fpa3_gaura(node, rv);

	return aura;
}

int cvmx_fpa3_release_aura(cvmx_fpa3_gaura_t aura)
{
	struct global_resource_tag tag = get_fpa3_aura_resource_tag(aura.node);
	int laura = aura.laura;

	if (!__cvmx_fpa3_aura_valid(aura))
		return -1;

	return cvmx_free_global_resource_range_multiple(tag, &laura, 1);
}

/**
 */
cvmx_fpa3_pool_t cvmx_fpa3_reserve_pool(int node, int desired_pool_num)
{
	u64 owner = cvmx_get_app_id();
	int rv = 0;
	struct global_resource_tag tag;
	cvmx_fpa3_pool_t pool;

	if (node == -1)
		node = cvmx_get_node_num();

	tag = get_fpa3_pool_resource_tag(node);

	if (cvmx_create_global_resource_range(tag, cvmx_fpa3_num_pools()) !=
	    0) {
		printf("ERROR: %s: global resource create node=%u\n", __func__,
		       node);
		return CVMX_FPA3_INVALID_POOL;
	}

	if (desired_pool_num >= 0)
		rv = cvmx_reserve_global_resource_range(tag, owner,
							desired_pool_num, 1);
	else
		rv = cvmx_resource_alloc_reverse(tag, owner);

	if (rv < 0) {
		/* Desired pool is already in use */
		return CVMX_FPA3_INVALID_POOL;
	}

	pool = __cvmx_fpa3_pool(node, rv);

	return pool;
}

int cvmx_fpa3_release_pool(cvmx_fpa3_pool_t pool)
{
	struct global_resource_tag tag = get_fpa3_pool_resource_tag(pool.node);
	int lpool = pool.lpool;

	if (!__cvmx_fpa3_pool_valid(pool))
		return -1;

	if (cvmx_create_global_resource_range(tag, cvmx_fpa3_num_pools()) !=
	    0) {
		printf("ERROR: %s: global resource create node=%u\n", __func__,
		       pool.node);
		return -1;
	}

	return cvmx_free_global_resource_range_multiple(tag, &lpool, 1);
}

cvmx_fpa1_pool_t cvmx_fpa1_reserve_pool(int desired_pool_num)
{
	u64 owner = cvmx_get_app_id();
	struct global_resource_tag tag;
	int rv;

	tag = get_fpa1_resource_tag();

	if (cvmx_create_global_resource_range(tag, CVMX_FPA1_NUM_POOLS) != 0) {
		printf("ERROR: %s: global resource not created\n", __func__);
		return -1;
	}

	if (desired_pool_num >= 0) {
		rv = cvmx_reserve_global_resource_range(tag, owner,
							desired_pool_num, 1);
	} else {
		rv = cvmx_resource_alloc_reverse(tag, owner);
	}

	if (rv < 0) {
		printf("ERROR: %s: FPA_POOL %d unavailable\n", __func__,
		       desired_pool_num);
		return CVMX_RESOURCE_ALREADY_RESERVED;
	}
	return (cvmx_fpa1_pool_t)rv;
}

int cvmx_fpa1_release_pool(cvmx_fpa1_pool_t pool)
{
	struct global_resource_tag tag;

	tag = get_fpa1_resource_tag();

	return cvmx_free_global_resource_range_multiple(tag, &pool, 1);
}
