/***********************************************************************
 *
 *  Copyright (c) 2018  Broadcom
 *  All Rights Reserved
 *
<:label-BRCM:2018:proprietary:standard

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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <errno.h>
#include <unistd.h>

#include "nvram_netlink.h"
#include "nvram_debug.h"

t_NL_HANDLER nl_handler = {
    .sock_nl = INVALID_SOCKET,
    .uni_pid = 0
};

static void valuepair_set(t_WLCSM_NAME_VALUEPAIR *v, const char *name, const char *value,int len)
{
    t_WLCSM_NAME_VALUEPAIR *vp=v;
    /* init to 0 */
    memset((void *)vp,0,_get_valuepair_total_len(name,value,len));
    /* set name first */
    vp->len=strlen(name)+1;
    strcpy(vp->value,name);

    /* move pointer to  */
    vp = _get_valuepair_value(v);
    if(value) {
        vp->len=(len?len:(strlen(value)+1));
        memcpy(&(vp->value),value,vp->len);
    } else
        vp->len=0;
}

t_WLCSM_NAME_VALUEPAIR *get_namevalue_buf(const char *name, const char *value, int len)
{
    t_WLCSM_NAME_VALUEPAIR *buf=NULL;
    int buflen=_get_valuepair_total_len(name,value,len);
    buf=(t_WLCSM_NAME_VALUEPAIR *)malloc(buflen);
    if(buf!=NULL)
        valuepair_set(buf,name,value,len);
    return buf;
}

static int _create_nl_socket(int *sock,int *pid)
{

    struct sockaddr_nl src_addr;
    int retry_count=10;

    *sock = socket(PF_NETLINK, SOCK_RAW, m_NETLINK_PORT);
    if(*sock < 0)
        return 1;
    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = *pid;
    src_addr.nl_groups = 0;

    while(bind(*sock, (struct sockaddr*)&src_addr,
               sizeof(src_addr)) < 0) {
        src_addr.nl_pid= ++(*pid);
        if(--retry_count>0) continue;
        log_error("pid:%d binding netlink socket error!!!", *pid);
        return 1;
    }
    return 0;

}
int netlink_init(void)
{
    if(NL_NOT_INITED) {
        nl_handler.uni_pid=getpid();
        if(_create_nl_socket(&nl_handler.sock_nl,&nl_handler.uni_pid)!=0) {
            log_error("create sock_nl error");
            return 1;
        }
    }
    return 0;
}

int netlink_free(void)
{
    if(!NL_NOT_INITED)
    {
        close(nl_handler.sock_nl);
        nl_handler.sock_nl = INVALID_SOCKET;
    }
    return 0;
}

int netlink_send_mesg(int msg_type, char *buf, int len)
{
    struct sockaddr_nl dest_addr;
    struct nlmsghdr *nlh;
    struct iovec iov;
    struct msghdr msg;
    char *ptr;
    int buf_size;
    int link_socket;
    int ret = 0;

    ret = netlink_init();
    if(ret != 0)
        return 1;

    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0;   /* For Linux Kernel */
    dest_addr.nl_groups = 0; /* unicast */

    buf_size = sizeof(t_WLCSM_MSG_HDR) + len;
    buf_size = NLMSG_SPACE(buf_size);

    nlh = (struct nlmsghdr *)malloc(buf_size);

    if(!nlh)  {

        log_error("!!!!! could not allocate memroy");
        return 1;
    }

    memset(nlh, 0, buf_size);

    /* Fill the netlink message header */
    nlh->nlmsg_len = buf_size;
    nlh->nlmsg_type = 5;
    nlh->nlmsg_flags = 0;
    nlh->nlmsg_seq = 1;
    nlh->nlmsg_pid = nl_handler.uni_pid;

    /* Fill in the netlink message payload */
    /* send registration */
    ptr = NLMSG_DATA(nlh);

    WLCSM_SET_TYPE(ptr, msg_type);
    WLCSM_SET_LEN(ptr, len);
    WLCSM_SET_PID(ptr, 0);

    ptr += sizeof(t_WLCSM_MSG_HDR);

    if(len)
        memcpy(ptr, buf, len);

    memset(&iov, 0, sizeof(struct iovec));
    memset(&msg, 0, sizeof(struct msghdr));
    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = NULL;
    msg.msg_controllen = 0;
    msg.msg_flags = 0;

    link_socket = nl_handler.sock_nl;

    if (IS_VALID_SOCKET(link_socket)) {
        if (sendmsg(link_socket, &msg, 0) < 0) {
            log_error("netlink_send_mesg: socket = %d error %d: %s", link_socket, errno, strerror(errno));
            ret = 1;
        }
    } else
    {
        log_error("pid:%d has no socket setup yet!!!", getpid());
        ret = 1;
    }

    free(nlh);

    return ret;
} /* netlink_send_mesg */

t_WLCSM_MSG_HDR *netlink_recv_mesg(char *to_buf)
{
    struct sockaddr_nl src_addr;
    struct iovec iov;
    struct msghdr msg;
    int len = 0;
    int fromsocket=0;
    struct nlmsghdr *nlh;
    t_WLCSM_MSG_HDR *hdr=NULL;

    fromsocket=nl_handler.sock_nl;

    if(!IS_VALID_SOCKET(fromsocket)) {
        log_error("return NULL from rec, why socke is not initialized?");
        return NULL;
    }

    nlh= (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_NLRCV_BUF_SIZE));
    if(!nlh) {
        log_error("failed to allocate nlmsghdr");
        to_buf[0]='\0';
        return NULL;
    }
    memset(nlh, 0, NLMSG_SPACE(MAX_NLRCV_BUF_SIZE));
    memset(&iov, 0, sizeof(struct iovec));
    memset(&msg, 0, sizeof(struct msghdr));
    iov.iov_base = (void *)nlh;
    iov.iov_len = MAX_NLRCV_BUF_SIZE;
    msg.msg_name = (void *)&src_addr;
    msg.msg_namelen = sizeof(src_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    /* Read message from kernel */
    if((len=recvmsg(fromsocket, &msg, 0)) >= (int)sizeof(t_WLCSM_MSG_HDR)) {
        if (!(msg.msg_flags & MSG_TRUNC)) {
            hdr=(t_WLCSM_MSG_HDR *)NLMSG_DATA(nlh);
            hdr->pid=nlh->nlmsg_pid;
            memcpy(to_buf,hdr,sizeof(t_WLCSM_MSG_HDR)+hdr->len);
            free(nlh);
            return (t_WLCSM_MSG_HDR *)to_buf;
        } else {
            log_error("Message:%d too long.  Ignored.",len);
        }
    }
    free(nlh);
    return NULL;
}
