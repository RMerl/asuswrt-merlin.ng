/*************************************************
* <:copyright-BRCM:2013:proprietary:standard
*
*    Copyright (c) 2013 Broadcom
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
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <asm/types.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/in.h>
#include <linux/if.h>
#include <signal.h>
#include <errno.h>
#include "wlcsm_lib_netlink.h"
#include "wlcsm_lib_nvram.h"

#include  <sys/prctl.h>

/** 	@file	 wlcsm_lib_netlink.c
 *	@brief	 wlcsm netlink and wlmgnr related functions
 *
 */
static unsigned int   g_max_pid=40000;


typedef struct t_wlcsm_agent {
    int            sock_nl;
    int            sock_nl_notify;
    int            mngr_sock_nl;
    char           running;
    unsigned int   vifiBits;
    pthread_t      listening_thread_id;
    int            uni_pid;
    int            mngr_pid;
} t_WLCSM_AGENT;

#define WLCSM_INVALID_SOCKET -1
/* use a designated initializer to initialize a wlcsm_agent */
t_WLCSM_AGENT wlcsm_agent = {
    .sock_nl = WLCSM_INVALID_SOCKET,
    .sock_nl_notify = WLCSM_INVALID_SOCKET,
    .mngr_sock_nl = WLCSM_INVALID_SOCKET,
    .running = 0,
    .vifiBits = 0,
    .listening_thread_id = 0,
    .uni_pid = 0
};


#define MAX_NAME_LEN         (15)
#define m_NETLINK_PORT       (31)

#define GBM_ISMNGR           (1)
#define GBM_NONE_MNGR        (0)


#define m_MNGR_RECV 2
#define m_MULTICAST_RECV 1
#define m_UNICAST_RECV 0

int g_WLCSM_IS_DAEMON= m_NOTAS_DAEMON;

#define WLCSM_IS_VALID_SOCKET(_fd_) ((_fd_) > WLCSM_INVALID_SOCKET)
#define WLCSM_NL_IS_INITED (wlcsm_agent.sock_nl != WLCSM_INVALID_SOCKET)
#define WLCSM_NL_NOT_INITED (wlcsm_agent.sock_nl == WLCSM_INVALID_SOCKET)
#define WLCSM_NL_INITIALIZE(vl)  do { if (WLCSM_NL_NOT_INITED) _wlcsm_init(vl); } while(0)

static void _valuepair_set(t_WLCSM_NAME_VALUEPAIR *v,char *name,char *value,int len)
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


void wlcsm_dump_buf(char *buf, int len)
{
    int i;
    printf("======================== wlcsm recive ===================================\n");
    for(i =0; i < len; i++) {
        printf("%02x", (unsigned char)buf[i]);
        if(!((i+1)%2))
            printf(" ");
        if(!((i+1)%16))
            printf("\n");
    }
    printf("\n");
    printf("=======================END====================================\n");
}



static void  _handle_notification (t_WLCSM_MSG_HDR *hdr )
{

    WLCSM_TRACE(WLCSM_TRACE_LOG,"---:%s:%d  msg hdr type:%d, len:%d \r\n",__FUNCTION__,__LINE__, hdr->type,hdr->len );
    char *name,*value;
    switch ( hdr->type ) {
    case WLCSM_MSG_NVRAM_SET:
        name=VALUEPAIR_NAME((t_WLCSM_NAME_VALUEPAIR *)(hdr+1));
        value=VALUEPAIR_VALUE((t_WLCSM_NAME_VALUEPAIR *)(hdr+1));
        WLCSM_TRACE(WLCSM_TRACE_LOG,"---:%s:%d  name:%s, value:%s on process:%d\r\n",__FUNCTION__,__LINE__,name,value,getpid() );
        if(name && value) {
            wlcsm_nvram_local_update(name,value);
        }
        break;
    case WLCSM_MSG_NVRAM_UNSET:
        name=VALUEPAIR_NAME((t_WLCSM_NAME_VALUEPAIR *)(hdr+1));
        if(name)
            wlcsm_nvram_local_update(name,NULL);
        break;
    case WLCSM_MSG_NVRAM_COMMIT: {
        WLCSM_TRACE(WLCSM_TRACE_LOG,"---:%s:%d  NVRAM COMMIT NOTIFICATION RECEIVED \r\n",__FUNCTION__,__LINE__ );
        wlcsm_nvram_commit_update();
    }
    break;
    default:
        break;
    }
    return ;
}

