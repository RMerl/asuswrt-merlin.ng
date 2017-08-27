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

#include "af_alg_prf.h"
#include "af_alg_ops.h"

typedef struct private_af_alg_prf_t private_af_alg_prf_t;

/**
 * Private data of a af_alg_prf_t object.
 */
struct private_af_alg_prf_t {

	/**
	 * Public af_alg_prf_t interface.
	 */
	af_alg_prf_t public;

	/**
	 * AF_ALG operations
	 */
	af_alg_ops_t *ops;

	/**
	 * Size of the PRF output
	 */
	size_t block_size;

	/**
	 * Default key size
	 */
	size_t key_size;

	/**
	 * Using an XCBC algorithm?
	 */
	bool xcbc;
};

/**
 * Algorithm database
 */
static struct {
	pseudo_random_function_t id;
	char *name;
	size_t block_size;
	bool xcbc;
} algs[AF_ALG_PRF] = {
	{PRF_HMAC_SHA1,			"hmac(sha1)",		20,		FALSE,	},
	{PRF_HMAC_SHA2_256,		"hmac(sha256)",		32,		FALSE,	},
	{PRF_HMAC_MD5,			"hmac(md5)",		16,		FALSE,	},
	{PRF_HMAC_SHA2_384,		"hmac(sha384)",		48,		FALSE,	},
	{PRF_HMAC_SHA2_512,		"hmac(sha512)",		64,		FALSE,	},
	{PRF_AES128_XCBC,		"xcbc(aes)",		16,		TRUE,	},
	{PRF_CAMELLIA128_XCBC,	"xcbc(camellia)",	16,		TRUE,	},
};

/**
 * See header.
 */
void af_alg_prf_probe(plugin_feature_t *features, int *pos)
{
	af_alg_ops_t *ops;
	int i;

	for (i = 0; i < countof(algs); i++)
	{
		ops = af_alg_ops_create("hash", algs[i].name);
		if (ops)
		{
			ops->destroy(ops);
			features[(*pos)++] = PLUGIN_PROVIDE(PRF, algs[i].id);
		}
	}
}

/**
 * Get the kernel algorithm string and block size for our identifier
 */
static size_t lookup_alg(pseudo_random_function_t algo, char **name, bool *xcbc)
{
	int i;

	for (i = 0; i < countof(algs); i++)
	{
		if (algs[i].id == algo)
		{
			*name = algs[i].name;
			*xcbc = algs[i].xcbc;
			return algs[i].block_size;
		}
	}
	return 0;
}

METHOD(prf_t, get_bytes, bool,
	private_af_alg_prf_t *this, chunk_t seed, u_int8_t *buffer)
{
	return this->ops->hash(this->ops, seed, buffer, this->block_size);
}

METHOD(prf_t, allocate_bytes, bool,
	private_af_alg_prf_t *this, chunk_t seed, chunk_t *chunk)
{
	if (chunk)
	{
		*chunk = chunk_alloc(this->block_size);
		return get_bytes(this, seed, chunk->ptr);
	}
	return get_bytes(this, seed, NULL);
}

METHOD(prf_t, get_block_size, size_t,
	private_af_alg_prf_t *this)
{
	return this->block_size;
}

METHOD(prf_t, get_key_size, size_t,
	private_af_alg_prf_t *this)
{
	return this->block_size;
}

METHOD(prf_t, set_key, bool,
	private_af_alg_prf_t *this, chunk_t key)
{
	char buf[this->block_size];

	if (this->xcbc)
	{
		/* The kernel currently does not support variable length XCBC keys,
		 * do RFC4434 key padding/reduction manually. */
		if (key.len < this->block_size)
		{
			memset(buf, 0, this->block_size);
			memcpy(buf, key.ptr, key.len);
			key = chunk_from_thing(buf);
		}
		else if (key.len > this->block_size)
		{
			memset(buf, 0, this->block_size);
			if (!this->ops->set_key(this->ops, chunk_from_thing(buf)) ||
				!this->ops->hash(this->ops, key, buf, this->block_size))
			{
				return FALSE;
			}
			key = chunk_from_thing(buf);
		}
	}
	return this->ops->set_key(this->ops, key);
}

METHOD(prf_t, destroy, void,
	private_af_alg_prf_t *this)
{
	this->ops->destroy(this->ops);
	free(this);
}

/*
 * Described in header.
 */
af_alg_prf_t *af_alg_prf_create(pseudo_random_function_t algo)
{
	private_af_alg_prf_t *this;
	size_t block_size;
	bool xcbc;
	char *name;

	block_size = lookup_alg(algo, &name, &xcbc);
	if (!block_size)
	{	/* not supported by kernel */
		return NULL;
	}

	INIT(this,
		.public = {
			.prf = {
				.get_bytes = _get_bytes,
				.allocate_bytes = _allocate_bytes,
				.get_block_size = _get_block_size,
				.get_key_size = _get_key_size,
				.set_key = _set_key,
				.destroy = _destroy,
			},
		},
		.ops = af_alg_ops_create("hash", name),
		.block_size = block_size,
		.xcbc = xcbc,
	);
	if (!this->ops)
	{
		free(this);
		return NULL;
	}
	return &this->public;
}
