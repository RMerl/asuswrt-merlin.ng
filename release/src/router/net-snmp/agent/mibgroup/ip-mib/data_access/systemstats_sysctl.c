/*
 *  ipSystemStatsTable and ipIfStatsTable interface MIB architecture support
 *
 * $Id$
 */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/data_access/ipstats.h>
#include <net-snmp/data_access/systemstats.h>

#include "../ipSystemStatsTable/ipSystemStatsTable.h"
#include "systemstats_private.h"

#if defined(NETSNMP_IFNET_NEEDS_KERNEL) && !defined(_KERNEL)
#define _KERNEL 1
#define _I_DEFINED_KERNEL
#endif
#if NETSNMP_IFNET_NEEDS_KERNEL_STRUCTURES
#define _KERNEL_STRUCTURES
#endif

#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>

#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/protosw.h>

#include <net/if.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip_var.h>
#if HAVE_NETINET6_IP6_VAR_H
#include <sys/queue.h>
#include <netinet6/ip6_var.h>
#endif
#if !defined(freebsd7) && !defined(openbsd5)
#include <netinet6/in6_var.h>
#endif

#ifdef darwin

/* This struct is in netinet6/ip6_var.h which Apple for obscure reasons
 * do not distribute as part of /usr/include :-(
 */

struct	ip6stat {
	u_quad_t ip6s_total;		/* total packets received */
	u_quad_t ip6s_tooshort;		/* packet too short */
	u_quad_t ip6s_toosmall;		/* not enough data */
	u_quad_t ip6s_fragments;	/* fragments received */
	u_quad_t ip6s_fragdropped;	/* frags dropped(dups, out of space) */
	u_quad_t ip6s_fragtimeout;	/* fragments timed out */
	u_quad_t ip6s_fragoverflow;	/* fragments that exceeded limit */
	u_quad_t ip6s_forward;		/* packets forwarded */
	u_quad_t ip6s_cantforward;	/* packets rcvd for unreachable dest */
	u_quad_t ip6s_redirectsent;	/* packets forwarded on same net */
	u_quad_t ip6s_delivered;	/* datagrams delivered to upper level*/
	u_quad_t ip6s_localout;		/* total ip packets generated here */
	u_quad_t ip6s_odropped;		/* lost packets due to nobufs, etc. */
	u_quad_t ip6s_reassembled;	/* total packets reassembled ok */
	u_quad_t ip6s_fragmented;	/* datagrams sucessfully fragmented */
	u_quad_t ip6s_ofragments;	/* output fragments created */
	u_quad_t ip6s_cantfrag;		/* don't fragment flag was set, etc. */
	u_quad_t ip6s_badoptions;	/* error in option processing */
	u_quad_t ip6s_noroute;		/* packets discarded due to no route */
	u_quad_t ip6s_badvers;		/* ip6 version != 6 */
	u_quad_t ip6s_rawout;		/* total raw ip packets generated */
	u_quad_t ip6s_badscope;		/* scope error */
	u_quad_t ip6s_notmember;	/* don't join this multicast group */
	u_quad_t ip6s_nxthist[256];	/* next header history */
	u_quad_t ip6s_m1;		/* one mbuf */
	u_quad_t ip6s_m2m[32];		/* two or more mbuf */
	u_quad_t ip6s_mext1;		/* one ext mbuf */
	u_quad_t ip6s_mext2m;		/* two or more ext mbuf */
	u_quad_t ip6s_exthdrtoolong;	/* ext hdr are not continuous */
	u_quad_t ip6s_nogif;		/* no match gif found */
	u_quad_t ip6s_toomanyhdr;	/* discarded due to too many headers */

	/*
	 * statistics for improvement of the source address selection
	 * algorithm:
	 * XXX: hardcoded 16 = # of ip6 multicast scope types + 1
	 */
	/* number of times that address selection fails */
	u_quad_t ip6s_sources_none;
	/* number of times that an address on the outgoing I/F is chosen */
	u_quad_t ip6s_sources_sameif[16];
	/* number of times that an address on a non-outgoing I/F is chosen */
	u_quad_t ip6s_sources_otherif[16];
	/*
	 * number of times that an address that has the same scope
	 * from the destination is chosen.
	 */
	u_quad_t ip6s_sources_samescope[16];
	/*
	 * number of times that an address that has a different scope
	 * from the destination is chosen.
	 */
	u_quad_t ip6s_sources_otherscope[16];
	/* number of times that an deprecated address is chosen */
	u_quad_t ip6s_sources_deprecated[16];

