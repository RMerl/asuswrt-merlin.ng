/*
 * (C) 2005-2011 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef _LIBNETFILTER_CONNTRACK_H_
#define _LIBNETFILTER_CONNTRACK_H_

#include <stdbool.h>
#include <netinet/in.h>
#include <libnfnetlink/linux_nfnetlink.h>
#include <libnfnetlink/libnfnetlink.h>
#include <libnetfilter_conntrack/linux_nfnetlink_conntrack.h>
#include <libnetfilter_conntrack/linux_nf_conntrack_common.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
	CONNTRACK = NFNL_SUBSYS_CTNETLINK,
	EXPECT = NFNL_SUBSYS_CTNETLINK_EXP
};

/*
 * Subscribe to all possible conntrack event groups. Use this 
 * flag in case that you want to catch up all the possible 
 * events. Do not use this flag for dumping or any other
 * similar operation.
 */
#define NFCT_ALL_CT_GROUPS (NF_NETLINK_CONNTRACK_NEW|NF_NETLINK_CONNTRACK_UPDATE|NF_NETLINK_CONNTRACK_DESTROY)

struct nfct_handle;

/*
 * [Open|close] a conntrack handler
 */
extern struct nfct_handle *nfct_open(uint8_t, unsigned);
extern struct nfct_handle *nfct_open_nfnl(struct nfnl_handle *nfnlh,
					  uint8_t subsys_id,
					  unsigned int subscriptions);
extern int nfct_close(struct nfct_handle *cth);

extern int nfct_fd(struct nfct_handle *cth);
extern const struct nfnl_handle *nfct_nfnlh(struct nfct_handle *cth);

/* 
 * NEW libnetfilter_conntrack API 
 */

/* high level API */

#include <sys/types.h>

/* conntrack object */
struct nf_conntrack;

