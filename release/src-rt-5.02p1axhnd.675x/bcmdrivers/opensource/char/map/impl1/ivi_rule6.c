/*************************************************************************
 *
 * ivi_rule_v6.c :
 *
 * MAP-T/MAP-E 6to4 Prefix Mapping Kernel Module
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

#include "ivi_rule6.h"

u8 u_byte = 1;  // 0: no u-byte; 1: set u-byte in v6 addr

struct rule6_node {
	struct rule6_node *parent;
	struct rule6_node *left;
	struct rule6_node *right;
	struct in6_addr key;
	u32 prefix4;
	int bit_pos;  // plen6 + plen4
	int plen6;  // actuall ipv6 prefix length
	int plen4;  // actuall ipv4 prefix length
	u16 ratio;
	u16 adjacent;
	u8 format;
	u8 flag;
};

#define RN_RINFO 0x0001

static struct rule6_node *radix = NULL;
static spinlock_t radix_lock;

#ifdef IVI_DEBUG
/* Memory counter */
static int balance = 0;
#endif


/*
 *	test bit
 */
#if defined(__LITTLE_ENDIAN)
# define BITOP_BE32_SWIZZLE	(0x1F & ~7)
#else
# define BITOP_BE32_SWIZZLE	0
#endif

static __inline__ __be32 addr_bit_set(const void *token, int pos)
{
	const __be32 *addr = token;
	/*
	 * Here,
	 * 	1 << ((~pos ^ BITOP_BE32_SWIZZLE) & 0x1f)
	 * is optimized version of
	 *	htonl(1 << ((~pos)&0x1F))
	 * See include/asm-generic/bitops/le.h.
	 */
	return (__force __be32)(1 << ((~pos ^ BITOP_BE32_SWIZZLE) & 0x1f)) & addr[pos >> 5];
}

static __inline__ struct rule6_node * node_alloc(void)
{
	struct rule6_node *n;
	n = kzalloc(sizeof(struct rule6_node), GFP_ATOMIC);
#ifdef IVI_DEBUG
	balance++;
#endif
	return n;
}

static __inline__ void node_free(struct rule6_node *n)
{
	kfree(n);
#ifdef IVI_DEBUG
	balance--;
#endif
}

/*
 * Rule insertion
 */

static struct rule6_node* radix_insert_node(const struct in6_addr *addr, struct rule_info *rule)
{
	struct rule6_node *fn, *in, *ln, *pn;
	int plen, bit;
	u32 dir;

	pn = NULL;
	dir = 0;
	if (rule->format == ADDR_FMT_MAPT)
		plen = ubyte_adjust_bit(rule->plen6);
	else
		plen = rule->plen6 + rule->plen4;

	if (!radix)
		goto root_empty;

	fn = radix;

	do {
		/* Prefix match */
		if (plen < fn->bit_pos || !ipv6_prefix_equal(&fn->key, addr, fn->bit_pos))
			goto insert_above;

		/* Exact match ? */
		if (plen == fn->bit_pos) {
			fn->flag |= RN_RINFO;  /* fn contains rule info now */
			return fn;
		}

		/*
		 * We have more bits to go
		 */

		/* Try to walk down on tree. */
		dir = addr_bit_set(addr, fn->bit_pos);
		pn = fn;
		fn = dir ? fn->right: fn->left;
	} while (fn);

root_empty:
	/*
	 * We have walked to the bottom of tree.
	 * Create new leaf node without children.
	 */
	ln = node_alloc();
	if (ln == NULL)
		return NULL;

	/* set new leaf's key */
	ln->key = *addr;
	ln->prefix4 = rule->prefix4;
	ln->bit_pos = plen;
	ln->plen6 = rule->plen6;
	ln->plen4 = rule->plen4;
	ln->ratio = rule->ratio;
	ln->adjacent = rule->adjacent;
	ln->format = rule->format;
	ln->parent = pn;
	ln->flag |= RN_RINFO;

	if (!pn) {
		/* we have empty root */
		radix = ln;
	} else {
		if (dir)
			pn->right = ln;
		else
			pn->left  = ln;
	}

	return ln;


insert_above:
	/*
	 * split since we don't have a common prefix anymore or
	 * we have a less significant route.
	 * we've to insert an intermediate node on the list
	 * this new node will point to the one we need to create
	 * and the current
	 */

	pn = fn->parent;

