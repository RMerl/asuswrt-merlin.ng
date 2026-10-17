// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2007,2009-2014 Freescale Semiconductor, Inc.
 * Copyright (C) 2021, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <command.h>
#include <cpu_func.h>
#include <dm.h>
#include <env.h>
#include <event.h>
#include <init.h>
#include <log.h>
#include <net.h>
#include <pci.h>
#include <time.h>
#include <dm/simple_bus.h>
#include <dm/uclass-internal.h>
#include <asm/global_data.h>
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
#include <virtio_types.h>
#include <virtio.h>

DECLARE_GLOBAL_DATA_PTR;

/* Virtual address range for PCI region maps */
#define SYS_PCI_MAP_START	0x80000000
#define SYS_PCI_MAP_END		0xe0000000

static void *get_fdt_virt(void)
{
	if (gd->flags & GD_FLG_RELOC)
		return (void *)gd->fdt_blob;
	else
		return (void *)CFG_SYS_TMPVIRT;
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

int checkboard(void)
{
	return 0;
}

static int pci_map_region(phys_addr_t paddr, phys_size_t size, ulong *pmap_addr)
{
	ulong map_addr;

	if (!pmap_addr)
		return 0;

	map_addr = *pmap_addr;

	/* Align map_addr */
	map_addr += size - 1;
	map_addr &= ~(size - 1);

	if (map_addr + size >= SYS_PCI_MAP_END)
		return -1;

	/* Map virtual memory for range */
	assert(!tlb_map_range(map_addr, paddr, size, TLB_MAP_IO));
	*pmap_addr = map_addr + size;

	return 0;
}

static void platform_bus_map_region(ulong map_addr, phys_addr_t paddr,
				    phys_size_t size)
{
	/* Align map_addr */
	map_addr += size - 1;
	map_addr &= ~(size - 1);

	/* Map virtual memory for range */
	assert(!tlb_map_range(map_addr, paddr, size, TLB_MAP_IO));
}

int misc_init_r(void)
{
	struct udevice *dev;
	struct pci_region *io;
	struct pci_region *mem;
	struct pci_region *pre;
	ulong map_addr;
	int ret;

	/* Ensure PCI is probed */
	uclass_first_device(UCLASS_PCI, &dev);

	pci_get_regions(dev, &io, &mem, &pre);

	/* Start MMIO and PIO range maps above RAM */
	map_addr = SYS_PCI_MAP_START;

	/* Map MMIO range */
	ret = pci_map_region(mem->phys_start, mem->size, &map_addr);
	if (ret)
		return ret;

	/* Map PIO range */
	ret = pci_map_region(io->phys_start, io->size, &map_addr);
	if (ret)
		return ret;

	/*
	 * Make sure virtio bus is enumerated so that peripherals
	 * on the virtio bus can be discovered by their drivers.
	 */
	virtio_init();

	/*
	 * U-Boot is relocated to RAM already, let's delete the temporary FDT
	 * virtual-physical mapping that was used in the pre-relocation phase.
	 */
	disable_tlb(find_tlb_idx((void *)CFG_SYS_TMPVIRT, 1));

	/*
	 * Detect the presence of the platform bus node, and
	 * create a virtual memory mapping for it.
	 */
	for (ret = uclass_find_first_device(UCLASS_SIMPLE_BUS, &dev);
	     dev;
	     ret = uclass_find_next_device(&dev)) {
		if (device_is_compatible(dev, "qemu,platform")) {
			struct simple_bus_plat *plat = dev_get_uclass_plat(dev);

			platform_bus_map_region(CONFIG_PLATFORM_BUS_MAP_ADDR,
						plat->target, plat->size);
			break;
		}
	}

	return 0;
}

static int last_stage_init(void)
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

	return 0;
}
EVENT_SPY_SIMPLE(EVT_LAST_STAGE_INIT, last_stage_init);

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
	assert(!tlb_map_range(CFG_SYS_CCSRBAR, CFG_SYS_CCSRBAR_PHYS,
			      1024 * 1024, TLB_MAP_IO));

	/* Create a RAM map that spans all accessible RAM */
	setup_ddr_tlbs(ram_size >> 20);

	/* Create a map for the TLB */
	assert(!tlb_map_range((ulong)get_fdt_virt(), get_fdt_phys(),
			      1024 * 1024, TLB_MAP_RAM));
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

int get_clocks(void)
{
	sys_info_t sys_info;

	get_sys_info(&sys_info);

	gd->cpu_clk = sys_info.freq_processor[0];
	gd->bus_clk = sys_info.freq_systembus;
	gd->mem_clk = sys_info.freq_ddrbus;
	gd->arch.lbc_clk = sys_info.freq_ddrbus;

	return 0;
}

unsigned long get_tbclk(void)
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
ulong get_bus_freq(ulong dummy)
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
	 * The QEMU U-Boot target only needs to drive the first core,
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

/**
 * Return the virtual address of FDT that was passed by QEMU
 *
 * Return: virtual address of FDT received from QEMU in r3 register
 */
void *board_fdt_blob_setup(int *err)
{
	*err = 0;
	return get_fdt_virt();
}

/* See CFG_SYS_NS16550_CLK in arch/powerpc/include/asm/config.h */
int get_serial_clock(void)
{
	return get_bus_freq(0);
}
