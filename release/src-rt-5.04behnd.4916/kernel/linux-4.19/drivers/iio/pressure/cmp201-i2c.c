#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/acpi.h>
#include <linux/of.h>
#include <linux/regmap.h>

#include "cmp201.h"

static int cmp201_i2c_probe(struct i2c_client *client,
			    const struct i2c_device_id *id)
{
	struct regmap *regmap;
	const struct regmap_config *regmap_config;

	switch (id->driver_data) {
	case CMP201_CHIP_ID:
		regmap_config = &cmp201_regmap_config;
		break;
	default:
		return -EINVAL;
	}

	regmap = devm_regmap_init_i2c(client, regmap_config);
	if (IS_ERR(regmap)) {
		dev_err(&client->dev, "failed to allocate register map\n");
		return PTR_ERR(regmap);
	}

	return cmp201_common_probe(&client->dev,
				   regmap,
				   id->driver_data,
				   id->name,
				   client->irq);
}

static int cmp201_i2c_remove(struct i2c_client *client)
{
	return cmp201_common_remove(&client->dev);
}

static const struct acpi_device_id cmp201_acpi_i2c_match[] = {
	{"CMP201", CMP201_CHIP_ID },
	{ },
};
MODULE_DEVICE_TABLE(acpi, cmp201_acpi_i2c_match);

#ifdef CONFIG_OF
static const struct of_device_id cmp201_of_i2c_match[] = {
	{ .compatible = "cmc,cmp201", .data = (void *)CMP201_CHIP_ID },
	{ },
};
MODULE_DEVICE_TABLE(of, cmp201_of_i2c_match);
#else
#define cmp201_of_i2c_match NULL
#endif

static const struct i2c_device_id cmp201_i2c_id[] = {
	{"cmp201", CMP201_CHIP_ID },
	{ },
};
MODULE_DEVICE_TABLE(i2c, cmp201_i2c_id);

static struct i2c_driver cmp201_i2c_driver = {
	.driver = {
		.name	= "cmp201",
		.acpi_match_table = ACPI_PTR(cmp201_acpi_i2c_match),
		.of_match_table = of_match_ptr(cmp201_of_i2c_match),
		.pm = &cmp201_dev_pm_ops,
	},
	.probe		= cmp201_i2c_probe,
	.remove		= cmp201_i2c_remove,
	.id_table	= cmp201_i2c_id,
};
module_i2c_driver(cmp201_i2c_driver);

MODULE_AUTHOR("CoretronicMEMS");
MODULE_DESCRIPTION("Driver for CMC CMP201 pressure and temperature sensor");
MODULE_LICENSE("GPL v2");
