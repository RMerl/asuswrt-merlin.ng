// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2018  Intel Corporation. All rights reserved.
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include <dirent.h>
#include <ftw.h>
#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <time.h>
#include <sys/time.h>

#include <ell/ell.h>

#include "mesh/util.h"

static bool debug_enabled;

void print_packet(const char *label, const void *data, uint16_t size)
{
	struct timeval pkt_time;

	if (!debug_enabled)
		return;

	gettimeofday(&pkt_time, NULL);

	if (size > 0) {
		char *str;

		str = l_util_hexstring(data, size);
		l_debug("%05d.%03d %s: %s",
				(uint32_t) pkt_time.tv_sec % 100000,
				(uint32_t) pkt_time.tv_usec/1000, label, str);
		l_free(str);
	} else
		l_debug("%05d.%03d %s: empty",
				(uint32_t) pkt_time.tv_sec % 100000,
				(uint32_t) pkt_time.tv_usec/1000, label);
}

uint32_t get_timestamp_secs(void)
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec;
}

bool str2hex(const char *str, uint16_t in_len, uint8_t *out,
							uint16_t out_len)
{
	uint16_t i;

	if (in_len < out_len * 2)
		return false;

	for (i = 0; i < out_len; i++) {
		if (sscanf(&str[i * 2], "%02hhx", &out[i]) != 1)
			return false;
	}

	return true;
}

size_t hex2str(uint8_t *in, size_t in_len, char *out, size_t out_len)
{
	static const char hexdigits[] = "0123456789abcdef";
	size_t i;

	if (in_len * 2 > (out_len - 1))
		return 0;

	for (i = 0; i < in_len; i++) {
		out[i * 2] = hexdigits[in[i] >> 4];
		out[i * 2 + 1] = hexdigits[in[i] & 0xf];
	}

	out[in_len * 2] = '\0';
	return i;
}

int create_dir(const char *dir_name)
{
	struct stat st;
	char dir[PATH_MAX + 1], *prev, *next;
	int err;

	err = stat(dir_name, &st);
	if (!err && S_ISREG(st.st_mode))
		return 0;

	memset(dir, 0, PATH_MAX + 1);
	strcat(dir, "/");

	prev = strchr(dir_name, '/');

	while (prev) {
		next = strchr(prev + 1, '/');
		if (!next)
			break;

		if (next - prev == 1) {
			prev = next;
			continue;
		}

		strncat(dir, prev + 1, next - prev);
		mkdir(dir, 0755);

		prev = next;
	}

	mkdir(dir_name, 0755);

	return 0;
}

static int del_fobject(const char *fpath, const struct stat *sb, int typeflag,
						struct FTW *ftwbuf)
{
	switch (typeflag) {
	case FTW_DP:
		rmdir(fpath);
		l_debug("RMDIR %s", fpath);
		break;

	case FTW_SL:
	default:
		remove(fpath);
		l_debug("RM %s", fpath);
		break;
	}
	return 0;
}


void del_path(const char *path)
{
	nftw(path, del_fobject, 5, FTW_DEPTH | FTW_PHYS);
}

void enable_debug(void)
{
	debug_enabled = true;
	l_debug_enable("*");
}
