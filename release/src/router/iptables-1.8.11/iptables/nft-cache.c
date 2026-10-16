/*
 * (C) 2012 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This code has been sponsored by Sophos Astaro <http://www.sophos.com>
 */

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <xtables.h>

#include <linux/netfilter/nf_tables.h>

#include <libmnl/libmnl.h>
#include <libnftnl/gen.h>
#include <libnftnl/set.h>
#include <libnftnl/table.h>

#include "nft.h"
#include "nft-cache.h"
#include "nft-chain.h"

/* users may define NDEBUG */
static void assert_nft_restart(struct nft_handle *h)
{
	int rc = nft_restart(h);

	assert(rc >= 0);
}

static void cache_chain_list_insert(struct list_head *list, const char *name)
{
	struct cache_chain *pos = NULL, *new;

	list_for_each_entry(pos, list, head) {
		int cmp = strcmp(pos->name, name);

		if (!cmp)
			return;
		if (cmp > 0)
			break;
	}

	new = xtables_malloc(sizeof(*new));
	new->name = xtables_strdup(name);
	list_add_tail(&new->head, pos ? &pos->head : list);
}

void nft_cache_level_set(struct nft_handle *h, int level,
			 const struct nft_cmd *cmd)
{
	struct nft_cache_req *req = &h->cache_req;

	if (level > req->level)
		req->level = level;

	if (!cmd || !cmd->table || req->all_chains)
		return;

	if (!req->table)
		req->table = xtables_strdup(cmd->table);
	else
		assert(!strcmp(req->table, cmd->table));

	if (!cmd->chain) {
		req->all_chains = true;
		return;
	}

	cache_chain_list_insert(&req->chain_list, cmd->chain);
	if (cmd->rename)
		cache_chain_list_insert(&req->chain_list, cmd->rename);
	if (cmd->jumpto)
		cache_chain_list_insert(&req->chain_list, cmd->jumpto);
}

static int genid_cb(const struct nlmsghdr *nlh, void *data)
{
	uint32_t *genid = data;
	struct nftnl_gen *gen;

	gen = nftnl_gen_alloc();
	if (!gen)
		return MNL_CB_ERROR;

	if (nftnl_gen_nlmsg_parse(nlh, gen) < 0)
		goto out;

	*genid = nftnl_gen_get_u32(gen, NFTNL_GEN_ID);

	nftnl_gen_free(gen);
	return MNL_CB_STOP;
out:
	nftnl_gen_free(gen);
	return MNL_CB_ERROR;
}

static void mnl_genid_get(struct nft_handle *h, uint32_t *genid)
{
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	int ret;

	nlh = nftnl_nlmsg_build_hdr(buf, NFT_MSG_GETGEN, 0, 0, h->seq);
	ret = mnl_talk(h, nlh, genid_cb, genid);
	if (ret == 0)
		return;

	xtables_error(RESOURCE_PROBLEM,
		      "Could not fetch rule set generation id: %s",
		      nft_strerror(errno));
}

static int nftnl_table_list_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nftnl_table *nftnl = nftnl_table_alloc();
	const struct builtin_table *t;
	struct nft_handle *h = data;
	const char *name;

	if (!nftnl)
		return MNL_CB_OK;

	if (nftnl_table_nlmsg_parse(nlh, nftnl) < 0)
		goto out;

	name = nftnl_table_get_str(nftnl, NFTNL_TABLE_NAME);
	if (!name)
		goto out;

	t = nft_table_builtin_find(h, name);
	if (!t)
		goto out;

	h->cache->table[t->type].exists = true;
out:
	nftnl_table_free(nftnl);
	return MNL_CB_OK;
}

static int fetch_table_cache(struct nft_handle *h)
{
	struct nlmsghdr *nlh;
	char buf[16536];
	int i, ret;

	nlh = nftnl_nlmsg_build_hdr(buf, NFT_MSG_GETTABLE, h->family,
				    NLM_F_DUMP, h->seq);

	ret = mnl_talk(h, nlh, nftnl_table_list_cb, h);
	if (ret < 0 && errno == EINTR)
		assert_nft_restart(h);

	for (i = 0; i < NFT_TABLE_MAX; i++) {
		enum nft_table_type type = h->tables[i].type;

		if (!h->tables[i].name)
			continue;

		h->cache->table[type].chains = nft_chain_list_alloc();

		h->cache->table[type].sets = nftnl_set_list_alloc();
		if (!h->cache->table[type].sets)
			return 0;
	}

	return 1;
}

