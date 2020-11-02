/*
 * Frontend command-line utility client for ACSD
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id: acsd_cli.c 784556 2020-03-02 10:22:00Z $
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "acsd.h"

enum {
	CMDARG_COMMAND = 0,	/* cmdarg[] array indices. */
	CMDARG_PARAMETER = 1,
	CMDARG_VALUE = 2,
	CMDARG_MAX
};

typedef enum {
	ACTION_DO_COMMAND = 0,		/* Regular cli command */
	ACTION_SHOW_COMMANDS,		/* Show command list */
	ACTION_SHOW_USAGE,		/* Provide usage help */
	ACTION_REPORT_VARIABLES,	/* Dump all variables */
	ACTION_REPORT_EVERYTHING	/* Dump everything */
} cli_action_t;

typedef struct acs_cli_info {
	int socket;			/* Server connection socket number, -1 if closed */
	cli_action_t	action;		/* Type of action to perform. */
	const char *cmdarg[CMDARG_MAX];	/* ASCIZ String pointers: command, parameter, and value */
	const char *wl_ifname;		/* User specified server wl interface name */
	const char *server_host;	/* User specified server host (default=127.0.0.1) */
	const char *server_port;	/* User specified server port (def=ACSD_DFLT_CLI_PORT) */
	char cmd_buf[ACSD_BUFSIZE_4K];	/* Server command and response buffer */
} acs_cli_info_t;

#define DEFAULT_WL_IFNAME	"eth1"
#define DEFAULT_SERVER_HOST	"127.0.0.1"
#define xstr(s) str(s)		/* See http://gcc.gnu.org/onlinedocs/cpp/Stringification.html */
#define str(s) #s
#define DEFAULT_SERVER_PORT	xstr(ACSD_DFLT_CLI_PORT)	/* Stringified numeric value */

static int do_dump_bss(acs_cli_info_t *, int);
static int do_dump_chanim(acs_cli_info_t *, int);
static int do_dump_candidate(acs_cli_info_t *, int);
static int do_dump_cscore(acs_cli_info_t *, int);
static int do_dump_acs_record(acs_cli_info_t *, int);

typedef struct {
	const char *option;			/* The dump option string being processed */
	int (*func)(acs_cli_info_t *ctx, int rcount); /* Special dump function, NULL for text */
	const char *help_text;			/* Human readable help text for dump option list */
} dispatch_table_t;

static const dispatch_table_t dump_table[] = {
	{ "acs_record", do_dump_acs_record,	"ACSD Channel switch log records" },
	{ "acsd_stats", NULL,			"ACSD Statistics" },
	{ "bss",	do_dump_bss,		"BSS counters per channel" },
	{ "candidate",	do_dump_candidate,	"Channel switch candidate details" },
	{ "chanim",	do_dump_chanim,		"Channel Interference monitor counters" },
	{ "cscore",	do_dump_cscore,		"Channel switch candidate scores" },
	{ "dfsr",	NULL,			"ACSD DFS Reentry Information" },
	{ "scanresults", NULL,			"ACSD Scan results" },
	{ NULL, NULL }
};

/* ACS CLI command verbs (to distinguish them from variables). Needed for command building. */
static const char *
acsd_commands[] = {
	"autochannel",
	"csscan",
	"dump",
	"get",
	"info",
	"set",
	"status",	/* Not implemented on server ? */
	NULL
};

