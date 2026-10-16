/* Shared library to add Segment Routing Header (SRH) matching support.
 *
 * Author:
 *       Ahmed Abdelsalam       <amsalam20@gmail.com>
 */

#include <stdio.h>
#include <xtables.h>
#include <linux/netfilter_ipv6/ip6t_srh.h>
#include <string.h>

/* srh command-line options */
enum {
	O_SRH_NEXTHDR,
	O_SRH_LEN_EQ,
	O_SRH_LEN_GT,
	O_SRH_LEN_LT,
	O_SRH_SEGS_EQ,
	O_SRH_SEGS_GT,
	O_SRH_SEGS_LT,
	O_SRH_LAST_EQ,
	O_SRH_LAST_GT,
	O_SRH_LAST_LT,
	O_SRH_TAG,
	O_SRH_PSID,
	O_SRH_NSID,
	O_SRH_LSID,
};

static void srh_help(void)
{
	printf(
"srh match options:\n"
"[!] --srh-next-hdr		next-hdr        Next Header value of SRH\n"
"[!] --srh-hdr-len-eq		hdr_len         Hdr Ext Len value of SRH\n"
"[!] --srh-hdr-len-gt		hdr_len         Hdr Ext Len value of SRH\n"
"[!] --srh-hdr-len-lt		hdr_len         Hdr Ext Len value of SRH\n"
"[!] --srh-segs-left-eq		segs_left       Segments Left value of SRH\n"
"[!] --srh-segs-left-gt		segs_left       Segments Left value of SRH\n"
"[!] --srh-segs-left-lt		segs_left       Segments Left value of SRH\n"
"[!] --srh-last-entry-eq 	last_entry      Last Entry value of SRH\n"
"[!] --srh-last-entry-gt 	last_entry      Last Entry value of SRH\n"
"[!] --srh-last-entry-lt 	last_entry      Last Entry value of SRH\n"
"[!] --srh-tag			tag             Tag value of SRH\n"
"[!] --srh-psid			addr[/mask]	SRH previous SID\n"
"[!] --srh-nsid			addr[/mask]	SRH next SID\n"
"[!] --srh-lsid			addr[/mask]	SRH Last SID\n");
}

#define s struct ip6t_srh
static const struct xt_option_entry srh_opts[] = {
	{ .name = "srh-next-hdr", .id = O_SRH_NEXTHDR, .type = XTTYPE_UINT8,
	.flags = XTOPT_INVERT | XTOPT_PUT, XTOPT_POINTER(s, next_hdr)},
	{ .name = "srh-hdr-len-eq", .id = O_SRH_LEN_EQ, .type = XTTYPE_UINT8,
	.flags = XTOPT_INVERT | XTOPT_PUT, XTOPT_POINTER(s, hdr_len)},
	{ .name = "srh-hdr-len-gt", .id = O_SRH_LEN_GT, .type = XTTYPE_UINT8,
	.flags = XTOPT_INVERT | XTOPT_PUT, XTOPT_POINTER(s, hdr_len)},
	{ .name = "srh-hdr-len-lt", .id = O_SRH_LEN_LT, .type = XTTYPE_UINT8,
	.flags = XTOPT_INVERT | XTOPT_PUT, XTOPT_POINTER(s, hdr_len)},
	{ .name = "srh-segs-left-eq", .id = O_SRH_SEGS_EQ, .type = XTTYPE_UINT8,
	.flags = XTOPT_INVERT | XTOPT_PUT, XTOPT_POINTER(s, segs_left)},
	{ .name = "srh-segs-left-gt", .id = O_SRH_SEGS_GT, .type = XTTYPE_UINT8,
	.flags = XTOPT_INVERT | XTOPT_PUT, XTOPT_POINTER(s, segs_left)},
	{ .name = "srh-segs-left-lt", .id = O_SRH_SEGS_LT, .type = XTTYPE_UINT8,
	.flags = XTOPT_INVERT | XTOPT_PUT, XTOPT_POINTER(s, segs_left)},
	{ .name = "srh-last-entry-eq", .id = O_SRH_LAST_EQ, .type = XTTYPE_UINT8,
	.flags = XTOPT_INVERT | XTOPT_PUT, XTOPT_POINTER(s, last_entry)},
	{ .name = "srh-last-entry-gt", .id = O_SRH_LAST_GT, .type = XTTYPE_UINT8,
	.flags = XTOPT_INVERT | XTOPT_PUT, XTOPT_POINTER(s, last_entry)},
	{ .name = "srh-last-entry-lt", .id = O_SRH_LAST_LT, .type = XTTYPE_UINT8,
	.flags = XTOPT_INVERT | XTOPT_PUT, XTOPT_POINTER(s, last_entry)},
	{ .name = "srh-tag", .id = O_SRH_TAG, .type = XTTYPE_UINT16,
	.flags = XTOPT_INVERT | XTOPT_PUT, XTOPT_POINTER(s, tag)},
	{ }
};
#undef s

