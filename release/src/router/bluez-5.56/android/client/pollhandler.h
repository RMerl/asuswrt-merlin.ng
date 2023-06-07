/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Copyright (C) 2013 Intel Corporation
 *
 */

#include <poll.h>

/* Function to be called when there are event for some descriptor */
typedef void (*poll_handler)(struct pollfd *pollfd);

int poll_register_fd(int fd, short events, poll_handler ph);
int poll_unregister_fd(int fd, poll_handler ph);

void poll_dispatch_loop(void);
