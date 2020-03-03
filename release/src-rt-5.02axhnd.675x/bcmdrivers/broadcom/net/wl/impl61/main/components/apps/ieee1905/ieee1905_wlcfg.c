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
 * $Change: 116460 $
 ***********************************************************************/

/*
 * IEEE1905 WLCFG AUTO CONFIGURATION
 */
#if defined(WIRELESS)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/time.h>
#include <bcmnvram.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include "i5ctl.h"
#include "i5ctl_wlcfg.h"
#include "ieee1905_wlcfg.h"
#include "ieee1905_trace.h"
#include "ieee1905_wlmetric.h"
#include "ieee1905_message.h"
#include "ieee1905_datamodel_priv.h"
#include "ieee1905_glue.h"
#include "ieee1905_tlv.h"
#include "ieee1905_interface.h"
#include <wldefs.h>
#if defined(DSLCPE_WLCSM_EXT)
#include <stdarg.h>
#include <wlcsm_lib_api.h>
#endif // endif
#include <wps.h>
#include <wps_1905.h>
#include "shutils.h"
#include <ethernet.h>
#ifdef MULTIAP
#include <wpstypes.h>
#include <dh.h>
#include <md5.h>
#include <bn.h>
#include <sha256.h>
#include <hmac_sha256.h>
#include <aes.h>
#include <linux/sockios.h>
#include <linux/ethtool.h>
#endif	/* MULTIAP */

#define WPS_1905_SOCKET_MAX_RECV_BUFFER_SIZE 4096
#define WPS_1905_RECEIVE_TIMEOUT_MS          3000
#define WPS_1905_RESTART_TIMEOUT_MS          15000

#define I5_TRACE_MODULE i5TraceWlcfg

#ifdef MULTIAP
#define BUF_SIZE_1536_BITS	192
#define txt  "Wi-Fi Easy and Secure Key Derivation"
#define KDF_KEY_BITS            640

static uint8 DH_P_VALUE[BUF_SIZE_1536_BITS] =
{
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xC9, 0x0F, 0xDA, 0xA2, 0x21, 0x68, 0xC2, 0x34,
        0xC4, 0xC6, 0x62, 0x8B, 0x80, 0xDC, 0x1C, 0xD1,
        0x29, 0x02, 0x4E, 0x08, 0x8A, 0x67, 0xCC, 0x74,
        0x02, 0x0B, 0xBE, 0xA6, 0x3B, 0x13, 0x9B, 0x22,
        0x51, 0x4A, 0x08, 0x79, 0x8E, 0x34, 0x04, 0xDD,
        0xEF, 0x95, 0x19, 0xB3, 0xCD, 0x3A, 0x43, 0x1B,
        0x30, 0x2B, 0x0A, 0x6D, 0xF2, 0x5F, 0x14, 0x37,
        0x4F, 0xE1, 0x35, 0x6D, 0x6D, 0x51, 0xC2, 0x45,
        0xE4, 0x85, 0xB5, 0x76, 0x62, 0x5E, 0x7E, 0xC6,
        0xF4, 0x4C, 0x42, 0xE9, 0xA6, 0x37, 0xED, 0x6B,
        0x0B, 0xFF, 0x5C, 0xB6, 0xF4, 0x06, 0xB7, 0xED,
        0xEE, 0x38, 0x6B, 0xFB, 0x5A, 0x89, 0x9F, 0xA5,
        0xAE, 0x9F, 0x24, 0x11, 0x7C, 0x4B, 0x1F, 0xE6,
        0x49, 0x28, 0x66, 0x51, 0xEC, 0xE4, 0x5B, 0x3D,
        0xC2, 0x00, 0x7C, 0xB8, 0xA1, 0x63, 0xBF, 0x05,
        0x98, 0xDA, 0x48, 0x36, 0x1C, 0x55, 0xD3, 0x9A,
        0x69, 0x16, 0x3F, 0xA8, 0xFD, 0x24, 0xCF, 0x5F,
        0x83, 0x65, 0x5D, 0x23, 0xDC, 0xA3, 0xAD, 0x96,
        0x1C, 0x62, 0xF3, 0x56, 0x20, 0x85, 0x52, 0xBB,
        0x9E, 0xD5, 0x29, 0x07, 0x70, 0x96, 0x96, 0x6D,
        0x67, 0x0C, 0x35, 0x4E, 0x4A, 0xBC, 0x98, 0x04,
        0xF1, 0x74, 0x6C, 0x08, 0xCA, 0x23, 0x73, 0x27,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

static uint32 DH_G_VALUE = 2;

static int wlcfg_encrypt_data(unsigned char *plain_txt, int txt_len, unsigned char *encr_key,
	unsigned char **cipherText, int *cipher_len, unsigned char *iv);
static void wlcfg_decrypt_data(unsigned char *cipher, int cipher_len, unsigned char *iv,
	unsigned char *keyWrapKey, unsigned char **plain_txt, int *plain_len);

extern void RAND_linux_init();
extern void RAND_bytes(unsigned char *buf, int num);

#endif	/* MULTIAP */

#ifndef MULTIAP
static int wps1905ProcessUnsolicited(i5_socket_type *psock, WPS_1905_MESSAGE *pMsg);
#endif /* MULTIAP */

#ifndef MULTIAP
/*
 * ===  FUNCTION  ======================================================================
 *         Name:  wps1905ControlSocketReady
 *  Description:  check if connnection to wps monitor is available
 * =====================================================================================
 */
static int wps1905ControlSocketReady( void )
{
    return (i5_config.wl_control_socket.ptmr ? 0 : 1);
}
#endif /* MULTIAP */

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  wps1905InitMessage
 *  Description:  Init a WPS_1905_Message structure with data of len appended
 *   return:  WPS_1905_MESSAGE structure or NULL
 * =====================================================================================
 */
static WPS_1905_MESSAGE *wps1905InitMessage(char const *ifname,WPS_1905_CTL_CMD cmd,int len)
{
    WPS_1905_MESSAGE * pmsg=(WPS_1905_MESSAGE *)malloc(len+sizeof(WPS_1905_MESSAGE));
    if(pmsg)
    {
        if ( ifname )
        {
            strncpy(pmsg->ifName, ifname, I5_MAX_IFNAME-1);
            pmsg->ifName[I5_MAX_IFNAME-1] = '\0';
        }
        else
        {
            pmsg->ifName[0] = '\0';
        }
        pmsg->cmd=cmd;
        pmsg->len=len;
    }
    return pmsg;
}

static int wps1905SendMsg( i5_socket_type *psock, WPS_1905_MESSAGE *pMsg )
{
    struct sockaddr_in sockAddr;
    int                sendLen;
    int                uiLen;

    /* kernel address */
    memset(&sockAddr, 0, sizeof(sockAddr));
    sockAddr.sin_family      = AF_INET;
    sockAddr.sin_addr.s_addr = inet_addr(WPS_1905_ADDR);
    sockAddr.sin_port        = htons(WPS_1905_PORT);

    uiLen   = pMsg->len+sizeof(WPS_1905_MESSAGE);
    sendLen = sendto(psock->sd, pMsg, uiLen, 0, (struct sockaddr *)&sockAddr, sizeof(sockAddr));
    if (uiLen != sendLen)
    {
        printf("%s: sendto failed", __FUNCTION__);
        return -1;
    }

    return 0;
}

#ifndef MULTIAP

/* ===  FUNCTION  ======================================================================
*         Name:  wps1905Register
*  Description:  register with the wps monitor. Retry interval is
*                WPS_1905_RECEIVE_TIMEOUT_MS
*       return:
* =====================================================================================
*/
static void wps1905Register( void *arg )
{
    WPS_1905_MESSAGE   *pMsg;
    controlSockStruct  *pctrlsock = (controlSockStruct*)arg;
    struct sockaddr_in  sockAddr;
    socklen_t           addrLen = sizeof(sockAddr);

    i5TimerFree(pctrlsock->ptmr);
    pctrlsock->ptmr = NULL;

    memset(&sockAddr, 0, sizeof(sockAddr));
    if ( getsockname(pctrlsock->psock->sd, (struct sockaddr *)&sockAddr, &addrLen) < 0)
    {
        i5TraceError("getsockname failed\n");
    }
    else
    {
        unsigned short portNo = ntohs(sockAddr.sin_port);
        i5Trace("Registering UDP port %d with WPS\n", portNo);
        pMsg = wps1905InitMessage(NULL, WPS_1905_CTL_CLIENT_REGISTER, sizeof(unsigned short));
        if(pMsg!=NULL)
        {
            memcpy(WPS1905MSG_DATAPTR(pMsg),&portNo, sizeof(unsigned short));
            wps1905SendMsg(pctrlsock->psock, pMsg);
            free(pMsg);
        }
    }

    pctrlsock->ptmr = i5TimerNew(WPS_1905_RECEIVE_TIMEOUT_MS, wps1905Register, pctrlsock);
}

static WPS_1905_MESSAGE *wps1905ReceiveMsg(i5_socket_type *psock, unsigned char *pBuf, unsigned int maxLen)
{
    int                  recvlen;
    struct sockaddr      src;
    unsigned int         addrlen = sizeof(struct sockaddr);

    recvlen = recvfrom(psock->sd, pBuf, maxLen, 0, &src, &addrlen);
    if (recvlen < 0)
    {
        i5TraceInfo("wps1905ReceiveMsg: receive error (errno=%d, %s)\n", errno, strerror(errno));
        /* likely error is a timeout - try to register with kernel again */
        if ( i5_config.wl_control_socket.ptmr != NULL ) {
          i5TimerFree(i5_config.wl_control_socket.ptmr);
        }
        i5_config.wl_control_socket.ptmr = i5TimerNew(0, wps1905Register, &i5_config.wl_control_socket);
        return NULL;
    }
    else if (recvlen < sizeof(WPS_1905_MESSAGE))
    {
        printf("wps1905ReceiveMsg: invalid receive length\n");
        return NULL;
    }
    return (WPS_1905_MESSAGE *)pBuf;
}

static int wps1905ProcessUnsolicited(i5_socket_type *psock, WPS_1905_MESSAGE *pMsg)
{
    int messageProcessed = 0;

    switch ( pMsg->cmd ) {
        case WPS_1905_CTL_CLIENT_REGISTER:
        {
            i5_dm_device_type *pdevice;

            if ( i5_config.wl_control_socket.ptmr ) {
              i5TimerFree(i5_config.wl_control_socket.ptmr);
              i5_config.wl_control_socket.ptmr = NULL;
            }

            i5Trace("WPS_1905_CTL_CLIENT_REGISTER received\n");

            pdevice = i5DmGetSelfDevice();
            if ( pdevice != NULL )
            {
                i5_dm_interface_type *pinterface = pdevice->interface_list.ll.next;
                while (pinterface != NULL)
                {
                    if ( i5DmIsInterfaceWireless(pinterface->MediaType) )
                    {
                        i5Trace("wlparent %s, Renew %d, registrar %d, wsc %d, change %d, mediaType %x\n",
                               pinterface->wlParentName, pinterface->isRenewPending, I5_IS_REGISTRAR(i5_config.flags),
                               pinterface->confStatus, pinterface->credChanged, pinterface->MediaType);

                        if ( (1 == pinterface->isRenewPending) &&
                             (WPS_1905_CONF_NOCHANGE_CONFIGURED == pinterface->confStatus) &&
                             I5_IS_REGISTRAR(i5_config.flags) &&
                             (1 == pinterface->credChanged) )
                        {
                            unsigned int freqBand = i5TlvGetFreqBandFromMediaType(pinterface->MediaType);
                            i5Trace("sending AP renew - band %d\n", freqBand);
                            i5MessageApAutoconfigurationRenewSend(NULL, NULL, freqBand);
                        }
                        pinterface->isRenewPending = 0;
                    }
                    pinterface = pinterface->ll.next;
                }
            }

            /* check to see if any interfaces need configuration */
            i5WlcfgApAutoconfigurationStart( NULL );
            messageProcessed = 1;
            break;
        }

        case WPS_1905_NOTIFY_CLIENT_RESTART:
        {
            WPS_1905_NOTIFY_MESSAGE *msg=(WPS_1905_NOTIFY_MESSAGE *)(pMsg+1);
            i5_dm_device_type *pdevice;
            i5_dm_interface_type *pinterface;

            i5Trace("WPS_1905_NOTIFY_CLIENT_RESTART received\n");

            pdevice = i5DmGetSelfDevice();
            if ( pdevice != NULL )
            {
                pinterface = pdevice->interface_list.ll.next;
                while (pinterface != NULL)
                {
                    if ( i5DmIsInterfaceWireless(pinterface->MediaType) &&
                         (0 == strcmp(msg->ifName, pinterface->wlParentName)) )
                    {
                        pinterface->isRenewPending = 1;
                        pinterface->credChanged    = msg->credentialChanged;
                        pinterface->confStatus     = msg->confStatus;
                        if ( i5_config.wl_control_socket.ptmr != NULL ) {
                            i5TimerFree(i5_config.wl_control_socket.ptmr);
                        }

                        i5_config.wl_control_socket.ptmr = i5TimerNew(WPS_1905_RESTART_TIMEOUT_MS, wps1905Register, &i5_config.wl_control_socket);
                    }
                    pinterface = pinterface->ll.next;
                }
            }
            messageProcessed = 1;
            break;
        }

        default:
            break;
    }
    return messageProcessed;
}
#endif /* MULTIAP */

static void wps1905ProcessSocket(i5_socket_type *psock)
{
    /* No need to process for MULTIAP as we are not using WPS. The WPS socket is only used for
     * Push button request. Which doesnt send any reply back
     */
#ifndef MULTIAP
    unsigned char     buffer[WPS_1905_SOCKET_MAX_RECV_BUFFER_SIZE];
    WPS_1905_MESSAGE *pMsg=NULL;
    int               ret;

    memset(buffer, 0, WPS_1905_SOCKET_MAX_RECV_BUFFER_SIZE);
    pMsg = wps1905ReceiveMsg(psock, buffer, WPS_1905_SOCKET_MAX_RECV_BUFFER_SIZE );

    if (pMsg == NULL) {
        i5TraceInfo("wps1905ReceiveMsg returned NULL\n");
        return;
    }

    ret  = wps1905ProcessUnsolicited(psock, pMsg);
    if ( 0 == ret )
    {
        i5TraceInfo("Unexpected unsolicited wl message received: cmd %d\n", (int)pMsg->cmd);
    }
    else
    {
        i5TraceInfo("Message cmd %d, len %d, status %d\n", (int)pMsg->cmd, (int)pMsg->len, (int)pMsg->status);
    }
#endif /* MULTIAP */
}

#ifndef MULTIAP
static WPS_1905_MESSAGE *wps1905GetResponse(i5_socket_type *psock, int cmdExpected, unsigned char *pBuf, int maxLen)
{
    WPS_1905_MESSAGE *pMsg=NULL;
    int               ret;
    struct timespec   ts;
    struct timeval    end_tv;
    struct timeval    now_tv;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    end_tv.tv_sec  = ts.tv_sec + (WPS_1905_RECEIVE_TIMEOUT_MS / 1000);
    end_tv.tv_usec = ts.tv_nsec/1000 + ((WPS_1905_RECEIVE_TIMEOUT_MS % 1000) * 1000);

    while ( 1 )
    {
        pMsg = wps1905ReceiveMsg(psock, pBuf, maxLen );
        if ( NULL == pMsg )
        {
            return NULL;
        }
        ret  = wps1905ProcessUnsolicited(psock, pMsg);
        if ( 0 == ret )
        {
            if ( (cmdExpected == pMsg->cmd) ||
                    ((cmdExpected == WPS_1905_CTL_WSCPROCESS) && (WPS_1905_CTL_GETM2 == pMsg->cmd)) ||
                    ((cmdExpected == WPS_1905_CTL_WSCPROCESS) && (WPS_1905_CTL_CONFIGUREAP == pMsg->cmd)) )
            {
                break;
            }
            else
            {
                i5Trace("Unexpected wl message received: cmd %d, exp %d\n", (int)pMsg->cmd, cmdExpected);
            }
        }

        clock_gettime(CLOCK_MONOTONIC, &ts);
        now_tv.tv_sec  = ts.tv_sec;
        now_tv.tv_usec = ts.tv_nsec/1000;
        if (timercmp(&end_tv, &now_tv, <))
        {
            i5TraceInfo("Timeout waiting for response\n");
            if ( i5_config.wl_control_socket.ptmr != NULL ) {
              i5TimerFree(i5_config.wl_control_socket.ptmr);
            }

            i5_config.wl_control_socket.ptmr = i5TimerNew(WPS_1905_RECEIVE_TIMEOUT_MS, wps1905Register, &i5_config.wl_control_socket);
            return NULL;
        }
    }

    i5Trace("Message cmd %d, len %d, status %d\n", (int)pMsg->cmd, (int)pMsg->len, (int)pMsg->status);
    return pMsg;
}

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  wps1905SendRequest
 *  Description:  Send specific request to WPS daemon with content.
 *   Parameters:  ifname- wirelss interface name
 *         cmd-   command of request
 *         rmsg-   content of the requst
 *         len-   the length of content.
 * =====================================================================================
 */
static int wps1905SendRequest(i5_socket_type *psock,char const *ifname,WPS_1905_CTL_CMD cmd, unsigned char *rmsg,unsigned int len)
{
    WPS_1905_MESSAGE *pMsg=NULL;

    i5Trace("sending request - cmd %d\n", cmd);

    /* if command has no message, can use NULL as rmsg,len is ignored then */
    if(wps1905ControlSocketReady())
    {
        if(rmsg==NULL)
            pMsg=wps1905InitMessage(ifname,cmd,0);
        else
        {
            pMsg=wps1905InitMessage(ifname,cmd,len);
            if(pMsg!=NULL)
            {
                memcpy(WPS1905MSG_DATAPTR(pMsg),rmsg,len);
            }
            else
            {
                return -1;
            }
        }
        wps1905SendMsg(psock, pMsg);
        free(pMsg);
    }
    else
    {
        return -1;
    }
    return 0;
}

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  wps1905Commander
 *  Description:  sending WPS request and expect matching repsonse.
 *   Parameters:  ifname- wireles interface name
 *         cmd-   command of request
 *         msg-   content of the requst
 *         len-   the length of content.
 *         respmsg- the response content
 *         len-   the length of the return content
 *       return:  return command execution status
 * =====================================================================================
 */
static int wps1905Commander(char const *ifname,WPS_1905_CTL_CMD cmd,unsigned char *msg, int len,unsigned char **respmsg,int *rlen)
{
    WPS_1905_MESSAGE *pRespMsg=NULL;
    int status=-1;

    *rlen = 0;
    if ( 0 == wps1905ControlSocketReady() )
    {
        return status;
    }

    if(0 == wps1905SendRequest(i5_config.wl_control_socket.psock, ifname, cmd, msg, len))
    {
        unsigned char  buffer[WPS_1905_SOCKET_MAX_RECV_BUFFER_SIZE];

        memset(buffer, 0, WPS_1905_SOCKET_MAX_RECV_BUFFER_SIZE);
        pRespMsg = wps1905GetResponse(i5_config.wl_control_socket.psock, cmd, buffer, WPS_1905_SOCKET_MAX_RECV_BUFFER_SIZE);
        if ( NULL == pRespMsg )
        {
            return status;
        }
        if ( pRespMsg->len > 0 )
        {
            unsigned char *retmsg=malloc(pRespMsg->len);
            if (retmsg)
            {
                memcpy(retmsg,(void *)(pRespMsg+1),pRespMsg->len);
                *respmsg=retmsg;
                *rlen=pRespMsg->len;
            }
        }
        status = pRespMsg->status;
    }
    else
    {
        printf("%s: %d  error sending message to WPS \n",__FUNCTION__,__LINE__ );
    }
    return status;
}

/*-----------------------------------------------------------------------------
 *
 *  The following functions are Wireless Autoconfiguraiton APIs which should be
 *  called in 1905 process. For Unit testing, i5ctl commands are developed to
 *  use these functions for verification
 *-----------------------------------------------------------------------------*/
/*
 * ===  FUNCTION  ======================================================================
 *         Name:  wps1905WlInstance
 *  Description:  get numbers of wirelesss interfaces on board
 *       return:  <0 error, likely WPS is not running or did not get response from WPS in 10 seconds
 *           >=0 number of wireless interfaces.
 * =====================================================================================
 */
static int wps1905WlInstance()
{
    unsigned char* retmsg=NULL;
    int length=0;

    int status=wps1905Commander(NULL,WPS_1905_CTL_WLINSTANCE,NULL,0,&retmsg,&length);
    if(retmsg)
        free(retmsg);
    return status;
}

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  wps1905WscStatus
 *  Description:  tell what is current WSC s/atus
 *       return:  <0 error, likely WPS is not running or did not get response from WPS in 10 seconds
 *           WPS_1905_WSCAP_CONFIGURED: WPS enabled, WSC AP is configured by either M2
 *                      manually set.
 *           WPS_1905_WSCAP_UNCONFIGURED: WPS enabled, WSC AP is not configured.
 *           WPS_1905_WSCAP_SESSIONONGOING: There is WPS session is on going
 * =====================================================================================
 */
static int wps1905WscStatus( )
{
    unsigned char* retmsg=NULL;
    int length=0;
    int status=wps1905Commander(NULL,WPS_1905_CTL_WSCSTATUS,NULL,0,&retmsg,&length);
    if(retmsg)
        free(retmsg);
    return status;
}

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  wps1905WscEnabled
 *  Description:  check if WSC is enabled for the given inteface
 *       return:  <0 error, likely WPS is not running or time out occurred
 *           WPS_1905_WSCAP_ENABLED
 *           WPS_1905_WSCAP_DISABLED
 * =====================================================================================
 */
static int wps1905WscEnabled(char const *ifname)
{
    unsigned char* retmsg=NULL;
    int length=0;
    int status=wps1905Commander(ifname,WPS_1905_CTL_WSCENABLED,NULL,0,&retmsg,&length);
    if(retmsg)
        free(retmsg);
    return status;
}

/* ===  FUNCTION  ======================================================================
*         Name:  wps1905StopApAutoConf
*  Description:  action to stop AP autoconfiguration
*       return:  <0 error, likely WPS is not running or did not get response from WPS in 10 seconds
*           1: successful
*           0: failure
* =====================================================================================
*/
static int wps1905StopApAutoConf( )
{
    unsigned char* retmsg=NULL;
    int length=0;

    int status=wps1905Commander(NULL,WPS_1905_CTL_WSCCANCEL,NULL,0,&retmsg,&length);
    if(retmsg)
        free(retmsg);
    return status;
}

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  wps1905GetMessageInfo
 *  Description:  This function is used to get the message type of a received WSC message
 *   Parameters:
 *         msg-    incoming 1905 WSC message
 *         len-    incoming message length
 *
 *       return:  error or message type

 * =====================================================================================
 */
static int wps1905GetMessageInfo(unsigned char *msg, unsigned int len, int *pmtype, int *prfband)
{
    WPS_1905_M_MESSAGE_INFO *pminfo=NULL;
    int length=0;

    int status=wps1905Commander(NULL, WPS_1905_CTL_GETMINFO, msg, len, (unsigned char **)&pminfo, &length);

    if ( (pminfo == NULL ) || (length != sizeof(WPS_1905_M_MESSAGE_INFO)) || (status != 0) )
    {
        return -1;
    }

    *pmtype  = pminfo->mtype;
    *prfband = pminfo->rfband;
    free(pminfo);
    return 0;
}

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  wps1905WscProcess
 *  Description:  This function is used to handle all 1905 WSC messages,now we expect
 *        this message to be either M1 or M2 message,in standard, it is not clear
 *        it is in EAP format or not, but for now,we will handle only bare M1/M2
 *        message without EAP header(which is generated from other APIs).We can
 *        change it later with clarification for this.
 *   Parameters:  ifname-   wireless interface name
 *         msg-    incoming 1905 WSC message
 *         len-    incoming message length
 *         retmsg-  pointer to a possible returned message [remember to release
 *             if it is not NULL in your caller]
 *         length-   returned message length
 *
 *       return:  <0 error, likely WPS is not running or did not get response from WPS in 10 seconds
 *         For M2 mesage:
 *           WPS_1905_M2SET_DONE:    Successfully set AP.
 *           WPS_1905_M2SET_NOMATCHING:  M2 information does not match to M1
 *           WPS_1905_M2SET_NOSESSION:   There is no session to use this M2 information
 *           WPS_1905_M2SET_NOAUTHENTICATOR:  M2 message does not have Authentication info
 *
 *           For M1 Message:
 *             WPS_1905_M1HANDLE_M2DATA:   successful get M2 information
 *           WPS_1905_M1HANDLE_NOTREGISRAR : CPE can not act as Registrar.
 *
 *           For UNKNOW Message:
 *             1)WPS_1905_UNKNOWNWSCMESSAGE.
 *
 *         other: error happen in WPS.
 * =====================================================================================
 */
