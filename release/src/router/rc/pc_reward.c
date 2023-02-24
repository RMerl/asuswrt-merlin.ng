#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <time.h>
#include "rc.h"


#if 0
#define _dprintf(fmt, args...) do{ \
		FILE *fp = fopen("/dev/console", "a+"); \
		if(fp){ \
			fprintf(fp, "[_dprintf: %s] ", __FUNCTION__); \
			fprintf(fp, fmt, ## args); \
			fclose(fp); \
		} \
	}while(0)
#endif

static void retreive_daytime_from_ts(const char *s_ts, pc_event_s *pc_event) {
	char *ptr;
	struct tm *ptm;
	if (!pc_event)
		return;
	//_dprintf("retreive_daytime_from_ts s_ts=%s...1\n", s_ts);
	pc_event->utc = 0;
	pc_event->ts = (time_t)strtol(s_ts, &ptr, 10);
	//_dprintf("retreive_daytime_from_ts s_ts=%s, ptr=%s...2\n", s_ts, ptr);
	if (ptr && !strcasecmp(ptr, "Z"))
		pc_event->utc = 1;

	if (pc_event->utc)
		ptm = localtime(&pc_event->ts);
	else
		ptm = gmtime(&pc_event->ts); // It represents the timestamp is local time, no need to convert it.
	//_dprintf("retreive_daytime_from_ts s_ts=%s, utc=%d...3\n", s_ts, pc_event->utc);
	pc_event->end_year = ptm->tm_year + 1900;
	pc_event->end_mon = ptm->tm_mon + 1;
	pc_event->end_day = ptm->tm_mday;
	pc_event->end_hour = ptm->tm_hour;
	pc_event->end_min = ptm->tm_min;
	pc_event->end_sec = ptm->tm_sec;
}

static int match_daytime(pc_event_s *pc_event) {
	return pc_event->ts > time(NULL);
}

/*static int is_valid_pc_reward_events(pc_event_s *e_list) {
	pc_event_s *follow_e;

	if (!e_list)
		return 0;

	for(follow_e = e_list; follow_e != NULL; follow_e = follow_e->next){
		return match_daytime(follow_e);
	}
	return 0;
}*/
char *target_chains[] = {	"PControls"
#ifdef RTCONFIG_TIME_QUOTA
							, "TIME_QUOTA"
#endif
						};

static void clean_invalid_pc_reward(pc_s *pc_list) {

	pc_s *follow_pc;
	pc_event_s *follow_e;
	char *lan_if = nvram_safe_get("lan_ifname");
	int i;

	follow_pc = pc_list;
	if(follow_pc == NULL){
		_dprintf("Couldn't get the rules of Parental Control Reward correctly!\n");
		return;
	}

	for(follow_pc = pc_list; follow_pc != NULL; follow_pc = follow_pc->next){
		for(follow_e = follow_pc->events; follow_e  != NULL; follow_e = follow_e->next) {
			// If access type is block and the current time is not in the next event, skip these two events, 
			if (follow_e &&!match_daytime(follow_e)) {
				const char *chk_type;
				char follow_addr[18] = {0};
				char cmd_buf[512];

#ifdef RTCONFIG_AMAS
				if (strlen(follow_pc->mac) && amas_lib_device_ip_query(follow_pc->mac, follow_addr)) {
					chk_type = iptables_chk_ip;
				} else
#endif
				{
					chk_type = iptables_chk_mac;
					snprintf(follow_addr, sizeof(follow_addr), "%s", follow_pc->mac);
				}
				if(!follow_addr[0])
					chk_type = "";

				// filter table
				for (i=0; i<sizeof(target_chains)/sizeof(char *); i++) {
					//_dprintf("The event of Parental Control Reward is out of the day. clean it.\n");
					snprintf(cmd_buf, sizeof(cmd_buf), "iptables -D %s -i %s -m time --kerneltz --datestop %d-%d-%dT%d:%d:%d %s %s -j RETURN", 
						target_chains[i], lan_if, follow_e->end_year, follow_e->end_mon, follow_e->end_day, follow_e->end_hour, 
						follow_e->end_min, follow_e->end_sec, chk_type, follow_addr);
					system(cmd_buf);
					// If the rule is ip base, also try to delete rule by mac.
					if (!strcmp(chk_type, iptables_chk_ip)) {
						snprintf(cmd_buf, sizeof(cmd_buf), "iptables -D %s -i %s -m time --kerneltz --datestop %d-%d-%dT%d:%d:%d %s %s -j RETURN", 
							target_chains[i], lan_if, follow_e->end_year, follow_e->end_mon, follow_e->end_day, follow_e->end_hour, 
							follow_e->end_min, follow_e->end_sec, iptables_chk_mac, follow_pc->mac);
						system(cmd_buf);
					}
				}
				// nat table
				snprintf(cmd_buf, sizeof(cmd_buf), "iptables -t nat -D PCREDIRECT -i %s -m time --kerneltz --datestop %d-%d-%dT%d:%d:%d %s %s -j RETURN", 
					lan_if, follow_e->end_year, follow_e->end_mon, follow_e->end_day, follow_e->end_hour, 
					follow_e->end_min, follow_e->end_sec, chk_type, follow_addr);
				system(cmd_buf);
				// If the rule is ip base, also try to delete rule by mac.
				if (!strcmp(chk_type, iptables_chk_ip)) {
					snprintf(cmd_buf, sizeof(cmd_buf), "iptables -t nat -D PCREDIRECT -i %s -m time --kerneltz --datestop %d-%d-%dT%d:%d:%d %s %s -j RETURN", 
						lan_if, follow_e->end_year, follow_e->end_mon, follow_e->end_day, follow_e->end_hour, 
						follow_e->end_min, follow_e->end_sec, iptables_chk_mac, follow_pc->mac);
					system(cmd_buf);
				}
			}
		}
	}
}