/* conntrack attributes */
enum nf_conntrack_attr {
	ATTR_ORIG_IPV4_SRC = 0,			/* u32 bits */
	ATTR_IPV4_SRC = ATTR_ORIG_IPV4_SRC,	/* alias */
	ATTR_ORIG_IPV4_DST,			/* u32 bits */
	ATTR_IPV4_DST = ATTR_ORIG_IPV4_DST,	/* alias */
	ATTR_REPL_IPV4_SRC,			/* u32 bits */
	ATTR_REPL_IPV4_DST,			/* u32 bits */
	ATTR_ORIG_IPV6_SRC = 4,			/* u128 bits */
	ATTR_IPV6_SRC = ATTR_ORIG_IPV6_SRC,	/* alias */
	ATTR_ORIG_IPV6_DST,			/* u128 bits */
	ATTR_IPV6_DST = ATTR_ORIG_IPV6_DST,	/* alias */
	ATTR_REPL_IPV6_SRC,			/* u128 bits */
	ATTR_REPL_IPV6_DST,			/* u128 bits */
	ATTR_ORIG_PORT_SRC = 8,			/* u16 bits */
	ATTR_PORT_SRC = ATTR_ORIG_PORT_SRC,	/* alias */
	ATTR_ORIG_PORT_DST,			/* u16 bits */
	ATTR_PORT_DST = ATTR_ORIG_PORT_DST,	/* alias */
	ATTR_REPL_PORT_SRC,			/* u16 bits */
	ATTR_REPL_PORT_DST,			/* u16 bits */
	ATTR_ICMP_TYPE = 12,			/* u8 bits */
	ATTR_ICMP_CODE,				/* u8 bits */
	ATTR_ICMP_ID,				/* u16 bits */
	ATTR_ORIG_L3PROTO,			/* u8 bits */
	ATTR_L3PROTO = ATTR_ORIG_L3PROTO,	/* alias */
	ATTR_REPL_L3PROTO = 16,			/* u8 bits */
	ATTR_ORIG_L4PROTO,			/* u8 bits */
	ATTR_L4PROTO = ATTR_ORIG_L4PROTO,	/* alias */
	ATTR_REPL_L4PROTO,			/* u8 bits */
	ATTR_TCP_STATE,				/* u8 bits */
	ATTR_SNAT_IPV4 = 20,			/* u32 bits */
	ATTR_DNAT_IPV4,				/* u32 bits */
	ATTR_SNAT_PORT,				/* u16 bits */
	ATTR_DNAT_PORT,				/* u16 bits */
	ATTR_TIMEOUT = 24,			/* u32 bits */
	ATTR_MARK,				/* u32 bits */
	ATTR_ORIG_COUNTER_PACKETS,		/* u64 bits */
	ATTR_REPL_COUNTER_PACKETS,		/* u64 bits */
	ATTR_ORIG_COUNTER_BYTES = 28,		/* u64 bits */
	ATTR_REPL_COUNTER_BYTES,		/* u64 bits */
	ATTR_USE,				/* u32 bits */
	ATTR_ID,				/* u32 bits */
	ATTR_STATUS = 32,			/* u32 bits  */
	ATTR_TCP_FLAGS_ORIG,			/* u8 bits */
	ATTR_TCP_FLAGS_REPL,			/* u8 bits */
	ATTR_TCP_MASK_ORIG,			/* u8 bits */
	ATTR_TCP_MASK_REPL = 36,		/* u8 bits */
	ATTR_MASTER_IPV4_SRC,			/* u32 bits */
	ATTR_MASTER_IPV4_DST,			/* u32 bits */
	ATTR_MASTER_IPV6_SRC,			/* u128 bits */
	ATTR_MASTER_IPV6_DST = 40,		/* u128 bits */
	ATTR_MASTER_PORT_SRC,			/* u16 bits */
	ATTR_MASTER_PORT_DST,			/* u16 bits */
	ATTR_MASTER_L3PROTO,			/* u8 bits */
	ATTR_MASTER_L4PROTO = 44,		/* u8 bits */
	ATTR_SECMARK,				/* u32 bits */
	ATTR_ORIG_NAT_SEQ_CORRECTION_POS,	/* u32 bits */
	ATTR_ORIG_NAT_SEQ_OFFSET_BEFORE,	/* u32 bits */
	ATTR_ORIG_NAT_SEQ_OFFSET_AFTER = 48,	/* u32 bits */
	ATTR_REPL_NAT_SEQ_CORRECTION_POS,	/* u32 bits */
	ATTR_REPL_NAT_SEQ_OFFSET_BEFORE,	/* u32 bits */
	ATTR_REPL_NAT_SEQ_OFFSET_AFTER,		/* u32 bits */
	ATTR_SCTP_STATE = 52,			/* u8 bits */
	ATTR_SCTP_VTAG_ORIG,			/* u32 bits */
	ATTR_SCTP_VTAG_REPL,			/* u32 bits */
	ATTR_HELPER_NAME,			/* string (30 bytes max) */
	ATTR_DCCP_STATE = 56,			/* u8 bits */
	ATTR_DCCP_ROLE,				/* u8 bits */
	ATTR_DCCP_HANDSHAKE_SEQ,		/* u64 bits */
	ATTR_TCP_WSCALE_ORIG,			/* u8 bits */
	ATTR_TCP_WSCALE_REPL = 60,		/* u8 bits */
	ATTR_ZONE,				/* u16 bits */
	ATTR_SECCTX,				/* string */
	ATTR_TIMESTAMP_START,			/* u64 bits, linux >= 2.6.38 */
	ATTR_TIMESTAMP_STOP = 64,		/* u64 bits, linux >= 2.6.38 */
	ATTR_HELPER_INFO,			/* variable length */
	ATTR_CONNLABELS,			/* variable length */
	ATTR_CONNLABELS_MASK,			/* variable length */
	ATTR_ORIG_ZONE,				/* u16 bits */
	ATTR_REPL_ZONE,				/* u16 bits */
	ATTR_SNAT_IPV6,				/* u128 bits */
	ATTR_DNAT_IPV6,				/* u128 bits */
	ATTR_SYNPROXY_ISN,			/* u32 bits */
	ATTR_SYNPROXY_ITS,			/* u32 bits */
	ATTR_SYNPROXY_TSOFF,			/* u32 bits */
	ATTR_MAX
};

