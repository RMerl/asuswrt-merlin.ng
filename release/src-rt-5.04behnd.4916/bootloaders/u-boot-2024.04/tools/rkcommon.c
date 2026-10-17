// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Google,  Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * (C) 2017 Theobroma Systems Design und Consulting GmbH
 *
 * Helper functions for Rockchip images
 */

#include "imagetool.h"
#include <image.h>
#include <u-boot/sha256.h>
#include <rc4.h>
#include "mkimage.h"
#include "rkcommon.h"

enum {
	RK_MAGIC		= 0x0ff0aa55,
	RK_MAGIC_V2		= 0x534E4B52,
};

enum {
	RK_HEADER_V1	= 1,
	RK_HEADER_V2	= 2,
};

enum hash_type {
	HASH_NONE	= 0,
	HASH_SHA256	= 1,
	HASH_SHA512	= 2,
};

/**
 * struct image_entry
 *
 * @size_and_off:	[31:16]image size;[15:0]image offset
 * @address:	default as 0xFFFFFFFF
 * @flag:	no use
 * @counter:	no use
 * @hash:	hash of image
 *
 */
struct image_entry {
	uint32_t size_and_off;
	uint32_t address;
	uint32_t flag;
	uint32_t counter;
	uint8_t reserved[8];
	uint8_t hash[64];
};

/**
 * struct header0_info_v2 - v2 header block for rockchip BootRom
 *
 * This is stored at SD card block 64 (where each block is 512 bytes)
 *
 * @magic:	Magic (must be RK_MAGIC_V2)
 * @size_and_nimage:	[31:16]number of images;[15:0]
 *			offset to hash field of header(unit as 4Byte)
 * @boot_flag:	[3:0]hash type(0:none,1:sha256,2:sha512)
 * @signature:	hash or signature for header info
 *
 */
struct header0_info_v2 {
	uint32_t magic;
	uint8_t reserved[4];
	uint32_t size_and_nimage;
	uint32_t boot_flag;
	uint8_t reserved1[104];
	struct image_entry images[4];
	uint8_t reserved2[1064];
	uint8_t hash[512];
};

/**
 * struct header0_info - header block for boot ROM
 *
 * This is stored at SD card block 64 (where each block is 512 bytes, or at
 * the start of SPI flash. It is encoded with RC4.
 *
 * @magic:		Magic (must be RK_MAGIC)
 * @disable_rc4:	0 to use rc4 for boot image,  1 to use plain binary
 * @init_offset:	Offset in blocks of the SPL code from this header
 *			block. E.g. 4 means 2KB after the start of this header.
 * Other fields are not used by U-Boot
 */
struct header0_info {
	uint32_t magic;
	uint8_t reserved[4];
	uint32_t disable_rc4;
	uint16_t init_offset;
	uint8_t reserved1[492];
	uint16_t init_size;
	uint16_t init_boot_size;
	uint8_t reserved2[2];
};

/**
 * struct header1_info
 */
struct header1_info {
	uint32_t magic;
};

/**
 * struct spl_info - spl info for each chip
 *
 * @imagename:		Image name(passed by "mkimage -n")
 * @spl_hdr:		Boot ROM requires a 4-bytes spl header
 * @spl_size:		Spl size(include extra 4-bytes spl header)
 * @spl_rc4:		RC4 encode the SPL binary (same key as header)
 * @header_ver:		header block version
 */
struct spl_info {
	const char *imagename;
	const char *spl_hdr;
	const uint32_t spl_size;
	const bool spl_rc4;
	const uint32_t header_ver;
};

