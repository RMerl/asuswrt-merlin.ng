/*
 * Code ported from Nomadik GPIO driver in ST-Ericsson Linux kernel code.
 * The purpose is that GPIO config found in kernel should work by simply
 * copy-paste it to U-Boot.
 *
 * Original Linux authors:
 * Copyright (C) 2008,2009 STMicroelectronics
 * Copyright (C) 2009 Alessandro Rubini <rubini@unipv.it>
 *   Rewritten based on work by Prafulla WADASKAR <prafulla.wadaskar@st.com>
 *
 * Ported to U-Boot by:
 * Copyright (C) 2010 Joakim Axelsson <joakim.axelsson AT stericsson.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <common.h>
#include <asm/io.h>

#include <asm/arch/db8500_gpio.h>
#include <asm/arch/db8500_pincfg.h>
#include <linux/compiler.h>

#define IO_ADDR(x) (void *) (x)

/*
 * The GPIO module in the db8500 Systems-on-Chip is an
 * AMBA device, managing 32 pins and alternate functions. The logic block
 * is currently only used in the db8500.
 */

#define GPIO_TOTAL_PINS		268
#define GPIO_PINS_PER_BLOCK	32
#define GPIO_BLOCKS_COUNT	(GPIO_TOTAL_PINS/GPIO_PINS_PER_BLOCK + 1)
#define GPIO_BLOCK(pin)		(((pin + GPIO_PINS_PER_BLOCK) >> 5) - 1)
#define GPIO_PIN_WITHIN_BLOCK(pin)	((pin)%(GPIO_PINS_PER_BLOCK))

/* Register in the logic block */
#define DB8500_GPIO_DAT		0x00
#define DB8500_GPIO_DATS	0x04
#define DB8500_GPIO_DATC	0x08
#define DB8500_GPIO_PDIS	0x0c
#define DB8500_GPIO_DIR		0x10
#define DB8500_GPIO_DIRS	0x14
#define DB8500_GPIO_DIRC	0x18
#define DB8500_GPIO_SLPC	0x1c
#define DB8500_GPIO_AFSLA	0x20
#define DB8500_GPIO_AFSLB	0x24

#define DB8500_GPIO_RIMSC	0x40
#define DB8500_GPIO_FIMSC	0x44
#define DB8500_GPIO_IS		0x48
#define DB8500_GPIO_IC		0x4c
#define DB8500_GPIO_RWIMSC	0x50
#define DB8500_GPIO_FWIMSC	0x54
#define DB8500_GPIO_WKS		0x58

static void __iomem *get_gpio_addr(unsigned gpio)
{
	/* Our list of GPIO chips */
	static void __iomem *gpio_addrs[GPIO_BLOCKS_COUNT] = {
		IO_ADDR(CFG_GPIO_0_BASE),
		IO_ADDR(CFG_GPIO_1_BASE),
		IO_ADDR(CFG_GPIO_2_BASE),
		IO_ADDR(CFG_GPIO_3_BASE),
		IO_ADDR(CFG_GPIO_4_BASE),
		IO_ADDR(CFG_GPIO_5_BASE),
		IO_ADDR(CFG_GPIO_6_BASE),
		IO_ADDR(CFG_GPIO_7_BASE),
		IO_ADDR(CFG_GPIO_8_BASE)
	};

	return gpio_addrs[GPIO_BLOCK(gpio)];
}

static unsigned get_gpio_offset(unsigned gpio)
{
	return GPIO_PIN_WITHIN_BLOCK(gpio);
}

/* Can only be called from config_pin. Don't configure alt-mode directly */
static void gpio_set_mode(unsigned gpio, enum db8500_gpio_alt mode)
{
	void __iomem *addr = get_gpio_addr(gpio);
	unsigned offset = get_gpio_offset(gpio);
	u32 bit = 1 << offset;
	u32 afunc, bfunc;

	afunc = readl(addr + DB8500_GPIO_AFSLA) & ~bit;
	bfunc = readl(addr + DB8500_GPIO_AFSLB) & ~bit;
	if (mode & DB8500_GPIO_ALT_A)
		afunc |= bit;
	if (mode & DB8500_GPIO_ALT_B)
		bfunc |= bit;
	writel(afunc, addr + DB8500_GPIO_AFSLA);
	writel(bfunc, addr + DB8500_GPIO_AFSLB);
}

