// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (c) Copyright 2008 Nobuhiro Iwamatsu <iwamatsu.nobuhiro@renesas.com>
 * (c) Copyright 2008 Renesas Solutions Corp.
 */

#include <common.h>
#include <command.h>
#include <asm/byteorder.h>
#include <asm/zimage.h>

#ifdef CONFIG_SYS_DEBUG
static void hexdump(unsigned char *buf, int len)
{
	int i;

	for (i = 0; i < len; i++) {
		if ((i % 16) == 0)
			printf("%s%08x: ", i ? "\n" : "",
							(unsigned int)&buf[i]);
		printf("%02x ", buf[i]);
	}
	printf("\n");
}
#endif

#ifdef CONFIG_SH_SDRAM_OFFSET
#define GET_INITRD_START(initrd, linux) (initrd - linux + CONFIG_SH_SDRAM_OFFSET)
#else
#define GET_INITRD_START(initrd, linux) (initrd - linux)
#endif

static void set_sh_linux_param(unsigned long param_addr, unsigned long data)
{
	*(unsigned long *)(param_addr) = data;
}

static unsigned long sh_check_cmd_arg(char *cmdline, char *key, int base)
{
	unsigned long val = 0;
	char *p = strstr(cmdline, key);
	if (p) {
		p += strlen(key);
		val = simple_strtol(p, NULL, base);
	}
	return val;
}

int do_bootm_linux(int flag, int argc, char * const argv[], bootm_headers_t *images)
{
	/* Linux kernel load address */
	void (*kernel) (void) = (void (*)(void))images->ep;
	/* empty_zero_page */
	unsigned char *param
		= (unsigned char *)image_get_load(images->legacy_hdr_os);
	/* Linux kernel command line */
	char *cmdline = (char *)param + COMMAND_LINE;
	/* PAGE_SIZE */
	unsigned long size = images->ep - (unsigned long)param;
	char *bootargs = env_get("bootargs");

	/*
	 * allow the PREP bootm subcommand, it is required for bootm to work
	 */
	if (flag & BOOTM_STATE_OS_PREP)
		return 0;

	if ((flag != 0) && (flag != BOOTM_STATE_OS_GO))
		return 1;

	/* Clear zero page */
	memset(param, 0, size);

	/* Set commandline */
	strcpy(cmdline, bootargs);

	/* Initrd */
	if (images->rd_start || images->rd_end) {
		unsigned long ramdisk_flags = 0;
		int val = sh_check_cmd_arg(bootargs, CMD_ARG_RD_PROMPT, 10);
		if (val == 1)
				ramdisk_flags |= RD_PROMPT;
		else
				ramdisk_flags &= ~RD_PROMPT;

		val = sh_check_cmd_arg(bootargs, CMD_ARG_RD_DOLOAD, 10);
		if (val == 1)
				ramdisk_flags |= RD_DOLOAD;
		else
				ramdisk_flags &= ~RD_DOLOAD;

		set_sh_linux_param((unsigned long)param + MOUNT_ROOT_RDONLY, 0x0001);
		set_sh_linux_param((unsigned long)param + RAMDISK_FLAGS, ramdisk_flags);
		set_sh_linux_param((unsigned long)param + ORIG_ROOT_DEV, 0x0200);
		set_sh_linux_param((unsigned long)param + LOADER_TYPE, 0x0001);
		set_sh_linux_param((unsigned long)param + INITRD_START,
			GET_INITRD_START(images->rd_start, CONFIG_SYS_SDRAM_BASE));
		set_sh_linux_param((unsigned long)param + INITRD_SIZE,
			images->rd_end - images->rd_start);
	}

	/* Boot kernel */
	kernel();

	/* does not return */
	return 1;
}
