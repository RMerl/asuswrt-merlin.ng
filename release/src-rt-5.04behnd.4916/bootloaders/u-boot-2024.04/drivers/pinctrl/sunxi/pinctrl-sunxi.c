// SPDX-License-Identifier: GPL-2.0

#include <clk.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/pinctrl.h>
#include <errno.h>
#include <malloc.h>
#include <sunxi_gpio.h>

#include <asm/gpio.h>

extern U_BOOT_DRIVER(gpio_sunxi);

/*
 * This structure implements a simplified view of the possible pinmux settings:
 * Each mux value is assumed to be the same for a given function, across the
 * pins in each group (almost universally true, with same rare exceptions not
 * relevant to U-Boot), but also across different ports (not true in many
 * cases). We ignore the first problem, and work around the latter by just
 * supporting one particular port for a each function. This works fine for all
 * board configurations so far. If this would need to be revisited, we could
 * add a "u8 port;" below and match that, with 0 encoding the "don't care" case.
 */
struct sunxi_pinctrl_function {
	const char	name[sizeof("gpio_out")];
	u8		mux;
};

struct sunxi_pinctrl_desc {
	const struct sunxi_pinctrl_function	*functions;
	u8					num_functions;
	u8					first_bank;
	u8					num_banks;
};

struct sunxi_pinctrl_plat {
	void __iomem *base;
};

static int sunxi_pinctrl_get_pins_count(struct udevice *dev)
{
	const struct sunxi_pinctrl_desc *desc = dev_get_priv(dev);

	return desc->num_banks * SUNXI_GPIOS_PER_BANK;
}

static const char *sunxi_pinctrl_get_pin_name(struct udevice *dev,
					      uint pin_selector)
{
	const struct sunxi_pinctrl_desc *desc = dev_get_priv(dev);
	static char pin_name[sizeof("PN31")] __section(".data");

	snprintf(pin_name, sizeof(pin_name), "P%c%d",
		 pin_selector / SUNXI_GPIOS_PER_BANK + desc->first_bank + 'A',
		 pin_selector % SUNXI_GPIOS_PER_BANK);

	return pin_name;
}

static int sunxi_pinctrl_get_functions_count(struct udevice *dev)
{
	const struct sunxi_pinctrl_desc *desc = dev_get_priv(dev);

	return desc->num_functions;
}

static const char *sunxi_pinctrl_get_function_name(struct udevice *dev,
						   uint func_selector)
{
	const struct sunxi_pinctrl_desc *desc = dev_get_priv(dev);

	return desc->functions[func_selector].name;
}

static int sunxi_pinctrl_pinmux_set(struct udevice *dev, uint pin_selector,
				    uint func_selector)
{
	const struct sunxi_pinctrl_desc *desc = dev_get_priv(dev);
	struct sunxi_pinctrl_plat *plat = dev_get_plat(dev);
	int bank = pin_selector / SUNXI_GPIOS_PER_BANK;
	int pin	 = pin_selector % SUNXI_GPIOS_PER_BANK;

	debug("set mux: %-4s => %s (%d)\n",
	      sunxi_pinctrl_get_pin_name(dev, pin_selector),
	      sunxi_pinctrl_get_function_name(dev, func_selector),
	      desc->functions[func_selector].mux);

	sunxi_gpio_set_cfgbank(plat->base + bank * SUNXI_PINCTRL_BANK_SIZE,
			       pin, desc->functions[func_selector].mux);

	return 0;
}

static const struct pinconf_param sunxi_pinctrl_pinconf_params[] = {
	{ "bias-disable",	PIN_CONFIG_BIAS_DISABLE,	 0 },
	{ "bias-pull-down",	PIN_CONFIG_BIAS_PULL_DOWN,	 2 },
	{ "bias-pull-up",	PIN_CONFIG_BIAS_PULL_UP,	 1 },
	{ "drive-strength",	PIN_CONFIG_DRIVE_STRENGTH,	10 },
};

static int sunxi_pinctrl_pinconf_set_pull(struct sunxi_pinctrl_plat *plat,
					  uint bank, uint pin, uint bias)
{
	void *regs = plat->base + bank * SUNXI_PINCTRL_BANK_SIZE;

	sunxi_gpio_set_pull_bank(regs, pin, bias);

	return 0;
}

static int sunxi_pinctrl_pinconf_set_drive(struct sunxi_pinctrl_plat *plat,
					   uint bank, uint pin, uint drive)
{
	void *regs = plat->base + bank * SUNXI_PINCTRL_BANK_SIZE;

	if (drive < 10 || drive > 40)
		return -EINVAL;

	/* Convert mA to the register value, rounding down. */
	sunxi_gpio_set_drv_bank(regs, pin, drive / 10 - 1);

	return 0;
}

static int sunxi_pinctrl_pinconf_set(struct udevice *dev, uint pin_selector,
				     uint param, uint val)
{
	struct sunxi_pinctrl_plat *plat = dev_get_plat(dev);
	int bank = pin_selector / SUNXI_GPIOS_PER_BANK;
	int pin  = pin_selector % SUNXI_GPIOS_PER_BANK;

	switch (param) {
	case PIN_CONFIG_BIAS_DISABLE:
	case PIN_CONFIG_BIAS_PULL_DOWN:
	case PIN_CONFIG_BIAS_PULL_UP:
		return sunxi_pinctrl_pinconf_set_pull(plat, bank, pin, val);
	case PIN_CONFIG_DRIVE_STRENGTH:
		return sunxi_pinctrl_pinconf_set_drive(plat, bank, pin, val);
	}

	return -EINVAL;
}

