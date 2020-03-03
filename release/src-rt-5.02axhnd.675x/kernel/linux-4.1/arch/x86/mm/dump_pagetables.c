/*
 * Debug helper to dump the current kernel pagetables of the system
 * so that we can see what the various memory ranges are set to.
 *
 * (C) Copyright 2008 Intel Corporation
 *
 * Author: Arjan van de Ven <arjan@linux.intel.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2
 * of the License.
 */

#include <linux/debugfs.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/seq_file.h>

#include <asm/pgtable.h>

/*
 * The dumper groups pagetable entries of the same type into one, and for
 * that it needs to keep some state when walking, and flush this state
 * when a "break" in the continuity is found.
 */
struct pg_state {
	int level;
	pgprot_t current_prot;
	unsigned long start_address;
	unsigned long current_address;
	const struct addr_marker *marker;
	unsigned long lines;
	bool to_dmesg;
};

struct addr_marker {
	unsigned long start_address;
	const char *name;
	unsigned long max_lines;
};

/* indices for address_markers; keep sync'd w/ address_markers below */
enum address_markers_idx {
	USER_SPACE_NR = 0,
#ifdef CONFIG_X86_64
	KERNEL_SPACE_NR,
	LOW_KERNEL_NR,
	VMALLOC_START_NR,
	VMEMMAP_START_NR,
# ifdef CONFIG_X86_ESPFIX64
	ESPFIX_START_NR,
# endif
	HIGH_KERNEL_NR,
	MODULES_VADDR_NR,
	MODULES_END_NR,
#else
	KERNEL_SPACE_NR,
	VMALLOC_START_NR,
	VMALLOC_END_NR,
# ifdef CONFIG_HIGHMEM
	PKMAP_BASE_NR,
# endif
	FIXADDR_START_NR,
#endif
};

/* Address space markers hints */
static struct addr_marker address_markers[] = {
	{ 0, "User Space" },
#ifdef CONFIG_X86_64
	{ 0x8000000000000000UL, "Kernel Space" },
	{ PAGE_OFFSET,		"Low Kernel Mapping" },
	{ VMALLOC_START,        "vmalloc() Area" },
	{ VMEMMAP_START,        "Vmemmap" },
# ifdef CONFIG_X86_ESPFIX64
	{ ESPFIX_BASE_ADDR,	"ESPfix Area", 16 },
# endif
# ifdef CONFIG_EFI
	{ EFI_VA_END,		"EFI Runtime Services" },
# endif
	{ __START_KERNEL_map,   "High Kernel Mapping" },
	{ MODULES_VADDR,        "Modules" },
	{ MODULES_END,          "End Modules" },
#else
	{ PAGE_OFFSET,          "Kernel Mapping" },
	{ 0/* VMALLOC_START */, "vmalloc() Area" },
	{ 0/*VMALLOC_END*/,     "vmalloc() End" },
# ifdef CONFIG_HIGHMEM
	{ 0/*PKMAP_BASE*/,      "Persisent kmap() Area" },
# endif
	{ 0/*FIXADDR_START*/,   "Fixmap Area" },
#endif
	{ -1, NULL }		/* End of list */
};

/* Multipliers for offsets within the PTEs */
#define PTE_LEVEL_MULT (PAGE_SIZE)
#define PMD_LEVEL_MULT (PTRS_PER_PTE * PTE_LEVEL_MULT)
#define PUD_LEVEL_MULT (PTRS_PER_PMD * PMD_LEVEL_MULT)
#define PGD_LEVEL_MULT (PTRS_PER_PUD * PUD_LEVEL_MULT)

