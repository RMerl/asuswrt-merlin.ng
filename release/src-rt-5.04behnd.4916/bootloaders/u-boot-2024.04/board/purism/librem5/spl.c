// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 NXP
 * Copyright 2021 Purism
 */

#include <common.h>
#include <asm/io.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/arch/ddr.h>
#include <asm/arch/imx8mq_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/clock.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/gpio.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <fsl_esdhc_imx.h>
#include <mmc.h>
#include <power/pmic.h>
#include <power/bd71837.h>
#include <hang.h>
#include <init.h>
#include <spl.h>
#include <usb.h>
#include <dwc3-uboot.h>
#include <linux/delay.h>
#include <linux/usb/gadget.h>
#include "librem5.h"

DECLARE_GLOBAL_DATA_PTR;

void spl_dram_init(void)
{
	/* ddr init */
	if ((get_cpu_rev() & 0xfff) == CHIP_REV_2_1)
		ddr_init(&dram_timing);
	else
		ddr_init(&dram_timing_b0);
}

int spl_board_boot_device(enum boot_device boot_dev_spl)
{
	log_debug("%s : starting\n", __func__);

	switch (boot_dev_spl) {
	case SD1_BOOT:
	case MMC1_BOOT:
		return BOOT_DEVICE_MMC1;
	case USB_BOOT:
		return BOOT_DEVICE_BOARD;
	default:
		return BOOT_DEVICE_NONE;
	}
}

#define ECSPI_PAD_CTRL (PAD_CTL_DSE2 | PAD_CTL_HYS)

static const iomux_v3_cfg_t ecspi_pads[] = {
	IMX8MQ_PAD_ECSPI1_SCLK__ECSPI1_SCLK | MUX_PAD_CTRL(ECSPI_PAD_CTRL),
	IMX8MQ_PAD_ECSPI1_SS0__GPIO5_IO9 | MUX_PAD_CTRL(ECSPI_PAD_CTRL),
	IMX8MQ_PAD_ECSPI1_MOSI__ECSPI1_MOSI | MUX_PAD_CTRL(ECSPI_PAD_CTRL),
	IMX8MQ_PAD_ECSPI1_MISO__ECSPI1_MISO | MUX_PAD_CTRL(ECSPI_PAD_CTRL),
};

int board_ecspi_init(void)
{
	imx_iomux_v3_setup_multiple_pads(ecspi_pads, ARRAY_SIZE(ecspi_pads));

	return 0;
}

int board_spi_cs_gpio(unsigned int bus, unsigned int cs)
{
	return (bus == 0 && cs == 0) ? (SPI1_SS0) : -1;
}

#define I2C_PAD_CTRL	(PAD_CTL_PUE | PAD_CTL_ODE | PAD_CTL_DSE7 | PAD_CTL_FSEL3)
#define PC MUX_PAD_CTRL(I2C_PAD_CTRL)
struct i2c_pads_info i2c_pad_info1 = {
	.scl = {
		.i2c_mode = IMX8MQ_PAD_I2C1_SCL__I2C1_SCL | PC,
		.gpio_mode = IMX8MQ_PAD_I2C1_SCL__GPIO5_IO14 | PC,
		.gp = IMX_GPIO_NR(5, 14),
	},
	.sda = {
		.i2c_mode = IMX8MQ_PAD_I2C1_SDA__I2C1_SDA | PC,
		.gpio_mode = IMX8MQ_PAD_I2C1_SDA__GPIO5_IO15 | PC,
		.gp = IMX_GPIO_NR(5, 15),
	},
};

struct i2c_pads_info i2c_pad_info2 = {
	.scl = {
		.i2c_mode = IMX8MQ_PAD_I2C2_SCL__I2C2_SCL | PC,
		.gpio_mode = IMX8MQ_PAD_I2C2_SCL__GPIO5_IO16 | PC,
		.gp = IMX_GPIO_NR(5, 16),
	},
	.sda = {
		.i2c_mode = IMX8MQ_PAD_I2C2_SDA__I2C2_SDA | PC,
		.gpio_mode = IMX8MQ_PAD_I2C2_SDA__GPIO5_IO17 | PC,
		.gp = IMX_GPIO_NR(5, 17),
	},
};

struct i2c_pads_info i2c_pad_info3 = {
	.scl = {
		.i2c_mode = IMX8MQ_PAD_I2C3_SCL__I2C3_SCL | PC,
		.gpio_mode = IMX8MQ_PAD_I2C3_SCL__GPIO5_IO18 | PC,
		.gp = IMX_GPIO_NR(5, 18),
	},
	.sda = {
		.i2c_mode = IMX8MQ_PAD_I2C3_SDA__I2C3_SDA | PC,
		.gpio_mode = IMX8MQ_PAD_I2C3_SDA__GPIO5_IO19 | PC,
		.gp = IMX_GPIO_NR(5, 19),
	},
};

