/*
 * utils/mountd/auth.c
 *
 * Authentication procedures for mountd.
 *
 * Copyright (C) 1995, 1996 Olaf Kirch <okir@monad.swb.de>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "sockaddr.h"
#include "misc.h"
#include "nfslib.h"
#include "exportfs.h"
#include "mountd.h"
#include "v4root.h"

enum auth_error
{
  bad_path,
  unknown_host,
  no_entry,
  not_exported,
  illegal_port,
  success
};

static void		auth_fixpath(char *path);
static nfs_export my_exp;
static nfs_client my_client;

extern int new_cache;
extern int use_ipaddr;

void
auth_init(void)
{
	auth_reload();
	xtab_mount_write();
}

/*
 * A client can match many different netgroups and it's tough to know
 * beforehand whether it will. If the concatenated string of netgroup
 * m_hostnames is >512 bytes, then enable the "use_ipaddr" mode. This
 * makes mountd change how it matches a client ip address when a mount
 * request comes in. It's more efficient at handling netgroups at the
 * expense of larger kernel caches.
 */
static void
check_useipaddr(void)
{
	nfs_client *clp;
	int old_use_ipaddr = use_ipaddr;
	unsigned int len = 0;

	/* add length of m_hostname + 1 for the comma */
	for (clp = clientlist[MCL_NETGROUP]; clp; clp = clp->m_next)
		len += (strlen(clp->m_hostname) + 1);

	if (len > (NFSCLNT_IDMAX / 2))
		use_ipaddr = 1;
	else
		use_ipaddr = 0;

	if (use_ipaddr != old_use_ipaddr)
		cache_flush(1);
}

unsigned int
auth_reload()
{
	struct stat		stb;
	static ino_t		last_inode;
	static int		last_fd = -1;
	static unsigned int	counter;
	int			fd;

	if ((fd = open(_PATH_ETAB, O_RDONLY)) < 0) {
		xlog(L_FATAL, "couldn't open %s", _PATH_ETAB);
	} else if (fstat(fd, &stb) < 0) {
		xlog(L_FATAL, "couldn't stat %s", _PATH_ETAB);
		close(fd);
	} else if (last_fd != -1 && stb.st_ino == last_inode) {
		/* We opened the etab file before, and its inode
		 * number hasn't changed since then.
		 */
		close(fd);
		return counter;
	} else {
		/* Need to process entries from the etab file.  Close
		 * the file descriptor from the previous open (last_fd),
		 * and keep the current file descriptor open to prevent
		 * the file system reusing the current inode number
		 * (last_inode).
		 */
		if (last_fd != -1)
			close(last_fd);
		last_fd = fd;
		last_inode = stb.st_ino;
	}

	export_freeall();
	memset(&my_client, 0, sizeof(my_client));
	xtab_export_read();
	check_useipaddr();
	v4root_set();

	++counter;

	return counter;
}

static char *get_client_ipaddr_name(const struct sockaddr *caller)
{
	char buf[INET6_ADDRSTRLEN + 1];

	buf[0] = '$';
	host_ntop(caller, buf + 1, sizeof(buf) - 1);
	return strdup(buf);
}

static char *
get_client_hostname(const struct sockaddr *caller, struct addrinfo *ai,
		enum auth_error *error)
{
	char *n;

	if (use_ipaddr)
		return get_client_ipaddr_name(caller);
	n = client_compose(ai);
	*error = unknown_host;
	if (!n)
		return NULL;
	if (*n)
		return n;
	free(n);
	return strdup("DEFAULT");
}

bool ipaddr_client_matches(nfs_export *exp, struct addrinfo *ai)
{
	return client_check(exp->m_client, ai);
}

bool namelist_client_matches(nfs_export *exp, char *dom)
{
	return client_member(dom, exp->m_client->m_hostname);
}

bool client_matches(nfs_export *exp, char *dom, struct addrinfo *ai)
{
	if (is_ipaddr_client(dom))
		return ipaddr_client_matches(exp, ai);
	return namelist_client_matches(exp, dom);
}

/* return static nfs_export with details filled in */
static nfs_export *
auth_authenticate_newcache(const struct sockaddr *caller,
			   const char *path, struct addrinfo *ai,
			   enum auth_error *error)
{
	nfs_export *exp;
	int i;

	free(my_client.m_hostname);

	my_client.m_hostname = get_client_hostname(caller, ai, error);
	if (my_client.m_hostname == NULL)
		return NULL;

