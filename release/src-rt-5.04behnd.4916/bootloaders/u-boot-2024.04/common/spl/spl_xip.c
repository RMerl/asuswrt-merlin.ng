// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
 * Author(s): Vikas Manocha, <vikas.manocha@st.com> for STMicroelectronics.
 */

#include <common.h>
#include <image.h>
#include <log.h>
#include <spl.h>

static int spl_xip(struct spl_image_info *spl_image,
		   struct spl_boot_device *bootdev)
{
#if CONFIG_IS_ENABLED(OS_BOOT)
	if (!spl_start_uboot()) {
		spl_image->arg = (void *)CONFIG_SPL_PAYLOAD_ARGS_ADDR;
		spl_image->name = "Linux";
		spl_image->os = IH_OS_LINUX;
		spl_image->load_addr = CONFIG_SYS_LOAD_ADDR;
		spl_image->entry_point = CONFIG_SYS_LOAD_ADDR;
		debug("spl: payload xipImage, load addr: 0x%lx\n",
		      spl_image->load_addr);
		return 0;
	}
#endif
	return(spl_parse_image_header(spl_image, bootdev,
	       (const struct legacy_img_hdr *)CFG_SYS_UBOOT_BASE));
}
SPL_LOAD_IMAGE_METHOD("XIP", 0, BOOT_DEVICE_XIP, spl_xip);
