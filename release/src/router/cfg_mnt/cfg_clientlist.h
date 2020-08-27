#ifndef __CFG_CLIENTLIST_H__
#define __CFG_CLIENTLIST_H__

#include <json.h>

#define CLIENTLIST_FILE_LOCK	"clientlist"
#define CLIENT_LIST_JSON_PATH	"/tmp/clientlist.json"
#define WIREDCLIENTLIST_FILE_LOCK	"wiredclientlist"
#define WIRED_CLIENT_LIST_JSON_PATH	"/tmp/wiredclientlist.json"
#define CURRENT_WIRED_CLIENT_LIST_JSON_PATH	"/tmp/current_wired_client_list.json"

extern int cm_prepareClientListMsg(char *msg, int msgLen);
extern void cm_processClientList(char *msg);
extern int cm_checkReWiredConnected(char *reMac, char *modelName);
extern int cm_needUpdateWiredClientlLst(json_object *wiredClientList);

#endif /* __CFG_CLIENTLIST_H__ */
/* End of cfg_clientlist.h */