static int wps1905WscProcess(char const *ifname,int cmd,unsigned char *msg,unsigned int len,unsigned char **retmsg, int *length)
{
    int status=wps1905Commander(ifname,cmd,msg,len,retmsg,length);
    return status;
}
#endif /* MULTIAP */

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  _i5WlcfgCtlResponse
 *  Description:  internal function to send response to i5ctl command line.
 * =====================================================================================
 */
static void _i5WlcfgCtlResponse(i5_socket_type *psock, t_I5_API_CMD_NAME cmd, char subcmd, int status, char *buf, int len)
{
    char               *pMsgBuf;
    int                 retMsgLen;
    t_I5_API_WLCFG_MSG *pRetMsg;

    if((buf!=NULL)&&len==0)
        len=strlen(buf)+1; /*  this is for for sending string case */
    retMsgLen = len + sizeof(t_I5_API_WLCFG_MSG);
    pMsgBuf = malloc(retMsgLen);
    if((buf!=NULL) && (len>0))
    {
        memcpy(pMsgBuf + sizeof(t_I5_API_WLCFG_MSG), buf, len);
    }
    pRetMsg = (t_I5_API_WLCFG_MSG *)pMsgBuf;
    pRetMsg->subcmd = subcmd;
    pRetMsg->status = status;
    i5apiSendMessage(psock->sd, cmd, pMsgBuf, retMsgLen);
    free(pMsgBuf);
}

unsigned int i5WlCfgAreMediaTypesCompatible(unsigned short mediaType1, unsigned short mediaType2)
{
  if (mediaType1 == mediaType2) {
    return 1;
  }
  else if ((mediaType1 == I5_MEDIA_TYPE_WIFI_N24) &&
           ((mediaType2 == I5_MEDIA_TYPE_WIFI_A) || (mediaType2 == I5_MEDIA_TYPE_WIFI_B) || (mediaType2 == I5_MEDIA_TYPE_WIFI_G))
          ) {
    return 1;
  }
  else if ((mediaType2 == I5_MEDIA_TYPE_WIFI_N24) &&
           ((mediaType1 == I5_MEDIA_TYPE_WIFI_A) || (mediaType1 == I5_MEDIA_TYPE_WIFI_B) || (mediaType1 == I5_MEDIA_TYPE_WIFI_G))
          ) {
    return 1;
  }
  else if (((mediaType2 == I5_MEDIA_TYPE_WIFI_N5) && (mediaType1 == I5_MEDIA_TYPE_WIFI_AC)) ||
             ((mediaType1 == I5_MEDIA_TYPE_WIFI_N5) && (mediaType2 == I5_MEDIA_TYPE_WIFI_AC))
            ) {
    return 1;
  }
  return 0;
}

static unsigned short i5WlCfgFetchWirelessNBand(char const *ifname)
{
  char *line = NULL;

  line = i5WlCfgGetNvVal(ifname, "nband");
  if (line != NULL) {
    i5TraceInfo("nband: %s", line);
    switch(line[0]) {
      case '2':
        return I5_MEDIA_TYPE_WIFI_N24;
      case '1':
        return I5_MEDIA_TYPE_WIFI_N5;
      default:
        return I5_MEDIA_TYPE_UNKNOWN;
    }
  }

  return I5_MEDIA_TYPE_UNKNOWN;
}

/* Get wlX_ or wlX.y Prefix from OS specific interface name */
int
i5WlCfgGetPrefix(const char *ifname, char *prefix, int prefix_len)
{
	int ret = 0;
	char wl_name[I5_MAX_IFNAME];

	/* Convert eth name to wl name - returns 0 if success */
	ret = osifname_to_nvifname(ifname, wl_name, sizeof(wl_name));
	if (ret != 0) {
		i5TraceError("osifname_to_nvifname failed for %s\n\n", ifname);
		return ret;
	}

	/* Get prefix of the interface from Driver */
	if (make_wl_prefix(prefix, prefix_len, 1, wl_name) == NULL) {
		i5TraceError("failed to get prefix for %s\n\n", ifname);
		return -1;
	}
	prefix[strlen(prefix) - 1] = '\0';
	i5TraceInfo("Interface: %s, wl_name: %s, Prefix: %s\n", ifname, wl_name, prefix);

	return ret;
}

/* returns : media_type on success,
 *         : I5_MEDIA_TYPE_UNKNOWN on failure
 */
unsigned short i5WlCfgFetchWirelessIfInfo(char const *ifname, unsigned char *pMediaInfo, int *pMediaLen,
                                          unsigned char *netTechOui, unsigned char *netTechVariant, unsigned char *netTechName, unsigned char *url, int sizeUrl)
{
  char *line = NULL;
  const char *wlName;
  char wlParentIf[I5_MAX_IFNAME];
#ifdef MULTIAP
  char *nvval = NULL;
#endif /* MULTIAP */
  chanspec_t chanspec;

  if ( pMediaInfo ) {
    if ( *pMediaLen < i5TlvMediaSpecificInfoWiFi_Length ) {
      i5TraceError("invalid media info length\n");
      return I5_MEDIA_TYPE_UNKNOWN;
    }
    else {
      /* all zero media info for now */
      memset(pMediaInfo, 0, i5TlvMediaSpecificInfoWiFi_Length);
      *pMediaLen = i5TlvMediaSpecificInfoWiFi_Length;
    }
  }

  if ( 0 == strncmp(I5_GLUE_WLCFG_WDS_NAME_STRING, ifname, strlen(I5_GLUE_WLCFG_WL_NAME_STRING)) ) {
    strcpy(&wlParentIf[0], I5_GLUE_WLCFG_WL_NAME_STRING);
    wlParentIf[strlen(I5_GLUE_WLCFG_WL_NAME_STRING)] = ifname[strlen(I5_GLUE_WLCFG_WDS_NAME_STRING)];
    wlParentIf[strlen(I5_GLUE_WLCFG_WL_NAME_STRING) + 1] = '\0';
    i5TraceInfo("Interface: %s, wl_name: %s, Prefix: %s\n", ifname, wlParentIf, wlParentIf);
    wlName = wlParentIf;
  } else if (i5WlCfgGetPrefix(ifname, wlParentIf, sizeof(wlParentIf)) == 0) {
    wlName = wlParentIf;
  } else {
    wlName = (char *)ifname;
  }

#ifdef MULTIAP
  nvval = i5WlCfgGetNvVal(wlName, "mode");
  if (strcmp(nvval, "sta") == 0) {
    i5_config.dwds_enabled = 1;
  }
#endif /* MULTIAP */
  line = i5WlCfgGetNvVal(wlName, "phytype");
  if (line != NULL) {
    i5TraceInfo("ifname %s phytype: %s", ifname, line);
    if (strncmp(line,WL_PHY_TYPE_A,1) == 0) {
      return I5_MEDIA_TYPE_WIFI_A;
    }
    else if (strncmp(line,WL_PHY_TYPE_B,1) == 0) {
      return I5_MEDIA_TYPE_WIFI_B;
    }
    else if (strncmp(line,WL_PHY_TYPE_G,1) == 0) {
      return I5_MEDIA_TYPE_WIFI_G;
    }
    else if ( (strncmp(line,WL_PHY_TYPE_N,1) == 0) ||
              (strncmp(line,WL_PHY_TYPE_H,1) == 0) ||
              (strncmp(line,WL_PHY_TYPE_LP,1) == 0) ) {
      return i5WlCfgFetchWirelessNBand(wlName);
    }
    else if (strncmp(line,WL_PHY_TYPE_AC,1) == 0) {
      i5WlCfgGetChanspec((char *)ifname, &chanspec);
      if (CHSPEC_BAND(chanspec) == WL_CHANSPEC_BAND_5G) {
        return I5_MEDIA_TYPE_WIFI_AC;
      } else {
        return I5_MEDIA_TYPE_WIFI_N24;
      }
    }
 /* The codes for AD and AF, supported by 1905, are not supported in wldefs.h
    } else if (strncmp(line,WL_PHY_TYPE_AD,1)) {
      return I5_MEDIA_TYPE_WIFI_AD;
    } else if (strncmp(line,WL_PHY_TYPE_AF,1)) {
        return I5_MEDIA_TYPE_WIFI_AF;
    } */
    return I5_MEDIA_TYPE_UNKNOWN;
  }
  return I5_MEDIA_TYPE_UNKNOWN;
}

char *i5WlcfgGetWlParentInterface(char const *ifname, char *wlParentIf)
{
  /* if ifname matches the wds string then we want to return the wl name string plus the wlindex
     (first number after name string) - ifname = wdsx.y, return wlx */
  if ( 0 == strncmp(I5_GLUE_WLCFG_WDS_NAME_STRING, ifname, strlen(I5_GLUE_WLCFG_WDS_NAME_STRING)) ) {
    snprintf(wlParentIf, I5_MAX_IFNAME, "%s%c", I5_GLUE_WLCFG_WL_NAME_STRING, ifname[strlen(I5_GLUE_WLCFG_WDS_NAME_STRING)]);
    return wlParentIf;
  }
  else {
    if ( 0 == strncmp(I5_GLUE_WLCFG_WL_NAME_STRING, ifname, strlen(I5_GLUE_WLCFG_WL_NAME_STRING)) ) {
      return (char *)ifname;
    } else if (i5WlCfgGetPrefix(ifname, wlParentIf, I5_MAX_IFNAME) == 0) {
      return wlParentIf;
    }
    else {
      return NULL;
    }
  }
}

char *i5WlcfgGetIfnameFromWlParent(const char *wlParent)
{
  char cmd[256];
  char *ifname;

  sprintf(cmd, "%s_ifname", wlParent);
  ifname = nvram_safe_get(cmd);
  return ifname;
}

int i5WlCfgGetWdsMacFromName(char const *ifname, char *macString, int maxLen)
{
  char const *wdsIdxStr;
  int         wlIndex;
  int         wdsIndex;
  char       *endptr;

  /* format is <name><wlidx>.<wdsidx> */
  if ( 0 == strncmp(I5_GLUE_WLCFG_WDS_NAME_STRING, ifname, strlen(I5_GLUE_WLCFG_WDS_NAME_STRING)) ) {
    int i;
    char cmdStr[256] = "";

    /* find first digit in string */
    for ( i = 0; i < strlen(ifname); i++ )
    {
       if ( isdigit(ifname[i]) ) {
         break;
       }
    }

    if ( i == strlen(ifname) ) {
      return -1;
    }

    wdsIdxStr = strstr(ifname, ".");
    if ( NULL == wdsIdxStr ) {
      return -1;
    }

    errno = 0;
    wlIndex = strtol(&ifname[i], &endptr, 10);
    if ( (errno != 0) || (endptr == &ifname[i])) {
      return -1;
    }

    errno = 0;
    wdsIndex = strtol(&wdsIdxStr[1], &endptr, 10);
    if ( (errno != 0) || (endptr == wdsIdxStr)) {
      return -1;
    }

    snprintf(cmdStr, sizeof(cmdStr), "wl -i %s%d wds > /tmp/wlwds", I5_GLUE_WLCFG_WL_NAME_STRING, wlIndex);
    system(cmdStr);
    return i5InterfaceSearchFileForIndex("/tmp/wlwds", wdsIndex, macString, maxLen);
  }
  return -1;
}

unsigned int i5WlCfgIsApConfigured(char const *ifname)
{
    i5_dm_device_type *pdevice = i5DmGetSelfDevice();

    if ( pdevice != NULL )
    {
        if (I5_IS_REGISTRAR(i5_config.flags))
        {
            return 1;
        }
        else
        {
            i5_dm_interface_type *pinterface = pdevice->interface_list.ll.next;
            while ( pinterface )
            {
                if ( 0 == strcmp(pinterface->wlParentName, ifname) )
                {
                    if ( pinterface->isConfigured )
                    {
                        return 1;
                    }
                    break;
                }
                pinterface = pinterface->ll.next;
            }
        }
    }
    return 0;
}

#ifndef MULTIAP
void i5WlcfgApAutoConfigTimer(void * arg)
{
    apSearchSendStruct *pApSearch = (apSearchSendStruct *)arg;
    apSearchEntry      *pentry;
    int                 timeout;

    i5Trace("AP timeout occurred\n");

    if ( pApSearch->timer != NULL )
    {
        i5TimerFree(pApSearch->timer);
        pApSearch->timer = NULL;
    }

    if ( NULL == pApSearch->searchEntryList.ll.next )
    {
        i5TraceInfo("no entries - cancel timer\n");
        return;
    }

    if ( 0 == wps1905ControlSocketReady() )
    {
        pApSearch->timer = i5TimerNew(I5_MESSAGE_AP_SEARCH_NOT_READY_INTERVAL_MSEC, i5WlcfgApAutoConfigTimer, pApSearch);
        return;
    }

    /* a timeout has occurred or one of the entries is finished,
       move to the next search entry, send the search message
       for the entry, and start the timer */
    pentry = pApSearch->activeSearchEntry;
    if ( pentry != NULL )
    {
        pentry = pentry->ll.next;
    }

    if ( pentry == NULL )
    {
        pentry = pApSearch->searchEntryList.ll.next;
    }

    timeout = I5_MESSAGE_AP_SEARCH_NOT_READY_INTERVAL_MSEC;
    pApSearch->activeSearchEntry = pentry;
    pentry->callCounter++;
    do
    {
        if ( 0 == pentry->renew )
        {
            if ( pentry->callCounter < I5_MESSAGE_AP_SEARCH_START_COUNT )
            {
                pentry->expiryTime = I5_MESSAGE_AP_SEARCH_START_INTERVAL_MSEC;
            }
            else
            {
                pentry->expiryTime = I5_MESSAGE_AP_SEARCH_PERIODIC_INTERVAL_MSEC;
            }
            timeout = pentry->expiryTime;
            i5Trace("AP AutoConfiguration Send Band[%d] ifname[%s]\n", pentry->freqBand,
              pentry->ifname);
            i5MessageApAutoconfigurationSearchSend(pentry->freqBand);
            pentry->expectedMsg = i5MessageApAutoconfigurationResponseValue;
        }
        else
        {
            i5_socket_type *psock;
            int             rc;
            unsigned char  *pData;
            int             dataLen = 0;

            psock = i5SocketFindDevSocketByName(pentry->rxIfname);
            if ( NULL == psock )
            {
                i5TraceError("RX interface no longer available going to send search\n");
                pentry->renew = 0;
                pentry->callCounter = 0;
                continue;
            }
            else
            {
                /* clear active session */
                wps1905StopApAutoConf();

                /* Fetch M1 from Wireless */
                rc = wps1905WscProcess(pentry->ifname, WPS_1905_CTL_STARTAUTOCONF, NULL, 0, &pData, &dataLen);
                if (WPS_1905_RESHANDLE_M1DATA == rc)
                {
                    i5Trace("AP Autoconfiguration WSC send on Band[%d]\n", pentry->freqBand);
                    i5MessageApAutoconfigurationWscSend(psock, pentry->registrarMac, pData, dataLen, NULL);
                    pentry->expiryTime = I5_MESSAGE_AP_SEARCH_M1_M2_WAITING_MSEC;
                    timeout = pentry->expiryTime;
                }

                pentry->expectedMsg = i5MessageApAutoconfigurationWscValue;
                pentry->renew = 0;
                pentry->callCounter = I5_MESSAGE_AP_SEARCH_START_COUNT;
            }
        }
    } while( 0 );
    pApSearch->timer = i5TimerNew(timeout, i5WlcfgApAutoConfigTimer, pApSearch);
}
#endif /* MULTIAP */

