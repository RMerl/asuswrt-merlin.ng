#include <stdio.h>
#include <xtables.h>
#include <linux/netfilter_ipv4/ipt_ah.h>

enum {
	O_AHSPI = 0,
};

static void ah_help(void)
{
	printf(
"ah match options:\n"
"[!] --ahspi spi[:spi]\n"
"				match spi (range)\n");
}

static const struct xt_option_entry ah_opts[] = {
	{.name = "ahspi", .id = O_AHSPI, .type = XTTYPE_UINT32RC,
	 .flags = XTOPT_INVERT | XTOPT_PUT,
	 XTOPT_POINTER(struct ipt_ah, spis)},
	XTOPT_TABLEEND,
};

static void ah_init(struct xt_entry_match *m)
{
	struct ipt_ah *ahinfo = (void *)m->data;

	ahinfo->spis[1] = ~0U;
}

static void ah_parse(struct xt_option_call *cb)
{
	struct ipt_ah *ahinfo = cb->data;

	xtables_option_parse(cb);
	if (cb->nvals == 1)
		ahinfo->spis[1] = ahinfo->spis[0];
	if (cb->invert)
		ahinfo->invflags |= IPT_AH_INV_SPI;
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
		printf("%s", name);
		if (min == max) {
			printf(":%s", inv);
			printf("%u", min);
		} else {
			printf("s:%s", inv);
			printf("%u",min);
			printf(":");
			printf("%u",max);
		}
	}
}

static void ah_print(const void *ip, const struct xt_entry_match *match,
                     int numeric)
{
	const struct ipt_ah *ah = (struct ipt_ah *)match->data;

	printf(" ah ");
	print_spis("spi", ah->spis[0], ah->spis[1],
		    ah->invflags & IPT_AH_INV_SPI);
	if (ah->invflags & ~IPT_AH_INV_MASK)
		printf(" Unknown invflags: 0x%X",
		       ah->invflags & ~IPT_AH_INV_MASK);
}

static void ah_save(const void *ip, const struct xt_entry_match *match)
{
	const struct ipt_ah *ahinfo = (struct ipt_ah *)match->data;
	bool inv_spi = ahinfo->invflags & IPT_AH_INV_SPI;

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

}

static int ah_xlate(struct xt_xlate *xl,
		    const struct xt_xlate_mt_params *params)
{
	const struct ipt_ah *ahinfo = (struct ipt_ah *)params->match->data;
	bool inv_spi = ahinfo->invflags & IPT_AH_INV_SPI;

	if (!skip_spi_match(ahinfo->spis[0], ahinfo->spis[1], inv_spi)) {
		xt_xlate_add(xl, "ah spi%s ", inv_spi ? " !=" : "");
		if (ahinfo->spis[0] != ahinfo->spis[1])
			xt_xlate_add(xl, "%u-%u", ahinfo->spis[0],
				   ahinfo->spis[1]);
		else
			xt_xlate_add(xl, "%u", ahinfo->spis[0]);
	} else {
		xt_xlate_add(xl, "meta l4proto ah");
	}

	return 1;
}

static struct xtables_match ah_mt_reg = {
	.name		= "ah",
	.version	= XTABLES_VERSION,
	.family		= NFPROTO_IPV4,
	.size		= XT_ALIGN(sizeof(struct ipt_ah)),
	.userspacesize	= XT_ALIGN(sizeof(struct ipt_ah)),
	.help		= ah_help,
	.init		= ah_init,
	.print		= ah_print,
	.save		= ah_save,
	.x6_parse	= ah_parse,
	.x6_options	= ah_opts,
	.xlate		= ah_xlate,
};

void
_init(void)
{
	xtables_register_match(&ah_mt_reg);
}
