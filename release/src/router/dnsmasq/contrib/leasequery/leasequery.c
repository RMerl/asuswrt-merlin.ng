/* leasequery is Copyright (c) 2025 Simon Kelley

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

/* Author's email: simon@thekelleys.org.uk */

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <limits.h>
#include <net/if.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <netdb.h>
#include <linux/types.h>
#include <linux/capability.h>
#include <arpa/inet.h>
#include <ctype.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

#define INADDRSZ        4
#define ADDRSTRLEN      46

#define option_len(opt) ((int)(((unsigned char *)(opt))[1]))
#define option_ptr(opt, i) ((void *)&(((unsigned char *)(opt))[2u+(unsigned int)(i)]))

#define DHCP_CHADDR_MAX  16
#define DHCP_SERVER_PORT 67
#define DHCP_CLIENT_PORT 68
#define BOOTREQUEST      1
#define BOOTREPLY        2

#define DHCP_COOKIE              0x63825363

#define DHCPLEASEQUERY          10
#define DHCPLEASEUNASSIGNED     11
#define DHCPLEASEUNKNOWN        12
#define DHCPLEASEACTIVE         13

#define OPTION_END               255
#define OPTION_ROUTER            3
#define OPTION_VENDOR_CLASS_OPT  43
#define OPTION_LEASE_TIME        51
#define OPTION_MESSAGE_TYPE      53
#define OPTION_SERVER_IDENTIFIER 54
#define OPTION_REQUESTED_OPTIONS 55
#define OPTION_VENDOR_ID         60
#define OPTION_CLIENT_ID         61

/* flags in top of length field for DHCP-option tables */
#define OT_ADDR_LIST    0x8000
#define OT_RFC1035_NAME 0x4000
#define OT_INTERNAL     0x2000
#define OT_NAME         0x1000
#define OT_CSTRING      0x0800
#define OT_DEC          0x0400 
#define OT_TIME         0x0200

#define GETSHORT(s, cp) do { \
	unsigned char *t_cp = (unsigned char *)(cp); \
	(s) = ((u16)t_cp[0] << 8) \
	    | ((u16)t_cp[1]) \
	    ; \
	(cp) += 2; \
  } while(0)

#define GETLONG(l, cp) do { \
	unsigned char *t_cp = (unsigned char *)(cp); \
	(l) = ((u32)t_cp[0] << 24) \
	    | ((u32)t_cp[1] << 16) \
	    | ((u32)t_cp[2] << 8) \
	    | ((u32)t_cp[3]) \
	    ; \
	(cp) += 4; \
  } while (0)

#define PUTSHORT(s, cp) do { \
	u16 t_s = (u16)(s); \
	unsigned char *t_cp = (unsigned char *)(cp); \
	*t_cp++ = t_s >> 8; \
	*t_cp   = t_s; \
	(cp) += 2; \
  } while(0)

#define PUTLONG(l, cp) do { \
	u32 t_l = (u32)(l); \
	unsigned char *t_cp = (unsigned char *)(cp); \
	*t_cp++ = t_l >> 24; \
	*t_cp++ = t_l >> 16; \
	*t_cp++ = t_l >> 8; \
	*t_cp   = t_l; \
	(cp) += 4; \
  } while (0)

union all_addr {
  struct in_addr addr4;
  struct in6_addr addr6;
};
  
struct dhcp_packet_with_opts{
  struct dhcp_packet {
    unsigned char op, htype, hlen, hops;
    unsigned int xid;
    unsigned short secs, flags;
    struct in_addr ciaddr, yiaddr, siaddr, giaddr;
    unsigned char chaddr[DHCP_CHADDR_MAX], sname[64], file[128];
  } header;
  unsigned char options[312];
};

