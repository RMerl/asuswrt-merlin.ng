#ifndef __SCHED_V2_SHARED_H__
#define __SCHED_V2_SHARED_H__

#define MAX_NVRAM_SCHED_LEN 4097
#define MAX_SCHED_RULES 64

#define SCHED_V2_DISABLE 0
#define SCHED_V2_ENABLE 1

enum _sched_v1_index {
	SCHED_V1_IDX_START_DAY = 0,
	SCHED_V1_IDX_END_DAY = 1,
	SCHED_V1_IDX_START_HOUR = 2,
	SCHED_V1_IDX_END_HOUR = 4,
};

typedef struct _sched_v1 sched_v1_t;
struct _sched_v1 {
	int start_day;
	int end_day;
	int start_hour;
	int end_hour;
	sched_v1_t *prev;
	sched_v1_t *next;
};

typedef enum _sched_v2_type sched_v2_type;
enum _sched_v2_type {
	SCHED_V2_TYPE_UNKNOWN = 0,
	SCHED_V2_TYPE_DAY,
	SCHED_V2_TYPE_WEEK,
	SCHED_V2_TYPE_WEEK_ONLINE,
	SCHED_V2_TYPE_TIMESTAMP
};

typedef enum _sched_v2_d_index sched_v2_d_index;
enum _sched_v2_d_index {
	SCHED_V2_D_IDX_TYPE = 0,
	SCHED_V2_D_IDX_ENABLE = 1,
	SCHED_V2_D_IDX_START_HOUR = 2,
	SCHED_V2_D_IDX_START_MINUTE = 4,
	SCHED_V2_D_IDX_END_HOUR = 6,
	SCHED_V2_D_IDX_END_MINUTE = 8,
	SCHED_V2_D_IDX_NULL = 10,
};

typedef enum _sched_v2_w_index sched_v2_w_index;
enum _sched_v2_w_index {
	SCHED_V2_W_IDX_TYPE = 0,
	SCHED_V2_W_IDX_ENABLE = 1,
	SCHED_V2_W_IDX_DAY_OF_WEEK = 2,
	SCHED_V2_W_IDX_START_HOUR = 4,
	SCHED_V2_W_IDX_START_MINUTE = 6,
	SCHED_V2_W_IDX_END_HOUR = 8,
	SCHED_V2_W_IDX_END_MINUTE = 10,
	SCHED_V2_W_IDX_NULL = 12,
};

typedef enum _sched_v2_t_index sched_v2_t_index;
enum _sched_v2_t_index {
	SCHED_V2_T_IDX_S_TYPE = 0,
	SCHED_V2_T_IDX_S_FLAG = 1,
	SCHED_V2_T_IDX_S_TIMESTAMP = 2,
	SCHED_V2_T_IDX_S_Z_MARK = 12,
	SCHED_V2_T_IDX_DELIMITER = 13,
	SCHED_V2_T_IDX_E_TYPE = 14,
	SCHED_V2_T_IDX_E_FLAG = 15,
	SCHED_V2_T_IDX_E_TIMESTAMP = 16,
	SCHED_V2_T_IDX_E_Z_MARK = 26,
	SCHED_V2_T_IDX_NULL = 27,
};

typedef struct _sched_v2_d sched_v2_d_t;
struct _sched_v2_d {
	int enable;
	int start_hour;
	int start_minute;
	int end_hour;
	int end_minute;
};

typedef struct _sched_v2_w sched_v2_w_t;
struct _sched_v2_w {
	int enable;
	int day_of_week;
	int start_hour;
	int start_minute;
	int end_hour;
	int end_minute;
	unsigned int rule_to_number;
};

typedef struct _sched_v2_t sched_v2_t_t;
struct _sched_v2_t {
	int s_flag;
	time_t s_ts;
	char s_z_mark;
	int e_flag;
	time_t e_ts;
	char e_z_mark;
};

typedef struct _sched_v2 sched_v2_t;
struct _sched_v2 {
	sched_v2_type type;
	sched_v2_d_t value_d;
	sched_v2_w_t value_w;
	sched_v2_t_t value_t;
	sched_v2_t *prev;
	sched_v2_t *next;
};

#define	TIMESCHED_SCHEDULE 1 << 0
#define TIMESCHED_ACCESSTIME 1 << 1

#define SCHED_DEBUG "/tmp/SCHED_DEBUG"
#define SCHED_DBG(fmt, arg...) \
	do { if (nvram_get_int("sched_dbg") || f_exists(SCHED_DEBUG) > 0) \
        dbg("SCHED(%d)/%s(%d) : "fmt"\n", getpid(), __func__, __LINE__, ##arg); \
	} while (0)
#if 0
extern char *convert_to_str_sched_v1(const char *str_sched, char *buf, int buf_size) ;
#endif
extern void free_sched_v2_list(sched_v2_t **sched_v2_list);
extern int parse_str_v2_to_sched_v2_list(const char *str_sched_v2, sched_v2_t **sched_v2_list, int merge_same_period, int skip_disabled);

extern int check_sched_v2_on_off(const char *sched_str);
extern int check_expire_on_off(const char *sched_str);
extern void convert_wl_sched_v1_to_sched_v2();
extern void convert_pc_sched_v1_to_sched_v2();

#endif /* !__SCHED_V2_SHARED_H__ */