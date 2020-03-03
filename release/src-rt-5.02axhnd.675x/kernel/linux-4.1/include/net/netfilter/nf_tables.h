#ifndef _NET_NF_TABLES_H
#define _NET_NF_TABLES_H

#include <linux/module.h>
#include <linux/list.h>
#include <linux/netfilter.h>
#include <linux/netfilter/nfnetlink.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter/nf_tables.h>
#include <linux/u64_stats_sync.h>
#include <net/netlink.h>

#define NFT_JUMP_STACK_SIZE	16

struct nft_pktinfo {
	struct sk_buff			*skb;
	const struct net_device		*in;
	const struct net_device		*out;
	const struct nf_hook_ops	*ops;
	u8				nhoff;
	u8				thoff;
	u8				tprot;
	/* for x_tables compatibility */
	struct xt_action_param		xt;
};

static inline void nft_set_pktinfo(struct nft_pktinfo *pkt,
				   const struct nf_hook_ops *ops,
				   struct sk_buff *skb,
				   const struct nf_hook_state *state)
{
	pkt->skb = skb;
	pkt->in = pkt->xt.in = state->in;
	pkt->out = pkt->xt.out = state->out;
	pkt->ops = ops;
	pkt->xt.hooknum = ops->hooknum;
	pkt->xt.family = ops->pf;
}

/**
 * 	struct nft_verdict - nf_tables verdict
 *
 * 	@code: nf_tables/netfilter verdict code
 * 	@chain: destination chain for NFT_JUMP/NFT_GOTO
 */
struct nft_verdict {
	u32				code;
	struct nft_chain		*chain;
};

struct nft_data {
	union {
		u32			data[4];
		struct nft_verdict	verdict;
	};
} __attribute__((aligned(__alignof__(u64))));

/**
 *	struct nft_regs - nf_tables register set
 *
 *	@data: data registers
 *	@verdict: verdict register
 *
 *	The first four data registers alias to the verdict register.
 */
struct nft_regs {
	union {
		u32			data[20];
		struct nft_verdict	verdict;
	};
};

static inline void nft_data_copy(u32 *dst, const struct nft_data *src,
				 unsigned int len)
{
	memcpy(dst, src, len);
}

static inline void nft_data_debug(const struct nft_data *data)
{
	pr_debug("data[0]=%x data[1]=%x data[2]=%x data[3]=%x\n",
		 data->data[0], data->data[1],
		 data->data[2], data->data[3]);
}

/**
 *	struct nft_ctx - nf_tables rule/set context
 *
 *	@net: net namespace
 * 	@afi: address family info
 * 	@table: the table the chain is contained in
 * 	@chain: the chain the rule is contained in
 *	@nla: netlink attributes
 *	@portid: netlink portID of the original message
 *	@seq: netlink sequence number
 *	@report: notify via unicast netlink message
 */
struct nft_ctx {
	struct net			*net;
	struct nft_af_info		*afi;
	struct nft_table		*table;
	struct nft_chain		*chain;
	const struct nlattr * const 	*nla;
	u32				portid;
	u32				seq;
	bool				report;
};

struct nft_data_desc {
	enum nft_data_types		type;
	unsigned int			len;
};

int nft_data_init(const struct nft_ctx *ctx,
		  struct nft_data *data, unsigned int size,
		  struct nft_data_desc *desc, const struct nlattr *nla);
void nft_data_uninit(const struct nft_data *data, enum nft_data_types type);
int nft_data_dump(struct sk_buff *skb, int attr, const struct nft_data *data,
		  enum nft_data_types type, unsigned int len);

static inline enum nft_data_types nft_dreg_to_type(enum nft_registers reg)
{
	return reg == NFT_REG_VERDICT ? NFT_DATA_VERDICT : NFT_DATA_VALUE;
}

static inline enum nft_registers nft_type_to_reg(enum nft_data_types type)
{
	return type == NFT_DATA_VERDICT ? NFT_REG_VERDICT : NFT_REG_1 * NFT_REG_SIZE / NFT_REG32_SIZE;
}

unsigned int nft_parse_register(const struct nlattr *attr);
int nft_dump_register(struct sk_buff *skb, unsigned int attr, unsigned int reg);

