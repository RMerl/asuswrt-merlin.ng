#include <string.h>
#include <json.h>
#include <time.h>

#include <sqlite3.h>
#include "shared.h"
#include "time_quota.h"
#include "rc.h"
#include "pc.h"
#ifdef RTCONFIG_LIB_CODB
#include <cosql_utils.h>
#endif

//#define CLEAN_RULE_MODE 1

#define TQ_FLAG_NORMAL 			1
#define TQ_FLAG_RULE_ADD_NEEDED 1<<1
#define TQ_FLAG_RULE_ADDED 		1<<2
#define TQ_FLAG_RULE_DEL_NEEDED 1<<3
#define TQ_FLAG_OBJ_DEL_NEEDED	1<<4

#define TO_UL(s) strtoul(s, NULL, 10)
#define TO_L(s) strtol(s, NULL, 10)
#define TO_HEX_L(s) strtol(s, NULL, 16)

static int g_fw_rules_update_needed = 0;

static inline int get_time_quota_interval() {
	return nvram_get_int("nf_conntrack_timer") ? : TIME_QUOTA_INTERVAL;
}

static int is_same_local_day(time_t time1, time_t time2) {
	struct tm tm1 = *localtime(&time1), tm2 = *localtime(&time2);

	//TQ_DBG("time1=%llu, time2=%llu, %d", time1, time2, (tm1.tm_year == tm2.tm_year && tm1.tm_mon == tm2.tm_mon && tm1.tm_mday == tm2.tm_mday));
	return (/*(tm1 = *localtime(&time1)) && (tm2 = *localtime(&time2)) &&*/
			 (tm1.tm_year == tm2.tm_year && tm1.tm_mon == tm2.tm_mon && tm1.tm_mday == tm2.tm_mday));
}

//It could get ip address from amas_lib by using mac if no data in db.
static int get_client_sec_used_from_db(char *mac, int allowed_bg_traffic, unsigned long s_ts, unsigned long e_ts) {
	char ip_addr[18] = {0};

	if (strlen(mac) && amas_lib_device_ip_query(mac, ip_addr)) {
		int lock;
		sqlite3 *db;
		int row_count;
		struct in_addr src_ip_addr;

		inet_pton(AF_INET, ip_addr, &src_ip_addr);

		lock = file_lock("nfcm_app");
#ifdef RTCONFIG_LIB_CODB
		sql_column_match_t and_matches[3];
		sql_column_match_t or_matches[2];
		int ret;
		if ((db = cosql_open(TQ_NFCM_DB)) == NULL) {
			TQ_DBG("Can't open database %s", TQ_NFCM_DB);
			file_unlock(lock);
			return 0;
		}
		if (f_exists(TIME_QUOTA_DEBUG))
			cosql_enable_debug(db, 1);
		and_matches[0].name = "timestamp";	and_matches[0].type = COLUMN_TYPE_INT;		and_matches[0].operation = OP_TYPE_GREATER_THAN;			and_matches[0].value.i = (int)s_ts;
		and_matches[1].name = "timestamp";	and_matches[1].type = COLUMN_TYPE_INT;		and_matches[1].operation = OP_TYPE_LESSER_THAN_OR_EQUAL;	and_matches[1].value.i = (int)e_ts;
		and_matches[2].name = "src_ip";		and_matches[2].type = COLUMN_TYPE_UINT;		and_matches[2].operation = OP_TYPE_EQUAL;					and_matches[2].value.ui = src_ip_addr.s_addr;
		or_matches[0].name = "up_speed";	or_matches[0].type = COLUMN_TYPE_UINT64;	or_matches[0].operation = OP_TYPE_GREATER_THAN;				or_matches[0].value.ui64 = (uint64_t)allowed_bg_traffic;
		or_matches[1].name = "dn_speed";	or_matches[1].type = COLUMN_TYPE_UINT64;	or_matches[1].operation = OP_TYPE_GREATER_THAN;				or_matches[1].value.ui64 = (uint64_t)allowed_bg_traffic;
		ret = cosql_count_matchs(db, 
				sizeof(and_matches)/sizeof(sql_column_match_t), &and_matches[0],
				sizeof(or_matches)/sizeof(sql_column_match_t), &or_matches[0],
				&row_count);
		if (ret != COSQL_OK) {
			TQ_DBG("SQL error: %d", ret);
			cosql_close(db);
			file_unlock(lock);
			return 0;
		}

		cosql_close(db);
#else
		char sql_s[512] = {0};
		char **pResult, *zErr = NULL;
		int field_count;
		if (sqlite3_open(TQ_NFCM_DB, &db) != SQLITE_OK) {
			TQ_DBG("Can't open database %s", sqlite3_errmsg(db));
			file_unlock(lock);
			return 0;
		}

		snprintf(sql_s, sizeof(sql_s), "SELECT * FROM DATA_INFO WHERE timestamp > %d and timestamp <= %d and src_ip = %u and (up_speed > %llu or dn_speed > %llu);",
		(int)s_ts, (int)e_ts, src_ip_addr.s_addr, allowed_bg_traffic, allowed_bg_traffic);
		TQ_DBG("SQL:%s", sql_s);

		sqlite3_get_table(db, sql_s, &pResult, &row_count, &field_count, &zErr);
		if(zErr != NULL){
			TQ_DBG("SQL error: %s", zErr);
			sqlite3_free(zErr);
			sqlite3_close(db);
			file_unlock(lock);
			return 0;
		}

		sqlite3_free_table(pResult);
		sqlite3_close(db);
#endif
		file_unlock(lock);
		return (row_count) > 0 ? (e_ts - s_ts) : 0;
	} else {
		TQ_DBG("No IP address associated. mac=%s", mac);
	}

	return 0;
}

