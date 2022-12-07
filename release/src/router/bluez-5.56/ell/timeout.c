/*
 *
 *  Embedded Linux library
 *
 *  Copyright (C) 2011-2014  Intel Corporation. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <time.h>
#include <limits.h>

#include "util.h"
#include "timeout.h"
#include "private.h"

/**
 * SECTION:timeout
 * @short_description: Timeout support
 *
 * Timeout support
 */

/**
 * l_timeout:
 *
 * Opague object representing the timeout.
 */
struct l_timeout {
	int fd;
	l_timeout_notify_cb_t callback;
	l_timeout_destroy_cb_t destroy;
	void *user_data;
};

static void timeout_destroy(void *user_data)
{
	struct l_timeout *timeout = user_data;

	close(timeout->fd);
	timeout->fd = -1;

	if (timeout->destroy)
		timeout->destroy(timeout->user_data);
}

static void timeout_callback(int fd, uint32_t events, void *user_data)
{
	struct l_timeout *timeout = user_data;
	uint64_t expired;
	ssize_t result;

	result = read(timeout->fd, &expired, sizeof(expired));
	if (result != sizeof(expired))
		return;

	if (timeout->callback)
		timeout->callback(timeout, timeout->user_data);
}

static inline int timeout_set(int fd, unsigned int seconds, long nanoseconds)
{
	struct itimerspec itimer;

	memset(&itimer, 0, sizeof(itimer));
	itimer.it_interval.tv_sec = 0;
	itimer.it_interval.tv_nsec = 0;
	itimer.it_value.tv_sec = seconds;
	itimer.it_value.tv_nsec = nanoseconds;

	return timerfd_settime(fd, 0, &itimer, NULL);
}

static bool convert_ms(unsigned long milliseconds, unsigned int *seconds,
			long *nanoseconds)
{
	unsigned long big_seconds = milliseconds / 1000;

	if (big_seconds > UINT_MAX)
		return false;

	*seconds = big_seconds;
	*nanoseconds = (milliseconds % 1000) * 1000000L;

	return true;
}

/**
 * timeout_create_with_nanoseconds:
 * @seconds: number of seconds
 * @nanoseconds: number of nanoseconds
 * @callback: timeout callback function
 * @user_data: user data provided to timeout callback function
 * @destroy: destroy function for user data
 *
 * Create new timeout callback handling.
 *
 * The timeout will only fire once. The timeout handling needs to be rearmed
 * with one of the l_timeout_modify functions to trigger again.
 *
 * Returns: a newly allocated #l_timeout object. On failure, the function
 * returns NULL.
 **/
static struct l_timeout *timeout_create_with_nanoseconds(unsigned int seconds,
			long nanoseconds, l_timeout_notify_cb_t callback,
			void *user_data, l_timeout_destroy_cb_t destroy)
{
	struct l_timeout *timeout;
	int err;

	if (unlikely(!callback))
		return NULL;

	timeout = l_new(struct l_timeout, 1);

	timeout->callback = callback;
	timeout->destroy = destroy;
	timeout->user_data = user_data;

	timeout->fd = timerfd_create(CLOCK_MONOTONIC,
					TFD_NONBLOCK | TFD_CLOEXEC);
	if (timeout->fd < 0) {
		l_free(timeout);
		return NULL;
	}

	if (seconds > 0 || nanoseconds > 0) {
		if (timeout_set(timeout->fd, seconds, nanoseconds) < 0) {
			close(timeout->fd);
			l_free(timeout);
			return NULL;
		}
	}

	err = watch_add(timeout->fd, EPOLLIN | EPOLLONESHOT, timeout_callback,
			timeout, timeout_destroy);

	if (err < 0) {
		l_free(timeout);
		return NULL;
	}

	return timeout;
}

