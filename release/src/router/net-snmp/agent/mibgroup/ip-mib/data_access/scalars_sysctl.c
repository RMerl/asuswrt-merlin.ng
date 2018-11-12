/*
 *  IP-MIB architecture support
 *
 * $Id$
 */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/data_access/ip_scalars.h>

#include <sys/types.h>
#include <sys/protosw.h>
#include <sys/sysctl.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <errno.h>
#include <stdlib.h>

/* XXX: the values passed to netsnmp_arch_ip_scalars(..) may or may not be
 * portable to the other BSDs -- it seems to be portable back to FreeBSD 4.x
 * (Darwin 10.8.0) at least.
 */

static int
netsnmp_arch_ip_scalars_sysctl(const char *access_module,
                               int mib[], size_t mib_len,
                               u_long *old_value, u_long *new_value)
{
    int newint, oldint;
    size_t needed;
    int rc;

    needed = sizeof(oldint);
    if (new_value)
        newint = *new_value;

    rc = sysctl(mib, mib_len, &oldint, &needed,
                new_value ? &newint : NULL, (new_value ? sizeof(newint) : 0));
    if (rc == -1) {
        DEBUGMSGTL((access_module, "sysctl %s failed - %s\n",
                    (new_value == NULL ? "get" : "set"),
                    strerror(errno)));
    }
    *old_value = oldint;
    return rc;
}

int ipDefaultTTL_mib[] = {
    CTL_NET,
    PF_INET,
    IPPROTO_IP,
    IPCTL_DEFTTL
};

int ipForwarding_mib[] = {
    CTL_NET,
    PF_INET,
    IPPROTO_IP,
    IPCTL_FORWARDING
};

int ipv6IpDefaultHopLimit_mib[] = {
    CTL_NET,
    PF_INET6,
    IPPROTO_IP,
    IPV6CTL_DEFHLIM
};

int ipv6IpForwarding_mib[] = {
    CTL_NET,
    PF_INET6,
    IPPROTO_IP,
    IPV6CTL_FORWARDING
};

#define MIB_LEN(a)		(sizeof(a) / sizeof(*a))

#define IPDEFAULTTTL_LEN	MIB_LEN(ipDefaultTTL_mib)
#define IPFORWARDING_LEN	MIB_LEN(ipForwarding_mib)
#define IPV6IPDEFAULTHOPLIMIT_LEN	MIB_LEN(ipv6IpDefaultHopLimit_mib)
#define IPV6IPFORWARDING_LEN	MIB_LEN(ipv6IpForwarding_mib)

int
netsnmp_arch_ip_scalars_ipDefaultTTL_get(u_long *value)
{
    int rc;

    if (NULL == value)
        return -1;

    rc = netsnmp_arch_ip_scalars_sysctl("access:ipDefaultTTL",
                                        ipDefaultTTL_mib,
                                        IPDEFAULTTTL_LEN,
                                        value, NULL);
    if (rc != 0)
        return -2;

    return 0;
}

int
netsnmp_arch_ip_scalars_ipDefaultTTL_set(u_long value)
{
    int rc;

    if (1 == value)
        ;
    else if (2 == value)
        value = 0;
    else {
        DEBUGMSGTL(("access:ipDefaultTTL", "bad value %ld\n",
                    value));
        return SNMP_ERR_WRONGVALUE;
    }

    rc = netsnmp_arch_ip_scalars_sysctl("access:ipForwarding",
                                        ipForwarding_mib,
                                        IPFORWARDING_LEN,
                                        NULL, &value);
    if (rc != 0)
        return -1;

    return 0;
}

int
netsnmp_arch_ip_scalars_ipForwarding_get(u_long *value)
{
    int rc;

    if (NULL == value)
        return -1;

    rc = netsnmp_arch_ip_scalars_sysctl("access:ipForwarding",
                                        ipForwarding_mib,
                                        IPFORWARDING_LEN,
                                        value, NULL);
    if (rc != 0)
        return -2;

    /* On FreeBSD 7.2 at least, the value passed to the sysctl can be coerced
     * into a non-zero value; convert it into a value that's sane per the
     * IP-MIB definition.
     */
    if (*value != 0)
        *value = 1;

    return 0;
}

