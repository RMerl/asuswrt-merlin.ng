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
 * IEEE1905 Bridge Utility
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <linux/if_arp.h>
#if defined(SUPPORT_BRIDGE_DEDICATED_PORT) && defined(WIRELESS) && \
	defined(SUPPORT_IEEE1905_FM)
#include <bcm_local_kernel_include/linux/if_bridge.h>
#else
#include <linux/if_bridge.h>
#endif // endif
#include <linux/netlink.h>
#include <linux/neighbour.h>
#include <linux/rtnetlink.h>
#include "i5api.h"
#include "ieee1905_brutil.h"
#include "ieee1905_socket.h"
#include "ieee1905_datamodel_priv.h"
#include "ieee1905_interface.h"
#include "ieee1905_trace.h"
#if defined(BRCM_CMS_BUILD)
#include "ieee1905_cmsutil.h"
#endif // endif

#define I5_TRACE_MODULE    i5TraceBrUtil

#define I5_BR_UTIL_MACS_PER_READ    32
#define I5_BR_UTIL_POLL_INTERVAL_MS 2000

#ifndef NTF_MASTER
#define NTF_MASTER 0x04
#endif // endif

static timer_elem_type *pBrUpdateTimer = NULL;

static int _i5BrUtilGetIndexFromName(const char *ifname)
{
   char           sfspath[MAX_PATH_NAME];
   int            rt;
   FILE          *fp;
   int            ifindex = 0;

   snprintf(sfspath, MAX_PATH_NAME, "/sys/class/net/%s/ifindex", ifname);
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

static int _i5BrUtilFilterMac(struct __fdb_entry *fdb)
{
#if defined(WIRELESS)
  unsigned char      altWlMac[MAC_ADDR_LEN];
  i5_dm_device_type *pdevice;
#endif // endif
#if defined(SUPPORT_HOMEPLUG)
  unsigned char      plcFwMac[MAC_ADDR_LEN] = {0x00, 0x1F, 0x84, 0x00, 0x00, 0x00};
#endif // endif

  if (fdb->is_local) {
    return 1;
  }

#if defined(WIRELESS)
  pdevice = i5DmGetSelfDevice();
  if (pdevice) {
    i5_dm_interface_type *pinterface = pdevice->interface_list.ll.next;
    while (pinterface != NULL) {
      if ( i5DmIsInterfaceWireless(pinterface->MediaType) ) {
        memcpy(&altWlMac[0], pinterface->InterfaceId, MAC_ADDR_LEN);
        altWlMac[0] = altWlMac[0] ^ 2;
        if ( 0 == memcmp(&altWlMac[0], &fdb->mac_addr[0], MAC_ADDR_LEN) ) {
          return 1;
        }
      }
      pinterface = pinterface->ll.next;
    }
  }
#endif // endif

#if defined(SUPPORT_HOMEPLUG)
  if ( 0 == memcmp(&plcFwMac[0], &fdb->mac_addr[0], MAC_ADDR_LEN) ) {
    return 1;
  }
#endif // endif

  return 0;
}

int i5BrUtilDevIsBridge(const char *ifname)
{
  char path[MAX_PATH_NAME];
  struct stat st;
  int rc;

  snprintf(path, MAX_PATH_NAME, "/sys/class/net/%s/bridge", ifname);
  rc = stat(path, &st);
  if ( (0 == rc) && S_ISDIR(st.st_mode)) {
    return 1;
  }
  return 0;
}

int i5BrUtilGetPortFromName(const char *ifname)
{
   char  sfspath[MAX_PATH_NAME];
   int   port_num;
   FILE *f;

   snprintf(sfspath, MAX_PATH_NAME, "/sys/class/net/%s/brport/port_no", ifname);
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

int i5BrUtilGetNameFromPort(const char *brname, int portNum, char *ifname)
{
  DIR *dp;
  struct dirent *ep;
  char path[MAX_PATH_NAME];
  int retval = -1;

  snprintf(path, MAX_PATH_NAME, "/sys/class/net/%s/brif", brname);
  dp = opendir (path);
  if (dp != NULL)
  {
    while ((ep = readdir(dp)) != NULL) {
      int port_no = -1;
      FILE *f;

      snprintf(path, MAX_PATH_NAME, "/sys/class/net/%s/brif/%s/port_no", brname, ep->d_name);
      f = fopen(path, "r");
      if ( f ) {
        int rc = fscanf(f, "%x", &port_no);
        fclose(f);
        if ((rc == 1) && ( port_no == portNum )) {
          if ( strlen(ep->d_name) < I5_MAX_IFNAME ) {
            strncpy(ifname, ep->d_name, I5_MAX_IFNAME-1);
            ifname[I5_MAX_IFNAME-1] = '\0';
            retval = 0;
          }
          break;
        }
      }
    }
    closedir (dp);
  }
  else {
    printf("Warning can't read interface list\n");
  }

  return retval;

}

void i5BrUtilProcessMacs( const char *brname, void *arg)
{
  FILE *f;
  int i, n, j;
  struct __fdb_entry fe[I5_BR_UTIL_MACS_PER_READ];
  char path[MAX_PATH_NAME];

  snprintf(path, MAX_PATH_NAME, "/sys/class/net/%s/brforward", brname);
  f = fopen(path, "r");
  if (f) {
    while( 1 ) {
      n = fread(fe, sizeof(struct __fdb_entry), I5_BR_UTIL_MACS_PER_READ, f);
      for( i = 0; i < n; i++ ) {
        if ( 0 == _i5BrUtilFilterMac(&fe[i]) ) {
          i5_dm_device_type              *pDmDevice;
          i5_dm_1905_neighbor_type       *pDmNeighbor;
          i5_dm_interface_type           *pDmInterface;
          i5_dm_bridging_tuple_info_type *pDmBrTuple;
          int found = 0;

          pDmDevice = (i5_dm_device_type *)i5_dm_network_topology.device_list.ll.next;
          while ((pDmDevice != NULL) && (0 == found)) {
            /* exclude 1905 interfaces and 1905 neighbors */
            pDmInterface = (i5_dm_interface_type *)pDmDevice->interface_list.ll.next;
            while (pDmInterface != NULL) {
              if ( 0 == memcmp(pDmInterface->InterfaceId, &fe[i].mac_addr[0], MAC_ADDR_LEN) ) {
                found = 1;
                break;
              }
              pDmNeighbor = i5Dm1905NeighborFind(pDmDevice, pDmInterface->InterfaceId, &fe[i].mac_addr[0]);
              if ( NULL != pDmNeighbor ) {
                found = 1;
                break;
              }
              pDmInterface = pDmInterface->ll.next;
            }
            if ( 1 == found ) {
              break;
            }
            /* exclude devices listed in the bridging tuples */
            pDmBrTuple = (i5_dm_bridging_tuple_info_type *)pDmDevice->bridging_tuple_list.ll.next;
            while ((pDmBrTuple != NULL) && (0 == found)) {
              for ( j = 0; j < pDmBrTuple->forwardingInterfaceListNumEntries; j++ ) {
                if ( 0 == memcmp(&pDmBrTuple->ForwardingInterfaceList[j*MAC_ADDR_LEN], &fe[i].mac_addr[0], MAC_ADDR_LEN) ) {
                  found = 1;
                  break;
                }
              }
              pDmBrTuple = pDmBrTuple->ll.next;
            }
            pDmDevice = pDmDevice->ll.next;
          }
          if ( 0 == found ) {
            char ifname[I5_MAX_IFNAME];
            int rc = i5BrUtilGetNameFromPort(brname, fe[i].port_no, ifname);
            if ( 0 == rc ) {
              unsigned char macAddr[MAC_ADDR_LEN];
              rc = i5InterfaceInfoGet(ifname, &macAddr[0]);
              if ( rc > 0 ) {
                rc = i5DmLegacyNeighborUpdate(&i5_config.i5_mac_address[0], &macAddr[0], &fe[i].mac_addr[0]);
              }
            }
          }
        }
      }
      if ( n < I5_BR_UTIL_MACS_PER_READ ) {
        break;
      }
    }
    fclose(f);
  }
  else {
    i5TraceError("unable to read forwarding table for %s\n", brname);
  }
}
/* Returns 1 for forwarding, 0 if in another state, -1 if it doesn't exist
 */
int i5BrUtilGetPortStpState(const char *ifname)
{
  FILE *stateFile;
  char stpState[2];
  char path[MAX_PATH_NAME];
  struct stat st;
  int rc = 0;

  i5Trace("%s \n", ifname);

  snprintf(path, MAX_PATH_NAME, "/sys/class/net/%s/brport/bridge", ifname);
  rc = stat(path, &st);
  if ( (0 != rc) || !S_ISDIR(st.st_mode)) {
    return -1;
  }

  snprintf(path, MAX_PATH_NAME, "/sys/class/net/%s/brport/state", ifname);
  stateFile = fopen (path, "r");
  if (stateFile != NULL)
  {
    int n = fread(stpState, sizeof(stpState), 1, stateFile);
    if ( (n > 0) && (stpState[0] == '3') ) {
      rc = 1;
    }
    fclose(stateFile);
    return rc;
  }
  return -1;
}

int i5BrUtilGetPortList(const char *brname, unsigned char maxCnt, char *pPortList)
{
  DIR *dp;
  struct dirent *ep;
  char brif_path[MAX_PATH_NAME];
  int cnt = 0;
  char *pos = pPortList;

  i5Trace("\n");

  snprintf(brif_path, MAX_PATH_NAME, "/sys/class/net/%s/brif", brname);
  dp = opendir (brif_path);
  if (dp != NULL)
  {
    while ((ep = readdir(dp)) != NULL) {
      if ( ep->d_name[0] == '.' ) {
         continue;
      }
      if (cnt < maxCnt) {
         if ( strlen(ep->d_name) > (I5_MAX_IFNAME-1) ) {
           i5TraceError("Port name too long (%s)\n", ep->d_name);
           continue;
         }
         else {
          strncpy(pos, ep->d_name, I5_MAX_IFNAME);
         }
      }
      else {
        break;
      }
      cnt++;
      pos += I5_MAX_IFNAME;
    }
    closedir (dp);
  }
  else {
    i5TraceError("Cannot read port list for %s\n", brname);
  }
  return cnt;
}

void i5BrUtilUpdate( void *arg )
{
  i5_dm_device_type *pdevice;
  i5_dm_bridging_tuple_info_type *pbrtuple;

  i5Trace("\n");

  if (pBrUpdateTimer) {
    i5TimerFree(pBrUpdateTimer);
    pBrUpdateTimer = NULL;
  }

  pdevice = i5DmGetSelfDevice();
  if ( pdevice != NULL ) {
    i5DmLegacyNeighborPending(&i5_config.i5_mac_address[0]);
    pbrtuple = (i5_dm_bridging_tuple_info_type *)pdevice->bridging_tuple_list.ll.next;
    while (pbrtuple != NULL) {
      i5BrUtilProcessMacs(pbrtuple->ifname, NULL);
      pbrtuple = pbrtuple->ll.next;
    }
    i5DmLegacyNeighborDone(&i5_config.i5_mac_address[0]);
  }

  pBrUpdateTimer = i5TimerNew(I5_BR_UTIL_POLL_INTERVAL_MS, i5BrUtilUpdate, NULL);
}

char * i5BrUtilGetBridgeName(const char * ifname, char * bridgeIfname) {

    // is there a better way to do this???

    FILE *fp;
    unsigned int brifindex;
    int rt;
    const int maxCmdLen = I5_MAX_IFNAME + 42;
    char cmd[maxCmdLen];

    snprintf(cmd, maxCmdLen, "cat /sys/class/net/%s/brport/bridge/ifindex", ifname);

    fp = popen(cmd, "r");
    if (fp == NULL)
        return NULL;
    rt = fscanf(fp, "%u", &brifindex);
    pclose(fp);

    if (rt == 0)
        return NULL;

    return if_indextoname(brifindex,bridgeIfname);
}

#if defined(SUPPORT_BRIDGE_DEDICATED_PORT) && defined(WIRELESS) && \
	defined(SUPPORT_IEEE1905_FM)
int i5BrUtilMarkDedicatedStpPort(const char *brname, const char *ifname, int setting)
{
   char  sfspath[MAX_PATH_NAME];
   FILE *fp;

   if ((ifname == NULL) || (brname == NULL)) {
      return -1;
   }

   snprintf(sfspath, MAX_PATH_NAME, "/sys/class/net/%s/brport/stp_dedicated", ifname);
   fp = fopen(sfspath, "w");
   if ( fp ) {
      char buf[16];
      int  rc;

      snprintf(buf, 16, "%u", setting);
      rc = fputs(buf, fp);
      fclose(fp);
      if (rc > 0) {
         return 0;
      }
   }

   return -1;
}
#endif // endif

int i5BrUtilReadFdb(const char *ifname, struct __fdb_entry *fdbs, int offset, int num)
{
   FILE *f;
   int   cnt = 0;
   char  sfspath[MAX_PATH_NAME];

   snprintf(sfspath, MAX_PATH_NAME, "/sys/class/net/%s/brforward", ifname);
   f = fopen(sfspath, "r");
   if (f)
   {
      fseek(f, offset*sizeof(struct __fdb_entry), SEEK_SET);
      cnt = fread(fdbs, sizeof(struct __fdb_entry), num, f);
      fclose(f);
   }

   return cnt;
}

int i5BrUtilAddStaticFdbEntry(const char *ifname, const char *macaddr)
{
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
   ndm->ndm_ifindex = _i5BrUtilGetIndexFromName(ifname);
   if ( 0 == ndm->ndm_ifindex )
   {
      return -1;
   }
   ndm->ndm_family = PF_BRIDGE;
   ndm->ndm_flags  = NTF_MASTER;
   ndm->ndm_state = NUD_NOARP;

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

   return 0;
}

int i5BrUtilDelFdbEntry(const char *ifname, const unsigned char *macaddr)
{
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
   ndm->ndm_ifindex = _i5BrUtilGetIndexFromName(ifname);
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

   return 0;
}

int i5BrUtilFlushFdbs(const char *brname, const char *ifname)
{
   int                port_no = -1;
   struct __fdb_entry fdb[I5_BR_UTIL_MACS_PER_READ];
   int                offset = 0;
   int                cnt;

   if ( ifname != NULL )
   {
      port_no = i5BrUtilGetPortFromName(ifname);
      if ( port_no < 0 )
      {
         return port_no;
      }
   }

   do
   {
      /* loop through and remove fdb entries */
      cnt = i5BrUtilReadFdb(brname, fdb, offset, I5_BR_UTIL_MACS_PER_READ);
      offset += I5_BR_UTIL_MACS_PER_READ;
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
               /* only remove learned entries */
               if (0 != fdb[i].ageing_timer_value)
               {
                  i5BrUtilDelFdbEntry(ifname, fdb[i].mac_addr);
                  offset -= 1;
               }
            }
         }
      }
   } while (cnt > 0);

   return 0;
}

void i5BrUtilInit( void )
{
  pBrUpdateTimer = i5TimerNew(I5_BR_UTIL_POLL_INTERVAL_MS, i5BrUtilUpdate, NULL);
}

void i5BrUtilDeinit( void )
{
  if ( pBrUpdateTimer != NULL ) {
    i5TimerFree(pBrUpdateTimer);
    pBrUpdateTimer = NULL;
  }
}
