#include <stdio.h>
#include <xtables.h>
#include <linux/netfilter/xt_FSMARK.h>

enum {
	O_ID = 0,
};

static void FSMARK_help(void)
{
	printf(
"FSMARK target options:\n"
"    --id val                   mandatory, specify flow stats query ID to use\n");
}

static const struct xt_option_entry FSMARK_opts[] = {
	{.name = "id",      .id = O_ID,      .type = XTTYPE_UINT8},
	XTOPT_TABLEEND,
};

static void FSMARK_init(struct xt_entry_target *t)
{
	struct xt_fsmark_tginfo *info = (void *)t->data;

	info->flags = 0;
}

static void FSMARK_parse(struct xt_option_call *cb)
{
	struct xt_fsmark_tginfo *info = cb->data;

	xtables_option_parse(cb);
	switch (cb->entry->id) {
	case O_ID:
	    if (cb->val.u8 >= MAX_NF_FSMARK_QUERIES)
	        xtables_error(PARAMETER_PROBLEM, "query id %d valid range is (0 .. %d)", cb->val.u8, MAX_NF_FSMARK_QUERIES-1);
	    else {
            info->id = cb->val.u8;
	        info->flags |= XT_FSMARK_VALID_ID;
	    }
    	break;
	}
}

static void
FSMARK_print(const void *ip, const struct xt_entry_target *t, int numeric)
{
	const struct xt_fsmark_tginfo *info = (void *)t->data;

	printf(" FSMARK");
	if (info->flags & XT_FSMARK_VALID_ID)    
	    printf(" id %d", info->id);
}

static void FSMARK_save(const void *ip, const struct xt_entry_target *t)
{
	const struct xt_fsmark_tginfo *info = (void *)t->data;

	if (info->flags & XT_FSMARK_VALID_ID)    
	    printf(" --id %d", info->id);
}

static int FSMARK_xlate(struct xt_xlate *xl,
		     const struct xt_xlate_tg_params *params)
{
	const struct xt_fsmark_tginfo *info = (void *)params->target->data;

	if (info->flags & XT_FSMARK_VALID_ID)    
	    xt_xlate_add(xl, "id %d", info->id);

	return 1;
}

static struct xtables_target fsmark_tg = {
	.family		= NFPROTO_UNSPEC,
	.name		= "FSMARK",
	.version	= XTABLES_VERSION,
	.size		= XT_ALIGN(sizeof(struct xt_fsmark_tginfo)),
	.userspacesize	= XT_ALIGN(sizeof(struct xt_fsmark_tginfo)),
	.help		= FSMARK_help,
	.init		= FSMARK_init,
	.print		= FSMARK_print,
	.save		= FSMARK_save,
	.x6_parse	= FSMARK_parse,
	.x6_options	= FSMARK_opts,
	.xlate		= FSMARK_xlate,
};

void
_init(void)
{
	xtables_register_target(&fsmark_tg);
}
