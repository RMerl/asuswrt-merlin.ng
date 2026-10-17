// SPDX-License-Identifier: GPL-2.0+
/*
 * Atmel PIO4 device driver
 *
 * Copyright (C) 2015 Atmel Corporation
 *		 Wenyou.Yang <wenyou.yang@atmel.com>
 */
#include <common.h>
#include <clk.h>
#include <dm.h>
#include <fdtdec.h>
#include <malloc.h>
#include <asm/arch/hardware.h>
#include <asm/global_data.h>
#include <asm/gpio.h>
#include <linux/bitops.h>
#include <mach/gpio.h>
#include <mach/atmel_pio4.h>

DECLARE_GLOBAL_DATA_PTR;

static struct atmel_pio4_port *atmel_pio4_port_base(u32 port)
{
	struct atmel_pio4_port *base = NULL;

	switch (port) {
	case AT91_PIO_PORTA:
		base = (struct atmel_pio4_port *)ATMEL_BASE_PIOA;
		break;
	case AT91_PIO_PORTB:
		base = (struct atmel_pio4_port *)ATMEL_BASE_PIOB;
		break;
	case AT91_PIO_PORTC:
		base = (struct atmel_pio4_port *)ATMEL_BASE_PIOC;
		break;
	case AT91_PIO_PORTD:
		base = (struct atmel_pio4_port *)ATMEL_BASE_PIOD;
		break;
#if (ATMEL_PIO_PORTS > 4)
	case AT91_PIO_PORTE:
		base = (struct atmel_pio4_port *)ATMEL_BASE_PIOE;
		break;
#endif
	default:
		printf("Error: Atmel PIO4: Failed to get PIO base of port#%d!\n",
		       port);
		break;
	}

	return base;
}

static int atmel_pio4_config_io_func(u32 port, u32 pin,
				     u32 func, u32 config)
{
	struct atmel_pio4_port *port_base;
	u32 reg, mask;

	if (pin >= ATMEL_PIO_NPINS_PER_BANK)
		return -EINVAL;

	port_base = atmel_pio4_port_base(port);
	if (!port_base)
		return -EINVAL;

	mask = 1 << pin;
	reg = func;
	reg |= config;

	writel(mask, &port_base->mskr);
	writel(reg, &port_base->cfgr);

	return 0;
}

int atmel_pio4_set_gpio(u32 port, u32 pin, u32 config)
{
	return atmel_pio4_config_io_func(port, pin,
					 ATMEL_PIO_CFGR_FUNC_GPIO,
					 config);
}

int atmel_pio4_set_a_periph(u32 port, u32 pin, u32 config)
{
	return atmel_pio4_config_io_func(port, pin,
					 ATMEL_PIO_CFGR_FUNC_PERIPH_A,
					 config);
}

int atmel_pio4_set_b_periph(u32 port, u32 pin, u32 config)
{
	return atmel_pio4_config_io_func(port, pin,
					 ATMEL_PIO_CFGR_FUNC_PERIPH_B,
					 config);
}

int atmel_pio4_set_c_periph(u32 port, u32 pin, u32 config)
{
	return atmel_pio4_config_io_func(port, pin,
					 ATMEL_PIO_CFGR_FUNC_PERIPH_C,
					 config);
}

int atmel_pio4_set_d_periph(u32 port, u32 pin, u32 config)
{
	return atmel_pio4_config_io_func(port, pin,
					 ATMEL_PIO_CFGR_FUNC_PERIPH_D,
					 config);
}

int atmel_pio4_set_e_periph(u32 port, u32 pin, u32 config)
{
	return atmel_pio4_config_io_func(port, pin,
					 ATMEL_PIO_CFGR_FUNC_PERIPH_E,
					 config);
}

int atmel_pio4_set_f_periph(u32 port, u32 pin, u32 config)
{
	return atmel_pio4_config_io_func(port, pin,
					 ATMEL_PIO_CFGR_FUNC_PERIPH_F,
					 config);
}

