// SPDX-License-Identifier: GPL-2.0+
/*
 * Atmel PIO pinctrl driver
 *
 * Copyright (C) 2016 Atmel Corporation
 *               Wenyou.Yang <wenyou.yang@atmel.com>
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <asm/global_data.h>
#include <dm/pinctrl.h>
#include <asm/hardware.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <linux/err.h>
#include <mach/at91_pio.h>

DECLARE_GLOBAL_DATA_PTR;

#define MAX_GPIO_BANKS		5
#define MAX_NB_GPIO_PER_BANK	32

#define MAX_PINMUX_ENTRIES	200

struct at91_pinctrl_priv {
	struct at91_port *reg_base[MAX_GPIO_BANKS];
	u32 nbanks;
};

#define PULL_UP			BIT(0)
#define MULTI_DRIVE		BIT(1)
#define DEGLITCH		BIT(2)
#define PULL_DOWN		BIT(3)
#define DIS_SCHMIT		BIT(4)
#define DRIVE_STRENGTH_SHIFT	5
#define DRIVE_STRENGTH_MASK	0x3
#define DRIVE_STRENGTH		(DRIVE_STRENGTH_MASK << DRIVE_STRENGTH_SHIFT)
#define OUTPUT			BIT(7)
#define OUTPUT_VAL_SHIFT	8
#define OUTPUT_VAL		(0x1 << OUTPUT_VAL_SHIFT)
#define SLEWRATE_SHIFT	9
#define SLEWRATE_MASK	0x1
#define SLEWRATE	(SLEWRATE_MASK << SLEWRATE_SHIFT)
#define DEBOUNCE		BIT(16)
#define DEBOUNCE_VAL_SHIFT	17
#define DEBOUNCE_VAL		(0x3fff << DEBOUNCE_VAL_SHIFT)

/**
 * These defines will translated the dt binding settings to our internal
 * settings. They are not necessarily the same value as the register setting.
 * The actual drive strength current of low, medium and high must be looked up
 * from the corresponding device datasheet. This value is different for pins
 * that are even in the same banks. It is also dependent on VCC.
 * DRIVE_STRENGTH_DEFAULT is just a placeholder to avoid changing the drive
 * strength when there is no dt config for it.
 */
enum drive_strength_bit {
	DRIVE_STRENGTH_BIT_DEF,
	DRIVE_STRENGTH_BIT_LOW,
	DRIVE_STRENGTH_BIT_MED,
	DRIVE_STRENGTH_BIT_HI,
};

#define DRIVE_STRENGTH_BIT_MSK(name)	(DRIVE_STRENGTH_BIT_##name << \
					 DRIVE_STRENGTH_SHIFT)

enum slewrate_bit {
	SLEWRATE_BIT_DIS,
	SLEWRATE_BIT_ENA,
};

#define SLEWRATE_BIT_MSK(name)		(SLEWRATE_BIT_##name << SLEWRATE_SHIFT)

enum at91_mux {
	AT91_MUX_GPIO = 0,
	AT91_MUX_PERIPH_A = 1,
	AT91_MUX_PERIPH_B = 2,
	AT91_MUX_PERIPH_C = 3,
	AT91_MUX_PERIPH_D = 4,
};

/**
 * struct at91_pinctrl_mux_ops - describes an AT91 mux ops group
 * on new IP with support for periph C and D the way to mux in
 * periph A and B has changed
 * So provide the right callbacks
 * if not present means the IP does not support it
 * @mux_A_periph: assign the corresponding pin to the peripheral A function.
 * @mux_B_periph: assign the corresponding pin to the peripheral B function.
 * @mux_C_periph: assign the corresponding pin to the peripheral C function.
 * @mux_D_periph: assign the corresponding pin to the peripheral D function.
 * @set_deglitch: enable/disable the deglitch feature.
 * @set_debounce: enable/disable the debounce feature.
 * @set_pulldown: enable/disable the pulldown feature.
 * @disable_schmitt_trig: disable schmitt trigger
 */
