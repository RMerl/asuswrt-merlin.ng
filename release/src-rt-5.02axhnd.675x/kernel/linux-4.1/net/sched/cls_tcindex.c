/*
 * net/sched/cls_tcindex.c	Packet classifier for skb->tc_index
 *
 * Written 1998,1999 by Werner Almesberger, EPFL ICA
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <net/act_api.h>
#include <net/netlink.h>
#include <net/pkt_cls.h>

/*
 * Passing parameters to the root seems to be done more awkwardly than really
 * necessary. At least, u32 doesn't seem to use such dirty hacks. To be
 * verified. FIXME.
 */

#define PERFECT_HASH_THRESHOLD	64	/* use perfect hash if not bigger */
#define DEFAULT_HASH_SIZE	64	/* optimized for diffserv */


struct tcindex_filter_result {
	struct tcf_exts		exts;
	struct tcf_result	res;
};

struct tcindex_filter {
	u16 key;
	struct tcindex_filter_result result;
	struct tcindex_filter __rcu *next;
	struct rcu_head rcu;
};


struct tcindex_data {
	struct tcindex_filter_result *perfect; /* perfect hash; NULL if none */
	struct tcindex_filter __rcu **h; /* imperfect hash; */
	struct tcf_proto *tp;
	u16 mask;		/* AND key with mask */
	u32 shift;		/* shift ANDed key to the right */
	u32 hash;		/* hash table size; 0 if undefined */
	u32 alloc_hash;		/* allocated size */
	u32 fall_through;	/* 0: only classify if explicit match */
	struct rcu_head rcu;
};

static inline int
tcindex_filter_is_set(struct tcindex_filter_result *r)
{
	return tcf_exts_is_predicative(&r->exts) || r->res.classid;
}

static struct tcindex_filter_result *
tcindex_lookup(struct tcindex_data *p, u16 key)
{
	if (p->perfect) {
		struct tcindex_filter_result *f = p->perfect + key;

		return tcindex_filter_is_set(f) ? f : NULL;
	} else if (p->h) {
		struct tcindex_filter __rcu **fp;
		struct tcindex_filter *f;

		fp = &p->h[key % p->hash];
		for (f = rcu_dereference_bh_rtnl(*fp);
		     f;
		     fp = &f->next, f = rcu_dereference_bh_rtnl(*fp))
			if (f->key == key)
				return &f->result;
	}

	return NULL;
}


static int tcindex_classify(struct sk_buff *skb, const struct tcf_proto *tp,
			    struct tcf_result *res)
{
	struct tcindex_data *p = rcu_dereference_bh(tp->root);
	struct tcindex_filter_result *f;
	int key = (skb->tc_index & p->mask) >> p->shift;

	pr_debug("tcindex_classify(skb %p,tp %p,res %p),p %p\n",
		 skb, tp, res, p);

	f = tcindex_lookup(p, key);
	if (!f) {
		if (!p->fall_through)
			return -1;
		res->classid = TC_H_MAKE(TC_H_MAJ(tp->q->handle), key);
		res->class = 0;
		pr_debug("alg 0x%x\n", res->classid);
		return 0;
	}
	*res = f->res;
	pr_debug("map 0x%x\n", res->classid);

	return tcf_exts_exec(skb, &f->exts, res);
}


static unsigned long tcindex_get(struct tcf_proto *tp, u32 handle)
{
	struct tcindex_data *p = rtnl_dereference(tp->root);
	struct tcindex_filter_result *r;

	pr_debug("tcindex_get(tp %p,handle 0x%08x)\n", tp, handle);
	if (p->perfect && handle >= p->alloc_hash)
		return 0;
	r = tcindex_lookup(p, handle);
	return r && tcindex_filter_is_set(r) ? (unsigned long) r : 0UL;
}

static int tcindex_init(struct tcf_proto *tp)
{
	struct tcindex_data *p;

	pr_debug("tcindex_init(tp %p)\n", tp);
	p = kzalloc(sizeof(struct tcindex_data), GFP_KERNEL);
	if (!p)
		return -ENOMEM;

	p->mask = 0xffff;
	p->hash = DEFAULT_HASH_SIZE;
	p->fall_through = 1;

	rcu_assign_pointer(tp->root, p);
	return 0;
}

static int
tcindex_delete(struct tcf_proto *tp, unsigned long arg)
{
	struct tcindex_data *p = rtnl_dereference(tp->root);
	struct tcindex_filter_result *r = (struct tcindex_filter_result *) arg;
	struct tcindex_filter __rcu **walk;
	struct tcindex_filter *f = NULL;

	pr_debug("tcindex_delete(tp %p,arg 0x%lx),p %p\n", tp, arg, p);
	if (p->perfect) {
		if (!r->res.class)
			return -ENOENT;
	} else {
		int i;

		for (i = 0; i < p->hash; i++) {
			walk = p->h + i;
			for (f = rtnl_dereference(*walk); f;
			     walk = &f->next, f = rtnl_dereference(*walk)) {
				if (&f->result == r)
					goto found;
			}
		}
		return -ENOENT;

found:
		rcu_assign_pointer(*walk, rtnl_dereference(f->next));
	}
	tcf_unbind_filter(tp, &r->res);
	tcf_exts_destroy(&r->exts);
	if (f)
		kfree_rcu(f, rcu);
	return 0;
}

