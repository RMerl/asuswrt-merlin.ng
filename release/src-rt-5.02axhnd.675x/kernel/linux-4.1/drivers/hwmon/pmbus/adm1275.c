/*
 * Hardware monitoring driver for Analog Devices ADM1275 Hot-Swap Controller
 * and Digital Power Monitor
 *
 * Copyright (c) 2011 Ericsson AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include "pmbus.h"

enum chips { adm1075, adm1275, adm1276 };

#define ADM1275_PEAK_IOUT		0xd0
#define ADM1275_PEAK_VIN		0xd1
#define ADM1275_PEAK_VOUT		0xd2
#define ADM1275_PMON_CONFIG		0xd4

#define ADM1275_VIN_VOUT_SELECT		(1 << 6)
#define ADM1275_VRANGE			(1 << 5)
#define ADM1075_IRANGE_50		(1 << 4)
#define ADM1075_IRANGE_25		(1 << 3)
#define ADM1075_IRANGE_MASK		((1 << 3) | (1 << 4))

#define ADM1275_IOUT_WARN2_LIMIT	0xd7
#define ADM1275_DEVICE_CONFIG		0xd8

#define ADM1275_IOUT_WARN2_SELECT	(1 << 4)

#define ADM1276_PEAK_PIN		0xda

#define ADM1275_MFR_STATUS_IOUT_WARN2	(1 << 0)

#define ADM1075_READ_VAUX		0xdd
#define ADM1075_VAUX_OV_WARN_LIMIT	0xde
#define ADM1075_VAUX_UV_WARN_LIMIT	0xdf
#define ADM1075_VAUX_STATUS		0xf6

#define ADM1075_VAUX_OV_WARN		(1<<7)
#define ADM1075_VAUX_UV_WARN		(1<<6)

struct adm1275_data {
	int id;
	bool have_oc_fault;
	struct pmbus_driver_info info;
};

#define to_adm1275_data(x)  container_of(x, struct adm1275_data, info)

static int adm1275_read_word_data(struct i2c_client *client, int page, int reg)
{
	const struct pmbus_driver_info *info = pmbus_get_driver_info(client);
	const struct adm1275_data *data = to_adm1275_data(info);
	int ret = 0;

	if (page)
		return -ENXIO;

	switch (reg) {
	case PMBUS_IOUT_UC_FAULT_LIMIT:
		if (data->have_oc_fault) {
			ret = -ENXIO;
			break;
		}
		ret = pmbus_read_word_data(client, 0, ADM1275_IOUT_WARN2_LIMIT);
		break;
	case PMBUS_IOUT_OC_FAULT_LIMIT:
		if (!data->have_oc_fault) {
			ret = -ENXIO;
			break;
		}
		ret = pmbus_read_word_data(client, 0, ADM1275_IOUT_WARN2_LIMIT);
		break;
	case PMBUS_VOUT_OV_WARN_LIMIT:
		if (data->id != adm1075) {
			ret = -ENODATA;
			break;
		}
		ret = pmbus_read_word_data(client, 0,
					   ADM1075_VAUX_OV_WARN_LIMIT);
		break;
	case PMBUS_VOUT_UV_WARN_LIMIT:
		if (data->id != adm1075) {
			ret = -ENODATA;
			break;
		}
		ret = pmbus_read_word_data(client, 0,
					   ADM1075_VAUX_UV_WARN_LIMIT);
		break;
	case PMBUS_READ_VOUT:
		if (data->id != adm1075) {
			ret = -ENODATA;
			break;
		}
		ret = pmbus_read_word_data(client, 0, ADM1075_READ_VAUX);
		break;
	case PMBUS_VIRT_READ_IOUT_MAX:
		ret = pmbus_read_word_data(client, 0, ADM1275_PEAK_IOUT);
		break;
	case PMBUS_VIRT_READ_VOUT_MAX:
		ret = pmbus_read_word_data(client, 0, ADM1275_PEAK_VOUT);
		break;
	case PMBUS_VIRT_READ_VIN_MAX:
		ret = pmbus_read_word_data(client, 0, ADM1275_PEAK_VIN);
		break;
	case PMBUS_VIRT_READ_PIN_MAX:
		if (data->id == adm1275) {
			ret = -ENXIO;
			break;
		}
		ret = pmbus_read_word_data(client, 0, ADM1276_PEAK_PIN);
		break;
	case PMBUS_VIRT_RESET_IOUT_HISTORY:
	case PMBUS_VIRT_RESET_VOUT_HISTORY:
	case PMBUS_VIRT_RESET_VIN_HISTORY:
		break;
	case PMBUS_VIRT_RESET_PIN_HISTORY:
		if (data->id == adm1275)
			ret = -ENXIO;
		break;
	default:
		ret = -ENODATA;
		break;
	}
	return ret;
}

static int adm1275_write_word_data(struct i2c_client *client, int page, int reg,
				   u16 word)
{
	int ret;

	if (page)
		return -ENXIO;

	switch (reg) {
	case PMBUS_IOUT_UC_FAULT_LIMIT:
	case PMBUS_IOUT_OC_FAULT_LIMIT:
		ret = pmbus_write_word_data(client, 0, ADM1275_IOUT_WARN2_LIMIT,
					    word);
		break;
	case PMBUS_VIRT_RESET_IOUT_HISTORY:
		ret = pmbus_write_word_data(client, 0, ADM1275_PEAK_IOUT, 0);
		break;
	case PMBUS_VIRT_RESET_VOUT_HISTORY:
		ret = pmbus_write_word_data(client, 0, ADM1275_PEAK_VOUT, 0);
		break;
	case PMBUS_VIRT_RESET_VIN_HISTORY:
		ret = pmbus_write_word_data(client, 0, ADM1275_PEAK_VIN, 0);
		break;
	case PMBUS_VIRT_RESET_PIN_HISTORY:
		ret = pmbus_write_word_data(client, 0, ADM1276_PEAK_PIN, 0);
		break;
	default:
		ret = -ENODATA;
		break;
	}
	return ret;
}

static int adm1275_read_byte_data(struct i2c_client *client, int page, int reg)
{
	const struct pmbus_driver_info *info = pmbus_get_driver_info(client);
	const struct adm1275_data *data = to_adm1275_data(info);
	int mfr_status, ret;

	if (page > 0)
		return -ENXIO;

	switch (reg) {
	case PMBUS_STATUS_IOUT:
		ret = pmbus_read_byte_data(client, page, PMBUS_STATUS_IOUT);
		if (ret < 0)
			break;
		mfr_status = pmbus_read_byte_data(client, page,
						  PMBUS_STATUS_MFR_SPECIFIC);
		if (mfr_status < 0) {
			ret = mfr_status;
			break;
		}
		if (mfr_status & ADM1275_MFR_STATUS_IOUT_WARN2) {
			ret |= data->have_oc_fault ?
			  PB_IOUT_OC_FAULT : PB_IOUT_UC_FAULT;
		}
		break;
	case PMBUS_STATUS_VOUT:
		if (data->id != adm1075) {
			ret = -ENODATA;
			break;
		}
		ret = 0;
		mfr_status = pmbus_read_byte_data(client, 0,
						  ADM1075_VAUX_STATUS);
		if (mfr_status & ADM1075_VAUX_OV_WARN)
			ret |= PB_VOLTAGE_OV_WARNING;
		if (mfr_status & ADM1075_VAUX_UV_WARN)
			ret |= PB_VOLTAGE_UV_WARNING;
		break;
	default:
		ret = -ENODATA;
		break;
	}
	return ret;
}

static const struct i2c_device_id adm1275_id[] = {
	{ "adm1075", adm1075 },
	{ "adm1275", adm1275 },
	{ "adm1276", adm1276 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, adm1275_id);

static int adm1275_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	u8 block_buffer[I2C_SMBUS_BLOCK_MAX + 1];
	int config, device_config;
	int ret;
	struct pmbus_driver_info *info;
	struct adm1275_data *data;
	const struct i2c_device_id *mid;

	if (!i2c_check_functionality(client->adapter,
				     I2C_FUNC_SMBUS_READ_BYTE_DATA
				     | I2C_FUNC_SMBUS_BLOCK_DATA))
		return -ENODEV;

	ret = i2c_smbus_read_block_data(client, PMBUS_MFR_ID, block_buffer);
	if (ret < 0) {
		dev_err(&client->dev, "Failed to read Manufacturer ID\n");
		return ret;
	}
	if (ret != 3 || strncmp(block_buffer, "ADI", 3)) {
		dev_err(&client->dev, "Unsupported Manufacturer ID\n");
		return -ENODEV;
	}

	ret = i2c_smbus_read_block_data(client, PMBUS_MFR_MODEL, block_buffer);
	if (ret < 0) {
		dev_err(&client->dev, "Failed to read Manufacturer Model\n");
		return ret;
	}
	for (mid = adm1275_id; mid->name[0]; mid++) {
		if (!strncasecmp(mid->name, block_buffer, strlen(mid->name)))
			break;
	}
	if (!mid->name[0]) {
		dev_err(&client->dev, "Unsupported device\n");
		return -ENODEV;
	}

	if (id->driver_data != mid->driver_data)
		dev_notice(&client->dev,
			   "Device mismatch: Configured %s, detected %s\n",
			   id->name, mid->name);

	config = i2c_smbus_read_byte_data(client, ADM1275_PMON_CONFIG);
	if (config < 0)
		return config;

	device_config = i2c_smbus_read_byte_data(client, ADM1275_DEVICE_CONFIG);
	if (device_config < 0)
		return device_config;

	data = devm_kzalloc(&client->dev, sizeof(struct adm1275_data),
			    GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	data->id = mid->driver_data;

	info = &data->info;

	info->pages = 1;
	info->format[PSC_VOLTAGE_IN] = direct;
	info->format[PSC_VOLTAGE_OUT] = direct;
	info->format[PSC_CURRENT_OUT] = direct;
	info->m[PSC_CURRENT_OUT] = 807;
	info->b[PSC_CURRENT_OUT] = 20475;
	info->R[PSC_CURRENT_OUT] = -1;
	info->func[0] = PMBUS_HAVE_IOUT | PMBUS_HAVE_STATUS_IOUT;

	info->read_word_data = adm1275_read_word_data;
	info->read_byte_data = adm1275_read_byte_data;
	info->write_word_data = adm1275_write_word_data;

	if (data->id == adm1075) {
		info->m[PSC_VOLTAGE_IN] = 27169;
		info->b[PSC_VOLTAGE_IN] = 0;
		info->R[PSC_VOLTAGE_IN] = -1;
		info->m[PSC_VOLTAGE_OUT] = 27169;
		info->b[PSC_VOLTAGE_OUT] = 0;
		info->R[PSC_VOLTAGE_OUT] = -1;
	} else if (config & ADM1275_VRANGE) {
		info->m[PSC_VOLTAGE_IN] = 19199;
		info->b[PSC_VOLTAGE_IN] = 0;
		info->R[PSC_VOLTAGE_IN] = -2;
		info->m[PSC_VOLTAGE_OUT] = 19199;
		info->b[PSC_VOLTAGE_OUT] = 0;
		info->R[PSC_VOLTAGE_OUT] = -2;
	} else {
		info->m[PSC_VOLTAGE_IN] = 6720;
		info->b[PSC_VOLTAGE_IN] = 0;
		info->R[PSC_VOLTAGE_IN] = -1;
		info->m[PSC_VOLTAGE_OUT] = 6720;
		info->b[PSC_VOLTAGE_OUT] = 0;
		info->R[PSC_VOLTAGE_OUT] = -1;
	}

	if (device_config & ADM1275_IOUT_WARN2_SELECT)
		data->have_oc_fault = true;

	switch (data->id) {
	case adm1075:
		info->format[PSC_POWER] = direct;
		info->b[PSC_POWER] = 0;
		info->R[PSC_POWER] = -1;
		switch (config & ADM1075_IRANGE_MASK) {
		case ADM1075_IRANGE_25:
			info->m[PSC_POWER] = 8549;
			info->m[PSC_CURRENT_OUT] = 806;
			break;
		case ADM1075_IRANGE_50:
			info->m[PSC_POWER] = 4279;
			info->m[PSC_CURRENT_OUT] = 404;
			break;
		default:
			dev_err(&client->dev, "Invalid input current range");
			info->m[PSC_POWER] = 0;
			info->m[PSC_CURRENT_OUT] = 0;
			break;
		}
		info->func[0] |= PMBUS_HAVE_VIN | PMBUS_HAVE_PIN
		  | PMBUS_HAVE_STATUS_INPUT;
		if (config & ADM1275_VIN_VOUT_SELECT)
			info->func[0] |=
			  PMBUS_HAVE_VOUT | PMBUS_HAVE_STATUS_VOUT;
		break;
	case adm1275:
		if (config & ADM1275_VIN_VOUT_SELECT)
			info->func[0] |=
			  PMBUS_HAVE_VOUT | PMBUS_HAVE_STATUS_VOUT;
		else
			info->func[0] |=
			  PMBUS_HAVE_VIN | PMBUS_HAVE_STATUS_INPUT;
		break;
	case adm1276:
		info->format[PSC_POWER] = direct;
		info->func[0] |= PMBUS_HAVE_VIN | PMBUS_HAVE_PIN
		  | PMBUS_HAVE_STATUS_INPUT;
		if (config & ADM1275_VIN_VOUT_SELECT)
			info->func[0] |=
			  PMBUS_HAVE_VOUT | PMBUS_HAVE_STATUS_VOUT;
		if (config & ADM1275_VRANGE) {
			info->m[PSC_POWER] = 6043;
			info->b[PSC_POWER] = 0;
			info->R[PSC_POWER] = -2;
		} else {
			info->m[PSC_POWER] = 2115;
			info->b[PSC_POWER] = 0;
			info->R[PSC_POWER] = -1;
		}
		break;
	}

	return pmbus_do_probe(client, id, info);
}

static struct i2c_driver adm1275_driver = {
	.driver = {
		   .name = "adm1275",
		   },
	.probe = adm1275_probe,
	.remove = pmbus_do_remove,
	.id_table = adm1275_id,
};

module_i2c_driver(adm1275_driver);

MODULE_AUTHOR("Guenter Roeck");
MODULE_DESCRIPTION("PMBus driver for Analog Devices ADM1275 and compatibles");
MODULE_LICENSE("GPL");
