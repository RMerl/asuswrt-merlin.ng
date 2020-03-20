/*
 * Driver for /dev/crypto device (aka CryptoDev)
 *
 * Copyright (c) 2010,2011 Nikos Mavrogiannopoulos <nmav@gnutls.org>
 * Portions Copyright (c) 2010 Michael Weiser
 * Portions Copyright (c) 2010 Phil Sutter
 *
 * This file is part of linux cryptodev.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <linux/crypto.h>
#include <linux/mm.h>
#include <linux/highmem.h>
#include <linux/ioctl.h>
#include <linux/random.h>
#include <linux/scatterlist.h>
#include <linux/uaccess.h>
#include <crypto/algapi.h>
#include <crypto/hash.h>
#include <crypto/cryptodev.h>
#include <crypto/aead.h>
#include <linux/rtnetlink.h>
#include <crypto/authenc.h>
#include "cryptodev_int.h"


struct cryptodev_result {
	struct completion completion;
	int err;
};

static void cryptodev_complete(struct crypto_async_request *req, int err)
{
	struct cryptodev_result *res = req->data;

	if (err == -EINPROGRESS)
		return;

	res->err = err;
	complete(&res->completion);
}

int cryptodev_get_cipher_keylen(unsigned int *keylen, struct session_op *sop,
		int aead)
{
	/*
	 * For blockciphers (AES-CBC) or non-composite aead ciphers (like AES-GCM),
	 * the key length is simply the cipher keylen obtained from userspace. If
	 * the cipher is composite aead, the keylen is the sum of cipher keylen,
	 * hmac keylen and a key header length. This key format is the one used in
	 * Linux kernel for composite aead ciphers (crypto/authenc.c)
	 */
	unsigned int klen = sop->keylen;

	if (unlikely(sop->keylen > CRYPTO_CIPHER_MAX_KEY_LEN))
		return -EINVAL;

	if (aead && sop->mackeylen) {
		if (unlikely(sop->mackeylen > CRYPTO_HMAC_MAX_KEY_LEN))
			return -EINVAL;
		klen += sop->mackeylen;
		klen += RTA_SPACE(sizeof(struct crypto_authenc_key_param));
	}

	*keylen = klen;
	return 0;
}

int cryptodev_get_cipher_key(uint8_t *key, struct session_op *sop, int aead)
{
	/*
	 * Get cipher key from user-space. For blockciphers just copy it from
	 * user-space. For composite aead ciphers combine it with the hmac key in
	 * the format used by Linux kernel in crypto/authenc.c:
	 *
	 * [[AUTHENC_KEY_HEADER + CIPHER_KEYLEN] [AUTHENTICATION KEY] [CIPHER KEY]]
	 */
	struct crypto_authenc_key_param *param;
	struct rtattr *rta;
	int ret = 0;

	if (aead && sop->mackeylen) {
		/*
		 * Composite aead ciphers. The first four bytes are the header type and
		 * header length for aead keys
		 */
		rta = (void *)key;
		rta->rta_type = CRYPTO_AUTHENC_KEYA_PARAM;
		rta->rta_len = RTA_LENGTH(sizeof(*param));

		/*
		 * The next four bytes hold the length of the encryption key
		 */
		param = RTA_DATA(rta);
		param->enckeylen = cpu_to_be32(sop->keylen);

		/* Advance key pointer eight bytes and copy the hmac key */
		key += RTA_SPACE(sizeof(*param));
		if (unlikely(copy_from_user(key, sop->mackey, sop->mackeylen))) {
			ret = -EFAULT;
			goto error;
		}
		/* Advance key pointer past the hmac key */
		key += sop->mackeylen;
	}
	/* now copy the blockcipher key */
	if (unlikely(copy_from_user(key, sop->key, sop->keylen)))
		ret = -EFAULT;

error:
	return ret;
}


