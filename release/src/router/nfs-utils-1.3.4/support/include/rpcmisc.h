/*
 * rpcmisc	Support for RPC startup, dispatching and logging.
 *
 * Copyright (C) 1995 Olaf Kirch <okir@monad.swb.de>
 */

#ifndef RPCMISC_H
#define RPCMISC_H

#include <rpc/rpc.h>
#include <rpc/pmap_clnt.h>

#ifdef __STDC__
#   define CONCAT(a,b)		a##b
#   define STRING(a)		#a
#else
#   define CONCAT(a,b)		a/**/b
#   define STRING(a)		"a"
#endif

typedef bool_t	(*rpcsvc_fn_t)(struct svc_req *, void *argp, void *resp);

struct rpc_dentry {
	const char	*name;
	rpcsvc_fn_t	func;
	xdrproc_t	xdr_arg_fn;		/* argument XDR */
	size_t		xdr_arg_size;
	xdrproc_t	xdr_res_fn;		/* result XDR */
	size_t		xdr_res_size;
};

struct rpc_dtable {
	struct rpc_dentry *entries;
	rpcproc_t		nproc;
};

#define dtable_ent(func, vers, arg_type, res_type) \
	{	STRING(func), \
		(rpcsvc_fn_t)func##_##vers##_svc, \
		(xdrproc_t)xdr_##arg_type, sizeof(arg_type), \
		(xdrproc_t)xdr_##res_type, sizeof(res_type), \
	}

void		nfs_svc_unregister(const rpcprog_t program,
				const rpcvers_t version);
unsigned int	nfs_svc_create(char *name, const rpcprog_t program,
				const rpcvers_t version,
				void (*dispatch)(struct svc_req *, SVCXPRT *),
				const uint16_t port);
void		rpc_init(char *name, int prog, int vers,
				void (*dispatch)(struct svc_req *, SVCXPRT *),
				int defport);
void		rpc_dispatch(struct svc_req *rq, SVCXPRT *xprt,
				struct rpc_dtable *dtable, int nvers,
				void *argp, void *resp);
int		getservport(u_long number, const char *proto);

extern int	_rpcpmstart;
extern unsigned int	_rpcprotobits;
extern int	_rpcsvcdirty;

static inline struct sockaddr_in *nfs_getrpccaller_in(SVCXPRT *xprt)
{
	return (struct sockaddr_in *)(char *)svc_getcaller(xprt);
}

static inline struct sockaddr *nfs_getrpccaller(SVCXPRT *xprt)
{
	return (struct sockaddr *)(char *)svc_getcaller(xprt);
}

#endif /* RPCMISC_H */
