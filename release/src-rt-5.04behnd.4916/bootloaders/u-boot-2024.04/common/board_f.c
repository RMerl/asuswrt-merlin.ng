// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * (C) Copyright 2002-2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 */

#include <common.h>
#include <bloblist.h>
#include <bootstage.h>
#include <clock_legacy.h>
#include <console.h>
#include <cpu.h>
#include <cpu_func.h>
#include <cyclic.h>
#include <display_options.h>
#include <dm.h>
#include <env.h>
#include <env_internal.h>
#include <event.h>
#include <fdtdec.h>
#include <fs.h>
#include <hang.h>
#include <i2c.h>
#include <init.h>
#include <initcall.h>
#include <log.h>
#include <malloc.h>
#include <mapmem.h>
#include <os.h>
#include <post.h>
#include <relocate.h>
#include <serial.h>
#include <spl.h>
#include <status_led.h>
#include <sysreset.h>
#include <timer.h>
#include <trace.h>
#include <video.h>
#include <watchdog.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/sections.h>
#include <dm/root.h>
#include <linux/errno.h>
#include <linux/log2.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * TODO(sjg@chromium.org): IMO this code should be
 * refactored to a single function, something like:
 *
 * void led_set_state(enum led_colour_t colour, int on);
 */
/************************************************************************
 * Coloured LED functionality
 ************************************************************************
 * May be supplied by boards if desired
 */
__weak void coloured_LED_init(void) {}
__weak void red_led_on(void) {}
__weak void red_led_off(void) {}
__weak void green_led_on(void) {}
__weak void green_led_off(void) {}
__weak void yellow_led_on(void) {}
__weak void yellow_led_off(void) {}
__weak void blue_led_on(void) {}
__weak void blue_led_off(void) {}

/*
 * Why is gd allocated a register? Prior to reloc it might be better to
 * just pass it around to each function in this file?
 *
 * After reloc one could argue that it is hardly used and doesn't need
 * to be in a register. Or if it is it should perhaps hold pointers to all
 * global data for all modules, so that post-reloc we can avoid the massive
 * literal pool we get on ARM. Or perhaps just encourage each module to use
 * a structure...
 */

#if defined(CONFIG_WATCHDOG) || defined(CONFIG_HW_WATCHDOG)
static int init_func_watchdog_init(void)
{
# if defined(CONFIG_HW_WATCHDOG) && \
	(defined(CONFIG_M68K) || defined(CONFIG_MICROBLAZE) || \
	defined(CONFIG_SH) || \
	defined(CONFIG_DESIGNWARE_WATCHDOG) || \
	defined(CONFIG_IMX_WATCHDOG))
	hw_watchdog_init();
	puts("       Watchdog enabled\n");
# endif
	schedule();

	return 0;
}

int init_func_watchdog_reset(void)
{
	schedule();

	return 0;
}
#endif /* CONFIG_WATCHDOG */

__weak void board_add_ram_info(int use_default)
{
	/* please define platform specific board_add_ram_info() */
}

static int init_baud_rate(void)
{
	gd->baudrate = env_get_ulong("baudrate", 10, CONFIG_BAUDRATE);
	return 0;
}

static int display_text_info(void)
{
#if !defined(CONFIG_SANDBOX) && !defined(CONFIG_EFI_APP)
	ulong bss_start, bss_end, text_base;

	bss_start = (ulong)__bss_start;
	bss_end = (ulong)__bss_end;

#ifdef CONFIG_TEXT_BASE
	text_base = CONFIG_TEXT_BASE;
#else
	text_base = CONFIG_SYS_MONITOR_BASE;
#endif

	debug("U-Boot code: %08lX -> %08lX  BSS: -> %08lX\n",
	      text_base, bss_start, bss_end);
#endif

	return 0;
}

#ifdef CONFIG_SYSRESET
static int print_resetinfo(void)
{
	struct udevice *dev;
	char status[256];
	bool status_printed = false;
	int ret;

	/*
	 * Not all boards have sysreset drivers available during early
	 * boot, so don't fail if one can't be found.
	 */
	for (ret = uclass_first_device_check(UCLASS_SYSRESET, &dev); dev;
	     ret = uclass_next_device_check(&dev)) {
		if (ret) {
			debug("%s: %s sysreset device (error: %d)\n",
			      __func__, dev->name, ret);
			continue;
		}

		if (!sysreset_get_status(dev, status, sizeof(status))) {
			printf("%s%s", status_printed ? " " : "", status);
			status_printed = true;
		}
	}
	if (status_printed)
		printf("\n");

	return 0;
}
#endif

