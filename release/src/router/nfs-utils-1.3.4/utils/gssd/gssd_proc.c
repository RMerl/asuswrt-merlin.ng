/*
  gssd_proc.c

  Copyright (c) 2000-2004 The Regents of the University of Michigan.
  All rights reserved.

  Copyright (c) 2000 Dug Song <dugsong@UMICH.EDU>.
  Copyright (c) 2001 Andy Adamson <andros@UMICH.EDU>.
  Copyright (c) 2002 Marius Aamodt Eriksen <marius@UMICH.EDU>.
  Copyright (c) 2002 Bruce Fields <bfields@UMICH.EDU>
  Copyright (c) 2004 Kevin Coffman <kwc@umich.edu>
  Copyright (c) 2014 David H?rdeman <david@hardeman.nu>
  All rights reserved, all wrongs reversed.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
  3. Neither the name of the University nor the names of its
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif	/* HAVE_CONFIG_H */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sys/param.h>
#include <rpc/rpc.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/fsuid.h>

#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include <string.h>
#include <dirent.h>
#include <poll.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <gssapi/gssapi.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <syscall.h>

#include "gssd.h"
#include "err_util.h"
#include "gss_util.h"
#include "krb5_util.h"
#include "context.h"
#include "nfsrpc.h"
#include "nfslib.h"
#include "gss_names.h"

/* Encryption types supported by the kernel rpcsec_gss code */
int num_krb5_enctypes = 0;
krb5_enctype *krb5_enctypes = NULL;

/*
 * Parse the supported encryption type information
 */
static int
parse_enctypes(char *enctypes)
{
	int n = 0;
	char *curr, *comma;
	int i;
	static char *cached_types;

	if (cached_types && strcmp(cached_types, enctypes) == 0)
		return 0;
	free(cached_types);

	if (krb5_enctypes != NULL) {
		free(krb5_enctypes);
		krb5_enctypes = NULL;
		num_krb5_enctypes = 0;
	}

	/* count the number of commas */
	for (curr = enctypes; curr && *curr != '\0'; curr = ++comma) {
		comma = strchr(curr, ',');
		if (comma != NULL)
			n++;
		else
			break;
	}
	/* If no more commas and we're not at the end, there's one more value */
	if (*curr != '\0')
		n++;

	/* Empty string, return an error */
	if (n == 0)
		return ENOENT;

	/* Allocate space for enctypes array */
	if ((krb5_enctypes = (int *) calloc(n, sizeof(int))) == NULL) {
		return ENOMEM;
	}

	/* Now parse each value into the array */
	for (curr = enctypes, i = 0; curr && *curr != '\0'; curr = ++comma) {
		krb5_enctypes[i++] = atoi(curr);
		comma = strchr(curr, ',');
		if (comma == NULL)
			break;
	}

	num_krb5_enctypes = n;
	if ((cached_types = malloc(strlen(enctypes)+1)))
		strcpy(cached_types, enctypes);

	return 0;
}

static void
do_downcall(int k5_fd, uid_t uid, struct authgss_private_data *pd,
	    gss_buffer_desc *context_token, OM_uint32 lifetime_rec,
	    gss_buffer_desc *acceptor)
{
	char    *buf = NULL, *p = NULL, *end = NULL;
	unsigned int timeout = context_timeout;
	unsigned int buf_size = 0;

	printerr(2, "doing downcall: lifetime_rec=%u acceptor=%.*s\n",
		lifetime_rec, acceptor->length, acceptor->value);
	buf_size = sizeof(uid) + sizeof(timeout) + sizeof(pd->pd_seq_win) +
		sizeof(pd->pd_ctx_hndl.length) + pd->pd_ctx_hndl.length +
		sizeof(context_token->length) + context_token->length +
		sizeof(acceptor->length) + acceptor->length;
	p = buf = malloc(buf_size);
	if (!buf)
		goto out_err;

	end = buf + buf_size;

	/* context_timeout set by -t option overrides context lifetime */
	if (timeout == 0)
		timeout = lifetime_rec;
	if (WRITE_BYTES(&p, end, uid)) goto out_err;
	if (WRITE_BYTES(&p, end, timeout)) goto out_err;
	if (WRITE_BYTES(&p, end, pd->pd_seq_win)) goto out_err;
	if (write_buffer(&p, end, &pd->pd_ctx_hndl)) goto out_err;
	if (write_buffer(&p, end, context_token)) goto out_err;
	if (write_buffer(&p, end, acceptor)) goto out_err;

	if (write(k5_fd, buf, p - buf) < p - buf) goto out_err;
	free(buf);
	return;
out_err:
	free(buf);
	printerr(1, "Failed to write downcall!\n");
	return;
}

