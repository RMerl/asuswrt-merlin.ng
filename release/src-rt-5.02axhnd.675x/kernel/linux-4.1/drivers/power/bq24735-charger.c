/*
 * Battery charger driver for TI BQ24735
 *
 * Copyright (c) 2013, NVIDIA CORPORATION.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/power_supply.h>
#include <linux/slab.h>

#include <linux/power/bq24735-charger.h>

#define BQ24735_CHG_OPT			0x12
#define BQ24735_CHG_OPT_CHARGE_DISABLE	(1 << 0)
#define BQ24735_CHG_OPT_AC_PRESENT	(1 << 4)
#define BQ24735_CHARGE_CURRENT		0x14
#define BQ24735_CHARGE_CURRENT_MASK	0x1fc0
#define BQ24735_CHARGE_VOLTAGE		0x15
#define BQ24735_CHARGE_VOLTAGE_MASK	0x7ff0
#define BQ24735_INPUT_CURRENT		0x3f
#define BQ24735_INPUT_CURRENT_MASK	0x1f80
#define BQ24735_MANUFACTURER_ID		0xfe
#define BQ24735_DEVICE_ID		0xff

struct bq24735 {
	struct power_supply		*charger;
	struct power_supply_desc	charger_desc;
	struct i2c_client		*client;
	struct bq24735_platform		*pdata;
};

static inline struct bq24735 *to_bq24735(struct power_supply *psy)
{
	return power_supply_get_drvdata(psy);
}

static enum power_supply_property bq24735_charger_properties[] = {
	POWER_SUPPLY_PROP_ONLINE,
};

static inline int bq24735_write_word(struct i2c_client *client, u8 reg,
				     u16 value)
{
	return i2c_smbus_write_word_data(client, reg, le16_to_cpu(value));
}

static inline int bq24735_read_word(struct i2c_client *client, u8 reg)
{
	s32 ret = i2c_smbus_read_word_data(client, reg);

	return ret < 0 ? ret : le16_to_cpu(ret);
}

static int bq24735_update_word(struct i2c_client *client, u8 reg,
			       u16 mask, u16 value)
{
	unsigned int tmp;
	int ret;

	ret = bq24735_read_word(client, reg);
	if (ret < 0)
		return ret;

	tmp = ret & ~mask;
	tmp |= value & mask;

	return bq24735_write_word(client, reg, tmp);
}

static inline int bq24735_enable_charging(struct bq24735 *charger)
{
	return bq24735_update_word(charger->client, BQ24735_CHG_OPT,
				   BQ24735_CHG_OPT_CHARGE_DISABLE,
				   ~BQ24735_CHG_OPT_CHARGE_DISABLE);
}

static inline int bq24735_disable_charging(struct bq24735 *charger)
{
	return bq24735_update_word(charger->client, BQ24735_CHG_OPT,
				   BQ24735_CHG_OPT_CHARGE_DISABLE,
				   BQ24735_CHG_OPT_CHARGE_DISABLE);
}

static int bq24735_config_charger(struct bq24735 *charger)
{
	struct bq24735_platform *pdata = charger->pdata;
	int ret;
	u16 value;

	if (pdata->charge_current) {
		value = pdata->charge_current & BQ24735_CHARGE_CURRENT_MASK;

		ret = bq24735_write_word(charger->client,
					 BQ24735_CHARGE_CURRENT, value);
		if (ret < 0) {
			dev_err(&charger->client->dev,
				"Failed to write charger current : %d\n",
				ret);
			return ret;
		}
	}

	if (pdata->charge_voltage) {
		value = pdata->charge_voltage & BQ24735_CHARGE_VOLTAGE_MASK;

		ret = bq24735_write_word(charger->client,
					 BQ24735_CHARGE_VOLTAGE, value);
		if (ret < 0) {
			dev_err(&charger->client->dev,
				"Failed to write charger voltage : %d\n",
				ret);
			return ret;
		}
	}

	if (pdata->input_current) {
		value = pdata->input_current & BQ24735_INPUT_CURRENT_MASK;

		ret = bq24735_write_word(charger->client,
					 BQ24735_INPUT_CURRENT, value);
		if (ret < 0) {
			dev_err(&charger->client->dev,
				"Failed to write input current : %d\n",
				ret);
			return ret;
		}
	}

	return 0;
}

static bool bq24735_charger_is_present(struct bq24735 *charger)
{
	struct bq24735_platform *pdata = charger->pdata;
	int ret;

	if (pdata->status_gpio_valid) {
		ret = gpio_get_value_cansleep(pdata->status_gpio);
		return ret ^= pdata->status_gpio_active_low == 0;
	} else {
		int ac = 0;

		ac = bq24735_read_word(charger->client, BQ24735_CHG_OPT);
		if (ac < 0) {
			dev_err(&charger->client->dev,
				"Failed to read charger options : %d\n",
				ac);
			return false;
		}
		return (ac & BQ24735_CHG_OPT_AC_PRESENT) ? true : false;
	}

	return false;
}

static irqreturn_t bq24735_charger_isr(int irq, void *devid)
{
	struct power_supply *psy = devid;
	struct bq24735 *charger = to_bq24735(psy);

	if (bq24735_charger_is_present(charger))
		bq24735_enable_charging(charger);
	else
		bq24735_disable_charging(charger);

	power_supply_changed(psy);

	return IRQ_HANDLED;
}

static int bq24735_charger_get_property(struct power_supply *psy,
					enum power_supply_property psp,
					union power_supply_propval *val)
{
	struct bq24735 *charger = to_bq24735(psy);

	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = bq24735_charger_is_present(charger) ? 1 : 0;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static struct bq24735_platform *bq24735_parse_dt_data(struct i2c_client *client)
{
	struct bq24735_platform *pdata;
	struct device_node *np = client->dev.of_node;
	u32 val;
	int ret;
	enum of_gpio_flags flags;

	pdata = devm_kzalloc(&client->dev, sizeof(*pdata), GFP_KERNEL);
	if (!pdata) {
		dev_err(&client->dev,
			"Memory alloc for bq24735 pdata failed\n");
		return NULL;
	}

	pdata->status_gpio = of_get_named_gpio_flags(np, "ti,ac-detect-gpios",
						     0, &flags);

	if (flags & OF_GPIO_ACTIVE_LOW)
		pdata->status_gpio_active_low = 1;

	ret = of_property_read_u32(np, "ti,charge-current", &val);
	if (!ret)
		pdata->charge_current = val;

	ret = of_property_read_u32(np, "ti,charge-voltage", &val);
	if (!ret)
		pdata->charge_voltage = val;

	ret = of_property_read_u32(np, "ti,input-current", &val);
	if (!ret)
		pdata->input_current = val;

	return pdata;
}

static int bq24735_charger_probe(struct i2c_client *client,
				 const struct i2c_device_id *id)
{
	int ret;
	struct bq24735 *charger;
	struct power_supply_desc *supply_desc;
	struct power_supply_config psy_cfg = {};
	char *name;

	charger = devm_kzalloc(&client->dev, sizeof(*charger), GFP_KERNEL);
	if (!charger)
		return -ENOMEM;

	charger->pdata = client->dev.platform_data;

	if (IS_ENABLED(CONFIG_OF) && !charger->pdata && client->dev.of_node)
		charger->pdata = bq24735_parse_dt_data(client);

	if (!charger->pdata) {
		dev_err(&client->dev, "no platform data provided\n");
		return -EINVAL;
	}

	name = (char *)charger->pdata->name;
	if (!name) {
		name = kasprintf(GFP_KERNEL, "bq24735@%s",
				 dev_name(&client->dev));
		if (!name) {
			dev_err(&client->dev, "Failed to alloc device name\n");
			return -ENOMEM;
		}
	}

	charger->client = client;

	supply_desc = &charger->charger_desc;

	supply_desc->name = name;
	supply_desc->type = POWER_SUPPLY_TYPE_MAINS;
	supply_desc->properties = bq24735_charger_properties;
	supply_desc->num_properties = ARRAY_SIZE(bq24735_charger_properties);
	supply_desc->get_property = bq24735_charger_get_property;

	psy_cfg.supplied_to = charger->pdata->supplied_to;
	psy_cfg.num_supplicants = charger->pdata->num_supplicants;
	psy_cfg.of_node = client->dev.of_node;
	psy_cfg.drv_data = charger;

	i2c_set_clientdata(client, charger);

	ret = bq24735_read_word(client, BQ24735_MANUFACTURER_ID);
	if (ret < 0) {
		dev_err(&client->dev, "Failed to read manufacturer id : %d\n",
			ret);
		goto err_free_name;
	} else if (ret != 0x0040) {
		dev_err(&client->dev,
			"manufacturer id mismatch. 0x0040 != 0x%04x\n", ret);
		ret = -ENODEV;
		goto err_free_name;
	}

	ret = bq24735_read_word(client, BQ24735_DEVICE_ID);
	if (ret < 0) {
		dev_err(&client->dev, "Failed to read device id : %d\n", ret);
		goto err_free_name;
	} else if (ret != 0x000B) {
		dev_err(&client->dev,
			"device id mismatch. 0x000b != 0x%04x\n", ret);
		ret = -ENODEV;
		goto err_free_name;
	}

	if (gpio_is_valid(charger->pdata->status_gpio)) {
		ret = devm_gpio_request(&client->dev,
					charger->pdata->status_gpio,
					name);
		if (ret) {
			dev_err(&client->dev,
				"Failed GPIO request for GPIO %d: %d\n",
				charger->pdata->status_gpio, ret);
		}

		charger->pdata->status_gpio_valid = !ret;
	}

	ret = bq24735_config_charger(charger);
	if (ret < 0) {
		dev_err(&client->dev, "failed in configuring charger");
		goto err_free_name;
	}

	/* check for AC adapter presence */
	if (bq24735_charger_is_present(charger)) {
		ret = bq24735_enable_charging(charger);
		if (ret < 0) {
			dev_err(&client->dev, "Failed to enable charging\n");
			goto err_free_name;
		}
	}

	charger->charger = power_supply_register(&client->dev, supply_desc,
						 &psy_cfg);
	if (IS_ERR(charger->charger)) {
		ret = PTR_ERR(charger->charger);
		dev_err(&client->dev, "Failed to register power supply: %d\n",
			ret);
		goto err_free_name;
	}

	if (client->irq) {
		ret = devm_request_threaded_irq(&client->dev, client->irq,
						NULL, bq24735_charger_isr,
						IRQF_TRIGGER_RISING |
						IRQF_TRIGGER_FALLING |
						IRQF_ONESHOT,
						supply_desc->name,
						charger->charger);
		if (ret) {
			dev_err(&client->dev,
				"Unable to register IRQ %d err %d\n",
				client->irq, ret);
			goto err_unregister_supply;
		}
	}

	return 0;
