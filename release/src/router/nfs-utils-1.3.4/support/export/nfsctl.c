/*
 * support/export/nfsctl.c
 *
 * Communicate export information to knfsd.
 *
 * Copyright (C) 1995 Olaf Kirch <okir@monad.swb.de>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include "nfslib.h"
#include "exportfs.h"
#include "xio.h"

static int	expsetup(struct nfsctl_export *exparg, nfs_export *exp, int unexport);
static int	cltsetup(struct nfsctl_client *cltarg, nfs_client *clp);

int
export_export(nfs_export *exp)
{
	nfs_client *	clp = exp->m_client;
	struct nfsctl_export	exparg;
	struct nfsctl_client	cltarg;

	if (!clp->m_exported && (clp->m_type != MCL_GSS)) {
		if (!cltsetup(&cltarg, clp))
			return 0;
		if (nfsaddclient(&cltarg) < 0)
			return 0;
		clp->m_exported = 1;
	}
	if (!expsetup(&exparg, exp, 0))
		return 0;
	if (nfsexport(&exparg) < 0)
		return 0;
	exp->m_exported = 1;
	return 1;
}

int
export_unexport(nfs_export *exp)
{
	struct nfsctl_export	exparg;

	if (!expsetup(&exparg, exp, 1) || nfsunexport(&exparg) < 0)
		return 0;
	exp->m_exported = 0;
	return 1;
}

static void
str_tolower(char *s)
{
	for ( ; *s; s++)
		if (isupper(*s))
			*s = tolower(*s);
}

static int
cltsetup(struct nfsctl_client *cltarg, nfs_client *clp)
{
	int i, j;

	if (clp->m_type != MCL_FQDN) {
		xlog(L_ERROR, "internal: can't export non-FQDN host");
		return 0;
	}
	memset(cltarg, 0, sizeof(*cltarg));
	strncpy(cltarg->cl_ident, clp->m_hostname,
		sizeof (cltarg->cl_ident) - 1);
	str_tolower(cltarg->cl_ident);

	j = 0;
	for (i = 0; i < clp->m_naddr && i < NFSCLNT_ADDRMAX; i++) {
		const struct sockaddr_in *sin = get_addrlist_in(clp, i);
		if (sin->sin_family == AF_INET)
			cltarg->cl_addrlist[j++] = sin->sin_addr;
	}
	if (j == 0) {
		xlog(L_ERROR, "internal: no supported addresses in nfs_client");
		return 0;
	}

	cltarg->cl_naddr = j;
	return 1;
}

static int
expsetup(struct nfsctl_export *exparg, nfs_export *exp, int unexport)
{
	nfs_client		*clp = exp->m_client;
	struct stat		stb;

	if (stat(exp->m_export.e_path, &stb) < 0)
		return 0;

	memset(exparg, 0, sizeof(*exparg));
	strncpy(exparg->ex_path, exp->m_export.e_path,
		sizeof (exparg->ex_path) - 1);
	strncpy(exparg->ex_client, clp->m_hostname,
		sizeof (exparg->ex_client) - 1);
	str_tolower(exparg->ex_client);
	exparg->ex_flags    = exp->m_export.e_flags;
	exparg->ex_dev      = (!unexport && (exp->m_export.e_flags & NFSEXP_FSID)) ?
			(__nfsd_dev_t)exp->m_export.e_fsid : stb.st_dev;
	exparg->ex_ino      = stb.st_ino;
	exparg->ex_anon_uid = exp->m_export.e_anonuid;
	exparg->ex_anon_gid = exp->m_export.e_anongid;

	return 1;
}
