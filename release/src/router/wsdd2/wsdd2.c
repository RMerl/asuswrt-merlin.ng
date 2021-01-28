/*
   WSDD - Web Service Dynamic Discovery protocol server

   Main file for general network handling.
  
  	Copyright (c) 2016 NETGEAR
  	Copyright (c) 2016 Hiro Sugawara
  
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "wsdd.h"

int debug_L, debug_W, debug_N;
int ifindex = 0;
char *ifname = NULL;

static int netlink_recv(struct endpoint *ep);

static struct service services[] = {
	{
		.name	= "wsdd-mcast-v4",
		.family	= AF_INET,
		.type	= SOCK_DGRAM,
		.port_name	= "wsdd",
		.port_num	= 3702,
		.mcast_addr	= "239.255.255.250",
		.init	= wsd_init,
		.recv	= wsd_recv,
		.exit	= wsd_exit,
	},
	{
		.name	= "wsdd-mcast-v6",
		.family	= AF_INET6,
		.type	= SOCK_DGRAM,
		.port_name	= "wsdd",
		.port_num	= 3702,
		.mcast_addr	= "ff02::c",
		.init	= wsd_init,
		.recv	= wsd_recv,
		.exit	= wsd_exit,
	},
	{
		.name	= "wsdd-http-v4",
		.family	= AF_INET,
		.type	= SOCK_STREAM,
		.port_name	= "wsdd",
		.port_num	= 3702,
		.recv	= wsd_recv,
	},
	{
		.name	= "wsdd-http-v6",
		.family	= AF_INET6,
		.type	= SOCK_STREAM,
		.port_name	= "wsdd",
		.port_num	= 3702,
		.recv	= wsd_recv,
	},
	{
		.name	= "llmnr-mcast-v4",
		.family	= AF_INET,
		.type	= SOCK_DGRAM,
		.port_name	= "llmnr",
		.port_num	= 5355,
		.mcast_addr	= "224.0.0.252",
		.init	= llmnr_init,
		.recv	= llmnr_recv,
		.exit	= llmnr_exit,
	},
	{
		.name	= "llmnr-mcast-v6",
		.family	= AF_INET6,
		.type	= SOCK_DGRAM,
		.port_name	= "llmnr",
		.port_num	= 5355,
		.mcast_addr	= "ff02::1:3",
		.init	= llmnr_init,
		.recv	= llmnr_recv,
		.exit	= llmnr_exit,
	},
	{
		.name	= "llmnr-tcp-v4",
		.family	= AF_INET,
		.type	= SOCK_STREAM,
		.port_name	= "llmnr",
		.port_num	= 5355,
		.init	= llmnr_init,
		.recv	= llmnr_recv,
		.exit	= llmnr_exit,
	},
	{
		.name	= "llmnr-tcp-v6",
		.family	= AF_INET6,
		.type	= SOCK_STREAM,
		.port_name	= "llmnr",
		.port_num	= 5355,
		.init	= llmnr_init,
		.recv	= llmnr_recv,
		.exit	= llmnr_exit,
	},
	{
		.name	= "ifaddr-netlink-v4v6",
		.family	= AF_NETLINK,
		.type	= SOCK_RAW,
		.protocol	= NETLINK_ROUTE,
		.nl_groups	= RTMGRP_LINK |
					RTMGRP_IPV4_IFADDR |
					RTMGRP_IPV6_IFADDR,
		.recv	= netlink_recv,
	},
};

int connected_if(const _saddr_t *sa, _saddr_t *ci)
{
	struct ifaddrs *ifaddr, *ifa;
	int rv = -1;

	if (getifaddrs(&ifaddr))
		return -1;

	ci->ss.ss_family = sa->ss.ss_family;

	for (ifa = ifaddr; ifa; ifa = ifa->ifa_next) {
		const uint8_t *_if, *_nm, *_sa;
		uint8_t *_ca;
		size_t alen;

		if (!ifa->ifa_addr || sa->ss.ss_family != ifa->ifa_addr->sa_family)
			continue;

		if (debug_W >= 5) {
			char name[_ADDRSTRLEN];

			if (inet_ntop(ifa->ifa_addr->sa_family,
					_SIN_ADDR((_saddr_t *)ifa->ifa_addr),
					name, sizeof name))
				printf("%s: %s: if=%s ",
					__func__, ifa->ifa_name,name);
			if (inet_ntop(sa->ss.ss_family,
					_SIN_ADDR(sa), name, sizeof name))
				printf("sc=%s ", name);
			if (inet_ntop(ifa->ifa_netmask->sa_family,
					_SIN_ADDR((_saddr_t *)ifa->ifa_netmask),
					name, sizeof name))
				printf("nm=%s\n", name);
		}

		switch (sa->ss.ss_family) {
		case AF_INET:
			_if = (uint8_t *)
				&((_saddr_t *)ifa->ifa_addr)->in.sin_addr;
			_nm = (uint8_t *)
				&((_saddr_t *)ifa->ifa_netmask)->in.sin_addr;
			_sa = (uint8_t *)&sa->in.sin_addr;
			_ca = (uint8_t *)&ci->in.sin_addr;
			alen = sizeof sa->in.sin_addr;
			break;
		case AF_INET6:
			_if = (uint8_t *)
				&((_saddr_t *)ifa->ifa_addr)->in6.sin6_addr;
			_nm = (uint8_t *)
				&((_saddr_t *)ifa->ifa_netmask)->in6.sin6_addr;
			_sa = (uint8_t *)&sa->in6.sin6_addr;
			_ca = (uint8_t *)&ci->in6.sin6_addr;
			alen = sizeof sa->in6.sin6_addr;
			break;
		default:
			continue;
		}

		rv = 0;
		int i;
		for (i = 0; i < alen; i++)
			if ((_if[i] & _nm[i]) != (_sa[i] & _nm[i])) {
				rv = -1;
				break;
			}
		if (!rv) {
			memcpy(_ca, _if, alen);
			break;
		}
	}

	if (debug_W >= 4) {
		char name[_ADDRSTRLEN];

		if (inet_ntop(ci->ss.ss_family, _SIN_ADDR(ci),
				name, sizeof name))
			printf("%s: ci=%s\n\n", __func__, name);
	}

	freeifaddrs(ifaddr);
	errno = EADDRNOTAVAIL;
	return rv;
}

char *ip2uri(const char *ip)
{
	if (*ip == '[' || !strchr(ip, ':'))
		return strdup(ip);

	char *uri = NULL;

#if 0	/* WINDOWS 7 does not honor [xx::xx] notation. */
	asprintf(&uri, "[%s]", ip);
