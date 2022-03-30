// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2007,2009-2014 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <command.h>
#include <pci.h>
#include <asm/processor.h>
#include <asm/mmu.h>
#include <asm/fsl_pci.h>
#include <asm/io.h>
#include <linux/libfdt.h>
#include <fdt_support.h>
#include <netdev.h>
#include <fdtdec.h>
#include <errno.h>
#include <malloc.h>

DECLARE_GLOBAL_DATA_PTR;

static void *get_fdt_virt(void)
{
	return (void *)CONFIG_SYS_TMPVIRT;
}

static uint64_t get_fdt_phys(void)
{
	return (uint64_t)(uintptr_t)gd->fdt_blob;
}

static void map_fdt_as(int esel)
{
	u32 mas0, mas1, mas2, mas3, mas7;
	uint64_t fdt_phys = get_fdt_phys();
	unsigned long fdt_phys_tlb = fdt_phys & ~0xffffful;
	unsigned long fdt_virt_tlb = (ulong)get_fdt_virt() & ~0xffffful;

	mas0 = MAS0_TLBSEL(1) | MAS0_ESEL(esel);
	mas1 = MAS1_VALID | MAS1_TID(0) | MAS1_TS | MAS1_TSIZE(BOOKE_PAGESZ_1M);
	mas2 = FSL_BOOKE_MAS2(fdt_virt_tlb, 0);
	mas3 = FSL_BOOKE_MAS3(fdt_phys_tlb, 0, MAS3_SW|MAS3_SR);
	mas7 = FSL_BOOKE_MAS7(fdt_phys_tlb);

	write_tlb(mas0, mas1, mas2, mas3, mas7);
}

uint64_t get_phys_ccsrbar_addr_early(void)
{
	void *fdt = get_fdt_virt();
	uint64_t r;
	int size, node;
	u32 naddr;
	const fdt32_t *prop;

	/*
	 * To be able to read the FDT we need to create a temporary TLB
	 * map for it.
	 */
	map_fdt_as(10);
	node = fdt_path_offset(fdt, "/soc");
	naddr = fdt_address_cells(fdt, node);
	prop = fdt_getprop(fdt, node, "ranges", &size);
	r = fdt_translate_address(fdt, node, prop + naddr);
	disable_tlb(10);

	return r;
}

int board_early_init_f(void)
{
	return 0;
}

int checkboard(void)
{
	return 0;
}

static int pci_map_region(void *fdt, int pci_node, int range_id,
			  phys_size_t *ppaddr, pci_addr_t *pvaddr,
			  pci_size_t *psize, ulong *pmap_addr)
{
	uint64_t addr;
	uint64_t size;
	ulong map_addr;
	int r;

	r = fdt_read_range(fdt, pci_node, range_id, NULL, &addr, &size);
	if (r)
		return r;

	if (ppaddr)
		*ppaddr = addr;
	if (psize)
		*psize = size;

	if (!pmap_addr)
		return 0;

	map_addr = *pmap_addr;

	/* Align map_addr */
	map_addr += size - 1;
	map_addr &= ~(size - 1);

	if (map_addr + size >= CONFIG_SYS_PCI_MAP_END)
		return -1;

	/* Map virtual memory for range */
	assert(!tlb_map_range(map_addr, addr, size, TLB_MAP_IO));
	*pmap_addr = map_addr + size;

	if (pvaddr)
		*pvaddr = map_addr;

	return 0;
}

void pci_init_board(void)
{
	struct pci_controller *pci_hoses;
	void *fdt = get_fdt_virt();
	int pci_node = -1;
	int pci_num = 0;
	int pci_count = 0;
	ulong map_addr;

	puts("\n");

	/* Start MMIO and PIO range maps above RAM */
	map_addr = CONFIG_SYS_PCI_MAP_START;

	/* Count and allocate PCI buses */
	pci_node = fdt_node_offset_by_prop_value(fdt, pci_node,
			"device_type", "pci", 4);
	while (pci_node != -FDT_ERR_NOTFOUND) {
		pci_node = fdt_node_offset_by_prop_value(fdt, pci_node,
				"device_type", "pci", 4);
		pci_count++;
	}

	if (pci_count) {
		pci_hoses = malloc(sizeof(struct pci_controller) * pci_count);
	} else {
		printf("PCI: disabled\n\n");
		return;
	}

	/* Spawn PCI buses based on device tree */
	pci_node = fdt_node_offset_by_prop_value(fdt, pci_node,
			"device_type", "pci", 4);
	while (pci_node != -FDT_ERR_NOTFOUND) {
		struct fsl_pci_info pci_info = { };
		const fdt32_t *reg;
		int r;

		reg = fdt_getprop(fdt, pci_node, "reg", NULL);
		pci_info.regs = fdt_translate_address(fdt, pci_node, reg);

		/* Map MMIO range */
		r = pci_map_region(fdt, pci_node, 0, &pci_info.mem_phys, NULL,
				   &pci_info.mem_size, &map_addr);
		if (r)
			break;

		/* Map PIO range */
		r = pci_map_region(fdt, pci_node, 1, &pci_info.io_phys, NULL,
				   &pci_info.io_size, &map_addr);
		if (r)
			break;

		/*
		 * The PCI framework finds virtual addresses for the buses
		 * through our address map, so tell it the physical addresses.
		 */
		pci_info.mem_bus = pci_info.mem_phys;
		pci_info.io_bus = pci_info.io_phys;

		/* Instantiate */
		pci_info.pci_num = pci_num + 1;

		fsl_setup_hose(&pci_hoses[pci_num], pci_info.regs);
		printf("PCI: base address %lx\n", pci_info.regs);

		fsl_pci_init_port(&pci_info, &pci_hoses[pci_num], pci_num);

		/* Jump to next PCI node */
		pci_node = fdt_node_offset_by_prop_value(fdt, pci_node,
				"device_type", "pci", 4);
		pci_num++;
	}

	puts("\n");
}

