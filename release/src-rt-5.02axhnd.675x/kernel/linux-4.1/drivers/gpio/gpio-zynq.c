/*
 * Xilinx Zynq GPIO device driver
 *
 * Copyright (C) 2009 - 2014 Xilinx, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 */

#include <linux/bitops.h>
#include <linux/clk.h>
#include <linux/gpio/driver.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>

#define DRIVER_NAME "zynq-gpio"

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

#define ZYNQ_GPIO_BANK0_PIN_MIN	0
#define ZYNQ_GPIO_BANK0_PIN_MAX	(ZYNQ_GPIO_BANK0_PIN_MIN + \
					ZYNQ_GPIO_BANK0_NGPIO - 1)
#define ZYNQ_GPIO_BANK1_PIN_MIN	(ZYNQ_GPIO_BANK0_PIN_MAX + 1)
#define ZYNQ_GPIO_BANK1_PIN_MAX	(ZYNQ_GPIO_BANK1_PIN_MIN + \
					ZYNQ_GPIO_BANK1_NGPIO - 1)
#define ZYNQ_GPIO_BANK2_PIN_MIN	(ZYNQ_GPIO_BANK1_PIN_MAX + 1)
#define ZYNQ_GPIO_BANK2_PIN_MAX	(ZYNQ_GPIO_BANK2_PIN_MIN + \
					ZYNQ_GPIO_BANK2_NGPIO - 1)
#define ZYNQ_GPIO_BANK3_PIN_MIN	(ZYNQ_GPIO_BANK2_PIN_MAX + 1)
#define ZYNQ_GPIO_BANK3_PIN_MAX	(ZYNQ_GPIO_BANK3_PIN_MIN + \
					ZYNQ_GPIO_BANK3_NGPIO - 1)


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

/**
 * struct zynq_gpio - gpio device private data structure
 * @chip:	instance of the gpio_chip
 * @base_addr:	base address of the GPIO device
 * @clk:	clock resource for this controller
 * @irq:	interrupt for the GPIO device
 */
struct zynq_gpio {
	struct gpio_chip chip;
	void __iomem *base_addr;
	struct clk *clk;
	int irq;
};

static struct irq_chip zynq_gpio_level_irqchip;
static struct irq_chip zynq_gpio_edge_irqchip;
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
					  unsigned int *bank_pin_num)
{
	switch (pin_num) {
	case ZYNQ_GPIO_BANK0_PIN_MIN ... ZYNQ_GPIO_BANK0_PIN_MAX:
		*bank_num = 0;
		*bank_pin_num = pin_num;
		break;
	case ZYNQ_GPIO_BANK1_PIN_MIN ... ZYNQ_GPIO_BANK1_PIN_MAX:
		*bank_num = 1;
		*bank_pin_num = pin_num - ZYNQ_GPIO_BANK1_PIN_MIN;
		break;
	case ZYNQ_GPIO_BANK2_PIN_MIN ... ZYNQ_GPIO_BANK2_PIN_MAX:
		*bank_num = 2;
		*bank_pin_num = pin_num - ZYNQ_GPIO_BANK2_PIN_MIN;
		break;
	case ZYNQ_GPIO_BANK3_PIN_MIN ... ZYNQ_GPIO_BANK3_PIN_MAX:
		*bank_num = 3;
		*bank_pin_num = pin_num - ZYNQ_GPIO_BANK3_PIN_MIN;
		break;
	default:
		WARN(true, "invalid GPIO pin number: %u", pin_num);
		*bank_num = 0;
		*bank_pin_num = 0;
		break;
	}
}

static const unsigned int zynq_gpio_bank_offset[] = {
	ZYNQ_GPIO_BANK0_PIN_MIN,
	ZYNQ_GPIO_BANK1_PIN_MIN,
	ZYNQ_GPIO_BANK2_PIN_MIN,
	ZYNQ_GPIO_BANK3_PIN_MIN,
};

/**
 * zynq_gpio_get_value - Get the state of the specified pin of GPIO device
 * @chip:	gpio_chip instance to be worked on
 * @pin:	gpio pin number within the device
 *
 * This function reads the state of the specified pin of the GPIO device.
 *
 * Return: 0 if the pin is low, 1 if pin is high.
 */