/**
 * db8500_gpio_set_pull() - enable/disable pull up/down on a gpio
 * @gpio: pin number
 * @pull: one of DB8500_GPIO_PULL_DOWN, DB8500_GPIO_PULL_UP,
 *  and DB8500_GPIO_PULL_NONE
 *
 * Enables/disables pull up/down on a specified pin.  This only takes effect if
 * the pin is configured as an input (either explicitly or by the alternate
 * function).
 *
 * NOTE: If enabling the pull up/down, the caller must ensure that the GPIO is
 * configured as an input.  Otherwise, due to the way the controller registers
 * work, this function will change the value output on the pin.
 */
void db8500_gpio_set_pull(unsigned gpio, enum db8500_gpio_pull pull)
{
	void __iomem *addr = get_gpio_addr(gpio);
	unsigned offset = get_gpio_offset(gpio);
	u32 bit = 1 << offset;
	u32 pdis;

	pdis = readl(addr + DB8500_GPIO_PDIS);
	if (pull == DB8500_GPIO_PULL_NONE)
		pdis |= bit;
	else
		pdis &= ~bit;
	writel(pdis, addr + DB8500_GPIO_PDIS);

	if (pull == DB8500_GPIO_PULL_UP)
		writel(bit, addr + DB8500_GPIO_DATS);
	else if (pull == DB8500_GPIO_PULL_DOWN)
		writel(bit, addr + DB8500_GPIO_DATC);
}

void db8500_gpio_make_input(unsigned gpio)
{
	void __iomem *addr = get_gpio_addr(gpio);
	unsigned offset = get_gpio_offset(gpio);

	writel(1 << offset, addr + DB8500_GPIO_DIRC);
}

int db8500_gpio_get_input(unsigned gpio)
{
	void __iomem *addr = get_gpio_addr(gpio);
	unsigned offset = get_gpio_offset(gpio);
	u32 bit = 1 << offset;

	printf("db8500_gpio_get_input gpio=%u addr=%p offset=%u bit=%#x\n",
		gpio, addr, offset, bit);

	return (readl(addr + DB8500_GPIO_DAT) & bit) != 0;
}

void db8500_gpio_make_output(unsigned gpio, int val)
{
	void __iomem *addr = get_gpio_addr(gpio);
	unsigned offset = get_gpio_offset(gpio);

	writel(1 << offset, addr + DB8500_GPIO_DIRS);
	db8500_gpio_set_output(gpio, val);
}

void db8500_gpio_set_output(unsigned gpio, int val)
{
	void __iomem *addr = get_gpio_addr(gpio);
	unsigned offset = get_gpio_offset(gpio);

	if (val)
		writel(1 << offset, addr + DB8500_GPIO_DATS);
	else
		writel(1 << offset, addr + DB8500_GPIO_DATC);
}

/**
 * config_pin - configure a pin's mux attributes
 * @cfg: pin configuration
 *
 * Configures a pin's mode (alternate function or GPIO), its pull up status,
 * and its sleep mode based on the specified configuration.  The @cfg is
 * usually one of the SoC specific macros defined in mach/<soc>-pins.h.  These
 * are constructed using, and can be further enhanced with, the macros in
 * plat/pincfg.h.
 *
 * If a pin's mode is set to GPIO, it is configured as an input to avoid
 * side-effects.  The gpio can be manipulated later using standard GPIO API
 * calls.
 */
static void config_pin(unsigned long cfg)
{
	int pin = PIN_NUM(cfg);
	int pull = PIN_PULL(cfg);
	int af = PIN_ALT(cfg);
	int output = PIN_DIR(cfg);
	int val = PIN_VAL(cfg);

	if (output)
		db8500_gpio_make_output(pin, val);
	else {
		db8500_gpio_make_input(pin);
		db8500_gpio_set_pull(pin, pull);
	}

	gpio_set_mode(pin, af);
}

/**
 * db8500_config_pins - configure several pins at once
 * @cfgs: array of pin configurations
 * @num: number of elments in the array
 *
 * Configures several pins using config_pin(). Refer to that function for
 * further information.
 */
void db8500_gpio_config_pins(unsigned long *cfgs, size_t num)
{
	size_t i;

	for (i = 0; i < num; i++)
		config_pin(cfgs[i]);
}
