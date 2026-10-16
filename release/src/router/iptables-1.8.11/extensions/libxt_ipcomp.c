#include <stdio.h>
#include <xtables.h>
#include <linux/netfilter/xt_ipcomp.h>

enum {
	O_compSPI = 0,
	O_compRES,
};

static void comp_help(void)
{
	printf(
"comp match options:\n"
"[!] --ipcompspi spi[:spi]\n"
"				match spi (range)\n");
}

static const struct xt_option_entry comp_opts[] = {
	{.name = "ipcompspi", .id = O_compSPI, .type = XTTYPE_UINT32RC,
	 .flags = XTOPT_INVERT | XTOPT_PUT,
	 XTOPT_POINTER(struct xt_ipcomp, spis)},
	{.name = "compres", .id = O_compRES, .type = XTTYPE_NONE},
	XTOPT_TABLEEND,
};
#undef s

static void comp_parse(struct xt_option_call *cb)
{
	struct xt_ipcomp *compinfo = cb->data;

	xtables_option_parse(cb);
	switch (cb->entry->id) {
	case O_compSPI:
		if (cb->nvals == 1)
			compinfo->spis[1] = compinfo->spis[0];
		if (cb->invert)
			compinfo->invflags |= XT_IPCOMP_INV_SPI;
		break;
	case O_compRES:
		compinfo->hdrres = 1;
		break;
	}
}

static void
print_spis(const char *name, uint32_t min, uint32_t max,
	    int invert)
{
	const char *inv = invert ? "!" : "";

	if (min != 0 || max != 0xFFFFFFFF || invert) {
		if (min == max)
			printf("%s:%s%u", name, inv, min);
		else
			printf("%ss:%s%u:%u", name, inv, min, max);
	}
}

static void comp_print(const void *ip, const struct xt_entry_match *match,
                     int numeric)
{
	const struct xt_ipcomp *comp = (struct xt_ipcomp *)match->data;

	printf(" comp ");
	print_spis("spi", comp->spis[0], comp->spis[1],
		    comp->invflags & XT_IPCOMP_INV_SPI);

	if (comp->hdrres)
		printf(" reserved");

	if (comp->invflags & ~XT_IPCOMP_INV_MASK)
		printf(" Unknown invflags: 0x%X",
		       comp->invflags & ~XT_IPCOMP_INV_MASK);
}

static void comp_save(const void *ip, const struct xt_entry_match *match)
{
	const struct xt_ipcomp *compinfo = (struct xt_ipcomp *)match->data;
	bool inv_spi = compinfo->invflags & XT_IPCOMP_INV_SPI;

	if (!(compinfo->spis[0] == 0
	    && compinfo->spis[1] == UINT32_MAX
	    && !inv_spi)) {
		printf("%s --ipcompspi ", inv_spi ? " !" : "");
		if (compinfo->spis[0]
		    != compinfo->spis[1])
			printf("%u:%u",
			       compinfo->spis[0],
			       compinfo->spis[1]);
		else
			printf("%u",
			       compinfo->spis[0]);
	}

	if (compinfo->hdrres != 0 )
		printf(" --compres");
}

static int comp_xlate(struct xt_xlate *xl,
		      const struct xt_xlate_mt_params *params)
{
	const struct xt_ipcomp *compinfo =
		(struct xt_ipcomp *)params->match->data;

	/* ignore compinfo->hdrres like kernel's xt_ipcomp.c does */

	xt_xlate_add(xl, "comp cpi %s",
		     compinfo->invflags & XT_IPCOMP_INV_SPI ? "!= " : "");
	if (compinfo->spis[0] != compinfo->spis[1])
		xt_xlate_add(xl, "%u-%u", compinfo->spis[0],
			     compinfo->spis[1]);
	else
		xt_xlate_add(xl, "%u", compinfo->spis[0]);

	return 1;
}

static struct xtables_match comp_mt_reg = {
	.name          = "ipcomp",
	.version       = XTABLES_VERSION,
	.family        = NFPROTO_UNSPEC,
	.size          = XT_ALIGN(sizeof(struct xt_ipcomp)),
	.userspacesize = XT_ALIGN(sizeof(struct xt_ipcomp)),
	.help          = comp_help,
	.print         = comp_print,
	.save          = comp_save,
	.x6_parse      = comp_parse,
	.x6_options    = comp_opts,
	.xlate         = comp_xlate,
};

void
_init(void)
{
	xtables_register_match(&comp_mt_reg);
};