/* Known ACS daemon variables. There may be more variables in the driver. */
/* (Retrieving the list from the driver would actually be a better approach -- maybe later). */
static const char *
acsd_variables[] = {	/* Sorted alphabetically for nicer help display */
	"acs_chan_dwell_time",
	"acs_chan_flop_period",
	"acs_ci_scan_timeout",
	"acs_ci_scan_timer",
	"acs_cs_scan_timer",
	"acs_dfs",
	"acs_ci_scan_chanim_stats",
	"acs_flags",
	"acs_far_sta_rssi",
	"acs_nofcs_least_rssi",
	"acs_policy",
	"acs_scan_chanim_stats",
	"acs_scan_entry_expire",
	"acs_trigger_var",
	"acs_txdelay_cnt",
	"acs_txdelay_period",
	"acs_txdelay_ratio",
	"acs_tx_idle_cnt",
	"chanim_flags",
	"lockout_period",
	"max_acs",
	"mode",
	"msglevel",
	"sample_period",
	"threshold_time",
	"test_dfsr",		/* use ZDFS_2G or ZDFS_5G or full-CAC */
	"test_preclear",	/* use ZDFS_2G or ZDFS_5G */
	"zdfs_5g_move",		/* use ZDFS_5G and move on success */
	"zdfs_2g_move",		/* use ZDFS_2G and move on success */
	"zdfs_5g_preclear",	/* use ZDFS_5G but not move - ETSI only */
	"zdfs_2g_preclear",	/* use ZDFS_2G but not move - ETSI only */
	"bw_upgradable",
	"fallback_to_primary",
	"acs_switch_score_thresh",	/* channel switch score threshold */
	"ci_scan_txop_limit",   /* ci scan txop limit */
	"acs_txop_limit",	/* txop limit */
	"acs_switch_score_thresh_hi", /* TOTAL SCORE thresh comparsion before fallback */
	"acs_txop_limit_hi",	/* txop limit threshold before fall back to regular CAC */
	NULL
};

static const dispatch_table_t *
dispatch_lookup(const dispatch_table_t *dtable, const char *str)
{
	while (dtable->option) {
		if (!strcmp(dtable->option, str)) {
			return dtable;
		}
		++dtable;
	}
	return NULL;
}

static int
acsdc_table_lookup(const char **table, const char *str)
{
	int i;
	for (i = 0; table[i]; ++i) {
		if (strcmp(table[i], str) == 0) {
			return i;
		}
	}
	return BCME_NOTFOUND;
}

static int
show_command_list(void)
{
	int i;

	printf("Available commands:\n");
	for (i = 0; acsd_commands[i]; ++i) {
		printf("%12s%c", acsd_commands[i], ((i+1) % 6) ? ' ':'\n');
	}
	printf("\n");

	printf("Dump options:\n");
	for (i = 0; dump_table[i].help_text; ++i) {
		printf("%12s    %s\n", dump_table[i].option, dump_table[i].help_text);
	}
	printf("\n");

	printf("Available variables (more may exist in the daemon):\n");
	for (i = 0; acsd_variables[i]; ++i) {
		printf("%24s%c", acsd_variables[i], ((i+1) % 3) ? ' ':'\n');
	}
	printf("\n");

	return BCME_OK;
}

/*
 * Handlers and helper functions for processing and displaying various dump command responses.
 */
static void
acs_dump_ch_score_head(ch_score_t * scores, bool dump_all)
{
	int i;

	for (i = 0; i < CH_SCORE_MAX; ++i) {
		if (dump_all || scores[i].weight) {
			printf("%8.8s ", acs_ch_score_name(i));
		}
	}
	printf("\n");
}

static void
acs_dump_ch_score_body(ch_score_t * scores, bool dump_all)
{
	int i;

	for (i = 0; i < CH_SCORE_MAX; ++i) {
		ch_score_t *score = &scores[i];
		if (dump_all || score->weight) {
			printf("%8d ", score->score * score->weight);
		}
	}
	printf("\n");
}

static void
acs_dump_reason(ch_candidate_t * candi)
{
	printf("%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s\n",
		(candi->reason & ACS_INVALID_COEX)? "COEX " : "",
		(candi->reason & ACS_INVALID_INTF_CCA)? "INTF_CCA " : "",
		(candi->reason & ACS_INVALID_INTF_BGN)? "INTF_BGN " : "",
		(candi->reason & ACS_INVALID_OVLP)? "OVERLAP " : "",
		(candi->reason & ACS_INVALID_NOISE)? "NOISE " : "",
		(candi->reason & ACS_INVALID_ALIGN)? "NON-ALIGNED " : "",
		(candi->reason & ACS_INVALID_144)? "144 " : "",
		(candi->reason & ACS_INVALID_DFS)? "DFS " : "",
		(candi->reason & ACS_INVALID_CHAN_FLOP_PERIOD)? "CHAN_FLOP " : "",
		(candi->reason & ACS_INVALID_EXCL)? "EXCLUDE " : "",
		(candi->reason & ACS_INVALID_MISMATCH_SB)? "MISMATCH_SB " : "",
		(candi->reason & ACS_INVALID_SAMECHAN)? "SAMECHAN " : "",
		(candi->reason & ACS_INVALID_DFS_NO_11H) ? "DFS_NO_11H " : "",
		(candi->reason & ACS_INVALID_LPCHAN) ? "LP_CHAN " : "",
		(candi->reason & ACS_INVALID_AVOID_PREV) ? "AVOID_PREV " : "",
		(candi->reason & ACS_INVALID_NONDFS) ? "NON_DFS" : "");
}

