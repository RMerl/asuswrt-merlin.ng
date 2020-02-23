/*************************************************************************
 *
 * ivi_rule.c :
 *
 * MAP-T/MAP-E 4to6 Prefix Mapping Kernel Module
 *
 * Copyright (C) 2013 CERNET Network Center
 * All rights reserved.
 * 
 * Design and coding: 
 *   Xing Li <xing@cernet.edu.cn> 
 *	 Congxiao Bao <congxiao@cernet.edu.cn>
 *   Wentao Shang <wentaoshang@gmail.com>
 * 	 Yuncheng Zhu <haoyu@cernet.edu.cn> 
 * 	 Guoliang Han <bupthgl@gmail.com>
 * 
 * Contributions:
 *
 * This file is part of MAP-T/MAP-E Kernel Module.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * You should have received a copy of the GNU General Public License 
 * along with MAP-T/MAP-E Kernel Module. If not, see 
 * <http://www.gnu.org/licenses/>.
 *
 * For more versions, please send an email to <bupthgl@gmail.com> to
 * obtain an password to access the svn server.
 *
 * LIC: GPLv2
 *
 ************************************************************************/

#include "ivi_rule.h"

#define KEYLENGTH 32

typedef u32 t_key;

#define T_TNODE 0
#define T_LEAF  1
#define NODE_TYPE_MASK  0x1UL
#define NODE_TYPE(node) ((node)->parent & NODE_TYPE_MASK)

#define IS_TNODE(n) (!(n->parent & T_LEAF))
#define IS_LEAF(n) (n->parent & T_LEAF)

static const int halve_threshold = 25;
static const int inflate_threshold = 50;
static const int halve_threshold_root = 15;
static const int inflate_threshold_root = 30;

struct tentry {
	unsigned long parent;
	t_key key;
};

struct tnode {
	unsigned long parent;
	t_key key;
	unsigned char pos;
	unsigned char bits;
	unsigned int full_children;
	unsigned int empty_children;
	struct tentry *child[0];
};

struct tleaf_info {
	struct hlist_node node;
	int plen;
	u32 mask_plen;
	struct in6_addr prefix6;
	int prefix6_len;
	u16 ratio;
	u16 adjacent;
	u8 format;
	u8 transport;
	u8 extension;
};

struct tleaf {
	unsigned long parent;
	t_key key;
	struct hlist_head head;
};

static struct tentry *trie = NULL;
static spinlock_t trie_lock;

#ifdef IVI_DEBUG
/* Memory counter */
static int balance = 0;
#endif

static inline struct tnode* node_parent(const struct tentry *node)
{
	return (struct tnode *)(node->parent & ~NODE_TYPE_MASK);
}

static inline void node_set_parent(struct tentry *node, const struct tnode *ptr)
{
	node->parent = (unsigned long)ptr | NODE_TYPE(node);
}

static inline struct tentry* tnode_get_child(const struct tnode *tn, unsigned int i)
{
	return (tn->child[i]);
}

static inline int tnode_child_length(const struct tnode *tn)
{
	return 1 << tn->bits;
}

static inline t_key mask_pfx(t_key k, unsigned int l)
{
	return (l == 0) ? 0 : k >> (KEYLENGTH-l) << (KEYLENGTH-l);
}

static inline t_key tkey_extract_bits(t_key a, unsigned int offset, unsigned int bits)
{
	if (offset < KEYLENGTH)
		return ((t_key)(a << offset)) >> (KEYLENGTH - bits);
	else
		return 0;
}

static inline int tkey_equals(t_key a, t_key b)
{
	return a == b;
}

static inline int tkey_sub_equals(t_key a, int offset, int bits, t_key b)
{
	if (bits == 0 || offset >= KEYLENGTH)
		return 1;
	bits = bits > KEYLENGTH ? KEYLENGTH : bits;
	return ((a ^ b) << offset) >> (KEYLENGTH - bits) == 0;
}

