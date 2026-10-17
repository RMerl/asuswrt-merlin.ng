// SPDX-License-Identifier: GPL-2.0+
/*
 * Driver for Microchip USB251xB USB 2.0 Hi-Speed Hub Controller
 * Configuration via SMBus.
 *
 * Copyright (c) 2017 SKIDATA AG
 *
 * This work is based on the USB3503 driver by Dongjin Kim and
 * a not-accepted patch by Fabien Lahoudere, see:
 * https://patchwork.kernel.org/patch/9257715/
 */

#include <common.h>
#include <asm/gpio.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <errno.h>
#include <i2c.h>
#include <linux/delay.h>
#include <linux/utf.h>
#include <power/regulator.h>

/* Internal Register Set Addresses & Default Values acc. to DS00001692C */
#define USB251XB_ADDR_VENDOR_ID_LSB	0x00
#define USB251XB_ADDR_VENDOR_ID_MSB	0x01
#define USB251XB_DEF_VENDOR_ID		0x0424

#define USB251XB_ADDR_PRODUCT_ID_LSB	0x02
#define USB251XB_ADDR_PRODUCT_ID_MSB	0x03

#define USB251XB_ADDR_DEVICE_ID_LSB	0x04
#define USB251XB_ADDR_DEVICE_ID_MSB	0x05
#define USB251XB_DEF_DEVICE_ID		0x0BB3

#define USB251XB_ADDR_CONFIG_DATA_1	0x06
#define USB251XB_DEF_CONFIG_DATA_1	0x9B
#define USB251XB_ADDR_CONFIG_DATA_2	0x07
#define USB251XB_DEF_CONFIG_DATA_2	0x20
#define USB251XB_ADDR_CONFIG_DATA_3	0x08
#define USB251XB_DEF_CONFIG_DATA_3	0x02

#define USB251XB_ADDR_NON_REMOVABLE_DEVICES	0x09
#define USB251XB_DEF_NON_REMOVABLE_DEVICES	0x00

#define USB251XB_ADDR_PORT_DISABLE_SELF	0x0A
#define USB251XB_DEF_PORT_DISABLE_SELF	0x00
#define USB251XB_ADDR_PORT_DISABLE_BUS	0x0B
#define USB251XB_DEF_PORT_DISABLE_BUS	0x00

#define USB251XB_ADDR_MAX_POWER_SELF	0x0C
#define USB251XB_DEF_MAX_POWER_SELF	0x01
#define USB251XB_ADDR_MAX_POWER_BUS	0x0D
#define USB251XB_DEF_MAX_POWER_BUS	0x32

#define USB251XB_ADDR_MAX_CURRENT_SELF	0x0E
#define USB251XB_DEF_MAX_CURRENT_SELF	0x01
#define USB251XB_ADDR_MAX_CURRENT_BUS	0x0F
#define USB251XB_DEF_MAX_CURRENT_BUS	0x32

#define USB251XB_ADDR_POWER_ON_TIME	0x10
#define USB251XB_DEF_POWER_ON_TIME	0x32

#define USB251XB_ADDR_LANGUAGE_ID_HIGH	0x11
#define USB251XB_ADDR_LANGUAGE_ID_LOW	0x12
#define USB251XB_DEF_LANGUAGE_ID	0x0000

#define USB251XB_STRING_BUFSIZE			62
#define USB251XB_ADDR_MANUFACTURER_STRING_LEN	0x13
#define USB251XB_ADDR_MANUFACTURER_STRING	0x16
#define USB251XB_DEF_MANUFACTURER_STRING	"Microchip"

#define USB251XB_ADDR_PRODUCT_STRING_LEN	0x14
#define USB251XB_ADDR_PRODUCT_STRING		0x54

#define USB251XB_ADDR_SERIAL_STRING_LEN		0x15
#define USB251XB_ADDR_SERIAL_STRING		0x92
#define USB251XB_DEF_SERIAL_STRING		""

#define USB251XB_ADDR_BATTERY_CHARGING_ENABLE	0xD0
#define USB251XB_DEF_BATTERY_CHARGING_ENABLE	0x00

#define USB251XB_ADDR_BOOST_UP	0xF6
#define USB251XB_DEF_BOOST_UP	0x00
#define USB251XB_ADDR_BOOST_57	0xF7
#define USB251XB_DEF_BOOST_57	0x00
#define USB251XB_ADDR_BOOST_14	0xF8
#define USB251XB_DEF_BOOST_14	0x00