int nft_validate_register_load(enum nft_registers reg, unsigned int len);
int nft_validate_register_store(const struct nft_ctx *ctx,
				enum nft_registers reg,
				const struct nft_data *data,
				enum nft_data_types type, unsigned int len);

/**
 *	struct nft_userdata - user defined data associated with an object
 *
 *	@len: length of the data
 *	@data: content
 *
 *	The presence of user data is indicated in an object specific fashion,
 *	so a length of zero can't occur and the value "len" indicates data
 *	of length len + 1.
 */
struct nft_userdata {
	u8			len;
	unsigned char		data[0];
};

/**
 *	struct nft_set_elem - generic representation of set elements
 *
 *	@key: element key
 *	@priv: element private data and extensions
 */
struct nft_set_elem {
	union {
		u32		buf[NFT_DATA_VALUE_MAXLEN / sizeof(u32)];
		struct nft_data	val;
	} key;
	void			*priv;
};

struct nft_set;
struct nft_set_iter {
	unsigned int	count;
	unsigned int	skip;
	int		err;
	int		(*fn)(const struct nft_ctx *ctx,
			      const struct nft_set *set,
			      const struct nft_set_iter *iter,
			      const struct nft_set_elem *elem);
};

/**
 *	struct nft_set_desc - description of set elements
 *
 *	@klen: key length
 *	@dlen: data length
 *	@size: number of set elements
 */
struct nft_set_desc {
	unsigned int		klen;
	unsigned int		dlen;
	unsigned int		size;
};

/**
 *	enum nft_set_class - performance class
 *
 *	@NFT_LOOKUP_O_1: constant, O(1)
 *	@NFT_LOOKUP_O_LOG_N: logarithmic, O(log N)
 *	@NFT_LOOKUP_O_N: linear, O(N)
 */
enum nft_set_class {
	NFT_SET_CLASS_O_1,
	NFT_SET_CLASS_O_LOG_N,
	NFT_SET_CLASS_O_N,
};

/**
 *	struct nft_set_estimate - estimation of memory and performance
 *				  characteristics
 *
 *	@size: required memory
 *	@class: lookup performance class
 */
struct nft_set_estimate {
	unsigned int		size;
	enum nft_set_class	class;
};

struct nft_set_ext;
struct nft_expr;

/**
 *	struct nft_set_ops - nf_tables set operations
 *
 *	@lookup: look up an element within the set
 *	@insert: insert new element into set
 *	@activate: activate new element in the next generation
 *	@deactivate: deactivate element in the next generation
 *	@remove: remove element from set
 *	@walk: iterate over all set elemeennts
 *	@privsize: function to return size of set private data
 *	@init: initialize private data of new set instance
 *	@destroy: destroy private data of set instance
 *	@list: nf_tables_set_ops list node
 *	@owner: module reference
 *	@elemsize: element private size
 *	@features: features supported by the implementation
 */
struct nft_set_ops {
	bool				(*lookup)(const struct nft_set *set,
						  const u32 *key,
						  const struct nft_set_ext **ext);
	bool				(*update)(struct nft_set *set,
						  const u32 *key,
						  void *(*new)(struct nft_set *,
							       const struct nft_expr *,
							       struct nft_regs *),
						  const struct nft_expr *expr,
						  struct nft_regs *regs,
						  const struct nft_set_ext **ext);

	int				(*insert)(const struct nft_set *set,
						  const struct nft_set_elem *elem);
	void				(*activate)(const struct nft_set *set,
						    const struct nft_set_elem *elem);
	void *				(*deactivate)(const struct nft_set *set,
						      const struct nft_set_elem *elem);
	void				(*remove)(const struct nft_set *set,
						  const struct nft_set_elem *elem);
	void				(*walk)(const struct nft_ctx *ctx,
						const struct nft_set *set,
						struct nft_set_iter *iter);

	unsigned int			(*privsize)(const struct nlattr * const nla[]);
	bool				(*estimate)(const struct nft_set_desc *desc,
						    u32 features,
						    struct nft_set_estimate *est);
	int				(*init)(const struct nft_set *set,
						const struct nft_set_desc *desc,
						const struct nlattr * const nla[]);
	void				(*destroy)(const struct nft_set *set);

