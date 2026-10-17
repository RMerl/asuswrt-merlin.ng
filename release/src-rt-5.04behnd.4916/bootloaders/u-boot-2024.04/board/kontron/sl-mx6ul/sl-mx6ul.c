// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Kontron Electronics GmbH
 */

#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>
#include <env_internal.h>
#include <fdt_support.h>
#include <phy.h>
#include <sl-mx6ul-common.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();

	return 0;
}

int ft_board_setup(void *blob, struct bd_info *bd)
{
	/*
	 * Overwrite the memory size in the devicetree that is
	 * passed to the kernel with the actual size detected.
	 */
	return fdt_fixup_memory(blob, PHYS_SDRAM, gd->ram_size);
}

static int setup_fec(void)
{
	struct iomuxc *const iomuxc_regs = (struct iomuxc *)IOMUXC_BASE_ADDR;
	int ret;

	/*
	 * Use 50M anatop loopback REF_CLK1 for ENET1,
	 * clear gpr1[13], set gpr1[17].
	 */
	clrsetbits_le32(&iomuxc_regs->gpr[1], IOMUX_GPR1_FEC1_MASK,
			IOMUX_GPR1_FEC1_CLOCK_MUX1_SEL_MASK);

	/*
	 * Use 50M anatop loopback REF_CLK2 for ENET2,
	 * clear gpr1[14], set gpr1[18].
	 */
	clrsetbits_le32(&iomuxc_regs->gpr[1], IOMUX_GPR1_FEC2_MASK,
			IOMUX_GPR1_FEC2_CLOCK_MUX1_SEL_MASK);

	ret = enable_fec_anatop_clock(0, ENET_50MHZ);
	if (ret)
		return ret;

	ret = enable_fec_anatop_clock(1, ENET_50MHZ);
	if (ret)
		return ret;

	return 0;
}

int board_phy_config(struct phy_device *phydev)
{
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1f, 0x8190);

	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

int board_early_init_f(void)
{
	enable_qspi_clk(0);

	return 0;
}

int board_init(void)
{
	/* Address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	setup_fec();

	return 0;
}

enum env_location env_get_location(enum env_operation op, int prio)
{
	if (prio)
		return ENVL_UNKNOWN;

	if (sl_mx6ul_is_spi_nor_boot() && CONFIG_IS_ENABLED(ENV_IS_IN_SPI_FLASH))
		return ENVL_SPI_FLASH;
	else if (CONFIG_IS_ENABLED(ENV_IS_IN_MMC))
		return ENVL_MMC;

	return ENVL_NOWHERE;
}
