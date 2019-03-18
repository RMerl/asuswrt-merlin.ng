#ifndef __CFG_DUALBANDLIST_H__
#define __CFG_DUALBANDLIST_H__

#define CLIENT_ASSOCIATED_LIST_JSON_PATH	"/tmp/assoclist.json"
#define DUAL_BAND_LIST_PATH	CFG_MNT_FOLDER"dblist"
#define DUAL_BAND_LIST_FILE_LOCK	"dblist"
#define MAX_DUAL_BAND_STA		256
#define MAX_SIZE_DBSTA_LIST		(256 * 32)

typedef struct _DBSTA_ALL_TABLE {
	char dbsta_all[MAX_SIZE_DBSTA_LIST];
	int num;
} DBSTA_ALL_TABLE, *P_DBSTA_ALL_TABLE;

extern int cm_initDBListSharedMemory();
extern void cm_destroyDBListSharedMemory(int toFile);
extern void cm_loadFileToDBListSharedMemory();
extern void cm_writeDBListSharedMemoryToFile();
extern void cm_detachDBStaListSharedMemory();
extern int cm_checkDualBandCapability(char *staMac);
extern int cm_checkDualBandListUpdate(char *staMac);
extern void cm_updateDualBandList(char *staMac);
extern void cm_handleDualBandListUpdate(unsigned char *msg);
extern int cm_prepareDualBandListMsg(char *msg, int msgLen);
extern int cm_staSupportDualBandCapability(json_object *root, char *band, char *staMac, long ts);
extern void cm_updateDualBandStaTimestamp(char *staMac);
extern void cm_checkDBListUpdated();

#endif /* __CFG_DUALBANDLIST_H__ */
/* End of cfg_dualbandlist.h */
