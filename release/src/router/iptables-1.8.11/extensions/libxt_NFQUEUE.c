/* Shared library add-on to iptables for NFQ
 *
 * (C) 2005 by Harald Welte <laforge@netfilter.org>
 *
 * This program is distributed under the terms of GNU GPL v2, 1991
 *
 */
#include <stdio.h>
#include <xtables.h>
#include <linux/netfilter/xt_NFQUEUE.h>

enum {
	O_QUEUE_NUM = 0,
	O_QUEUE_BALANCE,
	O_QUEUE_BYPASS,
	O_QUEUE_CPU_FANOUT,
	F_QUEUE_NUM     = 1 << O_QUEUE_NUM,
	F_QUEUE_BALANCE = 1 << O_QUEUE_BALANCE,
	F_QUEUE_CPU_FANOUT = 1 << O_QUEUE_CPU_FANOUT,
};

static void NFQUEUE_help(void)
{
	printf(
"NFQUEUE target options\n"
"  --queue-num value		Send packet to QUEUE number <value>.\n"
"  		                Valid queue numbers are 0-65535\n"
);
}

static void NFQUEUE_help_v1(void)
{
	printf(
"NFQUEUE target options\n"
"  --queue-num value            Send packet to QUEUE number <value>.\n"
"                               Valid queue numbers are 0-65535\n"
"  --queue-balance first:last	Balance flows between queues <value> to <value>.\n");
}

static void NFQUEUE_help_v2(void)
{
	printf(
"NFQUEUE target options\n"
"  --queue-num value            Send packet to QUEUE number <value>.\n"
"                               Valid queue numbers are 0-65535\n"
"  --queue-balance first:last   Balance flows between queues <value> to <value>.\n"
"  --queue-bypass		Bypass Queueing if no queue instance exists.\n"
"  --queue-cpu-fanout	Use current CPU (no hashing)\n");
}

static void NFQUEUE_help_v3(void)
{
	printf(
"NFQUEUE target options\n"
"  --queue-num value            Send packet to QUEUE number <value>.\n"
"                               Valid queue numbers are 0-65535\n"
"  --queue-balance first:last   Balance flows between queues <value> to <value>.\n"
"  --queue-bypass               Bypass Queueing if no queue instance exists.\n"
"  --queue-cpu-fanout	Use current CPU (no hashing)\n");
}

#define s struct xt_NFQ_info
static const struct xt_option_entry NFQUEUE_opts[] = {
	{.name = "queue-num", .id = O_QUEUE_NUM, .type = XTTYPE_UINT16,
	 .flags = XTOPT_PUT, XTOPT_POINTER(s, queuenum),
	 .excl = F_QUEUE_BALANCE},
	{.name = "queue-balance", .id = O_QUEUE_BALANCE, .max = UINT16_MAX - 1,
	 .type = XTTYPE_UINT16RC, .excl = F_QUEUE_NUM},
	{.name = "queue-bypass", .id = O_QUEUE_BYPASS, .type = XTTYPE_NONE},
	{.name = "queue-cpu-fanout", .id = O_QUEUE_CPU_FANOUT,
	 .type = XTTYPE_NONE, .also = F_QUEUE_BALANCE},
	XTOPT_TABLEEND,
};
#undef s

static void NFQUEUE_parse(struct xt_option_call *cb)
{
	xtables_option_parse(cb);
	if (cb->entry->id == O_QUEUE_BALANCE)
		xtables_error(PARAMETER_PROBLEM, "NFQUEUE target: "
				   "--queue-balance not supported (kernel too old?)");
}

static void NFQUEUE_parse_v1(struct xt_option_call *cb)
{
	struct xt_NFQ_info_v1 *info = cb->data;
	const uint16_t *r = cb->val.u16_range;

	xtables_option_parse(cb);
	switch (cb->entry->id) {
	case O_QUEUE_BALANCE:
		if (cb->nvals != 2)
			xtables_error(PARAMETER_PROBLEM,
				"Bad range \"%s\"", cb->arg);
		if (r[0] >= r[1])
			xtables_error(PARAMETER_PROBLEM, "%u should be less than %u",
				r[0], r[1]);
		info->queuenum = r[0];
		info->queues_total = r[1] - r[0] + 1;
		break;
	}
}

static void NFQUEUE_parse_v2(struct xt_option_call *cb)
{
	struct xt_NFQ_info_v2 *info = cb->data;
	const uint16_t *r = cb->val.u16_range;

	xtables_option_parse(cb);
	switch (cb->entry->id) {
	case O_QUEUE_BALANCE:
		if (cb->nvals != 2)
			xtables_error(PARAMETER_PROBLEM,
				"Bad range \"%s\"", cb->arg);
		if (r[0] >= r[1])
			xtables_error(PARAMETER_PROBLEM,
				      "%u should be less than %u",
				      r[0], r[1]);
		info->queuenum = r[0];
		info->queues_total = r[1] - r[0] + 1;
		break;
	case O_QUEUE_BYPASS:
		info->bypass |= NFQ_FLAG_BYPASS;
		break;
	}
}

