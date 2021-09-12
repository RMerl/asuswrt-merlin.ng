/* -*- mode: c; c-file-style: "openbsd" -*- */
/*
 * Copyright (c) 2008 Vincent Bernat <bernat@luffy.cx>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/* This file contains code for privilege separation. When an error arises in
 * monitor (which is running as root), it just stops instead of trying to
 * recover. This module also contains proxies to privileged operations. In this
 * case, error can be non fatal. */

#include "lldpd.h"
#include "trace.h"

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <limits.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <grp.h>
#include <sys/utsname.h>
#include <sys/ioctl.h>
#include <netinet/if_ether.h>

#ifdef HAVE_LINUX_CAPABILITIES
#include <sys/capability.h>
#include <sys/prctl.h>
#endif

#if defined HOST_OS_FREEBSD || defined HOST_OS_OSX || defined HOST_OS_DRAGONFLY
# include <net/if_dl.h>
#endif
#if defined HOST_OS_SOLARIS
# include <sys/sockio.h>
#endif

/* Use resolv.h */
#ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif
#ifdef HAVE_NETINET_IN_H
#  include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_NAMESER_H
#  include <arpa/nameser.h> /* DNS HEADER struct */
#endif
#ifdef HAVE_NETDB_H
#  include <netdb.h>
#endif
#ifdef HAVE_RESOLV_H
#  include <resolv.h>
#endif

/* Bionic has res_init() but it's not in any header */
#if defined HAVE_RES_INIT && defined __BIONIC__
int res_init (void);
#endif

#ifdef ENABLE_PRIVSEP
static int monitored = -1;		/* Child */
#endif

/* Proxies */
static void
priv_ping()
{
	int rc;
	enum priv_cmd cmd = PRIV_PING;
	must_write(PRIV_UNPRIVILEGED, &cmd, sizeof(enum priv_cmd));
	priv_wait();
	must_read(PRIV_UNPRIVILEGED, &rc, sizeof(int));
	log_debug("privsep", "monitor ready");
}

/* Proxy for ctl_cleanup */
void
priv_ctl_cleanup(const char *ctlname)
{
	int rc, len = strlen(ctlname);
	enum priv_cmd cmd = PRIV_DELETE_CTL_SOCKET;
	must_write(PRIV_UNPRIVILEGED, &cmd, sizeof(enum priv_cmd));
	must_write(PRIV_UNPRIVILEGED, &len, sizeof(int));
	must_write(PRIV_UNPRIVILEGED, ctlname, len);
	priv_wait();
	must_read(PRIV_UNPRIVILEGED, &rc, sizeof(int));
}

/* Proxy for gethostname */
char *
priv_gethostname()
{
	static char *buf = NULL;
	int len;
	enum priv_cmd cmd = PRIV_GET_HOSTNAME;
	must_write(PRIV_UNPRIVILEGED, &cmd, sizeof(enum priv_cmd));
	priv_wait();
	must_read(PRIV_UNPRIVILEGED, &len, sizeof(int));
	if (len < 0 || len > 255)
		fatalx("privsep", "too large value requested");
	if ((buf = (char*)realloc(buf, len+1)) == NULL)
		fatal("privsep", NULL);
	must_read(PRIV_UNPRIVILEGED, buf, len);
	buf[len] = '\0';
	return buf;
}


int
priv_iface_init(int index, char *iface)
{
	int rc;
	char dev[IFNAMSIZ] = {};
	enum priv_cmd cmd = PRIV_IFACE_INIT;
	must_write(PRIV_UNPRIVILEGED, &cmd, sizeof(enum priv_cmd));
	must_write(PRIV_UNPRIVILEGED, &index, sizeof(int));
	strlcpy(dev, iface, IFNAMSIZ);
	must_write(PRIV_UNPRIVILEGED, dev, IFNAMSIZ);
	priv_wait();
	must_read(PRIV_UNPRIVILEGED, &rc, sizeof(int));
	if (rc != 0) return -1;
	return receive_fd(PRIV_UNPRIVILEGED);
}

