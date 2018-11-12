/*
 *  Interface MIB architecture support
 */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/data_access/arp.h>
#include <net-snmp/data_access/interface.h>

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_types.h>
#include <net/route.h>
#include <netinet/if_ether.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#if !defined(SA_SIZE) && !defined(RT_ROUNDUP)
#define RT_ROUNDUP(a)  \
	((a) > 0 ? (1 + (((a) - 1) | (sizeof(long) - 1))) : sizeof(long))
#endif

static int _load_arp_table_from_sysctl(netsnmp_arp_access *);
static int _load_ndp_table_from_sysctl(netsnmp_arp_access *);

netsnmp_arp_access *
netsnmp_access_arp_create(u_int init_flags,
                          NetsnmpAccessArpUpdate *update_hook,
                          NetsnmpAccessArpGC *gc_hook,
                          int *cache_timeout, int *cache_flags,
                          char *cache_expired)
{
    netsnmp_arp_access *access;

    access = SNMP_MALLOC_TYPEDEF(netsnmp_arp_access);
    if (NULL == access) {
        snmp_log(LOG_ERR, "malloc error in netsnmp_access_arp_create\n");
        return NULL;
    }

    access->arch_magic = NULL;
    access->magic = NULL;
    access->update_hook = update_hook;
    access->gc_hook = gc_hook;
    access->synchronized = 0;

    if (cache_timeout != NULL)
        *cache_timeout = 5;
    if (cache_flags != NULL)
        *cache_flags |= NETSNMP_CACHE_DONT_FREE_BEFORE_LOAD
                        | NETSNMP_CACHE_AUTO_RELOAD;
    access->cache_expired = cache_expired;

    return access;
}

int netsnmp_access_arp_delete(netsnmp_arp_access *access)
{
    if (NULL == access)
        return 0;

    netsnmp_access_arp_unload(access);
    free(access);

    return 0;
}

int netsnmp_access_arp_load(netsnmp_arp_access *access)
{
    int err = 0;

    access->generation++;
    DEBUGMSGTL(("access:route:container",
                "route_container_arch_load ipv4\n"));
    err = _load_arp_table_from_sysctl(access);
    if (err != 0) {
        NETSNMP_LOGONCE((LOG_ERR,
            "netsnmp_access_arp_ipv4 failed %d\n", err));
        goto out;
    }
    access->gc_hook(access);
    access->synchronized = (err == 0);

#ifdef NETSNMP_ENABLE_IPV6
    DEBUGMSGTL(("access:route:container",
                "route_container_arch_load ipv6\n"));
    err = _load_ndp_table_from_sysctl(access);
    if (err != 0) {
        NETSNMP_LOGONCE((LOG_ERR,
            "netsnmp_access_arp_ipv6 failed %d\n", err));
        goto out;
    }
    access->gc_hook(access);
    access->synchronized = (err == 0);
#endif

out:
    return (err == 0 ? 0 : -1);
}

int netsnmp_access_arp_unload(netsnmp_arp_access *access)
{
    access->synchronized = 0;

    return 0;
}

static int
_load_arp_table_from_sysctl(netsnmp_arp_access *access)
{
    netsnmp_arp_entry *entry;
    struct rt_msghdr *rtm;
    struct sockaddr_inarp *sin2;
    struct sockaddr_dl *sdl;
    char *buf, *lim, *newbuf, *next;
    int mib[6];
    size_t needed;
    int err, st;

    netsnmp_assert(NULL != access);

    mib[0] = CTL_NET;
    mib[1] = PF_ROUTE;
    mib[2] = 0;
    mib[3] = AF_INET;
    mib[4] = NET_RT_FLAGS;
#ifdef RTF_LLINFO
    mib[5] = RTF_LLINFO;
#elif defined(RTF_LLDATA)
    mib[5] = RTF_LLDATA;
#else
    mib[5] = 0;
#endif

    err = 0;
    buf = newbuf = NULL;

    /* The following logic was adapted from search(..) in usr.sbin/arp/arp.c */

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
        st = sysctl(mib, 6, buf, &needed, NULL, 0);
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

        rtm = (struct rt_msghdr *)next;
        sin2 = (struct sockaddr_inarp*)(rtm + 1);
#ifdef SA_SIZE
        sdl = (struct sockaddr_dl*)((char *)sin2 + SA_SIZE(sin2));
#else
	sdl = (struct sockaddr_dl*)(void *)(RT_ROUNDUP(sin2->sin_len) + (char *)(void *)sin2);
#endif

        entry = netsnmp_access_arp_entry_create();
        if (NULL == entry) {
            err = ENOMEM;
            break;
        }

        entry->generation = access->generation;
        entry->if_index = rtm->rtm_index;

        entry->arp_ipaddress_len = 4;

        memcpy(entry->arp_ipaddress, &sin2->sin_addr.s_addr,
            entry->arp_ipaddress_len);

        /* HW Address */
        entry->arp_physaddress_len = 6;
        if (0 < sdl->sdl_alen &&
            sdl->sdl_alen <= NETSNMP_ACCESS_ARP_PHYSADDR_BUF_SIZE) {
            memcpy(entry->arp_physaddress, LLADDR(sdl), sdl->sdl_alen);
            /* Process status */
            /* XXX: setting this value for all states is wrong. */
            entry->arp_state = INETNETTOMEDIASTATE_REACHABLE;
        } else {
            entry->arp_physaddress[0] = '\0';
            entry->arp_state = INETNETTOMEDIASTATE_INCOMPLETE;
        }

        /* Process type */
        /* XXX: more states should be handled here, probably.. */
        if (rtm->rtm_rmx.rmx_expire == 0)
            entry->arp_type = INETNETTOMEDIATYPE_STATIC;
        else
            entry->arp_type = INETNETTOMEDIATYPE_DYNAMIC;

        access->update_hook(access, entry);

    }

