/* -*- mode: c; c-basic-offset: 2 -*- */
/* 
 * Copyright (C) 2007-2012 David Bird (Coova Technologies) <support@coova.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#include "system.h"
#include <asm/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <err.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>

#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include "chilli.h"
#include "rtmon.h"

#define MYPROTO NETLINK_ROUTE
#define MYMGRP RTMGRP_IPV4_ROUTE

#define inaddr(x)    (((struct sockaddr_in *)&ifr->x)->sin_addr)
#define inaddr2(p,x) (((struct sockaddr_in *)&(p)->x)->sin_addr)

struct netlink_req {
  struct nlmsghdr nlmsg_info;
  struct rtmsg rtmsg_info;
  char buffer[2048];
};

static int open_netlink() {
  int sock = socket(AF_NETLINK, SOCK_RAW, MYPROTO);
  struct sockaddr_nl addr;
  
  memset((void *)&addr, 0, sizeof(addr));
  
  if (sock<0)
    return sock;

  addr.nl_family = AF_NETLINK;
  addr.nl_pid = getpid();
  addr.nl_groups = MYMGRP;

  if (bind(sock,(struct sockaddr *)&addr,sizeof(addr)) < 0)
    return -1;

  return sock;
}

int rtmon_init(struct rtmon_t *rtmon, rtmon_callback func) {
  memset(rtmon, 0, sizeof(struct rtmon_t));

  rtmon->fd = open_netlink();
  rtmon->cb = func;

  if (rtmon->fd > 0) {

    rtmon_discover_ifaces(rtmon);
    rtmon_discover_routes(rtmon);
    
    if (_options.debug) {
      rtmon_print_ifaces(rtmon, 1);
      rtmon_print_routes(rtmon, 1);
    }
    
    rtmon_check_updates(rtmon);
    return 0;
  }
  return -1;
}

static int netlink_route_request(int fd) {
  struct sockaddr_nl local;
  struct sockaddr_nl peer;   
  struct msghdr msg_info;
  struct netlink_req req;
  struct iovec iov_info;
  int rtn;

  bzero(&local, sizeof(local));
  local.nl_family = AF_NETLINK;
  local.nl_pad = 0;
  local.nl_pid = getpid() + 1;
  local.nl_groups = 0;

  if (bind(fd, (struct sockaddr*) &local, sizeof(local)) < 0) {
    perror("bind");
    return -1;
  }
  
  bzero(&peer, sizeof(peer));
  peer.nl_family = AF_NETLINK;
  peer.nl_pad = 0;
  peer.nl_pid = 0;
  peer.nl_groups = 0;
  
  bzero(&msg_info, sizeof(msg_info));
  msg_info.msg_name = (void *) &peer;
  msg_info.msg_namelen = sizeof(peer);
  
  bzero(&req, sizeof(req));
  
  req.nlmsg_info.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
  req.nlmsg_info.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
  req.nlmsg_info.nlmsg_type = RTM_GETROUTE;
  
  req.rtmsg_info.rtm_family = AF_INET;
  req.rtmsg_info.rtm_table = RT_TABLE_MAIN;
  
  iov_info.iov_base = (void *) &req.nlmsg_info;
  iov_info.iov_len = req.nlmsg_info.nlmsg_len;
  msg_info.msg_iov = &iov_info;
  msg_info.msg_iovlen = 1;
  
  rtn = sendmsg(fd, &msg_info, 0);
  if (rtn < 0) {
    perror("sendmsg");
    return -1;
  }
  return 0;
}

static int netlink_route_results(int fd, char *b, size_t blen) {
  int len = 0;

  bzero(b, blen);

  while (blen > 0) {
    int rtn = recv(fd, b, blen, 0);

    if (rtn < 0) {
      perror("recv");
      break;
    }

    struct nlmsghdr *hdr = (struct nlmsghdr *) b;
    
    if (hdr->nlmsg_type == NLMSG_DONE)	{
      break;
    }

    b += rtn;
    blen -= rtn;
    len += rtn;
  }

  return len;
}

static int rtmon_add_route(struct rtmon_t *rtmon, struct rtmon_route *rt) {
  int sz = rtmon->_route_sz;
  struct rtmon_route *dst = 0;
  int i;

  rt->has_data = 1;

  for (i=0; i < sz; i++) {
    if (!memcmp(&rtmon->_routes[i], rt, sizeof(struct rtmon_route))) {
      log_dbg("Already have this route for %s", inet_ntoa(rt->destination));
      return 0;
    }
    if (!dst && !rtmon->_routes[i].has_data) {
      dst = &rtmon->_routes[i];
    }
  }

  if (!dst) {
    if (sz) {
      rtmon->_routes = 
	(struct rtmon_route *)realloc(rtmon->_routes, 
				      (sz + 1) * sizeof(struct rtmon_route));
      dst = &rtmon->_routes[sz];
    } else {
      dst = rtmon->_routes = 
	(struct rtmon_route *)malloc(sizeof(struct rtmon_route));
    }

    rtmon->_route_sz++;
  }

  if (!dst) return -1;
  memcpy(dst, rt, sizeof(struct rtmon_route));
  return 0;
}

/**
* Extract each route table entry and print
*/
static void netlink_parse_routes(struct rtmon_t *rtmon, char *b, int blen) {
  struct nlmsghdr *hdr = (struct nlmsghdr *) b;
  struct rtattr * attr;
  struct rtmon_route rt;
  int payload;

  for(; NLMSG_OK(hdr, blen); hdr = NLMSG_NEXT(hdr, blen)) {
    struct rtmsg * rtm = (struct rtmsg *) NLMSG_DATA(hdr);

    if (rtm->rtm_table != RT_TABLE_MAIN)
      continue;

    attr = (struct rtattr *) RTM_RTA(rtm);
    payload = RTM_PAYLOAD(hdr);

    memset(&rt, 0, sizeof(rt));

    for (;RTA_OK(attr, payload); attr = RTA_NEXT(attr, payload)) {
      switch(attr->rta_type) {
      case RTA_DST:
	rt.destination = *(struct in_addr *)RTA_DATA(attr);
	break;
      case RTA_GATEWAY:
	rt.gateway = *(struct in_addr *)RTA_DATA(attr);
	break;
      case RTA_OIF:
	rt.if_index = *((int *) RTA_DATA(attr));
      default:
	break; 
      }
    }

    {
      uint32_t mask = 0;
      int i;

      for (i=0; i<rtm->rtm_dst_len; i++) {
	mask |= (1 << (32-i-1));
      }

      rt.netmask.s_addr = htonl(mask);
    }

    rtmon_add_route(rtmon, &rt);
  }
}

