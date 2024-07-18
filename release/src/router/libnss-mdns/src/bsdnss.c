/*
  This file is part of nss-mdns.

  nss-mdns is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published
  by the Free Software Foundation; either version 2 of the License,
  or (at your option) any later version.

  nss-mdns is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with nss-mdns; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

/* Original author: Bruce M. Simpson <bms@FreeBSD.org> */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/param.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <sys/ktrace.h>

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <nss.h>

#include <netinet/in.h>
#include <netdb.h>

#include "avahi.h"
#include "config.h"
#include "util.h"
#include "nss.h"

#ifdef MDNS_MINIMAL
/*
 * FreeBSD support prefers Avahi.
 */
#endif

/*
 * To turn on utrace() records, compile with -DDEBUG_UTRACE.
 */
#ifdef DEBUG_UTRACE
#define _NSS_UTRACE(msg)                                                       \
    do {                                                                       \
        static const char __msg[] = msg;                                       \
        (void)utrace(__msg, sizeof(__msg));                                    \
    } while (0)
#else
#define _NSS_UTRACE(msg)
#endif

ns_mtab* nss_module_register(const char* source, unsigned int* mtabsize,
                             nss_module_unregister_fn* unreg);

typedef enum nss_status (*_bsd_nsstub_fn_t)(const char*, struct hostent*, char*,
                                            size_t, int*, int*);

static NSS_METHOD_PROTOTYPE(__nss_bsdcompat_getaddrinfo);
static NSS_METHOD_PROTOTYPE(__nss_bsdcompat_gethostbyaddr_r);
static NSS_METHOD_PROTOTYPE(__nss_bsdcompat_gethostbyname2_r);
static NSS_METHOD_PROTOTYPE(__nss_bsdcompat_ghbyaddr);
static NSS_METHOD_PROTOTYPE(__nss_bsdcompat_ghbyname);

static ns_mtab methods[] = {
    /* database, name, method, mdata */
    {NSDB_HOSTS, "getaddrinfo", __nss_bsdcompat_getaddrinfo, NULL},
    {NSDB_HOSTS, "gethostbyaddr_r", __nss_bsdcompat_gethostbyaddr_r, NULL},
    {NSDB_HOSTS, "gethostbyname2_r", __nss_bsdcompat_gethostbyname2_r, NULL},
    {NSDB_HOSTS, "ghbyaddr", __nss_bsdcompat_ghbyaddr, NULL},
    {NSDB_HOSTS, "ghbyname", __nss_bsdcompat_ghbyname, NULL},
};

ns_mtab* nss_module_register(const char* source, unsigned int* mtabsize,
                             nss_module_unregister_fn* unreg) {

    *mtabsize = sizeof(methods) / sizeof(methods[0]);
    *unreg = NULL;
    return (methods);
}

/*
 * Calling convention:
 * ap: const char *name (optional), struct addrinfo *pai (hints, optional)
 * retval: struct addrinfo **
 *
 * name must always be specified by libc; pai is allocated
 * by libc and must always be specified.
 *
 * We can malloc() addrinfo instances and hang them off ai->next;
 * canonnames may also be malloc()'d.
 * libc is responsible for mapping our ns error return to gai_strerror().
 *
 * libc calls us only to look up qualified hostnames. We don't need to
 * worry about port numbers; libc will call getservbyname() and explore
 * the appropriate maps configured in nsswitch.conf(5).
 *
 * _errno and _h_errno are unused by getaddrinfo(), as it is
 * [mostly] OS independent interface implemented by Win32.
 */