/* conntrack attribute groups */
enum nf_conntrack_attr_grp {
	ATTR_GRP_ORIG_IPV4 = 0,			/* struct nfct_attr_grp_ipv4 */
	ATTR_GRP_REPL_IPV4,			/* struct nfct_attr_grp_ipv4 */
	ATTR_GRP_ORIG_IPV6,			/* struct nfct_attr_grp_ipv6 */
	ATTR_GRP_REPL_IPV6,			/* struct nfct_attr_grp_ipv6 */
	ATTR_GRP_ORIG_PORT = 4,			/* struct nfct_attr_grp_port */
	ATTR_GRP_REPL_PORT,			/* struct nfct_attr_grp_port */
	ATTR_GRP_ICMP,				/* struct nfct_attr_grp_icmp */
	ATTR_GRP_MASTER_IPV4,			/* struct nfct_attr_grp_ipv4 */
	ATTR_GRP_MASTER_IPV6 = 8,		/* struct nfct_attr_grp_ipv6 */
	ATTR_GRP_MASTER_PORT,			/* struct nfct_attr_grp_port */
	ATTR_GRP_ORIG_COUNTERS,			/* struct nfct_attr_grp_ctrs */
	ATTR_GRP_REPL_COUNTERS,			/* struct nfct_attr_grp_ctrs */
	ATTR_GRP_ORIG_ADDR_SRC = 12,		/* union nfct_attr_grp_addr */
	ATTR_GRP_ORIG_ADDR_DST,			/* union nfct_attr_grp_addr */
	ATTR_GRP_REPL_ADDR_SRC,			/* union nfct_attr_grp_addr */
	ATTR_GRP_REPL_ADDR_DST,			/* union nfct_attr_grp_addr */
	ATTR_GRP_MAX
};

struct nfct_attr_grp_ipv4 {
	uint32_t src, dst;
};

struct nfct_attr_grp_ipv6 {
	uint32_t src[4], dst[4];
};

struct nfct_attr_grp_port {
	uint16_t sport, dport;
};

struct nfct_attr_grp_icmp {
	uint16_t id;
	uint8_t code, type;
};

struct nfct_attr_grp_ctrs {
	uint64_t packets;
	uint64_t bytes;
};

union nfct_attr_grp_addr {
	uint32_t ip;
	uint32_t ip6[4];
	uint32_t addr[4];
};

/* message type */
enum nf_conntrack_msg_type {
	NFCT_T_UNKNOWN = 0,

	NFCT_T_NEW_BIT = 0,
	NFCT_T_NEW = (1 << NFCT_T_NEW_BIT),

	NFCT_T_UPDATE_BIT = 1,
	NFCT_T_UPDATE = (1 << NFCT_T_UPDATE_BIT),

	NFCT_T_DESTROY_BIT = 2,
	NFCT_T_DESTROY = (1 << NFCT_T_DESTROY_BIT),

	NFCT_T_ALL = NFCT_T_NEW | NFCT_T_UPDATE | NFCT_T_DESTROY,

	NFCT_T_ERROR_BIT = 31,
	NFCT_T_ERROR = (1 << NFCT_T_ERROR_BIT),
};

/* constructor / destructor */
extern struct nf_conntrack *nfct_new(void);
extern void nfct_destroy(struct nf_conntrack *ct);

/* clone */
struct nf_conntrack *nfct_clone(const struct nf_conntrack *ct);

/* object size */
extern __attribute__((deprecated)) size_t nfct_sizeof(const struct nf_conntrack *ct);

/* maximum object size */
extern __attribute__((deprecated)) size_t nfct_maxsize(void);

