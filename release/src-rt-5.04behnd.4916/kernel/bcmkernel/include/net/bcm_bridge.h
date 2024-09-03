/*
<:copyright-BRCM:2022:DUAL/GPL:standard

   Copyright (c) 2022 Broadcom 
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
#ifndef _BCM_BRIDGE_H
#define _BCM_BRIDGE_H
int bcm_br_hook_handle_frame_finish(struct sk_buff *skb, int state);
int bcm_br_hook_should_deliver(const struct sk_buff *skb, const struct net_bridge_port *p);
int bcm_br_hook_br_flood(struct sk_buff *skb, struct net_bridge *br);
#endif /* _BCM_BRIDGE_H */
