// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Synopsys, Inc. All rights reserved.
 * Author: Eugeniy Paltsev <Eugeniy.Paltsev@synopsys.com>
 */

#include <common.h>
#include <config.h>
#include <linux/printk.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <asm/arcregs.h>
#include <fdt_support.h>
#include <dwmmc.h>
#include <malloc.h>
#include <usb.h>

#include "clk-lib.h"
#include "env-lib.h"

DECLARE_GLOBAL_DATA_PTR;

#define ALL_CPU_MASK		GENMASK(NR_CPUS - 1, 0)
#define MASTER_CPU_ID		0
#define APERTURE_SHIFT		28
#define NO_CCM			0x10
#define SLAVE_CPU_READY		0x12345678
#define BOOTSTAGE_1		1 /* after SP, FP setup, before HW init */
#define BOOTSTAGE_2		2 /* after HW init, before self halt */
#define BOOTSTAGE_3		3 /* after self halt */
#define BOOTSTAGE_4		4 /* before app launch */
#define BOOTSTAGE_5		5 /* after app launch, unreachable */

#define RESET_VECTOR_ADDR	0x0

#define CREG_BASE		(ARC_PERIPHERAL_BASE + 0x1000)
#define CREG_CPU_START		(CREG_BASE + 0x400)
#define CREG_CPU_START_MASK	0xF

#define SDIO_BASE		(ARC_PERIPHERAL_BASE + 0xA000)
#define SDIO_UHS_REG_EXT	(SDIO_BASE + 0x108)
#define SDIO_UHS_REG_EXT_DIV_2	(2 << 30)

/* Uncached access macros */
#define arc_read_uncached_32(ptr)	\
({					\
	unsigned int __ret;		\
	__asm__ __volatile__(		\
	"	ld.di %0, [%1]	\n"	\
	: "=r"(__ret)			\
	: "r"(ptr));			\
	__ret;				\
})

#define arc_write_uncached_32(ptr, data)\
({					\
	__asm__ __volatile__(		\
	"	st.di %0, [%1]	\n"	\
	:				\
	: "r"(data), "r"(ptr));		\
})

struct hsdk_env_core_ctl {
	u32_env entry[NR_CPUS];
	u32_env iccm[NR_CPUS];
	u32_env dccm[NR_CPUS];
};

struct hsdk_env_common_ctl {
	bool halt_on_boot;
	u32_env core_mask;
	u32_env cpu_freq;
	u32_env axi_freq;
	u32_env tun_freq;
	u32_env nvlim;
	u32_env icache;
	u32_env dcache;
};

/*
 * Uncached cross-cpu structure. All CPUs must access to this structure fields
 * only with arc_read_uncached_32() / arc_write_uncached_32() accessors (which
 * implement ld.di / st.di instructions). Simultaneous cached and uncached
 * access to this area will lead to data loss.
 * We flush all data caches in board_early_init_r() as we don't want to have
 * any dirty line in L1d$ or SL$ in this area.
 */
struct hsdk_cross_cpu {
	/* slave CPU ready flag */
	u32 ready_flag;
	/* address of the area, which can be used for stack by slave CPU */
	u32 stack_ptr;
	/* slave CPU status - bootstage number */
	s32 status[NR_CPUS];

	/*
	 * Slave CPU data - it is copy of corresponding fields in
	 * hsdk_env_core_ctl and hsdk_env_common_ctl structures which are
	 * required for slave CPUs initialization.
	 * This fields can be populated by copying from hsdk_env_core_ctl
	 * and hsdk_env_common_ctl structures with sync_cross_cpu_data()
	 * function.
	 */
	u32 entry[NR_CPUS];
	u32 iccm[NR_CPUS];
	u32 dccm[NR_CPUS];

	u32 core_mask;
	u32 icache;
	u32 dcache;

	u8 cache_padding[ARCH_DMA_MINALIGN];
} __aligned(ARCH_DMA_MINALIGN);

/* Place for slave CPUs temporary stack */
static u32 slave_stack[256 * NR_CPUS] __aligned(ARCH_DMA_MINALIGN);

static struct hsdk_env_common_ctl env_common = {};
static struct hsdk_env_core_ctl env_core = {};
static struct hsdk_cross_cpu cross_cpu_data;

static const struct env_map_common env_map_common[] = {
	{ "core_mask",	ENV_HEX, true,	0x1, 0xF,	&env_common.core_mask },
	{ "non_volatile_limit", ENV_HEX, true, 0, 0xF,	&env_common.nvlim },
	{ "icache_ena",	ENV_HEX, true,	0, 1,		&env_common.icache },
	{ "dcache_ena",	ENV_HEX, true,	0, 1,		&env_common.dcache },
	{}
};

static const struct env_map_common env_map_clock[] = {
	{ "cpu_freq",	ENV_DEC, false,	100, 1000,	&env_common.cpu_freq },
	{ "axi_freq",	ENV_DEC, false,	200, 800,	&env_common.axi_freq },
	{ "tun_freq",	ENV_DEC, false,	0, 150,		&env_common.tun_freq },
	{}
};

