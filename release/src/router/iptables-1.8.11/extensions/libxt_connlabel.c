#define _GNU_SOURCE
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <xtables.h>
#include <linux/netfilter/xt_connlabel.h>
#include <libnetfilter_conntrack/libnetfilter_conntrack.h>

enum {
	O_LABEL = 0,
	O_SET = 1,
};

static struct nfct_labelmap *map;

static void connlabel_mt_help(void)
{
	puts(
"connlabel match options:\n"
"[!] --label name     Match if label has been set on connection\n"
"    --set            Set label on connection");
}

static const struct xt_option_entry connlabel_mt_opts[] = {
	{.name = "label", .id = O_LABEL, .type = XTTYPE_STRING,
	 .min = 1, .flags = XTOPT_MAND|XTOPT_INVERT},
	{.name = "set", .id = O_SET, .type = XTTYPE_NONE},
	XTOPT_TABLEEND,
};

/* cannot do this via _init, else static builds might spew error message
 * for every iptables invocation.
 */
static int connlabel_open(void)
{
	const char *fname;

	if (map)
		return 0;

	map = nfct_labelmap_new(NULL);
	if (map != NULL)
		return 0;

	fname = nfct_labels_get_path();
	if (errno) {
		fprintf(stderr, "Warning: cannot open %s: %s\n",
			fname, strerror(errno));
	} else {
		xtables_error(RESOURCE_PROBLEM,
			"cannot parse %s: no labels found", fname);
	}
	return 1;
}

static int connlabel_value_parse(const char *in)
{
	char *end;
	unsigned long value = strtoul(in, &end, 0);

	if (in[0] == '\0' || *end != '\0')
		return -1;

	return value;
}

static void connlabel_mt_parse(struct xt_option_call *cb)
{
	struct xt_connlabel_mtinfo *info = cb->data;
	int tmp;

	xtables_option_parse(cb);

	switch (cb->entry->id) {
	case O_LABEL:
		tmp = connlabel_value_parse(cb->arg);
		if (tmp < 0 && !connlabel_open())
			tmp = nfct_labelmap_get_bit(map, cb->arg);
		if (tmp < 0)
			xtables_error(PARAMETER_PROBLEM,
				      "label '%s' not found or invalid value",
				      cb->arg);

		info->bit = tmp;
		if (cb->invert)
			info->options |= XT_CONNLABEL_OP_INVERT;
		break;
	case O_SET:
		info->options |= XT_CONNLABEL_OP_SET;
		break;
	}

}

static const char *connlabel_get_name(int b)
{
	const char *name;

	if (connlabel_open())
		return NULL;

	name = nfct_labelmap_get_name(map, b);
	if (name && strcmp(name, ""))
		return name;
	return NULL;
}

static void
connlabel_mt_print_op(const struct xt_connlabel_mtinfo *info, const char *prefix)
{
	if (info->options & XT_CONNLABEL_OP_SET)
		printf(" %sset", prefix);
}

static void
connlabel_mt_print(const void *ip, const struct xt_entry_match *match, int numeric)
{
	const struct xt_connlabel_mtinfo *info = (const void *)match->data;
	const char *name = connlabel_get_name(info->bit);

	printf(" connlabel");
	if (info->options & XT_CONNLABEL_OP_INVERT)
		printf(" !");
	if (numeric || name == NULL) {
		printf(" %u", info->bit);
	} else {
		printf(" '%s'", name);
	}
	connlabel_mt_print_op(info, "");
}

static void
connlabel_mt_save(const void *ip, const struct xt_entry_match *match)
{
	const struct xt_connlabel_mtinfo *info = (const void *)match->data;
	const char *name = connlabel_get_name(info->bit);

	if (info->options & XT_CONNLABEL_OP_INVERT)
		printf(" !");
	if (name)
		printf(" --label \"%s\"", name);
	else
		printf(" --label \"%u\"", info->bit);
	connlabel_mt_print_op(info, "--");
}

static int connlabel_mt_xlate(struct xt_xlate *xl,
			      const struct xt_xlate_mt_params *params)
{
	const struct xt_connlabel_mtinfo *info =
		(const void *)params->match->data;
	const char *name = connlabel_get_name(info->bit);
	char *valbuf = NULL;

	if (name == NULL) {
		if (asprintf(&valbuf, "%u", info->bit) < 0)
			return 0;
		name = valbuf;
	}

	if (info->options & XT_CONNLABEL_OP_SET)
		xt_xlate_add(xl, "ct label set %s ", name);

	xt_xlate_add(xl, "ct label ");
	if (info->options & XT_CONNLABEL_OP_INVERT)
		xt_xlate_add(xl, "and %s != ", name);
	xt_xlate_add(xl, "%s", name);

	free(valbuf);
	return 1;
}

static struct xtables_match connlabel_mt_reg = {
	.family        = NFPROTO_UNSPEC,
	.name          = "connlabel",
	.version       = XTABLES_VERSION,
	.size          = XT_ALIGN(sizeof(struct xt_connlabel_mtinfo)),
	.userspacesize = offsetof(struct xt_connlabel_mtinfo, bit),
	.help          = connlabel_mt_help,
	.print         = connlabel_mt_print,
	.save          = connlabel_mt_save,
	.x6_parse      = connlabel_mt_parse,
	.x6_options    = connlabel_mt_opts,
	.xlate	       = connlabel_mt_xlate,
};

void _init(void)
{
	xtables_register_match(&connlabel_mt_reg);
}
