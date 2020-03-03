/*
 * arch/xtensa/kernel/setup.c
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1995  Linus Torvalds
 * Copyright (C) 2001 - 2005  Tensilica Inc.
 *
 * Chris Zankel	<chris@zankel.net>
 * Joe Taylor	<joe@tensilica.com, joetylr@yahoo.com>
 * Kevin Chea
 * Marc Gauthier<marc@tensilica.com> <marc@alumni.uwaterloo.ca>
 */

#include <linux/errno.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/proc_fs.h>
#include <linux/screen_info.h>
#include <linux/bootmem.h>
#include <linux/kernel.h>
#include <linux/percpu.h>
#include <linux/clk-provider.h>
#include <linux/cpu.h>
#include <linux/of_fdt.h>
#include <linux/of_platform.h>

#if defined(CONFIG_VGA_CONSOLE) || defined(CONFIG_DUMMY_CONSOLE)
# include <linux/console.h>
#endif

#ifdef CONFIG_RTC
# include <linux/timex.h>
#endif

#ifdef CONFIG_PROC_FS
# include <linux/seq_file.h>
#endif

#include <asm/bootparam.h>
#include <asm/mmu_context.h>
#include <asm/pgtable.h>
#include <asm/processor.h>
#include <asm/timex.h>
#include <asm/platform.h>
#include <asm/page.h>
#include <asm/setup.h>
#include <asm/param.h>
#include <asm/traps.h>
#include <asm/smp.h>
#include <asm/sysmem.h>

#include <platform/hardware.h>

#if defined(CONFIG_VGA_CONSOLE) || defined(CONFIG_DUMMY_CONSOLE)
struct screen_info screen_info = { 0, 24, 0, 0, 0, 80, 0, 0, 0, 24, 1, 16};
#endif

#ifdef CONFIG_BLK_DEV_FD
extern struct fd_ops no_fd_ops;
struct fd_ops *fd_ops;
#endif

extern struct rtc_ops no_rtc_ops;
struct rtc_ops *rtc_ops;

#ifdef CONFIG_BLK_DEV_INITRD
extern unsigned long initrd_start;
extern unsigned long initrd_end;
int initrd_is_mapped = 0;
extern int initrd_below_start_ok;
#endif

#ifdef CONFIG_OF
void *dtb_start = __dtb_start;
#endif

unsigned char aux_device_present;
extern unsigned long loops_per_jiffy;

/* Command line specified as configuration option. */

static char __initdata command_line[COMMAND_LINE_SIZE];

#ifdef CONFIG_CMDLINE_BOOL
static char default_command_line[COMMAND_LINE_SIZE] __initdata = CONFIG_CMDLINE;
#endif

/*
 * Boot parameter parsing.
 *
 * The Xtensa port uses a list of variable-sized tags to pass data to
 * the kernel. The first tag must be a BP_TAG_FIRST tag for the list
 * to be recognised. The list is terminated with a zero-sized
 * BP_TAG_LAST tag.
 */

typedef struct tagtable {
	u32 tag;
	int (*parse)(const bp_tag_t*);
} tagtable_t;

#define __tagtable(tag, fn) static tagtable_t __tagtable_##fn 		\
	__attribute__((used, section(".taglist"))) = { tag, fn }

/* parse current tag */

static int __init parse_tag_mem(const bp_tag_t *tag)
{
	struct bp_meminfo *mi = (struct bp_meminfo *)(tag->data);

	if (mi->type != MEMORY_TYPE_CONVENTIONAL)
		return -1;

	return add_sysmem_bank(mi->start, mi->end);
}

__tagtable(BP_TAG_MEMORY, parse_tag_mem);

#ifdef CONFIG_BLK_DEV_INITRD

static int __init parse_tag_initrd(const bp_tag_t* tag)
{
	struct bp_meminfo *mi = (struct bp_meminfo *)(tag->data);

	initrd_start = (unsigned long)__va(mi->start);
	initrd_end = (unsigned long)__va(mi->end);

	return 0;
}

__tagtable(BP_TAG_INITRD, parse_tag_initrd);

#endif /* CONFIG_BLK_DEV_INITRD */

#ifdef CONFIG_OF

static int __init parse_tag_fdt(const bp_tag_t *tag)
{
	dtb_start = __va(tag->data[0]);
	return 0;
}

__tagtable(BP_TAG_FDT, parse_tag_fdt);

#endif /* CONFIG_OF */