#ifndef MULTIAP
static void i5WlcfgApAutoconfigurationWscValueDone(apSearchEntry *pentry)
{
  i5_dm_device_type  *pdevice = i5DmGetSelfDevice();
  i5_dm_interface_type *pinterface = pdevice->interface_list.ll.next;
  apSearchSendStruct *pApSearch = &i5_config.apSearch;

  /* set wl interface to configured */
  while ( pinterface != NULL )
  {
      if ( i5DmIsInterfaceWireless(pinterface->MediaType) &&
           (0 == strcmp(pinterface->wlParentName, pentry->ifname)) )
      {
          pinterface->isConfigured = 1;
          break;
      }
      pinterface = pinterface->ll.next;
  }

  /* remove entry from list */
  pApSearch->activeSearchEntry = NULL;
  i5LlItemFree(&pApSearch->searchEntryList, pentry);
  i5WlcfgApAutoConfigTimer(pApSearch);
}
#endif // endif

#ifdef MULTIAP

/* Check if the AL MAC address present in the BSS info table which the controller has */
int i5WlCfgIsALMACPresentInControllerTable(unsigned char *al_mac)
{
  dll_t *item_p, *next_p;
  ieee1905_client_bssinfo_type *list;

  for (item_p = dll_head_p(&i5_config.client_bssinfo_list.head);
    !dll_end(&i5_config.client_bssinfo_list.head, item_p);
    item_p = next_p) {
    next_p = dll_next_p(item_p);
    list = (ieee1905_client_bssinfo_type*)item_p;

    if (memcmp(list->ALID, al_mac, sizeof(list->ALID)) == 0) {
      return 1;
    }
  }

  return 0;
}

/* Calls the function to create and send M1 */
static int i5WlCfgCreateAndSendM1(i5_socket_type *psock, unsigned char *al_mac,
  unsigned int freqband, unsigned char *ifr_mac)
{

  /* Fetch M1 from Wireless */
  i5Trace("freqband[%d] RadioMAC "I5_MAC_DELIM_FMT" Fetch M1\n", freqband, I5_MAC_PRM(ifr_mac));
  if (i5_config.m1 != NULL || i5_config.keys != NULL) {
    wlcfg_wsc_free(i5_config.m1, i5_config.keys);
    i5_config.m1 = NULL;
    i5_config.keys = NULL;
  }

  if (Wlcfg_proto_create_m1(freqband, &i5_config.m1, &i5_config.m1_len, &i5_config.keys) == 0) {
    i5TraceError("freqband[%d] RadioMAC "I5_MAC_DELIM_FMT" Fetch M1 Failed\n", freqband,
      I5_MAC_PRM(ifr_mac));
    return -1;
  }

  i5Trace("Send M1 freqband[%d] RadioMAC "I5_MAC_DELIM_FMT"\n", freqband, I5_MAC_PRM(ifr_mac));

  i5MessageApAutoconfigurationWscM1Send(psock, al_mac, i5_config.m1, i5_config.m1_len, ifr_mac);

  if (i5_config.ptmrWSC) {
    i5TimerFree(i5_config.ptmrWSC);
  }

  /* Create timer after sending M1. If the M2 is not received within the timeout, send M1 for
   * other unconfigured interface
   */
  i5_config.ptmrWSC = i5TimerNew(I5_MESSAGE_AP_SEARCH_M1_M2_WAITING_MSEC,
    i5WlCfgMultiApWSCTimeout, NULL);

  return 0;
}

/* Sends M1 for the unconfigured radio */
static int i5WlCfgSendM1ForUnConfiguredRadio(i5_socket_type *psock,
  unsigned char *neighbor_al_mac_address)
{
  i5_dm_device_type *pdevice = i5DmGetSelfDevice();
  i5_dm_interface_type *pinterface = NULL;
  unsigned int freqband;

  if (pdevice == NULL) {
    return -1;
  }

configstart:
  if (i5DmIsAllInterfacesConfigured()) {
    i5Trace("All the interfaces are configured\n");
    return 0;
  }

  i5Trace("curWSCMac["I5_MAC_DELIM_FMT"] Config START\n",
    I5_MAC_PRM(i5_config.curWSCMac));

  for (pinterface = pdevice->interface_list.ll.next; pinterface; pinterface = pinterface->ll.next) {
    /* For wireless interface */
    if (!i5DmIsInterfaceWireless(pinterface->MediaType)) {
	    continue;
    }
    /* If the MAC is NULL and if its not configured send M1 for this radio */
    if (i5DmIsMacNull(i5_config.curWSCMac)) {
      if (!pinterface->isConfigured) {
        /* Interface is not configured so M1 has to be sent for this interface */
        break;
      }
    } else if (memcmp(i5_config.curWSCMac, pinterface->InterfaceId,
      sizeof(i5_config.curWSCMac)) == 0) {
      /* Current WSC M1 has sent for this interface so, choose next interface to send M1 */
      continue;
    } else if (!pinterface->isConfigured) {
      /* Interface is not configured so M1 has to be sent for this interface */
      break;
    }
  }

  if (pinterface) {
    i5Trace("curWSCMac["I5_MAC_DELIM_FMT"] and Interface["I5_MAC_DELIM_FMT"] is not configured\n",
            I5_MAC_PRM(i5_config.curWSCMac), I5_MAC_PRM(pinterface->InterfaceId));
    freqband = pinterface->band;
    if (freqband & BAND_2G) {
      freqband = 1;
      i5Trace("freqband[%d] for "I5_MAC_DELIM_FMT" \n",freqband, I5_MAC_PRM(pinterface->InterfaceId));
    } else if (freqband & (BAND_5GL | BAND_5GH)) {
      freqband = 2;
      i5Trace("freqband[%d] for "I5_MAC_DELIM_FMT" \n",freqband, I5_MAC_PRM(pinterface->InterfaceId));
    }

   memcpy(i5_config.curWSCMac, pinterface->InterfaceId, sizeof(i5_config.curWSCMac));
   i5WlCfgCreateAndSendM1(psock, neighbor_al_mac_address, freqband, pinterface->InterfaceId);
  } else {
    i5Trace("curWSCMac["I5_MAC_DELIM_FMT"] and pinterface is NULL so continue\n",
            I5_MAC_PRM(i5_config.curWSCMac));
    memset(i5_config.curWSCMac, 0, sizeof(i5_config.curWSCMac));
    goto configstart;
  }

  return 0;
}

/* Process the Ap autoconfiguration search and response */
int i5WlCfgProcessAPAutoConfigSearch(i5_message_type *pmsg, unsigned int freqband,
  unsigned char *searcher_al_mac_address)
{
  i5_dm_device_type *pdevice;

  if (pmsg == NULL) {
      i5TraceInfo("pmsg is empty\n");
      return -1;
  }

  i5Trace("MessageType[%d] Registrar[%d] freqband[%d]\n", i5MessageTypeGet(pmsg),
    I5_IS_REGISTRAR(i5_config.flags), freqband);

  /* If registrar process the AP-autoconfiguration search message recieved from agent */
  if (I5_IS_REGISTRAR(i5_config.flags)) {
    /* If the device is not discovered yet, send topology query find */
    pdevice = i5DmDeviceFind(searcher_al_mac_address);
    if (pdevice == NULL) {
      i5MessageTopologyQuerySend(pmsg->psock, searcher_al_mac_address);
      i5Trace("Device["I5_MAC_DELIM_FMT"] Not detected. Dont send search response.\n",
        I5_MAC_PRM(searcher_al_mac_address));
      return 0;
    }

    time(&pdevice->active_time);
    i5MessageApAutoconfigurationResponseSend(pmsg, freqband, searcher_al_mac_address);
  } else {
    if (i5_config.ptmrApSearch) {
      i5TimerFree(i5_config.ptmrApSearch);
      i5_config.ptmrApSearch = NULL;
    }

    if(i5_config.cbs.set_bh_sta_params) {
      i5_dm_interface_type *pdmif;

      pdmif = i5DmInterfaceFind(i5DmGetSelfDevice(), pmsg->psock->u.sll.mac_address);
      if (pdmif) {
        if (i5DmIsInterfaceWireless(pdmif->MediaType)) {
          i5Trace("Bakchaul interface %s ["I5_MAC_DELIM_FMT"] is wireless."
            "Enable Backhaul STA roaming\n", pdmif->ifname, I5_MAC_PRM(pdmif->InterfaceId));
          i5_config.cbs.set_bh_sta_params(IEEE1905_BH_STA_ROAM_ENAB_VAP_FOLLOW);
        } else {
          i5Trace("Bakchaul interface %s ["I5_MAC_DELIM_FMT"] is not wireless."
          "Disable Backhaul STA roaming\n", pdmif->ifname, I5_MAC_PRM(pdmif->InterfaceId));
          i5_config.cbs.set_bh_sta_params(IEEE1905_BH_STA_ROAM_DISB_VAP_UP);
        }
      }
    }

    /* Set controller psock */
    pdevice = i5DmDeviceFind(i5MessageSrcMacAddressGet(pmsg));
    if (pdevice != NULL) {
      pdevice->psock = pmsg->psock;
      time(&pdevice->active_time);
    }

    /* If controller is found already, don't continue */
    if (i5_config.flags & I5_CONFIG_FLAG_CONTROLLER_FOUND)
      return 0;

    /* If agent process the AP-autoconfiguration response from controller */
    i5_config.flags |= I5_CONFIG_FLAG_CONTROLLER_FOUND;

    /* Send M1 for unconfigured radio */
    i5WlCfgSendM1ForUnConfiguredRadio(pmsg->psock, i5MessageSrcMacAddressGet(pmsg));
  }

  return 0;
}

/* Timeout after sending M1 */
void i5WlCfgMultiApWSCTimeout(void *arg)
{
  i5_dm_device_type *pdeviceController;
  i5Trace("Multi AP WSC Timeout\n");

  if (i5_config.ptmrWSC != NULL ) {
    i5TimerFree(i5_config.ptmrWSC);
    i5_config.ptmrWSC = NULL;
  }

  /* Set the currently sent M1 to NULL. So that it should not accept */
  memset(i5_config.curWSCMac, 0, sizeof(i5_config.curWSCMac));

  pdeviceController = i5DmFindController();
  if (pdeviceController == NULL || pdeviceController->psock == NULL) {
    /* Controller not found search again */
    i5WlCfgMultiApControllerSearch(NULL);

    return;
  }

  /* Controller found send M1 */
  i5WlCfgSendM1ForUnConfiguredRadio(pdeviceController->psock, pdeviceController->DeviceId);
}

/* Generate all M2 from BSS info table based on the radio capability supported */
static int i5WlCfgGenerateAllM2s(unsigned char *almac, ieee1905_radio_caps_type *RadioCaps,
  unsigned int wcs_band, unsigned char *m1, int m1_len, int *if_band, unsigned char *ifrMapFlags)
{
  dll_t *item_p, *next_p;
  ieee1905_client_bssinfo_type *list;
  unsigned int rindex = 0;
  unsigned char count;
  unsigned char *m2;
  int m2_len;
  unsigned char tmpALMac[MAC_ADDR_LEN], found = 0;
  int band_in_use;

  *if_band = BAND_INV;
  if (!RadioCaps->Valid) {
    i5TraceError("RadioCaps not valid\n");
    return 0;
  }

  /* Number of Operating classes supported for the radio */
  count = RadioCaps->List ? RadioCaps->List[0] : 0;
  rindex++;
  if (count <= 0) {
    i5TraceError("Operating classes supported for this radio is 0\n");
    return 0;
  }

  *if_band = band_in_use = ieee1905_get_band_from_radiocaps(RadioCaps);
  i5TraceInfo("band from radiocaps: 0x%x\n", band_in_use);

  if (band_in_use == BAND_INV) {
    i5TraceError("Failed to get band from Radio capabilities\n");
    return 0;
  }

  /* Just to handle the case where the BSS info for agent is not specified. In that case
   * Try to provide the default set
   */
  if (!i5WlCfgIsALMACPresentInControllerTable(almac)) {
    memset(tmpALMac, 0, sizeof(tmpALMac));
  } else {
    memcpy(tmpALMac, almac, sizeof(tmpALMac));
  }

  /* For each BSS info in controller table check the matching AL MAC address */
  for (item_p = dll_head_p(&i5_config.client_bssinfo_list.head);
    !dll_end(&i5_config.client_bssinfo_list.head, item_p);
    item_p = next_p) {

    next_p = dll_next_p(item_p);
    list = (ieee1905_client_bssinfo_type*)item_p;
    i5TraceInfo("Bss Info: ALID["I5_MAC_DELIM_FMT"] band_flag: [0x%x]\n",
      I5_MAC_PRM(list->ALID), list->band_flag);
    /* if the bands got from radio capabilities not matching this bss band flag, skip it */
    if (!(band_in_use & list->band_flag)) {
      continue;
    }

    /* If the MAX BSS supported exceeds stop it */
    if (i5_config.m2_count >= RadioCaps->maxBSSSupported) {
      goto end;
    }

    if (memcmp(list->ALID, tmpALMac, sizeof(list->ALID)) == 0) {

      i5Trace("M2 For ALID["I5_MAC_DELIM_FMT"] band_flag[0x%x] ssid_len[%d] ssid[%s] auth[0x%x] "
        "encr[0x%x] pwd_len[%d] Password[%s] bkhaul[%d] frnthaul[%d]\n",
        I5_MAC_PRM(list->ALID), list->band_flag, list->ssid.SSID_len,
        list->ssid.SSID, list->AuthType, list->EncryptType,
        list->NetworkKey.key_len, list->NetworkKey.key, list->BackHaulBSS, list->FrontHaulBSS);
      found = 1;
      if (Wlcfg_proto_create_m2(wcs_band, m1, m1_len, list, &m2, &m2_len) == 1) {
        i5DmM2New(m2, m2_len);
        (*ifrMapFlags) |= (list->BackHaulBSS ? IEEE1905_MAP_FLAG_BACKHAUL : 0);
        (*ifrMapFlags) |= (list->FrontHaulBSS ? IEEE1905_MAP_FLAG_FRONTHAUL : 0);
        (*ifrMapFlags) |= (list->Guest ? IEEE1905_MAP_FLAG_GUEST : 0);
        i5Trace("band[%d] M2 Created Total[%d] ifrMapFlags[%x]\n", list->band_flag,
          i5_config.m2_count, *ifrMapFlags);
      }
    }
  }

  /* If no BSS is matching send Tear down for this radio */
  if (!found) {
    ieee1905_client_bssinfo_type teardown_bss;

    memset(&teardown_bss, 0, sizeof(teardown_bss));
    teardown_bss.TearDown = 1;

    if (Wlcfg_proto_create_m2(wcs_band, m1, m1_len, &teardown_bss, &m2, &m2_len) == 1) {
      i5DmM2New(m2, m2_len);
      i5Trace("Teardown M2 Created. M2 Count %d\n", i5_config.m2_count);
    }
  }

end:
  return i5_config.m2_count;
}

/* Process the Ap autoconfiguration WSC M1 message */
int i5WlCfgProcessAPAutoConfigWSCM1(i5_message_type *pmsg, i5_dm_device_type *pdevice,
  unsigned char *pWscData, int wscDataLen, unsigned char *radioMac,
  ieee1905_radio_caps_type *RadioCaps)
{
  int msgType;
  wsc_data_t wscDetails;
  int if_band = BAND_INV;
  i5_dm_interface_type *pdmif;
  unsigned char ifrMapFlags = 0;

  if ((pmsg == NULL) || (pWscData == NULL) || (RadioCaps == NULL)) {
    i5TraceInfo("pmsg or WSC Data or RadioCaps empty\n");
    return -1;
  }

  msgType = i5MessageTypeGet(pmsg);
  i5Trace("MessageType[%d], Registrar[%d] radioMac[" I5_MAC_DELIM_FMT"]\n", msgType,
    I5_IS_REGISTRAR(i5_config.flags), I5_MAC_PRM(radioMac));

  if (!I5_IS_REGISTRAR(i5_config.flags)) {
    i5TraceError("Not Registrar. Dont process\n");
    return -1;
  }

  wlcfg_wsc_get_data(pWscData, wscDataLen, &wscDetails);

  i5DmM2ListFree();

  if (i5WlCfgGenerateAllM2s(i5MessageSrcMacAddressGet(pmsg), RadioCaps,
    wscDetails.rf_band, pWscData, wscDataLen, &if_band, &ifrMapFlags) > 0) {
    i5Trace("Total M2s[%d] For ["I5_MAC_DELIM_FMT"] ifrMapFlags[%x]\n", i5_config.m2_count,
      I5_MAC_PRM(radioMac), ifrMapFlags);
    i5MessageApAutoconfigurationWscM2Send(pmsg->psock, i5MessageSrcMacAddressGet(pmsg), radioMac);
  }

  /* Get the interface, update the band and MAP Flags */
  pdmif = i5DmInterfaceFind(pdevice, radioMac);
  if (pdmif) {
    pdmif->band = (unsigned char)if_band;
    pdmif->mapFlags = ifrMapFlags;
    i5DmUpdateMAPFlagsFromControllerBSSTable(pdevice, pdmif);
  }

  if (i5_config.cbs.ap_configured) {
    i5_config.cbs.ap_configured(i5MessageSrcMacAddressGet(pmsg), radioMac, if_band);
  }
  i5DmM2ListFree();

  return 0;
}

