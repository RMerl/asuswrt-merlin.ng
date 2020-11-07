/*
* <:copyright-BRCM:2015:proprietary:standard
* 
*    Copyright (c) 2015 Broadcom 
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
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <poll.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <linux/netlink.h>
#include <bcm_mcast_api.h>
#include <fcntl.h>
#include <sys/ioctl.h>

int bcm_mcast_api_socket_create(int *nl_sock, int portid)
{
    struct sockaddr_nl src_addr;
    int                sd;
    int                rc;

    if ( NULL == nl_sock )
    {
       return -1;
    }

    *nl_sock = -1;
    sd = socket(PF_NETLINK, SOCK_RAW, NETLINK_BCM_MCAST);
    if(sd < 0)
    {
        return sd;
    }

    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = portid;
    src_addr.nl_groups = 0;
    rc = bind(sd, (struct sockaddr *)&src_addr, sizeof(src_addr));
    if(rc < 0)
    {
        close(sd);
        return -1;
    }

    *nl_sock = sd;
    return 0;
} /* bcm_mcast_api_socket_create */

int bcm_mcast_api_nl_recv(int nl_sock, char *rx_buf, int rx_bufsize, bcm_mcast_api_rcv_func process_func)
{
   int                   recvLen;
   struct iovec          iov;
   struct msghdr         msg;
   struct nlmsghdr      *nl_msgHdr;
   unsigned char        *pdata;
   struct sockaddr_nl    sa;
   int                   data_len;

   if ( (nl_sock < 0) ||
        (NULL == rx_buf) ||
        (0 == rx_bufsize) || 
        (NULL == process_func) )
   {
      return -EINVAL;
   }

   memset(&msg, 0, sizeof(struct msghdr));
   msg.msg_name    = (void*)&sa;
   msg.msg_namelen = sizeof(sa);
   msg.msg_iov     = &iov;
   msg.msg_iovlen  = 1;
 
   iov.iov_base = (void *)rx_buf;
   iov.iov_len = rx_bufsize;
 
   recvLen = recvmsg(nl_sock, &msg, 0);
   if(recvLen <= 0)
   {
       return -EIO;
   }

   if (msg.msg_flags & MSG_TRUNC)
   {
      return -ENOMEM;
   }

   for(nl_msgHdr = (struct nlmsghdr *)rx_buf; 
       NLMSG_OK(nl_msgHdr, (unsigned int)recvLen);
       nl_msgHdr = NLMSG_NEXT(nl_msgHdr, recvLen))
   {
      if ((nl_msgHdr->nlmsg_type == NLMSG_NOOP) ||
          (nl_msgHdr->nlmsg_type == NLMSG_DONE) ||
          (nl_msgHdr->nlmsg_type == NLMSG_OVERRUN) )
      {
         continue;
      }

      if (nl_msgHdr->nlmsg_type == NLMSG_ERROR)
      {
         struct nlmsgerr *errmsg = NLMSG_DATA(nl_msgHdr);
         printf("ERROR Message %d, sent by portid %x generated an error %d\n",
                errmsg->msg.nlmsg_type, errmsg->msg.nlmsg_pid, errmsg->error);
         continue;
      }

      if ((nl_msgHdr->nlmsg_type < BCM_MCAST_MSG_BASE) ||
          (nl_msgHdr->nlmsg_type >= BCM_MCAST_MSG_MAX))
      {
         continue;
      }

      pdata = NLMSG_DATA(nl_msgHdr);
      data_len = nl_msgHdr->nlmsg_len;
      process_func(nl_msgHdr->nlmsg_type, pdata, data_len);
   }
   return 0;
}

