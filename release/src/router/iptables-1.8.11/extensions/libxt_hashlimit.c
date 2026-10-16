/* ip6tables match extension for limiting packets per destination
 *
 * (C) 2003-2004 by Harald Welte <laforge@netfilter.org>
 *
 * Development of this code was funded by Astaro AG, http://www.astaro.com/
 *
 * Based on ipt_limit.c by
 * Jérôme de Vivie   <devivie@info.enserb.u-bordeaux.fr>
 * Hervé Eychenne    <rv@wallfire.org>
 *
 * Error corections by nmalykh@bilim.com (22.01.2005)
 */
#define _BSD_SOURCE 1
#define _DEFAULT_SOURCE 1
#define _ISOC99_SOURCE 1
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <xtables.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter/xt_hashlimit.h>

#define XT_HASHLIMIT_BURST	5
#define XT_HASHLIMIT_BURST_MAX_v1	10000
#define XT_HASHLIMIT_BURST_MAX		1000000

#define XT_HASHLIMIT_BYTE_EXPIRE	15
#define XT_HASHLIMIT_BYTE_EXPIRE_BURST	60

/* miliseconds */
#define XT_HASHLIMIT_GCINTERVAL	1000

struct hashlimit_mt_udata {
	uint32_t mult;
};

static void hashlimit_help(void)
{
	printf(
"hashlimit match options:\n"
"--hashlimit <avg>		max average match rate\n"
"                                [Packets per second unless followed by \n"
"                                /sec /minute /hour /day postfixes]\n"
"--hashlimit-mode <mode>		mode is a comma-separated list of\n"
"					dstip,srcip,dstport,srcport\n"
"--hashlimit-name <name>		name for /proc/net/ipt_hashlimit/\n"
"[--hashlimit-burst <num>]	number to match in a burst, default %u\n"
"[--hashlimit-htable-size <num>]	number of hashtable buckets\n"
"[--hashlimit-htable-max <num>]	number of hashtable entries\n"
"[--hashlimit-htable-gcinterval]	interval between garbage collection runs\n"
"[--hashlimit-htable-expire]	after which time are idle entries expired?\n",
XT_HASHLIMIT_BURST);
}

enum {
	O_UPTO = 0,
	O_ABOVE,
	O_LIMIT,
	O_MODE,
	O_SRCMASK,
	O_DSTMASK,
	O_NAME,
	O_BURST,
	O_HTABLE_SIZE,
	O_HTABLE_MAX,
	O_HTABLE_GCINT,
	O_HTABLE_EXPIRE,
	O_RATEMATCH,
	O_INTERVAL,
	F_BURST         = 1 << O_BURST,
	F_UPTO          = 1 << O_UPTO,
	F_ABOVE         = 1 << O_ABOVE,
	F_HTABLE_EXPIRE = 1 << O_HTABLE_EXPIRE,
	F_RATEMATCH	= 1 << O_RATEMATCH,
};

static void hashlimit_mt_help(void)
{
	printf(
"hashlimit match options:\n"
"  --hashlimit-upto <avg>           max average match rate\n"
"                                   [Packets per second unless followed by \n"
"                                   /sec /minute /hour /day postfixes]\n"
"  --hashlimit-above <avg>          min average match rate\n"
"  --hashlimit-mode <mode>          mode is a comma-separated list of\n"
"                                   dstip,srcip,dstport,srcport (or none)\n"
"  --hashlimit-srcmask <length>     source address grouping prefix length\n"
"  --hashlimit-dstmask <length>     destination address grouping prefix length\n"
"  --hashlimit-name <name>          name for /proc/net/ipt_hashlimit\n"
"  --hashlimit-burst <num>	    number to match in a burst, default %u\n"
"  --hashlimit-htable-size <num>    number of hashtable buckets\n"
"  --hashlimit-htable-max <num>     number of hashtable entries\n"
"  --hashlimit-htable-gcinterval    interval between garbage collection runs\n"
"  --hashlimit-htable-expire        after which time are idle entries expired?\n"
"\n", XT_HASHLIMIT_BURST);
}

static void hashlimit_mt_help_v3(void)
{
	printf(
"hashlimit match options:\n"
"  --hashlimit-upto <avg>           max average match rate\n"
"                                   [Packets per second unless followed by \n"
"                                   /sec /minute /hour /day postfixes]\n"
"  --hashlimit-above <avg>          min average match rate\n"
"  --hashlimit-mode <mode>          mode is a comma-separated list of\n"
"                                   dstip,srcip,dstport,srcport (or none)\n"
"  --hashlimit-srcmask <length>     source address grouping prefix length\n"
"  --hashlimit-dstmask <length>     destination address grouping prefix length\n"
"  --hashlimit-name <name>          name for /proc/net/ipt_hashlimit\n"
"  --hashlimit-burst <num>	    number to match in a burst, default %u\n"
"  --hashlimit-htable-size <num>    number of hashtable buckets\n"
"  --hashlimit-htable-max <num>     number of hashtable entries\n"
"  --hashlimit-htable-gcinterval    interval between garbage collection runs\n"
"  --hashlimit-htable-expire        after which time are idle entries expired?\n"
"  --hashlimit-rate-match           rate match the flow without rate-limiting it\n"
"  --hashlimit-rate-interval        interval in seconds for hashlimit-rate-match\n"
"\n", XT_HASHLIMIT_BURST);
}

#define s struct xt_hashlimit_info
static const struct xt_option_entry hashlimit_opts[] = {
	{.name = "hashlimit", .id = O_UPTO, .excl = F_ABOVE,
	 .type = XTTYPE_STRING},
	{.name = "hashlimit-burst", .id = O_BURST, .type = XTTYPE_UINT32,
	 .min = 1, .max = XT_HASHLIMIT_BURST_MAX_v1, .flags = XTOPT_PUT,
	 XTOPT_POINTER(s, cfg.burst)},
	{.name = "hashlimit-htable-size", .id = O_HTABLE_SIZE,
	 .type = XTTYPE_UINT32, .flags = XTOPT_PUT,
	 XTOPT_POINTER(s, cfg.size)},
	{.name = "hashlimit-htable-max", .id = O_HTABLE_MAX,
	 .type = XTTYPE_UINT32, .flags = XTOPT_PUT,
	 XTOPT_POINTER(s, cfg.max)},
	{.name = "hashlimit-htable-gcinterval", .id = O_HTABLE_GCINT,
	 .type = XTTYPE_UINT32, .flags = XTOPT_PUT,
	 XTOPT_POINTER(s, cfg.gc_interval)},
	{.name = "hashlimit-htable-expire", .id = O_HTABLE_EXPIRE,
	 .type = XTTYPE_UINT32, .flags = XTOPT_PUT,
	 XTOPT_POINTER(s, cfg.expire)},
	{.name = "hashlimit-mode", .id = O_MODE, .type = XTTYPE_STRING,
	 .flags = XTOPT_MAND},
	{.name = "hashlimit-name", .id = O_NAME, .type = XTTYPE_STRING,
	 .flags = XTOPT_MAND | XTOPT_PUT, XTOPT_POINTER(s, name), .min = 1},
	XTOPT_TABLEEND,
};
#undef s

