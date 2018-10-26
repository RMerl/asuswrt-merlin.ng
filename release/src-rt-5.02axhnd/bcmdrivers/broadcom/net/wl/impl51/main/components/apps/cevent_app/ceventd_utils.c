/*
 * Cevent daemon utils
 *
 *
 * Copyright (C) 2018, Broadcom. All Rights Reserved.
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
 * $Id: ceventd_utils.c 762277 2018-05-11 13:55:04Z $
 */

#include "ceventd.h"
#include <errno.h>

#define BUFSZ	32

/* default verbosity */
CA_VERBOSE ca_d_verbose = CA_VERBOSE_ERR;

/* for argument list, see CA_LOG_EXT; DELIM is the delimiter string between columns */
#define CA_LOG_DELIM(DELIM, cwksp, ca_ts, ifname, sta, flags, status, reason, auth, \
		at, dir, hd, fmt, arg...) do { if (cwksp->out) { \
	if ((cwksp->num_logs % CA_OUT_HEAD_FACTOR) == 0) { \
		cwksp->out_size += fprintf(cwksp->out, \
			"CEVENTD[%d]: %-8s" DELIM "%-8s" DELIM "%-17s" DELIM "%5s" DELIM "%4s"\
			DELIM "%4s" DELIM "%4s"DELIM "%-8s" DELIM "%-3s" DELIM "%-24s" DELIM \
			"[CE: %2s" DELIM "%3s" DELIM "%-19s" DELIM "%2s" DELIM "%-19s" DELIM "%6s" \
			DELIM "%-8s]\n", (cwksp)->pid, \
			"TIME(ms)", "IfName", "Sta_MAC", "Flags", "Stts", \
			"Rsn", "Auth", "At", "Dir", "Header", \
			"VE", "Len", "Type", "DO", "SubType", "Flags", "CE_TS_ms"); \
	} \
	cwksp->out_size += fprintf(cwksp->out, \
		"CEVENTD[%d]: %08llu" DELIM "%-8s" DELIM "%-17s" DELIM "%05d" DELIM "%04d" \
		DELIM "%04d" DELIM "%04d" DELIM "%-8s" DELIM "%-3s" DELIM "%-24s" \
		DELIM "" fmt "\n", \
		(cwksp)->pid, ca_ts, ifname, sta, flags, status, \
		reason, auth, at, dir, hd, \
		##arg); \
} \
if (cwksp->log_syslogd == 1 ) { /* in syslog case, ident and pid are added through openlog */ \
	syslog(LOG_NOTICE, \
		"%08llu" DELIM "%-8s" DELIM "%-17s" DELIM "%5d" DELIM "%04d" \
		DELIM "%04d" DELIM "%04d" DELIM "%-8s" DELIM "%-3s" DELIM "%-24s" \
		DELIM "" fmt "\n", \
		ca_ts, ifname, sta, flags, status, reason, \
		auth, at, dir, hd, \
		##arg); \
} \
if (cwksp->log_console == 1) { \
	printf("CEVENTD[%d]: %08llu" DELIM "%-8s" DELIM "%-17s" DELIM "%5d" DELIM "%04d" \
		DELIM "%04d" DELIM "%04d" DELIM "%-8s" DELIM "%-3s" DELIM "%-24s" \
		DELIM "" fmt "\n", \
		(cwksp)->pid, ca_ts, ifname, sta, flags, status, \
		reason, auth, at, dir, hd, \
		##arg); \
} (cwksp)->recent_event_ts = ca_ts; } while (0)

/* Logs event details into cwksp->out file stream (which could be stdout).
 * Also adds number of characters printed to cwksp->out_size.
 * Params:
 *   ca_wksp_t * cwksp	: workspace pointer
 *   uint64 ca_ts	: cevent/application's event receive time stamp (in ms)
 *   char * ifname	: interface name (eg. eth0, wl0.1, wds0.1.3)
 *   sta		: STA MAC address
 *   int16 flags	: flags in event (if any)
 *   int32 status	: status code in event (if any)
 *   int32 reason	: reason code in event (if any)
 *   int32 auth		: auth type in event (if any)
 *   char * at		: event is from module (eg. driver, nas, eapd, ...)
 *   char * dir		: direction of frame in driver/app - Tx/Rx/None.
 *   char * hd		: heading string (eg. M1, G1, PTK-INS, ...)
 *   fmt		: format specifier for any extra arguments in 'arg...' (if present else "")
 *   arg...		: optional additional arguments to be printed as per 'fmt'
 */
#define CA_LOG_EXT(cwksp, ca_ts, ifname, sta, flags, status, reason, \
	       auth, at, dir, hd, fmt, arg...) do { \
	if (cwksp->log_type == CA_LOG_TYPE_SSV) { \
		CA_LOG_DELIM(" ", cwksp, ca_ts, ifname, sta, flags, status, reason, auth, \
				at, dir, hd, fmt, ##arg); \
	} else if (cwksp->log_type == CA_LOG_TYPE_CSV) { \
		CA_LOG_DELIM(", ", cwksp, ca_ts, ifname, sta, flags, status, reason, auth, \
				at, dir, hd, fmt, ##arg); \
	} \
	if ((cwksp->num_logs % CA_OUT_FLUSH_FACTOR) == 0) { \
		fflush(cwksp->out); \
	} \
	if (cwksp->out != stdout && cwksp->out_size > cwksp->out_limit) { \
		ca_out_file_reinit(cwksp); \
	} \
	cwksp->num_logs ++; \
} while (0)

/* for argument list, see CA_LOG_EXT */
#define CA_LOG(cwksp, ca_ts, ifname, sta, flags, status, reason, auth, at, dir, hd) \
	CA_LOG_EXT(cwksp, ca_ts, ifname, sta, flags, status, reason, auth, at, dir, hd, "")

/* for argument list, see CA_LOG_EXT , last additional argument here is wl_cevent_t pointer */
#define CA_LOG_CE(cwksp, ca_ts, ifname, sta, flags, status, reason, auth, at, dir, hd, ce) do { \
	char *yhd = ((NULL != (void *)(hd)) && (hd)[0]) ? (hd) : \
			(CEVENT_ST2STR((ce)->type, (ce)->subtype)); \
	char xhd[BUFSZ]; \
	snprintf(xhd, sizeof(xhd), "%s%s", yhd, \
			CEVENT_D2C_D2CFLSTR((ce)->type, (ce)->subtype, (ce)->flags)); \
	if (cwksp->log_type == CA_LOG_TYPE_SSV) { \
		CA_LOG_EXT(cwksp, (ce)->timestamp, ifname, sta, flags, status, \
			reason, auth, at, dir, xhd, \
			"[CE: %2u %3u %3u/0x%04X/%-8s %2u " \
			"%3u/0x%04X/%-8s 0x%04X %08llu]", \
			(ce)->version, (ce)->length, (ce)->type, (ce)->type, \
			(CEVENT_TYPE2STR((ce)->type)), \
			(ce)->data_offset, (ce)->subtype, (ce)->subtype, \
			(CEVENT_ST2STR((ce)->type, (ce)->subtype)), \
			(ce)->flags, (ce)->timestamp); \
	} else if (cwksp->log_type == CA_LOG_TYPE_CSV) { \
		CA_LOG_EXT(cwksp, (ce)->timestamp, ifname, sta, flags, status, \
			reason, auth, at, dir, xhd, \
			"[CE: %2u, %3u, %3u/0x%04X/%-8s, %2u, " \
			"%3u/0x%04X/%-8s, 0x%04X, %08llu]", \
			(ce)->version, (ce)->length, (ce)->type, (ce)->type, \
			(CEVENT_TYPE2STR((ce)->type)), \
			(ce)->data_offset, (ce)->subtype, (ce)->subtype, \
			(CEVENT_ST2STR((ce)->type, (ce)->subtype)), \
			(ce)->flags, (ce)->timestamp); \
	} \
} while (0)

static char *cevent_type_str[] = {
	"CE_CTRL",
	"CE_D2C",
	"CE_A2C",
	"CE_E2C",
	"CE_D2A",
	"CE_A2D",
};
#define CEVENT_TYPE2STR(T)	(((T) >= 0 && (T) < CEVENT_TYPE_LAST) ? \
				cevent_type_str[T] : "--")

static char *cevent_d2c_st_str[] = {
	"AUTH_TX",		/* 0 */
	"ASSOC_TX",		/* 1 */
	"EAP_TX",		/* 2 */
	"DISASSOC_TX",		/* 3 */
	"DEAUTH_TX",		/* 4 */
	"ST_TX_LAST",		/* 5 */

	"AUTH_RX",		/* 6 */
	"ASSOC_RX",		/* 7 */
	"EAP_RX",		/* 8 */
	"DISASSOC_RX",		/* 9 */
	"DEAUTH_RX",		/* 10 */
	"ST_RX_LAST",		/* 11 */

	"IF",			/* 12 */

	"ST_LAST"		/* 13 */
};

char * cevent_d2c_tx_flag_str[] = {"_QUEUED", "_SUCCESS", "_FAIL"};

#define CEVENT_D2C_TX_FLAG_STR(FL) ((FL) >= 0 && (FL) <= CEVENT_D2C_FLAG_FAIL ? \
		cevent_d2c_tx_flag_str[FL] : "")

#define CEVENT_D2C_D2CFLSTR(T, ST, FL)	(((T) != CEVENT_TYPE_D2C || (ST) >= CEVENT_D2C_ST_TX_LAST) \
		? "" : CEVENT_D2C_TX_FLAG_STR((FL) & 0xFFu))

