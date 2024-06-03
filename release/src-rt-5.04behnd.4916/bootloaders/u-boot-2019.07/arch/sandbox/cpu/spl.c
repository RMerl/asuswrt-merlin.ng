// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2016 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <os.h>
#include <spl.h>
#include <asm/spl.h>
#include <asm/state.h>

DECLARE_GLOBAL_DATA_PTR;

void board_init_f(ulong flag)
{
	struct sandbox_state *state = state_get_current();

	gd->arch.ram_buf = state->ram_buf;
	gd->ram_size = state->ram_size;
}

u32 spl_boot_device(void)
{
	return BOOT_DEVICE_BOARD;
}

static int spl_board_load_image(struct spl_image_info *spl_image,
				struct spl_boot_device *bootdev)
{
	char fname[256];
	int ret;

	ret = os_find_u_boot(fname, sizeof(fname));
	if (ret) {
		printf("(%s not found, error %d)\n", fname, ret);
		return ret;
	}

	/* Set up spl_image to boot from jump_to_image_no_args() */
	spl_image->arg = strdup(fname);
	if (!spl_image->arg)
		return log_msg_ret("Setup exec filename", -ENOMEM);

	return 0;
}
SPL_LOAD_IMAGE_METHOD("sandbox", 0, BOOT_DEVICE_BOARD, spl_board_load_image);

void spl_board_init(void)
{
	struct sandbox_state *state = state_get_current();
	struct udevice *dev;

	preloader_console_init();
	if (state->show_of_platdata) {
		/*
		 * Scan all the devices so that we can output their platform
		 * data. See sandbox_spl_probe().
		 */
		printf("Scanning misc devices\n");
		for (uclass_first_device(UCLASS_MISC, &dev);
		     dev;
		     uclass_next_device(&dev))
			;
	}
}

void __noreturn jump_to_image_no_args(struct spl_image_info *spl_image)
{
	const char *fname = spl_image->arg;

	if (fname) {
		os_fd_restore();
		os_spl_to_uboot(fname);
	} else {
		printf("No filename provided for U-Boot\n");
	}
	hang();
}
