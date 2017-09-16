/* This is copied from portmap 4.0-29 in RedHat. */

 /*
  * pmap_check - additional portmap security.
  * 
  * Always reject non-local requests to update the portmapper tables.
  * 
  * Refuse to forward mount requests to the nfs mount daemon. Otherwise, the
  * requests would appear to come from the local system, and nfs export
  * restrictions could be bypassed.
  * 
  * Refuse to forward requests to the nfsd process.
  * 
  * Refuse to forward requests to NIS (YP) daemons; The only exception is the
  * YPPROC_DOMAIN_NONACK broadcast rpc call that is used to establish initial
  * contact with the NIS server.
  * 
  * Always allocate an unprivileged port when forwarding a request.
  * 
  * If compiled with -DCHECK_PORT, require that requests to register or
  * unregister a privileged port come from a privileged port. This makes it
  * more difficult to replace a critical service by a trojan.
  * 
  * If compiled with -DHOSTS_ACCESS, reject requests from hosts that are not
  * authorized by the /etc/hosts.{allow,deny} files. The local system is
  * always treated as an authorized host. The access control tables are never
  * consulted for requests from the local system, and are always consulted
  * for requests from other hosts.
  * 
  * Author: Wietse Venema (wietse@wzv.win.tue.nl), dept. of Mathematics and
  * Computing Science, Eindhoven University of Technology, The Netherlands.
  */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_LIBWRAP
#include <unistd.h>
#include <string.h>
#include <rpc/rpc.h>
#include <rpc/pmap_prot.h>
#include <netdb.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/queue.h>
#include <sys/stat.h>
#include <tcpd.h>

#include "sockaddr.h"
#include "tcpwrapper.h"
#include "xlog.h"

#ifdef SYSV40
#include <netinet/in.h>
#include <rpc/rpcent.h>
#endif	/* SYSV40 */

#define ALLOW 1
#define DENY 0

#ifdef IPV6_SUPPORTED
static void
present_address(const struct sockaddr *sap, char *buf, const size_t buflen)
{
	const struct sockaddr_in *sin = (const struct sockaddr_in *)sap;
	const struct sockaddr_in6 *sin6 = (const struct sockaddr_in6 *)sap;
	socklen_t len = (socklen_t)buflen;

	switch (sap->sa_family) {
	case AF_INET:
		if (inet_ntop(AF_INET, &sin->sin_addr, buf, len) != 0)
			return;
	case AF_INET6:
		if (inet_ntop(AF_INET6, &sin6->sin6_addr, buf, len) != 0)
			return;
	}

	memset(buf, 0, buflen);
	strncpy(buf, "unrecognized caller", buflen);
}
#else	/* !IPV6_SUPPORTED */
static void
present_address(const struct sockaddr *sap, char *buf, const size_t buflen)
{
	const struct sockaddr_in *sin = (const struct sockaddr_in *)sap;
	socklen_t len = (socklen_t)buflen;

	if (sap->sa_family == AF_INET)
		if (inet_ntop(AF_INET, &sin->sin_addr, buf, len) != 0)
			return;

	memset(buf, 0, buflen);
	strncpy(buf, "unrecognized caller", (size_t)buflen);
}
#endif	/* !IPV6_SUPPORTED */

typedef struct _haccess_t {
	TAILQ_ENTRY(_haccess_t)	list;
	int			allowed;
	union nfs_sockaddr	address;
} haccess_t;

#define HASH_TABLE_SIZE 1021
typedef struct _hash_head {
	TAILQ_HEAD(host_list, _haccess_t) h_head;
} hash_head;

static hash_head haccess_tbl[HASH_TABLE_SIZE];

static unsigned long
strtoint(const char *str)
{
	unsigned long i, n = 0;
	size_t len = strlen(str);

	for (i = 0; i < len; i++)
		n += (unsigned char)str[i] * i;

	return n;
}

static unsigned int
hashint(const unsigned long num)
{
	return (unsigned int)(num % HASH_TABLE_SIZE);
}

