// SPDX-License-Identifier: GPL-2.0+
/*
 * Xilinx Zynq GPIO device driver
 *
 * Copyright (C) 2015 DAVE Embedded Systems <devel@dave.eu>
 *
 * Most of code taken from linux kernel driver (linux/drivers/gpio/gpio-zynq.c)
 * Copyright (C) 2009 - 2014 Xilinx, Inc.
 */

#include <common.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <dm.h>
#include <fdtdec.h>

/* Maximum banks */
#define ZYNQ_GPIO_MAX_BANK	4

#define ZYNQ_GPIO_BANK0_NGPIO	32
#define ZYNQ_GPIO_BANK1_NGPIO	22
#define ZYNQ_GPIO_BANK2_NGPIO	32
#define ZYNQ_GPIO_BANK3_NGPIO	32

#define ZYNQ_GPIO_NR_GPIOS	(ZYNQ_GPIO_BANK0_NGPIO + \
				 ZYNQ_GPIO_BANK1_NGPIO + \
				 ZYNQ_GPIO_BANK2_NGPIO + \
				 ZYNQ_GPIO_BANK3_NGPIO)

#define ZYNQMP_GPIO_MAX_BANK	6

#define ZYNQMP_GPIO_BANK0_NGPIO	26
#define ZYNQMP_GPIO_BANK1_NGPIO	26
#define ZYNQMP_GPIO_BANK2_NGPIO	26
#define ZYNQMP_GPIO_BANK3_NGPIO	32
#define ZYNQMP_GPIO_BANK4_NGPIO	32
#define ZYNQMP_GPIO_BANK5_NGPIO	32

#define ZYNQMP_GPIO_NR_GPIOS	174

#define ZYNQ_GPIO_BANK0_PIN_MIN(str)	0
#define ZYNQ_GPIO_BANK0_PIN_MAX(str)	(ZYNQ_GPIO_BANK0_PIN_MIN(str) + \
					ZYNQ##str##_GPIO_BANK0_NGPIO - 1)
#define ZYNQ_GPIO_BANK1_PIN_MIN(str)	(ZYNQ_GPIO_BANK0_PIN_MAX(str) + 1)
#define ZYNQ_GPIO_BANK1_PIN_MAX(str)	(ZYNQ_GPIO_BANK1_PIN_MIN(str) + \
					ZYNQ##str##_GPIO_BANK1_NGPIO - 1)
#define ZYNQ_GPIO_BANK2_PIN_MIN(str)	(ZYNQ_GPIO_BANK1_PIN_MAX(str) + 1)
#define ZYNQ_GPIO_BANK2_PIN_MAX(str)	(ZYNQ_GPIO_BANK2_PIN_MIN(str) + \
					ZYNQ##str##_GPIO_BANK2_NGPIO - 1)
#define ZYNQ_GPIO_BANK3_PIN_MIN(str)	(ZYNQ_GPIO_BANK2_PIN_MAX(str) + 1)
#define ZYNQ_GPIO_BANK3_PIN_MAX(str)	(ZYNQ_GPIO_BANK3_PIN_MIN(str) + \
					ZYNQ##str##_GPIO_BANK3_NGPIO - 1)
#define ZYNQ_GPIO_BANK4_PIN_MIN(str)	(ZYNQ_GPIO_BANK3_PIN_MAX(str) + 1)
#define ZYNQ_GPIO_BANK4_PIN_MAX(str)	(ZYNQ_GPIO_BANK4_PIN_MIN(str) + \
					ZYNQ##str##_GPIO_BANK4_NGPIO - 1)
#define ZYNQ_GPIO_BANK5_PIN_MIN(str)	(ZYNQ_GPIO_BANK4_PIN_MAX(str) + 1)
#define ZYNQ_GPIO_BANK5_PIN_MAX(str)	(ZYNQ_GPIO_BANK5_PIN_MIN(str) + \
					ZYNQ##str##_GPIO_BANK5_NGPIO - 1)

