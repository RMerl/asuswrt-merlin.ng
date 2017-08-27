/*
 * Wireless Application Event Service
 * appevent utility (Linux)
 *
 * Copyright (C) 2017, Broadcom. All Rights Reserved.
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
 * $Id: $
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <typedefs.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <bcmtimer.h>
#include <bcmendian.h>
#include <shutils.h>
#include <security_ipc.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <appevent_hdr.h>

#ifdef BCMDBG
#define APPEVENT_DBG(fmt, arg...) printf("%s: "fmt, __FUNCTION__ , ## arg)
#else
#define APPEVENT_DBG(fmt, arg...)
#endif /* BCMDBG */
/* shared routine: send application event to appeventd daemon */
int
app_event_sendup(int event_id, int status, unsigned char *data, int data_len)
{
	int reuse = 1;
	struct sockaddr_in addr;
	int apps_sock;
	app_event_t *app_event;
	int len;
	char *pkt;

	/* open socket */
	apps_sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (apps_sock < 0) {
		APPEVENT_DBG("UDP Open failed.\n");
		return -1;
	}

	if (setsockopt(apps_sock, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse,
		sizeof(reuse)) < 0) {
		APPEVENT_DBG("UDP setsockopt failed.\n");
		close(apps_sock);
		return -1;
	}

	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(APPS_EVENT_UDP_PORT);
	if (bind(apps_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		APPEVENT_DBG("UDP Bind failed, close eventd appSocket %d\n", apps_sock);
		close(apps_sock);
		return -1;
	}

	if (apps_sock >= 0) {
		/* send to eventd */
		int sentBytes = 0;
		struct sockaddr_in to;

		to.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
		to.sin_family = AF_INET;
		to.sin_port = htons(APPS_EVENT_UDP_PORT);

		len = sizeof(app_event_h) + data_len;
		if ((pkt = (char *)malloc(len)) != NULL) {
			app_event = (app_event_t *)pkt;

			app_event->h.event_id = event_id;
			app_event->h.timestamp = time(NULL);
			app_event->h.status = status;

			memcpy(pkt + sizeof(app_event_h), data, data_len);

			sentBytes = sendto(apps_sock, (char*)app_event, len, 0,
				(struct sockaddr *)&to, sizeof(struct sockaddr_in));

			if (sentBytes != len) {
				APPEVENT_DBG("UDP send failed; sentBytes = %d\n", sentBytes);
			}

			free(pkt);
		}

		close(apps_sock);
	}

	return 0;
}
