// SPDX-License-Identifier: GPL-2.0+
/*
 *
 * Common functions for OMAP4/5 based boards
 *
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Author :
 *	Aneesh V	<aneesh@ti.com>
 *	Steve Sakoman	<steve@sakoman.com>
 */
#include <common.h>
#include <debug_uart.h>
#include <fdtdec.h>
#include <spl.h>
#include <asm/arch/sys_proto.h>
#include <linux/sizes.h>
#include <asm/emif.h>
#include <asm/omap_common.h>
#include <linux/compiler.h>
#include <asm/system.h>
#include <dm/root.h>

DECLARE_GLOBAL_DATA_PTR;

void do_set_mux(u32 base, struct pad_conf_entry const *array, int size)
{
	int i;
	struct pad_conf_entry *pad = (struct pad_conf_entry *) array;

	for (i = 0; i < size; i++, pad++)
		writew(pad->val, base + pad->offset);
}

static void set_mux_conf_regs(void)
{
	switch (omap_hw_init_context()) {
	case OMAP_INIT_CONTEXT_SPL:
		set_muxconf_regs();
		break;
	case OMAP_INIT_CONTEXT_UBOOT_AFTER_SPL:
		break;
	case OMAP_INIT_CONTEXT_UBOOT_FROM_NOR:
	case OMAP_INIT_CONTEXT_UBOOT_AFTER_CH:
		set_muxconf_regs();
		break;
	}
}

u32 cortex_rev(void)
{

	unsigned int rev;

	/* Read Main ID Register (MIDR) */
	asm ("mrc p15, 0, %0, c0, c0, 0" : "=r" (rev));

	return rev;
}

static void omap_rev_string(void)
{
	u32 omap_rev = omap_revision();
	u32 soc_variant	= (omap_rev & 0xF0000000) >> 28;
	u32 omap_variant = (omap_rev & 0xFFFF0000) >> 16;
	u32 major_rev = (omap_rev & 0x00000F00) >> 8;
	u32 minor_rev = (omap_rev & 0x000000F0) >> 4;

	const char *sec_s, *package = NULL;

	switch (get_device_type()) {
	case TST_DEVICE:
		sec_s = "TST";
		break;
	case EMU_DEVICE:
		sec_s = "EMU";
		break;
	case HS_DEVICE:
		sec_s = "HS";
		break;
	case GP_DEVICE:
		sec_s = "GP";
		break;
	default:
		sec_s = "?";
	}

#if defined(CONFIG_DRA7XX)
	if (is_dra76x()) {
		switch (omap_rev & 0xF) {
		case DRA762_ABZ_PACKAGE:
			package = "ABZ";
			break;
		case DRA762_ACD_PACKAGE:
		default:
			package = "ACD";
			break;
		}
	}
#endif

	if (soc_variant)
		printf("OMAP");
	else
		printf("DRA");
	printf("%x-%s ES%x.%x", omap_variant, sec_s, major_rev, minor_rev);
	if (package)
		printf(" %s package\n", package);
	else
		puts("\n");
}

#ifdef CONFIG_SPL_BUILD
void spl_display_print(void)
{
	omap_rev_string();
}
#endif

void __weak srcomp_enable(void)
{
}

/**
 * do_board_detect() - Detect board description
 *
 * Function to detect board description. This is expected to be
 * overridden in the SoC family board file where desired.
 */
void __weak do_board_detect(void)
{
}

/**
 * vcores_init() - Assign omap_vcores based on board
 *
 * Function to pick the vcores based on board. This is expected to be
 * overridden in the SoC family board file where desired.
 */
void __weak vcores_init(void)
{
}

void s_init(void)
{
}

/**
 * init_package_revision() - Initialize package revision
 *
 * Function to get the pacakage information. This is expected to be
 * overridden in the SoC family file where desired.
 */
void __weak init_package_revision(void)
{
}

/**
 * early_system_init - Does Early system initialization.
 *
 * Does early system init of watchdog, muxing,  andclocks
 * Watchdog disable is done always. For the rest what gets done
 * depends on the boot mode in which this function is executed when
 *   1. SPL running from SRAM
 *   2. U-Boot running from FLASH
 *   3. U-Boot loaded to SDRAM by SPL
 *   4. U-Boot loaded to SDRAM by ROM code using the
 *	Configuration Header feature
 * Please have a look at the respective functions to see what gets
 * done in each of these cases
 * This function is called with SRAM stack.
 */
