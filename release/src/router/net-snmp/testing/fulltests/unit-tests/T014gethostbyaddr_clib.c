/*
 * HEADER Testing netsnmp_gethostbyaddr() 
 */

SOCK_STARTUP;

{
    int             ran_test = 0;
#ifdef HAVE_GETHOSTBYADDR
    struct hostent *h, *h2 = NULL;
    struct in_addr  v4loop;
    struct sockaddr_in sin_addr;
    int             s;

    v4loop.s_addr = htonl(INADDR_LOOPBACK);
    memset(&sin_addr, 0, sizeof(sin_addr));
    sin_addr.sin_family = AF_INET;
    sin_addr.sin_addr = v4loop;
    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s >= 0) {
        if (bind(s, (struct sockaddr *) &sin_addr, sizeof(sin_addr)) >= 0) {
            h = netsnmp_gethostbyaddr(&v4loop, sizeof(v4loop), AF_INET);
            if (h)
                h2 = gethostbyname(h->h_name);
            OKF(h && (strcmp(h->h_name, "localhost") == 0 ||
                      (h2 && memcmp(h2->h_addr, &v4loop.s_addr,
                                    sizeof(v4loop.s_addr)) == 0)),
                ("127.0.0.1 lookup (%s -> %s)", h ? h->h_name : "(failed)",
                 h2 ? inet_ntoa(*(struct in_addr *) h2->
                                h_addr) : "(failed)"));
            ran_test = 1;
        }
        close(s);
    }
#endif
    if (!ran_test)
        OKF(1, ("Skipped IPv4 test"));
}

{
    struct hostent *h;
#ifdef cygwin
    static const struct in6_addr v6loop = { { IN6ADDR_LOOPBACK_INIT } };
#else
    static const struct in6_addr v6loop = IN6ADDR_LOOPBACK_INIT;
#endif
    struct sockaddr_in6 sin6_addr;
    struct addrinfo hints, *addr = NULL, *ap;
    char            buf[64];
    int             s, res, ran_test = 0;

    memset(&sin6_addr, 0, sizeof(sin6_addr));
    sin6_addr.sin6_family = AF_INET6;
    sin6_addr.sin6_addr = v6loop;
    s = socket(AF_INET6, SOCK_DGRAM, 0);
    if (s >= 0) {
        if (bind(s, (struct sockaddr *) &sin6_addr, sizeof(sin6_addr)) >=
            0) {
            addr = NULL;
            strcpy(buf, "(failed)");
            h = netsnmp_gethostbyaddr(&v6loop, sizeof(v6loop), AF_INET6);
            if (h) {
                memset(&hints, 0, sizeof(hints));
                hints.ai_family = AF_INET6;
                res = getaddrinfo(h->h_name, NULL, &hints, &addr);
                if (res == 0) {
                    for (ap = addr; ap; ap = ap->ai_next) {
                        if (ap->ai_family == AF_INET6) {
                            inet_ntop(ap->ai_family,
                                      &((struct sockaddr_in6 *) ap->
                                        ai_addr)->sin6_addr, buf,
                                      sizeof(buf));
                            break;
                        }
                    }
                    if (!ap)
                        strcpy(buf, "no AF_INET6 address found");
                } else {
                    snprintf(buf, sizeof(buf), "getaddrinfo() failed: %s",
                             strerror(errno));
                }
            }
            OKF(h && (strcmp(h->h_name, "localhost") == 0 ||
                      (res == 0 && addr && memcmp(addr->ai_addr, &sin6_addr,
                                                  sizeof(sin6_addr)) == 0)),
                ("::1 lookup (%s -> %s)", h ? h->h_name : "(failed)",
                 buf));
            if (addr)
                freeaddrinfo(addr);
            ran_test = 1;
        }
        close(s);
    }
    if (!ran_test)
        OKF(1, ("Skipped IPv6 test"));
}

SOCK_CLEANUP;
