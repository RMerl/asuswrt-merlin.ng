#ifndef _NFT_CHAIN_H_
#define _NFT_CHAIN_H_

#include <libnftnl/chain.h>
#include <libiptc/linux_list.h>

struct nft_handle;

struct nft_chain {
	struct list_head	head;
	struct hlist_node	hnode;
	struct nft_chain	**base_slot;
	struct nftnl_chain	*nftnl;
	bool			fake;
};

#define CHAIN_NAME_HSIZE	512

struct nft_chain_list {
	struct list_head	list;
	struct hlist_head	names[CHAIN_NAME_HSIZE];
};

struct nft_chain *nft_chain_alloc(struct nftnl_chain *nftnl, bool fake);
void nft_chain_free(struct nft_chain *c);

struct nft_chain_list *nft_chain_list_alloc(void);
void nft_chain_list_free(struct nft_chain_list *list);
void nft_chain_list_del(struct nft_chain *c);

#endif /* _NFT_CHAIN_H_ */