static void
acs_dump_cscore_head(ch_candidate_t *cscore)
{
	printf("%7s %8s %3s %3s ", "Channel", "(Chspec)", "Use", "DFS");
	acs_dump_ch_score_head(cscore->chscore, TRUE);
}

static void
acs_dump_cscore_body(ch_candidate_t * candi)
{
	char buf[CHANSPEC_STR_LEN];

	printf("%7s (0x%04x)  %3s %s", wf_chspec_ntoa(candi->chspec, buf), candi->chspec,
		(candi->is_dfs) ? "dfs" : " - ",
		(candi->valid) ? "" : " Invalid:");

	if (candi->valid) {
		acs_dump_ch_score_body(candi->chscore, TRUE);
	} else {
		acs_dump_reason(candi);
	}
}

static void
acs_dump_candi(ch_candidate_t * candi)
{
	printf("Candidate channel spec: 0x%x\n", candi->chspec);
	printf(" Valid: %s\n", (candi->valid)? "TRUE" : "FALSE");

	if (candi->valid) {
		acs_dump_score(candi->chscore);
	} else {
		printf(" Reason: ");
		acs_dump_reason(candi);
	}
}

static int
do_dump_bss(acs_cli_info_t *ctx, int rcount)
{
	acs_chan_bssinfo_t *cur = (acs_chan_bssinfo_t *) ctx->cmd_buf;
	int ncis;
	int i;
	ncis = rcount / sizeof(acs_chan_bssinfo_t);

	printf("channel nCtrl nExt20 nExt40 nExt80\n");
	for (i = 0; i < ncis; i++) {
		printf("%3d  %5d%6d%7d%7d\n", cur->channel, cur->nCtrl,
			cur->nExt20, cur->nExt40, cur->nExt80);
		cur++;
	}
	return BCME_OK;
}

static int
do_dump_chanim(acs_cli_info_t *ctx, int rcount)
{
	wl_chanim_stats_t * chanim_stats = (wl_chanim_stats_t *) ctx->cmd_buf;
	int i, j;

	chanim_stats->version = ntohl(chanim_stats->version);
	chanim_stats->buflen = ntohl(chanim_stats->buflen);
	chanim_stats->count = ntohl(chanim_stats->count);

	if (!((chanim_stats->version == WL_CHANIM_STATS_VERSION) ||
		(chanim_stats->version == WL_CHANIM_STATS_V2))) {
		printf("Version Mismatch\n");
		return BCME_VERSION;
	}

	printf("Chanim Stats: version: %d, count: %d\n",
		chanim_stats->version, chanim_stats->count);
	if (chanim_stats->version == WL_CHANIM_STATS_V2) {
		chanim_stats_v2_t *statsv2 = (chanim_stats_v2_t *)chanim_stats->stats;
		printf("chanspec tx   inbss   obss   nocat   nopkt   doze   txop   "
				"goodtx  badtx  glitch   badplcp  knoise  timestamp\n");

		for (i = 0; i < chanim_stats->count; i++) {
			statsv2->chanspec = ntohs(statsv2->chanspec);
			printf("0x%4x\t", statsv2->chanspec);

			for (j = 0; j < CCASTATS_V2_MAX; j++)
				printf("%d\t", statsv2->ccastats[j]);

			printf("%d\t%d\t%d\t%d", ntohl(statsv2->glitchcnt),
					ntohl(statsv2->badplcp), statsv2->bgnoise,
					ntohl(statsv2->timestamp));
			printf("\n");
			statsv2 ++;
		}
	} else if (chanim_stats->version == WL_CHANIM_STATS_VERSION) {
		chanim_stats_t *statsv3 = (chanim_stats_t *)chanim_stats->stats;
		printf("chanspec tx   inbss   obss   nocat   nopkt   doze   txop   "
				"goodtx  badtx  glitch   badplcp  knoise  timestamp\n");

		for (i = 0; i < chanim_stats->count; i++) {
			statsv3->chanspec = ntohs(statsv3->chanspec);
			printf("0x%4x\t", statsv3->chanspec);

			for (j = 0; j < CCASTATS_MAX; j++)
				printf("%d\t", statsv3->ccastats[j]);

			printf("%d\t%d\t%d\t%d", ntohl(statsv3->glitchcnt),
					ntohl(statsv3->badplcp), statsv3->bgnoise,
					ntohl(statsv3->timestamp));
			printf("\n");
			statsv3 ++;
		}
	}
	return BCME_OK;
}