static int sunxi_pinctrl_get_pin_muxing(struct udevice *dev, uint pin_selector,
					char *buf, int size)
{
	struct sunxi_pinctrl_plat *plat = dev_get_plat(dev);
	int bank = pin_selector / SUNXI_GPIOS_PER_BANK;
	int pin	 = pin_selector % SUNXI_GPIOS_PER_BANK;
	int mux  = sunxi_gpio_get_cfgbank(plat->base + bank * SUNXI_PINCTRL_BANK_SIZE, pin);

	switch (mux) {
	case SUNXI_GPIO_INPUT:
		strlcpy(buf, "gpio input", size);
		break;
	case SUNXI_GPIO_OUTPUT:
		strlcpy(buf, "gpio output", size);
		break;
	case SUNXI_GPIO_DISABLE:
		strlcpy(buf, "disabled", size);
		break;
	default:
		snprintf(buf, size, "function %d", mux);
		break;
	}

	return 0;
}

static const struct pinctrl_ops sunxi_pinctrl_ops = {
	.get_pins_count		= sunxi_pinctrl_get_pins_count,
	.get_pin_name		= sunxi_pinctrl_get_pin_name,
	.get_functions_count	= sunxi_pinctrl_get_functions_count,
	.get_function_name	= sunxi_pinctrl_get_function_name,
	.pinmux_set		= sunxi_pinctrl_pinmux_set,
	.pinconf_num_params	= ARRAY_SIZE(sunxi_pinctrl_pinconf_params),
	.pinconf_params		= sunxi_pinctrl_pinconf_params,
	.pinconf_set		= sunxi_pinctrl_pinconf_set,
	.set_state		= pinctrl_generic_set_state,
	.get_pin_muxing		= sunxi_pinctrl_get_pin_muxing,
};

static int sunxi_pinctrl_bind(struct udevice *dev)
{
	struct sunxi_pinctrl_plat *plat = dev_get_plat(dev);
	struct sunxi_pinctrl_desc *desc;
	struct sunxi_gpio_plat *gpio_plat;
	struct udevice *gpio_dev;
	int i, ret;

	desc = (void *)dev_get_driver_data(dev);
	if (!desc)
		return -EINVAL;
	dev_set_priv(dev, desc);

	plat->base = dev_read_addr_ptr(dev);

	ret = device_bind_driver_to_node(dev, "gpio_sunxi", dev->name,
					 dev_ofnode(dev), &gpio_dev);
	if (ret)
		return ret;

	for (i = 0; i < desc->num_banks; ++i) {
		gpio_plat = malloc(sizeof(*gpio_plat));
		if (!gpio_plat)
			return -ENOMEM;

		gpio_plat->regs = plat->base + i * SUNXI_PINCTRL_BANK_SIZE;
		gpio_plat->bank_name[0] = 'P';
		gpio_plat->bank_name[1] = 'A' + desc->first_bank + i;
		gpio_plat->bank_name[2] = '\0';

		ret = device_bind(gpio_dev, DM_DRIVER_REF(gpio_sunxi),
				  gpio_plat->bank_name, gpio_plat,
				  ofnode_null(), NULL);
		if (ret)
			return ret;
	}

	return 0;
}

static int sunxi_pinctrl_probe(struct udevice *dev)
{
	struct clk *apb_clk;

	apb_clk = devm_clk_get(dev, "apb");
	if (!IS_ERR(apb_clk))
		clk_enable(apb_clk);

	return 0;
}

static const struct sunxi_pinctrl_function suniv_f1c100s_pinctrl_functions[] = {
	{ "gpio_in",	0 },
	{ "gpio_out",	1 },
	{ "i2c0",	3 },	/* PE11-PE12 */
	{ "i2c1",	3 },	/* PD5-PD6 */
	{ "mmc0",	2 },	/* PF0-PF5 */
	{ "mmc1",	3 },	/* PC0-PC2 */
	{ "spi0",	2 },	/* PC0-PC3 */
#if IS_ENABLED(CONFIG_UART0_PORT_F)
	{ "uart0",	3 },	/* PF2-PF4 */
#else
	{ "uart0",	5 },	/* PE0-PE1 */
#endif
	{ "uart1",	5 },	/* PA0-PA3 */
};

static const struct sunxi_pinctrl_desc __maybe_unused suniv_f1c100s_pinctrl_desc = {
	.functions	= suniv_f1c100s_pinctrl_functions,
	.num_functions	= ARRAY_SIZE(suniv_f1c100s_pinctrl_functions),
	.first_bank	= SUNXI_GPIO_A,
	.num_banks	= 6,
};

static const struct sunxi_pinctrl_function sun4i_a10_pinctrl_functions[] = {
	{ "emac",	2 },	/* PA0-PA17 */
	{ "gpio_in",	0 },
	{ "gpio_out",	1 },
	{ "i2c0",	2 },	/* PB0-PB1 */
	{ "i2c1",	2 },	/* PB18-PB19 */
	{ "mmc0",	2 },	/* PF0-PF5 */
#if IS_ENABLED(CONFIG_MMC1_PINS_PH)
	{ "mmc1",	5 },	/* PH22-PH27 */
#else
	{ "mmc1",	4 },	/* PG0-PG5 */
#endif
	{ "mmc2",	3 },	/* PC6-PC15 */
	{ "mmc3",	2 },	/* PI4-PI9 */
	{ "nand0",	2 },	/* PC0-PC24 */
	{ "spi0",	3 },	/* PC0-PC2, PC23 */
#if IS_ENABLED(CONFIG_UART0_PORT_F)
	{ "uart0",	4 },	/* PF2-PF4 */
#else
	{ "uart0",	2 },	/* PB22-PB23 */
#endif
};

