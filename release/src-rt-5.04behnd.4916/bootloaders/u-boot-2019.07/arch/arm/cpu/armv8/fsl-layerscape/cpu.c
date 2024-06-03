// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2017 NXP
 * Copyright 2014-2015 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <fsl_ddr_sdram.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <asm/system.h>
#include <fm_eth.h>
#include <asm/armv8/mmu.h>
#include <asm/io.h>
#include <asm/arch/fsl_serdes.h>
#include <asm/arch/soc.h>
#include <asm/arch/cpu.h>
#include <asm/arch/speed.h>
#include <fsl_immap.h>
#include <asm/arch/mp.h>
#include <efi_loader.h>
#include <fsl-mc/fsl_mc.h>
#ifdef CONFIG_FSL_ESDHC
#include <fsl_esdhc.h>
#endif
#include <asm/armv8/sec_firmware.h>
#ifdef CONFIG_SYS_FSL_DDR
#include <fsl_ddr.h>
#endif
#include <asm/arch/clock.h>
#include <hwconfig.h>
#include <fsl_qbman.h>

#ifdef CONFIG_TFABOOT
#include <environment.h>
#ifdef CONFIG_CHAIN_OF_TRUST
#include <fsl_validate.h>
#endif
#endif

DECLARE_GLOBAL_DATA_PTR;

static struct cpu_type cpu_type_list[] = {
	CPU_TYPE_ENTRY(LS2080A, LS2080A, 8),
	CPU_TYPE_ENTRY(LS2085A, LS2085A, 8),
	CPU_TYPE_ENTRY(LS2045A, LS2045A, 4),
	CPU_TYPE_ENTRY(LS2088A, LS2088A, 8),
	CPU_TYPE_ENTRY(LS2084A, LS2084A, 8),
	CPU_TYPE_ENTRY(LS2048A, LS2048A, 4),
	CPU_TYPE_ENTRY(LS2044A, LS2044A, 4),
	CPU_TYPE_ENTRY(LS2081A, LS2081A, 8),
	CPU_TYPE_ENTRY(LS2041A, LS2041A, 4),
	CPU_TYPE_ENTRY(LS1043A, LS1043A, 4),
	CPU_TYPE_ENTRY(LS1043A, LS1043A_P23, 4),
	CPU_TYPE_ENTRY(LS1023A, LS1023A, 2),
	CPU_TYPE_ENTRY(LS1023A, LS1023A_P23, 2),
	CPU_TYPE_ENTRY(LS1046A, LS1046A, 4),
	CPU_TYPE_ENTRY(LS1026A, LS1026A, 2),
	CPU_TYPE_ENTRY(LS2040A, LS2040A, 4),
	CPU_TYPE_ENTRY(LS1012A, LS1012A, 1),
	CPU_TYPE_ENTRY(LS1028A, LS1028A, 2),
	CPU_TYPE_ENTRY(LS1088A, LS1088A, 8),
	CPU_TYPE_ENTRY(LS1084A, LS1084A, 8),
	CPU_TYPE_ENTRY(LS1048A, LS1048A, 4),
	CPU_TYPE_ENTRY(LS1044A, LS1044A, 4),
	CPU_TYPE_ENTRY(LX2160A, LX2160A, 16),
	CPU_TYPE_ENTRY(LX2120A, LX2120A, 12),
	CPU_TYPE_ENTRY(LX2080A, LX2080A, 8),
};

#define EARLY_PGTABLE_SIZE 0x5000
static struct mm_region early_map[] = {
#ifdef CONFIG_FSL_LSCH3
	{ CONFIG_SYS_FSL_CCSR_BASE, CONFIG_SYS_FSL_CCSR_BASE,
	  CONFIG_SYS_FSL_CCSR_SIZE,
	  PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
	  PTE_BLOCK_NON_SHARE | PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
	{ CONFIG_SYS_FSL_OCRAM_BASE, CONFIG_SYS_FSL_OCRAM_BASE,
	  SYS_FSL_OCRAM_SPACE_SIZE,
	  PTE_BLOCK_MEMTYPE(MT_NORMAL) | PTE_BLOCK_NON_SHARE
	},
	{ CONFIG_SYS_FSL_QSPI_BASE1, CONFIG_SYS_FSL_QSPI_BASE1,
	  CONFIG_SYS_FSL_QSPI_SIZE1,
	  PTE_BLOCK_MEMTYPE(MT_NORMAL) | PTE_BLOCK_NON_SHARE},
#ifdef CONFIG_FSL_IFC
	/* For IFC Region #1, only the first 4MB is cache-enabled */
	{ CONFIG_SYS_FSL_IFC_BASE1, CONFIG_SYS_FSL_IFC_BASE1,
	  CONFIG_SYS_FSL_IFC_SIZE1_1,
	  PTE_BLOCK_MEMTYPE(MT_NORMAL) | PTE_BLOCK_NON_SHARE
	},
	{ CONFIG_SYS_FSL_IFC_BASE1 + CONFIG_SYS_FSL_IFC_SIZE1_1,
	  CONFIG_SYS_FSL_IFC_BASE1 + CONFIG_SYS_FSL_IFC_SIZE1_1,
	  CONFIG_SYS_FSL_IFC_SIZE1 - CONFIG_SYS_FSL_IFC_SIZE1_1,
	  PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) | PTE_BLOCK_NON_SHARE
	},
	{ CONFIG_SYS_FLASH_BASE, CONFIG_SYS_FSL_IFC_BASE1,
	  CONFIG_SYS_FSL_IFC_SIZE1,
	  PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) | PTE_BLOCK_NON_SHARE
	},
#endif
	{ CONFIG_SYS_FSL_DRAM_BASE1, CONFIG_SYS_FSL_DRAM_BASE1,
	  CONFIG_SYS_FSL_DRAM_SIZE1,
#if defined(CONFIG_TFABOOT) || \
	(defined(CONFIG_SPL) && !defined(CONFIG_SPL_BUILD))
	  PTE_BLOCK_MEMTYPE(MT_NORMAL) |
#else	/* Start with nGnRnE and PXN and UXN to prevent speculative access */
	  PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) | PTE_BLOCK_PXN | PTE_BLOCK_UXN |
#endif
	  PTE_BLOCK_OUTER_SHARE | PTE_BLOCK_NS
	},
#ifdef CONFIG_FSL_IFC
	/* Map IFC region #2 up to CONFIG_SYS_FLASH_BASE for NAND boot */
	{ CONFIG_SYS_FSL_IFC_BASE2, CONFIG_SYS_FSL_IFC_BASE2,
	  CONFIG_SYS_FLASH_BASE - CONFIG_SYS_FSL_IFC_BASE2,
	  PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) | PTE_BLOCK_NON_SHARE
	},
#endif
	{ CONFIG_SYS_FSL_DCSR_BASE, CONFIG_SYS_FSL_DCSR_BASE,
	  CONFIG_SYS_FSL_DCSR_SIZE,
	  PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
	  PTE_BLOCK_NON_SHARE | PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
	{ CONFIG_SYS_FSL_DRAM_BASE2, CONFIG_SYS_FSL_DRAM_BASE2,
	  CONFIG_SYS_FSL_DRAM_SIZE2,
	  PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) | PTE_BLOCK_PXN | PTE_BLOCK_UXN |
	  PTE_BLOCK_OUTER_SHARE | PTE_BLOCK_NS
	},
#ifdef CONFIG_SYS_FSL_DRAM_BASE3
	{ CONFIG_SYS_FSL_DRAM_BASE3, CONFIG_SYS_FSL_DRAM_BASE3,
	  CONFIG_SYS_FSL_DRAM_SIZE3,
	  PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) | PTE_BLOCK_PXN | PTE_BLOCK_UXN |
	  PTE_BLOCK_OUTER_SHARE | PTE_BLOCK_NS
	},