int cryptodev_cipher_init(struct cipher_data *out, const char *alg_name,
				uint8_t *keyp, size_t keylen, int stream, int aead)
{
	int ret;

	if (aead == 0) {
		struct ablkcipher_alg *alg;

		out->async.s = crypto_alloc_ablkcipher(alg_name, 0, 0);
		if (unlikely(IS_ERR(out->async.s))) {
			ddebug(1, "Failed to load cipher %s", alg_name);
				return -EINVAL;
		}

		alg = crypto_ablkcipher_alg(out->async.s);
		if (alg != NULL) {
			/* Was correct key length supplied? */
			if (alg->max_keysize > 0 &&
					unlikely((keylen < alg->min_keysize) ||
					(keylen > alg->max_keysize))) {
				ddebug(1, "Wrong keylen '%zu' for algorithm '%s'. Use %u to %u.",
						keylen, alg_name, alg->min_keysize, alg->max_keysize);
				ret = -EINVAL;
				goto error;
			}
		}

		out->blocksize = crypto_ablkcipher_blocksize(out->async.s);
		out->ivsize = crypto_ablkcipher_ivsize(out->async.s);
		out->alignmask = crypto_ablkcipher_alignmask(out->async.s);

		ret = crypto_ablkcipher_setkey(out->async.s, keyp, keylen);
	} else {
		out->async.as = crypto_alloc_aead(alg_name, 0, 0);
		if (unlikely(IS_ERR(out->async.as))) {
			ddebug(1, "Failed to load cipher %s", alg_name);
			return -EINVAL;
		}

		out->blocksize = crypto_aead_blocksize(out->async.as);
		out->ivsize = crypto_aead_ivsize(out->async.as);
		out->alignmask = crypto_aead_alignmask(out->async.as);

		ret = crypto_aead_setkey(out->async.as, keyp, keylen);
	}

	if (unlikely(ret)) {
		ddebug(1, "Setting key failed for %s-%zu.", alg_name, keylen*8);
		ret = -EINVAL;
		goto error;
	}

	out->stream = stream;
	out->aead = aead;

	out->async.result = kzalloc(sizeof(*out->async.result), GFP_KERNEL);
	if (unlikely(!out->async.result)) {
		ret = -ENOMEM;
		goto error;
	}

	init_completion(&out->async.result->completion);

	if (aead == 0) {
		out->async.request = ablkcipher_request_alloc(out->async.s, GFP_KERNEL);
		if (unlikely(!out->async.request)) {
			derr(1, "error allocating async crypto request");
			ret = -ENOMEM;
			goto error;
		}

		ablkcipher_request_set_callback(out->async.request,
					CRYPTO_TFM_REQ_MAY_BACKLOG,
					cryptodev_complete, out->async.result);
	} else {
		out->async.arequest = aead_request_alloc(out->async.as, GFP_KERNEL);
		if (unlikely(!out->async.arequest)) {
			derr(1, "error allocating async crypto request");
			ret = -ENOMEM;
			goto error;
		}

		aead_request_set_callback(out->async.arequest,
					CRYPTO_TFM_REQ_MAY_BACKLOG,
					cryptodev_complete, out->async.result);
	}

	out->init = 1;
	return 0;
error:
	if (aead == 0) {
		if (out->async.request)
			ablkcipher_request_free(out->async.request);
		if (out->async.s)
			crypto_free_ablkcipher(out->async.s);
	} else {
		if (out->async.arequest)
			aead_request_free(out->async.arequest);
		if (out->async.as)
			crypto_free_aead(out->async.as);
	}
	kfree(out->async.result);

	return ret;
}

void cryptodev_cipher_deinit(struct cipher_data *cdata)
{
	if (cdata->init) {
		if (cdata->aead == 0) {
			if (cdata->async.request)
				ablkcipher_request_free(cdata->async.request);
			if (cdata->async.s)
				crypto_free_ablkcipher(cdata->async.s);
		} else {
			if (cdata->async.arequest)
				aead_request_free(cdata->async.arequest);
			if (cdata->async.as)
				crypto_free_aead(cdata->async.as);
		}

		kfree(cdata->async.result);
		cdata->init = 0;
	}
}

static inline int waitfor(struct cryptodev_result *cr, ssize_t ret)
{
	switch (ret) {
	case 0:
		break;
	case -EINPROGRESS:
	case -EBUSY:
		wait_for_completion(&cr->completion);
		/* At this point we known for sure the request has finished,
		 * because wait_for_completion above was not interruptible.
		 * This is important because otherwise hardware or driver
		 * might try to access memory which will be freed or reused for
		 * another request. */

		if (unlikely(cr->err)) {
			derr(0, "error from async request: %d", cr->err);
			return cr->err;
		}

		break;
	default:
		return ret;
	}

	return 0;
}

