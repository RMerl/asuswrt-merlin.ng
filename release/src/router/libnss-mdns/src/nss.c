/*
  This file is part of nss-mdns.

  nss-mdns is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published
  by the Free Software Foundation; either version 2 of the License,
  or (at your option) any later version.

  nss-mdns is distributed in the hope that it will be useful, but1
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

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <netdb.h>
#include <sys/socket.h>
#include <nss.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "avahi.h"
#include "util.h"
#include "nss.h"

static avahi_resolve_result_t do_avahi_resolve_name(int af, const char* name,
                                                    userdata_t* userdata) {
    bool ipv4_found = false;
    bool ipv6_found = false;

    if (af == AF_INET || af == AF_UNSPEC) {
        query_address_result_t address_result;
        switch (avahi_resolve_name(AF_INET, name, &address_result)) {
        case AVAHI_RESOLVE_RESULT_SUCCESS:
            append_address_to_userdata(&address_result, userdata);
            ipv4_found = true;
            break;

        case AVAHI_RESOLVE_RESULT_HOST_NOT_FOUND:
            break;

        case AVAHI_RESOLVE_RESULT_UNAVAIL:
            // Something went wrong, just fail.
            return AVAHI_RESOLVE_RESULT_UNAVAIL;
        }
    }

    if (af == AF_INET6 || af == AF_UNSPEC) {
        query_address_result_t address_result;
        switch (avahi_resolve_name(AF_INET6, name, &address_result)) {
        case AVAHI_RESOLVE_RESULT_SUCCESS:
            append_address_to_userdata(&address_result, userdata);
            ipv6_found = true;
            break;

        case AVAHI_RESOLVE_RESULT_HOST_NOT_FOUND:
            break;

        case AVAHI_RESOLVE_RESULT_UNAVAIL:
            // Something went wrong, just fail.
            return AVAHI_RESOLVE_RESULT_UNAVAIL;
        }
    }

    if (ipv4_found || ipv6_found) {
        return AVAHI_RESOLVE_RESULT_SUCCESS;
    } else {
        return AVAHI_RESOLVE_RESULT_HOST_NOT_FOUND;
    }
}

enum nss_status _nss_mdns_gethostbyname_impl(const char* name, int af,
                                             userdata_t* u, int* errnop,
                                             int* h_errnop) {

    FILE* mdns_allow_file = NULL;
    use_name_result_t result;

#ifdef NSS_IPV4_ONLY
    if (af == AF_UNSPEC) {
        af = AF_INET;
    }
#endif

#ifdef NSS_IPV6_ONLY
    if (af == AF_UNSPEC) {
        af = AF_INET6;
    }
#endif

#ifdef NSS_IPV4_ONLY
    if (af != AF_INET)
#elif NSS_IPV6_ONLY
    if (af != AF_INET6)
#else
    if (af != AF_INET && af != AF_INET6 && af != AF_UNSPEC)
#endif
    {
        *errnop = EINVAL;
        *h_errnop = NO_RECOVERY;
        return NSS_STATUS_UNAVAIL;
    }

    u->count = 0;

#ifndef MDNS_MINIMAL
    mdns_allow_file = fopen(MDNS_ALLOW_FILE, "r");
#endif
    result = verify_name_allowed_with_soa(name, mdns_allow_file,
                                          TEST_LOCAL_SOA_AUTO);
#ifndef MDNS_MINIMAL
    if (mdns_allow_file)
        fclose(mdns_allow_file);
#endif

    if (result == USE_NAME_RESULT_SKIP) {
        *errnop = EINVAL;
        *h_errnop = NO_RECOVERY;
        return NSS_STATUS_UNAVAIL;
    }

    switch (do_avahi_resolve_name(af, name, u)) {
    case AVAHI_RESOLVE_RESULT_SUCCESS:
        return NSS_STATUS_SUCCESS;

    case AVAHI_RESOLVE_RESULT_HOST_NOT_FOUND:
        *errnop = ETIMEDOUT;
        *h_errnop = HOST_NOT_FOUND;
        if (result == USE_NAME_RESULT_OPTIONAL) {
            /* continue to dns plugin if DNS .local zone is detected. */
            *h_errnop = TRY_AGAIN;
            return NSS_STATUS_UNAVAIL;
	}
        return NSS_STATUS_NOTFOUND;

    case AVAHI_RESOLVE_RESULT_UNAVAIL:
    default:
        *errnop = ETIMEDOUT;
        *h_errnop = NO_RECOVERY;
        return NSS_STATUS_UNAVAIL;
    }
}

