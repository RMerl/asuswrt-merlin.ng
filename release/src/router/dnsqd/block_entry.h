#ifndef __BLOCK_ENTRY_H__	
#define __BLOCK_ENTRY_H__	

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "list.h"
#include "dns.h"
#include "log.h"

typedef struct block_entry_node_s {
  char name[DNS_MAX_NAME_LEN + 1];
  int type;
  struct list_head list;
} block_entry_node_t;

extern void block_entry_list_free(struct list_head *list);
extern block_entry_node_t* block_entry_node_new();
extern void block_entry_list_dump(struct list_head *list);
#endif // __BLOCK_ENTRY_H__	