static const struct sunxi_pinctrl_desc __maybe_unused sun4i_a10_pinctrl_desc = {
	.functions	= sun4i_a10_pinctrl_functions,
	.num_functions	= ARRAY_SIZE(sun4i_a10_pinctrl_functions),
	.first_bank	= SUNXI_GPIO_A,
	.num_banks	= 9,
};

static const struct sunxi_pinctrl_function sun5i_a13_pinctrl_functions[] = {
	{ "emac",	2 },	/* PA0-PA17 */
	{ "gpio_in",	0 },
	{ "gpio_out",	1 },
	{ "i2c0",	2 },	/* PB0-PB1 */
	{ "i2c1",	2 },	/* PB15-PB16 */
	{ "mmc0",	2 },	/* PF0-PF5 */
	{ "mmc1",	2 },	/* PG3-PG8 */
	{ "mmc2",	3 },	/* PC6-PC15 */
	{ "nand0",	2 },	/* PC0-PC19 */
	{ "spi0",	3 },	/* PC0-PC3 */
#if IS_ENABLED(CONFIG_UART0_PORT_F)
	{ "uart0",	4 },	/* PF2-PF4 */
#else
	{ "uart0",	2 },	/* PB19-PB20 */
#endif
	{ "uart1",	4 },	/* PG3-PG4 */
};

static const struct sunxi_pinctrl_desc __maybe_unused sun5i_a13_pinctrl_desc = {
	.functions	= sun5i_a13_pinctrl_functions,
	.num_functions	= ARRAY_SIZE(sun5i_a13_pinctrl_functions),
	.first_bank	= SUNXI_GPIO_A,
	.num_banks	= 7,
};

static const struct sunxi_pinctrl_function sun6i_a31_pinctrl_functions[] = {
	{ "gmac",	2 },	/* PA0-PA27 */
	{ "gpio_in",	0 },
	{ "gpio_out",	1 },
	{ "i2c0",	2 },	/* PH14-PH15 */
	{ "i2c1",	2 },	/* PH16-PH17 */
	{ "mmc0",	2 },	/* PF0-PF5 */
	{ "mmc1",	2 },	/* PG0-PG5 */
	{ "mmc2",	3 },	/* PC6-PC15, PC24 */
	{ "mmc3",	4 },	/* PC6-PC15, PC24 */
	{ "nand0",	2 },	/* PC0-PC26 */
	{ "spi0",	3 },	/* PC0-PC2, PC27 */
#if IS_ENABLED(CONFIG_UART0_PORT_F)
	{ "uart0",	3 },	/* PF2-PF4 */
#else
	{ "uart0",	2 },	/* PH20-PH21 */
#endif
};

static const struct sunxi_pinctrl_desc __maybe_unused sun6i_a31_pinctrl_desc = {
	.functions	= sun6i_a31_pinctrl_functions,
	.num_functions	= ARRAY_SIZE(sun6i_a31_pinctrl_functions),
	.first_bank	= SUNXI_GPIO_A,
	.num_banks	= 8,
};

static const struct sunxi_pinctrl_function sun6i_a31_r_pinctrl_functions[] = {
	{ "gpio_in",	0 },
	{ "gpio_out",	1 },
	{ "s_i2c",	2 },	/* PL0-PL1 */
	{ "s_p2wi",	3 },	/* PL0-PL1 */
	{ "s_uart",	2 },	/* PL2-PL3 */
};

static const struct sunxi_pinctrl_desc __maybe_unused sun6i_a31_r_pinctrl_desc = {
	.functions	= sun6i_a31_r_pinctrl_functions,
	.num_functions	= ARRAY_SIZE(sun6i_a31_r_pinctrl_functions),
	.first_bank	= SUNXI_GPIO_L,
	.num_banks	= 2,
};

static const struct sunxi_pinctrl_function sun7i_a20_pinctrl_functions[] = {
	{ "emac",	2 },	/* PA0-PA17 */
	{ "gmac",	5 },	/* PA0-PA17 */
	{ "gpio_in",	0 },
	{ "gpio_out",	1 },
	{ "i2c0",	2 },	/* PB0-PB1 */
	{ "i2c1",	2 },	/* PB18-PB19 */
	{ "mmc0",	2 },	/* PF0-PF5 */
#if IS_ENABLED(CONFIG_MMC1_PINS_PH)
	{ "mmc1",	5 },	/* PH22-PH27 */
#else
	{ "mmc1",	4 },	/* PG0-PG5 */
#endif
	{ "mmc2",	3 },	/* PC5-PC15, PC24 */
	{ "nand0",	2 },	/* PC0-PC24 */
	{ "spi0",	3 },	/* PC0-PC2, PC23 */
#if IS_ENABLED(CONFIG_UART0_PORT_F)
	{ "uart0",	4 },	/* PF2-PF4 */
#else
	{ "uart0",	2 },	/* PB22-PB23 */
#endif
};

