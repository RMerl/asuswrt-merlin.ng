#ifndef __CFG_AFC_H__
#define __CFG_AFC_H__

#define AFC_INFO_JSON_PATH	TEMP_ROOT_PATH"/afc_info.json"
#define AFC_LIST_JSON_PATH	TEMP_ROOT_PATH"/afc.json"
#define MAX_VERTICAL_UNCERTAINTY	100		/* meters */
#define MAX_HORIZONTAL_UNCERTAINTY	325		/* major & minor is same, 650/2 meters */

enum afcListAction {
	AFC_LIST_ADD = 1,
	AFC_LIST_UPDATE,
	AFC_LIST_DELETE,
	AFC_LIST_MAX
};

extern int cm_removeAfcRelatedInfoByMac(char *mac);
extern void cm_computeAfcPathLoss(CM_CLIENT_TABLE *p_client_tbl, json_object *list);
extern void cm_updateAfcByMac(char *reMac, char *heightUncert, char *horizMajorUncert, char *horizMinorUncert, int action);
extern int cm_prepareAfcInfoByMac(json_object *root, char *reMac);

#endif /* __CFG_AFC_H__ */
/* End of cfg_afc.h */