static int
do_error_downcall(int k5_fd, uid_t uid, int err)
{
	char	buf[1024];
	char	*p = buf, *end = buf + 1024;
	unsigned int timeout = 0;
	int	zero = 0;

	printerr(2, "doing error downcall\n");

	if (WRITE_BYTES(&p, end, uid)) goto out_err;
	if (WRITE_BYTES(&p, end, timeout)) goto out_err;
	/* use seq_win = 0 to indicate an error: */
	if (WRITE_BYTES(&p, end, zero)) goto out_err;
	if (WRITE_BYTES(&p, end, err)) goto out_err;

	if (write(k5_fd, buf, p - buf) < p - buf) goto out_err;
	return 0;
out_err:
	printerr(1, "Failed to write error downcall!\n");
	return -1;
}

/*
 * If the port isn't already set, do an rpcbind query to the remote server
 * using the program and version and get the port.
 *
 * Newer kernels send the value of the port= mount option in the "info"
 * file for the upcall or '0' for NFSv2/3. For NFSv4 it sends the value
 * of the port= option or '2049'. The port field in a new sockaddr should
 * reflect the value that was sent by the kernel.
 */
static int
populate_port(struct sockaddr *sa, const socklen_t salen,
	      const rpcprog_t program, const rpcvers_t version,
	      const unsigned short protocol)
{
	struct sockaddr_in	*s4 = (struct sockaddr_in *) sa;
#ifdef IPV6_SUPPORTED
	struct sockaddr_in6	*s6 = (struct sockaddr_in6 *) sa;
#endif /* IPV6_SUPPORTED */
	unsigned short		port;

	/*
	 * Newer kernels send the port in the upcall. If we already have
	 * the port, there's no need to look it up.
	 */
	switch (sa->sa_family) {
	case AF_INET:
		if (s4->sin_port != 0) {
			printerr(2, "DEBUG: port already set to %d\n",
				 ntohs(s4->sin_port));
			return 1;
		}
		break;
#ifdef IPV6_SUPPORTED
	case AF_INET6:
		if (s6->sin6_port != 0) {
			printerr(2, "DEBUG: port already set to %d\n",
				 ntohs(s6->sin6_port));
			return 1;
		}
		break;
#endif /* IPV6_SUPPORTED */
	default:
		printerr(0, "ERROR: unsupported address family %d\n",
			    sa->sa_family);
		return 0;
	}

	/*
	 * Newer kernels that send the port in the upcall set the value to
	 * 2049 for NFSv4 mounts when one isn't specified. The check below is
	 * only for kernels that don't send the port in the upcall. For those
	 * we either have to do an rpcbind query or set it to the standard
	 * port. Doing a query could be problematic (firewalls, etc), so take
	 * the latter approach.
	 */
	if (program == 100003 && version == 4) {
		port = 2049;
		goto set_port;
	}

	port = nfs_getport(sa, salen, program, version, protocol);
	if (!port) {
		printerr(0, "ERROR: unable to obtain port for prog %ld "
			    "vers %ld\n", program, version);
		return 0;
	}

set_port:
	printerr(2, "DEBUG: setting port to %hu for prog %lu vers %lu\n", port,
		 program, version);

	switch (sa->sa_family) {
	case AF_INET:
		s4->sin_port = htons(port);
		break;
#ifdef IPV6_SUPPORTED
	case AF_INET6:
		s6->sin6_port = htons(port);
		break;
#endif /* IPV6_SUPPORTED */
	}

	return 1;
}

/*
 * Create an RPC connection and establish an authenticated
 * gss context with a server.
 */
