// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <spl.h>

#ifdef CONFIG_SPL_LOAD_FIT
static ulong spl_nor_load_read(struct spl_load_info *load, ulong sector,
			       ulong count, void *buf)
{
	debug("%s: sector %lx, count %lx, buf %p\n",
	      __func__, sector, count, buf);
	memcpy(buf, (void *)sector, count);

	return count;
}
#endif

static int spl_nor_load_image(struct spl_image_info *spl_image,
			      struct spl_boot_device *bootdev)
{
	int ret;
	__maybe_unused const struct image_header *header;
	__maybe_unused struct spl_load_info load;

	/*
	 * Loading of the payload to SDRAM is done with skipping of
	 * the mkimage header in this SPL NOR driver
	 */
	spl_image->flags |= SPL_COPY_PAYLOAD_ONLY;

#ifdef CONFIG_SPL_OS_BOOT
	if (!spl_start_uboot()) {
		/*
		 * Load Linux from its location in NOR flash to its defined
		 * location in SDRAM
		 */
		header = (const struct image_header *)CONFIG_SYS_OS_BASE;
#if CONFIG_IS_ENABLED(LOAD_FIT)
		if (image_get_magic(header) == FDT_MAGIC) {
			debug("Found FIT\n");
			load.bl_len = 1;
			load.read = spl_nor_load_read;

			ret = spl_load_simple_fit(spl_image, &load,
						  CONFIG_SYS_OS_BASE,
						  (void *)header);

			return ret;
		}
#endif
		if (image_get_os(header) == IH_OS_LINUX) {
			/* happy - was a Linux */

			ret = spl_parse_image_header(spl_image, header);
			if (ret)
				return ret;

			memcpy((void *)spl_image->load_addr,
			       (void *)(CONFIG_SYS_OS_BASE +
					sizeof(struct image_header)),
			       spl_image->size);
#ifdef CONFIG_SYS_FDT_BASE
			spl_image->arg = (void *)CONFIG_SYS_FDT_BASE;
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
#ifdef CONFIG_SPL_LOAD_FIT
	header = (const struct image_header *)CONFIG_SYS_UBOOT_BASE;
	if (image_get_magic(header) == FDT_MAGIC) {
		debug("Found FIT format U-Boot\n");
		load.bl_len = 1;
		load.read = spl_nor_load_read;
		ret = spl_load_simple_fit(spl_image, &load,
					  CONFIG_SYS_UBOOT_BASE,
					  (void *)header);

		return ret;
	}
#endif
	ret = spl_parse_image_header(spl_image,
			(const struct image_header *)CONFIG_SYS_UBOOT_BASE);
	if (ret)
		return ret;

	memcpy((void *)(unsigned long)spl_image->load_addr,
	       (void *)(CONFIG_SYS_UBOOT_BASE + sizeof(struct image_header)),
	       spl_image->size);

	return 0;
}
SPL_LOAD_IMAGE_METHOD("NOR", 0, BOOT_DEVICE_NOR, spl_nor_load_image);
