#ifndef __WEBAPI_H__
#define __WEBAPI_H__

#include <json.h>
#ifdef RTCONFIG_CFGSYNC
#include <cfg_capability.h>
#endif

enum {
	HTTP_OK = 200,
	HTTP_FAIL = 400,
	HTTP_CHPASS_FAIL,
	HTTP_CHPASS_FAIL_MAX,
	HTTP_AUTH_EXPIRE,
	HTTP_RULE_ADD_SUCCESS = 2001,
	HTTP_RULE_DEL_SUCCESS,
	HTTP_NORULE_DEL,
	HTTP_RULE_MODIFY_SUCCESS,
	HTTP_OVER_MAX_RULE_LIMIT = 4000,
	HTTP_INVALID_ACTION,
	HTTP_INVALID_MAC,
	HTTP_INVALID_ENABLE_OPT,
	HTTP_INVALID_NAME,
	HTTP_INVALID_EMAIL,
	HTTP_INVALID_INPUT,
	HTTP_INVALID_IPADDR,
	HTTP_INVALID_TS,
	HTTP_INVALID_FILE,
	HTTP_INVALID_SUPPORT,
	HTTP_REMOTE_CTRL_DISABLE,
	HTTP_NO_CHANGE,
	ASUSAPI_NOT_SUPPORT,
	HTTP_SHMGET_FAIL = 5000,
	HTTP_FB_SVR_FAIL,
	HTTP_DM_SVR_FAIL
};

struct RWD_MAPPING_TABLE {
        char *name;
        char *path;
        char *white_theme;
};

struct JFFS_BACKUP_PROFILE_S {
        char *name;
        char *folder;
        char *include;
        char *exclude;
        char *rc_service;
        int sync_flag; //0:nvram 1:openvpn 2:ipsec 3:usericon
};

extern struct JFFS_BACKUP_PROFILE_S jffs_backup_profile_t[];
#define JFFS_CFGS_EXCLUDE "/jffs/exclude_lists"

#define PROFILE_HEADER  "HDR1"
#ifdef RTCONFIG_DSL
#define PROFILE_HEADER_NEW      "N55U"
#else
#ifdef RTCONFIG_QCA
#define PROFILE_HEADER_NEW      "AC55U"
#elif defined(RTCONFIG_LANTIQ)
#define PROFILE_HEADER_NEW      "BLUE"
#else
#define PROFILE_HEADER_NEW      "HDR2"
#endif /* RTCONFIG_QCA */
#endif /* RTCONFIG_DSL */

#ifdef RTCONFIG_OPENVPN
#define VPN_CLIENT_UPLOAD	"/tmp/openvpn_file"
#define JFFS_OPENVPN		"/jffs/openvpn/"
#define OPENVPN_UPLOAD_FLODER	"/tmp/server_ovpn_file"
#define OPENVPN_UPLOAD_FILE	"/tmp/server_ovpn_file/server_ovpn.tgz"
#define OPENVPN_EXPORT_FILE	"/tmp/server_ovpn.cert"

extern int upload_server_ovpn_cert_cgi();
extern int gen_server_ovpn_file();
#endif /* RTCONFIG_OPENVPN */

#ifdef RTCONFIG_IPSEC
#define IPSEC_UPLOAD_FLODER     "/tmp/server_ipsec_file"
#define JFFS_CA_FILES           "/jffs/ca_files/"
#define IPSEC_UPLOAD_FILE       "/tmp/server_ipsec_file/server_ipsec.tgz"
#define IPSEC_EXPORT_FILE	"/tmp/server_ipsec.cert"

extern int upload_server_ipsec_cert_cgi();
extern int gen_server_ipsec_file();
#endif /* RTCONFIG_IPSEC */

#ifdef RTCONFIG_AMAS_CENTRAL_CONTROL
#define CFG_CNTRL_EXPORT_FILE	"/tmp/cfg_cntrl.bak"
#endif

extern struct nvram_tuple router_defaults[];
#define BLACKLIST_CONFIG_FILE "/tmp/blacklist_config.json"
#define SAVE_CONFIG_SYNC_FILE "/tmp/save_config_sync.json"
extern int upload_blacklist_config_cgi(char *blacklist_config);
extern int upload_config_sync_cgi();
extern int start_config_sync_cgi();
extern int get_ui_support_info(struct json_object *ui_support_obj);
extern void httpd_nvram_commit(void);
extern int get_nvram_dlen(char *name);
extern int is_port_in_use(int port);
//#ifdef RTCONFIG_WIREGUARD
extern int set_wireguard_server(struct json_object *wireguard_server_obj, int *wgsc_idx);
extern int set_wireguard_client(struct json_object *wireguard_client_obj, int *wgc_idx);
//#endif
extern int get_wl_nband_list();
extern char *wl_nband_to_wlx(char *nv_name, char *wl_name, size_t len);
extern int gen_jffs_backup_profile(char *name, char *file_path);
extern int upload_jffs_profile(char *name, int do_rc);
extern int get_rwd_table(struct json_object *rwd_mapping);
extern int enable_wireguard_client(int wgc_index, char *vpnc_enable);
extern int delete_wireguard_client(int wgc_index);
extern int get_wgc_connect_status(struct json_object *wgc_connect_status_obj);
extern int del_wgsc_list(int s_unit, int c_unit);
extern int get_wgsc_list(int s_unit, struct json_object *wgsc_list_array);
extern int get_ASUS_privacy_policy_tbl(struct json_object *ASUS_privacy_policy_tbl);
extern int get_ASUS_privacy_policy_info(struct json_object *ASUS_privacy_policy_info);
extern int set_ASUS_NEW_EULA(char *ASUS_NEW_EULA, char *from_service);
extern int set_ASUS_privacy_policy(char *ASUS_privacy_policy, char *force_version, char *from_service);
extern int set_app_mnt(char *app_mnt);
extern int get_app_mnt(struct json_object *app_mnt_obj);
#ifdef RTCONFIG_CFGSYNC
#define CFG_SERVER_PID		"/var/run/cfg_server.pid"
extern int is_cfg_server_ready();
extern void notify_cfg_server(json_object *cfg_root, int check);
extern int check_cfg_changed(json_object *root);
extern int save_changed_param(json_object *cfg_root, char *param);
#endif	/* RTCONFIG_CFGSYNC */
extern void reset_accpw();
extern int b64_decode(const char *str, unsigned char *space, int size);
extern int do_chpass(char *cur_username, char *cur_passwd, char *new_username, char *new_passwd, char *restart_httpd, char *defpass_enable, int from_service_id);
extern int is_noFwManual(void);
extern int check_cmd_injection_blacklist(char *para);
extern int check_xss_blacklist(char* para, int check_www);
extern int validate_apply_input_value(char *name, char *value);
extern int detect_vul_scan(void);
#endif /* !__WEBAPI_H__ */
