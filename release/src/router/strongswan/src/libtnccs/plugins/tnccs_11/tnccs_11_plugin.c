/*
 * Copyright (C) 2010-2017 Andreas Steffen
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

#include "tnccs_11_plugin.h"
#include "tnccs_11.h"

#include <tnc/tnccs/tnccs_manager.h>
#include <libxml/parser.h>

METHOD(plugin_t, get_name, char*,
	tnccs_11_plugin_t *this)
{
	return "tnccs-11";
}

METHOD(plugin_t, get_features, int,
	tnccs_11_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_CALLBACK(tnccs_method_register, tnccs_11_create),
			PLUGIN_PROVIDE(CUSTOM, "tnccs-1.1"),
				PLUGIN_DEPENDS(CUSTOM, "tnccs-manager"),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	tnccs_11_plugin_t *this)
{
	xmlCleanupParser();
	free(this);
}

/*
 * see header file
 */
plugin_t *tnccs_11_plugin_create()
{
	tnccs_11_plugin_t *this;

	INIT(this,
		.plugin = {
			.get_name = _get_name,
			.get_features = _get_features,
			.destroy = _destroy,
		},
	);
	xmlInitParser();

	return &this->plugin;
}
