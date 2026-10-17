// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020-2022 Linaro Limited
 */

#define LOG_CATEGORY UCLASS_REGULATOR

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <scmi_agent.h>
#include <scmi_protocols.h>
#include <asm/types.h>
#include <dm/device.h>
#include <dm/device_compat.h>
#include <dm/device-internal.h>
#include <linux/kernel.h>
#include <power/regulator.h>

/**
 * struct scmi_regulator_platdata - Platform data for a scmi voltage domain regulator
 * @domain_id: ID representing the regulator for the related SCMI agent
 */
struct scmi_regulator_platdata {
	u32 domain_id;
};

static int scmi_voltd_set_enable(struct udevice *dev, bool enable)
{
	struct scmi_regulator_platdata *pdata = dev_get_plat(dev);
	struct scmi_voltd_config_set_in in = {
		.domain_id = pdata->domain_id,
		.config = enable ? SCMI_VOLTD_CONFIG_ON : SCMI_VOLTD_CONFIG_OFF,
	};
	struct scmi_voltd_config_set_out out;
	struct scmi_msg msg = SCMI_MSG_IN(SCMI_PROTOCOL_ID_VOLTAGE_DOMAIN,
					  SCMI_VOLTAGE_DOMAIN_CONFIG_SET,
					  in, out);
	int ret;

	ret = devm_scmi_process_msg(dev, &msg);
	if (ret)
		return ret;

	return scmi_to_linux_errno(out.status);
}

static int scmi_voltd_get_enable(struct udevice *dev)
{
	struct scmi_regulator_platdata *pdata = dev_get_plat(dev);
	struct scmi_voltd_config_get_in in = {
		.domain_id = pdata->domain_id,
	};
	struct scmi_voltd_config_get_out out;
	struct scmi_msg msg = SCMI_MSG_IN(SCMI_PROTOCOL_ID_VOLTAGE_DOMAIN,
					  SCMI_VOLTAGE_DOMAIN_CONFIG_GET,
					  in, out);
	int ret;

	ret = devm_scmi_process_msg(dev, &msg);
	if (ret < 0)
		return ret;

	ret = scmi_to_linux_errno(out.status);
	if (ret < 0)
		return ret;

	return out.config == SCMI_VOLTD_CONFIG_ON;
}

static int scmi_voltd_set_voltage_level(struct udevice *dev, int uV)
{
	struct scmi_regulator_platdata *pdata = dev_get_plat(dev);
	struct scmi_voltd_level_set_in in = {
		.domain_id = pdata->domain_id,
		.voltage_level = uV,
	};
	struct scmi_voltd_level_set_out out;
	struct scmi_msg msg = SCMI_MSG_IN(SCMI_PROTOCOL_ID_VOLTAGE_DOMAIN,
					  SCMI_VOLTAGE_DOMAIN_LEVEL_SET,
					  in, out);
	int ret;

	ret = devm_scmi_process_msg(dev, &msg);
	if (ret < 0)
		return ret;

	return scmi_to_linux_errno(out.status);
}

static int scmi_voltd_get_voltage_level(struct udevice *dev)
{
	struct scmi_regulator_platdata *pdata = dev_get_plat(dev);
	struct scmi_voltd_level_get_in in = {
		.domain_id = pdata->domain_id,
	};
	struct scmi_voltd_level_get_out out;
	struct scmi_msg msg = SCMI_MSG_IN(SCMI_PROTOCOL_ID_VOLTAGE_DOMAIN,
					  SCMI_VOLTAGE_DOMAIN_LEVEL_GET,
					  in, out);
	int ret;

	ret = devm_scmi_process_msg(dev, &msg);
	if (ret < 0)
		return ret;

	ret = scmi_to_linux_errno(out.status);
	if (ret < 0)
		return ret;

	return out.voltage_level;
}

static int scmi_regulator_of_to_plat(struct udevice *dev)
{
	struct scmi_regulator_platdata *pdata = dev_get_plat(dev);
	fdt_addr_t reg;

	reg = dev_read_addr(dev);
	if (reg == FDT_ADDR_T_NONE)
		return -EINVAL;

	pdata->domain_id = (u32)reg;

	return 0;
}

static int scmi_regulator_probe(struct udevice *dev)
{
	struct scmi_regulator_platdata *pdata = dev_get_plat(dev);
	struct scmi_voltd_attr_in in = { 0 };
	struct scmi_voltd_attr_out out = { 0 };
	struct scmi_msg scmi_msg = {
		.protocol_id = SCMI_PROTOCOL_ID_VOLTAGE_DOMAIN,
		.message_id = SCMI_VOLTAGE_DOMAIN_ATTRIBUTES,
		.in_msg = (u8 *)&in,
		.in_msg_sz = sizeof(in),
		.out_msg = (u8 *)&out,
		.out_msg_sz = sizeof(out),
	};
	int ret;

	ret = devm_scmi_of_get_channel(dev);
	if (ret)
		return ret;

	/* Check voltage domain is known from SCMI server */
	in.domain_id = pdata->domain_id;

	ret = devm_scmi_process_msg(dev, &scmi_msg);
	if (ret) {
		dev_err(dev, "Failed to query voltage domain %u: %d\n",
			pdata->domain_id, ret);
		return -ENXIO;
	}

	return 0;
}

static const struct dm_regulator_ops scmi_voltd_ops = {
	.get_value = scmi_voltd_get_voltage_level,
	.set_value = scmi_voltd_set_voltage_level,
	.get_enable = scmi_voltd_get_enable,
	.set_enable = scmi_voltd_set_enable,
};

U_BOOT_DRIVER(scmi_regulator) = {
	.name = "scmi_regulator",
	.id = UCLASS_REGULATOR,
	.ops = &scmi_voltd_ops,
	.probe = scmi_regulator_probe,
	.of_to_plat = scmi_regulator_of_to_plat,
	.plat_auto = sizeof(struct scmi_regulator_platdata),
};

static int scmi_regulator_bind(struct udevice *dev)
{
	struct driver *drv;
	ofnode node;
	int ret;

	drv = DM_DRIVER_GET(scmi_regulator);

	ofnode_for_each_subnode(node, dev_ofnode(dev)) {
		ret = device_bind(dev, drv, ofnode_get_name(node),
				  NULL, node, NULL);
		if (ret)
			return ret;
	}

	return 0;
}

U_BOOT_DRIVER(scmi_voltage_domain) = {
	.name = "scmi_voltage_domain",
	.id = UCLASS_NOP,
	.bind = scmi_regulator_bind,
};
