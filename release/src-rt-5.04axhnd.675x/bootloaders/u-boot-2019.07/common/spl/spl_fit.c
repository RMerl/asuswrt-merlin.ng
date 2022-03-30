// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <errno.h>
#include <fpga.h>
#include <image.h>
#include <linux/libfdt.h>
#include <spl.h>

#ifndef CONFIG_SYS_BOOTM_LEN
#define CONFIG_SYS_BOOTM_LEN	(64 << 20)
#endif

__weak void board_spl_fit_post_load(ulong load_addr, size_t length)
{
}

__weak ulong board_spl_fit_size_align(ulong size)
{
	return size;
}

__weak void board_spl_fit_pre_load(struct spl_image_info * image_info, 
	struct spl_load_info *load_info, void * fit, ulong start_sector, ulong sector_count)
{
}

/**
 * spl_fit_get_image_name(): By using the matching configuration subnode,
 * retrieve the name of an image, specified by a property name and an index
 * into that.
 * @fit:	Pointer to the FDT blob.
 * @images:	Offset of the /images subnode.
 * @type:	Name of the property within the configuration subnode.
 * @index:	Index into the list of strings in this property.
 * @outname:	Name of the image
 *
 * Return:	0 on success, or a negative error number
 */
static int spl_fit_get_image_name(const void *fit, int images,
				  const char *type, int index,
				  char **outname)
{
	const char *name, *str;
	__maybe_unused int node;
	int conf_node;
	int len, i;

	conf_node = fit_find_config_node(fit);
	if (conf_node < 0) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		printf("No matching DT out of these options:\n");
		for (node = fdt_first_subnode(fit, conf_node);
		     node >= 0;
		     node = fdt_next_subnode(fit, node)) {
			name = fdt_getprop(fit, node, "description", &len);
			printf("   %s\n", name);
		}
#endif
		return conf_node;
	}

	name = fdt_getprop(fit, conf_node, type, &len);
	if (!name) {
		debug("cannot find property '%s': %d\n", type, len);
		return -EINVAL;
	}

	str = name;
	for (i = 0; i < index; i++) {
		str = strchr(str, '\0') + 1;
		if (!str || (str - name >= len)) {
			debug("no string for index %d\n", index);
			return -E2BIG;
		}
	}

	*outname = (char *)str;
	return 0;
}

/**
 * spl_fit_get_image_node(): By using the matching configuration subnode,
 * retrieve the name of an image, specified by a property name and an index
 * into that.
 * @fit:	Pointer to the FDT blob.
 * @images:	Offset of the /images subnode.
 * @type:	Name of the property within the configuration subnode.
 * @index:	Index into the list of strings in this property.
 *
 * Return:	the node offset of the respective image node or a negative
 *		error number.
 */
static int spl_fit_get_image_node(const void *fit, int images,
				  const char *type, int index)
{
	char *str;
	int err;
	int node;

	err = spl_fit_get_image_name(fit, images, type, index, &str);
	if (err)
		return err;

	debug("%s: '%s'\n", type, str);

	node = fdt_subnode_offset(fit, images, str);
	if (node < 0) {
		debug("cannot find image node '%s': %d\n", str, node);
		return -EINVAL;
	}

	return node;
}

static int get_aligned_image_offset(struct spl_load_info *info, int offset)
{
	/*
	 * If it is a FS read, get the first address before offset which is
	 * aligned to ARCH_DMA_MINALIGN. If it is raw read return the
	 * block number to which offset belongs.
	 */
	if (info->filename)
		return offset & ~(ARCH_DMA_MINALIGN - 1);

	return offset / info->bl_len;
}

static int get_aligned_image_overhead(struct spl_load_info *info, int offset)
{
	/*
	 * If it is a FS read, get the difference between the offset and
	 * the first address before offset which is aligned to
	 * ARCH_DMA_MINALIGN. If it is raw read return the offset within the
	 * block.
	 */
	if (info->filename)
		return offset & (ARCH_DMA_MINALIGN - 1);

	return offset % info->bl_len;
}

static int get_aligned_image_size(struct spl_load_info *info, int data_size,
				  int offset)
{
	data_size = data_size + get_aligned_image_overhead(info, offset);

	if (info->filename)
		return data_size;

	return (data_size + info->bl_len - 1) / info->bl_len;
}

