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
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <dirent.h>
#include <sys/stat.h>
#include <bcm_local_kernel_include/linux/sockios.h>
#include <bcm_local_kernel_include/linux/if_bridge.h>
#include <linux/netlink.h>
#include <linux/neighbour.h>
#include <linux/rtnetlink.h>
#include "bridgeutil.h"
#include <linux/version.h>

#define SYSFS_PATH_MAX        256
#define MAX_FDB_READ_PER_ITER 64

static int get_ifindex_from_name(const char *ifname)
{
   char           sfspath[SYSFS_PATH_MAX];
   int            rt;
   FILE          *fp;
   int            ifindex = 0; 

   snprintf(sfspath, SYSFS_PATH_MAX, "/sys/class/net/%s/ifindex", ifname);
   fp = fopen(sfspath, "r");
   if (fp != NULL)
   {
      rt = fscanf(fp, "%i", &ifindex);
      fclose(fp);
      if (rt == 1)
      {
         return ifindex;
      }
   }
   return 0;
}

int br_util_is_bridge(const char *brname)
{
   char        sfspath[SYSFS_PATH_MAX];
   struct stat st;
 
   snprintf(sfspath, SYSFS_PATH_MAX, "/sys/class/net/%s/bridge", brname);
   if ( (0 == stat(sfspath, &st)) && S_ISDIR(st.st_mode) )
   {
     return 1;
   }
   return 0;
}

int br_util_get_port_number(const char *ifname)
{
   char  sfspath[SYSFS_PATH_MAX];
   int   port_num;
   FILE *f;

   snprintf(sfspath, SYSFS_PATH_MAX, "/sys/class/net/%s/brport/port_no", ifname);
   f = fopen(sfspath, "r");
   if ( f ) 
   {
      int rc = fscanf(f, "%x", &port_num);
      fclose(f);
      if (rc == 1)
      {
         return port_num;
      }
   }
 
   return -EINVAL;
}

int br_util_get_port_name(const char *brname, int port_num, char *ifname, int maxlen)
{
   DIR           *dp;
   struct dirent *ep;
   char           sfspath[SYSFS_PATH_MAX];
   int            rt;
   int            ret = -1;
   FILE          *fp;
   int            portno;

   snprintf(sfspath, SYSFS_PATH_MAX, "/sys/class/net/%s/brif", brname);
   dp = opendir(sfspath);
   if (dp != NULL)
   {
      while ((ep = readdir(dp)) != NULL)
      {
         if ((0 == strcmp(ep->d_name, ".")) || (0 == strcmp(ep->d_name, "..")))
         {
            continue;
         }

         snprintf(sfspath, SYSFS_PATH_MAX, "/sys/class/net/%s/brport/port_no", ep->d_name);
         fp = fopen(sfspath, "r");
         if (fp != NULL)
         {
            rt = fscanf(fp, "%i", &portno);
            fclose(fp);
            if ( rt != 0 )
            {
               if ( portno == port_num )
               {
                  strncpy(ifname, ep->d_name, ((maxlen < IFNAMSIZ) ? maxlen : IFNAMSIZ));
                  ret = 0;
                  break;
               }
            }
         }
      }
      closedir (dp);
   }

   return ret;
}

int br_util_get_bridges(int ifindices[], unsigned int *num)
{
   DIR           *dp;
   struct dirent *ep;
   char           sfspath[SYSFS_PATH_MAX];
   int            bridx = 0;

   snprintf(sfspath, SYSFS_PATH_MAX, "/sys/class/net");
   dp = opendir(sfspath);
   if (dp != NULL)
   {
      while (((ep = readdir(dp)) != NULL) && (bridx < *num))
      {
         if ((0 == strcmp(ep->d_name, ".")) || (0 == strcmp(ep->d_name, "..")))
         {
            continue;
         }

         if ( br_util_is_bridge(ep->d_name) )
         {
            /* bridge interface */
            FILE         *fp;
            unsigned int  brifindex; 
            int           rt;

            snprintf(sfspath, SYSFS_PATH_MAX, "/sys/class/net/%s/ifindex", ep->d_name);
            fp = fopen(sfspath, "r");
            if (fp != NULL)
            {
               rt = fscanf(fp, "%u", &brifindex);
               fclose(fp);
               if ( rt != 0 )
               {
                  ifindices[bridx] = brifindex;
                  bridx++;
               }
            }
         }
      }
      closedir (dp);
   }
   *num = bridx;
   return 0;
}

