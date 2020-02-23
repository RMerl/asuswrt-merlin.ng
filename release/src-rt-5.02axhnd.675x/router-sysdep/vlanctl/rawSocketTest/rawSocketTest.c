/***********************************************************************
 *
 *  Copyright (c) 2007-2010  Broadcom Corporation
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


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>

static int sockfd;
static char src_mac[ETH_ALEN];
static char dest_mac[ETH_ALEN] = {0x02, 0x00, 0x00, 0x00, 0x00, 0x01};

static int initSocketAddress(struct sockaddr_ll* socket_address, char *ifName)
{
    struct ifreq ifr;
    int i;

   /* RAW communication */
    socket_address->sll_family = AF_PACKET; /* TX */
    socket_address->sll_protocol = 0; /* BIND */ /* FIXME: htons(ETH_P_ALL) ??? */

    memset(&ifr, 0x00, sizeof(ifr));
    strncpy(ifr.ifr_name, ifName, sizeof(ifr.ifr_name));
    if(ioctl(sockfd, SIOCGIFINDEX, &ifr) < 0)
    {
        printf("Fail to get ifindex\n");
        return -1;
    }
    socket_address->sll_ifindex = ifr.ifr_ifindex; /* BIND, TX */

    socket_address->sll_hatype = 0; /* RX */
    socket_address->sll_pkttype = 0; /* RX */

    socket_address->sll_halen = ETH_ALEN; /* TX */

    /* MAC */
    for(i = 0; i < ETH_ALEN; i++)
    {
        socket_address->sll_addr[i] = dest_mac[i]; /* TX */
    }
    socket_address->sll_addr[6] = 0x00;/*not used*/
    socket_address->sll_addr[7] = 0x00;/*not used*/

    /* Get source MAC of the Interface we want to bind to */
    memset(&ifr, 0x00, sizeof(ifr));
    strncpy(ifr.ifr_name, ifName, sizeof(ifr.ifr_name));
    if(ioctl(sockfd, SIOCGIFHWADDR, &ifr) < 0)
    {
        printf("Fail to get hw addr\n");
        return -1;
    }

    for(i = 0; i < ETH_ALEN; i++)
    {
        src_mac[i] = (unsigned char)ifr.ifr_hwaddr.sa_data[i];
    }

    printf("Binding to %s: ifindex <%d>, protocol <0x%04X>...\n",
           ifName, socket_address->sll_ifindex, socket_address->sll_protocol);

    /* Bind to Interface */
    if(bind(sockfd, (struct sockaddr*)socket_address, sizeof(struct sockaddr_ll)) < 0)
    {
        printf("Binding error\n");
        return -1;
    }

    printf("Done!\n");

    return 0;
}

static inline void dumpHexData(void *head, int len)
{
    int i;
    unsigned char *curPtr = (unsigned char*)head;

    printf("Address : 0x%08X", (unsigned int)curPtr);

    for (i = 0; i < len; ++i) {
        if (i % 4 == 0)
            printf(" ");       
        if (i % 16 == 0)
            printf("\n0x%04X:  ", i);
        printf("%02X ", *curPtr++);
    }

    printf("\n");
}

#define PACKET_TAGGED_IP { 0x02, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x01, 0x81, 0x00, 0x60, 0x64, 0x08, 0x00, 0x45, 0x4F, 0x00, 0xB6, 0x00, 0x00, 0x00, 0x00, 0x40, 0x72, 0xEA, 0xFA, 0xC6, 0x13, 0x01, 0x64, 0xC6, 0x13, 0x01, 0x02 }

static char txBuffer[ETH_FRAME_LEN] = PACKET_TAGGED_IP;

static int sendFrame(struct sockaddr_ll* socket_address)
{
    int len;

    /* send packet */
    len = sendto(sockfd, txBuffer, 128, 0,
                 (struct sockaddr*)socket_address, sizeof(*socket_address));
    if (len == -1)
    {
        printf("ERROR: Packet TX failed!\n");
        return -1;
    }

    printf("===> TX PACKET (%d bytes)\n", len);
    dumpHexData(txBuffer, len);

    return 0;
}

static char rxBuffer[ETH_FRAME_LEN];

int main(void)
{
    int ret;
    int len;
    char *ifName = "eth0.v0";
    socklen_t fromlen;
    struct sockaddr_ll socket_address;

    printf("Opening socket on %s\n", ifName);
    if((sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) == -1)
    {
        printf("ERROR: Could not open Raw Socket");
        return -1;
    }

    ret = initSocketAddress(&socket_address, ifName);
    if(ret)
    {
        goto out;
    }

    do {
        printf("Listening to %s...\n", ifName);

        fromlen = sizeof(struct sockaddr_ll);
        len = recvfrom(sockfd, rxBuffer, ETH_FRAME_LEN, 0,
                       (struct sockaddr*)&socket_address, &fromlen);
        if(len < 0)
        {
            printf("ERROR: recvfrom\n");
            ret = -1;
            goto out;
        }

        printf("===> RX PACKET (%d bytes)\n", len);
        dumpHexData(rxBuffer, len);

        sleep(1);

        ret = sendFrame(&socket_address);
        if(ret)
        {
            goto out;
        }

    } while(1);

out:
    close(sockfd);
    return ret;
}
