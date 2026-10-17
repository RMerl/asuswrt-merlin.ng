// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright(C) 2023 Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <dm.h>
#include <asm/gpio.h>
#include <power/max77663.h>
#include <power/pmic.h>

#define NUM_ENTRIES				11 /* 8 GPIOs + 3 KEYs  */
#define NUM_GPIOS				8

#define MAX77663_CNFG1_GPIO			0x36
#define GPIO_REG_ADDR(offset)			(MAX77663_CNFG1_GPIO + (offset))

#define MAX77663_CNFG_GPIO_DIR_MASK		BIT(1)
#define MAX77663_CNFG_GPIO_DIR_INPUT		BIT(1)
#define MAX77663_CNFG_GPIO_DIR_OUTPUT		0
#define MAX77663_CNFG_GPIO_INPUT_VAL_MASK	BIT(2)
#define MAX77663_CNFG_GPIO_OUTPUT_VAL_MASK	BIT(3)
#define MAX77663_CNFG_GPIO_OUTPUT_VAL_HIGH	BIT(3)
#define MAX77663_CNFG_GPIO_OUTPUT_VAL_LOW	0
#define MAX77663_CNFG_IRQ			GENMASK(5, 4)

#define MAX77663_ONOFFSTAT_REG			0x15
#define   EN0					BIT(2) /* KEY 2 */
#define   ACOK					BIT(1) /* KEY 1 */
#define   LID					BIT(0) /* KEY 0 */

static int max77663_gpio_direction_input(struct udevice *dev, unsigned int offset)
{
	int ret;

	if (offset >= NUM_GPIOS)
		return 0;

	ret = pmic_clrsetbits(dev->parent, GPIO_REG_ADDR(offset),
			      MAX77663_CNFG_GPIO_DIR_MASK,
			      MAX77663_CNFG_GPIO_DIR_INPUT);
	if (ret < 0)
		log_debug("%s: CNFG_GPIOx dir update failed: %d\n", __func__, ret);

	return ret;
}

static int max77663_gpio_direction_output(struct udevice *dev, unsigned int offset,
					  int value)
{
	u8 val;
	int ret;

	if (offset >= NUM_GPIOS)
		return -EINVAL;

	val = (value) ? MAX77663_CNFG_GPIO_OUTPUT_VAL_HIGH :
				MAX77663_CNFG_GPIO_OUTPUT_VAL_LOW;

	ret = pmic_clrsetbits(dev->parent, GPIO_REG_ADDR(offset),
			      MAX77663_CNFG_GPIO_OUTPUT_VAL_MASK, val);
	if (ret < 0) {
		log_debug("%s: CNFG_GPIOx val update failed: %d\n", __func__, ret);
		return ret;
	}

	ret = pmic_clrsetbits(dev->parent, GPIO_REG_ADDR(offset),
			      MAX77663_CNFG_GPIO_DIR_MASK,
			      MAX77663_CNFG_GPIO_DIR_OUTPUT);
	if (ret < 0)
		log_debug("%s: CNFG_GPIOx dir update failed: %d\n", __func__, ret);

	return ret;
}

static int max77663_gpio_get_value(struct udevice *dev, unsigned int offset)
{
	int ret;

	if (offset >= NUM_GPIOS) {
		ret = pmic_reg_read(dev->parent, MAX77663_ONOFFSTAT_REG);
		if (ret < 0) {
			log_debug("%s: ONOFFSTAT_REG read failed: %d\n", __func__, ret);
			return ret;
		}

		return !!(ret & BIT(offset - NUM_GPIOS));
	}

	ret = pmic_reg_read(dev->parent, GPIO_REG_ADDR(offset));
	if (ret < 0) {
		log_debug("%s: CNFG_GPIOx read failed: %d\n", __func__, ret);
		return ret;
	}

	if (ret & MAX77663_CNFG_GPIO_DIR_MASK)
		return !!(ret & MAX77663_CNFG_GPIO_INPUT_VAL_MASK);
	else
		return !!(ret & MAX77663_CNFG_GPIO_OUTPUT_VAL_MASK);
}

static int max77663_gpio_set_value(struct udevice *dev, unsigned int offset,
				   int value)
{
	u8 val;
	int ret;

	if (offset >= NUM_GPIOS)
		return -EINVAL;

	val = (value) ? MAX77663_CNFG_GPIO_OUTPUT_VAL_HIGH :
				MAX77663_CNFG_GPIO_OUTPUT_VAL_LOW;

	ret = pmic_clrsetbits(dev->parent, GPIO_REG_ADDR(offset),
			      MAX77663_CNFG_GPIO_OUTPUT_VAL_MASK, val);
	if (ret < 0)
		log_debug("%s: CNFG_GPIO_OUT update failed: %d\n", __func__, ret);

	return ret;
}

static int max77663_gpio_get_function(struct udevice *dev, unsigned int offset)
{
	int ret;

	if (offset >= NUM_GPIOS)
		return GPIOF_INPUT;

	ret = pmic_reg_read(dev->parent, GPIO_REG_ADDR(offset));
	if (ret < 0) {
		log_debug("%s: CNFG_GPIOx read failed: %d\n", __func__, ret);
		return ret;
	}

	if (ret & MAX77663_CNFG_GPIO_DIR_MASK)
		return GPIOF_INPUT;
	else
		return GPIOF_OUTPUT;
}

static const struct dm_gpio_ops max77663_gpio_ops = {
	.direction_input	= max77663_gpio_direction_input,
	.direction_output	= max77663_gpio_direction_output,
	.get_value		= max77663_gpio_get_value,
	.set_value		= max77663_gpio_set_value,
	.get_function		= max77663_gpio_get_function,
};

static int max77663_gpio_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	int i, ret;

	uc_priv->gpio_count = NUM_ENTRIES;
	uc_priv->bank_name = "GPIO";

	/*
	 * GPIO interrupts may be left ON after bootloader, hence let's
	 * pre-initialize hardware to the expected state by disabling all
	 * the interrupts.
	 */
	for (i = 0; i < NUM_GPIOS; i++) {
		ret = pmic_clrsetbits(dev->parent, GPIO_REG_ADDR(i),
				      MAX77663_CNFG_IRQ, 0);
		if (ret < 0) {
			log_debug("%s: failed to disable interrupt: %d\n", __func__, ret);
			return ret;
		}
	}

	return 0;
}

U_BOOT_DRIVER(max77663_gpio) = {
	.name	= MAX77663_GPIO_DRIVER,
	.id	= UCLASS_GPIO,
	.probe	= max77663_gpio_probe,
	.ops	= &max77663_gpio_ops,
};