	u_quad_t ip6s_forward_cachehit;
	u_quad_t ip6s_forward_cachemiss;
};

#endif /* darwin*/

static int _systemstats_v4(netsnmp_container* container, u_int load_flags);

#if defined (NETSNMP_ENABLE_IPV6)
static int _systemstats_v6(netsnmp_container* container, u_int load_flags);
#endif

static int ncpus;

void
netsnmp_access_systemstats_arch_init(void)
{
    int    ncpu_mib[]  = { CTL_HW, HW_NCPU };
    size_t siz = sizeof(ncpus);
    if (sysctl(ncpu_mib, 2, &ncpus, &siz, NULL, 0) < 0) {
	snmp_log_perror("hw.ncpu");
        ncpus = 1;
    }
}

/*
 *
 * @retval  0 success
 * @retval -1 no container specified
 * @retval -2 could not open file
 * @retval -3 could not create entry (probably malloc)
 * @retval -4 file format error
 */
int
netsnmp_access_systemstats_container_arch_load(netsnmp_container* container,
                                             u_int load_flags)
{
    int rc1;
#if defined (NETSNMP_ENABLE_IPV6)
    int rc2;
#endif

    if (NULL == container) {
        snmp_log(LOG_ERR, "no container specified/found for access_systemstats_\n");
        return -1;
    }
    
    /*
     * load v4 and v6 stats. Even if one fails, try the other.
     * If they have the same rc, return it. if the differ, return
     * the smaller one. No log messages, since each individual function
     * would have logged its own message.
     */
    rc1 = _systemstats_v4(container, load_flags);
#if defined (NETSNMP_ENABLE_IPV6)
    rc2 = _systemstats_v6(container, load_flags);
    if ((rc1 == rc2) || (rc1 < rc2))
        return rc1;
        
    return rc2;
#else
    return rc1;
#endif
}

/*
 * Based on load_flags, load ipSystemStatsTable or ipIfStatsTable for ipv4 entries. 
 */
#ifdef __NetBSD__

