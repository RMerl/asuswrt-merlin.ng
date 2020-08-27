/*
 * (C) 2012 by Hans Schillstrom <hans.schillstrom@ericsson.com>
 * (C) 2012 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Description: shared library add-on to iptables to add HMARK target support
 *
 * Initial development by Hans Schillstrom. Pablo's improvements to this piece
 * of software has been sponsored by Sophos Astaro <http://www.sophos.com>.
 */

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "xtables.h"
#include <linux/netfilter/xt_HMARK.h>

static void HMARK_help(void)
{
	printf(
"HMARK target options, i.e. modify hash calculation by:\n"
"  --hmark-tuple [src|dst|sport|dport|spi|proto|ct][,...]\n"
"  --hmark-mod value		    nfmark modulus value\n"
"  --hmark-offset value		    Last action add value to nfmark\n\n"
"  --hmark-rnd			    Random see for hashing\n"
" Alternatively, fine tuning of what will be included in hash calculation\n"
"  --hmark-src-prefix length	    Source address mask CIDR prefix\n"
"  --hmark-dst-prefix length	    Dest address mask CIDR prefix\n"
"  --hmark-sport-mask value	    Mask src port with value\n"
"  --hmark-dport-mask value	    Mask dst port with value\n"
"  --hmark-spi-mask value	    For esp and ah AND spi with value\n"
"  --hmark-sport value		    OR src port with value\n"
"  --hmark-dport value		    OR dst port with value\n"
"  --hmark-spi value		    For esp and ah OR spi with value\n"
"  --hmark-proto-mask value	    Mask Protocol with value\n");
}

#define hi struct xt_hmark_info

enum {
	O_HMARK_SADDR_MASK,
	O_HMARK_DADDR_MASK,
	O_HMARK_SPI,
	O_HMARK_SPI_MASK,
	O_HMARK_SPORT,
	O_HMARK_DPORT,
	O_HMARK_SPORT_MASK,
	O_HMARK_DPORT_MASK,
	O_HMARK_PROTO_MASK,
	O_HMARK_RND,
	O_HMARK_MODULUS,
	O_HMARK_OFFSET,
	O_HMARK_CT,
	O_HMARK_TYPE,
};

#define HMARK_OPT_PKT_MASK			\
	((1 << O_HMARK_SADDR_MASK)		| \
	 (1 << O_HMARK_DADDR_MASK)		| \
	 (1 << O_HMARK_SPI_MASK)		| \
	 (1 << O_HMARK_SPORT_MASK)		| \
	 (1 << O_HMARK_DPORT_MASK)		| \
	 (1 << O_HMARK_PROTO_MASK)		| \
	 (1 << O_HMARK_SPI_MASK)		| \
	 (1 << O_HMARK_SPORT)			| \
	 (1 << O_HMARK_DPORT)			| \
	 (1 << O_HMARK_SPI))

