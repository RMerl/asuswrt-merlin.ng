// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 NXP
 */

#include <config.h>
#include <common.h>
#include <dm.h>
#include <errno.h>
#include <log.h>
#include <thermal.h>
#include <asm/global_data.h>
#include <dm/device-internal.h>
#include <dm/device.h>
#include <firmware/imx/sci/sci.h>
#include <linux/delay.h>
#include <linux/libfdt.h>

DECLARE_GLOBAL_DATA_PTR;

struct imx_sc_thermal_plat {
	int critical;
	int alert;
	int polling_delay;
	int id;
	bool zone_node;
};

static int read_temperature(struct udevice *dev, int *temp)
{
	s16 celsius;
	s8 tenths;
	int ret;

	sc_rsrc_t *sensor_rsrc = (sc_rsrc_t *)dev_get_driver_data(dev);

	struct imx_sc_thermal_plat *pdata = dev_get_plat(dev);

	if (!temp)
		return -EINVAL;

	ret = sc_misc_get_temp(-1, sensor_rsrc[pdata->id], SC_C_TEMP,
			       &celsius, &tenths);
	if (ret) {
		printf("Error: get temperature failed! (error = %d)\n", ret);
		return ret;
	}

	*temp = celsius * 1000 + tenths * 100;

	return 0;
}

int imx_sc_thermal_get_temp(struct udevice *dev, int *temp)
{
	struct imx_sc_thermal_plat *pdata = dev_get_plat(dev);
	int cpu_temp = 0;
	int ret;

	ret = read_temperature(dev, &cpu_temp);
	if (ret)
		return ret;

	while (cpu_temp >= pdata->alert) {
		printf("CPU Temperature (%dC) beyond alert (%dC), close to critical (%dC)",
		       cpu_temp, pdata->alert, pdata->critical);
		puts(" waiting...\n");
		mdelay(pdata->polling_delay);
		ret = read_temperature(dev, &cpu_temp);
		if (ret)
			return ret;
		if (cpu_temp >= pdata->alert && !pdata->alert)
			break;
	}

	*temp = cpu_temp / 1000;

	return 0;
}

static const struct dm_thermal_ops imx_sc_thermal_ops = {
	.get_temp	= imx_sc_thermal_get_temp,
};

static int imx_sc_thermal_probe(struct udevice *dev)
{
	debug("%s dev name %s\n", __func__, dev->name);
	return 0;
}

static int imx_sc_thermal_bind(struct udevice *dev)
{
	struct imx_sc_thermal_plat *pdata = dev_get_plat(dev);
	int reg, ret;
	int offset;
	const char *name;
	const void *prop;

	debug("%s dev name %s\n", __func__, dev->name);

	prop = fdt_getprop(gd->fdt_blob, dev_of_offset(dev), "compatible",
			   NULL);
	if (!prop)
		return 0;

	pdata->zone_node = 1;

	reg = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev), "tsens-num", 0);
	if (reg == 0) {
		printf("%s: no temp sensor number provided!\n", __func__);
		return -EINVAL;
	}

	offset = fdt_subnode_offset(gd->fdt_blob, 0, "thermal-zones");
	fdt_for_each_subnode(offset, gd->fdt_blob, offset) {
		/* Bind the subnode to this driver */
		name = fdt_get_name(gd->fdt_blob, offset, NULL);

		ret = device_bind_with_driver_data(dev, dev->driver, name,
						   dev->driver_data,
						   offset_to_ofnode(offset),
						   NULL);
		if (ret)
			printf("Error binding driver '%s': %d\n",
			       dev->driver->name, ret);
	}
	return 0;
}

static int imx_sc_thermal_of_to_plat(struct udevice *dev)
{
	struct imx_sc_thermal_plat *pdata = dev_get_plat(dev);
	struct fdtdec_phandle_args args;
	const char *type;
	int ret;
	int trips_np;

	debug("%s dev name %s\n", __func__, dev->name);

	if (pdata->zone_node)
		return 0;

	ret = fdtdec_parse_phandle_with_args(gd->fdt_blob, dev_of_offset(dev),
					     "thermal-sensors",
					     "#thermal-sensor-cells",
					     0, 0, &args);
	if (ret)
		return ret;

	if (args.node != dev_of_offset(dev->parent))
		return -EFAULT;

	if (args.args_count >= 1)
		pdata->id = args.args[0];
	else
		pdata->id = 0;

	debug("args.args_count %d, id %d\n", args.args_count, pdata->id);

	pdata->polling_delay = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev),
					      "polling-delay", 1000);

	trips_np = fdt_subnode_offset(gd->fdt_blob, dev_of_offset(dev),
				      "trips");
	fdt_for_each_subnode(trips_np, gd->fdt_blob, trips_np) {
		type = fdt_getprop(gd->fdt_blob, trips_np, "type", NULL);
		if (type) {
			if (strcmp(type, "critical") == 0) {
				pdata->critical = fdtdec_get_int(gd->fdt_blob,
								 trips_np,
								 "temperature",
								 85);
			} else if (strcmp(type, "passive") == 0) {
				pdata->alert = fdtdec_get_int(gd->fdt_blob,
							      trips_np,
							      "temperature",
							      80);
			}
		}
	}

	debug("id %d polling_delay %d, critical %d, alert %d\n", pdata->id,
	      pdata->polling_delay, pdata->critical, pdata->alert);

	return 0;
}

static const sc_rsrc_t imx8qm_sensor_rsrc[] = {
	SC_R_A53, SC_R_A72, SC_R_GPU_0_PID0, SC_R_GPU_1_PID0,
	SC_R_DRC_0, SC_R_DRC_1, SC_R_VPU_PID0, SC_R_PMIC_0,
	SC_R_PMIC_1, SC_R_PMIC_2,
};

static const sc_rsrc_t imx8qxp_sensor_rsrc[] = {
	SC_R_SYSTEM, SC_R_DRC_0, SC_R_PMIC_0,
	SC_R_PMIC_1, SC_R_PMIC_2,
};

static const struct udevice_id imx_sc_thermal_ids[] = {
	{ .compatible = "nxp,imx8qm-sc-tsens", .data =
		(ulong)&imx8qm_sensor_rsrc, },
	{ .compatible = "nxp,imx8qxp-sc-tsens", .data =
		(ulong)&imx8qxp_sensor_rsrc, },
	{ }
};

U_BOOT_DRIVER(imx_sc_thermal) = {
	.name	= "imx_sc_thermal",
	.id	= UCLASS_THERMAL,
	.ops	= &imx_sc_thermal_ops,
	.of_match = imx_sc_thermal_ids,
	.bind = imx_sc_thermal_bind,
	.probe	= imx_sc_thermal_probe,
	.of_to_plat = imx_sc_thermal_of_to_plat,
	.plat_auto	= sizeof(struct imx_sc_thermal_plat),
	.flags  = DM_FLAG_PRE_RELOC,
};
