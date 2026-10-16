#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#if defined(__GLIBC__) && __GLIBC__ == 2
#include <net/ethernet.h>
#else
#include <linux/if_ether.h>
#endif
#include <xtables.h>
#include <linux/netfilter_ipv4/ipt_realm.h>

enum {
	O_REALM = 0,
};

static void realm_help(void)
{
	printf(
"realm match options:\n"
"[!] --realm value[/mask]\n"
"				Match realm\n");
}

static const struct xt_option_entry realm_opts[] = {
	{.name = "realm", .id = O_REALM, .type = XTTYPE_STRING,
	 .flags = XTOPT_MAND | XTOPT_INVERT},
	XTOPT_TABLEEND,
};

static const char f_realms[] = "/etc/iproute2/rt_realms";
/* array of realms from f_realms[] */
static struct xtables_lmap *realms;

static void realm_parse(struct xt_option_call *cb)
{
	struct xt_realm_info *ri = cb->data;
	unsigned int id, mask;

	xtables_option_parse(cb);
	xtables_parse_val_mask(cb, &id, &mask, realms);

	ri->id = id;
	ri->mask = mask;

	if (cb->invert)
		ri->invert = 1;
}

static void realm_print(const void *ip, const struct xt_entry_match *match,
			int numeric)
{
	const struct xt_realm_info *ri = (const void *)match->data;

	if (ri->invert)
		printf(" !");

	printf(" realm");
	xtables_print_val_mask(ri->id, ri->mask, numeric ? NULL : realms);
}

static void realm_save(const void *ip, const struct xt_entry_match *match)
{
	const struct xt_realm_info *ri = (const void *)match->data;

	if (ri->invert)
		printf(" !");

	printf(" --realm");
	xtables_print_val_mask(ri->id, ri->mask, realms);
}

static void
print_realm_xlate(unsigned long id, unsigned long mask,
		  int numeric, struct xt_xlate *xl, uint32_t op)
{
	const char *name = NULL;

	if (mask != 0xffffffff)
		xt_xlate_add(xl, " and 0x%lx %s 0x%lx", mask,
			   op == XT_OP_EQ ? "==" : "!=", id);
	else {
		if (numeric == 0)
			name = xtables_lmap_id2name(realms, id);
		if (name)
			xt_xlate_add(xl, " %s%s",
				   op == XT_OP_EQ ? "" : "!= ", name);
		else
			xt_xlate_add(xl, " %s0x%lx",
				   op == XT_OP_EQ ? "" : "!= ", id);
	}
}

static int realm_xlate(struct xt_xlate *xl,
		       const struct xt_xlate_mt_params *params)
{
	const struct xt_realm_info *ri = (const void *)params->match->data;
	enum xt_op op = XT_OP_EQ;

	if (ri->invert)
		op = XT_OP_NEQ;

	xt_xlate_add(xl, "rtclassid");
	print_realm_xlate(ri->id, ri->mask, 0, xl, op);

	return 1;
}

static struct xtables_match realm_mt_reg = {
	.name		= "realm",
	.version	= XTABLES_VERSION,
	.family		= NFPROTO_IPV4,
	.size		= XT_ALIGN(sizeof(struct xt_realm_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct xt_realm_info)),
	.help		= realm_help,
	.print		= realm_print,
	.save		= realm_save,
	.x6_parse	= realm_parse,
	.x6_options	= realm_opts,
	.xlate		= realm_xlate,
};

void _init(void)
{
	realms = xtables_lmap_init(f_realms);
	if (realms == NULL && errno != ENOENT)
		fprintf(stderr, "Warning: %s: %s\n", f_realms,
			strerror(errno));

	xtables_register_match(&realm_mt_reg);
}
