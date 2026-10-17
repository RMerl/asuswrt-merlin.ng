/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2017 Intel Corporation.
 * Copyright 2019 Google LLC
 *
 * Modified from coreboot gpio.h
 */

#ifndef __ASM_INTEL_PINCTRL_H
#define __ASM_INTEL_PINCTRL_H

#include <dm/pinctrl.h>
#include <linux/bitops.h>

/**
 * struct pad_config - config for a pad
 * @pad: offset of pad within community
 * @pad_config: Pad config data corresponding to DW0, DW1, etc.
 */
struct pad_config {
	int pad;
	u32 pad_config[4];
};

#include <asm/arch/gpio.h>

/* GPIO community IOSF sideband clock gating */
#define MISCCFG_GPSIDEDPCGEN	BIT(5)
/* GPIO community RCOMP clock gating */
#define MISCCFG_GPRCOMPCDLCGEN	BIT(4)
/* GPIO community RTC clock gating */
#define MISCCFG_GPRTCDLCGEN	BIT(3)
/* GFX controller clock gating */
#define MISCCFG_GSXSLCGEN	BIT(2)
/* GPIO community partition clock gating */
#define MISCCFG_GPDPCGEN	BIT(1)
/* GPIO community local clock gating */
#define MISCCFG_GPDLCGEN	BIT(0)
/* Enable GPIO community power management configuration */
#define MISCCFG_ENABLE_GPIO_PM_CONFIG (MISCCFG_GPSIDEDPCGEN | \
	MISCCFG_GPRCOMPCDLCGEN | MISCCFG_GPRTCDLCGEN | MISCCFG_GSXSLCGEN \
	| MISCCFG_GPDPCGEN | MISCCFG_GPDLCGEN)

/*
 * GPIO numbers may not be contiguous and instead will have a different
 * starting pin number for each pad group.
 */
#define INTEL_GPP_BASE(first_of_community, start_of_group, end_of_group,\
			group_pad_base)					\
	{								\
		.first_pad = (start_of_group) - (first_of_community),	\
		.size = (end_of_group) - (start_of_group) + 1,		\
		.acpi_pad_base = (group_pad_base),			\
	}

/*
 * A pad base of -1 indicates that this group uses contiguous numbering
 * and a pad base should not be used for this group.
 */
#define PAD_BASE_NONE	-1

/* The common/default group numbering is contiguous */
#define INTEL_GPP(first_of_community, start_of_group, end_of_group)	\
	INTEL_GPP_BASE(first_of_community, start_of_group, end_of_group,\
		       PAD_BASE_NONE)

/**
 * struct reset_mapping - logical to actual value for PADRSTCFG in DW0
 *
 * Note that the values are expected to be within the field placement of the
 * register itself. i.e. if the reset field is at 31:30 then the values within
 * logical and chipset should occupy 31:30.
 */
struct reset_mapping {
	u32 logical;
	u32 chipset;
};

/**
 * struct pad_group - describes the groups within each community
 *
 * @first_pad: offset of first pad of the group relative to the community
 * @size: size of the group
 * @acpi_pad_base: starting pin number for the pads in this group when they are
 *	used in ACPI.  This is only needed if the pins are not contiguous across
 *	groups. Most groups will have this set to PAD_BASE_NONE and use
 *	contiguous numbering for ACPI.
 */
struct pad_group {
	int first_pad;
	uint size;
	int acpi_pad_base;
};

