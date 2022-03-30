// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2011-2013
 * Texas Instruments, <www.ti.com>
 */

#include <common.h>
#include <i2c.h>
#include <linux/errno.h>
#include <power/pmic.h>
#include <power/tps65218.h>

#ifndef CONFIG_DM_I2C
int tps65218_reg_read(uchar dest_reg, uchar *dest_val)
{
	uchar read_val;
	int ret;

	ret = i2c_read(TPS65218_CHIP_PM, dest_reg, 1, &read_val, 1);
	if (ret)
		return ret;

	*dest_val = read_val;

	return 0;
}

/**
 *  tps65218_reg_write() - Generic function that can write a TPS65218 PMIC
 *			   register or bit field regardless of protection
 *			   level.
 *
 *  @prot_level:	   Register password protection.  Use
 *			   TPS65218_PROT_LEVEL_NONE,
 *			   TPS65218_PROT_LEVEL_1 or TPS65218_PROT_LEVEL_2
 *  @dest_reg:		   Register address to write.
 *  @dest_val:		   Value to write.
 *  @mask:		   Bit mask (8 bits) to be applied.  Function will only
 *			   change bits that are set in the bit mask.
 *
 *  @return:		   0 for success, not 0 on failure, as per the i2c API
 */
int tps65218_reg_write(uchar prot_level, uchar dest_reg, uchar dest_val,
		       uchar mask)
{
	uchar read_val;
	uchar xor_reg;
	int ret;

	/*
	 * If we are affecting only a bit field, read dest_reg and apply the
	 * mask
	 */
	if (mask != TPS65218_MASK_ALL_BITS) {
		ret = i2c_read(TPS65218_CHIP_PM, dest_reg, 1, &read_val, 1);
		if (ret)
			return ret;
		read_val &= (~mask);
		read_val |= (dest_val & mask);
		dest_val = read_val;
	}

	if (prot_level > 0) {
		xor_reg = dest_reg ^ TPS65218_PASSWORD_UNLOCK;
		ret = i2c_write(TPS65218_CHIP_PM, TPS65218_PASSWORD, 1,
				&xor_reg, 1);
		if (ret)
			return ret;
	}

	ret = i2c_write(TPS65218_CHIP_PM, dest_reg, 1, &dest_val, 1);
	if (ret)
		return ret;

	if (prot_level == TPS65218_PROT_LEVEL_2) {
		ret = i2c_write(TPS65218_CHIP_PM, TPS65218_PASSWORD, 1,
				&xor_reg, 1);
		if (ret)
			return ret;

		ret = i2c_write(TPS65218_CHIP_PM, dest_reg, 1, &dest_val, 1);
		if (ret)
			return ret;
	}

	return 0;
}
#else
struct udevice *tps65218_dev __attribute__((section(".data"))) = NULL;

int tps65218_reg_read(uchar dest_reg, uchar *dest_val)
{
	uchar read_val;
	int ret;

	if (!tps65218_dev)
		return -ENODEV;

	ret = dm_i2c_read(tps65218_dev, dest_reg,  &read_val, 1);
	if (ret)
		return ret;

	*dest_val = read_val;

	return 0;
}

int tps65218_reg_write(uchar prot_level, uchar dest_reg, uchar dest_val,
		       uchar mask)
{
	uchar read_val;
	uchar xor_reg;
	int ret;

	if (!tps65218_dev)
		return -ENODEV;

	/*
	 * If we are affecting only a bit field, read dest_reg and apply the
	 * mask
	 */
	if (mask != TPS65218_MASK_ALL_BITS) {
		ret = dm_i2c_read(tps65218_dev, dest_reg, &read_val, 1);
		if (ret)
			return ret;

		read_val &= (~mask);
		read_val |= (dest_val & mask);
		dest_val = read_val;
	}

	if (prot_level > 0) {
		xor_reg = dest_reg ^ TPS65218_PASSWORD_UNLOCK;
		ret = dm_i2c_write(tps65218_dev, TPS65218_PASSWORD, &xor_reg,
				   1);
		if (ret)
			return ret;
	}

	ret = dm_i2c_write(tps65218_dev, dest_reg, &dest_val, 1);
	if (ret)
		return ret;

	if (prot_level == TPS65218_PROT_LEVEL_2) {
		ret = dm_i2c_write(tps65218_dev, TPS65218_PASSWORD, &xor_reg,
				   1);
		if (ret)
			return ret;

		ret = dm_i2c_write(tps65218_dev, dest_reg, &dest_val, 1);
		if (ret)
			return ret;
	}

	return 0;
}
#endif