static uint32_t djb_hash(const char *key)
{
	uint32_t i, hash = 5381;

	for (i = 0; i < strlen(key); i++)
		hash = ((hash << 5) + hash) + key[i];

	return hash;
}

static struct hlist_head *chain_name_hlist(struct nft_handle *h,
					   const struct builtin_table *t,
					   const char *chain)
{
	int key = djb_hash(chain) % CHAIN_NAME_HSIZE;

	return &h->cache->table[t->type].chains->names[key];
}

struct nft_chain *
nft_chain_find(struct nft_handle *h, const char *table, const char *chain)
{
	const struct builtin_table *t;
	struct hlist_node *node;
	struct nft_chain *c;

	t = nft_table_builtin_find(h, table);
	if (!t)
		return NULL;

	hlist_for_each_entry(c, node, chain_name_hlist(h, t, chain), hnode) {
		if (!strcmp(nftnl_chain_get_str(c->nftnl, NFTNL_CHAIN_NAME),
			    chain))
			return c;
	}
	return NULL;
}

static int
nft_cache_add_base_chain(struct nft_handle *h, const struct builtin_table *t,
			 struct nft_chain *nc)
{
	uint32_t hooknum = nftnl_chain_get_u32(nc->nftnl, NFTNL_CHAIN_HOOKNUM);
	const char *name = nftnl_chain_get_str(nc->nftnl, NFTNL_CHAIN_NAME);
	const char *type = nftnl_chain_get_str(nc->nftnl, NFTNL_CHAIN_TYPE);
	uint32_t prio = nftnl_chain_get_u32(nc->nftnl, NFTNL_CHAIN_PRIO);
	const struct builtin_chain *bc = NULL;
	int i;

	for (i = 0; i < NF_IP_NUMHOOKS && t->chains[i].name != NULL; i++) {
		if (hooknum == t->chains[i].hook) {
			bc = &t->chains[i];
			break;
		}
	}

	if (!bc ||
	    prio != bc->prio ||
	    strcmp(name, bc->name) ||
	    strcmp(type, bc->type))
		return -EINVAL;

	nc->base_slot = &h->cache->table[t->type].base_chains[hooknum];
	if (*nc->base_slot)
		return -EEXIST;

	*nc->base_slot = nc;
	return 0;
}

int nft_cache_add_chain(struct nft_handle *h, const struct builtin_table *t,
			struct nftnl_chain *c, bool fake)
{
	const char *cname = nftnl_chain_get_str(c, NFTNL_CHAIN_NAME);
	struct nft_chain *nc = nft_chain_alloc(c, fake);
	int ret;

	if (nftnl_chain_is_set(c, NFTNL_CHAIN_HOOKNUM)) {
		ret = nft_cache_add_base_chain(h, t, nc);
		if (ret) {
			h->cache->table[t->type].tainted = true;
			nft_chain_free(nc);
			return ret;
		}
	} else {
		list_add_tail(&nc->head,
			      &h->cache->table[t->type].chains->list);
	}
	hlist_add_head(&nc->hnode, chain_name_hlist(h, t, cname));
	return 0;
}

static void __nft_chain_list_sort(struct list_head *list,
				  int (*cmp)(struct nft_chain *a,
					     struct nft_chain *b))
{
	struct nft_chain *pivot, *cur, *sav;
	LIST_HEAD(sublist);

	if (list_empty(list))
		return;

	/* grab first item as pivot (dividing) value */
	pivot = list_entry(list->next, struct nft_chain, head);
	list_del(&pivot->head);

	/* move any smaller value into sublist */
	list_for_each_entry_safe(cur, sav, list, head) {
		if (cmp(pivot, cur) > 0) {
			list_del(&cur->head);
			list_add_tail(&cur->head, &sublist);
		}
	}
	/* conquer divided */
	__nft_chain_list_sort(&sublist, cmp);
	__nft_chain_list_sort(list, cmp);