static inline int tkey_mismatch(t_key a, int offset, t_key b)
{
	t_key diff = a ^ b;
	int i = offset;

	if (!diff)
		return 0;
	while ((diff << i) >> (KEYLENGTH-1) == 0)
		i++;
	return i;
}

/* Caller must free or store all the children of tnode 'n'
 *   before calling this function to free it.
 */
static inline void tnode_free(struct tnode *n)
{
	if (!n)
		return;

	kfree(n);
#ifdef IVI_DEBUG
	balance--;
#endif
}

static inline void tleaf_info_free(struct tleaf_info *li)
{
	if (!li)
		return;

	kfree(li);
#ifdef IVI_DEBUG
	balance--;
#endif
}

static inline void tleaf_free(struct tleaf *l)
{
	if (!l)
		return;
	
	kfree(l);
#ifdef IVI_DEBUG
	balance--;
#endif
}

static void tentry_free(struct tentry *node)
{
	if (!node)
		return;

	if (IS_LEAF(node))
		tleaf_free((struct tleaf *)node);
	else
		tnode_free((struct tnode *)node);
}

static void tnode_clean_free(struct tnode *tn)
{
	int i;
	struct tentry *tofree;

	if (!tn)
		return;

	for (i = 0; i < tnode_child_length(tn); i++) {
		tofree = tn->child[i];
		if (tofree)
			tentry_free(tofree);
	}
	tnode_free(tn);
}

static struct tleaf *tleaf_new(void)
{
	struct tleaf *l = (struct tleaf *)kmalloc(sizeof(struct tleaf), GFP_ATOMIC);
#ifdef IVI_DEBUG
	balance++;
#endif
	if (l) {
		l->parent = T_LEAF;
		INIT_HLIST_HEAD(&l->head);
	}
	return l;
}

static struct tleaf_info *tleaf_info_new(int plen)
{
	struct tleaf_info *li = (struct tleaf_info *)kzalloc(sizeof(struct tleaf_info), GFP_ATOMIC);
#ifdef IVI_DEBUG
	balance++;
#endif
	if (li) {
		li->plen = plen;
		li->mask_plen = ntohl(inet_make_mask(plen));
		INIT_HLIST_NODE(&li->node);
	}
	return li;
}


static struct tnode *tnode_new(t_key key, int pos, int bits)
{
	size_t size = sizeof(struct tnode) + (sizeof(struct tentry *) << bits);
	struct tnode *tn = (struct tnode *)kzalloc(size, GFP_ATOMIC);
#ifdef IVI_DEBUG
	balance++;
#endif

	if (tn) {
		tn->parent = T_TNODE;
		tn->pos = pos;
		tn->bits = bits;
		tn->key = key;
		tn->full_children = 0;
		tn->empty_children = 1 << bits;
	}

	return tn;
}

/*
 * Check whether a tnode 'chi' is "full", i.e. it is an internal node
 * and no bits are skipped.
 */
static inline int tnode_full(const struct tnode *tn, const struct tentry *chi)
{
	struct tnode *n;

	if (chi == NULL || IS_LEAF(chi))
		return 0;
	
	n = (struct tnode *)chi;

	return n->pos == tn->pos + tn->bits;
}

/*
 * Add a child at position i overwriting the old value.
 * Caller must store the old pointer value if it is not NULL,
 *   otherwise the old memory will be lost.
 * Update the value of full_children and empty_children.
 */
static void tnode_put_child_reorg(struct tnode *tn, int i, struct tentry *n, int wasfull)
{
	struct tentry *chi = tn->child[i];
	int isfull;

	/* update emptyChildren */
	if (n == NULL && chi != NULL)
		tn->empty_children++;
	else if (n != NULL && chi == NULL)
		tn->empty_children--;

	/* update fullChildren */
	if (wasfull == -1)
		wasfull = tnode_full(tn, chi);

	isfull = tnode_full(tn, n);
	if (wasfull && !isfull)
		tn->full_children--;
	else if (!wasfull && isfull)
		tn->full_children++;

	if (n)
		node_set_parent(n, tn);

	tn->child[i] = n;
}