static int
_systemstats_v4(netsnmp_container* container, u_int load_flags)
{
    netsnmp_systemstats_entry *entry = NULL;
    uint64_t ipstat[IP_NSTATS];
    size_t len = sizeof(ipstat);

    if (sysctlbyname("net.inet.ip.stats", &ipstat, &len, NULL, 0) < 0) {
	NETSNMP_LOGONCE((LOG_ERR, "Cannot sysctlbyname net.inet.ip.stats\n"));
	return -2;
    }

    DEBUGMSGTL(("access:systemstats:container:arch", "load v4 (flags %x)\n",
                load_flags));

    netsnmp_assert(container != NULL); /* load function shoulda checked this */

    if (load_flags & NETSNMP_ACCESS_SYSTEMSTATS_LOAD_IFTABLE) {
        /* we do not support ipIfStatsTable for ipv4 */
        return 0;
    }

    entry = netsnmp_access_systemstats_entry_create(1, 0,
		"ipSystemStatsTable.ipv4");
    if(NULL == entry) {
	snmp_log(LOG_ERR, "systemstats_v4: cannot create entry\n");
	netsnmp_access_systemstats_container_free(container,
						  NETSNMP_ACCESS_SYSTEMSTATS_FREE_NOFLAGS);
	return -3;
    }

    /*
     * OK - we've now got (or created) the data structure for
     *      this systemstats, including any "static" information.
     * Now parse the rest of the line (i.e. starting from 'stats')
     *      to extract the relevant statistics, and populate
     *      data structure accordingly.
     */

    entry->stats.HCInReceives.low = ipstat[IP_STAT_TOTAL] & 0xffffffff;
    entry->stats.HCInReceives.high = ipstat[IP_STAT_TOTAL] >> 32;
    entry->stats.InHdrErrors = ipstat[IP_STAT_BADSUM]
		    + ipstat[IP_STAT_TOOSHORT] + ipstat[IP_STAT_TOOSMALL]
	            + ipstat[IP_STAT_BADHLEN] + ipstat[IP_STAT_BADLEN];
    entry->stats.InAddrErrors = ipstat[IP_STAT_CANTFORWARD];
    entry->stats.HCOutForwDatagrams.low = ipstat[IP_STAT_FORWARD] & 0xffffffff;
    entry->stats.HCOutForwDatagrams.high = ipstat[IP_STAT_FORWARD] >> 32;
    entry->stats.InUnknownProtos = ipstat[IP_STAT_NOPROTO];
    entry->stats.InDiscards = ipstat[IP_STAT_FRAGDROPPED];
    entry->stats.HCInDelivers.low = ipstat[IP_STAT_DELIVERED] & 0xffffffff;
    entry->stats.HCInDelivers.high = ipstat[IP_STAT_DELIVERED] >> 32;
    entry->stats.HCOutRequests.low = ipstat[IP_STAT_LOCALOUT] & 0xffffffff;
    entry->stats.HCOutRequests.high = ipstat[IP_STAT_LOCALOUT] >> 32;
    entry->stats.HCOutDiscards.low = ipstat[IP_STAT_ODROPPED] & 0xffffffff;
    entry->stats.HCOutDiscards.high = ipstat[IP_STAT_ODROPPED] >> 32;
    entry->stats.HCOutNoRoutes.low = ipstat[IP_STAT_NOGIF] & 0xffffffff;
    entry->stats.HCOutNoRoutes.high = ipstat[IP_STAT_NOGIF] >> 32;
    /* entry->stats. = scan_vals[12]; / * ReasmTimeout */
    entry->stats.ReasmReqds = ipstat[IP_STAT_FRAGMENTS];
    entry->stats.ReasmOKs = ipstat[IP_STAT_REASSEMBLED];
    entry->stats.ReasmFails = ipstat[IP_STAT_FRAGDROPPED]
		    + ipstat[IP_STAT_FRAGTIMEOUT];
    entry->stats.HCOutFragOKs.low = ipstat[IP_STAT_FRAGMENTS] & 0xffffffff;
    entry->stats.HCOutFragOKs.high = ipstat[IP_STAT_FRAGMENTS] >> 32;
    entry->stats.HCOutFragFails.low = ipstat[IP_STAT_CANTFRAG] & 0xffffffff;
    entry->stats.HCOutFragFails.high = ipstat[IP_STAT_CANTFRAG] >> 32;
    entry->stats.HCOutFragCreates.low = ipstat[IP_STAT_OFRAGMENTS] & 0xffffffff;
    entry->stats.HCOutFragCreates.high = ipstat[IP_STAT_OFRAGMENTS] >> 32;

    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_HCINRECEIVES] = 1;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_INHDRERRORS] = 1;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_INADDRERRORS] = 1;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_HCOUTFORWDATAGRAMS] = 1;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_INUNKNOWNPROTOS] = 1;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_INDISCARDS] = 1;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_HCINDELIVERS] = 1;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_HCOUTREQUESTS] = 1;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_HCOUTDISCARDS] = 1;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_HCOUTNOROUTES] = 1;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_REASMREQDS] = 1;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_REASMOKS] = 1;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_REASMFAILS] = 1;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_HCOUTFRAGOKS] = 1;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_HCOUTFRAGFAILS] = 1;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_HCOUTFRAGCREATES] = 1;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_DISCONTINUITYTIME] = 1;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_REFRESHRATE] = 1;

    /*
     * add to container
     */
    if (CONTAINER_INSERT(container, entry) < 0)
    {
	snmp_log(LOG_ERR, "error with systemstats_v4: insert into container failed.\n");
	netsnmp_access_systemstats_entry_free(entry);
    }

    return 0;
}


#if defined (NETSNMP_ENABLE_IPV6)

