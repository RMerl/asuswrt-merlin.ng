// SPDX-License-Identifier: GPL-2.0+
/*
 * Generate MediaTek BootROM header for SPL/U-Boot images
 *
 * Copyright (C) 2018 MediaTek Inc.
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#include <time.h>
#include <image.h>
#include <u-boot/crc.h>
#include <u-boot/sha256.h>
#include "imagetool.h"
#include "mtk_image.h"
#include "mtk_nand_headers.h"

static const struct brom_img_type {
	const char *name;
	enum brlyt_img_type type;
} brom_images[] = {
	{
		.name = "nand",
		.type = BRLYT_TYPE_NAND
	}, {
		.name = "emmc",
		.type = BRLYT_TYPE_EMMC
	}, {
		.name = "nor",
		.type = BRLYT_TYPE_NOR
	}, {
		.name = "sdmmc",
		.type = BRLYT_TYPE_SDMMC
	}, {
		.name = "snand",
		.type = BRLYT_TYPE_SNAND
	}, {
		.name = "spim-nand",
		.type = BRLYT_TYPE_SNAND
	}
};

/* Indicates whether we're generating or verifying */
static bool img_gen;
static uint32_t img_size;

/* Image type selected by user */
static enum brlyt_img_type hdr_media;
static uint32_t hdr_offset;
static int use_lk_hdr;
static int use_mt7621_hdr;
static bool is_arm64_image;

/* LK image name */
static char lk_name[32] = "U-Boot";

/* CRC32 normal table required by MT7621 image */
static uint32_t crc32tbl[256];

/* NAND header selected by user */
static const struct nand_header_type *hdr_nand;
static uint32_t hdr_nand_size;

/* GFH header + 2 * 4KB pages of NAND */
static char hdr_tmp[sizeof(struct gfh_header) + 0x2000];

static uint32_t crc32_normal_cal(uint32_t crc, const void *data, size_t length,
				 const uint32_t *crc32c_table)
{
	const uint8_t *p = data;

	while (length--)
		crc = crc32c_table[(uint8_t)((crc >> 24) ^ *p++)] ^ (crc << 8);

	return crc;
}

static void crc32_normal_init(uint32_t *crc32c_table, uint32_t poly)
{
	uint32_t v, i, j;

	for (i = 0; i < 256; i++) {
		v = i << 24;
		for (j = 0; j < 8; j++)
			v = (v << 1) ^ ((v & (1 << 31)) ? poly : 0);

		crc32c_table[i] = v;
	}
}

static int mtk_image_check_image_types(uint8_t type)
{
	if (type == IH_TYPE_MTKIMAGE)
		return EXIT_SUCCESS;
	else
		return EXIT_FAILURE;
}

static int mtk_brom_parse_imagename(const char *imagename)
{
#define is_blank_char(c) \
	((c) == '\t' || (c) == '\n' || (c) == '\r' || (c) == ' ')

	char *buf = strdup(imagename), *key, *val, *end, *next;
	int i;

	/* User passed arguments from image name */
	static const char *media = "";
	static const char *hdr_offs = "";
	static const char *nandinfo = "";
	static const char *lk = "";
	static const char *mt7621 = "";
	static const char *arm64_param = "";

	key = buf;
	while (key) {
		next = strchr(key, ';');
		if (next)
			*next = 0;

		val = strchr(key, '=');
		if (val) {
			*val++ = 0;

			/* Trim key */
			while (is_blank_char(*key))
				key++;

			end = key + strlen(key) - 1;
			while ((end >= key) && is_blank_char(*end))
				end--;
			end++;

			if (is_blank_char(*end))
				*end = 0;

			/* Trim value */
			while (is_blank_char(*val))
				val++;

			end = val + strlen(val) - 1;
			while ((end >= val) && is_blank_char(*end))
				end--;
			end++;

			if (is_blank_char(*end))
				*end = 0;

			/* record user passed arguments */
			if (!strcmp(key, "media"))
				media = val;

			if (!strcmp(key, "hdroffset"))
				hdr_offs = val;

			if (!strcmp(key, "nandinfo"))
				nandinfo = val;

			if (!strcmp(key, "lk"))
				lk = val;

			if (!strcmp(key, "mt7621"))
				mt7621 = val;

			if (!strcmp(key, "lkname"))
				snprintf(lk_name, sizeof(lk_name), "%s", val);

			if (!strcmp(key, "arm64"))
				arm64_param = val;
		}

		if (next)
			key = next + 1;
		else
			break;
	}

	/* if user specified LK image header, skip following checks */
	if (lk && lk[0] == '1') {
		use_lk_hdr = 1;
		free(buf);
		return 0;
	}

	/* if user specified MT7621 image header, skip following checks */
	if (mt7621 && mt7621[0] == '1') {
		use_mt7621_hdr = 1;
		free(buf);
		return 0;
	}

	/* parse media type */
	for (i = 0; i < ARRAY_SIZE(brom_images); i++) {
		if (!strcmp(brom_images[i].name, media)) {
			hdr_media = brom_images[i].type;
			break;
		}
	}

	/* parse nand header type */
	hdr_nand = mtk_nand_header_find(nandinfo);

	/* parse device header offset */
	if (hdr_offs && hdr_offs[0])
		hdr_offset = strtoul(hdr_offs, NULL, 0);

	if (arm64_param && arm64_param[0] == '1')
		is_arm64_image = true;

	free(buf);

	if (hdr_media == BRLYT_TYPE_INVALID) {
		fprintf(stderr, "Error: media type is invalid or missing.\n");
		fprintf(stderr, "       Please specify -n \"media=<type>\"\n");
		return -EINVAL;
	}

	if ((hdr_media == BRLYT_TYPE_NAND || hdr_media == BRLYT_TYPE_SNAND) &&
	    !hdr_nand) {
		fprintf(stderr, "Error: nand info is invalid or missing.\n");
		fprintf(stderr, "       Please specify -n \"media=%s;"
				"nandinfo=<info>\"\n", media);
		return -EINVAL;
	}

	if (hdr_media == BRLYT_TYPE_NAND || hdr_media == BRLYT_TYPE_SNAND)
		hdr_nand_size = mtk_nand_header_size(hdr_nand);

	return 0;
}

