/*
 * support/include/exportfs.h
 *
 * Declarations for exportfs and mountd
 *
 * Copyright (C) 1995, 1996 Olaf Kirch <okir@monad.swb.de>
 */

#ifndef EXPORTFS_H
#define EXPORTFS_H

#include <netdb.h>
#include <string.h>

#include "sockaddr.h"
#include "nfslib.h"

enum {
	MCL_FQDN = 0,
	MCL_SUBNETWORK,
	MCL_IPADDR = MCL_SUBNETWORK,
	MCL_WILDCARD,
	MCL_NETGROUP,
	MCL_ANONYMOUS,
	MCL_GSS,
	MCL_MAXTYPES
};

enum {
	FSLOC_NONE = 0,
	FSLOC_REFER,
	FSLOC_REPLICA,
	FSLOC_STUB
};

#ifndef EXP_LOCKFILE
#define EXP_LOCKFILE "/var/lib/nfs/export-lock"
#endif

typedef struct mclient {
	struct mclient *	m_next;
	char *			m_hostname;
	int			m_type;
	int			m_naddr;
	union nfs_sockaddr	m_addrlist[NFSCLNT_ADDRMAX];
	int			m_exported;	/* exported to nfsd */
	int			m_count;
} nfs_client;

static inline const struct sockaddr *
get_addrlist(const nfs_client *clp, const int i)
{
	return &clp->m_addrlist[i].sa;
}

static inline const struct sockaddr_in *
get_addrlist_in(const nfs_client *clp, const int i)
{
	return &clp->m_addrlist[i].s4;
}

static inline const struct sockaddr_in6 *
get_addrlist_in6(const nfs_client *clp, const int i)
{
	return &clp->m_addrlist[i].s6;
}

static inline void
set_addrlist_in(nfs_client *clp, const int i, const struct sockaddr_in *sin)
{
	memcpy(&clp->m_addrlist[i].s4, sin, sizeof(*sin));
}

static inline void
set_addrlist_in6(nfs_client *clp, const int i, const struct sockaddr_in6 *sin6)
{
	memcpy(&clp->m_addrlist[i].s6, sin6, sizeof(*sin6));
}

static inline void
set_addrlist(nfs_client *clp, const int i, const struct sockaddr *sap)
{
	switch (sap->sa_family) {
	case AF_INET:
		memcpy(&clp->m_addrlist[i].s4, sap, sizeof(struct sockaddr_in));
		break;
#ifdef IPV6_SUPPORTED
	case AF_INET6:
		memcpy(&clp->m_addrlist[i].s6, sap, sizeof(struct sockaddr_in6));
		break;
#endif
	}
}

typedef struct mexport {
	struct mexport *	m_next;
	struct mclient *	m_client;
	struct exportent	m_export;
	int			m_exported;	/* known to knfsd. -1 means not sure */
	int			m_xtabent  : 1,	/* xtab entry exists */
				m_mayexport: 1,	/* derived from xtabbed */
				m_changed  : 1, /* options (may) have changed */
				m_warned   : 1; /* warned about multiple exports
						 * matching one client */
} nfs_export;

#define HASH_TABLE_SIZE 1021
#define DEFAULT_TTL	(30 * 60)

typedef struct _exp_hash_entry {
	nfs_export * p_first;
  	nfs_export * p_last;
} exp_hash_entry;

typedef struct _exp_hash_table {
	nfs_export * p_head;
	exp_hash_entry entries[HASH_TABLE_SIZE];
} exp_hash_table;

extern exp_hash_table exportlist[MCL_MAXTYPES];

extern nfs_client *		clientlist[MCL_MAXTYPES];

nfs_client *			client_lookup(char *hname, int canonical);
nfs_client *			client_dup(const nfs_client *clp,
						const struct addrinfo *ai);
int				client_gettype(char *hname);
int				client_check(const nfs_client *clp,
						const struct addrinfo *ai);
void				client_release(nfs_client *);
void				client_freeall(void);
char *				client_compose(const struct addrinfo *ai);
struct addrinfo *		client_resolve(const struct sockaddr *sap);
int 				client_member(const char *client,
						const char *name);

int				export_read(char *fname);
void				export_reset(nfs_export *);
nfs_export *			export_lookup(char *hname, char *path, int caconical);
nfs_export *			export_find(const struct addrinfo *ai,
						const char *path);
nfs_export *			export_allowed(const struct addrinfo *ai,
						const char *path);
nfs_export *			export_create(struct exportent *, int canonical);
void				exportent_release(struct exportent *);
void				export_freeall(void);
int				export_export(nfs_export *);
int				export_unexport(nfs_export *);

int				xtab_mount_read(void);
int				xtab_export_read(void);
int				xtab_mount_write(void);
int				xtab_export_write(void);
void				xtab_append(nfs_export *);

int				secinfo_addflavor(struct flav_info *, struct exportent *);

char *				host_ntop(const struct sockaddr *sap,
						char *buf, const size_t buflen);
__attribute__((__malloc__))
struct addrinfo *		host_pton(const char *paddr);
__attribute__((__malloc__))
struct addrinfo *		host_addrinfo(const char *hostname);
__attribute__((__malloc__))
char *				host_canonname(const struct sockaddr *sap);
__attribute__((__malloc__))
struct addrinfo *		host_reliable_addrinfo(const struct sockaddr *sap);
__attribute__((__malloc__))
struct addrinfo *		host_numeric_addrinfo(const struct sockaddr *sap);

int				rmtab_read(void);

struct nfskey *			key_lookup(char *hname);

struct export_features {
	unsigned int flags;
	unsigned int secinfo_flags;
};

struct export_features *get_export_features(void);
void fix_pseudoflavor_flags(struct exportent *ep);

#endif /* EXPORTFS_H */
