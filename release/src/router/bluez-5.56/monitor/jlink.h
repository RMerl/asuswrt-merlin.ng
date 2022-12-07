/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2018  Codecoup
 *
 *
 */

int jlink_init(void);
int jlink_connect(char *cfg);
int jlink_start_rtt(char *cfg);
int jlink_rtt_read(void *buf, size_t size);
