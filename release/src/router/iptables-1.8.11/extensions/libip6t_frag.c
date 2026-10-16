#include <stdio.h>
#include <xtables.h>
#include <linux/netfilter_ipv6/ip6t_frag.h>

enum {
	O_FRAGID = 0,
	O_FRAGLEN,
	O_FRAGRES,
	O_FRAGFIRST,
	O_FRAGMORE,
	O_FRAGLAST,
	F_FRAGMORE = 1 << O_FRAGMORE,
	F_FRAGLAST = 1 << O_FRAGLAST,
};

static void frag_help(void)
{
	printf(
"frag match options:\n"
"[!] --fragid id[:id]           match the id (range)\n"
"[!] --fraglen length           total length of this header\n"
" --fragres                     check the reserved field too\n"
" --fragfirst                   matches on the first fragment\n"
" [--fragmore|--fraglast]       there are more fragments or this\n"
"                               is the last one\n");
}

#define s struct ip6t_frag
static const struct xt_option_entry frag_opts[] = {
	{.name = "fragid", .id = O_FRAGID, .type = XTTYPE_UINT32RC,
	 .flags = XTOPT_INVERT | XTOPT_PUT, XTOPT_POINTER(s, ids)},
	{.name = "fraglen", .id = O_FRAGLEN, .type = XTTYPE_UINT32,
	 .flags = XTOPT_INVERT | XTOPT_PUT, XTOPT_POINTER(s, hdrlen)},
	{.name = "fragres", .id = O_FRAGRES, .type = XTTYPE_NONE},
	{.name = "fragfirst", .id = O_FRAGFIRST, .type = XTTYPE_NONE},
	{.name = "fragmore", .id = O_FRAGMORE, .type = XTTYPE_NONE,
	 .excl = F_FRAGLAST},
	{.name = "fraglast", .id = O_FRAGLAST, .type = XTTYPE_NONE,
	 .excl = F_FRAGMORE},
	XTOPT_TABLEEND,
};
#undef s

static void frag_init(struct xt_entry_match *m)
{
	struct ip6t_frag *fraginfo = (void *)m->data;

	fraginfo->ids[1] = ~0U;
}

static void frag_parse(struct xt_option_call *cb)
{
	struct ip6t_frag *fraginfo = cb->data;

	xtables_option_parse(cb);
	switch (cb->entry->id) {
	case O_FRAGID:
		if (cb->nvals == 1)
			fraginfo->ids[1] = fraginfo->ids[0];
		if (cb->invert)
			fraginfo->invflags |= IP6T_FRAG_INV_IDS;
		/*
		 * Note however that IP6T_FRAG_IDS is not tested by anything,
		 * so it is merely here for completeness.
		 */
		fraginfo->flags |= IP6T_FRAG_IDS;
		break;
	case O_FRAGLEN:
		/*
		 * As of Linux 3.0, the kernel does not check for
		 * fraglen at all.
		 */
		if (cb->invert)
			fraginfo->invflags |= IP6T_FRAG_INV_LEN;
		fraginfo->flags |= IP6T_FRAG_LEN;
		break;
	case O_FRAGRES:
		fraginfo->flags |= IP6T_FRAG_RES;
		break;
	case O_FRAGFIRST:
		fraginfo->flags |= IP6T_FRAG_FST;
		break;
	case O_FRAGMORE:
		fraginfo->flags |= IP6T_FRAG_MF;
		break;
	case O_FRAGLAST:
		fraginfo->flags |= IP6T_FRAG_NMF;
		break;
	}
}

static bool skip_ids_match(uint32_t min, uint32_t max, bool inv)
{
	return min == 0 && max == UINT32_MAX && !inv;
}

static void
print_ids(const char *name, uint32_t min, uint32_t max,
	    int invert)
{
	const char *inv = invert ? "!" : "";

	if (!skip_ids_match(min, max, invert)) {
		printf("%s", name);
		if (min == max)
			printf(":%s%u", inv, min);
		else
			printf("s:%s%u:%u", inv, min, max);
	}
}

