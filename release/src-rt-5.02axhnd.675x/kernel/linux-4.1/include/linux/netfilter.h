#ifndef __LINUX_NETFILTER_H
#define __LINUX_NETFILTER_H

#include <linux/init.h>
#include <linux/skbuff.h>
#include <linux/net.h>
#include <linux/if.h>
#include <linux/in.h>
#include <linux/in6.h>
#include <linux/wait.h>
#include <linux/list.h>
#include <linux/static_key.h>
#include <uapi/linux/netfilter.h>
#ifdef CONFIG_NETFILTER
static inline int NF_DROP_GETERR(int verdict)
{
	return -(verdict >> NF_VERDICT_QBITS);
}

static inline int nf_inet_addr_cmp(const union nf_inet_addr *a1,
				   const union nf_inet_addr *a2)
{
	return a1->all[0] == a2->all[0] &&
	       a1->all[1] == a2->all[1] &&
	       a1->all[2] == a2->all[2] &&
	       a1->all[3] == a2->all[3];
}

static inline void nf_inet_addr_mask(const union nf_inet_addr *a1,
				     union nf_inet_addr *result,
				     const union nf_inet_addr *mask)
{
	result->all[0] = a1->all[0] & mask->all[0];
	result->all[1] = a1->all[1] & mask->all[1];
	result->all[2] = a1->all[2] & mask->all[2];
	result->all[3] = a1->all[3] & mask->all[3];
}

int netfilter_init(void);

/* Largest hook number + 1 */
#define NF_MAX_HOOKS 8

struct sk_buff;

struct nf_hook_ops;

struct sock;

struct nf_hook_state {
	unsigned int hook;
	int thresh;
	u_int8_t pf;
	struct net_device *in;
	struct net_device *out;
	struct sock *sk;
	int (*okfn)(struct sock *, struct sk_buff *);
};

static inline void nf_hook_state_init(struct nf_hook_state *p,
				      unsigned int hook,
				      int thresh, u_int8_t pf,
				      struct net_device *indev,
				      struct net_device *outdev,
				      struct sock *sk,
				      int (*okfn)(struct sock *, struct sk_buff *))
{
	p->hook = hook;
	p->thresh = thresh;
	p->pf = pf;
	p->in = indev;
	p->out = outdev;
	p->sk = sk;
	p->okfn = okfn;
}

typedef unsigned int nf_hookfn(const struct nf_hook_ops *ops,
			       struct sk_buff *skb,
			       const struct nf_hook_state *state);

struct nf_hook_ops {
	struct list_head list;

	/* User fills in from here down. */
	nf_hookfn	*hook;
	struct module	*owner;
	void		*priv;
	u_int8_t	pf;
	unsigned int	hooknum;
	/* Hooks are ordered in ascending priority. */
	int		priority;
};

struct nf_sockopt_ops {
	struct list_head list;

	u_int8_t pf;

	/* Non-inclusive ranges: use 0/0/NULL to never get called. */
	int set_optmin;
	int set_optmax;
	int (*set)(struct sock *sk, int optval, void __user *user, unsigned int len);
#ifdef CONFIG_COMPAT
	int (*compat_set)(struct sock *sk, int optval,
			void __user *user, unsigned int len);
#endif
	int get_optmin;
	int get_optmax;
	int (*get)(struct sock *sk, int optval, void __user *user, int *len);
#ifdef CONFIG_COMPAT
	int (*compat_get)(struct sock *sk, int optval,
			void __user *user, int *len);
#endif
	/* Use the module struct to lock set/get code in place */
	struct module *owner;
};

/* Function to register/unregister hook points. */
int nf_register_hook(struct nf_hook_ops *reg);
void nf_unregister_hook(struct nf_hook_ops *reg);
int nf_register_hooks(struct nf_hook_ops *reg, unsigned int n);
void nf_unregister_hooks(struct nf_hook_ops *reg, unsigned int n);

/* Functions to register get/setsockopt ranges (non-inclusive).  You
   need to check permissions yourself! */
int nf_register_sockopt(struct nf_sockopt_ops *reg);
void nf_unregister_sockopt(struct nf_sockopt_ops *reg);

extern struct list_head nf_hooks[NFPROTO_NUMPROTO][NF_MAX_HOOKS];

#ifdef HAVE_JUMP_LABEL
extern struct static_key nf_hooks_needed[NFPROTO_NUMPROTO][NF_MAX_HOOKS];

