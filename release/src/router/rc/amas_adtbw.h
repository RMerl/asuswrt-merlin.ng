#define AMAS_ADTBW_TIMER 1   //sec
#define AMAS_ADTBW_DFT_POLL_INTERVAL 10
#define AMAS_ADTBW_DFT_TIMEOUT_WARM_UP 90 //sec
#define AMAS_ADTBW_DFT_TIMEOUT_SWITCH 60 //sec
#define AMAS_ADTBW_DFT_BW80_RSSI_THRESH_US -57
#define AMAS_ADTBW_DFT_BW160_RSSI_THRESH_US -69
#define AMAS_ADTBW_DFT_BW80_RSSI_THRESH_EU -60
#define AMAS_ADTBW_DFT_BW160_RSSI_THRESH_EU -66
#define AMAS_ADTBW_DFT_BW80_HIT_THRESH 3
#define AMAS_ADTBW_DFT_BW160_HIT_THRESH 3

#define AMAS_ADTBW_DFT_CTRLCH_B3 104
#define AMAS_ADTBW_DFT_CTRLCH_B4 149

#define AMAS_ADTBW_BW_SWITCH_SUCCESS 0
#define AMAS_ADTBW_BW_SWITCH_FAILURE 1
#define AMAS_ADTBW_BW_SWITCH_RADAR_DET 2
#define AMAS_ADTBW_BW_SWITCH_RADAR_NOT_DET 0

#define AMAS_ADTBW_SKU_UNSUPPORT 0
#define AMAS_ADTBW_SKU_US 1
#define AMAS_ADTBW_SKU_EU 2

#define AMAS_ADTBW_DEBUG "/tmp/AMAS_ADTBW_DEBUG"
#define MACF_UP "%02X:%02X:%02X:%02X:%02X:%02X"
#define LOG_TITLE_AMAS_ADTBW "amas_adtbw"

#define AMAS_ADTBW_SYSLOG(fmt, arg...) \
	do { logmessage(LOG_TITLE_AMAS_ADTBW, fmt, ##arg); \
	} while (0)

#define AMAS_ADTBW_DBG(fmt, arg...) \
	do { if (f_exists(AMAS_ADTBW_DEBUG) || amas_adtbw_dbg) \
		dbg("AMAS_ADTBW %s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); \
	    if(amas_adtbw_syslog) \
		logmessage(LOG_TITLE_AMAS_ADTBW, fmt, ##arg); \
	} while (0)

typedef enum {
	LED_NO_ACTION = 0,
	LED_WARM_UP,
	LED_CAC,
	LED_BW160
} amas_adtbw_led_t;

typedef struct	amas_adtbw_config {
	int poll_itval;
    	int unit;
	char ifname[16];
	int multiple_re;
	int rssi_bw80;
	int rssi_bw160;
	uint8 hit_bw80;
	uint8 hit_bw160;
	uint8 sku;
} amas_adtbw_conf_t;

typedef struct amas_adtbw_state {
	amas_adtbw_led_t led_state;
	time_t time_first_re_assoc;
	time_t time_switch_160m;
	uint8 dfs_block_remain;
	uint8 first_re_assoc;
	uint8 re_count;
	uint8 stop_switch_160m;

} amas_adtbw_state_t;

int amas_adtbw_dbg;
int amas_adtbw_syslog;
amas_adtbw_conf_t adtbw_config;
amas_adtbw_state_t adtbw_state;
void start_amas_adtbw(void);
void stop_amas_adtbw(void);
void amas_adtbw_get_config(void);

extern int amas_adtbw_enable(void);
extern int amas_adtbw_dont_check(chanspec_t chanspec);
extern chanspec_t amas_adtbw_get_chanspec(char* ifname);
extern int amas_adtbw_check_bw_switch(chanspec_t chsp, int *do_imdtly);
extern int amas_adtbw_do_bw_switch(chanspec_t chsp, int bw160);
extern int amas_adtbw_conduct_cac(void);

