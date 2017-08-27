/*
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

/**
 * $Id$
 * @file udpfromto.c
 * @brief Like recvfrom, but also stores the destination IP address. Useful on multihomed hosts.
 *
 * @copyright 2007 Alan DeKok <aland@deployingradius.com>
 * @copyright 2002 Miquel van Smoorenburg
 */
RCSID("$Id$")

#include <freeradius-devel/udpfromto.h>

#ifdef WITH_UDPFROMTO

#ifdef HAVE_SYS_UIO_H
#  include <sys/uio.h>
#endif

#include <fcntl.h>

/*
 *	More portability idiocy
 *	Mac OSX Lion doesn't define SOL_IP.  But IPPROTO_IP works.
 */
#ifndef SOL_IP
#  define SOL_IP IPPROTO_IP
#endif

/*
 * glibc 2.4 and uClibc 0.9.29 introduce IPV6_RECVPKTINFO etc. and
 * change IPV6_PKTINFO This is only supported in Linux kernel >=
 * 2.6.14
 *
 * This is only an approximation because the kernel version that libc
 * was compiled against could be older or newer than the one being
 * run.  But this should not be a problem -- we just keep using the
 * old kernel interface.
 */
#ifdef __linux__
#  ifdef IPV6_RECVPKTINFO
#    include <linux/version.h>
#    if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,14)
#      ifdef IPV6_2292PKTINFO
#        undef IPV6_RECVPKTINFO
#        undef IPV6_PKTINFO
#        define IPV6_RECVPKTINFO IPV6_2292PKTINFO
#        define IPV6_PKTINFO IPV6_2292PKTINFO
#      endif
#    endif
/* Fall back to the legacy socket option if IPV6_RECVPKTINFO isn't defined */
#  elif defined(IPV6_2292PKTINFO)
#      define IPV6_RECVPKTINFO IPV6_2292PKTINFO
#  endif
#endif

/*
 *	Linux requires IPV6_RECVPKTINFO for the setsockopt() call,
 *	but sendmsg() and recvmsg() require IPV6_PKTINFO. <sigh>
 *
 *	We want all *other* (i.e. sane) systems to use IPV6_PKTINFO
 *	for all three calls.
 */
#ifdef IPV6_PKTINFO
#  ifdef __linux__
#    define FR_IPV6_RECVPKTINFO IPV6_RECVPKTINFO
#  else
#    define FR_IPV6_RECVPKTINFO IPV6_PKTINFO
#  endif
#endif

int udpfromto_init(int s)
{
	int proto, flag, opt = 1;
	struct sockaddr_storage si;
	socklen_t si_len = sizeof(si);

	errno = ENOSYS;

	proto = -1;

	if (getsockname(s, (struct sockaddr *) &si, &si_len) < 0) {
		return -1;
	}

	if (si.ss_family == AF_INET) {
#ifdef HAVE_IP_PKTINFO
		/*
		 *	Linux
		 */
		proto = SOL_IP;
		flag = IP_PKTINFO;
#endif

#ifdef IP_RECVDSTADDR
		/*
		 *	Set the IP_RECVDSTADDR option (BSD).  Note:
		 *	IP_RECVDSTADDR == IP_SENDSRCADDR
		 */
		proto = IPPROTO_IP;
		flag = IP_RECVDSTADDR;
#endif

#ifdef AF_INET6
	} else if (si.ss_family == AF_INET6) {
#  ifdef IPV6_PKTINFO
		/*
		 *	This should actually be standard IPv6
		 */
		proto = IPPROTO_IPV6;

		/*
		 *	Work around Linux-specific hackery.
		 */
		flag = FR_IPV6_RECVPKTINFO;
#  endif
#endif
	} else {
		/*
		 *	Unknown AF.
		 */
		return -1;
	}

	/*
	 *	Unsupported.  Don't worry about it.
	 */
	if (proto < 0) return 0;

	return setsockopt(s, proto, flag, &opt, sizeof(opt));
}