static int __nss_bsdcompat_getaddrinfo(void* retval, void* mdata __unused,
                                       va_list ap) {
    enum nss_status status;
    int _errno = 0;
    int _h_errno = 0;
    struct addrinfo sentinel = {0}, *curp = &sentinel;
    const char* name;
    const struct addrinfo* pai;
    struct addrinfo** resultp;
    userdata_t u;

    _NSS_UTRACE("__nss_bsdcompat_getaddrinfo: called");

    name = va_arg(ap, const char*);
    pai = va_arg(ap, struct addrinfo*);
    resultp = (struct addrinfo**)retval;

    if (name == NULL || pai == NULL) {
        *resultp = NULL;
        return (NS_UNAVAIL);
    }

    extern enum nss_status _nss_mdns_gethostbyname_impl(
        const char* name, int af, userdata_t* u, int* errnop, int* h_errnop);

    status = _nss_mdns_gethostbyname_impl(name, pai->ai_family, &u, &_errno,
                                          &_h_errno);
    status = __nss_compat_result(status, _errno);
    if (status != NS_SUCCESS) {
        return (status);
    }

    for (int i = 0; i < u.count; i++) {
        struct addrinfo* ai = (struct addrinfo*)malloc(
            sizeof(struct addrinfo) + sizeof(struct sockaddr_storage));
        if (ai == NULL) {
            if (sentinel.ai_next != NULL)
                freeaddrinfo(sentinel.ai_next);
            *resultp = NULL;
            return (NS_UNAVAIL);
        }
        struct sockaddr* psa = (struct sockaddr*)(ai + 1);

        memset(ai, 0, sizeof(struct addrinfo));
        ai->ai_flags = pai->ai_flags;
        ai->ai_socktype = pai->ai_socktype;
        ai->ai_protocol = pai->ai_protocol;
        ai->ai_family = u.result[i].af;
        memset(psa, 0, sizeof(struct sockaddr_storage));
        psa->sa_len = ai->ai_addrlen;
        psa->sa_family = ai->ai_family;
        ai->ai_addr = psa;
        switch (ai->ai_family) {
        case AF_INET:
            ai->ai_addrlen = sizeof(struct sockaddr_in);
            memcpy(&((struct sockaddr_in*)psa)->sin_addr, &u.result[i].address,
                   ai->ai_addrlen);
            break;
        case AF_INET6:
            ai->ai_addrlen = sizeof(struct sockaddr_in6);
            memcpy(&((struct sockaddr_in6*)psa)->sin6_addr,
                   &u.result[i].address, ai->ai_addrlen);
            break;
        default:
            ai->ai_addrlen = sizeof(struct sockaddr_storage);
            memcpy(psa->sa_data, &u.result[i].address, ai->ai_addrlen);
        }

        curp->ai_next = ai;
        curp = ai;
    }

    *resultp = sentinel.ai_next;
    return (status);
}

/*
 * Calling convention:
 * ap: const u_char *uaddr, socklen_t len, int af, struct hostent *hp,
 *     char *buf, size_t buflen, int ret_errno, int *h_errnop
 * retval: should be set to NULL or hp passed in
 */
static int __nss_bsdcompat_gethostbyaddr_r(void* retval, void* mdata __unused,
                                           va_list ap) {
    void* addr;
    char* buf;
    int* h_errnop;
    struct hostent* hp;
    struct hostent** resultp;
    int af;
    size_t buflen;
    int len;
    int ret_errno;
    enum nss_status status;

    addr = va_arg(ap, void*);
    len = va_arg(ap, socklen_t);
    af = va_arg(ap, int);
    hp = va_arg(ap, struct hostent*);
    buf = va_arg(ap, char*);
    buflen = va_arg(ap, size_t);
    ret_errno = va_arg(ap, int);
    h_errnop = va_arg(ap, int*);
    resultp = (struct hostent**)retval;

    *resultp = NULL;
    status = _nss_mdns_gethostbyaddr_r(addr, len, af, hp, buf, buflen,
                                       &ret_errno, h_errnop);

    status = __nss_compat_result(status, *h_errnop);
    if (status == NS_SUCCESS)
        *resultp = hp;
    return (status);
}

/*
 * Calling convention:
 * ap: const char *name, int af, struct hostent *hp, char *buf,
 *     size_t buflen, int ret_errno, int *h_errnop
 * retval is a struct hostent **result passed in by the libc client,
 * which is responsible for allocating storage.
 */