#define s struct xt_hashlimit_mtinfo1
static const struct xt_option_entry hashlimit_mt_opts_v1[] = {
	{.name = "hashlimit-upto", .id = O_UPTO, .excl = F_ABOVE,
	 .type = XTTYPE_STRING, .flags = XTOPT_INVERT},
	{.name = "hashlimit-above", .id = O_ABOVE, .excl = F_UPTO,
	 .type = XTTYPE_STRING, .flags = XTOPT_INVERT},
	{.name = "hashlimit", .id = O_UPTO, .excl = F_ABOVE,
	 .type = XTTYPE_STRING, .flags = XTOPT_INVERT}, /* old name */
	{.name = "hashlimit-srcmask", .id = O_SRCMASK, .type = XTTYPE_PLEN},
	{.name = "hashlimit-dstmask", .id = O_DSTMASK, .type = XTTYPE_PLEN},
	{.name = "hashlimit-burst", .id = O_BURST, .type = XTTYPE_STRING},
	{.name = "hashlimit-htable-size", .id = O_HTABLE_SIZE,
	 .type = XTTYPE_UINT32, .flags = XTOPT_PUT,
	 XTOPT_POINTER(s, cfg.size)},
	{.name = "hashlimit-htable-max", .id = O_HTABLE_MAX,
	 .type = XTTYPE_UINT32, .flags = XTOPT_PUT,
	 XTOPT_POINTER(s, cfg.max)},
	{.name = "hashlimit-htable-gcinterval", .id = O_HTABLE_GCINT,
	 .type = XTTYPE_UINT32, .flags = XTOPT_PUT,
	 XTOPT_POINTER(s, cfg.gc_interval)},
	{.name = "hashlimit-htable-expire", .id = O_HTABLE_EXPIRE,
	 .type = XTTYPE_UINT32, .flags = XTOPT_PUT,
	 XTOPT_POINTER(s, cfg.expire)},
	{.name = "hashlimit-mode", .id = O_MODE, .type = XTTYPE_STRING},
	{.name = "hashlimit-name", .id = O_NAME, .type = XTTYPE_STRING,
	 .flags = XTOPT_MAND | XTOPT_PUT, XTOPT_POINTER(s, name), .min = 1},
	XTOPT_TABLEEND,
};
#undef s

#define s struct xt_hashlimit_mtinfo2
static const struct xt_option_entry hashlimit_mt_opts_v2[] = {
	{.name = "hashlimit-upto", .id = O_UPTO, .excl = F_ABOVE,
	 .type = XTTYPE_STRING, .flags = XTOPT_INVERT},
	{.name = "hashlimit-above", .id = O_ABOVE, .excl = F_UPTO,
	 .type = XTTYPE_STRING, .flags = XTOPT_INVERT},
	{.name = "hashlimit", .id = O_UPTO, .excl = F_ABOVE,
	 .type = XTTYPE_STRING, .flags = XTOPT_INVERT}, /* old name */
	{.name = "hashlimit-srcmask", .id = O_SRCMASK, .type = XTTYPE_PLEN},
	{.name = "hashlimit-dstmask", .id = O_DSTMASK, .type = XTTYPE_PLEN},
	{.name = "hashlimit-burst", .id = O_BURST, .type = XTTYPE_STRING},
	{.name = "hashlimit-htable-size", .id = O_HTABLE_SIZE,
	 .type = XTTYPE_UINT32, .flags = XTOPT_PUT,
	 XTOPT_POINTER(s, cfg.size)},
	{.name = "hashlimit-htable-max", .id = O_HTABLE_MAX,
	 .type = XTTYPE_UINT32, .flags = XTOPT_PUT,
	 XTOPT_POINTER(s, cfg.max)},
	{.name = "hashlimit-htable-gcinterval", .id = O_HTABLE_GCINT,
	 .type = XTTYPE_UINT32, .flags = XTOPT_PUT,
	 XTOPT_POINTER(s, cfg.gc_interval)},
	{.name = "hashlimit-htable-expire", .id = O_HTABLE_EXPIRE,
	 .type = XTTYPE_UINT32, .flags = XTOPT_PUT,
	 XTOPT_POINTER(s, cfg.expire)},
	{.name = "hashlimit-mode", .id = O_MODE, .type = XTTYPE_STRING},
	{.name = "hashlimit-name", .id = O_NAME, .type = XTTYPE_STRING,
	 .flags = XTOPT_MAND | XTOPT_PUT, XTOPT_POINTER(s, name), .min = 1},
	XTOPT_TABLEEND,
};
#undef s

#define s struct xt_hashlimit_mtinfo3
static const struct xt_option_entry hashlimit_mt_opts[] = {
	{.name = "hashlimit-upto", .id = O_UPTO, .excl = F_ABOVE,
	 .type = XTTYPE_STRING, .flags = XTOPT_INVERT},
	{.name = "hashlimit-above", .id = O_ABOVE, .excl = F_UPTO,
	 .type = XTTYPE_STRING, .flags = XTOPT_INVERT},
	{.name = "hashlimit", .id = O_UPTO, .excl = F_ABOVE,
	 .type = XTTYPE_STRING, .flags = XTOPT_INVERT}, /* old name */
	{.name = "hashlimit-srcmask", .id = O_SRCMASK, .type = XTTYPE_PLEN},
	{.name = "hashlimit-dstmask", .id = O_DSTMASK, .type = XTTYPE_PLEN},
	{.name = "hashlimit-burst", .id = O_BURST, .type = XTTYPE_STRING},
	{.name = "hashlimit-htable-size", .id = O_HTABLE_SIZE,
	 .type = XTTYPE_UINT32, .flags = XTOPT_PUT,
	 XTOPT_POINTER(s, cfg.size)},
	{.name = "hashlimit-htable-max", .id = O_HTABLE_MAX,
	 .type = XTTYPE_UINT32, .flags = XTOPT_PUT,
	 XTOPT_POINTER(s, cfg.max)},
	{.name = "hashlimit-htable-gcinterval", .id = O_HTABLE_GCINT,
	 .type = XTTYPE_UINT32, .flags = XTOPT_PUT,
	 XTOPT_POINTER(s, cfg.gc_interval)},
	{.name = "hashlimit-htable-expire", .id = O_HTABLE_EXPIRE,
	 .type = XTTYPE_UINT32, .flags = XTOPT_PUT,
	 XTOPT_POINTER(s, cfg.expire)},
	{.name = "hashlimit-mode", .id = O_MODE, .type = XTTYPE_STRING},
	{.name = "hashlimit-name", .id = O_NAME, .type = XTTYPE_STRING,
	 .flags = XTOPT_MAND | XTOPT_PUT, XTOPT_POINTER(s, name), .min = 1},
	{.name = "hashlimit-rate-match", .id = O_RATEMATCH, .type = XTTYPE_NONE},
	{.name = "hashlimit-rate-interval", .id = O_INTERVAL, .type = XTTYPE_STRING},
	XTOPT_TABLEEND,
};
#undef s

static int
cfg_copy(struct hashlimit_cfg3 *to, const void *from, int revision)
{
	if (revision == 1) {
		struct hashlimit_cfg1 *cfg = (struct hashlimit_cfg1 *)from;

		to->mode = cfg->mode;
		to->avg = cfg->avg;
		to->burst = cfg->burst;
		to->size = cfg->size;
		to->max = cfg->max;
		to->gc_interval = cfg->gc_interval;
		to->expire = cfg->expire;
		to->srcmask = cfg->srcmask;
		to->dstmask = cfg->dstmask;
	} else if (revision == 2) {
		struct hashlimit_cfg2 *cfg = (struct hashlimit_cfg2 *)from;

		to->mode = cfg->mode;
		to->avg = cfg->avg;
		to->burst = cfg->burst;
		to->size = cfg->size;
		to->max = cfg->max;
		to->gc_interval = cfg->gc_interval;
		to->expire = cfg->expire;
		to->srcmask = cfg->srcmask;
		to->dstmask = cfg->dstmask;
	} else if (revision == 3) {
		memcpy(to, from, sizeof(struct hashlimit_cfg3));
	} else {
		return -EINVAL;
	}

	return 0;
}

static uint64_t cost_to_bytes(uint64_t cost)
{
	uint64_t r;

	r = cost ? UINT32_MAX / cost : UINT32_MAX;
	r = (r - 1) << XT_HASHLIMIT_BYTE_SHIFT;
	return r;
}

static uint64_t bytes_to_cost(uint64_t bytes)
{
	uint32_t r = bytes >> XT_HASHLIMIT_BYTE_SHIFT;
	return UINT32_MAX / (r+1);
}

static uint32_t get_factor(int chr)
{
	switch (chr) {
	case 'm': return 1024 * 1024;
	case 'k': return 1024;
	}
	return 1;
}

static void burst_error_v1(void)
{
	xtables_error(PARAMETER_PROBLEM, "bad value for option "
			"\"--hashlimit-burst\", or out of range (1-%u).", XT_HASHLIMIT_BURST_MAX_v1);
}

static void burst_error(void)
{
	xtables_error(PARAMETER_PROBLEM, "bad value for option "
			"\"--hashlimit-burst\", or out of range (1-%u).", XT_HASHLIMIT_BURST_MAX);
}

