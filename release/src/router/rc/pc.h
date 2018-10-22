#define MIN_DAY 1
#define MAX_DAY 7

#define MIN_HOUR 0
#define MAX_HOUR 23

//#define BLOCKLOCAL

#define iptables_chk_mac "-m mac --mac-source"
extern char *datestr[];

typedef struct pc_event pc_event_s;
struct pc_event{
	char e_name[32];
	int start_day;
	int end_day;
	int start_hour;
	int end_hour;
	int start_min;
	int end_min;
	pc_event_s *next;
};

typedef struct pc pc_s;
struct pc{
	int enabled;
	char device[32];
	char mac[18];
	pc_event_s *events;
	unsigned long long timestamp;
	pc_s *next;
};

extern pc_s *get_all_pc_list(pc_s **pc_list);
extern pc_s *get_all_pc_tmp_list(pc_s **pc_list);

extern pc_event_s *initial_event(pc_event_s **target_e);
extern void free_event_list(pc_event_s **target_list);
extern pc_event_s *cp_event(pc_event_s **dest, const pc_event_s *src);
extern void print_event_list(pc_event_s *e_list);

extern pc_s *initial_pc(pc_s **target_pc);
extern void free_pc_list(pc_s **target_list);
extern pc_s *cp_pc(pc_s **dest, const pc_s *src);
extern void print_pc_list(pc_s *pc_list);

extern pc_s *match_enabled_pc_list(pc_s *pc_list, pc_s **target_list, int enabled);
extern pc_s *match_day_pc_list(pc_s *pc_list, pc_s **target_list, int target_day);
extern pc_s *match_daytime_pc_list(pc_s *pc_list, pc_s **target_list, int target_day, int target_hour);

extern void config_daytime_string(pc_s *pc_list, FILE *fp, char *logaccept, char *logdrop, int temp);
extern void config_pause_block_string(pc_s *pc_list, FILE *fp, char *logaccept, char *logdrop, int temp);
extern int count_pc_rules(pc_s *pc_list, int enabled);
