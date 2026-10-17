// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021 Philippe Reynes <philippe.reynes@softathome.com>
 */

#include <common.h>
#include <asm/global_data.h>
DECLARE_GLOBAL_DATA_PTR;
#include <image.h>
#include <mapmem.h>

#include <u-boot/sha256.h>

/*
 * Offset of the image
 *
 * This value is used to skip the header before really launching the image
 */
ulong image_load_offset;

/*
 * This function gathers information about the signature check
 * that could be done before launching the image.
 *
 * return:
 * < 0 => an error has occurred
 *   0 => OK
 *   1 => no setup
 */
static int image_pre_load_sig_setup(struct image_sig_info *info)
{
	const void *algo_name, *padding_name, *key, *mandatory;
	const u32 *sig_size;
	int key_len;
	int node, ret = 0;
	char *sig_info_path = NULL;

	if (!info) {
		log_err("ERROR: info is NULL for image pre-load sig check\n");
		ret = -EINVAL;
		goto out;
	}

	memset(info, 0, sizeof(*info));

	sig_info_path = env_get("pre_load_sig_info_path");
	if (!sig_info_path)
		sig_info_path = IMAGE_PRE_LOAD_PATH;

	node = fdt_path_offset(gd_fdt_blob(), sig_info_path);
	if (node < 0) {
		log_info("INFO: no info for image pre-load sig check\n");
		ret = 1;
		goto out;
	}

	algo_name = fdt_getprop(gd_fdt_blob(), node,
				IMAGE_PRE_LOAD_PROP_ALGO_NAME, NULL);
	if (!algo_name) {
		printf("ERROR: no algo_name for image pre-load sig check\n");
		ret = -EINVAL;
		goto out;
	}

	padding_name = fdt_getprop(gd_fdt_blob(), node,
				   IMAGE_PRE_LOAD_PROP_PADDING_NAME, NULL);
	if (!padding_name) {
		log_info("INFO: no padding_name provided, so using pkcs-1.5\n");
		padding_name = "pkcs-1.5";
	}

	sig_size = fdt_getprop(gd_fdt_blob(), node,
			       IMAGE_PRE_LOAD_PROP_SIG_SIZE, NULL);
	if (!sig_size) {
		log_err("ERROR: no signature-size for image pre-load sig check\n");
		ret = -EINVAL;
		goto out;
	}

	key = fdt_getprop(gd_fdt_blob(), node,
			  IMAGE_PRE_LOAD_PROP_PUBLIC_KEY, &key_len);
	if (!key) {
		log_err("ERROR: no key for image pre-load sig check\n");
		ret = -EINVAL;
		goto out;
	}

	info->algo_name		= (char *)algo_name;
	info->padding_name	= (char *)padding_name;
	info->key		= (uint8_t *)key;
	info->key_len		= key_len;
	info->sig_size		= fdt32_to_cpu(*sig_size);

	mandatory = fdt_getprop(gd_fdt_blob(), node,
				IMAGE_PRE_LOAD_PROP_MANDATORY, NULL);
	if (mandatory && !strcmp((char *)mandatory, "yes"))
		info->mandatory = 1;

	/* Compute signature information */
	info->sig_info.name     = info->algo_name;
	info->sig_info.padding  = image_get_padding_algo(info->padding_name);
	info->sig_info.checksum = image_get_checksum_algo(info->sig_info.name);
	info->sig_info.crypto   = image_get_crypto_algo(info->sig_info.name);
	info->sig_info.key      = info->key;
	info->sig_info.keylen   = info->key_len;

 out:
	return ret;
}

static int image_pre_load_sig_get_magic(ulong addr, u32 *magic)
{
	struct sig_header_s *sig_header;
	int ret = 0;

	sig_header = (struct sig_header_s *)map_sysmem(addr, SIG_HEADER_LEN);
	if (!sig_header) {
		log_err("ERROR: can't map first header\n");
		ret = -EFAULT;
		goto out;
	}

	*magic = fdt32_to_cpu(sig_header->magic);

	unmap_sysmem(sig_header);

 out:
	return ret;
}

static int image_pre_load_sig_get_header_size(ulong addr, u32 *header_size)
{
	struct sig_header_s *sig_header;
	int ret = 0;

	sig_header = (struct sig_header_s *)map_sysmem(addr, SIG_HEADER_LEN);
	if (!sig_header) {
		log_err("ERROR: can't map first header\n");
		ret = -EFAULT;
		goto out;
	}

	*header_size = fdt32_to_cpu(sig_header->header_size);

	unmap_sysmem(sig_header);

 out:
	return ret;
}

/*
 * return:
 * < 0 => no magic and magic mandatory (or error when reading magic)
 *   0 => magic found
 *   1 => magic NOT found
 */
static int image_pre_load_sig_check_magic(struct image_sig_info *info, ulong addr)
{
	u32 magic;
	int ret = 1;

	ret = image_pre_load_sig_get_magic(addr, &magic);
	if (ret < 0)
		goto out;

	if (magic != IMAGE_PRE_LOAD_SIG_MAGIC) {
		if (info->mandatory) {
			log_err("ERROR: signature is mandatory\n");
			ret = -EINVAL;
			goto out;
		}
		ret = 1;
		goto out;
	}

	ret = 0; /* magic found */

 out:
	return ret;
}

