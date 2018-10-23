/*
 * (C) 2006-2012 by Pablo Neira Ayuso <pablo@netfilter.org>
 * (C) 2011-2012 by Vyatta Inc <http://www.vyatta.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "filter.h"
#include "bitops.h"
#include "jhash.h"
#include "hash.h"
#include "vector.h"
#include "conntrackd.h"
#include "log.h"

#include <libnetfilter_conntrack/libnetfilter_conntrack.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

struct ct_filter {
	int logic[CT_FILTER_MAX];
	uint32_t l4protomap[IPPROTO_MAX/32];
	uint16_t statemap[IPPROTO_MAX];
	struct hashtable *h;
	struct hashtable *h6;
	struct vector *v;
	struct vector *v6;
};

/* XXX: These should be configurable, better use a rb-tree */
#define FILTER_POOL_SIZE 128
#define FILTER_POOL_LIMIT INT_MAX

static uint32_t ct_filter_hash(const void *data, const struct hashtable *table)
{
	const uint32_t *f = data;

	return jhash_1word(*f, 0) % table->hashsize;
}

static uint32_t ct_filter_hash6(const void *data, const struct hashtable *table)
{
	return jhash2(data, 4, 0) % table->hashsize;
}

static int ct_filter_compare(const void *data1, const void *data2)
{
	const struct ct_filter_ipv4_hnode *f1 = data1;
	const uint32_t *f2 = data2;

	return f1->ip == *f2;
}

static int ct_filter_compare6(const void *data1, const void *data2)
{
	const struct ct_filter_ipv6_hnode *f = data1;

	return memcmp(f->ipv6, data2, sizeof(uint32_t)*4) == 0;
}

struct ct_filter *ct_filter_create(void)
{
	int i;
	struct ct_filter *filter;

	filter = calloc(sizeof(struct ct_filter), 1);
	if (!filter)
		return NULL;

	filter->h = hashtable_create(FILTER_POOL_SIZE,
				     FILTER_POOL_LIMIT,
				     ct_filter_hash,
				     ct_filter_compare);
	if (!filter->h) {
		free(filter);
		return NULL;
	}

	filter->h6 = hashtable_create(FILTER_POOL_SIZE,
				      FILTER_POOL_LIMIT,
				      ct_filter_hash6,
				      ct_filter_compare6);
	if (!filter->h6) {
		free(filter->h);
		free(filter);
		return NULL;
	}

	filter->v = vector_create(sizeof(struct ct_filter_netmask_ipv4));
	if (!filter->v) {
		free(filter->h6);
		free(filter->h);
		free(filter);
		return NULL;
	}

	filter->v6 = vector_create(sizeof(struct ct_filter_netmask_ipv6));
	if (!filter->v6) {
		free(filter->v);
		free(filter->h6);
		free(filter->h);
		free(filter);
		return NULL;
	}

	for (i=0; i<CT_FILTER_MAX; i++)
		filter->logic[i] = -1;

	return filter;
}

void ct_filter_destroy(struct ct_filter *filter)
{
	hashtable_destroy(filter->h);
	hashtable_destroy(filter->h6);
	vector_destroy(filter->v);
	vector_destroy(filter->v6);
	free(filter);
}

/* this is ugly, but it simplifies read_config_yy.y */
static struct ct_filter *__filter_alloc(struct ct_filter *filter)
{
	if (!STATE(us_filter)) {
		STATE(us_filter) = ct_filter_create();
		if (!STATE(us_filter)) {
			dlog(LOG_ERR, "Can't create ignore pool!");
			exit(EXIT_FAILURE);
		}
	}

	return STATE(us_filter);
}

void ct_filter_set_logic(struct ct_filter *filter,
			 enum ct_filter_type type,
			 enum ct_filter_logic logic)
{
	filter = __filter_alloc(filter);
	filter->logic[type] = logic;
}