/* Register offsets for the GPIO device */
/* LSW Mask & Data -WO */
#define ZYNQ_GPIO_DATA_LSW_OFFSET(BANK)	(0x000 + (8 * BANK))
/* MSW Mask & Data -WO */
#define ZYNQ_GPIO_DATA_MSW_OFFSET(BANK)	(0x004 + (8 * BANK))
/* Data Register-RW */
#define ZYNQ_GPIO_DATA_RO_OFFSET(BANK)	(0x060 + (4 * BANK))
/* Direction mode reg-RW */
#define ZYNQ_GPIO_DIRM_OFFSET(BANK)	(0x204 + (0x40 * BANK))
/* Output enable reg-RW */
#define ZYNQ_GPIO_OUTEN_OFFSET(BANK)	(0x208 + (0x40 * BANK))
/* Interrupt mask reg-RO */
#define ZYNQ_GPIO_INTMASK_OFFSET(BANK)	(0x20C + (0x40 * BANK))
/* Interrupt enable reg-WO */
#define ZYNQ_GPIO_INTEN_OFFSET(BANK)	(0x210 + (0x40 * BANK))
/* Interrupt disable reg-WO */
#define ZYNQ_GPIO_INTDIS_OFFSET(BANK)	(0x214 + (0x40 * BANK))
/* Interrupt status reg-RO */
#define ZYNQ_GPIO_INTSTS_OFFSET(BANK)	(0x218 + (0x40 * BANK))
/* Interrupt type reg-RW */
#define ZYNQ_GPIO_INTTYPE_OFFSET(BANK)	(0x21C + (0x40 * BANK))
/* Interrupt polarity reg-RW */
#define ZYNQ_GPIO_INTPOL_OFFSET(BANK)	(0x220 + (0x40 * BANK))
/* Interrupt on any, reg-RW */
#define ZYNQ_GPIO_INTANY_OFFSET(BANK)	(0x224 + (0x40 * BANK))

/* Disable all interrupts mask */
#define ZYNQ_GPIO_IXR_DISABLE_ALL	0xFFFFFFFF

/* Mid pin number of a bank */
#define ZYNQ_GPIO_MID_PIN_NUM 16

/* GPIO upper 16 bit mask */
#define ZYNQ_GPIO_UPPER_MASK 0xFFFF0000

struct zynq_gpio_platdata {
	phys_addr_t base;
	const struct zynq_platform_data *p_data;
};

/**
 * struct zynq_platform_data -  zynq gpio platform data structure
 * @label:	string to store in gpio->label
 * @ngpio:	max number of gpio pins
 * @max_bank:	maximum number of gpio banks
 * @bank_min:	this array represents bank's min pin
 * @bank_max:	this array represents bank's max pin
 */
struct zynq_platform_data {
	const char *label;
	u16 ngpio;
	u32 max_bank;
	u32 bank_min[ZYNQMP_GPIO_MAX_BANK];
	u32 bank_max[ZYNQMP_GPIO_MAX_BANK];
};

static const struct zynq_platform_data zynqmp_gpio_def = {
	.label = "zynqmp_gpio",
	.ngpio = ZYNQMP_GPIO_NR_GPIOS,
	.max_bank = ZYNQMP_GPIO_MAX_BANK,
	.bank_min[0] = ZYNQ_GPIO_BANK0_PIN_MIN(MP),
	.bank_max[0] = ZYNQ_GPIO_BANK0_PIN_MAX(MP),
	.bank_min[1] = ZYNQ_GPIO_BANK1_PIN_MIN(MP),
	.bank_max[1] = ZYNQ_GPIO_BANK1_PIN_MAX(MP),
	.bank_min[2] = ZYNQ_GPIO_BANK2_PIN_MIN(MP),
	.bank_max[2] = ZYNQ_GPIO_BANK2_PIN_MAX(MP),
	.bank_min[3] = ZYNQ_GPIO_BANK3_PIN_MIN(MP),
	.bank_max[3] = ZYNQ_GPIO_BANK3_PIN_MAX(MP),
	.bank_min[4] = ZYNQ_GPIO_BANK4_PIN_MIN(MP),
	.bank_max[4] = ZYNQ_GPIO_BANK4_PIN_MAX(MP),
	.bank_min[5] = ZYNQ_GPIO_BANK5_PIN_MIN(MP),
	.bank_max[5] = ZYNQ_GPIO_BANK5_PIN_MAX(MP),
};