static uint64_t parse_burst(const char *burst, int revision)
{
	uintmax_t v;
	char *end;
	uint64_t max = (revision == 1) ? UINT32_MAX : UINT64_MAX;
	uint64_t burst_max = (revision == 1) ?
			      XT_HASHLIMIT_BURST_MAX_v1 : XT_HASHLIMIT_BURST_MAX;

	if (!xtables_strtoul(burst, &end, &v, 1, max) ||
		(*end == 0 && v > burst_max)) {
		if (revision == 1)
			burst_error_v1();
		else
			burst_error();
	}

	v *= get_factor(*end);
	if (v > max)
		xtables_error(PARAMETER_PROBLEM, "bad value for option "
			"\"--hashlimit-burst\", value \"%s\" too large "
				"(max %"PRIu64"mb).", burst, max/1024/1024);
	return v;
}

static bool parse_bytes(const char *rate, void *val, struct hashlimit_mt_udata *ud, int revision)
{
	unsigned int factor = 1;
	uint64_t tmp, r;
	const char *mode = strstr(rate, "b/s");
	uint64_t max = (revision == 1) ? UINT32_MAX : UINT64_MAX;

	if (!mode || mode == rate)
		return false;

	mode--;
	r = atoll(rate);
	if (r == 0)
		return false;

	factor = get_factor(*mode);
	tmp = (uint64_t) r * factor;
	if (tmp > max)
		xtables_error(PARAMETER_PROBLEM,
			      "Rate value too large \"%"PRIu64"\" (max %"PRIu64")",
			      tmp, max);

	tmp = bytes_to_cost(tmp);
	if (tmp == 0)
		xtables_error(PARAMETER_PROBLEM, "Rate too high \"%s\"", rate);

	ud->mult = XT_HASHLIMIT_BYTE_EXPIRE;

	if(revision == 1)
		*((uint32_t*)val) = tmp;
	else
		*((uint64_t*)val) = tmp;

	return true;
}

static
int parse_rate(const char *rate, void *val, struct hashlimit_mt_udata *ud, int revision)
{
	const char *delim;
	uint64_t tmp, r;
	uint64_t scale = (revision == 1) ? XT_HASHLIMIT_SCALE : XT_HASHLIMIT_SCALE_v2;

	ud->mult = 1;  /* Seconds by default. */
	delim = strchr(rate, '/');
	if (delim) {
		if (strlen(delim+1) == 0)
			return 0;

		if (strncasecmp(delim+1, "second", strlen(delim+1)) == 0)
			ud->mult = 1;
		else if (strncasecmp(delim+1, "minute", strlen(delim+1)) == 0)
			ud->mult = 60;
		else if (strncasecmp(delim+1, "hour", strlen(delim+1)) == 0)
			ud->mult = 60*60;
		else if (strncasecmp(delim+1, "day", strlen(delim+1)) == 0)
			ud->mult = 24*60*60;
		else
			return 0;
	}
	r = atoll(rate);
	if (!r)
		return 0;

	tmp = scale * ud->mult / r;
	if (tmp == 0)
		/*
		 * The rate maps to infinity. (1/day is the minimum they can
		 * specify, so we are ok at that end).
		 */
		xtables_error(PARAMETER_PROBLEM, "Rate too fast \"%s\"", rate);

	if(revision == 1)
		*((uint32_t*)val) = tmp;
	else
		*((uint64_t*)val) = tmp;

	return 1;
}

static int parse_interval(const char *rate, uint32_t *val)
{
	int r = atoi(rate);
	if (r <= 0)
		return 0;

	*val = r;
	return 1;
}

static void hashlimit_init(struct xt_entry_match *m)
{
	struct xt_hashlimit_info *r = (struct xt_hashlimit_info *)m->data;

	r->cfg.burst = XT_HASHLIMIT_BURST;
	r->cfg.gc_interval = XT_HASHLIMIT_GCINTERVAL;

}

static void hashlimit_mt4_init_v1(struct xt_entry_match *match)
{
	struct xt_hashlimit_mtinfo1 *info = (void *)match->data;

	info->cfg.mode        = 0;
	info->cfg.burst       = XT_HASHLIMIT_BURST;
	info->cfg.gc_interval = XT_HASHLIMIT_GCINTERVAL;
	info->cfg.srcmask     = 32;
	info->cfg.dstmask     = 32;
}

static void hashlimit_mt6_init_v1(struct xt_entry_match *match)
{
	struct xt_hashlimit_mtinfo1 *info = (void *)match->data;

	info->cfg.mode        = 0;
	info->cfg.burst       = XT_HASHLIMIT_BURST;
	info->cfg.gc_interval = XT_HASHLIMIT_GCINTERVAL;
	info->cfg.srcmask     = 128;
	info->cfg.dstmask     = 128;
}

static void hashlimit_mt4_init_v2(struct xt_entry_match *match)
{
	struct xt_hashlimit_mtinfo2 *info = (void *)match->data;

	info->cfg.mode        = 0;
	info->cfg.burst       = XT_HASHLIMIT_BURST;
	info->cfg.gc_interval = XT_HASHLIMIT_GCINTERVAL;
	info->cfg.srcmask     = 32;
	info->cfg.dstmask     = 32;
}

static void hashlimit_mt6_init_v2(struct xt_entry_match *match)
{
	struct xt_hashlimit_mtinfo2 *info = (void *)match->data;

	info->cfg.mode        = 0;
	info->cfg.burst       = XT_HASHLIMIT_BURST;
	info->cfg.gc_interval = XT_HASHLIMIT_GCINTERVAL;
	info->cfg.srcmask     = 128;
	info->cfg.dstmask     = 128;
}

static void hashlimit_mt4_init(struct xt_entry_match *match)
{
	struct xt_hashlimit_mtinfo3 *info = (void *)match->data;

	info->cfg.mode        = 0;
	info->cfg.burst       = XT_HASHLIMIT_BURST;
	info->cfg.gc_interval = XT_HASHLIMIT_GCINTERVAL;
	info->cfg.srcmask     = 32;
	info->cfg.dstmask     = 32;
	info->cfg.interval    = 0;
}

static void hashlimit_mt6_init(struct xt_entry_match *match)
{
	struct xt_hashlimit_mtinfo3 *info = (void *)match->data;

	info->cfg.mode        = 0;
	info->cfg.burst       = XT_HASHLIMIT_BURST;
	info->cfg.gc_interval = XT_HASHLIMIT_GCINTERVAL;
	info->cfg.srcmask     = 128;
	info->cfg.dstmask     = 128;
	info->cfg.interval    = 0;
}

/* Parse a 'mode' parameter into the required bitmask */
static int parse_mode(uint32_t *mode, const char *option_arg)
{
	char *tok;
	char *arg = xtables_strdup(option_arg);

	for (tok = strtok(arg, ",|");
	     tok;
	     tok = strtok(NULL, ",|")) {
		if (!strcmp(tok, "dstip"))
			*mode |= XT_HASHLIMIT_HASH_DIP;
		else if (!strcmp(tok, "srcip"))
			*mode |= XT_HASHLIMIT_HASH_SIP;
		else if (!strcmp(tok, "srcport"))
			*mode |= XT_HASHLIMIT_HASH_SPT;
		else if (!strcmp(tok, "dstport"))
			*mode |= XT_HASHLIMIT_HASH_DPT;
		else {
			free(arg);
			return -1;
		}
	}
	free(arg);
	return 0;
}

static void hashlimit_parse(struct xt_option_call *cb)
{
	struct xt_hashlimit_info *info = cb->data;

	xtables_option_parse(cb);
	switch (cb->entry->id) {
	case O_UPTO:
		if (!parse_rate(cb->arg, &info->cfg.avg, cb->udata, 1))
			xtables_param_act(XTF_BAD_VALUE, "hashlimit",
			          "--hashlimit-upto", cb->arg);
		break;
	case O_MODE:
		if (parse_mode(&info->cfg.mode, cb->arg) < 0)
			xtables_param_act(XTF_BAD_VALUE, "hashlimit",
			          "--hashlimit-mode", cb->arg);
		break;
	}
}

