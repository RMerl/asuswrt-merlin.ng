// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2019, Softathome
 */

#ifdef USE_HOSTCC
#include "mkimage.h"
#include <time.h>
#else
#include <common.h>
#include <malloc.h>
#include <asm/global_data.h>
DECLARE_GLOBAL_DATA_PTR;
#endif /* !USE_HOSdTCC*/
#include <image.h>
#include <uboot_aes.h>
#include <u-boot/aes.h>

struct cipher_algo cipher_algos[] = {
	{
		.name = "aes128",
		.key_len = AES128_KEY_LENGTH,
		.iv_len  = AES_BLOCK_LENGTH,
#if IMAGE_ENABLE_ENCRYPT
		.calculate_type = EVP_aes_128_cbc,
#endif
		.encrypt = image_aes_encrypt,
		.decrypt = image_aes_decrypt,
		.add_cipher_data = image_aes_add_cipher_data
	},
	{
		.name = "aes192",
		.key_len = AES192_KEY_LENGTH,
		.iv_len  = AES_BLOCK_LENGTH,
#if IMAGE_ENABLE_ENCRYPT
		.calculate_type = EVP_aes_192_cbc,
#endif
		.encrypt = image_aes_encrypt,
		.decrypt = image_aes_decrypt,
		.add_cipher_data = image_aes_add_cipher_data
	},
	{
		.name = "aes256",
		.key_len = AES256_KEY_LENGTH,
		.iv_len  = AES_BLOCK_LENGTH,
#if IMAGE_ENABLE_ENCRYPT
		.calculate_type = EVP_aes_256_cbc,
#endif
		.encrypt = image_aes_encrypt,
		.decrypt = image_aes_decrypt,
		.add_cipher_data = image_aes_add_cipher_data
	}
};

struct cipher_algo *image_get_cipher_algo(const char *full_name)
{
	int i;
	const char *name;

	for (i = 0; i < ARRAY_SIZE(cipher_algos); i++) {
		name = cipher_algos[i].name;
		if (!strncmp(name, full_name, strlen(name)))
			return &cipher_algos[i];
	}

	return NULL;
}

static int fit_image_setup_decrypt(struct image_cipher_info *info,
				   const void *fit, int image_noffset,
				   int cipher_noffset)
{
	const void *fdt = gd_fdt_blob();
	const char *node_name;
	char node_path[128];
	int noffset;
	char *algo_name;
	int ret;

	node_name = fit_get_name(fit, image_noffset, NULL);
	if (!node_name) {
		printf("Can't get node name\n");
		return -1;
	}

	if (fit_image_cipher_get_algo(fit, cipher_noffset, &algo_name)) {
		printf("Can't get algo name for cipher '%s' in image '%s'\n",
		       node_name, node_name);
		return -1;
	}

	info->keyname = fdt_getprop(fit, cipher_noffset, FIT_KEY_HINT, NULL);
	if (!info->keyname) {
		printf("Can't get key name\n");
		return -1;
	}

	info->iv = fdt_getprop(fit, cipher_noffset, "iv", NULL);
	info->ivname = fdt_getprop(fit, cipher_noffset, "iv-name-hint", NULL);

	if (!info->iv && !info->ivname) {
		printf("Can't get IV or IV name\n");
		return -1;
	}

	info->fit = fit;
	info->node_noffset = image_noffset;
	info->name = algo_name;
	info->cipher = image_get_cipher_algo(algo_name);
	if (!info->cipher) {
		printf("Can't get cipher\n");
		return -1;
	}

	ret = fit_image_get_data_size_unciphered(fit, image_noffset,
						 &info->size_unciphered);
	if (ret) {
		printf("Can't get size of unciphered data\n");
		return -1;
	}

	/*
	 * Search the cipher node in the u-boot fdt
	 * the path should be: /cipher/key-<algo>-<key>-<iv>
	 */
	if (info->ivname)
		snprintf(node_path, sizeof(node_path), "/%s/key-%s-%s-%s",
			 FIT_CIPHER_NODENAME, algo_name, info->keyname, info->ivname);
	else
		snprintf(node_path, sizeof(node_path), "/%s/key-%s-%s",
			 FIT_CIPHER_NODENAME, algo_name, info->keyname);

	noffset = fdt_path_offset(fdt, node_path);
	if (noffset < 0) {
		printf("Can't found cipher node offset\n");
		return -1;
	}

	/* read key */
	info->key = fdt_getprop(fdt, noffset, "key", NULL);
	if (!info->key) {
		printf("Can't get key in cipher node '%s'\n", node_path);
		return -1;
	}

	/* read iv */
	if (!info->iv) {
		info->iv = fdt_getprop(fdt, noffset, "iv", NULL);
		if (!info->iv) {
			printf("Can't get IV in cipher node '%s'\n", node_path);
			return -1;
		}
	}

	return 0;
}

int fit_image_decrypt_data(const void *fit,
			   int image_noffset, int cipher_noffset,
			   const void *data_ciphered, size_t size_ciphered,
			   void **data_unciphered, size_t *size_unciphered)
{
	struct image_cipher_info info;
	int ret;

	ret = fit_image_setup_decrypt(&info, fit, image_noffset,
				      cipher_noffset);
	if (ret < 0)
		goto out;

	ret = info.cipher->decrypt(&info, data_ciphered, size_ciphered,
				   data_unciphered, size_unciphered);

 out:
	return ret;
}
