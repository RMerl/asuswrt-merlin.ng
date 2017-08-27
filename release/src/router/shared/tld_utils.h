/*
	Traffic Limiter usage
*/

#ifdef RTCONFIG_TRAFFIC_LIMITER
/* debug message */
#define TLD_DEBUG               "/tmp/TLD_DEBUG"
#define TLD_DEBUG_LOG           "/tmp/TLD_LOG"

#define TL_DBG(fmt,args...) \
	if(f_exists(TLD_DEBUG) > 0) { \
		dbg("[TRAFFIC LIMITER][%s:(%d)]"fmt, __FUNCTION__, __LINE__, ##args); \
	}

#define TL_LOG(fmt,args...) \
	if(f_exists(TLD_DEBUG_LOG) > 0) { \
		char info[200]; \
		snprintf(info, sizeof(info), "echo \"[TRAFFIC LIMITER]"fmt"\" >> /tmp/TLD.log", ##args); \
		system(info); \
	}

/* define */
extern int TL_UNIT_S; // traffic limiter dual wan unit start
extern int TL_UNIT_E; // traffic limiter dual wan unit end
extern char *traffic_limtier_count_path();
extern unsigned int traffic_limiter_read_bit(const char *type);
extern void traffic_limiter_set_bit(const char *type, int unit);
extern void traffic_limiter_clear_bit(const char *type, int unit);
extern double traffic_limiter_get_realtime(int unit);
extern int traffic_limiter_dualwan_check(char *dualwan_mode);
#endif