static inline bool nf_hooks_active(u_int8_t pf, unsigned int hook)
{
	if (__builtin_constant_p(pf) &&
	    __builtin_constant_p(hook))
		return static_key_false(&nf_hooks_needed[pf][hook]);

	return !list_empty(&nf_hooks[pf][hook]);
}
#else
static inline bool nf_hooks_active(u_int8_t pf, unsigned int hook)
{
	return !list_empty(&nf_hooks[pf][hook]);
}
#endif

int nf_hook_slow(struct sk_buff *skb, struct nf_hook_state *state);

/**
 *	nf_hook_thresh - call a netfilter hook
 *	
 *	Returns 1 if the hook has allowed the packet to pass.  The function
 *	okfn must be invoked by the caller in this case.  Any other return
 *	value indicates the packet has been consumed by the hook.
 */
static inline int nf_hook_thresh(u_int8_t pf, unsigned int hook,
				 struct sock *sk,
				 struct sk_buff *skb,
				 struct net_device *indev,
				 struct net_device *outdev,
				 int (*okfn)(struct sock *, struct sk_buff *),
				 int thresh)
{
	if (nf_hooks_active(pf, hook)) {
		struct nf_hook_state state;

		nf_hook_state_init(&state, hook, thresh, pf,
				   indev, outdev, sk, okfn);
		return nf_hook_slow(skb, &state);
	}
	return 1;
}

static inline int nf_hook(u_int8_t pf, unsigned int hook, struct sock *sk,
			  struct sk_buff *skb, struct net_device *indev,
			  struct net_device *outdev,
			  int (*okfn)(struct sock *, struct sk_buff *))
{
	return nf_hook_thresh(pf, hook, sk, skb, indev, outdev, okfn, INT_MIN);
}
                   
/* Activate hook; either okfn or kfree_skb called, unless a hook
   returns NF_STOLEN (in which case, it's up to the hook to deal with
   the consequences).

   Returns -ERRNO if packet dropped.  Zero means queued, stolen or
   accepted.
*/

/* RR:
   > I don't want nf_hook to return anything because people might forget
   > about async and trust the return value to mean "packet was ok".

   AK:
   Just document it clearly, then you can expect some sense from kernel
   coders :)
*/

static inline int
NF_HOOK_THRESH(uint8_t pf, unsigned int hook, struct sock *sk,
	       struct sk_buff *skb, struct net_device *in,
	       struct net_device *out,
	       int (*okfn)(struct sock *, struct sk_buff *), int thresh)
{
	int ret = nf_hook_thresh(pf, hook, sk, skb, in, out, okfn, thresh);
	if (ret == 1)
		ret = okfn(sk, skb);
	return ret;
}

static inline int
NF_HOOK_COND(uint8_t pf, unsigned int hook, struct sock *sk,
	     struct sk_buff *skb, struct net_device *in, struct net_device *out,
	     int (*okfn)(struct sock *, struct sk_buff *), bool cond)
{
	int ret;

	if (!cond ||
	    ((ret = nf_hook_thresh(pf, hook, sk, skb, in, out, okfn, INT_MIN)) == 1))
		ret = okfn(sk, skb);
	return ret;
}

static inline int
NF_HOOK(uint8_t pf, unsigned int hook, struct sock *sk, struct sk_buff *skb,
	struct net_device *in, struct net_device *out,
	int (*okfn)(struct sock *, struct sk_buff *))
{
	return NF_HOOK_THRESH(pf, hook, sk, skb, in, out, okfn, INT_MIN);
}

/* Call setsockopt() */
int nf_setsockopt(struct sock *sk, u_int8_t pf, int optval, char __user *opt,
		  unsigned int len);
int nf_getsockopt(struct sock *sk, u_int8_t pf, int optval, char __user *opt,
		  int *len);
#ifdef CONFIG_COMPAT
int compat_nf_setsockopt(struct sock *sk, u_int8_t pf, int optval,
		char __user *opt, unsigned int len);
int compat_nf_getsockopt(struct sock *sk, u_int8_t pf, int optval,
		char __user *opt, int *len);
#endif

/* Call this before modifying an existing packet: ensures it is
   modifiable and linear to the point you care about (writable_len).
   Returns true or false. */
int skb_make_writable(struct sk_buff *skb, unsigned int writable_len);

struct flowi;
struct nf_queue_entry;

struct nf_afinfo {
	unsigned short	family;
	__sum16		(*checksum)(struct sk_buff *skb, unsigned int hook,
				    unsigned int dataoff, u_int8_t protocol);
	__sum16		(*checksum_partial)(struct sk_buff *skb,
					    unsigned int hook,
					    unsigned int dataoff,
					    unsigned int len,
					    u_int8_t protocol);
	int		(*route)(struct net *net, struct dst_entry **dst,
				 struct flowi *fl, bool strict);
	void		(*saveroute)(const struct sk_buff *skb,
				     struct nf_queue_entry *entry);
	int		(*reroute)(struct sk_buff *skb,
				   const struct nf_queue_entry *entry);
	int		route_key_size;
};

