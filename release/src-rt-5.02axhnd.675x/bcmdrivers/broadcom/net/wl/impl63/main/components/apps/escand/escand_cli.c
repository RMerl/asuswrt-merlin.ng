/*
 * Frontend command-line utility client for ESCAND
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
 * $Id: escand_cli.c 765135 2018-06-20 06:35:04Z $
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "escand.h"

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

typedef struct escand_cli_info {
	int socket;			/* Server connection socket number, -1 if closed */
	cli_action_t	action;		/* Type of action to perform. */
	const char *cmdarg[CMDARG_MAX];	/* ASCIZ String pointers: command, parameter, and value */
	const char *wl_ifname;		/* User specified server wl interface name */
	const char *server_host;	/* User specified server host (default=127.0.0.1) */
	const char *server_port;	/* User specified server port (def=ESCAND_DFLT_CLI_PORT) */
	char cmd_buf[ESCAND_BUFSIZE_4K];	/* Server command and response buffer */
} escand_cli_info_t;

#define DEFAULT_WL_IFNAME	"eth1"
#define DEFAULT_SERVER_HOST	"127.0.0.1"
#define xstr(s) str(s)		/* See http://gcc.gnu.org/onlinedocs/cpp/Stringification.html */
#define str(s) #s
#define DEFAULT_SERVER_PORT	xstr(ESCAND_DFLT_CLI_PORT)	/* Stringified numeric value */

static int do_dump_bss(escand_cli_info_t *, int);

typedef struct {
	const char *option;			/* The dump option string being processed */
	int (*func)(escand_cli_info_t *ctx, int rcount); /* Special dump function, NULL for text */
	const char *help_text;			/* Human readable help text for dump option list */
} dispatch_table_t;

static const dispatch_table_t dump_table[] = {
	{ "stats", 	NULL,			"ESCAND Statistics" },
	{ "bss",	do_dump_bss,		"BSS counters per channel" },
	{ "scanresults", NULL,			"ESCAND Scan results" },
	{ NULL, NULL }
};

/* ESCAND CLI command verbs (to distinguish them from variables). Needed for command building. */
static const char *
escand_commands[] = {
	"chanlist",
	/* "csscan", For future use */
	"dump",
	"get",
	"info",
	"readchans",
	"set",
	NULL
};

/* Known ESCAND daemon variables. There may be more variables in the driver. */
/* (Retrieving the list from the driver would actually be a better approach -- maybe later). */
static const char *
escand_variables[] = {	/* Sorted alphabetically for nicer help display */
	"escand_ci_scan_timeout",
	"escand_ci_scan_timer",
	"escand_cs_scan_timer",
	"escand_flags",
	"escand_far_sta_rssi",
	"escand_scan_entry_expire",
	"mode",
	"msglevel",
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
escandc_table_lookup(const char **table, const char *str)
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

	printf("\nAvailable commands:\n");
	for (i = 0; escand_commands[i]; ++i) {
		printf("%12s%c", escand_commands[i], ((i+1) % 6) ? ' ':'\n');
	}
	printf("\n");

	printf("\nDump options:\n");
	for (i = 0; dump_table[i].help_text; ++i) {
		printf("%12s    %s\n", dump_table[i].option, dump_table[i].help_text);
	}
	printf("\n");

	printf("\nAvailable variables (more may exist in the daemon):\n");
	for (i = 0; escand_variables[i]; ++i) {
		printf("%24s%c", escand_variables[i], ((i+1) % 3) ? ' ':'\n');
	}
	printf("\n");

	return BCME_OK;
}

static int
do_dump_bss(escand_cli_info_t *ctx, int rcount)
{
	escand_chan_bssinfo_t *cur = (escand_chan_bssinfo_t *) ctx->cmd_buf;
	int ncis;
	int i;
	ncis = rcount / sizeof(escand_chan_bssinfo_t);

	printf("channel nCtrl nExt20 nExt40 nExt80\n");
	for (i = 0; i < ncis; i++) {
		printf("%3d  %5d%6d%7d%7d\n", cur->channel, cur->nCtrl,
			cur->nExt20, cur->nExt40, cur->nExt80);
		cur++;
	}
	return BCME_OK;
}

/*
 * escandc_check_resp_err() - check whether an error response was received rather than binary data.
 */
