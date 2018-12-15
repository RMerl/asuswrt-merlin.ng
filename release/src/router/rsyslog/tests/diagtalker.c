/* A yet very simple tool to talk to imdiag (this replaces the
 * previous Java implementation in order to get fewer dependencies).
 *
 * Copyright 2010-2018 Rainer Gerhards and Adiscon GmbH.
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
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#if defined(__FreeBSD__)
#include <netinet/in.h>
#endif

static char *targetIP = "127.0.0.1";
static int targetPort = 13500;


/* open a single tcp connection
 */
int openConn(int *fd)
{
	int sock;
	struct sockaddr_in addr;
	int port;
	int retries = 0;

	if((sock=socket(AF_INET, SOCK_STREAM, 0))==-1) {
		perror("socket()");
		exit(1);
	}

	port = targetPort;
	memset((char *) &addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	if(inet_aton(targetIP, &addr.sin_addr)==0) {
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}
	while(1) { /* loop broken inside */
		if(connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
			break;
		} else {
			if(retries++ == 50) {
				perror("connect()");
				fprintf(stderr, "[%d] connect() failed\n", port);
				exit(1);
			} else {
				fprintf(stderr, "[%d] connect failed, retrying...\n", port);
				usleep(100000); /* ms = 1000 us! */
			}
		}
	}
	if(retries > 0) {
		fprintf(stderr, "[%d] connection established.\n", port);
	}

	*fd = sock;
	return 0;
}


/* send a string
 */
static void
sendCmd(int fd, char *buf, int len)
{
	int lenSend;

	lenSend = send(fd, buf, len, 0);
	if(lenSend != len) {
		perror("sending string");
		exit(1);
	}
}


/* wait for a response from remote system
 */
static void
waitRsp(int fd, char *buf, int len)
{
	int ret;

	ret = recv(fd, buf, len - 1, 0);
	if(ret < 0) {
		perror("receiving response");
		exit(1);
	}
	/* we assume the message was complete, it may be better to wait
	 * for a LF...
	 */
	buf[ret] = '\0';
}


/* do the actual processing
 */
static void
doProcessing()
{
	int fd;
	int len;
	char line[2048];

	openConn(&fd);
	while(!feof(stdin)) {
		if(fgets(line, sizeof(line) - 1, stdin) == NULL)
			break;
		len = strlen(line);
		sendCmd(fd, line, len);
		waitRsp(fd, line, sizeof(line));
		printf("imdiag[%d]: %s", targetPort, line);
		if (strstr(line, "imdiag::error") != NULL) {
			exit(1);
		}
	}
}


/* Run the test.
 * rgerhards, 2009-04-03
 */
int main(int argc, char *argv[])
{
	int ret = 0;
	int opt;

	while((opt = getopt(argc, argv, "t:p:")) != -1) {
		switch (opt) {
		case 't':	targetIP = optarg;
				break;
		case 'p':	targetPort = atoi(optarg);
				break;
		default:	printf("invalid option '%c' or value missing - terminating...\n", opt);
				exit (1);
				break;
		}
	}

	doProcessing();

	exit(ret);
}
