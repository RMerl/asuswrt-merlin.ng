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
#include <console.h>
#include <cpu.h>
#include <dm.h>
#include <environment.h>
#include <fdtdec.h>
#include <fs.h>
#include <i2c.h>
#include <initcall.h>
#include <malloc.h>
#include <mapmem.h>
#include <os.h>
#include <post.h>
#include <relocate.h>
#ifdef CONFIG_SPL
#include <spl.h>
#endif
#include <status_led.h>
#include <sysreset.h>
#include <timer.h>
#include <trace.h>
#include <video.h>
#include <watchdog.h>
#ifdef CONFIG_MACH_TYPE
#include <asm/mach-types.h>
#endif
#if defined(CONFIG_MP) && defined(CONFIG_PPC)
#include <asm/mp.h>
#endif
#include <asm/io.h>
#include <asm/sections.h>
#include <dm/root.h>
#include <linux/errno.h>

/*
 * Pointer to initial global data area
 *
 * Here we initialize it if needed.
 */
#ifdef XTRN_DECLARE_GLOBAL_DATA_PTR
#undef	XTRN_DECLARE_GLOBAL_DATA_PTR
#define XTRN_DECLARE_GLOBAL_DATA_PTR	/* empty = allocate here */
DECLARE_GLOBAL_DATA_PTR = (gd_t *)(CONFIG_SYS_INIT_GD_ADDR);
#else
DECLARE_GLOBAL_DATA_PTR;
#endif

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
	WATCHDOG_RESET();

	return 0;
}

int init_func_watchdog_reset(void)
{
	WATCHDOG_RESET();

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

	bss_start = (ulong)&__bss_start;
	bss_end = (ulong)&__bss_end;

#ifdef CONFIG_SYS_TEXT_BASE
	text_base = CONFIG_SYS_TEXT_BASE;
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
	int ret;

	ret = uclass_first_device_err(UCLASS_SYSRESET, &dev);
	if (ret) {
		debug("%s: No sysreset device found (error: %d)\n",
		      __func__, ret);
		/* Not all boards have sysreset drivers available during early
		 * boot, so don't fail if one can't be found.
		 */
		return 0;
	}

	if (!sysreset_get_status(dev, status, sizeof(status)))
		printf("%s", status);

	return 0;
}
#endif

