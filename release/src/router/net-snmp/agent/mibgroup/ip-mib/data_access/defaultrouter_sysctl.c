/*
 *  Interface MIB architecture support
 *
 * $Id:$
 */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/data_access/defaultrouter.h>

#include "ip-mib/ipDefaultRouterTable/ipDefaultRouterTable.h"
#include "defaultrouter_private.h"

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <net/route.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>

#if !defined(SA_SIZE) && !defined(RT_ROUNDUP)
#define RT_ROUNDUP(a)  \
        ((a) > 0 ? (1 + (((a) - 1) | (sizeof(long) - 1))) : sizeof(long))
#endif

/**---------------------------------------------------------------------*/
/*
 * local static prototypes
 */
static int _load_defaultrouter_from_sysctl(netsnmp_container *, int);

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

    err = _load_defaultrouter_from_sysctl(container, AF_INET);
    if (err != 0) {
        u_int flags = NETSNMP_ACCESS_DEFAULTROUTER_FREE_KEEP_CONTAINER;
        netsnmp_access_defaultrouter_container_free(container, flags);
        goto out;
    }

#ifdef NETSNMP_ENABLE_IPV6
    err = _load_defaultrouter_from_sysctl(container, AF_INET6);
    if (err != 0) {
        u_int flags = NETSNMP_ACCESS_DEFAULTROUTER_FREE_KEEP_CONTAINER;
        netsnmp_access_defaultrouter_container_free(container, flags);
        goto out;
    }
#endif

out:
    return err;
}

/**
 *
 * @retval  0 no errors
 * @retval !0 errors
 */
static int
_load_defaultrouter_from_sysctl(netsnmp_container *container, int family)
{
    netsnmp_defaultrouter_entry *entry;
    struct rt_msghdr *rtm;
    struct sockaddr *dst_sa, *gw_sa;
    char *buf, *lim, *newbuf, *next;
    char address[NETSNMP_ACCESS_DEFAULTROUTER_BUF_SIZE];
    int mib[6];
    size_t address_len, needed;
    int address_type, err, preference, st;

    netsnmp_assert(NULL != container);

    mib[0] = CTL_NET;
    mib[1] = PF_ROUTE;
    mib[2] = 0;
    mib[3] = family;
    mib[4] = NET_RT_DUMP;
    mib[5] = 0;

    err = 0;

    buf = newbuf = NULL;

    if (family == AF_INET) {
        address_len = 4;
        address_type = INETADDRESSTYPE_IPV4;
#ifdef NETSNMP_ENABLE_IPV6
    } else if (family == AF_INET6) {
        address_len = 16;
        address_type = INETADDRESSTYPE_IPV6;
#endif
    } else {
        err = EINVAL;
        goto out;
    }

    if (sysctl(mib, 6, NULL, &needed, NULL, 0) < 0) {
        err = errno;
        goto out;
    }

    /* Empty arp table. */
    if (needed == 0)
        goto out;

    for (;;) {
        newbuf = realloc(buf, needed);
        if (newbuf == NULL) {
            err = ENOMEM;
            goto out;
        }
        buf = newbuf;
        st = sysctl(mib, sizeof(mib) / sizeof(mib[0]), buf, &needed, NULL, 0);
        if (st == 0 || errno != ENOMEM)
            break;
        else
            needed += needed / 8; /* XXX: why 8? */
    }
    if (st == -1) {
        err = errno;
        goto out;
    }

    lim = buf + needed;
    for (next = buf; next < lim; next += rtm->rtm_msglen) {
#ifdef NETSNMP_ENABLE_IPV6
	struct in6_addr in6addr_any = IN6ADDR_ANY_INIT;
#endif

        rtm = (struct rt_msghdr *)next;

        if (!(rtm->rtm_addrs & RTA_GATEWAY))
            continue;

        dst_sa = (struct sockaddr*)(rtm + 1);
#ifdef SA_SIZE
        gw_sa = (struct sockaddr*)(SA_SIZE(dst_sa) + (char*)dst_sa);
#else
        gw_sa = (struct sockaddr*)(RT_ROUNDUP(dst_sa->sa_len) + (char*)dst_sa);
#endif

        switch (family) {
        case AF_INET:
            if (((struct sockaddr_in*)dst_sa)->sin_addr.s_addr != INADDR_ANY)
                continue;
	    memcpy(address, &((struct sockaddr_in*)gw_sa)->sin_addr.s_addr,
	           address_len);
            break;
#ifdef NETSNMP_ENABLE_IPV6
        case AF_INET6:
            if (memcmp(((struct sockaddr_in6*)dst_sa)->sin6_addr.s6_addr,
			&in6addr_any, sizeof in6addr_any) != 0)
		continue; /* XXX: need to determine qualifying criteria for
                       * default gateways in IPv6. */
            memcpy(address, &((struct sockaddr_in6*)dst_sa)->sin6_addr.s6_addr,
		   address_len);
            break;
#endif
        default:
            break;
        }

        entry = netsnmp_access_defaultrouter_entry_create();
        if (NULL == entry) {
            err = ENOMEM;
            break;
        }

        /* XXX: this is wrong (hardcoding the router preference to medium). */
        preference = 0;

        entry->ns_dr_index    = ++idx_offset;
        entry->dr_addresstype = address_type;
        entry->dr_address_len = address_len;
        memcpy(entry->dr_address, address, sizeof(address));
        entry->dr_if_index = rtm->rtm_index;
        entry->dr_lifetime    = rtm->rtm_rmx.rmx_expire;
        entry->dr_preference  = preference;

        if (CONTAINER_INSERT(container, entry) < 0) {
            DEBUGMSGTL(("access:arp:container",
                        "error with defaultrouter_entry: "
                        "insert into container failed.\n"));
            netsnmp_access_defaultrouter_entry_free(entry);
            goto out;
        }
    }

out:
    free(buf);
    return err;
}
