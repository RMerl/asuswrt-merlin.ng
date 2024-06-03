// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Hans de Goede <hdegoede@redhat.com>
 *
 * Sunxi PMIC bus access helpers
 *
 * The axp152 & axp209 use an i2c bus, the axp221 uses the p2wi bus and the
 * axp223 uses the rsb bus, these functions abstract this.
 */

#include <common.h>
#include <asm/arch/p2wi.h>
#include <asm/arch/rsb.h>
#include <i2c.h>
#include <asm/arch/pmic_bus.h>

#define AXP152_I2C_ADDR			0x30

#define AXP209_I2C_ADDR			0x34

#define AXP221_CHIP_ADDR		0x68
#define AXP221_CTRL_ADDR		0x3e
#define AXP221_INIT_DATA		0x3e

/* AXP818 device and runtime addresses are same as AXP223 */
#define AXP223_DEVICE_ADDR		0x3a3
#define AXP223_RUNTIME_ADDR		0x2d

int pmic_bus_init(void)
{
	/* This cannot be 0 because it is used in SPL before BSS is ready */
	static int needs_init = 1;
	__maybe_unused int ret;

	if (!needs_init)
		return 0;

#if defined CONFIG_AXP221_POWER || defined CONFIG_AXP809_POWER || defined CONFIG_AXP818_POWER
# ifdef CONFIG_MACH_SUN6I
	p2wi_init();
	ret = p2wi_change_to_p2wi_mode(AXP221_CHIP_ADDR, AXP221_CTRL_ADDR,
				       AXP221_INIT_DATA);
# elif defined CONFIG_MACH_SUN8I_R40
	/* Nothing. R40 uses the AXP221s in I2C mode */
	ret = 0;
# else
	ret = rsb_init();
	if (ret)
		return ret;

	ret = rsb_set_device_address(AXP223_DEVICE_ADDR, AXP223_RUNTIME_ADDR);
# endif
	if (ret)
		return ret;
#endif

	needs_init = 0;
	return 0;
}

int pmic_bus_read(u8 reg, u8 *data)
{
#ifdef CONFIG_AXP152_POWER
	return i2c_read(AXP152_I2C_ADDR, reg, 1, data, 1);
#elif defined CONFIG_AXP209_POWER
	return i2c_read(AXP209_I2C_ADDR, reg, 1, data, 1);
#elif defined CONFIG_AXP221_POWER || defined CONFIG_AXP809_POWER || defined CONFIG_AXP818_POWER
# ifdef CONFIG_MACH_SUN6I
	return p2wi_read(reg, data);
# elif defined CONFIG_MACH_SUN8I_R40
	return i2c_read(AXP209_I2C_ADDR, reg, 1, data, 1);
# else
	return rsb_read(AXP223_RUNTIME_ADDR, reg, data);
# endif
#endif
}

int pmic_bus_write(u8 reg, u8 data)
{
#ifdef CONFIG_AXP152_POWER
	return i2c_write(AXP152_I2C_ADDR, reg, 1, &data, 1);
#elif defined CONFIG_AXP209_POWER
	return i2c_write(AXP209_I2C_ADDR, reg, 1, &data, 1);
#elif defined CONFIG_AXP221_POWER || defined CONFIG_AXP809_POWER || defined CONFIG_AXP818_POWER
# ifdef CONFIG_MACH_SUN6I
	return p2wi_write(reg, data);
# elif defined CONFIG_MACH_SUN8I_R40
	return i2c_write(AXP209_I2C_ADDR, reg, 1, &data, 1);
# else
	return rsb_write(AXP223_RUNTIME_ADDR, reg, data);
# endif
#endif
}

int pmic_bus_setbits(u8 reg, u8 bits)
{
	int ret;
	u8 val;

	ret = pmic_bus_read(reg, &val);
	if (ret)
		return ret;

	if ((val & bits) == bits)
		return 0;

	val |= bits;
	return pmic_bus_write(reg, val);
}

int pmic_bus_clrbits(u8 reg, u8 bits)
{
	int ret;
	u8 val;

	ret = pmic_bus_read(reg, &val);
	if (ret)
		return ret;

	if (!(val & bits))
		return 0;

	val &= ~bits;
	return pmic_bus_write(reg, val);
}