int recvfromto(int s, void *buf, size_t len, int flags,
	       struct sockaddr *from, socklen_t *fromlen,
	       struct sockaddr *to, socklen_t *tolen)
{
	struct msghdr msgh;
	struct cmsghdr *cmsg;
	struct iovec iov;
	char cbuf[256];
	int err;
	struct sockaddr_storage si;
	socklen_t si_len = sizeof(si);

#if !defined(IP_PKTINFO) && !defined(IP_RECVDSTADDR) && !defined (IPV6_PKTINFO)
	/*
	 *	If the recvmsg() flags aren't defined, fall back to
	 *	using recvfrom().
	 */
	to = NULL:
#endif

	/*
	 *	Catch the case where the caller passes invalid arguments.
	 */
	if (!to || !tolen) return recvfrom(s, buf, len, flags, from, fromlen);

	/*
	 *	recvmsg doesn't provide sin_port so we have to
	 *	retrieve it using getsockname().
	 */
	if (getsockname(s, (struct sockaddr *)&si, &si_len) < 0) {
		return -1;
	}

	/*
	 *	Initialize the 'to' address.  It may be INADDR_ANY here,
	 *	with a more specific address given by recvmsg(), below.
	 */
	if (si.ss_family == AF_INET) {
#if !defined(IP_PKTINFO) && !defined(IP_RECVDSTADDR)
		return recvfrom(s, buf, len, flags, from, fromlen);
#else
		struct sockaddr_in *dst = (struct sockaddr_in *) to;
		struct sockaddr_in *src = (struct sockaddr_in *) &si;

		if (*tolen < sizeof(*dst)) {
			errno = EINVAL;
			return -1;
		}
		*tolen = sizeof(*dst);
		*dst = *src;
#endif
	}

#ifdef AF_INET6
	else if (si.ss_family == AF_INET6) {
#if !defined(IPV6_PKTINFO)
		return recvfrom(s, buf, len, flags, from, fromlen);
#else
		struct sockaddr_in6 *dst = (struct sockaddr_in6 *) to;
		struct sockaddr_in6 *src = (struct sockaddr_in6 *) &si;

		if (*tolen < sizeof(*dst)) {
			errno = EINVAL;
			return -1;
		}
		*tolen = sizeof(*dst);
		*dst = *src;
#endif
	}
#endif
	/*
	 *	Unknown address family.
	 */
	else {
		errno = EINVAL;
		return -1;
	}

	/* Set up iov and msgh structures. */
	memset(&msgh, 0, sizeof(struct msghdr));
	iov.iov_base = buf;
	iov.iov_len  = len;
	msgh.msg_control = cbuf;
	msgh.msg_controllen = sizeof(cbuf);
	msgh.msg_name = from;
	msgh.msg_namelen = fromlen ? *fromlen : 0;
	msgh.msg_iov  = &iov;
	msgh.msg_iovlen = 1;
	msgh.msg_flags = 0;

	/* Receive one packet. */
	if ((err = recvmsg(s, &msgh, flags)) < 0) {
		return err;
	}

	if (fromlen) *fromlen = msgh.msg_namelen;

	/* Process auxiliary received data in msgh */
	for (cmsg = CMSG_FIRSTHDR(&msgh);
	     cmsg != NULL;
	     cmsg = CMSG_NXTHDR(&msgh,cmsg)) {

#ifdef IP_PKTINFO
		if ((cmsg->cmsg_level == SOL_IP) &&
		    (cmsg->cmsg_type == IP_PKTINFO)) {
			struct in_pktinfo *i =
				(struct in_pktinfo *) CMSG_DATA(cmsg);
			((struct sockaddr_in *)to)->sin_addr = i->ipi_addr;
			*tolen = sizeof(struct sockaddr_in);
			break;
		}
#endif

#ifdef IP_RECVDSTADDR
		if ((cmsg->cmsg_level == IPPROTO_IP) &&
		    (cmsg->cmsg_type == IP_RECVDSTADDR)) {
			struct in_addr *i = (struct in_addr *) CMSG_DATA(cmsg);
			((struct sockaddr_in *)to)->sin_addr = *i;
			*tolen = sizeof(struct sockaddr_in);
			break;
		}
#endif

#ifdef IPV6_PKTINFO
		if ((cmsg->cmsg_level == IPPROTO_IPV6) &&
		    (cmsg->cmsg_type == IPV6_PKTINFO)) {
			struct in6_pktinfo *i =
				(struct in6_pktinfo *) CMSG_DATA(cmsg);
			((struct sockaddr_in6 *)to)->sin6_addr = i->ipi6_addr;
			*tolen = sizeof(struct sockaddr_in6);
			break;
		}
#endif
	}

