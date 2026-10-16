#include <stdio.h>
#include <xtables.h>
#include <linux/netfilter_ipv6/ip6t_ah.h>

enum {
	O_AHSPI = 0,
	O_AHLEN,
	O_AHRES,
};

static void ah_help(void)
{
	printf(
"ah match options:\n"
"[!] --ahspi spi[:spi]          match spi (range)\n"
"[!] --ahlen length             total length of this header\n"
" --ahres                       check the reserved field too\n");
}

#define s struct ip6t_ah
static const struct xt_option_entry ah_opts[] = {
	{.name = "ahspi", .id = O_AHSPI, .type = XTTYPE_UINT32RC,
	 .flags = XTOPT_INVERT | XTOPT_PUT, XTOPT_POINTER(s, spis)},
	{.name = "ahlen", .id = O_AHLEN, .type = XTTYPE_UINT32,
	 .flags = XTOPT_INVERT | XTOPT_PUT, XTOPT_POINTER(s, hdrlen)},
	{.name = "ahres", .id = O_AHRES, .type = XTTYPE_NONE},
	XTOPT_TABLEEND,
};
#undef s

static void ah_init(struct xt_entry_match *m)
{
	struct ip6t_ah *ahinfo = (void *)m->data;

	/* Defaults for when no --ahspi is used at all */
	ahinfo->spis[1] = ~0U;
}

static void ah_parse(struct xt_option_call *cb)
{
	struct ip6t_ah *ahinfo = cb->data;

	xtables_option_parse(cb);
	switch (cb->entry->id) {
	case O_AHSPI:
		if (cb->nvals == 1)
			ahinfo->spis[1] = ahinfo->spis[0];
		if (cb->invert)
			ahinfo->invflags |= IP6T_AH_INV_SPI;
		break;
	case O_AHLEN:
		if (cb->invert)
			ahinfo->invflags |= IP6T_AH_INV_LEN;
		break;
	case O_AHRES:
		ahinfo->hdrres = 1;
		break;
	}
}

static bool skip_spi_match(uint32_t min, uint32_t max, bool inv)
{
	return min == 0 && max == UINT32_MAX && !inv;
}

static void
print_spis(const char *name, uint32_t min, uint32_t max,
	    int invert)
{
	const char *inv = invert ? "!" : "";

	if (!skip_spi_match(min, max, invert)) {
		if (min == max)
			printf("%s:%s%u", name, inv, min);
		else
			printf("%ss:%s%u:%u", name, inv, min, max);
	}
}

static void
print_len(const char *name, uint32_t len, int invert)
{
	const char *inv = invert ? "!" : "";

	if (len != 0 || invert)
		printf("%s:%s%u", name, inv, len);
}

static void ah_print(const void *ip, const struct xt_entry_match *match,
                     int numeric)
{
	const struct ip6t_ah *ah = (struct ip6t_ah *)match->data;

	printf(" ah ");
	print_spis("spi", ah->spis[0], ah->spis[1],
		    ah->invflags & IP6T_AH_INV_SPI);
	print_len("length", ah->hdrlen, 
		    ah->invflags & IP6T_AH_INV_LEN);

	if (ah->hdrres)
		printf(" reserved");

	if (ah->invflags & ~IP6T_AH_INV_MASK)
		printf(" Unknown invflags: 0x%X",
		       ah->invflags & ~IP6T_AH_INV_MASK);
}

static void ah_save(const void *ip, const struct xt_entry_match *match)
{
	const struct ip6t_ah *ahinfo = (struct ip6t_ah *)match->data;
	bool inv_spi = ahinfo->invflags & IP6T_AH_INV_SPI;

	if (!skip_spi_match(ahinfo->spis[0], ahinfo->spis[1], inv_spi)) {
		printf("%s --ahspi ", inv_spi ? " !" : "");
		if (ahinfo->spis[0]
		    != ahinfo->spis[1])
			printf("%u:%u",
			       ahinfo->spis[0],
			       ahinfo->spis[1]);
		else
			printf("%u",
			       ahinfo->spis[0]);
	}

	if (ahinfo->hdrlen != 0 || (ahinfo->invflags & IP6T_AH_INV_LEN) ) {
		printf("%s --ahlen %u",
			(ahinfo->invflags & IP6T_AH_INV_LEN) ? " !" : "",
			ahinfo->hdrlen);
	}

	if (ahinfo->hdrres != 0 )
		printf(" --ahres");
}

static int ah_xlate(struct xt_xlate *xl,
		    const struct xt_xlate_mt_params *params)
{
	const struct ip6t_ah *ahinfo = (struct ip6t_ah *)params->match->data;
	bool inv_spi = ahinfo->invflags & IP6T_AH_INV_SPI;
	char *space = "";

	if (!skip_spi_match(ahinfo->spis[0], ahinfo->spis[1], inv_spi)) {
		xt_xlate_add(xl, "ah spi%s ", inv_spi ? " !=" : "");
		if (ahinfo->spis[0] != ahinfo->spis[1])
			xt_xlate_add(xl, "%u-%u", ahinfo->spis[0],
				     ahinfo->spis[1]);
		else
			xt_xlate_add(xl, "%u", ahinfo->spis[0]);
		space = " ";
	}

	if (ahinfo->hdrlen != 0 || (ahinfo->invflags & IP6T_AH_INV_LEN)) {
		xt_xlate_add(xl, "%sah hdrlength%s %u", space,
			     (ahinfo->invflags & IP6T_AH_INV_LEN) ? " !=" : "",
			     ahinfo->hdrlen);
		space = " ";
	}

	if (ahinfo->hdrres != 0) {
		xt_xlate_add(xl, "%sah reserved %u", space, ahinfo->hdrres);
		space = " ";
	}

	if (!space[0]) /* plain '-m ah' */
		xt_xlate_add(xl, "exthdr ah exists");

	return 1;
}

static struct xtables_match ah_mt6_reg = {
	.name          = "ah",
	.version       = XTABLES_VERSION,
	.family        = NFPROTO_IPV6,
	.size          = XT_ALIGN(sizeof(struct ip6t_ah)),
	.userspacesize = XT_ALIGN(sizeof(struct ip6t_ah)),
	.help          = ah_help,
	.init          = ah_init,
	.print         = ah_print,
	.save          = ah_save,
	.x6_parse      = ah_parse,
	.x6_options    = ah_opts,
	.xlate	       = ah_xlate,
};

void
_init(void)
{
	xtables_register_match(&ah_mt6_reg);
}