struct at91_pinctrl_mux_ops {
	void (*mux_A_periph)(struct at91_port *pio, u32 mask);
	void (*mux_B_periph)(struct at91_port *pio, u32 mask);
	void (*mux_C_periph)(struct at91_port *pio, u32 mask);
	void (*mux_D_periph)(struct at91_port *pio, u32 mask);
	void (*set_deglitch)(struct at91_port *pio, u32 mask, bool is_on);
	void (*set_debounce)(struct at91_port *pio, u32 mask, bool is_on,
			     u32 div);
	void (*set_pulldown)(struct at91_port *pio, u32 mask, bool is_on);
	void (*disable_schmitt_trig)(struct at91_port *pio, u32 mask);
	void (*set_drivestrength)(struct at91_port *pio, u32 pin,
				  u32 strength);
	void (*set_slewrate)(struct at91_port *pio, u32 pin, u32 slewrate);
};

static u32 two_bit_pin_value_shift_amount(u32 pin)
{
	/* return the shift value for a pin for "two bit" per pin registers,
	 * i.e. drive strength */
	return 2 * ((pin >= MAX_NB_GPIO_PER_BANK/2)
			? pin - MAX_NB_GPIO_PER_BANK/2 : pin);
}

static void at91_mux_disable_interrupt(struct at91_port *pio, u32 mask)
{
	writel(mask, &pio->idr);
}

static void at91_mux_set_pullup(struct at91_port *pio, u32 mask, bool on)
{
	if (on)
		writel(mask, &pio->mux.pio3.ppddr);

	writel(mask, (on ? &pio->puer : &pio->pudr));
}

static void at91_mux_set_output(struct at91_port *pio, unsigned mask,
				bool is_on, bool val)
{
	writel(mask, (val ? &pio->sodr : &pio->codr));
	writel(mask, (is_on ? &pio->oer : &pio->odr));
}

static void at91_mux_set_multidrive(struct at91_port *pio, u32 mask, bool on)
{
	writel(mask, (on ? &pio->mder : &pio->mddr));
}

static void at91_mux_set_A_periph(struct at91_port *pio, u32 mask)
{
	writel(mask, &pio->mux.pio2.asr);
}

static void at91_mux_set_B_periph(struct at91_port *pio, u32 mask)
{
	writel(mask, &pio->mux.pio2.bsr);
}

static void at91_mux_pio3_set_A_periph(struct at91_port *pio, u32 mask)
{
	writel(readl(&pio->mux.pio3.abcdsr1) & ~mask, &pio->mux.pio3.abcdsr1);
	writel(readl(&pio->mux.pio3.abcdsr2) & ~mask, &pio->mux.pio3.abcdsr2);
}

static void at91_mux_pio3_set_B_periph(struct at91_port *pio, u32 mask)
{
	writel(readl(&pio->mux.pio3.abcdsr1) | mask, &pio->mux.pio3.abcdsr1);
	writel(readl(&pio->mux.pio3.abcdsr2) & ~mask, &pio->mux.pio3.abcdsr2);
}

static void at91_mux_pio3_set_C_periph(struct at91_port *pio, u32 mask)
{
	writel(readl(&pio->mux.pio3.abcdsr1) & ~mask, &pio->mux.pio3.abcdsr1);
	writel(readl(&pio->mux.pio3.abcdsr2) | mask, &pio->mux.pio3.abcdsr2);
}

static void at91_mux_pio3_set_D_periph(struct at91_port *pio, u32 mask)
{
	writel(readl(&pio->mux.pio3.abcdsr1) | mask, &pio->mux.pio3.abcdsr1);
	writel(readl(&pio->mux.pio3.abcdsr2) | mask, &pio->mux.pio3.abcdsr2);
}

static void at91_mux_set_deglitch(struct at91_port *pio, u32 mask, bool is_on)
{
	writel(mask, (is_on ? &pio->ifer : &pio->ifdr));
}

static void at91_mux_pio3_set_deglitch(struct at91_port *pio,
				       u32 mask, bool is_on)
{
	if (is_on)
		writel(mask, &pio->mux.pio3.ifscdr);
	at91_mux_set_deglitch(pio, mask, is_on);
}