static int zynq_gpio_get_value(struct gpio_chip *chip, unsigned int pin)
{
	u32 data;
	unsigned int bank_num, bank_pin_num;
	struct zynq_gpio *gpio = container_of(chip, struct zynq_gpio, chip);

	zynq_gpio_get_bank_pin(pin, &bank_num, &bank_pin_num);

	data = readl_relaxed(gpio->base_addr +
			     ZYNQ_GPIO_DATA_RO_OFFSET(bank_num));

	return (data >> bank_pin_num) & 1;
}

/**
 * zynq_gpio_set_value - Modify the state of the pin with specified value
 * @chip:	gpio_chip instance to be worked on
 * @pin:	gpio pin number within the device
 * @state:	value used to modify the state of the specified pin
 *
 * This function calculates the register offset (i.e to lower 16 bits or
 * upper 16 bits) based on the given pin number and sets the state of a
 * gpio pin to the specified value. The state is either 0 or non-zero.
 */
static void zynq_gpio_set_value(struct gpio_chip *chip, unsigned int pin,
				int state)
{
	unsigned int reg_offset, bank_num, bank_pin_num;
	struct zynq_gpio *gpio = container_of(chip, struct zynq_gpio, chip);

	zynq_gpio_get_bank_pin(pin, &bank_num, &bank_pin_num);

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
	state = !!state;
	state = ~(1 << (bank_pin_num + ZYNQ_GPIO_MID_PIN_NUM)) &
		((state << bank_pin_num) | ZYNQ_GPIO_UPPER_MASK);

	writel_relaxed(state, gpio->base_addr + reg_offset);
}

/**
 * zynq_gpio_dir_in - Set the direction of the specified GPIO pin as input
 * @chip:	gpio_chip instance to be worked on
 * @pin:	gpio pin number within the device
 *
 * This function uses the read-modify-write sequence to set the direction of
 * the gpio pin as input.
 *
 * Return: 0 always
 */
static int zynq_gpio_dir_in(struct gpio_chip *chip, unsigned int pin)
{
	u32 reg;
	unsigned int bank_num, bank_pin_num;
	struct zynq_gpio *gpio = container_of(chip, struct zynq_gpio, chip);

	zynq_gpio_get_bank_pin(pin, &bank_num, &bank_pin_num);

	/* bank 0 pins 7 and 8 are special and cannot be used as inputs */
	if (bank_num == 0 && (bank_pin_num == 7 || bank_pin_num == 8))
		return -EINVAL;

	/* clear the bit in direction mode reg to set the pin as input */
	reg = readl_relaxed(gpio->base_addr + ZYNQ_GPIO_DIRM_OFFSET(bank_num));
	reg &= ~BIT(bank_pin_num);
	writel_relaxed(reg, gpio->base_addr + ZYNQ_GPIO_DIRM_OFFSET(bank_num));

	return 0;
}

/**
 * zynq_gpio_dir_out - Set the direction of the specified GPIO pin as output
 * @chip:	gpio_chip instance to be worked on
 * @pin:	gpio pin number within the device
 * @state:	value to be written to specified pin
 *
 * This function sets the direction of specified GPIO pin as output, configures
 * the Output Enable register for the pin and uses zynq_gpio_set to set
 * the state of the pin to the value specified.
 *
 * Return: 0 always
 */
static int zynq_gpio_dir_out(struct gpio_chip *chip, unsigned int pin,
			     int state)
{
	u32 reg;
	unsigned int bank_num, bank_pin_num;
	struct zynq_gpio *gpio = container_of(chip, struct zynq_gpio, chip);

	zynq_gpio_get_bank_pin(pin, &bank_num, &bank_pin_num);

	/* set the GPIO pin as output */
	reg = readl_relaxed(gpio->base_addr + ZYNQ_GPIO_DIRM_OFFSET(bank_num));
	reg |= BIT(bank_pin_num);
	writel_relaxed(reg, gpio->base_addr + ZYNQ_GPIO_DIRM_OFFSET(bank_num));

	/* configure the output enable reg for the pin */
	reg = readl_relaxed(gpio->base_addr + ZYNQ_GPIO_OUTEN_OFFSET(bank_num));
	reg |= BIT(bank_pin_num);
	writel_relaxed(reg, gpio->base_addr + ZYNQ_GPIO_OUTEN_OFFSET(bank_num));

	/* set the state of the pin */
	zynq_gpio_set_value(chip, pin, state);
	return 0;
}