	return err;
}

int sendfromto(int s, void *buf, size_t len, int flags,
	       struct sockaddr *from, socklen_t fromlen,
	       struct sockaddr *to, socklen_t tolen)
{
	struct msghdr msgh;
	struct cmsghdr *cmsg;
	struct iovec iov;
	char cbuf[256];

#if !defined(IP_PKTINFO) && !defined(IP_SENDSRCADDR) && !defined(IPV6_PKTINFO)
	/*
	 *	If the sendmsg() flags aren't defined, fall back to
	 *	using sendto().
	 */
	from = NULL;
#endif

	/*
	 *	Catch the case where the caller passes invalid arguments.
	 */
	if (!from || (fromlen == 0) || (from->sa_family == AF_UNSPEC)) {
		return sendto(s, buf, len, flags, to, tolen);
	}

	/* Set up iov and msgh structures. */
	memset(&msgh, 0, sizeof(struct msghdr));
	iov.iov_base = buf;
	iov.iov_len = len;
	msgh.msg_iov = &iov;
	msgh.msg_iovlen = 1;
	msgh.msg_name = to;
	msgh.msg_namelen = tolen;

	if (from->sa_family == AF_INET) {
#if !defined(IP_PKTINFO) && !defined(IP_SENDSRCADDR)
		return sendto(s, buf, len, flags, to, tolen);
#else
		struct sockaddr_in *s4 = (struct sockaddr_in *) from;

#  ifdef IP_PKTINFO
		struct in_pktinfo *pkt;

		msgh.msg_control = cbuf;
		msgh.msg_controllen = CMSG_SPACE(sizeof(*pkt));

		cmsg = CMSG_FIRSTHDR(&msgh);
		cmsg->cmsg_level = SOL_IP;
		cmsg->cmsg_type = IP_PKTINFO;
		cmsg->cmsg_len = CMSG_LEN(sizeof(*pkt));

		pkt = (struct in_pktinfo *) CMSG_DATA(cmsg);
		memset(pkt, 0, sizeof(*pkt));
		pkt->ipi_spec_dst = s4->sin_addr;
#  endif

#  ifdef IP_SENDSRCADDR
		struct in_addr *in;

		msgh.msg_control = cbuf;
		msgh.msg_controllen = CMSG_SPACE(sizeof(*in));

		cmsg = CMSG_FIRSTHDR(&msgh);
		cmsg->cmsg_level = IPPROTO_IP;
		cmsg->cmsg_type = IP_SENDSRCADDR;
		cmsg->cmsg_len = CMSG_LEN(sizeof(*in));

		in = (struct in_addr *) CMSG_DATA(cmsg);
		*in = s4->sin_addr;
#  endif
#endif	/* IP_PKTINFO or IP_SENDSRCADDR */
	}

#ifdef AF_INET6
	else if (from->sa_family == AF_INET6) {
#  if !defined(IPV6_PKTINFO)
		return sendto(s, buf, len, flags, to, tolen);
#  else
		struct sockaddr_in6 *s6 = (struct sockaddr_in6 *) from;

		struct in6_pktinfo *pkt;

		msgh.msg_control = cbuf;
		msgh.msg_controllen = CMSG_SPACE(sizeof(*pkt));

		cmsg = CMSG_FIRSTHDR(&msgh);
		cmsg->cmsg_level = IPPROTO_IPV6;
		cmsg->cmsg_type = IPV6_PKTINFO;
		cmsg->cmsg_len = CMSG_LEN(sizeof(*pkt));

		pkt = (struct in6_pktinfo *) CMSG_DATA(cmsg);
		memset(pkt, 0, sizeof(*pkt));
		pkt->ipi6_addr = s6->sin6_addr;
#  endif	/* IPV6_PKTINFO */
	}
#endif

	/*
	 *	Unknown address family.
	 */
	else {
		errno = EINVAL;
		return -1;
	}

	return sendmsg(s, &msgh, flags);
}


#ifdef TESTING
/*
 *	Small test program to test recvfromto/sendfromto
 *
 *	use a virtual IP address as first argument to test
 *
 *	reply packet should originate from virtual IP and not
 *	from the default interface the alias is bound to
 */