int ct_filter_add_ip(struct ct_filter *filter, void *data, uint8_t family)
{
	int id;
	filter = __filter_alloc(filter);

	switch(family) {
		case AF_INET:
			id = hashtable_hash(filter->h, data);
			if (!hashtable_find(filter->h, data, id)) {
				struct ct_filter_ipv4_hnode *n;
				n = malloc(sizeof(struct ct_filter_ipv4_hnode));
				if (n == NULL)
					return 0;
				memcpy(&n->ip, data, sizeof(uint32_t));
				hashtable_add(filter->h, &n->node, id);
				return 0;
			}
			break;
		case AF_INET6:
			id = hashtable_hash(filter->h6, data);
			if (!hashtable_find(filter->h6, data, id)) {
				struct ct_filter_ipv6_hnode *n;
				n = malloc(sizeof(struct ct_filter_ipv6_hnode));
				if (n == NULL)
					return 0;
				memcpy(n->ipv6, data, sizeof(uint32_t)*4);
				hashtable_add(filter->h6, &n->node, id);
				return 0;
			}
			break;
	}
	return 1;
}

static int cmp_ipv4_addr(const void *a, const void *b)
{
	return memcmp(a, b, sizeof(struct ct_filter_netmask_ipv4)) == 0;
}

static int cmp_ipv6_addr(const void *a, const void *b)
{
	return memcmp(a, b, sizeof(struct ct_filter_netmask_ipv6)) == 0;
}

int ct_filter_add_netmask(struct ct_filter *filter, void *data, uint8_t family)
{
	filter = __filter_alloc(filter);

	switch(family) {
		case AF_INET:
			if (vector_iterate(filter->v, data, cmp_ipv4_addr)) {
				errno = EEXIST;
				return 0;
			}
			vector_add(filter->v, data);
			break;
		case AF_INET6:
			if (vector_iterate(filter->v, data, cmp_ipv6_addr)) {
				errno = EEXIST;
				return 0;
			}
			vector_add(filter->v6, data);
			break;
	}
	return 1;
}

void ct_filter_add_proto(struct ct_filter *f, int protonum)
{
	f = __filter_alloc(f);

	set_bit_u32(protonum, f->l4protomap);
}

void ct_filter_add_state(struct ct_filter *f, int protonum, int val)
{
	f = __filter_alloc(f);

	set_bit_u16(val, &f->statemap[protonum]);
}

static inline int
__ct_filter_test_ipv4(struct ct_filter *f, const struct nf_conntrack *ct)
{
	int id_src, id_dst;
	uint32_t src, dst;

	/* we only use the real source and destination address */
	src = nfct_get_attr_u32(ct, ATTR_ORIG_IPV4_SRC);
	dst = nfct_get_attr_u32(ct, ATTR_REPL_IPV4_SRC);

	id_src = hashtable_hash(f->h, &src);
	id_dst = hashtable_hash(f->h, &dst);

	return hashtable_find(f->h, &src, id_src) ||
	       hashtable_find(f->h, &dst, id_dst);
}

static inline int
__ct_filter_test_ipv6(struct ct_filter *f, const struct nf_conntrack *ct)
{
	int id_src, id_dst;
	const uint32_t *src, *dst;

	src = nfct_get_attr(ct, ATTR_ORIG_IPV6_SRC);
	dst = nfct_get_attr(ct, ATTR_REPL_IPV6_SRC);

	id_src = hashtable_hash(f->h6, src);
	id_dst = hashtable_hash(f->h6, dst);

	return hashtable_find(f->h6, src, id_src) ||
	       hashtable_find(f->h6, dst, id_dst);
}

static int
__ct_filter_test_mask4(const void *ptr, const void *ct)
{
	const struct ct_filter_netmask_ipv4 *elem = ptr;
	const uint32_t src = nfct_get_attr_u32(ct, ATTR_ORIG_IPV4_SRC);
	const uint32_t dst = nfct_get_attr_u32(ct, ATTR_REPL_IPV4_SRC);

	return ((elem->ip & elem->mask) == (src & elem->mask) || 
		(elem->ip & elem->mask) == (dst & elem->mask));
}

