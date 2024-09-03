#ifndef __CFG_SMARTHAUL_H__
#define __CFG_SMARTHAUL_H__

#define SMARTHAUL_LIST_JSON_PATH	TEMP_ROOT_PATH"/smarthaul.json"
#define SMARTHAUL_IPC_SOCKET_PATH	"/var/run/smarthaul_ipc_socket"
#define SMARTHAUL_INFO_JSON_PATH	TEMP_ROOT_PATH"/smarthaul_info.json"
#define DEVICE_INFO_JSON_PATH	TEMP_ROOT_PATH"/device_info.json"

enum smartHaulListAction {
	SHLIST_ADD = 1,
	SHLIST_UPDATE,
	SHLIST_DELETE,
	SHLIST_MAX
};

extern int cm_smartHaulSetTidmap(unsigned char *data);
extern int cm_smartHaulPacketProcess(unsigned char *data);
extern void cm_updateTidmapByMac(char *reMac, char *tidmapInfo, int action);
extern int cm_prepareTidmapInfoByMac(json_object *root, char *reMac);
extern int cm_forwardEventToSmartHaul(char *reMac, char *data, int event);
extern void cm_resetDeviceInfo();
extern int cm_checkDeviceInfo(json_object *info);
extern void cm_setTidmapInfo(unsigned char *msg);
extern void cm_setTidmapInfoByNotify(unsigned char *msg);
//extern int cm_checkSmartHaulInfoExist(char *reMac);

#endif /* __CFG_SMARTHAUL_H__ */
/* End of cfg_smarthaul.h */