int br_util_get_bridge_wan_interfaces(const char *brName, int ifindices[], unsigned int *num)
{
   DIR           *dp;
   struct dirent *ep;
   char           sfspath[SYSFS_PATH_MAX];
   int            socket_fd;
   int            rt;
   struct ifreq   ifr_port;
   int            wanidx = 0;
   FILE          *fp;
   int            ifindex; 

   socket_fd = socket(AF_INET, SOCK_STREAM, 0);
   if (socket_fd < 0)
   {
       return -EIO;
   }

   if ( 0 == br_util_is_bridge(brName) )
   {
      close(socket_fd);
      return -EINVAL;
   }

   snprintf(sfspath, SYSFS_PATH_MAX, "/sys/class/net/%s/brif", brName);
   dp = opendir(sfspath);
   if (dp != NULL)
   {
      while (((ep = readdir(dp)) != NULL) && (wanidx < *num))
      {
         if ((0 == strcmp(ep->d_name, ".")) || (0 == strcmp(ep->d_name, "..")))
         {
            continue;
         }

         snprintf(sfspath, SYSFS_PATH_MAX, "/sys/class/net/%s/ifindex", ep->d_name);
         fp = fopen(sfspath, "r");
         if (fp != NULL)
         {
            rt = fscanf(fp, "%i", &ifindex);
            fclose(fp);
            if ( rt != 0 )
            {
               memset(&ifr_port, 0, sizeof(struct ifreq));
               strncpy(ifr_port.ifr_name, ep->d_name, IFNAMSIZ);
               rt = ioctl(socket_fd, SIOCDEVISWANDEV, (void*)&ifr_port);
               if ((rt >= 0) && (1 == ifr_port.ifr_flags))
               {
                  ifindices[wanidx] = ifindex;
                  wanidx++;
               }
            }
         }
      }
      closedir (dp);
   }
   close(socket_fd);
   *num = wanidx;
   return 0;
}

int br_util_get_bridge_port_interfaces(const char *brName, int ifindices[], unsigned int *num)
{
   DIR           *dp;
   struct dirent *ep;
   char           sfspath[SYSFS_PATH_MAX];
   int            rt;
   int            prtidx = 0;
   FILE          *fp;
   int            ifindex; 

   snprintf(sfspath, SYSFS_PATH_MAX, "/sys/class/net/%s/brif", brName);
   dp = opendir(sfspath);
   if (dp != NULL)
   {
      while (((ep = readdir(dp)) != NULL) && (prtidx < *num))
      {
         if ((0 == strcmp(ep->d_name, ".")) || (0 == strcmp(ep->d_name, "..")))
         {
            continue;
         }

         snprintf(sfspath, SYSFS_PATH_MAX, "/sys/class/net/%s/ifindex", ep->d_name);
         fp = fopen(sfspath, "r");
         if (fp != NULL)
         {
            rt = fscanf(fp, "%i", &ifindex);
            fclose(fp);
            if ( rt != 0 )
            {
               ifindices[prtidx] = ifindex;
               prtidx++;
            }
         }
      }
      closedir (dp);
   }
   *num = prtidx;
   return 0;
}

