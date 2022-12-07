/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2002-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */

int read_discoverable_timeout(const char *src, int *timeout);
int read_pairable_timeout(const char *src, int *timeout);
int read_on_mode(const char *src, char *mode, int length);
int read_local_name(const bdaddr_t *bdaddr, char *name);
sdp_record_t *record_from_string(const char *str);
sdp_record_t *find_record_in_list(sdp_list_t *recs, const char *uuid);
