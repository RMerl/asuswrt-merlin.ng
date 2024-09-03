#ifndef _BCM_BR_FDB_H
#define _BCM_BR_FDB_H
/*
<:copyright-BRCM:2013:DUAL/GPL:standard

   Copyright (c) 2013 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/

int bcm_br_has_fdb_expired(const struct net_bridge *br,
				  const struct net_bridge_fdb_entry *fdb);

int bcm_br_fdb_notify(struct net_bridge *br,
		       const struct net_bridge_fdb_entry *fdb, int type,
		       bool swdev_notify);

int bcm_br_fdb_init(struct net_bridge *br, struct net_bridge_fdb_entry *fdb);

int bcm_br_fdb_fill_info(const struct net_bridge_fdb_entry *fdb);

int bcm_br_fdb_update(struct net_bridge_fdb_entry *fdb,
					struct net_bridge_port *source);

int bcm_br_fdb_cleanup(struct net_bridge_fdb_entry *fdb, 
                       unsigned long time_now, unsigned long delay);
                       
unsigned int bcm_br_fdb_mac_limit(struct sk_buff *skb);
#endif