/* 
{
	"mac1" : {
				"name" : "xxx", 
				"group_id" : "AAAAAAAAAA", 
				"group_name" : "g1", 
				"join_ts" : "ts1", 
				"update_ts" : "ts2", 
				"sec_used" : 220,
				"flag" : 0, 
			},
	"mac2" : {
				"name" : "yyy", 
				"group_id" : "BBBBBBBBBB", 
				"group_name" : "g1", 
				"join_ts" : "ts1", 
				"update_ts" : "ts2", 
				"sec_used" : 150,
				"flag" : 0, 
			},
	...
	"macn" : {
				"name" : "zzz", 
				"group_id" : "CCCCCCCCCC", 
				"group_name" : "g1", 
				"join_ts" : "ts1", 
				"update_ts" : "ts2", 
				"sec_used" : 100,
				"flag" : 0, 
			}
}
*/
int get_tq_client_list(json_object *client_list, char *now_s) {
	char *g, *p, *buf;
	int have_data = 0;
	char *name, *mac, *groupn, *arg1, *arg2, *arg3, *arg4, *arg5, *groupid;
	json_object *client_attr = NULL;

	if (!client_list)
		return have_data;

	g = buf = strdup((const char *)nvram_safe_get("custom_clientlist"));
	if(strcmp(buf, "") != 0) {
		while (buf) {
			if ((p = strsep(&g, "<")) == NULL)
				break;

			if((vstrsep(p, ">", &name, &mac, &arg1, &arg2, &arg3, &arg4, &groupn, &arg5, &groupid)) != 9)
				continue;

			if (!strcmp(groupn, ""))
				continue;

			client_attr = json_object_new_object();
			json_object_object_add(client_attr, "name", json_object_new_string(name));
			json_object_object_add(client_attr, "group_id", json_object_new_string(groupid));
			json_object_object_add(client_attr, "group_name", json_object_new_string(groupn));
			json_object_object_add(client_attr, "join_ts", json_object_new_string(now_s));
			json_object_object_add(client_attr, "update_ts", json_object_new_string(now_s));
			json_object_object_add(client_attr, "sec_used", json_object_new_int(0));
			json_object_object_add(client_attr, "flag", json_object_new_int(TQ_FLAG_NORMAL));
			json_object_object_add(client_list, mac, client_attr);

			if(!have_data)
				have_data = 1;
		}
	}
	free(buf);
	return have_data;
}

