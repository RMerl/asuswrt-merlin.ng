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
#include <sys/types.h>
#include <net/if.h>
#include <linux/if_addr.h>
#include <linux/rtnetlink.h>
#include <ifaddrs.h>

#include "sysutil_net.h"
#include "sysutil.h"

#define NLMSG_BUF_SIZE 4096
struct route_req {
     struct nlmsghdr header;
     struct ifinfomsg msg;
};
struct link_req {
     int parent_ifindex;
     int vlanid;
};

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

   strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name)-1);
   ifr.ifr_name[sizeof(ifr.ifr_name)-1] = '\0';
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

   strncpy(intf.ifr_name, ifname, sizeof(intf.ifr_name)-1);
   intf.ifr_name[sizeof(intf.ifr_name)-1] = '\0';

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

   strncpy(intf.ifr_name, ifname, sizeof(intf.ifr_name)-1);
   intf.ifr_name[sizeof(intf.ifr_name)-1] = '\0';

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

int sysUtl_interfaceExists(const char *ifname)
{
    struct ifaddrs *ifaddr, *ifa;
    int ret = 0;

    if (getifaddrs(&ifaddr) == -1)
    {
        perror("getifaddrs");
        return 0;
    }

    /* Walk through linked list, maintaining head pointer so we
        can free list later */
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL ||
            ifa->ifa_addr->sa_family != AF_PACKET)
            continue;
        if (!strcmp(ifa->ifa_name, ifname))
        {
            ret = 1;
            break;
        }
    }

    freeifaddrs(ifaddr);
    return ret;
}

static char* sysUtl_getNestedRta(struct rtattr *rta_in, int len_in, int rta_type, int *rta_len)
{
    struct rtattr *rta = rta_in;
    int len = len_in;
    for(; RTA_OK(rta, len); rta = RTA_NEXT(rta, len)) {
        if(rta->rta_type == rta_type) {
            *rta_len = rta->rta_len;
            return (char *)RTA_DATA(rta);
        }
    }
    return NULL;
}

static int sysUtl_parseVlan(struct rtattr *rta_in)
{
    char* linkinfo = NULL;
    char* datainfo = NULL;
    char* vlaninfo = NULL;
    int datainfo_len = 0;
    int vlaninfo_len = 0;
    int vlan = INVALID_VLANID;
    struct rtattr *rta = rta_in;

    linkinfo = (char *)((char *) rta + NLA_HDRLEN);
    datainfo = sysUtl_getNestedRta((struct rtattr *)linkinfo, rta->rta_len, IFLA_INFO_DATA, &datainfo_len);
    if (datainfo) {
        vlaninfo = sysUtl_getNestedRta((struct rtattr *)datainfo, datainfo_len, IFLA_VLAN_ID, &vlaninfo_len);
        if (vlaninfo) {
            vlan = *(unsigned short *)vlaninfo;
        }
    }
    return vlan;
}

static int sysUtl_getLink(const char *ifname, struct link_req *link_req) 
{
    char buf[NLMSG_BUF_SIZE] = {0};
    size_t seq_num = 0;
    struct sockaddr_nl sa = {0};
    struct iovec iov = {0};
    struct msghdr msg = {0};
    struct nlmsghdr *nh;
    struct route_req req;

    int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if(fd < 0) {
        perror("socket():");
        goto exit_ret;
    }

    memset(&req, 0, sizeof(req));
    req.header.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));
    req.header.nlmsg_flags = NLM_F_REQUEST;
    req.header.nlmsg_type = RTM_GETLINK;
    req.header.nlmsg_seq = ++seq_num;
    req.msg.ifi_family = AF_UNSPEC;
    req.msg.ifi_index = if_nametoindex(ifname);
    req.msg.ifi_change = 0xFFFFFFFF;

    sa.nl_family = AF_NETLINK;
    iov.iov_base = &req;
    iov.iov_len = req.header.nlmsg_len;
    msg.msg_name = &sa;
    msg.msg_namelen = sizeof(sa);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    if(sendmsg(fd, &msg, 0) < 0) {
        perror("sendmsg():");
        goto exit;
    }

    iov.iov_base = buf;
    iov.iov_len = NLMSG_BUF_SIZE;

    for(;;) {
        // wait for a message(block)
        int msg_len = recvmsg(fd, &msg, 0);
        if (msg_len < 0) {
            perror("recvmsg():");
            goto exit;
        }

        for(nh = (struct nlmsghdr *)buf;
            NLMSG_OK(nh, msg_len); nh = NLMSG_NEXT(nh, msg_len)) {
            int len;
            struct ifinfomsg *infomsg;
            struct rtattr *rta;

            if(nh->nlmsg_type == NLMSG_DONE)
                goto exit;

            if(nh->nlmsg_type != RTM_BASE)
                continue;

            // message payload
            infomsg = (struct ifinfomsg *)NLMSG_DATA(nh); 
            // message attribute
            rta = IFLA_RTA(infomsg);
            // attribute length
            len = nh->nlmsg_len - NLMSG_LENGTH(sizeof(*infomsg));
            // parse the message attributes
            for(; RTA_OK(rta, len); rta = RTA_NEXT(rta, len)) {
                if(rta->rta_type == IFLA_LINK) {
                    // get the parent's ifindex
                    link_req->parent_ifindex = *(unsigned short *)((char *) rta + NLA_HDRLEN);
                }
                if(rta->rta_type == IFLA_LINKINFO) {
                    // get the vlan
                    link_req->vlanid = sysUtl_parseVlan(rta);
                }
            }
        }

    // do not wait for the next message
    break;
    }

exit:
    close(fd);
exit_ret:
    return 0;
}

int sysUtl_getLowerDeviceIfindex(const char *ifname) 
{
    struct link_req link_req;

    memset(&link_req, 0, sizeof(struct link_req));
    link_req.parent_ifindex = INVALID_IFINDEX;
    sysUtl_getLink(ifname, &link_req);

    return link_req.parent_ifindex;
}

int sysUtl_getVlanId(const char *ifname)
{
    struct link_req link_req;

    memset(&link_req, 0, sizeof(struct link_req));
    link_req.vlanid = INVALID_VLANID;
    sysUtl_getLink(ifname, &link_req);

    return link_req.vlanid;
}