static const struct env_map_percpu env_map_core[] = {
	{ "core_iccm", ENV_HEX, true, {NO_CCM, 0, NO_CCM, 0}, {NO_CCM, 0xF, NO_CCM, 0xF}, &env_core.iccm },
	{ "core_dccm", ENV_HEX, true, {NO_CCM, 0, NO_CCM, 0}, {NO_CCM, 0xF, NO_CCM, 0xF}, &env_core.dccm },
	{}
};

static const struct env_map_common env_map_mask[] = {
	{ "core_mask",	ENV_HEX, false,	0x1, 0xF,	&env_common.core_mask },
	{}
};

static const struct env_map_percpu env_map_go[] = {
	{ "core_entry", ENV_HEX, true, {0, 0, 0, 0}, {U32_MAX, U32_MAX, U32_MAX, U32_MAX}, &env_core.entry },
	{}
};

static void sync_cross_cpu_data(void)
{
	u32 value;

	for (u32 i = 0; i < NR_CPUS; i++) {
		value = env_core.entry[i].val;
		arc_write_uncached_32(&cross_cpu_data.entry[i], value);
	}

	for (u32 i = 0; i < NR_CPUS; i++) {
		value = env_core.iccm[i].val;
		arc_write_uncached_32(&cross_cpu_data.iccm[i], value);
	}

	for (u32 i = 0; i < NR_CPUS; i++) {
		value = env_core.dccm[i].val;
		arc_write_uncached_32(&cross_cpu_data.dccm[i], value);
	}

	value = env_common.core_mask.val;
	arc_write_uncached_32(&cross_cpu_data.core_mask, value);

	value = env_common.icache.val;
	arc_write_uncached_32(&cross_cpu_data.icache, value);

	value = env_common.dcache.val;
	arc_write_uncached_32(&cross_cpu_data.dcache, value);
}

/* Can be used only on master CPU */
static bool is_cpu_used(u32 cpu_id)
{
	return !!(env_common.core_mask.val & BIT(cpu_id));
}

/* TODO: add ICCM BCR and DCCM BCR runtime check */
static void init_slave_cpu_func(u32 core)
{
	u32 val;

	/* Remap ICCM to another memory region if it exists */
	val = arc_read_uncached_32(&cross_cpu_data.iccm[core]);
	if (val != NO_CCM)
		write_aux_reg(ARC_AUX_ICCM_BASE, val << APERTURE_SHIFT);

	/* Remap DCCM to another memory region if it exists */
	val = arc_read_uncached_32(&cross_cpu_data.dccm[core]);
	if (val != NO_CCM)
		write_aux_reg(ARC_AUX_DCCM_BASE, val << APERTURE_SHIFT);

	if (arc_read_uncached_32(&cross_cpu_data.icache))
		icache_enable();
	else
		icache_disable();

	if (arc_read_uncached_32(&cross_cpu_data.dcache))
		dcache_enable();
	else
		dcache_disable();
}

static void init_cluster_nvlim(void)
{
	u32 val = env_common.nvlim.val << APERTURE_SHIFT;

	flush_dcache_all();
	write_aux_reg(ARC_AUX_NON_VOLATILE_LIMIT, val);
	write_aux_reg(AUX_AUX_CACHE_LIMIT, val);
	flush_n_invalidate_dcache_all();
}

static void init_master_icache(void)
{
	if (icache_status()) {
		/* I$ is enabled - we need to disable it */
		if (!env_common.icache.val)
			icache_disable();
	} else {
		/* I$ is disabled - we need to enable it */
		if (env_common.icache.val) {
			icache_enable();

			/* invalidate I$ right after enable */
			invalidate_icache_all();
		}
	}
}

static void init_master_dcache(void)
{
	if (dcache_status()) {
		/* D$ is enabled - we need to disable it */
		if (!env_common.dcache.val)
			dcache_disable();
	} else {
		/* D$ is disabled - we need to enable it */
		if (env_common.dcache.val)
			dcache_enable();

		/* TODO: probably we need ti invalidate D$ right after enable */
	}
}

static int cleanup_before_go(void)
{
	disable_interrupts();
	sync_n_cleanup_cache_all();

	return 0;
}

void slave_cpu_set_boot_addr(u32 addr)
{
	/* All cores have reset vector pointing to 0 */
	writel(addr, (void __iomem *)RESET_VECTOR_ADDR);

	/* Make sure other cores see written value in memory */
	sync_n_cleanup_cache_all();
}

static inline void halt_this_cpu(void)
{
	__builtin_arc_flag(1);
}

static void smp_kick_cpu_x(u32 cpu_id)
{
	int cmd = readl((void __iomem *)CREG_CPU_START);

	if (cpu_id > NR_CPUS)
		return;

	cmd &= ~CREG_CPU_START_MASK;
	cmd |= (1 << cpu_id);
	writel(cmd, (void __iomem *)CREG_CPU_START);
}

static u32 prepare_cpu_ctart_reg(void)
{
	int cmd = readl((void __iomem *)CREG_CPU_START);

	cmd &= ~CREG_CPU_START_MASK;

	return cmd | env_common.core_mask.val;
}

