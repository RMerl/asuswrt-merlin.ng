/*
*    Copyright (c) 2003-2022 Broadcom
*    All Rights Reserved
*
<:label-BRCM:2022:DUAL/GPL:standard

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

#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/ipv6.h>

#include <linux/netfilter/xt_scs.h>
#include <linux/netfilter/x_tables.h>

#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv6/ip6_tables.h>
#include <net/ip.h>

#include <net/netfilter/nf_conntrack.h>

#if defined(CONFIG_BLOG)
#include <linux/blog.h>
#endif
#include <linux/bcm_skb_defines.h>
    
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Xtables: stream classification service");
MODULE_ALIAS("ipt_scs");
MODULE_ALIAS("ip6t_scs");

#if 0
#define DEBUG_SCS(fmt, ...) printk("@@%s:L%d "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define DEBUG_SCS(fmt, ...) {}
#endif

struct xt_scs_data {
    struct list_head    head;
    spinlock_t          lock;
};

static void 
scs_flow_flush(struct nf_conn *ct)
{
#if defined(CONFIG_BLOG)
	blog_lock();
	/* remove flow from flow cache if exists */
    if ((ct->bcm_ext.blog_key[IP_CT_DIR_ORIGINAL] != BLOG_KEY_FC_INVALID)
	    || (ct->bcm_ext.blog_key[IP_CT_DIR_REPLY] != BLOG_KEY_FC_INVALID)) {
        blog_notify(DESTROY_FLOWTRACK, (void*)ct,
                            (uint32_t)ct->bcm_ext.blog_key[IP_CT_DIR_ORIGINAL],
                            (uint32_t)ct->bcm_ext.blog_key[IP_CT_DIR_REPLY]);
    }
	blog_unlock();
#endif
}

/* Returns 1 if the spi is matched by the range, 0 otherwise */
static inline bool
spi_match(u_int32_t val, u_int32_t mask, u_int32_t spi, bool invert)
{
        bool r;
        DEBUG_SCS("spi_match: 0x%x %c= 0x%x/0x%x\n", spi, invert ? '!' : ' ', val, mask);
        r = ((spi & mask) == (val & mask)) ^ invert;
        DEBUG_SCS(" result %s\n", r ? "PASS" : "FAILED");
        return r;
}

// based on xt_esp.c with modification to support esp over udp
static bool scs_esp_mt(const struct sk_buff *skb, struct xt_action_param *par)
{
    const struct ip_esp_hdr *eh;
    struct ip_esp_hdr _esp;
    const struct xt_scs_mtinfo *info = par->matchinfo;

    const struct iphdr *iph = NULL;
    const struct ipv6hdr *ip6h = NULL;
    const struct udphdr *udph = NULL;
    struct udphdr _udph;
    int proto = -1;
    bool match;

    /* Must not be a fragment. */
    if (par->fragoff != 0)
            return false;

	if (xt_family(par) == NFPROTO_IPV6) {
		ip6h = ipv6_hdr(skb);
		if (!ip6h)
			return false;
		proto = ip6h->nexthdr;
		if (!par->thoff) {
			unsigned int flags = IP6_FH_F_AUTH;
			unsigned short frag_off;
			ipv6_find_hdr(skb, &par->thoff, -1, &frag_off, &flags);
			par->fragoff = frag_off;
		}
	} else {
		iph = ip_hdr(skb);
		if (!iph)
			return false;
		proto = iph->protocol;
	}

	if (proto == IPPROTO_UDP) {
		//for NAT-T
		udph = skb_header_pointer(skb, par->thoff, sizeof(_udph), &_udph);
		if (udph) {
			if (info->invflags & XT_SCS_VALID_UDPESP_L4P) {
				uint16_t l4p = htons(info->udpesp_port);
				if (udph->source == l4p || udph->dest == l4p)
					eh = skb_header_pointer(skb, par->thoff + sizeof(struct udphdr), sizeof(_esp), &_esp);
				else
					return false;
			} else { 
				if (udph->source == htons(4500) || udph->dest == htons(4500) ||
					udph->source == htons(500) || udph->dest == htons(500))
					eh = skb_header_pointer(skb, par->thoff + sizeof(struct udphdr), sizeof(_esp), &_esp);
				else
					return false;
			}
		} else {
			return false;
		}
	} else if (proto == IPPROTO_ESP) {
		//not NAT-T
		eh = skb_header_pointer(skb, par->thoff, sizeof(_esp), &_esp);
		if (!eh) {
			/* We've been asked to examine this packet, and we
			 * can't.  Hence, no choice but to drop.
			 */
			DEBUG_SCS("Dropping evil ESP tinygram.\n");
			par->hotdrop = true;
			return false;
		}
	} else {
		//not esp data
		return false;
	}

	match = spi_match(info->spis[0], info->spis[1], ntohl(eh->spi),
					 !!(info->invflags & XT_SCS_INV_SPI));
	if (proto == IPPROTO_UDP) {
		// For now if pkt is ESPoUDP don't accelerate
#if defined(CONFIG_BLOG)
		blog_skip((struct sk_buff *)skb, blog_skip_reason_nf_xt_skiplog);
#endif
		// TODO: if match, mark blog ESPoUDP flag
		// TODO: if not match, check keepalive packet, if yes don't accelerate
		// if (udph->len == htons(sizeof(struct udphdr)+1) ...
	}
	return match;
}

