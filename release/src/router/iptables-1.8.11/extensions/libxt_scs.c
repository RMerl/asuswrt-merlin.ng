#include <stdio.h>
#include <xtables.h>
#include <linux/netfilter/xt_scs.h>

enum {
	O_ESPSPI = 0,
	O_UP_SET,
	O_IS_UP_SET,
	O_UDPESP_PORT,
};

static void scs_help(void)
{
	printf(
"scs match options:\n"
"    [!] --espspi spi[/mask]	match spi (mask)\n"
"        --udpesp <value>       match UDP L4 ports\n"
"    [!] --up-is-set            match user priority already set\n"
"        --up-set <value>       override user priority\n");
}

static const struct xt_option_entry scs_opts[] = {
	{.name = "espspi",    .id = O_ESPSPI,    .type = XTTYPE_MARKMASK32, .flags = XTOPT_INVERT},
	{.name = "udpesp",    .id = O_UDPESP_PORT,.type= XTTYPE_UINT16, },
	{.name = "up-is-set", .id = O_IS_UP_SET, .type = XTTYPE_NONE, .flags = XTOPT_INVERT},
	{.name = "up-set",    .id = O_UP_SET,    .type = XTTYPE_UINT8, },
	XTOPT_TABLEEND,
};

static void scs_init(struct xt_entry_match *m)
{
	struct xt_scs_mtinfo *info = (void *)m->data;

	info->spis[1] = ~0U;
}

static void scs_parse(struct xt_option_call *cb)
{
	struct xt_scs_mtinfo *info = cb->data;

	xtables_option_parse(cb);
	switch (cb->entry->id) {
	case O_ESPSPI:  
	    info->invflags |= XT_SCS_VALID_SPIS;
    	if (cb->invert) info->invflags |= XT_SCS_INV_SPI;
        info->spis[0] = cb->val.mark; info->spis[1] = cb->val.mask;
    	break;
	case O_IS_UP_SET: 
	    info->invflags |= XT_SCS_VALID_UP_OVRD;
    	if (cb->invert) info->invflags |= XT_SCS_INV_UP_OVRD;
    	break;
    case O_UP_SET:  
        info->invflags |= XT_SCS_VALID_UP_VAL;
        info->up_val = cb->val.u8;
        break; 
    case O_UDPESP_PORT:  
        info->invflags |= XT_SCS_VALID_UDPESP_L4P;
        info->udpesp_port = cb->val.u16;
        break; 
	}
}

static void
print_spis(const char *name, uint32_t val, uint32_t mask,
	    int invert)
{
	const char *inv = invert ? "!" : "";

    if (mask == ~0U)
        printf(" %s:%s%x", name, inv, val);
    else
        printf(" %s:%s%x/%x", name, inv, val, mask);
}

static void
scs_print(const void *ip, const struct xt_entry_match *match, int numeric)
{
	const struct xt_scs_mtinfo *info = (struct xt_scs_mtinfo *)match->data;

	printf(" scs");
	if (info->invflags & XT_SCS_VALID_UP_OVRD)    
	    printf(" %s up-is-set", (info->invflags & XT_SCS_INV_UP_OVRD) ? "!":"");
	if (info->invflags & XT_SCS_VALID_SPIS)    
	    print_spis("spi", info->spis[0], info->spis[1], info->invflags & XT_SCS_INV_SPI);
	if (info->invflags & XT_SCS_VALID_UP_VAL)
	    printf(" up-set %u", info->up_val);
	if (info->invflags & XT_SCS_VALID_UDPESP_L4P)
	    printf(" udpesp %u", info->udpesp_port);
}

static void scs_save(const void *ip, const struct xt_entry_match *match)
{
	const struct xt_scs_mtinfo *info = (struct xt_scs_mtinfo *)match->data;

	if (info->invflags & XT_SCS_VALID_SPIS) {
		printf("%s --espspi ",
			(info->invflags & XT_SCS_INV_SPI) ? " !" : "");
        if (info->spis[1] == ~0U)
			printf("%x", info->spis[0]);
	    else
			printf("%x/%x", info->spis[0], info->spis[1]);
	}
	if (info->invflags & XT_SCS_VALID_UP_OVRD)    
	    printf(" %s --up-is-set", (info->invflags & XT_SCS_INV_UP_OVRD) ? "!":"");
	if (info->invflags & XT_SCS_VALID_UP_VAL)
	    printf(" --up-set %u", info->up_val);
	if (info->invflags & XT_SCS_VALID_UDPESP_L4P)
	    printf(" --udpesp %u", info->udpesp_port);
}

static int scs_xlate(struct xt_xlate *xl,
		     const struct xt_xlate_mt_params *params)
{
	const struct xt_scs_mtinfo *info = (struct xt_scs_mtinfo *)params->match->data;

	if (info->invflags & XT_SCS_VALID_SPIS) {
		xt_xlate_add(xl, " scs spi%s", (info->invflags & XT_SCS_INV_SPI) ? " !=" : "");
        if (info->spis[1] == ~0U)
			xt_xlate_add(xl, " %x", info->spis[0]);
		else
			xt_xlate_add(xl, " %x/%x", info->spis[0], info->spis[1]);
	}
	if (info->invflags & XT_SCS_VALID_UP_OVRD)    
	    xt_xlate_add(xl, " %s up-is-set", (info->invflags & XT_SCS_INV_UP_OVRD) ? "!":"");
	if (info->invflags & XT_SCS_VALID_UP_VAL)
	    xt_xlate_add(xl, " up-set %u", info->up_val);
	if (info->invflags & XT_SCS_VALID_UDPESP_L4P)
	    xt_xlate_add(xl, " udpesp %u", info->udpesp_port);

	return 1;
}

static struct xtables_match scs_match = {
	.family		= NFPROTO_UNSPEC,
	.name		= "scs",
	.version	= XTABLES_VERSION,
	.size		= XT_ALIGN(sizeof(struct xt_scs_mtinfo)),
	.userspacesize	= XT_ALIGN(sizeof(struct xt_scs_mtinfo)),
	.help		= scs_help,
	.init		= scs_init,
	.print		= scs_print,
	.save		= scs_save,
	.x6_parse	= scs_parse,
	.x6_options	= scs_opts,
	.xlate		= scs_xlate,
};

void
_init(void)
{
	xtables_register_match(&scs_match);
}