static int
create_auth_rpc_client(struct clnt_info *clp,
		       char *tgtname,
		       CLIENT **clnt_return,
		       AUTH **auth_return,
		       uid_t uid,
		       int authtype,
		       gss_cred_id_t cred)
{
	CLIENT			*rpc_clnt = NULL;
	struct rpc_gss_sec	sec;
	AUTH			*auth = NULL;
	int			retval = -1;
	OM_uint32		min_stat;
	char			rpc_errmsg[1024];
	int			protocol;
	struct timeval	timeout;
	struct sockaddr		*addr = (struct sockaddr *) &clp->addr;
	socklen_t		salen;

	sec.qop = GSS_C_QOP_DEFAULT;
	sec.svc = RPCSEC_GSS_SVC_NONE;
	sec.cred = cred;
	sec.req_flags = 0;
	if (authtype == AUTHTYPE_KRB5) {
		sec.mech = (gss_OID)&krb5oid;
		sec.req_flags = GSS_C_MUTUAL_FLAG;
	}
	else {
		printerr(0, "ERROR: Invalid authentication type (%d) "
			"in create_auth_rpc_client\n", authtype);
		goto out_fail;
	}


	if (authtype == AUTHTYPE_KRB5) {
#ifdef HAVE_SET_ALLOWABLE_ENCTYPES
		/*
		 * Do this before creating rpc connection since we won't need
		 * rpc connection if it fails!
		 */
		if (limit_krb5_enctypes(&sec)) {
			printerr(1, "WARNING: Failed while limiting krb5 "
				    "encryption types for user with uid %d\n",
				 uid);
			goto out_fail;
		}
#endif
	}

	/* create an rpc connection to the nfs server */

	printerr(2, "creating %s client for server %s\n", clp->protocol,
			clp->servername);

	protocol = IPPROTO_TCP;
	if ((strcmp(clp->protocol, "udp")) == 0)
		protocol = IPPROTO_UDP;

	switch (addr->sa_family) {
	case AF_INET:
		salen = sizeof(struct sockaddr_in);
		break;
#ifdef IPV6_SUPPORTED
	case AF_INET6:
		salen = sizeof(struct sockaddr_in6);
		break;
#endif /* IPV6_SUPPORTED */
	default:
		printerr(1, "ERROR: Unknown address family %d\n",
			 addr->sa_family);
		goto out_fail;
	}

	if (!populate_port(addr, salen, clp->prog, clp->vers, protocol))
		goto out_fail;

	/* set the timeout according to the requested valued */
	timeout.tv_sec = (long) rpc_timeout;
	timeout.tv_usec = (long) 0;

	rpc_clnt = nfs_get_rpcclient(addr, salen, protocol, clp->prog,
				     clp->vers, &timeout);
	if (!rpc_clnt) {
		snprintf(rpc_errmsg, sizeof(rpc_errmsg),
			 "WARNING: can't create %s rpc_clnt to server %s for "
			 "user with uid %d",
			 protocol == IPPROTO_TCP ? "tcp" : "udp",
			 clp->servername, uid);
		printerr(0, "%s\n",
			 clnt_spcreateerror(rpc_errmsg));
		goto out_fail;
	}
	if (!tgtname)
		tgtname = clp->servicename;

	printerr(2, "creating context with server %s\n", tgtname);
	auth = authgss_create_default(rpc_clnt, tgtname, &sec);
	if (!auth) {
		/* Our caller should print appropriate message */
		printerr(2, "WARNING: Failed to create krb5 context for "
			    "user with uid %d for server %s\n",
			 uid, tgtname);
		goto out_fail;
	}

	/* Success !!! */
	rpc_clnt->cl_auth = auth;
	*clnt_return = rpc_clnt;
	*auth_return = auth;
	retval = 0;

  out:
	if (sec.cred != GSS_C_NO_CREDENTIAL)
		gss_release_cred(&min_stat, &sec.cred);
	return retval;

  out_fail:
	/* Only destroy here if failure.  Otherwise, caller is responsible */
	if (rpc_clnt) clnt_destroy(rpc_clnt);

	goto out;
}

/*
 * Create the context as the user (not as root).
 *
 * Note that we change the *real* uid here, as changing the effective uid is
 * not sufficient. This is due to an unfortunate historical error in the MIT
 * krb5 libs, where they used %{uid} in the default_ccache_name. Changing that
 * now might break some applications so we're sort of stuck with it.
 *
 * Unfortunately, doing this leaves the forked child vulnerable to signals and
 * renicing, but this is the best we can do. In the event that a child is
 * signalled before downcalling, the kernel will just eventually time out the
 * upcall attempt.
 */
