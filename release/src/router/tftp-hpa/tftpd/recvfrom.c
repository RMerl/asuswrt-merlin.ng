/* ----------------------------------------------------------------------- *
 *
 *   Copyright 2001-2006 H. Peter Anvin - All Rights Reserved
 *
 *   This program is free software available under the same license
 *   as the "OpenBSD" operating system, distributed at
 *   http://www.openbsd.org/.
 *
 * ----------------------------------------------------------------------- */

/*
 * recvfrom.c
 *
 * Emulate recvfrom() using recvmsg(), but try to capture the local address
 * since some TFTP clients consider it an error to get the reply from another
 * IP address than the request was sent to.
 *
 */

#include "config.h"             /* Must be included first! */
#include "common/tftpsubs.h"
#include "recvfrom.h"
#ifdef HAVE_MACHINE_PARAM_H
#include <machine/param.h>      /* Needed on some versions of FreeBSD */
#endif

#if defined(HAVE_RECVMSG) && defined(HAVE_MSGHDR_MSG_CONTROL)

#include <sys/uio.h>

#ifdef IP_PKTINFO
# ifndef HAVE_STRUCT_IN_PKTINFO
#  ifdef __linux__
/* Assume this version of glibc simply lacks the definition */
struct in_pktinfo {
    int ipi_ifindex;
    struct in_addr ipi_spec_dst;
    struct in_addr ipi_addr;
};
#  else
#   undef IP_PKTINFO            /* No definition, no way to get it */
#  endif
# endif
#endif

#ifndef CMSG_LEN
# define CMSG_LEN(size)	 (sizeof(struct cmsghdr) + (size))
#endif
#ifndef CMSG_SPACE
# define CMSG_SPACE(size) (sizeof(struct cmsghdr) + (size))
#endif

/*
 * Check to see if this is a valid local address, meaning that we can
 * legally bind to it.
 */
static int address_is_local(const union sock_addr *addr)
{
    union sock_addr sa1, sa2;
    int sockfd = -1;
    int e;
    int rv = 0;
    socklen_t addrlen;

    memcpy(&sa1, addr, sizeof sa1);

    /* Multicast or universal broadcast address? */
    if (sa1.sa.sa_family == AF_INET) {
        if (ntohl(sa1.si.sin_addr.s_addr) >= (224UL << 24))
            return 0;
	sa1.si.sin_port = 0;	/* Any port */
    }
#ifdef HAVE_IPV6
    else if (sa1.sa.sa_family == AF_INET6) {
        if (IN6_IS_ADDR_MULTICAST(&sa1.s6.sin6_addr))
            return 0;
	sa1.s6.sin6_port = 0;	/* Any port */
    }
#endif
    else
        return 0;

    sockfd = socket(sa1.sa.sa_family, SOCK_DGRAM, 0);
    if (sockfd < 0)
        goto err;

    if (bind(sockfd, &sa1.sa, SOCKLEN(&sa1)))
        goto err;

    addrlen = SOCKLEN(addr);
    if (getsockname(sockfd, (struct sockaddr *)&sa2, &addrlen))
        goto err;

    if (sa1.sa.sa_family != sa2.sa.sa_family)
	goto err;

    if (sa2.sa.sa_family == AF_INET)
        rv = sa1.si.sin_addr.s_addr == sa2.si.sin_addr.s_addr;
#ifdef HAVE_IPV6
    else if (sa2.sa.sa_family == AF_INET6)
        rv = IN6_ARE_ADDR_EQUAL(&sa1.s6.sin6_addr, &sa2.s6.sin6_addr);
#endif
    else
        rv = 0;

err:
    e = errno;

    if (sockfd >= 0)
        close(sockfd);

    errno = e;
    return rv;
}

int
myrecvfrom(int s, void *buf, int len, unsigned int flags,
           struct sockaddr *from, socklen_t * fromlen,
           union sock_addr *myaddr)
{
    struct msghdr msg;
    struct iovec iov;
    int n;
    struct cmsghdr *cmptr;
    union {
        struct cmsghdr cm;
#ifdef IP_PKTINFO
        char control[CMSG_SPACE(sizeof(struct in_addr)) +
                     CMSG_SPACE(sizeof(struct in_pktinfo))];
#else
        char control[CMSG_SPACE(sizeof(struct in_addr))];
#endif
#ifdef HAVE_IPV6
#ifdef HAVE_STRUCT_IN6_PKTINFO
        char control6[CMSG_SPACE(sizeof(struct in6_addr)) +
                     CMSG_SPACE(sizeof(struct in6_pktinfo))];
#else
        char control6[CMSG_SPACE(sizeof(struct in6_addr))];
#endif
#endif
    } control_un;
    int on = 1;
#ifdef IP_PKTINFO
    struct in_pktinfo pktinfo;
#endif
#ifdef HAVE_STRUCT_IN6_PKTINFO
    struct in6_pktinfo pktinfo6;
#endif

