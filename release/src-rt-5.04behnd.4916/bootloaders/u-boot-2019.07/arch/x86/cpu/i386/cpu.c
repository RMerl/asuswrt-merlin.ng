// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008-2011
 * Graeme Russ, <graeme.russ@gmail.com>
 *
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, <daniel@omicron.se>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
 *
 * Part of this file is adapted from coreboot
 * src/arch/x86/lib/cpu.c
 */

#include <common.h>
#include <malloc.h>
#include <asm/control_regs.h>
#include <asm/cpu.h>
#include <asm/mp.h>
#include <asm/msr.h>
#include <asm/mtrr.h>
#include <asm/processor-flags.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Constructor for a conventional segment GDT (or LDT) entry
 * This is a macro so it can be used in initialisers
 */
#define GDT_ENTRY(flags, base, limit)			\
	((((base)  & 0xff000000ULL) << (56-24)) |	\
	 (((flags) & 0x0000f0ffULL) << 40) |		\
	 (((limit) & 0x000f0000ULL) << (48-16)) |	\
	 (((base)  & 0x00ffffffULL) << 16) |		\
	 (((limit) & 0x0000ffffULL)))

struct gdt_ptr {
	u16 len;
	u32 ptr;
} __packed;

struct cpu_device_id {
	unsigned vendor;
	unsigned device;
};

struct cpuinfo_x86 {
	uint8_t x86;            /* CPU family */
	uint8_t x86_vendor;     /* CPU vendor */
	uint8_t x86_model;
	uint8_t x86_mask;
};

/*
 * List of cpu vendor strings along with their normalized
 * id values.
 */
static const struct {
	int vendor;
	const char *name;
} x86_vendors[] = {
	{ X86_VENDOR_INTEL,     "GenuineIntel", },
	{ X86_VENDOR_CYRIX,     "CyrixInstead", },
	{ X86_VENDOR_AMD,       "AuthenticAMD", },
	{ X86_VENDOR_UMC,       "UMC UMC UMC ", },
	{ X86_VENDOR_NEXGEN,    "NexGenDriven", },
	{ X86_VENDOR_CENTAUR,   "CentaurHauls", },
	{ X86_VENDOR_RISE,      "RiseRiseRise", },
	{ X86_VENDOR_TRANSMETA, "GenuineTMx86", },
	{ X86_VENDOR_TRANSMETA, "TransmetaCPU", },
	{ X86_VENDOR_NSC,       "Geode by NSC", },
	{ X86_VENDOR_SIS,       "SiS SiS SiS ", },
};

static void load_ds(u32 segment)
{
	asm volatile("movl %0, %%ds" : : "r" (segment * X86_GDT_ENTRY_SIZE));
}

static void load_es(u32 segment)
{
	asm volatile("movl %0, %%es" : : "r" (segment * X86_GDT_ENTRY_SIZE));
}

static void load_fs(u32 segment)
{
	asm volatile("movl %0, %%fs" : : "r" (segment * X86_GDT_ENTRY_SIZE));
}

static void load_gs(u32 segment)
{
	asm volatile("movl %0, %%gs" : : "r" (segment * X86_GDT_ENTRY_SIZE));
}

static void load_ss(u32 segment)
{
	asm volatile("movl %0, %%ss" : : "r" (segment * X86_GDT_ENTRY_SIZE));
}

static void load_gdt(const u64 *boot_gdt, u16 num_entries)
{
	struct gdt_ptr gdt;

	gdt.len = (num_entries * X86_GDT_ENTRY_SIZE) - 1;
	gdt.ptr = (ulong)boot_gdt;

	asm volatile("lgdtl %0\n" : : "m" (gdt));
}

