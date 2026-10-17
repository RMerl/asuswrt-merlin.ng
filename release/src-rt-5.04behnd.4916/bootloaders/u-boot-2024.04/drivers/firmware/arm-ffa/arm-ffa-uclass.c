// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022-2023 Arm Limited and/or its affiliates <open-source-office@arm.com>
 *
 * Authors:
 *   Abdellatif El Khlifi <abdellatif.elkhlifi@arm.com>
 */
#include <common.h>
#include <arm_ffa.h>
#include <arm_ffa_priv.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <string.h>
#include <uuid.h>
#include <asm/global_data.h>
#include <dm/device-internal.h>
#include <dm/devres.h>
#include <dm/root.h>
#include <linux/errno.h>
#include <linux/sizes.h>

DECLARE_GLOBAL_DATA_PTR;

/* Error mapping declarations */

int ffa_to_std_errmap[MAX_NUMBER_FFA_ERR] = {
	[NOT_SUPPORTED] = -EOPNOTSUPP,
	[INVALID_PARAMETERS] = -EINVAL,
	[NO_MEMORY] = -ENOMEM,
	[BUSY] = -EBUSY,
	[INTERRUPTED] = -EINTR,
	[DENIED] = -EACCES,
	[RETRY] = -EAGAIN,
	[ABORTED] = -ECANCELED,
};

static struct ffa_abi_errmap err_msg_map[FFA_ERRMAP_COUNT] = {
	[FFA_ID_TO_ERRMAP_ID(FFA_VERSION)] = {
		{
			[NOT_SUPPORTED] =
			"NOT_SUPPORTED: A Firmware Framework implementation does not exist",
		},
	},
	[FFA_ID_TO_ERRMAP_ID(FFA_ID_GET)] = {
		{
			[NOT_SUPPORTED] =
			"NOT_SUPPORTED: This function is not implemented at this FF-A instance",
		},
	},
	[FFA_ID_TO_ERRMAP_ID(FFA_FEATURES)] = {
		{
			[NOT_SUPPORTED] =
			"NOT_SUPPORTED: FFA_RXTX_MAP is not implemented at this FF-A instance",
		},
	},
	[FFA_ID_TO_ERRMAP_ID(FFA_PARTITION_INFO_GET)] = {
		{
			[NOT_SUPPORTED] =
			"NOT_SUPPORTED: This function is not implemented at this FF-A instance",
			[INVALID_PARAMETERS] =
			"INVALID_PARAMETERS: Unrecognized UUID",
			[NO_MEMORY] =
			"NO_MEMORY: Results cannot fit in RX buffer of the caller",
			[BUSY] =
			"BUSY: RX buffer of the caller is not free",
			[DENIED] =
			"DENIED: Callee is not in a state to handle this request",
		},
	},
	[FFA_ID_TO_ERRMAP_ID(FFA_RXTX_UNMAP)] = {
		{
			[NOT_SUPPORTED] =
			"NOT_SUPPORTED: FFA_RXTX_UNMAP is not implemented at this FF-A instance",
			[INVALID_PARAMETERS] =
			"INVALID_PARAMETERS: No buffer pair registered on behalf of the caller",
		},
	},
	[FFA_ID_TO_ERRMAP_ID(FFA_RX_RELEASE)] = {
		{
			[NOT_SUPPORTED] =
			"NOT_SUPPORTED: FFA_RX_RELEASE is not implemented at this FF-A instance",
			[DENIED] =
			"DENIED: Caller did not have ownership of the RX buffer",
		},
	},
	[FFA_ID_TO_ERRMAP_ID(FFA_RXTX_MAP)] = {
		{
			[NOT_SUPPORTED] =
			"NOT_SUPPORTED: This function is not implemented at this FF-A instance",
			[INVALID_PARAMETERS] =
			"INVALID_PARAMETERS: Field(s) in input parameters incorrectly encoded",
			[NO_MEMORY] =
			"NO_MEMORY: Not enough memory",
			[DENIED] =
			"DENIED: Buffer pair already registered",
		},
	},
};

/**
 * ffa_to_std_errno() - convert FF-A error code to standard error code
 * @ffa_errno:	Error code returned by the FF-A ABI
 *
 * Map the given FF-A error code as specified
 * by the spec to a u-boot standard error code.
 *
 * Return:
 *
 * The standard error code on success. . Otherwise, failure
 */