static int
escandc_check_resp_err(escand_cli_info_t *context)
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
process_response(escand_cli_info_t *ctx, int rcount)
{
	ctx->cmd_buf[rcount] = '\0';		/* Nul terminate the response, just in case. */

	/* Special handling for non-ascii "dump" command results. */

	if (!strcmp(ctx->cmdarg[CMDARG_COMMAND], "dump")) {
		const dispatch_table_t *dispatch;

		dispatch = dispatch_lookup(dump_table, ctx->cmdarg[CMDARG_PARAMETER]);
		if (dispatch && dispatch->func) {
			if (escandc_check_resp_err(ctx)) {
				return BCME_OK;	/* escandc_check_resp_err() displays error msg */
			}
			return dispatch->func(ctx, rcount);
		}
	}

	/* Display the generic response text message. */
	if (ctx->cmd_buf[0]) printf("%s\n", ctx->cmd_buf);

	return BCME_OK;
}
/*
 * Print the usage of escand_cli utility
 */
static void
usage(const char *pname)
{
	printf("usage: %s [options] <command>|<variable>[ <value>]\n"
		"options are:\n"
		"   -i, --interface <ifname>  Apply command to specified wl interface on server\n"
		"   -C, --commands            Lists available commands and variables to get/set\n"
		"   -S, --settings            Retrieves and displays all known variables\n"
		"   -R, --report              Retrieves and displays all known information.\n"
		"   -h, --help                This help text.\n"
		"\n"
		"NOTE:- Start the escand on target to use this command\n", pname);
}
/*
 * parse_commandline() - Parse the user provided command line directly into the context fields.
 */
static int
parse_commandline(escand_cli_info_t *ctx, int argc, char *argv[])
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
 * connect_to_server() - Establish a TCP connection to the ESCAND server command port.
 *
 * On success, the context socket is updated and BCME_OK is returned.
 *
 */
static int
connect_to_server(escand_cli_info_t *ctx)
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
disconnect_from_server(escand_cli_info_t *ctx)
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
create_server_command(escand_cli_info_t *ctx)
{
	int len;

	if (escandc_table_lookup(escand_commands, ctx->cmdarg[CMDARG_COMMAND]) == BCME_NOTFOUND) {
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

	len = escand_snprintf(ctx->cmd_buf, ESCAND_BUFSIZE_4K, "%s&ifname=%s",
		ctx->cmdarg[CMDARG_COMMAND], ctx->wl_ifname);

	if (ctx->cmdarg[CMDARG_PARAMETER][0]) {
		len += escand_snprintf(ctx->cmd_buf+len, ESCAND_BUFSIZE_4K - len,
			"&param=%s", ctx->cmdarg[CMDARG_PARAMETER]);
	}

	if (ctx->cmdarg[CMDARG_VALUE][0]) {
		len += escand_snprintf(ctx->cmd_buf+len, ESCAND_BUFSIZE_4K - len,
			"&value=%d", (int)strtoul(ctx->cmdarg[CMDARG_VALUE], NULL, 0));
	}
	++len;	/* Add the trailing nul byte, not included in len so far */

	if (len >= ESCAND_BUFSIZE_4K) {
		fprintf(stderr, "Error setting up server command string: %s\n", strerror(errno));
		return -1;	/* Return negative length */
	}
	return len;
}

/*
 * do_command_response() - set up and send a command to the server, read and process the response.
 */
static int
do_command_response(escand_cli_info_t *ctx)
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

	len = sread(ctx->socket, ctx->cmd_buf, ESCAND_BUFSIZE_4K-1);
	if (len < 0) {
		fprintf(stderr, "Failed to read response from server: %s\n", strerror(errno));
		return BCME_ERROR;
	}

	/* Process the response */
	return process_response(ctx, len);
}

static int
report_all_variables(escand_cli_info_t *ctx)
{
	int rc = BCME_OK;
	int i;

	ctx->cmdarg[CMDARG_COMMAND] = "get";

	printf("[ %s: escand variables ]\n", ctx->wl_ifname);

	for (i = 0; (escand_variables[i] && (rc == BCME_OK)); ++i) {
		rc = connect_to_server(ctx);
		if (rc == BCME_OK) {
			printf("%25s = ", escand_variables[i]);
			fflush(stdout);
			ctx->cmdarg[CMDARG_PARAMETER] = escand_variables[i];
			rc = do_command_response(ctx);
			disconnect_from_server(ctx);
		}
	}
	return rc;
}

static int
report_all_dumps(escand_cli_info_t *ctx)
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
report_everything(escand_cli_info_t *ctx)
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
	static escand_cli_info_t ai;
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