	struct list_head		list;
	struct module			*owner;
	unsigned int			elemsize;
	u32				features;
};

int nft_register_set(struct nft_set_ops *ops);
void nft_unregister_set(struct nft_set_ops *ops);

/**
 * 	struct nft_set - nf_tables set instance
 *
 *	@list: table set list node
 *	@bindings: list of set bindings
 * 	@name: name of the set
 * 	@ktype: key type (numeric type defined by userspace, not used in the kernel)
 * 	@dtype: data type (verdict or numeric type defined by userspace)
 * 	@size: maximum set size
 * 	@nelems: number of elements
 * 	@ndeact: number of deactivated elements queued for removal
 * 	@timeout: default timeout value in msecs
 * 	@gc_int: garbage collection interval in msecs
 *	@policy: set parameterization (see enum nft_set_policies)
 * 	@ops: set ops
 * 	@pnet: network namespace
 * 	@flags: set flags
 * 	@klen: key length
 * 	@dlen: data length
 * 	@data: private set data
 */
struct nft_set {
	struct list_head		list;
	struct list_head		bindings;
	char				name[IFNAMSIZ];
	u32				ktype;
	u32				dtype;
	u32				size;
	atomic_t			nelems;
	u32				ndeact;
	u64				timeout;
	u32				gc_int;
	u16				policy;
	/* runtime data below here */
	const struct nft_set_ops	*ops ____cacheline_aligned;
	possible_net_t			pnet;
	u16				flags;
	u8				klen;
	u8				dlen;
	unsigned char			data[]
		__attribute__((aligned(__alignof__(u64))));
};

static inline void *nft_set_priv(const struct nft_set *set)
{
	return (void *)set->data;
}

static inline struct nft_set *nft_set_container_of(const void *priv)
{
	return (void *)priv - offsetof(struct nft_set, data);
}

struct nft_set *nf_tables_set_lookup(const struct nft_table *table,
				     const struct nlattr *nla);
struct nft_set *nf_tables_set_lookup_byid(const struct net *net,
					  const struct nlattr *nla);

static inline unsigned long nft_set_gc_interval(const struct nft_set *set)
{
	return set->gc_int ? msecs_to_jiffies(set->gc_int) : HZ;
}

/**
 *	struct nft_set_binding - nf_tables set binding
 *
 *	@list: set bindings list node
 *	@chain: chain containing the rule bound to the set
 *	@flags: set action flags
 *
 *	A set binding contains all information necessary for validation
 *	of new elements added to a bound set.
 */
struct nft_set_binding {
	struct list_head		list;
	const struct nft_chain		*chain;
	u32				flags;
};

int nf_tables_bind_set(const struct nft_ctx *ctx, struct nft_set *set,
		       struct nft_set_binding *binding);
void nf_tables_unbind_set(const struct nft_ctx *ctx, struct nft_set *set,
			  struct nft_set_binding *binding);

/**
 *	enum nft_set_extensions - set extension type IDs
 *
 *	@NFT_SET_EXT_KEY: element key
 *	@NFT_SET_EXT_DATA: mapping data
 *	@NFT_SET_EXT_FLAGS: element flags
 *	@NFT_SET_EXT_TIMEOUT: element timeout
 *	@NFT_SET_EXT_EXPIRATION: element expiration time
 *	@NFT_SET_EXT_USERDATA: user data associated with the element
 *	@NFT_SET_EXT_EXPR: expression assiociated with the element
 *	@NFT_SET_EXT_NUM: number of extension types
 */
enum nft_set_extensions {
	NFT_SET_EXT_KEY,
	NFT_SET_EXT_DATA,
	NFT_SET_EXT_FLAGS,
	NFT_SET_EXT_TIMEOUT,
	NFT_SET_EXT_EXPIRATION,
	NFT_SET_EXT_USERDATA,
	NFT_SET_EXT_EXPR,
	NFT_SET_EXT_NUM
};