#if defined(CONFIG_DISPLAY_CPUINFO) && CONFIG_IS_ENABLED(CPU)
static int print_cpuinfo(void)
{
	struct udevice *dev;
	char desc[512];
	int ret;

	dev = cpu_get_current_dev();
	if (!dev) {
		debug("%s: Could not get CPU device\n",
		      __func__);
		return -ENODEV;
	}

	ret = cpu_get_desc(dev, desc, sizeof(desc));
	if (ret) {
		debug("%s: Could not get CPU description (err = %d)\n",
		      dev->name, ret);
		return ret;
	}

	printf("CPU:   %s\n", desc);

	return 0;
}
#endif

static int announce_dram_init(void)
{
	puts("DRAM:  ");
	return 0;
}

/*
 * From input size calculate its nearest rounded unit scale (multiply of 2^10)
 * and value in calculated unit scale multiplied by 10 (as fractional fixed
 * point number with one decimal digit), which is human natural format,
 * same what uses print_size() function for displaying. Mathematically it is:
 * round_nearest(val * 2^scale) = size * 10; where: 10 <= val < 10240.
 *
 * For example for size=87654321 we calculate scale=20 and val=836 which means
 * that input has natural human format 83.6 M (mega = 2^20).
 */
#define compute_size_scale_val(size, scale, val) do { \
	scale = ilog2(size) / 10 * 10; \
	val = (10 * size + ((1ULL << scale) >> 1)) >> scale; \
	if (val == 10240) { val = 10; scale += 10; } \
} while (0)

/*
 * Check if the sizes in their natural units written in decimal format with
 * one fraction number are same.
 */
static int sizes_near(unsigned long long size1, unsigned long long size2)
{
	unsigned int size1_scale, size1_val, size2_scale, size2_val;

	compute_size_scale_val(size1, size1_scale, size1_val);
	compute_size_scale_val(size2, size2_scale, size2_val);

	return size1_scale == size2_scale && size1_val == size2_val;
}

static int show_dram_config(void)
{
	unsigned long long size;
	int i;

	debug("\nRAM Configuration:\n");
	for (i = size = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		size += gd->bd->bi_dram[i].size;
		debug("Bank #%d: %llx ", i,
		      (unsigned long long)(gd->bd->bi_dram[i].start));
#ifdef DEBUG
		print_size(gd->bd->bi_dram[i].size, "\n");
#endif
	}
	debug("\nDRAM:  ");

	print_size(gd->ram_size, "");
	if (!sizes_near(gd->ram_size, size)) {
		printf(" (effective ");
		print_size(size, ")");
	}
	board_add_ram_info(0);
	putc('\n');

	return 0;
}

__weak int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = gd->ram_base;
	gd->bd->bi_dram[0].size = get_effective_memsize();

	return 0;
}

#if CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
static int init_func_i2c(void)
{
	puts("I2C:   ");
	i2c_init_all();
	puts("ready\n");
	return 0;
}
#endif

static int setup_mon_len(void)
{
#if defined(__ARM__) || defined(__MICROBLAZE__)
	gd->mon_len = (ulong)__bss_end - (ulong)_start;
#elif defined(CONFIG_SANDBOX) && !defined(__riscv)
	gd->mon_len = (ulong)_end - (ulong)_init;
#elif defined(CONFIG_SANDBOX)
	/* gcc does not provide _init in crti.o on RISC-V */
	gd->mon_len = 0;
#elif defined(CONFIG_EFI_APP)
	gd->mon_len = (ulong)_end - (ulong)_init;
#elif defined(CONFIG_NIOS2) || defined(CONFIG_XTENSA)
	gd->mon_len = CONFIG_SYS_MONITOR_LEN;
#elif defined(CONFIG_SH) || defined(CONFIG_RISCV)
	gd->mon_len = (ulong)(__bss_end) - (ulong)(_start);
#elif defined(CONFIG_SYS_MONITOR_BASE)
	/* TODO: use (ulong)__bss_end - (ulong)__text_start; ? */
	gd->mon_len = (ulong)__bss_end - CONFIG_SYS_MONITOR_BASE;
#endif
	return 0;
}