static const struct opttab_t {
  char *name;
  u16 val, size;
} opttab[] = {
  { "netmask", 1, OT_ADDR_LIST },
  { "time-offset", 2, 4 },
  { "router", 3, OT_ADDR_LIST  },
  { "dns-server", 6, OT_ADDR_LIST },
  { "log-server", 7, OT_ADDR_LIST },
  { "lpr-server", 9, OT_ADDR_LIST },
  { "hostname", 12, OT_INTERNAL | OT_NAME },
  { "boot-file-size", 13, 2 | OT_DEC },
  { "domain-name", 15, OT_NAME },
  { "swap-server", 16, OT_ADDR_LIST },
  { "root-path", 17, OT_NAME },
  { "extension-path", 18, OT_NAME },
  { "ip-forward-enable", 19, 1 },
  { "non-local-source-routing", 20, 1 },
  { "policy-filter", 21, OT_ADDR_LIST },
  { "max-datagram-reassembly", 22, 2 | OT_DEC },
  { "default-ttl", 23, 1 | OT_DEC },
  { "mtu", 26, 2 | OT_DEC },
  { "all-subnets-local", 27, 1 },
  { "broadcast", 28, OT_INTERNAL | OT_ADDR_LIST },
  { "router-discovery", 31, 1 },
  { "router-solicitation", 32, OT_ADDR_LIST },
  { "static-route", 33, OT_ADDR_LIST },
  { "trailer-encapsulation", 34, 1 },
  { "arp-timeout", 35, 4 | OT_DEC },
  { "ethernet-encap", 36, 1 },
  { "tcp-ttl", 37, 1 },
  { "tcp-keepalive", 38, 4 | OT_DEC },
  { "nis-domain", 40, OT_NAME },
  { "nis-server", 41, OT_ADDR_LIST },
  { "ntp-server", 42, OT_ADDR_LIST },
  { "vendor-encap", 43, OT_INTERNAL },
  { "netbios-ns", 44, OT_ADDR_LIST },
  { "netbios-dd", 45, OT_ADDR_LIST },
  { "netbios-nodetype", 46, 1 },
  { "netbios-scope", 47, 0 },
  { "x-windows-fs", 48, OT_ADDR_LIST },
  { "x-windows-dm", 49, OT_ADDR_LIST },
  { "requested-address", 50, OT_INTERNAL | OT_ADDR_LIST },
  { "lease-time", 51, OT_INTERNAL | OT_TIME },
  { "option-overload", 52, OT_INTERNAL },
  { "message-type", 53, OT_INTERNAL | OT_DEC },
  { "server-identifier", 54, OT_INTERNAL | OT_ADDR_LIST },
  { "parameter-request", 55, OT_INTERNAL },
  { "message", 56, OT_INTERNAL },
  { "max-message-size", 57, OT_INTERNAL },
  { "T1", 58, OT_TIME},
  { "T2", 59, OT_TIME},
  { "vendor-class", 60, OT_NAME },
  { "client-id", 61, OT_INTERNAL },
  { "nis+-domain", 64, OT_NAME },
  { "nis+-server", 65, OT_ADDR_LIST },
  { "tftp-server", 66, OT_NAME },
  { "bootfile-name", 67, OT_NAME },
  { "mobile-ip-home", 68, OT_ADDR_LIST }, 
  { "smtp-server", 69, OT_ADDR_LIST }, 
  { "pop3-server", 70, OT_ADDR_LIST }, 
  { "nntp-server", 71, OT_ADDR_LIST }, 
  { "irc-server", 74, OT_ADDR_LIST }, 
  { "user-class", 77, 0 },
  { "rapid-commit", 80, 0 },
  { "FQDN", 81, OT_INTERNAL },
  { "agent-info", 82, OT_INTERNAL },
  { "last-transaction", 91, 4 | OT_TIME },
  { "associated-ip", 92, OT_ADDR_LIST },
  { "client-arch", 93, 2 | OT_DEC },
  { "client-interface-id", 94, 0 },
  { "client-machine-id", 97, 0 },
  { "posix-timezone", 100, OT_NAME }, /* RFC 4833, Sec. 2 */
  { "tzdb-timezone", 101, OT_NAME }, /* RFC 4833, Sec. 2 */
  { "ipv6-only", 108, 4 | OT_DEC },  /* RFC 8925 */ 
  { "subnet-select", 118, OT_INTERNAL },
  { "domain-search", 119, OT_RFC1035_NAME },
  { "sip-server", 120, 0 },
  { "classless-static-route", 121, 0 },
  { "vendor-id-encap", 125, 0 },
  { "tftp-server-address", 150, OT_ADDR_LIST },
  { "server-ip-address", 255, OT_ADDR_LIST }, /* special, internal only, sets siaddr */
  { NULL, 0, 0 }
};

static void prettyprint_time(char *buf, unsigned int t);
static char *print_mac(char *buff, unsigned char *mac, int len);

int lookup_dhcp_opt(char *name)
{
  const struct opttab_t *t = opttab;
  int i;

  for (i = 0; t[i].name; i++)
    if (strcasecmp(t[i].name, name) == 0)
      return t[i].val;
  
  return -1;
}

