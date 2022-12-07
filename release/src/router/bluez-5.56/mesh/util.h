/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2018  Intel Corporation. All rights reserved.
 *
 *
 */

uint32_t get_timestamp_secs(void);
bool str2hex(const char *str, uint16_t in_len, uint8_t *out,
							uint16_t out_len);
size_t hex2str(uint8_t *in, size_t in_len, char *out, size_t out_len);
void print_packet(const char *label, const void *data, uint16_t size);
int create_dir(const char *dir_name);
void del_path(const char *path);
void enable_debug(void);
