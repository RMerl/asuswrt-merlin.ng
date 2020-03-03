/*
 * Copyright (C) 2010 Loongson Inc. & Lemote Inc. &
 *                    Insititute of Computing Technology
 * Author:  Xiang Gao, gaoxiang@ict.ac.cn
 *          Huacai Chen, chenhc@lemote.com
 *          Xiaofu Meng, Shuangshuang Zhang
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/mmzone.h>
#include <linux/module.h>
#include <linux/nodemask.h>
#include <linux/swap.h>
#include <linux/memblock.h>
#include <linux/bootmem.h>
#include <linux/pfn.h>
#include <linux/highmem.h>
#include <asm/page.h>
#include <asm/pgalloc.h>
#include <asm/sections.h>
#include <linux/irq.h>
#include <asm/bootinfo.h>
#include <asm/mc146818-time.h>
#include <asm/time.h>
#include <asm/wbflush.h>
#include <boot_param.h>

static struct node_data prealloc__node_data[MAX_NUMNODES];
unsigned char __node_distances[MAX_NUMNODES][MAX_NUMNODES];
EXPORT_SYMBOL(__node_distances);
struct node_data *__node_data[MAX_NUMNODES];
EXPORT_SYMBOL(__node_data);

static void enable_lpa(void)
{
	unsigned long value;

	value = __read_32bit_c0_register($16, 3);
	value |= 0x00000080;
	__write_32bit_c0_register($16, 3, value);
	value = __read_32bit_c0_register($16, 3);
	pr_info("CP0_Config3: CP0 16.3 (0x%lx)\n", value);

	value = __read_32bit_c0_register($5, 1);
	value |= 0x20000000;
	__write_32bit_c0_register($5, 1, value);
	value = __read_32bit_c0_register($5, 1);
	pr_info("CP0_PageGrain: CP0 5.1 (0x%lx)\n", value);
}

static void cpu_node_probe(void)
{
	int i;

	nodes_clear(node_possible_map);
	nodes_clear(node_online_map);
	for (i = 0; i < loongson_sysconf.nr_nodes; i++) {
		node_set_state(num_online_nodes(), N_POSSIBLE);
		node_set_online(num_online_nodes());
	}

	pr_info("NUMA: Discovered %d cpus on %d nodes\n",
		loongson_sysconf.nr_cpus, num_online_nodes());
}

static int __init compute_node_distance(int row, int col)
{
	int package_row = row * loongson_sysconf.cores_per_node /
				loongson_sysconf.cores_per_package;
	int package_col = col * loongson_sysconf.cores_per_node /
				loongson_sysconf.cores_per_package;

	if (col == row)
		return 0;
	else if (package_row == package_col)
		return 40;
	else
		return 100;
}

static void __init init_topology_matrix(void)
{
	int row, col;

	for (row = 0; row < MAX_NUMNODES; row++)
		for (col = 0; col < MAX_NUMNODES; col++)
			__node_distances[row][col] = -1;

	for_each_online_node(row) {
		for_each_online_node(col) {
			__node_distances[row][col] =
				compute_node_distance(row, col);
		}
	}
}

static unsigned long nid_to_addroffset(unsigned int nid)
{
	unsigned long result;
	switch (nid) {
	case 0:
	default:
		result = NODE0_ADDRSPACE_OFFSET;
		break;
	case 1:
		result = NODE1_ADDRSPACE_OFFSET;
		break;
	case 2:
		result = NODE2_ADDRSPACE_OFFSET;
		break;
	case 3:
		result = NODE3_ADDRSPACE_OFFSET;
		break;
	}
	return result;
}

static void __init szmem(unsigned int node)
{
	u32 i, mem_type;
	static unsigned long num_physpages = 0;
	u64 node_id, node_psize, start_pfn, end_pfn, mem_start, mem_size;

	/* Parse memory information and activate */
	for (i = 0; i < loongson_memmap->nr_map; i++) {
		node_id = loongson_memmap->map[i].node_id;
		if (node_id != node)
			continue;

		mem_type = loongson_memmap->map[i].mem_type;
		mem_size = loongson_memmap->map[i].mem_size;
		mem_start = loongson_memmap->map[i].mem_start;

		switch (mem_type) {
		case SYSTEM_RAM_LOW:
			start_pfn = ((node_id << 44) + mem_start) >> PAGE_SHIFT;
			node_psize = (mem_size << 20) >> PAGE_SHIFT;
			end_pfn  = start_pfn + node_psize;
			num_physpages += node_psize;
			pr_info("Node%d: mem_type:%d, mem_start:0x%llx, mem_size:0x%llx MB\n",
				(u32)node_id, mem_type, mem_start, mem_size);
			pr_info("       start_pfn:0x%llx, end_pfn:0x%llx, num_physpages:0x%lx\n",
				start_pfn, end_pfn, num_physpages);
			add_memory_region((node_id << 44) + mem_start,
				(u64)mem_size << 20, BOOT_MEM_RAM);
			memblock_add_node(PFN_PHYS(start_pfn),
				PFN_PHYS(end_pfn - start_pfn), node);
			break;
		case SYSTEM_RAM_HIGH:
			start_pfn = ((node_id << 44) + mem_start) >> PAGE_SHIFT;
			node_psize = (mem_size << 20) >> PAGE_SHIFT;
			end_pfn  = start_pfn + node_psize;
			num_physpages += node_psize;
			pr_info("Node%d: mem_type:%d, mem_start:0x%llx, mem_size:0x%llx MB\n",
				(u32)node_id, mem_type, mem_start, mem_size);
			pr_info("       start_pfn:0x%llx, end_pfn:0x%llx, num_physpages:0x%lx\n",
				start_pfn, end_pfn, num_physpages);
			add_memory_region((node_id << 44) + mem_start,
				(u64)mem_size << 20, BOOT_MEM_RAM);
			memblock_add_node(PFN_PHYS(start_pfn),
				PFN_PHYS(end_pfn - start_pfn), node);
			break;
		case MEM_RESERVED:
			pr_info("Node%d: mem_type:%d, mem_start:0x%llx, mem_size:0x%llx MB\n",
				(u32)node_id, mem_type, mem_start, mem_size);
			add_memory_region((node_id << 44) + mem_start,
				(u64)mem_size << 20, BOOT_MEM_RESERVED);
			memblock_reserve(((node_id << 44) + mem_start),
				mem_size << 20);
			break;
		}
	}
}

