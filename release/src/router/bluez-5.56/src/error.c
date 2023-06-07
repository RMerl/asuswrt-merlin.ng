// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2006-2010  Nokia Corporation
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *  Copyright (C) 2007-2008  Fabien Chevalier <fabchevalier@free.fr>
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gdbus/gdbus.h"

#include "error.h"

DBusMessage *btd_error_invalid_args(DBusMessage *msg)
{
	return btd_error_invalid_args_str(msg,
					"Invalid arguments in method call");
}

DBusMessage *btd_error_invalid_args_str(DBusMessage *msg, const char *str)
{
	return g_dbus_create_error(msg, ERROR_INTERFACE ".InvalidArguments",
					"%s", str);
}

DBusMessage *btd_error_busy(DBusMessage *msg)
{
	return g_dbus_create_error(msg, ERROR_INTERFACE ".InProgress",
					"Operation already in progress");
}

DBusMessage *btd_error_already_exists(DBusMessage *msg)
{
	return g_dbus_create_error(msg, ERROR_INTERFACE ".AlreadyExists",
					"Already Exists");
}

DBusMessage *btd_error_not_supported(DBusMessage *msg)
{
	return g_dbus_create_error(msg, ERROR_INTERFACE ".NotSupported",
					"Operation is not supported");
}

DBusMessage *btd_error_not_connected(DBusMessage *msg)
{
	return g_dbus_create_error(msg, ERROR_INTERFACE ".NotConnected",
					"Not Connected");
}

DBusMessage *btd_error_already_connected(DBusMessage *msg)
{
	return g_dbus_create_error(msg, ERROR_INTERFACE ".AlreadyConnected",
					"Already Connected");
}

DBusMessage *btd_error_in_progress(DBusMessage *msg)
{
	return g_dbus_create_error(msg, ERROR_INTERFACE ".InProgress",
					"In Progress");
}

DBusMessage *btd_error_not_available(DBusMessage *msg)
{
	return g_dbus_create_error(msg, ERROR_INTERFACE ".NotAvailable",
					"Operation currently not available");
}

DBusMessage *btd_error_does_not_exist(DBusMessage *msg)
{
	return g_dbus_create_error(msg, ERROR_INTERFACE ".DoesNotExist",
					"Does Not Exist");
}

DBusMessage *btd_error_not_authorized(DBusMessage *msg)
{
	return g_dbus_create_error(msg, ERROR_INTERFACE ".NotAuthorized",
						"Operation Not Authorized");
}

DBusMessage *btd_error_not_permitted(DBusMessage *msg, const char *str)
{
	return g_dbus_create_error(msg, ERROR_INTERFACE ".NotPermitted",
					"%s", str);
}

DBusMessage *btd_error_no_such_adapter(DBusMessage *msg)
{
	return g_dbus_create_error(msg, ERROR_INTERFACE ".NoSuchAdapter",
					"No such adapter");
}

DBusMessage *btd_error_agent_not_available(DBusMessage *msg)
{
	return g_dbus_create_error(msg, ERROR_INTERFACE ".AgentNotAvailable",
					"Agent Not Available");
}

DBusMessage *btd_error_not_ready(DBusMessage *msg)
{
	return g_dbus_create_error(msg, ERROR_INTERFACE ".NotReady",
					"Resource Not Ready");
}

DBusMessage *btd_error_failed(DBusMessage *msg, const char *str)
{
	return g_dbus_create_error(msg, ERROR_INTERFACE
					".Failed", "%s", str);
}
