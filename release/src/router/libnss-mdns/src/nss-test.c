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

#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#ifdef __FreeBSD__
#include <netinet/in.h>
#endif

static int gai(const char* node) {
    struct addrinfo hints;
    struct addrinfo *result, *rp;

    char str[INET_ADDRSTRLEN];
    char str6[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_CANONNAME;
    hints.ai_socktype = SOCK_STREAM;

    fprintf(stderr, "* doing node lookup with getaddrinfo...\n");

    int s = getaddrinfo(node, NULL, &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        return 1;
    }
    int i = 0;
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        if (rp->ai_canonname) {
            fprintf(stderr, "[%d] official name: %s\n", i, rp->ai_canonname);
        }
        switch (rp->ai_family) {
        case AF_INET:
            inet_ntop(AF_INET, &((struct sockaddr_in*)rp->ai_addr)->sin_addr,
                      str, sizeof(str));
            fprintf(stderr, "[%d] addr type: inet\n[%d] address: %s\n", i, i,
                    str);
            break;
        case AF_INET6:
            inet_ntop(AF_INET6, &((struct sockaddr_in6*)rp->ai_addr)->sin6_addr,
                      str6, sizeof(str6));
            int scope_id = ((struct sockaddr_in6*)rp->ai_addr)->sin6_scope_id;
            if (scope_id) {
                fprintf(stderr, "[%d] addr type: inet6\n[%d] address: %s%%%d\n",
                        i, i, str6, scope_id);
            } else {
                fprintf(stderr, "[%d] addr type: inet6\n[%d] address: %s\n", i,
                        i, str6);
            }
            break;
        }
        fprintf(stderr, "\n");
        i++;
    }
    freeaddrinfo(result);

    return 0;
}

static int gethostbyX(const char* node) {
    struct hostent* he;
    in_addr_t** a;
    uint8_t t[256];

    if (inet_pton(AF_INET, node, &t) > 0) {
        fprintf(stderr, "* doing ipv4 lookup with gethostbyaddr...\n");
        he = gethostbyaddr(t, 4, AF_INET);
    } else if (inet_pton(AF_INET6, node, &t) > 0) {
        fprintf(stderr, "* doing ipv6 lookup with gethostbyaddr...\n");
        he = gethostbyaddr(t, 16, AF_INET6);
    } else {
        fprintf(stderr, "* doing name lookup with gethostbyname...\n");
        he = gethostbyname(node);
    }

    if (!he) {
        fprintf(stderr, "lookup failed\n");
        return 1;
    }

    fprintf(stderr, "official name: %s\n", he->h_name);

    if (!he->h_aliases || !he->h_aliases[0])
        fprintf(stderr, "no aliases\n");
    else {
        char** h;
        fprintf(stderr, "aliases:");
        for (h = he->h_aliases; *h; h++)
            fprintf(stderr, " %s", *h);
        fprintf(stderr, "\n");
    }

    fprintf(stderr, "addr type: %s\n",
            he->h_addrtype == AF_INET
                ? "inet"
                : (he->h_addrtype == AF_INET6 ? "inet6" : NULL));
    fprintf(stderr, "addr length: %i\n", he->h_length);

    fprintf(stderr, "addresses:");
    for (a = (in_addr_t**)he->h_addr_list; *a; a++) {
        char txt[256];
        fprintf(stderr, " %s", inet_ntop(he->h_addrtype, *a, txt, sizeof(txt)));
    }
    fprintf(stderr, "\n");

    return 0;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr,
                "Requires 1 argument: either a host or numeric address\n");
        return 1;
    }
    const char* node = argv[1];
    gethostbyX(node);
    fprintf(stderr, "\n\n");
    gai(node);
    return 0;
}
