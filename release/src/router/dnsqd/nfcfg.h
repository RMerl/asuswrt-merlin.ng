#ifndef __NFCFG_H__
#define __NFCFG_H__

#include "list.h"
#include "log.h"

#ifndef ETHER_ADDR_LENGTH
#define ETHER_ADDR_LENGTH 18 // 11:22:33:44:55:66
#endif

#define CLIENTLIST_FILE_LOCK   "clientlist"
#define CLIENTLIST_JSON_PATH   "/tmp/clientlist.json"

typedef struct cli_node_s {
    bool isv4;

    char mac[ETHER_ADDR_LENGTH];
    char type[ETHER_ADDR_LENGTH];

    struct in_addr ipv4;
    struct in6_addr ipv6;

    //phy_port_t layer1_info;
    int lru_time;

    struct list_head list;
} cli_node_t;

extern cli_node_t *cli_node_new();
extern void cli_node_free(cli_node_t *cli);
extern void cli_list_free(struct list_head *list);
extern void cli_list_dump(char *title, struct list_head *list);
extern void cli_list_file_parse(struct list_head *cli_list);

#endif /* __NFCFG_H__ */
