// SPDX-License-Identifier: GPL-2.0+
/*
 * CI20 setup code
 *
 * Copyright (c) 2013 Imagination Technologies
 * Author: Paul Burton <paul.burton@imgtec.com>
 */

#include <common.h>
#include <environment.h>
#include <net.h>
#include <netdev.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <mach/jz4780.h>
#include <mach/jz4780_dram.h>
#include <mach/jz4780_gpio.h>

struct ci20_otp {
	u32	serial_number;
	u32	date;
	u8	manufacturer[2];
	u8	mac[6];
} __packed;

static void ci20_mux_mmc(void)
{
	void __iomem *gpio_regs = (void __iomem *)GPIO_BASE;

	/* setup MSC1 pins */
	writel(0x30f00000, gpio_regs + GPIO_PXINTC(4));
	writel(0x30f00000, gpio_regs + GPIO_PXMASKC(4));
	writel(0x30f00000, gpio_regs + GPIO_PXPAT1C(4));
	writel(0x30f00000, gpio_regs + GPIO_PXPAT0C(4));
	writel(0x30f00000, gpio_regs + GPIO_PXPENC(4));
	jz4780_clk_ungate_mmc();
}

#ifndef CONFIG_SPL_BUILD

static void ci20_mux_eth(void)
{
	void __iomem *gpio_regs = (void __iomem *)GPIO_BASE;

#ifdef CONFIG_NAND
	/* setup pins (some already setup for NAND) */
	writel(0x04030000, gpio_regs + GPIO_PXINTC(0));
	writel(0x04030000, gpio_regs + GPIO_PXMASKC(0));
	writel(0x04030000, gpio_regs + GPIO_PXPAT1C(0));
	writel(0x04030000, gpio_regs + GPIO_PXPAT0C(0));
	writel(0x04030000, gpio_regs + GPIO_PXPENS(0));
#else
	/* setup pins (as above +NAND CS +RD/WE +SDx +SAx) */
	writel(0x0dff00ff, gpio_regs + GPIO_PXINTC(0));
	writel(0x0dff00ff, gpio_regs + GPIO_PXMASKC(0));
	writel(0x0dff00ff, gpio_regs + GPIO_PXPAT1C(0));
	writel(0x0dff00ff, gpio_regs + GPIO_PXPAT0C(0));
	writel(0x0dff00ff, gpio_regs + GPIO_PXPENS(0));
	writel(0x00000003, gpio_regs + GPIO_PXINTC(1));
	writel(0x00000003, gpio_regs + GPIO_PXMASKC(1));
	writel(0x00000003, gpio_regs + GPIO_PXPAT1C(1));
	writel(0x00000003, gpio_regs + GPIO_PXPAT0C(1));
	writel(0x00000003, gpio_regs + GPIO_PXPENS(1));
#endif
}

static void ci20_mux_jtag(void)
{
#ifdef CONFIG_JTAG
	void __iomem *gpio_regs = (void __iomem *)GPIO_BASE;

	/* enable JTAG */
	writel(3 << 30, gpio_regs + GPIO_PXINTC(0));
	writel(3 << 30, gpio_regs + GPIO_PXMASKC(0));
	writel(3 << 30, gpio_regs + GPIO_PXPAT1C(0));
	writel(3 << 30, gpio_regs + GPIO_PXPAT0C(0));
#endif
}

static void ci20_mux_nand(void)
{
	void __iomem *gpio_regs = (void __iomem *)GPIO_BASE;

	/* setup pins */
	writel(0x002c00ff, gpio_regs + GPIO_PXINTC(0));
	writel(0x002c00ff, gpio_regs + GPIO_PXMASKC(0));
	writel(0x002c00ff, gpio_regs + GPIO_PXPAT1C(0));
	writel(0x002c00ff, gpio_regs + GPIO_PXPAT0C(0));
	writel(0x002c00ff, gpio_regs + GPIO_PXPENS(0));
	writel(0x00000003, gpio_regs + GPIO_PXINTC(1));
	writel(0x00000003, gpio_regs + GPIO_PXMASKC(1));
	writel(0x00000003, gpio_regs + GPIO_PXPAT1C(1));
	writel(0x00000003, gpio_regs + GPIO_PXPAT0C(1));
	writel(0x00000003, gpio_regs + GPIO_PXPENS(1));

	/* FRB0_N */
	jz47xx_gpio_direction_input(JZ_GPIO(0, 20));
	writel(20, gpio_regs + GPIO_PXPENS(0));

	/* disable write protect */
	jz47xx_gpio_direction_output(JZ_GPIO(5, 22), 1);
}