/* slave CPU entry for configuration */
__attribute__((naked, noreturn, flatten)) noinline void hsdk_core_init_f(void)
{
	__asm__ __volatile__(
		"ld.di	r8,	[%0]\n"
		"mov	%%sp,	r8\n"
		"mov	%%fp,	%%sp\n"
		: /* no output */
		: "r" (&cross_cpu_data.stack_ptr));

	invalidate_icache_all();

	arc_write_uncached_32(&cross_cpu_data.status[CPU_ID_GET()], BOOTSTAGE_1);
	init_slave_cpu_func(CPU_ID_GET());

	arc_write_uncached_32(&cross_cpu_data.ready_flag, SLAVE_CPU_READY);
	arc_write_uncached_32(&cross_cpu_data.status[CPU_ID_GET()], BOOTSTAGE_2);

	/* Halt the processor until the master kick us again */
	halt_this_cpu();

	/*
	 * 3 NOPs after FLAG 1 instruction are no longer required for ARCv2
	 * cores but we leave them for gebug purposes.
	 */
	__builtin_arc_nop();
	__builtin_arc_nop();
	__builtin_arc_nop();

	arc_write_uncached_32(&cross_cpu_data.status[CPU_ID_GET()], BOOTSTAGE_3);

	/* get the updated entry - invalidate i$ */
	invalidate_icache_all();

	arc_write_uncached_32(&cross_cpu_data.status[CPU_ID_GET()], BOOTSTAGE_4);

	/* Run our program */
	((void (*)(void))(arc_read_uncached_32(&cross_cpu_data.entry[CPU_ID_GET()])))();

	/* This bootstage is unreachable as we don't return from app we launch */
	arc_write_uncached_32(&cross_cpu_data.status[CPU_ID_GET()], BOOTSTAGE_5);

	/* Something went terribly wrong */
	while (true)
		halt_this_cpu();
}

static void clear_cross_cpu_data(void)
{
	arc_write_uncached_32(&cross_cpu_data.ready_flag, 0);
	arc_write_uncached_32(&cross_cpu_data.stack_ptr, 0);

	for (u32 i = 0; i < NR_CPUS; i++)
		arc_write_uncached_32(&cross_cpu_data.status[i], 0);
}

static noinline void do_init_slave_cpu(u32 cpu_id)
{
	/* attempts number for check clave CPU ready_flag */
	u32 attempts = 100;
	u32 stack_ptr = (u32)(slave_stack + (64 * cpu_id));

	if (cpu_id >= NR_CPUS)
		return;

	arc_write_uncached_32(&cross_cpu_data.ready_flag, 0);

	/* Use global unique place for each slave cpu stack */
	arc_write_uncached_32(&cross_cpu_data.stack_ptr, stack_ptr);

	debug("CPU %u: stack pool base: %p\n", cpu_id, slave_stack);
	debug("CPU %u: current slave stack base: %x\n", cpu_id, stack_ptr);
	slave_cpu_set_boot_addr((u32)hsdk_core_init_f);

	smp_kick_cpu_x(cpu_id);

	debug("CPU %u: cross-cpu flag: %x [before timeout]\n", cpu_id,
	      arc_read_uncached_32(&cross_cpu_data.ready_flag));

	while (!arc_read_uncached_32(&cross_cpu_data.ready_flag) && attempts--)
		mdelay(10);

	/* Just to be sure that slave cpu is halted after it set ready_flag */
	mdelay(20);

	/*
	 * Only print error here if we reach timeout as there is no option to
	 * halt slave cpu (or check that slave cpu is halted)
	 */
	if (!attempts)
		pr_err("CPU %u is not responding after init!\n", cpu_id);

	/* Check current stage of slave cpu */
	if (arc_read_uncached_32(&cross_cpu_data.status[cpu_id]) != BOOTSTAGE_2)
		pr_err("CPU %u status is unexpected: %d\n", cpu_id,
		       arc_read_uncached_32(&cross_cpu_data.status[cpu_id]));

	debug("CPU %u: cross-cpu flag: %x [after timeout]\n", cpu_id,
	      arc_read_uncached_32(&cross_cpu_data.ready_flag));
	debug("CPU %u: status: %d [after timeout]\n", cpu_id,
	      arc_read_uncached_32(&cross_cpu_data.status[cpu_id]));
}

static void do_init_slave_cpus(void)
{
	clear_cross_cpu_data();
	sync_cross_cpu_data();

	debug("cross_cpu_data location: %#x\n", (u32)&cross_cpu_data);

	for (u32 i = MASTER_CPU_ID + 1; i < NR_CPUS; i++)
		if (is_cpu_used(i))
			do_init_slave_cpu(i);
}

static void do_init_master_cpu(void)
{
	/*
	 * Setup master caches even if master isn't used as we want to use
	 * same cache configuration on all running CPUs
	 */
	init_master_icache();
	init_master_dcache();
}

