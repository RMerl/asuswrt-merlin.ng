// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <bootm.h>
#include <command.h>
#include <image.h>
#include <irq_func.h>
#include <lmb.h>
#include <log.h>
#include <linux/compiler.h>

int __weak bootz_setup(ulong image, ulong *start, ulong *end)
{
	/* Please define bootz_setup() for your platform */

	puts("Your platform's zImage format isn't supported yet!\n");
	return -1;
}

/*
 * zImage booting support
 */
static int bootz_start(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[], struct bootm_headers *images)
{
	ulong zi_start, zi_end;
	struct bootm_info bmi;
	int ret;

	bootm_init(&bmi);
	if (argc)
		bmi.addr_img = argv[0];
	if (argc > 1)
		bmi.conf_ramdisk = argv[1];
	if (argc > 2)
		bmi.conf_fdt = argv[2];
	/* do not set up argc and argv[] since nothing uses them */

	ret = bootm_run_states(&bmi, BOOTM_STATE_START);

	/* Setup Linux kernel zImage entry point */
	if (!argc) {
		images->ep = image_load_addr;
		debug("*  kernel: default image load address = 0x%08lx\n",
				image_load_addr);
	} else {
		images->ep = hextoul(argv[0], NULL);
		debug("*  kernel: cmdline image address = 0x%08lx\n",
			images->ep);
	}

	ret = bootz_setup(images->ep, &zi_start, &zi_end);
	if (ret != 0)
		return 1;

	lmb_reserve(&images->lmb, images->ep, zi_end - zi_start);

	/*
	 * Handle the BOOTM_STATE_FINDOTHER state ourselves as we do not
	 * have a header that provide this informaiton.
	 */
	if (bootm_find_images(image_load_addr, cmd_arg1(argc, argv),
			      cmd_arg2(argc, argv), images->ep,
			      zi_end - zi_start))
		return 1;

	return 0;
}

int do_bootz(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct bootm_info bmi;
	int ret;

	/* Consume 'bootz' */
	argc--; argv++;

	if (bootz_start(cmdtp, flag, argc, argv, &images))
		return 1;

	/*
	 * We are doing the BOOTM_STATE_LOADOS state ourselves, so must
	 * disable interrupts ourselves
	 */
	bootm_disable_interrupts();

	images.os.os = IH_OS_LINUX;

	bootm_init(&bmi);
	if (argc)
		bmi.addr_img = argv[0];
	if (argc > 1)
		bmi.conf_ramdisk = argv[1];
	if (argc > 2)
		bmi.conf_fdt = argv[2];
	bmi.cmd_name = "bootz";

	ret = bootz_run(&bmi);

	return ret;
}

U_BOOT_LONGHELP(bootz,
	"[addr [initrd[:size]] [fdt]]\n"
	"    - boot Linux zImage stored in memory\n"
	"\tThe argument 'initrd' is optional and specifies the address\n"
	"\tof the initrd in memory. The optional argument ':size' allows\n"
	"\tspecifying the size of RAW initrd.\n"
#if defined(CONFIG_OF_LIBFDT)
	"\tWhen booting a Linux kernel which requires a flat device-tree\n"
	"\ta third argument is required which is the address of the\n"
	"\tdevice-tree blob. To boot that kernel without an initrd image,\n"
	"\tuse a '-' for the second argument. If you do not pass a third\n"
	"\ta bd_info struct will be passed instead\n"
#endif
	);

U_BOOT_CMD(
	bootz,	CONFIG_SYS_MAXARGS,	1,	do_bootz,
	"boot Linux zImage image from memory", bootz_help_text
);