static int mtk_image_check_params(struct image_tool_params *params)
{
	if (!params->addr) {
		fprintf(stderr, "Error: Load Address must be set.\n");
		return -EINVAL;
	}

	if (!params->imagename) {
		fprintf(stderr, "Error: Image Name must be set.\n");
		return -EINVAL;
	}

	return mtk_brom_parse_imagename(params->imagename);
}

static int mtk_image_vrec_header(struct image_tool_params *params,
				 struct image_type_params *tparams)
{
	if (use_lk_hdr) {
		tparams->header_size = sizeof(union lk_hdr);
		tparams->hdr = &hdr_tmp;
		memset(&hdr_tmp, 0xff, tparams->header_size);
		return 0;
	}

	if (use_mt7621_hdr) {
		tparams->header_size = image_get_header_size();
		tparams->hdr = &hdr_tmp;
		memset(&hdr_tmp, 0, tparams->header_size);
		return 0;
	}

	if (hdr_media == BRLYT_TYPE_NAND || hdr_media == BRLYT_TYPE_SNAND)
		tparams->header_size = hdr_nand_size;
	else
		tparams->header_size = sizeof(struct gen_device_header);

	tparams->header_size += sizeof(struct gfh_header);
	tparams->hdr = &hdr_tmp;

	memset(&hdr_tmp, 0xff, tparams->header_size);

	return SHA256_SUM_LEN;
}

static int mtk_image_verify_gfh(struct gfh_header *gfh, uint32_t type, int print)
{
	if (strcmp(gfh->file_info.name, GFH_FILE_INFO_NAME))
		return -1;

	if (le32_to_cpu(gfh->file_info.flash_type) != type)
		return -1;

	if (print)
		printf("Load Address: %08x\n",
		       le32_to_cpu(gfh->file_info.load_addr) +
		       le32_to_cpu(gfh->file_info.jump_offset));

	if (print)
		printf("Architecture: %s\n", is_arm64_image ? "ARM64" : "ARM");

	return 0;
}

