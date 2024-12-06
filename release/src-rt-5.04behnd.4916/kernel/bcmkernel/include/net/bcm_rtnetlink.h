/*
 *    Copyright (c) 2003-2022 Broadcom
 *    All Rights Reserved
 *
 * <:label-BRCM:2022:DUAL/GPL:standard
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
