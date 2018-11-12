/* Portions of this file are subject to the following copyright(s).  See
 * the Net-SNMP's COPYING file for more details and other copyrights
 * that may apply:
 */
/*
 * Portions of this file are copyrighted by:
 * Copyright Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * Use is subject to license terms specified in the COPYING file
 * distributed with the Net-SNMP package.
 *
 * Portions of this file are copyrighted by:
 * Copyright (c) 2016 VMware, Inc. All rights reserved.
 * Use is subject to license terms specified in the COPYING file
 * distributed with the Net-SNMP package.
 */

#include <net-snmp/net-snmp-config.h>

#include <net-snmp/types.h>
#include <net-snmp/library/snmpUDPDomain.h>
#include <net-snmp/library/snmpUDPIPv4BaseDomain.h>

#include <stddef.h>
#include <stdio.h>
#include <sys/types.h>
#include <ctype.h>
#include <errno.h>

#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#if HAVE_NETDB_H
#include <netdb.h>
#endif
#if HAVE_SYS_UIO_H
#include <sys/uio.h>
#endif

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#include <net-snmp/types.h>
#include <net-snmp/output_api.h>
#include <net-snmp/config_api.h>

#include <net-snmp/library/snmp_transport.h>
#include <net-snmp/library/snmpSocketBaseDomain.h>
#include <net-snmp/library/system.h>
#include <net-snmp/library/tools.h>

#include "inet_ntop.h"
#include "inet_pton.h"

#ifndef INADDR_NONE
#define INADDR_NONE	-1
#endif

#ifndef INET_ADDRSTRLEN
#define INET_ADDRSTRLEN 16
#endif

static netsnmp_tdomain udpDomain;

/*
 * needs to be in sync with the definitions in snmplib/snmpTCPDomain.c 
 * and perl/agent/agent.xs 
 */
typedef netsnmp_indexed_addr_pair netsnmp_udp_addr_pair;

int
netsnmp_sockaddr_in2(struct sockaddr_in *addr,
                     const char *inpeername, const char *default_target);

/*
 * Return a string representing the address in data, or else the "far end"
 * address if data is NULL.  
 */

char *
netsnmp_udp_fmtaddr(netsnmp_transport *t, const void *data, int len)
{
    return netsnmp_ipv4_fmtaddr("UDP", t, data, len);
}


#if defined(HAVE_IP_PKTINFO) || (defined(HAVE_IP_RECVDSTADDR) && defined(HAVE_IP_SENDSRCADDR))

int netsnmp_udp_recvfrom(int s, void *buf, int len, struct sockaddr *from, socklen_t *fromlen, struct sockaddr *dstip, socklen_t *dstlen, int *if_index)
{
    /** udpipv4 just calls udpbase. should we skip directly to there? */
    return netsnmp_udpipv4_recvfrom(s, buf, len, from, fromlen, dstip, dstlen,
                                    if_index);
}

int netsnmp_udp_sendto(int fd, const struct in_addr *srcip, int if_index,
                       const struct sockaddr *remote, const void *data, int len)
{
    /** udpipv4 just calls udpbase. should we skip directly to there? */
    return netsnmp_udpipv4_sendto(fd, srcip, if_index, remote, data, len);
}
#endif /* HAVE_IP_PKTINFO || HAVE_IP_RECVDSTADDR */

/*
 * Common initialization of udp transport.
 */

static netsnmp_transport *
netsnmp_udp_transport_base(netsnmp_transport *t)
{
    if (NULL == t) {
        return NULL;
    }

    /*
     * Set Domain
     */

    t->domain = netsnmpUDPDomain;
    t->domain_length = netsnmpUDPDomain_len;

    /*
     * 16-bit length field, 8 byte UDP header, 20 byte IPv4 header  
     */

    t->msgMaxSize = 0xffff - 8 - 20;
    t->f_recv     = netsnmp_udpbase_recv;
    t->f_send     = netsnmp_udpbase_send;
    t->f_close    = netsnmp_socketbase_close;
    t->f_accept   = NULL;
    t->f_fmtaddr  = netsnmp_udp_fmtaddr;
    t->f_get_taddr = netsnmp_ipv4_get_taddr;

    return t;
}

