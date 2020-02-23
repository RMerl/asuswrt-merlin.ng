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

/*
 * IEEE1905 Netlink
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/socket.h>
#include <linux/if.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <linux/rtnetlink.h>
#include <linux/genetlink.h>
#include "ieee1905_socket.h"
#include "ieee1905_security.h"
#include "ieee1905_netlink.h"
#include "ieee1905_trace.h"
#include "ieee1905_interface.h"
#include "ieee1905_flowmanager.h"
#include "ieee1905_brutil.h"
#include "ieee1905_message.h"
#include "ieee1905_datamodel_priv.h"

#define I5_TRACE_MODULE i5TraceNetlink

/* cannot include linux/if.h and net/if.h */
char *if_indextoname(unsigned int ifindex, char *ifname);

static int i5NetlinkParseAttr(struct rtattr *tb[], int max, struct rtattr *rta, int len)
{
  memset(tb, 0, sizeof(struct rtattr *) * (max + 1));
  while (RTA_OK(rta, len))
  {
    if (rta->rta_type <= max)
    {
      tb[rta->rta_type] = rta;
    }
    rta = RTA_NEXT(rta,len);
  }

  if (len)
  {
    printf("!!!Deficit %d, rta_len=%d\n", len, rta->rta_len);
  }
  return 0;
}

static void i5NetlinkProcess(i5_socket_type *psock)
{
  int                   recvLen;
  char                  buf[4096];
  struct iovec          iov = { buf, sizeof(buf)};
  struct msghdr         msg;
  struct nlmsghdr      *nl_msgHdr;
  int                   len;
  struct rtattr        *rta;
  unsigned char         oper_status = 0;
  struct sockaddr_nl    sa;
  i5_socket_type       *pif;
  char *                if_name;

  i5TraceInfo("\n");
  memset(&msg, 0, sizeof(struct msghdr));
  msg.msg_name    = (void*)&sa;
  msg.msg_namelen = sizeof(sa);
  msg.msg_iov     = &iov;
  msg.msg_iovlen  = 1 ;

  recvLen = recvmsg(psock->sd, &msg, 0);
  if(recvLen <= 0) {
    i5TraceError("not able to recieve msg from netlink\n");
    return;
  }

  for(nl_msgHdr = (struct nlmsghdr *) buf;
      NLMSG_OK(nl_msgHdr, (unsigned int)recvLen);
      nl_msgHdr = NLMSG_NEXT(nl_msgHdr, recvLen))
  {
    if (nl_msgHdr->nlmsg_type == NLMSG_DONE)
    {
      return;
    }

    if (nl_msgHdr->nlmsg_type == NLMSG_ERROR)
    {
      continue;
    }

    if ( i5_socket_type_netlink == psock->type )
    {
      struct rtattr    *tb[IFLA_MAX + 1];
      struct ifinfomsg *ifi;

      if ((RTM_NEWLINK == nl_msgHdr->nlmsg_type) ||
          (RTM_DELLINK == nl_msgHdr->nlmsg_type) )
      {
        if (nl_msgHdr->nlmsg_pid !=0)
        {
          continue;
        }

        ifi = NLMSG_DATA (nl_msgHdr);
        len = nl_msgHdr->nlmsg_len - NLMSG_LENGTH (sizeof (struct ifinfomsg));
        if (len < 0)
        {
          continue;
        }

        rta = IFLA_RTA(ifi);
        i5NetlinkParseAttr(tb, IFLA_MAX, rta, len);

        if (tb[IFLA_IFNAME] == NULL || tb[IFLA_OPERSTATE] == NULL)
        {
          continue;
        }

        if (tb[IFLA_MASTER] != NULL) {
          int   mstIndex;
          char  ifname[I5_MAX_IFNAME];

          mstIndex= *(__u32 *)RTA_DATA(tb[IFLA_MASTER]);
          if ( if_indextoname(mstIndex, &ifname[0]) != NULL ) {
            if ( i5BrUtilDevIsBridge(&ifname[0]) ) {
              pif = i5SocketFindDevSocketByIndex(mstIndex);
              if ( pif ) {
                if (pif->u.sll.notify) {
                  pif->u.sll.notify(pif, IF_OPER_UNKNOWN);
                }
              }
            }
          }
        }

        if_name = (char *)RTA_DATA(tb[IFLA_IFNAME]);
        oper_status = *(__u8 *)RTA_DATA(tb[IFLA_OPERSTATE]);
        pif = i5SocketFindDevSocketByName(if_name);
        i5TraceInfo("Event for interface %s, ifi %d, pif %p, operstatus %d\n", if_name, ifi->ifi_index, pif, oper_status);

        /* check if interface informatin has changed - delete/create the socket if it has */
        if ( pif ) {
          unsigned char mac_addr[MAC_ADDR_LEN];
          i5InterfaceInfoGet(if_name, &mac_addr[0]);
          if ( (pif->u.sll.sa.sll_ifindex != ifi->ifi_index) ||
               (0 != memcmp(&mac_addr[0], &pif->u.sll.mac_address[0], MAC_ADDR_LEN))) {
            /* if index or mac changed treat as if_down */
            i5TraceInfo("Interface info change for %s\n", if_name);
            if (pif->u.sll.notify) {
              pif->u.sll.notify(pif, IF_OPER_DOWN);
            }
#if defined(SUPPORT_IEEE1905_FM)
            i5FlowManagerDeactivateInterface(pif);
#endif // endif
            i5MessageCancel(pif);
            i5SocketClose(pif);
            pif = NULL;
          }
        }

        if ( pif ) {
          if (pif->u.sll.notify) {
            pif->u.sll.notify(pif, oper_status);
          }
          /* interface socket only exists if interface was operational so only IF_OPER_DOWN matters here */
          if (IF_OPER_DOWN == oper_status) {
#if defined(SUPPORT_IEEE1905_FM)
            i5FlowManagerDeactivateInterface(pif);
#endif // endif
            i5MessageCancel(pif);
            i5SocketClose(pif);
          }
        }
        else {
          if ((IF_OPER_UP == oper_status) || (IF_OPER_UNKNOWN == oper_status)) {
            i5InterfaceAdd(if_name, I5_MATCH_MEDIA_TYPE_ANY);
          }
        }
      }
    }
  }
}

static void i5NetlinkRouteRegister(void)
{
  i5_socket_type  *psock;
  int sd;

  sd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  if ( sd < 0 ) {
    printf("unable to create netlink socket - errno=%d", errno);
    return;
  }

  psock = i5SocketNew(sd, i5_socket_type_netlink, i5NetlinkProcess);
  if ( psock == NULL ) {
    close(sd);
    return;
  }
  psock->u.snl.sa.nl_family = AF_NETLINK;
  psock->u.snl.sa.nl_groups = RTMGRP_LINK;

  if((bind(sd, (struct sockaddr *)&(psock->u.snl.sa), sizeof(struct sockaddr_nl))) < 0)
  {
    i5SocketClose(psock);
  }
}

void i5NetlinkInit( void )
{
   i5NetlinkRouteRegister();
}