	/* find 1st bit in difference between the 2 addrs.

	   See comment in __ipv6_addr_diff: bit may be an invalid value,
	   but if it is >= plen, the value is ignored in any case.
	 */
	bit = ipv6_addr_diff(addr, &fn->key);

	/*
	 *		(intermediate)[in]
	 *	          /	   \
	 *	(new leaf node)[ln] (old node)[fn]
	 */
	if (plen > bit) {
		in = node_alloc();
		ln = node_alloc();

		if (in == NULL || ln == NULL) {
			if (in)
				node_free(in);
			if (ln)
				node_free(ln);
			return NULL;
		}

		/* use fn's key as in's key */
		in->key = fn->key;
		in->bit_pos = bit;
		in->parent = pn;
		in->flag = 0; /* in's flag is cleared */

		if (!pn) {
			/* in is root now */
			radix = in;
		} else {
			/* update parent pointer */
			if (dir)
				pn->right = in;
			else
				pn->left  = in;
		}

		/* set new leaf's key */
		ln->key = *addr;
		ln->prefix4 = rule->prefix4;
		ln->bit_pos = plen;
		ln->plen6 = rule->plen6;
		ln->plen4 = rule->plen4;
		ln->ratio = rule->ratio;
		ln->adjacent = rule->adjacent;
		ln->format = rule->format;
		ln->parent = in;
		ln->flag |= RN_RINFO;

		fn->parent = in;

		if (addr_bit_set(addr, bit)) {
			in->right = ln;
			in->left  = fn;
		} else {
			in->left  = ln;
			in->right = fn;
		}
	} else { /* plen <= bit */

		/*
		 *		(new leaf node)[ln]
		 *	          /	   \
		 *	     (old node)[fn] NULL
		 */
		ln = node_alloc();

		if (ln == NULL)
			return NULL;

		/* set new leaf's key */
		ln->key = *addr;
		ln->prefix4 = rule->prefix4;
		ln->bit_pos = plen;
		ln->plen6 = rule->plen6;
		ln->plen4 = rule->plen4;
		ln->ratio = rule->ratio;
		ln->adjacent = rule->adjacent;
		ln->format = rule->format;
		ln->parent = pn;
		ln->flag |= RN_RINFO;

		if (!pn) {
			 /* ln is root now */
			radix = ln;
		} else {
			if (dir)
				pn->right = ln;
			else
				pn->left  = ln;
		}

		if (addr_bit_set(&fn->key, plen))
			ln->right = fn;
		else
			ln->left  = fn;

		fn->parent = ln;
	}
	return ln;
}

int ivi_rule6_insert(struct rule_info *rule)
{
	int ret, plen6;

	if (rule->plen4 > 0) {
		/* concatenate ipv6 prefix and ipv4 prefix */
		/* overwrite on 'rule' memory */
		plen6 = rule->plen6 >> 3;
		if (rule->format == ADDR_FMT_MAPT) {
			// Special care for MAP-T format
			// Currently no IPv4 bits is copied, LPM is totally done on Rule IPv6 Prefix
		} else {
			rule->prefix6.s6_addr[plen6] = (unsigned char)(rule->prefix4 >> 24);
			rule->prefix6.s6_addr[plen6 + 1] = (unsigned char)((rule->prefix4 >> 16) & 0xff);
			rule->prefix6.s6_addr[plen6 + 2] = (unsigned char)((rule->prefix4 >> 8) & 0xff);
			rule->prefix6.s6_addr[plen6 + 3] = (unsigned char)(rule->prefix4 & 0xff);
		}
	}

	spin_lock_bh(&radix_lock);
	if (radix_insert_node(&rule->prefix6, rule) == NULL) {
		ret = -1;
#ifdef IVI_DEBUG_RULE
		printk(KERN_DEBUG "ivi_rule6_insert: failed to insert entry " NIP6_FMT " plen6 = %d, plen4 = %d, ratio = %d, adjacent = %d, addr-format %d\n", 
			NIP6(rule->prefix6), rule->plen6, rule->plen4, rule->ratio, rule->adjacent, rule->format);
#endif
	} else {
		ret = 0;
#ifdef IVI_DEBUG_RULE
		printk(KERN_DEBUG "ivi_rule6_insert: " NIP6_FMT " plen6 = %d, prefix4 = " NIP4_FMT ", plen4 = %d, ratio = %d, adjacent = %d, addr-format %d\n", 
			NIP6(rule->prefix6), rule->plen6, NIP4(rule->prefix4), rule->plen4, rule->ratio, rule->adjacent, rule->format);
#endif
	}
	spin_unlock_bh(&radix_lock);
	return ret;
}


