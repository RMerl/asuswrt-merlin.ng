// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 samtec automotive software & electronics gmbh
 * Copyright (C) 2017-2019 softing automotive electronics gmbH
 *
 * Author: Christoph Fritz <chf.fritz@googlemail.com>
 */

#include <asm/arch/clock.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/io.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <linux/sizes.h>
#include <common.h>
#include <environment.h>
#include <fsl_esdhc.h>
#include <mmc.h>
#include <i2c.h>
#include <miiphy.h>
#include <netdev.h>
#include <power/pmic.h>
#include <power/pfuze100_pmic.h>
#include <usb.h>
#include <usb/ehci-ci.h>
#include <pwm.h>
#include <wait_bit.h>

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL  (PAD_CTL_HYS | PAD_CTL_PUS_100K_UP |	\
	PAD_CTL_PKE | PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |	\
	PAD_CTL_SRE_FAST)

#define ENET_PAD_CTRL  (PAD_CTL_PUS_100K_UP | PAD_CTL_PKE |	\
	PAD_CTL_SPEED_HIGH | PAD_CTL_DSE_48ohm |		\
	PAD_CTL_SRE_FAST)

#define ENET_CLK_PAD_CTRL  PAD_CTL_DSE_34ohm

#define ENET_RX_PAD_CTRL  (PAD_CTL_PKE |			\
	PAD_CTL_PUS_100K_DOWN | PAD_CTL_SPEED_HIGH |		\
	PAD_CTL_SRE_FAST)

#define I2C_PAD_CTRL  (PAD_CTL_HYS | PAD_CTL_PUS_100K_UP |	\
	PAD_CTL_PKE | PAD_CTL_ODE | PAD_CTL_SPEED_MED |		\
	PAD_CTL_DSE_40ohm)

#define USDHC_CLK_PAD_CTRL  (PAD_CTL_HYS | PAD_CTL_SPEED_MED |	\
	PAD_CTL_DSE_80ohm | PAD_CTL_SRE_FAST)

#define USDHC_PAD_CTRL  (PAD_CTL_HYS | PAD_CTL_PUS_47K_UP |	\
	PAD_CTL_PKE |  PAD_CTL_SPEED_MED | PAD_CTL_DSE_80ohm |	\
	PAD_CTL_SRE_FAST)

#define USDHC_RESET_CTRL (PAD_CTL_HYS | PAD_CTL_PUS_47K_UP |	\
	PAD_CTL_PKE |  PAD_CTL_SPEED_MED | PAD_CTL_DSE_80ohm)

#define GPIO_PAD_CTRL  (PAD_CTL_HYS | PAD_CTL_PUS_100K_UP |	\
	PAD_CTL_PKE)

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();

	return 0;
}

