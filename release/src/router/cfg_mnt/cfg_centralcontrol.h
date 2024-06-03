#ifndef __CFG_CENTRALCONTROL_H__
#define __CFG_CENTRALCONTROL_H__

#define COMMON_CONFIG	"common"
#define VALUE_BUF_MAX	(16384)

enum ruleAction {
	RULE_ADD = 1,
	RULE_DEL,
	RULE_UPDATE
};

enum followRule {
	FOLLOW_COMMON = 1,
	FOLLOW_PRIVATE
};

#define COMMON_CONFIG_FILE_LOCK	"common_config"

extern int cm_updateCommonToPrivateConfig(char *mac, unsigned char *ftList, json_object *cfgRoot);
extern int cm_transformCfgToArray(json_object *cfgObj, json_object *arrayObj);
extern int cm_updatePrivateRuleByMac(char *mac, json_object *cfgObj, int follow, int action);
extern int cm_checkParamFollowRule(char *mac, char *param, int rule);
extern int cm_updateCommonConfig();
#ifdef UPDATE_COMMON_CONFIG
extern int cm_updateCommonConfigToFile(json_object *cfgRoot);
extern int cm_updateValueToConfigFile(char *name, char *value);
#endif
extern char *cm_getValueFromCommonConfig(const char *name, char *valueBuf, int valueBufLen);
extern json_object *cm_getCommonObjFromCommonConfigFile();
extern char *cm_getStrValueFromCommonObj(json_object *commonObj, const char *name, char *valueBuf, int valueBufLen);
extern int cm_getIntValueFromCommonObj(json_object *commonObj, const char *name);
extern int cm_isParamExistInCommonObj(json_object *commonObj, const char *name);
#ifdef RTCONFIG_AMAS_CAP_CONFIG
extern void cm_addConfigFromCommonFileByFeature(json_object *inRoot, json_object *outRoot);
#endif

#endif /* __CFG_CENTRALCONTROL_H__ */
/* End of cfg_centralcontrol.h */