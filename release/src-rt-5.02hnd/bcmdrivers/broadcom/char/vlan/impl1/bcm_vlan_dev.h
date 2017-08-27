/*
<:copyright-broadcom 
 
 Copyright (c) 2002 Broadcom Corporation 
 All Rights Reserved 
 No portions of this material may be reproduced in any form without the 
 written permission of: 
          Broadcom Corporation 
          16215 Alton Parkway 
          Irvine, California 92619 
 All information contained in this document is Broadcom Corporation 
 company private, proprietary, and trade secret. 
 
:>
*/
#ifndef _BCM_VLAN_DEV_H_
#define _BCM_VLAN_DEV_H_

int bcmVlan_devInit(struct net_device *vlanDev);
void bcmVlan_devUninit(struct net_device *dev);
int bcmVlan_devOpen(struct net_device *vlanDev);
int bcmVlan_devStop(struct net_device *vlanDev);
int bcmVlan_devSetMacAddress(struct net_device *vlanDev, void *p);
void bcmVlan_devSetRxMode(struct net_device *vlanDev);
void bcmVlan_devChangeRxFlags(struct net_device *vlanDev, int change);
int bcmVlan_devIoctl(struct net_device *vlanDev, struct ifreq *ifr, int cmd);
int bcmVlan_devNeighSetup(struct net_device *vlanDev, struct neigh_parms *pa);
int bcmVlan_devHardHeader(struct sk_buff *skb, struct net_device *vlanDev,
                          unsigned short type, const void *daddr,
                          const void *saddr, unsigned len);
struct net_device_stats *bcmVlan_devGetStats(struct net_device *vlanDev);
struct net_device_stats * bcmVlan_devCollectStats(struct net_device * dev_p);
void bcmVlan_devUpdateStats(struct net_device * dev_p, BlogStats_t *blogStats_p);
void bcmVlan_devClearStats(struct net_device * dev_p);
int bcmVlan_devChangeMtu(struct net_device *dev, int new_mtu);
int bcmVlan_devRebuildHeader(struct sk_buff *skb);
int bcmVlan_devHardStartXmit(struct sk_buff *skb, struct net_device *dev);
int bcmVlan_devReceiveSkb(struct sk_buff **skbp);

#endif /* _BCM_VLAN_DEV_H_ */

