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

#ifndef _BCM_MCAST_IPV6_H_
#define _BCM_MCAST_IPV6_H_

#define BCM_IN6_ARE_ADDR_EQUAL(a,b)                                       \
       ((((__const uint32_t *) (a))[0] == ((__const uint32_t *) (b))[0])  \
    &&  (((__const uint32_t *) (a))[1] == ((__const uint32_t *) (b))[1])  \
    &&  (((__const uint32_t *) (a))[2] == ((__const uint32_t *) (b))[2])  \
    &&  (((__const uint32_t *) (a))[3] == ((__const uint32_t *) (b))[3])) 

#define BCM_IN6_ASSIGN_ADDR(a,b)                                  \
    do {                                                          \
        ((uint32_t *) (a))[0] = ((__const uint32_t *) (b))[0];    \
        ((uint32_t *) (a))[1] = ((__const uint32_t *) (b))[1];    \
        ((uint32_t *) (a))[2] = ((__const uint32_t *) (b))[2];    \
        ((uint32_t *) (a))[3] = ((__const uint32_t *) (b))[3];    \
    } while(0)

#define BCM_IN6_IS_ADDR_MULTICAST(a) (((__const uint8_t *) (a))[0] == 0xff)

#define BCM_IN6_IS_ADDR_MC_NODELOCAL(a) \
   (BCM_IN6_IS_ADDR_MULTICAST(a)                     \
    && ((((__const uint8_t *) (a))[1] & 0xf) == 0x1))

#define BCM_IN6_IS_ADDR_MC_LINKLOCAL(a) \
   (BCM_IN6_IS_ADDR_MULTICAST(a)                     \
    && ((((__const uint8_t *) (a))[1] & 0xf) == 0x2))

#define BCM_IN6_IS_ADDR_MC_SITELOCAL(a) \
   (BCM_IN6_IS_ADDR_MULTICAST(a)                     \
    && ((((__const uint8_t *) (a))[1] & 0xf) == 0x5))

#define BCM_IN6_IS_ADDR_MC_ORGLOCAL(a) \
   (BCM_IN6_IS_ADDR_MULTICAST(a)                     \
    && ((((__const uint8_t *) (a))[1] & 0xf) == 0x8))

#define BCM_IN6_IS_ADDR_MC_GLOBAL(a) \
   (BCM_IN6_IS_ADDR_MULTICAST(a) \
    && ((((__const uint8_t *) (a))[1] & 0xf) == 0xe))

#define BCM_IN6_IS_ADDR_MC_SCOPE0(a) \
   (BCM_IN6_IS_ADDR_MULTICAST(a)                     \
    && ((((__const uint8_t *) (a))[1] & 0xf) == 0x0))

/* Identify IPV6 L2 multicast by checking whether the most 12 bytes are 0 */
#define BCM_IN6_IS_ADDR_L2_MCAST(a)         \
    !((((__const uint32_t *) (a))[0])       \
        || (((__const uint32_t *) (a))[1])  \
        || (((__const uint32_t *) (a))[2]))


typedef struct mld_src_entry
{
   struct in6_addr   src;
   unsigned long     tstamp;
   int               filt_mode;
} t_mld_src_entry;

typedef struct mld_rep_entry
{
   struct in6_addr     rep;
   unsigned char       repMac[6];
   unsigned char       rep_proto_ver;
   unsigned long       tstamp;
   struct list_head    list;
} t_mld_rep_entry;

typedef struct mld_grp_entry
{
   struct hlist_node  hlist;
   struct net_device *dst_dev;
   struct in6_addr    grp;
   struct list_head   rep_list;
   t_mld_src_entry   src_entry;
   uint16_t           lan_tci; /* vlan id */
   uint32_t           wan_tci; /* vlan id */
   int                num_tags;
   char               type;
#if defined(CONFIG_BLOG)
   BlogActivateKey_t  blog_idx;
   char               root;
#endif
   uint32_t           info; 
   int                lanppp;
   struct net_device *from_dev;
} t_mld_grp_entry;

int bcm_mcast_mld_process_ignore_group_list (int count, t_BCM_MCAST_IGNORE_GROUP_ENTRY* ignoreMsgPtr);
int bcm_mcast_mld_wipe_group(bcm_mcast_ifdata *parent_if, int dest_ifindex, struct in6_addr *gpAddr);
void bcm_mcast_mld_del_entry(bcm_mcast_ifdata *pif, 
                                         t_mld_grp_entry *mld_fdb,
                                         struct in6_addr   *rep,
                                         unsigned char    *repMac);
void bcm_mcast_mld_update_bydev( bcm_mcast_ifdata *pif, struct net_device *dev, int activate);
int bcm_mcast_mld_add(struct net_device *from_dev,
                           int wan_ops,
                           bcm_mcast_ifdata *pif, 
                           struct net_device *dst_dev, 
                           struct in6_addr *grp, 
                           struct in6_addr *rep,
                           unsigned char *repMac,
                           unsigned char rep_proto_ver,
                           int mode, 
                           uint16_t tci, 
                           struct in6_addr *src,
                           int lanppp,
                           uint32_t info);
int bcm_mcast_mld_remove(struct net_device *from_dev,
                              bcm_mcast_ifdata *pif, 
                              struct net_device *dst_dev, 
                              struct in6_addr *grp, 
                              struct in6_addr *rep, 
                              int mode, 
                              struct in6_addr *src,
                              uint32_t info);
void bcm_mcast_mld_wipe_reporter_for_port (bcm_mcast_ifdata *pif,
                                                struct in6_addr *rep, 
                                                struct net_device *rep_dev);
void bcm_mcast_mld_wipe_reporter_by_mac (bcm_mcast_ifdata *pif,
                                              unsigned char *repMac);
void bcm_mcast_mld_init(bcm_mcast_ifdata *pif);
void bcm_mcast_mld_fini(bcm_mcast_ifdata *pif);
void bcm_mcast_mld_delayed_skb_queue(bcm_mcast_ifdata *pif, struct sk_buff *skb);
int bcm_mcast_mld_admission_queue(bcm_mcast_ifdata *pif, struct sk_buff *skb);
void bcm_mcast_mld_admission_update_bydev(bcm_mcast_ifdata *pif, struct net_device *dev);
int bcm_mcast_mld_admission_process(bcm_mcast_ifdata *pif, int packet_index, int admitted);
int bcm_mcast_mld_should_deliver(bcm_mcast_ifdata *pif, 
                                  const struct ipv6hdr *pipv6,
                                  struct net_device *src_dev,
                                  struct net_device *dst_dev);
int bcm_mcast_mld_display(struct seq_file *seq, bcm_mcast_ifdata *pif);
int bcm_mcast_mld_control_filter(const struct in6_addr * ipv6);

#if defined(CONFIG_BLOG)
t_mld_grp_entry *bcm_mcast_mld_fdb_copy(bcm_mcast_ifdata       *pif, 
                                          const t_mld_grp_entry *mld_fdb);
void bcm_mcast_mld_process_blog_enable( bcm_mcast_ifdata *pif, int enable);
#endif  

#endif /* _BCM_MCAST_IPV6_H_ */

