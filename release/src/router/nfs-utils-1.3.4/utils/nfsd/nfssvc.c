/*
 * utils/nfsd/nfssvc.c
 *
 * Run an NFS daemon.
 *
 * Copyright (C) 1995, 1996 Olaf Kirch <okir@monad.swb.de>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "nfslib.h"
#include "xlog.h"
#include "nfssvc.h"

#ifndef NFSD_FS_DIR
#define NFSD_FS_DIR	  "/proc/fs/nfsd"
#endif

#define NFSD_PORTS_FILE   NFSD_FS_DIR "/portlist"
#define NFSD_VERS_FILE    NFSD_FS_DIR "/versions"
#define NFSD_THREAD_FILE  NFSD_FS_DIR "/threads"

/*
 * declaring a common static scratch buffer here keeps us from having to
 * continually thrash the stack. The value of 128 bytes here is really just a
 * SWAG and can be increased if necessary. It ought to be enough for the
 * routines below however.
 */
char buf[128];

/*
 * Using the "new" interfaces for nfsd requires that /proc/fs/nfsd is
 * actually mounted. Make an attempt to mount it here if it doesn't appear
 * to be. If the mount attempt fails, no big deal -- fall back to using nfsctl
 * instead.
 */
void
nfssvc_mount_nfsdfs(char *progname)
{
	int err;
	struct stat statbuf;

	err = stat(NFSD_THREAD_FILE, &statbuf);
	if (err == 0)
		return;

	if (errno != ENOENT) {
		xlog(L_ERROR, "Unable to stat %s: errno %d (%m)",
				NFSD_THREAD_FILE, errno);
		return;
	}

	/*
	 * this call can return an error if modprobe is set up to automatically
	 * mount nfsdfs when nfsd.ko is plugged in. So, ignore the return
	 * code from it and just check for the "threads" file afterward.
	 */
	system("/bin/mount -t nfsd nfsd " NFSD_FS_DIR " >/dev/null 2>&1");

	err = stat(NFSD_THREAD_FILE, &statbuf);
	if (err == 0)
		return;

	xlog(L_WARNING, "Unable to access " NFSD_FS_DIR " errno %d (%m)." 
		"\nPlease try, as root, 'mount -t nfsd nfsd " NFSD_FS_DIR 
		"' and then restart %s to correct the problem", errno, progname);

	return;
}

/*
 * Are there already sockets configured? If not, then it is safe to try to
 * open some and pass them through.
 *
 * Note: If the user explicitly asked for 'udp', then we should probably check
 * if that is open, and should open it if not. However we don't yet. All
 * sockets have to be opened when the first daemon is started.
 */
int
nfssvc_inuse(void)
{
	int fd, n;

	fd = open(NFSD_PORTS_FILE, O_RDONLY);

	/* problem opening file, assume that nothing is configured */
	if (fd < 0)
		return 0;

	n = read(fd, buf, sizeof(buf));
	close(fd);

	xlog(D_GENERAL, "knfsd is currently %s", (n > 0) ? "up" : "down");

	return (n > 0);
}

