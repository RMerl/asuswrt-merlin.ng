/* Copyright 2017 Jozsef Kadlecsik (kadlec@netfilter.org)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef LIBIPSET_ARGS_H
#define LIBIPSET_ARGS_H

/* Keywords */
enum ipset_keywords {
	IPSET_ARG_NONE = 0,
	/* Family and aliases */
	IPSET_ARG_FAMILY,			/* family */
	IPSET_ARG_INET,				/* -4 */
	IPSET_ARG_INET6,			/* -6 */
	/* Hash types */
	IPSET_ARG_HASHSIZE,			/* hashsize */
	IPSET_ARG_MAXELEM,			/* maxelem */
	/* Ignored options: backward compatibilty */
	IPSET_ARG_PROBES,			/* probes */
	IPSET_ARG_RESIZE,			/* resize */
	IPSET_ARG_GC,				/* gc */
	IPSET_ARG_IGNORED_FROM,			/* from */
	IPSET_ARG_IGNORED_TO,			/* to */
	IPSET_ARG_IGNORED_NETWORK,		/* network */
	/* List type */
	IPSET_ARG_SIZE,				/* size */
	/* IP-type elements */
	IPSET_ARG_IPRANGE,			/* range */
	IPSET_ARG_NETMASK,			/* netmask */
	/* Port-type elements */
	IPSET_ARG_PORTRANGE,			/* range */
	/* Setname type elements */
	IPSET_ARG_BEFORE,			/* before */
	IPSET_ARG_AFTER,			/* after */
	/* Backward compatibility */
	IPSET_ARG_FROM_IP,			/* from */
	IPSET_ARG_TO_IP,			/* to */
	IPSET_ARG_NETWORK,			/* network */
	IPSET_ARG_FROM_PORT,			/* from */
	IPSET_ARG_TO_PORT,			/* to */
	/* Extra flags, options */
	IPSET_ARG_FORCEADD,			/* forceadd */
	IPSET_ARG_MARKMASK,			/* markmask */
	IPSET_ARG_NOMATCH,			/* nomatch */
	IPSET_ARG_IFACE_WILDCARD,		/* interface wildcard match */
	/* Extensions */
	IPSET_ARG_TIMEOUT,			/* timeout */
	IPSET_ARG_COUNTERS,			/* counters */
	IPSET_ARG_PACKETS,			/* packets */
	IPSET_ARG_BYTES,			/* bytes */
	IPSET_ARG_COMMENT,			/* comment */
	IPSET_ARG_ADT_COMMENT,			/* comment */
	IPSET_ARG_SKBINFO,			/* skbinfo */
	IPSET_ARG_SKBMARK,			/* skbmark */
	IPSET_ARG_SKBPRIO,			/* skbprio */
	IPSET_ARG_SKBQUEUE,			/* skbqueue */
	IPSET_ARG_MAX,
};

#ifdef __cplusplus
extern "C" {
#endif

extern const struct ipset_arg * ipset_keyword(enum ipset_keywords i);
extern const char * ipset_ignored_optname(unsigned int opt);
#ifdef __cplusplus
}
#endif

#endif /* LIBIPSET_ARGS_H */