void arch_setup_gd(gd_t *new_gd)
{
	u64 *gdt_addr;

	gdt_addr = new_gd->arch.gdt;

	/*
	 * CS: code, read/execute, 4 GB, base 0
	 *
	 * Some OS (like VxWorks) requires GDT entry 1 to be the 32-bit CS
	 */
	gdt_addr[X86_GDT_ENTRY_UNUSED] = GDT_ENTRY(0xc09b, 0, 0xfffff);
	gdt_addr[X86_GDT_ENTRY_32BIT_CS] = GDT_ENTRY(0xc09b, 0, 0xfffff);

	/* DS: data, read/write, 4 GB, base 0 */
	gdt_addr[X86_GDT_ENTRY_32BIT_DS] = GDT_ENTRY(0xc093, 0, 0xfffff);

	/* FS: data, read/write, 4 GB, base (Global Data Pointer) */
	new_gd->arch.gd_addr = new_gd;
	gdt_addr[X86_GDT_ENTRY_32BIT_FS] = GDT_ENTRY(0xc093,
		     (ulong)&new_gd->arch.gd_addr, 0xfffff);

	/* 16-bit CS: code, read/execute, 64 kB, base 0 */
	gdt_addr[X86_GDT_ENTRY_16BIT_CS] = GDT_ENTRY(0x009b, 0, 0x0ffff);

	/* 16-bit DS: data, read/write, 64 kB, base 0 */
	gdt_addr[X86_GDT_ENTRY_16BIT_DS] = GDT_ENTRY(0x0093, 0, 0x0ffff);

	gdt_addr[X86_GDT_ENTRY_16BIT_FLAT_CS] = GDT_ENTRY(0x809b, 0, 0xfffff);
	gdt_addr[X86_GDT_ENTRY_16BIT_FLAT_DS] = GDT_ENTRY(0x8093, 0, 0xfffff);

	load_gdt(gdt_addr, X86_GDT_NUM_ENTRIES);
	load_ds(X86_GDT_ENTRY_32BIT_DS);
	load_es(X86_GDT_ENTRY_32BIT_DS);
	load_gs(X86_GDT_ENTRY_32BIT_DS);
	load_ss(X86_GDT_ENTRY_32BIT_DS);
	load_fs(X86_GDT_ENTRY_32BIT_FS);
}

#ifdef CONFIG_HAVE_FSP
/*
 * Setup FSP execution environment GDT
 *
 * Per Intel FSP external architecture specification, before calling any FSP
 * APIs, we need make sure the system is in flat 32-bit mode and both the code
 * and data selectors should have full 4GB access range. Here we reuse the one
 * we used in arch/x86/cpu/start16.S, and reload the segement registers.
 */
void setup_fsp_gdt(void)
{
	load_gdt((const u64 *)(gdt_rom + CONFIG_RESET_SEG_START), 4);
	load_ds(X86_GDT_ENTRY_32BIT_DS);
	load_ss(X86_GDT_ENTRY_32BIT_DS);
	load_es(X86_GDT_ENTRY_32BIT_DS);
	load_fs(X86_GDT_ENTRY_32BIT_DS);
	load_gs(X86_GDT_ENTRY_32BIT_DS);
}
#endif

/*
 * Cyrix CPUs without cpuid or with cpuid not yet enabled can be detected
 * by the fact that they preserve the flags across the division of 5/2.
 * PII and PPro exhibit this behavior too, but they have cpuid available.
 */

/*
 * Perform the Cyrix 5/2 test. A Cyrix won't change
 * the flags, while other 486 chips will.
 */
static inline int test_cyrix_52div(void)
{
	unsigned int test;

	__asm__ __volatile__(
	     "sahf\n\t"		/* clear flags (%eax = 0x0005) */
	     "div %b2\n\t"	/* divide 5 by 2 */
	     "lahf"		/* store flags into %ah */
	     : "=a" (test)
	     : "0" (5), "q" (2)
	     : "cc");

	/* AH is 0x02 on Cyrix after the divide.. */
	return (unsigned char) (test >> 8) == 0x02;
}

/*
 *	Detect a NexGen CPU running without BIOS hypercode new enough
 *	to have CPUID. (Thanks to Herbert Oppmann)
 */
static int deep_magic_nexgen_probe(void)
{
	int ret;

	__asm__ __volatile__ (
		"	movw	$0x5555, %%ax\n"
		"	xorw	%%dx,%%dx\n"
		"	movw	$2, %%cx\n"
		"	divw	%%cx\n"
		"	movl	$0, %%eax\n"
		"	jnz	1f\n"
		"	movl	$1, %%eax\n"
		"1:\n"
		: "=a" (ret) : : "cx", "dx");
	return  ret;
}

