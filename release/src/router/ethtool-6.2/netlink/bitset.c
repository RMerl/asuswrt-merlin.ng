/*
 * bitset.h - netlink bitset helpers
 *
 * Functions for easier handling of ethtool netlink bitset attributes.
 */

#include <stdio.h>
#include <errno.h>

#include "../common.h"
#include "netlink.h"
#include "bitset.h"

uint32_t bitset_get_count(const struct nlattr *bitset, int *retptr)
{
	const struct nlattr *attr;

	mnl_attr_for_each_nested(attr, bitset) {
		if (mnl_attr_get_type(attr) != ETHTOOL_A_BITSET_SIZE)
			continue;
		*retptr = 0;
		return mnl_attr_get_u32(attr);
	}

	*retptr = -EFAULT;
	return 0;
}

bool bitset_is_compact(const struct nlattr *bitset)
{
	const struct nlattr *attr;

	mnl_attr_for_each_nested(attr, bitset) {
		switch(mnl_attr_get_type(attr)) {
		case ETHTOOL_A_BITSET_BITS:
			return 0;
		case ETHTOOL_A_BITSET_VALUE:
		case ETHTOOL_A_BITSET_MASK:
			return 1;
		}
	}

	return false;
}

bool bitset_get_bit(const struct nlattr *bitset, bool mask, unsigned int idx,
		    int *retptr)
{
	const struct nlattr *bitset_tb[ETHTOOL_A_BITSET_MAX + 1] = {};
	DECLARE_ATTR_TB_INFO(bitset_tb);
	const struct nlattr *bits;
	const struct nlattr *bit;
	bool nomask;
	int ret;

	*retptr = 0;
	ret = mnl_attr_parse_nested(bitset, attr_cb, &bitset_tb_info);
	if (ret < 0)
		goto err;

	nomask = bitset_tb[ETHTOOL_A_BITSET_NOMASK];
	if (mask && nomask) {
		/* Trying to determine if a bit is set in the mask of a "no
		 * mask" bitset doesn't make sense.
		 */
		ret = -EFAULT;
		goto err;
	}

	bits = mask ? bitset_tb[ETHTOOL_A_BITSET_MASK] :
		      bitset_tb[ETHTOOL_A_BITSET_VALUE];
	if (bits) {
		const uint32_t *bitmap =
			(const uint32_t *)mnl_attr_get_payload(bits);

		if (idx >= 8 * mnl_attr_get_payload_len(bits))
			return false;
		return bitmap[idx / 32] & (1U << (idx % 32));
	}

	bits = bitset_tb[ETHTOOL_A_BITSET_BITS];
	if (!bits)
		goto err;
	mnl_attr_for_each_nested(bit, bits) {
		const struct nlattr *tb[ETHTOOL_A_BITSET_BIT_MAX + 1] = {};
		DECLARE_ATTR_TB_INFO(tb);
		unsigned int my_idx;

		if (mnl_attr_get_type(bit) != ETHTOOL_A_BITSET_BITS_BIT)
			continue;
		ret = mnl_attr_parse_nested(bit, attr_cb, &tb_info);
		if (ret < 0)
			goto err;
		ret = -EFAULT;
		if (!tb[ETHTOOL_A_BITSET_BIT_INDEX])
			goto err;

		my_idx = mnl_attr_get_u32(tb[ETHTOOL_A_BITSET_BIT_INDEX]);
		if (my_idx == idx)
			return mask || nomask || tb[ETHTOOL_A_BITSET_BIT_VALUE];
	}

	return false;
err:
	fprintf(stderr, "malformed netlink message (bitset)\n");
	*retptr = ret;
	return false;
}

bool bitset_is_empty(const struct nlattr *bitset, bool mask, int *retptr)
{
	const struct nlattr *bitset_tb[ETHTOOL_A_BITSET_MAX + 1] = {};
	DECLARE_ATTR_TB_INFO(bitset_tb);
	const struct nlattr *bits;
	const struct nlattr *bit;
	int ret;

	*retptr = 0;
	ret = mnl_attr_parse_nested(bitset, attr_cb, &bitset_tb_info);
	if (ret < 0)
		goto err;

	bits = mask ? bitset_tb[ETHTOOL_A_BITSET_MASK] :
		      bitset_tb[ETHTOOL_A_BITSET_VALUE];
	if (bits) {
		const uint32_t *bitmap =
			(const uint32_t *)mnl_attr_get_payload(bits);
		unsigned int n = mnl_attr_get_payload_len(bits);
		unsigned int i;

		ret = -EFAULT;
		if (n % 4)
			goto err;
		for (i = 0; i < n / 4; i++)
			if (bitmap[i])
				return false;
		return true;
	}

	bits = bitset_tb[ETHTOOL_A_BITSET_BITS];
	if (!bits)
		goto err;
	mnl_attr_for_each_nested(bit, bits) {
		const struct nlattr *tb[ETHTOOL_A_BITSET_BIT_MAX + 1] = {};
		DECLARE_ATTR_TB_INFO(tb);

		if (mnl_attr_get_type(bit) != ETHTOOL_A_BITSET_BITS_BIT)
			continue;
		if (mask || bitset_tb[ETHTOOL_A_BITSET_NOMASK])
			return false;

		ret = mnl_attr_parse_nested(bit, attr_cb, &tb_info);
		if (ret < 0)
			goto err;
		if (tb[ETHTOOL_A_BITSET_BIT_VALUE])
			return false;
	}

	return true;
err:
	fprintf(stderr, "malformed netlink message (bitset)\n");
	*retptr = ret;
	return true;
}

