/*
 * stropts.c -- NFS mount using C string to pass options to kernel
 *
 * Copyright (C) 2007 Oracle.  All rights reserved.
 * Copyright (C) 2007 Chuck Lever <chuck.lever@oracle.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 0211-1301 USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <time.h>

#include <sys/socket.h>
#include <sys/mount.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "sockaddr.h"
#include "xcommon.h"
#include "mount.h"
#include "nls.h"
#include "nfsrpc.h"
#include "mount_constants.h"
#include "stropts.h"
#include "error.h"
#include "network.h"
#include "parse_opt.h"
#include "version.h"
#include "parse_dev.h"
#include "conffile.h"

#ifndef NFS_PROGRAM
#define NFS_PROGRAM	(100003)
#endif

#ifndef NFS_PORT
#define NFS_PORT	(2049)
#endif

#ifndef NFS_MAXHOSTNAME
#define NFS_MAXHOSTNAME		(255)
#endif

#ifndef NFS_MAXPATHNAME
#define NFS_MAXPATHNAME		(1024)
#endif

#ifndef NFS_DEF_FG_TIMEOUT_MINUTES
#define NFS_DEF_FG_TIMEOUT_MINUTES	(2u)
#endif

#ifndef NFS_DEF_BG_TIMEOUT_MINUTES
#define NFS_DEF_BG_TIMEOUT_MINUTES	(10000u)
#endif

extern int nfs_mount_data_version;
extern char *progname;
extern int verbose;
extern int sloppy;

struct nfsmount_info {
	const char		*spec,		/* server:/path */
				*node,		/* mounted-on dir */
				*type;		/* "nfs" or "nfs4" */
	char			*hostname;	/* server's hostname */
	struct addrinfo		*address;	/* server's addresses */
	sa_family_t		family;		/* Address family */

	struct mount_options	*options;	/* parsed mount options */
	char			**extra_opts;	/* string for /etc/mtab */

	struct nfs_version	version;	/* NFS version */
	int			flags,		/* MS_ flags */
				fake,		/* actually do the mount? */
				child;		/* forked bg child? */
};


static void nfs_default_version(struct nfsmount_info *mi)
{
#ifdef MOUNT_CONFIG
	extern struct nfs_version config_default_vers;
	/*
	 * Use the default value set in the config file when
	 * the version has not been explicitly set.
	 */
	if (config_default_vers.v_mode == V_PARSE_ERR) {
		mi->version.v_mode = V_PARSE_ERR;
		return;
	}

	if (mi->version.v_mode == V_DEFAULT &&
		config_default_vers.v_mode != V_DEFAULT) {
		mi->version.major = config_default_vers.major;
		mi->version.minor = config_default_vers.minor;
		return;
	}

	if (mi->version.v_mode == V_GENERAL) {
		if (config_default_vers.v_mode != V_DEFAULT &&
		    mi->version.major == config_default_vers.major)
			mi->version.minor = config_default_vers.minor;
		return;
	}

#endif /* MOUNT_CONFIG */
	mi->version.major = 4;
	mi->version.minor = 2;
}

/*
 * Obtain a retry timeout value based on the value of the "retry=" option.
 *
 * Returns a time_t timeout timestamp, in seconds.
 */
static time_t nfs_parse_retry_option(struct mount_options *options,
				     const time_t default_timeout)
{
	time_t timeout_minutes;
	long tmp;

	timeout_minutes = default_timeout;
	switch (po_get_numeric(options, "retry", &tmp)) {
	case PO_NOT_FOUND:
		break;
	case PO_FOUND:
		if (tmp >= 0) {
			timeout_minutes = tmp;
			break;
		}
		/*FALLTHROUGH*/
	case PO_BAD_VALUE:
		if (verbose)
			nfs_error(_("%s: invalid retry timeout was specified; "
					"using default timeout"), progname);
		break;
	}

	return time(NULL) + (timeout_minutes * 60);
}

/*
 * Convert the passed-in sockaddr-style address to presentation
 * format, then append an option of the form "keyword=address".
 *
 * Returns 1 if the option was appended successfully; otherwise zero.
 */