#if defined(CONFIG_DISPLAY_CPUINFO) && CONFIG_IS_ENABLED(CPU)
static int print_cpuinfo(void)
{
	struct udevice *dev;
	char desc[512];
	int ret;

	ret = uclass_first_device_err(UCLASS_CPU, &dev);
	if (ret) {
		debug("%s: Could not get CPU device (err = %d)\n",
		      __func__, ret);
		return ret;
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

static int show_dram_config(void)
{
	unsigned long long size;

#ifdef CONFIG_NR_DRAM_BANKS
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
#else
	size = gd->ram_size;
#endif

	print_size(size, "");
	board_add_ram_info(0);
	putc('\n');

	return 0;
}

__weak int dram_init_banksize(void)
{
#if defined(CONFIG_NR_DRAM_BANKS) && defined(CONFIG_SYS_SDRAM_BASE)
	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size = get_effective_memsize();
#endif

	return 0;
}

#if defined(CONFIG_SYS_I2C)
static int init_func_i2c(void)
{
	puts("I2C:   ");
#ifdef CONFIG_SYS_I2C
	i2c_init_all();
#else
	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
#endif
	puts("ready\n");
	return 0;
}
#endif

#if defined(CONFIG_VID)
__weak int init_func_vid(void)
{
	return 0;
}
#endif

static int setup_mon_len(void)
{
#if defined(__ARM__) || defined(__MICROBLAZE__)
	gd->mon_len = (ulong)&__bss_end - (ulong)_start;
#elif defined(CONFIG_SANDBOX) || defined(CONFIG_EFI_APP)
	gd->mon_len = (ulong)&_end - (ulong)_init;
#elif defined(CONFIG_NIOS2) || defined(CONFIG_XTENSA)
	gd->mon_len = CONFIG_SYS_MONITOR_LEN;
#elif defined(CONFIG_NDS32) || defined(CONFIG_SH) || defined(CONFIG_RISCV)
	gd->mon_len = (ulong)(&__bss_end) - (ulong)(&_start);
#elif defined(CONFIG_SYS_MONITOR_BASE)
	/* TODO: use (ulong)&__bss_end - (ulong)&__text_start; ? */
	gd->mon_len = (ulong)&__bss_end - CONFIG_SYS_MONITOR_BASE;
#endif
	return 0;
}

static int setup_spl_handoff(void)
{
#if CONFIG_IS_ENABLED(HANDOFF)
	gd->spl_handoff = bloblist_find(BLOBLISTT_SPL_HANDOFF,
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
__weak ulong board_get_usable_ram_top(ulong total_size)
{
#ifdef CONFIG_SYS_SDRAM_BASE
	/*
	 * Detect whether we have so much RAM that it goes past the end of our
	 * 32-bit address space. If so, clip the usable RAM so it doesn't.
	 */
	if (gd->ram_top < CONFIG_SYS_SDRAM_BASE)
		/*
		 * Will wrap back to top of 32-bit space when reservations
		 * are made.
		 */
		return 0;
#endif
	return gd->ram_top;
}

static int setup_dest_addr(void)
{
	debug("Monitor len: %08lX\n", gd->mon_len);
	/*
	 * Ram is setup, size stored in gd !!
	 */
	debug("Ram size: %08lX\n", (ulong)gd->ram_size);
#if defined(CONFIG_SYS_MEM_TOP_HIDE)
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
#ifdef CONFIG_SYS_SDRAM_BASE
	gd->ram_base = CONFIG_SYS_SDRAM_BASE;
#endif
	gd->ram_top = gd->ram_base + get_effective_memsize();
	gd->ram_top = board_get_usable_ram_top(gd->mon_len);
	gd->relocaddr = gd->ram_top;
	debug("Ram top: %08lX\n", (ulong)gd->ram_top);
#if defined(CONFIG_MP) && (defined(CONFIG_MPC86xx) || defined(CONFIG_E500))
	/*
	 * We need to make sure the location we intend to put secondary core
	 * boot code is reserved and not used by any part of u-boot
	 */
	if (gd->relocaddr > determine_mp_bootpg(NULL)) {
		gd->relocaddr = determine_mp_bootpg(NULL);
		debug("Reserving MP boot page to %08lx\n", gd->relocaddr);
	}
#endif
	return 0;
}

#ifdef CONFIG_PRAM
/* reserve protected RAM */
static int reserve_pram(void)
{
	ulong reg;

	reg = env_get_ulong("pram", 10, CONFIG_PRAM);
	gd->relocaddr -= (reg << 10);		/* size is in kB */
	debug("Reserving %ldk for protected RAM at %08lx\n", reg,
	      gd->relocaddr);
	return 0;
}
#endif /* CONFIG_PRAM */

/* Round memory pointer down to next 4 kB limit */
static int reserve_round_4k(void)
{
	gd->relocaddr &= ~(4096 - 1);
	return 0;
}

#ifdef CONFIG_ARM
__weak int reserve_mmu(void)
{
#if !(CONFIG_IS_ENABLED(SYS_ICACHE_OFF) && CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
	/* reserve TLB table */
	gd->arch.tlb_size = PGTABLE_SIZE;
	gd->relocaddr -= gd->arch.tlb_size;

	/* round down to next 64 kB limit */
	gd->relocaddr &= ~(0x10000 - 1);

	gd->arch.tlb_addr = gd->relocaddr;
	debug("TLB table from %08lx to %08lx\n", gd->arch.tlb_addr,
	      gd->arch.tlb_addr + gd->arch.tlb_size);

#ifdef CONFIG_SYS_MEM_RESERVE_SECURE
	/*
	 * Record allocated tlb_addr in case gd->tlb_addr to be overwritten
	 * with location within secure ram.
	 */
	gd->arch.tlb_allocated = gd->arch.tlb_addr;
#endif
#endif

	return 0;
}
#endif

static int reserve_video(void)
{
#ifdef CONFIG_DM_VIDEO
	ulong addr;
	int ret;

	addr = gd->relocaddr;
	ret = video_reserve(&addr);
	if (ret)
		return ret;
	gd->relocaddr = addr;
#elif defined(CONFIG_LCD)
#  ifdef CONFIG_FB_ADDR
	gd->fb_base = CONFIG_FB_ADDR;
#  else
	/* reserve memory for LCD display (always full pages) */
	gd->relocaddr = lcd_setmem(gd->relocaddr);
	gd->fb_base = gd->relocaddr;
#  endif /* CONFIG_FB_ADDR */
#elif defined(CONFIG_VIDEO) && \
		(!defined(CONFIG_PPC)) && \
		!defined(CONFIG_ARM) && !defined(CONFIG_X86) && \
		!defined(CONFIG_M68K)
	/* reserve memory for video display (always full pages) */
	gd->relocaddr = video_setmem(gd->relocaddr);
	gd->fb_base = gd->relocaddr;
#endif

	return 0;
}

static int reserve_trace(void)
{
#ifdef CONFIG_TRACE
	gd->relocaddr -= CONFIG_TRACE_BUFFER_SIZE;
	gd->trace_buff = map_sysmem(gd->relocaddr, CONFIG_TRACE_BUFFER_SIZE);
	debug("Reserving %dk for trace data at: %08lx\n",
	      CONFIG_TRACE_BUFFER_SIZE >> 10, gd->relocaddr);
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

#ifdef CONFIG_SYS_NONCACHED_MEMORY
static int reserve_noncached(void)
{
	/*
	 * The value of gd->start_addr_sp must match the value of malloc_start
	 * calculated in boatrd_f.c:initr_malloc(), which is passed to
	 * board_r.c:mem_malloc_init() and then used by
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
	gd->start_addr_sp = gd->start_addr_sp - TOTAL_MALLOC_LEN;
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
		gd->start_addr_sp -= sizeof(bd_t);
		gd->bd = (bd_t *)map_sysmem(gd->start_addr_sp, sizeof(bd_t));
		memset(gd->bd, '\0', sizeof(bd_t));
		debug("Reserving %zu Bytes for Board Info at: %08lx\n",
		      sizeof(bd_t), gd->start_addr_sp);
	}
	return 0;
}

static int setup_machine(void)
{
#ifdef CONFIG_MACH_TYPE
	gd->bd->bi_arch_number = CONFIG_MACH_TYPE; /* board id for Linux */
#endif
	return 0;
}

static int reserve_global_data(void)
{
	gd->start_addr_sp -= sizeof(gd_t);
	gd->new_gd = (gd_t *)map_sysmem(gd->start_addr_sp, sizeof(gd_t));
	debug("Reserving %zu Bytes for Global Data at: %08lx\n",
	      sizeof(gd_t), gd->start_addr_sp);
	return 0;
}

static int reserve_fdt(void)
{
#ifndef CONFIG_OF_EMBED
	/*
	 * If the device tree is sitting immediately above our image then we
	 * must relocate it. If it is embedded in the data section, then it
	 * will be relocated with other data.
	 */
	if (gd->fdt_blob) {
		gd->fdt_size = ALIGN(fdt_totalsize(gd->fdt_blob) + 0x1000, 32);

		gd->start_addr_sp -= gd->fdt_size;
		gd->new_fdt = map_sysmem(gd->start_addr_sp, gd->fdt_size);
		debug("Reserving %lu Bytes for FDT at: %08lx\n",
		      gd->fdt_size, gd->start_addr_sp);
	}
#endif

	return 0;
}

static int reserve_bootstage(void)
{
#ifdef CONFIG_BOOTSTAGE
	int size = bootstage_get_size();

	gd->start_addr_sp -= size;
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
	gd->start_addr_sp -= 16;
	gd->start_addr_sp &= ~0xf;

	/*
	 * let the architecture-specific code tailor gd->start_addr_sp and
	 * gd->irq_sp
	 */
	return arch_reserve_stacks();
}

static int reserve_bloblist(void)
{
#ifdef CONFIG_BLOBLIST
	gd->start_addr_sp -= CONFIG_BLOBLIST_SIZE;
	gd->new_bloblist = map_sysmem(gd->start_addr_sp, CONFIG_BLOBLIST_SIZE);
#endif

	return 0;
}

static int display_new_sp(void)
{
	debug("New Stack Pointer is: %08lx\n", gd->start_addr_sp);

	return 0;
}

#if defined(CONFIG_M68K) || defined(CONFIG_MIPS) || defined(CONFIG_PPC) || \
	defined(CONFIG_SH)
static int setup_board_part1(void)
{
	bd_t *bd = gd->bd;

	/*
	 * Save local variables to board info struct
	 */
	bd->bi_memstart = CONFIG_SYS_SDRAM_BASE;	/* start of memory */
	bd->bi_memsize = gd->ram_size;			/* size in bytes */

#ifdef CONFIG_SYS_SRAM_BASE
	bd->bi_sramstart = CONFIG_SYS_SRAM_BASE;	/* start of SRAM */
	bd->bi_sramsize = CONFIG_SYS_SRAM_SIZE;		/* size  of SRAM */
#endif

#if defined(CONFIG_E500) || defined(CONFIG_MPC86xx)
	bd->bi_immr_base = CONFIG_SYS_IMMR;	/* base  of IMMR register     */
#endif
#if defined(CONFIG_M68K)
	bd->bi_mbar_base = CONFIG_SYS_MBAR;	/* base of internal registers */
#endif
#if defined(CONFIG_MPC83xx)
	bd->bi_immrbar = CONFIG_SYS_IMMR;
#endif

	return 0;
}
#endif

#if defined(CONFIG_PPC) || defined(CONFIG_M68K)
static int setup_board_part2(void)
{
	bd_t *bd = gd->bd;

	bd->bi_intfreq = gd->cpu_clk;	/* Internal Freq, in Hz */
	bd->bi_busfreq = gd->bus_clk;	/* Bus Freq,      in Hz */
#if defined(CONFIG_CPM2)
	bd->bi_cpmfreq = gd->arch.cpm_clk;
	bd->bi_brgfreq = gd->arch.brg_clk;
	bd->bi_sccfreq = gd->arch.scc_clk;
	bd->bi_vco = gd->arch.vco_out;
#endif /* CONFIG_CPM2 */
#if defined(CONFIG_M68K) && defined(CONFIG_PCI)
	bd->bi_pcifreq = gd->pci_clk;
#endif
#if defined(CONFIG_EXTRA_CLOCK)
	bd->bi_inpfreq = gd->arch.inp_clk;	/* input Freq in Hz */
	bd->bi_vcofreq = gd->arch.vco_clk;	/* vco Freq in Hz */
	bd->bi_flbfreq = gd->arch.flb_clk;	/* flexbus Freq in Hz */
#endif

	return 0;
}
#endif

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
#ifndef CONFIG_OF_EMBED
	if (gd->flags & GD_FLG_SKIP_RELOC)
		return 0;
	if (gd->new_fdt) {
		memcpy(gd->new_fdt, gd->fdt_blob, gd->fdt_size);
		gd->fdt_blob = gd->new_fdt;
	}
#endif

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
	}
#endif

	return 0;
}

static int reloc_bloblist(void)
{
#ifdef CONFIG_BLOBLIST
	if (gd->flags & GD_FLG_SKIP_RELOC)
		return 0;
	if (gd->new_bloblist) {
		int size = CONFIG_BLOBLIST_SIZE;

		debug("Copying bloblist from %p to %p, size %x\n",
		      gd->bloblist, gd->new_bloblist, size);
		memcpy(gd->new_bloblist, gd->bloblist, size);
		gd->bloblist = gd->new_bloblist;
	}
#endif

	return 0;
}

static int setup_reloc(void)
{
	if (gd->flags & GD_FLG_SKIP_RELOC) {
		debug("Skipping relocation due to flag\n");
		return 0;
	}

#ifdef CONFIG_SYS_TEXT_BASE
#ifdef ARM
	gd->reloc_off = gd->relocaddr - (unsigned long)__image_copy_start;
#elif defined(CONFIG_M68K)
	/*
	 * On all ColdFire arch cpu, monitor code starts always
	 * just after the default vector table location, so at 0x400
	 */
	gd->reloc_off = gd->relocaddr - (CONFIG_SYS_TEXT_BASE + 0x400);
#elif !defined(CONFIG_SANDBOX)
	gd->reloc_off = gd->relocaddr - CONFIG_SYS_TEXT_BASE;
#endif
#endif
	memcpy(gd->new_gd, (char *)gd, sizeof(gd_t));

	debug("Relocation Offset is: %08lx\n", gd->reloc_off);
	debug("Relocating to %08lx, new gd at %08lx, sp at %08lx\n",
	      gd->relocaddr, (ulong)map_to_sysmem(gd->new_gd),
	      gd->start_addr_sp);

	return 0;
}

#ifdef CONFIG_OF_BOARD_FIXUP
static int fix_fdt(void)
{
	return board_fix_fdt((void *)gd->fdt_blob);
}
#endif

/* ARM calls relocate_code from its crt0.S */
#if !defined(CONFIG_ARM) && !defined(CONFIG_SANDBOX) && \
		!CONFIG_IS_ENABLED(X86_64)

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
	board_init_f_r_trampoline(gd->start_addr_sp);
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

static int initf_console_record(void)
{
#if defined(CONFIG_CONSOLE_RECORD) && CONFIG_VAL(SYS_MALLOC_F_LEN)
	return console_record_init();
#else
	return 0;
#endif
}

static int initf_dm(void)
{
#if defined(CONFIG_DM) && CONFIG_VAL(SYS_MALLOC_F_LEN)
	int ret;

	bootstage_start(BOOTSTATE_ID_ACCUM_DM_F, "dm_f");
	ret = dm_init_and_scan(true);
	bootstage_accum(BOOTSTATE_ID_ACCUM_DM_F);
	if (ret)
		return ret;
#endif
#ifdef CONFIG_TIMER_EARLY
	ret = dm_timer_init();
	if (ret)
		return ret;
#endif

	return 0;
}

/* Architecture-specific memory reservation */
__weak int reserve_arch(void)
{
	return 0;
}

__weak int arch_cpu_init_dm(void)
{
	return 0;
}

static const init_fnc_t init_sequence_f[] = {
	setup_mon_len,
#ifdef CONFIG_OF_CONTROL
	fdtdec_setup,
#endif
#ifdef CONFIG_TRACE
	trace_early_init,
#endif
	initf_malloc,
	log_init,
	initf_bootstage,	/* uses its own timer, so does not need DM */
#ifdef CONFIG_BLOBLIST
	bloblist_init,
#endif
	setup_spl_handoff,
	initf_console_record,
#if defined(CONFIG_HAVE_FSP)
	arch_fsp_init,
#endif
	arch_cpu_init,		/* basic arch cpu dependent setup */
	mach_cpu_init,		/* SoC/machine dependent CPU setup */
	initf_dm,
	arch_cpu_init_dm,
#if defined(CONFIG_BOARD_EARLY_INIT_F)
	board_early_init_f,
#endif
#if defined(CONFIG_PPC) || defined(CONFIG_SYS_FSL_CLK) || defined(CONFIG_M68K)
	/* get CPU and bus clocks according to the environment variable */
	get_clocks,		/* get CPU and bus clocks (etc.) */
#endif
#if !defined(CONFIG_M68K)
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
#if defined(CONFIG_PPC) || defined(CONFIG_SH) || defined(CONFIG_X86)
	checkcpu,
#endif
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
#if defined(CONFIG_MISC_INIT_F)
	misc_init_f,
#endif
	INIT_FUNC_WATCHDOG_RESET
#if defined(CONFIG_SYS_I2C)
	init_func_i2c,
#endif
#if defined(CONFIG_VID) && !defined(CONFIG_SPL)
	init_func_vid,
#endif
	announce_dram_init,
	dram_init,		/* configure available RAM banks */
#ifdef CONFIG_POST
	post_init_f,
#endif
	INIT_FUNC_WATCHDOG_RESET
#if defined(CONFIG_SYS_DRAM_TEST)
	testdram,
#endif /* CONFIG_SYS_DRAM_TEST */
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
#ifdef CONFIG_PRAM
	reserve_pram,
#endif
	reserve_round_4k,
#ifdef CONFIG_ARM
	reserve_mmu,
#endif
	reserve_video,
	reserve_trace,
	reserve_uboot,
	reserve_malloc,
	reserve_board,
	setup_machine,
	reserve_global_data,
	reserve_fdt,
	reserve_bootstage,
	reserve_bloblist,
	reserve_arch,
	reserve_stacks,
	dram_init_banksize,
	show_dram_config,
#if defined(CONFIG_M68K) || defined(CONFIG_MIPS) || defined(CONFIG_PPC) || \
	defined(CONFIG_SH)
	setup_board_part1,
#endif
#if defined(CONFIG_PPC) || defined(CONFIG_M68K)
	INIT_FUNC_WATCHDOG_RESET
	setup_board_part2,
#endif
	display_new_sp,
#ifdef CONFIG_OF_BOARD_FIXUP
	fix_fdt,
#endif
	INIT_FUNC_WATCHDOG_RESET
	reloc_fdt,
	reloc_bootstage,
	reloc_bloblist,
	setup_reloc,
#if defined(CONFIG_X86) || defined(CONFIG_ARC)
	copy_uboot_to_ram,
	do_elf_reloc_fixups,
	clear_bss,
#endif
#if defined(CONFIG_XTENSA)
	clear_bss,
#endif
#if !defined(CONFIG_ARM) && !defined(CONFIG_SANDBOX) && \
		!CONFIG_IS_ENABLED(X86_64)
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