extern const struct nf_afinfo __rcu *nf_afinfo[NFPROTO_NUMPROTO];
static inline const struct nf_afinfo *nf_get_afinfo(unsigned short family)
{
	return rcu_dereference(nf_afinfo[family]);
}

static inline __sum16
nf_checksum(struct sk_buff *skb, unsigned int hook, unsigned int dataoff,
	    u_int8_t protocol, unsigned short family)
{
	const struct nf_afinfo *afinfo;
	__sum16 csum = 0;

	rcu_read_lock();
	afinfo = nf_get_afinfo(family);
	if (afinfo)
		csum = afinfo->checksum(skb, hook, dataoff, protocol);
	rcu_read_unlock();
	return csum;
}

static inline __sum16
nf_checksum_partial(struct sk_buff *skb, unsigned int hook,
		    unsigned int dataoff, unsigned int len,
		    u_int8_t protocol, unsigned short family)
{
	const struct nf_afinfo *afinfo;
	__sum16 csum = 0;

	rcu_read_lock();
	afinfo = nf_get_afinfo(family);
	if (afinfo)
		csum = afinfo->checksum_partial(skb, hook, dataoff, len,
						protocol);
	rcu_read_unlock();
	return csum;
}

int nf_register_afinfo(const struct nf_afinfo *afinfo);
void nf_unregister_afinfo(const struct nf_afinfo *afinfo);

#include <net/flow.h>
extern void (*nf_nat_decode_session_hook)(struct sk_buff *, struct flowi *);

static inline void
nf_nat_decode_session(struct sk_buff *skb, struct flowi *fl, u_int8_t family)
{
#ifdef CONFIG_NF_NAT_NEEDED
	void (*decodefn)(struct sk_buff *, struct flowi *);

	rcu_read_lock();
	decodefn = rcu_dereference(nf_nat_decode_session_hook);
	if (decodefn)
		decodefn(skb, fl);
	rcu_read_unlock();
#endif
}

#else /* !CONFIG_NETFILTER */
#define NF_HOOK(pf, hook, sk, skb, indev, outdev, okfn) (okfn)(sk, skb)
#define NF_HOOK_COND(pf, hook, sk, skb, indev, outdev, okfn, cond) (okfn)(sk, skb)
static inline int nf_hook_thresh(u_int8_t pf, unsigned int hook,
				 struct sock *sk,
				 struct sk_buff *skb,
				 struct net_device *indev,
				 struct net_device *outdev,
				 int (*okfn)(struct sock *sk, struct sk_buff *), int thresh)
{
	return okfn(sk, skb);
}
static inline int nf_hook(u_int8_t pf, unsigned int hook, struct sock *sk,
			  struct sk_buff *skb, struct net_device *indev,
			  struct net_device *outdev,
			  int (*okfn)(struct sock *, struct sk_buff *))
{
	return 1;
}
struct flowi;
static inline void
nf_nat_decode_session(struct sk_buff *skb, struct flowi *fl, u_int8_t family)
{
}
#endif /*CONFIG_NETFILTER*/

#if defined(CONFIG_NF_CONNTRACK) || defined(CONFIG_NF_CONNTRACK_MODULE)
extern void (*ip_ct_attach)(struct sk_buff *, const struct sk_buff *) __rcu;
void nf_ct_attach(struct sk_buff *, const struct sk_buff *);
extern void (*nf_ct_destroy)(struct nf_conntrack *) __rcu;

struct nf_conn;
enum ip_conntrack_info;
struct nlattr;

struct nfq_ct_hook {
	size_t (*build_size)(const struct nf_conn *ct);
	int (*build)(struct sk_buff *skb, struct nf_conn *ct);
	int (*parse)(const struct nlattr *attr, struct nf_conn *ct);
	int (*attach_expect)(const struct nlattr *attr, struct nf_conn *ct,
			     u32 portid, u32 report);
	void (*seq_adjust)(struct sk_buff *skb, struct nf_conn *ct,
			   enum ip_conntrack_info ctinfo, s32 off);
};
extern struct nfq_ct_hook __rcu *nfq_ct_hook;
#else
static inline void nf_ct_attach(struct sk_buff *new, struct sk_buff *skb) {}
#endif

#endif /*__LINUX_NETFILTER_H*/