static void at91_mux_pio3_set_debounce(struct at91_port *pio, u32 mask,
				       bool is_on, u32 div)
{
	if (is_on) {
		writel(mask, &pio->mux.pio3.ifscer);
		writel(div & PIO_SCDR_DIV, &pio->mux.pio3.scdr);
		writel(mask, &pio->ifer);
	} else {
		writel(mask, &pio->mux.pio3.ifscdr);
	}
}

static void at91_mux_pio3_set_pulldown(struct at91_port *pio,
				       u32 mask, bool is_on)
{
	if (is_on)
		writel(mask, &pio->pudr);

	writel(mask, (is_on ? &pio->mux.pio3.ppder : &pio->mux.pio3.ppddr));
}

static void at91_mux_pio3_disable_schmitt_trig(struct at91_port *pio,
					       u32 mask)
{
	writel(readl(&pio->schmitt) | mask, &pio->schmitt);
}

static void set_drive_strength(void *reg, u32 pin, u32 strength)
{
	u32 shift = two_bit_pin_value_shift_amount(pin);

	clrsetbits_le32(reg, DRIVE_STRENGTH_MASK << shift, strength << shift);
}

static void at91_mux_sama5d3_set_drivestrength(struct at91_port *pio,
					       u32 pin, u32 setting)
{
	void *reg;

	reg = &pio->driver12;
	if (pin >= MAX_NB_GPIO_PER_BANK / 2)
		reg = &pio->driver2;

	/* do nothing if setting is zero */
	if (!setting)
		return;

	/* strength is 1 to 1 with setting for SAMA5 */
	set_drive_strength(reg, pin, setting);
}

static void at91_mux_sam9x5_set_drivestrength(struct at91_port *pio,
					      u32 pin, u32 setting)
{
	void *reg;

	reg = &pio->driver1;
	if (pin >= MAX_NB_GPIO_PER_BANK / 2)
		reg = &pio->driver12;

	/* do nothing if setting is zero */
	if (!setting)
		return;

	/* strength is inverse on SAM9x5s with our defines
	 * 0 = hi, 1 = med, 2 = low, 3 = rsvd */
	setting = DRIVE_STRENGTH_BIT_MSK(HI) - setting;

	set_drive_strength(reg, pin, setting);
}

static void at91_mux_sam9x60_set_drivestrength(struct at91_port *pio, u32 pin,
					       u32 setting)
{
	void *reg = &pio->driver12;
	u32 tmp;

	if (setting <= DRIVE_STRENGTH_BIT_DEF ||
	    setting == DRIVE_STRENGTH_BIT_MED ||
	    setting > DRIVE_STRENGTH_BIT_HI)
		return;

	tmp = readl(reg);

	/* Strength is 0: low, 1: hi */
	if (setting == DRIVE_STRENGTH_BIT_LOW)
		tmp &= ~BIT(pin);
	else
		tmp |= BIT(pin);

	writel(tmp, reg);
}

static void at91_mux_sam9x60_set_slewrate(struct at91_port *pio, u32 pin,
					  u32 setting)
{
	void *reg = &pio->reserved12[3];
	u32 tmp;

	if (setting < SLEWRATE_BIT_DIS || setting > SLEWRATE_BIT_ENA)
		return;

	tmp = readl(reg);

	if (setting == SLEWRATE_BIT_DIS)
		tmp &= ~BIT(pin);
	else
		tmp |= BIT(pin);

	writel(tmp, reg);
}

static struct at91_pinctrl_mux_ops at91rm9200_ops = {
	.mux_A_periph	= at91_mux_set_A_periph,
	.mux_B_periph	= at91_mux_set_B_periph,
	.set_deglitch	= at91_mux_set_deglitch,
};

static struct at91_pinctrl_mux_ops at91sam9x5_ops = {
	.mux_A_periph	= at91_mux_pio3_set_A_periph,
	.mux_B_periph	= at91_mux_pio3_set_B_periph,
	.mux_C_periph	= at91_mux_pio3_set_C_periph,
	.mux_D_periph	= at91_mux_pio3_set_D_periph,
	.set_deglitch	= at91_mux_pio3_set_deglitch,
	.set_debounce	= at91_mux_pio3_set_debounce,
	.set_pulldown	= at91_mux_pio3_set_pulldown,
	.disable_schmitt_trig = at91_mux_pio3_disable_schmitt_trig,
	.set_drivestrength = at91_mux_sam9x5_set_drivestrength,
};

