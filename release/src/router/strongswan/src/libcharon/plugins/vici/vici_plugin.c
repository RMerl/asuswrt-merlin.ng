/*
 * Copyright (C) 2014 Martin Willi
 * Copyright (C) 2014 revosec AG
 *
 * Copyright (C) 2015-2016 Andreas Steffen
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

/*
 * Copyright (C) 2014 Timo Teräs <timo.teras@iki.fi>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "vici_plugin.h"
#include "vici_dispatcher.h"
#include "vici_query.h"
#include "vici_control.h"
#include "vici_cred.h"
#include "vici_config.h"
#include "vici_attribute.h"
#include "vici_authority.h"
#include "vici_logger.h"

#include <library.h>
#include <daemon.h>

typedef struct private_vici_plugin_t private_vici_plugin_t;

/**
 * Private members of vici_plugin_t
 */
struct private_vici_plugin_t {

	/**
	 * public functions
	 */
	vici_plugin_t public;

	/**
	 * Dispatcher, creating socket
	 */
	vici_dispatcher_t *dispatcher;

	/**
	 * Query commands
	 */
	vici_query_t *query;

	/**
	 * Control commands
	 */
	vici_control_t *control;

	/**
	 * Credential backend
	 */
	vici_cred_t *cred;

	/**
	 * Certification Authority backend
	 */
	vici_authority_t *authority;

	/**
	 * Configuration backend
	 */
	vici_config_t *config;

	/**
	 * IKE attribute backend
	 */
	vici_attribute_t *attrs;

	/**
	 * Generic debug logger
	 */
	vici_logger_t *logger;
};

METHOD(plugin_t, get_name, char*,
	private_vici_plugin_t *this)
{
	return "vici";
}

/**
 * Register vici plugin features
 */
static bool register_vici(private_vici_plugin_t *this,
						  plugin_feature_t *feature, bool reg, void *data)
{
	if (reg)
	{
		char *uri;

		uri = lib->settings->get_str(lib->settings, "%s.plugins.vici.socket",
									 VICI_DEFAULT_URI, lib->ns);
		this->dispatcher = vici_dispatcher_create(uri);
		if (this->dispatcher)
		{
			this->query = vici_query_create(this->dispatcher);
			this->control = vici_control_create(this->dispatcher);
			this->cred = vici_cred_create(this->dispatcher);
			this->authority = vici_authority_create(this->dispatcher,
													this->cred);
			lib->credmgr->add_set(lib->credmgr, &this->cred->set);
			lib->credmgr->add_set(lib->credmgr, &this->authority->set);
			this->config = vici_config_create(this->dispatcher, this->authority,
											  this->cred);
			this->attrs = vici_attribute_create(this->dispatcher);
			this->logger = vici_logger_create(this->dispatcher);

			charon->backends->add_backend(charon->backends,
										  &this->config->backend);
			charon->attributes->add_provider(charon->attributes,
											 &this->attrs->provider);
			charon->bus->add_logger(charon->bus, &this->logger->logger);
			charon->bus->add_listener(charon->bus, &this->query->listener);
			return TRUE;
		}
		return FALSE;
	}
	else
	{
		charon->bus->remove_listener(charon->bus, &this->query->listener);
		charon->bus->remove_logger(charon->bus, &this->logger->logger);
		charon->attributes->remove_provider(charon->attributes,
											&this->attrs->provider);
		charon->backends->remove_backend(charon->backends,
										 &this->config->backend);

		this->logger->destroy(this->logger);
		this->attrs->destroy(this->attrs);
		this->config->destroy(this->config);
		lib->credmgr->remove_set(lib->credmgr, &this->cred->set);
		lib->credmgr->remove_set(lib->credmgr, &this->authority->set);
		this->authority->destroy(this->authority);
		this->cred->destroy(this->cred);
		this->control->destroy(this->control);
		this->query->destroy(this->query);
		this->dispatcher->destroy(this->dispatcher);
	}
	return TRUE;
}

METHOD(plugin_t, get_features, int,
	private_vici_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_CALLBACK((plugin_feature_callback_t)register_vici, NULL),
			PLUGIN_PROVIDE(CUSTOM, "vici"),
				PLUGIN_SDEPEND(CUSTOM, "counters"),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_vici_plugin_t *this)
{
	free(this);
}

/*
 * see header file
 */
plugin_t *vici_plugin_create()
{
	private_vici_plugin_t *this;

	INIT(this,
		.public = {
			.plugin = {
				.get_name = _get_name,
				.reload = (void*)return_false,
				.get_features = _get_features,
				.destroy = _destroy,
			},
		},
	);

	return &this->public.plugin;
}