static int ffa_to_std_errno(int ffa_errno)
{
	int err_idx = -ffa_errno;

	/* Map the FF-A error code to the standard u-boot error code */
	if (err_idx > 0 && err_idx < MAX_NUMBER_FFA_ERR)
		return ffa_to_std_errmap[err_idx];
	return -EINVAL;
}

/**
 * ffa_print_error_log() - print the error log corresponding to the selected FF-A ABI
 * @ffa_id:	FF-A ABI ID
 * @ffa_errno:	Error code returned by the FF-A ABI
 *
 * Map the FF-A error code to the error log relevant to the
 * selected FF-A ABI. Then the error log is printed.
 *
 * Return:
 *
 * 0 on success. . Otherwise, failure
 */
static int ffa_print_error_log(u32 ffa_id, int ffa_errno)
{
	int err_idx = -ffa_errno, abi_idx = 0;

	/* Map the FF-A error code to the corresponding error log */

	if (err_idx <= 0 || err_idx >= MAX_NUMBER_FFA_ERR)
		return -EINVAL;

	if (ffa_id < FFA_FIRST_ID || ffa_id > FFA_LAST_ID)
		return -EINVAL;

	abi_idx = FFA_ID_TO_ERRMAP_ID(ffa_id);

	if (!err_msg_map[abi_idx].err_str[err_idx])
		return -EINVAL;

	log_err("%s\n", err_msg_map[abi_idx].err_str[err_idx]);

	return 0;
}

/* FF-A ABIs implementation (U-Boot side) */

/**
 * invoke_ffa_fn() - SMC wrapper
 * @args: FF-A ABI arguments to be copied to Xn registers
 * @res: FF-A ABI return data to be copied from Xn registers
 *
 * Calls low level SMC implementation.
 * This function should be implemented by the user driver.
 */
void __weak invoke_ffa_fn(ffa_value_t args, ffa_value_t *res)
{
}

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
int ffa_get_version_hdlr(struct udevice *dev)
{
	u16 major, minor;
	ffa_value_t res = {0};
	int ffa_errno;
	struct ffa_priv *uc_priv;

	invoke_ffa_fn((ffa_value_t){
			.a0 = FFA_SMC_32(FFA_VERSION), .a1 = FFA_VERSION_1_0,
			}, &res);

	ffa_errno = res.a0;
	if (ffa_errno < 0) {
		ffa_print_error_log(FFA_VERSION, ffa_errno);
		return ffa_to_std_errno(ffa_errno);
	}

	major = GET_FFA_MAJOR_VERSION(res.a0);
	minor = GET_FFA_MINOR_VERSION(res.a0);

	log_debug("FF-A driver %d.%d\nFF-A framework %d.%d\n",
		 FFA_MAJOR_VERSION, FFA_MINOR_VERSION, major, minor);

	if (major == FFA_MAJOR_VERSION && minor >= FFA_MINOR_VERSION) {
		log_debug("FF-A versions are compatible\n");

		if (dev) {
			uc_priv = dev_get_uclass_priv(dev);
			if (uc_priv)
				uc_priv->fwk_version = res.a0;
		}

		return 0;
	}

	log_err("versions are incompatible\nExpected: %d.%d , Found: %d.%d\n",
		FFA_MAJOR_VERSION, FFA_MINOR_VERSION, major, minor);

	return -EPROTONOSUPPORT;
}

/**
 * ffa_get_endpoint_id() - FFA_ID_GET handler function
 * @dev: The FF-A bus device
 *
 * Implement FFA_ID_GET FF-A function
 * to get from the secure world u-boot endpoint ID
 *
 * Return:
 *
 * 0 on success. Otherwise, failure
 */
static int ffa_get_endpoint_id(struct udevice *dev)
{
	ffa_value_t res = {0};
	int ffa_errno;
	struct ffa_priv *uc_priv = dev_get_uclass_priv(dev);

	invoke_ffa_fn((ffa_value_t){
			.a0 = FFA_SMC_32(FFA_ID_GET),
			}, &res);

	if (res.a0 == FFA_SMC_32(FFA_SUCCESS)) {
		uc_priv->id = GET_SELF_ENDPOINT_ID((u32)res.a2);
		log_debug("FF-A endpoint ID is %u\n", uc_priv->id);

		return 0;
	}

	ffa_errno = res.a2;

	ffa_print_error_log(FFA_ID_GET, ffa_errno);

	return ffa_to_std_errno(ffa_errno);
}