#define s struct ip6t_srh1
static const struct xt_option_entry srh1_opts[] = {
	{ .name = "srh-next-hdr", .id = O_SRH_NEXTHDR, .type = XTTYPE_UINT8,
	.flags = XTOPT_INVERT | XTOPT_PUT, XTOPT_POINTER(s, next_hdr)},
	{ .name = "srh-hdr-len-eq", .id = O_SRH_LEN_EQ, .type = XTTYPE_UINT8,
	.flags = XTOPT_INVERT | XTOPT_PUT, XTOPT_POINTER(s, hdr_len)},
	{ .name = "srh-hdr-len-gt", .id = O_SRH_LEN_GT, .type = XTTYPE_UINT8,
	.flags = XTOPT_INVERT | XTOPT_PUT, XTOPT_POINTER(s, hdr_len)},
	{ .name = "srh-hdr-len-lt", .id = O_SRH_LEN_LT, .type = XTTYPE_UINT8,
	.flags = XTOPT_INVERT | XTOPT_PUT, XTOPT_POINTER(s, hdr_len)},
	{ .name = "srh-segs-left-eq", .id = O_SRH_SEGS_EQ, .type = XTTYPE_UINT8,
	.flags = XTOPT_INVERT | XTOPT_PUT, XTOPT_POINTER(s, segs_left)},
	{ .name = "srh-segs-left-gt", .id = O_SRH_SEGS_GT, .type = XTTYPE_UINT8,
	.flags = XTOPT_INVERT | XTOPT_PUT, XTOPT_POINTER(s, segs_left)},
	{ .name = "srh-segs-left-lt", .id = O_SRH_SEGS_LT, .type = XTTYPE_UINT8,
	.flags = XTOPT_INVERT | XTOPT_PUT, XTOPT_POINTER(s, segs_left)},
	{ .name = "srh-last-entry-eq", .id = O_SRH_LAST_EQ, .type = XTTYPE_UINT8,
	.flags = XTOPT_INVERT | XTOPT_PUT, XTOPT_POINTER(s, last_entry)},
	{ .name = "srh-last-entry-gt", .id = O_SRH_LAST_GT, .type = XTTYPE_UINT8,
	.flags = XTOPT_INVERT | XTOPT_PUT, XTOPT_POINTER(s, last_entry)},
	{ .name = "srh-last-entry-lt", .id = O_SRH_LAST_LT, .type = XTTYPE_UINT8,
	.flags = XTOPT_INVERT | XTOPT_PUT, XTOPT_POINTER(s, last_entry)},
	{ .name = "srh-tag", .id = O_SRH_TAG, .type = XTTYPE_UINT16,
	.flags = XTOPT_INVERT | XTOPT_PUT, XTOPT_POINTER(s, tag)},
	{ .name = "srh-psid", .id = O_SRH_PSID, .type = XTTYPE_HOSTMASK,
	.flags = XTOPT_INVERT},
	{ .name = "srh-nsid", .id = O_SRH_NSID, .type = XTTYPE_HOSTMASK,
	.flags = XTOPT_INVERT},
	{ .name = "srh-lsid", .id = O_SRH_LSID, .type = XTTYPE_HOSTMASK,
	.flags = XTOPT_INVERT},
	{ }
};
#undef s

