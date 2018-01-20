#ifndef _OPENVPN_CONFIG_H
#define _OPENVPN_CONFIG_H

#define OVPN_SERVER_MAX	2
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
	OVPN_CLIENT_CA_EXTRA,
	OVPN_SERVER_CA_EXTRA,
}ovpn_key_t;

typedef enum ovpn_status{
	OVPN_STS_STOP = 0,
	OVPN_STS_INIT,
	OVPN_STS_RUNNING,
	OVPN_STS_STOPPING,
}ovpn_status_t;


#define OVPN_ACCNT_MAX	15


int _set_crt_parsed(const char *name, char *file_path);
extern int set_ovpn_key(ovpn_type_t type, int unit, ovpn_key_t key_type, char *buf, char *path);
extern char *get_ovpn_filename(ovpn_type_t type, int unit, ovpn_key_t key_type, char *buf, size_t buf_len);
extern char *get_ovpn_key(ovpn_type_t type, int unit, ovpn_key_t key_type, char *buf, size_t len);
extern char *get_parsed_crt(const char *name, char *buf, size_t buf_len);
extern void update_ovpn_status(ovpn_type_t type, int unit, ovpn_status_t status_type);
extern void reset_ovpn_setting(ovpn_type_t type, int unit);
extern int ovpn_key_exists(ovpn_type_t type, int unit, ovpn_key_t key_type);
extern int ovpn_crt_is_empty(const char *name);
extern char *get_ovpn_custom(ovpn_type_t type, int unit, char* buffer, int bufferlen);
extern int set_ovpn_custom(ovpn_type_t type, int unit, char* buffer);

#endif
