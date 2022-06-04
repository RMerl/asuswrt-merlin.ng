#ifdef CONFIG_BCM_KF_MHI
/*
<:copyright-BRCM:2011:DUAL/GPL:standard

   Copyright (c) 2011 Broadcom 
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
/*
 * File: net/af_mhi.h
 *
 * MHI Protocol Family kernel definitions
 */

#ifndef __LINUX_NET_AFMHI_H
#define __LINUX_NET_AFMHI_H

#include <linux/types.h>
#include <linux/socket.h>

#include <net/sock.h>


extern int mhi_register_protocol(int protocol);
extern int mhi_unregister_protocol(int protocol);
extern int mhi_protocol_registered(int protocol);

extern int mhi_skb_send(struct sk_buff *skb, struct net_device *dev, u8 proto);


#endif /* __LINUX_NET_AFMHI_H */
#endif /* CONFIG_BCM_KF_MHI */