struct i2c_pads_info i2c_pad_info4 = {
	.scl = {
		.i2c_mode = IMX8MQ_PAD_I2C4_SCL__I2C4_SCL | PC,
		.gpio_mode = IMX8MQ_PAD_I2C4_SCL__GPIO5_IO20 | PC,
		.gp = IMX_GPIO_NR(5, 20),
	},
	.sda = {
		.i2c_mode = IMX8MQ_PAD_I2C4_SDA__I2C4_SDA | PC,
		.gpio_mode = IMX8MQ_PAD_I2C4_SDA__GPIO5_IO21 | PC,
		.gp = IMX_GPIO_NR(5, 21),
	},
};

#define UART_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_FSEL1)

static const iomux_v3_cfg_t uart_pads[] = {
	IMX8MQ_PAD_UART1_RXD__UART1_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
	IMX8MQ_PAD_UART1_TXD__UART1_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
	IMX8MQ_PAD_UART2_RXD__UART2_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
	IMX8MQ_PAD_UART2_TXD__UART2_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
	IMX8MQ_PAD_UART3_RXD__UART3_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
	IMX8MQ_PAD_UART3_TXD__UART3_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
	IMX8MQ_PAD_ECSPI2_SCLK__UART4_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
	IMX8MQ_PAD_ECSPI2_MOSI__UART4_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

#define USDHC1_PWR_GPIO IMX_GPIO_NR(2, 10)
#define USDHC2_PWR_GPIO IMX_GPIO_NR(2, 19)

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	switch (cfg->esdhc_base) {
	case USDHC1_BASE_ADDR:
		ret = 1;
		break;
	case USDHC2_BASE_ADDR:
		ret = 1;
		break;
	}

	return ret;
}

#define USDHC_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_HYS | PAD_CTL_PUE | \
			 PAD_CTL_FSEL1)
#define USDHC_GPIO_PAD_CTRL (PAD_CTL_PUE | PAD_CTL_DSE1)

static const iomux_v3_cfg_t usdhc1_pads[] = {
	IMX8MQ_PAD_SD1_CLK__USDHC1_CLK | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD1_CMD__USDHC1_CMD | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD1_DATA0__USDHC1_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD1_DATA1__USDHC1_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD1_DATA2__USDHC1_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD1_DATA3__USDHC1_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD1_DATA4__USDHC1_DATA4 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD1_DATA5__USDHC1_DATA5 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD1_DATA6__USDHC1_DATA6 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD1_DATA7__USDHC1_DATA7 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD1_RESET_B__GPIO2_IO10 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static const iomux_v3_cfg_t usdhc2_pads[] = {
	IMX8MQ_PAD_SD2_CLK__USDHC2_CLK | MUX_PAD_CTRL(USDHC_PAD_CTRL), /* 0xd6 */
	IMX8MQ_PAD_SD2_CMD__USDHC2_CMD | MUX_PAD_CTRL(USDHC_PAD_CTRL), /* 0xd6 */
	IMX8MQ_PAD_SD2_DATA0__USDHC2_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL), /* 0xd6 */
	IMX8MQ_PAD_SD2_DATA1__USDHC2_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL), /* 0xd6 */
	IMX8MQ_PAD_SD2_DATA2__USDHC2_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL), /* 0x16 */
	IMX8MQ_PAD_SD2_DATA3__USDHC2_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL), /* 0xd6 */
	IMX8MQ_PAD_SD2_RESET_B__GPIO2_IO19 | MUX_PAD_CTRL(USDHC_GPIO_PAD_CTRL),
};

static struct fsl_esdhc_cfg usdhc_cfg[2] = {
	{USDHC1_BASE_ADDR, 0, 8},
	{USDHC2_BASE_ADDR, 0, 4},
};