/* Process the Ap autoconfiguration WSC M2 message */
int i5WlCfgProcessAPAutoConfigWSCM2(i5_message_type *pmsg, unsigned char *radioMac,
  ieee1905_vendor_data *msg_data)
{
  i5_wsc_m2_type *m2s;
  ieee1905_client_bssinfo_type *bssinfo;
  ieee1905_glist_t bssinfo_list; /* List of type ieee1905_client_bssinfo_type */
  i5_dm_device_type *pdmdev;
  i5_dm_interface_type *pdmif;
  int ret = 0, msgType;
  ieee1905_radio_caps_type *RadioCaps = NULL;
  int if_band = BAND_INV;

  /* Initialize list of BSS info */
  ieee1905_glist_init(&bssinfo_list);

  if (i5_config.ptmrWSC) {
    i5TimerFree(i5_config.ptmrWSC);
    i5_config.ptmrWSC = NULL;
  }

  if (pmsg == NULL) {
    i5TraceError("pmsg NULL\n");
    ret = -1;
    goto end;
  }

  if (i5_config.m2_count <= 0) {
    i5TraceError("No M2 Messages\n");
    ret = -1;
    goto end;
  }

  msgType = i5MessageTypeGet(pmsg);
  i5Trace("MessageType[%d], Registrar[%d] radioMac[" I5_MAC_DELIM_FMT"]\n", msgType,
    I5_IS_REGISTRAR(i5_config.flags), I5_MAC_PRM(radioMac));

  if (memcmp(i5_config.curWSCMac, radioMac, sizeof(i5_config.curWSCMac)) != 0) {
    /* unexpected message - ignore */
    i5TraceInfo("Unexpected Current MAC[" I5_MAC_DELIM_FMT"] radioMac[" I5_MAC_DELIM_FMT"]\n",
      I5_MAC_PRM(i5_config.curWSCMac), I5_MAC_PRM(radioMac));
    ret = -1;
    goto end;
  }

  pdmdev = i5DmGetSelfDevice();
  if ( NULL == pdmdev ) {
    i5TraceError("Local Device Not Found\n");
    ret = -1;
    goto end;
  }
  pdmif = i5DmInterfaceFind(pdmdev, radioMac);
  if (pdmif == NULL) {
    i5TraceError("RadioMac["I5_MAC_DELIM_FMT"] Not Found\n", I5_MAC_PRM(radioMac));
    ret = -1;
    goto end;
  }

  /* Now process each M2 */
  m2s = (i5_wsc_m2_type*)i5_config.m2_list.ll.next;
  while (m2s != NULL) {
    /* Allocate memory for this bssinfo */
    bssinfo = (ieee1905_client_bssinfo_type *)malloc(sizeof(*bssinfo));
    if (!bssinfo) {
      i5TraceDirPrint("Insufficient memory\n");
      ret = -1;
      goto end;
    }
    memset(bssinfo, 0, sizeof(*bssinfo));
    if (Wlcfg_proto_process_m2(i5_config.keys, i5_config.m1, i5_config.m1_len,
      m2s->m2, m2s->m2_len, bssinfo) != 0) {
      i5TraceError("RadioMac["I5_MAC_DELIM_FMT"] Failed to configure\n", I5_MAC_PRM(radioMac));
      goto end;
    }

    i5TraceInfo("vendor msg: %s len: %d ssid len: %d ssid: %s \n", msg_data->vendorSpec_msg,
      msg_data->vendorSpec_len, bssinfo->ssid.SSID_len, bssinfo->ssid.SSID);
    if (msg_data->vendorSpec_msg && bssinfo->ssid.SSID) {
      int i, count, ssid_len;
      unsigned char *pbuf = msg_data->vendorSpec_msg;

      count = *pbuf++;
      for (i = 0; i < count; i++) {
        ssid_len = *pbuf++;

        if (ssid_len != bssinfo->ssid.SSID_len) {
	  pbuf += ssid_len;
          continue;
        }
        if (memcmp(pbuf, bssinfo->ssid.SSID, ssid_len) == 0 ) {
          i5Trace("Guest bss found\n");
          bssinfo->Guest = 1;
        }
	pbuf += ssid_len;
      }
    }

    /* Append this BSS info obj to list, for this M2 TLV */
    ieee1905_glist_append(&bssinfo_list, (dll_t*)bssinfo);

    /* If we get tear down no need to process further */
    if (bssinfo->TearDown == 1) {
      break;
    }
    m2s = m2s->ll.next;
  }

  if ((ret = i5_config.cbs.create_bss_on_ifr(pdmif->ifname, &bssinfo_list)) == 0) {
    i5_config.isNewBssCreated = 1;
  }

  /* Free memory and cleanup bssinfo_list */
  i5DmGlistCleanup(&bssinfo_list);

  pdmif->flags |= I5_FLAG_IFR_M2_RECEIVED;
  /* If a new bss created and M1 is sent to all interfaces at least once, restart */
  if (i5_config.isNewBssCreated && i5DmIsM1SentToAllWirelessInterfaces()) {
    i5_config.cbs.create_bss_on_ifr(pdmif->ifname, NULL);
  }

  if (i5_config.m1 != NULL && i5_config.keys != NULL) {
    wlcfg_wsc_free(i5_config.m1, i5_config.keys);
    i5_config.m1 = NULL;
    i5_config.keys = NULL;
  }
  /* set wl interface to configured */
  pdmif->isConfigured = 1;

  /* If Agent's wl interface is configured, Callback to Vendor Specific Requests */
  if (I5_IS_MULTIAP_AGENT(i5_config.flags)) {

     RadioCaps = &pdmif->ApCaps.RadioCaps;
     if_band = ieee1905_get_band_from_radiocaps(RadioCaps);

     if (i5_config.cbs.ap_configured) {
       i5_config.cbs.ap_configured(i5MessageSrcMacAddressGet(pmsg), radioMac, if_band);
     }
  }

  /* Send M1 for next interface if not configured */
  i5WlCfgSendM1ForUnConfiguredRadio(pmsg->psock, i5MessageSrcMacAddressGet(pmsg));
  return 0;

end:
  /* Free memory and cleanup bssinfo_list */
  i5DmGlistCleanup(&bssinfo_list);

  /* Something failed, so start the WSC process */
  i5_config.ptmrWSC = i5TimerNew(I5_MESSAGE_AP_SEARCH_START_INTERVAL_MSEC,
    i5WlCfgMultiApWSCTimeout, NULL);
  return ret;
}
#else /* MULTIAP */
int i5WlcfgApAutoConfigProcessMessage( i5_message_type *pmsg, unsigned int freqband,
	unsigned char *pWscData, int wscDataLen, unsigned char *radioMac)
{
    i5_dm_device_type  *pdevice = i5DmGetSelfDevice();
    apSearchSendStruct *pApSearch = &i5_config.apSearch;
    int                 rc;
    int                 msgType;
    unsigned char      *pData;
    int                 dataLen;
    int                 matchingIf;
    char                ifname[I5_MAX_IFNAME];
    char                ifname_tmp[I5_MAX_IFNAME];

    if ( (NULL == pmsg) || (NULL == pdevice) ) {
      i5TraceInfo("pmsg and pdevice empty\n");
      return -1;
    }

    if (0 == wps1905ControlSocketReady())
    {
        i5WlcfgApAutoConfigTimer(pApSearch);
        return -1;
    }

    msgType = i5MessageTypeGet(pmsg);
    if (radioMac)
	    i5Trace(" msg=%d, reg %d freqband=%d radioMac " I5_MAC_DELIM_FMT" \n", msgType, I5_IS_REGISTRAR(i5_config.flags), freqband, I5_MAC_PRM(radioMac));
    else
	    i5Trace(" msg=%d, reg %d freqband=%d\n", msgType, I5_IS_REGISTRAR(i5_config.flags), freqband);

    if (I5_IS_REGISTRAR(i5_config.flags))
    {
        if ( WPS_1905_WSCAP_CONFIGURED == wps1905WscStatus(0) )
        {
            switch (msgType)
            {
                case i5MessageApAutoconfigurationSearchValue:
                {
                    if (0 == i5_config.freqBandEnable[freqband] )
                    {
                        i5Trace("freqband[%d] Not enabled\n", freqband);
                        return -1;
                    }

                    matchingIf = i5DmIsWifiBandSupported(&ifname[0], freqband);

                    /* AP autoconfiguration is enabled for the provided band
                       if matchingif is 0 then respond using a different band if available */
                    if ( 0 == matchingIf )
                    {
                        i5_dm_interface_type *pinterface = pdevice->interface_list.ll.next;
                        while ( pinterface )
                        {
                            if ( i5DmIsInterfaceWireless(pinterface->MediaType) &&
                                 i5WlCfgIsApConfigured(pinterface->wlParentName) &&
                                 (WPS_1905_WSCAP_ENABLED == wps1905WscEnabled(pinterface->wlParentName)))
                            {
                                freqband = i5TlvGetFreqBandFromMediaType(pinterface->MediaType);
                                break;
                            }
                            pinterface = pinterface->ll.next;
                        }
                        if ( NULL == pinterface )
                        {
                            i5TraceInfo("pinterface NULL\n");
                            return -1;
                        }
                    }
                    else if ( i5WlCfgIsApConfigured(ifname) )
                    {
                        if ( WPS_1905_WSCAP_ENABLED != wps1905WscEnabled(ifname) )
                        {
                            i5TraceInfo("WSC is not enabled on interface %s matching band %d\n", ifname, freqband);
                            return -1;
                        }
                    }
                    i5MessageApAutoconfigurationResponseSend(pmsg, freqband);
                    break;
                }

                case i5MessageApAutoconfigurationWscValue:
                {
                    int mtype;
                    int rfband;

                    rc = wps1905GetMessageInfo(pWscData, wscDataLen, &mtype, &rfband);
                    if ( (rc != 0) || (mtype != WPS_ID_MESSAGE_M1) ) {
                        i5TraceInfo("Not M1 in Registrar\n");
                        return -1;
                    }

                    if ( (rfband < WPS_RFBAND_24GHZ) || (rfband > WPS_RFBAND_50GHZ) )
                    {
                        i5TraceInfo("Band %d not supported\n", rfband);
                        return -1;
                    }
                    /* convert rfband to i5MessageFreqBand */
                    rfband--;

                    if (0 == i5_config.freqBandEnable[rfband] )
                    {
                        i5TraceInfo("rfband[%d] not enabled\n", rfband);
                        return -1;
                    }

                    matchingIf = i5DmIsWifiBandSupported(&ifname[0], rfband);
                    if (i5DmGetIfnameFromMediaSpecific(&ifname_tmp[0],
                      i5MessageSrcMacAddressGet(pmsg), radioMac) == 0) {
                      I5STRNCPY(ifname, ifname_tmp, sizeof(ifname));
                    }

                    i5Trace(" msg=%d, reg %d freqband=%d radioMac " I5_MAC_DELIM_FMT" ifname[%s]\n",
                      msgType, I5_IS_REGISTRAR(i5_config.flags), rfband, I5_MAC_PRM(radioMac), ifname);
                    if ( 0 == matchingIf )
                    {
                        i5_dm_interface_type *pinterface = pdevice->interface_list.ll.next;
                        while ( pinterface )
                        {
                            if ( i5DmIsInterfaceWireless(pinterface->MediaType) &&
                                 i5WlCfgIsApConfigured(pinterface->wlParentName) &&
                                 (WPS_1905_WSCAP_ENABLED == wps1905WscEnabled(pinterface->wlParentName)) )
                            {
                                rfband = i5TlvGetFreqBandFromMediaType(pinterface->MediaType);
                                strncpy(ifname, pinterface->wlParentName, I5_MAX_IFNAME);
                                ifname[I5_MAX_IFNAME-1] = '\0';
                                break;
                            }
                            pinterface = pinterface->ll.next;
                        }

                        if ( NULL == pinterface )
                        {
                            i5TraceInfo("pinterface NULL\n");
                            return -1;
                        }
                    }
                    else if (!i5WlCfgIsApConfigured(ifname))
                    {
                        i5TraceInfo("band not supported or wl not configured\n");
                        return -1;
                    }

                    if ( WPS_1905_WSCAP_ENABLED != wps1905WscEnabled(ifname) )
                    {
                        i5TraceInfo("WSC is not enabled on interface %s matching band %d\n", ifname, rfband);
                        return -1;
                    }
                    wps1905StopApAutoConf();
                    rc = wps1905WscProcess(ifname, WPS_1905_CTL_GETM2, pWscData, wscDataLen, &pData, &dataLen);
                    if ( rc != WPS_1905_M1HANDLE_M2DATA ) {
                      i5TraceInfo("rc[%d] != WPS_1905_M1HANDLE_M2DATA[%d]\n", rc, WPS_1905_M1HANDLE_M2DATA);
                        return -1;
                    }
                    i5MessageApAutoconfigurationWscSend(pmsg->psock, i5MessageSrcMacAddressGet(pmsg), pData, dataLen, radioMac);
                    free(pData);
                    break;
                }

                default:
                    break;
            }
        }
    }
    else
    {
        apSearchEntry *pentry = pApSearch->activeSearchEntry;
        if ( (NULL == pentry) || (msgType != pentry->expectedMsg) )
        {
            /* unexpected message - ignore */
            i5TraceInfo("Unexpected\n");
            return -1;
        }

        switch ( msgType )
        {
            case i5MessageApAutoconfigurationResponseValue:
            {
                unsigned char ifr_mac[MAC_ADDR_LEN];
                /* band does not match active entry */
                if ( freqband != pentry->freqBand )
                {
                    i5TraceInfo("freqband[%d] != pentry->freqBand[%d]\n", freqband, pentry->freqBand);
                    return -1;
                }

                /* clear active session */
                wps1905StopApAutoConf();
                i5Trace("Fetch M1 ifname[%s]\n", pentry->ifname);
                /* Fetch M1 from Wireless */
                rc = wps1905WscProcess(pentry->ifname, WPS_1905_CTL_STARTAUTOCONF, NULL, 0, &pData, &dataLen);
                if (WPS_1905_RESHANDLE_M1DATA != rc)
                {
                    i5TraceInfo("rc[%d] != WPS_1905_RESHANDLE_M1DATA[%d]\n", rc, WPS_1905_RESHANDLE_M1DATA);
                    return -1;
                }
                pentry->expectedMsg = i5MessageApAutoconfigurationWscValue;

                i5GetInterfaceIDFromIfname(pentry->ifname, ifr_mac);
                i5Trace("Send M1 freqband[%d] Ifname[%s] MAC "I5_MAC_DELIM_FMT"\n", freqband,
                  pentry->ifname, I5_MAC_PRM(ifr_mac));
                i5MessageApAutoconfigurationWscSend(pmsg->psock, i5MessageSrcMacAddressGet(pmsg), pData, dataLen, ifr_mac);
                free(pData);
                if ( pApSearch->timer )
                {
                    i5TimerFree(pApSearch->timer);
                }

                pApSearch->timer = i5TimerNew(I5_MESSAGE_AP_SEARCH_M1_M2_WAITING_MSEC, i5WlcfgApAutoConfigTimer, pApSearch);
                break;
            }

            case i5MessageApAutoconfigurationWscValue:
            {
                int mtype;
                int rfband;

                rc = wps1905GetMessageInfo(pWscData, wscDataLen, &mtype, &rfband);
                if ( (rc != 0) || (mtype != WPS_ID_MESSAGE_M2) )
                {
                    i5TraceInfo("rc[%d] mtype[%d] != %d\n", rc, mtype, WPS_ID_MESSAGE_M2);
                    return -1;
                }

                if ( (rfband < WPS_RFBAND_24GHZ) || (rfband > WPS_RFBAND_50GHZ) )
                {
                    i5TraceInfo("rfband[%d] not supported\n", rfband);
                    return -1;
                }
                /* convert rfband to i5MessageFreqBand */
                rfband--;

                if ( pentry->freqBand != rfband )
                {
                    i5TraceInfo("pentry->freqBand[%d] != rfband[%d]\n", pentry->freqBand, rfband);
                    return -1;
                }

                i5Trace("Set M2 ifname[%s]\n", pentry->ifname);
                rc = wps1905WscProcess(pentry->ifname, WPS_1905_CTL_CONFIGUREAP, pWscData, wscDataLen, &pData, &dataLen);
                if (rc != WPS_1905_M2SET_DONE)
                {
                    i5WlcfgApAutoConfigTimer(pApSearch);
                    i5TraceInfo("rc[%d] != WPS_1905_M2SET_DONE[%d]\n", rc, WPS_1905_M2SET_DONE);
                    return -1;
                }

                /* set wl interface to configured */
                i5WlcfgApAutoconfigurationWscValueDone(pentry);
                break;
            }

            default:
                break;
        }
    }

    return 0;
}
#endif /* MULTIAP */

#ifdef MULTIAP
void i5WlCfgMultiApControllerSearch(void *arg)
{
    unsigned char *ifr_mac = (unsigned char*)arg;
    i5_dm_device_type *device;
    i5_dm_interface_type *interface;

    i5Trace("Multi AP Controller search Timeout\n");

    /* Before starting the search, kill the WSC message timer */
    if (i5_config.ptmrWSC  != NULL ) {
        i5TimerFree(i5_config.ptmrWSC );
        i5_config.ptmrWSC  = NULL;
    }

    /* Before starting the search, kill the controller search timer */
    if (i5_config.ptmrApSearch != NULL ) {
        i5TimerFree(i5_config.ptmrApSearch);
        i5_config.ptmrApSearch = NULL;
    }

    if (!I5_IS_START_MESSAGE(i5_config.flags)) {
      i5Trace("Messaging not started\n");
      return;
    }

    if (I5_IS_REGISTRAR(i5_config.flags)) {
        i5Trace(I5_MAC_DELIM_FMT" Registrar Quit\n", I5_MAC_PRM(i5_config.i5_mac_address));
        return;
    }

    /* If a new bss created and M1 is sent to all interfaces at least once, restart */
    if (i5_config.isNewBssCreated && i5DmIsM1SentToAllWirelessInterfaces()) {
      i5_config.cbs.create_bss_on_ifr(NULL, NULL);
    }

    /* make it as cotroller not found */
    i5_config.flags &= ~I5_CONFIG_FLAG_CONTROLLER_FOUND;

    /* If the MAC is provided, change it to unconfigured */
    if (ifr_mac != NULL) {
      i5Trace(I5_MAC_DELIM_FMT" MAC Provided. Make it unconfigured\n", I5_MAC_PRM(ifr_mac));
      if ((device = i5DmGetSelfDevice()) != NULL) {
        if ((interface = i5DmInterfaceFind(device, ifr_mac)) != NULL) {
          interface->isConfigured = 0;
        }
      }
    }

    /* Make all virtual interfaces as configured */
    i5DmPreConfigureVirtualInterfaces();
    i5MessageApAutoconfigurationSearchSend(I5_FREQ_BAND_2G);

    i5_config.ptmrApSearch = i5TimerNew(I5_MESSAGE_AP_SEARCH_START_INTERVAL_MSEC,
        i5WlCfgMultiApControllerSearch, NULL);
}
#else
void i5WlcfgApAutoconfigurationStart(const char *ifname)
{
    apSearchSendStruct   *pApSearch = &i5_config.apSearch;
    apSearchEntry        *pentry;
    i5_dm_device_type    *pdevice;
    i5_dm_interface_type *pinterface;

    i5Trace("\n");

    if ( 0 == wps1905ControlSocketReady() )
    {
        return;
    }

    pdevice = i5DmGetSelfDevice();
    if ( NULL == pdevice)
    {
        return;
    }

    pinterface = pdevice->interface_list.ll.next;
    while ( pinterface )
    {
        if ( i5DmIsInterfaceWireless(pinterface->MediaType) &&
             ((NULL == ifname) ||
              (0 == strcmp(ifname, pinterface->wlParentName))) )
        {
            int freqBand = i5TlvGetFreqBandFromMediaType(pinterface->MediaType);
            if ( (!I5_IS_REGISTRAR(i5_config.flags)) &&
                 (0 == i5WlCfgIsApConfigured(pinterface->wlParentName)) &&
                 (WPS_1905_WSCAP_ENABLED == wps1905WscEnabled(pinterface->wlParentName)) &&
                 (i5MessageFreqBand_Reserved != freqBand) && (1 == i5_config.freqBandEnable[freqBand]) )
            {
                /* add interface to the search list if it is not already present */
                pentry = pApSearch->searchEntryList.ll.next;
                while ( pentry != NULL )
                {
                   if ( 0 == strcmp(pinterface->wlParentName, pentry->ifname) )
                   {
                       break;
                   }
                   pentry = pentry->ll.next;
                }
                if ( NULL == pentry )
                {
                    pentry = (apSearchEntry *)malloc(sizeof(apSearchEntry));
                    if (pentry)
                    {
                        memset(pentry, 0, sizeof(apSearchEntry));
                        strncpy(pentry->ifname, pinterface->wlParentName, I5_MAX_IFNAME-1);
                        pentry->ifname[I5_MAX_IFNAME-1] = '\0';
                        pentry->freqBand = i5TlvGetFreqBandFromMediaType(pinterface->MediaType);
                        pentry->expectedMsg = i5MessageApAutoconfigurationResponseValue;
                        i5Trace("adding interface %s to the ap search list - band %d\n", pinterface->wlParentName, pentry->freqBand);
                        i5LlItemAdd(NULL, &pApSearch->searchEntryList, pentry);
                    }
                }
            }
        }
        pinterface = pinterface->ll.next;
    }

    pentry = pApSearch->searchEntryList.ll.next;
    while ( pentry != NULL )
    {
        if ( i5MessageApAutoconfigurationResponseValue == pentry->expectedMsg )
        {
            pentry->callCounter = 0;
        }
        pentry = pentry->ll.next;
    }

    if ( (i5_config.apSearch.timer == NULL) ||
         ((pApSearch->activeSearchEntry != NULL) &&
          (i5MessageApAutoconfigurationResponseValue == pApSearch->activeSearchEntry->expectedMsg)) )
    {
        i5WlcfgApAutoConfigTimer(pApSearch);
    }
}
#endif /* MULTIAP */

