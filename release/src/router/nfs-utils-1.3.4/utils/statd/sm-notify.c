/*
 * Send NSM notify calls to all hosts listed in /var/lib/sm
 *
 * Copyright (C) 2004-2006 Olaf Kirch <okir@suse.de>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <err.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <sys/param.h>
#include <sys/syslog.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <time.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <netdb.h>
#include <errno.h>
#include <grp.h>

#include "sockaddr.h"
#include "xlog.h"
#include "nsm.h"
#include "nfsrpc.h"

/* glibc before 2.3.4 */
#ifndef AI_NUMERICSERV
#define AI_NUMERICSERV	0
#endif

#define NSM_TIMEOUT	2
#define NSM_MAX_TIMEOUT	120	/* don't make this too big */

#define NLM_END_GRACE_FILE	"/proc/fs/lockd/nlm_end_grace"

struct nsm_host {
	struct nsm_host *	next;
	char *			name;
	const char *		mon_name;
	const char *		my_name;
	char *			notify_arg;
	struct addrinfo		*ai;
	time_t			last_used;
	time_t			send_next;
	unsigned int		timeout;
	unsigned int		retries;
	uint32_t		xid;
};

static char		nsm_hostname[SM_MAXSTRLEN + 1];
static int		nsm_state;
static int		nsm_family = AF_INET;
static int		opt_debug = 0;
static _Bool		opt_update_state = true;
static unsigned int	opt_max_retry = 15 * 60;
static char *		opt_srcaddr = NULL;
static char *		opt_srcport = NULL;

static void		notify(const int sock);
static int		notify_host(int, struct nsm_host *);
static void		recv_reply(int);
static void		insert_host(struct nsm_host *);
static struct nsm_host *find_host(uint32_t);
static int		record_pid(void);

static struct nsm_host *	hosts = NULL;

__attribute__((__malloc__))
static struct addrinfo *
smn_lookup(const char *name)
{
	struct addrinfo	*ai = NULL;
	struct addrinfo hint = {
		.ai_family	= (nsm_family == AF_INET ? AF_INET: AF_UNSPEC),
		.ai_protocol	= (int)IPPROTO_UDP,
	};
	int error;

	error = getaddrinfo(name, NULL, &hint, &ai);
	if (error != 0) {
		xlog(D_GENERAL, "getaddrinfo(3): %s", gai_strerror(error));
		return NULL;
	}

	return ai;
}

#ifdef HAVE_GETNAMEINFO
static char *
smn_get_hostname(const struct sockaddr *sap, const socklen_t salen,
		const char *name)
{
	char buf[NI_MAXHOST];
	int error;

	error = getnameinfo(sap, salen, buf, sizeof(buf), NULL, 0, NI_NAMEREQD);
	if (error != 0) {
		xlog(L_ERROR, "my_name '%s' is unusable: %s",
			name, gai_strerror(error));
		return NULL;
	}
	return strdup(buf);
}
#else	/* !HAVE_GETNAMEINFO */
static char *
smn_get_hostname(const struct sockaddr *sap,
		__attribute__ ((unused)) const socklen_t salen,
		const char *name)
{
	const struct sockaddr_in *sin = (const struct sockaddr_in *)(char *)sap;
	const struct in_addr *addr = &sin->sin_addr;
	struct hostent *hp;

	if (sap->sa_family != AF_INET) {
		xlog(L_ERROR, "my_name '%s' is unusable: Bad address family",
			name);
		return NULL;
	}

	hp = gethostbyaddr(addr, (socklen_t)sizeof(addr), AF_INET);
	if (hp == NULL) {
		xlog(L_ERROR, "my_name '%s' is unusable: %s",
			name, hstrerror(h_errno));
		return NULL;
	}
	return strdup(hp->h_name);
}
#endif	/* !HAVE_GETNAMEINFO */

