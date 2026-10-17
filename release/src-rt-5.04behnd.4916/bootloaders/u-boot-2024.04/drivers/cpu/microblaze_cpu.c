// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022, Ovidiu Panait <ovpanait@gmail.com>
 */
#include <common.h>
#include <cpu.h>
#include <dm.h>
#include <asm/cpuinfo.h>
#include <asm/global_data.h>
#include <asm/pvr.h>

DECLARE_GLOBAL_DATA_PTR;

#define update_cpuinfo_pvr(pvr, ci, name)					\
{										\
	u32 tmp = PVR_##name(pvr);						\
	if (ci != tmp)								\
		printf("PVR value for " #name " does not match static data!\n");\
	ci = tmp;								\
}

static int microblaze_cpu_probe_all(void)
{
	int ret;

	ret = cpu_probe_all();
	if (ret)
		return log_msg_ret("Microblaze cpus probe failed\n", ret);

	return 0;
}
EVENT_SPY_SIMPLE(EVT_DM_POST_INIT_F, microblaze_cpu_probe_all);

static void microblaze_set_cpuinfo_pvr(struct microblaze_cpuinfo *ci)
{
	u32 pvr[PVR_FULL_COUNT];

	microblaze_get_all_pvrs(pvr);

	update_cpuinfo_pvr(pvr, ci->icache_size, ICACHE_BYTE_SIZE);
	update_cpuinfo_pvr(pvr, ci->icache_line_length, ICACHE_LINE_LEN);

	update_cpuinfo_pvr(pvr, ci->dcache_size, DCACHE_BYTE_SIZE);
	update_cpuinfo_pvr(pvr, ci->dcache_line_length, DCACHE_LINE_LEN);

	update_cpuinfo_pvr(pvr, ci->use_mmu, USE_MMU);
	update_cpuinfo_pvr(pvr, ci->ver_code, VERSION);
	update_cpuinfo_pvr(pvr, ci->fpga_code, TARGET_FAMILY);
}

static void microblaze_set_cpuinfo_static(struct udevice *dev,
					  struct microblaze_cpuinfo *ci)
{
	const char *hw_ver = CONFIG_XILINX_MICROBLAZE0_HW_VER;
	const char *fpga_family = CONFIG_XILINX_MICROBLAZE0_FPGA_FAMILY;

	ci->icache_size = dev_read_u32_default(dev, "i-cache-size", 0);
	ci->icache_line_length = dev_read_u32_default(dev,
						"i-cache-line-size", 0);

	ci->dcache_size = dev_read_u32_default(dev, "d-cache-size", 0);
	ci->dcache_line_length = dev_read_u32_default(dev,
						"d-cache-line-size", 0);

	ci->cpu_freq = dev_read_u32_default(dev, "clock-frequency", 0);
	ci->addr_size = dev_read_u32_default(dev, "xlnx,addr-size", 32);
	ci->use_mmu = dev_read_u32_default(dev, "xlnx,use-mmu", 0);

	ci->ver_code = microblaze_lookup_cpu_version_code(hw_ver);
	ci->fpga_code = microblaze_lookup_fpga_family_code(fpga_family);
}

static int microblaze_cpu_probe(struct udevice *dev)
{
	microblaze_set_cpuinfo_static(dev, gd_cpuinfo());

	if (microblaze_cpu_has_pvr_full())
		microblaze_set_cpuinfo_pvr(gd_cpuinfo());
	else
		debug("No PVR support. Using only static CPU info.\n");

	return 0;
}

static int microblaze_cpu_get_desc(const struct udevice *dev, char *buf,
				   int size)
{
	struct microblaze_cpuinfo *ci = gd_cpuinfo();
	const char *cpu_ver, *fpga_family;
	u32 cpu_freq_mhz;
	int ret;

	cpu_freq_mhz = ci->cpu_freq / 1000000;
	cpu_ver = microblaze_lookup_cpu_version_string(ci->ver_code);
	fpga_family = microblaze_lookup_fpga_family_string(ci->fpga_code);

	ret = snprintf(buf, size,
		       "MicroBlaze @ %uMHz, Rev: %s, FPGA family: %s",
		       cpu_freq_mhz, cpu_ver, fpga_family);
	if (ret < 0)
		return ret;

	return (ret >= size) ? -ENOSPC : 0;
}

static int microblaze_cpu_get_info(const struct udevice *dev,
				   struct cpu_info *info)
{
	struct microblaze_cpuinfo *ci = gd_cpuinfo();

	info->cpu_freq = ci->cpu_freq;
	info->address_width = ci->addr_size;

	if (ci->icache_size || ci->dcache_size)
		info->features |= BIT(CPU_FEAT_L1_CACHE);

	if (ci->use_mmu)
		info->features |= BIT(CPU_FEAT_MMU);

	return 0;
}

static int microblaze_cpu_get_count(const struct udevice *dev)
{
	return 1;
}

static const struct cpu_ops microblaze_cpu_ops = {
	.get_desc	= microblaze_cpu_get_desc,
	.get_info	= microblaze_cpu_get_info,
	.get_count	= microblaze_cpu_get_count,
};

static const struct udevice_id microblaze_cpu_ids[] = {
	{ .compatible = "xlnx,microblaze-11.0" },
	{ .compatible = "xlnx,microblaze-10.0" },
	{ .compatible = "xlnx,microblaze-9.6" },
	{ .compatible = "xlnx,microblaze-9.5" },
	{ .compatible = "xlnx,microblaze-9.4" },
	{ .compatible = "xlnx,microblaze-9.3" },
	{ .compatible = "xlnx,microblaze-9.2" },
	{ .compatible = "xlnx,microblaze-9.1" },
	{ .compatible = "xlnx,microblaze-9.0" },
	{ .compatible = "xlnx,microblaze-8.50.c" },
	{ .compatible = "xlnx,microblaze-8.50.b" },
	{ .compatible = "xlnx,microblaze-8.50.a" },
	{ .compatible = "xlnx,microblaze-8.40.b" },
	{ .compatible = "xlnx,microblaze-8.40.a" },
	{ .compatible = "xlnx,microblaze-8.30.a" },
	{ .compatible = "xlnx,microblaze-8.20.b" },
	{ .compatible = "xlnx,microblaze-8.20.a" },
	{ .compatible = "xlnx,microblaze-8.10.a" },
	{ .compatible = "xlnx,microblaze-8.00.b" },
	{ .compatible = "xlnx,microblaze-8.00.a" },
	{ .compatible = "xlnx,microblaze-7.30.b" },
	{ .compatible = "xlnx,microblaze-7.30.a" },
	{ .compatible = "xlnx,microblaze-7.20.d" },
	{ .compatible = "xlnx,microblaze-7.20.c" },
	{ .compatible = "xlnx,microblaze-7.20.b" },
	{ .compatible = "xlnx,microblaze-7.20.a" },
	{ .compatible = "xlnx,microblaze-7.10.d" },
	{ .compatible = "xlnx,microblaze-7.10.c" },
	{ .compatible = "xlnx,microblaze-7.10.b" },
	{ .compatible = "xlnx,microblaze-7.10.a" },
	{ .compatible = "xlnx,microblaze-7.00.b" },
	{ .compatible = "xlnx,microblaze-7.00.a" },
	{ .compatible = "xlnx,microblaze-6.00.b" },
	{ .compatible = "xlnx,microblaze-6.00.a" },
	{ .compatible = "xlnx,microblaze-5.00.c" },
	{ .compatible = "xlnx,microblaze-5.00.b" },
	{ .compatible = "xlnx,microblaze-5.00.a" },
	{ }
};

U_BOOT_DRIVER(microblaze_cpu) = {
	.name		= "microblaze_cpu",
	.id		= UCLASS_CPU,
	.of_match	= microblaze_cpu_ids,
	.probe		= microblaze_cpu_probe,
	.ops		= &microblaze_cpu_ops,
	.flags		= DM_FLAG_PRE_RELOC,
};