/**
 * spl_load_fit_image(): load the image described in a certain FIT node
 * @info:	points to information about the device to load data from
 * @sector:	the start sector of the FIT image on the device
 * @fit:	points to the flattened device tree blob describing the FIT
 *		image
 * @base_offset: the beginning of the data area containing the actual
 *		image data, relative to the beginning of the FIT
 * @node:	offset of the DT node describing the image to load (relative
 *		to @fit)
 * @image_info:	will be filled with information about the loaded image
 *		If the FIT node does not contain a "load" (address) property,
 *		the image gets loaded to the address pointed to by the
 *		load_addr member in this struct.
 *
 * Return:	0 on success or a negative error number.
 */
static int spl_load_fit_image(struct spl_load_info *info, ulong sector,
			      void *fit, ulong base_offset, int node,
			      struct spl_image_info *image_info)
{
	int offset;
	size_t length;
	int len;
	ulong size;
	ulong load_addr, load_ptr;
	void *src;
	ulong overhead;
	int nr_sectors;
	int align_len = ARCH_DMA_MINALIGN - 1;
	uint8_t image_comp = -1, type = -1;
	const void *data;
	bool external_data = false;

	if (IS_ENABLED(CONFIG_SPL_FPGA_SUPPORT) ||
	    (IS_ENABLED(CONFIG_SPL_OS_BOOT) && IS_ENABLED(CONFIG_SPL_GZIP))) {
		if (fit_image_get_type(fit, node, &type))
			puts("Cannot get image type.\n");
		else
			debug("%s ", genimg_get_type_name(type));
	}

	if (IS_ENABLED(CONFIG_SPL_OS_BOOT) && IS_ENABLED(CONFIG_SPL_GZIP)) {
		if (fit_image_get_comp(fit, node, &image_comp))
			puts("Cannot get image compression format.\n");
		else
			debug("%s ", genimg_get_comp_name(image_comp));
	}

	if (fit_image_get_load(fit, node, &load_addr))
		load_addr = image_info->load_addr;

	if (!fit_image_get_data_position(fit, node, &offset)) {
		external_data = true;
	} else if (!fit_image_get_data_offset(fit, node, &offset)) {
		offset += base_offset;
		external_data = true;
	}

	if (external_data) {
		/* External data */
		if (fit_image_get_data_size(fit, node, &len))
			return -ENOENT;

		load_ptr = (load_addr + align_len) & ~align_len;
		length = len;

		overhead = get_aligned_image_overhead(info, offset);
		nr_sectors = get_aligned_image_size(info, length, offset);

		if (info->read(info,
			       sector + get_aligned_image_offset(info, offset),
			       nr_sectors, (void *)load_ptr) != nr_sectors)
			return -EIO;

		debug("External data: dst=%lx, offset=%x, size=%lx\n",
		      load_ptr, offset, (unsigned long)length);
		src = (void *)load_ptr + overhead;
	} else {
		/* Embedded data */
		if (fit_image_get_data(fit, node, &data, &length)) {
			puts("Cannot get image data/size\n");
			return -ENOENT;
		}
		debug("Embedded data: dst=%lx, size=%lx\n", load_addr,
		      (unsigned long)length);
		src = (void *)data;
	}

#if CONFIG_IS_ENABLED(FIT_SIGNATURE)
	printf("## Checking hash(es) for Image %s ... ",
	       fit_get_name(fit, node, NULL));
	if (!fit_image_verify_with_data(fit, node,
					 src, length))
		return -EPERM;
	puts("OK\n");
#endif

#ifdef CONFIG_SPL_FIT_IMAGE_POST_PROCESS
	board_fit_image_post_process(&src, &length);
#endif

	if (IS_ENABLED(CONFIG_SPL_GZIP) && image_comp == IH_COMP_GZIP) {
		size = length;
		if (gunzip((void *)load_addr, CONFIG_SYS_BOOTM_LEN,
			   src, &size)) {
			puts("Uncompressing error\n");
			return -EIO;
		}
		length = size;
	} else {
		memcpy((void *)load_addr, src, length);
	}

	if (image_info) {
		image_info->load_addr = load_addr;
		image_info->size = length;
		image_info->entry_point = fdt_getprop_u32(fit, node, "entry");
	}

	return 0;
}

