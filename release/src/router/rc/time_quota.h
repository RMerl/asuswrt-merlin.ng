#ifndef __TIME_QUOTA__

#define TIME_QUOTA_INTERVAL 10

#define TQ_NFCM_DB "/jffs/nfcm_app.db"

#define TIME_QUOTA_DEBUG "/tmp/TIME_QUOTA_DEBUG"

#define TQ_DBG(fmt,args...) do { \
	if(f_exists(TIME_QUOTA_DEBUG) > 0) { \
		_dprintf("[TIME_QUOTA][%s:(%d)]"fmt"\n", __FUNCTION__, __LINE__, ##args); \
	} \
} while(0)

#define TQ_CHAIN_NAME "TIME_QUOTA"
#define TQ_RULE_SCRIPT_PATH "/tmp/tq_rules.sh"
#define TQ_RULE_PERSIST_SCRIPT_PATH "/tmp/tq_rules_persist.sh"

int get_tq_client_list(json_object *client_list, char *now_s);
int check_and_update_tq_client_list(json_object *client_list, char *now_s);
int get_tq_profile_list(json_object *profile_list, char *now_s);
void apply_tq_rules_for_restart_firewall(json_object *rule_persist_list);
int check_and_update_tq_profile_list(json_object *profile_list, json_object *client_list, json_object *rule_persist_list, char *now_s);

#endif