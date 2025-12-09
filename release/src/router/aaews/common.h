#ifndef _CMD_COMMON_H
#define _CMD_COMMON_H

#include "shared.h"
#ifdef RTCONFIG_LIBASUSLOG
#include "libasuslog.h"
#endif

#define AIHOME_API_LEVEL	EXTEND_AIHOME_API_LEVEL // From shared/shared.h

#define DEVICE_DESC_FLAG_ADD_TNL_VER 0x00000001

#define ASUS_DEVICE_NAME_LEN    64

#define APP_LOG_PATH	"/tmp/aaews_log"
#define SDK_LOG_PATH	"/tmp"
#define SDK_LOG_FLAG_FILE "/tmp/tnl_log_on.txt" 
#define PATH_LEN	128
#define ID_MAX_LEN	64
#define PWD_MAX_LEN	64
#define URL_MAX_LEN	128
#define FLAG_LEN	2
#define MAC_LEN     20
#define ACCOUNT_LEN     64
#define PWD_LEN     16

#define NVRAM_FIRMVER "firmver"
#define NVRAM_BUILDNO "buildno"
#define NVRAM_EXTENDNO "extendno"
#define NVRAM_MODEL_NAME     "productid"
#define NVRAM_ODMPID     "odmpid"
#define NVRAM_HTTPS_LANPORT     "https_lanport"
#define NVRAM_HTTP_ENABLE     "http_enable"

#define APP_DBG			1
#define KAL_SEC			43200
#define TIMER_SEC		2
#define DECLARE_CLEAR_MEM(type, var, len) \
	type var[len]; \
	memset(var, 0, len );

#define DEVICE_TYPE		"93"
#define PERMISSION 		"1"
#define NAT_JSON		"{\"nattype\":\"%d\"}"
#define	ASUS_DEVICE_NAME	"ASUS-Router"
#define ASUS_DEVICE_SERVICE	"1001"
#define ASUS_DEVICE_ACCOUNT_BINDING_SERVICE	"1004"

//#define AICLOID_LEAST_VER	"2.1.0.0"
//#define AIHOME_LEAST_VER	"1.0.0.2.29"
#define ASUS_DEVICE_DESC	"{\"public\":\"0\", \"name\":\"%s\", \"tnlver\":\"%s\", \"AiHOMEAPILevel\":\"%d\", \"aae_enable\":\"%d\", \"fwver\":\"%s\", \"modelname\":\"%s\"}"
#define ASUS_DESC_DEF		"Router"
#define ASUS_DEVICE_DESC_LEN	1024
#define GEMTEK_DEVICE_NAME      "AiCam"
#define GEMTEK_DEVICE_SERVICE   "1002"
#define GEMTEK_DEVICE_DESC	"{\"name\":\"AiCam\"}"

#define APILEVEL_STATUS_SUPPORT                "0"
#define APILEVEL_STATUS_APILEVEL_NOT_SUPPORT   "1"
#define APILEVEL_STATUS_FW_VERSION_NOT_SUPPORT "2"
#define APILEVEL_STATUS_END_OF_LIFE            "3"

#define AWS_CERTS_PATH "/jffs/awscerts"
#define AWS_CERTS_CA_FILE "/jffs/awscerts/amazon-root-ca.pem"
#define AWS_CERTS_CRT_FILE "/jffs/awscerts/device.pem.crt"
#define AWS_CERTS_KEY_FILE "/jffs/awscerts/private.pem.key"

#define AAE_TUNNEL_STATUS_RES "{\"function_name\": \"tunnel_status\", \"status\": \"%d\"}"
#define AAE_TUNNEL_TEST_RES "{\"function_name\": \"tunnel_test\", \"source\": \"%s\", \"target\": \"%s\", \"type\" : \"%s\", \"error\": \"%d\"}"

#define my_memcpy(dst, src, dst_len, src_len) {memcpy(dst, src, dst_len < src_len ? dst_len : src_len);}

#define IS_EULA_OR_PPV2_SIGNED() ((nvram_get_int("ASUS_EULA") == 1) || (get_ASUS_privacy_policy_state(ASUS_PP_ACCOUNT_BINDING) == 1))

extern char *generate_device_desc(int public, char *tnl_sdk_version, char *out_buf, int out_len);

#ifdef RTCONFIG_LIBASUSLOG
#define AAE_MAX_LOG_LEN 960
#define AAE_DBG_LOG	"aae.log"
extern char *__progname;
#define AAEDBG(fmt,args...) \
		asusdebuglog(LOG_INFO, AAE_DBG_LOG, LOG_CUSTOM, LOG_SHOWTIME, 50, "[%s][%d]][%s:(%d)] "fmt"\n", __progname, getpid(), __FUNCTION__, __LINE__, ##args);
#else
#define AAEDBG(fmt,args...)
#endif

#endif