int atmel_pio4_set_g_periph(u32 port, u32 pin, u32 config)
{
	return atmel_pio4_config_io_func(port, pin,
					 ATMEL_PIO_CFGR_FUNC_PERIPH_G,
					 config);
}

int atmel_pio4_set_pio_output(u32 port, u32 pin, u32 value)
{
	struct atmel_pio4_port *port_base;
	u32 reg, mask;

	if (pin >= ATMEL_PIO_NPINS_PER_BANK)
		return -EINVAL;

	port_base = atmel_pio4_port_base(port);
	if (!port_base)
		return -EINVAL;

	mask = 0x01 << pin;
	reg = ATMEL_PIO_CFGR_FUNC_GPIO | ATMEL_PIO_DIR_MASK;

	writel(mask, &port_base->mskr);
	writel(reg, &port_base->cfgr);

	if (value)
		writel(mask, &port_base->sodr);
	else
		writel(mask, &port_base->codr);

	return 0;
}

int atmel_pio4_get_pio_input(u32 port, u32 pin)
{
	struct atmel_pio4_port *port_base;
	u32 reg, mask;

	if (pin >= ATMEL_PIO_NPINS_PER_BANK)
		return -EINVAL;

	port_base = atmel_pio4_port_base(port);
	if (!port_base)
		return -EINVAL;

	mask = 0x01 << pin;
	reg = ATMEL_PIO_CFGR_FUNC_GPIO;

	writel(mask, &port_base->mskr);
	writel(reg, &port_base->cfgr);

	return (readl(&port_base->pdsr) & mask) ? 1 : 0;
}

#if CONFIG_IS_ENABLED(DM_GPIO)

/**
 * struct atmel_pioctrl_data - Atmel PIO controller (pinmux + gpio) data struct
 * @nbanks: number of PIO banks
 * @last_bank_count: number of lines in the last bank (can be less than
 *     the rest of the banks).
 */
struct atmel_pioctrl_data {
	u32 nbanks;
	u32 last_bank_count;
};

struct atmel_pio4_plat {
	struct atmel_pio4_port *reg_base;
};

static struct atmel_pio4_port *atmel_pio4_bank_base(struct udevice *dev,
						    u32 bank)
{
	struct atmel_pio4_plat *plat = dev_get_plat(dev);
	struct atmel_pio4_port *port_base =
			(struct atmel_pio4_port *)((u32)plat->reg_base +
			ATMEL_PIO_BANK_OFFSET * bank);

	return port_base;
}

static int atmel_pio4_direction_input(struct udevice *dev, unsigned offset)
{
	u32 bank = ATMEL_PIO_BANK(offset);
	u32 line = ATMEL_PIO_LINE(offset);
	struct atmel_pio4_port *port_base = atmel_pio4_bank_base(dev, bank);
	u32 mask = BIT(line);

	writel(mask, &port_base->mskr);

	clrbits_le32(&port_base->cfgr,
		     ATMEL_PIO_CFGR_FUNC_MASK | ATMEL_PIO_DIR_MASK);

	return 0;
}

static int atmel_pio4_direction_output(struct udevice *dev,
				       unsigned offset, int value)
{
	u32 bank = ATMEL_PIO_BANK(offset);
	u32 line = ATMEL_PIO_LINE(offset);
	struct atmel_pio4_port *port_base = atmel_pio4_bank_base(dev, bank);
	u32 mask = BIT(line);

	writel(mask, &port_base->mskr);

	clrsetbits_le32(&port_base->cfgr,
			ATMEL_PIO_CFGR_FUNC_MASK, ATMEL_PIO_DIR_MASK);

	if (value)
		writel(mask, &port_base->sodr);
	else
		writel(mask, &port_base->codr);

	return 0;
}

static int atmel_pio4_get_value(struct udevice *dev, unsigned offset)
{
	u32 bank = ATMEL_PIO_BANK(offset);
	u32 line = ATMEL_PIO_LINE(offset);
	struct atmel_pio4_port *port_base = atmel_pio4_bank_base(dev, bank);
	u32 mask = BIT(line);

	return (readl(&port_base->pdsr) & mask) ? 1 : 0;
}

