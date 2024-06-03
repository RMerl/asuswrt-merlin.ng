// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012, Stefano Babic <sbabic@denx.de>
 *
 * Based on flea3.c and mx35pdk.c
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
#include <mc13892.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <linux/types.h>
#include <asm/gpio.h>
#include <asm/arch/sys_proto.h>
#include <netdev.h>
#include <spl.h>

#define CCM_CCMR_CONFIG		0x003F4208

#define ESDCTL_DDR2_CONFIG	0x007FFC3F

/* For MMC */
#define GPIO_MMC_CD	7
#define GPIO_MMC_WP	8

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
	};

	/* setup pins for FEC */
	imx_iomux_v3_setup_multiple_pads(fec_pads, ARRAY_SIZE(fec_pads));
}

int woodburn_init(void)
{
	struct ccm_regs *ccm =
		(struct ccm_regs *)IMX_CCM_BASE;

	/* initialize PLL and clock configuration */
	writel(CCM_CCMR_CONFIG, &ccm->ccmr);

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
	setup_iomux_fec();

	/* setup GPIO1_4 FEC_ENABLE signal */
	imx_iomux_v3_setup_pad(MX35_PAD_SCKR__GPIO1_4);
	gpio_direction_output(4, 1);
	imx_iomux_v3_setup_pad(MX35_PAD_HCKT__GPIO1_9);
	gpio_direction_output(9, 1);

	return 0;
}

#if defined(CONFIG_SPL_BUILD)
void board_init_f(ulong dummy)
{
	/* Set the stack pointer. */
	asm volatile("mov sp, %0\n" : : "r"(CONFIG_SPL_STACK));

	/* Initialize MUX and SDRAM */
	woodburn_init();

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	preloader_console_init();
	timer_init();

	board_init_r(NULL, 0);
}

void spl_board_init(void)
{
}

#endif


/* Booting from NOR in external mode */
int board_early_init_f(void)
{
	return woodburn_init();
}


int board_init(void)
{
	struct pmic *p;
	u32 val;
	int ret;

	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	ret = pmic_init(I2C_PMIC);
	if (ret)
		return ret;

	p = pmic_get("FSL_PMIC");

	/*
	 * Set switchers in Auto in NORMAL mode & STANDBY mode
	 * Setup the switcher mode for SW1 & SW2
	 */
	pmic_reg_read(p, REG_SW_4, &val);
	val = (val & ~((SWMODE_MASK << SWMODE1_SHIFT) |
		(SWMODE_MASK << SWMODE2_SHIFT)));
	val |= (SWMODE_AUTO_AUTO << SWMODE1_SHIFT) |
		(SWMODE_AUTO_AUTO << SWMODE2_SHIFT);
	/* Set SWILIMB */
	val |= (1 << 22);
	pmic_reg_write(p, REG_SW_4, val);

	/* Setup the switcher mode for SW3 & SW4 */
	pmic_reg_read(p, REG_SW_5, &val);
	val &= ~((SWMODE_MASK << SWMODE4_SHIFT) |
		(SWMODE_MASK << SWMODE3_SHIFT));
	val |= (SWMODE_AUTO_AUTO << SWMODE4_SHIFT) |
		(SWMODE_AUTO_AUTO << SWMODE3_SHIFT);
	pmic_reg_write(p, REG_SW_5, val);

	/* Set VGEN1 to 3.15V */
	pmic_reg_read(p, REG_SETTING_0, &val);
	val &= ~(VGEN1_MASK);
	val |= VGEN1_3_15;
	pmic_reg_write(p, REG_SETTING_0, val);

	pmic_reg_read(p, REG_MODE_0, &val);
	val |= VGEN1EN;
	pmic_reg_write(p, REG_MODE_0, val);
	udelay(2000);

	return 0;
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

	/* MMC Card Detect on GPIO1_7 */
	imx_iomux_v3_setup_pad(MX35_PAD_SCKT__GPIO1_7);
	gpio_direction_input(GPIO_MMC_CD);

	/* MMC Write Protection on GPIO1_8 */
	imx_iomux_v3_setup_pad(MX35_PAD_FST__GPIO1_8);
	gpio_direction_input(GPIO_MMC_WP);

	esdhc_cfg.sdhc_clk = mxc_get_clock(MXC_ESDHC1_CLK);

	return fsl_esdhc_initialize(bis, &esdhc_cfg);
}

int board_mmc_getcd(struct mmc *mmc)
{
	return !gpio_get_value(GPIO_MMC_CD);
}
#endif

u32 get_board_rev(void)
{
	int rev = 0;

	return (get_cpu_rev() & ~(0xF << 8)) | (rev & 0xF) << 8;
}