static int
change_identity(uid_t uid)
{
	struct passwd	*pw;

	/* drop list of supplimentary groups first */
	if (syscall(SYS_setgroups, 0, 0) != 0) {
		printerr(0, "WARNING: unable to drop supplimentary groups!");
		return errno;
	}

	/* try to get pwent for user */
	pw = getpwuid(uid);
	if (!pw) {
		/* if that doesn't work, try to get one for "nobody" */
		errno = 0;
		pw = getpwnam("nobody");
		if (!pw) {
			printerr(0, "WARNING: unable to determine gid for uid %u\n", uid);
			return errno ? errno : ENOENT;
		}
	}

	/* Switch the UIDs and GIDs. */
	/* For the threaded version we have to set uid,gid per thread instead
	 * of per process. glibc setresuid() when called from a thread, it'll
	 * send a signal to all other threads to synchronize the uid in all
	 * other threads. To bypass this, we have to call syscall() directly.
	 */
	if (syscall(SYS_setresgid, pw->pw_gid, pw->pw_gid, pw->pw_gid) != 0) {
		printerr(0, "WARNING: failed to set gid to %u!\n", pw->pw_gid);
		return errno;
	}

	if (syscall(SYS_setresuid, uid, uid, uid) != 0) {
		printerr(0, "WARNING: Failed to setuid for user with uid %u\n",
				uid);
		return errno;
	}

	return 0;
}

AUTH *
krb5_not_machine_creds(struct clnt_info *clp, uid_t uid, char *tgtname,
			int *downcall_err, int *chg_err, CLIENT **rpc_clnt)
{
	AUTH		*auth = NULL;
	gss_cred_id_t	gss_cred;
	char		**dname;
	int		err, resp = -1;

	printerr(2, "krb5_not_machine_creds: uid %d tgtname %s\n", 
		uid, tgtname);

	*chg_err = change_identity(uid);
	if (*chg_err) {
		printerr(0, "WARNING: failed to change identity: %s",
			strerror(*chg_err));
		goto out;
	}

	/** Tell krb5 gss which credentials cache to use.
	 * Try first to acquire credentials directly via GSSAPI
	 */
	err = gssd_acquire_user_cred(&gss_cred);
	if (err == 0)
		resp = create_auth_rpc_client(clp, tgtname, rpc_clnt,
						&auth, uid,
						AUTHTYPE_KRB5, gss_cred);

	/** if create_auth_rplc_client fails try the traditional
	 * method of trolling for credentials
	 */
	for (dname = ccachesearch; resp != 0 && *dname != NULL; dname++) {
		err = gssd_setup_krb5_user_gss_ccache(uid, clp->servername,
						*dname);
		if (err == -EKEYEXPIRED)
			*downcall_err = -EKEYEXPIRED;
		else if (err == 0)
			resp = create_auth_rpc_client(clp, tgtname, rpc_clnt,
						&auth, uid,AUTHTYPE_KRB5,
						GSS_C_NO_CREDENTIAL);
	}

out:
	return auth;
}

AUTH *
krb5_use_machine_creds(struct clnt_info *clp, uid_t uid, char *tgtname,
		    char *service, CLIENT **rpc_clnt)
{
	AUTH	*auth = NULL;
	char	**credlist = NULL;
	char	**ccname;
	int	nocache = 0;
	int	success = 0;

	printerr(2, "krb5_use_machine_creds: uid %d tgtname %s\n", 
		uid, tgtname);

	do {
		gssd_refresh_krb5_machine_credential(clp->servername, NULL,
						service);
	/*
	 * Get a list of credential cache names and try each
	 * of them until one works or we've tried them all
	 */
		if (gssd_get_krb5_machine_cred_list(&credlist)) {
			printerr(0, "ERROR: No credentials found "
				"for connection to server %s\n",
				clp->servername);
			goto out;
		}
		for (ccname = credlist; ccname && *ccname; ccname++) {
			u_int min_stat;

			if (gss_krb5_ccache_name(&min_stat, *ccname, NULL) !=
					GSS_S_COMPLETE) {
				printerr(1, "WARNING: gss_krb5_ccache_name "
					 "with name '%s' failed (%s)\n",
					 *ccname, error_message(min_stat));
				continue;
			}
			if ((create_auth_rpc_client(clp, tgtname, rpc_clnt,
						&auth, uid,
						AUTHTYPE_KRB5,
						GSS_C_NO_CREDENTIAL)) == 0) {
				/* Success! */
				success++;
				break;
			}
			printerr(2, "WARNING: Failed to create machine krb5 "
				"context with cred cache %s for server %s\n",
				*ccname, clp->servername);
		}
		gssd_free_krb5_machine_cred_list(credlist);
		if (!success) {
			if(nocache == 0) {
				nocache++;
				printerr(2, "WARNING: Machine cache prematurely "
					"expired or corrupted trying to "
					"recreate cache for server %s\n",
					clp->servername);
			} else {
				printerr(1, "ERROR: Failed to create machine "
					 "krb5 context with any credentials "
					 "cache for server %s\n",
					clp->servername);
				goto out;
			}
		}
	} while(!success);

out:
	return auth;
}

