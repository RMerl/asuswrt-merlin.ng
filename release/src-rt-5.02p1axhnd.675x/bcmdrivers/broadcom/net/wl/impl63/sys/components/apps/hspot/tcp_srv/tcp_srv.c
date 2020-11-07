/*
 * TCP socket for running sigma.
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id:$
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include "trace.h"
#include "tcp_srv.h"

#define MAX_TCP_CLIENT_NUM 100
#define LISTEN_BACKLOG_VAL 10
typedef struct {
	int port;
	/* Two socket address structures - One for the server itself and the other for client */
	struct sockaddr_in serv_addr;
	struct sockaddr_in client_addr;
	int listenFd;
	int clientFds[MAX_TCP_CLIENT_NUM];
	int numClients;
	int last_fd;
} tcpSrvT;

static tcpSrvT gTcpSrv;
static int server_created = 0;

static void *tcpRxHandle = NULL;
static tcpHandlerT tcpRxHandler = NULL;
/* create a tcp socket listener and bind it */
/* return the file descriptor of the listener */

int tcpSrvCreate(int port)
{
	int listenFd = -1;
	struct sockaddr_in *serv_addr = &gTcpSrv.serv_addr;
	int optval;

	bzero(&gTcpSrv, sizeof(gTcpSrv));

	listenFd = gTcpSrv.listenFd = socket(AF_INET, SOCK_STREAM, 0);

	if (listenFd < 0) {
		printf("TCP listen socket failed errno=%d\n", errno);
		return -1;
	}

	optval = 1;
	if (setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
		printf("setsockopt failed: %s\n", strerror(errno));
	}

	optval = fcntl(listenFd, F_GETFL);
	if (optval < 0) {
		printf("fcntl F_GETFL failed: %s\n", strerror(errno));
	}
	optval = (optval | O_NONBLOCK);
	if (fcntl(listenFd, F_SETFL, optval) < 0) {
		printf("fcntl set O_NONBLOCK failed: %s\n", strerror(errno));
	}

	printf("TCP listen socket %d created\n", listenFd);
	server_created = 1;
	/* Fill server's address family */
	serv_addr->sin_family = AF_INET;

	/* Server should allow connections from any ip address */
	serv_addr->sin_addr.s_addr = INADDR_ANY;
	serv_addr->sin_port = htons(port);

	if (bind(listenFd, (struct sockaddr *)serv_addr, sizeof(struct sockaddr_in)) < 0) {
		printf("Failed to bind: %s\n", strerror(errno));
		close(listenFd);
		return -1;
	}

	if (listen(listenFd, LISTEN_BACKLOG_VAL) < 0) {
		printf("Failed to listen: %s\n", strerror(errno));
		close(listenFd);
		return -1;
	}

	gTcpSrv.last_fd = listenFd;
	return listenFd;
}

/* destroy and cleanup */
void tcpSrvDestroy(void)
{
	int i;

	printf("TCP destroy\n");

	shutdown(gTcpSrv.listenFd, SHUT_RDWR);
	for (i = 0; i < gTcpSrv.numClients; i++) {
		if (gTcpSrv.clientFds[i] > 0)
			shutdown(gTcpSrv.clientFds[i], SHUT_RDWR);
	}
	close(gTcpSrv.listenFd);
	for (i = 0; i < gTcpSrv.numClients; i++) {
		if (gTcpSrv.clientFds[i] > 0)
			close(gTcpSrv.clientFds[i]);
	}
	sleep(1);
}

/* add all sockets : listenner + clients and return the highest fd */
int tcpSetSelectFds(fd_set *rfds)
{
	int i;
	int numClients = 0;
	if (! server_created)
		return -1;

	gTcpSrv.last_fd = gTcpSrv.listenFd;
	FD_SET(gTcpSrv.listenFd, rfds);
	for (i = 0; i < gTcpSrv.numClients; i++) {
		if (gTcpSrv.clientFds[i] > 0) {
			FD_SET(gTcpSrv.clientFds[i], rfds);
			numClients = i + 1;
			if (gTcpSrv.last_fd < gTcpSrv.clientFds[i])
				gTcpSrv.last_fd = gTcpSrv.clientFds[i];
		}
	}
	gTcpSrv.numClients = numClients;
	return gTcpSrv.last_fd;
}

/* buffer to use for exchanging  RX and TX data to be respectively
 * received from client to application or sent by application to client
 */
static char TcpKeyInput[2000];

/* passed automatically to the callback
 * can be use for the answer . Alternatively, the application can use TcpSendBuffer directly.
 */
static char TcpSendBuf[2000];

