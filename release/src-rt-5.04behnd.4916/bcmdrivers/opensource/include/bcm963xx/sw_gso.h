/*
* <:copyright-BRCM:2022:DUAL/GPL:standard
* 
*    Copyright (c) 2022 Broadcom 
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
:>
*/

#ifndef _SW_GSO_H_
#define _SW_GSO_H_

#include <linux/nbuff.h>

typedef int (*gso_skb_prehandle_cb) (struct sk_buff * \
                            ,struct net_device * \
                            );

typedef int (*sw_gso_enqueue_cb) (struct sk_buff * \
                            ,struct net_device * \
                            ,gso_skb_prehandle_cb skb_pre_handle \
                            ,HardStartXmitFuncP fkb_post_handle );

typedef int (*gso_skb_xmit) (struct sk_buff * \
                            ,struct net_device * \
                            ,HardStartXmitFuncP fkb_post_handle );
#endif