static const struct sunxi_pinctrl_desc __maybe_unused sun7i_a20_pinctrl_desc = {
	.functions	= sun7i_a20_pinctrl_functions,
	.num_functions	= ARRAY_SIZE(sun7i_a20_pinctrl_functions),
	.first_bank	= SUNXI_GPIO_A,
	.num_banks	= 9,
};

static const struct sunxi_pinctrl_function sun8i_a23_pinctrl_functions[] = {
	{ "gpio_in",	0 },
	{ "gpio_out",	1 },
	{ "i2c0",	2 },	/* PH2-PH3 */
	{ "i2c1",	2 },	/* PH4-PH5 */
	{ "mmc0",	2 },	/* PF0-PF5 */
	{ "mmc1",	2 },	/* PG0-PG5 */
	{ "mmc2",	3 },	/* PC5-PC16 */
	{ "nand0",	2 },	/* PC0-PC16 */
	{ "spi0",	3 },	/* PC0-PC3 */
#if IS_ENABLED(CONFIG_UART0_PORT_F)
	{ "uart0",	3 },	/* PF2-PF4 */
#endif
	{ "uart1",	2 },	/* PG6-PG7 */
	{ "uart2",	2 },	/* PB0-PB1 */
};

static const struct sunxi_pinctrl_desc __maybe_unused sun8i_a23_pinctrl_desc = {
	.functions	= sun8i_a23_pinctrl_functions,
	.num_functions	= ARRAY_SIZE(sun8i_a23_pinctrl_functions),
	.first_bank	= SUNXI_GPIO_A,
	.num_banks	= 8,
};

static const struct sunxi_pinctrl_function sun8i_a23_r_pinctrl_functions[] = {
	{ "gpio_in",	0 },
	{ "gpio_out",	1 },
	{ "s_i2c",	3 },	/* PL0-PL1 */
	{ "s_rsb",	2 },	/* PL0-PL1 */
	{ "s_uart",	2 },	/* PL2-PL3 */
};

static const struct sunxi_pinctrl_desc __maybe_unused sun8i_a23_r_pinctrl_desc = {
	.functions	= sun8i_a23_r_pinctrl_functions,
	.num_functions	= ARRAY_SIZE(sun8i_a23_r_pinctrl_functions),
	.first_bank	= SUNXI_GPIO_L,
	.num_banks	= 1,
};

static const struct sunxi_pinctrl_function sun8i_a33_pinctrl_functions[] = {
	{ "gpio_in",	0 },
	{ "gpio_out",	1 },
	{ "i2c0",	2 },	/* PH2-PH3 */
	{ "i2c1",	2 },	/* PH4-PH5 */
	{ "mmc0",	2 },	/* PF0-PF5 */
	{ "mmc1",	2 },	/* PG0-PG5 */
	{ "mmc2",	3 },	/* PC5-PC16 */
	{ "nand0",	2 },	/* PC0-PC16 */
	{ "spi0",	3 },	/* PC0-PC3 */
#if IS_ENABLED(CONFIG_UART0_PORT_F)
	{ "uart0",	3 },	/* PF2-PF4 */
#else
	{ "uart0",	3 },	/* PB0-PB1 */
#endif
	{ "uart1",	2 },	/* PG6-PG7 */
	{ "uart2",	2 },	/* PB0-PB1 */
};

static const struct sunxi_pinctrl_desc __maybe_unused sun8i_a33_pinctrl_desc = {
	.functions	= sun8i_a33_pinctrl_functions,
	.num_functions	= ARRAY_SIZE(sun8i_a33_pinctrl_functions),
	.first_bank	= SUNXI_GPIO_A,
	.num_banks	= 8,
};

static const struct sunxi_pinctrl_function sun8i_a83t_pinctrl_functions[] = {
	{ "gmac",	4 },	/* PD2-PD23 */
	{ "gpio_in",	0 },
	{ "gpio_out",	1 },
	{ "i2c0",	2 },	/* PH0-PH1 */
	{ "i2c1",	2 },	/* PH2-PH3 */
	{ "mmc0",	2 },	/* PF0-PF5 */
	{ "mmc1",	2 },	/* PG0-PG5 */
	{ "mmc2",	3 },	/* PC5-PC16 */
	{ "nand0",	2 },	/* PC0-PC18 */
	{ "spi0",	3 },	/* PC0-PC3 */
#if IS_ENABLED(CONFIG_UART0_PORT_F)
	{ "uart0",	3 },	/* PF2-PF4 */
#else
	{ "uart0",	2 },	/* PB9-PB10 */
#endif
	{ "uart1",	2 },	/* PG6-PG7 */
	{ "uart2",	2 },	/* PB0-PB1 */
};

static const struct sunxi_pinctrl_desc __maybe_unused sun8i_a83t_pinctrl_desc = {
	.functions	= sun8i_a83t_pinctrl_functions,
	.num_functions	= ARRAY_SIZE(sun8i_a83t_pinctrl_functions),
	.first_bank	= SUNXI_GPIO_A,
	.num_banks	= 8,
};

static const struct sunxi_pinctrl_function sun8i_a83t_r_pinctrl_functions[] = {
	{ "gpio_in",	0 },
	{ "gpio_out",	1 },
	{ "s_i2c",	2 },	/* PL8-PL9 */
	{ "s_rsb",	2 },	/* PL0-PL1 */
	{ "s_uart",	2 },	/* PL2-PL3 */
};

