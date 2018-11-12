/*
 *  udp_endpointTable MIB architecture support for OpenBSD
 */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/data_access/ipaddress.h>
#include <net-snmp/data_access/udp_endpoint.h>
#include <net-snmp/agent/auto_nlist.h>

#include "udp-mib/udpEndpointTable/udpEndpointTable_constants.h"
#include "udp-mib/data_access/udp_endpoint_private.h"

#include "mibII/mibII_common.h"

#if HAVE_NETINET_UDP_H
#include <netinet/udp.h>
#endif
#if HAVE_NETINET_UDP_VAR_H
#include <netinet/udp_var.h>
#endif

#if HAVE_KVM_GETFILES
#if defined(HAVE_KVM_GETFILE2) || !defined(openbsd5)
#undef HAVE_KVM_GETFILES
#endif
#endif

#if HAVE_KVM_GETFILES
#include <kvm.h>
#include <sys/sysctl.h>
#define _KERNEL /* for DTYPE_SOCKET */
#include <sys/file.h>
#undef _KERNEL
#endif

#ifdef HAVE_KVM_GETFILES
static int _kvmload(netsnmp_container *container, u_int flags);
#else
static int _load(netsnmp_container *container, u_int flags);
#endif

/*
 * initialize arch specific storage
 *
 * @retval  0: success
 * @retval <0: error
 */
int
netsnmp_arch_udp_endpoint_entry_init(netsnmp_udp_endpoint_entry *entry)
{
    /*
     * init
     */
    return 0;
}

/*
 * cleanup arch specific storage
 */
void
netsnmp_arch_udp_endpoint_entry_cleanup(netsnmp_udp_endpoint_entry *entry)
{
    /*
     * cleanup
     */
}

/*
 * copy arch specific storage
 */
int
netsnmp_arch_udp_endpoint_entry_copy(netsnmp_udp_endpoint_entry *lhs,
                                  netsnmp_udp_endpoint_entry *rhs)
{
    return 0;
}

/*
 * delete an entry
 */
int
netsnmp_arch_udp_endpoint_entry_delete(netsnmp_udp_endpoint_entry *entry)
{
    if (NULL == entry)
        return -1;
    /** xxx-rks:9 udp_endpoint delete not implemented */
    return -1;
}


/**
 *
 * @retval  0 no errors
 * @retval !0 errors
 */
int
netsnmp_arch_udp_endpoint_container_load(netsnmp_container *container,
                                    u_int load_flags )
{
    int rc = 0;

    DEBUGMSGTL(("access:udp_endpoint:container",
                "udp_endpoint_container_arch_load (flags %x)\n", load_flags));

    if (NULL == container) {
        snmp_log(LOG_ERR, "no container specified/found for access_udp_endpoint\n");
        return -1;
    }

#ifdef HAVE_KVM_GETFILES
    rc = _kvmload(container, load_flags);
#else
    rc = _load(container, load_flags);
#endif

    return rc;
}

#ifdef HAVE_KVM_GETFILES
/**
 *
 * @retval  0 no errors
 * @retval !0 errors
 */
