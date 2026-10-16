#ifndef _NFT_RULEPARSE_H_
#define _NFT_RULEPARSE_H_

#include <linux/netfilter/nf_tables.h>

#include <libnftnl/expr.h>

#include "xshared.h"

enum nft_ctx_reg_type {
	NFT_XT_REG_UNDEF,
	NFT_XT_REG_PAYLOAD,
	NFT_XT_REG_IMMEDIATE,
	NFT_XT_REG_META_DREG,
};

struct nft_xt_ctx_reg {
	enum nft_ctx_reg_type type:8;

	union {
		struct {
			uint32_t base;
			uint32_t offset;
			uint32_t len;
		} payload;
		struct {
			uint32_t data[4];
			uint8_t len;
		} immediate;
		struct {
			uint32_t key;
		} meta_dreg;
		struct {
			uint32_t key;
		} meta_sreg;
	};

	struct {
		uint32_t mask[4];
		uint32_t xor[4];
		bool set;
	} bitwise;
};

struct nft_xt_ctx {
	struct iptables_command_state *cs;
	struct nftnl_expr_iter *iter;
	struct nft_handle *h;
	uint32_t flags;
	const char *table;

	struct nft_xt_ctx_reg regs[1 + 16];

	const char *errmsg;
};

static inline struct nft_xt_ctx_reg *nft_xt_ctx_get_sreg(struct nft_xt_ctx *ctx, enum nft_registers reg)
{
	switch (reg) {
	case NFT_REG_VERDICT:
		return &ctx->regs[0];
	case NFT_REG_1:
		return &ctx->regs[1];
	case NFT_REG_2:
		return &ctx->regs[5];
	case NFT_REG_3:
		return &ctx->regs[9];
	case NFT_REG_4:
		return &ctx->regs[13];
	case NFT_REG32_00...NFT_REG32_15:
		return &ctx->regs[reg - NFT_REG32_00];
	default:
		ctx->errmsg = "Unknown register requested";
		break;
	}

	return NULL;
}

static inline void nft_xt_reg_clear(struct nft_xt_ctx_reg *r)
{
	r->type = 0;
	r->bitwise.set = false;
}

static inline struct nft_xt_ctx_reg *nft_xt_ctx_get_dreg(struct nft_xt_ctx *ctx, enum nft_registers reg)
{
	struct nft_xt_ctx_reg *r = nft_xt_ctx_get_sreg(ctx, reg);

	if (r)
		nft_xt_reg_clear(r);

	return r;
}

struct nft_ruleparse_ops {
	void (*meta)(struct nft_xt_ctx *ctx,
		     const struct nft_xt_ctx_reg *sreg,
		     struct nftnl_expr *e,
		     struct iptables_command_state *cs);
	void (*payload)(struct nft_xt_ctx *ctx,
			const struct nft_xt_ctx_reg *sreg,
			struct nftnl_expr *e,
			struct iptables_command_state *cs);
	void (*lookup)(struct nft_xt_ctx *ctx, struct nftnl_expr *e);
	void (*match)(struct xtables_match *m,
		      struct iptables_command_state *cs);
	void (*target)(struct xtables_target *t,
		       struct iptables_command_state *cs);
};

extern struct nft_ruleparse_ops nft_ruleparse_ops_arp;
extern struct nft_ruleparse_ops nft_ruleparse_ops_bridge;
extern struct nft_ruleparse_ops nft_ruleparse_ops_ipv4;
extern struct nft_ruleparse_ops nft_ruleparse_ops_ipv6;

void *nft_create_match(struct nft_xt_ctx *ctx,
		       struct iptables_command_state *cs,
		       const char *name, bool reuse);
void *nft_create_target(struct nft_xt_ctx *ctx, const char *name);


bool nft_rule_to_iptables_command_state(struct nft_handle *h,
					const struct nftnl_rule *r,
					struct iptables_command_state *cs);

#define min(x, y) ((x) < (y) ? (x) : (y))
#define max(x, y) ((x) > (y) ? (x) : (y))

int parse_meta(struct nft_xt_ctx *ctx, struct nftnl_expr *e, uint8_t key,
	       char *iniface, char *outiface, uint8_t *invflags);

int nft_parse_hl(struct nft_xt_ctx *ctx, struct nftnl_expr *e,
		 struct iptables_command_state *cs);

#endif /* _NFT_RULEPARSE_H_ */