#define pt_dump_seq_printf(m, to_dmesg, fmt, args...)		\
({								\
	if (to_dmesg)					\
		printk(KERN_INFO fmt, ##args);			\
	else							\
		if (m)						\
			seq_printf(m, fmt, ##args);		\
})

#define pt_dump_cont_printf(m, to_dmesg, fmt, args...)		\
({								\
	if (to_dmesg)					\
		printk(KERN_CONT fmt, ##args);			\
	else							\
		if (m)						\
			seq_printf(m, fmt, ##args);		\
})

/*
 * Print a readable form of a pgprot_t to the seq_file
 */
static void printk_prot(struct seq_file *m, pgprot_t prot, int level, bool dmsg)
{
	pgprotval_t pr = pgprot_val(prot);
	static const char * const level_name[] =
		{ "cr3", "pgd", "pud", "pmd", "pte" };

	if (!pgprot_val(prot)) {
		/* Not present */
		pt_dump_cont_printf(m, dmsg, "                              ");
	} else {
		if (pr & _PAGE_USER)
			pt_dump_cont_printf(m, dmsg, "USR ");
		else
			pt_dump_cont_printf(m, dmsg, "    ");
		if (pr & _PAGE_RW)
			pt_dump_cont_printf(m, dmsg, "RW ");
		else
			pt_dump_cont_printf(m, dmsg, "ro ");
		if (pr & _PAGE_PWT)
			pt_dump_cont_printf(m, dmsg, "PWT ");
		else
			pt_dump_cont_printf(m, dmsg, "    ");
		if (pr & _PAGE_PCD)
			pt_dump_cont_printf(m, dmsg, "PCD ");
		else
			pt_dump_cont_printf(m, dmsg, "    ");

		/* Bit 7 has a different meaning on level 3 vs 4 */
		if (level <= 3 && pr & _PAGE_PSE)
			pt_dump_cont_printf(m, dmsg, "PSE ");
		else
			pt_dump_cont_printf(m, dmsg, "    ");
		if ((level == 4 && pr & _PAGE_PAT) ||
		    ((level == 3 || level == 2) && pr & _PAGE_PAT_LARGE))
			pt_dump_cont_printf(m, dmsg, "pat ");
		else
			pt_dump_cont_printf(m, dmsg, "    ");
		if (pr & _PAGE_GLOBAL)
			pt_dump_cont_printf(m, dmsg, "GLB ");
		else
			pt_dump_cont_printf(m, dmsg, "    ");
		if (pr & _PAGE_NX)
			pt_dump_cont_printf(m, dmsg, "NX ");
		else
			pt_dump_cont_printf(m, dmsg, "x  ");
	}
	pt_dump_cont_printf(m, dmsg, "%s\n", level_name[level]);
}

/*
 * On 64 bits, sign-extend the 48 bit address to 64 bit
 */
static unsigned long normalize_addr(unsigned long u)
{
#ifdef CONFIG_X86_64
	return (signed long)(u << 16) >> 16;
#else
	return u;
#endif
}

/*
 * This function gets called on a break in a continuous series
 * of PTE entries; the next one is different so we need to
 * print what we collected so far.
 */
static void note_page(struct seq_file *m, struct pg_state *st,
		      pgprot_t new_prot, int level)
{
	pgprotval_t prot, cur;
	static const char units[] = "BKMGTPE";

	/*
	 * If we have a "break" in the series, we need to flush the state that
	 * we have now. "break" is either changing perms, levels or
	 * address space marker.
	 */
	prot = pgprot_val(new_prot) & PTE_FLAGS_MASK;
	cur = pgprot_val(st->current_prot) & PTE_FLAGS_MASK;

	if (!st->level) {
		/* First entry */
		st->current_prot = new_prot;
		st->level = level;
		st->marker = address_markers;
		st->lines = 0;
		pt_dump_seq_printf(m, st->to_dmesg, "---[ %s ]---\n",
				   st->marker->name);
	} else if (prot != cur || level != st->level ||
		   st->current_address >= st->marker[1].start_address) {
		const char *unit = units;
		unsigned long delta;
		int width = sizeof(unsigned long) * 2;

		/*
		 * Now print the actual finished series
		 */
		if (!st->marker->max_lines ||
		    st->lines < st->marker->max_lines) {
			pt_dump_seq_printf(m, st->to_dmesg,
					   "0x%0*lx-0x%0*lx   ",
					   width, st->start_address,
					   width, st->current_address);

			delta = st->current_address - st->start_address;
			while (!(delta & 1023) && unit[1]) {
				delta >>= 10;
				unit++;
			}
			pt_dump_cont_printf(m, st->to_dmesg, "%9lu%c ",
					    delta, *unit);
			printk_prot(m, st->current_prot, st->level,
				    st->to_dmesg);
		}
		st->lines++;

		/*
		 * We print markers for special areas of address space,
		 * such as the start of vmalloc space etc.
		 * This helps in the interpretation.
		 */
		if (st->current_address >= st->marker[1].start_address) {
			if (st->marker->max_lines &&
			    st->lines > st->marker->max_lines) {
				unsigned long nskip =
					st->lines - st->marker->max_lines;
				pt_dump_seq_printf(m, st->to_dmesg,
						   "... %lu entr%s skipped ... \n",
						   nskip,
						   nskip == 1 ? "y" : "ies");
			}
			st->marker++;
			st->lines = 0;
			pt_dump_seq_printf(m, st->to_dmesg, "---[ %s ]---\n",
					   st->marker->name);
		}

		st->start_address = st->current_address;
		st->current_prot = new_prot;
		st->level = level;
	}
}

static void walk_pte_level(struct seq_file *m, struct pg_state *st, pmd_t addr,
							unsigned long P)
{
	int i;
	pte_t *start;

	start = (pte_t *) pmd_page_vaddr(addr);
	for (i = 0; i < PTRS_PER_PTE; i++) {
		pgprot_t prot = pte_pgprot(*start);

		st->current_address = normalize_addr(P + i * PTE_LEVEL_MULT);
		note_page(m, st, prot, 4);
		start++;
	}
}

#if PTRS_PER_PMD > 1

static void walk_pmd_level(struct seq_file *m, struct pg_state *st, pud_t addr,
							unsigned long P)
{
	int i;
	pmd_t *start;

	start = (pmd_t *) pud_page_vaddr(addr);
	for (i = 0; i < PTRS_PER_PMD; i++) {
		st->current_address = normalize_addr(P + i * PMD_LEVEL_MULT);
		if (!pmd_none(*start)) {
			pgprotval_t prot = pmd_val(*start) & PTE_FLAGS_MASK;

			if (pmd_large(*start) || !pmd_present(*start))
				note_page(m, st, __pgprot(prot), 3);
			else
				walk_pte_level(m, st, *start,
					       P + i * PMD_LEVEL_MULT);
		} else
			note_page(m, st, __pgprot(0), 3);
		start++;
	}
}

#else
#define walk_pmd_level(m,s,a,p) walk_pte_level(m,s,__pmd(pud_val(a)),p)
#define pud_large(a) pmd_large(__pmd(pud_val(a)))
#define pud_none(a)  pmd_none(__pmd(pud_val(a)))
#endif

#if PTRS_PER_PUD > 1

static void walk_pud_level(struct seq_file *m, struct pg_state *st, pgd_t addr,
							unsigned long P)
{
	int i;
	pud_t *start;

	start = (pud_t *) pgd_page_vaddr(addr);

	for (i = 0; i < PTRS_PER_PUD; i++) {
		st->current_address = normalize_addr(P + i * PUD_LEVEL_MULT);
		if (!pud_none(*start)) {
			pgprotval_t prot = pud_val(*start) & PTE_FLAGS_MASK;

			if (pud_large(*start) || !pud_present(*start))
				note_page(m, st, __pgprot(prot), 2);
			else
				walk_pmd_level(m, st, *start,
					       P + i * PUD_LEVEL_MULT);
		} else
			note_page(m, st, __pgprot(0), 2);

		start++;
	}
}

#else
#define walk_pud_level(m,s,a,p) walk_pmd_level(m,s,__pud(pgd_val(a)),p)
#define pgd_large(a) pud_large(__pud(pgd_val(a)))
#define pgd_none(a)  pud_none(__pud(pgd_val(a)))
#endif

void ptdump_walk_pgd_level(struct seq_file *m, pgd_t *pgd)
{
#ifdef CONFIG_X86_64
	pgd_t *start = (pgd_t *) &init_level4_pgt;
#else
	pgd_t *start = swapper_pg_dir;
#endif
	int i;
	struct pg_state st = {};

	if (pgd) {
		start = pgd;
		st.to_dmesg = true;
	}

	for (i = 0; i < PTRS_PER_PGD; i++) {
		st.current_address = normalize_addr(i * PGD_LEVEL_MULT);
		if (!pgd_none(*start)) {
			pgprotval_t prot = pgd_val(*start) & PTE_FLAGS_MASK;

			if (pgd_large(*start) || !pgd_present(*start))
				note_page(m, &st, __pgprot(prot), 1);
			else
				walk_pud_level(m, &st, *start,
					       i * PGD_LEVEL_MULT);
		} else
			note_page(m, &st, __pgprot(0), 1);

		start++;
	}

	/* Flush out the last page */
	st.current_address = normalize_addr(PTRS_PER_PGD*PGD_LEVEL_MULT);
	note_page(m, &st, __pgprot(0), 0);
}

static int ptdump_show(struct seq_file *m, void *v)
{
	ptdump_walk_pgd_level(m, NULL);
	return 0;
}

static int ptdump_open(struct inode *inode, struct file *filp)
{
	return single_open(filp, ptdump_show, NULL);
}

static const struct file_operations ptdump_fops = {
	.open		= ptdump_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int pt_dump_init(void)
{
	struct dentry *pe;

#ifdef CONFIG_X86_32
	/* Not a compile-time constant on x86-32 */
	address_markers[VMALLOC_START_NR].start_address = VMALLOC_START;
	address_markers[VMALLOC_END_NR].start_address = VMALLOC_END;
# ifdef CONFIG_HIGHMEM
	address_markers[PKMAP_BASE_NR].start_address = PKMAP_BASE;
# endif
	address_markers[FIXADDR_START_NR].start_address = FIXADDR_START;
#endif

	pe = debugfs_create_file("kernel_page_tables", 0600, NULL, NULL,
				 &ptdump_fops);
	if (!pe)
		return -ENOMEM;

	return 0;
}

__initcall(pt_dump_init);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Arjan van de Ven <arjan@linux.intel.com>");
MODULE_DESCRIPTION("Kernel debugging helper that dumps pagetables");
