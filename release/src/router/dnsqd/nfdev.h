#ifndef __NFDEV_H__	
#define __NFDEV_H__	

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "list.h"
#include "dns.h"
#include "log.h"

typedef struct nfdev_node_s {
  int isv4;
  int timestamp;
  uint32_t client_ip;
  struct in6_addr srcv6;
  uint64_t tx;
  uint64_t rx;
  uint64_t count;
  char domain_name[DNS_MAX_NAME_LEN + 1];
  char mac[ETHER_ADDR_LENGTH];
  char dev_type[DEV_TYPE_LEN + 1];
  struct list_head list;
} nfdev_node_t;

extern void nfdev_list_free(struct list_head *list);
extern nfdev_node_t* nfdev_node_new();
extern void nfdev_list_dump(struct list_head *list);
#endif // __NFDEV_H__	
