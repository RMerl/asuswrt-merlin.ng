/*
 *  tcpConnTable MIB architecture support for OpenBSD
 */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/data_access/tcpConn.h>
#include <net-snmp/agent/auto_nlist.h>

#include "tcp-mib/tcpConnectionTable/tcpConnectionTable_constants.h"
#include "tcp-mib/data_access/tcpConn_private.h"

#include "mibII/mibII_common.h"

#if HAVE_NETINET_TCP_H
#include <netinet/tcp.h>
#endif
#if HAVE_NETINET_TCP_TIMER_H
#include <netinet/tcp_timer.h>
#endif
#if HAVE_NETINET_TCPIP_H
#include <netinet/tcpip.h>
#endif
#if HAVE_NETINET_TCP_VAR_H
#include <netinet/tcp_var.h>
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
netsnmp_arch_tcpconn_entry_init(netsnmp_tcpconn_entry *entry)
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
netsnmp_arch_tcpconn_entry_cleanup(netsnmp_tcpconn_entry *entry)
{
    /*
     * cleanup
     */
}

/*
 * copy arch specific storage
 */
int
netsnmp_arch_tcpconn_entry_copy(netsnmp_tcpconn_entry *lhs,
                                  netsnmp_tcpconn_entry *rhs)
{
    return 0;
}

/*
 * delete an entry
 */
int
netsnmp_arch_tcpconn_entry_delete(netsnmp_tcpconn_entry *entry)
{
    if (NULL == entry)
        return -1;
    /** xxx-rks:9 tcpConn delete not implemented */
    return -1;
}


/**
 *
 * @retval  0 no errors
 * @retval !0 errors
 */
