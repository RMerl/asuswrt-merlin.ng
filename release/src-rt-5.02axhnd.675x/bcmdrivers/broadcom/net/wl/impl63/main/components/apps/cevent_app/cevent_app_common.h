/*
 * Cevent app common header
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
 * $Id: cevent_app.h 739183 2018-01-05 11:13:50Z $
 */

#ifndef __CEVENT_APP_COMMON_H__
#define __CEVENT_APP_COMMON_H__

#include <typedefs.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <netinet/in.h>
#include <ethernet.h>
#include <arpa/inet.h>
#include <security_ipc.h>
#include <sys/socket.h>
#include <assert.h>
#include <eapol.h>
#include <eap.h>
#include <bcmevent.h>
#include <bcmutils.h>
#include <bcmnvram.h>
#include <sys/time.h>
#include <time.h>
#include <ctype.h>
#include <shutils.h>

#define CA_FILE_PATH_LEN	256	/* path name length */

#define CA_CLI_RSP_LEN		4096	/* cevent cli socket response limit */
#define CA_CLI_SERVER_HOST_STR	"127.0.0.1"
#define CA_CLI_SERVER_PORT	CEVENT_CLI_SERVER_UDP_PORT /* reserved in shared/security_ipc.h */
#define CA_CLI_SERVER_PORT_STR	xstr(CA_CLI_SERVER_PORT)
#define CA_CLI_SOCKET_BACKLOG	6	/* max queue depth of pending connections */

typedef enum {
	CA_VERBOSE_PRT = 0,
	CA_VERBOSE_ERR = 1,
	CA_VERBOSE_MSG = 2,
	CA_VERBOSE_DBG = 3
} CA_VERBOSE;

#ifndef IFNAMSIZ
#define IFNAMSIZ 16
#endif /* IFNAMSIZ */

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))
#endif /* ARRAY_SIZE */

#ifndef MIN
#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#endif /* MIN */

#ifndef MAX
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))
#endif /* MAX */

#ifndef xstr
#define xstr(s)			str(s)
#endif /* xstr */
#ifndef str
#define str(s)			#s
#endif /* str */

#define CA_DEFAULT_FD		-1	/* socket FD initial value */

#define CA_CLI_MAG	0xCECAC710u	/* fixed magic bytes */
#define CA_CLI_VER_1	1		/* version 1 */
#define CA_CLI_VER	CA_CLI_VER_1	/* current version */

typedef enum {
	CA_ACT_DEFAULT		= 0,
	CA_ACT_HELP		= 1,
	CA_ACT_DUMP		= 2,
	CA_ACT_STATUS		= 3,
	CA_ACT_PAUSE		= 4,
	CA_ACT_RESUME		= 5,
	CA_ACT_END		= 6,
	CA_ACT_FLUSH		= 7,
	CA_ACT_LAST
} ca_act_t;

#define CA_CLI_FLAG_RSP		1

/* access ca_cli_hdr_t's data using fixed_len offset */
#define CA_CLI_DATA(phdr) (((char*)(phdr)) + ((phdr)->fixed_len > 0 ? (phdr)->fixed_len : \
		sizeof(*(phdr))))

/* structure of content passed around in CLI request to server or response to client */
typedef struct {
	uint32 mag;		/* fixed magic bytes; see CA_CLI_MAG */
	uint16 ver;		/* version; see CA_CLI_VER */
	uint16 len;		/* length of structure including data if any */
	uint16 fixed_len;	/* length till before data */
	char iface[IFNAMSIZ+1];
	ca_act_t act;		/* see actions in ca_act_t */
	uint32 flags;		/* flags about cmd/data */
	uint32 subtype;		/* subtype to qualify cmd */
	/* optional elements into same version can be added here; access data using CA_CLI_DATA() */
	char data[];		/* variable length content depending on cmd */
} ca_cli_hdr_t;

extern void
ca_hexdump_ascii(const char *title, const unsigned char *buf, unsigned int len);

extern uint64
ca_get_curr_time();

/* use this macro to get return value including trailing NULL byte '\0' */
#define CA_SNPRINTF_1(st, sz, fmt, arg...) (ca_snprintf(st, sz, fmt, ##arg) + 1)

extern int
ca_snprintf(char *str, size_t size, const char *format, ...);

extern int
ca_swrite(int fd, char *buf, unsigned int size);

extern int
ca_sread(int fd, char *buf, unsigned int size);

#endif /* __CEVENT_APP_COMMON_H__ */
