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
*:>
*/

int bcm_mcast_mld_add_entry(struct net_device *from_dev,
                            int wan_ops,
                            bcm_mcast_ifdata *pif, 
                            struct net_device *dst_dev, 
                            struct net_device *to_accel_dev, 
                            struct in6_addr *grp, 
                            struct in6_addr *rep,
                            unsigned char *repMac,
                            unsigned char rep_proto_ver,
                            int mode, 
                            uint16_t tci, 
                            uint16_t grpVid,
                            struct in6_addr *src,
                            int lanppp,
                            uint32_t info);

int bcm_mcast_mld_del_entry(bcm_mcast_ifdata *pif,
                             t_mld_grp_entry *mld_fdb,
                             struct in6_addr   *rep,
                             unsigned char    *repMac);

void bcm_mcast_mld_remove_entry(struct net_device *from_dev,
                                bcm_mcast_ifdata *pif, 
                                struct net_device *dst_dev, 
                                struct in6_addr *grp, 
                                struct in6_addr *rep, 
                                int mode, 
                                struct in6_addr *src,
                                uint32_t info);

int bcm_mcast_mld_update_entry(bcm_mcast_ifdata *pif,
                               struct net_device *dst_dev, 
                               struct in6_addr *grp, 
                               struct in6_addr *rep,
                               unsigned char *repMac,
                               unsigned char rep_proto_ver,
                               int mode, 
                               struct in6_addr *src,
                               struct net_device *from_dev,
                               uint32_t info);

t_mld_rep_entry *bcm_mcast_mld_rep_find(const t_mld_grp_entry *mc_fdb,
                                        const struct in6_addr *rep,
                                        unsigned char *repMac);