/**
 * zynq_gpio_irq_mask - Disable the interrupts for a gpio pin
 * @irq_data:	per irq and chip data passed down to chip functions
 *
 * This function calculates gpio pin number from irq number and sets the
 * bit in the Interrupt Disable register of the corresponding bank to disable
 * interrupts for that pin.
 */
static void zynq_gpio_irq_mask(struct irq_data *irq_data)
{
	unsigned int device_pin_num, bank_num, bank_pin_num;
	struct zynq_gpio *gpio = irq_data_get_irq_chip_data(irq_data);

	device_pin_num = irq_data->hwirq;
	zynq_gpio_get_bank_pin(device_pin_num, &bank_num, &bank_pin_num);
	writel_relaxed(BIT(bank_pin_num),
		       gpio->base_addr + ZYNQ_GPIO_INTDIS_OFFSET(bank_num));
}

/**
 * zynq_gpio_irq_unmask - Enable the interrupts for a gpio pin
 * @irq_data:	irq data containing irq number of gpio pin for the interrupt
 *		to enable
 *
 * This function calculates the gpio pin number from irq number and sets the
 * bit in the Interrupt Enable register of the corresponding bank to enable
 * interrupts for that pin.
 */
static void zynq_gpio_irq_unmask(struct irq_data *irq_data)
{
	unsigned int device_pin_num, bank_num, bank_pin_num;
	struct zynq_gpio *gpio = irq_data_get_irq_chip_data(irq_data);

	device_pin_num = irq_data->hwirq;
	zynq_gpio_get_bank_pin(device_pin_num, &bank_num, &bank_pin_num);
	writel_relaxed(BIT(bank_pin_num),
		       gpio->base_addr + ZYNQ_GPIO_INTEN_OFFSET(bank_num));
}

/**
 * zynq_gpio_irq_ack - Acknowledge the interrupt of a gpio pin
 * @irq_data:	irq data containing irq number of gpio pin for the interrupt
 *		to ack
 *
 * This function calculates gpio pin number from irq number and sets the bit
 * in the Interrupt Status Register of the corresponding bank, to ACK the irq.
 */
static void zynq_gpio_irq_ack(struct irq_data *irq_data)
{
	unsigned int device_pin_num, bank_num, bank_pin_num;
	struct zynq_gpio *gpio = irq_data_get_irq_chip_data(irq_data);

	device_pin_num = irq_data->hwirq;
	zynq_gpio_get_bank_pin(device_pin_num, &bank_num, &bank_pin_num);
	writel_relaxed(BIT(bank_pin_num),
		       gpio->base_addr + ZYNQ_GPIO_INTSTS_OFFSET(bank_num));
}

/**
 * zynq_gpio_irq_enable - Enable the interrupts for a gpio pin
 * @irq_data:	irq data containing irq number of gpio pin for the interrupt
 *		to enable
 *
 * Clears the INTSTS bit and unmasks the given interrrupt.
 */
static void zynq_gpio_irq_enable(struct irq_data *irq_data)
{
	/*
	 * The Zynq GPIO controller does not disable interrupt detection when
	 * the interrupt is masked and only disables the propagation of the
	 * interrupt. This means when the controller detects an interrupt
	 * condition while the interrupt is logically disabled it will propagate
	 * that interrupt event once the interrupt is enabled. This will cause
	 * the interrupt consumer to see spurious interrupts to prevent this
	 * first make sure that the interrupt is not asserted and then enable
	 * it.
	 */
	zynq_gpio_irq_ack(irq_data);
	zynq_gpio_irq_unmask(irq_data);
}

/**
 * zynq_gpio_set_irq_type - Set the irq type for a gpio pin
 * @irq_data:	irq data containing irq number of gpio pin
 * @type:	interrupt type that is to be set for the gpio pin
 *
 * This function gets the gpio pin number and its bank from the gpio pin number
 * and configures the INT_TYPE, INT_POLARITY and INT_ANY registers.
 *
 * Return: 0, negative error otherwise.
 * TYPE-EDGE_RISING,  INT_TYPE - 1, INT_POLARITY - 1,  INT_ANY - 0;
 * TYPE-EDGE_FALLING, INT_TYPE - 1, INT_POLARITY - 0,  INT_ANY - 0;
 * TYPE-EDGE_BOTH,    INT_TYPE - 1, INT_POLARITY - NA, INT_ANY - 1;
 * TYPE-LEVEL_HIGH,   INT_TYPE - 0, INT_POLARITY - 1,  INT_ANY - NA;
 * TYPE-LEVEL_LOW,    INT_TYPE - 0, INT_POLARITY - 0,  INT_ANY - NA
 */
