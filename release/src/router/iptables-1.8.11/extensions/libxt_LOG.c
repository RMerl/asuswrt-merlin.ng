#include <stdio.h>
#include <string.h>
#define SYSLOG_NAMES
#include <syslog.h>
#include <xtables.h>
#include <linux/netfilter/xt_LOG.h>

#define LOG_DEFAULT_LEVEL LOG_WARNING

enum {
	/* make sure the values correspond with XT_LOG_* bit positions */
	O_LOG_TCPSEQ = 0,
	O_LOG_TCPOPTS,
	O_LOG_IPOPTS,
	O_LOG_UID,
	__O_LOG_NFLOG,
	O_LOG_MAC,
	O_LOG_LEVEL,
	O_LOG_PREFIX,
};

static void LOG_help(void)
{
	printf(
"LOG target options:\n"
" --log-level level		Level of logging (numeric or see syslog.conf)\n"
" --log-prefix prefix		Prefix log messages with this prefix.\n"
" --log-tcp-sequence		Log TCP sequence numbers.\n"
" --log-tcp-options		Log TCP options.\n"
" --log-ip-options		Log IP options.\n"
" --log-uid			Log UID owning the local socket.\n"
" --log-macdecode		Decode MAC addresses and protocol.\n");
}

#define s struct xt_log_info
static const struct xt_option_entry LOG_opts[] = {
	{.name = "log-level", .id = O_LOG_LEVEL, .type = XTTYPE_SYSLOGLEVEL,
	 .flags = XTOPT_PUT, XTOPT_POINTER(s, level)},
	{.name = "log-prefix", .id = O_LOG_PREFIX, .type = XTTYPE_STRING,
	 .flags = XTOPT_PUT, XTOPT_POINTER(s, prefix), .min = 1},
	{.name = "log-tcp-sequence", .id = O_LOG_TCPSEQ, .type = XTTYPE_NONE},
	{.name = "log-tcp-options", .id = O_LOG_TCPOPTS, .type = XTTYPE_NONE},
	{.name = "log-ip-options", .id = O_LOG_IPOPTS, .type = XTTYPE_NONE},
	{.name = "log-uid", .id = O_LOG_UID, .type = XTTYPE_NONE},
	{.name = "log-macdecode", .id = O_LOG_MAC, .type = XTTYPE_NONE},
	XTOPT_TABLEEND,
};
#undef s

static void LOG_init(struct xt_entry_target *t)
{
	struct xt_log_info *loginfo = (void *)t->data;

	loginfo->level = LOG_DEFAULT_LEVEL;
}

static void LOG_parse(struct xt_option_call *cb)
{
	struct xt_log_info *info = cb->data;

	xtables_option_parse(cb);
	switch (cb->entry->id) {
	case O_LOG_PREFIX:
		if (strchr(cb->arg, '\n') != NULL)
			xtables_error(PARAMETER_PROBLEM,
				   "Newlines not allowed in --log-prefix");
		break;
	case O_LOG_TCPSEQ:
	case O_LOG_TCPOPTS:
	case O_LOG_IPOPTS:
	case O_LOG_UID:
	case O_LOG_MAC:
		info->logflags |= 1 << cb->entry->id;
		break;
	}
}

static const char *priority2name(unsigned char level)
{
	int i;

	for (i = 0; prioritynames[i].c_name; ++i) {
		if (level == prioritynames[i].c_val)
			return prioritynames[i].c_name;
	}
	return NULL;
}

