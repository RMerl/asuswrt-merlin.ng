#ifndef __NFCM_H__
#define __NFCM_H__

#include <libnetfilter_conntrack_tcp.h>

#include "list.h"
#include "log.h"

enum exittype {
	OTHER_PROBLEM = 1,
	PARAMETER_PROBLEM,
	VERSION_PROBLEM
};

enum ct_options {
	CT_OPT_ORIG_SRC_BIT	= 0,
	CT_OPT_ORIG_SRC 	= (1 << CT_OPT_ORIG_SRC_BIT),

	CT_OPT_ORIG_DST_BIT	= 1,
	CT_OPT_ORIG_DST		= (1 << CT_OPT_ORIG_DST_BIT),

	CT_OPT_ORIG		= (CT_OPT_ORIG_SRC | CT_OPT_ORIG_DST),

	CT_OPT_REPL_SRC_BIT	= 2,
	CT_OPT_REPL_SRC		= (1 << CT_OPT_REPL_SRC_BIT),

	CT_OPT_REPL_DST_BIT	= 3,
	CT_OPT_REPL_DST		= (1 << CT_OPT_REPL_DST_BIT),

	CT_OPT_REPL		= (CT_OPT_REPL_SRC | CT_OPT_REPL_DST),

	CT_OPT_PROTO_BIT	= 4,
	CT_OPT_PROTO		= (1 << CT_OPT_PROTO_BIT),

	CT_OPT_TUPLE_ORIG	= (CT_OPT_ORIG | CT_OPT_PROTO),
	CT_OPT_TUPLE_REPL	= (CT_OPT_REPL | CT_OPT_PROTO),

	CT_OPT_TIMEOUT_BIT	= 5,
	CT_OPT_TIMEOUT		= (1 << CT_OPT_TIMEOUT_BIT),

	CT_OPT_STATUS_BIT	= 6,
	CT_OPT_STATUS		= (1 << CT_OPT_STATUS_BIT),

	CT_OPT_ZERO_BIT		= 7,
	CT_OPT_ZERO		= (1 << CT_OPT_ZERO_BIT),

	CT_OPT_EVENT_MASK_BIT	= 8,
	CT_OPT_EVENT_MASK	= (1 << CT_OPT_EVENT_MASK_BIT),

	CT_OPT_EXP_SRC_BIT	= 9,
	CT_OPT_EXP_SRC		= (1 << CT_OPT_EXP_SRC_BIT),

	CT_OPT_EXP_DST_BIT	= 10,
	CT_OPT_EXP_DST		= (1 << CT_OPT_EXP_DST_BIT),

	CT_OPT_MASK_SRC_BIT	= 11,
	CT_OPT_MASK_SRC		= (1 << CT_OPT_MASK_SRC_BIT),

	CT_OPT_MASK_DST_BIT	= 12,
	CT_OPT_MASK_DST		= (1 << CT_OPT_MASK_DST_BIT),

	CT_OPT_NATRANGE_BIT	= 13,
	CT_OPT_NATRANGE		= (1 << CT_OPT_NATRANGE_BIT),

	CT_OPT_MARK_BIT		= 14,
	CT_OPT_MARK		= (1 << CT_OPT_MARK_BIT),

	CT_OPT_ID_BIT		= 15,
	CT_OPT_ID		= (1 << CT_OPT_ID_BIT),

	CT_OPT_FAMILY_BIT	= 16,
	CT_OPT_FAMILY		= (1 << CT_OPT_FAMILY_BIT),

	CT_OPT_SRC_NAT_BIT	= 17,
	CT_OPT_SRC_NAT		= (1 << CT_OPT_SRC_NAT_BIT),

	CT_OPT_DST_NAT_BIT	= 18,
	CT_OPT_DST_NAT		= (1 << CT_OPT_DST_NAT_BIT),

	CT_OPT_OUTPUT_BIT	= 19,
	CT_OPT_OUTPUT		= (1 << CT_OPT_OUTPUT_BIT),

	CT_OPT_SECMARK_BIT	= 20,
	CT_OPT_SECMARK		= (1 << CT_OPT_SECMARK_BIT),

	CT_OPT_BUFFERSIZE_BIT	= 21,
	CT_OPT_BUFFERSIZE	= (1 << CT_OPT_BUFFERSIZE_BIT),