/**
 * ffa_set_rxtx_buffers_pages_cnt() - set the minimum number of pages in each of the RX/TX buffers
 * @dev: The FF-A bus device
 * @prop_field: properties field obtained from FFA_FEATURES ABI
 *
 * Set the minimum number of pages in each of the RX/TX buffers in uc_priv
 *
 * Return:
 *
 * rxtx_min_pages field contains the returned number of pages
 * 0 on success. Otherwise, failure
 */
static int ffa_set_rxtx_buffers_pages_cnt(struct udevice *dev, u32 prop_field)
{
	struct ffa_priv *uc_priv = dev_get_uclass_priv(dev);

	switch (prop_field) {
	case RXTX_4K:
		uc_priv->pair.rxtx_min_pages = 1;
		break;
	case RXTX_16K:
		uc_priv->pair.rxtx_min_pages = 4;
		break;
	case RXTX_64K:
		uc_priv->pair.rxtx_min_pages = 16;
		break;
	default:
		log_err("RX/TX buffer size not supported\n");
		return -EINVAL;
	}

	return 0;
}

/**
 * ffa_get_rxtx_map_features_hdlr() - FFA_FEATURES handler function with FFA_RXTX_MAP argument
 * @dev: The FF-A bus device
 *
 * Implement FFA_FEATURES FF-A function to retrieve the FFA_RXTX_MAP features
 *
 * Return:
 *
 * 0 on success. Otherwise, failure
 */
static int ffa_get_rxtx_map_features_hdlr(struct udevice *dev)
{
	ffa_value_t res = {0};
	int ffa_errno;

	invoke_ffa_fn((ffa_value_t){
			.a0 = FFA_SMC_32(FFA_FEATURES),
			.a1 = FFA_SMC_64(FFA_RXTX_MAP),
			}, &res);

	if (res.a0 == FFA_SMC_32(FFA_SUCCESS))
		return ffa_set_rxtx_buffers_pages_cnt(dev, res.a2);

	ffa_errno = res.a2;
	ffa_print_error_log(FFA_FEATURES, ffa_errno);

	return ffa_to_std_errno(ffa_errno);
}

/**
 * ffa_free_rxtx_buffers() - free the RX/TX buffers
 * @dev: The FF-A bus device
 *
 * Free the RX/TX buffers
 */
static void ffa_free_rxtx_buffers(struct udevice *dev)
{
	struct ffa_priv *uc_priv = dev_get_uclass_priv(dev);

	log_debug("Freeing FF-A RX/TX buffers\n");

	if (uc_priv->pair.rxbuf) {
		free(uc_priv->pair.rxbuf);
		uc_priv->pair.rxbuf = NULL;
	}

	if (uc_priv->pair.txbuf) {
		free(uc_priv->pair.txbuf);
		uc_priv->pair.txbuf = NULL;
	}
}

/**
 * ffa_alloc_rxtx_buffers() - allocate the RX/TX buffers
 * @dev: The FF-A bus device
 *
 * Used by ffa_map_rxtx_buffers to allocate
 * the RX/TX buffers before mapping them. The allocated memory is physically
 * contiguous since memalign ends up calling malloc which allocates
 * contiguous memory in u-boot.
 * The size of the memory allocated is the minimum allowed.
 *
 * Return:
 *
 * 0 on success. Otherwise, failure
 */
static int ffa_alloc_rxtx_buffers(struct udevice *dev)
{
	u64 bytes;
	struct ffa_priv *uc_priv = dev_get_uclass_priv(dev);

	log_debug("Using %lu 4KB page(s) for FF-A RX/TX buffers size\n",
		  uc_priv->pair.rxtx_min_pages);

	bytes = uc_priv->pair.rxtx_min_pages * SZ_4K;

	/*
	 * The alignment of the RX and TX buffers must be equal
	 * to the larger translation granule size
	 * Assumption: Memory allocated with memalign is always physically contiguous
	 */

	uc_priv->pair.rxbuf = memalign(bytes, bytes);
	if (!uc_priv->pair.rxbuf) {
		log_err("failure to allocate RX buffer\n");
		return -ENOBUFS;
	}

	log_debug("FF-A RX buffer at virtual address %p\n", uc_priv->pair.rxbuf);

	uc_priv->pair.txbuf = memalign(bytes, bytes);
	if (!uc_priv->pair.txbuf) {
		free(uc_priv->pair.rxbuf);
		uc_priv->pair.rxbuf = NULL;
		log_err("failure to allocate the TX buffer\n");
		return -ENOBUFS;
	}

	log_debug("FF-A TX buffer at virtual address %p\n", uc_priv->pair.txbuf);

	/* Make sure the buffers are cleared before use */
	memset(uc_priv->pair.rxbuf, 0, bytes);
	memset(uc_priv->pair.txbuf, 0, bytes);

	return 0;
}

