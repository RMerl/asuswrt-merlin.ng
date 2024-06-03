// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2007, Guennadi Liakhovetski <lg@denx.de>
 *
 * (C) Copyright 2008-2010 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/clock.h>
#include <asm/arch/iomux-mx35.h>
#include <i2c.h>
#include <power/pmic.h>
#include <fsl_pmic.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <mc9sdz60.h>
#include <mc13892.h>
#include <linux/types.h>
#include <asm/gpio.h>
#include <asm/arch/sys_proto.h>
#include <netdev.h>
#include <asm/mach-types.h>

#ifndef CONFIG_BOARD_LATE_INIT
#error "CONFIG_BOARD_LATE_INIT must be set for this board"
#endif

#ifndef CONFIG_BOARD_EARLY_INIT_F
#error "CONFIG_BOARD_EARLY_INIT_F must be set for this board"
#endif

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	u32 size1, size2;

	size1 = get_ram_size((void *)PHYS_SDRAM_1, PHYS_SDRAM_1_SIZE);
	size2 = get_ram_size((void *)PHYS_SDRAM_2, PHYS_SDRAM_2_SIZE);

	gd->ram_size = size1 + size2;

	return 0;
}

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size = PHYS_SDRAM_2_SIZE;

	return 0;
}

#define I2C_PAD_CTRL	(PAD_CTL_HYS | PAD_CTL_PUS_100K_DOWN | PAD_CTL_ODE)

static void setup_iomux_i2c(void)
{
	static const iomux_v3_cfg_t i2c1_pads[] = {
		NEW_PAD_CTRL(MX35_PAD_I2C1_CLK__I2C1_SCL, I2C_PAD_CTRL),
		NEW_PAD_CTRL(MX35_PAD_I2C1_DAT__I2C1_SDA, I2C_PAD_CTRL),
	};

	/* setup pins for I2C1 */
	imx_iomux_v3_setup_multiple_pads(i2c1_pads, ARRAY_SIZE(i2c1_pads));
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

#define USBOTG_IN_PAD_CTRL	(PAD_CTL_HYS | PAD_CTL_PUS_100K_DOWN | \
				 PAD_CTL_DSE_LOW | PAD_CTL_SRE_SLOW)
#define USBOTG_OUT_PAD_CTRL	(PAD_CTL_DSE_LOW | PAD_CTL_SRE_SLOW)

static void setup_iomux_usbotg(void)
{
	static const iomux_v3_cfg_t usbotg_pads[] = {
		NEW_PAD_CTRL(MX35_PAD_USBOTG_PWR__USB_TOP_USBOTG_PWR,
				USBOTG_OUT_PAD_CTRL),
		NEW_PAD_CTRL(MX35_PAD_USBOTG_OC__USB_TOP_USBOTG_OC,
				USBOTG_IN_PAD_CTRL),
	};

	/* Set up pins for USBOTG. */
	imx_iomux_v3_setup_multiple_pads(usbotg_pads, ARRAY_SIZE(usbotg_pads));
}

#define FEC_PAD_CTRL	(PAD_CTL_DSE_LOW | PAD_CTL_SRE_SLOW)

static void setup_iomux_fec(void)
{
	static const iomux_v3_cfg_t fec_pads[] = {
		NEW_PAD_CTRL(MX35_PAD_FEC_TX_CLK__FEC_TX_CLK, FEC_PAD_CTRL |
					PAD_CTL_HYS | PAD_CTL_PUS_100K_DOWN),
		NEW_PAD_CTRL(MX35_PAD_FEC_RX_CLK__FEC_RX_CLK, FEC_PAD_CTRL |
					PAD_CTL_HYS | PAD_CTL_PUS_100K_DOWN),
		NEW_PAD_CTRL(MX35_PAD_FEC_RX_DV__FEC_RX_DV, FEC_PAD_CTRL |
					PAD_CTL_HYS | PAD_CTL_PUS_100K_DOWN),
		NEW_PAD_CTRL(MX35_PAD_FEC_COL__FEC_COL, FEC_PAD_CTRL |
					PAD_CTL_HYS | PAD_CTL_PUS_100K_DOWN),
		NEW_PAD_CTRL(MX35_PAD_FEC_RDATA0__FEC_RDATA_0, FEC_PAD_CTRL |
					PAD_CTL_HYS | PAD_CTL_PUS_100K_DOWN),
		NEW_PAD_CTRL(MX35_PAD_FEC_TDATA0__FEC_TDATA_0, FEC_PAD_CTRL),
		NEW_PAD_CTRL(MX35_PAD_FEC_TX_EN__FEC_TX_EN, FEC_PAD_CTRL),
		NEW_PAD_CTRL(MX35_PAD_FEC_MDC__FEC_MDC, FEC_PAD_CTRL),
		NEW_PAD_CTRL(MX35_PAD_FEC_MDIO__FEC_MDIO, FEC_PAD_CTRL |
					PAD_CTL_HYS | PAD_CTL_PUS_22K_UP),
		NEW_PAD_CTRL(MX35_PAD_FEC_TX_ERR__FEC_TX_ERR, FEC_PAD_CTRL),
		NEW_PAD_CTRL(MX35_PAD_FEC_RX_ERR__FEC_RX_ERR, FEC_PAD_CTRL |
					PAD_CTL_HYS | PAD_CTL_PUS_100K_DOWN),
		NEW_PAD_CTRL(MX35_PAD_FEC_CRS__FEC_CRS, FEC_PAD_CTRL |
					PAD_CTL_HYS | PAD_CTL_PUS_100K_DOWN),
		NEW_PAD_CTRL(MX35_PAD_FEC_RDATA1__FEC_RDATA_1, FEC_PAD_CTRL |
					PAD_CTL_HYS | PAD_CTL_PUS_100K_DOWN),
		NEW_PAD_CTRL(MX35_PAD_FEC_TDATA1__FEC_TDATA_1, FEC_PAD_CTRL),
		NEW_PAD_CTRL(MX35_PAD_FEC_RDATA2__FEC_RDATA_2, FEC_PAD_CTRL |
					PAD_CTL_HYS | PAD_CTL_PUS_100K_DOWN),
		NEW_PAD_CTRL(MX35_PAD_FEC_TDATA2__FEC_TDATA_2, FEC_PAD_CTRL),
		NEW_PAD_CTRL(MX35_PAD_FEC_RDATA3__FEC_RDATA_3, FEC_PAD_CTRL |
					PAD_CTL_HYS | PAD_CTL_PUS_100K_DOWN),
		NEW_PAD_CTRL(MX35_PAD_FEC_TDATA3__FEC_TDATA_3, FEC_PAD_CTRL),
	};

	/* setup pins for FEC */
	imx_iomux_v3_setup_multiple_pads(fec_pads, ARRAY_SIZE(fec_pads));
}

