/*
 * Copyright (C) 2010 Andreas Steffen
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

#include "tnc_tnccs_plugin.h"
#include "tnc_tnccs_manager.h"

#include <tnc/tnc.h>

#include <utils/debug.h>

typedef struct private_tnc_tnccs_plugin_t private_tnc_tnccs_plugin_t;

/**
 * Private data of a tnc_tnccs_plugin_t object.
 */
struct private_tnc_tnccs_plugin_t {

	/**
	 * Public interface.
	 */
	tnc_tnccs_plugin_t public;

};


METHOD(plugin_t, get_name, char*,
	private_tnc_tnccs_plugin_t *this)
{
	return "tnc-tnccs";
}

METHOD(plugin_t, get_features, int,
	private_tnc_tnccs_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_CALLBACK(tnc_manager_register, tnc_tnccs_manager_create),
			PLUGIN_PROVIDE(CUSTOM, "tnccs-manager"),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_tnc_tnccs_plugin_t *this)
{
	libtnccs_deinit();
	free(this);
}

/*
 * see header file
 */
plugin_t *tnc_tnccs_plugin_create(void)
{
	private_tnc_tnccs_plugin_t *this;

	if (lib->integrity)
	{
		if (lib->integrity->check(lib->integrity, "libtnccs", libtnccs_init))
		{
			DBG1(DBG_LIB,
				 "lib    'libtnccs': passed file and segment integrity tests");
		}
		else
		{
			DBG1(DBG_LIB,
				 "lib    'libtnccs': failed integrity tests");
			return NULL;
		}
	}

	INIT(this,
		.public = {
			.plugin = {
				.get_name = _get_name,
				.get_features = _get_features,
				.destroy = _destroy,
			},
		},
	);

	libtnccs_init();

	return &this->public.plugin;
}

