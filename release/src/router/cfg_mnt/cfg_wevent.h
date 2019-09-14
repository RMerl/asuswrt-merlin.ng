#ifndef __CFG_WEVENT_H__
#define __CFG_WEVENT_H__

#define WEVENT_SOCKET_PATH	"/etc/wevent_socket"
#define MAX_WEVENT_SOCKET_CLIENT    5
#define MAX_EVENT_INFO_LEN    512
#define WCLIENT_LIST_JSON_PATH	"/tmp/wclientlist.json"
#define WEVENT_FILE_LOCK	"wevent"
#define ALLWCLIENT_LIST_JSON_PATH	"/tmp/allwclientlist.json"
#define ALLWEVENT_FILE_LOCK	"allwevent"
#define WEVENT_TIMESTAMP	"ts"
#define WEVENT_EVENT		"event"

extern void cm_rcvWeventHandler();
extern void cm_weventPacketProcess(unsigned char *data);
extern void cm_processProbeReq(unsigned char *data);	//for test
extern int cm_checkReWifiConnected(char *reMac, char *modelName);
extern void cm_removeReWifiConnectedHistory(char *reMac);

#endif /* __CFG_WEVENT_H__ */
/* End of cfg_wevent.h */
