/*
 * NetBSD implementation for mapping the IP stat arrays into struct's
 * Required for NetBSD versions produced after April 7th 2008 (4+)
 *
 * Based on: http://mail-index.netbsd.org/pkgsrc-users/2008/04/27/msg007095.html
 */

#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/protosw.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip_var.h>
#include <netinet/icmp_var.h>
#include <netinet/icmp6.h>
#include <netinet/tcp.h>
#include <netinet/tcp_timer.h>
#include <netinet/tcp_var.h>
#include <netinet/udp.h>
#include <netinet/udp_var.h>

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "kernel_netbsd.h"

#if defined(NETBSD_STATS_VIA_SYSCTL)

int
netbsd_read_icmp_stat(struct icmp_mib *mib)
{
    uint64_t icmpstat[ICMP_NSTATS];
    size_t   size = sizeof(icmpstat);
    int      i;

    (void)memset(mib, 0, sizeof(*mib));

    if (-1 == sysctlbyname("net.inet.icmp.stats", icmpstat, &size, NULL, 0)) {
	snmp_perror("netbsd_read_icmp_stat: net.inet.icmp.stats");
        return -1;
    }

    mib->icmpInMsgs = icmpstat[ICMP_STAT_BADCODE]
        + icmpstat[ICMP_STAT_TOOSHORT]
        + icmpstat[ICMP_STAT_CHECKSUM]
        + icmpstat[ICMP_STAT_BADLEN];
    for (i = 0; i <= ICMP_MAXTYPE; i++)
        mib->icmpInMsgs  += icmpstat[ICMP_STAT_INHIST + i];
    mib->icmpInErrors = icmpstat[ICMP_STAT_BADCODE]
        + icmpstat[ICMP_STAT_TOOSHORT]
        + icmpstat[ICMP_STAT_CHECKSUM]
        + icmpstat[ICMP_STAT_BADLEN];
    mib->icmpInDestUnreachs = icmpstat[ICMP_STAT_INHIST + ICMP_UNREACH];
    mib->icmpInTimeExcds = icmpstat[ICMP_STAT_INHIST + ICMP_TIMXCEED];
    mib->icmpInParmProbs = icmpstat[ICMP_STAT_INHIST + ICMP_PARAMPROB];
    mib->icmpInSrcQuenchs = icmpstat[ICMP_STAT_INHIST + ICMP_SOURCEQUENCH];
    mib->icmpInRedirects = icmpstat[ICMP_STAT_INHIST + ICMP_REDIRECT];
    mib->icmpInEchos = icmpstat[ICMP_STAT_INHIST + ICMP_ECHO];
    mib->icmpInEchoReps = icmpstat[ICMP_STAT_INHIST + ICMP_ECHOREPLY];
    mib->icmpInTimestamps = icmpstat[ICMP_STAT_INHIST + ICMP_TSTAMP];
    mib->icmpInTimestampReps
        = icmpstat[ICMP_STAT_INHIST + ICMP_TSTAMPREPLY];
    mib->icmpInAddrMasks = icmpstat[ICMP_STAT_INHIST + ICMP_MASKREQ];
    mib->icmpInAddrMaskReps = icmpstat[ICMP_STAT_INHIST + ICMP_MASKREPLY];
    mib->icmpOutMsgs = icmpstat[ICMP_STAT_OLDSHORT]
        + icmpstat[ICMP_STAT_OLDICMP];
    for (i = 0; i <= ICMP_MAXTYPE; i++)
        mib->icmpOutMsgs += icmpstat[ICMP_STAT_OUTHIST + i];
    mib->icmpOutErrors = icmpstat[ICMP_STAT_OLDSHORT]
        + icmpstat[ICMP_STAT_OLDICMP];
    mib->icmpOutDestUnreachs = icmpstat[ICMP_STAT_OUTHIST + ICMP_UNREACH];
    mib->icmpOutTimeExcds = icmpstat[ICMP_STAT_OUTHIST + ICMP_TIMXCEED];
    mib->icmpOutParmProbs = icmpstat[ICMP_STAT_OUTHIST + ICMP_PARAMPROB];
    mib->icmpOutSrcQuenchs
        = icmpstat[ICMP_STAT_OUTHIST + ICMP_SOURCEQUENCH];
    mib->icmpOutRedirects = icmpstat[ICMP_STAT_OUTHIST + ICMP_REDIRECT];
    mib->icmpOutEchos = icmpstat[ICMP_STAT_OUTHIST + ICMP_ECHO];
    mib->icmpOutEchoReps = icmpstat[ICMP_STAT_OUTHIST + ICMP_ECHOREPLY];
    mib->icmpOutTimestamps = icmpstat[ICMP_STAT_OUTHIST + ICMP_TSTAMP];
    mib->icmpOutTimestampReps
        = icmpstat[ICMP_STAT_OUTHIST + ICMP_TSTAMPREPLY];
    mib->icmpOutAddrMasks = icmpstat[ICMP_STAT_OUTHIST + ICMP_MASKREQ];
    mib->icmpOutAddrMaskReps = icmpstat[ICMP_STAT_OUTHIST + ICMP_MASKREPLY];

    return 0;
}


