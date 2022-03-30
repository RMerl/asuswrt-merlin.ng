#ifndef _BCM_BR_FDB_H
#define _BCM_BR_FDB_H
/*
<:copyright-BRCM:2013:DUAL/GPL:standard

   Copyright (c) 2013 Broadcom 
   All Rights Reserved

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

int bcm_br_has_fdb_expired(const struct net_bridge *br,
				  const struct net_bridge_fdb_entry *fdb);

int bcm_br_fdb_notify(struct net_bridge *br,
		       const struct net_bridge_fdb_entry *fdb, int type,
		       bool swdev_notify);

int bcm_br_fdb_init(struct net_bridge_fdb_entry *fdb);

int bcm_br_fdb_fill_info(const struct net_bridge_fdb_entry *fdb);

int bcm_br_fdb_update(struct net_bridge_fdb_entry *fdb,
					struct net_bridge_port *source);

int bcm_br_fdb_cleanup(struct net_bridge_fdb_entry *fdb, 
                       unsigned long time_now, unsigned long delay);
                       
unsigned int bcm_br_fdb_mac_limit(struct sk_buff *skb);
#endif
