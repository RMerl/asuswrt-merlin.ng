// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2007, Guennadi Liakhovetski <lg@denx.de>
 *
 * (C) Copyright 2008-2010 Freescale Semiconductor, Inc.
 *
 * Copyright (C) 2011, Stefano Babic <sbabic@denx.de>
 */

#include <common.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/iomux-mx35.h>
#include <i2c.h>
#include <linux/types.h>
#include <asm/gpio.h>
#include <asm/arch/sys_proto.h>
#include <netdev.h>
#include <fdt_support.h>
#include <mtd_node.h>
#include <jffs2/load_kernel.h>

#ifndef CONFIG_BOARD_EARLY_INIT_F
#error "CONFIG_BOARD_EARLY_INIT_F must be set for this board"
#endif

#define CCM_CCMR_CONFIG		0x003F4208

#define ESDCTL_DDR2_CONFIG	0x007FFC3F

static inline void dram_wait(unsigned int count)
{
	volatile unsigned int wait = count;

	while (wait--)
		;
}

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	gd->ram_size = get_ram_size((long *)PHYS_SDRAM_1,
		PHYS_SDRAM_1_SIZE);

	return 0;
}

static void board_setup_sdram(void)
{
	struct esdc_regs *esdc = (struct esdc_regs *)ESDCTL_BASE_ADDR;

	/* Initialize with default values both CSD0/1 */
	writel(0x2000, &esdc->esdctl0);
	writel(0x2000, &esdc->esdctl1);


	mx3_setup_sdram_bank(CSD0_BASE_ADDR, ESDCTL_DDR2_CONFIG,
			     13, 10, 2, 0x8080);
}

static void setup_iomux_uart3(void)
{
	static const iomux_v3_cfg_t uart3_pads[] = {
		MX35_PAD_RTS2__UART3_RXD_MUX,
		MX35_PAD_CTS2__UART3_TXD_MUX,
	};

	imx_iomux_v3_setup_multiple_pads(uart3_pads, ARRAY_SIZE(uart3_pads));
}

#define I2C_PAD_CTRL	(PAD_CTL_HYS | PAD_CTL_PUS_100K_DOWN | PAD_CTL_ODE)

static void setup_iomux_i2c(void)
{
	static const iomux_v3_cfg_t i2c_pads[] = {
		NEW_PAD_CTRL(MX35_PAD_I2C1_CLK__I2C1_SCL, I2C_PAD_CTRL),
		NEW_PAD_CTRL(MX35_PAD_I2C1_DAT__I2C1_SDA, I2C_PAD_CTRL),

		NEW_PAD_CTRL(MX35_PAD_TX3_RX2__I2C3_SCL, I2C_PAD_CTRL),
		NEW_PAD_CTRL(MX35_PAD_TX2_RX3__I2C3_SDA, I2C_PAD_CTRL),
	};

	imx_iomux_v3_setup_multiple_pads(i2c_pads, ARRAY_SIZE(i2c_pads));
}


static void setup_iomux_spi(void)
{
	static const iomux_v3_cfg_t spi_pads[] = {
		MX35_PAD_CSPI1_MOSI__CSPI1_MOSI,
		MX35_PAD_CSPI1_MISO__CSPI1_MISO,
		MX35_PAD_CSPI1_SS0__CSPI1_SS0,
		MX35_PAD_CSPI1_SS1__CSPI1_SS1,
		MX35_PAD_CSPI1_SCLK__CSPI1_SCLK,
	};

	imx_iomux_v3_setup_multiple_pads(spi_pads, ARRAY_SIZE(spi_pads));
}