static int tcindex_destroy_element(struct tcf_proto *tp,
				   unsigned long arg,
				   struct tcf_walker *walker)
{
	return tcindex_delete(tp, arg);
}

static void __tcindex_destroy(struct rcu_head *head)
{
	struct tcindex_data *p = container_of(head, struct tcindex_data, rcu);

	kfree(p->perfect);
	kfree(p->h);
	kfree(p);
}

static inline int
valid_perfect_hash(struct tcindex_data *p)
{
	return  p->hash > (p->mask >> p->shift);
}

static const struct nla_policy tcindex_policy[TCA_TCINDEX_MAX + 1] = {
	[TCA_TCINDEX_HASH]		= { .type = NLA_U32 },
	[TCA_TCINDEX_MASK]		= { .type = NLA_U16 },
	[TCA_TCINDEX_SHIFT]		= { .type = NLA_U32 },
	[TCA_TCINDEX_FALL_THROUGH]	= { .type = NLA_U32 },
	[TCA_TCINDEX_CLASSID]		= { .type = NLA_U32 },
};

static void tcindex_filter_result_init(struct tcindex_filter_result *r)
{
	memset(r, 0, sizeof(*r));
	tcf_exts_init(&r->exts, TCA_TCINDEX_ACT, TCA_TCINDEX_POLICE);
}

static void __tcindex_partial_destroy(struct rcu_head *head)
{
	struct tcindex_data *p = container_of(head, struct tcindex_data, rcu);

	kfree(p->perfect);
	kfree(p);
}

static int
tcindex_set_parms(struct net *net, struct tcf_proto *tp, unsigned long base,
		  u32 handle, struct tcindex_data *p,
		  struct tcindex_filter_result *r, struct nlattr **tb,
		  struct nlattr *est, bool ovr)
{
	int err, balloc = 0;
	struct tcindex_filter_result new_filter_result, *old_r = r;
	struct tcindex_filter_result cr;
	struct tcindex_data *cp, *oldp;
	struct tcindex_filter *f = NULL; /* make gcc behave */
	struct tcf_exts e;

	tcf_exts_init(&e, TCA_TCINDEX_ACT, TCA_TCINDEX_POLICE);
	err = tcf_exts_validate(net, tp, tb, est, &e, ovr);
	if (err < 0)
		return err;

	err = -ENOMEM;
	/* tcindex_data attributes must look atomic to classifier/lookup so
	 * allocate new tcindex data and RCU assign it onto root. Keeping
	 * perfect hash and hash pointers from old data.
	 */
	cp = kzalloc(sizeof(*cp), GFP_KERNEL);
	if (!cp)
		goto errout;

	cp->mask = p->mask;
	cp->shift = p->shift;
	cp->hash = p->hash;
	cp->alloc_hash = p->alloc_hash;
	cp->fall_through = p->fall_through;
	cp->tp = tp;

	if (p->perfect) {
		int i;

		cp->perfect = kmemdup(p->perfect,
				      sizeof(*r) * cp->hash, GFP_KERNEL);
		if (!cp->perfect)
			goto errout;
		for (i = 0; i < cp->hash; i++)
			tcf_exts_init(&cp->perfect[i].exts,
				      TCA_TCINDEX_ACT, TCA_TCINDEX_POLICE);
		balloc = 1;
	}
	cp->h = p->h;

	tcindex_filter_result_init(&new_filter_result);
	tcindex_filter_result_init(&cr);
	if (old_r)
		cr.res = r->res;

	if (tb[TCA_TCINDEX_HASH])
		cp->hash = nla_get_u32(tb[TCA_TCINDEX_HASH]);

	if (tb[TCA_TCINDEX_MASK])
		cp->mask = nla_get_u16(tb[TCA_TCINDEX_MASK]);

	if (tb[TCA_TCINDEX_SHIFT])
		cp->shift = nla_get_u32(tb[TCA_TCINDEX_SHIFT]);

	err = -EBUSY;

	/* Hash already allocated, make sure that we still meet the
	 * requirements for the allocated hash.
	 */
	if (cp->perfect) {
		if (!valid_perfect_hash(cp) ||
		    cp->hash > cp->alloc_hash)
			goto errout_alloc;
	} else if (cp->h && cp->hash != cp->alloc_hash) {
		goto errout_alloc;
	}