static int bcm_mcast_api_nl_tx(int sock_nl_in, int msg_type, unsigned char *pBuf, unsigned int len, unsigned char *pBuf2, unsigned int len2, int set)
{
   struct sockaddr_nl addr, dest_addr;
   socklen_t          addrLen = sizeof(addr);
   int                bufSize;
   struct nlmsghdr   *nlh;
   struct iovec       iov[5];
   int                iovcnt;
   struct msghdr      msg;
   int                ret = 0;
   int                sock_nl_local;
   char               buf[NLMSG_HDRLEN];
   char               pad[NLMSG_ALIGNTO];

   if ( ((len > 0)  && (NULL == pBuf)) || 
        ((len == 0) && (NULL != pBuf)) )
   {
       return -EINVAL;
   }

   if ((len2 > 0)  && (NULL == pBuf2))
   {
       return -EINVAL;
   }

   if ( sock_nl_in < 0 )
   {
      if ( bcm_mcast_api_socket_create(&sock_nl_local, 0) < 0 )
      {
         return -EIO;
      }
   }
   else
   {
      sock_nl_local = sock_nl_in;
   }

   /* retrieve the port id from the socket so it can be used in nlh */
   memset(&addr, 0, sizeof(struct sockaddr_nl));
   if ( getsockname(sock_nl_local, (struct sockaddr *)&addr, &addrLen) < 0)
   {
      return -EINVAL;
   }
   if ( 0 == addr.nl_pid )
   {
      return -EINVAL;
   }

   memset(&dest_addr, 0, sizeof(struct sockaddr_nl));
   dest_addr.nl_family = AF_NETLINK;
   dest_addr.nl_pid    = 0;
   dest_addr.nl_groups = 0;

   /* Fill the netlink message header */
   bufSize = NLMSG_SPACE(len+len2);
   memset(&buf[0], 0, NLMSG_HDRLEN);
   nlh = (struct nlmsghdr *)&buf[0];
   nlh->nlmsg_len   = bufSize;
   nlh->nlmsg_type  = msg_type;
   nlh->nlmsg_seq   = 0;
   nlh->nlmsg_pid   = addr.nl_pid;
   nlh->nlmsg_flags = NLM_F_REQUEST;
   if ( set ) 
   {
      nlh->nlmsg_flags |= NLM_F_CREATE|NLM_F_REPLACE;
   }

   memset(&iov, 0, sizeof(iov));
   memset(&msg, 0, sizeof(msg));
   iovcnt = 0;
   iov[iovcnt].iov_base = (void *)nlh;
   iov[iovcnt].iov_len  = NLMSG_HDRLEN;
   iovcnt++;
   iov[iovcnt].iov_base = (void *)pBuf;
   iov[iovcnt].iov_len  = len;
   iovcnt++;

   if (len2)
   {
      iov[iovcnt].iov_base = (void *)pBuf2;
      iov[iovcnt].iov_len  = len2;
      iovcnt++;
   }

   /* make sure message is aligned */
   if ( bufSize > (NLMSG_HDRLEN + len + len2) )
   {
      iov[iovcnt].iov_base = (void *)&pad;
      iov[iovcnt].iov_len  = bufSize - (NLMSG_HDRLEN + len + len2);
      iovcnt++;
   }
   msg.msg_name       = (void *)&dest_addr;
   msg.msg_namelen    = sizeof(dest_addr);
   msg.msg_iov        = iov;
   msg.msg_iovlen     = iovcnt;
   msg.msg_control    = NULL;
   msg.msg_controllen = 0;
   msg.msg_flags      = 0;

   if (sendmsg(sock_nl_local, &msg, 0) < 0)
   {
      ret = -EIO;
   }

   if ( sock_nl_in < 0 )
   {
      close(sock_nl_local);
   }

   return ret;
} /* bcm_mcast_api_nl_tx */

