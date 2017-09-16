/*
 * support/export/export.c
 *
 * Maintain list of exported file systems.
 *
 * Copyright (C) 1995, 1996 Olaf Kirch <okir@monad.swb.de>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <sys/types.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <stdlib.h>
#include "xmalloc.h"
#include "nfslib.h"
#include "exportfs.h"

exp_hash_table exportlist[MCL_MAXTYPES] = {{NULL, {{NULL,NULL}, }}, }; 
static int export_hash(char *);

static void	export_init(nfs_export *exp, nfs_client *clp,
					struct exportent *nep);
static void	export_add(nfs_export *exp);
static int	export_check(const nfs_export *exp, const struct addrinfo *ai,
				const char *path);
static nfs_export *
		export_allowed_internal(const struct addrinfo *ai,
				const char *path);

void
exportent_release(struct exportent *eep)
{
	xfree(eep->e_squids);
	xfree(eep->e_sqgids);
	free(eep->e_mountpoint);
	free(eep->e_fslocdata);
	free(eep->e_uuid);
	xfree(eep->e_hostname);
}

static void
export_free(nfs_export *exp)
{
	exportent_release(&exp->m_export);
	xfree(exp);
}

static void warn_duplicated_exports(nfs_export *exp, struct exportent *eep)
{
	if (exp->m_export.e_flags != eep->e_flags) {
		xlog(L_ERROR, "incompatible duplicated export entries:");
		xlog(L_ERROR, "\t%s:%s (0x%x) [IGNORED]", eep->e_hostname,
				eep->e_path, eep->e_flags);
		xlog(L_ERROR, "\t%s:%s (0x%x)", exp->m_export.e_hostname,
				exp->m_export.e_path, exp->m_export.e_flags);
	} else {
		xlog(L_ERROR, "duplicated export entries:");
		xlog(L_ERROR, "\t%s:%s", eep->e_hostname, eep->e_path);
		xlog(L_ERROR, "\t%s:%s", exp->m_export.e_hostname,
				exp->m_export.e_path);
	}
}

/**
 * export_read - read entries from /etc/exports
 * @fname: name of file to read from
 *
 * Returns number of read entries.
 */
int
export_read(char *fname)
{
	struct exportent	*eep;
	nfs_export		*exp;

	int volumes = 0;

	setexportent(fname, "r");
	while ((eep = getexportent(0,1)) != NULL) {
		exp = export_lookup(eep->e_hostname, eep->e_path, 0);
		if (!exp) {
			if (export_create(eep, 0))
				/* possible complaints already logged */
				volumes++;
		}
		else
			warn_duplicated_exports(exp, eep);
	}
	endexportent();

	return volumes;
}

/**
 * export_create - create an in-core nfs_export record from an export entry
 * @xep: export entry to lookup
 * @canonical: if set, e_hostname is known to be canonical DNS name
 *
 * Returns a freshly instantiated export record, or NULL if
 * a problem occurred.
 */
nfs_export *
export_create(struct exportent *xep, int canonical)
{
	nfs_client	*clp;
	nfs_export	*exp;

	if (!(clp = client_lookup(xep->e_hostname, canonical))) {
		/* bad export entry; complaint already logged */
		return NULL;
	}
	exp = (nfs_export *) xmalloc(sizeof(*exp));
	export_init(exp, clp, xep);
	export_add(exp);

	return exp;
}

static void
export_init(nfs_export *exp, nfs_client *clp, struct exportent *nep)
{
	struct exportent	*e = &exp->m_export;

	dupexportent(e, nep);
	if (nep->e_hostname)
		e->e_hostname = xstrdup(nep->e_hostname);

	exp->m_exported = 0;
	exp->m_xtabent = 0;
	exp->m_mayexport = 0;
	exp->m_changed = 0;
	exp->m_warned = 0;
	exp->m_client = clp;
	clp->m_count++;
}

/*
 * Duplicate exports data. The in-core export struct retains the
 * original hostname from /etc/exports, while the in-core client struct
 * gets the newly found FQDN.
 */
static nfs_export *
export_dup(nfs_export *exp, const struct addrinfo *ai)
{
	nfs_export		*new;
	nfs_client		*clp;

	new = (nfs_export *) xmalloc(sizeof(*new));
	memcpy(new, exp, sizeof(*new));
	dupexportent(&new->m_export, &exp->m_export);
	if (exp->m_export.e_hostname)
		new->m_export.e_hostname = xstrdup(exp->m_export.e_hostname);
	clp = client_dup(exp->m_client, ai);
	if (clp == NULL) {
		export_free(new);
		return NULL;
	}
	clp->m_count++;
	new->m_client = clp;
	new->m_mayexport = exp->m_mayexport;
	new->m_exported = 0;
	new->m_xtabent = 0;
	new->m_changed = 0;
	new->m_warned = 0;
	export_add(new);

	return new;
}

