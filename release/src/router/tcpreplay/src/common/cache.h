/* $Id$ */

/*
 *   Copyright (c) 2001-2010 Aaron Turner <aturner at synfin dot net>
 *   Copyright (c) 2013-2022 Fred Klassen <tcpreplay at appneta dot com> - AppNeta
 *
 *   The Tcpreplay Suite of tools is free software: you can redistribute it 
 *   and/or modify it under the terms of the GNU General Public License as 
 *   published by the Free Software Foundation, either version 3 of the 
 *   License, or with the authors permission any later version.
 *
 *   The Tcpreplay Suite is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with the Tcpreplay Suite.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "defines.h"

#define CACHEMAGIC "tcpprep"
#define CACHEVERSION "04"
#define CACHEDATASIZE 255
#define CACHE_PACKETS_PER_BYTE 4    /* number of packets / byte */
#define CACHE_BITS_PER_PACKET 2     /* number of bits / packet */

#define SEND 1
#define DONT_SEND 0

/* 
 * CACHEVERSION History:
 * 01 - Initial release.  1 bit of data/packet (primary or secondary nic)
 * 02 - 2 bits of data/packet (drop/send & primary or secondary nic)
 * 03 - Write integers in network-byte order
 * 04 - Increase num_packets from 32 to 64 bit integer
 */

struct tcpr_cache_s {
    char data[CACHEDATASIZE];
    unsigned int packets;       /* number of packets tracked in data */
    struct tcpr_cache_s *next;
};
typedef struct tcpr_cache_s tcpr_cache_t;

/*
 * Each byte in cache_type.data represents CACHE_PACKETS_PER_BYTE (4) number of packets
 * Each packet has CACHE_BITS_PER_PACKETS (2) bits of data.
 * High Bit: 1 = send, 0 = don't send
 * Low Bit: 1 = primary interface, 0 = secondary interface
*/

/*
 * cache_file_header Data structure defining a file as a tcpprep cache file
 * and it's version
 * 
 * If you need to enhance this struct, do so AFTER the version field and be sure
 * to increment  CACHEVERSION
 */
struct tcpr_cache_file_hdr_s {
    char magic[8];
    char version[4];
    /* begin version 2 features */
    /* version 3 puts everything in network-byte order */
    /* version 4 makes num_packets a 64 bit int */
    u_int64_t num_packets;      /* total # of packets in file */
    u_int16_t packets_per_byte;
    u_int16_t comment_len;      /* how long is the user comment? */
} __attribute__((__packed__));

typedef struct tcpr_cache_file_hdr_s tcpr_cache_file_hdr_t;

enum tcpr_dir_e {
    TCPR_DIR_ERROR  = -1,
    TCPR_DIR_NOSEND = 0,
    TCPR_DIR_C2S    = 1, /* aka PRIMARY */
    TCPR_DIR_S2C    = 2 /* aka SECONDARY */
};
typedef enum tcpr_dir_e tcpr_dir_t;


COUNTER write_cache(tcpr_cache_t *, const int, COUNTER, char *);
tcpr_dir_t add_cache(tcpr_cache_t **, const int, const tcpr_dir_t);
COUNTER read_cache(char **, const char *, char **);
tcpr_dir_t check_cache(char *, COUNTER);

/* return values for check_cache 
#define CACHE_ERROR -1
#define CACHE_NOSEND 0  // NULL 
#define CACHE_PRIMARY 1
#define CACHE_SECONDARY 2
*/


/* macro to change a bitstring to 8 bits */
#define BIT_STR(x) x[0], x[1], x[2], x[3], x[4], x[5], x[6], x[7]

/* string of 8 zeros */
#define EIGHT_ZEROS "\060\060\060\060\060\060\060\060"