out:
    free(buf);
    return err;
}

static int
_load_ndp_table_from_sysctl(netsnmp_arp_access *access)
{
#if 1
    netsnmp_arp_entry *entry;
    struct rt_msghdr *rtm;
    struct sockaddr_in6 *sin2;
    struct sockaddr_dl *sdl;
    size_t needed;
    int err, mib[6], st;
    char *buf, *lim, *newbuf, *next;

    netsnmp_assert(NULL != access);

    mib[0] = CTL_NET;
    mib[1] = PF_ROUTE;
    mib[2] = 0;
    mib[3] = AF_INET6;
    mib[4] = NET_RT_FLAGS;
#ifdef RTF_LLINFO
    mib[5] = RTF_LLINFO;
#elif defined(RTF_LLDATA)
    mib[5] = RTF_LLDATA;
#else
    mib[5] = 0;
#endif

    err = 0;
    buf = newbuf = NULL;

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
        st = sysctl(mib, 6, buf, &needed, NULL, 0);
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

        rtm = (struct rt_msghdr *)next;
        sin2 = (struct sockaddr_in6*)(rtm + 1);
#ifdef SA_SIZE
        sdl = (struct sockaddr_dl*)((char *)sin2 + SA_SIZE(sin2));
#else
	sdl = (struct sockaddr_dl*)(void *)(RT_ROUNDUP(sin2->sin6_len) + (char *)(void *)sin2);
#endif

        if (!(rtm->rtm_flags & RTF_HOST) ||
            IN6_IS_ADDR_MULTICAST(&sin2->sin6_addr))
            continue;

        entry = netsnmp_access_arp_entry_create();
        if (NULL == entry) {
            err = ENOMEM;
            break;
        }

        entry->generation = access->generation;
        entry->if_index = rtm->rtm_index;

        entry->arp_ipaddress_len = 16;

        memcpy(entry->arp_ipaddress, &sin2->sin6_addr.s6_addr,
            entry->arp_ipaddress_len);

        /* HW Address */
        entry->arp_physaddress_len = sdl->sdl_alen;
        if (0 < sdl->sdl_alen &&
            sdl->sdl_alen <= NETSNMP_ACCESS_ARP_PHYSADDR_BUF_SIZE) {
            memcpy(entry->arp_physaddress, LLADDR(sdl), sdl->sdl_alen);
            /* Process status */
            /* XXX: setting this value for all states is wrong. */
            entry->arp_state = INETNETTOMEDIASTATE_REACHABLE;
        } else {
            entry->arp_physaddress[0] = '\0';
            entry->arp_state = INETNETTOMEDIASTATE_INCOMPLETE;
        }

        /* Process type */
        /* XXX: more states should be handled here, probably.. */
        if (rtm->rtm_rmx.rmx_expire == 0)
            entry->arp_type = INETNETTOMEDIATYPE_STATIC;
        else
            entry->arp_type = INETNETTOMEDIATYPE_DYNAMIC;

        access->update_hook(access, entry);

    }

out:
    free(buf);
    return err;
#else
    return 0;
#endif
}
