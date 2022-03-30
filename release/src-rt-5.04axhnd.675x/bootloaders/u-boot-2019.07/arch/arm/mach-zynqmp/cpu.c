// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014 - 2015 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 */

#include <common.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <asm/armv8/mmu.h>
#include <asm/io.h>

#define ZYNQ_SILICON_VER_MASK	0xF000
#define ZYNQ_SILICON_VER_SHIFT	12

DECLARE_GLOBAL_DATA_PTR;

/*
 * Number of filled static entries and also the first empty
 * slot in zynqmp_mem_map.
 */
#define ZYNQMP_MEM_MAP_USED	4

#if !defined(CONFIG_ZYNQMP_NO_DDR)
#define DRAM_BANKS CONFIG_NR_DRAM_BANKS
#else
#define DRAM_BANKS 0
#endif

#if defined(CONFIG_DEFINE_TCM_OCM_MMAP)
#define TCM_MAP 1
#else
#define TCM_MAP 0
#endif

/* +1 is end of list which needs to be empty */
#define ZYNQMP_MEM_MAP_MAX (ZYNQMP_MEM_MAP_USED + DRAM_BANKS + TCM_MAP + 1)

static struct mm_region zynqmp_mem_map[ZYNQMP_MEM_MAP_MAX] = {
	{
		.virt = 0x80000000UL,
		.phys = 0x80000000UL,
		.size = 0x70000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		.virt = 0xf8000000UL,
		.phys = 0xf8000000UL,
		.size = 0x07e00000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		.virt = 0x400000000UL,
		.phys = 0x400000000UL,
		.size = 0x400000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		.virt = 0x1000000000UL,
		.phys = 0x1000000000UL,
		.size = 0xf000000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}
};

void mem_map_fill(void)
{
	int banks = ZYNQMP_MEM_MAP_USED;

#if defined(CONFIG_DEFINE_TCM_OCM_MMAP)
	zynqmp_mem_map[banks].virt = 0xffe00000UL;
	zynqmp_mem_map[banks].phys = 0xffe00000UL;
	zynqmp_mem_map[banks].size = 0x00200000UL;
	zynqmp_mem_map[banks].attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
				      PTE_BLOCK_INNER_SHARE;
	banks = banks + 1;
#endif

#if !defined(CONFIG_ZYNQMP_NO_DDR)
	for (int i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		/* Zero size means no more DDR that's this is end */
		if (!gd->bd->bi_dram[i].size)
			break;

		zynqmp_mem_map[banks].virt = gd->bd->bi_dram[i].start;
		zynqmp_mem_map[banks].phys = gd->bd->bi_dram[i].start;
		zynqmp_mem_map[banks].size = gd->bd->bi_dram[i].size;
		zynqmp_mem_map[banks].attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
					      PTE_BLOCK_INNER_SHARE;
		banks = banks + 1;
	}
#endif
}

struct mm_region *mem_map = zynqmp_mem_map;

u64 get_page_table_size(void)
{
	return 0x14000;
}

#if defined(CONFIG_SYS_MEM_RSVD_FOR_MMU) || defined(CONFIG_DEFINE_TCM_OCM_MMAP)
void tcm_init(u8 mode)
{
	puts("WARNING: Initializing TCM overwrites TCM content\n");
	initialize_tcm(mode);
	memset((void *)ZYNQMP_TCM_BASE_ADDR, 0, ZYNQMP_TCM_SIZE);
}
#endif

#ifdef CONFIG_SYS_MEM_RSVD_FOR_MMU
int reserve_mmu(void)
{
	tcm_init(TCM_LOCK);
	gd->arch.tlb_size = PGTABLE_SIZE;
	gd->arch.tlb_addr = ZYNQMP_TCM_BASE_ADDR;

	return 0;
}
#endif

