/*
* <:copyright-BRCM:2022:DUAL/GPL:standard
* 
*    Copyright (c) 2022 Broadcom 
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
