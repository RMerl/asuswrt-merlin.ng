/*
 * Copyright (C) 2022 Tobias Brunner, codelabs GmbH
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "kdf_plugin.h"
#include "kdf_kdf.h"

#include <library.h>

typedef struct private_kdf_plugin_t private_kdf_plugin_t;

/**
 * Private data
 */
struct private_kdf_plugin_t {

	/**
	 * Public interface
	 */
	kdf_plugin_t public;
};

METHOD(plugin_t, get_name, char*,
	private_kdf_plugin_t *this)
{
	return "kdf";
}

METHOD(plugin_t, get_features, int,
	private_kdf_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_REGISTER(KDF, kdf_kdf_create),
			PLUGIN_PROVIDE(KDF, KDF_PRF),
				PLUGIN_SDEPEND(PRF, PRF_HMAC_SHA1),
				PLUGIN_SDEPEND(PRF, PRF_HMAC_SHA2_256),
				PLUGIN_SDEPEND(PRF, PRF_HMAC_SHA2_384),
				PLUGIN_SDEPEND(PRF, PRF_HMAC_SHA2_512),
				PLUGIN_SDEPEND(PRF, PRF_AES128_XCBC),
				PLUGIN_SDEPEND(PRF, PRF_AES128_CMAC),
			PLUGIN_PROVIDE(KDF, KDF_PRF_PLUS),
				PLUGIN_SDEPEND(PRF, PRF_HMAC_SHA1),
				PLUGIN_SDEPEND(PRF, PRF_HMAC_SHA2_256),
				PLUGIN_SDEPEND(PRF, PRF_HMAC_SHA2_384),
				PLUGIN_SDEPEND(PRF, PRF_HMAC_SHA2_512),
				PLUGIN_SDEPEND(PRF, PRF_AES128_XCBC),
				PLUGIN_SDEPEND(PRF, PRF_AES128_CMAC),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_kdf_plugin_t *this)
{
	free(this);
}

/*
 * Described in header
 */
plugin_t *kdf_plugin_create()
{
	private_kdf_plugin_t *this;

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
