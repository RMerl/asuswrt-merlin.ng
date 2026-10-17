/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright 2019 Google LLC
 * Copyright 2020 B&R Industrial Automation GmbH - http://www.br-automation.com
 */

#ifndef __ASM_ARCH_FSP_BINDINGS_H
#define __ASM_ARCH_FSP_BINDINGS_H

#include <asm/arch/fsp/fsp_m_upd.h>
#include <asm/arch/fsp/fsp_s_upd.h>

#define ARRAY_SIZE_OF_MEMBER(s, m) (ARRAY_SIZE((((s *)0)->m)))
#define SIZE_OF_MEMBER(s, m) (sizeof((((s *)0)->m)))

enum conf_type {
	FSP_UINT8,
	FSP_UINT16,
	FSP_UINT32,
	FSP_UINT64,
	FSP_STRING,
	FSP_LPDDR4_SWIZZLE,
};

/**
 * struct fsp_binding - Binding describing devicetree/FSP relationships
 * @offset:   Offset within the FSP config structure
 * @propname: Name of property to read
 * @type:     Type of the property to read
 * @count:    If the property is expected to be an array, this is the
 *            number of expected elements
 *            Set to 0 if the property is expected to be a scalar
 *
 * The struct fsp_binding is used to describe the relationship between
 * values stored in devicetree and where they are placed in the FSP
 * configuration structure.
 */
struct fsp_binding {
	size_t offset;
	char *propname;
	enum conf_type type;
	size_t count;
};

/*
 * LPDDR4 helper routines for configuring the memory UPD for LPDDR4 operation.
 * There are four physical LPDDR4 channels, each 32-bits wide. There are two
 * logical channels using two physical channels together to form a 64-bit
 * interface to memory for each logical channel.
 */

enum {
	LP4_PHYS_CH0A,
	LP4_PHYS_CH0B,
	LP4_PHYS_CH1A,
	LP4_PHYS_CH1B,

	LP4_NUM_PHYS_CHANNELS,
};

/*
 * The DQs within a physical channel can be bit-swizzled within each byte.
 * Within a channel the bytes can be swapped, but the DQs need to be routed
 * with the corresponding DQS (strobe).
 */
enum {
	LP4_DQS0,
	LP4_DQS1,
	LP4_DQS2,
	LP4_DQS3,

	LP4_NUM_BYTE_LANES,
	DQ_BITS_PER_DQS		= 8,
};

/* Provide bit swizzling per DQS and byte swapping within a channel */
struct lpddr4_chan_swizzle_cfg {
	u8 dqs[LP4_NUM_BYTE_LANES][DQ_BITS_PER_DQS];
};

struct lpddr4_swizzle_cfg {
	struct lpddr4_chan_swizzle_cfg phys[LP4_NUM_PHYS_CHANNELS];
};

/**
 * fsp_m_update_config_from_dtb() - Read FSP-M config from devicetree node
 * @node: Valid node reference to read property from
 * @cfg:  Pointer to FSP-M config structure
 * Return: 0 on success, -ve on error
 *
 * This function reads the configuration for FSP-M from the provided
 * devicetree node and saves it in the FSP-M configuration structure.
 * Configuration options that are not present in the devicetree are
 * left at their current value.
 */
int fsp_m_update_config_from_dtb(ofnode node, struct fsp_m_config *cfg);

/**
 * fsp_s_update_config_from_dtb() - Read FSP-S config from devicetree node
 * @node: Valid node reference to read property from
 * @cfg:  Pointer to FSP-S config structure
 * Return: 0 on success, -ve on error
 *
 * This function reads the configuration for FSP-S from the provided
 * devicetree node and saves it in the FSP-S configuration structure.
 * Configuration options that are not present in the devicetree are
 * left at their current value.
 */
int fsp_s_update_config_from_dtb(ofnode node, struct fsp_s_config *cfg);

#endif