static void ci20_mux_uart(void)
{
	void __iomem *gpio_regs = (void __iomem *)GPIO_BASE;

	/* UART0 */
	writel(0x9, gpio_regs + GPIO_PXINTC(5));
	writel(0x9, gpio_regs + GPIO_PXMASKC(5));
	writel(0x9, gpio_regs + GPIO_PXPAT1C(5));
	writel(0x9, gpio_regs + GPIO_PXPAT0C(5));
	writel(0x9, gpio_regs + GPIO_PXPENC(5));
	jz4780_clk_ungate_uart(0);

	/* UART 1 and 2 */
	jz4780_clk_ungate_uart(1);
	jz4780_clk_ungate_uart(2);

#ifndef CONFIG_JTAG
	/* UART3 */
	writel(1 << 12, gpio_regs + GPIO_PXINTC(3));
	writel(1 << 12, gpio_regs + GPIO_PXMASKS(3));
	writel(1 << 12, gpio_regs + GPIO_PXPAT1S(3));
	writel(1 << 12, gpio_regs + GPIO_PXPAT0C(3));
	writel(3 << 30, gpio_regs + GPIO_PXINTC(0));
	writel(3 << 30, gpio_regs + GPIO_PXMASKC(0));
	writel(3 << 30, gpio_regs + GPIO_PXPAT1C(0));
	writel(1 << 30, gpio_regs + GPIO_PXPAT0C(0));
	writel(1 << 31, gpio_regs + GPIO_PXPAT0S(0));
	jz4780_clk_ungate_uart(3);
#endif

	/* UART4 */
	writel(0x100400, gpio_regs + GPIO_PXINTC(2));
	writel(0x100400, gpio_regs + GPIO_PXMASKC(2));
	writel(0x100400, gpio_regs + GPIO_PXPAT1S(2));
	writel(0x100400, gpio_regs + GPIO_PXPAT0C(2));
	writel(0x100400, gpio_regs + GPIO_PXPENC(2));
	jz4780_clk_ungate_uart(4);
}

int board_early_init_f(void)
{
	ci20_mux_jtag();
	ci20_mux_uart();

	ci20_mux_eth();
	ci20_mux_mmc();
	ci20_mux_nand();

	/* SYS_POWER_IND high (LED blue, VBUS off) */
	jz47xx_gpio_direction_output(JZ_GPIO(5, 15), 0);

	/* LEDs off */
	jz47xx_gpio_direction_output(JZ_GPIO(2, 0), 0);
	jz47xx_gpio_direction_output(JZ_GPIO(2, 1), 0);
	jz47xx_gpio_direction_output(JZ_GPIO(2, 2), 0);
	jz47xx_gpio_direction_output(JZ_GPIO(2, 3), 0);

	return 0;
}

int misc_init_r(void)
{
	const u32 efuse_clk = jz4780_clk_get_efuse_clk();
	struct ci20_otp otp;
	char manufacturer[3];

	/* Read the board OTP data */
	jz4780_efuse_init(efuse_clk);
	jz4780_efuse_read(0x18, 16, (u8 *)&otp);

	/* Set MAC address */
	if (!is_valid_ethaddr(otp.mac)) {
		/* no MAC assigned, generate one from the unique chip ID */
		jz4780_efuse_read(0x8, 4, &otp.mac[0]);
		jz4780_efuse_read(0x12, 2, &otp.mac[4]);
		otp.mac[0] = (otp.mac[0] | 0x02) & ~0x01;
	}
	eth_env_set_enetaddr("ethaddr", otp.mac);

	/* Put other board information into the environment */
	env_set_ulong("serial#", otp.serial_number);
	env_set_ulong("board_date", otp.date);
	manufacturer[0] = otp.manufacturer[0];
	manufacturer[1] = otp.manufacturer[1];
	manufacturer[2] = 0;
	env_set("board_mfr", manufacturer);

	return 0;
}

