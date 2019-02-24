/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include "af_alg_crypter.h"
#include "af_alg_ops.h"

typedef struct private_af_alg_crypter_t private_af_alg_crypter_t;

/**
 * Private data of af_alg_crypter_t
 */
struct private_af_alg_crypter_t {

	/**
	 * Public part of this class.
	 */
	af_alg_crypter_t public;

	/**
	 * AF_ALG operations
	 */
	af_alg_ops_t *ops;

	/**
	 * Size of the truncated signature
	 */
	size_t block_size;

	/**
	 * Size of the keymat
	 */
	size_t keymat_size;

	/**
	 * Size of initialization vector
	 */
	size_t iv_size;
};

/**
 * Algorithm database
 */
static struct {
	encryption_algorithm_t id;
	char *name;
	size_t block_size;
	/* key size of the algorithm */
	size_t key_size;
	/* size of the keying material (key + nonce for ctr mode) */
	size_t keymat_size;
	size_t iv_size;
} algs[AF_ALG_CRYPTER] = {
	{ENCR_DES,			"cbc(des)",					 8,	 8,	 8,	 8,	},
	{ENCR_DES_ECB,		"ecb(des)",					 8,	 8,	 8,	 0,	},
	{ENCR_3DES,			"cbc(des3_ede)",			 8,	24,	24,	 8,	},
	{ENCR_AES_CBC,		"cbc(aes)",					16,	16,	16,	16,	},
	{ENCR_AES_CBC,		"cbc(aes)",					16,	24,	24,	16,	},
	{ENCR_AES_CBC,		"cbc(aes)",					16,	32,	32,	16,	},
	{ENCR_AES_CTR,		"rfc3686(ctr(aes))",		 1,	16,	20,	 8,	},
	{ENCR_AES_CTR,		"rfc3686(ctr(aes))",		 1,	24,	28,	 8,	},
	{ENCR_AES_CTR,		"rfc3686(ctr(aes))",		 1,	32,	36,	 8,	},
	{ENCR_CAMELLIA_CBC,	"cbc(camellia)",			16,	16,	16,	16,	},
	{ENCR_CAMELLIA_CBC,	"cbc(camellia)",			16,	24,	24,	16,	},
	{ENCR_CAMELLIA_CBC,	"cbc(camellia)",			16,	32,	32,	16,	},
	{ENCR_CAMELLIA_CTR,	"rfc3686(ctr(camellia))",	 1,	16,	20,	 8,	},
	{ENCR_CAMELLIA_CTR,	"rfc3686(ctr(camellia))",	 1,	24,	28,	 8,	},
	{ENCR_CAMELLIA_CTR,	"rfc3686(ctr(camellia))",	 1,	32,	36,	 8,	},
	{ENCR_CAST,			"cbc(cast5)",				 8,	16,	16,	 8,	},
	{ENCR_BLOWFISH,		"cbc(blowfish)",			 8,	16,	16,	 8,	},
	{ENCR_BLOWFISH,		"cbc(blowfish)",			 8,	24,	24,	 8,	},
	{ENCR_BLOWFISH,		"cbc(blowfish)",			 8,	32,	32,	 8,	},
	{ENCR_SERPENT_CBC,	"cbc(serpent)",				16,	16,	16,	16,	},
	{ENCR_SERPENT_CBC,	"cbc(serpent)",				16,	24,	24,	16,	},
	{ENCR_SERPENT_CBC,	"cbc(serpent)",				16,	32,	32,	16,	},
	{ENCR_TWOFISH_CBC,	"cbc(twofish)",				16,	16,	16,	16,	},
	{ENCR_TWOFISH_CBC,	"cbc(twofish)",				16,	24,	24,	16,	},
	{ENCR_TWOFISH_CBC,	"cbc(twofish)",				16,	32,	32,	16,	},
};

/**
 * See header.
 */
void af_alg_crypter_probe(plugin_feature_t *features, int *pos)
{
	af_alg_ops_t *ops;
	int i;

	for (i = 0; i < countof(algs); i++)
	{
		ops = af_alg_ops_create("skcipher", algs[i].name);
		if (ops)
		{
			ops->destroy(ops);
			features[(*pos)++] = PLUGIN_PROVIDE(CRYPTER,
												algs[i].id, algs[i].key_size);
		}
	}
}

/**
 * Get the kernel algorithm string and block/key size for our identifier
 */
static size_t lookup_alg(encryption_algorithm_t algo, char **name,
						 size_t key_size, size_t *keymat_size, size_t *iv_size)
{
	int i;

	for (i = 0; i < countof(algs); i++)
	{
		if (algs[i].id == algo &&
			(key_size == 0 || algs[i].key_size == key_size))
		{
			*name = algs[i].name;
			*keymat_size = algs[i].keymat_size;
			*iv_size = algs[i].iv_size;
			return algs[i].block_size;
		}
	}
	return 0;
}

METHOD(crypter_t, decrypt, bool,
	private_af_alg_crypter_t *this, chunk_t data, chunk_t iv, chunk_t *dst)
{
	if (dst)
	{
		*dst = chunk_alloc(data.len);
		return this->ops->crypt(this->ops, ALG_OP_DECRYPT, iv, data, dst->ptr);
	}
	return this->ops->crypt(this->ops, ALG_OP_DECRYPT, iv, data, data.ptr);
}

METHOD(crypter_t, encrypt, bool,
	private_af_alg_crypter_t *this, chunk_t data, chunk_t iv, chunk_t *dst)
{
	if (dst)
	{
		*dst = chunk_alloc(data.len);
		return this->ops->crypt(this->ops, ALG_OP_ENCRYPT, iv, data, dst->ptr);
	}
	return this->ops->crypt(this->ops, ALG_OP_ENCRYPT, iv, data, data.ptr);
}

METHOD(crypter_t, get_block_size, size_t,
	private_af_alg_crypter_t *this)
{
	return this->block_size;
}

METHOD(crypter_t, get_iv_size, size_t,
	private_af_alg_crypter_t *this)
{
	return this->iv_size;
}

METHOD(crypter_t, get_key_size, size_t,
	private_af_alg_crypter_t *this)
{
	return this->keymat_size;
}

METHOD(crypter_t, set_key, bool,
	private_af_alg_crypter_t *this, chunk_t key)
{
	return this->ops->set_key(this->ops, key);
}

METHOD(crypter_t, destroy, void,
	private_af_alg_crypter_t *this)
{
	this->ops->destroy(this->ops);
	free(this);
}

/*
 * Described in header
 */
af_alg_crypter_t *af_alg_crypter_create(encryption_algorithm_t algo,
										size_t key_size)
{
	private_af_alg_crypter_t *this;
	size_t block_size, keymat_size, iv_size;
	char *name;

	block_size = lookup_alg(algo, &name, key_size, &keymat_size, &iv_size);
	if (!block_size)
	{	/* not supported by kernel */
		return NULL;
	}

	INIT(this,
		.public = {
			.crypter = {
				.encrypt = _encrypt,
				.decrypt = _decrypt,
				.get_block_size = _get_block_size,
				.get_iv_size = _get_iv_size,
				.get_key_size = _get_key_size,
				.set_key = _set_key,
				.destroy = _destroy,
			},
		},
		.block_size = block_size,
		.keymat_size = keymat_size,
		.iv_size = iv_size,
		.ops = af_alg_ops_create("skcipher", name),
	);

	if (!this->ops)
	{
		free(this);
		return NULL;
	}
	return &this->public;
}
