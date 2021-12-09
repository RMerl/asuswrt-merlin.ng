/*
*    Copyright (c) 2015 Broadcom Corporation
*    All Rights Reserved
*
<:label-BRCM:2015:DUAL/GPL:standard

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/
#ifndef _BCM_MCAST_H
#define _BCM_MCAST_H

#if !defined(__KERNEL__)
#define NETLINK_BCM_MCAST         30
#endif

#define BCM_MCAST_MAX_SRC_IF      10
#define BCM_MCAST_NL_RX_BUF_SIZE  2048

#define BCM_MCAST_PROTO_IPV4 0x1
#define BCM_MCAST_PROTO_IPV6 0x2
#define BCM_MCAST_PROTO_ALL  (BCM_MCAST_PROTO_IPV4 | BCM_MCAST_PROTO_IPV6)

#define BCM_MCAST_SNOOP_IN_ADD    1
#define BCM_MCAST_SNOOP_IN_CLEAR  2
#define BCM_MCAST_SNOOP_EX_ADD    3
#define BCM_MCAST_SNOOP_EX_CLEAR  4

#define BCM_MCAST_EVT_SNOOP_ADD       1
#define BCM_MCAST_EVT_SNOOP_DEL       2

#define BCM_MCAST_IF_UNKNOWN      0
#define BCM_MCAST_IF_BRIDGED      1
#define BCM_MCAST_IF_ROUTED       2

#define BCM_MCAST_SNOOPING_DISABLED_FLOOD  0  /* snooping is disabled, IP multicast is flooded */
#define BCM_MCAST_SNOOPING_STANDARD_MODE   1  /* snoping is enabled, unsolicited IP multicast is flooded */
#define BCM_MCAST_SNOOPING_BLOCKING_MODE   2  /* snoping is enabled, unsolicited IP mutlicast is dropped */

#define BCM_MCAST_LAN2LAN_SNOOPING_DISABLE 0
#define BCM_MCAST_LAN2LAN_SNOOPING_ENABLE  1

#define BCM_MCAST_PROTO_RATE_LIMIT_MAX     500

typedef enum bcm_mcast_msgtype 
{
    BCM_MCAST_MSG_BASE = 0x10, /* NLMSG_MIN_TYPE, */
    BCM_MCAST_MSG_REGISTER = BCM_MCAST_MSG_BASE, /* usr - > krnl -> usr */
    BCM_MCAST_MSG_UNREGISTER, /* usr - > krnl -> usr */
    BCM_MCAST_MSG_IGMP_PKT, /* krnl -> usr */
    BCM_MCAST_MSG_IGMP_SNOOP_ENTRY,
    BCM_MCAST_MSG_MLD_PKT, /* krnl -> usr */
    BCM_MCAST_MSG_MLD_SNOOP_ENTRY,
    BCM_MCAST_MSG_IGMP_PURGE_ENTRY,
    BCM_MCAST_MSG_MLD_PURGE_ENTRY,
    BCM_MCAST_MSG_IF_CHANGE,
    BCM_MCAST_MSG_MC_FDB_CLEANUP, /* clean up for MIB RESET */
    BCM_MCAST_MSG_SET_PRI_QUEUE,
    BCM_MCAST_MSG_UPLINK_INDICATION,
    BCM_MCAST_MSG_IGMP_PURGE_REPORTER,
    BCM_MCAST_MSG_MLD_PURGE_REPORTER,
    BCM_MCAST_MSG_CONTROLS_ADMISSION,
    BCM_MCAST_MSG_ADMISSION_RESULT,
    BCM_MCAST_MSG_SNOOP_CFG,
    BCM_MCAST_MSG_PROTO_RATE_LIMIT_CFG,
    BCM_MCAST_MSG_IGNORE_GROUP_LIST,
    BCM_MCAST_MSG_IGMP_DROP_GROUP,
    BCM_MCAST_MSG_MLD_DROP_GROUP,
    BCM_MCAST_MSG_SET_TIMEOUT,
    BCM_MCAST_MSG_BLOG_ENABLE,
    BCM_MCAST_MSG_QUERY_TRIGGER,
    BCM_MCAST_MSG_MAX
} t_BCM_MCAST_MSGTYPES;
#define BCM_MCAST_NR_MSGTYPES (BCM_MCAST_MSG_MAX - BCM_MCAST_MSG_BASE)

typedef struct bcm_mcast_register 
{
    int primary;
    int result;
} t_BCM_MCAST_REGISTER;

typedef struct bcm_mcast_if_change
{
   int               ifi;
   int               proto;
} t_BCM_MCAST_IF_CHANGE;

