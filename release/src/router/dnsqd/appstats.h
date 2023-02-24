#ifndef __APPSTATS_H__	
#define __APPSTATS_H__	

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "list.h"
#include "dns.h"
#include "log.h"

typedef struct appstats_node_s {
  int isv4;
  uint32_t client_ip;
  struct in6_addr srcv6;
  uint64_t up_bytes;
  uint64_t dn_bytes;
  uint64_t up_dif_bytes;
  uint64_t dn_dif_bytes;
  char name[DNS_MAX_NAME_LEN + 1];
  char mac[ETHER_ADDR_LENGTH];
  struct list_head list;
} appstats_node_t;

extern void appstats_list_free(struct list_head *list);
extern appstats_node_t* appstats_node_new();
extern void appstats_list_dump(struct list_head *list);
#endif // __APPSTATS_H__		
