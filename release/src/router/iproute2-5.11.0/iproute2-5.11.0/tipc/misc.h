/*
 * misc.h	Miscellaneous TIPC helper functions.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Richard Alpe <richard.alpe@ericsson.com>
 */

#ifndef _TIPC_MISC_H
#define _TIPC_MISC_H

#include <stdint.h>

uint32_t str2addr(char *str);
int str2nodeid(char *str, uint8_t *id);
void nodeid2str(uint8_t *id, char *str);
void hash2nodestr(uint32_t hash, char *str);
int str2key(char *str, struct tipc_aead_key *key);

#endif