static int zynq_gpio_set_irq_type(struct irq_data *irq_data, unsigned int type)
{
	u32 int_type, int_pol, int_any;
	unsigned int device_pin_num, bank_num, bank_pin_num;
	struct zynq_gpio *gpio = irq_data_get_irq_chip_data(irq_data);

	device_pin_num = irq_data->hwirq;
	zynq_gpio_get_bank_pin(device_pin_num, &bank_num, &bank_pin_num);

	int_type = readl_relaxed(gpio->base_addr +
				 ZYNQ_GPIO_INTTYPE_OFFSET(bank_num));
	int_pol = readl_relaxed(gpio->base_addr +
				ZYNQ_GPIO_INTPOL_OFFSET(bank_num));
	int_any = readl_relaxed(gpio->base_addr +
				ZYNQ_GPIO_INTANY_OFFSET(bank_num));

	/*
	 * based on the type requested, configure the INT_TYPE, INT_POLARITY
	 * and INT_ANY registers
	 */
	switch (type) {
	case IRQ_TYPE_EDGE_RISING:
		int_type |= BIT(bank_pin_num);
		int_pol |= BIT(bank_pin_num);
		int_any &= ~BIT(bank_pin_num);
		break;
	case IRQ_TYPE_EDGE_FALLING:
		int_type |= BIT(bank_pin_num);
		int_pol &= ~BIT(bank_pin_num);
		int_any &= ~BIT(bank_pin_num);
		break;
	case IRQ_TYPE_EDGE_BOTH:
		int_type |= BIT(bank_pin_num);
		int_any |= BIT(bank_pin_num);
		break;
	case IRQ_TYPE_LEVEL_HIGH:
		int_type &= ~BIT(bank_pin_num);
		int_pol |= BIT(bank_pin_num);
		break;
	case IRQ_TYPE_LEVEL_LOW:
		int_type &= ~BIT(bank_pin_num);
		int_pol &= ~BIT(bank_pin_num);
		break;
	default:
		return -EINVAL;
	}

	writel_relaxed(int_type,
		       gpio->base_addr + ZYNQ_GPIO_INTTYPE_OFFSET(bank_num));
	writel_relaxed(int_pol,
		       gpio->base_addr + ZYNQ_GPIO_INTPOL_OFFSET(bank_num));
	writel_relaxed(int_any,
		       gpio->base_addr + ZYNQ_GPIO_INTANY_OFFSET(bank_num));

	if (type & IRQ_TYPE_LEVEL_MASK) {
		__irq_set_chip_handler_name_locked(irq_data->irq,
			&zynq_gpio_level_irqchip, handle_fasteoi_irq, NULL);
	} else {
		__irq_set_chip_handler_name_locked(irq_data->irq,
			&zynq_gpio_edge_irqchip, handle_level_irq, NULL);
	}

	return 0;
}

static int zynq_gpio_set_wake(struct irq_data *data, unsigned int on)
{
	struct zynq_gpio *gpio = irq_data_get_irq_chip_data(data);

	irq_set_irq_wake(gpio->irq, on);

	return 0;
}

/* irq chip descriptor */
static struct irq_chip zynq_gpio_level_irqchip = {
	.name		= DRIVER_NAME,
	.irq_enable	= zynq_gpio_irq_enable,
	.irq_eoi	= zynq_gpio_irq_ack,
	.irq_mask	= zynq_gpio_irq_mask,
	.irq_unmask	= zynq_gpio_irq_unmask,
	.irq_set_type	= zynq_gpio_set_irq_type,
	.irq_set_wake	= zynq_gpio_set_wake,
	.flags		= IRQCHIP_EOI_THREADED | IRQCHIP_EOI_IF_HANDLED |
			  IRQCHIP_MASK_ON_SUSPEND,
};

static struct irq_chip zynq_gpio_edge_irqchip = {
	.name		= DRIVER_NAME,
	.irq_enable	= zynq_gpio_irq_enable,
	.irq_ack	= zynq_gpio_irq_ack,
	.irq_mask	= zynq_gpio_irq_mask,
	.irq_unmask	= zynq_gpio_irq_unmask,
	.irq_set_type	= zynq_gpio_set_irq_type,
	.irq_set_wake	= zynq_gpio_set_wake,
	.flags		= IRQCHIP_MASK_ON_SUSPEND,
};

