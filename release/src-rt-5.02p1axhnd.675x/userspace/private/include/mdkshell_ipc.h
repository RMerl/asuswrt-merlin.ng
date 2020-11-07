/***********************************************************************
 *
 *  Copyright (c) 2007  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2012:proprietary:standard

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>
 *
 ************************************************************************/

#ifndef _MDKSHELL_IPC_H
#define _MDKSHELL_IPC_H

/******************************************************************************
 *
 * Linux User mode, mdkshell/swmdk socket ipc support
 *
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <unistd.h>
#include <inttypes.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <mdkshell_ipc.h>

extern char *mdksh_readline_from_socket(const char *prompt, char *buf, int max);
extern int mdksh_writeline_to_socket(const char *buf);
extern int mdksh_open_listener(char *sockname);
extern int mdksh_accept_conn(int sfd);
extern int mdksh_connect_to_socket(char *sockname);
extern int mdksh_read_from_socket(int conn_sfd, char *buf, int max_len);
extern int mdksh_write_to_socket(int conn_sfd, const char *buf, int len);

#define PROMPT_STR "MDK."
#define MDKSH_SOCK_NAME "/var/mdkshell_sock"
extern int mdksh_sock_conn;

/*
 *  following used by mdkshell pthread. 
 */
inline char *
mdksh_readline_from_socket(const char *prompt, char *buf, int max)
{
    int len;

    if (write(mdksh_sock_conn, prompt, strlen(prompt) + 1) < 0)
    {
        perror( "Error writing prompt to socket\n");
    }
    if ((len = mdksh_read_from_socket(mdksh_sock_conn, buf, max)) <= 0) {
        //printf("%s len %d, Remote may be closed\n", __FUNCTION__, len);
        return NULL;
    } else {
        buf[len-1] = 0;
    }
    return buf;
}
inline int
mdksh_writeline_to_socket(const char *buf)
{
    int len = strlen(buf) + 1;

    return mdksh_write_to_socket(mdksh_sock_conn, buf, len);
}

inline int mdksh_open_listener(char *sockname)
{
    int sfd;
    struct sockaddr_un server;

    if ((sfd = socket(AF_UNIX, SOCK_SEQPACKET, 0)) < 0) {
        perror("opening socket");
        return -1;
    }
    server.sun_family = AF_UNIX;
    strcpy(server.sun_path, sockname);

    if( access(sockname,F_OK) == 0 ){
        unlink(sockname);
    }

    if (bind(sfd, (struct sockaddr *) &server, sizeof(struct sockaddr_un)) < 0) {
        perror("binding socket");
        return -1;
    }
    if (listen(sfd, 5) < 0) {
        perror("socket listen, ");
        return -1;
    }
    return sfd;
}
inline int mdksh_accept_conn(int sfd)
{
    int conn_sfd;
    if ((conn_sfd = accept(sfd, 0, 0)) < 0) {
        perror("accept");
        return -1;
    }
    return conn_sfd;
}
inline int mdksh_connect_to_socket(char *sockname)
{
    int sfd;
    struct sockaddr_un server;

    if ((sfd = socket(AF_UNIX, SOCK_SEQPACKET, 0)) < 0) {
        perror("opening socket");
        return(-1);
    }
    server.sun_family = AF_UNIX;
    strcpy(server.sun_path, sockname);

    if (connect(sfd, (struct sockaddr *) &server, sizeof(struct sockaddr_un)) < 0) {
        close(sfd);
        perror("connecting socket");
        return(-1);
    }
    return sfd;
}
/*
 * following used by mdkshell to collect command output.
 */
inline int mdksh_read_from_socket(int conn_sfd, char *buf, int max_len)
{
    int rval;
    if ((rval = read(conn_sfd, buf, max_len)) < 0) {
        perror("reading message");
    } else if (rval == 0) {
        //printf("Ending connection\n");
    }
    return rval;
}
inline int mdksh_write_to_socket(int conn_sfd, const char *buf, int len)
{
    if (write(conn_sfd, buf, len) < 0) {
        perror("writing on socket");
        return -1;
    }
    return 0;
} 

#endif
