/*
 * Copyright (C) 2011 Andreas Steffen
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

#include "tnccs_dynamic_plugin.h"
#include "tnccs_dynamic.h"

#include <tnc/tnccs/tnccs_manager.h>

METHOD(plugin_t, get_name, char*,
	tnccs_dynamic_plugin_t *this)
{
	return "tnccs-dynamic";
}

METHOD(plugin_t, get_features, int,
	tnccs_dynamic_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_CALLBACK(tnccs_method_register, tnccs_dynamic_create),
			PLUGIN_PROVIDE(CUSTOM, "tnccs-dynamic"),
				PLUGIN_DEPENDS(CUSTOM, "tnccs-1.1"),
				PLUGIN_DEPENDS(CUSTOM, "tnccs-2.0"),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	tnccs_dynamic_plugin_t *this)
{
	free(this);
}

/*
 * see header file
 */
plugin_t *tnccs_dynamic_plugin_create()
{
	tnccs_dynamic_plugin_t *this;

	INIT(this,
		.plugin = {
			.get_name = _get_name,
			.get_features = _get_features,
			.destroy = _destroy,
		},
	);

	return &this->plugin;
}
