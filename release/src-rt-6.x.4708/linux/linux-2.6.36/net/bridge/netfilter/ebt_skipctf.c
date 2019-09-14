#include <linux/module.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter_bridge/ebtables.h>
#ifdef HNDCTF
#include <ctf/hndctf.h>
#endif /* HNDCTF */

static unsigned int
ebt_skipctf_tg(struct sk_buff *skb, const struct xt_action_param *par)
{
#ifdef HNDCTF
	PKTSETSKIPCT(NULL, skb);
#endif /* HNDCTF */

	return EBT_CONTINUE;
}

static struct xt_target ebt_skipctf_tg_reg __read_mostly = {
	.name		= "skipctf",
	.revision	= 0,
	.family		= NFPROTO_BRIDGE,
	.target		= ebt_skipctf_tg,
	.me		= THIS_MODULE,
};

static int __init ebt_skipctf_init(void)
{
	return xt_register_target(&ebt_skipctf_tg_reg);
}

static void __exit ebt_skipctf_fini(void)
{
	xt_unregister_target(&ebt_skipctf_tg_reg);
}

module_init(ebt_skipctf_init);
module_exit(ebt_skipctf_fini);
MODULE_DESCRIPTION("Ebtables: skipctf target");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom");