static inline void put_child(struct tnode *tn, int i, struct tentry *n)
{
	tnode_put_child_reorg(tn, i, n, -1);
}

static struct tentry *resize(struct tnode *tn);
static struct tnode *inflate(struct tnode *tn);
static struct tnode *halve(struct tnode *tn);

static struct tnode *inflate(struct tnode *tn)
{
	struct tnode *oldtnode = tn;
	int olen = tnode_child_length(tn);
	int i;

	tn = tnode_new(oldtnode->key, oldtnode->pos, oldtnode->bits + 1);

	if (!tn)
		return NULL;

	/* Create new internal tnode if necessary, with its children left empty */
	for (i = 0; i < olen; i++) {
		struct tnode *inode;
		inode = (struct tnode *)tnode_get_child(oldtnode, i);
		if (tnode_full(oldtnode, (struct tentry *)inode) && inode->bits > 1) {
			struct tnode *left, *right;
			t_key m;
			
			m = ~0U << (KEYLENGTH - 1) >> inode->pos;

			left = tnode_new(inode->key & (~m), inode->pos + 1, inode->bits - 1);
			if (!left) {
				tnode_free(tn);
				return NULL;
			}

			right = tnode_new(inode->key | m, inode->pos + 1, inode->bits - 1);
			if (!right) {
				tnode_free(left);
				tnode_free(tn);
				return NULL;
			}

			/* Insert the *doubled* internal tnodes */
			put_child(tn, 2*i, (struct tentry *)left);
			put_child(tn, 2*i+1, (struct tentry *)right);
		}
	}
	
	/* Fill in the children of the *doubled* internal tnodes */
	for (i = 0; i < olen; i++) {
		struct tnode *inode;
		struct tentry *node = tnode_get_child(oldtnode, i);
		struct tnode *left, *right;
		int size, j;

		/* An empty child */
		if (node == NULL)
			continue;

		/* A leaf or an internal node with skipped bits */
		if (IS_LEAF(node) || ((struct tnode *)node)->pos > tn->pos + tn->bits) {
				if (tkey_extract_bits(node->key, oldtnode->pos + oldtnode->bits, 1) == 0)
					put_child(tn, 2*i, node);
				else
					put_child(tn, 2*i+1, node);
				continue;
		}

		/* An internal (full) node with two children */
		inode = (struct tnode *) node;

		if (inode->bits == 1) {
			put_child(tn, 2*i, inode->child[0]);
			put_child(tn, 2*i+1, inode->child[1]);
			tnode_free(inode);
			continue;
		}

		/* An internal (full) node with more than two children */
		left = (struct tnode *)tnode_get_child(tn, 2*i);
		put_child(tn, 2*i, NULL); /* Temporarily remove */

		right = (struct tnode *)tnode_get_child(tn, 2*i+1);
		put_child(tn, 2*i+1, NULL); /* Temporarily remove */

		size = tnode_child_length(left);
		for (j = 0; j < size; j++) {
			put_child(left, j, inode->child[j]);
			put_child(right, j, inode->child[j + size]);
		}

		put_child(tn, 2*i, resize(left)); /* Restore */
		put_child(tn, 2*i+1, resize(right)); /* Restore */

		tnode_free(inode);
	}
	tnode_free(oldtnode);
	return tn;
}

static struct tnode *halve(struct tnode *tn)
{
	struct tnode *oldtnode = tn;
	struct tentry *left, *right;
	int i;
	int olen = tnode_child_length(tn);

	tn = tnode_new(oldtnode->key, oldtnode->pos, oldtnode->bits - 1);

	if (!tn)
		return NULL;

	/* Create new internal tnode if necessary, with its children left empty */
	for (i = 0; i < olen; i += 2) {
		left = tnode_get_child(oldtnode, i);
		right = tnode_get_child(oldtnode, i+1);

		/* Two nonempty children */
		if (left && right) {
			struct tnode *newn;
			/* Create a *full* tnode */
			newn = tnode_new(left->key, tn->pos + tn->bits, 1);
			if (!newn) {
				tnode_clean_free(tn);
				return NULL;
			}
			put_child(tn, i/2, (struct tentry *)newn);
		}
	}

