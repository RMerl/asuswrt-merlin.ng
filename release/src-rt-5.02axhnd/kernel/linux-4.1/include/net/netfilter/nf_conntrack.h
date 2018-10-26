/*
 * Connection state tracking for netfilter.  This is separated from,
 * but required by, the (future) NAT layer; it can also be used by an iptables
 * extension.
 *
 * 16 Dec 2003: Yasuyuki Kozakai @USAGI <yasuyuki.kozakai@toshiba.co.jp>
 *	- generalize L3 protocol dependent part.
 *
 * Derived from include/linux/netfiter_ipv4/ip_conntrack.h
 */

#ifndef _NF_CONNTRACK_H
#define _NF_CONNTRACK_H

#include <linux/netfilter/nf_conntrack_common.h>

#include <linux/bitops.h>
#include <linux/compiler.h>
#include <linux/atomic.h>

#include <linux/netfilter/nf_conntrack_tcp.h>
#include <linux/netfilter/nf_conntrack_dccp.h>
#include <linux/netfilter/nf_conntrack_sctp.h>
#include <linux/netfilter/nf_conntrack_proto_gre.h>
#if defined(CONFIG_BCM_KF_PROTO_IPSEC) && \
	(defined(CONFIG_NF_CONNTRACK_IPSEC) || defined(CONFIG_NF_CONNTRACK_IPSEC_MODULE))
#include <linux/netfilter/nf_conntrack_ipsec.h>
#include <linux/netfilter/nf_conntrack_proto_esp.h>
#endif
#include <net/netfilter/ipv6/nf_conntrack_icmpv6.h>

#include <net/netfilter/nf_conntrack_tuple.h>

/* per conntrack: protocol private data */
union nf_conntrack_proto {
	/* insert conntrack proto private data here */
	struct nf_ct_dccp dccp;
	struct ip_ct_sctp sctp;
	struct ip_ct_tcp tcp;
	struct nf_ct_gre gre;
#if defined(CONFIG_BCM_KF_PROTO_ESP) && \
	(defined(CONFIG_NF_CT_PROTO_ESP) || defined(CONFIG_NF_CT_PROTO_ESP_MODULE))
	struct nf_ct_esp esp;
#endif
};

union nf_conntrack_expect_proto {
	/* insert expect proto private data here */
};

#if defined(CONFIG_BCM_KF_NETFILTER)
#define NF_ALG_BUFFER_SIZE 2000
#endif

#include <linux/types.h>
#include <linux/skbuff.h>
#include <linux/timer.h>

#ifdef CONFIG_NETFILTER_DEBUG
#define NF_CT_ASSERT(x)		WARN_ON(!(x))
#else
#define NF_CT_ASSERT(x)
#endif

struct nf_conntrack_helper;

/* Must be kept in sync with the classes defined by helpers */
#define NF_CT_MAX_EXPECT_CLASSES	4

/* nf_conn feature for connections that have a helper */
struct nf_conn_help {
	/* Helper. if any */
	struct nf_conntrack_helper __rcu *helper;

	struct hlist_head expectations;

	/* Current number of expected connections */
	u8 expecting[NF_CT_MAX_EXPECT_CLASSES];

	/* private helper information. */
	char data[];
};

#include <net/netfilter/ipv4/nf_conntrack_ipv4.h>
#include <net/netfilter/ipv6/nf_conntrack_ipv6.h>

#if defined(CONFIG_BCM_KF_DPI) && defined(CONFIG_BCM_DPI_MODULE)
#include <linux/dpi.h>
#endif

struct nf_conn {
	/* Usage count in here is 1 for hash table/destruct timer, 1 per skb,
	 * plus 1 for any connection(s) we are `master' for
	 *
	 * Hint, SKB address this struct and refcnt via skb->nfct and
	 * helpers nf_conntrack_get() and nf_conntrack_put().
	 * Helper nf_ct_put() equals nf_conntrack_put() by dec refcnt,
	 * beware nf_ct_get() is different and don't inc refcnt.
	 */
	struct nf_conntrack ct_general;

