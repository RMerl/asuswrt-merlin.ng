/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */

int connection_register(struct btd_service *service);
void connection_unregister(struct btd_service *service);
int connection_connect(struct btd_service *service);
int connection_disconnect(struct btd_service *service);
