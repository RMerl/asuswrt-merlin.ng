// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 NXP
 */

#include <common.h>
#include <cpu.h>
#include <dm.h>
#include <thermal.h>
#include <asm/global_data.h>
#include <asm/system.h>
#include <firmware/imx/sci/sci.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch-imx/cpu.h>
#include <asm/armv8/cpu.h>
#include <imx_thermal.h>
#include <linux/bitops.h>
#include <linux/clk-provider.h>

DECLARE_GLOBAL_DATA_PTR;

struct cpu_imx_plat {
	const char *name;
	const char *rev;
	const char *type;
	u32 cpu_rsrc;
	u32 cpurev;
	u32 freq_mhz;
	u32 mpidr;
};

static const char *get_imx_type_str(u32 imxtype)
{
	switch (imxtype) {
	case MXC_CPU_IMX8QXP:
	case MXC_CPU_IMX8QXP_A0:
		return "8QXP";
	case MXC_CPU_IMX8QM:
		return "8QM";
	case MXC_CPU_IMX93:
		return "93(52)";/* iMX93 Dual core with NPU */
	case MXC_CPU_IMX9351:
		return "93(51)";/* iMX93 Single core with NPU */
	case MXC_CPU_IMX9332:
		return "93(32)";/* iMX93 Dual core without NPU */
	case MXC_CPU_IMX9331:
		return "93(31)";/* iMX93 Single core without NPU */
	case MXC_CPU_IMX9322:
		return "93(22)";/* iMX93 9x9 Dual core  */
	case MXC_CPU_IMX9321:
		return "93(21)";/* iMX93 9x9 Single core  */
	case MXC_CPU_IMX9312:
		return "93(12)";/* iMX93 9x9 Dual core without NPU */
	case MXC_CPU_IMX9311:
		return "93(11)";/* iMX93 9x9 Single core without NPU */
	default:
		return "??";
	}
}

static const char *get_imx_rev_str(u32 rev)
{
	static char revision[4];

	if (IS_ENABLED(CONFIG_IMX8)) {
		switch (rev) {
		case CHIP_REV_A:
			return "A";
		case CHIP_REV_B:
			return "B";
		case CHIP_REV_C:
			return "C";
		default:
			return "?";
		}
	} else {
		revision[0] = '1' + (((rev & 0xf0) - CHIP_REV_1_0) >> 4);
		revision[1] = '.';
		revision[2] = '0' + (rev & 0xf);
		revision[3] = '\0';

		return revision;
	}
}

static void set_core_data(struct udevice *dev)
{
	struct cpu_imx_plat *plat = dev_get_plat(dev);

	if (device_is_compatible(dev, "arm,cortex-a35")) {
		plat->cpu_rsrc = SC_R_A35;
		plat->name = "A35";
	} else if (device_is_compatible(dev, "arm,cortex-a53")) {
		plat->cpu_rsrc = SC_R_A53;
		plat->name = "A53";
	} else if (device_is_compatible(dev, "arm,cortex-a72")) {
		plat->cpu_rsrc = SC_R_A72;
		plat->name = "A72";
	} else if (device_is_compatible(dev, "arm,cortex-a55")) {
		plat->name = "A55";
	} else {
		plat->cpu_rsrc = SC_R_A53;
		plat->name = "?";
	}
}

#if IS_ENABLED(CONFIG_DM_THERMAL)
static int cpu_imx_get_temp(struct cpu_imx_plat *plat)
{
	struct udevice *thermal_dev;
	int cpu_tmp, ret;
	int idx = 1; /* use "cpu-thermal0" device */

	if (IS_ENABLED(CONFIG_IMX8)) {
		if (plat->cpu_rsrc == SC_R_A72)
			idx = 2; /* use "cpu-thermal1" device */
	} else {
		idx = 1;
	}

	ret = uclass_get_device(UCLASS_THERMAL, idx, &thermal_dev);
	if (!ret) {
		ret = thermal_get_temp(thermal_dev, &cpu_tmp);
		if (ret)
			return 0xdeadbeef;
	} else {
		return 0xdeadbeef;
	}

	return cpu_tmp;
}
#else
static int cpu_imx_get_temp(struct cpu_imx_plat *plat)
{
	return 0;
}
#endif

__weak u32 get_cpu_temp_grade(int *minc, int *maxc)
{
	return 0;
}

