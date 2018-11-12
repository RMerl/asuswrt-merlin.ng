/*
 * sysctl interface for icmp stats for others than NetBSD
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

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "kernel_sysctl.h"

#if defined(NETSNMP_CAN_USE_SYSCTL)

int
sysctl_read_icmp_stat(struct icmp_mib *mib)
{
    struct icmpstat icmpstat;
    size_t   size = sizeof(icmpstat);
    int      i;
    static int      sname[4] =
        { CTL_NET, PF_INET, IPPROTO_ICMP, ICMPCTL_STATS };

    (void)memset(mib, 0, sizeof(*mib));

    if (-1 == sysctl(sname, 4, &icmpstat, &size, NULL, 0)) {
	snmp_perror("sysctl_read_icmp_stat: net.inet.icmp.stats");
        return -1;
    }

    mib->icmpInMsgs = icmpstat.icps_badcode
        + icmpstat.icps_tooshort
        + icmpstat.icps_checksum
        + icmpstat.icps_badlen;
    mib->icmpInErrors = mib->icmpInMsgs;
    for (i = 0; i <= ICMP_MAXTYPE; i++)
        mib->icmpInMsgs  += icmpstat.icps_inhist[i];
    mib->icmpInDestUnreachs = icmpstat.icps_inhist[ICMP_UNREACH];
    mib->icmpInTimeExcds = icmpstat.icps_inhist[ICMP_TIMXCEED];
    mib->icmpInParmProbs = icmpstat.icps_inhist[ICMP_PARAMPROB];
    mib->icmpInSrcQuenchs = icmpstat.icps_inhist[ICMP_SOURCEQUENCH];
    mib->icmpInRedirects = icmpstat.icps_inhist[ICMP_REDIRECT];
    mib->icmpInEchos = icmpstat.icps_inhist[ICMP_ECHO];
    mib->icmpInEchoReps = icmpstat.icps_inhist[ICMP_ECHOREPLY];
    mib->icmpInTimestamps = icmpstat.icps_inhist[ICMP_TSTAMP];
    mib->icmpInTimestampReps = icmpstat.icps_inhist[ICMP_TSTAMPREPLY];
    mib->icmpInAddrMasks = icmpstat.icps_inhist[ICMP_MASKREQ];
    mib->icmpInAddrMaskReps = icmpstat.icps_inhist[ICMP_MASKREPLY];
    mib->icmpOutMsgs = icmpstat.icps_oldshort + icmpstat.icps_oldicmp;
    for (i = 0; i <= ICMP_MAXTYPE; i++)
        mib->icmpOutMsgs += icmpstat.icps_outhist[i];
    mib->icmpOutErrors = icmpstat.icps_oldshort + icmpstat.icps_oldicmp;
    mib->icmpOutDestUnreachs = icmpstat.icps_outhist[ICMP_UNREACH];
    mib->icmpOutTimeExcds = icmpstat.icps_outhist[ICMP_TIMXCEED];
    mib->icmpOutParmProbs = icmpstat.icps_outhist[ICMP_PARAMPROB];
    mib->icmpOutSrcQuenchs = icmpstat.icps_outhist[ICMP_SOURCEQUENCH];
    mib->icmpOutRedirects = icmpstat.icps_outhist[ICMP_REDIRECT];
    mib->icmpOutEchos = icmpstat.icps_outhist[ICMP_ECHO];
    mib->icmpOutEchoReps = icmpstat.icps_outhist[ICMP_ECHOREPLY];
    mib->icmpOutTimestamps = icmpstat.icps_outhist[ICMP_TSTAMP];
    mib->icmpOutTimestampReps = icmpstat.icps_outhist[ICMP_TSTAMPREPLY];
    mib->icmpOutAddrMasks = icmpstat.icps_outhist[ICMP_MASKREQ];
    mib->icmpOutAddrMaskReps = icmpstat.icps_outhist[ICMP_MASKREPLY];

    return 0;
}


int
sysctl_read_icmp6_stat(struct icmp6_mib *mib)
{
    struct icmp6stat icmpstat;
    size_t   size = sizeof(icmpstat);
    int      i;
    static int      sname[4] =
        { CTL_NET, PF_INET6, IPPROTO_ICMPV6, ICMPV6CTL_STATS };

    (void)memset(mib, 0, sizeof(*mib));

    if (-1 == sysctl(sname, 4, &icmpstat, &size, NULL, 0)) {
	snmp_perror("sysctl_read_icmp6_stat: net.inet6.icmp6.stats");
        return -1;
    }

    mib->icmp6InMsgs = icmpstat.icp6s_badcode
            + icmpstat.icp6s_tooshort
	    + icmpstat.icp6s_checksum
            + icmpstat.icp6s_badlen;
    mib->icmp6InErrors = mib->icmp6InMsgs;
    for (i = 0; i <= ICMP6_MAXTYPE; i++)
        mib->icmp6InMsgs  += icmpstat.icp6s_inhist[i];
    mib->icmp6InDestUnreachs = icmpstat.icp6s_inhist[ICMP6_DST_UNREACH];
    mib->icmp6InPktTooBigs = icmpstat.icp6s_inhist[ICMP6_PACKET_TOO_BIG];
    mib->icmp6InTimeExcds = icmpstat.icp6s_inhist[ICMP6_TIME_EXCEEDED];
    mib->icmp6InParmProblems = icmpstat.icp6s_inhist[ICMP6_PARAM_PROB];
    mib->icmp6InEchos = icmpstat.icp6s_inhist[ICMP6_ECHO_REQUEST];
    mib->icmp6InEchoReplies = icmpstat.icp6s_inhist[ICMP6_ECHO_REPLY];
    mib->icmp6InGroupMembQueries = icmpstat.icp6s_inhist[MLD_LISTENER_QUERY];
    mib->icmp6InGroupMembResponses = icmpstat.icp6s_inhist[MLD_LISTENER_REPORT];
    mib->icmp6InRouterSolicits = icmpstat.icp6s_inhist[ND_ROUTER_SOLICIT];
    mib->icmp6InRouterAdvertisements = icmpstat.icp6s_inhist[ND_ROUTER_ADVERT];
    mib->icmp6InNeighborSolicits = icmpstat.icp6s_inhist[ND_NEIGHBOR_SOLICIT];
    mib->icmp6InNeighborAdvertisements = icmpstat.icp6s_inhist[ND_NEIGHBOR_ADVERT];
    mib->icmp6InRedirects = icmpstat.icp6s_inhist[ND_REDIRECT];

    mib->icmp6OutMsgs = icmpstat.icp6s_canterror
        + icmpstat.icp6s_toofreq;
    for (i = 0; i <= ICMP6_MAXTYPE; i++)
        mib->icmp6OutMsgs += icmpstat.icp6s_outhist[i];
    mib->icmp6OutDestUnreachs = icmpstat.icp6s_outhist[ICMP6_DST_UNREACH];
    mib->icmp6OutPktTooBigs =  icmpstat.icp6s_outhist[ICMP6_PACKET_TOO_BIG];
    mib->icmp6OutTimeExcds = icmpstat.icp6s_outhist[ICMP6_TIME_EXCEEDED];
    mib->icmp6OutParmProblems = icmpstat.icp6s_outhist[ICMP6_PARAM_PROB];
    mib->icmp6OutEchos = icmpstat.icp6s_outhist[ICMP6_ECHO_REQUEST];
    mib->icmp6OutEchoReplies = icmpstat.icp6s_outhist[ICMP6_ECHO_REPLY];
    mib->icmp6OutRouterSolicits =  icmpstat.icp6s_outhist[ND_ROUTER_SOLICIT];
    mib->icmp6OutNeighborSolicits =  icmpstat.icp6s_outhist[ND_NEIGHBOR_SOLICIT];
    mib->icmp6OutNeighborAdvertisements =  icmpstat.icp6s_outhist[ND_NEIGHBOR_ADVERT];
    mib->icmp6OutRedirects = icmpstat.icp6s_outhist[ND_REDIRECT];
    mib->icmp6OutGroupMembResponses =  icmpstat.icp6s_outhist[MLD_LISTENER_REPORT];
    mib->icmp6OutGroupMembReductions =  icmpstat.icp6s_outhist[MLD_LISTENER_DONE];

    return 0;
}


int
sysctl_read_icmp_msg_stat(struct icmp_mib *mib,
                          struct icmp4_msg_mib *msgmib,
			  int *flag)
{
    sysctl_read_icmp_stat(mib);
    *flag = 0;
    return 0;
}


int
sysctl_read_icmp6_msg_stat(struct icmp6_mib *mib,
                           struct icmp6_msg_mib *msgmib,
			   int *support)
{
    struct icmp6stat icmpstat;
    size_t   size = sizeof(icmpstat);
    int      i;
    static int      sname[4] =
        { CTL_NET, PF_INET6, IPPROTO_ICMPV6, ICMPV6CTL_STATS };

    sysctl_read_icmp6_stat(mib);

    if (-1 == sysctl(sname, 4, &icmpstat, &size, NULL, 0)) {
	snmp_perror("sysctl_read_icmp6_stat: net.inet6.icmp6.stats");
        return -1;
    }

    for (i = 0; i < 256; i++) {
	msgmib->vals[i].InType = icmpstat.icp6s_inhist[i];
	msgmib->vals[i].OutType = icmpstat.icp6s_outhist[i];
    }
    *support = 1;
    return 0;
}


#endif