static void srh_init(struct xt_entry_match *m)
{
	struct ip6t_srh *srhinfo = (void *)m->data;

	srhinfo->mt_flags = 0;
	srhinfo->mt_invflags = 0;
}

static void srh1_init(struct xt_entry_match *m)
{
	struct ip6t_srh1 *srhinfo = (void *)m->data;

	srhinfo->mt_flags = 0;
	srhinfo->mt_invflags = 0;
	memset(srhinfo->psid_addr.s6_addr, 0, sizeof(srhinfo->psid_addr.s6_addr));
	memset(srhinfo->nsid_addr.s6_addr, 0, sizeof(srhinfo->nsid_addr.s6_addr));
	memset(srhinfo->lsid_addr.s6_addr, 0, sizeof(srhinfo->lsid_addr.s6_addr));
	memset(srhinfo->psid_msk.s6_addr, 0, sizeof(srhinfo->psid_msk.s6_addr));
	memset(srhinfo->nsid_msk.s6_addr, 0, sizeof(srhinfo->nsid_msk.s6_addr));
	memset(srhinfo->lsid_msk.s6_addr, 0, sizeof(srhinfo->lsid_msk.s6_addr));
}

static void srh_parse(struct xt_option_call *cb)
{
	struct ip6t_srh *srhinfo = cb->data;

	xtables_option_parse(cb);
	switch (cb->entry->id) {
	case O_SRH_NEXTHDR:
		srhinfo->mt_flags |= IP6T_SRH_NEXTHDR;
		if (cb->invert)
			srhinfo->mt_invflags |= IP6T_SRH_INV_NEXTHDR;
		break;
	case O_SRH_LEN_EQ:
		srhinfo->mt_flags |= IP6T_SRH_LEN_EQ;
		if (cb->invert)
			srhinfo->mt_invflags |= IP6T_SRH_INV_LEN_EQ;
		break;
	case O_SRH_LEN_GT:
		srhinfo->mt_flags |= IP6T_SRH_LEN_GT;
		if (cb->invert)
			srhinfo->mt_invflags |= IP6T_SRH_INV_LEN_GT;
		break;
	case O_SRH_LEN_LT:
		srhinfo->mt_flags |= IP6T_SRH_LEN_LT;
		if (cb->invert)
			srhinfo->mt_invflags |= IP6T_SRH_INV_LEN_LT;
		break;
	case O_SRH_SEGS_EQ:
		srhinfo->mt_flags |= IP6T_SRH_SEGS_EQ;
		if (cb->invert)
			srhinfo->mt_invflags |= IP6T_SRH_INV_SEGS_EQ;
		break;
	case O_SRH_SEGS_GT:
		srhinfo->mt_flags |= IP6T_SRH_SEGS_GT;
		if (cb->invert)
			srhinfo->mt_invflags |= IP6T_SRH_INV_SEGS_GT;
		break;
	case O_SRH_SEGS_LT:
		srhinfo->mt_flags |= IP6T_SRH_SEGS_LT;
		if (cb->invert)
			srhinfo->mt_invflags |= IP6T_SRH_INV_SEGS_LT;
		break;
	case O_SRH_LAST_EQ:
		srhinfo->mt_flags |= IP6T_SRH_LAST_EQ;
		if (cb->invert)
			srhinfo->mt_invflags |= IP6T_SRH_INV_LAST_EQ;
		break;
	case O_SRH_LAST_GT:
		srhinfo->mt_flags |= IP6T_SRH_LAST_GT;
		if (cb->invert)
			srhinfo->mt_invflags |= IP6T_SRH_INV_LAST_GT;
		break;
	case O_SRH_LAST_LT:
		srhinfo->mt_flags |= IP6T_SRH_LAST_LT;
		if (cb->invert)
			srhinfo->mt_invflags |= IP6T_SRH_INV_LAST_LT;
		break;
	case O_SRH_TAG:
		srhinfo->mt_flags |= IP6T_SRH_TAG;
		if (cb->invert)
			srhinfo->mt_invflags |= IP6T_SRH_INV_TAG;
		break;
	}
}