int
netbsd_read_icmp6_stat(struct icmp6_mib *mib)
{
    uint64_t icmpstat[ICMP6_NSTATS];
    size_t   size = sizeof(icmpstat);
    int      i;

    (void)memset(mib, 0, sizeof(*mib));

    if (-1 == sysctlbyname("net.inet6.icmp6.stats", icmpstat, &size, NULL, 0)) {
	snmp_perror("netbsd_read_icmp6_stat: net.inet6.icmp6.stats");
        return -1;
    }

    mib->icmp6InMsgs = icmpstat[ICMP6_STAT_BADCODE]
            + icmpstat[ICMP_STAT_TOOSHORT]
	    + icmpstat[ICMP_STAT_CHECKSUM]
            + icmpstat[ICMP_STAT_BADLEN];
    for (i = 0; i <= ICMP6_MAXTYPE; i++)
        mib->icmp6InMsgs  += icmpstat[ICMP6_STAT_INHIST + i];
    mib->icmp6InErrors = icmpstat[ICMP6_STAT_BADCODE]
        + icmpstat[ICMP6_STAT_TOOSHORT]
        + icmpstat[ICMP6_STAT_CHECKSUM]
        + icmpstat[ICMP6_STAT_BADLEN];
    mib->icmp6InDestUnreachs = icmpstat[ICMP6_STAT_INHIST + ICMP6_DST_UNREACH];
    mib->icmp6InPktTooBigs = icmpstat[ICMP6_STAT_INHIST + ICMP6_PACKET_TOO_BIG];
    mib->icmp6InTimeExcds = icmpstat[ICMP6_STAT_INHIST + ICMP6_TIME_EXCEEDED];
    mib->icmp6InParmProblems = icmpstat[ICMP6_STAT_INHIST + ICMP6_PARAM_PROB];
    mib->icmp6InEchos = icmpstat[ICMP6_STAT_INHIST + ICMP6_ECHO_REQUEST];
    mib->icmp6InEchoReplies = icmpstat[ICMP6_STAT_INHIST + ICMP6_ECHO_REPLY];
    mib->icmp6InGroupMembQueries = icmpstat[ICMP6_STAT_INHIST + MLD_LISTENER_QUERY];
    mib->icmp6InGroupMembResponses = icmpstat[ICMP6_STAT_INHIST + MLD_LISTENER_REPORT];
    mib->icmp6InRouterSolicits = icmpstat[ICMP6_STAT_INHIST + ND_ROUTER_SOLICIT];
    mib->icmp6InRouterAdvertisements = icmpstat[ICMP6_STAT_INHIST + ND_ROUTER_ADVERT];
    mib->icmp6InNeighborSolicits = icmpstat[ICMP6_STAT_INHIST + ND_NEIGHBOR_SOLICIT];
    mib->icmp6InNeighborAdvertisements = icmpstat[ICMP6_STAT_INHIST + ND_NEIGHBOR_ADVERT];
    mib->icmp6InRedirects = icmpstat[ICMP6_STAT_INHIST + ND_REDIRECT];

    mib->icmp6OutMsgs = icmpstat[ICMP6_STAT_BADCODE]
        + icmpstat[ICMP6_STAT_TOOSHORT]
        + icmpstat[ICMP6_STAT_CHECKSUM]
        + icmpstat[ICMP6_STAT_BADLEN];
    for (i = 0; i <= ICMP6_MAXTYPE; i++)
        mib->icmp6OutMsgs += icmpstat[ICMP6_STAT_OUTHIST + i];
    mib->icmp6OutDestUnreachs = icmpstat[ICMP6_STAT_OUTHIST + ICMP6_DST_UNREACH];
    mib->icmp6OutPktTooBigs =  icmpstat[ICMP6_STAT_OUTHIST + ICMP6_PACKET_TOO_BIG];
    mib->icmp6OutTimeExcds = icmpstat[ICMP6_STAT_OUTHIST + ICMP6_TIME_EXCEEDED];
    mib->icmp6OutParmProblems = icmpstat[ICMP6_STAT_OUTHIST + ICMP6_PARAM_PROB];
    mib->icmp6OutEchos = icmpstat[ICMP6_STAT_OUTHIST + ICMP6_ECHO_REQUEST];
    mib->icmp6OutEchoReplies = icmpstat[ICMP6_STAT_OUTHIST + ICMP6_ECHO_REPLY];
    mib->icmp6OutRouterSolicits =  icmpstat[ICMP6_STAT_OUTHIST + ND_ROUTER_SOLICIT];
    mib->icmp6OutNeighborSolicits =  icmpstat[ICMP6_STAT_OUTHIST + ND_NEIGHBOR_SOLICIT];
    mib->icmp6OutNeighborAdvertisements =  icmpstat[ICMP6_STAT_OUTHIST + ND_NEIGHBOR_ADVERT];
    mib->icmp6OutRedirects = icmpstat[ICMP6_STAT_OUTHIST + ND_REDIRECT];
    mib->icmp6OutGroupMembResponses =  icmpstat[ICMP6_STAT_OUTHIST + MLD_LISTENER_REPORT];
    mib->icmp6OutGroupMembReductions =  icmpstat[ICMP6_STAT_OUTHIST + MLD_LISTENER_DONE];

    return 0;
}