static void frag_print(const void *ip, const struct xt_entry_match *match,
                       int numeric)
{
	const struct ip6t_frag *frag = (struct ip6t_frag *)match->data;

	printf(" frag ");
	print_ids("id", frag->ids[0], frag->ids[1],
		    frag->invflags & IP6T_FRAG_INV_IDS);

	if (frag->flags & IP6T_FRAG_LEN) {
		printf(" length:%s%u",
			frag->invflags & IP6T_FRAG_INV_LEN ? "!" : "",
			frag->hdrlen);
	}

	if (frag->flags & IP6T_FRAG_RES)
		printf(" reserved");

	if (frag->flags & IP6T_FRAG_FST)
		printf(" first");

	if (frag->flags & IP6T_FRAG_MF)
		printf(" more");

	if (frag->flags & IP6T_FRAG_NMF)
		printf(" last");

	if (frag->invflags & ~IP6T_FRAG_INV_MASK)
		printf(" Unknown invflags: 0x%X",
		       frag->invflags & ~IP6T_FRAG_INV_MASK);
}

static void frag_save(const void *ip, const struct xt_entry_match *match)
{
	const struct ip6t_frag *fraginfo = (struct ip6t_frag *)match->data;
	bool inv_ids = fraginfo->invflags & IP6T_FRAG_INV_IDS;

	if (!skip_ids_match(fraginfo->ids[0], fraginfo->ids[1], inv_ids)) {
		printf("%s --fragid ", inv_ids ? " !" : "");
		if (fraginfo->ids[0]
		    != fraginfo->ids[1])
			printf("%u:%u",
			       fraginfo->ids[0],
			       fraginfo->ids[1]);
		else
			printf("%u",
			       fraginfo->ids[0]);
	}

	if (fraginfo->flags & IP6T_FRAG_LEN) {
		printf("%s --fraglen %u",
			(fraginfo->invflags & IP6T_FRAG_INV_LEN) ? " !" : "",
			fraginfo->hdrlen);
	}

	if (fraginfo->flags & IP6T_FRAG_RES)
		printf(" --fragres");

	if (fraginfo->flags & IP6T_FRAG_FST)
		printf(" --fragfirst");

	if (fraginfo->flags & IP6T_FRAG_MF)
		printf(" --fragmore");

	if (fraginfo->flags & IP6T_FRAG_NMF)
		printf(" --fraglast");
}

#define XLATE_FLAGS (IP6T_FRAG_RES | IP6T_FRAG_FST | \
		     IP6T_FRAG_MF | IP6T_FRAG_NMF)

static int frag_xlate(struct xt_xlate *xl,
		      const struct xt_xlate_mt_params *params)
{
	const struct ip6t_frag *fraginfo =
		(struct ip6t_frag *)params->match->data;
	bool inv_ids = fraginfo->invflags & IP6T_FRAG_INV_IDS;

	if (!skip_ids_match(fraginfo->ids[0], fraginfo->ids[1], inv_ids)) {
		xt_xlate_add(xl, "frag id %s", inv_ids ?  "!= " : "");
		if (fraginfo->ids[0] != fraginfo->ids[1])
			xt_xlate_add(xl, "%u-%u", fraginfo->ids[0],
				     fraginfo->ids[1]);
		else
			xt_xlate_add(xl, "%u", fraginfo->ids[0]);

	} else if (!(fraginfo->flags & XLATE_FLAGS)) {
		xt_xlate_add(xl, "exthdr frag exists");
		return 1;
	}

	/* ignore ineffective IP6T_FRAG_LEN bit */

	if (fraginfo->flags & IP6T_FRAG_RES)
		xt_xlate_add(xl, "frag reserved 1");

	if (fraginfo->flags & IP6T_FRAG_FST)
		xt_xlate_add(xl, "frag frag-off 0");

	if (fraginfo->flags & IP6T_FRAG_MF)
		xt_xlate_add(xl, "frag more-fragments 1");

	if (fraginfo->flags & IP6T_FRAG_NMF)
		xt_xlate_add(xl, "frag more-fragments 0");

	return 1;
}

static struct xtables_match frag_mt6_reg = {
	.name          = "frag",
	.version       = XTABLES_VERSION,
	.family        = NFPROTO_IPV6,
	.size          = XT_ALIGN(sizeof(struct ip6t_frag)),
	.userspacesize = XT_ALIGN(sizeof(struct ip6t_frag)),
	.help          = frag_help,
	.init          = frag_init,
	.print         = frag_print,
	.save          = frag_save,
	.x6_parse      = frag_parse,
	.x6_options    = frag_opts,
	.xlate	       = frag_xlate,
};

void
_init(void)
{
	xtables_register_match(&frag_mt6_reg);
}
