/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2014  Intel Corporation. All rights reserved.
 *
 *
 */

#define BLUEZ_AUDIO_MTU 1024

static const char BLUEZ_AUDIO_SK_PATH[] = "\0bluez_audio_socket";

#define AUDIO_SERVICE_ID		0
#define AUDIO_SERVICE_ID_MAX		AUDIO_SERVICE_ID

#define AUDIO_STATUS_SUCCESS		IPC_STATUS_SUCCESS
#define AUDIO_STATUS_FAILED		0x01

#define AUDIO_OP_STATUS			IPC_OP_STATUS

#define AUDIO_OP_OPEN			0x01
struct audio_preset {
	uint8_t len;
	uint8_t data[0];
} __attribute__((packed));

struct audio_cmd_open {
	uint8_t uuid[16];
	uint8_t codec;
	uint8_t presets;
	struct audio_preset preset[0];
} __attribute__((packed));

struct audio_rsp_open {
	uint8_t id;
} __attribute__((packed));

#define AUDIO_OP_CLOSE			0x02
struct audio_cmd_close {
	uint8_t id;
} __attribute__((packed));

#define AUDIO_OP_OPEN_STREAM		0x03
struct audio_cmd_open_stream {
	uint8_t id;
} __attribute__((packed));

struct audio_rsp_open_stream {
	uint16_t id;
	uint16_t mtu;
	struct audio_preset preset[0];
} __attribute__((packed));

#define AUDIO_OP_CLOSE_STREAM		0x04
struct audio_cmd_close_stream {
	uint8_t id;
} __attribute__((packed));

#define AUDIO_OP_RESUME_STREAM		0x05
struct audio_cmd_resume_stream {
	uint8_t id;
} __attribute__((packed));

#define AUDIO_OP_SUSPEND_STREAM		0x06
struct audio_cmd_suspend_stream {
	uint8_t id;
} __attribute__((packed));
