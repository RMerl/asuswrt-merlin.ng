/*
 * connection tracking helpers.
 *
 * 16 Dec 2003: Yasuyuki Kozakai @USAGI <yasuyuki.kozakai@toshiba.co.jp>
 *	- generalize L3 protocol dependent part.
 *
 * Derived from include/linux/netfiter_ipv4/ip_conntrack_helper.h
 */

#ifndef _NF_CONNTRACK_HELPER_H
#define _NF_CONNTRACK_HELPER_H
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_extend.h>
#include <net/netfilter/nf_conntrack_expect.h>

struct module;

enum nf_ct_helper_flags {
	NF_CT_HELPER_F_USERSPACE	= (1 << 0),
	NF_CT_HELPER_F_CONFIGURED	= (1 << 1),
};

#define NF_CT_HELPER_NAME_LEN	16

struct nf_conntrack_helper {
	struct hlist_node hnode;	/* Internal use. */

	char name[NF_CT_HELPER_NAME_LEN]; /* name of the module */
	struct module *me;		/* pointer to self */
	const struct nf_conntrack_expect_policy *expect_policy;

	/* length of internal data, ie. sizeof(struct nf_ct_*_master) */
	size_t data_len;

	/* Tuple of things we will help (compared against server response) */
	struct nf_conntrack_tuple tuple;

	/* Function to call when data passes; return verdict, or -1 to
           invalidate. */
	int (*help)(struct sk_buff *skb,
		    unsigned int protoff,
		    struct nf_conn *ct,
		    enum ip_conntrack_info conntrackinfo);

	void (*destroy)(struct nf_conn *ct);

	int (*from_nlattr)(struct nlattr *attr, struct nf_conn *ct);
	int (*to_nlattr)(struct sk_buff *skb, const struct nf_conn *ct);
	unsigned int expect_class_max;

	unsigned int flags;
	unsigned int queue_num;		/* For user-space helpers. */
};

struct nf_conntrack_helper *__nf_conntrack_helper_find(const char *name,
						       u16 l3num, u8 protonum);

struct nf_conntrack_helper *nf_conntrack_helper_try_module_get(const char *name,
							       u16 l3num,
							       u8 protonum);

int nf_conntrack_helper_register(struct nf_conntrack_helper *);
void nf_conntrack_helper_unregister(struct nf_conntrack_helper *);

struct nf_conn_help *nf_ct_helper_ext_add(struct nf_conn *ct,
					  struct nf_conntrack_helper *helper,
					  gfp_t gfp);

int __nf_ct_try_assign_helper(struct nf_conn *ct, struct nf_conn *tmpl,
			      gfp_t flags);

void nf_ct_helper_destroy(struct nf_conn *ct);

static inline struct nf_conn_help *nfct_help(const struct nf_conn *ct)
{
	return nf_ct_ext_find(ct, NF_CT_EXT_HELPER);
}

static inline void *nfct_help_data(const struct nf_conn *ct)
{
	struct nf_conn_help *help;

	help = nf_ct_ext_find(ct, NF_CT_EXT_HELPER);

	return (void *)help->data;
}

int nf_conntrack_helper_pernet_init(struct net *net);
void nf_conntrack_helper_pernet_fini(struct net *net);

int nf_conntrack_helper_init(void);
void nf_conntrack_helper_fini(void);

int nf_conntrack_broadcast_help(struct sk_buff *skb, unsigned int protoff,
				struct nf_conn *ct,
				enum ip_conntrack_info ctinfo,
				unsigned int timeout);

struct nf_ct_helper_expectfn {
	struct list_head head;
	const char *name;
	void (*expectfn)(struct nf_conn *ct, struct nf_conntrack_expect *exp);
};

__printf(3,4)
void nf_ct_helper_log(struct sk_buff *skb, const struct nf_conn *ct,
		      const char *fmt, ...);

void nf_ct_helper_expectfn_register(struct nf_ct_helper_expectfn *n);
void nf_ct_helper_expectfn_unregister(struct nf_ct_helper_expectfn *n);
struct nf_ct_helper_expectfn *
nf_ct_helper_expectfn_find_by_name(const char *name);
struct nf_ct_helper_expectfn *
nf_ct_helper_expectfn_find_by_symbol(const void *symbol);

extern struct hlist_head *nf_ct_helper_hash;
extern unsigned int nf_ct_helper_hsize;

#endif /*_NF_CONNTRACK_HELPER_H*/
