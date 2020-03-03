#ifndef _NFT_META_H_
#define _NFT_META_H_

struct nft_meta {
	enum nft_meta_keys	key:8;
	union {
		enum nft_registers	dreg:8;
		enum nft_registers	sreg:8;
	};
};

extern const struct nla_policy nft_meta_policy[];

int nft_meta_get_init(const struct nft_ctx *ctx,
		      const struct nft_expr *expr,
		      const struct nlattr * const tb[]);

int nft_meta_set_init(const struct nft_ctx *ctx,
		      const struct nft_expr *expr,
		      const struct nlattr * const tb[]);

int nft_meta_get_dump(struct sk_buff *skb,
		      const struct nft_expr *expr);

int nft_meta_set_dump(struct sk_buff *skb,
		      const struct nft_expr *expr);

void nft_meta_get_eval(const struct nft_expr *expr,
		       struct nft_regs *regs,
		       const struct nft_pktinfo *pkt);

void nft_meta_set_eval(const struct nft_expr *expr,
		       struct nft_regs *regs,
		       const struct nft_pktinfo *pkt);

#endif
