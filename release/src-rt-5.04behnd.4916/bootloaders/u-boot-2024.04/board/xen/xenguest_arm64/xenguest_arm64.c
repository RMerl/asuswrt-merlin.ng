// SPDX-License-Identifier: GPL-2.0
/*
 * (C) 2013
 * David Feng <fenghua@phytium.com.cn>
 * Sharma Bhupesh <bhupesh.sharma@freescale.com>
 *
 * (C) 2020 EPAM Systems Inc
 */

#include <common.h>
#include <log.h>
#include <cpu_func.h>
#include <dm.h>
#include <errno.h>
#include <malloc.h>
#include <xen.h>
#include <asm/global_data.h>
#include <virtio_types.h>
#include <virtio.h>

#include <asm/io.h>
#include <asm/armv8/mmu.h>
#include <asm/xen.h>
#include <asm/xen/hypercall.h>
#include <asm/xen/system.h>

#include <linux/compiler.h>

#include <xen/gnttab.h>
#include <xen/hvm.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	return 0;
}

/*
 * Use fdt provided by Xen: according to
 * https://www.kernel.org/doc/Documentation/arm64/booting.txt
 * x0 is the physical address of the device tree blob (dtb) in system RAM.
 * This is stored in rom_pointer during low level init.
 */
void *board_fdt_blob_setup(int *err)
{
	*err = 0;
	if (fdt_magic(rom_pointer[0]) != FDT_MAGIC) {
		*err = -ENXIO;
		return NULL;
	}
	return (void *)rom_pointer[0];
}

/*
 * MAX_MEM_MAP_REGIONS should respect to:
 * 3 Xen related regions
 * 6 regions for 2 PCI Host bridges
 * 10 regions for MMIO devices
 * 2 memory regions
 */
#define MAX_MEM_MAP_REGIONS 22
static struct mm_region xen_mem_map[MAX_MEM_MAP_REGIONS];
struct mm_region *mem_map = xen_mem_map;

static int get_next_memory_node(const void *blob, int mem)
{
	do {
		mem = fdt_node_offset_by_prop_value(blob, mem,
						    "device_type", "memory", 7);
	} while (!fdtdec_get_is_enabled(blob, mem));

	return mem;
}

#ifdef CONFIG_VIRTIO_BLK
#ifdef CONFIG_VIRTIO_PCI
static void add_pci_mem_map(const void *blob, int *cnt)
{
	struct fdt_resource reg_res;
	int node = -1, len = 0, cells_per_record = 0, max_regions = 0;
	int pci_addr_cells = 0, addr_cells = 0, size_cells = 0;

	while ((node = fdt_node_offset_by_prop_value(blob, node, "compatible",
						     "pci-host-ecam-generic",
						     sizeof("pci-host-ecam-generic"))) >= 0) {
		if ((*cnt) >= MAX_MEM_MAP_REGIONS ||
		    fdt_get_resource(blob, node, "reg", 0, &reg_res) < 0)
			return;

		xen_mem_map[*cnt].virt = reg_res.start;
		xen_mem_map[*cnt].phys = reg_res.start;
		xen_mem_map[*cnt].size = fdt_resource_size(&reg_res);
		xen_mem_map[*cnt].attrs = (PTE_BLOCK_MEMTYPE(MT_NORMAL) |
								   PTE_BLOCK_INNER_SHARE);
		(*cnt)++;

		const u32 *prop = fdt_getprop(blob, node, "ranges", &len);

		if (!prop)
			return;

		pci_addr_cells =  fdt_address_cells(blob, node);
		addr_cells = fdt_address_cells(blob, 0);
		size_cells = fdt_size_cells(blob, node);

		/* PCI addresses are always 3-cells */
		len /= sizeof(u32);
		cells_per_record = pci_addr_cells + addr_cells + size_cells;
		max_regions = len / cells_per_record + CONFIG_NR_DRAM_BANKS;

		for (int i = 0; i < max_regions; i++, len -= cells_per_record) {
			u64 pci_addr, addr, size;
			int space_code;
			u32 flags;

			if (((*cnt) >= MAX_MEM_MAP_REGIONS) || len < cells_per_record)
				return;

			flags = fdt32_to_cpu(prop[0]);
			space_code = (flags >> 24) & 3;
			pci_addr = fdtdec_get_number(prop + 1, 2);
			prop += pci_addr_cells;
			addr = fdtdec_get_number(prop, addr_cells);
			prop += addr_cells;
			size = fdtdec_get_number(prop, size_cells);
			prop += size_cells;

			xen_mem_map[*cnt].virt = addr;
			xen_mem_map[*cnt].phys = addr;
			xen_mem_map[*cnt].size = size;
			xen_mem_map[*cnt].attrs = (PTE_BLOCK_MEMTYPE(MT_NORMAL) |
						   PTE_BLOCK_INNER_SHARE);
			(*cnt)++;
		}
	}
}
#endif