static void NFQUEUE_parse_v3(struct xt_option_call *cb)
{
	struct xt_NFQ_info_v3 *info = cb->data;
	const uint16_t *r = cb->val.u16_range;

	xtables_option_parse(cb);
	switch (cb->entry->id) {
	case O_QUEUE_BALANCE:
		if (cb->nvals != 2)
			xtables_error(PARAMETER_PROBLEM,
				"Bad range \"%s\"", cb->arg);
		if (r[0] >= r[1])
			xtables_error(PARAMETER_PROBLEM,
				      "%u should be less than %u",
				      r[0], r[1]);
		info->queuenum = r[0];
		info->queues_total = r[1] - r[0] + 1;
		break;
	case O_QUEUE_BYPASS:
		info->flags |= NFQ_FLAG_BYPASS;
		break;
	case O_QUEUE_CPU_FANOUT:
		info->flags |= NFQ_FLAG_CPU_FANOUT;
		break;
	}
}

static void NFQUEUE_print(const void *ip,
                          const struct xt_entry_target *target, int numeric)
{
	const struct xt_NFQ_info *tinfo =
		(const struct xt_NFQ_info *)target->data;
	printf(" NFQUEUE num %u", tinfo->queuenum);
}

static void NFQUEUE_print_v1(const void *ip,
                             const struct xt_entry_target *target, int numeric)
{
	const struct xt_NFQ_info_v1 *tinfo = (const void *)target->data;
	unsigned int last = tinfo->queues_total;

	if (last > 1) {
		last += tinfo->queuenum - 1;
		printf(" NFQUEUE balance %u:%u", tinfo->queuenum, last);
	} else {
		printf(" NFQUEUE num %u", tinfo->queuenum);
	}
}

static void NFQUEUE_print_v2(const void *ip,
                             const struct xt_entry_target *target, int numeric)
{
	const struct xt_NFQ_info_v2 *info = (void *) target->data;
	unsigned int last = info->queues_total;

	if (last > 1) {
		last += info->queuenum - 1;
		printf(" NFQUEUE balance %u:%u", info->queuenum, last);
	} else
		printf(" NFQUEUE num %u", info->queuenum);

	if (info->bypass & NFQ_FLAG_BYPASS)
		printf(" bypass");
}

static void NFQUEUE_print_v3(const void *ip,
                             const struct xt_entry_target *target, int numeric)
{
	const struct xt_NFQ_info_v3 *info = (void *)target->data;
	unsigned int last = info->queues_total;

	if (last > 1) {
		last += info->queuenum - 1;
		printf(" NFQUEUE balance %u:%u", info->queuenum, last);
	} else
		printf(" NFQUEUE num %u", info->queuenum);

	if (info->flags & NFQ_FLAG_BYPASS)
		printf(" bypass");

	if (info->flags & NFQ_FLAG_CPU_FANOUT)
		printf(" cpu-fanout");
}

static void NFQUEUE_save(const void *ip, const struct xt_entry_target *target)
{
	const struct xt_NFQ_info *tinfo =
		(const struct xt_NFQ_info *)target->data;

	printf(" --queue-num %u", tinfo->queuenum);
}

static void NFQUEUE_save_v1(const void *ip, const struct xt_entry_target *target)
{
	const struct xt_NFQ_info_v1 *tinfo = (const void *)target->data;
	unsigned int last = tinfo->queues_total;

	if (last > 1) {
		last += tinfo->queuenum - 1;
		printf(" --queue-balance %u:%u", tinfo->queuenum, last);
	} else {
		printf(" --queue-num %u", tinfo->queuenum);
	}
}

static void NFQUEUE_save_v2(const void *ip, const struct xt_entry_target *target)
{
	const struct xt_NFQ_info_v2 *info = (void *) target->data;
	unsigned int last = info->queues_total;

	if (last > 1) {
		last += info->queuenum - 1;
		printf(" --queue-balance %u:%u", info->queuenum, last);
	} else
		printf(" --queue-num %u", info->queuenum);

	if (info->bypass & NFQ_FLAG_BYPASS)
		printf(" --queue-bypass");
}

static void NFQUEUE_save_v3(const void *ip,
			    const struct xt_entry_target *target)
{
	const struct xt_NFQ_info_v3 *info = (void *)target->data;
	unsigned int last = info->queues_total;

	if (last > 1) {
		last += info->queuenum - 1;
		printf(" --queue-balance %u:%u", info->queuenum, last);
	} else
		printf(" --queue-num %u", info->queuenum);

	if (info->flags & NFQ_FLAG_BYPASS)
		printf(" --queue-bypass");

	if (info->flags & NFQ_FLAG_CPU_FANOUT)
		printf(" --queue-cpu-fanout");
}

static void NFQUEUE_init_v1(struct xt_entry_target *t)
{
	struct xt_NFQ_info_v1 *tinfo = (void *)t->data;
	tinfo->queues_total = 1;
}