int
netbsd_read_icmp_msg_stat(struct icmp_mib *mib,
                          struct icmp4_msg_mib *msgmib,
			  int *flag)
{
    netbsd_read_icmp_stat(mib);
    *flag = 0;
    return 0;
}


int
netbsd_read_icmp6_msg_stat(struct icmp6_mib *mib,
                           struct icmp6_msg_mib *msgmib,
			   int *support)
{
    uint64_t icmpstat[ICMP6_NSTATS];
    size_t   size = sizeof(icmpstat);
    int      i;

    netbsd_read_icmp6_stat(mib);

    if (-1 == sysctlbyname("net.inet6.icmp6.stats", icmpstat, &size, NULL, 0)) {
	snmp_perror("netbsd_read_icmp6_stat: net.inet6.icmp6.stats");
        return -1;
    }

    for (i = 0; i < 256; i++) {
	msgmib->vals[i].InType = icmpstat[ICMP6_STAT_INHIST+i];
	msgmib->vals[i].OutType = icmpstat[ICMP6_STAT_OUTHIST+i];
    }
    *support = 1;
    return 0;
}


int
netbsd_read_ip_stat(struct ip_mib *mib)
{
    uint64_t ipstat[IP_NSTATS];
    size_t   size = sizeof(ipstat);
    int      i;
    static   int sname[4] = { 4, 2, 0, 0 }; /* CTL_NET, PF_INET, IPPROTO_IP, 0 */
    size_t   len;

    (void)memset(mib, 0, sizeof(*mib));

    if (-1 == sysctlbyname("net.inet.ip.stats", ipstat, &size, NULL, 0)) {
	snmp_perror("netbsd_read_ip_stat: net.inet.ip.stats");
        return -1;
    }

    mib->ipForwarding = 0;
    len = sizeof i;
    sname[3] = IPCTL_FORWARDING;
    if (0 == sysctl(sname, 4, &i, &len, 0, 0)) {
        mib->ipForwarding = (long)i;
    }

    mib->ipDefaultTTL = 0;
    sname[3] = IPCTL_DEFTTL;     
    if (0 == sysctl(sname, 4, &i, &len, 0, 0)) {
        mib->ipDefaultTTL = (long)i;
    }

    mib->ipInReceives = ipstat[IP_STAT_TOTAL];
    mib->ipInHdrErrors = ipstat[IP_STAT_BADSUM]
        + ipstat[IP_STAT_TOOSHORT] + ipstat[IP_STAT_TOOSMALL]
        + ipstat[IP_STAT_BADHLEN] + ipstat[IP_STAT_BADLEN];
    mib->ipInAddrErrors = ipstat[IP_STAT_CANTFORWARD];
    mib->ipForwDatagrams = ipstat[IP_STAT_FORWARD];
    mib->ipInUnknownProtos = ipstat[IP_STAT_NOPROTO];
    mib->ipInDiscards = ipstat[IP_STAT_FRAGDROPPED]; /* FIXME */
    mib->ipInDelivers = ipstat[IP_STAT_DELIVERED];
    mib->ipOutRequests = ipstat[IP_STAT_LOCALOUT];
    mib->ipOutDiscards = ipstat[IP_STAT_ODROPPED];
    mib->ipOutNoRoutes = 0; /* FIXME */
    mib->ipReasmTimeout = 0; /* IPFRAGTTL; */
    mib->ipReasmReqds = ipstat[IP_STAT_FRAGMENTS];
    mib->ipReasmOKs = ipstat[IP_STAT_REASSEMBLED];
    mib->ipReasmFails = ipstat[IP_STAT_FRAGDROPPED]
        + ipstat[IP_STAT_FRAGTIMEOUT];
    mib->ipFragOKs = ipstat[IP_STAT_FRAGMENTS];
    mib->ipFragFails = ipstat[IP_STAT_CANTFRAG];
    mib->ipFragCreates = ipstat[IP_STAT_OFRAGMENTS];
    mib->ipRoutingDiscards = ipstat[IP_STAT_NOROUTE];
    
    return 0;
}


