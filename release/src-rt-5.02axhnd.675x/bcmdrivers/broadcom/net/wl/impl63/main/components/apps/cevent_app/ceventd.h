/*
 * Cevent daemon header
 *
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
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
 * $Id: ceventd.h 777386 2019-07-31 06:15:54Z $
 */

#ifndef __CEVENTD_H__
#define __CEVENTD_H__

#include "cevent_app_common.h"

/* enable structure packing */
#include <packed_section_start.h>

/* default verbosity */
extern CA_VERBOSE ca_d_verbose;

#define CA_OUT_FLUSH_FACTOR	4	/* fflush for every these many log entries */
#define CA_OUT_HEAD_FACTOR	50	/* print key heading for every these many log entries */
#define CA_OUT_LIMIT_MIN_KB	1	/* min valid number for nvram ceventd_out_limit */
#define CA_OUT_LIMIT_MAX_KB	1024	/* max valid number for nvram ceventd_out_limit */
#define CA_OUT_LIMIT_DEFAULT_KB	64	/* default size of log file in kB */

#define CA_WKSP_FLAG_SHUTDOWN	(1<<1)
#define CA_WKSP_FLAG_USR1	(1<<2)
#define CA_WKSP_FLAG_PAUSE	(1<<3)

#define CA_PKT_LEN		4096 /* cevent eapd socket packet limit */
#define CA_CLI_REQ_LEN		1024 /* cevent cli socket request limit */

#if CA_CLI_REQ_LEN > CA_CLI_RSP_LEN
#error "CA_CLI_REQ_LEN must be less than or equal to CA_CLI_RSP_LEN"
#endif /* CA_CLI_REQ_LEN > CA_CLI_RSP_LEN */

#define CA_IS_EAPOL_OR_PREAUTH(et)							\
	((((et) == ETHER_TYPE_802_1X) || ((et) == ETHER_TYPE_802_1X_PREAUTH)) ?		\
	TRUE : FALSE)

#define CA_WATCHDOG_PERIOD_SEC	1	/* run watchdog every these many seconds */

/* Prints with CEVENTD__<verb> and function name banner
 * where <verb> could be any suffix but typically verbosity suffix like "ERR", "MSG", "DBG"
 */
#define CA_PRT(verb, fmt, arg...) \
	printf("CEVENTD_" verb "> %s(): "fmt, __FUNCTION__, ##arg)

#if defined(BCMDBG)
#define CA_ERR(fmt, arg...) CA_PRT("ERR", fmt, ##arg)
#else /* BCMINTERNAL || BCMDBG */
#define CA_ERR(fmt, arg...) if (ca_d_verbose >= CA_VERBOSE_ERR) CA_PRT("ERR", fmt, ##arg)
#endif // endif

#define CA_MSG(fmt, arg...) if (ca_d_verbose >= CA_VERBOSE_MSG) CA_PRT("MSG", fmt, ##arg)
#define CA_DBG(fmt, arg...) if (ca_d_verbose >= CA_VERBOSE_DBG) CA_PRT("DBG", fmt, ##arg)
/* debug log without banner */
#define CA_DBG_MIN(fmt, arg...) if (ca_d_verbose >= CA_VERBOSE_DBG) printf(fmt, ##arg)

#define CA_LOG_TYPE_SSV	0 /* space separated values with header per CA_OUT_HEAD_FACTOR */
#define CA_LOG_TYPE_CSV	1 /* comma separated values with header per CA_OUT_HEAD_FACTOR */

typedef struct {
	/* *** NOTE: Keep pkt_* elements at top to retain 4 byte boundary *** */
	uint8 pkt_eapd[CA_PKT_LEN];		/* buffer for eapd socket send/recv */
	uint8 pkt_cli_req[CA_CLI_REQ_LEN];	/* buffer for cli socket to receive request */
	uint8 pkt_cli_rsp[CA_CLI_RSP_LEN];	/* buffer for cli socket to send response */

	int pid;		/* current process id */
	int pid_fd;		/* FD to lock for this process ID */
	uint32 flags;		/* see CA_WKSP_FLAG_* defines for possible flag values */
	char interface[IFNAMSIZ+1];		/* LAN interface name */
	uint32	log_type;	/* see CA_LOG_TYPE_* */
	char out_path[CA_FILE_PATH_LEN];	/* output file path */
	char out_bak_path[CA_FILE_PATH_LEN];	/* output backup file path */
	FILE *out;		/* event log output stream; defaults to stdout */
	uint32 out_size;	/* estimated size of output file */
	uint32 out_limit;	/* size limit of output file in bytes */
	uint32 num_logs;	/* number of log entries written so far */
	uint32 log_console;	/* log output to console */
	uint32 log_syslogd;	/* send output to syslogd */
	uint32 tick;		/* tick seconds */
	uint64 recent_event_ts;	/* timestamp of recent most event */
	uint64 watchdog_ts;	/* timestamp when watchdog ran */
	int eapd_fd;		/* socket FD to receive/send bcmevent packets from/to eapd */
	int cli_fd;		/* server socket FD to listen from CLI */
} ca_wksp_t;

/* WPS Message types */
#define CA_WSC_START            0x01
#define CA_WSC_ACK              0x02
#define CA_WSC_NACK             0x03
#define CA_WSC_MSG              0x04
#define CA_WSC_DONE             0x05
#define CA_WSC_FRAG_ACK		0x06

#define CA_WPS_ID_MESSAGE_M1        0x04
#define CA_WPS_ID_MESSAGE_M2        0x05
#define CA_WPS_ID_MESSAGE_M2D       0x06
#define CA_WPS_ID_MESSAGE_M3        0x07
#define CA_WPS_ID_MESSAGE_M4        0x08
#define CA_WPS_ID_MESSAGE_M5        0x09
#define CA_WPS_ID_MESSAGE_M6        0x0A
#define CA_WPS_ID_MESSAGE_M7        0x0B
#define CA_WPS_ID_MESSAGE_M8        0x0C
#define CA_WPS_ID_MESSAGE_ACK       0x0D
#define CA_WPS_ID_MESSAGE_NACK      0x0E
#define CA_WPS_ID_MESSAGE_DONE      0x0F

#define CA_WPS_MSGTYPE_OFFSET		9

typedef BWL_PRE_PACKED_STRUCT struct ca_wps_eap_header_tag {
	uint8 code;
	uint8 id;
	uint16 length;
	uint8 type;
	uint8 vendorId[3];
	uint32 vendorType;
	uint8 opcode;
	uint8 flags;
} BWL_POST_PACKED_STRUCT ca_wps_eap_hdr;

extern char *ca_bcm_event_str[WLC_E_LAST + 1]; /* array is auto-generated by gen_be_str.sh */

extern int
ca_out_file_init(ca_wksp_t *cwksp);

extern int
ca_out_file_reinit(ca_wksp_t *cwksp);

extern int
ca_out_file_flush(ca_wksp_t *cwksp);

extern int
ca_cli_init(ca_wksp_t *cwksp);

extern int
ca_cli_deinit(ca_wksp_t *cwksp);

extern int
ca_open_eapd(ca_wksp_t *cwksp);

extern void
ca_close_eapd(ca_wksp_t *cwksp);

extern int
ca_eapd_send_pkt(ca_wksp_t *cwksp, struct iovec *frags, int nfrags);

extern int
ca_socket_process(ca_wksp_t *cwksp);

#endif /* __CEVENTD_H__ */
