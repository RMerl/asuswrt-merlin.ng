/*
 * Freescale Management Complex (MC) bus private declarations
 *
 * Copyright (C) 2014 Freescale Semiconductor, Inc.
 * Author: German Rivera <German.Rivera@freescale.com>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */
#ifndef _FSL_MC_PRIVATE_H_
#define _FSL_MC_PRIVATE_H_

#include "../include/mc.h"
#include <linux/mutex.h>
#include <linux/stringify.h>

#define FSL_MC_DPRC_DRIVER_NAME    "fsl_mc_dprc"

#define FSL_MC_DEVICE_MATCH(_mc_dev, _obj_desc) \
	(strcmp((_mc_dev)->obj_desc.type, (_obj_desc)->type) == 0 && \
	 (_mc_dev)->obj_desc.id == (_obj_desc)->id)

#define FSL_MC_IS_ALLOCATABLE(_obj_type) \
	(strcmp(_obj_type, "dpbp") == 0 || \
	 strcmp(_obj_type, "dpmcp") == 0 || \
	 strcmp(_obj_type, "dpcon") == 0)

/**
 * struct fsl_mc - Private data of a "fsl,qoriq-mc" platform device
 * @root_mc_bus_dev: MC object device representing the root DPRC
 * @addr_translation_ranges: array of bus to system address translation ranges
 */
struct fsl_mc {
	struct fsl_mc_device *root_mc_bus_dev;
	uint8_t num_translation_ranges;
	struct fsl_mc_addr_translation_range *translation_ranges;
};

/**
 * struct fsl_mc_addr_translation_range - bus to system address translation
 * range
 * @start_mc_addr: Start MC address of the range being translated
 * @end_mc_addr: MC address of the first byte after the range (last MC
 * address of the range is end_mc_addr - 1)
 * @start_phys_addr: system physical address corresponding to start_mc_addr
 */
struct fsl_mc_addr_translation_range {
	uint64_t start_mc_addr;
	uint64_t end_mc_addr;
	phys_addr_t start_phys_addr;
};

/**
 * struct fsl_mc_resource_pool - Pool of MC resources of a given
 * type
 * @type: type of resources in the pool
 * @max_count: maximum number of resources in the pool
 * @free_count: number of free resources in the pool
 * @mutex: mutex to serialize access to the pool's free list
 * @free_list: anchor node of list of free resources in the pool
 * @mc_bus: pointer to the MC bus that owns this resource pool
 */
struct fsl_mc_resource_pool {
	enum fsl_mc_pool_type type;
	int16_t max_count;
	int16_t free_count;
	struct mutex mutex;	/* serializes access to free_list */
	struct list_head free_list;
	struct fsl_mc_bus *mc_bus;
};

/**
 * struct fsl_mc_bus - logical bus that corresponds to a physical DPRC
 * @mc_dev: fsl-mc device for the bus device itself.
 * @resource_pools: array of resource pools (one pool per resource type)
 * for this MC bus. These resources represent allocatable entities
 * from the physical DPRC.
 * @scan_mutex: Serializes bus scanning
 */
struct fsl_mc_bus {
	struct fsl_mc_device mc_dev;
	struct fsl_mc_resource_pool resource_pools[FSL_MC_NUM_POOL_TYPES];
	struct mutex scan_mutex;    /* serializes bus scanning */
};

#define to_fsl_mc_bus(_mc_dev) \
	container_of(_mc_dev, struct fsl_mc_bus, mc_dev)

int __must_check fsl_mc_device_add(struct dprc_obj_desc *obj_desc,
				   struct fsl_mc_io *mc_io,
				   struct device *parent_dev,
				   struct fsl_mc_device **new_mc_dev);

void fsl_mc_device_remove(struct fsl_mc_device *mc_dev);

int dprc_scan_container(struct fsl_mc_device *mc_bus_dev);

int dprc_scan_objects(struct fsl_mc_device *mc_bus_dev);

int __init dprc_driver_init(void);

void __exit dprc_driver_exit(void);

int __init fsl_mc_allocator_driver_init(void);

void __exit fsl_mc_allocator_driver_exit(void);

int __must_check fsl_mc_resource_allocate(struct fsl_mc_bus *mc_bus,
					  enum fsl_mc_pool_type pool_type,
					  struct fsl_mc_resource
							  **new_resource);

void fsl_mc_resource_free(struct fsl_mc_resource *resource);

#endif /* _FSL_MC_PRIVATE_H_ */