enum hsdk_axi_masters {
	M_HS_CORE = 0,
	M_HS_RTT,
	M_AXI_TUN,
	M_HDMI_VIDEO,
	M_HDMI_AUDIO,
	M_USB_HOST,
	M_ETHERNET,
	M_SDIO,
	M_GPU,
	M_DMAC_0,
	M_DMAC_1,
	M_DVFS
};

#define UPDATE_VAL	1

/*
 * m	master		AXI_M_m_SLV0	AXI_M_m_SLV1	AXI_M_m_OFFSET0	AXI_M_m_OFFSET1
 * 0	HS (CBU)	0x11111111	0x63111111	0xFEDCBA98	0x0E543210
 * 1	HS (RTT)	0x77777777	0x77777777	0xFEDCBA98	0x76543210
 * 2	AXI Tunnel	0x88888888	0x88888888	0xFEDCBA98	0x76543210
 * 3	HDMI-VIDEO	0x77777777	0x77777777	0xFEDCBA98	0x76543210
 * 4	HDMI-ADUIO	0x77777777	0x77777777	0xFEDCBA98	0x76543210
 * 5	USB-HOST	0x77777777	0x77999999	0xFEDCBA98	0x76DCBA98
 * 6	ETHERNET	0x77777777	0x77999999	0xFEDCBA98	0x76DCBA98
 * 7	SDIO		0x77777777	0x77999999	0xFEDCBA98	0x76DCBA98
 * 8	GPU		0x77777777	0x77777777	0xFEDCBA98	0x76543210
 * 9	DMAC (port #1)	0x77777777	0x77777777	0xFEDCBA98	0x76543210
 * 10	DMAC (port #2)	0x77777777	0x77777777	0xFEDCBA98	0x76543210
 * 11	DVFS		0x00000000	0x60000000	0x00000000	0x00000000
 *
 * Please read ARC HS Development IC Specification, section 17.2 for more
 * information about apertures configuration.
 * NOTE: we intentionally modify default settings in U-boot. Default settings
 * are specified in "Table 111 CREG Address Decoder register reset values".
 */

#define CREG_AXI_M_SLV0(m)  ((void __iomem *)(CREG_BASE + 0x020 * (m)))
#define CREG_AXI_M_SLV1(m)  ((void __iomem *)(CREG_BASE + 0x020 * (m) + 0x004))
#define CREG_AXI_M_OFT0(m)  ((void __iomem *)(CREG_BASE + 0x020 * (m) + 0x008))
#define CREG_AXI_M_OFT1(m)  ((void __iomem *)(CREG_BASE + 0x020 * (m) + 0x00C))
#define CREG_AXI_M_UPDT(m)  ((void __iomem *)(CREG_BASE + 0x020 * (m) + 0x014))

#define CREG_AXI_M_HS_CORE_BOOT	((void __iomem *)(CREG_BASE + 0x010))

#define CREG_PAE	((void __iomem *)(CREG_BASE + 0x180))
#define CREG_PAE_UPDT	((void __iomem *)(CREG_BASE + 0x194))

