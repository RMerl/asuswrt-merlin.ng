// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021 Ronetix GmbH
 */

#include <init.h>
#include <net.h>
#include <asm/arch/clock.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/mx7-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>
#include <asm/gpio.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/io.h>
#include <common.h>
#include <i2c.h>
#include <miiphy.h>
#include <power/pmic.h>
#include <power/pfuze3000_pmic.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();
	return 0;
}

int power_init_board(void)
{
	struct udevice *dev;
	int ret;
	unsigned int reg, rev;

	ret = pmic_get("pmic@8", &dev);
	if (ret == -ENODEV) {
		puts("No pmic\n");
		return 0;
	}
	if (ret != 0)
		return ret;

	reg = pmic_reg_read(dev, PFUZE3000_DEVICEID);
	rev = pmic_reg_read(dev, PFUZE3000_REVID);
	printf("PMIC:  PFUZE3000 DEV_ID=0x%x REV_ID=0x%x\n", reg, rev);

	/* disable Low Power Mode during standby mode */
	reg = pmic_reg_read(dev, PFUZE3000_LDOGCTL);
	reg |= 0x1;
	pmic_reg_write(dev, PFUZE3000_LDOGCTL, reg);

	/* SW1A/1B mode set to APS/APS */
	reg = 0x8;
	pmic_reg_write(dev, PFUZE3000_SW1AMODE, reg);
	pmic_reg_write(dev, PFUZE3000_SW1BMODE, reg);

	/* SW1A/1B standby voltage set to 1.025V */
	reg = 0xd;
	pmic_reg_write(dev, PFUZE3000_SW1ASTBY, reg);
	pmic_reg_write(dev, PFUZE3000_SW1BSTBY, reg);

	/* decrease SW1B normal voltage to 0.975V */
	reg = pmic_reg_read(dev, PFUZE3000_SW1BVOLT);
	reg &= ~0x1f;
	reg |= PFUZE3000_SW1AB_SETP(975);
	pmic_reg_write(dev, PFUZE3000_SW1BVOLT, reg);

	return 0;
}

static int setup_fec(void)
{
	return set_clk_enet(ENET_125MHZ);
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	setup_fec();

	return 0;
}

int board_late_init(void)
{
	return 0;
}

int checkboard(void)
{
	puts("Board: iMX7-CM\n");
	return 0;
}