static void hashlimit_mt_parse_v1(struct xt_option_call *cb)
{
	struct xt_hashlimit_mtinfo1 *info = cb->data;

	xtables_option_parse(cb);
	switch (cb->entry->id) {
	case O_BURST:
		info->cfg.burst = parse_burst(cb->arg, 1);
		break;
	case O_UPTO:
		if (cb->invert)
			info->cfg.mode |= XT_HASHLIMIT_INVERT;
		if (parse_bytes(cb->arg, &info->cfg.avg, cb->udata, 1))
			info->cfg.mode |= XT_HASHLIMIT_BYTES;
		else if (!parse_rate(cb->arg, &info->cfg.avg, cb->udata, 1))
			xtables_param_act(XTF_BAD_VALUE, "hashlimit",
			          "--hashlimit-upto", cb->arg);
		break;
	case O_ABOVE:
		if (!cb->invert)
			info->cfg.mode |= XT_HASHLIMIT_INVERT;
		if (parse_bytes(cb->arg, &info->cfg.avg, cb->udata, 1))
			info->cfg.mode |= XT_HASHLIMIT_BYTES;
		else if (!parse_rate(cb->arg, &info->cfg.avg, cb->udata, 1))
			xtables_param_act(XTF_BAD_VALUE, "hashlimit",
			          "--hashlimit-above", cb->arg);
		break;
	case O_MODE:
		if (parse_mode(&info->cfg.mode, cb->arg) < 0)
			xtables_param_act(XTF_BAD_VALUE, "hashlimit",
			          "--hashlimit-mode", cb->arg);
		break;
	case O_SRCMASK:
		info->cfg.srcmask = cb->val.hlen;
		break;
	case O_DSTMASK:
		info->cfg.dstmask = cb->val.hlen;
		break;
	}
}

static void hashlimit_mt_parse_v2(struct xt_option_call *cb)
{
	struct xt_hashlimit_mtinfo2 *info = cb->data;

	xtables_option_parse(cb);
	switch (cb->entry->id) {
	case O_BURST:
		info->cfg.burst = parse_burst(cb->arg, 2);
		break;
	case O_UPTO:
		if (cb->invert)
			info->cfg.mode |= XT_HASHLIMIT_INVERT;
		if (parse_bytes(cb->arg, &info->cfg.avg, cb->udata, 2))
			info->cfg.mode |= XT_HASHLIMIT_BYTES;
		else if (!parse_rate(cb->arg, &info->cfg.avg, cb->udata, 2))
			xtables_param_act(XTF_BAD_VALUE, "hashlimit",
			          "--hashlimit-upto", cb->arg);
		break;
	case O_ABOVE:
		if (!cb->invert)
			info->cfg.mode |= XT_HASHLIMIT_INVERT;
		if (parse_bytes(cb->arg, &info->cfg.avg, cb->udata, 2))
			info->cfg.mode |= XT_HASHLIMIT_BYTES;
		else if (!parse_rate(cb->arg, &info->cfg.avg, cb->udata, 2))
			xtables_param_act(XTF_BAD_VALUE, "hashlimit",
			          "--hashlimit-above", cb->arg);
		break;
	case O_MODE:
		if (parse_mode(&info->cfg.mode, cb->arg) < 0)
			xtables_param_act(XTF_BAD_VALUE, "hashlimit",
			          "--hashlimit-mode", cb->arg);
		break;
	case O_SRCMASK:
		info->cfg.srcmask = cb->val.hlen;
		break;
	case O_DSTMASK:
		info->cfg.dstmask = cb->val.hlen;
		break;
	}
}

static void hashlimit_mt_parse(struct xt_option_call *cb)
{
	struct xt_hashlimit_mtinfo3 *info = cb->data;

	xtables_option_parse(cb);
	switch (cb->entry->id) {
	case O_BURST:
		info->cfg.burst = parse_burst(cb->arg, 2);
		break;
	case O_UPTO:
		if (cb->invert)
			info->cfg.mode |= XT_HASHLIMIT_INVERT;
		if (parse_bytes(cb->arg, &info->cfg.avg, cb->udata, 2))
			info->cfg.mode |= XT_HASHLIMIT_BYTES;
		else if (!parse_rate(cb->arg, &info->cfg.avg, cb->udata, 2))
			xtables_param_act(XTF_BAD_VALUE, "hashlimit",
			          "--hashlimit-upto", cb->arg);
		break;
	case O_ABOVE:
		if (!cb->invert)
			info->cfg.mode |= XT_HASHLIMIT_INVERT;
		if (parse_bytes(cb->arg, &info->cfg.avg, cb->udata, 2))
			info->cfg.mode |= XT_HASHLIMIT_BYTES;
		else if (!parse_rate(cb->arg, &info->cfg.avg, cb->udata, 2))
			xtables_param_act(XTF_BAD_VALUE, "hashlimit",
			          "--hashlimit-above", cb->arg);
		break;
	case O_MODE:
		if (parse_mode(&info->cfg.mode, cb->arg) < 0)
			xtables_param_act(XTF_BAD_VALUE, "hashlimit",
			          "--hashlimit-mode", cb->arg);
		break;
	case O_SRCMASK:
		info->cfg.srcmask = cb->val.hlen;
		break;
	case O_DSTMASK:
		info->cfg.dstmask = cb->val.hlen;
		break;
	case O_RATEMATCH:
		info->cfg.mode |= XT_HASHLIMIT_RATE_MATCH;
		break;
	case O_INTERVAL:
		if (!parse_interval(cb->arg, &info->cfg.interval))
			xtables_param_act(XTF_BAD_VALUE, "hashlimit",
				"--hashlimit-rate-interval", cb->arg);
	}
}

static void hashlimit_check(struct xt_fcheck_call *cb)
{
	const struct hashlimit_mt_udata *udata = cb->udata;
	struct xt_hashlimit_info *info = cb->data;

	if (!(cb->xflags & (F_UPTO | F_ABOVE)))
		xtables_error(PARAMETER_PROBLEM,
				"You have to specify --hashlimit");
	if (!(cb->xflags & F_HTABLE_EXPIRE))
		info->cfg.expire = udata->mult * 1000; /* from s to msec */
}

static void hashlimit_mt_check_v1(struct xt_fcheck_call *cb)
{
	const struct hashlimit_mt_udata *udata = cb->udata;
	struct xt_hashlimit_mtinfo1 *info = cb->data;

	if (!(cb->xflags & (F_UPTO | F_ABOVE)))
		xtables_error(PARAMETER_PROBLEM,
				"You have to specify --hashlimit");
	if (!(cb->xflags & F_HTABLE_EXPIRE))
		info->cfg.expire = udata->mult * 1000; /* from s to msec */

	if (info->cfg.mode & XT_HASHLIMIT_BYTES) {
		uint32_t burst = 0;
		if (cb->xflags & F_BURST) {
			if (info->cfg.burst < cost_to_bytes(info->cfg.avg))
				xtables_error(PARAMETER_PROBLEM,
					"burst cannot be smaller than %"PRIu64"b",
					cost_to_bytes(info->cfg.avg));

			burst = info->cfg.burst;
			burst /= cost_to_bytes(info->cfg.avg);
			if (info->cfg.burst % cost_to_bytes(info->cfg.avg))
				burst++;
			if (!(cb->xflags & F_HTABLE_EXPIRE))
				info->cfg.expire = XT_HASHLIMIT_BYTE_EXPIRE_BURST * 1000;
		}
		info->cfg.burst = burst;
	} else if (info->cfg.burst > XT_HASHLIMIT_BURST_MAX_v1)
		burst_error_v1();
}

