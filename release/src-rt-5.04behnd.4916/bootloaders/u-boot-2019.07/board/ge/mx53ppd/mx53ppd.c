// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2017 General Electric Company
 *
 * Based on board/freescale/mx53loco/mx53loco.c:
 *
 * Copyright (C) 2011 Freescale Semiconductor, Inc.
 * Jason Liu <r64343@freescale.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/clock.h>
#include <asm/arch/iomux-mx53.h>
#include <asm/arch/clock.h>
#include <linux/errno.h>
#include <linux/libfdt.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/mach-imx/mx5_video.h>
#include <environment.h>
#include <netdev.h>
#include <i2c.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <asm/gpio.h>
#include <power/pmic.h>
#include <dialog_pmic.h>
#include <fsl_pmic.h>
#include <linux/fb.h>
#include <ipu_pixfmt.h>
#include <version.h>
#include <watchdog.h>
#include "ppd_gpio.h"
#include <stdlib.h>
#include "../../ge/common/ge_common.h"
#include "../../ge/common/vpd_reader.h"

#define MX53PPD_LCD_POWER		IMX_GPIO_NR(3, 24)

DECLARE_GLOBAL_DATA_PTR;

static u32 mx53_dram_size[2];

phys_size_t get_effective_memsize(void)
{
	/*
	 * WARNING: We must override get_effective_memsize() function here
	 * to report only the size of the first DRAM bank. This is to make
	 * U-Boot relocator place U-Boot into valid memory, that is, at the
	 * end of the first DRAM bank. If we did not override this function
	 * like so, U-Boot would be placed at the address of the first DRAM
	 * bank + total DRAM size - sizeof(uboot), which in the setup where
	 * each DRAM bank contains 512MiB of DRAM would result in placing
	 * U-Boot into invalid memory area close to the end of the first
	 * DRAM bank.
	 */
	return mx53_dram_size[0];
}

int dram_init(void)
{
	mx53_dram_size[0] = get_ram_size((void *)PHYS_SDRAM_1, 1 << 30);
	mx53_dram_size[1] = get_ram_size((void *)PHYS_SDRAM_2, 1 << 30);

	gd->ram_size = mx53_dram_size[0] + mx53_dram_size[1];

	return 0;
}

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = mx53_dram_size[0];

	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size = mx53_dram_size[1];

	return 0;
}

u32 get_board_rev(void)
{
	return get_cpu_rev() & ~(0xF << 8);
}

#define UART_PAD_CTRL	(PAD_CTL_HYS | PAD_CTL_DSE_HIGH | \
			 PAD_CTL_PUS_100K_UP | PAD_CTL_ODE)

#ifdef CONFIG_USB_EHCI_MX5
int board_ehci_hcd_init(int port)
{
	/* request VBUS power enable pin, GPIO7_8 */
	imx_iomux_v3_setup_pad(MX53_PAD_PATA_DA_2__GPIO7_8);
	gpio_direction_output(IMX_GPIO_NR(7, 8), 1);
	return 0;
}
#endif

static void setup_iomux_fec(void)
{
	static const iomux_v3_cfg_t fec_pads[] = {
		NEW_PAD_CTRL(MX53_PAD_FEC_MDIO__FEC_MDIO, PAD_CTL_HYS |
			     PAD_CTL_DSE_HIGH | PAD_CTL_PUS_22K_UP |
			     PAD_CTL_ODE),
		NEW_PAD_CTRL(MX53_PAD_FEC_MDC__FEC_MDC, PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_FEC_RXD1__FEC_RDATA_1,
			     PAD_CTL_HYS | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_FEC_RXD0__FEC_RDATA_0,
			     PAD_CTL_HYS | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_FEC_TXD1__FEC_TDATA_1, PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_FEC_TXD0__FEC_TDATA_0, PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_FEC_TX_EN__FEC_TX_EN, PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_FEC_REF_CLK__FEC_TX_CLK,
			     PAD_CTL_HYS | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_FEC_RX_ER__FEC_RX_ER,
			     PAD_CTL_HYS | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_FEC_CRS_DV__FEC_RX_DV,
			     PAD_CTL_HYS | PAD_CTL_PKE),
	};

	imx_iomux_v3_setup_multiple_pads(fec_pads, ARRAY_SIZE(fec_pads));
}

#define I2C_PAD_CTRL	(PAD_CTL_SRE_FAST | PAD_CTL_DSE_HIGH | \
			 PAD_CTL_PUS_100K_UP | PAD_CTL_ODE)

static void setup_iomux_i2c(void)
{
	static const iomux_v3_cfg_t i2c1_pads[] = {
		NEW_PAD_CTRL(MX53_PAD_CSI0_DAT8__I2C1_SDA, I2C_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_CSI0_DAT9__I2C1_SCL, I2C_PAD_CTRL),
	};

	imx_iomux_v3_setup_multiple_pads(i2c1_pads, ARRAY_SIZE(i2c1_pads));
}