int check_and_update_tq_client_list(json_object *client_list, char *now_s) {
	json_object *tmp_client_list = NULL;
	json_object *client = NULL;

	if (!client_list)
		return -1;

	tmp_client_list = json_object_new_object();
	get_tq_client_list(tmp_client_list, now_s);
	json_object_object_foreach(tmp_client_list, tmp_mac, tmp_client) {
		json_object *jgroup_id1 = NULL, *jgroup_id2 = NULL;
		json_object *value = NULL;

		// Skip empty group namme, this means the client doesn't belong to any group.
		if (json_object_object_get_ex(tmp_client, "group_id", &jgroup_id1) &&
			!strcmp(json_object_get_string(jgroup_id1), ""))
			continue;

		json_object_object_get_ex(client_list, tmp_mac, &client);

		if (client) { // old client update sec_used and update_ts
			json_object *jprev_update_ts/*, *jprev_sec_used*/;
			//int sec_used = 0;

			// Check if the group of client is changed. If true, reset the sec_used
			if (jgroup_id1 && json_object_object_get_ex(client, "group_id", &jgroup_id2)) {
				if (strcmp(json_object_to_json_string(jgroup_id1), json_object_to_json_string(jgroup_id2))) {
					json_object_object_add(client, "group_id", json_object_new_string(json_object_get_string(jgroup_id1)));
					json_object_object_add(client, "sec_used", json_object_new_int(0));
					//json_object_object_add(client, "flag", json_object_new_int(TQ_FLAG_RULE_DEL_NEEDED));
				}
			}

			// Check if new day came. If true, reset the sec_used
			if (json_object_object_get_ex(client, "update_ts", &jprev_update_ts) && 
				!is_same_local_day(TO_UL(now_s), TO_UL(json_object_get_string(jprev_update_ts)))) {
				json_object_object_add(client, "sec_used", json_object_new_int(0));
				//json_object_object_add(client, "flag", json_object_new_int(TQ_FLAG_RULE_DEL_NEEDED));
			}

			if (json_object_object_get_ex(tmp_client, "name", &value))
				json_object_object_add(client, "name", json_object_new_string(json_object_get_string(value)));
			if (jgroup_id1)
				json_object_object_add(client, "group_id", json_object_new_string(json_object_get_string(jgroup_id1)));
			if (json_object_object_get_ex(tmp_client, "group_name", &value))
				json_object_object_add(client, "group_name", json_object_new_string(json_object_get_string(value)));

			// Update sec_used
			//if (json_object_object_get_ex(client, "sec_used", &jprev_sec_used))
			//	sec_used = (int)TO_L(json_object_get_string(jprev_sec_used));

			/*if (jprev_update_ts)
				sec_used += get_client_sec_used_from_db(tmp_mac, TO_UL(json_object_get_string(jprev_update_ts)), TO_UL(now_s));
			else
				sec_used += get_client_sec_used_from_db(tmp_mac, (TO_UL(now_s)-TIME_QUOTA_INTERVAL), TO_UL(now_s));

			json_object_object_add(client, "sec_used", json_object_new_int(sec_used));*/

			// Update update_ts
			json_object_object_add(client, "update_ts", json_object_new_string(now_s));

		} else { // new client
			json_object *new_client = json_object_new_object();

			if (json_object_object_get_ex(tmp_client, "name", &value))
				json_object_object_add(new_client, "name", json_object_new_string(json_object_get_string(value)));
			if (json_object_object_get_ex(tmp_client, "join_ts", &value))
				json_object_object_add(new_client, "join_ts", json_object_new_string(now_s));
			if (json_object_object_get_ex(tmp_client, "group_name", &value))
				json_object_object_add(new_client, "group_name", json_object_new_string(json_object_get_string(value)));
			if (jgroup_id1)
				json_object_object_add(new_client, "group_id", json_object_new_string(json_object_get_string(jgroup_id1)));
			json_object_object_add(new_client, "update_ts", json_object_new_string(now_s));
			json_object_object_add(new_client, "sec_used", json_object_new_int(0));
			json_object_object_add(new_client, "flag", json_object_new_int(0));

			json_object_object_add(client_list, tmp_mac, new_client);
		}
	}

	json_object_put(tmp_client_list);

	// Should move the action "Rmove obsolete client" to a proper place due to firewall rules cleaning needed.
	/*json_object_object_foreach(client_list, cp_mac, cp_client) {
		json_object *json_udpate_ts = NULL;
		// Check the update_ts and remove obsolete client
		if (json_object_object_get_ex(cp_client, "update_ts", &json_udpate_ts)) {
			char *update_ts = (char *) json_object_get_string(json_udpate_ts);
			if (update_ts && strcmp(update_ts, now_s)) {
				json_object_object_del(client_list, cp_mac);
			}
		}
	}*/

	return 0;
}

/*
nvram : 
test>1048000>50000<test1>0><>0><test2>10000>a<test3>4>24<test4>4>1
name>quota>allow_backgroud_speed<...<...
{
	"group_id1" : {
				"quota" : 3600, 
				"allowed_bg_traffic" : 50000, 
				"gentle_pause" : 0, 
				"quota_used" : 204800
				"join_ts" : "ts1", 
				"update_ts" : "ts2", 
				"flag" : 0, 
			   },
	"group_id2" : {
				"quota" : 7200, 
				"allowed_bg_traffic" : 50000, 
				"gentle_pause" : 0, 
				"quota_used" : 204, 
				"join_ts" : "ts1", 
				"update_ts" : "ts2", 
				"flag" : 0, 
			   },
	...
	"group_id3" : {
				"quota" : 7200, 
				"allowed_bg_traffic" : 50000, 
				"gentle_pause" : 0, 
				"quota_used" : 3500, 
				"join_ts" : "ts1", 
				"update_ts" : "ts2", 
				"flag" : 0, 
			   },
}
*/
int get_tq_profile_list(json_object *profile_list, char *now_s) {
	char *g, *p, *buf;
	int have_data = 0;
	char *enable, *groupid, *wday, *quota, *allowed_bg_traffic, *gentle_pause;
	json_object *profile_attr = NULL;
	time_t now_t;
	struct tm now_tm;

	if (!profile_list)
		return have_data;

	g = buf = strdup((const char *)nvram_safe_get("time_quota_profile"));
	if(strcmp(buf, "") != 0) {
		while (buf) {
			if ((p = strsep(&g, "<")) == NULL)
				break;

			if((vstrsep(p, ">", &enable, &groupid, &wday, &quota, &allowed_bg_traffic, &gentle_pause)) != 6)
				continue;

			if (!strcmp(enable, "0"))
				continue;

			// only focus today, skip other day's setting.
			now_t = TO_UL(now_s);
			now_tm = *localtime(&now_t);
			if ((TO_HEX_L(wday) & (1 << now_tm.tm_wday)) == 0)
				continue;

			// Skip empty group name and zero quota.
			if (!strcmp(groupid, "") || !TO_UL(quota))
				continue;

			profile_attr = json_object_new_object();
			json_object_object_add(profile_attr, "quota", json_object_new_int(TO_L(quota)));
			json_object_object_add(profile_attr, "allowed_bg_traffic", json_object_new_int(TO_L(allowed_bg_traffic)));
			json_object_object_add(profile_attr, "gentle_pause", json_object_new_int(TO_L(gentle_pause)));
			json_object_object_add(profile_attr, "quota_used", json_object_new_int(0));
			json_object_object_add(profile_attr, "join_ts", json_object_new_string(now_s));
			json_object_object_add(profile_attr, "update_ts", json_object_new_string(now_s));
			json_object_object_add(profile_attr, "flag", json_object_new_int(TQ_FLAG_NORMAL));
			json_object_object_add(profile_list, groupid, profile_attr);

			if(!have_data)
				have_data = 1;
		}
	}
	free(buf);
	return have_data;
}

