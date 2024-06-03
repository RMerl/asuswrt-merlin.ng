// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 */
#include <config.h>

#include <twl6030.h>

static struct twl6030_data *twl;

static struct twl6030_data twl6030_info = {
	.chip_type	= chip_TWL6030,
	.adc_rbase	= GPCH0_LSB,
	.adc_ctrl	= CTRL_P2,
	.adc_enable	= CTRL_P2_SP2,
	.vbat_mult	= TWL6030_VBAT_MULT,
	.vbat_shift	= TWL6030_VBAT_SHIFT,
};

static struct twl6030_data twl6032_info = {
	.chip_type	= chip_TWL6032,
	.adc_rbase	= TWL6032_GPCH0_LSB,
	.adc_ctrl	= TWL6032_CTRL_P1,
	.adc_enable	= CTRL_P1_SP1,
	.vbat_mult	= TWL6032_VBAT_MULT,
	.vbat_shift	= TWL6032_VBAT_SHIFT,
};


static int twl6030_gpadc_read_channel(u8 channel_no)
{
	u8 lsb = 0;
	u8 msb = 0;
	int ret = 0;

	ret = twl6030_i2c_read_u8(TWL6030_CHIP_ADC,
				  twl->adc_rbase + channel_no * 2, &lsb);
	if (ret)
		return ret;

	ret = twl6030_i2c_read_u8(TWL6030_CHIP_ADC,
				  twl->adc_rbase + 1 + channel_no * 2, &msb);
	if (ret)
		return ret;

	return (msb << 8) | lsb;
}

static int twl6030_gpadc_sw2_trigger(void)
{
	u8 val;
	int ret = 0;

	ret = twl6030_i2c_write_u8(TWL6030_CHIP_ADC,
				   twl->adc_ctrl, twl->adc_enable);
	if (ret)
		return ret;

	/* Waiting until the SW1 conversion ends*/
	val =  CTRL_P2_BUSY;

	while (!((val & CTRL_P2_EOCP2) && (!(val & CTRL_P2_BUSY)))) {
		ret = twl6030_i2c_read_u8(TWL6030_CHIP_ADC,
					  twl->adc_ctrl, &val);
		if (ret)
			return ret;
		udelay(1000);
	}

	return 0;
}

void twl6030_power_off(void)
{
	twl6030_i2c_write_u8(TWL6030_CHIP_PM, TWL6030_PHOENIX_DEV_ON,
		TWL6030_PHOENIX_APP_DEVOFF | TWL6030_PHOENIX_CON_DEVOFF |
		TWL6030_PHOENIX_MOD_DEVOFF);
}

void twl6030_stop_usb_charging(void)
{
	twl6030_i2c_write_u8(TWL6030_CHIP_CHARGER, CONTROLLER_CTRL1, 0);

	return;
}

void twl6030_start_usb_charging(void)
{
	twl6030_i2c_write_u8(TWL6030_CHIP_CHARGER,
			     CHARGERUSB_VICHRG, CHARGERUSB_VICHRG_1500);
	twl6030_i2c_write_u8(TWL6030_CHIP_CHARGER,
			     CHARGERUSB_CINLIMIT, CHARGERUSB_CIN_LIMIT_NONE);
	twl6030_i2c_write_u8(TWL6030_CHIP_CHARGER,
			     CONTROLLER_INT_MASK, MBAT_TEMP);
	twl6030_i2c_write_u8(TWL6030_CHIP_CHARGER,
			     CHARGERUSB_INT_MASK, MASK_MCHARGERUSB_THMREG);
	twl6030_i2c_write_u8(TWL6030_CHIP_CHARGER,
			     CHARGERUSB_VOREG, CHARGERUSB_VOREG_4P0);
	twl6030_i2c_write_u8(TWL6030_CHIP_CHARGER,
			     CHARGERUSB_CTRL2, CHARGERUSB_CTRL2_VITERM_400);
	twl6030_i2c_write_u8(TWL6030_CHIP_CHARGER, CHARGERUSB_CTRL1, TERM);
	/* Enable USB charging */
	twl6030_i2c_write_u8(TWL6030_CHIP_CHARGER,
			     CONTROLLER_CTRL1, CONTROLLER_CTRL1_EN_CHARGER);
	return;
}

