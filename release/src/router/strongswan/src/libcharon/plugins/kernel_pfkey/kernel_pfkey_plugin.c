/*
 * Copyright (C) 2008 Tobias Brunner
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


#include "kernel_pfkey_plugin.h"

#include "kernel_pfkey_ipsec.h"

typedef struct private_kernel_pfkey_plugin_t private_kernel_pfkey_plugin_t;

/**
 * private data of kernel PF_KEY plugin
 */
struct private_kernel_pfkey_plugin_t {
	/**
	 * implements plugin interface
	 */
	kernel_pfkey_plugin_t public;
};

METHOD(plugin_t, get_name, char*,
	private_kernel_pfkey_plugin_t *this)
{
	return "kernel-pfkey";
}

METHOD(plugin_t, get_features, int,
	private_kernel_pfkey_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_CALLBACK(kernel_ipsec_register, kernel_pfkey_ipsec_create),
			PLUGIN_PROVIDE(CUSTOM, "kernel-ipsec"),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_kernel_pfkey_plugin_t *this)
{
	free(this);
}

/*
 * see header file
 */
plugin_t *kernel_pfkey_plugin_create()
{
	private_kernel_pfkey_plugin_t *this;

	if (!lib->caps->check(lib->caps, CAP_NET_ADMIN))
	{	/* required to open PF_KEY sockets */
		DBG1(DBG_KNL, "kernel-pfkey plugin requires CAP_NET_ADMIN capability");
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
