/* -*- mode: c; c-basic-offset: 2 -*- */
/* 
 * Copyright (C) 2007-2012 David Bird (Coova Technologies) <support@coova.com>
 * Copyright (C) 2003, 2004, 2005 Mondru AB.
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

/*
 *  A tunnel is the back-haul link which chilli sends traffic. Typically,
 *  this is a single tun/tap interface allowing chilli to simply pass on
 *  packets to the kernel for processing (iptables) and routing. Without the
 *  tun/tap interface, chilli must decide for itself how to route traffic,
 *  maintaining a socket into each back-haul interface. One or more tunnels
 *  are required.
 *
 */

#include "chilli.h"
#include "debug.h"
#ifdef ENABLE_MULTIROUTE
#include "rtmon.h"
extern struct rtmon_t _rtmon;
#endif

#define inaddr(x)    (((struct sockaddr_in *)&ifr->x)->sin_addr)
#define inaddr2(p,x) (((struct sockaddr_in *)&(p)->x)->sin_addr)

#ifdef ENABLE_MULTIROUTE
net_interface * tun_nextif(struct tun_t *tun) {
  net_interface *netif;
  int i;

  if (tun->_interface_count == TUN_MAX_INTERFACES)
    return 0;

  for (i=0; i<TUN_MAX_INTERFACES; i++) {
    netif = &tun->_interfaces[i];
    if (!netif->ifindex && !netif->fd) {
      if (!netif->idx)
	netif->idx = tun->_interface_count;
      tun->_interface_count++;
      return netif;
    }
  }

  return 0;
}

net_interface * tun_newif(struct tun_t *tun, net_interface *netif) {
  net_interface *newif = tun_nextif(tun);
      
  if (newif) {
    int idx = newif->idx;
    memcpy(newif, netif, sizeof(net_interface));
    newif->idx = idx;

    if (newif->devflags & IFF_POINTOPOINT)
      newif->flags |= NET_PPPHDR | NET_ETHHDR;
  }

  return newif;
}

void tun_delif(struct tun_t *tun, int ifindex) {
  net_interface *netif;
  int i;

  for (i=0; i<TUN_MAX_INTERFACES; i++) {
    netif = &tun->_interfaces[i];
    if (netif->ifindex == ifindex) {
      int idx = netif->idx;
      net_select_dereg(tun->sctx, netif->fd);
      net_close(netif);
      memset(netif, 0, sizeof(net_interface));
      netif->idx = idx;
      tun->_interface_count--;
      return;
    }
  }
}

int tun_name2idx(struct tun_t *tun, char *name) {
  int i;

  for (i=0; i<TUN_MAX_INTERFACES; i++) 
    if (!strcmp(name, tun->_interfaces[i].devname))
      return tun->_interfaces[i].idx;

  /* Not found? Check for discovery */
  {
    struct rtmon_iface *rti = rtmon_find(&_rtmon, name);
    if (rti) {
      net_interface *newif = 0;
      net_interface netif;
      log_dbg("Discoving TUN %s", name);
      memset(&netif, 0, sizeof(netif));
      safe_strncpy(netif.devname, rti->devname, sizeof(netif.devname));
      memcpy(netif.hwaddr, rti->hwaddr, sizeof(netif.hwaddr));
      netif.address = rti->address;
      netif.netmask = rti->netmask;
      netif.gateway = rti->gateway;
      netif.broadcast = rti->broadcast;
      netif.devflags = rti->devflags;
      netif.mtu = rti->mtu;
      netif.ifindex = rti->index;
      
      newif = tun_newif(tun, &netif);
      if (newif) {

	if (net_init(newif, 0, ETH_P_ALL, 1, 0) < 0) {
	  log_err(errno, "net_init");
	}
	else {
	  net_select_reg(tun->sctx, 
			 newif->fd,
			 SELECT_READ, (select_callback) tun_decaps, 
			 tun, newif->idx);
	}
	
	return newif->idx;
      }
    }
  }

  return 0; /* tun/tap index */
}

