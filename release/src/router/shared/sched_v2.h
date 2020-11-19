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

typedef struct _sched_v2 sched_v2_t;
struct _sched_v2 {
	sched_v2_type type;
	sched_v2_d_t value_d;
	sched_v2_w_t value_w;
	sched_v2_t *prev;
	sched_v2_t *next;
};

#define SCHED_DEBUG "/tmp/SCHED_DEBUG"
#define SCHED_DBG(fmt, arg...) \
	do { if (nvram_get_int("sched_dbg") || f_exists(SCHED_DEBUG) > 0) \
        dbg("SCHED(%d)/%s(%d) : "fmt"\n", getpid(), __func__, __LINE__, ##arg); \
	} while (0)
#if 0
extern char *convert_to_str_sched_v1(const char *str_sched, char *buf, int buf_size) ;
#endif
extern void free_sched_v2_list(sched_v2_t **sched_v2_list);
extern int parse_str_v2_to_sched_v2_list(const char *str_sched_v2, sched_v2_t **sched_v2_list, int merge_same_period);

extern int check_sched_v2_on_off(const char *sched_str);
extern void convert_wl_sched_v1_to_sched_v2();
extern void convert_pc_sched_v1_to_sched_v2();

#endif /* !__SCHED_V2_SHARED_H__ */