static int mtk_image_verify_gen_header(const uint8_t *ptr, int print)
{
	union gen_boot_header *gbh = (union gen_boot_header *)ptr;
	uint32_t gfh_offset, total_size, devh_size;
	struct brom_layout_header *bh;
	struct gfh_header *gfh;
	const char *bootmedia;

	if (!strcmp(gbh->name, SF_BOOT_NAME))
		bootmedia = "Serial NOR";
	else if (!strcmp(gbh->name, EMMC_BOOT_NAME))
		bootmedia = "eMMC";
	else if (!strcmp(gbh->name, SDMMC_BOOT_NAME))
		bootmedia = "SD/MMC";
	else
		return -1;

	if (print)
		printf("Boot Media:   %s\n", bootmedia);

	if (le32_to_cpu(gbh->version) != 1 ||
	    le32_to_cpu(gbh->size) != sizeof(union gen_boot_header))
		return -1;

	bh = (struct brom_layout_header *)(ptr + le32_to_cpu(gbh->size));

	if (strcmp(bh->name, BRLYT_NAME))
		return -1;

	if (le32_to_cpu(bh->magic) != BRLYT_MAGIC ||
	    (le32_to_cpu(bh->type) != BRLYT_TYPE_NOR &&
	    le32_to_cpu(bh->type) != BRLYT_TYPE_EMMC &&
	    le32_to_cpu(bh->type) != BRLYT_TYPE_SDMMC))
		return -1;

	devh_size = sizeof(struct gen_device_header);

	if (img_gen) {
		gfh_offset = devh_size;
	} else {
		gfh_offset = le32_to_cpu(bh->header_size);

		if (gfh_offset + sizeof(struct gfh_header) > img_size) {
			/*
			 * This may happen if the hdr_offset used to generate
			 * this image is not zero.
			 * Since device header size is not fixed, we can't
			 * cover all possible cases.
			 * Assuming the image is valid only if the real
			 * device header size equals to devh_size.
			 */
			total_size = le32_to_cpu(bh->total_size);

			if (total_size - gfh_offset > img_size - devh_size)
				return -1;

			gfh_offset = devh_size;
		}
	}

	gfh = (struct gfh_header *)(ptr + gfh_offset);

	return mtk_image_verify_gfh(gfh, GFH_FLASH_TYPE_GEN, print);
}

static int mtk_image_verify_nand_header(const uint8_t *ptr, int print)
{
	struct brom_layout_header *bh;
	struct nand_header_info info;
	struct gfh_header *gfh;
	const char *bootmedia;
	int ret;

	ret = mtk_nand_header_info(ptr, &info);
	if (ret < 0)
		return ret;

	if (!ret) {
		bh = (struct brom_layout_header *)(ptr + info.page_size);

		if (strcmp(bh->name, BRLYT_NAME))
			return -1;

		if (le32_to_cpu(bh->magic) != BRLYT_MAGIC)
			return -1;

		if (le32_to_cpu(bh->type) == BRLYT_TYPE_NAND)
			bootmedia = "Parallel NAND";
		else if (le32_to_cpu(bh->type) == BRLYT_TYPE_SNAND)
			bootmedia = "Serial NAND (SNFI/AP)";
		else
			return -1;
	} else {
		if (info.snfi)
			bootmedia = "Serial NAND (SNFI/HSM)";
		else
			bootmedia = "Serial NAND (SPIM)";
	}

	if (print) {
		printf("Boot Media:   %s\n", bootmedia);

		if (info.page_size >= 1024)
			printf("Page Size:    %dKB\n", info.page_size >> 10);
		else
			printf("Page Size:    %dB\n", info.page_size);

		printf("Spare Size:   %dB\n", info.spare_size);
	}

	gfh = (struct gfh_header *)(ptr + info.gfh_offset);

	return mtk_image_verify_gfh(gfh, GFH_FLASH_TYPE_NAND, print);
}

static uint32_t crc32be_cal(const void *data, size_t length)
{
	uint32_t crc = 0;
	uint8_t c;

	if (crc32tbl[1] != MT7621_IH_CRC_POLYNOMIAL)
		crc32_normal_init(crc32tbl, MT7621_IH_CRC_POLYNOMIAL);

	crc = crc32_normal_cal(crc, data, length, crc32tbl);

	for (; length; length >>= 8) {
		c = length & 0xff;
		crc = crc32_normal_cal(crc, &c, 1, crc32tbl);
	}

	return ~crc;
}

