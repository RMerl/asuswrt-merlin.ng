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

#ifndef _BCM_MCAST_IPV4_H_
#define _BCM_MCAST_IPV4_H_


typedef struct igmp_src_entry
{
   struct in_addr   src;
   unsigned long    tstamp;
   int              filt_mode;
} t_igmp_src_entry;

typedef struct igmp_rep_entry
{
   struct in_addr      rep;
   unsigned char       repMac[6];
   unsigned char       rep_proto_ver;
   unsigned long       tstamp;
   struct list_head    list;
} t_igmp_rep_entry;

typedef struct igmp_grp_entry
{
   struct hlist_node  hlist;
   struct net_device *dst_dev;
   struct net_device *to_accel_dev;
   struct in_addr     rxGrp;
   struct in_addr     txGrp;
   struct list_head   rep_list;
   t_igmp_src_entry   src_entry;
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
   int                excludePort;
   char               enRtpSeqCheck;  
   struct net_device *from_dev;
} t_igmp_grp_entry;

int bcm_mcast_igmp_process_ignore_group_list (int count, t_BCM_MCAST_IGNORE_GROUP_ENTRY* ignoreMsgPtr);
int bcm_mcast_igmp_wipe_group(bcm_mcast_ifdata *parent_if, int dest_ifindex, struct in_addr *gpAddr);
void bcm_mcast_igmp_del_entry(bcm_mcast_ifdata *pif, 
                              t_igmp_grp_entry *igmp_fdb,
                              struct in_addr   *rep,
                              unsigned char    *repMac);
void bcm_mcast_igmp_update_bydev( bcm_mcast_ifdata *pif, struct net_device *dev, int activate);
int bcm_mcast_igmp_add(struct net_device *from_dev,
                           int wan_ops,
                           bcm_mcast_ifdata *pif, 
                           struct net_device *dst_dev, 
                           struct net_device *to_accel_dev, 
                           struct in_addr *rxGrp, 
                           struct in_addr *txGrp, 
                           struct in_addr *rep,
                           unsigned char *repMac,
                           unsigned char rep_proto_ver,
                           int mode, 
                           uint16_t tci, 
                           struct in_addr *src,
                           int lanppp,
                           int excludePort,
                           char enRtpSeqCheck,
                           uint32_t info);
int bcm_mcast_igmp_remove(struct net_device *from_dev,
                              bcm_mcast_ifdata *pif, 
                              struct net_device *dst_dev, 
                              struct in_addr *rxGrp, 
                              struct in_addr *txGrp, 
                              struct in_addr *rep, 
                              int mode, 
                              struct in_addr *src,
                              uint32_t info);
void bcm_mcast_igmp_wipe_reporter_for_port (bcm_mcast_ifdata *pif,
                                                struct in_addr *rep, 
                                                struct net_device *rep_dev);
void bcm_mcast_igmp_wipe_reporter_by_mac(bcm_mcast_ifdata *pif,
                                             unsigned char *repMac);
void bcm_mcast_igmp_init(bcm_mcast_ifdata *pif);
void bcm_mcast_igmp_fini(bcm_mcast_ifdata *pif);
int bcm_mcast_igmp_admission_queue(bcm_mcast_ifdata *pif, struct sk_buff *skb);
void bcm_mcast_igmp_admission_update_bydev(bcm_mcast_ifdata *pif, struct net_device *dev);
int bcm_mcast_igmp_admission_process(bcm_mcast_ifdata *pif, int packet_index, int admitted);
int bcm_mcast_igmp_should_deliver(bcm_mcast_ifdata *pif, 
                                  const struct iphdr *pip,
                                  struct net_device *src_dev,
                                  struct net_device *dst_dev,
                                  int pkt_type);
int bcm_mcast_igmp_display(struct seq_file *seq, bcm_mcast_ifdata *pif);
int bcm_mcast_igmp_control_filter(__be32 dest_ip);

#if defined(CONFIG_BLOG)
t_igmp_grp_entry *bcm_mcast_igmp_fdb_copy(bcm_mcast_ifdata       *pif, 
                                          const t_igmp_grp_entry *igmp_fdb);
void bcm_mcast_igmp_process_blog_enable( bcm_mcast_ifdata *pif, int enable );
#endif

#endif /* _BCM_MCAST_IPV4_H_ */

