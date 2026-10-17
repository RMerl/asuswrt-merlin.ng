// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011 Samsung Electronics
 * Lukasz Majewski <l.majewski@samsung.com>
 *
 * (C) Copyright 2010
 * Stefano Babic, DENX Software Engineering, sbabic@denx.de
 *
 * (C) Copyright 2008-2009 Freescale Semiconductor, Inc.
 * (C) Copyright 2019 NXP
 */

#include <common.h>
#include <log.h>
#include <linux/types.h>
#include <power/pmic.h>
#include <i2c.h>
#include <linux/compiler.h>

int pmic_reg_write(struct pmic *p, u32 reg, u32 val)
{
	unsigned char buf[4] = { 0 };

	if (check_reg(p, reg))
		return -EINVAL;
#if CONFIG_IS_ENABLED(DM_I2C)
	struct udevice *dev;
	int ret;

	ret = i2c_get_chip_for_busnum(p->bus, pmic_i2c_addr,
				      1, &dev);
	if (ret) {
		printf("%s: Cannot find udev for a bus %d\n", __func__,
		       p->bus);
		return -ENXIO;
	}
#else /* Non DM I2C support - will be removed */
	I2C_SET_BUS(p->bus);
#endif

	switch (pmic_i2c_tx_num) {
	case 3:
		if (p->sensor_byte_order == PMIC_SENSOR_BYTE_ORDER_BIG) {
			buf[2] = (cpu_to_le32(val) >> 16) & 0xff;
			buf[1] = (cpu_to_le32(val) >> 8) & 0xff;
			buf[0] = cpu_to_le32(val) & 0xff;
		} else {
			buf[0] = (cpu_to_le32(val) >> 16) & 0xff;
			buf[1] = (cpu_to_le32(val) >> 8) & 0xff;
			buf[2] = cpu_to_le32(val) & 0xff;
		}
		break;
	case 2:
		if (p->sensor_byte_order == PMIC_SENSOR_BYTE_ORDER_BIG) {
			buf[1] = (cpu_to_le32(val) >> 8) & 0xff;
			buf[0] = cpu_to_le32(val) & 0xff;
		} else {
			buf[0] = (cpu_to_le32(val) >> 8) & 0xff;
			buf[1] = cpu_to_le32(val) & 0xff;
		}
		break;
	case 1:
		buf[0] = cpu_to_le32(val) & 0xff;
		break;
	default:
		printf("%s: invalid tx_num: %d", __func__, pmic_i2c_tx_num);
		return -EINVAL;
	}

#if CONFIG_IS_ENABLED(DM_I2C)
	return dm_i2c_write(dev, reg, buf, pmic_i2c_tx_num);
#else
	return i2c_write(pmic_i2c_addr, reg, 1, buf, pmic_i2c_tx_num);
#endif
}

int pmic_reg_read(struct pmic *p, u32 reg, u32 *val)
{
	unsigned char buf[4] = { 0 };
	u32 ret_val = 0;
	int ret;

	if (check_reg(p, reg))
		return -EINVAL;

#if CONFIG_IS_ENABLED(DM_I2C)
	struct udevice *dev;

	ret = i2c_get_chip_for_busnum(p->bus, pmic_i2c_addr,
				      1, &dev);
	if (ret) {
		printf("%s: Cannot find udev for a bus %d\n", __func__,
		       p->bus);
		return -ENXIO;
	}
	ret = dm_i2c_read(dev, reg, buf, pmic_i2c_tx_num);
#else /* Non DM I2C support - will be removed */
	I2C_SET_BUS(p->bus);
	ret = i2c_read(pmic_i2c_addr, reg, 1, buf, pmic_i2c_tx_num);
#endif
	if (ret)
		return ret;

	switch (pmic_i2c_tx_num) {
	case 3:
		if (p->sensor_byte_order == PMIC_SENSOR_BYTE_ORDER_BIG)
			ret_val = le32_to_cpu(buf[2] << 16
					      | buf[1] << 8 | buf[0]);
		else
			ret_val = le32_to_cpu(buf[0] << 16 |
					      buf[1] << 8 | buf[2]);
		break;
	case 2:
		if (p->sensor_byte_order == PMIC_SENSOR_BYTE_ORDER_BIG)
			ret_val = le32_to_cpu(buf[1] << 8 | buf[0]);
		else
			ret_val = le32_to_cpu(buf[0] << 8 | buf[1]);
		break;
	case 1:
		ret_val = le32_to_cpu(buf[0]);
		break;
	default:
		printf("%s: invalid tx_num: %d", __func__, pmic_i2c_tx_num);
		return -EINVAL;
	}
	memcpy(val, &ret_val, sizeof(ret_val));

	return 0;
}

int pmic_probe(struct pmic *p)
{
	debug("Bus: %d PMIC:%s probed!\n", p->bus, p->name);
#if CONFIG_IS_ENABLED(DM_I2C)
	struct udevice *dev;
	int ret;

	ret = i2c_get_chip_for_busnum(p->bus, pmic_i2c_addr,
				      1, &dev);
	if (ret) {
		printf("%s: Cannot find udev for a bus %d\n", __func__,
		       p->bus);
		return -ENXIO;
	}
#else /* Non DM I2C support - will be removed */
	i2c_set_bus_num(p->bus);
	if (i2c_probe(pmic_i2c_addr)) {
		printf("Can't find PMIC:%s\n", p->name);
		return -ENODEV;
	}
#endif

	return 0;
}
