/*
 * Copyright (c) 2020  Red Hat GmbH.  Author: Phil Sutter <phil@nwl.cc>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <stdlib.h>
#include <xtables.h>

#include "nft-chain.h"

struct nft_chain *nft_chain_alloc(struct nftnl_chain *nftnl, bool fake)
{
	struct nft_chain *c = xtables_malloc(sizeof(*c));

	INIT_LIST_HEAD(&c->head);
	c->nftnl = nftnl;
	c->fake = fake;

	return c;
}

void nft_chain_free(struct nft_chain *c)
{
	if (c->nftnl)
		nftnl_chain_free(c->nftnl);
	free(c);
}

struct nft_chain_list *nft_chain_list_alloc(void)
{
	struct nft_chain_list *list = xtables_malloc(sizeof(*list));
	int i;

	INIT_LIST_HEAD(&list->list);
	for (i = 0; i < CHAIN_NAME_HSIZE; i++)
		INIT_HLIST_HEAD(&list->names[i]);

	return list;
}

void nft_chain_list_del(struct nft_chain *c)
{
	list_del(&c->head);
	hlist_del(&c->hnode);
}

void nft_chain_list_free(struct nft_chain_list *list)
{
	struct nft_chain *c, *c2;

	list_for_each_entry_safe(c, c2, &list->list, head) {
		nft_chain_list_del(c);
		nft_chain_free(c);
	}
	free(list);
}