/* In a lock step mode, don't read a client socket if a previous RX hasn't been processed yet */
static int TcpReadLocked = 0;

static int data_received = 0;

void tcpSetLockRead(int lock_on)
{
	TcpReadLocked = lock_on;
}

static void TcpCallHandlers(char *rx, char *tx)
{
	if (tcpRxHandler) {
		tx[0] = 0;
		tcpRxHandler(tcpRxHandle, rx, tx);
		if (strlen(tx)) {
			TcpSendBuffer(rx, tx);
		}
	}
}

/*  */
int tcpProcessSelect(fd_set *rfds)
{
	int i;
	static char buffer[1600];

	if (!server_created) {
		return -1;
	}
	if (FD_ISSET(gTcpSrv.listenFd, rfds)) {
		size_t size;
		int clientFd;
		size = sizeof(struct sockaddr);
		clientFd = accept(gTcpSrv.listenFd,
			(struct sockaddr *)&gTcpSrv.client_addr, &size);

		if (clientFd >= 0) {
			char *ip;
			int port;
			ip = inet_ntoa(gTcpSrv.client_addr.sin_addr);
			port = ntohs(gTcpSrv.client_addr.sin_port);
			printf("connected to TCP client %s %d\n", ip, port);
			gTcpSrv.last_fd = gTcpSrv.last_fd > clientFd ? gTcpSrv.last_fd:clientFd;
			for (i = 0; i < MAX_TCP_CLIENT_NUM; i++) {
				if (gTcpSrv.clientFds[i] == 0) {
					gTcpSrv.clientFds[i] = clientFd;
					break;
				}
			}
			if (i < MAX_TCP_CLIENT_NUM) {
				if (gTcpSrv.numClients <= i)
					gTcpSrv.numClients = i + 1;
			} else {
				printf("no room left for new client %d\n", clientFd);
				close(clientFd);
			}
		} else {
			printf("accept wrong socket number %d\n", clientFd);
		}
	}

	for (i = 0; i < gTcpSrv.numClients; i++) {
		if ((gTcpSrv.clientFds[i] > 0) && FD_ISSET(gTcpSrv.clientFds[i], rfds)) {
			int count;
			int *ptr_int = (int *)TcpKeyInput;
			printf("receiving from client socket %d\n", gTcpSrv.clientFds[i]);
			count = read(gTcpSrv.clientFds[i], buffer, sizeof(buffer));
			if (count == 0) {
				printf("read zero bytes: %d\n", i);
				close(gTcpSrv.clientFds[i]);
				gTcpSrv.clientFds[i] = 0;
				continue;
			}
			else if (count < 2) {
				continue;
			}
			else {
				/* remove CR and LF */
				buffer[count-2] = 0;
				printf("read %d char : %s\n", strlen(buffer), buffer);
			}

			/* XXX this should be dependent of the nature ot the previous input
			 * a better protocol should provide a code at the begining indicating if
			 * the command is blocking or not  or we should provide an API to
			 * the application for unlocking
			 */

			if (TcpReadLocked) {
				/* ignore the new input */
				printf("TcpReadLocked - dropping TCP input\n");
				continue;
			}
			/* TcpReadLocked must be set before data_received else TcpSendBuffer
			 * will clear TcpReadLocked before it is set
			 */
			TcpReadLocked = 1;
			/* store fd */
			*ptr_int = gTcpSrv.clientFds[i];
			/* copy data */
			strcpy(TcpKeyInput+sizeof(int), buffer);
			data_received = 1;
			TcpCallHandlers(TcpKeyInput+sizeof(int), TcpSendBuf);
		}
	}
	return gTcpSrv.last_fd;
}

char * TcpGetClientBuffer()
{
	/* printf("data received %d\n", data_received); */
	if (data_received) {
		data_received = 0;
		return TcpKeyInput+sizeof(int);
	}
	return 0;
}

/* the input buffer is used to be able to refer to its content
 * and to the socket it was received on
 */
void TcpSendBuffer(char *input, char *buf)
{
	int sock;
	/* retrieve socket */
	sock = *((int *)(input - sizeof(int)));
	sprintf(buf, "%s\n", buf);
	printf("sending %s on socket  %d\n", buf, sock);
	/*  for now,  unlock receiving */
	/* the app could use a code to indicate that it would like to send
	 * more before strating receiving again
	 */
	/* TcpReadLocked must be clear before write() else an immediate TCP read will be ignored */
	TcpReadLocked = 0;
	write(sock, buf, strlen(buf));
}

void tcpSubscribeTcpHandler(void *handle, tcpHandlerT tcpReceiveHandler)
{
	tcpRxHandle = handle;
	tcpRxHandler = tcpReceiveHandler;
}
