/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011-2014  Intel Corporation
 *  Copyright (C) 2002-2010  Marcel Holtmann <marcel@holtmann.org>
 *
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
int mainloop_run_with_signal(mainloop_signal_func func, void *user_data);

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
int mainloop_sd_notify(const char *state);