/**
 * ffa_map_rxtx_buffers_hdlr() - FFA_RXTX_MAP handler function
 * @dev: The FF-A bus device
 *
 * Implement FFA_RXTX_MAP FF-A function to map the RX/TX buffers
 *
 * Return:
 *
 * 0 on success. Otherwise, failure
 */
static int ffa_map_rxtx_buffers_hdlr(struct udevice *dev)
{
	int ret;
	ffa_value_t res = {0};
	int ffa_errno;
	struct ffa_priv *uc_priv = dev_get_uclass_priv(dev);

	ret = ffa_alloc_rxtx_buffers(dev);
	if (ret)
		return ret;

	/*
	 * we need to pass the physical addresses of the RX/TX buffers
	 * in u-boot physical/virtual mapping is 1:1
	 * no need to convert from virtual to physical
	 */

	invoke_ffa_fn((ffa_value_t){
			.a0 = FFA_SMC_64(FFA_RXTX_MAP),
			.a1 = map_to_sysmem(uc_priv->pair.txbuf),
			.a2 = map_to_sysmem(uc_priv->pair.rxbuf),
			.a3 = uc_priv->pair.rxtx_min_pages,
			}, &res);

	if (res.a0 == FFA_SMC_32(FFA_SUCCESS)) {
		log_debug("FF-A RX/TX buffers mapped\n");
		return 0;
	}

	ffa_errno = res.a2;
	ffa_print_error_log(FFA_RXTX_MAP, ffa_errno);

	ffa_free_rxtx_buffers(dev);

	return ffa_to_std_errno(ffa_errno);
}

/**
 * ffa_unmap_rxtx_buffers_hdlr() - FFA_RXTX_UNMAP handler function
 * @dev: The FF-A bus device
 *
 * Implement FFA_RXTX_UNMAP FF-A function to unmap the RX/TX buffers
 *
 * Return:
 *
 * 0 on success. Otherwise, failure
 */
int ffa_unmap_rxtx_buffers_hdlr(struct udevice *dev)
{
	ffa_value_t res = {0};
	int ffa_errno;
	struct ffa_priv *uc_priv;

	log_debug("unmapping FF-A RX/TX buffers\n");

	uc_priv = dev_get_uclass_priv(dev);

	invoke_ffa_fn((ffa_value_t){
			.a0 = FFA_SMC_32(FFA_RXTX_UNMAP),
			.a1 = PREP_SELF_ENDPOINT_ID(uc_priv->id),
			}, &res);

	if (res.a0 == FFA_SMC_32(FFA_SUCCESS)) {
		ffa_free_rxtx_buffers(dev);
		return 0;
	}

	ffa_errno = res.a2;
	ffa_print_error_log(FFA_RXTX_UNMAP, ffa_errno);

	return ffa_to_std_errno(ffa_errno);
}

/**
 * ffa_release_rx_buffer_hdlr() - FFA_RX_RELEASE handler function
 * @dev: The FF-A bus device
 *
 * Invoke FFA_RX_RELEASE FF-A function to release the ownership of the RX buffer
 *
 * Return:
 *
 * 0 on success. Otherwise, failure
 */
static int ffa_release_rx_buffer_hdlr(struct udevice *dev)
{
	ffa_value_t res = {0};
	int ffa_errno;

	invoke_ffa_fn((ffa_value_t){
			.a0 = FFA_SMC_32(FFA_RX_RELEASE),
			}, &res);

	if (res.a0 == FFA_SMC_32(FFA_SUCCESS))
		return 0;

	ffa_errno = res.a2;
	ffa_print_error_log(FFA_RX_RELEASE, ffa_errno);

	return ffa_to_std_errno(ffa_errno);
}

/**
 * ffa_uuid_are_identical() - check whether two given UUIDs are identical
 * @uuid1: first UUID
 * @uuid2: second UUID
 *
 * Used by ffa_read_partitions_info to search for a UUID in the partitions descriptors table
 *
 * Return:
 *
 * 1 when UUIDs match. Otherwise, 0
 */
static bool ffa_uuid_are_identical(const struct ffa_partition_uuid *uuid1,
				   const struct ffa_partition_uuid *uuid2)
{
	if (!uuid1 || !uuid2)
		return 0;

	return !memcmp(uuid1, uuid2, sizeof(struct ffa_partition_uuid));
}