/**
 *	struct nft_set_ext_type - set extension type
 *
 * 	@len: fixed part length of the extension
 * 	@align: alignment requirements of the extension
 */
struct nft_set_ext_type {
	u8	len;
	u8	align;
};

extern const struct nft_set_ext_type nft_set_ext_types[];

/**
 *	struct nft_set_ext_tmpl - set extension template
 *
 *	@len: length of extension area
 *	@offset: offsets of individual extension types
 */
struct nft_set_ext_tmpl {
	u16	len;
	u8	offset[NFT_SET_EXT_NUM];
};

/**
 *	struct nft_set_ext - set extensions
 *
 *	@genmask: generation mask
 *	@offset: offsets of individual extension types
 *	@data: beginning of extension data
 */
struct nft_set_ext {
	u8	genmask;
	u8	offset[NFT_SET_EXT_NUM];
	char	data[0];
};

static inline void nft_set_ext_prepare(struct nft_set_ext_tmpl *tmpl)
{
	memset(tmpl, 0, sizeof(*tmpl));
	tmpl->len = sizeof(struct nft_set_ext);
}

static inline void nft_set_ext_add_length(struct nft_set_ext_tmpl *tmpl, u8 id,
					  unsigned int len)
{
	tmpl->len	 = ALIGN(tmpl->len, nft_set_ext_types[id].align);
	BUG_ON(tmpl->len > U8_MAX);
	tmpl->offset[id] = tmpl->len;
	tmpl->len	+= nft_set_ext_types[id].len + len;
}

static inline void nft_set_ext_add(struct nft_set_ext_tmpl *tmpl, u8 id)
{
	nft_set_ext_add_length(tmpl, id, 0);
}

static inline void nft_set_ext_init(struct nft_set_ext *ext,
				    const struct nft_set_ext_tmpl *tmpl)
{
	memcpy(ext->offset, tmpl->offset, sizeof(ext->offset));
}

static inline bool __nft_set_ext_exists(const struct nft_set_ext *ext, u8 id)
{
	return !!ext->offset[id];
}

static inline bool nft_set_ext_exists(const struct nft_set_ext *ext, u8 id)
{
	return ext && __nft_set_ext_exists(ext, id);
}

static inline void *nft_set_ext(const struct nft_set_ext *ext, u8 id)
{
	return (void *)ext + ext->offset[id];
}

static inline struct nft_data *nft_set_ext_key(const struct nft_set_ext *ext)
{
	return nft_set_ext(ext, NFT_SET_EXT_KEY);
}

static inline struct nft_data *nft_set_ext_data(const struct nft_set_ext *ext)
{
	return nft_set_ext(ext, NFT_SET_EXT_DATA);
}

static inline u8 *nft_set_ext_flags(const struct nft_set_ext *ext)
{
	return nft_set_ext(ext, NFT_SET_EXT_FLAGS);
}

static inline u64 *nft_set_ext_timeout(const struct nft_set_ext *ext)
{
	return nft_set_ext(ext, NFT_SET_EXT_TIMEOUT);
}

static inline unsigned long *nft_set_ext_expiration(const struct nft_set_ext *ext)
{
	return nft_set_ext(ext, NFT_SET_EXT_EXPIRATION);
}

static inline struct nft_userdata *nft_set_ext_userdata(const struct nft_set_ext *ext)
{
	return nft_set_ext(ext, NFT_SET_EXT_USERDATA);
}

static inline struct nft_expr *nft_set_ext_expr(const struct nft_set_ext *ext)
{
	return nft_set_ext(ext, NFT_SET_EXT_EXPR);
}

static inline bool nft_set_elem_expired(const struct nft_set_ext *ext)
{
	return nft_set_ext_exists(ext, NFT_SET_EXT_EXPIRATION) &&
	       time_is_before_eq_jiffies(*nft_set_ext_expiration(ext));
}

static inline struct nft_set_ext *nft_set_elem_ext(const struct nft_set *set,
						   void *elem)
{
	return elem + set->ops->elemsize;
}

void *nft_set_elem_init(const struct nft_set *set,
			const struct nft_set_ext_tmpl *tmpl,
			const u32 *key, const u32 *data,
			u64 timeout, gfp_t gfp);