static int atmel_pio4_set_value(struct udevice *dev,
				unsigned offset, int value)
{
	u32 bank = ATMEL_PIO_BANK(offset);
	u32 line = ATMEL_PIO_LINE(offset);
	struct atmel_pio4_port *port_base = atmel_pio4_bank_base(dev, bank);
	u32 mask = BIT(line);

	if (value)
		writel(mask, &port_base->sodr);
	else
		writel(mask, &port_base->codr);

	return 0;
}

static int atmel_pio4_get_function(struct udevice *dev, unsigned offset)
{
	u32 bank = ATMEL_PIO_BANK(offset);
	u32 line = ATMEL_PIO_LINE(offset);
	struct atmel_pio4_port *port_base = atmel_pio4_bank_base(dev, bank);
	u32 mask = BIT(line);

	writel(mask, &port_base->mskr);

	return (readl(&port_base->cfgr) &
		ATMEL_PIO_DIR_MASK) ? GPIOF_OUTPUT : GPIOF_INPUT;
}

static const struct dm_gpio_ops atmel_pio4_ops = {
	.direction_input	= atmel_pio4_direction_input,
	.direction_output	= atmel_pio4_direction_output,
	.get_value		= atmel_pio4_get_value,
	.set_value		= atmel_pio4_set_value,
	.get_function		= atmel_pio4_get_function,
};

static int atmel_pio4_bind(struct udevice *dev)
{
	return dm_scan_fdt_dev(dev);
}

static int atmel_pio4_probe(struct udevice *dev)
{
	struct atmel_pio4_plat *plat = dev_get_plat(dev);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct atmel_pioctrl_data *pioctrl_data;
	struct clk clk;
	fdt_addr_t addr_base;
	u32 nbanks;
	int ret;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret)
		return ret;

	ret = clk_enable(&clk);
	if (ret)
		return ret;

	addr_base = dev_read_addr(dev);
	if (addr_base == FDT_ADDR_T_NONE)
		return -EINVAL;

	plat->reg_base = (struct atmel_pio4_port *)addr_base;

	pioctrl_data = (struct atmel_pioctrl_data *)dev_get_driver_data(dev);
	nbanks = pioctrl_data->nbanks;

	uc_priv->bank_name = fdt_get_name(gd->fdt_blob, dev_of_offset(dev),
					  NULL);
	uc_priv->gpio_count = nbanks * ATMEL_PIO_NPINS_PER_BANK;

	/* if last bank has limited number of pins, adjust accordingly */
	if (pioctrl_data->last_bank_count != ATMEL_PIO_NPINS_PER_BANK) {
		uc_priv->gpio_count -= ATMEL_PIO_NPINS_PER_BANK;
		uc_priv->gpio_count += pioctrl_data->last_bank_count;
	}

	return 0;
}

/*
 * The number of banks can be different from a SoC to another one.
 * We can have up to 16 banks.
 */
static const struct atmel_pioctrl_data atmel_sama5d2_pioctrl_data = {
	.nbanks	= 4,
	.last_bank_count = ATMEL_PIO_NPINS_PER_BANK,
};

static const struct atmel_pioctrl_data microchip_sama7g5_pioctrl_data = {
	.nbanks	= 5,
	.last_bank_count = 8, /* 5th bank has only 8 lines on sama7g5 */
};

static const struct udevice_id atmel_pio4_ids[] = {
	{
		.data = (ulong)&atmel_sama5d2_pioctrl_data,
	}, {
		.data = (ulong)&microchip_sama7g5_pioctrl_data,
	},
	{}
};

U_BOOT_DRIVER(gpio_atmel_pio4) = {
	.name	= "gpio_atmel_pio4",
	.id	= UCLASS_GPIO,
	.ops	= &atmel_pio4_ops,
	.probe	= atmel_pio4_probe,
	.bind	= atmel_pio4_bind,
	.of_match = atmel_pio4_ids,
	.plat_auto	= sizeof(struct atmel_pio4_plat),
};

#endif
