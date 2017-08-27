/*
 * Copyright (C) 2010 Andreas Steffen
 * Hochschule fuer Technik Rapperswil
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

#include "eap_tnc_plugin.h"
#include "eap_tnc.h"

#include <daemon.h>

METHOD(plugin_t, get_name, char*,
	eap_tnc_plugin_t *this)
{
	return "eap-tnc";
}

METHOD(plugin_t, get_features, int,
	eap_tnc_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_CALLBACK(eap_method_register, eap_tnc_create_server),
			PLUGIN_PROVIDE(EAP_SERVER, EAP_TNC),
				PLUGIN_DEPENDS(EAP_SERVER, EAP_TTLS),
				PLUGIN_DEPENDS(CUSTOM, "tnccs-manager"),
		PLUGIN_CALLBACK(eap_method_register, eap_tnc_create_peer),
			PLUGIN_PROVIDE(EAP_PEER, EAP_TNC),
				PLUGIN_DEPENDS(EAP_PEER, EAP_TTLS),
				PLUGIN_DEPENDS(CUSTOM, "tnccs-manager"),
		PLUGIN_CALLBACK(eap_method_register, eap_tnc_pt_create_server),
			PLUGIN_PROVIDE(EAP_SERVER, EAP_PT_EAP),
				PLUGIN_DEPENDS(EAP_SERVER, EAP_TTLS),
				PLUGIN_DEPENDS(CUSTOM, "tnccs-manager"),
		PLUGIN_CALLBACK(eap_method_register, eap_tnc_pt_create_peer),
			PLUGIN_PROVIDE(EAP_PEER, EAP_PT_EAP),
				PLUGIN_DEPENDS(EAP_PEER, EAP_TTLS),
				PLUGIN_DEPENDS(CUSTOM, "tnccs-manager"),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	eap_tnc_plugin_t *this)
{
	free(this);
}

/*
 * see header file
 */
plugin_t *eap_tnc_plugin_create()
{
	eap_tnc_plugin_t *this;

	INIT(this,
		.plugin = {
			.get_name = _get_name,
			.get_features = _get_features,
			.destroy = _destroy,
		},
	);

	return &this->plugin;
}

