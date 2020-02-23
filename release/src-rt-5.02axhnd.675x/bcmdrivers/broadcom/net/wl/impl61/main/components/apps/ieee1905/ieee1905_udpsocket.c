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

#if defined(IEEE1905_KERNEL_MODULE_PB_SUPPORT)
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include "ieee1905_socket.h"
#include "ieee1905_security.h"
#include "ieee1905_trace.h"
#include <ieee1905_module.h>

#define I5_TRACE_MODULE i5TraceUdpSocket

#define I5_UDP_SOCKET_MAX_RECV_BUFFER_SIZE 4096

int _i5UdpSocketSendMsg( i5_socket_type *psock, unsigned char *pData, unsigned int len )
{
  struct sockaddr_in sockAddr;
  int                sendLen;

  /* kernel address */
  memset(&sockAddr, 0, sizeof(sockAddr));
  sockAddr.sin_family      = AF_INET;
  sockAddr.sin_addr.s_addr = htonl(0x7f000001);
  sockAddr.sin_port        = htons(I5_UDP_SERVER_PORT);

  sendLen = sendto(psock->sd, pData, len, 0, (struct sockaddr *)&sockAddr, sizeof(sockAddr));
  if (len != sendLen) {
    printf("%s: sendto failed (%d/%d)\n", __FUNCTION__, len, sendLen);
    return -1;
  }

  return 0;
}

static void _i5UdpSocketProcess(i5_socket_type *psock)
{
  unsigned char       buffer[I5_UDP_SOCKET_MAX_RECV_BUFFER_SIZE];
  int                 recvlen;
  struct sockaddr     src;
  struct sockaddr_in *pSrc = (struct sockaddr_in *)&src;
  unsigned int        addrlen = sizeof(struct sockaddr);
  t_I5_UDP_MSG       *pUdpMsg;

  memset(buffer, 0, I5_UDP_SOCKET_MAX_RECV_BUFFER_SIZE);
  recvlen = recvfrom(psock->sd, buffer, I5_UDP_SOCKET_MAX_RECV_BUFFER_SIZE, 0, &src, &addrlen);
  if (recvlen < 0) {
    printf("receive error\n");
    return;
  }

  if ( I5_UDP_SERVER_PORT != ntohs(pSrc->sin_port) ) {
    return;
  }

  pUdpMsg = (t_I5_UDP_MSG *)&buffer[0];
  switch (pUdpMsg->cmd) {
    case I5_UDP_CMD_PUSH_BUTTON_NOTIFY:
      i5Trace("received I5_UDP_CMD_PUSH_BUTTON_NOTIFY\n");
      i5SecurityProcessLocalPushButtonEvent();
      break;

    default:
      printf("unknown message (%d)\n", pUdpMsg->cmd);
      break;
  }

}

void i5UdpSocketSendPushButtonRegistration( unsigned int bRegister )
{
  t_i5_UDP_PUSH_BUTTON_REGISTER_MSG  msg;
  i5_socket_type                    *psock;

  i5Trace("\n");

  psock = i5SocketFindUdpSocketByProcess(_i5UdpSocketProcess);
  if (NULL == psock)
  {
    printf("IEEE1905 udp socket is not available!\n");
    return;
  }

  msg.udpMsg.cmd = I5_UDP_CMD_PUSH_BUTTON_REGISTER;
  msg.udpMsg.len = sizeof(t_i5_UDP_PUSH_BUTTON_REGISTER_MSG) - sizeof(t_I5_UDP_MSG);
  msg.reg        = bRegister;
  _i5UdpSocketSendMsg(psock, (unsigned char *)&msg, sizeof(t_i5_UDP_PUSH_BUTTON_REGISTER_MSG));

}

void i5UdpSocketTriggerWirelessPushButtonEvent( int forced )
{
  t_i5_UDP_PUSH_BUTTON_HANDLE_MSG msg;
  i5_socket_type                  *psock;

  i5Trace("\n");

  psock = i5SocketFindUdpSocketByProcess(_i5UdpSocketProcess);
  if (NULL == psock)
  {
    printf("IEEE1905 udp socket is not available!\n");
    return;
  }

  msg.udpMsg.cmd = I5_UDP_CMD_SES_BUTTON_TRIGGER;
  msg.udpMsg.len = sizeof(t_i5_UDP_PUSH_BUTTON_HANDLE_MSG) - sizeof(t_I5_UDP_MSG);;
  msg.forced = forced;
  _i5UdpSocketSendMsg(psock, (unsigned char *)&msg, sizeof(t_i5_UDP_PUSH_BUTTON_HANDLE_MSG));

}

void i5UdpSocketInit( void )
{
  i5_socket_type    *psock;
  int                sd;
  t_I5_UDP_MSG       udpMsg;

  sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if ( sd < 0 ) {
    printf("i5_udp_scoket_init: failed to create udp socket\n");
    return;
  }

  psock = i5SocketNew(sd, i5_socket_type_udp, _i5UdpSocketProcess);
  if ( NULL == psock ) {
    printf("i5UdpSocketInit: failed to create i5_socket_type_udp\n");
    close(sd);
    return;
  }

  /* userspace address */
  psock->u.sinl.sa.sin_family      = AF_INET;
  psock->u.sinl.sa.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(psock->sd, (struct sockaddr *)&(psock->u.sinl.sa), sizeof(struct sockaddr_in)) < 0) {
    printf("i5UdpSocketInit: failed to bind to udp receive socket \n");
    perror("bind");
    i5SocketClose(psock);
    return;
  }

  udpMsg.cmd = I5_UDP_CMD_CLIENT_REGISTER;
  udpMsg.len = 0;
  _i5UdpSocketSendMsg(psock, (unsigned char *)&udpMsg, sizeof(t_I5_UDP_MSG));

  return;
}

void i5UdpSocketDeinit( void )
{
  i5UdpSocketSendPushButtonRegistration( 0 );

  return;
}

#endif // endif