#ifdef CONFIG_DRIVER_DM9000
int board_eth_init(bd_t *bis)
{
	/* Enable clock */
	jz4780_clk_ungate_ethernet();

	/* Enable power (PB25) */
	jz47xx_gpio_direction_output(JZ_GPIO(1, 25), 1);

	/* Reset (PF12) */
	mdelay(10);
	jz47xx_gpio_direction_output(JZ_GPIO(5, 12), 0);
	mdelay(10);
	jz47xx_gpio_direction_output(JZ_GPIO(5, 12), 1);
	mdelay(10);

	return dm9000_initialize(bis);
}
#endif /* CONFIG_DRIVER_DM9000 */
#endif

static u8 ci20_revision(void)
{
	void __iomem *gpio_regs = (void __iomem *)GPIO_BASE;
	int val;

	jz47xx_gpio_direction_input(JZ_GPIO(2, 18));
	jz47xx_gpio_direction_input(JZ_GPIO(2, 19));

	/* Enable pullups */
	writel(BIT(18) | BIT(19), gpio_regs + GPIO_PXPENC(2));

	/* Read PC18/19 for version */
	val = (!!jz47xx_gpio_get_value(JZ_GPIO(2, 18))) |
	     ((!!jz47xx_gpio_get_value(JZ_GPIO(2, 19))) << 1);

	if (val == 3)	/* Rev 1 boards had no pulldowns - giving 3 */
		return 1;
	if (val == 1)	/* Rev 2 boards pulldown port C bit 18 giving 1 */
		return 2;

	return 0;
}

int dram_init(void)
{
	gd->ram_size = sdram_size(0) + sdram_size(1);
	return 0;
}

/* U-Boot common routines */
int checkboard(void)
{
	printf("Board: Creator CI20 (rev.%d)\n", ci20_revision());
	return 0;
}

#ifdef CONFIG_SPL_BUILD

#if defined(CONFIG_SPL_MMC_SUPPORT)
int board_mmc_init(bd_t *bd)
{
	ci20_mux_mmc();
	return jz_mmc_init((void __iomem *)MSC0_BASE);
}
#endif

static const struct jz4780_ddr_config K4B2G0846Q_48_config = {
	.timing = {
		(4 << DDRC_TIMING1_TRTP_BIT) | (13 << DDRC_TIMING1_TWTR_BIT) |
		(6 << DDRC_TIMING1_TWR_BIT) | (5 << DDRC_TIMING1_TWL_BIT),

		(4 << DDRC_TIMING2_TCCD_BIT) | (15 << DDRC_TIMING2_TRAS_BIT) |
		(6 << DDRC_TIMING2_TRCD_BIT) | (6 << DDRC_TIMING2_TRL_BIT),

		(4 << DDRC_TIMING3_ONUM) | (7 << DDRC_TIMING3_TCKSRE_BIT) |
		(6 << DDRC_TIMING3_TRP_BIT) | (4 << DDRC_TIMING3_TRRD_BIT) |
		(21 << DDRC_TIMING3_TRC_BIT),

		(31 << DDRC_TIMING4_TRFC_BIT) | (1 << DDRC_TIMING4_TRWCOV_BIT) |
		(4 << DDRC_TIMING4_TCKE_BIT) | (9 << DDRC_TIMING4_TMINSR_BIT) |
		(8 << DDRC_TIMING4_TXP_BIT) | (3 << DDRC_TIMING4_TMRD_BIT),

		(8 << DDRC_TIMING5_TRTW_BIT) | (4 << DDRC_TIMING5_TRDLAT_BIT) |
		(4 << DDRC_TIMING5_TWDLAT_BIT),

		(25 << DDRC_TIMING6_TXSRD_BIT) | (12 << DDRC_TIMING6_TFAW_BIT) |
		(2 << DDRC_TIMING6_TCFGW_BIT) | (2 << DDRC_TIMING6_TCFGR_BIT),
	},

	/* PHY */
	/* Mode Register 0 */
	.mr0 = 0x420,
#ifdef SDRAM_DISABLE_DLL
	.mr1 = (DDR3_MR1_DIC_7 | DDR3_MR1_RTT_DIS | DDR3_MR1_DLL_DISABLE),
#else
	.mr1 = (DDR3_MR1_DIC_7 | DDR3_MR1_RTT_DIS),
#endif

	.ptr0 = 0x002000d4,
	.ptr1 = 0x02230d40,
	.ptr2 = 0x04013880,

	.dtpr0 = 0x2a8f6690,
	.dtpr1 = 0x00400860,
	.dtpr2 = 0x10042a00,

	.pullup = 0x0b,
	.pulldn = 0x0b,
};