static t_WLCSM_MSG_HDR    *_netlink_recv_mesg(int fromNotifier,char *to_buf)
{
    struct sockaddr_nl src_addr;
    struct iovec iov;
    struct msghdr msg;
    int len = 0;
    int fromsocket=0;
    struct nlmsghdr *nlh;
    t_WLCSM_MSG_HDR *hdr=NULL;

    if(fromNotifier==m_MULTICAST_RECV)
        fromsocket=wlcsm_agent.sock_nl_notify;
    else if(fromNotifier==m_MNGR_RECV)
        fromsocket=wlcsm_agent.mngr_sock_nl;
    else
        fromsocket=wlcsm_agent.sock_nl;

    if(!WLCSM_IS_VALID_SOCKET(fromsocket)) {
        fprintf(stderr,"%s:%d  return NULL from rec, why socke is not initialized? \r\n",__FUNCTION__,__LINE__ );
        return NULL;
    }

    nlh= (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_NLRCV_BUF_SIZE));
    if(!nlh) {
        WLCSM_TRACE(WLCSM_TRACE_ERR," RETURN NULLLL? \r\n" );
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
            fprintf(stderr, "Message:%d too long.  Ignored.\n",len);
        }
    }
    free(nlh);
    return NULL;
}


static void *_nl_listening_routine(void *arg)
{
    t_WLCSM_MSG_HDR *hdr;
    int stat=PTHREAD_CANCEL_ENABLE;
    pthread_setcancelstate(stat,&stat);
    wlcsm_agent.running=1;
    static char listening_routine_buff[MAX_NLRCV_BUF_SIZE];
    while(wlcsm_agent.running) {
        hdr=_netlink_recv_mesg(m_MULTICAST_RECV,listening_routine_buff);
        if(hdr && (hdr->pid!=wlcsm_agent.uni_pid)) {
            _handle_notification(hdr);
        }
    }

    pthread_exit((void *)0);
}

static int  _wlcsm_create_nl_socket(int *sock,int *pid,int mlcast_id)
{

    struct sockaddr_nl src_addr;
    int retry_count=10;

    *sock = socket(PF_NETLINK, SOCK_RAW, m_NETLINK_PORT);
    if(*sock < 0)
        return WLCSM_GEN_ERR;
    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = *pid;
    src_addr.nl_groups = mlcast_id;

    while(bind(*sock, (struct sockaddr*)&src_addr,
               sizeof(src_addr)) < 0) {
        src_addr.nl_pid= ++(*pid);
        if(--retry_count>0) continue;
        fprintf(stderr, "%s:%d: pid:%d binding netlink socket error!!!\n",__FUNCTION__,__LINE__,*pid);
        return WLCSM_GEN_ERR;
    }
    return WLCSM_SUCCESS;

}


