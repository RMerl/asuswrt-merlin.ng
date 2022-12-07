/*
 * preflight.c
 *
 * Copyright (C) 2013 Nikias Bassen <nikias@gmx.li>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 or version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <pthread.h>

#include <sys/time.h>

#ifdef HAVE_LIBIMOBILEDEVICE
#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>
#include <libimobiledevice/notification_proxy.h>
#endif

#include "preflight.h"
#include "device.h"
#include "client.h"
#include "conf.h"
#include "log.h"
#include "usb.h"

extern int no_preflight;

#ifdef HAVE_LIBIMOBILEDEVICE
#ifndef HAVE_ENUM_IDEVICE_CONNECTION_TYPE
enum idevice_connection_type {
	CONNECTION_USBMUXD = 1,
	CONNECTION_NETWORK
};
#endif

struct idevice_private {
	char *udid;
	uint32_t mux_id;
	enum idevice_connection_type conn_type;
	void *conn_data;
	int version;
};

struct cb_data {
	idevice_t dev;
	np_client_t np;
	int is_device_connected;
	int is_finished;
};

static void lockdownd_set_untrusted_host_buid(lockdownd_client_t lockdown)
{
	char* system_buid = NULL;
	config_get_system_buid(&system_buid);
	usbmuxd_log(LL_DEBUG, "%s: Setting UntrustedHostBUID to %s", __func__, system_buid);
	lockdownd_set_value(lockdown, NULL, "UntrustedHostBUID", plist_new_string(system_buid));
	free(system_buid);
}

void preflight_device_remove_cb(void *data)
{
	if (!data)
		return;
	struct cb_data *cbdata = (struct cb_data*)data;
	cbdata->is_device_connected = 0;
}

static void np_callback(const char* notification, void* userdata)
{
	struct cb_data *cbdata = (struct cb_data*)userdata;
	idevice_t dev = cbdata->dev;
	struct idevice_private *_dev = (struct idevice_private*)dev;

	lockdownd_client_t lockdown = NULL;
	lockdownd_error_t lerr;

	if (strlen(notification) == 0) {
		cbdata->np = NULL;
		return;
	}

	if (strcmp(notification, "com.apple.mobile.lockdown.request_pair") == 0) {
		usbmuxd_log(LL_INFO, "%s: user trusted this computer on device %s, pairing now", __func__, _dev->udid);
		lerr = lockdownd_client_new(dev, &lockdown, "usbmuxd");
		if (lerr != LOCKDOWN_E_SUCCESS) {
			usbmuxd_log(LL_ERROR, "%s: ERROR: Could not connect to lockdownd on device %s, lockdown error %d", __func__, _dev->udid, lerr);
			cbdata->is_finished = 1;
			return;
		}

		lerr = lockdownd_pair(lockdown, NULL);
		if (lerr != LOCKDOWN_E_SUCCESS) {
			usbmuxd_log(LL_ERROR, "%s: ERROR: Pair failed for device %s, lockdown error %d", __func__, _dev->udid, lerr);
			lockdownd_client_free(lockdown);
			cbdata->is_finished = 1;
			return;
		}
		lockdownd_client_free(lockdown);
		cbdata->is_finished = 1;

	} else if (strcmp(notification, "com.apple.mobile.lockdown.request_host_buid") == 0) {
		lerr = lockdownd_client_new(cbdata->dev, &lockdown, "usbmuxd");
		if (lerr != LOCKDOWN_E_SUCCESS) {
			usbmuxd_log(LL_ERROR, "%s: ERROR: Could not connect to lockdownd on device %s, lockdown error %d", __func__, _dev->udid, lerr);
		} else {
			lockdownd_set_untrusted_host_buid(lockdown);
			lockdownd_client_free(lockdown);
		}
	}
}

static void* preflight_worker_handle_device_add(void* userdata)
{
	struct device_info *info = (struct device_info*)userdata;
	struct idevice_private *_dev = (struct idevice_private*)malloc(sizeof(struct idevice_private));
	_dev->udid = strdup(info->serial);
	_dev->mux_id = info->id;
	_dev->conn_type = CONNECTION_USBMUXD;
	_dev->conn_data = NULL;
	_dev->version = 0;

	idevice_t dev = (idevice_t)_dev;

	lockdownd_client_t lockdown = NULL;
	lockdownd_error_t lerr;

	plist_t value = NULL;
	char* version_str = NULL;

	usbmuxd_log(LL_INFO, "%s: Starting preflight on device %s...", __func__, _dev->udid);

retry:
	lerr = lockdownd_client_new(dev, &lockdown, "usbmuxd");
	if (lerr != LOCKDOWN_E_SUCCESS) {
		usbmuxd_log(LL_ERROR, "%s: ERROR: Could not connect to lockdownd on device %s, lockdown error %d", __func__, _dev->udid, lerr);
		goto leave;
	}

	char *type = NULL;
	lerr = lockdownd_query_type(lockdown, &type);
	if (!type) {
		usbmuxd_log(LL_ERROR, "%s: ERROR: Could not get lockdownd type from device %s, lockdown error %d", __func__, _dev->udid, lerr);
		goto leave;
	}

	if (strcmp(type, "com.apple.mobile.lockdown") != 0) {
		// make restore mode devices visible
		free(type);
		usbmuxd_log(LL_INFO, "%s: Finished preflight on device %s", __func__, _dev->udid);
		client_device_add(info);
		goto leave;
	}
	free(type);

	int is_device_paired = 0;
	char *host_id = NULL;
	if (config_has_device_record(dev->udid)) {
		config_device_record_get_host_id(dev->udid, &host_id);
		lerr = lockdownd_start_session(lockdown, host_id, NULL, NULL);
		if (host_id)
			free(host_id);
		if (lerr == LOCKDOWN_E_SUCCESS) {
			usbmuxd_log(LL_INFO, "%s: StartSession success for device %s", __func__, _dev->udid);
			usbmuxd_log(LL_INFO, "%s: Finished preflight on device %s", __func__, _dev->udid);
			client_device_add(info);
			goto leave;
		}

		usbmuxd_log(LL_INFO, "%s: StartSession failed on device %s, lockdown error %d", __func__, _dev->udid, lerr);
	} else {
		lerr = LOCKDOWN_E_INVALID_HOST_ID;
	}
	switch (lerr) {
	case LOCKDOWN_E_INVALID_HOST_ID:
		usbmuxd_log(LL_INFO, "%s: Device %s is not paired with this host.", __func__, _dev->udid);
		break;
	case LOCKDOWN_E_SSL_ERROR:
		usbmuxd_log(LL_ERROR, "%s: The stored pair record for device %s is invalid. Removing.", __func__, _dev->udid);
		if (config_remove_device_record(_dev->udid) == 0) {
			lockdownd_client_free(lockdown);
			lockdown = NULL;
			goto retry;
		} else {
			usbmuxd_log(LL_ERROR, "%s: Could not remove pair record for device %s", __func__, _dev->udid);
		}
		break;
	default:
		is_device_paired = 1;
		break;
	}

	lerr = lockdownd_get_value(lockdown, NULL, "ProductVersion", &value);
	if (lerr != LOCKDOWN_E_SUCCESS) {
		usbmuxd_log(LL_ERROR, "%s: ERROR: Could not get ProductVersion from device %s, lockdown error %d", __func__, _dev->udid, lerr);
		goto leave;
	}

	if (value && plist_get_node_type(value) == PLIST_STRING) {
		plist_get_string_val(value, &version_str);
	}

	if (!version_str) {
		usbmuxd_log(LL_ERROR, "%s: Could not get ProductVersion string from device %s handle %d", __func__, _dev->udid, (int)(long)_dev->conn_data);
		goto leave;
	}

	int version_major = strtol(version_str, NULL, 10);
	if (version_major >= 7) {
		/* iOS 7.0 and later */
		usbmuxd_log(LL_INFO, "%s: Found ProductVersion %s device %s", __func__, version_str, _dev->udid);

		lockdownd_set_untrusted_host_buid(lockdown);

		/* if not paired, trigger the trust dialog to make sure it appears */
		if (!is_device_paired) {
			if (lockdownd_pair(lockdown, NULL) == LOCKDOWN_E_SUCCESS) {
				/* if device is still showing the setup screen it will pair even without trust dialog */
				usbmuxd_log(LL_INFO, "%s: Pair success for device %s", __func__, _dev->udid);
				usbmuxd_log(LL_INFO, "%s: Finished preflight on device %s", __func__, _dev->udid);
				client_device_add(info);
				goto leave;
			}
		}

		lockdownd_service_descriptor_t service = NULL;
		lerr = lockdownd_start_service(lockdown, "com.apple.mobile.insecure_notification_proxy", &service);
		if (lerr != LOCKDOWN_E_SUCCESS) {
			/* even though we failed, simple mode should still work, so only warn of an error */
			usbmuxd_log(LL_INFO, "%s: ERROR: Could not start insecure_notification_proxy on %s, lockdown error %d", __func__, _dev->udid, lerr);
			client_device_add(info);
			goto leave;
		}

		np_client_t np = NULL;
		np_client_new(dev, service, &np);

		lockdownd_service_descriptor_free(service);
		service = NULL;

		lockdownd_client_free(lockdown);
		lockdown = NULL;

		struct cb_data cbdata;
		cbdata.dev = dev;
		cbdata.np = np;
		cbdata.is_device_connected = 1;
		cbdata.is_finished = 0;

		np_set_notify_callback(np, np_callback, (void*)&cbdata);
		device_set_preflight_cb_data(info->id, (void*)&cbdata);

		const char* spec[] = {
			"com.apple.mobile.lockdown.request_pair",
			"com.apple.mobile.lockdown.request_host_buid",
			NULL
		};
		np_observe_notifications(np, spec);

		/* TODO send notification to user's desktop */

		usbmuxd_log(LL_INFO, "%s: Waiting for user to trust this computer on device %s", __func__, _dev->udid);

		/* make device visible anyways */
		client_device_add(info);

		while (cbdata.np && cbdata.is_device_connected && !cbdata.is_finished) {
			sleep(1);
		}
		device_set_preflight_cb_data(info->id, NULL);

		usbmuxd_log(LL_INFO, "%s: Finished waiting for notification from device %s, is_device_connected %d", __func__, _dev->udid, cbdata.is_device_connected);

		if (cbdata.np) {
			np_client_free(cbdata.np);
		}
	} else {
		/* iOS 6.x and earlier */
		lerr = lockdownd_pair(lockdown, NULL);
		if (lerr != LOCKDOWN_E_SUCCESS) {
			if (lerr == LOCKDOWN_E_PASSWORD_PROTECTED) {
				usbmuxd_log(LL_INFO, "%s: Device %s is locked with a passcode. Cannot pair.", __func__, _dev->udid);
				/* TODO send notification to user's desktop */
			} else {
				usbmuxd_log(LL_ERROR, "%s: ERROR: Pair failed for device %s, lockdown error %d", __func__, _dev->udid, lerr);
			}

			usbmuxd_log(LL_INFO, "%s: Finished preflight on device %s", __func__, _dev->udid);

			/* make device visible anyways */
			client_device_add(info);

			goto leave;
		}

		host_id = NULL;
		config_device_record_get_host_id(dev->udid, &host_id);
		lerr = lockdownd_start_session(lockdown, host_id, NULL, NULL);
		free(host_id);
		if (lerr != LOCKDOWN_E_SUCCESS) {
			usbmuxd_log(LL_ERROR, "%s: ERROR StartSession failed on device %s, lockdown error %d", __func__, _dev->udid, lerr);
			goto leave;
		}

		lerr = lockdownd_validate_pair(lockdown, NULL);
		if (lerr != LOCKDOWN_E_SUCCESS) {
			usbmuxd_log(LL_ERROR, "%s: ERROR: ValidatePair failed for device %s, lockdown error %d", __func__, _dev->udid, lerr);
			goto leave;
		}

		usbmuxd_log(LL_INFO, "%s: Finished preflight on device %s", __func__, _dev->udid);

		/* emit device added event and thus make device visible to clients */
		client_device_add(info);
	}

