/*
 * Copyright (C) 2023 Andreas Steffen, strongSec GmbH
 *
 * Copyright (C) secunet Security Networks AG
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

#include "openxpki_plugin.h"

#include "openxpki_ocsp_responder.h"

typedef struct private_openxpki_plugin_t private_openxpki_plugin_t;

/**
 * Private data of an openxpki_plugin_t object.
 */
struct private_openxpki_plugin_t {

	/**
	 * Public interface.
	 */
	openxpki_plugin_t public;

	/**
	 * OCSP responder
	 */
	ocsp_responder_t *ocsp_responder;
};

METHOD(plugin_t, get_name, char*,
	private_openxpki_plugin_t *this)
{
	return "openxpki";
}

static bool plugin_cb(private_openxpki_plugin_t *this,
					  plugin_feature_t *feature, bool reg, void *cb_data)
{
	if (reg)
	{
		this->ocsp_responder = openxpki_ocsp_responder_create();
		if (this->ocsp_responder)
		{
			lib->ocsp->add_responder(lib->ocsp, this->ocsp_responder);
		}
	}
	else
	{
		if (this->ocsp_responder)
		{
			lib->ocsp->remove_responder(lib->ocsp, this->ocsp_responder);
			this->ocsp_responder->destroy(this->ocsp_responder);
		}
	}
	return TRUE;
}

METHOD(plugin_t, get_features, int,
	private_openxpki_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_CALLBACK((plugin_feature_callback_t)plugin_cb, NULL),
			PLUGIN_PROVIDE(CUSTOM, "ocsp-responder"),
				PLUGIN_DEPENDS(DATABASE, DB_MYSQL),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_openxpki_plugin_t *this)
{
	free(this);
}

/*
 * see header file
 */
plugin_t *openxpki_plugin_create()
{
	private_openxpki_plugin_t *this;

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