int
netsnmp_arch_ip_scalars_ipForwarding_set(u_long value)
{
    int rc;

    if (1 == value)
        ;
    else if (2 == value)
        value = 0;
    else {
        DEBUGMSGTL(("access:ipForwarding", "bad value %ld\n",
                    value));
        return SNMP_ERR_WRONGVALUE;
    }

    rc = netsnmp_arch_ip_scalars_sysctl("access:ipForwarding",
                                        ipForwarding_mib,
                                        IPFORWARDING_LEN,
                                        NULL, &value);
    if (rc != 0)
        return -1;

    return 0;
}

int
netsnmp_arch_ip_scalars_ipv6IpDefaultHopLimit_get(u_long *value)
{
    int rc;

    if (NULL == value)
        return -1;

    rc = netsnmp_arch_ip_scalars_sysctl("access:ipv6IpDefaultHopLimit",
                                        ipv6IpDefaultHopLimit_mib,
                                        IPV6IPDEFAULTHOPLIMIT_LEN,
                                        value, NULL);
    if (rc != 0)
        return -2;

    return 0;
}

int
netsnmp_arch_ip_scalars_ipv6IpDefaultHopLimit_set(u_long value)
{
    int rc;

    if (1 == value)
        ;
    else if (2 == value)
        value = 0;
    else {
        DEBUGMSGTL(("access:ipv6IpDefaultHopLimit", "bad value %ld\n",
                    value));
        return SNMP_ERR_WRONGVALUE;
    }

    rc = netsnmp_arch_ip_scalars_sysctl("access:ipForwarding",
                                        ipv6IpDefaultHopLimit_mib,
                                        IPV6IPDEFAULTHOPLIMIT_LEN,
                                        NULL, &value);
    if (rc != 0)
        return -1;

    return 0;
}

int
netsnmp_arch_ip_scalars_ipv6IpForwarding_get(u_long *value)
{
    int rc;

    if (NULL == value)
        return -1;

    rc = netsnmp_arch_ip_scalars_sysctl("access:ipv6IpForwarding",
                                        ipv6IpForwarding_mib,
                                        IPV6IPFORWARDING_LEN,
                                        value, NULL);
    if (rc != 0)
        return -2;

    /* On FreeBSD 7.2 at least, the value passed to the sysctl can be coerced
     * into a non-zero value; convert it into a value that's sane per the
     * IP-MIB definition.
     */
    if (*value != 0)
        *value = 1;

    return 0;
}

int
netsnmp_arch_ip_scalars_ipv6IpForwarding_set(u_long value)
{
    int rc;

    if (1 == value)
        ;
    else if (2 == value)
        value = 0;
    else {
        DEBUGMSGTL(("access:ipForwarding", "bad value %ld\n",
                    value));
        return SNMP_ERR_WRONGVALUE;
    }

    rc = netsnmp_arch_ip_scalars_sysctl("access:ipv6IpForwarding",
                                        ipv6IpForwarding_mib,
                                        IPV6IPFORWARDING_LEN,
                                        NULL, &value);
    if (rc != 0)
        return -1;

    return 0;
}

static long ipReasmTimeout_val;

void
netsnmp_arch_ip_scalars_register_handlers(void)
{
    static oid ipReasmTimeout_oid[] = { 1, 3, 6, 1, 2, 1, 4, 13, 0 };

    /* 
     * This value is static at compile time on FreeBSD; it really should be a
     * probed via either sysctl or sysconf at runtime as the compiled value and
     * the runtime value compiled into the kernel can vary.
     *
     * Please refer to sys/protosw.h for more details on what this value is (in
     * particular PR_SLOWHZ).
     */
    ipReasmTimeout_val = IPFRAGTTL / PR_SLOWHZ;

    netsnmp_register_long_instance("ipReasmTimeout",
                                   ipReasmTimeout_oid,
                                   OID_LENGTH(ipReasmTimeout_oid),
                                   &ipReasmTimeout_val, NULL);
}