static int __init parse_tag_cmdline(const bp_tag_t* tag)
{
	strlcpy(command_line, (char *)(tag->data), COMMAND_LINE_SIZE);
	return 0;
}

__tagtable(BP_TAG_COMMAND_LINE, parse_tag_cmdline);

static int __init parse_bootparam(const bp_tag_t* tag)
{
	extern tagtable_t __tagtable_begin, __tagtable_end;
	tagtable_t *t;

	/* Boot parameters must start with a BP_TAG_FIRST tag. */

	if (tag->id != BP_TAG_FIRST) {
		printk(KERN_WARNING "Invalid boot parameters!\n");
		return 0;
	}

	tag = (bp_tag_t*)((unsigned long)tag + sizeof(bp_tag_t) + tag->size);

	/* Parse all tags. */

	while (tag != NULL && tag->id != BP_TAG_LAST) {
	 	for (t = &__tagtable_begin; t < &__tagtable_end; t++) {
			if (tag->id == t->tag) {
				t->parse(tag);
				break;
			}
		}
		if (t == &__tagtable_end)
			printk(KERN_WARNING "Ignoring tag "
			       "0x%08x\n", tag->id);
		tag = (bp_tag_t*)((unsigned long)(tag + 1) + tag->size);
	}

	return 0;
}

#ifdef CONFIG_OF
bool __initdata dt_memory_scan = false;

#if XCHAL_HAVE_PTP_MMU && XCHAL_HAVE_SPANNING_WAY
unsigned long xtensa_kio_paddr = XCHAL_KIO_DEFAULT_PADDR;
EXPORT_SYMBOL(xtensa_kio_paddr);

static int __init xtensa_dt_io_area(unsigned long node, const char *uname,
		int depth, void *data)
{
	const __be32 *ranges;
	int len;

	if (depth > 1)
		return 0;

	if (!of_flat_dt_is_compatible(node, "simple-bus"))
		return 0;

	ranges = of_get_flat_dt_prop(node, "ranges", &len);
	if (!ranges)
		return 1;
	if (len == 0)
		return 1;

	xtensa_kio_paddr = of_read_ulong(ranges+1, 1);
	/* round down to nearest 256MB boundary */
	xtensa_kio_paddr &= 0xf0000000;

	return 1;
}
#else
static int __init xtensa_dt_io_area(unsigned long node, const char *uname,
		int depth, void *data)
{
	return 1;
}
#endif

void __init early_init_dt_add_memory_arch(u64 base, u64 size)
{
	if (!dt_memory_scan)
		return;

	size &= PAGE_MASK;
	add_sysmem_bank(base, base + size);
}

void * __init early_init_dt_alloc_memory_arch(u64 size, u64 align)
{
	return __alloc_bootmem(size, align, 0);
}

void __init early_init_devtree(void *params)
{
	if (sysmem.nr_banks == 0)
		dt_memory_scan = true;

	early_init_dt_scan(params);
	of_scan_flat_dt(xtensa_dt_io_area, NULL);

	if (!command_line[0])
		strlcpy(command_line, boot_command_line, COMMAND_LINE_SIZE);
}

static int __init xtensa_device_probe(void)
{
	of_clk_init(NULL);
	of_platform_populate(NULL, of_default_bus_match_table, NULL, NULL);
	return 0;
}

device_initcall(xtensa_device_probe);

#endif /* CONFIG_OF */

/*
 * Initialize architecture. (Early stage)
 */

void __init init_arch(bp_tag_t *bp_start)
{
	/* Parse boot parameters */

	if (bp_start)
		parse_bootparam(bp_start);

#ifdef CONFIG_OF
	early_init_devtree(dtb_start);
#endif

	if (sysmem.nr_banks == 0) {
		add_sysmem_bank(PLATFORM_DEFAULT_MEM_START,
				PLATFORM_DEFAULT_MEM_START +
				PLATFORM_DEFAULT_MEM_SIZE);
	}

#ifdef CONFIG_CMDLINE_BOOL
	if (!command_line[0])
		strlcpy(command_line, default_command_line, COMMAND_LINE_SIZE);
#endif

	/* Early hook for platforms */

	platform_init(bp_start);

	/* Initialize MMU. */

	init_mmu();
}

/*
 * Initialize system. Setup memory and reserve regions.
 */