/* set option */
enum {
	NFCT_SOPT_UNDO_SNAT,
	NFCT_SOPT_UNDO_DNAT,
	NFCT_SOPT_UNDO_SPAT,
	NFCT_SOPT_UNDO_DPAT,
	NFCT_SOPT_SETUP_ORIGINAL,
	NFCT_SOPT_SETUP_REPLY,
	__NFCT_SOPT_MAX,
};
#define NFCT_SOPT_MAX (__NFCT_SOPT_MAX - 1)

/* get option */
enum {
	NFCT_GOPT_IS_SNAT,
	NFCT_GOPT_IS_DNAT,
	NFCT_GOPT_IS_SPAT,
	NFCT_GOPT_IS_DPAT,
	__NFCT_GOPT_MAX,
};
#define NFCT_GOPT_MAX (__NFCT_GOPT_MAX - 1)

extern int nfct_setobjopt(struct nf_conntrack *ct, unsigned int option);
extern int nfct_getobjopt(const struct nf_conntrack *ct, unsigned int option);

/* register / unregister callback */

extern int nfct_callback_register(struct nfct_handle *h,
				  enum nf_conntrack_msg_type type,
				  int (*cb)(enum nf_conntrack_msg_type type,
				  	    struct nf_conntrack *ct,
					    void *data),
				  void *data);

extern void nfct_callback_unregister(struct nfct_handle *h);

/* register / unregister callback: extended version including netlink header */

extern int nfct_callback_register2(struct nfct_handle *h,
				   enum nf_conntrack_msg_type type,
				   int (*cb)(const struct nlmsghdr *nlh,
				   	     enum nf_conntrack_msg_type type,
				  	     struct nf_conntrack *ct,
					     void *data),
				   void *data);

extern void nfct_callback_unregister2(struct nfct_handle *h);

/* callback verdict */
enum {
	NFCT_CB_FAILURE = -1,   /* failure */
	NFCT_CB_STOP = 0,       /* stop the query */
	NFCT_CB_CONTINUE = 1,   /* keep iterating through data */
	NFCT_CB_STOLEN = 2,     /* like continue, but ct is not freed */
};

/* bitmask setter/getter */
struct nfct_bitmask;

struct nfct_bitmask *nfct_bitmask_new(unsigned int maxbit);
struct nfct_bitmask *nfct_bitmask_clone(const struct nfct_bitmask *);
unsigned int nfct_bitmask_maxbit(const struct nfct_bitmask *);

void nfct_bitmask_set_bit(struct nfct_bitmask *, unsigned int bit);
int nfct_bitmask_test_bit(const struct nfct_bitmask *, unsigned int bit);
void nfct_bitmask_unset_bit(struct nfct_bitmask *, unsigned int bit);
void nfct_bitmask_destroy(struct nfct_bitmask *);
void nfct_bitmask_clear(struct nfct_bitmask *);
bool nfct_bitmask_equal(const struct nfct_bitmask *, const struct nfct_bitmask *);

/* connlabel name <-> bit translation mapping */
struct nfct_labelmap;

const char *nfct_labels_get_path(void);
struct nfct_labelmap *nfct_labelmap_new(const char *mapfile);
void nfct_labelmap_destroy(struct nfct_labelmap *map);
const char *nfct_labelmap_get_name(struct nfct_labelmap *m, unsigned int bit);
int nfct_labelmap_get_bit(struct nfct_labelmap *m, const char *name);

/* setter */
extern void nfct_set_attr(struct nf_conntrack *ct,
			  const enum nf_conntrack_attr type,
			  const void *value);

extern void nfct_set_attr_u8(struct nf_conntrack *ct,
			     const enum nf_conntrack_attr type,
			     uint8_t value);

extern void nfct_set_attr_u16(struct nf_conntrack *ct,
			      const enum nf_conntrack_attr type,
			      uint16_t value);

extern void nfct_set_attr_u32(struct nf_conntrack *ct,
			      const enum nf_conntrack_attr type,
			      uint32_t value);

