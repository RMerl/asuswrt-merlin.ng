#ifndef __CFG_CLIENTLIST_H__
#define __CFG_CLIENTLIST_H__

#define CLIENTLIST_FILE_LOCK	"clientlist"
#define CLIENT_LIST_JSON_PATH	"/tmp/clientlist.json"

extern int cm_prepareClientListMsg(char *msg, int msgLen);
extern void cm_processClientList(char *msg);

#endif /* __CFG_CLIENTLIST_H__ */
/* End of cfg_clientlist.h */