	err = -EINVAL;
	if (tb[TCA_TCINDEX_FALL_THROUGH])
		cp->fall_through = nla_get_u32(tb[TCA_TCINDEX_FALL_THROUGH]);

	if (!cp->hash) {
		/* Hash not specified, use perfect hash if the upper limit
		 * of the hashing index is below the threshold.
		 */
		if ((cp->mask >> cp->shift) < PERFECT_HASH_THRESHOLD)
			cp->hash = (cp->mask >> cp->shift) + 1;
		else
			cp->hash = DEFAULT_HASH_SIZE;
	}

	if (!cp->perfect && !cp->h)
		cp->alloc_hash = cp->hash;

	/* Note: this could be as restrictive as if (handle & ~(mask >> shift))
	 * but then, we'd fail handles that may become valid after some future
	 * mask change. While this is extremely unlikely to ever matter,
	 * the check below is safer (and also more backwards-compatible).
	 */
	if (cp->perfect || valid_perfect_hash(cp))
		if (handle >= cp->alloc_hash)
			goto errout_alloc;


	err = -ENOMEM;
	if (!cp->perfect && !cp->h) {
		if (valid_perfect_hash(cp)) {
			int i;

			cp->perfect = kcalloc(cp->hash, sizeof(*r), GFP_KERNEL);
			if (!cp->perfect)
				goto errout_alloc;
			for (i = 0; i < cp->hash; i++)
				tcf_exts_init(&cp->perfect[i].exts,
					      TCA_TCINDEX_ACT,
					      TCA_TCINDEX_POLICE);
			balloc = 1;
		} else {
			struct tcindex_filter __rcu **hash;

			hash = kcalloc(cp->hash,
				       sizeof(struct tcindex_filter *),
				       GFP_KERNEL);

			if (!hash)
				goto errout_alloc;

			cp->h = hash;
			balloc = 2;
		}
	}

	if (cp->perfect)
		r = cp->perfect + handle;
	else
		r = tcindex_lookup(cp, handle) ? : &new_filter_result;

	if (r == &new_filter_result) {
		f = kzalloc(sizeof(*f), GFP_KERNEL);
		if (!f)
			goto errout_alloc;
		f->key = handle;
		tcindex_filter_result_init(&f->result);
		f->next = NULL;
	}

	if (tb[TCA_TCINDEX_CLASSID]) {
		cr.res.classid = nla_get_u32(tb[TCA_TCINDEX_CLASSID]);
		tcf_bind_filter(tp, &cr.res, base);
	}

	if (old_r)
		tcf_exts_change(tp, &r->exts, &e);
	else
		tcf_exts_change(tp, &cr.exts, &e);

	if (old_r && old_r != r)
		tcindex_filter_result_init(old_r);

	oldp = p;
	r->res = cr.res;
	rcu_assign_pointer(tp->root, cp);

	if (r == &new_filter_result) {
		struct tcindex_filter *nfp;
		struct tcindex_filter __rcu **fp;

		tcf_exts_change(tp, &f->result.exts, &r->exts);

		fp = cp->h + (handle % cp->hash);
		for (nfp = rtnl_dereference(*fp);
		     nfp;
		     fp = &nfp->next, nfp = rtnl_dereference(*fp))
				; /* nothing */

		rcu_assign_pointer(*fp, f);
	}

	if (oldp)
		call_rcu(&oldp->rcu, __tcindex_partial_destroy);
	return 0;

errout_alloc:
	if (balloc == 1)
		kfree(cp->perfect);
	else if (balloc == 2)
		kfree(cp->h);
errout:
	kfree(cp);
	tcf_exts_destroy(&e);
	return err;
}

static int
tcindex_change(struct net *net, struct sk_buff *in_skb,
	       struct tcf_proto *tp, unsigned long base, u32 handle,
	       struct nlattr **tca, unsigned long *arg, bool ovr)
{
	struct nlattr *opt = tca[TCA_OPTIONS];
	struct nlattr *tb[TCA_TCINDEX_MAX + 1];
	struct tcindex_data *p = rtnl_dereference(tp->root);
	struct tcindex_filter_result *r = (struct tcindex_filter_result *) *arg;
	int err;

	pr_debug("tcindex_change(tp %p,handle 0x%08x,tca %p,arg %p),opt %p,"
	    "p %p,r %p,*arg 0x%lx\n",
	    tp, handle, tca, arg, opt, p, r, arg ? *arg : 0L);

	if (!opt)
		return 0;

	err = nla_parse_nested(tb, TCA_TCINDEX_MAX, opt, tcindex_policy);
	if (err < 0)
		return err;

	return tcindex_set_parms(net, tp, base, handle, p, r, tb,
				 tca[TCA_RATE], ovr);
}

