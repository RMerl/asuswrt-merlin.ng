// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2011-2013
 * Texas Instruments, <www.ti.com>
 */

#include <common.h>
#include <i2c.h>
#include <power/tps65910.h>

struct udevice *tps65910_dev __section(".data") = NULL;

static inline int tps65910_read_reg(int addr, uchar *buf)
{
#if !CONFIG_IS_ENABLED(DM_I2C)
	return i2c_read(TPS65910_CTRL_I2C_ADDR, addr, 1, buf, 1);
#else
	int rc;

	rc = dm_i2c_reg_read(tps65910_dev, addr);
	if (rc < 0)
		return rc;
	*buf = (uchar)rc;
	return 0;
#endif
}

static inline int tps65910_write_reg(int addr, uchar *buf)
{
#if !CONFIG_IS_ENABLED(DM_I2C)
	return i2c_write(TPS65910_CTRL_I2C_ADDR, addr, 1, buf, 1);
#else
	return dm_i2c_reg_write(tps65910_dev, addr, *buf);
#endif
}

int power_tps65910_init(unsigned char bus)
{
#if CONFIG_IS_ENABLED(DM_I2C)
	struct udevice *dev = NULL;
	int rc;

	rc = i2c_get_chip_for_busnum(bus, TPS65910_CTRL_I2C_ADDR, 1, &dev);

	if (rc)
		return rc;
	tps65910_dev = dev;
#endif
	return 0;
}

/*
 * tps65910_set_i2c_control() - Set the TPS65910 to be controlled via the I2C
 *				interface.
 * @return:		       0 on success, not 0 on failure
 */
int tps65910_set_i2c_control(void)
{
	int ret;
	uchar buf;

	/* VDD1/2 voltage selection register access by control i/f */
	ret = tps65910_read_reg(TPS65910_DEVCTRL_REG, &buf);

	if (ret)
		return ret;

	buf |= TPS65910_DEVCTRL_REG_SR_CTL_I2C_SEL_CTL_I2C;

	return tps65910_write_reg(TPS65910_DEVCTRL_REG, &buf);
}

/*
 * tps65910_voltage_update() - Voltage switching for MPU frequency switching.
 * @module:		       mpu - 0, core - 1
 * @vddx_op_vol_sel:	       vdd voltage to set
 * @return:		       0 on success, not 0 on failure
 */
int tps65910_voltage_update(unsigned int module, unsigned char vddx_op_vol_sel)
{
	uchar buf;
	unsigned int reg_offset;
	int ret;

	if (module == MPU)
		reg_offset = TPS65910_VDD1_OP_REG;
	else
		reg_offset = TPS65910_VDD2_OP_REG;

	/* Select VDDx OP   */
	ret = tps65910_read_reg(reg_offset, &buf);
	if (ret)
		return ret;

	buf &= ~TPS65910_OP_REG_CMD_MASK;

	ret = tps65910_write_reg(reg_offset, &buf);
	if (ret)
		return ret;

	/* Configure VDDx OP  Voltage */
	ret = tps65910_read_reg(reg_offset, &buf);
	if (ret)
		return ret;

	buf &= ~TPS65910_OP_REG_SEL_MASK;
	buf |= vddx_op_vol_sel;

	ret = tps65910_write_reg(reg_offset, &buf);
	if (ret)
		return ret;

	ret = tps65910_read_reg(reg_offset, &buf);
	if (ret)
		return ret;

	if ((buf & TPS65910_OP_REG_SEL_MASK) != vddx_op_vol_sel)
		return 1;

	return 0;
}
