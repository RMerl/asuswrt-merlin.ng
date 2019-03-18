#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bcmnvram.h>
#include <shutils.h>
#include "rc.h"

#ifdef RTCONFIG_PERMISSION_MANAGEMENT
#include <PMS_DBAPIs.h>
#endif

//#define BLOCKLOCAL
char *datestr[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

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

pc_event_s *initial_event(pc_event_s **target_e){
	pc_event_s *tmp_e;

	if(target_e == NULL)
		return NULL;

	*target_e = (pc_event_s *)malloc(sizeof(pc_event_s));
	if(*target_e == NULL)
		return NULL;

	tmp_e = *target_e;

	memset(tmp_e->e_name, 0, 32);
	tmp_e->start_day = 0;
	tmp_e->end_day = 0;
	tmp_e->start_hour = 0;
	tmp_e->end_hour = 0;
	tmp_e->start_min = 0;
	tmp_e->end_min = 0;
	tmp_e->next = NULL;

	return tmp_e;
}

void free_event_list(pc_event_s **target_list){
	pc_event_s *tmp_e, *old_e;

	if(target_list == NULL)
		return;

	tmp_e = *target_list;
	while(tmp_e != NULL){
		old_e = tmp_e;
		tmp_e = tmp_e->next;
		free(old_e);
	}

	return;
}

pc_event_s *get_event_list(pc_event_s **target_list, char *target_string){
	char word[4096], *next_word;
	pc_event_s **follow_e_list;
	int i, n_t;
	char *ptr, *ptr_end, bak;

	if(target_list == NULL || target_string == NULL)
		return NULL;

	follow_e_list = target_list;
	n_t = 1;
	foreach_60(word, target_string, next_word){
		if(n_t){
			if(initial_event(follow_e_list) == NULL){
				_dprintf("No memory!!(follow_e_list)\n");
				continue;
			}

			++i;
			n_t = 0;

			strlcpy((*follow_e_list)->e_name, word, 32);
		}
		else{
			n_t = 1;

			// start_day
			ptr = word;
			ptr_end = ptr+1;
			bak = ptr_end[0];
			ptr_end[0] = 0;
			(*follow_e_list)->start_day = atoi(ptr);
			ptr_end[0] = bak;

			// end_day
			ptr = word+1;
			ptr_end = ptr+1;
			bak = ptr_end[0];
			ptr_end[0] = 0;
			(*follow_e_list)->end_day = atoi(ptr);
			ptr_end[0] = bak;

			// start_hour
			ptr = word+2;
			ptr_end = ptr+2;
			bak = ptr_end[0];
			ptr_end[0] = 0;
			(*follow_e_list)->start_hour = atoi(ptr);
			ptr_end[0] = bak;

			// end_hour
			ptr = word+4;
			ptr_end = ptr+2;
			bak = ptr_end[0];
			ptr_end[0] = 0;
			(*follow_e_list)->end_hour = atoi(ptr);
			ptr_end[0] = bak;

			if((*follow_e_list)->start_hour >= 24){
				(*follow_e_list)->start_day += 1;
				(*follow_e_list)->start_hour -= 24;
			}

			if((*follow_e_list)->start_day >= 7){
				(*follow_e_list)->start_day -= 7;
			}

			if((*follow_e_list)->end_hour >= 24){
				(*follow_e_list)->end_day += 1;
				(*follow_e_list)->end_hour -= 24;
			}

			if((*follow_e_list)->end_day >= 7){
				(*follow_e_list)->end_day -= 7;
			}

			while(*follow_e_list != NULL)
				follow_e_list = &((*follow_e_list)->next);
		}
	}

	return *target_list;
}

pc_event_s *cp_event(pc_event_s **dest, const pc_event_s *src){
	if(initial_event(dest) == NULL){
		_dprintf("No memory!!(dest)\n");
		return NULL;
	}

	strlcpy((*dest)->e_name, src->e_name, sizeof((*dest)->e_name));
	(*dest)->start_day = src->start_day;
	(*dest)->end_day = src->end_day;
	(*dest)->start_hour = src->start_hour;
	(*dest)->end_hour = src->end_hour;
	(*dest)->start_min = src->start_min;
	(*dest)->end_min = src->end_min;

	return *dest;
}

void print_event_list(pc_event_s *e_list){
	pc_event_s *follow_e;
	int i;

	if(e_list == NULL)
		return;

	i = 0;
	for(follow_e = e_list; follow_e != NULL; follow_e = follow_e->next){
		++i;
		_dprintf("   %3dth event:\n", i);
		_dprintf("        e_name: %s.\n", follow_e->e_name);
		_dprintf("         start: %2d:%2d on %s.\n", follow_e->start_hour, follow_e->start_min, datestr[follow_e->start_day]);
		_dprintf("           end: %2d:%2d on %s.\n", follow_e->end_hour, follow_e->end_min, datestr[follow_e->end_day]);
		if(follow_e->next != NULL)
			_dprintf("------------------------------\n");
	}
}

pc_s *initial_pc(pc_s **target_pc){
	pc_s *tmp_pc;

	if(target_pc == NULL)
		return NULL;

	*target_pc = (pc_s *)malloc(sizeof(pc_s));
	if(*target_pc == NULL)
		return NULL;

	tmp_pc = *target_pc;

	tmp_pc->enabled = 0;
	tmp_pc->state = INITIAL;
	tmp_pc->prev_state = INITIAL;
	tmp_pc->dtimes = nvram_get_int("questcf")?:0;
	memset(tmp_pc->device, 0, 32);
	memset(tmp_pc->mac, 0, 18);
	tmp_pc->events = NULL;
	tmp_pc->timestamp = 0;
	tmp_pc->next = NULL;

	return tmp_pc;
}

void free_pc_list(pc_s **target_list){
	pc_s *tmp_pc, *old_pc;

	if(target_list == NULL)
		return;

	tmp_pc = *target_list;
	while(tmp_pc != NULL){
		free_event_list(&(tmp_pc->events));

		old_pc = tmp_pc;
		tmp_pc = tmp_pc->next;
		free(old_pc);
	}

	return;
}

pc_s *cp_pc(pc_s **dest, const pc_s *src){
	pc_event_s *follow_e, **follow_e_list;

	if(initial_pc(dest) == NULL){
		_dprintf("No memory!!(dest)\n");
		return NULL;
	}

	(*dest)->enabled = src->enabled;
	strlcpy((*dest)->device, src->device, sizeof((*dest)->device));
	strlcpy((*dest)->mac, src->mac, sizeof((*dest)->mac));

	follow_e_list = &((*dest)->events);
	for(follow_e = src->events; follow_e != NULL; follow_e = follow_e->next){
		cp_event(follow_e_list, follow_e);

		while(*follow_e_list != NULL)
			follow_e_list = &((*follow_e_list)->next);
	}

	return *dest;
}

#ifdef RTCONFIG_PERMISSION_MANAGEMENT
/*
	permission management copy the origin rule into separate mac
	dev group use "src->device" and strip '@'
*/
static
pc_s *dup_pc_with_mac(pc_s **dest, const pc_s *src, const char *mac){
	pc_event_s *follow_e, **follow_e_list;

	if(initial_pc(dest) == NULL){
		_dprintf("No memory!!(dest)\n");
		return NULL;
	}

	(*dest)->enabled = src->enabled;
	strcpy((*dest)->device, src->device+1);   // strip '@'
	strcpy((*dest)->mac, mac);                // new mac from permission group

	follow_e_list = &((*dest)->events);
	for(follow_e = src->events; follow_e != NULL; follow_e = follow_e->next){
		cp_event(follow_e_list, follow_e);

		while(*follow_e_list != NULL)
			follow_e_list = &((*follow_e_list)->next);
	}

	return *dest;
}
#endif

pc_s *get_all_pc_list(pc_s **pc_list){
	char word[4096], *next_word;
	pc_s *follow_pc, **follow_pc_list;
	int i;

	if(pc_list == NULL)
		return NULL;

	follow_pc_list = pc_list;
	foreach_62(word, nvram_safe_get("MULTIFILTER_ENABLE"), next_word){
		if(initial_pc(follow_pc_list) == NULL){
			_dprintf("No memory!!(follow_pc_list)\n");
			continue;
		}

		if(strlen(word) > 0)
			(*follow_pc_list)->enabled = atoi(word);

		while(*follow_pc_list != NULL)
			follow_pc_list = &((*follow_pc_list)->next);
	}

	follow_pc = *pc_list;
	i = 0;
	foreach_62(word, nvram_safe_get("MULTIFILTER_DEVICENAME"), next_word){
		++i;
		if(follow_pc == NULL){
			_dprintf("*** %3dth Parental Control rule(DEVICENAME) had something wrong!\n", i);
			return *pc_list;
		}

		strlcpy(follow_pc->device, word, 32);

		follow_pc = follow_pc->next;
	}

	follow_pc = *pc_list;
	i = 0;
	foreach_62(word, nvram_safe_get("MULTIFILTER_MAC"), next_word){
		++i;
		if(follow_pc == NULL){
			_dprintf("*** %3dth Parental Control rule(MAC) had something wrong!\n", i);
			return *pc_list;
		}

		strlcpy(follow_pc->mac, word, 18);

		follow_pc = follow_pc->next;
	}

	follow_pc = *pc_list;
	i = 0;
	foreach_62(word, nvram_safe_get("MULTIFILTER_MACFILTER_DAYTIME"), next_word){
		++i;
		if(follow_pc == NULL){
			_dprintf("*** %3dth Parental Control rule(DAYTIME) had something wrong!\n", i);
			return *pc_list;
		}

		get_event_list(&(follow_pc->events), word);

		follow_pc = follow_pc->next;
	}

#ifdef RTCONFIG_PERMISSION_MANAGEMENT
	follow_pc = *pc_list;
	i = 0;
	while (follow_pc != NULL)
	{
		i++;
		//printf("[PC][%d] %s, %s, %s\n", i, follow_pc->device, follow_pc->mac, follow_pc->device+1);
		if (follow_pc->device[0] == '@') {
			int dev_num, group_num;
			PMS_DEVICE_INFO_T *dev_list = NULL;
			PMS_DEVICE_GROUP_INFO_T *group_list = NULL, *follow_group = NULL;

			/* Get account / group list */
			if (PMS_GetDeviceInfo(PMS_ACTION_GET_FULL, &dev_list, &group_list, &dev_num, &group_num) < 0) {
				_dprintf("Can't read dev / group list\n");
				break;
			}

			/* Get the mac list of certain group */
			for (follow_group = group_list; follow_group != NULL; follow_group = follow_group->next) {
				if (!strcmp(follow_group->name, (follow_pc->device)+1)) {
					PMS_OWNED_INFO_T *owned_dev = follow_group->owned_device;
					while (owned_dev != NULL) {
						PMS_DEVICE_INFO_T *dev_owned = (PMS_DEVICE_INFO_T *) owned_dev->member;
						//printf("[PC][%s] %s\n", follow_group->name, dev_owned->mac);
						owned_dev = owned_dev->next;
						dup_pc_with_mac(follow_pc_list, follow_pc, dev_owned->mac);
						while (*follow_pc_list != NULL)
							follow_pc_list = &((*follow_pc_list)->next);
					}
				}
			}

			/* Free device and group list*/
			PMS_FreeDevInfo(&dev_list, &group_list);
		}
		follow_pc = follow_pc->next;
	}
#endif

	return *pc_list;
}

void print_pc_list(pc_s *pc_list){
	pc_s *follow_pc;
	int i;

	if(pc_list == NULL)
		return;

	i = 0;
	for(follow_pc = pc_list; follow_pc != NULL; follow_pc = follow_pc->next){
		++i;
		_dprintf("*** %3dth rule:\n", i);
		_dprintf("   enabled: %d.\n", follow_pc->enabled);
		_dprintf("    device: %s.\n", follow_pc->device);
		_dprintf("       mac: %s.\n", follow_pc->mac);
		print_event_list(follow_pc->events);
		_dprintf(" timestamp: %llu.\n", follow_pc->timestamp);
		_dprintf("******************************\n");
	}
}

#ifdef RTCONFIG_CONNTRACK
void flush_pc_list(pc_s *pc_list){
	pc_s *follow_pc;
	int i;

	if(pc_list == NULL)
		return;

	char tip[16];
	for(follow_pc = pc_list; follow_pc != NULL; follow_pc = follow_pc->next){
		if(follow_pc->enabled) {
			memset(tip, 0, sizeof(tip));
			if(arpcache(follow_pc->mac, tip)==0) {
				_dprintf("\n[pc flush] clean conntracks of %s\n", tip);
				eval("conntrack", "-D", "-s", tip);
			}
		}
	}
}
#endif

pc_s *match_enabled_pc_list(pc_s *pc_list, pc_s **target_list, int enabled){
	pc_s *follow_pc, **follow_target_list;

	if(pc_list == NULL || target_list == NULL)
		return NULL;

	if(enabled != 0 && enabled != 1 && enabled != 2)
		return NULL;

	follow_target_list = target_list;
	for(follow_pc = pc_list; follow_pc != NULL; follow_pc = follow_pc->next){
		if(follow_pc->enabled == enabled){
			cp_pc(follow_target_list, follow_pc);

			while(*follow_target_list != NULL)
				follow_target_list = &((*follow_target_list)->next);
		}
	}

	return *target_list;
}

pc_s *match_day_pc_list(pc_s *pc_list, pc_s **target_list, int target_day){
	pc_s *follow_pc, **follow_target_list;
	pc_event_s *follow_e;

	if(pc_list == NULL || target_list == NULL)
		return NULL;

	if(target_day < MIN_DAY || target_day > MAX_DAY)
		return NULL;

	follow_target_list = target_list;
	for(follow_pc = pc_list; follow_pc != NULL; follow_pc = follow_pc->next){
		for(follow_e = follow_pc->events; follow_e != NULL; follow_e = follow_e->next){
			if(target_day >= follow_e->start_day && target_day <= follow_e->end_day){
				cp_pc(follow_target_list, follow_pc);

				while(*follow_target_list != NULL)
					follow_target_list = &((*follow_target_list)->next);

				break;
			}
		}
	}

	return *target_list;
}

#ifdef RTCONFIG_CONNTRACK
int cleantrack_daytime_pc_list(pc_s *pc_list, int target_day, int target_hour, int verb){
	pc_s *follow_pc;
	pc_event_s *follow_e;
	int target_num, com_start, com_end;
	int fcf = nvram_get_int("forcedcf")? : 0;	/* force delete pclist conntracks */
	
	if(pc_list == NULL)
		return -1;

	if(target_day < MIN_DAY || target_day > MAX_DAY)
		return -1;

	if(target_hour < MIN_HOUR || target_hour > MAX_HOUR)
		return -1;

	target_num = target_day*24+target_hour;

	for(follow_pc = pc_list; follow_pc != NULL; follow_pc = follow_pc->next){
		if(!follow_pc->enabled)
			continue;

		follow_pc->prev_state = follow_pc->state;
		for(follow_e = follow_pc->events; follow_e != NULL; follow_e = follow_e->next){
			com_start = follow_e->start_day*24+follow_e->start_hour;
			com_end = follow_e->end_day*24+follow_e->end_hour;

			follow_pc->state = BLOCKED;
			if(target_num >= com_start && target_num < com_end){ /* in allowed zone */
				follow_pc->state = NONBLOCK;
				follow_pc->dtimes = nvram_get_int("questcf")?:0;
				break;
			}
		}
		
		if(verb) {
			_dprintf("\nCHK [%s] pc pre/now state:[%d][%d], dtimes=%d, fcf=%d\n", follow_pc->mac, follow_pc->prev_state, follow_pc->state, follow_pc->dtimes, fcf);
			_dprintf("now_day/hr:%d/%d\n", target_day, target_hour);
		}
		/* denial zone critical zone */
		if(((follow_pc->prev_state==NONBLOCK||follow_pc->prev_state==INITIAL) && follow_pc->state==BLOCKED) ||
		   (follow_pc->prev_state==DTIME) ||
		   fcf ) {
			char tip[16];
			if(verb)
				_dprintf("\n[pc] (%d)change to a denial zone [%s]\n", fcf, follow_pc->mac);
			/* go clean denial-mac's conntracks */
			if(arpcache(follow_pc->mac, tip)==0) {
				_dprintf("\n[pc] delete conntracks of %s\n", tip);
				eval("conntrack", "-D", "-s", tip);
			}
#ifdef HND_ROUTER
			eval("fc", "flush");
#elif RTCONFIG_BCMARM
			/* TBD. ctf ipct entries cleanup. */
#endif
			if(follow_pc->dtimes-- > 0) 
				follow_pc->state = DTIME;
			else
				follow_pc->state = BLOCKED;
		}
	}

	return 0;
}
#endif

pc_s *match_daytime_pc_list(pc_s *pc_list, pc_s **target_list, int target_day, int target_hour){
	pc_s *follow_pc, **follow_target_list;
	pc_event_s *follow_e;
	int target_num, com_start, com_end;

	if(pc_list == NULL || target_list == NULL)
		return NULL;

	if(target_day < MIN_DAY || target_day > MAX_DAY)
		return NULL;

	if(target_hour < MIN_HOUR || target_hour > MAX_HOUR)
		return NULL;

	target_num = target_day*24+target_hour;

	follow_target_list = target_list;
	for(follow_pc = pc_list; follow_pc != NULL; follow_pc = follow_pc->next){
		for(follow_e = follow_pc->events; follow_e != NULL; follow_e = follow_e->next){
			com_start = follow_e->start_day*24+follow_e->start_hour;
			com_end = follow_e->end_day*24+follow_e->end_hour;

			if(target_num >= com_start && target_num < com_end){ // exclude the end of daytime.
				cp_pc(follow_target_list, follow_pc);

				while(*follow_target_list != NULL)
					follow_target_list = &((*follow_target_list)->next);

				break;
			}
		}
	}

	return *target_list;
}

// Parental Control:
// MAC address not in list -> ACCEPT.
// MAC address in list and in time period -> ACCEPT.
// MAC address in list and not in time period -> DROP.
void config_daytime_string(pc_s *pc_list, FILE *fp, char *logaccept, char *logdrop, int temp){

	pc_s *enabled_list = NULL, *follow_pc;
	pc_event_s *follow_e;
	char *lan_if = nvram_safe_get("lan_ifname");
	int i;
#ifdef BLOCKLOCAL
	char *ftype;
#endif
	char *fftype;

#ifdef BLOCKLOCAL
	ftype = logaccept;
#endif
	fftype = "PControls";

	follow_pc = match_enabled_pc_list(pc_list, &enabled_list, 1);
	if(follow_pc == NULL){
		_dprintf("Couldn't get the enabled rules of Parental-control correctly!\n");
		return;
	}

	for(follow_pc = enabled_list; follow_pc != NULL; follow_pc = follow_pc->next){
		const char *chk_type;
		char follow_addr[18] = {0};
#ifdef RTCONFIG_AMAS
		_dprintf("config_daytime_string\n");
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

		for(follow_e = follow_pc->events; follow_e != NULL; follow_e = follow_e->next){
			if(follow_e->start_day != follow_e->end_day && follow_e->end_day == 0)
				follow_e->end_day = 7;

			if(follow_e->start_day == follow_e->end_day){
				if(follow_e->start_hour == follow_e->end_hour && follow_e->start_min == follow_e->end_min){ // whole week.
#ifdef BLOCKLOCAL
					fprintf(fp, "-A FORWARD -i %s %s %s -j %s\n", lan_if, chk_type, follow_addr, ftype);
#endif
					fprintf(fp, "-A FORWARD -i %s %s %s -j %s\n", lan_if, chk_type, follow_addr, fftype);
				}
				else{
#ifdef BLOCKLOCAL
					fprintf(fp, "-A INPUT -i %s -m time", lan_if);
					if(follow_e->start_hour > 0 || follow_e->start_min > 0)
						fprintf(fp, " --timestart %d:%d", follow_e->start_hour, follow_e->start_min);
					if(follow_e->end_hour > 0 || follow_e->end_min > 0)
						fprintf(fp, " --timestop %d:%d", follow_e->end_hour, follow_e->end_min);
					fprintf(fp, DAYS_PARAM "%s %s %s -j %s\n", datestr[follow_e->start_day], chk_type, follow_addr, ftype);
#endif
					fprintf(fp, "-A FORWARD -i %s -m time", lan_if);
					if(follow_e->start_hour > 0 || follow_e->start_min > 0)
						fprintf(fp, " --timestart %d:%d", follow_e->start_hour, follow_e->start_min);
					if(follow_e->end_hour > 0 || follow_e->end_min > 0)
						fprintf(fp, " --timestop %d:%d", follow_e->end_hour, follow_e->end_min);
					fprintf(fp, DAYS_PARAM "%s %s %s -j %s\n", datestr[follow_e->start_day], chk_type, follow_addr, fftype);

					if(follow_e->start_hour > follow_e->end_hour){
#ifdef BLOCKLOCAL
						fprintf(fp, "-A INPUT -i %s -m time" DAYS_PARAM, lan_if);
						for(i = follow_e->start_day+1; i < follow_e->start_day+7; ++i)
							fprintf(fp, "%s%s", (i == follow_e->start_day+1)?"":",", datestr[i%7]);
						fprintf(fp, " %s %s -j %s\n", chk_type, follow_addr, ftype);
#endif

						fprintf(fp, "-A FORWARD -i %s -m time" DAYS_PARAM, lan_if);
						for(i = follow_e->start_day+1; i < follow_e->start_day+7; ++i)
							fprintf(fp, "%s%s", (i == follow_e->start_day+1)?"":",", datestr[i%7]);
						fprintf(fp, " %s %s -j %s\n", chk_type, follow_addr, fftype);
					}
				}
			}
			else if(follow_e->start_day < follow_e->end_day){
				// first interval.
#ifdef BLOCKLOCAL
				fprintf(fp, "-A INPUT -i %s -m time", lan_if);
				if(follow_e->start_hour > 0 || follow_e->start_min > 0)
					fprintf(fp, " --timestart %d:%d", follow_e->start_hour, follow_e->start_min);
				fprintf(fp, DAYS_PARAM "%s %s %s -j %s\n", datestr[follow_e->start_day], chk_type, follow_addr, ftype);
#endif
				fprintf(fp, "-A FORWARD -i %s -m time", lan_if);
				if(follow_e->start_hour > 0 || follow_e->start_min > 0)
					fprintf(fp, " --timestart %d:%d", follow_e->start_hour, follow_e->start_min);
				fprintf(fp, DAYS_PARAM "%s %s %s -j %s\n", datestr[follow_e->start_day], chk_type, follow_addr, fftype);

				// middle interval.
				if(follow_e->end_day-follow_e->start_day > 1){
#ifdef BLOCKLOCAL
					fprintf(fp, "-A INPUT -i %s -m time" DAYS_PARAM, lan_if);
					for(i = follow_e->start_day+1; i < follow_e->end_day; ++i)
						fprintf(fp, "%s%s", (i == follow_e->start_day+1)?"":",", datestr[i]);
					fprintf(fp, " %s %s -j %s\n", chk_type, follow_addr, ftype);
#endif

					fprintf(fp, "-A FORWARD -i %s -m time" DAYS_PARAM, lan_if);
					for(i = follow_e->start_day+1; i < follow_e->end_day; ++i)
						fprintf(fp, "%s%s", (i == follow_e->start_day+1)?"":",", datestr[i]);
					fprintf(fp, " %s %s -j %s\n", chk_type, follow_addr, fftype);
				}

				// end interval.
				if(follow_e->end_hour > 0 || follow_e->end_min > 0){
#ifdef BLOCKLOCAL
					fprintf(fp, "-A INPUT -i %s -m time", lan_if);

					fprintf(fp, " --timestop %d:%d", follow_e->end_hour, follow_e->end_min);
					fprintf(fp, DAYS_PARAM "%s %s %s -j %s\n", datestr[follow_e->end_day], chk_type, follow_addr, ftype);
#endif
					fprintf(fp, "-A FORWARD -i %s -m time", lan_if);
					fprintf(fp, " --timestop %d:%d", follow_e->end_hour, follow_e->end_min);
					fprintf(fp, DAYS_PARAM "%s %s %s -j %s\n", datestr[follow_e->end_day], chk_type, follow_addr, fftype);
				}
			}
			else
				; // Don't care "start_day > end_day".
		}

		// MAC address in list and not in time period -> DROP.
		if(!temp){
#ifdef BLOCKLOCAL
			fprintf(fp, "-A INPUT -i %s %s %s -j DROP\n", lan_if, chk_type, follow_addr);
#endif
			fprintf(fp, "-A FORWARD -i %s %s %s -j DROP\n", lan_if, chk_type, follow_addr);
		}
	}

	free_pc_list(&enabled_list);
}

void config_pause_block_string(pc_s *pc_list, FILE *fp, char *logaccept, char *logdrop, int temp){

	pc_s *enabled_list = NULL, *follow_pc;
	char *lan_if = nvram_safe_get("lan_ifname");
#ifdef BLOCKLOCAL
	char *ftype;

	ftype = logaccept;
#endif

	follow_pc = match_enabled_pc_list(pc_list, &enabled_list, 2);
	if(follow_pc == NULL){
		_dprintf("Couldn't get the pause rules of Parental-control correctly!\n");
		return;
	}

	for(follow_pc = enabled_list; follow_pc != NULL; follow_pc = follow_pc->next){
		const char *chk_mac = iptables_chk_mac;
		if(!follow_pc->mac[0])
			chk_mac = "";

//_dprintf("[PC] mac=%s\n", follow_pc->mac);
#ifdef RTCONFIG_PERMISSION_MANAGEMENT
		if (!strcmp(follow_pc->mac, "")) continue;
#endif
		// MAC address in list and not in time period -> DROP.
		if(!temp){
#ifdef BLOCKLOCAL
			fprintf(fp, "-A INPUT -i %s %s %s -j DROP\n", lan_if, chk_mac, follow_pc->mac);
#endif
			fprintf(fp, "-A FORWARD -i %s %s %s -j DROP\n", lan_if, chk_mac, follow_pc->mac);
		}
	}

	free_pc_list(&enabled_list);
}

int count_pc_rules(pc_s *pc_list, int enabled){
	pc_s *enabled_list = NULL, *follow_pc;
	int count;

	follow_pc = match_enabled_pc_list(pc_list, &enabled_list, enabled);
	if(follow_pc == NULL){
		_dprintf("Couldn't get the enabled rules of Parental-control correctly!\n");
		return 0;
	}

	for(count = 0, follow_pc = enabled_list; follow_pc != NULL; follow_pc = follow_pc->next)
		++count;

	free_pc_list(&enabled_list);

	return count;
}

int pc_main(int argc, char *argv[]){
	pc_s *pc_list = NULL, *enabled_list = NULL, *daytime_list = NULL;

	get_all_pc_list(&pc_list);

	if(argc == 1 || (argc == 2 && !strcmp(argv[1], "show"))){
		print_pc_list(pc_list);
	}
	else if((argc == 2 && !strcmp(argv[1], "enabled"))
			|| (argc == 3 && !strcmp(argv[1], "enabled") && (!strcmp(argv[2], "0") || !strcmp(argv[2], "1") || !strcmp(argv[2], "2")))){
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
		config_daytime_string(pc_list, stderr, "ACCEPT", "logdrop", 0);
	}
#ifdef RTCONFIG_CONNTRACK
	else if(argc == 2 && !strcmp(argv[1], "flush")){
		flush_pc_list(pc_list);
	}
#endif
	else{
		printf("Usage: pc [show]\n"
		       "          showrules\n"
		       "          enabled [1 | 0]\n"
		       "          daytime [1-7] [0-23]\n"
		       "          apply\n"
#ifdef RTCONFIG_CONNTRACK
		       "          flush\n"
#endif
		       );
	}

	free_pc_list(&pc_list);

	return 0;
}