#endif
#elif defined(CONFIG_FSL_LSCH2)
	{ CONFIG_SYS_FSL_CCSR_BASE, CONFIG_SYS_FSL_CCSR_BASE,
	  CONFIG_SYS_FSL_CCSR_SIZE,
	  PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
	  PTE_BLOCK_NON_SHARE | PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
	{ CONFIG_SYS_FSL_OCRAM_BASE, CONFIG_SYS_FSL_OCRAM_BASE,
	  SYS_FSL_OCRAM_SPACE_SIZE,
	  PTE_BLOCK_MEMTYPE(MT_NORMAL) | PTE_BLOCK_NON_SHARE
	},
	{ CONFIG_SYS_FSL_DCSR_BASE, CONFIG_SYS_FSL_DCSR_BASE,
	  CONFIG_SYS_FSL_DCSR_SIZE,
	  PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
	  PTE_BLOCK_NON_SHARE | PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
	{ CONFIG_SYS_FSL_QSPI_BASE, CONFIG_SYS_FSL_QSPI_BASE,
	  CONFIG_SYS_FSL_QSPI_SIZE,
	  PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) | PTE_BLOCK_NON_SHARE
	},
#ifdef CONFIG_FSL_IFC
	{ CONFIG_SYS_FSL_IFC_BASE, CONFIG_SYS_FSL_IFC_BASE,
	  CONFIG_SYS_FSL_IFC_SIZE,
	  PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) | PTE_BLOCK_NON_SHARE
	},
#endif
	{ CONFIG_SYS_FSL_DRAM_BASE1, CONFIG_SYS_FSL_DRAM_BASE1,
	  CONFIG_SYS_FSL_DRAM_SIZE1,
#if defined(CONFIG_TFABOOT) || \
	(defined(CONFIG_SPL) && !defined(CONFIG_SPL_BUILD))
	  PTE_BLOCK_MEMTYPE(MT_NORMAL) |
#else	/* Start with nGnRnE and PXN and UXN to prevent speculative access */
	  PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) | PTE_BLOCK_PXN | PTE_BLOCK_UXN |
#endif
	  PTE_BLOCK_OUTER_SHARE | PTE_BLOCK_NS
	},
	{ CONFIG_SYS_FSL_DRAM_BASE2, CONFIG_SYS_FSL_DRAM_BASE2,
	  CONFIG_SYS_FSL_DRAM_SIZE2,
	  PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) | PTE_BLOCK_PXN | PTE_BLOCK_UXN |
	  PTE_BLOCK_OUTER_SHARE | PTE_BLOCK_NS
	},
#endif
	{},	/* list terminator */
};