#define USB251XB_ADDR_PORT_SWAP	0xFA
#define USB251XB_DEF_PORT_SWAP	0x00

#define USB251XB_ADDR_PORT_MAP_12	0xFB
#define USB251XB_DEF_PORT_MAP_12	0x00
#define USB251XB_ADDR_PORT_MAP_34	0xFC
#define USB251XB_DEF_PORT_MAP_34	0x00 /* USB251{3B/i,4B/i,7/i} only */
#define USB251XB_ADDR_PORT_MAP_56	0xFD
#define USB251XB_DEF_PORT_MAP_56	0x00 /* USB2517/i only */
#define USB251XB_ADDR_PORT_MAP_7	0xFE
#define USB251XB_DEF_PORT_MAP_7		0x00 /* USB2517/i only */

#define USB251XB_ADDR_STATUS_COMMAND		0xFF
#define USB251XB_STATUS_COMMAND_SMBUS_DOWN	0x04
#define USB251XB_STATUS_COMMAND_RESET		0x02
#define USB251XB_STATUS_COMMAND_ATTACH		0x01

#define USB251XB_I2C_REG_SZ	0x100
#define USB251XB_I2C_WRITE_SZ	0x10

#define DRIVER_NAME	"usb251xb"
#define DRIVER_DESC	"Microchip USB 2.0 Hi-Speed Hub Controller"

struct usb251xb {
	struct device *dev;
	struct i2c_client *i2c;
	struct udevice *vdd;
	u8 skip_config;
	struct gpio_desc gpio_reset;
	u32 vendor_id;
	u32 product_id;
	u32 device_id;
	u8  conf_data1;
	u8  conf_data2;
	u8  conf_data3;
	u8  non_rem_dev;
	u8  port_disable_sp;
	u8  port_disable_bp;
	u8  max_power_sp;
	u8  max_power_bp;
	u8  max_current_sp;
	u8  max_current_bp;
	u8  power_on_time;
	u32 lang_id;
	u8 manufacturer_len;
	u8 product_len;
	u8 serial_len;
	s16 manufacturer[USB251XB_STRING_BUFSIZE];
	s16 product[USB251XB_STRING_BUFSIZE];
	s16 serial[USB251XB_STRING_BUFSIZE];
	u8  bat_charge_en;
	u32 boost_up;
	u8  boost_57;
	u8  boost_14;
	u8  port_swap;
	u8  port_map12;
	u8  port_map34;
	u8  port_map56;
	u8  port_map7;
	u8  status;
};

struct usb251xb_data {
	u16 product_id;
	u8 port_cnt;
	bool led_support;
	bool bat_support;
	char product_str[USB251XB_STRING_BUFSIZE / 2]; /* ASCII string */
};

static const struct usb251xb_data usb2422_data = {
	.product_id = 0x2422,
	.port_cnt = 2,
	.led_support = false,
	.bat_support = true,
	.product_str = "USB2422",
};

static const struct usb251xb_data usb2512b_data = {
	.product_id = 0x2512,
	.port_cnt = 2,
	.led_support = false,
	.bat_support = true,
	.product_str = "USB2512B",
};

static const struct usb251xb_data usb2512bi_data = {
	.product_id = 0x2512,
	.port_cnt = 2,
	.led_support = false,
	.bat_support = true,
	.product_str = "USB2512Bi",
};

static const struct usb251xb_data usb2513b_data = {
	.product_id = 0x2513,
	.port_cnt = 3,
	.led_support = false,
	.bat_support = true,
	.product_str = "USB2513B",
};

static const struct usb251xb_data usb2513bi_data = {
	.product_id = 0x2513,
	.port_cnt = 3,
	.led_support = false,
	.bat_support = true,
	.product_str = "USB2513Bi",
};

static const struct usb251xb_data usb2514b_data = {
	.product_id = 0x2514,
	.port_cnt = 4,
	.led_support = false,
	.bat_support = true,
	.product_str = "USB2514B",
};

static const struct usb251xb_data usb2514bi_data = {
	.product_id = 0x2514,
	.port_cnt = 4,
	.led_support = false,
	.bat_support = true,
	.product_str = "USB2514Bi",
};

static const struct usb251xb_data usb2517_data = {
	.product_id = 0x2517,
	.port_cnt = 7,
	.led_support = true,
	.bat_support = false,
	.product_str = "USB2517",
};

static const struct usb251xb_data usb2517i_data = {
	.product_id = 0x2517,
	.port_cnt = 7,
	.led_support = true,
	.bat_support = false,
	.product_str = "USB2517i",
};

