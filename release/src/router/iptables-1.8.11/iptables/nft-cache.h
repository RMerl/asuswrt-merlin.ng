#ifndef _NFT_CACHE_H_
#define _NFT_CACHE_H_

#include <libnftnl/chain.h>

struct nft_handle;
struct nft_chain;
struct nft_cmd;
struct builtin_table;

void nft_cache_level_set(struct nft_handle *h, int level,
			 const struct nft_cmd *cmd);
void nft_rebuild_cache(struct nft_handle *h);
void nft_release_cache(struct nft_handle *h);
void flush_chain_cache(struct nft_handle *h, const char *tablename);
int flush_rule_cache(struct nft_handle *h, const char *table,
		     struct nft_chain *c);
void nft_cache_build(struct nft_handle *h);
int nft_cache_add_chain(struct nft_handle *h, const struct builtin_table *t,
			struct nftnl_chain *c, bool fake);
int nft_cache_sort_chains(struct nft_handle *h, const char *table);

struct nft_chain *
nft_chain_find(struct nft_handle *h, const char *table, const char *chain);

struct nftnl_set_list *
nft_set_list_get(struct nft_handle *h, const char *table, const char *set);

#endif /* _NFT_CACHE_H_ */