	for (i = 0; i < olen; i += 2) {
		struct tnode *newBinNode;

		left = tnode_get_child(oldtnode, i);
		right = tnode_get_child(oldtnode, i+1);

		/* At least one of the children is empty */
		if (left == NULL) {
			if (right == NULL)    /* Both are empty */
				continue;
			put_child(tn, i/2, right);
			continue;
		}

		if (right == NULL) {
			put_child(tn, i/2, left);
			continue;
		}

		/* Two nonempty children */
		newBinNode = (struct tnode *)tnode_get_child(tn, i/2);
		put_child(tn, i/2, NULL); /* Temporarily remove */
		put_child(newBinNode, 0, left);
		put_child(newBinNode, 1, right);
		put_child(tn, i/2, resize(newBinNode)); /* Restore */
	}
	tnode_free(oldtnode);
	return tn;
}

#define MAX_WORK 10
static struct tentry *resize(struct tnode *tn)
{
	int i;
	struct tnode *old_tn;
	int inflate_threshold_use;
	int halve_threshold_use;
	int max_work;

	if (!tn)
		return NULL;

	/* No children */
	if (tn->empty_children == tnode_child_length(tn)) {
		tnode_free(tn);
		return NULL;
	}
	/* One child */
	if (tn->empty_children == tnode_child_length(tn) - 1)
		goto one_child;
	
	/*
	 * Double as long as the resulting node has a number of
	 *   nonempty nodes that are above the threshold.
	 */
	
	/* Keep root node larger  */
	if (!node_parent((struct tentry *)tn)) {
		inflate_threshold_use = inflate_threshold_root;
		halve_threshold_use = halve_threshold_root;
	} else {
		inflate_threshold_use = inflate_threshold;
		halve_threshold_use = halve_threshold;
	}

	max_work = MAX_WORK;
	while ((tn->full_children > 0 && max_work-- &&
		50 * (tn->full_children + tnode_child_length(tn) - tn->empty_children)
		>= inflate_threshold_use * tnode_child_length(tn))) {

			old_tn = tn;
			tn = inflate(tn);

			if (!tn) {
				tn = old_tn;
				break;
			}
	}

	/* Return if at least one inflate is run */
	if (max_work != MAX_WORK)
		return (struct tentry *)tn;
	
	/*
	 * Halve as long as the number of empty children in this
	 * node is above threshold.
	 */
	
	max_work = MAX_WORK;
	while (tn->bits > 1 && max_work-- &&
		   100 * (tnode_child_length(tn) - tn->empty_children) <
		   halve_threshold_use * tnode_child_length(tn)) {

		old_tn = tn;
		tn = halve(tn);

		if (!tn) {
			tn = old_tn;
			break;
		}
	}

	/* Only one child remains */
	if (tn->empty_children == tnode_child_length(tn) - 1) {
one_child:
		for (i = 0; i < tnode_child_length(tn); i++) {
			struct tentry *n;
			n = tn->child[i];
			if (!n)
				continue;

			/* compress one level */
			node_set_parent(n, NULL);
			tnode_free(tn);
			return n;
		}
	}
	return (struct tentry *)tn;
}

static void trie_rebalance(struct tnode *tn)
{
	int wasfull;
	t_key cindex, key;
	struct tnode *tp;

	key = tn->key;

	while (tn != NULL && (tp = node_parent((struct tentry *)tn)) != NULL) {
		cindex = tkey_extract_bits(key, tp->pos, tp->bits);
		wasfull = tnode_full(tp, tnode_get_child(tp, cindex));

		tn = (struct tnode *)resize((struct tnode *)tn);

		tnode_put_child_reorg((struct tnode *)tp, cindex, (struct tentry *)tn, wasfull);

		tp = node_parent((struct tentry *) tn);
		if (!tp)
			trie = (struct tentry *)tn;

		if (!tp)
			break;
		tn = tp;
	}

	/* Handle last (top) tnode */
	if (IS_TNODE(tn))
		tn = (struct tnode *)resize((struct tnode *)tn);

	trie = (struct tentry *)tn;
}