static const struct xt_option_entry HMARK_opts[] = {
	{ .name  = "hmark-tuple",
	  .type  = XTTYPE_STRING,
	  .id	 = O_HMARK_TYPE,
	},
	{ .name  = "hmark-src-prefix",
	  .type  = XTTYPE_PLENMASK,
	  .id	 = O_HMARK_SADDR_MASK,
	  .flags = XTOPT_PUT, XTOPT_POINTER(hi, src_mask)
	},
	{ .name  = "hmark-dst-prefix",
	  .type  = XTTYPE_PLENMASK,
	  .id	 = O_HMARK_DADDR_MASK,
	  .flags = XTOPT_PUT, XTOPT_POINTER(hi, dst_mask)
	},
	{ .name  = "hmark-sport-mask",
	  .type  = XTTYPE_UINT16,
	  .id	 = O_HMARK_SPORT_MASK,
	  .flags = XTOPT_PUT, XTOPT_POINTER(hi, port_mask.p16.src)
	},
	{ .name  = "hmark-dport-mask",
	  .type  = XTTYPE_UINT16,
	  .id	 = O_HMARK_DPORT_MASK,
	  .flags = XTOPT_PUT, XTOPT_POINTER(hi, port_mask.p16.dst)
	},
	{ .name  = "hmark-spi-mask",
	  .type  = XTTYPE_UINT32,
	  .id	 = O_HMARK_SPI_MASK,
	  .flags = XTOPT_PUT, XTOPT_POINTER(hi, port_mask.v32)
	},
	{ .name  = "hmark-sport",
	  .type  = XTTYPE_UINT16,
	  .id	 = O_HMARK_SPORT,
	  .flags = XTOPT_PUT, XTOPT_POINTER(hi, port_set.p16.src)
	},
	{ .name  = "hmark-dport",
	  .type  = XTTYPE_UINT16,
	  .id	 = O_HMARK_DPORT,
	  .flags = XTOPT_PUT, XTOPT_POINTER(hi, port_set.p16.dst)
	},
	{ .name  = "hmark-spi",
	  .type  = XTTYPE_UINT32,
	  .id	 = O_HMARK_SPI,
	  .flags = XTOPT_PUT, XTOPT_POINTER(hi, port_set.v32)
	},
	{ .name  = "hmark-proto-mask",
	  .type  = XTTYPE_UINT16,
	  .id	 = O_HMARK_PROTO_MASK,
	  .flags = XTOPT_PUT, XTOPT_POINTER(hi, proto_mask)
	},
	{ .name  = "hmark-rnd",
	  .type  = XTTYPE_UINT32,
	  .id	 = O_HMARK_RND,
	  .flags = XTOPT_PUT, XTOPT_POINTER(hi, hashrnd)
	},
	{ .name = "hmark-mod",
	  .type = XTTYPE_UINT32,
	  .id = O_HMARK_MODULUS,
	  .min = 1,
	  .flags = XTOPT_PUT | XTOPT_MAND, XTOPT_POINTER(hi, hmodulus)
	},
	{ .name  = "hmark-offset",
	  .type  = XTTYPE_UINT32,
	  .id	 = O_HMARK_OFFSET,
	  .flags = XTOPT_PUT, XTOPT_POINTER(hi, hoffset)
	},
	XTOPT_TABLEEND,
};

static int
hmark_parse(const char *type, size_t len, struct xt_hmark_info *info,
	    unsigned int *xflags)
{
	if (strncasecmp(type, "ct", len) == 0) {
		info->flags |= XT_HMARK_FLAG(XT_HMARK_CT);
		*xflags |= (1 << O_HMARK_CT);
	} else if (strncasecmp(type, "src", len) == 0) {
		memset(&info->src_mask, 0xff, sizeof(info->src_mask));
		info->flags |= XT_HMARK_FLAG(XT_HMARK_SADDR_MASK);
		*xflags |= (1 << O_HMARK_SADDR_MASK);
	} else if (strncasecmp(type, "dst", len) == 0) {
		memset(&info->dst_mask, 0xff, sizeof(info->dst_mask));
		info->flags |= XT_HMARK_FLAG(XT_HMARK_DADDR_MASK);
		*xflags |= (1 << O_HMARK_DADDR_MASK);
	} else if (strncasecmp(type, "sport", len) == 0) {
		memset(&info->port_mask.p16.src, 0xff,
			sizeof(info->port_mask.p16.src));
		info->flags |= XT_HMARK_FLAG(XT_HMARK_SPORT_MASK);
		*xflags |= (1 << O_HMARK_SPORT_MASK);
	} else if (strncasecmp(type, "dport", len) == 0) {
		memset(&info->port_mask.p16.dst, 0xff,
			sizeof(info->port_mask.p16.dst));
		info->flags |= XT_HMARK_FLAG(XT_HMARK_DPORT_MASK);
		*xflags |= (1 << O_HMARK_DPORT_MASK);
	} else if (strncasecmp(type, "proto", len) == 0) {
		memset(&info->proto_mask, 0xff, sizeof(info->proto_mask));
		info->flags |= XT_HMARK_FLAG(XT_HMARK_PROTO_MASK);
		*xflags |= (1 << O_HMARK_PROTO_MASK);
	} else if (strncasecmp(type, "spi", len) == 0) {
		memset(&info->port_mask.v32, 0xff, sizeof(info->port_mask.v32));
		info->flags |= XT_HMARK_FLAG(XT_HMARK_SPI_MASK);
		*xflags |= (1 << O_HMARK_SPI_MASK);
	} else
		return 0;

	return 1;
}