static int nfs_append_generic_address_option(const struct sockaddr *sap,
					     const socklen_t salen,
					     const char *keyword,
					     struct mount_options *options)
{
	char address[NI_MAXHOST];
	char new_option[512];
	int len;

	if (!nfs_present_sockaddr(sap, salen, address, sizeof(address)))
		goto out_err;

	len = snprintf(new_option, sizeof(new_option), "%s=%s",
						keyword, address);
	if (len < 0 || (size_t)len >= sizeof(new_option))
		goto out_err;

	if (po_append(options, new_option) != PO_SUCCEEDED)
		goto out_err;

	return 1;

out_err:
	nfs_error(_("%s: failed to construct %s option"), progname, keyword);
	return 0;
}

/*
 * Append the 'addr=' option to the options string to pass a resolved
 * server address to the kernel.  After a successful mount, this address
 * is also added to /etc/mtab for use when unmounting.
 *
 * If 'addr=' is already present, we strip it out.  This prevents users
 * from setting a bogus 'addr=' option themselves, and also allows bg
 * retries to recompute the server's address, in case it has changed.
 *
 * Returns 1 if 'addr=' option appended successfully;
 * otherwise zero.
 */
static int nfs_append_addr_option(const struct sockaddr *sap,
				  socklen_t salen,
				  struct mount_options *options)
{
	po_remove_all(options, "addr");
	return nfs_append_generic_address_option(sap, salen, "addr", options);
}

/*
 * Called to discover our address and append an appropriate 'clientaddr='
 * option to the options string.
 *
 * Returns 1 if 'clientaddr=' option created successfully or if
 * 'clientaddr=' option is already present; otherwise zero.
 */
static int nfs_append_clientaddr_option(const struct sockaddr *sap,
					socklen_t salen,
					struct mount_options *options)
{
	union nfs_sockaddr address;
	struct sockaddr *my_addr = &address.sa;
	socklen_t my_len = sizeof(address);

	if (po_contains(options, "clientaddr") == PO_FOUND)
		return 1;

	nfs_callback_address(sap, salen, my_addr, &my_len);

	return nfs_append_generic_address_option(my_addr, my_len,
							"clientaddr", options);
}

/*
 * Determine whether to append a 'mountaddr=' option.  The option is needed if:
 *
 *   1. "mounthost=" was specified, or
 *   2. The address families for proto= and mountproto= are different.
 */
static int nfs_fix_mounthost_option(struct mount_options *options,
		const char *nfs_hostname)
{
	union nfs_sockaddr address;
	struct sockaddr *sap = &address.sa;
	socklen_t salen = sizeof(address);
	sa_family_t nfs_family, mnt_family;
	char *mounthost;

	if (!nfs_nfs_proto_family(options, &nfs_family))
		return 0;
	if (!nfs_mount_proto_family(options, &mnt_family))
		return 0;

	mounthost = po_get(options, "mounthost");
	if (mounthost == NULL) {
		if (nfs_family == mnt_family)
			return 1;
		mounthost = (char *)nfs_hostname;
	}

	if (!nfs_lookup(mounthost, mnt_family, sap, &salen)) {
		nfs_error(_("%s: unable to determine mount server's address"),
				progname);
		return 0;
	}

	return nfs_append_generic_address_option(sap, salen,
							"mountaddr", options);
}

/*
 * Returns zero if the "lock" option is in effect, but statd
 * can't be started.  Otherwise, returns 1.
 */
static const char *nfs_lock_opttbl[] = {
	"nolock",
	"lock",
	NULL,
};

static int nfs_verify_lock_option(struct mount_options *options)
{
	if (po_rightmost(options, nfs_lock_opttbl) == 0)
		return 1;

	if (!start_statd()) {
		nfs_error(_("%s: rpc.statd is not running but is "
			    "required for remote locking."), progname);
		nfs_error(_("%s: Either use '-o nolock' to keep "
			    "locks local, or start statd."), progname);
		errno = EALREADY; /* Don't print further error message */
		return 0;
	}

	return 1;
}

static int nfs_append_sloppy_option(struct mount_options *options)
{
	if (!sloppy || linux_version_code() < MAKE_VERSION(2, 6, 27))
		return 1;

	if (po_append(options, "sloppy") == PO_FAILED)
		return 0;
	return 1;
}

