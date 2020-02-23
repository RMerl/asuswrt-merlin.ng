/***********************************************************************
 *
 *  Copyright (c) 2013  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2013:proprietary:standard
 *
 *  This program is the proprietary software of Broadcom and/or its
 *  licensors, and may only be used, duplicated, modified or distributed pursuant
 *  to the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied), right
 *  to use, or waiver of any kind with respect to the Software, and Broadcom
 *  expressly reserves all rights in and to the Software and all intellectual
 *  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 *  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 *  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1. This program, including its structure, sequence and organization,
 *     constitutes the valuable trade secrets of Broadcom, and you shall use
 *     all reasonable efforts to protect the confidentiality thereof, and to
 *     use this information only in connection with your use of Broadcom
 *     integrated circuit products.
 *
 *  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
 *     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
 *     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
 *     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
 *     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
 *     PERFORMANCE OF THE SOFTWARE.
 *
 *  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
 *     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
 *     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 *     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
 *     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
 *     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
 *     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
 *     LIMITED REMEDY.
 * :>
 *
 * $Change: 111969 $
 ***********************************************************************/

/*-
 * interface glue to ieee1905 daemon:
 *
 * open command (creates socket -- perhaps multiple times)
 * send message command
 * wait for up to a second to see if there is a response [optional]
 * receive response
 * close command (destroys socket)
-*/

#ifndef _I5API_C_
#define _I5API_C_

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <poll.h>
#include <errno.h>
#include <fcntl.h>
#include "i5api.h"
#include "ieee1905_glue.h"

/* create session to ieee1905 daemon -- return socket */
int i5apiOpen(void) {
  struct sockaddr_in sa = { 0 };
  int                rc;
  int                sd;
  int                flags;
  unsigned short port = I5_GLUE_CONTROL_SOCK_PORT;

  if (i5_controller_port == 1) {
    port = I5_GLUE_CONTROL_SOCK_CONTROLLER_PORT;
  }

  sd = socket(AF_INET, SOCK_STREAM, 0);
  if (sd == -1) {
    fprintf(stderr, "socket() error, %s\n", strerror(errno));
    return -1;
  }

  sa.sin_family      = AF_INET;
  sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  rc = bind(sd, (struct sockaddr *)&sa, sizeof(struct sockaddr_in));
  if(rc == -1) {
    fprintf(stderr, "bind() failed, errno=%d\n", errno);
    close(sd);
    return -1;
  }

  /* Connect to ieee1905 daemon */
  sa.sin_family      = AF_INET;
  sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  sa.sin_port        = htons(port);
  rc = connect(sd, (struct sockaddr *)&sa, sizeof(struct sockaddr_in));
  if (rc == -1) {
    if (errno == EACCES)
      fprintf(stderr, "IEEE server socket not present\n");
    else
      fprintf(stderr, "connect to port %d failed, errno=%d\n", port, errno);
    close(sd);
    return -1;
  }

  flags = fcntl(sd, F_GETFL, 0);
  if(flags < 0) {
    printf("cannot retrieve socket flags. error=%s", strerror(errno));
  }
  if ( fcntl(sd, F_SETFL, flags | O_NONBLOCK) < 0 ) {
    printf("cannot set socket to non-blocking. error=%s", strerror(errno));
  }

  return sd;
}

/* close session to ieee1905 daemon */
int i5apiClose(int sd) {
  return close(sd);
}

/* prepend header to user cmd/data and send message -- return amount sent */
int i5apiSendMessage(int sd, int cmd, void *data, size_t datalen) {
  struct msghdr msg = { 0 };
  struct iovec iov[2];
  t_I5_API_MSG hdr;
  int rc;

  hdr.cmd = cmd;
  hdr.len = sizeof hdr + datalen;
  iov[0].iov_base = &hdr;
  iov[0].iov_len = sizeof hdr;
  iov[1].iov_base = data;
  iov[1].iov_len = datalen;
  msg.msg_iov = iov;
  msg.msg_iovlen = sizeof iov / sizeof *iov;
  /* just use writev(sd, iov, sizeof iov / sizeof *iov)? */
  rc = sendmsg(sd, &msg, MSG_NOSIGNAL);
  if (rc == -1) {
    fprintf(stderr, "send to port %d failed, errno=%d\n", I5_GLUE_CONTROL_SOCK_PORT, errno);
    return -1;
  }
  return rc;
}