#define RULE_FINDING "-A FORWARD -m state --state INVALID -j DROP"
static int find_rule_insert_pos(int ipv6) {
	FILE *fp = NULL;
	if (!ipv6)
		fp = popen("iptables -S FORWARD", "r");
	else
		fp = popen("ip6tables -S FORWARD", "r");

	int num = 1;
	if (fp) {
		int count = 0;
		char buf[256] = {};
		while (++count && fgets(buf, sizeof(buf), fp) != NULL) {
			TQ_DBG("find_rule_insert_pos : %s", buf);
			if (strstr(buf, RULE_FINDING)) {
				num = count;
				break;
			}
		}
		pclose(fp);
	}
	return num;
}

#ifdef CLEAN_RULE_MODE
#define TQ_RULE_FINDING "-j " TQ_CHAIN_NAME
static int prepare_time_quota_rules_clean(FILE *write_to, int ipv6) {
	FILE *fp = NULL;
	if (!ipv6)
		fp = popen("iptables -S FORWARD", "r");
	else
		fp = popen("ip6tables -S FORWARD", "r");

	int num = 0;
	if (fp) {
		char buf[256] = {};
		while (fgets(buf, sizeof(buf), fp) != NULL) {
			TQ_DBG("find_rule_insert_pos : %s", buf);
			if (buf[1] == 'A' && strstr(buf, TQ_RULE_FINDING)) {
				buf[1] = 'D';
				if (!ipv6)
					fprintf(write_to, "iptables %s", buf);
				else
					fprintf(write_to, "ip6tables %s", buf);
				num++;
			}
		}
		pclose(fp);
	}
	return num;
}
#endif

static inline void write_rule_cmd(FILE *fp, int action, const char *chk_type, char *lan_if, char *mac, char *addr) {
	char *action_s = NULL, *logic_s, rule_pos[16] = {0};
#ifdef RTCONFIG_IPV6
	char rule_pos_ipv6[16] = {0};
#endif
	if (action == TQ_FLAG_RULE_ADD_NEEDED) {
		action_s = "-I";
		logic_s = "||";
		snprintf(rule_pos, sizeof(rule_pos), "%d", find_rule_insert_pos(0));
#ifdef RTCONFIG_IPV6
		if (ipv6_enabled()){
			snprintf(rule_pos_ipv6, sizeof(rule_pos_ipv6), "%d", find_rule_insert_pos(1));
		}
#endif
	}
	else if (action == TQ_FLAG_RULE_DEL_NEEDED) {
		action_s = "-D";
		logic_s = "&&";
	}
	if (action_s) {
		if (!strcmp(chk_type, iptables_chk_ip)) {
			fprintf(fp, "iptables -C FORWARD -i %s %s %s -j %s && iptables -D FORWARD -i %s %s %s -j %s\n", 
				lan_if, iptables_chk_mac, mac, TQ_CHAIN_NAME, 
				lan_if, iptables_chk_mac, mac, TQ_CHAIN_NAME);
		}
		fprintf(fp, "iptables -C FORWARD -i %s %s %s -j %s %s iptables %s FORWARD %s -i %s %s %s -j %s\n", 
			lan_if, chk_type, addr, TQ_CHAIN_NAME, 
			logic_s, action_s, rule_pos, lan_if, chk_type, addr, TQ_CHAIN_NAME);
#ifdef RTCONFIG_IPV6
		if (ipv6_enabled()){
			if (!strcmp(chk_type, iptables_chk_ip)) {
				fprintf(fp, "ip6tables -C FORWARD -i %s %s %s -j %s && ip6tables -D FORWARD -i %s %s %s -j %s\n", 
					lan_if, iptables_chk_mac, mac, TQ_CHAIN_NAME, 
					lan_if, iptables_chk_mac, mac, TQ_CHAIN_NAME);
			}
			fprintf(fp, "ip6tables -C FORWARD -i %s %s %s -j %s %s ip6tables %s FORWARD %s -i %s %s %s -j %s\n", 
				lan_if, chk_type, addr, TQ_CHAIN_NAME, 
				logic_s, action_s, rule_pos_ipv6, lan_if, chk_type, addr, TQ_CHAIN_NAME);
		}
#endif
		// clean conntrack and fc if any.
		if (action == TQ_FLAG_RULE_ADD_NEEDED) {
			char tip[16] = {0};
			if (!strcmp(chk_type, iptables_chk_ip))
				fprintf(fp, "conntrack -D -s %s\n", addr);
			else if (arpcache(mac, tip) == 0 && strlen(tip)) {
				fprintf(fp, "conntrack -D -s %s\n", tip);
			}
#ifdef HND_ROUTER
			fprintf(fp, "fc flush\n");
#elif defined(RTCONFIG_BCMARM)
			/* TBD. ctf ipct entries cleanup. */
#endif
		}
	}

}