static int nfs_set_version(struct nfsmount_info *mi)
{
	if (!nfs_nfs_version(mi->options, &mi->version))
		return 0;

	if (strncmp(mi->type, "nfs4", 4) == 0)
		mi->version.major = 4;

	/*
	 * Before 2.6.32, the kernel NFS client didn't
	 * support "-t nfs vers=4" mounts, so NFS version
	 * 4 cannot be included when autonegotiating
	 * while running on those kernels.
	 */
	if (mi->version.v_mode == V_DEFAULT &&
	    linux_version_code() <= MAKE_VERSION(2, 6, 31)) {
		mi->version.major = 3;
		mi->version.v_mode = V_SPECIFIC;
	}

	/*
	 * If we still don't know, check for version-specific
	 * mount options.
	 */
	if (mi->version.v_mode == V_DEFAULT) {
		if (po_contains(mi->options, "mounthost") ||
		    po_contains(mi->options, "mountaddr") ||
		    po_contains(mi->options, "mountvers") ||
		    po_contains(mi->options, "mountproto")) {
			mi->version.major = 3;
			mi->version.v_mode = V_SPECIFIC;
		}
	}

	/*
	 * If enabled, see if the default version was
	 * set in the config file
	 */
	if (mi->version.v_mode != V_SPECIFIC) {
		nfs_default_version(mi);
		/*
		 * If the version was not specifically set, it will
		 * be set by autonegotiation later, so remove it now:
		 */
		po_remove_all(mi->options, "v4");
		po_remove_all(mi->options, "vers");
		po_remove_all(mi->options, "nfsvers");
	}

	if (mi->version.v_mode == V_PARSE_ERR)
		return 0;

	return 1;
}

/*
 * Set up mandatory non-version specific NFS mount options.
 *
 * Returns 1 if successful; otherwise zero.
 */
static int nfs_validate_options(struct nfsmount_info *mi)
{
	if (!nfs_parse_devname(mi->spec, &mi->hostname, NULL))
		return 0;

	if (!nfs_nfs_proto_family(mi->options, &mi->family))
		return 0;

	/*
	 * A remount is not going to be able to change the server's address,
	 * nor should we try to resolve another address for the server as we
	 * may end up with a different address.
	 * A non-remount will set 'addr' from ->hostname
	 */
	po_remove_all(mi->options, "addr");

	if (!nfs_set_version(mi))
		return 0;

	if (!nfs_append_sloppy_option(mi->options))
		return 0;

	return 1;
}

/*
 * Get NFS/mnt server addresses from mount options
 *
 * Returns 1 and fills in @nfs_saddr, @nfs_salen, @mnt_saddr, and @mnt_salen
 * if all goes well; otherwise zero.
 */
static int nfs_extract_server_addresses(struct mount_options *options,
					struct sockaddr *nfs_saddr,
					socklen_t *nfs_salen,
					struct sockaddr *mnt_saddr,
					socklen_t *mnt_salen)
{
	char *option;

	option = po_get(options, "addr");
	if (option == NULL)
		return 0;
	if (!nfs_string_to_sockaddr(option, nfs_saddr, nfs_salen))
		return 0;

	option = po_get(options, "mountaddr");
	if (option == NULL) {
		memcpy(mnt_saddr, nfs_saddr, *nfs_salen);
		*mnt_salen = *nfs_salen;
	} else if (!nfs_string_to_sockaddr(option, mnt_saddr, mnt_salen))
		return 0;

	return 1;
}

static int nfs_construct_new_options(struct mount_options *options,
				     struct sockaddr *nfs_saddr,
				     struct pmap *nfs_pmap,
				     struct sockaddr *mnt_saddr,
				     struct pmap *mnt_pmap)
{
	char new_option[64];
	char *netid;

	po_remove_all(options, "nfsprog");
	po_remove_all(options, "mountprog");

	po_remove_all(options, "v2");
	po_remove_all(options, "v3");
	po_remove_all(options, "vers");
	po_remove_all(options, "nfsvers");
	snprintf(new_option, sizeof(new_option) - 1,
		 "vers=%lu", nfs_pmap->pm_vers);
	if (po_append(options, new_option) == PO_FAILED)
		return 0;

