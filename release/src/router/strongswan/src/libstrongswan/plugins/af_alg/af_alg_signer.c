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

#include "af_alg_signer.h"
#include "af_alg_ops.h"

typedef struct private_af_alg_signer_t private_af_alg_signer_t;

/**
 * Private data structure with signing context.
 */
struct private_af_alg_signer_t {

	/**
	 * Public interface of af_alg_signer_t.
	 */
	af_alg_signer_t public;

	/**
	 * AF_ALG operations
	 */
	af_alg_ops_t *ops;

	/**
	 * Size of the truncated signature
	 */
	size_t block_size;

	/**
	 * Default key size
	 */
	size_t key_size;
};

/**
 * Algorithm database
 */
static struct {
	integrity_algorithm_t id;
	char *name;
	size_t block_size;
	size_t key_size;
} algs[AF_ALG_SIGNER] = {
	{AUTH_HMAC_SHA1_96,			"hmac(sha1)",		12,		20,	},
	{AUTH_HMAC_SHA1_128,		"hmac(sha1)",		16,		20,	},
	{AUTH_HMAC_SHA1_160,		"hmac(sha1)",		20,		20,	},
	{AUTH_HMAC_SHA2_256_96,		"hmac(sha256)",		12,		32,	},
	{AUTH_HMAC_SHA2_256_128,	"hmac(sha256)",		16,		32,	},
	{AUTH_HMAC_MD5_96,			"hmac(md5)",		12,		16,	},
	{AUTH_HMAC_MD5_128,			"hmac(md5)",		16,		16,	},
	{AUTH_HMAC_SHA2_256_256,	"hmac(sha384)",		32,		32,	},
	{AUTH_HMAC_SHA2_384_192,	"hmac(sha384)",		24,		48,	},
	{AUTH_HMAC_SHA2_384_384,	"hmac(sha384)",		48,		48,	},
	{AUTH_HMAC_SHA2_512_256,	"hmac(sha512)",		32,		64,	},
	{AUTH_HMAC_SHA2_512_512,	"hmac(sha512)",		64,		64,	},
	{AUTH_AES_XCBC_96,			"xcbc(aes)",		12,		16,	},
	{AUTH_CAMELLIA_XCBC_96,		"xcbc(camellia)",	12,		16,	},
};

/**
 * See header.
 */
void af_alg_signer_probe(plugin_feature_t *features, int *pos)
{
	af_alg_ops_t *ops;
	int i;

	for (i = 0; i < countof(algs); i++)
	{
		ops = af_alg_ops_create("hash", algs[i].name);
		if (ops)
		{
			ops->destroy(ops);
			features[(*pos)++] = PLUGIN_PROVIDE(SIGNER, algs[i].id);
		}
	}
}

/**
 * Get the kernel algorithm string and block/key size for our identifier
 */
static size_t lookup_alg(integrity_algorithm_t algo, char **name,
						 size_t *key_size)
{
	int i;

	for (i = 0; i < countof(algs); i++)
	{
		if (algs[i].id == algo)
		{
			*name = algs[i].name;
			*key_size = algs[i].key_size;
			return algs[i].block_size;
		}
	}
	return 0;
}

METHOD(signer_t, get_signature, bool,
	private_af_alg_signer_t *this, chunk_t data, uint8_t *buffer)
{
	return this->ops->hash(this->ops, data, buffer, this->block_size);
}

METHOD(signer_t, allocate_signature, bool,
	private_af_alg_signer_t *this, chunk_t data, chunk_t *chunk)
{
	if (chunk)
	{
		*chunk = chunk_alloc(this->block_size);
		return get_signature(this, data, chunk->ptr);
	}
	return get_signature(this, data, NULL);
}

METHOD(signer_t, verify_signature, bool,
	private_af_alg_signer_t *this, chunk_t data, chunk_t signature)
{
	char sig[this->block_size];

	if (signature.len != this->block_size)
	{
		return FALSE;
	}
	if (!get_signature(this, data, sig))
	{
		return FALSE;
	}
	return memeq_const(signature.ptr, sig, signature.len);
}

METHOD(signer_t, get_key_size, size_t,
	private_af_alg_signer_t *this)
{
	return this->key_size;
}

METHOD(signer_t, get_block_size, size_t,
	private_af_alg_signer_t *this)
{
	return this->block_size;
}

METHOD(signer_t, set_key, bool,
	private_af_alg_signer_t *this, chunk_t key)
{
	this->ops->reset(this->ops);
	return this->ops->set_key(this->ops, key);
}

METHOD(signer_t, destroy, void,
	private_af_alg_signer_t *this)
{
	this->ops->destroy(this->ops);
	free(this);
}

/*
 * Described in header
 */
af_alg_signer_t *af_alg_signer_create(integrity_algorithm_t algo)
{
	private_af_alg_signer_t *this;
	size_t block_size, key_size;
	char *name;

	block_size = lookup_alg(algo, &name, &key_size);
	if (!block_size)
	{	/* not supported by kernel */
		return NULL;
	}

	INIT(this,
		.public = {
			.signer = {
				.get_signature = _get_signature,
				.allocate_signature = _allocate_signature,
				.verify_signature = _verify_signature,
				.get_key_size = _get_key_size,
				.get_block_size = _get_block_size,
				.set_key = _set_key,
				.destroy = _destroy,
			},
		},
		.ops = af_alg_ops_create("hash", name),
		.block_size = block_size,
		.key_size = key_size,
	);
	if (!this->ops)
	{
		free(this);
		return NULL;
	}
	return &this->public;
}
