/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2006-2010  Nokia Corporation
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */

#define AUDIO_CONTROL_INTERFACE "org.bluez.MediaControl1"

struct btd_service;

int control_init_target(struct btd_service *service);
int control_init_remote(struct btd_service *service);
void control_unregister(struct btd_service *service);

int control_connect(struct btd_service *service);
int control_disconnect(struct btd_service *service);

int control_set_player(struct btd_service *service, const char *path);