int
priv_iface_multicast(const char *name, const u_int8_t *mac, int add)
{
	int rc;
	enum priv_cmd cmd = PRIV_IFACE_MULTICAST;
	must_write(PRIV_UNPRIVILEGED, &cmd, sizeof(enum priv_cmd));
	must_write(PRIV_UNPRIVILEGED, name, IFNAMSIZ);
	must_write(PRIV_UNPRIVILEGED, mac, ETHER_ADDR_LEN);
	must_write(PRIV_UNPRIVILEGED, &add, sizeof(int));
	priv_wait();
	must_read(PRIV_UNPRIVILEGED, &rc, sizeof(int));
	return rc;
}

int
priv_iface_description(const char *name, const char *description)
{
	int rc, len = strlen(description);
	enum priv_cmd cmd = PRIV_IFACE_DESCRIPTION;
	must_write(PRIV_UNPRIVILEGED, &cmd, sizeof(enum priv_cmd));
	must_write(PRIV_UNPRIVILEGED, name, IFNAMSIZ);
	must_write(PRIV_UNPRIVILEGED, &len, sizeof(int));
	must_write(PRIV_UNPRIVILEGED, description, len);
	priv_wait();
	must_read(PRIV_UNPRIVILEGED, &rc, sizeof(int));
	return rc;
}

/* Proxy to set interface in promiscuous mode */
int
priv_iface_promisc(const char *ifname)
{
	int rc;
	enum priv_cmd cmd = PRIV_IFACE_PROMISC;
	must_write(PRIV_UNPRIVILEGED, &cmd, sizeof(enum priv_cmd));
	must_write(PRIV_UNPRIVILEGED, ifname, IFNAMSIZ);
	priv_wait();
	must_read(PRIV_UNPRIVILEGED, &rc, sizeof(int));
	return rc;
}

int
priv_snmp_socket(struct sockaddr_un *addr)
{
	int rc;
	enum priv_cmd cmd = PRIV_SNMP_SOCKET;
	must_write(PRIV_UNPRIVILEGED, &cmd, sizeof(enum priv_cmd));
	must_write(PRIV_UNPRIVILEGED, addr, sizeof(struct sockaddr_un));
	priv_wait();
	must_read(PRIV_UNPRIVILEGED, &rc, sizeof(int));
	if (rc < 0)
		return rc;
	return receive_fd(PRIV_UNPRIVILEGED);
}

static void
asroot_ping()
{
	int rc = 1;
	must_write(PRIV_PRIVILEGED, &rc, sizeof(int));
}

static void
asroot_ctl_cleanup()
{
	int len;
	char *ctlname;
	int rc = 0;

	must_read(PRIV_PRIVILEGED, &len, sizeof(int));
	if (len < 0 || len > PATH_MAX)
		fatalx("privsep", "too large value requested");
	if ((ctlname = (char*)malloc(len+1)) == NULL)
		fatal("privsep", NULL);

	must_read(PRIV_PRIVILEGED, ctlname, len);
	ctlname[len] = 0;

	ctl_cleanup(ctlname);
	free(ctlname);

	/* Ack */
	must_write(PRIV_PRIVILEGED, &rc, sizeof(int));
}

static void
asroot_gethostname()
{
	struct utsname un;
	struct addrinfo hints = {
		.ai_flags = AI_CANONNAME
	};
	struct addrinfo *res;
	int len;
	if (uname(&un) < 0)
		fatal("privsep", "failed to get system information");
	if (getaddrinfo(un.nodename, NULL, &hints, &res) != 0) {
		log_info("privsep", "unable to get system name");
#ifdef HAVE_RES_INIT
		res_init();
#endif
                len = strlen(un.nodename);
                must_write(PRIV_PRIVILEGED, &len, sizeof(int));
                must_write(PRIV_PRIVILEGED, un.nodename, len);
        } else {
                len = strlen(res->ai_canonname);
                must_write(PRIV_PRIVILEGED, &len, sizeof(int));
                must_write(PRIV_PRIVILEGED, res->ai_canonname, len);
		freeaddrinfo(res);
        }
}

static void
asroot_iface_init()
{
	int rc = -1, fd = -1;
	int ifindex;
	char name[IFNAMSIZ];
	must_read(PRIV_PRIVILEGED, &ifindex, sizeof(ifindex));
	must_read(PRIV_PRIVILEGED, &name, sizeof(name));
	name[sizeof(name) - 1] = '\0';

	TRACE(LLDPD_PRIV_INTERFACE_INIT(name));
	rc = asroot_iface_init_os(ifindex, name, &fd);
	must_write(PRIV_PRIVILEGED, &rc, sizeof(rc));
	if (rc == 0 && fd >=0) send_fd(PRIV_PRIVILEGED, fd);
	if (fd >= 0) close(fd);
}