static bool
scs_mt(const struct sk_buff *skb, struct xt_action_param *par)
{
    const struct xt_scs_mtinfo *info = par->matchinfo;

    DEBUG_SCS("rule:%8p flags=%x up_val=%x spis[0,1]=%x,%x\n", info, info->invflags, info->up_val, info->spis[0], info->spis[1]);

    if (info->invflags & XT_SCS_VALID_UP_OVRD) {
        DEBUG_SCS("up_chk_ovrd %d\n", GET_WLAN_PRIORITY_OVRD(skb));
        if (info->invflags & XT_SCS_INV_UP_OVRD)
            return !GET_WLAN_PRIORITY_OVRD(skb);
        else
            return GET_WLAN_PRIORITY_OVRD(skb);
    }

    if (info->invflags & XT_SCS_VALID_SPIS) {
        // if match fall thru, otherwise return false
        if (!scs_esp_mt(skb, par))
            return false;
    }

    if (info->invflags & XT_SCS_VALID_UP_VAL) {
        struct xt_scs_data *data = info->data;
        struct nf_conn *ct;
        enum ip_conntrack_info ctinfo;
        
        struct sk_buff *s = (struct sk_buff*)skb;
        SET_WLAN_PRIORITY_OVRD(s, 1);
        s->mark = SKBMARK_SET_Q_PRIO(skb->mark, info->up_val);
        DEBUG_SCS("up_ovrd %x\n", info->up_val);
        
        ct = nf_ct_get(skb, &ctinfo);

        if (ct && !CT_SCS_INITED(ct)) {
            struct nf_scs *ct_scs = &ct->bcm_ext.scs;
            ct_scs->entry.next = ct_scs->entry.prev = &ct_scs->entry;

            spin_lock_bh(&data->lock);
            ct_scs->lock = &data->lock;
            list_add_tail(&ct_scs->entry, &data->head);
            spin_unlock_bh(&data->lock);
            DEBUG_SCS("add ct=%8p\n", ct);
        }
        return true;
    }
    return false;
}

static int scs_mt_check(const struct xt_mtchk_param *par)
{
    struct xt_scs_mtinfo *info = par->matchinfo;
    struct xt_scs_data *data;

    /* ct linked list is for up-set rules */
    if (!(info->invflags & XT_SCS_VALID_UP_VAL)) return 0;
    
    DEBUG_SCS("rule:%8p flags=%x up_val=%x spis[0,1]=%x,%x\n", info, info->invflags, info->up_val, info->spis[0], info->spis[1]);
    /* init private data */
    data = kmalloc(sizeof(*data), GFP_KERNEL);
    info->data = data;
    if (data) {
        data->head.next = data->head.prev = &data->head;
        spin_lock_init(&data->lock);
    }
    
    return PTR_ERR_OR_ZERO(data);
}

static void scs_mt_destroy(const struct xt_mtdtor_param *par)
{
    struct xt_scs_mtinfo *info = par->matchinfo;
    struct xt_scs_data *data = info->data;

    /* ct linked list is for up-set rules */
    if (!(info->invflags & XT_SCS_VALID_UP_VAL)) return;

    DEBUG_SCS("rule:%8p flags=%x up_val=%x spis[0,1]=%x,%x\n", info, info->invflags, info->up_val, info->spis[0], info->spis[1]);
    /* traverse list to deactivate entry, flush flow */
    if (!data) return;

    spin_lock_bh(&data->lock);
    while (!list_empty(&data->head)) {
        struct nf_conn *ct;

        ct = container_of(data->head.next, struct nf_conn, bcm_ext.scs.entry);
        list_del(&ct->bcm_ext.scs.entry);
        ct->bcm_ext.scs.lock = NULL;

        spin_unlock_bh(&data->lock);
        scs_flow_flush(ct);
        DEBUG_SCS("remove ct=%8p\n", ct);
        spin_lock_bh(&data->lock);
    }
    spin_unlock_bh(&data->lock);
    
    info->data = NULL;
    kfree(data);
}

static struct xt_match scs_mt_reg __read_mostly = {
	.name           = "scs",
	.revision       = 0,
	.family         = NFPROTO_UNSPEC,
	.checkentry     = scs_mt_check,
	.match          = scs_mt,
	.matchsize      = sizeof(struct xt_scs_mtinfo),
	.usersize       = offsetof(struct xt_scs_mtinfo, data),
	.destroy        = scs_mt_destroy,
	.me             = THIS_MODULE,
};

static int __init scs_mt_init(void)
{
    DEBUG_SCS("\n");
	return xt_register_match(&scs_mt_reg);
}

static void __exit scs_mt_exit(void)
{
    DEBUG_SCS("\n");
	xt_unregister_match(&scs_mt_reg);
}

module_init(scs_mt_init);
module_exit(scs_mt_exit);