int board_early_init_f(void)
{
	struct ccm_regs *ccm =
		(struct ccm_regs *)IMX_CCM_BASE;

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
		MXC_CCM_CGR1_IPU_MASK,
		&ccm->cgr1);

	/* Setup NAND */
	__raw_writel(readl(&ccm->rcsr) | MXC_CCM_RCSR_NFC_FMS, &ccm->rcsr);

	setup_iomux_i2c();
	setup_iomux_usbotg();
	setup_iomux_fec();
	setup_iomux_spi();

	return 0;
}

int board_init(void)
{
	gd->bd->bi_arch_number = MACH_TYPE_MX35_3DS;	/* board id for linux */
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	return 0;
}

static inline int pmic_detect(void)
{
	unsigned int id;
	struct pmic *p = pmic_get("FSL_PMIC");
	if (!p)
		return -ENODEV;

	pmic_reg_read(p, REG_IDENTIFICATION, &id);

	id = (id >> 6) & 0x7;
	if (id == 0x7)
		return 1;
	return 0;
}

u32 get_board_rev(void)
{
	int rev;

	rev = pmic_detect();

	return (get_cpu_rev() & ~(0xF << 8)) | (rev & 0xF) << 8;
}

int board_late_init(void)
{
	u8 val;
	u32 pmic_val;
	struct pmic *p;
	int ret;

	ret = pmic_init(I2C_0);
	if (ret)
		return ret;

	if (pmic_detect()) {
		p = pmic_get("FSL_PMIC");
		imx_iomux_v3_setup_pad(MX35_PAD_WDOG_RST__WDOG_WDOG_B);

		pmic_reg_read(p, REG_SETTING_0, &pmic_val);
		pmic_reg_write(p, REG_SETTING_0,
			pmic_val | VO_1_30V | VO_1_50V);
		pmic_reg_read(p, REG_MODE_0, &pmic_val);
		pmic_reg_write(p, REG_MODE_0, pmic_val | VGEN3EN);

		imx_iomux_v3_setup_pad(MX35_PAD_COMPARE__GPIO1_5);

		gpio_direction_output(IMX_GPIO_NR(1, 5), 1);
	}

	val = mc9sdz60_reg_read(MC9SDZ60_REG_GPIO_1) | 0x04;
	mc9sdz60_reg_write(MC9SDZ60_REG_GPIO_1, val);
	mdelay(200);

	val = mc9sdz60_reg_read(MC9SDZ60_REG_RESET_1) & 0x7F;
	mc9sdz60_reg_write(MC9SDZ60_REG_RESET_1, val);
	mdelay(200);

	val |= 0x80;
	mc9sdz60_reg_write(MC9SDZ60_REG_RESET_1, val);

	/* Print board revision */
	printf("Board: MX35 PDK %d.0\n", ((get_board_rev() >> 8) + 1) & 0x0F);

	return 0;
}

int board_eth_init(bd_t *bis)
{
#if defined(CONFIG_SMC911X)
	int rc = smc911x_initialize(0, CONFIG_SMC911X_BASE);
	if (rc)
		return rc;
#endif
	return cpu_eth_init(bis);
}

#if defined(CONFIG_FSL_ESDHC)

struct fsl_esdhc_cfg esdhc_cfg = {MMC_SDHC1_BASE_ADDR};

int board_mmc_init(bd_t *bis)
{
	static const iomux_v3_cfg_t sdhc1_pads[] = {
		MX35_PAD_SD1_CMD__ESDHC1_CMD,
		MX35_PAD_SD1_CLK__ESDHC1_CLK,
		MX35_PAD_SD1_DATA0__ESDHC1_DAT0,
		MX35_PAD_SD1_DATA1__ESDHC1_DAT1,
		MX35_PAD_SD1_DATA2__ESDHC1_DAT2,
		MX35_PAD_SD1_DATA3__ESDHC1_DAT3,
	};

	/* configure pins for SDHC1 only */
	imx_iomux_v3_setup_multiple_pads(sdhc1_pads, ARRAY_SIZE(sdhc1_pads));

	esdhc_cfg.sdhc_clk = mxc_get_clock(MXC_ESDHC1_CLK);
	return fsl_esdhc_initialize(bis, &esdhc_cfg);
}

int board_mmc_getcd(struct mmc *mmc)
{
	return !(mc9sdz60_reg_read(MC9SDZ60_REG_DES_FLAG) & 0x4);
}
#endif
