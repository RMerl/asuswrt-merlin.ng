/*
 * ll_map.c
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <net/if.h>

#include "libnetlink.h"
#include "ll_map.h"
#include "list.h"
#include "utils.h"

struct ll_cache {
	struct hlist_node idx_hash;
	struct hlist_node name_hash;
	unsigned	flags;
	unsigned 	index;
	unsigned short	type;
	struct list_head altnames_list;
	char		name[];
};

#define IDXMAP_SIZE	1024
static struct hlist_head idx_head[IDXMAP_SIZE];
static struct hlist_head name_head[IDXMAP_SIZE];

static struct ll_cache *ll_get_by_index(unsigned index)
{
	struct hlist_node *n;
	unsigned h = index & (IDXMAP_SIZE - 1);

	hlist_for_each(n, &idx_head[h]) {
		struct ll_cache *im
			= container_of(n, struct ll_cache, idx_hash);
		if (im->index == index)
			return im;
	}

	return NULL;
}

unsigned namehash(const char *str)
{
	unsigned hash = 5381;

	while (*str)
		hash = ((hash << 5) + hash) + *str++; /* hash * 33 + c */

	return hash;
}

static struct ll_cache *ll_get_by_name(const char *name)
{
	struct hlist_node *n;
	unsigned h = namehash(name) & (IDXMAP_SIZE - 1);

	hlist_for_each(n, &name_head[h]) {
		struct ll_cache *im
			= container_of(n, struct ll_cache, name_hash);

		if (strcmp(im->name, name) == 0)
			return im;
	}

	return NULL;
}

static struct ll_cache *ll_entry_create(struct ifinfomsg *ifi,
					const char *ifname,
					struct ll_cache *parent_im)
{
	struct ll_cache *im;
	unsigned int h;

	im = malloc(sizeof(*im) + strlen(ifname) + 1);
	if (!im)
		return NULL;
	im->index = ifi->ifi_index;
	strcpy(im->name, ifname);
	im->type = ifi->ifi_type;
	im->flags = ifi->ifi_flags;

	if (parent_im) {
		list_add_tail(&im->altnames_list, &parent_im->altnames_list);
	} else {
		/* This is parent, insert to index hash. */
		h = ifi->ifi_index & (IDXMAP_SIZE - 1);
		hlist_add_head(&im->idx_hash, &idx_head[h]);
		INIT_LIST_HEAD(&im->altnames_list);
	}

	h = namehash(ifname) & (IDXMAP_SIZE - 1);
	hlist_add_head(&im->name_hash, &name_head[h]);
	return im;
}

static void ll_entry_destroy(struct ll_cache *im, bool im_is_parent)
{
	hlist_del(&im->name_hash);
	if (im_is_parent)
		hlist_del(&im->idx_hash);
	else
		list_del(&im->altnames_list);
	free(im);
}

static void ll_entry_update(struct ll_cache *im, struct ifinfomsg *ifi,
			    const char *ifname)
{
	unsigned int h;

	im->flags = ifi->ifi_flags;
	if (!strcmp(im->name, ifname))
		return;
	hlist_del(&im->name_hash);
	h = namehash(ifname) & (IDXMAP_SIZE - 1);
	hlist_add_head(&im->name_hash, &name_head[h]);
}

static void ll_altname_entries_create(struct ll_cache *parent_im,
				      struct ifinfomsg *ifi, struct rtattr **tb)
{
	struct rtattr *i, *proplist = tb[IFLA_PROP_LIST];
	int rem;

	if (!proplist)
		return;
	rem = RTA_PAYLOAD(proplist);
	for (i = RTA_DATA(proplist); RTA_OK(i, rem);
	     i = RTA_NEXT(i, rem)) {
		if (i->rta_type != IFLA_ALT_IFNAME)
			continue;
		ll_entry_create(ifi, rta_getattr_str(i), parent_im);
	}
}

static void ll_altname_entries_destroy(struct ll_cache *parent_im)
{
	struct ll_cache *im, *tmp;

	list_for_each_entry_safe(im, tmp, &parent_im->altnames_list,
				 altnames_list)
		ll_entry_destroy(im, false);
}

static void ll_altname_entries_update(struct ll_cache *parent_im,
				      struct ifinfomsg *ifi, struct rtattr **tb)
{
	struct rtattr *i, *proplist = tb[IFLA_PROP_LIST];
	struct ll_cache *im;
	int rem;

	if (!proplist) {
		ll_altname_entries_destroy(parent_im);
		return;
	}

	/* Simply compare the altname list with the cached one
	 * and if it does not fit 1:1, recreate the cached list
	 * from scratch.
	 */
	im = list_first_entry(&parent_im->altnames_list, typeof(*im),
			      altnames_list);
	rem = RTA_PAYLOAD(proplist);
	for (i = RTA_DATA(proplist); RTA_OK(i, rem);
	     i = RTA_NEXT(i, rem)) {
		if (i->rta_type != IFLA_ALT_IFNAME)
			continue;
		if (!im || strcmp(rta_getattr_str(i), im->name))
			goto recreate_altname_entries;
		im = list_next_entry(im, altnames_list);
	}
	if (list_next_entry(im, altnames_list))
		goto recreate_altname_entries;
	return;

recreate_altname_entries:
	ll_altname_entries_destroy(parent_im);
	ll_altname_entries_create(parent_im, ifi, tb);
}

