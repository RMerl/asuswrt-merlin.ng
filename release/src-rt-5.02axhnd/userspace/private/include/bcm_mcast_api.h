/*
* <:copyright-BRCM:2015:proprietary:standard
* 
*    Copyright (c) 2015 Broadcom 
*    All Rights Reserved
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
:>
*/

/***************************************************************************
 * File Name  : bcm_mcast_api.h
 *
 * Description: API definitions for creating snooping entries.
 *
 ***************************************************************************/
#ifndef __BCM_MCAST_API_H__
#define __BCM_MCAST_API_H__

#include <net/if.h>
#include <netinet/in.h>
#include <netinet/icmp6.h>
#include <bcm_mcast.h>

#define IP_UPNP_ADDR        0xEFFFFFFA /* UPnP / SSDP */
#define IP_NTFY_SRVR_ADDR   0xE000FF87 /* Notificatoin Server*/

/* Identify IPV4 L2 multicast by checking whether the most bytes is 0 */
#define SNOOP_IN_IS_ADDR_L2_MCAST(a)        \
    !(((__const unsigned char *) (a))[0])

#define SNOOP_IN6_IS_ADDR_MULTICAST(a) (((__const unsigned char *) (a))[0] == 0xff)
#define SNOOP_IN6_IS_ADDR_MC_SCOPE0(a) \
	(SNOOP_IN6_IS_ADDR_MULTICAST(a) \
	 && ((((__const unsigned char *) (a))[1] & 0xf) == 0x0))
#define SNOOP_IN6_IS_ADDR_MC_NODELOCAL(a) \
	(SNOOP_IN6_IS_ADDR_MULTICAST(a)					      \
	 && ((((__const unsigned char *) (a))[1] & 0xf) == 0x1))
#define SNOOP_IN6_IS_ADDR_MC_LINKLOCAL(a) \
	(SNOOP_IN6_IS_ADDR_MULTICAST(a)					      \
	 && ((((__const unsigned char *) (a))[1] & 0xf) == 0x2))
/* Identify IPV6 L2 multicast by checking whether the most 12 bytes are 0 */
#define SNOOP_IN6_IS_ADDR_L2_MCAST(a)       \
    !((((__const uint32_t *) (a))[0])       \
        || (((__const uint32_t *) (a))[1])  \
        || (((__const uint32_t *) (a))[2]))

typedef void (*bcm_mcast_api_rcv_func)(int type, unsigned char *pdata, int data_len);

/* IGMPv3 and MLDv2 group record types */
#define MODE_IS_INCLUDE          1
#define MODE_IS_EXCLUDE          2
#define MODE_TO_INCLUDE          3
#define MODE_TO_EXCLUDE          4
#define MODE_ALLOW_NEW_SRCS      5
#define MODE_BLOCK_OLD_SRCS      6

/* IGMP defines and types */
#define IGMP_MEMBERSHIP_QUERY       0x11
#define IGMP_V1_MEMBERSHIP_REPORT   0x12
#define IGMP_V2_MEMBERSHIP_REPORT   0x16
#define IGMP_V2_MEMBERSHIP_LEAVE    0x17
#define IGMP_V3_MEMBERSHIP_REPORT   0x22

#define IGMP_VERSION_1              0x12
#define IGMP_VERSION_2              0x16
#define IGMP_VERSION_3              0x22

/* IGMP query format */
typedef struct igmpv3_query 
{
   unsigned char             type; 	/* version & type of IGMP message */
   unsigned char             code; 	/* subtype for routing msgs */
   unsigned short            cksum;	/* IP-style checksum */
   struct in_addr            group;	/* group address being reported */
#if __BYTE_ORDER == __LITTLE_ENDIAN
   unsigned char             qrv:3,
                             suppress:1,
                             resv:4;
#else /* __BYTE_ORDER == __BIG_ENDIAN */
   unsigned char             resv:4,
                             suppress:1,
                             qrv:3;
#endif
   unsigned char             qqi;   	/* querier's query interval */
   unsigned short            numsrc;    /* number of sources */
   struct in_addr            sources[0]; /* source addresses */
} t_IGMPv3_QUERY;

/* IGMPv1/IGMPv2 report and query format */
typedef struct igmpv12_report
{
   unsigned char             type;  /* version & type */
   unsigned char             code;  /* unused */
   unsigned short            cksum; /* IP-style checksum */
   struct in_addr            group; /* group address being reported */
} t_IGMPv12_REPORT;

typedef t_IGMPv12_REPORT t_IGMPv12_QUERY;

/* IGMPv3 group record format */
typedef struct igmp_grp_record
{
   unsigned char             type;			/* record type */
   unsigned char             datalen;		/* amount of aux data */
   unsigned short            numsrc;		/* number of sources */
   struct in_addr            group;		/* the group being reported */
   struct in_addr            sources[0];	/* source addresses */
} t_IGMP_GRP_RECORD;

/* IGMPv3 report format */
typedef struct igmp_v3_report
{
   unsigned char             type;	/* version & type of IGMP message */
   unsigned char             rsv0;	/* reserved */
   unsigned short            cksum; 	/* IP-style checksum */
   unsigned short            rsv1;	/* reserved */
   unsigned short            numgrps;   /* number of groups*/
   t_IGMP_GRP_RECORD         group[0];  /* group records */
} t_IGMPv3_REPORT;


/* MLD defines and message types */
#define ICMPV6_MLD_V1V2_QUERY           130
#define ICMPV6_MLD_V1_REPORT            131
#define ICMPV6_MLD_V1_DONE              132
#define ICMPV6_MLD_V2_REPORT            143