char *option_string(unsigned int opt, unsigned char *val, int opt_len, char *buf, int buf_len)
{
  int o, i, j, nodecode = 0;
  const struct opttab_t *ot = opttab;
  char addrbuff[ADDRSTRLEN];
  
  for (o = 0; ot[o].name; o++)
    if (ot[o].val == opt)
      {
	if (buf)
	  {
	    memset(buf, 0, buf_len);
	    
	    if (ot[o].size & OT_ADDR_LIST) 
	      {
		union all_addr addr;
		int addr_len = INADDRSZ;

		for (buf[0]= 0, i = 0; i <= opt_len - addr_len; i += addr_len) 
		  {
		    if (i != 0)
		      strncat(buf, ", ", buf_len - strlen(buf));
		    /* align */
		    memcpy(&addr, &val[i], addr_len); 
		    inet_ntop(AF_INET, &val[i], addrbuff, ADDRSTRLEN);
		    strncat(buf, addrbuff, buf_len - strlen(buf));
		  }
	      }
	    else if (ot[o].size & OT_NAME)
		for (i = 0, j = 0; i < opt_len && j < buf_len ; i++)
		  {
		    char c = val[i];
		    if (isprint((unsigned char)c))
		      buf[j++] = c;
		  }
	    else if ((ot[o].size & (OT_DEC | OT_TIME)) && opt_len != 0)
	      {
		unsigned int dec = 0;
		
		for (i = 0; i < opt_len; i++)
		  dec = (dec << 8) | val[i]; 

		if (ot[o].size & OT_TIME)
		  prettyprint_time(buf, dec);
		else
		  sprintf(buf, "%u", dec);
	      }
	    else
	      nodecode = 1;
	  }
	break;
      }

  if (opt_len != 0 && buf && (!ot[o].name || nodecode))
    {
      int trunc  = 0;
      if (opt_len > 14)
	{
	  trunc = 1;
	  opt_len = 14;
	}
      print_mac(buf, val, opt_len);
      if (trunc)
	strncat(buf, "...", buf_len - strlen(buf));
    

    }

  return ot[o].name ? ot[o].name : "";

}

static void prettyprint_time(char *buf, unsigned int t)
{
  if (t == 0xffffffff)
    sprintf(buf, "infinite");
  else
    {
      unsigned int x, p = 0;
       if ((x = t/86400))
	p += sprintf(&buf[p], "%ud", x);
       if ((x = (t/3600)%24))
	p += sprintf(&buf[p], "%uh", x);
      if ((x = (t/60)%60))
	p += sprintf(&buf[p], "%um", x);
      if ((x = t%60))
	sprintf(&buf[p], "%us", x);
    }
}

static char *print_mac(char *buff, unsigned char *mac, int len)
{
  char *p = buff;
  int i;
   
  if (len == 0)
    sprintf(p, "<null>");
  else
    for (i = 0; i < len; i++)
      p += sprintf(p, "%.2x%s", mac[i], (i == len - 1) ? "" : ":");
  
  return buff;
}

static unsigned char *dhcp_skip_opts(struct dhcp_packet_with_opts *pktp)
{
  unsigned char *start = (unsigned char *)(((unsigned int *)&pktp->options[0]) + 1);
  while (*start != 0)
    start += start[1] + 2;
  return start;
}

static unsigned char *dhcp_find_opt(struct dhcp_packet_with_opts *pktp, int optno)
{
  unsigned char *start = (unsigned char *)(((unsigned int *)&pktp->options[0]) + 1);
  while (*start != OPTION_END)
    {
      if (*start == optno)
	return start;
      start += start[1] + 2;
    }

  return NULL;
}

static void option_put(struct dhcp_packet_with_opts *pktp, int opt, int len, unsigned int val)
{
  int i;
  unsigned char *p = dhcp_skip_opts(pktp);
  
  if (p) 
    {
      *p++ = opt;
      *p++ = len;
      for (i = 0; i < len; i++)
      *(p++) = val >> (8 * (len - (i + 1)));
    }
}

static void option_put_string(struct dhcp_packet_with_opts *pktp, int opt, 
			      const char *string, int null_term)
{
  unsigned char *p;
  size_t len = strlen(string);

  if (null_term && len != 255)
    len++;

  if ((p = dhcp_skip_opts(pktp)))
    {
      *p++ = opt;
      *p++ = len;
      memcpy(p, string, len);
    }
}


/* in may equal out, when maxlen may be -1 (No max len). 
   Return -1 for extraneous no-hex chars found. */
