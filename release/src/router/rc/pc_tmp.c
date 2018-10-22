#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bcmnvram.h>
#include <shutils.h>
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

pc_event_s *get_event_tmp_list(pc_event_s **target_list, char *target_string){
	pc_event_s **follow_e_list;
	char *ptr, *ptr_end, bak;

	if(target_list == NULL || target_string == NULL)
		return NULL;

	follow_e_list = target_list;

	if(initial_event(follow_e_list) == NULL){
		_dprintf("No memory!!(follow_e_list)\n");
		return NULL;
	}

	snprintf((*follow_e_list)->e_name, sizeof((*follow_e_list)->e_name), "%s", target_string);

	// start_day
	ptr = target_string;
	ptr_end = ptr+1;
	bak = ptr_end[0];
	ptr_end[0] = 0;
	(*follow_e_list)->start_day = atoi(ptr);
	ptr_end[0] = bak;

	// end_day
	ptr = target_string+1;
	if(ptr == NULL)
		return NULL;
	ptr_end = ptr+1;
	bak = ptr_end[0];
	ptr_end[0] = 0;
	(*follow_e_list)->end_day = atoi(ptr);
	ptr_end[0] = bak;

	// start_hour
	ptr = target_string+2;
	if(ptr == NULL)
		return NULL;
	ptr_end = ptr+2;
	bak = ptr_end[0];
	ptr_end[0] = 0;
	(*follow_e_list)->start_hour = atoi(ptr);
	ptr_end[0] = bak;

	// end_hour
	ptr = target_string+4;
	if(ptr == NULL)
		return NULL;
	ptr_end = ptr+2;
	bak = ptr_end[0];
	ptr_end[0] = 0;
	(*follow_e_list)->end_hour = atoi(ptr);
	ptr_end[0] = bak;

	// start_min
	ptr = target_string+6;
	if(ptr == NULL)
		return NULL;
	ptr_end = ptr+2;
	bak = ptr_end[0];
	ptr_end[0] = 0;
	(*follow_e_list)->start_min = atoi(ptr);
	ptr_end[0] = bak;

	// end_min
	ptr = target_string+8;
	if(ptr == NULL)
		return NULL;
	ptr_end = ptr+2;
	bak = ptr_end[0];
	ptr_end[0] = 0;
	(*follow_e_list)->end_min = atoi(ptr);
	ptr_end[0] = bak;

	if((*follow_e_list)->start_min >= 60){
		(*follow_e_list)->start_hour += 1;
		(*follow_e_list)->start_min -= 60;
	}

	if((*follow_e_list)->start_hour >= 24){
		(*follow_e_list)->start_day += 1;
		(*follow_e_list)->start_hour -= 24;
	}

	if((*follow_e_list)->start_day >= 7){
		(*follow_e_list)->start_day -= 7;
	}

	if((*follow_e_list)->end_min >= 60){
		(*follow_e_list)->end_hour += 1;
		(*follow_e_list)->end_min -= 60;
	}

	if((*follow_e_list)->end_hour >= 24){
		(*follow_e_list)->end_day += 1;
		(*follow_e_list)->end_hour -= 24;
	}

	if((*follow_e_list)->end_day >= 7){
		(*follow_e_list)->end_day -= 7;
	}

	return *target_list;
}


// ex. MULTIFILTER_TMP="1>Karen>FF:EE:CC:DD:BB:AA>0010232060>12345678<1>ASUS>00:11:22:33:44:55>5623235060<1>James>AA:BB:CC:DD:EE:FF>3623225060>22335577"
pc_s *get_all_pc_tmp_list(pc_s **pc_list){
	char *nvp, *nv, *b;
	char word[4096], *next_word;
	pc_s *follow_pc, **follow_pc_list;
	int count;

	if(pc_list == NULL)
		return NULL;

	nv = nvp = strdup(nvram_safe_get("MULTIFILTER_TMP"));
	if(!nv || strlen(nv) <= 0){
		_dprintf("Can't duplicate the nvram: MULTIFILTER_TMP.\n");
		return NULL;
	}

	follow_pc_list = pc_list;

	while((b = strsep(&nvp, "<")) != NULL){
		if(initial_pc(follow_pc_list) == NULL){
			_dprintf("No memory!!(follow_pc_list)\n");
			return NULL;
		}

		follow_pc = *follow_pc_list;

		count = 0;
		foreach_62(word, b, next_word){
			switch(count){
				case 0: // enabled
					if(strlen(word) > 0)
						follow_pc->enabled = atoi(word);
					else
						follow_pc->enabled = 1;

					break;
				case 1: // device
					snprintf(follow_pc->device, sizeof(follow_pc->device), "%s", word);

					break;

				case 2: // mac
					snprintf(follow_pc->mac, sizeof(follow_pc->mac), "%s", word);

					break;
				case 3: // events
					get_event_tmp_list(&(follow_pc->events), word);

					break;
				case 4: // timestamp
					follow_pc->timestamp = strtoull(word, NULL, 10);

					break;
			}

			++count;
		}

		while((*follow_pc_list) != NULL){
			follow_pc_list = &((*follow_pc_list)->next);
		}
	}

	return *pc_list;
}

