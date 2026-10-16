#ifndef _NFT_H_
#define _NFT_H_

#include "xshared.h"
#include "nft-shared.h"
#include "nft-cache.h"
#include "nft-chain.h"
#include "nft-cmd.h"
#include <libiptc/linux_list.h>

enum nft_table_type {
	NFT_TABLE_MANGLE	= 0,
	NFT_TABLE_SECURITY,
	NFT_TABLE_RAW,
	NFT_TABLE_FILTER,
	NFT_TABLE_NAT,
	NFT_TABLE_BROUTE,
};
#define NFT_TABLE_MAX	(NFT_TABLE_BROUTE + 1)

struct builtin_chain {
	const char *name;
	const char *type;
	uint32_t prio;
	uint32_t hook;
};

struct builtin_table {
	const char *name;
	enum nft_table_type type;
	struct builtin_chain chains[NF_INET_NUMHOOKS];
};

enum nft_cache_level {
	NFT_CL_TABLES,
	NFT_CL_CHAINS,
	NFT_CL_SETS,
	NFT_CL_RULES,
	NFT_CL_FAKE	/* must be last entry */
};

struct nft_cache {
	struct {
		struct nft_chain	*base_chains[NF_INET_NUMHOOKS];
		struct nft_chain_list	*chains;
		struct nftnl_set_list	*sets;
		bool			exists;
		bool			sorted;
		bool			tainted;
	} table[NFT_TABLE_MAX];
};

enum obj_update_type {
	NFT_COMPAT_TABLE_ADD,
	NFT_COMPAT_TABLE_FLUSH,
	NFT_COMPAT_CHAIN_ADD,
	NFT_COMPAT_CHAIN_USER_ADD,
	NFT_COMPAT_CHAIN_DEL,
	NFT_COMPAT_CHAIN_USER_FLUSH,
	NFT_COMPAT_CHAIN_UPDATE,
	NFT_COMPAT_CHAIN_RENAME,
	NFT_COMPAT_CHAIN_ZERO,
	NFT_COMPAT_RULE_APPEND,
	NFT_COMPAT_RULE_INSERT,
	NFT_COMPAT_RULE_REPLACE,
	NFT_COMPAT_RULE_DELETE,
	NFT_COMPAT_RULE_FLUSH,
	NFT_COMPAT_SET_ADD,
	NFT_COMPAT_RULE_LIST,
	NFT_COMPAT_RULE_CHECK,
	NFT_COMPAT_CHAIN_RESTORE,
	NFT_COMPAT_RULE_SAVE,
	NFT_COMPAT_RULE_ZERO,
	NFT_COMPAT_BRIDGE_USER_CHAIN_UPDATE,
	NFT_COMPAT_RULE_CHANGE_COUNTERS,
};

struct cache_chain {
	struct list_head head;
	char *name;
};

struct nft_cache_req {
	enum nft_cache_level	level;
	char			*table;
	bool			all_chains;
	struct list_head	chain_list;
};

struct nft_handle {
	int			family;
	struct mnl_socket	*nl;
	int			nlsndbuffsiz;
	int			nlrcvbuffsiz;
	uint32_t		portid;
	uint32_t		seq;
	uint32_t		nft_genid;
	uint32_t		rule_id;
	struct list_head	obj_list;
	int			obj_list_num;
	struct nftnl_batch	*batch;
	struct list_head	err_list;
	struct nft_family_ops	*ops;
	const struct builtin_table *tables;
	unsigned int		cache_index;
	struct nft_cache	__cache[2];
	struct nft_cache	*cache;
	struct nft_cache_req	cache_req;
	bool			restore;
	bool			noflush;
	int8_t			config_done;
	struct list_head	cmd_list;
	bool			cache_init;
	int			verbose;

	/* meta data, for error reporting */
	struct {
		unsigned int	lineno;
	} error;
};

int mnl_talk(struct nft_handle *h, struct nlmsghdr *nlh,
	     int (*cb)(const struct nlmsghdr *nlh, void *data),
	     void *data);
int nft_init(struct nft_handle *h, int family);
void nft_fini(struct nft_handle *h);
int nft_restart(struct nft_handle *h);

/*
 * Operations with tables.
 */
struct nftnl_table;
struct nftnl_chain_list;

int nft_for_each_table(struct nft_handle *h, int (*func)(struct nft_handle *h, const char *tablename, void *data), void *data);
bool nft_table_find(struct nft_handle *h, const char *tablename);
int nft_table_purge_chains(struct nft_handle *h, const char *table, struct nftnl_chain_list *list);
int nft_table_flush(struct nft_handle *h, const char *table);
const struct builtin_table *nft_table_builtin_find(struct nft_handle *h, const char *table);
int nft_xt_fake_builtin_chains(struct nft_handle *h, const char *table, const char *chain);

/*
 * Operations with chains.
 */
struct nftnl_chain;

int nft_chain_set(struct nft_handle *h, const char *table, const char *chain, const char *policy, const struct xt_counters *counters);
int nft_chain_save(struct nft_chain *c, void *data);
int nft_chain_user_add(struct nft_handle *h, const char *chain, const char *table);
int nft_chain_del(struct nft_handle *h, const char *chain, const char *table, bool verbose);
int nft_chain_restore(struct nft_handle *h, const char *chain, const char *table);
int nft_chain_user_rename(struct nft_handle *h, const char *chain, const char *table, const char *newname);
int nft_chain_zero_counters(struct nft_handle *h, const char *chain, const char *table, bool verbose);
const struct builtin_chain *nft_chain_builtin_find(const struct builtin_table *t, const char *chain);
bool nft_chain_exists(struct nft_handle *h, const char *table, const char *chain);
void nft_bridge_chain_postprocess(struct nft_handle *h,
				  struct nftnl_chain *c);