static struct tleaf_info *find_leaf_info(struct tleaf *l, int plen)
{
	struct tleaf_info *p;
	
	if (!l) 
		return NULL;
		
	hlist_for_each_entry(p, &l->head, node) {
		if (p->plen == plen)
			return p;
	}
	return NULL;
}

static void insert_leaf_info(struct tleaf *l, struct tleaf_info *li)
{
	struct tleaf_info *p = NULL;
	struct tleaf_info *last = NULL;
	struct hlist_head *head = &l->head;

	if (hlist_empty(head)) {
		hlist_add_head(&li->node, head);
	} else {
		hlist_for_each_entry(p, head, node) {
			if (li->plen > p->plen)
				break;

			last = p;
		}
		if (last)
			hlist_add_behind(&last->node, &li->node);
		else
			hlist_add_before(&li->node, &p->node);
	}
}

static struct tleaf *fib_find_node(unsigned int key)
{
	int pos;
	struct tnode *tn;
	struct tentry *n;

	pos = 0;
	n = trie;

	while (n != NULL &&  NODE_TYPE(n) == T_TNODE) {
		tn = (struct tnode *) n;
		if (tkey_sub_equals(tn->key, pos, tn->pos-pos, key)) {
			pos = tn->pos + tn->bits;
			n = tnode_get_child(tn, tkey_extract_bits(key, tn->pos, tn->bits));
		} else
			break;
	}

	/* Case we have found a leaf. Compare prefixes */
	if (n != NULL && IS_LEAF(n) && tkey_equals(key, n->key))
		return (struct tleaf *)n;

	return NULL;
}

static int check_leaf(struct tleaf *l, t_key key, struct in6_addr *prefix6, int *plen4, int *plen6, u16 *ratio, u16 *adjacent, u8 *fmt, u8 *transpt, u8 *ext6)
{
	struct tleaf_info *li;
	struct hlist_head *head = &l->head;
	hlist_for_each_entry(li, head, node) {
		if (l->key == (key & li->mask_plen)) {
			*prefix6 = li->prefix6;
			if (plen4)
				*plen4 = li->plen;
			if (plen6)
				*plen6 = li->prefix6_len;
			if (ratio)
				*ratio = li->ratio;
			if (adjacent)
				*adjacent = li->adjacent;
			if (fmt)
				*fmt = li->format;
			if (transpt)
				*transpt = li->transport;
			if (ext6)
				*ext6 = li->extension;
#ifdef IVI_DEBUG_RULE
			printk(KERN_DEBUG "ivi_rule_lookup: " NIP4_FMT "/%d -> " NIP6_FMT "/%d, ratio = %d, adjacent = %d, addr-format %d, transport %d, extension %d\n", 
				NIP4(key), li->plen, NIP6(li->prefix6), li->prefix6_len, li->ratio, li->adjacent, li->format, li->transport, li->extension);
#endif
			return 0;
		}
	}
	return 1;
}

