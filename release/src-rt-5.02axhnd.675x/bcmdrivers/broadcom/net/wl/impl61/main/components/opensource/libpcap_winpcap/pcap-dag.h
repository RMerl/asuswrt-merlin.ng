/*
 * pcap-dag.c: Packet capture interface for Endace DAG card.
 *
 * The functionality of this code attempts to mimic that of pcap-linux as much
 * as possible.  This code is only needed when compiling in the DAG card code
 * at the same time as another type of device.
 *
 * Author: Richard Littin, Sean Irvine ({richard,sean}@reeltwo.com)
 *
 * @(#) $Header: /tcpdump/master/libpcap/pcap-dag.h,v 1.4.2.3 2008-04-04 19:39:06 guy Exp $ (LBL)
 */

pcap_t *dag_create(const char *, char *);
int dag_platform_finddevs(pcap_if_t **devlistp, char *errbuf);

#ifndef TYPE_AAL5
#define TYPE_AAL5               4
#endif // endif

#ifndef TYPE_MC_HDLC
#define TYPE_MC_HDLC            5
#endif // endif

#ifndef TYPE_MC_RAW
#define TYPE_MC_RAW             6
#endif // endif

#ifndef TYPE_MC_ATM
#define TYPE_MC_ATM             7
#endif // endif

#ifndef TYPE_MC_RAW_CHANNEL
#define TYPE_MC_RAW_CHANNEL     8
#endif // endif

#ifndef TYPE_MC_AAL5
#define TYPE_MC_AAL5            9
#endif // endif

#ifndef TYPE_COLOR_HDLC_POS
#define TYPE_COLOR_HDLC_POS     10
#endif // endif

#ifndef TYPE_COLOR_ETH
#define TYPE_COLOR_ETH          11
#endif // endif

#ifndef TYPE_MC_AAL2
#define TYPE_MC_AAL2            12
#endif // endif

#ifndef TYPE_IP_COUNTER
#define TYPE_IP_COUNTER         13
#endif // endif

#ifndef TYPE_TCP_FLOW_COUNTER
#define TYPE_TCP_FLOW_COUNTER   14
#endif // endif

#ifndef TYPE_DSM_COLOR_HDLC_POS
#define TYPE_DSM_COLOR_HDLC_POS 15
#endif // endif

#ifndef TYPE_DSM_COLOR_ETH
#define TYPE_DSM_COLOR_ETH      16
#endif // endif

#ifndef TYPE_COLOR_MC_HDLC_POS
#define TYPE_COLOR_MC_HDLC_POS  17
#endif // endif

#ifndef TYPE_AAL2
#define TYPE_AAL2               18
#endif // endif

#ifndef TYPE_COLOR_HASH_POS
#define TYPE_COLOR_HASH_POS     19
#endif // endif

#ifndef TYPE_COLOR_HASH_ETH
#define TYPE_COLOR_HASH_ETH     20
#endif // endif

#ifndef TYPE_INFINIBAND
#define TYPE_INFINIBAND         21
#endif // endif

#ifndef TYPE_IPV4
#define TYPE_IPV4               22
#endif // endif

#ifndef TYPE_IPV6
#define TYPE_IPV6               23
#endif // endif

#ifndef TYPE_PAD
#define TYPE_PAD                48
#endif // endif
