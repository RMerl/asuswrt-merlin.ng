// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 MediaTek Inc. All Rights Reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#include <common.h>
#include <dm.h>
#include <asm/global_data.h>
#include <dm/pinctrl.h>
#include <linux/bitops.h>
#include <linux/io.h>

#include "pinctrl-mtmips-common.h"

DECLARE_GLOBAL_DATA_PTR;

#define AGPIO_OFS			0
#define GPIOMODE1_OFS			0x24
#define GPIOMODE2_OFS			0x28

#define EPHY4_1_PAD_SHIFT		17
#define EPHY4_1_PAD_MASK		0x0f
#define EPHY0_SHIFT			16
#define RF_OLT_MODE_SHIFT		12
#define N9_EINT_SRC_SHIFT		9
#define WLED_OD_SHIFT			8
#define REF_CLKO_PAD_SHIFT		4
#define I2S_CLK_PAD_SHIFT		3
#define I2S_WS_PAD_SHIFT		2
#define I2S_SDO_PAD_SHIFT		1
#define I2S_SDI_PAD_SHIFT		0

#define GM4_MASK			3

#define P4LED_K_SHIFT			26
#define P3LED_K_SHIFT			24
#define P2LED_K_SHIFT			22
#define P1LED_K_SHIFT			20
#define P0LED_K_SHIFT			18
#define WLED_K_SHIFT			16
#define P4LED_A_SHIFT			10
#define P3LED_A_SHIFT			8
#define P2LED_A_SHIFT			6
#define P1LED_A_SHIFT			4
#define P0LED_A_SHIFT			2
#define WLED_A_SHIFT			0

#define PWM1_SHIFT			30
#define PWM0_SHIFT			28
#define UART2_SHIFT			26
#define UART1_SHIFT			24
#define I2C_SHIFT			20
#define REFCLK_SHIFT			18
#define PERST_SHIFT			16
#define ESD_SHIFT			15
#define WDT_SHIFT			14
#define SPI_SHIFT			12
#define SDMODE_SHIFT			10
#define UART0_SHIFT			8
#define I2S_SHIFT			6
#define SPI_CS1_SHIFT			4
#define SPIS_SHIFT			2
#define GPIO0_SHIFT			0

#define PAD_PU_G0_REG			0x00
#define PAD_PU_G1_REG			0x04
#define PAD_PD_G0_REG			0x10
#define PAD_PD_G1_REG			0x14
#define PAD_SR_G0_REG			0x20
#define PAD_SR_G1_REG			0x24
#define PAD_SMT_G0_REG			0x30
#define PAD_SMT_G1_REG			0x34
#define PAD_E2_G0_REG			0x40
#define PAD_E2_G1_REG			0x44
#define PAD_E4_G0_REG			0x50
#define PAD_E4_G1_REG			0x54
#define PAD_E8_G0_REG			0x60
#define PAD_E8_G1_REG			0x64

#define PIN_CONFIG_DRIVE_STRENGTH_28	(PIN_CONFIG_END + 1)
#define PIN_CONFIG_DRIVE_STRENGTH_4G	(PIN_CONFIG_END + 2)

struct mt7628_pinctrl_priv {
	struct mtmips_pinctrl_priv mp;

	void __iomem *pcbase;
};

#if CONFIG_IS_ENABLED(PINMUX)
static const struct mtmips_pmx_func ephy4_1_pad_grp[] = {
	FUNC("digital", 0xf),
	FUNC("analog", 0),
};

static const struct mtmips_pmx_func ephy0_grp[] = {
	FUNC("disable", 1),
	FUNC("enable", 0),
};

static const struct mtmips_pmx_func rf_olt_grp[] = {
	FUNC("enable", 1),
	FUNC("disable", 0),
};

static const struct mtmips_pmx_func n9_eint_src_grp[] = {
	FUNC("gpio", 1),
	FUNC("utif", 0),
};