static void usb251xb_reset(struct usb251xb *hub)
{
	dm_gpio_set_value(&hub->gpio_reset, 1);
	udelay(10);	/* >=1us RESET_N asserted */
	dm_gpio_set_value(&hub->gpio_reset, 0);

	/* wait for hub recovery/stabilization */
	udelay(750);	/* >=500us after RESET_N deasserted */
}

static int usb251xb_connect(struct udevice *dev)
{
	struct usb251xb *hub = dev_get_priv(dev);
	char i2c_wb[USB251XB_I2C_REG_SZ];
	int err, i;

	memset(i2c_wb, 0, USB251XB_I2C_REG_SZ);

	if (hub->skip_config) {
		dev_info(dev, "Skip hub configuration, only attach.\n");
		i2c_wb[0] = 0x01;
		i2c_wb[1] = USB251XB_STATUS_COMMAND_ATTACH;

		usb251xb_reset(hub);

		err = dm_i2c_write(dev, USB251XB_ADDR_STATUS_COMMAND, i2c_wb, 2);
		if (err) {
			dev_err(dev, "attaching hub failed: %d\n", err);
			return err;
		}
		return 0;
	}

	i2c_wb[USB251XB_ADDR_VENDOR_ID_MSB]     = (hub->vendor_id >> 8) & 0xFF;
	i2c_wb[USB251XB_ADDR_VENDOR_ID_LSB]     = hub->vendor_id & 0xFF;
	i2c_wb[USB251XB_ADDR_PRODUCT_ID_MSB]    = (hub->product_id >> 8) & 0xFF;
	i2c_wb[USB251XB_ADDR_PRODUCT_ID_LSB]    = hub->product_id & 0xFF;
	i2c_wb[USB251XB_ADDR_DEVICE_ID_MSB]     = (hub->device_id >> 8) & 0xFF;
	i2c_wb[USB251XB_ADDR_DEVICE_ID_LSB]     = hub->device_id & 0xFF;
	i2c_wb[USB251XB_ADDR_CONFIG_DATA_1]     = hub->conf_data1;
	i2c_wb[USB251XB_ADDR_CONFIG_DATA_2]     = hub->conf_data2;
	i2c_wb[USB251XB_ADDR_CONFIG_DATA_3]     = hub->conf_data3;
	i2c_wb[USB251XB_ADDR_NON_REMOVABLE_DEVICES] = hub->non_rem_dev;
	i2c_wb[USB251XB_ADDR_PORT_DISABLE_SELF] = hub->port_disable_sp;
	i2c_wb[USB251XB_ADDR_PORT_DISABLE_BUS]  = hub->port_disable_bp;
	i2c_wb[USB251XB_ADDR_MAX_POWER_SELF]    = hub->max_power_sp;
	i2c_wb[USB251XB_ADDR_MAX_POWER_BUS]     = hub->max_power_bp;
	i2c_wb[USB251XB_ADDR_MAX_CURRENT_SELF]  = hub->max_current_sp;
	i2c_wb[USB251XB_ADDR_MAX_CURRENT_BUS]   = hub->max_current_bp;
	i2c_wb[USB251XB_ADDR_POWER_ON_TIME]     = hub->power_on_time;
	i2c_wb[USB251XB_ADDR_LANGUAGE_ID_HIGH]  = (hub->lang_id >> 8) & 0xFF;
	i2c_wb[USB251XB_ADDR_LANGUAGE_ID_LOW]   = hub->lang_id & 0xFF;
	i2c_wb[USB251XB_ADDR_MANUFACTURER_STRING_LEN] = hub->manufacturer_len;
	i2c_wb[USB251XB_ADDR_PRODUCT_STRING_LEN]      = hub->product_len;
	i2c_wb[USB251XB_ADDR_SERIAL_STRING_LEN]       = hub->serial_len;
	memcpy(&i2c_wb[USB251XB_ADDR_MANUFACTURER_STRING], hub->manufacturer,
	       USB251XB_STRING_BUFSIZE);
	memcpy(&i2c_wb[USB251XB_ADDR_SERIAL_STRING], hub->serial,
	       USB251XB_STRING_BUFSIZE);
	memcpy(&i2c_wb[USB251XB_ADDR_PRODUCT_STRING], hub->product,
	       USB251XB_STRING_BUFSIZE);
	i2c_wb[USB251XB_ADDR_BATTERY_CHARGING_ENABLE] = hub->bat_charge_en;
	i2c_wb[USB251XB_ADDR_BOOST_UP]          = hub->boost_up;
	i2c_wb[USB251XB_ADDR_BOOST_57]          = hub->boost_57;
	i2c_wb[USB251XB_ADDR_BOOST_14]          = hub->boost_14;
	i2c_wb[USB251XB_ADDR_PORT_SWAP]         = hub->port_swap;
	i2c_wb[USB251XB_ADDR_PORT_MAP_12]       = hub->port_map12;
	i2c_wb[USB251XB_ADDR_PORT_MAP_34]       = hub->port_map34;
	i2c_wb[USB251XB_ADDR_PORT_MAP_56]       = hub->port_map56;
	i2c_wb[USB251XB_ADDR_PORT_MAP_7]        = hub->port_map7;
	i2c_wb[USB251XB_ADDR_STATUS_COMMAND] = USB251XB_STATUS_COMMAND_ATTACH;

	usb251xb_reset(hub);

	/* write registers */
	for (i = 0; i < (USB251XB_I2C_REG_SZ / USB251XB_I2C_WRITE_SZ); i++) {
		int offset = i * USB251XB_I2C_WRITE_SZ;
		char wbuf[USB251XB_I2C_WRITE_SZ + 1];

		/* The first data byte transferred tells the hub how many data
		 * bytes will follow (byte count).
		 */
		wbuf[0] = USB251XB_I2C_WRITE_SZ;
		memcpy(&wbuf[1], &i2c_wb[offset], USB251XB_I2C_WRITE_SZ);

		dev_dbg(dev, "writing %d byte block %d to 0x%02X\n",
			USB251XB_I2C_WRITE_SZ, i, offset);

		err = dm_i2c_write(dev, offset, wbuf, USB251XB_I2C_WRITE_SZ + 1);
		if (err)
			goto out_err;
	}

	dev_info(dev, "Hub configuration was successful.\n");
	return 0;

out_err:
	dev_err(dev, "configuring block %d failed: %d\n", i, err);
	return err;
}