#ifndef __FreeBSD__
enum nss_status _nss_mdns_gethostbyname4_r(const char* name,
                                           struct gaih_addrtuple** pat,
                                           char* buffer, size_t buflen,
                                           int* errnop, int* h_errnop,
                                           int32_t* ttlp) {

    (void)ttlp;

    userdata_t u;
    buffer_t buf;

    enum nss_status status =
        _nss_mdns_gethostbyname_impl(name, AF_UNSPEC, &u, errnop, h_errnop);
    if (status != NSS_STATUS_SUCCESS) {
        return status;
    }
    buffer_init(&buf, buffer, buflen);
    return convert_userdata_to_addrtuple(&u, name, pat, &buf, errnop, h_errnop);
}
#endif

enum nss_status _nss_mdns_gethostbyname3_r(const char* name, int af,
                                           struct hostent* result, char* buffer,
                                           size_t buflen, int* errnop,
                                           int* h_errnop, int32_t* ttlp,
                                           char** canonp) {

    (void)ttlp;
    (void)canonp;

    buffer_t buf;
    userdata_t u;

    // The interfaces for gethostbyname3_r and below do not actually support
    // returning results for more than one address family
    if (af == AF_UNSPEC) {
#ifdef NSS_IPV6_ONLY
        af = AF_INET6;
#else
        af = AF_INET;
#endif
    }

    enum nss_status status = _nss_mdns_gethostbyname_impl(name, af, &u, errnop, h_errnop);
    if (status != NSS_STATUS_SUCCESS) {
        return status;
    }
    buffer_init(&buf, buffer, buflen);
    return convert_userdata_for_name_to_hostent(&u, name, af, result, &buf,
                                                errnop, h_errnop);
}

enum nss_status _nss_mdns_gethostbyname2_r(const char* name, int af,
                                           struct hostent* result, char* buffer,
                                           size_t buflen, int* errnop,
                                           int* h_errnop) {

    return _nss_mdns_gethostbyname3_r(name, af, result, buffer, buflen, errnop,
                                      h_errnop, NULL, NULL);
}

enum nss_status _nss_mdns_gethostbyname_r(const char* name,
                                          struct hostent* result, char* buffer,
                                          size_t buflen, int* errnop,
                                          int* h_errnop) {

    return _nss_mdns_gethostbyname2_r(name, AF_UNSPEC, result, buffer, buflen,
                                      errnop, h_errnop);
}

enum nss_status _nss_mdns_gethostbyaddr_r(const void* addr, int len, int af,
                                          struct hostent* result, char* buffer,
                                          size_t buflen, int* errnop,
                                          int* h_errnop) {

    size_t address_length;
    char t[256];

    /* Check for address types */
    address_length =
        af == AF_INET ? sizeof(ipv4_address_t) : sizeof(ipv6_address_t);

    if (len < (int)address_length ||
#ifdef NSS_IPV4_ONLY
        af != AF_INET
#elif NSS_IPV6_ONLY
        af != AF_INET6
#else
        (af != AF_INET && af != AF_INET6)
#endif
    ) {
        *errnop = EINVAL;
        *h_errnop = NO_RECOVERY;
        return NSS_STATUS_UNAVAIL;
    }

#ifdef MDNS_MINIMAL
    /* Only query for 169.254.0.0/16 IPv4 in minimal mode */
    if ((af == AF_INET &&
         ((ntohl(*(const uint32_t*)addr) & 0xFFFF0000UL) != 0xA9FE0000UL)) ||
        (af == AF_INET6 && !(((const uint8_t*)addr)[0] == 0xFE &&
                             (((const uint8_t*)addr)[1] >> 6) == 2))) {
        *errnop = EINVAL;
        *h_errnop = NO_RECOVERY;
        return NSS_STATUS_UNAVAIL;
    }
#endif

    /* Lookup using Avahi */
    buffer_t buf;
    switch (avahi_resolve_address(af, addr, t, sizeof(t))) {
    case AVAHI_RESOLVE_RESULT_SUCCESS:
        buffer_init(&buf, buffer, buflen);
        return convert_name_and_addr_to_hostent(t, addr, address_length, af,
                                                result, &buf, errnop, h_errnop);

    case AVAHI_RESOLVE_RESULT_HOST_NOT_FOUND:
        *errnop = ETIMEDOUT;
        *h_errnop = HOST_NOT_FOUND;
        return NSS_STATUS_NOTFOUND;

    case AVAHI_RESOLVE_RESULT_UNAVAIL:
    default:
        *errnop = ETIMEDOUT;
        *h_errnop = NO_RECOVERY;
        return NSS_STATUS_UNAVAIL;
    }
}
