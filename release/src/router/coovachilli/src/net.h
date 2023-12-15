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


#ifndef _NET_H
#define _NET_H

#include "system.h"
#include "pkt.h"
#ifdef ENABLE_NETNAT
#include "nat.h"
#endif

#ifdef USING_PCAP
#include <pcap.h>
#endif

#ifdef HAVE_NETFILTER_QUEUE
#include <linux/types.h>
#include <linux/netfilter.h>
#include <libnetfilter_queue/libnetfilter_queue.h>
#endif

#ifdef USING_MMAP
#define HAVE_PACKET_RING
#define HAVE_PACKET_RX_RING
#define HAVE_PACKET_TX_RING
#define HAVE_TPACKET2

#ifndef PACKET_TX_RING
#define PACKET_TX_RING		13
#define PACKET_LOSS		14
#define TP_STATUS_AVAILABLE	0x0
#define TP_STATUS_SEND_REQUEST	0x1
#define TP_STATUS_SENDING	0x2
#define TP_STATUS_WRONG_FORMAT	0x4
#endif 

struct device_stats
{
  uint64_t		read_cnt;
  uint64_t		read_bytes;
  struct timespec	read_time;
  uint64_t		write_cnt;
  uint64_t		write_bytes;
  struct timespec	write_time;
  uint32_t		other_cnt;
  struct timespec	other_time;
  uint64_t		io_slots;
  uint64_t		io_runs;
  uint64_t		queue_length;
  uint32_t		queue_stall;
  uint32_t		queue_over;
  uint32_t		ata_err;
  uint32_t		proto_err;
};

struct netif_stats
{
  uint64_t		rx_cnt;
  uint64_t		rx_bytes;
  uint64_t		rx_runs;
  uint64_t		tx_cnt;
  uint64_t		tx_bytes;
  uint64_t		tx_runs;
  uint32_t		rx_buffers_full;
  uint32_t		tx_buffers_full;
  uint32_t		dropped;
  uint32_t		ignored;
  uint32_t		broadcast;
};

struct device_config
{
  char			*path;
  /* The shelf number is in network byte order */
  unsigned		shelf;
  unsigned		slot;
  int			queue_length;
  int			direct_io;
  int			trace_io;
  int			read_only;
  int			broadcast;
  long			max_delay;
  long			merge_delay;
};

struct netif_config
{
  int			mtu;
  int			ring_size;
  int			send_buf_size;
  int			recv_buf_size;
};

struct ring
{
  /* Total length of the ring buffer */
  unsigned len;
  /* Number of frames (packets) in the ring buffer */
  unsigned cnt;
  /* The index of the next frame to use */
  unsigned idx;
  /* Frame size in the ring buffer */
  unsigned frame_size;
  /* Block size of the ring buffer */
  unsigned block_size;
  /* Pointers to the individual frames */
  void **frames;
};

struct _net_interface;
struct queue_item
{
  struct _net_interface *iface;
  struct timespec	start;
  void			*buf;
  unsigned		bufsize;
  unsigned		length;
  
  unsigned long long	offset;
  
  unsigned		hdrlen;
  union
  {
    struct pkt_ethhdr_t	ethhdr;
    struct pkt_ethhdr8021q_t ethhdr8021q;
  };
};

#ifdef ENABLE_LARGELIMITS
#define DEF_RING_SIZE (6 * 1024)
#else
#define DEF_RING_SIZE (256)
#endif

#endif

#define SELECT_READ 1
#define SELECT_WRITE 2
#define SELECT_RESET 4

typedef int (*select_callback) (void *data, int idx);

typedef struct {
  int fd;
  int idx;
  char evts;
  select_callback cb;
  void *ctx;
} select_fd;

typedef struct {
  int count;
  select_fd desc[MAX_SELECT];
#ifdef USING_POLL
#ifdef HAVE_SYS_EPOLL_H
  int efd;
  struct epoll_event events[MAX_SELECT];
#else
  struct pollfd pfds[MAX_SELECT];
#endif
#else
  int maxfd;
  fd_set rfds, wfds, efds;
  struct timeval idleTime;
#endif
} select_ctx;

typedef struct _net_interface {
  uint8_t idx;

  /* hardware/link */
  uint16_t protocol;
  uint8_t hwtype;
  uint8_t hwaddr[PKT_ETH_ALEN];
  char devname[IFNAMSIZ+1];
  int devflags;
  int ifindex;
  int mtu;

  /* network/address */
  struct in_addr address;
  struct in_addr network;
  struct in_addr netmask;
  struct in_addr broadcast;
  struct in_addr gateway;

#ifdef ENABLE_IPV6
  struct in6_addr address_v6;
#endif

#ifdef ENABLE_MULTIROUTE
#if(1) /* XXX  ONE2ONE */
  struct in_addr nataddress;
#endif
#endif

  /* socket/descriptor */
  int fd;

#ifdef HAVE_NETFILTER_QUEUE
  struct nfq_handle *h;
  struct nfq_q_handle *qh;
#endif

#ifdef USING_PCAP
  pcap_t *pd;
#endif

#ifdef USING_MMAP
  int is_active;

  struct netif_config	cfg;
  struct netif_stats	stats;

  struct ring rx_ring;
  struct ring tx_ring;
  /* The address of the memory-mapped rings (first RX, then TX) */
  void *ring_ptr;
  /* The length of the mapped area */
  unsigned ring_len;
  /* The length of the frame header in the rings */
  int tp_hdrlen;
#endif

#if defined(__linux__)
  struct sockaddr_ll dest;
#endif

  /* routing */
  uint8_t gwaddr[PKT_ETH_ALEN];

#ifdef ENABLE_NETNAT
  nat_t *nat;
#endif

  select_ctx *sctx;

  uint8_t flags;
#define NET_PROMISC (1<<0)
#define NET_USEMAC  (1<<1)
#define NET_ETHHDR  (1<<2)
#define NET_PPPHDR  (1<<3)

} net_interface;

