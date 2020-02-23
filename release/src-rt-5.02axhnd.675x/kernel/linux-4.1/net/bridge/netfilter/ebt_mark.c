/*
 *  ebt_mark
 *
 *	Authors:
 *	Bart De Schuymer <bdschuym@pandora.be>
 *
 *  July, 2002
 *
 */

/* The mark target can be used in any chain,
 * I believe adding a mangle table just for marking is total overkill.
 * Marking a frame doesn't really change anything in the frame anyway.
 */

#include <linux/module.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter_bridge/ebtables.h>
#include <linux/netfilter_bridge/ebt_mark_t.h>

static unsigned int
ebt_mark_tg(struct sk_buff *skb, const struct xt_action_param *par)
{
	const struct ebt_mark_t_info *info = par->targinfo;
	int action = info->target & -16;

	if (action == MARK_SET_VALUE)
		skb->mark = info->mark;
	else if (action == MARK_OR_VALUE)
		skb->mark |= info->mark;
	else if (action == MARK_AND_VALUE)
		skb->mark &= info->mark;
#if !defined(CONFIG_BCM_KF_NETFILTER)
	else
		skb->mark ^= info->mark;
#else
	else if (action == MARK_XOR_VALUE)
		skb->mark ^= info->mark;
	else
   {
		skb->vtag = (unsigned short)(info->mark);

      /* if the 8021p priority field (bits 0-3) of skb->vtag is not zero, we need
       * to do p-bit marking.
       */
      if (skb->vtag & 0xf)
      {
         unsigned short TCI = 0;

         /* if this is a vlan frame, we want to re-mark its p-bit with the 8021p
          * priority in skb->vtag.
          * if this is not a vlan frame, we want to add a 8021p tag to it, with
          * vid=0 and p-bit=the 8021p priority in skb->vtag.
          */
	      if ((skb->protocol == __constant_htons(ETH_P_8021Q)))
	      {
	              struct vlan_hdr *frame = (struct vlan_hdr *)(skb_network_header(skb));

		      TCI = ntohs(frame->h_vlan_TCI);

            /* Since the 8021p priority value in vtag had been incremented by 1,
             * we need to minus 1 from it to get the exact value.
             */
            TCI = (TCI & 0x1fff) | (((skb->vtag & 0xf) - 1) << 13);

		      frame->h_vlan_TCI = htons(TCI);
   	   }
         else
         {
	    if ((skb_mac_header(skb) - skb->head) < VLAN_HLEN)
            {
               printk("ebt_mark_tg: No headroom for VLAN tag. Marking is not done.\n");
            }
            else
            {
   	         struct vlan_ethhdr *ethHeader;

               skb->protocol = __constant_htons(ETH_P_8021Q);
               skb->mac_header -= VLAN_HLEN;
               skb->network_header -= VLAN_HLEN;
               skb->data -= VLAN_HLEN;
	            skb->len  += VLAN_HLEN;

               /* Move the mac addresses to the beginning of the new header. */
               memmove(skb_mac_header(skb), skb_mac_header(skb) + VLAN_HLEN, 2 * ETH_ALEN);

               ethHeader = (struct vlan_ethhdr *)(skb_mac_header(skb));

               ethHeader->h_vlan_proto = __constant_htons(ETH_P_8021Q);

               /* Since the 8021p priority value in vtag had been incremented by 1,
                * we need to minus 1 from it to get the exact value.
                */
               TCI = (TCI & 0x1fff) | (((skb->vtag & 0xf) - 1) << 13);

               ethHeader->h_vlan_TCI = htons(TCI);
            }
         }
         skb->vtag = 0;
      }
   }
#endif // CONFIG_BCM_KF_NETFILTER

	return info->target | ~EBT_VERDICT_BITS;
}

static int ebt_mark_tg_check(const struct xt_tgchk_param *par)
{
	const struct ebt_mark_t_info *info = par->targinfo;
	int tmp;

	tmp = info->target | ~EBT_VERDICT_BITS;
	if (BASE_CHAIN && tmp == EBT_RETURN)
		return -EINVAL;
	if (tmp < -NUM_STANDARD_TARGETS || tmp >= 0)
		return -EINVAL;
	tmp = info->target & ~EBT_VERDICT_BITS;
	if (tmp != MARK_SET_VALUE && tmp != MARK_OR_VALUE &&
#if defined(CONFIG_BCM_KF_NETFILTER)
	    tmp != MARK_AND_VALUE && tmp != MARK_XOR_VALUE &&
            tmp != VTAG_SET_VALUE)
#else
	    tmp != MARK_AND_VALUE && tmp != MARK_XOR_VALUE)
#endif
		return -EINVAL;
	return 0;
}
#ifdef CONFIG_COMPAT
struct compat_ebt_mark_t_info {
	compat_ulong_t mark;
	compat_uint_t target;
};

static void mark_tg_compat_from_user(void *dst, const void *src)
{
	const struct compat_ebt_mark_t_info *user = src;
	struct ebt_mark_t_info *kern = dst;

	kern->mark = user->mark;
	kern->target = user->target;
}

static int mark_tg_compat_to_user(void __user *dst, const void *src)
{
	struct compat_ebt_mark_t_info __user *user = dst;
	const struct ebt_mark_t_info *kern = src;

	if (put_user(kern->mark, &user->mark) ||
	    put_user(kern->target, &user->target))
		return -EFAULT;
	return 0;
}
#endif

static struct xt_target ebt_mark_tg_reg __read_mostly = {
	.name		= "mark",
	.revision	= 0,
	.family		= NFPROTO_BRIDGE,
	.target		= ebt_mark_tg,
	.checkentry	= ebt_mark_tg_check,
	.targetsize	= sizeof(struct ebt_mark_t_info),
#ifdef CONFIG_COMPAT
	.compatsize	= sizeof(struct compat_ebt_mark_t_info),
	.compat_from_user = mark_tg_compat_from_user,
	.compat_to_user	= mark_tg_compat_to_user,
#endif
	.me		= THIS_MODULE,
};

static int __init ebt_mark_init(void)
{
	return xt_register_target(&ebt_mark_tg_reg);
}

static void __exit ebt_mark_fini(void)
{
	xt_unregister_target(&ebt_mark_tg_reg);
}

module_init(ebt_mark_init);
module_exit(ebt_mark_fini);
MODULE_DESCRIPTION("Ebtables: Packet mark modification");
MODULE_LICENSE("GPL");
