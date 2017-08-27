#ifndef _NAT_NVRAM_H
#define _NAT_NVRAM_H
#define AAE_DEVID	"aae_deviceid"
#define AAE_ENABLE	"aae_enable"
#define AAE_STATUS	"aae_status"
#define AAE_USERNAME	"aae_username"
#define AAE_PWD		"aae_password"
#define AAE_SDK_LOG_LEVEL "aae_sdk_log_level"
#define ROUTER_MAC	"lan_hwaddr"
#define LINK_INTERNET 	"link_internet"
#define WAN_ACCESS "misc_http_x"
#define DDNS_HOSTNAME "ddns_hostname_x"
/*#define HTTPS_WAN_PORT "misc_httpsport_x"
#define HTTP_WAN_PORT "misc_httpport_x"
#define HTTP_ENABLE "http_enable"*/
//#define AAE_SEM_NAME "AAE_ENABLE_SEM"
#define NV_DBG 1
int nvram_is_aae_enable();
int nvram_set_aae_enable(const char* aae_enable);
int nvram_set_aae_status(const char* aae_status );
int nvram_get_aae_pwd(char** aae_pwd);
int nvram_get_aae_username(char** aae_username);
int nvram_set_aae_info(const char* deviceid);
int nvram_get_aae_sdk_log_level();
int nvram_set_aae_sdk_log_level(const char* aae_sdk_log_level);
int nvram_get_wan_access(char* wan_access);
int nvram_get_ddns_name(char* ddns_hostname);
/*int nvram_get_https_wan_port(char* https_wan_port);
int nvram_get_http_wan_port(char* http_wan_port);
int nvram_get_http_enable(char* http_enable);*/
void WatchingNVram();
int nvram_get_mac_addr(char* mac_addr);
int nvram_get_link_internet();
#endif