typedef int (*net_handler)(void *ctx, struct pkt_buffer *pb);

#define net_sflags(n,f) dev_set_flags((n)->devname, (f))
#define net_gflags(n) dev_get_flags((n)->devname, &(n)->devflags)

int net_open(net_interface *netif);
int net_open_eth(net_interface *netif);
int net_reopen(net_interface *netif);
int net_init(net_interface *netif, char *ifname, uint16_t protocol, int promisc, uint8_t *mac);
int net_route(struct in_addr *dst, struct in_addr *gateway, struct in_addr *mask, int delete);
int net_set_mtu(net_interface *netif, size_t mtu);
ssize_t net_write(int sock, void *d, size_t dlen);

int safe_sendto(int s, const void *b, size_t blen, int flags,
		const struct sockaddr *dest_addr, socklen_t addrlen);

int net_select_init(select_ctx *sctx);
int net_select_prepare(select_ctx *sctx);
int net_select(select_ctx *sctx);
int net_run_selected(select_ctx *sctx, int status);

int net_select_zero(select_ctx *sctx);
int net_select_addfd(select_ctx *sctx, int fd, int evts);
int net_select_rmfd(select_ctx *sctx, int fd);
int net_select_modfd(select_ctx *sctx, int fd, int evts);
int net_select_fd(select_ctx *sctx, int fd, char evts);
int net_select_read_fd(select_ctx *sctx, int fd);
int net_select_write_fd(select_ctx *sctx, int fd);

ssize_t net_read_eth(net_interface *netif, void *d, size_t slen);

#if defined(__linux__)
ssize_t net_write_eth(net_interface *netif, void *d, size_t dlen, 
		      struct sockaddr_ll *dest);
#endif

ssize_t net_read_dispatch(net_interface *netif, net_handler func, void *ctx);
ssize_t net_read_dispatch_eth(net_interface *netif, net_handler func, void *ctx);

int net_open_nfqueue(net_interface *netif, uint16_t q, int (*cb)());

int net_select_reg(select_ctx *sctx, int fd, char evts, 
		   select_callback cb, void *ctx, int idx);
int net_select_rereg(select_ctx *sctx, int oldfd, int newfd);
int net_select_dereg(select_ctx *sctx, int oldfd);

int net_getip(char *dev, struct in_addr *addr);
#ifdef ENABLE_IPV6
int net_getip6(char *dev, struct in6_addr *addr);
#endif

int net_getmac(const char *ifname, char *macaddr);

int net_close(net_interface *netif);

/*
#ifdef USING_POLL
#define fd_setR(sfd,fds)   if ((sfd) > 0) { (fds)->pfds[(fds)->count].fd = (sfd); (fds)->pfds[(fds)->count++].events = POLLIN; }
#define fd_setW(sfd,fds)   if ((sfd) > 0) { (fds)->pfds[(fds)->count].fd = (sfd); (fds)->pfds[(fds)->count++].events = POLLOUT; }
#define fd_issetR(fd,fds)  ((fd) > 0 && (fds)->rfds & (1<<fd))
#else
#define net_maxfd(this,max)  fd_max((this)->fd,(max))
#endif
#define net_fdsetR(this,fds) fd_setR((this)->fd, (fds))
#define net_fdsetW(this,fds) fd_setW((this)->fd, (fds))
#define net_issetR(this,fds) fd_issetR((this)->fd, (fds))
#define net_issetW(this,fds) fd_issetW((this)->fd, (fds))

#if defined(USING_PCAP)
#define net_close(this)     if ((this)->pd) pcap_close((this)->pd); (this)->pd=0; (this)->fd=0
#else
#define net_close(this)     if ((this)->fd > 0) close((this)->fd); (this)->fd=0
#endif
*/

#define fd_zero(fds)       FD_ZERO((fds));
#define fd_set(fd,fds)     if ((fd) > 0) FD_SET((fd), (fds))
#define fd_isset(fd,fds)   ((fd) > 0) && FD_ISSET((fd), (fds))
#define fd_max(fd,max)     (max) = (max) > (fd) ? (max) : (fd)

#define net_add_route(dst,gw,mask) net_route(dst,gw,mask,0)
#define net_del_route(dst,gw,mask) net_route(dst,gw,mask,1)

int dev_set_flags(char const *dev, int flags);
int dev_get_flags(char const *dev, int *flags);
int dev_set_addr(char const *devname, struct in_addr *addr, 
		 struct in_addr *gateway, struct in_addr *netmask);

int net_set_address(net_interface *netif, struct in_addr *address, 
		    struct in_addr *gateway, struct in_addr *netmask);

void net_run(net_interface *iface);

#endif