static void prepare_rule(FILE *fp, json_object *rule_persist_list, char *lan_if, char *mac, int action, int gentle_pause) {
	TQ_DBG("mac=%s, action=%d", mac, action);

	const char *chk_type;
	char follow_addr[18] = {0};

	if (!fp)
		return;

#ifdef RTCONFIG_AMAS
	if (strlen(mac) && amas_lib_device_ip_query(mac, follow_addr)) {
		chk_type = iptables_chk_ip;
	} else
#endif
	{
		chk_type = iptables_chk_mac;
		snprintf(follow_addr, sizeof(follow_addr), "%s", mac);
	}

	write_rule_cmd(fp, action, chk_type, lan_if, mac, follow_addr);

	if (action == TQ_FLAG_RULE_ADD_NEEDED) {
		if (rule_persist_list) {
			json_object *jclient_attr = json_object_new_object();
			json_object_object_add(jclient_attr, "gentle_pause", json_object_new_int(gentle_pause));
			json_object_object_add(rule_persist_list, mac, jclient_attr);
		}
		g_fw_rules_update_needed = 1;
	}
	else if (action == TQ_FLAG_RULE_DEL_NEEDED) {
		if (rule_persist_list)
			json_object_object_del(rule_persist_list, mac);
		g_fw_rules_update_needed = 1;
	}
}

void apply_tq_rules_for_restart_firewall(json_object *rule_persist_list) {
	if (rule_persist_list && json_object_object_length(rule_persist_list)) {
		FILE *fp = NULL;
		if ((fp = fopen(TQ_RULE_PERSIST_SCRIPT_PATH, "w")) == NULL)
			return;
		fprintf(fp, "#!/bin/sh\n");
		chmod(TQ_RULE_PERSIST_SCRIPT_PATH, 0777);
		char *lan_if = strdup(nvram_safe_get("lan_ifname"));
		json_object_object_foreach(rule_persist_list, mac, rule) {
			const char *chk_type;
			char follow_addr[18] = {0};
			(void)rule;
		#ifdef RTCONFIG_AMAS
			if (strlen(mac) && amas_lib_device_ip_query(mac, follow_addr)) {
				chk_type = iptables_chk_ip;
			} else
		#endif
			{
				chk_type = iptables_chk_mac;
				snprintf(follow_addr, sizeof(follow_addr), "%s", mac);
			}
			write_rule_cmd(fp, TQ_FLAG_RULE_ADD_NEEDED, (char *)chk_type, lan_if, mac, follow_addr);
		}
		free(lan_if);
		fclose(fp);
		system(TQ_RULE_PERSIST_SCRIPT_PATH);
	}
}

#ifdef CLEAN_RULE_MODE //uncompleted
void apply_tq_rules(json_object *client_list) {
	if (client_list && json_object_object_length(client_list)) {
		FILE *fp = NULL;
		if ((fp=fopen(TQ_RULE_SCRIPT_PATH, "w"))==NULL) return;
		fprintf(fp, "#!/bin/sh\n");
		chmod(TQ_RULE_SCRIPT_PATH, 0777);

		prepare_time_quota_rules_clean(fp, 0);
#ifdef RTCONFIG_IPV6
		if (ipv6_enabled())
			prepare_time_quota_rules_clean(fp, 0);
#endif
		char *lan_if = strdup((const char *)nvram_safe_get("lan_ifname"));
		json_object_object_foreach(client_list, mac, client) {
			const char *chk_type;
			char follow_addr[18] = {0};
			json_object *jflag = NULL;
			char rule[512];
			int flag = 0;
			if (json_object_object_get_ex(client_list, "flag", &jflag) && jflag)
				flag = json_object_get_int(jflag);

			if ((flag & TQ_FLAG_RULE_ADDED)) {
#ifdef RTCONFIG_AMAS
				if (strlen(mac) && amas_lib_device_ip_query(mac, follow_addr)) {
					chk_type = iptables_chk_ip;
				} else
#endif
				{
					chk_type = iptables_chk_mac;
					snprintf(follow_addr, sizeof(follow_addr), "%s", mac);
				}
				snprintf(rule, sizeof(rule), "iptables -C FORWARD -i %s %s %s -j %s || iptables -I FORWARD %d -i %s %s %s -j %s", 
					lan_if, chk_type, follow_addr, TQ_CHAIN_NAME, find_rule_insert_pos(0), lan_if, chk_type, follow_addr, TQ_CHAIN_NAME);
				TQ_DBG("%s", rule);
#ifdef RTCONFIG_IPV6
				if (ipv6_enabled()) {
					snprintf(rule, sizeof(rule), "ip6tables -C FORWARD -i %s %s %s -j %s || ip6tables -I FORWARD %d -i %s %s %s -j %s", 
						lan_if, chk_type, follow_addr, TQ_CHAIN_NAME, find_rule_insert_pos(1), lan_if, chk_type, follow_addr, TQ_CHAIN_NAME);
					TQ_DBG("%s", rule);
				}
#endif
			}
		}
		free(lan_if);
		fclose(fp);
		system(TQ_RULE_SCRIPT_PATH);
	}
}
#endif