static int
__ct_filter_test_mask6(const void *ptr, const void *ct)
{
	const struct ct_filter_netmask_ipv6 *elem = ptr;
	const uint32_t *src = nfct_get_attr(ct, ATTR_ORIG_IPV6_SRC);
	const uint32_t *dst = nfct_get_attr(ct, ATTR_REPL_IPV6_SRC);

	return (((elem->ip[0] & elem->mask[0]) == (src[0] & elem->mask[0]) &&
		 (elem->ip[1] & elem->mask[1]) == (src[1] & elem->mask[1]) &&
		 (elem->ip[2] & elem->mask[2]) == (src[2] & elem->mask[2]) &&
		 (elem->ip[3] & elem->mask[3]) == (src[3] & elem->mask[3])) ||
		((elem->ip[0] & elem->mask[0]) == (dst[0] & elem->mask[0]) &&
		 (elem->ip[1] & elem->mask[1]) == (dst[1] & elem->mask[1]) &&
		 (elem->ip[2] & elem->mask[2]) == (dst[2] & elem->mask[2]) &&
		 (elem->ip[3] & elem->mask[3]) == (dst[3] & elem->mask[3])));
}

static int
__ct_filter_test_state(struct ct_filter *f, const struct nf_conntrack *ct)
{
	uint16_t val = 0;
	uint8_t protonum = nfct_get_attr_u8(ct, ATTR_L4PROTO);

	switch(protonum) {
	case IPPROTO_TCP:
		if (!nfct_attr_is_set(ct, ATTR_TCP_STATE))
			return -1;

		val = nfct_get_attr_u8(ct, ATTR_TCP_STATE);
		break;
	default:
		return -1;
	}

	return test_bit_u16(val, &f->statemap[protonum]);
}

static int
ct_filter_check(struct ct_filter *f, const struct nf_conntrack *ct)
{
	int ret, protonum = nfct_get_attr_u8(ct, ATTR_L4PROTO);

	/* no event filtering at all */
	if (f == NULL)
		return 1;

	if (f->logic[CT_FILTER_L4PROTO] != -1) {
		ret = test_bit_u32(protonum, f->l4protomap);
		if (ret ^ f->logic[CT_FILTER_L4PROTO])
			return 0;
	}

	if (f->logic[CT_FILTER_ADDRESS] != -1) {
		switch(nfct_get_attr_u8(ct, ATTR_L3PROTO)) {
		case AF_INET:
			ret = vector_iterate(f->v, ct, __ct_filter_test_mask4);
			if (ret ^ f->logic[CT_FILTER_ADDRESS])
				return 0;
			ret = __ct_filter_test_ipv4(f, ct);
			if (ret ^ f->logic[CT_FILTER_ADDRESS])
				return 0;
			break;
		case AF_INET6:
			ret = vector_iterate(f->v6, ct, __ct_filter_test_mask6);
			if (ret ^ f->logic[CT_FILTER_ADDRESS])
				return 0;
			ret = __ct_filter_test_ipv6(f, ct);
			if (ret ^ f->logic[CT_FILTER_ADDRESS])
				return 0;
			break;
		default:
			break;
		}
	}

	if (f->logic[CT_FILTER_STATE] != -1) {
		ret = __ct_filter_test_state(f, ct);
		/* ret is -1 if we don't know what to do */
		if (ret != -1 && ret ^ f->logic[CT_FILTER_STATE])
			return 0;
	}

	return 1;
}

static inline int ct_filter_sanity_check(const struct nf_conntrack *ct)
{
	if (!nfct_attr_is_set(ct, ATTR_L3PROTO)) {
		dlog(LOG_ERR, "missing layer 3 protocol");
		return 0;
	}

	switch(nfct_get_attr_u8(ct, ATTR_L3PROTO)) {
	case AF_INET:
		if (!nfct_attr_is_set(ct, ATTR_ORIG_IPV4_SRC) ||
		    !nfct_attr_is_set(ct, ATTR_REPL_IPV4_SRC)) {
		    	dlog(LOG_ERR, "missing IPv4 address. "
				      "You forgot to load "
				      "nf_conntrack_ipv4?");
			return 0;
		}
		break;
	case AF_INET6:
		if (!nfct_attr_is_set(ct, ATTR_ORIG_IPV6_SRC) ||
		    !nfct_attr_is_set(ct, ATTR_REPL_IPV6_SRC)) {
		    	dlog(LOG_ERR, "missing IPv6 address. "
				      "You forgot to load "
				      "nf_conntrack_ipv6?");
			return 0;
		}
		break;
	}
	return 1;
}