static const struct mtmips_pmx_func wlen_od_grp[] = {
	FUNC("enable", 1),
	FUNC("disable", 0),
};

static const struct mtmips_pmx_func ref_clko_grp[] = {
	FUNC("digital", 1),
	FUNC("analog", 0),
};

static const struct mtmips_pmx_func i2s_clk_grp[] = {
	FUNC("digital", 1),
	FUNC("analog", 0),
};

static const struct mtmips_pmx_func i2s_ws_grp[] = {
	FUNC("digital", 1),
	FUNC("analog", 0),
};

static const struct mtmips_pmx_func i2s_sdo_grp[] = {
	FUNC("digital", 1),
	FUNC("analog", 0),
};

static const struct mtmips_pmx_func i2s_sdi_grp[] = {
	FUNC("digital", 1),
	FUNC("analog", 0),
};

static const struct mtmips_pmx_func pwm1_grp[] = {
	FUNC("sdxc d6", 3),
	FUNC("utif", 2),
	FUNC("gpio", 1),
	FUNC("pwm1", 0),
};

static const struct mtmips_pmx_func pwm0_grp[] = {
	FUNC("sdxc d7", 3),
	FUNC("utif", 2),
	FUNC("gpio", 1),
	FUNC("pwm0", 0),
};

static const struct mtmips_pmx_func uart2_grp[] = {
	FUNC("sdxc d5 d4", 3),
	FUNC("pwm", 2),
	FUNC("gpio", 1),
	FUNC("uart2", 0),
};

static const struct mtmips_pmx_func uart1_grp[] = {
	FUNC("sw_r", 3),
	FUNC("pwm", 2),
	FUNC("gpio", 1),
	FUNC("uart1", 0),
};

static const struct mtmips_pmx_func i2c_grp[] = {
	FUNC("-", 3),
	FUNC("debug", 2),
	FUNC("gpio", 1),
	FUNC("i2c", 0),
};

static const struct mtmips_pmx_func refclk_grp[] = {
	FUNC("gpio", 1),
	FUNC("refclk", 0),
};

static const struct mtmips_pmx_func perst_grp[] = {
	FUNC("gpio", 1),
	FUNC("perst", 0),
};

static const struct mtmips_pmx_func esd_grp[] = {
	FUNC("router", 1),
	FUNC("iot", 0),
};

static const struct mtmips_pmx_func wdt_grp[] = {
	FUNC("gpio", 1),
	FUNC("wdt", 0),
};

static const struct mtmips_pmx_func spi_grp[] = {
	FUNC("gpio", 1),
	FUNC("spi", 0),
};

static const struct mtmips_pmx_func sd_mode_grp[] = {
	FUNC("n9 jtag", 3),
	FUNC("utif1", 2),
	FUNC("gpio", 1),
	FUNC("sdxc", 0),
};

static const struct mtmips_pmx_func uart0_grp[] = {
	FUNC("-", 3),
	FUNC("-", 2),
	FUNC("gpio", 1),
	FUNC("uart0", 0),
};

static const struct mtmips_pmx_func i2s_grp[] = {
	FUNC("antenna", 3),
	FUNC("pcm", 2),
	FUNC("gpio", 1),
	FUNC("i2s", 0),
};

static const struct mtmips_pmx_func spi_cs1_grp[] = {
	FUNC("-", 3),
	FUNC("refclk", 2),
	FUNC("gpio", 1),
	FUNC("spi cs1", 0),
};

static const struct mtmips_pmx_func spis_grp[] = {
	FUNC("pwm_uart2", 3),
	FUNC("utif", 2),
	FUNC("gpio", 1),
	FUNC("spis", 0),
};

static const struct mtmips_pmx_func gpio0_grp[] = {
	FUNC("perst", 3),
	FUNC("refclk", 2),
	FUNC("gpio", 1),
	FUNC("gpio0", 0),
};

static const struct mtmips_pmx_func wled_a_grp[] = {
	FUNC("-", 3),
	FUNC("-", 2),
	FUNC("gpio", 1),
	FUNC("led", 0),
};

