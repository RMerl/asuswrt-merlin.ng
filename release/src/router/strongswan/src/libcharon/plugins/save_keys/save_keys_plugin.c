/*
 * Copyright (C) 2016 Codrut Cristian Grosu (codrut.cristian.grosu@gmail.com)
 * Copyright (C) 2016 IXIA (http://www.ixiacom.com)
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

#include "save_keys_plugin.h"
#include "save_keys_listener.h"

#include <daemon.h>

typedef struct private_save_keys_plugin_t private_save_keys_plugin_t;

/**
 * Private data.
 */
struct private_save_keys_plugin_t {

	/**
	 * Implements plugin interface.
	 */
	save_keys_plugin_t public;

	/**
	 * Listener saving keys to file.
	 */
	save_keys_listener_t *listener;
};

METHOD(plugin_t, get_name, char*,
	private_save_keys_plugin_t *this)
{
	return "save-keys";
}

/**
 * Register listener.
 */
static bool plugin_cb(private_save_keys_plugin_t *this,
					plugin_feature_t *feature, bool reg, void *cb_data)
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

METHOD(plugin_t, get_features, int,
	private_save_keys_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_CALLBACK((plugin_feature_callback_t)plugin_cb, NULL),
			PLUGIN_PROVIDE(CUSTOM, "save-keys"),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_save_keys_plugin_t *this)
{
	this->listener->destroy(this->listener);
	free(this);
}

/**
 * Plugin constructor.
 */
plugin_t *save_keys_plugin_create()
{
	private_save_keys_plugin_t *this;

	INIT(this,
		.public = {
			.plugin = {
				.get_name = _get_name,
				.get_features = _get_features,
				.destroy = _destroy,
			},
		},
		.listener = save_keys_listener_create(),
	);

	return &this->public.plugin;
}
