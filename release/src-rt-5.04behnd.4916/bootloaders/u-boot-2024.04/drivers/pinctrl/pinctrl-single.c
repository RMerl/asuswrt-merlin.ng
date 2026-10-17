// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) EETS GmbH, 2017, Felix Brack <f.brack@eets.ch>
 * Copyright (C) 2021 Dario Binacchi <dariobin@libero.it>
 */

#include <common.h>
#include <mapmem.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <dm/of_access.h>
#include <dm/pinctrl.h>
#include <linux/libfdt.h>
#include <linux/list.h>
#include <asm/io.h>
#include <sort.h>

/**
 * struct single_pdata - platform data
 * @base: first configuration register
 * @offset: index of last configuration register
 * @mask: configuration-value mask bits
 * @width: configuration register bit width
 * @bits_per_mux: true if one register controls more than one pin
 */
struct single_pdata {
	void *base;
	int offset;
	u32 mask;
	u32 width;
	u32 args_count;
	bool bits_per_mux;
};

/**
 * struct single_func - pinctrl function
 * @node: list node
 * @name: pinctrl function name
 * @npins: number of entries in pins array
 * @pins: pins array
 */
struct single_func {
	struct list_head node;
	const char *name;
	unsigned int npins;
	unsigned int *pins;
};

/**
 * struct single_gpiofunc_range - pin ranges with same mux value of gpio fun
 * @offset: offset base of pins
 * @npins: number pins with the same mux value of gpio function
 * @gpiofunc: mux value of gpio function
 * @node: list node
 */
struct single_gpiofunc_range {
	u32 offset;
	u32 npins;
	u32 gpiofunc;
	struct list_head node;
};

/**
 * struct single_priv - private data
 * @bits_per_pin: number of bits per pin
 * @npins: number of selectable pins
 * @pin_name: temporary buffer to store the pin name
 * @functions: list pin functions
 * @gpiofuncs: list gpio functions
 */
struct single_priv {
#if (IS_ENABLED(CONFIG_SANDBOX))
	u32 *sandbox_regs;
#endif
	unsigned int bits_per_pin;
	unsigned int npins;
	char pin_name[PINNAME_SIZE];
	struct list_head functions;
	struct list_head gpiofuncs;
};

/**
 * struct single_fdt_bits_cfg - pin configuration
 *
 * This structure is used for the pin configuration parameters in case
 * the register controls more than one pin.
 *
 * @reg: configuration register offset
 * @val: configuration register value
 * @mask: configuration register mask
 */
struct single_fdt_bits_cfg {
	fdt32_t reg;
	fdt32_t val;
	fdt32_t mask;
};

#if (!IS_ENABLED(CONFIG_SANDBOX))

static unsigned int single_read(struct udevice *dev, void *reg)
{
	struct single_pdata *pdata = dev_get_plat(dev);

	switch (pdata->width) {
	case 8:
		return readb(reg);
	case 16:
		return readw(reg);
	default: /* 32 bits */
		return readl(reg);
	}

	return readb(reg);
}

static void single_write(struct udevice *dev, unsigned int val, void *reg)
{
	struct single_pdata *pdata = dev_get_plat(dev);

	switch (pdata->width) {
	case 8:
		writeb(val, reg);
		break;
	case 16:
		writew(val, reg);
		break;
	default: /* 32 bits */
		writel(val, reg);
	}
}

#else /* CONFIG_SANDBOX  */

static unsigned int single_read(struct udevice *dev, void *reg)
{
	struct single_priv *priv = dev_get_priv(dev);

	return priv->sandbox_regs[map_to_sysmem(reg)];
}

static void single_write(struct udevice *dev, unsigned int val, void *reg)
{
	struct single_priv *priv = dev_get_priv(dev);

	priv->sandbox_regs[map_to_sysmem(reg)] = val;
}

#endif /* CONFIG_SANDBOX  */

/**
 * single_get_pin_by_offset() - get a pin based on the register offset
 * @dev: single driver instance
 * @offset: register offset from the base
 */
