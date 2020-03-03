/*
 * Copyright (c) 2008-2009 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Development of this code funded by Astaro AG (http://www.astaro.com/)
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/rbtree.h>
#include <linux/netlink.h>
#include <linux/netfilter.h>
#include <linux/netfilter/nf_tables.h>
#include <net/netfilter/nf_tables.h>

static DEFINE_SPINLOCK(nft_rbtree_lock);

struct nft_rbtree {
	struct rb_root		root;
};

struct nft_rbtree_elem {
	struct rb_node		node;
	struct nft_set_ext	ext;
};


static bool nft_rbtree_lookup(const struct nft_set *set, const u32 *key,
			      const struct nft_set_ext **ext)
{
	const struct nft_rbtree *priv = nft_set_priv(set);
	const struct nft_rbtree_elem *rbe, *interval = NULL;
	const struct rb_node *parent;
	u8 genmask = nft_genmask_cur(read_pnet(&set->pnet));
	int d;

	spin_lock_bh(&nft_rbtree_lock);
	parent = priv->root.rb_node;
	while (parent != NULL) {
		rbe = rb_entry(parent, struct nft_rbtree_elem, node);

		d = memcmp(nft_set_ext_key(&rbe->ext), key, set->klen);
		if (d < 0) {
			parent = parent->rb_left;
			interval = rbe;
		} else if (d > 0)
			parent = parent->rb_right;
		else {
found:
			if (!nft_set_elem_active(&rbe->ext, genmask)) {
				parent = parent->rb_left;
				continue;
			}
			if (nft_set_ext_exists(&rbe->ext, NFT_SET_EXT_FLAGS) &&
			    *nft_set_ext_flags(&rbe->ext) &
			    NFT_SET_ELEM_INTERVAL_END)
				goto out;
			spin_unlock_bh(&nft_rbtree_lock);

			*ext = &rbe->ext;
			return true;
		}
	}

	if (set->flags & NFT_SET_INTERVAL && interval != NULL) {
		rbe = interval;
		goto found;
	}
out:
	spin_unlock_bh(&nft_rbtree_lock);
	return false;
}

static int __nft_rbtree_insert(const struct nft_set *set,
			       struct nft_rbtree_elem *new)
{
	struct nft_rbtree *priv = nft_set_priv(set);
	struct nft_rbtree_elem *rbe;
	struct rb_node *parent, **p;
	u8 genmask = nft_genmask_next(read_pnet(&set->pnet));
	int d;

	parent = NULL;
	p = &priv->root.rb_node;
	while (*p != NULL) {
		parent = *p;
		rbe = rb_entry(parent, struct nft_rbtree_elem, node);
		d = memcmp(nft_set_ext_key(&rbe->ext),
			   nft_set_ext_key(&new->ext),
			   set->klen);
		if (d < 0)
			p = &parent->rb_left;
		else if (d > 0)
			p = &parent->rb_right;
		else {
			if (nft_set_elem_active(&rbe->ext, genmask))
				return -EEXIST;
			p = &parent->rb_left;
		}
	}
	rb_link_node(&new->node, parent, p);
	rb_insert_color(&new->node, &priv->root);
	return 0;
}

static int nft_rbtree_insert(const struct nft_set *set,
			     const struct nft_set_elem *elem)
{
	struct nft_rbtree_elem *rbe = elem->priv;
	int err;

	spin_lock_bh(&nft_rbtree_lock);
	err = __nft_rbtree_insert(set, rbe);
	spin_unlock_bh(&nft_rbtree_lock);

	return err;
}

static void nft_rbtree_remove(const struct nft_set *set,
			      const struct nft_set_elem *elem)
{
	struct nft_rbtree *priv = nft_set_priv(set);
	struct nft_rbtree_elem *rbe = elem->priv;

	spin_lock_bh(&nft_rbtree_lock);
	rb_erase(&rbe->node, &priv->root);
	spin_unlock_bh(&nft_rbtree_lock);
}

static void nft_rbtree_activate(const struct nft_set *set,
				const struct nft_set_elem *elem)
{
	struct nft_rbtree_elem *rbe = elem->priv;

	nft_set_elem_change_active(set, &rbe->ext);
}

static void *nft_rbtree_deactivate(const struct nft_set *set,
				   const struct nft_set_elem *elem)
{
	const struct nft_rbtree *priv = nft_set_priv(set);
	const struct rb_node *parent = priv->root.rb_node;
	struct nft_rbtree_elem *rbe;
	u8 genmask = nft_genmask_cur(read_pnet(&set->pnet));
	int d;

	while (parent != NULL) {
		rbe = rb_entry(parent, struct nft_rbtree_elem, node);

		d = memcmp(nft_set_ext_key(&rbe->ext), &elem->key.val,
					   set->klen);
		if (d < 0)
			parent = parent->rb_left;
		else if (d > 0)
			parent = parent->rb_right;
		else {
			if (!nft_set_elem_active(&rbe->ext, genmask)) {
				parent = parent->rb_left;
				continue;
			}
			nft_set_elem_change_active(set, &rbe->ext);
			return rbe;
		}
	}
	return NULL;
}

static void nft_rbtree_walk(const struct nft_ctx *ctx,
			    const struct nft_set *set,
			    struct nft_set_iter *iter)
{
	const struct nft_rbtree *priv = nft_set_priv(set);
	struct nft_rbtree_elem *rbe;
	struct nft_set_elem elem;
	struct rb_node *node;
	u8 genmask = nft_genmask_cur(read_pnet(&set->pnet));

	spin_lock_bh(&nft_rbtree_lock);
	for (node = rb_first(&priv->root); node != NULL; node = rb_next(node)) {
		rbe = rb_entry(node, struct nft_rbtree_elem, node);

		if (iter->count < iter->skip)
			goto cont;
		if (!nft_set_elem_active(&rbe->ext, genmask))
			goto cont;

		elem.priv = rbe;

		iter->err = iter->fn(ctx, set, iter, &elem);
		if (iter->err < 0) {
			spin_unlock_bh(&nft_rbtree_lock);
			return;
		}
cont:
		iter->count++;
	}
	spin_unlock_bh(&nft_rbtree_lock);
}

static unsigned int nft_rbtree_privsize(const struct nlattr * const nla[])
{
	return sizeof(struct nft_rbtree);
}

static int nft_rbtree_init(const struct nft_set *set,
			   const struct nft_set_desc *desc,
			   const struct nlattr * const nla[])
{
	struct nft_rbtree *priv = nft_set_priv(set);

	priv->root = RB_ROOT;
	return 0;
}

static void nft_rbtree_destroy(const struct nft_set *set)
{
	struct nft_rbtree *priv = nft_set_priv(set);
	struct nft_rbtree_elem *rbe;
	struct rb_node *node;

	while ((node = priv->root.rb_node) != NULL) {
		rb_erase(node, &priv->root);
		rbe = rb_entry(node, struct nft_rbtree_elem, node);
		nft_set_elem_destroy(set, rbe);
	}
}

static bool nft_rbtree_estimate(const struct nft_set_desc *desc, u32 features,
				struct nft_set_estimate *est)
{
	unsigned int nsize;

	nsize = sizeof(struct nft_rbtree_elem);
	if (desc->size)
		est->size = sizeof(struct nft_rbtree) + desc->size * nsize;
	else
		est->size = nsize;

	est->class = NFT_SET_CLASS_O_LOG_N;

	return true;
}

static struct nft_set_ops nft_rbtree_ops __read_mostly = {
	.privsize	= nft_rbtree_privsize,
	.elemsize	= offsetof(struct nft_rbtree_elem, ext),
	.estimate	= nft_rbtree_estimate,
	.init		= nft_rbtree_init,
	.destroy	= nft_rbtree_destroy,
	.insert		= nft_rbtree_insert,
	.remove		= nft_rbtree_remove,
	.deactivate	= nft_rbtree_deactivate,
	.activate	= nft_rbtree_activate,
	.lookup		= nft_rbtree_lookup,
	.walk		= nft_rbtree_walk,
	.features	= NFT_SET_INTERVAL | NFT_SET_MAP,
	.owner		= THIS_MODULE,
};

static int __init nft_rbtree_module_init(void)
{
	return nft_register_set(&nft_rbtree_ops);
}

static void __exit nft_rbtree_module_exit(void)
{
	nft_unregister_set(&nft_rbtree_ops);
}

module_init(nft_rbtree_module_init);
module_exit(nft_rbtree_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Patrick McHardy <kaber@trash.net>");
MODULE_ALIAS_NFT_SET();
