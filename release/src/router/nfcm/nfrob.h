#ifndef __NFROB_H__
#define __NFROB_H__

#include "list.h"
#include "log.h"

enum rob_node_attr {
	ROB_ATTR_PORT = 1,
	ROB_ATTR_STATE = 2,
	ROB_ATTR_ENABLED = 3,
	ROB_ATTR_STP = 5,
	ROB_ATTR_VLAN = 7,
	ROB_ATTR_JUMBO = 9,
	ROB_ATTR_MAC = 11,
	
	ROB_ATTR_MAX
};

typedef struct rob_node_s {
	bool iswl;
	uint8_t port;
	bool state;
	bool enabled;
	uint16_t vlan;
	char mac[ETHER_ADDR_LENGTH];

	struct list_head list;
} rob_node_t;

extern void rob_list_free(struct list_head *list);

#endif // __NFROB_H__	