static const struct mtmips_pmx_func p0led_a_grp[] = {
	FUNC("jtag", 3),
	FUNC("rsvd", 2),
	FUNC("gpio", 1),
	FUNC("led", 0),
};

static const struct mtmips_pmx_func p1led_a_grp[] = {
	FUNC("jtag", 3),
	FUNC("utif", 2),
	FUNC("gpio", 1),
	FUNC("led", 0),
};

static const struct mtmips_pmx_func p2led_a_grp[] = {
	FUNC("jtag", 3),
	FUNC("utif", 2),
	FUNC("gpio", 1),
	FUNC("led", 0),
};

static const struct mtmips_pmx_func p3led_a_grp[] = {
	FUNC("jtag", 3),
	FUNC("utif", 2),
	FUNC("gpio", 1),
	FUNC("led", 0),
};

static const struct mtmips_pmx_func p4led_a_grp[] = {
	FUNC("jtag", 3),
	FUNC("utif", 2),
	FUNC("gpio", 1),
	FUNC("led", 0),
};

static const struct mtmips_pmx_func wled_k_grp[] = {
	FUNC("-", 3),
	FUNC("-", 2),
	FUNC("gpio", 1),
	FUNC("led", 0),
};

static const struct mtmips_pmx_func p0led_k_grp[] = {
	FUNC("jtag", 3),
	FUNC("rsvd", 2),
	FUNC("gpio", 1),
	FUNC("led", 0),
};

static const struct mtmips_pmx_func p1led_k_grp[] = {
	FUNC("jtag", 3),
	FUNC("utif", 2),
	FUNC("gpio", 1),
	FUNC("led", 0),
};

static const struct mtmips_pmx_func p2led_k_grp[] = {
	FUNC("jtag", 3),
	FUNC("utif", 2),
	FUNC("gpio", 1),
	FUNC("led", 0),
};

static const struct mtmips_pmx_func p3led_k_grp[] = {
	FUNC("jtag", 3),
	FUNC("utif", 2),
	FUNC("gpio", 1),
	FUNC("led", 0),
};

static const struct mtmips_pmx_func p4led_k_grp[] = {
	FUNC("jtag", 3),
	FUNC("utif", 2),
	FUNC("gpio", 1),
	FUNC("led", 0),
};