int nft_chain_foreach(struct nft_handle *h, const char *table,
		      int (*cb)(struct nft_chain *c, void *data),
		      void *data);


/*
 * Operations with sets.
 */
struct nftnl_set *nft_set_batch_lookup_byid(struct nft_handle *h,
					    uint32_t set_id);

/*
 * Operations with rule-set.
 */
struct nft_rule_ctx {
	int			command;
};

struct nftnl_rule *nft_rule_new(struct nft_handle *h, struct nft_rule_ctx *rule, const char *chain, const char *table, struct iptables_command_state *cs);
int nft_rule_append(struct nft_handle *h, const char *chain, const char *table, struct nftnl_rule *r, struct nftnl_rule *ref, bool verbose);
int nft_rule_insert(struct nft_handle *h, const char *chain, const char *table, struct nftnl_rule *r, int rulenum, bool verbose);
int nft_rule_check(struct nft_handle *h, const char *chain, const char *table, struct nftnl_rule *r, bool verbose);
int nft_rule_delete(struct nft_handle *h, const char *chain, const char *table, struct nftnl_rule *r, bool verbose);
int nft_rule_delete_num(struct nft_handle *h, const char *chain, const char *table, int rulenum, bool verbose);
int nft_rule_replace(struct nft_handle *h, const char *chain, const char *table, struct nftnl_rule *r, int rulenum, bool verbose);
int nft_rule_list(struct nft_handle *h, const char *chain, const char *table, int rulenum, unsigned int format);
int nft_rule_list_save(struct nft_handle *h, const char *chain, const char *table, int rulenum, int counters);
int nft_rule_save(struct nft_handle *h, const char *table, unsigned int format);
int nft_rule_flush(struct nft_handle *h, const char *chain, const char *table, bool verbose);
int nft_rule_zero_counters(struct nft_handle *h, const char *chain, const char *table, int rulenum);
bool nft_rule_is_policy_rule(struct nftnl_rule *r);

/*
 * Operations used in userspace tools
 */
int add_counters(struct nftnl_rule *r, uint64_t packets, uint64_t bytes);
int add_verdict(struct nftnl_rule *r, int verdict);
int add_match(struct nft_handle *h, struct nft_rule_ctx *ctx,
	      struct nftnl_rule *r, struct xt_entry_match *m);
int add_target(struct nftnl_rule *r, struct xt_entry_target *t);
int add_jumpto(struct nftnl_rule *r, const char *name, int verdict);
int add_action(struct nftnl_rule *r, struct iptables_command_state *cs, bool goto_set);
int add_log(struct nftnl_rule *r, struct iptables_command_state *cs);
char *get_comment(const void *data, uint32_t data_len);

enum nft_rule_print {
	NFT_RULE_APPEND,
	NFT_RULE_DEL,
};

bool nft_rule_print_save(struct nft_handle *h, const struct nftnl_rule *r,
			 enum nft_rule_print type, unsigned int format);

uint32_t nft_invflags2cmp(uint32_t invflags, uint32_t flag);

/*
 * global commit and abort
 */
int nft_commit(struct nft_handle *h);
int nft_bridge_commit(struct nft_handle *h);
int nft_abort(struct nft_handle *h);

/*
 * revision compatibility.
 */
int nft_compatible_revision(const char *name, uint8_t rev, int opt);

/*
 * Error reporting.
 */
const char *nft_strerror(int err);

/* For xtables.c */
int do_commandx(struct nft_handle *h, int argc, char *argv[], char **table, bool restore);
/* For xtables-arptables.c */
int nft_init_arp(struct nft_handle *h, const char *pname);
int do_commandarp(struct nft_handle *h, int argc, char *argv[], char **table, bool restore);
/* For xtables-eb.c */
int nft_init_eb(struct nft_handle *h, const char *pname);
void nft_fini_eb(struct nft_handle *h);
int do_commandeb(struct nft_handle *h, int argc, char *argv[], char **table, bool restore);

/*
 * Translation from iptables to nft
 */
struct xt_buf;

bool xlate_find_match(const struct iptables_command_state *cs, const char *p_name);
bool xlate_find_protomatch(const struct iptables_command_state *cs, uint16_t proto);
int xlate_matches(const struct iptables_command_state *cs, struct xt_xlate *xl);
int xlate_action(const struct iptables_command_state *cs, bool goto_set,
		 struct xt_xlate *xl);
void xlate_ifname(struct xt_xlate *xl, const char *nftmeta, const char *ifname,
		  bool invert);

/*
 * ARP
 */

struct arpt_entry;

int nft_arp_rule_append(struct nft_handle *h, const char *chain,
			const char *table, struct arpt_entry *fw,
			bool verbose);
int nft_arp_rule_insert(struct nft_handle *h, const char *chain,
			const char *table, struct arpt_entry *fw,
			int rulenum, bool verbose);

void nft_rule_to_arpt_entry(struct nftnl_rule *r, struct arpt_entry *fw);

bool nft_is_table_compatible(struct nft_handle *h,
			     const char *table, const char *chain);
bool nft_is_table_tainted(struct nft_handle *h, const char *table);
void nft_assert_table_compatible(struct nft_handle *h,
				 const char *table, const char *chain);

int ebt_set_user_chain_policy(struct nft_handle *h, const char *table,
			      const char *chain, const char *policy);

#endif