void i5WlcfgApAutoconfigurationStop(char const *ifname)
{
#ifndef MULTIAP
    apSearchSendStruct *pApSearch = &i5_config.apSearch;
    apSearchEntry *pentry = pApSearch->searchEntryList.ll.next;
    apSearchEntry *pnext;

    i5Trace("\n");

    while ( pentry != NULL )
    {
       pnext = pentry->ll.next;
       if ( (NULL == ifname) || (0 == strcmp(ifname, pentry->ifname)) )
       {
           i5Trace("Removing entry for %s\n", pentry->ifname);
           i5LlItemRemove(&pApSearch->searchEntryList, pentry);
           if ( pentry == pApSearch->activeSearchEntry )
           {
               pApSearch->activeSearchEntry = NULL;
               i5WlcfgApAutoConfigTimer(pApSearch);
           }

           if ( ifname != NULL )
           {
               break;
           }
       }
       pentry = pnext;
    }
    /* active entry was removed so reconfigure the timer */
    if ( NULL == pApSearch->activeSearchEntry )
    {
       i5WlcfgApAutoConfigTimer(pApSearch);
    }
#endif /* MULTIAP */
}

#ifndef MULTIAP
int i5WlcfgApAutoconfigurationRenewProcess(i5_message_type *pmsg, unsigned int freqband,
  unsigned char *neighbor_al_mac_address)
{
    i5_dm_device_type *pdevice = i5DmGetSelfDevice();
    apSearchEntry *pentry;
    char           wlname[I5_MAX_IFNAME];
    int            matchingIf = i5DmIsWifiBandSupported(wlname, freqband);

    i5Trace(" band %d, match %d, name %s\n", freqband, matchingIf, (matchingIf ? wlname : "NULL"));

    if ( (NULL == pdevice) || (0 == matchingIf) )
    {
        return -1;
    }

    if (I5_IS_REGISTRAR(i5_config.flags))
    {
        i5Trace(" Device is registrar - ignore renew\n");
        return -1;
    }

    /* ignore message if configuration is already in progress */
    pentry = i5_config.apSearch.searchEntryList.ll.next;
    while ( pentry != NULL )
    {
       if ( 0 == strcmp(wlname, pentry->ifname) ) {
          break;
       }
       pentry = pentry->ll.next;
    }

    if ( pentry )
    {
        i5Trace(" Configuration already in progress\n");
        return -1;
    }

    if ( (WPS_1905_WSCAP_ENABLED == wps1905WscEnabled(wlname)) &&
         (1 == i5_config.freqBandEnable[freqband]) )
    {
        pentry = (apSearchEntry *)malloc(sizeof(apSearchEntry));
        if (pentry) {
            strncpy(pentry->ifname, wlname, I5_MAX_IFNAME-1);
            pentry->ifname[I5_MAX_IFNAME-1] = '\0';
            pentry->freqBand = freqband;
            pentry->renew = 1;
            pentry->expectedMsg = i5MessageApAutoconfigurationWscValue;
            strncpy(pentry->rxIfname, pmsg->psock->u.sll.ifname, I5_MAX_IFNAME-1);
            pentry->rxIfname[I5_MAX_IFNAME-1] = '\0';
            memcpy(pentry->registrarMac, neighbor_al_mac_address, MAC_ADDR_LEN);
            i5LlItemAdd(NULL, &i5_config.apSearch.searchEntryList, pentry);
            i5Trace(" Adding interface %s for renew\n", wlname);
        }

        if ( i5_config.apSearch.timer == NULL ) {
            i5WlcfgApAutoConfigTimer(&i5_config.apSearch);
        }
    }

    return 0;
}
#else /* MULTIAP */

/* make all the interface as not configured */
void i5WlcfgMarkAllInterfacesUnconfigured()
{
    i5_dm_device_type *pdevice = i5DmGetSelfDevice();
    i5_dm_interface_type *pdmif;

    pdmif = (i5_dm_interface_type *)pdevice->interface_list.ll.next;
    while (pdmif != NULL) {
      if (i5DmIsInterfaceWireless(pdmif->MediaType) && !i5WlCfgIsVirtualInterface(pdmif->ifname)) {
        pdmif->isConfigured = 0;
	pdmif->flags &= ~I5_FLAG_IFR_M1_SENT;
	pdmif->flags &= ~I5_FLAG_IFR_M2_RECEIVED;
      }
      pdmif = pdmif->ll.next;
    }
    i5_config.isNewBssCreated = 0;

  /* set current WSC MAC to NULL to make sure that it sends M1 for all interfaces */
  memset(i5_config.curWSCMac, 0, sizeof(i5_config.curWSCMac));
}

int i5WlcfgApAutoconfigurationRenewProcess(i5_message_type *pmsg, unsigned int freqband,
  unsigned char *neighbor_al_mac_address)
{
    i5_dm_device_type *pdevice = i5DmDeviceFind(neighbor_al_mac_address);

    i5Trace(" band %d\n", freqband);

    if (NULL == pdevice) {
        i5MessageTopologyQuerySend(pmsg->psock, neighbor_al_mac_address);
        i5TraceError("NO device found\n");
        return -1;
    }

    if (I5_IS_REGISTRAR(i5_config.flags)) {
        i5Trace(" Device is registrar - ignore renew\n");
        return -1;
    }

    time(&pdevice->active_time);

    /* make all the interface as not configured */
    i5WlcfgMarkAllInterfacesUnconfigured();

    if (pdevice->psock && (pdevice->psock != pmsg->psock)) {
      /* Renew received from different controller socket. Start with search */
      pdevice->psock = pmsg->psock;
      i5WlCfgMultiApControllerSearch(NULL);
    } else {
      /* Start the WSC message timer(M1) to renew all the interfaces */
      i5WlCfgMultiApWSCTimeout(NULL);
    }

    return 0;
}
#endif /* MULTIAP */

#if defined(DSLCPE_WLCSM_EXT)
void wlcsm_event_handler(t_WLCSM_EVENT_TYPE type,...) {
   va_list arglist;
   va_start(arglist,type);

   switch ( type ) {
      case WLCSM_EVT_NVRAM_CHANGED:
         {
            char * name=va_arg(arglist,char *);
            char * value=va_arg(arglist,char *);
            char * oldvalue=va_arg(arglist,char *);
            printf("--:%s:%d  nvram change received name:%s,value:%s,oldvalue:%s\r\n",__FUNCTION__,__LINE__ ,name,value,oldvalue);

         }
         break;
      case WLCSM_EVT_NVRAM_COMMITTED:
         printf("---:%s:%d  nvram committed \r\n",__FUNCTION__,__LINE__ );
         break;

      default:
         break;
   }

   va_end(arglist);
}
#endif // endif

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  i5ctlWlcfgHandler
 *  Description:  function handler to handle i5ctl command
 * =====================================================================================
 */
int i5ctlWlcfgHandler(i5_socket_type *psock, t_I5_API_WLCFG_MSG *pMsg)
{
    int status = 0;
    int cmd    = I5_API_CMD_WLCFG;

    i5Trace("sub command is %d, interface %s\n", pMsg->subcmd, pMsg->ifname);
    switch(pMsg->subcmd) {
#if defined(DSLCPE_WLCSM_EXT)
    case  I5_CTL_WLCFG_NVRAM_SET: {
        char buf[100];
        char *name,*value;
        strncpy(value = buf,(char *)(pMsg+1), 100);
        name = strsep(&value, "=");
        wlcsm_nvram_set(name, value);
        _i5WlcfgCtlResponse(psock,cmd,pMsg->subcmd,status,NULL,0);
    }
    break;
    case  I5_CTL_WLCFG_NVRAM_UNSET: {
        wlcsm_nvram_unset((char *)(pMsg+1));
        _i5WlcfgCtlResponse(psock,cmd,pMsg->subcmd,status,"OK",3);
    }
    break;
    case  I5_CTL_WLCFG_NVRAM_COMMIT: {
        wlcsm_nvram_commit();
        _i5WlcfgCtlResponse(psock,cmd,pMsg->subcmd,status,"OK",3);
    }
    break;
    case  I5_CTL_WLCFG_NVRAM_GET: {
        char *value=wlcsm_nvram_get((char *)(pMsg+1));
        if(value)
            _i5WlcfgCtlResponse(psock,cmd,pMsg->subcmd,status,value,strlen(value)+1);
        else
            _i5WlcfgCtlResponse(psock,cmd,pMsg->subcmd,status,NULL,0);
    }
    break;
    case I5_CTL_WLCFG_NVRAM_TRACE:
        wlcsm_set_trace_level((char *)(pMsg+1));
        _i5WlcfgCtlResponse(psock,cmd,pMsg->subcmd,status,"OK",3);
        break;
#endif // endif

#ifndef MULTIAP
    case  I5_CTL_WLCFG_WSCSTATUS:
        status=wps1905WscStatus();
        _i5WlcfgCtlResponse(psock,cmd,pMsg->subcmd,status,NULL,0);
        break;
    case  I5_CTL_WLCFG_WSCCANCEL:
        status=wps1905StopApAutoConf();
        _i5WlcfgCtlResponse(psock,cmd,pMsg->subcmd,status,NULL,0);
        break;
    case  I5_CTL_WLCFG_WSCENABLED:
        status=wps1905WscEnabled(pMsg->ifname);
        _i5WlcfgCtlResponse(psock,cmd,pMsg->subcmd,status,NULL,0);
        break;
    case  I5_CTL_WLCFG_WLINSTANCE:
        status=wps1905WlInstance();
        _i5WlcfgCtlResponse(psock,cmd,pMsg->subcmd,status,NULL,0);
        break;
#endif /* MULTIAP */
    case  I5_CTL_WLCFG_APISCONFIGURED:
        status=i5WlCfgIsApConfigured(pMsg->ifname);
        _i5WlcfgCtlResponse(psock,cmd,pMsg->subcmd,status,NULL,0);
        break;
    default:
        status=wlm_1905_i5ctl_handler(psock,pMsg);
        if(status<0)
        {
            i5Trace("Unsupported wlcfg sub command\n");
            _i5WlcfgCtlResponse(psock,cmd,pMsg->subcmd,1,"unsupported sub command",0);
        }
        else
            _i5WlcfgCtlResponse(psock,cmd,pMsg->subcmd,1,"success",0);
        break;
    }
    return status;
}

void i5WlCfgInit( void )
{
    int                sd;
    struct timeval     tv;

    sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if ( sd < 0 )
    {
        printf("i5WlCfgInit: failed to create udp socket\n");
        return;
    }

    tv.tv_sec  = WPS_1905_RECEIVE_TIMEOUT_MS / 1000;
    tv.tv_usec = (WPS_1905_RECEIVE_TIMEOUT_MS % 1000) * 1000;
    if (setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval)) < 0)
    {
        printf("could not set receive timeout socket option: errno %d\n", errno);
    }

    i5_config.wl_control_socket.psock = i5SocketNew(sd, i5_socket_type_udp, wps1905ProcessSocket);
    if ( NULL == i5_config.wl_control_socket.psock )
    {
        printf("i5WlCfgInit: failed to create i5_socket_type_udp\n");
        close(sd);
        return;
    }

    /* userspace address*/
    i5_config.wl_control_socket.psock->u.sinl.sa.sin_family      = AF_INET;
    i5_config.wl_control_socket.psock->u.sinl.sa.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(i5_config.wl_control_socket.psock->sd, (struct sockaddr *)&(i5_config.wl_control_socket.psock->u.sinl.sa), sizeof(struct sockaddr_in)) < 0)
    {
        printf("Failed to bind to udp receive socket: errno %d\n", errno);
        i5SocketClose(i5_config.wl_control_socket.psock);
        return;
    }

#ifndef MULTIAP
    i5_config.wl_control_socket.ptmr = i5TimerNew(WPS_1905_RECEIVE_TIMEOUT_MS, wps1905Register, &i5_config.wl_control_socket);
#endif /* MULTIAP */

#if defined(DSLCPE_WLCSM_EXT)
    wlcsm_register_event_generic_hook(wlcsm_event_handler);
    wlcsm_set_trace(3);
#endif // endif

}

void i5WlCfgDeInit( void )
{
#if defined(DSLCPE_WLCSM_EXT)
   wlcsm_shutdown();
#endif // endif
}

/* Get the AL MAC address */
int i5WlCfgGet1905MacAddress(unsigned int multiapMode, unsigned char *MACAddress)
{
  int i = 0;
  char *nvval = NULL;
  char strMAC[20] = {0};
  char *nvvar = I5_WLCFG_NVRAM_AL_MAC;

  if (I5_IS_MULTIAP_CONTROLLER(multiapMode)) {
    nvvar = I5_WLCFG_NVRAM_CTL_AL_MAC;
  }

  nvval = nvram_get(nvvar);
  if (nvval) {
    if (ether_atoe(nvval, MACAddress)) {
      return 0;
    }
  }

  /* Generate the AL MAC. The MAC address will be hardware address with local administrator
   * bit set plus 1
   */
  nvval = nvram_get(I5_WLCFG_NVRAM_LAN_HWADDR);
  ether_atoe(nvval, MACAddress);

  ETHER_SET_LOCALADDR(MACAddress);

  /* Increment the address by 1 */
  for (i = MAC_ADDR_LEN - 1; i >= 0; i--) {
    if (MACAddress[i] == 0xFF) {
      MACAddress[i] = 0;
    } else {
      MACAddress[i]++;
      break;
    }
  }

  if (I5_IS_MULTIAP_CONTROLLER(multiapMode)) {
    if (MACAddress[MAC_ADDR_LEN - 1] == 0xFF) {
      MACAddress[MAC_ADDR_LEN - 1] = 0;
    } else {
      MACAddress[MAC_ADDR_LEN - 1]++;
    }
  }

  ether_etoa(MACAddress, strMAC);
  nvram_set(nvvar, strMAC);
  nvram_commit();

  return 0;
}

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  wl_ioctl
 *  Description:  function to ioctl to wl driver
 * =====================================================================================
 */
int wl_ioctl(char *name, int cmd, void *buf, int len)
{
    struct ifreq ifr;
    wl_ioctl_t ioc;
    int ret = 0;
    int s;
    /*  open socket to kernel */
    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket");
        return errno;
    }
    ioc.cmd = cmd;
    ioc.buf = buf;
    ioc.len = len;
    /*  initializing the remaining fields */
    ioc.set = 0;
    ioc.used = 0;
    ioc.needed = 0;
    strncpy(ifr.ifr_name, name, I5_MAX_IFNAME);
    ifr.ifr_data = (caddr_t) &ioc;
    if ((ret = ioctl(s, SIOCDEVPRIVATE, &ifr)) < 0)
        printf("%s:%d  wl_ioctl error:%d \r\n",__FUNCTION__,__LINE__,ret );
    close(s);
    return ret;
}

/* Get the interface mode sta/ap */
char *i5WlCfgGetNvVal(const char *ifname, const char *nvName)
{
  char tmp[32] = {0};

  if (ifname) {
    snprintf(tmp, sizeof(tmp), "%s_%s", ifname, nvName);
  } else {
    snprintf(tmp, sizeof(tmp), "%s", nvName);
  }

  return nvram_safe_get(tmp);
}

#ifdef MULTIAP
/* Get dot11CurrentChannelBandwidth from chanspec */
static unsigned char i5WlCfgDot11BWFromChanspec(unsigned short chanspec)
{
  if (CHSPEC_IS20(chanspec)) {
    return WL_WIDE_BW_CHAN_WIDTH_20;
  } else if (CHSPEC_IS40(chanspec)) {
    return WL_WIDE_BW_CHAN_WIDTH_40;
  } else if (CHSPEC_IS80(chanspec)) {
    return WL_WIDE_BW_CHAN_WIDTH_80;
  } else if (CHSPEC_IS160(chanspec)) {
    return WL_WIDE_BW_CHAN_WIDTH_160;
  } else if (CHSPEC_IS8080(chanspec)) {
    return WL_WIDE_BW_CHAN_WIDTH_80_80;
  }

  return WL_WIDE_BW_CHAN_WIDTH_20;
}

/* Get dot11CurrentChannelBandwidth from chanspec */
static unsigned int i5WlCfgBWFromDot11BW(unsigned char dot11bw)
{
  if (dot11bw == WL_WIDE_BW_CHAN_WIDTH_20) {
    return WL_CHANSPEC_BW_20;
  } else if (dot11bw == WL_WIDE_BW_CHAN_WIDTH_40) {
    return WL_CHANSPEC_BW_40;
  } else if (dot11bw == WL_WIDE_BW_CHAN_WIDTH_80) {
    return WL_CHANSPEC_BW_80;
  } else if (dot11bw == WL_WIDE_BW_CHAN_WIDTH_160) {
    return WL_CHANSPEC_BW_160;
  } else if (dot11bw == WL_WIDE_BW_CHAN_WIDTH_80_80) {
    return WL_CHANSPEC_BW_8080;
  }

  return WL_WIDE_BW_CHAN_WIDTH_20;
}

/* Create mediaspecific info */
void i5WlCfgCreateMediaInfo(unsigned char *InterfaceId, unsigned char *bssid,
  unsigned short chanspec, unsigned char mapflags, unsigned char *MediaSpecificInfo)
{
  unsigned char index = 0;

  /* if the interface is STA we should add BSSID of the bSTA */
  if (mapflags & IEEE1905_MAP_FLAG_STA) {
    if (bssid) {
      memcpy(MediaSpecificInfo, bssid, MAC_ADDR_LEN);
    }
    index += MAC_ADDR_LEN;
    MediaSpecificInfo[index++] = I5_MEDIA_INFO_ROLE_STA;
  } else {
    memcpy(MediaSpecificInfo, InterfaceId, MAC_ADDR_LEN);
    index += MAC_ADDR_LEN;
    MediaSpecificInfo[index++] = I5_MEDIA_INFO_ROLE_AP;
  }
  MediaSpecificInfo[index++] = i5WlCfgDot11BWFromChanspec(chanspec);
  MediaSpecificInfo[index++] = (unsigned char)wf_chspec_ctlchan(chanspec);
}

/* Creates and updates the media type and media specific info from chanspec */
void i5WlCfgUpdateMediaInfo(unsigned char *InterfaceId, unsigned short chanspec,
  unsigned char *bssid, unsigned char mapflags)
{
  i5_dm_device_type *pdmdev;
  i5_dm_interface_type *pdmif;

  /* Find the local Device */
  pdmdev = i5DmGetSelfDevice();
  if (pdmdev == NULL) {
    i5TraceError("Device " I5_MAC_FMT "Not Found\n", I5_MAC_PRM(i5_config.i5_mac_address));
    return;
  }

  /* Find the interface in the device */
  pdmif = i5DmInterfaceFind(pdmdev, InterfaceId);
  if (pdmif == NULL) {
    i5TraceError("Interface " I5_MAC_FMT "Not Found\n", I5_MAC_PRM(InterfaceId));
    return;
  }

  if (CHSPEC_BAND(chanspec) == WL_CHANSPEC_BAND_5G) {
    pdmif->MediaType = I5_MEDIA_TYPE_WIFI_AC;
  } else {
    pdmif->MediaType = I5_MEDIA_TYPE_WIFI_N24;
  }

  i5WlCfgCreateMediaInfo(InterfaceId, bssid, chanspec, mapflags, pdmif->MediaSpecificInfo);
  pdmif->MediaSpecificInfoSize = i5TlvMediaSpecificInfoWiFi_Length;
}