/**
 * ffa_read_partitions_info() - read queried partition data
 * @dev: The FF-A bus device
 * @count: The number of partitions queried
 * @part_uuid: Pointer to the partition(s) UUID
 *
 * Read the partitions information returned by the FFA_PARTITION_INFO_GET and saves it in uc_priv
 *
 * Return:
 *
 * uc_priv is updated with the partition(s) information
 * 0 is returned on success. Otherwise, failure
 */
static int ffa_read_partitions_info(struct udevice *dev, u32 count,
				    struct ffa_partition_uuid *part_uuid)
{
	struct ffa_priv *uc_priv = dev_get_uclass_priv(dev);

	if (!count) {
		log_err("no partition detected\n");
		return -ENODATA;
	}

	log_debug("Reading FF-A partitions data from the RX buffer\n");

	if (!part_uuid) {
		/* Querying information of all partitions */
		u64 buf_bytes;
		u64 data_bytes;
		u32 desc_idx;
		struct ffa_partition_info *parts_info;

		data_bytes = count * sizeof(struct ffa_partition_desc);

		buf_bytes = uc_priv->pair.rxtx_min_pages * SZ_4K;

		if (data_bytes > buf_bytes) {
			log_err("partitions data size exceeds the RX buffer size:\n");
			log_err("    sizes in bytes: data %llu , RX buffer %llu\n",
				data_bytes,
				buf_bytes);

			return -ENOMEM;
		}

		uc_priv->partitions.descs = devm_kmalloc(dev, data_bytes, __GFP_ZERO);
		if (!uc_priv->partitions.descs) {
			log_err("cannot  allocate partitions data buffer\n");
			return -ENOMEM;
		}

		parts_info = uc_priv->pair.rxbuf;

		for (desc_idx = 0 ; desc_idx < count ; desc_idx++) {
			uc_priv->partitions.descs[desc_idx].info =
				parts_info[desc_idx];

			log_debug("FF-A partition ID %x : info cached\n",
				  uc_priv->partitions.descs[desc_idx].info.id);
		}

		uc_priv->partitions.count = count;

		log_debug("%d FF-A partition(s) found and cached\n", count);

	} else {
		u32 rx_desc_idx, cached_desc_idx;
		struct ffa_partition_info *parts_info;
		u8 desc_found;

		parts_info = uc_priv->pair.rxbuf;

		/*
		 * Search for the SP IDs read from the RX buffer
		 * in the already cached SPs.
		 * Update the UUID when ID found.
		 */
		for (rx_desc_idx = 0; rx_desc_idx < count ; rx_desc_idx++) {
			desc_found = 0;

			/* Search the current ID in the cached partitions */
			for (cached_desc_idx = 0;
			     cached_desc_idx < uc_priv->partitions.count;
			     cached_desc_idx++) {
				/* Save the UUID */
				if (uc_priv->partitions.descs[cached_desc_idx].info.id ==
				    parts_info[rx_desc_idx].id) {
					uc_priv->partitions.descs[cached_desc_idx].sp_uuid =
						*part_uuid;

					desc_found = 1;
					break;
				}
			}

			if (!desc_found)
				return -ENODATA;
		}
	}

	return  0;
}

/**
 * ffa_query_partitions_info() - invoke FFA_PARTITION_INFO_GET and save partitions data
 * @dev: The FF-A bus device
 * @part_uuid: Pointer to the partition(s) UUID
 * @pcount: Pointer to the number of partitions variable filled when querying
 *
 * Execute the FFA_PARTITION_INFO_GET to query the partitions data.
 * Then, call ffa_read_partitions_info to save the data in uc_priv.
 *
 * After reading the data the RX buffer is released using ffa_release_rx_buffer
 *
 * Return:
 *
 * When part_uuid is NULL, all partitions data are retrieved from secure world
 * When part_uuid is non NULL, data for partitions matching the given UUID are
 * retrieved and the number of partitions is returned
 * 0 is returned on success. Otherwise, failure
 */