static int bcm_mcast_api_nl_wait_rsp(int nl_sock, int match_type, unsigned char *rx_buf, int rx_bufsize)
{
   int           ret;
   struct pollfd pfd;

   pfd.fd      = nl_sock;
   pfd.events  = POLLIN;
   pfd.revents = 0;

   /* wait up to 5 seconds for a response */
   ret = poll(&pfd, 1, 5000);
   if (-1 == ret)
   {
      printf("bcm_mcast_api_nl_wait_rsp: poll returned an error %d, %s\n", errno, strerror(errno));
      return ret;
   }
   else if (0 == ret)
   {
      printf("bcm_mcast_api_nl_wait_rsp: poll timed out\n");
      return -1;
   }
   else
   {
      int                   recvLen;
      struct iovec          iov;
      struct msghdr         msg;
      struct nlmsghdr      *nl_msgHdr;
      struct sockaddr_nl    sa;

      if ( pfd.revents & ~(POLLIN|POLLHUP|POLLPRI))
      {
         printf("bcm_mcast_api_nl_wait_rsp: unexpected socket event %x\n", pfd.revents);
         return -1;
      }

      memset(&msg, 0, sizeof(struct msghdr));
      msg.msg_name    = (void*)&sa;
      msg.msg_namelen = sizeof(sa);
      msg.msg_iov     = &iov;
      msg.msg_iovlen  = 1;
 
      iov.iov_base = (void *)rx_buf;
      iov.iov_len = rx_bufsize;

      recvLen = recvmsg(nl_sock, &msg, 0);
      if(recvLen <= 0)
      {
          return -EIO;
      }

      ret = -1;
      if (msg.msg_flags & MSG_TRUNC)
      {
         return -ENOMEM;
      }

      nl_msgHdr = (struct nlmsghdr *)rx_buf;
      if ( NLMSG_OK(nl_msgHdr, (unsigned int)recvLen) )
      {
         if (nl_msgHdr->nlmsg_type != match_type)
         {
            if (nl_msgHdr->nlmsg_type == NLMSG_ERROR)
            {
               struct nlmsgerr *errmsg = NLMSG_DATA(nl_msgHdr);
               printf("bcm_mcast_api_nl_wait_rsp: ERROR with message %d,"
                      "sent by portid %x, error %d\n",
                      errmsg->msg.nlmsg_type, errmsg->msg.nlmsg_pid, errmsg->error);
            }
            else
            {
               printf("bcm_mcast_api_nl_wait_rsp: expected %d received %d\n", match_type, nl_msgHdr->nlmsg_type);
            }
         }
         else
         {
            ret = 0;
         }
      }
   }
   return ret;
} /* bcm_mcast_api_nl_wait_rsp */

int bcm_mcast_api_register(int sock_nl, int primary)
{
   t_BCM_MCAST_REGISTER msg;

   msg.primary = primary;
   msg.result = 0;

   return bcm_mcast_api_nl_tx(sock_nl, BCM_MCAST_MSG_REGISTER, (unsigned char *)&msg, sizeof(msg), NULL, 0, 1);
} /* bcm_mcast_api_register */

int bcm_mcast_api_unregister(int sock_nl, int primary)
{
   t_BCM_MCAST_REGISTER msg;

   msg.primary = primary;
   msg.result = 0;

   return bcm_mcast_api_nl_tx(sock_nl, BCM_MCAST_MSG_UNREGISTER, (unsigned char *)&msg, sizeof(msg), NULL, 0, 1);
} /* bcm_mcast_api_unregister */

int bcm_mcast_api_uplink(int sock_nl, int uplink)
{
   t_BCM_MCAST_UPLINK msg;

   msg.uplink = uplink;

   return bcm_mcast_api_nl_tx(sock_nl, BCM_MCAST_MSG_UPLINK_INDICATION, (unsigned char *)&msg, sizeof(msg), NULL, 0, 1);
} /* bcm_mcast_api_uplink */

int bcm_mcast_api_set_priority_queue(int sock_nl, int pri_queue)
{
   t_BCM_MCAST_PRIORITY_QUEUE msg;

   msg.pri_queue = pri_queue;

   return bcm_mcast_api_nl_tx(sock_nl, BCM_MCAST_MSG_SET_PRI_QUEUE, (unsigned char *)&msg, sizeof(msg), NULL, 0, 1);
} /* bcm_mcast_api_set_priority_queue */

int bcm_mcast_api_admission_filter(int sock_nl, unsigned char igmpEn, unsigned char mldEn)
{
   t_BCM_MCAST_ADMISSION_FILTER msg;

   msg.igmpAdmissionEn = igmpEn;
   msg.mldAdmissionEn  = mldEn;

   return bcm_mcast_api_nl_tx(sock_nl, BCM_MCAST_MSG_CONTROLS_ADMISSION, (unsigned char *)&msg, sizeof(msg), NULL, 0, 1);
} /* bcm_mcast_api_admission_filter */

int bcm_mcast_api_admission_result(int sock_nl, int ifi, int admitted, uintptr_t packetIndex, int proto)
{
   t_BCM_MCAST_ADMISSION_RESULT msg;

   msg.parent_ifi = ifi;
   msg.admitted = admitted;
   msg.packetIndex = packetIndex;
   msg.proto = proto;
   return bcm_mcast_api_nl_tx(sock_nl, BCM_MCAST_MSG_ADMISSION_RESULT, (unsigned char *)&msg, sizeof(msg), NULL, 0, 1);
} /* bcm_mcast_api_admission_result */

