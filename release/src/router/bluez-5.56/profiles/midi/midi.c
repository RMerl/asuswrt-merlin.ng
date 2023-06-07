// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2015,2016 Felipe F. Tonello <eu@felipetonello.com>
 *  Copyright (C) 2016 ROLI Ltd.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#include <alsa/asoundlib.h>

#include "lib/bluetooth.h"
#include "lib/sdp.h"
#include "lib/uuid.h"

#include "src/plugin.h"
#include "src/adapter.h"
#include "src/device.h"
#include "src/profile.h"
#include "src/service.h"
#include "src/shared/util.h"
#include "src/shared/att.h"
#include "src/shared/queue.h"
#include "src/shared/gatt-db.h"
#include "src/shared/gatt-client.h"
#include "src/shared/io.h"
#include "src/log.h"
#include "attrib/att.h"

#include "libmidi.h"

struct midi {
	struct btd_device *dev;
	struct gatt_db *db;
	struct bt_gatt_client *client;
	unsigned int io_cb_id;
	struct io *io;
	uint16_t midi_io_handle;

	/* ALSA handlers */
	snd_seq_t *seq_handle;
	int seq_client_id;
	int seq_port_id;

	/* MIDI parser*/
	struct midi_read_parser midi_in;
	struct midi_write_parser midi_out;
};

static bool midi_write_cb(struct io *io, void *user_data)
{
	struct midi *midi = user_data;
	int err;

	void foreach_cb(const struct midi_write_parser *parser, void *user_data) {
		struct midi *midi = user_data;
		bt_gatt_client_write_without_response(midi->client,
		                                      midi->midi_io_handle,
		                                      false,
		                                      midi_write_data(parser),
		                                      midi_write_data_size(parser));
	};

	do {
		snd_seq_event_t *event = NULL;

		err = snd_seq_event_input(midi->seq_handle, &event);

		if (err < 0 || !event)
			break;

		midi_read_ev(&midi->midi_out, event, foreach_cb, midi);

	} while (err > 0);

	if (midi_write_has_data(&midi->midi_out))
		bt_gatt_client_write_without_response(midi->client,
		                                      midi->midi_io_handle,
		                                      false,
		                                      midi_write_data(&midi->midi_out),
		                                      midi_write_data_size(&midi->midi_out));

	midi_write_reset(&midi->midi_out);

	return true;
}

static void midi_io_value_cb(uint16_t value_handle, const uint8_t *value,
                             uint16_t length, void *user_data)
{
	struct midi *midi = user_data;
	snd_seq_event_t ev;
	unsigned int i = 0;

	if (length < 3) {
		warn("MIDI I/O: Wrong packet format: length is %u bytes but it should "
		     "be at least 3 bytes", length);
		return;
	}

	snd_seq_ev_clear(&ev);
	snd_seq_ev_set_source(&ev, midi->seq_port_id);
	snd_seq_ev_set_subs(&ev);
	snd_seq_ev_set_direct(&ev);

	midi_read_reset(&midi->midi_in);

	while (i < length) {
		size_t count = midi_read_raw(&midi->midi_in, value + i, length - i, &ev);

		if (count == 0)
			goto _err;

		if (ev.type != SND_SEQ_EVENT_NONE)
			snd_seq_event_output_direct(midi->seq_handle, &ev);

		i += count;
	}

	return;

_err:
	error("Wrong BLE-MIDI message");
}

static void midi_io_ccc_written_cb(uint16_t att_ecode, void *user_data)
{
	if (att_ecode != 0) {
		error("MIDI I/O: notifications not enabled %s",
		      att_ecode2str(att_ecode));
		return;
	}

	DBG("MIDI I/O: notification enabled");
}

static void midi_io_initial_read_cb(bool success, uint8_t att_ecode,
                                    const uint8_t *value, uint16_t length,
                                    void *user_data)
{
	struct midi *midi = user_data;

	if (!success) {
		error("MIDI I/O: Failed to read initial request");
		return;
	}

	/* request notify */
	midi->io_cb_id =
		bt_gatt_client_register_notify(midi->client,
		                               midi->midi_io_handle,
		                               midi_io_ccc_written_cb,
		                               midi_io_value_cb,
		                               midi,
		                               NULL);
}

static void handle_midi_io(struct midi *midi, uint16_t value_handle)
{
	DBG("MIDI I/O handle: 0x%04x", value_handle);

	midi->midi_io_handle = value_handle;

	/*
	 * The BLE-MIDI 1.0 spec specifies that The Central shall attempt to
	 * read the MIDI I/O characteristic of the Peripheral right after
	 * estrablhishing a connection with the accessory.
	 */
	if (!bt_gatt_client_read_value(midi->client,
	                               value_handle,
	                               midi_io_initial_read_cb,
	                               midi,
	                               NULL))
		DBG("MIDI I/O: Failed to send request to read initial value");
}