	/* merge divided and pivot again */
	list_add_tail(&pivot->head, &sublist);
	list_splice(&sublist, list);
}

static int nft_chain_cmp_byname(struct nft_chain *a, struct nft_chain *b)
{
	const char *aname = nftnl_chain_get_str(a->nftnl, NFTNL_CHAIN_NAME);
	const char *bname = nftnl_chain_get_str(b->nftnl, NFTNL_CHAIN_NAME);

	return strcmp(aname, bname);
}

int nft_cache_sort_chains(struct nft_handle *h, const char *table)
{
	const struct builtin_table *t = nft_table_builtin_find(h, table);

	if (!t)
		return -1;

	if (h->cache->table[t->type].sorted)
		return 0;

	__nft_chain_list_sort(&h->cache->table[t->type].chains->list,
			      nft_chain_cmp_byname);
	h->cache->table[t->type].sorted = true;
	return 0;
}

struct nftnl_chain_list_cb_data {
	struct nft_handle *h;
	const struct builtin_table *t;
};

static int nftnl_chain_list_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nftnl_chain_list_cb_data *d = data;
	const struct builtin_table *t = d->t;
	struct nft_handle *h = d->h;
	struct nftnl_chain *c;
	const char *tname;

	c = nftnl_chain_alloc();
	if (c == NULL)
		goto err;

	if (nftnl_chain_nlmsg_parse(nlh, c) < 0)
		goto out;

	tname = nftnl_chain_get_str(c, NFTNL_CHAIN_TABLE);

	if (!t) {
		t = nft_table_builtin_find(h, tname);
		if (!t)
			goto out;
	} else if (strcmp(t->name, tname)) {
		goto out;
	}

	nft_cache_add_chain(h, t, c, false);
	return MNL_CB_OK;
out:
	nftnl_chain_free(c);
err:
	return MNL_CB_OK;
}

struct nftnl_set_list_cb_data {
	struct nft_handle *h;
	const struct builtin_table *t;
};

static int nftnl_set_list_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nftnl_set_list_cb_data *d = data;
	const struct builtin_table *t = d->t;
	struct nftnl_set_list *list;
	struct nft_handle *h = d->h;
	const char *tname, *sname;
	struct nftnl_set *s;

	s = nftnl_set_alloc();
	if (s == NULL)
		return MNL_CB_OK;

	if (nftnl_set_nlmsg_parse(nlh, s) < 0)
		goto out_free;

	tname = nftnl_set_get_str(s, NFTNL_SET_TABLE);

	if (!t)
		t = nft_table_builtin_find(h, tname);
	else if (strcmp(t->name, tname))
		goto out_free;

	if (!t)
		goto out_free;

	list = h->cache->table[t->type].sets;
	sname = nftnl_set_get_str(s, NFTNL_SET_NAME);

	if (nftnl_set_list_lookup_byname(list, sname))
		goto out_free;

	nftnl_set_list_add_tail(s, list);

	return MNL_CB_OK;
out_free:
	nftnl_set_free(s);
	return MNL_CB_OK;
}

static int set_elem_cb(const struct nlmsghdr *nlh, void *data)
{
	return nftnl_set_elems_nlmsg_parse(nlh, data) ? -1 : MNL_CB_OK;
}

static bool set_has_elements(struct nftnl_set *s)
{
	struct nftnl_set_elems_iter *iter;
	bool ret = false;

	iter = nftnl_set_elems_iter_create(s);
	if (iter) {
		ret = !!nftnl_set_elems_iter_cur(iter);
		nftnl_set_elems_iter_destroy(iter);
	}
	return ret;
}

