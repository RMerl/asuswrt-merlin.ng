#ifndef __VPNC_FUSION_H__
#define __VPNC_FUSION_H__

#include <net/if.h>

#define INTERNET_ROUTE_TABLE_ID	1

#define L2TP_VPNC_PID	"/var/run/l2tpd-vpnc%d.pid"
#define L2TP_VPNC_CTRL	"/var/run/l2tpctrl-vpnc%d"
#define L2TP_VPNC_CONF	"/tmp/l2tp-vpnc%d.conf"

#define SAFE_FREE(x)	if(x) {free(x); x=NULL;}

#define PROTO_PPTP "PPTP"
#define PROTO_L2TP "L2TP"
#define PROTO_OVPN "OpenVPN"
#define PROTO_IPSec "IPSec"
#define PROTO_WG "WireGuard"
#define PROTO_HMA "HMA"
#define PROTO_NORDVPN "NordVPN"
#define PROTO_SURFSHARK "Surfshark"
#define PROTO_CYBERGHOST "CyberGhost"

#define MAX_VPNC_DATA_LEN	256
#define MAX_VPNC_PROFILE	16
#define MAX_DEV_POLICY		64
#define MAX_VPNC_CONN		4

typedef enum{
	VPNC_PROTO_UNDEF,
	VPNC_PROTO_PPTP,
	VPNC_PROTO_L2TP,
	VPNC_PROTO_OVPN,
	VPNC_PROTO_IPSEC,
	VPNC_PROTO_WG,
	VPNC_PROTO_HMA,
	VPNC_PROTO_NORDVPN,
}VPNC_PROTO;

typedef enum{
	VPNC_PPTP_OPT_UNDEF,
	VPNC_PPTP_OPT_AUTO,
	VPNC_PPTP_OPT_MPPC,
	VPNC_PPTP_OPT_MPPE40,
	VPNC_PPTP_OPT_MPPE56,
	VPNC_PPTP_OPT_MPPE128
}VPNC_PPTP_OPT;

typedef struct _vpnc_basic_conf{
	char server[MAX_VPNC_DATA_LEN];
	char username[MAX_VPNC_DATA_LEN];
	char password[MAX_VPNC_DATA_LEN];
}VPNC_BASIC_CONF;

typedef struct _vpnc_pptp{
	VPNC_PPTP_OPT option;
}VPNC_PPTP;

typedef struct _vpnc_ovpn{
	int ovpn_idx; // 1~5
}VPNC_OVPN;

typedef struct _vpnc_wg{
	int wg_idx;
}VPNC_WG;

typedef struct _vpnc_tp{
	int tpvpn_idx;
	char region[48];
	char conntype[8];
}VPNC_TPVPN;

typedef struct _vpnc_ipsec{
	int prof_idx;
}VPNC_IPSEC;

typedef struct _vpnc_profile{
	int active;	//0: inactive, 1:active
	int vpnc_idx;	// 1 ~ MAX_VPNC_PROFILE
	VPNC_PROTO protocol;
	VPNC_BASIC_CONF basic;
	union {
		VPNC_PPTP pptp;
		VPNC_OVPN ovpn;
		VPNC_WG wg;
		VPNC_TPVPN tpvpn;
		VPNC_IPSEC ipsec;
	}config;	
}VPNC_PROFILE;

typedef struct _vpnc_dev_policy{
	int active;	// 0:disable ; 1:enable
	char src_ip[16];	//ip address of client
	char dst_ip[16];	//ip address of destinaction
	int vpnc_idx;	//vpn client index
#ifdef RTCONFIG_VPN_FUSION_SUPPORT_INTERFACE
	char iif[IFNAMSIZ];
#endif
}VPNC_DEV_POLICY;

typedef enum{
	VPNC_ROUTE_ADD,
	VPNC_ROUTE_DEL,	
}VPNC_ROUTE_CMD;

#define VPNC_UNIT_BASIC 5

#define VPNC_PROFILE_VER_OLD	0
#define VPNC_PROFILE_VER1		1

#define VPNC_RULE_PRIORITY	"100"

#define VPNC_RULE_PRIORITY_NETWORK	1000
extern char vpnc_resolv_path[] ;
#ifdef RTCONFIG_VPN_FUSION_SUPPORT_INTERFACE
extern int vpnc_set_iif_routing_rule(const int vpnc_idx, const char* br_ifname);
#endif

#define VPNC_RULE_PRIORITY_DEFAULT		"10000"

#ifdef RTCONFIG_TPVPN
#define TPVPN_PSZ_HMA       "hma"
#define TPVPN_PSZ_NORDVPN   "nordvpn"
#define TPVPN_PSZ_SUFRSHARK PROTO_SURFSHARK
#define TPVPN_FILE_LOCK     "tpvpn"

#define TPVPN_STS_ERROR    -1
#define TPVPN_STS_STOP      0
#define TPVPN_STS_INIT      1
#define TPVPN_STS_DONE      2
#define TPVPN_STS_2FA       3
#define TPVPN_ERRNO_NONE    0
#define TPVPN_ERRNO_AUTH    1
#define TPVPN_ERRNO_SERVER  2
#define TPVPN_ERRNO_KEYFULL 3
#define TPVPN_ERRNO_CONFIG  4
#define TPVPN_ERRNO_NET     5
#endif

#if defined(RTCONFIG_VPN_FUSION) || defined(RTCONFIG_WIREGUARD) || defined(RTCONFIG_NORDVPN) || defined(RTCONFIG_SURFSHARK)
#define WG_SERVER_MAX  2
#define WG_SERVER_CLIENT_MAX   10
#define WG_CLIENT_MAX  5
#define WG_SERVER_IF_PREFIX    "wgs"
#define WG_CLIENT_IF_PREFIX    "wgc"
#define WG_SERVER_NVRAM_PREFIX "wgs"
#define WG_CLIENT_NVRAM_PREFIX "wgc"
#define WG_SERVER_SUBNET6_BASE 0x0110
extern int read_wgc_config_file(const char* file_path, int wgc_unit);
extern int is_wgc_connected(int unit);
#endif

#ifdef RTCONFIG_WIREGUARD
typedef enum wg_type{
        WG_TYPE_SERVER = 0,
        WG_TYPE_CLIENT
}wg_type_t;

enum {
        WG_NF_DEL = 0,
        WG_NF_ADD
};
#if defined(RTCONFIG_HND_ROUTER_AX_6756) || defined(RTCONFIG_BCM_502L07P2) || defined(RTCONFIG_HND_ROUTER_AX_675X)
#define WG_DIR_CONF    "/etc/wg"
typedef enum {
        WG_PORT_DST = 0,
        WG_PORT_SRC,
        WG_PORT_BOTH,
}wg_port_t;
extern void hnd_skip_wg_port(int add, int port, wg_port_t type);
extern void hnd_skip_wg_network(int add, const char* net);
extern void hnd_skip_wg_all_lan(int add);
extern int _wg_check_same_port(wg_type_t type, int unit, int port);
#endif
#endif

#ifdef RTCONFIG_WIREGUARD
extern void reset_wgc_setting(int unit);
#endif

#endif
