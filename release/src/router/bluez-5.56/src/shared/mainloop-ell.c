// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2019  Intel Corporation
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

#include <ell/ell.h>

#include "mainloop.h"

static bool is_initialized;
static int exit_status;
static mainloop_signal_func sig_func;

static void l_sig_func(uint32_t signo, void *user_data)
{
	if (sig_func)
		sig_func(signo, user_data);
}

void mainloop_init(void)
{
	is_initialized = l_main_init();
}

void mainloop_quit(void)
{
	l_main_quit();
}

void mainloop_exit_success(void)
{
	exit_status = EXIT_SUCCESS;
	l_main_quit();
}

void mainloop_exit_failure(void)
{
	exit_status = EXIT_FAILURE;
	l_main_quit();
}

int mainloop_run(void)
{
	if (!is_initialized)
		return -EINVAL;

	l_main_run();

	is_initialized = false;
	sig_func = NULL;

	return exit_status;
}

int mainloop_run_with_signal(mainloop_signal_func func, void *user_data)
{
	if (!is_initialized || !func)
		return -EINVAL;

	/* Workaround for sign discrepancy in ell and bluez */
	sig_func = func;

	return l_main_run_with_signal(l_sig_func, user_data);
}

int mainloop_add_fd(int fd, uint32_t events, mainloop_event_func callback,
				void *user_data, mainloop_destroy_func destroy)
{
	return -ENOSYS;
}

int mainloop_modify_fd(int fd, uint32_t events)
{
	return -ENOSYS;
}

int mainloop_remove_fd(int fd)
{
	return -ENOSYS;
}

int mainloop_add_timeout(unsigned int msec, mainloop_timeout_func callback,
				void *user_data, mainloop_destroy_func destroy)
{
	return -ENOSYS;
}

int mainloop_modify_timeout(int fd, unsigned int msec)
{
	return -ENOSYS;
}

int mainloop_remove_timeout(int id)
{
	return -ENOSYS;
}

int mainloop_set_signal(sigset_t *mask, mainloop_signal_func callback,
				void *user_data, mainloop_destroy_func destroy)
{
	return -ENOSYS;
}
