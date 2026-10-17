/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2022-2023 Arm Limited and/or its affiliates <open-source-office@arm.com>
 *
 * Authors:
 *   Abdellatif El Khlifi <abdellatif.elkhlifi@arm.com>
 */

#ifndef __ARM_FFA_PRV_H
#define __ARM_FFA_PRV_H

#include <mapmem.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>

/* This header is exclusively used by the FF-A Uclass and FF-A driver(s) */

/* Arm FF-A driver name */
#define FFA_DRV_NAME "arm_ffa"

/* The FF-A SMC function definitions */

#if CONFIG_IS_ENABLED(SANDBOX)

/* Providing Arm SMCCC declarations to sandbox */

/**
 * struct sandbox_smccc_1_2_regs - emulated SMC call arguments or results
 * @a0-a17 argument values from registers 0 to 17
 */
struct sandbox_smccc_1_2_regs {
	ulong a0;
	ulong a1;
	ulong a2;
	ulong a3;
	ulong a4;
	ulong a5;
	ulong a6;
	ulong a7;
	ulong a8;
	ulong a9;
	ulong a10;
	ulong a11;
	ulong a12;
	ulong a13;
	ulong a14;
	ulong a15;
	ulong a16;
	ulong a17;
};

typedef struct sandbox_smccc_1_2_regs ffa_value_t;

#define ARM_SMCCC_FAST_CALL		1UL
#define ARM_SMCCC_OWNER_STANDARD	4
#define ARM_SMCCC_SMC_32		0
#define ARM_SMCCC_SMC_64		1
#define ARM_SMCCC_TYPE_SHIFT		31
#define ARM_SMCCC_CALL_CONV_SHIFT	30
#define ARM_SMCCC_OWNER_MASK		0x3f
#define ARM_SMCCC_OWNER_SHIFT		24
#define ARM_SMCCC_FUNC_MASK		0xffff

#define ARM_SMCCC_CALL_VAL(type, calling_convention, owner, func_num) \
	(((type) << ARM_SMCCC_TYPE_SHIFT) | \
	((calling_convention) << ARM_SMCCC_CALL_CONV_SHIFT) | \
	(((owner) & ARM_SMCCC_OWNER_MASK) << ARM_SMCCC_OWNER_SHIFT) | \
	((func_num) & ARM_SMCCC_FUNC_MASK))

#else
/* CONFIG_ARM64 */
#include <linux/arm-smccc.h>
typedef struct arm_smccc_1_2_regs ffa_value_t;
#endif

/* Defining the function pointer type for the function executing the FF-A ABIs */
typedef void (*invoke_ffa_fn_t)(ffa_value_t args, ffa_value_t *res);

/* FF-A driver version definitions */

#define MAJOR_VERSION_MASK		GENMASK(30, 16)
#define MINOR_VERSION_MASK		GENMASK(15, 0)
#define GET_FFA_MAJOR_VERSION(x)		\
				((u16)(FIELD_GET(MAJOR_VERSION_MASK, (x))))
#define GET_FFA_MINOR_VERSION(x)		\
				((u16)(FIELD_GET(MINOR_VERSION_MASK, (x))))
#define PACK_VERSION_INFO(major, minor)			\
	(FIELD_PREP(MAJOR_VERSION_MASK, (major)) |	\
	 FIELD_PREP(MINOR_VERSION_MASK, (minor)))

#define FFA_MAJOR_VERSION		(1)
#define FFA_MINOR_VERSION		(0)
#define FFA_VERSION_1_0		\
			PACK_VERSION_INFO(FFA_MAJOR_VERSION, FFA_MINOR_VERSION)

/* Endpoint ID mask (u-boot endpoint ID) */

#define GET_SELF_ENDPOINT_ID_MASK		GENMASK(15, 0)
#define GET_SELF_ENDPOINT_ID(x)		\
			((u16)(FIELD_GET(GET_SELF_ENDPOINT_ID_MASK, (x))))

#define PREP_SELF_ENDPOINT_ID_MASK		GENMASK(31, 16)
#define PREP_SELF_ENDPOINT_ID(x)		\
			(FIELD_PREP(PREP_SELF_ENDPOINT_ID_MASK, (x)))

/* Partition endpoint ID mask  (partition with which u-boot communicates with) */

#define PREP_PART_ENDPOINT_ID_MASK		GENMASK(15, 0)
#define PREP_PART_ENDPOINT_ID(x)		\
			(FIELD_PREP(PREP_PART_ENDPOINT_ID_MASK, (x)))

/* Definitions of the Arm FF-A interfaces supported by the Arm FF-A driver */

#define FFA_SMC(calling_convention, func_num)				\
	ARM_SMCCC_CALL_VAL(ARM_SMCCC_FAST_CALL, (calling_convention),	\
			   ARM_SMCCC_OWNER_STANDARD, (func_num))