int _wlcsm_init(int isdaemon)
{
    g_WLCSM_IS_DAEMON=isdaemon;
    if(WLCSM_NL_NOT_INITED) {
        FILE *fp;
        char tempStr[64];
        unsigned int num=0;
        wlcsm_agent.uni_pid=getpid();
        if(_wlcsm_create_nl_socket(&wlcsm_agent.sock_nl,&wlcsm_agent.uni_pid,0)!=WLCSM_SUCCESS) {
            WLCSM_TRACE(WLCSM_TRACE_DBG, "create sock_nl error \n");
            return WLCSM_GEN_ERR;
        }

        fp=fopen("/proc/sys/kernel/pid_max","r");
        if(fp) {
            if((fgets(tempStr,63,fp)!=NULL) && (sscanf(tempStr,"%u",&num)))
                g_max_pid=num+2;
            fclose(fp);
        }
    }


#ifdef WITH_MNGR_CHANNEL

    wlcsm_agent.mngr_pid=wlcsm_agent.uni_pid+g_max_pid;

    if((!WLCSM_IS_VALID_SOCKET(wlcsm_agent.mngr_sock_nl))&&(_wlcsm_create_nl_socket(&wlcsm_agent.mngr_sock_nl,&wlcsm_agent.mngr_pid,0)!=WLCSM_SUCCESS)) {
        WLCSM_TRACE(WLCSM_TRACE_DBG, "create mngr_sock_nl error \n");
        close(wlcsm_agent.sock_nl);
        wlcsm_agent.sock_nl = WLCSM_INVALID_SOCKET;
        return WLCSM_GEN_ERR;
    } else
        WLCSM_TRACE(WLCSM_TRACE_LOG," ____REGistered:%x socket for mngr \r\n",wlcsm_agent.mngr_pid);
#endif


    if(isdaemon==m_AS_DAEMON && !WLCSM_IS_VALID_SOCKET(wlcsm_agent.sock_nl_notify)) {
        int pid=0;
        if(_wlcsm_create_nl_socket(&wlcsm_agent.sock_nl_notify,&pid,1)!=WLCSM_SUCCESS) {
            close(wlcsm_agent.sock_nl);
            close(wlcsm_agent.mngr_sock_nl);
            wlcsm_agent.sock_nl = WLCSM_INVALID_SOCKET;
            wlcsm_agent.mngr_sock_nl = WLCSM_INVALID_SOCKET;
            WLCSM_TRACE(WLCSM_TRACE_DBG, "create sock_nl_mutlicast error \n");
            return WLCSM_GEN_ERR;
        }
        /* create the pthread to listening to the netlink socket */
        if((!wlcsm_agent.listening_thread_id) && (!pthread_create(&(wlcsm_agent.listening_thread_id),NULL,_nl_listening_routine,NULL))) {
            WLCSM_TRACE(WLCSM_TRACE_DBG, "Pthread is created from process:%d     .......... \n",getpid());
        }
    }
    return WLCSM_SUCCESS;
}

int wlcsm_init(void )
{
    return _wlcsm_init(m_AS_DAEMON);
}

int wlcsm_reinit(void) {

    wlcsm_shutdown();
    _wlcsm_init(g_WLCSM_IS_DAEMON);
    return 0;
}

int wlcsm_shutdown(void)
{
    int ret=0;
    wlcsm_agent.running=0;

    WLCSM_TRACE(WLCSM_TRACE_DBG," WLcsm shutdonw is getting called \r\n" );

    if(WLCSM_IS_VALID_SOCKET(wlcsm_agent.sock_nl_notify)) {
        shutdown(wlcsm_agent.sock_nl_notify,SHUT_RDWR);
        close(wlcsm_agent.sock_nl_notify);
        WLCSM_TRACE(WLCSM_TRACE_DBG," close notifiy  \r\n" );
        wlcsm_agent.sock_nl_notify = WLCSM_INVALID_SOCKET;
    }

    if(wlcsm_agent.listening_thread_id) {
        ret=pthread_cancel(wlcsm_agent.listening_thread_id);
        //    wlcsm_agent.listening_thread_id=0;
    }

    if(WLCSM_IS_VALID_SOCKET(wlcsm_agent.sock_nl)) {
        shutdown(wlcsm_agent.sock_nl,SHUT_RDWR);
        WLCSM_TRACE(WLCSM_TRACE_DBG," close notifiy  \r\n" );
        close(wlcsm_agent.sock_nl);
        wlcsm_agent.sock_nl = WLCSM_INVALID_SOCKET;
    }
    if(WLCSM_IS_VALID_SOCKET(wlcsm_agent.mngr_sock_nl)) {
        shutdown(wlcsm_agent.mngr_sock_nl,SHUT_RDWR);
        WLCSM_TRACE(WLCSM_TRACE_DBG," close notifiy  \r\n" );
        close(wlcsm_agent.mngr_sock_nl);
        wlcsm_agent.mngr_sock_nl = WLCSM_INVALID_SOCKET;
    }

    return ret;
}

