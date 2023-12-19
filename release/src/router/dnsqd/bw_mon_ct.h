#ifndef __BW_MON_H__
#define __BW_MON_H__

#include <libnetfilter_conntrack/libnetfilter_conntrack.h>
#include <libnetfilter_conntrack/libnetfilter_conntrack_tcp.h>
#include <libnetfilter_conntrack_tcp.h>
#include <internal/internal.h>

//#include <libnetfilter_conntrack_tcp.h>
#include "list.h"
#include "log.h"
#include <sys/types.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>

#define DEFAULT_IPV6_ADDR "fec0::1"


typedef struct bw_nf_node_s {
    bool isv4;
    char src_mac[ETHER_ADDR_LENGTH];

    uint8_t proto;

    struct in_addr srcv4;
    uint16_t src_port;
    struct in6_addr srcv6;
    char src6_ip[INET6_ADDRSTRLEN]; // for db    

    struct in_addr dstv4;
    uint16_t dst_port;    //when proto is ICMP, the dst_port is "(type<<8)|code"
    struct in6_addr dstv6;
    char dst6_ip[INET6_ADDRSTRLEN]; // for db    

    uint64_t up_bytes;
    uint64_t up_dif_bytes;
   // uint64_t up_ttl_bytes;
   // int64_t  up_speed; // bytes per second

    uint64_t dn_bytes;
    uint64_t dn_dif_bytes;
   // uint64_t dn_ttl_bytes;
   // int64_t  dn_speed; // bytes per second

   // phy_port_t layer1_info;

   // time_t timestamp; //first timestamp the node created
   // time_t time_in;   // time elasped in the invalid state
   // uint8_t state; // dedicated for TCP conntrack
    int is_app_checked;
    char app_name[64];
    int app_id;
    int cat_id;

    int available;
    struct list_head list;
} bw_nf_node_t;

typedef struct lan_info_s {
    char ifname[IFNAMESIZE];
    bool enabled;
    struct in_addr addr;
    struct in_addr subnet;
    struct list_head list;
} lan_info_t;


extern int init_bw_mon_ct();
extern int deinit_bw_mon_ct();
extern int bw_mon_ct_cb(enum nf_conntrack_msg_type type,
                        struct nf_conntrack *ct,
                        void *data);


extern bool is_in_lanv4(struct in_addr *src);
extern bool is_in_lanv6(struct in6_addr *src);
extern bool is_local_link_addr(struct in6_addr *addr);
extern bool is_router_addr(struct in_addr *addr);
extern bool is_router_v6addr(struct in6_addr *addr);
extern bool is_multi_addr(struct in_addr *addr);
extern bool is_broadcast_addr(struct in_addr *addr);
extern bool is_acceptable_addr(bw_nf_node_t *nn);

extern void nf_list_dump(char *title, struct list_head *list);
extern void nf_list_free(struct list_head *list);
extern void nf_list_move(struct list_head *dst, struct list_head *src);

extern int bw_ct_list_to_json(struct list_head *bw_ct_list);
extern void bw_nf_list_diff_calc(struct list_head *iplist, struct list_head *bklist);
extern int bw_nf_list_statistics_calc(struct list_head *iplist, struct list_head *smlist);

#endif // __BW_MON_H__