/*
 * Presentation addresses are converted to their canonical hostnames.
 * If the IP address does not map to a hostname, it is an error:
 * we never send a presentation address as the argument of SM_NOTIFY.
 *
 * If "name" is not a presentation address, it is left alone.  This
 * allows the administrator some flexibility if DNS isn't configured
 * exactly how sm-notify prefers it.
 *
 * Returns NUL-terminated C string containing the result, or NULL
 * if the canonical name doesn't exist or cannot be determined.
 * The caller must free the result with free(3).
 */
__attribute__((__malloc__))
static char *
smn_verify_my_name(const char *name)
{
	struct addrinfo *ai = NULL;
	struct addrinfo hint = {
#ifdef IPV6_SUPPORTED
		.ai_family	= AF_UNSPEC,
#else	/* !IPV6_SUPPORTED */
		.ai_family	= AF_INET,
#endif	/* !IPV6_SUPPORTED */
		.ai_flags	= AI_NUMERICHOST,
	};
	char *retval;
	int error;

	error = getaddrinfo(name, NULL, &hint, &ai);
	switch (error) {
	case 0:
		/* @name was a presentation address */
		retval = smn_get_hostname(ai->ai_addr, ai->ai_addrlen, name);
		freeaddrinfo(ai);
		if (retval == NULL)
			return NULL;
		break;
	case EAI_NONAME:
		/* @name was not a presentation address */
		retval = strdup(name);
		break;
	default:
		xlog(L_ERROR, "my_name '%s' is unusable: %s",
			name, gai_strerror(error));
		return NULL;
	}

	xlog(D_GENERAL, "Canonical name for my_name '%s': %s",
			name, retval);
	return retval;
}

__attribute__((__malloc__))
static struct nsm_host *
smn_alloc_host(const char *hostname, const char *mon_name,
		const char *my_name, const time_t timestamp)
{
	struct nsm_host	*host;

	host = calloc(1, sizeof(*host));
	if (host == NULL)
		goto out_nomem;

	/*
	 * mon_name and my_name are preserved so sm-notify can
	 * find the right monitor record to remove when it is
	 * done processing this host.
	 */
	host->name = strdup(hostname);
	host->mon_name = (const char *)strdup(mon_name);
	host->my_name = (const char *)strdup(my_name);
	host->notify_arg = strdup(opt_srcaddr != NULL ?
					nsm_hostname : my_name);
	if (host->name == NULL ||
	    host->mon_name == NULL ||
	    host->my_name == NULL ||
	    host->notify_arg == NULL) {
		free(host->notify_arg);
		free((void *)host->my_name);
		free((void *)host->mon_name);
		free(host->name);
		free(host);
		goto out_nomem;
	}

	host->last_used = timestamp;
	host->timeout = NSM_TIMEOUT;
	host->retries = 100;		/* force address retry */

	return host;

out_nomem:
	xlog_warn("Unable to allocate memory");
	return NULL;
}

static void smn_forget_host(struct nsm_host *host)
{
	xlog(D_CALL, "Removing %s (%s, %s) from notify list",
			host->name, host->mon_name, host->my_name);

	nsm_delete_notified_host(host->name, host->mon_name, host->my_name);

	free(host->notify_arg);
	free((void *)host->my_name);
	free((void *)host->mon_name);
	free(host->name);
	if (host->ai)
		freeaddrinfo(host->ai);

	free(host);
}

static unsigned int
smn_get_host(const char *hostname,
		__attribute__ ((unused)) const struct sockaddr *sap,
		const struct mon *m, const time_t timestamp)
{
	struct nsm_host	*host;

	host = smn_alloc_host(hostname,
		m->mon_id.mon_name, m->mon_id.my_id.my_name, timestamp);
	if (host == NULL)
		return 0;

	insert_host(host);
	return 1;
}