static void
asroot_iface_multicast()
{
	int sock = -1, add, rc = 0;
	struct ifreq ifr = { .ifr_name = {} };
	must_read(PRIV_PRIVILEGED, ifr.ifr_name, IFNAMSIZ);
#if defined HOST_OS_LINUX
	must_read(PRIV_PRIVILEGED, ifr.ifr_hwaddr.sa_data, ETHER_ADDR_LEN);
#elif defined HOST_OS_FREEBSD || defined HOST_OS_OSX || defined HOST_OS_DRAGONFLY
	/* Black magic from mtest.c */
	struct sockaddr_dl *dlp = ALIGNED_CAST(struct sockaddr_dl *, &ifr.ifr_addr);
	dlp->sdl_len = sizeof(struct sockaddr_dl);
	dlp->sdl_family = AF_LINK;
	dlp->sdl_index = 0;
	dlp->sdl_nlen = 0;
	dlp->sdl_alen = ETHER_ADDR_LEN;
	dlp->sdl_slen = 0;
	must_read(PRIV_PRIVILEGED, LLADDR(dlp), ETHER_ADDR_LEN);
#elif defined HOST_OS_OPENBSD || defined HOST_OS_NETBSD || defined HOST_OS_SOLARIS
	struct sockaddr *sap = (struct sockaddr *)&ifr.ifr_addr;
#if ! defined HOST_OS_SOLARIS
	sap->sa_len = sizeof(struct sockaddr);
#endif
	sap->sa_family = AF_UNSPEC;
	must_read(PRIV_PRIVILEGED, sap->sa_data, ETHER_ADDR_LEN);
#else
#error Unsupported OS
#endif

	must_read(PRIV_PRIVILEGED, &add, sizeof(int));
	if (((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) ||
	    ((ioctl(sock, (add)?SIOCADDMULTI:SIOCDELMULTI,
		    &ifr) < 0) && (errno != EADDRINUSE)))
		rc = errno;

	if (sock != -1) close(sock);
	must_write(PRIV_PRIVILEGED, &rc, sizeof(rc));
}

static void
asroot_iface_description()
{
	char name[IFNAMSIZ];
	char *description;
	int len, rc;
	must_read(PRIV_PRIVILEGED, &name, sizeof(name));
	name[sizeof(name) - 1] = '\0';
	must_read(PRIV_PRIVILEGED, &len, sizeof(int));
	if (len < 0 || len > PATH_MAX)
		fatalx("privsep", "too large value requested");
	if ((description = (char*)malloc(len+1)) == NULL)
		fatal("privsep", NULL);

	must_read(PRIV_PRIVILEGED, description, len);
	description[len] = 0;
	TRACE(LLDPD_PRIV_INTERFACE_DESCRIPTION(name, description));
	rc = asroot_iface_description_os(name, description);
	must_write(PRIV_PRIVILEGED, &rc, sizeof(rc));
	free(description);
}

static void
asroot_iface_promisc()
{
	char name[IFNAMSIZ];
	int rc;
	must_read(PRIV_PRIVILEGED, &name, sizeof(name));
	name[sizeof(name) - 1] = '\0';
	rc = asroot_iface_promisc_os(name);
	must_write(PRIV_PRIVILEGED, &rc, sizeof(rc));
}

static void
asroot_snmp_socket()
{
	int sock, rc;
	static struct sockaddr_un *addr = NULL;
	struct sockaddr_un bogus;

	if (!addr) {
		addr = (struct sockaddr_un *)malloc(sizeof(struct sockaddr_un));
		if (!addr) fatal("privsep", NULL);
		must_read(PRIV_PRIVILEGED, addr, sizeof(struct sockaddr_un));
	} else
		/* We have already been asked to connect to a socket. We will
		 * connect to the same socket. */
		must_read(PRIV_PRIVILEGED, &bogus, sizeof(struct sockaddr_un));
	if (addr->sun_family != AF_UNIX)
		fatal("privsep", "someone is trying to trick me");
	addr->sun_path[sizeof(addr->sun_path)-1] = '\0';

	if ((sock = socket(PF_UNIX, SOCK_STREAM, 0)) < 0) {
		log_warn("privsep", "cannot open socket");
		must_write(PRIV_PRIVILEGED, &sock, sizeof(int));
		return;
	}
        if ((rc = connect(sock, (struct sockaddr *) addr,
		    sizeof(struct sockaddr_un))) != 0) {
		log_info("privsep", "cannot connect to %s: %s",
                          addr->sun_path, strerror(errno));
		close(sock);
		rc = -1;
		must_write(PRIV_PRIVILEGED, &rc, sizeof(int));
		return;
        }

	int flags;
	if ((flags = fcntl(sock, F_GETFL, NULL)) < 0 ||
	    fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0) {
		log_warn("privsep", "cannot set sock %s to non-block : %s",
                          addr->sun_path, strerror(errno));

		close(sock);
		rc = -1;
		must_write(PRIV_PRIVILEGED, &rc, sizeof(int));
		return;
	}

	must_write(PRIV_PRIVILEGED, &rc, sizeof(int));
	send_fd(PRIV_PRIVILEGED, sock);
	close(sock);
}

struct dispatch_actions {
	enum priv_cmd msg;
	void(*function)(void);
};

static struct dispatch_actions actions[] = {
	{PRIV_PING, asroot_ping},
	{PRIV_DELETE_CTL_SOCKET, asroot_ctl_cleanup},
	{PRIV_GET_HOSTNAME, asroot_gethostname},
#ifdef HOST_OS_LINUX
	{PRIV_OPEN, asroot_open},
#endif
	{PRIV_IFACE_INIT, asroot_iface_init},
	{PRIV_IFACE_MULTICAST, asroot_iface_multicast},
	{PRIV_IFACE_DESCRIPTION, asroot_iface_description},
	{PRIV_IFACE_PROMISC, asroot_iface_promisc},
	{PRIV_SNMP_SOCKET, asroot_snmp_socket},
	{-1, NULL}
};

/* Main loop, run as root */
static void
priv_loop(int privileged, int once)
{
	enum priv_cmd cmd;
	struct dispatch_actions *a;

#ifdef ENABLE_PRIVSEP
	setproctitle("monitor.");
#ifdef USE_SECCOMP
	if (priv_seccomp_init(privileged, monitored) != 0)
	   fatal("privsep", "cannot continue without seccomp setup");
#endif
#endif
	while (!may_read(PRIV_PRIVILEGED, &cmd, sizeof(enum priv_cmd))) {
		log_debug("privsep", "received command %d", cmd);
		for (a = actions; a->function != NULL; a++) {
			if (cmd == a->msg) {
				a->function();
				break;
			}
		}
		if (a->function == NULL)
			fatalx("privsep", "bogus message received");
		if (once) break;
	}
}

/* This function is a NOOP when privilege separation is enabled. In
 * the other case, it should be called when we wait an action from the
 * privileged side. */
void
priv_wait() {
#ifndef ENABLE_PRIVSEP
	/* We have no remote process on the other side. Let's emulate it. */
	priv_loop(0, 1);
#endif
}


#ifdef ENABLE_PRIVSEP
static void
priv_exit_rc_status(int rc, int status) {
	switch (rc) {
	case 0:
		/* kill child */
		kill(monitored, SIGTERM);
		/* we will receive a sigchld in the future */
		return;
	case -1:
		/* child doesn't exist anymore, we consider this is an error to
		 * be here */
		_exit(1);
		break;
	default:
		/* Monitored child has terminated */
		/* Mimic the exit state of the child */
		if (WIFEXITED(status)) {
			/* Normal exit */
			_exit(WEXITSTATUS(status));
		}
		if (WIFSIGNALED(status)) {
			/* Terminated with signal */
			signal(WTERMSIG(status), SIG_DFL);
			raise(WTERMSIG(status));
			_exit(1); /* We consider that not being killed is an error. */
		}
		/* Other cases, consider this as an error. */
		_exit(1);
		break;
	}
}

static void
priv_exit()
{
	int status;
	int rc;
	rc = waitpid(monitored, &status, WNOHANG);
	priv_exit_rc_status(rc, status);
}

/* If priv parent gets a TERM or HUP, pass it through to child instead */
static void
sig_pass_to_chld(int sig)
{
	int oerrno = errno;
	if (monitored != -1)
		kill(monitored, sig);
	errno = oerrno;
}

/* If priv parent gets a SIGCHLD, it will exit if this is the monitored
 * process. Other processes (including lldpcli)) are just reaped without
 * consequences. */
static void
sig_chld(int sig)
{
	int status;
	int rc = waitpid(monitored, &status, WNOHANG);
	if (rc == 0) {
		while ((rc = waitpid(-1, &status, WNOHANG)) > 0) {
			if (rc == monitored) priv_exit_rc_status(rc, status);
		}
		return;
	}
	priv_exit_rc_status(rc, status);
}

/* Create a directory recursively. */
static int mkdir_p(const char *pathname, mode_t mode)
{
	char path[PATH_MAX+1];
	char *current;

	if (strlcpy(path, pathname, sizeof(path)) >= sizeof(path)) {
		errno = ENAMETOOLONG;
		return -1;
	}

	/* Use strtok which will provides non-empty tokens only. */
	for (current = path + 1; *current; current++) {
		if (*current != '/') continue;
		*current = '\0';
		if (mkdir(path, mode) != 0 && errno != EEXIST)
			return -1;
		*current = '/';
	}
	if (mkdir(path, mode) != 0 && errno != EEXIST)
		return -1;

	errno = 0;
	return 0;
}

/* Initialization */
#define LOCALTIME "/etc/localtime"
static void
priv_setup_chroot(const char *chrootdir)
{
	/* Create chroot if it does not exist */
	if (mkdir_p(chrootdir, 0755) == -1) {
		fatal("privsep", "unable to create chroot directory");
	}

	/* Check if /etc/localtime exists in chroot or outside chroot */
	char path[1024];
	int source = -1, destination = -1;
	if (snprintf(path, sizeof(path),
		"%s" LOCALTIME, chrootdir) >= sizeof(path))
		return;
	if ((source = open(LOCALTIME, O_RDONLY)) == -1) {
		if (errno == ENOENT)
			return;
		log_warn("privsep", "cannot read " LOCALTIME);
		return;
	}

	/* Prepare copy of /etc/localtime */
	path[strlen(chrootdir) + 4] = '\0';
	if (mkdir(path, 0755) == -1) {
		if (errno != EEXIST) {
			log_warn("privsep", "unable to create %s directory",
			    path);
			close(source);
			return;
		}
	}
	path[strlen(chrootdir) + 4] = '/';

	/* Do copy */
	char buffer[1024];
	ssize_t n;
	mode_t old = umask(S_IWGRP | S_IWOTH);
	if ((destination = open(path,
		    O_WRONLY | O_CREAT | O_TRUNC | O_EXCL, 0666)) == -1) {
		if (errno != EEXIST)
			log_warn("privsep", "cannot create %s", path);
		close(source);
		umask(old);
		return;
	}
	umask(old);
	while ((n = read(source, buffer, sizeof(buffer))) > 0) {
		ssize_t nw, left = n;
		char *p = buffer;
		while (left > 0) {
			if ((nw = write(destination, p, left)) == -1) {
				if (errno == EINTR) continue;
				log_warn("privsep", "cannot write to %s", path);
				close(source);
				close(destination);
				unlink(path);
				return;
			}
			left -= nw;
			p += nw;
		}
	}
	if (n == -1) {
		log_warn("privsep", "cannot read " LOCALTIME);
		unlink(path);
	} else {
		log_info("privsep", LOCALTIME " copied to chroot");
	}
	close(source);
	close(destination);
}
#else /* !ENABLE_PRIVSEP */

/* Reap any children. It should only be lldpcli since there is not monitored
 * process. */
static void
sig_chld(int sig)
{
	int status = 0;
	while (waitpid(-1, &status, WNOHANG) > 0);
}

#endif

void
priv_drop(uid_t uid, gid_t gid)
{
	gid_t gidset[1];
	gidset[0] = gid;
	log_debug("privsep", "dropping privileges");
#ifdef HAVE_SETRESGID
	if (setresgid(gid, gid, gid) == -1)
		fatal("privsep", "setresgid() failed");
#else
	if (setregid(gid, gid) == -1)
		fatal("privsep", "setregid() failed");
#endif
	if (setgroups(1, gidset) == -1)
		fatal("privsep", "setgroups() failed");
#ifdef HAVE_SETRESUID
	if (setresuid(uid, uid, uid) == -1)
		fatal("privsep", "setresuid() failed");
#else
	if (setreuid(uid, uid) == -1)
		fatal("privsep", "setreuid() failed");
#endif
}

void
priv_caps(uid_t uid, gid_t gid)
{
#ifdef HAVE_LINUX_CAPABILITIES
	cap_t caps;
	const char *caps_strings[2] = {
		"cap_dac_override,cap_net_raw,cap_net_admin,cap_setuid,cap_setgid=pe",
		"cap_dac_override,cap_net_raw,cap_net_admin=pe"
	};
	log_debug("privsep", "getting CAP_NET_RAW/ADMIN and CAP_DAC_OVERRIDE privilege");
	if (!(caps = cap_from_text(caps_strings[0])))
		fatal("privsep", "unable to convert caps");
	if (cap_set_proc(caps) == -1) {
		log_warn("privsep", "unable to drop privileges, monitor running as root");
		cap_free(caps);
		return;
	}
	cap_free(caps);

	if (prctl(PR_SET_KEEPCAPS, 1L, 0L, 0L, 0L) == -1)
		fatal("privsep", "cannot keep capabilities");
	priv_drop(uid, gid);

	log_debug("privsep", "dropping extra capabilities");
	if (!(caps = cap_from_text(caps_strings[1])))
		fatal("privsep", "unable to convert caps");
	if (cap_set_proc(caps) == -1)
		fatal("privsep", "unable to drop extra privileges");
	cap_free(caps);
#else
	log_info("privsep", "no libcap support, running monitor as root");
#endif
}

void
#ifdef ENABLE_PRIVSEP
priv_init(const char *chrootdir, int ctl, uid_t uid, gid_t gid)
#else
priv_init(void)
#endif
{

	int pair[2];

	/* Create socket pair */
	if (socketpair(AF_UNIX, SOCK_STREAM, PF_UNSPEC, pair) < 0) {
		fatal("privsep",
		    "unable to create socket pair for privilege separation");
	}

	priv_unprivileged_fd(pair[0]);
	priv_privileged_fd(pair[1]);

#ifdef ENABLE_PRIVSEP
	/* Spawn off monitor */
	if ((monitored = fork()) < 0)
		fatal("privsep", "unable to fork monitor");
	switch (monitored) {
	case 0:
		/* We are in the children, drop privileges */
		if (RUNNING_ON_VALGRIND)
			log_warnx("privsep", "running on valgrind, keep privileges");
		else {
			priv_setup_chroot(chrootdir);
			if (chroot(chrootdir) == -1)
				fatal("privsep", "unable to chroot");
			if (chdir("/") != 0)
				fatal("privsep", "unable to chdir");
			priv_drop(uid, gid);
		}
		close(pair[1]);
		priv_ping();
		break;
	default:
		/* We are in the monitor */
		if (ctl != -1) close(ctl);
		close(pair[0]);
		if (atexit(priv_exit) != 0)
			fatal("privsep", "unable to set exit function");

		priv_caps(uid, gid);

		/* Install signal handlers */
		const struct sigaction pass_to_child = {
			.sa_handler = sig_pass_to_chld,
			.sa_flags = SA_RESTART
		};
		sigaction(SIGALRM, &pass_to_child, NULL);
		sigaction(SIGTERM, &pass_to_child, NULL);
		sigaction(SIGHUP,  &pass_to_child, NULL);
		sigaction(SIGINT,  &pass_to_child, NULL);
		sigaction(SIGQUIT, &pass_to_child, NULL);
		const struct sigaction child = {
			.sa_handler = sig_chld,
			.sa_flags = SA_RESTART
		};
		sigaction(SIGCHLD, &child, NULL);
		sig_chld(0);	/* Reap already dead children */
		priv_loop(pair[1], 0);
		exit(0);
	}
#else
	const struct sigaction child = {
		.sa_handler = sig_chld,
		.sa_flags = SA_RESTART
	};
	sigaction(SIGCHLD, &child, NULL);
	sig_chld(0);	/* Reap already dead children */
	log_warnx("priv", "no privilege separation available");
	priv_ping();
#endif
}