static unsigned int
HASH(const char *addr, const unsigned long program)
{
	return hashint(strtoint(addr) + program);
}

static void
haccess_add(const struct sockaddr *sap, const char *address,
		const unsigned long program, const int allowed)
{
	hash_head *head;
	haccess_t *hptr;
	unsigned int hash;

	hptr = (haccess_t *)malloc(sizeof(haccess_t));
	if (hptr == NULL)
		return;

	hash = HASH(address, program);
	head = &(haccess_tbl[hash]);

	hptr->allowed = allowed;
	memcpy(&hptr->address, sap, (size_t)nfs_sockaddr_length(sap));

	if (TAILQ_EMPTY(&head->h_head))
		TAILQ_INSERT_HEAD(&head->h_head, hptr, list);
	else
		TAILQ_INSERT_TAIL(&head->h_head, hptr, list);
}

static haccess_t *
haccess_lookup(const struct sockaddr *sap, const char *address,
		const unsigned long program)
{
	hash_head *head;
	haccess_t *hptr;
	unsigned int hash;

	hash = HASH(address, program);
	head = &(haccess_tbl[hash]);

	TAILQ_FOREACH(hptr, &head->h_head, list) {
		if (nfs_compare_sockaddr(&hptr->address.sa, sap))
			return hptr;
	}
	return NULL;
}

static void
logit(const char *address)
{
	xlog_warn("connect from %s denied: request from unauthorized host",
			address);
}

static int
good_client(char *name, struct sockaddr *sap)
{
	struct request_info req;

	request_init(&req, RQ_DAEMON, name, RQ_CLIENT_SIN, sap, 0);
	sock_methods(&req);

	if (hosts_access(&req)) 
		return ALLOW;

	return DENY;
}

static int
check_files(void)
{
	static time_t allow_mtime, deny_mtime;
	struct stat astat, dstat;
	int changed = 0;

	if (stat("/etc/hosts.allow", &astat) < 0)
		astat.st_mtime = 0;
	if (stat("/etc/hosts.deny", &dstat) < 0)
		dstat.st_mtime = 0;

	if(!astat.st_mtime || !dstat.st_mtime)
		return changed;

	if (astat.st_mtime != allow_mtime)
		changed = 1;
	else if (dstat.st_mtime != deny_mtime)
		changed = 1;

	allow_mtime = astat.st_mtime;
	deny_mtime = dstat.st_mtime;

	return changed;
}

/**
 * check_default - additional checks for NULL, DUMP, GETPORT and unknown
 * @name: pointer to '\0'-terminated ASCII string containing name of the
 *		daemon requesting the access check
 * @sap: pointer to sockaddr containing network address of caller
 * @program: RPC program number caller is attempting to access
 *
 * Returns TRUE if the caller is allowed access; otherwise FALSE is returned.
 */
int
check_default(char *name, struct sockaddr *sap, const unsigned long program)
{
	haccess_t *acc = NULL;
	int changed = check_files();
	char buf[INET6_ADDRSTRLEN];

	present_address(sap, buf, sizeof(buf));

	acc = haccess_lookup(sap, buf, program);
	if (acc != NULL && changed == 0) {
		xlog(D_GENERAL, "%s: access by %s %s (cached)", __func__,
			buf, acc->allowed ? "ALLOWED" : "DENIED");
		return acc->allowed;
	}

	if (!(from_local(sap) || good_client(name, sap))) {
		logit(buf);
		if (acc != NULL)
			acc->allowed = FALSE;
		else
			haccess_add(sap, buf, program, FALSE);
		xlog(D_GENERAL, "%s: access by %s DENIED", __func__, buf);
		return (FALSE);
	}

	if (acc != NULL)
		acc->allowed = TRUE;
	else
		haccess_add(sap, buf, program, TRUE);
	xlog(D_GENERAL, "%s: access by %s ALLOWED", __func__, buf);

	return (TRUE);
}

#endif	/* HAVE_LIBWRAP */
