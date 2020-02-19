#ifndef __IC_H__
#define __IC_H__

typedef struct ic_event ic_event_s;
struct ic_event{
	char e_name[32];
	int type;
	int start_year;
	int end_year;
	int start_mon;
	int end_mon;
	int start_day;
	int end_day;
	int start_hour;
	int end_hour;
	int start_min;
	int end_min;
	int access;
	time_t ts;
	int utc;
	ic_event_s *next;
};

enum {
	IC_ACCESS_BLOCKED,
	IC_ACCESS_ALLOWED
};

enum {
	IC_TYPE_TIME,
	IC_TYPE_DAY,
	IC_TYPE_WEEK
};

typedef struct ic ic_s;
struct ic{
	int enabled;
	char state;
	char prev_state;
	int dtimes;
	char device[32];
	char mac[18];
	ic_event_s *events;
	unsigned long long timestamp;
	char e_str[64];
	ic_s *next;
};

extern ic_s *get_all_ic_list(ic_s **ic_list);
extern ic_s *get_all_ic_tmp_list(ic_s **ic_s_list);

extern ic_event_s *initial_ic_event(ic_event_s **target_e);
extern void free_ic_event_list(ic_event_s **target_list);
extern ic_event_s *cp_ic_event(ic_event_s **dest, const ic_event_s *src);
extern void print_ic_event_list(ic_event_s *e_list);

extern ic_s *initial_ic(ic_s **target_ic);
extern void free_ic_list(ic_s **target_list);
extern ic_s *cp_ic(ic_s **dest, const ic_s *src);
extern void print_ic_list(ic_s *ic_list);

extern void config_ic_rule_string(ic_s *ic_list, FILE *fp, char *logaccept, char *logdrop, int temp);
extern int count_ic_rules(ic_s *ic_list/*, int enabled*/);
#endif