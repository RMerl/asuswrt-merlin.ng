/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2006-2010  Nokia Corporation
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *  Copyright (C) 2011  BMW Car IT GmbH. All rights reserved.
 *
 *
 */

struct a2dp_sep;
struct a2dp_setup;

typedef void (*a2dp_endpoint_select_t) (struct a2dp_setup *setup, void *ret,
					int size);
typedef void (*a2dp_endpoint_config_t) (struct a2dp_setup *setup, gboolean ret);

struct a2dp_endpoint {
	const char *(*get_name) (struct a2dp_sep *sep, void *user_data);
	const char *(*get_path) (struct a2dp_sep *sep, void *user_data);
	size_t (*get_capabilities) (struct a2dp_sep *sep,
						uint8_t **capabilities,
						void *user_data);
	int (*select_configuration) (struct a2dp_sep *sep,
						uint8_t *capabilities,
						size_t length,
						struct a2dp_setup *setup,
						a2dp_endpoint_select_t cb,
						void *user_data);
	int (*set_configuration) (struct a2dp_sep *sep,
						uint8_t *configuration,
						size_t length,
						struct a2dp_setup *setup,
						a2dp_endpoint_config_t cb,
						void *user_data);
	void (*clear_configuration) (struct a2dp_sep *sep, void *user_data);
	void (*set_delay) (struct a2dp_sep *sep, uint16_t delay,
							void *user_data);
};

typedef void (*a2dp_discover_cb_t) (struct avdtp *session, GSList *seps,
						int err, void *user_data);
typedef void (*a2dp_select_cb_t) (struct avdtp *session, struct a2dp_sep *sep,
					GSList *caps, int err,
					void *user_data);
typedef void (*a2dp_config_cb_t) (struct avdtp *session, struct a2dp_sep *sep,
					struct avdtp_stream *stream, int err,
					void *user_data);
typedef void (*a2dp_stream_cb_t) (struct avdtp *session, int err,
					void *user_data);

struct a2dp_sep *a2dp_add_sep(struct btd_adapter *adapter, uint8_t type,
				uint8_t codec, gboolean delay_reporting,
				struct a2dp_endpoint *endpoint,
				void *user_data, GDestroyNotify destroy,
				int *err);
void a2dp_remove_sep(struct a2dp_sep *sep);


unsigned int a2dp_discover(struct avdtp *session, a2dp_discover_cb_t cb,
							void *user_data);
unsigned int a2dp_select_capabilities(struct avdtp *session,
					uint8_t type, const char *sender,
					a2dp_select_cb_t cb,
					void *user_data);
unsigned int a2dp_config(struct avdtp *session, struct a2dp_sep *sep,
				a2dp_config_cb_t cb, GSList *caps,
				void *user_data);
unsigned int a2dp_resume(struct avdtp *session, struct a2dp_sep *sep,
				a2dp_stream_cb_t cb, void *user_data);
unsigned int a2dp_suspend(struct avdtp *session, struct a2dp_sep *sep,
				a2dp_stream_cb_t cb, void *user_data);
gboolean a2dp_cancel(unsigned int id);

gboolean a2dp_sep_lock(struct a2dp_sep *sep, struct avdtp *session);
gboolean a2dp_sep_unlock(struct a2dp_sep *sep, struct avdtp *session);
struct avdtp_stream *a2dp_sep_get_stream(struct a2dp_sep *sep);
struct btd_device *a2dp_setup_get_device(struct a2dp_setup *setup);
const char *a2dp_setup_remote_path(struct a2dp_setup *setup);
struct avdtp *a2dp_avdtp_get(struct btd_device *device);