	po_remove_all(options, "proto");
	po_remove_all(options, "udp");
	po_remove_all(options, "tcp");
	netid = nfs_get_netid(nfs_saddr->sa_family, nfs_pmap->pm_prot);
	if (netid == NULL)
		return 0;
	snprintf(new_option, sizeof(new_option) - 1,
			 "proto=%s", netid);
	free(netid);
	if (po_append(options, new_option) == PO_FAILED)
		return 0;

	if(po_remove_all(options, "port") == PO_FOUND ||
	   nfs_pmap->pm_port != NFS_PORT) {
		snprintf(new_option, sizeof(new_option) - 1,
			 "port=%lu", nfs_pmap->pm_port);
		if (po_append(options, new_option) == PO_FAILED)
			return 0;
	}

	po_remove_all(options, "mountvers");
	snprintf(new_option, sizeof(new_option) - 1,
		 "mountvers=%lu", mnt_pmap->pm_vers);
	if (po_append(options, new_option) == PO_FAILED)
		return 0;

	po_remove_all(options, "mountproto");
	netid = nfs_get_netid(mnt_saddr->sa_family, mnt_pmap->pm_prot);
	if (netid == NULL)
		return 0;
	snprintf(new_option, sizeof(new_option) - 1,
			 "mountproto=%s", netid);
	free(netid);
	if (po_append(options, new_option) == PO_FAILED)
		return 0;

	po_remove_all(options, "mountport");
	snprintf(new_option, sizeof(new_option) - 1,
		 "mountport=%lu", mnt_pmap->pm_port);
	if (po_append(options, new_option) == PO_FAILED)
		return 0;

	return 1;
}

/*
 * Reconstruct the mount option string based on a portmapper probe
 * of the server.  Returns one if the server's portmapper returned
 * something we can use, otherwise zero.
 *
 * To handle version and transport protocol fallback properly, we
 * need to parse some of the mount options in order to set up a
 * portmap probe.  Mount options that nfs_rewrite_pmap_mount_options()
 * doesn't recognize are left alone.
 *
 * Returns TRUE if rewriting was successful; otherwise
 * FALSE is returned if some failure occurred.
 */
static int
nfs_rewrite_pmap_mount_options(struct mount_options *options, int checkv4)
{
	union nfs_sockaddr nfs_address;
	struct sockaddr *nfs_saddr = &nfs_address.sa;
	socklen_t nfs_salen = sizeof(nfs_address);
	struct pmap nfs_pmap;
	union nfs_sockaddr mnt_address;
	struct sockaddr *mnt_saddr = &mnt_address.sa;
	socklen_t mnt_salen = sizeof(mnt_address);
	unsigned long protocol;
	struct pmap mnt_pmap;

	/*
	 * Version and transport negotiation is not required
	 * and does not work for RDMA mounts.
	 */
	if (!nfs_nfs_protocol(options, &protocol)) {
		errno = EINVAL;
		return 0;
	}
	if (protocol == NFSPROTO_RDMA)
		goto out;

	/*
	 * Extract just the options needed to contact server.
	 * Bail now if any of these have bad values.
	 */
	if (!nfs_extract_server_addresses(options, nfs_saddr, &nfs_salen,
						mnt_saddr, &mnt_salen)) {
		errno = EINVAL;
		return 0;
	}
	if (!nfs_options2pmap(options, &nfs_pmap, &mnt_pmap)) {
		errno = EINVAL;
		return 0;
	}

	/*
	 * The kernel NFS client doesn't support changing the RPC
	 * program number for these services, so force the value of
	 * these fields before probing the server's ports.
	 */
	nfs_pmap.pm_prog = NFS_PROGRAM;
	mnt_pmap.pm_prog = MOUNTPROG;

	/*
	 * If the server's rpcbind service isn't available, we can't
	 * negotiate.  Bail now if we can't contact it.
	 */
	if (!nfs_probe_bothports(mnt_saddr, mnt_salen, &mnt_pmap,
				 nfs_saddr, nfs_salen, &nfs_pmap, checkv4)) {
		errno = ESPIPE;
		if (rpc_createerr.cf_stat == RPC_PROGNOTREGISTERED)
			errno = EOPNOTSUPP;
		else if (rpc_createerr.cf_stat == RPC_AUTHERROR)
			errno = EACCES;
		else if (rpc_createerr.cf_stat == RPC_TIMEDOUT)
			errno = ETIMEDOUT;
		else if (rpc_createerr.cf_stat == RPC_PROGVERSMISMATCH)
			errno = EPROTONOSUPPORT;
		else if (rpc_createerr.cf_error.re_errno != 0)
			errno = rpc_createerr.cf_error.re_errno;
		return 0;
	}

	if (!nfs_construct_new_options(options, nfs_saddr, &nfs_pmap,
					mnt_saddr, &mnt_pmap)) {
		if (rpc_createerr.cf_stat == RPC_UNKNOWNPROTO)
			errno = EPROTONOSUPPORT;
		else
			errno = EINVAL;
		return 0;
	}

out:
	errno = 0;
	return 1;
}