static int
nfssvc_setfds(const struct addrinfo *hints, const char *node, const char *port)
{
	int fd, on = 1, fac = L_ERROR;
	int sockfd = -1, rc = 0;
	struct addrinfo *addrhead = NULL, *addr;
	char *proto, *family;

	/*
	 * if file can't be opened, then assume that it's not available and
	 * that the caller should just fall back to the old nfsctl interface
 	 */
	fd = open(NFSD_PORTS_FILE, O_WRONLY);
	if (fd < 0)
		return 0;

	rc = getaddrinfo(node, port, hints, &addrhead);
	if (rc == EAI_NONAME && !strcmp(port, "nfs")) {
		snprintf(buf, sizeof(buf), "%d", NFS_PORT);
		rc = getaddrinfo(node, buf, hints, &addrhead);
	}

	if (rc != 0) {
		xlog(L_ERROR, "unable to resolve %s:%s: %s",
			node ? node : "ANYADDR", port,
			rc == EAI_SYSTEM ? strerror(errno) :
				gai_strerror(rc));
		goto error;
	}

	addr = addrhead;
	while(addr) {
		/* skip non-TCP / non-UDP sockets */
		switch(addr->ai_protocol) {
		case IPPROTO_UDP:
			proto = "UDP";
			break;
		case IPPROTO_TCP:
			proto = "TCP";
			break;
		default:
			addr = addr->ai_next;
			continue;
		}

		switch(addr->ai_addr->sa_family) {
		case AF_INET:
			family = "AF_INET";
			break;
#ifdef IPV6_SUPPORTED
		case AF_INET6:
			family = "AF_INET6";
			break;
#endif /* IPV6_SUPPORTED */
		default:
			addr = addr->ai_next;
			continue;
		}

		/* open socket and prepare to hand it off to kernel */
		sockfd = socket(addr->ai_family, addr->ai_socktype,
				addr->ai_protocol);
		if (sockfd < 0) {
			if (errno != EAFNOSUPPORT) {
				xlog(L_ERROR, "unable to create %s %s socket: "
				     "errno %d (%m)", family, proto, errno);
				rc = errno;
				goto error;
			}
			addr = addr->ai_next;
			continue;
		}

		xlog(D_GENERAL, "Created %s %s socket.", family, proto);

#ifdef IPV6_SUPPORTED
		if (addr->ai_family == AF_INET6 &&
		    setsockopt(sockfd, IPPROTO_IPV6, IPV6_V6ONLY, &on, sizeof(on))) {
			xlog(L_ERROR, "unable to set IPV6_V6ONLY: "
				"errno %d (%m)\n", errno);
			rc = errno;
			goto error;
		}
#endif /* IPV6_SUPPORTED */
		if (addr->ai_protocol == IPPROTO_TCP &&
		    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) {
			xlog(L_ERROR, "unable to set SO_REUSEADDR on %s "
				"socket: errno %d (%m)", family, errno);
			rc = errno;
			goto error;
		}
		if (bind(sockfd, addr->ai_addr, addr->ai_addrlen)) {
			xlog(L_ERROR, "unable to bind %s %s socket: "
				"errno %d (%m)", family, proto, errno);
			rc = errno;
			goto error;
		}
		if (addr->ai_protocol == IPPROTO_TCP && listen(sockfd, 64)) {
			xlog(L_ERROR, "unable to create listening socket: "
				"errno %d (%m)", errno);
			rc = errno;
			goto error;
		}

		if (fd < 0)
			fd = open(NFSD_PORTS_FILE, O_WRONLY);

		if (fd < 0) {
			xlog(L_ERROR, "couldn't open ports file: errno "
				      "%d (%m)", errno);
			goto error;
		}

		snprintf(buf, sizeof(buf), "%d\n", sockfd); 
		if (write(fd, buf, strlen(buf)) != (ssize_t)strlen(buf)) {
			/*
			 * this error may be common on older kernels that don't
			 * support IPv6, so turn into a debug message.
			 */
			if (errno == EAFNOSUPPORT)
				fac = D_ALL;
			xlog(fac, "writing fd to kernel failed: errno %d (%m)",
				  errno);
			rc = errno;
			goto error;
		}
		close(fd);
		close(sockfd);
		sockfd = fd = -1;
		addr = addr->ai_next;
	}
error:
	if (fd >= 0)
		close(fd);
	if (sockfd >= 0)
		close(sockfd);
	if (addrhead)
		freeaddrinfo(addrhead);
	return rc;
}

int
nfssvc_set_sockets(const unsigned int protobits,
		   const char *host, const char *port)
{
	struct addrinfo hints = { .ai_flags = AI_PASSIVE };

#ifdef IPV6_SUPPORTED
	hints.ai_family = AF_UNSPEC;
#else  /* IPV6_SUPPORTED */
	hints.ai_family = AF_INET;
#endif /* IPV6_SUPPORTED */

	if (!NFSCTL_ANYPROTO(protobits))
		return EPROTOTYPE;
	else if (!NFSCTL_UDPISSET(protobits))
		hints.ai_protocol = IPPROTO_TCP;
	else if (!NFSCTL_TCPISSET(protobits))
		hints.ai_protocol = IPPROTO_UDP;

	return nfssvc_setfds(&hints, host, port);
}