/*
 * Rule lookup
 */

static struct rule6_node* radix_lookup(const struct in6_addr *addr)
{
	struct rule6_node *fn, *next;
	u32 dir;

	if (unlikely(!radix))  /* empty radix tree */
		return NULL;

	/*
	 * Descend on a tree
	 */
	fn = radix;

	for (;;) {
		dir = addr_bit_set(addr, fn->bit_pos);

		next = dir ? fn->right : fn->left;

		if (!next)
			break;

		fn = next;
	}

	while (fn) {
		if ((fn->flag & RN_RINFO) && 
		    ipv6_prefix_equal(&fn->key, addr, fn->bit_pos)) {
			return fn;
		}

		/* backtrace */
		fn = fn->parent;
	}

	return NULL;
}

int ivi_rule6_lookup(struct in6_addr *addr, int *plen, u32 *prefix4, int *plen4, u16 *ratio, u16 *adjacent, u8 *fmt)
{
	struct rule6_node* n;
	int ret;

	if (!plen)
		return -1;

	ret = -1;
	*plen = 0;

	spin_lock_bh(&radix_lock);
	
	n = radix_lookup(addr);

	if (n) {
#ifdef IVI_DEBUG_RULE
		printk(KERN_DEBUG "ivi_rule6_lookup: " NIP6_FMT " -> %d\n", NIP6(n->key), n->bit_pos);
#endif
		if (plen)
			*plen = n->plen6;
		if (prefix4)
			*prefix4 = n->prefix4;
		if (plen4)
			*plen4 = n->plen4;
		if (ratio)
			*ratio = n->ratio;
		if (adjacent)
			*adjacent = n->adjacent;
		if (fmt)
			*fmt = n->format;
		ret = 0;
	}
	
	spin_unlock_bh(&radix_lock);

	return ret;
	
}


/*
 * Rule deletion
 */

static struct rule6_node* radix_delete_trim(struct rule6_node* fn)
{
	u32 children;
	struct rule6_node *pn, *child;

	if (unlikely(!fn)) /* nothing to be done */
		return NULL;

	/*
	 * Delete
	 */

	pn = fn->parent;

	children = 0;
	child = NULL;
	if (fn->left) {
		child = fn->left;
		children++;
	}

	if (fn->right) {
		child = fn->right;
		children++;
	}

	if (children == 2) {
		/* clear flag and set 'fn' to be an intermediate node without rule info */
		fn->flag &= ~RN_RINFO;
		fn = pn;  /* backtrace */
	} else if (children == 1) {
		/* move the single child up */
		child->parent = pn;

		if (!pn) {
			/* child is root now */
			radix = child;
		} else {
			/* update parent pointers */
			if (pn->left == fn)
				pn->left = child;
			else
				pn->right = child;
		}
	
		node_free(fn);
		fn = pn;  /* backtrace */
	} else {
		/* 'fn' is leaf, simply free it */
		if (!pn) {
			/* radix tree is empty now */
			radix = NULL;
		} else {
			/* update parent pointers */
			if (pn->left == fn)
				pn->left = NULL;
			else
				pn->right = NULL;
		}
		node_free(fn);
		fn = pn;  /* backtrace */
	}
	
	/*
	 * Trim
	 */
	 
	while(fn) {
		if (fn->flag & RN_RINFO) /* never trim a rule info node */
			break;

		pn = fn->parent;
	
		children = 0;
		child = NULL;
		if (fn->left) {
			child = fn->left;
			children++;
		}
	
		if (fn->right) {
			child = fn->right;
			children++;
		}

		if (children == 2) {
			/* never trim a node with two children */
			break;
		} else if (children == 1) {
			/* move the single child up */
			child->parent = pn;

			if (!pn) {
				/* child is root now */
				radix = child;
			} else {
				/* update parent pointers */
				if (pn->left == fn)
					pn->left = child;
				else
					pn->right = child;
			}
		
			node_free(fn);
			fn = pn;  /* backtrace */
		} else {
			/* 'fn' is leaf, simply free it */
			if (!pn) {
				/* radix tree is empty now */
				radix = NULL;
			} else {
				/* update parent pointers */
				if (pn->left == fn)
					pn->left = NULL;
				else
					pn->right = NULL;
			}
			node_free(fn);
			fn = pn;  /* backtrace */
		}
	}

	return fn;
}

