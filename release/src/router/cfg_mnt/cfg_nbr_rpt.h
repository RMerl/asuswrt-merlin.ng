#ifndef __CFG_NBR_RPT_H__
#define __CFG_NBR_RPT_H__

#define MAX_NBR_DATA_BUFLEN	512
#define NBR_LIST_JSON_FILE		TEMP_ROOT_PATH"/nbr_list.json"
#define NBR_LIST_PRIVATE_JSON_FILE		TEMP_ROOT_PATH"/nbr_list_private.json"

extern void cm_updateNbrData(char *msg);
extern int cm_getNbrData(char *buf, size_t len);
extern void cm_updateNbrListVersion();
extern int cm_prepareNbrList(unsigned char *msg, json_object *outRoot);
extern void cm_updateNbrList(unsigned char *msg);
extern int cm_updateOnlineInNbrData(char *mac, int online);

#endif /* __CFG_NBR_RPT_H__ */
/* End of cfg_nbr_rpt.h */
