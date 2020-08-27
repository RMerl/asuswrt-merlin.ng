#ifndef __NFFDB_H__
#define __NFFDB_H__

#include "list.h"
#include "log.h"

#ifndef MAC_STR_LEN
#define MAC_STR_LEN 18
#endif

enum fdb_node_sw_attr {
	FDB_ATTR_SW_MAC = 0,
	FDB_ATTR_SW_FID,
	FDB_ATTR_SW_STATIC,
	FDB_ATTR_SW_PORT,
	
	FDB_ATTR_SW_MAX
};

enum fdb_node_wl_attr {
	FDB_ATTR_WL_MAC = 0,
	FDB_ATTR_WL_AID,
	FDB_ATTR_WL_CHAN,
	FDB_ATTR_WL_TXRATE,
	FDB_ATTR_WL_RXRATE,
	FDB_ATTR_WL_RSSI,
	
	FDB_ATTR_WL_MAX
};

typedef struct fdb_node_s {
#if defined(RTAX89U) || defined(GTAXY16000)
    uint8_t sw[2];
#endif
    bool iswl;
    uint8_t port;  // switch: physical port, wireless: 2G, 5G
	char mac[ETHER_ADDR_LENGTH];

	struct list_head list;
} fdb_node_t;

extern void fdb_list_free(struct list_head *list);



#endif // __NFFDB_H__
