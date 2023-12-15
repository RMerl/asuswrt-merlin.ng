/*
 * extapi.h - external interface of netlink code
 *
 * Declarations needed by non-netlink code (mostly ethtool.c).
 */

#ifndef ETHTOOL_EXTAPI_H__
#define ETHTOOL_EXTAPI_H__

struct cmd_context;
struct nl_context;

typedef int (*nl_func_t)(struct cmd_context *);
typedef bool (*nl_chk_t)(struct cmd_context *);

#ifdef ETHTOOL_ENABLE_NETLINK

void netlink_run_handler(struct cmd_context *ctx, nl_chk_t nlchk,
			 nl_func_t nlfunc, bool no_fallback);

int nl_gset(struct cmd_context *ctx);
int nl_sset(struct cmd_context *ctx);
int nl_permaddr(struct cmd_context *ctx);
int nl_gfeatures(struct cmd_context *ctx);
int nl_sfeatures(struct cmd_context *ctx);
int nl_gprivflags(struct cmd_context *ctx);
int nl_sprivflags(struct cmd_context *ctx);
int nl_gring(struct cmd_context *ctx);
int nl_sring(struct cmd_context *ctx);
int nl_gchannels(struct cmd_context *ctx);
int nl_schannels(struct cmd_context *ctx);
int nl_gcoalesce(struct cmd_context *ctx);
int nl_scoalesce(struct cmd_context *ctx);
int nl_gpause(struct cmd_context *ctx);
int nl_spause(struct cmd_context *ctx);
int nl_geee(struct cmd_context *ctx);
int nl_seee(struct cmd_context *ctx);
int nl_tsinfo(struct cmd_context *ctx);
int nl_cable_test(struct cmd_context *ctx);
int nl_cable_test_tdr(struct cmd_context *ctx);
int nl_gtunnels(struct cmd_context *ctx);
int nl_gfec(struct cmd_context *ctx);
int nl_sfec(struct cmd_context *ctx);
bool nl_gstats_chk(struct cmd_context *ctx);
int nl_gstats(struct cmd_context *ctx);
int nl_gmodule(struct cmd_context *ctx);
int nl_smodule(struct cmd_context *ctx);
int nl_monitor(struct cmd_context *ctx);
int nl_getmodule(struct cmd_context *ctx);
int nl_grss(struct cmd_context *ctx);

void nl_monitor_usage(void);

int nl_get_eeprom_page(struct cmd_context *ctx,
		       struct ethtool_module_eeprom *request);

#else /* ETHTOOL_ENABLE_NETLINK */

static inline void netlink_run_handler(struct cmd_context *ctx __maybe_unused,
				       nl_chk_t nlchk __maybe_unused,
				       nl_func_t nlfunc __maybe_unused,
				       bool no_fallback)
{
	if (no_fallback) {
		fprintf(stderr,
			"Command requires kernel netlink support which is not "
			"enabled in this ethtool binary\n");
		exit(1);
	}
}

static inline int nl_monitor(struct cmd_context *ctx __maybe_unused)
{
	fprintf(stderr, "Netlink not supported by ethtool, option --monitor unsupported.\n");
	return -EOPNOTSUPP;
}

static inline void nl_monitor_usage(void)
{
}

static inline int
nl_get_eeprom_page(struct cmd_context *ctx __maybe_unused,
		   struct ethtool_module_eeprom *request __maybe_unused)
{
	fprintf(stderr, "Netlink not supported by ethtool.\n");
	return -EOPNOTSUPP;
}

#define nl_gset			NULL
#define nl_sset			NULL
#define nl_permaddr		NULL
#define nl_gfeatures		NULL
#define nl_sfeatures		NULL
#define nl_gprivflags		NULL
#define nl_sprivflags		NULL
#define nl_gring		NULL
#define nl_sring		NULL
#define nl_gchannels		NULL
#define nl_schannels		NULL
#define nl_gcoalesce		NULL
#define nl_scoalesce		NULL
#define nl_gpause		NULL
#define nl_spause		NULL
#define nl_geee			NULL
#define nl_seee			NULL
#define nl_tsinfo		NULL
#define nl_cable_test		NULL
#define nl_cable_test_tdr	NULL
#define nl_gtunnels		NULL
#define nl_gfec			NULL
#define nl_sfec			NULL
#define nl_gstats_chk		NULL
#define nl_gstats		NULL
#define nl_getmodule		NULL
#define nl_gmodule		NULL
#define nl_smodule		NULL
#define nl_grss			NULL

#endif /* ETHTOOL_ENABLE_NETLINK */

#endif /* ETHTOOL_EXTAPI_H__ */
