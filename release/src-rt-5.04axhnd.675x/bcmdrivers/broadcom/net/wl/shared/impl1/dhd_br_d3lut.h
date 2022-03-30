/*
    Copyright (c) 2019 Broadcom
    All Rights Reserved

    <:label-BRCM:2019:DUAL/GPL:standard
    
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

#ifndef _dhd_br_d3lut_h_
#define _dhd_br_d3lut_h_

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
extern void dhd_handle_br_d3lut(struct net_device *src_dev, struct net_device *dst_dev, struct sk_buff *skb);
extern void dhd_update_d3lut(struct net_device *dst_dev, struct sk_buff *skb);
#endif /* kernel >= 4.19.0 */

#endif