int ivi_rule_lookup(u32 key, struct in6_addr *prefix6, int *plen4, int *plen6, u16 *ratio, u16 *adjacent, u8 *fmt, u8 *transpt, u8 *ext6)
{
	int ret;
	struct tentry *n;
	struct tnode *pn;
	unsigned int pos, bits;
	unsigned int chopped_off;
	t_key cindex = 0;
	unsigned int current_prefix_length = KEYLENGTH;
	struct tnode *cn;
	t_key pref_mismatch;

	spin_lock_bh(&trie_lock);
	
	n = trie;
	if (!n)
		goto failed;

	/* Just a leaf? */
	if (IS_LEAF(n)) {
		ret = check_leaf((struct tleaf *)n, key, prefix6, plen4, plen6, ratio, adjacent, fmt, transpt, ext6);
		goto found;
	}

	pn = (struct tnode *)n;
	chopped_off = 0;

	while (pn) {
		pos = pn->pos;
		bits = pn->bits;

		if (!chopped_off)
			cindex = tkey_extract_bits(mask_pfx(key, current_prefix_length), pos, bits);

		n = tnode_get_child(pn, cindex);

		if (n == NULL) {
			goto backtrace;
		}

		if (IS_LEAF(n)) {
			ret = check_leaf((struct tleaf *)n, key, prefix6, plen4, plen6, ratio, adjacent, fmt, transpt, ext6);
			if (ret > 0)
				goto backtrace;
			goto found;
		}

		cn = (struct tnode *)n;

		if (current_prefix_length < pos + bits) {
			if (tkey_extract_bits(cn->key, current_prefix_length,
				cn->pos - current_prefix_length)
				|| !(cn->child[0]))
				goto backtrace;
		}

		pref_mismatch = mask_pfx(cn->key ^ key, cn->pos);

		if (pref_mismatch) {
			int mp = KEYLENGTH - fls(pref_mismatch);

			if (tkey_extract_bits(cn->key, mp, cn->pos - mp) != 0)
				goto backtrace;

			if (current_prefix_length >= cn->pos)
				current_prefix_length = mp;
		}

		pn = (struct tnode *)n;  /* Descend */
		chopped_off = 0;
		continue;

backtrace:
		chopped_off++;

		/* As zero don't change the child key (cindex) */
		while ((chopped_off <= pn->bits)
			   && !(cindex & (1<<(chopped_off-1))))
			chopped_off++;

		/* Decrease current_... with bits chopped off */
		if (current_prefix_length > pn->pos + pn->bits - chopped_off)
			current_prefix_length = pn->pos + pn->bits - chopped_off;

		/*
		 * Either we do the actual chop off according or if we have
		 * chopped off all bits in this tnode walk up to our parent.
		 */
		if (chopped_off <= pn->bits) {
			cindex &= ~(1 << (chopped_off-1));
		} else {
			struct tnode *parent = node_parent((struct tentry *) pn);
			if (!parent)
				goto failed;

			/* Get Child's index */
			cindex = tkey_extract_bits(pn->key, parent->pos, parent->bits);
			pn = parent;
			chopped_off = 0;
			goto backtrace;
		}
	}
failed:
	ret = 1;
found:
	spin_unlock_bh(&trie_lock);
	return ret;
}

static struct tleaf_info* trie_insert_node(u32 key, u32 plen)
{
	int pos, newpos;
	int missbit;
	struct tleaf *l;
	struct tleaf_info *li = NULL;
	struct tentry *n;
	struct tnode *tp = NULL, *tn = NULL;
	t_key cindex;
	
	pos = 0;
	n = (struct tentry *)trie;

	while (n != NULL && NODE_TYPE(n) == T_TNODE) {
		tn = (struct tnode *)n;
		if (tkey_sub_equals(tn->key, pos, tn->pos-pos, key)) {
			tp = tn;
			pos = tn->pos + tn->bits;
			n = tnode_get_child(tn, tkey_extract_bits(key, tn->pos, tn->bits));
		} else
			break;
	}

	/*
	 * n ----> NULL, LEAF or TNODE
	 *
	 * tp is n's (parent) ----> NULL or TNODE
	 */

	/* Case 1: n is a leaf. Compare prefixes */
	if (n != NULL && IS_LEAF(n) && tkey_equals(key, n->key)) {
		l = (struct tleaf *)n;
		li = tleaf_info_new(plen);
		if (!li)
			return NULL;

		insert_leaf_info(l, li);
		return li;
	}
	l = tleaf_new();

	if (!l)
		return NULL;

	l->key = key;
	li = tleaf_info_new(plen);

	if (!li) {
		tleaf_free(l);
		return NULL;
	}

	insert_leaf_info(l, li);

