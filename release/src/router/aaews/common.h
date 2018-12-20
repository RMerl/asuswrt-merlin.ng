#ifndef _CMD_COMMON_H
#define _CMD_COMMON_H

#include <shared.h>

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
//#define AICLOID_LEAST_VER	"2.1.0.0"
//#define AIHOME_LEAST_VER	"1.0.0.2.29"
#define ASUS_DEVICE_DESC	"{\"public\":\"0\", \"name\":\"%s\", \"tnlver\":\"%s\", \"AiHOMEAPILevel\":\"%d\", \"aae_enable\":\"%d\", \"fwver\":\"%s\", \"modelname\":\"%s\"}"
#define ASUS_DESC_DEF		"Router"
#define ASUS_DEVICE_DESC_LEN	512	
																																				#define GEMTEK_DEVICE_NAME      "AiCam"
#define GEMTEK_DEVICE_SERVICE   "1002"
#define GEMTEK_DEVICE_DESC	"{\"name\":\"AiCam\"}"

#define APILEVEL_STATUS_SUPPORT                "0"
#define APILEVEL_STATUS_APILEVEL_NOT_SUPPORT   "1"
#define APILEVEL_STATUS_FW_VERSION_NOT_SUPPORT "2"
#define APILEVEL_STATUS_END_OF_LIFE            "3"


#define my_memcpy(dst, src, dst_len, src_len) {memcpy(dst, src, dst_len < src_len ? dst_len : src_len);}

extern int DeviceDescGen(char *buf, int *buf_len, int flag);

#endif