static void ll_entries_create(struct ifinfomsg *ifi, struct rtattr **tb)
{
	struct ll_cache *parent_im;

	if (!tb[IFLA_IFNAME])
		return;
	parent_im = ll_entry_create(ifi, rta_getattr_str(tb[IFLA_IFNAME]),
				    NULL);
	if (!parent_im)
		return;
	ll_altname_entries_create(parent_im, ifi, tb);
}

static void ll_entries_destroy(struct ll_cache *parent_im)
{
	ll_altname_entries_destroy(parent_im);
	ll_entry_destroy(parent_im, true);
}

static void ll_entries_update(struct ll_cache *parent_im,
			      struct ifinfomsg *ifi, struct rtattr **tb)
{
	if (tb[IFLA_IFNAME])
		ll_entry_update(parent_im, ifi,
				rta_getattr_str(tb[IFLA_IFNAME]));
	ll_altname_entries_update(parent_im, ifi, tb);
}

int ll_remember_index(struct nlmsghdr *n, void *arg)
{
	struct ifinfomsg *ifi = NLMSG_DATA(n);
	struct ll_cache *im;
	struct rtattr *tb[IFLA_MAX+1];

	if (n->nlmsg_type != RTM_NEWLINK && n->nlmsg_type != RTM_DELLINK)
		return 0;

	if (n->nlmsg_len < NLMSG_LENGTH(sizeof(*ifi)))
		return -1;

	im = ll_get_by_index(ifi->ifi_index);
	if (n->nlmsg_type == RTM_DELLINK) {
		if (im)
			ll_entries_destroy(im);
		return 0;
	}

	parse_rtattr_flags(tb, IFLA_MAX, IFLA_RTA(ifi),
			   IFLA_PAYLOAD(n), NLA_F_NESTED);
	if (im)
		ll_entries_update(im, ifi, tb);
	else
		ll_entries_create(ifi, tb);
	return 0;
}

const char *ll_idx_n2a(unsigned int idx)
{
	static char buf[IFNAMSIZ];

	snprintf(buf, sizeof(buf), "if%u", idx);
	return buf;
}

static unsigned int ll_idx_a2n(const char *name)
{
	unsigned int idx;

	if (sscanf(name, "if%u", &idx) != 1)
		return 0;
	return idx;
}

static int ll_link_get(const char *name, int index)
{
	struct {
		struct nlmsghdr		n;
		struct ifinfomsg	ifm;
		char			buf[1024];
	} req = {
		.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg)),
		.n.nlmsg_flags = NLM_F_REQUEST,
		.n.nlmsg_type = RTM_GETLINK,
		.ifm.ifi_index = index,
	};
	__u32 filt_mask = RTEXT_FILTER_VF | RTEXT_FILTER_SKIP_STATS;
	struct rtnl_handle rth = {};
	struct nlmsghdr *answer;
	int rc = 0;

	if (rtnl_open(&rth, 0) < 0)
		return 0;

	addattr32(&req.n, sizeof(req), IFLA_EXT_MASK, filt_mask);
	if (name)
		addattr_l(&req.n, sizeof(req),
			  !check_ifname(name) ? IFLA_IFNAME : IFLA_ALT_IFNAME,
			  name, strlen(name) + 1);

	if (rtnl_talk_suppress_rtnl_errmsg(&rth, &req.n, &answer) < 0)
		goto out;

	/* add entry to cache */
	rc  = ll_remember_index(answer, NULL);
	if (!rc) {
		struct ifinfomsg *ifm = NLMSG_DATA(answer);

		rc = ifm->ifi_index;
	}

	free(answer);
out:
	rtnl_close(&rth);
	return rc;
}

const char *ll_index_to_name(unsigned int idx)
{
	static char buf[IFNAMSIZ];
	const struct ll_cache *im;

	if (idx == 0)
		return "*";

	im = ll_get_by_index(idx);
	if (im)
		return im->name;

	if (ll_link_get(NULL, idx) == idx) {
		im = ll_get_by_index(idx);
		if (im)
			return im->name;
	}

	if (if_indextoname(idx, buf) == NULL)
		snprintf(buf, IFNAMSIZ, "if%u", idx);

	return buf;
}

int ll_index_to_type(unsigned idx)
{
	const struct ll_cache *im;

	if (idx == 0)
		return -1;

	im = ll_get_by_index(idx);
	return im ? im->type : -1;
}

int ll_index_to_flags(unsigned idx)
{
	const struct ll_cache *im;

	if (idx == 0)
		return 0;

	im = ll_get_by_index(idx);
	return im ? im->flags : -1;
}

unsigned ll_name_to_index(const char *name)
{
	const struct ll_cache *im;
	unsigned idx;

	if (name == NULL)
		return 0;

	im = ll_get_by_name(name);
	if (im)
		return im->index;

	idx = ll_link_get(name, 0);
	if (idx == 0)
		idx = if_nametoindex(name);
	if (idx == 0)
		idx = ll_idx_a2n(name);
	return idx;
}

void ll_drop_by_index(unsigned index)
{
	struct ll_cache *im;

	im = ll_get_by_index(index);
	if (!im)
		return;

	hlist_del(&im->idx_hash);
	hlist_del(&im->name_hash);

	free(im);
}

void ll_init_map(struct rtnl_handle *rth)
{
	static int initialized;

	if (initialized)
		return;

	if (rtnl_linkdump_req(rth, AF_UNSPEC) < 0) {
		perror("Cannot send dump request");
		exit(1);
	}

	if (rtnl_dump_filter(rth, ll_remember_index, NULL) < 0) {
		fprintf(stderr, "Dump terminated\n");
		exit(1);
	}

	initialized = 1;
}
