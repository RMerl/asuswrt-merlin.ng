/*
    Copyright (c) 2019 Broadcom
    All Rights Reserved

    <:label-BRCM:2019:DUAL/GPL:standard

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

#ifndef _dhd_br_d3lut_h_
#define _dhd_br_d3lut_h_

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
extern void dhd_handle_br_d3lut(struct net_device *src_dev, struct net_device *dst_dev, struct sk_buff *skb);
extern void dhd_update_d3lut(struct net_device *dst_dev, struct sk_buff *skb);
#endif /* kernel >= 4.19.0 */

#endif
