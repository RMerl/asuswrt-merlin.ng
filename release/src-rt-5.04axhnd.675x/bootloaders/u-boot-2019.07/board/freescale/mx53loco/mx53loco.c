// SPDX-License-Identifier: GPL-2.0+
/*
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
#include <asm/mach-imx/mx5_video.h>
#include <netdev.h>
#include <i2c.h>
#include <input.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <asm/gpio.h>
#include <power/pmic.h>
#include <dialog_pmic.h>
#include <fsl_pmic.h>
#include <linux/fb.h>
#include <ipu_pixfmt.h>

#define MX53LOCO_LCD_POWER		IMX_GPIO_NR(3, 24)

DECLARE_GLOBAL_DATA_PTR;

u32 get_board_rev(void)
{
	struct iim_regs *iim = (struct iim_regs *)IMX_IIM_BASE;
	struct fuse_bank *bank = &iim->bank[0];
	struct fuse_bank0_regs *fuse =
		(struct fuse_bank0_regs *)bank->fuse_regs;

	int rev = readl(&fuse->gp[6]);

	if (!i2c_probe(CONFIG_SYS_DIALOG_PMIC_I2C_ADDR))
		rev = 0;

	return (get_cpu_rev() & ~(0xF << 8)) | (rev & 0xF) << 8;
}

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
			PAD_CTL_DSE_HIGH | PAD_CTL_PUS_22K_UP | PAD_CTL_ODE),
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

#ifdef CONFIG_FSL_ESDHC
struct fsl_esdhc_cfg esdhc_cfg[2] = {
	{MMC_SDHC1_BASE_ADDR},
	{MMC_SDHC3_BASE_ADDR},
};

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret;

	imx_iomux_v3_setup_pad(MX53_PAD_EIM_DA11__GPIO3_11);
	gpio_direction_input(IMX_GPIO_NR(3, 11));
	imx_iomux_v3_setup_pad(MX53_PAD_EIM_DA13__GPIO3_13);
	gpio_direction_input(IMX_GPIO_NR(3, 13));

	if (cfg->esdhc_base == MMC_SDHC1_BASE_ADDR)
		ret = !gpio_get_value(IMX_GPIO_NR(3, 13));
	else
		ret = !gpio_get_value(IMX_GPIO_NR(3, 11));

	return ret;
}

#define SD_CMD_PAD_CTRL		(PAD_CTL_HYS | PAD_CTL_DSE_HIGH | \
				 PAD_CTL_PUS_100K_UP)
#define SD_PAD_CTRL		(PAD_CTL_HYS | PAD_CTL_PUS_47K_UP | \
				 PAD_CTL_DSE_HIGH)

int board_mmc_init(bd_t *bis)
{
	static const iomux_v3_cfg_t sd1_pads[] = {
		NEW_PAD_CTRL(MX53_PAD_SD1_CMD__ESDHC1_CMD, SD_CMD_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_SD1_CLK__ESDHC1_CLK, SD_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_SD1_DATA0__ESDHC1_DAT0, SD_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_SD1_DATA1__ESDHC1_DAT1, SD_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_SD1_DATA2__ESDHC1_DAT2, SD_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_SD1_DATA3__ESDHC1_DAT3, SD_PAD_CTRL),
		MX53_PAD_EIM_DA13__GPIO3_13,
	};

	static const iomux_v3_cfg_t sd2_pads[] = {
		NEW_PAD_CTRL(MX53_PAD_PATA_RESET_B__ESDHC3_CMD,
				SD_CMD_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_PATA_IORDY__ESDHC3_CLK, SD_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_PATA_DATA8__ESDHC3_DAT0, SD_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_PATA_DATA9__ESDHC3_DAT1, SD_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_PATA_DATA10__ESDHC3_DAT2, SD_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_PATA_DATA11__ESDHC3_DAT3, SD_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_PATA_DATA0__ESDHC3_DAT4, SD_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_PATA_DATA1__ESDHC3_DAT5, SD_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_PATA_DATA2__ESDHC3_DAT6, SD_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_PATA_DATA3__ESDHC3_DAT7, SD_PAD_CTRL),
		MX53_PAD_EIM_DA11__GPIO3_11,
	};

	u32 index;
	int ret;

	esdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
	esdhc_cfg[1].sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);

	for (index = 0; index < CONFIG_SYS_FSL_ESDHC_NUM; index++) {
		switch (index) {
		case 0:
			imx_iomux_v3_setup_multiple_pads(sd1_pads,
							 ARRAY_SIZE(sd1_pads));
			break;
		case 1:
			imx_iomux_v3_setup_multiple_pads(sd2_pads,
							 ARRAY_SIZE(sd2_pads));
			break;
		default:
			printf("Warning: you configured more ESDHC controller"
				"(%d) as supported by the board(2)\n",
				CONFIG_SYS_FSL_ESDHC_NUM);
			return -EINVAL;
		}
		ret = fsl_esdhc_initialize(bis, &esdhc_cfg[index]);
		if (ret)
			return ret;
	}

	return 0;
}
#endif

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

static int power_init(void)
{
	unsigned int val;
	int ret;
	struct pmic *p;

	if (!i2c_probe(CONFIG_SYS_DIALOG_PMIC_I2C_ADDR)) {
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

	if (!i2c_probe(CONFIG_SYS_FSL_PMIC_I2C_ADDR)) {
		ret = pmic_init(I2C_0);
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
	setup_iomux_fec();
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
	setup_iomux_i2c();

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
