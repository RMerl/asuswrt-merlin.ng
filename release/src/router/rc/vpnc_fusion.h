#ifndef __VPNC_FUSION_H__
#define __VPNC_FUSION_H__

#include <net/if.h>

#define L2TP_VPNC_PID	"/var/run/l2tpd-vpnc%d.pid"
#define L2TP_VPNC_CTRL	"/var/run/l2tpctrl-vpnc%d"
#define L2TP_VPNC_CONF	"/tmp/l2tp-vpnc%d.conf"

#define PROTO_PPTP "PPTP"
#define PROTO_L2TP "L2TP"
#define PROTO_OVPN "OpenVPN"
#define PROTO_IPSec "IPSec"
#define PROTO_WG "WireGuard"
#define PROTO_HMA "HMA"
#define PROTO_NORDVPN "NordVPN"

#define MAX_VPNC_DATA_LEN	68
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

extern char vpnc_resolv_path[] ;
#ifdef RTCONFIG_VPN_FUSION_SUPPORT_INTERFACE
extern int vpnc_set_iif_routing_rule(const int vpnc_idx, const char* br_ifname);
#endif
#ifdef RTCONFIG_MULTILAN_CFG
extern int vpnc_set_iptables_rule_by_sdn(MTLAN_T *pmtl, size_t mtl_sz, int restart_all_sdn);
#endif

VPNC_PROTO vpnc_get_proto_in_profile_by_vpnc_id(const int vpnc_id);

int vpnc_set_policy_by_ifname(const char *vpnc_ifname, const int action);

int update_default_routing_rule();
int clean_routing_rule_by_vpnc_idx(const int vpnc_idx);
int clean_vpnc_setting_value(const int vpnc_idx);
#endif