extern void nfct_set_attr_u64(struct nf_conntrack *ct,
			      const enum nf_conntrack_attr type,
			      uint64_t value);

extern void nfct_set_attr_l(struct nf_conntrack *ct,
			    const enum nf_conntrack_attr type,
			    const void *value,
			    size_t len);

/* getter */
extern const void *nfct_get_attr(const struct nf_conntrack *ct,
				 const enum nf_conntrack_attr type);

extern uint8_t nfct_get_attr_u8(const struct nf_conntrack *ct,
				 const enum nf_conntrack_attr type);

extern uint16_t nfct_get_attr_u16(const struct nf_conntrack *ct,
				   const enum nf_conntrack_attr type);

extern uint32_t nfct_get_attr_u32(const struct nf_conntrack *ct,
				   const enum nf_conntrack_attr type);

extern uint64_t nfct_get_attr_u64(const struct nf_conntrack *ct,
				   const enum nf_conntrack_attr type);

/* checker */
extern int nfct_attr_is_set(const struct nf_conntrack *ct,
			    const enum nf_conntrack_attr type);

extern int nfct_attr_is_set_array(const struct nf_conntrack *ct,
				  const enum nf_conntrack_attr *type_array,
				  int size);

/* unsetter */
extern int nfct_attr_unset(struct nf_conntrack *ct,
			   const enum nf_conntrack_attr type);

/* group setter */
extern void nfct_set_attr_grp(struct nf_conntrack *ct,
			      const enum nf_conntrack_attr_grp type,
			      const void *value);
/* group getter */
extern int nfct_get_attr_grp(const struct nf_conntrack *ct,
			     const enum nf_conntrack_attr_grp type,
			     void *data);

/* group checker */
extern int nfct_attr_grp_is_set(const struct nf_conntrack *ct,
				const enum nf_conntrack_attr_grp type);

/* unsetter */
extern int nfct_attr_grp_unset(struct nf_conntrack *ct,
			       const enum nf_conntrack_attr_grp type);

/* print */

/* output type */
enum {
	NFCT_O_PLAIN,
	NFCT_O_DEFAULT = NFCT_O_PLAIN,
	NFCT_O_XML,
	NFCT_O_MAX
};

/* output flags */
enum {
	NFCT_OF_SHOW_LAYER3_BIT = 0,
	NFCT_OF_SHOW_LAYER3 = (1 << NFCT_OF_SHOW_LAYER3_BIT),

	NFCT_OF_TIME_BIT = 1,
	NFCT_OF_TIME = (1 << NFCT_OF_TIME_BIT),

	NFCT_OF_ID_BIT = 2,
	NFCT_OF_ID = (1 << NFCT_OF_ID_BIT),

	NFCT_OF_TIMESTAMP_BIT = 3,
	NFCT_OF_TIMESTAMP = (1 << NFCT_OF_TIMESTAMP_BIT),
};

extern int nfct_snprintf(char *buf, 
			 unsigned int size,
			 const struct nf_conntrack *ct,
			 const unsigned int msg_type,
			 const unsigned int out_type,
			 const unsigned int out_flags);

extern int nfct_snprintf_labels(char *buf,
				unsigned int size,
				const struct nf_conntrack *ct,
				const unsigned int msg_type,
				const unsigned int out_type,
				const unsigned int out_flags,
				struct nfct_labelmap *map);

/* comparison */
extern int nfct_compare(const struct nf_conntrack *ct1,
			const struct nf_conntrack *ct2);

enum {
	NFCT_CMP_ALL = 0,
	NFCT_CMP_ORIG = (1 << 0),
	NFCT_CMP_REPL = (1 << 1),
	NFCT_CMP_TIMEOUT_EQ = (1 << 2),
	NFCT_CMP_TIMEOUT_GT = (1 << 3),
	NFCT_CMP_TIMEOUT_GE = (NFCT_CMP_TIMEOUT_EQ | NFCT_CMP_TIMEOUT_GT),
	NFCT_CMP_TIMEOUT_LT = (1 << 4),
	NFCT_CMP_TIMEOUT_LE = (NFCT_CMP_TIMEOUT_EQ | NFCT_CMP_TIMEOUT_LT),
	NFCT_CMP_MASK = (1 << 5),
	NFCT_CMP_STRICT = (1 << 6),
};

