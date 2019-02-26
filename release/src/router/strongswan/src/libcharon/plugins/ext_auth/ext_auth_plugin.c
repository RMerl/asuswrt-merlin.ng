/*
 * Copyright (c) 2014 Vyronas Tsingaras (vtsingaras@it.auth.gr)
 * Copyright (C) 2014 Martin Willi
 * Copyright (C) 2014 revosec AG
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

#include "ext_auth_plugin.h"
#include "ext_auth_listener.h"

#include <daemon.h>

typedef struct private_ext_auth_plugin_t private_ext_auth_plugin_t;

/**
 * private data of ext_auth plugin
 */
struct private_ext_auth_plugin_t {

	/**
	 * implements plugin interface
	 */
	ext_auth_plugin_t public;

	/**
	 * Listener verifying peers during authorization
	 */
	ext_auth_listener_t *listener;
};

METHOD(plugin_t, get_name, char*,
	private_ext_auth_plugin_t *this)
{
	return "ext-auth";
}

/**
 * Create a listener instance, NULL on error
 */
static ext_auth_listener_t* create_listener()
{
	char *script;

	script = lib->settings->get_str(lib->settings,
					"%s.plugins.ext-auth.script", NULL, lib->ns);
	if (!script)
	{
		DBG1(DBG_CFG, "no script for ext-auth script defined, disabled");
		return NULL;
	}
	DBG1(DBG_CFG, "using ext-auth script '%s'", script);
	return ext_auth_listener_create(script);
}

/**
 * Register listener
 */
static bool plugin_cb(private_ext_auth_plugin_t *this,
					  plugin_feature_t *feature, bool reg, void *cb_data)
{
	if (reg)
	{
		this->listener = create_listener();
		if (!this->listener)
		{
			return FALSE;
		}
		charon->bus->add_listener(charon->bus, &this->listener->listener);
	}
	else
	{
		if (this->listener)
		{
			charon->bus->remove_listener(charon->bus, &this->listener->listener);
			this->listener->destroy(this->listener);
		}
	}
	return TRUE;
}

METHOD(plugin_t, get_features, int,
	private_ext_auth_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_CALLBACK((plugin_feature_callback_t)plugin_cb, NULL),
			PLUGIN_PROVIDE(CUSTOM, "ext_auth"),
	};
	*features = f;
	return countof(f);
}


METHOD(plugin_t, reload, bool,
	private_ext_auth_plugin_t *this)
{
	ext_auth_listener_t *listener;

	/* reload new listener overlapped */
	listener = create_listener();
	if (listener)
	{
		charon->bus->add_listener(charon->bus, &listener->listener);
	}
	if (this->listener)
	{
		charon->bus->remove_listener(charon->bus, &this->listener->listener);
		this->listener->destroy(this->listener);
	}
	this->listener = listener;

	return TRUE;
}

METHOD(plugin_t, destroy, void,
	private_ext_auth_plugin_t *this)
{
	free(this);
}

/**
 * Plugin constructor
 */
plugin_t *ext_auth_plugin_create()
{
	private_ext_auth_plugin_t *this;

	INIT(this,
		.public = {
			.plugin = {
				.get_name = _get_name,
				.get_features = _get_features,
				.reload = _reload,
				.destroy = _destroy,
			},
		},
	);

	return &this->public.plugin;
}