int tun_discover(struct tun_t *this) {
  net_interface netif;
  struct ifconf ic;
  int fd, len, i;

  if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    log_err(errno, "socket() failed");
    return -1;
  }

  ic.ifc_buf=0;
  ic.ifc_len=0;

  if (ioctl(fd, SIOCGIFCONF, &ic) < 0) {
    log_err(errno, "ioctl(SIOCGIFCONF)");
    close(fd);
    return -1;
  }

  if (!(ic.ifc_buf = calloc((size_t)ic.ifc_len, 1))) {
    log_err(errno, "calloc(ic.ifc_buf)");
    close(fd);
    return -1;
  }

  if (ioctl(fd, SIOCGIFCONF, &ic) < 0) {
    log_err(errno, "ioctl(SIOCGIFCONF)");
    free(ic.ifc_buf);
    close(fd);
    return -1;
  }
    
  len = (ic.ifc_len / sizeof(struct ifreq));

  for (i=0; i<len; ++i) {
    struct ifreq *ifr = (struct ifreq *)&ic.ifc_req[i];

    memset(&netif, 0, sizeof(netif));

    /* device name and address */
    safe_strncpy(netif.devname, ifr->ifr_name, sizeof(netif.devname));
    netif.address = inaddr(ifr_addr);

    log_dbg("Interface: %s", ifr->ifr_name);

    if (!strcmp(ifr->ifr_name, _options.dhcpif)) {
      log_dbg("skipping dhcpif %s", _options.dhcpif);
      continue;
    }

    if (!strncmp(ifr->ifr_name, "tun", 3) || !strncmp(ifr->ifr_name, "tap", 3)) {
      log_dbg("skipping tun/tap %s", _options.dhcpif);
      continue;
    }

    log_dbg("\tIP Address:\t%s", inet_ntoa(inaddr(ifr_addr)));


    /* netmask */
    if (-1 < ioctl(fd, SIOCGIFNETMASK, (caddr_t)ifr)) {

      netif.netmask = inaddr(ifr_addr);
      log_dbg("\tNetmask:\t%s", inet_ntoa(inaddr(ifr_addr)));

    } else log_err(errno, "ioctl(SIOCGIFNETMASK)");

    /* hardware address */
#ifdef SIOCGIFHWADDR
    if (-1 < ioctl(fd, SIOCGIFHWADDR, (caddr_t)ifr)) {
      switch (ifr->ifr_hwaddr.sa_family) {
      case  ARPHRD_PPP:
	netif.flags |= NET_PPPHDR | NET_ETHHDR;
	break;
      case  ARPHRD_NETROM:  
      case  ARPHRD_ETHER:  
      case  ARPHRD_EETHER:  
      case  ARPHRD_IEEE802: 
	{
	  unsigned char *u = (unsigned char *)&ifr->ifr_addr.sa_data;

	  memcpy(netif.hwaddr, u, 6);

	  log_dbg("\tHW Address:\t%2.2X-%2.2X-%2.2X-%2.2X-%2.2X-%2.2x",
		  u[0], u[1], u[2], u[3], u[4], u[5]);
	}
	break;
      }
    } else log_err(errno, "ioctl(SIOCGIFHWADDR)");
#else
#ifdef SIOCGENADDR
    if (-1 < ioctl(fd, SIOCGENADDR, (caddr_t)ifr)) {
      unsigned char *u = (unsigned char *)&ifr->ifr_enaddr;

      memcpy(netif.hwaddr, u, 6);

      log_dbg("\tHW Address:\t%2.2X-%2.2X-%2.2X-%2.2X-%2.2X-%2.2x",
		  u[0], u[1], u[2], u[3], u[4], u[5]);
    } else log_err(errno, "ioctl(SIOCGENADDR)");
#else
#warning Do not know how to find interface hardware address
#endif /* SIOCGENADDR */
#endif /* SIOCGIFHWADDR */

    /* flags */
    if (-1 < ioctl(fd, SIOCGIFFLAGS, (caddr_t)ifr)) {

      netif.devflags = ifr->ifr_flags;

    } else log_err(errno, "ioctl(SIOCGIFFLAGS)");

    /* point-to-point gateway */
    if (netif.devflags & IFF_POINTOPOINT) {
      if (-1 < ioctl(fd, SIOCGIFDSTADDR, (caddr_t)ifr)) {

	netif.flags |= NET_PPPHDR;
	netif.gateway = inaddr(ifr_addr);
	log_dbg("\tPoint-to-Point:\t%s", inet_ntoa(inaddr(ifr_dstaddr)));

      } else log_err(errno, "ioctl(SIOCGIFDSTADDR)");
    }

    /* broadcast address */
    if (netif.devflags & IFF_BROADCAST) {
      if (-1 < ioctl(fd, SIOCGIFBRDADDR, (caddr_t)ifr)) {
	
	netif.broadcast = inaddr(ifr_addr);
	log_dbg("\tBroadcast:\t%s", inet_ntoa(inaddr(ifr_addr)));
	
      } else log_err(errno, "ioctl(SIOCGIFBRDADDR)");
    }

    /* mtu */
    if (-1 < ioctl(fd, SIOCGIFMTU, (caddr_t)ifr)) {
      
      netif.mtu = ifr->ifr_mtu;
      log_dbg("\tMTU:      \t%u",  ifr->ifr_mtu);
      
    } else log_err(errno, "ioctl(SIOCGIFMTU)");
    
    /* if (0 == ioctl(fd, SIOCGIFMETRIC, ifr)) */

    if (netif.address.s_addr == htonl(INADDR_LOOPBACK) ||
        netif.address.s_addr == INADDR_ANY ||
        netif.address.s_addr == INADDR_NONE)
      continue;

    {
      net_interface *newif = tun_newif(tun, &netif);
      
      if (newif) {

	if (net_init(newif, 0, ETH_P_ALL, 1, 0) < 0) {
	  log_err(errno, "net_init");
	}
	
	if (!strcmp(_options.routeif, netif.devname))
	  tun->routeidx = newif->idx;

      } else {
	log_dbg("no room for interface %s", netif.devname);
      }
    }
  }

  free(ic.ifc_buf);
  close(fd);
  return 0;
}
#endif


#if defined(__linux__)

int tun_nlattr(struct nlmsghdr *n, int nsize, int type, void *d, size_t dlen) {
  size_t len = RTA_LENGTH(dlen);
  size_t alen = NLMSG_ALIGN(n->nlmsg_len);
  struct rtattr *rta = (struct rtattr*) (((void*)n) + alen);
  if (alen + len > nsize)
    return -1;
  rta->rta_len = len;
  rta->rta_type = type;
  memcpy(RTA_DATA(rta), d, dlen);
  n->nlmsg_len = alen + len;
  return 0;
}

