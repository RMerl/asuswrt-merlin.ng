/*
 *  Interface MIB architecture support
 *
 * $Id$
 */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include "mibII/mibII_common.h"

#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/data_access/interface.h>
#include <net-snmp/data_access/route.h>
#include <net-snmp/data_access/ipaddress.h>
#include <net/if_dl.h>

#include "ip-forward-mib/data_access/route_sysctl.h"
#include "ip-forward-mib/inetCidrRouteTable/inetCidrRouteTable_constants.h"
#include "if-mib/data_access/interface_ioctl.h"
#include "route_private.h"

static int _load_ipv4(netsnmp_container*, int*);
static int _load_ipv6(netsnmp_container*, int*);
static int _create_ipv4(netsnmp_route_entry *);
static int _create_ipv6(netsnmp_route_entry *);
static int _delete_ipv4(netsnmp_route_entry *);
static int _delete_ipv6(netsnmp_route_entry *);

#if !defined(SA_SIZE) && !defined(RT_ROUNDUP)
#define RT_ROUNDUP(a)  \
        ((a) > 0 ? (1 + (((a) - 1) | (sizeof(long) - 1))) : sizeof(long))
#endif
#ifndef SA_SIZE
#define SA_SIZE(x) RT_ROUNDUP(((struct sockaddr *)(x))->sa_len)
#endif

/** arch specific load
 * @internal
 *
 * @retval  0 success
 * @retval -1 no container specified
 * @retval -2 could not access data source
 */
int
netsnmp_access_route_container_arch_load(netsnmp_container* container,
					 u_int load_flags)
{
    int count, err;

    err = 0;

    DEBUGMSGTL(("access:route:container", "route_container_arch_load\n"));

    if (container == NULL) {
        snmp_log(LOG_ERR, "no container specified/found for access_route\n");
        return -1;
    }

    err = _load_ipv4(container, &count);
    if (err != 0) {
        NETSNMP_LOGONCE((LOG_ERR, "_load_ipv4 failed %d\n", err));
        goto out;
    }

    if (err != 0 || load_flags & NETSNMP_ACCESS_ROUTE_LOAD_IPV4_ONLY)
	return err;

    err = _load_ipv6(container, &count);
    if (err != 0) {
        NETSNMP_LOGONCE((LOG_ERR, "_load_ipv6 failed %d\n", err));
        goto out;
    }

out:
    return (err == 0 ? 0 : -3);
}

/** arch specific new route entry creation
 * @internal
 *
 * @retval  0 success
 * @retval -1 invalid entry specified
 * @retval -2 could not create route entry
 */
int
netsnmp_arch_route_create(netsnmp_route_entry *entry)
{
    int err;

    if (NULL == entry)
        return -1;

    switch (entry->rt_dest_type) {
    case INETADDRESSTYPE_IPV4:
        err = _create_ipv4(entry);
        break;
#ifdef NETSNMP_ENABLE_IPV6
    case INETADDRESSTYPE_IPV6:
        err = _create_ipv6(entry);
        break;
#endif
    default:
        err = 0;
    }

    return (err == 0 ? 0 : -2);
}

/** arch specific new route entry deletion
 * @internal
 *
 * @retval  0 success
 * @retval -1 invalid entry specified
 * @retval -2 could not delete route entry
 */
int
netsnmp_arch_route_delete(netsnmp_route_entry *entry)
{
    int err;

    DEBUGMSGTL(("access:route:container",
                "route_container_arch_delete\n"));

    if (NULL == entry)
        return -1;

    switch (entry->rt_dest_type) {
    case INETADDRESSTYPE_IPV4:
        err = _delete_ipv4(entry);
        break;
#ifdef NETSNMP_ENABLE_IPV6
    case INETADDRESSTYPE_IPV6:
        err = _delete_ipv6(entry);
        break;
#endif
    default:
        err = 0;
    }

    return (err == 0 ? 0 : -2);
}

#if defined(freebsd4) || defined(netbsd4) || defined(openbsd) || defined(darwin)
static int
_type_from_flags(int flags)
{
    if (flags & RTF_UP) {
        if (flags & RTF_BLACKHOLE)
            return INETCIDRROUTETYPE_REJECT;
#ifdef RTF_LOCAL
        else if (flags & RTF_LOCAL)
            return INETCIDRROUTETYPE_LOCAL;
#endif
        else if (flags & RTF_GATEWAY)
            return INETCIDRROUTETYPE_REMOTE;
        else
            return INETCIDRROUTETYPE_OTHER;
    }
    return 0;
}