/*
 * Open a UDP-based transport for SNMP.  Local is TRUE if addr is the local
 * address to bind to (i.e. this is a server-type session); otherwise addr is
 * the remote address to send things to.
 */
netsnmp_transport *
netsnmp_udp_transport(const struct sockaddr_in *addr, int local)
{
    netsnmp_transport *t = NULL;

    t = netsnmp_udpipv4base_transport(addr, local);
    if (NULL != t) {
        netsnmp_udp_transport_base(t);
    }
    return t;
}

/*
 * Open a UDP-based transport for SNMP.  Local is TRUE if addr is the local
 * address to bind to (i.e. this is a server-type session); otherwise addr is
 * the remote address to send things to and src_addr is the optional addr
 * to send from.
 */
netsnmp_transport *
netsnmp_udp_transport_with_source(const struct sockaddr_in *addr, int local,
                                  const struct sockaddr_in *src_addr)

{
    netsnmp_transport *t = NULL;

    t = netsnmp_udpipv4base_transport_with_source(addr, local, src_addr);
    if (NULL != t) {
        netsnmp_udp_transport_base(t);
    }
    return t;
}
#if !defined(NETSNMP_DISABLE_SNMPV1) || !defined(NETSNMP_DISABLE_SNMPV2C)
/*
 * The following functions provide the "com2sec" configuration token
 * functionality for compatibility.
 */

#define EXAMPLE_NETWORK		"NETWORK"
#define EXAMPLE_COMMUNITY	"COMMUNITY"

struct com2SecEntry_s {
    const char *secName;
    const char *contextName;
    struct com2SecEntry_s *next;
    in_addr_t   network;
    in_addr_t   mask;
    int         negate;
    const char  community[1];
};

static com2SecEntry   *com2SecList = NULL, *com2SecListLast = NULL;

int
netsnmp_udp_com2SecEntry_create(com2SecEntry **entryp, const char *community,
                    const char *secName, const char *contextName,
                    struct in_addr *network, struct in_addr *mask,
                    int negate)
{
    int communityLen, secNameLen, contextNameLen, len;
    com2SecEntry* e;
    char* last;
    struct in_addr dflt_network, dflt_mask;

    if (NULL != entryp)
        *entryp = NULL;

    if (NULL == community || NULL == secName)
        return C2SE_ERR_MISSING_ARG;

    if (NULL == network) {
        network = &dflt_network;
        dflt_network.s_addr = 0;
    }
    if (NULL == mask) {
        mask = &dflt_mask;
        dflt_mask.s_addr = 0;
    }

    /** Check that the network and mask are consistent. */
    if (network->s_addr & ~mask->s_addr)
        return C2SE_ERR_MASK_MISMATCH;

    communityLen = strlen(community);
    if (communityLen > COMMUNITY_MAX_LEN)
        return C2SE_ERR_COMMUNITY_TOO_LONG;

    secNameLen = strlen(secName);
    if (secNameLen > VACM_MAX_STRING)
        return C2SE_ERR_SECNAME_TOO_LONG;

    contextNameLen = contextName ? strlen(contextName) : 0;
    if (contextNameLen > VACM_MAX_STRING)
        return C2SE_ERR_CONTEXT_TOO_LONG;

    /** alloc space for struct + 3 strings with NULLs */
    len = offsetof(com2SecEntry, community) + communityLen + secNameLen +
        contextNameLen + 3;
    e = (com2SecEntry*)calloc(len, 1);
    if (e == NULL)
        return C2SE_ERR_MEMORY;
    last = ((char*)e) + offsetof(com2SecEntry, community);

    DEBUGIF("netsnmp_udp_parse_security") {
        char buf1[INET_ADDRSTRLEN];
        char buf2[INET_ADDRSTRLEN];
        DEBUGMSGTL(("netsnmp_udp_parse_security",
                    "<\"%s\", %s/%s> => \"%s\"\n", community,
                    inet_ntop(AF_INET, network, buf1, sizeof(buf1)),
                    inet_ntop(AF_INET, mask, buf2, sizeof(buf2)),
                    secName));
    }

    memcpy(last, community, communityLen);
    last += communityLen + 1;
    memcpy(last, secName, secNameLen);
    e->secName = last;
    last += secNameLen + 1;
    if (contextNameLen) {
        memcpy(last, contextName, contextNameLen);
        e->contextName = last;
    } else
        e->contextName = last - 1;
    e->network = network->s_addr;
    e->mask = mask->s_addr;
    e->negate = negate;
    e->next = NULL;

    if (com2SecListLast != NULL) {
        com2SecListLast->next = e;
        com2SecListLast = e;
    } else {
        com2SecListLast = com2SecList = e;
    }

    if (NULL != entryp)
        *entryp = e;

    return C2SE_ERR_SUCCESS;
}

