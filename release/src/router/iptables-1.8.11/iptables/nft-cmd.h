#ifndef _NFT_CMD_H_
#define _NFT_CMD_H_

#include <libiptc/linux_list.h>
#include <stdbool.h>
#include "nft.h"

struct nftnl_rule;

struct nft_cmd {
	struct list_head		head;
	int				command;
	const char			*table;
	const char			*chain;
	const char			*jumpto;
	int				rulenum;
	bool				verbose;
	unsigned int			format;
	struct {
		struct nftnl_rule	*rule;
		struct nftnl_set	*set;
	} obj;
	const char			*policy;
	struct xt_counters		counters;
	uint8_t				counter_op;
	const char			*rename;
	int				counters_save;
	struct {
		unsigned int		lineno;
	} error;
};

struct nft_cmd *nft_cmd_new(struct nft_handle *h, int command,
			    const char *table, const char *chain,
			    struct iptables_command_state *state,
			    int rulenum, bool verbose);
void nft_cmd_free(struct nft_cmd *cmd);

int nft_cmd_rule_append(struct nft_handle *h, const char *chain,
			const char *table, struct iptables_command_state *state,
                        bool verbose);
int nft_cmd_rule_insert(struct nft_handle *h, const char *chain,
			const char *table, struct iptables_command_state *state,
			int rulenum, bool verbose);
int nft_cmd_rule_delete(struct nft_handle *h, const char *chain,
                        const char *table, struct iptables_command_state *state,
			bool verbose);
int nft_cmd_rule_delete_num(struct nft_handle *h, const char *chain,
			    const char *table, int rulenum, bool verbose);
int nft_cmd_rule_flush(struct nft_handle *h, const char *chain,
		       const char *table, bool verbose);
int nft_cmd_zero_counters(struct nft_handle *h, const char *chain,
			  const char *table, bool verbose);
int nft_cmd_chain_user_add(struct nft_handle *h, const char *chain,
			   const char *table);
int nft_cmd_chain_del(struct nft_handle *h, const char *chain,
		      const char *table, bool verbose);
int nft_cmd_chain_zero_counters(struct nft_handle *h, const char *chain,
				const char *table, bool verbose);
int nft_cmd_rule_list(struct nft_handle *h, const char *chain,
		      const char *table, int rulenum, unsigned int format);
int nft_cmd_rule_check(struct nft_handle *h, const char *chain,
                       const char *table, void *data, bool verbose);
int nft_cmd_chain_set(struct nft_handle *h, const char *table,
		      const char *chain, const char *policy,
		      const struct xt_counters *counters);
int nft_cmd_chain_user_rename(struct nft_handle *h,const char *chain,
			      const char *table, const char *newname);
int nft_cmd_rule_replace(struct nft_handle *h, const char *chain,
			 const char *table, void *data, int rulenum,
			 bool verbose);
int nft_cmd_table_flush(struct nft_handle *h, const char *table, bool verbose);
int nft_cmd_chain_restore(struct nft_handle *h, const char *chain,
			  const char *table);
int nft_cmd_rule_zero_counters(struct nft_handle *h, const char *chain,
			       const char *table, int rulenum);
int nft_cmd_rule_list_save(struct nft_handle *h, const char *chain,
			   const char *table, int rulenum, int counters);
int ebt_cmd_user_chain_policy(struct nft_handle *h, const char *table,
			      const char *chain, const char *policy);
int nft_cmd_rule_change_counters(struct nft_handle *h,
				 const char *chain, const char *table,
				 struct iptables_command_state *cs,
				 int rule_nr, uint8_t counter_op, bool verbose);
void nft_cmd_table_new(struct nft_handle *h, const char *table);

#endif /* _NFT_CMD_H_ */
