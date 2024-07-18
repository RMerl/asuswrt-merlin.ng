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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <assert.h>
#include <unistd.h>

#include "avahi.h"
#include "util.h"

#define WHITESPACE " \t"

static FILE* open_socket(void) {
    int fd = -1;
    struct sockaddr_un sa;
    FILE* f = NULL;

    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
        goto fail;

    set_cloexec(fd);

    memset(&sa, 0, sizeof(sa));
    sa.sun_family = AF_UNIX;
    strncpy(sa.sun_path, AVAHI_SOCKET, sizeof(sa.sun_path) - 1);
    sa.sun_path[sizeof(sa.sun_path) - 1] = 0;

    if (connect(fd, (struct sockaddr*)&sa, sizeof(sa)) < 0)
        goto fail;

    if (!(f = fdopen(fd, "r+")))
        goto fail;

    return f;

fail:
    if (fd >= 0)
        close(fd);

    return NULL;
}

static avahi_resolve_result_t
avahi_resolve_name_with_socket(FILE* f, int af, const char* name,
                               query_address_result_t* result) {
    char* p;
    char ln[256];

    fprintf(f, "RESOLVE-HOSTNAME%s %s\n", af == AF_INET ? "-IPV4" : "-IPV6",
            name);
    fflush(f);

    if (!(fgets(ln, sizeof(ln), f))) {
        return AVAHI_RESOLVE_RESULT_UNAVAIL;
    }

    if (ln[0] != '+') {
        return AVAHI_RESOLVE_RESULT_HOST_NOT_FOUND;
    }

    result->af = af;

    p = ln + 1;
    p += strspn(p, WHITESPACE);

    /* Store interface number */
    result->scopeid = (uint32_t)strtol(p, NULL, 0);
    p += strcspn(p, WHITESPACE);
    p += strspn(p, WHITESPACE);

    /* Skip protocol */
    p += strcspn(p, WHITESPACE);
    p += strspn(p, WHITESPACE);

    /* Skip host name */
    p += strcspn(p, WHITESPACE);
    p += strspn(p, WHITESPACE);

    /* Cut off end of line */
    *(p + strcspn(p, "\n\r\t ")) = 0;

    if (inet_pton(af, p, &(result->address)) <= 0) {
        return AVAHI_RESOLVE_RESULT_UNAVAIL;
    }

    return AVAHI_RESOLVE_RESULT_SUCCESS;
}

avahi_resolve_result_t avahi_resolve_name(int af, const char* name,
                                          query_address_result_t* result) {
    if (af != AF_INET && af != AF_INET6) {
        return AVAHI_RESOLVE_RESULT_UNAVAIL;
    }

    FILE* f = open_socket();
    if (!f) {
        return AVAHI_RESOLVE_RESULT_UNAVAIL;
    }

    avahi_resolve_result_t ret =
        avahi_resolve_name_with_socket(f, af, name, result);
    fclose(f);
    return ret;
}

static avahi_resolve_result_t
avahi_resolve_address_with_socket(FILE* f, int af, const void* data, char* name,
                                  size_t name_len) {
    char* p;
    char a[256], ln[256];

    fprintf(f, "RESOLVE-ADDRESS %s\n", inet_ntop(af, data, a, sizeof(a)));

    if (!(fgets(ln, sizeof(ln), f))) {
        return AVAHI_RESOLVE_RESULT_UNAVAIL;
    }

    if (ln[0] != '+') {
        return AVAHI_RESOLVE_RESULT_HOST_NOT_FOUND;
    }

    p = ln + 1;
    p += strspn(p, WHITESPACE);

    /* Skip interface */
    p += strcspn(p, WHITESPACE);
    p += strspn(p, WHITESPACE);

    /* Skip protocol */
    p += strcspn(p, WHITESPACE);
    p += strspn(p, WHITESPACE);

    /* Cut off end of line */
    *(p + strcspn(p, "\n\r\t ")) = 0;

    strncpy(name, p, name_len - 1);
    name[name_len - 1] = 0;

    // Success.
    return AVAHI_RESOLVE_RESULT_SUCCESS;
}

avahi_resolve_result_t avahi_resolve_address(int af, const void* data,
                                             char* name, size_t name_len) {
    if (af != AF_INET && af != AF_INET6) {
        return AVAHI_RESOLVE_RESULT_UNAVAIL;
    }

    FILE* f = open_socket();
    if (!f) {
        return AVAHI_RESOLVE_RESULT_UNAVAIL;
    }

    avahi_resolve_result_t ret =
        avahi_resolve_address_with_socket(f, af, data, name, name_len);
    fclose(f);
    return ret;
}