static const struct zynq_platform_data zynq_gpio_def = {
	.label = "zynq_gpio",
	.ngpio = ZYNQ_GPIO_NR_GPIOS,
	.max_bank = ZYNQ_GPIO_MAX_BANK,
	.bank_min[0] = ZYNQ_GPIO_BANK0_PIN_MIN(),
	.bank_max[0] = ZYNQ_GPIO_BANK0_PIN_MAX(),
	.bank_min[1] = ZYNQ_GPIO_BANK1_PIN_MIN(),
	.bank_max[1] = ZYNQ_GPIO_BANK1_PIN_MAX(),
	.bank_min[2] = ZYNQ_GPIO_BANK2_PIN_MIN(),
	.bank_max[2] = ZYNQ_GPIO_BANK2_PIN_MAX(),
	.bank_min[3] = ZYNQ_GPIO_BANK3_PIN_MIN(),
	.bank_max[3] = ZYNQ_GPIO_BANK3_PIN_MAX(),
};

/**
 * zynq_gpio_get_bank_pin - Get the bank number and pin number within that bank
 * for a given pin in the GPIO device
 * @pin_num:	gpio pin number within the device
 * @bank_num:	an output parameter used to return the bank number of the gpio
 *		pin
 * @bank_pin_num: an output parameter used to return pin number within a bank
 *		  for the given gpio pin
 *
 * Returns the bank number and pin offset within the bank.
 */
static inline void zynq_gpio_get_bank_pin(unsigned int pin_num,
					  unsigned int *bank_num,
					  unsigned int *bank_pin_num,
					  struct udevice *dev)
{
	struct zynq_gpio_platdata *platdata = dev_get_platdata(dev);
	u32 bank;

	for (bank = 0; bank < platdata->p_data->max_bank; bank++) {
		if (pin_num >= platdata->p_data->bank_min[bank] &&
		    pin_num <= platdata->p_data->bank_max[bank]) {
			*bank_num = bank;
			*bank_pin_num = pin_num -
					platdata->p_data->bank_min[bank];
			return;
		}
	}

	if (bank >= platdata->p_data->max_bank) {
		printf("Invalid bank and pin num\n");
		*bank_num = 0;
		*bank_pin_num = 0;
	}
}

static int gpio_is_valid(unsigned gpio, struct udevice *dev)
{
	struct zynq_gpio_platdata *platdata = dev_get_platdata(dev);

	return gpio < platdata->p_data->ngpio;
}

static int check_gpio(unsigned gpio, struct udevice *dev)
{
	if (!gpio_is_valid(gpio, dev)) {
		printf("ERROR : check_gpio: invalid GPIO %d\n", gpio);
		return -1;
	}
	return 0;
}

static int zynq_gpio_get_value(struct udevice *dev, unsigned gpio)
{
	u32 data;
	unsigned int bank_num, bank_pin_num;
	struct zynq_gpio_platdata *platdata = dev_get_platdata(dev);

	if (check_gpio(gpio, dev) < 0)
		return -1;

	zynq_gpio_get_bank_pin(gpio, &bank_num, &bank_pin_num, dev);

	data = readl(platdata->base +
			     ZYNQ_GPIO_DATA_RO_OFFSET(bank_num));

	return (data >> bank_pin_num) & 1;
}

static int zynq_gpio_set_value(struct udevice *dev, unsigned gpio, int value)
{
	unsigned int reg_offset, bank_num, bank_pin_num;
	struct zynq_gpio_platdata *platdata = dev_get_platdata(dev);

	if (check_gpio(gpio, dev) < 0)
		return -1;

	zynq_gpio_get_bank_pin(gpio, &bank_num, &bank_pin_num, dev);

	if (bank_pin_num >= ZYNQ_GPIO_MID_PIN_NUM) {
		/* only 16 data bits in bit maskable reg */
		bank_pin_num -= ZYNQ_GPIO_MID_PIN_NUM;
		reg_offset = ZYNQ_GPIO_DATA_MSW_OFFSET(bank_num);
	} else {
		reg_offset = ZYNQ_GPIO_DATA_LSW_OFFSET(bank_num);
	}

	/*
	 * get the 32 bit value to be written to the mask/data register where
	 * the upper 16 bits is the mask and lower 16 bits is the data
	 */
	value = !!value;
	value = ~(1 << (bank_pin_num + ZYNQ_GPIO_MID_PIN_NUM)) &
		((value << bank_pin_num) | ZYNQ_GPIO_UPPER_MASK);

	writel(value, platdata->base + reg_offset);

	return 0;
}