int bcm_mcast_api_if_change(int sock_nl, int ifi, int proto)
{
   t_BCM_MCAST_IF_CHANGE chg;
   
   chg.ifi = ifi;
   chg.proto = proto;
   return bcm_mcast_api_nl_tx(sock_nl, BCM_MCAST_MSG_IF_CHANGE, (unsigned char *)&chg, sizeof(chg), NULL, 0, 1);
} /* bcm_mcast_api_if_change */

int bcm_mcast_api_ipv4_purge_reporter(int sock_nl, int ifi, int dstdev_ifi, struct in_addr *rep)
{
   t_BCM_MCAST_IGMP_PURGE_REPORTER msg;

   msg.parent_ifi = ifi;
   msg.dstdev_ifi = dstdev_ifi;
   msg.rep.s_addr = rep->s_addr;

   return bcm_mcast_api_nl_tx(sock_nl, BCM_MCAST_MSG_IGMP_PURGE_REPORTER, (unsigned char *)&msg, sizeof(msg), NULL, 0, 1);
} /* bcm_mcast_api_ipv4_purge_reporter */

int bcm_mcast_api_ipv6_purge_reporter(int sock_nl, int ifi, int dstdev_ifi, struct in6_addr *rep)
{
   t_BCM_MCAST_MLD_PURGE_REPORTER msg;

   msg.parent_ifi = ifi;
   msg.dstdev_ifi = dstdev_ifi;
   msg.rep.s6_addr32[0] = rep->s6_addr32[0];
   msg.rep.s6_addr32[1] = rep->s6_addr32[1];
   msg.rep.s6_addr32[2] = rep->s6_addr32[2];
   msg.rep.s6_addr32[3] = rep->s6_addr32[3];

   return bcm_mcast_api_nl_tx(sock_nl, BCM_MCAST_MSG_MLD_PURGE_REPORTER, (unsigned char *)&msg, sizeof(msg), NULL, 0, 1);
} /* bcm_mcast_api_ipv6_purge_reporter */

int bcm_mcast_api_fdb_cleanup(int sock_nl)
{
   return bcm_mcast_api_nl_tx(sock_nl, BCM_MCAST_MSG_MC_FDB_CLEANUP, NULL, 0, NULL, 0, 1);
} /* bcm_mcast_api_fdb_cleanup */

int bcm_mcast_api_set_snooping_cfg(int sock_nl, int ifi, int proto, int mode, int l2lenable)
{
   t_BCM_MCAST_SNOOP_CFG msg;

   msg.ifi = ifi;
   msg.proto = proto;
   msg.mode = mode;
   msg.l2lenable = l2lenable;
   return bcm_mcast_api_nl_tx(sock_nl, BCM_MCAST_MSG_SNOOP_CFG, (unsigned char *)&msg, sizeof(msg), NULL, 0, 1);
}

int bcm_mcast_api_set_proto_rate_limit(int sock_nl, int ifi, int proto, int rate)
{
   t_BCM_MCAST_PROTO_RATE_LIMIT msg;

   msg.ifi = ifi;
   msg.proto = proto;
   msg.rate = rate;
   return bcm_mcast_api_nl_tx(sock_nl, BCM_MCAST_MSG_PROTO_RATE_LIMIT_CFG, (unsigned char *)&msg, sizeof(msg), NULL, 0, 1);
}

int bcm_mcast_api_snooping_exception (int sock_nl, int proto, int count, t_BCM_MCAST_IGNORE_GROUP_ENTRY *groupEntries)
{
  t_BCM_MCAST_IGNORE_GROUP_MESSAGE entryList;

  entryList.proto = proto;
  entryList.count = count;

  return bcm_mcast_api_nl_tx(sock_nl, BCM_MCAST_MSG_IGNORE_GROUP_LIST, 
                             (unsigned char *) &entryList, sizeof(entryList), 
                             (unsigned char *) groupEntries, sizeof(groupEntries[0]) * count, 1 );
}


int bcm_mcast_api_send_group_timeout (int sock_nl, int proto, int generalMembershipTimeoutSecs)
{
  t_BCM_MCAST_TIMEOUT_ENTRY timeoutEntry;
  timeoutEntry.proto = proto;
  timeoutEntry.generalMembershipTimeoutSecs = generalMembershipTimeoutSecs;

  return bcm_mcast_api_nl_tx(sock_nl, BCM_MCAST_MSG_SET_TIMEOUT, 
                             (unsigned char *) &timeoutEntry, sizeof(timeoutEntry), 
                             NULL, 0, 1 );
}