#define FFA_SMC_32(func_num)				FFA_SMC(ARM_SMCCC_SMC_32, (func_num))
#define FFA_SMC_64(func_num)				FFA_SMC(ARM_SMCCC_SMC_64, (func_num))

enum ffa_abis {
	FFA_ERROR                 = 0x60,
	FFA_SUCCESS               = 0x61,
	FFA_INTERRUPT             = 0x62,
	FFA_VERSION               = 0x63,
	FFA_FEATURES              = 0x64,
	FFA_RX_RELEASE            = 0x65,
	FFA_RXTX_MAP              = 0x66,
	FFA_RXTX_UNMAP            = 0x67,
	FFA_PARTITION_INFO_GET    = 0x68,
	FFA_ID_GET                = 0x69,
	FFA_RUN                   = 0x6d,
	FFA_MSG_SEND_DIRECT_REQ   = 0x6f,
	FFA_MSG_SEND_DIRECT_RESP  = 0x70,

	/* To be updated when adding new FFA IDs */
	FFA_FIRST_ID              = FFA_ERROR, /* Lowest number ID */
	FFA_LAST_ID               = FFA_MSG_SEND_DIRECT_RESP, /* Highest number ID */
};

enum ffa_abi_errcode {
	NOT_SUPPORTED = 1,
	INVALID_PARAMETERS,
	NO_MEMORY,
	BUSY,
	INTERRUPTED,
	DENIED,
	RETRY,
	ABORTED,
	MAX_NUMBER_FFA_ERR
};

extern int ffa_to_std_errmap[MAX_NUMBER_FFA_ERR];

/* Container structure and helper macros to map between an FF-A error and relevant error log */
struct ffa_abi_errmap {
	char *err_str[MAX_NUMBER_FFA_ERR];
};

#define FFA_ERRMAP_COUNT (FFA_LAST_ID - FFA_FIRST_ID + 1)
#define FFA_ID_TO_ERRMAP_ID(ffa_id) ((ffa_id) - FFA_FIRST_ID)

/**
 * enum ffa_rxtx_buf_sizes - minimum sizes supported
 * for the RX/TX buffers
 */
enum ffa_rxtx_buf_sizes {
	RXTX_4K,
	RXTX_64K,
	RXTX_16K
};

/**
 * struct ffa_rxtxpair - Hosts the RX/TX buffers virtual addresses
 * @rxbuf:	virtual address of the RX buffer
 * @txbuf:	virtual address of the TX buffer
 * @rxtx_min_pages:	RX/TX buffers minimum size in pages
 *
 * Hosts the virtual addresses of the mapped RX/TX buffers
 * These addresses are used by the FF-A functions that use the RX/TX buffers
 */
struct ffa_rxtxpair {
	void *rxbuf; /* Virtual address returned by memalign */
	void *txbuf; /* Virtual address returned by memalign */
	size_t rxtx_min_pages; /* Minimum number of pages in each of the RX/TX buffers */
};

struct ffa_partition_desc;

/**
 * struct ffa_partitions - descriptors for all secure partitions
 * @count:	The number of partitions descriptors
 * @descs	The partitions descriptors table
 *
 * Contains the partitions descriptors table
 */
struct ffa_partitions {
	u32 count;
	struct ffa_partition_desc *descs; /* Virtual address */
};

/**
 * struct ffa_priv - the driver private data structure
 *
 * @fwk_version:	FF-A framework version
 * @emul:	FF-A sandbox emulator
 * @id:	u-boot endpoint ID
 * @partitions:	The partitions descriptors structure
 * @pair:	The RX/TX buffers pair
 *
 * The device private data structure containing all the
 * data read from secure world.
 */
struct ffa_priv {
	u32 fwk_version;
	struct udevice *emul;
	u16 id;
	struct ffa_partitions partitions;
	struct ffa_rxtxpair pair;
};

/**
 * ffa_get_version_hdlr() - FFA_VERSION handler function
 * @dev: The FF-A bus device
 *
 * Implement FFA_VERSION FF-A function
 * to get from the secure world the FF-A framework version
 * FFA_VERSION is used to discover the FF-A framework.
 *
 * Return:
 *
 * 0 on success. Otherwise, failure
 */
int ffa_get_version_hdlr(struct udevice *dev);

/**
 * invoke_ffa_fn() - SMC wrapper
 * @args: FF-A ABI arguments to be copied to Xn registers
 * @res: FF-A ABI return data to be copied from Xn registers
 *
 * Calls low level SMC implementation.
 * This function should be implemented by the user driver.
 */
void invoke_ffa_fn(ffa_value_t args, ffa_value_t *res);

#endif