int parse_hex(char *in, unsigned char *out, int maxlen, 
	      unsigned int *wildcard_mask, int *mac_type)
{
  int done = 0, mask = 0, i = 0;
  char *r;
    
  if (mac_type)
    *mac_type = 0;
  
  while (!done && (maxlen == -1 || i < maxlen))
    {
      for (r = in; *r != 0 && *r != ':' && *r != '-' && *r != ' '; r++)
	if (*r != '*' && !isxdigit((unsigned char)*r))
	  return -1;
      
      if (*r == 0)
	done = 1;
      
      if (r != in )
	{
	  if (*r == '-' && i == 0 && mac_type)
	   {
	      *r = 0;
	      *mac_type = strtol(in, NULL, 16);
	      mac_type = NULL;
	   }
	  else
	    {
	      *r = 0;
	      if (strcmp(in, "*") == 0)
		{
		  mask = (mask << 1) | 1;
		  i++;
		}
	      else
		{
		  int j, bytes = (1 + (r - in))/2;
		  for (j = 0; j < bytes; j++)
		    { 
		      char sav;
		      if (j < bytes - 1)
			{
			  sav = in[(j+1)*2];
			  in[(j+1)*2] = 0;
			}
		      /* checks above allow mix of hexdigit and *, which
			 is illegal. */
		      if (strchr(&in[j*2], '*'))
			return -1;
		      out[i] = strtol(&in[j*2], NULL, 16);
		      mask = mask << 1;
		      if (++i == maxlen)
			break; 
		      if (j < bytes - 1)
			in[(j+1)*2] = sav;
		    }
		}
	    }
	}
      in = r+1;
    }
  
  if (wildcard_mask)
    *wildcard_mask = mask;

  return i;
}

static char *split_chr(char *s, char c)
{
  char *comma, *p;

  if (!s || !(comma = strchr(s, c)))
    return NULL;
  
  p = comma;
  *comma = ' ';
  
  for (; *comma == ' '; comma++);
  
  for (; (p >= s) && *p == ' '; p--)
    *p = 0;
  
  return comma;
}

static char *split(char *s)
{
  return split_chr(s, ',');
}