static int
do_dump_candidate_int(acs_cli_info_t *ctx, int rcount, bool pretty_display)
{
	ch_candidate_t *candi = (ch_candidate_t *) ctx->cmd_buf;
	int i, j;
	int count;
	enum { CSCORE_NONE, CSCORE_HEAD, CSCORE_BODY } cscore;

	cscore = (pretty_display) ? CSCORE_HEAD : CSCORE_NONE;

	count = rcount / sizeof(ch_candidate_t);

	for (i = 0; i < count; i++, candi++) {
		candi->chspec = ntohs(candi->chspec);
		candi->reason = ntohs(candi->reason);

		for (j = 0; j < CH_SCORE_MAX; j++) {
			candi->chscore[j].score = ntohl(candi->chscore[j].score);
			candi->chscore[j].weight = ntohl(candi->chscore[j].weight);
		}

		if (cscore == CSCORE_HEAD) {
			/* Display header line first time around. */
			printf("ACSD Candidate Scores for next Channel Switch:\n");
			acs_dump_cscore_head(candi);
			cscore = CSCORE_BODY;
		}

		if (cscore == CSCORE_BODY) {
			acs_dump_cscore_body(candi);
		} else {
			acs_dump_candi(candi);
		}
	}
	return BCME_OK;
}

static int
do_dump_candidate(acs_cli_info_t *ctx, int rcount)
{
	return do_dump_candidate_int(ctx, rcount, FALSE);
}

static int
do_dump_cscore(acs_cli_info_t *ctx, int rcount)
{
	return do_dump_candidate_int(ctx, rcount, TRUE);
}

static const char *
acs_trigger_name(int trigger)
{
	static const char *trig_str[APCS_MAX] = {
		"INIT", "IOCTL", "CHANIM", "TIMER",
		"CISCAN", "TXOP", "NONACS", "DFS-REENTRY", "TXFAIL", "ZDFS"
	};
	return (trigger < APCS_MAX) ? trig_str[trigger] : "(invalid)";
}

static int
do_dump_acs_record(acs_cli_info_t *ctx, int rcount)
{
	chanim_acs_record_t *result = (chanim_acs_record_t *) ctx->cmd_buf;
	int i;
	int count;
	char buf[CHANSPEC_STR_LEN];

	count = rcount / sizeof(chanim_acs_record_t);

	if (!result || (!count)) {
		printf("There is no ACS recorded\n");
		return BCME_OK;
	}

	printf("Timestamp(s) ACS-Trigger  ChanSpec  BG-Noise  CCA-Count Idle\n");
	for (i = 0; i < count; i++) {
		time_t ltime = ntohl(result->timestamp);
		printf("%8u \t%s \t%13s \t%d \t%d \t%d\n",
			(uint32)ltime,
			acs_trigger_name(result->trigger),
			wf_chspec_ntoa_ex(ntohs(result->selected_chspc), buf),
			result->bgnoise,
			result->ccastats,
			result->chan_idle);
		result++;
	}

	return BCME_OK;
}

