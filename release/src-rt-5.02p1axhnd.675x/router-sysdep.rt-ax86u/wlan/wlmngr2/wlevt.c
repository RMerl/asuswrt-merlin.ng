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

/* This file includes the Wlan Driver Event handling */


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <linux/if_packet.h>
#include <wlcsm_linux.h>
#include <wlcsm_lib_api.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <bcmnvram.h>
#include "wlsyscall.h"
#include <wlif_utils.h>

#include <typedefs.h>
#include <wlioctl.h>

/* Following Definition is from eapd.h. When eapd.h is opened, these definition should be removed*/
#define EAPD_WKSP_PORT_INDEX_SHIFT	4
#define EAPD_WKSP_SPORT_OFFSET		(1 << 5)

#define EAPD_WKSP_MEVENT_UDP_PORT       44000
#define EAPD_WKSP_MEVENT_UDP_RPORT      EAPD_WKSP_MEVENT_UDP_PORT
#define EAPD_WKSP_MEVENT_UDP_SPORT      EAPD_WKSP_MEVENT_UDP_PORT + EAPD_WKSP_SPORT_OFFSET


int sock = -1;

int wldsltr_alloc(int);
int wlmngr_alloc(int);
void wldsltr_free(void );
void wlmngr_free(void );

/* Init basic data structure */
static int wlevt_init( void )
{
    int reuse = 1;
    int err = 0;

    struct sockaddr_in sockaddr;

    /* open loopback socket to communicate with EAPD */
    memset(&sockaddr, 0, sizeof(sockaddr));
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    sockaddr.sin_port = htons(EAPD_WKSP_MEVENT_UDP_SPORT);

    if (( sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        printf("%s@%d Unable to create socket\n", __FUNCTION__, __LINE__ );
        err = -1;
    } else if ( (err = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse))) < 0) {
        printf("%s@%d: Unable to setsockopt to loopback socket %d.\n", __FUNCTION__, __LINE__, sock);
    } else if ( (err = bind(sock, (struct sockaddr *)&sockaddr, sizeof(sockaddr))) < 0) {
        printf("%s@%d Unable to bind to loopback socket %d\n", __FUNCTION__, __LINE__, sock);
    }

    if ( err < 0  && sock >= 0 ) {
        printf("%s@%d: failure. Close socket\n", __FUNCTION__, __LINE__ );
        close(sock);
        return err;
    }
#ifdef DSLCPE_EVT
    else
        printf("%s@%d: opened loopback socket %d\n", __FUNCTION__, __LINE__, sock);
#endif

    return err;

}

/* De-initialization */
static void wlevt_deinit (void )
{
    if ( sock >= 0 )
        close(sock);

    return;
}

int wlevt_send_msg(unsigned int idx,unsigned int sub_idx, WL_STA_EVENT_DETAIL *sta)
{
    WLCSM_TRACE(WLCSM_TRACE_LOG," SEND MESSGE TO WLMNGR from WLEVT \r\n" );
    /* Delay 2 seconds,as event is send up fast and the communication between
       wlmngr and wlevt is faster than previous implementation, it will allow
       the wlmngr to get full completed information
    */
    sleep(2);
    return wlcsm_mngr_update_stalist(idx,sub_idx,sta,sizeof(WL_STA_EVENT_DETAIL));
}


#define SOCK_RCV_BUF_LEN (4096)
/* Recv and handle event from wlan Driver */
static void wlevt_main_loop( void )
{
    static char pkt[SOCK_RCV_BUF_LEN] = {0};
    struct sockaddr_in from;
    int sock_len = sizeof(struct sockaddr_in);
    bcm_event_t *dpkt;
    fd_set fdset;
    int fdmax = -1;
    int sta_type,event_type;

    if (daemon(1, 1) == -1) {
        printf("%s: daemon error\n", __FUNCTION__);
        exit(errno);
    }

    FD_ZERO(&fdset);
    FD_SET(sock, &fdset);
    fdmax = sock;

    while ( 1 ) {
        sta_type=-1;
        select(fdmax+1, &fdset, NULL, NULL, NULL);
        if (FD_ISSET(sock, &fdset) && recvfrom(sock, pkt, SOCK_RCV_BUF_LEN, 0, (struct sockaddr *)&from, (socklen_t *)&sock_len)>0) {
            dpkt = (bcm_event_t *)(pkt+IFNAMSIZ);
            event_type = ntohl(dpkt->event.event_type);
            WLCSM_TRACE(WLCSM_TRACE_DBG," event:%lu\r\n",event_type);
            /* when STA leaves without notice bys sending disassociate, dongle will detec tand send WLC_E_DEAUTH */
            if (event_type == WLC_E_AUTH_IND || event_type == WLC_E_ASSOC_IND || event_type == WLC_E_REASSOC_IND)
                sta_type = STA_CONNECTED;
            else if( event_type == WLC_E_DEAUTH_IND || event_type == WLC_E_DEAUTH || event_type == WLC_E_DISASSOC ||
                     event_type == WLC_E_DISASSOC_IND)
                sta_type=STA_DISCONNECTED;

            if(sta_type>=0) {
                /* Send event to update assoc list*/
                unsigned int idx=0,sub_idx=0;
                WL_STA_EVENT_DETAIL sta_detail;
                WLCSM_TRACE(WLCSM_TRACE_DBG," event:%lu\r\n",sta_type);
                sta_detail.event_type = sta_type;
                memcpy(&(sta_detail.mac), BINMAC_STR_CONVERT((char *)(&(dpkt->event.addr))),MACSTR_LEN);
                WLCSM_TRACE(WLCSM_TRACE_DBG, "event sta:%s\n",sta_detail.mac);
                sscanf(dpkt->event.ifname,"wl%u.%u",&idx,&sub_idx);
                wlevt_send_msg(idx,sub_idx,&sta_detail);
            }
        }
    }
    return;
}

/* Event Handling Main, called from wldaemon */
int main( void )
{
    WLCSM_SET_TRACE("wlevt");
    if (wlevt_init() >=0) {
        wlevt_main_loop();
    }
    wlevt_deinit();
    exit(-1);
}