void init_memory_bridge(void)
{
	u32 reg;

	/*
	 * M_HS_CORE has one unic register - BOOT.
	 * We need to clean boot mirror (BOOT[1:0]) bits in them.
	 */
	reg = readl(CREG_AXI_M_HS_CORE_BOOT) & (~0x3);
	writel(reg, CREG_AXI_M_HS_CORE_BOOT);
	writel(0x11111111, CREG_AXI_M_SLV0(M_HS_CORE));
	writel(0x63111111, CREG_AXI_M_SLV1(M_HS_CORE));
	writel(0xFEDCBA98, CREG_AXI_M_OFT0(M_HS_CORE));
	writel(0x0E543210, CREG_AXI_M_OFT1(M_HS_CORE));
	writel(UPDATE_VAL, CREG_AXI_M_UPDT(M_HS_CORE));

	writel(0x77777777, CREG_AXI_M_SLV0(M_HS_RTT));
	writel(0x77777777, CREG_AXI_M_SLV1(M_HS_RTT));
	writel(0xFEDCBA98, CREG_AXI_M_OFT0(M_HS_RTT));
	writel(0x76543210, CREG_AXI_M_OFT1(M_HS_RTT));
	writel(UPDATE_VAL, CREG_AXI_M_UPDT(M_HS_RTT));

	writel(0x88888888, CREG_AXI_M_SLV0(M_AXI_TUN));
	writel(0x88888888, CREG_AXI_M_SLV1(M_AXI_TUN));
	writel(0xFEDCBA98, CREG_AXI_M_OFT0(M_AXI_TUN));
	writel(0x76543210, CREG_AXI_M_OFT1(M_AXI_TUN));
	writel(UPDATE_VAL, CREG_AXI_M_UPDT(M_AXI_TUN));

	writel(0x77777777, CREG_AXI_M_SLV0(M_HDMI_VIDEO));
	writel(0x77777777, CREG_AXI_M_SLV1(M_HDMI_VIDEO));
	writel(0xFEDCBA98, CREG_AXI_M_OFT0(M_HDMI_VIDEO));
	writel(0x76543210, CREG_AXI_M_OFT1(M_HDMI_VIDEO));
	writel(UPDATE_VAL, CREG_AXI_M_UPDT(M_HDMI_VIDEO));

	writel(0x77777777, CREG_AXI_M_SLV0(M_HDMI_AUDIO));
	writel(0x77777777, CREG_AXI_M_SLV1(M_HDMI_AUDIO));
	writel(0xFEDCBA98, CREG_AXI_M_OFT0(M_HDMI_AUDIO));
	writel(0x76543210, CREG_AXI_M_OFT1(M_HDMI_AUDIO));
	writel(UPDATE_VAL, CREG_AXI_M_UPDT(M_HDMI_AUDIO));

	writel(0x77777777, CREG_AXI_M_SLV0(M_USB_HOST));
	writel(0x77999999, CREG_AXI_M_SLV1(M_USB_HOST));
	writel(0xFEDCBA98, CREG_AXI_M_OFT0(M_USB_HOST));
	writel(0x76DCBA98, CREG_AXI_M_OFT1(M_USB_HOST));
	writel(UPDATE_VAL, CREG_AXI_M_UPDT(M_USB_HOST));

	writel(0x77777777, CREG_AXI_M_SLV0(M_ETHERNET));
	writel(0x77999999, CREG_AXI_M_SLV1(M_ETHERNET));
	writel(0xFEDCBA98, CREG_AXI_M_OFT0(M_ETHERNET));
	writel(0x76DCBA98, CREG_AXI_M_OFT1(M_ETHERNET));
	writel(UPDATE_VAL, CREG_AXI_M_UPDT(M_ETHERNET));

	writel(0x77777777, CREG_AXI_M_SLV0(M_SDIO));
	writel(0x77999999, CREG_AXI_M_SLV1(M_SDIO));
	writel(0xFEDCBA98, CREG_AXI_M_OFT0(M_SDIO));
	writel(0x76DCBA98, CREG_AXI_M_OFT1(M_SDIO));
	writel(UPDATE_VAL, CREG_AXI_M_UPDT(M_SDIO));

	writel(0x77777777, CREG_AXI_M_SLV0(M_GPU));
	writel(0x77777777, CREG_AXI_M_SLV1(M_GPU));
	writel(0xFEDCBA98, CREG_AXI_M_OFT0(M_GPU));
	writel(0x76543210, CREG_AXI_M_OFT1(M_GPU));
	writel(UPDATE_VAL, CREG_AXI_M_UPDT(M_GPU));

	writel(0x77777777, CREG_AXI_M_SLV0(M_DMAC_0));
	writel(0x77777777, CREG_AXI_M_SLV1(M_DMAC_0));
	writel(0xFEDCBA98, CREG_AXI_M_OFT0(M_DMAC_0));
	writel(0x76543210, CREG_AXI_M_OFT1(M_DMAC_0));
	writel(UPDATE_VAL, CREG_AXI_M_UPDT(M_DMAC_0));

	writel(0x77777777, CREG_AXI_M_SLV0(M_DMAC_1));
	writel(0x77777777, CREG_AXI_M_SLV1(M_DMAC_1));
	writel(0xFEDCBA98, CREG_AXI_M_OFT0(M_DMAC_1));
	writel(0x76543210, CREG_AXI_M_OFT1(M_DMAC_1));
	writel(UPDATE_VAL, CREG_AXI_M_UPDT(M_DMAC_1));

	writel(0x00000000, CREG_AXI_M_SLV0(M_DVFS));
	writel(0x60000000, CREG_AXI_M_SLV1(M_DVFS));
	writel(0x00000000, CREG_AXI_M_OFT0(M_DVFS));
	writel(0x00000000, CREG_AXI_M_OFT1(M_DVFS));
	writel(UPDATE_VAL, CREG_AXI_M_UPDT(M_DVFS));

	writel(0x00000000, CREG_PAE);
	writel(UPDATE_VAL, CREG_PAE_UPDT);
}

static void setup_clocks(void)
{
	ulong rate;

	/* Setup CPU clock */
	if (env_common.cpu_freq.set) {
		rate = env_common.cpu_freq.val;
		soc_clk_ctl("cpu-clk", &rate, CLK_ON | CLK_SET | CLK_MHZ);
	}

	/* Setup TUN clock */
	if (env_common.tun_freq.set) {
		rate = env_common.tun_freq.val;
		if (rate)
			soc_clk_ctl("tun-clk", &rate, CLK_ON | CLK_SET | CLK_MHZ);
		else
			soc_clk_ctl("tun-clk", NULL, CLK_OFF);
	}

	if (env_common.axi_freq.set) {
		rate = env_common.axi_freq.val;
		soc_clk_ctl("axi-clk", &rate, CLK_SET | CLK_ON | CLK_MHZ);
	}
}

static void do_init_cluster(void)
{
	/*
	 * A multi-core ARC HS configuration always includes only one
	 * ARC_AUX_NON_VOLATILE_LIMIT register, which is shared by all the
	 * cores.
	 */
	init_cluster_nvlim();
}

static int check_master_cpu_id(void)
{
	if (CPU_ID_GET() == MASTER_CPU_ID)
		return 0;

	pr_err("u-boot runs on non-master cpu with id: %lu\n", CPU_ID_GET());

	return -ENOENT;
}