static void rearrange_list(json_object *profile_list, json_object *client_list, json_object *rule_persist_list, char *now_s) {
	FILE *fp = NULL;
	int lock;

	if (!profile_list || !client_list)
		return;

	char *lan_if = nvram_safe_get("lan_ifname");
	if ((fp=fopen(TQ_RULE_SCRIPT_PATH, "w"))==NULL) return;
	fprintf(fp, "#!/bin/sh\n");
	chmod(TQ_RULE_SCRIPT_PATH, 0777);

	if (!json_object_object_length(profile_list)) {
		json_object_object_foreach(client_list, cp_mac, cp_client) {
			json_object *jclient_flag = NULL;
			if (json_object_object_get_ex(cp_client, "flag", &jclient_flag) && 
				jclient_flag && 
				(json_object_get_int(jclient_flag) & TQ_FLAG_RULE_ADDED) > 0) { // this case should not happen.
				TQ_DBG("there are rules need to delete.");
#ifdef CLEAN_RULE_MODE
				g_fw_rules_update_needed = 1;
#else
				prepare_rule(fp, rule_persist_list, lan_if, cp_mac, TQ_FLAG_RULE_DEL_NEEDED, 0);
#endif
			}
			json_object_object_del(client_list, cp_mac);
		}
	} else {
		// Handle firewall rules and obsolete profile or client deletion.
		// First, check obsolete client.
		json_object_object_foreach(client_list, cp_mac, cp_client) {
			json_object *judpate_ts = NULL, *jclient_group = NULL;
			json_object *jclient_flag = NULL;

			json_object_object_get_ex(cp_client, "flag", &jclient_flag);
			// Check the update_ts and remove obsolete client
			if (json_object_object_get_ex(cp_client, "update_ts", &judpate_ts)) {
				char *update_ts = (char *) json_object_get_string(judpate_ts);
				if (update_ts && strcmp(update_ts, now_s)) {
					//json_object_object_del(client_list, cp_mac);
					if (jclient_flag && (json_object_get_int(jclient_flag) & TQ_FLAG_RULE_ADDED) > 0)
						json_object_object_add(cp_client, "flag", json_object_new_int(TQ_FLAG_RULE_ADDED | TQ_FLAG_OBJ_DEL_NEEDED));
					else
						json_object_object_add(cp_client, "flag", json_object_new_int(TQ_FLAG_OBJ_DEL_NEEDED));
				}
			}

			// Check the client that doesn't belong to any time quota profile
			if (json_object_object_get_ex(cp_client, "group_id", &jclient_group)) {
				json_object *jproile = NULL;
				if (jclient_group && !json_object_object_get_ex(profile_list, json_object_get_string(jclient_group), &jproile)) {
					//json_object_object_del(client_list, cp_mac);
					if (jclient_flag && (json_object_get_int(jclient_flag) & TQ_FLAG_RULE_ADDED) > 0)
						json_object_object_add(cp_client, "flag", json_object_new_int(TQ_FLAG_RULE_ADDED | TQ_FLAG_OBJ_DEL_NEEDED));
					else
						json_object_object_add(cp_client, "flag", json_object_new_int(TQ_FLAG_OBJ_DEL_NEEDED));
				}
			}
		}

		// Second, check profile and its client.
		json_object_object_foreach(profile_list, cp_group, cp_profile) {
			json_object *judpate_ts = NULL;
			json_object *jquota = NULL;
			json_object *jquota_used = NULL;
			json_object *jpflag = NULL;
			json_object *jgentle_pause = NULL;
			int profile_flag = 0, gentle_pause = 0;

			json_object_object_get_ex(cp_profile, "update_ts", &judpate_ts);
			json_object_object_get_ex(cp_profile, "quota", &jquota);
			json_object_object_get_ex(cp_profile, "quota_used", &jquota_used);

			// The priority is the following
			// 	1.	TQ_FLAG_OBJ_DEL_NEEDED
			//	2.	TQ_FLAG_RULE_DEL_NEEDED
			// 	3.	TQ_FLAG_RULE_ADD_NEEDED
			if (json_object_object_get_ex(cp_profile, "flag", &jpflag) && 
				json_object_get_int(jpflag) != TQ_FLAG_RULE_DEL_NEEDED) {
				// Handle quota exceeded
				if (jquota_used && jquota && json_object_get_int(jquota_used) >= json_object_get_int(jquota)) {
					//json_object_object_del(profile_list, cp_group);
					// TODO handle firewall rules clean.
					json_object_object_add(cp_profile, "flag", json_object_new_int(TQ_FLAG_RULE_ADD_NEEDED));
				}
			}

			// Check the update_ts for obsolete profile
			if (judpate_ts) {
				char *update_ts = (char *) json_object_get_string(judpate_ts);

				if (update_ts && strcmp(update_ts, now_s)) {
					//json_object_object_del(profile_list, cp_group);
					// TODO handle firewall rules clean.
					json_object_object_add(cp_profile, "flag", json_object_new_int(TQ_FLAG_OBJ_DEL_NEEDED));
				}
			}

			if (json_object_object_get_ex(cp_profile, "gentle_pause", &jgentle_pause))
				gentle_pause = json_object_get_int(jgentle_pause);

			if (json_object_object_get_ex(cp_profile, "flag", &jpflag))
				profile_flag = json_object_get_int(jpflag);

			// Find all clients which are in the group and perform respective action.
			json_object_object_foreach(client_list, cp_mac, cp_client) {
				json_object *jclient_group = NULL;
				json_object *jclient_flag = NULL;
				json_object_object_get_ex(cp_client, "group_id", &jclient_group);
				json_object_object_get_ex(cp_client, "flag", &jclient_flag);

				// Perform delete for obsolete client
				if (jclient_flag && (json_object_get_int(jclient_flag) & TQ_FLAG_OBJ_DEL_NEEDED) > 0) {
					if ((json_object_get_int(jclient_flag) & TQ_FLAG_RULE_ADDED) > 0) {
						TQ_DBG("%s need to delete1!!!!!", cp_mac);
#ifdef CLEAN_RULE_MODE
				g_fw_rules_update_needed = 1;
#else
						prepare_rule(fp, rule_persist_list, lan_if, cp_mac, TQ_FLAG_RULE_DEL_NEEDED, gentle_pause);
#endif
					}
					json_object_object_del(client_list, cp_mac);
					continue;
				}

				// The client in the group need to add the block rule.
				if (jclient_group &&
					!strcmp(cp_group, json_object_get_string(jclient_group))) {

					if (profile_flag == TQ_FLAG_OBJ_DEL_NEEDED) { // Perform rule deletion for client of profile which is obsolete.
						if ((json_object_get_int(jclient_flag) & TQ_FLAG_RULE_ADDED) > 0)
							TQ_DBG("%s need to delete2!!!!!", cp_mac);
#ifdef CLEAN_RULE_MODE
							g_fw_rules_update_needed = 1;
#else
							prepare_rule(fp, rule_persist_list, lan_if, cp_mac, TQ_FLAG_RULE_DEL_NEEDED, gentle_pause);
#endif
						json_object_object_del(client_list, cp_mac);
						continue;
					} else if (profile_flag == TQ_FLAG_RULE_ADD_NEEDED) { // Perform rule adding for client of profile which quota exceeded.
						if ((json_object_get_int(jclient_flag) & TQ_FLAG_RULE_ADDED) == 0) {
#ifdef CLEAN_RULE_MODE
							g_fw_rules_update_needed = 1;
#else
							prepare_rule(fp, rule_persist_list, lan_if, cp_mac, TQ_FLAG_RULE_ADD_NEEDED, gentle_pause);
#endif
							json_object_object_add(cp_client, "flag", json_object_new_int(TQ_FLAG_RULE_ADDED));
						}
					} else if (profile_flag == TQ_FLAG_RULE_DEL_NEEDED) { // Perform rule deletion for client of profile which quota reset.
#ifdef CLEAN_RULE_MODE
						g_fw_rules_update_needed = 1;
#else
						prepare_rule(fp, rule_persist_list, lan_if, cp_mac, TQ_FLAG_RULE_DEL_NEEDED, gentle_pause);
#endif
						json_object_object_add(cp_client, "flag", json_object_new_int(TQ_FLAG_NORMAL));
					}
				}
			}

			if (profile_flag == TQ_FLAG_RULE_ADD_NEEDED) {
				json_object_object_add(cp_profile, "flag", json_object_new_int(TQ_FLAG_RULE_ADDED));
			} else {
				if (profile_flag == TQ_FLAG_OBJ_DEL_NEEDED) // Perform rule deletion for profile which is obsolete.
					json_object_object_del(profile_list, cp_group);
				else // Reset flag
					json_object_object_add(cp_profile, "flag", json_object_new_int(TQ_FLAG_NORMAL));
			}
		}
	}
	fclose(fp);

	if (g_fw_rules_update_needed) {
#ifdef CLEAN_RULE_MODE
		apply_tq_rules(client_list);
#else
		system(TQ_RULE_SCRIPT_PATH);
		system("cp /tmp/tq_rules.sh /tmp/tq_rules_bak.sh");
		//if(nvram_get_int("time_quota_dbg"))

		lock = file_lock(TQ_RULE_PERSIST_LOCK);
		json_object_to_file(TQ_RULE_PERSIST_JSON_PATH, rule_persist_list);
		file_unlock(lock);
		g_fw_rules_update_needed = 0;
#endif
	}

}