t_WLCSM_MSG_HDR* wlcsm_unicast_recv_mesg(char *buf)
{
    return  _netlink_recv_mesg(m_UNICAST_RECV,buf);
}

t_WLCSM_MSG_HDR* wlcsm_mngr_recv_mesg(char *buf)
{
    return  _netlink_recv_mesg(m_MNGR_RECV,buf);
}


int _wlcsm_netlink_send_mesg(int msg_type, char *buf, int len,unsigned int to_pid)
{
    struct sockaddr_nl dest_addr;
    struct nlmsghdr *nlh;
    struct iovec iov;
    struct msghdr msg;
    char *ptr;
    int buf_size;
    int link_socket;

    WLCSM_NL_INITIALIZE(g_WLCSM_IS_DAEMON);

    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0;   /* For Linux Kernel */
    dest_addr.nl_groups = 0; /* unicast */

    buf_size = sizeof(t_WLCSM_MSG_HDR) + len;
    buf_size = NLMSG_SPACE(buf_size);

    nlh = (struct nlmsghdr *)malloc(buf_size);

    if(!nlh)  {

        fprintf(stderr,"%s:%d  !!!!! could not allocate memroy \r\n",__FUNCTION__,__LINE__ );
        return WLCSM_GEN_ERR;
    }

    memset(nlh, 0, buf_size);

    /* Fill the netlink message header */
    nlh->nlmsg_len = buf_size;
    nlh->nlmsg_type = 5;
    nlh->nlmsg_flags = 0;
    nlh->nlmsg_seq = 1;
    /* use the parent's first pid number which stored first time it use nvram.work for both NPTL and linux
     * thread, LinuxThread has different PID for thread, especially when process daemoned
     */
    if(to_pid)
        nlh->nlmsg_pid = wlcsm_agent.mngr_pid;
    else
        nlh->nlmsg_pid = wlcsm_agent.uni_pid;

    /* Fill in the netlink message payload */
    /* send registration */
    ptr = NLMSG_DATA(nlh);

    WLCSM_SET_TYPE(ptr, msg_type);
    WLCSM_SET_LEN(ptr, len);
    WLCSM_SET_PID(ptr, to_pid);

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

    if(to_pid)
        link_socket= wlcsm_agent.mngr_sock_nl;
    else
        link_socket=wlcsm_agent.sock_nl;
    if (WLCSM_IS_VALID_SOCKET(link_socket)) {
        if (sendmsg(link_socket, &msg, 0) < 0) {
            WLCSM_TRACE(WLCSM_TRACE_DBG,"wlcsm_netlink_send_mesg: socket = %d error %d: %s\n",
                        link_socket, errno, strerror(errno));
            perror("can't send message \n");
        }
    } else
        WLCSM_TRACE(WLCSM_TRACE_DBG,"---:%s:%d  !!!pid:%d has no socket setup yet!!! \r\n",__FUNCTION__,__LINE__,getpid() );

    free(nlh);

    return WLCSM_SUCCESS;
} /* wlcsm_netlink_send_mesg */

int wlcsm_netlink_send_mesg(int msg_type, char *buf, int len)
{
    return  _wlcsm_netlink_send_mesg(msg_type,buf,len,0);
}


int wlcsm_mngr_run_cmd(unsigned int radio_idx,unsigned int sub_idx,unsigned int source,WLCSM_MNGR_CMD cmd,char *value)
{
    WLCSM_MNGR_CMD_SET_SOURCE(radio_idx,source);
    WLCSM_MNGR_CMD_SET_CMD(radio_idx,cmd);
    return wlcsm_mngr_get(radio_idx,sub_idx,value,value)?0:1;
}


int wlcsm_mngr_resume_restart(void)
{
    char temp[16];
    int radio_idx=0;
    sprintf(temp,"resume_restart");
    WLCSM_MNGR_CMD_SET_CMD(radio_idx,WLCSM_MNGR_CMD_RESUME_RESTART);
    return wlcsm_mngr_get(radio_idx,0,temp,temp)?0:1;
}

