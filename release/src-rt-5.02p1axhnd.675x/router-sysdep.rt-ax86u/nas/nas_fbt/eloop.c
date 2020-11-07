/*
 * Event loop
 * Copyright 2002-2003, Jouni Malinen <jkmaline@cc.hut.fi>
 * Copyright 2004, Instant802 Networks, Inc.
 * All Rights Reserved.
 *
 *
 * Copyright 2019 Broadcom
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
 *
 * $Id: eloop.c 701315 2017-05-24 13:08:15Z $
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#include "eloop.h"

struct eloop_sock {
	int sock;
	void *eloop_data;
	void *user_data;
	void (*handler)(int sock, void *eloop_ctx, void *sock_ctx);
};

struct eloop_timeout {
	clock_t time;
	void *eloop_data;
	void *user_data;
	void (*handler)(void *eloop_ctx, void *sock_ctx);
	struct eloop_timeout *next;
};

struct eloop_signal {
	int sig;
	void *user_data;
	void (*handler)(int sig, void *eloop_ctx, void *signal_ctx);
	int signaled;
};

struct eloop_data {
	void *user_data;

	int max_sock, reader_count;
	struct eloop_sock *readers;

	struct eloop_timeout *timeout;
	long ticks_per_second;

	int signal_count;
	struct eloop_signal *signals;
	int signaled;

	int terminate;
	int sock_unregistered;
};

static struct eloop_data eloop;

#define ELOOP_BEFORE(x, y)\
	((y) - (x) > 0)

void eloop_init(void *user_data)
{
	memset(&eloop, 0, sizeof(eloop));
	eloop.user_data = user_data;
	eloop.ticks_per_second = sysconf(_SC_CLK_TCK);
}

int eloop_register_read_sock(int sock,
		void (*handler)(int sock, void *eloop_ctx, void *sock_ctx),
		void *eloop_data, void *user_data)
{
	struct eloop_sock *tmp;

	tmp = (struct eloop_sock *)
		realloc(eloop.readers,
				(eloop.reader_count + 1) * sizeof(struct eloop_sock));
	if (tmp == NULL)
		return -1;

	tmp[eloop.reader_count].sock = sock;
	tmp[eloop.reader_count].eloop_data = eloop_data;
	tmp[eloop.reader_count].user_data = user_data;
	tmp[eloop.reader_count].handler = handler;
	eloop.reader_count++;
	eloop.readers = tmp;
	if (sock > eloop.max_sock)
		eloop.max_sock = sock;

	return 0;
}

int eloop_unregister_read_sock(int sock)
{
	int i;

	for (i = 0; i < eloop.reader_count; i++) {
		if (eloop.readers[i].sock == sock)
			break;
	}

	if (i >= eloop.reader_count)
		return -1;

	if (i + 1 < eloop.reader_count)
		memmove(&eloop.readers[i], &eloop.readers[i + 1],
				(eloop.reader_count - i - 1) *
				sizeof(struct eloop_sock));
	eloop.reader_count--;

	eloop.sock_unregistered = 1;

	/* max_sock for select need not be exact, so no need to update it */
	/* don't bother reallocating block, since this area is quite small and
	 * next registration will realloc anyway
	 */

	return 0;
}

int eloop_register_timeout(unsigned int secs, unsigned int usecs,
		void (*handler)(void *eloop_ctx, void *timeout_ctx),
		void *eloop_data, void *user_data)
{
	struct eloop_timeout *timeout, *tmp, *prev;
	struct tms unused;

	timeout = (struct eloop_timeout *) malloc(sizeof(*timeout));
	if (timeout == NULL)
		return -1;
	timeout->time = times(&unused);
	/* Need to make sure secs isn't more than half the clock tick space. */
	timeout->time += secs * eloop.ticks_per_second;
	timeout->time += usecs / (1000000 / eloop.ticks_per_second);
	timeout->eloop_data = eloop_data;
	timeout->user_data = user_data;
	timeout->handler = handler;
	timeout->next = NULL;

	if (eloop.timeout == NULL) {
		eloop.timeout = timeout;
		return 0;
	}

	prev = NULL;
	tmp = eloop.timeout;
	while (tmp != NULL) {
		if (ELOOP_BEFORE(timeout->time, tmp->time))
			break;
		prev = tmp;
		tmp = tmp->next;
	}

	if (prev == NULL) {
		timeout->next = eloop.timeout;
		eloop.timeout = timeout;
	} else {
		timeout->next = prev->next;
		prev->next = timeout;
	}

	return 0;
}

int eloop_cancel_timeout(void (*handler)(void *eloop_ctx, void *sock_ctx),
		void *eloop_data, void *user_data)
{
	struct eloop_timeout *timeout, *prev, *next;
	int removed = 0;

	prev = NULL;
	timeout = eloop.timeout;
	while (timeout != NULL) {
		next = timeout->next;

		if (timeout->handler == handler &&
			(timeout->eloop_data == eloop_data ||
			eloop_data == ELOOP_ALL_CTX) &&
			(timeout->user_data == user_data ||
			user_data == ELOOP_ALL_CTX)) {
			if (prev == NULL)
				eloop.timeout = next;
			else
				prev->next = next;
			free(timeout);
			removed++;
		} else
			prev = timeout;

		timeout = next;
	}

	return removed;
}