static int set_fetch_elem_cb(struct nftnl_set *s, void *data)
{
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nft_handle *h = data;
	struct nlmsghdr *nlh;
	int ret;

	if (set_has_elements(s))
		return 0;

	nlh = nftnl_nlmsg_build_hdr(buf, NFT_MSG_GETSETELEM, h->family,
				    NLM_F_DUMP, h->seq);
	nftnl_set_elems_nlmsg_build_payload(nlh, s);

	ret = mnl_talk(h, nlh, set_elem_cb, s);

	if (!ret && h->verbose > 1) {
		fprintf(stdout, "set ");
		nftnl_set_fprintf(stdout, s, 0, 0);
		fprintf(stdout, "\n");
	}
	return ret;
}

static int fetch_set_cache(struct nft_handle *h,
			   const struct builtin_table *t, const char *set)
{
	struct nftnl_set_list_cb_data d = {
		.h = h,
		.t = t,
	};
	uint16_t flags = NLM_F_DUMP;
	struct nftnl_set *s = NULL;
	struct nlmsghdr *nlh;
	char buf[16536];
	int i, ret;

	if (t) {
		s = nftnl_set_alloc();
		if (!s)
			return -1;

		nftnl_set_set_str(s, NFTNL_SET_TABLE, t->name);

		if (set) {
			nftnl_set_set_str(s, NFTNL_SET_NAME, set);
			flags = NLM_F_ACK;
		}
	}

	nlh = nftnl_nlmsg_build_hdr(buf, NFT_MSG_GETSET,
				    h->family, flags, h->seq);

	if (s) {
		nftnl_set_nlmsg_build_payload(nlh, s);
		nftnl_set_free(s);
	}

	ret = mnl_talk(h, nlh, nftnl_set_list_cb, &d);
	if (ret < 0 && errno == EINTR) {
		assert_nft_restart(h);
		return ret;
	}

	if (t) {
		nftnl_set_list_foreach(h->cache->table[t->type].sets,
				       set_fetch_elem_cb, h);
	} else {
		for (i = 0; i < NFT_TABLE_MAX; i++) {
			enum nft_table_type type = h->tables[i].type;

			if (!h->tables[i].name)
				continue;

			nftnl_set_list_foreach(h->cache->table[type].sets,
					       set_fetch_elem_cb, h);
		}
	}
	return ret;
}

static int __fetch_chain_cache(struct nft_handle *h,
			       const struct builtin_table *t,
			       const struct nftnl_chain *c)
{
	struct nftnl_chain_list_cb_data d = {
		.h = h,
		.t = t,
	};
	char buf[16536];
	struct nlmsghdr *nlh;
	int ret;

	nlh = nftnl_nlmsg_build_hdr(buf, NFT_MSG_GETCHAIN, h->family,
				    c ? NLM_F_ACK : NLM_F_DUMP, h->seq);
	if (c)
		nftnl_chain_nlmsg_build_payload(nlh, c);

	ret = mnl_talk(h, nlh, nftnl_chain_list_cb, &d);
	if (ret < 0 && errno == EINTR)
		assert_nft_restart(h);

	return ret;
}

static int fetch_chain_cache(struct nft_handle *h,
			     const struct builtin_table *t,
			     struct list_head *chains)
{
	struct cache_chain *cc;
	struct nftnl_chain *c;
	int rc, ret = 0;

	if (!chains)
		return __fetch_chain_cache(h, t, NULL);

	assert(t);

	c = nftnl_chain_alloc();
	if (!c)
		return -1;

	nftnl_chain_set_str(c, NFTNL_CHAIN_TABLE, t->name);

	list_for_each_entry(cc, chains, head) {
		nftnl_chain_set_str(c, NFTNL_CHAIN_NAME, cc->name);
		rc = __fetch_chain_cache(h, t, c);
		if (rc)
			ret = rc;
	}

	nftnl_chain_free(c);
	return ret;
}

struct rule_list_cb_data {
	struct nftnl_chain *chain;
	int verbose;
};

static int nftnl_rule_list_cb(const struct nlmsghdr *nlh, void *data)
{
	struct rule_list_cb_data *rld = data;
	struct nftnl_chain *c = rld->chain;
	struct nftnl_rule *r;

	r = nftnl_rule_alloc();
	if (r == NULL)
		return MNL_CB_OK;

	if (nftnl_rule_nlmsg_parse(nlh, r) < 0) {
		nftnl_rule_free(r);
		return MNL_CB_OK;
	}

	if (rld->verbose > 1) {
		nftnl_rule_fprintf(stdout, r, 0, 0);
		fprintf(stdout, "\n");
	}
	nftnl_chain_rule_add_tail(r, c);
	return MNL_CB_OK;
}

