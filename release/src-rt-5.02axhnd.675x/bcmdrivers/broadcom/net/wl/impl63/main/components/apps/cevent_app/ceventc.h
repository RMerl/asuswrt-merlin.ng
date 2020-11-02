/*
 * Cevent client header
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
 * $Id$
 */

#ifndef __CEVENTC_H__
#define __CEVENTC_H__

#include "cevent_app_common.h"

extern int cli_verbose;

#if defined(BCMDBG)
#define CC_DBG(fmt, arg...) do { \
	if (cli_verbose) { fprintf(stderr, "CEVENT_CLI_DBG> %s: " fmt, __FUNCTION__, ##arg); } \
} while (0)
#else /* BCMINTERNAL || BCMDBG */
#define CC_DBG(fmt, arg...)
#endif // endif

#define CC_ERR(fmt, arg...) fprintf(stderr, "CEVENT_CLI_ERR> %s: " fmt, __FUNCTION__, ##arg)

typedef struct {
	/* *** NOTE: Keep pkt_* elements at top to retain 4 byte boundary *** */
	uint8 pkt_cli[CA_CLI_RSP_LEN];	/* buffer for cli socket to send request/receive response */

	uint32 mode;		/* cli mode */
	int argc;		/* Number of input args from shell */
	char **argv;		/* Input args from shell */
	int argx;		/* Index of fist unused arg */
	int cli_fd;		/* CLI client socket FD */
	ca_act_t act;		/* action to be performed */
	char iface[IFNAMSIZ+1];
	char mac[18];
	char shost[256];		/* server host name */
	char sport[8];			/* server port */
	char out_path[CA_FILE_PATH_LEN];	/* output file path */
	char out_bak_path[CA_FILE_PATH_LEN];	/* output backup file path */
} ca_cli_t;

extern int
ca_cli_dump_local(ca_cli_t *ctx);

#endif /* __CEVENTC_H__ */
