#ifndef __CFG_MLO_H__
#define __CFG_MLO_H__

#include <json.h>

extern void prepareMldGroup(void);
extern int set_Mlo_Parameter(struct json_object *cfgRoot);
extern void cm_transMlo_Parameter(char *mac, int reBandNum, json_object *outRoot, json_object *inRoot);

#endif /* __CFG_MLO_H__ */
/* End of cfg_mlo.h */