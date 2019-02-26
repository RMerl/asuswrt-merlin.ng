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

#include "af_alg_plugin.h"

#include <library.h>

#include "af_alg_hasher.h"
#include "af_alg_signer.h"
#include "af_alg_prf.h"
#include "af_alg_crypter.h"

#include <unistd.h>

typedef struct private_af_alg_plugin_t private_af_alg_plugin_t;

/**
 * private data of af_alg_plugin
 */
struct private_af_alg_plugin_t {

	/**
	 * public functions
	 */
	af_alg_plugin_t public;
};

METHOD(plugin_t, get_name, char*,
	private_af_alg_plugin_t *this)
{
	return "af-alg";
}

static bool af_alg_supported()
{
	int fd;

	fd = socket(AF_ALG, SOCK_SEQPACKET, 0);
	if (fd != -1)
	{
		close(fd);
		return true;
	}
	return false;
}

METHOD(plugin_t, get_features, int,
	private_af_alg_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[AF_ALG_HASHER + AF_ALG_SIGNER +
							  AF_ALG_PRF + AF_ALG_CRYPTER + 4] = {};
	static int count = 0;

	if (!count)
	{	/* initialize only once */
		if (!af_alg_supported())
		{
			return 0;
		}
		f[count++] = PLUGIN_REGISTER(HASHER, af_alg_hasher_create);
		af_alg_hasher_probe(f, &count);
		f[count++] = PLUGIN_REGISTER(SIGNER, af_alg_signer_create);
		af_alg_signer_probe(f, &count);
		f[count++] = PLUGIN_REGISTER(PRF, af_alg_prf_create);
		af_alg_prf_probe(f, &count);
		f[count++] = PLUGIN_REGISTER(CRYPTER, af_alg_crypter_create);
		af_alg_crypter_probe(f, &count);
	}
	*features = f;
	return count;
}

METHOD(plugin_t, destroy, void,
	private_af_alg_plugin_t *this)
{
	free(this);
}

/*
 * see header file
 */
plugin_t *af_alg_plugin_create()
{
	private_af_alg_plugin_t *this;

	INIT(this,
		.public = {
			.plugin = {
				.get_name = _get_name,
				.get_features = _get_features,
				.destroy = _destroy,
			},
		},
	);

	return &this->public.plugin;
}