int pc_tmp_main(int argc, char *argv[]){
	pc_s *pc_list = NULL, *enabled_list = NULL, *daytime_list = NULL;

	get_all_pc_tmp_list(&pc_list);

	if(argc == 1 || (argc == 2 && !strcmp(argv[1], "show"))){
		print_pc_list(pc_list);
	}
	else if(argc == 2 && !strcmp(argv[1], "count")){
		_dprintf("%d\n", count_pc_rules(pc_list, 1));
	}
	else if((argc == 2 && !strcmp(argv[1], "enabled"))
			|| (argc == 3 && !strcmp(argv[1], "enabled") && (!strcmp(argv[2], "0") || !strcmp(argv[2], "1")))){
		if(argc == 2)
			match_enabled_pc_list(pc_list, &enabled_list, 1);
		else
			match_enabled_pc_list(pc_list, &enabled_list, atoi(argv[2]));

		print_pc_list(enabled_list);

		free_pc_list(&enabled_list);
	}
	else if(argc == 4 && !strcmp(argv[1], "daytime")
			&& (atoi(argv[2]) >= MIN_DAY && atoi(argv[2]) <= MAX_DAY)
			&& (atoi(argv[3]) >= MIN_HOUR && atoi(argv[3]) <= MAX_HOUR)
			){
		match_daytime_pc_list(pc_list, &daytime_list, atoi(argv[2]), atoi(argv[3]));

		print_pc_list(daytime_list);

		free_pc_list(&daytime_list);
	}
	else if(argc == 2 && !strcmp(argv[1], "apply")){
		char prefix[]="wanXXXXXX_", tmp[100];
		int wan_unit = wan_primary_ifunit();
		snprintf(prefix, sizeof(prefix), "wan%d_", wan_unit);

		char *wan_if = get_wan_ifname(wan_unit);
		char *wan_ip = nvram_safe_get(strcat_r(prefix, "ipaddr", tmp));
		char *lan_if = nvram_safe_get("lan_ifname");
		char *lan_ip = nvram_safe_get("lan_ipaddr");
		char logaccept[32], logdrop[32];

		if(nvram_match("fw_log_x", "accept") || nvram_match("fw_log_x", "both"))
			strlcpy(logaccept, "logaccept", sizeof(logaccept));
		else
			strlcpy(logaccept, "ACCEPT", sizeof(logaccept));
		if(nvram_match("fw_log_x", "drop") || nvram_match("fw_log_x", "both"))
			strlcpy(logdrop, "logdrop", sizeof(logdrop));
		else
			strlcpy(logdrop, "DROP", sizeof(logdrop));

		match_enabled_pc_list(pc_list, &enabled_list, 1);

		filter_setting(wan_if, wan_ip, lan_if, lan_ip, logaccept, logdrop);

		free_pc_list(&enabled_list);
	}
	else if(argc == 2 && !strcmp(argv[1], "showrules")){
		config_daytime_string(pc_list, stderr, "ACCEPT", "logdrop", 1);
	}
	else{
		printf("Usage: pc [show]\n"
		       "          showrules\n"
		       "          enabled [1 | 0]\n"
		       "          daytime [1-7] [0-23]\n"
		       "          apply\n"
		       );
	}

	free_pc_list(&pc_list);

	return 0;
}

