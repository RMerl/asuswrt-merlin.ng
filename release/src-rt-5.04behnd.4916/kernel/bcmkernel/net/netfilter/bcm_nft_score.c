/*
*    Copyright (c) 2003-2025 Broadcom
*    All Rights Reserved
*
<:label-BRCM:2024:DUAL/GPL:standard

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
#include <net/netfilter/nf_tables.h>
#include <net/netfilter/nf_conntrack.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("nft: Broadcom SCORE match support");
MODULE_ALIAS("nft_score");

#if 0
#define DEBUG_PRINTK(fmt, ...) printk("@@[%s.%d] "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define DEBUG_PRINTK(fmt, ...) {}
#endif

enum nft_score_attributes {
    NFTA_SCORE_UNSPEC,
    NFTA_SCORE_CLEAR,
    NFTA_SCORE_DCP,
    NFTA_SCORE_ORIG_BINIDX,
    NFTA_SCORE_REPLY_BINIDX,
    NFTA_SCORE_APP_GROUP_ID,
    __NFTA_SCORE_MAX,
};
#define NFTA_SCORE_MAX (__NFTA_SCORE_MAX - 1)

struct nft_score {
    uint8_t clear;
    uint8_t dcp;
    uint8_t orig_binidx;
    uint8_t reply_binidx;
    uint8_t app_group_id;
};

static const struct nla_policy nft_score_policy[NFTA_SCORE_MAX + 1] = {
    [NFTA_SCORE_CLEAR]          = { .type = NLA_U8 },
    [NFTA_SCORE_DCP]            = { .type = NLA_U8 },
    [NFTA_SCORE_ORIG_BINIDX]    = { .type = NLA_U8 },
    [NFTA_SCORE_REPLY_BINIDX]   = { .type = NLA_U8 },
    [NFTA_SCORE_APP_GROUP_ID]   = { .type = NLA_U8 },
};

inline void set_score_dcp_info_entry(ct_score_info_t* info, uint32_t dcp, uint32_t orig_binidx, uint32_t reply_binidx, uint32_t app_group_id)
{
    ct_score_entry_t *entry = &(info->entries[dcp]);
    entry->orig_binidx = orig_binidx;
    entry->reply_binidx = reply_binidx;
    entry->app_group_id = app_group_id;
    SCORE_DCP_ENTRY_SET(info, dcp);
}

inline void clr_score_dcp_info_entry(ct_score_info_t* info, uint32_t dcp)
{
    info->entries[dcp].val16 = 0;
    SCORE_DCP_ENTRY_CLR(info, dcp);
}

static void nft_score_eval(const struct nft_expr *expr, struct nft_regs *regs, const struct nft_pktinfo *pkt) {
    struct nft_score *priv = nft_expr_priv(expr);
    struct sk_buff *skb = pkt->skb;
    struct nf_conn *ct;
    enum ip_conntrack_info ctinfo;

    DEBUG_PRINTK("clear=%u, dcp=0x%x, orig_binidx=0x%x, reply_binidx=0x%x, app_grp_id=0x%x.\n",
                   priv->clear, priv->dcp, priv->orig_binidx, priv->reply_binidx, priv->app_group_id);

    ct = nf_ct_get(skb, &ctinfo);
    if (unlikely(!ct))
    {
        regs->verdict.code = NFT_BREAK;
        return;
    }

    if(!priv->clear)
    {
        if (!ct->bcm_ext.score)
        {
            ct->bcm_ext.score = kmalloc(sizeof(ct_score_info_t), GFP_KERNEL);
            if (!ct->bcm_ext.score)
            {
                printk("%s() can't allocate ct score struct!!\n", __FUNCTION__);
                regs->verdict.code = NFT_BREAK;
                return;
            }
            ct->bcm_ext.score->dcp_bmap = 0;
        }
        set_score_dcp_info_entry(ct->bcm_ext.score, priv->dcp, priv->orig_binidx, priv->reply_binidx, priv->app_group_id);
        DEBUG_PRINTK("add/set dcp=%d\n", priv->dcp);
    }
    else
    {
        if (ct->bcm_ext.score)
        {
            clr_score_dcp_info_entry(ct->bcm_ext.score, priv->dcp);
            DEBUG_PRINTK("clear dcp=%d\n", priv->dcp);
        }
    }

    regs->verdict.code = NFT_CONTINUE;
    return;

}

static int nft_score_init(const struct nft_ctx *ctx, const struct nft_expr *expr, const struct nlattr * const tb[]) {
    struct nft_score *priv = nft_expr_priv(expr);

    priv->clear = 0;
    priv->dcp = 0;
    priv->orig_binidx = 0;
    priv->reply_binidx = 0;
    priv->app_group_id = 0;

    if (tb[NFTA_SCORE_CLEAR])
    {
        priv->clear = nla_get_u8(tb[NFTA_SCORE_CLEAR]);
    }

    if (tb[NFTA_SCORE_DCP])
    {
        priv->dcp = nla_get_u8(tb[NFTA_SCORE_DCP]);
    }

    if (tb[NFTA_SCORE_ORIG_BINIDX])
    {
        priv->orig_binidx = nla_get_u8(tb[NFTA_SCORE_ORIG_BINIDX]);
    }

    if (tb[NFTA_SCORE_REPLY_BINIDX])
    {
        priv->reply_binidx = nla_get_u8(tb[NFTA_SCORE_REPLY_BINIDX]);
    }

    if (tb[NFTA_SCORE_APP_GROUP_ID])
    {
        priv->app_group_id = nla_get_u8(tb[NFTA_SCORE_APP_GROUP_ID]);
    }

    if (priv->dcp >= MAX_SCORE_DCP_INFO_ENTRIES ||
        priv->orig_binidx >= MAX_BINIDX ||
        priv->reply_binidx >= MAX_BINIDX ||
        priv->app_group_id >= MAX_APP_GROUP_ID )
    {
        DEBUG_PRINTK("parameters out of bound!!\n");
        return -EINVAL;
    }

    DEBUG_PRINTK("clear=%u, dcp=0x%x, orig_binidx=0x%x, reply_binidx=0x%x, app_grp_id=0x%x.\n",
                   priv->clear, priv->dcp, priv->orig_binidx, priv->reply_binidx, priv->app_group_id);

    return 0;

}

static int nft_score_dump(struct sk_buff *skb, const struct nft_expr *expr) {
    const struct nft_score *priv = nft_expr_priv(expr);

    if (nla_put_u8(skb, NFTA_SCORE_CLEAR, priv->clear))
        return -1;

    if (nla_put_u8(skb, NFTA_SCORE_DCP, priv->dcp))
        return -1;

    if (nla_put_u8(skb, NFTA_SCORE_ORIG_BINIDX, priv->orig_binidx))
        return -1;

    if (nla_put_u8(skb, NFTA_SCORE_REPLY_BINIDX, priv->reply_binidx))
        return -1;

    if (nla_put_u8(skb, NFTA_SCORE_APP_GROUP_ID, priv->app_group_id))
        return -1;

    DEBUG_PRINTK("clear=%u, dcp=0x%x, orig_binidx=0x%x, reply_binidx=0x%x, app_grp_id=0x%x.\n",
                   priv->clear, priv->dcp, priv->orig_binidx, priv->reply_binidx, priv->app_group_id);

    return 0;
}

static struct nft_expr_type nft_score_type;
static const struct nft_expr_ops nft_score_op = {
    .eval    = nft_score_eval,
    .size    = NFT_EXPR_SIZE(sizeof(struct nft_score)),
    .init    = nft_score_init,
    .dump    = nft_score_dump,
    .type    = &nft_score_type,
};

static struct nft_expr_type nft_score_type __read_mostly =  {
    .ops = &nft_score_op,
    .name = "score",
    .owner = THIS_MODULE,
    .policy = nft_score_policy,
    .maxattr = NFTA_SCORE_MAX,
};

static int __init nft_score_module_init(void)
{       
    return nft_register_expr(&nft_score_type);
}

static void __exit nft_score_module_exit(void)
{
    nft_unregister_expr(&nft_score_type);
}

module_init(nft_score_module_init);
module_exit(nft_score_module_exit);