static int usb251xb_probe(struct udevice *dev)
{
	struct usb251xb *hub = dev_get_priv(dev);
	int err;

	if (CONFIG_IS_ENABLED(DM_REGULATOR) && hub->vdd) {
		err = regulator_set_enable(hub->vdd, true);
		if (err)
			return err;
	}

	err = usb251xb_connect(dev);
	if (err) {
		dev_err(dev, "Failed to connect hub (%d)\n", err);
		return err;
	}

	dev_info(dev, "Hub probed successfully\n");

	return 0;
}

static void usb251xb_get_ports_field(struct udevice *dev,
				     const char *prop_name, u8 port_cnt,
				     bool ds_only, u8 *fld)
{
	u32 i, port;
	int ret;

	for (i = 0; i < port_cnt; i++) {
		ret = dev_read_u32_index(dev, prop_name, i, &port);
		if (ret)
			continue;
		if (port >= ds_only ? 1 : 0 && port <= port_cnt)
			*fld |= BIT(port);
		else
			dev_warn(dev, "port %u doesn't exist\n", port);
	}
}

static int usb251xb_of_to_plat(struct udevice *dev)
{
	struct usb251xb_data *data =
		(struct usb251xb_data *)dev_get_driver_data(dev);
	struct usb251xb *hub = dev_get_priv(dev);
	char str[USB251XB_STRING_BUFSIZE / 2];
	const char *cproperty_char;
	u32 property_u32 = 0;
	int len, err;

	if (dev_read_bool(dev, "skip-config"))
		hub->skip_config = 1;
	else
		hub->skip_config = 0;

	err = gpio_request_by_name(dev, "reset-gpios", 0, &hub->gpio_reset,
				   GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE);
	if (err && err != -ENOENT) {
		dev_err(dev, "unable to request GPIO reset pin (%d)\n", err);
		return err;
	}

	if (CONFIG_IS_ENABLED(DM_REGULATOR)) {
		err = device_get_supply_regulator(dev, "vdd-supply",
						  &hub->vdd);
		if (err && err != -ENOENT) {
			dev_err(dev, "Warning: cannot get power supply\n");
			return err;
		}
	}

	hub->vendor_id = dev_read_u16_default(dev, "vendor-id",
					      USB251XB_DEF_VENDOR_ID);

	hub->product_id = dev_read_u16_default(dev, "product-id",
					       data->product_id);

	hub->device_id = dev_read_u16_default(dev, "device-id",
					      USB251XB_DEF_DEVICE_ID);

	hub->conf_data1 = USB251XB_DEF_CONFIG_DATA_1;
	if (dev_read_bool(dev, "self-powered")) {
		hub->conf_data1 |= BIT(7);

		/* Configure Over-Current sens when self-powered */
		hub->conf_data1 &= ~BIT(2);
		if (dev_read_bool(dev, "ganged-sensing"))
			hub->conf_data1 &= ~BIT(1);
		else if (dev_read_bool(dev, "individual-sensing"))
			hub->conf_data1 |= BIT(1);
	} else if (dev_read_bool(dev, "bus-powered")) {
		hub->conf_data1 &= ~BIT(7);

		/* Disable Over-Current sense when bus-powered */
		hub->conf_data1 |= BIT(2);
	}

	if (dev_read_bool(dev, "disable-hi-speed"))
		hub->conf_data1 |= BIT(5);

	if (dev_read_bool(dev, "multi-tt"))
		hub->conf_data1 |= BIT(4);
	else if (dev_read_bool(dev, "single-tt"))
		hub->conf_data1 &= ~BIT(4);

	if (dev_read_bool(dev, "disable-eop"))
		hub->conf_data1 |= BIT(3);

	if (dev_read_bool(dev, "individual-port-switching"))
		hub->conf_data1 |= BIT(0);
	else if (dev_read_bool(dev, "ganged-port-switching"))
		hub->conf_data1 &= ~BIT(0);

	hub->conf_data2 = USB251XB_DEF_CONFIG_DATA_2;
	if (dev_read_bool(dev, "dynamic-power-switching"))
		hub->conf_data2 |= BIT(7);

	if (!dev_read_u32(dev, "oc-delay-us", &property_u32)) {
		if (property_u32 == 100) {
			/* 100 us*/
			hub->conf_data2 &= ~BIT(5);
			hub->conf_data2 &= ~BIT(4);
		} else if (property_u32 == 4000) {
			/* 4 ms */
			hub->conf_data2 &= ~BIT(5);
			hub->conf_data2 |= BIT(4);
		} else if (property_u32 == 16000) {
			/* 16 ms */
			hub->conf_data2 |= BIT(5);
			hub->conf_data2 |= BIT(4);
		} else {
			/* 8 ms (DEFAULT) */
			hub->conf_data2 |= BIT(5);
			hub->conf_data2 &= ~BIT(4);
		}
	}

	if (dev_read_bool(dev, "compound-device"))
		hub->conf_data2 |= BIT(3);

	hub->conf_data3 = USB251XB_DEF_CONFIG_DATA_3;
	if (dev_read_bool(dev, "port-mapping-mode"))
		hub->conf_data3 |= BIT(3);

	if (data->led_support && dev_read_bool(dev, "led-usb-mode"))
		hub->conf_data3 &= ~BIT(1);

	if (dev_read_bool(dev, "string-support"))
		hub->conf_data3 |= BIT(0);

	hub->non_rem_dev = USB251XB_DEF_NON_REMOVABLE_DEVICES;
	usb251xb_get_ports_field(dev, "non-removable-ports", data->port_cnt,
				 true, &hub->non_rem_dev);

	hub->port_disable_sp = USB251XB_DEF_PORT_DISABLE_SELF;
	usb251xb_get_ports_field(dev, "sp-disabled-ports", data->port_cnt,
				 true, &hub->port_disable_sp);

	hub->port_disable_bp = USB251XB_DEF_PORT_DISABLE_BUS;
	usb251xb_get_ports_field(dev, "bp-disabled-ports", data->port_cnt,
				 true, &hub->port_disable_bp);

	hub->max_power_sp = USB251XB_DEF_MAX_POWER_SELF;
	if (!dev_read_u32(dev, "sp-max-total-current-microamp", &property_u32))
		hub->max_power_sp = min_t(u8, property_u32 / 2000, 50);

	hub->max_power_bp = USB251XB_DEF_MAX_POWER_BUS;
	if (!dev_read_u32(dev, "bp-max-total-current-microamp", &property_u32))
		hub->max_power_bp = min_t(u8, property_u32 / 2000, 255);

	hub->max_current_sp = USB251XB_DEF_MAX_CURRENT_SELF;
	if (!dev_read_u32(dev, "sp-max-removable-current-microamp",
			  &property_u32))
		hub->max_current_sp = min_t(u8, property_u32 / 2000, 50);

	hub->max_current_bp = USB251XB_DEF_MAX_CURRENT_BUS;
	if (!dev_read_u32(dev, "bp-max-removable-current-microamp",
			  &property_u32))
		hub->max_current_bp = min_t(u8, property_u32 / 2000, 255);

	hub->power_on_time = USB251XB_DEF_POWER_ON_TIME;
	if (!dev_read_u32(dev, "power-on-time-ms", &property_u32))
		hub->power_on_time = min_t(u8, property_u32 / 2, 255);

	hub->lang_id = dev_read_u16_default(dev, "language-id",
					    USB251XB_DEF_LANGUAGE_ID);

	hub->boost_up = dev_read_u8_default(dev, "boost-up",
					    USB251XB_DEF_BOOST_UP);

	cproperty_char = dev_read_string(dev, "manufacturer");
	strlcpy(str, cproperty_char ? : USB251XB_DEF_MANUFACTURER_STRING,
		sizeof(str));
	hub->manufacturer_len = strlen(str) & 0xFF;
	memset(hub->manufacturer, 0, USB251XB_STRING_BUFSIZE);
	len = min_t(size_t, USB251XB_STRING_BUFSIZE / 2, strlen(str));
	len = utf8_to_utf16le(str, hub->manufacturer, len);

	cproperty_char = dev_read_string(dev, "product");
	strlcpy(str, cproperty_char ? : data->product_str, sizeof(str));
	hub->product_len = strlen(str) & 0xFF;
	memset(hub->product, 0, USB251XB_STRING_BUFSIZE);
	len = min_t(size_t, USB251XB_STRING_BUFSIZE / 2, strlen(str));
	len = utf8_to_utf16le(str, hub->product, len);

	cproperty_char = dev_read_string(dev, "serial");
	strlcpy(str, cproperty_char ? : USB251XB_DEF_SERIAL_STRING,
		sizeof(str));
	hub->serial_len = strlen(str) & 0xFF;
	memset(hub->serial, 0, USB251XB_STRING_BUFSIZE);
	len = min_t(size_t, USB251XB_STRING_BUFSIZE / 2, strlen(str));
	len = utf8_to_utf16le(str, hub->serial, len);

	/*
	 * The datasheet documents the register as 'Port Swap' but in real the
	 * register controls the USB DP/DM signal swapping for each port.
	 */
	hub->port_swap = USB251XB_DEF_PORT_SWAP;
	usb251xb_get_ports_field(dev, "swap-dx-lanes", data->port_cnt,
				 false, &hub->port_swap);

	/* The following parameters are currently not exposed to devicetree, but
	 * may be as soon as needed.
	 */
	hub->bat_charge_en = USB251XB_DEF_BATTERY_CHARGING_ENABLE;
	hub->boost_57 = USB251XB_DEF_BOOST_57;
	hub->boost_14 = USB251XB_DEF_BOOST_14;
	hub->port_map12 = USB251XB_DEF_PORT_MAP_12;
	hub->port_map34 = USB251XB_DEF_PORT_MAP_34;
	hub->port_map56 = USB251XB_DEF_PORT_MAP_56;
	hub->port_map7  = USB251XB_DEF_PORT_MAP_7;

	return 0;
}