static int ffa_query_partitions_info(struct udevice *dev, struct ffa_partition_uuid *part_uuid,
				     u32 *pcount)
{
	struct ffa_partition_uuid query_uuid = {0};
	ffa_value_t res = {0};
	int ffa_errno;

	/*
	 * If a UUID is specified. Information for one or more
	 * partitions in the system is queried. Otherwise, information
	 * for all installed partitions is queried
	 */

	if (part_uuid) {
		if (!pcount)
			return -EINVAL;

		query_uuid = *part_uuid;
	} else if (pcount) {
		return -EINVAL;
	}

	invoke_ffa_fn((ffa_value_t){
			.a0 = FFA_SMC_32(FFA_PARTITION_INFO_GET),
			.a1 = query_uuid.a1,
			.a2 = query_uuid.a2,
			.a3 = query_uuid.a3,
			.a4 = query_uuid.a4,
			}, &res);

	if (res.a0 == FFA_SMC_32(FFA_SUCCESS)) {
		int ret;

		/*
		 * res.a2 contains the count of partition information descriptors
		 * populated in the RX buffer
		 */
		if (res.a2) {
			ret = ffa_read_partitions_info(dev, (u32)res.a2, part_uuid);
			if (ret) {
				log_err("failed reading SP(s) data , err (%d)\n", ret);
				ffa_release_rx_buffer_hdlr(dev);
				return -EINVAL;
			}
		}

		/* Return the SP count (when querying using a UUID) */
		if (pcount)
			*pcount = (u32)res.a2;

		/*
		 * After calling FFA_PARTITION_INFO_GET the buffer ownership
		 * is assigned to the consumer (u-boot). So, we need to give
		 * the ownership back to the SPM or hypervisor
		 */
		ret = ffa_release_rx_buffer_hdlr(dev);

		return ret;
	}

	ffa_errno = res.a2;
	ffa_print_error_log(FFA_PARTITION_INFO_GET, ffa_errno);

	return ffa_to_std_errno(ffa_errno);
}

/**
 * ffa_get_partitions_info_hdlr() - FFA_PARTITION_INFO_GET handler function
 *	@uuid_str: pointer to the UUID string
 *	@sp_count: address of the variable containing the number of partitions matching the UUID
 *			 The variable is set by the driver
 *	@sp_descs: address of the descriptors of the partitions matching the UUID
 *			 The address is set by the driver
 *
 * Return the number of partitions and their descriptors matching the UUID
 *
 * Query the secure partition data from uc_priv.
 * If not found, invoke FFA_PARTITION_INFO_GET FF-A function to query the partition information
 * from secure world.
 *
 * A client of the FF-A driver should know the UUID of the service it wants to
 * access. It should use the UUID to request the FF-A driver to provide the
 * partition(s) information of the service. The FF-A driver uses
 * PARTITION_INFO_GET to obtain this information. This is implemented through
 * ffa_get_partitions_info_hdlr() function.
 * If the partition(s) matching the UUID found, the partition(s) information and the
 * number are returned.
 * If no partition matching the UUID is found in the cached area, a new FFA_PARTITION_INFO_GET
 * call is issued.
 * If not done yet, the UUID is updated in the cached area.
 * This assumes that partitions data does not change in the secure world.
 * Otherwise u-boot will have an outdated partition data. The benefit of caching
 * the information in the FF-A driver is to accommodate discovery after
 * ExitBootServices().
 *
 * Return:
 *
 * @sp_count: the number of partitions
 * @sp_descs: address of the partitions descriptors
 *
 * On success 0 is returned. Otherwise, failure
 */
int ffa_get_partitions_info_hdlr(struct udevice *dev, const char *uuid_str,
				 u32 *sp_count, struct ffa_partition_desc **sp_descs)
{
	u32 i;
	struct ffa_partition_uuid part_uuid = {0};
	struct ffa_priv *uc_priv;
	struct ffa_partition_desc *rx_descs;

	uc_priv = dev_get_uclass_priv(dev);

	if (!uc_priv->partitions.count || !uc_priv->partitions.descs) {
		log_err("no partition installed\n");
		return -EINVAL;
	}

	if (!uuid_str) {
		log_err("no UUID provided\n");
		return -EINVAL;
	}

	if (!sp_count) {
		log_err("no count argument provided\n");
		return -EINVAL;
	}

	if (!sp_descs) {
		log_err("no info argument provided\n");
		return -EINVAL;
	}

	if (uuid_str_to_le_bin(uuid_str, (unsigned char *)&part_uuid)) {
		log_err("invalid UUID\n");
		return -EINVAL;
	}

	log_debug("Searching FF-A partitions using the provided UUID\n");

	*sp_count = 0;
	*sp_descs = uc_priv->pair.rxbuf;
	rx_descs = *sp_descs;

	/* Search in the cached partitions */
	for (i = 0; i < uc_priv->partitions.count; i++)
		if (ffa_uuid_are_identical(&uc_priv->partitions.descs[i].sp_uuid,
					   &part_uuid)) {
			log_debug("FF-A partition ID %x matches the provided UUID\n",
				  uc_priv->partitions.descs[i].info.id);

			(*sp_count)++;
			*rx_descs++ = uc_priv->partitions.descs[i];
			}

	if (!(*sp_count)) {
		int ret;

		log_debug("No FF-A partition found. Querying framework ...\n");

		ret = ffa_query_partitions_info(dev, &part_uuid, sp_count);

		if (!ret) {
			log_debug("Number of FF-A partition(s) matching the UUID: %d\n", *sp_count);

			if (*sp_count)
				ret = ffa_get_partitions_info_hdlr(dev, uuid_str, sp_count,
								   sp_descs);
			else
				ret = -ENODATA;
		}

		return ret;
	}

	return 0;
}

