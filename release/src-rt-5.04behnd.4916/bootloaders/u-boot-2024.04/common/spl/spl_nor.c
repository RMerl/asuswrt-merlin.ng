// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <image.h>
#include <imx_container.h>
#include <log.h>
#include <spl.h>
#include <spl_load.h>

static ulong spl_nor_load_read(struct spl_load_info *load, ulong sector,
			       ulong count, void *buf)
{
	debug("%s: sector %lx, count %lx, buf %p\n",
	      __func__, sector, count, buf);
	memcpy(buf, map_sysmem(sector, count), count);

	return count;
}

unsigned long __weak spl_nor_get_uboot_base(void)
{
	return CFG_SYS_UBOOT_BASE;
}

static int spl_nor_load_image(struct spl_image_info *spl_image,
			      struct spl_boot_device *bootdev)
{
	struct spl_load_info load;

	/*
	 * Loading of the payload to SDRAM is done with skipping of
	 * the mkimage header in this SPL NOR driver
	 */
	spl_image->flags |= SPL_COPY_PAYLOAD_ONLY;

#if CONFIG_IS_ENABLED(OS_BOOT)
	if (!spl_start_uboot()) {
		/*
		 * Load Linux from its location in NOR flash to its defined
		 * location in SDRAM
		 */
		const struct legacy_img_hdr *header =
			(const struct legacy_img_hdr *)CONFIG_SYS_OS_BASE;
		if (image_get_magic(header) == FDT_MAGIC) {
			int ret;

			debug("Found FIT\n");
			spl_set_bl_len(&load, 1);
			load.read = spl_nor_load_read;

			ret = spl_load_simple_fit(spl_image, &load,
						  CONFIG_SYS_OS_BASE,
						  (void *)header);

#if defined CONFIG_SPL_PAYLOAD_ARGS_ADDR && defined CONFIG_CMD_SPL_NOR_OFS
			memcpy((void *)CONFIG_SPL_PAYLOAD_ARGS_ADDR,
			       (void *)CONFIG_CMD_SPL_NOR_OFS,
			       CONFIG_CMD_SPL_WRITE_SIZE);
#endif
			return ret;
		}
#endif
		if (image_get_os(header) == IH_OS_LINUX) {
			/* happy - was a Linux */
			int ret;

			ret = spl_parse_image_header(spl_image, bootdev, header);
			if (ret)
				return ret;

			memcpy((void *)spl_image->load_addr,
			       (void *)(CONFIG_SYS_OS_BASE +
					sizeof(struct legacy_img_hdr)),
			       spl_image->size);
#ifdef CONFIG_SPL_PAYLOAD_ARGS_ADDR
			spl_image->arg = (void *)CONFIG_SPL_PAYLOAD_ARGS_ADDR;
#endif

			return 0;
		} else {
			puts("The Expected Linux image was not found.\n"
			     "Please check your NOR configuration.\n"
			     "Trying to start u-boot now...\n");
		}
	}
#endif

	/*
	 * Load real U-Boot from its location in NOR flash to its
	 * defined location in SDRAM
	 */
	spl_set_bl_len(&load, 1);
	load.read = spl_nor_load_read;
	return spl_load(spl_image, bootdev, &load, 0, spl_nor_get_uboot_base());
}
SPL_LOAD_IMAGE_METHOD("NOR", 0, BOOT_DEVICE_NOR, spl_nor_load_image);