static struct at91_pinctrl_mux_ops sama5d3_ops = {
	.mux_A_periph	= at91_mux_pio3_set_A_periph,
	.mux_B_periph	= at91_mux_pio3_set_B_periph,
	.mux_C_periph	= at91_mux_pio3_set_C_periph,
	.mux_D_periph	= at91_mux_pio3_set_D_periph,
	.set_deglitch	= at91_mux_pio3_set_deglitch,
	.set_debounce	= at91_mux_pio3_set_debounce,
	.set_pulldown	= at91_mux_pio3_set_pulldown,
	.disable_schmitt_trig = at91_mux_pio3_disable_schmitt_trig,
	.set_drivestrength = at91_mux_sama5d3_set_drivestrength,
};

static struct at91_pinctrl_mux_ops sam9x60_ops = {
	.mux_A_periph	= at91_mux_pio3_set_A_periph,
	.mux_B_periph	= at91_mux_pio3_set_B_periph,
	.mux_C_periph	= at91_mux_pio3_set_C_periph,
	.mux_D_periph	= at91_mux_pio3_set_D_periph,
	.set_deglitch	= at91_mux_pio3_set_deglitch,
	.set_debounce	= at91_mux_pio3_set_debounce,
	.set_pulldown	= at91_mux_pio3_set_pulldown,
	.disable_schmitt_trig = at91_mux_pio3_disable_schmitt_trig,
	.set_drivestrength = at91_mux_sam9x60_set_drivestrength,
	.set_slewrate   = at91_mux_sam9x60_set_slewrate,
};

static void at91_mux_gpio_disable(struct at91_port *pio, u32 mask)
{
	writel(mask, &pio->pdr);
}

static void at91_mux_gpio_enable(struct at91_port *pio, u32 mask, bool input)
{
	writel(mask, &pio->per);
	writel(mask, (input ? &pio->odr : &pio->oer));
}

static int at91_pmx_set(struct at91_pinctrl_mux_ops *ops,
			struct at91_port *pio, u32 mask, enum at91_mux mux)
{
	at91_mux_disable_interrupt(pio, mask);
	switch (mux) {
	case AT91_MUX_GPIO:
		at91_mux_gpio_enable(pio, mask, 1);
		break;
	case AT91_MUX_PERIPH_A:
		ops->mux_A_periph(pio, mask);
		break;
	case AT91_MUX_PERIPH_B:
		ops->mux_B_periph(pio, mask);
		break;
	case AT91_MUX_PERIPH_C:
		if (!ops->mux_C_periph)
			return -EINVAL;
		ops->mux_C_periph(pio, mask);
		break;
	case AT91_MUX_PERIPH_D:
		if (!ops->mux_D_periph)
			return -EINVAL;
		ops->mux_D_periph(pio, mask);
		break;
	}
	if (mux)
		at91_mux_gpio_disable(pio, mask);

	return 0;
}

static int at91_pinconf_set(struct at91_pinctrl_mux_ops *ops,
			    struct at91_port *pio, u32 pin, u32 config)
{
	u32 mask = BIT(pin);

	if ((config & PULL_UP) && (config & PULL_DOWN))
		return -EINVAL;

	at91_mux_set_output(pio, mask, config & OUTPUT,
			    (config & OUTPUT_VAL) >> OUTPUT_VAL_SHIFT);
	at91_mux_set_pullup(pio, mask, config & PULL_UP);
	at91_mux_set_multidrive(pio, mask, config & MULTI_DRIVE);
	if (ops->set_deglitch)
		ops->set_deglitch(pio, mask, config & DEGLITCH);
	if (ops->set_debounce)
		ops->set_debounce(pio, mask, config & DEBOUNCE,
			(config & DEBOUNCE_VAL) >> DEBOUNCE_VAL_SHIFT);
	if (ops->set_pulldown)
		ops->set_pulldown(pio, mask, config & PULL_DOWN);
	if (ops->disable_schmitt_trig && config & DIS_SCHMIT)
		ops->disable_schmitt_trig(pio, mask);
	if (ops->set_drivestrength)
		ops->set_drivestrength(pio, pin,
			(config & DRIVE_STRENGTH) >> DRIVE_STRENGTH_SHIFT);
	if (ops->set_slewrate)
		ops->set_slewrate(pio, pin,
			(config & SLEWRATE) >> SLEWRATE_SHIFT);

