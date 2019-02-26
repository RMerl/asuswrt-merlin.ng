/*
 * Copyright (C) 2012 Martin Willi
 * Copyright (C) 2012 revosec AG
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

#include "xauth_pam_plugin.h"
#include "xauth_pam.h"
#include "xauth_pam_listener.h"

#include <daemon.h>

#ifndef CAP_AUDIT_WRITE
#define CAP_AUDIT_WRITE 29
#endif

typedef struct private_xauth_pam_plugin_t private_xauth_pam_plugin_t;

/**
 * private data of xauth_pam plugin
 */
struct private_xauth_pam_plugin_t {

	/**
	 * implements plugin interface
	 */
	xauth_pam_plugin_t public;

	/**
	 * Listener
	 */
	xauth_pam_listener_t *listener;

	/**
	 * Do PAM session management?
	 */
	bool session;
};

/**
 * Register XAuth method and listener
 */
static bool register_listener(private_xauth_pam_plugin_t *this,
							  plugin_feature_t *feature, bool reg, void *data)
{
	if (reg)
	{
		charon->bus->add_listener(charon->bus, &this->listener->listener);
	}
	else
	{
		charon->bus->remove_listener(charon->bus, &this->listener->listener);
	}
	return TRUE;
}

METHOD(plugin_t, get_name, char*,
	private_xauth_pam_plugin_t *this)
{
	return "xauth-pam";
}

METHOD(plugin_t, get_features, int,
	private_xauth_pam_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_CALLBACK(xauth_method_register, xauth_pam_create_server),
			PLUGIN_PROVIDE(XAUTH_SERVER, "pam"),
		PLUGIN_CALLBACK((plugin_feature_callback_t)register_listener, NULL),
			PLUGIN_PROVIDE(CUSTOM, "pam-session"),
	};
	*features = f;
	if (!this->session)
	{
		return 2;
	}
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_xauth_pam_plugin_t *this)
{
	this->listener->destroy(this->listener);
	free(this);
}

/*
 * see header file
 */
plugin_t *xauth_pam_plugin_create()
{
	private_xauth_pam_plugin_t *this;

	/* required for PAM authentication */
	if (!lib->caps->keep(lib->caps, CAP_AUDIT_WRITE))
	{
		DBG1(DBG_DMN, "xauth-pam plugin requires CAP_AUDIT_WRITE capability");
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
		.session = lib->settings->get_str(lib->settings,
						"%s.plugins.xauth-pam.session", FALSE, lib->ns),
		.listener = xauth_pam_listener_create(),
	);

	return &this->public.plugin;
}
