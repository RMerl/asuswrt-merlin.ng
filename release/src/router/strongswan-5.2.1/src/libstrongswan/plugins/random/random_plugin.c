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

#include "random_plugin.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <library.h>
#include <utils/debug.h>
#include "random_rng.h"

#ifndef DEV_RANDOM
# define DEV_RANDOM "/dev/random"
#endif

#ifndef DEV_URANDOM
# define DEV_URANDOM "/dev/urandom"
#endif

typedef struct private_random_plugin_t private_random_plugin_t;

/**
 * private data of random_plugin
 */
struct private_random_plugin_t {

	/**
	 * public functions
	 */
	random_plugin_t public;
};

/** /dev/random file descriptor */
static int dev_random = -1;
/** /dev/urandom file descriptor */
static int dev_urandom = -1;

/** Is strong randomness equivalent to true randomness? */
static bool strong_equals_true = FALSE;

/**
 * See header.
 */
int random_plugin_get_dev_random()
{
	return dev_random;
}

/**
 * See header.
 */
int random_plugin_get_dev_urandom()
{
	return dev_urandom;
}

/**
 * See header.
 */
bool random_plugin_get_strong_equals_true()
{
	return strong_equals_true;
}

/**
 * Open a random device file
 */
static bool open_dev(char *file, int *fd)
{
	*fd = open(file, O_RDONLY);
	if (*fd == -1)
	{
		DBG1(DBG_LIB, "opening \"%s\" failed: %s", file, strerror(errno));
		return FALSE;
	}
	if (fcntl(*fd, F_SETFD, FD_CLOEXEC) == -1)
	{
		DBG1(DBG_LIB, "setting FD_CLOEXEC for \"%s\" failed: %s",
			 file, strerror(errno));
	}
	return TRUE;
}

METHOD(plugin_t, get_name, char*,
	private_random_plugin_t *this)
{
	return "random";
}

METHOD(plugin_t, get_features, int,
	private_random_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_REGISTER(RNG, random_rng_create),
			PLUGIN_PROVIDE(RNG, RNG_STRONG),
			PLUGIN_PROVIDE(RNG, RNG_TRUE),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_random_plugin_t *this)
{
	if (dev_random != -1)
	{
		close(dev_random);
	}
	if (dev_urandom != -1)
	{
		close(dev_urandom);
	}
	free(this);
}

/*
 * see header file
 */
plugin_t *random_plugin_create()
{
	private_random_plugin_t *this;
	char *urandom_file, *random_file;

	INIT(this,
		.public = {
			.plugin = {
				.get_name = _get_name,
				.get_features = _get_features,
				.destroy = _destroy,
			},
		},
	);

	strong_equals_true = lib->settings->get_bool(lib->settings,
						"%s.plugins.random.strong_equals_true", FALSE, lib->ns);
	urandom_file = lib->settings->get_str(lib->settings,
						"%s.plugins.random.urandom", DEV_URANDOM, lib->ns);
	random_file = lib->settings->get_str(lib->settings,
						"%s.plugins.random.random", DEV_RANDOM, lib->ns);
	if (!open_dev(urandom_file, &dev_urandom) ||
		!open_dev(random_file, &dev_random))
	{
		destroy(this);
		return NULL;
	}

	return &this->public.plugin;
}