int board_mmc_init(struct bd_info *bis)
{
	int i, ret;
	/*
	 * According to the board_mmc_init() the following map is done:
	 * (U-Boot device node)    (Physical Port)
	 * mmc0                    USDHC1
	 * mmc1                    USDHC2
	 */
	for (i = 0; i < CFG_SYS_FSL_USDHC_NUM; i++) {
		log_debug("Initializing FSL USDHC port %d\n", i);
		switch (i) {
		case 0:
			init_clk_usdhc(0);
			usdhc_cfg[0].sdhc_clk = mxc_get_clock(USDHC1_CLK_ROOT);
			imx_iomux_v3_setup_multiple_pads(usdhc1_pads,
							 ARRAY_SIZE(usdhc1_pads));
			gpio_request(USDHC1_PWR_GPIO, "usdhc1_reset");
			gpio_direction_output(USDHC1_PWR_GPIO, 0);
			udelay(500);
			gpio_direction_output(USDHC1_PWR_GPIO, 1);
			break;
		case 1:
			init_clk_usdhc(1);
			usdhc_cfg[1].sdhc_clk = mxc_get_clock(USDHC2_CLK_ROOT);
			imx_iomux_v3_setup_multiple_pads(usdhc2_pads,
							 ARRAY_SIZE(usdhc2_pads));
			gpio_request(USDHC2_PWR_GPIO, "usdhc2_reset");
			gpio_direction_output(USDHC2_PWR_GPIO, 0);
			udelay(500);
			gpio_direction_output(USDHC2_PWR_GPIO, 1);
			break;
		default:
			log_err("Warning: USDHC controller(%d) not supported\n", i + 1);
			return -EINVAL;
		}

		ret = fsl_esdhc_initialize(bis, &usdhc_cfg[i]);
		if (ret)
			return ret;
	}

	return 0;
}

#define LDO_VOLT_EN                     BIT(6)

/*
 * Disable the charger - it will be re-enabled in u-boot
 */
void disable_charger_bq25895(void)
{
	u8 val;
	int timeout = 1000; // ms

	/* Set the i2c bus */
	i2c_set_bus_num(3);

	/* disable ship mode if BATFET_DLY is set */
	val = i2c_reg_read(0x6a, 0x09);
	log_debug("REG09 0x%x\n", val);
	if (val & 0x28) {
		val = val & ~0x28;
		i2c_reg_write(0x6a, 0x09, val);
	}

	/* disable and trigger DPDM, ICO, HVDCP and MaxCharge */
	val = i2c_reg_read(0x6a, 0x02);
	log_debug("REG02 0x%x\n", val);
	val &= 0xe0;
	i2c_reg_write(0x6a, 0x02, val);

	/* disable charger and enable BAT_LOADEN */
	val = i2c_reg_read(0x6a, 0x03);
	log_debug("REG03 0x%x\n", val);
	val = (val | 0x80) & ~0x10;
	i2c_reg_write(0x6a, 0x03, val);

	mdelay(10);

	/* force ADC conversions */
	val = i2c_reg_read(0x6a, 0x02);
	log_debug("REG02 0x%x\n", val);
	val = (val | 0x80) & ~0x40;
	i2c_reg_write(0x6a, 0x02, val);

	do {
		mdelay(10);
		timeout -= 10;
	} while ((i2c_reg_read(0x6a, 0x02) & 0x80) && (timeout > 0));

	/* enable STAT pin */
	val = i2c_reg_read(0x6a, 0x07);
	log_debug("REG07 0x%x\n", val);
	val = val & ~0x40;
	i2c_reg_write(0x6a, 0x07, val);

	/* check VBUS */
	val = i2c_reg_read(0x6a, 0x11);
	log_debug("VBUS good %d\n", (val >> 7) & 1);
	log_debug("VBUS mV %d\n", (val & 0x7f) * 100 + 2600);

	/* check VBAT */
	val = i2c_reg_read(0x6a, 0x0e);
	log_debug("VBAT mV %d\n", (val & 0x7f) * 20 + 2304);

	/* limit the VINDPM to 3.9V  */
	i2c_reg_write(0x6a, 0x0d, 0x8d);

	/* set the max voltage to 4.192V */
	val = i2c_reg_read(0x6a, 0x6);
	val = (val & ~0xFC) | 0x16 << 2;
	i2c_reg_write(0x6a, 0x6, val);

	/* set the SYS_MIN to 3.7V */
	val = i2c_reg_read(0x6a, 0x3);
	val = val | 0xE;
	i2c_reg_write(0x6a, 0x3, val);

	/* disable BAT_LOADEN */
	val = i2c_reg_read(0x6a, 0x03);
	log_debug("REG03 0x%x\n", val);
	val = val & ~0x80;
	i2c_reg_write(0x6a, 0x03, val);
}

#define I2C_PMIC	0
#define POWER_BD71837_I2C_BUS	0
#define POWER_BD71837_I2C_ADDR	0x4B