static void zynq_gpio_handle_bank_irq(struct zynq_gpio *gpio,
				      unsigned int bank_num,
				      unsigned long pending)
{
	unsigned int bank_offset = zynq_gpio_bank_offset[bank_num];
	struct irq_domain *irqdomain = gpio->chip.irqdomain;
	int offset;

	if (!pending)
		return;

	for_each_set_bit(offset, &pending, 32) {
		unsigned int gpio_irq;

		gpio_irq = irq_find_mapping(irqdomain, offset + bank_offset);
		generic_handle_irq(gpio_irq);
	}
}

/**
 * zynq_gpio_irqhandler - IRQ handler for the gpio banks of a gpio device
 * @irq:	irq number of the gpio bank where interrupt has occurred
 * @desc:	irq descriptor instance of the 'irq'
 *
 * This function reads the Interrupt Status Register of each bank to get the
 * gpio pin number which has triggered an interrupt. It then acks the triggered
 * interrupt and calls the pin specific handler set by the higher layer
 * application for that pin.
 * Note: A bug is reported if no handler is set for the gpio pin.
 */
static void zynq_gpio_irqhandler(unsigned int irq, struct irq_desc *desc)
{
	u32 int_sts, int_enb;
	unsigned int bank_num;
	struct zynq_gpio *gpio = irq_get_handler_data(irq);
	struct irq_chip *irqchip = irq_desc_get_chip(desc);

	chained_irq_enter(irqchip, desc);

	for (bank_num = 0; bank_num < ZYNQ_GPIO_MAX_BANK; bank_num++) {
		int_sts = readl_relaxed(gpio->base_addr +
					ZYNQ_GPIO_INTSTS_OFFSET(bank_num));
		int_enb = readl_relaxed(gpio->base_addr +
					ZYNQ_GPIO_INTMASK_OFFSET(bank_num));
		zynq_gpio_handle_bank_irq(gpio, bank_num, int_sts & ~int_enb);
	}

	chained_irq_exit(irqchip, desc);
}

static int __maybe_unused zynq_gpio_suspend(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	int irq = platform_get_irq(pdev, 0);
	struct irq_data *data = irq_get_irq_data(irq);

	if (!irqd_is_wakeup_set(data))
		return pm_runtime_force_suspend(dev);

	return 0;
}

static int __maybe_unused zynq_gpio_resume(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	int irq = platform_get_irq(pdev, 0);
	struct irq_data *data = irq_get_irq_data(irq);

	if (!irqd_is_wakeup_set(data))
		return pm_runtime_force_resume(dev);

	return 0;
}

static int __maybe_unused zynq_gpio_runtime_suspend(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct zynq_gpio *gpio = platform_get_drvdata(pdev);

	clk_disable_unprepare(gpio->clk);

	return 0;
}

static int __maybe_unused zynq_gpio_runtime_resume(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct zynq_gpio *gpio = platform_get_drvdata(pdev);

	return clk_prepare_enable(gpio->clk);
}

static int zynq_gpio_request(struct gpio_chip *chip, unsigned offset)
{
	int ret;

	ret = pm_runtime_get_sync(chip->dev);

	/*
	 * If the device is already active pm_runtime_get() will return 1 on
	 * success, but gpio_request still needs to return 0.
	 */
	return ret < 0 ? ret : 0;
}

static void zynq_gpio_free(struct gpio_chip *chip, unsigned offset)
{
	pm_runtime_put(chip->dev);
}

static const struct dev_pm_ops zynq_gpio_dev_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(zynq_gpio_suspend, zynq_gpio_resume)
	SET_RUNTIME_PM_OPS(zynq_gpio_runtime_suspend,
			zynq_gpio_runtime_resume, NULL)
};

/**
 * zynq_gpio_probe - Initialization method for a zynq_gpio device
 * @pdev:	platform device instance
 *
 * This function allocates memory resources for the gpio device and registers
 * all the banks of the device. It will also set up interrupts for the gpio
 * pins.
 * Note: Interrupts are disabled for all the banks during initialization.
 *
 * Return: 0 on success, negative error otherwise.
 */