    /* Try to enable getting the return address */
#ifdef IP_RECVDSTADDR
    if (from->sa_family == AF_INET)
        setsockopt(s, IPPROTO_IP, IP_RECVDSTADDR, &on, sizeof(on));
#endif
#ifdef IP_PKTINFO
    if (from->sa_family == AF_INET)
        setsockopt(s, IPPROTO_IP, IP_PKTINFO, &on, sizeof(on));
#endif
#ifdef HAVE_IPV6
#ifdef IPV6_RECVPKTINFO
    if (from->sa_family == AF_INET6)
        setsockopt(s, IPPROTO_IPV6, IPV6_RECVPKTINFO, &on, sizeof(on));
#endif
#endif
    bzero(&msg, sizeof msg);    /* Clear possible system-dependent fields */
    msg.msg_control = control_un.control;
    msg.msg_controllen = sizeof(control_un);
    msg.msg_flags = 0;

    msg.msg_name = from;
    msg.msg_namelen = *fromlen;
    iov.iov_base = buf;
    iov.iov_len = len;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    if ((n = recvmsg(s, &msg, flags)) < 0)
        return n;               /* Error */

    *fromlen = msg.msg_namelen;

    if (myaddr) {
        bzero(myaddr, sizeof(*myaddr));
        myaddr->sa.sa_family = from->sa_family;

        if (msg.msg_controllen < sizeof(struct cmsghdr) ||
            (msg.msg_flags & MSG_CTRUNC))
            return n;           /* No information available */

        for (cmptr = CMSG_FIRSTHDR(&msg); cmptr != NULL;
             cmptr = CMSG_NXTHDR(&msg, cmptr)) {

            if (from->sa_family == AF_INET) {
                myaddr->sa.sa_family = AF_INET;
#ifdef IP_RECVDSTADDR
                if (cmptr->cmsg_level == IPPROTO_IP &&
                    cmptr->cmsg_type == IP_RECVDSTADDR) {
                    memcpy(&myaddr->si.sin_addr, CMSG_DATA(cmptr),
                           sizeof(struct in_addr));
                }
#endif

#ifdef IP_PKTINFO
                if (cmptr->cmsg_level == IPPROTO_IP &&
                    cmptr->cmsg_type == IP_PKTINFO) {
                    memcpy(&pktinfo, CMSG_DATA(cmptr),
                           sizeof(struct in_pktinfo));
                    memcpy(&myaddr->si.sin_addr, &pktinfo.ipi_addr,
                           sizeof(struct in_addr));
                }
#endif
            }
#ifdef HAVE_IPV6
            else if (from->sa_family == AF_INET6) {
                myaddr->sa.sa_family = AF_INET6;
#ifdef IP6_RECVDSTADDR
                if (cmptr->cmsg_level == IPPROTO_IPV6 &&
                    cmptr->cmsg_type == IPV6_RECVDSTADDR )
                    memcpy(&myaddr->s6.sin6_addr, CMSG_DATA(cmptr),
                           sizeof(struct in6_addr));
#endif

#ifdef HAVE_STRUCT_IN6_PKTINFO
                if (cmptr->cmsg_level == IPPROTO_IPV6 &&
                    (cmptr->cmsg_type == IPV6_RECVPKTINFO ||
                     cmptr->cmsg_type == IPV6_PKTINFO)) {
                    memcpy(&pktinfo6, CMSG_DATA(cmptr),
                           sizeof(struct in6_pktinfo));
                    memcpy(&myaddr->s6.sin6_addr, &pktinfo6.ipi6_addr,
                           sizeof(struct in6_addr));
                }
#endif
            }
#endif
        }
        /* If the address is not a valid local address,
         * then bind to any address...
         */
        if (address_is_local(myaddr) != 1) {
            if (myaddr->sa.sa_family == AF_INET)
                ((struct sockaddr_in *)myaddr)->sin_addr.s_addr = INADDR_ANY;
#ifdef HAVE_IPV6
            else if (myaddr->sa.sa_family == AF_INET6)
                memset(&myaddr->s6.sin6_addr, 0, sizeof(struct in6_addr));
#endif
        }
    }
    return n;
}

#else                           /* pointless... */

int
myrecvfrom(int s, void *buf, int len, unsigned int flags,
           struct sockaddr *from, socklen_t * fromlen,
           union sock_addr *myaddr)
{
    /* There is no way we can get the local address, fudge it */

    bzero(myaddr, sizeof(*myaddr));
    myaddr->sa.sa_family = from->sa_family;
    sa_set_port(myaddr, htons(IPPORT_TFTP));

    return recvfrom(s, buf, len, flags, from, fromlen);
}

#endif