leave:
	if (value)
		plist_free(value);
	if (version_str)
		free(version_str);
	if (lockdown)
		lockdownd_client_free(lockdown);
	if (dev)
		idevice_free(dev);

	free((char*)info->serial);
	free(info);

	return NULL;
}
#else
void preflight_device_remove_cb(void *data)
{
}
#endif

void preflight_worker_device_add(struct device_info* info)
{
	if (info->pid == PID_APPLE_T2_COPROCESSOR || no_preflight == 1) {
		client_device_add(info);
		return;
	}

#ifdef HAVE_LIBIMOBILEDEVICE
	struct device_info *infocopy = (struct device_info*)malloc(sizeof(struct device_info));

	memcpy(infocopy, info, sizeof(struct device_info));
	if (info->serial) {
		infocopy->serial = strdup(info->serial);
	}

	pthread_t th;
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	int perr = pthread_create(&th, &attr, preflight_worker_handle_device_add, infocopy);
	if (perr != 0) {
		free((char*)infocopy->serial);
		free(infocopy);
		usbmuxd_log(LL_ERROR, "ERROR: failed to start preflight worker thread for device %s: %s (%d). Invoking client_device_add() directly but things might not work as expected.", info->serial, strerror(perr), perr);
		client_device_add(info);
	}
#else
	client_device_add(info);
#endif
}
