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

#include <inet/ip.h>
#include <inet/mib2.h>

#include "ip-forward-mib/data_access/route_ioctl.h"
#include "ip-forward-mib/inetCidrRouteTable/inetCidrRouteTable_constants.h"
#include "if-mib/data_access/interface_ioctl.h"
#include "route_private.h"

static int _load_v4(netsnmp_container *container, u_long *count);
static int _load_v6(netsnmp_container *container, u_long *count);

/** arch specific load
 * @internal
 *
 * @retval  0 success
 * @retval -1 no container specified
 * @retval -2 could not open data file
 */
int
netsnmp_access_route_container_arch_load(netsnmp_container* container,
                                         u_int load_flags)
{
    u_long          count = 0;
    int             rc;

    DEBUGMSGTL(("access:route:container",
                "route_container_arch_load (flags %x)\n", load_flags));

    if (NULL == container) {
        snmp_log(LOG_ERR, "no container specified/found for access_route\n");
        return -1;
    }

    rc = _load_v4(container, &count);
    
#ifdef NETSNMP_ENABLE_IPV6
    if((0 != rc) || (load_flags & NETSNMP_ACCESS_ROUTE_LOAD_IPV4_ONLY))
        return rc;

    /*
     * load ipv6. ipv6 module might not be loaded,
     * so ignore -2 err (file not found)
     */
    rc = _load_v6(container, &count);
    if (-2 == rc)
        rc = 0;
#endif

    return rc;
}

/*
 * create a new entry
 */
int
netsnmp_arch_route_create(netsnmp_route_entry *entry)
{
    if (NULL == entry)
        return -1;

    if (4 != entry->rt_dest_len) {
        DEBUGMSGT(("access:route:create", "only ipv4 supported\n"));
        return -2;
    }

    /* return _netsnmp_ioctl_route_set_v4(entry); */
    return -2;
}

/*
 * create a new entry
 */
int
netsnmp_arch_route_delete(netsnmp_route_entry *entry)
{
    if (NULL == entry)
        return -1;

    if (4 != entry->rt_dest_len) {
        DEBUGMSGT(("access:route:create", "only ipv4 supported\n"));
        return -2;
    }

    /* return _netsnmp_ioctl_route_delete_v4(entry); */
    return -2;
}


static int
IP_Cmp_Route(void *addr, void *ep)
{
    mib2_ipRouteEntry_t *Ep = ep, *Addr = addr;

    if ((Ep->ipRouteDest == Addr->ipRouteDest) &&
        (Ep->ipRouteNextHop == Addr->ipRouteNextHop) &&
        (Ep->ipRouteType == Addr->ipRouteType) &&
        (Ep->ipRouteProto == Addr->ipRouteProto) &&
        (Ep->ipRouteMask == Addr->ipRouteMask) &&
        (Ep->ipRouteInfo.re_max_frag == Addr->ipRouteInfo.re_max_frag) &&
        (Ep->ipRouteInfo.re_rtt == Addr->ipRouteInfo.re_rtt) &&
        (Ep->ipRouteInfo.re_ref == Addr->ipRouteInfo.re_ref) &&
        (Ep->ipRouteInfo.re_frag_flag == Addr->ipRouteInfo.re_frag_flag) &&
        (Ep->ipRouteInfo.re_src_addr == Addr->ipRouteInfo.re_src_addr) &&
        (Ep->ipRouteInfo.re_ire_type == Addr->ipRouteInfo.re_ire_type) &&
        (Ep->ipRouteInfo.re_obpkt == Addr->ipRouteInfo.re_obpkt) &&
        (Ep->ipRouteInfo.re_ibpkt == Addr->ipRouteInfo.re_ibpkt)
        )
        return (0);
    else
        return (1);             /* Not found */
}


static int
IP6_Cmp_Route(void *addr, void *ep)
{
    mib2_ipv6RouteEntry_t *Ep = ep, *Addr = addr;

    if ((memcmp(&Ep->ipv6RouteDest, &Addr->ipv6RouteDest, 16) == 0) &&
        (memcmp(&Ep->ipv6RouteNextHop, &Addr->ipv6RouteNextHop, 16) == 0) &&
        (Ep->ipv6RouteType == Addr->ipv6RouteType) &&
        (Ep->ipv6RouteInfo.re_max_frag == Addr->ipv6RouteInfo.re_max_frag) &&
        (Ep->ipv6RouteInfo.re_rtt == Addr->ipv6RouteInfo.re_rtt) &&
        (Ep->ipv6RouteInfo.re_ref == Addr->ipv6RouteInfo.re_ref) &&
        (Ep->ipv6RouteInfo.re_frag_flag == Addr->ipv6RouteInfo.re_frag_flag) &&
        (memcmp(&Ep->ipv6RouteInfo.re_src_addr, &Addr->ipv6RouteInfo.re_src_addr, 16) == 0) &&
        (Ep->ipv6RouteInfo.re_ire_type == Addr->ipv6RouteInfo.re_ire_type) &&
        (Ep->ipv6RouteInfo.re_obpkt == Addr->ipv6RouteInfo.re_obpkt) &&
        (Ep->ipv6RouteInfo.re_ibpkt == Addr->ipv6RouteInfo.re_ibpkt)
        )
        return (0);
    else
        return (1);             /* Not found */
}


