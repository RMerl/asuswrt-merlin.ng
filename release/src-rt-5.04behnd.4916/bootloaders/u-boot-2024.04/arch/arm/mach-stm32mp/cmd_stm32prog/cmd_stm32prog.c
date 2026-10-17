// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2020, STMicroelectronics - All Rights Reserved
 */

#include <common.h>
#include <bootm.h>
#include <command.h>
#include <dfu.h>
#include <image.h>
#include <asm/arch/stm32prog.h>
#include <linux/printk.h>
#include "stm32prog.h"

struct stm32prog_data *stm32prog_data;

static void enable_vidconsole(void)
{
	char *stdname;
	char buf[64];

	stdname = env_get("stdout");
	if (!stdname || !strstr(stdname, "vidconsole")) {
		if (!stdname)
			snprintf(buf, sizeof(buf), "serial,vidconsole");
		else
			snprintf(buf, sizeof(buf), "%s,vidconsole", stdname);
		env_set("stdout", buf);
	}

	stdname = env_get("stderr");
	if (!stdname || !strstr(stdname, "vidconsole")) {
		if (!stdname)
			snprintf(buf, sizeof(buf), "serial,vidconsole");
		else
			snprintf(buf, sizeof(buf), "%s,vidconsole", stdname);
		env_set("stderr", buf);
	}
}

static int do_stm32prog(struct cmd_tbl *cmdtp, int flag, int argc,
			char * const argv[])
{
	ulong	addr, size;
	int dev, ret;
	enum stm32prog_link_t link = LINK_UNDEFINED;
	bool reset = false;
	struct image_header_s header;
	struct stm32prog_data *data;

	if (argc < 3 ||  argc > 5)
		return CMD_RET_USAGE;

	if (IS_ENABLED(CONFIG_CMD_STM32PROG_USB) && !strcmp(argv[1], "usb"))
		link = LINK_USB;
	else if (IS_ENABLED(CONFIG_CMD_STM32PROG_SERIAL) && !strcmp(argv[1], "serial"))
		link = LINK_SERIAL;

	if (link == LINK_UNDEFINED) {
		log_err("not supported link=%s\n", argv[1]);
		return CMD_RET_USAGE;
	}

	dev = (int)dectoul(argv[2], NULL);

	addr = CONFIG_SYS_LOAD_ADDR;
	size = 0;
	if (argc > 3) {
		addr = hextoul(argv[3], NULL);
		if (!addr)
			return CMD_RET_FAILURE;
	}
	if (argc > 4)
		size = hextoul(argv[4], NULL);

	/* check STM32IMAGE presence */
	if (size == 0) {
		stm32prog_header_check(addr, &header);
		if (header.type == HEADER_STM32IMAGE) {
			size = header.image_length + header.length;
		}
	}

	if (IS_ENABLED(CONFIG_VIDEO))
		enable_vidconsole();

	data = (struct stm32prog_data *)malloc(sizeof(*data));

	if (!data) {
		log_err("Alloc failed.");
		return CMD_RET_FAILURE;
	}
	stm32prog_data = data;

	ret = stm32prog_init(data, addr, size);
	if (ret)
		log_debug("Invalid or missing layout file at 0x%lx.\n", addr);

	/* prepare DFU for device read/write */
	ret = stm32prog_dfu_init(data);
	if (ret)
		goto cleanup;

	switch (link) {
	case LINK_SERIAL:
		ret = stm32prog_serial_init(data, dev);
		if (ret)
			goto cleanup;
		reset = stm32prog_serial_loop(data);
		break;
	case LINK_USB:
		reset = stm32prog_usb_loop(data, dev);
		break;
	default:
		goto cleanup;
	}

	stm32prog_clean(data);
	free(stm32prog_data);
	stm32prog_data = NULL;

	puts("Download done\n");

	if (data->uimage) {
		char boot_addr_start[20];
		char dtb_addr[20];
		char initrd_addr[40];
		char *fdt_arg, *initrd_arg;
		const void *uimage = (void *)data->uimage;
		const void *dtb = (void *)data->dtb;
		const void *initrd = (void *)data->initrd;
		struct bootm_info bmi;

		fdt_arg = dtb_addr;
		if (!dtb)
			fdt_arg = env_get("fdtcontroladdr");
		else
			snprintf(dtb_addr, sizeof(dtb_addr) - 1, "0x%p", dtb);

		snprintf(boot_addr_start, sizeof(boot_addr_start) - 1,
			 "0x%p", uimage);

		initrd_arg = NULL;
		if (initrd) {
			snprintf(initrd_addr, sizeof(initrd_addr) - 1,
				 "0x%p:0x%zx", initrd, data->initrd_size);
			initrd_arg = initrd_addr;
		}

		printf("Booting kernel at %s %s %s...\n\n\n", boot_addr_start,
		       initrd_arg ?: "-", fdt_arg);

		bootm_init(&bmi);
		bmi.addr_img = boot_addr_start;
		bmi.conf_ramdisk = initrd_arg;
		bmi.conf_fdt = fdt_arg;

		/* Try bootm for legacy and FIT format image */
		if (genimg_get_format(uimage) != IMAGE_FORMAT_INVALID)
			bootm_run(&bmi);
		else if (IS_ENABLED(CONFIG_CMD_BOOTZ))
			bootz_run(&bmi);
	}
	if (data->script)
		cmd_source_script(data->script, NULL, NULL);

	if (reset) {
		puts("Reset...\n");
		run_command("reset", 0);
	}

	return CMD_RET_SUCCESS;

cleanup:
	stm32prog_clean(data);
	free(stm32prog_data);
	stm32prog_data = NULL;

	return CMD_RET_FAILURE;
}

U_BOOT_CMD(stm32prog, 5, 0, do_stm32prog,
	   "start communication with tools STM32Cubeprogrammer",
	   "<link> <dev> [<addr>] [<size>]\n"
	   "  <link> = serial|usb\n"
	   "  <dev>  = device instance\n"
	   "  <addr> = address of flashlayout\n"
	   "  <size> = size of flashlayout (optional for image with STM32 header)\n"
);


bool stm32prog_get_fsbl_nor(void)
{
	if (stm32prog_data)
		return stm32prog_data->fsbl_nor_detected;

	return false;
}