/*
 * Do the mount(2) system call.
 *
 * Returns TRUE if successful, otherwise FALSE.
 * "errno" is set to reflect the individual error.
 */
static int nfs_sys_mount(struct nfsmount_info *mi, struct mount_options *opts)
{
	char *options = NULL;
	int result;

	if (mi->fake)
		return 1;

	if (po_join(opts, &options) == PO_FAILED) {
		errno = EIO;
		return 0;
	}

	result = mount(mi->spec, mi->node, mi->type,
			mi->flags & ~(MS_USER|MS_USERS), options);
	free(options);

	if (verbose && result) {
		int save = errno;
		nfs_error(_("%s: mount(2): %s"), progname, strerror(save));
		errno = save;
	}
	return !result;
}

static int nfs_do_mount_v3v2(struct nfsmount_info *mi,
			     struct sockaddr *sap, socklen_t salen,
			     int checkv4)
{
	struct mount_options *options = po_dup(mi->options);
	int result = 0;

	if (!options) {
		errno = ENOMEM;
		return result;
	}
	errno = 0;
	if (!nfs_append_addr_option(sap, salen, options)) {
		if (errno == 0)
			errno = EINVAL;
		goto out_fail;
	}

	if (!nfs_fix_mounthost_option(options, mi->hostname)) {
		if (errno == 0)
			errno = EINVAL;
		goto out_fail;
	}
	if (!mi->fake && !nfs_verify_lock_option(options)) {
		if (errno == 0)
			errno = EINVAL;
		goto out_fail;
	}

	/*
	 * Options we negotiate below may be stale by the time this
	 * file system is unmounted.  In order to force umount.nfs
	 * to renegotiate with the server, only write the user-
	 * specified options, and not negotiated options, to /etc/mtab.
	 */
	if (po_join(options, mi->extra_opts) == PO_FAILED) {
		errno = ENOMEM;
		goto out_fail;
	}

	if (verbose)
		printf(_("%s: trying text-based options '%s'\n"),
			progname, *mi->extra_opts);

	if (!nfs_rewrite_pmap_mount_options(options, checkv4))
		goto out_fail;

	result = nfs_sys_mount(mi, options);

out_fail:
	po_destroy(options);
	return result;
}

/*
 * Attempt a "-t nfs vers=2" or "-t nfs vers=3" mount.
 *
 * Returns TRUE if successful, otherwise FALSE.
 * "errno" is set to reflect the individual error.
 */
static int nfs_try_mount_v3v2(struct nfsmount_info *mi, int checkv4)
{
	struct addrinfo *ai;
	int ret = 0;

	for (ai = mi->address; ai != NULL; ai = ai->ai_next) {
		ret = nfs_do_mount_v3v2(mi, ai->ai_addr, ai->ai_addrlen, checkv4);
		if (ret != 0)
			return ret;

		switch (errno) {
		case ECONNREFUSED:
		case EOPNOTSUPP:
		case EHOSTUNREACH:
		case ETIMEDOUT:
		case EACCES:
			continue;
		default:
			goto out;
		}
	}
out:
	return ret;
}

static int nfs_do_mount_v4(struct nfsmount_info *mi,
		struct sockaddr *sap, socklen_t salen)
{
	struct mount_options *options = po_dup(mi->options);
	int result = 0;
	char version_opt[16];
	char *extra_opts = NULL;

	if (!options) {
		errno = ENOMEM;
		return result;
	}

