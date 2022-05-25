/*
 * Command Line Interface for CEVENT
 *
 *
 * Copyright (C) 2022, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 *
 * $Id:$
 */

#include "ceventc.h"

int cli_verbose = 0;

#define CA_CMD_HELP	"help"
#define CA_CMD_DUMP	"dump"
#define CA_CMD_STATUS	"status"
#define CA_CMD_PAUSE	"pause"
#define CA_CMD_RESUME	"resume"
#define CA_CMD_END	"end"
#define CA_CMD_FLUSH	"flush"

static int ca_cli_connect(ca_cli_t *ctx);
static int ca_cli_disconnect(ca_cli_t *ctx);
static int ca_cli_usage(ca_cli_t *ctx);
static int ca_cli_getopt(ca_cli_t *ctx);
static int ca_cli_act(ca_cli_t *ctx);
static int ca_cli_parse_rsp(ca_cli_t *ctx, size_t sz);
static int ca_cli_parse_input(ca_cli_t *ctx);
static int ca_cli_remote(ca_cli_t *ctx, int len);
static int ca_cli_process(ca_cli_t *ctx);

static int
ca_cli_connect(ca_cli_t *ctx)
{
	char *shost = CA_CLI_SERVER_HOST_STR;
	char *sport = CA_CLI_SERVER_PORT_STR;
	struct addrinfo hints = {};
	struct addrinfo *results, *target;
	int rc, sock;

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	rc = getaddrinfo(shost, sport, &hints, &results);
	if (rc) {
		CC_ERR("%s port %s error %s\n", shost, sport, gai_strerror(rc));
		return BCME_BADADDR;
	}
	for (target = results; target; target = target->ai_next) {
		sock = socket(target->ai_family, target->ai_socktype, target->ai_protocol);
		if (sock == -1) {
			continue;
		}
		if (connect(sock, target->ai_addr, target->ai_addrlen) != -1) {
			break; /* success */
		}

		close(sock);
	}
	freeaddrinfo(results);

	if (!target) {
		CC_ERR("Could not connect to %s port %s\n", shost, sport);
		return BCME_BADADDR;
	}

	ctx->cli_fd = sock;
	return BCME_OK;
}

static int
ca_cli_disconnect(ca_cli_t *ctx)
{
	if (ctx->cli_fd < 0) {
		return BCME_ERROR;
	}
	close(ctx->cli_fd);
	ctx->cli_fd = CA_DEFAULT_FD;

	return BCME_OK;
}

static int
ca_cli_usage(ca_cli_t *ctx)
{
	printf("Version: %d\n", CA_CLI_VER);
	printf("Usage:\n");
	printf("\tdump\t\t - dump logs \n");
	printf("\tpause\t\t - pause logging on all interfaces or single by passing -i <iface>\n");
	printf("\tresume\t\t - resume logging on all interfaces or single by passing -i <iface>\n");
	printf("\tstatus\t\t - show daemon status\n");
	printf("\tflush\t\t - flushes log file(s)\n");
	printf("\thelp\t\t - show this help\n");
	printf("\t-h\t\t\t - show this help\n");
	printf("\t-v\t\t\t - show version\n");
	printf("\t-i <interface>\t\t - filter by interface\n");
	printf("\t-m <mac_addr>\t\t - filter by mac address\n");
	// printf("\t-p <server port>\t - server port\n"); // TODO: remote connection support
	// printf("\t-s <server address>\t - server address\n"); // TODO: remote connection support
#if defined(BCMDBG)
	printf("\t-V\t\t\t - use as first option to see verbose debug logs\n");
#endif

	return BCME_OK;
}