/**
 * ffa_cache_partitions_info() - Query and saves all secure partitions data
 * @dev: The FF-A bus device
 *
 * Invoke FFA_PARTITION_INFO_GET FF-A function to query from secure world
 * all partitions information.
 *
 * The FFA_PARTITION_INFO_GET call is issued with nil UUID as an argument.
 * All installed partitions information are returned. We cache them in uc_priv
 * and we keep the UUID field empty (in FF-A 1.0 UUID is not provided by the partition descriptor)
 *
 * Called at the device probing level.
 * ffa_cache_partitions_info uses ffa_query_partitions_info to get the data
 *
 * Return:
 *
 * 0 on success. Otherwise, failure
 */
static int ffa_cache_partitions_info(struct udevice *dev)
{
	return ffa_query_partitions_info(dev, NULL, NULL);
}

/**
 * ffa_msg_send_direct_req_hdlr() - FFA_MSG_SEND_DIRECT_{REQ,RESP} handler function
 * @dev: The FF-A bus device
 * @dst_part_id: destination partition ID
 * @msg: pointer to the message data preallocated by the client (in/out)
 * @is_smc64: select 64-bit or 32-bit FF-A ABI
 *
 * Implement FFA_MSG_SEND_DIRECT_{REQ,RESP}
 * FF-A functions.
 *
 * FFA_MSG_SEND_DIRECT_REQ is used to send the data to the secure partition.
 * The response from the secure partition is handled by reading the
 * FFA_MSG_SEND_DIRECT_RESP arguments.
 *
 * The maximum size of the data that can be exchanged is 40 bytes which is
 * sizeof(struct ffa_send_direct_data) as defined by the FF-A specification 1.0
 * in the section relevant to FFA_MSG_SEND_DIRECT_{REQ,RESP}
 *
 * Return:
 *
 * 0 on success. Otherwise, failure
 */
int ffa_msg_send_direct_req_hdlr(struct udevice *dev, u16 dst_part_id,
				 struct ffa_send_direct_data *msg, bool is_smc64)
{
	ffa_value_t res = {0};
	int ffa_errno;
	u64 req_mode, resp_mode;
	struct ffa_priv *uc_priv;

	uc_priv = dev_get_uclass_priv(dev);

	/* No partition installed */
	if (!uc_priv->partitions.count || !uc_priv->partitions.descs)
		return -ENODEV;

	if (is_smc64) {
		req_mode = FFA_SMC_64(FFA_MSG_SEND_DIRECT_REQ);
		resp_mode = FFA_SMC_64(FFA_MSG_SEND_DIRECT_RESP);
	} else {
		req_mode = FFA_SMC_32(FFA_MSG_SEND_DIRECT_REQ);
		resp_mode = FFA_SMC_32(FFA_MSG_SEND_DIRECT_RESP);
	}

	invoke_ffa_fn((ffa_value_t){
			.a0 = req_mode,
			.a1 = PREP_SELF_ENDPOINT_ID(uc_priv->id) |
				PREP_PART_ENDPOINT_ID(dst_part_id),
			.a2 = 0,
			.a3 = msg->data0,
			.a4 = msg->data1,
			.a5 = msg->data2,
			.a6 = msg->data3,
			.a7 = msg->data4,
			}, &res);

	while (res.a0 == FFA_SMC_32(FFA_INTERRUPT))
		invoke_ffa_fn((ffa_value_t){
			.a0 = FFA_SMC_32(FFA_RUN),
			.a1 = res.a1,
			}, &res);

	if (res.a0 == FFA_SMC_32(FFA_SUCCESS)) {
		/* Message sent with no response */
		return 0;
	}

	if (res.a0 == resp_mode) {
		/* Message sent with response extract the return data */
		msg->data0 = res.a3;
		msg->data1 = res.a4;
		msg->data2 = res.a5;
		msg->data3 = res.a6;
		msg->data4 = res.a7;

		return 0;
	}

