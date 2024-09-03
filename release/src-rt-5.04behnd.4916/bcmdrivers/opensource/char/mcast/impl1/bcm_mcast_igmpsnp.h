/*
* <:copyright-BRCM:2020:DUAL/GPL:standard
* 
*    Copyright (c) 2020 Broadcom 
*    All Rights Reserved
* 
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, version 2, as published by
* the Free Software Foundation (the "GPL").
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* 
* A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
* writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
* 
* :>
*/

int bcm_mcast_igmp_add_entry(struct net_device *from_dev,
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
                             uint16_t grpVid,
                             struct in_addr *src,
                             int lanppp,
                             int excludePort,
                             char enRtpSeqCheck,
                             uint32_t info);

int  bcm_mcast_igmp_del_entry(bcm_mcast_ifdata *pif,
                              t_igmp_grp_entry *igmp_fdb,
                              struct in_addr   *rep,
                              unsigned char    *repMac);

void bcm_mcast_igmp_remove_entry(struct net_device *from_dev,
                                 bcm_mcast_ifdata *pif, 
                                 struct net_device *dst_dev, 
                                 struct in_addr *rxGrp, 
                                 struct in_addr *txGrp, 
                                 struct in_addr *rep, 
                                 int mode,
                                 struct in_addr *src,
                                 uint32_t info);

int bcm_mcast_igmp_update_entry(bcm_mcast_ifdata *pif, 
                                struct net_device *dst_dev, 
                                struct in_addr *rxGrp,
                                struct in_addr *txGrp,
                                struct in_addr *rep,
                                unsigned char *repMac,
                                unsigned char rep_proto_ver,
                                int mode, 
                                struct in_addr *src,
                                struct net_device *from_dev,
                                uint32_t info);

t_igmp_rep_entry *bcm_mcast_igmp_rep_find(const t_igmp_grp_entry *mc_fdb,
                                          const struct in_addr *rep,
                                          unsigned char *repMac);