static int
_systemstats_v6_load_systemstats(netsnmp_container* container, u_int load_flags)
{
    netsnmp_systemstats_entry *entry = NULL;
    uint64_t ipstat[IP6_NSTATS];
    size_t len = sizeof(ipstat);

    if (sysctlbyname("net.inet6.ip6.stats", &ipstat, &len, NULL, 0) < 0) {
	NETSNMP_LOGONCE((LOG_ERR, "Cannot sysctlbyname net.inet6.ip6.stats\n"));
	return -2;
    }

    DEBUGMSGTL(("access:systemstats:container:arch", "load v6 (flags %x)\n",
                load_flags));

    netsnmp_assert(container != NULL); /* load function shoulda checked this */

    entry = netsnmp_access_systemstats_entry_create(2, 0,
		"ipSystemStatsTable.ipv6");
    if(NULL == entry) {
	snmp_log(LOG_ERR, "systemstats_v6_load_systemstats: cannot create entry\n");
	netsnmp_access_systemstats_container_free(container,
						  NETSNMP_ACCESS_SYSTEMSTATS_FREE_NOFLAGS);
	return -3;
    }

    /*
     * OK - we've now got (or created) the data structure for
     *      this systemstats, including any "static" information.
     */

    entry->stats.HCInReceives.low = ipstat[IP6_STAT_TOTAL] & 0xffffffff;
    entry->stats.HCInReceives.high = ipstat[IP6_STAT_TOTAL] >> 32;
    entry->stats.InHdrErrors = ipstat[IP6_STAT_BADOPTIONS]
		    + ipstat[IP6_STAT_TOOSHORT] + ipstat[IP6_STAT_TOOSMALL]
		    + ipstat[IP6_STAT_TOOMANYHDR] + ipstat[IP6_STAT_EXTHDRTOOLONG];
    entry->stats.InAddrErrors = ipstat[IP6_STAT_CANTFORWARD];
    entry->stats.HCOutForwDatagrams.low = ipstat[IP6_STAT_FORWARD] & 0xffffffff;
    entry->stats.HCOutForwDatagrams.high = ipstat[IP6_STAT_FORWARD] >> 32;
    entry->stats.InDiscards = ipstat[IP6_STAT_FRAGDROPPED];
    entry->stats.HCInDelivers.low = ipstat[IP6_STAT_DELIVERED] & 0xffffffff;
    entry->stats.HCInDelivers.high = ipstat[IP6_STAT_DELIVERED] >> 32;
    entry->stats.HCOutRequests.low = ipstat[IP6_STAT_LOCALOUT] & 0xffffffff;
    entry->stats.HCOutRequests.high = ipstat[IP6_STAT_LOCALOUT] >> 32;
    entry->stats.HCOutDiscards.low = ipstat[IP6_STAT_ODROPPED] & 0xffffffff;
    entry->stats.HCOutDiscards.high = ipstat[IP6_STAT_ODROPPED] >> 32;
    entry->stats.HCOutNoRoutes.low = ipstat[IP6_STAT_NOGIF] & 0xffffffff;
    entry->stats.HCOutNoRoutes.high = ipstat[IP6_STAT_NOGIF] >> 32;
    /* entry->stats. = scan_vals[12]; / * ReasmTimeout */
    entry->stats.ReasmReqds = ipstat[IP6_STAT_FRAGMENTS];
    entry->stats.ReasmOKs = ipstat[IP6_STAT_REASSEMBLED];
    entry->stats.ReasmFails = ipstat[IP6_STAT_FRAGDROPPED]
		    + ipstat[IP6_STAT_FRAGTIMEOUT];
    entry->stats.HCOutFragOKs.low = ipstat[IP6_STAT_FRAGMENTS] & 0xffffffff;
    entry->stats.HCOutFragOKs.high = ipstat[IP6_STAT_FRAGMENTS] >> 32;
    entry->stats.HCOutFragFails.low = ipstat[IP6_STAT_CANTFRAG] & 0xffffffff;
    entry->stats.HCOutFragFails.high = ipstat[IP6_STAT_CANTFRAG] >> 32;
    entry->stats.HCOutFragCreates.low = ipstat[IP6_STAT_OFRAGMENTS] & 0xffffffff;
    entry->stats.HCOutFragCreates.high = ipstat[IP6_STAT_OFRAGMENTS] >> 32;

    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_HCINRECEIVES] = 1;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_INHDRERRORS] = 1;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_INADDRERRORS] = 1;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_HCOUTFORWDATAGRAMS] = 1;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_INDISCARDS] = 1;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_HCINDELIVERS] = 1;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_HCOUTREQUESTS] = 1;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_HCOUTDISCARDS] = 1;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_HCOUTNOROUTES] = 1;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_REASMREQDS] = 1;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_REASMOKS] = 1;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_REASMFAILS] = 1;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_HCOUTFRAGOKS] = 1;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_HCOUTFRAGFAILS] = 1;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_HCOUTFRAGCREATES] = 1;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_DISCONTINUITYTIME] = 1;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_REFRESHRATE] = 1;

    /*
     * add to container
     */
    if (CONTAINER_INSERT(container, entry) < 0)
    {
	snmp_log(LOG_ERR, "systemstats_v6_load_systemstats: cannot insert entry\n");
	DEBUGMSGTL(("access:systemstats:container","error with systemstats_entry: insert into container failed.\n"));
	netsnmp_access_systemstats_entry_free(entry);
    }

    return 0;
}


/*
 * load ipIfStatsTable for ipv6 entries
 */
static int 
_systemstats_v6_load_ifstats(netsnmp_container* container, u_int load_flags)
{
    struct if_nameindex *ifs = if_nameindex();
    int ix;
    int rc = 0;

    for (ix = 0; ifs[ix].if_index; ix++) {
	struct in6_ifstat *ifs6;
	struct in6_ifreq ifr;
	int s;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifs[ix].if_name, sizeof(ifr.ifr_name)-1);
	if ((s = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
	    rc = -1;
	    break;
	}
	if (ioctl(s, SIOCGIFSTAT_IN6, (caddr_t)&ifr) < 0) {
	    rc = -2;
	    close(s);
	    break;
	}
	close(s);
	ifs6 = &ifr.ifr_ifru.ifru_stat;
    }
    if_freenameindex(ifs);
    return rc;
}
#endif

