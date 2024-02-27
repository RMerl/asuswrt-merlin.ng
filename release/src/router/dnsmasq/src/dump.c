/* dnsmasq is Copyright (c) 2000-2024 Simon Kelley

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 dated June, 1991, or
   (at your option) version 3 dated 29 June, 2007.
 
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
     
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "dnsmasq.h"

#ifdef HAVE_DUMPFILE

#include <netinet/icmp6.h>

static u32 packet_count;
static void do_dump_packet(int mask, void *packet, size_t len,
			   union mysockaddr *src, union mysockaddr *dst, int port, int proto);

/* https://wiki.wireshark.org/Development/LibpcapFileFormat */
struct pcap_hdr_s {
        u32 magic_number;   /* magic number */
        u16 version_major;  /* major version number */
        u16 version_minor;  /* minor version number */
        u32 thiszone;       /* GMT to local correction */
        u32 sigfigs;        /* accuracy of timestamps */
        u32 snaplen;        /* max length of captured packets, in octets */
        u32 network;        /* data link type */
};

struct pcaprec_hdr_s {
        u32 ts_sec;         /* timestamp seconds */
        u32 ts_usec;        /* timestamp microseconds */
        u32 incl_len;       /* number of octets of packet saved in file */
        u32 orig_len;       /* actual length of packet */
};


void dump_init(void)
{
  struct stat buf;
  struct pcap_hdr_s header;
  struct pcaprec_hdr_s pcap_header;

  packet_count = 0;
  
  if (stat(daemon->dump_file, &buf) == -1)
    {
      /* doesn't exist, create and add header */
      header.magic_number = 0xa1b2c3d4;
      header.version_major = 2;
      header.version_minor = 4;
      header.thiszone = 0;
      header.sigfigs = 0;
      header.snaplen = daemon->edns_pktsz + 200; /* slop for IP/UDP headers */
      header.network = 101; /* DLT_RAW http://www.tcpdump.org/linktypes.html */

      if (errno != ENOENT ||
	  (daemon->dumpfd = creat(daemon->dump_file, S_IRUSR | S_IWUSR)) == -1 ||
	  !read_write(daemon->dumpfd, (void *)&header, sizeof(header), 0))
	die(_("cannot create %s: %s"), daemon->dump_file, EC_FILE);
    }
  else if ((daemon->dumpfd = open(daemon->dump_file, O_APPEND | O_RDWR)) == -1 ||
	   !read_write(daemon->dumpfd, (void *)&header, sizeof(header), 1))
    die(_("cannot access %s: %s"), daemon->dump_file, EC_FILE);
  else if (header.magic_number != 0xa1b2c3d4)
    die(_("bad header in %s"), daemon->dump_file, EC_FILE);
  else
    {
      /* count existing records */
      while (read_write(daemon->dumpfd, (void *)&pcap_header, sizeof(pcap_header), 1))
	{
	  lseek(daemon->dumpfd, pcap_header.incl_len, SEEK_CUR);
	  packet_count++;
	}
    }
}

void dump_packet_udp(int mask, void *packet, size_t len,
		     union mysockaddr *src, union mysockaddr *dst, int fd)
{
  union mysockaddr fd_addr;
  socklen_t addr_len = sizeof(fd_addr);

  if (daemon->dumpfd != -1 && (mask & daemon->dump_mask))
     {
       /* if fd is negative it carries a port number (negated) 
	  which we use as a source or destination when not otherwise
	  specified so wireshark can ID the packet. 
	  If both src and dst are specified, set this to -1 to avoid
	  a spurious getsockname() call. */
       int port = (fd < 0) ? -fd : -1;
       
       /* fd >= 0 is a file descriptor and the address of that file descriptor is used
	  in place of a NULL src or dst. */
       if (fd >= 0 && getsockname(fd, (struct sockaddr *)&fd_addr, &addr_len) != -1)
	 {
	   if (!src)
	     src = &fd_addr;
	   
	   if (!dst)
	     dst = &fd_addr;
	 }
       
       do_dump_packet(mask, packet, len, src, dst, port, IPPROTO_UDP);
     }
}

void dump_packet_icmp(int mask, void *packet, size_t len,
		      union mysockaddr *src, union mysockaddr *dst)
{
  if (daemon->dumpfd != -1 && (mask & daemon->dump_mask))
    do_dump_packet(mask, packet, len, src, dst, -1, IPPROTO_ICMP);
}

static void do_dump_packet(int mask, void *packet, size_t len,
			   union mysockaddr *src, union mysockaddr *dst, int port, int proto)
{
  struct ip ip;
  struct ip6_hdr ip6;
  int family;
  struct udphdr {
    u16 uh_sport;               /* source port */
    u16 uh_dport;               /* destination port */
    u16 uh_ulen;                /* udp length */
    u16 uh_sum;                 /* udp checksum */
  } udp;
  struct pcaprec_hdr_s pcap_header;
  struct timeval time;
  u32 i, sum;
  void *iphdr;
  size_t ipsz;
  int rc;
     
  /* if port != -1 it carries a port number 
     which we use as a source or destination when not otherwise
     specified so wireshark can ID the packet. 
     If both src and dst are specified, set this to -1 to avoid
     a spurious getsockname() call. */
  udp.uh_sport = udp.uh_dport = htons(port < 0 ? 0 : port);
  
  if (src)
    family = src->sa.sa_family;
  else
    family = dst->sa.sa_family;

