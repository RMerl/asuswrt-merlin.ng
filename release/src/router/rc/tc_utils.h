#ifndef __TC_UTILS_H__
#define __TC_UTILS_H__

#ifdef RTCONFIG_LIBASUSLOG
#include <libasuslog.h>

#define TENCENT_DBG(fmt, arg...) do {\
	int save_errno = errno; \
	if (1) \
		asusdebuglog(LOG_INFO, "/jffs/tencent.log", LOG_CUSTOM, LOG_SHOWTIME, 0, "[%s(%d)]:"fmt"\n", __FUNCTION__, __LINE__ , ##arg); \
	errno = save_errno; \
}while(0)
#else
#define TENCENT_DBG(fmt,args...) \
	if(1) { \
		char info[1024]; \
		snprintf(info, sizeof(info), "echo \"[TENCENT_DBG][%s:(%d)] "fmt"\" >> /tmp/tencent.log", __FUNCTION__, __LINE__, ##args); \
		system(info); \
	}
#endif

#define TC_STATE_GOING			"going"
#define TC_STATE_COMPLETED 		"completed"
#define TC_STATE_FAILED			"failed"
#define TC_STATE_CANCELLED		"cancelled"
#define TC_STATE_EXPIRED		"expired"
#define TC_STATE_DELETE			"delete" //for tc_utils use only

/* error code for disk check */
enum DISK_ERROR {
	DISK_SPACE_ENOUGH = 0,
	DISK_SPACE_NOT_ENOUGH,
	DISK_READ_INFO_FAIL,
	DISK_PARTITION_NOT_FOUND
};

enum TC_DOWNLOAD_ERROR {
	OK = 0,
	TC_MD5_MISMATCH,
	TC_PARTITION_NOT_FOUND,
	TC_SPACE_NOT_ENOUGH,
	TC_READ_DISKINFO_FAIL,
	TC_CURL_ERROR
};

#define TC_SESSION_FILE			"/jffs/tencent_session.json"
#define SESSION_RECORD	 		"/jffs/tencent_session_update.json"
#define TC_DOWNLOAD_PID_FILE 	"/var/run/tc_download.pid"
#define MINIMAL_PROGRESS_FUNCTIONALITY_INTERVAL		3

extern int tc_download_main(int argc, char *argv[]);
int check_available_space(struct json_object *filelist, char *download_device);
long long get_file_size(char *file_path);
int remove_dir(char *dirPath);
int start_session_download(char *sessionFile);
int tc_curl_download_file(char *url, char *file_path, int retry, int check_CA);
void update_session_record(char *record_file, struct json_object *update_target);
#endif	/* !__TC_UTILS_H__ */