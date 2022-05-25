/*
* <:copyright-BRCM:2020:DUAL/GPL:standard
* 
*    Copyright (c) 2020 Broadcom 
*    All Rights Reserved
* 
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
* 
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
* 
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
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

static inline int bcm_mcast_mld_hash(const struct in6_addr *grp)
{
   return (jhash_1word((grp->s6_addr32[0] | grp->s6_addr32[3]), mcast_ctrl->ipv6_hash_salt) & (BCM_MCAST_HASH_SIZE - 1));
}

