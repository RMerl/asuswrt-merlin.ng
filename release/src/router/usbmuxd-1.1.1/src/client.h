/*
 * client.h
 *
 * Copyright (C) 2009 Hector Martin <hector@marcansoft.com>
 * Copyright (C) 2009 Nikias Bassen <nikias@gmx.li>
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

#ifndef CLIENT_H
#define CLIENT_H

#include <stdint.h>
#include "usbmuxd-proto.h"

struct device_info;
struct mux_client;

int client_read(struct mux_client *client, void *buffer, uint32_t len);
int client_write(struct mux_client *client, void *buffer, uint32_t len);
int client_set_events(struct mux_client *client, short events);
void client_close(struct mux_client *client);
int client_notify_connect(struct mux_client *client, enum usbmuxd_result result);

void client_device_add(struct device_info *dev);
void client_device_remove(int device_id);
void client_device_paired(int device_id);

int client_accept(int fd);
void client_get_fds(struct fdlist *list);
void client_process(int fd, short events);

void client_init(void);
void client_shutdown(void);

#endif