	if (po_contains(options, "mounthost") ||
		po_contains(options, "mountaddr") ||
		po_contains(options, "mountvers") ||
		po_contains(options, "mountproto")) {
	/*
	 * Since these mountd options are set assume version 3
	 * is wanted so error out with EPROTONOSUPPORT so the
	 * protocol negation starts with v3.
	 */
		errno = EPROTONOSUPPORT;
		goto out_fail;
	}

	if (mi->version.v_mode != V_SPECIFIC) {
		if (mi->version.v_mode == V_GENERAL)
			snprintf(version_opt, sizeof(version_opt) - 1,
				"vers=%lu", mi->version.major);
		else
			snprintf(version_opt, sizeof(version_opt) - 1,
				"vers=%lu.%lu", mi->version.major,
				mi->version.minor);

		if (po_append(options, version_opt) == PO_FAILED) {
			errno = EINVAL;
			goto out_fail;
		}
	}

	if (!nfs_append_addr_option(sap, salen, options)) {
		errno = EINVAL;
		goto out_fail;
	}

	if (!nfs_append_clientaddr_option(sap, salen, options)) {
		errno = EINVAL;
		goto out_fail;
	}

	if (po_join(options, &extra_opts) == PO_FAILED) {
		errno = ENOMEM;
		goto out_fail;
	}

	if (verbose)
		printf(_("%s: trying text-based options '%s'\n"),
			progname, extra_opts);

	result = nfs_sys_mount(mi, options);

	/*
	 * If success, update option string to be recorded in /etc/mtab.
	 */
	if (result) {
	    free(*mi->extra_opts);
	    *mi->extra_opts = extra_opts;
	} else
	    free(extra_opts);

out_fail:
	po_destroy(options);
	return result;
}

/*
 * Attempt a "-t nfs -o vers=4" or "-t nfs4" mount.
 *
 * Returns TRUE if successful, otherwise FALSE.
 * "errno" is set to reflect the individual error.
 */
static int nfs_try_mount_v4(struct nfsmount_info *mi)
{
	struct addrinfo *ai;
	int ret = 0;

	for (ai = mi->address; ai != NULL; ai = ai->ai_next) {
		ret = nfs_do_mount_v4(mi, ai->ai_addr, ai->ai_addrlen);
		if (ret != 0)
			return ret;

		switch (errno) {
		case ECONNREFUSED:
		case EHOSTUNREACH:
		case ETIMEDOUT:
		case EACCES:
			continue;
		default:
			goto out;
		}
	}
out:
	return ret;
}

/*
 * Handle NFS version and transport protocol
 * autonegotiation.
 *
 * When no version or protocol is specified on the
 * command line, mount.nfs negotiates with the server
 * to determine appropriate settings for the new
 * mount point.
 *
 * Returns TRUE if successful, otherwise FALSE.
 * "errno" is set to reflect the individual error.
 */
static int nfs_autonegotiate(struct nfsmount_info *mi)
{
	int result;

	result = nfs_try_mount_v4(mi);
check_result:
	if (result)
		return result;

	switch (errno) {
	case EPROTONOSUPPORT:
		/* A clear indication that the server or our
		 * client does not support NFS version 4 and minor */
	case EINVAL:
		/* A less clear indication that our client
		 * does not support NFSv4 minor version. */
		if (mi->version.v_mode == V_GENERAL &&
			mi->version.minor == 0)
				return result;
		if (mi->version.v_mode != V_SPECIFIC) {
			if (mi->version.minor > 0) {
				mi->version.minor--;
				result = nfs_try_mount_v4(mi);
				goto check_result;
			}
		}

		goto fall_back;
	case ENOENT:
		/* Legacy Linux servers don't export an NFS
		 * version 4 pseudoroot. */
		goto fall_back;
	case EPERM:
		/* Linux servers prior to 2.6.25 may return
		 * EPERM when NFS version 4 is not supported. */
		goto fall_back;
	case ECONNREFUSED:
		/* UDP-Only servers won't support v4, but maybe it
		 * just isn't ready yet.  So try v3, but double-check
		 * with rpcbind for v4. */
		result = nfs_try_mount_v3v2(mi, TRUE);
		if (result == 0 && errno == EAGAIN) {
			/* v4 server seems to be registered now. */
			result = nfs_try_mount_v4(mi);
			if (result == 0 && errno != ECONNREFUSED)
				goto check_result;
		}
		return result;
	default:
		return result;
	}

fall_back:
	return nfs_try_mount_v3v2(mi, FALSE);
}