int br_util_read_fdb(const char *ifname, struct __fdb_entry *fdbs, int offset, int num)
{
   FILE *f;
   int   cnt = 0;
   char  sfspath[SYSFS_PATH_MAX];
   
   snprintf(sfspath, SYSFS_PATH_MAX, "/sys/class/net/%s/brforward", ifname);
   f = fopen(sfspath, "r");
   if (f)
   {
      fseek(f, offset*sizeof(struct __fdb_entry), SEEK_SET);
      cnt = fread(fdbs, sizeof(struct __fdb_entry), num, f);
      fclose(f);
   }

   return cnt;
}

int br_util_add_fdb_entry(const char *ifname, const unsigned char *macaddr, int type)
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(3,8,0)
   struct nlmsghdr   *nlh;
   struct ndmsg      *ndm;
   struct rtattr     *rta;
   char               buf[NLMSG_HDRLEN + NLMSG_ALIGN(sizeof(*ndm)) + 
                          RTA_ALIGN(sizeof(*rta)) + RTA_ALIGN(ETH_ALEN)];
   int                fd;

   memset(&buf[0], 0, sizeof(buf));
   nlh = (struct nlmsghdr *)&buf[0];
   nlh->nlmsg_len = sizeof(buf);
   nlh->nlmsg_flags = NLM_F_REQUEST|NLM_F_CREATE|NLM_F_REPLACE; /* create or replace */
   nlh->nlmsg_type = RTM_NEWNEIGH;
   nlh->nlmsg_seq = 0;

   ndm = (struct ndmsg *)NLMSG_DATA(nlh);
   ndm->ndm_ifindex = get_ifindex_from_name(ifname);
   if ( 0 == ndm->ndm_ifindex )
   {
      return -1;
   }
   ndm->ndm_family = PF_BRIDGE;
   ndm->ndm_flags  = NTF_MASTER;
   if (type == BR_UTIL_FDB_TYPE_STATIC )
   {
      ndm->ndm_state = NUD_NOARP;
   }
   else
   {
      ndm->ndm_state = NUD_REACHABLE;
   }

   rta = (struct rtattr *)(NLMSG_DATA(nlh) + NLMSG_ALIGN(sizeof(*ndm)));
   rta->rta_type = NDA_LLADDR;
   rta->rta_len = RTA_LENGTH(ETH_ALEN);
   memcpy(RTA_DATA(rta), macaddr, ETH_ALEN);

   fd = socket(AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, NETLINK_ROUTE);
   if (fd < 0)
   {
      printf("Cannot open netlink socket\n");
      return -1;
   }
   else
   {
      int status;
      struct sockaddr_nl nladdr;
      struct iovec iov = {
         .iov_base = (void*)nlh,
         .iov_len = nlh->nlmsg_len
      };
      struct msghdr msg = {
         .msg_name = &nladdr,
         .msg_namelen = sizeof(nladdr),
         .msg_iov = &iov,
         .msg_iovlen = 1,
      };

      memset(&nladdr, 0, sizeof(nladdr));
      nladdr.nl_family = AF_NETLINK;

      status = sendmsg(fd, &msg, 0);
      if (status < 0)
      {
         printf("Cannot talk to rtnetlink");
      }
      close(fd);
   }
#endif /* LINUX_VERSION_CODE */

   return 0;
}