static const struct mtmips_pmx_group mt7628_pinmux_data[] = {
	GRP("ephy4_1_pad", ephy4_1_pad_grp, AGPIO_OFS, EPHY4_1_PAD_SHIFT,
	    EPHY4_1_PAD_MASK),
	GRP("ephy0", ephy0_grp, AGPIO_OFS, EPHY0_SHIFT, 1),
	GRP("rf_olt", rf_olt_grp, AGPIO_OFS, RF_OLT_MODE_SHIFT, 1),
	GRP("n9_eint_src", n9_eint_src_grp, AGPIO_OFS, N9_EINT_SRC_SHIFT, 1),
	GRP("wlen_od", wlen_od_grp, AGPIO_OFS, WLED_OD_SHIFT, 1),
	GRP("ref_clko_pad", ref_clko_grp, AGPIO_OFS, REF_CLKO_PAD_SHIFT, 1),
	GRP("i2s_clk_pad", i2s_clk_grp, AGPIO_OFS, I2S_CLK_PAD_SHIFT, 1),
	GRP("i2s_ws_pad", i2s_ws_grp, AGPIO_OFS, I2S_WS_PAD_SHIFT, 1),
	GRP("i2s_sdo_pad", i2s_sdo_grp, AGPIO_OFS, I2S_SDO_PAD_SHIFT, 1),
	GRP("i2s_sdi_pad", i2s_sdi_grp, AGPIO_OFS, I2S_SDI_PAD_SHIFT, 1),
	GRP("pwm1", pwm1_grp, GPIOMODE1_OFS, PWM1_SHIFT, GM4_MASK),
	GRP("pwm0", pwm0_grp, GPIOMODE1_OFS, PWM0_SHIFT, GM4_MASK),
	GRP("uart2", uart2_grp, GPIOMODE1_OFS, UART2_SHIFT, GM4_MASK),
	GRP("uart1", uart1_grp, GPIOMODE1_OFS, UART1_SHIFT, GM4_MASK),
	GRP("i2c", i2c_grp, GPIOMODE1_OFS, I2C_SHIFT, GM4_MASK),
	GRP("refclk", refclk_grp, GPIOMODE1_OFS, REFCLK_SHIFT, 1),
	GRP("perst", perst_grp, GPIOMODE1_OFS, PERST_SHIFT, 1),
	GRP("sd router", esd_grp, GPIOMODE1_OFS, ESD_SHIFT, 1),
	GRP("wdt", wdt_grp, GPIOMODE1_OFS, WDT_SHIFT, 1),
	GRP("spi", spi_grp, GPIOMODE1_OFS, SPI_SHIFT, 1),
	GRP("sdmode", sd_mode_grp, GPIOMODE1_OFS, SDMODE_SHIFT, GM4_MASK),
	GRP("uart0", uart0_grp, GPIOMODE1_OFS, UART0_SHIFT, GM4_MASK),
	GRP("i2s", i2s_grp, GPIOMODE1_OFS, I2S_SHIFT, GM4_MASK),
	GRP("spi cs1", spi_cs1_grp, GPIOMODE1_OFS, SPI_CS1_SHIFT, GM4_MASK),
	GRP("spis", spis_grp, GPIOMODE1_OFS, SPIS_SHIFT, GM4_MASK),
	GRP("gpio0", gpio0_grp, GPIOMODE1_OFS, GPIO0_SHIFT, GM4_MASK),
	GRP("wled_a", wled_a_grp, GPIOMODE2_OFS, WLED_A_SHIFT, GM4_MASK),
	GRP("p0led_a", p0led_a_grp, GPIOMODE2_OFS, P0LED_A_SHIFT, GM4_MASK),
	GRP("p1led_a", p1led_a_grp, GPIOMODE2_OFS, P1LED_A_SHIFT, GM4_MASK),
	GRP("p2led_a", p2led_a_grp, GPIOMODE2_OFS, P2LED_A_SHIFT, GM4_MASK),
	GRP("p3led_a", p3led_a_grp, GPIOMODE2_OFS, P3LED_A_SHIFT, GM4_MASK),
	GRP("p4led_a", p4led_a_grp, GPIOMODE2_OFS, P4LED_A_SHIFT, GM4_MASK),
	GRP("wled_k", wled_k_grp, GPIOMODE2_OFS, WLED_K_SHIFT, GM4_MASK),
	GRP("p0led_k", p0led_k_grp, GPIOMODE2_OFS, P0LED_K_SHIFT, GM4_MASK),
	GRP("p1led_k", p1led_k_grp, GPIOMODE2_OFS, P1LED_K_SHIFT, GM4_MASK),
	GRP("p2led_k", p2led_k_grp, GPIOMODE2_OFS, P2LED_K_SHIFT, GM4_MASK),
	GRP("p3led_k", p3led_k_grp, GPIOMODE2_OFS, P3LED_K_SHIFT, GM4_MASK),
	GRP("p4led_k", p4led_k_grp, GPIOMODE2_OFS, P4LED_K_SHIFT, GM4_MASK),
};

static int mt7628_get_groups_count(struct udevice *dev)
{
	return ARRAY_SIZE(mt7628_pinmux_data);
}

static const char *mt7628_get_group_name(struct udevice *dev,
					 unsigned int selector)
{
	return mt7628_pinmux_data[selector].name;
}
#endif /* CONFIG_IS_ENABLED(PINMUX) */

