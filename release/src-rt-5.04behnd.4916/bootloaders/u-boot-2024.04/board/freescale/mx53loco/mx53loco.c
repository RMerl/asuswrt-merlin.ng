// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011 Freescale Semiconductor, Inc.
 * Jason Liu <r64343@freescale.com>
 */

#include <common.h>
#include <init.h>
#include <log.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/clock.h>
#include <asm/arch/iomux-mx53.h>
#include <asm/arch/clock.h>
#include <env.h>
#include <linux/errno.h>
#include <asm/mach-imx/mx5_video.h>
#include <i2c.h>
#include <input.h>
#include <fsl_esdhc_imx.h>
#include <asm/gpio.h>
#include <power/pmic.h>
#include <dialog_pmic.h>
#include <fsl_pmic.h>
#include <linux/fb.h>
#include <ipu_pixfmt.h>
#include <dm/uclass.h>
#include <dm/device.h>

#define MX53LOCO_LCD_POWER		IMX_GPIO_NR(3, 24)

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_REVISION_TAG
u32 get_board_rev(void)
{
	struct iim_regs *iim = (struct iim_regs *)IMX_IIM_BASE;
	struct fuse_bank *bank = &iim->bank[0];
	struct fuse_bank0_regs *fuse =
		(struct fuse_bank0_regs *)bank->fuse_regs;
	struct udevice *bus;
	struct udevice *dev;

	int rev = readl(&fuse->gp[6]);

	ret = uclass_get_device_by_seq(UCLASS_I2C, 0, &bus);
	if (ret)
		return ret;

	if (!dm_i2c_probe(bus, CFG_SYS_DIALOG_PMIC_I2C_ADDR, 0, &dev))
		rev = 0;

	return (get_cpu_rev() & ~(0xF << 8)) | (rev & 0xF) << 8;
}
#endif

#define UART_PAD_CTRL	(PAD_CTL_HYS | PAD_CTL_DSE_HIGH | \
			 PAD_CTL_PUS_100K_UP | PAD_CTL_ODE)

static void setup_iomux_uart(void)
{
	static const iomux_v3_cfg_t uart_pads[] = {
		NEW_PAD_CTRL(MX53_PAD_CSI0_DAT11__UART1_RXD_MUX, UART_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_CSI0_DAT10__UART1_TXD_MUX, UART_PAD_CTRL),
	};

	imx_iomux_v3_setup_multiple_pads(uart_pads, ARRAY_SIZE(uart_pads));
}

static int power_init(void)
{
	unsigned int val;
	int ret;
	struct pmic *p;
	struct udevice *bus;
	struct udevice *dev;

	ret = uclass_get_device_by_seq(UCLASS_I2C, 0, &bus);
	if (ret)
		return ret;

	if (!dm_i2c_probe(bus, CFG_SYS_DIALOG_PMIC_I2C_ADDR, 0, &dev)) {
		ret = pmic_dialog_init(I2C_PMIC);
		if (ret)
			return ret;

		p = pmic_get("DIALOG_PMIC");
		if (!p)
			return -ENODEV;

		env_set("fdt_file", "imx53-qsb.dtb");

		/* Set VDDA to 1.25V */
		val = DA9052_BUCKCORE_BCOREEN | DA_BUCKCORE_VBCORE_1_250V;
		ret = pmic_reg_write(p, DA9053_BUCKCORE_REG, val);
		if (ret) {
			printf("Writing to BUCKCORE_REG failed: %d\n", ret);
			return ret;
		}

		pmic_reg_read(p, DA9053_SUPPLY_REG, &val);
		val |= DA9052_SUPPLY_VBCOREGO;
		ret = pmic_reg_write(p, DA9053_SUPPLY_REG, val);
		if (ret) {
			printf("Writing to SUPPLY_REG failed: %d\n", ret);
			return ret;
		}

		/* Set Vcc peripheral to 1.30V */
		ret = pmic_reg_write(p, DA9053_BUCKPRO_REG, 0x62);
		if (ret) {
			printf("Writing to BUCKPRO_REG failed: %d\n", ret);
			return ret;
		}

		ret = pmic_reg_write(p, DA9053_SUPPLY_REG, 0x62);
		if (ret) {
			printf("Writing to SUPPLY_REG failed: %d\n", ret);
			return ret;
		}

		return ret;
	}

	if (!dm_i2c_probe(bus, CFG_SYS_FSL_PMIC_I2C_ADDR, 0, &dev)) {
		ret = pmic_init(0);
		if (ret)
			return ret;

		p = pmic_get("FSL_PMIC");
		if (!p)
			return -ENODEV;

		env_set("fdt_file", "imx53-qsrb.dtb");

		/* Set VDDGP to 1.25V for 1GHz on SW1 */
		pmic_reg_read(p, REG_SW_0, &val);
		val = (val & ~SWx_VOLT_MASK_MC34708) | SWx_1_250V_MC34708;
		ret = pmic_reg_write(p, REG_SW_0, val);
		if (ret) {
			printf("Writing to REG_SW_0 failed: %d\n", ret);
			return ret;
		}

		/* Set VCC as 1.30V on SW2 */
		pmic_reg_read(p, REG_SW_1, &val);
		val = (val & ~SWx_VOLT_MASK_MC34708) | SWx_1_300V_MC34708;
		ret = pmic_reg_write(p, REG_SW_1, val);
		if (ret) {
			printf("Writing to REG_SW_1 failed: %d\n", ret);
			return ret;
		}

		/* Set global reset timer to 4s */
		pmic_reg_read(p, REG_POWER_CTL2, &val);
		val = (val & ~TIMER_MASK_MC34708) | TIMER_4S_MC34708;
		ret = pmic_reg_write(p, REG_POWER_CTL2, val);
		if (ret) {
			printf("Writing to REG_POWER_CTL2 failed: %d\n", ret);
			return ret;
		}

		/* Set VUSBSEL and VUSBEN for USB PHY supply*/
		pmic_reg_read(p, REG_MODE_0, &val);
		val |= (VUSBSEL_MC34708 | VUSBEN_MC34708);
		ret = pmic_reg_write(p, REG_MODE_0, val);
		if (ret) {
			printf("Writing to REG_MODE_0 failed: %d\n", ret);
			return ret;
		}

		/* Set SWBST to 5V in auto mode */
		val = SWBST_AUTO;
		ret = pmic_reg_write(p, SWBST_CTRL, val);
		if (ret) {
			printf("Writing to SWBST_CTRL failed: %d\n", ret);
			return ret;
		}

		return ret;
	}

	return -1;
}

static void clock_1GHz(void)
{
	int ret;
	u32 ref_clk = MXC_HCLK;
	/*
	 * After increasing voltage to 1.25V, we can switch
	 * CPU clock to 1GHz and DDR to 400MHz safely
	 */
	ret = mxc_set_clock(ref_clk, 1000, MXC_ARM_CLK);
	if (ret)
		printf("CPU:   Switch CPU clock to 1GHZ failed\n");

	ret = mxc_set_clock(ref_clk, 400, MXC_PERIPH_CLK);
	ret |= mxc_set_clock(ref_clk, 400, MXC_DDR_CLK);
	if (ret)
		printf("CPU:   Switch DDR clock to 400MHz failed\n");
}

int board_early_init_f(void)
{
	setup_iomux_uart();
	setup_iomux_lcd();

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

int board_init(void)
{
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	mxc_set_sata_internal_clock();

	return 0;
}

int board_late_init(void)
{
	if (!power_init())
		clock_1GHz();

	return 0;
}

int checkboard(void)
{
	puts("Board: MX53 LOCO\n");

	return 0;
}