static int zynq_gpio_probe(struct platform_device *pdev)
{
	int ret, bank_num;
	struct zynq_gpio *gpio;
	struct gpio_chip *chip;
	struct resource *res;

	gpio = devm_kzalloc(&pdev->dev, sizeof(*gpio), GFP_KERNEL);
	if (!gpio)
		return -ENOMEM;

	platform_set_drvdata(pdev, gpio);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	gpio->base_addr = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(gpio->base_addr))
		return PTR_ERR(gpio->base_addr);

	gpio->irq = platform_get_irq(pdev, 0);
	if (gpio->irq < 0) {
		dev_err(&pdev->dev, "invalid IRQ\n");
		return gpio->irq;
	}

	/* configure the gpio chip */
	chip = &gpio->chip;
	chip->label = "zynq_gpio";
	chip->owner = THIS_MODULE;
	chip->dev = &pdev->dev;
	chip->get = zynq_gpio_get_value;
	chip->set = zynq_gpio_set_value;
	chip->request = zynq_gpio_request;
	chip->free = zynq_gpio_free;
	chip->direction_input = zynq_gpio_dir_in;
	chip->direction_output = zynq_gpio_dir_out;
	chip->base = -1;
	chip->ngpio = ZYNQ_GPIO_NR_GPIOS;

	/* Enable GPIO clock */
	gpio->clk = devm_clk_get(&pdev->dev, NULL);
	if (IS_ERR(gpio->clk)) {
		dev_err(&pdev->dev, "input clock not found.\n");
		return PTR_ERR(gpio->clk);
	}
	ret = clk_prepare_enable(gpio->clk);
	if (ret) {
		dev_err(&pdev->dev, "Unable to enable clock.\n");
		return ret;
	}

	/* report a bug if gpio chip registration fails */
	ret = gpiochip_add(chip);
	if (ret) {
		dev_err(&pdev->dev, "Failed to add gpio chip\n");
		goto err_disable_clk;
	}

	/* disable interrupts for all banks */
	for (bank_num = 0; bank_num < ZYNQ_GPIO_MAX_BANK; bank_num++)
		writel_relaxed(ZYNQ_GPIO_IXR_DISABLE_ALL, gpio->base_addr +
			       ZYNQ_GPIO_INTDIS_OFFSET(bank_num));

	ret = gpiochip_irqchip_add(chip, &zynq_gpio_edge_irqchip, 0,
				   handle_level_irq, IRQ_TYPE_NONE);
	if (ret) {
		dev_err(&pdev->dev, "Failed to add irq chip\n");
		goto err_rm_gpiochip;
	}

	gpiochip_set_chained_irqchip(chip, &zynq_gpio_edge_irqchip, gpio->irq,
				     zynq_gpio_irqhandler);

	pm_runtime_set_active(&pdev->dev);
	pm_runtime_enable(&pdev->dev);

	return 0;

err_rm_gpiochip:
	gpiochip_remove(chip);
err_disable_clk:
	clk_disable_unprepare(gpio->clk);

	return ret;
}

/**
 * zynq_gpio_remove - Driver removal function
 * @pdev:	platform device instance
 *
 * Return: 0 always
 */
static int zynq_gpio_remove(struct platform_device *pdev)
{
	struct zynq_gpio *gpio = platform_get_drvdata(pdev);

	pm_runtime_get_sync(&pdev->dev);
	gpiochip_remove(&gpio->chip);
	clk_disable_unprepare(gpio->clk);
	device_set_wakeup_capable(&pdev->dev, 0);
	return 0;
}

static struct of_device_id zynq_gpio_of_match[] = {
	{ .compatible = "xlnx,zynq-gpio-1.0", },
	{ /* end of table */ }
};
MODULE_DEVICE_TABLE(of, zynq_gpio_of_match);

static struct platform_driver zynq_gpio_driver = {
	.driver	= {
		.name = DRIVER_NAME,
		.pm = &zynq_gpio_dev_pm_ops,
		.of_match_table = zynq_gpio_of_match,
	},
	.probe = zynq_gpio_probe,
	.remove = zynq_gpio_remove,
};

/**
 * zynq_gpio_init - Initial driver registration call
 *
 * Return: value from platform_driver_register
 */
static int __init zynq_gpio_init(void)
{
	return platform_driver_register(&zynq_gpio_driver);
}
postcore_initcall(zynq_gpio_init);

MODULE_AUTHOR("Xilinx Inc.");
MODULE_DESCRIPTION("Zynq GPIO driver");
MODULE_LICENSE("GPL");