	return 0;
}

static int at91_pin_check_config(struct udevice *dev, u32 bank, u32 pin)
{
	struct at91_pinctrl_priv *priv = dev_get_priv(dev);

	if (bank >= priv->nbanks) {
		debug("pin conf bank %d >= nbanks %d\n", bank, priv->nbanks);
		return -EINVAL;
	}

	if (pin >= MAX_NB_GPIO_PER_BANK) {
		debug("pin conf pin %d >= %d\n", pin, MAX_NB_GPIO_PER_BANK);
		return -EINVAL;
	}

	return 0;
}

static int at91_pinctrl_set_state(struct udevice *dev, struct udevice *config)
{
	struct at91_pinctrl_priv *priv = dev_get_priv(dev);
	const void *blob = gd->fdt_blob;
	int node = dev_of_offset(config);
	u32 cells[MAX_PINMUX_ENTRIES];
	const u32 *list = cells;
	u32 bank, pin;
	u32 conf, mask, count, i;
	int size;
	int ret;
	enum at91_mux mux;
	struct at91_port *pio;
	struct at91_pinctrl_mux_ops *ops =
			(struct at91_pinctrl_mux_ops *)dev_get_driver_data(dev);

	/*
	 * the binding format is atmel,pins = <bank pin mux CONFIG ...>,
	 * do sanity check and calculate pins number
	 */
	size = fdtdec_get_int_array_count(blob, node, "atmel,pins",
					  cells, ARRAY_SIZE(cells));

	/* we do not check return since it's safe node passed down */
	count = size >> 2;
	if (!count)
		return -EINVAL;

	for (i = 0; i < count; i++) {
		bank = *list++;
		pin = *list++;
		mux = *list++;
		conf = *list++;

		ret = at91_pin_check_config(dev, bank, pin);
		if (ret)
			return ret;

		pio = priv->reg_base[bank];
		mask = BIT(pin);

		ret = at91_pmx_set(ops, pio, mask, mux);
		if (ret)
			return ret;

		ret = at91_pinconf_set(ops, pio, pin, conf);
		if (ret)
			return ret;
	}

	return 0;
}

const struct pinctrl_ops at91_pinctrl_ops  = {
	.set_state = at91_pinctrl_set_state,
};

static int at91_pinctrl_probe(struct udevice *dev)
{
	struct at91_pinctrl_priv *priv = dev_get_priv(dev);
	fdt_addr_t addr_base;
	int index;

	for (index = 0; index < MAX_GPIO_BANKS; index++) {
		addr_base = devfdt_get_addr_index(dev, index);
		if (addr_base == FDT_ADDR_T_NONE)
			break;

		priv->reg_base[index] = (struct at91_port *)addr_base;
	}

	priv->nbanks = index;

	return 0;
}

static const struct udevice_id at91_pinctrl_match[] = {
	{ .compatible = "atmel,sama5d3-pinctrl", .data = (ulong)&sama5d3_ops },
	{ .compatible = "atmel,at91sam9x5-pinctrl", .data = (ulong)&at91sam9x5_ops },
	{ .compatible = "atmel,at91rm9200-pinctrl", .data = (ulong)&at91rm9200_ops },
	{ .compatible = "microchip,sam9x60-pinctrl", .data = (ulong)&sam9x60_ops },
	{}
};

U_BOOT_DRIVER(atmel_sama5d3_pinctrl) = {
	.name = "atmel_sama5d3_pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = at91_pinctrl_match,
	.probe = at91_pinctrl_probe,
	.priv_auto	= sizeof(struct at91_pinctrl_priv),
	.ops = &at91_pinctrl_ops,
};

DM_DRIVER_ALIAS(atmel_sama5d3_pinctrl, atmel_at91rm9200_pinctrl)