int wlcsm_mngr_halt_restart(void)
{
    char temp[16];
    int radio_idx=0;
    sprintf(temp,"halt_restart");
    WLCSM_MNGR_CMD_SET_CMD(radio_idx,WLCSM_MNGR_CMD_HALT_RESTART);
    return wlcsm_mngr_get(radio_idx,0,temp,temp)?0:1;
}

int wlcsm_mngr_restart_all(int savedm,int wait)
{
    char temp[16];
    unsigned int radio_idx=0;
    sprintf(temp,"restartall");
    if(savedm) {
        WLCSM_MNGR_CMD_SET_SAVEDM(radio_idx);
        WLCSM_MNGR_CMD_SET_SOURCE(radio_idx,WLCSM_MNGR_RESTART_NVRAM);
    } else
        WLCSM_MNGR_CMD_SET_SOURCE(radio_idx,WLCSM_MNGR_RESTART_MDM_ALL);

    if(wait)
        WLCSM_MNGR_CMD_SET_WAIT(radio_idx);

    WLCSM_MNGR_CMD_SET_CMD(radio_idx,WLCSM_MNGR_CMD_RESTART);

    return wlcsm_mngr_get(radio_idx,0,temp,temp)?0:1;
}

int wlcsm_mngr_restart(unsigned int radio_idx,unsigned int source,int savedm,int wait)
{
    char temp[16];
    WLCSM_MNGR_CMD_SET_SOURCE(radio_idx,source);
    sprintf(temp,"restart");

    if(savedm)
        WLCSM_MNGR_CMD_SET_SAVEDM(radio_idx);
    if(wait)
        WLCSM_MNGR_CMD_SET_WAIT(radio_idx);

    WLCSM_MNGR_CMD_SET_CMD(radio_idx,WLCSM_MNGR_CMD_RESTART);


    return wlcsm_mngr_get(radio_idx,0,temp,temp)?0:1;
}

void *wlcsm_mngr_cmd(unsigned int radio_idx,unsigned int sub_idx,WLCSM_MNGR_CMD cmd,void *value)
{
    //return wlcsm_mngr_get(radio_idx,WLCSM_MNGR_CMD_SET_CMD(sub_idx,cmd),value,value);
    return wlcsm_mngr_get(WLCSM_MNGR_CMD_SET_CMD(radio_idx,cmd),sub_idx,value,value);
}

char    *wlcsm_mngr_get_dmname(char *varValue)
{
    return wlcsm_mngr_get(0,0,"_dmname_",varValue);
}


char *wlcsm_mngr_safe_get(unsigned int radio_idx,unsigned int sub_idx,char * name,char *varValue)
{

    char *value=wlcsm_mngr_get(radio_idx,sub_idx,name,varValue);
    return value?value:"";
}

char *wlcsm_mngr_get(unsigned int radio_idx,unsigned int sub_idx,char * name, char *value)
{
    int current_pos=0;

    int buflen=_get_valuepair_total_len(name,NULL,0)+sizeof(t_WLCSM_MNGR_VARHDR);
    t_WLCSM_MNGR_VAR  *p_mngr_var=(t_WLCSM_MNGR_VAR *)malloc(buflen);
    t_WLCSM_MSG_HDR *hdr;
    char temp_buff[MAX_NLRCV_BUF_SIZE];

    if(p_mngr_var) {
        p_mngr_var->hdr.radio_idx=radio_idx;
        p_mngr_var->hdr.sub_idx=sub_idx;
        _valuepair_set(&(p_mngr_var->pair),name,NULL,0);

        if(_wlcsm_netlink_send_mesg(WLCSM_MSG_GETWL_VAR,(char *)p_mngr_var,buflen,GBM_ISMNGR)== WLCSM_SUCCESS) {
            do {
                hdr=wlcsm_mngr_recv_mesg(temp_buff);
                WLCSM_TRACE(WLCSM_TRACE_LOG," !!!!!!!!!!!!hdr->len:%d and hdr->type:%d \r\n",hdr->len,hdr->type);
                if(hdr && ((hdr->type==WLCSM_MSG_GETWL_VAR_RESP ||hdr->type==WLCSM_MSG_GETWL_VAR_RESP_DONE) && hdr->len && hdr->pid)) {
                    WLCSM_TRACE(WLCSM_TRACE_LOG," !!!!!!!!!!!!hdr->len:%d:%s \r\n",hdr->len,(char *)(hdr+1));
                    memcpy(&value[current_pos],(char *)(hdr+1),hdr->len);
                    current_pos+=hdr->len;
                } else {
                    if(!hdr)
                        WLCSM_TRACE(WLCSM_TRACE_DBG,"%s:%d  NOT RECEIVED anything \r\n",__FUNCTION__,__LINE__ );
                    else if(!hdr->pid)
                        WLCSM_TRACE(WLCSM_TRACE_DBG,"%s:%d  MNGR Process has not been started yet \r\n",__FUNCTION__,__LINE__ );
                    else
                        WLCSM_TRACE(WLCSM_TRACE_DBG,"%s:%d  getMng var:%s, hdr->pid:%d,hdr->type:%d and radio_idx:%d\r\n",__FUNCTION__,__LINE__,name,hdr->pid,hdr->type,radio_idx );
                    free(p_mngr_var);
                    return NULL;
                }
            } while(hdr->type!=WLCSM_MSG_GETWL_VAR_RESP_DONE);
        }
        free(p_mngr_var);
    } else
        return NULL;
    return value;
}