typedef struct bcm_mcast_pkt_info
{
    int                       parent_ifi;
    int                       rxdev_ifi;
    int                       data_len;
    int                       lanppp;
    int                       packetIndex;
    union {
       struct in6_addr        ipv6rep;
       struct in_addr         ipv4rep;
    };
    unsigned short            tci; /* vlan id */
    unsigned char             repMac[6];
    unsigned char             pkt[0];
} t_BCM_MCAST_PKT_INFO;

typedef struct bcm_mcast_igmp_purge_entry
{
    struct in_addr            grp;
    struct in_addr            src;
    struct in_addr            rep;
    int                       parent_ifi;
    int                       rep_ifi;
    unsigned short            tci;
    unsigned char             rep_proto_ver;
} t_BCM_MCAST_IGMP_PURGE_ENTRY;

typedef struct bcm_mcast_query_trigger
{
    int                       rep_ifi;
} t_BCM_MCAST_QUERY_TRIGGER;

typedef struct bcm_mcast_igmp_purge_reporter 
{
   int                       parent_ifi;
   int                       dstdev_ifi;
   struct in_addr            rep;   
} t_BCM_MCAST_IGMP_PURGE_REPORTER;

typedef struct bcm_mcast_mld_purge_reporter 
{
   int                       parent_ifi;
   int                       dstdev_ifi;
   struct in6_addr           rep;
} t_BCM_MCAST_MLD_PURGE_REPORTER;

typedef struct bcm_mcast_admission_filter
{
    int igmpAdmissionEn;
    int mldAdmissionEn;    
} t_BCM_MCAST_ADMISSION_FILTER;

typedef struct bcm_mcast_admission_result
{
    int               parent_ifi;
    int               packetIndex;
    int               admitted;
    int               proto;
} t_BCM_MCAST_ADMISSION_RESULT;

typedef struct bcm_mcast_uplink
{
    int uplink;
} t_BCM_MCAST_UPLINK;

typedef struct bcm_mcast_priority_queue
{
    int pri_queue;
} t_BCM_MCAST_PRIORITY_QUEUE;

typedef struct bcm_mcast_blog_enable
{
    int blog_enable;
} t_BCM_MCAST_BLOG_ENABLE;

typedef struct bcm_mcast_wan_info
{
   int                       ifi;
   int                       if_ops;
} t_BCM_MCAST_WAN_INFO;

typedef t_BCM_MCAST_WAN_INFO t_BCM_MCAST_WAN_INFO_ARRAY[BCM_MCAST_MAX_SRC_IF];

typedef struct bcm_mcast_igmp_snoop_entry
{
   int                       parent_ifi;
   int                       dstdev_ifi;
   /* Internal, ignore endianness */
   unsigned int              mode;
   unsigned int              code;
   unsigned short            tci;/* vlan id */
   t_BCM_MCAST_WAN_INFO      wan_info[BCM_MCAST_MAX_SRC_IF];
   int                       lanppp;
   int                       excludePort;  
   char                      enRtpSeqCheck;
   /* Standard, use big endian */
   struct in_addr            rxGrp;
   struct in_addr            txGrp;
   struct in_addr            src;
   struct in_addr            rep;
   unsigned char             repMac[6];
   unsigned char             rep_proto_ver;
} t_BCM_MCAST_IGMP_SNOOP_ENTRY;

typedef struct bcm_mcast_igmp_drop_group_entry 
{
   int                       parent_ifindex;
   int                       dest_ifindex;
   struct in_addr            group;
} t_BCM_MCAST_IGMP_DROP_GROUP_ENTRY;

typedef struct bcm_mcast_mld_drop_group_entry 
{
   int                       parent_ifindex;
   int                       dest_ifindex;
   struct in6_addr           group;
} t_BCM_MCAST_MLD_DROP_GROUP_ENTRY;

typedef struct bcm_mcast_mld_snoop_entry 
{
   int                       parent_ifi;
   int                       dstdev_ifi;
   /* Internal, ignore endianness */
   unsigned int              mode;
   unsigned int              code;
   unsigned short            tci;
   t_BCM_MCAST_WAN_INFO      wan_info[BCM_MCAST_MAX_SRC_IF];
   int                       lanppp;
   /* External, use big endian */
   struct in6_addr           grp;
   struct in6_addr           src;
   struct in6_addr           rep;
   unsigned char             repMac[6];
   unsigned char             rep_proto_ver;
} t_BCM_MCAST_MLD_SNOOP_ENTRY;

typedef struct bcm_mcast_ignore_group_entry{
   struct in6_addr              address;
   struct in6_addr              mask;
} t_BCM_MCAST_IGNORE_GROUP_ENTRY;

