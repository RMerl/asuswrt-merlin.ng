// SPDX-License-Identifier: GPL-2.0+
/*
 * Bluegiga APX4 Development Kit
 *
 * Copyright (C) 2012 Bluegiga Technologies Oy
 *
 * Authors:
 * Veli-Pekka Peltola <veli-pekka.peltola@bluegiga.com>
 * Lauri Hintsala <lauri.hintsala@bluegiga.com>
 *
 * Based on m28evk.c:
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 * on behalf of DENX Software Engineering GmbH
 */

#include <common.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/setup.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux-mx28.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <linux/mii.h>
#include <miiphy.h>
#include <netdev.h>
#include <errno.h>

DECLARE_GLOBAL_DATA_PTR;

/* Functions */
int board_early_init_f(void)
{
	/* IO0 clock at 480MHz */
	mxs_set_ioclk(MXC_IOCLK0, 480000);
	/* IO1 clock at 480MHz */
	mxs_set_ioclk(MXC_IOCLK1, 480000);

	/* SSP0 clock at 96MHz */
	mxs_set_sspclk(MXC_SSPCLK0, 96000, 0);

	return 0;
}

int dram_init(void)
{
	return mxs_dram_init();
}

int board_init(void)
{
	/* Adress of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	return 0;
}

#ifdef CONFIG_CMD_MMC
int board_mmc_init(bd_t *bis)
{
	return mxsmmc_initialize(bis, 0, NULL, NULL);
}
#endif


#ifdef CONFIG_CMD_NET

#define MII_PHY_CTRL2 0x1f
int fecmxc_mii_postcall(int phy)
{
	/* change PHY RMII clock to 50MHz */
	miiphy_write("FEC", 0, MII_PHY_CTRL2, 0x8180);

	return 0;
}

int board_eth_init(bd_t *bis)
{
	int ret;
	struct eth_device *dev;

	ret = cpu_eth_init(bis);
	if (ret) {
		printf("FEC MXS: Unable to init FEC clocks\n");
		return ret;
	}

	ret = fecmxc_initialize(bis);
	if (ret) {
		printf("FEC MXS: Unable to init FEC\n");
		return ret;
	}

	dev = eth_get_dev_by_name("FEC");
	if (!dev) {
		printf("FEC MXS: Unable to get FEC device entry\n");
		return -EINVAL;
	}

	ret = fecmxc_register_mii_postcall(dev, fecmxc_mii_postcall);
	if (ret) {
		printf("FEC MXS: Unable to register FEC MII postcall\n");
		return ret;
	}

	return ret;
}
#endif

#ifdef CONFIG_SERIAL_TAG
#define MXS_OCOTP_MAX_TIMEOUT 1000000
void get_board_serial(struct tag_serialnr *serialnr)
{
	struct mxs_ocotp_regs *ocotp_regs =
		(struct mxs_ocotp_regs *)MXS_OCOTP_BASE;

	serialnr->high = 0;
	serialnr->low = 0;

	writel(OCOTP_CTRL_RD_BANK_OPEN, &ocotp_regs->hw_ocotp_ctrl_set);

	if (mxs_wait_mask_clr(&ocotp_regs->hw_ocotp_ctrl_reg, OCOTP_CTRL_BUSY,
		MXS_OCOTP_MAX_TIMEOUT)) {
		printf("MXS: Can't get serial number from OCOTP\n");
		return;
	}

	serialnr->low = readl(&ocotp_regs->hw_ocotp_cust3);
}
#endif

#ifdef CONFIG_REVISION_TAG
u32 get_board_rev(void)
{
	if (env_get("revision#") != NULL)
		return simple_strtoul(env_get("revision#"), NULL, 10);
	return 0;
}
#endif
