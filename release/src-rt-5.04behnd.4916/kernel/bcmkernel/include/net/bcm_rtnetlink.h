/*
 *    Copyright (c) 2003-2022 Broadcom
 *    All Rights Reserved
 *
 * <:label-BRCM:2022:DUAL/GPL:standard
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

struct rtnl_bcm_ext {
	__u32 iff_flags;
};

static size_t rtnl_bcm_ext_size(const struct net_device *dev)
{
	struct rtnl_bcm_ext *e;

	return nla_total_size(sizeof(e->iff_flags));
}
static int rtnl_fill_bcm_ext(struct sk_buff *skb, struct net_device *dev)
{
	struct nlattr *nest_parms;

	nest_parms = nla_nest_start(skb, IFLA_BCM_EXT | NLA_F_NESTED);
	if (!nest_parms)
		goto nla_put_failure;

	if (nla_put_u32(skb, IFLA_BCM_EXT_FLAGS,
			(u32)dev->bcm_nd_ext.iff_flags))
		goto nla_put_failure;

	nla_nest_end(skb, nest_parms);

	return 0;

nla_put_failure:
	return -1;
}
