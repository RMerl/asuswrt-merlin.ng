// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2017 Intel Corp.
 * Copyright 2019 Google LLC
 *
 * Taken partly from coreboot gpio.c
 */

#define LOG_CATEGORY UCLASS_GPIO

#include <common.h>
#include <dm.h>
#include <dt-structs.h>
#include <log.h>
#include <p2sb.h>
#include <asm/intel_pinctrl.h>
#include <asm-generic/gpio.h>
#include <asm/intel_pinctrl_defs.h>

static const struct reset_mapping rst_map[] = {
	{ .logical = PAD_CFG0_LOGICAL_RESET_PWROK, .chipset = 0U << 30 },
	{ .logical = PAD_CFG0_LOGICAL_RESET_DEEP, .chipset = 1U << 30 },
	{ .logical = PAD_CFG0_LOGICAL_RESET_PLTRST, .chipset = 2U << 30 },
};

/* Groups for each community */
static const struct pad_group apl_community_n_groups[] = {
	INTEL_GPP(N_OFFSET, N_OFFSET, GPIO_31),		/* NORTH 0 */
	INTEL_GPP(N_OFFSET, GPIO_32, JTAG_TRST_B),	/* NORTH 1 */
	INTEL_GPP(N_OFFSET, JTAG_TMS, SVID0_CLK),	/* NORTH 2 */
};

static const struct pad_group apl_community_w_groups[] = {
	INTEL_GPP(W_OFFSET, W_OFFSET, OSC_CLK_OUT_1),	/* WEST 0 */
	INTEL_GPP(W_OFFSET, OSC_CLK_OUT_2, SUSPWRDNACK),/* WEST 1 */
};

static const struct pad_group apl_community_sw_groups[] = {
	INTEL_GPP(SW_OFFSET, SW_OFFSET, SMB_ALERTB),	/* SOUTHWEST 0 */
	INTEL_GPP(SW_OFFSET, SMB_CLK, LPC_FRAMEB),	/* SOUTHWEST 1 */
};

static const struct pad_group apl_community_nw_groups[] = {
	INTEL_GPP(NW_OFFSET, NW_OFFSET, PROCHOT_B),	/* NORTHWEST 0 */
	INTEL_GPP(NW_OFFSET, PMIC_I2C_SCL, GPIO_106),	/* NORTHWEST 1 */
	INTEL_GPP(NW_OFFSET, GPIO_109, GPIO_123),	/* NORTHWEST 2 */
};

