/*
 *	ebt_u32 - kernel module to match u32 packet content
 *
 *	Original author: Don Cohen <don@isis.cs3-inc.com>
 *	(C) CC Computer Consultants GmbH, 2007
 *
 *    extend it by Broadcom at Jan, 2019
 */


#include <linux/in.h>
#include <linux/module.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter_bridge/ebtables.h>
#include <linux/netfilter_bridge/ebt_u32.h>
static bool u32_match_it(const struct ebt_u32_info *data,
			 const struct sk_buff *skb)
{
	const struct ebt_u32_test *ct;
	unsigned int testind;
	unsigned int nnums;
	unsigned int nvals;
	unsigned int i;
	__be32 n;
	u_int32_t pos;
	u_int32_t val;
	u_int32_t at;
	unsigned char * header;
	u_int32_t length = skb->mac_len + skb->len - skb->data_len;

	/* In iptable:
	 * Small example: "0 >> 28 == 4 && 8 & 0xFF0000 >> 16 = 6, 17"
	 * (=IPv4 and (TCP or UDP)). Outer loop runs over the "&&" operands.
	 * In ebtable, the start & mask = range, start from Mac Header.
	 */
	for (testind = 0; testind < data->ntests; ++testind) {
		ct  = &data->tests[testind];
		at  = 0;
		pos = ct->location[0].number;

		if (length < 4 || pos > length - 4)
			return false;

		header = (unsigned char *)skb_mac_header(skb);
		memcpy(&n, header + pos, sizeof(n));

		val   = ntohl(n);
		nnums = ct->nnums;

		/* Inner loop runs over "&", "<<", ">>" and "@" operands */
		for (i = 1; i < nnums; ++i) {
			u_int32_t number = ct->location[i].number;
			switch (ct->location[i].nextop) {
			case EBT_U32_AND:
				val &= number;
				break;
			case EBT_U32_LEFTSH:
				val <<= number;
				break;
			case EBT_U32_RIGHTSH:
				val >>= number;
				break;
			case EBT_U32_AT:
				if (at + val < at)
					return false;
				at += val;
				pos = number;
				if (at + 4 < at || length < at + 4 ||
				    pos > length - at - 4)
					return false;

				memcpy(&n, header + at + pos, sizeof(n));
				val = ntohl(n);
				break;
			}
		}

		/* Run over the "," and ":" operands */
		nvals = ct->nvalues;
		for (i = 0; i < nvals; ++i)
			if (ct->value[i].min <= val && val <= ct->value[i].max)
				break;

		if (i >= ct->nvalues)
			return false;
	}

	return true;
}


static bool
ebt_u32_mt(const struct sk_buff *skb, struct xt_action_param *par)
{
	const struct ebt_u32_info *data = par->matchinfo;
	bool ret;

	ret = u32_match_it(data, skb);
	return ret ^ data->invert;
}

static int ebt_u32_mt_check(const struct xt_mtchk_param *par)
{
	return 0;
}

static struct xt_match ebt_u32_mt_reg __read_mostly = {
	.name		= EBT_U32_MATCH,
	.revision	= 0,
	.family		= NFPROTO_BRIDGE,
	.match		= ebt_u32_mt,
	.checkentry	= ebt_u32_mt_check,
	.matchsize	= sizeof(struct ebt_u32_info),
	.me		= THIS_MODULE,
};

static int __init ebt_u32_init(void)
{
	return xt_register_match(&ebt_u32_mt_reg);
}

static void __exit ebt_u32_fini(void)
{
	xt_unregister_match(&ebt_u32_mt_reg);
}

module_init(ebt_u32_init);
module_exit(ebt_u32_fini);
MODULE_DESCRIPTION("Ebtables: u32 regress match");
MODULE_LICENSE("GPL");