#define MLD_VERSION_1                   1
#define MLD_VERSION_2                   2

/* MLDv1 report */
typedef struct mldv1_report
{
   struct icmp6_hdr          icmp6_hdr; /* ICMPv6 header */
   struct in6_addr           grp_addr; /* multicast group addr */
} t_MLDv1_REPORT;

typedef t_MLDv1_REPORT t_MLDv1_QUERY;

/* MLDv2 report record format */
typedef struct mld_grp_record
{
   unsigned char             type; /* record type */
   unsigned char             datalen; /* auxiliary data len */
   unsigned short            numsrc; /* number of sources */
   struct in6_addr           group; /* multicast group address */
   struct in6_addr           sources[0]; /* source addresses */
} t_MLD_GRP_RECORD;

/* MLDv2 report */
typedef struct mldv2_report
{
   unsigned char               type;
   unsigned char               code; 
   unsigned short              cksum;
   unsigned short              rsvd;
   unsigned short              numgrps; /* number of group records */
   t_MLD_GRP_RECORD            group[0]; /* multicast groups */
} t_MLDv2_REPORT;

/* MLD Query */
typedef struct mldv2_query
{
   struct icmp6_hdr          icmp6_hdr;
   struct in6_addr           grp_addr;
#if __BYTE_ORDER == __LITTLE_ENDIAN
   unsigned char             qrv:3,
                             suppress:1,
                             res:4;
#else /* __BYTE_ORDER == __BIG_ENDIAN */
   unsigned char             res:4,
                             suppress:1,
                             qrv:3;
#endif
   unsigned char             qqi;   	/* querier's query interval */
   unsigned short            num_srcs; /* number of sources */
   struct in6_addr           sources[0];
} t_MLDv2_QUERY;

/* function prototypes */
int bcm_mcast_api_update_igmp_snoop(int                    sock_nl_in, 
                                    int                    parent_ifi, 
                                    int                    dstdev_ifi,
                                    unsigned short         vid, 
                                    int                    lanppp, 
                                    const struct in_addr  *rxGrp, 
                                    const struct in_addr  *txGrp, 
                                    const struct in_addr  *src, 
                                    const struct in_addr  *rep, 
                                    unsigned char         *repMac, 
                                    unsigned char          rep_proto_ver,
                                    int                    filter_mode, 
                                    t_BCM_MCAST_WAN_INFO_ARRAY *wan_info,
                                    int                    excludePort, 
                                    char                   enRtpSeqCheck);

int bcm_mcast_api_update_mld_snoop(int                    sock_nl_in, 
                                   int                    parent_ifi, 
                                   int                    dstdev_ifi,
                                   unsigned short         vid, 
                                   int                    lanppp, 
                                   const struct in6_addr *grp, 
                                   const struct in6_addr *src, 
                                   const struct in6_addr *rep, 
                                   unsigned char         *repMac,
                                   unsigned char          rep_proto_ver,
                                   int                    filter_mode, 
                                   t_BCM_MCAST_WAN_INFO_ARRAY *wan_info);

int bcm_mcast_api_register(int sock_nl_in, int primary);
int bcm_mcast_api_unregister(int sock_nl_in, int primary);
int bcm_mcast_api_uplink(int sock_nl_in, int uplink);
int bcm_mcast_api_set_priority_queue(int sock_nl_in, int pri_queue);
int bcm_mcast_api_admission_filter(int sock_nl_in, unsigned char igmpEn, unsigned char mldEn);
int bcm_mcast_api_admission_result(int sock_nl_in, int ifi, int admitted, uintptr_t packetIndex, int proto);
int bcm_mcast_api_if_change(int sock_nl_in, int ifi, int proto);
int bcm_mcast_api_ipv4_purge_reporter(int sock_nl_in, int ifi, int dstdev_ifi, struct in_addr *rep);
int bcm_mcast_api_ipv6_purge_reporter(int sock_nl_in, int ifi, int dstdev_ifi, struct in6_addr *rep);
int bcm_mcast_api_fdb_cleanup(int sock_nl_in);
int bcm_mcast_api_set_snooping_cfg(int sock_nl, int ifi, int proto, int mode, int l2lenable);
int bcm_mcast_api_set_proto_rate_limit(int sock_nl, int ifi, int proto, int rate);
int bcm_mcast_api_snooping_exception (int sock_nl, int proto, int count, t_BCM_MCAST_IGNORE_GROUP_ENTRY *groupEntries);
int bcm_mcast_api_send_group_timeout (int sock_nl, int proto, int generalMembershipTimeoutSecs);
int bcm_mcast_api_igmp_drop_group(int sock_nl, int ifi, int rep_ifi, struct in_addr *gpAddr);
int bcm_mcast_api_mld_drop_group(int sock_nl, int ifi, int rep_ifi, struct in6_addr *gpAddr);
int bcm_mcast_api_blog_enable(int sock_nl, int enable);

int bcm_mcast_api_get_snooping_cfg(int ifi, int proto, int *mode, int *l2lenable);
int bcm_mcast_api_get_proto_rate_limit(int ifi, int proto, int *rate);

int bcm_mcast_api_socket_create(int *sock_nl, int portid);
int bcm_mcast_api_nl_recv(int nl_sock, char *rx_buf, int rx_bufsize, bcm_mcast_api_rcv_func process_func);

#endif /* __BCM_MCAST_API_H__ */