extern int nfct_cmp(const struct nf_conntrack *ct1,
		    const struct nf_conntrack *ct2,
		    unsigned int flags);


/* query */
enum nf_conntrack_query {
	NFCT_Q_CREATE,
	NFCT_Q_UPDATE,
	NFCT_Q_DESTROY,
	NFCT_Q_GET,
	NFCT_Q_FLUSH,
	NFCT_Q_DUMP,
	NFCT_Q_DUMP_RESET,
	NFCT_Q_CREATE_UPDATE,
	NFCT_Q_DUMP_FILTER,
	NFCT_Q_DUMP_FILTER_RESET,
};

extern int nfct_query(struct nfct_handle *h,
		      const enum nf_conntrack_query query,
		      const void *data);

extern int nfct_send(struct nfct_handle *h,
		     const enum nf_conntrack_query query,
		     const void *data);

extern int nfct_catch(struct nfct_handle *h);

/* copy */
enum {
	NFCT_CP_ALL = 0,
	NFCT_CP_ORIG = (1 << 0),
	NFCT_CP_REPL = (1 << 1),
	NFCT_CP_META = (1 << 2),
	NFCT_CP_OVERRIDE = (1 << 3),
};

extern void nfct_copy(struct nf_conntrack *dest,
		      const struct nf_conntrack *source,
		      unsigned int flags);

extern void nfct_copy_attr(struct nf_conntrack *ct1,
			   const struct nf_conntrack *ct2,
			   const enum nf_conntrack_attr type);

/* event filtering */

struct nfct_filter;

extern struct nfct_filter *nfct_filter_create(void);
extern void nfct_filter_destroy(struct nfct_filter *filter);

struct nfct_filter_proto {
	uint16_t proto;
	uint16_t state;
};
struct nfct_filter_ipv4 {
	uint32_t addr;
	uint32_t mask;
};
struct nfct_filter_ipv6 {
	uint32_t addr[4];
	uint32_t mask[4];
};

enum nfct_filter_attr {
	NFCT_FILTER_L4PROTO = 0,	/* uint32_t */
	NFCT_FILTER_L4PROTO_STATE,	/* struct nfct_filter_proto */
	NFCT_FILTER_SRC_IPV4,		/* struct nfct_filter_ipv4 */
	NFCT_FILTER_DST_IPV4,		/* struct nfct_filter_ipv4 */
	NFCT_FILTER_SRC_IPV6,		/* struct nfct_filter_ipv6 */
	NFCT_FILTER_DST_IPV6,		/* struct nfct_filter_ipv6 */
	NFCT_FILTER_MARK,		/* struct nfct_filter_dump_mark */
	NFCT_FILTER_MAX
};

extern void nfct_filter_add_attr(struct nfct_filter *filter,
				 const enum nfct_filter_attr attr,
				 const void *value);

extern void nfct_filter_add_attr_u32(struct nfct_filter *filter,
				     const enum nfct_filter_attr attr,
				     const uint32_t value);

enum nfct_filter_logic {
	NFCT_FILTER_LOGIC_POSITIVE,
	NFCT_FILTER_LOGIC_NEGATIVE,
	NFCT_FILTER_LOGIC_MAX
};

extern int nfct_filter_set_logic(struct nfct_filter *filter,
				 const enum nfct_filter_attr attr,
				 const enum nfct_filter_logic logic);

extern int nfct_filter_attach(int fd, struct nfct_filter *filter);
extern int nfct_filter_detach(int fd);

/* dump filtering */

struct nfct_filter_dump;

struct nfct_filter_dump_mark {
	uint32_t val;
	uint32_t mask;
};