/* we do user-space filtering for dump and resyncs */
int ct_filter_conntrack(const struct nf_conntrack *ct, int userspace)
{
	/* missing mandatory attributes in object */
	if (!ct_filter_sanity_check(ct))
		return 1;

	if (userspace && !ct_filter_check(STATE(us_filter), ct))
		return 1;

	return 0;
}

static inline int
ct_filter_master_sanity_check(const struct nf_conntrack *master)
{
	if (master == NULL) {
		dlog(LOG_ERR, "no master tuple in expectation");
		return 0;
	}

	if (!nfct_attr_is_set(master, ATTR_L3PROTO)) {
		dlog(LOG_ERR, "missing layer 3 protocol");
		return 0;
	}

	switch (nfct_get_attr_u8(master, ATTR_L3PROTO)) {
	case AF_INET:
		if (!nfct_attr_is_set(master, ATTR_IPV4_SRC) ||
		    !nfct_attr_is_set(master, ATTR_IPV4_DST)) {
		    	dlog(LOG_ERR, "missing IPv4 address. "
			     "You forgot to load nf_conntrack_ipv4?");
			return 0;
		}
		break;
	case AF_INET6:
		if (!nfct_attr_is_set(master, ATTR_IPV6_SRC) ||
		    !nfct_attr_is_set(master, ATTR_IPV6_DST)) {
		    	dlog(LOG_ERR, "missing IPv6 address. "
			     "You forgot to load nf_conntrack_ipv6?");
			return 0;
		}
		break;
	}
	return 1;
}

int ct_filter_master(const struct nf_conntrack *master)
{
	if (!ct_filter_master_sanity_check(master))
		return 1;

	/* Check if we've got a master conntrack for this expectation in our
	 * caches. If there is not, we don't want this expectation either.
	 */
	return STATE(mode)->internal->exp.find(master) ? 0 : 1;
}

struct exp_filter {
	struct list_head 	list;
};

struct exp_filter *exp_filter_create(void)
{
	struct exp_filter *f;

	f = calloc(1, sizeof(struct exp_filter));
	if (f == NULL)
		return NULL;

	INIT_LIST_HEAD(&f->list);
	return f;
}

struct exp_filter_item {
	struct list_head	head;
	char			helper_name[NFCT_HELPER_NAME_MAX];
};

/* this is ugly, but it simplifies read_config_yy.y */
static struct exp_filter *exp_filter_alloc(void)
{
	if (STATE(exp_filter) == NULL) {
		STATE(exp_filter) = exp_filter_create();
		if (STATE(exp_filter) == NULL) {
			dlog(LOG_ERR, "Can't init expectation filtering!");
			return NULL;
		}
	}
	return STATE(exp_filter);;
}

int exp_filter_add(struct exp_filter *f, const char *helper_name)
{
	struct exp_filter_item *item;

	f = exp_filter_alloc();
	if (f == NULL)
		return -1;

	list_for_each_entry(item, &f->list, head) {
		if (strncasecmp(item->helper_name, helper_name,
				NFCT_HELPER_NAME_MAX) == 0) {
			return -1;
		}
	}
	item = calloc(1, sizeof(struct exp_filter_item));
	if (item == NULL)
		return -1;

	strncpy(item->helper_name, helper_name, NFCT_HELPER_NAME_MAX);
	list_add(&item->head, &f->list);
	return 0;
}

int exp_filter_find(struct exp_filter *f, const struct nf_expect *exp)
{
	struct exp_filter_item *item;

	/* if filtering is not active, accept everything. */
	if (f == NULL)
		return 1;

	list_for_each_entry(item, &f->list, head) {
		const char *name;

		if (nfexp_attr_is_set(exp, ATTR_EXP_HELPER_NAME))
			name = nfexp_get_attr(exp, ATTR_EXP_HELPER_NAME);
		else {
			/* No helper name, this is likely to be a kernel older
			 * which does not include the helper name, just skip
			 * this so we don't crash.
			 */
			return 0;
		}

		/* we allow partial matching to support things like sip-PORT. */
		if (strncasecmp(item->helper_name, name,
				strlen(item->helper_name)) == 0) {
			return 1;
		}
	}
	return 0;
}