static bool has_cpuid(void)
{
	return flag_is_changeable_p(X86_EFLAGS_ID);
}

static bool has_mtrr(void)
{
	return cpuid_edx(0x00000001) & (1 << 12) ? true : false;
}

static int build_vendor_name(char *vendor_name)
{
	struct cpuid_result result;
	result = cpuid(0x00000000);
	unsigned int *name_as_ints = (unsigned int *)vendor_name;

	name_as_ints[0] = result.ebx;
	name_as_ints[1] = result.edx;
	name_as_ints[2] = result.ecx;

	return result.eax;
}

static void identify_cpu(struct cpu_device_id *cpu)
{
	char vendor_name[16];
	int i;

	vendor_name[0] = '\0'; /* Unset */
	cpu->device = 0; /* fix gcc 4.4.4 warning */

	/* Find the id and vendor_name */
	if (!has_cpuid()) {
		/* Its a 486 if we can modify the AC flag */
		if (flag_is_changeable_p(X86_EFLAGS_AC))
			cpu->device = 0x00000400; /* 486 */
		else
			cpu->device = 0x00000300; /* 386 */
		if ((cpu->device == 0x00000400) && test_cyrix_52div()) {
			memcpy(vendor_name, "CyrixInstead", 13);
			/* If we ever care we can enable cpuid here */
		}
		/* Detect NexGen with old hypercode */
		else if (deep_magic_nexgen_probe())
			memcpy(vendor_name, "NexGenDriven", 13);
	}
	if (has_cpuid()) {
		int  cpuid_level;

		cpuid_level = build_vendor_name(vendor_name);
		vendor_name[12] = '\0';

		/* Intel-defined flags: level 0x00000001 */
		if (cpuid_level >= 0x00000001) {
			cpu->device = cpuid_eax(0x00000001);
		} else {
			/* Have CPUID level 0 only unheard of */
			cpu->device = 0x00000400;
		}
	}
	cpu->vendor = X86_VENDOR_UNKNOWN;
	for (i = 0; i < ARRAY_SIZE(x86_vendors); i++) {
		if (memcmp(vendor_name, x86_vendors[i].name, 12) == 0) {
			cpu->vendor = x86_vendors[i].vendor;
			break;
		}
	}
}

static inline void get_fms(struct cpuinfo_x86 *c, uint32_t tfms)
{
	c->x86 = (tfms >> 8) & 0xf;
	c->x86_model = (tfms >> 4) & 0xf;
	c->x86_mask = tfms & 0xf;
	if (c->x86 == 0xf)
		c->x86 += (tfms >> 20) & 0xff;
	if (c->x86 >= 0x6)
		c->x86_model += ((tfms >> 16) & 0xF) << 4;
}

u32 cpu_get_family_model(void)
{
	return gd->arch.x86_device & 0x0fff0ff0;
}

u32 cpu_get_stepping(void)
{
	return gd->arch.x86_mask;
}

/* initialise FPU, reset EM, set MP and NE */
static void setup_cpu_features(void)
{
	const u32 em_rst = ~X86_CR0_EM;
	const u32 mp_ne_set = X86_CR0_MP | X86_CR0_NE;

	asm ("fninit\n" \
	"movl %%cr0, %%eax\n" \
	"andl %0, %%eax\n" \
	"orl  %1, %%eax\n" \
	"movl %%eax, %%cr0\n" \
	: : "i" (em_rst), "i" (mp_ne_set) : "eax");
}

static void setup_identity(void)
{
	/* identify CPU via cpuid and store the decoded info into gd->arch */
	if (has_cpuid()) {
		struct cpu_device_id cpu;
		struct cpuinfo_x86 c;

		identify_cpu(&cpu);
		get_fms(&c, cpu.device);
		gd->arch.x86 = c.x86;
		gd->arch.x86_vendor = cpu.vendor;
		gd->arch.x86_model = c.x86_model;
		gd->arch.x86_mask = c.x86_mask;
		gd->arch.x86_device = cpu.device;

		gd->arch.has_mtrr = has_mtrr();
	}
}

/* Don't allow PCI region 3 to use memory in the 2-4GB memory hole */
static void setup_pci_ram_top(void)
{
	gd->pci_ram_top = 0x80000000U;
}

