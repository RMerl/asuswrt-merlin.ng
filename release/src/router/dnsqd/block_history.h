#ifndef __BLOCK_HISTORY_H__	
#define __BLOCK_HISTORY_H__	

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "list.h"
#include "dns.h"
#include "log.h"

typedef struct block_history_node_s {
  int isv4;
  uint32_t client_ip;
  struct in6_addr srcv6;
  char name[DNS_MAX_NAME_LEN + 1];
  char mac[ETHER_ADDR_LENGTH];
  int timestamp;
  struct list_head list;
} block_history_node_t;

extern void block_history_list_free(struct list_head *list);
extern block_history_node_t* block_history_node_new();
extern void block_history_list_dump(struct list_head *list);
#endif // __BLOCK_HISTORY_H__	