int wlcsm_mngr_dm_register_event(WLCSM_DM_MNGR_EVENT event,int to_reigster)
{

    char temp[16];
    unsigned int radio_idx=0;
    sprintf(temp,"dmeventreg");
    if(to_reigster)
        WLCSM_MNGR_CMD_SET_SAVEDM(radio_idx);

    WLCSM_MNGR_CMD_SET_CMD(radio_idx,WLCSM_MNGR_CMD_REG_DM_EVENT);

    WLCSM_TRACE(WLCSM_TRACE_DBG," WLCSM_MNGR_CMD_REG_DM_EVENT REQUEST SEND...:%08x	 \r\n",radio_idx );
    return wlcsm_mngr_get(radio_idx,event,temp,temp)?0:1;
}

char *wlcsm_mngr_dm_get(unsigned int radio_idx,unsigned int sub_idx,unsigned int oid,unsigned short offset,char *varValue)
{
    char *name="odVal";
    int buflen=_get_valuepair_total_len(name,NULL,0)+sizeof(t_WLCSM_MNGR_VARHDR);
    t_WLCSM_MSG_HDR *hdr;
    t_WLCSM_MNGR_VAR  *p_mngr_var=(t_WLCSM_MNGR_VAR *)malloc(buflen);
    char temp_buf[MAX_NLRCV_BUF_SIZE];
    char *ret=NULL;
    WLCSM_MNGR_CMD_SET_CMD(radio_idx,WLCSM_MNGR_CMD_GET_DM_VAR);
    if(p_mngr_var) {
        p_mngr_var->hdr.radio_idx=radio_idx;
        p_mngr_var->hdr.sub_idx=sub_idx;
        p_mngr_var->hdr.dm_oid=oid;
        p_mngr_var->hdr.offset=offset;
        //p_mngr_var->hdr.extra_idx[0]=offset;
        //p_mngr_var->hdr.extra_idx[1]=0;
        _valuepair_set(&(p_mngr_var->pair),name,NULL,0);
        if((_wlcsm_netlink_send_mesg(WLCSM_MSG_GETWL_VAR,(char *)p_mngr_var,buflen,GBM_ISMNGR)== WLCSM_SUCCESS) &&
                (hdr=wlcsm_mngr_recv_mesg(temp_buf))) {
            if((hdr->type==WLCSM_MSG_GETWL_VAR_RESP_DONE) && hdr->len && hdr->pid) {
                /* don't use strcpy as we may need some datasturcutre return for wlmngr */
                memcpy(varValue,(char *)(hdr+1),hdr->len);
                ret=varValue;
            }
        }
        free(p_mngr_var);
    }
    return ret;
}


