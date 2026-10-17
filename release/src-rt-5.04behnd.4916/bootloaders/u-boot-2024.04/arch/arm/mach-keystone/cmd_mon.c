// SPDX-License-Identifier: GPL-2.0+
/*
 * K2HK: secure kernel command file
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#include <config.h>
#include <command.h>
#include <image.h>
#include <mach/mon.h>
asm(".arch_extension sec\n\t");

static int do_mon_install(struct cmd_tbl *cmdtp, int flag, int argc,
			  char *const argv[])
{
	u32 addr, dpsc_base = 0x1E80000, freq, load_addr, size;
	int     rcode = 0;
	struct legacy_img_hdr *header;
	u32 ecrypt_bm_addr = 0;

	if (argc < 2)
		return CMD_RET_USAGE;

	freq = CFG_SYS_HZ_CLOCK;

	addr = hextoul(argv[1], NULL);

	header = (struct legacy_img_hdr *)addr;

	if (image_get_magic(header) != IH_MAGIC) {
		printf("## Please update monitor image\n");
		return -EFAULT;
	}

	load_addr = image_get_load(header);
	size = image_get_data_size(header);
	memcpy((void *)load_addr, (void *)(addr + sizeof(struct legacy_img_hdr)),
	       size);

	if (argc >=  3)
		ecrypt_bm_addr = hextoul(argv[2], NULL);

	rcode = mon_install(load_addr, dpsc_base, freq, ecrypt_bm_addr);
	printf("## installed monitor @ 0x%x, freq [%d], status %d\n",
	       load_addr, freq, rcode);

	return 0;
}

U_BOOT_CMD(mon_install, 3, 0, do_mon_install,
	   "Install boot kernel at 'addr'",
	   ""
);

static void core_spin(void)
{
	while (1) {
		asm volatile (
			"dsb\n"
			"isb\n"
			"wfi\n"
		);
	}
}

int do_mon_power(struct cmd_tbl *cmdtp, int flag, int argc,
		 char *const argv[])
{
	int     rcode = 0, core_id, on;
	void (*fn)(void);

	fn = core_spin;

	if (argc < 3)
		return CMD_RET_USAGE;

	core_id = hextoul(argv[1], NULL);
	on = hextoul(argv[2], NULL);

	if (on)
		rcode = mon_power_on(core_id, fn);
	else
		rcode = mon_power_off(core_id);

	if (on) {
		if (!rcode)
			printf("core %d powered on successfully\n", core_id);
		else
			printf("core %d power on failure\n", core_id);
	} else {
		printf("core %d powered off successfully\n", core_id);
	}

	return 0;
}

U_BOOT_CMD(mon_power, 3, 0, do_mon_power,
	   "Power On/Off secondary core",
	   "mon_power <coreid> <oper>\n"
	   "- coreid (1-3) and oper (1 - ON, 0 - OFF)\n"
	   ""
);
