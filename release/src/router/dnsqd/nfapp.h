#ifndef __NFAPP_H__	
#define __NFAPP_H__	

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "list.h"
#include "dns.h"
#include "log.h"

typedef struct nfapp_node_s {
  int data_id;
  int timestamp;
  int isv4;
  uint32_t src_ip;
  struct in6_addr srcv6;
  uint16_t src_port;
  uint32_t dst_ip;
  struct in6_addr dstv6;
  uint16_t dst_port;
  uint64_t up_bytes;
  uint64_t up_dif_bytes;
  uint64_t dn_bytes;
  uint64_t dn_dif_bytes;
  uint64_t tx;
  uint64_t rx;
  char name[DNS_MAX_NAME_LEN + 1];
  char mac[ETHER_ADDR_LENGTH];
  char app_name[DNS_MAX_NAME_LEN + 1];
  struct list_head list;
} nfapp_node_t;

extern void nfapp_list_free(struct list_head *list);
extern nfapp_node_t* nfapp_node_new();
extern void nfapp_list_dump(struct list_head *list);
#endif // __NFAPP_H__	