	ffa_errno = res.a2;
	return ffa_to_std_errno(ffa_errno);
}

/* FF-A driver operations (used by clients for communicating with FF-A)*/

/**
 * ffa_partition_info_get() - FFA_PARTITION_INFO_GET driver operation
 *	@uuid_str: pointer to the UUID string
 *	@sp_count: address of the variable containing the number of partitions matching the UUID
 *			 The variable is set by the driver
 *	@sp_descs: address of the descriptors of the partitions matching the UUID
 *			 The address is set by the driver
 *
 * Driver operation for FFA_PARTITION_INFO_GET.
 * Please see ffa_get_partitions_info_hdlr() description for more details.
 *
 * Return:
 *
 * @sp_count: the number of partitions
 * @sp_descs: address of the partitions descriptors
 *
 * On success 0 is returned. Otherwise, failure
 */
int ffa_partition_info_get(struct udevice *dev, const char *uuid_str,
			   u32 *sp_count, struct ffa_partition_desc **sp_descs)
{
	struct ffa_bus_ops *ops = ffa_get_ops(dev);

	if (!ops->partition_info_get)
		return -ENOSYS;

	return ops->partition_info_get(dev, uuid_str, sp_count, sp_descs);
}

/**
 * ffa_sync_send_receive() - FFA_MSG_SEND_DIRECT_{REQ,RESP} driver operation
 * @dev: The FF-A bus device
 * @dst_part_id: destination partition ID
 * @msg: pointer to the message data preallocated by the client (in/out)
 * @is_smc64: select 64-bit or 32-bit FF-A ABI
 *
 * Driver operation for FFA_MSG_SEND_DIRECT_{REQ,RESP}.
 * Please see ffa_msg_send_direct_req_hdlr() description for more details.
 *
 * Return:
 *
 * 0 on success. Otherwise, failure
 */
int ffa_sync_send_receive(struct udevice *dev, u16 dst_part_id,
			  struct ffa_send_direct_data *msg, bool is_smc64)
{
	struct ffa_bus_ops *ops = ffa_get_ops(dev);

	if (!ops->sync_send_receive)
		return -ENOSYS;

	return ops->sync_send_receive(dev, dst_part_id, msg, is_smc64);
}

/**
 * ffa_rxtx_unmap() - FFA_RXTX_UNMAP driver operation
 * @dev: The FF-A bus device
 *
 * Driver operation for FFA_RXTX_UNMAP.
 * Please see ffa_unmap_rxtx_buffers_hdlr() description for more details.
 *
 * Return:
 *
 * 0 on success. Otherwise, failure
 */
int ffa_rxtx_unmap(struct udevice *dev)
{
	struct ffa_bus_ops *ops = ffa_get_ops(dev);

	if (!ops->rxtx_unmap)
		return -ENOSYS;

	return ops->rxtx_unmap(dev);
}

/**
 * ffa_do_probe() - probing FF-A framework
 * @dev:	the FF-A bus device (arm_ffa)
 *
 * Probing is triggered on demand by clients searching for the uclass.
 * At probe level the following actions are done:
 *	- saving the FF-A framework version in uc_priv
 *	- querying from secure world the u-boot endpoint ID
 *	- querying from secure world the supported features of FFA_RXTX_MAP
 *	- mapping the RX/TX buffers
 *	- querying from secure world all the partitions information
 *
 * All data queried from secure world is saved in uc_priv.
 *
 * Return:
 *
 * 0 on success. Otherwise, failure
 */
static int ffa_do_probe(struct udevice *dev)
{
	int ret;

	ret = ffa_get_version_hdlr(dev);
	if (ret)
		return ret;

	ret = ffa_get_endpoint_id(dev);
	if (ret)
		return ret;

	ret = ffa_get_rxtx_map_features_hdlr(dev);
	if (ret)
		return ret;

	ret = ffa_map_rxtx_buffers_hdlr(dev);
	if (ret)
		return ret;

	ret = ffa_cache_partitions_info(dev);
	if (ret) {
		ffa_unmap_rxtx_buffers_hdlr(dev);
		return ret;
	}

	return 0;
}

UCLASS_DRIVER(ffa) = {
	.name			= "ffa",
	.id			= UCLASS_FFA,
	.pre_probe		= ffa_do_probe,
	.pre_remove		= ffa_unmap_rxtx_buffers_hdlr,
	.per_device_auto	= sizeof(struct ffa_priv)
};
