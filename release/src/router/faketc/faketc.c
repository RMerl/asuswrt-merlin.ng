/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 *
 * Faketc - wedge for iproute2 tc to inject codel support
 *          into tc calls made by closed source libbwdpi
 *          in Asuswrt.
 *
 * Copyright 2019 Eric Sauvageau.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <bcmnvram.h>
#include <shared.h>

char **do_cmd(int argc, char **argv);
char **do_class(int argc, char **argv);
char **do_qdisc(int argc, char **argv);

int sched;
char overheadval[5];



char **do_cmd(int argc, char **argv)
{
	char **argvptr;
	int argcptr;

	argvptr = argv;
	argcptr = argc;

	/* Skip options to locate the object */
	while (argcptr > 1) {
		if (argvptr[1][0] != '-')
			break;
		argcptr--; argvptr++;
	}

	if (!strcmp(argvptr[1], "qdisc"))
		return do_qdisc(argc, argv);
	else if (!strcmp(argvptr[1], "class"))
		 return do_class(argc, argv);
	else
		return argv;
}


char **do_class(int argc, char **argv)
{
	int overhead;
	int i = 0, j = 0, found = 0;
	char **newargv;

	overhead = nvram_get_int("qos_overhead");

	if (overhead == 0)
		return argv;

	newargv = malloc(sizeof(argv) * (argc + 1 + 5));
	while (argv[i]) {
		if (!found && !strcmp(argv[i], "htb"))
			found = 1;
		newargv[j++] = argv[i++];
	}

	if (found) {
		newargv[j++] = "overhead";

		snprintf(overheadval, sizeof(overheadval)-1, "%d", overhead);
		newargv[j++] = overheadval;
		newargv[j++] = "linklayer";
		newargv[j++] = (nvram_get_int("qos_atm") ? "atm" : "ethernet");
		newargv[j++] = NULL;
	}

	return newargv;
}

char **do_qdisc(int argc, char **argv)
{
	char *interface = NULL;
	int i = 0, j = 0, found = 0;
	char **newargv;

	newargv = malloc(sizeof(argv) * (argc + 1 + 2));

	while (argv[i]) {
		if (!interface && !strcmp(argv[i], "dev")) {
			interface = argv[i+1];
			newargv[j++] = argv[i];
		} else if (!strcmp(argv[i], "sfq")) {
			found = 1;
			if (sched == 1)
				newargv[j++] = "codel";
			else if (sched == 2)
				newargv[j++] = "fq_codel";
			else
				newargv[j++] = argv[i];
		} else {
			newargv[j++] = argv[i];
		}
		i++;
	}

	if (!interface)
		return argv;

	if (found) {
		newargv[j++] = (strcmp(interface, "br0") ? "noecn" : "ecn");
		newargv[j++] = NULL;
	}

	return newargv;
}


int main(int argc, char **argv)
{
	char **newargv;

	sched = nvram_get_int("qos_sched");

        if ((argc > 4) && sched)
		newargv = do_cmd(argc, argv);
	else
		newargv = argv;

	newargv[0] = "/usr/sbin/realtc";
	execvp("/usr/sbin/realtc", (char **)newargv);

        return 0;
}