static noinline int prepare_cpus(void)
{
	int ret;

	ret = check_master_cpu_id();
	if (ret)
		return ret;

	ret = envs_process_and_validate(env_map_common, env_map_core, is_cpu_used);
	if (ret)
		return ret;

	printf("CPU start mask is %#x\n", env_common.core_mask.val);

	do_init_slave_cpus();
	do_init_master_cpu();
	do_init_cluster();

	return 0;
}

static int hsdk_go_run(u32 cpu_start_reg)
{
	/* Cleanup caches, disable interrupts */
	cleanup_before_go();

	if (env_common.halt_on_boot)
		halt_this_cpu();

	/*
	 * 3 NOPs after FLAG 1 instruction are no longer required for ARCv2
	 * cores but we leave them for gebug purposes.
	 */
	__builtin_arc_nop();
	__builtin_arc_nop();
	__builtin_arc_nop();

	/* Kick chosen slave CPUs */
	writel(cpu_start_reg, (void __iomem *)CREG_CPU_START);

	if (is_cpu_used(MASTER_CPU_ID))
		((void (*)(void))(env_core.entry[MASTER_CPU_ID].val))();
	else
		halt_this_cpu();

	pr_err("u-boot still runs on cpu [%ld]\n", CPU_ID_GET());

	/*
	 * We will never return after executing our program if master cpu used
	 * otherwise halt master cpu manually.
	 */
	while (true)
		halt_this_cpu();

	return 0;
}

int board_prep_linux(bootm_headers_t *images)
{
	int ret, ofst;
	char mask[15];

	ret = envs_read_validate_common(env_map_mask);
	if (ret)
		return ret;

	/* Rollback to default values */
	if (!env_common.core_mask.set) {
		env_common.core_mask.val = ALL_CPU_MASK;
		env_common.core_mask.set = true;
	}

	printf("CPU start mask is %#x\n", env_common.core_mask.val);

	if (!is_cpu_used(MASTER_CPU_ID))
		pr_err("ERR: try to launch linux with CPU[0] disabled! It doesn't work for ARC.\n");

	/*
	 * If we want to launch linux on all CPUs we don't need to patch
	 * linux DTB as it is default configuration
	 */
	if (env_common.core_mask.val == ALL_CPU_MASK)
		return 0;

	if (!IMAGE_ENABLE_OF_LIBFDT || !images->ft_len) {
		pr_err("WARN: core_mask setup will work properly only with external DTB!\n");
		return 0;
	}

	/* patch '/possible-cpus' property according to cpu mask */
	ofst = fdt_path_offset(images->ft_addr, "/");
	sprintf(mask, "%s%s%s%s",
		is_cpu_used(0) ? "0," : "",
		is_cpu_used(1) ? "1," : "",
		is_cpu_used(2) ? "2," : "",
		is_cpu_used(3) ? "3," : "");
	ret = fdt_setprop_string(images->ft_addr, ofst, "possible-cpus", mask);
	/*
	 * If we failed to patch '/possible-cpus' property we don't need break
	 * linux loading process: kernel will handle it but linux will print
	 * warning like "Timeout: CPU1 FAILED to comeup !!!".
	 * So warn here about error, but return 0 like no error had occurred.
	 */
	if (ret)
		pr_err("WARN: failed to patch '/possible-cpus' property, ret=%d\n",
		       ret);

	return 0;
}

void board_jump_and_run(ulong entry, int zero, int arch, uint params)
{
	void (*kernel_entry)(int zero, int arch, uint params);
	u32 cpu_start_reg;

	kernel_entry = (void (*)(int, int, uint))entry;

	/* Prepare CREG_CPU_START for kicking chosen CPUs */
	cpu_start_reg = prepare_cpu_ctart_reg();

	/* In case of run without hsdk_init */
	slave_cpu_set_boot_addr(entry);

	/* In case of run with hsdk_init */
	for (u32 i = 0; i < NR_CPUS; i++) {
		env_core.entry[i].val = entry;
		env_core.entry[i].set = true;
	}
	/* sync cross_cpu struct as we updated core-entry variables */
	sync_cross_cpu_data();

	/* Kick chosen slave CPUs */
	writel(cpu_start_reg, (void __iomem *)CREG_CPU_START);

	if (is_cpu_used(0))
		kernel_entry(zero, arch, params);
}

static int hsdk_go_prepare_and_run(void)
{
	/* Prepare CREG_CPU_START for kicking chosen CPUs */
	u32 reg = prepare_cpu_ctart_reg();

	if (env_common.halt_on_boot)
		printf("CPU will halt before application start, start application with debugger.\n");

	return hsdk_go_run(reg);
}

