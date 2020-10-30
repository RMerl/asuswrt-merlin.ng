#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <time.h>
#include "rc.h"

#define iptables_chk_mac " -m mac --mac-source"
#define iptables_chk_ip " -s"

static void retreive_daytime_from_ts(const char *s_ts, ic_event_s *ic_event) {
	char *ptr;
	struct tm *ptm;
	if (!ic_event)
		return;
	//_dprintf("retreive_daytime_from_ts s_ts=%s...1\n", s_ts);
	ic_event->utc = 0;
	ic_event->ts = (time_t)strtol(s_ts, &ptr, 10);
	//_dprintf("retreive_daytime_from_ts s_ts=%s, ptr=%s...2\n", s_ts, ptr);
	if (ptr && !strcasecmp(ptr, "Z"))
		ic_event->utc = 1;

	if (ic_event->utc)
		ptm = localtime(&ic_event->ts);
	else
		ptm = gmtime(&ic_event->ts); // It represents the timestamp is local time, no need to convert it.
	//_dprintf("retreive_daytime_from_ts s_ts=%s, utc=%d...3\n", s_ts, ic_event->utc);
	ic_event->start_year = ptm->tm_year + 1900;
	ic_event->start_mon = ptm->tm_mon + 1;
	ic_event->start_day = ptm->tm_mday;
	ic_event->start_hour = ptm->tm_hour;
	ic_event->start_min = ptm->tm_min;
}

ic_event_s *initial_ic_event(ic_event_s **target_e){
	ic_event_s *tmp_e;

	if(target_e == NULL)
		return NULL;

	*target_e = (ic_event_s *)malloc(sizeof(ic_event_s));
	if(*target_e == NULL)
		return NULL;

	tmp_e = *target_e;

	memset(tmp_e->e_name, 0, 32);
	tmp_e->type=IC_TYPE_TIME;
	tmp_e->start_year = 0;
	tmp_e->end_year = 0;
	tmp_e->start_mon = 0;
	tmp_e->end_mon = 0;
	tmp_e->start_day = 0;
	tmp_e->end_day = 0;
	tmp_e->start_hour = 0;
	tmp_e->end_hour = 0;
	tmp_e->start_min = 0;
	tmp_e->end_min = 0;
	tmp_e->next = NULL;
	tmp_e->access = IC_ACCESS_BLOCKED;
	tmp_e->ts = 0;
	tmp_e->utc = 0;

	return tmp_e;
}