int tun_gifindex(struct tun_t *this, uint32_t *index) {
  struct ifreq ifr;
  int fd;

  memset (&ifr, '\0', sizeof (ifr));
  ifr.ifr_addr.sa_family = AF_INET;
  ifr.ifr_dstaddr.sa_family = AF_INET;
  ifr.ifr_netmask.sa_family = AF_INET;
  safe_strncpy(ifr.ifr_name, tuntap(this).devname, IFNAMSIZ);
  if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    log_err(errno, "socket() failed");
  }
  if (ioctl(fd, SIOCGIFINDEX, &ifr)) {
    log_err(errno,"ioctl() failed");
    close(fd);
    return -1;
  }
  close(fd);
  *index = ifr.ifr_ifindex;
  return 0;
}
#endif

int tun_addaddr(struct tun_t *this, struct in_addr *addr,
		struct in_addr *dstaddr, struct in_addr *netmask) {

#if defined(__linux__)
  struct {
    struct nlmsghdr 	n;
    struct ifaddrmsg 	i;
    char buf[TUN_NLBUFSIZE];
  } req;
  
  struct sockaddr_nl local;
  size_t addr_len;
  int fd;
  
  struct sockaddr_nl nladdr;
  struct iovec iov;
  struct msghdr msg;

  uint32_t idx;

  if (!this->addrs) /* Use ioctl for first addr to make ping work */
    return tun_setaddr(this, addr, dstaddr, netmask);

  memset(&req, 0, sizeof(req));
  req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifaddrmsg));
  req.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE;
  req.n.nlmsg_type = RTM_NEWADDR;
  req.i.ifa_family = AF_INET;
  req.i.ifa_prefixlen = 32; /* 32 FOR IPv4 */
  req.i.ifa_flags = 0;
  req.i.ifa_scope = RT_SCOPE_HOST; /* TODO or 0 */

  if (tun_gifindex(this, &idx)) {
    log_err(errno,"tun_gifindex() failed");
    return -1;
  }

  req.i.ifa_index = idx;

  tun_nlattr(&req.n, sizeof(req), IFA_ADDRESS, addr, sizeof(addr));
  tun_nlattr(&req.n, sizeof(req), IFA_LOCAL, dstaddr, sizeof(dstaddr));

  if ((fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) < 0) {
    log_err(errno,"socket() failed");
    return -1;
  }

  memset(&local, 0, sizeof(local));
  local.nl_family = AF_NETLINK;
  local.nl_groups = 0;
  
  if (bind(fd, (struct sockaddr*)&local, sizeof(local)) < 0) {
    log_err(errno, "bind() failed");
    close(fd);
    return -1;
  }

  addr_len = sizeof(local);
  if (getsockname(fd, (struct sockaddr*)&local, (socklen_t *) &addr_len) < 0) {
    log_err(errno, "getsockname() failed");
    close(fd);
    return -1;
  }

  if (addr_len != sizeof(local)) {
    log_err(0, "Wrong address length %d", addr_len);
    close(fd);
    return -1;
  }

  if (local.nl_family != AF_NETLINK) {
    log_err(0, "Wrong address family %d", local.nl_family);
    close(fd);
    return -1;
  }
  
  iov.iov_base = (void*)&req.n;
  iov.iov_len = req.n.nlmsg_len;

  msg.msg_name = (void*)&nladdr;
  msg.msg_namelen = sizeof(nladdr),
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;
  msg.msg_control = NULL;
  msg.msg_controllen = 0;
  msg.msg_flags = 0;

  memset(&nladdr, 0, sizeof(nladdr));
  nladdr.nl_family = AF_NETLINK;
  nladdr.nl_pid = 0;
  nladdr.nl_groups = 0;

  req.n.nlmsg_seq = 0;
  req.n.nlmsg_flags |= NLM_F_ACK;

  if (sendmsg(fd, &msg, 0) < 0)
    log_err(errno, "sendmsg()");

  dev_set_flags(tuntap(this).devname, IFF_UP | IFF_RUNNING); 

  close(fd);
  this->addrs++;

  return 0;

#elif defined (__FreeBSD__) || defined (__APPLE__) || defined (__OpenBSD__) || defined (__NetBSD__)

  int fd;
  struct ifaliasreq      areq;

  /* TODO: Is this needed on FreeBSD? */
  if (!this->addrs) /* Use ioctl for first addr to make ping work */
    return tun_setaddr(this, addr, dstaddr, netmask); /* TODO dstaddr */

  memset(&areq, 0, sizeof(areq));

  /* Set up interface name */
  safe_strncpy(areq.ifra_name, tuntap(this).devname, IFNAMSIZ);

  ((struct sockaddr_in*) &areq.ifra_addr)->sin_family = AF_INET;
  ((struct sockaddr_in*) &areq.ifra_addr)->sin_len = sizeof(areq.ifra_addr);
  ((struct sockaddr_in*) &areq.ifra_addr)->sin_addr.s_addr = addr->s_addr;

  ((struct sockaddr_in*) &areq.ifra_mask)->sin_family = AF_INET;
  ((struct sockaddr_in*) &areq.ifra_mask)->sin_len    = sizeof(areq.ifra_mask);
  ((struct sockaddr_in*) &areq.ifra_mask)->sin_addr.s_addr = netmask->s_addr;

  /* For some reason FreeBSD uses ifra_broadcast for specifying dstaddr */
  ((struct sockaddr_in*) &areq.ifra_broadaddr)->sin_family = AF_INET;
  ((struct sockaddr_in*) &areq.ifra_broadaddr)->sin_len = sizeof(areq.ifra_broadaddr);
  ((struct sockaddr_in*) &areq.ifra_broadaddr)->sin_addr.s_addr = dstaddr->s_addr;

  /* Create a channel to the NET kernel. */
  if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    log_err(errno,
	    "socket() failed");
    return -1;
  }
  
  if (ioctl(fd, SIOCAIFADDR, (void *) &areq) < 0) {
    log_err(errno,
	    "ioctl(SIOCAIFADDR) failed");
    close(fd);
    return -1;
  }

  close(fd);
  this->addrs++;
  return 0;

