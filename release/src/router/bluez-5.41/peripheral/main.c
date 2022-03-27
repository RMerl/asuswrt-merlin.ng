/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2015  Intel Corporation. All rights reserved.
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h>
#include <poll.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mount.h>

#ifndef WAIT_ANY
#define WAIT_ANY (-1)
#endif

#include "src/shared/mainloop.h"
#include "peripheral/efivars.h"
#include "peripheral/attach.h"
#include "peripheral/gap.h"
#include "peripheral/log.h"

#ifndef RTCONFIG_LANTIQ
#define MS_STRICTATIME (1<<24)
#endif

static bool is_init = false;
static pid_t shell_pid = -1;

static const struct {
	const char *target;
	const char *linkpath;
} dev_table[] = {
	{ "/proc/self/fd",	"/dev/fd"	},
	{ "/proc/self/fd/0",	"/dev/stdin"	},
	{ "/proc/self/fd/1",	"/dev/stdout"	},
	{ "/proc/self/fd/2",	"/dev/stderr"	},
	{ }
};

static const struct {
	const char *fstype;
	const char *target;
	const char *options;
	unsigned long flags;
} mount_table[] = {
	{ "sysfs",    "/sys",		NULL,	MS_NOSUID|MS_NOEXEC|MS_NODEV },
	{ "proc",     "/proc",		NULL,	MS_NOSUID|MS_NOEXEC|MS_NODEV },
	{ "devtmpfs", "/dev",		NULL,	MS_NOSUID|MS_STRICTATIME },
	{ "efivarfs", "/sys/firmware/efi/efivars",
					NULL,	MS_NOSUID|MS_NOEXEC|MS_NODEV },
	{ "pstore",   "/sys/fs/pstore",	NULL,	MS_NOSUID|MS_NOEXEC|MS_NODEV },
	{ }
};

static void prepare_filesystem(void)
{
	int i;

	if (!is_init)
		return;

	for (i = 0; mount_table[i].fstype; i++) {
		struct stat st;

		if (lstat(mount_table[i].target, &st) < 0) {
			printf("Creating %s\n", mount_table[i].target);
			mkdir(mount_table[i].target, 0755);
		}

		printf("Mounting %s to %s\n", mount_table[i].fstype,
						mount_table[i].target);

		if (mount(mount_table[i].fstype,
				mount_table[i].target,
				mount_table[i].fstype,
				mount_table[i].flags, NULL) < 0)
			perror("Failed to mount filesystem");
	}

	for (i = 0; dev_table[i].target; i++) {
		printf("Linking %s to %s\n", dev_table[i].linkpath,
						dev_table[i].target);

		if (symlink(dev_table[i].target, dev_table[i].linkpath) < 0)
			perror("Failed to create device symlink");
	}
}

static void run_shell(void)
{
	pid_t pid;

	printf("Starting shell\n");

	pid = fork();
	if (pid < 0) {
		perror("Failed to fork new process");
		return;
	}

	if (pid == 0) {
		char *prg_argv[] = { "/bin/sh", NULL };
		char *prg_envp[] = { NULL };

		execve(prg_argv[0], prg_argv, prg_envp);
		exit(0);
	}

	printf("PID %d created\n", pid);

	shell_pid = pid;
}

static void exit_shell(void)
{
	shell_pid = -1;

	if (!is_init) {
		mainloop_quit();
		return;
	}

	run_shell();
}

static void signal_callback(int signum, void *user_data)
{
	switch (signum) {
	case SIGINT:
	case SIGTERM:
		mainloop_quit();
		break;
	case SIGCHLD:
		while (1) {
			pid_t pid;
			int status;

			pid = waitpid(WAIT_ANY, &status, WNOHANG);
			if (pid < 0 || pid == 0)
				break;

			if (WIFEXITED(status)) {
				printf("PID %d exited (status=%d)\n",
						pid, WEXITSTATUS(status));

				if (pid == shell_pid)
					exit_shell();
			} else if (WIFSIGNALED(status)) {
				printf("PID %d terminated (signal=%d)\n",
							pid, WTERMSIG(status));

				if (pid == shell_pid)
					exit_shell();
			}
		}
		break;
	}
}

int main(int argc, char *argv[])
{
	sigset_t mask;
	int exit_status;

	if (getpid() == 1 && getppid() == 0)
		is_init = true;

	mainloop_init();

	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGTERM);
	sigaddset(&mask, SIGCHLD);

	mainloop_set_signal(&mask, signal_callback, NULL, NULL);

	printf("Bluetooth periperhal ver %s\n", VERSION);

	prepare_filesystem();

	if (is_init) {
		uint8_t addr[6];

		if (efivars_read("BluetoothStaticAddress", NULL,
							addr, 6) < 0) {
			printf("Generating new persistent static address\n");

			addr[0] = rand();
			addr[1] = rand();
			addr[2] = rand();
			addr[3] = 0x34;
			addr[4] = 0x12;
			addr[5] = 0xc0;

			efivars_write("BluetoothStaticAddress",
					EFIVARS_NON_VOLATILE |
					EFIVARS_BOOTSERVICE_ACCESS |
					EFIVARS_RUNTIME_ACCESS,
					addr, 6);
		}

		gap_set_static_address(addr);

		run_shell();
	}

	log_open();
	gap_start();

	if (is_init)
		attach_start();

	exit_status = mainloop_run();

	if (is_init)
		attach_stop();

	gap_stop();
	log_close();

	return exit_status;
}