static int _load_v4(netsnmp_container *container, u_long *count)
{
    netsnmp_route_entry *entry;
    mib2_ipRouteEntry_t Curentry, Nextentry;
    int req_type;

    for (Nextentry.ipRouteDest = (u_long) -2, req_type = GET_FIRST;;
             Nextentry = Curentry, req_type = GET_NEXT) {
	if (getMibstat(MIB_IP_ROUTE, &Curentry, sizeof(mib2_ipRouteEntry_t),
		       req_type, &IP_Cmp_Route, &Nextentry) != 0)
	    break;
#ifdef HAVE_DEFINED_IRE_CACHE
	if (Curentry.ipRouteInfo.re_ire_type & IRE_CACHE)
	    continue;
#endif /* HAVE_DEFINED_IRE_CACHE */
	if (Curentry.ipRouteInfo.re_ire_type & IRE_BROADCAST)
	    continue;
	entry = netsnmp_access_route_entry_create();
	Curentry.ipRouteIfIndex.o_bytes[Curentry.ipRouteIfIndex.o_length] = '\0';
	entry->if_index = netsnmp_access_interface_index_find(
                Curentry.ipRouteIfIndex.o_bytes);
	entry->ns_rt_index = entry->if_index;

	entry->rt_dest_type = INETADDRESSTYPE_IPV4;
	entry->rt_dest_len = 4;
	memcpy(entry->rt_dest, &Curentry.ipRouteDest, 4);
	memcpy(&entry->rt_mask, &Curentry.ipRouteMask, 4);

	entry->rt_nexthop_type = INETADDRESSTYPE_IPV4;
	entry->rt_nexthop_len = 4;
	memcpy(entry->rt_nexthop, &Curentry.ipRouteNextHop, 4);

	entry->rt_pfx_len = netsnmp_ipaddress_ipv4_prefix_len(Curentry.ipRouteMask);
	entry->rt_type = Curentry.ipRouteType;
	entry->rt_proto = Curentry.ipRouteProto;
	entry->rt_age = Curentry.ipRouteAge;
	entry->rt_metric1 = Curentry.ipRouteMetric1;
	entry->rt_metric2 = Curentry.ipRouteMetric2;
	entry->rt_metric3 = Curentry.ipRouteMetric3;
	entry->rt_metric4 = Curentry.ipRouteMetric4;

	/*
	 * insert into container
	 */
	if (CONTAINER_INSERT(container, entry) < 0) {
	    DEBUGMSGTL(("access:route:container", "error with route_entry: insert into container failed.\n"));
	    netsnmp_access_route_entry_free(entry);
	    continue;
	}
	*count++;
    }
    return 0;
}


static int _load_v6(netsnmp_container *container, u_long *count)
{
    netsnmp_route_entry *entry;
    mib2_ipv6RouteEntry_t Curentry, Nextentry;
    int req_type;

    memset(&Nextentry, 0, sizeof(Nextentry));
    for (req_type = GET_FIRST;;
             Nextentry = Curentry, req_type = GET_NEXT) {
	if (getMibstat(MIB_IP6_ROUTE, &Curentry, sizeof(mib2_ipv6RouteEntry_t),
		       req_type, &IP6_Cmp_Route, &Nextentry) != 0)
	    break;
#ifdef HAVE_DEFINED_IRE_CACHE
	if (Curentry.ipv6RouteInfo.re_ire_type & IRE_CACHE)
	    continue;
#endif /* HAVE_DEFINED_IRE_CACHE */
	if (Curentry.ipv6RouteInfo.re_ire_type & IRE_BROADCAST)
	    continue;
	entry = netsnmp_access_route_entry_create();
	Curentry.ipv6RouteIfIndex.o_bytes[Curentry.ipv6RouteIfIndex.o_length] = '\0';
	entry->if_index = netsnmp_access_interface_index_find(
                Curentry.ipv6RouteIfIndex.o_bytes);
	entry->ns_rt_index = entry->if_index;

	entry->rt_dest_type = INETADDRESSTYPE_IPV6;
	entry->rt_dest_len = 16;
	memcpy(entry->rt_dest, &Curentry.ipv6RouteDest, 16);

	entry->rt_nexthop_type = INETADDRESSTYPE_IPV6;
	entry->rt_nexthop_len = 16;
	memcpy(entry->rt_nexthop, &Curentry.ipv6RouteNextHop, 16);

	entry->rt_pfx_len = Curentry.ipv6RoutePfxLength;
	entry->rt_type = Curentry.ipv6RouteType;
	entry->rt_proto = Curentry.ipv6RouteProtocol;
	entry->rt_age = Curentry.ipv6RouteAge;
	entry->rt_policy = calloc(3, sizeof(oid));
	entry->rt_policy_len = 3;
	entry->rt_policy[2] = Curentry.ipv6RoutePolicy;
	entry->rt_metric1 = Curentry.ipv6RouteMetric;
	entry->rt_metric2 = Curentry.ipv6RouteWeight;

	/*
	 * insert into container
	 */
	if (CONTAINER_INSERT(container, entry) < 0) {
	    DEBUGMSGTL(("access:route:container", "error with route_entry: insert into container failed.\n"));
	    netsnmp_access_route_entry_free(entry);
	    continue;
	}
	*count++;
    }
    return 0;
}
