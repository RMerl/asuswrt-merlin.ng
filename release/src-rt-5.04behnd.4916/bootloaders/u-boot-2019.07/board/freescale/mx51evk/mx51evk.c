// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux-mx51.h>
#include <linux/errno.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/clock.h>
#include <asm/mach-imx/mx5_video.h>
#include <i2c.h>
#include <input.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <power/pmic.h>
#include <fsl_pmic.h>
#include <mc13892.h>
#include <usb/ehci-ci.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_FSL_ESDHC
struct fsl_esdhc_cfg esdhc_cfg[2] = {
	{MMC_SDHC1_BASE_ADDR},
	{MMC_SDHC2_BASE_ADDR},
};
#endif

int dram_init(void)
{
	/* dram_init must store complete ramsize in gd->ram_size */
	gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE,
				PHYS_SDRAM_1_SIZE);
	return 0;
}

u32 get_board_rev(void)
{
	u32 rev = get_cpu_rev();
	if (!gpio_get_value(IMX_GPIO_NR(1, 22)))
		rev |= BOARD_REV_2_0 << BOARD_VER_OFFSET;
	return rev;
}

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

static void setup_iomux_fec(void)
{
	static const iomux_v3_cfg_t fec_pads[] = {
		NEW_PAD_CTRL(MX51_PAD_EIM_EB2__FEC_MDIO, PAD_CTL_HYS |
				PAD_CTL_PUS_22K_UP | PAD_CTL_ODE |
				PAD_CTL_DSE_HIGH | PAD_CTL_SRE_FAST),
		MX51_PAD_NANDF_CS3__FEC_MDC,
		NEW_PAD_CTRL(MX51_PAD_EIM_CS3__FEC_RDATA3, MX51_PAD_CTRL_2),
		NEW_PAD_CTRL(MX51_PAD_EIM_CS2__FEC_RDATA2, MX51_PAD_CTRL_2),
		NEW_PAD_CTRL(MX51_PAD_EIM_EB3__FEC_RDATA1, MX51_PAD_CTRL_2),
		MX51_PAD_NANDF_D9__FEC_RDATA0,
		MX51_PAD_NANDF_CS6__FEC_TDATA3,
		MX51_PAD_NANDF_CS5__FEC_TDATA2,
		MX51_PAD_NANDF_CS4__FEC_TDATA1,
		MX51_PAD_NANDF_D8__FEC_TDATA0,
		MX51_PAD_NANDF_CS7__FEC_TX_EN,
		MX51_PAD_NANDF_CS2__FEC_TX_ER,
		MX51_PAD_NANDF_RDY_INT__FEC_TX_CLK,
		NEW_PAD_CTRL(MX51_PAD_NANDF_RB2__FEC_COL, MX51_PAD_CTRL_4),
		NEW_PAD_CTRL(MX51_PAD_NANDF_RB3__FEC_RX_CLK, MX51_PAD_CTRL_4),
		MX51_PAD_EIM_CS5__FEC_CRS,
		MX51_PAD_EIM_CS4__FEC_RX_ER,
		NEW_PAD_CTRL(MX51_PAD_NANDF_D11__FEC_RX_DV, MX51_PAD_CTRL_4),
	};

	imx_iomux_v3_setup_multiple_pads(fec_pads, ARRAY_SIZE(fec_pads));
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

#ifdef CONFIG_USB_EHCI_MX5
#define MX51EVK_USBH1_HUB_RST	IMX_GPIO_NR(1, 7)
#define MX51EVK_USBH1_STP	IMX_GPIO_NR(1, 27)
#define MX51EVK_USB_CLK_EN_B	IMX_GPIO_NR(2, 1)
#define MX51EVK_USB_PHY_RESET	IMX_GPIO_NR(2, 5)

static void setup_usb_h1(void)
{
	static const iomux_v3_cfg_t usb_h1_pads[] = {
		MX51_PAD_USBH1_CLK__USBH1_CLK,
		MX51_PAD_USBH1_DIR__USBH1_DIR,
		MX51_PAD_USBH1_STP__USBH1_STP,
		MX51_PAD_USBH1_NXT__USBH1_NXT,
		MX51_PAD_USBH1_DATA0__USBH1_DATA0,
		MX51_PAD_USBH1_DATA1__USBH1_DATA1,
		MX51_PAD_USBH1_DATA2__USBH1_DATA2,
		MX51_PAD_USBH1_DATA3__USBH1_DATA3,
		MX51_PAD_USBH1_DATA4__USBH1_DATA4,
		MX51_PAD_USBH1_DATA5__USBH1_DATA5,
		MX51_PAD_USBH1_DATA6__USBH1_DATA6,
		MX51_PAD_USBH1_DATA7__USBH1_DATA7,

		NEW_PAD_CTRL(MX51_PAD_GPIO1_7__GPIO1_7, 0), /* H1 hub reset */
		MX51_PAD_EIM_D17__GPIO2_1,
		MX51_PAD_EIM_D21__GPIO2_5, /* PHY reset */
	};

	imx_iomux_v3_setup_multiple_pads(usb_h1_pads, ARRAY_SIZE(usb_h1_pads));
}

int board_ehci_hcd_init(int port)
{
	/* Set USBH1_STP to GPIO and toggle it */
	imx_iomux_v3_setup_pad(NEW_PAD_CTRL(MX51_PAD_USBH1_STP__GPIO1_27,
						MX51_USBH_PAD_CTRL));

	gpio_direction_output(MX51EVK_USBH1_STP, 0);
	gpio_direction_output(MX51EVK_USB_PHY_RESET, 0);
	mdelay(10);
	gpio_set_value(MX51EVK_USBH1_STP, 1);

	/* Set back USBH1_STP to be function */
	imx_iomux_v3_setup_pad(MX51_PAD_USBH1_STP__USBH1_STP);

	/* De-assert USB PHY RESETB */
	gpio_set_value(MX51EVK_USB_PHY_RESET, 1);

	/* Drive USB_CLK_EN_B line low */
	gpio_direction_output(MX51EVK_USB_CLK_EN_B, 0);

	/* Reset USB hub */
	gpio_direction_output(MX51EVK_USBH1_HUB_RST, 0);
	mdelay(2);
	gpio_set_value(MX51EVK_USBH1_HUB_RST, 1);
	return 0;
}
#endif

static void power_init(void)
{
	unsigned int val;
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)MXC_CCM_BASE;
	struct pmic *p;
	int ret;

	ret = pmic_init(CONFIG_FSL_PMIC_BUS);
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
	gpio_direction_output(IMX_GPIO_NR(2, 14), 0);

	udelay(500);

	gpio_set_value(IMX_GPIO_NR(2, 14), 1);
}

