/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011-2014  Intel Corporation
 *  Copyright (C) 2002-2010  Marcel Holtmann <marcel@holtmann.org>
 *
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

#include <signal.h>
#include <sys/epoll.h>

typedef void (*mainloop_destroy_func) (void *user_data);

typedef void (*mainloop_event_func) (int fd, uint32_t events, void *user_data);
typedef void (*mainloop_timeout_func) (int id, void *user_data);
typedef void (*mainloop_signal_func) (int signum, void *user_data);

void mainloop_init(void);
void mainloop_quit(void);
void mainloop_exit_success(void);
void mainloop_exit_failure(void);
int mainloop_run(void);

int mainloop_add_fd(int fd, uint32_t events, mainloop_event_func callback,
				void *user_data, mainloop_destroy_func destroy);
int mainloop_modify_fd(int fd, uint32_t events);
int mainloop_remove_fd(int fd);

int mainloop_add_timeout(unsigned int msec, mainloop_timeout_func callback,
				void *user_data, mainloop_destroy_func destroy);
int mainloop_modify_timeout(int fd, unsigned int msec);
int mainloop_remove_timeout(int id);

int mainloop_set_signal(sigset_t *mask, mainloop_signal_func callback,
				void *user_data, mainloop_destroy_func destroy);