void nft_set_elem_destroy(const struct nft_set *set, void *elem);

/**
 *	struct nft_set_gc_batch_head - nf_tables set garbage collection batch
 *
 *	@rcu: rcu head
 *	@set: set the elements belong to
 *	@cnt: count of elements
 */
struct nft_set_gc_batch_head {
	struct rcu_head			rcu;
	const struct nft_set		*set;
	unsigned int			cnt;
};

#define NFT_SET_GC_BATCH_SIZE	((PAGE_SIZE -				  \
				  sizeof(struct nft_set_gc_batch_head)) / \
				 sizeof(void *))

/**
 *	struct nft_set_gc_batch - nf_tables set garbage collection batch
 *
 * 	@head: GC batch head
 * 	@elems: garbage collection elements
 */
struct nft_set_gc_batch {
	struct nft_set_gc_batch_head	head;
	void				*elems[NFT_SET_GC_BATCH_SIZE];
};

struct nft_set_gc_batch *nft_set_gc_batch_alloc(const struct nft_set *set,
						gfp_t gfp);
void nft_set_gc_batch_release(struct rcu_head *rcu);

static inline void nft_set_gc_batch_complete(struct nft_set_gc_batch *gcb)
{
	if (gcb != NULL)
		call_rcu(&gcb->head.rcu, nft_set_gc_batch_release);
}

static inline struct nft_set_gc_batch *
nft_set_gc_batch_check(const struct nft_set *set, struct nft_set_gc_batch *gcb,
		       gfp_t gfp)
{
	if (gcb != NULL) {
		if (gcb->head.cnt + 1 < ARRAY_SIZE(gcb->elems))
			return gcb;
		nft_set_gc_batch_complete(gcb);
	}
	return nft_set_gc_batch_alloc(set, gfp);
}

static inline void nft_set_gc_batch_add(struct nft_set_gc_batch *gcb,
					void *elem)
{
	gcb->elems[gcb->head.cnt++] = elem;
}

/**
 *	struct nft_expr_type - nf_tables expression type
 *
 *	@select_ops: function to select nft_expr_ops
 *	@ops: default ops, used when no select_ops functions is present
 *	@list: used internally
 *	@name: Identifier
 *	@owner: module reference
 *	@policy: netlink attribute policy
 *	@maxattr: highest netlink attribute number
 *	@family: address family for AF-specific types
 *	@flags: expression type flags
 */
struct nft_expr_type {
	const struct nft_expr_ops	*(*select_ops)(const struct nft_ctx *,
						       const struct nlattr * const tb[]);
	const struct nft_expr_ops	*ops;
	struct list_head		list;
	const char			*name;
	struct module			*owner;
	const struct nla_policy		*policy;
	unsigned int			maxattr;
	u8				family;
	u8				flags;
};

#define NFT_EXPR_STATEFUL		0x1

/**
 *	struct nft_expr_ops - nf_tables expression operations
 *
 *	@eval: Expression evaluation function
 *	@size: full expression size, including private data size
 *	@init: initialization function
 *	@destroy: destruction function
 *	@dump: function to dump parameters
 *	@type: expression type
 *	@validate: validate expression, called during loop detection
 *	@data: extra data to attach to this expression operation
 */
struct nft_expr;
struct nft_expr_ops {
	void				(*eval)(const struct nft_expr *expr,
						struct nft_regs *regs,
						const struct nft_pktinfo *pkt);
	unsigned int			size;

	int				(*init)(const struct nft_ctx *ctx,
						const struct nft_expr *expr,
						const struct nlattr * const tb[]);
	void				(*destroy)(const struct nft_ctx *ctx,
						   const struct nft_expr *expr);
	int				(*dump)(struct sk_buff *skb,
						const struct nft_expr *expr);
	int				(*validate)(const struct nft_ctx *ctx,
						    const struct nft_expr *expr,
						    const struct nft_data **data);
	const struct nft_expr_type	*type;
	void				*data;
};

#define NFT_EXPR_MAXATTR		16
#define NFT_EXPR_SIZE(size)		(sizeof(struct nft_expr) + \
					 ALIGN(size, __alignof__(struct nft_expr)))

