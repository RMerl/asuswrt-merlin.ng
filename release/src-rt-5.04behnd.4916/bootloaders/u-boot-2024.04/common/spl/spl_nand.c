// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011
 * Corscience GmbH & Co. KG - Simon Schwarz <schwarz@corscience.de>
 */
#include <common.h>
#include <config.h>
#include <fdt_support.h>
#include <image.h>
#include <imx_container.h>
#include <log.h>
#include <spl.h>
#include <spl_load.h>
#include <asm/io.h>
#include <mapmem.h>
#include <nand.h>
#include <linux/libfdt_env.h>
#include <fdt.h>

uint32_t __weak spl_nand_get_uboot_raw_page(void)
{
	return CONFIG_SYS_NAND_U_BOOT_OFFS;
}

#if defined(CONFIG_SPL_NAND_RAW_ONLY)
static int spl_nand_load_image(struct spl_image_info *spl_image,
			struct spl_boot_device *bootdev)
{
	nand_init();

	printf("Loading U-Boot from 0x%08x (size 0x%08x) to 0x%08x\n",
	       CONFIG_SYS_NAND_U_BOOT_OFFS, CFG_SYS_NAND_U_BOOT_SIZE,
	       CFG_SYS_NAND_U_BOOT_DST);

	nand_spl_load_image(spl_nand_get_uboot_raw_page(),
			    CFG_SYS_NAND_U_BOOT_SIZE,
			    map_sysmem(CFG_SYS_NAND_U_BOOT_DST,
				       CFG_SYS_NAND_U_BOOT_SIZE));
	spl_set_header_raw_uboot(spl_image);
	nand_deselect();

	return 0;
}
#else

__weak u32 nand_spl_adjust_offset(u32 sector, u32 offs)
{
	return offs;
}

static ulong spl_nand_read(struct spl_load_info *load, ulong offs, ulong size,
			   void *dst)
{
	int err;
	ulong sector;

	debug("%s: offs %lx, size %lx, dst %p\n",
	      __func__, offs, size, dst);

	sector = *(int *)load->priv;
	offs = sector + nand_spl_adjust_offset(sector, offs - sector);
	err = nand_spl_load_image(offs, size, dst);
	spl_set_bl_len(load, nand_page_size());
	if (err)
		return 0;

	return size;
}

static int spl_nand_load_element(struct spl_image_info *spl_image,
				 struct spl_boot_device *bootdev, int offset)
{
	struct spl_load_info load;

	load.priv = &offset;
	spl_set_bl_len(&load, 1);
	load.read = spl_nand_read;
	return spl_load(spl_image, bootdev, &load, 0, offset);
}

static int spl_nand_load_image(struct spl_image_info *spl_image,
			       struct spl_boot_device *bootdev)
{
	int err;

#ifdef CONFIG_SPL_NAND_SOFTECC
	debug("spl: nand - using sw ecc\n");
#else
	debug("spl: nand - using hw ecc\n");
#endif
	nand_init();

#if CONFIG_IS_ENABLED(OS_BOOT)
	if (!spl_start_uboot()) {
		int *src, *dst;
		struct legacy_img_hdr *header =
			spl_get_load_buffer(0, sizeof(*header));

		/*
		 * load parameter image
		 * load to temp position since nand_spl_load_image reads
		 * a whole block which is typically larger than
		 * CONFIG_CMD_SPL_WRITE_SIZE therefore may overwrite
		 * following sections like BSS
		 */
		nand_spl_load_image(CONFIG_CMD_SPL_NAND_OFS,
			CONFIG_CMD_SPL_WRITE_SIZE,
			(void *)CONFIG_TEXT_BASE);
		/* copy to destintion */
		for (dst = (int *)CONFIG_SPL_PAYLOAD_ARGS_ADDR,
		     src = (int *)CONFIG_TEXT_BASE;
			src < (int *)(CONFIG_TEXT_BASE +
				      CONFIG_CMD_SPL_WRITE_SIZE);
		     src++, dst++) {
			writel(readl(src), dst);
		}

		/* load linux */
		nand_spl_load_image(CONFIG_SYS_NAND_SPL_KERNEL_OFFS,
			sizeof(*header), (void *)header);
		err = spl_parse_image_header(spl_image, bootdev, header);
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
	spl_nand_load_element(spl_image, bootdev, CONFIG_ENV_OFFSET);
#ifdef CONFIG_ENV_OFFSET_REDUND
	spl_nand_load_element(spl_image, bootdev, CONFIG_ENV_OFFSET_REDUND);
#endif
#endif
	/* Load u-boot */
	err = spl_nand_load_element(spl_image, bootdev, spl_nand_get_uboot_raw_page());
#ifdef CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND
#if CONFIG_SYS_NAND_U_BOOT_OFFS != CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND
	if (err)
		err = spl_nand_load_element(spl_image, bootdev,
					    CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND);
#endif
#endif
	nand_deselect();
	return err;
}
#endif
/* Use priorty 1 so that Ubi can override this */
SPL_LOAD_IMAGE_METHOD("NAND", 1, BOOT_DEVICE_NAND, spl_nand_load_image);