#elif defined (__sun__)
  
  if (!this->addrs) /* Use ioctl for first addr to make ping work */
    return tun_setaddr(this, addr, dstaddr, netmask);
  
  log_err(errno, "Setting multiple addresses not possible on Solaris");
  return -1;

#else
#error  "Unknown platform!"
#endif
}

int tun_setaddr(struct tun_t *this, struct in_addr *addr, struct in_addr *dstaddr, struct in_addr *netmask) {
  net_set_address(&tuntap(this), addr, dstaddr, netmask);
  
#if defined(__FreeBSD__) || defined (__APPLE__) || defined (__OpenBSD__) || defined (__NetBSD__)
  net_add_route(dstaddr, addr, netmask);
  this->routes = 1;
#endif

  return 0;
}

int tuntap_interface(struct _net_interface *netif) {
#if defined(__linux__)
  struct ifreq ifr;

#elif defined(__FreeBSD__) || defined (__APPLE__) || defined (__OpenBSD__) || defined (__NetBSD__)
  char devname[IFNAMSIZ+5]; /* "/dev/" + ifname */
  int devnum;
  struct ifaliasreq areq;
  int fd;

#elif defined(__sun__)
  int if_fd, ppa = -1;
  static int ip_fd = 0;
  int muxid;
  struct ifreq ifr;

#else
#error  "Unknown platform!"
#endif

  memset(netif, 0, sizeof(*netif));

  /*  memcpy(netif->gwaddr, _options.nexthop, PKT_ETH_ALEN);*/

#if defined(__linux__)
  /* Open the actual tun device */
  if ((netif->fd = open("/dev/net/tun", O_RDWR)) < 0) {
    log_err(errno, "open() failed");
    return -1;
  }

  ndelay_on(netif->fd);
  coe(netif->fd);
  
  /* Set device flags. For some weird reason this is also the method
     used to obtain the network interface name */

  memset(&ifr, 0, sizeof(ifr));

  /* Tun device, no packet info */
  ifr.ifr_flags = (
#ifdef ENABLE_TAP
		   _options.usetap ? IFF_TAP :
#endif
		   IFF_TUN) | IFF_NO_PI; 
  
  ifr.ifr_flags = ifr.ifr_flags
#ifdef IFF_MULTICAST
     | IFF_MULTICAST
#endif
#ifdef IFF_BROADCAST
     | IFF_BROADCAST
#endif
#ifdef IFF_PROMISC
     | IFF_PROMISC
#endif
#ifdef IFF_ONE_QUEUE
     | IFF_ONE_QUEUE
#endif
    ;

  if (_options.tundev && *_options.tundev && 
      strcmp(_options.tundev, "tap") && strcmp(_options.tundev, "tun"))
    safe_strncpy(ifr.ifr_name, _options.tundev, IFNAMSIZ);

  if (ioctl(netif->fd, TUNSETIFF, (void *) &ifr) < 0) {
    log_err(errno, "ioctl() failed");
    close(netif->fd);
    return -1;
  } 
  
#if defined(IFF_ONE_QUEUE) && defined(SIOCSIFTXQLEN)
  {
    struct ifreq nifr;
    int nfd;
    memset(&nifr, 0, sizeof(nifr));
    if ((nfd = socket (AF_INET, SOCK_DGRAM, 0)) >= 0) {
      safe_strncpy(nifr.ifr_name, ifr.ifr_name, IFNAMSIZ);
      nifr.ifr_qlen = _options.txqlen;
      
      if (ioctl(nfd, SIOCSIFTXQLEN, (void *) &nifr) >= 0) 
	log_info("TX queue length set to %d", _options.txqlen);
      else 
	log_err(errno, "Cannot set tx queue length on %s", ifr.ifr_name);

      close (nfd);
    } else {
      log_err(errno, "Cannot open socket on %s", ifr.ifr_name);
    }
  }
#endif
  
  safe_strncpy(netif->devname, ifr.ifr_name, IFNAMSIZ);
  
  ioctl(netif->fd, TUNSETNOCSUM, 1); /* Disable checksums */

  /* Get the MAC address of our tap interface */
#ifdef ENABLE_TAP
  if (_options.usetap) {
    int fd;
    netif->flags |= NET_ETHHDR;
    if ((fd = socket (AF_INET, SOCK_DGRAM, 0)) >= 0) {
      memset(&ifr, 0, sizeof(ifr));
      safe_strncpy(ifr.ifr_name, netif->devname, IFNAMSIZ);
      if (ioctl(fd, SIOCGIFHWADDR, &ifr) < 0) {
	log_err(errno, "ioctl(d=%d, request=%d) failed", fd, SIOCGIFHWADDR);
      }
      memcpy(netif->hwaddr, ifr.ifr_hwaddr.sa_data, PKT_ETH_ALEN);
      log_dbg("tap-mac: %s %.2X-%.2X-%.2X-%.2X-%.2X-%.2X", ifr.ifr_name,
	      netif->hwaddr[0],netif->hwaddr[1],netif->hwaddr[2],
	      netif->hwaddr[3],netif->hwaddr[4],netif->hwaddr[5]);
      close(fd);
    }
  }
#endif
  
  return 0;
  
#elif defined(__FreeBSD__) || defined (__APPLE__) || defined (__OpenBSD__) || defined (__NetBSD__)

  /* Find suitable device */
  for (devnum = 0; devnum < 255; devnum++) { 
    safe_snprintf(devname, sizeof(devname), "/dev/tun%d", devnum);
    if ((netif->fd = open(devname, O_RDWR)) >= 0) break;
    if (errno != EBUSY) break;
  } 

  if (netif->fd < 0) {
    log_err(errno, "Can't find tunnel device");
    return -1;
  }

  safe_snprintf(netif->devname, sizeof(netif->devname), "tun%d", devnum);

  /* The tun device we found might have "old" IP addresses allocated */
  /* We need to delete those. This problem is not present on Linux */

  memset(&areq, 0, sizeof(areq));

  /* Set up interface name */
  safe_strncpy(areq.ifra_name, netif->devname, sizeof(areq.ifra_name));

  /* Create a channel to the NET kernel. */
  if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    log_err(errno,"socket() failed");
    return -1;
  }
  
  /* Delete any IP addresses until SIOCDIFADDR fails */
  while (ioctl(fd, SIOCDIFADDR, (void *) &areq) != -1);

  close(fd);
  return 0;

