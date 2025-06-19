/***********************************************************************
 *
 *  Copyright (c) 2019  Broadcom Ltd
 *  All Rights Reserved
 *
<:label-BRCM:2019:DUAL/GPL:standard

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
 *
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/if_addr.h>

#include "sysutil_net.h"
#include "sysutil.h"


int sysUtl_openNetlinkSocket(int protocol __attribute__((unused)),
                             unsigned int pid __attribute__((unused)),
                             unsigned int groups __attribute__((unused)))
{
#ifdef DESKTOP_LINUX
   return 999;
#else
   int fd;
   struct sockaddr_nl addr;

   /* Open a socket for generic Linux link status monitoring */
    fd = socket(AF_NETLINK, SOCK_RAW, protocol);
    if (fd < 0)
    {
       fprintf(stderr, "openNetlinkSocket: Could not open netlink socket!\n");
       return -1;
    }
    

    memset(&addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_pid = pid;
    addr.nl_groups = groups;
    if (0 > bind(fd, (struct sockaddr *) &addr, sizeof(addr)))
    {
       fprintf(stderr, "openNetlinkSocket: Could not bind netlink socket!\n");
       close(fd);
       return -1;
    }
    return fd;
#endif  /* DESKTOP_LINUX */
}


int sysUtl_getIfindexByIfname(const char *ifname)
{
   int sockfd;
   int ifindex = -1;
   struct ifreq ifr;

   memset(&ifr, 0, sizeof(ifr));

   if (ifname == NULL)
   {
      return -1;
   }

   /* open socket to get INET info */
   if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
   {
      fprintf(stderr, "getIfindexByIfname: could not create socket!\n");
      return -1;
   }

   strcpy(ifr.ifr_name, ifname);
   if (ioctl(sockfd, SIOCGIFINDEX, &ifr) >= 0)
   {
      ifindex = ifr.ifr_ifindex;
   }

   close(sockfd);

   return ifindex;
}


int sysUtl_getIfnameByIndex(int index, char *intfName)
{
   struct ifreq ifr;
   int s = 0;

   memset(&ifr, 0, sizeof(ifr));
   ifr.ifr_ifindex = index;

   if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
   {
      fprintf(stderr, "getIfnameByIndex: could not create socket!\n");
      return -1;
   }

   if (ioctl(s, SIOCGIFNAME, &ifr) < 0)
   {
      close(s);
      return -1;
   }
   close(s);

   strcpy(intfName, ifr.ifr_name);

   return 0;
}


int sysUtl_isInterfaceLinkUp(const char *ifname)
{
   int  skfd;
   struct ifreq intf;
   int isUp = 0;

   if(ifname == NULL) {
      return 0;
   }

   if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
      return 0;
   }

   strcpy(intf.ifr_name, ifname);

   if (ioctl(skfd, SIOCGIFFLAGS, &intf) == -1) {
      isUp = 0;
   } else {
      isUp = (intf.ifr_flags & IFF_RUNNING) ? 1 : 0;
   }

   close(skfd);

#ifdef alt_method
   /* there is also some old code which uses SIOCGLINKSTATE.  Don't know
    * which way is better...  Should look in kernel: maybe the info comes
    * from the same place.
    */
   if (ioctl(socketFd, SIOCGLINKSTATE, &intf) != -1)
   {
      status = *(int*)(intf.ifr_data);
      if (status) { /* link is up */ }
   }
#endif /* alt_method */

   return isUp;
}

int sysUtl_isInterfaceConfigedUp(const char *ifname)
{
   int  skfd;
   struct ifreq intf;
   int isUp = 0;

   if(ifname == NULL) {
      return 0;
   }

   if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
      return 0;
   }

   strcpy(intf.ifr_name, ifname);

   if (ioctl(skfd, SIOCGIFFLAGS, &intf) == -1) {
      isUp = 0;
   } else {
      isUp = (intf.ifr_flags & IFF_UP) ? 1 : 0;
   }

   close(skfd);

   return isUp;
}


int sysUtl_isInterfaceUnderBridge(const char *brName, const char *intfName)
{
   char path[1024]={0};
   DIR *dir;
   struct dirent *d;
   int found=0;

   if (brName == NULL || intfName == NULL)
      return 0;

   snprintf(path, sizeof(path), "/sys/devices/virtual/net/%s/brif", brName);
   dir = opendir(path);
   if (dir == NULL)
      return 0;

   while((d = readdir(dir)) != NULL)
   {
      if (!strcmp(d->d_name, intfName))
      {
         found = 1;
         break;
      }
   }
   closedir(dir);

   return found;
}