int br_util_del_fdb_entry(const char *ifname, const unsigned char *macaddr)
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(3,8,0)
   struct nlmsghdr   *nlh;
   struct ndmsg      *ndm;
   struct rtattr     *rta;
   char               buf[NLMSG_HDRLEN + NLMSG_ALIGN(sizeof(*ndm)) + 
                          RTA_ALIGN(sizeof(*rta)) + RTA_ALIGN(ETH_ALEN)];
   int                fd;

   memset(&buf[0], 0, sizeof(buf));
   nlh = (struct nlmsghdr *)&buf[0];
   nlh->nlmsg_len = sizeof(buf);
   nlh->nlmsg_flags = NLM_F_REQUEST|NLM_F_CREATE|NLM_F_REPLACE; /* create or replace */
   nlh->nlmsg_type = RTM_DELNEIGH;
   nlh->nlmsg_seq = 0;

   ndm = (struct ndmsg *)NLMSG_DATA(nlh);
   ndm->ndm_ifindex = get_ifindex_from_name(ifname);
   if ( 0 == ndm->ndm_ifindex )
   {
      return -1;
   }
   ndm->ndm_family = PF_BRIDGE;
   ndm->ndm_flags  = NTF_MASTER;

   rta = (struct rtattr *)(NLMSG_DATA(nlh) + NLMSG_ALIGN(sizeof(*ndm)));
   rta->rta_type = NDA_LLADDR;
   rta->rta_len = RTA_LENGTH(ETH_ALEN);
   memcpy(RTA_DATA(rta), macaddr, ETH_ALEN);

   fd = socket(AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, NETLINK_ROUTE);
   if (fd < 0)
   {
      printf("Cannot open netlink socket\n");
      return -1;
   }
   else
   {
      int status;
      struct sockaddr_nl nladdr;
      struct iovec iov = {
         .iov_base = (void*)nlh,
         .iov_len = nlh->nlmsg_len
      };
      struct msghdr msg = {
         .msg_name = &nladdr,
         .msg_namelen = sizeof(nladdr),
         .msg_iov = &iov,
         .msg_iovlen = 1,
      };

      memset(&nladdr, 0, sizeof(nladdr));
      nladdr.nl_family = AF_NETLINK;

      status = sendmsg(fd, &msg, 0);
      if (status < 0)
      {
         printf("Cannot talk to rtnetlink");
      }
      close(fd);
   }

#endif /* LINUX_VERSION_CODE */
   return 0;
}

int br_util_flush_fdb(const char *brname, const char *ifname, int type)
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(3,8,0)
   int                port_no = -1;
   struct __fdb_entry fdb[MAX_FDB_READ_PER_ITER];
   int                offset = 0;
   int                cnt;

   if ( ifname != NULL )
   {
      port_no = br_util_get_port_number(ifname);
      if ( port_no < 0 )
      {
         return port_no;
      }
   }

   do
   {
      /* loop through and remove fdb entries */
      cnt = br_util_read_fdb(brname, fdb, offset, MAX_FDB_READ_PER_ITER);
      offset += MAX_FDB_READ_PER_ITER;
      if ( cnt )
      {
         int i;
         for ( i = 0; i < cnt; i++ )
         {
            if ( fdb[i].is_local )
            {
               continue;
            }

            if ( (port_no == -1) ||
                 (port_no == fdb[i].port_no) )
            {
               if ( (BR_UTIL_FDB_TYPE_STATIC == type) &&
                    (0 == fdb[i].ageing_timer_value ) )
               {
                  br_util_del_fdb_entry(ifname, fdb[i].mac_addr);
                  offset -= 1;
               }
               if ( (BR_UTIL_FDB_TYPE_DYNAMIC == type) &&
                    (0 != fdb[i].ageing_timer_value ) )
               {
                  br_util_del_fdb_entry(ifname, fdb[i].mac_addr);
                  offset -= 1;
               }
            }
         }
      }
   } while (cnt > 0);

#endif /* LINUX_VERSION_CODE */
   return 0;
}