	if (trie != NULL && n == NULL) {
		/* Case 2: n is NULL while we have root, just insert a new leaf */
		node_set_parent((struct tentry *)l, tp);
		cindex = tkey_extract_bits(key, tp->pos, tp->bits);
		put_child((struct tnode *)tp, cindex, (struct tentry *)l);
	} else {
		/* Case 3: n is a LEAF or a TNODE and the key doesn't match. */
		/*
		 * Add a new tnode here
		 * If root is NULL, first tnode need some special handling
		 */
		if (tp)
			pos = tp->pos + tp->bits;
		else
			pos = 0;

		if (n) {
			newpos = tkey_mismatch(key, pos, n->key);
			tn = tnode_new(n->key, newpos, 1);
		} else {
			newpos = 0;
			tn = tnode_new(key, newpos, 1); /* First tnode (root) */
		}

		if (tn == NULL)
			return NULL;

		node_set_parent((struct tentry *)tn, tp);

		missbit = tkey_extract_bits(key, newpos, 1);
		put_child(tn, missbit, (struct tentry *)l);
		put_child(tn, 1 - missbit, n);

		if (tp) {
			cindex = tkey_extract_bits(key, tp->pos, tp->bits);
			put_child((struct tnode *)tp, cindex, (struct tentry *)tn);
		} else {
			trie = (struct tentry *)tn;
			tp = tn;
		}
	}
	/* Re-balance the trie */
	trie_rebalance(tp);
	return li;
}

int ivi_rule_insert(struct rule_info *rule)
{
	u32 key, mask;
	int plen;
	struct tleaf *l;
	struct tleaf_info *li;

	if ((rule->plen4 > 32) || (rule->plen6 > 128))
		return -1;

	plen = rule->plen4;
	mask = ntohl(inet_make_mask(plen));
	key = rule->prefix4 & mask;

	spin_lock_bh(&trie_lock);
	l = fib_find_node(key);
	li = find_leaf_info(l, plen);
	if (li) {
		// Update satellite data.
		li->prefix6 = rule->prefix6;
		li->prefix6_len = rule->plen6;
		li->ratio = rule->ratio;
		li->adjacent = rule->adjacent;
		li->format = rule->format;
		li->transport = rule->transport;
		li->extension = rule->extension;
	} else {
		li = trie_insert_node(key, plen);
		// Insert satellite data.
		li->prefix6 = rule->prefix6;
		li->prefix6_len = rule->plen6;
		li->ratio = rule->ratio;
		li->adjacent = rule->adjacent;
		li->format = rule->format;
		li->transport = rule->transport;
		li->extension = rule->extension;
	}
	spin_unlock_bh(&trie_lock);
#ifdef IVI_DEBUG_RULE
	printk(KERN_DEBUG "ivi_rule_insert: " NIP4_FMT "/%d -> " NIP6_FMT "/%d, ratio %d, adjacent %d, addr-format %d, transport %d, extension %d\n", 
		NIP4(rule->prefix4), rule->plen4, NIP6(rule->prefix6), rule->plen6, rule->ratio, rule->adjacent, rule->format, rule->transport, rule->extension);
#endif
	return 0;
}

static void trie_leaf_remove(struct tleaf *l)
{
	struct tnode *tp = node_parent((struct tentry *)l);

	if (tp) {
		t_key cindex = tkey_extract_bits(l->key, tp->pos, tp->bits);
		put_child((struct tnode *)tp, cindex, NULL);
		trie_rebalance(tp);
	} else
		trie = NULL;

	tleaf_free(l);
}