int check_and_update_tq_profile_list(json_object *profile_list, json_object *client_list, json_object *rule_persist_list, char *now_s) {
	int lock;
	json_object *tmp_profile_list = NULL;
	json_object *profile = NULL;

	if (!profile_list || !client_list)
		return -1;

	check_and_update_tq_client_list(client_list, now_s);

	if (!json_object_object_length(client_list))
		return -1;

	tmp_profile_list = json_object_new_object();
	get_tq_profile_list(tmp_profile_list, now_s);
	json_object_object_foreach(tmp_profile_list, tmp_group, tmp_profile) {
		json_object *jquota1 = NULL, *jquota2 = NULL;
		json_object *jallowed_bg_traffic1 = NULL, *jallowed_bg_traffic2 = NULL;
		json_object *jgentle_pause1 = NULL/*, *jgentle_pause2 = NULL*/;

		// Skip empty quota, this means the profile is illegal.
		if (json_object_object_get_ex(tmp_profile, "quota", &jquota1) &&
			!json_object_get_int(jquota1)) {
			continue;
		}

		json_object_object_get_ex(tmp_profile, "allowed_bg_traffic", &jallowed_bg_traffic1);
		json_object_object_get_ex(tmp_profile, "gentle_pause", &jgentle_pause1);
		json_object_object_get_ex(profile_list, tmp_group, &profile);

		if (profile) { // old profile update check_ts
			json_object *jprev_update_ts;

			// Check if the quota and allowed_bg_traffic of profile is changed. If true, reset the sec_used
			if (jquota1 && 
				json_object_object_get_ex(profile, "quota", &jquota2) &&
				jallowed_bg_traffic1 && 
				json_object_object_get_ex(profile, "allowed_bg_traffic", &jallowed_bg_traffic2)) {
				if (json_object_get_int(jquota1) != json_object_get_int(jquota2) ||
					json_object_get_int(jallowed_bg_traffic1) != json_object_get_int(jallowed_bg_traffic2)) {
					json_object_object_add(profile, "quota", json_object_new_int(json_object_get_int(jquota1)));
					json_object_object_add(profile, "allowed_bg_traffic", json_object_new_int(json_object_get_int(jallowed_bg_traffic1)));
					json_object_object_add(profile, "quota_used", json_object_new_int(0));
					json_object_object_add(profile, "flag", json_object_new_int(TQ_FLAG_RULE_DEL_NEEDED));
				}
			}

			// Check if new day came. If true, reset the sec_used
			if (json_object_object_get_ex(profile, "update_ts", &jprev_update_ts) &&
				!is_same_local_day(TO_UL(now_s), TO_UL(json_object_get_string(jprev_update_ts)))) {
				json_object_object_add(profile, "quota_used", json_object_new_int(0));
				json_object_object_add(profile, "flag", json_object_new_int(TQ_FLAG_RULE_DEL_NEEDED));
			}

			// Update quota_used
			json_object *jquota_used;
			int quota_used = 0;

			if (json_object_object_get_ex(profile, "quota_used", &jquota_used))
				quota_used = (int)json_object_get_int(jquota_used);

			// Update client's sec used and profile's quota used.
			json_object_object_foreach(client_list, cp_mac, cp_client) {
				json_object *jgroup = NULL;
				if (json_object_object_get_ex(cp_client, "group_id", &jgroup) &&
					!strcmp(tmp_group, json_object_get_string(jgroup))) {
					json_object *jsec_used = NULL;
					int allowed_bg_traffic = 0;
					int sec_used = 0, accumulate_used = 0;

					if (json_object_object_get_ex(cp_client, "sec_used", &jsec_used) && jsec_used)
						accumulate_used = (int)json_object_get_int(jsec_used);

					if (jallowed_bg_traffic1)
						allowed_bg_traffic = json_object_get_int(jallowed_bg_traffic1);

					if (jprev_update_ts)
						sec_used += get_client_sec_used_from_db(cp_mac, allowed_bg_traffic, TO_UL(json_object_get_string(jprev_update_ts)), TO_UL(now_s));
					else
						sec_used += get_client_sec_used_from_db(cp_mac, allowed_bg_traffic, (TO_UL(now_s)-get_time_quota_interval()), TO_UL(now_s));

					// client's
					json_object_object_add(cp_client, "sec_used", json_object_new_int(accumulate_used+sec_used));

					quota_used += sec_used;
				}
			}
			// In the future, it may need to check if gentle_pause is changed
			if (jgentle_pause1)
				json_object_object_add(profile, "gentle_pause", json_object_new_int(json_object_get_int(jgentle_pause1)));
			json_object_object_add(profile, "quota_used", json_object_new_int(quota_used));

			// Update update_ts
			json_object_object_add(profile, "update_ts", json_object_new_string(now_s));
		} else { // new profile
			json_object *new_profile = json_object_new_object();

			if (jquota1)
				json_object_object_add(new_profile, "quota", json_object_new_int(json_object_get_int(jquota1)));
			if (jallowed_bg_traffic1)
				json_object_object_add(new_profile, "allowed_bg_traffic", json_object_new_int(json_object_get_int(jallowed_bg_traffic1)));
			if (jgentle_pause1)
				json_object_object_add(new_profile, "gentle_pause", json_object_new_int(json_object_get_int(jgentle_pause1)));
			json_object_object_add(new_profile, "quota_used", json_object_new_int(0));
			json_object_object_add(new_profile, "join_ts", json_object_new_string(now_s));
			json_object_object_add(new_profile, "update_ts", json_object_new_string(now_s));
			json_object_object_add(new_profile, "flag", json_object_new_int(TQ_FLAG_NORMAL));

			json_object_object_add(profile_list, tmp_group, new_profile);
		}
	}

	json_object_put(tmp_profile_list);

	rearrange_list(profile_list, client_list, rule_persist_list, now_s);


	lock = file_lock(TQ_PROFILE_LIST_LOCK);
	json_object_to_file(TQ_PROFILE_LIST_PATH, profile_list);
	file_unlock(lock);

	lock = file_lock(TQ_CLIENT_LIST_LOCK);
	json_object_to_file(TQ_CLIENT_LIST_PATH, client_list);
	file_unlock(lock);

	return 0;
}