static int cpu_imx_get_desc(const struct udevice *dev, char *buf, int size)
{
	struct cpu_imx_plat *plat = dev_get_plat(dev);
	const char *grade;
	int ret, temp;
	int minc, maxc;

	if (size < 100)
		return -ENOSPC;

	ret = snprintf(buf, size, "NXP i.MX%s Rev%s %s at %u MHz",
		       plat->type, plat->rev, plat->name, plat->freq_mhz);

	if (IS_ENABLED(CONFIG_IMX9)) {
		switch (get_cpu_temp_grade(&minc, &maxc)) {
		case TEMP_AUTOMOTIVE:
			grade = "Automotive temperature grade ";
			break;
		case TEMP_INDUSTRIAL:
			grade = "Industrial temperature grade ";
			break;
		case TEMP_EXTCOMMERCIAL:
			grade = "Extended Consumer temperature grade ";
			break;
		default:
			grade = "Consumer temperature grade ";
			break;
		}

		buf = buf + ret;
		size = size - ret;
		ret = snprintf(buf, size, "\nCPU:   %s (%dC to %dC)", grade, minc, maxc);
	}

	if (IS_ENABLED(CONFIG_DM_THERMAL)) {
		temp = cpu_imx_get_temp(plat);
		buf = buf + ret;
		size = size - ret;
		if (temp != 0xdeadbeef)
			ret = snprintf(buf, size, " at %dC", temp);
		else
			ret = snprintf(buf, size, " - invalid sensor data");
	}

	snprintf(buf + ret, size - ret, "\n");

	return 0;
}

static int cpu_imx_get_info(const struct udevice *dev, struct cpu_info *info)
{
	struct cpu_imx_plat *plat = dev_get_plat(dev);

	info->cpu_freq = plat->freq_mhz * 1000;
	info->features = BIT(CPU_FEAT_L1_CACHE) | BIT(CPU_FEAT_MMU);
	return 0;
}

static int cpu_imx_get_count(const struct udevice *dev)
{
	ofnode node;
	int num = 0;

	ofnode_for_each_subnode(node, dev_ofnode(dev->parent)) {
		const char *device_type;

		if (!ofnode_is_enabled(node))
			continue;

		device_type = ofnode_read_string(node, "device_type");
		if (!device_type)
			continue;

		if (!strcmp(device_type, "cpu"))
			num++;
	}

	return num;
}

static int cpu_imx_get_vendor(const struct udevice *dev,  char *buf, int size)
{
	snprintf(buf, size, "NXP");
	return 0;
}

static int cpu_imx_is_current(struct udevice *dev)
{
	struct cpu_imx_plat *plat = dev_get_plat(dev);

	if (plat->mpidr == (read_mpidr() & 0xffff))
		return 1;

	return 0;
}

static const struct cpu_ops cpu_imx_ops = {
	.get_desc	= cpu_imx_get_desc,
	.get_info	= cpu_imx_get_info,
	.get_count	= cpu_imx_get_count,
	.get_vendor	= cpu_imx_get_vendor,
	.is_current	= cpu_imx_is_current,
};

static const struct udevice_id cpu_imx_ids[] = {
	{ .compatible = "arm,cortex-a35" },
	{ .compatible = "arm,cortex-a53" },
	{ .compatible = "arm,cortex-a55" },
	{ .compatible = "arm,cortex-a72" },
	{ }
};

static ulong imx_get_cpu_rate(struct udevice *dev)
{
	struct cpu_imx_plat *plat = dev_get_plat(dev);
	struct clk clk;
	ulong rate;
	int ret;

	if (IS_ENABLED(CONFIG_IMX8)) {
		ret = sc_pm_get_clock_rate(-1, plat->cpu_rsrc, SC_PM_CLK_CPU,
					   (sc_pm_clock_rate_t *)&rate);
	} else {
		ret = clk_get_by_index(dev, 0, &clk);
		if (!ret) {
			rate = clk_get_rate(&clk);
			if (!rate)
				ret = -EOPNOTSUPP;
		}
	}
	if (ret) {
		printf("Could not read CPU frequency: %d\n", ret);
		return 0;
	}

	return rate;
}

static int imx_cpu_probe(struct udevice *dev)
{
	struct cpu_imx_plat *plat = dev_get_plat(dev);
	u32 cpurev;

	set_core_data(dev);
	cpurev = get_cpu_rev();
	plat->cpurev = cpurev;
	plat->rev = get_imx_rev_str(cpurev & 0xFFF);
	plat->type = get_imx_type_str((cpurev & 0xFF000) >> 12);
	plat->freq_mhz = imx_get_cpu_rate(dev) / 1000000;
	plat->mpidr = dev_read_addr(dev);
	if (plat->mpidr == FDT_ADDR_T_NONE) {
		printf("%s: Failed to get CPU reg property\n", __func__);
		return -EINVAL;
	}

	return 0;
}

U_BOOT_DRIVER(cpu_imx_drv) = {
	.name		= "imx_cpu",
	.id		= UCLASS_CPU,
	.of_match	= cpu_imx_ids,
	.ops		= &cpu_imx_ops,
	.probe		= imx_cpu_probe,
	.plat_auto	= sizeof(struct cpu_imx_plat),
	.flags		= DM_FLAG_PRE_RELOC,
};