static int
ca_cli_getopt(ca_cli_t *ctx)
{
	int num_flags = 0, eflag = 0, hflag = 0, vflag = 0;
	char *iopt = NULL, *mopt = NULL, *popt = NULL, *sopt = NULL;
	int i, c;
	ca_act_t act;

	act = ctx->act = CA_ACT_DEFAULT;

	opterr = 0;

	ctx->iface[0] = '\0';
	ctx->mac[0] = '\0';
	ctx->argx = 0;

	while ((c = getopt(ctx->argc, ctx->argv, "ehi:m:p:s:vV")) != -1) {
		switch (c) {
			case 'e':
				eflag = 1;
				act = CA_ACT_DEFAULT; /* echo */
				num_flags ++;
				break;
			case 'h':
				hflag = 1;
				act = CA_ACT_HELP;
				num_flags ++;
				break;
			case 'i':
				iopt = optarg;
				break;
			case 'm':
				mopt = optarg;
				break;
			case 'p':
				popt = optarg;
				break;
			case 's':
				sopt = optarg;
				break;
			case 'v':
				vflag = 1;
				printf("Version: %d\n", CA_CLI_VER);
				break;
			case 'V':
				cli_verbose = 1;
				break;
			case '?':
				if (optopt == 'i') {
					CC_ERR("Option -%c requires an argument\n", optopt);
				} else if (isprint(optopt)) {
					CC_ERR("Unknown option `-%c'\n", optopt);
				} else {
					CC_ERR("Unknown option character `\\x%x'\n", optopt);
				}
				return BCME_ERROR;
			default:
				return BCME_ERROR;
		}
	}

	CC_DBG("eflag:%d, hflag:%d, iopt:%s, mopt:%s, popt:%s, sopt:%s, vflag:%d\n",
			eflag, hflag, iopt, mopt, popt, sopt, vflag);

	if (num_flags > 1) {
		CC_ERR("Incorrect usage. Can not use '-x, -h, -q, ..' simultaneously\n");
		return BCME_USAGE_ERROR;
	}

	if (iopt) {
		snprintf(ctx->iface, sizeof(ctx->iface), "%s", iopt);
	}

	if (mopt) {
		snprintf(ctx->mac, sizeof(ctx->mac), "%s", mopt);
	}

	for (i = optind; i < ctx->argc; i++) {
		if (!strcasecmp(ctx->argv[i], CA_CMD_HELP)) {
			act = CA_ACT_HELP;
		} else if (!strcasecmp(ctx->argv[i], CA_CMD_DUMP)) {
			act = CA_ACT_DUMP;
		} else if (!strcasecmp(ctx->argv[i], CA_CMD_STATUS)) {
			act = CA_ACT_STATUS;
		} else if (!strcasecmp(ctx->argv[i], CA_CMD_PAUSE)) {
			act = CA_ACT_PAUSE;
		} else if (!strcasecmp(ctx->argv[i], CA_CMD_RESUME)) {
			act = CA_ACT_RESUME;
		} else if (!strcasecmp(ctx->argv[i], CA_CMD_END)) {
			act = CA_ACT_END;
		} else if (!strcasecmp(ctx->argv[i], CA_CMD_FLUSH)) {
			act = CA_ACT_FLUSH;
		} else {
			if (act != CA_ACT_DEFAULT) {
				CC_ERR("Unhandled non-option argument '%s'\n", ctx->argv[i]);
				return BCME_USAGE_ERROR;
			}
			if (!ctx->argx) {
				ctx->argx = i; /* index of first unused argument to echo */
			}
		}
	}

	CC_DBG("act: %d\n", act);
	ctx->act = act;

	return BCME_OK;
}

static int
ca_cli_act(ca_cli_t *ctx)
{
	char *buf = (char *) ctx->pkt_cli;
	const uint16 buf_max = sizeof(ctx->pkt_cli);
	ca_cli_hdr_t *hdr = (ca_cli_hdr_t *) buf;
	int buf_left = buf_max;

	/* prepare request to be sent to the server */
	memset(hdr, 0, sizeof(*hdr));
	hdr->mag = CA_CLI_MAG;
	hdr->ver = CA_CLI_VER;
	hdr->len = sizeof(*hdr); /* till data[] is appended */
	hdr->fixed_len = sizeof(*hdr);
	snprintf(hdr->iface, sizeof(hdr->iface), "%s", ctx->iface);
	hdr->act = ctx->act;
	buf_left -= sizeof(*hdr);

	switch (ctx->act) {
		case CA_ACT_HELP:
			return ca_cli_usage(ctx);
		case CA_ACT_DUMP:
			return ca_cli_dump_local(ctx); /* TODO: send req to server when remote */
		case CA_ACT_DEFAULT:
			CC_DBG("appended '%s'\n", ctx->argv[ctx->argx]);
			hdr->len += CA_SNPRINTF_1(CA_CLI_DATA(hdr), buf_left, ctx->argv[ctx->argx]);
			return hdr->len;
		case CA_ACT_STATUS:
		case CA_ACT_PAUSE:
		case CA_ACT_RESUME:
		case CA_ACT_END:
		case CA_ACT_FLUSH:
			/* nothing to append; action is mentioned in the header */
			return hdr->len;
		default:
			return BCME_ERROR;
	}
}