#else
	char name[HOST_NAME_MAX + 1];

	if (!gethostname(name, sizeof name - 1))
		uri = strdup(name);
#endif
	return uri;
}

static struct endpoint *endpoints;

static const struct sock_params {
	int family;
	const char *name;
	int ipproto_ip;
	int ip_multicast_loop;
	int ip_add_membership, ip_drop_membership;
	size_t llen, mlen, mreqlen;
} sock_params[] = {
	[AF_INET] = {
		.family	= AF_INET,
		.name	= "IPv4",
		.ipproto_ip	= IPPROTO_IP,
		.ip_multicast_loop	= IP_MULTICAST_LOOP,
		.ip_add_membership	= IP_ADD_MEMBERSHIP,
		.ip_drop_membership	= IP_DROP_MEMBERSHIP,
		.llen		= sizeof(struct sockaddr_in),
		.mlen		= sizeof(struct sockaddr_in),
		.mreqlen	= sizeof endpoints[0].mreq.ip_mreq,
	},
	[AF_INET6] = {
		.family	= AF_INET6,
		.name	= "IPv6",
		.ipproto_ip	= IPPROTO_IPV6,
		.ip_multicast_loop	= IPV6_MULTICAST_LOOP,
		.ip_add_membership	= IPV6_ADD_MEMBERSHIP,
		.ip_drop_membership	= IPV6_DROP_MEMBERSHIP,
		.llen		= sizeof(struct sockaddr_in6),
		.mlen		= sizeof(struct sockaddr_in6),
		.mreqlen	= sizeof endpoints[0].mreq.ipv6_mreq,
	},
	[AF_NETLINK] = {
		.family	= AF_NETLINK,
		.name	= "NETLINK",
		.llen		= sizeof(struct sockaddr_nl),
	},
};