#ifdef IPV6_SUPPORTED
static int smn_socket(void)
{
	int sock;

	/*
	 * Use an AF_INET socket if IPv6 is disabled on the
	 * local system.
	 */
	sock = socket(AF_INET6, SOCK_DGRAM, 0);
	if (sock == -1) {
		if (errno != EAFNOSUPPORT) {
			xlog(L_ERROR, "Failed to create RPC socket: %m");
			return -1;
		}
		sock = socket(AF_INET, SOCK_DGRAM, 0);
		if (sock < 0) {
			xlog(L_ERROR, "Failed to create RPC socket: %m");
			return -1;
		}
	} else
		nsm_family = AF_INET6;

	if (fcntl(sock, F_SETFL, O_NONBLOCK) == -1) {
		xlog(L_ERROR, "fcntl(3) on RPC socket failed: %m");
		goto out_close;
	}

	/*
	 * TI-RPC over IPv6 (udp6/tcp6) does not handle IPv4.  However,
	 * since sm-notify open-codes all of its RPC support, it can
	 * use a single socket and let the local network stack provide
	 * the correct mapping between address families automatically.
	 * This is the same thing that is done in the kernel.
	 */
	if (nsm_family == AF_INET6) {
		const int zero = 0;
		socklen_t zerolen = (socklen_t)sizeof(zero);

		if (setsockopt(sock, SOL_IPV6, IPV6_V6ONLY,
					(char *)&zero, zerolen) == -1) {
			xlog(L_ERROR, "setsockopt(3) on RPC socket failed: %m");
			goto out_close;
		}
	}

	return sock;

out_close:
	(void)close(sock);
	return -1;
}
#else	/* !IPV6_SUPPORTED */
static int smn_socket(void)
{
	int sock;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == -1) {
		xlog(L_ERROR, "Failed to create RPC socket: %m");
		return -1;
	}

	if (fcntl(sock, F_SETFL, O_NONBLOCK) == -1) {
		xlog(L_ERROR, "fcntl(3) on RPC socket failed: %m");
		(void)close(sock);
		return -1;
	}

	return sock;
}
#endif	/* !IPV6_SUPPORTED */

/*
 * If admin specified a source address or srcport, then convert those
 * to a sockaddr and return it.   Otherwise, return an ANYADDR address.
 */
__attribute__((__malloc__))
static struct addrinfo *
smn_bind_address(const char *srcaddr, const char *srcport)
{
	struct addrinfo *ai = NULL;
	struct addrinfo hint = {
		.ai_flags	= AI_NUMERICSERV | AI_V4MAPPED,
		.ai_family	= nsm_family,
		.ai_protocol	= (int)IPPROTO_UDP,
	};
	int error;

	if (srcaddr == NULL)
		hint.ai_flags |= AI_PASSIVE;

	/* Do not allow "node" and "service" parameters both to be NULL */
	if (srcport == NULL)
		error = getaddrinfo(srcaddr, "", &hint, &ai);
	else
		error = getaddrinfo(srcaddr, srcport, &hint, &ai);
	if (error != 0) {
		xlog(L_ERROR,
			"Invalid bind address or port for RPC socket: %s",
				gai_strerror(error));
		return NULL;
	}

	return ai;
}

#ifdef HAVE_LIBTIRPC
static int
smn_bindresvport(int sock, struct sockaddr *sap)
{
	return bindresvport_sa(sock, sap);
}

#else	/* !HAVE_LIBTIRPC */
static int
smn_bindresvport(int sock, struct sockaddr *sap)
{
	if (sap->sa_family != AF_INET) {
		errno = EAFNOSUPPORT;
		return -1;
	}

	return bindresvport(sock, (struct sockaddr_in *)(char *)sap);
}
#endif	/* !HAVE_LIBTIRPC */

/*
 * Prepare a socket for sending RPC requests
 *
 * Returns a bound datagram socket file descriptor, or -1 if
 * an error occurs.
 */