#elif defined(__sun__)

  if ((ip_fd = open("/dev/udp", O_RDWR, 0)) < 0) {
    log_err(errno, "Can't open /dev/udp");
    return -1;
  }
  
  if ((netif->fd = open("/dev/tun", O_RDWR, 0)) < 0) {
    log_err(errno, "Can't open /dev/tun");
    return -1;
  }
  
  /* Assign a new PPA and get its unit number. */
  if ((ppa = ioctl(netif->fd, TUNNEWPPA, -1)) < 0) {
    log_err(errno, "Can't assign new interface");
    return -1;
  }
  
  if ((if_fd = open("/dev/tun", O_RDWR, 0)) < 0) {
    log_err(errno, "Can't open /dev/tun (2)");
    return -1;
  }

  if (ioctl(if_fd, I_PUSH, "ip") < 0){
    log_err(errno, "Can't push IP module");
    return -1;
  }
  
  /* Assign ppa according to the unit number returned by tun device */
  if (ioctl(if_fd, IF_UNITSEL, (char *)&ppa) < 0) {
    log_err(errno, "Can't set PPA %d", ppa);
    return -1;
  }

  /* Link the two streams */
  if ((muxid = ioctl(ip_fd, I_LINK, if_fd)) < 0) {
    log_err(errno, "Can't link TUN device to IP");
    return -1;
  }

  close (if_fd);
  
  safe_snprintf(netif->devname, sizeof(netif->devname),
		"tun%d", ppa);

  memset(&ifr, 0, sizeof(ifr));
  safe_strncpy(ifr.ifr_name, netif->devname, sizeof(ifr.ifr_name));
  ifr.ifr_ip_muxid = muxid;
  
  if (ioctl(ip_fd, SIOCSIFMUXID, &ifr) < 0) {
    ioctl(ip_fd, I_PUNLINK, muxid);
    log_err(errno, "Can't set multiplexor id");
    return -1;
  }
  
  /*  if (fcntl (fd, F_SETFL, O_NONBLOCK) < 0)
      msg (M_ERR, "Set file descriptor to non-blocking failed"); */

  return 0;

#else
#error  "Unknown platform!"
#endif
}

int tun_new(struct tun_t **ptun) {
  struct tun_t *tun;

  if (!(tun = *ptun = calloc(1, sizeof(struct tun_t)))) {
    log_err(errno, "calloc() failed");
    return EOF;
  }

#ifdef ENABLE_MULTIROUTE
  tuntap_interface(tun_nextif(tun));

  if (_options.routeif) {
    tun_discover(tun);
  }
#else
  tuntap_interface(&tun->_tuntap);
#endif

  return 0;
}

int tun_free(struct tun_t *tun) {

  if (tun->routes) {
    /*XXX: todo! net_delete_route(&tuntap(tun)); */
  }

  tun_close(tun);

  /* TODO: For solaris we need to unlink streams */

  free(tun);
  return 0;
}

int tun_set_cb_ind(struct tun_t *this, 
		   int (*cb_ind) (struct tun_t *tun, struct pkt_buffer *pb, int idx)) {
  this->cb_ind = cb_ind;
  return 0;
}

struct tundecap {
  struct tun_t *this;
  int idx;
};