extern char _end;
extern char _stext;
extern char _WindowVectors_text_start;
extern char _WindowVectors_text_end;
extern char _DebugInterruptVector_literal_start;
extern char _DebugInterruptVector_text_end;
extern char _KernelExceptionVector_literal_start;
extern char _KernelExceptionVector_text_end;
extern char _UserExceptionVector_literal_start;
extern char _UserExceptionVector_text_end;
extern char _DoubleExceptionVector_literal_start;
extern char _DoubleExceptionVector_text_end;
#if XCHAL_EXCM_LEVEL >= 2
extern char _Level2InterruptVector_text_start;
extern char _Level2InterruptVector_text_end;
#endif
#if XCHAL_EXCM_LEVEL >= 3
extern char _Level3InterruptVector_text_start;
extern char _Level3InterruptVector_text_end;
#endif
#if XCHAL_EXCM_LEVEL >= 4
extern char _Level4InterruptVector_text_start;
extern char _Level4InterruptVector_text_end;
#endif
#if XCHAL_EXCM_LEVEL >= 5
extern char _Level5InterruptVector_text_start;
extern char _Level5InterruptVector_text_end;
#endif
#if XCHAL_EXCM_LEVEL >= 6
extern char _Level6InterruptVector_text_start;
extern char _Level6InterruptVector_text_end;
#endif



#ifdef CONFIG_S32C1I_SELFTEST
#if XCHAL_HAVE_S32C1I

static int __initdata rcw_word, rcw_probe_pc, rcw_exc;

/*
 * Basic atomic compare-and-swap, that records PC of S32C1I for probing.
 *
 * If *v == cmp, set *v = set.  Return previous *v.
 */
static inline int probed_compare_swap(int *v, int cmp, int set)
{
	int tmp;

	__asm__ __volatile__(
			"	movi	%1, 1f\n"
			"	s32i	%1, %4, 0\n"
			"	wsr	%2, scompare1\n"
			"1:	s32c1i	%0, %3, 0\n"
			: "=a" (set), "=&a" (tmp)
			: "a" (cmp), "a" (v), "a" (&rcw_probe_pc), "0" (set)
			: "memory"
			);
	return set;
}

/* Handle probed exception */

static void __init do_probed_exception(struct pt_regs *regs,
		unsigned long exccause)
{
	if (regs->pc == rcw_probe_pc) {	/* exception on s32c1i ? */
		regs->pc += 3;		/* skip the s32c1i instruction */
		rcw_exc = exccause;
	} else {
		do_unhandled(regs, exccause);
	}
}

/* Simple test of S32C1I (soc bringup assist) */

static int __init check_s32c1i(void)
{
	int n, cause1, cause2;
	void *handbus, *handdata, *handaddr; /* temporarily saved handlers */

	rcw_probe_pc = 0;
	handbus  = trap_set_handler(EXCCAUSE_LOAD_STORE_ERROR,
			do_probed_exception);
	handdata = trap_set_handler(EXCCAUSE_LOAD_STORE_DATA_ERROR,
			do_probed_exception);
	handaddr = trap_set_handler(EXCCAUSE_LOAD_STORE_ADDR_ERROR,
			do_probed_exception);

	/* First try an S32C1I that does not store: */
	rcw_exc = 0;
	rcw_word = 1;
	n = probed_compare_swap(&rcw_word, 0, 2);
	cause1 = rcw_exc;

	/* took exception? */
	if (cause1 != 0) {
		/* unclean exception? */
		if (n != 2 || rcw_word != 1)
			panic("S32C1I exception error");
	} else if (rcw_word != 1 || n != 1) {
		panic("S32C1I compare error");
	}

	/* Then an S32C1I that stores: */
	rcw_exc = 0;
	rcw_word = 0x1234567;
	n = probed_compare_swap(&rcw_word, 0x1234567, 0xabcde);
	cause2 = rcw_exc;

	if (cause2 != 0) {
		/* unclean exception? */
		if (n != 0xabcde || rcw_word != 0x1234567)
			panic("S32C1I exception error (b)");
	} else if (rcw_word != 0xabcde || n != 0x1234567) {
		panic("S32C1I store error");
	}

	/* Verify consistency of exceptions: */
	if (cause1 || cause2) {
		pr_warn("S32C1I took exception %d, %d\n", cause1, cause2);
		/* If emulation of S32C1I upon bus error gets implemented,
		   we can get rid of this panic for single core (not SMP) */
		panic("S32C1I exceptions not currently supported");
	}
	if (cause1 != cause2)
		panic("inconsistent S32C1I exceptions");

	trap_set_handler(EXCCAUSE_LOAD_STORE_ERROR, handbus);
	trap_set_handler(EXCCAUSE_LOAD_STORE_DATA_ERROR, handdata);
	trap_set_handler(EXCCAUSE_LOAD_STORE_ADDR_ERROR, handaddr);
	return 0;
}

