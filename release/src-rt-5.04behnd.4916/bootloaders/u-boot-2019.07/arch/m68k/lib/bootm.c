// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <command.h>
#include <image.h>
#include <u-boot/zlib.h>
#include <bzlib.h>
#include <watchdog.h>
#include <environment.h>
#include <asm/byteorder.h>
#ifdef CONFIG_SHOW_BOOT_PROGRESS
# include <status_led.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

#define PHYSADDR(x) x

#define LINUX_MAX_ENVS		256
#define LINUX_MAX_ARGS		256

static ulong get_sp (void);
static void set_clocks_in_mhz (bd_t *kbd);

void arch_lmb_reserve(struct lmb *lmb)
{
	ulong sp;

	/*
	 * Booting a (Linux) kernel image
	 *
	 * Allocate space for command line and board info - the
	 * address should be as high as possible within the reach of
	 * the kernel (see CONFIG_SYS_BOOTMAPSZ settings), but in unused
	 * memory, which means far enough below the current stack
	 * pointer.
	 */
	sp = get_sp();
	debug ("## Current stack ends at 0x%08lx ", sp);

	/* adjust sp by 1K to be safe */
	sp -= 1024;
	lmb_reserve(lmb, sp, (CONFIG_SYS_SDRAM_BASE + gd->ram_size - sp));
}

int do_bootm_linux(int flag, int argc, char * const argv[], bootm_headers_t *images)
{
	int ret;
	bd_t  *kbd;
	void  (*kernel) (bd_t *, ulong, ulong, ulong, ulong);
	struct lmb *lmb = &images->lmb;

	/*
	 * allow the PREP bootm subcommand, it is required for bootm to work
	 */
	if (flag & BOOTM_STATE_OS_PREP)
		return 0;

	if ((flag != 0) && (flag != BOOTM_STATE_OS_GO))
		return 1;

	/* allocate space for kernel copy of board info */
	ret = boot_get_kbd (lmb, &kbd);
	if (ret) {
		puts("ERROR with allocation of kernel bd\n");
		goto error;
	}
	set_clocks_in_mhz(kbd);

	ret = image_setup_linux(images);
	if (ret)
		goto error;

	kernel = (void (*)(bd_t *, ulong, ulong, ulong, ulong))images->ep;

	debug("## Transferring control to Linux (at address %08lx) ...\n",
	      (ulong) kernel);

	bootstage_mark(BOOTSTAGE_ID_RUN_OS);

	/*
	 * Linux Kernel Parameters (passing board info data):
	 *   sp+00: Ignore, side effect of using jsr to jump to kernel
	 *   sp+04: ptr to board info data
	 *   sp+08: initrd_start or 0 if no initrd
	 *   sp+12: initrd_end - unused if initrd_start is 0
	 *   sp+16: Start of command line string
	 *   sp+20: End   of command line string
	 */
	(*kernel)(kbd, images->initrd_start, images->initrd_end,
		  images->cmdline_start, images->cmdline_end);
	/* does not return */
error:
	return 1;
}

static ulong get_sp (void)
{
	ulong sp;

	asm("movel %%a7, %%d0\n"
	    "movel %%d0, %0\n": "=d"(sp): :"%d0");

	return sp;
}

static void set_clocks_in_mhz (bd_t *kbd)
{
	char *s;

	s = env_get("clocks_in_mhz");
	if (s) {
		/* convert all clock information to MHz */
		kbd->bi_intfreq /= 1000000L;
		kbd->bi_busfreq /= 1000000L;
	}
}
