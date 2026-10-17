// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002
 * Detlev Zundel, DENX Software Engineering, dzu@denx.de.
 */

/*
 * BMP handling routines
 */

#include <common.h>
#include <bmp_layout.h>
#include <command.h>
#include <dm.h>
#include <gzip.h>
#include <log.h>
#include <malloc.h>
#include <mapmem.h>
#include <splash.h>
#include <video.h>
#include <asm/byteorder.h>

/*
 * Allocate and decompress a BMP image using gunzip().
 *
 * Returns a pointer to the decompressed image data. This pointer is
 * aligned to 32-bit-aligned-address + 2.
 * See doc/README.displaying-bmps for explanation.
 *
 * The allocation address is passed to 'alloc_addr' and must be freed
 * by the caller after use.
 *
 * Returns NULL if decompression failed, or if the decompressed data
 * didn't contain a valid BMP signature or decompression is not enabled in
 * Kconfig.
 */
struct bmp_image *gunzip_bmp(unsigned long addr, unsigned long *lenp,
			     void **alloc_addr)
{
	void *dst;
	unsigned long len;
	struct bmp_image *bmp;

	if (!CONFIG_IS_ENABLED(VIDEO_BMP_GZIP))
		return NULL;

	/*
	 * Decompress bmp image
	 */
	len = CONFIG_VAL(VIDEO_LOGO_MAX_SIZE);
	/* allocate extra 3 bytes for 32-bit-aligned-address + 2 alignment */
	dst = malloc(CONFIG_VAL(VIDEO_LOGO_MAX_SIZE) + 3);
	if (!dst) {
		puts("Error: malloc in gunzip failed!\n");
		return NULL;
	}

	/* align to 32-bit-aligned-address + 2 */
	bmp = dst + 2;

	if (gunzip(bmp, CONFIG_VAL(VIDEO_LOGO_MAX_SIZE), map_sysmem(addr, 0),
		   &len)) {
		free(dst);
		return NULL;
	}
	if (len == CONFIG_VAL(VIDEO_LOGO_MAX_SIZE))
		puts("Image could be truncated (increase CONFIG_VIDEO_LOGO_MAX_SIZE)!\n");

	/*
	 * Check for bmp mark 'BM'
	 */
	if (!((bmp->header.signature[0] == 'B') &&
	      (bmp->header.signature[1] == 'M'))) {
		free(dst);
		return NULL;
	}

	debug("Gzipped BMP image detected!\n");

	*alloc_addr = dst;
	return bmp;
}

int bmp_info(ulong addr)
{
	struct bmp_image *bmp = (struct bmp_image *)map_sysmem(addr, 0);
	void *bmp_alloc_addr = NULL;
	unsigned long len;

	if (!((bmp->header.signature[0] == 'B') &&
	      (bmp->header.signature[1] == 'M')))
		bmp = gunzip_bmp(addr, &len, &bmp_alloc_addr);

	if (!bmp) {
		printf("There is no valid bmp file at the given address\n");
		return 1;
	}

	printf("Image size    : %d x %d\n", le32_to_cpu(bmp->header.width),
	       le32_to_cpu(bmp->header.height));
	printf("Bits per pixel: %d\n", le16_to_cpu(bmp->header.bit_count));
	printf("Compression   : %d\n", le32_to_cpu(bmp->header.compression));

	if (bmp_alloc_addr)
		free(bmp_alloc_addr);

	return 0;
}

int bmp_display(ulong addr, int x, int y)
{
	struct udevice *dev;
	int ret;
	struct bmp_image *bmp = map_sysmem(addr, 0);
	void *bmp_alloc_addr = NULL;
	unsigned long len;

	if (!((bmp->header.signature[0] == 'B') &&
	      (bmp->header.signature[1] == 'M')))
		bmp = gunzip_bmp(addr, &len, &bmp_alloc_addr);

	if (!bmp) {
		printf("There is no valid bmp file at the given address\n");
		return 1;
	}
	addr = map_to_sysmem(bmp);

	ret = uclass_first_device_err(UCLASS_VIDEO, &dev);
	if (!ret) {
		bool align = false;

		if (x == BMP_ALIGN_CENTER || y == BMP_ALIGN_CENTER)
			align = true;

		ret = video_bmp_display(dev, addr, x, y, align);
	}

	if (bmp_alloc_addr)
		free(bmp_alloc_addr);

	return ret ? CMD_RET_FAILURE : 0;
}