static int __nss_bsdcompat_gethostbyname2_r(void* retval, void* mdata __unused,
                                            va_list ap) {
    char* buf;
    const char* name;
    int* h_errnop;
    struct hostent* hp;
    struct hostent** resultp;
    int af;
    size_t buflen;
    int ret_errno;
    enum nss_status status;

    name = va_arg(ap, char*);
    af = va_arg(ap, int);
    hp = va_arg(ap, struct hostent*);
    buf = va_arg(ap, char*);
    buflen = va_arg(ap, size_t);
    ret_errno = va_arg(ap, int);
    h_errnop = va_arg(ap, int*);
    resultp = (struct hostent**)retval;

    *resultp = NULL;
    if (hp == NULL)
        return (NS_UNAVAIL);

    status = _nss_mdns_gethostbyname2_r(name, af, hp, buf, buflen, &ret_errno,
                                        h_errnop);

    status = __nss_compat_result(status, *h_errnop);
    if (status == NS_SUCCESS)
        *resultp = hp;
    return (status);
}

/*
 * Used by getipnodebyaddr(3).
 *
 * Calling convention:
 * ap: struct in[6]_addr *src, size_t len, int af, int *errp
 * retval: pointer to a pointer to an uninitialized struct hostent,
 * in which should be returned a single pointer to on-heap storage.
 *
 * This function is responsible for allocating on-heap storage.
 * The caller is responsible for calling freehostent() on the returned
 * storage.
 */
static int __nss_bsdcompat_ghbyaddr(void* retval, void* mdata __unused,
                                    va_list ap) {
    char* buffer;
    void* bufp;
    int* errp;
    struct hostent* hp;
    struct hostent** resultp;
    void* src;
    int af;
    size_t buflen = 1024;
    size_t len;
    int h_errnop;
    enum nss_status status;

    src = va_arg(ap, void*);
    len = va_arg(ap, size_t);
    af = va_arg(ap, int);
    errp = va_arg(ap, int*);
    resultp = (struct hostent**)retval;

    _NSS_UTRACE("__nss_bsdcompat_ghbyaddr: called");

    bufp = malloc((sizeof(struct hostent) + buflen));
    if (bufp == NULL) {
        *resultp = NULL;
        return (NS_UNAVAIL);
    }
    hp = (struct hostent*)bufp;
    buffer = (char*)(hp + 1);

    status = _nss_mdns_gethostbyaddr_r(src, len, af, hp, buffer, buflen, errp,
                                       &h_errnop);

    status = __nss_compat_result(status, *errp);
    if (status != NS_SUCCESS) {
        free(bufp);
        hp = NULL;
    }
    *resultp = hp;
    return (status);
}

/*
 * Used by getipnodebyname(3).
 *
 * Calling convention:
 * ap: const char *name, int af, int *errp
 * retval: pointer to a pointer to an uninitialized struct hostent.
 *
 * This function is responsible for allocating on-heap storage.
 * The caller is responsible for calling freehostent() on the returned
 * storage.
 */
static int __nss_bsdcompat_ghbyname(void* retval, void* mdata __unused,
                                    va_list ap) {
    char* buffer;
    void* bufp;
    int* errp;
    struct hostent* hp;
    struct hostent** resultp;
    char* name;
    int af;
    size_t buflen = 1024;
    int h_errnop;
    enum nss_status status;

    name = va_arg(ap, char*);
    af = va_arg(ap, int);
    errp = va_arg(ap, int*);
    resultp = (struct hostent**)retval;

    bufp = malloc((sizeof(struct hostent) + buflen));
    if (bufp == NULL) {
        *resultp = NULL;
        return (NS_UNAVAIL);
    }
    hp = (struct hostent*)bufp;
    buffer = (char*)(hp + 1);

    status =
        _nss_mdns_gethostbyname_r(name, hp, buffer, buflen, errp, &h_errnop);

    status = __nss_compat_result(status, *errp);
    if (status != NS_SUCCESS) {
        free(bufp);
        hp = NULL;
    }
    *resultp = hp;
    return (status);
}
