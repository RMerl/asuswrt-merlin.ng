/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2006-2010  Nokia Corporation
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *  Copyright (C) 2007-2008  Fabien Chevalier <fabchevalier@free.fr>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <dbus/dbus.h>

#define ERROR_INTERFACE "org.bluez.Error"

DBusMessage *btd_error_invalid_args(DBusMessage *msg);
DBusMessage *btd_error_invalid_args_str(DBusMessage *msg, const char *str);
DBusMessage *btd_error_busy(DBusMessage *msg);
DBusMessage *btd_error_already_exists(DBusMessage *msg);
DBusMessage *btd_error_not_supported(DBusMessage *msg);
DBusMessage *btd_error_not_connected(DBusMessage *msg);
DBusMessage *btd_error_already_connected(DBusMessage *msg);
DBusMessage *btd_error_not_available(DBusMessage *msg);
DBusMessage *btd_error_in_progress(DBusMessage *msg);
DBusMessage *btd_error_does_not_exist(DBusMessage *msg);
DBusMessage *btd_error_not_authorized(DBusMessage *msg);
DBusMessage *btd_error_not_permitted(DBusMessage *msg, const char *str);
DBusMessage *btd_error_no_such_adapter(DBusMessage *msg);
DBusMessage *btd_error_agent_not_available(DBusMessage *msg);
DBusMessage *btd_error_not_ready(DBusMessage *msg);
DBusMessage *btd_error_failed(DBusMessage *msg, const char *str);