static int setup_spl_handoff(void)
{
#if CONFIG_IS_ENABLED(HANDOFF)
	gd->spl_handoff = bloblist_find(BLOBLISTT_U_BOOT_SPL_HANDOFF,
					sizeof(struct spl_handoff));
	debug("Found SPL hand-off info %p\n", gd->spl_handoff);
#endif

	return 0;
}

__weak int arch_cpu_init(void)
{
	return 0;
}

__weak int mach_cpu_init(void)
{
	return 0;
}

/* Get the top of usable RAM */
__weak phys_addr_t board_get_usable_ram_top(phys_size_t total_size)
{
#if defined(CFG_SYS_SDRAM_BASE) && CFG_SYS_SDRAM_BASE > 0
	/*
	 * Detect whether we have so much RAM that it goes past the end of our
	 * 32-bit address space. If so, clip the usable RAM so it doesn't.
	 */
	if (gd->ram_top < CFG_SYS_SDRAM_BASE)
		/*
		 * Will wrap back to top of 32-bit space when reservations
		 * are made.
		 */
		return 0;
#endif
	return gd->ram_top;
}

__weak int arch_setup_dest_addr(void)
{
	return 0;
}

static int setup_dest_addr(void)
{
	debug("Monitor len: %08lX\n", gd->mon_len);
	/*
	 * Ram is setup, size stored in gd !!
	 */
	debug("Ram size: %08llX\n", (unsigned long long)gd->ram_size);
#if CONFIG_VAL(SYS_MEM_TOP_HIDE)
	/*
	 * Subtract specified amount of memory to hide so that it won't
	 * get "touched" at all by U-Boot. By fixing up gd->ram_size
	 * the Linux kernel should now get passed the now "corrected"
	 * memory size and won't touch it either. This should work
	 * for arch/ppc and arch/powerpc. Only Linux board ports in
	 * arch/powerpc with bootwrapper support, that recalculate the
	 * memory size from the SDRAM controller setup will have to
	 * get fixed.
	 */
	gd->ram_size -= CONFIG_SYS_MEM_TOP_HIDE;
#endif
#ifdef CFG_SYS_SDRAM_BASE
	gd->ram_base = CFG_SYS_SDRAM_BASE;
#endif
	gd->ram_top = gd->ram_base + get_effective_memsize();
	gd->ram_top = board_get_usable_ram_top(gd->mon_len);
	gd->relocaddr = gd->ram_top;
	debug("Ram top: %08llX\n", (unsigned long long)gd->ram_top);

	return arch_setup_dest_addr();
}

#ifdef CFG_PRAM
/* reserve protected RAM */
static int reserve_pram(void)
{
	ulong reg;

	reg = env_get_ulong("pram", 10, CFG_PRAM);
	gd->relocaddr -= (reg << 10);		/* size is in kB */
	debug("Reserving %ldk for protected RAM at %08lx\n", reg,
	      gd->relocaddr);
	return 0;
}
#endif /* CFG_PRAM */

/* Round memory pointer down to next 4 kB limit */
static int reserve_round_4k(void)
{
	gd->relocaddr &= ~(4096 - 1);
	return 0;
}

__weak int arch_reserve_mmu(void)
{
	return 0;
}

static int reserve_video_from_videoblob(void)
{
	if (IS_ENABLED(CONFIG_SPL_VIDEO_HANDOFF) && spl_phase() > PHASE_SPL) {
		struct video_handoff *ho;
		int ret = 0;

		ho = bloblist_find(BLOBLISTT_U_BOOT_VIDEO, sizeof(*ho));
		if (!ho)
			return log_msg_ret("Missing video bloblist", -ENOENT);

		ret = video_reserve_from_bloblist(ho);
		if (ret)
			return log_msg_ret("Invalid Video handoff info", ret);

		/* Sanity check fb from blob is before current relocaddr */
		if (likely(gd->relocaddr > (unsigned long)ho->fb))
			gd->relocaddr = ho->fb;
	}

	return 0;
}

