/*
 * Copyright (C) 2008 Martin Willi
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

#include "ha_plugin.h"
#include "ha_ike.h"
#include "ha_child.h"
#include "ha_socket.h"
#include "ha_tunnel.h"
#include "ha_dispatcher.h"
#include "ha_segments.h"
#include "ha_ctl.h"
#include "ha_cache.h"
#include "ha_attribute.h"

#include <daemon.h>
#include <config/child_cfg.h>

typedef struct private_ha_plugin_t private_ha_plugin_t;

/**
 * private data of ha plugin
 */
struct private_ha_plugin_t {

	/**
	 * implements plugin interface
	 */
	ha_plugin_t public;

	/**
	 * Communication socket
	 */
	ha_socket_t *socket;

	/**
	 * Tunnel securing sync messages.
	 */
	ha_tunnel_t *tunnel;

	/**
	 * IKE_SA synchronization
	 */
	ha_ike_t *ike;

	/**
	 * CHILD_SA synchronization
	 */
	ha_child_t *child;

	/**
	 * Dispatcher to process incoming messages
	 */
	ha_dispatcher_t *dispatcher;

	/**
	 * Active/Passive segment management
	 */
	ha_segments_t *segments;

	/**
	 * Interface to control segments at kernel level
	 */
	ha_kernel_t *kernel;

	/**
	 * Segment control interface via FIFO
	 */
	ha_ctl_t *ctl;

	/**
	 * Message cache for resynchronization
	 */
	ha_cache_t *cache;

	/**
	 * Attribute provider
	 */
	ha_attribute_t *attr;
};

METHOD(plugin_t, get_name, char*,
	private_ha_plugin_t *this)
{
	return "ha";
}

/**
 * Initialize plugin
 */
static bool initialize_plugin(private_ha_plugin_t *this)
{
	char *local, *remote, *secret;
	u_int count;
	bool fifo, monitor, resync;

	local = lib->settings->get_str(lib->settings,
								"%s.plugins.ha.local", NULL, lib->ns);
	remote = lib->settings->get_str(lib->settings,
								"%s.plugins.ha.remote", NULL, lib->ns);
	secret = lib->settings->get_str(lib->settings,
								"%s.plugins.ha.secret", NULL, lib->ns);
	fifo = lib->settings->get_bool(lib->settings,
								"%s.plugins.ha.fifo_interface", TRUE, lib->ns);
	monitor = lib->settings->get_bool(lib->settings,
								"%s.plugins.ha.monitor", TRUE, lib->ns);
	resync = lib->settings->get_bool(lib->settings,
								"%s.plugins.ha.resync", TRUE, lib->ns);
	count = min(SEGMENTS_MAX, lib->settings->get_int(lib->settings,
								"%s.plugins.ha.segment_count", 1, lib->ns));
	if (!local || !remote)
	{
		DBG1(DBG_CFG, "HA config misses local/remote address");
		return FALSE;
	}

	if (secret)
	{
		this->tunnel = ha_tunnel_create(local, remote, secret);
	}
	this->socket = ha_socket_create(local, remote);
	if (!this->socket)
	{
		return FALSE;
	}
	this->kernel = ha_kernel_create(count);
	this->segments = ha_segments_create(this->socket, this->kernel, this->tunnel,
							count, strcmp(local, remote) > 0, monitor);
	this->cache = ha_cache_create(this->kernel, this->socket, this->tunnel,
								  resync, count);
	if (fifo)
	{
		this->ctl = ha_ctl_create(this->segments, this->cache);
	}
	this->attr = ha_attribute_create(this->kernel, this->segments);
	this->dispatcher = ha_dispatcher_create(this->socket, this->segments,
										this->cache, this->kernel, this->attr);
	this->ike = ha_ike_create(this->socket, this->tunnel, this->cache);
	this->child = ha_child_create(this->socket, this->tunnel, this->segments,
								  this->kernel);
	return TRUE;
}

/**
 * Initialize plugin and register listener
 */
static bool plugin_cb(private_ha_plugin_t *this,
					  plugin_feature_t *feature, bool reg, void *cb_data)
{
	if (reg)
	{
		if (!initialize_plugin(this))
		{
			return FALSE;
		}
		charon->bus->add_listener(charon->bus, &this->segments->listener);
		charon->bus->add_listener(charon->bus, &this->ike->listener);
		charon->bus->add_listener(charon->bus, &this->child->listener);
		charon->attributes->add_provider(charon->attributes,
										 &this->attr->provider);
	}
	else
	{
		charon->attributes->remove_provider(charon->attributes,
											&this->attr->provider);
		charon->bus->remove_listener(charon->bus, &this->segments->listener);
		charon->bus->remove_listener(charon->bus, &this->ike->listener);
		charon->bus->remove_listener(charon->bus, &this->child->listener);
	}
	return TRUE;
}

METHOD(plugin_t, get_features, int,
	private_ha_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_CALLBACK((plugin_feature_callback_t)plugin_cb, NULL),
			PLUGIN_PROVIDE(CUSTOM, "ha"),
				PLUGIN_SDEPEND(CUSTOM, "kernel-ipsec"),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_ha_plugin_t *this)
{
	DESTROY_IF(this->ctl);
	DESTROY_IF(this->ike);
	DESTROY_IF(this->child);
	DESTROY_IF(this->dispatcher);
	DESTROY_IF(this->attr);
	DESTROY_IF(this->cache);
	DESTROY_IF(this->segments);
	DESTROY_IF(this->kernel);
	DESTROY_IF(this->socket);
	DESTROY_IF(this->tunnel);
	free(this);
}

/**
 * Plugin constructor
 */
plugin_t *ha_plugin_create()
{
	private_ha_plugin_t *this;

	if (!lib->caps->keep(lib->caps, CAP_CHOWN))
	{	/* required to chown(2) control socket, ha_kernel also needs it at
		 * runtime */
		DBG1(DBG_CFG, "ha plugin requires CAP_CHOWN capability");
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