struct msgnames_t {
  int id;
  char *msg;
} typenames[] = {
#define MSG(x) { x, #x }
  MSG(RTM_NEWROUTE),
  MSG(RTM_DELROUTE),
  MSG(RTM_GETROUTE),
#undef MSG
  {0,0}
};

static char *lookup_name(struct msgnames_t *db, int id) {
  static char name[512];
  struct msgnames_t *msgnamesiter;
  for (msgnamesiter=db; msgnamesiter->msg; ++msgnamesiter) {
    if (msgnamesiter->id == id)
      break;
  }
  if (msgnamesiter->msg) {
    return msgnamesiter->msg;
  }
  safe_snprintf(name,sizeof(name),"#%i\n",id);
  return name;
}

static char * idx2name(struct rtmon_t *rtmon, int idx) {
  int i;

  for (i=0; i<rtmon->_iface_sz; i++) 
    if (rtmon->_ifaces[i].has_data) 
      if (rtmon->_ifaces[i].index == idx)
	return rtmon->_ifaces[i].devname;

  return "n/a";
}

struct rtmon_iface * rtmon_find(struct rtmon_t *rtmon, char *name) {
  int i;

  for (i=0; i<rtmon->_iface_sz; i++) 
    if (rtmon->_ifaces[i].has_data) 
      if (!strcmp(rtmon->_ifaces[i].devname, name))
	return &rtmon->_ifaces[i];

  return 0;
}