static int spl_fit_append_fdt(struct spl_image_info *spl_image,
			      struct spl_load_info *info, ulong sector,
			      void *fit, int images, ulong base_offset)
{
	struct spl_image_info image_info;
	int node, ret;

	/* Figure out which device tree the board wants to use */
	node = spl_fit_get_image_node(fit, images, FIT_FDT_PROP, 0);
	if (node < 0) {
		debug("%s: cannot find FDT node\n", __func__);
		return node;
	}

	/*
	 * Read the device tree and place it after the image.
	 * Align the destination address to ARCH_DMA_MINALIGN.
	 */
	image_info.load_addr = spl_image->load_addr + spl_image->size;
	ret = spl_load_fit_image(info, sector, fit, base_offset, node,
				 &image_info);

	if (ret < 0)
		return ret;

	/* Make the load-address of the FDT available for the SPL framework */
	spl_image->fdt_addr = (void *)image_info.load_addr;
#if !CONFIG_IS_ENABLED(FIT_IMAGE_TINY)
	/* Try to make space, so we can inject details on the loadables */
	ret = fdt_shrink_to_minimum(spl_image->fdt_addr, 8192);
#endif

	return ret;
}

static int spl_fit_record_loadable(const void *fit, int images, int index,
				   void *blob, struct spl_image_info *image)
{
	int ret = 0;
#if !CONFIG_IS_ENABLED(FIT_IMAGE_TINY)
	char *name;
	int node;

	ret = spl_fit_get_image_name(fit, images, "loadables",
				     index, &name);
	if (ret < 0)
		return ret;

	node = spl_fit_get_image_node(fit, images, "loadables", index);

	ret = fdt_record_loadable(blob, index, name, image->load_addr,
				  image->size, image->entry_point,
				  fdt_getprop(fit, node, "type", NULL),
				  fdt_getprop(fit, node, "os", NULL));
#endif
	return ret;
}

static int spl_fit_image_get_os(const void *fit, int noffset, uint8_t *os)
{
#if CONFIG_IS_ENABLED(FIT_IMAGE_TINY) && !defined(CONFIG_SPL_OS_BOOT)
	return -ENOTSUPP;
#else
	return fit_image_get_os(fit, noffset, os);
#endif
}

int spl_load_simple_fit(struct spl_image_info *spl_image,
			struct spl_load_info *info, ulong sector, void *fit)
{
	int sectors;
	ulong size;
	unsigned long count;
	struct spl_image_info image_info;
	int node = -1;
	int images, ret;
	int base_offset, hsize, align_len = ARCH_DMA_MINALIGN - 1;
	int index = 0;

	/*
	 * For FIT with external data, figure out where the external images
	 * start. This is the base for the data-offset properties in each
	 * image.
	 */
	size = fdt_totalsize(fit);
	size = (size + 3) & ~3;
	size = board_spl_fit_size_align(size);
	base_offset = (size + 3) & ~3;

	/*
	 * So far we only have one block of data from the FIT. Read the entire
	 * thing, including that first block, placing it so it finishes before
	 * where we will load the image.
	 *
	 * Note that we will load the image such that its first byte will be
	 * at the load address. Since that byte may be part-way through a
	 * block, we may load the image up to one block before the load
	 * address. So take account of that here by subtracting an addition
	 * block length from the FIT start position.
	 *
	 * In fact the FIT has its own load address, but we assume it cannot
	 * be before CONFIG_SYS_TEXT_BASE.
	 *
	 * For FIT with data embedded, data is loaded as part of FIT image.
	 * For FIT with external data, data is not loaded in this step.
	 */
	hsize = (size + info->bl_len + align_len) & ~align_len;
	fit = spl_get_load_buffer(-hsize, hsize);
	sectors = get_aligned_image_size(info, size, 0);
	count = info->read(info, sector, sectors, fit);
	debug("fit read sector %lx, sectors=%d, dst=%p, count=%lu, size=0x%lx\n",
	      sector, sectors, fit, count, size);

	if (count == 0)
		return -EIO;

	/* Do any preprocessing */
	board_spl_fit_pre_load(spl_image, info, fit, sector, sectors);

