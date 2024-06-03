// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011
 * Corscience GmbH & Co. KG - Simon Schwarz <schwarz@corscience.de>
 */
#include <common.h>
#include <config.h>
#include <spl.h>
#include <asm/io.h>
#include <nand.h>
#include <linux/libfdt_env.h>
#include <fdt.h>

#if defined(CONFIG_SPL_NAND_RAW_ONLY)
static int spl_nand_load_image(struct spl_image_info *spl_image,
			struct spl_boot_device *bootdev)
{
	nand_init();

	printf("Loading U-Boot from 0x%08x (size 0x%08x) to 0x%08x\n",
	       CONFIG_SYS_NAND_U_BOOT_OFFS, CONFIG_SYS_NAND_U_BOOT_SIZE,
	       CONFIG_SYS_NAND_U_BOOT_DST);

	nand_spl_load_image(CONFIG_SYS_NAND_U_BOOT_OFFS,
			    CONFIG_SYS_NAND_U_BOOT_SIZE,
			    (void *)CONFIG_SYS_NAND_U_BOOT_DST);
	spl_set_header_raw_uboot(spl_image);
	nand_deselect();

	return 0;
}
#else

static ulong spl_nand_fit_read(struct spl_load_info *load, ulong offs,
			       ulong size, void *dst)
{
	int ret;

	ret = nand_spl_load_image(offs, size, dst);
	if (!ret)
		return size;
	else
		return 0;
}

static int spl_nand_load_element(struct spl_image_info *spl_image,
				 int offset, struct image_header *header)
{
	int err;

	err = nand_spl_load_image(offset, sizeof(*header), (void *)header);
	if (err)
		return err;

	if (CONFIG_IS_ENABLED(LOAD_FIT) &&
	    image_get_magic(header) == FDT_MAGIC) {
		struct spl_load_info load;

		debug("Found FIT\n");
		load.dev = NULL;
		load.priv = NULL;
		load.filename = NULL;
		load.bl_len = 1;
		load.read = spl_nand_fit_read;
		return spl_load_simple_fit(spl_image, &load, offset, header);
	} else {
		err = spl_parse_image_header(spl_image, header);
		if (err)
			return err;
		return nand_spl_load_image(offset, spl_image->size,
					   (void *)(ulong)spl_image->load_addr);
	}
}

static int spl_nand_load_image(struct spl_image_info *spl_image,
			       struct spl_boot_device *bootdev)
{
	int err;
	struct image_header *header;
	int *src __attribute__((unused));
	int *dst __attribute__((unused));

#ifdef CONFIG_SPL_NAND_SOFTECC
	debug("spl: nand - using sw ecc\n");
#else
	debug("spl: nand - using hw ecc\n");
#endif
	nand_init();

	header = spl_get_load_buffer(0, sizeof(*header));

#ifdef CONFIG_SPL_OS_BOOT
	if (!spl_start_uboot()) {
		/*
		 * load parameter image
		 * load to temp position since nand_spl_load_image reads
		 * a whole block which is typically larger than
		 * CONFIG_CMD_SPL_WRITE_SIZE therefore may overwrite
		 * following sections like BSS
		 */
		nand_spl_load_image(CONFIG_CMD_SPL_NAND_OFS,
			CONFIG_CMD_SPL_WRITE_SIZE,
			(void *)CONFIG_SYS_TEXT_BASE);
		/* copy to destintion */
		for (dst = (int *)CONFIG_SYS_SPL_ARGS_ADDR,
				src = (int *)CONFIG_SYS_TEXT_BASE;
				src < (int *)(CONFIG_SYS_TEXT_BASE +
				CONFIG_CMD_SPL_WRITE_SIZE);
				src++, dst++) {
			writel(readl(src), dst);
		}

		/* load linux */
		nand_spl_load_image(CONFIG_SYS_NAND_SPL_KERNEL_OFFS,
			sizeof(*header), (void *)header);
		err = spl_parse_image_header(spl_image, header);
		if (err)
			return err;
		if (header->ih_os == IH_OS_LINUX) {
			/* happy - was a linux */
			err = nand_spl_load_image(
				CONFIG_SYS_NAND_SPL_KERNEL_OFFS,
				spl_image->size,
				(void *)spl_image->load_addr);
			nand_deselect();
			return err;
		} else {
			puts("The Expected Linux image was not "
				"found. Please check your NAND "
				"configuration.\n");
			puts("Trying to start u-boot now...\n");
		}
	}
#endif
#ifdef CONFIG_NAND_ENV_DST
	spl_nand_load_element(spl_image, CONFIG_ENV_OFFSET, header);
#ifdef CONFIG_ENV_OFFSET_REDUND
	spl_nand_load_element(spl_image, CONFIG_ENV_OFFSET_REDUND, header);
#endif
#endif
	/* Load u-boot */
	err = spl_nand_load_element(spl_image, CONFIG_SYS_NAND_U_BOOT_OFFS,
				    header);
#ifdef CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND
#if CONFIG_SYS_NAND_U_BOOT_OFFS != CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND
	if (err)
		err = spl_nand_load_element(spl_image,
					    CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND,
					    header);
#endif
#endif
	nand_deselect();
	return err;
}
#endif
/* Use priorty 1 so that Ubi can override this */
SPL_LOAD_IMAGE_METHOD("NAND", 1, BOOT_DEVICE_NAND, spl_nand_load_image);