typedef struct bcm_mcast_ignore_group_message {
   int                            proto;
   int                            count;
   t_BCM_MCAST_IGNORE_GROUP_ENTRY ignoreEntry[0];
} t_BCM_MCAST_IGNORE_GROUP_MESSAGE;

typedef struct bcm_mcast_timeout_entry 
{
   int                       proto;
   int                       generalMembershipTimeoutSecs;
} t_BCM_MCAST_TIMEOUT_ENTRY;

typedef struct bcm_mcast_proto_rate_limit
{
    int               proto;
    int               ifi;
    int               rate;
} t_BCM_MCAST_PROTO_RATE_LIMIT;

typedef struct bcm_mcast_snoop_cfg
{
    int               proto;
    int               ifi;
    unsigned int      mode;
    int               l2lenable;
} t_BCM_MCAST_SNOOP_CFG;

typedef struct bcm_mcast_notify
{
   int proto;
   int ifindex;
   union {
      struct {
         struct in6_addr        ipv6grp;
         struct in6_addr        ipv6rep;
         struct in6_addr        ipv6src;
      };
      struct {
         struct in_addr        ipv4grp;
         struct in_addr        ipv4rep;
         struct in_addr        ipv4src;
      };
   };
   unsigned char repMac[6];
} t_BCM_MCAST_NOTIFY;

/* function externs */
#if defined(__KERNEL__)
#if defined(CONFIG_BR_MLD_SNOOP) || defined(CONFIG_BR_IGMP_SNOOP)
int bcm_mcast_is_snooping_enabled(struct net_device *dev, int proto);
int bcm_mcast_control_filter(void *grp, int proto);
int bcm_mcast_notify_register(struct notifier_block *nb);
int bcm_mcast_notify_unregister(struct notifier_block *nb);
int bcm_mcast_snoop_update_entry(int                proto,
                                 struct net_device *parent_dev,
                                 struct net_device *dst_dev,
                                 struct net_device *from_dev,
                                 int                wan_ops,
                                 void              *rxGrp, 
                                 void              *txGrp,
                                 void              *src,
                                 void              *rep,
                                 unsigned char     *repMac,
                                 unsigned char      rep_proto_ver,
                                 uint32_t           rep_info,
                                 int                mode, 
                                 uint16_t           tci, 
                                 int                lanppp,
                                 int                excludePort,
                                 char               enRtpSeqCheck);
int bcm_mcast_snoop_remove_entry(int                proto,
                                 struct net_device *parent_dev,
                                 struct net_device *dst_dev,
                                 struct net_device *from_dev,
                                 void              *rxGrp, 
                                 void              *txGrp,
                                 void              *src,
                                 void              *rep,
                                 uint32_t           rep_info,
                                 int                mode);

#if defined(CONFIG_BLOG) && (defined(CONFIG_BCM_WLAN) || defined(CONFIG_BCM_WLAN_MODULE))
int bcm_mcast_wlan_client_disconnect_notifier(struct net_device *dev, char *mac);
#endif

#else
static inline int bcm_mcast_is_snooping_enabled(struct net_device *dev, int proto) { return 0; }
static inline int bcm_mcast_control_filter(void *grp, int proto) { return 1; }
static inline int bcm_mcast_notify_register(struct notifier_block *nb) { return 0; }
static inline int bcm_mcast_notify_unregister(struct notifier_block *nb) { return 0; }
static inline int bcm_mcast_snoop_update_entry(int proto,
                                               struct net_device *parent_dev,
                                               struct net_device *dst_dev,
                                               struct net_device *from_dev,
                                               int                wan_ops,
                                               void              *rxGrp, 
                                               void              *txGrp,
                                               void              *src,
                                               void              *rep,
                                               unsigned char     *repMac,
                                               unsigned char      rep_proto_ver,
                                               uint32_t           rep_info,
                                               int                mode, 
                                               uint16_t           tci, 
                                               int                lanppp,
                                               int                excludePort,
                                               char               enRtpSeqCheck) { return 0; }
static inline int bcm_mcast_snoop_remove_entry(int                proto,
                                               struct net_device *parent_dev,
                                               struct net_device *dst_dev,
                                               struct net_device *from_dev,
                                               void              *rxGrp, 
                                               void              *txGrp,
                                               void              *src,
                                               void              *rep,
                                               uint32_t           rep_info,
                                               int                mode) { return 0; }

static inline int bcm_mcast_wlan_client_disconnect_notifier(struct net_device *dev, char *mac) { return 0; }

#endif /* CONFIG_BR_MLD_SNOOP || CONFIG_BR_IGMP_SNOOP */
#endif /* __KERNEL__ */

#endif /* _BCM_MCAST_H_ */