/* Get chanspec from Mediaspecific info */
void i5WlCfgGetChanspecFromMediaInfo(unsigned char *MediaSpecificInfo,
  unsigned short *outChanspec)
{
  unsigned char index = 0;
  unsigned char dot11bw = 0;
  unsigned char center_ch1 = 0;
  unsigned int bw = 0;
  unsigned short chanspec = 0;

  if (MediaSpecificInfo == NULL) {
    return;
  }

  index += 6; /* networkMembership */
  index++; /* role */
  dot11bw = MediaSpecificInfo[index++];
  center_ch1 = MediaSpecificInfo[index++];

  bw = i5WlCfgBWFromDot11BW(dot11bw);
  chanspec = wf_channel2chspec(center_ch1, bw);
  if (outChanspec) {
    *outChanspec = chanspec;
  }
}

static int
i5WlCfgIOVARGetBuf(char *ifname, char *iovar, void *param, int paramlen, void *bufptr, int buflen)
{
  int err;
  uint namelen;
  uint iolen;

  namelen = strlen(iovar) + 1;     /* length of iovar name plus null */
  iolen = namelen + paramlen;

  /* check for overflow */
  if (iolen > buflen)
    return -1;

  memcpy(bufptr, iovar, namelen); /* copy iovar name including null */
  memcpy((int8*)bufptr + namelen, param, paramlen);

  err = wl_ioctl(ifname, WLC_GET_VAR, bufptr, buflen);

  return (err);
}

/* Get Regulatory class of Interface */
int
i5WlCfgGetRclass(char* ifname, unsigned short chspec, unsigned short *out_rclass)
{
  int ret = 0;
  unsigned char buf[16] = {0};

  ret = i5WlCfgIOVARGetBuf(ifname, "rclass", &chspec, sizeof(chspec), buf, sizeof(buf));
  if (ret == 0) {
    *out_rclass = (*((unsigned short *)buf));
  }

  return 0;
}

/* Get chanspec of the interface */
int
i5WlCfgGetChanspec(char *ifname, chanspec_t *out_chanspec)
{
  int ret = 0;
  unsigned char buf[16] = {0};

  ret = i5WlCfgIOVARGetBuf(ifname, "chanspec", NULL, 0, buf, sizeof(buf));
  if (ret == 0)
  {
    *out_chanspec = (*((chanspec_t *)buf));
  }

  return ret;
}

int i5WlCfgIsInterfaceEnabled(char *ifname)
{
	char wlname[I5_MAX_IFNAME];
	char *wlparent, *nv;
	char nvname[50];

	wlparent = i5WlcfgGetWlParentInterface(ifname, &wlname[0]);
	if (!wlparent)
		return 0;

	snprintf(nvname, sizeof(nvname), "%s_bss_enabled", wlparent);
	nv = nvram_safe_get(nvname);
	if (strcmp(nv, "1") == 0)
		return 1;

	return 0;
}

void
i5WlCfgHandleWPSPBC(char *rfband, int mode)
{
  WPS_1905_MESSAGE   *pMsg = NULL;
  controlSockStruct  *pctrlsock = &i5_config.wl_control_socket;
  struct sockaddr_in  sockAddr;
  socklen_t           addrLen = sizeof(sockAddr);

  i5_dm_device_type *pdmdev;
  i5_dm_interface_type *pdmif = NULL;
  i5_dm_bss_type *item, *pbss = NULL, *pbss_fh = NULL;

  /* Find the interface matching the PBC band */
  if ((pdmdev = i5DmGetSelfDevice()) != NULL) {
    pdmif = (i5_dm_interface_type *)pdmdev->interface_list.ll.next;
    while (pdmif != NULL) {
      if (pdmif->MediaSpecificInfoSize <= 8)
        goto next;
      i5TraceInfo("ifname: %s channel: %d band: 0x%x\n", pdmif->ifname,
        pdmif->MediaSpecificInfo[8], pdmif->band);
      if ((strncmp("5GH", rfband, 3) == 0) && (pdmif->band & BAND_5GH)) {
        break;
      } else if ((strncmp("24G", rfband, 3) == 0) && (pdmif->band & BAND_2G)) {
        break;
      } else if ((strncmp("5GL", rfband, 3) == 0) && (pdmif->band & BAND_5GL)) {
        break;
      }
next:
      pdmif = pdmif->ll.next;
    }
  }

  if (!pdmif) {
    i5TraceError("No interface matching PBC band: %s\n", rfband);
    return;
  }

  if (mode == 1) { /* AP */
    /* Find a backhaul BSS in the current inteface */
    item = (i5_dm_bss_type *)pdmif->bss_list.ll.next;
    while (item != NULL) {
      i5TraceInfo("ifname:%s map: 0x%x\n", item->ifname, item->mapFlags);
      if (item->mapFlags & IEEE1905_MAP_FLAG_BACKHAUL) {
        if (!pbss) {
          pbss = item;
        }
        /* Check if is dedicated backhaul */
        if ((item->mapFlags & IEEE1905_MAP_FLAG_FRONTHAUL) == 0)
          break;
      }
      item = item->ll.next;
    }

    /* Find a Fronthaul bss in the current interface */
    item = (i5_dm_bss_type *)pdmif->bss_list.ll.next;
    while (item != NULL) {
      if ((item->mapFlags & IEEE1905_MAP_FLAG_FRONTHAUL) &&
        !(item->mapFlags & IEEE1905_MAP_FLAG_GUEST)) { /* No onboarding through Guest bss */
        pbss_fh = item;
        break;
      }
      item = item->ll.next;
    }

    if (!pbss || !pbss_fh) {
      i5TraceError("No bss configured as backhaul/fronthaul in band: %s\n", rfband);
      return;
    }
#ifdef CONFIG_HOSTAPD
    if (i5GlueIsHapdEnabled()) {
	    i5GlueWpsPbc(pbss_fh->ifname, pbss->ifname);
    }
#endif	/* CONFIG_HOSTAPD */
    nvram_set(I5_WLCFG_NVRAM_WPS_BH_BSS, pbss->ifname);
    nvram_set(I5_WLCFG_NVRAM_WPS_FH_IFNAME, pbss_fh->ifname);
  } else {
#ifdef CONFIG_HOSTAPD
      if (i5GlueIsHapdEnabled()) {
	    i5GlueWpsPbc(pdmif->ifname, NULL);
      }
#endif	/* CONFIG_HOSTAPD */
      nvram_set(I5_WLCFG_NVRAM_WPS_FH_IFNAME, pdmif->ifname);
  }

#ifdef CONFIG_HOSTAPD
  if (i5GlueIsHapdEnabled()) {
     // do nothing
     return;
  }
#endif	/* CONFIG_HOSTAPD */

  memset(&sockAddr, 0, sizeof(sockAddr));
  if ( getsockname(pctrlsock->psock->sd, (struct sockaddr *)&sockAddr, &addrLen) < 0)
  {
    i5TraceDirPrint("getsockname failed\n");
  }
  else
  {
    unsigned short portNo = ntohs(sockAddr.sin_port);
    i5Trace("Registering UDP port %d with WPS\n", portNo);
    pMsg = wps1905InitMessage(pdmif->ifname, WPS_1905_CTL_PBC, sizeof(unsigned short));
    if(pMsg!=NULL)
    {
      memcpy(WPS1905MSG_DATAPTR(pMsg),&portNo, sizeof(unsigned short));
      wps1905SendMsg(pctrlsock->psock, pMsg);
      free(pMsg);
    }
  }
}

static int
wlcfg_generate_dh_key_pair(unsigned char **priv, unsigned short *priv_len,
	unsigned char **pub, unsigned short *pub_len)
{
    DH *dh;
    unsigned int g;
    int ret = 0;

    if ((dh = DH_new()) == NULL) {
	i5TraceDirPrint("Insufficient memory\n");
        goto end;
    }

    if ((dh->p = BN_new()) == NULL) {
	i5TraceDirPrint("Insufficient memory\n");
	goto end;
    }

    if ((dh->g = BN_new()) == NULL) {
	i5TraceDirPrint("Insufficient memory\n");
	goto end;
    }

    if (BN_bin2bn(DH_P_VALUE, BUF_SIZE_1536_BITS, dh->p) == NULL) {
	i5TraceDirPrint("BN_bin2bn failed\n");
	goto end;
    }

    g = htonl(DH_G_VALUE);
    if (BN_bin2bn((uint8 *)&g, 4, dh->g) == NULL) {
	i5TraceDirPrint("BN_bin2bn failed\n");
	goto end;
    }
    RAND_linux_init();
    if (DH_generate_key(NULL, dh) == 0) {
	i5TraceDirPrint("DH_generate_key failed\n");
	goto end;
    }

    *priv_len = BN_num_bytes(dh->priv_key);
    *priv = (unsigned char*)malloc(*priv_len);
    if (!(*priv)) {
	i5TraceDirPrint("Insufficient memory\n");
	goto end;
    }
    BN_bn2bin(dh->priv_key, *priv);

    *pub_len = BN_num_bytes(dh->pub_key);
    *pub = (unsigned char*)malloc(*pub_len);
    if (!(*pub)) {
	i5TraceDirPrint("Insufficient memory\n");
	goto end;
    }
    BN_bn2bin(dh->pub_key, *pub);

    ret = 1;	/* Sucess */
end:

   if (dh) {
	DH_free(dh);
   }
   if (!ret && *pub) {
	free(*pub);
	*pub = NULL;
   }
   if (!ret && *priv) {
	free(*priv);
	*priv = NULL;
   }

   return ret;
}

static int
wlcfg_generate_dh_secret_key(unsigned char *secret_key, unsigned short *secret_len,
	unsigned char *pubkey, int pub_len, unsigned char *privkey, int priv_len)
{
    DH *dh;
    unsigned int g;
    int keylen = 0;
    int ret = 0;
    BIGNUM *pub = NULL;

    if ((dh = DH_new()) == NULL) {
	i5TraceDirPrint("Insufficient Memory\n");
        goto end;
    }

    if ((dh->p = BN_new()) == NULL) {
	i5TraceDirPrint("Insufficient Memory\n");
	goto end;
    }

    if ((dh->g = BN_new()) == NULL) {
	i5TraceDirPrint("Insufficient Memory\n");
	goto end;
    }

    if (BN_bin2bn(DH_P_VALUE, BUF_SIZE_1536_BITS, dh->p) == NULL) {
	i5TraceDirPrint("BN_bin2bn failed\n");
	goto end;
    }

    g = htonl(DH_G_VALUE);
    if (BN_bin2bn((uint8 *)&g, 4, dh->g) == NULL) {
	i5TraceDirPrint("BN_bin2bn failed\n");
	goto end;
    }

    if ((dh->priv_key = BN_bin2bn(privkey, priv_len, NULL)) == NULL) {
	i5TraceDirPrint("BN_bin2bn failed\n");
        goto end;
    }

    pub = BN_new();
    if (!pub){
	i5TraceDirPrint("BN_new failed\n");
	goto end;
    }

    if (BN_bin2bn(pubkey, pub_len, pub) == NULL) {
	i5TraceDirPrint("BN_bin2bn failed\n");
	BN_free(pub);
	pub = NULL;
	goto end;
    }

    /* If DH_compute_key_bn is successfull. Dont free pub */
    keylen = DH_compute_key_bn(secret_key, pub, dh);

    if(keylen < 0) {
	i5TraceDirPrint("DH_compute_key failed\n");
	BN_free(pub);
	pub = NULL;
	goto end;
    } else if(keylen < SIZE_PUB_KEY) {
	/* Zero padding */
	int i, j = SIZE_PUB_KEY - keylen;

	for (i = keylen - 1; i >= 0; i--) {
	  secret_key[i + j] = secret_key[i];
	}
	memset(secret_key, 0, j);
	keylen = SIZE_PUB_KEY;
    }

    *secret_len = keylen;
    ret = 1;	/* Success */
end:
   if (dh) {
	DH_free(dh);
   }
   return ret;
}

static int
wlcfg_encrypt_data(unsigned char *plain_txt, int txt_len, unsigned char *encr_key,
   unsigned char **cipherText, int *cipher_len, unsigned char *iv)
{
   unsigned char ivBuf[SIZE_128_BITS];
   /*  10 rounds for cbc 128  = (10+1) * 4 uint32 */
   unsigned int rk[44];
   unsigned char outBuf[1024];
   int encrypt_len;
   int ret = 0;

   if (txt_len == 0) {
      i5TraceDirPrint("Invalid parameters \n");
      return ret;
   }

   /* Generate a random iv */
   RAND_bytes(ivBuf, SIZE_128_BITS);
   memcpy(iv, ivBuf, SIZE_128_BITS);

   /* Now encrypt the plaintext and mac using the encryption key and IV. */
   rijndaelKeySetupEnc(rk, encr_key, 128);
   encrypt_len = aes_cbc_encrypt_pad(rk, 16, ivBuf, txt_len, plain_txt, outBuf, PAD_LEN_PADDING);
   *cipherText = (unsigned char*)malloc(encrypt_len);
   if (!(*cipherText)) {
      return ret;
   }
   memcpy(*cipherText, outBuf, encrypt_len);
   *cipher_len = encrypt_len;

   ret = 1;	/* Success */

   return ret;
}

static void
wlcfg_decrypt_data(unsigned char *cipher, int cipher_len, unsigned char *iv,
  unsigned char *keyWrapKey, unsigned char **plain_txt, int *plain_len)
{
   /*  10 rounds for cbc 128  = (10+1) * 4 uint32 */
   unsigned int rk[44];
   unsigned char outBuf[1024];
   int plaintext_len;

   rijndaelKeySetupDec(rk, keyWrapKey, 128);
   plaintext_len = aes_cbc_decrypt_pad(rk, 16, iv, cipher_len,
      cipher, outBuf, PAD_LEN_PADDING);
   *plain_txt = (unsigned char *)malloc(plaintext_len);
   if (!(*plain_txt)) {
      i5TraceDirPrint("Malloc failed\n");
      return;
   }
   memcpy(*plain_txt, outBuf, plaintext_len);
   *plain_len = plaintext_len;

   i5TraceInfo("Plain text len after decryption = %d\n", plaintext_len);
}

static int
wlcfg_wps_insertbyte(unsigned short id, unsigned char *buf, int bufIdx, unsigned char val)
{
    int ctr = 0;
    unsigned short len = 1;

    *((unsigned short *)&buf[bufIdx + ctr]) = htons(id);
    ctr += 2;

    *((unsigned short *)&buf[bufIdx + ctr]) = htons(len);
    ctr += 2;

    buf[bufIdx + ctr] = val;
    ctr++;

    return ctr;
}

static int
wlcfg_wps_insertshort(unsigned short id, unsigned char *buf, int bufIdx, unsigned short val)
{
    int ctr = 0;
    unsigned short len = 2;

    *((unsigned short *)&buf[bufIdx + ctr]) = htons(id);
    ctr += 2;

    *((unsigned short *)&buf[bufIdx + ctr]) = htons(len);
    ctr += 2;

    *((unsigned short *)&buf[bufIdx + ctr]) = htons(val);
    ctr += 2;

    return ctr;
}

static int
wlcfg_wps_insertbytes(unsigned short id, unsigned short len, unsigned char *buf,
  int bufIdx, void *val)
{
    int ctr = 0;

    *((unsigned short *)&buf[bufIdx + ctr]) = htons(id);
    ctr += 2;

    *((unsigned short *)&buf[bufIdx + ctr]) = htons(len);
    ctr += 2;

    memcpy(&buf[bufIdx + ctr], val, len);
    ctr += len;

    return ctr;
}

static void
wlcfg_derivekey(unsigned char *KDK,  unsigned char *prsnlString,
   int str_len, unsigned char *digest, int digest_len)
{
        unsigned char input[128] = {0}, output[1024];
        unsigned char hmac[SIZE_256_BITS];
        unsigned char *inPtr;
        unsigned int i = 0, iterations = 0;
        unsigned int temp, hmacLen = 0;
        int len = 0, outlen = 0;

        i5TraceInfo("Deriving a key of %d bits\n", digest_len);

        iterations = ((digest_len/8) + SIZE_256_BITS - 1)/SIZE_256_BITS;

        /*
         * Prepare the input buffer. During the iterations, we need only replace the
         * value of i at the start of the buffer.
         */
        temp = htonl(i);
        memcpy(&input[len], (uint8 *)&temp, SIZE_4_BYTES);
        len += SIZE_4_BYTES;
        memcpy(&input[len], prsnlString, str_len);
        len += str_len;
        temp = htonl(digest_len);
        memcpy(&input[len], (uint8 *)&temp, SIZE_4_BYTES);
        len += SIZE_4_BYTES;
        inPtr = &input[0];

        for (i = 0; i < iterations; i++) {
               /* Set the current value of i at the start of the input buffer */
               *(uint32 *)inPtr = htonl(i+1) ; /* i should start at 1 */
               hmac_sha256(KDK, SIZE_256_BITS, input, len, hmac, &hmacLen);
               memcpy(&output[outlen], hmac, hmacLen);
               outlen += hmacLen;
        }
        i5TraceInfo("Req Len = %d Total %d bytes copied to output\n", digest_len/8, outlen);

        /* Sanity check */
        if ((digest_len/8) > outlen) {
                i5TraceDirPrint("Key derivation generated less bits "
                        "than asked\n");
                return;
        }

        /*
         * We now have at least the number of key bits requested.
         * Return only the number of bits asked for. Discard the excess.
         */
        memcpy(digest, output, (digest_len/8));
        i5TraceInfo("End Deriving a key of %d bits\n", digest_len/8);
}