static void srh1_parse(struct xt_option_call *cb)
{
	struct ip6t_srh1 *srhinfo = cb->data;

	xtables_option_parse(cb);
	switch (cb->entry->id) {
	case O_SRH_NEXTHDR:
		srhinfo->mt_flags |= IP6T_SRH_NEXTHDR;
		if (cb->invert)
			srhinfo->mt_invflags |= IP6T_SRH_INV_NEXTHDR;
		break;
	case O_SRH_LEN_EQ:
		srhinfo->mt_flags |= IP6T_SRH_LEN_EQ;
		if (cb->invert)
			srhinfo->mt_invflags |= IP6T_SRH_INV_LEN_EQ;
		break;
	case O_SRH_LEN_GT:
		srhinfo->mt_flags |= IP6T_SRH_LEN_GT;
		if (cb->invert)
			srhinfo->mt_invflags |= IP6T_SRH_INV_LEN_GT;
		break;
	case O_SRH_LEN_LT:
		srhinfo->mt_flags |= IP6T_SRH_LEN_LT;
		if (cb->invert)
			srhinfo->mt_invflags |= IP6T_SRH_INV_LEN_LT;
		break;
	case O_SRH_SEGS_EQ:
		srhinfo->mt_flags |= IP6T_SRH_SEGS_EQ;
		if (cb->invert)
			srhinfo->mt_invflags |= IP6T_SRH_INV_SEGS_EQ;
		break;
	case O_SRH_SEGS_GT:
		srhinfo->mt_flags |= IP6T_SRH_SEGS_GT;
		if (cb->invert)
			srhinfo->mt_invflags |= IP6T_SRH_INV_SEGS_GT;
		break;
	case O_SRH_SEGS_LT:
		srhinfo->mt_flags |= IP6T_SRH_SEGS_LT;
		if (cb->invert)
			srhinfo->mt_invflags |= IP6T_SRH_INV_SEGS_LT;
		break;
	case O_SRH_LAST_EQ:
		srhinfo->mt_flags |= IP6T_SRH_LAST_EQ;
		if (cb->invert)
			srhinfo->mt_invflags |= IP6T_SRH_INV_LAST_EQ;
		break;
	case O_SRH_LAST_GT:
		srhinfo->mt_flags |= IP6T_SRH_LAST_GT;
		if (cb->invert)
			srhinfo->mt_invflags |= IP6T_SRH_INV_LAST_GT;
		break;
	case O_SRH_LAST_LT:
		srhinfo->mt_flags |= IP6T_SRH_LAST_LT;
		if (cb->invert)
			srhinfo->mt_invflags |= IP6T_SRH_INV_LAST_LT;
		break;
	case O_SRH_TAG:
		srhinfo->mt_flags |= IP6T_SRH_TAG;
		if (cb->invert)
			srhinfo->mt_invflags |= IP6T_SRH_INV_TAG;
		break;
	case O_SRH_PSID:
		srhinfo->mt_flags |= IP6T_SRH_PSID;
		srhinfo->psid_addr = cb->val.haddr.in6;
		srhinfo->psid_msk  = cb->val.hmask.in6;
		if (cb->invert)
			srhinfo->mt_invflags |= IP6T_SRH_INV_PSID;
		break;
	case O_SRH_NSID:
		srhinfo->mt_flags |= IP6T_SRH_NSID;
		srhinfo->nsid_addr = cb->val.haddr.in6;
		srhinfo->nsid_msk  = cb->val.hmask.in6;
		if (cb->invert)
			srhinfo->mt_invflags |= IP6T_SRH_INV_NSID;
		break;
	case O_SRH_LSID:
		srhinfo->mt_flags |= IP6T_SRH_LSID;
		srhinfo->lsid_addr = cb->val.haddr.in6;
		srhinfo->lsid_msk  = cb->val.hmask.in6;
		if (cb->invert)
			srhinfo->mt_invflags |= IP6T_SRH_INV_LSID;
		break;
	}
}