	spinlock_t	lock;
	u16		cpu;

#if defined(CONFIG_BCM_KF_BLOG)
#if defined(CONFIG_BLOG)
	unsigned int blog_key[2];	/* Associating 2=IP_CT_DIR_MAX blogged flows  */
	unsigned long idle_jiffies; /* connection idled duration, 0 means active  */
	unsigned long extra_jiffies;/* connection timeout value                   */
	unsigned long prev_idle;    /* previous idle state                        */
	struct timer_list prev_timeout;
#endif
	uint8_t iq_prio;			/* Ingress QoS Prio */
	uint8_t unused0;
	uint16_t unused1;
#endif

#if defined(CONFIG_BCM_KF_NETFILTER)
	uint32_t hw_accel_flows; /* Number of HW accelerated flows, need 32 bit for tunnel connection */
	uint32_t sw_accel_flows; /* Number of SW accelerated flows, need 32 bit for tunnel connection */
	struct list_head safe_list; 	/* regardless drop of connections */
	struct list_head *curr_safe_list; /* current safe_list for this connection*/
	struct list_head derived_connections; /* Used by master connection */
	struct list_head derived_list; /* Used by child connection */
	unsigned derived_timeout;	/* 0 means no derived_timeout, 0xFFFFFFFF
								 * means never timeout until master ct is
								 * disconnected, others means timeout secs */

	/* Have we seen traffic both ways yet? (bitset) */ // bcm version
	unsigned long status; // moved position for bcm

#if defined(CONFIG_NF_DYNDSCP) || defined(CONFIG_NF_DYNDSCP_MODULE)
	struct nf_tos_inheritance {
		u_int16_t status;
		u_int8_t dscp[2];		/* IP_CT_DIR_MAX */
	}dyndscp; 
#endif
	/*---------- Add any custom fields below this line ----------*/

	/* If we were expected by an expectation, this will be it */
	struct nf_conn *master;  // moved position for bcm
#endif /* CONFIG_BCM_KF_NETFILTER */

#if defined(CONFIG_BCM_KF_DPI) && defined(CONFIG_BCM_DPI_MODULE)
	struct dpi_info dpi;
#endif
	/* XXX should I move this to the tail ? - Y.K */
	/* These are my tuples; original and reply */
	struct nf_conntrack_tuple_hash tuplehash[IP_CT_DIR_MAX];

#if !defined(CONFIG_BCM_KF_NETFILTER)
	/* Have we seen traffic both ways yet? (bitset) */
	unsigned long status;

#endif
	/* Timer function; drops refcnt when it goes off. */
	struct timer_list timeout;

	possible_net_t ct_net;

	/* all members below initialized via memset */
	u8 __nfct_init_offset[0];

#if !defined(CONFIG_BCM_KF_NETFILTER)
	/* If we were expected by an expectation, this will be it */
	struct nf_conn *master;
#endif

#if defined(CONFIG_NF_CONNTRACK_MARK)
	u_int32_t mark;
#endif

#ifdef CONFIG_NF_CONNTRACK_SECMARK
	u_int32_t secmark;
#endif

	/* Extensions */
	struct nf_ct_ext *ext;

	/* Storage reserved for other modules, must be the last member */
	union nf_conntrack_proto proto;

#if defined(CONFIG_BCM_KF_RUNNER)
#if defined(CONFIG_BCM_RDPA) || defined(CONFIG_BCM_RDPA_MODULE)
	void *bl_ctx;
#endif /* CONFIG_BCM_RDPA || CONFIG_BCM_RDPA_MODULE */
#endif /* CONFIG_BCM_KF_RUNNER */

#if defined(CONFIG_BCM_KF_XT_MATCH_LAYER7) && \
	(defined(CONFIG_NETFILTER_XT_MATCH_LAYER7) || defined(CONFIG_NETFILTER_XT_MATCH_LAYER7_MODULE))
	struct {
		/*
		* e.g. "http". NULL before decision. "unknown" after decision
		* if no match.
		*/
		char *app_proto;
		/*
		* application layer data so far. NULL after match decision.
		*/
		char *app_data;
		unsigned int app_data_len;
	} layer7;
#endif 
};

