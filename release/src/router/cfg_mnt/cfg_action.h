#ifndef __CFG_ACTION_H__
#define __CFG_ACTION_H__

#include <json.h>

struct action_s {
	int event_id;		/* event id */
	int action;		/* action */
	int cap_action;	/* cap can do action */
	int cap_type;		/* capability type */
};

struct actionHandler_s  {
	int action;
	int (*func)(json_object *dataObj);
};

enum actionType {
	ACTION_NONE = 0,
	ACTION_REBOOT = 1,
	ACTION_RE_RECONNECT = 2,
	ACTION_FORCE_ROAMING = 3,
	ACTION_MAX
};

extern int cm_findActionInfo(int eid, int *action, int *capAction, int *capType);
extern void cm_actionHandler(unsigned char *msg);

#endif /* __CFG_ACTION_H__ */
/* End of cfg_action.h */
