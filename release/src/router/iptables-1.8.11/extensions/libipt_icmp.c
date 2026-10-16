#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <xtables.h>
#include <limits.h> /* INT_MAX in ip6_tables.h */
#include <linux/netfilter_ipv4/ip_tables.h>

#include "libxt_icmp.h"

/* special hack for icmp-type 'any': 
 * Up to kernel <=2.4.20 the problem was:
 * '-p icmp ' matches all icmp packets
 * '-p icmp -m icmp' matches _only_ ICMP type 0 :(
 * This is now fixed by initializing the field * to icmp type 0xFF
 * See: https://bugzilla.netfilter.org/cgi-bin/bugzilla/show_bug.cgi?id=37
 */

enum {
	O_ICMP_TYPE = 0,
};

static void icmp_help(void)
{
	printf(
"icmp match options:\n"
"[!] --icmp-type typename	match icmp type\n"
"[!] --icmp-type type[/code]	(or numeric type or type/code)\n");
	printf("Valid ICMP Types:");
	xt_print_icmp_types(icmp_codes, ARRAY_SIZE(icmp_codes));
}

static const struct xt_option_entry icmp_opts[] = {
	{.name = "icmp-type", .id = O_ICMP_TYPE, .type = XTTYPE_STRING,
	 .flags = XTOPT_MAND | XTOPT_INVERT},
	XTOPT_TABLEEND,
};

static void icmp_init(struct xt_entry_match *m)
{
	struct ipt_icmp *icmpinfo = (struct ipt_icmp *)m->data;

	icmpinfo->type = 0xFF;
	icmpinfo->code[1] = 0xFF;
}

static void icmp_parse(struct xt_option_call *cb)
{
	struct ipt_icmp *icmpinfo = cb->data;

	xtables_option_parse(cb);
	ipt_parse_icmp(cb->arg, &icmpinfo->type, icmpinfo->code);
	if (cb->invert)
		icmpinfo->invflags |= IPT_ICMP_INV;
}

static void print_icmptype(uint8_t type,
			   uint8_t code_min, uint8_t code_max,
			   int invert,
			   int numeric)
{
	if (!numeric) {
		unsigned int i;

		for (i = 0; i < ARRAY_SIZE(icmp_codes); ++i)
			if (icmp_codes[i].type == type
			    && icmp_codes[i].code_min == code_min
			    && icmp_codes[i].code_max == code_max)
				break;

		if (i != ARRAY_SIZE(icmp_codes)) {
			printf(" %s%s",
			       invert ? "!" : "",
			       icmp_codes[i].name);
			return;
		}
	}

	if (invert)
		printf(" !");

	printf("type %u", type);
	if (code_min == code_max)
		printf(" code %u", code_min);
	else if (code_min != 0 || code_max != 0xFF)
		printf(" codes %u-%u", code_min, code_max);
}

static void icmp_print(const void *ip, const struct xt_entry_match *match,
                       int numeric)
{
	const struct ipt_icmp *icmp = (struct ipt_icmp *)match->data;

	printf(" icmp");
	print_icmptype(icmp->type, icmp->code[0], icmp->code[1],
		       icmp->invflags & IPT_ICMP_INV,
		       numeric);

	if (icmp->invflags & ~IPT_ICMP_INV)
		printf(" Unknown invflags: 0x%X",
		       icmp->invflags & ~IPT_ICMP_INV);
}

static void icmp_save(const void *ip, const struct xt_entry_match *match)
{
	const struct ipt_icmp *icmp = (struct ipt_icmp *)match->data;

	if (icmp->invflags & IPT_ICMP_INV)
		printf(" !");

	/* special hack for 'any' case */
	if (icmp->type == 0xFF &&
	    icmp->code[0] == 0 && icmp->code[1] == 0xFF) {
		printf(" --icmp-type any");
	} else {
		printf(" --icmp-type %u", icmp->type);
		if (icmp->code[0] != 0 || icmp->code[1] != 0xFF)
			printf("/%u", icmp->code[0]);
	}
}

static unsigned int type_xlate_print(struct xt_xlate *xl, unsigned int icmptype,
				     unsigned int code_min,
				     unsigned int code_max)
{
	unsigned int i;

	if (code_min != code_max) {
		for (i = 0; i < ARRAY_SIZE(icmp_codes); ++i)
			if (icmp_codes[i].type == icmptype &&
			    icmp_codes[i].code_min == code_min &&
			    icmp_codes[i].code_max == code_max) {
				xt_xlate_add(xl, "%s", icmp_codes[i].name);
				return 1;
			}
	}

	return 0;
}

static int icmp_xlate(struct xt_xlate *xl,
		      const struct xt_xlate_mt_params *params)
{
	const struct ipt_icmp *info = (struct ipt_icmp *)params->match->data;

	if (info->type != 0xFF) {
		xt_xlate_add(xl, "icmp type%s ",
			     (info->invflags & IPT_ICMP_INV) ? " !=" : "");

		if (!type_xlate_print(xl, info->type, info->code[0],
				      info->code[1]))
			return 0;
	} else {
		/* '-m icmp --icmp-type any' is a noop by itself,
		 * but it eats a (mandatory) previous '-p icmp' so
		 * emit it here */
		xt_xlate_add(xl, "ip protocol icmp");
	}
	return 1;
}

static struct xtables_match icmp_mt_reg = {
	.name		= "icmp",
	.version	= XTABLES_VERSION,
	.family		= NFPROTO_IPV4,
	.size		= XT_ALIGN(sizeof(struct ipt_icmp)),
	.userspacesize	= XT_ALIGN(sizeof(struct ipt_icmp)),
	.help		= icmp_help,
	.init		= icmp_init,
	.print		= icmp_print,
	.save		= icmp_save,
	.x6_parse	= icmp_parse,
	.x6_options	= icmp_opts,
	.xlate		= icmp_xlate,
};

void _init(void)
{
	xtables_register_match(&icmp_mt_reg);
}