enum nfct_filter_dump_attr {
	NFCT_FILTER_DUMP_MARK = 0,	/* struct nfct_filter_dump_mark */
	NFCT_FILTER_DUMP_L3NUM,		/* uint8_t */
	NFCT_FILTER_DUMP_MAX
};

struct nfct_filter_dump *nfct_filter_dump_create(void);

void nfct_filter_dump_destroy(struct nfct_filter_dump *filter);

void nfct_filter_dump_set_attr(struct nfct_filter_dump *filter_dump,
			       const enum nfct_filter_dump_attr type,
			       const void *data);

void nfct_filter_dump_set_attr_u8(struct nfct_filter_dump *filter_dump,
				  const enum nfct_filter_dump_attr type,
				  uint8_t data);

/* low level API: netlink functions */

extern __attribute__((deprecated)) int
nfct_build_conntrack(struct nfnl_subsys_handle *ssh,
				void *req,
				size_t size,
				uint16_t type,
				uint16_t flags,
				const struct nf_conntrack *ct);

extern __attribute__((deprecated))
int nfct_parse_conntrack(enum nf_conntrack_msg_type msg,
				const struct nlmsghdr *nlh, 
				struct nf_conntrack *ct);

extern __attribute__((deprecated))
int nfct_build_query(struct nfnl_subsys_handle *ssh,
			    const enum nf_conntrack_query query,
			    const void *data,
			    void *req,
			    unsigned int size);

/* New low level API: netlink functions */

extern int nfct_nlmsg_build(struct nlmsghdr *nlh, const struct nf_conntrack *ct);
extern int nfct_nlmsg_parse(const struct nlmsghdr *nlh, struct nf_conntrack *ct);
extern int nfct_payload_parse(const void *payload, size_t payload_len, uint16_t l3num, struct nf_conntrack *ct);

/*
 * NEW expectation API
 */

/* expectation object */
struct nf_expect;

/* expect attributes */
enum nf_expect_attr {
	ATTR_EXP_MASTER = 0,	/* pointer to conntrack object */
	ATTR_EXP_EXPECTED,	/* pointer to conntrack object */
	ATTR_EXP_MASK,		/* pointer to conntrack object */
	ATTR_EXP_TIMEOUT,	/* u32 bits */
	ATTR_EXP_ZONE,		/* u16 bits */
	ATTR_EXP_FLAGS,		/* u32 bits */
	ATTR_EXP_HELPER_NAME,	/* string (16 bytes max) */
	ATTR_EXP_CLASS,		/* u32 bits */
	ATTR_EXP_NAT_TUPLE,	/* pointer to conntrack object */
	ATTR_EXP_NAT_DIR,	/* u8 bits */
	ATTR_EXP_FN,		/* string */
	ATTR_EXP_MAX
};

/* constructor / destructor */
extern struct nf_expect *nfexp_new(void);
extern void nfexp_destroy(struct nf_expect *exp);

/* clone */
extern struct nf_expect *nfexp_clone(const struct nf_expect *exp);

/* object size */
extern size_t nfexp_sizeof(const struct nf_expect *exp);

/* maximum object size */
extern size_t nfexp_maxsize(void);

/* register / unregister callback */

extern int nfexp_callback_register(struct nfct_handle *h,
				   enum nf_conntrack_msg_type type,
				   int (*cb)(enum nf_conntrack_msg_type type,
				  	     struct nf_expect *exp,
					     void *data),
				   void *data);

extern void nfexp_callback_unregister(struct nfct_handle *h);

/* register / unregister callback: extended version including netlink header */
extern int nfexp_callback_register2(struct nfct_handle *h,
				    enum nf_conntrack_msg_type type,
				    int (*cb)(const struct nlmsghdr *nlh,
				    	      enum nf_conntrack_msg_type type,
					      struct nf_expect *exp,
					      void *data),
				    void *data);

extern void nfexp_callback_unregister2(struct nfct_handle *h);

/* setter */
extern void nfexp_set_attr(struct nf_expect *exp,
			   const enum nf_expect_attr type,
			   const void *value);

