// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/*
 * Boot support
 */
#include <common.h>
#include <bootm.h>
#include <command.h>
#include <environment.h>
#include <errno.h>
#include <image.h>
#include <malloc.h>
#include <nand.h>
#include <asm/byteorder.h>
#include <linux/ctype.h>
#include <linux/err.h>
#include <u-boot/zlib.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_CMD_IMI)
static int image_info(unsigned long addr);
#endif

#if defined(CONFIG_CMD_IMLS)
#include <flash.h>
#include <mtd/cfi_flash.h>
extern flash_info_t flash_info[]; /* info for FLASH chips */
#endif

#if defined(CONFIG_CMD_IMLS) || defined(CONFIG_CMD_IMLS_NAND)
static int do_imls(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
#endif

/* we overload the cmd field with our state machine info instead of a
 * function pointer */
static cmd_tbl_t cmd_bootm_sub[] = {
	U_BOOT_CMD_MKENT(start, 0, 1, (void *)BOOTM_STATE_START, "", ""),
	U_BOOT_CMD_MKENT(loados, 0, 1, (void *)BOOTM_STATE_LOADOS, "", ""),
#ifdef CONFIG_SYS_BOOT_RAMDISK_HIGH
	U_BOOT_CMD_MKENT(ramdisk, 0, 1, (void *)BOOTM_STATE_RAMDISK, "", ""),
#endif
#ifdef CONFIG_OF_LIBFDT
	U_BOOT_CMD_MKENT(fdt, 0, 1, (void *)BOOTM_STATE_FDT, "", ""),
#endif
	U_BOOT_CMD_MKENT(cmdline, 0, 1, (void *)BOOTM_STATE_OS_CMDLINE, "", ""),
	U_BOOT_CMD_MKENT(bdt, 0, 1, (void *)BOOTM_STATE_OS_BD_T, "", ""),
	U_BOOT_CMD_MKENT(prep, 0, 1, (void *)BOOTM_STATE_OS_PREP, "", ""),
	U_BOOT_CMD_MKENT(fake, 0, 1, (void *)BOOTM_STATE_OS_FAKE_GO, "", ""),
	U_BOOT_CMD_MKENT(go, 0, 1, (void *)BOOTM_STATE_OS_GO, "", ""),
};

static int do_bootm_subcommand(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	int ret = 0;
	long state;
	cmd_tbl_t *c;

	c = find_cmd_tbl(argv[0], &cmd_bootm_sub[0], ARRAY_SIZE(cmd_bootm_sub));
	argc--; argv++;

	if (c) {
		state = (long)c->cmd;
		if (state == BOOTM_STATE_START)
			state |= BOOTM_STATE_FINDOS | BOOTM_STATE_FINDOTHER;
	} else {
		/* Unrecognized command */
		return CMD_RET_USAGE;
	}

	if (((state & BOOTM_STATE_START) != BOOTM_STATE_START) &&
	    images.state >= state) {
		printf("Trying to execute a command out of order\n");
		return CMD_RET_USAGE;
	}

	ret = do_bootm_states(cmdtp, flag, argc, argv, state, &images, 0);

	return ret;
}

/*******************************************************************/
/* bootm - boot application image from image in memory */
/*******************************************************************/

int do_bootm(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
#ifdef CONFIG_NEEDS_MANUAL_RELOC
	static int relocated = 0;

	if (!relocated) {
		int i;

		/* relocate names of sub-command table */
		for (i = 0; i < ARRAY_SIZE(cmd_bootm_sub); i++)
			cmd_bootm_sub[i].name += gd->reloc_off;

		relocated = 1;
	}
#endif

	/* determine if we have a sub command */
	argc--; argv++;
	if (argc > 0) {
		char *endp;

		simple_strtoul(argv[0], &endp, 16);
		/* endp pointing to NULL means that argv[0] was just a
		 * valid number, pass it along to the normal bootm processing
		 *
		 * If endp is ':' or '#' assume a FIT identifier so pass
		 * along for normal processing.
		 *
		 * Right now we assume the first arg should never be '-'
		 */
		if ((*endp != 0) && (*endp != ':') && (*endp != '#'))
			return do_bootm_subcommand(cmdtp, flag, argc, argv);
	}

	return do_bootm_states(cmdtp, flag, argc, argv, BOOTM_STATE_START |
		BOOTM_STATE_FINDOS | BOOTM_STATE_FINDOTHER |
		BOOTM_STATE_LOADOS |
#ifdef CONFIG_SYS_BOOT_RAMDISK_HIGH
		BOOTM_STATE_RAMDISK |
#endif
#if defined(CONFIG_PPC) || defined(CONFIG_MIPS)
		BOOTM_STATE_OS_CMDLINE |
#endif
		BOOTM_STATE_OS_PREP | BOOTM_STATE_OS_FAKE_GO |
		BOOTM_STATE_OS_GO, &images, 1);
}

int bootm_maybe_autostart(cmd_tbl_t *cmdtp, const char *cmd)
{
	const char *ep = env_get("autostart");

	if (ep && !strcmp(ep, "yes")) {
		char *local_args[2];
		local_args[0] = (char *)cmd;
		local_args[1] = NULL;
		printf("Automatic boot of image at addr 0x%08lX ...\n", load_addr);
		return do_bootm(cmdtp, 0, 1, local_args);
	}

	return 0;
}

#ifdef CONFIG_SYS_LONGHELP
static char bootm_help_text[] =
	"[addr [arg ...]]\n    - boot application image stored in memory\n"
	"\tpassing arguments 'arg ...'; when booting a Linux kernel,\n"
	"\t'arg' can be the address of an initrd image\n"
#if defined(CONFIG_OF_LIBFDT)
	"\tWhen booting a Linux kernel which requires a flat device-tree\n"
	"\ta third argument is required which is the address of the\n"
	"\tdevice-tree blob. To boot that kernel without an initrd image,\n"
	"\tuse a '-' for the second argument. If you do not pass a third\n"
	"\ta bd_info struct will be passed instead\n"
#endif
#if defined(CONFIG_FIT)
	"\t\nFor the new multi component uImage format (FIT) addresses\n"
	"\tmust be extended to include component or configuration unit name:\n"
	"\taddr:<subimg_uname> - direct component image specification\n"
	"\taddr#<conf_uname>   - configuration specification\n"
	"\tUse iminfo command to get the list of existing component\n"
	"\timages and configurations.\n"
#endif
	"\nSub-commands to do part of the bootm sequence.  The sub-commands "
	"must be\n"
	"issued in the order below (it's ok to not issue all sub-commands):\n"
	"\tstart [addr [arg ...]]\n"
	"\tloados  - load OS image\n"
#if defined(CONFIG_SYS_BOOT_RAMDISK_HIGH)
	"\tramdisk - relocate initrd, set env initrd_start/initrd_end\n"
#endif
#if defined(CONFIG_OF_LIBFDT)
	"\tfdt     - relocate flat device tree\n"
#endif
	"\tcmdline - OS specific command line processing/setup\n"
	"\tbdt     - OS specific bd_t processing\n"
	"\tprep    - OS specific prep before relocation or go\n"
#if defined(CONFIG_TRACE)
	"\tfake    - OS specific fake start without go\n"
#endif
	"\tgo      - start OS";
#endif

U_BOOT_CMD(
	bootm,	CONFIG_SYS_MAXARGS,	1,	do_bootm,
	"boot application image from memory", bootm_help_text
);

/*******************************************************************/
/* bootd - boot default image */
/*******************************************************************/
#if defined(CONFIG_CMD_BOOTD)
int do_bootd(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	return run_command(env_get("bootcmd"), flag);
}

U_BOOT_CMD(
	boot,	1,	1,	do_bootd,
	"boot default, i.e., run 'bootcmd'",
	""
);

/* keep old command name "bootd" for backward compatibility */
U_BOOT_CMD(
	bootd, 1,	1,	do_bootd,
	"boot default, i.e., run 'bootcmd'",
	""
);

#endif


/*******************************************************************/
/* iminfo - print header info for a requested image */
/*******************************************************************/
#if defined(CONFIG_CMD_IMI)
static int do_iminfo(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int	arg;
	ulong	addr;
	int	rcode = 0;

	if (argc < 2) {
		return image_info(load_addr);
	}

	for (arg = 1; arg < argc; ++arg) {
		addr = simple_strtoul(argv[arg], NULL, 16);
		if (image_info(addr) != 0)
			rcode = 1;
	}
	return rcode;
}

static int image_info(ulong addr)
{
	void *hdr = (void *)addr;

	printf("\n## Checking Image at %08lx ...\n", addr);

	switch (genimg_get_format(hdr)) {
#if defined(CONFIG_IMAGE_FORMAT_LEGACY)
	case IMAGE_FORMAT_LEGACY:
		puts("   Legacy image found\n");
		if (!image_check_magic(hdr)) {
			puts("   Bad Magic Number\n");
			return 1;
		}

		if (!image_check_hcrc(hdr)) {
			puts("   Bad Header Checksum\n");
			return 1;
		}

		image_print_contents(hdr);

		puts("   Verifying Checksum ... ");
		if (!image_check_dcrc(hdr)) {
			puts("   Bad Data CRC\n");
			return 1;
		}
		puts("OK\n");
		return 0;
#endif
#if defined(CONFIG_ANDROID_BOOT_IMAGE)
	case IMAGE_FORMAT_ANDROID:
		puts("   Android image found\n");
		android_print_contents(hdr);
		return 0;
#endif
#if defined(CONFIG_FIT)
	case IMAGE_FORMAT_FIT:
		puts("   FIT image found\n");

		if (!fit_check_format(hdr)) {
			puts("Bad FIT image format!\n");
			return 1;
		}

		fit_print_contents(hdr);

		if (!fit_all_image_verify(hdr)) {
			puts("Bad hash in FIT image!\n");
			return 1;
		}

		return 0;
#endif
	default:
		puts("Unknown image format!\n");
		break;
	}

	return 1;
}

U_BOOT_CMD(
	iminfo,	CONFIG_SYS_MAXARGS,	1,	do_iminfo,
	"print header information for application image",
	"addr [addr ...]\n"
	"    - print header information for application image starting at\n"
	"      address 'addr' in memory; this includes verification of the\n"
	"      image contents (magic number, header and payload checksums)"
);
#endif


/*******************************************************************/
/* imls - list all images found in flash */
/*******************************************************************/
#if defined(CONFIG_CMD_IMLS)
static int do_imls_nor(void)
{
	flash_info_t *info;
	int i, j;
	void *hdr;

	for (i = 0, info = &flash_info[0];
		i < CONFIG_SYS_MAX_FLASH_BANKS; ++i, ++info) {

		if (info->flash_id == FLASH_UNKNOWN)
			goto next_bank;
		for (j = 0; j < info->sector_count; ++j) {

			hdr = (void *)info->start[j];
			if (!hdr)
				goto next_sector;

			switch (genimg_get_format(hdr)) {
#if defined(CONFIG_IMAGE_FORMAT_LEGACY)
			case IMAGE_FORMAT_LEGACY:
				if (!image_check_hcrc(hdr))
					goto next_sector;

				printf("Legacy Image at %08lX:\n", (ulong)hdr);
				image_print_contents(hdr);

				puts("   Verifying Checksum ... ");
				if (!image_check_dcrc(hdr)) {
					puts("Bad Data CRC\n");
				} else {
					puts("OK\n");
				}
				break;
#endif
#if defined(CONFIG_FIT)
			case IMAGE_FORMAT_FIT:
				if (!fit_check_format(hdr))
					goto next_sector;

				printf("FIT Image at %08lX:\n", (ulong)hdr);
				fit_print_contents(hdr);
				break;
#endif
			default:
				goto next_sector;
			}

next_sector:		;
		}
next_bank:	;
	}
	return 0;
}
#endif

#if defined(CONFIG_CMD_IMLS_NAND)
static int nand_imls_legacyimage(struct mtd_info *mtd, int nand_dev,
				 loff_t off, size_t len)
{
	void *imgdata;
	int ret;

	imgdata = malloc(len);
	if (!imgdata) {
		printf("May be a Legacy Image at NAND device %d offset %08llX:\n",
				nand_dev, off);
		printf("   Low memory(cannot allocate memory for image)\n");
		return -ENOMEM;
	}

	ret = nand_read_skip_bad(mtd, off, &len, NULL, mtd->size, imgdata);
	if (ret < 0 && ret != -EUCLEAN) {
		free(imgdata);
		return ret;
	}

	if (!image_check_hcrc(imgdata)) {
		free(imgdata);
		return 0;
	}

	printf("Legacy Image at NAND device %d offset %08llX:\n",
			nand_dev, off);
	image_print_contents(imgdata);

	puts("   Verifying Checksum ... ");
	if (!image_check_dcrc(imgdata))
		puts("Bad Data CRC\n");
	else
		puts("OK\n");

	free(imgdata);

	return 0;
}

static int nand_imls_fitimage(struct mtd_info *mtd, int nand_dev, loff_t off,
			      size_t len)
{
	void *imgdata;
	int ret;

	imgdata = malloc(len);
	if (!imgdata) {
		printf("May be a FIT Image at NAND device %d offset %08llX:\n",
				nand_dev, off);
		printf("   Low memory(cannot allocate memory for image)\n");
		return -ENOMEM;
	}

	ret = nand_read_skip_bad(mtd, off, &len, NULL, mtd->size, imgdata);
	if (ret < 0 && ret != -EUCLEAN) {
		free(imgdata);
		return ret;
	}

	if (!fit_check_format(imgdata)) {
		free(imgdata);
		return 0;
	}

	printf("FIT Image at NAND device %d offset %08llX:\n", nand_dev, off);

	fit_print_contents(imgdata);
	free(imgdata);

	return 0;
}

static int do_imls_nand(void)
{
	struct mtd_info *mtd;
	int nand_dev = nand_curr_device;
	size_t len;
	loff_t off;
	u32 buffer[16];

	if (nand_dev < 0 || nand_dev >= CONFIG_SYS_MAX_NAND_DEVICE) {
		puts("\nNo NAND devices available\n");
		return -ENODEV;
	}

	printf("\n");

	for (nand_dev = 0; nand_dev < CONFIG_SYS_MAX_NAND_DEVICE; nand_dev++) {
		mtd = get_nand_dev_by_index(nand_dev);
		if (!mtd->name || !mtd->size)
			continue;

		for (off = 0; off < mtd->size; off += mtd->erasesize) {
			const image_header_t *header;
			int ret;

			if (nand_block_isbad(mtd, off))
				continue;

			len = sizeof(buffer);

			ret = nand_read(mtd, off, &len, (u8 *)buffer);
			if (ret < 0 && ret != -EUCLEAN) {
				printf("NAND read error %d at offset %08llX\n",
						ret, off);
				continue;
			}

			switch (genimg_get_format(buffer)) {
#if defined(CONFIG_IMAGE_FORMAT_LEGACY)
			case IMAGE_FORMAT_LEGACY:
				header = (const image_header_t *)buffer;

				len = image_get_image_size(header);
				nand_imls_legacyimage(mtd, nand_dev, off, len);
				break;
#endif
#if defined(CONFIG_FIT)
			case IMAGE_FORMAT_FIT:
				len = fit_get_size(buffer);
				nand_imls_fitimage(mtd, nand_dev, off, len);
				break;
#endif
			}
		}
	}

	return 0;
}
#endif

#if defined(CONFIG_CMD_IMLS) || defined(CONFIG_CMD_IMLS_NAND)
static int do_imls(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret_nor = 0, ret_nand = 0;

#if defined(CONFIG_CMD_IMLS)
	ret_nor = do_imls_nor();
#endif

#if defined(CONFIG_CMD_IMLS_NAND)
	ret_nand = do_imls_nand();
#endif

	if (ret_nor)
		return ret_nor;

	if (ret_nand)
		return ret_nand;

	return (0);
}

U_BOOT_CMD(
	imls,	1,		1,	do_imls,
	"list all images found in flash",
	"\n"
	"    - Prints information about all images found at sector/block\n"
	"      boundaries in nor/nand flash."
);
#endif
