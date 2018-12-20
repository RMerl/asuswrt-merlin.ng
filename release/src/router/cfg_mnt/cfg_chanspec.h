#ifndef __CFG_CHANSPEC_H__
#define __CFG_CHANSPEC_H__

#define MAX_CHANSPEC_BUFLEN	512
#define CHANSPEC_LIST_JSON_PATH	TEMP_ROOT_PATH"/chanspec_all.json"
#define CHANSPEC_PRIVATE_LIST_JSON_PATH	TEMP_ROOT_PATH"/chanspec_private.json"
#define CHANSPEC_FILE_LOCK	"chanspec"
#define CHANSPEC_AVAILABLE_LIST_JSON_PATH	TEMP_ROOT_PATH"/chanspec_avbl.json"
#define CHANSPEC_AVAILABLE_LIST_TXT_PATH	TEMP_ROOT_PATH"/chanspec_avbl.txt"
#define MAX_BAND_NUM	2
#define START_CHANNEL_NUMBER_5G	36

typedef uint32_t chinfo_t;
typedef uint32_t bwinfo_t;

extern void cm_updateChanspec(char *msg);
extern void cm_removeChanspecByMac(char *mac);
extern int cm_getChanspec(json_object *chanspecObj, int check);
extern void cm_updatePrivateChanspec();
extern int cm_loadPrivateChannel(chinfo_t *avblChannel, int channelCount);
extern int cm_isValidChannel(int channel);
extern void cm_resetChanspec();

#endif /* __CFG_CHANSPEC_H__ */
/* End of cfg_chanspec.h */