static const struct udevice_id usb251xb_of_match[] = {
	{
		.compatible = "microchip,usb2422",
		.data = (ulong)&usb2422_data,
	}, {
		.compatible = "microchip,usb2512b",
		.data = (ulong)&usb2512b_data,
	}, {
		.compatible = "microchip,usb2512bi",
		.data = (ulong)&usb2512bi_data,
	}, {
		.compatible = "microchip,usb2513b",
		.data = (ulong)&usb2513b_data,
	}, {
		.compatible = "microchip,usb2513bi",
		.data = (ulong)&usb2513bi_data,
	}, {
		.compatible = "microchip,usb2514b",
		.data = (ulong)&usb2514b_data,
	}, {
		.compatible = "microchip,usb2514bi",
		.data = (ulong)&usb2514bi_data,
	}, {
		.compatible = "microchip,usb2517",
		.data = (ulong)&usb2517_data,
	}, {
		.compatible = "microchip,usb2517i",
		.data = (ulong)&usb2517i_data,
	}
};

U_BOOT_DRIVER(usb251xb) = {
	.name		= "usb251xb",
	.id		= UCLASS_MISC,
	.of_match	= usb251xb_of_match,
	.of_to_plat	= usb251xb_of_to_plat,
	.probe		= usb251xb_probe,
	.priv_auto	= sizeof(struct usb251xb),
};
