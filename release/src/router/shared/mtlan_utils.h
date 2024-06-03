#ifndef MTLAN_UTILS_H
#define MTLAN_UTILS_H
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <ctype.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <bcmnvram.h>
#include <shared.h>
#include "shutils.h"

#ifdef RTCONFIG_MAX_MTLAN
#define MTLAN_MAXINUM  (RTCONFIG_MAX_MTLAN+1)
#else  	//RTCONFIG_MAX_MTLAN
#define MTLAN_MAXINUM             17 /* 1 (Default) + 16 */
#endif 	//RTCONFIG_MAX_MTLAN

#define MTLAN_VPNS_MAXINUM        16
#define MTLAN_GRE_MAXINUM         8
#define SDN_LIST_BASIC_PARAM      6
#define SDN_LIST_MAX_PARAM        22
#define VLAN_LIST_BASIC_PARAM     2
#define VLAN_LIST_MAX_PARAM       3
#define SUBNET_LIST_BASIC_PARAM   13
#define SUBNET_LIST_MAX_PARAM     21
#define CPLAN_MAXINUM             4 

enum {
	MTLAN_IFUNIT_BASE=50,
	MTLAN_IFUNIT_START=51,
	MTLAN_IFUNIT_END=58,
	MTLAN_IFUNIT_MAX
};

typedef enum {
	SDNFT_TYPE_SDN,
	SDNFT_TYPE_APG,
	SDNFT_TYPE_VPNC,
	SDNFT_TYPE_VPNS,
	SDNFT_TYPE_URLF,
	SDNFT_TYPE_NWF,
	SDNFT_TYPE_GRE,
	SDNFT_TYPE_FW,
	SDNFT_TYPE_KILLSW,
	SDNFT_TYPE_AHS,
	SDNFT_TYPE_WAN,
	SDNFT_TYPE_PPPRELAY,
	SDNFT_TYPE_WAN6,
	SDNFT_TYPE_MTWAN,
	SDNFT_TYPE_MSWAN,
	SDNFT_TYPE_MAX
} SDNFT_TYPE;

typedef struct __vlan_rl_t__
{
	int idx;
	int vid;
	int port_isolation;
} VLAN_T;

typedef struct __subnet_rl_t__
{
	int idx;
	char ifname[IFNAMSIZ];
	char br_ifname[IFNAMSIZ];
	char addr[INET_ADDRSTRLEN];
	char netmask[INET_ADDRSTRLEN];
	char subnet[INET_ADDRSTRLEN];
	unsigned int prefixlen;
	unsigned int dhcp_enable;
	char dhcp_min[INET_ADDRSTRLEN];
	char dhcp_max[INET_ADDRSTRLEN];
	unsigned int dhcp_lease;
	char domain_name[64];
	char dns[2][INET_ADDRSTRLEN];
	char wins[64];
	unsigned int dhcp_res;
	unsigned int dhcp_res_idx;
	unsigned int dot_enable;
	unsigned int dot_tls;
	/* ipv6 */
	unsigned int v6_enable;
	unsigned int v6_autoconf;
	char addr6[INET6_ADDRSTRLEN];
	char dhcp6_min[INET6_ADDRSTRLEN];
	char dhcp6_max[INET6_ADDRSTRLEN];
	char dns6[3][INET6_ADDRSTRLEN];
} SUBNET_T;

typedef struct __sdn_feature_t__
{
	int sdn_idx;
	int apg_idx;
	int vpnc_idx;
	int vpns_idx_rl[MTLAN_VPNS_MAXINUM];
	int dnsf_idx; /* DNS Filter */
	int urlf_idx; /* URL Filter */
	int nwf_idx;  /* Netwrok Service Filter */
	int cp_idx;   /* Captive Portal */
	int gre_idx_rl[MTLAN_GRE_MAXINUM];   /* GRE Keepalive */
	int fw_idx;  /* Firewall */
	int killsw_sw; /* Kill Switch */
	int ahs_sw;  /* Access Host Service Switch */
	int wan_idx;
	int ppprelay_sw; /* PPPoE Relay Switch */
	int wan6_idx;
	int mtwan_idx; /* Multiple Wan */
	int mswan_idx; /* Multi-Service Wan (bridge) */
} SDNFT_T;


typedef struct __multiple_lan_t__
{
	int enable;
	int vid;
	int port_isolation;
	char name[64];
	char createby[8];
	SUBNET_T nw_t;
	SDNFT_T sdn_t;
} MTLAN_T;

typedef enum {
	VPN_TYPE_SERVER = 0,
	VPN_TYPE_CLIENT
} VPN_TYPE_T;

typedef enum{
	VPN_PROTO_PPTP,
	VPN_PROTO_L2TP,
	VPN_PROTO_OVPN,
	VPN_PROTO_IPSEC,
	VPN_PROTO_WG,
	VPN_PROTO_L2GRE,
	VPN_PROTO_L3GRE,
	VPN_PROTO_UNKNOWN
}VPN_PROTO_T;

typedef struct __vpn_vpnx__
{
	// VPN_TYPE_T type;
	VPN_PROTO_T proto;
	int unit;
} VPN_VPNX_T;