int twl6030_get_battery_current(void)
{
	int battery_current = 0;
	u8 msb = 0;
	u8 lsb = 0;

	twl6030_i2c_read_u8(TWL6030_CHIP_CHARGER, FG_REG_11, &msb);
	twl6030_i2c_read_u8(TWL6030_CHIP_CHARGER, FG_REG_10, &lsb);
	battery_current = ((msb << 8) | lsb);

	/* convert 10 bit signed number to 16 bit signed number */
	if (battery_current >= 0x2000)
		battery_current = (battery_current - 0x4000);

	battery_current = battery_current * 3000 / 4096;
	printf("Battery Current: %d mA\n", battery_current);

	return battery_current;
}

int twl6030_get_battery_voltage(void)
{
	int battery_volt = 0;
	int ret = 0;
	u8 vbatch;

	if (twl->chip_type == chip_TWL6030) {
		vbatch = TWL6030_GPADC_VBAT_CHNL;
	} else {
		ret = twl6030_i2c_write_u8(TWL6030_CHIP_ADC,
					   TWL6032_GPSELECT_ISB,
					   TWL6032_GPADC_VBAT_CHNL);
		if (ret)
			return ret;
		vbatch = 0;
	}

	/* Start GPADC SW conversion */
	ret = twl6030_gpadc_sw2_trigger();
	if (ret) {
		printf("Failed to convert battery voltage\n");
		return ret;
	}

	/* measure Vbat voltage */
	battery_volt = twl6030_gpadc_read_channel(vbatch);
	if (battery_volt < 0) {
		printf("Failed to read battery voltage\n");
		return ret;
	}
	battery_volt = (battery_volt * twl->vbat_mult) >> twl->vbat_shift;
	printf("Battery Voltage: %d mV\n", battery_volt);

	return battery_volt;
}

void twl6030_init_battery_charging(void)
{
	u8 val = 0;
	int battery_volt = 0;
	int ret = 0;

	ret = twl6030_i2c_read_u8(TWL6030_CHIP_USB, USB_PRODUCT_ID_LSB, &val);
	if (ret) {
		puts("twl6030_init_battery_charging(): could not determine chip!\n");
		return;
	}
	if (val == 0x30) {
		twl = &twl6030_info;
	} else if (val == 0x32) {
		twl = &twl6032_info;
	} else {
		puts("twl6030_init_battery_charging(): unsupported chip type\n");
		return;
	}

	/* Enable VBAT measurement */
	if (twl->chip_type == chip_TWL6030) {
		twl6030_i2c_write_u8(TWL6030_CHIP_PM, MISC1, VBAT_MEAS);
		twl6030_i2c_write_u8(TWL6030_CHIP_ADC,
				     TWL6030_GPADC_CTRL,
				     GPADC_CTRL_SCALER_DIV4);
	} else {
		twl6030_i2c_write_u8(TWL6030_CHIP_ADC,
				     TWL6032_GPADC_CTRL2,
				     GPADC_CTRL2_CH18_SCALER_EN);
	}

	/* Enable GPADC module */
	ret = twl6030_i2c_write_u8(TWL6030_CHIP_CHARGER, TOGGLE1, FGS | GPADCS);
	if (ret) {
		printf("Failed to enable GPADC\n");
		return;
	}

	battery_volt = twl6030_get_battery_voltage();
	if (battery_volt < 0)
		return;

	if (battery_volt < 3000)
		printf("Main battery voltage too low!\n");

	/* Check for the presence of USB charger */
	twl6030_i2c_read_u8(TWL6030_CHIP_CHARGER, CONTROLLER_STAT1, &val);

	/* check for battery presence indirectly via Fuel gauge */
	if ((val & VBUS_DET) && (battery_volt < 3300))
		twl6030_start_usb_charging();

	return;
}