static int mtk_image_verify_mt7621_header(const uint8_t *ptr, int print)
{
	const struct legacy_img_hdr *hdr = (const struct legacy_img_hdr *)ptr;
	struct mt7621_nand_header *nhdr;
	uint32_t spl_size, crcval;
	struct legacy_img_hdr header;
	int ret;

	spl_size = image_get_size(hdr);

	if (spl_size > img_size) {
		if (print)
			printf("Incomplete SPL image\n");
		return -1;
	}

	ret = image_check_hcrc(hdr);
	if (!ret) {
		if (print)
			printf("Bad header CRC\n");
		return -1;
	}

	ret = image_check_dcrc(hdr);
	if (!ret) {
		if (print)
			printf("Bad data CRC\n");
		return -1;
	}

	/* Copy header so we can blank CRC field for re-calculation */
	memmove(&header, hdr, image_get_header_size());
	image_set_hcrc(&header, 0);

	nhdr = (struct mt7621_nand_header *)header.ih_name;
	crcval = be32_to_cpu(nhdr->crc);
	nhdr->crc = 0;

	if (crcval != crc32be_cal(&header, image_get_header_size())) {
		if (print)
			printf("Bad NAND header CRC\n");
		return -1;
	}

	if (print) {
		printf("Load Address: %08x\n", image_get_load(hdr));

		printf("Image Name:   %.*s\n", MT7621_IH_NMLEN,
		       image_get_name(hdr));

		if (IMAGE_ENABLE_TIMESTAMP) {
			printf("Created:      ");
			genimg_print_time((time_t)image_get_time(hdr));
		}

		printf("Data Size:    ");
		genimg_print_size(image_get_data_size(hdr));
	}

	return 0;
}

static int mtk_image_verify_header(unsigned char *ptr, int image_size,
				   struct image_tool_params *params)
{
	struct legacy_img_hdr *hdr = (struct legacy_img_hdr *)ptr;
	union lk_hdr *lk = (union lk_hdr *)ptr;

	/* nothing to verify for LK image header */
	if (le32_to_cpu(lk->magic) == LK_PART_MAGIC)
		return 0;

	img_size = image_size;

	if (image_get_magic(hdr) == IH_MAGIC)
		return mtk_image_verify_mt7621_header(ptr, 0);

	if (is_mtk_nand_header(ptr))
		return mtk_image_verify_nand_header(ptr, 0);
	else
		return mtk_image_verify_gen_header(ptr, 0);

	return -1;
}

static void mtk_image_print_header(const void *ptr, struct image_tool_params *params)
{
	struct legacy_img_hdr *hdr = (struct legacy_img_hdr *)ptr;
	union lk_hdr *lk = (union lk_hdr *)ptr;

	if (le32_to_cpu(lk->magic) == LK_PART_MAGIC) {
		printf("Image Type:   MediaTek LK Image\n");
		printf("Load Address: %08x\n", le32_to_cpu(lk->loadaddr));
		return;
	}

	printf("Image Type:   MediaTek BootROM Loadable Image\n");

	if (image_get_magic(hdr) == IH_MAGIC) {
		mtk_image_verify_mt7621_header(ptr, 1);
		return;
	}

	if (is_mtk_nand_header(ptr))
		mtk_image_verify_nand_header(ptr, 1);
	else
		mtk_image_verify_gen_header(ptr, 1);
}

static void put_brom_layout_header(struct brom_layout_header *hdr, int type)
{
	strncpy(hdr->name, BRLYT_NAME, sizeof(hdr->name));
	hdr->version = cpu_to_le32(1);
	hdr->magic = cpu_to_le32(BRLYT_MAGIC);
	hdr->type = cpu_to_le32(type);
}

static void put_ghf_common_header(struct gfh_common_header *gfh, uint16_t size,
				  uint16_t type, uint8_t ver)
{
	uint32_t magic_version = GFH_HEADER_MAGIC |
				 (uint32_t)ver << GFH_HEADER_VERSION_SHIFT;

	gfh->magic_version = cpu_to_le32(magic_version);
	gfh->size = cpu_to_le16(size);
	gfh->type = cpu_to_le16(type);
}