static void
hmark_parse_type(struct xt_option_call *cb)
{
	const char *arg = cb->arg;
	struct xt_hmark_info *info = cb->data;
	const char *comma;

	while ((comma = strchr(arg, ',')) != NULL) {
		if (comma == arg ||
		    !hmark_parse(arg, comma-arg, info, &cb->xflags))
			xtables_error(PARAMETER_PROBLEM, "Bad type \"%s\"", arg);
		arg = comma+1;
	}
	if (!*arg)
		xtables_error(PARAMETER_PROBLEM, "\"--hmark-tuple\" requires "
						 "a list of types with no "
						 "spaces, e.g. "
						 "src,dst,sport,dport,proto");
	if (strlen(arg) == 0 ||
	    !hmark_parse(arg, strlen(arg), info, &cb->xflags))
		xtables_error(PARAMETER_PROBLEM, "Bad type \"%s\"", arg);
}

static void HMARK_parse(struct xt_option_call *cb, int plen)
{
	struct xt_hmark_info *info = cb->data;

	xtables_option_parse(cb);

	switch (cb->entry->id) {
	case O_HMARK_TYPE:
		hmark_parse_type(cb);
		break;
	case O_HMARK_SADDR_MASK:
		info->flags |= XT_HMARK_FLAG(XT_HMARK_SADDR_MASK);
		break;
	case O_HMARK_DADDR_MASK:
		info->flags |= XT_HMARK_FLAG(XT_HMARK_DADDR_MASK);
		break;
	case O_HMARK_SPI:
		info->port_set.v32 = htonl(cb->val.u32);
		info->flags |= XT_HMARK_FLAG(XT_HMARK_SPI);
		break;
	case O_HMARK_SPORT:
		info->port_set.p16.src = htons(cb->val.u16);
		info->flags |= XT_HMARK_FLAG(XT_HMARK_SPORT);
		break;
	case O_HMARK_DPORT:
		info->port_set.p16.dst = htons(cb->val.u16);
		info->flags |= XT_HMARK_FLAG(XT_HMARK_DPORT);
		break;
	case O_HMARK_SPORT_MASK:
		info->port_mask.p16.src = htons(cb->val.u16);
		info->flags |= XT_HMARK_FLAG(XT_HMARK_SPORT_MASK);
		break;
	case O_HMARK_DPORT_MASK:
		info->port_mask.p16.dst = htons(cb->val.u16);
		info->flags |= XT_HMARK_FLAG(XT_HMARK_DPORT_MASK);
		break;
	case O_HMARK_SPI_MASK:
		info->port_mask.v32 = htonl(cb->val.u32);
		info->flags |= XT_HMARK_FLAG(XT_HMARK_SPI_MASK);
		break;
	case O_HMARK_PROTO_MASK:
		info->flags |= XT_HMARK_FLAG(XT_HMARK_PROTO_MASK);
		break;
	case O_HMARK_RND:
		info->flags |= XT_HMARK_FLAG(XT_HMARK_RND);
		break;
	case O_HMARK_MODULUS:
		info->flags |= XT_HMARK_FLAG(XT_HMARK_MODULUS);
		break;
	case O_HMARK_OFFSET:
		info->flags |= XT_HMARK_FLAG(XT_HMARK_OFFSET);
		break;
	case O_HMARK_CT:
		info->flags |= XT_HMARK_FLAG(XT_HMARK_CT);
		break;
	}
	cb->xflags |= (1 << cb->entry->id);
}

static void HMARK_ip4_parse(struct xt_option_call *cb)
{
	HMARK_parse(cb, 32);
}
static void HMARK_ip6_parse(struct xt_option_call *cb)
{
	HMARK_parse(cb, 128);
}

static void HMARK_check(struct xt_fcheck_call *cb)
{
	if (!(cb->xflags & (1 << O_HMARK_MODULUS)))
		xtables_error(PARAMETER_PROBLEM, "--hmark-mod is mandatory");
	if (!(cb->xflags & (1 << O_HMARK_RND)))
		xtables_error(PARAMETER_PROBLEM, "--hmark-rnd is mandatory");
	if (cb->xflags & (1 << O_HMARK_SPI_MASK) &&
	    (cb->xflags & ((1 << O_HMARK_SPORT_MASK) |
			   (1 << O_HMARK_DPORT_MASK))))
		xtables_error(PARAMETER_PROBLEM, "you cannot use "
				"--hmark-spi-mask and --hmark-?port-mask,"
				"at the same time");
	if (!((cb->xflags & HMARK_OPT_PKT_MASK) ||
	       cb->xflags & (1 << O_HMARK_CT)))
		xtables_error(PARAMETER_PROBLEM, "you have to specify "
				"--hmark-tuple at least");
}