// ex. MULTIFILTER_TMP="1>Karen>FF:EE:CC:DD:BB:AA>0010232060>12345678<1>ASUS>00:11:22:33:44:55>5623235060<1>James>AA:BB:CC:DD:EE:FF>3623225060>22335577"
pc_s *get_all_pc_reward_list(pc_s **pc_list){
	char *nvp, *nv, *b;
	char word[4096], *next_word;
	pc_s *follow_pc, **follow_pc_list;
	int count;

	if(pc_list == NULL)
		return NULL;

	nvp = nvram_safe_get("MULTIFILTER_REWARD");
	if (*nvp == '\0')
		return NULL;

	nv = nvp = strdup(nvp);
	if (!nv) {
		_dprintf("Can't duplicate the nvram: MULTIFILTER_REWARD.\n");
		return NULL;
	}

	follow_pc_list = pc_list;

	while((b = strsep(&nvp, ">")) != NULL){
		if(initial_pc(follow_pc_list) == NULL){
			_dprintf("No memory!!(follow_pc_reward_list)\n");
			free(nv);
			return NULL;
		}

		follow_pc = *follow_pc_list;

		count = 0;
		foreach_60(word, b, next_word){
			switch(count){
				case 0: // mac
					snprintf(follow_pc->mac, sizeof(follow_pc->mac), "%s", word);
					//_dprintf("follow_pc->mac=%s\n", follow_pc->mac);
					break;
				case 1: // timestamp
					if (strlen(word) > 2 && word[0] == 'T') {
						if(!follow_pc->events)
							initial_event(&follow_pc->events);

						follow_pc->events->access = (word[1] - '0');
						//_dprintf("follow_pc->events->access=%d\n", follow_pc->events->access);
						retreive_daytime_from_ts(&word[2], follow_pc->events);
						follow_pc->enabled = (follow_pc->events->access && 
												match_daytime(follow_pc->events) &&
												(strlen(follow_pc->mac) > 0));
						//_dprintf("follow_pc->enabled=%d\n", follow_pc->enabled);
					} else {
						++count;
						continue;
					}

					break;
			}

			++count;
		}

		while((*follow_pc_list) != NULL){
			follow_pc_list = &((*follow_pc_list)->next);
		}
	}

	free(nv);
	return *pc_list;
}

