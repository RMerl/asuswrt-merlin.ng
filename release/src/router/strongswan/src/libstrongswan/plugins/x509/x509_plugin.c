/*
 * Copyright (C) 2008-2009 Martin Willi
 * HSR Hochschule fuer Technik Rapperswil
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

#include "x509_plugin.h"

#include <library.h>
#include "x509_cert.h"
#include "x509_ac.h"
#include "x509_crl.h"
#include "x509_ocsp_request.h"
#include "x509_ocsp_response.h"
#include "x509_pkcs10.h"

typedef struct private_x509_plugin_t private_x509_plugin_t;

/**
 * private data of x509_plugin
 */
struct private_x509_plugin_t {

	/**
	 * public functions
	 */
	x509_plugin_t public;
};

METHOD(plugin_t, get_name, char*,
	private_x509_plugin_t *this)
{
	return "x509";
}

METHOD(plugin_t, get_features, int,
	private_x509_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_REGISTER(CERT_ENCODE, x509_cert_gen, FALSE),
			PLUGIN_PROVIDE(CERT_ENCODE, CERT_X509),
				PLUGIN_DEPENDS(HASHER, HASH_SHA1),
		PLUGIN_REGISTER(CERT_DECODE, x509_cert_load, TRUE),
			PLUGIN_PROVIDE(CERT_DECODE, CERT_X509),
				PLUGIN_DEPENDS(HASHER, HASH_SHA1),
				PLUGIN_DEPENDS(PUBKEY, KEY_ANY),

		PLUGIN_REGISTER(CERT_ENCODE, x509_ac_gen, FALSE),
			PLUGIN_PROVIDE(CERT_ENCODE, CERT_X509_AC),
		PLUGIN_REGISTER(CERT_DECODE, x509_ac_load, TRUE),
			PLUGIN_PROVIDE(CERT_DECODE, CERT_X509_AC),

		PLUGIN_REGISTER(CERT_ENCODE, x509_crl_gen, FALSE),
			PLUGIN_PROVIDE(CERT_ENCODE, CERT_X509_CRL),
		PLUGIN_REGISTER(CERT_DECODE, x509_crl_load, TRUE),
			PLUGIN_PROVIDE(CERT_DECODE, CERT_X509_CRL),

		PLUGIN_REGISTER(CERT_ENCODE, x509_ocsp_request_gen, FALSE),
			PLUGIN_PROVIDE(CERT_ENCODE, CERT_X509_OCSP_REQUEST),
				PLUGIN_DEPENDS(HASHER, HASH_SHA1),
				PLUGIN_DEPENDS(RNG, RNG_WEAK),
		PLUGIN_REGISTER(CERT_DECODE, x509_ocsp_response_load, TRUE),
			PLUGIN_PROVIDE(CERT_DECODE, CERT_X509_OCSP_RESPONSE),

		PLUGIN_REGISTER(CERT_ENCODE, x509_pkcs10_gen, FALSE),
			PLUGIN_PROVIDE(CERT_ENCODE, CERT_PKCS10_REQUEST),
		PLUGIN_REGISTER(CERT_DECODE, x509_pkcs10_load, TRUE),
			PLUGIN_PROVIDE(CERT_DECODE, CERT_PKCS10_REQUEST),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_x509_plugin_t *this)
{
	free(this);
}

/*
 * see header file
 */
plugin_t *x509_plugin_create()
{
	private_x509_plugin_t *this;

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

