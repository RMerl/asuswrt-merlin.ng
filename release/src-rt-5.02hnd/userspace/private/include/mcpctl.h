/*
* <:copyright-BRCM:2011:proprietary:standard
* 
*    Copyright (c) 2011 Broadcom 
*    All Rights Reserved
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
:>
*/

/***************************************************************************
 * File Name  : mcpctl.h
 *
 * Description: Definition of control message sent from mcpctl to mcpd
 *
 ***************************************************************************/
#ifndef __MCPCTL_H__
#define __MCPCTL_H__

#define MCPD_CTL_MAX_DATA 16

typedef int (t_MCPD_CTL_CMD_FUNC)(void *mcpdCtlCmd, int argc, char** argv);

/* Command message sent from mcpctl */
typedef enum t_mcpd_ctl_cmd_name
{
    MCPD_CTL_CMD_NONE = 0,
    MCPD_CTL_CMD_OBJINFO,
    MCPD_CTL_CMD_MEMINFO,
    MCPD_CTL_CMD_ALLINFO,
    MCPD_CTL_CMD_MCGRPMODE,
    MCPD_CTL_CMD_ADMISSION,
    MCPD_CTL_CMD_CONFIGINFO,
    MCPD_CTL_CMD_RELOAD,
} t_MCPD_CTL_CMD_NAME;

typedef struct t_mcpd_ctl_cmd
{
  char                *cmdstr;
  char                *description;
  t_MCPD_CTL_CMD_NAME  cmd;
  t_MCPD_CTL_CMD_FUNC *func;
  unsigned int         nargs;
} t_MCPD_CTL_CMD;

typedef enum t_mcpd_ctl_grpMode_types
{
    MCPD_CTL_CMD_MCGRPMODE_IANA = 0,
    MCPD_CTL_CMD_MCGRPMODE_FIRST_IN,
} t_MCPD_CTL_CMD_GRPMODE_TYPES;

typedef struct t_mcpd_ctl_msg
{
    t_MCPD_CTL_CMD_NAME cmd;
    int                 len;
} t_MCPD_CTL_MSG;

typedef struct t_mcpd_ctl_grp_mode
{
    int mode;
} t_MCPD_CTL_GRP_MODE;

typedef struct t_mcpd_ctl_admission
{
    int enable;
} t_MCPD_CTL_ADMISSION;


#define MCPD_CONTROL_SOCK_PORT     47753 /* 4'MC' */

int mcpd_api_open(void) {
  struct sockaddr_in sa = { 0 };
  int                rc;
  int                sd;
  int                flags;

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
  sa.sin_port        = htons( (unsigned short)MCPD_CONTROL_SOCK_PORT);
  rc = connect(sd, (struct sockaddr *)&sa, sizeof(struct sockaddr_in));
  if (rc == -1) {
    if (errno == EACCES)
      fprintf(stderr, "IEEE server socket not present\n");
    else
      fprintf(stderr, "connect to port %d failed, errno=%d\n", MCPD_CONTROL_SOCK_PORT, errno);
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
int mcpd_api_close(int sd) {
  return close(sd);
}

/* prepend header to user cmd/data and send message -- return amount sent */
int mcpd_api_send_message(int sd, int cmd, void *data, size_t datalen) {
  struct msghdr msg = { 0 };
  struct iovec iov[2];
  t_MCPD_CTL_MSG hdr;
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
    fprintf(stderr, "send to port %d failed, errno=%d\n", MCPD_CONTROL_SOCK_PORT, errno);
    return -1;
  }
  return rc;
}

#endif /* __MCPCTL_H__ */
