// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2019,Softathome
 */

#define OPENSSL_API_COMPAT 0x10101000L

#include "mkimage.h"
#include <stdio.h>
#include <string.h>
#include <image.h>
#include <time.h>
#include <openssl/bn.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/evp.h>
#include <openssl/engine.h>
#include <uboot_aes.h>

#if OPENSSL_VERSION_NUMBER >= 0x10000000L
#define HAVE_ERR_REMOVE_THREAD_STATE
#endif

int image_aes_encrypt(struct image_cipher_info *info,
		      unsigned char *data, int size,
		      unsigned char **cipher, int *cipher_len)
{
	EVP_CIPHER_CTX *ctx;
	unsigned char *buf = NULL;
	int buf_len, len, ret = 0;

	/* create and initialise the context */
	ctx = EVP_CIPHER_CTX_new();
	if (!ctx) {
		printf("Can't create context\n");
		return -1;
	}

	/* allocate a buffer for the result */
	buf = malloc(size + AES_BLOCK_LENGTH);
	if (!buf) {
		printf("Can't allocate memory to encrypt\n");
		ret = -1;
		goto out;
	}

	if (EVP_EncryptInit_ex(ctx, info->cipher->calculate_type(),
			       NULL, info->key, info->iv) != 1) {
		printf("Can't init encryption\n");
		ret = -1;
		goto out;
	}

	if (EVP_EncryptUpdate(ctx, buf, &len, data, size) != 1) {
		printf("Can't encrypt data\n");
		ret = -1;
		goto out;
	}

	buf_len = len;

	if (EVP_EncryptFinal_ex(ctx, buf + len, &len) != 1) {
		printf("Can't finalise the encryption\n");
		ret = -1;
		goto out;
	}

	buf_len += len;

	*cipher = buf;
	*cipher_len = buf_len;

 out:
	EVP_CIPHER_CTX_free(ctx);
	return ret;
}

int image_aes_add_cipher_data(struct image_cipher_info *info, void *keydest,
			      void *fit, int node_noffset)
{
	int parent, node;
	char name[128];
	int ret = 0;

	/* Either create or overwrite the named cipher node */
	parent = fdt_subnode_offset(keydest, 0, FIT_CIPHER_NODENAME);
	if (parent == -FDT_ERR_NOTFOUND) {
		parent = fdt_add_subnode(keydest, 0, FIT_CIPHER_NODENAME);
		if (parent < 0) {
			ret = parent;
			if (ret != -FDT_ERR_NOSPACE) {
				fprintf(stderr,
					"Couldn't create cipher node: %s\n",
					fdt_strerror(parent));
			}
		}
	}
	if (ret)
		goto done;

	/* Either create or overwrite the named key node */
	if (info->ivname)
		snprintf(name, sizeof(name), "key-%s-%s-%s",
			 info->name, info->keyname, info->ivname);
	else
		snprintf(name, sizeof(name), "key-%s-%s",
			 info->name, info->keyname);

	node = fdt_subnode_offset(keydest, parent, name);
	if (node == -FDT_ERR_NOTFOUND) {
		node = fdt_add_subnode(keydest, parent, name);
		if (node < 0) {
			ret = node;
			if (ret != -FDT_ERR_NOSPACE) {
				fprintf(stderr,
					"Could not create key subnode: %s\n",
					fdt_strerror(node));
			}
		}
	} else if (node < 0) {
		fprintf(stderr, "Cannot select keys parent: %s\n",
			fdt_strerror(node));
		ret = node;
	}

	if (ret)
		goto done;

	if (info->ivname)
		/* Store the IV in the u-boot device tree */
		ret = fdt_setprop(keydest, node, "iv",
				  info->iv, info->cipher->iv_len);
	else
		/* Store the IV in the FIT image */
		ret = fdt_setprop(fit, node_noffset, "iv",
				  info->iv, info->cipher->iv_len);

	if (!ret)
		ret = fdt_setprop(keydest, node, "key",
				  info->key, info->cipher->key_len);

	if (!ret)
		ret = fdt_setprop_u32(keydest, node, "key-len",
				      info->cipher->key_len);

done:
	if (ret)
		ret = ret == -FDT_ERR_NOSPACE ? -ENOSPC : -EIO;

	return ret;
}
