// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2019, softathome
 */

#ifndef USE_HOSTCC
#include <malloc.h>
#endif
#include <image.h>
#include <uboot_aes.h>

int image_aes_decrypt(struct image_cipher_info *info,
		      const void *cipher, size_t cipher_len,
		      void **data, size_t *size)
{
#ifndef USE_HOSTCC
	unsigned char key_exp[AES256_EXPAND_KEY_LENGTH];
	unsigned int aes_blocks, key_len = info->cipher->key_len;

	*data = (void*)cipher;
	*size = info->size_unciphered;

	memcpy(&key_exp[0], info->key, key_len);

	/* First we expand the key. */
	aes_expand_key((u8 *)info->key, key_len, key_exp);

	/* Calculate the number of AES blocks to encrypt. */
	aes_blocks = DIV_ROUND_UP(cipher_len, AES_BLOCK_LENGTH);

	aes_cbc_decrypt_blocks(key_len, key_exp, (u8 *)info->iv,
			       (u8 *)cipher, *data, aes_blocks);
#endif

	return 0;
}
