// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Sean Anderson <seanga2@gmail.com>
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <dm/pinctrl.h>
#include <dt-bindings/pinctrl/k210-pinctrl.h>
#include <mapmem.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/io.h>
#include <linux/err.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>

/*
 * The K210 only implements 8 drive levels, even though there is register space
 * for 16
 */
#define K210_PC_DRIVE_MASK GENMASK(11, 8)
#define K210_PC_DRIVE_SHIFT 8
#define K210_PC_DRIVE_0 (0 << K210_PC_DRIVE_SHIFT)
#define K210_PC_DRIVE_1 (1 << K210_PC_DRIVE_SHIFT)
#define K210_PC_DRIVE_2 (2 << K210_PC_DRIVE_SHIFT)
#define K210_PC_DRIVE_3 (3 << K210_PC_DRIVE_SHIFT)
#define K210_PC_DRIVE_4 (4 << K210_PC_DRIVE_SHIFT)
#define K210_PC_DRIVE_5 (5 << K210_PC_DRIVE_SHIFT)
#define K210_PC_DRIVE_6 (6 << K210_PC_DRIVE_SHIFT)
#define K210_PC_DRIVE_7 (7 << K210_PC_DRIVE_SHIFT)
#define K210_PC_DRIVE_MAX 7

#define K210_PC_MODE_MASK GENMASK(23, 12)
/*
 * output enabled == PC_OE & (PC_OE_INV ^ FUNCTION_OE) where FUNCTION_OE is a
 * physical signal from the function
 */
#define K210_PC_OE       BIT(12) /* Output Enable */
#define K210_PC_OE_INV   BIT(13) /* INVert function-controlled Output Enable */
#define K210_PC_DO_OE    BIT(14) /* set Data Out to the Output Enable signal */
#define K210_PC_DO_INV   BIT(15) /* INVert final Data Output */
#define K210_PC_PU       BIT(16) /* Pull Up */
#define K210_PC_PD       BIT(17) /* Pull Down */
/* Strong pull up not implemented on K210 */
#define K210_PC_SL       BIT(19) /* reduce SLew rate to prevent overshoot */
/* Same semantics as OE above */
#define K210_PC_IE       BIT(20) /* Input Enable */
#define K210_PC_IE_INV   BIT(21) /* INVert function-controlled Input Enable */
#define K210_PC_DI_INV   BIT(22) /* INVert Data Input */
#define K210_PC_ST       BIT(23) /* Schmitt Trigger */
#define K210_PC_DI       BIT(31) /* raw Data Input */
#define K210_PC_BIAS_MASK (K210_PC_PU & K210_PC_PD)

#define K210_PC_MODE_IN   (K210_PC_IE | K210_PC_ST)
#define K210_PC_MODE_OUT  (K210_PC_DRIVE_7 | K210_PC_OE)
#define K210_PC_MODE_I2C  (K210_PC_MODE_IN | K210_PC_SL | K210_PC_OE | \
			   K210_PC_PU)
#define K210_PC_MODE_SCCB (K210_PC_MODE_I2C | K210_PC_OE_INV | K210_PC_IE_INV)
#define K210_PC_MODE_SPI  (K210_PC_MODE_IN | K210_PC_IE_INV | \
			   K210_PC_MODE_OUT | K210_PC_OE_INV)
#define K210_PC_MODE_GPIO (K210_PC_MODE_IN | K210_PC_MODE_OUT)

#define K210_PG_FUNC GENMASK(7, 0)
#define K210_PG_DO BIT(8)
#define K210_PG_PIN GENMASK(22, 16)

#define PIN_CONFIG_OUTPUT_INVERT (PIN_CONFIG_END + 1)
#define PIN_CONFIG_INPUT_INVERT (PIN_CONFIG_END + 2)

struct k210_fpioa {
	u32 pins[48];
	u32 tie_en[8];
	u32 tie_val[8];
};