/*
 * Check if any bloblist received specifying reserved areas from previous stage and adjust
 * gd->relocaddr accordingly, so that we start reserving after pre-reserved areas
 * from previous stage.
 *
 * NOTE:
 * IT is recommended that all bloblists from previous stage are reserved from ram_top
 * as next stage will simply start reserving further regions after them.
 */
static int setup_relocaddr_from_bloblist(void)
{
	reserve_video_from_videoblob();

	return 0;
}

static int reserve_video(void)
{
	if (CONFIG_IS_ENABLED(VIDEO)) {
		ulong addr;
		int ret;

		addr = gd->relocaddr;
		ret = video_reserve(&addr);
		if (ret)
			return ret;
		debug("Reserving %luk for video at: %08lx\n",
		      ((unsigned long)gd->relocaddr - addr) >> 10, addr);
		gd->relocaddr = addr;
	}

	return 0;
}

static int reserve_trace(void)
{
#ifdef CONFIG_TRACE
	gd->relocaddr -= CONFIG_TRACE_BUFFER_SIZE;
	gd->trace_buff = map_sysmem(gd->relocaddr, CONFIG_TRACE_BUFFER_SIZE);
	debug("Reserving %luk for trace data at: %08lx\n",
	      (unsigned long)CONFIG_TRACE_BUFFER_SIZE >> 10, gd->relocaddr);
#endif

	return 0;
}

static int reserve_uboot(void)
{
	if (!(gd->flags & GD_FLG_SKIP_RELOC)) {
		/*
		 * reserve memory for U-Boot code, data & bss
		 * round down to next 4 kB limit
		 */
		gd->relocaddr -= gd->mon_len;
		gd->relocaddr &= ~(4096 - 1);
	#if defined(CONFIG_E500) || defined(CONFIG_MIPS)
		/* round down to next 64 kB limit so that IVPR stays aligned */
		gd->relocaddr &= ~(65536 - 1);
	#endif

		debug("Reserving %ldk for U-Boot at: %08lx\n",
		      gd->mon_len >> 10, gd->relocaddr);
	}

	gd->start_addr_sp = gd->relocaddr;

	return 0;
}

/*
 * reserve after start_addr_sp the requested size and make the stack pointer
 * 16-byte aligned, this alignment is needed for cast on the reserved memory
 * ref = x86_64 ABI: https://reviews.llvm.org/D30049: 16 bytes
 *     = ARMv8 Instruction Set Overview: quad word, 16 bytes
 */
static unsigned long reserve_stack_aligned(size_t size)
{
	return ALIGN_DOWN(gd->start_addr_sp - size, 16);
}

#ifdef CONFIG_SYS_NONCACHED_MEMORY
static int reserve_noncached(void)
{
	/*
	 * The value of gd->start_addr_sp must match the value of malloc_start
	 * calculated in board_r.c:initr_malloc(), which is passed to
	 * dlmalloc.c:mem_malloc_init() and then used by
	 * cache.c:noncached_init()
	 *
	 * These calculations must match the code in cache.c:noncached_init()
	 */
	gd->start_addr_sp = ALIGN(gd->start_addr_sp, MMU_SECTION_SIZE) -
		MMU_SECTION_SIZE;
	gd->start_addr_sp -= ALIGN(CONFIG_SYS_NONCACHED_MEMORY,
				   MMU_SECTION_SIZE);
	debug("Reserving %dM for noncached_alloc() at: %08lx\n",
	      CONFIG_SYS_NONCACHED_MEMORY >> 20, gd->start_addr_sp);

	return 0;
}
#endif

/* reserve memory for malloc() area */
static int reserve_malloc(void)
{
	gd->start_addr_sp = reserve_stack_aligned(TOTAL_MALLOC_LEN);
	debug("Reserving %dk for malloc() at: %08lx\n",
	      TOTAL_MALLOC_LEN >> 10, gd->start_addr_sp);
#ifdef CONFIG_SYS_NONCACHED_MEMORY
	reserve_noncached();
#endif

	return 0;
}

/* (permanently) allocate a Board Info struct */
static int reserve_board(void)
{
	if (!gd->bd) {
		gd->start_addr_sp = reserve_stack_aligned(sizeof(struct bd_info));
		gd->bd = (struct bd_info *)map_sysmem(gd->start_addr_sp,
						      sizeof(struct bd_info));
		memset(gd->bd, '\0', sizeof(struct bd_info));
		debug("Reserving %zu Bytes for Board Info at: %08lx\n",
		      sizeof(struct bd_info), gd->start_addr_sp);
	}
	return 0;
}