static void handle_characteristic(struct gatt_db_attribute *attr,
                                  void *user_data)
{
	struct midi *midi = user_data;
	uint16_t value_handle;
	bt_uuid_t uuid, midi_io_uuid;

	if (!gatt_db_attribute_get_char_data(attr, NULL, &value_handle, NULL,
	                                     NULL, &uuid)) {
		error("Failed to obtain characteristic data");
		return;
	}

	bt_string_to_uuid(&midi_io_uuid, MIDI_IO_UUID);

	if (bt_uuid_cmp(&midi_io_uuid, &uuid) == 0)
		handle_midi_io(midi, value_handle);
	else {
		char uuid_str[MAX_LEN_UUID_STR];

		bt_uuid_to_string(&uuid, uuid_str, sizeof(uuid_str));
		DBG("Unsupported characteristic: %s", uuid_str);
	}
}

static void foreach_midi_service(struct gatt_db_attribute *attr,
                                 void *user_data)
{
	struct midi *midi = user_data;

	gatt_db_service_foreach_char(attr, handle_characteristic, midi);
}

static int midi_device_probe(struct btd_service *service)
{
	struct btd_device *device = btd_service_get_device(service);
	struct midi *midi;
	char addr[18];

	ba2str(device_get_address(device), addr);
	DBG("MIDI GATT Driver profile probe (%s)", addr);

	/* Ignore, if we were probed for this device already */
	midi = btd_service_get_user_data(service);
	if (midi) {
		error("Profile probed twice for the same device!");
		return -EADDRINUSE;
	}

	midi = g_new0(struct midi, 1);
	if (!midi)
		return -ENOMEM;

	midi->dev = btd_device_ref(device);

	btd_service_set_user_data(service, midi);

	return 0;
}

static void midi_device_remove(struct btd_service *service)
{
	struct btd_device *device = btd_service_get_device(service);
	struct midi *midi;
	char addr[18];

	ba2str(device_get_address(device), addr);
	DBG("MIDI GATT Driver profile remove (%s)", addr);

	midi = btd_service_get_user_data(service);
	if (!midi) {
		error("MIDI Service not handled by profile");
		return;
	}

	btd_device_unref(midi->dev);
	g_free(midi);
}