void
netsnmp_udp_parse_security(const char *token, char *param)
{
    /** copy_nword does null term, so we need vars of max size + 2. */
    /** (one for null, one to detect param too long */
    char            secName[VACMSTRINGLEN]; /* == VACM_MAX_STRING + 2 */
    char            contextName[VACMSTRINGLEN];
    char            community[COMMUNITY_MAX_LEN + 2];
    char            source[271]; /* !(1)+dns-name(253)+/(1)+mask(15)+\0(1) */
    char            *sourcep;
    struct in_addr  network, mask;
    int             negate;
    int rc;

    /*
     * Get security, source address/netmask and community strings.
     */

    param = copy_nword( param, secName, sizeof(secName));
    if (strcmp(secName, "-Cn") == 0) {
        if (!param) {
            config_perror("missing CONTEXT_NAME parameter");
            return;
        }
        param = copy_nword( param, contextName, sizeof(contextName));
        if (!param) {
            config_perror("missing NAME parameter");
            return;
        }
        param = copy_nword( param, secName, sizeof(secName));
    } else
        contextName[0] = '\0';

    if (secName[0] == '\0') {
        config_perror("empty NAME parameter");
        return;
    }

    if (!param) {
        config_perror("missing SOURCE parameter");
        return;
    }
    param = copy_nword( param, source, sizeof(source));
    if (source[0] == '\0') {
        config_perror("empty SOURCE parameter");
        return;
    }
    if (strncmp(source, EXAMPLE_NETWORK, strlen(EXAMPLE_NETWORK)) == 0) {
        config_perror("example config NETWORK not properly configured");
        return;
    }

    if (!param) {
        config_perror("missing COMMUNITY parameter");
        return;
    }
    param = copy_nword( param, community, sizeof(community));
    if (community[0] == '\0') {
        config_perror("empty COMMUNITY parameter");
        return;
    }
    if ((strlen(community) + 1) == sizeof(EXAMPLE_COMMUNITY) &&
        memcmp(community, EXAMPLE_COMMUNITY, sizeof(EXAMPLE_COMMUNITY)) == 0) {
        config_perror("example config COMMUNITY not properly configured");
        return;
    }

    /* Deal with the "default" case first. */
    if (strcmp(source, "default") == 0) {
        network.s_addr = 0;
        mask.s_addr = 0;
        negate = 0;
    } else {
        if (*source == '!') {
            negate = 1;
            sourcep = source + 1;
        } else {
            negate = 0;
            sourcep = source;
        }

        /* Split the source/netmask parts */
        char *strmask = strchr(sourcep, '/');
        if (strmask != NULL)
            /* Mask given. */
            *strmask++ = '\0';

        /* Try interpreting as a dotted quad. */
        if (inet_pton(AF_INET, sourcep, &network) == 0) {
            /* Nope, wasn't a dotted quad.  Must be a hostname. */
            int ret = netsnmp_gethostbyname_v4(sourcep, &network.s_addr);
            if (ret < 0) {
                config_perror("cannot resolve source hostname");
                return;
            }
        }

        /* Now work out the mask. */
        if (strmask == NULL || *strmask == '\0') {
            /* No mask was given. Assume /32 */
            mask.s_addr = (in_addr_t)(~0UL);
        } else {
            /* Try to interpret mask as a "number of 1 bits". */
            char* cp;
            long maskLen = strtol(strmask, &cp, 10);
            if (*cp == '\0') {
                if (0 < maskLen && maskLen <= 32)
                    mask.s_addr = htonl((in_addr_t)(~0UL << (32 - maskLen)));
                else if (0 == maskLen)
                    mask.s_addr = 0;
                else {
                    config_perror("bad mask length");
                    return;
                }
            }
            /* Try to interpret mask as a dotted quad. */
            else if (inet_pton(AF_INET, strmask, &mask) == 0) {
                config_perror("bad mask");
                return;
            }

            /* Check that the network and mask are consistent. */
            if (network.s_addr & ~mask.s_addr) {
                config_perror("source/mask mismatch");
                return;
            }
        }
    }

    /*
     * Everything is okay.  Copy the parameters to the structure allocated
     * above and add it to END of the list.
     */
    rc = netsnmp_udp_com2SecEntry_create(NULL, community, secName, contextName,
                                         &network, &mask, negate);
    switch(rc) {
        case C2SE_ERR_SUCCESS:
            break;
        case C2SE_ERR_CONTEXT_TOO_LONG:
            config_perror("context name too long");
            break;
        case C2SE_ERR_COMMUNITY_TOO_LONG:
            config_perror("community name too long");
            break;
        case C2SE_ERR_SECNAME_TOO_LONG:
            config_perror("security name too long");
            break;
        case C2SE_ERR_MASK_MISMATCH:
            config_perror("source/mask mismatch");
            break;
        case C2SE_ERR_MISSING_ARG:
        default:
            config_perror("unexpected error; could not create com2SecEntry");
    }
}

