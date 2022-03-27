/*
 * Copyright (C) 2013 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <stdio.h>
#include <errno.h>
#include <poll.h>

#include "pollhandler.h"

/*
 * Code that allows to poll multiply file descriptors for events
 * File descriptors can be added and removed at runtime
 *
 * Call poll_register_fd function first to add file descriptors to monitor
 * Then call poll_dispatch_loop that will poll all registered file descriptors
 * as long as they are not unregistered.
 *
 * When event happen on given fd appropriate user supplied handler is called
 */

/* Maximum number of files to monitor */
#define MAX_OPEN_FD 10

/* Storage for pollfd structures for monitored file descriptors */
static struct pollfd fds[MAX_OPEN_FD];
static poll_handler fds_handler[MAX_OPEN_FD];
/* Number of registered file descriptors */
static int fds_count = 0;

/*
 * Function polls file descriptor in loop and calls appropriate handler
 * on event. Function returns when there is no more file descriptor to
 * monitor
 */
void poll_dispatch_loop(void)
{
	while (fds_count > 0) {
		int i;
		int cur_fds_count = fds_count;
		int ready = poll(fds, fds_count, 1000);

		for (i = 0; i < fds_count && ready > 0; ++i) {
			if (fds[i].revents == 0)
				continue;

			fds_handler[i](fds + i);
			ready--;
			/*
			 * If handler was remove from table
			 * just skip the rest and poll again
			 * This is due to reordering of tables in
			 * register/unregister functions
			 */
			if (cur_fds_count != fds_count)
				break;
		}
	}
}

/*
 * Registers file descriptor to be monitored for events (see man poll(2))
 * for events.
 *
 * return non negative value on success
 * -EMFILE when there are to much descriptors
 */
int poll_register_fd(int fd, short events, poll_handler ph)
{
	if (fds_count >= MAX_OPEN_FD)
		return -EMFILE;

	fds_handler[fds_count] = ph;
	fds[fds_count].fd = fd;
	fds[fds_count].events = events;
	fds_count++;

	return fds_count;
}

/*
 * Unregisters file descriptor
 * Both fd and ph must match previously registered data
 *
 * return 0 if unregister succeeded
 * -EBADF if arguments do not match any register handler
 */
int poll_unregister_fd(int fd, poll_handler ph)
{
	int i;

	for (i = 0; i < fds_count; ++i) {
		if (fds_handler[i] == ph && fds[i].fd == fd) {
			fds_count--;
			if (i < fds_count) {
				fds[i].fd = fds[fds_count].fd;
				fds[i].events = fds[fds_count].events;
				fds_handler[i] = fds_handler[fds_count];
			}
			return 0;
		}
	}
	return -EBADF;
}