#else /* XCHAL_HAVE_S32C1I */

/* This condition should not occur with a commercially deployed processor.
   Display reminder for early engr test or demo chips / FPGA bitstreams */
static int __init check_s32c1i(void)
{
	pr_warn("Processor configuration lacks atomic compare-and-swap support!\n");
	return 0;
}

#endif /* XCHAL_HAVE_S32C1I */
early_initcall(check_s32c1i);
#endif /* CONFIG_S32C1I_SELFTEST */


void __init setup_arch(char **cmdline_p)
{
	strlcpy(boot_command_line, command_line, COMMAND_LINE_SIZE);
	*cmdline_p = command_line;

	/* Reserve some memory regions */

#ifdef CONFIG_BLK_DEV_INITRD
	if (initrd_start < initrd_end) {
		initrd_is_mapped = mem_reserve(__pa(initrd_start),
					       __pa(initrd_end), 0) == 0;
		initrd_below_start_ok = 1;
	} else {
		initrd_start = 0;
	}
#endif

	mem_reserve(__pa(&_stext),__pa(&_end), 1);

	mem_reserve(__pa(&_WindowVectors_text_start),
		    __pa(&_WindowVectors_text_end), 0);

	mem_reserve(__pa(&_DebugInterruptVector_literal_start),
		    __pa(&_DebugInterruptVector_text_end), 0);

	mem_reserve(__pa(&_KernelExceptionVector_literal_start),
		    __pa(&_KernelExceptionVector_text_end), 0);

	mem_reserve(__pa(&_UserExceptionVector_literal_start),
		    __pa(&_UserExceptionVector_text_end), 0);

	mem_reserve(__pa(&_DoubleExceptionVector_literal_start),
		    __pa(&_DoubleExceptionVector_text_end), 0);

#if XCHAL_EXCM_LEVEL >= 2
	mem_reserve(__pa(&_Level2InterruptVector_text_start),
		    __pa(&_Level2InterruptVector_text_end), 0);
#endif
#if XCHAL_EXCM_LEVEL >= 3
	mem_reserve(__pa(&_Level3InterruptVector_text_start),
		    __pa(&_Level3InterruptVector_text_end), 0);
#endif
#if XCHAL_EXCM_LEVEL >= 4
	mem_reserve(__pa(&_Level4InterruptVector_text_start),
		    __pa(&_Level4InterruptVector_text_end), 0);
#endif
#if XCHAL_EXCM_LEVEL >= 5
	mem_reserve(__pa(&_Level5InterruptVector_text_start),
		    __pa(&_Level5InterruptVector_text_end), 0);
#endif
#if XCHAL_EXCM_LEVEL >= 6
	mem_reserve(__pa(&_Level6InterruptVector_text_start),
		    __pa(&_Level6InterruptVector_text_end), 0);
#endif

	parse_early_param();
	bootmem_init();

	unflatten_and_copy_device_tree();

	platform_setup(cmdline_p);

#ifdef CONFIG_SMP
	smp_init_cpus();
#endif

	paging_init();
	zones_init();

#ifdef CONFIG_VT
# if defined(CONFIG_VGA_CONSOLE)
	conswitchp = &vga_con;
# elif defined(CONFIG_DUMMY_CONSOLE)
	conswitchp = &dummy_con;
# endif
#endif

#ifdef CONFIG_PCI
	platform_pcibios_init();
#endif
}

static DEFINE_PER_CPU(struct cpu, cpu_data);

static int __init topology_init(void)
{
	int i;

	for_each_possible_cpu(i) {
		struct cpu *cpu = &per_cpu(cpu_data, i);
		cpu->hotpluggable = !!i;
		register_cpu(cpu, i);
	}

	return 0;
}
subsys_initcall(topology_init);

void machine_restart(char * cmd)
{
	platform_restart();
}

void machine_halt(void)
{
	platform_halt();
	while (1);
}

void machine_power_off(void)
{
	platform_power_off();
	while (1);
}
#ifdef CONFIG_PROC_FS

/*
 * Display some core information through /proc/cpuinfo.
 */