static int
smn_create_socket(const char *srcaddr, const char *srcport)
{
	int sock, retry_cnt = 0;
	struct addrinfo *ai;

retry:
	sock = smn_socket();
	if (sock == -1)
		return -1;

	ai = smn_bind_address(srcaddr, srcport);
	if (ai == NULL) {
		(void)close(sock);
		return -1;
	}

	/* Use source port if provided on the command line,
	 * otherwise use bindresvport */
	if (srcport) {
		if (bind(sock, ai->ai_addr, ai->ai_addrlen) == -1) {
			xlog(L_ERROR, "Failed to bind RPC socket: %m");
			freeaddrinfo(ai);
			(void)close(sock);
			return -1;
		}
	} else {
		struct servent *se;

		if (smn_bindresvport(sock, ai->ai_addr) == -1) {
			xlog(L_ERROR,
				"bindresvport on RPC socket failed: %m");
			freeaddrinfo(ai);
			(void)close(sock);
			return -1;
		}

		/* try to avoid known ports */
		se = getservbyport((int)nfs_get_port(ai->ai_addr), "udp");
		if (se != NULL && retry_cnt < 100) {
			retry_cnt++;
			freeaddrinfo(ai);
			(void)close(sock);
			goto retry;
		}
	}

	freeaddrinfo(ai);
	return sock;
}

/* Inform the kernel that it's OK to lift lockd's grace period */
static void
nsm_lift_grace_period(void)
{
	int fd;

	fd = open(NLM_END_GRACE_FILE, O_WRONLY);
	if (fd < 0) {
		/* Don't warn if file isn't present */
		if (errno != ENOENT)
			xlog(L_WARNING, "Unable to open %s: %m",
				NLM_END_GRACE_FILE);
		return;
	}

	if (write(fd, "Y", 1) < 0)
		xlog(L_WARNING, "Unable to write to %s: %m", NLM_END_GRACE_FILE);

	close(fd);
	return;
}

int
main(int argc, char **argv)
{
	int	c, sock, force = 0;
	char *	progname;

	progname = strrchr(argv[0], '/');
	if (progname != NULL)
		progname++;
	else
		progname = argv[0];

	while ((c = getopt(argc, argv, "dm:np:v:P:f")) != -1) {
		switch (c) {
		case 'f':
			force = 1;
			break;
		case 'd':
			opt_debug++;
			break;
		case 'm':
			opt_max_retry = atoi(optarg) * 60;
			break;
		case 'n':
			opt_update_state = false;
			break;
		case 'p':
			opt_srcport = optarg;
			break;
		case 'v':
			opt_srcaddr = optarg;
			break;
		case 'P':
			if (!nsm_setup_pathnames(argv[0], optarg))
				exit(1);
			break;

		default:
			goto usage;
		}
	}

	if (optind < argc) {
usage:		fprintf(stderr,
			"Usage: %s -notify [-dfq] [-m max-retry-minutes] [-p srcport]\n"
			"            [-P /path/to/state/directory] [-v my_host_name]\n",
			progname);
		exit(1);
	}

	if (opt_debug) {
		xlog_syslog(0);
		xlog_stderr(1);
		xlog_config(D_ALL, 1);
	} else {
		xlog_syslog(1);
		xlog_stderr(0);
	}

	xlog_open(progname);
	xlog(L_NOTICE, "Version " VERSION " starting");

	if (nsm_is_default_parentdir()) {
		if (record_pid() == 0 && force == 0 && opt_update_state) {
			/* already run, don't try again */
			xlog(L_NOTICE, "Already notifying clients; Exiting!");
			exit(0);
		}
	}

	if (opt_srcaddr != NULL) {
		char *name;

		name = smn_verify_my_name(opt_srcaddr);
		if (name == NULL)
			exit(1);

		strncpy(nsm_hostname, name, sizeof(nsm_hostname));
		free(name);
	}

	(void)nsm_retire_monitored_hosts();
	if (nsm_load_notify_list(smn_get_host) == 0) {
		xlog(D_GENERAL, "No hosts to notify; exiting");
		nsm_lift_grace_period();
		return 0;
	}

	nsm_state = nsm_get_state(opt_update_state);
	if (nsm_state == 0)
		exit(1);
	nsm_update_kernel_state(nsm_state);

	if (!opt_debug) {
		xlog(L_NOTICE, "Backgrounding to notify hosts...\n");

		if (daemon(0, 0) < 0) {
			xlog(L_ERROR, "unable to background: %m");
			exit(1);
		}

		close(0);
		close(1);
		close(2);
	}

	sock = smn_create_socket(opt_srcaddr, opt_srcport);
	if (sock == -1)
		exit(1);

	if (!nsm_drop_privileges(-1))
		exit(1);

	notify(sock);

	if (hosts) {
		struct nsm_host	*hp;

		while ((hp = hosts) != 0) {
			hosts = hp->next;
			xlog(L_NOTICE, "Unable to notify %s, giving up",
				hp->name);
		}
		exit(1);
	}

	exit(0);
}

