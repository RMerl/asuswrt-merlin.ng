/*
 * prettymsg.c - human readable message dump
 *
 * Support for pretty print of an ethtool netlink message
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <limits.h>
#include <inttypes.h>
#include <linux/genetlink.h>
#include <linux/rtnetlink.h>
#include <linux/if_link.h>
#include <../../libmnl-1.0.4/include/libmnl/libmnl.h>

#include "prettymsg.h"

#define __INDENT 4
#define __DUMP_LINE 16
#define __DUMP_BLOCK 4

static void __print_binary_short(uint8_t *adata, unsigned int alen)
{
	unsigned int i;

	if (!alen)
		return;
	printf("%02x", adata[0]);
	for (i = 1; i < alen; i++)
		printf("%c%02x", (i % __DUMP_BLOCK) ? ':' : ' ',  adata[i]);
}

static void __print_binary_long(uint8_t *adata, unsigned int alen,
				unsigned int level)
{
	unsigned int i;

	for (i = 0; i < alen; i++) {
		if (i % __DUMP_LINE == 0)
			printf("\n%*s", __INDENT * (level + 2), "");
		else if (i % __DUMP_BLOCK == 0)
			printf("  ");
		else
			putchar(' ');
		printf("%02x", adata[i]);
	}
}

static int pretty_print_attr(const struct nlattr *attr,
			     const struct pretty_nla_desc *desc,
			     unsigned int ndesc, unsigned int level,
			     int err_offset, bool in_array)
{
	unsigned int alen = mnl_attr_get_payload_len(attr);
	unsigned int atype = mnl_attr_get_type(attr);
	unsigned int desc_idx = in_array ? 0 : atype;
	void *adata = mnl_attr_get_payload(attr);
	const struct pretty_nla_desc *adesc;
	const char *prefix = "    ";
	bool nested;

	adesc = (desc && desc_idx < ndesc) ? &desc[desc_idx] : NULL;
	nested = (adesc && (adesc->format == NLA_NESTED ||
			    adesc->format == NLA_ARRAY)) ||
		 (attr->nla_type & NLA_F_NESTED);
	if (err_offset >= 0 &&
	    err_offset < (nested ? NLA_HDRLEN : attr->nla_len)) {
		prefix = "===>";
		if (err_offset)
			fprintf(stderr,
				"ethtool: bad_attr inside an attribute (offset %d)\n",
				err_offset);
	}
	if (adesc && adesc->name && !in_array)
		printf("%s%*s%s", prefix, level * __INDENT, "", adesc->name);
	else
		printf("%s%*s[%u]", prefix, level * __INDENT, "", atype);

	if (nested) {
		struct nlattr *child;
		int ret = 0;

		putchar('\n');
		mnl_attr_for_each_nested(child, attr) {
			bool array = adesc && adesc->format == NLA_ARRAY;
			unsigned int child_off;

			child_off = (const char *)child - (const char *)attr;
			ret = pretty_print_attr(child,
						adesc ? adesc->children : NULL,
						adesc ? adesc->n_children : 0,
						level + 1,
						err_offset - child_off, array);
			if (ret < 0)
				break;
		}

		return ret;
	}

	printf(" = ");
	switch(adesc ? adesc->format : NLA_BINARY) {
	case NLA_U8:
		printf("%u", mnl_attr_get_u8(attr));
		break;
	case NLA_U16:
		printf("%u", mnl_attr_get_u16(attr));
		break;
	case NLA_U32:
		printf("%u", mnl_attr_get_u32(attr));
		break;
	case NLA_U64:
		printf("%" PRIu64, mnl_attr_get_u64(attr));
		break;
	case NLA_X8:
		printf("0x%02x", mnl_attr_get_u8(attr));
		break;
	case NLA_X16:
		printf("0x%04x", mnl_attr_get_u16(attr));
		break;
	case NLA_X32:
		printf("0x%08x", mnl_attr_get_u32(attr));
		break;
	case NLA_X64:
		printf("%" PRIx64, mnl_attr_get_u64(attr));
		break;
	case NLA_S8:
		printf("%d", (int)mnl_attr_get_u8(attr));
		break;
	case NLA_S16:
		printf("%d", (int)mnl_attr_get_u16(attr));
		break;
	case NLA_S32:
		printf("%d", (int)mnl_attr_get_u32(attr));
		break;
	case NLA_S64:
		printf("%" PRId64, (int64_t)mnl_attr_get_u64(attr));
		break;
	case NLA_STRING:
		printf("\"%.*s\"", alen, (const char *)adata);
		break;
	case NLA_FLAG:
		printf("true");
		break;
	case NLA_BOOL:
		printf("%s", mnl_attr_get_u8(attr) ? "on" : "off");
		break;
	case NLA_U8_ENUM:
	case NLA_U32_ENUM: {
		uint32_t val;

		if (adesc->format == NLA_U8_ENUM)
			val = mnl_attr_get_u8(attr);
		else
			val = mnl_attr_get_u32(attr);

		if (adesc && val < adesc->n_names && adesc->names[val])
			printf("%s", adesc->names[val]);
		else
			printf("%u", val);
		break;
	}
	default:
		if (alen <= __DUMP_LINE)
			__print_binary_short(adata, alen);
		else
			__print_binary_long(adata, alen, level);
	}
	putchar('\n');

	return 0;
}

static int pretty_print_nlmsg(const struct nlmsghdr *nlhdr,
			      unsigned int payload_offset,
			      const struct pretty_nla_desc *desc,
			      unsigned int ndesc, unsigned int err_offset)
{
	const struct nlattr *attr;
	int attr_offset;
	int ret;

	mnl_attr_for_each(attr, nlhdr, payload_offset) {
		attr_offset = (const char *)attr - (const char *)nlhdr;
		ret = pretty_print_attr(attr, desc, ndesc, 1,
					err_offset - attr_offset, false);
		if (ret < 0)
			return ret;
	}

	return 0;
}

int pretty_print_genlmsg(const struct nlmsghdr *nlhdr,
			 const struct pretty_nlmsg_desc *desc,
			 unsigned int ndesc, unsigned int err_offset)
{
	const struct pretty_nlmsg_desc *msg_desc;
	const struct genlmsghdr *genlhdr;

	if (mnl_nlmsg_get_payload_len(nlhdr) < GENL_HDRLEN) {
		fprintf(stderr, "ethtool: message too short (%u bytes)\n",
			nlhdr->nlmsg_len);
		return -EINVAL;
	}
	genlhdr = mnl_nlmsg_get_payload(nlhdr);
	msg_desc = (desc && genlhdr->cmd < ndesc) ? &desc[genlhdr->cmd] : NULL;
	if (msg_desc && msg_desc->name)
		printf("    %s\n", msg_desc->name);
	else
		printf("    [%u]\n", genlhdr->cmd);

	return pretty_print_nlmsg(nlhdr, GENL_HDRLEN,
				  msg_desc ? msg_desc->attrs : NULL,
				  msg_desc ? msg_desc->n_attrs : 0, err_offset);
}

static void rtm_link_summary(const struct ifinfomsg *ifinfo)
{
	if (ifinfo->ifi_family)
		printf(" family=%u", ifinfo->ifi_family);
	if (ifinfo->ifi_type)
		printf(" type=0x%04x", ifinfo->ifi_type);
	if (ifinfo->ifi_index)
		printf(" ifindex=%d", ifinfo->ifi_index);
	if (ifinfo->ifi_flags)
		printf(" flags=0x%x", ifinfo->ifi_flags);
	if (ifinfo->ifi_change)
		printf(" change=0x%x", ifinfo->ifi_change);
}

int pretty_print_rtnlmsg(const struct nlmsghdr *nlhdr, unsigned int err_offset)
{
	const unsigned int idx = (nlhdr->nlmsg_type - RTM_BASE) / 4;
	const struct pretty_nlmsg_desc *msg_desc = NULL;
	unsigned int hdrlen = USHRT_MAX;

	if (nlhdr->nlmsg_type < rtnl_msg_n_desc)
		msg_desc = &rtnl_msg_desc[nlhdr->nlmsg_type];
	if (idx < rtnl_msghdr_n_len)
		hdrlen = rtnl_msghdr_lengths[idx];
	if (hdrlen < USHRT_MAX && mnl_nlmsg_get_payload_len(nlhdr) < hdrlen) {
		fprintf(stderr, "ethtool: message too short (%u bytes)\n",
			nlhdr->nlmsg_len);
		return -EINVAL;
	}
	if (msg_desc && msg_desc->name)
		printf("    %s", msg_desc->name);
	else
		printf("    [%u]", nlhdr->nlmsg_type);
	if (idx == (RTM_NEWLINK - RTM_BASE) / 4)
		rtm_link_summary(mnl_nlmsg_get_payload(nlhdr));
	putchar('\n');
	if (hdrlen == USHRT_MAX)
		return 0;

	return pretty_print_nlmsg(nlhdr, hdrlen,
				  msg_desc ? msg_desc->attrs : NULL,
				  msg_desc ? msg_desc->n_attrs : 0, err_offset);
}