static struct spl_info spl_infos[] = {
	{ "px30", "RK33", 0x2800, false, RK_HEADER_V1 },
	{ "rk3036", "RK30", 0x1000, false, RK_HEADER_V1 },
	{ "rk3066", "RK30", 0x8000 - 0x800, true, RK_HEADER_V1 },
	{ "rk3128", "RK31", 0x1800, false, RK_HEADER_V1 },
	{ "rk3188", "RK31", 0x8000 - 0x800, true, RK_HEADER_V1 },
	{ "rk322x", "RK32", 0x8000 - 0x1000, false, RK_HEADER_V1 },
	{ "rk3288", "RK32", 0x8000, false, RK_HEADER_V1 },
	{ "rk3308", "RK33", 0x40000 - 0x1000, false, RK_HEADER_V1 },
	{ "rk3328", "RK32", 0x8000 - 0x800, false, RK_HEADER_V1 },
	{ "rk3368", "RK33", 0x8000 - 0x1000, false, RK_HEADER_V1 },
	{ "rk3399", "RK33", 0x30000 - 0x2000, false, RK_HEADER_V1 },
	{ "rv1108", "RK11", 0x1800, false, RK_HEADER_V1 },
	{ "rv1126", "110B", 0x10000 - 0x1000, false, RK_HEADER_V1 },
	{ "rk3568", "RK35", 0x10000 - 0x1000, false, RK_HEADER_V2 },
	{ "rk3588", "RK35", 0x100000 - 0x1000, false, RK_HEADER_V2 },
};

/**
 * struct spl_params - spl params parsed in check_params()
 *
 * @init_file:		Init data file path
 * @init_size:		Aligned size of init data in bytes
 * @boot_file:		Boot data file path
 * @boot_size:		Aligned size of boot data in bytes
 */

struct spl_params {
	char *init_file;
	uint32_t init_size;
	char *boot_file;
	uint32_t boot_size;
};

static struct spl_params spl_params = { 0 };

static const unsigned char rc4_key[16] = {
	124, 78, 3, 4, 85, 5, 9, 7,
	45, 44, 123, 56, 23, 13, 23, 17
};

static struct spl_info *rkcommon_get_spl_info(char *imagename)
{
	int i;

	if (!imagename)
		return NULL;

	for (i = 0; i < ARRAY_SIZE(spl_infos); i++)
		if (!strncmp(imagename, spl_infos[i].imagename, 6))
			return spl_infos + i;

	return NULL;
}

static int rkcommon_get_aligned_size(struct image_tool_params *params,
				     const char *fname)
{
	int size;

	size = imagetool_get_filesize(params, fname);
	if (size < 0)
		return -1;

	/*
	 * Pad to a 2KB alignment, as required for init/boot size by the ROM
	 * (see https://lists.denx.de/pipermail/u-boot/2017-May/293268.html)
	 */
	return ROUND(size, RK_SIZE_ALIGN);
}

