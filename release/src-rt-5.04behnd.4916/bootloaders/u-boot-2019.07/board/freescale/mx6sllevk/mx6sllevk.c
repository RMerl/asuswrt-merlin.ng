// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Freescale Semiconductor, Inc.
 */

#include <asm/arch/clock.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/io.h>
#include <common.h>
#include <linux/sizes.h>
#include <mmc.h>
#include <power/pmic.h>
#include <power/pfuze100_pmic.h>
#include "../common/pfuze.h"

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL  (PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |		\
	PAD_CTL_DSE_40ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();

	return 0;
}

static iomux_v3_cfg_t const uart1_pads[] = {
	MX6_PAD_UART1_TXD__UART1_DCE_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_UART1_RXD__UART1_DCE_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static iomux_v3_cfg_t const wdog_pads[] = {
	MX6_PAD_WDOG_B__WDOG1_B | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static void setup_iomux_uart(void)
{
	imx_iomux_v3_setup_multiple_pads(uart1_pads, ARRAY_SIZE(uart1_pads));
}

#ifdef CONFIG_DM_PMIC_PFUZE100
int power_init_board(void)
{
	struct udevice *dev;
	int ret;
	u32 dev_id, rev_id, i;
	u32 switch_num = 6;
	u32 offset = PFUZE100_SW1CMODE;

	ret = pmic_get("pfuze100", &dev);
	if (ret == -ENODEV)
		return 0;

	if (ret != 0)
		return ret;

	dev_id = pmic_reg_read(dev, PFUZE100_DEVICEID);
	rev_id = pmic_reg_read(dev, PFUZE100_REVID);
	printf("PMIC: PFUZE100! DEV_ID=0x%x REV_ID=0x%x\n", dev_id, rev_id);


	/* Init mode to APS_PFM */
	pmic_reg_write(dev, PFUZE100_SW1ABMODE, APS_PFM);

	for (i = 0; i < switch_num - 1; i++)
		pmic_reg_write(dev, offset + i * SWITCH_SIZE, APS_PFM);

	/* set SW1AB staby volatage 0.975V */
	pmic_clrsetbits(dev, PFUZE100_SW1ABSTBY, 0x3f, 0x1b);

	/* set SW1AB/VDDARM step ramp up time from 16us to 4us/25mV */
	pmic_clrsetbits(dev, PFUZE100_SW1ABCONF, 0xc0, 0x40);

	/* set SW1C staby volatage 0.975V */
	pmic_clrsetbits(dev, PFUZE100_SW1CSTBY, 0x3f, 0x1b);

	/* set SW1C/VDDSOC step ramp up time to from 16us to 4us/25mV */
	pmic_clrsetbits(dev, PFUZE100_SW1CCONF, 0xc0, 0x40);

	return 0;
}
#endif

int board_early_init_f(void)
{
	setup_iomux_uart();

	return 0;
}

int board_init(void)
{
	/* Address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	return 0;
}

int board_late_init(void)
{
	imx_iomux_v3_setup_multiple_pads(wdog_pads, ARRAY_SIZE(wdog_pads));

	return 0;
}

int checkboard(void)
{
	puts("Board: MX6SLL EVK\n");

	return 0;
}

int board_mmc_get_env_dev(int devno)
{
	return devno;
}

int mmc_map_to_kernel_blk(int devno)
{
	return devno;
}