static void hashlimit_mt_check_v2(struct xt_fcheck_call *cb)
{
	const struct hashlimit_mt_udata *udata = cb->udata;
	struct xt_hashlimit_mtinfo2 *info = cb->data;

	if (!(cb->xflags & (F_UPTO | F_ABOVE)))
		xtables_error(PARAMETER_PROBLEM,
				"You have to specify --hashlimit");
	if (!(cb->xflags & F_HTABLE_EXPIRE))
		info->cfg.expire = udata->mult * 1000; /* from s to msec */

	if (info->cfg.mode & XT_HASHLIMIT_BYTES) {
		uint32_t burst = 0;
		if (cb->xflags & F_BURST) {
			if (info->cfg.burst < cost_to_bytes(info->cfg.avg))
				xtables_error(PARAMETER_PROBLEM,
					"burst cannot be smaller than %"PRIu64"b",
					cost_to_bytes(info->cfg.avg));

			burst = info->cfg.burst;
			burst /= cost_to_bytes(info->cfg.avg);
			if (info->cfg.burst % cost_to_bytes(info->cfg.avg))
				burst++;
			if (!(cb->xflags & F_HTABLE_EXPIRE))
				info->cfg.expire = XT_HASHLIMIT_BYTE_EXPIRE_BURST * 1000;
		}
		info->cfg.burst = burst;
	} else if (info->cfg.burst > XT_HASHLIMIT_BURST_MAX)
		burst_error();
}

static void hashlimit_mt_check(struct xt_fcheck_call *cb)
{
	const struct hashlimit_mt_udata *udata = cb->udata;
	struct xt_hashlimit_mtinfo3 *info = cb->data;

	if (!(cb->xflags & (F_UPTO | F_ABOVE)))
		xtables_error(PARAMETER_PROBLEM,
				"You have to specify --hashlimit");
	if (!(cb->xflags & F_HTABLE_EXPIRE))
		info->cfg.expire = udata->mult * 1000; /* from s to msec */

	if (info->cfg.mode & XT_HASHLIMIT_BYTES) {
		uint32_t burst = 0;
		if (cb->xflags & F_BURST) {
			if (info->cfg.burst < cost_to_bytes(info->cfg.avg))
				xtables_error(PARAMETER_PROBLEM,
					"burst cannot be smaller than %"PRIu64"b", cost_to_bytes(info->cfg.avg));

			burst = info->cfg.burst;
			burst /= cost_to_bytes(info->cfg.avg);
			if (info->cfg.burst % cost_to_bytes(info->cfg.avg))
				burst++;
			if (!(cb->xflags & F_HTABLE_EXPIRE))
				info->cfg.expire = XT_HASHLIMIT_BYTE_EXPIRE_BURST * 1000;
		}
		info->cfg.burst = burst;
	} else if (info->cfg.burst > XT_HASHLIMIT_BURST_MAX)
		burst_error();

	if (cb->xflags & F_RATEMATCH) {
		if (!(info->cfg.mode & XT_HASHLIMIT_BYTES))
			info->cfg.avg /= udata->mult;

		if (info->cfg.interval == 0) {
			if (info->cfg.mode & XT_HASHLIMIT_BYTES)
				info->cfg.interval = 1;
			else
				info->cfg.interval = udata->mult;
		}
	}
}

struct rates {
	const char *name;
	uint64_t mult;
} rates_v1[] = { { "day", XT_HASHLIMIT_SCALE*24*60*60 },
		 { "hour", XT_HASHLIMIT_SCALE*60*60 },
		 { "min", XT_HASHLIMIT_SCALE*60 },
		 { "sec", XT_HASHLIMIT_SCALE } };

static const struct rates rates[] = {
	{ "day", XT_HASHLIMIT_SCALE_v2*24*60*60 },
	{ "hour", XT_HASHLIMIT_SCALE_v2*60*60 },
	{ "min", XT_HASHLIMIT_SCALE_v2*60 },
	{ "sec", XT_HASHLIMIT_SCALE_v2 } };

static uint32_t print_rate(uint64_t period, int revision)
{
	unsigned int i;
	const struct rates *_rates = (revision == 1) ? rates_v1 : rates;
	uint64_t scale = (revision == 1) ? XT_HASHLIMIT_SCALE : XT_HASHLIMIT_SCALE_v2;

	if (period == 0) {
		printf(" %f", INFINITY);
		return 0;
	}

	for (i = 1; i < ARRAY_SIZE(rates); ++i)
		if (period > _rates[i].mult
            || _rates[i].mult/period < _rates[i].mult%period)
			break;

	printf(" %"PRIu64"/%s", _rates[i-1].mult / period, _rates[i-1].name);
	/* return in msec */
	return _rates[i-1].mult / scale * 1000;
}

static const struct {
	const char *name;
	uint32_t thresh;
} units[] = {
	{ "m", 1024 * 1024 },
	{ "k", 1024 },
	{ "", 1 },
};

static uint32_t print_bytes(uint64_t avg, uint64_t burst, const char *prefix)
{
	unsigned int i;
	unsigned long long r;

	r = cost_to_bytes(avg);

	for (i = 0; i < ARRAY_SIZE(units) -1; ++i)
		if (r >= units[i].thresh &&
		    bytes_to_cost(r & ~(units[i].thresh - 1)) == avg)
			break;
	printf(" %llu%sb/s", r/units[i].thresh, units[i].name);

	if (burst == 0)
		return XT_HASHLIMIT_BYTE_EXPIRE * 1000;

	r *= burst;
	printf(" %s", prefix);
	for (i = 0; i < ARRAY_SIZE(units) -1; ++i)
		if (r >= units[i].thresh)
			break;

	printf("burst %llu%sb", r / units[i].thresh, units[i].name);
	return XT_HASHLIMIT_BYTE_EXPIRE_BURST * 1000;
}

static void print_mode(unsigned int mode, char separator)
{
	bool prevmode = false;

	putchar(' ');
	if (mode & XT_HASHLIMIT_HASH_SIP) {
		fputs("srcip", stdout);
		prevmode = 1;
	}
	if (mode & XT_HASHLIMIT_HASH_SPT) {
		if (prevmode)
			putchar(separator);
		fputs("srcport", stdout);
		prevmode = 1;
	}
	if (mode & XT_HASHLIMIT_HASH_DIP) {
		if (prevmode)
			putchar(separator);
		fputs("dstip", stdout);
		prevmode = 1;
	}
	if (mode & XT_HASHLIMIT_HASH_DPT) {
		if (prevmode)
			putchar(separator);
		fputs("dstport", stdout);
	}
}

static void hashlimit_print(const void *ip,
                            const struct xt_entry_match *match, int numeric)
{
	const struct xt_hashlimit_info *r = (const void *)match->data;
	uint32_t quantum;

	fputs(" limit: avg", stdout);
	quantum = print_rate(r->cfg.avg, 1);
	printf(" burst %u", r->cfg.burst);
	fputs(" mode", stdout);
	print_mode(r->cfg.mode, '-');
	if (r->cfg.size)
		printf(" htable-size %u", r->cfg.size);
	if (r->cfg.max)
		printf(" htable-max %u", r->cfg.max);
	if (r->cfg.gc_interval != XT_HASHLIMIT_GCINTERVAL)
		printf(" htable-gcinterval %u", r->cfg.gc_interval);
	if (r->cfg.expire != quantum)
		printf(" htable-expire %u", r->cfg.expire);
}

static void
hashlimit_mt_print(const struct hashlimit_cfg3 *cfg, unsigned int dmask, int revision)
{
	uint64_t quantum;
	uint64_t period;

	if (cfg->mode & XT_HASHLIMIT_INVERT)
		fputs(" limit: above", stdout);
	else
		fputs(" limit: up to", stdout);

	if (cfg->mode & XT_HASHLIMIT_BYTES) {
		quantum = print_bytes(cfg->avg, cfg->burst, "");
	} else {
		if (revision == 3) {
			period = cfg->avg;
			if (cfg->interval != 0)
				period *= cfg->interval;

			quantum = print_rate(period, revision);
		} else {
			quantum = print_rate(cfg->avg, revision);
		}
		printf(" burst %llu", cfg->burst);
	}
	if (cfg->mode & (XT_HASHLIMIT_HASH_SIP | XT_HASHLIMIT_HASH_SPT |
	    XT_HASHLIMIT_HASH_DIP | XT_HASHLIMIT_HASH_DPT)) {
		fputs(" mode", stdout);
		print_mode(cfg->mode, '-');
	}
	if (cfg->size != 0)
		printf(" htable-size %u", cfg->size);
	if (cfg->max != 0)
		printf(" htable-max %u", cfg->max);
	if (cfg->gc_interval != XT_HASHLIMIT_GCINTERVAL)
		printf(" htable-gcinterval %u", cfg->gc_interval);
	if (cfg->expire != quantum)
		printf(" htable-expire %u", cfg->expire);

	if (cfg->srcmask != dmask)
		printf(" srcmask %u", cfg->srcmask);
	if (cfg->dstmask != dmask)
		printf(" dstmask %u", cfg->dstmask);

	if ((revision == 3) && (cfg->mode & XT_HASHLIMIT_RATE_MATCH))
		printf(" rate-match");

	if ((revision == 3) && (cfg->mode & XT_HASHLIMIT_RATE_MATCH))
		if (cfg->interval != 1)
			printf(" rate-interval %u", cfg->interval);
}