static int reserve_global_data(void)
{
	gd->start_addr_sp = reserve_stack_aligned(sizeof(gd_t));
	gd->new_gd = (gd_t *)map_sysmem(gd->start_addr_sp, sizeof(gd_t));
	debug("Reserving %zu Bytes for Global Data at: %08lx\n",
	      sizeof(gd_t), gd->start_addr_sp);
	return 0;
}

static int reserve_fdt(void)
{
	if (!IS_ENABLED(CONFIG_OF_EMBED)) {
		/*
		 * If the device tree is sitting immediately above our image
		 * then we must relocate it. If it is embedded in the data
		 * section, then it will be relocated with other data.
		 */
		if (gd->fdt_blob) {
			gd->fdt_size = ALIGN(fdt_totalsize(gd->fdt_blob), 32);

			gd->start_addr_sp = reserve_stack_aligned(gd->fdt_size);
			gd->new_fdt = map_sysmem(gd->start_addr_sp, gd->fdt_size);
			debug("Reserving %lu Bytes for FDT at: %08lx\n",
			      gd->fdt_size, gd->start_addr_sp);
		}
	}

	return 0;
}

static int reserve_bootstage(void)
{
#ifdef CONFIG_BOOTSTAGE
	int size = bootstage_get_size();

	gd->start_addr_sp = reserve_stack_aligned(size);
	gd->new_bootstage = map_sysmem(gd->start_addr_sp, size);
	debug("Reserving %#x Bytes for bootstage at: %08lx\n", size,
	      gd->start_addr_sp);
#endif

	return 0;
}

__weak int arch_reserve_stacks(void)
{
	return 0;
}

static int reserve_stacks(void)
{
	/* make stack pointer 16-byte aligned */
	gd->start_addr_sp = reserve_stack_aligned(16);

	/*
	 * let the architecture-specific code tailor gd->start_addr_sp and
	 * gd->irq_sp
	 */
	return arch_reserve_stacks();
}

static int reserve_bloblist(void)
{
#ifdef CONFIG_BLOBLIST
	/* Align to a 4KB boundary for easier reading of addresses */
	gd->start_addr_sp = ALIGN_DOWN(gd->start_addr_sp -
				       CONFIG_BLOBLIST_SIZE_RELOC, 0x1000);
	gd->new_bloblist = map_sysmem(gd->start_addr_sp,
				      CONFIG_BLOBLIST_SIZE_RELOC);
#endif

	return 0;
}

static int display_new_sp(void)
{
	debug("New Stack Pointer is: %08lx\n", gd->start_addr_sp);

	return 0;
}

__weak int arch_setup_bdinfo(void)
{
	return 0;
}

int setup_bdinfo(void)
{
	struct bd_info *bd = gd->bd;

	if (IS_ENABLED(CONFIG_SYS_HAS_SRAM)) {
		bd->bi_sramstart = CONFIG_SYS_SRAM_BASE; /* start of SRAM */
		bd->bi_sramsize = CONFIG_SYS_SRAM_SIZE;  /* size  of SRAM */
	}

	return arch_setup_bdinfo();
}

#ifdef CONFIG_POST
static int init_post(void)
{
	post_bootmode_init();
	post_run(NULL, POST_ROM | post_bootmode_get(0));

	return 0;
}
#endif

static int reloc_fdt(void)
{
	if (!IS_ENABLED(CONFIG_OF_EMBED)) {
		if (gd->new_fdt) {
			memcpy(gd->new_fdt, gd->fdt_blob,
			       fdt_totalsize(gd->fdt_blob));
			gd->fdt_blob = gd->new_fdt;
		}
	}

	return 0;
}

static int reloc_bootstage(void)
{
#ifdef CONFIG_BOOTSTAGE
	if (gd->flags & GD_FLG_SKIP_RELOC)
		return 0;
	if (gd->new_bootstage) {
		int size = bootstage_get_size();

		debug("Copying bootstage from %p to %p, size %x\n",
		      gd->bootstage, gd->new_bootstage, size);
		memcpy(gd->new_bootstage, gd->bootstage, size);
		gd->bootstage = gd->new_bootstage;
		bootstage_relocate();
	}
#endif

	return 0;
}