int bcm_mcast_api_igmp_drop_group(int sock_nl, int ifi, int rep_ifi, struct in_addr *gpAddr)
{
   t_BCM_MCAST_IGMP_DROP_GROUP_ENTRY dropMsg;

   dropMsg.parent_ifindex = ifi;
   dropMsg.dest_ifindex = rep_ifi;
   dropMsg.group.s_addr = gpAddr->s_addr;

   return bcm_mcast_api_nl_tx(sock_nl, BCM_MCAST_MSG_IGMP_DROP_GROUP, 
                              (unsigned char *) &dropMsg, sizeof(dropMsg), 
                              NULL, 0, 1);
}

int bcm_mcast_api_mld_drop_group(int sock_nl, int ifi, int rep_ifi, struct in6_addr *gpAddr)
{
   t_BCM_MCAST_MLD_DROP_GROUP_ENTRY dropMsg;

   dropMsg.parent_ifindex = ifi;
   dropMsg.dest_ifindex = rep_ifi;
   memcpy(&dropMsg.group, gpAddr, sizeof(struct in6_addr));

   return bcm_mcast_api_nl_tx(sock_nl, BCM_MCAST_MSG_MLD_DROP_GROUP, 
                              (unsigned char *) &dropMsg, sizeof(dropMsg), 
                              NULL, 0, 1);
}

int bcm_mcast_api_blog_enable(int sock_nl, int enable)
{
   t_BCM_MCAST_BLOG_ENABLE msg;

   msg.blog_enable = enable;

   return bcm_mcast_api_nl_tx(sock_nl, BCM_MCAST_MSG_BLOG_ENABLE, (unsigned char *)&msg, sizeof(msg), NULL, 0, 1);
} /* bcm_mcast_api_set_priority_queue */

/*
 * Add/Update IGMP snooping entry
 */
int bcm_mcast_api_update_igmp_snoop(int                    sock_nl, 
                                    int                    parent_ifi, 
                                    int                    dstdev_ifi,
                                    int                    to_acceldev_ifi,
                                    unsigned short         vid, 
                                    int                    lanppp, 
                                    const struct in_addr  *rxGrp, 
                                    const struct in_addr  *txGrp, 
                                    const struct in_addr  *src, 
                                    const struct in_addr  *rep, 
                                    unsigned char         *repMac, 
                                    unsigned char          rep_proto_ver,
                                    int                    filter_mode, 
                                    t_BCM_MCAST_WAN_INFO_ARRAY *wan_info,
                                    int                    excludePort, 
                                    char                   enRtpSeqCheck)
{
   t_BCM_MCAST_IGMP_SNOOP_ENTRY snoopEntry;

   if (!rxGrp || !txGrp || !src || !rep || !wan_info)
   {
    return -EINVAL;
   }

   if (!(IN_MULTICAST(ntohl(txGrp->s_addr)) ||
       SNOOP_IN_IS_ADDR_L2_MCAST(txGrp)) ||
       ((txGrp->s_addr & ntohl(0xFFFFFF00)) == ntohl(0xE0000000)))
   {
       return -EINVAL;
   }
 
   if (!(IN_MULTICAST(ntohl(rxGrp->s_addr)) ||
       SNOOP_IN_IS_ADDR_L2_MCAST(rxGrp)) ||
       ((rxGrp->s_addr & ntohl(0xFFFFFF00)) == ntohl(0xE0000000)) )
   {
       return -EINVAL;
   }
 
   memset(&snoopEntry, 0, sizeof(t_BCM_MCAST_IGMP_SNOOP_ENTRY));
   snoopEntry.parent_ifi = parent_ifi;
   snoopEntry.dstdev_ifi = dstdev_ifi;
   snoopEntry.to_acceldev_ifi = to_acceldev_ifi;
   memcpy(&snoopEntry.rxGrp, rxGrp, sizeof(struct in_addr));
   memcpy(&snoopEntry.txGrp, txGrp, sizeof(struct in_addr));
   memcpy(&snoopEntry.src, src, sizeof(struct in_addr));
   memcpy(&snoopEntry.rep, rep, sizeof(struct in_addr));
   memcpy(&snoopEntry.repMac, repMac, ETH_ALEN);
   memcpy(&snoopEntry.wan_info, wan_info, sizeof(t_BCM_MCAST_WAN_INFO_ARRAY));
   snoopEntry.excludePort = excludePort;
   snoopEntry.enRtpSeqCheck = enRtpSeqCheck;
   snoopEntry.mode = filter_mode;
   snoopEntry.tci = vid;
   snoopEntry.lanppp = lanppp;
   snoopEntry.rep_proto_ver = rep_proto_ver;

   return bcm_mcast_api_nl_tx(sock_nl, BCM_MCAST_MSG_IGMP_SNOOP_ENTRY, 
                              (unsigned char *)&snoopEntry, 
                              sizeof(snoopEntry),  NULL, 0, 1);
}