ssize_t cryptodev_cipher_encrypt(struct cipher_data *cdata,
		const struct scatterlist *src, struct scatterlist *dst,
		size_t len)
{
	int ret;

	reinit_completion(&cdata->async.result->completion);

	if (cdata->aead == 0) {
		ablkcipher_request_set_crypt(cdata->async.request,
			(struct scatterlist *)src, dst,
			len, cdata->async.iv);
		ret = crypto_ablkcipher_encrypt(cdata->async.request);
	} else {
		aead_request_set_crypt(cdata->async.arequest,
			(struct scatterlist *)src, dst,
			len, cdata->async.iv);
		ret = crypto_aead_encrypt(cdata->async.arequest);
	}

	return waitfor(cdata->async.result, ret);
}

ssize_t cryptodev_cipher_decrypt(struct cipher_data *cdata,
		const struct scatterlist *src, struct scatterlist *dst,
		size_t len)
{
	int ret;

	reinit_completion(&cdata->async.result->completion);
	if (cdata->aead == 0) {
		ablkcipher_request_set_crypt(cdata->async.request,
			(struct scatterlist *)src, dst,
			len, cdata->async.iv);
		ret = crypto_ablkcipher_decrypt(cdata->async.request);
	} else {
		aead_request_set_crypt(cdata->async.arequest,
			(struct scatterlist *)src, dst,
			len, cdata->async.iv);
		ret = crypto_aead_decrypt(cdata->async.arequest);
	}

	return waitfor(cdata->async.result, ret);
}

/* Hash functions */

int cryptodev_hash_init(struct hash_data *hdata, const char *alg_name,
			int hmac_mode, void *mackey, size_t mackeylen)
{
	int ret;

	hdata->async.s = crypto_alloc_ahash(alg_name, 0, 0);
	if (unlikely(IS_ERR(hdata->async.s))) {
		ddebug(1, "Failed to load transform for %s", alg_name);
		return -EINVAL;
	}

	/* Copy the key from user and set to TFM. */
	if (hmac_mode != 0) {
		ret = crypto_ahash_setkey(hdata->async.s, mackey, mackeylen);
		if (unlikely(ret)) {
			ddebug(1, "Setting hmac key failed for %s-%zu.",
					alg_name, mackeylen*8);
			ret = -EINVAL;
			goto error;
		}
	}

	hdata->digestsize = crypto_ahash_digestsize(hdata->async.s);
	hdata->alignmask = crypto_ahash_alignmask(hdata->async.s);

	hdata->async.result = kzalloc(sizeof(*hdata->async.result), GFP_KERNEL);
	if (unlikely(!hdata->async.result)) {
		ret = -ENOMEM;
		goto error;
	}

	init_completion(&hdata->async.result->completion);

	hdata->async.request = ahash_request_alloc(hdata->async.s, GFP_KERNEL);
	if (unlikely(!hdata->async.request)) {
		derr(0, "error allocating async crypto request");
		ret = -ENOMEM;
		goto error;
	}

	ahash_request_set_callback(hdata->async.request,
			CRYPTO_TFM_REQ_MAY_BACKLOG,
			cryptodev_complete, hdata->async.result);

	ret = crypto_ahash_init(hdata->async.request);
	if (unlikely(ret)) {
		derr(0, "error in crypto_hash_init()");
		goto error_request;
	}

	hdata->init = 1;
	return 0;

error_request:
	ahash_request_free(hdata->async.request);
error:
	kfree(hdata->async.result);
	crypto_free_ahash(hdata->async.s);
	return ret;
}

void cryptodev_hash_deinit(struct hash_data *hdata)
{
	if (hdata->init) {
		if (hdata->async.request)
			ahash_request_free(hdata->async.request);
		kfree(hdata->async.result);
		if (hdata->async.s)
			crypto_free_ahash(hdata->async.s);
		hdata->init = 0;
	}
}

int cryptodev_hash_reset(struct hash_data *hdata)
{
	int ret;

	ret = crypto_ahash_init(hdata->async.request);
	if (unlikely(ret)) {
		derr(0, "error in crypto_hash_init()");
		return ret;
	}

	return 0;

}

ssize_t cryptodev_hash_update(struct hash_data *hdata,
				struct scatterlist *sg, size_t len)
{
	int ret;

	reinit_completion(&hdata->async.result->completion);
	ahash_request_set_crypt(hdata->async.request, sg, NULL, len);

	ret = crypto_ahash_update(hdata->async.request);

	return waitfor(hdata->async.result, ret);
}

int cryptodev_hash_final(struct hash_data *hdata, void *output)
{
	int ret;

	reinit_completion(&hdata->async.result->completion);
	ahash_request_set_crypt(hdata->async.request, NULL, output, 0);

	ret = crypto_ahash_final(hdata->async.request);

	return waitfor(hdata->async.result, ret);
}