void config_pc_reward_string(pc_s *pc_list, FILE *fp){

	pc_s *follow_pc;
	pc_event_s *follow_e;
	char *lan_if = nvram_safe_get("lan_ifname");

	follow_pc = pc_list;
	if(follow_pc == NULL){
		_dprintf("Couldn't get the rules of Parental Control Reward correctly!\n");
		return;
	}

	for(follow_pc = pc_list; follow_pc != NULL; follow_pc = follow_pc->next){
		const char *chk_type;
		char follow_addr[18] = {0};
		if (!follow_pc->enabled)
			continue;

#ifdef RTCONFIG_AMAS
		if (strlen(follow_pc->mac) && amas_lib_device_ip_query(follow_pc->mac, follow_addr)) {
			chk_type = iptables_chk_ip;
		} else
#endif
		{
			chk_type = iptables_chk_mac;
			snprintf(follow_addr, sizeof(follow_addr), "%s", follow_pc->mac);
		}
		if(!follow_addr[0])
			chk_type = "";

//_dprintf("[PC] mac=%s\n", follow_pc->mac);
#ifdef RTCONFIG_PERMISSION_MANAGEMENT
		if (!strcmp(follow_pc->mac, "")) continue;
#endif

		for(follow_e = follow_pc->events; follow_e != NULL; follow_e = follow_e->next) {

			// If access type is block and the current time is not in the next event, skip these two events, 
			if (!match_daytime(follow_e)) {
				_dprintf("The event of Parental Control Reward is out of the day.\n");
				continue;
			}

			// check the next follow event has differen access. if true treat it as a period event. Set datestop and the reverse rule for the unmatched time.
			if (follow_e) {
				int i;
				for (i=0; i<sizeof(target_chains)/sizeof(char *); i++) {
					fprintf(fp, "-A %s -i %s -m time", target_chains[i], lan_if);
					fprintf(fp, " --kerneltz --datestop %d-%d-%dT%d:%d:%d", follow_e->end_year, follow_e->end_mon, follow_e->end_day, 
						follow_e->end_hour, follow_e->end_min, follow_e->end_sec);
					fprintf(fp, " %s %s -j RETURN\n", chk_type, follow_addr);
				}
			}
		}
	}

	//clean_invalid_pc_reward(pc_list);
}

void config_pc_reward_redirect(FILE *fp){
	pc_s *pc_reward_list = NULL, *enabled_list = NULL, *follow_pc;
	pc_event_s *follow_e;
	char *lan_if = nvram_safe_get("lan_ifname");
#ifndef RTCONFIG_PC_SCHED_V3
	char *datestr[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
	int i;
#endif

	follow_pc = get_all_pc_reward_list(&pc_reward_list);
	if(follow_pc == NULL){
		_dprintf("Couldn't get the Parental-control rules correctly!\n");
		return;
	}

	follow_pc = match_enabled_pc_list(pc_reward_list, &enabled_list, 1);
	free_pc_list(&pc_reward_list);
	if(follow_pc == NULL){
		_dprintf("Couldn't get the enabled rules of Parental-control Reward correctly!\n");
		return;
	}

	for(follow_pc = enabled_list; follow_pc != NULL; follow_pc = follow_pc->next){
		const char *chk_type;
		char follow_addr[18] = {0};
#ifdef RTCONFIG_AMAS
		_dprintf("config_pc_reward_redirect\n");
		if (strlen(follow_pc->mac) && amas_lib_device_ip_query(follow_pc->mac, follow_addr)) {
			chk_type = iptables_chk_ip;
		} else
#endif
		{
			chk_type = iptables_chk_mac;
			snprintf(follow_addr, sizeof(follow_addr), "%s", follow_pc->mac);
		}

#ifdef RTCONFIG_PERMISSION_MANAGEMENT
		if (!strcmp(follow_pc->mac, "")) continue;
#endif

		for(follow_e = follow_pc->events; follow_e != NULL; follow_e = follow_e->next){
			fprintf(fp, "-A PCREDIRECT -i %s -m time", lan_if);
			fprintf(fp, " --kerneltz --datestop %d-%d-%dT%d:%d:%d", follow_e->end_year, follow_e->end_mon, follow_e->end_day, 
				follow_e->end_hour, follow_e->end_min, follow_e->end_sec);
			fprintf(fp, " %s %s -j RETURN\n", chk_type, follow_addr);
		}
	}

	free_pc_list(&enabled_list);
}

int is_in_pc_reward_period(char *mac) {
	pc_s *pc_reward_list = NULL, *follow_pc;
	int in_period = 0;
	if (!mac)
		return in_period;

	get_all_pc_reward_list(&pc_reward_list);

	for(follow_pc = pc_reward_list; follow_pc != NULL; follow_pc = follow_pc->next){
		if (nvram_get_int("pcdbg"))
			_dprintf("is_in_pc_reward_period in_mac=%s, mac=%s, enabled=%d\n", mac, follow_pc->mac, follow_pc->enabled);
		if (!strcmp(follow_pc->mac , mac) && follow_pc->enabled) {
			in_period = 1;
			break;
		}
	}

	free_pc_list(&pc_reward_list);
	return in_period;
}

int pc_reward_main(int argc, char *argv[]){
	pc_s *pc_list = NULL;

	get_all_pc_reward_list(&pc_list);

	if(argc == 2 && !strcmp(argv[1], "clean")){
		clean_invalid_pc_reward(pc_list);
	}
	else{
		printf("Usage: pc clean\n");
	}

	free_pc_list(&pc_list);

	return 0;
}