#else

static int
_systemstats_v4(netsnmp_container* container, u_int load_flags)
{
    netsnmp_systemstats_entry *entry = NULL;
#ifdef __DragonFly__
    size_t len = ncpus*sizeof(struct ip_stats);
    struct ip_stats *ipstat = malloc(len);
    int c;
#else
    size_t len = sizeof(struct ipstat);
    struct ipstat *ipstat = malloc(len);
#endif
    int mib[] = { CTL_NET, PF_INET, IPPROTO_IP, IPCTL_STATS };

    if (sysctl(mib, sizeof(mib) / sizeof(mib[0]), ipstat, &len, NULL, 0) == -1) {
	snmp_log_perror("Cannot sysctlbyname net.inet.ip.stats");
	free(ipstat);
	return -2;
    }

    DEBUGMSGTL(("access:systemstats:container:arch", "load v4 (flags %x)\n",
                load_flags));

    netsnmp_assert(container != NULL); /* load function shoulda checked this */

    if (load_flags & NETSNMP_ACCESS_SYSTEMSTATS_LOAD_IFTABLE) {
        /* we do not support ipIfStatsTable for ipv4 */
	free(ipstat);
        return 0;
    }

    entry = netsnmp_access_systemstats_entry_create(1, 0,
		"ipSystemStatsTable.ipv4");
    if(NULL == entry) {
	netsnmp_access_systemstats_container_free(container,
						  NETSNMP_ACCESS_SYSTEMSTATS_FREE_NOFLAGS);
	free(ipstat);
	return -3;
    }

    /*
     * OK - we've now got (or created) the data structure for
     *      this systemstats, including any "static" information.
     * Now parse the rest of the line (i.e. starting from 'stats')
     *      to extract the relevant statistics, and populate
     *      data structure accordingly.
     */

#ifdef dragonfly
    for (c = 1; c < ncpus; c++) {
	int i, n = sizeof(struct ip_stats)/sizeof(u_long);
	u_long *up = (u_long *)ipstat;
	u_long *cp = (u_long *)(ipstat+c);
	for (i = 0; i < n; i++) {
	    *up += *cp;
	    up++;
	    cp++;
	}
    }
#endif
    entry->stats.HCInReceives.low = ipstat->ips_total & 0xffffffff;
#ifndef darwin
    entry->stats.HCInReceives.high = ipstat->ips_total >> 32;
#endif
    entry->stats.InHdrErrors = ipstat->ips_badsum + ipstat->ips_tooshort
		            + ipstat->ips_toosmall + ipstat->ips_badhlen
			    + ipstat->ips_badlen;
    entry->stats.InAddrErrors = ipstat->ips_cantforward;
    entry->stats.HCOutForwDatagrams.low = ipstat->ips_forward & 0xffffffff;
#ifndef darwin
    entry->stats.HCOutForwDatagrams.high = ipstat->ips_forward >> 32;
#endif
    entry->stats.InUnknownProtos = ipstat->ips_noproto;
    entry->stats.InDiscards = ipstat->ips_fragdropped;
    entry->stats.HCInDelivers.low = ipstat->ips_delivered & 0xffffffff;
#ifndef darwin
    entry->stats.HCInDelivers.high = ipstat->ips_delivered >> 32;
#endif
    entry->stats.HCOutRequests.low = ipstat->ips_localout & 0xffffffff;
#ifndef darwin
    entry->stats.HCOutRequests.high = ipstat->ips_localout >> 32;
#endif
    entry->stats.HCOutDiscards.low = ipstat->ips_odropped & 0xffffffff;
#ifndef darwin
    entry->stats.HCOutDiscards.high = ipstat->ips_odropped >> 32;
#endif
    entry->stats.HCOutNoRoutes.low = ipstat->ips_nogif & 0xffffffff;
#ifndef darwin
    entry->stats.HCOutNoRoutes.high = ipstat->ips_nogif >> 32;
#endif
    /* entry->stats. = scan_vals[12]; / * ReasmTimeout */
    entry->stats.ReasmReqds = ipstat->ips_fragments;
    entry->stats.ReasmOKs = ipstat->ips_reassembled;
    entry->stats.ReasmFails = ipstat->ips_fragdropped + ipstat->ips_fragtimeout;
    entry->stats.HCOutFragOKs.low = ipstat->ips_fragments & 0xffffffff;
#ifndef darwin
    entry->stats.HCOutFragOKs.high = ipstat->ips_fragments >> 32;
#endif
    entry->stats.HCOutFragFails.low = ipstat->ips_cantfrag & 0xffffffff;
#ifndef darwin
    entry->stats.HCOutFragFails.high = ipstat->ips_cantfrag >> 32;
#endif
    entry->stats.HCOutFragCreates.low = ipstat->ips_ofragments & 0xffffffff;
#ifndef darwin
    entry->stats.HCOutFragCreates.high = ipstat->ips_ofragments >> 32;
#endif

    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_HCINRECEIVES] = 1;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_INHDRERRORS] = 1;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_INADDRERRORS] = 1;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_HCOUTFORWDATAGRAMS] = 1;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_INUNKNOWNPROTOS] = 1;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_INDISCARDS] = 1;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_HCINDELIVERS] = 1;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_HCOUTREQUESTS] = 1;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_HCOUTDISCARDS] = 1;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_HCOUTNOROUTES] = 1;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_REASMREQDS] = 1;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_REASMOKS] = 1;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_REASMFAILS] = 1;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_HCOUTFRAGOKS] = 1;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_HCOUTFRAGFAILS] = 1;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_HCOUTFRAGCREATES] = 1;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_DISCONTINUITYTIME] = 1;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_REFRESHRATE] = 1;

    /*
     * add to container
     */
    if (CONTAINER_INSERT(container, entry) < 0)
    {
	DEBUGMSGTL(("access:systemstats:container","error with systemstats_entry: insert into container failed.\n"));
	netsnmp_access_systemstats_entry_free(entry);
    }

    free(ipstat);
    return 0;
}