static int
c_show(struct seq_file *f, void *slot)
{
	/* high-level stuff */
	seq_printf(f, "CPU count\t: %u\n"
		      "CPU list\t: %*pbl\n"
		      "vendor_id\t: Tensilica\n"
		      "model\t\t: Xtensa " XCHAL_HW_VERSION_NAME "\n"
		      "core ID\t\t: " XCHAL_CORE_ID "\n"
		      "build ID\t: 0x%x\n"
		      "byte order\t: %s\n"
		      "cpu MHz\t\t: %lu.%02lu\n"
		      "bogomips\t: %lu.%02lu\n",
		      num_online_cpus(),
		      cpumask_pr_args(cpu_online_mask),
		      XCHAL_BUILD_UNIQUE_ID,
		      XCHAL_HAVE_BE ?  "big" : "little",
		      ccount_freq/1000000,
		      (ccount_freq/10000) % 100,
		      loops_per_jiffy/(500000/HZ),
		      (loops_per_jiffy/(5000/HZ)) % 100);

	seq_printf(f,"flags\t\t: "
#if XCHAL_HAVE_NMI
		     "nmi "
#endif
#if XCHAL_HAVE_DEBUG
		     "debug "
# if XCHAL_HAVE_OCD
		     "ocd "
# endif
#endif
#if XCHAL_HAVE_DENSITY
	    	     "density "
#endif
#if XCHAL_HAVE_BOOLEANS
		     "boolean "
#endif
#if XCHAL_HAVE_LOOPS
		     "loop "
#endif
#if XCHAL_HAVE_NSA
		     "nsa "
#endif
#if XCHAL_HAVE_MINMAX
		     "minmax "
#endif
#if XCHAL_HAVE_SEXT
		     "sext "
#endif
#if XCHAL_HAVE_CLAMPS
		     "clamps "
#endif
#if XCHAL_HAVE_MAC16
		     "mac16 "
#endif
#if XCHAL_HAVE_MUL16
		     "mul16 "
#endif
#if XCHAL_HAVE_MUL32
		     "mul32 "
#endif
#if XCHAL_HAVE_MUL32_HIGH
		     "mul32h "
#endif
#if XCHAL_HAVE_FP
		     "fpu "
#endif
#if XCHAL_HAVE_S32C1I
		     "s32c1i "
#endif
		     "\n");

	/* Registers. */
	seq_printf(f,"physical aregs\t: %d\n"
		     "misc regs\t: %d\n"
		     "ibreak\t\t: %d\n"
		     "dbreak\t\t: %d\n",
		     XCHAL_NUM_AREGS,
		     XCHAL_NUM_MISC_REGS,
		     XCHAL_NUM_IBREAK,
		     XCHAL_NUM_DBREAK);


	/* Interrupt. */
	seq_printf(f,"num ints\t: %d\n"
		     "ext ints\t: %d\n"
		     "int levels\t: %d\n"
		     "timers\t\t: %d\n"
		     "debug level\t: %d\n",
		     XCHAL_NUM_INTERRUPTS,
		     XCHAL_NUM_EXTINTERRUPTS,
		     XCHAL_NUM_INTLEVELS,
		     XCHAL_NUM_TIMERS,
		     XCHAL_DEBUGLEVEL);

	/* Cache */
	seq_printf(f,"icache line size: %d\n"
		     "icache ways\t: %d\n"
		     "icache size\t: %d\n"
		     "icache flags\t: "
#if XCHAL_ICACHE_LINE_LOCKABLE
		     "lock "
#endif
		     "\n"
		     "dcache line size: %d\n"
		     "dcache ways\t: %d\n"
		     "dcache size\t: %d\n"
		     "dcache flags\t: "
#if XCHAL_DCACHE_IS_WRITEBACK
		     "writeback "
#endif
#if XCHAL_DCACHE_LINE_LOCKABLE
		     "lock "
#endif
		     "\n",
		     XCHAL_ICACHE_LINESIZE,
		     XCHAL_ICACHE_WAYS,
		     XCHAL_ICACHE_SIZE,
		     XCHAL_DCACHE_LINESIZE,
		     XCHAL_DCACHE_WAYS,
		     XCHAL_DCACHE_SIZE);

	return 0;
}

/*
 * We show only CPU #0 info.
 */
static void *
c_start(struct seq_file *f, loff_t *pos)
{
	return (*pos == 0) ? (void *)1 : NULL;
}

static void *
c_next(struct seq_file *f, void *v, loff_t *pos)
{
	return NULL;
}

static void
c_stop(struct seq_file *f, void *v)
{
}

const struct seq_operations cpuinfo_op =
{
	.start	= c_start,
	.next	= c_next,
	.stop	= c_stop,
	.show	= c_show,
};

#endif /* CONFIG_PROC_FS */
