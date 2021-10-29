/*
* <:copyright-BRCM:2013:DUAL/GPL:standard
* 
*    Copyright (c) 2013 Broadcom 
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
* :>
*/

#if defined(CONFIG_BLOG)
#include <linux/blog_rule.h>
int blog_rule_to_vlan_action(blogRule_t *blog_rule, rdpa_traffic_dir dir, bdmf_object_handle *vlan_action_obj);
#endif /* CONFIG_BLOG */
static inline int is_tpid(uint16_t h_proto)
{
    return h_proto == 0x8100 || h_proto == 0x88A8;
}

