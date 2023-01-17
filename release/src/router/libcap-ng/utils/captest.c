/*
 * captest.c - A program that demonstrates and outputs capabilities
 * Copyright (c) 2009, 2013, 2020 Red Hat Inc.
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
 */
#include "config.h"
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <cap-ng.h>
#include <sys/prctl.h>
#ifdef HAVE_LINUX_SECUREBITS_H
#include <linux/securebits.h>
#endif

/* children can't get caps back */
#ifndef SECURE_NOROOT
#define SECURE_NOROOT                   0
#endif
#ifndef SECURE_NOROOT_LOCKED
#define SECURE_NOROOT_LOCKED            1  /* make bit-0 immutable */
#endif
/* Setuid apps run by uid 0 don't get caps back */
#ifndef SECURE_NO_SETUID_FIXUP
#define SECURE_NO_SETUID_FIXUP          2  
#endif
#ifndef SECURE_NO_SETUID_FIXUP_LOCKED
#define SECURE_NO_SETUID_FIXUP_LOCKED   3  /* make bit-2 immutable */
#endif

static int text = 0, no_child = 0, lock = 0, ambient = 0;

static void report(void)
{
	int rc, escalated = 0, need_comma = 0;
	uid_t uid, euid, suid;
	gid_t gid, egid, sgid;

	// Refresh what we have for capabilities
	if (capng_get_caps_process()) {
		printf("Error getting capabilities\n");
		exit(1);
	}

	// Check user credentials
	getresuid(&uid, &euid, &suid);
	getresgid(&gid, &egid, &sgid);
	if (no_child) {
		if ((uid != euid && uid != 0) ||
					capng_have_capability(CAPNG_EFFECTIVE,
						 CAP_SETUID)) {
			printf("Attempting to regain root...");
			setuid(0);
			getresuid(&uid, &euid, &suid);
			if (uid == 0) {
				printf("SUCCESS - PRIVILEGE ESCALATION POSSIBLE\n");
				setgid(0);
				getresgid(&gid, &egid, &sgid);
				escalated = 1;
			} else
				printf("FAILED\n");
		}
		printf("Child ");
	}
	printf("User  credentials uid:%u euid:%u suid:%u\n", uid, euid, suid);
	if (no_child)
		printf("Child ");
	printf("Group credentials gid:%u egid:%u sgid:%u\n", gid, egid, sgid);
	if (uid != euid || gid != egid)
		printf("Note: app has mismatching credentials!!\n");

	// Check capabilities
	if (text) {
		if (capng_have_capabilities(CAPNG_SELECT_CAPS) == CAPNG_NONE) {
			if (no_child)
				printf("Child capabilities: none\n");
			else
				printf("Current capabilities: none\n");
		} else {
			if (no_child)
				printf("Child ");
			printf("Effective: ");
			capng_print_caps_text(CAPNG_PRINT_STDOUT,
					CAPNG_EFFECTIVE);
			printf("\n");
			if (no_child)
				printf("Child ");
			printf("Permitted: ");
			capng_print_caps_text(CAPNG_PRINT_STDOUT,
					CAPNG_PERMITTED);
			printf("\n");
			if (no_child)
				printf("Child ");
			printf("Inheritable: ");
			capng_print_caps_text(CAPNG_PRINT_STDOUT,
					CAPNG_INHERITABLE);
			printf("\n");
			if (no_child)
				printf("Child ");
			printf("Bounding Set: ");
			capng_print_caps_text(CAPNG_PRINT_STDOUT,
					CAPNG_BOUNDING_SET);
			printf("\n");
			if (no_child)
				printf("Child ");
			printf("Ambient: ");
			capng_print_caps_text(CAPNG_PRINT_STDOUT,
					CAPNG_AMBIENT);
			printf("\n");
		}
	} else {
		if (capng_have_capabilities(CAPNG_SELECT_CAPS) == CAPNG_NONE) {
			if (no_child)
				printf("Child capabilities: none\n");
			else
				printf("Current capabilities: none\n");
		} else {
			if (no_child)
				printf("Child capabilities:\n");
			capng_print_caps_numeric(CAPNG_PRINT_STDOUT,
					CAPNG_SELECT_ALL);
		}
	}

	// Now check securebits flags
#ifdef PR_SET_SECUREBITS
	if (no_child)
		printf("Child ");
	printf("securebits flags: ");
	rc = prctl(PR_GET_SECUREBITS, 1 << SECURE_NOROOT);
	if (rc & (1 << SECURE_NOROOT)) {
		printf("NOROOT");
		need_comma = 1;
	}
	rc = prctl(PR_GET_SECUREBITS, 1 << SECURE_NOROOT_LOCKED);
	if (rc & (1 << SECURE_NOROOT_LOCKED)) {
		if (need_comma)
			printf(", ");
		printf("NOROOT_LOCKED");
		need_comma = 1;
	}
	rc = prctl(PR_GET_SECUREBITS, 1 << SECURE_NO_SETUID_FIXUP);
	if (rc & (1 << SECURE_NO_SETUID_FIXUP)) {
		if (need_comma)
			printf(", ");
		printf("NO_SETUID_FIXUP");
		need_comma = 1;
	}
	rc = prctl(PR_GET_SECUREBITS, 1 << SECURE_NO_SETUID_FIXUP_LOCKED);
	if (rc & (1 << SECURE_NO_SETUID_FIXUP_LOCKED)) {
		if (need_comma)
			printf(", ");
		printf("NO_SETUID_FIXUP_LOCKED");
		need_comma = 1;
	}
	if (need_comma == 0)
		printf("none");
	printf("\n");
#endif
	// Now do child process checks
	if (no_child == 0 || escalated) {
		printf("Attempting direct access to shadow...");
		if (access("/etc/shadow", R_OK) == 0)
			printf("SUCCESS\n");
		else
			printf("FAILED (%s)\n", strerror(errno));
	}
	if (no_child == 0) {
		printf("Attempting to access shadow by child process...");
		rc = system("cat /etc/shadow > /dev/null 2>&1");
		if (rc == 0)
			printf("SUCCESS\n");
		else
			printf("FAILED\n");
		if (text)
			system("/usr/bin/captest --no-child --text");
		else
			system("/usr/bin/captest --no-child");
	}
}

