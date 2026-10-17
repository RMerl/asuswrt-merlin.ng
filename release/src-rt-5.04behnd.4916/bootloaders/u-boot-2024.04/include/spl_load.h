/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) Sean Anderson <seanga2@gmail.com>
 */
#ifndef	_SPL_LOAD_H_
#define	_SPL_LOAD_H_

#include <image.h>
#include <imx_container.h>
#include <mapmem.h>
#include <spl.h>

static inline int _spl_load(struct spl_image_info *spl_image,
			    const struct spl_boot_device *bootdev,
			    struct spl_load_info *info, size_t size,
			    size_t offset)
{
	struct legacy_img_hdr *header =
		spl_get_load_buffer(-sizeof(*header), sizeof(*header));
	ulong base_offset, image_offset, overhead;
	int read, ret;

	read = info->read(info, offset, ALIGN(sizeof(*header),
					      spl_get_bl_len(info)), header);
	if (read < sizeof(*header))
		return -EIO;

	if (image_get_magic(header) == FDT_MAGIC) {
		if (IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL)) {
			void *buf;

			/*
			 * In order to support verifying images in the FIT, we
			 * need to load the whole FIT into memory. Try and
			 * guess how much we need to load by using the total
			 * size. This will fail for FITs with external data,
			 * but there's not much we can do about that.
			 */
			if (!size)
				size = round_up(fdt_totalsize(header), 4);
			buf = map_sysmem(CONFIG_SYS_LOAD_ADDR, size);
			read = info->read(info, offset,
					  ALIGN(size, spl_get_bl_len(info)),
					  buf);
			if (read < size)
				return -EIO;

			return spl_parse_image_header(spl_image, bootdev, buf);
		}

		if (CONFIG_IS_ENABLED(LOAD_FIT))
			return spl_load_simple_fit(spl_image, info, offset,
						   header);
	}

	if (IS_ENABLED(CONFIG_SPL_LOAD_IMX_CONTAINER) &&
	    valid_container_hdr((void *)header))
		return spl_load_imx_container(spl_image, info, offset);

	if (IS_ENABLED(CONFIG_SPL_LZMA) &&
	    image_get_magic(header) == IH_MAGIC &&
	    image_get_comp(header) == IH_COMP_LZMA) {
		spl_image->flags |= SPL_COPY_PAYLOAD_ONLY;
		ret = spl_parse_image_header(spl_image, bootdev, header);
		if (ret)
			return ret;

		return spl_load_legacy_lzma(spl_image, info, offset);
	}

	ret = spl_parse_image_header(spl_image, bootdev, header);
	if (ret)
		return ret;

	base_offset = spl_image->offset;
	/* Only NOR sets this flag. */
	if (IS_ENABLED(CONFIG_SPL_NOR_SUPPORT) &&
	    spl_image->flags & SPL_COPY_PAYLOAD_ONLY)
		base_offset += sizeof(*header);
	image_offset = ALIGN_DOWN(base_offset, spl_get_bl_len(info));
	overhead = base_offset - image_offset;
	size = ALIGN(spl_image->size + overhead, spl_get_bl_len(info));

	read = info->read(info, offset + image_offset, size,
			  map_sysmem(spl_image->load_addr - overhead, size));
	return read < spl_image->size ? -EIO : 0;
}

/*
 * Although spl_load results in size reduction for callers, this is generally
 * not enough to counteract the bloat if there is only one caller. The core
 * problem is that the compiler can't optimize across translation units. The
 * general solution to this is CONFIG_LTO, but that is not available on all
 * architectures. Perform a pseudo-LTO just for this function by declaring it
 * inline if there is one caller, and extern otherwise.
 */
#define SPL_LOAD_USERS \
	IS_ENABLED(CONFIG_SPL_BLK_FS) + \
	IS_ENABLED(CONFIG_SPL_FS_EXT4) + \
	IS_ENABLED(CONFIG_SPL_FS_FAT) + \
	IS_ENABLED(CONFIG_SPL_SYS_MMCSD_RAW_MODE) + \
	(IS_ENABLED(CONFIG_SPL_NAND_SUPPORT) && !IS_ENABLED(CONFIG_SPL_UBI)) + \
	IS_ENABLED(CONFIG_SPL_NET) + \
	IS_ENABLED(CONFIG_SPL_NOR_SUPPORT) + \
	IS_ENABLED(CONFIG_SPL_SEMIHOSTING) + \
	IS_ENABLED(CONFIG_SPL_SPI_LOAD) + \
	0

#if SPL_LOAD_USERS > 1
/**
 * spl_load() - Parse a header and load the image
 * @spl_image: Image data which will be filled in by this function
 * @bootdev: The device to load from
 * @info: Describes how to load additional information from @bootdev. At the
 *        minimum, read() and bl_len must be populated.
 * @size: The size of the image, in bytes, if it is known in advance. Some boot
 *        devices (such as filesystems) know how big an image is before parsing
 *        the header. If 0, then the size will be determined from the header.
 * @offset: The offset from the start of @bootdev, in bytes. This should have
 *          the offset @header was loaded from. It will be added to any offsets
 *          passed to @info->read().
 *
 * This function determines the image type (FIT, legacy, i.MX, raw, etc), calls
 * the appropriate parsing function, determines the load address, and the loads
 * the image from storage. It is designed to replace ad-hoc image loading which
 * may not support all image types (especially when config options are
 * involved).
 *
 * Return: 0 on success, or a negative error on failure
 */
int spl_load(struct spl_image_info *spl_image,
	     const struct spl_boot_device *bootdev, struct spl_load_info *info,
	     size_t size, size_t offset);
#else
static inline int spl_load(struct spl_image_info *spl_image,
			   const struct spl_boot_device *bootdev,
			   struct spl_load_info *info, size_t size,
			   size_t offset)
{
	return _spl_load(spl_image, bootdev, info, size, offset);
}
#endif

#endif /* _SPL_LOAD_H_ */
