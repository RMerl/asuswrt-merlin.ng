/*
 * Copyright (C) 2008-2009 Martin Willi
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

/**
 * @defgroup nm_service nm_service
 * @{ @ingroup nm
 */

#ifndef NM_SERVICE_H_
#define NM_SERVICE_H_

#include <glib.h>
#include <glib-object.h>
#include <NetworkManager.h>

#include "nm_creds.h"
#include "nm_handler.h"

#define NM_TYPE_STRONGSWAN_PLUGIN            (nm_strongswan_plugin_get_type ())
#define NM_STRONGSWAN_PLUGIN(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), NM_TYPE_STRONGSWAN_PLUGIN, NMStrongswanPlugin))
#define NM_STRONGSWAN_PLUGIN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), NM_TYPE_STRONGSWAN_PLUGIN, NMStrongswanPluginClass))
#define NM_IS_STRONGSWAN_PLUGIN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NM_TYPE_STRONGSWAN_PLUGIN))
#define NM_IS_STRONGSWAN_PLUGIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((obj), NM_TYPE_STRONGSWAN_PLUGIN))
#define NM_STRONGSWAN_PLUGIN_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), NM_TYPE_STRONGSWAN_PLUGIN, NMStrongswanPluginClass))

#define NM_DBUS_SERVICE_STRONGSWAN    "org.freedesktop.NetworkManager.strongswan"
#define NM_DBUS_INTERFACE_STRONGSWAN  "org.freedesktop.NetworkManager.strongswan"
#define NM_DBUS_PATH_STRONGSWAN       "/org/freedesktop/NetworkManager/strongswan"

typedef struct {
	NMVpnServicePlugin parent;
} NMStrongswanPlugin;

typedef struct {
	NMVpnServicePluginClass parent;
} NMStrongswanPluginClass;

GType nm_strongswan_plugin_get_type(void);

NMStrongswanPlugin *nm_strongswan_plugin_new(nm_creds_t *creds,
											 nm_handler_t *handler);

#endif /** NM_SERVICE_H_ @}*/
