// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2014-2018 Renesas Electronics Europe Limited
 *
 * Phil Edworthy <phil.edworthy@renesas.com>
 * Based on a driver originally written by Michel Pollet at Renesas.
 */

#include <dt-bindings/pinctrl/rzn1-pinctrl.h>

#include <dm/device.h>
#include <dm/device_compat.h>
#include <dm/pinctrl.h>
#include <dm/read.h>
#include <regmap.h>

/* Field positions and masks in the pinmux registers */
#define RZN1_L1_PIN_DRIVE_STRENGTH	10
#define RZN1_L1_PIN_DRIVE_STRENGTH_4MA	0
#define RZN1_L1_PIN_DRIVE_STRENGTH_6MA	1
#define RZN1_L1_PIN_DRIVE_STRENGTH_8MA	2
#define RZN1_L1_PIN_DRIVE_STRENGTH_12MA	3
#define RZN1_L1_PIN_PULL		8
#define RZN1_L1_PIN_PULL_NONE		0
#define RZN1_L1_PIN_PULL_UP		1
#define RZN1_L1_PIN_PULL_DOWN		3
#define RZN1_L1_FUNCTION		0
#define RZN1_L1_FUNC_MASK		0xf
#define RZN1_L1_FUNCTION_L2		0xf

/*
 * The hardware manual describes two levels of multiplexing, but it's more
 * logical to think of the hardware as three levels, with level 3 consisting of
 * the multiplexing for Ethernet MDIO signals.
 *
 * Level 1 functions go from 0 to 9, with level 1 function '15' (0xf) specifying
 * that level 2 functions are used instead. Level 2 has a lot more options,
 * going from 0 to 61. Level 3 allows selection of MDIO functions which can be
 * floating, or one of seven internal peripherals. Unfortunately, there are two
 * level 2 functions that can select MDIO, and two MDIO channels so we have four
 * sets of level 3 functions.
 *
 * For this driver, we've compounded the numbers together, so:
 *    0 to   9 is level 1
 *   10 to  71 is 10 + level 2 number
 *   72 to  79 is 72 + MDIO0 source for level 2 MDIO function.
 *   80 to  87 is 80 + MDIO0 source for level 2 MDIO_E1 function.
 *   88 to  95 is 88 + MDIO1 source for level 2 MDIO function.
 *   96 to 103 is 96 + MDIO1 source for level 2 MDIO_E1 function.
 * Examples:
 *  Function 28 corresponds UART0
 *  Function 73 corresponds to MDIO0 to GMAC0
 *
 * There are 170 configurable pins (called PL_GPIO in the datasheet).
 */

/*
 * Structure detailing the HW registers on the RZ/N1 devices.
 * Both the Level 1 mux registers and Level 2 mux registers have the same
 * structure. The only difference is that Level 2 has additional MDIO registers
 * at the end.
 */
struct rzn1_pinctrl_regs {
	u32	conf[170];
	u32	pad0[86];
	u32	status_protect;	/* 0x400 */
	/* MDIO mux registers, level2 only */
	u32	l2_mdio[2];
};

#define NUM_CONF	ARRAY_SIZE(((struct rzn1_pinctrl_regs *)0)->conf)

#define level1_write(map, member, val) \
	regmap_range_set(map, 0, struct rzn1_pinctrl_regs, member, val)

#define level1_read(map, member, valp) \
	regmap_range_get(map, 0, struct rzn1_pinctrl_regs, member, valp)

#define level2_write(map, member, val) \
	regmap_range_set(map, 1, struct rzn1_pinctrl_regs, member, val)

#define level2_read(map, member, valp) \
	regmap_range_get(map, 1, struct rzn1_pinctrl_regs, member, valp)

/**
 * struct rzn1_pmx_func - describes rzn1 pinmux functions
 * @name: the name of this specific function
 * @groups: corresponding pin groups
 * @num_groups: the number of groups
 */
struct rzn1_pmx_func {
	const char *name;
	const char **groups;
	unsigned int num_groups;
};

/**
 * struct rzn1_pin_group - describes an rzn1 pin group
 * @name: the name of this specific pin group
 * @func: the name of the function selected by this group
 * @npins: the number of pins in this group array, i.e. the number of
 *	elements in .pins so we can iterate over that array
 * @pins: array of pins. Needed due to pinctrl_ops.get_group_pins()
 * @pin_ids: array of pin_ids, i.e. the value used to select the mux
 */
struct rzn1_pin_group {
	const char *name;
	const char *func;
	unsigned int npins;
	unsigned int *pins;
	u8 *pin_ids;
};

struct rzn1_pinctrl {
	struct device *dev;
	struct clk *clk;
	struct pinctrl_dev *pctl;
	u32 lev1_protect_phys;
	u32 lev2_protect_phys;
	int mdio_func[2];

	struct rzn1_pin_group *groups;
	unsigned int ngroups;