struct k210_pc_priv {
	struct clk clk;
	struct k210_fpioa __iomem *fpioa; /* FPIOA register */
	struct regmap *sysctl; /* Sysctl regmap */
	u32 power_offset; /* Power bank register offset */
};

#ifdef CONFIG_CMD_PINMUX
static const char k210_pc_pin_names[][6] = {
#define PIN(i) \
	[i] = "IO_" #i
	PIN(0),
	PIN(1),
	PIN(2),
	PIN(3),
	PIN(4),
	PIN(5),
	PIN(6),
	PIN(7),
	PIN(8),
	PIN(9),
	PIN(10),
	PIN(11),
	PIN(12),
	PIN(13),
	PIN(14),
	PIN(15),
	PIN(16),
	PIN(17),
	PIN(18),
	PIN(19),
	PIN(20),
	PIN(21),
	PIN(22),
	PIN(23),
	PIN(24),
	PIN(25),
	PIN(26),
	PIN(27),
	PIN(28),
	PIN(29),
	PIN(30),
	PIN(31),
	PIN(32),
	PIN(33),
	PIN(34),
	PIN(35),
	PIN(36),
	PIN(37),
	PIN(38),
	PIN(39),
	PIN(40),
	PIN(41),
	PIN(42),
	PIN(43),
	PIN(44),
	PIN(45),
	PIN(46),
	PIN(47),
#undef PIN
};

static int k210_pc_get_pins_count(struct udevice *dev)
{
	return ARRAY_SIZE(k210_pc_pin_names);
};

static const char *k210_pc_get_pin_name(struct udevice *dev, unsigned selector)
{
	return k210_pc_pin_names[selector];
}
#endif /* CONFIG_CMD_PINMUX */

/* These are just power domains */
static const char k210_pc_group_names[][3] = {
	[0] = "A0",
	[1] = "A1",
	[2] = "A2",
	[3] = "B3",
	[4] = "B4",
	[5] = "B5",
	[6] = "C6",
	[7] = "C7",
};

static int k210_pc_get_groups_count(struct udevice *dev)
{
	return ARRAY_SIZE(k210_pc_group_names);
}

static const char *k210_pc_get_group_name(struct udevice *dev,
					  unsigned selector)
{
	return k210_pc_group_names[selector];
}

enum k210_pc_mode_id {
	K210_PC_DEFAULT_DISABLED,
	K210_PC_DEFAULT_IN,
	K210_PC_DEFAULT_IN_TIE,
	K210_PC_DEFAULT_OUT,
	K210_PC_DEFAULT_I2C,
	K210_PC_DEFAULT_SCCB,
	K210_PC_DEFAULT_SPI,
	K210_PC_DEFAULT_GPIO,
	K210_PC_DEFAULT_INT13,
};