static char *cevent_a2c_st_str[] = {
	"PTK_INSTALL",
	"GTK_INSTALL",
	"M1_RX",
	"M1_TX",
	"M2_RX",
	"M2_TX",
	"M3_RX",
	"M3_TX",
	"M4_RX",
	"M4_TX",
	"TIMEOUT",
	"LAST",
};

#define CEVENT_A2C_ST2STR(ST)	(((ST) >= 0 && (ST) < CEVENT_A2C_ST_LAST) ?	\
				cevent_a2c_st_str[ST] : "--")

#define CEVENT_D2C_ST2STR(ST)	(((ST) >= 0 && (ST) < CEVENT_D2C_ST_LAST) ? \
				cevent_d2c_st_str[ST] : "--")

#define CEVENT_ST2STR(T, ST) (((T) == CEVENT_TYPE_D2C) ? CEVENT_D2C_ST2STR(ST) : \
				(((T) == CEVENT_TYPE_A2C) ? CEVENT_A2C_ST2STR(ST) :	\
				("--")))

/* prune reason codes */
char *ca_prune_str[] = {
	"PRUNE_UNKNOWN",	/* DUMMY */
	"ERR_ENCR_MISMATCH",	/* encryption mismatch */
	"ERR_BCAST_BSSID",	/* AP uses a broadcast BSSID */
	"ERR_MAC_DENY",		/* STA's MAC addr is in AP's MAC deny list */
	"ERR_MAC_NA",		/* STA's MAC addr is not in AP's MAC allow list */
	"ERR_REG_PASSV",	/* AP not allowed due to regulatory restriction */
	"ERR_SPCT_MGMT",	/* AP does not support STA locale spectrum mgmt */
	"ERR_RADAR",		/* AP is on a radar channel of STA locale */
	"ERR_RSN_MISMATCH",	/* STA does not support AP's RSN */
	"ERR_NO_COMMON_RATES",	/* No rates in common with AP */
	"ERR_BASIC_RATES",	/* STA does not support all basic rates of BSS */
	"ERR_CCXFAST_PREVAP",	/* CCX FAST ROAM: prune previous AP */
	"ERR_CIPHER_NA",	/* BSS's cipher not supported */
	"ERR_KNOWN_STA",	/* AP is already known to us as a STA */
	"ERR_CCXFAST_DROAM",	/* CCX FAST ROAM: prune unqualified AP */
	"ERR_WDS_PEER",		/* AP is already known to us as a WDS peer */
	"ERR_QBSS_LOAD",	/* QBSS LOAD - AAC is too low */
	"ERR_HOME_AP",		/* prune home AP */
	"ERR_AP_BLOCKED",	/* prune blocked AP */
	"ERR_NO_DIAG_SUPPORT",	/* prune due to diagnostic mode not supported */
	"ERR_AUTH_RESP_MAC",	/* suppress auth resp by MAC filter */
};

static char *ca_eap_code_str[] = {
	"INDEX_ZERO",
	"EAP_REQ",
	"EAP_RESP",
	"EAP_SUCCESS",
	"EAP_FAILURE"
};

#define CA_EAP_CODE2STR(C)	(((C) > 0 && (C) <= (EAP_FAILURE))	? \
					ca_eap_code_str[C] : "--")

static int ca_identify_a2d_d2a(ca_wksp_t *cwksp, wl_cevent_t  *ce, char *pheader, size_t hsz,
		char *pat, size_t asz);
