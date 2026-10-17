// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018-2021 NXP
 */

#define LOG_CATEGORY LOGC_ARCH
#include <common.h>
#include <stdlib.h>
#include <errno.h>
#include <imx_container.h>
#include <log.h>
#include <mapmem.h>
#include <spl.h>
#ifdef CONFIG_AHAB_BOOT
#include <asm/mach-imx/ahab.h>
#endif

static struct boot_img_t *read_auth_image(struct spl_image_info *spl_image,
					  struct spl_load_info *info,
					  struct container_hdr *container,
					  int image_index,
					  ulong container_offset)
{
	struct boot_img_t *images;
	ulong offset, overhead, size;

	if (image_index > container->num_images) {
		debug("Invalid image number\n");
		return NULL;
	}

	images = (struct boot_img_t *)((u8 *)container +
				       sizeof(struct container_hdr));

	if (!IS_ALIGNED(images[image_index].offset, spl_get_bl_len(info))) {
		printf("%s: image%d offset not aligned to %u\n",
		       __func__, image_index, spl_get_bl_len(info));
		return NULL;
	}

	size = ALIGN(images[image_index].size, spl_get_bl_len(info));
	offset = images[image_index].offset + container_offset;

	debug("%s: container: %p offset: %lu size: %lu\n", __func__,
	      container, offset, size);
	if (info->read(info, offset, size,
		       map_sysmem(images[image_index].dst - overhead,
				  images[image_index].size)) <
	    images[image_index].size) {
		printf("%s wrong\n", __func__);
		return NULL;
	}

#ifdef CONFIG_AHAB_BOOT
	if (ahab_verify_cntr_image(&images[image_index], image_index))
		return NULL;
#endif

	return &images[image_index];
}

static int read_auth_container(struct spl_image_info *spl_image,
			       struct spl_load_info *info, ulong offset)
{
	struct container_hdr *container = NULL;
	u16 length;
	int i, size, ret = 0;

	size = ALIGN(CONTAINER_HDR_ALIGNMENT, spl_get_bl_len(info));

	/*
	 * It will not override the ATF code, so safe to use it here,
	 * no need malloc
	 */
	container = malloc(size);
	if (!container)
		return -ENOMEM;

	debug("%s: container: %p offset: %lu size: %u\n", __func__,
	      container, offset, size);
	if (info->read(info, offset, size, container) <
	    CONTAINER_HDR_ALIGNMENT) {
		ret = -EIO;
		goto end;
	}

	if (!valid_container_hdr(container)) {
		log_err("Wrong container header\n");
		ret = -ENOENT;
		goto end;
	}

	if (!container->num_images) {
		log_err("Wrong container, no image found\n");
		ret = -ENOENT;
		goto end;
	}

	length = container->length_lsb + (container->length_msb << 8);
	debug("Container length %u\n", length);

	if (length > CONTAINER_HDR_ALIGNMENT) {
		size = ALIGN(length, spl_get_bl_len(info));

		free(container);
		container = malloc(size);
		if (!container)
			return -ENOMEM;

		debug("%s: container: %p offset: %lu size: %u\n",
		      __func__, container, offset, size);
		if (info->read(info, offset, size, container) < length) {
			ret = -EIO;
			goto end;
		}
	}

#ifdef CONFIG_AHAB_BOOT
	ret = ahab_auth_cntr_hdr(container, length);
	if (ret)
		goto end_auth;
#endif

	for (i = 0; i < container->num_images; i++) {
		struct boot_img_t *image = read_auth_image(spl_image, info,
							   container, i,
							   offset);

		if (!image) {
			ret = -EINVAL;
			goto end_auth;
		}

		if (i == 0) {
			spl_image->load_addr = image->dst;
			spl_image->entry_point = image->entry;
		}
	}

end_auth:
#ifdef CONFIG_AHAB_BOOT
	ahab_auth_release();
#endif

end:
	free(container);

	return ret;
}

int spl_load_imx_container(struct spl_image_info *spl_image,
			   struct spl_load_info *info, ulong offset)
{
	return read_auth_container(spl_image, info, offset);
}