static void put_ghf_header(struct gfh_header *gfh, int file_size,
			   int dev_hdr_size, int load_addr, int flash_type)
{
	uint32_t cfg_bits;

	memset(gfh, 0, sizeof(struct gfh_header));

	/* GFH_FILE_INFO header */
	put_ghf_common_header(&gfh->file_info.gfh, sizeof(gfh->file_info),
			      GFH_TYPE_FILE_INFO, 1);
	strncpy(gfh->file_info.name, GFH_FILE_INFO_NAME,
		sizeof(gfh->file_info.name));
	gfh->file_info.unused = cpu_to_le32(1);
	gfh->file_info.file_type = cpu_to_le16(1);
	gfh->file_info.flash_type = flash_type;
	gfh->file_info.sig_type = GFH_SIG_TYPE_SHA256;
	gfh->file_info.load_addr = cpu_to_le32(load_addr - sizeof(*gfh));
	gfh->file_info.total_size = cpu_to_le32(file_size - dev_hdr_size);
	gfh->file_info.max_size = cpu_to_le32(file_size);
	gfh->file_info.hdr_size = sizeof(*gfh);
	gfh->file_info.sig_size = SHA256_SUM_LEN;
	gfh->file_info.jump_offset = sizeof(*gfh);
	gfh->file_info.processed = cpu_to_le32(1);

	/* GFH_BL_INFO header */
	put_ghf_common_header(&gfh->bl_info.gfh, sizeof(gfh->bl_info),
			      GFH_TYPE_BL_INFO, 1);
	gfh->bl_info.attr = cpu_to_le32(1);

	/* GFH_BROM_CFG header */
	put_ghf_common_header(&gfh->brom_cfg.gfh, sizeof(gfh->brom_cfg),
			      GFH_TYPE_BROM_CFG, 3);
	cfg_bits = GFH_BROM_CFG_USBDL_AUTO_DETECT_DIS |
		   GFH_BROM_CFG_USBDL_BY_KCOL0_TIMEOUT_EN |
		   GFH_BROM_CFG_USBDL_BY_FLAG_TIMEOUT_EN;
	gfh->brom_cfg.usbdl_by_kcol0_timeout_ms = cpu_to_le32(5000);
	if (is_arm64_image) {
		gfh->brom_cfg.jump_bl_arm64 = GFH_BROM_CFG_JUMP_BL_ARM64;
		cfg_bits |= GFH_BROM_CFG_JUMP_BL_ARM64_EN;
	}
	gfh->brom_cfg.cfg_bits = cpu_to_le32(cfg_bits);

	/* GFH_BL_SEC_KEY header */
	put_ghf_common_header(&gfh->bl_sec_key.gfh, sizeof(gfh->bl_sec_key),
			      GFH_TYPE_BL_SEC_KEY, 1);

	/* GFH_ANTI_CLONE header */
	put_ghf_common_header(&gfh->anti_clone.gfh, sizeof(gfh->anti_clone),
			      GFH_TYPE_ANTI_CLONE, 1);
	gfh->anti_clone.ac_offset = cpu_to_le32(0x10);
	gfh->anti_clone.ac_len = cpu_to_le32(0x80);

	/* GFH_BROM_SEC_CFG header */
	put_ghf_common_header(&gfh->brom_sec_cfg.gfh,
			      sizeof(gfh->brom_sec_cfg),
			      GFH_TYPE_BROM_SEC_CFG, 1);
	gfh->brom_sec_cfg.cfg_bits =
		cpu_to_le32(BROM_SEC_CFG_JTAG_EN | BROM_SEC_CFG_UART_EN);
}

static void put_hash(uint8_t *buff, int size)
{
	sha256_context ctx;

	sha256_starts(&ctx);
	sha256_update(&ctx, buff, size);
	sha256_finish(&ctx, buff + size);
}

static void mtk_image_set_gen_header(void *ptr, off_t filesize,
				     uint32_t loadaddr)
{
	struct gen_device_header *hdr = (struct gen_device_header *)ptr;
	struct gfh_header *gfh;
	const char *bootname = NULL;

	if (hdr_media == BRLYT_TYPE_NOR)
		bootname = SF_BOOT_NAME;
	else if (hdr_media == BRLYT_TYPE_EMMC)
		bootname = EMMC_BOOT_NAME;
	else if (hdr_media == BRLYT_TYPE_SDMMC)
		bootname = SDMMC_BOOT_NAME;

	/* Generic device header */
	snprintf(hdr->boot.name, sizeof(hdr->boot.name), "%s", bootname);
	hdr->boot.version = cpu_to_le32(1);
	hdr->boot.size = cpu_to_le32(sizeof(hdr->boot));

	/* BRLYT header */
	put_brom_layout_header(&hdr->brlyt, hdr_media);
	hdr->brlyt.header_size = cpu_to_le32(hdr_offset + sizeof(*hdr));
	hdr->brlyt.total_size = cpu_to_le32(hdr_offset + filesize);
	hdr->brlyt.header_size_2 = hdr->brlyt.header_size;
	hdr->brlyt.total_size_2 = hdr->brlyt.total_size;

	/* GFH header */
	gfh = (struct gfh_header *)(ptr + sizeof(struct gen_device_header));
	put_ghf_header(gfh, filesize, sizeof(struct gen_device_header),
		       loadaddr, GFH_FLASH_TYPE_GEN);

	/* Generate SHA256 hash */
	put_hash((uint8_t *)gfh,
		 filesize - sizeof(struct gen_device_header) - SHA256_SUM_LEN);
}