/**
 * struct pad_community - community of pads
 *
 * This describes a community, or each group within a community when multiple
 * groups exist inside a community
 *
 * @name: Community name
 * @num_gpi_regs: number of gpi registers in community
 * @max_pads_per_group: number of pads in each group; number of pads bit-mapped
 *	in each GPI status/en and Host Own Reg
 * @first_pad: first pad in community
 * @last_pad: last pad in community
 * @host_own_reg_0: offset to Host Ownership Reg 0
 * @gpi_int_sts_reg_0: offset to GPI Int STS Reg 0
 * @gpi_int_en_reg_0: offset to GPI Int Enable Reg 0
 * @gpi_smi_sts_reg_0: offset to GPI SMI STS Reg 0
 * @gpi_smi_en_reg_0: offset to GPI SMI EN Reg 0
 * @pad_cfg_base: offset to first PAD_GFG_DW0 Reg
 * @gpi_status_offset: specifies offset in struct gpi_status
 * @port: PCR Port ID
 * @reset_map: PADRSTCFG logical to chipset mapping
 * @num_reset_vals: number of values in @reset_map
 * @groups; list of groups for this community
 * @num_groups: number of groups
 */
struct pad_community {
	const char *name;
	size_t num_gpi_regs;
	size_t max_pads_per_group;
	uint first_pad;
	uint last_pad;
	u16 host_own_reg_0;
	u16 gpi_int_sts_reg_0;
	u16 gpi_int_en_reg_0;
	u16 gpi_smi_sts_reg_0;
	u16 gpi_smi_en_reg_0;
	u16 pad_cfg_base;
	u8 gpi_status_offset;
	u8 port;
	const struct reset_mapping *reset_map;
	size_t num_reset_vals;
	const struct pad_group *groups;
	size_t num_groups;
};

/**
 * struct intel_pinctrl_priv - private data for each pinctrl device
 *
 * @comm: Pad community for this device
 * @num_cfgs: Number of configuration words for each pad
 * @itss: ITSS device (for interrupt handling)
 * @itss_pol_cfg: Use to program Interrupt Polarity Control (IPCx) register
 *	Each bit represents IRQx Active High Polarity Disable configuration:
 *	when set to 1, the interrupt polarity associated with IRQx is inverted
 *	to appear as Active Low to IOAPIC and vice versa
 */
struct intel_pinctrl_priv {
	const struct pad_community *comm;
	int num_cfgs;
	struct udevice *itss;
	bool itss_pol_cfg;
};

/* Exported common operations for the pinctrl driver */
extern const struct pinctrl_ops intel_pinctrl_ops;

/* Exported common probe function for the pinctrl driver */
int intel_pinctrl_probe(struct udevice *dev);

/**
 * intel_pinctrl_of_to_plat() - Handle common plat setup
 *
 * @dev: Pinctrl device
 * @comm: Pad community for this device
 * @num_cfgs: Number of configuration words for each pad
 * Return: 0 if OK, -EDOM if @comm is NULL, other -ve value on other error
 */
int intel_pinctrl_of_to_plat(struct udevice *dev,
			     const struct pad_community *comm, int num_cfgs);

/**
 * pinctrl_route_gpe() - set GPIO groups for the general-purpose-event blocks
 *
 * The values from PMC register GPE_CFG are passed which is then mapped to
 * proper groups for MISCCFG. This basically sets the MISCCFG register bits:
 *  dw0 = gpe0_route[11:8]. This is ACPI GPE0b.
 *  dw1 = gpe0_route[15:12]. This is ACPI GPE0c.
 *  dw2 = gpe0_route[19:16]. This is ACPI GPE0d.
 *
 * @dev: ITSS device
 * @gpe0b: Value for GPE0B
 * @gpe0c: Value for GPE0C
 * @gpe0d: Value for GPE0D
 * Return: 0 if OK, -ve on error
 */
int pinctrl_route_gpe(struct udevice *dev, uint gpe0b, uint gpe0c, uint gpe0d);

/**
 * pinctrl_config_pads() - Configure a list of pads
 *
 * Configures multiple pads using the provided data from the device tree.
 *
 * @dev: pinctrl device (any will do)
 * @pads: Pad data, consisting of a pad number followed by num_cfgs entries
 *	containing the data for that pad (num_cfgs is set by the pinctrl device)
 * @pads_count: Number of pads to configure
 * Return: 0 if OK, -ve on error
 */
int pinctrl_config_pads(struct udevice *dev, u32 *pads, int pads_count);