static int single_get_pin_by_offset(struct udevice *dev, unsigned int offset)
{
	struct single_pdata *pdata = dev_get_plat(dev);
	struct single_priv *priv = dev_get_priv(dev);

	if (offset > pdata->offset) {
		dev_err(dev, "mux offset out of range: 0x%x (0x%x)\n",
			offset, pdata->offset);
		return -EINVAL;
	}

	if (pdata->bits_per_mux)
		return (offset * BITS_PER_BYTE) / priv->bits_per_pin;

	return offset / (pdata->width / BITS_PER_BYTE);
}

static int single_get_offset_by_pin(struct udevice *dev, unsigned int pin)
{
	struct single_pdata *pdata = dev_get_plat(dev);
	struct single_priv *priv = dev_get_priv(dev);
	unsigned int mux_bytes;

	if (pin >= priv->npins)
		return -EINVAL;

	mux_bytes = pdata->width / BITS_PER_BYTE;
	if (pdata->bits_per_mux) {
		int byte_num;

		byte_num = (priv->bits_per_pin * pin) / BITS_PER_BYTE;
		return (byte_num / mux_bytes) * mux_bytes;
	}

	return pin * mux_bytes;
}

static const char *single_get_pin_function(struct udevice *dev,
					   unsigned int pin)
{
	struct single_priv *priv = dev_get_priv(dev);
	struct single_func *func;
	int i;

	list_for_each_entry(func, &priv->functions, node) {
		for (i = 0; i < func->npins; i++) {
			if (pin == func->pins[i])
				return func->name;

			if (pin < func->pins[i])
				break;
		}
	}

	return NULL;
}

static int single_get_pin_muxing(struct udevice *dev, unsigned int pin,
				 char *buf, int size)
{
	struct single_pdata *pdata = dev_get_plat(dev);
	struct single_priv *priv = dev_get_priv(dev);
	phys_addr_t phys_reg;
	void *reg;
	const char *fname;
	unsigned int val;
	int offset, pin_shift = 0;

	offset = single_get_offset_by_pin(dev, pin);
	if (offset < 0)
		return offset;

	reg = pdata->base + offset;
	val = single_read(dev, reg);

	phys_reg = map_to_sysmem(reg);

	if (pdata->bits_per_mux)
		pin_shift = pin % (pdata->width / priv->bits_per_pin) *
			priv->bits_per_pin;

	val &= (pdata->mask << pin_shift);
	fname = single_get_pin_function(dev, pin);
	snprintf(buf, size, "%pa 0x%08x %s", &phys_reg, val,
		 fname ? fname : "UNCLAIMED");
	return 0;
}

static int single_request(struct udevice *dev, int pin, int flags)
{
	struct single_priv *priv = dev_get_priv(dev);
	struct single_pdata *pdata = dev_get_plat(dev);
	struct single_gpiofunc_range *frange = NULL;
	struct list_head *pos, *tmp;
	void *reg;
	int mux_bytes = 0;
	u32 data;

	/* If function mask is null, needn't enable it. */
	if (!pdata->mask)
		return -ENOTSUPP;

	list_for_each_safe(pos, tmp, &priv->gpiofuncs) {
		frange = list_entry(pos, struct single_gpiofunc_range, node);
		if ((pin >= frange->offset + frange->npins) ||
		    pin < frange->offset)
			continue;

		mux_bytes = pdata->width / BITS_PER_BYTE;
		reg = pdata->base + pin * mux_bytes;

		data = single_read(dev, reg);
		data &= ~pdata->mask;
		data |= frange->gpiofunc;
		single_write(dev, data, reg);
		break;
	}

	return 0;
}

