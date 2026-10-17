/* SPDX-License-Identifier: GPL-2.0+ */
/* Copyright 2013-2016 Freescale Semiconductor, Inc.
 * Copyright 2017, 2023 NXP
 */
#ifndef __FSL_DPMNG_CMD_H
#define __FSL_DPMNG_CMD_H

/* Command IDs */
#define DPMNG_CMDID_GET_VERSION			0x8311

#pragma pack(push, 1)
struct dpmng_rsp_get_version {
	__le32 revision;
	__le32 version_major;
	__le32 version_minor;
};

#pragma pack(pop)

#endif /* __FSL_DPMNG_CMD_H */