#if CONFIG_IS_ENABLED(PINCONF)
static const struct pinconf_param mt7628_conf_params[] = {
	{ "bias-disable", PIN_CONFIG_BIAS_DISABLE, 0 },
	{ "bias-pull-up", PIN_CONFIG_BIAS_PULL_UP, 1 },
	{ "bias-pull-down", PIN_CONFIG_BIAS_PULL_DOWN, 1 },
	{ "input-schmitt-enable", PIN_CONFIG_INPUT_SCHMITT_ENABLE, 1 },
	{ "input-schmitt-disable", PIN_CONFIG_INPUT_SCHMITT_ENABLE, 0 },
	{ "drive-strength-28", PIN_CONFIG_DRIVE_STRENGTH_28, 0 },
	{ "drive-strength-4g", PIN_CONFIG_DRIVE_STRENGTH_4G, 0 },
	{ "slew-rate", PIN_CONFIG_SLEW_RATE, 0 },
};

static const char *const mt7628_pins[] = {
	"i2s_sdi",
	"i2s_sdo",
	"i2s_ws",
	"i2s_clk",
	"i2s_sclk",
	"i2c_sd",
	"spi_cs1",
	"spi_clk",
	"spi_mosi",
	"spi_miso",
	"spi_cs0",
	"gpio0",
	"uart0_txd",
	"uart0_rxd",
	"spis_cs",
	"spis_clk",
	"spis_miso",
	"spis_mosi",
	"pwm_ch0",
	"pwm_ch1",
	"uart2_txd",
	"uart2_rxd",
	"sd_wp",
	"sd_cd",
	"sd_d1",
	"sd_d0",
	"sd_clk",
	"sd_cmd",
	"sd_d3",
	"sd_d2",
	"ephy_led4_k",
	"ephy_led3_k",
	"ephy_led2_k",
	"ephy_led1_k",
	"ephy_led0_k",
	"wled_k",
	"perst_n",
	"co_clko",
	"wdt",
	"ephy_led4_a",
	"ephy_led3_a",
	"ephy_led2_a",
	"ephy_led1_a",
	"ephy_led0_a",
	"wled_a",
	"uart1_txd",
	"uart1_rxd",
};

static const u32 mt7628_drv_strength_28_tbl[] = {2, 4, 6, 8};
static const u32 mt7628_drv_strength_4g_tbl[] = {4, 8, 12, 16};

static int mt7628_set_drv_strength(void __iomem *base, u32 val, u32 bit,
				   const u32 tbl[], u32 reg_lo, u32 reg_hi)
{
	int i;

	for (i = 0; i < 4; i++)
		if (tbl[i] == val)
			break;

	if (i >= 4)
		return -EINVAL;

	clrsetbits_32(base + reg_lo, BIT(bit), (i & 1) << bit);
	clrsetbits_32(base + reg_hi, BIT(bit), ((i >> 1) & 1) << bit);

	return 0;
}

static int mt7628_get_pins_count(struct udevice *dev)
{
	return ARRAY_SIZE(mt7628_pins);
}

static const char *mt7628_get_pin_name(struct udevice *dev,
				       unsigned int selector)
{
	return mt7628_pins[selector];
}

