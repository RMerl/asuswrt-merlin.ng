#ifndef _OPENVPN_CONFIG_H
#define _OPENVPN_CONFIG_H

#include "rtconfig.h"

#define OVPN_SERVER_MAX	1
#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2) || defined(RTCONFIG_UBIFS)
#define OVPN_CLIENT_MAX	5
#else
#define OVPN_CLIENT_MAX	1
#endif

#define OVPN_CCD_MAX	16

#define OVPN_CLIENT_BASE 10
#define OVPN_SERVER_BASE 20

#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2) || defined(RTCONFIG_UBIFS)
#define OVPN_DIR_SAVE	"/jffs/openvpn"
#endif

typedef enum ovpn_type{
	OVPN_TYPE_SERVER = 0,
	OVPN_TYPE_CLIENT
}ovpn_type_t;


typedef enum ovpn_key {
	OVPN_CLIENT_STATIC = 0,
	OVPN_CLIENT_CA,
	OVPN_CLIENT_CERT,
	OVPN_CLIENT_KEY,
	OVPN_CLIENT_CRL,
	OVPN_SERVER_STATIC,
	OVPN_SERVER_CA,
	OVPN_SERVER_CA_KEY,
	OVPN_SERVER_CERT,
	OVPN_SERVER_KEY,
	OVPN_SERVER_DH,
	OVPN_SERVER_CRL,
	OVPN_SERVER_CLIENT_CERT,
	OVPN_SERVER_CLIENT_KEY,
}ovpn_key_t;


typedef enum ovpn_if{
	OVPN_IF_TUN = 0,
	OVPN_IF_TAP
}ovpn_if_t;


typedef enum ovpn_auth{
	OVPN_AUTH_STATIC = 0,
	OVPN_AUTH_TLS
}ovpn_auth_t;


typedef struct ovpn_ccd_val {
	int enable;
	char name[68];
	char network[16];
	char netmask[16];
	int push;
}ovpn_ccd_val_t;


typedef struct ovpn_ccd_info {
	ovpn_ccd_val_t ccd_val[OVPN_CCD_MAX];
	int count;
}ovpn_ccd_info_t;


typedef struct ovpn_sconf_common {
	int enable[OVPN_SERVER_MAX];
	int dns[OVPN_SERVER_MAX];
}ovpn_sconf_common_t;


typedef struct ovpn_sconf {
	int enable;
// Tunnel options
	char proto[8];
	int port;
	ovpn_if_t if_type;
	char if_name[8];	//interface name
	char local[16];
	char remote[16];
	int verb;	//verbosity
	char comp[16];	//LZO compression: "yes", "no", or "adaptive"
	ovpn_auth_t auth_mode;	//authentication mode: static, tls
	int useronly;	//client certificte not required

//Server mode
	char network[16];
	char netmask[16];
	int dhcp;	//DHCP-proxy mode
	char pool_start[16];	//--server-bridge gateway netmask pool-start-IP pool-end-IP
	char pool_end[16];
	int redirect_gateway;
	int push_lan;
	int push_dns;
	int ccd;	//client config dir
	int c2c;	//client to client
	int ccd_excl;	//ccd-exclusive
	ovpn_ccd_info_t ccd_info;

//Data Channel Encryption Options:
	int direction;	//key-direction of secret or tls-auth (hmac)
	char digest[32]; //HMAC message digest algorithm: e.g. SHA1, RSA-SHA512, ecdsa-with-SHA1
	char cipher[32];	//cipher algorithm: e.g. AES-128-CBC, CAMELLIA-256-CBC

//TLS Mode Options:
	int reneg;	//TLS Renegotiation Time

	int tls_keysize;	//auto generation

//Router options and info
	char firewall[8];	//auto
	int poll;	//polling interval of cron job in seconds
	char lan_ipaddr[16];
	char lan_netmask[16];

	char custom[4096];
}ovpn_sconf_t;


typedef struct ovpn_cconf_common {
	int enable[OVPN_CLIENT_MAX];
}ovpn_cconf_common_t;


typedef struct ovpn_cconf {
	int enable;
// Tunnel options
	char addr[128];	//remote server address
	int retry;	//retry resolve hostname
	char proto[8];
	int port;
	ovpn_if_t if_type;
	char if_name[8];	//interface name
	char local[16];
	char remote[16];
	char netmask[16];
	int redirect_gateway;
	char gateway[16];
	int verb;	//verbosity
	char comp[16];	//LZO compression: "yes", "no", or "adaptive"
	ovpn_auth_t auth_mode;	//authentication mode: static, tls
	int userauth;	//username, password
	int useronly;	//client certificte not required
	char username[64];
	char password[64];

//Data Channel Encryption Options:
	int direction;	//key-direction of secret or tls-auth (hmac)
	char digest[32]; //HMAC message digest algorithm: e.g. SHA1, RSA-SHA512, ecdsa-with-SHA1
	char cipher[32];	//cipher algorithm: e.g. AES-128-CBC, CAMELLIA-256-CBC

//TLS Mode Options:
	int reneg;	//TLS Renegotiation Time
	int tls_remote;	//(DEPRECATED)
	char common_name[32];

//Router options and info
	char firewall[8];	//auto
	int poll;	//polling interval of cron job in seconds
	int bridge;
	int nat;
	int adns;

	char custom[4096];
}ovpn_cconf_t;


typedef enum ovpn_status{
	OVPN_STS_STOP = 0,
	OVPN_STS_INIT,
	OVPN_STS_RUNNING,
	OVPN_STS_STOPPING,
}ovpn_status_t;


#define OVPN_ACCNT_MAX	15
typedef struct ovpn_accnt
{
	char username[128];
	char password[128];
} ovpn_accnt_t;


typedef struct ovpn_accnt_info
{
	ovpn_accnt_t account[OVPN_ACCNT_MAX];
	int count;
} ovpn_accnt_info_t;


extern ovpn_sconf_common_t* get_ovpn_sconf_common(ovpn_sconf_common_t* conf);
extern ovpn_cconf_common_t* get_ovpn_cconf_common(ovpn_cconf_common_t* conf);
extern ovpn_sconf_t* get_ovpn_sconf(int unit, ovpn_sconf_t* conf);
extern ovpn_cconf_t* get_ovpn_cconf(int unit, ovpn_cconf_t* conf);

extern char* get_ovpn_key(ovpn_type_t type, int unit, ovpn_key_t key_type, char *buf, size_t len);
extern int set_ovpn_key(ovpn_type_t type, int unit, ovpn_key_t key_type, char *buf, char *path);
extern int ovpn_key_exists(ovpn_type_t type, int unit, ovpn_key_t key_type);

extern char* get_lan_cidr(char* buf, size_t len);
extern char* get_ovpn_sconf_remote(char* buf, size_t len);
extern void update_ovpn_status(ovpn_type_t type, int unit, ovpn_status_t status_type);
extern void wait_time_sync(int max);

extern ovpn_accnt_info_t* get_ovpn_accnt(ovpn_accnt_info_t *accnt_info);

extern void reset_ovpn_setting(ovpn_type_t type, int unit);
#endif