int main(int argc, char **argv)
{
  int fd;
  struct ifreq ifr;
  struct in_addr iface_addr, server_addr, lease_addr;
  struct dhcp_packet_with_opts pkt;
  struct sockaddr_in saddr;
  unsigned char *p;
  ssize_t sz;
  unsigned char mac[DHCP_CHADDR_MAX], clid[256], req_options[256];
  char buff[500];
  int clid_len = 0, mac_len = 0, mac_type = 0, opts_len = 0;
  unsigned short port = DHCP_SERVER_PORT;
  unsigned int xid;
  struct timeval tv;
  
  gettimeofday(&tv, NULL);
  srand(tv.tv_usec);
  xid = rand();
  server_addr.s_addr = lease_addr.s_addr = iface_addr.s_addr = 0;
  
  if ((fd = socket(PF_INET, SOCK_DGRAM, 0)) == -1)
    {
      perror("leasequery: cannot create socket");
      exit(1);
    }
  
  while (1)
    {
      int option = getopt(argc, argv, "i:s:l:m:c:p:r:");

      if (option == -1)
	break;
      
      switch (option) 
	{
	default:
	  fprintf(stderr,
		  "-p <port>             port on DHCP server.\n"
		  "-i <interface>        exit interface to DHCP server.\n"
		  "-s <inet-addr>        DHCP server address.\n"
		  "-l <inet-addr>        Query lease by IP address.\n"
		  "-m <MAC-addr>         Query lease by MAC address.\n"
		  "-c <hex>              Query lease by client-id.\n"
		  "-r <name>|<int>[,...] List of options to return.\n");
	  exit(1);
		  		  
	case 'p':
	  port = atoi(optarg);
	  break;
	  
	case 'i':
	  strncpy(ifr.ifr_name, optarg, IF_NAMESIZE);
	   ifr.ifr_addr.sa_family = AF_INET;
	   if (ioctl(fd, SIOCGIFADDR, &ifr) == -1)
	    {
	      perror("leasequery: cannot get interface address");
	      exit(1);
	    }

	  iface_addr = ((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr;
	  break;

	case 's':
	  if (inet_pton(AF_INET, optarg, &server_addr) <= 0)
	    {
	      fprintf(stderr, "leasequery: bad server address\n");
	      exit(1);
	    }
	  break;

	case 'l':
	  if (inet_pton(AF_INET, optarg, &lease_addr) <= 0)
	    {
	      fprintf(stderr, "leasequery: bad lease address\n");
	      exit(1);
	    }
	  break;

	case 'm':
	  mac_type = 1; /* default ethernet */
	  mac_len = parse_hex(optarg, mac, DHCP_CHADDR_MAX, NULL, &mac_type);
	  if (!mac_type)
	    mac_type  = 1; /* default ethernet */
	  break;

	case 'c':
	  clid_len = parse_hex(optarg, clid, 256, NULL, NULL);
	  break;

	case 'r':
	  {
	    char *comma;
	    int opt;
	    
	    while (optarg)
	      {
		comma = split(optarg);
		
		if ((opt = lookup_dhcp_opt(optarg)) != -1 || (opt = atoi(optarg)) != 0) 
		  req_options[opts_len++] = opt;
		
		optarg = comma;
	      }
	    
	    break;
	  }
	}
    }

  if (!server_addr.s_addr)
    {
      fprintf(stderr, "leasequery: no server address\n");
      exit(1);
    }
    
  memset(&pkt, 0, sizeof pkt);
  pkt.header.op = BOOTREQUEST;
  pkt.header.xid = xid;
  pkt.header.ciaddr = lease_addr;
  pkt.header.hlen = mac_len;
  pkt.header.htype = mac_type;
  memcpy(pkt.header.chaddr, mac, mac_len);
  *((unsigned int *)&pkt.options[0]) = htonl(DHCP_COOKIE);

  /* Dnsmasq extension. */
  pkt.header.giaddr.s_addr = iface_addr.s_addr ? iface_addr.s_addr : INADDR_BROADCAST;
    
  option_put(&pkt, OPTION_MESSAGE_TYPE, 1, DHCPLEASEQUERY);
  
  if (clid_len != 0)
    {
      p = dhcp_skip_opts(&pkt);
      *p++ = OPTION_CLIENT_ID;
      *p++ = clid_len;
      memcpy(p, clid, clid_len);
    }
  
  if (opts_len != 0)
    {
      p = dhcp_skip_opts(&pkt); 
      *p++ = OPTION_REQUESTED_OPTIONS;
      *p++ = opts_len;
      memcpy(p, req_options, opts_len);
    }
  
  if (iface_addr.s_addr)
    {
      saddr.sin_family = AF_INET;
      saddr.sin_port = htons(port);
      saddr.sin_addr.s_addr = iface_addr.s_addr;
      
      if (bind(fd, (struct sockaddr *)&saddr, sizeof(struct sockaddr_in)))
	{
	  perror("leasequery: cannot bind DHCP server socket");
	  exit(1);
	}
    }
	  
  saddr.sin_family = AF_INET;
  saddr.sin_port = htons(port);
  saddr.sin_addr = server_addr;

  while((sz = sendto(fd, &pkt, dhcp_skip_opts(&pkt) - ((unsigned char *)&pkt), 0, (struct sockaddr *)&saddr, sizeof(saddr))) == -1 &&
	errno == EINTR);

  if (sz == -1)
    {
      perror("leasequery: sendto()");
      exit(1);
    }
  
  sz = recv(fd, &pkt, sizeof(pkt), 0);

  if (sz == -1)
    {
      perror("leasequery: recv()");
      exit(1);
    }
  
  if (sz >= sizeof(pkt.header) &&
      pkt.header.op == BOOTREPLY &&
      (p = dhcp_find_opt(&pkt, OPTION_MESSAGE_TYPE)) &&
      pkt.header.xid == xid)
    {
      if (p[2] == DHCPLEASEUNASSIGNED)
	printf("UNASSIGNED\n");
      else if (p[2] == DHCPLEASEUNKNOWN)
	printf("UNKNOWN\n");
      else if (p[2] == DHCPLEASEACTIVE)
	{
	  print_mac(buff, pkt.header.chaddr, pkt.header.hlen);
	  printf("ACTIVE %s %s\n", inet_ntoa(pkt.header.ciaddr), buff);
      
	  p = (unsigned char *)(((unsigned int *)&pkt.options[0]) + 1);
	  
	  while (*p != OPTION_END)
	    {
	      if (*p != OPTION_MESSAGE_TYPE)
		{
		  char *optname = option_string(p[0], option_ptr(p, 0), option_len(p), buff, 500);
		  
		  printf("size:%3d option:%3d %s  %s\n", option_len(p), p[0], optname, buff);
		}
	      p += p[1] + 2;
	    }
	}
    }
}