int rkcommon_check_params(struct image_tool_params *params)
{
	int i, size;

	/*
	 * If this is a operation (list or extract), the don't require
	 * imagename to be set.
	 */
	if (params->lflag || params->iflag)
		return EXIT_SUCCESS;

	if (!rkcommon_get_spl_info(params->imagename))
		goto err_spl_info;

	spl_params.init_file = params->datafile;

	spl_params.boot_file = strchr(spl_params.init_file, ':');
	if (spl_params.boot_file) {
		*spl_params.boot_file = '\0';
		spl_params.boot_file += 1;
	}

	size = rkcommon_get_aligned_size(params, spl_params.init_file);
	if (size < 0)
		return EXIT_FAILURE;
	spl_params.init_size = size;

	/* Boot file is optional, and only for back-to-bootrom functionality. */
	if (spl_params.boot_file) {
		size = rkcommon_get_aligned_size(params, spl_params.boot_file);
		if (size < 0)
			return EXIT_FAILURE;
		spl_params.boot_size = size;
	}

	if (spl_params.init_size > rkcommon_get_spl_size(params)) {
		fprintf(stderr,
			"Error: SPL image is too large (size %#x than %#x)\n",
			spl_params.init_size, rkcommon_get_spl_size(params));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;

err_spl_info:
	fprintf(stderr, "ERROR: imagename (%s) is not supported!\n",
		params->imagename ? params->imagename : "NULL");

	fprintf(stderr, "Available imagename:");
	for (i = 0; i < ARRAY_SIZE(spl_infos); i++)
		fprintf(stderr, "\t%s", spl_infos[i].imagename);
	fprintf(stderr, "\n");

	return EXIT_FAILURE;
}

const char *rkcommon_get_spl_hdr(struct image_tool_params *params)
{
	struct spl_info *info = rkcommon_get_spl_info(params->imagename);

	/*
	 * info would not be NULL, because of we checked params before.
	 */
	return info->spl_hdr;
}

int rkcommon_get_spl_size(struct image_tool_params *params)
{
	struct spl_info *info = rkcommon_get_spl_info(params->imagename);

	/*
	 * info would not be NULL, because of we checked params before.
	 */
	return info->spl_size;
}

bool rkcommon_need_rc4_spl(struct image_tool_params *params)
{
	struct spl_info *info = rkcommon_get_spl_info(params->imagename);

	/*
	 * info would not be NULL, because of we checked params before.
	 */
	return info->spl_rc4;
}

bool rkcommon_is_header_v2(struct image_tool_params *params)
{
	struct spl_info *info = rkcommon_get_spl_info(params->imagename);

	return (info->header_ver == RK_HEADER_V2);
}

static void do_sha256_hash(uint8_t *buf, uint32_t size, uint8_t *out)
{
	sha256_context ctx;

	sha256_starts(&ctx);
	sha256_update(&ctx, buf, size);
	sha256_finish(&ctx, out);
}

static void rkcommon_set_header0(void *buf, struct image_tool_params *params)
{
	struct header0_info *hdr = buf;
	uint32_t init_boot_size;

	memset(buf, '\0', RK_INIT_OFFSET * RK_BLK_SIZE);
	hdr->magic = cpu_to_le32(RK_MAGIC);
	hdr->disable_rc4 = cpu_to_le32(!rkcommon_need_rc4_spl(params));
	hdr->init_offset = cpu_to_le16(RK_INIT_OFFSET);
	hdr->init_size   = cpu_to_le16(spl_params.init_size / RK_BLK_SIZE);

	/*
	 * init_boot_size needs to be set, as it is read by the BootROM
	 * to determine the size of the next-stage bootloader (e.g. U-Boot
	 * proper), when used with the back-to-bootrom functionality.
	 *
	 * see https://lists.denx.de/pipermail/u-boot/2017-May/293267.html
	 * for a more detailed explanation by Andy Yan
	 */
	if (spl_params.boot_file)
		init_boot_size = spl_params.init_size + spl_params.boot_size;
	else
		init_boot_size = spl_params.init_size + RK_MAX_BOOT_SIZE;
	hdr->init_boot_size = cpu_to_le16(init_boot_size / RK_BLK_SIZE);

	rc4_encode(buf, RK_BLK_SIZE, rc4_key);
}

static void rkcommon_set_header0_v2(void *buf, struct image_tool_params *params)
{
	struct header0_info_v2 *hdr = buf;
	uint32_t sector_offset, image_sector_count;
	uint32_t image_size_array[2];
	uint8_t *image_ptr = NULL;
	int i;

	printf("Image Type:   Rockchip %s boot image\n",
		rkcommon_get_spl_hdr(params));
	memset(buf, '\0', RK_INIT_OFFSET * RK_BLK_SIZE);
	hdr->magic   = cpu_to_le32(RK_MAGIC_V2);
	hdr->size_and_nimage = cpu_to_le32((2 << 16) + 384);
	hdr->boot_flag = cpu_to_le32(HASH_SHA256);
	sector_offset = 4;
	image_size_array[0] = spl_params.init_size;
	image_size_array[1] = spl_params.boot_size;

	for (i = 0; i < 2; i++) {
		image_sector_count = image_size_array[i] / RK_BLK_SIZE;
		hdr->images[i].size_and_off = cpu_to_le32((image_sector_count
							<< 16) + sector_offset);
		hdr->images[i].address = 0xFFFFFFFF;
		hdr->images[i].counter = cpu_to_le32(i + 1);
		image_ptr = buf + sector_offset * RK_BLK_SIZE;
		do_sha256_hash(image_ptr, image_size_array[i],
			       hdr->images[i].hash);
		sector_offset = sector_offset + image_sector_count;
	}

	do_sha256_hash(buf, (void *)hdr->hash - buf, hdr->hash);
}

void rkcommon_set_header(void *buf,  struct stat *sbuf,  int ifd,
			 struct image_tool_params *params)
{
	struct header1_info *hdr = buf + RK_SPL_HDR_START;

	if (rkcommon_is_header_v2(params)) {
		rkcommon_set_header0_v2(buf, params);
	} else {
		rkcommon_set_header0(buf, params);

		/* Set up the SPL name (i.e. copy spl_hdr over) */
		if (memcmp(&hdr->magic, "RSAK", 4))
			memcpy(&hdr->magic, rkcommon_get_spl_hdr(params), RK_SPL_HDR_SIZE);

		if (rkcommon_need_rc4_spl(params))
			rkcommon_rc4_encode_spl(buf, RK_SPL_HDR_START,
						spl_params.init_size);

		if (spl_params.boot_file) {
			if (rkcommon_need_rc4_spl(params))
				rkcommon_rc4_encode_spl(buf + RK_SPL_HDR_START,
							spl_params.init_size,
							spl_params.boot_size);
		}
	}
}

static inline unsigned int rkcommon_offset_to_spi(unsigned int offset)
{
	/*
	 * While SD/MMC images use a flat addressing, SPI images are padded
	 * to use the first 2K of every 4K sector only.
	 */
	return ((offset & ~0x7ff) << 1) + (offset & 0x7ff);
}

static int rkcommon_parse_header(const void *buf, struct header0_info *header0,
				 struct spl_info **spl_info)
{
	unsigned int hdr1_offset;
	struct header1_info *hdr1_sdmmc, *hdr1_spi;
	int i;

	if (spl_info)
		*spl_info = NULL;

	/*
	 * The first header (hdr0) is always RC4 encoded, so try to decrypt
	 * with the well-known key.
	 */
	memcpy((void *)header0, buf, sizeof(struct header0_info));
	rc4_encode((void *)header0, sizeof(struct header0_info), rc4_key);

	if (le32_to_cpu(header0->magic) != RK_MAGIC)
		return -EPROTO;

	/* We don't support RC4 encoded image payloads here, yet... */
	if (le32_to_cpu(header0->disable_rc4) == 0)
		return -ENOSYS;

	hdr1_offset = le16_to_cpu(header0->init_offset) * RK_BLK_SIZE;
	hdr1_sdmmc = (struct header1_info *)(buf + hdr1_offset);
	hdr1_spi = (struct header1_info *)(buf +
					   rkcommon_offset_to_spi(hdr1_offset));

	for (i = 0; i < ARRAY_SIZE(spl_infos); i++) {
		if (!memcmp(&hdr1_sdmmc->magic, spl_infos[i].spl_hdr,
			    RK_SPL_HDR_SIZE)) {
			if (spl_info)
				*spl_info = &spl_infos[i];
			return IH_TYPE_RKSD;
		} else if (!memcmp(&hdr1_spi->magic, spl_infos[i].spl_hdr,
				   RK_SPL_HDR_SIZE)) {
			if (spl_info)
				*spl_info = &spl_infos[i];
			return IH_TYPE_RKSPI;
		}
	}

	return -1;
}

static int rkcommon_parse_header_v2(const void *buf, struct header0_info_v2 *header)
{
	memcpy((void *)header, buf, sizeof(struct header0_info_v2));

	if (le32_to_cpu(header->magic) != RK_MAGIC_V2)
		return -EPROTO;

	return 0;
}

int rkcommon_verify_header(unsigned char *buf, int size,
			   struct image_tool_params *params)
{
	struct header0_info header0;
	struct spl_info *img_spl_info, *spl_info;
	int ret;

	/* spl_hdr is abandon on header_v2 */
	if ((*(uint32_t *)buf) == RK_MAGIC_V2)
		return 0;

	ret = rkcommon_parse_header(buf, &header0, &img_spl_info);

	/* If this is the (unimplemented) RC4 case, then rewrite the result */
	if (ret == -ENOSYS)
		return 0;

	if (ret < 0)
		return ret;

	/*
	 * If no 'imagename' is specified via the commandline (e.g. if this is
	 * 'dumpimage -l' w/o any further constraints), we accept any spl_info.
	 */
	if (params->imagename == NULL)
		return 0;

	/* Match the 'imagename' against the 'spl_hdr' found */
	spl_info = rkcommon_get_spl_info(params->imagename);
	if (spl_info && img_spl_info)
		return strcmp(spl_info->spl_hdr, img_spl_info->spl_hdr);

	return -ENOENT;
}

void rkcommon_print_header(const void *buf, struct image_tool_params *params)
{
	struct header0_info header0;
	struct header0_info_v2 header0_v2;
	struct spl_info *spl_info;
	uint8_t image_type;
	int ret, boot_size, init_size;

	if ((*(uint32_t *)buf) == RK_MAGIC_V2) {
		ret = rkcommon_parse_header_v2(buf, &header0_v2);

		if (ret < 0) {
			fprintf(stderr, "Error: image verification failed\n");
			return;
		}

		init_size = header0_v2.images[0].size_and_off >> 16;
		init_size = init_size * RK_BLK_SIZE;
		boot_size = header0_v2.images[1].size_and_off >> 16;
		boot_size = boot_size * RK_BLK_SIZE;
	} else {
		ret = rkcommon_parse_header(buf, &header0, &spl_info);

		/* If this is the (unimplemented) RC4 case, then fail silently */
		if (ret == -ENOSYS)
			return;

		if (ret < 0) {
			fprintf(stderr, "Error: image verification failed\n");
			return;
		}

		image_type = ret;
		init_size = header0.init_size * RK_BLK_SIZE;
		boot_size = header0.init_boot_size * RK_BLK_SIZE - init_size;

		printf("Image Type:   Rockchip %s (%s) boot image\n",
		       spl_info->spl_hdr,
		       (image_type == IH_TYPE_RKSD) ? "SD/MMC" : "SPI");
	}

	printf("Init Data Size: %d bytes\n", init_size);

	if (boot_size != RK_MAX_BOOT_SIZE)
		printf("Boot Data Size: %d bytes\n", boot_size);
}

void rkcommon_rc4_encode_spl(void *buf, unsigned int offset, unsigned int size)
{
	unsigned int remaining = size;

	while (remaining > 0) {
		int step = (remaining > RK_BLK_SIZE) ? RK_BLK_SIZE : remaining;

		rc4_encode(buf + offset, step, rc4_key);
		offset += RK_BLK_SIZE;
		remaining -= step;
	}
}

int rkcommon_vrec_header(struct image_tool_params *params,
			 struct image_type_params *tparams)
{
	/*
	 * The SPL image looks as follows:
	 *
	 * 0x0    header0 (see rkcommon.c)
	 * 0x800  spl_name ('RK30', ..., 'RK33')
	 *        (start of the payload for AArch64 payloads: we expect the
	 *        first 4 bytes to be available for overwriting with our
	 *        spl_name)
	 * 0x804  first instruction to be executed
	 *        (start of the image/payload for 32bit payloads)
	 *
	 * For AArch64 (ARMv8) payloads, natural alignment (8-bytes) is
	 * required for its sections (so the image we receive needs to
	 * have the first 4 bytes reserved for the spl_name).  Reserving
	 * these 4 bytes is done using the BOOT0_HOOK infrastructure.
	 *
	 * The header is always at 0x800 (as we now use a payload
	 * prepadded using the boot0 hook for all targets): the first
	 * 4 bytes of these images can safely be overwritten using the
	 * boot magic.
	 */
	tparams->header_size = RK_SPL_HDR_START;

	/* Allocate, clear and install the header */
	tparams->hdr = malloc(tparams->header_size);
	if (!tparams->hdr) {
		fprintf(stderr, "%s: Can't alloc header: %s\n",
			params->cmdname, strerror(errno));
		exit(EXIT_FAILURE);
	}
	memset(tparams->hdr, 0, tparams->header_size);

	/*
	 * We need to store the original file-size (i.e. before padding), as
	 * imagetool does not set this during its adjustment of file_size.
	 */
	params->orig_file_size = tparams->header_size +
		spl_params.init_size + spl_params.boot_size;

	params->file_size = ROUND(params->orig_file_size, RK_SIZE_ALIGN);

	/* Ignoring pad len, since we are using our own copy_image() */
	return 0;
}

static int pad_file(struct image_tool_params *params, int ifd, int pad)
{
	uint8_t zeros[4096];

	memset(zeros, 0, sizeof(zeros));

	while (pad > 0) {
		int todo = sizeof(zeros);

		if (todo > pad)
			todo = pad;
		if (write(ifd, (char *)&zeros, todo) != todo) {
			fprintf(stderr, "%s: Write error on %s: %s\n",
				params->cmdname, params->imagefile,
				strerror(errno));
			return -1;
		}
		pad -= todo;
	}

	return 0;
}

static int copy_file(struct image_tool_params *params, int ifd,
		     const char *file, int padded_size)
{
	int dfd;
	struct stat sbuf;
	unsigned char *ptr;
	int size;

	if (params->vflag)
		fprintf(stderr, "Adding Image %s\n", file);

	dfd = open(file, O_RDONLY | O_BINARY);
	if (dfd < 0) {
		fprintf(stderr, "%s: Can't open %s: %s\n",
			params->cmdname, file, strerror(errno));
		return -1;
	}

	if (fstat(dfd, &sbuf) < 0) {
		fprintf(stderr, "%s: Can't stat %s: %s\n",
			params->cmdname, file, strerror(errno));
		goto err_close;
	}

	if (params->vflag)
		fprintf(stderr, "Size %u(pad to %u)\n",
			(int)sbuf.st_size, padded_size);

	ptr = mmap(0, sbuf.st_size, PROT_READ, MAP_SHARED, dfd, 0);
	if (ptr == MAP_FAILED) {
		fprintf(stderr, "%s: Can't read %s: %s\n",
			params->cmdname, file, strerror(errno));
		goto err_munmap;
	}

	size = sbuf.st_size;
	if (write(ifd, ptr, size) != size) {
		fprintf(stderr, "%s: Write error on %s: %s\n",
			params->cmdname, params->imagefile, strerror(errno));
		goto err_munmap;
	}

	munmap((void *)ptr, sbuf.st_size);
	close(dfd);
	return pad_file(params, ifd, padded_size - size);

err_munmap:
	munmap((void *)ptr, sbuf.st_size);
err_close:
	close(dfd);
	return -1;
}

int rockchip_copy_image(int ifd, struct image_tool_params *params)
{
	int ret;

	ret = copy_file(params, ifd, spl_params.init_file,
			spl_params.init_size);
	if (ret)
		return ret;

	if (spl_params.boot_file) {
		ret = copy_file(params, ifd, spl_params.boot_file,
				spl_params.boot_size);
		if (ret)
			return ret;
	}

	return pad_file(params, ifd,
			params->file_size - params->orig_file_size);
}
