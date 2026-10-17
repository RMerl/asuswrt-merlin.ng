/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Freescale Layerscape MC I/O wrapper
 * Data Path Buffer Pool API
 *  Contains initialization APIs and runtime control APIs for DPBP
 *
 * Copyright 2013-2016 Freescale Semiconductor, Inc.
 * Copyright 2017-2023 NXP
 */

#ifndef __FSL_DPBP_H
#define __FSL_DPBP_H

/* DPBP Version */
#define DPBP_VER_MAJOR				3
#define DPBP_VER_MINOR				3

/* Command IDs */
#define DPBP_CMDID_CLOSE				0x8001
#define DPBP_CMDID_OPEN					0x8041
#define DPBP_CMDID_CREATE				0x9041
#define DPBP_CMDID_DESTROY				0x9841
#define DPBP_CMDID_GET_API_VERSION			0xa041

#define DPBP_CMDID_ENABLE				0x0021
#define DPBP_CMDID_DISABLE				0x0031
#define DPBP_CMDID_GET_ATTR				0x0041
#define DPBP_CMDID_RESET				0x0051

#pragma pack(push, 1)

struct dpbp_cmd_open {
	__le32 dpbp_id;
};

struct dpbp_cmd_destroy {
	__le32 object_id;
};

struct dpbp_rsp_get_attributes {
	__le16 pad;
	__le16 bpid;
	__le32 id;
};

#pragma pack(pop)

struct fsl_mc_io;

int dpbp_open(struct fsl_mc_io *mc_io, u32 cmd_flags, int dpbp_id, u16 *token);

int dpbp_close(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token);

/**
 * struct dpbp_cfg - Structure representing DPBP configuration
 * @options:	place holder
 */
struct dpbp_cfg {
	u32 options;
};

int dpbp_create(struct fsl_mc_io *mc_io, u16 dprc_token, u32 cmd_flags,
		const struct dpbp_cfg *cfg, u32 *obj_id);

int dpbp_destroy(struct fsl_mc_io *mc_io, u16 dprc_token, u32 cmd_flags,
		 u32 obj_id);

int dpbp_enable(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token);

int dpbp_disable(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token);

int dpbp_reset(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token);

/**
 * struct dpbp_attr - Structure representing DPBP attributes
 * @id:		DPBP object ID
 * @version:	DPBP version
 * @bpid:	Hardware buffer pool ID; should be used as an argument in
 *		acquire/release operations on buffers
 */
struct dpbp_attr {
	u32 id;
	u16 bpid;
};

int dpbp_get_attributes(struct fsl_mc_io *mc_io, u32 cmd_flags, u16 token,
			struct dpbp_attr *attr);

int dpbp_get_api_version(struct fsl_mc_io *mc_io, u32 cmd_flags,
			 u16 *major_ver, u16 *minor_ver);

#endif /* __FSL_DPBP_H */