static int
_load_routing_table_from_sysctl(netsnmp_container* container, int *index,
    int family)
{
    char *buf, *lim, *next;
    int mib[6];
    size_t needed;
    int err;
    u_char dest_len, dest_type;
    struct rt_msghdr *rtm;

    buf = NULL;
    err = 0;

    if (family == AF_INET) {
        dest_len = 4;
        dest_type = INETADDRESSTYPE_IPV4;
    } else if (family == AF_INET6) {
        dest_len = 16;
        dest_type = INETADDRESSTYPE_IPV6;
    } else {
        err = EINVAL;
        goto out;
    }

    mib[0] = CTL_NET;
    mib[1] = PF_ROUTE;
    mib[2] = 0;
    mib[3] = family;
    mib[4] = NET_RT_DUMP;
    mib[5] = 0;

    if (sysctl(mib, (sizeof(mib) / sizeof(mib[0])), NULL, &needed, NULL,
        0) == -1) {
        err = errno;
        goto out;
    }

    if ((buf = malloc(needed)) == NULL) {
        err = ENOMEM;
        goto out;
    }

    if (sysctl(mib, (sizeof(mib) / sizeof(mib[0])), buf, &needed, NULL,
        0) == -1) {
        err = errno;
        goto out;
    }

    lim = buf + needed;

    for (next = buf; next < lim; next += rtm->rtm_msglen) {    
	struct sockaddr *if_name = NULL, *if_addr = NULL;
	struct sockaddr *dest_sa = NULL, *gateway_sa = NULL, *netmask_sa = NULL;
	netsnmp_route_entry *entry;
	char *addr_ptr;

        rtm = (struct rt_msghdr*)next;

        /* 
         * Some code in netstat checks for this ("netmasks done" case).
         * Filter this out (I don't know why it should exist).
         */
        if (rtm->rtm_addrs == RTA_DST)
            continue;
#ifdef RTF_CLONED
	if (rtm->rtm_flags & RTF_CLONED)
	    continue;
#endif
#ifdef RTF_WASCLONED
	if (rtm->rtm_flags & RTF_WASCLONED)
	    continue;
#endif

        entry = netsnmp_access_route_entry_create();
        if (entry == NULL)
            return -3;
        memset(entry->rt_dest, 0, dest_len);
        entry->rt_mask = 0;
        memset(entry->rt_nexthop, 0, dest_len);

        addr_ptr = (char *)(rtm + 1);

	if (rtm->rtm_addrs &  RTA_DST) {
	    dest_sa = (struct sockaddr *)addr_ptr;
	    addr_ptr += SA_SIZE(dest_sa);
	}
	if (rtm->rtm_addrs &  RTA_GATEWAY) {
	    gateway_sa = (struct sockaddr *)addr_ptr;
	    addr_ptr += SA_SIZE(gateway_sa);
	}
	if (rtm->rtm_addrs &  RTA_NETMASK) {
	    netmask_sa = (struct sockaddr *)addr_ptr;
	    addr_ptr += SA_SIZE(netmask_sa);
	}
	if (rtm->rtm_addrs &  RTA_IFP) {
	    if_name = (struct sockaddr *)addr_ptr;
	    addr_ptr += SA_SIZE(if_name);
	}
	if (rtm->rtm_addrs &  RTA_IFA) {
	    if_addr = (struct sockaddr *)addr_ptr;
	    addr_ptr += SA_SIZE(if_addr);
	}

        entry->if_index = rtm->rtm_index;

        /* arbitrary index */
        entry->ns_rt_index = ++(*index);

        /* copy dest & next hop */
        entry->rt_dest_type = dest_type;
        entry->rt_dest_len = dest_len;
        if (rtm->rtm_addrs & RTA_DST) {
            if (family == AF_INET)
                memcpy(entry->rt_dest,
                    &((struct sockaddr_in*)dest_sa)->sin_addr, dest_len);
#ifdef NETSNMP_ENABLE_IPV6
            if (family == AF_INET6)
                memcpy(entry->rt_dest,
                    &((struct sockaddr_in6*)dest_sa)->sin6_addr, dest_len);
#endif
        }

        entry->rt_nexthop_type = dest_type;
        entry->rt_nexthop_len = dest_len;
        if (rtm->rtm_addrs & RTA_GATEWAY) {
            if (family == AF_INET)
                memcpy(entry->rt_nexthop,
                    &((struct sockaddr_in*)gateway_sa)->sin_addr, dest_len);
#ifdef NETSNMP_ENABLE_IPV6
            if (family == AF_INET6)
                memcpy(entry->rt_nexthop,
                    &((struct sockaddr_in6*)gateway_sa)->sin6_addr, dest_len);
#endif
        }
        else {
            if (family == AF_INET)
                memcpy(entry->rt_nexthop,
                    &((struct sockaddr_in*)if_addr)->sin_addr, dest_len);
#ifdef NETSNMP_ENABLE_IPV6
            if (family == AF_INET6)
                memcpy(entry->rt_nexthop,
                    &((struct sockaddr_in6*)if_addr)->sin6_addr, dest_len);
#endif
        }

        if (family == AF_INET) {
	    if (netmask_sa) {
            /* count bits in mask */
		entry->rt_pfx_len = netsnmp_ipaddress_ipv4_prefix_len(
			((struct sockaddr_in *)netmask_sa)->sin_addr.s_addr);
		memcpy(&entry->rt_mask, &((struct sockaddr_in *)netmask_sa)->sin_addr, 4);
	    }
	    else {
		entry->rt_pfx_len = 32;
	    }
        }
#ifdef NETSNMP_ENABLE_IPV6
        if (family == AF_INET6) {
	    if (netmask_sa) {
            /* count bits in mask */
		entry->rt_pfx_len = netsnmp_ipaddress_ipv6_prefix_len(
			((struct sockaddr_in6 *)netmask_sa)->sin6_addr);
		memcpy(&entry->rt_mask, &((struct sockaddr_in6 *)netmask_sa)->sin6_addr, 16);
	    }
	    else {
		entry->rt_pfx_len = 128;
	    }
        }
#endif

#ifdef USING_IP_FORWARD_MIB_INETCIDRROUTETABLE_INETCIDRROUTETABLE_MODULE
        /*
    inetCidrRoutePolicy OBJECT-TYPE 
        SYNTAX     OBJECT IDENTIFIER 
        MAX-ACCESS not-accessible 
        STATUS     current 
        DESCRIPTION 
               "This object is an opaque object without any defined 
                semantics.  Its purpose is to serve as an additional 
                index which may delineate between multiple entries to 
                the same destination.  The value { 0 0 } shall be used 
                as the default value for this object."
        */
	entry->rt_policy = calloc(3, sizeof(oid));
	entry->rt_policy[2] = entry->if_index;
	entry->rt_policy_len = sizeof(oid)*3;
#endif

        entry->rt_type = _type_from_flags(rtm->rtm_flags);
	entry->rt_age = rtm->rtm_rmx.rmx_expire;
	entry->rt_nexthop_as = 0;
	/* entry->rt_metric1 = rtm->rtm_rmx.rmx_hopcount; */

        if (rtm->rtm_flags & RTF_DYNAMIC)
            entry->rt_proto = IANAIPROUTEPROTOCOL_ICMP;
        else if (rtm->rtm_flags & RTF_STATIC)
            entry->rt_proto = IANAIPROUTEPROTOCOL_NETMGMT;
#ifdef RTF_LOCAL
        else if (rtm->rtm_flags & RTF_LOCAL)
            entry->rt_proto = IANAIPROUTEPROTOCOL_LOCAL;
#endif
        else
            entry->rt_proto = IANAIPROUTEPROTOCOL_OTHER;

        if (CONTAINER_INSERT(container, entry) < 0) {
            DEBUGMSGTL(("access:route:container",
                "error with route_entry: insert into container failed.\n"));
            netsnmp_access_route_entry_free(entry);
            continue;
        }
    }

out:
    free(buf);

    return err;
}