static struct mm_region final_map[] = {
#ifdef CONFIG_FSL_LSCH3
	{ CONFIG_SYS_FSL_CCSR_BASE, CONFIG_SYS_FSL_CCSR_BASE,
	  CONFIG_SYS_FSL_CCSR_SIZE,
	  PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
	  PTE_BLOCK_NON_SHARE | PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
	{ CONFIG_SYS_FSL_OCRAM_BASE, CONFIG_SYS_FSL_OCRAM_BASE,
	  SYS_FSL_OCRAM_SPACE_SIZE,
	  PTE_BLOCK_MEMTYPE(MT_NORMAL) | PTE_BLOCK_NON_SHARE
	},
	{ CONFIG_SYS_FSL_DRAM_BASE1, CONFIG_SYS_FSL_DRAM_BASE1,
	  CONFIG_SYS_FSL_DRAM_SIZE1,
	  PTE_BLOCK_MEMTYPE(MT_NORMAL) |
	  PTE_BLOCK_OUTER_SHARE | PTE_BLOCK_NS
	},
	{ CONFIG_SYS_FSL_QSPI_BASE1, CONFIG_SYS_FSL_QSPI_BASE1,
	  CONFIG_SYS_FSL_QSPI_SIZE1,
	  PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
	  PTE_BLOCK_NON_SHARE | PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
	{ CONFIG_SYS_FSL_QSPI_BASE2, CONFIG_SYS_FSL_QSPI_BASE2,
	  CONFIG_SYS_FSL_QSPI_SIZE2,
	  PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
	  PTE_BLOCK_NON_SHARE | PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
#ifdef CONFIG_FSL_IFC
	{ CONFIG_SYS_FSL_IFC_BASE2, CONFIG_SYS_FSL_IFC_BASE2,
	  CONFIG_SYS_FSL_IFC_SIZE2,
	  PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
	  PTE_BLOCK_NON_SHARE | PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
#endif
	{ CONFIG_SYS_FSL_DCSR_BASE, CONFIG_SYS_FSL_DCSR_BASE,
	  CONFIG_SYS_FSL_DCSR_SIZE,
	  PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
	  PTE_BLOCK_NON_SHARE | PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
	{ CONFIG_SYS_FSL_MC_BASE, CONFIG_SYS_FSL_MC_BASE,
	  CONFIG_SYS_FSL_MC_SIZE,
	  PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
	  PTE_BLOCK_NON_SHARE | PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
	{ CONFIG_SYS_FSL_NI_BASE, CONFIG_SYS_FSL_NI_BASE,
	  CONFIG_SYS_FSL_NI_SIZE,
	  PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
	  PTE_BLOCK_NON_SHARE | PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
	/* For QBMAN portal, only the first 64MB is cache-enabled */
	{ CONFIG_SYS_FSL_QBMAN_BASE, CONFIG_SYS_FSL_QBMAN_BASE,
	  CONFIG_SYS_FSL_QBMAN_SIZE_1,
	  PTE_BLOCK_MEMTYPE(MT_NORMAL) |
	  PTE_BLOCK_NON_SHARE | PTE_BLOCK_PXN | PTE_BLOCK_UXN | PTE_BLOCK_NS
	},
	{ CONFIG_SYS_FSL_QBMAN_BASE + CONFIG_SYS_FSL_QBMAN_SIZE_1,
	  CONFIG_SYS_FSL_QBMAN_BASE + CONFIG_SYS_FSL_QBMAN_SIZE_1,
	  CONFIG_SYS_FSL_QBMAN_SIZE - CONFIG_SYS_FSL_QBMAN_SIZE_1,
	  PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
	  PTE_BLOCK_NON_SHARE | PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
	{ CONFIG_SYS_PCIE1_PHYS_ADDR, CONFIG_SYS_PCIE1_PHYS_ADDR,
	  CONFIG_SYS_PCIE1_PHYS_SIZE,
	  PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
	  PTE_BLOCK_NON_SHARE | PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
	{ CONFIG_SYS_PCIE2_PHYS_ADDR, CONFIG_SYS_PCIE2_PHYS_ADDR,
	  CONFIG_SYS_PCIE2_PHYS_SIZE,
	  PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
	  PTE_BLOCK_NON_SHARE | PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
#ifdef CONFIG_SYS_PCIE3_PHYS_ADDR
	{ CONFIG_SYS_PCIE3_PHYS_ADDR, CONFIG_SYS_PCIE3_PHYS_ADDR,
	  CONFIG_SYS_PCIE3_PHYS_SIZE,
	  PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
	  PTE_BLOCK_NON_SHARE | PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
#endif
#ifdef CONFIG_SYS_PCIE4_PHYS_ADDR
	{ CONFIG_SYS_PCIE4_PHYS_ADDR, CONFIG_SYS_PCIE4_PHYS_ADDR,
	  CONFIG_SYS_PCIE4_PHYS_SIZE,
	  PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
	  PTE_BLOCK_NON_SHARE | PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
#endif
#ifdef SYS_PCIE5_PHYS_ADDR
	{ SYS_PCIE5_PHYS_ADDR, SYS_PCIE5_PHYS_ADDR,
	  SYS_PCIE5_PHYS_SIZE,
	  PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
	  PTE_BLOCK_NON_SHARE | PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
#endif
#ifdef SYS_PCIE6_PHYS_ADDR
	{ SYS_PCIE6_PHYS_ADDR, SYS_PCIE6_PHYS_ADDR,
	  SYS_PCIE6_PHYS_SIZE,
	  PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
	  PTE_BLOCK_NON_SHARE | PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
#endif
	{ CONFIG_SYS_FSL_WRIOP1_BASE, CONFIG_SYS_FSL_WRIOP1_BASE,
	  CONFIG_SYS_FSL_WRIOP1_SIZE,
	  PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
	  PTE_BLOCK_NON_SHARE | PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
	{ CONFIG_SYS_FSL_AIOP1_BASE, CONFIG_SYS_FSL_AIOP1_BASE,
	  CONFIG_SYS_FSL_AIOP1_SIZE,
	  PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
	  PTE_BLOCK_NON_SHARE | PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
	{ CONFIG_SYS_FSL_PEBUF_BASE, CONFIG_SYS_FSL_PEBUF_BASE,
	  CONFIG_SYS_FSL_PEBUF_SIZE,
	  PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
	  PTE_BLOCK_NON_SHARE | PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
	{ CONFIG_SYS_FSL_DRAM_BASE2, CONFIG_SYS_FSL_DRAM_BASE2,
	  CONFIG_SYS_FSL_DRAM_SIZE2,
	  PTE_BLOCK_MEMTYPE(MT_NORMAL) |
	  PTE_BLOCK_OUTER_SHARE | PTE_BLOCK_NS
	},
#ifdef CONFIG_SYS_FSL_DRAM_BASE3
	{ CONFIG_SYS_FSL_DRAM_BASE3, CONFIG_SYS_FSL_DRAM_BASE3,
	  CONFIG_SYS_FSL_DRAM_SIZE3,
	  PTE_BLOCK_MEMTYPE(MT_NORMAL) |
	  PTE_BLOCK_OUTER_SHARE | PTE_BLOCK_NS
	},
#endif
#elif defined(CONFIG_FSL_LSCH2)
	{ CONFIG_SYS_FSL_BOOTROM_BASE, CONFIG_SYS_FSL_BOOTROM_BASE,
	  CONFIG_SYS_FSL_BOOTROM_SIZE,
	  PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
	  PTE_BLOCK_NON_SHARE | PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
	{ CONFIG_SYS_FSL_CCSR_BASE, CONFIG_SYS_FSL_CCSR_BASE,
	  CONFIG_SYS_FSL_CCSR_SIZE,
	  PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
	  PTE_BLOCK_NON_SHARE | PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
	{ CONFIG_SYS_FSL_OCRAM_BASE, CONFIG_SYS_FSL_OCRAM_BASE,
	  SYS_FSL_OCRAM_SPACE_SIZE,
	  PTE_BLOCK_MEMTYPE(MT_NORMAL) | PTE_BLOCK_NON_SHARE
	},
	{ CONFIG_SYS_FSL_DCSR_BASE, CONFIG_SYS_FSL_DCSR_BASE,
	  CONFIG_SYS_FSL_DCSR_SIZE,
	  PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
	  PTE_BLOCK_NON_SHARE | PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
	{ CONFIG_SYS_FSL_QSPI_BASE, CONFIG_SYS_FSL_QSPI_BASE,
	  CONFIG_SYS_FSL_QSPI_SIZE,
	  PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
	  PTE_BLOCK_NON_SHARE | PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
#ifdef CONFIG_FSL_IFC
	{ CONFIG_SYS_FSL_IFC_BASE, CONFIG_SYS_FSL_IFC_BASE,
	  CONFIG_SYS_FSL_IFC_SIZE,
	  PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) | PTE_BLOCK_NON_SHARE
	},
#endif
	{ CONFIG_SYS_FSL_DRAM_BASE1, CONFIG_SYS_FSL_DRAM_BASE1,
	  CONFIG_SYS_FSL_DRAM_SIZE1,
	  PTE_BLOCK_MEMTYPE(MT_NORMAL) |
	  PTE_BLOCK_OUTER_SHARE | PTE_BLOCK_NS
	},
	{ CONFIG_SYS_FSL_QBMAN_BASE, CONFIG_SYS_FSL_QBMAN_BASE,
	  CONFIG_SYS_FSL_QBMAN_SIZE,
	  PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
	  PTE_BLOCK_NON_SHARE | PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
	{ CONFIG_SYS_FSL_DRAM_BASE2, CONFIG_SYS_FSL_DRAM_BASE2,
	  CONFIG_SYS_FSL_DRAM_SIZE2,
	  PTE_BLOCK_MEMTYPE(MT_NORMAL) |
	  PTE_BLOCK_OUTER_SHARE | PTE_BLOCK_NS
	},
	{ CONFIG_SYS_PCIE1_PHYS_ADDR, CONFIG_SYS_PCIE1_PHYS_ADDR,
	  CONFIG_SYS_PCIE1_PHYS_SIZE,
	  PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
	  PTE_BLOCK_NON_SHARE | PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
	{ CONFIG_SYS_PCIE2_PHYS_ADDR, CONFIG_SYS_PCIE2_PHYS_ADDR,
	  CONFIG_SYS_PCIE2_PHYS_SIZE,
	  PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
	  PTE_BLOCK_NON_SHARE | PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
#ifdef CONFIG_SYS_PCIE3_PHYS_ADDR
	{ CONFIG_SYS_PCIE3_PHYS_ADDR, CONFIG_SYS_PCIE3_PHYS_ADDR,
	  CONFIG_SYS_PCIE3_PHYS_SIZE,
	  PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
	  PTE_BLOCK_NON_SHARE | PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
#endif
	{ CONFIG_SYS_FSL_DRAM_BASE3, CONFIG_SYS_FSL_DRAM_BASE3,
	  CONFIG_SYS_FSL_DRAM_SIZE3,
	  PTE_BLOCK_MEMTYPE(MT_NORMAL) |
	  PTE_BLOCK_OUTER_SHARE | PTE_BLOCK_NS
	},
#endif
#ifdef CONFIG_SYS_MEM_RESERVE_SECURE
	{},	/* space holder for secure mem */
#endif
	{},
};

struct mm_region *mem_map = early_map;

void cpu_name(char *name)
{
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	unsigned int i, svr, ver;

	svr = gur_in32(&gur->svr);
	ver = SVR_SOC_VER(svr);

	for (i = 0; i < ARRAY_SIZE(cpu_type_list); i++)
		if ((cpu_type_list[i].soc_ver & SVR_WO_E) == ver) {
			strcpy(name, cpu_type_list[i].name);
#ifdef CONFIG_ARCH_LX2160A
			if (IS_C_PROCESSOR(svr))
				strcat(name, "C");
#endif

			if (IS_E_PROCESSOR(svr))
				strcat(name, "E");

			sprintf(name + strlen(name), " Rev%d.%d",
				SVR_MAJ(svr), SVR_MIN(svr));
			break;
		}

	if (i == ARRAY_SIZE(cpu_type_list))
		strcpy(name, "unknown");
}

#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
/*
 * To start MMU before DDR is available, we create MMU table in SRAM.
 * The base address of SRAM is CONFIG_SYS_FSL_OCRAM_BASE. We use three
 * levels of translation tables here to cover 40-bit address space.
 * We use 4KB granule size, with 40 bits physical address, T0SZ=24
 * Address above EARLY_PGTABLE_SIZE (0x5000) is free for other purpose.
 * Note, the debug print in cache_v8.c is not usable for debugging
 * these early MMU tables because UART is not yet available.
 */
static inline void early_mmu_setup(void)
{
	unsigned int el = current_el();

	/* global data is already setup, no allocation yet */
	if (el == 3)
		gd->arch.tlb_addr = CONFIG_SYS_FSL_OCRAM_BASE;
	else
		gd->arch.tlb_addr = CONFIG_SYS_DDR_SDRAM_BASE;
	gd->arch.tlb_fillptr = gd->arch.tlb_addr;
	gd->arch.tlb_size = EARLY_PGTABLE_SIZE;

	/* Create early page tables */
	setup_pgtables();

	/* point TTBR to the new table */
	set_ttbr_tcr_mair(el, gd->arch.tlb_addr,
			  get_tcr(el, NULL, NULL) &
			  ~(TCR_ORGN_MASK | TCR_IRGN_MASK),
			  MEMORY_ATTRIBUTES);

	set_sctlr(get_sctlr() | CR_M);
}

static void fix_pcie_mmu_map(void)
{
#ifdef CONFIG_ARCH_LS2080A
	unsigned int i;
	u32 svr, ver;
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);

	svr = gur_in32(&gur->svr);
	ver = SVR_SOC_VER(svr);

	/* Fix PCIE base and size for LS2088A */
	if ((ver == SVR_LS2088A) || (ver == SVR_LS2084A) ||
	    (ver == SVR_LS2048A) || (ver == SVR_LS2044A) ||
	    (ver == SVR_LS2081A) || (ver == SVR_LS2041A)) {
		for (i = 0; i < ARRAY_SIZE(final_map); i++) {
			switch (final_map[i].phys) {
			case CONFIG_SYS_PCIE1_PHYS_ADDR:
				final_map[i].phys = 0x2000000000ULL;
				final_map[i].virt = 0x2000000000ULL;
				final_map[i].size = 0x800000000ULL;
				break;
			case CONFIG_SYS_PCIE2_PHYS_ADDR:
				final_map[i].phys = 0x2800000000ULL;
				final_map[i].virt = 0x2800000000ULL;
				final_map[i].size = 0x800000000ULL;
				break;
#ifdef CONFIG_SYS_PCIE3_PHYS_ADDR
			case CONFIG_SYS_PCIE3_PHYS_ADDR:
				final_map[i].phys = 0x3000000000ULL;
				final_map[i].virt = 0x3000000000ULL;
				final_map[i].size = 0x800000000ULL;
				break;
#endif
#ifdef CONFIG_SYS_PCIE4_PHYS_ADDR
			case CONFIG_SYS_PCIE4_PHYS_ADDR:
				final_map[i].phys = 0x3800000000ULL;
				final_map[i].virt = 0x3800000000ULL;
				final_map[i].size = 0x800000000ULL;
				break;
#endif
			default:
				break;
			}
		}
	}
#endif
}

/*
 * The final tables look similar to early tables, but different in detail.
 * These tables are in DRAM. Sub tables are added to enable cache for
 * QBMan and OCRAM.
 *
 * Put the MMU table in secure memory if gd->arch.secure_ram is valid.
 * OCRAM will be not used for this purpose so gd->arch.secure_ram can't be 0.
 */
static inline void final_mmu_setup(void)
{
	u64 tlb_addr_save = gd->arch.tlb_addr;
	unsigned int el = current_el();
	int index;

	/* fix the final_map before filling in the block entries */
	fix_pcie_mmu_map();

	mem_map = final_map;

	/* Update mapping for DDR to actual size */
	for (index = 0; index < ARRAY_SIZE(final_map) - 2; index++) {
		/*
		 * Find the entry for DDR mapping and update the address and
		 * size. Zero-sized mapping will be skipped when creating MMU
		 * table.
		 */
		switch (final_map[index].virt) {
		case CONFIG_SYS_FSL_DRAM_BASE1:
			final_map[index].virt = gd->bd->bi_dram[0].start;
			final_map[index].phys = gd->bd->bi_dram[0].start;
			final_map[index].size = gd->bd->bi_dram[0].size;
			break;
#ifdef CONFIG_SYS_FSL_DRAM_BASE2
		case CONFIG_SYS_FSL_DRAM_BASE2:
#if (CONFIG_NR_DRAM_BANKS >= 2)
			final_map[index].virt = gd->bd->bi_dram[1].start;
			final_map[index].phys = gd->bd->bi_dram[1].start;
			final_map[index].size = gd->bd->bi_dram[1].size;
#else
			final_map[index].size = 0;
#endif
		break;
#endif
#ifdef CONFIG_SYS_FSL_DRAM_BASE3
		case CONFIG_SYS_FSL_DRAM_BASE3:
#if (CONFIG_NR_DRAM_BANKS >= 3)
			final_map[index].virt = gd->bd->bi_dram[2].start;
			final_map[index].phys = gd->bd->bi_dram[2].start;
			final_map[index].size = gd->bd->bi_dram[2].size;
#else
			final_map[index].size = 0;
#endif
		break;
#endif
		default:
			break;
		}
	}

#ifdef CONFIG_SYS_MEM_RESERVE_SECURE
	if (gd->arch.secure_ram & MEM_RESERVE_SECURE_MAINTAINED) {
		if (el == 3) {
			/*
			 * Only use gd->arch.secure_ram if the address is
			 * recalculated. Align to 4KB for MMU table.
			 */
			/* put page tables in secure ram */
			index = ARRAY_SIZE(final_map) - 2;
			gd->arch.tlb_addr = gd->arch.secure_ram & ~0xfff;
			final_map[index].virt = gd->arch.secure_ram & ~0x3;
			final_map[index].phys = final_map[index].virt;
			final_map[index].size = CONFIG_SYS_MEM_RESERVE_SECURE;
			final_map[index].attrs = PTE_BLOCK_OUTER_SHARE;
			gd->arch.secure_ram |= MEM_RESERVE_SECURE_SECURED;
			tlb_addr_save = gd->arch.tlb_addr;
		} else {
			/* Use allocated (board_f.c) memory for TLB */
			tlb_addr_save = gd->arch.tlb_allocated;
			gd->arch.tlb_addr = tlb_addr_save;
		}
	}
#endif

	/* Reset the fill ptr */
	gd->arch.tlb_fillptr = tlb_addr_save;

	/* Create normal system page tables */
	setup_pgtables();

	/* Create emergency page tables */
	gd->arch.tlb_addr = gd->arch.tlb_fillptr;
	gd->arch.tlb_emerg = gd->arch.tlb_addr;
	setup_pgtables();
	gd->arch.tlb_addr = tlb_addr_save;

	/* Disable cache and MMU */
	dcache_disable();	/* TLBs are invalidated */
	invalidate_icache_all();

	/* point TTBR to the new table */
	set_ttbr_tcr_mair(el, gd->arch.tlb_addr, get_tcr(el, NULL, NULL),
			  MEMORY_ATTRIBUTES);

	set_sctlr(get_sctlr() | CR_M);
}

u64 get_page_table_size(void)
{
	return 0x10000;
}

int arch_cpu_init(void)
{
	/*
	 * This function is called before U-Boot relocates itself to speed up
	 * on system running. It is not necessary to run if performance is not
	 * critical. Skip if MMU is already enabled by SPL or other means.
	 */
	if (get_sctlr() & CR_M)
		return 0;

	icache_enable();
	__asm_invalidate_dcache_all();
	__asm_invalidate_tlb_all();
	early_mmu_setup();
	set_sctlr(get_sctlr() | CR_C);
	return 0;
}

void mmu_setup(void)
{
	final_mmu_setup();
}

/*
 * This function is called from common/board_r.c.
 * It recreates MMU table in main memory.
 */
void enable_caches(void)
{
	mmu_setup();
	__asm_invalidate_tlb_all();
	icache_enable();
	dcache_enable();
}
#endif	/* !CONFIG_IS_ENABLED(SYS_DCACHE_OFF) */

#ifdef CONFIG_TFABOOT
enum boot_src __get_boot_src(u32 porsr1)
{
	enum boot_src src = BOOT_SOURCE_RESERVED;
	u32 rcw_src = (porsr1 & RCW_SRC_MASK) >> RCW_SRC_BIT;
#if !defined(CONFIG_NXP_LSCH3_2)
	u32 val;
#endif
	debug("%s: rcw_src 0x%x\n", __func__, rcw_src);

#if defined(CONFIG_FSL_LSCH3)
#if defined(CONFIG_NXP_LSCH3_2)
	switch (rcw_src) {
	case RCW_SRC_SDHC1_VAL:
		src = BOOT_SOURCE_SD_MMC;
	break;
	case RCW_SRC_SDHC2_VAL:
		src = BOOT_SOURCE_SD_MMC2;
	break;
	case RCW_SRC_I2C1_VAL:
		src = BOOT_SOURCE_I2C1_EXTENDED;
	break;
	case RCW_SRC_FLEXSPI_NAND2K_VAL:
		src = BOOT_SOURCE_XSPI_NAND;
	break;
	case RCW_SRC_FLEXSPI_NAND4K_VAL:
		src = BOOT_SOURCE_XSPI_NAND;
	break;
	case RCW_SRC_RESERVED_1_VAL:
		src = BOOT_SOURCE_RESERVED;
	break;
	case RCW_SRC_FLEXSPI_NOR_24B:
		src = BOOT_SOURCE_XSPI_NOR;
	break;
	default:
		src = BOOT_SOURCE_RESERVED;
	}
#else
	val = rcw_src & RCW_SRC_TYPE_MASK;
	if (val == RCW_SRC_NOR_VAL) {
		val = rcw_src & NOR_TYPE_MASK;

		switch (val) {
		case NOR_16B_VAL:
		case NOR_32B_VAL:
			src = BOOT_SOURCE_IFC_NOR;
		break;
		default:
			src = BOOT_SOURCE_RESERVED;
		}
	} else {
		/* RCW SRC Serial Flash */
		val = rcw_src & RCW_SRC_SERIAL_MASK;
		switch (val) {
		case RCW_SRC_QSPI_VAL:
		/* RCW SRC Serial NOR (QSPI) */
			src = BOOT_SOURCE_QSPI_NOR;
			break;
		case RCW_SRC_SD_CARD_VAL:
		/* RCW SRC SD Card */
			src = BOOT_SOURCE_SD_MMC;
			break;
		case RCW_SRC_EMMC_VAL:
		/* RCW SRC EMMC */
			src = BOOT_SOURCE_SD_MMC;
			break;
		case RCW_SRC_I2C1_VAL:
		/* RCW SRC I2C1 Extended */
			src = BOOT_SOURCE_I2C1_EXTENDED;
			break;
		default:
			src = BOOT_SOURCE_RESERVED;
		}
	}
#endif
#elif defined(CONFIG_FSL_LSCH2)
	/* RCW SRC NAND */
	val = rcw_src & RCW_SRC_NAND_MASK;
	if (val == RCW_SRC_NAND_VAL) {
		val = rcw_src & NAND_RESERVED_MASK;
		if (val != NAND_RESERVED_1 && val != NAND_RESERVED_2)
			src = BOOT_SOURCE_IFC_NAND;

	} else {
		/* RCW SRC NOR */
		val = rcw_src & RCW_SRC_NOR_MASK;
		if (val == NOR_8B_VAL || val == NOR_16B_VAL) {
			src = BOOT_SOURCE_IFC_NOR;
		} else {
			switch (rcw_src) {
			case QSPI_VAL1:
			case QSPI_VAL2:
				src = BOOT_SOURCE_QSPI_NOR;
				break;
			case SD_VAL:
				src = BOOT_SOURCE_SD_MMC;
				break;
			default:
				src = BOOT_SOURCE_RESERVED;
			}
		}
	}
#endif

	if (CONFIG_IS_ENABLED(SYS_FSL_ERRATUM_A010539) && !rcw_src)
		src = BOOT_SOURCE_QSPI_NOR;

	debug("%s: src 0x%x\n", __func__, src);
	return src;
}

enum boot_src get_boot_src(void)
{
	struct pt_regs regs;
	u32 porsr1 = 0;

#if defined(CONFIG_FSL_LSCH3)
	u32 __iomem *dcfg_ccsr = (u32 __iomem *)DCFG_BASE;
#elif defined(CONFIG_FSL_LSCH2)
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
#endif

	if (current_el() == 2) {
		regs.regs[0] = SIP_SVC_RCW;

		smc_call(&regs);
		if (!regs.regs[0])
			porsr1 = regs.regs[1];
	}

	if (current_el() == 3 || !porsr1) {
#ifdef CONFIG_FSL_LSCH3
		porsr1 = in_le32(dcfg_ccsr + DCFG_PORSR1 / 4);
#elif defined(CONFIG_FSL_LSCH2)
		porsr1 = in_be32(&gur->porsr1);
#endif
	}

	debug("%s: porsr1 0x%x\n", __func__, porsr1);

	return __get_boot_src(porsr1);
}

#ifdef CONFIG_ENV_IS_IN_MMC
int mmc_get_env_dev(void)
{
	enum boot_src src = get_boot_src();
	int dev = CONFIG_SYS_MMC_ENV_DEV;

	switch (src) {
	case BOOT_SOURCE_SD_MMC:
		dev = 0;
		break;
	case BOOT_SOURCE_SD_MMC2:
		dev = 1;
		break;
	default:
		break;
	}

	return dev;
}
#endif

enum env_location env_get_location(enum env_operation op, int prio)
{
	enum boot_src src = get_boot_src();
	enum env_location env_loc = ENVL_NOWHERE;

	if (prio)
		return ENVL_UNKNOWN;

#ifdef	CONFIG_ENV_IS_NOWHERE
	return env_loc;
#endif

	switch (src) {
	case BOOT_SOURCE_IFC_NOR:
		env_loc = ENVL_FLASH;
		break;
	case BOOT_SOURCE_QSPI_NOR:
		/* FALLTHROUGH */
	case BOOT_SOURCE_XSPI_NOR:
		env_loc = ENVL_SPI_FLASH;
		break;
	case BOOT_SOURCE_IFC_NAND:
		/* FALLTHROUGH */
	case BOOT_SOURCE_QSPI_NAND:
		/* FALLTHROUGH */
	case BOOT_SOURCE_XSPI_NAND:
		env_loc = ENVL_NAND;
		break;
	case BOOT_SOURCE_SD_MMC:
		/* FALLTHROUGH */
	case BOOT_SOURCE_SD_MMC2:
		env_loc =  ENVL_MMC;
		break;
	case BOOT_SOURCE_I2C1_EXTENDED:
		/* FALLTHROUGH */
	default:
		break;
	}

	return env_loc;
}
#endif	/* CONFIG_TFABOOT */

u32 initiator_type(u32 cluster, int init_id)
{
	struct ccsr_gur *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	u32 idx = (cluster >> (init_id * 8)) & TP_CLUSTER_INIT_MASK;
	u32 type = 0;

	type = gur_in32(&gur->tp_ityp[idx]);
	if (type & TP_ITYP_AV)
		return type;

	return 0;
}

u32 cpu_pos_mask(void)
{
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	int i = 0;
	u32 cluster, type, mask = 0;

	do {
		int j;

		cluster = gur_in32(&gur->tp_cluster[i].lower);
		for (j = 0; j < TP_INIT_PER_CLUSTER; j++) {
			type = initiator_type(cluster, j);
			if (type && (TP_ITYP_TYPE(type) == TP_ITYP_TYPE_ARM))
				mask |= 1 << (i * TP_INIT_PER_CLUSTER + j);
		}
		i++;
	} while ((cluster & TP_CLUSTER_EOC) == 0x0);

	return mask;
}

u32 cpu_mask(void)
{
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	int i = 0, count = 0;
	u32 cluster, type, mask = 0;

	do {
		int j;

		cluster = gur_in32(&gur->tp_cluster[i].lower);
		for (j = 0; j < TP_INIT_PER_CLUSTER; j++) {
			type = initiator_type(cluster, j);
			if (type) {
				if (TP_ITYP_TYPE(type) == TP_ITYP_TYPE_ARM)
					mask |= 1 << count;
				count++;
			}
		}
		i++;
	} while ((cluster & TP_CLUSTER_EOC) == 0x0);

	return mask;
}

/*
 * Return the number of cores on this SOC.
 */
int cpu_numcores(void)
{
	return hweight32(cpu_mask());
}

int fsl_qoriq_core_to_cluster(unsigned int core)
{
	struct ccsr_gur __iomem *gur =
		(void __iomem *)(CONFIG_SYS_FSL_GUTS_ADDR);
	int i = 0, count = 0;
	u32 cluster;

	do {
		int j;

		cluster = gur_in32(&gur->tp_cluster[i].lower);
		for (j = 0; j < TP_INIT_PER_CLUSTER; j++) {
			if (initiator_type(cluster, j)) {
				if (count == core)
					return i;
				count++;
			}
		}
		i++;
	} while ((cluster & TP_CLUSTER_EOC) == 0x0);

	return -1;      /* cannot identify the cluster */
}

u32 fsl_qoriq_core_to_type(unsigned int core)
{
	struct ccsr_gur __iomem *gur =
		(void __iomem *)(CONFIG_SYS_FSL_GUTS_ADDR);
	int i = 0, count = 0;
	u32 cluster, type;

	do {
		int j;

		cluster = gur_in32(&gur->tp_cluster[i].lower);
		for (j = 0; j < TP_INIT_PER_CLUSTER; j++) {
			type = initiator_type(cluster, j);
			if (type) {
				if (count == core)
					return type;
				count++;
			}
		}
		i++;
	} while ((cluster & TP_CLUSTER_EOC) == 0x0);

	return -1;      /* cannot identify the cluster */
}

#ifndef CONFIG_FSL_LSCH3
uint get_svr(void)
{
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);

	return gur_in32(&gur->svr);
}
#endif

#ifdef CONFIG_DISPLAY_CPUINFO
int print_cpuinfo(void)
{
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	struct sys_info sysinfo;
	char buf[32];
	unsigned int i, core;
	u32 type, rcw, svr = gur_in32(&gur->svr);

	puts("SoC: ");

	cpu_name(buf);
	printf(" %s (0x%x)\n", buf, svr);
	memset((u8 *)buf, 0x00, ARRAY_SIZE(buf));
	get_sys_info(&sysinfo);
	puts("Clock Configuration:");
	for_each_cpu(i, core, cpu_numcores(), cpu_mask()) {
		if (!(i % 3))
			puts("\n       ");
		type = TP_ITYP_VER(fsl_qoriq_core_to_type(core));
		printf("CPU%d(%s):%-4s MHz  ", core,
		       type == TY_ITYP_VER_A7 ? "A7 " :
		       (type == TY_ITYP_VER_A53 ? "A53" :
		       (type == TY_ITYP_VER_A57 ? "A57" :
		       (type == TY_ITYP_VER_A72 ? "A72" : "   "))),
		       strmhz(buf, sysinfo.freq_processor[core]));
	}
	/* Display platform clock as Bus frequency. */
	printf("\n       Bus:      %-4s MHz  ",
	       strmhz(buf, sysinfo.freq_systembus / CONFIG_SYS_FSL_PCLK_DIV));
	printf("DDR:      %-4s MT/s", strmhz(buf, sysinfo.freq_ddrbus));
#ifdef CONFIG_SYS_DPAA_FMAN
	printf("  FMAN:     %-4s MHz", strmhz(buf, sysinfo.freq_fman[0]));
#endif
#ifdef CONFIG_SYS_FSL_HAS_DP_DDR
	if (soc_has_dp_ddr()) {
		printf("     DP-DDR:   %-4s MT/s",
		       strmhz(buf, sysinfo.freq_ddrbus2));
	}
#endif
	puts("\n");

	/*
	 * Display the RCW, so that no one gets confused as to what RCW
	 * we're actually using for this boot.
	 */
	puts("Reset Configuration Word (RCW):");
	for (i = 0; i < ARRAY_SIZE(gur->rcwsr); i++) {
		rcw = gur_in32(&gur->rcwsr[i]);
		if ((i % 4) == 0)
			printf("\n       %08x:", i * 4);
		printf(" %08x", rcw);
	}
	puts("\n");

	return 0;
}
#endif

#ifdef CONFIG_FSL_ESDHC
int cpu_mmc_init(bd_t *bis)
{
	return fsl_esdhc_mmc_init(bis);
}
#endif

int cpu_eth_init(bd_t *bis)
{
	int error = 0;

#if defined(CONFIG_FSL_MC_ENET) && !defined(CONFIG_SPL_BUILD)
	error = fsl_mc_ldpaa_init(bis);
#endif
#ifdef CONFIG_FMAN_ENET
	fm_standard_init(bis);
#endif
	return error;
}

static inline int check_psci(void)
{
	unsigned int psci_ver;

	psci_ver = sec_firmware_support_psci_version();
	if (psci_ver == PSCI_INVALID_VER)
		return 1;

	return 0;
}

static void config_core_prefetch(void)
{
	char *buf = NULL;
	char buffer[HWCONFIG_BUFFER_SIZE];
	const char *prefetch_arg = NULL;
	size_t arglen;
	unsigned int mask;
	struct pt_regs regs;

	if (env_get_f("hwconfig", buffer, sizeof(buffer)) > 0)
		buf = buffer;

	prefetch_arg = hwconfig_subarg_f("core_prefetch", "disable",
					 &arglen, buf);

	if (prefetch_arg) {
		mask = simple_strtoul(prefetch_arg, NULL, 0) & 0xff;
		if (mask & 0x1) {
			printf("Core0 prefetch can't be disabled\n");
			return;
		}

#define SIP_PREFETCH_DISABLE_64 0xC200FF13
		regs.regs[0] = SIP_PREFETCH_DISABLE_64;
		regs.regs[1] = mask;
		smc_call(&regs);

		if (regs.regs[0])
			printf("Prefetch disable config failed for mask ");
		else
			printf("Prefetch disable config passed for mask ");
		printf("0x%x\n", mask);
	}
}

int arch_early_init_r(void)
{
#ifdef CONFIG_SYS_FSL_ERRATUM_A009635
	u32 svr_dev_id;
	/*
	 * erratum A009635 is valid only for LS2080A SoC and
	 * its personalitiesi
	 */
	svr_dev_id = get_svr();
	if (IS_SVR_DEV(svr_dev_id, SVR_DEV(SVR_LS2080A)))
		erratum_a009635();
#endif
#if defined(CONFIG_SYS_FSL_ERRATUM_A009942) && defined(CONFIG_SYS_FSL_DDR)
	erratum_a009942_check_cpo();
#endif
	if (check_psci()) {
		debug("PSCI: PSCI does not exist.\n");

		/* if PSCI does not exist, boot secondary cores here */
		if (fsl_layerscape_wake_seconday_cores())
			printf("Did not wake secondary cores\n");
	}

	config_core_prefetch();

#ifdef CONFIG_SYS_HAS_SERDES
	fsl_serdes_init();
#endif
#ifdef CONFIG_SYS_FSL_HAS_RGMII
	/* some dpmacs in armv8a based freescale layerscape SOCs can be
	 * configured via both serdes(sgmii, xfi, xlaui etc) bits and via
	 * EC*_PMUX(rgmii) bits in RCW.
	 * e.g. dpmac 17 and 18 in LX2160A can be configured as SGMII from
	 * serdes bits and as RGMII via EC1_PMUX/EC2_PMUX bits
	 * Now if a dpmac is enabled by serdes bits then it takes precedence
	 * over EC*_PMUX bits. i.e. in LX2160A if we select serdes protocol
	 * that configures dpmac17 as SGMII and set the EC1_PMUX as RGMII,
	 * then the dpmac is SGMII and not RGMII.
	 *
	 * Therefore, move the fsl_rgmii_init after fsl_serdes_init. in
	 * fsl_rgmii_init function of SOC, we will check if the dpmac is enabled
	 * or not? if it is (fsl_serdes_init has already enabled the dpmac),
	 * then don't enable it.
	 */
	fsl_rgmii_init();
#endif
#ifdef CONFIG_FMAN_ENET
	fman_enet_init();
#endif
#ifdef CONFIG_SYS_DPAA_QBMAN
	setup_qbman_portals();
#endif
	return 0;
}

int timer_init(void)
{
	u32 __iomem *cntcr = (u32 *)CONFIG_SYS_FSL_TIMER_ADDR;
#ifdef CONFIG_FSL_LSCH3
	u32 __iomem *cltbenr = (u32 *)CONFIG_SYS_FSL_PMU_CLTBENR;
#endif
#if defined(CONFIG_ARCH_LS2080A) || defined(CONFIG_ARCH_LS1088A)
	u32 __iomem *pctbenr = (u32 *)FSL_PMU_PCTBENR_OFFSET;
	u32 svr_dev_id;
#endif
#ifdef COUNTER_FREQUENCY_REAL
	unsigned long cntfrq = COUNTER_FREQUENCY_REAL;

	/* Update with accurate clock frequency */
	if (current_el() == 3)
		asm volatile("msr cntfrq_el0, %0" : : "r" (cntfrq) : "memory");
#endif

#ifdef CONFIG_FSL_LSCH3
	/* Enable timebase for all clusters.
	 * It is safe to do so even some clusters are not enabled.
	 */
	out_le32(cltbenr, 0xf);
#endif

#if defined(CONFIG_ARCH_LS2080A) || defined(CONFIG_ARCH_LS1088A)
	/*
	 * In certain Layerscape SoCs, the clock for each core's
	 * has an enable bit in the PMU Physical Core Time Base Enable
	 * Register (PCTBENR), which allows the watchdog to operate.
	 */
	setbits_le32(pctbenr, 0xff);
	/*
	 * For LS2080A SoC and its personalities, timer controller
	 * offset is different
	 */
	svr_dev_id = get_svr();
	if (IS_SVR_DEV(svr_dev_id, SVR_DEV(SVR_LS2080A)))
		cntcr = (u32 *)SYS_FSL_LS2080A_LS2085A_TIMER_ADDR;

#endif

	/* Enable clock for timer
	 * This is a global setting.
	 */
	out_le32(cntcr, 0x1);

	return 0;
}

__efi_runtime_data u32 __iomem *rstcr = (u32 *)CONFIG_SYS_FSL_RST_ADDR;

void __efi_runtime reset_cpu(ulong addr)
{
	u32 val;

#ifdef CONFIG_ARCH_LX2160A
	val = in_le32(rstcr);
	val |= 0x01;
	out_le32(rstcr, val);
#else
	/* Raise RESET_REQ_B */
	val = scfg_in32(rstcr);
	val |= 0x02;
	scfg_out32(rstcr, val);
#endif
}

#ifdef CONFIG_EFI_LOADER

void __efi_runtime EFIAPI efi_reset_system(
		       enum efi_reset_type reset_type,
		       efi_status_t reset_status,
		       unsigned long data_size, void *reset_data)
{
	switch (reset_type) {
	case EFI_RESET_COLD:
	case EFI_RESET_WARM:
	case EFI_RESET_PLATFORM_SPECIFIC:
		reset_cpu(0);
		break;
	case EFI_RESET_SHUTDOWN:
		/* Nothing we can do */
		break;
	}

	while (1) { }
}

efi_status_t efi_reset_system_init(void)
{
	return efi_add_runtime_mmio(&rstcr, sizeof(*rstcr));
}

#endif

/*
 * Calculate reserved memory with given memory bank
 * Return aligned memory size on success
 * Return (ram_size + needed size) for failure
 */
phys_size_t board_reserve_ram_top(phys_size_t ram_size)
{
	phys_size_t ram_top = ram_size;

#if defined(CONFIG_FSL_MC_ENET) && !defined(CONFIG_SPL_BUILD)
	ram_top = mc_get_dram_block_size();
	if (ram_top > ram_size)
		return ram_size + ram_top;

	ram_top = ram_size - ram_top;
	/* The start address of MC reserved memory needs to be aligned. */
	ram_top &= ~(CONFIG_SYS_MC_RSV_MEM_ALIGN - 1);
#endif

	return ram_size - ram_top;
}

phys_size_t get_effective_memsize(void)
{
	phys_size_t ea_size, rem = 0;

	/*
	 * For ARMv8 SoCs, DDR memory is split into two or three regions. The
	 * first region is 2GB space at 0x8000_0000. Secure memory needs to
	 * allocated from first region. If the memory extends to  the second
	 * region (or the third region if applicable), Management Complex (MC)
	 * memory should be put into the highest region, i.e. the end of DDR
	 * memory. CONFIG_MAX_MEM_MAPPED is set to the size of first region so
	 * U-Boot doesn't relocate itself into higher address. Should DDR be
	 * configured to skip the first region, this function needs to be
	 * adjusted.
	 */
	if (gd->ram_size > CONFIG_MAX_MEM_MAPPED) {
		ea_size = CONFIG_MAX_MEM_MAPPED;
		rem = gd->ram_size - ea_size;
	} else {
		ea_size = gd->ram_size;
	}

#ifdef CONFIG_SYS_MEM_RESERVE_SECURE
	/* Check if we have enough space for secure memory */
	if (ea_size > CONFIG_SYS_MEM_RESERVE_SECURE)
		ea_size -= CONFIG_SYS_MEM_RESERVE_SECURE;
	else
		printf("Error: No enough space for secure memory.\n");
#endif
	/* Check if we have enough memory for MC */
	if (rem < board_reserve_ram_top(rem)) {
		/* Not enough memory in high region to reserve */
		if (ea_size > board_reserve_ram_top(ea_size))
			ea_size -= board_reserve_ram_top(ea_size);
		else
			printf("Error: No enough space for reserved memory.\n");
	}

	return ea_size;
}

#ifdef CONFIG_TFABOOT
phys_size_t tfa_get_dram_size(void)
{
	struct pt_regs regs;
	phys_size_t dram_size = 0;

	regs.regs[0] = SMC_DRAM_BANK_INFO;
	regs.regs[1] = -1;

	smc_call(&regs);
	if (regs.regs[0])
		return 0;

	dram_size = regs.regs[1];
	return dram_size;
}

static int tfa_dram_init_banksize(void)
{
	int i = 0, ret = 0;
	struct pt_regs regs;
	phys_size_t dram_size = tfa_get_dram_size();

	debug("dram_size %llx\n", dram_size);

	if (!dram_size)
		return -EINVAL;

	do {
		regs.regs[0] = SMC_DRAM_BANK_INFO;
		regs.regs[1] = i;

		smc_call(&regs);
		if (regs.regs[0]) {
			ret = -EINVAL;
			break;
		}

		debug("bank[%d]: start %lx, size %lx\n", i, regs.regs[1],
		      regs.regs[2]);
		gd->bd->bi_dram[i].start = regs.regs[1];
		gd->bd->bi_dram[i].size = regs.regs[2];

		dram_size -= gd->bd->bi_dram[i].size;

		i++;
	} while (dram_size);

	if (i > 0)
		ret = 0;

#if defined(CONFIG_FSL_MC_ENET) && !defined(CONFIG_SPL_BUILD)
	/* Assign memory for MC */
#ifdef CONFIG_SYS_DDR_BLOCK3_BASE
	if (gd->bd->bi_dram[2].size >=
	    board_reserve_ram_top(gd->bd->bi_dram[2].size)) {
		gd->arch.resv_ram = gd->bd->bi_dram[2].start +
			    gd->bd->bi_dram[2].size -
			    board_reserve_ram_top(gd->bd->bi_dram[2].size);
	} else
#endif
	{
		if (gd->bd->bi_dram[1].size >=
		    board_reserve_ram_top(gd->bd->bi_dram[1].size)) {
			gd->arch.resv_ram = gd->bd->bi_dram[1].start +
				gd->bd->bi_dram[1].size -
				board_reserve_ram_top(gd->bd->bi_dram[1].size);
		} else if (gd->bd->bi_dram[0].size >
			   board_reserve_ram_top(gd->bd->bi_dram[0].size)) {
			gd->arch.resv_ram = gd->bd->bi_dram[0].start +
				gd->bd->bi_dram[0].size -
				board_reserve_ram_top(gd->bd->bi_dram[0].size);
		}
	}
#endif	/* CONFIG_FSL_MC_ENET */

	return ret;
}
#endif

int dram_init_banksize(void)
{
#ifdef CONFIG_SYS_DP_DDR_BASE_PHY
	phys_size_t dp_ddr_size;
#endif

#ifdef CONFIG_TFABOOT
	if (!tfa_dram_init_banksize())
		return 0;
#endif
	/*
	 * gd->ram_size has the total size of DDR memory, less reserved secure
	 * memory. The DDR extends from low region to high region(s) presuming
	 * no hole is created with DDR configuration. gd->arch.secure_ram tracks
	 * the location of secure memory. gd->arch.resv_ram tracks the location
	 * of reserved memory for Management Complex (MC). Because gd->ram_size
	 * is reduced by this function if secure memory is reserved, checking
	 * gd->arch.secure_ram should be done to avoid running it repeatedly.
	 */

#ifdef CONFIG_SYS_MEM_RESERVE_SECURE
	if (gd->arch.secure_ram & MEM_RESERVE_SECURE_MAINTAINED) {
		debug("No need to run again, skip %s\n", __func__);

		return 0;
	}
#endif

	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
	if (gd->ram_size > CONFIG_SYS_DDR_BLOCK1_SIZE) {
		gd->bd->bi_dram[0].size = CONFIG_SYS_DDR_BLOCK1_SIZE;
		gd->bd->bi_dram[1].start = CONFIG_SYS_DDR_BLOCK2_BASE;
		gd->bd->bi_dram[1].size = gd->ram_size -
					  CONFIG_SYS_DDR_BLOCK1_SIZE;
#ifdef CONFIG_SYS_DDR_BLOCK3_BASE
		if (gd->bi_dram[1].size > CONFIG_SYS_DDR_BLOCK2_SIZE) {
			gd->bd->bi_dram[2].start = CONFIG_SYS_DDR_BLOCK3_BASE;
			gd->bd->bi_dram[2].size = gd->bd->bi_dram[1].size -
						  CONFIG_SYS_DDR_BLOCK2_SIZE;
			gd->bd->bi_dram[1].size = CONFIG_SYS_DDR_BLOCK2_SIZE;
		}
#endif
	} else {
		gd->bd->bi_dram[0].size = gd->ram_size;
	}
#ifdef CONFIG_SYS_MEM_RESERVE_SECURE
	if (gd->bd->bi_dram[0].size >
				CONFIG_SYS_MEM_RESERVE_SECURE) {
		gd->bd->bi_dram[0].size -=
				CONFIG_SYS_MEM_RESERVE_SECURE;
		gd->arch.secure_ram = gd->bd->bi_dram[0].start +
				      gd->bd->bi_dram[0].size;
		gd->arch.secure_ram |= MEM_RESERVE_SECURE_MAINTAINED;
		gd->ram_size -= CONFIG_SYS_MEM_RESERVE_SECURE;
	}
#endif	/* CONFIG_SYS_MEM_RESERVE_SECURE */

#if defined(CONFIG_FSL_MC_ENET) && !defined(CONFIG_SPL_BUILD)
	/* Assign memory for MC */
#ifdef CONFIG_SYS_DDR_BLOCK3_BASE
	if (gd->bd->bi_dram[2].size >=
	    board_reserve_ram_top(gd->bd->bi_dram[2].size)) {
		gd->arch.resv_ram = gd->bd->bi_dram[2].start +
			    gd->bd->bi_dram[2].size -
			    board_reserve_ram_top(gd->bd->bi_dram[2].size);
	} else
#endif
	{
		if (gd->bd->bi_dram[1].size >=
		    board_reserve_ram_top(gd->bd->bi_dram[1].size)) {
			gd->arch.resv_ram = gd->bd->bi_dram[1].start +
				gd->bd->bi_dram[1].size -
				board_reserve_ram_top(gd->bd->bi_dram[1].size);
		} else if (gd->bd->bi_dram[0].size >
			   board_reserve_ram_top(gd->bd->bi_dram[0].size)) {
			gd->arch.resv_ram = gd->bd->bi_dram[0].start +
				gd->bd->bi_dram[0].size -
				board_reserve_ram_top(gd->bd->bi_dram[0].size);
		}
	}
#endif	/* CONFIG_FSL_MC_ENET */

#ifdef CONFIG_SYS_DP_DDR_BASE_PHY
#ifdef CONFIG_SYS_DDR_BLOCK3_BASE
#error "This SoC shouldn't have DP DDR"
#endif
	if (soc_has_dp_ddr()) {
		/* initialize DP-DDR here */
		puts("DP-DDR:  ");
		/*
		 * DDR controller use 0 as the base address for binding.
		 * It is mapped to CONFIG_SYS_DP_DDR_BASE for core to access.
		 */
		dp_ddr_size = fsl_other_ddr_sdram(CONFIG_SYS_DP_DDR_BASE_PHY,
					  CONFIG_DP_DDR_CTRL,
					  CONFIG_DP_DDR_NUM_CTRLS,
					  CONFIG_DP_DDR_DIMM_SLOTS_PER_CTLR,
					  NULL, NULL, NULL);
		if (dp_ddr_size) {
			gd->bd->bi_dram[2].start = CONFIG_SYS_DP_DDR_BASE;
			gd->bd->bi_dram[2].size = dp_ddr_size;
		} else {
			puts("Not detected");
		}
	}
#endif

#ifdef CONFIG_SYS_MEM_RESERVE_SECURE
	debug("%s is called. gd->ram_size is reduced to %lu\n",
	      __func__, (ulong)gd->ram_size);
#endif

	return 0;
}

#if CONFIG_IS_ENABLED(EFI_LOADER)
void efi_add_known_memory(void)
{
	int i;
	phys_addr_t ram_start, start;
	phys_size_t ram_size;
	u64 pages;

	/* Add RAM */
	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
#ifdef CONFIG_SYS_DP_DDR_BASE_PHY
#ifdef CONFIG_SYS_DDR_BLOCK3_BASE
#error "This SoC shouldn't have DP DDR"
#endif
		if (i == 2)
			continue;	/* skip DP-DDR */
#endif
		ram_start = gd->bd->bi_dram[i].start;
		ram_size = gd->bd->bi_dram[i].size;
#ifdef CONFIG_RESV_RAM
		if (gd->arch.resv_ram >= ram_start &&
		    gd->arch.resv_ram < ram_start + ram_size)
			ram_size = gd->arch.resv_ram - ram_start;
#endif
		start = (ram_start + EFI_PAGE_MASK) & ~EFI_PAGE_MASK;
		pages = (ram_size + EFI_PAGE_MASK) >> EFI_PAGE_SHIFT;

		efi_add_memory_map(start, pages, EFI_CONVENTIONAL_MEMORY,
				   false);
	}
}
#endif

/*
 * Before DDR size is known, early MMU table have DDR mapped as device memory
 * to avoid speculative access. To relocate U-Boot to DDR, "normal memory"
 * needs to be set for these mappings.
 * If a special case configures DDR with holes in the mapping, the holes need
 * to be marked as invalid. This is not implemented in this function.
 */
void update_early_mmu_table(void)
{
	if (!gd->arch.tlb_addr)
		return;

	if (gd->ram_size <= CONFIG_SYS_FSL_DRAM_SIZE1) {
		mmu_change_region_attr(
					CONFIG_SYS_SDRAM_BASE,
					gd->ram_size,
					PTE_BLOCK_MEMTYPE(MT_NORMAL)	|
					PTE_BLOCK_OUTER_SHARE		|
					PTE_BLOCK_NS			|
					PTE_TYPE_VALID);
	} else {
		mmu_change_region_attr(
					CONFIG_SYS_SDRAM_BASE,
					CONFIG_SYS_DDR_BLOCK1_SIZE,
					PTE_BLOCK_MEMTYPE(MT_NORMAL)	|
					PTE_BLOCK_OUTER_SHARE		|
					PTE_BLOCK_NS			|
					PTE_TYPE_VALID);
#ifdef CONFIG_SYS_DDR_BLOCK3_BASE
#ifndef CONFIG_SYS_DDR_BLOCK2_SIZE
#error "Missing CONFIG_SYS_DDR_BLOCK2_SIZE"
#endif
		if (gd->ram_size - CONFIG_SYS_DDR_BLOCK1_SIZE >
		    CONFIG_SYS_DDR_BLOCK2_SIZE) {
			mmu_change_region_attr(
					CONFIG_SYS_DDR_BLOCK2_BASE,
					CONFIG_SYS_DDR_BLOCK2_SIZE,
					PTE_BLOCK_MEMTYPE(MT_NORMAL)	|
					PTE_BLOCK_OUTER_SHARE		|
					PTE_BLOCK_NS			|
					PTE_TYPE_VALID);
			mmu_change_region_attr(
					CONFIG_SYS_DDR_BLOCK3_BASE,
					gd->ram_size -
					CONFIG_SYS_DDR_BLOCK1_SIZE -
					CONFIG_SYS_DDR_BLOCK2_SIZE,
					PTE_BLOCK_MEMTYPE(MT_NORMAL)	|
					PTE_BLOCK_OUTER_SHARE		|
					PTE_BLOCK_NS			|
					PTE_TYPE_VALID);
		} else
#endif
		{
			mmu_change_region_attr(
					CONFIG_SYS_DDR_BLOCK2_BASE,
					gd->ram_size -
					CONFIG_SYS_DDR_BLOCK1_SIZE,
					PTE_BLOCK_MEMTYPE(MT_NORMAL)	|
					PTE_BLOCK_OUTER_SHARE		|
					PTE_BLOCK_NS			|
					PTE_TYPE_VALID);
		}
	}
}

__weak int dram_init(void)
{
	fsl_initdram();
#if (!defined(CONFIG_SPL) && !defined(CONFIG_TFABOOT)) || \
	defined(CONFIG_SPL_BUILD)
	/* This will break-before-make MMU for DDR */
	update_early_mmu_table();
#endif

	return 0;
}