/*
 * This is a single pass through the fg/bg loop.
 *
 * Returns TRUE if successful, otherwise FALSE.
 * "errno" is set to reflect the individual error.
 */
static int nfs_try_mount(struct nfsmount_info *mi)
{
	int result = 0;

	if (mi->address == NULL) {
		struct addrinfo hint = {
			.ai_protocol	= (int)IPPROTO_UDP,
		};
		int error;
		struct addrinfo *address;

		hint.ai_family = (int)mi->family;
		error = getaddrinfo(mi->hostname, NULL, &hint, &address);
		if (error != 0) {
			if (error == EAI_AGAIN)
				errno = EAGAIN;
			else {
				nfs_error(_("%s: Failed to resolve server %s: %s"),
					  progname, mi->hostname, gai_strerror(error));
				errno = EALREADY;
			}
			return 0;
		}

		if (!nfs_append_addr_option(address->ai_addr,
					    address->ai_addrlen, mi->options))
			return 0;
		mi->address = address;
	}

	switch (mi->version.major) {
		case 2:
		case 3:
			result = nfs_try_mount_v3v2(mi, FALSE);
			break;
		case 4:
			if (mi->version.v_mode != V_SPECIFIC)
				result = nfs_autonegotiate(mi);
			else
				result = nfs_try_mount_v4(mi);
			break;
		default:
			errno = EIO;
	}

	return result;
}

/*
 * Distinguish between permanent and temporary errors.
 *
 * Basically, we retry if communication with the server has
 * failed so far, but fail immediately if there is a local
 * error (like a bad mount option).
 *
 * ESTALE is also a temporary error because some servers
 * return ESTALE when a share is temporarily offline.
 *
 * Returns 1 if we should fail immediately, or 0 if we
 * should retry.
 */
static int nfs_is_permanent_error(int error)
{
	switch (error) {
	case ESTALE:
	case ETIMEDOUT:
	case ECONNREFUSED:
	case EHOSTUNREACH:
	case EAGAIN:
		return 0;	/* temporary */
	default:
		return 1;	/* permanent */
	}
}

/*
 * Handle "foreground" NFS mounts.
 *
 * Retry the mount request for as long as the 'retry=' option says.
 *
 * Returns a valid mount command exit code.
 */
static int nfsmount_fg(struct nfsmount_info *mi)
{
	unsigned int secs = 1;
	time_t timeout;

	timeout = nfs_parse_retry_option(mi->options,
					 NFS_DEF_FG_TIMEOUT_MINUTES);
	if (verbose)
		printf(_("%s: timeout set for %s"),
			progname, ctime(&timeout));

	for (;;) {
		if (nfs_try_mount(mi))
			return EX_SUCCESS;

		if (errno == EBUSY)
			/* The only cause of EBUSY is if exactly the desired
			 * filesystem is already mounted.  That can arguably
			 * be seen as success.  "mount -a" tries to optimise
			 * out this case but sometimes fails.  Help it out
			 * by pretending everything is rosy
			 */
			return EX_SUCCESS;

		if (nfs_is_permanent_error(errno))
			break;

		if (time(NULL) > timeout) {
			errno = ETIMEDOUT;
			break;
		}

		if (errno != ETIMEDOUT) {
			if (sleep(secs))
				break;
			secs <<= 1;
			if (secs > 10)
				secs = 10;
		}
	};

	mount_error(mi->spec, mi->node, errno);
	return EX_FAIL;
}

/*
 * Handle "background" NFS mount [first try]
 *
 * Returns a valid mount command exit code.
 *
 * EX_BG should cause the caller to fork and invoke nfsmount_child.
 */
static int nfsmount_parent(struct nfsmount_info *mi)
{
	if (nfs_try_mount(mi))
		return EX_SUCCESS;

	/* retry background mounts when the server is not up */
	if (nfs_is_permanent_error(errno) && errno != EOPNOTSUPP) {
		mount_error(mi->spec, mi->node, errno);
		return EX_FAIL;
	}

	sys_mount_errors(mi->hostname, errno, 1, 1);
	return EX_BG;
}