typedef struct __cp_type_rl__
{
	int cp_idx;
	int cp_type;
} CP_TYPE_RL;

typedef struct __cp_profile__
{
	int cp_enable;
	int cp_auth_type;
	int cp_conntimeout;
	int cp_idletimeout;
	int cp_authtimeout;
	char redirecturl[128];
	int cp_term_service;
	int cp_bw_limit_ul;
	int cp_bw_limit_dl;
	char cp_nas_id[128];
	char cp_idx_ui_customizes[32];
} CP_PROFILE;

typedef struct __cp_local_auth__
{
	char passcode[1024];
} CP_LOCAL_AUTH;

typedef struct __cp_radius_profile__
{
	int radius_idx;
} CP_RADIUS_PROFILE;

typedef struct __cp_radius_list__
{
 	int idx;
    char authipaddr1[64];
    int authport1;
    char authkey1[64];
    char acct_ipaddr1[64];
    int acct_port1;
    char acct_key1[64];
    char authipaddr2[64];
    int authport2;
    char authkey2[64];
    char acct_ipaddr2[64];
    int acct_port2;
    char acct_key2[64];
} CP_RADIUS_LIST;

#ifdef RTCONFIG_VPN_FUSION
#define VPN_PROTO_PPTP_STR 		PROTO_PPTP
#define VPN_PROTO_L2TP_STR		PROTO_L2TP
#define VPN_PROTO_OVPN_STR		PROTO_OVPN
#define VPN_PROTO_IPSEC_STR		PROTO_IPSec
#define VPN_PROTO_WG_STR		PROTO_WG
#define VPN_PROTO_HMA_STR		PROTO_HMA
#define VPN_PROTO_NORDVPN_STR	PROTO_NORDVPN
#define VPN_PROTO_SURFSHARK_STR	PROTO_SURFSHARK
#define VPN_PROTO_CYBERGHOST_STR	PROTO_CYBERGHOST
#else
#define VPN_PROTO_PPTP_STR "PPTP"
#define VPN_PROTO_L2TP_STR "L2TP"
#define VPN_PROTO_OVPN_STR "OpenVPN"
#define VPN_PROTO_IPSEC_STR "IPSec"
#define VPN_PROTO_WG_STR "WireGuard"
#define VPN_PROTO_HMA_STR "HMA"
#define VPN_PROTO_NORDVPN_STR "NordVPN"
#define VPN_PROTO_SURFSHARK_STR "Surfshark"
#define VPN_PROTO_CYBERGHOST_STR "CyberGhost"
#endif
#define VPN_PROTO_L2GRE_STR "L2GRE"
#define VPN_PROTO_L3GRE_STR "L3GRE"

extern void *INIT_MTLAN(const unsigned int sz);
extern void FREE_MTLAN(void *p);

extern MTLAN_T  *get_mtlan(MTLAN_T *nwlst, size_t *lst_sz);
extern SUBNET_T *get_mtsubnet(SUBNET_T *sublst, size_t *lst_sz, const int is_rm);
extern VLAN_T   *get_mtvlan(VLAN_T *vlst, size_t *sz, const int is_rm);
extern MTLAN_T  *get_mtlan_by_vid(const unsigned int vid, MTLAN_T *nwlst, size_t *lst_ze);
extern MTLAN_T  *get_mtlan_by_idx(SDNFT_TYPE type, const unsigned int idx, MTLAN_T *nwlst, size_t *lst_sz);
extern MTLAN_T  *get_rm_mtlan(MTLAN_T *nwlst, size_t *lst_sz);
extern MTLAN_T  *get_rm_mtlan_by_vid(const unsigned int vid, MTLAN_T *nwlst, size_t *lst_ze);
extern MTLAN_T  *get_rm_mtlan_by_idx(SDNFT_TYPE type, const unsigned int idx, MTLAN_T *nwlst, size_t *lst_sz);

extern size_t get_mtlan_cnt();
extern size_t get_rm_mtlan_cnt();
extern int sdn_enable(void);

extern int get_vpns_idx_by_proto_unit(VPN_PROTO_T proto, int unit);
extern int get_vpnc_idx_by_proto_unit(VPN_PROTO_T proto, int unit);
extern int get_gre_idx_by_proto_unit(VPN_PROTO_T proto, int unit);
extern char* get_vpns_ifname_by_vpns_idx(int vpns_idx, char* buf, size_t len);
extern char* get_vpns_iprange_by_vpns_idx(int vpns_idx, char* buf, size_t len);
extern VPN_VPNX_T* get_vpnx_by_vpnc_idx(VPN_VPNX_T* vpnx, int vpnc_idx);
extern VPN_VPNX_T* get_vpnx_by_vpns_idx(VPN_VPNX_T* vpnx, int vpns_idx);
extern VPN_VPNX_T* get_vpnx_by_gre_idx(VPN_VPNX_T* vpnx, int gre_idx);
extern int mtlan_extend_prefix_by_subnet_idx(const char* prefix, int prefix_length
				, uint8_t subnet_idx, int subnet_length, char* buf, size_t len);
#endif