static int tun_decaps_cb(void *ctx, struct pkt_buffer *pb) {
  struct tundecap *c = (struct tundecap *)ctx;
  struct pkt_iphdr_t *iph;
  int ethsize = 0;

  char ethhdr = (tun(c->this, c->idx).flags & NET_ETHHDR) != 0;

  size_t length = pkt_buffer_length(pb);
  uint8_t *packet = pkt_buffer_head(pb);

  if (c->idx) ethhdr = 0;

#if(_debug_ > 1)
  log_dbg("tun_decaps(idx=%d, len=%d)", tun(c->this, c->idx).ifindex, length);
#endif

  if (length < PKT_IP_HLEN)
    return -1;

  if (ethhdr) {
    
    if (length < PKT_IP_HLEN + PKT_ETH_HLEN)
      return -1;
    
    ethsize = PKT_ETH_HLEN;
    iph = pkt_iphdr(packet);
    
  } else {
    
    iph = (struct pkt_iphdr_t *)packet;
    
  }

#if defined(HAVE_NETFILTER_QUEUE) || defined(HAVE_NETFILTER_COOVA)
  if (_options.uamlisten.s_addr != _options.dhcplisten.s_addr) {
    iph->daddr  = iph->daddr & ~(_options.mask.s_addr);
    iph->daddr |= _options.dhcplisten.s_addr & _options.mask.s_addr;
    chksum(iph);
  }
#endif

  if (c->idx > 0) {
#ifdef ENABLE_NETNAT
    struct pkt_udphdr_t *udph = pkt_udphdr(packet);
    if (iph->daddr == tun(c->this, c->idx).address.s_addr &&
	ntohs(udph->dst) > 10000 && ntohs(udph->dst) < 30000) {
      if (nat_undo(c->this, c->idx, packet, length))
	return -1;
    }
#endif

#ifdef ENABLE_MULTIROUTE
    if (_options.routeonetone) {
      iph->daddr = tun(c->this, c->idx).nataddress.s_addr;
      chksum(iph);
    }
#endif

    if ((iph->daddr & _options.mask.s_addr) != _options.net.s_addr) {
#if(_debug_)
      struct in_addr addr;
      addr.s_addr = iph->daddr;
      log_dbg("pkt not for our network %s",inet_ntoa(addr));
#endif
      return -1;
    }
  } 
  
  if (!_options.usetap) {
    if (iph->version_ihl != PKT_IP_VER_HLEN) {
#if(_debug_)
      log_dbg("dropping non-IPv4");
#endif
      return -1;
    }
    
    if ((int)ntohs(iph->tot_len) + ethsize > length) {
      log_dbg("dropping ip packet; ip-len=%d + eth-hdr=%d > read-len=%d",
	      (int)ntohs(iph->tot_len),
	      ethsize, (int)length);
      return -1;
    }
  }

  return c->this->cb_ind(c->this, pb, c->idx);
}

int tun_decaps(struct tun_t *this, int idx) {

#if defined(__linux__) 
  ssize_t length;
  struct tundecap c;
  
  c.this = this;
  c.idx = idx;

  if (idx > 0) 
    length = net_read_dispatch_eth(&tun(this, idx), tun_decaps_cb, &c);
  else
    length = net_read_dispatch(&tun(this, idx), tun_decaps_cb, &c);

  if (length < 0) 
    return -1;
  
  return length;
  
#elif defined (__FreeBSD__) || defined (__APPLE__) || defined (__OpenBSD__) || defined (__NetBSD__)
  struct tundecap c;
  struct pkt_buffer pb;
  uint8_t packet[PKT_MAX_LEN];
  ssize_t length;

  pkt_buffer_init(&pb, packet, sizeof(packet), PKT_BUFFER_IPOFF);
  
  c.this = this;
  c.idx = idx;
  
  if ((length = safe_read(tun(this, idx).fd, 
			  pkt_buffer_head(&pb), 
			  pkt_buffer_size(&pb))) <= 0) {
    log_err(errno, "read() failed");
    return -1;
  }

  pb.length = length;
  
  /*
    log_dbg("tun_decaps(%d) %s",length,tun(tun,idx).devname);
  */

  if (this->cb_ind) {
#if defined (__OpenBSD__)
    /* tun interface adds 4 bytes to front of packet under OpenBSD */
    pb.length -= 4;
    pb.offset += 4;
#else
    return tun_decaps_cb(&c, &pb);
#endif
  }

  return 0;

#elif defined (__sun__)
  struct pkt_buffer pb;
  uint8_t packet[PKT_MAX_LEN+4];
  ssize_t length;
  struct strbuf sbuf;
  int f = 0;

  pkt_buffer_init(&pb, packet, sizeof(packet), 4);
  
  sbuf.maxlen = pkt_buffer_size(&pb);
  sbuf.buf = pkt_buffer_head(&pb);
  if (getmsg(tun(this, idx).fd, NULL, &sbuf, &f) < 0) {
    log_err(errno, "getmsg() failed");
    return -1;
  }

  pb.length = sbuf.len;

  if (this->cb_ind)
    return this->cb_ind(this, &pb);
  return 0;
  
#endif
}

/*
static uint32_t dnatip[1024];
static uint16_t dnatport[1024];
*/

