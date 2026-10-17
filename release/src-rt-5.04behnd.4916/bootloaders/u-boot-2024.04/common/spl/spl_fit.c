// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <errno.h>
#include <fpga.h>
#include <gzip.h>
#include <image.h>
#include <log.h>
#include <memalign.h>
#include <mapmem.h>
#include <spl.h>
#include <sysinfo.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <linux/libfdt.h>
#include <linux/printk.h>

DECLARE_GLOBAL_DATA_PTR;

struct spl_fit_info {
	const void *fit;	/* Pointer to a valid FIT blob */
	size_t ext_data_offset;	/* Offset to FIT external data (end of FIT) */
	int images_node;	/* FDT offset to "/images" node */
	int conf_node;		/* FDT offset to selected configuration node */
};

__weak ulong board_spl_fit_size_align(ulong size)
{
	return size;
}

static int find_node_from_desc(const void *fit, int node, const char *str)
{
	int child;

	if (node < 0)
		return -EINVAL;

	/* iterate the FIT nodes and find a matching description */
	for (child = fdt_first_subnode(fit, node); child >= 0;
	     child = fdt_next_subnode(fit, child)) {
		int len;
		const char *desc = fdt_getprop(fit, child, FIT_DESC_PROP, &len);

		if (!desc)
			continue;

		if (!strcmp(desc, str))
			return child;
	}

	return -ENOENT;
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
static int spl_fit_get_image_name(const struct spl_fit_info *ctx,
				  const char *type, int index,
				  const char **outname)
{
	struct udevice *sysinfo;
	const char *name, *str;
	__maybe_unused int node;
	int len, i;
	bool found = true;

	name = fdt_getprop(ctx->fit, ctx->conf_node, type, &len);
	if (!name) {
		debug("cannot find property '%s': %d\n", type, len);
		return -EINVAL;
	}

	str = name;
	for (i = 0; i < index; i++) {
		str = strchr(str, '\0') + 1;
		if (!str || (str - name >= len)) {
			found = false;
			break;
		}
	}

	if (!found && CONFIG_IS_ENABLED(SYSINFO) && !sysinfo_get(&sysinfo)) {
		int rc;
		/*
		 * no string in the property for this index. Check if the
		 * sysinfo-level code can supply one.
		 */
		rc = sysinfo_detect(sysinfo);
		if (rc)
			return rc;

		rc = sysinfo_get_fit_loadable(sysinfo, index - i - 1, type,
					      &str);
		if (rc && rc != -ENOENT)
			return rc;

		if (!rc) {
			/*
			 * The sysinfo provided a name for a loadable.
			 * Try to match it against the description properties
			 * first. If no matching node is found, use it as a
			 * node name.
			 */
			int node;
			int images = fdt_path_offset(ctx->fit, FIT_IMAGES_PATH);

			node = find_node_from_desc(ctx->fit, images, str);
			if (node > 0)
				str = fdt_get_name(ctx->fit, node, NULL);

			found = true;
		}
	}

	if (!found) {
		debug("no string for index %d\n", index);
		return -E2BIG;
	}

	*outname = str;
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
static int spl_fit_get_image_node(const struct spl_fit_info *ctx,
				  const char *type, int index)
{
	const char *str;
	int err;
	int node;

	err = spl_fit_get_image_name(ctx, type, index, &str);
	if (err)
		return err;

	debug("%s: '%s'\n", type, str);

	node = fdt_subnode_offset(ctx->fit, ctx->images_node, str);
	if (node < 0) {
		pr_err("cannot find image node '%s': %d\n", str, node);
		return -EINVAL;
	}

	return node;
}

static int get_aligned_image_offset(struct spl_load_info *info, int offset)
{
	return ALIGN_DOWN(offset, spl_get_bl_len(info));
}

static int get_aligned_image_overhead(struct spl_load_info *info, int offset)
{
	return offset & (spl_get_bl_len(info) - 1);
}

static int get_aligned_image_size(struct spl_load_info *info, int data_size,
				  int offset)
{
	data_size = data_size + get_aligned_image_overhead(info, offset);

	return ALIGN(data_size, spl_get_bl_len(info));
}

/**
 * load_simple_fit(): load the image described in a certain FIT node
 * @info:	points to information about the device to load data from
 * @sector:	the start sector of the FIT image on the device
 * @ctx:	points to the FIT context structure
 * @node:	offset of the DT node describing the image to load (relative
 *		to @fit)
 * @image_info:	will be filled with information about the loaded image
 *		If the FIT node does not contain a "load" (address) property,
 *		the image gets loaded to the address pointed to by the
 *		load_addr member in this struct, if load_addr is not 0
 *
 * Return:	0 on success or a negative error number.
 */
static int load_simple_fit(struct spl_load_info *info, ulong fit_offset,
			   const struct spl_fit_info *ctx, int node,
			   struct spl_image_info *image_info)
{
	int offset;
	size_t length;
	int len;
	ulong size;
	ulong load_addr;
	void *load_ptr;
	void *src;
	ulong overhead;
	uint8_t image_comp = -1, type = -1;
	const void *data;
	const void *fit = ctx->fit;
	bool external_data = false;

	if (IS_ENABLED(CONFIG_SPL_FPGA) ||
	    (IS_ENABLED(CONFIG_SPL_OS_BOOT) && spl_decompression_enabled())) {
		if (fit_image_get_type(fit, node, &type))
			puts("Cannot get image type.\n");
		else
			debug("%s ", genimg_get_type_name(type));
	}

	if (spl_decompression_enabled()) {
		fit_image_get_comp(fit, node, &image_comp);
		debug("%s ", genimg_get_comp_name(image_comp));
	}

	if (fit_image_get_load(fit, node, &load_addr)) {
		if (!image_info->load_addr) {
			printf("Can't load %s: No load address and no buffer\n",
			       fit_get_name(fit, node, NULL));
			return -ENOBUFS;
		}
		load_addr = image_info->load_addr;
	}

	if (!fit_image_get_data_position(fit, node, &offset)) {
		external_data = true;
	} else if (!fit_image_get_data_offset(fit, node, &offset)) {
		offset += ctx->ext_data_offset;
		external_data = true;
	}

	if (external_data) {
		void *src_ptr;

		/* External data */
		if (fit_image_get_data_size(fit, node, &len))
			return -ENOENT;

		/* Dont bother to copy 0 byte data, but warn, though */
		if (!len) {
			log_warning("%s: Skip load '%s': image size is 0!\n",
				    __func__, fit_get_name(fit, node, NULL));
			return 0;
		}

		if (spl_decompression_enabled() &&
		    (image_comp == IH_COMP_GZIP || image_comp == IH_COMP_LZMA))
			src_ptr = map_sysmem(ALIGN(CONFIG_SYS_LOAD_ADDR, ARCH_DMA_MINALIGN), len);
		else
			src_ptr = map_sysmem(ALIGN(load_addr, ARCH_DMA_MINALIGN), len);
		length = len;

		overhead = get_aligned_image_overhead(info, offset);
		size = get_aligned_image_size(info, length, offset);

		if (info->read(info,
			       fit_offset +
			       get_aligned_image_offset(info, offset), size,
			       src_ptr) < length)
			return -EIO;

		debug("External data: dst=%p, offset=%x, size=%lx\n",
		      src_ptr, offset, (unsigned long)length);
		src = src_ptr + overhead;
	} else {
		/* Embedded data */
		if (fit_image_get_data(fit, node, &data, &length)) {
			puts("Cannot get image data/size\n");
			return -ENOENT;
		}
		debug("Embedded data: dst=%lx, size=%lx\n", load_addr,
		      (unsigned long)length);
		src = (void *)data;	/* cast away const */
	}

	if (CONFIG_IS_ENABLED(FIT_SIGNATURE)) {
		printf("## Checking hash(es) for Image %s ... ",
		       fit_get_name(fit, node, NULL));
		if (!fit_image_verify_with_data(fit, node, gd_fdt_blob(), src,
						length))
			return -EPERM;
		puts("OK\n");
	}
	/* Decrypt data before uncompress/move */
	if (IS_ENABLED(CONFIG_FIT_CIPHER) && IMAGE_ENABLE_DECRYPT)  {
		if (fit_image_uncipher(fit, node, &src, &length)) {
			return -EACCES;
		}
	}

	if (CONFIG_IS_ENABLED(FIT_IMAGE_POST_PROCESS))
		board_fit_image_post_process(fit, node, &src, &length);

	load_ptr = map_sysmem(load_addr, length);
	if (IS_ENABLED(CONFIG_SPL_GZIP) && image_comp == IH_COMP_GZIP) {
		size = length;
		if (gunzip(load_ptr, CONFIG_SYS_BOOTM_LEN, src, &size)) {
			puts("Uncompressing error\n");
			return -EIO;
		}
		length = size;
	} else if (IS_ENABLED(CONFIG_SPL_LZMA) && image_comp == IH_COMP_LZMA) {
		size = CONFIG_SYS_BOOTM_LEN;
		ulong loadEnd;

		if (image_decomp(IH_COMP_LZMA, CONFIG_SYS_LOAD_ADDR, 0, 0,
				 load_ptr, src, length, size, &loadEnd)) {
			puts("Uncompressing error\n");
			return -EIO;
		}
		length = loadEnd - CONFIG_SYS_LOAD_ADDR;
	} else {
		memcpy(load_ptr, src, length);
	}

	if (image_info) {
		ulong entry_point;

		image_info->load_addr = load_addr;
		image_info->size = length;

		if (!fit_image_get_entry(fit, node, &entry_point))
			image_info->entry_point = entry_point;
		else
			image_info->entry_point = FDT_ERROR;
	}

	return 0;
}

static bool os_takes_devicetree(uint8_t os)
{
	switch (os) {
	case IH_OS_U_BOOT:
		return true;
	case IH_OS_LINUX:
		return IS_ENABLED(CONFIG_SPL_OS_BOOT) ||
		       IS_ENABLED(CONFIG_SPL_OPENSBI);
	default:
		return false;
	}
}

__weak int board_spl_fit_append_fdt_skip(const char *name)
{
	return 0;	/* Do not skip */
}

static int spl_fit_append_fdt(struct spl_image_info *spl_image,
			      struct spl_load_info *info, ulong offset,
			      const struct spl_fit_info *ctx)
{
	struct spl_image_info image_info;
	int node, ret = 0, index = 0;

	/*
	 * Use the address following the image as target address for the
	 * device tree.
	 */
	image_info.load_addr = spl_image->load_addr + spl_image->size;

	/* Figure out which device tree the board wants to use */
	node = spl_fit_get_image_node(ctx, FIT_FDT_PROP, index++);
	if (node < 0) {
		size_t size;

		debug("%s: cannot find FDT node\n", __func__);

		/*
		 * U-Boot did not find a device tree inside the FIT image. Use
		 * the U-Boot device tree instead.
		 */
		if (!gd->fdt_blob)
			return node;

		/*
		 * Make the load-address of the FDT available for the SPL
		 * framework
		 */
		size = fdt_totalsize(gd->fdt_blob);
		spl_image->fdt_addr = map_sysmem(image_info.load_addr, size);
		memcpy(spl_image->fdt_addr, gd->fdt_blob, size);
	} else {
		ret = load_simple_fit(info, offset, ctx, node, &image_info);
		if (ret < 0)
			return ret;

		spl_image->fdt_addr = phys_to_virt(image_info.load_addr);
	}

	if (CONFIG_IS_ENABLED(FIT_IMAGE_TINY))
		return 0;

#if CONFIG_IS_ENABLED(LOAD_FIT_APPLY_OVERLAY)
		void *tmpbuffer = NULL;

		for (; ; index++) {
			const char *str;

			ret = spl_fit_get_image_name(ctx, FIT_FDT_PROP, index, &str);
			if (ret == -E2BIG) {
				debug("%s: No additional FDT node\n", __func__);
				ret = 0;
				break;
			} else if (ret < 0) {
				continue;
			}

			ret = board_spl_fit_append_fdt_skip(str);
			if (ret)
				continue;

			node = fdt_subnode_offset(ctx->fit, ctx->images_node, str);
			if (node < 0) {
				debug("%s: unable to find FDT node %d\n",
				      __func__, index);
				continue;
			}

			if (!tmpbuffer) {
				/*
				 * allocate memory to store the DT overlay
				 * before it is applied. It may not be used
				 * depending on how the overlay is stored, so
				 * don't fail yet if the allocation failed.
				 */
				size_t size = CONFIG_SPL_LOAD_FIT_APPLY_OVERLAY_BUF_SZ;

				tmpbuffer = malloc_cache_aligned(size);
				if (!tmpbuffer)
					debug("%s: unable to allocate space for overlays\n",
					      __func__);
			}
			image_info.load_addr = (ulong)tmpbuffer;
			ret = load_simple_fit(info, offset, ctx, node,
					      &image_info);
			if (ret < 0)
				break;

			/* Make room in FDT for changes from the overlay */
			ret = fdt_increase_size(spl_image->fdt_addr,
						image_info.size);
			if (ret < 0)
				break;

			ret = fdt_overlay_apply_verbose(spl_image->fdt_addr,
							(void *)image_info.load_addr);
			if (ret) {
				pr_err("failed to apply DT overlay %s\n",
				       fit_get_name(ctx->fit, node, NULL));
				break;
			}

			debug("%s: DT overlay %s applied\n", __func__,
			      fit_get_name(ctx->fit, node, NULL));
		}
		free(tmpbuffer);
		if (ret)
			return ret;
#endif
	/* Try to make space, so we can inject details on the loadables */
	ret = fdt_shrink_to_minimum(spl_image->fdt_addr, 8192);
	if (ret < 0)
		return ret;

	return ret;
}

static int spl_fit_record_loadable(const struct spl_fit_info *ctx, int index,
				   void *blob, struct spl_image_info *image)
{
	int ret = 0;
	const char *name;
	int node;

	if (CONFIG_IS_ENABLED(FIT_IMAGE_TINY))
		return 0;

	ret = spl_fit_get_image_name(ctx, "loadables", index, &name);
	if (ret < 0)
		return ret;

	node = spl_fit_get_image_node(ctx, "loadables", index);

	ret = fdt_record_loadable(blob, index, name, image->load_addr,
			image->size, image->entry_point,
			fdt_getprop(ctx->fit, node, FIT_TYPE_PROP, NULL),
			fdt_getprop(ctx->fit, node, FIT_OS_PROP, NULL),
			fdt_getprop(ctx->fit, node, FIT_ARCH_PROP, NULL));

	return ret;
}

static int spl_fit_image_is_fpga(const void *fit, int node)
{
	const char *type;

	if (!IS_ENABLED(CONFIG_SPL_FPGA))
		return 0;

	type = fdt_getprop(fit, node, FIT_TYPE_PROP, NULL);
	if (!type)
		return 0;

	return !strcmp(type, "fpga");
}

static int spl_fit_image_get_os(const void *fit, int noffset, uint8_t *os)
{
	if (!CONFIG_IS_ENABLED(FIT_IMAGE_TINY) || CONFIG_IS_ENABLED(OS_BOOT))
		return fit_image_get_os(fit, noffset, os);

	const char *name = fdt_getprop(fit, noffset, FIT_OS_PROP, NULL);
	if (!name)
		return -ENOENT;

	/*
	 * We don't care what the type of the image actually is,
	 * only whether or not it is U-Boot. This saves some
	 * space by omitting the large table of OS types.
	 */
	if (!strcmp(name, "u-boot"))
		*os = IH_OS_U_BOOT;
	else
		*os = IH_OS_INVALID;

	return 0;
}

/*
 * The purpose of the FIT load buffer is to provide a memory location that is
 * independent of the load address of any FIT component.
 */
static void *spl_get_fit_load_buffer(size_t size)
{
	void *buf;

	buf = malloc_cache_aligned(size);
	if (!buf) {
		pr_err("Could not get FIT buffer of %lu bytes\n", (ulong)size);
		pr_err("\tcheck CONFIG_SPL_SYS_MALLOC_SIZE\n");
		buf = spl_get_load_buffer(0, size);
	}
	return buf;
}

__weak void *board_spl_fit_buffer_addr(ulong fit_size, int sectors, int bl_len)
{
	return spl_get_fit_load_buffer(sectors * bl_len);
}

/*
 * Weak default function to allow customizing SPL fit loading for load-only
 * use cases by allowing to skip the parsing/processing of the FIT contents
 * (so that this can be done separately in a more customized fashion)
 */
__weak bool spl_load_simple_fit_skip_processing(void)
{
	return false;
}

/*
 * Weak default function to allow fixes after fit header
 * is loaded.
 */
__weak void *spl_load_simple_fit_fix_load(const void *fit)
{
	return (void *)fit;
}

static void warn_deprecated(const char *msg)
{
	printf("DEPRECATED: %s\n", msg);
	printf("\tSee doc/uImage.FIT/source_file_format.txt\n");
}

static int spl_fit_upload_fpga(struct spl_fit_info *ctx, int node,
			       struct spl_image_info *fpga_image)
{
	const char *compatible;
	int ret;
	int devnum = 0;
	int flags = 0;

	debug("FPGA bitstream at: %x, size: %x\n",
	      (u32)fpga_image->load_addr, fpga_image->size);

	compatible = fdt_getprop(ctx->fit, node, "compatible", NULL);
	if (!compatible) {
		warn_deprecated("'fpga' image without 'compatible' property");
	} else {
		if (CONFIG_IS_ENABLED(FPGA_LOAD_SECURE))
			flags = fpga_compatible2flag(devnum, compatible);
		if (strcmp(compatible, "u-boot,fpga-legacy"))
			debug("Ignoring compatible = %s property\n",
			      compatible);
	}

	ret = fpga_load(devnum, (void *)fpga_image->load_addr,
			fpga_image->size, BIT_FULL, flags);
	if (ret) {
		printf("%s: Cannot load the image to the FPGA\n", __func__);
		return ret;
	}

	puts("FPGA image loaded from FIT\n");

	return 0;
}

static int spl_fit_load_fpga(struct spl_fit_info *ctx,
			     struct spl_load_info *info, ulong offset)
{
	int node, ret;

	struct spl_image_info fpga_image = {
		.load_addr = 0,
	};

	node = spl_fit_get_image_node(ctx, "fpga", 0);
	if (node < 0)
		return node;

	warn_deprecated("'fpga' property in config node. Use 'loadables'");

	/* Load the image and set up the fpga_image structure */
	ret = load_simple_fit(info, offset, ctx, node, &fpga_image);
	if (ret) {
		printf("%s: Cannot load the FPGA: %i\n", __func__, ret);
		return ret;
	}

	return spl_fit_upload_fpga(ctx, node, &fpga_image);
}

static int spl_simple_fit_read(struct spl_fit_info *ctx,
			       struct spl_load_info *info, ulong offset,
			       const void *fit_header)
{
	unsigned long count, size;
	void *buf;

	/*
	 * For FIT with external data, figure out where the external images
	 * start. This is the base for the data-offset properties in each
	 * image.
	 */
	size = ALIGN(fdt_totalsize(fit_header), 4);
	size = board_spl_fit_size_align(size);
	ctx->ext_data_offset = ALIGN(size, 4);

	/*
	 * So far we only have one block of data from the FIT. Read the entire
	 * thing, including that first block.
	 *
	 * For FIT with data embedded, data is loaded as part of FIT image.
	 * For FIT with external data, data is not loaded in this step.
	 */
	size = get_aligned_image_size(info, size, 0);
	buf = board_spl_fit_buffer_addr(size, size, 1);

	count = info->read(info, offset, size, buf);
	ctx->fit = buf;
	debug("fit read offset %lx, size=%lu, dst=%p, count=%lu\n",
	      offset, size, buf, count);

	return (count == 0) ? -EIO : 0;
}

static int spl_simple_fit_parse(struct spl_fit_info *ctx)
{
	/* Find the correct subnode under "/configurations" */
	ctx->conf_node = fit_find_config_node(ctx->fit);
	if (ctx->conf_node < 0)
		return -EINVAL;

	if (CONFIG_IS_ENABLED(FIT_SIGNATURE)) {
		printf("## Checking hash(es) for config %s ... ",
		       fit_get_name(ctx->fit, ctx->conf_node, NULL));

		if (fit_config_verify(ctx->fit, ctx->conf_node))
			return -EPERM;

		puts("OK\n");
	}

	/* find the node holding the images information */
	ctx->images_node = fdt_path_offset(ctx->fit, FIT_IMAGES_PATH);
	if (ctx->images_node < 0) {
		debug("%s: Cannot find /images node: %d\n", __func__,
		      ctx->images_node);
		return -EINVAL;
	}

	return 0;
}

int spl_load_simple_fit(struct spl_image_info *spl_image,
			struct spl_load_info *info, ulong offset, void *fit)
{
	struct spl_image_info image_info;
	struct spl_fit_info ctx;
	int node = -1;
	int ret;
	int index = 0;
	int firmware_node;

	ret = spl_simple_fit_read(&ctx, info, offset, fit);
	if (ret < 0)
		return ret;

	/* skip further processing if requested to enable load-only use cases */
	if (spl_load_simple_fit_skip_processing())
		return 0;

	ctx.fit = spl_load_simple_fit_fix_load(ctx.fit);


	ret = spl_simple_fit_parse(&ctx);
	if (ret < 0)
		return ret;

	if (IS_ENABLED(CONFIG_SPL_FPGA))
		spl_fit_load_fpga(&ctx, info, offset);

	/*
	 * Find the U-Boot image using the following search order:
	 *   - start at 'firmware' (e.g. an ARM Trusted Firmware)
	 *   - fall back 'kernel' (e.g. a Falcon-mode OS boot
	 *   - fall back to using the first 'loadables' entry
	 */
	if (node < 0)
		node = spl_fit_get_image_node(&ctx, FIT_FIRMWARE_PROP, 0);

	if (node < 0 && IS_ENABLED(CONFIG_SPL_OS_BOOT))
		node = spl_fit_get_image_node(&ctx, FIT_KERNEL_PROP, 0);

	if (node < 0) {
		debug("could not find firmware image, trying loadables...\n");
		node = spl_fit_get_image_node(&ctx, "loadables", 0);
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
	ret = load_simple_fit(info, offset, &ctx, node, spl_image);
	if (ret)
		return ret;

	/*
	 * For backward compatibility, we treat the first node that is
	 * as a U-Boot image, if no OS-type has been declared.
	 */
	if (!spl_fit_image_get_os(ctx.fit, node, &spl_image->os))
		debug("Image OS is %s\n", genimg_get_os_name(spl_image->os));
	else if (!IS_ENABLED(CONFIG_SPL_OS_BOOT))
		spl_image->os = IH_OS_U_BOOT;

	/*
	 * Booting a next-stage U-Boot may require us to append the FDT.
	 * We allow this to fail, as the U-Boot image might embed its FDT.
	 */
	if (os_takes_devicetree(spl_image->os)) {
		ret = spl_fit_append_fdt(spl_image, info, offset, &ctx);
		if (ret < 0 && spl_image->os != IH_OS_U_BOOT)
			return ret;
	}

	firmware_node = node;
	/* Now check if there are more images for us to load */
	for (; ; index++) {
		uint8_t os_type = IH_OS_INVALID;

		node = spl_fit_get_image_node(&ctx, "loadables", index);
		if (node < 0)
			break;

		/*
		 * if the firmware is also a loadable, skip it because
		 * it already has been loaded. This is typically the case with
		 * u-boot.img generated by mkimage.
		 */
		if (firmware_node == node)
			continue;

		image_info.load_addr = 0;
		ret = load_simple_fit(info, offset, &ctx, node, &image_info);
		if (ret < 0) {
			printf("%s: can't load image loadables index %d (ret = %d)\n",
			       __func__, index, ret);
			return ret;
		}

		if (spl_fit_image_is_fpga(ctx.fit, node))
			spl_fit_upload_fpga(&ctx, node, &image_info);

		if (!spl_fit_image_get_os(ctx.fit, node, &os_type))
			debug("Loadable is %s\n", genimg_get_os_name(os_type));

		if (os_takes_devicetree(os_type)) {
			spl_fit_append_fdt(&image_info, info, offset, &ctx);
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
			spl_fit_record_loadable(&ctx, index,
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

	return 0;
}

/* Parse and load full fitImage in SPL */
int spl_load_fit_image(struct spl_image_info *spl_image,
		       const struct legacy_img_hdr *header)
{
	struct bootm_headers images;
	const char *fit_uname_config = NULL;
	uintptr_t fdt_hack;
	const char *uname;
	ulong fw_data = 0, dt_data = 0, img_data = 0;
	ulong fw_len = 0, dt_len = 0, img_len = 0;
	int idx, conf_noffset;
	int ret;

#if CONFIG_IS_ENABLED(FIT_SIGNATURE)
	images.verify = 1;
#endif
	ret = fit_image_load(&images, virt_to_phys((void *)header),
			     NULL, &fit_uname_config,
			     IH_ARCH_DEFAULT, IH_TYPE_STANDALONE, -1,
			     FIT_LOAD_OPTIONAL, &fw_data, &fw_len);
	if (ret >= 0) {
		printf("DEPRECATED: 'standalone = ' property.");
		printf("Please use either 'firmware =' or 'kernel ='\n");
	} else {
		ret = fit_image_load(&images, virt_to_phys((void *)header),
				     NULL, &fit_uname_config, IH_ARCH_DEFAULT,
				     IH_TYPE_FIRMWARE, -1, FIT_LOAD_OPTIONAL,
				     &fw_data, &fw_len);
	}

	if (ret < 0) {
		ret = fit_image_load(&images, virt_to_phys((void *)header),
				     NULL, &fit_uname_config, IH_ARCH_DEFAULT,
				     IH_TYPE_KERNEL, -1, FIT_LOAD_OPTIONAL,
				     &fw_data, &fw_len);
	}

	if (ret < 0)
		return ret;

	spl_image->size = fw_len;
	spl_image->load_addr = fw_data;
	if (fit_image_get_entry(header, ret, &spl_image->entry_point))
		spl_image->entry_point = fw_data;
	if (fit_image_get_os(header, ret, &spl_image->os))
		spl_image->os = IH_OS_INVALID;
	spl_image->name = genimg_get_os_name(spl_image->os);

	debug(SPL_TPL_PROMPT "payload image: %32s load addr: 0x%lx size: %d\n",
	      spl_image->name, spl_image->load_addr, spl_image->size);

#if CONFIG_IS_ENABLED(FIT_SIGNATURE)
	images.verify = 1;
#endif
	ret = fit_image_load(&images, virt_to_phys((void *)header), NULL,
			     &fit_uname_config, IH_ARCH_DEFAULT, IH_TYPE_FLATDT,
			     -1, FIT_LOAD_OPTIONAL, &dt_data, &dt_len);
	if (ret >= 0) {
		spl_image->fdt_addr = (void *)dt_data;

		if (spl_image->os == IH_OS_U_BOOT) {
			/* HACK: U-Boot expects FDT at a specific address */
			fdt_hack = spl_image->load_addr + spl_image->size;
			fdt_hack = (fdt_hack + 3) & ~3;
			debug("Relocating FDT to %p\n", spl_image->fdt_addr);
			memcpy((void *)fdt_hack, spl_image->fdt_addr, dt_len);
		}
	}

	conf_noffset = fit_conf_get_node((const void *)header,
					 fit_uname_config);
	if (conf_noffset < 0)
		return 0;

	for (idx = 0;
	     uname = fdt_stringlist_get((const void *)header, conf_noffset,
					FIT_LOADABLE_PROP, idx,
				NULL), uname;
	     idx++) {
#if CONFIG_IS_ENABLED(FIT_SIGNATURE)
		images.verify = 1;
#endif
		ret = fit_image_load(&images, (ulong)header,
				     &uname, &fit_uname_config,
				     IH_ARCH_DEFAULT, IH_TYPE_LOADABLE, -1,
				     FIT_LOAD_OPTIONAL_NON_ZERO,
				     &img_data, &img_len);
		if (ret < 0)
			return ret;
	}

	return 0;
}
