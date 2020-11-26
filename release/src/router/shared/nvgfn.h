#ifndef __NVGFN_H__
#define __NVGFN_H__

/* DEBUG DEFINE */
#define NVGFN_DEBUG             "/tmp/NVGFN_DEBUG"
#define NVGFN_DBG(fmt,args...) \
	if(f_exists(NVGFN_DEBUG) > 0) { \
		_dprintf("[NVGFN][%s:(%d)]"fmt, __FUNCTION__, __LINE__, ##args); \
	}

typedef struct nvgfn_qos_rule_t {
	char channel[8];        // audio, control, mic, video
	char protocol[8];       // udp, tcp
	int port;               // 1024-65535
	char direction[8];		// in, out, inout
} NVGFN_QOS_RULE_T;

typedef struct nvgfn_mcs_index_t {
	int McsIndexAuto;		// 1: auto  0:
	int McsIndex;			// 0-23
	char McsIndexType[8];	// ht, vht
	int SpatialStreams;		// 1-3
} NVGFN_MCS_INDEX_T;

/* NVIDIA GeForceNow UPnP service DEFINE */
extern void nvgfn_kernel_setting(int val);
extern void nvgfn_mcs_isauto(int val);
extern char *upnp_nvram_get(char *name);
extern int upnp_nvram_set(char *name, char *value);
extern int nvgfn_GetPacketDropCount(int *value);
extern int nvgfn_GetQosState(int *value);
extern int nvgfn_SetQosState(char *value);
extern int nvgfn_GetQosRule(NVGFN_QOS_RULE_T *qos_rule);
extern int nvgfn_SetQoSRule(NVGFN_QOS_RULE_T *qos_rule);
extern int nvgfn_GetWifiScanState(int *value);
extern int nvgfn_SetWifiScanState(char *value);
extern int nvgfn_GetWifiScanInterval(int *value);
extern int nvgfn_SetWifiScanInterval(char *value);
extern int nvgfn_GetDownloadBandwidth(int *value);
extern int nvgfn_SetDownloadBandwidth(char *value);
extern int nvgfn_GetDownloadBandwidthReservation(int *value);
extern int nvgfn_SetDownloadBandwidthReservation(char *value);
extern int nvgfn_GetUploadBandwidth(int *value);
extern int nvgfn_SetUploadBandwidth(char *value);
extern int nvgfn_GetUploadBandwidthReservation(int *value);
extern int nvgfn_SetUploadBandwidthReservation(char *value);
extern int nvgfn_GetMcsIndex(NVGFN_MCS_INDEX_T *input);
extern int nvgfn_SetMcsIndex(NVGFN_MCS_INDEX_T *input);
#endif	/* !__NVGFN_H__ */
