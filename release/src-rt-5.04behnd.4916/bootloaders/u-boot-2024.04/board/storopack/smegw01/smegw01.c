// SPDX-License-Identifier: GPL-2.0+
// Copyright (C) 2021 Fabio Estevam <festevam@denx.de>

#include <init.h>
#include <net.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/mx7-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>
#include <asm/gpio.h>
#include <asm/mach-imx/hab.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/io.h>
#include <common.h>
#include <env.h>
#include <env_internal.h>
#include <asm/arch/crm_regs.h>
#include <asm/setup.h>
#include <asm/bootm.h>
#include <mmc.h>

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL  (PAD_CTL_DSE_3P3V_49OHM | PAD_CTL_PUS_PU100KOHM | \
			PAD_CTL_HYS)

int dram_init(void)
{
	gd->ram_size = PHYS_SDRAM_SIZE;

	return 0;
}

static iomux_v3_cfg_t const wdog_pads[] = {
	MX7D_PAD_GPIO1_IO00__WDOG1_WDOG_B | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static iomux_v3_cfg_t const uart1_pads[] = {
	MX7D_PAD_UART1_TX_DATA__UART1_DCE_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX7D_PAD_UART1_RX_DATA__UART1_DCE_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static void setup_iomux_uart(void)
{
	imx_iomux_v3_setup_multiple_pads(uart1_pads, ARRAY_SIZE(uart1_pads));
};

static int setup_fec(void)
{
	struct iomuxc_gpr_base_regs *const iomuxc_gpr_regs =
			(struct iomuxc_gpr_base_regs *)IOMUXC_GPR_BASE_ADDR;
	int ret;

	/* Use 125M anatop REF_CLK1 for ENET1, clear gpr1[13], gpr1[17]*/
	clrsetbits_le32(&iomuxc_gpr_regs->gpr[1],
			(IOMUXC_GPR_GPR1_GPR_ENET1_TX_CLK_SEL_MASK |
			IOMUXC_GPR_GPR1_GPR_ENET1_CLK_DIR_MASK), 0);

	ret = set_clk_enet(ENET_125MHZ);
	if (ret)
		return ret;

	return 0;
}

int board_early_init_f(void)
{
	setup_iomux_uart();
	setup_fec();
	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	return 0;
}

int board_late_init(void)
{
	struct wdog_regs *wdog = (struct wdog_regs *)WDOG1_BASE_ADDR;
	unsigned char eth1addr[6];

	imx_iomux_v3_setup_multiple_pads(wdog_pads, ARRAY_SIZE(wdog_pads));

	set_wdog_reset(wdog);

	/*
	 * Do not assert internal WDOG_RESET_B_DEB(controlled by bit 4),
	 * since we use PMIC_PWRON to reset the board.
	 */
	clrsetbits_le16(&wdog->wcr, 0, 0x10);

	/* Get the second MAC address */
	imx_get_mac_from_fuse(1, eth1addr);
	if (!env_get("eth1addr") && is_valid_ethaddr(eth1addr))
		eth_env_set_enetaddr("eth1addr", eth1addr);

	return 0;
}

uint mmc_get_env_part(struct mmc *mmc)
{
	uint part = EXT_CSD_EXTRACT_BOOT_PART(mmc->part_config);

	if (part == 7)
		part = 0;
	return part;
}