static struct single_func *single_allocate_function(struct udevice *dev,
						    unsigned int group_pins)
{
	struct single_func *func;

	func = devm_kmalloc(dev, sizeof(*func), GFP_KERNEL);
	if (!func)
		return ERR_PTR(-ENOMEM);

	func->pins = devm_kmalloc(dev, sizeof(unsigned int) * group_pins,
				  GFP_KERNEL);
	if (!func->pins)
		return ERR_PTR(-ENOMEM);

	return func;
}

static int single_pin_compare(const void *s1, const void *s2)
{
	int pin1 = *(const unsigned int *)s1;
	int pin2 = *(const unsigned int *)s2;

	return pin1 - pin2;
}

/**
 * single_configure_pins() - Configure pins based on FDT data
 *
 * @dev: Pointer to single pin configuration device which is the parent of
 *       the pins node holding the pin configuration data.
 * @pins: Pointer to the first element of an array of register/value pairs
 *        of type 'u32'. Each such pair describes the pin to be configured 
 *        and the value to be used for configuration.
 *        The value can either be a simple value if #pinctrl-cells = 1
 *        or a configuration value and a pin mux mode value if it is 2
 *        This pointer points to a 'pinctrl-single,pins' property in the
 *        device-tree.
 * @size: Size of the 'pins' array in bytes.
 *        The number of cells in the array therefore equals to
 *        'size / sizeof(u32)'.
 * @fname: Function name.
 */
static int single_configure_pins(struct udevice *dev,
				 const u32 *pins,
				 int size, const char *fname)
{
	struct single_pdata *pdata = dev_get_plat(dev);
	struct single_priv *priv = dev_get_priv(dev);
	int stride = pdata->args_count + 1;
	int n, pin, count = size / sizeof(u32);
	struct single_func *func;
	void *reg;
	u32 offset, val, mux;

	/* If function mask is null, needn't enable it. */
	if (!pdata->mask)
		return 0;

	func = single_allocate_function(dev, count);
	if (IS_ERR(func))
		return PTR_ERR(func);

	func->name = fname;
	func->npins = 0;
	for (n = 0; n < count; n += stride) {
		offset = fdt32_to_cpu(pins[n]);
		if (offset > pdata->offset) {
			dev_err(dev, "  invalid register offset 0x%x\n",
				offset);
			continue;
		}

		/* if the pinctrl-cells is 2 then the second cell contains the mux */
		if (stride == 3)
			mux = fdt32_to_cpu(pins[n + 2]);
		else
			mux = 0;

		reg = pdata->base + offset;
		val = (fdt32_to_cpu(pins[n + 1]) | mux) & pdata->mask;
		pin = single_get_pin_by_offset(dev, offset);
		if (pin < 0) {
			dev_err(dev, "  failed to get pin by offset %x\n",
				offset);
			continue;
		}

		single_write(dev, (single_read(dev, reg) & ~pdata->mask) | val,
			     reg);
		dev_dbg(dev, "  reg/val %pa/0x%08x\n", &reg, val);
		func->pins[func->npins] = pin;
		func->npins++;
	}

	qsort(func->pins, func->npins, sizeof(func->pins[0]),
	      single_pin_compare);
	list_add(&func->node, &priv->functions);
	return 0;
}

