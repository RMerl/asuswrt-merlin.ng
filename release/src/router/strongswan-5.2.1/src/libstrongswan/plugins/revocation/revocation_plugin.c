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

#include "revocation_plugin.h"

#include <library.h>
#include "revocation_validator.h"

typedef struct private_revocation_plugin_t private_revocation_plugin_t;

/**
 * private data of revocation_plugin
 */
struct private_revocation_plugin_t {

	/**
	 * public functions
	 */
	revocation_plugin_t public;

	/**
	 * Validator implementation instance.
	 */
	revocation_validator_t *validator;
};

METHOD(plugin_t, get_name, char*,
	private_revocation_plugin_t *this)
{
	return "revocation";
}

/**
 * Register validator
 */
static bool plugin_cb(private_revocation_plugin_t *this,
					  plugin_feature_t *feature, bool reg, void *cb_data)
{
	if (reg)
	{
		lib->credmgr->add_validator(lib->credmgr, &this->validator->validator);
	}
	else
	{
		lib->credmgr->remove_validator(lib->credmgr,
									   &this->validator->validator);
	}
	return TRUE;
}

METHOD(plugin_t, get_features, int,
	private_revocation_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_CALLBACK((plugin_feature_callback_t)plugin_cb, NULL),
			PLUGIN_PROVIDE(CUSTOM, "revocation"),
				PLUGIN_SDEPEND(CERT_ENCODE, CERT_X509_OCSP_REQUEST),
				PLUGIN_SDEPEND(CERT_DECODE, CERT_X509_OCSP_RESPONSE),
				PLUGIN_SDEPEND(CERT_DECODE, CERT_X509_CRL),
				PLUGIN_SDEPEND(CERT_DECODE, CERT_X509),
				PLUGIN_SDEPEND(FETCHER, NULL),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_revocation_plugin_t *this)
{
	this->validator->destroy(this->validator);
	free(this);
}

/*
 * see header file
 */
plugin_t *revocation_plugin_create()
{
	private_revocation_plugin_t *this;

	INIT(this,
		.public = {
			.plugin = {
				.get_name = _get_name,
				.get_features = _get_features,
				.destroy = _destroy,
			},
		},
		.validator = revocation_validator_create(),
	);

	return &this->public.plugin;
}