static int nft_rule_list_update(struct nft_chain *nc, void *data)
{
	struct nftnl_chain *c = nc->nftnl;
	struct nft_handle *h = data;
	struct rule_list_cb_data rld = {
		.chain = c,
		.verbose = h->verbose,
	};
	char buf[16536];
	struct nlmsghdr *nlh;
	struct nftnl_rule *rule;
	int ret;

	if (nftnl_rule_lookup_byindex(c, 0))
		return 0;

	rule = nftnl_rule_alloc();
	if (!rule)
		return -1;

	nftnl_rule_set_str(rule, NFTNL_RULE_TABLE,
			   nftnl_chain_get_str(c, NFTNL_CHAIN_TABLE));
	nftnl_rule_set_str(rule, NFTNL_RULE_CHAIN,
			   nftnl_chain_get_str(c, NFTNL_CHAIN_NAME));

	nlh = nftnl_nlmsg_build_hdr(buf, NFT_MSG_GETRULE, h->family,
				    NLM_F_DUMP, h->seq);
	nftnl_rule_nlmsg_build_payload(nlh, rule);

	ret = mnl_talk(h, nlh, nftnl_rule_list_cb, &rld);
	if (ret < 0 && errno == EINTR)
		assert_nft_restart(h);

	nftnl_rule_free(rule);

	if (h->family == NFPROTO_BRIDGE)
		nft_bridge_chain_postprocess(h, c);

	return 0;
}

static int fetch_rule_cache(struct nft_handle *h,
			    const struct builtin_table *t)
{
	int i;

	if (t)
		return nft_chain_foreach(h, t->name, nft_rule_list_update, h);

	for (i = 0; i < NFT_TABLE_MAX; i++) {

		if (!h->tables[i].name)
			continue;

		if (nft_chain_foreach(h, h->tables[i].name,
				      nft_rule_list_update, h))
			return -1;
	}
	return 0;
}

static int flush_cache(struct nft_handle *h, struct nft_cache *c,
		       const char *tablename);

static void
__nft_build_cache(struct nft_handle *h)
{
	struct nft_cache_req *req = &h->cache_req;
	const struct builtin_table *t = NULL;
	struct list_head *chains = NULL;
	uint32_t genid_check;

	if (h->cache_init)
		return;

	if (req->table) {
		t = nft_table_builtin_find(h, req->table);
		if (!req->all_chains)
			chains = &req->chain_list;
	}

	h->cache_init = true;
retry:
	mnl_genid_get(h, &h->nft_genid);

	if (req->level >= NFT_CL_TABLES)
		fetch_table_cache(h);
	if (req->level == NFT_CL_FAKE)
		goto genid_check;
	if (req->level >= NFT_CL_CHAINS)
		fetch_chain_cache(h, t, chains);
	if (req->level >= NFT_CL_SETS)
		fetch_set_cache(h, t, NULL);
	if (req->level >= NFT_CL_RULES)
		fetch_rule_cache(h, t);
genid_check:
	mnl_genid_get(h, &genid_check);
	if (h->nft_genid != genid_check) {
		flush_cache(h, h->cache, NULL);
		goto retry;
	}
}

static void __nft_flush_cache(struct nft_handle *h)
{
	if (!h->cache_index) {
		h->cache_index++;
		h->cache = &h->__cache[h->cache_index];
	} else {
		flush_chain_cache(h, NULL);
	}
}

static int ____flush_rule_cache(struct nftnl_rule *r, void *data)
{
	nftnl_rule_list_del(r);
	nftnl_rule_free(r);

	return 0;
}

static int __flush_rule_cache(struct nft_chain *c, void *data)
{
	return nftnl_rule_foreach(c->nftnl, ____flush_rule_cache, NULL);
}