static int reloc_bloblist(void)
{
#ifdef CONFIG_BLOBLIST
	/*
	 * Relocate only if we are supposed to send it
	 */
	if ((gd->flags & GD_FLG_SKIP_RELOC) &&
	    CONFIG_BLOBLIST_SIZE == CONFIG_BLOBLIST_SIZE_RELOC) {
		debug("Not relocating bloblist\n");
		return 0;
	}
	if (gd->new_bloblist) {
		int size = CONFIG_BLOBLIST_SIZE;

		debug("Copying bloblist from %p to %p, size %x\n",
		      gd->bloblist, gd->new_bloblist, size);
		bloblist_reloc(gd->new_bloblist, CONFIG_BLOBLIST_SIZE_RELOC,
			       gd->bloblist, size);
		gd->bloblist = gd->new_bloblist;
	}
#endif

	return 0;
}

static int setup_reloc(void)
{
	if (!(gd->flags & GD_FLG_SKIP_RELOC)) {
#ifdef CONFIG_TEXT_BASE
#ifdef ARM
		gd->reloc_off = gd->relocaddr - (unsigned long)__image_copy_start;
#elif defined(CONFIG_MICROBLAZE)
		gd->reloc_off = gd->relocaddr - (u32)_start;
#elif defined(CONFIG_M68K)
		/*
		 * On all ColdFire arch cpu, monitor code starts always
		 * just after the default vector table location, so at 0x400
		 */
		gd->reloc_off = gd->relocaddr - (CONFIG_TEXT_BASE + 0x400);
#elif !defined(CONFIG_SANDBOX)
		gd->reloc_off = gd->relocaddr - CONFIG_TEXT_BASE;
#endif
#endif
	}

	memcpy(gd->new_gd, (char *)gd, sizeof(gd_t));

	if (gd->flags & GD_FLG_SKIP_RELOC) {
		debug("Skipping relocation due to flag\n");
	} else {
		debug("Relocation Offset is: %08lx\n", gd->reloc_off);
		debug("Relocating to %08lx, new gd at %08lx, sp at %08lx\n",
		      gd->relocaddr, (ulong)map_to_sysmem(gd->new_gd),
		      gd->start_addr_sp);
	}

	return 0;
}

#ifdef CONFIG_OF_BOARD_FIXUP
static int fix_fdt(void)
{
	return board_fix_fdt((void *)gd->fdt_blob);
}
#endif

/* ARM calls relocate_code from its crt0.S */
#if !defined(CONFIG_ARM) && !defined(CONFIG_SANDBOX)

static int jump_to_copy(void)
{
	if (gd->flags & GD_FLG_SKIP_RELOC)
		return 0;
	/*
	 * x86 is special, but in a nice way. It uses a trampoline which
	 * enables the dcache if possible.
	 *
	 * For now, other archs use relocate_code(), which is implemented
	 * similarly for all archs. When we do generic relocation, hopefully
	 * we can make all archs enable the dcache prior to relocation.
	 */
#if defined(CONFIG_X86) || defined(CONFIG_ARC)
	/*
	 * SDRAM and console are now initialised. The final stack can now
	 * be setup in SDRAM. Code execution will continue in Flash, but
	 * with the stack in SDRAM and Global Data in temporary memory
	 * (CPU cache)
	 */
	arch_setup_gd(gd->new_gd);
# if CONFIG_IS_ENABLED(X86_64)
		board_init_f_r_trampoline64(gd->new_gd, gd->start_addr_sp);
# else
		board_init_f_r_trampoline(gd->start_addr_sp);
# endif
#else
	relocate_code(gd->start_addr_sp, gd->new_gd, gd->relocaddr);
#endif

	return 0;
}
#endif

