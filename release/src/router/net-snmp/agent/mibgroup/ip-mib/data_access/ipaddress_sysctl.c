/*
 *  Interface MIB architecture support
 *
 * $Id$
 */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-features.h>
#include <net-snmp/net-snmp-includes.h>
#include "mibII/mibII_common.h"

#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/data_access/ipaddress.h>
#include <net-snmp/data_access/interface.h>

#include "ip-mib/ipAddressTable/ipAddressTable_constants.h"
#include "ip-mib/ipAddressPrefixTable/ipAddressPrefixTable_constants.h"
#include "mibgroup/util_funcs.h"

#include <errno.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>

netsnmp_feature_require(prefix_info)
netsnmp_feature_require(find_prefix_info)

netsnmp_feature_child_of(ipaddress_arch_entry_copy, ipaddress_common)

#ifdef NETSNMP_FEATURE_REQUIRE_IPADDRESS_ARCH_ENTRY_COPY
netsnmp_feature_require(ipaddress_ioctl_entry_copy)
#endif /* NETSNMP_FEATURE_REQUIRE_IPADDRESS_ARCH_ENTRY_COPY */

#include "ipaddress_ioctl.h"
#include "ipaddress_private.h"
#include "if-mib/data_access/interface_private.h"

#define ROUNDUP(a) \
  ((a) > 0 ? (1 + (((a) - 1) | (sizeof(long) - 1))) : sizeof(long))

/*
 * initialize arch specific storage
 *
 * @retval  0: success
 * @retval <0: error
 */
int
netsnmp_arch_ipaddress_entry_init(netsnmp_ipaddress_entry *entry)
{
    /*
     * init ipv4 stuff
     */
    /* if (NULL == netsnmp_ioctl_ipaddress_entry_init(entry)) */
    /*     return -1; */

    /*
     * init ipv6 stuff
     *   so far, we can just share the ipv4 stuff, so nothing to do
     */
    
    return 0;
}

/*
 * cleanup arch specific storage
 */
void
netsnmp_arch_ipaddress_entry_cleanup(netsnmp_ipaddress_entry *entry)
{
    /*
     * cleanup ipv4 stuff
     */
    /*netsnmp_ioctl_ipaddress_entry_cleanup(entry); */

    /*
     * cleanup ipv6 stuff
     *   so far, we can just share the ipv4 stuff, so nothing to do
     */
}

#ifndef NETSNMP_FEATURE_REMOVE_IPADDRESS_ARCH_ENTRY_COPY
/*
 * copy arch specific storage
 */
int
netsnmp_arch_ipaddress_entry_copy(netsnmp_ipaddress_entry *lhs,
                                  netsnmp_ipaddress_entry *rhs)
{
    int rc;

    rc = 0;

    /*
     * copy ipv4 stuff
     */
    /*rc = netsnmp_ioctl_ipaddress_entry_copy(lhs, rhs); */
    if (rc)
        return rc;

    /*
     * copy ipv6 stuff
     *   so far, we can just share the ipv4 stuff, so nothing to do
     */

    return rc;
}
#endif /* NETSNMP_FEATURE_REMOVE_IPADDRESS_ARCH_ENTRY_COPY */

/*
 * create a new entry
 */
int
netsnmp_arch_ipaddress_create(netsnmp_ipaddress_entry *entry)
{
    if (NULL == entry)
        return -1;

    if (4 == entry->ia_address_len) {
        return -1;
    } else if (16 == entry->ia_address_len) {
        return -1;
    } else {
        DEBUGMSGT(("access:ipaddress:create", "wrong length of IP address\n"));
        return -2;
    }
}

/*
 * create a new entry
 */
int
netsnmp_arch_ipaddress_delete(netsnmp_ipaddress_entry *entry)
{
    if (NULL == entry)
        return -1;

    if (4 == entry->ia_address_len) {
        return -2;
    } else if (16 == entry->ia_address_len) {
        return -3;
    } else {
        DEBUGMSGT(("access:ipaddress:create", "only ipv4 supported\n"));
        return -2;
    }
}

/**
 *
 * @retval  0 no errors
 * @retval !0 errors
 */