void rtmon_print_ifaces(struct rtmon_t *rtmon, int fd) {
  char line[512];
  int i;

  safe_snprintf(line,512,"\nSystem Interfaces\n");
  safe_write(fd, line, strlen(line));
  
  for (i=0; i < rtmon->_iface_sz; i++) {

    if (rtmon->_ifaces[i].has_data) {
      unsigned char *u = rtmon->_ifaces[i].hwaddr;

      safe_snprintf(line,512,"%d) %s (%d)", 
		    i, rtmon->_ifaces[i].devname, rtmon->_ifaces[i].index);
      safe_write(fd, line, strlen(line));

      if (rtmon->_ifaces[i].address.s_addr) {
	safe_snprintf(line,512," ip=%s", inet_ntoa(rtmon->_ifaces[i].address));
	safe_write(fd, line, strlen(line));
      }

      safe_snprintf(line,512," net=%s", inet_ntoa(rtmon->_ifaces[i].network));
      safe_write(fd, line, strlen(line));

      safe_snprintf(line,512," mask=%s", inet_ntoa(rtmon->_ifaces[i].netmask));
      safe_write(fd, line, strlen(line));

      if (rtmon->_ifaces[i].broadcast.s_addr) {
	safe_snprintf(line,512," bcase=%s", inet_ntoa(rtmon->_ifaces[i].broadcast));
	safe_write(fd, line, strlen(line));
      }

      if (rtmon->_ifaces[i].gateway.s_addr) {
	safe_snprintf(line,512," peer=%s", inet_ntoa(rtmon->_ifaces[i].gateway));
	safe_write(fd, line, strlen(line));
      }

      safe_snprintf(line,512," mac=%2.2X-%2.2X-%2.2X-%2.2X-%2.2X-%2.2x",
		    u[0], u[1], u[2], u[3], u[4], u[5]);
      safe_write(fd, line, strlen(line));

      safe_snprintf(line,512," mtu=%u\n",  rtmon->_ifaces[i].mtu);
      safe_write(fd, line, strlen(line));
    }
  }
}

void rtmon_print_routes(struct rtmon_t *rtmon, int fd) {
  char line[512];
  int i;

  safe_snprintf(line,512,"\nSystem Routes\n");
  safe_write(fd, line, strlen(line));

  for (i=0; i < rtmon->_route_sz; i++) {
    if (rtmon->_routes[i].has_data) {
      safe_snprintf(line,512,"%d) dst=%s", i, inet_ntoa(rtmon->_routes[i].destination));
      safe_write(fd, line, strlen(line));
      safe_snprintf(line,512," mask=%s", inet_ntoa(rtmon->_routes[i].netmask));
      safe_write(fd, line, strlen(line));
      if (rtmon->_routes[i].gateway.s_addr) {
	safe_snprintf(line,512," gw=%s", inet_ntoa(rtmon->_routes[i].gateway));
	safe_write(fd, line, strlen(line));
      }
      safe_snprintf(line,512," dev=%s (%d)\n", idx2name(rtmon, rtmon->_routes[i].if_index), rtmon->_routes[i].if_index);
      safe_write(fd, line, strlen(line));
    }
  }
}

static const char *mactoa(uint8_t *m) {
  static char buff[256];
  safe_snprintf(buff, sizeof(buff), 
		"%02x:%02x:%02x:%02x:%02x:%02x",
		m[0], m[1], m[2], m[3], m[4], m[5]);
  return (buff);
}

void rtmon_check_updates(struct rtmon_t *rtmon) {
  int i, j;
  for (i=0; i < rtmon->_route_sz; i++) {
    if (rtmon->_routes[i].has_data) {
      if (rtmon->_routes[i].destination.s_addr == 0) {

	log_dbg("Default Route %s", inet_ntoa(rtmon->_routes[i].gateway));

	for (j=0; j < rtmon->_iface_sz; j++) {
	  if (rtmon->_ifaces[j].has_data) {
	    if (rtmon->_routes[i].if_index == rtmon->_ifaces[j].index) {
	      struct arpreq areq;
	      struct sockaddr_in *sin;
	      int s, attempt=0, retries=3;
	      
	      log_dbg("Route Interface %s", rtmon->_ifaces[j].devname);

	      if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("socket");
		return;
	      }

	      memset(&areq, 0, sizeof(areq));
	      sin = (struct sockaddr_in *) &areq.arp_pa;

	      sin->sin_family = AF_INET;
	      sin->sin_addr.s_addr = rtmon->_routes[i].gateway.s_addr;

	      safe_strncpy(areq.arp_dev, rtmon->_ifaces[j].devname, sizeof(areq.arp_dev));

	      while (attempt < retries) {
		struct sockaddr_in addr;
		char b[1]={0};
		
		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_addr = sin->sin_addr;
		addr.sin_port = htons(10000);
		
		if (sendto(s, b, sizeof(b), 0,
			   (struct sockaddr *) &addr, 
			   sizeof(addr)) < 0)
		  perror("sendto");

		if (ioctl(s, SIOCGARP, (caddr_t) &areq) == -1) {

		  if (errno == ENXIO) {
		    log_dbg("%s -- no entry\n", inet_ntoa(sin->sin_addr));
		    attempt++;
		    sleep(1);
		    continue;
		  }
		  else { perror("SIOCGARP"); break; }

		} else {

		  log_dbg("MAC %s", mactoa((uint8_t *)&areq.arp_ha.sa_data));
		  memcpy(rtmon->_routes[i].gwaddr, &areq.arp_ha.sa_data, sizeof(rtmon->_routes[i].gwaddr));

		  if (rtmon->cb(rtmon, &rtmon->_ifaces[j], &rtmon->_routes[i]))
		    log_err(errno, "callback failed");

		  break;
		}
	      }

	      close(s);
	      return;
	    }
	  }
	}
      }
    }
  }
}