/* Record the board_init_f() bootstage (after arch_cpu_init()) */
static int initf_bootstage(void)
{
	bool from_spl = IS_ENABLED(CONFIG_SPL_BOOTSTAGE) &&
			IS_ENABLED(CONFIG_BOOTSTAGE_STASH);
	int ret;

	ret = bootstage_init(!from_spl);
	if (ret)
		return ret;
	if (from_spl) {
		const void *stash = map_sysmem(CONFIG_BOOTSTAGE_STASH_ADDR,
					       CONFIG_BOOTSTAGE_STASH_SIZE);

		ret = bootstage_unstash(stash, CONFIG_BOOTSTAGE_STASH_SIZE);
		if (ret && ret != -ENOENT) {
			debug("Failed to unstash bootstage: err=%d\n", ret);
			return ret;
		}
	}

	bootstage_mark_name(BOOTSTAGE_ID_START_UBOOT_F, "board_init_f");

	return 0;
}

static int initf_dm(void)
{
#if defined(CONFIG_DM) && CONFIG_IS_ENABLED(SYS_MALLOC_F)
	int ret;

	bootstage_start(BOOTSTAGE_ID_ACCUM_DM_F, "dm_f");
	ret = dm_init_and_scan(true);
	bootstage_accum(BOOTSTAGE_ID_ACCUM_DM_F);
	if (ret)
		return ret;

	if (IS_ENABLED(CONFIG_TIMER_EARLY)) {
		ret = dm_timer_init();
		if (ret)
			return ret;
	}
#endif

	return 0;
}

/* Architecture-specific memory reservation */
__weak int reserve_arch(void)
{
	return 0;
}

__weak int checkcpu(void)
{
	return 0;
}

__weak int clear_bss(void)
{
	return 0;
}

static const init_fnc_t init_sequence_f[] = {
	setup_mon_len,
#ifdef CONFIG_OF_CONTROL
	fdtdec_setup,
#endif
#ifdef CONFIG_TRACE_EARLY
	trace_early_init,
#endif
	initf_malloc,
	log_init,
	initf_bootstage,	/* uses its own timer, so does not need DM */
	event_init,
	bloblist_maybe_init,
	setup_spl_handoff,
#if defined(CONFIG_CONSOLE_RECORD_INIT_F)
	console_record_init,
#endif
	INITCALL_EVENT(EVT_FSP_INIT_F),
	arch_cpu_init,		/* basic arch cpu dependent setup */
	mach_cpu_init,		/* SoC/machine dependent CPU setup */
	initf_dm,
#if defined(CONFIG_BOARD_EARLY_INIT_F)
	board_early_init_f,
#endif
#if defined(CONFIG_PPC) || defined(CONFIG_SYS_FSL_CLK) || defined(CONFIG_M68K)
	/* get CPU and bus clocks according to the environment variable */
	get_clocks,		/* get CPU and bus clocks (etc.) */
#endif
#if !defined(CONFIG_M68K) || (defined(CONFIG_M68K) && !defined(CONFIG_MCFTMR))
	timer_init,		/* initialize timer */
#endif
#if defined(CONFIG_BOARD_POSTCLK_INIT)
	board_postclk_init,
#endif
	env_init,		/* initialize environment */
	init_baud_rate,		/* initialze baudrate settings */
	serial_init,		/* serial communications setup */
	console_init_f,		/* stage 1 init of console */
	display_options,	/* say that we are here */
	display_text_info,	/* show debugging info if required */
	checkcpu,
#if defined(CONFIG_SYSRESET)
	print_resetinfo,
#endif
#if defined(CONFIG_DISPLAY_CPUINFO)
	print_cpuinfo,		/* display cpu info (and speed) */
#endif
#if defined(CONFIG_DTB_RESELECT)
	embedded_dtb_select,
#endif
#if defined(CONFIG_DISPLAY_BOARDINFO)
	show_board_info,
#endif
	INIT_FUNC_WATCHDOG_INIT
	INITCALL_EVENT(EVT_MISC_INIT_F),
	INIT_FUNC_WATCHDOG_RESET
#if CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
	init_func_i2c,
#endif
	announce_dram_init,
	dram_init,		/* configure available RAM banks */
#ifdef CONFIG_POST
	post_init_f,
#endif
	INIT_FUNC_WATCHDOG_RESET
#if defined(CFG_SYS_DRAM_TEST)
	testdram,
#endif /* CFG_SYS_DRAM_TEST */
	INIT_FUNC_WATCHDOG_RESET

#ifdef CONFIG_POST
	init_post,
#endif
	INIT_FUNC_WATCHDOG_RESET
	/*
	 * Now that we have DRAM mapped and working, we can
	 * relocate the code and continue running from DRAM.
	 *
	 * Reserve memory at end of RAM for (top down in that order):
	 *  - area that won't get touched by U-Boot and Linux (optional)
	 *  - kernel log buffer
	 *  - protected RAM
	 *  - LCD framebuffer
	 *  - monitor code
	 *  - board info struct
	 */
	setup_dest_addr,
#ifdef CONFIG_OF_BOARD_FIXUP
	fix_fdt,
#endif
#ifdef CFG_PRAM
	reserve_pram,
#endif
	reserve_round_4k,
	setup_relocaddr_from_bloblist,
	arch_reserve_mmu,
	reserve_video,
	reserve_trace,
	reserve_uboot,
	reserve_malloc,
	reserve_board,
	reserve_global_data,
	reserve_fdt,
	reserve_bootstage,
	reserve_bloblist,
	reserve_arch,
	reserve_stacks,
	dram_init_banksize,
	show_dram_config,
	INIT_FUNC_WATCHDOG_RESET
	setup_bdinfo,
	display_new_sp,
	INIT_FUNC_WATCHDOG_RESET
	reloc_fdt,
	reloc_bootstage,
	reloc_bloblist,
	setup_reloc,
#if defined(CONFIG_X86) || defined(CONFIG_ARC)
	copy_uboot_to_ram,
	do_elf_reloc_fixups,
#endif
	clear_bss,
	/*
	 * Deregister all cyclic functions before relocation, so that
	 * gd->cyclic_list does not contain any references to pre-relocation
	 * devices. Drivers will register their cyclic functions anew when the
	 * devices are probed again.
	 *
	 * This should happen as late as possible so that the window where a
	 * watchdog device is not serviced is as small as possible.
	 */
	cyclic_unregister_all,
#if !defined(CONFIG_ARM) && !defined(CONFIG_SANDBOX)
	jump_to_copy,
#endif
	NULL,
};

