#ifndef __DET_DNS_H__
#define __DET_DNS_H__

#define T_A 1	  // Ipv4 address
#define T_NS 2	  // Nameserver
#define T_CNAME 5 // canonical name
#define T_SOA 6	  /* start of authority zone */
#define T_PTR 12  /* domain name pointer */
#define T_MX 15	  // Mail server

int get_host_by_name(unsigned char *host, int query_type, const char *ifname, const char *dns_server, const int timeout, char *ip, const size_t ip_sz);

#endif