static int image_pre_load_sig_check_header_sig(struct image_sig_info *info, ulong addr)
{
	void *header;
	struct image_region reg;
	u32 sig_len;
	u8 *sig;
	int ret = 0;

	/* Only map header of the header and its signature */
	header = (void *)map_sysmem(addr, SIG_HEADER_LEN + info->sig_size);
	if (!header) {
		log_err("ERROR: can't map header\n");
		ret = -EFAULT;
		goto out;
	}

	reg.data = header;
	reg.size = SIG_HEADER_LEN;

	sig = (uint8_t *)header + SIG_HEADER_LEN;
	sig_len = info->sig_size;

	ret = info->sig_info.crypto->verify(&info->sig_info, &reg, 1, sig, sig_len);
	if (ret) {
		log_err("ERROR: header signature check has failed (err=%d)\n", ret);
		ret = -EINVAL;
		goto out_unmap;
	}

 out_unmap:
	unmap_sysmem(header);

 out:
	return ret;
}

static int image_pre_load_sig_check_img_sig_sha256(struct image_sig_info *info, ulong addr)
{
	struct sig_header_s *sig_header;
	u32 header_size, offset_img_sig;
	void *header;
	u8 sha256_img_sig[SHA256_SUM_LEN];
	int ret = 0;

	sig_header = (struct sig_header_s *)map_sysmem(addr, SIG_HEADER_LEN);
	if (!sig_header) {
		log_err("ERROR: can't map first header\n");
		ret = -EFAULT;
		goto out;
	}

	header_size = fdt32_to_cpu(sig_header->header_size);
	offset_img_sig = fdt32_to_cpu(sig_header->offset_img_sig);

	header = (void *)map_sysmem(addr, header_size);
	if (!header) {
		log_err("ERROR: can't map header\n");
		ret = -EFAULT;
		goto out_sig_header;
	}

	sha256_csum_wd(header + offset_img_sig, info->sig_size,
		       sha256_img_sig, CHUNKSZ_SHA256);

	ret = memcmp(sig_header->sha256_img_sig, sha256_img_sig, SHA256_SUM_LEN);
	if (ret) {
		log_err("ERROR: sha256 of image signature is invalid\n");
		ret = -EFAULT;
		goto out_header;
	}

 out_header:
	unmap_sysmem(header);
 out_sig_header:
	unmap_sysmem(sig_header);
 out:
	return ret;
}

static int image_pre_load_sig_check_img_sig(struct image_sig_info *info, ulong addr)
{
	struct sig_header_s *sig_header;
	u32 header_size, image_size, offset_img_sig;
	void *image;
	struct image_region reg;
	u32 sig_len;
	u8 *sig;
	int ret = 0;

	sig_header = (struct sig_header_s *)map_sysmem(addr, SIG_HEADER_LEN);
	if (!sig_header) {
		log_err("ERROR: can't map first header\n");
		ret = -EFAULT;
		goto out;
	}

	header_size = fdt32_to_cpu(sig_header->header_size);
	image_size = fdt32_to_cpu(sig_header->image_size);
	offset_img_sig = fdt32_to_cpu(sig_header->offset_img_sig);

	unmap_sysmem(sig_header);

	image = (void *)map_sysmem(addr, header_size + image_size);
	if (!image) {
		log_err("ERROR: can't map full image\n");
		ret = -EFAULT;
		goto out;
	}

	reg.data = image + header_size;
	reg.size = image_size;

	sig = (uint8_t *)image + offset_img_sig;
	sig_len = info->sig_size;

	ret = info->sig_info.crypto->verify(&info->sig_info, &reg, 1, sig, sig_len);
	if (ret) {
		log_err("ERROR: signature check has failed (err=%d)\n", ret);
		ret = -EINVAL;
		goto out_unmap_image;
	}

	log_info("INFO: signature check has succeed\n");

 out_unmap_image:
	unmap_sysmem(image);

 out:
	return ret;
}

int image_pre_load_sig(ulong addr)
{
	struct image_sig_info info;
	int ret;

	ret = image_pre_load_sig_setup(&info);
	if (ret < 0)
		goto out;
	if (ret > 0) {
		ret = 0;
		goto out;
	}

	ret = image_pre_load_sig_check_magic(&info, addr);
	if (ret < 0)
		goto out;
	if (ret > 0) {
		ret = 0;
		goto out;
	}

	/* Check the signature of the signature header */
	ret = image_pre_load_sig_check_header_sig(&info, addr);
	if (ret < 0)
		goto out;

	/* Check sha256 of the image signature */
	ret = image_pre_load_sig_check_img_sig_sha256(&info, addr);
	if (ret < 0)
		goto out;

	/* Check the image signature */
	ret = image_pre_load_sig_check_img_sig(&info, addr);
	if (!ret) {
		u32 header_size;

		ret = image_pre_load_sig_get_header_size(addr, &header_size);
		if (ret) {
			log_err("%s: can't get header size\n", __func__);
			ret = -EINVAL;
			goto out;
		}

		image_load_offset += header_size;
	}

 out:
	return ret;
}

int image_pre_load(ulong addr)
{
	int ret = 0;

	image_load_offset = 0;

	if (CONFIG_IS_ENABLED(IMAGE_PRE_LOAD_SIG))
		ret = image_pre_load_sig(addr);

	return ret;
}