int tun_write(struct tun_t *tun, uint8_t *pack, size_t len, int idx) {
#if defined (__OpenBSD__)

  unsigned char buffer[PKT_MAX_LEN+4];

  /* Can we user writev here to be more efficient??? */
  *((uint32_t *)(&buffer))=htonl(AF_INET);
  memcpy(&buffer[4], pack, len);

  return safe_write(tun(tun, idx).fd, buffer, len+4);

#elif defined(__linux__) || defined (__FreeBSD__) || defined (__APPLE__) || defined (__NetBSD__)

#ifdef ENABLE_MULTIROUTE
  if (idx > 0 && tun(tun, idx).flags & NET_PPPHDR) {
    struct sockaddr_ll addr;
    size_t ethlen = sizeofeth(pack);
    memset(&addr,0,sizeof(addr));
    addr.sll_family = AF_PACKET;
    addr.sll_protocol = pkt_ethhdr(pack)->prot;
    addr.sll_ifindex = tun(tun, idx).ifindex;
    pack += ethlen;
    len  -= ethlen;
    return net_write_eth(&tun(tun, idx), pack, len, &addr);
  }
#endif

  return safe_write(tun(tun, idx).fd, pack, len);

#elif defined (__sun__)

  struct strbuf sbuf;
  sbuf.len = len;      
  sbuf.buf = pack;
  return putmsg(tun(tun, idx).fd, NULL, &sbuf, 0);

#endif
}

int tun_encaps(struct tun_t *tun, uint8_t *pack, size_t len, int idx) {
  int result;

  if (_options.tcpwin)
    pkt_shape_tcpwin(pkt_iphdr(pack), _options.tcpwin);

  if (_options.tcpmss)
    pkt_shape_tcpmss(pack, &len);

#ifdef ENABLE_MULTIROUTE
  if (idx > 0) {
    struct pkt_iphdr_t *iph = pkt_iphdr(pack);
    if ((iph->daddr & _options.mask.s_addr) == _options.net.s_addr ||
	iph->daddr == dhcp->uamlisten.s_addr) {
      log_dbg("Using route idx == 0 (tun/tap)");
      idx = 0;
    }
  }

  if (_options.routeonetone && idx > 0) {
    struct pkt_iphdr_t *iph = pkt_iphdr(pack);
    if (!tun(tun, idx).nataddress.s_addr)
      tun(tun, idx).nataddress.s_addr = iph->saddr;
    iph->saddr = tun(tun, idx).address.s_addr;
    chksum(iph);
  }
#endif

#ifdef ENABLE_NETNAT
  if (idx > 0) {
    if (nat_do(tun, idx, pack, len)) {
      log_err(0, "unable to nat packet!");
    }
  }
#endif

#if defined(HAVE_NETFILTER_QUEUE) || defined(HAVE_NETFILTER_COOVA)
  if (_options.uamlisten.s_addr != _options.dhcplisten.s_addr) {
    struct pkt_iphdr_t *iph = pkt_iphdr(pack);

    iph->saddr  = iph->saddr & ~(_options.mask.s_addr);
    iph->saddr |= _options.uamlisten.s_addr & _options.mask.s_addr;

    chksum(iph);
  }
#endif

  if (tun(tun, idx).flags & NET_ETHHDR) {
    uint8_t *gwaddr = _options.nexthop; /*tun(tun, idx).gwaddr;*/
    struct pkt_ethhdr_t *ethh = (struct pkt_ethhdr_t *)pack;
    /* memcpy(ethh->src, tun(tun, idx).hwaddr, PKT_ETH_ALEN); */

    /*
     * TODO: When using ieee8021q, the vlan tag has to be stripped
     * off for the non-vlan WAN.
     */
    if (gwaddr[0] == 0 && gwaddr[1] == 0 && gwaddr[2] == 0 && 
	gwaddr[3] == 0 && gwaddr[4] == 0 && gwaddr[5] == 0) {
      /*  
       *  If there isn't a 'nexthop' (gwaddr) for the interface,
       *  default to the tap interface's MAC instead, so that the kernel
       *  will route it. 
       */
      if (idx == 0) {
	gwaddr = tun(tun, idx).hwaddr;
      } else {
	gwaddr = tun(tun, idx).gwaddr;
	copy_mac6(ethh->src, tun(tun, idx).hwaddr);
      }
    }

    copy_mac6(ethh->dst, gwaddr);

#if(_debug_)
    log_dbg("writing to tap src=%.2x:%.2x:%.2x:%.2x:%.2x:%.2x "
	    "dst=%.2x:%.2x:%.2x:%.2x:%.2x:%.2x len=%d",
	    ethh->src[0],ethh->src[1],ethh->src[2],
	    ethh->src[3],ethh->src[4],ethh->src[5],
	    ethh->dst[0],ethh->dst[1],ethh->dst[2],
	    ethh->dst[3],ethh->dst[4],ethh->dst[5], len);
#endif
    
  } else {
    size_t ethlen = sizeofeth(pack);
    pack += ethlen;
    len  -= ethlen;
  }

#if(_debug_ > 1)
  log_dbg("tun_encaps(%s) len=%d", tun(tun,idx).devname, len);
#endif

  result = tun_write(tun, pack, len, idx);

  if (result < 0) {
    log_err(errno, "tun_write(%d)", result);
  }

  return result;
}

