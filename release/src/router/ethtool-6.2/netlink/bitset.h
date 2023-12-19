/*
 * bitset.h - netlink bitset helpers
 *
 * Declarations of helpers for handling ethtool netlink bitsets.
 */

#ifndef ETHTOOL_NETLINK_BITSET_H__
#define ETHTOOL_NETLINK_BITSET_H__

#include <../../libmnl-1.0.4/include/libmnl/libmnl.h>
#include <linux/netlink.h>
#include <linux/genetlink.h>
#include <linux/ethtool_netlink.h>
#include "strset.h"

typedef void (*bitset_walk_callback)(unsigned int, const char *, bool, void *);

uint32_t bitset_get_count(const struct nlattr *bitset, int *retptr);
bool bitset_get_bit(const struct nlattr *bitset, bool mask, unsigned int idx,
		    int *retptr);
bool bitset_is_compact(const struct nlattr *bitset);
bool bitset_is_empty(const struct nlattr *bitset, bool mask, int *retptr);
uint32_t *get_compact_bitset_value(const struct nlattr *bitset);
uint32_t *get_compact_bitset_mask(const struct nlattr *bitset);
int walk_bitset(const struct nlattr *bitset, const struct stringset *labels,
		bitset_walk_callback cb, void *data);

#endif /* ETHTOOL_NETLINK_BITSET_H__ */