int br_util_get_fdb_limit(const char *ifname, unsigned int type)
{
   char  sfspath[SYSFS_PATH_MAX];
   int   count;
   int   isbridge;
   FILE *fp;

   if ((ifname == NULL) || (type > BR_UTIL_LIMIT_TYPE_USED))
   {
      return -1;
   }

   isbridge = br_util_is_bridge(ifname);
   if ( isbridge && (type == BR_UTIL_LIMIT_TYPE_MIN))
   {
      return -1;
   }

   if ( isbridge )
   {
      if (type == BR_UTIL_LIMIT_TYPE_MAX)
      {
         /* read min */
         snprintf(sfspath, SYSFS_PATH_MAX, "/sys/class/net/%s/bridge/max_fdb_entries", ifname);
      }
      else
      {
         /* read used */
         snprintf(sfspath, SYSFS_PATH_MAX, "/sys/class/net/%s/bridge/used_fdb_entries", ifname);
      }
   }
   else
   {
      if (type == BR_UTIL_LIMIT_TYPE_MIN)
      {
         /* read max */
         snprintf(sfspath, SYSFS_PATH_MAX, "/sys/class/net/%s/brport/min_fdb_entries", ifname);
      }
      else if (type == BR_UTIL_LIMIT_TYPE_MAX)
      {
         /* read used */
         snprintf(sfspath, SYSFS_PATH_MAX, "/sys/class/net/%s/brport/max_fdb_entries", ifname);
      }
      else
      {
         /* read min */
         snprintf(sfspath, SYSFS_PATH_MAX, "/sys/class/net/%s/brport/used_fdb_entries", ifname);
      }
   }

   fp = fopen(sfspath, "r");
   if ( fp ) 
   {
      int rc = fscanf(fp, "%d", &count);
      fclose(fp);
      if (rc == 1)
      {
         return count;
      }
   }

   return -1;
}

int br_util_set_fdb_limit(const char *brname, const char *ifname, unsigned int type, unsigned int limit)
{
   char  sfspath[SYSFS_PATH_MAX];
   FILE *fp;

   if ((brname == NULL) || (type > BR_UTIL_LIMIT_TYPE_MAX))
   {
      return -1;
   }

   /* no min setting for bridge */
   if ( (ifname == NULL) && (type == BR_UTIL_LIMIT_TYPE_MIN))
   {
      return -1;
   }

   if ( ifname == NULL )
   {
      /* check that new max setting is greater than current used */
      if ( limit > 0 )
      {
         int used = br_util_get_fdb_limit(brname, BR_UTIL_LIMIT_TYPE_USED);
         if ( used > limit )
         {
            return -1;
         }
      }
      snprintf(sfspath, SYSFS_PATH_MAX, "/sys/class/net/%s/bridge/max_fdb_entries", brname);
   }
   else
   {
      if (type == BR_UTIL_LIMIT_TYPE_MIN)
      {
         if ( limit > 0 )
         {
            int brmax = br_util_get_fdb_limit(brname, BR_UTIL_LIMIT_TYPE_MAX);
            if ( brmax > 0 )
            {
               /* calc new reserved/used setting and verify it does not exceed
                  bridge max */
               int newused;
               int curused = br_util_get_fdb_limit(ifname, BR_UTIL_LIMIT_TYPE_USED);
               int curmin = br_util_get_fdb_limit(ifname, BR_UTIL_LIMIT_TYPE_MIN);
               int brused = br_util_get_fdb_limit(brname, BR_UTIL_LIMIT_TYPE_USED);

               newused = ((curused > curmin) ? curused : curmin);
               newused = brused - newused;
               newused += ((curused > limit) ? curused : limit);
               if ( newused > brmax )
               {
                  return -1;
               }
            }
         }
         snprintf(sfspath, SYSFS_PATH_MAX, "/sys/class/net/%s/brport/min_fdb_entries", ifname);
      }
      else
      {
         /* check that new max setting is greater than current used */
         if ( limit > 0 )
         {
            int used = br_util_get_fdb_limit(ifname, BR_UTIL_LIMIT_TYPE_USED);
            if ( used > limit )
            {
               return -1;
            }
         }
         snprintf(sfspath, SYSFS_PATH_MAX, "/sys/class/net/%s/brport/max_fdb_entries", ifname);
      }
   }

   fp = fopen(sfspath, "w");
   if ( fp ) 
   {
      char buf[16];
      int  rc;

      snprintf(buf, 16, "%u", limit);
      rc = fputs(buf, fp);
      fclose(fp);
      if (rc > 0)
      {
         return 0;
      }
   }

   return -1;
}