/**
 *	struct nft_expr - nf_tables expression
 *
 *	@ops: expression ops
 *	@data: expression private data
 */
struct nft_expr {
	const struct nft_expr_ops	*ops;
	unsigned char			data[];
};

static inline void *nft_expr_priv(const struct nft_expr *expr)
{
	return (void *)expr->data;
}

struct nft_expr *nft_expr_init(const struct nft_ctx *ctx,
			       const struct nlattr *nla);
void nft_expr_destroy(const struct nft_ctx *ctx, struct nft_expr *expr);
int nft_expr_dump(struct sk_buff *skb, unsigned int attr,
		  const struct nft_expr *expr);

static inline void nft_expr_clone(struct nft_expr *dst, struct nft_expr *src)
{
	__module_get(src->ops->type->owner);
	memcpy(dst, src, src->ops->size);
}

/**
 *	struct nft_rule - nf_tables rule
 *
 *	@list: used internally
 *	@handle: rule handle
 *	@genmask: generation mask
 *	@dlen: length of expression data
 *	@udata: user data is appended to the rule
 *	@data: expression data
 */
struct nft_rule {
	struct list_head		list;
	u64				handle:42,
					genmask:2,
					dlen:12,
					udata:1;
	unsigned char			data[]
		__attribute__((aligned(__alignof__(struct nft_expr))));
};

static inline struct nft_expr *nft_expr_first(const struct nft_rule *rule)
{
	return (struct nft_expr *)&rule->data[0];
}

static inline struct nft_expr *nft_expr_next(const struct nft_expr *expr)
{
	return ((void *)expr) + expr->ops->size;
}

static inline struct nft_expr *nft_expr_last(const struct nft_rule *rule)
{
	return (struct nft_expr *)&rule->data[rule->dlen];
}

static inline struct nft_userdata *nft_userdata(const struct nft_rule *rule)
{
	return (void *)&rule->data[rule->dlen];
}

/*
 * The last pointer isn't really necessary, but the compiler isn't able to
 * determine that the result of nft_expr_last() is always the same since it
 * can't assume that the dlen value wasn't changed within calls in the loop.
 */
#define nft_rule_for_each_expr(expr, last, rule) \
	for ((expr) = nft_expr_first(rule), (last) = nft_expr_last(rule); \
	     (expr) != (last); \
	     (expr) = nft_expr_next(expr))

enum nft_chain_flags {
	NFT_BASE_CHAIN			= 0x1,
	NFT_CHAIN_INACTIVE		= 0x2,
};

/**
 *	struct nft_chain - nf_tables chain
 *
 *	@rules: list of rules in the chain
 *	@list: used internally
 *	@table: table that this chain belongs to
 *	@handle: chain handle
 *	@use: number of jump references to this chain
 *	@level: length of longest path to this chain
 *	@flags: bitmask of enum nft_chain_flags
 *	@name: name of the chain
 */
struct nft_chain {
	struct list_head		rules;
	struct list_head		list;
	struct nft_table		*table;
	u64				handle;
	u32				use;
	u16				level;
	u8				flags;
	char				name[NFT_CHAIN_MAXNAMELEN];
};

enum nft_chain_type {
	NFT_CHAIN_T_DEFAULT = 0,
	NFT_CHAIN_T_ROUTE,
	NFT_CHAIN_T_NAT,
	NFT_CHAIN_T_MAX
};

/**
 * 	struct nf_chain_type - nf_tables chain type info
 *
 * 	@name: name of the type
 * 	@type: numeric identifier
 * 	@family: address family
 * 	@owner: module owner
 * 	@hook_mask: mask of valid hooks
 * 	@hooks: hookfn overrides
 */
struct nf_chain_type {
	const char			*name;
	enum nft_chain_type		type;
	int				family;
	struct module			*owner;
	unsigned int			hook_mask;
	nf_hookfn			*hooks[NF_MAX_HOOKS];
};

int nft_chain_validate_dependency(const struct nft_chain *chain,
				  enum nft_chain_type type);
int nft_chain_validate_hooks(const struct nft_chain *chain,
                             unsigned int hook_flags);

