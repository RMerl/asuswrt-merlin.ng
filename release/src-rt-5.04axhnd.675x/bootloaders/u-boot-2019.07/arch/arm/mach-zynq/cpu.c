// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Michal Simek <monstr@monstr.eu>
 * Copyright (C) 2012 Xilinx, Inc. All rights reserved.
 */
#include <common.h>
#include <zynqpl.h>
#include <asm/io.h>
#include <asm/arch/clk.h>
#include <asm/arch/hardware.h>
#include <asm/arch/ps7_init_gpl.h>
#include <asm/arch/sys_proto.h>

#define ZYNQ_SILICON_VER_MASK	0xF0000000
#define ZYNQ_SILICON_VER_SHIFT	28

#if (defined(CONFIG_FPGA) && !defined(CONFIG_SPL_BUILD)) || \
    (defined(CONFIG_SPL_FPGA_SUPPORT) && defined(CONFIG_SPL_BUILD))
xilinx_desc fpga = {
	.family = xilinx_zynq,
	.iface = devcfg,
	.operations = &zynq_op,
};
#endif

static const struct {
	u8 idcode;
#if defined(CONFIG_FPGA)
	u32 fpga_size;
#endif
	char *devicename;
} zynq_fpga_descs[] = {
	ZYNQ_DESC(7Z007S),
	ZYNQ_DESC(7Z010),
	ZYNQ_DESC(7Z012S),
	ZYNQ_DESC(7Z014S),
	ZYNQ_DESC(7Z015),
	ZYNQ_DESC(7Z020),
	ZYNQ_DESC(7Z030),
	ZYNQ_DESC(7Z035),
	ZYNQ_DESC(7Z045),
	ZYNQ_DESC(7Z100),
	{ /* Sentinel */ },
};

int arch_cpu_init(void)
{
	zynq_slcr_unlock();
#ifndef CONFIG_SPL_BUILD
	/* Device config APB, unlock the PCAP */
	writel(0x757BDF0D, &devcfg_base->unlock);
	writel(0xFFFFFFFF, &devcfg_base->rom_shadow);

#if (CONFIG_SYS_SDRAM_BASE == 0)
	/* remap DDR to zero, FILTERSTART */
	writel(0, &scu_base->filter_start);

	/* OCM_CFG, Mask out the ROM, map ram into upper addresses */
	writel(0x1F, &slcr_base->ocm_cfg);
	/* FPGA_RST_CTRL, clear resets on AXI fabric ports */
	writel(0x0, &slcr_base->fpga_rst_ctrl);
	/* Set urgent bits with register */
	writel(0x0, &slcr_base->ddr_urgent_sel);
	/* Urgent write, ports S2/S3 */
	writel(0xC, &slcr_base->ddr_urgent);
#endif
#endif
	zynq_slcr_lock();

	return 0;
}

unsigned int zynq_get_silicon_version(void)
{
	return (readl(&devcfg_base->mctrl) & ZYNQ_SILICON_VER_MASK)
						>> ZYNQ_SILICON_VER_SHIFT;
}

void reset_cpu(ulong addr)
{
	zynq_slcr_cpu_reset();
	while (1)
		;
}

#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
void enable_caches(void)
{
	/* Enable D-cache. I-cache is already enabled in start.S */
	dcache_enable();
}
#endif

static int __maybe_unused cpu_desc_id(void)
{
	u32 idcode;
	u8 i;

	idcode = zynq_slcr_get_idcode();
	for (i = 0; zynq_fpga_descs[i].idcode; i++) {
		if (zynq_fpga_descs[i].idcode == idcode)
			return i;
	}

	return -ENODEV;
}

#if defined(CONFIG_ARCH_EARLY_INIT_R)
int arch_early_init_r(void)
{
#if (defined(CONFIG_FPGA) && !defined(CONFIG_SPL_BUILD)) || \
    (defined(CONFIG_SPL_FPGA_SUPPORT) && defined(CONFIG_SPL_BUILD))
	int cpu_id = cpu_desc_id();

	if (cpu_id < 0)
		return 0;

	fpga.size = zynq_fpga_descs[cpu_id].fpga_size;
	fpga.name = zynq_fpga_descs[cpu_id].devicename;
	fpga_init();
	fpga_add(fpga_xilinx, &fpga);
#endif
	return 0;
}
#endif

#ifdef CONFIG_DISPLAY_CPUINFO
int print_cpuinfo(void)
{
	u32 version;
	int cpu_id = cpu_desc_id();

	if (cpu_id < 0)
		return 0;

	version = zynq_get_silicon_version() << 1;
	if (version > (PCW_SILICON_VERSION_3 << 1))
		version += 1;

	printf("CPU:   Zynq %s\n", zynq_fpga_descs[cpu_id].devicename);
	printf("Silicon: v%d.%d\n", version >> 1, version & 1);
	return 0;
}
#endif
