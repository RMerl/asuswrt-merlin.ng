/*
 * netcap.c - A program that lists network apps with capabilities
 * Copyright (c) 2009-10,2012,2020 Red Hat Inc.
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
 *
 * The /proc/net/tcp|udp|raw parsing code was borrowed from netstat.c
 */

#include "config.h"
#include <stdio.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <pwd.h>
#include "cap-ng.h"
#include "proc-llist.h"

static llist l;
static int perm_warn = 0, header = 0, last_uid = -1;
static char *tacct = NULL;

static void usage(void)
{
	fprintf(stderr, "usage: netcap\n");
	exit(1);
}

static int collect_process_info(void)
{
	DIR *d, *f;
	struct dirent *ent;
	d = opendir("/proc");
	if (d == NULL) {
		fprintf(stderr, "Can't open /proc: %s\n", strerror(errno));
		return 1;
	}
	while (( ent = readdir(d) )) {
		FILE *sf;
		int pid, ppid;
		capng_results_t caps;
		char buf[100];
		char *tmp, cmd[16], state;
		char *text = NULL, *bounds = NULL, *ambient = NULL;
		int fd, len, euid = -1;

		// Skip non-process dir entries
		if(*ent->d_name<'0' || *ent->d_name>'9')
			continue;
		errno = 0;
		pid = strtol(ent->d_name, NULL, 10);
		if (errno)
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

		// now get the capabilities
		capng_clear(CAPNG_SELECT_ALL);
		capng_setpid(pid);
		if (capng_get_caps_process())
			continue;
		caps = capng_have_capabilities(CAPNG_SELECT_CAPS);
		if (caps <= CAPNG_NONE)
			continue;
		if (caps == CAPNG_FULL) {
			text = strdup("full");
			if (!text) {
				fprintf(stderr, "Out of memory\n");
				continue;
			}
		} else
			text = capng_print_caps_text(CAPNG_PRINT_BUFFER,
					CAPNG_PERMITTED);

		// Get the effective uid
		snprintf(buf, 32, "/proc/%d/status", pid);
		sf = fopen(buf, "rte");
		if (sf == NULL)
			euid = 0;
		else {
			int line = 0;
			__fsetlocking(sf, FSETLOCKING_BYCALLER);
			while (fgets(buf, sizeof(buf), sf)) {
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
			fclose(sf);
		}

		caps = capng_have_capabilities(CAPNG_SELECT_AMBIENT);
		if (caps > CAPNG_NONE)
			ambient = strdup("@");
		else
			ambient = strdup("");
		if (!ambient) {
			fprintf(stderr, "Out of memory\n");
			free(text);
			continue;
		}

		// Now record the bounding set information
		caps = capng_have_capabilities(CAPNG_SELECT_BOUNDS);
		if (caps > CAPNG_NONE)
			bounds = strdup("+");
		else
			bounds = strdup("");
		if (!bounds) {
			fprintf(stderr, "Out of memory\n");
			free(text);
			free(ambient);
			continue;
		}

		// Now lets get the inodes each process has open
		snprintf(buf, 32, "/proc/%d/fd", pid);
		f = opendir(buf);
		if (f == NULL) {
			if (errno == EACCES) {
				if (perm_warn == 0) {
					fprintf(stderr,
						"You may need to be root to "
						"get a full report\n");
					perm_warn = 1;
				}
			} else
				fprintf(stderr, "Can't open %s: %s\n", buf,
					strerror(errno));
			free(text);
			free(bounds);
			free(ambient);
			continue;
		}
		// For each file in the fd dir...
		while (( ent = readdir(f) )) {
			char line[256], ln[256], *s, *e;
			unsigned long inode;
			lnode node;
			int llen;

			if (ent->d_name[0] == '.')
				continue;
			snprintf(ln, 256, "%s/%s", buf, ent->d_name);
			if ((llen = readlink(ln, line, sizeof(line)-1)) < 0)
				continue;
			line[llen] = 0;

			// Only look at the socket entries
			if (memcmp(line, "socket:", 7) == 0) {
				// Type 1 sockets
				s = strchr(line+7, '[');
				if (s == NULL)
					continue;
				s++;
				e = strchr(s, ']');
				if (e == NULL)
					continue;
				*e = 0;
			} else if (memcmp(line, "[0000]:", 7) == 0) {
				// Type 2 sockets
				s = line + 8;
			} else
				continue;
			errno = 0;
			inode = strtoul(s, NULL, 10);
			if (errno)
				continue;
			node.ppid = ppid;
			node.pid = pid;
			node.uid = euid;
			node.cmd = strdup(cmd);
			node.inode = inode;
			node.capabilities = strdup(text);
			node.bounds = strdup(bounds);
			node.ambient = strdup(ambient);
			if (node.cmd && node.capabilities && node.bounds &&
			    node.ambient)
				// We make one entry for each socket inode
				list_append(&l, &node);
			else {
				free(node.cmd);
				free(node.capabilities);
				free(node.bounds);
				free(node.ambient);
			}
		}
		closedir(f);
		free(text);
		free(bounds);
		free(ambient);
	}
	closedir(d);
	return 0;
}

static void report_finding(unsigned int port, const char *type, const char *ifc)
{
	struct passwd *p;
	lnode *n = list_get_cur(&l);

	// And print out anything with capabilities
	if (header == 0) {
		printf("%-5s %-5s %-10s %-16s %-8s %-6s %s\n",
			"ppid", "pid", "acct", "command", "type", "port",
			"capabilities");
			header = 1;
	}
	if (n->uid == 0) {
		// Take short cut for this one
		tacct = "root";
		last_uid = 0;
	} else if (last_uid != (int)n->uid) {
		// Only look up if name changed
		p = getpwuid(n->uid);
		last_uid = n->uid;
		if (p)
			tacct = p->pw_name;
		// If not taking this branch, use last val
	}
	if (tacct) {
		printf("%-5d %-5d %-10s", n->ppid, n->pid, tacct);
	} else
		printf("%-5d %-5d %-10d", n->ppid, n->pid, last_uid);
	printf(" %-16s %-8s", n->cmd, type);
	if (ifc)
		printf(" %-6s", ifc);
	else
		printf(" %-6u", port);
	printf(" %s %s%s\n", n->capabilities, n->ambient, n->bounds);
}

static void read_tcp(const char *proc, const char *type)
{
	int line = 0;
	FILE *f;
	char buf[256];
	unsigned long rxq, txq, time_len, retr, inode;
	unsigned int local_port, rem_port, state, timer_run;
	int d, uid, timeout;
	char rem_addr[128], local_addr[128], more[512];

	f = fopen(proc, "rte");
	if (f == NULL) {
		if (errno != ENOENT)
			fprintf(stderr, "Can't open %s: %s\n",
				proc, strerror(errno));
		return;
	}
	__fsetlocking(f, FSETLOCKING_BYCALLER);
	while (fgets(buf, sizeof(buf), f)) {
		if (line == 0) {
			line++;
			continue;
		}
		more[0] = 0;
		sscanf(buf, "%d: %64[0-9A-Fa-f]:%X %64[0-9A-Fa-f]:%X %X "
			"%lX:%lX %X:%lX %lX %d %d %lu %511s\n",
			&d, local_addr, &local_port, rem_addr, &rem_port,
			&state, &txq, &rxq, &timer_run, &time_len, &retr,
			&uid, &timeout, &inode, more);
		if (list_find_inode(&l, inode))
			report_finding(local_port, type, NULL);
	}
	fclose(f);
}

static void read_udp(const char *proc, const char *type)
{
	int line = 0;
	FILE *f;
	char buf[256];
	unsigned long rxq, txq, time_len, retr, inode;
	unsigned int local_port, rem_port, state, timer_run;
	int d, uid, timeout;
	char rem_addr[128], local_addr[128], more[512];

	f = fopen(proc, "rte");
	if (f == NULL) {
		if (errno != ENOENT)
			fprintf(stderr, "Can't open %s: %s\n",
					proc, strerror(errno));
		return;
	}
	__fsetlocking(f, FSETLOCKING_BYCALLER);
	while (fgets(buf, sizeof(buf), f)) {
		if (line == 0) {
			line++;
			continue;
		}
		more[0] = 0;
		sscanf(buf, "%d: %64[0-9A-Fa-f]:%X %64[0-9A-Fa-f]:%X %X "
			"%lX:%lX %X:%lX %lX %d %d %lu %511s\n",
			&d, local_addr, &local_port, rem_addr, &rem_port,
			&state, &txq, &rxq, &timer_run, &time_len, &retr,
			&uid, &timeout, &inode, more);
		if (list_find_inode(&l, inode))
			report_finding(local_port, type, NULL);
	}
	fclose(f);
}

static void read_raw(const char *proc, const char *type)
{
	int line = 0;
	FILE *f;
	char buf[256];
	unsigned long rxq, txq, time_len, retr, inode;
	unsigned int local_port, rem_port, state, timer_run;
	int d, uid, timeout;
	char rem_addr[128], local_addr[128], more[512];

	f = fopen(proc, "rte");
	if (f == NULL) {
		if (errno != ENOENT)
			fprintf(stderr, "Can't open %s: %s\n",
					proc, strerror(errno));
		return;
	}
	__fsetlocking(f, FSETLOCKING_BYCALLER);
	while (fgets(buf, sizeof(buf), f)) {
		if (line == 0) {
			line++;
			continue;
		}
		more[0] = 0;
		sscanf(buf, "%d: %64[0-9A-Fa-f]:%X %64[0-9A-Fa-f]:%X %X "
			"%lX:%lX %X:%lX %lX %d %d %lu %511s\n",
			&d, local_addr, &local_port, rem_addr, &rem_port,
			&state, &txq, &rxq, &timer_run, &time_len, &retr,
			&uid, &timeout, &inode, more);
		if (list_find_inode(&l, inode))
			report_finding(0, type, NULL);
	}
	fclose(f);
}

// Caller must have buffer > 16 bytes
static void get_interface(unsigned int iface, char *ifc)
{
	unsigned int line = 0;
	FILE *f;
	char buf[256], more[256];

	// Terminate the interface in case of error
	*ifc = 0;

	// Increment the interface number since header is 2 lines long
	iface++;

	f = fopen("/proc/net/dev", "rte");
	if (f == NULL) {
		if (errno != ENOENT)
			fprintf(stderr, "Can't open /proc/net/dev: %s\n",
				strerror(errno));
		return;
	}
	__fsetlocking(f, FSETLOCKING_BYCALLER);
	while (fgets(buf, sizeof(buf), f)) {
		if (line == iface) {
			char *c;
			sscanf(buf, "%16s: %255s\n", ifc, more);
			c = strchr(ifc, ':');
			if (c)
				*c = 0;
			fclose(f);
			return;
		}
		line++;
	}
	fclose(f);
}

static void read_packet(void)
{
	int line = 0;
	FILE *f;
	char buf[256];
	unsigned long sk, inode;
	unsigned int ref_cnt, type, proto, iface, r, rmem, uid;
	char more[256], ifc[32];

	f = fopen("/proc/net/packet", "rte");
	if (f == NULL) {
		if (errno != ENOENT)
			fprintf(stderr, "Can't open /proc/net/packet: %s\n",
				strerror(errno));
		return;
	}
	__fsetlocking(f, FSETLOCKING_BYCALLER);
	while (fgets(buf, sizeof(buf), f)) {
		if (line == 0) {
			line++;
			continue;
		}
		more[0] = 0;
		sscanf(buf, "%lX %u %u %X %u %u %u %u %lu %255s\n",
			&sk, &ref_cnt, &type, &proto, &iface,
			&r, &rmem, &uid, &inode, more);
		get_interface(iface, ifc);
		if (list_find_inode(&l, inode))
			report_finding(0, "pkt", ifc);
	}
	fclose(f);
}

int main(int argc, char __attribute__((unused)) *argv[])
{
	if (argc > 1) {
		fputs("Too many arguments\n", stderr);
		usage();
	}

	list_create(&l);
	collect_process_info();

	// Now we check the tcp socket list...
	read_tcp("/proc/net/tcp", "tcp");
	read_tcp("/proc/net/tcp6", "tcp6");

	// Next udp sockets...
	read_udp("/proc/net/udp", "udp");
	read_udp("/proc/net/udp6", "udp6");
	read_udp("/proc/net/udplite", "udplite");
	read_udp("/proc/net/udplite6", "udplite6");

	// Next, raw sockets...
	read_raw("/proc/net/raw", "raw");
	read_raw("/proc/net/raw6", "raw6");

	// And last, read packet sockets
	read_packet();

	// Could also do icmp,netlink,unix

	list_clear(&l);
	return 0;
}