#ifdef CONFIG_FSL_ESDHC
int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret;

	imx_iomux_v3_setup_pad(NEW_PAD_CTRL(MX51_PAD_GPIO1_0__GPIO1_0,
						NO_PAD_CTRL));
	gpio_direction_input(IMX_GPIO_NR(1, 0));
	imx_iomux_v3_setup_pad(NEW_PAD_CTRL(MX51_PAD_GPIO1_6__GPIO1_6,
						NO_PAD_CTRL));
	gpio_direction_input(IMX_GPIO_NR(1, 6));

	if (cfg->esdhc_base == MMC_SDHC1_BASE_ADDR)
		ret = !gpio_get_value(IMX_GPIO_NR(1, 0));
	else
		ret = !gpio_get_value(IMX_GPIO_NR(1, 6));

	return ret;
}

int board_mmc_init(bd_t *bis)
{
	static const iomux_v3_cfg_t sd1_pads[] = {
		NEW_PAD_CTRL(MX51_PAD_SD1_CMD__SD1_CMD, PAD_CTL_DSE_MAX |
			PAD_CTL_HYS | PAD_CTL_PUS_47K_UP | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_SD1_CLK__SD1_CLK, PAD_CTL_DSE_MAX |
			PAD_CTL_PUS_47K_UP | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_SD1_DATA0__SD1_DATA0, PAD_CTL_DSE_MAX |
			PAD_CTL_HYS | PAD_CTL_PUS_47K_UP | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_SD1_DATA1__SD1_DATA1, PAD_CTL_DSE_MAX |
			PAD_CTL_HYS | PAD_CTL_PUS_47K_UP | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_SD1_DATA2__SD1_DATA2, PAD_CTL_DSE_MAX |
			PAD_CTL_HYS | PAD_CTL_PUS_47K_UP | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_SD1_DATA3__SD1_DATA3, PAD_CTL_DSE_MAX |
			PAD_CTL_HYS | PAD_CTL_PUS_100K_DOWN | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_GPIO1_0__SD1_CD, PAD_CTL_HYS),
		NEW_PAD_CTRL(MX51_PAD_GPIO1_1__SD1_WP, PAD_CTL_HYS),
	};

	static const iomux_v3_cfg_t sd2_pads[] = {
		NEW_PAD_CTRL(MX51_PAD_SD2_CMD__SD2_CMD,
				PAD_CTL_DSE_MAX | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_SD2_CLK__SD2_CLK,
				PAD_CTL_DSE_MAX | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_SD2_DATA0__SD2_DATA0,
				PAD_CTL_DSE_MAX | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_SD2_DATA1__SD2_DATA1,
				PAD_CTL_DSE_MAX | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_SD2_DATA2__SD2_DATA2,
				PAD_CTL_DSE_MAX | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_SD2_DATA3__SD2_DATA3,
				PAD_CTL_DSE_MAX | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_GPIO1_6__GPIO1_6, PAD_CTL_HYS),
		NEW_PAD_CTRL(MX51_PAD_GPIO1_5__GPIO1_5, PAD_CTL_HYS),
	};

	u32 index;
	int ret;

	esdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
	esdhc_cfg[1].sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);

	for (index = 0; index < CONFIG_SYS_FSL_ESDHC_NUM;
			index++) {
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

int board_early_init_f(void)
{
	setup_iomux_uart();
	setup_iomux_fec();
#ifdef CONFIG_USB_EHCI_MX5
	setup_usb_h1();
#endif
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