/**
 * netsnmp_access_route_container_arch_load* functions.
 */
static int
_load_ipv4(netsnmp_container* container, int *index)
{

    DEBUGMSGTL(("access:route:container",
                "route_container_arch_load ipv4\n"));
    return _load_routing_table_from_sysctl(container, index, AF_INET);
}

static int
_load_ipv6(netsnmp_container* container, int *index)
{

#ifdef NETSNMP_ENABLE_IPV6
    DEBUGMSGTL(("access:route:container",
                "route_container_arch_load ipv6\n"));
    return _load_routing_table_from_sysctl(container, index, AF_INET6);
#else
    return 0;
#endif
}

/**
 * netsnmp_access_route_create* functions.
 *
 * TODO: add logic similar to newroute(..) in sbin/route/route.c .
 */
static int
_create_ipv4(netsnmp_route_entry *entry)
{

    return 0;
}

static int
_create_ipv6(netsnmp_route_entry *entry)
{

    return 0;
}

/**
 * netsnmp_access_route_delete* functions.
 *
 * TODO: add logic similar to newroute(..) in sbin/route/route.c .
 */
static int
_delete_ipv4(netsnmp_route_entry *entry)
{

    return 0;
}

static int
_delete_ipv6(netsnmp_route_entry *entry)
{
#ifdef NETSNMP_ENABLE_IPV6

#endif
    return 0;
}
#endif /* defined(freebsd7) || defined(netbsd) || defined(openbsd) */