static void srh_print(const void *ip, const struct xt_entry_match *match,
			int numeric)
{
	const struct ip6t_srh *srhinfo = (struct ip6t_srh *)match->data;

	printf(" srh");
	if (srhinfo->mt_flags & IP6T_SRH_NEXTHDR)
		printf(" next-hdr:%s%d", srhinfo->mt_invflags & IP6T_SRH_INV_NEXTHDR ? "!" : "",
			srhinfo->next_hdr);
	if (srhinfo->mt_flags & IP6T_SRH_LEN_EQ)
		printf(" hdr-len-eq:%s%d", srhinfo->mt_invflags & IP6T_SRH_INV_LEN_EQ ? "!" : "",
			srhinfo->hdr_len);
	if (srhinfo->mt_flags & IP6T_SRH_LEN_GT)
		printf(" hdr-len-gt:%s%d", srhinfo->mt_invflags & IP6T_SRH_INV_LEN_GT ? "!" : "",
			srhinfo->hdr_len);
	if (srhinfo->mt_flags & IP6T_SRH_LEN_LT)
		printf(" hdr-len-lt:%s%d", srhinfo->mt_invflags & IP6T_SRH_INV_LEN_LT ? "!" : "",
			srhinfo->hdr_len);
	if (srhinfo->mt_flags & IP6T_SRH_SEGS_EQ)
		printf(" segs-left-eq:%s%d", srhinfo->mt_invflags & IP6T_SRH_INV_SEGS_EQ ? "!" : "",
			srhinfo->segs_left);
	if (srhinfo->mt_flags & IP6T_SRH_SEGS_GT)
		printf(" segs-left-gt:%s%d", srhinfo->mt_invflags & IP6T_SRH_INV_SEGS_GT ? "!" : "",
			srhinfo->segs_left);
	if (srhinfo->mt_flags & IP6T_SRH_SEGS_LT)
		printf(" segs-left-lt:%s%d", srhinfo->mt_invflags & IP6T_SRH_INV_SEGS_LT ? "!" : "",
			srhinfo->segs_left);
	if (srhinfo->mt_flags & IP6T_SRH_LAST_EQ)
		printf(" last-entry-eq:%s%d", srhinfo->mt_invflags & IP6T_SRH_INV_LAST_EQ ? "!" : "",
			srhinfo->last_entry);
	if (srhinfo->mt_flags & IP6T_SRH_LAST_GT)
		printf(" last-entry-gt:%s%d", srhinfo->mt_invflags & IP6T_SRH_INV_LAST_GT ? "!" : "",
			srhinfo->last_entry);
	if (srhinfo->mt_flags & IP6T_SRH_LAST_LT)
		printf(" last-entry-lt:%s%d", srhinfo->mt_invflags & IP6T_SRH_INV_LAST_LT ? "!" : "",
			srhinfo->last_entry);
	if (srhinfo->mt_flags & IP6T_SRH_TAG)
		printf(" tag:%s%d", srhinfo->mt_invflags & IP6T_SRH_INV_TAG ? "!" : "",
			srhinfo->tag);
}