static void usage(void)
{
	printf("usage: captest [ --ambient | --drop-all | --drop-caps | --id | --init-grp ] [ --lock ] [ --text ]\n");
}

int main(int argc, char *argv[])
{
	int which = 0, i;

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--text") == 0)
			text = 1;
		else if (strcmp(argv[i], "--no-child") == 0)
			no_child = 1;
		else if (strcmp(argv[i], "--lock") == 0)
			lock = 1;
		else if (strcmp(argv[i], "--drop-all") == 0)
			which = 1;
		else if (strcmp(argv[i], "--drop-caps") == 0)
			which = 2;
		else if (strcmp(argv[i], "--id") == 0)
			which = 3;
		else if (strcmp(argv[i], "--init-grp") == 0)
			which = 4;
		else if (strcmp(argv[i], "--ambient") == 0)
			ambient = 1;
		else {
			usage();
			return 0;
		}
	}
	switch (which)
	{
		case 1:
			capng_clear(CAPNG_SELECT_ALL);
			if (lock)
				capng_lock();
			capng_apply(CAPNG_SELECT_ALL);
			report();
			break;
		case 2:
			capng_clear(CAPNG_SELECT_CAPS);
			if (ambient)
				capng_update(CAPNG_ADD, CAPNG_AMBIENT,
					     CAP_CHOWN);
			if (lock)
				capng_lock();
			if (ambient)
			    capng_apply(CAPNG_SELECT_CAPS|CAPNG_SELECT_AMBIENT);
			else
				capng_apply(CAPNG_SELECT_CAPS);
			report();
			break;
		case 3:
		case 4: {
			int rc;

			capng_clear(CAPNG_SELECT_BOTH);
			capng_update(CAPNG_ADD, CAPNG_EFFECTIVE|CAPNG_PERMITTED,
					CAP_CHOWN);
			if (ambient)
				capng_update(CAPNG_ADD,
					     CAPNG_INHERITABLE|CAPNG_AMBIENT,
					     CAP_CHOWN);
			if (which == 4)
				rc = capng_change_id(99, 99,
				CAPNG_INIT_SUPP_GRP | CAPNG_CLEAR_BOUNDING);
			else
				rc = capng_change_id(99, 99,
				CAPNG_DROP_SUPP_GRP | CAPNG_CLEAR_BOUNDING);
			if (rc < 0) {
				printf("Error changing uid: %d\n", rc);
				capng_print_caps_text(CAPNG_PRINT_STDOUT,
					CAPNG_EFFECTIVE);
				printf("\n");
				exit(1);
			}
			printf("Keeping CAP_CHOWN to show capabilities across uid change.\n");
			report();
			} break;
		case 0:
			if (lock)
				capng_lock();
			report();
			break;
	}
	return 0;
}