struct nft_stats {
	u64			bytes;
	u64			pkts;
	struct u64_stats_sync	syncp;
};

#define NFT_HOOK_OPS_MAX		2

/**
 *	struct nft_base_chain - nf_tables base chain
 *
 *	@ops: netfilter hook ops
 *	@pnet: net namespace that this chain belongs to
 *	@type: chain type
 *	@policy: default policy
 *	@stats: per-cpu chain stats
 *	@chain: the chain
 */
struct nft_base_chain {
	struct nf_hook_ops		ops[NFT_HOOK_OPS_MAX];
	possible_net_t			pnet;
	const struct nf_chain_type	*type;
	u8				policy;
	struct nft_stats __percpu	*stats;
	struct nft_chain		chain;
};

static inline struct nft_base_chain *nft_base_chain(const struct nft_chain *chain)
{
	return container_of(chain, struct nft_base_chain, chain);
}

unsigned int nft_do_chain(struct nft_pktinfo *pkt,
			  const struct nf_hook_ops *ops);

/**
 *	struct nft_table - nf_tables table
 *
 *	@list: used internally
 *	@chains: chains in the table
 *	@sets: sets in the table
 *	@hgenerator: handle generator state
 *	@use: number of chain references to this table
 *	@flags: table flag (see enum nft_table_flags)
 *	@name: name of the table
 */
struct nft_table {
	struct list_head		list;
	struct list_head		chains;
	struct list_head		sets;
	u64				hgenerator;
	u32				use;
	u16				flags;
	char				name[NFT_TABLE_MAXNAMELEN];
};

/**
 *	struct nft_af_info - nf_tables address family info
 *
 *	@list: used internally
 *	@family: address family
 *	@nhooks: number of hooks in this family
 *	@owner: module owner
 *	@tables: used internally
 *	@nops: number of hook ops in this family
 *	@hook_ops_init: initialization function for chain hook ops
 *	@hooks: hookfn overrides for packet validation
 */
struct nft_af_info {
	struct list_head		list;
	int				family;
	unsigned int			nhooks;
	struct module			*owner;
	struct list_head		tables;
	unsigned int			nops;
	void				(*hook_ops_init)(struct nf_hook_ops *,
							 unsigned int);
	nf_hookfn			*hooks[NF_MAX_HOOKS];
};

int nft_register_afinfo(struct net *, struct nft_af_info *);
void nft_unregister_afinfo(struct nft_af_info *);

int nft_register_chain_type(const struct nf_chain_type *);
void nft_unregister_chain_type(const struct nf_chain_type *);

int nft_register_expr(struct nft_expr_type *);
void nft_unregister_expr(struct nft_expr_type *);

#define nft_dereference(p)					\
	nfnl_dereference(p, NFNL_SUBSYS_NFTABLES)

#define MODULE_ALIAS_NFT_FAMILY(family)	\
	MODULE_ALIAS("nft-afinfo-" __stringify(family))

#define MODULE_ALIAS_NFT_CHAIN(family, name) \
	MODULE_ALIAS("nft-chain-" __stringify(family) "-" name)

#define MODULE_ALIAS_NFT_AF_EXPR(family, name) \
	MODULE_ALIAS("nft-expr-" __stringify(family) "-" name)

#define MODULE_ALIAS_NFT_EXPR(name) \
	MODULE_ALIAS("nft-expr-" name)

#define MODULE_ALIAS_NFT_SET() \
	MODULE_ALIAS("nft-set")

/*
 * The gencursor defines two generations, the currently active and the
 * next one. Objects contain a bitmask of 2 bits specifying the generations
 * they're active in. A set bit means they're inactive in the generation
 * represented by that bit.
 *
 * New objects start out as inactive in the current and active in the
 * next generation. When committing the ruleset the bitmask is cleared,
 * meaning they're active in all generations. When removing an object,
 * it is set inactive in the next generation. After committing the ruleset,
 * the objects are removed.
 */
static inline unsigned int nft_gencursor_next(const struct net *net)
{
	return net->nft.gencursor + 1 == 1 ? 1 : 0;
}

static inline u8 nft_genmask_next(const struct net *net)
{
	return 1 << nft_gencursor_next(net);
}

