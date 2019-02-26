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

#include "af_alg_hasher.h"
#include "af_alg_ops.h"

typedef struct private_af_alg_hasher_t private_af_alg_hasher_t;

/**
 * Private data of af_alg_hasher_t
 */
struct private_af_alg_hasher_t {

	/**
	 * Public part of this class.
	 */
	af_alg_hasher_t public;

	/**
	 * AF_ALG operations
	 */
	af_alg_ops_t *ops;

	/**
	 * Size of the hash
	 */
	size_t size;
};

/**
 * Algorithm database
 */
static struct {
	hash_algorithm_t id;
	char *name;
	size_t size;
} algs[AF_ALG_HASHER] = {
	{HASH_MD4,			"md4",			HASH_SIZE_MD4 		},
	{HASH_MD5,			"md5",			HASH_SIZE_MD5 		},
	{HASH_SHA1,			"sha1",			HASH_SIZE_SHA1		},
	{HASH_SHA224,		"sha224",		HASH_SIZE_SHA224	},
	{HASH_SHA256,		"sha256",		HASH_SIZE_SHA256	},
	{HASH_SHA384,		"sha384",		HASH_SIZE_SHA384	},
	{HASH_SHA512,		"sha512",		HASH_SIZE_SHA512	},
};

/**
 * See header.
 */
void af_alg_hasher_probe(plugin_feature_t *features, int *pos)
{
	af_alg_ops_t *ops;
	int i;

	for (i = 0; i < countof(algs); i++)
	{
		ops = af_alg_ops_create("hash", algs[i].name);
		if (ops)
		{
			ops->destroy(ops);
			features[(*pos)++] = PLUGIN_PROVIDE(HASHER, algs[i].id);
		}
	}
}

/**
 * Get the kernel algorithm string and hash size for our identifier
 */
static size_t lookup_alg(hash_algorithm_t algo, char **name)
{
	int i;

	for (i = 0; i < countof(algs); i++)
	{
		if (algs[i].id == algo)
		{
			*name = algs[i].name;
			return algs[i].size;
		}
	}
	return 0;
}

METHOD(hasher_t, get_hash_size, size_t,
	private_af_alg_hasher_t *this)
{
	return this->size;
}

METHOD(hasher_t, reset, bool,
	private_af_alg_hasher_t *this)
{
	this->ops->reset(this->ops);
	return TRUE;
}

METHOD(hasher_t, get_hash, bool,
	private_af_alg_hasher_t *this, chunk_t chunk, uint8_t *hash)
{
	return this->ops->hash(this->ops, chunk, hash, this->size);
}

METHOD(hasher_t, allocate_hash, bool,
	private_af_alg_hasher_t *this, chunk_t chunk, chunk_t *hash)
{
	if (hash)
	{
		*hash = chunk_alloc(get_hash_size(this));
		return get_hash(this, chunk, hash->ptr);
	}
	return get_hash(this, chunk, NULL);
}

METHOD(hasher_t, destroy, void,
	private_af_alg_hasher_t *this)
{
	this->ops->destroy(this->ops);
	free(this);
}

/*
 * Described in header
 */
af_alg_hasher_t *af_alg_hasher_create(hash_algorithm_t algo)
{
	private_af_alg_hasher_t *this;
	char *name;
	size_t size;

	size = lookup_alg(algo, &name);
	if (!size)
	{	/* not supported by kernel */
		return NULL;
	}

	INIT(this,
		.public = {
			.hasher = {
				.get_hash = _get_hash,
				.allocate_hash = _allocate_hash,
				.get_hash_size = _get_hash_size,
				.reset = _reset,
				.destroy = _destroy,
			},
		},
		.ops = af_alg_ops_create("hash", name),
		.size = size,
	);
	if (!this->ops)
	{
		free(this);
		return NULL;
	}
	return &this->public;
}