/*
 * this code uses the userland rpcsec gss library to create a krb5
 * context on behalf of the kernel
 */
static void
process_krb5_upcall(struct clnt_info *clp, uid_t uid, int fd, char *tgtname,
		    char *service)
{
	CLIENT			*rpc_clnt = NULL;
	AUTH			*auth = NULL;
	struct authgss_private_data pd;
	gss_buffer_desc		token;
	int			err, downcall_err = -EACCES;
	OM_uint32		maj_stat, min_stat, lifetime_rec;
	gss_name_t		gacceptor = GSS_C_NO_NAME;
	gss_OID			mech;
	gss_buffer_desc		acceptor  = {0};

	token.length = 0;
	token.value = NULL;
	memset(&pd, 0, sizeof(struct authgss_private_data));

	/*
	 * If "service" is specified, then the kernel is indicating that
	 * we must use machine credentials for this request.  (Regardless
	 * of the uid value or the setting of root_uses_machine_creds.)
	 * If the service value is "*", then any service name can be used.
	 * Otherwise, it specifies the service name that should be used.
	 * (For now, the values of service will only be "*" or "nfs".)
	 *
	 * Restricting gssd to use "nfs" service name is needed for when
	 * the NFS server is doing a callback to the NFS client.  In this
	 * case, the NFS server has to authenticate itself as "nfs" --
	 * even if there are other service keys such as "host" or "root"
	 * in the keytab.
	 *
	 * Another case when the kernel may specify the service attribute
	 * is when gssd is being asked to create the context for a
	 * SETCLIENT_ID operation.  In this case, machine credentials
	 * must be used for the authentication.  However, the service name
	 * used for this case is not important.
	 *
	 */
	if (uid != 0 || (uid == 0 && root_uses_machine_creds == 0 &&
				service == NULL)) {

		auth = krb5_not_machine_creds(clp, uid, tgtname, &downcall_err,
						&err, &rpc_clnt);
		if (err)
			goto out_return_error;
	}
	if (auth == NULL) {
		if (uid == 0 && (root_uses_machine_creds == 1 ||
				service != NULL)) {
			auth =	krb5_use_machine_creds(clp, uid, tgtname,
							service, &rpc_clnt);
			if (auth == NULL)
				goto out_return_error;
		} else {
			/* krb5_not_machine_creds logs the error */
			goto out_return_error;
		}
	}

	if (!authgss_get_private_data(auth, &pd)) {
		printerr(1, "WARNING: Failed to obtain authentication "
			    "data for user with uid %d for server %s\n",
			 uid, clp->servername);
		goto out_return_error;
	}

	/* Grab the context lifetime and acceptor name out of the ctx. */
	maj_stat = gss_inquire_context(&min_stat, pd.pd_ctx, NULL, &gacceptor,
				       &lifetime_rec, &mech, NULL, NULL, NULL);

	if (maj_stat != GSS_S_COMPLETE) {
		printerr(1, "WARNING: Failed to inquire context "
			    "maj_stat (0x%x)\n", maj_stat);
		lifetime_rec = 0;
	} else {
		get_hostbased_client_buffer(gacceptor, mech, &acceptor);
		gss_release_name(&min_stat, &gacceptor);
	}

	/*
	 * The serialization can mean turning pd.pd_ctx into a lucid context. If
	 * that happens then the pd.pd_ctx will be unusable, so we must never
	 * try to use it after this point.
	 */
	if (serialize_context_for_kernel(&pd.pd_ctx, &token, &krb5oid, NULL)) {
		printerr(1, "WARNING: Failed to serialize krb5 context for "
			    "user with uid %d for server %s\n",
			 uid, clp->servername);
		goto out_return_error;
	}

	do_downcall(fd, uid, &pd, &token, lifetime_rec, &acceptor);

out:
	gss_release_buffer(&min_stat, &acceptor);
	if (token.value)
		free(token.value);
#ifdef HAVE_AUTHGSS_FREE_PRIVATE_DATA
	if (pd.pd_ctx_hndl.length != 0 || pd.pd_ctx != 0)
		authgss_free_private_data(&pd);
#endif
	if (auth)
		AUTH_DESTROY(auth);
	if (rpc_clnt)
		clnt_destroy(rpc_clnt);

	return;

out_return_error:
	do_error_downcall(fd, uid, downcall_err);
	goto out;
}

