/*
 * misc.c	Miscellaneous TIPC helper functions.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Richard Alpe <richard.alpe@ericsson.com>
 */

#include <stdio.h>
#include <stdint.h>
#include <linux/tipc.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include "misc.h"

#define IN_RANGE(val, low, high) ((val) <= (high) && (val) >= (low))

uint32_t str2addr(char *str)
{
	unsigned int z, c, n;
	char dummy;

	if (sscanf(str, "%u.%u.%u%c", &z, &c, &n, &dummy) != 3) {
		fprintf(stderr, "invalid network address, syntax: Z.C.N\n");
		return 0;
	}

	if (IN_RANGE(z, 0, 255) && IN_RANGE(c, 0, 4095) && IN_RANGE(n, 0, 4095))
		return tipc_addr(z, c, n);

	fprintf(stderr, "invalid network address \"%s\"\n", str);
	return 0;
}

static int is_hex(char *arr, int last)
{
	int i;

	while (!arr[last])
		last--;

	for (i = 0; i <= last; i++) {
		if (!IN_RANGE(arr[i], '0', '9') &&
		    !IN_RANGE(arr[i], 'a', 'f') &&
		    !IN_RANGE(arr[i], 'A', 'F'))
			return 0;
	}
	return 1;
}

static int is_name(char *arr, int last)
{
	int i;
	char c;

	while (!arr[last])
		last--;

	if (last > 15)
		return 0;

	for (i = 0; i <= last; i++) {
		c = arr[i];
		if (!IN_RANGE(c, '0', '9') && !IN_RANGE(c, 'a', 'z') &&
		    !IN_RANGE(c, 'A', 'Z') && c != '-' && c != '_' &&
		    c != '.' && c != ':' && c != '@')
			return 0;
	}
	return 1;
}

int str2nodeid(char *str, uint8_t *id)
{
	int len = strlen(str);
	int i;

	if (len > 32)
		return -1;

	if (is_name(str, len - 1)) {
		memcpy(id, str, len);
		return 0;
	}
	if (!is_hex(str, len - 1))
		return -1;

	str[len] = '0';
	for (i = 0; i < 16; i++) {
		if (sscanf(&str[2 * i], "%2hhx", &id[i]) != 1)
			break;
	}
	return 0;
}

int str2key(char *str, struct tipc_aead_key *key)
{
	int len = strlen(str);
	int ishex = 0;
	int i;

	/* Check if the input is a hex string (i.e. 0x...) */
	if (len > 2 && strncmp(str, "0x", 2) == 0) {
	    ishex = is_hex(str + 2, len - 2 - 1);
	    if (ishex) {
		len -= 2;
		str += 2;
	    }
	}

	/* Obtain key: */
	if (!ishex) {
		key->keylen = len;
		memcpy(key->key, str, len);
	} else {
		/* Convert hex string to key */
		key->keylen = (len + 1) / 2;
		for (i = 0; i < key->keylen; i++) {
			if (i == 0 && len % 2 != 0) {
				if (sscanf(str, "%1hhx", &key->key[0]) != 1)
					return -1;
				str += 1;
				continue;
			}
			if (sscanf(str, "%2hhx", &key->key[i]) != 1)
				return -1;
			str += 2;
		}
	}

	return 0;
}

void nodeid2str(uint8_t *id, char *str)
{
	int i;

	if (is_name((char *)id, 15)) {
		memcpy(str, id, 16);
		return;
	}

	for (i = 0; i < 16; i++)
		sprintf(&str[2 * i], "%02x", id[i]);

	for (i = 31; str[i] == '0'; i--)
		str[i] = 0;
}

void hash2nodestr(uint32_t hash, char *str)
{
	struct tipc_sioc_nodeid_req nr = {};
	int sd;

	sd = socket(AF_TIPC, SOCK_RDM, 0);
	if (sd < 0) {
		fprintf(stderr, "opening TIPC socket: %s\n", strerror(errno));
		return;
	}
	nr.peer = hash;
	if (!ioctl(sd, SIOCGETNODEID, &nr))
		nodeid2str((uint8_t *)nr.node_id, str);
	close(sd);
}
