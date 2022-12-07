/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */

struct bnep;

int bnep_init(void);
int bnep_cleanup(void);

struct bnep *bnep_new(int sk, uint16_t local_role, uint16_t remote_role,
								char *iface);
void bnep_free(struct bnep *session);

typedef void (*bnep_connect_cb) (char *iface, int err, void *data);
typedef void (*bnep_disconnect_cb) (void *data);
int bnep_connect(struct bnep *session, bnep_connect_cb conn_cb,
					bnep_disconnect_cb disconn_cb,
					void *conn_data, void *disconn_data);
void bnep_disconnect(struct bnep *session);

int bnep_server_add(int sk, char *bridge, char *iface, const bdaddr_t *addr,
						uint8_t *setup_data, int len);
void bnep_server_delete(char *bridge, char *iface, const bdaddr_t *addr);