/* wait until response ready -- return -1 on error */
int i5apiWait(int sd, int waitsec) {
  const unsigned ms = 1000;
  struct pollfd fds[1];

  fds[0].fd = sd;
  fds[0].events = POLLIN;
  fds[0].revents = 0;
  switch (poll(fds, sizeof fds / sizeof *fds, waitsec * ms)) {
  case -1: /*error*/
    fprintf(stderr, "poll failed, errno=%d\n", errno);
    return -1;
  case 0: /*timeout*/
    return -1;
  default:
    return 0;
  }
}

/* receive response -- returns -1 on error, else payload length */
/* -- blocking receive of response */
/* -- reads header, then payload with size from header */
int i5apiRecvResponse(int sd, void **data, size_t datalen) {
  t_I5_API_MSG hdr;
  char *body;
  int rc;

  /* receive response header */
  rc = recv(sd, &hdr, sizeof hdr, 0);
  if (rc == -1) {
    fprintf(stderr, "i5apiRecvResponse: recv failed, errno=%d\n", errno);
    return -1;
  } else if (rc < sizeof hdr) {
    fprintf(stderr, "i5apiRecvResponse: recv failed, length=%d\n", rc);
    return -1;
  }

  if (hdr.len > 65536) {
    fprintf(stderr, "Maximum rx size 65536 bytes exceeded, hdr.len = %d cmd = %d\n", hdr.len, hdr.cmd);
    /* Flush recv queue */
    while (rc > 0) {
      rc = recv(sd, *data, datalen, 0);
    }
    errno = E2BIG;
    return -1;
  }

  /* allocate buffer if not provided or too small */
  hdr.len -= sizeof hdr;
  if (hdr.len > datalen) {
    if (*data == 0) {
      datalen = hdr.len;
      body = malloc(datalen);
      if (body == 0) {
        fprintf(stderr, "malloc failed, errno=%d\n", errno);
        return -1;
      }
      *data = body;
    }
    else {
      /* buffer provided but too small -- discard and return fail */
      while (hdr.len) {
        rc = recv(sd, *data, datalen, 0);
        if (rc == -1)
          break;
        hdr.len -= rc;
      }
      errno = E2BIG;
      return -1;
    }
  }
  else {
    datalen = hdr.len;
    body = *data;
  }

  /* receive response payload */
  /* Note: if select() was done for header it doesn't
   * mean that the recv() for the payload won't block
   */
  while (hdr.len) {
    rc = recv(sd, body, hdr.len, 0);
    if (rc == -1) {
      fprintf(stderr, "recv failed, errno=%d\n", errno);
      return -1;
    } else if (rc == 0) {
      break; /* eof/nonblocking? */
    }
    hdr.len -= rc;
    body += rc;
  }
  return datalen;
}

/* perform transaction -- return -1 on error, else length of reply */
/* -- open socket
 * -- send message
 * -- allow up to one second for response
 * -- read response (allocating if necessary)
 * -- close socket
 */
int i5apiTransaction(int cmd, void *reqdata, size_t reqlen, void **repdata, size_t replen) {
  int sd;
  int rc;

  sd = i5apiOpen();
  if (sd == -1) {
    return sd;
  }

  rc = i5apiSendMessage(sd, cmd, reqdata, reqlen);
  if (rc == -1) {
    i5apiClose(sd);
    return rc;
  }

  rc = i5apiWait(sd, 1);
  if (rc == -1) {
    i5apiClose(sd);
    return rc;
  }

  rc = i5apiRecvResponse(sd, repdata, replen);
  if (rc == -1) {
    i5apiClose(sd);
    return rc;
  }

  i5apiClose(sd);
  return rc;
}

#endif // endif