	struct rzn1_pmx_func *functions;
	unsigned int nfunctions;
};

struct rzn1_pinctrl_priv {
	struct regmap *regmap;
	u32 lev1_protect_phys;
	u32 lev2_protect_phys;

	struct clk *clk;
};

enum {
	LOCK_LEVEL1 = 0x1,
	LOCK_LEVEL2 = 0x2,
	LOCK_ALL = LOCK_LEVEL1 | LOCK_LEVEL2,
};

static void rzn1_hw_set_lock(struct rzn1_pinctrl_priv *priv, u8 lock, u8 value)
{
	/*
	 * The pinmux configuration is locked by writing the physical address of
	 * the status_protect register to itself. It is unlocked by writing the
	 * address | 1.
	 */
	if (lock & LOCK_LEVEL1) {
		u32 val = priv->lev1_protect_phys | !(value & LOCK_LEVEL1);

		level1_write(priv->regmap, status_protect, val);
	}

	if (lock & LOCK_LEVEL2) {
		u32 val = priv->lev2_protect_phys | !(value & LOCK_LEVEL2);

		level2_write(priv->regmap, status_protect, val);
	}
}

static void rzn1_pinctrl_mdio_select(struct rzn1_pinctrl_priv *priv, int mdio,
				     u32 func)
{
	debug("setting mdio%d to %u\n", mdio, func);

	level2_write(priv->regmap, l2_mdio[mdio], func);
}

/*
 * Using a composite pin description, set the hardware pinmux registers
 * with the corresponding values.
 * Make sure to unlock write protection and reset it afterward.
 *
 * NOTE: There is no protection for potential concurrency, it is assumed these
 * calls are serialized already.
 */
static int rzn1_set_hw_pin_func(struct rzn1_pinctrl_priv *priv,
				unsigned int pin, unsigned int func)
{
	u32 l1_cache;
	u32 l2_cache;
	u32 l1;
	u32 l2;

	/* Level 3 MDIO multiplexing */
	if (func >= RZN1_FUNC_MDIO0_HIGHZ &&
	    func <= RZN1_FUNC_MDIO1_E1_SWITCH) {
		int mdio_channel;
		u32 mdio_func;

		if (func <= RZN1_FUNC_MDIO1_HIGHZ)
			mdio_channel = 0;
		else
			mdio_channel = 1;

		/* Get MDIO func, and convert the func to the level 2 number */
		if (func <= RZN1_FUNC_MDIO0_SWITCH) {
			mdio_func = func - RZN1_FUNC_MDIO0_HIGHZ;
			func = RZN1_FUNC_ETH_MDIO;
		} else if (func <= RZN1_FUNC_MDIO0_E1_SWITCH) {
			mdio_func = func - RZN1_FUNC_MDIO0_E1_HIGHZ;
			func = RZN1_FUNC_ETH_MDIO_E1;
		} else if (func <= RZN1_FUNC_MDIO1_SWITCH) {
			mdio_func = func - RZN1_FUNC_MDIO1_HIGHZ;
			func = RZN1_FUNC_ETH_MDIO;
		} else {
			mdio_func = func - RZN1_FUNC_MDIO1_E1_HIGHZ;
			func = RZN1_FUNC_ETH_MDIO_E1;
		}
		rzn1_pinctrl_mdio_select(priv, mdio_channel, mdio_func);
	}

	/* Note here, we do not allow anything past the MDIO Mux values */
	if (pin >= NUM_CONF || func >= RZN1_FUNC_MDIO0_HIGHZ)
		return -EINVAL;

	level1_read(priv->regmap, conf[pin], &l1);
	l1_cache = l1;
	level2_read(priv->regmap, conf[pin], &l2);
	l2_cache = l2;

	debug("setting func for pin %u to %u\n", pin, func);

	l1 &= ~(RZN1_L1_FUNC_MASK << RZN1_L1_FUNCTION);

	if (func < RZN1_FUNC_L2_OFFSET) {
		l1 |= (func << RZN1_L1_FUNCTION);
	} else {
		l1 |= (RZN1_L1_FUNCTION_L2 << RZN1_L1_FUNCTION);

		l2 = func - RZN1_FUNC_L2_OFFSET;
	}

	/* If either configuration changes, we update both anyway */
	if (l1 != l1_cache || l2 != l2_cache) {
		level1_write(priv->regmap, conf[pin], l1);
		level2_write(priv->regmap, conf[pin], l2);
	}

	return 0;
}