static void HMARK_print(const struct xt_hmark_info *info)
{
	if (info->flags & XT_HMARK_FLAG(XT_HMARK_SPORT_MASK))
		printf("sport-mask 0x%x ", htons(info->port_mask.p16.src));
	if (info->flags & XT_HMARK_FLAG(XT_HMARK_DPORT_MASK))
		printf("dport-mask 0x%x ", htons(info->port_mask.p16.dst));
	if (info->flags & XT_HMARK_FLAG(XT_HMARK_SPI_MASK))
		printf("spi-mask 0x%x ", htonl(info->port_mask.v32));
	if (info->flags & XT_HMARK_FLAG(XT_HMARK_SPORT))
		printf("sport 0x%x ", htons(info->port_set.p16.src));
	if (info->flags & XT_HMARK_FLAG(XT_HMARK_DPORT))
		printf("dport 0x%x ", htons(info->port_set.p16.dst));
	if (info->flags & XT_HMARK_FLAG(XT_HMARK_SPI))
		printf("spi 0x%x ", htonl(info->port_set.v32));
	if (info->flags & XT_HMARK_FLAG(XT_HMARK_PROTO_MASK))
		printf("proto-mask 0x%x ", info->proto_mask);
	if (info->flags & XT_HMARK_FLAG(XT_HMARK_RND))
		printf("rnd 0x%x ", info->hashrnd);
}

static void HMARK_ip6_print(const void *ip,
			    const struct xt_entry_target *target, int numeric)
{
	const struct xt_hmark_info *info =
			(const struct xt_hmark_info *)target->data;

	printf(" HMARK ");
	if (info->flags & XT_HMARK_FLAG(XT_HMARK_MODULUS))
		printf("mod %u ", info->hmodulus);
	if (info->flags & XT_HMARK_FLAG(XT_HMARK_OFFSET))
		printf("+ 0x%x ", info->hoffset);
	if (info->flags & XT_HMARK_FLAG(XT_HMARK_CT))
		printf("ct, ");
	if (info->flags & XT_HMARK_FLAG(XT_HMARK_SADDR_MASK))
		printf("src-prefix %s ",
		       xtables_ip6mask_to_numeric(&info->src_mask.in6) + 1);
	if (info->flags & XT_HMARK_FLAG(XT_HMARK_DADDR_MASK))
		printf("dst-prefix %s ",
		       xtables_ip6mask_to_numeric(&info->dst_mask.in6) + 1);
	HMARK_print(info);
}
static void HMARK_ip4_print(const void *ip,
			    const struct xt_entry_target *target, int numeric)
{
	const struct xt_hmark_info *info =
		(const struct xt_hmark_info *)target->data;

	printf(" HMARK ");
	if (info->flags & XT_HMARK_FLAG(XT_HMARK_MODULUS))
		printf("mod %u ", info->hmodulus);
	if (info->flags & XT_HMARK_FLAG(XT_HMARK_OFFSET))
		printf("+ 0x%x ", info->hoffset);
	if (info->flags & XT_HMARK_FLAG(XT_HMARK_CT))
		printf("ct, ");
	if (info->flags & XT_HMARK_FLAG(XT_HMARK_SADDR_MASK))
		printf("src-prefix %u ",
		       xtables_ipmask_to_cidr(&info->src_mask.in));
	if (info->flags & XT_HMARK_FLAG(XT_HMARK_DADDR_MASK))
		printf("dst-prefix %u ",
		       xtables_ipmask_to_cidr(&info->dst_mask.in));
	HMARK_print(info);
}

