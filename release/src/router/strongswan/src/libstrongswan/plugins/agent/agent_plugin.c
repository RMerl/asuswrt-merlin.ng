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

#include "agent_plugin.h"

#include <library.h>
#include "agent_private_key.h"

typedef struct private_agent_plugin_t private_agent_plugin_t;

/**
 * private data of agent_plugin
 */
struct private_agent_plugin_t {

	/**
	 * public functions
	 */
	agent_plugin_t public;
};

METHOD(plugin_t, get_name, char*,
	private_agent_plugin_t *this)
{
	return "agent";
}

METHOD(plugin_t, get_features, int,
	private_agent_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_REGISTER(PRIVKEY, agent_private_key_open, FALSE),
			PLUGIN_PROVIDE(PRIVKEY, KEY_ANY),
			PLUGIN_PROVIDE(PRIVKEY, KEY_RSA),
			PLUGIN_PROVIDE(PRIVKEY, KEY_ECDSA),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_agent_plugin_t *this)
{
	free(this);
}

/*
 * see header file
 */
plugin_t *agent_plugin_create()
{
	private_agent_plugin_t *this;

	/* required to connect to ssh-agent socket */
	if (!lib->caps->keep(lib->caps, CAP_DAC_OVERRIDE))
	{
		DBG1(DBG_DMN, "agent plugin requires CAP_DAC_OVERRIDE capability");
		return NULL;
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

	return &this->public.plugin;
}
