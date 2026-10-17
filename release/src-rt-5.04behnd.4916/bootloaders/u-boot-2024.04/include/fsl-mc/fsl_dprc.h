/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Freescale Layerscape MC I/O wrapper
 *
 * Copyright 2013-2016 Freescale Semiconductor, Inc.
 * Copyright 2017, 2023 NXP
 */
#ifndef _FSL_DPRC_H
#define _FSL_DPRC_H

/* DPRC Version */
#define DPRC_VER_MAJOR				6
#define DPRC_VER_MINOR				1

/* Command IDs */
#define DPRC_CMDID_CLOSE			0x8001
#define DPRC_CMDID_OPEN				0x8051

#define DPRC_CMDID_GET_API_VERSION              0xa051

#define DPRC_CMDID_CREATE_CONT			0x1511
#define DPRC_CMDID_DESTROY_CONT			0x1521
#define DPRC_CMDID_GET_CONT_ID			0x8301

#define DPRC_CMDID_CONNECT			0x1671
#define DPRC_CMDID_DISCONNECT			0x1681
#define DPRC_CMDID_GET_CONNECTION		0x16C1

#pragma pack(push, 1)
struct dprc_cmd_open {
	__le32 container_id;
};

struct dprc_cmd_create_container {
	__le32 options;
	__le32 icid;
	__le32 pad1;
	__le32 portal_id;
	u8 label[16];
};

struct dprc_rsp_create_container {
	__le64 pad0;
	__le32 child_container_id;
	__le32 pad1;
	__le64 child_portal_addr;
};

struct dprc_cmd_destroy_container {
	__le32 child_container_id;
};

struct dprc_cmd_connect {
	__le32 ep1_id;
	__le16 ep1_interface_id;
	__le16 pad0;

	__le32 ep2_id;
	__le16 ep2_interface_id;
	__le16 pad1;

	u8 ep1_type[16];

	__le32 max_rate;
	__le32 committed_rate;

	u8 ep2_type[16];
};

struct dprc_cmd_disconnect {
	__le32 id;
	__le32 interface_id;
	u8 type[16];
};

struct dprc_cmd_get_connection {
	__le32 ep1_id;
	__le16 ep1_interface_id;
	__le16 pad;

	u8 ep1_type[16];
};

struct dprc_rsp_get_connection {
	__le64 pad[3];
	__le32 ep2_id;
	__le16 ep2_interface_id;
	__le16 pad1;
	u8 ep2_type[16];
	__le32 state;
};

#pragma pack(pop)

/* Data Path Resource Container API
 * Contains DPRC API for managing and querying DPAA resources
 */

struct fsl_mc_io;

/**
 * Set this value as the icid value in dprc_cfg structure when creating a
 * container, in case the ICID is not selected by the user and should be
 * allocated by the DPRC from the pool of ICIDs.
 */
#define DPRC_GET_ICID_FROM_POOL			(u16)(~(0))

/**
 * Set this value as the portal_id value in dprc_cfg structure when creating a
 * container, in case the portal ID is not specifically selected by the
 * user and should be allocated by the DPRC from the pool of portal ids.
 */
#define DPRC_GET_PORTAL_ID_FROM_POOL	(int)(~(0))

int dprc_get_container_id(struct fsl_mc_io *mc_io, u32 cmd_flags, int *container_id);

int dprc_open(struct fsl_mc_io *mc_io, u32 cmd_flags, int container_id, u16 *token);

int dprc_close(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token);

/**
 * Container general options
 *
 * These options may be selected at container creation by the container creator
 * and can be retrieved using dprc_get_attributes()
 */

/* Spawn Policy Option allowed - Indicates that the new container is allowed
 * to spawn and have its own child containers.
 */
#define DPRC_CFG_OPT_SPAWN_ALLOWED		0x00000001

/* General Container allocation policy - Indicates that the new container is
 * allowed to allocate requested resources from its parent container; if not
 * set, the container is only allowed to use resources in its own pools; Note
 * that this is a container's global policy, but the parent container may
 * override it and set specific quota per resource type.
 */
#define DPRC_CFG_OPT_ALLOC_ALLOWED		0x00000002

/* Object initialization allowed - software context associated with this
 * container is allowed to invoke object initialization operations.
 */
#define DPRC_CFG_OPT_OBJ_CREATE_ALLOWED		0x00000004

/* Topology change allowed - software context associated with this
 * container is allowed to invoke topology operations, such as attach/detach
 * of network objects.
 */
#define DPRC_CFG_OPT_TOPOLOGY_CHANGES_ALLOWED	0x00000008


/* AIOP - Indicates that container belongs to AIOP. */
#define DPRC_CFG_OPT_AIOP			0x00000020

/* IRQ Config - Indicates that the container allowed to configure its IRQs.*/
#define DPRC_CFG_OPT_IRQ_CFG_ALLOWED		0x00000040

/**
 * struct dprc_cfg - Container configuration options
 * @icid: Container's ICID; if set to 'DPRC_GET_ICID_FROM_POOL', a free
 *		ICID value is allocated by the DPRC
 * @portal_id: Portal ID; if set to 'DPRC_GET_PORTAL_ID_FROM_POOL', a free
 *		portal ID is allocated by the DPRC
 * @options: Combination of 'DPRC_CFG_OPT_<X>' options
 * @label: Object's label
 */
struct dprc_cfg {
	u16 icid;
	int portal_id;
	uint64_t options;
	char label[16];
};

int dprc_create_container(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token,
			  struct dprc_cfg *cfg, int *child_container_id,
			  uint64_t *child_portal_offset);

int dprc_destroy_container(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token,
			   int child_container_id);

/**
 * struct dprc_connection_cfg - Connection configuration.
 *				Used for virtual connections only
 * @committed_rate: Committed rate (Mbits/s)
 * @max_rate: Maximum rate (Mbits/s)
 */
struct dprc_connection_cfg {
	u32 committed_rate;
	u32 max_rate;
};

/**
 * struct dprc_endpoint - Endpoint description for link connect/disconnect
 *			operations
 * @type:	Endpoint object type: NULL terminated string
 * @id:		Endpoint object ID
 * @if_id:	Interface ID; should be set for endpoints with multiple
 *		interfaces ("dpsw", "dpdmux"); for others, always set to 0
 */
struct dprc_endpoint {
	char type[16];
	int id;
	u16 if_id;
};

int dprc_connect(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token,
		 const struct dprc_endpoint *endpoint1,
		 const struct dprc_endpoint *endpoint2,
		 const struct dprc_connection_cfg *cfg);

int dprc_disconnect(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token,
		    const struct dprc_endpoint *endpoint);

int dprc_get_connection(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token,
			const struct dprc_endpoint *endpoint1,
			struct dprc_endpoint *endpoint2, int *state);

int dprc_get_api_version(struct fsl_mc_io *mc_io, u32 cmd_flags,
			 u16 *major_ver, u16 *minor_ver);

#endif /* _FSL_DPRC_H */
