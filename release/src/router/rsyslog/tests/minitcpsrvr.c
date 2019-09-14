/* a very simplistic tcp receiver for the rsyslog testbench.
 *
 * Copyright 2016 Rainer Gerhards and Adiscon GmbH.
 *
 * This file is part of the rsyslog project.
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
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#if defined(__FreeBSD__)
#include <netinet/in.h>
#endif

static void
errout(char *reason)
{
	perror(reason);
	exit(1);
}

static void
usage(void)
{
	fprintf(stderr, "usage: minitcpsrvr -t ip-addr -p port -f outfile\n");
	exit (1);
}

int
main(int argc, char *argv[])
{
	int fds;
	int fdc;
	int fdf = -1;
	struct sockaddr_in srvAddr;
	struct sockaddr_in cliAddr;
	unsigned int srvAddrLen;
	unsigned int cliAddrLen;
	char wrkBuf[4096];
	ssize_t nRead;
	int opt;
	int sleeptime = 0;
	char *targetIP = NULL;
	int targetPort = -1;

	while((opt = getopt(argc, argv, "t:p:f:s:")) != -1) {
		switch (opt) {
		case 's':
			sleeptime = atoi(optarg);
			break;
		case 't':
			targetIP = optarg;
			break;
		case 'p':
			targetPort = atoi(optarg);
			break;
		case 'f':
			if(!strcmp(optarg, "-")) {
				fdf = 1;
			} else {
				fdf = open(optarg, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR|S_IWUSR);
				if(fdf == -1) errout(argv[3]);
			}
			break;
		default:
			fprintf(stderr, "invalid option '%c' or value missing - terminating...\n", opt);
			usage();
			break;
		}
	}

	if(targetIP == NULL) {
		fprintf(stderr, "-t parameter missing -- terminating\n");
		usage();
	}
	if(targetPort == -1) {
		fprintf(stderr, "-p parameter missing -- terminating\n");
		usage();
	}
	if(fdf == -1) {
		fprintf(stderr, "-f parameter missing -- terminating\n");
		usage();
	}

	if(sleeptime) {
		printf("minitcpsrv: deliberate sleep of %d seconds\n", sleeptime);
		sleep(sleeptime);
		printf("minitcpsrv: end sleep\n");
	}

	fds = socket(AF_INET, SOCK_STREAM, 0);
	srvAddr.sin_family = AF_INET;
	srvAddr.sin_addr.s_addr = inet_addr(targetIP);
	srvAddr.sin_port = htons(targetPort);
	srvAddrLen = sizeof(srvAddr);
	if(bind(fds, (struct sockaddr *)&srvAddr, srvAddrLen) != 0)
		errout("bind");
	if(listen(fds, 20) != 0) errout("listen");
	cliAddrLen = sizeof(cliAddr);

	fdc = accept(fds, (struct sockaddr *)&cliAddr, &cliAddrLen);
	while(1) {
		nRead = read(fdc, wrkBuf, sizeof(wrkBuf));
		if(nRead == 0) break;
		if(write(fdf, wrkBuf, nRead) != nRead)
			errout("write");
	}
	/* let the OS do the cleanup */
	return 0;
}