static inline struct nf_conn *
nf_ct_tuplehash_to_ctrack(const struct nf_conntrack_tuple_hash *hash)
{
	return container_of(hash, struct nf_conn,
			    tuplehash[hash->tuple.dst.dir]);
}

static inline u_int16_t nf_ct_l3num(const struct nf_conn *ct)
{
	return ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.l3num;
}

static inline u_int8_t nf_ct_protonum(const struct nf_conn *ct)
{
	return ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum;
}

#define nf_ct_tuple(ct, dir) (&(ct)->tuplehash[dir].tuple)

/* get master conntrack via master expectation */
#define master_ct(conntr) (conntr->master)

extern struct net init_net;

static inline struct net *nf_ct_net(const struct nf_conn *ct)
{
	return read_pnet(&ct->ct_net);
}

/* Alter reply tuple (maybe alter helper). */
void nf_conntrack_alter_reply(struct nf_conn *ct,
			      const struct nf_conntrack_tuple *newreply);

/* Is this tuple taken? (ignoring any belonging to the given
   conntrack). */
int nf_conntrack_tuple_taken(const struct nf_conntrack_tuple *tuple,
			     const struct nf_conn *ignored_conntrack);

/* Return conntrack_info and tuple hash for given skb. */
static inline struct nf_conn *
nf_ct_get(const struct sk_buff *skb, enum ip_conntrack_info *ctinfo)
{
	*ctinfo = skb->nfctinfo;
	return (struct nf_conn *)skb->nfct;
}

/* decrement reference count on a conntrack */
static inline void nf_ct_put(struct nf_conn *ct)
{
	NF_CT_ASSERT(ct);
	nf_conntrack_put(&ct->ct_general);
}

/* Protocol module loading */
int nf_ct_l3proto_try_module_get(unsigned short l3proto);
void nf_ct_l3proto_module_put(unsigned short l3proto);

/*
 * Allocate a hashtable of hlist_head (if nulls == 0),
 * or hlist_nulls_head (if nulls == 1)
 */
void *nf_ct_alloc_hashtable(unsigned int *sizep, int nulls);

void nf_ct_free_hashtable(void *hash, unsigned int size);

struct nf_conntrack_tuple_hash *
__nf_conntrack_find(struct net *net, u16 zone,
		    const struct nf_conntrack_tuple *tuple);

int nf_conntrack_hash_check_insert(struct nf_conn *ct);
bool nf_ct_delete(struct nf_conn *ct, u32 pid, int report);

bool nf_ct_get_tuplepr(const struct sk_buff *skb, unsigned int nhoff,
		       u_int16_t l3num, struct nf_conntrack_tuple *tuple);
bool nf_ct_invert_tuplepr(struct nf_conntrack_tuple *inverse,
			  const struct nf_conntrack_tuple *orig);

void __nf_ct_refresh_acct(struct nf_conn *ct, enum ip_conntrack_info ctinfo,
			  const struct sk_buff *skb,
			  unsigned long extra_jiffies, int do_acct);

/* Refresh conntrack for this many jiffies and do accounting */
static inline void nf_ct_refresh_acct(struct nf_conn *ct,
				      enum ip_conntrack_info ctinfo,
				      const struct sk_buff *skb,
				      unsigned long extra_jiffies)
{
	__nf_ct_refresh_acct(ct, ctinfo, skb, extra_jiffies, 1);
}