/**
 * l_timeout_create:
 * @seconds: timeout in seconds
 * @callback: timeout callback function
 * @user_data: user data provided to timeout callback function
 * @destroy: destroy function for user data
 *
 * Create new timeout callback handling.
 *
 * The timeout will only fire once. The timeout handling needs to be rearmed
 * with one of the l_timeout_modify functions to trigger again.
 *
 * Returns: a newly allocated #l_timeout object. On failure, the function
 * returns NULL.
 **/
LIB_EXPORT struct l_timeout *l_timeout_create(unsigned int seconds,
			l_timeout_notify_cb_t callback,
			void *user_data, l_timeout_destroy_cb_t destroy)
{
	return timeout_create_with_nanoseconds(seconds, 0, callback,
							user_data, destroy);
}

/**
 * l_timeout_create_ms:
 * @milliseconds: timeout in milliseconds
 * @callback: timeout callback function
 * @user_data: user data provided to timeout callback function
 * @destroy: destroy function for user data
 *
 * Create new timeout callback handling.
 *
 * The timeout will only fire once. The timeout handling needs to be rearmed
 * with one of the l_timeout_modify functions to trigger again.
 *
 * Returns: a newly allocated #l_timeout object. On failure, the function
 * returns NULL.
 **/
LIB_EXPORT struct l_timeout *l_timeout_create_ms(unsigned long milliseconds,
			l_timeout_notify_cb_t callback,
			void *user_data, l_timeout_destroy_cb_t destroy)
{
	unsigned int seconds;
	long nanoseconds;

	if (!convert_ms(milliseconds, &seconds, &nanoseconds))
		return NULL;

	return timeout_create_with_nanoseconds(seconds, nanoseconds, callback,
						user_data, destroy);
}

/**
 * l_timeout_modify:
 * @timeout: timeout object
 * @seconds: timeout in seconds
 *
 * Modify an existing @timeout and rearm it.
 **/
LIB_EXPORT void l_timeout_modify(struct l_timeout *timeout,
					unsigned int seconds)
{
	if (unlikely(!timeout))
		return;

	if (unlikely(timeout->fd < 0))
		return;

	if (seconds > 0) {
		if (timeout_set(timeout->fd, seconds, 0) < 0)
			return;
	}

	watch_modify(timeout->fd, EPOLLIN | EPOLLONESHOT, true);
}

/**
 * l_timeout_modify_ms:
 * @timeout: timeout object
 * @milliseconds: number of milliseconds
 *
 * Modify an existing @timeout and rearm it.
 **/
LIB_EXPORT void l_timeout_modify_ms(struct l_timeout *timeout,
					unsigned long milliseconds)
{
	if (unlikely(!timeout))
		return;

	if (unlikely(timeout->fd < 0))
		return;

	if (milliseconds > 0) {
		unsigned int sec;
		long nanosec;

		if (!convert_ms(milliseconds, &sec, &nanosec) ||
			timeout_set(timeout->fd, sec, nanosec) < 0)
			return;
	}

	watch_modify(timeout->fd, EPOLLIN | EPOLLONESHOT, true);
}

/**
 * l_timeout_remove:
 * @timeout: timeout object
 *
 * Remove timeout handling.
 **/
LIB_EXPORT void l_timeout_remove(struct l_timeout *timeout)
{
	if (unlikely(!timeout))
		return;

	watch_remove(timeout->fd, false);

	l_free(timeout);
}

/**
 * l_timeout_set_callback:
 * @timeout: timeout object
 * @callback: The new callback
 * @user_data: The new user_data
 * @destroy: The new destroy function
 *
 * Sets the new notify callback for @timeout.  If the old user_data object had
 * a destroy function set, then that function will be called.
 */
LIB_EXPORT void l_timeout_set_callback(struct l_timeout *timeout,
					l_timeout_notify_cb_t callback,
					void *user_data,
					l_timeout_destroy_cb_t destroy)
{
	if (unlikely(!timeout))
		return;

	if (timeout->destroy)
		timeout->destroy(timeout->user_data);

	timeout->callback = callback;
	timeout->user_data = user_data;
	timeout->destroy = destroy;
}
