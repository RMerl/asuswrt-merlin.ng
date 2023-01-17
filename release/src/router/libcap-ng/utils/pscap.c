/*
 * pscap.c - A program that lists running processes with capabilities
 * Copyright (c) 2009,2012,2020 Red Hat Inc.
 * All Rights Reserved.
 *
 * This software may be freely redistributed and/or modified under the
 * terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor
 * Boston, MA 02110-1335, USA.
 *
 * Authors:
 *   Steve Grubb <sgrubb@redhat.com>
 */

#include "config.h"
#include <stdio.h>
#include <stdio_ext.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdbool.h>
#include <sys/stat.h>
#include "cap-ng.h"

#define CMD_LEN 16
#define USERNS_MARK_LEN 2

static void usage(void)
{
	fprintf(stderr, "usage: pscap [-a]\n");
	exit(1);
}

/*
 * Precise recursive checks for parent-child relation between namespaces 
 * using ioctl() were avoided, because there didn't seem to be any case when
 * we may dereference the namespace symlink in /proc/PID/ns for processes in
 * user namespaces other than the current or child ones. Thus, the check just
 * tries to dereference the link and checks that it does not point to the
 * current NS.
 */
static bool in_child_userns(int pid)
{
	char ns_file_path[32];
	struct stat statbuf;
	ino_t own_ns_inode;
	dev_t own_ns_dev;

	if (stat("/proc/self/ns/user", &statbuf) < 0)
		return false;

	own_ns_inode = statbuf.st_ino;
	own_ns_dev = statbuf.st_dev;

	snprintf(ns_file_path, 32, "/proc/%d/ns/user", pid);
	if (stat(ns_file_path, &statbuf) < 0)
		return false;

	return statbuf.st_ino != own_ns_inode || statbuf.st_dev != own_ns_dev;
}

int main(int argc, char *argv[])
{
	DIR *d;
	struct dirent *ent;
	int header = 0, show_all = 0, caps;
	pid_t our_pid = getpid();

	if (argc > 2) {
		fputs("Too many arguments\n", stderr);
		usage();
	}
	if (argc == 2) {
		if (strcmp(argv[1], "-a") == 0)
			show_all = 1;
		else
			usage();
	}

	d = opendir("/proc");
	if (d == NULL) {
		fprintf(stderr, "Can't open /proc: %s\n", strerror(errno));
		return 1;
	}
	while (( ent = readdir(d) )) {
		int pid, ppid, uid = -1, euid = -1;
		char buf[100];
		char *tmp, cmd[CMD_LEN + USERNS_MARK_LEN], state, *name = NULL;
		int fd, len;
		struct passwd *p;

		// Skip non-process dir entries
		if(*ent->d_name<'0' || *ent->d_name>'9')
			continue;
		errno = 0;
		pid = strtol(ent->d_name, NULL, 10);
		if (errno)
			continue;

		/* Skip our pid so we aren't listed */
		if (pid == our_pid)
			continue;

		// Parse up the stat file for the proc
		snprintf(buf, 32, "/proc/%d/stat", pid);
		fd = open(buf, O_RDONLY|O_CLOEXEC, 0);
		if (fd < 0)
			continue;
		len = read(fd, buf, sizeof buf - 1);
		close(fd);
		if (len < 40)
			continue;
		buf[len] = 0;
		tmp = strrchr(buf, ')');
		if (tmp)
			*tmp = 0;
		else
			continue;
		memset(cmd, 0, sizeof(cmd));
		sscanf(buf, "%d (%15c", &ppid, cmd);
		sscanf(tmp+2, "%c %d", &state, &ppid);

		// Skip kthreads
		if (pid == 2 || ppid == 2)
			continue;

		if (!show_all && pid == 1)
			continue;

		// now get the capabilities
		capng_clear(CAPNG_SELECT_ALL);
		capng_setpid(pid);
		if (capng_get_caps_process())
			continue;

		// And print out anything with capabilities
		caps = capng_have_capabilities(CAPNG_SELECT_CAPS);
		if (caps > CAPNG_NONE) {
			// Get the effective uid
			FILE *f;
			int line;
			snprintf(buf, 32, "/proc/%d/status", pid);
			f = fopen(buf, "rte");
			if (f == NULL)
				euid = 0;
			else {
				line = 0;
				__fsetlocking(f, FSETLOCKING_BYCALLER);
				while (fgets(buf, sizeof(buf), f)) {
					if (line == 0) {
						line++;
						continue;
					}
					if (memcmp(buf, "Uid:", 4) == 0) {
						int id;
						sscanf(buf, "Uid: %d %d",
							&id, &euid);
						break;
					}
				}
				fclose(f);
			}

			if (header == 0) {
				printf("%-5s %-5s %-10s  %-18s  %s\n",
				    "ppid", "pid", "name", "command",
				    "capabilities");
				header = 1;
			}
			if (euid == 0) {
				// Take short cut for this one
				name = "root";
				uid = 0;
			} else if (euid != uid) {
				// Only look up if name changed
				p = getpwuid(euid);
				uid = euid;
				if (p)
					name = p->pw_name;
				// If not taking this branch, use last val
			}

			if (in_child_userns(pid))
				strcat(cmd, " *");

			if (name) {
				printf("%-5d %-5d %-10s  %-18s  ", ppid, pid,
					name, cmd);
			} else
				printf("%-5d %-5d %-10d  %-18s  ", ppid, pid,
					uid, cmd);
			if (caps == CAPNG_PARTIAL) {
				capng_print_caps_text(CAPNG_PRINT_STDOUT,
							CAPNG_PERMITTED);
				if (capng_have_capabilities(
					    CAPNG_SELECT_AMBIENT) > CAPNG_NONE)
					printf(" @");
				if (capng_have_capabilities(CAPNG_SELECT_BOUNDS)
							 > CAPNG_NONE)
					printf(" +");
				printf("\n");
			} else {
				printf("full");
				if (capng_have_capabilities(
					    CAPNG_SELECT_AMBIENT) > CAPNG_NONE)
					printf(" @");
				if (capng_have_capabilities(CAPNG_SELECT_BOUNDS)
							 > CAPNG_NONE)
					printf(" +");
				printf("\n");
			}
		}
	}
	closedir(d);
	return 0;
}