/*
 * Add/Update MLD snooping entry
 */
int bcm_mcast_api_update_mld_snoop(int                    sock_nl, 
                                   int                    parent_ifi, 
                                   int                    dstdev_ifi,
                                   int                    to_acceldev_ifi,
                                   unsigned short         vid, 
                                   int                    lanppp, 
                                   const struct in6_addr *grp, 
                                   const struct in6_addr *src, 
                                   const struct in6_addr *rep, 
                                   unsigned char         *repMac,
                                   unsigned char          rep_proto_ver,
                                   int                    filter_mode, 
                                   t_BCM_MCAST_WAN_INFO_ARRAY *wan_info)
{
   t_BCM_MCAST_MLD_SNOOP_ENTRY  snoopEntry6;

   if (!grp || !src || !rep || !wan_info)
   {
      return -EINVAL;
   }

   if(!(SNOOP_IN6_IS_ADDR_MULTICAST(grp) ||
       SNOOP_IN6_IS_ADDR_L2_MCAST(grp)) ||
       (SNOOP_IN6_IS_ADDR_MC_SCOPE0(grp)) ||
       (SNOOP_IN6_IS_ADDR_MC_NODELOCAL(grp)) ||
       (SNOOP_IN6_IS_ADDR_MC_LINKLOCAL(grp)))
   {
       return -EINVAL;
   }

   memset(&snoopEntry6, 0, sizeof(t_BCM_MCAST_MLD_SNOOP_ENTRY));
   snoopEntry6.parent_ifi = parent_ifi;
   snoopEntry6.dstdev_ifi = dstdev_ifi;
   snoopEntry6.to_acceldev_ifi = to_acceldev_ifi;
   memcpy(&snoopEntry6.grp, grp, sizeof(struct in6_addr));
   memcpy(&snoopEntry6.src, src, sizeof(struct in6_addr));
   memcpy(&snoopEntry6.rep, rep, sizeof(struct in6_addr));
   memcpy(&snoopEntry6.repMac, repMac, ETH_ALEN);
   memcpy(&snoopEntry6.wan_info, wan_info, sizeof(t_BCM_MCAST_WAN_INFO_ARRAY));
   snoopEntry6.mode = filter_mode;
   snoopEntry6.tci = vid;
   snoopEntry6.lanppp = lanppp;
   snoopEntry6.rep_proto_ver = rep_proto_ver;

   return bcm_mcast_api_nl_tx(sock_nl, BCM_MCAST_MSG_MLD_SNOOP_ENTRY, 
                              (unsigned char *)&snoopEntry6, 
                              sizeof(snoopEntry6), NULL, 0, 1);
}

int bcm_mcast_api_get_snooping_cfg(int ifi, int proto, int *mode, int *l2lenable)
{
   t_BCM_MCAST_SNOOP_CFG  msg;
   t_BCM_MCAST_SNOOP_CFG *prsp = NULL;
   int                    rsp_len = 0;
   unsigned char          buf[NLMSG_SPACE(sizeof(*prsp)) + sizeof(struct nlmsgerr)];
   int                    ret;
   int                    nl_sock;
   struct nlmsghdr       *nl_msgHdr;

   if ( (NULL == mode) || (NULL == l2lenable) )
   {
      return -1;
   }

   ret = bcm_mcast_api_socket_create(&nl_sock, 0);
   if ( ret < 0 )
   {
      return -1;
   }

   msg.proto = proto;
   msg.ifi = ifi;
   ret = bcm_mcast_api_nl_tx(nl_sock, BCM_MCAST_MSG_SNOOP_CFG, (unsigned char *)&msg, sizeof(msg), NULL, 0, 0);
   if ( ret < 0 )
   {
      close(nl_sock);
      return ret;
   }

   ret = bcm_mcast_api_nl_wait_rsp(nl_sock, BCM_MCAST_MSG_SNOOP_CFG, &buf[0], sizeof(buf));
   if ( ret < 0 )
   {
      close(nl_sock);
      return ret;
   }

   nl_msgHdr = (struct nlmsghdr *)&buf[0];
   prsp = (t_BCM_MCAST_SNOOP_CFG *)NLMSG_DATA(nl_msgHdr);
   rsp_len = nl_msgHdr->nlmsg_len;
   close(nl_sock);
   if ( rsp_len < NLMSG_SPACE(sizeof(*prsp)) )
   {
      return -1;
   }
   *l2lenable = prsp->l2lenable;
   *mode = prsp->mode;
   return 0;
}

