#ifndef __CFG_IPC_H__
#define __CFG_IPC_H__

#define CFGMNT_IPC_SOCKET_PATH	"/var/run/cfgmnt_ipc_socket"
#define CFGMNT_IPC_MAX_CONNECTION       10

extern void cm_rcvIpcHandler(int sock);
extern int cm_sendIpcHandler(char *ipcPath, char *data, int dataLen);

#endif /* __CFG_IPC_H__ */
/* End of cfg_ipc.h */
