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
#define vpnlog(level,x...) if(nvram_get_int("vpn_debug")>=level) syslog(LOG_INFO, #level ": " __LINE_T__ ": " x)

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
	OVPN_ERRNO_NET_CONN,
}ovpn_errno_t;

#define OVPN_ACCNT_MAX	15


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
#endif
