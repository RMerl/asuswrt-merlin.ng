// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021 Google LLC
 */

#define LOG_CATEGORY	UCLASS_ETH

#include <common.h>
#include <dm.h>
#include <log.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/sromc.h>

enum {
	FDT_SROM_PMC,
	FDT_SROM_TACP,
	FDT_SROM_TAH,
	FDT_SROM_TCOH,
	FDT_SROM_TACC,
	FDT_SROM_TCOS,
	FDT_SROM_TACS,

	FDT_SROM_TIMING_COUNT,
};

static int exyno5_sromc_probe(struct udevice *dev)
{
	u32 timing[FDT_SROM_TIMING_COUNT]; /* timing parameters */
	u32 smc_bw_conf, smc_bc_conf;
	int bank;	/* srom bank number */
	int width;	/* bus width in bytes */
	int ret;

	if (!IS_ENABLED(CONFIG_SMC911X))
		return 0;

	bank = dev_read_s32_default(dev, "bank", 0);
	width = dev_read_s32_default(dev, "width", 2);

	/* Ethernet needs data bus width of 16 bits */
	if (width != 2) {
		log_debug("Unsupported bus width %d\n", width);
		return log_msg_ret("width", -EINVAL);
	}
	ret = dev_read_u32_array(dev, "srom-timing", timing,
				 FDT_SROM_TIMING_COUNT);
	if (ret)
		return log_msg_ret("sromc", -EINVAL);

	smc_bw_conf = SROMC_DATA16_WIDTH(bank) | SROMC_BYTE_ENABLE(bank);
	smc_bc_conf = SROMC_BC_TACS(timing[FDT_SROM_TACS])   |
			SROMC_BC_TCOS(timing[FDT_SROM_TCOS]) |
			SROMC_BC_TACC(timing[FDT_SROM_TACC]) |
			SROMC_BC_TCOH(timing[FDT_SROM_TCOH]) |
			SROMC_BC_TAH(timing[FDT_SROM_TAH])   |
			SROMC_BC_TACP(timing[FDT_SROM_TACP]) |
			SROMC_BC_PMC(timing[FDT_SROM_PMC]);

	/* Select and configure the SROMC bank */
	exynos_pinmux_config(PERIPH_ID_SROMC, bank);
	s5p_config_sromc(bank, smc_bw_conf, smc_bc_conf);

	return 0;
}

static const struct udevice_id exyno5_sromc_ids[] = {
	{ .compatible = "samsung,exynos5-sromc" },
	{}
};

U_BOOT_DRIVER(exyno5_sromc) = {
	.name		= "exyno5_sromc",
	.id		= UCLASS_SIMPLE_BUS,
	.of_match	= exyno5_sromc_ids,
	.probe		= exyno5_sromc_probe,
};
