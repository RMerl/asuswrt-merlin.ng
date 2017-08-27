#ifndef __SHN_DC_H__
#define __SHN_DC_H__

#include <stdint.h>

typedef enum {
	SHN_DC_OK,
	SHN_DC_FAIL_BAD_ARG  = 100,	/* Bad argument */
	SHN_DC_FAIL_NO_PERM,			/* Invalid permission */
	SHN_DC_FAIL_NO_MEM,			/* No memory */
	SHN_DC_FAIL_INTL_ERR,			/* Internal error */
} shn_dc_ret_t;

int shn_dc_set_fw_ver(char *fw_ver);

#if 0
typedef struct {
	/* both in unit of Kbps */
	uint32_t uplink_bw;
	uint32_t downlink_bw;
} bw_info_t;

int shn_dc_set_bw(bw_info_t *bw_info);
#endif

#define SHN_DC_MAX_NR_MAIL	2
int shn_dc_set_email(uint8_t *email[], uint32_t nr_email);

int shn_dc_set_eula(uint8_t eula);

#if 0
typedef enum {
	DPI_CONF_NULL
	, DPI_CONF_APP_ID		/* Application Identification */
	, DPI_CONF_DEV_ID		/* Device Identification */
	, DPI_CONF_VIRT_PATCH		/* Virtual Patch */
	, DPI_CONF_WRS_APP		/* WRS for parental control */
	, DPI_CONF_WRS_CC		/* WRS for C&C detection */
	, DPI_CONF_WRS_SEC		/* WRS for security */
	, DPI_CONF_ANOMALY		/* Anomaly detection */
	, DPI_CONF_QOS			/* QoS */
//	, DPI_CONF_ANTIBOT		/* Anti-Botnet */
	, DPI_CONF_MAX
} dpi_conf_type_t;

typedef struct {
	dpi_conf_type_t type;
	/*
	 * 	0: disable
	 * 	1: enable
	 * 	>= 2: specific for each feature
	 */
	uint32_t value;
	uint64_t ts;	/* internal use */
} dpi_conf_t;

#define SHN_DC_MAX_NR_DPI_CONF	(DPI_CONF_MAX - 1)
int shn_dc_set_dpi_conf(dpi_conf_t dpi_conf[], uint32_t nr_conf);
#endif

#define PROXY_TYPE_SOCK4	(1 << 0)
#define PROXY_TYPE_SOCK5	(1 << 1)
#define PROXY_TYPE_HTTP		(1 << 2)

typedef struct {
	uint8_t type;
	/* NULL-terminated IP or hostname string */
	uint8_t *proxy;
	/* extra data; if no, use NULL */
	uint8_t *extra;
} proxy_info_t;

#define SHN_DC_MAX_NR_PROXY_INFO	2
int shn_dc_set_proxy_info(proxy_info_t proxy_info[], uint32_t nr_info);

#define SHN_DC_MAX_NR_ISP			2
int shn_dc_set_isp(uint8_t* isp[], uint32_t nr_isp);

typedef enum {
	HC_SEC_NULL /* do not use */
	, HC_SEC_BOT_CC	/* Botnet C&C communication detection */
	, HC_SEC_DDOS		/* DDoS defense */
	, HC_SEC_DNS_INT	/* DNS integrity */
	, HC_SEC_ARP_ATT	/* ARP attack */
	, HC_SEC_PORT_SCAN	/* Port scan detection */
	, HC_SEC_MAX /* do not use */
} hc_sec_type_t;

typedef struct {
	hc_sec_type_t type;
	/*
	 * 	0: disable
	 * 	1: enable
	 * 	>= 2: specific for each feature
	 */
	uint32_t value;
} hc_sec_t;

typedef enum {
	HC_SERV_NULL /* do not use */
	, HC_SERV_UPNP
	, HC_SERV_DDNS
	, HC_SERV_MAX /* do not use */
} hc_serv_type_t;

typedef struct {
	hc_serv_type_t type;
	/*
	 * 	0: disable
	 * 	1: enable
	 * 	>= 2: specific for each feature
	 */
	uint32_t value;
} hc_serv_t;

typedef enum {
	HC_CONF_NULL /* do not use */
	, HC_CONF_WPS_PROTECT
	, HC_CONF_WAN_MGMT
	, HC_CONF_MA_CTRL
	, HC_CONF_WAN_PING
	, HC_CONF_LAN_PING
	, HC_CONF_USR_PWD
	, HC_CONF_PWD_STREN
	, HC_CONF_DMZ_IP
	, HC_CONF_PORT_TG
	, HC_CONF_PORT_FW
	, HC_CONF_FTP_LOGIN
	, HC_CONF_SMB_LOGIN
	, HC_CONF_MAX /* do not use */
} hc_conf_type_t;

typedef struct {
	hc_conf_type_t type;
	/* Attribute: any strings */
	char *attribute;
} hc_conf_t;

#define SHN_DC_MAX_NR_HC_SEC	(HC_SEC_MAX - 1)
#define SHN_DC_MAX_NR_HC_SERV	(HC_SERV_MAX - 1)
#define SHN_DC_MAX_NR_HC_CONF	(HC_CONF_MAX - 1)

int shn_dc_set_hc_sec(hc_sec_t sec[], uint32_t nr_sec);
int shn_dc_set_hc_serv(hc_serv_t serv[], uint32_t nr_serv);
int shn_dc_set_hc_conf(hc_conf_t conf[], uint32_t nr_conf);

typedef enum {
	WL_BAND_NULL /* do not use */
	, WL_BAND_2G
	, WL_BAND_5G
	, WL_BAND_MAX /* do not use */
} wl_band_t;

typedef enum {
	WL_ENC_TYPE_OPEN
	, WL_ENC_TYPE_WEP
	, WL_ENC_TYPE_WPAPSK
	, WL_ENC_TYPE_WPA2PSK
	, WL_ENC_TYPE_WPA
	, WL_ENC_TYPE_WPA2
	, WL_ENC_TYPE_WPAAUTOPSK
	, WL_ENC_TYPE_MAX /* do not use */
} wl_enc_type_t;

typedef enum {
	WL_PWD_STREN_NONE
	, WL_PWD_STREN_POOR
	, WL_PWD_STREN_WEAK
	, WL_PWD_STREN_NORM
	, WL_PWD_STREN_GOOD
	, WL_PWD_STREN_NICE
	, WL_PWD_STREN_EXLT
	, WL_PWD_STREN_MAX /* do not use */
} wl_pwd_stren_t;

typedef struct {
	char *if_name; /* key */
	wl_band_t band;
	wl_enc_type_t enc_type;
	wl_pwd_stren_t pwd_stren;
} hc_wl_enc_t;

#define SHN_DC_MAX_NR_HC_WL_ENC	32

int shn_dc_set_hc_wl_enc(hc_wl_enc_t wl_enc[], uint32_t nr_wl_enc);

#endif	// !__SHN_DC_H__