int
Wlcfg_proto_create_m1(unsigned char band, unsigned char **m1, int *m1_size, i5_wps_wscKey **m1_keys)
{
   unsigned char *buf, *pmem;
   unsigned short aux16;
   int len, num;
   char *tmp;
   unsigned char oui[4] = {0x00, 0x50, 0xf2, 0x00};
   int os_version = 0x80000000;
   unsigned char nonce[SIZE_128_BITS], uuid[SIZE_UUID];
   i5_wps_wscKey *keys = NULL;
   int ret = 0;

   if ((pmem = (unsigned char *)malloc(I5_PACKET_BUF_LEN)) == NULL) {
      i5TraceDirPrint("malloc error\n");
      goto end;
   }

   buf = pmem;

   len = 0;
   /* Version */
   len += wlcfg_wps_insertbyte(WPS_ID_VERSION, buf, len, 0x10);
   /* Mesage Type */
   len += wlcfg_wps_insertbyte(WPS_ID_MSG_TYPE, buf, len, WPS_ID_MESSAGE_M1);
   /* Enrolee UUID */
   RAND_bytes(uuid, sizeof(uuid));
   len += wlcfg_wps_insertbytes(WPS_ID_UUID_E, SIZE_UUID, buf, len, uuid);

   /* Enrolee MAC */
   len += wlcfg_wps_insertbytes(WPS_ID_MAC_ADDR, MAC_ADDR_LEN, buf, len, i5_config.i5_mac_address);

   /* Enrolee Nonce */
   RAND_bytes(nonce, sizeof(nonce));
   len += wlcfg_wps_insertbytes(WPS_ID_ENROLLEE_NONCE, SIZE_128_BITS, buf, len, nonce);

   /* Public key */
   keys = (i5_wps_wscKey *)malloc(sizeof(*keys));
   if (!keys) {
	i5TraceDirPrint("malloc error\n");
	goto end;
   }
   memset(keys, 0, sizeof(*keys));
   if (!wlcfg_generate_dh_key_pair(&keys->priv_key, &keys->priv_len,
	&keys->pub_key, &keys->pub_len)) {
	i5TraceDirPrint("wlcfg_generate_dh_key_pair failed \n");
	goto end;
   }
   len += wlcfg_wps_insertbytes(WPS_ID_PUBLIC_KEY, keys->pub_len, buf, len, keys->pub_key);

   /* Auth type */
   aux16 = WPS_AUTHTYPE_OPEN | WPS_AUTHTYPE_WPAPSK | WPS_AUTHTYPE_WPA2PSK;
   len += wlcfg_wps_insertshort(WPS_ID_AUTH_TYPE_FLAGS, buf, len, aux16);

   /* Encryption type */
   aux16 = WPS_ENCRTYPE_NONE | WPS_ENCRTYPE_TKIP | WPS_ENCRTYPE_AES;
   len += wlcfg_wps_insertshort(WPS_ID_ENCR_TYPE_FLAGS, buf, len, aux16);

   /* Connection type */
   len += wlcfg_wps_insertbyte(WPS_ID_CONN_TYPE_FLAGS, buf, len, WPS_CONNTYPE_ESS);

   /* Configuration Method */
   aux16 = WPS_CONFMET_VIRT_PBC | WPS_CONFMET_PHY_PBC; /* VPBC | HPBC */
   len += wlcfg_wps_insertshort(WPS_ID_CONFIG_METHODS, buf, len, aux16);

   /* WPS State */
   len += wlcfg_wps_insertbyte(WPS_ID_SC_STATE, buf, len, 0x1);

   /* Manufecturer */
   tmp = nvram_safe_get("wps_device_name");
   num = strlen(tmp);
   len += wlcfg_wps_insertbytes(WPS_ID_MANUFACTURER, num, buf, len, tmp);

   /* Model */
   tmp = nvram_safe_get("wps_device_name");
   num = strlen(tmp);
   len += wlcfg_wps_insertbytes(WPS_ID_MODEL_NAME, num, buf, len, tmp);

   /* Model No */
   tmp = nvram_safe_get("wps_modelnum");
   num = strlen(tmp);
   len += wlcfg_wps_insertbytes(WPS_ID_MODEL_NUMBER, num, buf, len, tmp);

   /* Serial Number */
   tmp = nvram_safe_get("boardnum");
   num = strlen(tmp);
   len += wlcfg_wps_insertbytes(WPS_ID_SERIAL_NUM, num, buf, len, tmp);

   /* Primary Device Type */
   aux16 = WPS_ID_PRIM_DEV_TYPE;
   *((unsigned short *)&buf[len]) = htons(aux16);
   len += 2;

   aux16 = SIZE_8_BYTES;
   *((unsigned short *)&buf[len]) = htons(aux16);
   len += 2;

   aux16 = WPS_DEVICE_TYPE_CAT_NW_INFRA;
   *((unsigned short *)&buf[len]) = htons(aux16);
   len += 2;
   memcpy(&buf[len], oui, sizeof(oui));
   len += sizeof(oui);

   aux16 = WPS_DEVICE_TYPE_SUB_CAT_NW_ROUTER;
   *((unsigned short *)&buf[len]) = htons(aux16);
   len += 2;

   /* Device Name */
   tmp = nvram_safe_get("wps_device_name");
   num = strlen(tmp);
   len += wlcfg_wps_insertbytes(WPS_ID_DEVICE_NAME, num, buf, len, tmp);

   /* RF Bands */
   len += wlcfg_wps_insertbyte(WPS_ID_RF_BAND, buf, len, band);	/* 0x1: 2GHz, 0x2: 5GHz */

   /* Association State */
   aux16 = 0;	/* Not ASSOC */
   len += wlcfg_wps_insertshort(WPS_ID_ASSOC_STATE, buf, len, aux16);

   /* DEV Password ID */
   aux16 = WPS_DEVICEPWDID_PUSH_BTN;	/* PBC */
   len += wlcfg_wps_insertshort(WPS_ID_DEVICE_PWD_ID, buf, len, aux16);

   /* Config Error */
   aux16 = WPS_ERROR_NO_ERROR;	/* No ERR */
   len += wlcfg_wps_insertshort(WPS_ID_CONFIG_ERROR, buf, len, aux16);

   /* OS Version */
   len += wlcfg_wps_insertbytes(WPS_ID_OS_VERSION, 4, buf, len, &os_version);

   /* Vendor Extension */
   aux16 = WPS_ID_VENDOR_EXT;
   *((unsigned short *)&buf[len]) = htons(aux16);
   len += 2;

   aux16 = SIZE_6_BYTES;
   *((unsigned short *)&buf[len]) = htons(aux16);
   len += 2;

   memcpy(&buf[len], WFA_VENDOR_EXT_ID, SIZE_3_BYTES);
   len += SIZE_3_BYTES;

   buf[len] = WPS_WFA_SUBID_VERSION2;
   len++;
   buf[len] = SIZE_1_BYTE;
   len++;
   buf[len] = 0x20;     /* WPS Version - 2 */
   len++;

   *m1 = (unsigned char*)malloc(len);
   if (!(*m1)) {
      i5TraceDirPrint("malloc error\n");
      goto end;
   }
   *m1_size = len;
   memcpy(*m1, buf, len);
   *m1_keys = keys;
   ret = 1;	/* Sucess */
   i5TraceDirPrint("M1 creation is successful m1 len=[%d] \n", len);

end:
   if (pmem) {
	free(pmem);
	pmem = NULL;
   }

   if (!ret && keys) {
	if (keys->pub_key) {
	   free(keys->pub_key);
	   keys->pub_key = NULL;
	}
	if (keys->priv_key) {
	   free(keys->priv_key);
	   keys->priv_key = NULL;
	}

	free(keys);
	keys = NULL;
	i5TraceDirPrint("M1 creation is failed \n");
   }
   return ret;
}

/* Insert vendor extension subelement */
static int
wlcfg_wps_insert_vndr_ext_elem(unsigned char *buf, int bufIdx, ieee1905_client_bssinfo_type *bssinfo)
{
   unsigned char aux8;
   unsigned short aux16;
   int len = 0;

   /* Vendor Extension */
   aux16 = WPS_ID_VENDOR_EXT;
   *((unsigned short *)&buf[bufIdx + len]) = htons(aux16);
   len += 2;

   aux16 = 9;
   *((unsigned short *)&buf[bufIdx + len]) = htons(aux16);
   len += 2;

   memcpy(&buf[bufIdx + len], WFA_VENDOR_EXT_ID, SIZE_3_BYTES);
   len += SIZE_3_BYTES;

   buf[bufIdx + len] = WPS_WFA_SUBID_VERSION2;
   len++;
   buf[bufIdx + len] = SIZE_1_BYTE;
   len++;
   buf[bufIdx + len] = 0x20;     /* WPS Version - 2 */
   len++;

   /* Multiap attributes */
   buf[bufIdx + len] = WPS_WFA_SUBID_MAP_EXT_ATTR;
   len++;
   buf[bufIdx + len] = SIZE_1_BYTE;
   len++;

   aux8 = 0;

   if (bssinfo->FrontHaulBSS)
      aux8 |= IEEE1905_FRONTHAUL_BSS;

   if (bssinfo->BackHaulBSS)
      aux8 |= IEEE1905_BACKHAUL_BSS;

   if (bssinfo->TearDown)
      aux8 |= IEEE1905_TEAR_DOWN;

   buf[bufIdx + len] = aux8;
   len++;

   return len;
}

int
Wlcfg_proto_create_m2(unsigned char band, unsigned char *m1, int m1_len,
  ieee1905_client_bssinfo_type *bssinfo, unsigned char **m2, int *m2_size)
{
   bool m1_mac_present, m1_nonce_present, m1_pubkey_present;
   char *tmp;
   unsigned char *p, *pmem = NULL, *buf;
   unsigned char m1_mac[MAC_ADDR_LEN], m1_nonce[SIZE_128_BITS] = {0}, *m1_pubkey=NULL;
   unsigned char nonce[SIZE_128_BITS], hmac[SIZE_256_BITS], *hmacData = NULL, uuid[SIZE_UUID];
   unsigned char authKey[SIZE_256_BITS], keyWrapKey[SIZE_128_BITS], emsk[SIZE_256_BITS];
   unsigned short aux16 = 0;
   unsigned char oui[4] = {0x00, 0x50, 0xf2, 0x00};
   int m1_pubkey_len=0, len=0, num, ret = 0, os_version = 0x80000000;
   i5_wps_wscKey *keys = NULL;

   if (!bssinfo || !m1) {
      i5TraceDirPrint("Invalid data \n");
      goto end;
   }

   m1_mac_present = m1_nonce_present = m1_pubkey_present = FALSE;
   p = m1;

   while((p - m1) < m1_len) {
      unsigned short attr_type, attr_len;

      attr_type = ntohs(*((unsigned short *)p));
      p += 2;
      attr_len = ntohs(*((unsigned short *)p));
      p += 2;
      if (attr_type == WPS_ID_MAC_ADDR) {
	 if (attr_len != MAC_ADDR_LEN) {
            i5TraceDirPrint("Incorrect len [%d] for mac addr\n", attr_len);
	    return 0;
	 }
         memcpy(m1_mac, p, attr_len);
	 p += attr_len;
	 m1_mac_present = TRUE;
      } else if (attr_type == WPS_ID_ENROLLEE_NONCE) {
	 if (attr_len != SIZE_128_BITS) {
            i5TraceDirPrint("Incorrect len [%d] for enrolee nonce\n", attr_len);
	    return 0;
	 }
         memcpy(m1_nonce, p, attr_len);
	 p += attr_len;
	 m1_nonce_present = TRUE;
      } else if (attr_type == WPS_ID_PUBLIC_KEY) {
	 m1_pubkey_len = attr_len;
	 m1_pubkey = (unsigned char *)malloc(attr_len);
	 if (!m1_pubkey) {
            i5TraceDirPrint("Malloc failed\n");
	    return 0;
	 }
	 memcpy(m1_pubkey, p, attr_len);
	 p += attr_len;
	 m1_pubkey_present = TRUE;
      } else {
	 p += attr_len;
      }
   }

   if (!m1_mac_present || !m1_nonce_present || !m1_pubkey_present) {
      i5TraceDirPrint("Inclomplete m1 message received\n");
      return 0;
   }

   /* M2 msg build started */
   if ((pmem = (unsigned char *)malloc(I5_PACKET_BUF_LEN)) == NULL) {
      i5TraceDirPrint("malloc error\n");
      goto end;
   }
   buf = pmem;
   len = 0;

   /* Version */
   len += wlcfg_wps_insertbyte(WPS_ID_VERSION, buf, len, 0x10);
   /* Mesage Type */
   len += wlcfg_wps_insertbyte(WPS_ID_MSG_TYPE, buf, len, WPS_ID_MESSAGE_M2);
   /* Enrolee Nonce */
   len += wlcfg_wps_insertbytes(WPS_ID_ENROLLEE_NONCE, SIZE_128_BITS, buf, len, m1_nonce);

   /* Registrar Nonce */
   RAND_bytes(nonce, sizeof(nonce));
   len += wlcfg_wps_insertbytes(WPS_ID_REGISTRAR_NONCE, SIZE_128_BITS, buf, len, nonce);

   /* Registrar UUID */
   RAND_bytes(uuid, sizeof(uuid));
   len += wlcfg_wps_insertbytes(WPS_ID_UUID_R, SIZE_UUID, buf, len, uuid);

   /* Public key */
   keys = (i5_wps_wscKey *)malloc(sizeof(*keys));
   if (!keys) {
	i5TraceDirPrint("malloc error\n");
	goto end;
   }
   memset(keys, 0, sizeof(*keys));
   if (!wlcfg_generate_dh_key_pair(&keys->priv_key, &keys->priv_len,
      &keys->pub_key, &keys->pub_len)) {
      i5TraceDirPrint("wlcfg_generate_dh_key_pair failed \n");
      goto end;
   }

   len += wlcfg_wps_insertbytes(WPS_ID_PUBLIC_KEY, keys->pub_len, buf, len, keys->pub_key);

   /* Key derivation (no bytes are written to the output buffer in the next
    * block of code, we just obtain three cryptographic keys that are needed
    * later
    */
    {
    unsigned char DHKey[SIZE_256_BITS], kdk[SIZE_256_BITS], digest[3*SIZE_256_BITS];
    unsigned char secret[SIZE_PUB_KEY];
    unsigned short secret_len = 0;
    unsigned char data[SIZE_128_BITS + MAC_ADDR_LEN + SIZE_128_BITS];
    unsigned int data_len = 0;

    wlcfg_generate_dh_secret_key(secret, &secret_len, m1_pubkey, m1_pubkey_len, keys->priv_key, keys->priv_len);
    if (SHA256(secret, secret_len, DHKey) == NULL) {
       i5TraceDirPrint("Secret key generation failed \n");
       goto end;
    }
    memcpy(&data[data_len], m1_nonce, SIZE_128_BITS);
    data_len += SIZE_128_BITS;
    memcpy(&data[data_len], m1_mac, MAC_ADDR_LEN);
    data_len += MAC_ADDR_LEN;
    memcpy(&data[data_len], nonce, SIZE_128_BITS);
    data_len += SIZE_128_BITS;

    hmac_sha256(DHKey, SIZE_256_BITS, data, data_len, kdk, NULL);
    wlcfg_derivekey(kdk, (unsigned char *)txt, (unsigned int)strlen(txt), digest, KDF_KEY_BITS);
    memcpy(authKey, digest, SIZE_256_BITS);
    memcpy(keyWrapKey, digest + SIZE_256_BITS, SIZE_128_BITS);
    memcpy(emsk, digest + SIZE_256_BITS + SIZE_128_BITS, SIZE_256_BITS);
    }
   /* Auth type */
   aux16 = WPS_AUTHTYPE_OPEN | WPS_AUTHTYPE_WPAPSK | WPS_AUTHTYPE_WPA2PSK;
   len += wlcfg_wps_insertshort(WPS_ID_AUTH_TYPE_FLAGS, buf, len, aux16);

   /* Encryption type */
   aux16 = WPS_ENCRTYPE_NONE | WPS_ENCRTYPE_TKIP | WPS_ENCRTYPE_AES;
   len += wlcfg_wps_insertshort(WPS_ID_ENCR_TYPE_FLAGS, buf, len, aux16);

   /* Connection type */
   len += wlcfg_wps_insertbyte(WPS_ID_CONN_TYPE_FLAGS, buf, len, WPS_CONNTYPE_ESS);

   /* Configuration Method */
   aux16 = WPS_CONFMET_VIRT_PBC | WPS_CONFMET_PHY_PBC; /* VPBC | HPBC */
   len += wlcfg_wps_insertshort(WPS_ID_CONFIG_METHODS, buf, len, aux16);

   /* Manufecturer */
   tmp = nvram_safe_get("wps_device_name");
   num = strlen(tmp);
   len += wlcfg_wps_insertbytes(WPS_ID_MANUFACTURER, num, buf, len, tmp);

   /* Model */
   tmp = nvram_safe_get("wps_device_name");
   num = strlen(tmp);
   len += wlcfg_wps_insertbytes(WPS_ID_MODEL_NAME, num, buf, len, tmp);

   /* Model No */
   tmp = nvram_safe_get("wps_modelnum");
   num = strlen(tmp);
   len += wlcfg_wps_insertbytes(WPS_ID_MODEL_NUMBER, num, buf, len, tmp);

   /* Serial Number */
   tmp = nvram_safe_get("boardnum");
   num = strlen(tmp);
   len += wlcfg_wps_insertbytes(WPS_ID_SERIAL_NUM, num, buf, len, tmp);

   /* Primary Device Type */
   aux16 = WPS_ID_PRIM_DEV_TYPE;
   *((unsigned short *)&buf[len]) = htons(aux16);
   len += 2;

   aux16 = SIZE_8_BYTES;
   *((unsigned short *)&buf[len]) = htons(aux16);
   len += 2;

   aux16 = WPS_DEVICE_TYPE_CAT_NW_INFRA;
   *((unsigned short *)&buf[len]) = htons(aux16);
   len += 2;
   memcpy(&buf[len], oui, sizeof(oui));
   len += sizeof(oui);

   aux16 = WPS_DEVICE_TYPE_SUB_CAT_NW_ROUTER;
   *((unsigned short *)&buf[len]) = htons(aux16);
   len += 2;

   /* Device Name */
   tmp = nvram_safe_get("wps_device_name");
   num = strlen(tmp);
   len += wlcfg_wps_insertbytes(WPS_ID_DEVICE_NAME, num, buf, len, tmp);

   /* RF Bands */
   len += wlcfg_wps_insertbyte(WPS_ID_RF_BAND, buf, len, band);	/* 0x1: 2GHz, 0x2: 5GHz */

   /* Association State */
   aux16 = 1;	/* 0 Not ASSOC, 1 ASSOC */
   len += wlcfg_wps_insertshort(WPS_ID_ASSOC_STATE, buf, len, aux16);

   /* Config Error */
   aux16 = WPS_ERROR_NO_ERROR;	/* No ERR */
   len += wlcfg_wps_insertshort(WPS_ID_CONFIG_ERROR, buf, len, aux16);

   /* DEV Password ID */
   aux16 = WPS_DEVICEPWDID_PUSH_BTN;	/* PBC */
   len += wlcfg_wps_insertshort(WPS_ID_DEVICE_PWD_ID, buf, len, aux16);

   /* OS Version */
   len += wlcfg_wps_insertbytes(WPS_ID_OS_VERSION, 4, buf, len, &os_version);

   {
   /* Encryption Settings */
   unsigned char settings[200], hash[SIZE_256_BITS];
   int settings_len = 0, cipher_len = 0;
   unsigned char *cipher = NULL, iv[SIZE_128_BITS];

   /* SSID */
   settings_len += wlcfg_wps_insertbytes(WPS_ID_SSID, (unsigned short)bssinfo->ssid.SSID_len, settings, settings_len, bssinfo->ssid.SSID);

   /* Auth Type */
   settings_len += wlcfg_wps_insertshort(WPS_ID_AUTH_TYPE, settings, settings_len, bssinfo->AuthType);

   /* Encryption Type */
   settings_len += wlcfg_wps_insertshort(WPS_ID_ENCR_TYPE, settings, settings_len, bssinfo->EncryptType);

   /* Network Key */
   settings_len += wlcfg_wps_insertbytes(WPS_ID_NW_KEY, (unsigned short)bssinfo->NetworkKey.key_len, settings, settings_len, bssinfo->NetworkKey.key);

   /* MAC */
   settings_len += wlcfg_wps_insertbytes(WPS_ID_MAC_ADDR, MAC_ADDR_LEN, settings, settings_len, m1_mac);

   /* Vendor  Extension elements */
   settings_len += wlcfg_wps_insert_vndr_ext_elem(settings, settings_len, bssinfo);

   i5TraceInfo("AP Configuration settings \n");
   i5TraceInfo("SSID => %s\n", bssinfo->ssid.SSID);
   i5TraceInfo("PWD  => %s\n", bssinfo->NetworkKey.key);
   i5TraceInfo("Auth Type => %x\n", bssinfo->AuthType);
   i5TraceInfo("Encr Type => %x\n", bssinfo->EncryptType);

   hmac_sha256(authKey, SIZE_256_BITS, settings, settings_len, hash, NULL);

   /* Hash */
   settings_len += wlcfg_wps_insertbytes(WPS_ID_KEY_WRAP_AUTH, 8, settings, settings_len, hash);
   wlcfg_encrypt_data(settings, settings_len, keyWrapKey, &cipher, &cipher_len, iv);

   /* Add encrypted settings */
   aux16 =  WPS_ID_ENCR_SETTINGS;
   *((unsigned short *)&buf[len]) = htons(aux16);
   len += 2;

   aux16 = sizeof(iv) + cipher_len;
   *((unsigned short *)&buf[len]) = htons(aux16);
   len += 2;

   memcpy(&buf[len], iv, sizeof(iv));
   len += sizeof(iv);
   memcpy(&buf[len], cipher, cipher_len);
   len += cipher_len;
   if (cipher) {
      free(cipher);
      cipher = NULL;
   }
   }
   /* Authenticator */
   hmacData = (unsigned char *)malloc(len + m1_len);
   if (!hmacData) {
      i5TraceDirPrint("malloc failed \n");
      goto end;
   }
   memcpy(hmacData, m1, m1_len);
   memcpy(hmacData + m1_len, buf, len);
   hmac_sha256(authKey, SIZE_256_BITS, hmacData, len + m1_len, hmac, NULL);
   len += wlcfg_wps_insertbytes(WPS_ID_AUTHENTICATOR, SIZE_8_BYTES, buf, len, hmac);

   *m2 = (unsigned char*)malloc(len);
   if(!(*m2)) {
      i5TraceDirPrint("malloc failed \n");
      goto end;
   }
   *m2_size = len;
   memcpy(*m2, buf, len);

   ret = 1;	/* Success */
   i5TraceInfo("M2 creation is successful m2 len=[%d] \n", len);

end:
   if (!ret) {
        i5TraceDirPrint("M2 creation is failed \n");
   }
   if (pmem) {
	free(pmem);
	pmem = NULL;
   }

   if (keys) {
	if (keys->pub_key) {
	   free(keys->pub_key);
	   keys->pub_key = NULL;
	}
	if (keys->priv_key) {
	   free(keys->priv_key);
	   keys->priv_key = NULL;
	}

	free(keys);
	keys = NULL;
   }
   if (hmacData) {
      free(hmacData);
      hmacData = NULL;
   }

   return ret;
}

