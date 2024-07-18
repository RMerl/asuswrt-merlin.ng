#ifndef fooavahihfoo
#define fooavahihfoo

/*
  This file is part of nss-mdns.

  nss-mdns is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation; either version 2 of the
  License, or (at your option) any later version.

  nss-mdns is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with nss-mdns; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <inttypes.h>
#include <sys/types.h>

// Maximum number of entries to return.
#define MAX_ENTRIES 16

typedef struct {
    uint32_t address;
} ipv4_address_t;

typedef struct {
    uint8_t address[16];
} ipv6_address_t;

typedef struct {
    int af;
    union {
        ipv4_address_t ipv4;
        ipv6_address_t ipv6;
    } address;
    uint32_t scopeid;
} query_address_result_t;

typedef struct {
    int count;
    query_address_result_t result[MAX_ENTRIES];
} userdata_t;

typedef enum {
    AVAHI_RESOLVE_RESULT_SUCCESS,
    AVAHI_RESOLVE_RESULT_HOST_NOT_FOUND,
    AVAHI_RESOLVE_RESULT_UNAVAIL
} avahi_resolve_result_t;

avahi_resolve_result_t avahi_resolve_name(int af, const char* name,
                                          query_address_result_t* result);

avahi_resolve_result_t avahi_resolve_address(int af, const void* data,
                                             char* name, size_t name_len);

#endif