static void
hashlimit_mt4_print_v1(const void *ip, const struct xt_entry_match *match,
                   int numeric)
{
	const struct xt_hashlimit_mtinfo1 *info = (const void *)match->data;
	struct hashlimit_cfg3 cfg;
	int ret;

	ret = cfg_copy(&cfg, (const void *)&info->cfg, 1);

	if (ret)
		xtables_error(OTHER_PROBLEM, "unknown revision");

	hashlimit_mt_print(&cfg, 32, 1);
}

static void
hashlimit_mt6_print_v1(const void *ip, const struct xt_entry_match *match,
                   int numeric)
{
	const struct xt_hashlimit_mtinfo1 *info = (const void *)match->data;
	struct hashlimit_cfg3 cfg;
	int ret;

	ret = cfg_copy(&cfg, (const void *)&info->cfg, 1);

	if (ret)
		xtables_error(OTHER_PROBLEM, "unknown revision");

	hashlimit_mt_print(&cfg, 128, 1);
}

static void
hashlimit_mt4_print_v2(const void *ip, const struct xt_entry_match *match,
                   int numeric)
{
	const struct xt_hashlimit_mtinfo2 *info = (const void *)match->data;
	struct hashlimit_cfg3 cfg;
	int ret;

	ret = cfg_copy(&cfg, (const void *)&info->cfg, 2);

	if (ret)
		xtables_error(OTHER_PROBLEM, "unknown revision");

	hashlimit_mt_print(&cfg, 32, 2);
}

static void
hashlimit_mt6_print_v2(const void *ip, const struct xt_entry_match *match,
                   int numeric)
{
	const struct xt_hashlimit_mtinfo2 *info = (const void *)match->data;
	struct hashlimit_cfg3 cfg;
	int ret;

	ret = cfg_copy(&cfg, (const void *)&info->cfg, 2);

	if (ret)
		xtables_error(OTHER_PROBLEM, "unknown revision");

	hashlimit_mt_print(&cfg, 128, 2);
}
static void
hashlimit_mt4_print(const void *ip, const struct xt_entry_match *match,
                   int numeric)
{
	const struct xt_hashlimit_mtinfo3 *info = (const void *)match->data;

	hashlimit_mt_print(&info->cfg, 32, 3);
}

static void
hashlimit_mt6_print(const void *ip, const struct xt_entry_match *match,
                   int numeric)
{
	const struct xt_hashlimit_mtinfo3 *info = (const void *)match->data;

	hashlimit_mt_print(&info->cfg, 128, 3);
}

static void hashlimit_save(const void *ip, const struct xt_entry_match *match)
{
	const struct xt_hashlimit_info *r = (const void *)match->data;
	uint32_t quantum;

	fputs(" --hashlimit", stdout);
	quantum = print_rate(r->cfg.avg, 1);
	printf(" --hashlimit-burst %u", r->cfg.burst);

	fputs(" --hashlimit-mode", stdout);
	print_mode(r->cfg.mode, ',');

	printf(" --hashlimit-name %s", r->name);

	if (r->cfg.size)
		printf(" --hashlimit-htable-size %u", r->cfg.size);
	if (r->cfg.max)
		printf(" --hashlimit-htable-max %u", r->cfg.max);
	if (r->cfg.gc_interval != XT_HASHLIMIT_GCINTERVAL)
		printf(" --hashlimit-htable-gcinterval %u", r->cfg.gc_interval);
	if (r->cfg.expire != quantum)
		printf(" --hashlimit-htable-expire %u", r->cfg.expire);
}

static void
hashlimit_mt_save(const struct hashlimit_cfg3 *cfg, const char* name, unsigned int dmask, int revision)
{
	uint32_t quantum;

	if (cfg->mode & XT_HASHLIMIT_INVERT)
		fputs(" --hashlimit-above", stdout);
	else
		fputs(" --hashlimit-upto", stdout);

	if (cfg->mode & XT_HASHLIMIT_BYTES) {
		quantum = print_bytes(cfg->avg, cfg->burst, "--hashlimit-");
	} else {
		quantum = print_rate(cfg->avg, revision);
		printf(" --hashlimit-burst %llu", cfg->burst);
	}

	if (cfg->mode & (XT_HASHLIMIT_HASH_SIP | XT_HASHLIMIT_HASH_SPT |
	    XT_HASHLIMIT_HASH_DIP | XT_HASHLIMIT_HASH_DPT)) {
		fputs(" --hashlimit-mode", stdout);
		print_mode(cfg->mode, ',');
	}

	printf(" --hashlimit-name %s", name);

	if (cfg->size != 0)
		printf(" --hashlimit-htable-size %u", cfg->size);
	if (cfg->max != 0)
		printf(" --hashlimit-htable-max %u", cfg->max);
	if (cfg->gc_interval != XT_HASHLIMIT_GCINTERVAL)
		printf(" --hashlimit-htable-gcinterval %u", cfg->gc_interval);
	if (cfg->expire != quantum)
		printf(" --hashlimit-htable-expire %u", cfg->expire);

	if (cfg->srcmask != dmask)
		printf(" --hashlimit-srcmask %u", cfg->srcmask);
	if (cfg->dstmask != dmask)
		printf(" --hashlimit-dstmask %u", cfg->dstmask);

	if ((revision == 3) && (cfg->mode & XT_HASHLIMIT_RATE_MATCH))
		printf(" --hashlimit-rate-match");

	if ((revision == 3) && (cfg->mode & XT_HASHLIMIT_RATE_MATCH))
		if (cfg->interval != 1)
			printf(" --hashlimit-rate-interval %u", cfg->interval);
}

static void
hashlimit_mt4_save_v1(const void *ip, const struct xt_entry_match *match)
{
	const struct xt_hashlimit_mtinfo1 *info = (const void *)match->data;
	struct hashlimit_cfg3 cfg;
	int ret;

	ret = cfg_copy(&cfg, (const void *)&info->cfg, 1);

	if (ret)
		xtables_error(OTHER_PROBLEM, "unknown revision");

	hashlimit_mt_save(&cfg, info->name, 32, 1);
}

static void
hashlimit_mt6_save_v1(const void *ip, const struct xt_entry_match *match)
{
	const struct xt_hashlimit_mtinfo1 *info = (const void *)match->data;
	struct hashlimit_cfg3 cfg;
	int ret;

	ret = cfg_copy(&cfg, (const void *)&info->cfg, 1);

	if (ret)
		xtables_error(OTHER_PROBLEM, "unknown revision");

	hashlimit_mt_save(&cfg, info->name, 128, 1);
}

static void
hashlimit_mt4_save_v2(const void *ip, const struct xt_entry_match *match)
{
	const struct xt_hashlimit_mtinfo2 *info = (const void *)match->data;
	struct hashlimit_cfg3 cfg;
	int ret;

	ret = cfg_copy(&cfg, (const void *)&info->cfg, 2);

	if (ret)
		xtables_error(OTHER_PROBLEM, "unknown revision");

	hashlimit_mt_save(&cfg, info->name, 32, 2);
}

static void
hashlimit_mt6_save_v2(const void *ip, const struct xt_entry_match *match)
{
	const struct xt_hashlimit_mtinfo2 *info = (const void *)match->data;
	struct hashlimit_cfg3 cfg;
	int ret;

	ret = cfg_copy(&cfg, (const void *)&info->cfg, 2);

	if (ret)
		xtables_error(OTHER_PROBLEM, "unknown revision");

	hashlimit_mt_save(&cfg, info->name, 128, 2);
}