static const u32 k210_pc_mode_id_to_mode[] = {
#define DEFAULT(mode) \
	[K210_PC_DEFAULT_##mode] = K210_PC_MODE_##mode
	[K210_PC_DEFAULT_DISABLED] = 0,
	DEFAULT(IN),
	[K210_PC_DEFAULT_IN_TIE] = K210_PC_MODE_IN,
	DEFAULT(OUT),
	DEFAULT(I2C),
	DEFAULT(SCCB),
	DEFAULT(SPI),
	DEFAULT(GPIO),
	[K210_PC_DEFAULT_INT13] = K210_PC_MODE_IN | K210_PC_PU,
#undef DEFAULT
};

/* This saves around 2K vs having a pointer+mode */
struct k210_pcf_info {
#ifdef CONFIG_CMD_PINMUX
	char name[15];
#endif
	u8 mode_id;
};

static const struct k210_pcf_info k210_pcf_infos[] = {
#ifdef CONFIG_CMD_PINMUX
#define FUNC(id, mode) \
	[K210_PCF_##id] = { \
		.name = #id, \
		.mode_id = K210_PC_DEFAULT_##mode \
	}
#else
#define FUNC(id, mode) \
	[K210_PCF_##id] = { \
		.mode_id = K210_PC_DEFAULT_##mode \
	}
#endif
	FUNC(JTAG_TCLK,      IN),
	FUNC(JTAG_TDI,       IN),
	FUNC(JTAG_TMS,       IN),
	FUNC(JTAG_TDO,       OUT),
	FUNC(SPI0_D0,        SPI),
	FUNC(SPI0_D1,        SPI),
	FUNC(SPI0_D2,        SPI),
	FUNC(SPI0_D3,        SPI),
	FUNC(SPI0_D4,        SPI),
	FUNC(SPI0_D5,        SPI),
	FUNC(SPI0_D6,        SPI),
	FUNC(SPI0_D7,        SPI),
	FUNC(SPI0_SS0,       OUT),
	FUNC(SPI0_SS1,       OUT),
	FUNC(SPI0_SS2,       OUT),
	FUNC(SPI0_SS3,       OUT),
	FUNC(SPI0_ARB,       IN_TIE),
	FUNC(SPI0_SCLK,      OUT),
	FUNC(UARTHS_RX,      IN),
	FUNC(UARTHS_TX,      OUT),
	FUNC(RESV6,          IN),
	FUNC(RESV7,          IN),
	FUNC(CLK_SPI1,       OUT),
	FUNC(CLK_I2C1,       OUT),
	FUNC(GPIOHS0,        GPIO),
	FUNC(GPIOHS1,        GPIO),
	FUNC(GPIOHS2,        GPIO),
	FUNC(GPIOHS3,        GPIO),
	FUNC(GPIOHS4,        GPIO),
	FUNC(GPIOHS5,        GPIO),
	FUNC(GPIOHS6,        GPIO),
	FUNC(GPIOHS7,        GPIO),
	FUNC(GPIOHS8,        GPIO),
	FUNC(GPIOHS9,        GPIO),
	FUNC(GPIOHS10,       GPIO),
	FUNC(GPIOHS11,       GPIO),
	FUNC(GPIOHS12,       GPIO),
	FUNC(GPIOHS13,       GPIO),
	FUNC(GPIOHS14,       GPIO),
	FUNC(GPIOHS15,       GPIO),
	FUNC(GPIOHS16,       GPIO),
	FUNC(GPIOHS17,       GPIO),
	FUNC(GPIOHS18,       GPIO),
	FUNC(GPIOHS19,       GPIO),
	FUNC(GPIOHS20,       GPIO),
	FUNC(GPIOHS21,       GPIO),
	FUNC(GPIOHS22,       GPIO),
	FUNC(GPIOHS23,       GPIO),
	FUNC(GPIOHS24,       GPIO),
	FUNC(GPIOHS25,       GPIO),
	FUNC(GPIOHS26,       GPIO),
	FUNC(GPIOHS27,       GPIO),
	FUNC(GPIOHS28,       GPIO),
	FUNC(GPIOHS29,       GPIO),
	FUNC(GPIOHS30,       GPIO),
	FUNC(GPIOHS31,       GPIO),
	FUNC(GPIO0,          GPIO),
	FUNC(GPIO1,          GPIO),
	FUNC(GPIO2,          GPIO),
	FUNC(GPIO3,          GPIO),
	FUNC(GPIO4,          GPIO),
	FUNC(GPIO5,          GPIO),
	FUNC(GPIO6,          GPIO),
	FUNC(GPIO7,          GPIO),
	FUNC(UART1_RX,       IN),
	FUNC(UART1_TX,       OUT),
	FUNC(UART2_RX,       IN),
	FUNC(UART2_TX,       OUT),
	FUNC(UART3_RX,       IN),
	FUNC(UART3_TX,       OUT),
	FUNC(SPI1_D0,        SPI),
	FUNC(SPI1_D1,        SPI),
	FUNC(SPI1_D2,        SPI),
	FUNC(SPI1_D3,        SPI),
	FUNC(SPI1_D4,        SPI),
	FUNC(SPI1_D5,        SPI),
	FUNC(SPI1_D6,        SPI),
	FUNC(SPI1_D7,        SPI),
	FUNC(SPI1_SS0,       OUT),
	FUNC(SPI1_SS1,       OUT),
	FUNC(SPI1_SS2,       OUT),
	FUNC(SPI1_SS3,       OUT),
	FUNC(SPI1_ARB,       IN_TIE),
	FUNC(SPI1_SCLK,      OUT),
	FUNC(SPI2_D0,        SPI),
	FUNC(SPI2_SS,        IN),
	FUNC(SPI2_SCLK,      IN),
	FUNC(I2S0_MCLK,      OUT),
	FUNC(I2S0_SCLK,      OUT),
	FUNC(I2S0_WS,        OUT),
	FUNC(I2S0_IN_D0,     IN),
	FUNC(I2S0_IN_D1,     IN),
	FUNC(I2S0_IN_D2,     IN),
	FUNC(I2S0_IN_D3,     IN),
	FUNC(I2S0_OUT_D0,    OUT),
	FUNC(I2S0_OUT_D1,    OUT),
	FUNC(I2S0_OUT_D2,    OUT),
	FUNC(I2S0_OUT_D3,    OUT),
	FUNC(I2S1_MCLK,      OUT),
	FUNC(I2S1_SCLK,      OUT),
	FUNC(I2S1_WS,        OUT),
	FUNC(I2S1_IN_D0,     IN),
	FUNC(I2S1_IN_D1,     IN),
	FUNC(I2S1_IN_D2,     IN),
	FUNC(I2S1_IN_D3,     IN),
	FUNC(I2S1_OUT_D0,    OUT),
	FUNC(I2S1_OUT_D1,    OUT),
	FUNC(I2S1_OUT_D2,    OUT),
	FUNC(I2S1_OUT_D3,    OUT),
	FUNC(I2S2_MCLK,      OUT),
	FUNC(I2S2_SCLK,      OUT),
	FUNC(I2S2_WS,        OUT),
	FUNC(I2S2_IN_D0,     IN),
	FUNC(I2S2_IN_D1,     IN),
	FUNC(I2S2_IN_D2,     IN),
	FUNC(I2S2_IN_D3,     IN),
	FUNC(I2S2_OUT_D0,    OUT),
	FUNC(I2S2_OUT_D1,    OUT),
	FUNC(I2S2_OUT_D2,    OUT),
	FUNC(I2S2_OUT_D3,    OUT),
	FUNC(RESV0,          DISABLED),
	FUNC(RESV1,          DISABLED),
	FUNC(RESV2,          DISABLED),
	FUNC(RESV3,          DISABLED),
	FUNC(RESV4,          DISABLED),
	FUNC(RESV5,          DISABLED),
	FUNC(I2C0_SCLK,      I2C),
	FUNC(I2C0_SDA,       I2C),
	FUNC(I2C1_SCLK,      I2C),
	FUNC(I2C1_SDA,       I2C),
	FUNC(I2C2_SCLK,      I2C),
	FUNC(I2C2_SDA,       I2C),
	FUNC(DVP_XCLK,       OUT),
	FUNC(DVP_RST,        OUT),
	FUNC(DVP_PWDN,       OUT),
	FUNC(DVP_VSYNC,      IN),
	FUNC(DVP_HSYNC,      IN),
	FUNC(DVP_PCLK,       IN),
	FUNC(DVP_D0,         IN),
	FUNC(DVP_D1,         IN),
	FUNC(DVP_D2,         IN),
	FUNC(DVP_D3,         IN),
	FUNC(DVP_D4,         IN),
	FUNC(DVP_D5,         IN),
	FUNC(DVP_D6,         IN),
	FUNC(DVP_D7,         IN),
	FUNC(SCCB_SCLK,      SCCB),
	FUNC(SCCB_SDA,       SCCB),
	FUNC(UART1_CTS,      IN),
	FUNC(UART1_DSR,      IN),
	FUNC(UART1_DCD,      IN),
	FUNC(UART1_RI,       IN),
	FUNC(UART1_SIR_IN,   IN),
	FUNC(UART1_DTR,      OUT),
	FUNC(UART1_RTS,      OUT),
	FUNC(UART1_OUT2,     OUT),
	FUNC(UART1_OUT1,     OUT),
	FUNC(UART1_SIR_OUT,  OUT),
	FUNC(UART1_BAUD,     OUT),
	FUNC(UART1_RE,       OUT),
	FUNC(UART1_DE,       OUT),
	FUNC(UART1_RS485_EN, OUT),
	FUNC(UART2_CTS,      IN),
	FUNC(UART2_DSR,      IN),
	FUNC(UART2_DCD,      IN),
	FUNC(UART2_RI,       IN),
	FUNC(UART2_SIR_IN,   IN),
	FUNC(UART2_DTR,      OUT),
	FUNC(UART2_RTS,      OUT),
	FUNC(UART2_OUT2,     OUT),
	FUNC(UART2_OUT1,     OUT),
	FUNC(UART2_SIR_OUT,  OUT),
	FUNC(UART2_BAUD,     OUT),
	FUNC(UART2_RE,       OUT),
	FUNC(UART2_DE,       OUT),
	FUNC(UART2_RS485_EN, OUT),
	FUNC(UART3_CTS,      IN),
	FUNC(UART3_DSR,      IN),
	FUNC(UART3_DCD,      IN),
	FUNC(UART3_RI,       IN),
	FUNC(UART3_SIR_IN,   IN),
	FUNC(UART3_DTR,      OUT),
	FUNC(UART3_RTS,      OUT),
	FUNC(UART3_OUT2,     OUT),
	FUNC(UART3_OUT1,     OUT),
	FUNC(UART3_SIR_OUT,  OUT),
	FUNC(UART3_BAUD,     OUT),
	FUNC(UART3_RE,       OUT),
	FUNC(UART3_DE,       OUT),
	FUNC(UART3_RS485_EN, OUT),
	FUNC(TIMER0_TOGGLE1, OUT),
	FUNC(TIMER0_TOGGLE2, OUT),
	FUNC(TIMER0_TOGGLE3, OUT),
	FUNC(TIMER0_TOGGLE4, OUT),
	FUNC(TIMER1_TOGGLE1, OUT),
	FUNC(TIMER1_TOGGLE2, OUT),
	FUNC(TIMER1_TOGGLE3, OUT),
	FUNC(TIMER1_TOGGLE4, OUT),
	FUNC(TIMER2_TOGGLE1, OUT),
	FUNC(TIMER2_TOGGLE2, OUT),
	FUNC(TIMER2_TOGGLE3, OUT),
	FUNC(TIMER2_TOGGLE4, OUT),
	FUNC(CLK_SPI2,       OUT),
	FUNC(CLK_I2C2,       OUT),
	FUNC(INTERNAL0,      OUT),
	FUNC(INTERNAL1,      OUT),
	FUNC(INTERNAL2,      OUT),
	FUNC(INTERNAL3,      OUT),
	FUNC(INTERNAL4,      OUT),
	FUNC(INTERNAL5,      OUT),
	FUNC(INTERNAL6,      OUT),
	FUNC(INTERNAL7,      OUT),
	FUNC(INTERNAL8,      OUT),
	FUNC(INTERNAL9,      IN),
	FUNC(INTERNAL10,     IN),
	FUNC(INTERNAL11,     IN),
	FUNC(INTERNAL12,     IN),
	FUNC(INTERNAL13,     INT13),
	FUNC(INTERNAL14,     I2C),
	FUNC(INTERNAL15,     IN),
	FUNC(INTERNAL16,     IN),
	FUNC(INTERNAL17,     IN),
	FUNC(CONSTANT,       DISABLED),
	FUNC(INTERNAL18,     IN),
	FUNC(DEBUG0,         OUT),
	FUNC(DEBUG1,         OUT),
	FUNC(DEBUG2,         OUT),
	FUNC(DEBUG3,         OUT),
	FUNC(DEBUG4,         OUT),
	FUNC(DEBUG5,         OUT),
	FUNC(DEBUG6,         OUT),
	FUNC(DEBUG7,         OUT),
	FUNC(DEBUG8,         OUT),
	FUNC(DEBUG9,         OUT),
	FUNC(DEBUG10,        OUT),
	FUNC(DEBUG11,        OUT),
	FUNC(DEBUG12,        OUT),
	FUNC(DEBUG13,        OUT),
	FUNC(DEBUG14,        OUT),
	FUNC(DEBUG15,        OUT),
	FUNC(DEBUG16,        OUT),
	FUNC(DEBUG17,        OUT),
	FUNC(DEBUG18,        OUT),
	FUNC(DEBUG19,        OUT),
	FUNC(DEBUG20,        OUT),
	FUNC(DEBUG21,        OUT),
	FUNC(DEBUG22,        OUT),
	FUNC(DEBUG23,        OUT),
	FUNC(DEBUG24,        OUT),
	FUNC(DEBUG25,        OUT),
	FUNC(DEBUG26,        OUT),
	FUNC(DEBUG27,        OUT),
	FUNC(DEBUG28,        OUT),
	FUNC(DEBUG29,        OUT),
	FUNC(DEBUG30,        OUT),
	FUNC(DEBUG31,        OUT),
#undef FUNC
};

static int k210_pc_pinmux_set(struct udevice *dev, u32 pinmux_group)
{
	unsigned pin = FIELD_GET(K210_PG_PIN, pinmux_group);
	bool do_oe = FIELD_GET(K210_PG_DO, pinmux_group);
	unsigned func = FIELD_GET(K210_PG_FUNC, pinmux_group);
	struct k210_pc_priv *priv = dev_get_priv(dev);
	const struct k210_pcf_info *info = &k210_pcf_infos[func];
	u32 mode = k210_pc_mode_id_to_mode[info->mode_id];
	u32 val = func | mode | (do_oe ? K210_PC_DO_OE : 0);

	debug("%s(%.8x): IO_%.2u = %3u | %.8x\n", __func__, pinmux_group, pin,
	      func, mode);

	writel(val, &priv->fpioa->pins[pin]);
	return pin;
}

/* Max drive strength in uA */
static const int k210_pc_drive_strength[] = {
	[0] = 11200,
	[1] = 16800,
	[2] = 22300,
	[3] = 27800,
	[4] = 33300,
	[5] = 38700,
	[6] = 44100,
	[7] = 49500,
};

static int k210_pc_get_drive(unsigned max_strength_ua)
{
	int i;

	for (i = K210_PC_DRIVE_MAX; i >= 0; i--)
		if (k210_pc_drive_strength[i] < max_strength_ua)
			return i;

	return -EINVAL;
}

static int k210_pc_pinconf_set(struct udevice *dev, unsigned pin_selector,
			       unsigned param, unsigned argument)
{
	struct k210_pc_priv *priv = dev_get_priv(dev);
	u32 val = readl(&priv->fpioa->pins[pin_selector]);

	switch (param) {
	case PIN_CONFIG_BIAS_DISABLE:
		val &= ~K210_PC_BIAS_MASK;
		break;
	case PIN_CONFIG_BIAS_PULL_DOWN:
		if (argument)
			val |= K210_PC_PD;
		else
			return -EINVAL;
		break;
	case PIN_CONFIG_BIAS_PULL_UP:
		if (argument)
			val |= K210_PC_PU;
		else
			return -EINVAL;
		break;
	case PIN_CONFIG_DRIVE_STRENGTH:
		argument *= 1000;
	case PIN_CONFIG_DRIVE_STRENGTH_UA: {
		int drive = k210_pc_get_drive(argument);

		if (IS_ERR_VALUE(drive))
			return drive;
		val &= ~K210_PC_DRIVE_MASK;
		val |= FIELD_PREP(K210_PC_DRIVE_MASK, drive);
		break;
	}
	case PIN_CONFIG_INPUT_ENABLE:
		if (argument)
			val |= K210_PC_IE;
		else
			val &= ~K210_PC_IE;
		break;
	case PIN_CONFIG_INPUT_SCHMITT:
		argument = 1;
	case PIN_CONFIG_INPUT_SCHMITT_ENABLE:
		if (argument)
			val |= K210_PC_ST;
		else
			val &= ~K210_PC_ST;
		break;
	case PIN_CONFIG_OUTPUT:
		k210_pc_pinmux_set(dev,
				   K210_FPIOA(pin_selector, K210_PCF_CONSTANT));
		val = readl(&priv->fpioa->pins[pin_selector]);
		val |= K210_PC_MODE_OUT;

		if (!argument)
			val |= K210_PC_DO_INV;
		break;
	case PIN_CONFIG_OUTPUT_ENABLE:
		if (argument)
			val |= K210_PC_OE;
		else
			val &= ~K210_PC_OE;
		break;
	case PIN_CONFIG_SLEW_RATE:
		if (argument)
			val |= K210_PC_SL;
		else
			val &= ~K210_PC_SL;
		break;
	case PIN_CONFIG_OUTPUT_INVERT:
		if (argument)
			val |= K210_PC_DO_INV;
		else
			val &= ~K210_PC_DO_INV;
		break;
	case PIN_CONFIG_INPUT_INVERT:
		if (argument)
			val |= K210_PC_DI_INV;
		else
			val &= ~K210_PC_DI_INV;
		break;
	default:
		return -EINVAL;
	}

	writel(val, &priv->fpioa->pins[pin_selector]);
	return 0;
}

static int k210_pc_pinconf_group_set(struct udevice *dev,
				     unsigned group_selector, unsigned param,
				     unsigned argument)
{
	struct k210_pc_priv *priv = dev_get_priv(dev);

	if (param == PIN_CONFIG_POWER_SOURCE) {
		u32 bit = BIT(group_selector);

		regmap_update_bits(priv->sysctl, priv->power_offset, bit,
				   argument ? bit : 0);
	} else {
		return -EINVAL;
	}

	return 0;
}

#ifdef CONFIG_CMD_PINMUX
static int k210_pc_get_pin_muxing(struct udevice *dev, unsigned int selector,
				  char *buf, int size)
{
	struct k210_pc_priv *priv = dev_get_priv(dev);
	u32 val = readl(&priv->fpioa->pins[selector]);
	const struct k210_pcf_info *info = &k210_pcf_infos[val & K210_PCF_MASK];

	strncpy(buf, info->name, min((size_t)size, sizeof(info->name)));
	return 0;
}
#endif

static const struct pinconf_param k210_pc_pinconf_params[] = {
	{ "bias-disable", PIN_CONFIG_BIAS_DISABLE, 0 },
	{ "bias-pull-down", PIN_CONFIG_BIAS_PULL_DOWN, 1 },
	{ "bias-pull-up", PIN_CONFIG_BIAS_PULL_UP, 1 },
	{ "drive-strength", PIN_CONFIG_DRIVE_STRENGTH, U32_MAX },
	{ "drive-strength-ua", PIN_CONFIG_DRIVE_STRENGTH_UA, U32_MAX },
	{ "input-enable", PIN_CONFIG_INPUT_ENABLE, 1 },
	{ "input-disable", PIN_CONFIG_INPUT_ENABLE, 0 },
	{ "input-schmitt-enable", PIN_CONFIG_INPUT_SCHMITT_ENABLE, 1 },
	{ "input-schmitt-disable", PIN_CONFIG_INPUT_SCHMITT_ENABLE, 0 },
	{ "power-source", PIN_CONFIG_POWER_SOURCE, K210_PC_POWER_1V8 },
	{ "output-low", PIN_CONFIG_OUTPUT, 0 },
	{ "output-high", PIN_CONFIG_OUTPUT, 1 },
	{ "output-enable", PIN_CONFIG_OUTPUT_ENABLE, 1 },
	{ "output-disable", PIN_CONFIG_OUTPUT_ENABLE, 0 },
	{ "slew-rate", PIN_CONFIG_SLEW_RATE, 1 },
	{ "output-polarity-invert", PIN_CONFIG_OUTPUT_INVERT, 1},
	{ "input-polarity-invert", PIN_CONFIG_INPUT_INVERT, 1},
};

static const struct pinctrl_ops k210_pc_pinctrl_ops = {
#ifdef CONFIG_CMD_PINMUX
	.get_pins_count = k210_pc_get_pins_count,
	.get_pin_name = k210_pc_get_pin_name,
#endif
	.get_groups_count = k210_pc_get_groups_count,
	.get_group_name = k210_pc_get_group_name,
	.pinmux_property_set = k210_pc_pinmux_set,
	.pinconf_num_params = ARRAY_SIZE(k210_pc_pinconf_params),
	.pinconf_params = k210_pc_pinconf_params,
	.pinconf_set = k210_pc_pinconf_set,
	.pinconf_group_set = k210_pc_pinconf_group_set,
	.set_state = pinctrl_generic_set_state,
#ifdef CONFIG_CMD_PINMUX
	.get_pin_muxing = k210_pc_get_pin_muxing,
#endif
};

static int k210_pc_probe(struct udevice *dev)
{
	int ret, i, j;
	struct k210_pc_priv *priv = dev_get_priv(dev);
	struct ofnode_phandle_args args;

	priv->fpioa = dev_read_addr_ptr(dev);
	if (!priv->fpioa)
		return -EINVAL;

	ret = clk_get_by_index(dev, 0, &priv->clk);
	if (ret)
		return ret;

	ret = clk_enable(&priv->clk);
	if (ret && ret != -ENOSYS && ret != -ENOTSUPP)
		return ret;

	ret = dev_read_phandle_with_args(dev, "canaan,k210-sysctl-power",
					NULL, 1, 0, &args);
        if (ret)
		return ret;

	if (args.args_count != 1)
		return -EINVAL;

	priv->sysctl = syscon_node_to_regmap(args.node);
	if (IS_ERR(priv->sysctl))
		return PTR_ERR(priv->sysctl);

	priv->power_offset = args.args[0];

	debug("%s: fpioa = %p sysctl = %p power offset = %x\n", __func__,
	      priv->fpioa, (void *)priv->sysctl->ranges[0].start,
	      priv->power_offset);

	/* Init input ties */
	for (i = 0; i < ARRAY_SIZE(priv->fpioa->tie_en); i++) {
		u32 val = 0;

		for (j = 0; j < 32; j++)
			if (k210_pcf_infos[i * 32 + j].mode_id ==
			    K210_PC_DEFAULT_IN_TIE)
				val |= BIT(j);
		writel(val, &priv->fpioa->tie_en[i]);
		writel(val, &priv->fpioa->tie_val[i]);
	}

	return 0;
}

static const struct udevice_id k210_pc_ids[] = {
	{ .compatible = "canaan,k210-fpioa" },
	{ }
};

U_BOOT_DRIVER(pinctrl_k210) = {
	.name = "pinctrl_k210",
	.id = UCLASS_PINCTRL,
	.of_match = k210_pc_ids,
	.probe = k210_pc_probe,
	.priv_auto	= sizeof(struct k210_pc_priv),
	.ops = &k210_pc_pinctrl_ops,
};
