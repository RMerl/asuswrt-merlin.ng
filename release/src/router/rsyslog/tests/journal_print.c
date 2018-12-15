/* A testing tool that tries to emit message given as argument
 * to the journal, and, if succceds (at least per journald retcode).
 * If whole operation is successful there is need to actually check
 * that message is present in journal (also veryfing journal read perms)
 *
 * Retcodes:	0 - success
 * 		1 - wrong arguments (expects exactly one)
 * 		2 - failed to open journal for writing
 * 		3 - failed to actually write message to journal
 *
 * Part of the testbench for rsyslog.
 *
 * Copyright 2018 Red Hat Inc.
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
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include <systemd/sd-journal.h>
#include <systemd/sd-daemon.h>

int main(int argc, char *argv[])
{
	if(argc != 2) {
		fprintf(stderr, "usage: journal_print \"message\"\n");
		exit(1);
	}

	/* First, we need to determine whether journal is running at all */
	int fd;
	FILE *log;
	fd = sd_journal_stream_fd("imjournal_test", LOG_WARNING, 0);
	if (fd < 0) {
		fprintf(stderr, "Failed to create journal fd: %s\n", strerror(-fd));
		exit(2);
	}
	log = fdopen(fd, "w");
	if (!log) {
		fprintf(stderr, "Failed to create file object: %m\n");
		close(fd);
		exit(2);
	}

	/* Now we can try inserting something */
	if (fprintf(log, "%s", argv[1]) <= 0) {
		fprintf(stderr, "Failed to write to journal log: %m\n");
		close(fd);
		exit(3);
	}

	return(0);
}