static void
hashlimit_mt4_save(const void *ip, const struct xt_entry_match *match)
{
	const struct xt_hashlimit_mtinfo3 *info = (const void *)match->data;

	hashlimit_mt_save(&info->cfg, info->name, 32, 3);
}

static void
hashlimit_mt6_save(const void *ip, const struct xt_entry_match *match)
{
	const struct xt_hashlimit_mtinfo3 *info = (const void *)match->data;

	hashlimit_mt_save(&info->cfg, info->name, 128, 3);
}

static const struct rates rates_v1_xlate[] = {
	{ "day", XT_HASHLIMIT_SCALE * 24 * 60 * 60 },
	{ "hour", XT_HASHLIMIT_SCALE * 60 * 60 },
	{ "minute", XT_HASHLIMIT_SCALE * 60 },
	{ "second", XT_HASHLIMIT_SCALE } };

static const struct rates rates_xlate[] = {
	{ "day", XT_HASHLIMIT_SCALE_v2 * 24 * 60 * 60 },
	{ "hour", XT_HASHLIMIT_SCALE_v2 * 60 * 60 },
	{ "minute", XT_HASHLIMIT_SCALE_v2 * 60 },
	{ "second", XT_HASHLIMIT_SCALE_v2 } };

static void print_packets_rate_xlate(struct xt_xlate *xl, uint64_t avg,
				     int revision)
{
	unsigned int i;
	const struct rates *_rates = (revision == 1) ?
		rates_v1_xlate : rates_xlate;

	for (i = 1; i < ARRAY_SIZE(rates); ++i)
		if (avg > _rates[i].mult ||
		    _rates[i].mult / avg < _rates[i].mult % avg)
			break;

	xt_xlate_add(xl, " %" PRIu64 "/%s ",
		     _rates[i-1].mult / avg, _rates[i-1].name);
}

static void print_bytes_rate_xlate(struct xt_xlate *xl,
				   const struct hashlimit_cfg3 *cfg)
{
	unsigned int i;
	unsigned long long r;

	r = cost_to_bytes(cfg->avg);

	for (i = 0; i < ARRAY_SIZE(units) -1; ++i)
		if (r >= units[i].thresh &&
		    bytes_to_cost(r & ~(units[i].thresh - 1)) == cfg->avg)
			break;

	xt_xlate_add(xl, " %llu %sbytes/second", r / units[i].thresh,
		     units[i].name);

	r *= cfg->burst;
	for (i = 0; i < ARRAY_SIZE(units) -1; ++i)
		if (r >= units[i].thresh)
			break;

	if (cfg->burst > 0)
		xt_xlate_add(xl, " burst %llu %sbytes", r / units[i].thresh,
			     units[i].name);
}

static void hashlimit_print_subnet_xlate(struct xt_xlate *xl,
					 uint32_t nsub, int family)
{
	char sep = (family == NFPROTO_IPV4) ? '.' : ':';
	char *fmt = (family == NFPROTO_IPV4) ? "%u" : "%04x";
	unsigned int nblocks = (family == NFPROTO_IPV4) ? 4 : 8;
	unsigned int nbits = (family == NFPROTO_IPV4) ? 8 : 16;
	unsigned int acm, i;

	xt_xlate_add(xl, " and ");
	while (nblocks--) {
		acm = 0;

		for (i = 0; i < nbits; i++) {
			acm <<= 1;

			if (nsub > 0) {
				acm++;
				nsub--;
			}
		}

		xt_xlate_add_nospc(xl, fmt, acm);
		if (nblocks > 0)
			xt_xlate_add(xl, "%c", sep);
	}
}

static const char *const hashlimit_modes4_xlate[] = {
	[XT_HASHLIMIT_HASH_DIP]	= "ip daddr",
	[XT_HASHLIMIT_HASH_DPT]	= "tcp dport",
	[XT_HASHLIMIT_HASH_SIP]	= "ip saddr",
	[XT_HASHLIMIT_HASH_SPT]	= "tcp sport",
};

static const char *const hashlimit_modes6_xlate[] = {
	[XT_HASHLIMIT_HASH_DIP]	= "ip6 daddr",
	[XT_HASHLIMIT_HASH_DPT]	= "tcp dport",
	[XT_HASHLIMIT_HASH_SIP]	= "ip6 saddr",
	[XT_HASHLIMIT_HASH_SPT]	= "tcp sport",
};

static int hashlimit_mode_xlate(struct xt_xlate *xl,
				uint32_t mode, int family,
				unsigned int nsrc, unsigned int ndst)
{
	const char * const *_modes = (family == NFPROTO_IPV4) ?
		hashlimit_modes4_xlate : hashlimit_modes6_xlate;
	bool prevopt = false;
	unsigned int mask;

	mode &= ~XT_HASHLIMIT_INVERT & ~XT_HASHLIMIT_BYTES;

	for (mask = 1; mode > 0; mask <<= 1) {
		if (!(mode & mask))
			continue;

		if (!prevopt) {
			xt_xlate_add(xl, " ");
			prevopt = true;
		}
		else {
			xt_xlate_add(xl, " . ");
		}

		xt_xlate_add(xl, "%s", _modes[mask]);

		if (mask == XT_HASHLIMIT_HASH_DIP &&
		    ((family == NFPROTO_IPV4 && ndst != 32) ||
		     (family == NFPROTO_IPV6 && ndst != 128)))
			hashlimit_print_subnet_xlate(xl, ndst, family);
		else if (mask == XT_HASHLIMIT_HASH_SIP &&
			 ((family == NFPROTO_IPV4 && nsrc != 32) ||
			  (family == NFPROTO_IPV6 && nsrc != 128)))
			hashlimit_print_subnet_xlate(xl, nsrc, family);

		mode &= ~mask;
	}

	return prevopt;
}

static int hashlimit_mt_xlate(struct xt_xlate *xl, const char *name,
			      const struct hashlimit_cfg3 *cfg,
			      int revision, int family)
{
	int ret = 1;

	xt_xlate_add(xl, "meter %s {", name);
	ret = hashlimit_mode_xlate(xl, cfg->mode, family,
				   cfg->srcmask, cfg->dstmask);
	if (cfg->expire != 1000)
		xt_xlate_add(xl, " timeout %us", cfg->expire / 1000);
	xt_xlate_add(xl, " limit rate");

	if (cfg->mode & XT_HASHLIMIT_INVERT)
		xt_xlate_add(xl, " over");

	if (cfg->mode & XT_HASHLIMIT_BYTES)
		print_bytes_rate_xlate(xl, cfg);
	else {
		print_packets_rate_xlate(xl, cfg->avg, revision);
		if (cfg->burst != XT_HASHLIMIT_BURST)
			xt_xlate_add(xl, "burst %" PRIu64 " packets", (uint64_t)cfg->burst);

	}
	xt_xlate_add(xl, "}");

	return ret;
}

static int hashlimit_xlate(struct xt_xlate *xl,
			   const struct xt_xlate_mt_params *params)
{
	const struct xt_hashlimit_info *info = (const void *)params->match->data;
	int ret = 1;

	xt_xlate_add(xl, "meter %s {", info->name);
	ret = hashlimit_mode_xlate(xl, info->cfg.mode, NFPROTO_IPV4, 32, 32);
	xt_xlate_add(xl, " timeout %us limit rate", info->cfg.expire / 1000);
	print_packets_rate_xlate(xl, info->cfg.avg, 1);
	xt_xlate_add(xl, " burst %u packets", info->cfg.burst);
	xt_xlate_add(xl, "}");

	return ret;
}

static int hashlimit_mt4_xlate_v1(struct xt_xlate *xl,
				  const struct xt_xlate_mt_params *params)
{
	const struct xt_hashlimit_mtinfo1 *info =
		(const void *)params->match->data;
	struct hashlimit_cfg3 cfg;

	if (cfg_copy(&cfg, (const void *)&info->cfg, 1))
		xtables_error(OTHER_PROBLEM, "unknown revision");

	return hashlimit_mt_xlate(xl, info->name, &cfg, 1, NFPROTO_IPV4);
}