  if (family == AF_INET6)
    {
      iphdr = &ip6;
      ipsz = sizeof(ip6);
      memset(&ip6, 0, sizeof(ip6));
      
      ip6.ip6_vfc = 6 << 4;
      ip6.ip6_hops = 64;

      if ((ip6.ip6_nxt = proto) == IPPROTO_UDP)
	ip6.ip6_plen = htons(sizeof(struct udphdr) + len);
      else
	{
	  proto = ip6.ip6_nxt = IPPROTO_ICMPV6;
	  ip6.ip6_plen = htons(len);
	}
      
      if (src)
	{
	  memcpy(&ip6.ip6_src, &src->in6.sin6_addr, IN6ADDRSZ);
	  udp.uh_sport = src->in6.sin6_port;
	}
      
      if (dst)
	{
	  memcpy(&ip6.ip6_dst, &dst->in6.sin6_addr, IN6ADDRSZ);
	  udp.uh_dport = dst->in6.sin6_port;
	}
            
      /* start UDP checksum */
      for (sum = 0, i = 0; i < IN6ADDRSZ; i+=2)
	{
	  sum += ntohs((ip6.ip6_src.s6_addr[i] << 8) + (ip6.ip6_src.s6_addr[i+1])) ;
	  sum += ntohs((ip6.ip6_dst.s6_addr[i] << 8) + (ip6.ip6_dst.s6_addr[i+1])) ; 
	}
    }
  else
    {
      iphdr = &ip;
      ipsz = sizeof(ip);
      memset(&ip, 0, sizeof(ip));
      
      ip.ip_v = IPVERSION;
      ip.ip_hl = sizeof(struct ip) / 4;
      ip.ip_ttl = IPDEFTTL;

      if ((ip.ip_p = proto) == IPPROTO_UDP)
	ip.ip_len = htons(sizeof(struct ip) + sizeof(struct udphdr) + len);
      else
	{
	  ip.ip_len = htons(sizeof(struct ip) + len);
	  proto = ip.ip_p = IPPROTO_ICMP;
	}
      
      if (src)
	{
	  ip.ip_src = src->in.sin_addr;
	  udp.uh_sport = src->in.sin_port;
	}

      if (dst)
	{
	  ip.ip_dst = dst->in.sin_addr;
	  udp.uh_dport = dst->in.sin_port;
	}
      
      ip.ip_sum = 0;
      for (sum = 0, i = 0; i < sizeof(struct ip) / 2; i++)
	sum += ((u16 *)&ip)[i];
      while (sum >> 16)
	sum = (sum & 0xffff) + (sum >> 16);  
      ip.ip_sum = (sum == 0xffff) ? sum : ~sum;
      
      /* start UDP/ICMP checksum */
      sum = ip.ip_src.s_addr & 0xffff;
      sum += (ip.ip_src.s_addr >> 16) & 0xffff;
      sum += ip.ip_dst.s_addr & 0xffff;
      sum += (ip.ip_dst.s_addr >> 16) & 0xffff;
    }
  
  if (len & 1)
    ((unsigned char *)packet)[len] = 0; /* for checksum, in case length is odd. */

  if (proto == IPPROTO_UDP)
    {
      /* Add Remaining part of the pseudoheader. Note that though the
	 IPv6 pseudoheader is very different to the IPv4 one, the 
	 net result of this calculation is correct as long as the 
	 packet length is less than 65536, which is fine for us. */
      sum += htons(IPPROTO_UDP);
      sum += htons(sizeof(struct udphdr) + len);
      
      udp.uh_sum = 0;
      udp.uh_ulen = htons(sizeof(struct udphdr) + len);
      
      for (i = 0; i < sizeof(struct udphdr)/2; i++)
	sum += ((u16 *)&udp)[i];
      for (i = 0; i < (len + 1) / 2; i++)
	sum += ((u16 *)packet)[i];
      while (sum >> 16)
	sum = (sum & 0xffff) + (sum >> 16);
      udp.uh_sum = (sum == 0xffff) ? sum : ~sum;

      pcap_header.incl_len = pcap_header.orig_len = ipsz + sizeof(udp) + len;
    }
  else
    {
      /* ICMP - ICMPv6 packet is a superset of ICMP */
      struct icmp6_hdr *icmp = packet;
      
      /* See comment in UDP code above. */
      sum += htons(proto);
      sum += htons(len);
      
      icmp->icmp6_cksum = 0;
      for (i = 0; i < (len + 1) / 2; i++)
	sum += ((u16 *)packet)[i];
      while (sum >> 16)
	sum = (sum & 0xffff) + (sum >> 16);
      icmp->icmp6_cksum = (sum == 0xffff) ? sum : ~sum;

      pcap_header.incl_len = pcap_header.orig_len = ipsz + len;
    }
    
  rc = gettimeofday(&time, NULL);
  pcap_header.ts_sec = time.tv_sec;
  pcap_header.ts_usec = time.tv_usec;
  
  if (rc == -1 ||
      !read_write(daemon->dumpfd, (void *)&pcap_header, sizeof(pcap_header), 0) ||
      !read_write(daemon->dumpfd, iphdr, ipsz, 0) ||
      (proto == IPPROTO_UDP && !read_write(daemon->dumpfd, (void *)&udp, sizeof(udp), 0)) ||
      !read_write(daemon->dumpfd, (void *)packet, len, 0))
    my_syslog(LOG_ERR, _("failed to write packet dump"));
  else if (option_bool(OPT_EXTRALOG))
    my_syslog(LOG_INFO, _("%u dumping packet %u mask 0x%04x"),  daemon->log_display_id, ++packet_count, mask);
  else
    my_syslog(LOG_INFO, _("dumping packet %u mask 0x%04x"), ++packet_count, mask);

}

#endif