static void
export_add(nfs_export *exp)
{
	exp_hash_table *p_tbl;
	exp_hash_entry *p_hen;
	nfs_export *p_next;

	int type = exp->m_client->m_type;
	int pos;

	pos = export_hash(exp->m_export.e_path);
	p_tbl = &(exportlist[type]); /* pointer to hash table */
	p_hen = &(p_tbl->entries[pos]); /* pointer to hash table entry */

	if (!(p_hen->p_first)) { /* hash table entry is empty */ 
 		p_hen->p_first = exp;
 		p_hen->p_last  = exp;

 		exp->m_next = p_tbl->p_head;
 		p_tbl->p_head = exp;
	} else { /* hash table entry is NOT empty */
		p_next = p_hen->p_last->m_next;
		p_hen->p_last->m_next = exp;
		exp->m_next = p_next;
		p_hen->p_last = exp;
	}
}

/**
 * export_find - find or create a suitable nfs_export for @ai and @path
 * @ai: pointer to addrinfo for client
 * @path: '\0'-terminated ASCII string containing export path
 *
 * Returns a pointer to nfs_export data matching @ai and @path,
 * or NULL if an error occurs.
 */
nfs_export *
export_find(const struct addrinfo *ai, const char *path)
{
	nfs_export	*exp;
	int		i;

	for (i = 0; i < MCL_MAXTYPES; i++) {
		for (exp = exportlist[i].p_head; exp; exp = exp->m_next) {
			if (!export_check(exp, ai, path))
				continue;
			if (exp->m_client->m_type == MCL_FQDN)
				return exp;
			return export_dup(exp, ai);
		}
	}

	return NULL;
}

static nfs_export *
export_allowed_internal(const struct addrinfo *ai, const char *path)
{
	nfs_export	*exp;
	int		i;

	for (i = 0; i < MCL_MAXTYPES; i++) {
		for (exp = exportlist[i].p_head; exp; exp = exp->m_next) {
			if (!exp->m_mayexport ||
			    !export_check(exp, ai, path))
				continue;
			return exp;
		}
	}

	return NULL;
}

/**
 * export_allowed - determine if this export is allowed
 * @ai: pointer to addrinfo for client
 * @path: '\0'-terminated ASCII string containing export path
 *
 * Returns a pointer to nfs_export data matching @ai and @path,
 * or NULL if the export is not allowed.
 */
nfs_export *
export_allowed(const struct addrinfo *ai, const char *path)
{
	nfs_export		*exp;
	char			epath[MAXPATHLEN+1];
	char			*p = NULL;

	if (path [0] != '/') return NULL;

	strncpy(epath, path, sizeof (epath) - 1);
	epath[sizeof (epath) - 1] = '\0';

	/* Try the longest matching exported pathname. */
	while (1) {
		exp = export_allowed_internal(ai, epath);
		if (exp)
			return exp;
		/* We have to treat the root, "/", specially. */
		if (p == &epath[1]) break;
		p = strrchr(epath, '/');
		if (p == epath) p++;
		*p = '\0';
	}

	return NULL;
}

/**
 * export_lookup - search hash table for export entry
 * @hname: '\0'-terminated ASCII string containing client hostname to look for
 * @path: '\0'-terminated ASCII string containing export path to look for
 * @canonical: if set, @hname is known to be canonical DNS name
 *
 * Returns a pointer to nfs_export record matching @hname and @path,
 * or NULL if the export was not found.
 */
nfs_export *
export_lookup(char *hname, char *path, int canonical)
{
	nfs_client *clp;
	nfs_export *exp;
	exp_hash_entry *p_hen;

	int pos;

	clp = client_lookup(hname, canonical);
	if(clp == NULL)
		return NULL;

	pos = export_hash(path);
	p_hen = &(exportlist[clp->m_type].entries[pos]); 
	for(exp = p_hen->p_first; exp && (exp != p_hen->p_last->m_next); 
  			exp = exp->m_next) {
		if (exp->m_client == clp && !strcmp(exp->m_export.e_path, path)) {
  			return exp;
		}
	}
	return NULL;
}

static int
export_check(const nfs_export *exp, const struct addrinfo *ai, const char *path)
{
	if (strcmp(path, exp->m_export.e_path))
		return 0;

	return client_check(exp->m_client, ai);
}

/**
 * export_freeall - deallocate all nfs_export records
 *
 */
void
export_freeall(void)
{
	nfs_export	*exp, *nxt;
	int		i, j;

	for (i = 0; i < MCL_MAXTYPES; i++) {
		for (exp = exportlist[i].p_head; exp; exp = nxt) {
			nxt = exp->m_next;
			client_release(exp->m_client);
			export_free(exp);
		}
		for (j = 0; j < HASH_TABLE_SIZE; j++) {
			exportlist[i].entries[j].p_first = NULL;
			exportlist[i].entries[j].p_last = NULL;
		}
		exportlist[i].p_head = NULL;
	}
	client_freeall();
}

/*
 * Compute and returns integer from string. 
 * Note: Its understood the smae integers can be same for 
 *       different strings, but it should not matter.
 */
static unsigned int 
strtoint(char *str)
{
	int i = 0;
	unsigned int n = 0;

	while ( str[i] != '\0') {
		n+=((int)str[i])*i;
		i++;
	}
	return n;
}

/*
 * Hash function
 */
static int 
export_hash(char *str)
{
	unsigned int num = strtoint(str);

	return num % HASH_TABLE_SIZE;
}