	/* find the node holding the images information */
	images = fdt_path_offset(fit, FIT_IMAGES_PATH);
	if (images < 0) {
		debug("%s: Cannot find /images node: %d\n", __func__, images);
		return -1;
	}

#ifdef CONFIG_SPL_FPGA_SUPPORT
	node = spl_fit_get_image_node(fit, images, "fpga", 0);
	if (node >= 0) {
		/* Load the image and set up the spl_image structure */
		ret = spl_load_fit_image(info, sector, fit, base_offset, node,
					 spl_image);
		if (ret) {
			printf("%s: Cannot load the FPGA: %i\n", __func__, ret);
			return ret;
		}

		debug("FPGA bitstream at: %x, size: %x\n",
		      (u32)spl_image->load_addr, spl_image->size);

		ret = fpga_load(0, (const void *)spl_image->load_addr,
				spl_image->size, BIT_FULL);
		if (ret) {
			printf("%s: Cannot load the image to the FPGA\n",
			       __func__);
			return ret;
		}

		puts("FPGA image loaded from FIT\n");
		node = -1;
	}
#endif

	/*
	 * Find the U-Boot image using the following search order:
	 *   - start at 'firmware' (e.g. an ARM Trusted Firmware)
	 *   - fall back 'kernel' (e.g. a Falcon-mode OS boot
	 *   - fall back to using the first 'loadables' entry
	 */
	if (node < 0)
		node = spl_fit_get_image_node(fit, images, FIT_FIRMWARE_PROP,
					      0);
#ifdef CONFIG_SPL_OS_BOOT
	if (node < 0)
		node = spl_fit_get_image_node(fit, images, FIT_KERNEL_PROP, 0);
#endif
	if (node < 0) {
		debug("could not find firmware image, trying loadables...\n");
		node = spl_fit_get_image_node(fit, images, "loadables", 0);
		/*
		 * If we pick the U-Boot image from "loadables", start at
		 * the second image when later loading additional images.
		 */
		index = 1;
	}
	if (node < 0) {
		debug("%s: Cannot find u-boot image node: %d\n",
		      __func__, node);
		return -1;
	}

	/* Load the image and set up the spl_image structure */
	ret = spl_load_fit_image(info, sector, fit, base_offset, node,
				 spl_image);
	if (ret)
		return ret;

	/*
	 * For backward compatibility, we treat the first node that is
	 * as a U-Boot image, if no OS-type has been declared.
	 */
	if (!spl_fit_image_get_os(fit, node, &spl_image->os))
		debug("Image OS is %s\n", genimg_get_os_name(spl_image->os));
#if !defined(CONFIG_SPL_OS_BOOT)
	else
		spl_image->os = IH_OS_U_BOOT;
#endif

	/*
	 * Booting a next-stage U-Boot may require us to append the FDT.
	 * We allow this to fail, as the U-Boot image might embed its FDT.
	 */
	if (spl_image->os == IH_OS_U_BOOT)
		spl_fit_append_fdt(spl_image, info, sector, fit,
				   images, base_offset);

	/* Now check if there are more images for us to load */
	for (; ; index++) {
		uint8_t os_type = IH_OS_INVALID;

		node = spl_fit_get_image_node(fit, images, "loadables", index);
		if (node < 0)
			break;

		ret = spl_load_fit_image(info, sector, fit, base_offset, node,
					 &image_info);
		if (ret < 0)
			continue;

		if (!spl_fit_image_get_os(fit, node, &os_type))
			debug("Loadable is %s\n", genimg_get_os_name(os_type));
#if CONFIG_IS_ENABLED(FIT_IMAGE_TINY)
		else
			os_type = IH_OS_U_BOOT;
#endif

		if (os_type == IH_OS_U_BOOT) {
			spl_fit_append_fdt(&image_info, info, sector,
					   fit, images, base_offset);
			spl_image->fdt_addr = image_info.fdt_addr;
		}

		/*
		 * If the "firmware" image did not provide an entry point,
		 * use the first valid entry point from the loadables.
		 */
		if (spl_image->entry_point == FDT_ERROR &&
		    image_info.entry_point != FDT_ERROR)
			spl_image->entry_point = image_info.entry_point;

		/* Record our loadables into the FDT */
		if (spl_image->fdt_addr)
			spl_fit_record_loadable(fit, images, index,
						spl_image->fdt_addr,
						&image_info);
	}

	/*
	 * If a platform does not provide CONFIG_SYS_UBOOT_START, U-Boot's
	 * Makefile will set it to 0 and it will end up as the entry point
	 * here. What it actually means is: use the load address.
	 */
	if (spl_image->entry_point == FDT_ERROR || spl_image->entry_point == 0)
		spl_image->entry_point = spl_image->load_addr;

	spl_image->flags |= SPL_FIT_FOUND;

#ifdef CONFIG_SECURE_BOOT
	board_spl_fit_post_load((ulong)fit, size);
#endif

	return 0;
}