/* TODO(sjg@chromium.org): Consider moving this to device tree */
static const struct pad_community apl_gpio_communities[] = {
	{
		.port = PID_GPIO_N,
		.first_pad = N_OFFSET,
		.last_pad = SVID0_CLK,
		.num_gpi_regs = NUM_N_GPI_REGS,
		.gpi_status_offset = NUM_NW_GPI_REGS + NUM_W_GPI_REGS
			+ NUM_SW_GPI_REGS,
		.pad_cfg_base = PAD_CFG_BASE,
		.host_own_reg_0 = HOSTSW_OWN_REG_0,
		.gpi_int_sts_reg_0 = GPI_INT_STS_0,
		.gpi_int_en_reg_0 = GPI_INT_EN_0,
		.gpi_smi_sts_reg_0 = GPI_SMI_STS_0,
		.gpi_smi_en_reg_0 = GPI_SMI_EN_0,
		.max_pads_per_group = GPIO_MAX_NUM_PER_GROUP,
		.name = "GPIO_GPE_N",
		.reset_map = rst_map,
		.num_reset_vals = ARRAY_SIZE(rst_map),
		.groups = apl_community_n_groups,
		.num_groups = ARRAY_SIZE(apl_community_n_groups),
	}, {
		.port = PID_GPIO_NW,
		.first_pad = NW_OFFSET,
		.last_pad = GPIO_123,
		.num_gpi_regs = NUM_NW_GPI_REGS,
		.gpi_status_offset = NUM_W_GPI_REGS + NUM_SW_GPI_REGS,
		.pad_cfg_base = PAD_CFG_BASE,
		.host_own_reg_0 = HOSTSW_OWN_REG_0,
		.gpi_int_sts_reg_0 = GPI_INT_STS_0,
		.gpi_int_en_reg_0 = GPI_INT_EN_0,
		.gpi_smi_sts_reg_0 = GPI_SMI_STS_0,
		.gpi_smi_en_reg_0 = GPI_SMI_EN_0,
		.max_pads_per_group = GPIO_MAX_NUM_PER_GROUP,
		.name = "GPIO_GPE_NW",
		.reset_map = rst_map,
		.num_reset_vals = ARRAY_SIZE(rst_map),
		.groups = apl_community_nw_groups,
		.num_groups = ARRAY_SIZE(apl_community_nw_groups),
	}, {
		.port = PID_GPIO_W,
		.first_pad = W_OFFSET,
		.last_pad = SUSPWRDNACK,
		.num_gpi_regs = NUM_W_GPI_REGS,
		.gpi_status_offset = NUM_SW_GPI_REGS,
		.pad_cfg_base = PAD_CFG_BASE,
		.host_own_reg_0 = HOSTSW_OWN_REG_0,
		.gpi_int_sts_reg_0 = GPI_INT_STS_0,
		.gpi_int_en_reg_0 = GPI_INT_EN_0,
		.gpi_smi_sts_reg_0 = GPI_SMI_STS_0,
		.gpi_smi_en_reg_0 = GPI_SMI_EN_0,
		.max_pads_per_group = GPIO_MAX_NUM_PER_GROUP,
		.name = "GPIO_GPE_W",
		.reset_map = rst_map,
		.num_reset_vals = ARRAY_SIZE(rst_map),
		.groups = apl_community_w_groups,
		.num_groups = ARRAY_SIZE(apl_community_w_groups),
	}, {
		.port = PID_GPIO_SW,
		.first_pad = SW_OFFSET,
		.last_pad = LPC_FRAMEB,
		.num_gpi_regs = NUM_SW_GPI_REGS,
		.gpi_status_offset = 0,
		.pad_cfg_base = PAD_CFG_BASE,
		.host_own_reg_0 = HOSTSW_OWN_REG_0,
		.gpi_int_sts_reg_0 = GPI_INT_STS_0,
		.gpi_int_en_reg_0 = GPI_INT_EN_0,
		.gpi_smi_sts_reg_0 = GPI_SMI_STS_0,
		.gpi_smi_en_reg_0 = GPI_SMI_EN_0,
		.max_pads_per_group = GPIO_MAX_NUM_PER_GROUP,
		.name = "GPIO_GPE_SW",
		.reset_map = rst_map,
		.num_reset_vals = ARRAY_SIZE(rst_map),
		.groups = apl_community_sw_groups,
		.num_groups = ARRAY_SIZE(apl_community_sw_groups),
	},
};

static int apl_pinctrl_of_to_plat(struct udevice *dev)
{
	struct p2sb_child_plat *pplat;
	const struct pad_community *comm = NULL;
	int i;

#if CONFIG_IS_ENABLED(OF_PLATDATA)
	struct apl_gpio_plat *plat = dev_get_plat(dev);
	int ret;

	/*
	 * It would be nice to do this in the bind() method, but with
	 * of-platdata binding happens in the order that DM finds things in the
	 * linker list (i.e. alphabetical order by driver name). So the GPIO
	 * device may well be bound before its parent (p2sb), and this call
	 * will fail if p2sb is not bound yet.
	 */
	ret = p2sb_set_port_id(dev, plat->dtplat.intel_p2sb_port_id);
	if (ret)
		return log_msg_ret("Could not set port id", ret);
#endif
	/* Attach this device to its community structure */
	pplat = dev_get_parent_plat(dev);
	for (i = 0; i < ARRAY_SIZE(apl_gpio_communities); i++) {
		if (apl_gpio_communities[i].port == pplat->pid)
			comm = &apl_gpio_communities[i];
	}

	return intel_pinctrl_of_to_plat(dev, comm, 2);
}

#if CONFIG_IS_ENABLED(OF_REAL)
static const struct udevice_id apl_gpio_ids[] = {
	{ .compatible = "intel,apl-pinctrl"},
	{ }
};
#endif

U_BOOT_DRIVER(intel_apl_pinctrl) = {
	.name		= "intel_apl_pinctrl",
	.id		= UCLASS_PINCTRL,
	.of_match	= of_match_ptr(apl_gpio_ids),
	.probe		= intel_pinctrl_probe,
	.ops		= &intel_pinctrl_ops,
#if CONFIG_IS_ENABLED(OF_REAL)
	.bind		= dm_scan_fdt_dev,
#endif
	.of_to_plat = apl_pinctrl_of_to_plat,
	.priv_auto	= sizeof(struct intel_pinctrl_priv),
	.plat_auto	= sizeof(struct apl_gpio_plat),
};