	my_client.m_naddr = 1;
	set_addrlist(&my_client, 0, caller);
	my_exp.m_client = &my_client;

	exp = NULL;
	for (i = 0; !exp && i < MCL_MAXTYPES; i++)
		for (exp = exportlist[i].p_head; exp; exp = exp->m_next) {
			if (strcmp(path, exp->m_export.e_path))
				continue;
			if (!client_matches(exp, my_client.m_hostname, ai))
				continue;
			if (exp->m_export.e_flags & NFSEXP_V4ROOT)
				/* not acceptable for v[23] export */
				continue;
			break;
		}
	*error = not_exported;
	if (!exp)
		return NULL;

	my_exp.m_export = exp->m_export;
	exp = &my_exp;
	return exp;
}

static nfs_export *
auth_authenticate_internal(const struct sockaddr *caller, const char *path,
		struct addrinfo *ai, enum auth_error *error)
{
	nfs_export *exp;

	if (new_cache) {
		exp = auth_authenticate_newcache(caller, path, ai, error);
		if (!exp)
			return NULL;
	} else {
		exp = export_find(ai, path);
		if (exp == NULL) {
			*error = no_entry;
			return NULL;
		}
	}
	if (!(exp->m_export.e_flags & NFSEXP_INSECURE_PORT) &&
		     nfs_get_port(caller) >= IPPORT_RESERVED) {
		*error = illegal_port;
		return NULL;
	}
	*error = success;

	return exp;
}

nfs_export *
auth_authenticate(const char *what, const struct sockaddr *caller,
		const char *path)
{
	nfs_export	*exp = NULL;
	char		epath[MAXPATHLEN+1];
	char		*p = NULL;
	char		buf[INET6_ADDRSTRLEN];
	struct addrinfo *ai = NULL;
	enum auth_error	error = bad_path;

	if (path[0] != '/') {
		xlog(L_WARNING, "Bad path in %s request from %s: \"%s\"",
			     what, host_ntop(caller, buf, sizeof(buf)), path);
		return exp;
	}

	strncpy(epath, path, sizeof (epath) - 1);
	epath[sizeof (epath) - 1] = '\0';
	auth_fixpath(epath); /* strip duplicate '/' etc */

	ai = client_resolve(caller);
	if (ai == NULL)
		return exp;

	/* Try the longest matching exported pathname. */
	while (1) {
		exp = auth_authenticate_internal(caller, epath, ai, &error);
		if (exp || (error != not_exported && error != no_entry))
			break;
		/* We have to treat the root, "/", specially. */
		if (p == &epath[1]) break;
		p = strrchr(epath, '/');
		if (p == epath) p++;
		*p = '\0';
	}

	switch (error) {
	case bad_path:
		xlog(L_WARNING, "bad path in %s request from %s: \"%s\"",
		     what, host_ntop(caller, buf, sizeof(buf)), path);
		break;

	case unknown_host:
		xlog(L_WARNING, "refused %s request from %s for %s (%s): unmatched host",
		     what, host_ntop(caller, buf, sizeof(buf)), path, epath);
		break;

	case no_entry:
		xlog(L_WARNING, "refused %s request from %s for %s (%s): no export entry",
		     what, ai->ai_canonname, path, epath);
		break;

	case not_exported:
		xlog(L_WARNING, "refused %s request from %s for %s (%s): not exported",
		     what, ai->ai_canonname, path, epath);
		break;

	case illegal_port:
		xlog(L_WARNING, "refused %s request from %s for %s (%s): illegal port %u",
		     what, ai->ai_canonname, path, epath, nfs_get_port(caller));
		break;

	case success:
		xlog(L_NOTICE, "authenticated %s request from %s:%u for %s (%s)",
		     what, ai->ai_canonname, nfs_get_port(caller), path, epath);
		break;
	default:
		xlog(L_NOTICE, "%s request from %s:%u for %s (%s) gave %d",
		     what, ai->ai_canonname, nfs_get_port(caller),
			path, epath, error);
	}

	freeaddrinfo(ai);
	return exp;
}

static void
auth_fixpath(char *path)
{
	char	*sp, *cp;

	for (sp = cp = path; *sp; sp++) {
		if (*sp != '/' || sp[1] != '/')
			*cp++ = *sp;
	}
	while (cp > path+1 && cp[-1] == '/')
		cp--;
	*cp = '\0';
}