static int
ca_cli_parse_rsp(ca_cli_t *ctx, size_t sz)
{
	char *buf = (char *) ctx->pkt_cli;
	const uint16 buf_max = sizeof(ctx->pkt_cli);
	ca_cli_hdr_t *hdr = (ca_cli_hdr_t *) buf;
	char *data;
	int data_len;

	/* sanity check response header */
	if (sz > buf_max || sz < sizeof(*hdr) || hdr->mag != CA_CLI_MAG ||
			hdr->ver != CA_CLI_VER || hdr->len < sizeof(*hdr) ||
			hdr->fixed_len < sizeof(*hdr) || sz < hdr->len ||
			hdr->act < 0 || hdr->act >= CA_ACT_LAST ||
			!(hdr->flags & CA_CLI_FLAG_RSP)) {
		CC_ERR("response failed sanity (%d bytes, checking hdr %d bytes)\n",
				sz, sizeof(*hdr));
		if (sz >= sizeof(*hdr)) {
			CC_DBG("sz: %d, sizeof(hdr): %d, mag: 0x%08X, ver: %d, hdr.len: %d, "
					"hdr.fixed_len: %d, hdr.act: %d, hdr.flags: 0x%08x\n",
					sz, sizeof(*hdr), hdr->mag, hdr->ver,
					hdr->len, hdr->fixed_len, hdr->act,
					hdr->flags);
		}
		return BCME_ERROR;
	}

	CC_DBG("cli (ver: %d, len: %d, flen: %d, act: %d, flags: %d, sub: %d\n",
			hdr->ver, hdr->len, hdr->fixed_len, hdr->act, hdr->flags, hdr->subtype);

	if (hdr->len <= hdr->fixed_len) {
		/* no variable length data */
		return BCME_OK;
	}

	data = buf + hdr->fixed_len;
	data_len = hdr->len - hdr->fixed_len;

	data[data_len - 1] = '\0';
	printf("%s\n", data);

	return BCME_OK;
}

/* parse input in argc, argv
 * Returns
 *   0/BCME_OK  when locally processed
 *   positive   with length of ctx->pkt_cli filled if communication with server is required
 *   negative   on error
 */
static int
ca_cli_parse_input(ca_cli_t *ctx)
{
	int ret = 0;

	if (ctx->argc < 2 || !ctx->argv[1] || !ctx->argv[1][0]) {
		CC_ERR("See usage with '%s -h'\n", ctx->argv[0]);
		return BCME_OK;
	}

	if ((ret = ca_cli_getopt(ctx)) != BCME_OK) {
		return ret;
	}

	return ca_cli_act(ctx);
}

static int
ca_cli_remote(ca_cli_t *ctx, int len)
{
	char *buf = (char *) ctx->pkt_cli;
	const uint16 buf_max = sizeof(ctx->pkt_cli);
	int ret;

	if (len > buf_max) {
		CC_ERR("Failed to send command to server: len:%d > buf_max:%d\n", len, buf_max);
		return BCME_BUFTOOSHORT;
	}

	if ((ret = ca_cli_connect(ctx)) != BCME_OK) {
		CC_ERR("Failed to open socket with server\n");
		return ret;
	}

	if (ctx->cli_fd < 0) {
		CC_ERR("Failed to send command to server: no cli_fd\n");
		return BCME_ERROR;
	}

	CC_DBG("sending %d bytes to server\n", len);
	if (ca_swrite(ctx->cli_fd, buf, len) < len) {
		CC_ERR("Failed to send command to server: %s\n", strerror(errno));
		return BCME_ERROR;
	}

	/* ensure EOF reaches server */
	shutdown(ctx->cli_fd, SHUT_WR);

	len = ca_sread(ctx->cli_fd, buf, buf_max);
	if (len < 0) {
		CC_ERR("Failed to read response from server: %s\n", strerror(errno));
		return BCME_ERROR;
	}

	CC_DBG("read %d bytes\n", len);
	if (len > buf_max) {
		return BCME_BUFTOOSHORT;
	}

	ret = ca_cli_parse_rsp(ctx, len);

	return ca_cli_disconnect(ctx);
}

/* generate output depending on parsed input by either
 *  - processing locally or
 *  - sending request to the server and reading the response
 */
static int
ca_cli_process(ca_cli_t *ctx)
{
	int ret;

	ret = ca_cli_parse_input(ctx);
	if (ret < 0) {
		/* error */
		CC_ERR("Done (error: %d)\n", ret);
		return ret;
	}
	if (ret == 0) {
		/* process locally */
		CC_DBG("Done (local)\n");
		return ret;
	}

	/* ret > 0, communicate with the server */
	return ca_cli_remote(ctx, ret);
}

int main(int argc, char *argv[])
{
	const char * nv_mode = nvram_safe_get("ceventc_mode");
	ca_cli_t ctx = { .mode = 0, .cli_fd = CA_DEFAULT_FD, .argc = argc, .argv = argv };
	int rc;

	if (argc > 1 && argv[1][0] == '-' && argv[1][1] == 'V' && !argv[1][2]) {
	       cli_verbose = 1;		// first option is -V
	}
	if (nv_mode && nv_mode[0]) {
		CC_ERR("nvram ceventc mode %s\n", nv_mode);
		sscanf(nv_mode, "%u", &ctx.mode);
	}
	CC_DBG("cli mode %u\n", ctx.mode);

	rc = ca_cli_process(&ctx);

	return rc;
}
