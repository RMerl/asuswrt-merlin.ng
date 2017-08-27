/*
 * Copyright (C) 2012 Tobias Brunner
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

#include "android_log_plugin.h"
#include "android_log_logger.h"

#include <daemon.h>

typedef struct private_android_log_plugin_t private_android_log_plugin_t;

/**
 * Private data of an android_log_plugin_t object.
 */
struct private_android_log_plugin_t {

	/**
	 * Public android_log_plugin_t interface.
	 */
	android_log_plugin_t public;

	/**
	 * Android specific logger
	 */
	android_log_logger_t *logger;

};

METHOD(plugin_t, get_name, char*,
	private_android_log_plugin_t *this)
{
	return "android-log";
}

METHOD(plugin_t, get_features, int,
	private_android_log_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_NOOP,
			PLUGIN_PROVIDE(CUSTOM, "android-log"),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_android_log_plugin_t *this)
{
	charon->bus->remove_logger(charon->bus, &this->logger->logger);
	this->logger->destroy(this->logger);
	free(this);
}

/**
 * See header
 */
plugin_t *android_log_plugin_create()
{
	private_android_log_plugin_t *this;

	INIT(this,
		.public = {
			.plugin = {
				.get_name = _get_name,
				.get_features = _get_features,
				.destroy = _destroy,
			},
		},
		.logger = android_log_logger_create(),
	);

	charon->bus->add_logger(charon->bus, &this->logger->logger);

	return &this->public.plugin;
}