static void srh1_print(const void *ip, const struct xt_entry_match *match, int numeric)
{
	const struct ip6t_srh1 *srhinfo = (struct ip6t_srh1 *)match->data;

	printf(" srh");
	if (srhinfo->mt_flags & IP6T_SRH_NEXTHDR)
		printf(" next-hdr:%s%d", srhinfo->mt_invflags & IP6T_SRH_INV_NEXTHDR ? "!" : "",
			srhinfo->next_hdr);
	if (srhinfo->mt_flags & IP6T_SRH_LEN_EQ)
		printf(" hdr-len-eq:%s%d", srhinfo->mt_invflags & IP6T_SRH_INV_LEN_EQ ? "!" : "",
			srhinfo->hdr_len);
	if (srhinfo->mt_flags & IP6T_SRH_LEN_GT)
		printf(" hdr-len-gt:%s%d", srhinfo->mt_invflags & IP6T_SRH_INV_LEN_GT ? "!" : "",
			srhinfo->hdr_len);
	if (srhinfo->mt_flags & IP6T_SRH_LEN_LT)
		printf(" hdr-len-lt:%s%d", srhinfo->mt_invflags & IP6T_SRH_INV_LEN_LT ? "!" : "",
			srhinfo->hdr_len);
	if (srhinfo->mt_flags & IP6T_SRH_SEGS_EQ)
		printf(" segs-left-eq:%s%d", srhinfo->mt_invflags & IP6T_SRH_INV_SEGS_EQ ? "!" : "",
			srhinfo->segs_left);
	if (srhinfo->mt_flags & IP6T_SRH_SEGS_GT)
		printf(" segs-left-gt:%s%d", srhinfo->mt_invflags & IP6T_SRH_INV_SEGS_GT ? "!" : "",
			srhinfo->segs_left);
	if (srhinfo->mt_flags & IP6T_SRH_SEGS_LT)
		printf(" segs-left-lt:%s%d", srhinfo->mt_invflags & IP6T_SRH_INV_SEGS_LT ? "!" : "",
			srhinfo->segs_left);
	if (srhinfo->mt_flags & IP6T_SRH_LAST_EQ)
		printf(" last-entry-eq:%s%d", srhinfo->mt_invflags & IP6T_SRH_INV_LAST_EQ ? "!" : "",
			srhinfo->last_entry);
	if (srhinfo->mt_flags & IP6T_SRH_LAST_GT)
		printf(" last-entry-gt:%s%d", srhinfo->mt_invflags & IP6T_SRH_INV_LAST_GT ? "!" : "",
			srhinfo->last_entry);
	if (srhinfo->mt_flags & IP6T_SRH_LAST_LT)
		printf(" last-entry-lt:%s%d", srhinfo->mt_invflags & IP6T_SRH_INV_LAST_LT ? "!" : "",
			srhinfo->last_entry);
	if (srhinfo->mt_flags & IP6T_SRH_TAG)
		printf(" tag:%s%d", srhinfo->mt_invflags & IP6T_SRH_INV_TAG ? "!" : "",
			srhinfo->tag);
	if (srhinfo->mt_flags & IP6T_SRH_PSID)
		printf(" psid %s %s/%u", srhinfo->mt_invflags & IP6T_SRH_INV_PSID ? "!" : "",
			xtables_ip6addr_to_numeric(&srhinfo->psid_addr),
			xtables_ip6mask_to_cidr(&srhinfo->psid_msk));
	if (srhinfo->mt_flags & IP6T_SRH_NSID)
		printf(" nsid %s %s/%u", srhinfo->mt_invflags & IP6T_SRH_INV_NSID ? "!" : "",
			xtables_ip6addr_to_numeric(&srhinfo->nsid_addr),
			xtables_ip6mask_to_cidr(&srhinfo->nsid_msk));
	if (srhinfo->mt_flags & IP6T_SRH_LSID)
		printf(" lsid %s %s/%u", srhinfo->mt_invflags & IP6T_SRH_INV_LSID ? "!" : "",
			xtables_ip6addr_to_numeric(&srhinfo->lsid_addr),
			xtables_ip6mask_to_cidr(&srhinfo->lsid_msk));
}

