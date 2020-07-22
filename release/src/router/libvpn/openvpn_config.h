#include <syslog.h>

#ifndef _OPENVPN_CONFIG_H
#define _OPENVPN_CONFIG_H

#define OVPN_SERVER_MAX	2

#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2) || defined(RTCONFIG_UBIFS)
#define OVPN_CLIENT_MAX	5
#else
#define OVPN_CLIENT_MAX	1
#endif

#define OVPN_CCD_MAX	16

#define PUSH_LAN_METRIC 500

// interfaces
#define OVPN_CLIENT_BASEIF 10
#define OVPN_SERVER_BASEIF 20

#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2) || defined(RTCONFIG_UBIFS)
#define OVPN_DIR_SAVE	"/jffs/openvpn"
#endif

// Line number as text string
#define __LINE_T__ __LINE_T_(__LINE__)
#define __LINE_T_(x) __LINE_T(x)
#define __LINE_T(x) # x

#define VPN_LOG_ERROR -1
#define VPN_LOG_NOTE 0
#define VPN_LOG_INFO 1
#define VPN_LOG_EXTRA 2

// client_access
#define OVPN_CLT_ACCESS_LAN 0
#define OVPN_CLT_ACCESS_WAN 1
#define OVPN_CLT_ACCESS_BOTH 2


#define PEM_START_TAG "-----BEGIN"

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
	OVPN_CLIENT_EXTRA,
	OVPN_SERVER_EXTRA,
}ovpn_key_t;

typedef enum ovpn_status{
	OVPN_STS_ERROR = -1,
	OVPN_STS_STOP = 0,
	OVPN_STS_INIT,
	OVPN_STS_RUNNING,
	OVPN_STS_STOPPING,
}ovpn_status_t;

// OpenVPN routing policy modes (rgw)
enum {
	OVPN_RGW_NONE = 0,
	OVPN_RGW_ALL,
	OVPN_RGW_POLICY,
	OVPN_RGW_POLICY_STRICT
};

enum {
	OVPN_DNSMODE_IGNORE = 0,
	OVPN_DNSMODE_ALLOW,
	OVPN_DNSMODE_STRICT,
	OVPN_DNSMODE_EXCLUSIVE
};

typedef enum ovpn_errno{
	OVPN_ERRNO_NONE = 0,
	OVPN_ERRNO_IP,
	OVPN_ERRNO_ROUTE,
	OVPN_ERRNO_SSL = 4,
	OVPN_ERRNO_DH,
	OVPN_ERRNO_AUTH,
	OVPN_ERRNO_CONF,
	OVPN_ERRNO_NET_CONN
}ovpn_errno_t;

#define OVPN_ACCNT_MAX	15

typedef enum ovpn_if{
	OVPN_IF_TUN = 0,
	OVPN_IF_TAP
}ovpn_if_t;


typedef enum ovpn_auth{
	OVPN_AUTH_STATIC = 0,
	OVPN_AUTH_TLS
}ovpn_auth_t;

typedef struct ovpn_cconf {
	int enable;
	char progname[16];
// Tunnel options
	char addr[128];	//remote server address
	int did_resolv_addr;
	char resolv_addr[1024];
	int retry;	//retry resolve hostname
	char proto[16];
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
	int ncp;
	char ncp_ciphers[256];
	char cipher[32];	//cipher algorithm: e.g. AES-128-CBC, CAMELLIA-256-CBC

//TLS Mode Options:
	int reneg;	//TLS Renegotiation Time
	int tlscrypt;	//Encrypt and authenticate all control channel packets.
	int verify_x509_type;	//TYPE of verify-x509-name
	char verify_x509_name[32];	//NAME of verify-x509-name

//Router options and info
//	char firewall[8];	//auto
	int fw;		//Block inbound connections
	int poll;	//polling interval of cron job in seconds
	int bridge;
	int nat;
	int adns;

	char custom[4096];
}ovpn_cconf_t;

typedef struct ovpn_sconf {
	int enable;
	char progname[16];
// Tunnel options
	char proto[16];
	int port;
	ovpn_if_t if_type;
	char if_name[8];	//interface name
	char local[16];
	char remote[16];
	int verb;	//verbosity
	char comp[16];	//LZO compression: "yes", "no", or "adaptive"
	ovpn_auth_t auth_mode;	//authentication mode: static, tls
	int useronly;	//client certificte not required
	int userauth;

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
	char ccd_val[2048];
//	ovpn_ccd_info_t ccd_info;

//Data Channel Encryption Options:
	int direction;	//key-direction of secret or tls-auth (hmac)
	int tlscrypt;	//Encrypt and authenticate all control channel packets.
	char digest[32]; //HMAC message digest algorithm: e.g. SHA1, RSA-SHA512, ecdsa-with-SHA1
	int ncp;
	char ncp_ciphers[256];
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


int _set_crt_parsed(const char *name, char *file_path);
extern int set_ovpn_key(ovpn_type_t type, int unit, ovpn_key_t key_type, char *buf, char *path);
extern char *get_ovpn_filename(ovpn_type_t type, int unit, ovpn_key_t key_type, char *buf, size_t buf_len);
extern char *get_ovpn_key(ovpn_type_t type, int unit, ovpn_key_t key_type, char *buf, size_t len);
extern char *get_parsed_crt(const char *name, char *buf, size_t buf_len);
extern void update_ovpn_status(ovpn_type_t type, int unit, ovpn_status_t status_type, ovpn_errno_t err_no);
extern ovpn_status_t get_ovpn_status(ovpn_type_t type, int unit);
extern ovpn_errno_t get_ovpn_errno(ovpn_type_t type, int unit);
extern void reset_ovpn_setting(ovpn_type_t type, int unit, int full);
extern int ovpn_key_exists(ovpn_type_t type, int unit, ovpn_key_t key_type);
extern int ovpn_crt_is_empty(const char *name);
extern char *get_ovpn_custom(ovpn_type_t type, int unit, char* buffer, int bufferlen);
extern int set_ovpn_custom(ovpn_type_t type, int unit, char* buffer);
extern int get_max_dnsmode(void);
extern void write_ovpn_resolv_dnsmasq(FILE* dnsmasq_conf);
extern void write_ovpn_dnsmasq_config(FILE* dnsmasq_conf);
extern char *get_ovpn_remote_address(char *buf, int len);
extern void update_ovpn_profie_remote(void);
extern int ovpn_write_key(ovpn_type_t type, int unit, ovpn_key_t key_type);
extern int ovpn_setup_iface(char *iface, ovpn_if_t iface_type, int bridge);
extern void ovpn_remove_iface(ovpn_type_t type, int unit);
extern char *ovpn_get_runtime_filename(ovpn_type_t type, int unit, ovpn_key_t key_type, char *buffer, int len);
extern void ovpn_setup_dirs(ovpn_type_t type, int unit);
extern ovpn_cconf_t *ovpn_get_cconf(int unit);
extern ovpn_sconf_t *ovpn_get_sconf(int unit);
extern int ovpn_write_server_config(ovpn_sconf_t *sconf, int unit);
extern int ovpn_write_client_config(ovpn_cconf_t *cconf, int unit);
extern void ovpn_write_client_keys(ovpn_cconf_t *cconf, int unit);
extern void ovpn_write_server_keys(ovpn_sconf_t *sconf, int unit, int valid_client_cert);
extern void ovpn_setup_client_fw(ovpn_cconf_t *cconf, int unit);
extern void ovpn_setup_server_fw(ovpn_sconf_t *sconf, int unit);
extern void ovpn_setup_server_watchdog(ovpn_sconf_t *sconf, int unit);
#endif