	CT_OPT_ANY_NAT_BIT	= 22,
	CT_OPT_ANY_NAT		= (1 << CT_OPT_ANY_NAT_BIT),

	CT_OPT_ZONE_BIT		= 23,
	CT_OPT_ZONE		= (1 << CT_OPT_ZONE_BIT),

	CT_OPT_LABEL_BIT	= 24,
	CT_OPT_LABEL		= (1 << CT_OPT_LABEL_BIT),

	CT_OPT_ADD_LABEL_BIT	= 25,
	CT_OPT_ADD_LABEL	= (1 << CT_OPT_ADD_LABEL_BIT),

	CT_OPT_DEL_LABEL_BIT	= 26,
	CT_OPT_DEL_LABEL	= (1 << CT_OPT_DEL_LABEL_BIT),

	CT_OPT_ORIG_ZONE_BIT	= 27,
	CT_OPT_ORIG_ZONE	= (1 << CT_OPT_ORIG_ZONE_BIT),

	CT_OPT_REPL_ZONE_BIT	= 28,
	CT_OPT_REPL_ZONE	= (1 << CT_OPT_REPL_ZONE_BIT),

	CT_OPT_LOG_BIT		= 29,
	CT_OPT_LOG		= (1 << CT_OPT_LOG_BIT),
};
/* If you add a new option, you have to update NUMBER_OF_OPT in conntrack.h */

/* Update this mask to allow to filter based on new options. */
#define CT_COMPARISON (CT_OPT_PROTO | CT_OPT_ORIG | CT_OPT_REPL | \
		       CT_OPT_MARK | CT_OPT_SECMARK |  CT_OPT_STATUS | \
		       CT_OPT_ID | CT_OPT_ZONE | CT_OPT_LABEL | \
		       CT_OPT_ORIG_ZONE | CT_OPT_REPL_ZONE)

enum ct_direction {
	DIR_SRC = 0,
	DIR_DST = 1,
};


typedef enum {
    NFCM_DB_APP = 0,
    NFCM_DB_SUM = 1,
    NFCM_DB_TCP = 2,
    NFCM_DB_MAX = 3
} NFCM_DB_TYPE;

typedef enum {
    PHY_UNKNOWN = 0,
    PHY_ETHER = 1,
    PHY_WIRELESS = 2,
    PHY_MAX = 3
} PHY_TYPE;

typedef struct phy_port_s {
    PHY_TYPE eth_type;
    u_int8_t eth_port;

    // for wl guest network
    bool is_guest;
    char ifname[IFNAMESIZE];
} phy_port_t;

/*
struct sockaddr_in {
      short int sin_family;            // Address family AF_INET
      unsigned short int sin_port;     // Port number
      struct in_addr sin_addr;         // Internet address
      unsigned char sin_zero[8];       // Same size as struct sockaddr
};
struct in_addr {
      unsigned long s_addr;            // Internet address
};

struct sockaddr_in6 {
      sa_family_t sin6_family;         // AF_INET6
      in_port_t sin6_port;             // transport layer port #
      uint32_t sin6_flowinfo;          // IPv6 traffic class & flow info
      struct in6_addr sin6_addr;       // IPv6 address
      uint32_t sin6_scope_id;          // set of interfaces for a scope
};
struct in6_addr {
      uint8_t s6_addr[16];             // IPv6 address
};
*/

extern int nf_conntrack_timer;
extern int nf_conntrack_period;

typedef struct nf_node_s {
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
    uint64_t up_ttl_bytes;
    int64_t  up_speed; // bytes per second

    uint64_t dn_bytes;
    uint64_t dn_dif_bytes;
    uint64_t dn_ttl_bytes;
    int64_t  dn_speed; // bytes per second

    phy_port_t layer1_info;

    time_t timestamp; //first timestamp the node created
    time_t time_in;   // time elasped in the invalid state
    uint8_t state; // dedicated for TCP conntrack

    struct list_head list;
} nf_node_t;

typedef struct lan_info_s {
    char ifname[IFNAMESIZE];
    bool enabled;
    struct in_addr addr;
    struct in_addr subnet;
    struct list_head list;
} lan_info_t;

extern void nf_list_free(struct list_head *list);
extern bool is_acceptable_addr(nf_node_t *nn);
extern void get_date_time(int utc, char *buff, int len);

extern int get_eth_type(char *ethtype, PHY_TYPE etype);

#endif // __NFCM_H__