int bcm_mcast_api_get_proto_rate_limit(int ifi, int proto, int *rate)
{
   t_BCM_MCAST_PROTO_RATE_LIMIT  msg;
   t_BCM_MCAST_PROTO_RATE_LIMIT *prsp = NULL;
   int                           rsp_len = 0;
   unsigned char                 buf[NLMSG_SPACE(sizeof(*prsp)) + sizeof(struct nlmsgerr)];
   int                           ret;
   int                           nl_sock;
   struct nlmsghdr              *nl_msgHdr;

   if ( NULL == rate )
   {
      return -1;
   }

   ret = bcm_mcast_api_socket_create(&nl_sock, 0);
   if ( ret < 0 )
   {
      return -1;
   }

   msg.proto = proto;
   msg.ifi = ifi;
   ret = bcm_mcast_api_nl_tx(nl_sock, BCM_MCAST_MSG_PROTO_RATE_LIMIT_CFG, (unsigned char *)&msg, sizeof(msg), NULL, 0, 0);
   if ( ret < 0 )
   {
      close(nl_sock);
      return ret;
   }

   ret = bcm_mcast_api_nl_wait_rsp(nl_sock, BCM_MCAST_MSG_PROTO_RATE_LIMIT_CFG, &buf[0], sizeof(buf));
   if ( ret < 0 )
   {
      close(nl_sock);
      return ret;
   }

   nl_msgHdr = (struct nlmsghdr *)&buf[0];
   prsp = (t_BCM_MCAST_PROTO_RATE_LIMIT*)NLMSG_DATA(nl_msgHdr);
   rsp_len = nl_msgHdr->nlmsg_len;
   close(nl_sock);
   if ( rsp_len < NLMSG_SPACE(sizeof(*prsp)) )
   {
      return -1;
   }
   *rate = prsp->rate;
   return 0;
}

/*
 * Add/Update OvS Bridge Info 
 */
int bcm_mcast_api_update_ovs_brcfg(int sock_nl, t_ovs_mcpd_brcfg_info *pBrcfg)
{
   return bcm_mcast_api_nl_tx(sock_nl, BCM_MCAST_MSG_OVS_BRINFO_UPDATE, (unsigned char *)pBrcfg, sizeof(*pBrcfg), NULL, 0, 1);
} 

int bcm_mcast_api_stream_client_sock_connect(int port) 
{
  struct sockaddr_in sa = { 0 };
  int                rc;
  int                sd;
  int                flags;

  sd = socket(AF_INET, SOCK_STREAM, 0);
  if (sd == -1) 
  {
    printf("socket() error, %s\n", strerror(errno));
    return -1;
  }

  /* Connect to server */
  sa.sin_family      = AF_INET;
  sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  sa.sin_port        = htons((unsigned short)port);
  rc = connect(sd, (struct sockaddr *)&sa, sizeof(struct sockaddr_in));
  if (rc == -1) 
  {
    if (errno == EACCES)
      printf("server socket not present\n");
    else
      printf("connect to port %d failed, errno=%d\n", port, errno);
    close(sd);
    return -1;
  }

  flags = fcntl(sd, F_GETFL, 0);
  if(flags < 0) 
  {
    printf("cannot retrieve socket flags. error=%s", strerror(errno));
  }
  if ( fcntl(sd, F_SETFL, flags | O_NONBLOCK) < 0 ) {
    printf("cannot set socket to non-blocking. error=%s", strerror(errno));
  }

  return sd;
}

