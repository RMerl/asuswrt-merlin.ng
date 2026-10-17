/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2022-2023 Arm Limited and/or its affiliates <open-source-office@arm.com>
 *
 * Authors:
 *   Abdellatif El Khlifi <abdellatif.elkhlifi@arm.com>
 */

#ifndef __SANDBOX_ARM_FFA_PRV_H
#define __SANDBOX_ARM_FFA_PRV_H

#include <arm_ffa_priv.h>

/* This header is exclusively used by the Sandbox FF-A driver and emulator */

/* Non-secure physical FF-A instance */
#define NS_PHYS_ENDPOINT_ID (0)

#define GET_NS_PHYS_ENDPOINT_ID_MASK		GENMASK(31, 16)
#define GET_NS_PHYS_ENDPOINT_ID(x)		\
			((u16)(FIELD_GET(GET_NS_PHYS_ENDPOINT_ID_MASK, (x))))

/* Helper macro for reading the destination partition ID */
#define GET_DST_SP_ID_MASK		GENMASK(15, 0)
#define GET_DST_SP_ID(x)		\
			((u16)(FIELD_GET(GET_DST_SP_ID_MASK, (x))))

/* Helper macro for setting the source partition ID */
#define PREP_SRC_SP_ID_MASK		GENMASK(31, 16)
#define PREP_SRC_SP_ID(x)		\
			(FIELD_PREP(PREP_SRC_SP_ID_MASK, (x)))

/* Helper macro for setting the destination endpoint ID */
#define PREP_NS_PHYS_ENDPOINT_ID_MASK		GENMASK(15, 0)
#define PREP_NS_PHYS_ENDPOINT_ID(x)		\
			(FIELD_PREP(PREP_NS_PHYS_ENDPOINT_ID_MASK, (x)))

/*  RX/TX buffers minimum size */
#define RXTX_BUFFERS_MIN_SIZE (RXTX_4K)
#define RXTX_BUFFERS_MIN_PAGES (1)

/* MBZ registers info */

/* x1-x7 MBZ */
#define FFA_X1X7_MBZ_CNT (7)
#define FFA_X1X7_MBZ_REG_START (&res->a1)

/* x4-x7 MBZ */
#define FFA_X4X7_MBZ_CNT (4)
#define FFA_X4X7_MBZ_REG_START (&res->a4)

/* x3-x7 MBZ */
#define FFA_X3X7_MBZ_CNT (5)
#define FFA_X3_MBZ_REG_START (&res->a3)

/* number of emulated FF-A secure partitions (SPs) */
#define SANDBOX_PARTITIONS_CNT (4)

/* Binary data of the emulated services UUIDs */

/* service 1  UUID binary data (little-endian format) */
#define SANDBOX_SERVICE1_UUID_A1	0xed32d533
#define SANDBOX_SERVICE1_UUID_A2	0x99e64209
#define SANDBOX_SERVICE1_UUID_A3	0x9cc02d72
#define SANDBOX_SERVICE1_UUID_A4	0xcdd998a7

/* service 2  UUID binary data (little-endian format) */
#define SANDBOX_SERVICE2_UUID_A1	0xed32d544
#define SANDBOX_SERVICE2_UUID_A2	0x99e64209
#define SANDBOX_SERVICE2_UUID_A3	0x9cc02d72
#define SANDBOX_SERVICE2_UUID_A4	0xcdd998a7

/**
 * struct ffa_rxtxpair_info - structure hosting the RX/TX buffers flags
 * @rxbuf_owned:	RX buffer ownership flag (the owner is non secure world)
 * @rxbuf_mapped:	RX buffer mapping flag
 * @txbuf_owned	TX buffer ownership flag
 * @txbuf_mapped:	TX buffer mapping flag
 * @rxtx_buf_size:	RX/TX buffers size
 *
 * Hosts the ownership/mapping flags of the RX/TX buffers
 * When a buffer is owned/mapped its corresponding flag is set to 1 otherwise 0.
 */
struct ffa_rxtxpair_info {
	u8 rxbuf_owned;
	u8 rxbuf_mapped;
	u8 txbuf_owned;
	u8 txbuf_mapped;
	u32 rxtx_buf_size;
};

/**
 * struct sandbox_ffa_emul - emulator data
 *
 * @fwk_version:	FF-A framework version
 * @id:	u-boot endpoint ID
 * @partitions:	The partitions descriptors structure
 * @pair:	The RX/TX buffers pair
 * @pair_info:	The RX/TX buffers pair flags and size
 * @test_ffa_data:	The data of the FF-A bus under test
 *
 * Hosts all the emulated secure world data.
 */
struct sandbox_ffa_emul {
	u32 fwk_version;
	u16 id;
	struct ffa_partitions partitions;
	struct ffa_rxtxpair pair;
	struct ffa_rxtxpair_info pair_info;
};

/**
 * ffa_emul_find() - Finds the FF-A emulator
 * @dev:	the sandbox FF-A device (sandbox-arm-ffa)
 * @emulp:	the FF-A emulator device (sandbox-ffa-emul)
 * Return:
 * 0 on success. Otherwise, failure
 */
int ffa_emul_find(struct udevice *dev, struct udevice **emulp);

#endif
