/* Shared library add-on to iptables to add TCPMSS target support.
 *
 * Copyright (c) 2000 Marc Boucher
*/
#include "config.h"
#include <stdio.h>
#include <xtables.h>
#include <netinet/ip.h>
#include <linux/netfilter_ipv4/xt_TCPTERMAC.h>
#include <string.h>

enum {
	O_SET_TERMAC = 0,
	O_CLAMP_TERMAC,
};

struct termacinfo {
	struct xt_entry_target t;
	struct xt_tcptermac_info tcpmacopt;
};

static int _is_hex(char c)
{
    return (((c >= '0') && (c <= '9')) ||
            ((c >= 'A') && (c <= 'F')) ||
            ((c >= 'a') && (c <= 'f')));
}

static int string_to_hex(char *string, unsigned char *key, int len)
{
	char tmpBuf[4];
	int idx, ii=0;
	for (idx=0; idx<len; idx+=2) {
		tmpBuf[0] = string[idx];
		tmpBuf[1] = string[idx+1];
		tmpBuf[2] = 0;
		if ( !_is_hex(tmpBuf[0]) || !_is_hex(tmpBuf[1]))
			return 0;

		key[ii++] = (unsigned char) strtol(tmpBuf, (char**)NULL, 16);
	}
	return 1;
}



static void __TCPTERMAC_help(int hdrsize)
{
	printf(
"TCPTERMAC target v%s mutually-exclusive options:\n"
"  --set-termac value               explicitly set terminate mac option to specified value\n",
XTABLES_VERSION);
}

static void TCPTERMAC_help(void)
{
	__TCPTERMAC_help(sizeof(struct iphdr));
}

static void TCPTERMAC_help6(void)
{
	__TCPTERMAC_help(SIZEOF_STRUCT_IP6_HDR);
}

static const struct xt_option_entry TCPTERMAC4_opts[] = {
	{.name = "set-termac", .id = O_SET_TERMAC, .type = XTTYPE_STRING,
	 .flags = XTOPT_PUT, XTOPT_POINTER(struct xt_tcptermac_info, termac)},
	XTOPT_TABLEEND,
};
#if 0
static const struct xt_option_entry TCPTERMAC6_opts[] = {
	{.name = "set-mss", .id = O_SET_TERMAC, .type = XTTYPE_UINT16,
	 .min = 0, .max = UINT16_MAX - SIZEOF_STRUCT_IP6_HDR,
	 .flags = XTOPT_PUT, XTOPT_POINTER(struct xt_tcptermac_info, mss)},
	XTOPT_TABLEEND,
};
#endif
static void TCPTERMAC_parse(struct xt_option_call *cb)
{
	struct xt_tcptermac_info *termacinfo = cb->data;
	xtables_option_parse(cb);
	if (cb->entry->id == O_SET_TERMAC)
	{
		if(string_to_hex(cb->arg, termacinfo->termac, 28) ==0)
			xtables_error(PARAMETER_PROBLEM,
		           	"Bad TCPTERMAC value `%s'", cb->arg);
	}
}

static void TCPTERMAC_check(struct xt_fcheck_call *cb)
{
	if (cb->xflags == 0)
		xtables_error(PARAMETER_PROBLEM,
		           "TCPTERMAC target: At least one parameter is required");
}

static void TCPTERMAC_print(const void *ip, const struct xt_entry_target *target,
                         int numeric)
{
	const struct xt_tcptermac_info *optinfo =
		(const struct xt_tcptermac_info *)target->data;
    printf("TCPTERMAC set %08x%08x%08x%04x ", *(__be32 *)&(optinfo->termac[0]), *(__be32 *)&(optinfo->termac[4]), *(__be32 *)&(optinfo->termac[8]),*(__be16 *)&(optinfo->termac[12]));
}

static void TCPTERMAC_save(const void *ip, const struct xt_entry_target *target)
{
	const struct xt_tcptermac_info *optinfo =
		(const struct xt_tcptermac_info *)target->data;

    printf("--set-termac %x ", optinfo->termac);
}

static struct xtables_target tcptermac_tg_reg[] = {
	{
		.family        = NFPROTO_IPV4,
		.name          = "TCPTERMAC",
		.version       = XTABLES_VERSION,
		.size          = XT_ALIGN(sizeof(struct xt_tcptermac_info)),
		.userspacesize = XT_ALIGN(sizeof(struct xt_tcptermac_info)),
		.help          = TCPTERMAC_help,
		.print         = TCPTERMAC_print,
		.save          = TCPTERMAC_save,
		.x6_parse      = TCPTERMAC_parse,
		.x6_fcheck     = TCPTERMAC_check,
		.x6_options    = TCPTERMAC4_opts,
	},
#if 0
	{
		.family        = NFPROTO_IPV6,
		.name          = "TCPTERMAC",
		.version       = XTABLES_VERSION,
		.size          = XT_ALIGN(sizeof(struct xt_tcptermac_info)),
		.userspacesize = XT_ALIGN(sizeof(struct xt_tcptermac_info)),
		.help          = TCPTERMAC_help6,
		.print         = TCPTERMAC_print,
		.save          = TCPTERMAC_save,
		.x6_parse      = TCPTERMAC_parse,
		.x6_fcheck     = TCPTERMAC_check,
		.x6_options    = TCPTERMAC6_opts,
	},
#endif
};

void _init(void)
{
	xtables_register_targets(tcptermac_tg_reg, ARRAY_SIZE(tcptermac_tg_reg));
}