static int do_hsdk_go(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int ret;

	/*
	 * Check for 'halt' parameter. 'halt' = enter halt-mode just before
	 * starting the application; can be used for debug.
	 */
	if (argc > 1) {
		env_common.halt_on_boot = !strcmp(argv[1], "halt");
		if (!env_common.halt_on_boot) {
			pr_err("Unrecognised parameter: \'%s\'\n", argv[1]);
			return CMD_RET_FAILURE;
		}
	}

	ret = check_master_cpu_id();
	if (ret)
		return ret;

	ret = envs_process_and_validate(env_map_mask, env_map_go, is_cpu_used);
	if (ret)
		return ret;

	/* sync cross_cpu struct as we updated core-entry variables */
	sync_cross_cpu_data();

	ret = hsdk_go_prepare_and_run();

	return ret ? CMD_RET_FAILURE : CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	hsdk_go, 3, 0, do_hsdk_go,
	"Synopsys HSDK specific command",
	"     - Boot stand-alone application on HSDK\n"
	"hsdk_go halt - Boot stand-alone application on HSDK, halt CPU just before application run\n"
);

static int do_hsdk_init(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	static bool done = false;
	int ret;

	/* hsdk_init can be run only once */
	if (done) {
		printf("HSDK HW is already initialized! Please reset the board if you want to change the configuration.\n");
		return CMD_RET_FAILURE;
	}

	ret = prepare_cpus();
	if (!ret)
		done = true;

	return ret ? CMD_RET_FAILURE : CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	hsdk_init, 1, 0, do_hsdk_init,
	"Synopsys HSDK specific command",
	"- Init HSDK HW\n"
);

static int do_hsdk_clock_set(cmd_tbl_t *cmdtp, int flag, int argc,
			     char *const argv[])
{
	int ret = 0;

	/* Strip off leading subcommand argument */
	argc--;
	argv++;

	envs_cleanup_common(env_map_clock);

	if (!argc) {
		printf("Set clocks to values specified in environment\n");
		ret = envs_read_common(env_map_clock);
	} else {
		printf("Set clocks to values specified in args\n");
		ret = args_envs_enumerate(env_map_clock, 2, argc, argv);
	}

	if (ret)
		return CMD_RET_FAILURE;

	ret = envs_validate_common(env_map_clock);
	if (ret)
		return CMD_RET_FAILURE;

	/* Setup clock tree HW */
	setup_clocks();

	return CMD_RET_SUCCESS;
}

static int do_hsdk_clock_get(cmd_tbl_t *cmdtp, int flag, int argc,
			     char *const argv[])
{
	ulong rate;

	if (soc_clk_ctl("cpu-clk", &rate, CLK_GET | CLK_MHZ))
		return CMD_RET_FAILURE;

	if (env_set_ulong("cpu_freq", rate))
		return CMD_RET_FAILURE;

	if (soc_clk_ctl("tun-clk", &rate, CLK_GET | CLK_MHZ))
		return CMD_RET_FAILURE;

	if (env_set_ulong("tun_freq", rate))
		return CMD_RET_FAILURE;

	if (soc_clk_ctl("axi-clk", &rate, CLK_GET | CLK_MHZ))
		return CMD_RET_FAILURE;

	if (env_set_ulong("axi_freq", rate))
		return CMD_RET_FAILURE;

	printf("Clock values are saved to environment\n");

	return CMD_RET_SUCCESS;
}

static int do_hsdk_clock_print(cmd_tbl_t *cmdtp, int flag, int argc,
			       char *const argv[])
{
	/* Main clocks */
	soc_clk_ctl("cpu-clk", NULL, CLK_PRINT | CLK_MHZ);
	soc_clk_ctl("tun-clk", NULL, CLK_PRINT | CLK_MHZ);
	soc_clk_ctl("axi-clk", NULL, CLK_PRINT | CLK_MHZ);
	soc_clk_ctl("ddr-clk", NULL, CLK_PRINT | CLK_MHZ);

	return CMD_RET_SUCCESS;
}