static int rtmon_add_iface(struct rtmon_t *rtmon, struct rtmon_iface *ri) {
  int sz = rtmon->_iface_sz;
  struct rtmon_iface *dst = 0;
  int i;

  ri->has_data = 1 | RTMON_REMOVE;

  for (i=0; i < sz; i++) {
    log_dbg("i=%d sz=%d",i,sz);
    if (!memcmp(&rtmon->_ifaces[i], ri, sizeof(struct rtmon_iface))) {
      rtmon->_ifaces[i].has_data = 1;
      log_dbg("Already have this iface %s", ri->devname);
      return 0;
    }
    if (!dst && !rtmon->_ifaces[i].has_data) {
      dst = &rtmon->_ifaces[i];
    }
  }

  if (!dst) {

    if (sz) {
      rtmon->_ifaces = 
	(struct rtmon_iface *)realloc(rtmon->_ifaces, 
				      (sz + 1) * sizeof(struct rtmon_iface));
      dst = &rtmon->_ifaces[sz];
    } else {
      dst = rtmon->_ifaces = 
	(struct rtmon_iface *)malloc(sizeof(struct rtmon_iface));
    }

    rtmon->_iface_sz++;
  }

  if (!dst) return -1;

  ri->has_data = 1;
  memcpy(dst, ri, sizeof(struct rtmon_iface));
  return 0;
}

void rtmon_discover_ifaces(struct rtmon_t *rtmon) {
  struct rtmon_iface ri;
  struct ifconf ic;
  int fd, len, i;

  for (i=0; i < rtmon->_iface_sz; i++)
    if (rtmon->_ifaces[i].has_data)
      rtmon->_ifaces[i].has_data |= RTMON_REMOVE;
  
  if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    return;
  }
  
  ic.ifc_buf=0;
  ic.ifc_len=0;
    
  if (ioctl(fd, SIOCGIFCONF, &ic) < 0) {
    close(fd);
    return;
  }
  
  ic.ifc_buf = calloc((size_t)ic.ifc_len, 1);
  if (ioctl(fd, SIOCGIFCONF, &ic) < 0) {
    close(fd);
    free(ic.ifc_buf);
    return;
  }
  
  len = (ic.ifc_len/sizeof(struct ifreq));
  
  for (i=0; i < len; ++i) {
    struct ifreq *ifr = (struct ifreq *)&ic.ifc_req[i];

    memset(&ri, 0, sizeof(ri));
    
    /* device name and address */
    safe_strncpy(ri.devname, ifr->ifr_name, sizeof(ri.devname));
    ri.address = inaddr(ifr_addr);
    
    /* index */
    if (-1 < ioctl(fd, SIOCGIFINDEX, (caddr_t) ifr)) {
      ri.index = ifr->ifr_ifindex;
    }
    
    /* netmask */
    if (-1 < ioctl(fd, SIOCGIFNETMASK, (caddr_t)ifr)) {
      ri.netmask = inaddr(ifr_addr);
    } 
    
    ri.network.s_addr = ri.address.s_addr & ri.netmask.s_addr;
    
    /* hardware address */
#ifdef SIOCGIFHWADDR
    if (-1 < ioctl(fd, SIOCGIFHWADDR, (caddr_t)ifr)) {
      switch (ifr->ifr_hwaddr.sa_family) {
      case  ARPHRD_PPP:
	break;
      case  ARPHRD_NETROM:  
      case  ARPHRD_ETHER:  
      case  ARPHRD_EETHER:  
      case  ARPHRD_IEEE802: 
	{
	  unsigned char *u = (unsigned char *)&ifr->ifr_addr.sa_data;
	  memcpy(ri.hwaddr, u, 6);
	}
	break;
      }
    } 
#else
#ifdef SIOCGENADDR
    if (-1 < ioctl(fd, SIOCGENADDR, (caddr_t)ifr)) {
      unsigned char *u = (unsigned char *)&ifr->ifr_enaddr;
      memcpy(ri.hwaddr, u, 6);
    } 
#else
#warning Do not know how to find interface hardware address
#endif /* SIOCGENADDR */
#endif /* SIOCGIFHWADDR */
    
    
    /* flags */
    if (-1 < ioctl(fd, SIOCGIFFLAGS, (caddr_t)ifr)) {
      ri.devflags = ifr->ifr_flags;
    } 
    
    /* point-to-point gateway */
    if (ri.devflags & IFF_POINTOPOINT) {
      if (-1 < ioctl(fd, SIOCGIFDSTADDR, (caddr_t)ifr)) {
	ri.gateway = inaddr(ifr_addr);
      } 
    }
    
    /* broadcast address */
    if (ri.devflags & IFF_BROADCAST) {
      if (-1 < ioctl(fd, SIOCGIFBRDADDR, (caddr_t)ifr)) {
	ri.broadcast = inaddr(ifr_addr);
      } 
    }
    
    if (-1 < ioctl(fd, SIOCGIFMTU, (caddr_t)ifr)) {
      ri.mtu = ifr->ifr_mtu;
    } 

    rtmon_add_iface(rtmon, &ri);
  }

  for (i=0; i < rtmon->_iface_sz; i++) {
    if (rtmon->_ifaces[i].has_data & RTMON_REMOVE) {
      tun_delif(tun, rtmon->_ifaces[i].index);
      memset(&rtmon->_ifaces[i], 0, sizeof(struct rtmon_iface)); 
    }
  }
  
  free(ic.ifc_buf);
  close(fd);
}