static void LOG_print(const void *ip, const struct xt_entry_target *target,
                      int numeric)
{
	const struct xt_log_info *loginfo = (const void *)target->data;

	printf(" LOG");
	if (numeric)
		printf(" flags %u level %u",
		       loginfo->logflags, loginfo->level);
	else {
		const char *pname = priority2name(loginfo->level);

		if (pname)
			printf(" level %s", pname);
		else
			printf(" UNKNOWN level %u", loginfo->level);
		if (loginfo->logflags & XT_LOG_TCPSEQ)
			printf(" tcp-sequence");
		if (loginfo->logflags & XT_LOG_TCPOPT)
			printf(" tcp-options");
		if (loginfo->logflags & XT_LOG_IPOPT)
			printf(" ip-options");
		if (loginfo->logflags & XT_LOG_UID)
			printf(" uid");
		if (loginfo->logflags & XT_LOG_MACDECODE)
			printf(" macdecode");
		if (loginfo->logflags & ~(XT_LOG_MASK))
			printf(" unknown-flags");
	}

	if (strcmp(loginfo->prefix, "") != 0)
		printf(" prefix \"%s\"", loginfo->prefix);
}

static void LOG_save(const void *ip, const struct xt_entry_target *target)
{
	const struct xt_log_info *loginfo = (const void *)target->data;

	if (strcmp(loginfo->prefix, "") != 0) {
		printf(" --log-prefix");
		xtables_save_string(loginfo->prefix);
	}

	if (loginfo->level != LOG_DEFAULT_LEVEL)
		printf(" --log-level %d", loginfo->level);

	if (loginfo->logflags & XT_LOG_TCPSEQ)
		printf(" --log-tcp-sequence");
	if (loginfo->logflags & XT_LOG_TCPOPT)
		printf(" --log-tcp-options");
	if (loginfo->logflags & XT_LOG_IPOPT)
		printf(" --log-ip-options");
	if (loginfo->logflags & XT_LOG_UID)
		printf(" --log-uid");
	if (loginfo->logflags & XT_LOG_MACDECODE)
		printf(" --log-macdecode");
}

static int LOG_xlate(struct xt_xlate *xl,
		     const struct xt_xlate_tg_params *params)
{
	const struct xt_log_info *loginfo = (const void *)params->target->data;
	const char *pname = priority2name(loginfo->level);

	xt_xlate_add(xl, "log");
	if (strcmp(loginfo->prefix, "") != 0)
		xt_xlate_add(xl, " prefix \"%s\"", loginfo->prefix);

	if (loginfo->level != LOG_DEFAULT_LEVEL && pname)
		xt_xlate_add(xl, " level %s", pname);
	else if (!pname)
		return 0;

	if ((loginfo->logflags & XT_LOG_MASK) == XT_LOG_MASK) {
		xt_xlate_add(xl, " flags all");
	} else {
		if (loginfo->logflags & (XT_LOG_TCPSEQ | XT_LOG_TCPOPT)) {
			const char *delim = " ";

			xt_xlate_add(xl, " flags tcp");
			if (loginfo->logflags & XT_LOG_TCPSEQ) {
				xt_xlate_add(xl, " sequence");
				delim = ",";
			}
			if (loginfo->logflags & XT_LOG_TCPOPT)
				xt_xlate_add(xl, "%soptions", delim);
		}
		if (loginfo->logflags & XT_LOG_IPOPT)
			xt_xlate_add(xl, " flags ip options");
		if (loginfo->logflags & XT_LOG_UID)
			xt_xlate_add(xl, " flags skuid");
		if (loginfo->logflags & XT_LOG_MACDECODE)
			xt_xlate_add(xl, " flags ether");
	}

	return 1;
}
static struct xtables_target log_tg_reg = {
	.name          = "LOG",
	.version       = XTABLES_VERSION,
	.family        = NFPROTO_UNSPEC,
	.size          = XT_ALIGN(sizeof(struct xt_log_info)),
	.userspacesize = XT_ALIGN(sizeof(struct xt_log_info)),
	.help          = LOG_help,
	.init          = LOG_init,
	.print         = LOG_print,
	.save          = LOG_save,
	.x6_parse      = LOG_parse,
	.x6_options    = LOG_opts,
	.xlate	       = LOG_xlate,
};

void _init(void)
{
	xtables_register_target(&log_tg_reg);
}