#define I2C_PAD MUX_PAD_CTRL(I2C_PAD_CTRL)

static struct i2c_pads_info i2c_pad_info1 = {
	.scl = {
		.i2c_mode = MX53_PAD_EIM_D21__I2C1_SCL | I2C_PAD,
		.gpio_mode = MX53_PAD_EIM_D28__GPIO3_28 | I2C_PAD,
		.gp = IMX_GPIO_NR(3, 28)
	},
	.sda = {
		.i2c_mode = MX53_PAD_EIM_D28__I2C1_SDA | I2C_PAD,
		.gpio_mode = MX53_PAD_EIM_D21__GPIO3_21 | I2C_PAD,
		.gp = IMX_GPIO_NR(3, 21)
	}
};

static int clock_1GHz(void)
{
	int ret;
	u32 ref_clk = MXC_HCLK;
	/*
	 * After increasing voltage to 1.25V, we can switch
	 * CPU clock to 1GHz and DDR to 400MHz safely
	 */
	ret = mxc_set_clock(ref_clk, 1000, MXC_ARM_CLK);
	if (ret) {
		printf("CPU:   Switch CPU clock to 1GHZ failed\n");
		return -1;
	}

	ret = mxc_set_clock(ref_clk, 400, MXC_PERIPH_CLK);
	ret |= mxc_set_clock(ref_clk, 400, MXC_DDR_CLK);
	if (ret) {
		printf("CPU:   Switch DDR clock to 400MHz failed\n");
		return -1;
	}

	return 0;
}

void ppd_gpio_init(void)
{
	int i;

	imx_iomux_v3_setup_multiple_pads(ppd_pads, ARRAY_SIZE(ppd_pads));
	for (i = 0; i < ARRAY_SIZE(ppd_gpios); ++i)
		gpio_direction_output(ppd_gpios[i].gpio, ppd_gpios[i].value);
}

int board_early_init_f(void)
{
	setup_iomux_fec();
	setup_iomux_lcd();
	ppd_gpio_init();

	return 0;
}

/*
 * Do not overwrite the console
 * Use always serial for U-Boot console
 */
int overwrite_console(void)
{
	return 1;
}

#define VPD_TYPE_INVALID 0x00
#define VPD_BLOCK_NETWORK 0x20
#define VPD_BLOCK_HWID 0x44
#define VPD_PRODUCT_PPD 4
#define VPD_HAS_MAC1 0x1
#define VPD_MAC_ADDRESS_LENGTH 6

struct vpd_cache {
	u8 product_id;
	u8 has;
	unsigned char mac1[VPD_MAC_ADDRESS_LENGTH];
};

/*
 * Extracts MAC and product information from the VPD.
 */
static int vpd_callback(struct vpd_cache *userdata, u8 id, u8 version,
			u8 type, size_t size, u8 const *data)
{
	struct vpd_cache *vpd = userdata;

	if (id == VPD_BLOCK_HWID && version == 1 && type != VPD_TYPE_INVALID &&
	    size >= 1) {
		vpd->product_id = data[0];

	} else if (id == VPD_BLOCK_NETWORK && version == 1 &&
		   type != VPD_TYPE_INVALID) {
		if (size >= 6) {
			vpd->has |= VPD_HAS_MAC1;
			memcpy(vpd->mac1, data, VPD_MAC_ADDRESS_LENGTH);
		}
	}

	return 0;
}

static void process_vpd(struct vpd_cache *vpd)
{
	int fec_index = -1;

	if (vpd->product_id == VPD_PRODUCT_PPD)
		fec_index = 0;

	if (fec_index >= 0 && (vpd->has & VPD_HAS_MAC1))
		eth_env_set_enetaddr("ethaddr", vpd->mac1);
}

int board_init(void)
{
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	mxc_set_sata_internal_clock();
	setup_iomux_i2c();

	setup_i2c(1, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info1);

	return 0;
}

int misc_init_r(void)
{
	const char *cause;

	/* We care about WDOG only, treating everything else as
	 * a power-on-reset.
	 */
	if (get_imx_reset_cause() & 0x0010)
		cause = "WDOG";
	else
		cause = "POR";

	env_set("bootcause", cause);

	return 0;
}

int board_late_init(void)
{
	int res;
	struct vpd_cache vpd;

	memset(&vpd, 0, sizeof(vpd));
	res = read_vpd(&vpd, vpd_callback);
	if (!res)
		process_vpd(&vpd);
	else
		printf("Can't read VPD");

	res = clock_1GHz();
	if (res != 0)
		return res;

	print_cpuinfo();
	hw_watchdog_init();

	check_time();

	return 0;
}

int checkboard(void)
{
	puts("Board: GE PPD\n");

	return 0;
}

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *blob, bd_t *bd)
{
	fdt_setprop(blob, 0, "ge,boot-ver", version_string,
	                                    strlen(version_string) + 1);
	return 0;
}
#endif
