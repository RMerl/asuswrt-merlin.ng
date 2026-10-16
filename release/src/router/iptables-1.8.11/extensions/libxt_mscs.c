#include <stdbool.h>
#include <stdio.h>
#include <xtables.h>
#include <linux/netfilter/xt_mscs.h>

enum {
       O_BITMAP = 0,
       O_LIMIT,
       O_GLOBAL,
};

static void mscs_mt_help(void)
{
       printf(
"mscs match options:\n"
"    --up-bitmap value  Match upstream traffic with priority present in UP bitmap\n"
"    --up-limit value   Limit downstream priority when upstream matched\n"
"    --global           Global rule\n");
}

static const struct xt_option_entry mscs_mt_opts[] = {
       {.name = "up-bitmap",  .id = O_BITMAP,.type = XTTYPE_UINT8, },
       {.name = "up-limit",   .id = O_LIMIT, .type = XTTYPE_UINT8, },
       {.name = "global",     .id = O_GLOBAL,.type = XTTYPE_NONE,  },
       XTOPT_TABLEEND,
};

static void mscs_mt_parse(struct xt_option_call *cb)
{
       struct xt_mscs_mtinfo *info = cb->data;

       xtables_option_parse(cb);
       switch (cb->entry->id) {
       case O_BITMAP:  info->is_global = 0; info->up_bitmap = cb->val.u8; break;
       case O_LIMIT:   info->is_global = 0; info->up_limit = cb->val.u8; break;
       case O_GLOBAL:  info->is_global = 1; break;
       }
}

static void
mscs_mt_print(const void *ip, const struct xt_entry_match *match, int numeric)
{
       const struct xt_mscs_mtinfo *info = (const void *)match->data;

    printf(" mscs ");
    if (info->is_global)    printf("global");
    else                    printf("up-bitmap 0x%02x up-limit %d", info->up_bitmap, info->up_limit);
}

static void mscs_mt_save(const void *ip, const struct xt_entry_match *match)
{
       const struct xt_mscs_mtinfo *info = (const void *)match->data;

    if (info->is_global)    printf(" --global");
    else                    printf(" --up-bitmap 0x%02x --up-limit %d", info->up_bitmap, info->up_limit);
}

static void mscs_mt_check(struct xt_fcheck_call *cb)
{
       if (cb->xflags == 0)
               xtables_error(PARAMETER_PROBLEM,
                       "mscs match: Parameter --global or --up-bitmap is required");
}

static int mscs_mt_xlate(struct xt_xlate *xl,
                        const struct xt_xlate_mt_params *params)
{
       const struct xt_mscs_mtinfo *info = (const void *)params->match->data;

    if (info->is_global)    xt_xlate_add(xl, " global");
    else                    xt_xlate_add(xl, " up-bitmap 0x%02x up-limit %d", info->up_bitmap, info->up_limit);

       return 1;
}

static struct xtables_match mscs_mt_reg = {
       .version       = XTABLES_VERSION,
       .name          = "mscs",
       .family        = NFPROTO_UNSPEC,
       .size          = XT_ALIGN(sizeof(struct xt_mscs_mtinfo)),
       .userspacesize = XT_ALIGN(sizeof(struct xt_mscs_mtinfo)),
       .help          = mscs_mt_help,
       .print         = mscs_mt_print,
       .save          = mscs_mt_save,
       .x6_parse      = mscs_mt_parse,
       .x6_fcheck     = mscs_mt_check,
       .x6_options    = mscs_mt_opts,
       .xlate         = mscs_mt_xlate,
};

void _init(void)
{
       xtables_register_match(&mscs_mt_reg);
}