static void tcindex_walk(struct tcf_proto *tp, struct tcf_walker *walker)
{
	struct tcindex_data *p = rtnl_dereference(tp->root);
	struct tcindex_filter *f, *next;
	int i;

	pr_debug("tcindex_walk(tp %p,walker %p),p %p\n", tp, walker, p);
	if (p->perfect) {
		for (i = 0; i < p->hash; i++) {
			if (!p->perfect[i].res.class)
				continue;
			if (walker->count >= walker->skip) {
				if (walker->fn(tp,
				    (unsigned long) (p->perfect+i), walker)
				     < 0) {
					walker->stop = 1;
					return;
				}
			}
			walker->count++;
		}
	}
	if (!p->h)
		return;
	for (i = 0; i < p->hash; i++) {
		for (f = rtnl_dereference(p->h[i]); f; f = next) {
			next = rtnl_dereference(f->next);
			if (walker->count >= walker->skip) {
				if (walker->fn(tp, (unsigned long) &f->result,
				    walker) < 0) {
					walker->stop = 1;
					return;
				}
			}
			walker->count++;
		}
	}
}

static bool tcindex_destroy(struct tcf_proto *tp, bool force)
{
	struct tcindex_data *p = rtnl_dereference(tp->root);
	struct tcf_walker walker;

	if (!force)
		return false;

	pr_debug("tcindex_destroy(tp %p),p %p\n", tp, p);
	walker.count = 0;
	walker.skip = 0;
	walker.fn = tcindex_destroy_element;
	tcindex_walk(tp, &walker);

	RCU_INIT_POINTER(tp->root, NULL);
	call_rcu(&p->rcu, __tcindex_destroy);
	return true;
}


static int tcindex_dump(struct net *net, struct tcf_proto *tp, unsigned long fh,
    struct sk_buff *skb, struct tcmsg *t)
{
	struct tcindex_data *p = rtnl_dereference(tp->root);
	struct tcindex_filter_result *r = (struct tcindex_filter_result *) fh;
	struct nlattr *nest;

	pr_debug("tcindex_dump(tp %p,fh 0x%lx,skb %p,t %p),p %p,r %p\n",
		 tp, fh, skb, t, p, r);
	pr_debug("p->perfect %p p->h %p\n", p->perfect, p->h);

	nest = nla_nest_start(skb, TCA_OPTIONS);
	if (nest == NULL)
		goto nla_put_failure;

	if (!fh) {
		t->tcm_handle = ~0; /* whatever ... */
		if (nla_put_u32(skb, TCA_TCINDEX_HASH, p->hash) ||
		    nla_put_u16(skb, TCA_TCINDEX_MASK, p->mask) ||
		    nla_put_u32(skb, TCA_TCINDEX_SHIFT, p->shift) ||
		    nla_put_u32(skb, TCA_TCINDEX_FALL_THROUGH, p->fall_through))
			goto nla_put_failure;
		nla_nest_end(skb, nest);
	} else {
		if (p->perfect) {
			t->tcm_handle = r - p->perfect;
		} else {
			struct tcindex_filter *f;
			struct tcindex_filter __rcu **fp;
			int i;

			t->tcm_handle = 0;
			for (i = 0; !t->tcm_handle && i < p->hash; i++) {
				fp = &p->h[i];
				for (f = rtnl_dereference(*fp);
				     !t->tcm_handle && f;
				     fp = &f->next, f = rtnl_dereference(*fp)) {
					if (&f->result == r)
						t->tcm_handle = f->key;
				}
			}
		}
		pr_debug("handle = %d\n", t->tcm_handle);
		if (r->res.class &&
		    nla_put_u32(skb, TCA_TCINDEX_CLASSID, r->res.classid))
			goto nla_put_failure;

		if (tcf_exts_dump(skb, &r->exts) < 0)
			goto nla_put_failure;
		nla_nest_end(skb, nest);

		if (tcf_exts_dump_stats(skb, &r->exts) < 0)
			goto nla_put_failure;
	}

	return skb->len;

nla_put_failure:
	nla_nest_cancel(skb, nest);
	return -1;
}

static struct tcf_proto_ops cls_tcindex_ops __read_mostly = {
	.kind		=	"tcindex",
	.classify	=	tcindex_classify,
	.init		=	tcindex_init,
	.destroy	=	tcindex_destroy,
	.get		=	tcindex_get,
	.change		=	tcindex_change,
	.delete		=	tcindex_delete,
	.walk		=	tcindex_walk,
	.dump		=	tcindex_dump,
	.owner		=	THIS_MODULE,
};

static int __init init_tcindex(void)
{
	return register_tcf_proto_ops(&cls_tcindex_ops);
}

static void __exit exit_tcindex(void)
{
	unregister_tcf_proto_ops(&cls_tcindex_ops);
}

module_init(init_tcindex)
module_exit(exit_tcindex)
MODULE_LICENSE("GPL");
