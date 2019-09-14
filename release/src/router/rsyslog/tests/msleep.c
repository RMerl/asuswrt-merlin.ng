/* sleeps for the specified number of MILLIseconds.
 * Primarily meant as a portable tool available everywhere for the
 * testbench (sleep 0.1 does not work on all platforms).
 *
 * Part of the testbench for rsyslog.
 *
 * Copyright 2010 Rainer Gerhards and Adiscon GmbH.
 *
 * This file is part of rsyslog.
 *
 * Rsyslog is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Rsyslog is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Rsyslog.  If not, see <http://www.gnu.org/licenses/>.
 *
 * A copy of the GPL can be found in the file "COPYING" in this distribution.
 */
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#if defined(__FreeBSD__)
#include <sys/time.h>
#else
#include <time.h>
#endif
#if defined(HAVE_SYS_SELECT_H)
#include <sys/select.h>
#endif

int main(int argc, char *argv[])
{
	struct timeval tvSelectTimeout;
	long sleepTime;

	if(argc != 2) {
		fprintf(stderr, "usage: msleep <milliseconds>\n");
		exit(1);
	}

	sleepTime = atoi(argv[1]);
	tvSelectTimeout.tv_sec = sleepTime / 1000;
	tvSelectTimeout.tv_usec = (sleepTime % 1000) * 1000; /* micro seconds */
	if(select(0, NULL, NULL, NULL, &tvSelectTimeout) == -1) {
		perror("select");
		exit(1);
	}

	return 0;
}