#if defined (NETSNMP_ENABLE_IPV6)

/*
 * load ipSystemStatsTable for ipv6 entries
 */
static int 
_systemstats_v6_load_systemstats(netsnmp_container* container, u_int load_flags)
{
    struct ip6stat ip6stat;
    int mib[] = { CTL_NET, AF_INET6, IPPROTO_IPV6, IPV6CTL_STATS };
    size_t len = sizeof(ip6stat);
    netsnmp_systemstats_entry *entry = NULL;

    if (sysctl(mib, sizeof(mib) / sizeof(mib[0]), &ip6stat, &len, NULL, 0) == -1) {
  	NETSNMP_LOGONCE((LOG_ERR, "Cannot sysctl(CTL_NET, AF_INET6, IPPROTO_IPV6, IPV6CTL_STATS)\n"));
	return -1;
    }
    
    entry = netsnmp_access_systemstats_entry_create(2, 0,
            "ipSystemStatsTable.ipv6");
    if(NULL == entry)
        return -3;
    
    entry->stats.HCInReceives.low = ip6stat.ip6s_total & 0xffffffff;
    entry->stats.HCInReceives.high = ip6stat.ip6s_total >> 32;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_HCINRECEIVES] = 1;
    /*
    entry->stats.HCInOctets.low = scan_val & 0xffffffff;
    entry->stats.HCInOctets.high = scan_val  >> 32;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_HCINOCTETS] = 1;
    */
    entry->stats.InHdrErrors = ip6stat.ip6s_badoptions + ip6stat.ip6s_tooshort
                             + ip6stat.ip6s_toosmall + ip6stat.ip6s_badvers
			     + ip6stat.ip6s_toomanyhdr;
#if HAVE_STRUCT_IP6STAT_IP6S_EXTHDRTOOLONG
    entry->stats.InHdrErrors += ip6stat.ip6s_exthdrtoolong;