/**
 * tps65218_voltage_update() - Function to change a voltage level, as this
 *			       is a multi-step process.
 * @dc_cntrl_reg:	       DC voltage control register to change.
 * @volt_sel:		       New value for the voltage register
 * @return:		       0 for success, not 0 on failure.
 */
int tps65218_voltage_update(uchar dc_cntrl_reg, uchar volt_sel)
{
	if ((dc_cntrl_reg != TPS65218_DCDC1) &&
	    (dc_cntrl_reg != TPS65218_DCDC2) &&
	    (dc_cntrl_reg != TPS65218_DCDC3))
		return 1;

	/* set voltage level */
	if (tps65218_reg_write(TPS65218_PROT_LEVEL_2, dc_cntrl_reg, volt_sel,
			       TPS65218_DCDC_VSEL_MASK))
		return 1;

	/* set GO bit to initiate voltage transition */
	if (tps65218_reg_write(TPS65218_PROT_LEVEL_2, TPS65218_SLEW,
			       TPS65218_DCDC_GO, TPS65218_DCDC_GO))
		return 1;

	return 0;
}

/**
 * tps65218_toggle_fseal() - Perform the sequence that toggles the FSEAL bit.
 *
 * @return:		     0 on success, -EBADE if the sequence was broken
 */
int tps65218_toggle_fseal(void)
{
	if (tps65218_reg_write(TPS65218_PROT_LEVEL_NONE, TPS65218_PASSWORD,
			       0xb1, TPS65218_MASK_ALL_BITS))
		return -EBADE;

	if (tps65218_reg_write(TPS65218_PROT_LEVEL_NONE, TPS65218_PASSWORD,
			       0xfe, TPS65218_MASK_ALL_BITS))
		return -EBADE;

	if (tps65218_reg_write(TPS65218_PROT_LEVEL_NONE, TPS65218_PASSWORD,
			       0xa3, TPS65218_MASK_ALL_BITS))
		return -EBADE;

	return 0;
}

/**
 * tps65218_lock_fseal() - Perform the sequence that locks the FSEAL bit to 1.
 *
 * The FSEAL bit prevents the PMIC from turning off DCDC5 and DCDC6. It can be
 * toggled at most 3 times: 0->1, 1->0, and finally 0->1. After the third switch
 * its value is locked and can only be reset by powering off the PMIC entirely.
 *
 * @return:		   0 on success, -EBADE if the sequence was broken
 */
int tps65218_lock_fseal(void)
{
	int i;

	for (i = 0; i < 3; i++)
		if (tps65218_toggle_fseal())
			return -EBADE;

	return 0;
}

#ifndef CONFIG_DM_I2C
int power_tps65218_init(unsigned char bus)
{
	static const char name[] = "TPS65218_PMIC";
	struct pmic *p = pmic_alloc();

	if (!p) {
		printf("%s: POWER allocation error!\n", __func__);
		return -ENOMEM;
	}

	p->name = name;
	p->interface = PMIC_I2C;
	p->number_of_regs = TPS65218_PMIC_NUM_OF_REGS;
	p->hw.i2c.addr = TPS65218_CHIP_PM;
	p->hw.i2c.tx_num = 1;
	p->bus = bus;

	return 0;
}
#else
int power_tps65218_init(unsigned char bus)
{
	struct udevice *dev = NULL;
	int rc;

	rc = i2c_get_chip_for_busnum(bus, TPS65218_CHIP_PM, 1, &dev);
	if (rc)
		return rc;
	tps65218_dev = dev;
	return 0;
}
#endif