static const struct sunxi_pinctrl_desc __maybe_unused sun8i_a83t_r_pinctrl_desc = {
	.functions	= sun8i_a83t_r_pinctrl_functions,
	.num_functions	= ARRAY_SIZE(sun8i_a83t_r_pinctrl_functions),
	.first_bank	= SUNXI_GPIO_L,
	.num_banks	= 1,
};

static const struct sunxi_pinctrl_function sun8i_h3_pinctrl_functions[] = {
	{ "emac",	2 },	/* PD0-PD17 */
	{ "gpio_in",	0 },
	{ "gpio_out",	1 },
	{ "i2c0",	2 },	/* PA11-PA12 */
	{ "i2c1",	3 },	/* PA18-PA19 */
	{ "mmc0",	2 },	/* PF0-PF5 */
	{ "mmc1",	2 },	/* PG0-PG5 */
	{ "mmc2",	3 },	/* PC5-PC16 */
	{ "nand0",	2 },	/* PC0-PC16 */
	{ "spi0",	3 },	/* PC0-PC3 */
#if IS_ENABLED(CONFIG_UART0_PORT_F)
	{ "uart0",	3 },	/* PF2-PF4 */
#else
	{ "uart0",	2 },	/* PA4-PA5 */
#endif
	{ "uart1",	2 },	/* PG6-PG7 */
	{ "uart2",	2 },	/* PA0-PA1 */
};

static const struct sunxi_pinctrl_desc __maybe_unused sun8i_h3_pinctrl_desc = {
	.functions	= sun8i_h3_pinctrl_functions,
	.num_functions	= ARRAY_SIZE(sun8i_h3_pinctrl_functions),
	.first_bank	= SUNXI_GPIO_A,
	.num_banks	= 7,
};

static const struct sunxi_pinctrl_function sun8i_h3_r_pinctrl_functions[] = {
	{ "gpio_in",	0 },
	{ "gpio_out",	1 },
	{ "s_i2c",	2 },	/* PL0-PL1 */
	{ "s_uart",	2 },	/* PL2-PL3 */
};

static const struct sunxi_pinctrl_desc __maybe_unused sun8i_h3_r_pinctrl_desc = {
	.functions	= sun8i_h3_r_pinctrl_functions,
	.num_functions	= ARRAY_SIZE(sun8i_h3_r_pinctrl_functions),
	.first_bank	= SUNXI_GPIO_L,
	.num_banks	= 1,
};

static const struct sunxi_pinctrl_function sun8i_v3s_pinctrl_functions[] = {
	{ "emac",	4 },	/* PD0-PD17 */
	{ "gpio_in",	0 },
	{ "gpio_out",	1 },
	{ "i2c0",	2 },	/* PB6-PB7 */
	{ "i2c1",	2 },	/* PB8-PB9 */
	{ "mmc0",	2 },	/* PF0-PF5 */
	{ "mmc1",	2 },	/* PG0-PG5 */
	{ "mmc2",	2 },	/* PC0-PC10 */
	{ "spi0",	3 },	/* PC0-PC3 */
#if IS_ENABLED(CONFIG_UART0_PORT_F)
	{ "uart0",	3 },	/* PF2-PF4 */
#else
	{ "uart0",	3 },	/* PB8-PB9 */
#endif
	{ "uart1",	2 },	/* PG6-PG7 */
	{ "uart2",	2 },	/* PB0-PB1 */
};

static const struct sunxi_pinctrl_desc __maybe_unused sun8i_v3s_pinctrl_desc = {
	.functions	= sun8i_v3s_pinctrl_functions,
	.num_functions	= ARRAY_SIZE(sun8i_v3s_pinctrl_functions),
	.first_bank	= SUNXI_GPIO_A,
	.num_banks	= 7,
};

static const struct sunxi_pinctrl_function sun9i_a80_pinctrl_functions[] = {
	{ "gmac",	2 },	/* PA0-PA17 */
	{ "gpio_in",	0 },
	{ "gpio_out",	1 },
	{ "i2c0",	2 },	/* PH0-PH1 */
	{ "i2c1",	2 },	/* PH2-PH3 */
	{ "mmc0",	2 },	/* PF0-PF5 */
	{ "mmc1",	2 },	/* PG0-PG5 */
	{ "mmc2",	3 },	/* PC6-PC16 */
	{ "nand0",	2 },	/* PC0-PC18 */
	{ "spi0",	3 },	/* PC0-PC2, PC19 */
#if IS_ENABLED(CONFIG_UART0_PORT_F)
	{ "uart0",	4 },	/* PF2-PF4 */
#else
	{ "uart0",	2 },	/* PH12-PH13 */
#endif
};

static const struct sunxi_pinctrl_desc __maybe_unused sun9i_a80_pinctrl_desc = {
	.functions	= sun9i_a80_pinctrl_functions,
	.num_functions	= ARRAY_SIZE(sun9i_a80_pinctrl_functions),
	.first_bank	= SUNXI_GPIO_A,
	.num_banks	= 8,
};

static const struct sunxi_pinctrl_function sun9i_a80_r_pinctrl_functions[] = {
	{ "gpio_in",	0 },
	{ "gpio_out",	1 },
	{ "s_i2c0",	2 },	/* PN0-PN1 */
	{ "s_i2c1",	3 },	/* PM8-PM9 */
	{ "s_rsb",	3 },	/* PN0-PN1 */
	{ "s_uart",	3 },	/* PL0-PL1 */
};