#endif
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_INHDRERRORS] = 1;
    entry->stats.HCInNoRoutes.low = ip6stat.ip6s_cantforward & 0xffffffff;
    entry->stats.HCInNoRoutes.high = ip6stat.ip6s_cantforward >> 32;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_HCINNOROUTES] = 1;
    /*
    entry->stats.inAddrErrors = 0;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_INADDRERRORS] = 1;
    entry->stats.InUnknownProtos = scan_val;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_INUNKNOWNPROTOS] = 1;
    entry->stats.InTruncatedPkts = scan_val  & 0xffffffff;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_INTRUNCATEDPKTS] = 1;
    */
    entry->stats.HCInForwDatagrams.low = ip6stat.ip6s_forward & 0xffffffff;
    entry->stats.HCInForwDatagrams.high = ip6stat.ip6s_forward >> 32;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_HCINFORWDATAGRAMS] = 1;
    entry->stats.ReasmReqds = ip6stat.ip6s_fragments;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_REASMREQDS] = 1;
    entry->stats.ReasmOKs = ip6stat.ip6s_reassembled;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_REASMOKS] = 1;
    entry->stats.ReasmFails = ip6stat.ip6s_fragdropped + ip6stat.ip6s_fragtimeout;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_REASMFAILS] = 1;
    entry->stats.InDiscards = ip6stat.ip6s_fragdropped;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_INDISCARDS] = 1;
    entry->stats.HCInDelivers.low = ip6stat.ip6s_delivered  & 0xffffffff;
    entry->stats.HCInDelivers.high = ip6stat.ip6s_delivered >> 32;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_HCINDELIVERS] = 1;
    entry->stats.HCOutRequests.low = ip6stat.ip6s_localout & 0xffffffff;
    entry->stats.HCOutRequests.high = ip6stat.ip6s_localout >> 32;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_HCOUTREQUESTS] = 1;
    entry->stats.HCOutNoRoutes.low = ip6stat.ip6s_noroute & 0xffffffff;
    entry->stats.HCOutNoRoutes.high = ip6stat.ip6s_noroute >> 32;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_HCOUTNOROUTES] = 1;
    entry->stats.HCOutForwDatagrams.low = ip6stat.ip6s_forward & 0xffffffff;
    entry->stats.HCOutForwDatagrams.high = ip6stat.ip6s_forward >> 32;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_HCOUTFORWDATAGRAMS] = 1;
    entry->stats.HCOutDiscards.low = ip6stat.ip6s_odropped & 0xffffffff;
    entry->stats.HCOutDiscards.high = ip6stat.ip6s_odropped >> 32;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_HCOUTDISCARDS] = 1;
    entry->stats.HCOutFragReqds.low = (ip6stat.ip6s_fragmented + ip6stat.ip6s_cantfrag) & 0xffffffff;
    entry->stats.HCOutFragReqds.high = (ip6stat.ip6s_fragmented + ip6stat.ip6s_cantfrag) >> 32;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_HCOUTFRAGREQDS] = 1;
    entry->stats.HCOutFragOKs.low = ip6stat.ip6s_fragmented & 0xffffffff;
    entry->stats.HCOutFragOKs.high = ip6stat.ip6s_fragmented >> 32;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_HCOUTFRAGOKS] = 1;
    entry->stats.HCOutFragFails.low = ip6stat.ip6s_cantfrag & 0xffffffff;
    entry->stats.HCOutFragFails.high = ip6stat.ip6s_cantfrag >> 32;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_HCOUTFRAGFAILS] = 1;
    entry->stats.HCOutFragCreates.low = ip6stat.ip6s_ofragments & 0xffffffff;
    entry->stats.HCOutFragCreates.high = ip6stat.ip6s_ofragments >> 32;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_HCOUTFRAGCREATES] = 1;
    /*
    entry->stats.HCOutTransmits.low = scan_val & 0xffffffff;
    entry->stats.HCOutTransmits.high = scan_val >> 32;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_HCOUTTRANSMITS] = 1;
    entry->stats.HCOutMcastOctets.low = scan_val & 0xffffffff;
    entry->stats.HCOutMcastOctets.high = scan_val >> 32;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_HCOUTMCASTOCTETS] = 1;
    entry->stats.HCInMcastPkts.low = scan_val  & 0xffffffff;
    entry->stats.HCInMcastPkts.high = scan_val >> 32;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_HCINMCASTPKTS] = 1;
    entry->stats.HCInMcastOctets.low = scan_val  & 0xffffffff;
    entry->stats.HCInMcastOctets.high = scan_val >> 32;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_HCINMCASTOCTETS] = 1;
    entry->stats.HCOutMcastPkts.low = scan_val & 0xffffffff;
    entry->stats.HCOutMcastPkts.high = scan_val >> 32;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_HCOUTMCASTPKTS] = 1;
    entry->stats.HCOutOctets.low = scan_val & 0xffffffff;
    entry->stats.HCOutOctets.high = scan_val >> 32;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_HCOUTOCTETS] = 1;
    entry->stats.HCInBcastPkts.low = scan_val  & 0xffffffff;
    entry->stats.HCInBcastPkts.high = scan_val >> 32;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_HCINBCASTPKTS] = 1;
    entry->stats.HCOutBcastPkts.low = scan_val  & 0xffffffff;
    entry->stats.HCOutBcastPkts.high = scan_val >> 32;
    entry->stats.columnAvail[IPSYSTEMSTATSTABLE_HCOUTBCASTPKTS] = 1;
    */

    /*
     * add to container
     */
    if (CONTAINER_INSERT(container, entry) < 0) {
	DEBUGMSGTL(("access:systemstats:container","error with systemstats_entry: insert into container failed.\n"));
	netsnmp_access_systemstats_entry_free(entry);
    }

    return 1;
}