static int do_hsdk_clock_print_all(cmd_tbl_t *cmdtp, int flag, int argc,
				   char *const argv[])
{
	/*
	 * NOTE: as of today we don't use some peripherals like HDMI / EBI
	 * so we don't want to print their clocks ("hdmi-sys-clk", "hdmi-pll",
	 * "hdmi-clk", "ebi-clk"). Nevertheless their clock subsystems is fully
	 * functional and we can print their clocks if it is required
	 */

	/* CPU clock domain */
	soc_clk_ctl("cpu-pll", NULL, CLK_PRINT | CLK_MHZ);
	soc_clk_ctl("cpu-clk", NULL, CLK_PRINT | CLK_MHZ);
	printf("\n");

	/* SYS clock domain */
	soc_clk_ctl("sys-pll", NULL, CLK_PRINT | CLK_MHZ);
	soc_clk_ctl("apb-clk", NULL, CLK_PRINT | CLK_MHZ);
	soc_clk_ctl("axi-clk", NULL, CLK_PRINT | CLK_MHZ);
	soc_clk_ctl("eth-clk", NULL, CLK_PRINT | CLK_MHZ);
	soc_clk_ctl("usb-clk", NULL, CLK_PRINT | CLK_MHZ);
	soc_clk_ctl("sdio-clk", NULL, CLK_PRINT | CLK_MHZ);
/*	soc_clk_ctl("hdmi-sys-clk", NULL, CLK_PRINT | CLK_MHZ); */
	soc_clk_ctl("gfx-core-clk", NULL, CLK_PRINT | CLK_MHZ);
	soc_clk_ctl("gfx-dma-clk", NULL, CLK_PRINT | CLK_MHZ);
	soc_clk_ctl("gfx-cfg-clk", NULL, CLK_PRINT | CLK_MHZ);
	soc_clk_ctl("dmac-core-clk", NULL, CLK_PRINT | CLK_MHZ);
	soc_clk_ctl("dmac-cfg-clk", NULL, CLK_PRINT | CLK_MHZ);
	soc_clk_ctl("sdio-ref-clk", NULL, CLK_PRINT | CLK_MHZ);
	soc_clk_ctl("spi-clk", NULL, CLK_PRINT | CLK_MHZ);
	soc_clk_ctl("i2c-clk", NULL, CLK_PRINT | CLK_MHZ);
/*	soc_clk_ctl("ebi-clk", NULL, CLK_PRINT | CLK_MHZ); */
	soc_clk_ctl("uart-clk", NULL, CLK_PRINT | CLK_MHZ);
	printf("\n");

	/* DDR clock domain */
	soc_clk_ctl("ddr-clk", NULL, CLK_PRINT | CLK_MHZ);
	printf("\n");

	/* HDMI clock domain */
/*	soc_clk_ctl("hdmi-pll", NULL, CLK_PRINT | CLK_MHZ); */
/*	soc_clk_ctl("hdmi-clk", NULL, CLK_PRINT | CLK_MHZ); */
/*	printf("\n"); */

	/* TUN clock domain */
	soc_clk_ctl("tun-pll", NULL, CLK_PRINT | CLK_MHZ);
	soc_clk_ctl("tun-clk", NULL, CLK_PRINT | CLK_MHZ);
	soc_clk_ctl("rom-clk", NULL, CLK_PRINT | CLK_MHZ);
	soc_clk_ctl("pwm-clk", NULL, CLK_PRINT | CLK_MHZ);
	printf("\n");

	return CMD_RET_SUCCESS;
}

cmd_tbl_t cmd_hsdk_clock[] = {
	U_BOOT_CMD_MKENT(set, 3, 0, do_hsdk_clock_set, "", ""),
	U_BOOT_CMD_MKENT(get, 3, 0, do_hsdk_clock_get, "", ""),
	U_BOOT_CMD_MKENT(print, 4, 0, do_hsdk_clock_print, "", ""),
	U_BOOT_CMD_MKENT(print_all, 4, 0, do_hsdk_clock_print_all, "", ""),
};

static int do_hsdk_clock(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	cmd_tbl_t *c;

	if (argc < 2)
		return CMD_RET_USAGE;

	/* Strip off leading 'hsdk_clock' command argument */
	argc--;
	argv++;

	c = find_cmd_tbl(argv[0], cmd_hsdk_clock, ARRAY_SIZE(cmd_hsdk_clock));
	if (!c)
		return CMD_RET_USAGE;

	return c->cmd(cmdtp, flag, argc, argv);
}

U_BOOT_CMD(
	hsdk_clock, CONFIG_SYS_MAXARGS, 0, do_hsdk_clock,
	"Synopsys HSDK specific clock command",
	"set   - Set clock to values specified in environment / command line arguments\n"
	"hsdk_clock get   - Save clock values to environment\n"
	"hsdk_clock print - Print main clock values to console\n"
	"hsdk_clock print_all - Print all clock values to console\n"
);

/* init calls */
int board_early_init_f(void)
{
	/*
	 * Setup AXI apertures unconditionally as we want to have DDR
	 * in 0x00000000 region when we are kicking slave cpus.
	 */
	init_memory_bridge();

	/*
	 * Switch SDIO external ciu clock divider from default div-by-8 to
	 * minimum possible div-by-2.
	 */
	writel(SDIO_UHS_REG_EXT_DIV_2, (void __iomem *)SDIO_UHS_REG_EXT);

	return 0;
}

int board_early_init_r(void)
{
	/*
	 * TODO: Init USB here to be able read environment from USB MSD.
	 * It can be done with usb_init() call. We can't do it right now
	 * due to brocken USB IP SW reset and lack of USB IP HW reset in
	 * linux kernel (if we init USB here we will break USB in linux)
	 */

	/*
	 * Flush all d$ as we want to use uncached area with st.di / ld.di
	 * instructions and we don't want to have any dirty line in L1d$ or SL$
	 * in this area. It is enough to flush all d$ once here as we access to
	 * uncached area with regular st (non .di) instruction only when we copy
	 * data during u-boot relocation.
	 */
	flush_dcache_all();

	printf("Relocation Offset is: %08lx\n", gd->reloc_off);

	return 0;
}

int board_late_init(void)
{
	/*
	 * Populate environment with clock frequency values -
	 * run hsdk_clock get callback without uboot command run.
	 */
	do_hsdk_clock_get(NULL, 0, 0, NULL);

	return 0;
}

int checkboard(void)
{
	puts("Board: Synopsys ARC HS Development Kit\n");
	return 0;
};
