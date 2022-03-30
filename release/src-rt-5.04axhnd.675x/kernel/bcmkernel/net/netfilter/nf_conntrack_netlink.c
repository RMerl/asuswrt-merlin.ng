/*
<:copyright-BRCM:2019:GPL/GPL:standard

   Copyright (c) 2019 Broadcom 
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

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/skbuff.h>
#include <linux/netfilter.h>
#include <linux/netfilter/nfnetlink_conntrack.h>
#include <linux/dpi.h>

#include <net/netlink.h>
#include <net/netfilter/nf_conntrack.h>

#if IS_ENABLED(CONFIG_BCM_DPI)
static int ctnetlink_dpi_size(const struct nf_conn *ct)
{
	int len = dpi_url_len(ct->bcm_ext.dpi.url);

	return nla_total_size(sizeof(u_int32_t))	/* CTA_DPI_APP_ID */
	       + 6 * nla_total_size(sizeof(u_int8_t))	/* CTA_DPI_MAC */
	       + nla_total_size(sizeof(u_int32_t))	/* CTA_DPI_STATUS */
	       + nla_total_size(sizeof(char) * len)	/* CTA_DPI_URL */
	       ;
}

static inline int
ctnetlink_dump_dpi(struct sk_buff *skb, const struct nf_conn *ct)
{
	struct nlattr *nest_parms;
	uint32_t app_id;
	uint8_t *mac;
	uint8_t empty_mac[6] = {0};
	char *url;

	app_id = dpi_app_id(ct->bcm_ext.dpi.app);
	mac = dpi_mac(ct->bcm_ext.dpi.dev);
	if (!mac)
		mac = empty_mac;
	url = dpi_url(ct->bcm_ext.dpi.url);

	nest_parms = nla_nest_start(skb, CTA_DPI | NLA_F_NESTED);
	if (!nest_parms)
		goto nla_put_failure;

	if (nla_put_be32(skb, CTA_DPI_APP_ID, htonl(app_id)))
		goto nla_put_failure;
	if (nla_put(skb, CTA_DPI_MAC, sizeof(empty_mac), mac))
		goto nla_put_failure;
	if (nla_put_be32(skb, CTA_DPI_STATUS, htonl(ct->bcm_ext.dpi.flags)))
		goto nla_put_failure;
	if (url && nla_put_string(skb, CTA_DPI_URL, url))
		goto nla_put_failure;

	nla_nest_end(skb, nest_parms);

	return 0;

nla_put_failure:
	return -1;
}

static const struct nla_policy dpi_policy[CTA_DPI_MAX+1] = {
	[CTA_DPI_APP_ID]	= { .type = NLA_U32 },
	[CTA_DPI_MAC]		= { .type = NLA_BINARY,
				    .len = 6 * sizeof(uint8_t) },
	[CTA_DPI_STATUS]	= { .type = NLA_U32 },
};

static int
ctnetlink_change_dpi(struct nf_conn *ct, const struct nlattr * const cda[])
{
	const struct nlattr *attr = cda[CTA_DPI];
	struct nlattr *tb[CTA_DPI_MAX+1];
	int err = 0;

	err = nla_parse_nested(tb, CTA_DPI_MAX, attr, dpi_policy, NULL);
	if (err < 0)
		return err;

	if (tb[CTA_DPI_STATUS]) {
		unsigned long new = ntohl(nla_get_be32(tb[CTA_DPI_STATUS]));
		unsigned long old = ct->bcm_ext.dpi.flags;
		unsigned long changed;

		ct->bcm_ext.dpi.flags = (new & DPI_NL_CHANGE_MASK) |
					(old & ~DPI_NL_CHANGE_MASK);
		changed = old ^ ct->bcm_ext.dpi.flags;

		if (test_bit(DPI_CT_BLOCK_BIT, &changed))
			dpi_block(ct);
	}

	return 0;
}
#endif /* IS_ENABLED(CONFIG_BCM_DPI) */

#if IS_ENABLED(CONFIG_BCM_SGS)
static int ctnetlink_sgs_size(const struct nf_conn *ct)
{
	return nla_total_size(sizeof(u_int64_t)) /* CTA_SGS_SES_KEY */
		+ nla_total_size(sizeof(u_int64_t)) /* CTA_SGS_SES_START */
		;
}

static inline int
ctnetlink_dump_sgs(struct sk_buff *skb, const struct nf_conn *ct)
{
	struct nlattr *nest_parms;
	const struct nf_conn_tstamp *tstamp;

	nest_parms = nla_nest_start(skb, CTA_SGS | NLA_F_NESTED);
	if (!nest_parms)
		goto nla_put_failure;

	if (nla_put_be64(skb, CTA_SGS_SES_KEY, cpu_to_be64((u_int64_t)ct), CTA_SGS_SES_PAD))
		goto nla_put_failure;

	tstamp = nf_conn_tstamp_find(ct);
	if (tstamp) {
		if (nla_put_be64(skb, CTA_SGS_SES_START, cpu_to_be64((u_int64_t)tstamp->start), CTA_SGS_SES_PAD))
			goto nla_put_failure;
	}

	nla_nest_end(skb, nest_parms);

	return 0;

nla_put_failure:
	return -1;
}
#endif /* IS_ENABLED(CONFIG_BCM_SGS) */

/* netlink ct hooks */
int bcm_ctnetlink_size(const struct nf_conn *ct)
{
	return 0
#if IS_ENABLED(CONFIG_BCM_DPI)
	       + ctnetlink_dpi_size(ct)
#endif
#if IS_ENABLED(CONFIG_BCM_SGS)
	       + ctnetlink_sgs_size(ct)
#endif
	       ;
}
EXPORT_SYMBOL(bcm_ctnetlink_size);

int bcm_ctnetlink_dump(struct sk_buff *skb, const struct nf_conn *ct)
{
	int err = 0;

#if IS_ENABLED(CONFIG_BCM_DPI)
	err = ctnetlink_dump_dpi(skb, ct);
	if (err < 0)
		return err;
#endif

#if IS_ENABLED(CONFIG_BCM_SGS)
	err = ctnetlink_dump_sgs(skb, ct);
	if (err < 0)
		return err;
#endif

	return err;
}
EXPORT_SYMBOL(bcm_ctnetlink_dump);

int bcm_ctnetlink_change(struct nf_conn *ct, const struct nlattr * const cda[])
{
	int err = 0;

#if IS_ENABLED(CONFIG_BCM_DPI)
	if (cda[CTA_DPI]) {
		err = ctnetlink_change_dpi(ct, cda);
		if (err < 0)
			return err;
	}
#endif

	return err;
}
EXPORT_SYMBOL(bcm_ctnetlink_change);