err_unregister_supply:
	power_supply_unregister(charger->charger);
err_free_name:
	if (name != charger->pdata->name)
		kfree(name);

	return ret;
}

static int bq24735_charger_remove(struct i2c_client *client)
{
	struct bq24735 *charger = i2c_get_clientdata(client);

	if (charger->client->irq)
		devm_free_irq(&charger->client->dev, charger->client->irq,
			      &charger->charger);

	power_supply_unregister(charger->charger);

	if (charger->charger_desc.name != charger->pdata->name)
		kfree(charger->charger_desc.name);

	return 0;
}

static const struct i2c_device_id bq24735_charger_id[] = {
	{ "bq24735-charger", 0 },
	{}
};
MODULE_DEVICE_TABLE(i2c, bq24735_charger_id);

static const struct of_device_id bq24735_match_ids[] = {
	{ .compatible = "ti,bq24735", },
	{ /* end */ }
};
MODULE_DEVICE_TABLE(of, bq24735_match_ids);

static struct i2c_driver bq24735_charger_driver = {
	.driver = {
		.name = "bq24735-charger",
		.owner = THIS_MODULE,
		.of_match_table = bq24735_match_ids,
	},
	.probe = bq24735_charger_probe,
	.remove = bq24735_charger_remove,
	.id_table = bq24735_charger_id,
};

module_i2c_driver(bq24735_charger_driver);

MODULE_DESCRIPTION("bq24735 battery charging driver");
MODULE_AUTHOR("Darbha Sriharsha <dsriharsha@nvidia.com>");
MODULE_LICENSE("GPL v2");