int ivi_rule6_delete(struct rule_info *rule)
{
	struct rule6_node *fn, *next;
	u32 dir;
	int ret, plen6;

	ret = -1;

	if (rule->plen4 > 0) {
		/* concatenate ipv6 prefix and ipv4 prefix */
		/* overwrite on 'rule' memory */
		plen6 = rule->plen6 >> 3;
		rule->prefix6.s6_addr[ubyte_adjust(plen6)] = (unsigned char)(rule->prefix4 >> 24);
		rule->prefix6.s6_addr[ubyte_adjust(plen6 + 1)] = (unsigned char)((rule->prefix4 >> 16) & 0xff);
		rule->prefix6.s6_addr[ubyte_adjust(plen6 + 2)] = (unsigned char)((rule->prefix4 >> 8) & 0xff);
		rule->prefix6.s6_addr[ubyte_adjust(plen6 + 3)] = (unsigned char)(rule->prefix4 & 0xff);
	}
	
	spin_lock_bh(&radix_lock);

	if (unlikely(!radix)) {
		/* empty radix tree */
		spin_unlock_bh(&radix_lock);
		return -1;
	}

	/*
	 * Descend on a tree
	 */
	fn = radix;

	for (;;) {
		dir = addr_bit_set(&rule->prefix6, fn->bit_pos);

		next = dir ? fn->right : fn->left;

		if (!next)
			break;

		fn = next;
	}
	
	/* Exact match? */
	if ((fn->flag & RN_RINFO) 
	    && (fn->bit_pos == ubyte_adjust_bit(rule->plen6 + rule->plen4))
	    && (fn->plen6 == rule->plen6)
	    && (fn->ratio == rule->ratio)
	    && (fn->adjacent == rule->adjacent)
	    && (fn->format == rule->format)
	    && ipv6_prefix_equal(&fn->key, &rule->prefix6, fn->bit_pos)) {
		if (radix_delete_trim(fn) != NULL) {
			ret = 0;
#ifdef IVI_DEBUG_RULE
			printk(KERN_DEBUG "ivi_rule6_delete: " NIP6_FMT "/%d\n", NIP6(rule->prefix6), rule->plen6);
#endif
		}
	}

	spin_unlock_bh(&radix_lock);

	return ret;
}


/*
 * Traversal (root first)
 */

static __inline int child_dir(struct rule6_node *p, struct rule6_node *c)
{
	return (c == p->left) ? 0 : 1;
}

static struct rule6_node* next_rule6_info(struct rule6_node *p)
{
	struct rule6_node *c;
	int dir;

	c = NULL;
	
	do {
		if (c)
			dir = child_dir(p, c) + 1;
		else
			dir = 0;

		while (dir < 2) {
			c = (dir++ == 0) ? p->left : p->right;
			if (!c)
				continue;

			if (c->flag & RN_RINFO) {
				return c;
			}

			/* Descend and start scanning in new node */
			p = c;
			dir = 0;
		}

		/* Node empty, walk back up to parent */
		c = p;
	} while ((p = c->parent) != NULL);

	return NULL; /* Root of trie */
}

static struct rule6_node* first_rule6_info(void)
{
	if (unlikely(!radix)) /* empty radix tree */
		return NULL;

	if (radix->flag & RN_RINFO) /* root has rule info */
		return radix;

	return next_rule6_info(radix);
}

void ivi_rule6_flush(void)
{
	struct rule6_node *r, *rr = NULL;

	spin_lock_bh(&radix_lock);
	
	for (r = first_rule6_info(); r; r = next_rule6_info(r)) {
#ifdef IVI_DEBUG_RULE
		printk(KERN_DEBUG "ivi_rule6_flush: " NIP6_FMT "/%d\n", NIP6(r->key), r->bit_pos);
#endif
		if (rr)
			radix_delete_trim(rr);
		rr = r;
	}

	if (rr)
		radix_delete_trim(rr);

	spin_unlock_bh(&radix_lock);
}


int ivi_rule6_init(void) {
	radix = NULL;
	spin_lock_init(&radix_lock);
#ifdef IVI_DEBUG
	balance = 0;
	printk(KERN_DEBUG "IVI: ivi_rule6 loaded.\n");
#endif
	return 0;
}

void ivi_rule6_exit(void) {
	ivi_rule6_flush();
#ifdef IVI_DEBUG
	printk(KERN_DEBUG "IVI: ivi_rule6 unloaded.\n");
	printk(KERN_DEBUG "IVI: ivi_rule6 memory balance = %d\n", balance);
#endif
}