static iomux_v3_cfg_t const fec1_pads[] = {
	MX6_PAD_ENET1_MDC__ENET1_MDC | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET1_MDIO__ENET1_MDIO | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII1_RD0__ENET1_RX_DATA_0 | MUX_PAD_CTRL(ENET_RX_PAD_CTRL),
	MX6_PAD_RGMII1_RD1__ENET1_RX_DATA_1 | MUX_PAD_CTRL(ENET_RX_PAD_CTRL),
	MX6_PAD_RGMII1_TD0__ENET1_TX_DATA_0 | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII1_TD1__ENET1_TX_DATA_1 | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII1_RX_CTL__ENET1_RX_EN | MUX_PAD_CTRL(ENET_RX_PAD_CTRL),
	MX6_PAD_RGMII1_TX_CTL__ENET1_TX_EN | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET1_TX_CLK__ENET1_REF_CLK1 | MUX_PAD_CTRL(ENET_CLK_PAD_CTRL) |
		MUX_MODE_SION,
	/* LAN8720 PHY Reset */
	MX6_PAD_RGMII1_TD3__GPIO5_IO_9 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static iomux_v3_cfg_t const pwm_led_pads[] = {
	MX6_PAD_RGMII2_RD2__PWM2_OUT | MUX_PAD_CTRL(NO_PAD_CTRL), /* green */
	MX6_PAD_RGMII2_TD2__PWM6_OUT | MUX_PAD_CTRL(NO_PAD_CTRL), /* red */
	MX6_PAD_RGMII2_RD3__PWM1_OUT | MUX_PAD_CTRL(NO_PAD_CTRL), /* blue */
};

#define PHY_RESET IMX_GPIO_NR(5, 9)

int board_eth_init(bd_t *bis)
{
	struct iomuxc *iomuxc_regs = (struct iomuxc *)IOMUXC_BASE_ADDR;
	int ret;
	unsigned char eth1addr[6];

	/* just to get secound mac address */
	imx_get_mac_from_fuse(1, eth1addr);
	if (!env_get("eth1addr") && is_valid_ethaddr(eth1addr))
		eth_env_set_enetaddr("eth1addr", eth1addr);

	imx_iomux_v3_setup_multiple_pads(fec1_pads, ARRAY_SIZE(fec1_pads));

	/*
	 * Generate phy reference clock via pin IOMUX ENET_REF_CLK1/2 by erasing
	 * ENET1/2_TX_CLK_DIR gpr1[14:13], so that reference clock is driven by
	 * ref_enetpll0/1 and enable ENET1/2_TX_CLK output driver.
	 */
	clrsetbits_le32(&iomuxc_regs->gpr[1],
			IOMUX_GPR1_FEC1_CLOCK_MUX2_SEL_MASK |
			IOMUX_GPR1_FEC2_CLOCK_MUX2_SEL_MASK,
			IOMUX_GPR1_FEC1_CLOCK_MUX1_SEL_MASK |
			IOMUX_GPR1_FEC2_CLOCK_MUX1_SEL_MASK);

	ret = enable_fec_anatop_clock(0, ENET_50MHZ);
	if (ret)
		goto eth_fail;

	/* reset phy */
	gpio_request(PHY_RESET, "PHY-reset");
	gpio_direction_output(PHY_RESET, 0);
	mdelay(16);
	gpio_set_value(PHY_RESET, 1);
	mdelay(1);

	ret = fecmxc_initialize_multi(bis, 0, CONFIG_FEC_MXC_PHYADDR,
					IMX_FEC_BASE);
	if (ret)
		goto eth_fail;

	return ret;

eth_fail:
	printf("FEC MXC: %s:failed (%i)\n", __func__, ret);
	gpio_set_value(PHY_RESET, 0);
	return ret;
}

#define PC MUX_PAD_CTRL(I2C_PAD_CTRL)
/* I2C1 for PMIC */
static struct i2c_pads_info i2c_pad_info1 = {
	.scl = {
		.i2c_mode = MX6_PAD_GPIO1_IO00__I2C1_SCL | PC,
		.gpio_mode = MX6_PAD_GPIO1_IO00__GPIO1_IO_0 | PC,
		.gp = IMX_GPIO_NR(1, 0),
	},
	.sda = {
		.i2c_mode = MX6_PAD_GPIO1_IO01__I2C1_SDA | PC,
		.gpio_mode = MX6_PAD_GPIO1_IO01__GPIO1_IO_1 | PC,
		.gp = IMX_GPIO_NR(1, 1),
	},
};

static struct pmic *pfuze_init(unsigned char i2cbus)
{
	struct pmic *p;
	int ret;
	u32 reg;

	ret = power_pfuze100_init(i2cbus);
	if (ret)
		return NULL;

	p = pmic_get("PFUZE100");
	ret = pmic_probe(p);
	if (ret)
		return NULL;

	pmic_reg_read(p, PFUZE100_DEVICEID, &reg);
	printf("PMIC:  PFUZE100 ID=0x%02x\n", reg);

	/* Set SW1AB stanby volage to 0.975V */
	pmic_reg_read(p, PFUZE100_SW1ABSTBY, &reg);
	reg &= ~SW1x_STBY_MASK;
	reg |= SW1x_0_975V;
	pmic_reg_write(p, PFUZE100_SW1ABSTBY, reg);

	/* Set SW1AB/VDDARM step ramp up time from 16us to 4us/25mV */
	pmic_reg_read(p, PFUZE100_SW1ABCONF, &reg);
	reg &= ~SW1xCONF_DVSSPEED_MASK;
	reg |= SW1xCONF_DVSSPEED_4US;
	pmic_reg_write(p, PFUZE100_SW1ABCONF, reg);

	/* Set SW1C standby voltage to 0.975V */
	pmic_reg_read(p, PFUZE100_SW1CSTBY, &reg);
	reg &= ~SW1x_STBY_MASK;
	reg |= SW1x_0_975V;
	pmic_reg_write(p, PFUZE100_SW1CSTBY, reg);

	/* Set SW1C/VDDSOC step ramp up time from 16us to 4us/25mV */
	pmic_reg_read(p, PFUZE100_SW1CCONF, &reg);
	reg &= ~SW1xCONF_DVSSPEED_MASK;
	reg |= SW1xCONF_DVSSPEED_4US;
	pmic_reg_write(p, PFUZE100_SW1CCONF, reg);

	return p;
}

static int pfuze_mode_init(struct pmic *p, u32 mode)
{
	unsigned char offset, i, switch_num;
	u32 id;
	int ret;

	pmic_reg_read(p, PFUZE100_DEVICEID, &id);
	id = id & 0xf;

	if (id == 0) {
		switch_num = 6;
		offset = PFUZE100_SW1CMODE;
	} else if (id == 1) {
		switch_num = 4;
		offset = PFUZE100_SW2MODE;
	} else {
		printf("Not supported, id=%d\n", id);
		return -EINVAL;
	}

	ret = pmic_reg_write(p, PFUZE100_SW1ABMODE, mode);
	if (ret < 0) {
		printf("Set SW1AB mode error!\n");
		return ret;
	}

	for (i = 0; i < switch_num - 1; i++) {
		ret = pmic_reg_write(p, offset + i * SWITCH_SIZE, mode);
		if (ret < 0) {
			printf("Set switch 0x%x mode error!\n",
			       offset + i * SWITCH_SIZE);
			return ret;
		}
	}

	return ret;
}

int power_init_board(void)
{
	struct pmic *p;
	int ret;

	p = pfuze_init(I2C_PMIC);
	if (!p)
		return -ENODEV;

	ret = pfuze_mode_init(p, APS_PFM);
	if (ret < 0)
		return ret;

	return 0;
}

#ifdef CONFIG_USB_EHCI_MX6
static iomux_v3_cfg_t const usb_otg_pads[] = {
	/* OGT1 */
	MX6_PAD_GPIO1_IO09__USB_OTG1_PWR | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_GPIO1_IO10__ANATOP_OTG1_ID | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* OTG2 */
	MX6_PAD_GPIO1_IO12__USB_OTG2_PWR | MUX_PAD_CTRL(NO_PAD_CTRL)
};

static void setup_iomux_usb(void)
{
	imx_iomux_v3_setup_multiple_pads(usb_otg_pads,
					 ARRAY_SIZE(usb_otg_pads));
}

int board_usb_phy_mode(int port)
{
	if (port == 1)
		return USB_INIT_HOST;
	else
		return usb_phy_mode(port);
}
#endif

#ifdef CONFIG_PWM_IMX
static int set_pwm_leds(void)
{
	int ret;

	imx_iomux_v3_setup_multiple_pads(pwm_led_pads,
					 ARRAY_SIZE(pwm_led_pads));
	/* enable backlight PWM 2, green LED */
	ret = pwm_init(1, 0, 0);
	if (ret)
		goto error;
	/* duty cycle 200ns, period: 8000ns */
	ret = pwm_config(1, 200, 8000);
	if (ret)
		goto error;
	ret = pwm_enable(1);
	if (ret)
		goto error;

	/* enable backlight PWM 1, blue LED */
	ret = pwm_init(0, 0, 0);
	if (ret)
		goto error;
	/* duty cycle 200ns, period: 8000ns */
	ret = pwm_config(0, 200, 8000);
	if (ret)
		goto error;
	ret = pwm_enable(0);
	if (ret)
		goto error;

	/* enable backlight PWM 6, red LED */
	ret = pwm_init(5, 0, 0);
	if (ret)
		goto error;
	/* duty cycle 200ns, period: 8000ns */
	ret = pwm_config(5, 200, 8000);
	if (ret)
		goto error;
	ret = pwm_enable(5);

error:
	return ret;
}
#else
static int set_pwm_leds(void)
{
	return 0;
}
#endif

#define ADCx_HC0        0x00
#define ADCx_HS         0x08
#define ADCx_HS_C0      BIT(0)
#define ADCx_R0         0x0c
#define ADCx_CFG        0x14
#define ADCx_CFG_SWMODE 0x308
#define ADCx_GC         0x18
#define ADCx_GC_CAL     BIT(7)

static int read_adc(u32 *val)
{
	int ret;
	void __iomem *b = map_physmem(ADC1_BASE_ADDR, 0x100, MAP_NOCACHE);

	/* use software mode */
	writel(ADCx_CFG_SWMODE, b + ADCx_CFG);

	/* start auto calibration */
	setbits_le32(b + ADCx_GC, ADCx_GC_CAL);
	ret = wait_for_bit_le32(b + ADCx_GC, ADCx_GC_CAL, ADCx_GC_CAL, 10, 0);
	if (ret)
		goto adc_exit;

	/* start conversion */
	writel(0, b + ADCx_HC0);

	/* wait for conversion */
	ret = wait_for_bit_le32(b + ADCx_HS, ADCx_HS_C0, ADCx_HS_C0, 10, 0);
	if (ret)
		goto adc_exit;

	/* read result */
	*val = readl(b + ADCx_R0);

adc_exit:
	if (ret)
		printf("ADC failure (ret=%i)\n", ret);
	unmap_physmem(b, MAP_NOCACHE);
	return ret;
}

#define VAL_UPPER	2498
#define VAL_LOWER	1550

static int set_pin_state(void)
{
	u32 val;
	int ret;

	ret = read_adc(&val);
	if (ret)
		return ret;

	if (val >= VAL_UPPER)
		env_set("pin_state", "connected");
	else if (val < VAL_UPPER && val > VAL_LOWER)
		env_set("pin_state", "open");
	else
		env_set("pin_state", "button");

	return ret;
}

int board_late_init(void)
{
	int ret;

	ret = set_pwm_leds();
	if (ret)
		return ret;

	ret = set_pin_state();

	return ret;
}

int board_early_init_f(void)
{
	setup_iomux_usb();

	return 0;
}

int board_init(void)
{
	/* Address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

#ifdef CONFIG_SYS_I2C_MXC
	setup_i2c(0, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info1);
#endif

	return 0;
}

int checkboard(void)
{
	puts("Board: VIN|ING 2000\n");

	return 0;
}