int bcm_mcast_api_stream_sock_send(int sd, void *data, size_t datalen) 
{
    struct msghdr msg = { 0 };
    struct iovec iov;
    int rc;

    iov.iov_base = data;
    iov.iov_len = datalen;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    rc = sendmsg(sd, &msg, MSG_NOSIGNAL);
    if (rc == -1)
    {
        printf("send failed, errno=%d\n", errno);
        return -1;
    }
    return rc;
}

int bcm_mcast_api_stream_sock_close(int sd) 
{
    return close(sd);
}

int bcm_mcast_api_receive_message(int sd, void *buf, size_t buflen) 
{
    int     length;
    fd_set  rfds;
    struct  timeval timeout;
    int     retval;

    FD_ZERO(&rfds); 
    FD_SET(sd, &rfds); 

    /* Timeout set to 100ms.
       Timeout value has been set after testing various MCPD scenarios
       Though a value of 10ms worked most of the time, rcv timeouts were
       noticed during "mcp reload" when ovs forwards message to mcpd
       and waits for response. Specifically, mcpd_ovs_set_snoop_aging_time()
       gets invoked during "mcp reload" and it calls the system() command
       which blocks the MCPD. 50ms of timeout was sufficient in this case.
       However an additional buffering of 50ms was added and
       100ms was chosen for timeout */
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;

    retval = select(sd + 1, &rfds, NULL, NULL, &timeout);

    if (retval <= 0) {
        /* Error or connection timed out */
        printf("%s connection timed out retval %d error %s\n", __func__, retval, strerror(errno));
        return -1;
    }
    else
    {
        length = recv(sd, buf, buflen, 0);
        if (length < 0) {
            printf("%s length < 0\n", __func__);
           return -1;
        }
        else if (length == 0) {
            /* socket has been closed */
            printf("%s socket has been closed\n", __func__);
            close(sd);
            return -1;
        }
    }
    return 0;
}

int bcm_mcast_api_send_sock_msg(int port, void *pBuf, int buf_size)
{
    int bcmret = -1;
    int sd;

    sd = bcm_mcast_api_stream_client_sock_connect(port);
    if (sd < 0) 
    {
        printf("%s bcm_mcast_api_stream_client_sock_connect() failed\n", __func__);
        return bcmret;
    }
    bcmret = bcm_mcast_api_stream_sock_send(sd, pBuf, buf_size);

    bcm_mcast_api_stream_sock_close(sd);
    return bcmret;
}

int bcm_mcast_api_send_sock_msg_wait_rsp(int port, void *pBuf, int buf_size, void *rcv_buf, int rcv_bufsize)
{
    int bcmret = -1;
    int sd;

    sd = bcm_mcast_api_stream_client_sock_connect(port);
    if (sd < 0) 
    {
        printf("%s bcm_mcast_api_stream_client_sock_connect() failed\n", __func__); 
        return bcmret;
    }

    if ((bcmret = bcm_mcast_api_stream_sock_send(sd, pBuf, buf_size)) != -1)
    {
        bcmret = bcm_mcast_api_receive_message(sd, rcv_buf, rcv_bufsize);
    }

    bcm_mcast_api_stream_sock_close(sd);
    return bcmret;
}

int bcm_mcast_api_ifname_to_idx(char *ifname, int *ifIndex)
{
    int sockfd = -1;
    struct ifreq ifr;

    if (!ifname || !ifIndex) 
    {
        return -1;
    }

    sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        printf("can't open ipv4 socket");
        return -1;
    }

    memset(&ifr, 0, sizeof(struct ifreq));
    strcpy(ifr.ifr_name, ifname);
    if (ioctl(sockfd, SIOCGIFINDEX, &ifr) < 0)
    {
        close(sockfd);
        return -1;
    }

    *ifIndex = ifr.ifr_ifindex;
    close(sockfd);
    return 0;
}

char * bcm_mcast_api_ifindex_to_name(unsigned int ifindex, char *ifname)
{
    struct ifreq ifr;
    int status;
    int sockfd = -1;

    if (!ifname || !ifindex) 
    {
        return NULL;
    }

    sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        printf("%s can't open ipv4 socket\n", __func__);
        return NULL;
    }

    memset(&ifr, 0, sizeof(struct ifreq));
    ifr.ifr_ifindex = ifindex;

    status = ioctl(sockfd, SIOCGIFNAME, &ifr);

    close(sockfd);
    if (status < 0)
    {
        return NULL;
    }
    else
        return strncpy(ifname,ifr.ifr_name,IFNAMSIZ);
}