int power_bd71837_init(unsigned char bus)
{
	static const char name[] = BD718XX_REGULATOR_DRIVER;
	struct pmic *p = pmic_alloc();

	if (!p) {
		log_err("%s: POWER allocation error!\n", __func__);
		return -ENOMEM;
	}

	p->name = name;
	p->interface = I2C_PMIC;
	p->number_of_regs = BD718XX_MAX_REGISTER;
	p->hw.i2c.addr = POWER_BD71837_I2C_ADDR;
	p->hw.i2c.tx_num = 1;
	p->bus = bus;

	return 0;
}

int power_init_board(void)
{
	struct pmic *p;
	int ldo[] = {BD718XX_LDO5_VOLT, BD718XX_LDO6_VOLT,
		     BD71837_LDO7_VOLT};
	u32 val;
	int i, rv;

	/* Set the i2c bus */
	setup_i2c(0, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info1);

	/*
	 * Init PMIC
	 */
	rv = power_bd71837_init(POWER_BD71837_I2C_BUS);
	if (rv) {
		log_err("%s: power_bd71837_init(%d) error %d\n", __func__,
			POWER_BD71837_I2C_BUS, rv);
		goto out;
	}

	p = pmic_get(BD718XX_REGULATOR_DRIVER);
	if (!p) {
		log_err("%s: pmic_get(%s) failed\n", __func__, BD718XX_REGULATOR_DRIVER);
		rv = -ENODEV;
		goto out;
	}

	rv = pmic_probe(p);
	if (rv) {
		log_err("%s: pmic_probe() error %d\n", __func__, rv);
		goto out;
	}

	/*
	 * Unlock all regs
	 */
	pmic_reg_write(p, BD718XX_REGLOCK, 0);

	/* find the reset cause */
	pmic_reg_read(p, 0x29, &val);
	log_debug("%s: reset cause %d\n", __func__, val);

	/*
	 * Reconfigure default voltages and disable:
	 * - BUCK3: VDD_GPU_0V9 (1.00 -> 0.90)
	 * - BUCK4: VDD_VPU_0V9 (1.00 -> 0.90)
	 */
	pmic_reg_write(p, BD71837_BUCK3_VOLT_RUN, 0x14);
	pmic_reg_write(p, BD71837_BUCK4_VOLT_RUN, 0x14);

	/*
	 * Enable PHYs voltages: LDO5-7
	 */
	for (i = 0; i < ARRAY_SIZE(ldo); i++) {
		rv = pmic_reg_read(p, ldo[i], &val);
		if (rv) {
			log_err("%s: pmic_read(%x) error %d\n", __func__,
				ldo[i], rv);
			continue;
		}

		pmic_reg_write(p, ldo[i], val | LDO_VOLT_EN);
	}

	udelay(500);

	rv = 0;
out:
	return rv;
}

int dm_usb_gadget_handle_interrupts(struct udevice *dev)
{
	dwc3_uboot_handle_interrupt(dev);
	return 0;
}

static void dwc3_nxp_usb_phy_init(struct dwc3_device *dwc3)
{
	u32 RegData;

	RegData = readl(dwc3->base + USB_PHY_CTRL1);
	RegData &= ~(USB_PHY_CTRL1_VDATSRCENB0 | USB_PHY_CTRL1_VDATDETENB0 |
			USB_PHY_CTRL1_COMMONONN);
	RegData |= USB_PHY_CTRL1_RESET | USB_PHY_CTRL1_ATERESET;
	writel(RegData, dwc3->base + USB_PHY_CTRL1);

	RegData = readl(dwc3->base + USB_PHY_CTRL0);
	RegData |= USB_PHY_CTRL0_REF_SSP_EN;
	RegData &= ~USB_PHY_CTRL0_SSC_RANGE_MASK;
	RegData |= USB_PHY_CTRL0_SSC_RANGE_4003PPM;
	writel(RegData, dwc3->base + USB_PHY_CTRL0);

	RegData = readl(dwc3->base + USB_PHY_CTRL2);
	RegData |= USB_PHY_CTRL2_TXENABLEN0;
	writel(RegData, dwc3->base + USB_PHY_CTRL2);

	RegData = readl(dwc3->base + USB_PHY_CTRL1);
	RegData &= ~(USB_PHY_CTRL1_RESET | USB_PHY_CTRL1_ATERESET);
	writel(RegData, dwc3->base + USB_PHY_CTRL1);

	/* Disable rx term override */
	RegData = readl(dwc3->base + USB_PHY_CTRL6);
	RegData &= ~USB_PHY_CTRL6_RXTERM_OVERRIDE_SEL;
	writel(RegData, dwc3->base + USB_PHY_CTRL6);
}