static unsigned int zynqmp_get_silicon_version_secure(void)
{
	u32 ver;

	ver = readl(&csu_base->version);
	ver &= ZYNQMP_SILICON_VER_MASK;
	ver >>= ZYNQMP_SILICON_VER_SHIFT;

	return ver;
}

unsigned int zynqmp_get_silicon_version(void)
{
	if (current_el() == 3)
		return zynqmp_get_silicon_version_secure();

	gd->cpu_clk = get_tbclk();

	switch (gd->cpu_clk) {
	case 50000000:
		return ZYNQMP_CSU_VERSION_QEMU;
	}

	return ZYNQMP_CSU_VERSION_SILICON;
}

#define ZYNQMP_MMIO_READ	0xC2000014
#define ZYNQMP_MMIO_WRITE	0xC2000013

int __maybe_unused invoke_smc(u32 pm_api_id, u32 arg0, u32 arg1, u32 arg2,
			      u32 arg3, u32 *ret_payload)
{
	/*
	 * Added SIP service call Function Identifier
	 * Make sure to stay in x0 register
	 */
	struct pt_regs regs;

	regs.regs[0] = pm_api_id;
	regs.regs[1] = ((u64)arg1 << 32) | arg0;
	regs.regs[2] = ((u64)arg3 << 32) | arg2;

	smc_call(&regs);

	if (ret_payload != NULL) {
		ret_payload[0] = (u32)regs.regs[0];
		ret_payload[1] = upper_32_bits(regs.regs[0]);
		ret_payload[2] = (u32)regs.regs[1];
		ret_payload[3] = upper_32_bits(regs.regs[1]);
		ret_payload[4] = (u32)regs.regs[2];
	}

	return regs.regs[0];
}

unsigned int  __maybe_unused zynqmp_pmufw_version(void)
{
	int ret;
	u32 ret_payload[PAYLOAD_ARG_CNT];
	static u32 pm_api_version = ZYNQMP_PM_VERSION_INVALID;

	/*
	 * Get PMU version only once and later
	 * just return stored values instead of
	 * asking PMUFW again.
	 */
	if (pm_api_version == ZYNQMP_PM_VERSION_INVALID) {
		ret = invoke_smc(ZYNQMP_SIP_SVC_GET_API_VERSION, 0, 0, 0, 0,
				 ret_payload);
		pm_api_version = ret_payload[1];

		if (ret)
			panic("PMUFW is not found - Please load it!\n");
	}

	return pm_api_version;
}

static int zynqmp_mmio_rawwrite(const u32 address,
		      const u32 mask,
		      const u32 value)
{
	u32 data;
	u32 value_local = value;
	int ret;

	ret = zynqmp_mmio_read(address, &data);
	if (ret)
		return ret;

	data &= ~mask;
	value_local &= mask;
	value_local |= data;
	writel(value_local, (ulong)address);
	return 0;
}

static int zynqmp_mmio_rawread(const u32 address, u32 *value)
{
	*value = readl((ulong)address);
	return 0;
}

int zynqmp_mmio_write(const u32 address,
		      const u32 mask,
		      const u32 value)
{
	if (IS_ENABLED(CONFIG_SPL_BUILD) || current_el() == 3)
		return zynqmp_mmio_rawwrite(address, mask, value);
	else
		return invoke_smc(ZYNQMP_MMIO_WRITE, address, mask,
				  value, 0, NULL);

	return -EINVAL;
}

int zynqmp_mmio_read(const u32 address, u32 *value)
{
	u32 ret_payload[PAYLOAD_ARG_CNT];
	u32 ret;

	if (!value)
		return -EINVAL;

	if (IS_ENABLED(CONFIG_SPL_BUILD) || current_el() == 3) {
		ret = zynqmp_mmio_rawread(address, value);
	} else {
		ret = invoke_smc(ZYNQMP_MMIO_READ, address, 0, 0,
				 0, ret_payload);
		*value = ret_payload[1];
	}

	return ret;
}