/*
 * Notify hosts
 */
static void
notify(const int sock)
{
	time_t	failtime = 0;

	if (opt_max_retry)
		failtime = time(NULL) + opt_max_retry;

	while (hosts) {
		struct pollfd	pfd;
		time_t		now = time(NULL);
		unsigned int	sent = 0;
		struct nsm_host	*hp;
		long		wait;

		if (failtime && now >= failtime)
			break;

		while (hosts && ((wait = hosts->send_next - now) <= 0)) {
			/* Never send more than 10 packets at once */
			if (sent++ >= 10)
				break;

			/* Remove queue head */
			hp = hosts;
			hosts = hp->next;

			if (notify_host(sock, hp))
				continue;

			/* Set the timeout for this call, using an
			   exponential timeout strategy */
			wait = hp->timeout;
			if ((hp->timeout <<= 1) > NSM_MAX_TIMEOUT)
				hp->timeout = NSM_MAX_TIMEOUT;
			hp->send_next = now + wait;
			hp->retries++;

			insert_host(hp);
		}
		if (hosts == NULL)
			return;

		xlog(D_GENERAL, "Host %s due in %ld seconds",
				hosts->name, wait);

		pfd.fd = sock;
		pfd.events = POLLIN;

		wait *= 1000;
		if (wait < 100)
			wait = 100;
		if (poll(&pfd, 1, wait) != 1)
			continue;

		recv_reply(sock);
	}
}

/*
 * Send notification to a single host
 */
static int
notify_host(int sock, struct nsm_host *host)
{
	struct sockaddr *sap;
	socklen_t salen;

	if (host->ai == NULL) {
		host->ai = smn_lookup(host->name);
		if (host->ai == NULL) {
			xlog_warn("DNS resolution of %s failed; "
				"retrying later", host->name);
			return 0;
		}
	}

	/* If we retransmitted 4 times, reset the port to force
	 * a new portmap lookup (in case statd was restarted).
	 * We also rotate through multiple IP addresses at this
	 * point.
	 */
	if (host->retries >= 4) {
		/* don't rotate if there is only one addrinfo */
		if (host->ai->ai_next != NULL) {
			struct addrinfo *first = host->ai;
			struct addrinfo **next = &host->ai;

			/* remove the first entry from the list */
			host->ai = first->ai_next;
			first->ai_next = NULL;
			/* find the end of the list */
			next = &first->ai_next;
			while ( *next )
				next = & (*next)->ai_next;
			/* put first entry at end */
			*next = first;
		}

		nfs_set_port(host->ai->ai_addr, 0);
		host->retries = 0;
	}

	sap = host->ai->ai_addr;
	salen = host->ai->ai_addrlen;

	if (nfs_get_port(sap) == 0)
		host->xid = nsm_xmit_rpcbind(sock, sap, SM_PROG, SM_VERS);
	else
		host->xid = nsm_xmit_notify(sock, sap, salen,
					SM_PROG, host->notify_arg, nsm_state);

	return 0;
}

static void
smn_defer(struct nsm_host *host)
{
	host->xid = 0;
	host->send_next = time(NULL) + NSM_MAX_TIMEOUT;
	host->timeout = NSM_MAX_TIMEOUT;
	insert_host(host);
}

static void
smn_schedule(struct nsm_host *host)
{
	host->retries = 0;
	host->xid = 0;
	host->send_next = time(NULL);
	host->timeout = NSM_TIMEOUT;
	insert_host(host);
}

/*
 * Extract the returned port number and set up the SM_NOTIFY call.
 */
