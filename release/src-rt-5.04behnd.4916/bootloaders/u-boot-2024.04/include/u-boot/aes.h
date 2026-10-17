/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2019, Softathome
 */

#ifndef _AES_H
#define _AES_H

#include <errno.h>
#include <image.h>

#if IMAGE_ENABLE_ENCRYPT
int image_aes_encrypt(struct image_cipher_info *info,
		      const unsigned char *data, int size,
		      unsigned char **cipher, int *cipher_len);
int image_aes_add_cipher_data(struct image_cipher_info *info, void *keydest,
			      void *fit, int node_noffset);
#else
int image_aes_encrypt(struct image_cipher_info *info,
		      const unsigned char *data, int size,
		      unsigned char **cipher, int *cipher_len)
{
	return -ENXIO;
}

int image_aes_add_cipher_data(struct image_cipher_info *info, void *keydest,
			      void *fit, int node_noffset)
{
	return -ENXIO;
}
#endif /* IMAGE_ENABLE_ENCRYPT */

#if IMAGE_ENABLE_DECRYPT
int image_aes_decrypt(struct image_cipher_info *info,
		      const void *cipher, size_t cipher_len,
		      void **data, size_t *size);
#else
int image_aes_decrypt(struct image_cipher_info *info,
		      const void *cipher, size_t cipher_len,
		      void **data, size_t *size)
{
	return -ENXIO;
}
#endif /* IMAGE_ENABLE_DECRYPT */

#endif