int tun_runscript(struct tun_t *tun, char* script, int wait) {
  char saddr[TUN_ADDRSIZE];
  char smask[TUN_ADDRSIZE];
  char b[TUN_ADDRSIZE];
  struct in_addr net;
  pid_t pid;
  
  log_dbg("Running %s", script);

  net.s_addr = tuntap(tun).address.s_addr & tuntap(tun).netmask.s_addr;

  if ((pid = safe_fork()) < 0) {
    log_err(errno, "fork() returned -1!");
    return 0;
  }
  
  if (pid > 0) { /* Parent */
    if (wait) {
      int status = 0;
    again:
      if (waitpid(pid, &status, 0) == -1) {
	if (errno == EINTR) goto again;
	log_err(errno, "waiting for %s", script);
      }
    }
    return 0;
  }
  
  set_env("DHCPIF", VAL_STRING, _options.dhcpif ? _options.dhcpif : "", 0);
  set_env("DEV", VAL_STRING, tun(tun, 0).devname, 0);
  set_env("ADDR", VAL_IN_ADDR, &tuntap(tun).address, 0);
  set_env("MASK", VAL_IN_ADDR, &tuntap(tun).netmask, 0);
  set_env("NET", VAL_IN_ADDR, &net, 0);

  set_env("UAMLISTEN", VAL_IN_ADDR, &_options.uamlisten, 0);
  if (_options.dhcplisten.s_addr && 
      _options.dhcplisten.s_addr != _options.uamlisten.s_addr) {
    set_env("DHCPLISTEN", VAL_IN_ADDR, &_options.dhcplisten, 0);
  }

  safe_snprintf(b, sizeof(b), "%d", (int)_options.mtu);
  set_env("MTU", VAL_STRING, b, 0);

  safe_snprintf(b, sizeof(b), "%d", (int)_options.uamport);
  set_env("UAMPORT", VAL_STRING, b, 0);

#ifdef ENABLE_UAMUIPORT
  safe_snprintf(b, sizeof(b), "%d", (int)_options.uamuiport);
  set_env("UAMUIPORT", VAL_STRING, b, 0);
#endif

#ifdef ENABLE_LAYER3
  if (_options.layer3) {
    set_env("LAYER3", VAL_STRING, "1", 0);
  }
#endif

#ifdef ENABLE_IEEE8021Q
  if (_options.ieee8021q)
    set_env("IEEE8021Q", VAL_STRING, "1" , 0);
  if (_options.ieee8021q_only)
    set_env("ONLY8021Q", VAL_STRING, "1" , 0);
#endif

#ifdef HAVE_NETFILTER_COOVA
  if (_options.kname) {
    set_env("KNAME", VAL_STRING, _options.kname, 0);
  }
#endif

  if (execl(
#ifdef ENABLE_CHILLISCRIPT
	    SBINDIR "/chilli_script", SBINDIR "/chilli_script", _options.binconfig, 
#else
	    script,
#endif
	    script, tuntap(tun).devname, saddr, smask, (char *) 0) != 0) {
    
    log_err(errno, "execl(%s) did not return 0!",script);
    exit(0);
  }
  
  exit(0);
}


/* Currently unused 
int tun_addroute2(struct tun_t *this,
		  struct in_addr *dst,
		  struct in_addr *gateway,
		  struct in_addr *mask) {
  
  struct {
    struct nlmsghdr 	n;
    struct rtmsg 	r;
    char buf[TUN_NLBUFSIZE];
  } req;
  
  struct sockaddr_nl local;
  int addr_len;
  int fd;
  int status;
  struct sockaddr_nl nladdr;
  struct iovec iov;
  struct msghdr msg;

  memset(&req, 0, sizeof(req));
  req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
  req.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE;
  req.n.nlmsg_type = RTM_NEWROUTE;
  req.r.rtm_family = AF_INET;
  req.r.rtm_table  = RT_TABLE_MAIN;
  req.r.rtm_protocol = RTPROT_BOOT;
  req.r.rtm_scope  = RT_SCOPE_UNIVERSE;
  req.r.rtm_type  = RTN_UNICAST;
  tun_nlattr(&req.n, sizeof(req), RTA_DST, dst, 4);
  tun_nlattr(&req.n, sizeof(req), RTA_GATEWAY, gateway, 4);
  
  if ((fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) < 0) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno,
	    "socket() failed");
    return -1;
  }

  memset(&local, 0, sizeof(local));
  local.nl_family = AF_NETLINK;
  local.nl_groups = 0;
  
  if (bind(fd, (struct sockaddr*)&local, sizeof(local)) < 0) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno,
	    "bind() failed");
    close(fd);
    return -1;
  }

  addr_len = sizeof(local);
  if (getsockname(fd, (struct sockaddr*)&local, &addr_len) < 0) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno,
	    "getsockname() failed");
    close(fd);
    return -1;
  }

  if (addr_len != sizeof(local)) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "Wrong address length %d", addr_len);
    close(fd);
    return -1;
  }

  if (local.nl_family != AF_NETLINK) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "Wrong address family %d", local.nl_family);
    close(fd);
    return -1;
  }
  
  iov.iov_base = (void*)&req.n;
  iov.iov_len = req.n.nlmsg_len;

  msg.msg_name = (void*)&nladdr;
  msg.msg_namelen = sizeof(nladdr),
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;
  msg.msg_control = NULL;
  msg.msg_controllen = 0;
  msg.msg_flags = 0;

  memset(&nladdr, 0, sizeof(nladdr));
  nladdr.nl_family = AF_NETLINK;
  nladdr.nl_pid = 0;
  nladdr.nl_groups = 0;

  req.n.nlmsg_seq = 0;
  req.n.nlmsg_flags |= NLM_F_ACK;

  status = sendmsg(fd, &msg, 0);  * TODO: Error check *
  close(fd);
  return 0;
}
*/