/**
 * pinctrl_gpi_clear_int_cfg() - Set up the interrupts for use
 *
 * This enables the interrupt inputs and clears the status register bits
 *
 * Return: 0 if OK, -ve on error
 */
int pinctrl_gpi_clear_int_cfg(void);

/**
 * pinctrl_config_pads_for_node() - Configure pads
 *
 * Set up the pads using the data in a given node
 *
 * @dev: pinctrl device (any will do)
 * @node: Node containing the 'pads' property with the data in it
 * Return: 0 if OK, -ve on error
 */
int pinctrl_config_pads_for_node(struct udevice *dev, ofnode node);

/**
 * pinctrl_read_pads() - Read pad data from a node
 *
 * @dev: pinctrl device (any will do, it is just used to get config)
 * @node: Node to read pad data from
 * @prop: Property name to use (e.g. "pads")
 * @padsp: Returns a pointer to an allocated array of pad data, in the format:
 *	<pad>
 *	<pad_config0>
 *	<pad_config1>
 *	...
 *
 *	The number of pad config values is set by the pinctrl controller.
 *	The caller must free this array.
 * @pad_countp: Returns the number of pads read
 * @ereturn 0 if OK, -ve on error
 */
int pinctrl_read_pads(struct udevice *dev, ofnode node, const char *prop,
		      u32 **padsp, int *pad_countp);

/**
 * pinctrl_count_pads() - Count the number of pads in a pad array
 *
 * This used used with of-platdata where the array may be smaller than its
 * maximum size. This function searches for the last pad in the array by finding
 * the first 'zero' record
 *
 * This works out the number of records in the array. Each record has one word
 * for the pad and num_cfgs words for the config.
 *
 * @dev: pinctrl device (any will do)
 * @pads: Array of pad data
 * @size: Size of pad data in bytes
 * Return: number of pads represented by the data
 */
int pinctrl_count_pads(struct udevice *dev, u32 *pads, int size);

/**
 * intel_pinctrl_get_config_reg_offset() - Get offset of pin config registers
 *
 * This works out the register offset of a pin within the p2sb region.
 *
 * @dev: Pinctrl device
 * @offset: GPIO offset within this device
 * Return: register offset of first register within the GPIO p2sb region
 */
u32 intel_pinctrl_get_config_reg_offset(struct udevice *dev, uint offset);

/**
 * intel_pinctrl_get_config_reg_addr() - Get address of pin config registers
 *
 * This works out the absolute address of the registers for a pin
 * @dev: Pinctrl device
 * @offset: GPIO offset within this device
 * Return: register address of first register within the GPIO p2sb region
 */
u32 intel_pinctrl_get_config_reg_addr(struct udevice *dev, uint offset);

/**
 * intel_pinctrl_get_config_reg() - Get the value of a GPIO register
 *
 * @dev: Pinctrl device
 * @offset: GPIO offset within this device
 * Return: register value within the GPIO p2sb region
 */
u32 intel_pinctrl_get_config_reg(struct udevice *dev, uint offset);

/**
 * intel_pinctrl_get_pad() - Get pad information for a pad
 *
 * This is used by the GPIO controller to find the pinctrl used by a pad.
 *
 * @pad: Pad to check
 * @devp: Returns pinctrl device containing that pad
 * @offsetp: Returns offset of pad within that pinctrl device
 * Return: 0 if OK, -ENOTBLK if pad number is invalid
 */
int intel_pinctrl_get_pad(uint pad, struct udevice **devp, uint *offsetp);

/**
 * intel_pinctrl_get_acpi_pin() - Get the ACPI pin for a pinctrl pin
 *
 * Maps a pinctrl pin (in terms of its offset within the pins controlled by that
 * pinctrl) to an ACPI GPIO pin-table entry.
 *
 * @dev: Pinctrl device to check
 * @offset: Offset of pin within that device (0 = first)
 * Return: associated ACPI GPIO pin-table entry, or standard pin number if the
 *	ACPI pad base is not set
 */
int intel_pinctrl_get_acpi_pin(struct udevice *dev, uint offset);

#endif
