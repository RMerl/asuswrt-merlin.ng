#ifndef __CFG_CENTRALCONTROL_H__
#define __CFG_CENTRALCONTROL_H__

#define COMMON_CONFIG	"common"

enum ruleAction {
	RULE_ADD = 1,
	RULE_DEL,
	RULE_UPDATE
};

enum followRule {
	FOLLOW_CAP = 1,
	FOLLOW_RE
};

extern int cm_updateCommonToPrivateConfig(char *mac, unsigned char *ftList, json_object *cfgRoot);
extern int cm_transformCfgToArray(json_object *cfgObj, json_object *arrayObj);
extern int cm_updatePrivateRuleByMac(char *mac, json_object *cfgObj, int follow, int action);
extern int cm_checkParamFollowRule(char *mac, char *param, int rule);
extern int cm_updateCommonConfig();
#ifdef UPDATE_COMMON_CONFIG
extern int cm_updateCommonConfigToFile(json_object *cfgRoot);
#endif

#endif /* __CFG_CENTRALCONTROL_H__ */
/* End of cfg_centralcontrol.h */