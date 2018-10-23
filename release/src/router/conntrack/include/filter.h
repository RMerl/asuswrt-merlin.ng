#ifndef _FILTER_H_
#define _FILTER_H_

#include <stdint.h>
#include <string.h>
#include <netinet/in.h>
#include <hash.h>

enum ct_filter_type {
	CT_FILTER_L4PROTO,
	CT_FILTER_STATE,
	CT_FILTER_ADDRESS,	/* also for netmask */
	CT_FILTER_MAX
};

enum ct_filter_logic {
	CT_FILTER_NEGATIVE = 0,
	CT_FILTER_POSITIVE = 1,
};

struct ct_filter_ipv4_hnode {
	struct hashtable_node node;
	uint32_t ip;
};

struct ct_filter_ipv6_hnode {
	struct hashtable_node node;
	uint32_t ipv6[4];
};

struct ct_filter_netmask_ipv4 {
	uint32_t ip;
	uint32_t mask;
};

struct ct_filter_netmask_ipv6 {
	uint32_t ip[4];
	uint32_t mask[4];
};

struct nf_conntrack;
struct ct_filter;

struct ct_filter *ct_filter_create(void);
void ct_filter_destroy(struct ct_filter *filter);
int ct_filter_add_ip(struct ct_filter *filter, void *data, uint8_t family);
int ct_filter_add_netmask(struct ct_filter *filter, void *data, uint8_t family);
void ct_filter_add_proto(struct ct_filter *filter, int protonum);
void ct_filter_add_state(struct ct_filter *f, int protonum, int state);
void ct_filter_set_logic(struct ct_filter *f,
			 enum ct_filter_type type,
			 enum ct_filter_logic logic);
int ct_filter_conntrack(const struct nf_conntrack *ct, int userspace);
int ct_filter_master(const struct nf_conntrack *master);

struct exp_filter;
struct nf_expect;

struct exp_filter *exp_filter_create(void);
int exp_filter_add(struct exp_filter *f, const char *helper_name);
int exp_filter_find(struct exp_filter *f, const struct nf_expect *exp);

#endif