static int ca_identify_a2c(ca_wksp_t *cwksp, wl_cevent_t *ce, char *pheader, size_t hsz);
static int ca_identify_bcm_event(ca_wksp_t *cwksp, uint32 be_event_type, char *pheader, size_t hsz);
static int ca_identify_prune(ca_wksp_t *cwksp, uint32 reason, char *pheader, size_t hsz);
static int ca_process_eapd_pkt(ca_wksp_t *cwksp, int bytes, uint64 ca_ts);
static int ca_process_eapd_sock(ca_wksp_t *cwksp);
static int ca_process_cli_pkt(ca_wksp_t *cwksp, int32 sz_req, uint32 *sz_rsp);
static int ca_process_cli_sock(ca_wksp_t *cwksp);

static int ca_identify_eapol(ca_wksp_t *cwksp, wl_cevent_t *ce, eapol_header_t *eapol,
		char *pheader, size_t hsz);
static int ca_identify_eapol_keys(ca_wksp_t *cwksp, wl_cevent_t *ce, eapol_header_t *eapol,
		char *pheader, size_t hsz);
static int ca_identify_eap_code(ca_wksp_t *cwksp, wl_cevent_t *ce, eapol_header_t *ce_eapol,
		char *pheader, size_t hsz);
static void ca_identify_eap_type(ca_wksp_t *cwksp, wl_cevent_t *ce, eapol_header_t *eapol,
		char *pheader, size_t hsz);
static int ca_identify_wps_msg_type(uint8 wps_msg_type, char* pheader, size_t hsz);

int
ca_out_file_init(ca_wksp_t *cwksp)
{
	FILE * out = NULL;

	if (!cwksp) {
		CA_ERR("called with null cwksp\n");
		return BCME_ERROR;
	}

	out = fopen(cwksp->out_path, "a");

	if (out) {
		CA_DBG("opened file %s\n", cwksp->out_path);
		cwksp->out = out;
		fseek(out, 0, SEEK_END);
		cwksp->out_size = ftell(out);
		CA_DBG("file %s is of size %u bytes\n",
				cwksp->out_path, cwksp->out_size);
	} else {
		CA_ERR("error opening %s, using stdout\n", cwksp->out_path);
		fprintf(stderr, "error opening %s, using stdout\n", strerror(errno));
		cwksp->out = stdout;
	}

	return BCME_OK;
}

int
ca_out_file_reinit(ca_wksp_t *cwksp)
{
	if (!cwksp || !cwksp->out || cwksp->out == stdout) {
		CA_ERR("called with incorrect params\n");
		return BCME_ERROR;
	}

	fclose(cwksp->out);

	CA_DBG("size %d, limit %d\n", cwksp->out_size,
			cwksp->out_limit);
	if (cwksp->out_size > cwksp->out_limit) {
		CA_DBG("moving file '%s' to '%s'\n", cwksp->out_path,
			cwksp->out_bak_path);
		rename(cwksp->out_path, cwksp->out_bak_path); /* backup on the same mountpoint/fs */
	}

	return ca_out_file_init(cwksp);
}