void board_init_f(ulong boot_flags)
{
	gd->flags = boot_flags;
	gd->have_console = 0;

	if (initcall_run_list(init_sequence_f))
		hang();

#if !defined(CONFIG_ARM) && !defined(CONFIG_SANDBOX) && \
		!defined(CONFIG_EFI_APP) && !CONFIG_IS_ENABLED(X86_64) && \
		!defined(CONFIG_ARC)
	/* NOTREACHED - jump_to_copy() does not return */
	hang();
#endif
}

#if defined(CONFIG_X86) || defined(CONFIG_ARC)
/*
 * For now this code is only used on x86.
 *
 * init_sequence_f_r is the list of init functions which are run when
 * U-Boot is executing from Flash with a semi-limited 'C' environment.
 * The following limitations must be considered when implementing an
 * '_f_r' function:
 *  - 'static' variables are read-only
 *  - Global Data (gd->xxx) is read/write
 *
 * The '_f_r' sequence must, as a minimum, copy U-Boot to RAM (if
 * supported).  It _should_, if possible, copy global data to RAM and
 * initialise the CPU caches (to speed up the relocation process)
 *
 * NOTE: At present only x86 uses this route, but it is intended that
 * all archs will move to this when generic relocation is implemented.
 */
static const init_fnc_t init_sequence_f_r[] = {
#if !CONFIG_IS_ENABLED(X86_64)
	init_cache_f_r,
#endif

	NULL,
};

void board_init_f_r(void)
{
	if (initcall_run_list(init_sequence_f_r))
		hang();

	/*
	 * The pre-relocation drivers may be using memory that has now gone
	 * away. Mark serial as unavailable - this will fall back to the debug
	 * UART if available.
	 *
	 * Do the same with log drivers since the memory may not be available.
	 */
	gd->flags &= ~(GD_FLG_SERIAL_READY | GD_FLG_LOG_READY);
#ifdef CONFIG_TIMER
	gd->timer = NULL;
#endif

	/*
	 * U-Boot has been copied into SDRAM, the BSS has been cleared etc.
	 * Transfer execution from Flash to RAM by calculating the address
	 * of the in-RAM copy of board_init_r() and calling it
	 */
	(board_init_r + gd->reloc_off)((gd_t *)gd, gd->relocaddr);

	/* NOTREACHED - board_init_r() does not return */
	hang();
}
#endif /* CONFIG_X86 */