static int single_configure_bits(struct udevice *dev,
				 const struct single_fdt_bits_cfg *pins,
				 int size, const char *fname)
{
	struct single_pdata *pdata = dev_get_plat(dev);
	struct single_priv *priv = dev_get_priv(dev);
	int n, pin, count = size / sizeof(struct single_fdt_bits_cfg);
	int npins_in_reg, pin_num_from_lsb;
	struct single_func *func;
	void *reg;
	u32 offset, val, mask, bit_pos, val_pos, mask_pos, submask;

	/* If function mask is null, needn't enable it. */
	if (!pdata->mask)
		return 0;

	npins_in_reg = pdata->width / priv->bits_per_pin;
	func = single_allocate_function(dev, count * npins_in_reg);
	if (IS_ERR(func))
		return PTR_ERR(func);

	func->name = fname;
	func->npins = 0;
	for (n = 0; n < count; n++, pins++) {
		offset = fdt32_to_cpu(pins->reg);
		if (offset > pdata->offset) {
			dev_dbg(dev, "  invalid register offset 0x%x\n",
				offset);
			continue;
		}

		reg = pdata->base + offset;

		pin = single_get_pin_by_offset(dev, offset);
		if (pin < 0) {
			dev_err(dev, "  failed to get pin by offset 0x%pa\n",
				&reg);
			continue;
		}

		mask = fdt32_to_cpu(pins->mask);
		val = fdt32_to_cpu(pins->val) & mask;
		single_write(dev, (single_read(dev, reg) & ~mask) | val, reg);
		dev_dbg(dev, "  reg/val %pa/0x%08x\n", &reg, val);

		while (mask) {
			bit_pos = __ffs(mask);
			pin_num_from_lsb = bit_pos / priv->bits_per_pin;
			mask_pos = pdata->mask << bit_pos;
			val_pos = val & mask_pos;
			submask = mask & mask_pos;

			if ((mask & mask_pos) == 0) {
				dev_err(dev, "Invalid mask at 0x%x\n", offset);
				break;
			}

			mask &= ~mask_pos;

			if (submask != mask_pos) {
				dev_warn(dev,
					 "Invalid submask 0x%x at 0x%x\n",
					 submask, offset);
				continue;
			}

			func->pins[func->npins] = pin + pin_num_from_lsb;
			func->npins++;
		}
	}

	qsort(func->pins, func->npins, sizeof(func->pins[0]),
	      single_pin_compare);
	list_add(&func->node, &priv->functions);
	return 0;
}
static int single_set_state(struct udevice *dev,
			    struct udevice *config)
{
	const u32 *prop;
	const struct single_fdt_bits_cfg *prop_bits;
	int len;

	prop = dev_read_prop(config, "pinctrl-single,pins", &len);

	if (prop) {
		dev_dbg(dev, "configuring pins for %s\n", config->name);
		if (len % sizeof(u32)) {
			dev_dbg(dev, "  invalid pin configuration in fdt\n");
			return -FDT_ERR_BADSTRUCTURE;
		}
		single_configure_pins(dev, prop, len, config->name);
		return 0;
	}

	/* pinctrl-single,pins not found so check for pinctrl-single,bits */
	prop_bits = dev_read_prop(config, "pinctrl-single,bits", &len);
	if (prop_bits) {
		dev_dbg(dev, "configuring pins for %s\n", config->name);
		if (len % sizeof(struct single_fdt_bits_cfg)) {
			dev_dbg(dev, "  invalid bits configuration in fdt\n");
			return -FDT_ERR_BADSTRUCTURE;
		}
		single_configure_bits(dev, prop_bits, len, config->name);
		return 0;
	}

	/* Neither 'pinctrl-single,pins' nor 'pinctrl-single,bits' were found */
	return len;
}

static const char *single_get_pin_name(struct udevice *dev,
				       unsigned int selector)
{
	struct single_priv *priv = dev_get_priv(dev);

	if (selector >= priv->npins)
		snprintf(priv->pin_name, PINNAME_SIZE, "Error");
	else
		snprintf(priv->pin_name, PINNAME_SIZE, "PIN%u", selector);

	return priv->pin_name;
}

static int single_get_pins_count(struct udevice *dev)
{
	struct single_priv *priv = dev_get_priv(dev);

	return priv->npins;
}

static int single_add_gpio_func(struct udevice *dev)
{
	struct single_priv *priv = dev_get_priv(dev);
	const char *propname = "pinctrl-single,gpio-range";
	const char *cellname = "#pinctrl-single,gpio-range-cells";
	struct single_gpiofunc_range *range;
	struct ofnode_phandle_args gpiospec;
	int ret, i;

	for (i = 0; ; i++) {
		ret = ofnode_parse_phandle_with_args(dev_ofnode(dev), propname,
						     cellname, 0, i, &gpiospec);
		/* Do not treat it as error. Only treat it as end condition. */
		if (ret) {
			ret = 0;
			break;
		}
		range = devm_kzalloc(dev, sizeof(*range), GFP_KERNEL);
		if (!range) {
			ret = -ENOMEM;
			break;
		}
		range->offset = gpiospec.args[0];
		range->npins = gpiospec.args[1];
		range->gpiofunc = gpiospec.args[2];
		list_add_tail(&range->node, &priv->gpiofuncs);
	}
	return ret;
}

