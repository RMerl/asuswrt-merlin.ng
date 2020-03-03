/*
 * Extensible Firmware Interface
 *
 * Based on Extensible Firmware Interface Specification version 1.0
 *
 * Copyright (C) 1999 VA Linux Systems
 * Copyright (C) 1999 Walt Drummond <drummond@valinux.com>
 * Copyright (C) 1999-2002 Hewlett-Packard Co.
 *	David Mosberger-Tang <davidm@hpl.hp.com>
 *	Stephane Eranian <eranian@hpl.hp.com>
 *
 * All EFI Runtime Services are not implemented yet as EFI only
 * supports physical mode addressing on SoftSDV. This is to be fixed
 * in a future version.  --drummond 1999-07-20
 *
 * Implemented EFI runtime services and virtual mode calls.  --davidm
 *
 * Goutham Rao: <goutham.rao@intel.com>
 *	Skip non-WB memory and ignore empty memory ranges.
 */

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/ioport.h>
#include <linux/efi.h>

#include <asm/io.h>
#include <asm/desc.h>
#include <asm/page.h>
#include <asm/pgtable.h>
#include <asm/tlbflush.h>
#include <asm/efi.h>

/*
 * To make EFI call EFI runtime service in physical addressing mode we need
 * prolog/epilog before/after the invocation to claim the EFI runtime service
 * handler exclusively and to duplicate a memory mapping in low memory space,
 * say 0 - 3G.
 */

int __init efi_alloc_page_tables(void)
{
	return 0;
}

void efi_sync_low_kernel_mappings(void) {}
void __init efi_dump_pagetable(void) {}
int __init efi_setup_page_tables(unsigned long pa_memmap, unsigned num_pages)
{
	return 0;
}
void __init efi_cleanup_page_tables(unsigned long pa_memmap, unsigned num_pages)
{
}

void __init efi_map_region(efi_memory_desc_t *md)
{
	old_map_region(md);
}

void __init efi_map_region_fixed(efi_memory_desc_t *md) {}
void __init parse_efi_setup(u64 phys_addr, u32 data_len) {}

pgd_t * __init efi_call_phys_prolog(void)
{
	struct desc_ptr gdt_descr;
	pgd_t *save_pgd;

	/* Current pgd is swapper_pg_dir, we'll restore it later: */
	save_pgd = swapper_pg_dir;
	load_cr3(initial_page_table);
	__flush_tlb_all();

	gdt_descr.address = __pa(get_cpu_gdt_table(0));
	gdt_descr.size = GDT_SIZE - 1;
	load_gdt(&gdt_descr);

	return save_pgd;
}

void __init efi_call_phys_epilog(pgd_t *save_pgd)
{
	struct desc_ptr gdt_descr;

	gdt_descr.address = (unsigned long)get_cpu_gdt_table(0);
	gdt_descr.size = GDT_SIZE - 1;
	load_gdt(&gdt_descr);

	load_cr3(save_pgd);
	__flush_tlb_all();
}

void __init efi_runtime_mkexec(void)
{
	if (__supported_pte_mask & _PAGE_NX)
		runtime_code_page_mkexec();
}