int
netsnmp_arch_tcpconn_container_load(netsnmp_container *container,
                                    u_int load_flags )
{
    int rc = 0;

    DEBUGMSGTL(("access:tcpconn:container",
                "tcpconn_container_arch_load (flags %x)\n", load_flags));

    if (NULL == container) {
        snmp_log(LOG_ERR, "no container specified/found for access_tcpconn\n");
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
    int      StateMap[] = { 1, 2, 3, 4, 5, 8, 6, 10, 9, 7, 11 };
    netsnmp_tcpconn_entry  *entry;
    struct   kinfo_file *kf;
    int      count;
    int      state;
    int      rc = 0;

    kf = kvm_getfiles(kd, KERN_FILE_BYFILE, DTYPE_SOCKET, sizeof(struct kinfo_file), &count);

    while (count--) {
	if (kf->so_protocol != IPPROTO_TCP)
	    goto skip;

	state = StateMap[kf->t_state];

	if (load_flags) {
	    if (state == TCPCONNECTIONSTATE_LISTEN) {
		if (load_flags & NETSNMP_ACCESS_TCPCONN_LOAD_NOLISTEN) {
		    DEBUGMSGT(("verbose:access:tcpconn:container",
			       " skipping listen\n"));
		    goto skip;
		}
	    }
	    else if (load_flags & NETSNMP_ACCESS_TCPCONN_LOAD_ONLYLISTEN) {
		DEBUGMSGT(("verbose:access:tcpconn:container",
			    " skipping non-listen\n"));
		goto skip;
	    }
	}

#if !defined(NETSNMP_ENABLE_IPV6)
        if (kf->so_family == AF_INET6)
	    goto skip;
#endif

        entry = netsnmp_access_tcpconn_entry_create();
        if(NULL == entry) {
            rc = -3;
            break;
        }

        /** oddly enough, these appear to already be in network order */
        entry->loc_port = ntohs(kf->inp_lport);
        entry->rmt_port = ntohs(kf->inp_fport);
        entry->tcpConnState = StateMap[kf->t_state];
        entry->pid = kf->p_pid;
        
        /** the addr string may need work */
	if (kf->so_family == AF_INET6) {
	    entry->loc_addr_len = entry->rmt_addr_len = 16;
	    memcpy(entry->loc_addr, &kf->inp_laddru, 16);
	    memcpy(entry->rmt_addr, &kf->inp_faddru, 16);
	}
	else {
	    entry->loc_addr_len = entry->rmt_addr_len = 4;
	    memcpy(entry->loc_addr, &kf->inp_laddru, 4);
	    memcpy(entry->rmt_addr, &kf->inp_faddru, 4);
	}
	DEBUGMSGTL(("tcp-mib/data_access", "tcp %d %d %d\n",
	    entry->loc_addr_len, entry->loc_port, entry->rmt_port));

        /*
         * add entry to container
         */
        entry->arbitrary_index = CONTAINER_SIZE(container) + 1;
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
    struct tcpcb   tcpcb;
    int      StateMap[] = { 1, 2, 3, 4, 5, 8, 6, 10, 9, 7, 11 };
    netsnmp_tcpconn_entry  *entry;
    int      state;
    int      rc = 0;

    /*
     *  Read in the buffer containing the TCP table data
     */
    if (!auto_nlist(TCP_SYMBOL, (char *)&table, sizeof(table))) {
	DEBUGMSGTL(("tcp-mib/tcpConn_openbsd", "Failed to read tcp_symbol\n"));
	return -1;
    }

    prev = (struct inpcb *)&CIRCLEQ_FIRST(&table.inpt_queue);
    prev = NULL;
    head = next = CIRCLEQ_FIRST(&table.inpt_queue);

    while (next) {
	if (!NETSNMP_KLOOKUP(next, (char *)&inpcb, sizeof(inpcb))) {
	    DEBUGMSGTL(("tcp-mib/data_access/tcpConn", "klookup inpcb failed\n"));
	    break;
	}
	if (prev && CIRCLEQ_PREV(&inpcb, inp_queue) != prev) {
	    snmp_log(LOG_ERR,"tcbtable link error\n");
	    break;
	}
	prev = next;
	next = CIRCLEQ_NEXT(&inpcb, inp_queue);
	if (!NETSNMP_KLOOKUP(inpcb.inp_ppcb, (char *)&tcpcb, sizeof(tcpcb))) {
	    DEBUGMSGTL(("tcp-mib/data_access/tcpConn", "klookup tcpcb failed\n"));
	    break;
	}
	state = StateMap[tcpcb.t_state];

	if (load_flags) {
	    if (state == TCPCONNECTIONSTATE_LISTEN) {
		if (load_flags & NETSNMP_ACCESS_TCPCONN_LOAD_NOLISTEN) {
		    DEBUGMSGT(("verbose:access:tcpconn:container",
			       " skipping listen\n"));
		    goto skip;
		}
	    }
	    else if (load_flags & NETSNMP_ACCESS_TCPCONN_LOAD_ONLYLISTEN) {
		DEBUGMSGT(("verbose:access:tcpconn:container",
			    " skipping non-listen\n"));
		goto skip;
	    }
	}

#if !defined(NETSNMP_ENABLE_IPV6)
        if (inpcb.inp_flags & INP_IPV6)
	    goto skip;
#endif

        entry = netsnmp_access_tcpconn_entry_create();
        if(NULL == entry) {
            rc = -3;
            break;
        }

        /** oddly enough, these appear to already be in network order */
        entry->loc_port = ntohs(inpcb.inp_lport);
        entry->rmt_port = ntohs(inpcb.inp_fport);
        entry->tcpConnState = state;
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
	DEBUGMSGTL(("tcp-mib/data_access", "tcp %d %d %d\n",
	    entry->loc_addr_len, entry->loc_port, entry->rmt_port));

        /*
         * add entry to container
         */
        entry->arbitrary_index = CONTAINER_SIZE(container) + 1;
        CONTAINER_INSERT(container, entry);
skip:
	if (head == next)
	    break;
    }

    if(rc<0)
        return rc;

    return 0;
}
#endif /* HAVE_KVM_GETFILES */
