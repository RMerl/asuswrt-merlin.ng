/*
 * Copyright 2004-2009 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#include <linux/gfp.h>
#include <linux/swap.h>
#include <linux/bootmem.h>
#include <linux/uaccess.h>
#include <linux/export.h>
#include <asm/bfin-global.h>
#include <asm/pda.h>
#include <asm/cplbinit.h>
#include <asm/early_printk.h>
#include "blackfin_sram.h"

/*
 * ZERO_PAGE is a special page that is used for zero-initialized data and COW.
 * Let the bss do its zero-init magic so we don't have to do it ourselves.
 */
char empty_zero_page[PAGE_SIZE] __attribute__((aligned(PAGE_SIZE)));
EXPORT_SYMBOL(empty_zero_page);

#ifndef CONFIG_EXCEPTION_L1_SCRATCH
#if defined CONFIG_SYSCALL_TAB_L1
__attribute__((l1_data))
#endif
static unsigned long exception_stack[NR_CPUS][1024];
#endif

struct blackfin_pda cpu_pda[NR_CPUS];
EXPORT_SYMBOL(cpu_pda);

/*
 * paging_init() continues the virtual memory environment setup which
 * was begun by the code in arch/head.S.
 * The parameters are pointers to where to stick the starting and ending
 * addresses  of available kernel virtual memory.
 */
void __init paging_init(void)
{
	/*
	 * make sure start_mem is page aligned, otherwise bootmem and
	 * page_alloc get different views of the world
	 */
	unsigned long end_mem = memory_end & PAGE_MASK;

	unsigned long zones_size[MAX_NR_ZONES] = {
		[0] = 0,
		[ZONE_DMA] = (end_mem - CONFIG_PHY_RAM_BASE_ADDRESS) >> PAGE_SHIFT,
		[ZONE_NORMAL] = 0,
#ifdef CONFIG_HIGHMEM
		[ZONE_HIGHMEM] = 0,
#endif
	};

	/* Set up SFC/DFC registers (user data space) */
	set_fs(KERNEL_DS);

	pr_debug("free_area_init -> start_mem is %#lx virtual_end is %#lx\n",
	        PAGE_ALIGN(memory_start), end_mem);
	free_area_init_node(0, zones_size,
		CONFIG_PHY_RAM_BASE_ADDRESS >> PAGE_SHIFT, NULL);
}

asmlinkage void __init init_pda(void)
{
	unsigned int cpu = raw_smp_processor_id();

	early_shadow_stamp();

	/* Initialize the PDA fields holding references to other parts
	   of the memory. The content of such memory is still
	   undefined at the time of the call, we are only setting up
	   valid pointers to it. */
	memset(&cpu_pda[cpu], 0, sizeof(cpu_pda[cpu]));

#ifdef CONFIG_EXCEPTION_L1_SCRATCH
	cpu_pda[cpu].ex_stack = (unsigned long *)(L1_SCRATCH_START + \
					L1_SCRATCH_LENGTH);
#else
	cpu_pda[cpu].ex_stack = exception_stack[cpu + 1];
#endif

#ifdef CONFIG_SMP
	cpu_pda[cpu].imask = 0x1f;
#endif
}

void __init mem_init(void)
{
	char buf[64];

	high_memory = (void *)(memory_end & PAGE_MASK);
	max_mapnr = MAP_NR(high_memory);
	printk(KERN_DEBUG "Kernel managed physical pages: %lu\n", max_mapnr);

	/* This will put all low memory onto the freelists. */
	free_all_bootmem();

	snprintf(buf, sizeof(buf) - 1, "%uK DMA", DMA_UNCACHED_REGION >> 10);
	mem_init_print_info(buf);
}

#ifdef CONFIG_BLK_DEV_INITRD
void __init free_initrd_mem(unsigned long start, unsigned long end)
{
#ifndef CONFIG_MPU
	free_reserved_area((void *)start, (void *)end, -1, "initrd");
#endif
}
#endif

void __init_refok free_initmem(void)
{
#if defined CONFIG_RAMKERNEL && !defined CONFIG_MPU
	free_initmem_default(-1);
	if (memory_start == (unsigned long)(&__init_end))
		memory_start = (unsigned long)(&__init_begin);
#endif
}
