/* receives messages from a specified unix sockets and writes
 * output to specfied file.
 *
 * Command line options:
 * -s name of socket (required)
 * -o name of output file (stdout if not given)
 * -l add newline after each message received (default: do not add anything)
 * -t timeout in seconds (default 60)
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
#include <string.h>
#include <unistd.h>
#include <signal.h>
#if defined(_AIX)
	#include <sys/types.h>
	#include  <unistd.h>
	#include <sys/socket.h>
	#include <sys/socketvar.h>
#else
	#include <getopt.h>
#endif
#include <sys/un.h>
#include <netdb.h>
#include <poll.h>
#include <errno.h>
#if defined(__FreeBSD__)
#include <sys/socket.h>
#endif

#define DFLT_TIMEOUT 60

char *sockName = NULL;
int sock;
int addNL = 0;


/* called to clean up on exit
 */
void
cleanup(void)
{
	unlink(sockName);
	close(sock);
}


void
doTerm(int __attribute__((unused)) signum)
{
	exit(1);
}


void
usage(void)
{
	fprintf(stderr, "usage: uxsockrcvr -s /socket/name -o /output/file -l\n"
			"-l adds newline after each message received\n"
			"-s MUST be specified\n"
			"if -o ist not specified, stdout is used\n");
	exit(1);
}


int
main(int argc, char *argv[])
{
	int opt;
	int rlen;
	int timeout = DFLT_TIMEOUT;
	FILE *fp = stdout;
	unsigned char data[128*1024];
	struct  sockaddr_un addr; /* address of server */
	struct  sockaddr from;
	socklen_t fromlen;
	struct pollfd fds[1];

	if(argc < 2) {
		fprintf(stderr, "error: too few arguments!\n");
		usage();
	}

	while((opt = getopt(argc, argv, "s:o:lt:")) != EOF) {
		switch((char)opt) {
		case 'l':
			addNL = 1;
			break;
		case 's':
			sockName = optarg;
			break;
		case 'o':
			if((fp = fopen(optarg, "w")) == NULL) {
				perror(optarg);
				exit(1);
			}
			break;
		case 't':
			timeout = atoi(optarg);
			break;
		default:usage();
		}
	}

	timeout = timeout * 1000;

	if(sockName == NULL) {
		fprintf(stderr, "error: -s /socket/name must be specified!\n");
		exit(1);
	}

	if(signal(SIGTERM, doTerm) == SIG_ERR) {
		perror("signal(SIGTERM, ...)");
		exit(1);
	}
	if(signal(SIGINT, doTerm) == SIG_ERR) {
		perror("signal(SIGINT, ...)");
		exit(1);
	}

	/*      Create a UNIX datagram socket for server        */
	if ((sock = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
		perror("server: socket");
		exit(1);
	}

	atexit(cleanup);

	/*      Set up address structure for server socket      */
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, sockName);

	if (bind(sock, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
		close(sock);
		perror("server: bind");
		exit(1);
	}

	fds[0].fd = sock;
	fds[0].events = POLLIN;

	/* we now run in an endless loop. We do not check who sends us
	 * data. This should be no problem for our testbench use.
	 */

	while(1) {
		fromlen = sizeof(from);
		rlen = poll(fds, 1, timeout);
		if(rlen == -1) {
			perror("uxsockrcvr : poll\n");
			exit(1);
		} else if(rlen == 0) {
			fprintf(stderr, "Socket timed out - nothing to receive\n");
			exit(1);
		} else {
			rlen = recvfrom(sock, data, 2000, 0, &from, &fromlen);
			if(rlen == -1) {
				perror("uxsockrcvr : recv\n");
				exit(1);
			} else {
				fwrite(data, 1, rlen, fp);
				if(addNL)
					fputc('\n', fp);
			}
		}
	}

	return 0;
}
