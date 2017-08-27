#ifndef _CMD_COMMON_H
#define _CMD_COMMON_H

#define DEVICE_DESC_FLAG_ADD_TNL_VER 0x00000001

#define ASUS_DEVICE_NAME_LEN    64

#define KAL_TIME		43200
#define APP_LOG_PATH	"/tmp/aaews_log"
#define SDK_LOG_PATH	"/tmp"
#define SDK_LOG_FLAG_FILE "/tmp/tnl_log_on.txt" 
#define PATH_LEN	128
#define ID_MAX_LEN	64
#define PWD_MAX_LEN	64
#define URL_MAX_LEN	128
#define FLAG_LEN	2
#define MAC_LEN     20


#define my_memcpy(dst, src, dst_len, src_len) {memcpy(dst, src, dst_len < src_len ? dst_len : src_len);}

extern int DeviceDescGen(char *buf, int *buf_len, int flag);

#endif