void twl6030_power_mmc_init(int dev_index)
{
	u8 value = 0;

	if (dev_index == 0) {
		/* 3.0V voltage output for VMMC */
		twl6030_i2c_write_u8(TWL6030_CHIP_PM, TWL6030_VMMC_CFG_VOLTAGE,
			TWL6030_CFG_VOLTAGE_30);

		/* Enable P1 output for VMMC */
		twl6030_i2c_write_u8(TWL6030_CHIP_PM, TWL6030_VMMC_CFG_STATE,
			TWL6030_CFG_STATE_P1 | TWL6030_CFG_STATE_ON);
	} else if (dev_index == 1) {
		twl6030_i2c_read_u8(TWL6030_CHIP_PM, TWL6030_PH_STS_BOOT,
				    &value);
		/* BOOT2 indicates 1.8V/2.8V VAUX1 for eMMC */
		if (value & TWL6030_PH_STS_BOOT2) {
			/* 1.8V voltage output for VAUX1 */
			twl6030_i2c_write_u8(TWL6030_CHIP_PM, TWL6030_VAUX1_CFG_VOLTAGE,
				TWL6030_CFG_VOLTAGE_18);
		} else {
			/* 2.8V voltage output for VAUX1 */
			twl6030_i2c_write_u8(TWL6030_CHIP_PM, TWL6030_VAUX1_CFG_VOLTAGE,
				TWL6030_CFG_VOLTAGE_28);
		}

		/* Enable P1 output for VAUX */
		twl6030_i2c_write_u8(TWL6030_CHIP_PM, TWL6030_VAUX1_CFG_STATE,
			TWL6030_CFG_STATE_P1 | TWL6030_CFG_STATE_ON);
	}
}

void twl6030_usb_device_settings()
{
	u8 value = 0;

	/* 3.3V voltage output for VUSB */
	twl6030_i2c_write_u8(TWL6030_CHIP_PM, TWL6030_VUSB_CFG_VOLTAGE,
		TWL6030_CFG_VOLTAGE_33);

	/* Enable P1 output for VUSB */
	twl6030_i2c_write_u8(TWL6030_CHIP_PM, TWL6030_VUSB_CFG_STATE,
		TWL6030_CFG_STATE_P1 | TWL6030_CFG_STATE_ON);

	/* Select the input supply for VUSB regulator */
	twl6030_i2c_read_u8(TWL6030_CHIP_PM, TWL6030_MISC2, &value);
	value |= TWL6030_MISC2_VUSB_IN_VSYS;
	value &= ~TWL6030_MISC2_VUSB_IN_PMID;
	twl6030_i2c_write_u8(TWL6030_CHIP_PM, TWL6030_MISC2, value);
}

#ifdef CONFIG_DM_I2C
int twl6030_i2c_write_u8(u8 chip_no, u8 reg, u8 val)
{
	struct udevice *dev;
	int ret;

	ret = i2c_get_chip_for_busnum(0, chip_no, 1, &dev);
	if (ret) {
		pr_err("unable to get I2C bus. ret %d\n", ret);
		return ret;
	}
	ret = dm_i2c_reg_write(dev, reg, val);
	if (ret) {
		pr_err("writing to twl6030 failed. ret %d\n", ret);
		return ret;
	}
	return 0;
}

int twl6030_i2c_read_u8(u8 chip_no, u8 reg, u8 *valp)
{
	struct udevice *dev;
	int ret;

	ret = i2c_get_chip_for_busnum(0, chip_no, 1, &dev);
	if (ret) {
		pr_err("unable to get I2C bus. ret %d\n", ret);
		return ret;
	}
	ret = dm_i2c_reg_read(dev, reg);
	if (ret < 0) {
		pr_err("reading from twl6030 failed. ret %d\n", ret);
		return ret;
	}
	*valp = (u8)ret;
	return 0;
}
#endif
