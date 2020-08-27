#ifndef __NFMC_H__
#define __NFMC_H__

#include "list.h"
#include "log.h"

typedef struct mc_node_s {
    bool iswl;
    uint8_t port;
	char mac[ETHER_ADDR_LENGTH];

	struct list_head list;
} mc_node_t;

extern void mc_list_free(struct list_head *list);
extern int mc_list_parse(char *fname, struct list_head *list);

#endif // __NFMC_H__