static int midi_accept(struct btd_service *service)
{
	struct btd_device *device = btd_service_get_device(service);
	struct gatt_db *db = btd_device_get_gatt_db(device);
	struct bt_gatt_client *client = btd_device_get_gatt_client(device);
	bt_uuid_t midi_uuid;
	struct pollfd pfd;
	struct midi *midi;
	char addr[18];
	char device_name[MAX_NAME_LENGTH + 11]; /* 11 = " Bluetooth\0"*/
	int err;
	snd_seq_client_info_t *info;

	ba2str(device_get_address(device), addr);
	DBG("MIDI GATT Driver profile accept (%s)", addr);

	midi = btd_service_get_user_data(service);
	if (!midi) {
		error("MIDI Service not handled by profile");
		return -ENODEV;
	}

	/* Port Name */
	memset(device_name, 0, sizeof(device_name));
	if (device_name_known(device))
		device_get_name(device, device_name, sizeof(device_name));
	else
		strncpy(device_name, addr, sizeof(device_name));

	/* ALSA Sequencer Client and Port Setup */
	err = snd_seq_open(&midi->seq_handle, "default", SND_SEQ_OPEN_DUPLEX, 0);
	if (err < 0) {
		error("Could not open ALSA Sequencer: %s (%d)", snd_strerror(err), err);
		return err;
	}

	err = snd_seq_nonblock(midi->seq_handle, SND_SEQ_NONBLOCK);
	if (err < 0) {
		error("Could not set nonblock mode: %s (%d)", snd_strerror(err), err);
		goto _err_handle;
	}

	err = snd_seq_set_client_name(midi->seq_handle, device_name);
	if (err < 0) {
		error("Could not configure ALSA client: %s (%d)", snd_strerror(err), err);
		goto _err_handle;
	}

	err = snd_seq_client_id(midi->seq_handle);
	if (err < 0) {
		error("Could retreive ALSA client: %s (%d)", snd_strerror(err), err);
		goto _err_handle;
	}
	midi->seq_client_id = err;

	err = snd_seq_create_simple_port(midi->seq_handle, strcat(device_name, " Bluetooth"),
	                                 SND_SEQ_PORT_CAP_READ |
	                                 SND_SEQ_PORT_CAP_WRITE |
	                                 SND_SEQ_PORT_CAP_SUBS_READ |
	                                 SND_SEQ_PORT_CAP_SUBS_WRITE,
	                                 SND_SEQ_PORT_TYPE_MIDI_GENERIC |
	                                 SND_SEQ_PORT_TYPE_HARDWARE);
	if (err < 0) {
		error("Could not create ALSA port: %s (%d)", snd_strerror(err), err);
		goto _err_handle;
	}
	midi->seq_port_id = err;

	snd_seq_client_info_alloca(&info);
	err = snd_seq_get_client_info(midi->seq_handle, info);
	if (err < 0)
		goto _err_port;

	/* list of relevant sequencer events */
	snd_seq_client_info_event_filter_add(info, SND_SEQ_EVENT_NOTEOFF);
	snd_seq_client_info_event_filter_add(info, SND_SEQ_EVENT_NOTEON);
	snd_seq_client_info_event_filter_add(info, SND_SEQ_EVENT_KEYPRESS);
	snd_seq_client_info_event_filter_add(info, SND_SEQ_EVENT_CONTROLLER);
	snd_seq_client_info_event_filter_add(info, SND_SEQ_EVENT_PGMCHANGE);
	snd_seq_client_info_event_filter_add(info, SND_SEQ_EVENT_CHANPRESS);
	snd_seq_client_info_event_filter_add(info, SND_SEQ_EVENT_PITCHBEND);
	snd_seq_client_info_event_filter_add(info, SND_SEQ_EVENT_SYSEX);
	snd_seq_client_info_event_filter_add(info, SND_SEQ_EVENT_QFRAME);
	snd_seq_client_info_event_filter_add(info, SND_SEQ_EVENT_SONGPOS);
	snd_seq_client_info_event_filter_add(info, SND_SEQ_EVENT_SONGSEL);
	snd_seq_client_info_event_filter_add(info, SND_SEQ_EVENT_TUNE_REQUEST);
	snd_seq_client_info_event_filter_add(info, SND_SEQ_EVENT_CLOCK);
	snd_seq_client_info_event_filter_add(info, SND_SEQ_EVENT_START);
	snd_seq_client_info_event_filter_add(info, SND_SEQ_EVENT_CONTINUE);
	snd_seq_client_info_event_filter_add(info, SND_SEQ_EVENT_STOP);
	snd_seq_client_info_event_filter_add(info, SND_SEQ_EVENT_SENSING);
	snd_seq_client_info_event_filter_add(info, SND_SEQ_EVENT_RESET);
	snd_seq_client_info_event_filter_add(info, SND_SEQ_EVENT_CONTROL14);
	snd_seq_client_info_event_filter_add(info, SND_SEQ_EVENT_NONREGPARAM);
	snd_seq_client_info_event_filter_add(info, SND_SEQ_EVENT_REGPARAM);

	err = snd_seq_set_client_info(midi->seq_handle, info);
	if (err < 0)
		goto _err_port;


	/* Input file descriptors */
	snd_seq_poll_descriptors(midi->seq_handle, &pfd, 1, POLLIN);

	midi->io = io_new(pfd.fd);
	if (!midi->io) {
		error("Could not allocate I/O eventloop");
		goto _err_port;
	}

	io_set_read_handler(midi->io, midi_write_cb, midi, NULL);

	midi->db = gatt_db_ref(db);
	midi->client = bt_gatt_client_ref(client);

	err = midi_read_init(&midi->midi_in);
	if (err < 0) {
		error("Could not initialise MIDI input parser");
		goto _err_port;
	}

	err = midi_write_init(&midi->midi_out, bt_gatt_client_get_mtu(midi->client) - 3);
	if (err < 0) {
		error("Could not initialise MIDI output parser");
		goto _err_midi;
	}

	bt_string_to_uuid(&midi_uuid, MIDI_UUID);
	gatt_db_foreach_service(db, &midi_uuid, foreach_midi_service, midi);

	btd_service_connecting_complete(service, 0);

	return 0;

_err_midi:
	midi_read_free(&midi->midi_in);

_err_port:
	snd_seq_delete_simple_port(midi->seq_handle, midi->seq_port_id);

_err_handle:
	snd_seq_close(midi->seq_handle);
	midi->seq_handle = NULL;

	btd_service_connecting_complete(service, err);

	return err;
}

static int midi_disconnect(struct btd_service *service)
{
	struct btd_device *device = btd_service_get_device(service);
	struct midi *midi;
	char addr[18];

	ba2str(device_get_address(device), addr);
	DBG("MIDI GATT Driver profile disconnect (%s)", addr);

	midi = btd_service_get_user_data(service);
	if (!midi) {
		error("MIDI Service not handled by profile");
		return -ENODEV;
	}

	midi_read_free(&midi->midi_in);
	midi_write_free(&midi->midi_out);
	io_destroy(midi->io);
	snd_seq_delete_simple_port(midi->seq_handle, midi->seq_port_id);
	midi->seq_port_id = 0;
	snd_seq_close(midi->seq_handle);
	midi->seq_handle = NULL;

	/* Clean-up any old client/db */
	bt_gatt_client_unregister_notify(midi->client, midi->io_cb_id);
	bt_gatt_client_unref(midi->client);
	gatt_db_unref(midi->db);

	btd_service_disconnecting_complete(service, 0);

	return 0;
}

static struct btd_profile midi_profile = {
	.name = "MIDI GATT Driver",
	.remote_uuid = MIDI_UUID,
	.priority = BTD_PROFILE_PRIORITY_HIGH,
	.auto_connect = true,

	.device_probe = midi_device_probe,
	.device_remove = midi_device_remove,

	.accept = midi_accept,

	.disconnect = midi_disconnect,
};

static int midi_init(void)
{
	return btd_profile_register(&midi_profile);
}

static void midi_exit(void)
{
	btd_profile_unregister(&midi_profile);
}

BLUETOOTH_PLUGIN_DEFINE(midi, VERSION, BLUETOOTH_PLUGIN_PRIORITY_HIGH,
                        midi_init, midi_exit);
