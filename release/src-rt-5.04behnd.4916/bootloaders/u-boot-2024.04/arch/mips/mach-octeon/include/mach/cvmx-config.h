/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2018-2022 Marvell International Ltd.
 */

#ifndef __CVMX_CONFIG_H__
#define __CVMX_CONFIG_H__

/************************* Config Specific Defines ************************/
#define CVMX_LLM_NUM_PORTS 1

/**< PKO queues per port for interface 0 (ports 0-15) */
#define CVMX_PKO_QUEUES_PER_PORT_INTERFACE0 1

/**< PKO queues per port for interface 1 (ports 16-31) */
#define CVMX_PKO_QUEUES_PER_PORT_INTERFACE1 1

/**< PKO queues per port for interface 4 (AGL) */
#define CVMX_PKO_QUEUES_PER_PORT_INTERFACE4 1

/**< Limit on the number of PKO ports enabled for interface 0 */
#define CVMX_PKO_MAX_PORTS_INTERFACE0 CVMX_HELPER_PKO_MAX_PORTS_INTERFACE0

/**< Limit on the number of PKO ports enabled for interface 1 */
#define CVMX_PKO_MAX_PORTS_INTERFACE1 CVMX_HELPER_PKO_MAX_PORTS_INTERFACE1

/**< PKO queues per port for PCI (ports 32-35) */
#define CVMX_PKO_QUEUES_PER_PORT_PCI 1

/**< PKO queues per port for Loop devices (ports 36-39) */
#define CVMX_PKO_QUEUES_PER_PORT_LOOP 1

/**< PKO queues per port for SRIO0 devices (ports 40-41) */
#define CVMX_PKO_QUEUES_PER_PORT_SRIO0 1

/**< PKO queues per port for SRIO1 devices (ports 42-43) */
#define CVMX_PKO_QUEUES_PER_PORT_SRIO1 1

/************************* FPA allocation *********************************/
/* Pool sizes in bytes, must be multiple of a cache line */
#define CVMX_FPA_POOL_0_SIZE (16 * CVMX_CACHE_LINE_SIZE)
#define CVMX_FPA_POOL_1_SIZE (1 * CVMX_CACHE_LINE_SIZE)
#define CVMX_FPA_POOL_2_SIZE (8 * CVMX_CACHE_LINE_SIZE)
#define CVMX_FPA_POOL_3_SIZE (2 * CVMX_CACHE_LINE_SIZE)
#define CVMX_FPA_POOL_4_SIZE (0 * CVMX_CACHE_LINE_SIZE)
#define CVMX_FPA_POOL_5_SIZE (0 * CVMX_CACHE_LINE_SIZE)
#define CVMX_FPA_POOL_6_SIZE (8 * CVMX_CACHE_LINE_SIZE)
#define CVMX_FPA_POOL_7_SIZE (0 * CVMX_CACHE_LINE_SIZE)

/* Pools in use */
/**< Packet buffers */
#define CVMX_FPA_PACKET_POOL (0)
#ifndef CVMX_FPA_PACKET_POOL_SIZE
#define CVMX_FPA_PACKET_POOL_SIZE CVMX_FPA_POOL_0_SIZE
#endif

/**< Work queue entries */
#define CVMX_FPA_WQE_POOL      (1)
#define CVMX_FPA_WQE_POOL_SIZE CVMX_FPA_POOL_1_SIZE

/**< PKO queue command buffers */
#define CVMX_FPA_OUTPUT_BUFFER_POOL	 (2)
#define CVMX_FPA_OUTPUT_BUFFER_POOL_SIZE CVMX_FPA_POOL_2_SIZE

/**< BCH queue command buffers */
#define CVMX_FPA_BCH_POOL      (6)
#define CVMX_FPA_BCH_POOL_SIZE CVMX_FPA_POOL6_SIZE

/*************************  FAU allocation ********************************/
/* The fetch and add registers are allocated here.  They are arranged
 * in order of descending size so that all alignment constraints are
 * automatically met.
 * The enums are linked so that the following enum continues allocating
 * where the previous one left off, so the numbering within each
 * enum always starts with zero.  The macros take care of the address
 * increment size, so the values entered always increase by 1.
 * FAU registers are accessed with byte addresses.
 */

#define CVMX_FAU_REG_64_ADDR(x) (((x) << 3) + CVMX_FAU_REG_64_START)
typedef enum {
	CVMX_FAU_REG_64_START = 0,
	/**< FAU registers for the position in PKO command buffers */
	CVMX_FAU_REG_OQ_ADDR_INDEX = CVMX_FAU_REG_64_ADDR(0),
	/* Array of 36 */
	CVMX_FAU_REG_64_END = CVMX_FAU_REG_64_ADDR(36),
} cvmx_fau_reg_64_t;

#define CVMX_FAU_REG_32_ADDR(x) (((x) << 2) + CVMX_FAU_REG_32_START)
typedef enum {
	CVMX_FAU_REG_32_START = CVMX_FAU_REG_64_END,
	CVMX_FAU_REG_32_END = CVMX_FAU_REG_32_ADDR(0),
} cvmx_fau_reg_32_t;

#define CVMX_FAU_REG_16_ADDR(x) (((x) << 1) + CVMX_FAU_REG_16_START)
typedef enum {
	CVMX_FAU_REG_16_START = CVMX_FAU_REG_32_END,
	CVMX_FAU_REG_16_END = CVMX_FAU_REG_16_ADDR(0),
} cvmx_fau_reg_16_t;

#define CVMX_FAU_REG_8_ADDR(x) ((x) + CVMX_FAU_REG_8_START)
typedef enum {
	CVMX_FAU_REG_8_START = CVMX_FAU_REG_16_END,
	CVMX_FAU_REG_8_END = CVMX_FAU_REG_8_ADDR(0),
} cvmx_fau_reg_8_t;

/* The name CVMX_FAU_REG_AVAIL_BASE is provided to indicate the first available
 * FAU address that is not allocated in cvmx-config.h. This is 64 bit aligned.
 */
#define CVMX_FAU_REG_AVAIL_BASE ((CVMX_FAU_REG_8_END + 0x7) & (~0x7ULL))
#define CVMX_FAU_REG_END	(2048)

/********************** scratch memory allocation *************************/
/* Scratchpad memory allocation.  Note that these are byte memory addresses.
 * Some uses of scratchpad (IOBDMA for example) require the use of 8-byte
 * aligned addresses, so proper alignment needs to be taken into account.
 */

/**< Pre allocation for PKO queue command buffers */
#define CVMX_SCR_OQ_BUF_PRE_ALLOC (0)

/**< Generic scratch iobdma area */
#define CVMX_SCR_SCRATCH (8)

/**< First location available after cvmx-config.h allocated region. */
#define CVMX_SCR_REG_AVAIL_BASE (16)

#endif /* __CVMX_CONFIG_H__ */