static void __init node_mem_init(unsigned int node)
{
	unsigned long bootmap_size;
	unsigned long node_addrspace_offset;
	unsigned long start_pfn, end_pfn, freepfn;

	node_addrspace_offset = nid_to_addroffset(node);
	pr_info("Node%d's addrspace_offset is 0x%lx\n",
			node, node_addrspace_offset);

	get_pfn_range_for_nid(node, &start_pfn, &end_pfn);
	freepfn = start_pfn;
	if (node == 0)
		freepfn = PFN_UP(__pa_symbol(&_end)); /* kernel end address */
	pr_info("Node%d: start_pfn=0x%lx, end_pfn=0x%lx, freepfn=0x%lx\n",
		node, start_pfn, end_pfn, freepfn);

	__node_data[node] = prealloc__node_data + node;

	NODE_DATA(node)->bdata = &bootmem_node_data[node];
	NODE_DATA(node)->node_start_pfn = start_pfn;
	NODE_DATA(node)->node_spanned_pages = end_pfn - start_pfn;

	bootmap_size = init_bootmem_node(NODE_DATA(node), freepfn,
					start_pfn, end_pfn);
	free_bootmem_with_active_regions(node, end_pfn);
	if (node == 0) /* used by finalize_initrd() */
		max_low_pfn = end_pfn;

	/* This is reserved for the kernel and bdata->node_bootmem_map */
	reserve_bootmem_node(NODE_DATA(node), start_pfn << PAGE_SHIFT,
		((freepfn - start_pfn) << PAGE_SHIFT) + bootmap_size,
		BOOTMEM_DEFAULT);

	if (node == 0 && node_end_pfn(0) >= (0xffffffff >> PAGE_SHIFT)) {
		/* Reserve 0xff800000~0xffffffff for RS780E integrated GPU */
		reserve_bootmem_node(NODE_DATA(node),
				(node_addrspace_offset | 0xff800000),
				8 << 20, BOOTMEM_DEFAULT);
	}

	sparse_memory_present_with_active_regions(node);
}

static __init void prom_meminit(void)
{
	unsigned int node, cpu, active_cpu = 0;

	cpu_node_probe();
	init_topology_matrix();

	for (node = 0; node < loongson_sysconf.nr_nodes; node++) {
		if (node_online(node)) {
			szmem(node);
			node_mem_init(node);
			cpumask_clear(&__node_data[(node)]->cpumask);
		}
	}
	for (cpu = 0; cpu < loongson_sysconf.nr_cpus; cpu++) {
		node = cpu / loongson_sysconf.cores_per_node;
		if (node >= num_online_nodes())
			node = 0;

		if (loongson_sysconf.reserved_cpus_mask & (1<<cpu))
			continue;

		cpumask_set_cpu(active_cpu, &__node_data[(node)]->cpumask);
		pr_info("NUMA: set cpumask cpu %d on node %d\n", active_cpu, node);

		active_cpu++;
	}
}

void __init paging_init(void)
{
	unsigned node;
	unsigned long zones_size[MAX_NR_ZONES] = {0, };

	pagetable_init();

	for_each_online_node(node) {
		unsigned long  start_pfn, end_pfn;

		get_pfn_range_for_nid(node, &start_pfn, &end_pfn);

		if (end_pfn > max_low_pfn)
			max_low_pfn = end_pfn;
	}
#ifdef CONFIG_ZONE_DMA32
	zones_size[ZONE_DMA32] = MAX_DMA32_PFN;
#endif
	zones_size[ZONE_NORMAL] = max_low_pfn;
	free_area_init_nodes(zones_size);
}

void __init mem_init(void)
{
	high_memory = (void *) __va(get_num_physpages() << PAGE_SHIFT);
	free_all_bootmem();
	setup_zero_pages();	/* This comes from node 0 */
	mem_init_print_info(NULL);
}

/* All PCI device belongs to logical Node-0 */
int pcibus_to_node(struct pci_bus *bus)
{
	return 0;
}
EXPORT_SYMBOL(pcibus_to_node);

void __init prom_init_numa_memory(void)
{
	enable_lpa();
	prom_meminit();
}
EXPORT_SYMBOL(prom_init_numa_memory);