static void mtk_image_set_nand_header(void *ptr, off_t filesize,
				      uint32_t loadaddr)
{
	struct brom_layout_header *brlyt;
	struct gfh_header *gfh;
	uint32_t payload_pages, nand_page_size;

	/* NAND header */
	nand_page_size = mtk_nand_header_put(hdr_nand, ptr);

	if (nand_page_size) {
		/* BRLYT header */
		payload_pages = (filesize + nand_page_size - 1) /
				nand_page_size;
		brlyt = (struct brom_layout_header *)(ptr + nand_page_size);
		put_brom_layout_header(brlyt, hdr_media);
		brlyt->header_size = cpu_to_le32(2);
		brlyt->total_size = cpu_to_le32(payload_pages);
		brlyt->header_size_2 = brlyt->header_size;
		brlyt->total_size_2 = brlyt->total_size;
		brlyt->unused = cpu_to_le32(1);
	}

	/* GFH header */
	gfh = (struct gfh_header *)(ptr + hdr_nand_size);
	put_ghf_header(gfh, filesize, hdr_nand_size, loadaddr,
		       GFH_FLASH_TYPE_NAND);

	/* Generate SHA256 hash */
	put_hash((uint8_t *)gfh, filesize - hdr_nand_size - SHA256_SUM_LEN);
}

static void mtk_image_set_mt7621_header(void *ptr, off_t filesize,
					uint32_t loadaddr)
{
	struct legacy_img_hdr *hdr = (struct legacy_img_hdr *)ptr;
	struct mt7621_stage1_header *shdr;
	struct mt7621_nand_header *nhdr;
	uint32_t datasize, crcval;

	datasize = filesize - image_get_header_size();
	nhdr = (struct mt7621_nand_header *)hdr->ih_name;
	shdr = (struct mt7621_stage1_header *)(ptr + image_get_header_size());

	shdr->ep = cpu_to_be32(loadaddr);
	shdr->stage_size = cpu_to_be32(datasize);

	image_set_magic(hdr, IH_MAGIC);
	image_set_time(hdr, time(NULL));
	image_set_size(hdr, datasize);
	image_set_load(hdr, loadaddr);
	image_set_ep(hdr, loadaddr);
	image_set_os(hdr, IH_OS_U_BOOT);
	image_set_arch(hdr, IH_ARCH_MIPS);
	image_set_type(hdr, IH_TYPE_STANDALONE);
	image_set_comp(hdr, IH_COMP_NONE);

	crcval = crc32(0, (uint8_t *)shdr, datasize);
	image_set_dcrc(hdr, crcval);

	strncpy(nhdr->ih_name, "MT7621 NAND", MT7621_IH_NMLEN);

	nhdr->ih_stage_offset = cpu_to_be32(image_get_header_size());

	crcval = crc32be_cal(hdr, image_get_header_size());
	nhdr->crc = cpu_to_be32(crcval);

	crcval = crc32(0, (uint8_t *)hdr, image_get_header_size());
	image_set_hcrc(hdr, crcval);
}

static void mtk_image_set_header(void *ptr, struct stat *sbuf, int ifd,
				 struct image_tool_params *params)
{
	union lk_hdr *lk = (union lk_hdr *)ptr;

	if (use_lk_hdr) {
		lk->magic = cpu_to_le32(LK_PART_MAGIC);
		lk->size = cpu_to_le32(sbuf->st_size - sizeof(union lk_hdr));
		lk->loadaddr = cpu_to_le32(params->addr);
		lk->mode = 0xffffffff; /* must be non-zero */
		memset(lk->name, 0, sizeof(lk->name));
		strncpy(lk->name, lk_name, sizeof(lk->name));
		return;
	}

	img_gen = true;
	img_size = sbuf->st_size;

	if (use_mt7621_hdr) {
		mtk_image_set_mt7621_header(ptr, sbuf->st_size, params->addr);
		return;
	}

	if (hdr_media == BRLYT_TYPE_NAND || hdr_media == BRLYT_TYPE_SNAND)
		mtk_image_set_nand_header(ptr, sbuf->st_size, params->addr);
	else
		mtk_image_set_gen_header(ptr, sbuf->st_size, params->addr);
}

U_BOOT_IMAGE_TYPE(
	mtk_image,
	"MediaTek BootROM Loadable Image support",
	0,
	NULL,
	mtk_image_check_params,
	mtk_image_verify_header,
	mtk_image_print_header,
	mtk_image_set_header,
	NULL,
	mtk_image_check_image_types,
	NULL,
	mtk_image_vrec_header
);