int _wlcsm_mngr_dm_validate(unsigned int radio_idx,unsigned int sub_idx,unsigned int oid,unsigned short offset, char *value,unsigned int b_set)
{

    char *name="odVar";
    int buflen=_get_valuepair_total_len(name,value,0)+sizeof(t_WLCSM_MNGR_VARHDR);
    t_WLCSM_MNGR_VAR  *p_mngr_var=(t_WLCSM_MNGR_VAR *)malloc(buflen);
    char temp_buf[MAX_NLRCV_BUF_SIZE];
    if(b_set)
        WLCSM_MNGR_CMD_SET_CMD(radio_idx,WLCSM_MNGR_CMD_SET_DM_VAR);
    else
        WLCSM_MNGR_CMD_SET_CMD(radio_idx,WLCSM_MNGR_CMD_VALIDATE_DM_VAR);
    if(p_mngr_var) {
        p_mngr_var->hdr.radio_idx=radio_idx;
        p_mngr_var->hdr.sub_idx=sub_idx;
        p_mngr_var->hdr.dm_oid=oid;
        p_mngr_var->hdr.offset=offset;

        WLCSM_TRACE(WLCSM_TRACE_DBG," oid:%d,offset:%d,b_set:%d	 \r\n",oid, offset,b_set );

        _valuepair_set(&(p_mngr_var->pair),name,value,0);
        if(_wlcsm_netlink_send_mesg(WLCSM_MSG_SETWL_VAR,(char *)p_mngr_var,buflen,GBM_ISMNGR)== WLCSM_SUCCESS) {
            t_WLCSM_MSG_HDR *hdr=wlcsm_mngr_recv_mesg(temp_buf);
            if(hdr) {
                if(hdr->type==WLCSM_MSG_SETWL_VAR_RESP && hdr->pid) {
                    free(p_mngr_var);
                    if(hdr->len<2) {
                        WLCSM_TRACE(WLCSM_TRACE_DBG," return Suc \r\n" );
                        return WLCSM_SUCCESS;
                    } else {
                        WLCSM_TRACE(WLCSM_TRACE_DBG," return ERR \r\n" );
                        return WLCSM_GEN_ERR;
                    }
                } else if(!hdr->pid)
                    WLCSM_TRACE(WLCSM_TRACE_DBG,"%s:%d  MNGR Process has not been started yet \r\n",__FUNCTION__,__LINE__ );
            } else
                WLCSM_TRACE(WLCSM_TRACE_DBG,"%s:%d  mngr_dm_validate received nothing back!!! \r\n",__FUNCTION__,__LINE__ );
        }
        free(p_mngr_var);
    }
    return WLCSM_GEN_ERR;
}

int wlcsm_mngr_dm_validate(unsigned int radio_idx,unsigned int sub_idx,unsigned int oid,unsigned short offset, char *value)
{
    return  _wlcsm_mngr_dm_validate(radio_idx,sub_idx,oid,offset,value,0);
}

int wlcsm_mngr_dm_set(unsigned int radio_idx,unsigned int sub_idx,unsigned int oid,unsigned short offset, char *value)
{
    return  _wlcsm_mngr_dm_validate(radio_idx,sub_idx,oid,offset,value,1);
}


int _wlcsm_mngr_set(unsigned int radio_idx,unsigned int sub_idx,char * name, char *value,int len)
{
    int buflen=_get_valuepair_total_len(name,value,len)+sizeof(t_WLCSM_MNGR_VARHDR);
    t_WLCSM_MNGR_VAR  *p_mngr_var=(t_WLCSM_MNGR_VAR *)malloc(buflen);
    char temp_buf[MAX_NLRCV_BUF_SIZE];
    if(p_mngr_var) {
        p_mngr_var->hdr.radio_idx=radio_idx;
        p_mngr_var->hdr.sub_idx=sub_idx;
        _valuepair_set(&(p_mngr_var->pair),name,value,len);
        if(_wlcsm_netlink_send_mesg(WLCSM_MSG_SETWL_VAR,(char *)p_mngr_var,buflen,GBM_ISMNGR)== WLCSM_SUCCESS) {
            t_WLCSM_MSG_HDR *hdr=wlcsm_mngr_recv_mesg(temp_buf);
            if(hdr) {
                if(hdr->type==WLCSM_MSG_SETWL_VAR_RESP && hdr->pid) {
                    free(p_mngr_var);
                    return WLCSM_SUCCESS;
                } else if(!hdr->pid)
                    WLCSM_TRACE(WLCSM_TRACE_DBG,"%s:%d  MNGR Process has not been started yet \r\n",__FUNCTION__,__LINE__ );
                else
                    WLCSM_TRACE(WLCSM_TRACE_DBG,"%s:%d  SETMNGR wrong!!! \r\n",__FUNCTION__,__LINE__ );
            }
        }
        free(p_mngr_var);
    }
    return WLCSM_GEN_ERR;
}