static int open_ep(struct endpoint **epp, struct service *sv,
			const struct ifaddrs *ifa)
{
#define __FUNCTION__	"open_ep"
	struct endpoint *ep = calloc(sizeof *ep, 1);

	if (!(*epp = ep)) {
		errno = ENOMEM;
		err(EXIT_FAILURE, __FUNCTION__ ": malloc");
	}

	strncpy(ep->ifname, ifa->ifa_name, sizeof(ep->ifname)-1);
	ep->service	= sv;
	ep->family	= sv->family;
	ep->type	= sv->type;
	ep->protocol	= sv->protocol;

	if (sv->family >= ARRAY_SIZE(sock_params) ||
		!sock_params[ep->family].name) {
		ep->errstr = __FUNCTION__ ": Unsupported address family";
		ep->_errno = EINVAL;
		return -1;
	}

	if (sv->family == AF_INET || sv->family == AF_INET6) {
		const char *servicename[] = {
			[SOCK_STREAM]	= "tcp",
			[SOCK_DGRAM]	= "udp",
		};
		struct servent *se = getservbyname(sv->port_name,
						servicename[sv->type]);
		ep->port = se ? se->s_port : 0;
		if (!ep->port)
			ep->port = sv->port_num;
		if (!ep->port) {
			ep->errstr = __FUNCTION__ ": No port number";
			ep->_errno = EADDRNOTAVAIL;
			return -1;
		}
	}

	const struct sock_params *sp = &sock_params[ep->family];

	ep->mcast.ss.ss_family	= ep->family;
	ep->mlen	= sp->llen;

	ep->local.ss.ss_family = ep->family;
	ep->llen	= sp->llen;

	ep->mreqlen	= sp->mreqlen;

	switch (ep->family) {
	case AF_INET:
		if (sv->mcast_addr) {
			ep->mcast.in.sin_port	= htons(ep->port);
			if (inet_pton(ep->family, sv->mcast_addr,
				&ep->mcast.in.sin_addr.s_addr) != 1) {
				ep->errstr = __FUNCTION__ ": Bad mcast IP addr";
				ep->_errno = errno;
				return -1;
			}
			ep->mreq.ip_mreq.imr_multiaddr = ep->mcast.in.sin_addr;
#if 0
			ep->mreq.ip_mreq.imr_address	=
				((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
			ep->mreq.ip_mreq.imr_ifindex =
				if_nametoindex(ep->ifname);
#else
			ep->mreq.ip_mreq.imr_interface =
				((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
#endif
		}

		//ep->local.saddr_in = *(struct sockaddr_in *)ifa->ifa_addr;
		ep->local.in.sin_addr.s_addr = htonl(INADDR_ANY);
		ep->local.in.sin_port = htons(ep->port);
		break;
	case AF_INET6:
		if (sv->mcast_addr) {
			ep->mcast.in6.sin6_port = htons(ep->port);
			if (inet_pton(ep->family, sv->mcast_addr,
				ep->mcast.in6.sin6_addr.s6_addr) != 1) {
				ep->errstr =
					__FUNCTION__ ": Bad mcast IPv6 addr";
				ep->_errno = errno;
				return -1;
			}
			ep->mreq.ipv6_mreq.ipv6mr_multiaddr =
						ep->mcast.in6.sin6_addr;
			ep->mreq.ipv6_mreq.ipv6mr_interface =
						if_nametoindex(ep->ifname);
		}

		//ep->local.in6 = *(struct sockaddr_in6 *)ifa->ifa_addr;
		ep->local.in6.sin6_addr = in6addr_any;
		ep->local.in6.sin6_port = htons(ep->port);
		break;
	case AF_NETLINK:
		ep->local.nl.nl_groups = ep->service->nl_groups;
		break;
	}

	ep->sock = socket(ep->family, ep->type, ep->protocol);
	if (ep->sock < 0) {
		ep->errstr = __FUNCTION__ ": Can't open socket";
		ep->_errno = errno;
		return -1;
	}

	const unsigned int enable = 1;
	setsockopt(ep->sock, SOL_SOCKET, SO_REUSEADDR,
				&enable, sizeof enable);
#ifdef SO_REUSEPORT
	setsockopt(ep->sock, SOL_SOCKET, SO_REUSEPORT,
				&enable, sizeof enable);
#endif
#ifdef IPV6_V6ONLY
	if ((ep->family == AF_INET6) &&
		setsockopt(ep->sock, sp->ipproto_ip, IPV6_V6ONLY,
				&enable, sizeof enable)) {
		ep->errstr = __FUNCTION__ ": IPV6_V6ONLY";
		ep->_errno = errno;
		close(ep->sock);
		return -1;
	}
#endif

#ifdef SO_BINDTODEVICE
	if (!sv->mcast_addr &&
			(ep->family == AF_INET || ep->family == AF_INET6)) {
		struct ifreq ifr;
		strncpy(ifr.ifr_name, ep->ifname, IFNAMSIZ-1);
		if (setsockopt(ep->sock, SOL_SOCKET, SO_BINDTODEVICE,
				&ifr, sizeof(ifr))) {
			ep->errstr = __FUNCTION__ ": SO_BINDTODEVICE";
			ep->_errno = errno;
			close(ep->sock);
			return -1;
		}
	}
#endif

	if (bind(ep->sock, (struct sockaddr *)&ep->local, ep->llen)) {
		ep->errstr = __FUNCTION__ ": bind";
		ep->_errno = errno;
		close(ep->sock);
		ep->sock = -1;
		DEBUG(0, W, "%s: %s: %s",
			ep->service->name, ep->errstr, strerror(ep->_errno));
		return (ep->_errno == EADDRINUSE) ? 0 : -1;
	}

	if (sv->mcast_addr) {
		const unsigned int disable = 0, enable = 1;

		if ((ep->family == AF_INET) &&
			setsockopt(ep->sock, sp->ipproto_ip, IP_PKTINFO,
					&enable, sizeof enable)) {
			ep->errstr = __FUNCTION__ ": IP_PKTINFO";
			ep->_errno = errno;
			close(ep->sock);
			return -1;
		}
		/* Disable loopback. */
		if (setsockopt(ep->sock, sp->ipproto_ip, sp->ip_multicast_loop,
				&disable, sizeof disable)) {
			ep->errstr = __FUNCTION__ ": IP_MULTICAST_LOOP";
			ep->_errno = errno;
			close(ep->sock);
			return -1;
		}

		/* Set inbound multicast. */
		if (setsockopt(ep->sock, sp->ipproto_ip, sp->ip_add_membership,
				&ep->mreq, ep->mreqlen)) {
			ep->errstr = __FUNCTION__ ": IP_ADD_MEMBERSHIP";
			ep->_errno = errno;
			close(ep->sock);
			return -1;
		}
	}

	if (ep->type == SOCK_STREAM && listen(ep->sock, 5)) {
		ep->errstr = __FUNCTION__ ": listen";
		ep->_errno = errno;
		close(ep->sock);
		return -1;
	}

	if (ep->service->init && ep->service->init(ep)) {
		close(ep->sock);
		return -1;
	}
	return 0;
#undef __FUNCTION__
}

static void close_ep(struct endpoint *ep)
{
	if (ep->service->exit)
		ep->service->exit(ep);
	if (ep->service->mcast_addr) {
		setsockopt(ep->sock,
				sock_params[ep->family].ipproto_ip,
				sock_params[ep->family].ip_drop_membership,
				&ep->mreq, ep->mreqlen);
	}
	close(ep->sock);
}

#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>

static jmp_buf sigenv;
volatile sig_atomic_t restart;

void restart_service(void)
{
	DEBUG(1, W, "restarting service.");
	restart = 1;
	longjmp(sigenv, 1);
}

static bool is_new_addr(struct nlmsghdr *nh)
{
	struct ifaddrmsg *ifam = (struct ifaddrmsg *)NLMSG_DATA(nh);
	struct rtattr *rta = IFA_RTA(ifam);
	size_t rtasize = IFA_PAYLOAD(nh);

	if (nh->nlmsg_type != RTM_NEWADDR)
		return false;

	if (ifindex && ifam->ifa_index != ifindex) {
		char buf[IFNAMSIZ];
		if (!if_indextoname(ifindex, buf) || strcmp(buf, ifname) != 0)
			return false;
		ifindex = ifam->ifa_index;
	}

	while (RTA_OK(rta, rtasize)) {
		struct ifa_cacheinfo *cache_info;
		if (rta->rta_type == IFA_CACHEINFO) {
			cache_info = (struct ifa_cacheinfo *)(RTA_DATA(rta));
			if (cache_info->cstamp != cache_info->tstamp)
				return false;
		}
		rta = RTA_NEXT(rta, rtasize);
	}

	return true;
}

static int netlink_recv(struct endpoint *ep)
{
#define __FUNCTION__	"netlink_recv"
	char buf[4096];
	struct iovec iov = { buf, sizeof buf };
	struct sockaddr_nl sa;
	struct nlmsghdr *nh;
	struct msghdr msg = { &sa, sizeof sa, &iov, 1, NULL, 0, 0 };
	ssize_t msglen = recvmsg(ep->sock, &msg, 0);

	DEBUG(2, W, "%s: %zd bytes", __func__, msglen);
	if (msglen <= 0) {
		ep->_errno = errno;
		ep->errstr = __FUNCTION__ ": netlink_recv: recv";
		return -1;
	}


	for (nh = (struct nlmsghdr *)buf;
			NLMSG_OK(nh, msglen) && nh->nlmsg_type != NLMSG_DONE;
			nh = NLMSG_NEXT(nh, msglen)) {
		if (is_new_addr(nh) || nh->nlmsg_type == RTM_DELADDR) {
			DEBUG(1, W,
		"I/F address addition/change/deletion detected.");
			restart_service();
			break;
		}
	}

	return 0;
#undef __FUNCTION__
}

static void sighandler(int sig)
{
	DEBUG(0, W, "%s received.", strsignal(sig));
	switch (sig) {
	case SIGHUP:
		restart = 1;
		break;
	default:
		restart = 2;
	}
}

static void help(const char *prog, int ec, const char *fmt, ...)
{
	if (fmt) {
		va_list ap;

		va_start(ap, fmt);
		vprintf(fmt, ap);
		va_end(ap);
	}
	printf( "WSDD and LLMNR daemon\n"
		"Usage: %s [opts]\n"
		"       -4  IPv4 only\n"
		"       -6  IPv6 only\n"
		"       -L  LLMNR debug mode (incremental level)\n"
		"       -W  WSDD debug mode (incremental level)\n"
		"       -d  go daemon\n"
		"       -h  This message\n"
		"       -l  LLMNR only\n"
		"       -t  TCP only\n"
		"       -u  UDP only\n"
		"       -w  WSDD only\n"
		"       -i \"interface\"  Listening interface (optional)\n"
		"       -N  set NetbiosName manually\n"
		"       -G  set Workgroup manually\n"
		"       -b \"key1:val1,key2:val2,...\"  Boot parameters\n",
			prog);
	printBootInfoKeys(stdout, 11);
	exit(ec);
}

#define	_4	1
#define	_6	2
#define _TCP	1
#define _UDP	2
#define	_LLMNR	1
#define	_WSDD	2

int main(int argc, char **argv)
{
	bool daemon = false;
	int opt;
	const char *prog = basename(*argv);
	unsigned int ipv46 = 0, tcpudp = 0, llmnrwsdd = 0;

	while ((opt = getopt(argc, argv, "?46LWb:dhltuwi:N:G:")) != -1) {
		switch (opt) {
		case 'L':
			debug_L++;
			break;
		case 'W':
			debug_W++;
			break;
		case 'b':
			while (optarg)
				if (set_getresp(optarg, (const char **)&optarg))
					help(prog, EXIT_FAILURE,
						"bad key:val '%s'\n", optarg);
			break;
		case 'd':
			daemon = true;
			break;
		case 'h':
			help(prog, EXIT_SUCCESS, NULL);
			break;
		case '4':
			ipv46	|= _4;
			break;
		case '6':
			ipv46	|= _6;
			break;
		case 'l':
			llmnrwsdd |= _LLMNR;
			break;
		case 'w':
			llmnrwsdd |= _WSDD;
			break;
		case 't':
			tcpudp	|= _TCP;
			break;
		case 'u':
			tcpudp	|= _UDP;
			break;
		case 'i':
			ifname = optarg;
			ifindex = if_nametoindex(optarg);
			if (ifindex == 0)
				help(prog, EXIT_FAILURE,
					"bad interface '%s'\n", optarg);
			break;
		case 'N':
			if (optarg != NULL && strlen(optarg) > 1) {
				netbiosname = strdup(optarg);
			}
			break;
		case 'G':
			if (optarg != NULL && strlen(optarg) > 1) {
				workgroup = strdup(optarg);
			}
			break;
		case '?':
			if (optopt == 'b' || optopt == 'i' || optopt == 'N' || optopt == 'G')
				fprintf (stderr, "Option -%c requires an argument.\n", optopt);
		default:
			help(prog, EXIT_FAILURE, "bad option '%c'\n", opt);
		}
	}

	if (!ipv46)
		ipv46	= _4 | _6;
	if (!llmnrwsdd)
		llmnrwsdd = _LLMNR | _WSDD;
	if (!tcpudp)
		tcpudp	= _TCP | _UDP;

	if (daemon) {
		pid_t pid = fork();

		if (pid < 0)
			err(EXIT_FAILURE, "fork");
		if (pid)
			exit(EXIT_SUCCESS);
	}

	openlog(prog, LOG_PID, LOG_USER);
	syslog(LOG_USER | LOG_INFO, "starting.");

again:
	{}	/* Necessary to satisfy C syntax for statement labeling. */
	struct sigaction sigact, oldact;

	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;
	sigact.sa_handler = sighandler;

	if (sigaction(SIGINT, &sigact, &oldact) ||
		sigaction(SIGHUP, &sigact, &oldact) ||
		sigaction(SIGTERM, &sigact, &oldact)) {
		err(EXIT_FAILURE, "cannot install signal handler.");
	}

	struct ifaddrs *ifaddrs;
	fd_set fds;
	int svn, rv = 0, nfds = -1;
	struct endpoint *ep, *badep = NULL;

	FD_ZERO(&fds);

	if (getifaddrs(&ifaddrs))
		err(EXIT_FAILURE, "ifaddrs");

	for (svn = 0; svn < ARRAY_SIZE(services); svn++) {
		struct service *sv = &services[svn];

		if (!(ipv46 & _4) && sv->family == AF_INET)
			continue;
		if (!(ipv46 & _6) && sv->family == AF_INET6)
			continue;
		if (!(tcpudp & _TCP) && sv->type == SOCK_STREAM)
			continue;
		if (!(tcpudp & _UDP) && sv->type == SOCK_DGRAM)
			continue;
		if (!(llmnrwsdd & _LLMNR) && strstr(sv->name, "llmnr"))
			continue;
		if (!(llmnrwsdd & _WSDD) && strstr(sv->name, "wsdd"))
			continue;

		struct ifaddrs *ifa;

		if (sv->family == AF_INET || sv->family == AF_INET6) {
			for (ifa = ifaddrs; ifa; ifa = ifa->ifa_next) {
				if (!ifa->ifa_addr ||
					(ifa->ifa_addr->sa_family != sv->family) ||
					(ifa->ifa_flags & IFF_LOOPBACK) ||
					(ifa->ifa_flags & IFF_SLAVE) ||
					(ifname && strcmp(ifa->ifa_name, ifname)) ||
					(!strcmp(ifa->ifa_name, "LeafNets")) ||
					(!strncmp(ifa->ifa_name, "docker", 6)) ||
					(!strncmp(ifa->ifa_name, "veth", 4)) ||
					(!strncmp(ifa->ifa_name, "tun", 3)) ||
					(!strncmp(ifa->ifa_name, "zt", 2)) ||
					(sv->mcast_addr &&
					!(ifa->ifa_flags & IFF_MULTICAST)))
					continue;

				if (!ifname) {
					char path[sizeof("/sys/class/net//brport")+IFNAMSIZ];
					struct stat st;
					snprintf(path, sizeof(path), "/sys/class/net/%s/brport", ifa->ifa_name);
					if (stat(path, &st) == 0)
						continue;
				}

				char ifaddr[_ADDRSTRLEN];
				void *addr =
					_SIN_ADDR((_saddr_t *)ifa->ifa_addr);

				inet_ntop(ifa->ifa_addr->sa_family, addr,
						ifaddr, sizeof ifaddr);

				DEBUG(2, W, "%s %s %s@%s",
					sv->name,
					sv->mcast_addr ? sv->mcast_addr : "",
					ifa->ifa_name,
					ifaddr);

				if (open_ep(&ep, sv, ifa)) {
					syslog(LOG_USER | LOG_ERR, "error: %s: %s",
						ep->service->name, ep->errstr);
					free(ep);
					continue;
				} else if (ep->sock < 0)
					free(ep);
				else {
					ep->next = endpoints;
					endpoints = ep;
					FD_SET(ep->sock, &fds);
					if (nfds < ep->sock)
						nfds = ep->sock;
				}
			}
			if (badep)
				break;
		} else if (sv->family == AF_NETLINK) {
			const struct ifaddrs ifa = { .ifa_name = "netlink", };

			DEBUG(2, W, "%s @0x%x", sv->name, sv->nl_groups);
			if (open_ep(&ep, sv, &ifa)) {
				badep = ep;
				break;
			} else if (ep->sock < 0)
				free(ep);
			else {
				ep->next = endpoints;
				endpoints = ep;
				FD_SET(ep->sock, &fds);
				if (nfds < ep->sock)
					nfds = ep->sock;
			}
		}
	}

	freeifaddrs(ifaddrs);

	if (!badep) {
		int n = 0;

		if (setjmp(sigenv))
			goto end;

		do {
			fd_set rfds = fds;

			n = select(nfds + 1, &rfds, NULL, NULL, NULL);
			DEBUG(3, W, "select: n=%d", n);
			for (ep = endpoints; n > 0 && ep; ep = ep->next) {
				if (!FD_ISSET(ep->sock, &rfds))
					continue;
				DEBUG(3, W, "dispatch %s_recv",
					ep->service->name);
				n--;
				if (ep->service->recv) {
					int ret = ep->service->recv(ep);
					if (ret < 0) {
						DEBUG(1, W,
					"Detected %s socket error, restarting",
							ep->service->name);
						restart_service();
					}
				}
			}
		} while (n >= 0 && !restart);

		if (n < 0 && errno != EINTR) {
			syslog(LOG_USER | LOG_WARNING, "%s: select: %s",
				__func__, strerror(errno));
			rv = EXIT_FAILURE;
		}
	}
end:
	{}
	const char *badservice = NULL, *badbad = NULL;
	int baderrno = 0;

	if (badep) {
		badservice = badep->service->name ? badep->service->name : "";
		badbad = badep->errstr ? badep->errstr : "";
		baderrno = badep->_errno;
	}

	while (endpoints) {
		struct endpoint *tempep = endpoints->next;

		close_ep(endpoints);
		free(endpoints);
		endpoints = tempep;
	}

	if (badep) {
		syslog(LOG_USER | LOG_ERR, "%s: %s: terminating.",
			badservice, badbad);
		closelog();
		errno = baderrno;
		err(EXIT_FAILURE, "%s: %s", badservice, badbad);
	}

	if (restart == 1) {
		restart = 0;
		goto again;
	}

	syslog(LOG_USER | LOG_INFO, "terminating.");
	closelog();
	return rv;
}