static const struct sunxi_pinctrl_desc __maybe_unused sun9i_a80_r_pinctrl_desc = {
	.functions	= sun9i_a80_r_pinctrl_functions,
	.num_functions	= ARRAY_SIZE(sun9i_a80_r_pinctrl_functions),
	.first_bank	= SUNXI_GPIO_L,
	.num_banks	= 3,
};

static const struct sunxi_pinctrl_function sun20i_d1_pinctrl_functions[] = {
	{ "emac",	8 },	/* PE0-PE15 */
	{ "gpio_in",	0 },
	{ "gpio_out",	1 },
	{ "i2c0",	4 },	/* PB10-PB11 */
	{ "mmc0",	2 },	/* PF0-PF5 */
	{ "mmc1",	2 },	/* PG0-PG5 */
	{ "mmc2",	3 },	/* PC2-PC7 */
	{ "spi0",	2 },	/* PC2-PC7 */
#if IS_ENABLED(CONFIG_UART0_PORT_F)
	{ "uart0",	3 },	/* PF2,PF4 */
#else
	{ "uart0",	6 },	/* PB0-PB1, PB8-PB9, PE2-PE3 */
#endif
	{ "uart1",	2 },	/* PG6-PG7 */
	{ "uart2",	7 },	/* PB0-PB1 */
	{ "uart3",	7 },	/* PB6-PB7 */
};

static const struct sunxi_pinctrl_desc __maybe_unused sun20i_d1_pinctrl_desc = {
	.functions	= sun20i_d1_pinctrl_functions,
	.num_functions	= ARRAY_SIZE(sun20i_d1_pinctrl_functions),
	.first_bank	= SUNXI_GPIO_A,
	.num_banks	= 7,
};

static const struct sunxi_pinctrl_function sun50i_a64_pinctrl_functions[] = {
	{ "emac",	4 },	/* PD8-PD23 */
	{ "gpio_in",	0 },
	{ "gpio_out",	1 },
	{ "i2c0",	2 },	/* PH0-PH1 */
	{ "i2c1",	2 },	/* PH2-PH3 */
	{ "mmc0",	2 },	/* PF0-PF5 */
	{ "mmc1",	2 },	/* PG0-PG5 */
	{ "mmc2",	3 },	/* PC1-PC16 */
	{ "nand0",	2 },	/* PC0-PC16 */
	{ "pwm",	2 },	/* PD22 */
	{ "spi0",	4 },	/* PC0-PC3 */
#if IS_ENABLED(CONFIG_UART0_PORT_F)
	{ "uart0",	3 },	/* PF2-PF4 */
#else
	{ "uart0",	4 },	/* PB8-PB9 */
#endif
	{ "uart1",	2 },	/* PG6-PG7 */
	{ "uart2",	2 },	/* PB0-PB1 */
};

static const struct sunxi_pinctrl_desc __maybe_unused sun50i_a64_pinctrl_desc = {
	.functions	= sun50i_a64_pinctrl_functions,
	.num_functions	= ARRAY_SIZE(sun50i_a64_pinctrl_functions),
	.first_bank	= SUNXI_GPIO_A,
	.num_banks	= 8,
};

static const struct sunxi_pinctrl_function sun50i_a64_r_pinctrl_functions[] = {
	{ "gpio_in",	0 },
	{ "gpio_out",	1 },
	{ "s_i2c",	2 },	/* PL8-PL9 */
	{ "s_rsb",	2 },	/* PL0-PL1 */
	{ "s_uart",	2 },	/* PL2-PL3 */
};

static const struct sunxi_pinctrl_desc __maybe_unused sun50i_a64_r_pinctrl_desc = {
	.functions	= sun50i_a64_r_pinctrl_functions,
	.num_functions	= ARRAY_SIZE(sun50i_a64_r_pinctrl_functions),
	.first_bank	= SUNXI_GPIO_L,
	.num_banks	= 1,
};

static const struct sunxi_pinctrl_function sun50i_h5_pinctrl_functions[] = {
	{ "emac",	2 },	/* PD0-PD17 */
	{ "gpio_in",	0 },
	{ "gpio_out",	1 },
	{ "i2c0",	2 },	/* PA11-PA12 */
	{ "i2c1",	3 },	/* PA18-PA19 */
	{ "mmc0",	2 },	/* PF0-PF5 */
	{ "mmc1",	2 },	/* PG0-PG5 */
	{ "mmc2",	3 },	/* PC1-PC16 */
	{ "nand0",	2 },	/* PC0-PC16 */
	{ "spi0",	3 },	/* PC0-PC3 */
#if IS_ENABLED(CONFIG_UART0_PORT_F)
	{ "uart0",	3 },	/* PF2-PF4 */
#else
	{ "uart0",	2 },	/* PA4-PA5 */
#endif
	{ "uart1",	2 },	/* PG6-PG7 */
	{ "uart2",	2 },	/* PA0-PA1 */
};

static const struct sunxi_pinctrl_desc __maybe_unused sun50i_h5_pinctrl_desc = {
	.functions	= sun50i_h5_pinctrl_functions,
	.num_functions	= ARRAY_SIZE(sun50i_h5_pinctrl_functions),
	.first_bank	= SUNXI_GPIO_A,
	.num_banks	= 7,
};