int
ca_cli_init(ca_wksp_t *cwksp)
{
	unsigned int port = CA_CLI_SERVER_PORT;
	int reuse = 1;
	struct sockaddr_in saddr = {0};

	if ((cwksp->cli_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		CA_ERR("cli: unable to open server socket: %d/%s\n", errno, strerror(errno));
		goto c_exit0;
	}

	if (setsockopt(cwksp->cli_fd, SOL_SOCKET, SO_REUSEADDR,
		(char*)&reuse, sizeof(reuse)) < 0) {
		CA_ERR("cli: unable to setsockopt to server socket %d\n", cwksp->cli_fd);
		goto c_exit1;
	}

	/* bind loopback socket to communicate with CLI client */
	memset(&saddr, 0, sizeof(struct sockaddr_in));
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	saddr.sin_port = htons(port);
	if (bind(cwksp->cli_fd, (struct sockaddr *)&saddr, sizeof(saddr)) < 0) {
		CA_ERR("cli: unable to bind to socket %d err %d/%s\n", cwksp->cli_fd,
				errno, strerror(errno));
		goto c_exit1;
	}

	if (listen(cwksp->cli_fd, CA_CLI_SOCKET_BACKLOG) < 0) {
		CA_ERR("cli: socket listen failed: %d/%s\n", errno, strerror(errno));
		goto c_exit1;
	}

	CA_DBG("cli: opened loopback socket %d\n", cwksp->eapd_fd);
	return BCME_OK;

	/* error handling */
c_exit1:
	close(cwksp->cli_fd);

c_exit0:
	cwksp->cli_fd = CA_DEFAULT_FD;
	CA_ERR("cli: failed to open socket\n");
	return errno;
}

int
ca_cli_deinit(ca_wksp_t *cwksp)
{
	if (cwksp->cli_fd != CA_DEFAULT_FD) {
		CA_MSG("cli: close  socket %d\n", cwksp->cli_fd);
		close(cwksp->cli_fd);
		cwksp->cli_fd = CA_DEFAULT_FD;
	}

	return BCME_OK;
}

static int
ca_fill_status(ca_wksp_t *cwksp, char *str, size_t size)
{

	cwksp->out_path[sizeof(cwksp->out_path) - 1]  = '\0';
	cwksp->out_bak_path[sizeof(cwksp->out_bak_path) - 1]  = '\0';
	return CA_SNPRINTF_1(str, size, "Status\n"
			"\ttick: %ds\n"
			"\tpid: %d\n"
			"\tflags: 0x%04X\n"
			"\tlog type: %u\n"
			"\tnumber of logs done: %u\n"
			"\toutput file path: %s\n"
			"\toutput backup path: %s\n"
			"\toutput file limit: %d bytes\n"
			"\teapd socket fd: %d\n"
			"\tcli socket fd: %d\n"
			"\trecent event timestamp: %llums\n"
			"\twatchdog timestamp: %llums\n",
			cwksp->tick,
			cwksp->pid,
			cwksp->flags,
			cwksp->log_type,
			cwksp->num_logs,
			cwksp->out_path,
			cwksp->out_bak_path,
			cwksp->out_limit,
			cwksp->eapd_fd,
			cwksp->cli_fd,
			cwksp->recent_event_ts,
			cwksp->watchdog_ts);
}

/* parse command in pkt_cli_req buffer and reply with response in the pkt_cli_rsp buffer */
static int
ca_process_cli_pkt(ca_wksp_t *cwksp, int32 sz_req, uint32 *p_sz_rsp)
{
	char *req_buf = (char *) cwksp->pkt_cli_req;
	const uint16 req_buf_max = sizeof(cwksp->pkt_cli_req);
	char *rsp_buf = (char *) cwksp->pkt_cli_rsp;
	const uint16 rsp_buf_max = sizeof(cwksp->pkt_cli_rsp);
	ca_cli_hdr_t *req_hdr = (ca_cli_hdr_t *) req_buf;
	ca_cli_hdr_t *rsp_hdr = (ca_cli_hdr_t *) rsp_buf;
	int rem_len, data_len;

	*p_sz_rsp = 0;
	cwksp->pkt_cli_rsp[0] = '\0';

	if (!cwksp || sz_req < 0 || sz_req > req_buf_max || !p_sz_rsp) {
		return BCME_ERROR;
	}

	/* sanity check request header */
	if (sz_req < sizeof(*req_hdr) || req_hdr->mag != CA_CLI_MAG ||
			req_hdr->ver != CA_CLI_VER || req_hdr->len < sizeof(*req_hdr) ||
			req_hdr->fixed_len < sizeof(*req_hdr) || sz_req < req_hdr->len ||
			req_hdr->act < 0 || req_hdr->act >= CA_ACT_LAST ||
			(req_hdr->flags & CA_CLI_FLAG_RSP) != 0) {
		CA_ERR("request failed sanity (%d bytes, checking hdr %d bytes)\n",
				sz_req, sizeof(*req_hdr));
		if (sz_req >= sizeof(*req_hdr)) {
			CA_DBG("sz: %d, sizeof(hdr): %d, mag: 0x%08X, ver: %d, hdr.len: %d, "
					"hdr.fixed_len: %d, hdr.act: %d, hdr.flags: 0x%08x\n",
					sz_req, sizeof(*req_hdr), req_hdr->mag, req_hdr->ver,
					req_hdr->len, req_hdr->fixed_len, req_hdr->act,
					req_hdr->flags);
		}
		return BCME_ERROR;
	}

	CA_DBG("act: %d\n", req_hdr->act);
	*rsp_hdr = *req_hdr;			/* copy header */
	rsp_hdr->flags = CA_CLI_FLAG_RSP;	/* mark response flag */
	rsp_hdr->len = sizeof(*rsp_hdr);	/* reset len to header size till data is appended */
	rem_len = rsp_buf_max - rsp_hdr->len;
	switch (req_hdr->act) {
		case CA_ACT_DEFAULT:
			if (rsp_buf_max < req_hdr->len) {
				CA_ERR("required sz %d > buf sz %d\n", req_hdr->len, rsp_buf_max);
				break;
			}
			data_len = req_hdr->len - req_hdr->fixed_len;
			if (data_len > 0) { /* copy data if any */
				if (data_len > rem_len) {
					CA_ERR("cant copy %d, rem %d\n", data_len, rem_len);
					break;
				}
				memcpy(CA_CLI_DATA(rsp_hdr), CA_CLI_DATA(req_hdr), data_len);
				rsp_hdr->len += data_len;
			}
			*p_sz_rsp = rsp_hdr->len;
			break;
		case CA_ACT_DUMP:
			*p_sz_rsp = rsp_hdr->len; /* place holder; currently handled within cli */
			break;
		case CA_ACT_STATUS:
			rsp_hdr->len += ca_fill_status(cwksp, CA_CLI_DATA(rsp_hdr), rem_len);
			*p_sz_rsp = rsp_hdr->len;
			break;
		case CA_ACT_PAUSE:
			cwksp->flags |= CA_WKSP_FLAG_PAUSE;
			rsp_hdr->len += CA_SNPRINTF_1(CA_CLI_DATA(rsp_hdr), rem_len, "Paused");
			*p_sz_rsp = rsp_hdr->len;
			break;
		case CA_ACT_RESUME:
			cwksp->flags &= ~CA_WKSP_FLAG_PAUSE; /* un-pause */
			rsp_hdr->len += CA_SNPRINTF_1(CA_CLI_DATA(rsp_hdr), rem_len, "Resumed");
			*p_sz_rsp = rsp_hdr->len;
			break;
		case CA_ACT_END:
			cwksp->flags |= CA_WKSP_FLAG_SHUTDOWN;
			rsp_hdr->len += CA_SNPRINTF_1(CA_CLI_DATA(rsp_hdr), rem_len, "Shutting...");
			*p_sz_rsp = rsp_hdr->len;
			break;
		default:
			return BCME_ERROR;
	}

	return BCME_OK;
}

/* check for CLI client connecting,
 *	accept connection,
 *		receive command,
 *			process,
 *			respond and
 *		close connection
 */
static int
ca_process_cli_sock(ca_wksp_t *cwksp)
{
	uint32 sz_rsp = 0;
	int32 sz_req = 0;
	int fd = CA_DEFAULT_FD;
	struct sockaddr_in saddr;
	uint len = 0;		/* must be initialized to avoid EINVAL */
	char *req_buf = (char *) cwksp->pkt_cli_req;
	const uint16 req_buf_max = sizeof(cwksp->pkt_cli_req);
	char *rsp_buf = (char *) cwksp->pkt_cli_rsp;
	const uint16 rsp_buf_max = sizeof(cwksp->pkt_cli_rsp);

	if ((fd = accept(cwksp->cli_fd, (struct sockaddr*)&saddr, &len)) < 0) {
		if (errno == EINTR) {
			return BCME_OK;
		} else {
			CA_ERR("cli: socket accept failed: %d/%s\n", errno, strerror(errno));
			return BCME_ERROR;
		}
	}

	/* get command from CLI client */
	if ((sz_req = ca_sread(fd, req_buf, req_buf_max)) < 0) {
		CA_ERR("cli: failed reading from client: %d/%s\n", errno, strerror(errno));
		goto c_done;
	}
	// ca_hexdump_ascii(__FUNCTION__, req_buf, MIN(sz_req, CA_PKT_DUMP_LEN));

	CA_DBG("cli: received %d bytes\n", sz_req);
	if (sz_req >= req_buf_max) {
		CA_ERR("cli: command too large\n");
		goto c_done;
	}

	if (sz_req > 0) {
		ca_process_cli_pkt(cwksp, sz_req, &sz_rsp);
	}

	if (sz_rsp && sz_rsp < rsp_buf_max) {
		CA_DBG("sending cli response: %d bytes\n", sz_rsp);
		if (ca_swrite(fd, rsp_buf, sz_rsp) < 0) {
			CA_ERR("cli: failed to send response: %d/%s\n", errno, strerror(errno));
			goto c_done;
		}
	}

c_done:
	close(fd);
	return errno;
}

/* establish connection to EAPD's cevent sub-module to receive events
 */
int
ca_open_eapd(ca_wksp_t *cwksp)
{
	int reuse = 1;
	struct sockaddr_in saddr = {0};

	if ((cwksp->eapd_fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		CA_ERR("eapd: Unable to open server socket: %d/%s\n", errno, strerror(errno));
		goto e_exit0;
	}
	if (setsockopt(cwksp->eapd_fd, SOL_SOCKET, SO_REUSEADDR,
			(char*)&reuse, sizeof(reuse)) < 0) {
		CA_ERR("eapd: Unable to setsockopt to loopback socket %d.\n", cwksp->eapd_fd);
		goto e_exit1;
	}

	/* bind loopback socket to communicate with EAPD */
	memset(&saddr, 0, sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	saddr.sin_port = htons(EAPD_WKSP_CEVENT_UDP_SPORT);
	if (bind(cwksp->eapd_fd, (struct sockaddr *)&saddr, sizeof(saddr)) < 0) {
		CA_ERR("eapd: Unable to bind to socket %d err %d/%s\n", cwksp->eapd_fd,
				errno, strerror(errno));
		goto e_exit1;
	}
	CA_DBG("eapd: opened loopback socket %d\n", cwksp->eapd_fd);
	return BCME_OK;

	/* error handling */
e_exit1:
	close(cwksp->eapd_fd);

e_exit0:
	cwksp->eapd_fd = CA_DEFAULT_FD;
	CA_ERR("eapd: failed to open loopback socket\n");
	return errno;
}

void
ca_close_eapd(ca_wksp_t *cwksp)
{
	CA_DBG("eapd: closing loopback socket %d\n", cwksp->eapd_fd);

	/* clsoe eapd socket */
	if (cwksp->eapd_fd != CA_DEFAULT_FD) {
		close(cwksp->eapd_fd);
		cwksp->eapd_fd = CA_DEFAULT_FD;
		CA_DBG("eapd: closed loopback socket\n");
	}
	return;
}

/* transmit cevent control message through the socket */
int
ca_eapd_send_pkt(ca_wksp_t *cwksp, struct iovec *frags, int nfrags)
{
	struct msghdr mh;
	struct sockaddr_in to;
	struct iovec *iov;
	int i, rc = 0;

	if (!nfrags)
		return -1;
	assert(frags != NULL);

	if (frags->iov_len < sizeof(struct ether_header)) {
		return -1;
	}

	/* allocate iov buffer */
	iov = malloc(sizeof(struct iovec) * (nfrags + 1));
	if (iov == NULL) {
		return -1;
	}

	to.sin_addr.s_addr = inet_addr(EAPD_WKSP_UDP_ADDR);
	to.sin_family = AF_INET;
	to.sin_port = htons(EAPD_WKSP_CEVENT_UDP_RPORT);

	iov[0].iov_base = (void *)cwksp->interface;
	iov[0].iov_len = IFNAMSIZ;

	for (i = 1; i <= nfrags; i++) {
		iov[i].iov_base = frags[i-1].iov_base;
		iov[i].iov_len = frags[i-1].iov_len;
	}

	memset(&mh, 0, sizeof(mh));
	mh.msg_name = (void *)&to;
	mh.msg_namelen = sizeof(to);
	mh.msg_iov = iov;
	mh.msg_iovlen = nfrags + 1;

	if (sendmsg(cwksp->eapd_fd, &mh, 0) < 0) {
		rc = errno;
	}

	free(iov);
	return rc;
}

static int
ca_identify_wps_msg_type(uint8 wps_msg_type, char* pheader, size_t hsz)
{
	int len = 0;

	switch (wps_msg_type) {
		case CA_WPS_ID_MESSAGE_M1:
			CA_DBG("CA_WPS_ID_MESSAGE_M1");
			len = ca_snprintf(pheader, hsz, "_WPS_M1");
			break;
		case CA_WPS_ID_MESSAGE_M2:
			CA_DBG("CA_WPS_ID_MESSAGE_M2");
			len = ca_snprintf(pheader, hsz, "_WPS_M2");
			break;
		case CA_WPS_ID_MESSAGE_M2D:
			CA_DBG("CA_WPS_ID_MESSAGE_M2D");
			len = ca_snprintf(pheader, hsz, "_WPS_M2D");
			break;
		case CA_WPS_ID_MESSAGE_M3:
			CA_DBG("CA_WPS_ID_MESSAGE_M3");
			len = ca_snprintf(pheader, hsz, "_WPS_M3");
			break;
		case CA_WPS_ID_MESSAGE_M4:
			CA_DBG("CA_WPS_ID_MESSAGE_M4");
			len = ca_snprintf(pheader, hsz, "_WPS_M4");
			break;
		case CA_WPS_ID_MESSAGE_M5:
			CA_DBG("CA_WPS_ID_MESSAGE_M5");
			len = ca_snprintf(pheader, hsz, "_WPS_M5");
			break;
		case CA_WPS_ID_MESSAGE_M6:
			CA_DBG("CA_WPS_ID_MESSAGE_M6");
			len = ca_snprintf(pheader, hsz, "_WPS_M6");
			break;
		case CA_WPS_ID_MESSAGE_M7:
			CA_DBG("CA_WPS_ID_MESSAGE_M7");
			len = ca_snprintf(pheader, hsz, "_WPS_M7");
			break;
		case CA_WPS_ID_MESSAGE_M8:
			CA_DBG("CA_WPS_ID_MESSAGE_M8");
			len = ca_snprintf(pheader, hsz, "_WPS_M8");
			break;
		case CA_WPS_ID_MESSAGE_ACK:
			CA_DBG("CA_WPS_ID_MESSAGE_ACK");
			len = ca_snprintf(pheader, hsz, "_WPS_ACK");
			break;
		case CA_WPS_ID_MESSAGE_NACK:
			CA_DBG("CA_WPS_ID_MESSAGE_NACK");
			len = ca_snprintf(pheader, hsz, "_WPS_NACK");
			break;
		case CA_WPS_ID_MESSAGE_DONE:
			CA_DBG("CA_WPS_ID_MESSAGE_DONE");
			len = ca_snprintf(pheader, hsz, "_WSC_DONE");
			break;
		default:
			CA_DBG("Default: wps_msg_type %u\n", wps_msg_type);
			break;
	}

	return len;
}

static void
ca_identify_eap_type(ca_wksp_t *cwksp, wl_cevent_t *ce, eapol_header_t *eapol,
		char *pheader, size_t hsz)
{
	char *direction = (ce->type == CEVENT_TYPE_A2D ? "TX" : "RX");
	eap_header_t *eap = (eap_header_t *) eapol->body;
	int len = 0;

	switch (eap->type) {
		case EAP_IDENTITY:
			CA_DBG("EAP_IDENTITY\n");
			len += ca_snprintf(pheader + len, (hsz - len), "_ID_%s", direction);
			break;
		case EAP_NOTIFICATION:
			CA_DBG("EAP_NOTIFICATION\n");
			break;
		case EAP_NAK:
			CA_DBG("EAP_NAK\n");
			break;
		case EAP_MD5:
			CA_DBG("EAP_MD5\n");
			break;
		case EAP_OTP:
			CA_DBG("EAP_OTP\n");
			break;
		case EAP_GTC:
			CA_DBG("EAP_GTC\n");
			break;
		case EAP_TLS:
			CA_DBG("EAP_TLS\n");
			break;
		case EAP_EXPANDED:
			CA_DBG("EAP_EXPANDED\n");

			ca_wps_eap_hdr *ca_wps = (ca_wps_eap_hdr *)((char *)eapol->body);
			uint8 *wps_msg = NULL;
			uint8 wps_msg_type = 0;
			uint8 opcode = ca_wps->opcode;

			CA_DBG("opcode %u\n", opcode);

			if (opcode == CA_WSC_START) {
				snprintf(pheader, hsz, "_WSC_START_%s", direction);
			} else if (opcode == CA_WSC_ACK) {
				snprintf(pheader, hsz, "_WSC_ACK_%s", direction);
			} else if (opcode == CA_WSC_NACK) {
				snprintf(pheader, hsz, "_WSC_NACK_%s", direction);
			} else if (opcode == CA_WSC_MSG) {
				wps_msg =  (uint8 *)(ca_wps + 1);
				wps_msg_type = *(wps_msg + CA_WPS_MSGTYPE_OFFSET);
				CA_DBG("wps msg type %u\n", wps_msg_type);
				len = ca_identify_wps_msg_type(wps_msg_type, pheader, hsz);
				ca_snprintf(pheader + len, (hsz - len), "_%s", direction);
			} else if (opcode == CA_WSC_DONE) {
				snprintf(pheader, hsz, "_WSC_DONE_%s", direction);
			} else if (opcode == CA_WSC_FRAG_ACK) {
				snprintf(pheader, hsz, "_WSC_FRAG_ACK_%s", direction);
			} else {
				CA_DBG("Unhandled: opcode %u\n", opcode);
			}
			break;
		default: CA_DBG("Default: eap->type %u \n", eap->type);
			 break;
	} /* switch */

}

static int
ca_identify_eap_code(ca_wksp_t *cwksp, wl_cevent_t *ce,
		eapol_header_t *eapol, char *pheader, size_t hsz)
{
	eap_header_t *eap = (eap_header_t *) eapol->body;
	uint8 eap_code = eap->code;
	uint16 eapol_length = ntohs(eapol->length);
	char *direction = (ce->type == CEVENT_TYPE_A2D ? "TX" : "RX");
	int rc = BCME_OK;
	int len = 0;

	CA_DBG("EAP: code: %s/%u,  id %u, len %u\n", CA_EAP_CODE2STR(eap_code),
			eap_code, eap->id, ntohs(eap->length));

	if ((eap_code == EAP_REQUEST) || (eap_code == EAP_RESPONSE)) {
		len = ca_snprintf(pheader, hsz, CA_EAP_CODE2STR(eap_code));
		ca_identify_eap_type(cwksp, ce, eapol, (pheader + len), (hsz - len));
	} else if (eapol_length == EAP_HEADER_LEN &&
			(eap_code == EAP_SUCCESS || eap_code == EAP_FAILURE)) {
			snprintf(pheader, hsz, "%s_%s", CA_EAP_CODE2STR(eap_code), direction);
	} else {
		CA_DBG("Unknown eap_code %u\n", eap_code);
		rc = BCME_ERROR;
	}

	return rc;
}

static int
ca_identify_eapol_keys(ca_wksp_t *cwksp, wl_cevent_t *ce,
		eapol_header_t *eapol, char *pheader, size_t hsz)
{
	uint8 key_type;
	eapol_wpa_key_header_t *eapol_key = NULL;	/* WPA/802.11i/WPA2 EAPOL key */
	uint16 key_info, key_len, key_data_len;
	char *direction = (ce->type == CEVENT_TYPE_A2D ? "TX" : "RX");

	eapol_key = (eapol_wpa_key_header_t *)eapol->body; /* EAPOL-Key */

	key_type = (eapol_key->type);
	key_info = ntohs(eapol_key->key_info);
	key_len = ntohs(eapol_key->key_len);
	key_data_len = ntohs(eapol_key->data_len);

	CA_DBG("EAPOL-key: type %d info 0x%04x, len %d data_len: %d\n",
			key_type, key_info, key_len, key_data_len);

	if (key_info & WPA_KEY_REQ) {
		CA_DBG("Key Request\n");
		if (key_info & WPA_KEY_ERROR) {
			CA_DBG("Key Request Error\n");
			snprintf(pheader, hsz, "Key-Req-Err");
			return BCME_OK;
		}
	} else if (key_info & WPA_KEY_PAIRWISE) {
		uint16 value = key_info &
			(WPA_KEY_INSTALL | WPA_KEY_ACK | WPA_KEY_MIC);
		CA_DBG("Key Pairwise, install|ack|mic:%d\n", value);
		switch (value) {
			case WPA_KEY_ACK:
				CA_DBG("WPA Key Ack\n");
				snprintf(pheader, hsz, "M1_%s", direction);
				return BCME_OK;
			case WPA_KEY_MIC:
				if (key_data_len) {
					CA_DBG("WPA Key MIC and has key data - M2\n");
					snprintf(pheader, hsz, "M2_%s", direction);
				} else {
					CA_DBG("WPA Key MIC and no key data - M4\n");
					snprintf(pheader, hsz, "M4_%s", direction);
				}
				return BCME_OK;
			case (WPA_KEY_INSTALL | WPA_KEY_ACK | WPA_KEY_MIC):
				CA_DBG("WPA Key MIC - M3\n");
				snprintf(pheader, hsz, "M3_%s", direction);
				return BCME_OK;
			default:
				CA_MSG("Default case.\n");
				return BCME_UNSUPPORTED;
		}
	} else {
		if (key_info & WPA_KEY_ACK) {
			CA_DBG("Neither Req nor Pairwise. Has WPA Key Ack - G1\n");
			snprintf(pheader, hsz, "G1_%s", direction);
		} else {
			CA_DBG("Neither Req nor Pairwise. No WPA Key Ack - G2\n");
			snprintf(pheader, hsz, "G2_%s", direction);
		}
		return BCME_OK;
	}

	return BCME_UNSUPPORTED;

}

static int
ca_identify_eapol(ca_wksp_t *cwksp, wl_cevent_t *ce,
		eapol_header_t *eapol, char *pheader, size_t hsz)
{
	int rc = BCME_OK;
	char *direction = (ce->type == CEVENT_TYPE_A2D ? "TX" : "RX");

	CA_DBG("ce_ts %llums, eapol_ver %d, eapol_type %d, eapol_len %d\n",
			ce->timestamp, eapol->version, eapol->type, ntohs(eapol->length));

	switch (eapol->type) {
		case EAP_PACKET:
			CA_DBG("EAP_PACKET\n");
			rc = ca_identify_eap_code(cwksp, ce, eapol, pheader, hsz);
			break;
		case EAPOL_START:
			CA_DBG("EAPOL_START\n");
			snprintf(pheader, hsz, "EAPOL_START_%s", direction);
			break;
		case EAPOL_LOGOFF:
			CA_DBG("EAPOL_LOGOFF\n");
			break;
		case EAPOL_KEY:
			CA_DBG("EAPOL_KEY\n");
			rc = ca_identify_eapol_keys(cwksp, ce, eapol, pheader, hsz);
			break;
		case EAPOL_ASF:
			CA_DBG("EAPOL_ASF\n");
			break;
		default:
			CA_DBG("Default\n");
			break;
	}

	return rc;

}

/* Handle events from driver to application(s) or
 * frames(ex: EAPOL) from application to driver.
 */
static int
ca_identify_a2d_d2a(ca_wksp_t *cwksp, wl_cevent_t  *ce, char *pheader, size_t hsz,
		char *pat, size_t asz)
{
	struct ether_header *ce_eth = NULL;
	uint16 ce_eth_type;
	eapol_header_t *ce_eapol = NULL;

	CA_DBG("Checking for a2d d2a\n");
	snprintf(pat, asz, "EAPD");

	if (CE_HAS_ETH_HDR((ce->flags))) {
		CA_DBG("cevent has encapsulated frame header in data\n");
		/* Ethernet header present. Decapsulate. */
		ce_eth = (struct ether_header *)(ce->data);
		ce_eth_type = ntohs(ce_eth->ether_type);

		CA_DBG("ce ether type %d\n", ce_eth_type);

		if (CA_IS_EAPOL_OR_PREAUTH(ce_eth_type)) {
			ce_eapol = (eapol_header_t *)(ce_eth);
			CA_DBG("EAPOL or PREAUTH frame\n");
			return ca_identify_eapol(cwksp, ce, ce_eapol, pheader, hsz);
		} else {
			CA_MSG("NOT EAPOL or PREAUTH(unhandled)\n");
			if (ca_d_verbose == CA_VERBOSE_DBG && ce->length >= sizeof(*ce)) {
				ca_hexdump_ascii(__FUNCTION__, (uint8 *)ce,
						MIN(ce->length, CA_PKT_DUMP_LEN));
			}
		}
	} else {
		CA_MSG("No encapsulated frame header in cevent data\n");
		if (ca_d_verbose == CA_VERBOSE_DBG && ce->length >= sizeof(*ce)) {
				ca_hexdump_ascii(__FUNCTION__, (uint8 *)ce,
						MIN(ce->length, CA_PKT_DUMP_LEN));
		}
	}

	return BCME_UNSUPPORTED;
}

static int
ca_identify_a2c(ca_wksp_t *cwksp, wl_cevent_t *ce, char *pheader, size_t hsz)
{
	CA_DBG("Checking for a2c by ce subtype %u\n", ce->subtype);
	switch (ce->subtype)  {
		case CEVENT_A2C_ST_PTK_INSTALL:
			CA_MSG("PTK installation\n");
			snprintf(pheader, hsz, "PTK_INSTALL");
			return BCME_OK;
		case CEVENT_A2C_ST_GTK_INSTALL:
			CA_MSG("GTK installation\n");
			snprintf(pheader, hsz, "GTK_INSTALL");
			return BCME_OK;
		default:
			CA_MSG("Default. ce subtype: %u\n", ce->subtype);
			return BCME_UNSUPPORTED;
	}
}

static int
ca_identify_bcm_event(ca_wksp_t *cwksp, uint32 be_event_type, char *pheader, size_t hsz)
{
	if (be_event_type < ARRAY_SIZE(ca_bcm_event_str)) {
		CA_DBG("%s/%d\n", ca_bcm_event_str[be_event_type], be_event_type);
		snprintf(pheader, hsz, "%s/%d", ca_bcm_event_str[be_event_type], be_event_type);
		return BCME_OK;
	} else {
		CA_DBG("Unknown event %d\n", be_event_type);
		snprintf(pheader, hsz, "E-Unknown/%d", be_event_type);
		return BCME_UNSUPPORTED;
	}
}

static int
ca_identify_prune(ca_wksp_t *cwksp, uint32 reason, char *pheader, size_t hsz)
{
	if (reason && reason < ARRAY_SIZE(ca_prune_str)) {
		CA_DBG("%s\n", ca_prune_str[reason]);
		snprintf(pheader, hsz, "%s", ca_prune_str[reason]);
		return BCME_OK;
	} else {
		CA_DBG("Unknown prune reason %d\n", reason);
		return BCME_UNSUPPORTED;
	}
}

/* Process the dual encapsulated data received from cevnt sub-module of eapd layer.
 * Data received here is a brcmevent containing a cevent which futher contains the
 * actual data tapped at eapd by cevent sub-module. We need to handle the actual data
 * that is pointed to by cevent->data.
 */
static int
ca_process_eapd_pkt(ca_wksp_t *cwksp, int bytes, uint64 ca_ts)
{
	bcm_event_t *be = (bcm_event_t *)(cwksp->pkt_eapd);
	wl_cevent_t *ce =  (wl_cevent_t *)(be + 1);
	char header[BUFSZ] = "Generic";
	const size_t hsz = sizeof(header);
	char at[BUFSZ] = "Driver";
	char dir[BUFSZ] = "--";
	const size_t asz = sizeof(at);
	uint8 ifname[IFNAMSIZ+1] = "";
	char sta_ea[ETHER_ADDR_STR_LEN] = {0}; /* station address from event */
	struct ether_header *be_eth = NULL;
	wl_event_msg_t * be_event;
	uint16 be_eth_type;
	uint32 be_event_type;
	uint16 flags;
	uint32 status = ntohl(be->event.status);
	uint32 reason = ntohl(be->event.reason);
	uint32 auth   = htonl(be->event.auth_type);

	if (bytes < sizeof(*be)) {
		CA_DBG("received bytes %d too short to contain a BRCM event\n", bytes);
		return BCME_BUFTOOSHORT;
	}

	be_eth = (struct ether_header *) (&(be->eth));
	be_eth_type = ntohs(be_eth->ether_type);

	if (be_eth_type != ETHER_TYPE_BRCM) {
		CA_ERR("ether_type: %d, Not a BRCM event\n", be_eth_type);
		return BCME_UNSUPPORTED;
	}

	be_event =  (wl_event_msg_t *) (&(be->event));
	be_event_type = ntohl(be_event->event_type);
	flags = ntohs(be_event->flags);
	status = ntohl(be_event->status);
	reason = ntohl(be_event->reason);
	auth = ntohl(be_event->auth_type);

	memcpy(ifname, be_event->ifname, IFNAMSIZ);
	ifname[IFNAMSIZ] = '\0';

	ether_etoa((uchar *) &be_event->addr, sta_ea);

	CA_DBG("event_type %u\n", be_event_type);

	if  (be_event_type != WLC_E_CEVENT) {
		ca_identify_bcm_event(cwksp, be_event_type, header, hsz);
		if (be_event_type == WLC_E_PRUNE) {
			CA_DBG("prune reason:%d/0x%08x\n", reason, reason);
			ca_identify_prune(cwksp, reason, header, hsz);
		}
		CA_LOG(cwksp, ca_ts, ifname, sta_ea, flags, status, reason, auth, at, dir, header);

		if (ca_d_verbose ==  CA_VERBOSE_DBG) {
			ca_hexdump_ascii(__FUNCTION__, (uint8 *)be, MIN(bytes, CA_PKT_DUMP_LEN));
		}

		return BCME_OK;
	}

	if (bytes < (sizeof(*be) + sizeof(*ce))) {
		CA_DBG("received bytes %d too short to contain a Cevent\n", bytes);
		return BCME_BUFTOOSHORT;
	}

	if (bytes < (sizeof(*be) + ce->length)) {
		CA_DBG("receivd bytes %d too short to include Cevent len %u\n", bytes, ce->length);
		return BCME_BUFTOOSHORT;
	}

	if ((ce->flags) & CEVENT_FRAME_DIR_RX) {
		snprintf(dir, sizeof(dir), "RX");
	} else if ((ce->flags) & CEVENT_FRAME_DIR_TX) {
		snprintf(dir, sizeof(dir), "TX");
	} else {
		snprintf(dir, sizeof(dir), "--");
	}

	switch (ce->type) {
		case CEVENT_TYPE_A2D:
			snprintf(at, asz, "App");
			ca_identify_a2d_d2a(cwksp, ce, header, hsz, at, asz);
			break;
		case CEVENT_TYPE_D2A:
			snprintf(at, asz, "Driver");
			ca_identify_a2d_d2a(cwksp, ce, header, hsz, at, asz);
			break;
		case CEVENT_TYPE_A2C:
			snprintf(at, asz, "NAS"); /* TODO: use new field app_bmp & free subtype */
			header[0] = '\0';
			ca_identify_a2c(cwksp, ce, header, hsz);
			break;
		case CEVENT_TYPE_D2C:
			snprintf(at, asz, "Driver");
			header[0] = '\0';
			break;
		default:
			snprintf(at, asz, "Eapd");
			snprintf(header, hsz, "Generic-Cevent");
			if (ca_d_verbose ==  CA_VERBOSE_DBG) {
				ca_hexdump_ascii(__FUNCTION__, (uint8 *) be,
						MIN(bytes, CA_PKT_DUMP_LEN));
			}
			break;
	}

	CA_DBG("event identified as %s", header);
	CA_LOG_CE(cwksp, ca_ts, ifname, sta_ea, flags, status, reason, auth, at, dir, header, ce);

	return BCME_OK;
}

static int
ca_process_eapd_sock(ca_wksp_t *cwksp)
{
	int32 bytes;
	uint64 ca_ts = 0;	/* timestamp when received at cevent application */
	uint8 *buf = cwksp->pkt_eapd;
	const uint16 buf_max = sizeof(cwksp->pkt_eapd);

	if ((bytes = recv(cwksp->eapd_fd, buf, buf_max, 0)) < 0) {
		CA_DBG("recv bytes: %d\n", bytes);
		return BCME_ERROR;
	}
	CA_DBG("eapd socket received bytes: %d\n", bytes);
	if ((cwksp->flags & CA_WKSP_FLAG_PAUSE) != 0) {
		CA_DBG("paused; ignoring bytes: %d\n", bytes);
		return BCME_OK;
	}
	if (ca_d_verbose ==  CA_VERBOSE_DBG) {
		ca_hexdump_ascii(__FUNCTION__, buf, MIN(bytes, CA_PKT_DUMP_LEN));
	}
	ca_ts = ca_get_curr_time();
	CA_DBG("ca_ts %llums\n", ca_ts);

	return ca_process_eapd_pkt(cwksp, bytes, ca_ts);
}

/* process/recieve through the socket(s) */
int
ca_socket_process(ca_wksp_t *cwksp)
{
	fd_set fdset;		/* socket FD set for select */
	int32 fdmax;		/* sofket FD max */
	int32 width, status = 0;
	struct timeval tv = { .tv_sec = 0L, .tv_usec = 500000L }; /* total of 0.5s */

	if (!cwksp) {
		CA_DBG("cwksp NULL\n");
		return BCME_ERROR;
	}

	FD_ZERO(&fdset);
	fdmax = -1;

	if (cwksp->eapd_fd != CA_DEFAULT_FD) {
		FD_SET(cwksp->eapd_fd, &fdset);
		fdmax = cwksp->eapd_fd;
	}

	if (cwksp->cli_fd != CA_DEFAULT_FD) {
		FD_SET(cwksp->cli_fd, &fdset);
		fdmax = MAX(fdmax, cwksp->cli_fd);
	}

	width = fdmax + 1;

	/* check for data on enabled sockets */
	status = select(width, &fdset, NULL, NULL, &tv);

	if ((status == -1 && errno == EINTR) || (status == 0)) {
		return BCME_OK;
	}

	if (status <= 0) {
		CA_ERR("error from select: %d %s\n", errno, strerror(errno));
		return BCME_ERROR;
	}

	if (cwksp->eapd_fd !=  CA_DEFAULT_FD && FD_ISSET(cwksp->eapd_fd, &fdset)) {
		CA_DBG("socket receiving from EAPD\n");
		ca_process_eapd_sock(cwksp);
	}

	if (cwksp->cli_fd != CA_DEFAULT_FD && FD_ISSET(cwksp->cli_fd, &fdset)) {
		CA_DBG("socket receiving from CLI\n");
		ca_process_cli_sock(cwksp);
	}

	return BCME_OK;
}