void
netsnmp_udp_com2Sec_free(com2SecEntry *e)
{
    free(e);
}

int
netsnmp_udp_com2SecList_remove(com2SecEntry *e)
{
    com2SecEntry   *c = com2SecList, *p = NULL;
    for (; c != NULL; p = c, c = c->next) {
        if (e == c)
            break;
    }
    if (NULL == c)
        return 1;

    if (NULL == p)
        com2SecList = e->next;
    else
        p->next = e->next;
    e->next = NULL;

    if (e == com2SecListLast)
        com2SecListLast = p;

    return 0;
}

void
netsnmp_udp_com2SecList_free(void)
{
    com2SecEntry   *e = com2SecList;
    while (e != NULL) {
        com2SecEntry   *tmp = e;
        e = e->next;
        netsnmp_udp_com2Sec_free(tmp);
    }
    com2SecList = com2SecListLast = NULL;
}
#endif /* support for community based SNMP */

void
netsnmp_udp_agent_config_tokens_register(void)
{
#if !defined(NETSNMP_DISABLE_SNMPV1) || !defined(NETSNMP_DISABLE_SNMPV2C)
    register_app_config_handler("com2sec", netsnmp_udp_parse_security,
                                netsnmp_udp_com2SecList_free,
                                "[-Cn CONTEXT] secName IPv4-network-address[/netmask] community");
#endif /* support for community based SNMP */
}



/*
 * Return 0 if there are no com2sec entries, or return 1 if there ARE com2sec
 * entries.  On return, if a com2sec entry matched the passed parameters,
 * then *secName points at the appropriate security name, or is NULL if the
 * parameters did not match any com2sec entry.
 */