void free_ic_event_list(ic_event_s **target_list){
	ic_event_s *tmp_e, *old_e;

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

ic_event_s *get_ic_event_list(ic_event_s **target_list, char *target_string){
	char word[4096], *next_word;
	ic_event_s **follow_e_list;
	//int i, n_t;
	int e_day_size = 0, e_ptr_idx = 0;
	char *ptr, *ptr_end, bak;

	if(target_list == NULL || target_string == NULL)
		return NULL;

	follow_e_list = target_list;
	//n_t = 1;
	foreach_60(word, target_string, next_word) {
		e_ptr_idx = 0;
		if(initial_ic_event(follow_e_list) == NULL){
			_dprintf("No memory!!(follow_e_list)\n");
			continue;
		}

		// type
		ptr = word+e_ptr_idx;
		ptr_end = ptr+1;
		bak = ptr_end[0];
		ptr_end[0] = 0;
		strlcpy((*follow_e_list)->e_name, ptr, 32);
		if ((*follow_e_list)->e_name[0] == 'W')
			(*follow_e_list)->type = IC_TYPE_WEEK;
		ptr_end[0] = bak;
		e_ptr_idx += 1;

		// access
		ptr = word+e_ptr_idx;
		ptr_end = ptr+1;
		bak = ptr_end[0];
		ptr_end[0] = 0;
		(*follow_e_list)->access = atoi(ptr)==1 ? IC_ACCESS_ALLOWED : IC_ACCESS_BLOCKED;
		ptr_end[0] = bak;
		e_ptr_idx += 1;

		if ((*follow_e_list)->type == IC_TYPE_TIME) {
			// start_mon
			ptr = word+e_ptr_idx;
			///ptr_end = ptr+10;
			//bak = ptr_end[0];
			//ptr_end[0] = 0;
			retreive_daytime_from_ts(ptr, (*follow_e_list));
			//(*follow_e_list)->start_mon = atoi(ptr);
			//ptr_end[0] = bak;
			//e_ptr_idx += 10;
		} else {
			 if ((*follow_e_list)->type == IC_TYPE_WEEK) {
				e_day_size = 1;
				// start_day
				ptr = word+e_ptr_idx;
				ptr_end = ptr+e_day_size;
				bak = ptr_end[0];
				ptr_end[0] = 0;
				(*follow_e_list)->start_day = atoi(ptr);
				ptr_end[0] = bak;
				e_ptr_idx += e_day_size;
			}

			// start_hour
			ptr = word+e_ptr_idx;
			ptr_end = ptr+2;
			bak = ptr_end[0];
			ptr_end[0] = 0;
			(*follow_e_list)->start_hour = atoi(ptr);
			ptr_end[0] = bak;
			e_ptr_idx += 2;

			// start_min
			ptr = word+e_ptr_idx;
			ptr_end = ptr+2;
			bak = ptr_end[0];
			ptr_end[0] = 0;
			(*follow_e_list)->start_min = atoi(ptr);
			ptr_end[0] = bak;
			e_ptr_idx += 2;
		}

		while(*follow_e_list != NULL)
			follow_e_list = &((*follow_e_list)->next);
	}

	return *target_list;
}

ic_event_s *cp_ic_event(ic_event_s **dest, const ic_event_s *src){
	if(initial_ic_event(dest) == NULL){
		_dprintf("No memory!!(dest)\n");
		return NULL;
	}

	strlcpy((*dest)->e_name, src->e_name, sizeof((*dest)->e_name));
	(*dest)->type = src->type;
	(*dest)->start_year = src->start_year;
	(*dest)->end_year = src->end_year;
	(*dest)->start_mon = src->start_mon;
	(*dest)->end_mon = src->end_mon;
	(*dest)->start_day = src->start_day;
	(*dest)->end_day = src->end_day;
	(*dest)->start_hour = src->start_hour;
	(*dest)->end_hour = src->end_hour;
	(*dest)->start_min = src->start_min;
	(*dest)->end_min = src->end_min;
	(*dest)->access = src->access;
	(*dest)->ts = src->ts;
	(*dest)->utc = src->utc;

	return *dest;
}

static int match_daytime(ic_event_s *ic_event) {
	return ic_event->ts > time(NULL);
}

static time_t convert_to_utc(const time_t ts) {
	struct tm *putc_tm = gmtime(&ts);
	return mktime(putc_tm);
}

static time_t convert_to_local(const time_t ts) {
	struct tm *plocal_tm = localtime(&ts);
	return mktime(plocal_tm);
}

static int is_valid_valid_iptable_ts(time_t ts) {
	return 0 <= ts && ts < 2145888000L;
}

/*
   return value
                -1 : not a pair
                 0 : is invalid pair
                 1 : is valid pair

*/
static int is_valid_pair_ic_event(const ic_event_s *start_ic_event, const ic_event_s *end_ic_event) {
	time_t now = time(NULL);
	time_t start_ts, end_ts;

	if (!start_ic_event || !end_ic_event)
		return -1;

	start_ts = start_ic_event->utc ? start_ic_event->ts : convert_to_utc(start_ic_event->ts); 
	end_ts = end_ic_event->utc ? end_ic_event->ts : convert_to_utc(end_ic_event->ts);
	//_dprintf("     is_valid_pair_ic_event now=%ld\n", now);

	// Treat as not a pair.
	if (start_ic_event->access == end_ic_event->access)
		return -1;

	if (end_ic_event->access == IC_ACCESS_ALLOWED && now > end_ts)
		return 0;
	else
		return 1;
}

static int is_valid_ic_events(ic_event_s *e_list) {
	ic_event_s *follow_e;
	int is_valid = 0;

	if (!e_list)
		return 0;

	for(follow_e = e_list; follow_e != NULL; follow_e = follow_e->next){
		is_valid = is_valid_pair_ic_event(follow_e, follow_e->next);
		if (is_valid == -1)
			return is_valid_valid_iptable_ts(follow_e->ts);
		else
			return is_valid && is_valid_valid_iptable_ts(follow_e->ts) && is_valid_valid_iptable_ts(follow_e->next->ts);
	}
	return 0;
}

void print_ic_event_list(ic_event_s *e_list){
	ic_event_s *follow_e;
	int i;

	if(e_list == NULL)
		return;

	i = 0;
	for(follow_e = e_list; follow_e != NULL; follow_e = follow_e->next){
		++i;
		if (is_valid_pair_ic_event(follow_e, follow_e->next) == 0)
			_dprintf("     The next two events is !!!OUT OF THE DAY!!!\n");
		_dprintf("   %3dth event:\n", i);
		_dprintf("        e_name: %s.\n", follow_e->e_name);
		_dprintf("timestamp(UTC): %ld.\n", follow_e->utc ? follow_e->ts : convert_to_utc(follow_e->ts));
		_dprintf("     timestamp: %ld.\n", follow_e->utc ? convert_to_local(follow_e->ts) : follow_e->ts);
		if (follow_e->type == IC_TYPE_TIME) {
			_dprintf("         start: %2d:%2d on %4d-%2d-%2d.\n", follow_e->start_hour, follow_e->start_min, follow_e->start_year, follow_e->start_mon, follow_e->start_day);
			//_dprintf("           end: %2d:%2d on %2d-%2d .\n", follow_e->end_hour, follow_e->end_min, follow_e->end_mon, follow_e->end_day);
			_dprintf("        access: %s.\n", follow_e->access==IC_ACCESS_ALLOWED ? "ALLOWED" : "BLOCKED");
		} else {
			_dprintf("         start: %2d:%2d on %s.\n", follow_e->start_hour, follow_e->start_min, datestr[follow_e->start_day]);
			//_dprintf("           end: %2d:%2d on %s.\n", follow_e->end_hour, follow_e->end_min, datestr[follow_e->end_day]);
			_dprintf("        access: %s.\n", follow_e->access==IC_ACCESS_ALLOWED ? "ALLOWED" : "BLOCKED");
		}
		if(follow_e->next != NULL)
			_dprintf("------------------------------\n");
	}
}

ic_s *initial_ic(ic_s **target_ic){
	ic_s *tmp_ic;

	if(target_ic == NULL)
		return NULL;

	*target_ic = (ic_s *)malloc(sizeof(ic_s));
	if(*target_ic == NULL)
		return NULL;

	tmp_ic = *target_ic;

	tmp_ic->enabled = 0;
	tmp_ic->state = INITIAL;
	tmp_ic->prev_state = INITIAL;
	tmp_ic->dtimes = nvram_get_int("questcf")?:0;
	memset(tmp_ic->device, 0, 32);
	memset(tmp_ic->mac, 0, 18);
	tmp_ic->events = NULL;
	tmp_ic->timestamp = 0;
	tmp_ic->next = NULL;
	memset(tmp_ic->e_str, 0, sizeof(tmp_ic->e_str));

	return tmp_ic;
}

void free_ic_list(ic_s **target_list){
	ic_s *tmp_ic, *old_ic;

	if(target_list == NULL)
		return;

	tmp_ic = *target_list;
	while(tmp_ic != NULL){
		free_ic_event_list(&(tmp_ic->events));

		old_ic = tmp_ic;
		tmp_ic = tmp_ic->next;
		free(old_ic);
	}

	return;
}

ic_s *cp_ic(ic_s **dest, const ic_s *src){
	ic_event_s *follow_e, **follow_e_list;

	if(initial_ic(dest) == NULL){
		_dprintf("No memory!!(dest)\n");
		return NULL;
	}

	(*dest)->enabled = src->enabled;
	strlcpy((*dest)->device, src->device, sizeof((*dest)->device));
	strlcpy((*dest)->mac, src->mac, sizeof((*dest)->mac));

	follow_e_list = &((*dest)->events);
	for(follow_e = src->events; follow_e != NULL; follow_e = follow_e->next){
		cp_ic_event(follow_e_list, follow_e);

		while(*follow_e_list != NULL)
			follow_e_list = &((*follow_e_list)->next);
	}
	strlcpy((*dest)->e_str, src->e_str, sizeof((*dest)->e_str));

	return *dest;
}

ic_s *get_all_ic_list(ic_s **ic_list){
	char word[1024], *next_word;
	ic_s *follow_ic, **follow_ic_list;
	int i, count;
	char buf[4096];


	if(ic_list == NULL)
		return NULL;

	follow_ic_list = ic_list;
	snprintf(buf, sizeof(buf), "%s", nvram_safe_get("ICFILTER_MAC"));
	foreach_62_keep_empty_string(count, word, buf, next_word){
		if(initial_ic(follow_ic_list) == NULL){
			_dprintf("No memory!!(follow_ic_list)\n");
			continue;
		}

		snprintf((*follow_ic_list)->mac, sizeof((*follow_ic_list)->mac), "%s", word);
		(*follow_ic_list)->enabled = isValidMacAddress((*follow_ic_list)->mac);

		while(*follow_ic_list != NULL)
			follow_ic_list = &((*follow_ic_list)->next);
	}

	follow_ic = *ic_list;
	i = 0;
	snprintf(buf, sizeof(buf), "%s", nvram_safe_get("ICFILTER_MACFILTER_DAYTIME"));
	foreach_62_keep_empty_string(count, word, buf, next_word){
		++i;
		if(follow_ic == NULL){
			_dprintf("*** %3dth Internet Control rule(DAYTIME) had something wrong!\n", i);
			return *ic_list;
		}
		snprintf(follow_ic->e_str, sizeof(follow_ic->e_str), "%s", word);
		get_ic_event_list(&(follow_ic->events), word);
		if (follow_ic->enabled) {
			follow_ic->enabled = is_valid_ic_events(follow_ic->events);
		}
		follow_ic = follow_ic->next;
	}

	return *ic_list;
}

void print_ic_list(ic_s *ic_list){
	ic_s *follow_ic;
	int i;

	if(ic_list == NULL)
		return;

	i = 0;
	for(follow_ic = ic_list; follow_ic != NULL; follow_ic = follow_ic->next){
		++i;
		_dprintf("*** %3dth rule:\n", i);
		_dprintf("   enabled: %d.\n", follow_ic->enabled);
		_dprintf("    device: %s.\n", follow_ic->device);
		_dprintf("       mac: %s.\n", follow_ic->mac);
		print_ic_event_list(follow_ic->events);
		//_dprintf(" timestamp: %llu.\n", follow_ic->timestamp);
		_dprintf("******************************\n");
	}
}

static void clean_invalid_config(ic_s *ic_list) {
	ic_s *follow_ic;
	int rule_count = count_ic_rules(ic_list);
	int clean_count = 0;
	char mac_buf[1024] = {0}, daytime_buf[1024] = {0};
	int mac_len = 0, daytime_len = 0;

	// prepare mac and daytime string for config enabled
	for(follow_ic = ic_list; follow_ic != NULL; follow_ic = follow_ic->next){
		if (!follow_ic->enabled)
			continue;
		mac_len += snprintf(mac_buf+mac_len, sizeof(mac_buf)-mac_len, !mac_len ? "%s" : ">%s", follow_ic->mac);
		daytime_len += snprintf(daytime_buf+daytime_len, sizeof(daytime_buf)-daytime_len, !daytime_len ? "%s" : ">%s", follow_ic->e_str);
		clean_count++;
	}

	//_dprintf("INTERNETCTRL : rule_count=%d, clean_count=%d\n", rule_count, clean_count);
	//_dprintf("INTERNETCTRL : MAC=%s\n", mac_buf);
	//_dprintf("INTERNETCTRL : DAYTIME=%s\n", daytime_buf);

	if (rule_count != clean_count) {
		nvram_set("ICFILTER_MAC", mac_buf);
		nvram_set("ICFILTER_MACFILTER_DAYTIME", daytime_buf);
		nvram_commit();
	}

}

// Parental Control:
// MAC address not in list -> ACCEPT.
// MAC address in list and in time period -> ACCEPT.
// MAC address in list and not in time period -> DROP.
void config_ic_rule_string(ic_s *ic_list, FILE *fp, char *logaccept, char *logdrop, int temp){

	ic_s *follow_ic;
	ic_event_s *follow_e;
	char *lan_if = nvram_safe_get("lan_ifname");
#ifdef BLOCKLOCAL
	char *ftype;
#endif
	char *fftype, *fftype_reverse, *fftype_accept, *fftype_drop;

#ifdef BLOCKLOCAL
	ftype = logaccept;
#endif
	fftype_accept = "ICAccept";
	fftype_drop = "ICDrop";

	follow_ic = ic_list;
	if(follow_ic == NULL){
		_dprintf("Couldn't get the rules of Internet Control correctly!\n");
		return;
	}

	for(follow_ic = ic_list; follow_ic != NULL; follow_ic = follow_ic->next){
		const char *chk_type;
		char follow_addr[18] = {0};
		if (!follow_ic->enabled)
			continue;

#ifdef RTCONFIG_AMAS
		if (strlen(follow_ic->mac) && amas_lib_device_ip_query(follow_ic->mac, follow_addr)) {
			chk_type = iptables_chk_ip;
		} else
#endif
		{
			chk_type = iptables_chk_mac;
			snprintf(follow_addr, sizeof(follow_addr), "%s", follow_ic->mac);
		}
		if(!follow_addr[0])
			chk_type = "";

//_dprintf("[PC] mac=%s\n", follow_ic->mac);
#ifdef RTCONFIG_PERMISSION_MANAGEMENT
		if (!strcmp(follow_ic->mac, "")) continue;
#endif

		for(follow_e = follow_ic->events; follow_e != NULL; follow_e = follow_e->next) {
			fftype = follow_e->access == IC_ACCESS_ALLOWED ? fftype_accept : fftype_drop;
			fftype_reverse = follow_e->access == IC_ACCESS_ALLOWED ? fftype_drop : fftype_accept;

			if (follow_e->type == IC_TYPE_TIME) {
				ic_event_s *follow_e_next = follow_e->next;

				// If access type is block and the current time is not in the next event, skip these two events, 
				if (follow_e->access == IC_ACCESS_BLOCKED && 
					follow_e_next && follow_e_next->access == IC_ACCESS_ALLOWED && !match_daytime(follow_e_next)) {
					follow_e = follow_e_next;
					_dprintf("The event of Internet Control is out of the day.\n");
					continue;
				}

				fprintf(fp, "-A FORWARD -i %s -m time", lan_if);
				if(follow_e->start_year > 0 || follow_e->start_mon > 0 || follow_e->start_day > 0)
					fprintf(fp, " --kerneltz --datestart %d-%d-%dT%d:%d", follow_e->start_year, follow_e->start_mon, follow_e->start_day, follow_e->start_hour, follow_e->start_min);

				// check the next follow event has differen access. if true treat it as a period event. Set datestop and the reverse rule for the unmatched time.
				if (follow_e_next && follow_e->type == IC_TYPE_TIME && follow_e->access != follow_e_next->access) {
					fprintf(fp, " --datestop %d-%d-%dT%d:%d", follow_e_next->start_year, follow_e_next->start_mon, follow_e_next->start_day, follow_e_next->start_hour, follow_e_next->start_min);
					follow_e = follow_e_next;
					fprintf(fp, " %s %s -j %s\n", chk_type, follow_addr, fftype);

					// Write the reverse rule for the unmatched time.
					fprintf(fp, "-A FORWARD -i %s %s %s -j %s\n", lan_if, chk_type, follow_addr, fftype_reverse);
				} else {
					fprintf(fp, " %s %s -j %s\n", chk_type, follow_addr, fftype);
				}
			} else if (follow_e->type == IC_TYPE_WEEK) {
				fprintf(fp, "-A FORWARD -i %s -m time", lan_if);
				if(follow_e->start_hour > 0 || follow_e->start_min > 0)
					fprintf(fp, " --timestart %d:%d", follow_e->start_hour, follow_e->start_min);
				fprintf(fp, DAYS_PARAM "%s %s %s -j %s\n", datestr[follow_e->start_day], chk_type, follow_addr, fftype);
			}
		}
	}

	clean_invalid_config(ic_list);
}

int count_ic_rules(ic_s *ic_list/*, int enabled*/){
	ic_s *follow_ic;
	int count;

	follow_ic = ic_list;//match_enabled_ic_list(ic_list, &enabled_list, enabled);
	if(follow_ic == NULL){
		_dprintf("Couldn't get the rules of Internet Control correctly!\n");
		return 0;
	}

	for(count = 0, follow_ic = ic_list; follow_ic != NULL; follow_ic = follow_ic->next)
		++count;

	//free_ic_list(&enabled_list);

	return count;
}

int ic_main(int argc, char *argv[]){
	ic_s *ic_list = NULL;

	get_all_ic_list(&ic_list);

	if(argc == 1 || (argc == 2 && !strcmp(argv[1], "show"))){
		print_ic_list(ic_list);
	}
	/*else if((argc == 2 && !strcmp(argv[1], "enabled"))
			|| (argc == 3 && !strcmp(argv[1], "enabled") && (!strcmp(argv[2], "0") || !strcmp(argv[2], "1") || !strcmp(argv[2], "2")))){
		if(argc == 2)
			match_enabled_ic_list(ic_list, &enabled_list, 1);
		else
			match_enabled_ic_list(ic_list, &enabled_list, atoi(argv[2]));

		print_ic_list(enabled_list);

		free_ic_list(&enabled_list);
	}
	else if(argc == 4 && !strcmp(argv[1], "daytime")
			&& (atoi(argv[2]) >= MIN_DAY && atoi(argv[2]) <= MAX_DAY)
			&& (atoi(argv[3]) >= MIN_HOUR && atoi(argv[3]) <= MAX_HOUR)
			){
		match_daytime_ic_list(ic_list, &daytime_list, atoi(argv[2]), atoi(argv[3]));

		print_ic_list(daytime_list);

		free_ic_list(&daytime_list);
	}
	else if(argc == 2 && !strcmp(argv[1], "apply")){
		int wan_unit = wan_primary_ifunit();
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

		match_enabled_ic_list(ic_list, &enabled_list, 1);

		filter_setting(wan_unit, lan_if, lan_ip, logaccept, logdrop);

		free_ic_list(&enabled_list);
	}
	else if(argc == 2 && !strcmp(argv[1], "showrules")){
		config_daytime_string(ic_list, stderr, "ACCEPT", "logdrop", 0);
	}
#ifdef RTCONFIG_CONNTRACK
	else if(argc == 2 && !strcmp(argv[1], "flush")){
		flush_ic_list(ic_list);
	}
#endif*/
	else{
		printf("Usage: ic [show]\n"
		       "          showrules\n"
		       "          enabled [1 | 0]\n"
		       "          daytime [1-7] [0-23]\n"
		       "          apply\n"
#ifdef RTCONFIG_CONNTRACK
		       "          flush\n"
#endif
		       );
	}

	free_ic_list(&ic_list);

	return 0;
}