#  include <sys/wait.h>

#  define DEF_PORT 20000		/* default port to listen on */
#  define DESTIP "127.0.0.1"	/* send packet to localhost per default */
#  define TESTSTRING "foo"	/* what to send */
#  define TESTLEN 4			/* 4 bytes */

int main(int argc, char **argv)
{
	struct sockaddr_in from, to, in;
	char buf[TESTLEN];
	char *destip = DESTIP;
	int port = DEF_PORT;
	int n, server_socket, client_socket, fl, tl, pid;

	if (argc > 1) destip = argv[1];
	if (argc > 2) port = atoi(argv[2]);

	in.sin_family = AF_INET;
	in.sin_addr.s_addr = INADDR_ANY;
	in.sin_port = htons(port);
	fl = tl = sizeof(struct sockaddr_in);
	memset(&from, 0, sizeof(from));
	memset(&to,   0, sizeof(to));

	switch(pid = fork()) {
		case -1:
			perror("fork");
			return 0;
		case 0:
			/* child */
			usleep(100000);
			goto client;
	}

	/* parent: server */
	server_socket = socket(PF_INET, SOCK_DGRAM, 0);
	if (udpfromto_init(server_socket) != 0) {
		perror("udpfromto_init\n");
		waitpid(pid, NULL, WNOHANG);
		return 0;
	}

	if (bind(server_socket, (struct sockaddr *)&in, sizeof(in)) < 0) {
		perror("server: bind");
		waitpid(pid, NULL, WNOHANG);
		return 0;
	}

	printf("server: waiting for packets on INADDR_ANY:%d\n", port);
	if ((n = recvfromto(server_socket, buf, sizeof(buf), 0,
	    (struct sockaddr *)&from, &fl,
	    (struct sockaddr *)&to, &tl)) < 0) {
		perror("server: recvfromto");
		waitpid(pid, NULL, WNOHANG);
		return 0;
	}

	printf("server: received a packet of %d bytes [%s] ", n, buf);
	printf("(src ip:port %s:%d ",
		inet_ntoa(from.sin_addr), ntohs(from.sin_port));
	printf(" dst ip:port %s:%d)\n",
		inet_ntoa(to.sin_addr), ntohs(to.sin_port));

	printf("server: replying from address packet was received on to source address\n");

	if ((n = sendfromto(server_socket, buf, n, 0,
		(struct sockaddr *)&to, tl,
		(struct sockaddr *)&from, fl)) < 0) {
		perror("server: sendfromto");
	}

	waitpid(pid, NULL, 0);
	return 0;

client:
	close(server_socket);
	client_socket = socket(PF_INET, SOCK_DGRAM, 0);
	if (udpfromto_init(client_socket) != 0) {
		perror("udpfromto_init");
		fr_exit_now(0);
	}
	/* bind client on different port */
	in.sin_port = htons(port+1);
	if (bind(client_socket, (struct sockaddr *)&in, sizeof(in)) < 0) {
		perror("client: bind");
		fr_exit_now(0);
	}

	in.sin_port = htons(port);
	in.sin_addr.s_addr = inet_addr(destip);

	printf("client: sending packet to %s:%d\n", destip, port);
	if (sendto(client_socket, TESTSTRING, TESTLEN, 0,
			(struct sockaddr *)&in, sizeof(in)) < 0) {
		perror("client: sendto");
		fr_exit_now(0);
	}

	printf("client: waiting for reply from server on INADDR_ANY:%d\n", port+1);

	if ((n = recvfromto(client_socket, buf, sizeof(buf), 0,
	    (struct sockaddr *)&from, &fl,
	    (struct sockaddr *)&to, &tl)) < 0) {
		perror("client: recvfromto");
		fr_exit_now(0);
	}

	printf("client: received a packet of %d bytes [%s] ", n, buf);
	printf("(src ip:port %s:%d",
		inet_ntoa(from.sin_addr), ntohs(from.sin_port));
	printf(" dst ip:port %s:%d)\n",
		inet_ntoa(to.sin_addr), ntohs(to.sin_port));

	fr_exit_now(0);
}

#endif /* TESTING */
#endif /* WITH_UDPFROMTO */