/*
 * Handle "background" NFS mount [retry daemon]
 *
 * Returns a valid mount command exit code: EX_SUCCESS if successful,
 * EX_FAIL if a failure occurred.  There's nothing to catch the
 * error return, though, so we use sys_mount_errors to log the
 * failure.
 */
static int nfsmount_child(struct nfsmount_info *mi)
{
	unsigned int secs = 1;
	time_t timeout;

	timeout = nfs_parse_retry_option(mi->options,
					 NFS_DEF_BG_TIMEOUT_MINUTES);

	for (;;) {
		if (sleep(secs))
			break;
		secs <<= 1;
		if (secs > 120)
			secs = 120;

		if (nfs_try_mount(mi))
			return EX_SUCCESS;

		/* retry background mounts when the server is not up */
		if (nfs_is_permanent_error(errno) && errno != EOPNOTSUPP)
			break;

		if (time(NULL) > timeout)
			break;

		sys_mount_errors(mi->hostname, errno, 1, 1);
	};

	sys_mount_errors(mi->hostname, errno, 1, 0);
	return EX_FAIL;
}

/*
 * Handle "background" NFS mount
 *
 * Returns a valid mount command exit code.
 */
static int nfsmount_bg(struct nfsmount_info *mi)
{
	if (!mi->child)
		return nfsmount_parent(mi);
	else
		return nfsmount_child(mi);
}

/*
 * Usually all that is needed for an NFS remount is to change
 * generic mount options like "sync" or "ro".  These generic
 * options are controlled by mi->flags, not by text-based
 * options, and no contact with the server is needed.
 *
 * Take care with the /etc/mtab entry for this mount; just
 * calling update_mtab() will change an "-t nfs -o vers=4"
 * mount to an "-t nfs -o remount" mount, and that will
 * confuse umount.nfs.
 *
 * Returns a valid mount command exit code.
 */
static int nfs_remount(struct nfsmount_info *mi)
{
	if (nfs_sys_mount(mi, mi->options))
		return EX_SUCCESS;
	mount_error(mi->spec, mi->node, errno);
	return EX_FAIL;
}

/*
 * Process mount options and try a mount system call.
 *
 * Returns a valid mount command exit code.
 */
static const char *nfs_background_opttbl[] = {
	"bg",
	"fg",
	NULL,
};

static int nfsmount_start(struct nfsmount_info *mi)
{
	if (!nfs_validate_options(mi))
		return EX_FAIL;

	/*
	 * Avoid retry and negotiation logic when remounting
	 */
	if (mi->flags & MS_REMOUNT)
		return nfs_remount(mi);

	if (po_rightmost(mi->options, nfs_background_opttbl) == 0)
		return nfsmount_bg(mi);
	else
		return nfsmount_fg(mi);
}

/**
 * nfsmount_string - Mount an NFS file system using C string options
 * @spec: C string specifying remote share to mount ("hostname:path")
 * @node: C string pathname of local mounted-on directory
 * @type: C string that represents file system type ("nfs" or "nfs4")
 * @flags: MS_ style mount flags
 * @extra_opts:	pointer to C string containing fs-specific mount options
 *		(input and output argument)
 * @fake: flag indicating whether to carry out the whole operation
 * @child: one if this is a mount daemon (bg)
 *
 * Returns a valid mount command exit code.
 */
int nfsmount_string(const char *spec, const char *node, const char *type,
		    int flags, char **extra_opts, int fake, int child)
{
	struct nfsmount_info mi = {
		.spec		= spec,
		.node		= node,
		.address	= NULL,
		.type		= type,
		.extra_opts	= extra_opts,
		.flags		= flags,
		.fake		= fake,
		.child		= child,
	};
	int retval = EX_FAIL;

	mi.options = po_split(*extra_opts);
	if (mi.options) {
		retval = nfsmount_start(&mi);
		po_destroy(mi.options);
	} else
		nfs_error(_("%s: internal option parsing error"), progname);

	freeaddrinfo(mi.address);
	free(mi.hostname);
	return retval;
}