/*
 * acsdc_check_resp_err() - check whether an error response was received rather than binary data.
 */
static int
acsdc_check_resp_err(acs_cli_info_t *context)
{
	if (strstr(context->cmd_buf, "ERR:")) {
		printf("%s\n", context->cmd_buf);
		return BCME_ERROR;
	}
	return BCME_OK;
}
/*
 * process_response() - common function to process server responses.
 */
static int
process_response(acs_cli_info_t *ctx, int rcount)
{
	ctx->cmd_buf[rcount] = '\0';		/* Nul terminate the response, just in case. */

	/* Special handling for non-ascii "dump" command results. */

	if (!strcmp(ctx->cmdarg[CMDARG_COMMAND], "dump")) {
		const dispatch_table_t *dispatch;

		dispatch = dispatch_lookup(dump_table, ctx->cmdarg[CMDARG_PARAMETER]);
		if (dispatch && dispatch->func) {
			if (acsdc_check_resp_err(ctx)) {
				return BCME_OK;	/* acsdc_check_resp_err() displays error msg */
			}
			return dispatch->func(ctx, rcount);
		}
	}

	/* Display the generic response text message. */
	if (ctx->cmd_buf[0]) printf("%s\n", ctx->cmd_buf);

	return BCME_OK;
}
/*
 * Print the usage of acsd_cli utility
 */
static void
usage(const char *pname)
{
	printf("usage: %s [options] <command>|<variable>[ <value>]\n"
		"options are:\n"
		"   -i, --interface <ifname>  Apply command to specified wl interface on server\n"
		"   -p, --port <port>         Server TCP Port of remote ACSD\n"
		"   -s, --server <server>     Server address of remote ACSD (IP address or name)\n"
		"   -C, --commands            Lists available commands and variables to get/set\n"
		"   -S, --settings            Retrieves and displays all known variables\n"
		"   -R, --report              Retrieves and displays all known information.\n"
		"   -h, --help                This help text.\n"
		"\n"
		"NOTE:- Start the acsd on target to use this command\n", pname);
}
/*
 * parse_commandline() - Parse the user provided command line directly into the context fields.
 */
static int
parse_commandline(acs_cli_info_t *ctx, int argc, char *argv[])
{
	int argn = 1;
	int cmdidx = CMDARG_COMMAND;

	/* Process command line options */

	while ((argn < argc) && (argv[argn][0] == '-')) {
		int have_arg = (argn < (argc-1));
		char *opt = argv[argn];

		if ((!strcmp(opt, "-i") || !strcmp(opt, "--interface")) && have_arg) {
			ctx->wl_ifname = argv[++argn];
		} else
		if ((!strcmp(opt, "-s") || !strcmp(opt, "--server")) && have_arg) {
			ctx->server_host = argv[++argn];
		} else
		if ((!strcmp(opt, "-p") || !strcmp(opt, "--port")) && have_arg) {
			ctx->server_port = argv[++argn];
		} else
		if (!strcmp(opt, "-C") || !strcmp(opt, "--commands")) {
			ctx->action = ACTION_SHOW_COMMANDS;
			return BCME_OK;
		} else
		if (!strcmp(opt, "-R") || !strcmp(opt, "--report")) {
			ctx->action = ACTION_REPORT_EVERYTHING;
			return BCME_OK;
		} else
		if (!strcmp(opt, "-S") || !strcmp(opt, "--settings")) {
			ctx->action = ACTION_REPORT_VARIABLES;
			return BCME_OK;
		} else
		if (!strcmp(opt, "-?") ||!strcmp(opt, "-h") || !strcmp(opt, "--help")) {
			ctx->action = ACTION_SHOW_USAGE;
			return BCME_OK;
		} else {
			fprintf(stderr, "%s: unrecognized option '%s'\n"
				"Try '%s --help' for more information.\n",
				argv[0], opt, argv[0]);
			return BCME_BADOPTION;
		}
		++argn;
	}

	/* Process remaining arguments. Format : "command [param [value]]" */

	while (argn < argc) {
		if (cmdidx < CMDARG_MAX) {
			ctx->cmdarg[cmdidx++] = argv[argn];
		} else {
			/* Complain about any leftover arguments */
			fprintf(stderr, "Excess argument \"%s\"\n", argv[argn]);
		}
		++argn;
	}

	return ((cmdidx > 0) && (cmdidx <= CMDARG_MAX)) ? BCME_OK : BCME_BADARG;
}

