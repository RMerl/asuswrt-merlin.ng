// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <init.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux-mx51.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/clock.h>
#include <asm/mach-imx/mx5_video.h>
#include <i2c.h>
#include <input.h>
#include <mmc.h>
#include <fsl_esdhc_imx.h>
#include <power/pmic.h>
#include <fsl_pmic.h>
#include <mc13892.h>
#include <usb/ehci-ci.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	/* dram_init must store complete ramsize in gd->ram_size */
	gd->ram_size = get_ram_size((void *)CFG_SYS_SDRAM_BASE,
				PHYS_SDRAM_1_SIZE);
	return 0;
}

#ifdef CONFIG_REVISION_TAG
u32 get_board_rev(void)
{
	u32 rev = get_cpu_rev();
	if (!gpio_get_value(IMX_GPIO_NR(1, 22)))
		rev |= BOARD_REV_2_0 << BOARD_VER_OFFSET;
	return rev;
}
#endif

#define UART_PAD_CTRL	(PAD_CTL_HYS | PAD_CTL_PUS_100K_DOWN | PAD_CTL_DSE_HIGH)

static void setup_iomux_uart(void)
{
	static const iomux_v3_cfg_t uart_pads[] = {
		MX51_PAD_UART1_RXD__UART1_RXD,
		MX51_PAD_UART1_TXD__UART1_TXD,
		NEW_PAD_CTRL(MX51_PAD_UART1_RTS__UART1_RTS, UART_PAD_CTRL),
		NEW_PAD_CTRL(MX51_PAD_UART1_CTS__UART1_CTS, UART_PAD_CTRL),
	};

	imx_iomux_v3_setup_multiple_pads(uart_pads, ARRAY_SIZE(uart_pads));
}

#ifdef CONFIG_MXC_SPI
static void setup_iomux_spi(void)
{
	static const iomux_v3_cfg_t spi_pads[] = {
		NEW_PAD_CTRL(MX51_PAD_CSPI1_MOSI__ECSPI1_MOSI, PAD_CTL_HYS |
				PAD_CTL_DSE_HIGH | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_CSPI1_MISO__ECSPI1_MISO, PAD_CTL_HYS |
				PAD_CTL_DSE_HIGH | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_CSPI1_SS1__ECSPI1_SS1,
				MX51_GPIO_PAD_CTRL),
		MX51_PAD_CSPI1_SS0__ECSPI1_SS0,
		NEW_PAD_CTRL(MX51_PAD_CSPI1_RDY__ECSPI1_RDY, MX51_PAD_CTRL_2),
		NEW_PAD_CTRL(MX51_PAD_CSPI1_SCLK__ECSPI1_SCLK, PAD_CTL_HYS |
				PAD_CTL_DSE_HIGH | PAD_CTL_SRE_FAST),
	};

	imx_iomux_v3_setup_multiple_pads(spi_pads, ARRAY_SIZE(spi_pads));
}
#endif