#define DEV_SNMP6_DIRNAME   "/proc/net/dev_snmp6"
#define IFINDEX_LINE        "ifIndex"
#define DEV_FILENAME_LEN    64

/*
 * load ipIfStatsTable for ipv6 entries
 */
static int 
_systemstats_v6_load_ifstats(netsnmp_container* container, u_int load_flags)
{
    DIR            *dev_snmp6_dir;
    struct dirent  *dev_snmp6_entry;
    char           dev_filename[DEV_FILENAME_LEN];
    FILE           *devin;
    char           line[1024];
    char           *start = line;
    int            rc;
    char           *scan_str;
    uintmax_t       scan_val;
    netsnmp_systemstats_entry *entry = NULL;
            
    /*
     * try to open /proc/net/dev_snmp6 directory. If we can't, that' ok -
     * maybe it is not supported by the current running kernel.
     */
    if ((dev_snmp6_dir = opendir(DEV_SNMP6_DIRNAME)) == NULL) {
        DEBUGMSGTL(("access:ifstats",
        "Failed to load IPv6 IfStats Table (linux)\n"));
        return 0;
    }
    
    /*
     * Read each per interface statistics proc file
     */
    rc = 0;
    while ((dev_snmp6_entry = readdir(dev_snmp6_dir)) != NULL) {
        if (dev_snmp6_entry->d_name[0] == '.')
            continue;
    
        if (snprintf(dev_filename, DEV_FILENAME_LEN, "%s/%s", DEV_SNMP6_DIRNAME,
                dev_snmp6_entry->d_name) > DEV_FILENAME_LEN) {
            snmp_log(LOG_ERR, "Interface name %s is too long\n",
                    dev_snmp6_entry->d_name);
            continue;
        }
        if (NULL == (devin = fopen(dev_filename, "r"))) {
            snmp_log(LOG_ERR, "Failed to open %s\n", dev_filename);
            continue;
        }
    
        /*
         * If a stat file name is made of digits, the name is interface index.
         * If it is an interface name, the file includes a line labeled ifIndex.
         */
        if (isdigit(dev_snmp6_entry->d_name[0])) {
            scan_val = strtoull(dev_snmp6_entry->d_name, NULL, 0);
        } else {
            if (NULL == (start = fgets(line, sizeof(line), devin))) {
                snmp_log(LOG_ERR, "%s doesn't include any lines\n",
                        dev_filename);
                fclose(devin);
                continue;
            }
    
            if (0 != strncmp(start, IFINDEX_LINE, 7)) {
                snmp_log(LOG_ERR, "%s doesn't include ifIndex line",
                        dev_filename);
                fclose(devin);
                continue;
            }

            scan_str = strrchr(line, ' ');
            if (NULL == scan_str) {
                snmp_log(LOG_ERR, "%s is wrong format", dev_filename);
                fclose(devin);
                continue;
            }
            scan_val = strtoull(scan_str, NULL, 0);
        }
        
        entry = netsnmp_access_systemstats_entry_create(2, scan_val,
                "ipIfStatsTable.ipv6");
        if(NULL == entry) {
            fclose(devin);
            closedir(dev_snmp6_dir);
            return -3;
        }
        
        /* _systemstats_v6_load_file(entry, devin); */
        CONTAINER_INSERT(container, entry);
        fclose(devin);
    }
    closedir(dev_snmp6_dir);
    return 0;
}
#endif /* NETSNMP_ENABLE_IPV6 */
#endif

#ifdef NETSNMP_ENABLE_IPV6
/*
 * Based on load_flags, load ipSystemStatsTable or ipIfStatsTable for ipv6 entries. 
 */
static int
_systemstats_v6(netsnmp_container* container, u_int load_flags)
{
    DEBUGMSGTL(("access:systemstats:container:arch", "load v6 (flags %u)\n",
                load_flags));

    netsnmp_assert(container != NULL); /* load function shoulda checked this */

    if (load_flags & NETSNMP_ACCESS_SYSTEMSTATS_LOAD_IFTABLE) {
        /* load ipIfStatsTable */
        return _systemstats_v6_load_ifstats(container, load_flags);
    } else {
        /* load ipSystemStatsTable */
        return _systemstats_v6_load_systemstats(container, load_flags);
    }
}
#endif /* NETSNMP_ENABLE_IPV6 */