static int mt7628_pinconf_set(struct udevice *dev, unsigned int pin_selector,
			      unsigned int param, unsigned int argument)
{
	struct mt7628_pinctrl_priv *priv = dev_get_priv(dev);
	u32 offs, bit;
	int ret = 0;

	offs = (pin_selector / 32) * 4;
	bit = pin_selector % 32;

	switch (param) {
	case PIN_CONFIG_BIAS_DISABLE:
		clrbits_32(priv->pcbase + offs + PAD_PU_G0_REG, BIT(bit));
		clrbits_32(priv->pcbase + offs + PAD_PD_G0_REG, BIT(bit));
		break;
	case PIN_CONFIG_BIAS_PULL_UP:
		setbits_32(priv->pcbase + offs + PAD_PU_G0_REG, BIT(bit));
		clrbits_32(priv->pcbase + offs + PAD_PD_G0_REG, BIT(bit));
		break;
	case PIN_CONFIG_BIAS_PULL_DOWN:
		clrbits_32(priv->pcbase + offs + PAD_PU_G0_REG, BIT(bit));
		setbits_32(priv->pcbase + offs + PAD_PD_G0_REG, BIT(bit));
		break;
	case PIN_CONFIG_INPUT_SCHMITT_ENABLE:
		clrsetbits_32(priv->pcbase + offs + PAD_SMT_G0_REG,
			      BIT(bit), (!!argument) << bit);
		break;
	case PIN_CONFIG_DRIVE_STRENGTH_28:
		ret = mt7628_set_drv_strength(priv->pcbase + offs, argument,
					      bit, mt7628_drv_strength_28_tbl,
					      PAD_E2_G0_REG, PAD_E4_G0_REG);
		break;
	case PIN_CONFIG_DRIVE_STRENGTH_4G:
		ret = mt7628_set_drv_strength(priv->pcbase + offs, argument,
					      bit, mt7628_drv_strength_4g_tbl,
					      PAD_E4_G0_REG, PAD_E8_G0_REG);
		break;
	case PIN_CONFIG_SLEW_RATE:
		clrsetbits_32(priv->pcbase + offs + PAD_SR_G0_REG,
			      BIT(bit), (!!argument) << bit);
		break;
	default:
		ret = -EINVAL;
	}

	return ret;
}
#endif

static int mt7628_pinctrl_probe(struct udevice *dev)
{
	struct mt7628_pinctrl_priv *priv = dev_get_priv(dev);
	int ret = 0;

#if CONFIG_IS_ENABLED(PINMUX)
	ret = mtmips_pinctrl_probe(&priv->mp, ARRAY_SIZE(mt7628_pinmux_data),
				   mt7628_pinmux_data);
#endif /* CONFIG_IS_ENABLED(PINMUX) */

	return ret;
}

static int mt7628_pinctrl_of_to_plat(struct udevice *dev)
{
	struct mt7628_pinctrl_priv *priv = dev_get_priv(dev);

	priv->mp.base = (void __iomem *)dev_remap_addr_index(dev, 0);

	if (!priv->mp.base)
		return -EINVAL;

	priv->pcbase = (void __iomem *)dev_remap_addr_index(dev, 1);

	if (!priv->pcbase)
		return -EINVAL;

	return 0;
}

static const struct pinctrl_ops mt7628_pinctrl_ops = {
#if CONFIG_IS_ENABLED(PINMUX)
	.get_groups_count = mt7628_get_groups_count,
	.get_group_name = mt7628_get_group_name,
	.get_functions_count = mtmips_get_functions_count,
	.get_function_name = mtmips_get_function_name,
	.pinmux_group_set = mtmips_pinmux_group_set,
#endif /* CONFIG_IS_ENABLED(PINMUX) */
#if CONFIG_IS_ENABLED(PINCONF)
	.pinconf_num_params = ARRAY_SIZE(mt7628_conf_params),
	.pinconf_params = mt7628_conf_params,
	.get_pins_count = mt7628_get_pins_count,
	.get_pin_name = mt7628_get_pin_name,
	.pinconf_set = mt7628_pinconf_set,
#endif /* CONFIG_IS_ENABLED(PINCONF) */
	.set_state = pinctrl_generic_set_state,
};

static const struct udevice_id mt7628_pinctrl_ids[] = {
	{ .compatible = "mediatek,mt7628-pinctrl" },
	{ }
};

U_BOOT_DRIVER(mt7628_pinctrl) = {
	.name = "mt7628-pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = mt7628_pinctrl_ids,
	.of_to_plat = mt7628_pinctrl_of_to_plat,
	.ops = &mt7628_pinctrl_ops,
	.probe = mt7628_pinctrl_probe,
	.priv_auto	= sizeof(struct mt7628_pinctrl_priv),
};