static inline u8 nft_genmask_cur(const struct net *net)
{
	/* Use ACCESS_ONCE() to prevent refetching the value for atomicity */
	return 1 << ACCESS_ONCE(net->nft.gencursor);
}

#define NFT_GENMASK_ANY		((1 << 0) | (1 << 1))

/*
 * Set element transaction helpers
 */

static inline bool nft_set_elem_active(const struct nft_set_ext *ext,
				       u8 genmask)
{
	return !(ext->genmask & genmask);
}

static inline void nft_set_elem_change_active(const struct nft_set *set,
					      struct nft_set_ext *ext)
{
	ext->genmask ^= nft_genmask_next(read_pnet(&set->pnet));
}

/*
 * We use a free bit in the genmask field to indicate the element
 * is busy, meaning it is currently being processed either by
 * the netlink API or GC.
 *
 * Even though the genmask is only a single byte wide, this works
 * because the extension structure if fully constant once initialized,
 * so there are no non-atomic write accesses unless it is already
 * marked busy.
 */
#define NFT_SET_ELEM_BUSY_MASK	(1 << 2)

#if defined(__LITTLE_ENDIAN_BITFIELD)
#define NFT_SET_ELEM_BUSY_BIT	2
#elif defined(__BIG_ENDIAN_BITFIELD)
#define NFT_SET_ELEM_BUSY_BIT	(BITS_PER_LONG - BITS_PER_BYTE + 2)
#else
#error
#endif

static inline int nft_set_elem_mark_busy(struct nft_set_ext *ext)
{
	unsigned long *word = (unsigned long *)ext;

	BUILD_BUG_ON(offsetof(struct nft_set_ext, genmask) != 0);
	return test_and_set_bit(NFT_SET_ELEM_BUSY_BIT, word);
}

static inline void nft_set_elem_clear_busy(struct nft_set_ext *ext)
{
	unsigned long *word = (unsigned long *)ext;

	clear_bit(NFT_SET_ELEM_BUSY_BIT, word);
}

/**
 *	struct nft_trans - nf_tables object update in transaction
 *
 *	@list: used internally
 *	@msg_type: message type
 *	@ctx: transaction context
 *	@data: internal information related to the transaction
 */
struct nft_trans {
	struct list_head		list;
	int				msg_type;
	struct nft_ctx			ctx;
	char				data[0];
};

struct nft_trans_rule {
	struct nft_rule			*rule;
};

#define nft_trans_rule(trans)	\
	(((struct nft_trans_rule *)trans->data)->rule)

struct nft_trans_set {
	struct nft_set			*set;
	u32				set_id;
};

#define nft_trans_set(trans)	\
	(((struct nft_trans_set *)trans->data)->set)
#define nft_trans_set_id(trans)	\
	(((struct nft_trans_set *)trans->data)->set_id)

struct nft_trans_chain {
	bool				update;
	char				name[NFT_CHAIN_MAXNAMELEN];
	struct nft_stats __percpu	*stats;
	u8				policy;
};

#define nft_trans_chain_update(trans)	\
	(((struct nft_trans_chain *)trans->data)->update)
#define nft_trans_chain_name(trans)	\
	(((struct nft_trans_chain *)trans->data)->name)
#define nft_trans_chain_stats(trans)	\
	(((struct nft_trans_chain *)trans->data)->stats)
#define nft_trans_chain_policy(trans)	\
	(((struct nft_trans_chain *)trans->data)->policy)

struct nft_trans_table {
	bool				update;
	bool				enable;
};

#define nft_trans_table_update(trans)	\
	(((struct nft_trans_table *)trans->data)->update)
#define nft_trans_table_enable(trans)	\
	(((struct nft_trans_table *)trans->data)->enable)

struct nft_trans_elem {
	struct nft_set			*set;
	struct nft_set_elem		elem;
};

#define nft_trans_elem_set(trans)	\
	(((struct nft_trans_elem *)trans->data)->set)
#define nft_trans_elem(trans)	\
	(((struct nft_trans_elem *)trans->data)->elem)

#endif /* _NET_NF_TABLES_H */
