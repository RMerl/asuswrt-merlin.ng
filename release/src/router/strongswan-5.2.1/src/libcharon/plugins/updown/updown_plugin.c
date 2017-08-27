/*
 * Copyright (C) 2008 Martin Willi
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

#include "updown_plugin.h"
#include "updown_listener.h"
#include "updown_handler.h"

#include <daemon.h>
#include <hydra.h>

typedef struct private_updown_plugin_t private_updown_plugin_t;

/**
 * private data of updown plugin
 */
struct private_updown_plugin_t {

	/**
	 * implements plugin interface
	 */
	updown_plugin_t public;

	/**
	 * Listener interface, listens to CHILD_SA state changes
	 */
	updown_listener_t *listener;

	/**
	 * Attribute handler, to pass DNS servers to updown
	 */
	updown_handler_t *handler;
};

METHOD(plugin_t, get_name, char*,
	private_updown_plugin_t *this)
{
	return "updown";
}

/**
 * Register listener
 */
static bool plugin_cb(private_updown_plugin_t *this,
					  plugin_feature_t *feature, bool reg, void *cb_data)
{
	if (reg)
	{
		if (lib->settings->get_bool(lib->settings,
							"%s.plugins.updown.dns_handler", FALSE, lib->ns))
		{
			this->handler = updown_handler_create();
			hydra->attributes->add_handler(hydra->attributes,
										   &this->handler->handler);
		}
		this->listener = updown_listener_create(this->handler);
		charon->bus->add_listener(charon->bus, &this->listener->listener);
	}
	else
	{
		charon->bus->remove_listener(charon->bus, &this->listener->listener);
		this->listener->destroy(this->listener);
		if (this->handler)
		{
			this->handler->destroy(this->handler);
			hydra->attributes->remove_handler(hydra->attributes,
											  &this->handler->handler);
		}
	}
	return TRUE;
}

METHOD(plugin_t, get_features, int,
	private_updown_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_CALLBACK((plugin_feature_callback_t)plugin_cb, NULL),
			PLUGIN_PROVIDE(CUSTOM, "updown"),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_updown_plugin_t *this)
{
	free(this);
}

/*
 * see header file
 */
plugin_t *updown_plugin_create()
{
	private_updown_plugin_t *this;

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
