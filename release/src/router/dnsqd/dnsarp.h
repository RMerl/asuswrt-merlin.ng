#ifndef __NFARP_H__
#define __NFARP_H__

#include "log.h"
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

typedef struct arp_node_s {
  int isv4;
  struct in_addr srcv4;
  struct in6_addr srcv6;
  char mac[ETHER_ADDR_LENGTH];
  struct list_head list;
} arp_node_t;

extern void arp_list_free(struct list_head *list);
extern void arp_list_parse(struct list_head *arlist);
extern void arp_list_dump(struct list_head *arlist);
extern arp_node_t*  arp_list_search(bool isv4, char* ipstr, struct list_head *arlist);

//#define GET_IPV6_MAC_SYS_COMMAND "ip -6 neigh show"
#endif // __NFARP_H__