static int hashlimit_mt6_xlate_v1(struct xt_xlate *xl,
				  const struct xt_xlate_mt_params *params)
{
	const struct xt_hashlimit_mtinfo1 *info =
		(const void *)params->match->data;
	struct hashlimit_cfg3 cfg;

	if (cfg_copy(&cfg, (const void *)&info->cfg, 1))
		xtables_error(OTHER_PROBLEM, "unknown revision");

	return hashlimit_mt_xlate(xl, info->name, &cfg, 1, NFPROTO_IPV6);
}

static int hashlimit_mt4_xlate_v2(struct xt_xlate *xl,
				  const struct xt_xlate_mt_params *params)
{
	const struct xt_hashlimit_mtinfo2 *info =
		(const void *)params->match->data;
	struct hashlimit_cfg3 cfg;

	if (cfg_copy(&cfg, (const void *)&info->cfg, 2))
		xtables_error(OTHER_PROBLEM, "unknown revision");

	return hashlimit_mt_xlate(xl, info->name, &cfg, 2, NFPROTO_IPV4);
}

static int hashlimit_mt6_xlate_v2(struct xt_xlate *xl,
				  const struct xt_xlate_mt_params *params)
{
	const struct xt_hashlimit_mtinfo2 *info =
		(const void *)params->match->data;
	struct hashlimit_cfg3 cfg;

	if (cfg_copy(&cfg, (const void *)&info->cfg, 2))
		xtables_error(OTHER_PROBLEM, "unknown revision");

	return hashlimit_mt_xlate(xl, info->name, &cfg, 2, NFPROTO_IPV6);
}

static int hashlimit_mt4_xlate(struct xt_xlate *xl,
			       const struct xt_xlate_mt_params *params)
{
	const struct xt_hashlimit_mtinfo3 *info =
		(const void *)params->match->data;

	return hashlimit_mt_xlate(xl, info->name, &info->cfg, 3, NFPROTO_IPV4);
}

static int hashlimit_mt6_xlate(struct xt_xlate *xl,
			       const struct xt_xlate_mt_params *params)
{
	const struct xt_hashlimit_mtinfo3 *info =
		(const void *)params->match->data;

	return hashlimit_mt_xlate(xl, info->name, &info->cfg, 3, NFPROTO_IPV6);
}

static struct xtables_match hashlimit_mt_reg[] = {
	{
		.family        = NFPROTO_UNSPEC,
		.name          = "hashlimit",
		.version       = XTABLES_VERSION,
		.revision      = 0,
		.size          = XT_ALIGN(sizeof(struct xt_hashlimit_info)),
		.userspacesize = offsetof(struct xt_hashlimit_info, hinfo),
		.help          = hashlimit_help,
		.init          = hashlimit_init,
		.x6_parse      = hashlimit_parse,
		.x6_fcheck     = hashlimit_check,
		.print         = hashlimit_print,
		.save          = hashlimit_save,
		.x6_options    = hashlimit_opts,
		.udata_size    = sizeof(struct hashlimit_mt_udata),
		.xlate         = hashlimit_xlate,
	},
	{
		.version       = XTABLES_VERSION,
		.name          = "hashlimit",
		.revision      = 1,
		.family        = NFPROTO_IPV4,
		.size          = XT_ALIGN(sizeof(struct xt_hashlimit_mtinfo1)),
		.userspacesize = offsetof(struct xt_hashlimit_mtinfo1, hinfo),
		.help          = hashlimit_mt_help,
		.init          = hashlimit_mt4_init_v1,
		.x6_parse      = hashlimit_mt_parse_v1,
		.x6_fcheck     = hashlimit_mt_check_v1,
		.print         = hashlimit_mt4_print_v1,
		.save          = hashlimit_mt4_save_v1,
		.x6_options    = hashlimit_mt_opts_v1,
		.udata_size    = sizeof(struct hashlimit_mt_udata),
		.xlate         = hashlimit_mt4_xlate_v1,
	},
	{
		.version       = XTABLES_VERSION,
		.name          = "hashlimit",
		.revision      = 1,
		.family        = NFPROTO_IPV6,
		.size          = XT_ALIGN(sizeof(struct xt_hashlimit_mtinfo1)),
		.userspacesize = offsetof(struct xt_hashlimit_mtinfo1, hinfo),
		.help          = hashlimit_mt_help,
		.init          = hashlimit_mt6_init_v1,
		.x6_parse      = hashlimit_mt_parse_v1,
		.x6_fcheck     = hashlimit_mt_check_v1,
		.print         = hashlimit_mt6_print_v1,
		.save          = hashlimit_mt6_save_v1,
		.x6_options    = hashlimit_mt_opts_v1,
		.udata_size    = sizeof(struct hashlimit_mt_udata),
		.xlate         = hashlimit_mt6_xlate_v1,
	},
	{
		.version       = XTABLES_VERSION,
		.name          = "hashlimit",
		.revision      = 2,
		.family        = NFPROTO_IPV4,
		.size          = XT_ALIGN(sizeof(struct xt_hashlimit_mtinfo2)),
		.userspacesize = offsetof(struct xt_hashlimit_mtinfo2, hinfo),
		.help          = hashlimit_mt_help,
		.init          = hashlimit_mt4_init_v2,
		.x6_parse      = hashlimit_mt_parse_v2,
		.x6_fcheck     = hashlimit_mt_check_v2,
		.print         = hashlimit_mt4_print_v2,
		.save          = hashlimit_mt4_save_v2,
		.x6_options    = hashlimit_mt_opts_v2,
		.udata_size    = sizeof(struct hashlimit_mt_udata),
		.xlate         = hashlimit_mt4_xlate_v2,
	},
	{
		.version       = XTABLES_VERSION,
		.name          = "hashlimit",
		.revision      = 2,
		.family        = NFPROTO_IPV6,
		.size          = XT_ALIGN(sizeof(struct xt_hashlimit_mtinfo2)),
		.userspacesize = offsetof(struct xt_hashlimit_mtinfo2, hinfo),
		.help          = hashlimit_mt_help,
		.init          = hashlimit_mt6_init_v2,
		.x6_parse      = hashlimit_mt_parse_v2,
		.x6_fcheck     = hashlimit_mt_check_v2,
		.print         = hashlimit_mt6_print_v2,
		.save          = hashlimit_mt6_save_v2,
		.x6_options    = hashlimit_mt_opts_v2,
		.udata_size    = sizeof(struct hashlimit_mt_udata),
		.xlate         = hashlimit_mt6_xlate_v2,
	},
	{
		.version       = XTABLES_VERSION,
		.name          = "hashlimit",
		.revision      = 3,
		.family        = NFPROTO_IPV4,
		.size          = XT_ALIGN(sizeof(struct xt_hashlimit_mtinfo3)),
		.userspacesize = offsetof(struct xt_hashlimit_mtinfo3, hinfo),
		.help          = hashlimit_mt_help_v3,
		.init          = hashlimit_mt4_init,
		.x6_parse      = hashlimit_mt_parse,
		.x6_fcheck     = hashlimit_mt_check,
		.print         = hashlimit_mt4_print,
		.save          = hashlimit_mt4_save,
		.x6_options    = hashlimit_mt_opts,
		.udata_size    = sizeof(struct hashlimit_mt_udata),
		.xlate         = hashlimit_mt4_xlate,
	},
	{
		.version       = XTABLES_VERSION,
		.name          = "hashlimit",
		.revision      = 3,
		.family        = NFPROTO_IPV6,
		.size          = XT_ALIGN(sizeof(struct xt_hashlimit_mtinfo3)),
		.userspacesize = offsetof(struct xt_hashlimit_mtinfo3, hinfo),
		.help          = hashlimit_mt_help_v3,
		.init          = hashlimit_mt6_init,
		.x6_parse      = hashlimit_mt_parse,
		.x6_fcheck     = hashlimit_mt_check,
		.print         = hashlimit_mt6_print,
		.save          = hashlimit_mt6_save,
		.x6_options    = hashlimit_mt_opts,
		.udata_size    = sizeof(struct hashlimit_mt_udata),
		.xlate         = hashlimit_mt6_xlate,
	},
};

void _init(void)
{
	xtables_register_matches(hashlimit_mt_reg, ARRAY_SIZE(hashlimit_mt_reg));
}