/* Refresh conntrack for this many jiffies */
static inline void nf_ct_refresh(struct nf_conn *ct,
				 const struct sk_buff *skb,
				 unsigned long extra_jiffies)
{
	__nf_ct_refresh_acct(ct, 0, skb, extra_jiffies, 0);
}

bool __nf_ct_kill_acct(struct nf_conn *ct, enum ip_conntrack_info ctinfo,
		       const struct sk_buff *skb, int do_acct);

/* kill conntrack and do accounting */
static inline bool nf_ct_kill_acct(struct nf_conn *ct,
				   enum ip_conntrack_info ctinfo,
				   const struct sk_buff *skb)
{
	return __nf_ct_kill_acct(ct, ctinfo, skb, 1);
}

/* kill conntrack without accounting */
static inline bool nf_ct_kill(struct nf_conn *ct)
{
	return __nf_ct_kill_acct(ct, 0, NULL, 0);
}

/* These are for NAT.  Icky. */
extern s32 (*nf_ct_nat_offset)(const struct nf_conn *ct,
			       enum ip_conntrack_dir dir,
			       u32 seq);

/* Fake conntrack entry for untracked connections */
DECLARE_PER_CPU(struct nf_conn, nf_conntrack_untracked);
static inline struct nf_conn *nf_ct_untracked_get(void)
{
	return raw_cpu_ptr(&nf_conntrack_untracked);
}
void nf_ct_untracked_status_or(unsigned long bits);

/* Iterate over all conntracks: if iter returns true, it's deleted. */
void nf_ct_iterate_cleanup(struct net *net,
			   int (*iter)(struct nf_conn *i, void *data),
			   void *data, u32 portid, int report);
void nf_conntrack_free(struct nf_conn *ct);
#if defined(CONFIG_BCM_KF_NETFILTER)
struct nf_conn *
nf_conntrack_alloc(struct net *net, u16 zone,
				   struct sk_buff *skb,
				   const struct nf_conntrack_tuple *orig,
				   const struct nf_conntrack_tuple *repl,
				   gfp_t gfp);
#else
struct nf_conn *nf_conntrack_alloc(struct net *net, u16 zone,
				   const struct nf_conntrack_tuple *orig,
				   const struct nf_conntrack_tuple *repl,
				   gfp_t gfp);
#endif

static inline int nf_ct_is_template(const struct nf_conn *ct)
{
	return test_bit(IPS_TEMPLATE_BIT, &ct->status);
}

/* It's confirmed if it is, or has been in the hash table. */
static inline int nf_ct_is_confirmed(struct nf_conn *ct)
{
	return test_bit(IPS_CONFIRMED_BIT, &ct->status);
}

static inline int nf_ct_is_dying(struct nf_conn *ct)
{
	return test_bit(IPS_DYING_BIT, &ct->status);
}

static inline int nf_ct_is_untracked(const struct nf_conn *ct)
{
	return test_bit(IPS_UNTRACKED_BIT, &ct->status);
}

/* Packet is received from loopback */
static inline bool nf_is_loopback_packet(const struct sk_buff *skb)
{
	return skb->dev && skb->skb_iif && skb->dev->flags & IFF_LOOPBACK;
}

struct kernel_param;

int nf_conntrack_set_hashsize(const char *val, struct kernel_param *kp);
extern unsigned int nf_conntrack_htable_size;
extern unsigned int nf_conntrack_max;
extern unsigned int nf_conntrack_hash_rnd;
void init_nf_conntrack_hash_rnd(void);

void nf_conntrack_tmpl_insert(struct net *net, struct nf_conn *tmpl);

#define NF_CT_STAT_INC(net, count)	  __this_cpu_inc((net)->ct.stat->count)
#define NF_CT_STAT_INC_ATOMIC(net, count) this_cpu_inc((net)->ct.stat->count)

#define MODULE_ALIAS_NFCT_HELPER(helper) \
        MODULE_ALIAS("nfct-helper-" helper)

#endif /* _NF_CONNTRACK_H */
