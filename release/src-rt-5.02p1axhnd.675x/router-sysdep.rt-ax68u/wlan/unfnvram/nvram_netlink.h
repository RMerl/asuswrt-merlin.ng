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

#ifndef __NVRAM_NETLINK_H__
#define __NVRAM_NETLINK_H__

/*********************************************************************************************
 *
 *   This section includes the needed functions and structure definations
 *   to communicate with wlcsm.ko via linux netlink.
 *
 *   These definitions are copied from wlcsm_linux.h directly
 *   (wlcsm_linux.h: bcmdrivers/broadcom/char/wlcsm_ext/impl1/include) 
 */

#define WLCSM_SET_TYPE(x, v)  (((struct wlcsm_msg_hdr *)x)->type = v)
#define WLCSM_SET_LEN(x, v)  (((struct wlcsm_msg_hdr *)x)->len = v)
#define WLCSM_SET_PID(x, v)  (((struct wlcsm_msg_hdr *)x)->pid = v)
#define VALUEPAIR_NAME(v) (((t_WLCSM_NAME_VALUEPAIR *)(v))->value)
#define VALUEPAIR_NAME_LEN(v) (((t_WLCSM_NAME_VALUEPAIR *)(v))->len)
#define MAX_NLRCV_BUF_SIZE   1280 /*MAX BUF NEEDS TO BIGGER THAN NL_PACKET_SIZE */

typedef struct wlcsm_msg_hdr {
    unsigned short type;
    unsigned short len;
    unsigned int pid;
} t_WLCSM_MSG_HDR;

typedef enum wlcsm_msgtype {
    WLCSM_MSG_BASE = 0,
    WLCSM_MSG_REGISTER,
    WLCSM_MSG_NVRAM_SET,
    WLCSM_MSG_NVRAM_GET,
    WLCSM_MSG_NVRAM_UNSET,
    WLCSM_MSG_NVRAM_GETALL,
    WLCSM_MSG_NVRAM_GETALL_DONE,
    WLCSM_MSG_NVRAM_COMMIT,
#ifdef WLCSM_DEBUG
    WLCSM_MSG_NVRAM_SETTRACE,
    WLCSM_MSG_DEBUG_LOGMESSAGE,
    WLCSM_MSG_DEBUGPID_REG,
#endif
    WLCSM_MSG_GETWL_BASE,
    WLCSM_MSG_GETWL_VAR,
    WLCSM_MSG_GETWL_VAR_RESP,
    WLCSM_MSG_GETWL_VAR_RESP_DONE,
    WLCSM_MSG_SETWL_VAR,
    WLCSM_MSG_SETWL_VAR_RESP,
} t_WLCSM_MSGTYPES;

typedef struct t_wlcsm_name_valuepair {
    int len;                                /**< total length of this structure name:value */
    char value[1];                          /**< acting as pointer to the name:value pair string */
} t_WLCSM_NAME_VALUEPAIR;

static inline int _str_int_aligned_len(const char *name)
{
    int nlen=strlen(name)+1;
    int modlen=(nlen&(sizeof(int)-1));
    if(modlen) return (nlen+sizeof(int))&(~(sizeof(int)-1));
    else return nlen;
}

static inline int _get_valuepair_total_len(const char *name, const char *value, int len)
{
    return 2*sizeof(int)+_str_int_aligned_len(name)+(len?len:(value!=NULL?(strlen(value)+1):0));
}

static inline t_WLCSM_NAME_VALUEPAIR *_get_valuepair_value(t_WLCSM_NAME_VALUEPAIR *v)
{
    return (t_WLCSM_NAME_VALUEPAIR *)((char *)(v)+sizeof(int)+_str_int_aligned_len(VALUEPAIR_NAME(v)));
}

#define VALUEPAIR_VALUE(v) ((_get_valuepair_value((t_WLCSM_NAME_VALUEPAIR *)(v))->len)?\
			    (_get_valuepair_value((t_WLCSM_NAME_VALUEPAIR *)(v))->value):NULL)

/*************************************************************************************************/

#define m_NETLINK_PORT 31
#define INVALID_SOCKET -1
#define IS_VALID_SOCKET(_fd_) ((_fd_) > INVALID_SOCKET)
#define NL_NOT_INITED (nl_handler.sock_nl == INVALID_SOCKET)

typedef struct t_nl_handler {
    int            sock_nl;
    int            uni_pid;
} t_NL_HANDLER;

int netlink_init(void);
int netlink_free(void);
int netlink_send_mesg(int msg_type, char *buf, int len);
t_WLCSM_MSG_HDR *netlink_recv_mesg(char *to_buf);
t_WLCSM_NAME_VALUEPAIR *get_namevalue_buf(const char *name, const char *value, int len);

#endif