static void eloop_handle_signal(int sig)
{
	int i;

	eloop.signaled++;
	for (i = 0; i < eloop.signal_count; i++) {
		if (eloop.signals[i].sig == sig) {
			eloop.signals[i].signaled++;
			break;
		}
	}
}

static void eloop_process_pending_signals(void)
{
	int i;
	struct tms unused;
	clock_t now;
	static clock_t last_time = 0;
	int check_children = 0;

	if (last_time == 0)
	{
		last_time = times(&unused);
	}
	now = times(&unused);
	if ((now - last_time) >
			(60 * eloop.ticks_per_second))
	{
		check_children = 1;
		last_time = now;
	}

	if (!check_children && (eloop.signaled == 0))
		return;
	eloop.signaled = 0;

	for (i = 0; i < eloop.signal_count; i++) {
		if (eloop.signals[i].signaled ||
				(check_children && (eloop.signals[i].sig == SIGCHLD))) {
			eloop.signals[i].signaled = 0;
			eloop.signals[i].handler(eloop.signals[i].sig,
					eloop.user_data,
					eloop.signals[i].user_data);
		}
	}

}

int eloop_register_signal(int sig,
		void (*handler)(int sig, void *eloop_ctx, void *signal_ctx),
		void *user_data)
{
	struct eloop_signal *tmp;

	tmp = (struct eloop_signal *)
		realloc(eloop.signals,
				(eloop.signal_count + 1) *
				sizeof(struct eloop_signal));
	if (tmp == NULL)
		return -1;

	tmp[eloop.signal_count].sig = sig;
	tmp[eloop.signal_count].user_data = user_data;
	tmp[eloop.signal_count].handler = handler;
	tmp[eloop.signal_count].signaled = 0;
	eloop.signal_count++;
	eloop.signals = tmp;
	signal(sig, eloop_handle_signal);

	return 0;
}

/* Some packet from l2 call the handler */
void eloop_read(fd_set *fdset)
{
	int i;

	if (!eloop.terminate &&
			(eloop.timeout || eloop.reader_count > 0)) {

		eloop.sock_unregistered = 0;

		for (i = 0; i < eloop.reader_count; i++) {

			if (eloop.sock_unregistered)
				break;

			if (eloop.readers[i].sock >= 0 &&
				FD_ISSET(eloop.readers[i].sock, fdset)) {
				eloop.readers[i].handler(
					eloop.readers[i].sock,
					eloop.readers[i].eloop_data,
					eloop.readers[i].user_data);
			}
		}
	}
}

void eloop_run(void)
{
	fd_set rfds;
	int i, res;
	struct tms unused;
	clock_t now, next;
	struct timeval tv;

	while (!eloop.terminate &&
			(eloop.timeout || eloop.reader_count > 0)) {
		if (eloop.timeout) {
			now = times(&unused);
			if (ELOOP_BEFORE(now, eloop.timeout->time))
				next = eloop.timeout->time - now;
			else
				next = 0;

			tv.tv_sec = next / eloop.ticks_per_second;
			tv.tv_usec = (next % eloop.ticks_per_second)
				* (1000000 / eloop.ticks_per_second);
		}

		FD_ZERO(&rfds);
		for (i = 0; i < eloop.reader_count; i++)
			FD_SET(eloop.readers[i].sock, &rfds);
		res = select(eloop.max_sock + 1, &rfds, NULL, NULL,
				eloop.timeout ? &tv : NULL);
		if (res < 0 && errno != EINTR) {
			perror("select");
			return;
		}
		eloop_process_pending_signals();

		/* check if some registered timeouts have occurred */
		if (eloop.timeout) {
			struct eloop_timeout *tmp;

			now = times(&unused);
			if (! ELOOP_BEFORE(now, eloop.timeout->time)) {
				tmp = eloop.timeout;
				eloop.timeout = eloop.timeout->next;
				tmp->handler(tmp->eloop_data,
						tmp->user_data);
				free(tmp);
			}

		}

		if (res <= 0)
			continue;

		eloop.sock_unregistered = 0;

		for (i = 0; i < eloop.reader_count; i++) {

			if (eloop.sock_unregistered)
				break;

			if (FD_ISSET(eloop.readers[i].sock, &rfds)) {
				eloop.readers[i].handler(
						eloop.readers[i].sock,
						eloop.readers[i].eloop_data,
						eloop.readers[i].user_data);
			}
		}
	}
}

void eloop_terminate(void)
{
	eloop.terminate = 1;
}

void eloop_destroy(void)
{
	struct eloop_timeout *timeout, *prev;

	timeout = eloop.timeout;
	while (timeout != NULL) {
		prev = timeout;
		timeout = timeout->next;
		free(prev);
	}
	free(eloop.readers);
	free(eloop.signals);
}

int eloop_terminated(void)
{
	return eloop.terminate;
}

void * eloop_get_user_data(void)
{
	return eloop.user_data;
}