#if !defined(NETSNMP_DISABLE_SNMPV1) || !defined(NETSNMP_DISABLE_SNMPV2C)
int
netsnmp_udp_getSecName(void *opaque, int olength,
                       const char *community,
                       size_t community_len, const char **secName,
                       const char **contextName)
{
    const com2SecEntry *c;
    netsnmp_udp_addr_pair *addr_pair = (netsnmp_udp_addr_pair *) opaque;
    struct sockaddr_in *from = (struct sockaddr_in *) &(addr_pair->remote_addr);
    char           *ztcommunity = NULL;

    if (secName != NULL) {
        *secName = NULL;  /* Haven't found anything yet */
    }

    /*
     * Special case if there are NO entries (as opposed to no MATCHING
     * entries).
     */

    if (com2SecList == NULL) {
        DEBUGMSGTL(("netsnmp_udp_getSecName", "no com2sec entries\n"));
        return 0;
    }

    /*
     * If there is no IPv4 source address, then there can be no valid security
     * name.
     */

   DEBUGMSGTL(("netsnmp_udp_getSecName", "opaque = %p (len = %d), sizeof = %d, family = %d (%d)\n",
   opaque, olength, (int)sizeof(netsnmp_udp_addr_pair), from->sin_family, AF_INET));
    if (opaque == NULL || olength != sizeof(netsnmp_udp_addr_pair) ||
        from->sin_family != AF_INET) {
        DEBUGMSGTL(("netsnmp_udp_getSecName",
		    "no IPv4 source address in PDU?\n"));
        return 1;
    }

    DEBUGIF("netsnmp_udp_getSecName") {
	ztcommunity = (char *)malloc(community_len + 1);
	if (ztcommunity != NULL) {
	    memcpy(ztcommunity, community, community_len);
	    ztcommunity[community_len] = '\0';
	}

	DEBUGMSGTL(("netsnmp_udp_getSecName", "resolve <\"%s\", 0x%08lx>\n",
		    ztcommunity ? ztcommunity : "<malloc error>",
		    (unsigned long)(from->sin_addr.s_addr)));
    }

    for (c = com2SecList; c != NULL; c = c->next) {
        {
            char buf1[INET_ADDRSTRLEN];
            char buf2[INET_ADDRSTRLEN];
            DEBUGMSGTL(("netsnmp_udp_getSecName","compare <\"%s\", %s/%s>",
                        c->community,
                        inet_ntop(AF_INET, &c->network, buf1, sizeof(buf1)),
                        inet_ntop(AF_INET, &c->mask, buf2, sizeof(buf2))));
        }
        if ((community_len == strlen(c->community)) &&
	    (memcmp(community, c->community, community_len) == 0) &&
            ((from->sin_addr.s_addr & c->mask) == c->network)) {
            DEBUGMSG(("netsnmp_udp_getSecName", "... SUCCESS\n"));
            if (c->negate) {
                /*
                 * If we matched a negative entry, then we are done - claim that we
                 * matched nothing.
                 */
                DEBUGMSG(("netsnmp_udp_getSecName", "... <negative entry>\n"));
                break;
            }
            if (secName != NULL) {
                *secName = c->secName;
                *contextName = c->contextName;
            }
            break;
        }
        DEBUGMSG(("netsnmp_udp_getSecName", "... nope\n"));
    }
    if (ztcommunity != NULL) {
        free(ztcommunity);
    }
    return 1;
}
#endif /* support for community based SNMP */


netsnmp_transport *
netsnmp_udp_create_tstring(const char *str, int local,
			   const char *default_target)
{
    struct sockaddr_in addr;

    if (netsnmp_sockaddr_in2(&addr, str, default_target)) {
        return netsnmp_udp_transport(&addr, local);
    } else {
        return NULL;
    }
}

netsnmp_transport *
netsnmp_udp_create_tspec(netsnmp_tdomain_spec *tspec)
{
    netsnmp_transport *t = netsnmp_udpipv4base_tspec_transport(tspec);
    if (NULL != t) {
        netsnmp_udp_transport_base(t);
    }
    return t;

}

netsnmp_transport *
netsnmp_udp_create_ostring(const void *o, size_t o_len, int local)
{
    struct sockaddr_in sin;

    if (netsnmp_ipv4_ostring_to_sockaddr(&sin, o, o_len))
        return netsnmp_udp_transport(&sin, local);
    return NULL;
}


void
netsnmp_udp_ctor(void)
{
    udpDomain.name = netsnmpUDPDomain;
    udpDomain.name_length = netsnmpUDPDomain_len;
    udpDomain.prefix = (const char**)calloc(2, sizeof(char *));
    udpDomain.prefix[0] = "udp";

    udpDomain.f_create_from_tstring     = NULL;
    udpDomain.f_create_from_tstring_new = netsnmp_udp_create_tstring;
    udpDomain.f_create_from_tspec       = netsnmp_udp_create_tspec;
    udpDomain.f_create_from_ostring     = netsnmp_udp_create_ostring;

    netsnmp_tdomain_register(&udpDomain);
}