int last_stage_init(void)
{
	void *fdt = get_fdt_virt();
	int len = 0;
	const uint64_t *prop;
	int chosen;

	chosen = fdt_path_offset(fdt, "/chosen");
	if (chosen < 0) {
		printf("Couldn't find /chosen node in fdt\n");
		return -EIO;
	}

	/* -kernel boot */
	prop = fdt_getprop(fdt, chosen, "qemu,boot-kernel", &len);
	if (prop && (len >= 8))
		env_set_hex("qemu_kernel_addr", *prop);

	/* Give the user a variable for the host fdt */
	env_set_hex("fdt_addr_r", (ulong)fdt);

	return 0;
}

static uint64_t get_linear_ram_size(void)
{
	void *fdt = get_fdt_virt();
	const void *prop;
	int memory;
	int len;

	memory = fdt_path_offset(fdt, "/memory");
	prop = fdt_getprop(fdt, memory, "reg", &len);

	if (prop && len >= 16)
		return *(uint64_t *)(prop+8);

	panic("Couldn't determine RAM size");
}

int board_eth_init(bd_t *bis)
{
	return pci_eth_init(bis);
}

#if defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, bd_t *bd)
{
	FT_FSL_PCI_SETUP;

	return 0;
}
#endif

void print_laws(void)
{
	/* We don't emulate LAWs yet */
}

phys_size_t fixed_sdram(void)
{
	return get_linear_ram_size();
}

phys_size_t fsl_ddr_sdram_size(void)
{
	return get_linear_ram_size();
}

void init_tlbs(void)
{
	phys_size_t ram_size;

	/*
	 * Create a temporary AS=1 map for the fdt
	 *
	 * We use ESEL=0 here to overwrite the previous AS=0 map for ourselves
	 * which was only 4k big. This way we don't have to clear any other maps.
	 */
	map_fdt_as(0);

	/* Fetch RAM size from the fdt */
	ram_size = get_linear_ram_size();

	/* And remove our fdt map again */
	disable_tlb(0);

	/* Create an internal map of manually created TLB maps */
	init_used_tlb_cams();

	/* Create a dynamic AS=0 CCSRBAR mapping */
	assert(!tlb_map_range(CONFIG_SYS_CCSRBAR, CONFIG_SYS_CCSRBAR_PHYS,
			      1024 * 1024, TLB_MAP_IO));

	/* Create a RAM map that spans all accessible RAM */
	setup_ddr_tlbs(ram_size >> 20);

	/* Create a map for the TLB */
	assert(!tlb_map_range((ulong)get_fdt_virt(), get_fdt_phys(),
			      1024 * 1024, TLB_MAP_RAM));
}

void init_laws(void)
{
	/* We don't emulate LAWs yet */
}

static uint32_t get_cpu_freq(void)
{
	void *fdt = get_fdt_virt();
	int cpus_node = fdt_path_offset(fdt, "/cpus");
	int cpu_node = fdt_first_subnode(fdt, cpus_node);
	const char *prop = "clock-frequency";
	return fdt_getprop_u32_default_node(fdt, cpu_node, 0, prop, 0);
}

void get_sys_info(sys_info_t *sys_info)
{
	int freq = get_cpu_freq();

	memset(sys_info, 0, sizeof(sys_info_t));
	sys_info->freq_systembus = freq;
	sys_info->freq_ddrbus = freq;
	sys_info->freq_processor[0] = freq;
}

int get_clocks (void)
{
	sys_info_t sys_info;

	get_sys_info(&sys_info);

	gd->cpu_clk = sys_info.freq_processor[0];
	gd->bus_clk = sys_info.freq_systembus;
	gd->mem_clk = sys_info.freq_ddrbus;
	gd->arch.lbc_clk = sys_info.freq_ddrbus;

	return 0;
}

unsigned long get_tbclk (void)
{
	void *fdt = get_fdt_virt();
	int cpus_node = fdt_path_offset(fdt, "/cpus");
	int cpu_node = fdt_first_subnode(fdt, cpus_node);
	const char *prop = "timebase-frequency";
	return fdt_getprop_u32_default_node(fdt, cpu_node, 0, prop, 0);
}

/********************************************
 * get_bus_freq
 * return system bus freq in Hz
 *********************************************/
ulong get_bus_freq (ulong dummy)
{
	sys_info_t sys_info;
	get_sys_info(&sys_info);
	return sys_info.freq_systembus;
}

/*
 * Return the number of cores on this SOC.
 */
int cpu_numcores(void)
{
	/*
	 * The QEMU u-boot target only needs to drive the first core,
	 * spinning and device tree nodes get driven by QEMU itself
	 */
	return 1;
}

/*
 * Return a 32-bit mask indicating which cores are present on this SOC.
 */
u32 cpu_mask(void)
{
	return (1 << cpu_numcores()) - 1;
}