static const struct sunxi_pinctrl_function sun50i_h6_pinctrl_functions[] = {
	{ "emac",	5 },	/* PD0-PD20 */
	{ "gpio_in",	0 },
	{ "gpio_out",	1 },
	{ "i2c0",	2 },	/* PD25-PD26 */
	{ "i2c1",	4 },	/* PH5-PH6 */
	{ "mmc0",	2 },	/* PF0-PF5 */
	{ "mmc1",	2 },	/* PG0-PG5 */
	{ "mmc2",	3 },	/* PC1-PC14 */
	{ "nand0",	2 },	/* PC0-PC16 */
	{ "spi0",	4 },	/* PC0-PC7 */
#if IS_ENABLED(CONFIG_UART0_PORT_F)
	{ "uart0",	3 },	/* PF2-PF4 */
#else
	{ "uart0",	2 },	/* PH0-PH1 */
#endif
	{ "uart1",	2 },	/* PG6-PG7 */
};

static const struct sunxi_pinctrl_desc __maybe_unused sun50i_h6_pinctrl_desc = {
	.functions	= sun50i_h6_pinctrl_functions,
	.num_functions	= ARRAY_SIZE(sun50i_h6_pinctrl_functions),
	.first_bank	= SUNXI_GPIO_A,
	.num_banks	= 8,
};

static const struct sunxi_pinctrl_function sun50i_h6_r_pinctrl_functions[] = {
	{ "gpio_in",	0 },
	{ "gpio_out",	1 },
	{ "s_i2c",	3 },	/* PL0-PL1 */
	{ "s_rsb",	2 },	/* PL0-PL1 */
	{ "s_uart",	2 },	/* PL2-PL3 */
};

static const struct sunxi_pinctrl_desc __maybe_unused sun50i_h6_r_pinctrl_desc = {
	.functions	= sun50i_h6_r_pinctrl_functions,
	.num_functions	= ARRAY_SIZE(sun50i_h6_r_pinctrl_functions),
	.first_bank	= SUNXI_GPIO_L,
	.num_banks	= 2,
};

static const struct sunxi_pinctrl_function sun50i_h616_pinctrl_functions[] = {
	{ "emac0",	2 },	/* PI0-PI16 */
	{ "gpio_in",	0 },
	{ "gpio_out",	1 },
	{ "mmc0",	2 },	/* PF0-PF5 */
	{ "mmc1",	2 },	/* PG0-PG5 */
	{ "mmc2",	3 },	/* PC0-PC16 */
	{ "nand0",	2 },	/* PC0-PC16 */
	{ "spi0",	4 },	/* PC0-PC7, PC15-PC16 */
#if IS_ENABLED(CONFIG_UART0_PORT_F)
	{ "uart0",	3 },	/* PF2-PF4 */
#else
	{ "uart0",	2 },	/* PH0-PH1 */
#endif
	{ "uart1",	2 },	/* PG6-PG7 */
};

static const struct sunxi_pinctrl_desc __maybe_unused sun50i_h616_pinctrl_desc = {
	.functions	= sun50i_h616_pinctrl_functions,
	.num_functions	= ARRAY_SIZE(sun50i_h616_pinctrl_functions),
	.first_bank	= SUNXI_GPIO_A,
	.num_banks	= 9,
};

static const struct sunxi_pinctrl_function sun50i_h616_r_pinctrl_functions[] = {
	{ "gpio_in",	0 },
	{ "gpio_out",	1 },
	{ "s_i2c",	3 },	/* PL0-PL1 */
	{ "s_rsb",	2 },	/* PL0-PL1 */
	{ "s_uart",	2 },	/* PL2-PL3 */
};

static const struct sunxi_pinctrl_desc __maybe_unused sun50i_h616_r_pinctrl_desc = {
	.functions	= sun50i_h616_r_pinctrl_functions,
	.num_functions	= ARRAY_SIZE(sun50i_h616_r_pinctrl_functions),
	.first_bank	= SUNXI_GPIO_L,
	.num_banks	= 1,
};

