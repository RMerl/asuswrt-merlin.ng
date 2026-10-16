#include <stdio.h>
#include <xtables.h>
#include <linux/netfilter/xt_esp.h>

enum {
	O_ESPSPI = 0,
};

static void esp_help(void)
{
	printf(
"esp match options:\n"
"[!] --espspi spi[:spi]\n"
"				match spi (range)\n");
}

static const struct xt_option_entry esp_opts[] = {
	{.name = "espspi", .id = O_ESPSPI, .type = XTTYPE_UINT32RC,
	 .flags = XTOPT_INVERT | XTOPT_PUT,
	 XTOPT_POINTER(struct xt_esp, spis)},
	XTOPT_TABLEEND,
};

static void esp_init(struct xt_entry_match *m)
{
	struct xt_esp *espinfo = (void *)m->data;

	espinfo->spis[1] = ~0U;
}

static void esp_parse(struct xt_option_call *cb)
{
	struct xt_esp *espinfo = cb->data;

	xtables_option_parse(cb);
	if (cb->nvals == 1)
		espinfo->spis[1] = espinfo->spis[0];
	if (cb->invert)
		espinfo->invflags |= XT_ESP_INV_SPI;
}

static bool skip_spis_match(uint32_t min, uint32_t max, bool inv)
{
	return min == 0 && max == UINT32_MAX && !inv;
}

static void
print_spis(const char *name, uint32_t min, uint32_t max,
	    int invert)
{
	const char *inv = invert ? "!" : "";

	if (!skip_spis_match(min, max, invert)) {
		if (min == max)
			printf(" %s:%s%u", name, inv, min);
		else
			printf(" %ss:%s%u:%u", name, inv, min, max);
	}
}

static void
esp_print(const void *ip, const struct xt_entry_match *match, int numeric)
{
	const struct xt_esp *esp = (struct xt_esp *)match->data;

	printf(" esp");
	print_spis("spi", esp->spis[0], esp->spis[1],
		    esp->invflags & XT_ESP_INV_SPI);
	if (esp->invflags & ~XT_ESP_INV_MASK)
		printf(" Unknown invflags: 0x%X",
		       esp->invflags & ~XT_ESP_INV_MASK);
}

static void esp_save(const void *ip, const struct xt_entry_match *match)
{
	const struct xt_esp *espinfo = (struct xt_esp *)match->data;
	bool inv_spi = espinfo->invflags & XT_ESP_INV_SPI;

	if (!skip_spis_match(espinfo->spis[0], espinfo->spis[1], inv_spi)) {
		printf("%s --espspi ", inv_spi ? " !" : "");
		if (espinfo->spis[0]
		    != espinfo->spis[1])
			printf("%u:%u",
			       espinfo->spis[0],
			       espinfo->spis[1]);
		else
			printf("%u",
			       espinfo->spis[0]);
	}

}

static int esp_xlate(struct xt_xlate *xl,
		     const struct xt_xlate_mt_params *params)
{
	const struct xt_esp *espinfo = (struct xt_esp *)params->match->data;
	bool inv_spi = espinfo->invflags & XT_ESP_INV_SPI;

	if (!skip_spis_match(espinfo->spis[0], espinfo->spis[1], inv_spi)) {
		xt_xlate_add(xl, "esp spi%s", inv_spi ? " !=" : "");
		if (espinfo->spis[0] != espinfo->spis[1])
			xt_xlate_add(xl, " %u-%u", espinfo->spis[0],
				   espinfo->spis[1]);
		else
			xt_xlate_add(xl, " %u", espinfo->spis[0]);
	} else if (afinfo->family == NFPROTO_IPV4) {
		xt_xlate_add(xl, "meta l4proto esp");
	} else if (afinfo->family == NFPROTO_IPV6) {
		xt_xlate_add(xl, "exthdr esp exists");
	} else {
		return 0;
	}

	return 1;
}

static struct xtables_match esp_match = {
	.family		= NFPROTO_UNSPEC,
	.name		= "esp",
	.version	= XTABLES_VERSION,
	.size		= XT_ALIGN(sizeof(struct xt_esp)),
	.userspacesize	= XT_ALIGN(sizeof(struct xt_esp)),
	.help		= esp_help,
	.init		= esp_init,
	.print		= esp_print,
	.save		= esp_save,
	.x6_parse	= esp_parse,
	.x6_options	= esp_opts,
	.xlate		= esp_xlate,
};

void
_init(void)
{
	xtables_register_match(&esp_match);
}