static void
recv_rpcbind_reply(struct sockaddr *sap, struct nsm_host *host, XDR *xdr)
{
	uint16_t port = nsm_recv_rpcbind(sap->sa_family, xdr);

	if (port == 0) {
		/* No binding for statd... */
		xlog(D_GENERAL, "No statd on host %s", host->name);
		smn_defer(host);
	} else {
		xlog(D_GENERAL, "Processing rpcbind reply for %s (port %u)",
			host->name, port);
		nfs_set_port(sap, port);
		smn_schedule(host);
	}
}

/*
 * Successful NOTIFY call. Server returns void.
 *
 * Try sending another SM_NOTIFY with an unqualified "my_name"
 * argument.  Reuse the port number.  If "my_name" is already
 * unqualified, we're done.
 */
static void
recv_notify_reply(struct nsm_host *host)
{
	char *dot = strchr(host->notify_arg, '.');

	if (dot != NULL) {
		*dot = '\0';
		smn_schedule(host);
	} else {
		xlog(D_GENERAL, "Host %s notified successfully", host->name);
		smn_forget_host(host);
	}
}

/*
 * Receive reply from remote host
 */
static void
recv_reply(int sock)
{
	struct nsm_host	*hp;
	struct sockaddr *sap;
	char msgbuf[NSM_MAXMSGSIZE];
	uint32_t	xid;
	ssize_t		msglen;
	XDR		xdr;

	memset(msgbuf, 0 , sizeof(msgbuf));
	msglen = recv(sock, msgbuf, sizeof(msgbuf), 0);
	if (msglen < 0)
		return;

	xlog(D_GENERAL, "Received packet...");

	memset(&xdr, 0, sizeof(xdr));
	xdrmem_create(&xdr, msgbuf, (unsigned int)msglen, XDR_DECODE);
	xid = nsm_parse_reply(&xdr);
	if (xid == 0)
		goto out;

	/* Before we look at the data, find the host struct for
	   this reply */
	if ((hp = find_host(xid)) == NULL)
		goto out;

	sap = hp->ai->ai_addr;
	if (nfs_get_port(sap) == 0)
		recv_rpcbind_reply(sap, hp, &xdr);
	else
		recv_notify_reply(hp);

out:
	xdr_destroy(&xdr);
}

/*
 * Insert host into notification list, sorted by next send time
 */
static void
insert_host(struct nsm_host *host)
{
	struct nsm_host	**where, *p;

	where = &hosts;
	while ((p = *where) != 0) {
		/* Sort in ascending order of timeout */
		if (host->send_next < p->send_next)
			break;
		/* If we have the same timeout, put the
		 * most recently used host first.
		 * This makes sure that "recent" hosts
		 * get notified first.
		 */
		if (host->send_next == p->send_next
		 && host->last_used > p->last_used)
			break;
		where = &p->next;
	}

	host->next = *where;
	*where = host;
	xlog(D_GENERAL, "Added host %s to notify list", host->name);
}

/*
 * Find host given the XID
 */
static struct nsm_host *
find_host(uint32_t xid)
{
	struct nsm_host	**where, *p;

	where = &hosts;
	while ((p = *where) != 0) {
		if (p->xid == xid) {
			*where = p->next;
			return p;
		}
		where = &p->next;
	}
	return NULL;
}

/*
 * Record pid in /var/run/sm-notify.pid
 * This file should remain until a reboot, even if the
 * program exits.
 * If file already exists, fail.
 */
static int record_pid(void)
{
	char pid[20];
	ssize_t len;
	int fd;

	(void)snprintf(pid, sizeof(pid), "%d\n", (int)getpid());
	fd = open("/var/run/sm-notify.pid", O_CREAT|O_EXCL|O_WRONLY, 0600);
	if (fd < 0)
		return 0;

	len = write(fd, pid, strlen(pid));
	if ((len < 0) || ((size_t)len != strlen(pid))) {
		xlog_warn("Writing to pid file failed: errno %d (%m)",
				errno);
	}

	(void)close(fd);
	return 1;
}