static struct dwc3_device dwc3_device0_data = {
	.maximum_speed = USB_SPEED_HIGH,
	.base = USB1_BASE_ADDR,
	.dr_mode = USB_DR_MODE_PERIPHERAL,
	.index = 0,
};

static struct dwc3_device dwc3_device1_data = {
	.maximum_speed = USB_SPEED_HIGH,
	.base = USB2_BASE_ADDR,
	.dr_mode = USB_DR_MODE_HOST,
	.index = 1,
};

int board_usb_init(int index, enum usb_init_type init)
{
	int ret = 0;

	printf("%s : index %d type %d\n", __func__, index, init);

	if (index == 0 && init == USB_INIT_DEVICE) {
		dwc3_nxp_usb_phy_init(&dwc3_device0_data);
		ret = dwc3_uboot_init(&dwc3_device0_data);
	}
	if (index == 1 && init == USB_INIT_HOST) {
		dwc3_nxp_usb_phy_init(&dwc3_device1_data);
		ret = dwc3_uboot_init(&dwc3_device1_data);
	}

	return ret;
}

int board_usb_cleanup(int index, enum usb_init_type init)
{
	u32 RegData;
	struct dwc3_device *dwc3;

	printf("%s : %d\n", __func__, index);

	if (index == 0 && init == USB_INIT_DEVICE)
		dwc3 = &dwc3_device0_data;
	if (index == 1 && init == USB_INIT_HOST)
		dwc3 = &dwc3_device1_data;

	dwc3_uboot_exit(index);

	/* reset the phy */
	RegData = readl(dwc3->base + USB_PHY_CTRL1);
	RegData &= ~(USB_PHY_CTRL1_VDATSRCENB0 | USB_PHY_CTRL1_VDATDETENB0 |
			USB_PHY_CTRL1_COMMONONN);
	RegData |= USB_PHY_CTRL1_RESET | USB_PHY_CTRL1_ATERESET;
	writel(RegData, dwc3->base + USB_PHY_CTRL1);

	/* enable rx term override */
	RegData = readl(dwc3->base + USB_PHY_CTRL6);
	RegData |= USB_PHY_CTRL6_RXTERM_OVERRIDE_SEL;
	writel(RegData, dwc3->base + USB_PHY_CTRL6);

	return 0;
}

void spl_board_init(void)
{
	if (is_usb_boot())
		puts("USB Boot\n");
	else
		puts("Normal Boot\n");
}

void board_boot_order(u32 *spl_boot_list)
{
	if (is_usb_boot())
		spl_boot_list[0] = BOOT_DEVICE_BOARD;
	else
		spl_boot_list[0] = BOOT_DEVICE_MMC1;
}

#define WDOG_PAD_CTRL  (PAD_CTL_DSE6 | PAD_CTL_HYS | PAD_CTL_PUE)

static const iomux_v3_cfg_t wdog_pads[] = {
	IMX8MQ_PAD_GPIO1_IO02__WDOG1_WDOG_B | MUX_PAD_CTRL(WDOG_PAD_CTRL),
};

void board_init_f(ulong dummy)
{
	int ret;
	struct wdog_regs *wdog = (struct wdog_regs *)WDOG1_BASE_ADDR;

	arch_cpu_init();

	imx_iomux_v3_setup_multiple_pads(wdog_pads, ARRAY_SIZE(wdog_pads));
	set_wdog_reset(wdog);

	init_uart_clk(CONSOLE_UART_CLK);
	imx_iomux_v3_setup_multiple_pads(uart_pads, ARRAY_SIZE(uart_pads));

#ifdef CONSOLE_ON_UART4
	gpio_request(WIFI_EN, "WIFI_EN");
	gpio_direction_output(WIFI_EN, 1);
#endif

	board_early_init_f();

	timer_init();

	preloader_console_init();

	ret = spl_init();
	if (ret) {
		log_err("spl_init() failed: %d\n", ret);
		hang();
	}

	enable_tzc380();

	printf("Initializing pinmux\n");
	init_pinmux();
	gpio_direction_output(LED_G, 1);
	gpio_direction_output(MOTO, 1);
	mdelay(50);
	gpio_direction_output(MOTO, 0);

	/* Enable and configure i2c buses not used below */
	setup_i2c(1, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info2);
	setup_i2c(2, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info3);
	setup_i2c(3, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info4);

	power_init_board();

	disable_charger_bq25895();

	/* initialize this for M4 even if u-boot doesn't have SF_CMD */
	printf("Initializing ECSPI\n");
	board_ecspi_init();

	/* DDR initialization */
	printf("Initializing DRAM\n");
	spl_dram_init();
}