extern void nfexp_set_attr_u8(struct nf_expect *exp,
			      const enum nf_expect_attr type,
			      uint8_t value);

extern void nfexp_set_attr_u16(struct nf_expect *exp,
			       const enum nf_expect_attr type,
			       uint16_t value);

extern void nfexp_set_attr_u32(struct nf_expect *exp,
			       const enum nf_expect_attr type,
			       uint32_t value);

/* getter */
extern const void *nfexp_get_attr(const struct nf_expect *exp,
				  const enum nf_expect_attr type);

extern uint8_t nfexp_get_attr_u8(const struct nf_expect *exp,
				  const enum nf_expect_attr type);

extern uint16_t nfexp_get_attr_u16(const struct nf_expect *exp,
				    const enum nf_expect_attr type);

extern uint32_t nfexp_get_attr_u32(const struct nf_expect *exp,
				    const enum nf_expect_attr type);

/* checker */
extern int nfexp_attr_is_set(const struct nf_expect *exp,
			     const enum nf_expect_attr type);

/* unsetter */
extern int nfexp_attr_unset(struct nf_expect *exp,
			    const enum nf_expect_attr type);

/* query */
extern int nfexp_query(struct nfct_handle *h,
		       const enum nf_conntrack_query qt,
		       const void *data);

/* print */
extern int nfexp_snprintf(char *buf, 
			  unsigned int size,
			  const struct nf_expect *exp,
			  const unsigned int msg_type,
			  const unsigned int out_type,
			  const unsigned int out_flags);

/* compare */
extern int nfexp_cmp(const struct nf_expect *exp1,
		     const struct nf_expect *exp2,
		     unsigned int flags);

extern int nfexp_send(struct nfct_handle *h,
		      const enum nf_conntrack_query qt,
		      const void *data);

extern int nfexp_catch(struct nfct_handle *h);

/* low level API */
extern __attribute__((deprecated))
int nfexp_build_expect(struct nfnl_subsys_handle *ssh,
			      void *req,
			      size_t size,
			      uint16_t type,
			      uint16_t flags,
			      const struct nf_expect *exp);

extern __attribute__((deprecated))
int nfexp_parse_expect(enum nf_conntrack_msg_type type,
			      const struct nlmsghdr *nlh,
			      struct nf_expect *exp);

extern __attribute__((deprecated))
int nfexp_build_query(struct nfnl_subsys_handle *ssh,
			     const enum nf_conntrack_query qt,
			     const void *data,
			     void *buffer,
			     unsigned int size);

/* New low level API: netlink functions */

extern int nfexp_nlmsg_build(struct nlmsghdr *nlh, const struct nf_expect *exp);
extern int nfexp_nlmsg_parse(const struct nlmsghdr *nlh, struct nf_expect *exp);

/*
 * TCP flags
 */

/* Window scaling is advertised by the sender */
#define IP_CT_TCP_FLAG_WINDOW_SCALE             0x01

/* SACK is permitted by the sender */
#define IP_CT_TCP_FLAG_SACK_PERM                0x02

/* This sender sent FIN first */
#define IP_CT_TCP_FLAG_CLOSE_INIT               0x04

/* Be liberal in window checking */
#define IP_CT_TCP_FLAG_BE_LIBERAL               0x08

/* WARNING: do not use these constants in new applications, we keep them here
 * to avoid breaking backward compatibility. */
#define NFCT_DIR_ORIGINAL 0
#define NFCT_DIR_REPLY 1
#define NFCT_DIR_MAX NFCT_DIR_REPLY+1

/* xt_helper uses a length size of 30 bytes, however, no helper name in
 * the tree has exceeded 16 bytes length. Since 2.6.29, the maximum
 * length accepted is 16 bytes, this limit is enforced during module load. */
#define NFCT_HELPER_NAME_MAX	16

#ifdef __cplusplus
}
#endif

#endif	/* _LIBNETFILTER_CONNTRACK_H_ */