static void power_init(void)
{
	unsigned int val;
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)MXC_CCM_BASE;
	struct pmic *p;
	int ret;

	ret = pmic_init(CFG_FSL_PMIC_BUS);
	if (ret)
		return;

	p = pmic_get("FSL_PMIC");
	if (!p)
		return;

	/* Write needed to Power Gate 2 register */
	pmic_reg_read(p, REG_POWER_MISC, &val);
	val &= ~PWGT2SPIEN;
	pmic_reg_write(p, REG_POWER_MISC, val);

	/* Externally powered */
	pmic_reg_read(p, REG_CHARGE, &val);
	val |= ICHRG0 | ICHRG1 | ICHRG2 | ICHRG3 | CHGAUTOB;
	pmic_reg_write(p, REG_CHARGE, val);

	/* power up the system first */
	pmic_reg_write(p, REG_POWER_MISC, PWUP);

	/* Set core voltage to 1.1V */
	pmic_reg_read(p, REG_SW_0, &val);
	val = (val & ~SWx_VOLT_MASK) | SWx_1_100V;
	pmic_reg_write(p, REG_SW_0, val);

	/* Setup VCC (SW2) to 1.25 */
	pmic_reg_read(p, REG_SW_1, &val);
	val = (val & ~SWx_VOLT_MASK) | SWx_1_250V;
	pmic_reg_write(p, REG_SW_1, val);

	/* Setup 1V2_DIG1 (SW3) to 1.25 */
	pmic_reg_read(p, REG_SW_2, &val);
	val = (val & ~SWx_VOLT_MASK) | SWx_1_250V;
	pmic_reg_write(p, REG_SW_2, val);
	udelay(50);

	/* Raise the core frequency to 800MHz */
	writel(0x0, &mxc_ccm->cacrr);

	/* Set switchers in Auto in NORMAL mode & STANDBY mode */
	/* Setup the switcher mode for SW1 & SW2*/
	pmic_reg_read(p, REG_SW_4, &val);
	val = (val & ~((SWMODE_MASK << SWMODE1_SHIFT) |
		(SWMODE_MASK << SWMODE2_SHIFT)));
	val |= (SWMODE_AUTO_AUTO << SWMODE1_SHIFT) |
		(SWMODE_AUTO_AUTO << SWMODE2_SHIFT);
	pmic_reg_write(p, REG_SW_4, val);

	/* Setup the switcher mode for SW3 & SW4 */
	pmic_reg_read(p, REG_SW_5, &val);
	val = (val & ~((SWMODE_MASK << SWMODE3_SHIFT) |
		(SWMODE_MASK << SWMODE4_SHIFT)));
	val |= (SWMODE_AUTO_AUTO << SWMODE3_SHIFT) |
		(SWMODE_AUTO_AUTO << SWMODE4_SHIFT);
	pmic_reg_write(p, REG_SW_5, val);

	/* Set VDIG to 1.65V, VGEN3 to 1.8V, VCAM to 2.6V */
	pmic_reg_read(p, REG_SETTING_0, &val);
	val &= ~(VCAM_MASK | VGEN3_MASK | VDIG_MASK);
	val |= VDIG_1_65 | VGEN3_1_8 | VCAM_2_6;
	pmic_reg_write(p, REG_SETTING_0, val);

	/* Set VVIDEO to 2.775V, VAUDIO to 3V, VSD to 3.15V */
	pmic_reg_read(p, REG_SETTING_1, &val);
	val &= ~(VVIDEO_MASK | VSD_MASK | VAUDIO_MASK);
	val |= VSD_3_15 | VAUDIO_3_0 | VVIDEO_2_775;
	pmic_reg_write(p, REG_SETTING_1, val);

	/* Configure VGEN3 and VCAM regulators to use external PNP */
	val = VGEN3CONFIG | VCAMCONFIG;
	pmic_reg_write(p, REG_MODE_1, val);
	udelay(200);

	/* Enable VGEN3, VCAM, VAUDIO, VVIDEO, VSD regulators */
	val = VGEN3EN | VGEN3CONFIG | VCAMEN | VCAMCONFIG |
		VVIDEOEN | VAUDIOEN  | VSDEN;
	pmic_reg_write(p, REG_MODE_1, val);

	imx_iomux_v3_setup_pad(NEW_PAD_CTRL(MX51_PAD_EIM_A20__GPIO2_14,
						NO_PAD_CTRL));
	gpio_request(IMX_GPIO_NR(2, 14), "gpio2_14");
	gpio_direction_output(IMX_GPIO_NR(2, 14), 0);

	udelay(500);

	gpio_set_value(IMX_GPIO_NR(2, 14), 1);
}

int board_early_init_f(void)
{
	setup_iomux_uart();
	setup_iomux_lcd();

	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	return 0;
}

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
#ifdef CONFIG_MXC_SPI
	setup_iomux_spi();
	power_init();
#endif

	return 0;
}
#endif

/*
 * Do not overwrite the console
 * Use always serial for U-Boot console
 */
int overwrite_console(void)
{
	return 1;
}

int checkboard(void)
{
	puts("Board: MX51EVK\n");

	return 0;
}