int
Wlcfg_proto_process_m2(i5_wps_wscKey *key, unsigned char *m1, int m1_len,
   unsigned char *m2, int m2_len, ieee1905_client_bssinfo_type *bssinfo)
{
  int ret = -1;
  unsigned char *p;
  /* Data from m1 and m2 */
  unsigned char *plain_txt = NULL, map_attr = 0;
  unsigned char authKey[SIZE_256_BITS], keyWrapKey[SIZE_128_BITS], emsk[SIZE_256_BITS];
  unsigned char m2_nonce[SIZE_128_BITS] = {0}, *m2_pubkey = NULL, *m2_encrypted_settings = NULL, m2_authenticator[SIZE_8_BYTES];
  unsigned char m2_nonce_present, m2_pubkey_present, m2_encrypted_settings_present, m2_authenticator_present;
  unsigned short m1_privkey_len, m2_pubkey_len, m2_encrypted_settings_len;
  unsigned char m1_nonce[SIZE_128_BITS] = {0}, *m1_privkey = NULL, m1_mac[MAC_ADDR_LEN];
  bool m1_mac_present, m1_nonce_present, m1_pubkey_present;

  m1_privkey_len = m2_pubkey_len = m2_encrypted_settings_len = 0;
  if (!key || !m1 || !m2 || !bssinfo) {
    i5TraceDirPrint("Invalid data \n");
    goto end;
  }

  m1_privkey = key->priv_key;
  m1_privkey_len = key->priv_len;
  i5TraceInfo("m1_privkey_len = %d\n", m1_privkey_len);
  /* Extract data from m2 */
  m2_nonce_present = m2_pubkey_present = m2_encrypted_settings_present = m2_authenticator_present = 0;
  p = m2;
  while ((p - m2) < m2_len) {
    unsigned short attr_type, attr_len;

    attr_type = ntohs(*((unsigned short *)p));
    p += 2;
    attr_len = ntohs(*((unsigned short *)p));
    p += 2;

    if (attr_type == WPS_ID_REGISTRAR_NONCE) {
      if (attr_len != SIZE_128_BITS) {
        i5TraceDirPrint("Incorrect len [%d] for registrar nonce\n", attr_len);
        goto end;
      }
      memcpy(m2_nonce, p, attr_len);
      p += attr_len;
      m2_nonce_present = 1;
    } else if (attr_type == WPS_ID_PUBLIC_KEY) {
      m2_pubkey_len = attr_len;
      m2_pubkey = (unsigned char *)malloc(m2_pubkey_len);
      if (!m2_pubkey) {
        i5TraceDirPrint("Malloc error\n");
        goto end;
      }
      memcpy(m2_pubkey, p, m2_pubkey_len);
      p += m2_pubkey_len;
      m2_pubkey_present = 1;
    } else if (attr_type == WPS_ID_ENCR_SETTINGS) {
      m2_encrypted_settings_len = attr_len;
      m2_encrypted_settings = (unsigned char*)malloc(m2_encrypted_settings_len);
      if (!m2_encrypted_settings) {
        i5TraceDirPrint("Malloc error\n");
        goto end;
      }
      memcpy(m2_encrypted_settings, p, m2_encrypted_settings_len);
      p += m2_encrypted_settings_len;
      m2_encrypted_settings_present = 1;
    } else if (attr_type == WPS_ID_AUTHENTICATOR) {
      if (attr_len != 8) {
        i5TraceDirPrint("Incorrect len [%d] for authenticator\n", attr_len);
        goto end;
      }
      memcpy(m2_authenticator, p, attr_len);
      p += attr_len;
      m2_authenticator_present = 1;
    } else {
      p += attr_len;
    }

    if (m2_nonce_present && m2_pubkey_present && m2_encrypted_settings_present &&
      m2_authenticator_present) {
      break;
    }
  }

  if (!m2_nonce_present || !m2_pubkey_present || !m2_encrypted_settings_present ||
    !m2_authenticator_present) {
    i5TraceDirPrint("Missing attributes in the received M2 message"
      "nonce = %d, pubkey = %d,  settings = %d authenticator = %d\n",
      m2_nonce_present, m2_pubkey_present, m2_encrypted_settings_present,
      m2_authenticator_present);
    goto end;
  }

  m1_nonce_present = m1_pubkey_present = m1_mac_present = FALSE;
  p = m1;
  while ((p - m1) < m1_len) {
    unsigned short attr_type, attr_len;

    attr_type = ntohs(*((unsigned short *)p));
    p += 2;
    attr_len = ntohs(*((unsigned short *)p));
    p += 2;
    if (attr_type == WPS_ID_ENROLLEE_NONCE) {
      if (attr_len != SIZE_128_BITS) {
        i5TraceDirPrint("Incorrect len [%d] for enrolee nonce\n", attr_len);
        goto end;
      }
      memcpy(m1_nonce, p, SIZE_128_BITS);
      p += attr_len;
      m1_nonce_present = TRUE;
    } else if (attr_type == WPS_ID_PUBLIC_KEY) {
      p += attr_len;
      m1_pubkey_present = TRUE;
    } else if (attr_type == WPS_ID_MAC_ADDR) {
      if (attr_len != MAC_ADDR_LEN) {
        i5TraceDirPrint("Incorrect len [%d] for mac addr\n", attr_len);
        return 0;
      }
      memcpy(m1_mac, p, attr_len);
      p += attr_len;
      m1_mac_present = TRUE;
    } else {
      p += attr_len;
    }

    if (m1_mac_present && m1_nonce_present && m1_pubkey_present)
      break;
  }

  if (!m1_mac_present || !m1_nonce_present || !m1_pubkey_present) {
    i5TraceDirPrint("Missing attributes in m1 message\n");
    goto end;
  }

  /* Compute keys */
  {
  unsigned char DHKey[SIZE_256_BITS], kdk[SIZE_256_BITS], digest[3*SIZE_256_BITS];
  unsigned char secret[SIZE_PUB_KEY];
  unsigned short secret_len = 0;
  unsigned char data[SIZE_128_BITS + MAC_ADDR_LEN + SIZE_128_BITS];
  unsigned int len = 0;

  wlcfg_generate_dh_secret_key(secret, &secret_len, m2_pubkey, m2_pubkey_len, m1_privkey, m1_privkey_len);
  if (SHA256(secret, secret_len, DHKey) == NULL) {
    goto end;
  }
  memcpy(&data[len], m1_nonce, SIZE_128_BITS);
  len += SIZE_128_BITS;
  memcpy(&data[len], m1_mac, MAC_ADDR_LEN);
  len += MAC_ADDR_LEN;
  memcpy(&data[len], m2_nonce, SIZE_128_BITS);
  len += SIZE_128_BITS;

  hmac_sha256(DHKey, SIZE_256_BITS, data, len, kdk, NULL);
  wlcfg_derivekey(kdk, (unsigned char *)txt, (unsigned int)strlen(txt), digest, KDF_KEY_BITS);
  memcpy(authKey, digest, SIZE_256_BITS);
  memcpy(keyWrapKey, digest + SIZE_256_BITS, SIZE_128_BITS);
  memcpy(emsk, digest + SIZE_256_BITS + SIZE_128_BITS, SIZE_256_BITS);
  }

  /* Check Autentication */
  {
  /* Concatenate M1 and M2 (excluding the last 12 bytes, where the
  * authenticator attribute is contained) and calculate the HMAC, then
  * check it against the actual authenticator attribute value.
  */
  unsigned char hash[SIZE_256_BITS];
  int len = m1_len + m2_len - 12;
  unsigned char *hashData = (unsigned char *)malloc(len);
  if (!hashData) {
    i5TraceDirPrint("Malloc failed \n");
    goto end;
  }
  memcpy(hashData, m1, m1_len);
  memcpy(hashData + m1_len, m2, m2_len -12);
  hmac_sha256(authKey, SIZE_256_BITS, hashData, len, hash, NULL);

  if (memcmp(m2_authenticator, hash, SIZE_8_BYTES) != 0) {
    i5TraceDirPrint("M2 authentication is failed \n");
    free(hashData);
    hashData = NULL;
    goto end;
  } else {
    free(hashData);
    hashData = NULL;
  }
  }

  /* Decrypt the encryption settings */
  {
  unsigned char *cipher, iv[SIZE_128_BITS];
  int cipher_len = 0, plain_len = 0;

  memcpy(iv, m2_encrypted_settings, SIZE_128_BITS);
  cipher = m2_encrypted_settings + SIZE_128_BITS;
  cipher_len = m2_encrypted_settings_len - SIZE_128_BITS;
  wlcfg_decrypt_data(cipher, cipher_len, iv, keyWrapKey, &plain_txt, &plain_len);

  i5TraceInfo("After decrypt text len = %d\n", plain_len);
  p = plain_txt;
  while ((p - plain_txt) < plain_len) {
    unsigned short attr_type, attr_len;

    attr_type = ntohs(*((unsigned short *)p));
    p += 2;
    attr_len = ntohs(*((unsigned short *)p));
    p += 2;
    if (attr_type == WPS_ID_SSID) {
      if ((attr_len > IEEE1905_MAX_SSID_LEN) || (attr_len < 0))
        goto end;
      memcpy(bssinfo->ssid.SSID, p, attr_len);
      bssinfo->ssid.SSID_len = attr_len;
      p += attr_len;
    } else if (attr_type == WPS_ID_AUTH_TYPE) {
      if (attr_len != 2)
        goto end;
      bssinfo->AuthType = ntohs(*((unsigned short *)p));
      p += attr_len;
    } else if (attr_type == WPS_ID_ENCR_TYPE) {
      if (attr_len != 2)
        goto end;
      bssinfo->EncryptType = ntohs(*((unsigned short *)p));
      p += attr_len;
    } else if (attr_type == WPS_ID_NW_KEY) {
      if ((attr_len > IEEE1905_MAX_KEY_LEN) || (attr_len < 0))
        goto end;
      memcpy(bssinfo->NetworkKey.key, p, attr_len);
      bssinfo->NetworkKey.key_len = attr_len;
      p += attr_len;
    } else if (attr_type == WPS_ID_MAC_ADDR) {
      if (attr_len != MAC_ADDR_LEN)
        goto end;
      memcpy(bssinfo->ALID, p, attr_len);
      p += attr_len;
    } else if (attr_type == WPS_ID_VENDOR_EXT) {
      bool present = FALSE;
      unsigned char *q = p;

      while ((q - p) < attr_len) {
        if (*q == WPS_WFA_SUBID_MAP_EXT_ATTR) {
          q++;
          if (*q < SIZE_1_BYTE) {
            i5TraceDirPrint("Invalid length for multiap extension attr %d \n", *q);
            goto end;
          }
          q++;
          map_attr = *q;
          present = TRUE;
          break;
        }
        q++;
      }

      if (!present) {
        i5TraceDirPrint("MultiAp extension attribute is not present\n");
        goto end;
      }

      p += attr_len;
    } else {
      p += attr_len;
    }
  }
  bssinfo->BackHaulBSS = map_attr & IEEE1905_BACKHAUL_BSS ? 1 : 0;
  bssinfo->FrontHaulBSS = map_attr & IEEE1905_FRONTHAUL_BSS ? 1 : 0;
  bssinfo->TearDown = map_attr & IEEE1905_TEAR_DOWN ? 1 : 0;

  i5TraceInfo("Retrieved AP Configuration settings from wsc M2 \n");
  i5TraceInfo("SSID => %s\n", bssinfo->ssid.SSID);
  i5TraceInfo("PWD  => %s\n", bssinfo->NetworkKey.key);
  i5TraceInfo("Auth Type => %x\n", bssinfo->AuthType);
  i5TraceInfo("Encr Type => %x\n", bssinfo->EncryptType);
  i5TraceInfo("Backhaul BSS => %d\n", bssinfo->BackHaulBSS);
  i5TraceInfo("Fronthaul BSS => %d\n", bssinfo->FrontHaulBSS);
  i5TraceInfo("Tear Down => %d\n", bssinfo->TearDown);
  }

  /* Successfull */
  ret = 0;

  end:
  if (m2_pubkey) {
   free(m2_pubkey);
   m2_pubkey = NULL;
  }

  if (m2_encrypted_settings) {
   free(m2_encrypted_settings);
   m2_encrypted_settings = NULL;
  }

  if (plain_txt) {
   free(plain_txt);
   plain_txt = NULL;
  }

  return ret;
}

/* Free m1 data and wsc keys */
void
wlcfg_wsc_free(unsigned char *m1, i5_wps_wscKey *keys)
{
  if (m1) {
     free(m1);
     m1 = NULL;
  }

  if (keys) {
     if (keys->pub_key) {
        free(keys->pub_key);
        keys->pub_key = NULL;
     }
     if (keys->priv_key) {
        free(keys->priv_key);
        keys->priv_key = NULL;
     }

     free(keys);
     keys = NULL;
  }
}

/* Fetch wsc data  from wsc message */
void
wlcfg_wsc_get_data(unsigned char *msg, int len, wsc_data_t *data)
{
  unsigned char *p = msg;
  bool mac_present, band_present, msg_type_present;

  if (!data) {
     i5TraceDirPrint("Invalid input\n");
     return;
  }

  mac_present = band_present = msg_type_present = FALSE;

  while ((p - msg) < len) {
     unsigned short attr_type, attr_len;

     attr_type = ntohs(*((unsigned short *)p));
     p += 2;
     attr_len = ntohs(*((unsigned short *)p));
     p += 2;

     switch(attr_type) {
	case WPS_ID_MSG_TYPE:
	   if (attr_len != SIZE_1_BYTE) {
             i5TraceDirPrint("Incorrect len [%d] for wsc msg type\n", attr_len);
	   } else {
	     data->msg_type = *p;
	   }
           p += attr_len;
	   msg_type_present = TRUE;
	break;

        case WPS_ID_MAC_ADDR:
           if (attr_len != MAC_ADDR_LEN) {
              i5TraceDirPrint("Incorrect len [%d] for mac addr\n", attr_len);
           } else {
              memcpy(data->mac, p, MAC_ADDR_LEN);
	   }
           p += attr_len;
	   mac_present = TRUE;
	break;

        case WPS_ID_RF_BAND:
	   if (attr_len != SIZE_1_BYTE) {
              i5TraceDirPrint("Incorrect len [%d] for rf band\n", attr_len);
	   } else {
	      data->rf_band = *p;
	   }
	   p += attr_len;
	   band_present = TRUE;
	 break;

	 default:
	   p += attr_len;
     }

     if (mac_present && band_present && msg_type_present)
	 break;
  }
}

/* Checks whether the interface is virtual or not */
int i5WlCfgIsVirtualInterface(const char *ifname)
{
	int unit = -1, subunit = -1;

  /* Dont add if its virtual */
  if (get_ifname_unit(ifname, &unit, &subunit) < 0) {
    return 1;
  }
  if (unit < 0) {
    return 1;
  }
  if (subunit >= 0) {
    /* Its a virtual interface */
    i5Trace("ifname[%s] unit[%d] subunit[%d] is virtual\n", ifname, unit, subunit);
    return 1;
  }

  return 0;
}

/* Checks whether the interface is backhaul or not */
int i5WlCfgIsBackHaulInterface(const char *ifname)
{
  int iVal = 0;
  char *tmp = i5WlCfgGetNvVal(ifname, "map");

  if (tmp) {
    iVal = strtoul(tmp, NULL, 0);
    if (iVal & IEEE1905_MAP_FLAG_BACKHAUL) {
      return 1;
    }
  }

  return 0;
}

int
i5WlCfgChannelToband(unsigned char channel)
{
  int band = BAND_INV;

  if (channel > 0 && channel <= 14) {
     band = BAND_2G;
  } else if (channel >= 36 && channel <= 64) {
     band = BAND_5GL;
  } else if (channel >= 100) {
     band = BAND_5GH;
  }

  return band;
}

/* Check is the wireless interface is in forwarder or not */
int i5WlCfgIsInterfaceInFwder(const char *ifname)
{
  char name[IFNAMSIZ], *next = NULL;
  char *ifnames;

  ifnames = nvram_safe_get("fwd_wlandevs");
  foreach(name, ifnames, next) {
    /* Check for the ifname */
    if (strcmp(name, ifname) == 0) {
      i5TraceInfo("ifname[%s] is in fwd_wlandevs=[%s]\n", ifname, ifnames);
      return 1;
    }
  }

  return 0;
}

#if defined(__CONFIG_DHDAP__) && defined(__CONFIG_GMAC3__)
#include <dhdioctl.h>
#pragma message "DHDAP and GMAC3 enabled"

extern int dhd_probe(char *name);
extern int dhd_bssiovar_set(char *ifname, char *iovar, int bssidx, void *param, int paramlen);

/* Set IEEE1905 AL MAC or Multicast MAC, so that the driver will send the packet to socket created
 * on the interface directly instead of sending it to bridge socket
 */
int i5WlCfgSetIEEE1905MACInDHD(char *ifname, unsigned char *mac, int isUcast)
{
  int is_dhd = 0, dhd_ret = 0, unit = 0, subunit = 0;

  is_dhd = !dhd_probe(ifname);
  if (!is_dhd) {
    i5Trace("ifname[%s] isUcast[%d] MAC["I5_MAC_DELIM_FMT"] dhd_probe failed\n",
      ifname, isUcast, I5_MAC_PRM(mac));
    return -1;
  }

  get_ifname_unit(ifname, &unit, &subunit);
  subunit = (subunit <= 0) ? 0 : subunit;

  if (isUcast) {
    dhd_ret = dhd_bssiovar_set(ifname, "1905_al_ucast", subunit, mac, ETHER_ADDR_LEN);
  } else {
    dhd_ret = dhd_bssiovar_set(ifname, "1905_al_mcast", subunit, mac, ETHER_ADDR_LEN);
  }

  i5Trace("ifname[%s] subunit[%d] isUcast[%d] MAC["I5_MAC_DELIM_FMT"] dhd_ret[%d]\n",
    ifname, subunit, isUcast, I5_MAC_PRM(mac), dhd_ret);

  return dhd_ret;
}
#endif /* __CONFIG_DHDAP__ &&  __CONFIG_GMAC3__ */

/* Get Device type */
int i5WlCfgGetDevType(char *name, void *buf, int len)
{
  int s, ret;
  struct ifreq ifr;
  struct ethtool_drvinfo info;

  /* open socket to kernel */
  if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket");
    return -1;
  }

  /* get device type */
  memset(&info, 0, sizeof(info));
  info.cmd = ETHTOOL_GDRVINFO;
  ifr.ifr_data = (caddr_t)&info;
  strncpy(ifr.ifr_name, name, IFNAMSIZ);
  if ((ret = ioctl(s, SIOCETHTOOL, &ifr)) < 0) {
    *(char *)buf = '\0';
  } else {
    strncpy(buf, info.driver, len);
  }

  close(s);
  return ret;
}
#endif /* MULTIAP */
#endif // endif