#ifdef CONFIG_VIRTIO_MMIO
static void add_mmio_mem_map(const void *blob, int *cnt)
{
	int node = -1;
	struct fdt_resource reg_res;

	if ((*cnt) >= MAX_MEM_MAP_REGIONS)
		return;
	while ((node = fdt_node_offset_by_prop_value(blob, node, "compatible", "virtio,mmio",
						     sizeof("virtio,mmio"))) >= 0) {
		if (fdt_get_resource(blob, node, "reg", 0, &reg_res) < 0)
			return;
		xen_mem_map[*cnt].virt = reg_res.start;
		xen_mem_map[*cnt].phys = reg_res.start;
		xen_mem_map[*cnt].size = roundup(fdt_resource_size(&reg_res), PAGE_SIZE);
		xen_mem_map[*cnt].attrs = (PTE_BLOCK_MEMTYPE(MT_NORMAL) |
				PTE_BLOCK_INNER_SHARE);
		(*cnt)++;
	}
}
#endif
#endif

static int setup_mem_map(void)
{
	int i = 0, ret, mem, reg = 0;
	struct fdt_resource res;
	const void *blob = gd->fdt_blob;
	u64 gfn;
	phys_addr_t gnttab_base;
	phys_size_t gnttab_sz;

	memset(xen_mem_map, 0, sizeof(xen_mem_map));
	/*
	 * Add "magic" region which is used by Xen to provide some essentials
	 * for the guest: we need console and xenstore.
	 */
	ret = hvm_get_parameter_maintain_dcache(HVM_PARAM_CONSOLE_PFN, &gfn);
	if (ret < 0) {
		printf("%s: Can't get HVM_PARAM_CONSOLE_PFN, ret %d\n",
		       __func__, ret);
		return -EINVAL;
	}

	xen_mem_map[i].virt = PFN_PHYS(gfn);
	xen_mem_map[i].phys = PFN_PHYS(gfn);
	xen_mem_map[i].size = PAGE_SIZE;
	xen_mem_map[i].attrs = (PTE_BLOCK_MEMTYPE(MT_NORMAL) |
				PTE_BLOCK_INNER_SHARE);
	i++;

	ret = hvm_get_parameter_maintain_dcache(HVM_PARAM_STORE_PFN, &gfn);
	if (ret < 0) {
		printf("%s: Can't get HVM_PARAM_STORE_PFN, ret %d\n",
		       __func__, ret);
		return -EINVAL;
	}

	xen_mem_map[i].virt = PFN_PHYS(gfn);
	xen_mem_map[i].phys = PFN_PHYS(gfn);
	xen_mem_map[i].size = PAGE_SIZE;
	xen_mem_map[i].attrs = (PTE_BLOCK_MEMTYPE(MT_NORMAL) |
				PTE_BLOCK_INNER_SHARE);
	i++;

	/* Get Xen's suggested physical page assignments for the grant table. */
	get_gnttab_base(&gnttab_base, &gnttab_sz);

	xen_mem_map[i].virt = gnttab_base;
	xen_mem_map[i].phys = gnttab_base;
	xen_mem_map[i].size = gnttab_sz;
	xen_mem_map[i].attrs = (PTE_BLOCK_MEMTYPE(MT_NORMAL) |
				PTE_BLOCK_INNER_SHARE);
	i++;

	mem = get_next_memory_node(blob, -1);
	if (mem < 0) {
		printf("%s: Missing /memory node\n", __func__);
		return -EINVAL;
	}

	for (; i < MAX_MEM_MAP_REGIONS; i++) {
		ret = fdt_get_resource(blob, mem, "reg", reg++, &res);
		if (ret == -FDT_ERR_NOTFOUND) {
			reg = 0;
			mem = get_next_memory_node(blob, mem);
			if (mem == -FDT_ERR_NOTFOUND)
				break;

			ret = fdt_get_resource(blob, mem, "reg", reg++, &res);
			if (ret == -FDT_ERR_NOTFOUND)
				break;
		}
		if (ret != 0) {
			printf("No reg property for memory node\n");
			return -EINVAL;
		}

		xen_mem_map[i].virt = (phys_addr_t)res.start;
		xen_mem_map[i].phys = (phys_addr_t)res.start;
		xen_mem_map[i].size = (phys_size_t)(res.end - res.start + 1);
		xen_mem_map[i].attrs = (PTE_BLOCK_MEMTYPE(MT_NORMAL) |
					PTE_BLOCK_INNER_SHARE);
	}
#ifdef CONFIG_VIRTIO_BLK
#ifdef CONFIG_VIRTIO_PCI
	add_pci_mem_map(blob, &i);
#endif
#ifdef CONFIG_VIRTIO_MMIO
	add_mmio_mem_map(blob, &i);
#endif
#endif
	return 0;
}

void enable_caches(void)
{
	/* Re-setup the memory map as BSS gets cleared after relocation. */
	setup_mem_map();
	icache_enable();
	dcache_enable();
}

/* Read memory settings from the Xen provided device tree. */
int dram_init(void)
{
	int ret;

	ret = fdtdec_setup_mem_size_base();
	if (ret < 0)
		return ret;
	/* Setup memory map, so MMU page table size can be estimated. */
	return setup_mem_map();
}

int dram_init_banksize(void)
{
	return fdtdec_setup_memory_banksize();
}

/*
 * Board specific reset that is system reset.
 */
void reset_cpu(void)
{
}

int ft_system_setup(void *blob, struct bd_info *bd)
{
	return 0;
}

int ft_board_setup(void *blob, struct bd_info *bd)
{
	return 0;
}

int print_cpuinfo(void)
{
	printf("Xen virtual CPU\n");
	return 0;
}

void board_cleanup_before_linux(void)
{
	xen_fini();
}