/*
 * connect_to_server() - Establish a TCP connection to the ACSD server command port.
 *
 * On success, the context socket is updated and BCME_OK is returned.
 *
 */
static int
connect_to_server(acs_cli_info_t *ctx)
{
	struct addrinfo hints = {};
	struct addrinfo *results, *target;
	int rc, sock;

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	rc = getaddrinfo(ctx->server_host, ctx->server_port, &hints, &results);
	if (rc) {
		fprintf(stderr, "%s port %s error %s\n", ctx->server_host, ctx->server_port,
			gai_strerror(rc));
		return BCME_BADADDR;
	}
	for (target = results; target; target = target->ai_next) {
		sock = socket(target->ai_family, target->ai_socktype, target->ai_protocol);
		if (sock == -1)
			continue;

		if (connect(sock, target->ai_addr, target->ai_addrlen) != -1)
			break; /* success */

		close(sock);

	}
	freeaddrinfo(results);

	if (!target) {
		fprintf(stderr, "Could not connect to %s port %s.\n",
			ctx->server_host, ctx->server_port);
		return BCME_BADADDR;
	}

	ctx->socket = sock;
	return BCME_OK;
}

static int
disconnect_from_server(acs_cli_info_t *ctx)
{
	if (ctx->socket < 0)
		return BCME_ERROR;
	close(ctx->socket);
	ctx->socket = -1;
	return BCME_OK;
}

/*
 * create_server_command() - Set up the ASCIZ command string to send to the server.
 *
 * Returns the length of the command buffer (including the trailing nul byte) on success.
 * Returns a negative value otherwise.
 */
static int
create_server_command(acs_cli_info_t *ctx)
{
	int len;

	if (acsdc_table_lookup(acsd_commands, ctx->cmdarg[CMDARG_COMMAND]) == BCME_NOTFOUND) {
		/*
		 * This is not a regular command, assume it to be a variable name to get/set.
		 * Shift command arguments right by one, and set up the get/set command as needed.
		 */
		if (ctx->cmdarg[CMDARG_VALUE][0]) {
			fprintf(stderr, "get/set variable: Discarding arg \"%s\"\n",
				ctx->cmdarg[CMDARG_VALUE]);
		}
		ctx->cmdarg[CMDARG_VALUE] = ctx->cmdarg[CMDARG_PARAMETER];
		ctx->cmdarg[CMDARG_PARAMETER] = ctx->cmdarg[CMDARG_COMMAND];
		ctx->cmdarg[CMDARG_COMMAND] = (ctx->cmdarg[CMDARG_VALUE][0]) ? "set" : "get";
	}

	len = acs_snprintf(ctx->cmd_buf, ACSD_BUFSIZE_4K, "%s&ifname=%s",
		ctx->cmdarg[CMDARG_COMMAND], ctx->wl_ifname);

	if (ctx->cmdarg[CMDARG_PARAMETER][0]) {
		len += acs_snprintf(ctx->cmd_buf+len, ACSD_BUFSIZE_4K - len,
			"&param=%s", ctx->cmdarg[CMDARG_PARAMETER]);
	}

	if (ctx->cmdarg[CMDARG_VALUE][0]) {
		len += acs_snprintf(ctx->cmd_buf+len, ACSD_BUFSIZE_4K - len,
			"&value=%s", ctx->cmdarg[CMDARG_VALUE]);
	}
	++len;	/* Add the trailing nul byte, not included in len so far */

	if (len >= ACSD_BUFSIZE_4K) {
		fprintf(stderr, "Error setting up server command string: %s\n", strerror(errno));
		return -1;	/* Return negative length */
	}
	return len;
}

/*
 * do_command_response() - set up and send a command to the server, read and process the response.
 */