int flush_rule_cache(struct nft_handle *h, const char *table,
		     struct nft_chain *c)
{
	if (c)
		return __flush_rule_cache(c, NULL);

	nft_chain_foreach(h, table, __flush_rule_cache, NULL);
	return 0;
}

static int __flush_chain_cache(struct nft_chain *c, void *data)
{
	nft_chain_list_del(c);
	nft_chain_free(c);

	return 0;
}

static int __flush_set_cache(struct nftnl_set *s, void *data)
{
	nftnl_set_list_del(s);
	nftnl_set_free(s);

	return 0;
}

static void flush_base_chain_cache(struct nft_chain **base_chains)
{
	int i;

	for (i = 0; i < NF_INET_NUMHOOKS; i++) {
		if (!base_chains[i])
			continue;
		hlist_del(&base_chains[i]->hnode);
		nft_chain_free(base_chains[i]);
		base_chains[i] = NULL;
	}
}

static int flush_cache(struct nft_handle *h, struct nft_cache *c,
		       const char *tablename)
{
	const struct builtin_table *table;
	int i;

	if (tablename) {
		table = nft_table_builtin_find(h, tablename);
		if (!table)
			return 0;

		flush_base_chain_cache(c->table[table->type].base_chains);
		nft_chain_foreach(h, tablename, __flush_chain_cache, NULL);
		c->table[table->type].sorted = false;

		if (c->table[table->type].sets)
			nftnl_set_list_foreach(c->table[table->type].sets,
					       __flush_set_cache, NULL);
		return 0;
	}

	for (i = 0; i < NFT_TABLE_MAX; i++) {
		if (h->tables[i].name == NULL)
			continue;

		flush_base_chain_cache(c->table[i].base_chains);
		if (c->table[i].chains) {
			nft_chain_list_free(c->table[i].chains);
			c->table[i].chains = NULL;
			c->table[i].sorted = false;
		}

		if (c->table[i].sets) {
			nftnl_set_list_free(c->table[i].sets);
			c->table[i].sets = NULL;
		}

		c->table[i].exists = false;
	}

	return 1;
}

void flush_chain_cache(struct nft_handle *h, const char *tablename)
{
	if (!h->cache_init)
		return;

	if (flush_cache(h, h->cache, tablename))
		h->cache_init = false;
}

void nft_rebuild_cache(struct nft_handle *h)
{
	if (h->cache_init) {
		__nft_flush_cache(h);
		h->cache_init = false;
	}

	__nft_build_cache(h);
}

void nft_cache_build(struct nft_handle *h)
{
	struct nft_cache_req *req = &h->cache_req;
	const struct builtin_table *t = NULL;
	int i;

	if (req->table)
		t = nft_table_builtin_find(h, req->table);

	/* fetch builtin chains as well (if existing) so nft_xt_builtin_init()
	 * doesn't override policies by accident */
	if (t && !req->all_chains) {
		for (i = 0; i < NF_INET_NUMHOOKS; i++) {
			const char *cname = t->chains[i].name;

			if (!cname)
				break;
			cache_chain_list_insert(&req->chain_list, cname);
		}
	}

	__nft_build_cache(h);
}

void nft_release_cache(struct nft_handle *h)
{
	struct nft_cache_req *req = &h->cache_req;
	struct cache_chain *cc, *cc_tmp;

	while (h->cache_index)
		flush_cache(h, &h->__cache[h->cache_index--], NULL);
	flush_cache(h, &h->__cache[0], NULL);
	h->cache = &h->__cache[0];
	h->cache_init = false;

	if (req->level != NFT_CL_FAKE)
		req->level = NFT_CL_TABLES;
	if (req->table) {
		free(req->table);
		req->table = NULL;
	}
	req->all_chains = false;
	list_for_each_entry_safe(cc, cc_tmp, &req->chain_list, head) {
		list_del(&cc->head);
		free(cc->name);
		free(cc);
	}
}

struct nftnl_set_list *
nft_set_list_get(struct nft_handle *h, const char *table, const char *set)
{
	const struct builtin_table *t;

	t = nft_table_builtin_find(h, table);
	if (!t)
		return NULL;

	return h->cache->table[t->type].sets;
}
