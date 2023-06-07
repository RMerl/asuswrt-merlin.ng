// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2006-2010  Nokia Corporation
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *  Copyright (C) 2005-2007  Johan Hedberg <johan.hedberg@nokia.com>
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdint.h>

#include <glib.h>
#include <dbus/dbus.h>

#include "gdbus/gdbus.h"

#include "log.h"

#include "dbus-common.h"

static DBusConnection *connection = NULL;

void dict_append_entry(DBusMessageIter *dict,
			const char *key, int type, void *val)
{
	g_dbus_dict_append_entry(dict, key, type, val);
}

void dict_append_array(DBusMessageIter *dict, const char *key, int type,
			void *val, int n_elements)
{
	g_dbus_dict_append_array(dict, key, type, val, n_elements);
}

void set_dbus_connection(DBusConnection *conn)
{
	connection = conn;
}

DBusConnection *btd_get_dbus_connection(void)
{
	return connection;
}

const char *class_to_icon(uint32_t class)
{
	switch ((class & 0x1f00) >> 8) {
	case 0x01:
		return "computer";
	case 0x02:
		switch ((class & 0xfc) >> 2) {
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x05:
			return "phone";
		case 0x04:
			return "modem";
		}
		break;
	case 0x03:
		return "network-wireless";
	case 0x04:
		switch ((class & 0xfc) >> 2) {
		case 0x01:
		case 0x02:
			return "audio-card";	/* Headset */
		case 0x06:
			return "audio-card";	/* Headphone */
		case 0x0b: /* VCR */
		case 0x0c: /* Video Camera */
		case 0x0d: /* Camcorder */
			return "camera-video";
		default:
			return "audio-card";	/* Other audio device */
		}
		break;
	case 0x05:
		switch ((class & 0xc0) >> 6) {
		case 0x00:
			switch ((class & 0x1e) >> 2) {
			case 0x01:
			case 0x02:
				return "input-gaming";
			}
			break;
		case 0x01:
			return "input-keyboard";
		case 0x02:
			switch ((class & 0x1e) >> 2) {
			case 0x05:
				return "input-tablet";
			default:
				return "input-mouse";
			}
		}
		break;
	case 0x06:
		if (class & 0x80)
			return "printer";
		if (class & 0x20)
			return "camera-photo";
		break;
	}

	return NULL;
}

const char *gap_appearance_to_icon(uint16_t appearance)
{
	switch ((appearance & 0xffc0) >> 6) {
	case 0x00:
		return "unknown";
	case 0x01:
		return "phone";
	case 0x02:
		return "computer";
	case 0x05:
		return "video-display";
	case 0x0a:
		return "multimedia-player";
	case 0x0b:
		return "scanner";
	case 0x0f: /* HID Generic */
		switch (appearance & 0x3f) {
		case 0x01:
			return "input-keyboard";
		case 0x02:
			return "input-mouse";
		case 0x03:
		case 0x04:
			return "input-gaming";
		case 0x05:
			return "input-tablet";
		case 0x08:
			return "scanner";
		}
		break;
	}

	return NULL;
}