static void setup_mtrr(void)
{
	u64 mtrr_cap;

	/* Configure fixed range MTRRs for some legacy regions */
	if (!gd->arch.has_mtrr)
		return;

	mtrr_cap = native_read_msr(MTRR_CAP_MSR);
	if (mtrr_cap & MTRR_CAP_FIX) {
		/* Mark the VGA RAM area as uncacheable */
		native_write_msr(MTRR_FIX_16K_A0000_MSR,
				 MTRR_FIX_TYPE(MTRR_TYPE_UNCACHEABLE),
				 MTRR_FIX_TYPE(MTRR_TYPE_UNCACHEABLE));

		/*
		 * Mark the PCI ROM area as cacheable to improve ROM
		 * execution performance.
		 */
		native_write_msr(MTRR_FIX_4K_C0000_MSR,
				 MTRR_FIX_TYPE(MTRR_TYPE_WRBACK),
				 MTRR_FIX_TYPE(MTRR_TYPE_WRBACK));
		native_write_msr(MTRR_FIX_4K_C8000_MSR,
				 MTRR_FIX_TYPE(MTRR_TYPE_WRBACK),
				 MTRR_FIX_TYPE(MTRR_TYPE_WRBACK));
		native_write_msr(MTRR_FIX_4K_D0000_MSR,
				 MTRR_FIX_TYPE(MTRR_TYPE_WRBACK),
				 MTRR_FIX_TYPE(MTRR_TYPE_WRBACK));
		native_write_msr(MTRR_FIX_4K_D8000_MSR,
				 MTRR_FIX_TYPE(MTRR_TYPE_WRBACK),
				 MTRR_FIX_TYPE(MTRR_TYPE_WRBACK));

		/* Enable the fixed range MTRRs */
		msr_setbits_64(MTRR_DEF_TYPE_MSR, MTRR_DEF_TYPE_FIX_EN);
	}
}

int x86_cpu_init_f(void)
{
	if (ll_boot_init())
		setup_cpu_features();
	setup_identity();
	setup_mtrr();
	setup_pci_ram_top();

	/* Set up the i8254 timer if required */
	if (IS_ENABLED(CONFIG_I8254_TIMER))
		i8254_init();

	return 0;
}

int x86_cpu_reinit_f(void)
{
	setup_identity();
	setup_pci_ram_top();

	return 0;
}

void x86_enable_caches(void)
{
	unsigned long cr0;

	cr0 = read_cr0();
	cr0 &= ~(X86_CR0_NW | X86_CR0_CD);
	write_cr0(cr0);
	wbinvd();
}
void enable_caches(void) __attribute__((weak, alias("x86_enable_caches")));

void x86_disable_caches(void)
{
	unsigned long cr0;

	cr0 = read_cr0();
	cr0 |= X86_CR0_NW | X86_CR0_CD;
	wbinvd();
	write_cr0(cr0);
	wbinvd();
}
void disable_caches(void) __attribute__((weak, alias("x86_disable_caches")));

int dcache_status(void)
{
	return !(read_cr0() & X86_CR0_CD);
}

void cpu_enable_paging_pae(ulong cr3)
{
	__asm__ __volatile__(
		/* Load the page table address */
		"movl	%0, %%cr3\n"
		/* Enable pae */
		"movl	%%cr4, %%eax\n"
		"orl	$0x00000020, %%eax\n"
		"movl	%%eax, %%cr4\n"
		/* Enable paging */
		"movl	%%cr0, %%eax\n"
		"orl	$0x80000000, %%eax\n"
		"movl	%%eax, %%cr0\n"
		:
		: "r" (cr3)
		: "eax");
}

void cpu_disable_paging_pae(void)
{
	/* Turn off paging */
	__asm__ __volatile__ (
		/* Disable paging */
		"movl	%%cr0, %%eax\n"
		"andl	$0x7fffffff, %%eax\n"
		"movl	%%eax, %%cr0\n"
		/* Disable pae */
		"movl	%%cr4, %%eax\n"
		"andl	$0xffffffdf, %%eax\n"
		"movl	%%eax, %%cr4\n"
		:
		:
		: "eax");
}