static const struct udevice_id sunxi_pinctrl_ids[] = {
#ifdef CONFIG_PINCTRL_SUNIV_F1C100S
	{
		.compatible = "allwinner,suniv-f1c100s-pinctrl",
		.data = (ulong)&suniv_f1c100s_pinctrl_desc,
	},
#endif
#ifdef CONFIG_PINCTRL_SUN4I_A10
	{
		.compatible = "allwinner,sun4i-a10-pinctrl",
		.data = (ulong)&sun4i_a10_pinctrl_desc,
	},
#endif
#ifdef CONFIG_PINCTRL_SUN5I_A13
	{
		.compatible = "allwinner,sun5i-a10s-pinctrl",
		.data = (ulong)&sun5i_a13_pinctrl_desc,
	},
	{
		.compatible = "allwinner,sun5i-a13-pinctrl",
		.data = (ulong)&sun5i_a13_pinctrl_desc,
	},
#endif
#ifdef CONFIG_PINCTRL_SUN6I_A31
	{
		.compatible = "allwinner,sun6i-a31-pinctrl",
		.data = (ulong)&sun6i_a31_pinctrl_desc,
	},
	{
		.compatible = "allwinner,sun6i-a31s-pinctrl",
		.data = (ulong)&sun6i_a31_pinctrl_desc,
	},
#endif
#ifdef CONFIG_PINCTRL_SUN6I_A31_R
	{
		.compatible = "allwinner,sun6i-a31-r-pinctrl",
		.data = (ulong)&sun6i_a31_r_pinctrl_desc,
	},
#endif
#ifdef CONFIG_PINCTRL_SUN7I_A20
	{
		.compatible = "allwinner,sun7i-a20-pinctrl",
		.data = (ulong)&sun7i_a20_pinctrl_desc,
	},
#endif
#ifdef CONFIG_PINCTRL_SUN8I_A23
	{
		.compatible = "allwinner,sun8i-a23-pinctrl",
		.data = (ulong)&sun8i_a23_pinctrl_desc,
	},
#endif
#ifdef CONFIG_PINCTRL_SUN8I_A23_R
	{
		.compatible = "allwinner,sun8i-a23-r-pinctrl",
		.data = (ulong)&sun8i_a23_r_pinctrl_desc,
	},
#endif
#ifdef CONFIG_PINCTRL_SUN8I_A33
	{
		.compatible = "allwinner,sun8i-a33-pinctrl",
		.data = (ulong)&sun8i_a33_pinctrl_desc,
	},
#endif
#ifdef CONFIG_PINCTRL_SUN8I_A83T
	{
		.compatible = "allwinner,sun8i-a83t-pinctrl",
		.data = (ulong)&sun8i_a83t_pinctrl_desc,
	},
#endif
#ifdef CONFIG_PINCTRL_SUN8I_A83T_R
	{
		.compatible = "allwinner,sun8i-a83t-r-pinctrl",
		.data = (ulong)&sun8i_a83t_r_pinctrl_desc,
	},
#endif
#ifdef CONFIG_PINCTRL_SUN8I_H3
	{
		.compatible = "allwinner,sun8i-h3-pinctrl",
		.data = (ulong)&sun8i_h3_pinctrl_desc,
	},
#endif
#ifdef CONFIG_PINCTRL_SUN8I_H3_R
	{
		.compatible = "allwinner,sun8i-h3-r-pinctrl",
		.data = (ulong)&sun8i_h3_r_pinctrl_desc,
	},
#endif
#ifdef CONFIG_PINCTRL_SUN7I_A20
	{
		.compatible = "allwinner,sun8i-r40-pinctrl",
		.data = (ulong)&sun7i_a20_pinctrl_desc,
	},
#endif
#ifdef CONFIG_PINCTRL_SUN8I_V3S
	{
		.compatible = "allwinner,sun8i-v3-pinctrl",
		.data = (ulong)&sun8i_v3s_pinctrl_desc,
	},
	{
		.compatible = "allwinner,sun8i-v3s-pinctrl",
		.data = (ulong)&sun8i_v3s_pinctrl_desc,
	},
#endif
#ifdef CONFIG_PINCTRL_SUN9I_A80
	{
		.compatible = "allwinner,sun9i-a80-pinctrl",
		.data = (ulong)&sun9i_a80_pinctrl_desc,
	},
#endif
#ifdef CONFIG_PINCTRL_SUN9I_A80_R
	{
		.compatible = "allwinner,sun9i-a80-r-pinctrl",
		.data = (ulong)&sun9i_a80_r_pinctrl_desc,
	},
#endif
#ifdef CONFIG_PINCTRL_SUN20I_D1
	{
		.compatible = "allwinner,sun20i-d1-pinctrl",
		.data = (ulong)&sun20i_d1_pinctrl_desc,
	},
#endif
#ifdef CONFIG_PINCTRL_SUN50I_A64
	{
		.compatible = "allwinner,sun50i-a64-pinctrl",
		.data = (ulong)&sun50i_a64_pinctrl_desc,
	},
#endif
#ifdef CONFIG_PINCTRL_SUN50I_A64_R
	{
		.compatible = "allwinner,sun50i-a64-r-pinctrl",
		.data = (ulong)&sun50i_a64_r_pinctrl_desc,
	},
#endif
#ifdef CONFIG_PINCTRL_SUN50I_H5
	{
		.compatible = "allwinner,sun50i-h5-pinctrl",
		.data = (ulong)&sun50i_h5_pinctrl_desc,
	},
#endif
#ifdef CONFIG_PINCTRL_SUN50I_H6
	{
		.compatible = "allwinner,sun50i-h6-pinctrl",
		.data = (ulong)&sun50i_h6_pinctrl_desc,
	},
#endif
#ifdef CONFIG_PINCTRL_SUN50I_H6_R
	{
		.compatible = "allwinner,sun50i-h6-r-pinctrl",
		.data = (ulong)&sun50i_h6_r_pinctrl_desc,
	},
#endif
#ifdef CONFIG_PINCTRL_SUN50I_H616
	{
		.compatible = "allwinner,sun50i-h616-pinctrl",
		.data = (ulong)&sun50i_h616_pinctrl_desc,
	},
#endif
#ifdef CONFIG_PINCTRL_SUN50I_H616_R
	{
		.compatible = "allwinner,sun50i-h616-r-pinctrl",
		.data = (ulong)&sun50i_h616_r_pinctrl_desc,
	},
#endif
	{}
};

U_BOOT_DRIVER(sunxi_pinctrl) = {
	.name		= "sunxi-pinctrl",
	.id		= UCLASS_PINCTRL,
	.of_match	= sunxi_pinctrl_ids,
	.bind		= sunxi_pinctrl_bind,
	.probe		= sunxi_pinctrl_probe,
	.plat_auto	= sizeof(struct sunxi_pinctrl_plat),
	.ops		= &sunxi_pinctrl_ops,
};