int
netbsd_read_tcp_stat(struct tcp_mib *mib)
{
    uint64_t tcpstat[TCP_NSTATS];
    size_t   size = sizeof(tcpstat);

    (void)memset(mib, 0, sizeof(*mib));

    if (-1 == sysctlbyname("net.inet.tcp.stats", tcpstat, &size, NULL, 0)) {
	snmp_perror("netbsd_read_tcp_stat: net.inet.tcp.stats");
        return -1;
    }

    mib->tcpRtoAlgorithm = 4; /* Assume Van Jacobsen's algorithm */
    mib->tcpRtoMin = TCPTV_MIN;
    mib->tcpRtoMax = TCPTV_REXMTMAX;
    mib->tcpMaxConn = -1; /* Dynamic Maximum */
    mib->tcpActiveOpens = tcpstat[TCP_STAT_CONNATTEMPT];
    mib->tcpPassiveOpens = tcpstat[TCP_STAT_ACCEPTS];
    mib->tcpAttemptFails = tcpstat[TCP_STAT_CONNDROPS];
    mib->tcpEstabResets = tcpstat[TCP_STAT_DROPS];
    mib->tcpCurrEstab = 0; /* FIXME */
    mib->tcpInSegs = tcpstat[TCP_STAT_RCVTOTAL];
    mib->tcpOutSegs = tcpstat[TCP_STAT_SNDTOTAL]
        - tcpstat[TCP_STAT_SNDREXMITPACK];
    mib->tcpRetransSegs = tcpstat[TCP_STAT_SNDREXMITPACK];
    mib->tcpInErrs = tcpstat[TCP_STAT_RCVBADSUM]
        + tcpstat[TCP_STAT_RCVBADOFF]
        + tcpstat[TCP_STAT_RCVMEMDROP]
        + tcpstat[TCP_STAT_RCVSHORT];
    mib->tcpOutRsts = tcpstat[TCP_STAT_SNDCTRL]
        - tcpstat[TCP_STAT_CLOSED];
    mib->tcpInErrsValid = mib->tcpInErrs; /* FIXME */
    mib->tcpOutRstsValid = mib->tcpOutRsts; /* FIXME */

    return 0;
}


int
netbsd_read_udp_stat(struct udp_mib *mib)
{
    uint64_t udpstat[UDP_NSTATS];
    size_t   size = sizeof(udpstat);

    (void)memset(mib, 0, sizeof(*mib));

    if (-1 == sysctlbyname("net.inet.udp.stats", udpstat, &size, NULL, 0)) {
	snmp_perror("netbsd_read_udp_stat: net.inet.udp.stats");
        return -1;
    }

    mib->udpInDatagrams = udpstat[UDP_STAT_IPACKETS];
    mib->udpNoPorts = udpstat[UDP_STAT_NOPORT];
    mib->udpOutDatagrams = udpstat[UDP_STAT_OPACKETS];
    mib->udpInErrors = udpstat[UDP_STAT_HDROPS]
        + udpstat[UDP_STAT_BADSUM] /* + udpstat[UDP_STAT_DISCARD] */ /* FIXME */
        + udpstat[UDP_STAT_FULLSOCK] + udpstat[UDP_STAT_BADLEN];

    return 0;
}

#endif