static int single_probe(struct udevice *dev)
{
	struct single_pdata *pdata = dev_get_plat(dev);
	struct single_priv *priv = dev_get_priv(dev);
	u32 size;

	INIT_LIST_HEAD(&priv->functions);
	INIT_LIST_HEAD(&priv->gpiofuncs);

	size = pdata->offset + pdata->width / BITS_PER_BYTE;
	#if (IS_ENABLED(CONFIG_SANDBOX))
	priv->sandbox_regs =
		devm_kzalloc(dev, size * sizeof(*priv->sandbox_regs),
			     GFP_KERNEL);
	if (!priv->sandbox_regs)
		return -ENOMEM;
	#endif

	/* looks like a possible divide by 0, but data->width avoids this */
	priv->npins = size / (pdata->width / BITS_PER_BYTE);
	if (pdata->bits_per_mux) {
		if (!pdata->mask) {
			dev_err(dev, "function mask needs to be non-zero\n");
			return -EINVAL;
		}

		priv->bits_per_pin = fls(pdata->mask);
		priv->npins *= (pdata->width / priv->bits_per_pin);
	}

	if (single_add_gpio_func(dev))
		dev_dbg(dev, "gpio functions are not added\n");

	dev_dbg(dev, "%d pins\n", priv->npins);
	return 0;
}

static int single_of_to_plat(struct udevice *dev)
{
	void *addr;
	fdt_size_t size;
	struct single_pdata *pdata = dev_get_plat(dev);
	int ret;

	ret = dev_read_u32(dev, "pinctrl-single,register-width", &pdata->width);
	if (ret) {
		dev_err(dev, "missing register width\n");
		return ret;
	}

	switch (pdata->width) {
	case 8:
	case 16:
	case 32:
		break;
	default:
		dev_err(dev, "wrong register width\n");
		return -EINVAL;
	}

	addr = dev_read_addr_size_index_ptr(dev, 0, &size);
	if (!addr) {
		dev_err(dev, "failed to get base register address\n");
		return -EINVAL;
	}

	pdata->offset = size - pdata->width / BITS_PER_BYTE;
	pdata->base = addr;

	ret = dev_read_u32(dev, "pinctrl-single,function-mask", &pdata->mask);
	if (ret) {
		pdata->mask = 0;
		dev_warn(dev, "missing function register mask\n");
	}

	pdata->bits_per_mux = dev_read_bool(dev, "pinctrl-single,bit-per-mux");

	/* If no pinctrl-cells is present, default to old style of 2 cells with
	 * bits per mux and 1 cell otherwise.
	 */
	ret = dev_read_u32(dev, "#pinctrl-cells", &pdata->args_count);
	if (ret)
		pdata->args_count = pdata->bits_per_mux ? 2 : 1;

	return 0;
}

const struct pinctrl_ops single_pinctrl_ops = {
	.get_pins_count	= single_get_pins_count,
	.get_pin_name = single_get_pin_name,
	.set_state = single_set_state,
	.get_pin_muxing	= single_get_pin_muxing,
	.request = single_request,
};

static const struct udevice_id single_pinctrl_match[] = {
	{ .compatible = "pinctrl-single" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(single_pinctrl) = {
	.name = "single-pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = single_pinctrl_match,
	.ops = &single_pinctrl_ops,
	.plat_auto	= sizeof(struct single_pdata),
	.priv_auto = sizeof(struct single_priv),
	.of_to_plat = single_of_to_plat,
	.probe = single_probe,
};
