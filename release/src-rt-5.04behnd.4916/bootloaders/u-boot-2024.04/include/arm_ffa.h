/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2022-2023 Arm Limited and/or its affiliates <open-source-office@arm.com>
 *
 * Authors:
 *   Abdellatif El Khlifi <abdellatif.elkhlifi@arm.com>
 */

#ifndef __ARM_FFA_H
#define __ARM_FFA_H

#include <linux/printk.h>

/*
 * This header is public. It can be used by clients to access
 * data structures and definitions they need
 */

/*
 * struct ffa_partition_info - Partition information descriptor
 * @id:	Partition ID
 * @exec_ctxt:	Execution context count
 * @properties:	Partition properties
 *
 * Data structure containing information about partitions instantiated in the system
 * This structure is filled with the data queried by FFA_PARTITION_INFO_GET
 */
struct ffa_partition_info {
	u16 id;
	u16 exec_ctxt;
/* partition supports receipt of direct requests */
#define FFA_PARTITION_DIRECT_RECV	BIT(0)
/* partition can send direct requests. */
#define FFA_PARTITION_DIRECT_SEND	BIT(1)
/* partition can send and receive indirect messages. */
#define FFA_PARTITION_INDIRECT_MSG	BIT(2)
	u32 properties;
};

/*
 * struct ffa_partition_uuid - 16 bytes UUID transmitted by FFA_PARTITION_INFO_GET
 * @a1-4:	32-bit words access to the UUID data
 *
 */
struct ffa_partition_uuid {
	u32 a1; /* w1 */
	u32 a2; /* w2 */
	u32 a3; /* w3 */
	u32 a4; /* w4 */
};

/**
 * struct ffa_partition_desc - the secure partition descriptor
 * @info:	partition information
 * @sp_uuid:	the secure partition UUID
 *
 * Each partition has its descriptor containing the partitions information and the UUID
 */
struct ffa_partition_desc {
	struct ffa_partition_info info;
	struct ffa_partition_uuid sp_uuid;
};

/*
 * struct ffa_send_direct_data - Data structure hosting the data
 *                                       used by FFA_MSG_SEND_DIRECT_{REQ,RESP}
 * @data0-4:	Data read/written from/to x3-x7 registers
 *
 * Data structure containing the data to be sent by FFA_MSG_SEND_DIRECT_REQ
 * or read from FFA_MSG_SEND_DIRECT_RESP
 */

/* For use with FFA_MSG_SEND_DIRECT_{REQ,RESP} which pass data via registers */
struct ffa_send_direct_data {
	ulong data0; /* w3/x3 */
	ulong data1; /* w4/x4 */
	ulong data2; /* w5/x5 */
	ulong data3; /* w6/x6 */
	ulong data4; /* w7/x7 */
};

struct udevice;

/**
 * struct ffa_bus_ops - Operations for FF-A
 * @partition_info_get:	callback for the FFA_PARTITION_INFO_GET
 * @sync_send_receive:	callback for the FFA_MSG_SEND_DIRECT_REQ
 * @rxtx_unmap:	callback for the FFA_RXTX_UNMAP
 *
 * The data structure providing all the operations supported by the driver.
 * This structure is EFI runtime resident.
 */
struct ffa_bus_ops {
	int (*partition_info_get)(struct udevice *dev, const char *uuid_str,
				  u32 *sp_count, struct ffa_partition_desc **sp_descs);
	int (*sync_send_receive)(struct udevice *dev, u16 dst_part_id,
				 struct ffa_send_direct_data *msg,
				 bool is_smc64);
	int (*rxtx_unmap)(struct udevice *dev);
};

#define ffa_get_ops(dev)        ((struct ffa_bus_ops *)(dev)->driver->ops)

/**
 * ffa_rxtx_unmap() - FFA_RXTX_UNMAP driver operation
 * Please see ffa_unmap_rxtx_buffers_hdlr() description for more details.
 */
int ffa_rxtx_unmap(struct udevice *dev);

/**
 * ffa_unmap_rxtx_buffers_hdlr() - FFA_RXTX_UNMAP handler function
 * @dev: The arm_ffa bus device
 *
 * This function implements FFA_RXTX_UNMAP FF-A function
 * to unmap the RX/TX buffers
 *
 * Return:
 *
 * 0 on success. Otherwise, failure
 */
int ffa_unmap_rxtx_buffers_hdlr(struct udevice *dev);

/**
 * ffa_sync_send_receive() - FFA_MSG_SEND_DIRECT_{REQ,RESP} driver operation
 * Please see ffa_msg_send_direct_req_hdlr() description for more details.
 */
int ffa_sync_send_receive(struct udevice *dev, u16 dst_part_id,
			  struct ffa_send_direct_data *msg, bool is_smc64);

/**
 * ffa_msg_send_direct_req_hdlr() - FFA_MSG_SEND_DIRECT_{REQ,RESP} handler function
 * @dev: The arm_ffa bus device
 * @dst_part_id: destination partition ID
 * @msg: pointer to the message data preallocated by the client (in/out)
 * @is_smc64: select 64-bit or 32-bit FF-A ABI
 *
 * This function implements FFA_MSG_SEND_DIRECT_{REQ,RESP}
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
				 struct ffa_send_direct_data *msg, bool is_smc64);

/**
 * ffa_partition_info_get() - FFA_PARTITION_INFO_GET driver operation
 * Please see ffa_get_partitions_info_hdlr() description for more details.
 */
int ffa_partition_info_get(struct udevice *dev, const char *uuid_str,
			   u32 *sp_count, struct ffa_partition_desc **sp_descs);

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
 * If not found, invoke FFA_PARTITION_INFO_GET
 * FF-A function to query the partition information from secure world.
 *
 * A client of the FF-A driver should know the UUID of the service it wants to
 * access. It should use the UUID to request the FF-A driver to provide the
 * partition(s) information of the service. The FF-A driver uses
 * PARTITION_INFO_GET to obtain this information. This is implemented through
 * ffa_get_partitions_info_hdlr() function.
 * A new FFA_PARTITION_INFO_GET call is issued (first one performed through
 * ffa_cache_partitions_info) allowing to retrieve the partition(s) information.
 * They are not saved (already done). We only update the UUID in the cached area.
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
				 u32 *sp_count, struct ffa_partition_desc **sp_descs);

struct ffa_priv;

/**
 * ffa_set_smc_conduit() - Set the SMC conduit
 * @dev: The FF-A bus device
 *
 * Selects the SMC conduit by setting the FF-A ABI invoke function.
 *
 * Return:
 *
 * 0 on success. Otherwise, failure
 */
int ffa_set_smc_conduit(struct udevice *dev);

#endif
