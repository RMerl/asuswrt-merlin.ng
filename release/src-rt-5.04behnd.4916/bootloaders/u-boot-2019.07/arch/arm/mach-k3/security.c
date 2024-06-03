// SPDX-License-Identifier: GPL-2.0
/*
 * K3: Security functions
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 *	Andrew F. Davis <afd@ti.com>
 */

#include <common.h>
#include <dm.h>
#include <linux/soc/ti/ti_sci_protocol.h>
#include <mach/spl.h>
#include <spl.h>

void board_fit_image_post_process(void **p_image, size_t *p_size)
{
	struct udevice *dev;
	struct ti_sci_handle *ti_sci;
	struct ti_sci_proc_ops *proc_ops;
	u64 image_addr;
	u32 image_size;
	int ret;

	/* Get handle to Device Management and Security Controller (SYSFW) */
	ret = uclass_get_device_by_name(UCLASS_FIRMWARE, "dmsc", &dev);
	if (ret) {
		printf("Failed to get handle to SYSFW (%d)\n", ret);
		hang();
	}
	ti_sci = (struct ti_sci_handle *)(ti_sci_get_handle_from_sysfw(dev));
	proc_ops = &ti_sci->ops.proc_ops;

	image_addr = (uintptr_t)*p_image;

	debug("Authenticating image at address 0x%016llx\n", image_addr);

	/* Authenticate image */
	ret = proc_ops->proc_auth_boot_image(ti_sci, &image_addr, &image_size);
	if (ret) {
		printf("Authentication failed!\n");
		hang();
	}

	/*
	 * The image_size returned may be 0 when the authentication process has
	 * moved the image. When this happens no further processing on the
	 * image is needed or often even possible as it may have also been
	 * placed behind a firewall when moved.
	 */
	*p_size = image_size;

	/*
	 * Output notification of successful authentication to re-assure the
	 * user that the secure code is being processed as expected. However
	 * suppress any such log output in case of building for SPL and booting
	 * via YMODEM. This is done to avoid disturbing the YMODEM serial
	 * protocol transactions.
	 */
	if (!(IS_ENABLED(CONFIG_SPL_BUILD) &&
	      IS_ENABLED(CONFIG_SPL_YMODEM_SUPPORT) &&
	      spl_boot_device() == BOOT_DEVICE_UART))
		printf("Authentication passed\n");
}
