// SPDX-License-Identifier: GPL-2.0
/*
 * K3: Security functions
 *
 * Copyright (C) 2018-2022 Texas Instruments Incorporated - https://www.ti.com/
 *	Andrew F. Davis <afd@ti.com>
 */

#include <asm/io.h>
#include <cpu_func.h>
#include <dm.h>
#include <hang.h>
#include <image.h>
#include <log.h>
#include <asm/cache.h>
#include <linux/soc/ti/ti_sci_protocol.h>
#include <mach/spl.h>
#include <spl.h>
#include <linux/dma-mapping.h>

#include "common.h"

static bool ti_secure_cert_detected(void *p_image)
{
	/* Primitive certificate detection, check for DER starting with
	 * two 4-Octet SEQUENCE tags
	 */
	return (((u8 *)p_image)[0] == 0x30 && ((u8 *)p_image)[1] == 0x82 &&
		((u8 *)p_image)[4] == 0x30 && ((u8 *)p_image)[5] == 0x82);
}

/* Primitive certificate length, assumes one 2-Octet sized SEQUENCE */
static size_t ti_secure_cert_length(void *p_image)
{
	size_t seq_length = be16_to_cpu(readw_relaxed(p_image + 2));
	/* Add 4 for the SEQUENCE tag length */
	return seq_length + 4;
}

void ti_secure_image_check_binary(void **p_image, size_t *p_size)
{
	u32 image_size;
	size_t cert_length;
	image_size = *p_size;

	if (!image_size) {
		debug("%s: Image size is %d\n", __func__, image_size);
		return;
	}

	if (get_device_type() == K3_DEVICE_TYPE_GP) {
		if (ti_secure_cert_detected(*p_image)) {
			printf("Warning: Detected image signing certificate on GP device. "
			       "Skipping certificate to prevent boot failure. "
			       "This will fail if the image was also encrypted\n");

			cert_length = ti_secure_cert_length(*p_image);
			if (cert_length > *p_size) {
				printf("Invalid signing certificate size\n");
				return;
			}

			*p_image += cert_length;
			*p_size -= cert_length;
		}

		return;
	}
}

void ti_secure_image_post_process(void **p_image, size_t *p_size)
{
	struct ti_sci_handle *ti_sci = get_ti_sci_handle();
	struct ti_sci_proc_ops *proc_ops = &ti_sci->ops.proc_ops;
	u64 image_addr;
	u32 image_size;
	int ret;

	image_size = *p_size;
	if (!image_size) {
		debug("%s: Image size is %d\n", __func__, image_size);
		return;
	}

	if (get_device_type() == K3_DEVICE_TYPE_GP)
		return;

	if (get_device_type() != K3_DEVICE_TYPE_HS_SE &&
	    !ti_secure_cert_detected(*p_image)) {
		printf("Warning: Did not detect image signing certificate. "
		       "Skipping authentication to prevent boot failure. "
		       "This will fail on Security Enforcing(HS-SE) devices\n");
		return;
	}

	/* Clean out image so it can be seen by system firmware */
	image_addr = dma_map_single(*p_image, *p_size, DMA_BIDIRECTIONAL);

	debug("Authenticating image at address 0x%016llx\n", image_addr);
	debug("Authenticating image of size %d bytes\n", image_size);

	/* Authenticate image */
	ret = proc_ops->proc_auth_boot_image(ti_sci, &image_addr, &image_size);
	if (ret) {
		printf("Authentication failed!\n");
		hang();
	}

	/* Invalidate any stale lines over data written by system firmware */
	if (image_size)
		dma_unmap_single(image_addr, image_size, DMA_BIDIRECTIONAL);

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