static int rzn1_pinconf_set(struct rzn1_pinctrl_priv *priv, unsigned int pin,
			    unsigned int bias, unsigned int strength)
{
	u32 l1, l1_cache;
	u32 drv = RZN1_L1_PIN_DRIVE_STRENGTH_8MA;

	level1_read(priv->regmap, conf[pin], &l1);
	l1_cache = l1;

	switch (bias) {
	case PIN_CONFIG_BIAS_PULL_UP:
		debug("set pin %d pull up\n", pin);
		l1 &= ~(0x3 << RZN1_L1_PIN_PULL);
		l1 |= (RZN1_L1_PIN_PULL_UP << RZN1_L1_PIN_PULL);
		break;
	case PIN_CONFIG_BIAS_PULL_DOWN:
		debug("set pin %d pull down\n", pin);
		l1 &= ~(0x3 << RZN1_L1_PIN_PULL);
		l1 |= (RZN1_L1_PIN_PULL_DOWN << RZN1_L1_PIN_PULL);
		break;
	case PIN_CONFIG_BIAS_DISABLE:
		debug("set pin %d bias off\n", pin);
		l1 &= ~(0x3 << RZN1_L1_PIN_PULL);
		l1 |= (RZN1_L1_PIN_PULL_NONE << RZN1_L1_PIN_PULL);
		break;
	}

	switch (strength) {
	case 4:
		drv = RZN1_L1_PIN_DRIVE_STRENGTH_4MA;
		break;
	case 6:
		drv = RZN1_L1_PIN_DRIVE_STRENGTH_6MA;
		break;
	case 8:
		drv = RZN1_L1_PIN_DRIVE_STRENGTH_8MA;
		break;
	case 12:
		drv = RZN1_L1_PIN_DRIVE_STRENGTH_12MA;
		break;
	}

	debug("set pin %d drv %umA\n", pin, drv);

	l1 &= ~(0x3 << RZN1_L1_PIN_DRIVE_STRENGTH);
	l1 |= (drv << RZN1_L1_PIN_DRIVE_STRENGTH);

	if (l1 != l1_cache)
		level1_write(priv->regmap, conf[pin], l1);

	return 0;
}

static int rzn1_pinctrl_set_state(struct udevice *dev, struct udevice *config)
{
	struct rzn1_pinctrl_priv *priv = dev_get_priv(dev);
	int size;
	int ret;
	u32 val;
	u32 bias;

	/* Pullup/down bias, common to all pins in group */
	bias = PIN_CONFIG_BIAS_PULL_UP;
	if (dev_read_bool(config, "bias-disable"))
		bias = PIN_CONFIG_BIAS_DISABLE;
	else if (dev_read_bool(config, "bias-pull-up"))
		bias = PIN_CONFIG_BIAS_PULL_UP;
	else if (dev_read_bool(config, "bias-pull-down"))
		bias = PIN_CONFIG_BIAS_PULL_DOWN;

	/* Drive strength, common to all pins in group */
	u32 strength = dev_read_u32_default(config, "drive-strength", 8);

	/* Number of pins */
	ret = dev_read_size(config, "pinmux");
	if (ret < 0)
		return ret;

	size = ret / sizeof(val);

	for (int i = 0; i < size; i++) {
		ret = dev_read_u32_index(config, "pinmux", i, &val);
		if (ret)
			return ret;
		unsigned int pin = val & 0xff;
		unsigned int func = val >> 8;

		debug("%s pin %d func %d bias %d strength %d\n",
		      config->name, pin, func, bias, strength);

		rzn1_hw_set_lock(priv, LOCK_ALL, LOCK_ALL);
		rzn1_set_hw_pin_func(priv, pin, func);
		rzn1_pinconf_set(priv, pin, bias, strength);
		rzn1_hw_set_lock(priv, LOCK_ALL, 0);
	}

	return 0;
}

static struct pinctrl_ops rzn1_pinctrl_ops = {
	.set_state = rzn1_pinctrl_set_state,
};

static int rzn1_pinctrl_probe(struct udevice *dev)
{
	struct rzn1_pinctrl_priv *priv = dev_get_priv(dev);
	ofnode node = dev_ofnode(dev);
	int ret;

	ret = regmap_init_mem(node, &priv->regmap);
	if (ret)
		return ret;

	priv->lev1_protect_phys = (u32)regmap_get_range(priv->regmap, 0) +
		offsetof(struct rzn1_pinctrl_regs, status_protect);
	priv->lev2_protect_phys = (u32)regmap_get_range(priv->regmap, 1) +
		offsetof(struct rzn1_pinctrl_regs, status_protect);

	return 0;
}

static const struct udevice_id rzn1_pinctrl_ids[] = {
	{ .compatible = "renesas,rzn1-pinctrl", },
	{ },
};

U_BOOT_DRIVER(pinctrl_rzn1) = {
	.name	= "rzn1-pinctrl",
	.id	= UCLASS_PINCTRL,
	.of_match = rzn1_pinctrl_ids,
	.priv_auto = sizeof(struct rzn1_pinctrl_priv),
	.ops = &rzn1_pinctrl_ops,
	.probe = rzn1_pinctrl_probe,
	.flags = DM_FLAG_PRE_RELOC,
};