static const struct jz4780_ddr_config H5TQ2G83CFR_48_config = {
	.timing = {
		(4 << DDRC_TIMING1_TRTP_BIT) | (13 << DDRC_TIMING1_TWTR_BIT) |
		(6 << DDRC_TIMING1_TWR_BIT) | (5 << DDRC_TIMING1_TWL_BIT),

		(4 << DDRC_TIMING2_TCCD_BIT) | (16 << DDRC_TIMING2_TRAS_BIT) |
		(6 << DDRC_TIMING2_TRCD_BIT) | (6 << DDRC_TIMING2_TRL_BIT),

		(4 << DDRC_TIMING3_ONUM) | (7 << DDRC_TIMING3_TCKSRE_BIT) |
		(6 << DDRC_TIMING3_TRP_BIT) | (4 << DDRC_TIMING3_TRRD_BIT) |
		(22 << DDRC_TIMING3_TRC_BIT),

		(42 << DDRC_TIMING4_TRFC_BIT) | (1 << DDRC_TIMING4_TRWCOV_BIT) |
		(4 << DDRC_TIMING4_TCKE_BIT) | (7 << DDRC_TIMING4_TMINSR_BIT) |
		(3 << DDRC_TIMING4_TXP_BIT) | (3 << DDRC_TIMING4_TMRD_BIT),

		(8 << DDRC_TIMING5_TRTW_BIT) | (4 << DDRC_TIMING5_TRDLAT_BIT) |
		(4 << DDRC_TIMING5_TWDLAT_BIT),

		(25 << DDRC_TIMING6_TXSRD_BIT) | (20 << DDRC_TIMING6_TFAW_BIT) |
		(2 << DDRC_TIMING6_TCFGW_BIT) | (2 << DDRC_TIMING6_TCFGR_BIT),
	},

	/* PHY */
	/* Mode Register 0 */
	.mr0 = 0x420,
#ifdef SDRAM_DISABLE_DLL
	.mr1 = (DDR3_MR1_DIC_7 | DDR3_MR1_RTT_DIS | DDR3_MR1_DLL_DISABLE),
#else
	.mr1 = (DDR3_MR1_DIC_7 | DDR3_MR1_RTT_DIS),
#endif

	.ptr0 = 0x002000d4,
	.ptr1 = 0x02d30d40,
	.ptr2 = 0x04013880,

	.dtpr0 = 0x2c906690,
	.dtpr1 = 0x005608a0,
	.dtpr2 = 0x10042a00,

	.pullup = 0x0e,
	.pulldn = 0x0e,
};

#if (CONFIG_SYS_MHZ != 1200)
#error No DDR configuration for CPU speed
#endif

const struct jz4780_ddr_config *jz4780_get_ddr_config(void)
{
	const int board_revision = ci20_revision();

	if (board_revision == 2)
		return &K4B2G0846Q_48_config;
	else /* Fall back to H5TQ2G83CFR RAM */
		return &H5TQ2G83CFR_48_config;
}
#endif