static int
_kvmload(netsnmp_container *container, u_int load_flags)
{
    netsnmp_udp_endpoint_entry  *entry;
    struct   kinfo_file *kf;
    int      count;
    int      rc = 0;

    kf = kvm_getfiles(kd, KERN_FILE_BYFILE, DTYPE_SOCKET, sizeof(struct kinfo_file), &count);

    while (count--) {
	if (kf->so_protocol != IPPROTO_UDP)
	    goto skip;
#if !defined(NETSNMP_ENABLE_IPV6)
        if (kf->so_family == AF_INET6)
	    goto skip;
#endif

        entry = netsnmp_access_udp_endpoint_entry_create();
        if(NULL == entry) {
            rc = -3;
            break;
        }

        /** oddly enough, these appear to already be in network order */
        entry->loc_port = ntohs(kf->inp_lport);
        entry->rmt_port = ntohs(kf->inp_fport);
        entry->pid = kf->p_pid;
        
        /** the addr string may need work */
	if (kf->so_family == AF_INET6) {
	    entry->loc_addr_len = entry->rmt_addr_len = 16;
	    memcpy(entry->loc_addr, &kf->inp_laddru, 16);
	    memcpy(entry->rmt_addr, &kf->inp_faddru, 16);
	}
	else {
	    entry->loc_addr_len = entry->rmt_addr_len = 4;
	    memcpy(entry->loc_addr, &kf->inp_laddru[0], 4);
	    memcpy(entry->rmt_addr, &kf->inp_faddru[0], 4);
	}
	DEBUGMSGTL(("udp-mib/data_access", "udp %d %d %d\n",
	    entry->loc_addr_len, entry->loc_port, entry->rmt_port));

        /*
         * add entry to container
         */
        entry->index = CONTAINER_SIZE(container) + 1;
        CONTAINER_INSERT(container, entry);
skip:
	kf++;
    }

    if (rc < 0)
    return rc;
    return 0;
}

#else /* HAVE_KVM_GETFILES */

/**
 *
 * @retval  0 no errors
 * @retval !0 errors
 */
static int
_load(netsnmp_container *container, u_int load_flags)
{
    struct inpcbtable table;
    struct inpcb   *head, *next, *prev;
    struct inpcb   inpcb;
    netsnmp_udp_endpoint_entry  *entry;
    int      rc = 0;

    /*
     *  Read in the buffer containing the TCP table data
     */
    if (!auto_nlist(UDB_SYMBOL, (char *) &table, sizeof(table))) {
	DEBUGMSGTL(("udp-mib/udp_endpoint_openbsd", "Failed to read udp_symbol\n"));
	return -1;
    }

    prev = (struct inpcb *)&CIRCLEQ_FIRST(&table.inpt_queue);
    prev = NULL;
    head = next = CIRCLEQ_FIRST(&table.inpt_queue);

    while (next) {
	NETSNMP_KLOOKUP(next, (char *)&inpcb, sizeof(inpcb));
	if (prev && CIRCLEQ_PREV(&inpcb, inp_queue) != prev) {
	    snmp_log(LOG_ERR,"udbtable link error\n");
	    break;
	}
	prev = next;
	next = CIRCLEQ_NEXT(&inpcb, inp_queue);

#if !defined(NETSNMP_ENABLE_IPV6)
        if (inpcb.inp_flags & INP_IPV6)
            goto skip;
#endif
        entry = netsnmp_access_udp_endpoint_entry_create();
        if (NULL == entry) {
            rc = -3;
            break;
        }

        /** oddly enough, these appear to already be in network order */
        entry->loc_port = ntohs(inpcb.inp_lport);
        entry->rmt_port = ntohs(inpcb.inp_fport);
        entry->pid = 0;
        
        /** the addr string may need work */
	if (inpcb.inp_flags & INP_IPV6) {
	    entry->loc_addr_len = entry->rmt_addr_len = 16;
	    memcpy(entry->loc_addr, &inpcb.inp_laddr6, 16);
	    memcpy(entry->rmt_addr, &inpcb.inp_faddr6, 16);
	}
	else {
	    entry->loc_addr_len = entry->rmt_addr_len = 4;
	    memcpy(entry->loc_addr, &inpcb.inp_laddr, 4);
	    memcpy(entry->rmt_addr, &inpcb.inp_faddr, 4);
	}

        /*
         * add entry to container
         */
        entry->index = CONTAINER_SIZE(container) + 1;
        CONTAINER_INSERT(container, entry);
#if !defined(NETSNMP_ENABLE_IPV6)
    skip:
#endif
	if (next == head)
	    break;
    }

    if (rc < 0)
        return rc;

    return 0;
}
#endif /* HAVE_KVM_GETFILES */
