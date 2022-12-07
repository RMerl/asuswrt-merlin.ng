/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */

#define L2CAP_PSM_HIDP_CTRL	0x11
#define L2CAP_PSM_HIDP_INTR	0x13

struct input_device;
struct input_conn;

void input_set_idle_timeout(int timeout);
void input_enable_userspace_hid(bool state);
void input_set_classic_bonded_only(bool state);
bool input_get_classic_bonded_only(void);
void input_set_auto_sec(bool state);

int input_device_register(struct btd_service *service);
void input_device_unregister(struct btd_service *service);

bool input_device_exists(const bdaddr_t *src, const bdaddr_t *dst);
int input_device_set_channel(const bdaddr_t *src, const bdaddr_t *dst, int psm,
							GIOChannel *io);
int input_device_close_channels(const bdaddr_t *src, const bdaddr_t *dst);

int input_device_connect(struct btd_service *service);
int input_device_disconnect(struct btd_service *service);
