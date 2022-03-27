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

#include <poll.h>

/* Function to be called when there are event for some descriptor */
typedef void (*poll_handler)(struct pollfd *pollfd);

int poll_register_fd(int fd, short events, poll_handler ph);
int poll_unregister_fd(int fd, poll_handler ph);

void poll_dispatch_loop(void);