void early_system_init(void)
{
#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_MULTI_DTB_FIT)
	int ret;
	int rescan;
#endif
	init_omap_revision();
	hw_data_init();
	init_package_revision();

#ifdef CONFIG_SPL_BUILD
	if (warm_reset())
		force_emif_self_refresh();
#endif
	watchdog_init();
	set_mux_conf_regs();
#ifdef CONFIG_SPL_BUILD
	srcomp_enable();
	do_io_settings();
#endif
	setup_early_clocks();

#ifdef CONFIG_SPL_BUILD
	/*
	 * Save the boot parameters passed from romcode.
	 * We cannot delay the saving further than this,
	 * to prevent overwrites.
	 */
	save_omap_boot_params();
	spl_early_init();
#endif
	do_board_detect();

#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_MULTI_DTB_FIT)
	/*
	 * Board detection has been done.
	 * Let us see if another dtb wouldn't be a better match
	 * for our board
	 */
	ret = fdtdec_resetup(&rescan);
	if (!ret && rescan) {
		dm_uninit();
		dm_init_and_scan(true);
	}
#endif

	vcores_init();
#ifdef CONFIG_DEBUG_UART_OMAP
	debug_uart_init();
#endif
	prcm_init();
}

#ifdef CONFIG_SPL_BUILD
void board_init_f(ulong dummy)
{
	early_system_init();
#ifdef CONFIG_BOARD_EARLY_INIT_F
	board_early_init_f();
#endif
	/* For regular u-boot sdram_init() is called from dram_init() */
	sdram_init();
	gd->ram_size = omap_sdram_size();
}
#endif

int arch_cpu_init_dm(void)
{
	early_system_init();
	return 0;
}

/*
 * Routine: wait_for_command_complete
 * Description: Wait for posting to finish on watchdog
 */
void wait_for_command_complete(struct watchdog *wd_base)
{
	int pending = 1;
	do {
		pending = readl(&wd_base->wwps);
	} while (pending);
}

/*
 * Routine: watchdog_init
 * Description: Shut down watch dogs
 */
void watchdog_init(void)
{
	struct watchdog *wd2_base = (struct watchdog *)WDT2_BASE;

	writel(WD_UNLOCK1, &wd2_base->wspr);
	wait_for_command_complete(wd2_base);
	writel(WD_UNLOCK2, &wd2_base->wspr);
}


/*
 * This function finds the SDRAM size available in the system
 * based on DMM section configurations
 * This is needed because the size of memory installed may be
 * different on different versions of the board
 */
u32 omap_sdram_size(void)
{
	u32 section, i, valid;
	u64 sdram_start = 0, sdram_end = 0, addr,
	    size, total_size = 0, trap_size = 0, trap_start = 0;

	for (i = 0; i < 4; i++) {
		section	= __raw_readl(DMM_BASE + i*4);
		valid = (section & EMIF_SDRC_ADDRSPC_MASK) >>
			(EMIF_SDRC_ADDRSPC_SHIFT);
		addr = section & EMIF_SYS_ADDR_MASK;

		/* See if the address is valid */
		if ((addr >= TI_ARMV7_DRAM_ADDR_SPACE_START) &&
		    (addr < TI_ARMV7_DRAM_ADDR_SPACE_END)) {
			size = ((section & EMIF_SYS_SIZE_MASK) >>
				   EMIF_SYS_SIZE_SHIFT);
			size = 1 << size;
			size *= SZ_16M;

			if (valid != DMM_SDRC_ADDR_SPC_INVALID) {
				if (!sdram_start || (addr < sdram_start))
					sdram_start = addr;
				if (!sdram_end || ((addr + size) > sdram_end))
					sdram_end = addr + size;
			} else {
				trap_size = size;
				trap_start = addr;
			}
		}
	}

	if ((trap_start >= sdram_start) && (trap_start < sdram_end))
		total_size = (sdram_end - sdram_start) - (trap_size);
	else
		total_size = sdram_end - sdram_start;

	return total_size;
}


/*
 * Routine: dram_init
 * Description: sets uboots idea of sdram size
 */
int dram_init(void)
{
	sdram_init();
	gd->ram_size = omap_sdram_size();
	return 0;
}

/*
 * Print board information
 */
int checkboard(void)
{
	puts(sysinfo.board_string);
	return 0;
}

#if defined(CONFIG_DISPLAY_CPUINFO)
/*
 * Print CPU information
 */
int print_cpuinfo(void)
{
	puts("CPU  : ");
	omap_rev_string();

	return 0;
}
#endif
