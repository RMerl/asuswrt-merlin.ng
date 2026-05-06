#ifndef __CFG_IPC_H__
#define __CFG_IPC_H__

#include <json.h>

#define CFGMNT_IPC_SOCKET_PATH	"/var/run/cfgmnt_ipc_socket"
#define CFGMNT_IPC_MAX_CONNECTION       10
#ifndef MAX_IPC_PACKET_SIZE
#define MAX_IPC_PACKET_SIZE		8192
#endif

extern void cm_rcvIpcHandler(int sock);
extern int cm_sendIpcHandler(char *ipcPath, char *data, int dataLen);
extern int cm_updateConfigChanged(char *mac, json_object *config);

#endif /* __CFG_IPC_H__ */
/* End of cfg_ipc.h */
