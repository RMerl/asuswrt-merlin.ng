// SPDX-License-Identifier: GPL-2.0+
/*
 * TI SATA platform driver
 *
 * (C) Copyright 2013
 * Texas Instruments, <www.ti.com>
 */

#include <common.h>
#include <ahci.h>
#include <scsi.h>
#include <asm/arch/clock.h>
#include <asm/arch/sata.h>
#include <sata.h>
#include <asm/io.h>
#include "pipe3-phy.h"

static struct pipe3_dpll_map dpll_map_sata[] = {
	{12000000, {1000, 7, 4, 6, 0} },        /* 12 MHz */
	{16800000, {714, 7, 4, 6, 0} },         /* 16.8 MHz */
	{19200000, {625, 7, 4, 6, 0} },         /* 19.2 MHz */
	{20000000, {600, 7, 4, 6, 0} },         /* 20 MHz */
	{26000000, {461, 7, 4, 6, 0} },         /* 26 MHz */
	{38400000, {312, 7, 4, 6, 0} },         /* 38.4 MHz */
	{ },                                    /* Terminator */
};

struct omap_pipe3 sata_phy = {
	.pll_ctrl_base = (void __iomem *)TI_SATA_PLLCTRL_BASE,
	/* .power_reg is updated at runtime */
	.dpll_map = dpll_map_sata,
};

int init_sata(int dev)
{
	int ret;
	u32 val;

	sata_phy.power_reg = (void __iomem *)(*ctrl)->control_phy_power_sata;

	/* Power up the PHY */
	phy_pipe3_power_on(&sata_phy);

	/* Enable SATA module, No Idle, No Standby */
	val = TI_SATA_IDLE_NO | TI_SATA_STANDBY_NO;
	writel(val, TI_SATA_WRAPPER_BASE + TI_SATA_SYSCONFIG);

	ret = ahci_init((void __iomem *)DWC_AHSATA_BASE);

	return ret;
}

int reset_sata(int dev)
{
	return 0;
}

/* On OMAP platforms SATA provides the SCSI subsystem */
void scsi_init(void)
{
	init_sata(0);
	scsi_scan(1);
}

int scsi_bus_reset(struct udevice *dev)
{
	ahci_reset((void __iomem *)DWC_AHSATA_BASE);
	ahci_init((void __iomem *)DWC_AHSATA_BASE);

	return 0;
}
