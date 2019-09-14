#ifndef __VPNC_FUSION_H__
#define __VPNC_FUSION_H__

#define USE_MULTIPATH_ROUTE_TABLE	1
//#define USE_IPTABLE_ROUTE_TARGE		1

#ifdef USE_MULTIPATH_ROUTE_TABLE
#define INTERNET_ROUTE_TABLE_ID	1
#endif

#define L2TP_VPNC_PID	"/var/run/l2tpd-vpnc%d.pid"
#define L2TP_VPNC_CTRL	"/var/run/l2tpctrl-vpnc%d"
#define L2TP_VPNC_CONF	"/tmp/l2tp-vpnc%d.conf"

#define SAFE_FREE(x)	if(x) {free(x); x=NULL;}

#define PROTO_PPTP "PPTP"
#define PROTO_L2TP "L2TP"
#define PROTO_OVPN "OpenVPN"
#define PROTO_IPSec "IPSec"

#define MAX_VPNC_DATA_LEN	68
#define MAX_VPNC_PROFILE	16
#define MAX_DEV_POLICY		64
#define MAX_VPNC_CONN		4

typedef enum{
	VPNC_PROTO_UNDEF,
	VPNC_PROTO_PPTP,
	VPNC_PROTO_L2TP,
	VPNC_PROTO_OVPN,
	VPNC_PROTO_IPSEC
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

typedef struct _vpnc_profile{
	int active;	//0: inactive, 1:active
	int vpnc_idx;	// 1 ~ MAX_VPNC_PROFILE
	VPNC_PROTO protocol;
	VPNC_BASIC_CONF basic;
	union {
		VPNC_PPTP pptp;
		VPNC_OVPN ovpn;
	}config;	
}VPNC_PROFILE;

typedef struct _vpnc_dev_policy{
	int active;	// 0:disable ; 1:enable
#ifdef USE_IPTABLE_ROUTE_TARGE
	char mac[20];	//mac address of client
#else
	char src_ip[16];	//ip address of client
#endif
	char dst_ip[16];	//ip address of destinaction
	int vpnc_idx;	//vpn client index
}VPNC_DEV_POLICY;

#ifdef USE_MULTIPATH_ROUTE_TABLE
typedef enum{
	VPNC_ROUTE_ADD,
	VPNC_ROUTE_DEL,	
}VPNC_ROUTE_CMD;
#endif

#define VPNC_UNIT_BASIC 5

#define VPNC_PROFILE_VER_OLD	0
#define VPNC_PROFILE_VER1		1

#define VPNC_RULE_PRIORITY	"100"
#define VPNC_RULE_PRIORITY_DEFAULT		"10000"

#endif
