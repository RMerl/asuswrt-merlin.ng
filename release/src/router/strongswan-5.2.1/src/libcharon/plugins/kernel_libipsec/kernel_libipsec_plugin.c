/*
 * Copyright (C) 2012-2013 Tobias Brunner
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

#include "kernel_libipsec_plugin.h"
#include "kernel_libipsec_ipsec.h"
#include "kernel_libipsec_router.h"

#include <daemon.h>
#include <ipsec.h>
#include <networking/tun_device.h>

#define TUN_DEFAULT_MTU 1400

typedef struct private_kernel_libipsec_plugin_t private_kernel_libipsec_plugin_t;

/**
 * private data of "kernel" libipsec plugin
 */
struct private_kernel_libipsec_plugin_t {

	/**
	 * implements plugin interface
	 */
	kernel_libipsec_plugin_t public;

	/**
	 * TUN device created by this plugin
	 */
	tun_device_t *tun;

	/**
	 * Packet router
	 */
	kernel_libipsec_router_t *router;
};

METHOD(plugin_t, get_name, char*,
	private_kernel_libipsec_plugin_t *this)
{
	return "kernel-libipsec";
}

/**
 * Create the kernel_libipsec_router_t instance
 */
static bool create_router(private_kernel_libipsec_plugin_t *this,
						  plugin_feature_t *feature, bool reg, void *arg)
{
	if (reg)
	{	/* registers as packet handler etc. */
		this->router = kernel_libipsec_router_create();
	}
	else
	{
		DESTROY_IF(this->router);
	}
	return TRUE;
}

METHOD(plugin_t, get_features, int,
	private_kernel_libipsec_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_CALLBACK(kernel_ipsec_register, kernel_libipsec_ipsec_create),
			PLUGIN_PROVIDE(CUSTOM, "kernel-ipsec"),
		PLUGIN_CALLBACK((plugin_feature_callback_t)create_router, NULL),
			PLUGIN_PROVIDE(CUSTOM, "kernel-libipsec-router"),
				PLUGIN_DEPENDS(CUSTOM, "libcharon-receiver"),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_kernel_libipsec_plugin_t *this)
{
	if (this->tun)
	{
		lib->set(lib, "kernel-libipsec-tun", NULL);
		this->tun->destroy(this->tun);
	}
	libipsec_deinit();
	free(this);
}

/*
 * see header file
 */
plugin_t *kernel_libipsec_plugin_create()
{
	private_kernel_libipsec_plugin_t *this;

	if (!lib->caps->check(lib->caps, CAP_NET_ADMIN))
	{	/* required to create TUN devices */
		DBG1(DBG_KNL, "kernel-libipsec plugin requires CAP_NET_ADMIN "
			 "capability");
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

	if (!libipsec_init())
	{
		DBG1(DBG_LIB, "initialization of libipsec failed");
		destroy(this);
		return NULL;
	}

	this->tun = tun_device_create("ipsec%d");
	if (!this->tun)
	{
		DBG1(DBG_KNL, "failed to create TUN device");
		destroy(this);
		return NULL;
	}
	if (!this->tun->set_mtu(this->tun, TUN_DEFAULT_MTU) ||
		!this->tun->up(this->tun))
	{
		DBG1(DBG_KNL, "failed to configure TUN device");
		destroy(this);
		return NULL;
	}
	lib->set(lib, "kernel-libipsec-tun", this->tun);

	/* set TUN device as default to install VIPs */
	lib->settings->set_str(lib->settings, "%s.install_virtual_ip_on",
						   this->tun->get_name(this->tun), lib->ns);
	return &this->public.plugin;
}
