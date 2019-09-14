/* gethostn - a small diagnostic utility to show what the
 * gethostname() API returns. Of course, this tool duplicates
 * functionality already found in other tools. But the point is
 * that the API shall be called by a program that is compiled like
 * rsyslogd and does exactly what rsyslog does.
 *
 * Copyright 2008 Rainer Gerhards and Adiscon GmbH.
 *
 * This file is part of rsyslog.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *       -or-
 *       see COPYING.ASL20 in the source distribution
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int __attribute__((unused)) argc, char __attribute__((unused)) *argv[])
{
	char hostname[4096]; /* this should always be sufficient ;) */
	int err;

	err = gethostname(hostname, sizeof(hostname));

	if(err) {
		perror("gethostname failed");
		exit(1);
	}

	printf("hostname of this system is '%s'.\n", hostname);

	return 0;
}