int ivi_rule_delete(struct rule_info *rule)
{
	u32 key, mask;
	int plen, ret;
	struct tleaf *l;
	struct tleaf_info *li;

	ret = -1;

	key = rule->prefix4;
	plen = rule->plen4;

	if (plen > 32)
		goto out;

	mask = ntohl(inet_make_mask(plen));
	key = key & mask;

	spin_lock_bh(&trie_lock);
	l = fib_find_node(key);
	if (!l) {
		goto out_from_lock;
	}
	li = find_leaf_info(l, plen);
	if (!li)
		goto out_from_lock;

	/* Here we need to check whether 'li' matches the provided 'rule' 
	 *   since no check against *prefix6* is performed before.
	 */
	if (ipv6_addr_cmp(&li->prefix6, &rule->prefix6) || li->prefix6_len != rule->plen6 
		|| li->format != rule->format || li->ratio != rule->ratio || li->adjacent != rule->adjacent || li->transport != rule->transport)
		goto out_from_lock;
	
	hlist_del(&li->node);
	tleaf_info_free(li);
#ifdef IVI_DEBUG_RULE
	printk(KERN_DEBUG "ivi_rule_delete: " NIP4_FMT "/%d -> " NIP6_FMT "/%d, ratio = %d, adjacent = %d, addr-format %d, transport %d\n", 
		NIP4(rule->prefix4), rule->plen4, NIP6(rule->prefix6), rule->plen6, rule->ratio, rule->adjacent, rule->format, rule->transport);
#endif

	if (hlist_empty(&l->head))
		trie_leaf_remove(l);
	
	ret = 0;
out_from_lock:
	spin_unlock_bh(&trie_lock);
out:
	return ret;
}

/*
 * Scan for the next right_leaf starting at node c
 */
static struct tleaf *leaf_walk(struct tnode *p, struct tentry *c)
{
	do {
		t_key idx;

		if (c)
			idx = tkey_extract_bits(c->key, p->pos, p->bits) + 1;
		else
			idx = 0;

		while (idx < 1u << p->bits) {
			c = tnode_get_child(p, idx++);
			if (!c)
				continue;

			if (IS_LEAF(c)) {
				return (struct tleaf *)c;
			}

			/* Descend and start scanning in new node */
			p = (struct tnode *)c;
			idx = 0;
		}

		/* Node empty, walk back up to parent */
		c = (struct tentry *)p;
	} while ((p = node_parent(c)) != NULL);

	return NULL; /* Root of trie */
}

static struct tleaf *trie_first_leaf(struct tentry *t)
{

	struct tnode *n = (struct tnode *)t;

	if (!n)
		return NULL;

	if (IS_LEAF(n))          /* trie is just a leaf */
		return (struct tleaf *)n;

	return leaf_walk(n, NULL);
}

static struct tleaf *trie_next_leaf(struct tleaf *l)
{
	struct tentry *c = (struct tentry *)l;
	struct tnode *p = node_parent(c);

	if (!p)
		return NULL;	/* trie with just one leaf as its root */

	return leaf_walk(p, c);
}

static void trie_flush_leaf(struct tleaf *l)
{
	struct tleaf_info *li = NULL;
	struct hlist_node *loop;

	if (!l)
		return;

	hlist_for_each_entry_safe(li, loop, &l->head, node) {
		hlist_del(&li->node);
		tleaf_info_free(li);
	}
}

void ivi_rule_flush(void)
{
	struct tleaf *l, *ll = NULL;

	spin_lock_bh(&trie_lock);
	
	for (l = trie_first_leaf(trie); l; l = trie_next_leaf(l)) {
		trie_flush_leaf(l);

		if (ll && hlist_empty(&ll->head))
			trie_leaf_remove(ll);
		ll = l;
	}

	if (ll && hlist_empty(&ll->head))
		trie_leaf_remove(ll);

	spin_unlock_bh(&trie_lock);
}

int ivi_rule_init(void) {
	trie = NULL;
	spin_lock_init(&trie_lock);
#ifdef IVI_DEBUG
	balance = 0;
	printk(KERN_DEBUG "IVI: ivi_rule loaded.\n");
#endif
	return 0;
}

void ivi_rule_exit(void) {
	ivi_rule_flush();
#ifdef IVI_DEBUG
	printk(KERN_DEBUG "IVI: ivi_rule unloaded.\n");
	printk(KERN_DEBUG "IVI: ivi_rule memory balance = %d\n", balance);
#endif
}