int
nfssvc_set_rdmaport(const char *port)
{
	struct servent *sv = getservbyname(port, "tcp");
	int nport;
	char buf[20];
	int ret;
	int fd;

	if (sv)
		nport = ntohs(sv->s_port);
	else {
		char *ep;
		nport = strtol(port, &ep, 10);
		if (!*port || *ep) {
			xlog(L_ERROR, "unable to interpret port name %s",
			     port);
			return 1;
		}
	}

	fd = open(NFSD_PORTS_FILE, O_WRONLY);
	if (fd < 0)
		return 1;
	snprintf(buf, sizeof(buf), "rdma %d", nport);
	ret = 0;
	if (write(fd, buf, strlen(buf)) != (ssize_t)strlen(buf)) {
		ret= errno;
		xlog(L_ERROR, "Unable to request RDMA services: %m");
	}
	close(fd);
	return ret;
}

void
nfssvc_set_time(const char *type, const int seconds)
{
	char pathbuf[40];
	char nbuf[10];
	int fd;

	snprintf(pathbuf, sizeof(pathbuf), NFSD_FS_DIR "/nfsv4%stime", type);
	snprintf(nbuf, sizeof(nbuf), "%d", seconds);
	fd = open(pathbuf, O_WRONLY);
	if (fd >= 0) {
		if (write(fd, nbuf, strlen(nbuf)) != (ssize_t)strlen(nbuf))
			xlog(L_ERROR, "Unable to set nfsv4%stime: %m", type);
		close(fd);
	}
	if (strcmp(type, "grace") == 0) {
		/* set same value for lockd */
		fd = open("/proc/sys/fs/nfs/nlm_grace_period", O_WRONLY);
		if (fd >= 0) {
			write(fd, nbuf, strlen(nbuf));
			close(fd);
		}
	}
}

void
nfssvc_setvers(unsigned int ctlbits, unsigned int minorvers, unsigned int minorversset)
{
	int fd, n, off;
	char *ptr;

	ptr = buf;
	off = 0;
	fd = open(NFSD_VERS_FILE, O_WRONLY);
	if (fd < 0)
		return;

	for (n = NFS4_MINMINOR; n <= NFS4_MAXMINOR; n++) {
		if (NFSCTL_VERISSET(minorversset, n)) {
			if (NFSCTL_VERISSET(minorvers, n))
				off += snprintf(ptr+off, sizeof(buf) - off, "+4.%d ", n);
			else
				off += snprintf(ptr+off, sizeof(buf) - off, "-4.%d ", n);
		}
	}
	for (n = NFSD_MINVERS; n <= NFSD_MAXVERS; n++) {
		if (NFSCTL_VERISSET(ctlbits, n))
		    off += snprintf(ptr+off, sizeof(buf) - off, "+%d ", n);
		else
		    off += snprintf(ptr+off, sizeof(buf) - off, "-%d ", n);
	}
	xlog(D_GENERAL, "Writing version string to kernel: %s", buf);
	snprintf(ptr+off, sizeof(buf) - off, "\n");
	if (write(fd, buf, strlen(buf)) != (ssize_t)strlen(buf))
		xlog(L_ERROR, "Setting version failed: errno %d (%m)", errno);

	close(fd);

	return;
}

int
nfssvc_threads(unsigned short port, const int nrservs)
{
	struct nfsctl_arg	arg;
	struct servent *ent;
	ssize_t n;
	int fd;

	fd = open(NFSD_THREAD_FILE, O_WRONLY);
	if (fd < 0)
		fd = open("/proc/fs/nfs/threads", O_WRONLY);
	if (fd >= 0) {
		/* 2.5+ kernel with nfsd filesystem mounted.
		 * Just write the number of threads.
		 */
		snprintf(buf, sizeof(buf), "%d\n", nrservs);
		n = write(fd, buf, strlen(buf));
		close(fd);
		if (n != (ssize_t)strlen(buf))
			return -1;
		else
			return 0;
	}

	if (!port) {
		ent = getservbyname("nfs", "udp");
		if (ent != NULL)
			port = ntohs(ent->s_port);
		else
			port = NFS_PORT;
	}

	arg.ca_version = NFSCTL_VERSION;
	arg.ca_svc.svc_nthreads = nrservs;
	arg.ca_svc.svc_port = port;
	return nfsctl(NFSCTL_SVC, &arg, NULL);
}