static void srh_save(const void *ip, const struct xt_entry_match *match)
{
	const struct ip6t_srh *srhinfo = (struct ip6t_srh *)match->data;

	if (srhinfo->mt_flags & IP6T_SRH_NEXTHDR)
		printf("%s --srh-next-hdr %u", (srhinfo->mt_invflags & IP6T_SRH_INV_NEXTHDR) ? " !" : "",
			srhinfo->next_hdr);
	if (srhinfo->mt_flags & IP6T_SRH_LEN_EQ)
		printf("%s --srh-hdr-len-eq %u", (srhinfo->mt_invflags & IP6T_SRH_INV_LEN_EQ) ? " !" : "",
			srhinfo->hdr_len);
	if (srhinfo->mt_flags & IP6T_SRH_LEN_GT)
		printf("%s --srh-hdr-len-gt %u", (srhinfo->mt_invflags & IP6T_SRH_INV_LEN_GT) ? " !" : "",
			srhinfo->hdr_len);
	if (srhinfo->mt_flags & IP6T_SRH_LEN_LT)
		printf("%s --srh-hdr-len-lt %u", (srhinfo->mt_invflags & IP6T_SRH_INV_LEN_LT) ? " !" : "",
			srhinfo->hdr_len);
	if (srhinfo->mt_flags & IP6T_SRH_SEGS_EQ)
		printf("%s --srh-segs-left-eq %u", (srhinfo->mt_invflags & IP6T_SRH_INV_SEGS_EQ) ? " !" : "",
			srhinfo->segs_left);
	if (srhinfo->mt_flags & IP6T_SRH_SEGS_GT)
		printf("%s --srh-segs-left-gt %u", (srhinfo->mt_invflags & IP6T_SRH_INV_SEGS_GT) ? " !" : "",
			srhinfo->segs_left);
	if (srhinfo->mt_flags & IP6T_SRH_SEGS_LT)
		printf("%s --srh-segs-left-lt %u", (srhinfo->mt_invflags & IP6T_SRH_INV_SEGS_LT) ? " !" : "",
			srhinfo->segs_left);
	if (srhinfo->mt_flags & IP6T_SRH_LAST_EQ)
		printf("%s --srh-last-entry-eq %u", (srhinfo->mt_invflags & IP6T_SRH_INV_LAST_EQ) ? " !" : "",
			srhinfo->last_entry);
	if (srhinfo->mt_flags & IP6T_SRH_LAST_GT)
		printf("%s --srh-last-entry-gt %u", (srhinfo->mt_invflags & IP6T_SRH_INV_LAST_GT) ? " !" : "",
			srhinfo->last_entry);
	if (srhinfo->mt_flags & IP6T_SRH_LAST_LT)
		printf("%s --srh-last-entry-lt %u", (srhinfo->mt_invflags & IP6T_SRH_INV_LAST_LT) ? " !" : "",
			srhinfo->last_entry);
	if (srhinfo->mt_flags & IP6T_SRH_TAG)
		printf("%s --srh-tag %u", (srhinfo->mt_invflags & IP6T_SRH_INV_TAG) ? " !" : "",
			srhinfo->tag);
}