void rtmon_discover_routes(struct rtmon_t *rtmon) {
  int fd;
  char b[8192];
  int blen;
  int i;
  
  fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  
  if (fd < 0) {
    perror("Error in sock open");
    return;
  }
  
  netlink_route_request(fd);
  blen = netlink_route_results(fd, b, sizeof(b));
  netlink_parse_routes(rtmon, b, blen);

  for (i=0; i < rtmon->_iface_sz; i++)
    if (rtmon->_ifaces[i].has_data & RTMON_REMOVE)
      memset(&rtmon->_ifaces[i], 0, sizeof(struct rtmon_iface));
  
  close(fd);
}

int rtmon_read_event(struct rtmon_t *rtmon) {
  struct sockaddr_nl nladdr;
  struct msghdr msg;
  struct iovec iov[2];
  struct nlmsghdr nlh;
  char buffer[65536];
  int ret;

  iov[0].iov_base = (void *)&nlh;
  iov[0].iov_len = sizeof(nlh);
  iov[1].iov_base = (void *)buffer;
  iov[1].iov_len = sizeof(buffer);

  msg.msg_name = (void *)&(nladdr);
  msg.msg_namelen = sizeof(nladdr);
  msg.msg_iov = iov;
  msg.msg_iovlen = sizeof(iov)/sizeof(iov[0]);

  ret = recvmsg(rtmon->fd, &msg, 0);
  
  if (ret < 0) {
    return ret;
  }

  log_dbg("Type: %i (%s)",(nlh.nlmsg_type),lookup_name(typenames,nlh.nlmsg_type));

#define FLAG(x) if (nlh.nlmsg_flags & x) printf(" %s",#x)
  FLAG(NLM_F_REQUEST);
  FLAG(NLM_F_MULTI);
  FLAG(NLM_F_ACK);
  FLAG(NLM_F_ECHO);
  FLAG(NLM_F_REPLACE);
  FLAG(NLM_F_EXCL);
  FLAG(NLM_F_CREATE);
  FLAG(NLM_F_APPEND);
#undef FLAG

  log_dbg("Seq : %i Pid : %i",nlh.nlmsg_seq,nlh.nlmsg_pid);

  rtmon_discover_ifaces(rtmon);
  rtmon_discover_routes(rtmon);

  if (_options.debug) {
    rtmon_print_ifaces(rtmon, 1);
    rtmon_print_routes(rtmon, 1);
  }

  rtmon_check_updates(rtmon);
  
  return 0;
}

