#ifndef _LETSENCRYPT_CONFIG_H
#define _LETSENCRYPT_CONFIG_H

#define LE_ACME_CERT_HOME	"/jffs/.le"
#define LE_ACME_ACC_KEY	"account.key"
#define LE_ACME_DOMAIN_KEY	"domain.key"
#define LE_ACME_DOMAIN_CERT	"cert.pem"
#define LE_ACME_DOMAIN_FULLCHAIN	"fullchain.pem"

//certificate
typedef enum {
	LE_ST_NONE=0,
	LE_ST_ISSUED,
} le_st_t;

//rc
typedef enum {
	LE_STS_DISABLED=0,
	LE_STS_STOPPING,
	LE_STS_STOPPED,
	LE_STS_CHECKING,
	LE_STS_WAIT_RETRY,
	LE_STS_REVOKE,
	LE_STS_ISSUE,
	LE_STS_RENEW,
	LE_STS_WAIT_RENEW
} le_sts_t;

//acme
typedef enum {
	LE_SB_NONE=0,
	LE_SB_ACC,
	LE_SB_CHA,
	LE_SB_DNS,
	LE_SB_CHA_VRF,
	LE_SB_CSR,
	LE_SB_REVOKE
} le_sbsts_t;

//reason
typedef enum {
	LE_AUX_NONE=0,
	LE_AUX_CONFIG,
	LE_AUX_INTERNET,
	LE_AUX_NTP,
	LE_AUX_DDNS,
	LE_AUX_ACME
} le_auxsts_t;

typedef enum {
	LE_ACME_AUTH_HTTP=0,
	LE_ACME_AUTH_DNS,
	LE_ACME_AUTH_TLS
} le_acme_auth_t;

typedef struct {
	int enable;
	le_acme_auth_t authtype; 
	int force;
	int staging;
	int debug;
	char acme_logpath[128];
	char revoke_hostname[128];

	//router config
	int ddns_enabled;
	char ddns_hostname[128];
	int https_enabled;
	int webdav_enabled;
	char lan_ipaddr[16];

} le_conf_t;

void update_le_st(le_st_t status);
void update_le_sts(le_sts_t status);
void update_le_sbsts(le_sbsts_t status);
void update_le_auxsts(le_auxsts_t status);
le_conf_t* get_le_conf(le_conf_t* get_le_conf);
int check_le_configure(le_conf_t* conf);
le_auxsts_t check_le_status(void);
char* get_path_le_domain_fullchain(char* path, size_t len);
char* get_path_le_domain_cert(char* path, size_t len);
char* get_path_le_domain_key(char* path, size_t len);
#endif