static bool can_detect_long_mode(void)
{
	return cpuid_eax(0x80000000) > 0x80000000UL;
}

static bool has_long_mode(void)
{
	return cpuid_edx(0x80000001) & (1 << 29) ? true : false;
}

int cpu_has_64bit(void)
{
	return has_cpuid() && can_detect_long_mode() &&
		has_long_mode();
}

#define PAGETABLE_BASE		0x80000
#define PAGETABLE_SIZE		(6 * 4096)

/**
 * build_pagetable() - build a flat 4GiB page table structure for 64-bti mode
 *
 * @pgtable: Pointer to a 24iKB block of memory
 */
static void build_pagetable(uint32_t *pgtable)
{
	uint i;

	memset(pgtable, '\0', PAGETABLE_SIZE);

	/* Level 4 needs a single entry */
	pgtable[0] = (ulong)&pgtable[1024] + 7;

	/* Level 3 has one 64-bit entry for each GiB of memory */
	for (i = 0; i < 4; i++)
		pgtable[1024 + i * 2] = (ulong)&pgtable[2048] + 0x1000 * i + 7;

	/* Level 2 has 2048 64-bit entries, each repesenting 2MiB */
	for (i = 0; i < 2048; i++)
		pgtable[2048 + i * 2] = 0x183 + (i << 21UL);
}

int cpu_jump_to_64bit(ulong setup_base, ulong target)
{
	uint32_t *pgtable;

	pgtable = memalign(4096, PAGETABLE_SIZE);
	if (!pgtable)
		return -ENOMEM;

	build_pagetable(pgtable);
	cpu_call64((ulong)pgtable, setup_base, target);
	free(pgtable);

	return -EFAULT;
}

/*
 * Jump from SPL to U-Boot
 *
 * This function is work-in-progress with many issues to resolve.
 *
 * It works by setting up several regions:
 *   ptr      - a place to put the code that jumps into 64-bit mode
 *   gdt      - a place to put the global descriptor table
 *   pgtable  - a place to put the page tables
 *
 * The cpu_call64() code is copied from ROM and then manually patched so that
 * it has the correct GDT address in RAM. U-Boot is copied from ROM into
 * its pre-relocation address. Then we jump to the cpu_call64() code in RAM,
 * which changes to 64-bit mode and starts U-Boot.
 */
int cpu_jump_to_64bit_uboot(ulong target)
{
	typedef void (*func_t)(ulong pgtable, ulong setup_base, ulong target);
	uint32_t *pgtable;
	func_t func;
	char *ptr;

	pgtable = (uint32_t *)PAGETABLE_BASE;

	build_pagetable(pgtable);

	extern long call64_stub_size;
	ptr = malloc(call64_stub_size);
	if (!ptr) {
		printf("Failed to allocate the cpu_call64 stub\n");
		return -ENOMEM;
	}
	memcpy(ptr, cpu_call64, call64_stub_size);

	func = (func_t)ptr;

	/*
	 * Copy U-Boot from ROM
	 * TODO(sjg@chromium.org): Figure out a way to get the text base
	 * correctly here, and in the device-tree binman definition.
	 *
	 * Also consider using FIT so we get the correct image length and
	 * parameters.
	 */
	memcpy((char *)target, (char *)0xfff00000, 0x100000);

	/* Jump to U-Boot */
	func((ulong)pgtable, 0, (ulong)target);

	return -EFAULT;
}

#ifdef CONFIG_SMP
static int enable_smis(struct udevice *cpu, void *unused)
{
	return 0;
}

static struct mp_flight_record mp_steps[] = {
	MP_FR_BLOCK_APS(mp_init_cpu, NULL, mp_init_cpu, NULL),
	/* Wait for APs to finish initialization before proceeding */
	MP_FR_BLOCK_APS(NULL, NULL, enable_smis, NULL),
};

int x86_mp_init(void)
{
	struct mp_params mp_params;

	mp_params.parallel_microcode_load = 0,
	mp_params.flight_plan = &mp_steps[0];
	mp_params.num_records = ARRAY_SIZE(mp_steps);
	mp_params.microcode_pointer = 0;

	if (mp_init(&mp_params)) {
		printf("Warning: MP init failure\n");
		return -EIO;
	}

	return 0;
}
#endif