static void srh1_save(const void *ip, const struct xt_entry_match *match)
{
	const struct ip6t_srh1 *srhinfo = (struct ip6t_srh1 *)match->data;

	if (srhinfo->mt_flags & IP6T_SRH_NEXTHDR)
		printf("%s --srh-next-hdr %u", (srhinfo->mt_invflags & IP6T_SRH_INV_NEXTHDR) ? " !" : "",
			srhinfo->next_hdr);
	if (srhinfo->mt_flags & IP6T_SRH_LEN_EQ)
		printf("%s --srh-hdr-len-eq %u", (srhinfo->mt_invflags & IP6T_SRH_INV_LEN_EQ) ? " !" : "",
			srhinfo->hdr_len);
	if (srhinfo->mt_flags & IP6T_SRH_LEN_GT)
		printf("%s --srh-hdr-len-gt %u", (srhinfo->mt_invflags & IP6T_SRH_INV_LEN_GT) ? " !" : "",
			srhinfo->hdr_len);
	if (srhinfo->mt_flags & IP6T_SRH_LEN_LT)
		printf("%s --srh-hdr-len-lt %u", (srhinfo->mt_invflags & IP6T_SRH_INV_LEN_LT) ? " !" : "",
			srhinfo->hdr_len);
	if (srhinfo->mt_flags & IP6T_SRH_SEGS_EQ)
		printf("%s --srh-segs-left-eq %u", (srhinfo->mt_invflags & IP6T_SRH_INV_SEGS_EQ) ? " !" : "",
			srhinfo->segs_left);
	if (srhinfo->mt_flags & IP6T_SRH_SEGS_GT)
		printf("%s --srh-segs-left-gt %u", (srhinfo->mt_invflags & IP6T_SRH_INV_SEGS_GT) ? " !" : "",
			srhinfo->segs_left);
	if (srhinfo->mt_flags & IP6T_SRH_SEGS_LT)
		printf("%s --srh-segs-left-lt %u", (srhinfo->mt_invflags & IP6T_SRH_INV_SEGS_LT) ? " !" : "",
			srhinfo->segs_left);
	if (srhinfo->mt_flags & IP6T_SRH_LAST_EQ)
		printf("%s --srh-last-entry-eq %u", (srhinfo->mt_invflags & IP6T_SRH_INV_LAST_EQ) ? " !" : "",
			srhinfo->last_entry);
	if (srhinfo->mt_flags & IP6T_SRH_LAST_GT)
		printf("%s --srh-last-entry-gt %u", (srhinfo->mt_invflags & IP6T_SRH_INV_LAST_GT) ? " !" : "",
			srhinfo->last_entry);
	if (srhinfo->mt_flags & IP6T_SRH_LAST_LT)
		printf("%s --srh-last-entry-lt %u", (srhinfo->mt_invflags & IP6T_SRH_INV_LAST_LT) ? " !" : "",
			srhinfo->last_entry);
	if (srhinfo->mt_flags & IP6T_SRH_TAG)
		printf("%s --srh-tag %u", (srhinfo->mt_invflags & IP6T_SRH_INV_TAG) ? " !" : "",
			srhinfo->tag);
	if (srhinfo->mt_flags & IP6T_SRH_PSID)
		printf("%s --srh-psid %s/%u", srhinfo->mt_invflags & IP6T_SRH_INV_PSID ? " !" : "",
			xtables_ip6addr_to_numeric(&srhinfo->psid_addr),
			xtables_ip6mask_to_cidr(&srhinfo->psid_msk));
	if (srhinfo->mt_flags & IP6T_SRH_NSID)
		printf("%s --srh-nsid %s/%u", srhinfo->mt_invflags & IP6T_SRH_INV_NSID ? " !" : "",
			xtables_ip6addr_to_numeric(&srhinfo->nsid_addr),
			xtables_ip6mask_to_cidr(&srhinfo->nsid_msk));
	if (srhinfo->mt_flags & IP6T_SRH_LSID)
		printf("%s --srh-lsid %s/%u", srhinfo->mt_invflags & IP6T_SRH_INV_LSID ? " !" : "",
			xtables_ip6addr_to_numeric(&srhinfo->lsid_addr),
			xtables_ip6mask_to_cidr(&srhinfo->lsid_msk));
}

static struct xtables_match srh_mt6_reg[] = {
	{
		.name		= "srh",
		.version	= XTABLES_VERSION,
		.revision	= 0,
		.family		= NFPROTO_IPV6,
		.size		= XT_ALIGN(sizeof(struct ip6t_srh)),
		.userspacesize	= XT_ALIGN(sizeof(struct ip6t_srh)),
		.help		= srh_help,
		.init		= srh_init,
		.print		= srh_print,
		.save		= srh_save,
		.x6_parse	= srh_parse,
		.x6_options	= srh_opts,
	},
	{
		.name		= "srh",
		.version	= XTABLES_VERSION,
		.revision	= 1,
		.family		= NFPROTO_IPV6,
		.size		= XT_ALIGN(sizeof(struct ip6t_srh1)),
		.userspacesize	= XT_ALIGN(sizeof(struct ip6t_srh1)),
		.help		= srh_help,
		.init		= srh1_init,
		.print		= srh1_print,
		.save		= srh1_save,
		.x6_parse	= srh1_parse,
		.x6_options	= srh1_opts,
	}
};

void
_init(void)
{
	xtables_register_matches(srh_mt6_reg, ARRAY_SIZE(srh_mt6_reg));
}
