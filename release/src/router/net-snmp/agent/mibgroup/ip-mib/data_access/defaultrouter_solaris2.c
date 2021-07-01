/*
 *  Interface MIB architecture support
 *
 * $Id:$
 */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include "mibII/mibII_common.h"

#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/data_access/defaultrouter.h>

#include "ip-mib/ipDefaultRouterTable/ipDefaultRouterTable.h"
#include "defaultrouter_private.h"

#include <inet/ip.h>
#include <inet/mib2.h>

#if !defined(SA_SIZE) && !defined(RT_ROUNDUP)
#define RT_ROUNDUP(a)  \
        ((a) > 0 ? (1 + (((a) - 1) | (sizeof(long) - 1))) : sizeof(long))
#endif

/**---------------------------------------------------------------------*/
/*
 * local static prototypes
 */
static int _load_defaultrouter_from_mib2(netsnmp_container *, int);

static int idx_offset;

/*
 * initialize arch specific storage
 *
 * @retval  0: success
 * @retval <0: error
 */
int
netsnmp_arch_defaultrouter_entry_init(netsnmp_defaultrouter_entry *entry)
{
    /*
     * init
     */
    return 0;
}

/**
 *
 * @retval  0 no errors
 * @retval !0 errors
 */
int
netsnmp_arch_defaultrouter_container_load(netsnmp_container *container,
                                          u_int load_flags)
{
    int err;

    err = 0;
    idx_offset = 0;

    DEBUGMSGTL(("access:defaultrouter:entry:arch", "load\n"));
    if (NULL == container) {
        snmp_log(LOG_ERR,
            "netsnmp_arch_defaultrouter_container_load: container invalid\n");
        return 1;
    }

    err = _load_defaultrouter_from_mib2(container, AF_INET);
    if (err != 0) {
        u_int flags = NETSNMP_ACCESS_DEFAULTROUTER_FREE_KEEP_CONTAINER;
        netsnmp_access_defaultrouter_container_free(container, flags);
        goto out;
    }

#ifdef NETSNMP_ENABLE_IPV6
    err = _load_defaultrouter_from_mib2(container, AF_INET6);
    if (err != 0) {
        u_int flags = NETSNMP_ACCESS_DEFAULTROUTER_FREE_KEEP_CONTAINER;
        netsnmp_access_defaultrouter_container_free(container, flags);
        goto out;
    }
#endif

out:
    return err;
}


static int
IP_Cmp_Route(void *addr, void *ep)
{
    return (0);             /* found */
}


/**
 *
 * @retval  0 no errors
 * @retval !0 errors
 */
static int
_load_defaultrouter_from_mib2(netsnmp_container *container, int family)
{
    netsnmp_defaultrouter_entry *entry;
    mib2_ipRouteEntry_t Curentry, Nextentry;
    int req_type;
    int err = 0;
    int idx_offset = 0;

    for (Nextentry.ipRouteDest = (u_long) -2, req_type = GET_FIRST;;
             Nextentry = Curentry, req_type = GET_NEXT) {
        if (getMibstat(MIB_IP_ROUTE, &Curentry, sizeof(mib2_ipRouteEntry_t),
                       req_type, &IP_Cmp_Route, &Nextentry) != 0)
            break;
	DEBUGMSGTL(("access:defaultrouter", "nexthop %x type %x\n", Curentry.ipRouteNextHop, Curentry.ipRouteInfo.re_ire_type));
        if (!(Curentry.ipRouteInfo.re_ire_type & IRE_DEFAULT))
            continue;
        entry = netsnmp_access_defaultrouter_entry_create();
	Curentry.ipRouteIfIndex.o_bytes[Curentry.ipRouteIfIndex.o_length] = '\0';
        entry->ns_dr_index = ++idx_offset;
        entry->dr_if_index = netsnmp_access_interface_index_find(
                Curentry.ipRouteIfIndex.o_bytes);

        entry->dr_addresstype = INETADDRESSTYPE_IPV4;
        entry->dr_address_len = 4;
        memcpy(entry->dr_address, &Curentry.ipRouteNextHop, 4);

        entry->dr_lifetime = Curentry.ipRouteAge;
        entry->dr_preference = Curentry.ipRouteMetric1;

        if ((err = CONTAINER_INSERT(container, entry)) < 0) {
            DEBUGMSGTL(("access:defaultrouter:container",
                        "error with defaultrouter_entry: "
                        "insert into container failed.\n"));
            netsnmp_access_defaultrouter_entry_free(entry);
            /* goto out; */
        }
    }

out:
    return 0;
}