int wlcsm_mngr_set(unsigned int radio_idx,unsigned int sub_idx,char * name, char *value)
{
    return _wlcsm_mngr_set(radio_idx,sub_idx,name,value,0);

}

t_WLCSM_NAME_VALUEPAIR *wlcsm_get_namevalue_buf(char *name,char *value,int len)
{
    t_WLCSM_NAME_VALUEPAIR *buf=NULL;
    int buflen=_get_valuepair_total_len(name,value,len);
    buf=(t_WLCSM_NAME_VALUEPAIR *)malloc(buflen);
    if(buf!=NULL)
        _valuepair_set(buf,name,value,len);
    return buf;
}

int wlcsm_mngr_response(t_WLCSM_MSGTYPES type,char * name,char *value,int len,unsigned int to_pid)
{
    if(value!=NULL) {
        if(_wlcsm_netlink_send_mesg(type,value,len,to_pid)== WLCSM_SUCCESS) {
            return WLCSM_SUCCESS;
        }
    }
    return WLCSM_GEN_ERR;
}

int wlcsm_register_process (char *process_name)
{
    t_WLCSM_MSG_REGISTER *reg;
    char temp_buf[MAX_NLRCV_BUF_SIZE];
    int len=2*sizeof(int)+strlen(process_name)+1;

    WLCSM_NL_INITIALIZE(g_WLCSM_IS_DAEMON);

    if((reg=malloc(len))!=NULL) {
        reg->pid=wlcsm_agent.mngr_pid;
        reg->len=strlen(process_name)+1;
        memcpy(reg->process,process_name,reg->len);
        if(wlcsm_netlink_send_mesg(WLCSM_MSG_REGISTER,(char *)reg,len)== WLCSM_SUCCESS) {
            t_WLCSM_MSG_HDR *hdr=wlcsm_unicast_recv_mesg(temp_buf);
            if(hdr!=NULL && hdr->type==WLCSM_MSG_REGISTER) {
                free(reg);
                return wlcsm_agent.mngr_sock_nl;
            }
        }
        free(reg);
    }
    return -1;
}

int wlcsm_mngr_update_stalist(unsigned int radio_idx,unsigned int sub_idx,void *value,int len)
{
    //  WLCSM_MNGR_CMD_SET_SOURCE(radio_idx,source);
    WLCSM_MNGR_CMD_SET_CMD(radio_idx,WLCSM_MNGR_CMD_UPDATE_STALIST);
    return _wlcsm_mngr_set(radio_idx,sub_idx,"updatesta",value,len);
}

int wlcsm_mngr_nethotplug_notify(char *net,char *action)
{
    unsigned int radio_idx=0;
    WLCSM_MNGR_CMD_SET_CMD(radio_idx,WLCSM_MNGR_CMD_NETHOTPLUG);
    return _wlcsm_mngr_set(radio_idx,0,net,action,strlen(action)+1);
}

int wlcsm_mngr_dm_set_dbglevel(int dbglevel)
{
    int radio_idx=0;
    char value[48];
    int len= sprintf(value,"%d",dbglevel);
    WLCSM_MNGR_CMD_SET_CMD(radio_idx,WLCSM_MNGR_CMD_DMSETDBGLEVEL);
    return _wlcsm_mngr_set(radio_idx,0,"dmdbglevel",value,len+1);
}