int
netsnmp_arch_ipaddress_container_load(netsnmp_container *container,
                                      u_int load_flags)
{
    netsnmp_ipaddress_entry *entry = NULL;
    u_char *if_list = NULL, *cp;
    size_t if_list_size = 0;
    int sysctl_oid[] = { CTL_NET, PF_ROUTE, 0, 0, NET_RT_IFLIST, 0 };
    struct ifa_msghdr *ifa;
    struct sockaddr *a;
    int amask;
    int rc = 0;
    int idx_offset = 0;

    DEBUGMSGTL(("access:ipaddress:container:sysctl",
                "load (flags %u)\n", load_flags));

    if (NULL == container) {
        snmp_log(LOG_ERR, "no container specified/found for interface\n");
        return -1;
    }

    if (sysctl(sysctl_oid, sizeof(sysctl_oid)/sizeof(int), 0,
               &if_list_size, 0, 0) == -1) {
        snmp_log(LOG_ERR, "could not get interface info (size)\n");
        return -2;
    }

    if_list = (u_char*)malloc(if_list_size);
    if (if_list == NULL) {
        snmp_log(LOG_ERR, "could not allocate memory for interface info "
                 "(%u bytes)\n", (unsigned) if_list_size);
        return -3;
    } else {
        DEBUGMSGTL(("access:ipaddress:container:sysctl",
                    "allocated %u bytes for if_list\n",
                    (unsigned) if_list_size));
    }

    if (sysctl(sysctl_oid, sizeof(sysctl_oid)/sizeof(int), if_list,
               &if_list_size, 0, 0) == -1) {
        snmp_log(LOG_ERR, "could not get interface info\n");
        free(if_list);
        return -2;
    }

    /* pass 2: walk addresses */
    for (cp = if_list; cp < if_list + if_list_size; cp += ifa->ifam_msglen) {
        ifa = (struct ifa_msghdr *) cp;
        int rtax;

        if (ifa->ifam_type != RTM_NEWADDR)
            continue;

        DEBUGMSGTL(("access:ipaddress:container:sysctl",
                    "received 0x%x in RTM_NEWADDR for ifindex %u\n",
                    ifa->ifam_addrs, ifa->ifam_index));

        entry = netsnmp_access_ipaddress_entry_create();
        if (entry == NULL) {
            rc = -3;
            break;
        }

        a = (struct sockaddr *) (ifa + 1);
        entry->ia_status = IPADDRESSSTATUSTC_UNKNOWN;
        entry->ia_origin = IPADDRESSORIGINTC_OTHER;
	entry->ia_address_len = 0;
        for (amask = ifa->ifam_addrs, rtax = 0; amask != 0; amask >>= 1, rtax++) {
            if ((amask & 1) != 0) {
                entry->ns_ia_index = ++idx_offset;
                entry->if_index = ifa->ifam_index;
                DEBUGMSGTL(("access:ipaddress:container:sysctl",
                            "%d: a=%p, sa_len=%d, sa_family=0x%x\n",
                            (int)entry->if_index, a, a->sa_len, a->sa_family));

                if (a->sa_family == AF_INET || a->sa_family == 0) {
                    struct sockaddr_in *a4 = (struct sockaddr_in *)a;
		    char str[128];
		    DEBUGMSGTL(("access:ipaddress:container:sysctl",
		                "IPv4 addr %s\n", inet_ntop(AF_INET, &a4->sin_addr.s_addr, str, 128)));
                    if (rtax == RTAX_IFA) {
			entry->ia_address_len = 4;
                        memcpy(entry->ia_address, &a4->sin_addr.s_addr, entry->ia_address_len);
		    }
                    else if (rtax == RTAX_NETMASK)
                        entry->ia_prefix_len = netsnmp_ipaddress_ipv4_prefix_len(a4->sin_addr.s_addr);
                }
                else if (a->sa_family == AF_INET6) {
                    struct sockaddr_in6 *a6 = (struct sockaddr_in6 *)a;
		    char str[128];
		    DEBUGMSGTL(("access:ipaddress:container:sysctl",
		                "IPv6 addr %s\n", inet_ntop(AF_INET6, &a6->sin6_addr.s6_addr, str, 128)));
                    if (rtax == RTAX_IFA) {
			entry->ia_address_len = 16;
                        memcpy(entry->ia_address, &a6->sin6_addr, entry->ia_address_len);
		    }
                    else if (rtax == RTAX_NETMASK) {
                        entry->ia_prefix_len = netsnmp_ipaddress_ipv6_prefix_len(a6->sin6_addr);
			DEBUGMSGTL(("access:ipaddress:container:sysctl",
			            "prefix_len=%d\n", entry->ia_prefix_len));
		    }
                }
                a = (struct sockaddr *) ( ((char *) a) + ROUNDUP(a->sa_len) );
            }
        }
	if (entry->ia_address_len == 0) {
	    DEBUGMSGTL(("access:ipaddress:container:sysctl",
	                "entry skipped\n"));
	    netsnmp_access_ipaddress_entry_free(entry);
	}
	else if (CONTAINER_INSERT(container, entry) < 0) {
            DEBUGMSGTL(("access:ipaddress:container","error with ipaddress_entry: insert into container failed.\n"));
            netsnmp_access_ipaddress_entry_free(entry);
            continue;
        }
    }

    if (if_list != NULL)
        free(if_list);

    return 0;
}