static int zynq_gpio_direction_input(struct udevice *dev, unsigned gpio)
{
	u32 reg;
	unsigned int bank_num, bank_pin_num;
	struct zynq_gpio_platdata *platdata = dev_get_platdata(dev);

	if (check_gpio(gpio, dev) < 0)
		return -1;

	zynq_gpio_get_bank_pin(gpio, &bank_num, &bank_pin_num, dev);

	/* bank 0 pins 7 and 8 are special and cannot be used as inputs */
	if (bank_num == 0 && (bank_pin_num == 7 || bank_pin_num == 8))
		return -1;

	/* clear the bit in direction mode reg to set the pin as input */
	reg = readl(platdata->base + ZYNQ_GPIO_DIRM_OFFSET(bank_num));
	reg &= ~BIT(bank_pin_num);
	writel(reg, platdata->base + ZYNQ_GPIO_DIRM_OFFSET(bank_num));

	return 0;
}

static int zynq_gpio_direction_output(struct udevice *dev, unsigned gpio,
				      int value)
{
	u32 reg;
	unsigned int bank_num, bank_pin_num;
	struct zynq_gpio_platdata *platdata = dev_get_platdata(dev);

	if (check_gpio(gpio, dev) < 0)
		return -1;

	zynq_gpio_get_bank_pin(gpio, &bank_num, &bank_pin_num, dev);

	/* set the GPIO pin as output */
	reg = readl(platdata->base + ZYNQ_GPIO_DIRM_OFFSET(bank_num));
	reg |= BIT(bank_pin_num);
	writel(reg, platdata->base + ZYNQ_GPIO_DIRM_OFFSET(bank_num));

	/* configure the output enable reg for the pin */
	reg = readl(platdata->base + ZYNQ_GPIO_OUTEN_OFFSET(bank_num));
	reg |= BIT(bank_pin_num);
	writel(reg, platdata->base + ZYNQ_GPIO_OUTEN_OFFSET(bank_num));

	/* set the state of the pin */
	gpio_set_value(gpio, value);
	return 0;
}

static int zynq_gpio_get_function(struct udevice *dev, unsigned offset)
{
	u32 reg;
	unsigned int bank_num, bank_pin_num;
	struct zynq_gpio_platdata *platdata = dev_get_platdata(dev);

	if (check_gpio(offset, dev) < 0)
		return -1;

	zynq_gpio_get_bank_pin(offset, &bank_num, &bank_pin_num, dev);

	/* set the GPIO pin as output */
	reg = readl(platdata->base + ZYNQ_GPIO_DIRM_OFFSET(bank_num));
	reg &= BIT(bank_pin_num);
	if (reg)
		return GPIOF_OUTPUT;
	else
		return GPIOF_INPUT;
}

static const struct dm_gpio_ops gpio_zynq_ops = {
	.direction_input	= zynq_gpio_direction_input,
	.direction_output	= zynq_gpio_direction_output,
	.get_value		= zynq_gpio_get_value,
	.set_value		= zynq_gpio_set_value,
	.get_function		= zynq_gpio_get_function,
};

static const struct udevice_id zynq_gpio_ids[] = {
	{ .compatible = "xlnx,zynq-gpio-1.0",
	  .data = (ulong)&zynq_gpio_def},
	{ .compatible = "xlnx,zynqmp-gpio-1.0",
	  .data = (ulong)&zynqmp_gpio_def},
	{ }
};

static int zynq_gpio_probe(struct udevice *dev)
{
	struct zynq_gpio_platdata *platdata = dev_get_platdata(dev);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	const void *label_ptr;

	label_ptr = dev_read_prop(dev, "label", NULL);
	if (label_ptr) {
		uc_priv->bank_name = strdup(label_ptr);
		if (!uc_priv->bank_name)
			return -ENOMEM;
	} else {
		uc_priv->bank_name = dev->name;
	}

	if (platdata->p_data)
		uc_priv->gpio_count = platdata->p_data->ngpio;

	return 0;
}

static int zynq_gpio_ofdata_to_platdata(struct udevice *dev)
{
	struct zynq_gpio_platdata *platdata = dev_get_platdata(dev);

	platdata->base = (phys_addr_t)dev_read_addr(dev);

	platdata->p_data =
		(struct zynq_platform_data *)dev_get_driver_data(dev);

	return 0;
}

U_BOOT_DRIVER(gpio_zynq) = {
	.name	= "gpio_zynq",
	.id	= UCLASS_GPIO,
	.ops	= &gpio_zynq_ops,
	.of_match = zynq_gpio_ids,
	.ofdata_to_platdata = zynq_gpio_ofdata_to_platdata,
	.probe	= zynq_gpio_probe,
	.platdata_auto_alloc_size = sizeof(struct zynq_gpio_platdata),
};