static void setup_iomux_fec(void)
{
	static const iomux_v3_cfg_t fec_pads[] = {
		MX35_PAD_FEC_TX_CLK__FEC_TX_CLK,
		MX35_PAD_FEC_RX_CLK__FEC_RX_CLK,
		MX35_PAD_FEC_RX_DV__FEC_RX_DV,
		MX35_PAD_FEC_COL__FEC_COL,
		MX35_PAD_FEC_RDATA0__FEC_RDATA_0,
		MX35_PAD_FEC_TDATA0__FEC_TDATA_0,
		MX35_PAD_FEC_TX_EN__FEC_TX_EN,
		MX35_PAD_FEC_MDC__FEC_MDC,
		MX35_PAD_FEC_MDIO__FEC_MDIO,
		MX35_PAD_FEC_TX_ERR__FEC_TX_ERR,
		MX35_PAD_FEC_RX_ERR__FEC_RX_ERR,
		MX35_PAD_FEC_CRS__FEC_CRS,
		MX35_PAD_FEC_RDATA1__FEC_RDATA_1,
		MX35_PAD_FEC_TDATA1__FEC_TDATA_1,
		MX35_PAD_FEC_RDATA2__FEC_RDATA_2,
		MX35_PAD_FEC_TDATA2__FEC_TDATA_2,
		MX35_PAD_FEC_RDATA3__FEC_RDATA_3,
		MX35_PAD_FEC_TDATA3__FEC_TDATA_3,
		/* GPIO used to power off ethernet */
		MX35_PAD_STXFS4__GPIO2_31,
	};

	/* setup pins for FEC */
	imx_iomux_v3_setup_multiple_pads(fec_pads, ARRAY_SIZE(fec_pads));
}

int board_early_init_f(void)
{
	struct ccm_regs *ccm =
		(struct ccm_regs *)IMX_CCM_BASE;

	/* setup GPIO3_1 to set HighVCore signal */
	imx_iomux_v3_setup_pad(MX35_PAD_ATA_DA1__GPIO3_1);
	gpio_direction_output(65, 1);

	/* initialize PLL and clock configuration */
	writel(CCM_CCMR_CONFIG, &ccm->ccmr);

	writel(CCM_MPLL_532_HZ, &ccm->mpctl);
	writel(CCM_PPLL_300_HZ, &ccm->ppctl);

	/* Set the core to run at 532 Mhz */
	writel(0x00001000, &ccm->pdr0);

	/* Set-up RAM */
	board_setup_sdram();

	/* enable clocks */
	writel(readl(&ccm->cgr0) |
		MXC_CCM_CGR0_EMI_MASK |
		MXC_CCM_CGR0_EDIO_MASK |
		MXC_CCM_CGR0_EPIT1_MASK,
		&ccm->cgr0);

	writel(readl(&ccm->cgr1) |
		MXC_CCM_CGR1_FEC_MASK |
		MXC_CCM_CGR1_GPIO1_MASK |
		MXC_CCM_CGR1_GPIO2_MASK |
		MXC_CCM_CGR1_GPIO3_MASK |
		MXC_CCM_CGR1_I2C1_MASK |
		MXC_CCM_CGR1_I2C2_MASK |
		MXC_CCM_CGR1_I2C3_MASK,
		&ccm->cgr1);

	/* Set-up NAND */
	__raw_writel(readl(&ccm->rcsr) | MXC_CCM_RCSR_NFC_FMS, &ccm->rcsr);

	/* Set pinmux for the required peripherals */
	setup_iomux_uart3();
	setup_iomux_i2c();
	setup_iomux_fec();
	setup_iomux_spi();

	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	/* Enable power for ethernet */
	gpio_direction_output(63, 0);

	udelay(2000);

	return 0;
}

u32 get_board_rev(void)
{
	int rev = 0;

	return (get_cpu_rev() & ~(0xF << 8)) | (rev & 0xF) << 8;
}

/*
 * called prior to booting kernel or by 'fdt boardsetup' command
 *
 */
int ft_board_setup(void *blob, bd_t *bd)
{
	static const struct node_info nodes[] = {
		{ "physmap-flash.0", MTD_DEV_TYPE_NOR, },  /* NOR flash */
		{ "mxc_nand", MTD_DEV_TYPE_NAND, }, /* NAND flash */
	};

	if (env_get("fdt_noauto")) {
		puts("   Skiping ft_board_setup (fdt_noauto defined)\n");
		return 0;
	}

	fdt_fixup_mtdparts(blob, nodes, ARRAY_SIZE(nodes));

	return 0;
}