void
handle_krb5_upcall(struct clnt_upcall_info *info)
{
	struct clnt_info *clp = info->clp;

	printerr(2, "\n%s: uid %d (%s)\n", __func__, info->uid, clp->relpath);

	process_krb5_upcall(clp, info->uid, clp->krb5_fd, NULL, NULL);
	free(info);
}

void
handle_gssd_upcall(struct clnt_upcall_info *info)
{
	struct clnt_info	*clp = info->clp;
	uid_t			uid;
	char			*p;
	char			*mech = NULL;
	char			*uidstr = NULL;
	char			*target = NULL;
	char			*service = NULL;
	char			*enctypes = NULL;

	printerr(2, "\n%s: '%s' (%s)\n", __func__, info->lbuf, clp->relpath);

	for (p = strtok(info->lbuf, " "); p; p = strtok(NULL, " ")) {
		if (!strncmp(p, "mech=", strlen("mech=")))
			mech = p + strlen("mech=");
		else if (!strncmp(p, "uid=", strlen("uid=")))
			uidstr = p + strlen("uid=");
		else if (!strncmp(p, "enctypes=", strlen("enctypes=")))
			enctypes = p + strlen("enctypes=");
		else if (!strncmp(p, "target=", strlen("target=")))
			target = p + strlen("target=");
		else if (!strncmp(p, "service=", strlen("service=")))
			service = p + strlen("service=");
	}

	if (!mech || strlen(mech) < 1) {
		printerr(0, "WARNING: handle_gssd_upcall: "
			    "failed to find gss mechanism name "
			    "in upcall string '%s'\n", info->lbuf);
		goto out;
	}

	if (uidstr) {
		uid = (uid_t)strtol(uidstr, &p, 10);
		if (p == uidstr || *p != '\0')
			uidstr = NULL;
	}

	if (!uidstr) {
		printerr(0, "WARNING: handle_gssd_upcall: "
			    "failed to find uid "
			    "in upcall string '%s'\n", info->lbuf);
		goto out;
	}

	if (enctypes && parse_enctypes(enctypes) != 0) {
		printerr(0, "WARNING: handle_gssd_upcall: "
			 "parsing encryption types failed: errno %d\n", errno);
		goto out;
	}

	if (target && strlen(target) < 1) {
		printerr(0, "WARNING: handle_gssd_upcall: "
			 "failed to parse target name "
			 "in upcall string '%s'\n", info->lbuf);
		goto out;
	}

	/*
	 * The presence of attribute "service=" indicates that machine
	 * credentials should be used for this request.  If the value
	 * is "*", then any machine credentials available can be used.
	 * If the value is anything else, then machine credentials for
	 * the specified service name (always "nfs" for now) should be
	 * used.
	 */
	if (service && strlen(service) < 1) {
		printerr(0, "WARNING: handle_gssd_upcall: "
			 "failed to parse service type "
			 "in upcall string '%s'\n", info->lbuf);
		goto out;
	}

	if (strcmp(mech, "krb5") == 0 && clp->servername)
		process_krb5_upcall(clp, uid, clp->gssd_fd, target, service);
	else {
		if (clp->servername)
			printerr(0, "WARNING: handle_gssd_upcall: "
				 "received unknown gss mech '%s'\n", mech);
		do_error_downcall(clp->gssd_fd, uid, -EACCES);
	}
out:
	free(info);
	return;
}