static int NFQUEUE_xlate(struct xt_xlate *xl,
			 const struct xt_xlate_tg_params *params)
{
	const struct xt_NFQ_info *tinfo =
		(const struct xt_NFQ_info *)params->target->data;

	xt_xlate_add(xl, "queue num %u ", tinfo->queuenum);

	return 1;
}

static int NFQUEUE_xlate_v1(struct xt_xlate *xl,
			    const struct xt_xlate_tg_params *params)
{
	const struct xt_NFQ_info_v1 *tinfo = (const void *)params->target->data;
	unsigned int last = tinfo->queues_total;

	if (last > 1) {
		last += tinfo->queuenum - 1;
		xt_xlate_add(xl, "queue num %u-%u ", tinfo->queuenum, last);
	} else {
		xt_xlate_add(xl, "queue num %u ", tinfo->queuenum);
	}

	return 1;
}

static int NFQUEUE_xlate_v2(struct xt_xlate *xl,
			    const struct xt_xlate_tg_params *params)
{
	const struct xt_NFQ_info_v2 *info = (void *)params->target->data;
	unsigned int last = info->queues_total;

	if (last > 1) {
		last += info->queuenum - 1;
		xt_xlate_add(xl, "queue num %u-%u ", info->queuenum, last);
	} else
		xt_xlate_add(xl, "queue num %u ", info->queuenum);

	if (info->bypass & NFQ_FLAG_BYPASS)
		xt_xlate_add(xl, "bypass");

	return 1;
}

static int NFQUEUE_xlate_v3(struct xt_xlate *xl,
			    const struct xt_xlate_tg_params *params)
{
	const struct xt_NFQ_info_v3 *info = (void *)params->target->data;
	unsigned int last = info->queues_total;

	if (last > 1) {
		last += info->queuenum - 1;
		xt_xlate_add(xl, "queue num %u-%u ", info->queuenum, last);
	} else
		xt_xlate_add(xl, "queue num %u ", info->queuenum);

	if (info->flags & NFQ_FLAG_BYPASS)
		xt_xlate_add(xl, "bypass");

	if (info->flags & NFQ_FLAG_CPU_FANOUT)
		xt_xlate_add(xl, "%sfanout ",
			     info->flags & NFQ_FLAG_BYPASS ? "," : "");

	return 1;
}

static struct xtables_target nfqueue_targets[] = {
{
	.family		= NFPROTO_UNSPEC,
	.name		= "NFQUEUE",
	.version	= XTABLES_VERSION,
	.size		= XT_ALIGN(sizeof(struct xt_NFQ_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct xt_NFQ_info)),
	.help		= NFQUEUE_help,
	.print		= NFQUEUE_print,
	.save		= NFQUEUE_save,
	.x6_parse	= NFQUEUE_parse,
	.x6_options	= NFQUEUE_opts,
	.xlate		= NFQUEUE_xlate,
},{
	.family		= NFPROTO_UNSPEC,
	.revision	= 1,
	.name		= "NFQUEUE",
	.version	= XTABLES_VERSION,
	.size		= XT_ALIGN(sizeof(struct xt_NFQ_info_v1)),
	.userspacesize	= XT_ALIGN(sizeof(struct xt_NFQ_info_v1)),
	.help		= NFQUEUE_help_v1,
	.init		= NFQUEUE_init_v1,
	.print		= NFQUEUE_print_v1,
	.save		= NFQUEUE_save_v1,
	.x6_parse	= NFQUEUE_parse_v1,
	.x6_options	= NFQUEUE_opts,
	.xlate		= NFQUEUE_xlate_v1,
},{
	.family		= NFPROTO_UNSPEC,
	.revision	= 2,
	.name		= "NFQUEUE",
	.version	= XTABLES_VERSION,
	.size		= XT_ALIGN(sizeof(struct xt_NFQ_info_v2)),
	.userspacesize	= XT_ALIGN(sizeof(struct xt_NFQ_info_v2)),
	.help		= NFQUEUE_help_v2,
	.init		= NFQUEUE_init_v1,
	.print		= NFQUEUE_print_v2,
	.save		= NFQUEUE_save_v2,
	.x6_parse	= NFQUEUE_parse_v2,
	.x6_options	= NFQUEUE_opts,
	.xlate		= NFQUEUE_xlate_v2,
},{
	.family		= NFPROTO_UNSPEC,
	.revision	= 3,
	.name		= "NFQUEUE",
	.version	= XTABLES_VERSION,
	.size		= XT_ALIGN(sizeof(struct xt_NFQ_info_v3)),
	.userspacesize	= XT_ALIGN(sizeof(struct xt_NFQ_info_v3)),
	.help		= NFQUEUE_help_v3,
	.init		= NFQUEUE_init_v1,
	.print		= NFQUEUE_print_v3,
	.save		= NFQUEUE_save_v3,
	.x6_parse	= NFQUEUE_parse_v3,
	.x6_options	= NFQUEUE_opts,
	.xlate		= NFQUEUE_xlate_v3,
}
};

void _init(void)
{
	xtables_register_targets(nfqueue_targets, ARRAY_SIZE(nfqueue_targets));
}