static void HMARK_save(const struct xt_hmark_info *info)
{
	if (info->flags & XT_HMARK_FLAG(XT_HMARK_SPORT_MASK))
		printf(" --hmark-sport-mask 0x%04x",
		       htons(info->port_mask.p16.src));
	if (info->flags & XT_HMARK_FLAG(XT_HMARK_DPORT_MASK))
		printf(" --hmark-dport-mask 0x%04x",
		       htons(info->port_mask.p16.dst));
	if (info->flags & XT_HMARK_FLAG(XT_HMARK_SPI_MASK))
		printf(" --hmark-spi-mask 0x%08x",
		       htonl(info->port_mask.v32));
	if (info->flags & XT_HMARK_FLAG(XT_HMARK_SPORT))
		printf(" --hmark-sport 0x%04x",
		       htons(info->port_set.p16.src));
	if (info->flags & XT_HMARK_FLAG(XT_HMARK_DPORT))
		printf(" --hmark-dport 0x%04x",
		       htons(info->port_set.p16.dst));
	if (info->flags & XT_HMARK_FLAG(XT_HMARK_SPI))
		printf(" --hmark-spi 0x%08x", htonl(info->port_set.v32));
	if (info->flags & XT_HMARK_FLAG(XT_HMARK_PROTO_MASK))
		printf(" --hmark-proto-mask 0x%02x", info->proto_mask);
	if (info->flags & XT_HMARK_FLAG(XT_HMARK_RND))
		printf(" --hmark-rnd 0x%08x", info->hashrnd);
	if (info->flags & XT_HMARK_FLAG(XT_HMARK_MODULUS))
		printf(" --hmark-mod %u", info->hmodulus);
	if (info->flags & XT_HMARK_FLAG(XT_HMARK_OFFSET))
		printf(" --hmark-offset %u", info->hoffset);
	if (info->flags & XT_HMARK_FLAG(XT_HMARK_CT))
		printf(" --hmark-tuple ct");
}

static void HMARK_ip6_save(const void *ip, const struct xt_entry_target *target)
{
	const struct xt_hmark_info *info =
		(const struct xt_hmark_info *)target->data;
	int ret;

	if (info->flags & XT_HMARK_FLAG(XT_HMARK_SADDR_MASK)) {
		ret = xtables_ip6mask_to_cidr(&info->src_mask.in6);
		printf(" --hmark-src-prefix %d", ret);
	}
	if (info->flags & XT_HMARK_FLAG(XT_HMARK_DADDR_MASK)) {
		ret = xtables_ip6mask_to_cidr(&info->dst_mask.in6);
		printf(" --hmark-dst-prefix %d", ret);
	}
	HMARK_save(info);
}

static void HMARK_ip4_save(const void *ip, const struct xt_entry_target *target)
{
	const struct xt_hmark_info *info =
		(const struct xt_hmark_info *)target->data;
	int ret;

	if (info->flags & XT_HMARK_FLAG(XT_HMARK_SADDR_MASK)) {
		ret = xtables_ipmask_to_cidr(&info->src_mask.in);
		printf(" --hmark-src-prefix %d", ret);
	}
	if (info->flags & XT_HMARK_FLAG(XT_HMARK_DADDR_MASK)) {
		ret = xtables_ipmask_to_cidr(&info->dst_mask.in);
		printf(" --hmark-dst-prefix %d", ret);
	}
	HMARK_save(info);
}

static struct xtables_target mark_tg_reg[] = {
	{
		.family        = NFPROTO_IPV4,
		.name	       = "HMARK",
		.version       = XTABLES_VERSION,
		.size	       = XT_ALIGN(sizeof(struct xt_hmark_info)),
		.userspacesize = XT_ALIGN(sizeof(struct xt_hmark_info)),
		.help	       = HMARK_help,
		.print	       = HMARK_ip4_print,
		.save	       = HMARK_ip4_save,
		.x6_parse      = HMARK_ip4_parse,
		.x6_fcheck     = HMARK_check,
		.x6_options    = HMARK_opts,
	},
	{
		.family        = NFPROTO_IPV6,
		.name	       = "HMARK",
		.version       = XTABLES_VERSION,
		.size	       = XT_ALIGN(sizeof(struct xt_hmark_info)),
		.userspacesize = XT_ALIGN(sizeof(struct xt_hmark_info)),
		.help	       = HMARK_help,
		.print	       = HMARK_ip6_print,
		.save	       = HMARK_ip6_save,
		.x6_parse      = HMARK_ip6_parse,
		.x6_fcheck     = HMARK_check,
		.x6_options    = HMARK_opts,
	},
};

void _init(void)
{
	xtables_register_targets(mark_tg_reg, ARRAY_SIZE(mark_tg_reg));
}