static int
do_command_response(acs_cli_info_t *ctx)
{
	int len;

	if (ctx->socket < 0)
		return BCME_ERROR;

	len = create_server_command(ctx);
	if (len < 0) {
		return -1;
	}

	/* Send it */
	if (swrite(ctx->socket, ctx->cmd_buf, len) < len) {
		fprintf(stderr, "Failed to send command to server: %s\n", strerror(errno));
		return BCME_ERROR;
	}

	/* Help server get the data till EOF */
	shutdown(ctx->socket, SHUT_WR);

	/* Read the response */

	len = sread(ctx->socket, ctx->cmd_buf, ACSD_BUFSIZE_4K-1);
	if (len < 0) {
		fprintf(stderr, "Failed to read response from server: %s\n", strerror(errno));
		return BCME_ERROR;
	}

	/* Process the response */
	return process_response(ctx, len);
}

static int
report_all_variables(acs_cli_info_t *ctx)
{
	int rc = BCME_OK;
	int i;

	ctx->cmdarg[CMDARG_COMMAND] = "get";

	printf("[ %s: acsd variables ]\n", ctx->wl_ifname);

	for (i = 0; (acsd_variables[i] && (rc == BCME_OK)); ++i) {
		rc = connect_to_server(ctx);
		if (rc == BCME_OK) {
			printf("%25s = ", acsd_variables[i]);
			fflush(stdout);
			ctx->cmdarg[CMDARG_PARAMETER] = acsd_variables[i];
			rc = do_command_response(ctx);
			disconnect_from_server(ctx);
		}
	}
	return rc;
}

static int
report_all_dumps(acs_cli_info_t *ctx)
{
	int rc = BCME_OK;
	int i;

	ctx->cmdarg[CMDARG_COMMAND] = "dump";

	for (i = 0; (dump_table[i].option && (rc == 0)); ++i) {
		rc = connect_to_server(ctx);
		if (rc == BCME_OK) {
			printf("[ %s: dump %s ]\n", ctx->wl_ifname, dump_table[i].option);
			ctx->cmdarg[CMDARG_PARAMETER] = dump_table[i].option;
			rc = do_command_response(ctx);
			disconnect_from_server(ctx);
		}
	}
	return rc;
}

static int
report_everything(acs_cli_info_t *ctx)
{
	int rc;

	rc = connect_to_server(ctx);
	if (rc == BCME_OK) {
		printf("[ info ]\n");
		ctx->cmdarg[CMDARG_COMMAND] = "info";
		rc = do_command_response(ctx);
		disconnect_from_server(ctx);
	}

	if (rc == BCME_OK) {
		rc = report_all_dumps(ctx);
		if (rc == BCME_OK) {
			rc = report_all_variables(ctx);
		}
	}
	return rc;
}

int main(int argc, char **argv)
{
	static acs_cli_info_t ai;
	int rc;

	/* Initalise our context object */
	memset(&ai, 0, sizeof(ai));
	ai.wl_ifname	= DEFAULT_WL_IFNAME;
	ai.server_host	= DEFAULT_SERVER_HOST;
	ai.server_port	= DEFAULT_SERVER_PORT;
	ai.socket	= -1;
	for (rc = 0; rc < CMDARG_MAX; ++rc) {
		ai.cmdarg[rc] = "";
	}

	rc = parse_commandline(&ai, argc, argv);
	if (rc != BCME_OK) {
		return rc;
	}

	switch (ai.action) {

	case ACTION_SHOW_COMMANDS:
		rc = show_command_list();
		break;

	case ACTION_SHOW_USAGE:
		rc = BCME_OK;
		usage(argv[0]);
		break;

	case ACTION_REPORT_VARIABLES:
		rc = report_all_variables(&ai);
		break;

	case ACTION_REPORT_EVERYTHING:
		rc = report_everything(&ai);
		break;

	case ACTION_DO_COMMAND:
		rc = connect_to_server(&ai);
		if (rc == BCME_OK) {
			rc = do_command_response(&ai);
			disconnect_from_server(&ai);
		}
		break;
	}
	return rc;
}