static uint32_t *get_compact_bitset_attr(const struct nlattr *bitset,
					 uint16_t type)
{
	const struct nlattr *tb[ETHTOOL_A_BITSET_MAX + 1] = {};
	DECLARE_ATTR_TB_INFO(tb);
	unsigned int count;
	int ret;

	ret = mnl_attr_parse_nested(bitset, attr_cb, &tb_info);
	if (ret < 0)
		return NULL;
	if (!tb[ETHTOOL_A_BITSET_SIZE] || !tb[ETHTOOL_A_BITSET_VALUE] ||
	    !tb[type])
		return NULL;
	count = mnl_attr_get_u32(tb[ETHTOOL_A_BITSET_SIZE]);
	if (8 * mnl_attr_get_payload_len(tb[type]) < count)
		return NULL;

	return mnl_attr_get_payload(tb[type]);
}

uint32_t *get_compact_bitset_value(const struct nlattr *bitset)
{
	return get_compact_bitset_attr(bitset, ETHTOOL_A_BITSET_VALUE);
}

uint32_t *get_compact_bitset_mask(const struct nlattr *bitset)
{
	return get_compact_bitset_attr(bitset, ETHTOOL_A_BITSET_MASK);
}

int walk_bitset(const struct nlattr *bitset, const struct stringset *labels,
		bitset_walk_callback cb, void *data)
{
	const struct nlattr *bitset_tb[ETHTOOL_A_BITSET_MAX + 1] = {};
	DECLARE_ATTR_TB_INFO(bitset_tb);
	const struct nlattr *bits;
	const struct nlattr *bit;
	bool is_list;
	int ret;

	ret = mnl_attr_parse_nested(bitset, attr_cb, &bitset_tb_info);
	if (ret < 0)
		return ret;
	is_list = bitset_tb[ETHTOOL_A_BITSET_NOMASK];

	bits = bitset_tb[ETHTOOL_A_BITSET_VALUE];
	if (bits) {
		const struct nlattr *mask = bitset_tb[ETHTOOL_A_BITSET_MASK];
		unsigned int count, nwords, idx;
		uint32_t *val_bm;
		uint32_t *mask_bm;

		if (!bitset_tb[ETHTOOL_A_BITSET_SIZE])
			return -EFAULT;
		count = mnl_attr_get_u32(bitset_tb[ETHTOOL_A_BITSET_SIZE]);
		nwords = (count + 31) / 32;
		if ((mnl_attr_get_payload_len(bits) / 4 < nwords) ||
		    (mask && mnl_attr_get_payload_len(mask) / 4 < nwords))
			return -EFAULT;

		val_bm = mnl_attr_get_payload(bits);
		mask_bm = mask ? mnl_attr_get_payload(mask) : NULL;
		for (idx = 0; idx < count; idx++)
			if (!mask_bm || (mask_bm[idx / 32] & (1 << (idx % 32))))
				cb(idx, get_string(labels, idx),
				   val_bm[idx / 32] & (1 << (idx % 32)), data);
		return 0;
	}

	bits = bitset_tb[ETHTOOL_A_BITSET_BITS];
	if (!bits)
		return -EFAULT;
	mnl_attr_for_each_nested(bit, bits) {
		const struct nlattr *tb[ETHTOOL_A_BITSET_BIT_MAX + 1] = {};
		DECLARE_ATTR_TB_INFO(tb);
		const char *name;
		unsigned int idx;

		if (mnl_attr_get_type(bit) != ETHTOOL_A_BITSET_BITS_BIT)
			continue;

		ret = mnl_attr_parse_nested(bit, attr_cb, &tb_info);
		if (ret < 0 || !tb[ETHTOOL_A_BITSET_BIT_INDEX] ||
		    !tb[ETHTOOL_A_BITSET_BIT_NAME])
			return -EFAULT;

		idx = mnl_attr_get_u32(tb[ETHTOOL_A_BITSET_BIT_INDEX]);
		name = mnl_attr_get_str(tb[ETHTOOL_A_BITSET_BIT_NAME]);
		cb(idx, name, is_list || tb[ETHTOOL_A_BITSET_BIT_VALUE], data);
	}

	return 0;
}
