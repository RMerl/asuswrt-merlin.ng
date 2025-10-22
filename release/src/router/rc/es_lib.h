#ifndef __ES_LIB_H__
#define __ES_LIB_H__

#include <stdint.h>

#include <shared.h>

#define ES_SOCKET_PATH   "/var/run/es_socket"

#define ES_WLC_CONN      (1 << 0)
#define ES_WLC_DISCONN   (1 << 1)

#define ES_BAND_2G       WIFI_BAND_2G
#define ES_BAND_5G       WIFI_BAND_5G
#define ES_BAND_5GL      WIFI_BAND_5GL
#define ES_BAND_5GH      WIFI_BAND_5GH
#define ES_BAND_6G       WIFI_BAND_6G
#define ES_BAND_6GL      WIFI_BAND_6GL
#define ES_BAND_6GH      WIFI_BAND_6GH

#if defined(GTBE19000AI)
#define ES_BAND_MASK     (ES_BAND_2G | ES_BAND_5G | ES_BAND_6G)
#define ES_BAND_CAP_MASK (ES_BAND_2G | ES_BAND_5G | ES_BAND_6G)
#define ES_BAND_APD      (ES_BAND_6G)
#elif defined(GTBE96_AI)
#define ES_BAND_MASK     (ES_BAND_2G | ES_BAND_5GL | ES_BAND_5GH)
#define ES_BAND_CAP_MASK (ES_BAND_2G | ES_BAND_5G)
#define ES_BAND_APD      (ES_BAND_5GH)
#else
#define ES_BAND_MASK     0
#define ES_BAND_CAP_MASK 0
#define ES_BAND_APD      0
#endif

#define ES_STS_ON        0
#define ES_STS_OFF       1
#define ES_STS_HALF      2

#define ES_PWR_ETH_1G    1
#define ES_PWR_ETH_2P5G  100
#define ES_PWR_ETH_10G   10000
#define ES_PWR_USB_480   1
#define ES_PWR_USB_5000  100

typedef enum {
	ES_LOG_NONE,
	ES_LOG_ERROR,
	ES_LOG_DEFAULT,
	ES_LOG_INFO,
	ES_LOG_DEBUG,
	ES_LOG_MAX
} es_log_level_t;

typedef enum {
	ESR_NONE = 0,
	ESR_SET_DUMP,
	ESR_SET_LOG_LEVEL,
	ESR_SET_LOG_OUTPUT,
	ESR_ADD_STA_CONN_ST,
	ESR_ADD_STA_BAND_CAP,	// imply ES_WLC_CONN
	ESR_SET_STA_BAND_CAP,
	ESR_SET_STS_LED,
	ESR_SET_STS_AURA,
	ESR_SET_STS_SYS,
	ESR_SET_STS_ETH,
	ESR_SET_STS_USB,
	ESR_SET_STS_WLBAND,
	ESR_SET_HALT,
	ESR_SET_CHECK,
	ESR_SET_AUTO_RENEW_CYCLE,
	ESR_GET_HISTORY,
	ESR_GET_STATUS,
} esr_t;

typedef struct {
	esr_t type;
	union {
		es_log_level_t log_level;
		char log_path[64];
		uint8_t conn_st;
		uint8_t band_cap;
		struct {
			uint8_t status;
			char updater[16];
		} led;
		struct {
			uint8_t status;
			char updater[16];
		} led_aura;
		struct {
			uint8_t status;
			char updater[16];
		} sys;
		struct {
			uint32_t status;
			char updater[16];
		} eth;
		struct {
			uint16_t status;
			char updater[16];
		} usb;
		struct {
			uint8_t band;
			uint8_t status;
			char updater[16];
		} wlband;
		int halt;
		time_t cycle;
		struct {
			time_t duration;
			char output_path[64];
			size_t buf_size;
		} history;
		struct {
			char output_path[64];
			size_t buf_size;
		} status;
	};
} esr_data_t;


void es_log(es_log_level_t level, const char *message, ...);
void es_set_log_level(es_log_level_t level);
void es_set_log_ouput(const char* path);
const char *es_log_level_to_str(es_log_level_t level);

int es_do_write(int fd, const void* data, size_t len);
int es_do_read(int fd, void* data, size_t len);
int es_get_data(esr_data_t *s_data, void* data, size_t len);
int es_set_data(esr_data_t *s_data);

uint8_t es_get_band_by_unit(int unit);
int esr_set_dump();
int esr_set_check();
int esr_set_log_level(es_log_level_t level);
int esr_set_log_ouput(const char* path);
int esr_add_conn_st(uint8_t conn_st);
int esr_add_band_cap(uint8_t cap);
int esr_set_band_cap(uint8_t cap);
int esr_set_led(uint8_t status, const char* updater);
int esr_set_aura(uint8_t status, const char* updater);
int esr_set_sys(uint8_t status, const char* updater);
int esr_set_eth(uint32_t status, const char* updater);
int esr_set_usb(uint16_t status, const char* updater);
int esr_set_wlband(uint8_t band, uint8_t status, const char* updater);
int esr_set_wlunit(int unit, uint8_t status, const char* updater);
int esr_set_halt(int activate);
int esr_set_auto_renew_cycle(time_t cycle);
int esr_get_history_day(size_t num_of_days, const char* output_file, char* buf, size_t len);
int esr_get_history_hour(size_t num_of_hours, const char* output_file, char* buf, size_t len);
int esr_get_history_min(size_t num_of_mins, const char* output_file, char* buf, size_t len);
int esr_get_history_sec(size_t num_of_secs, const char* output_file, char* buf, size_t len);
int esr_get_status(const char* output_file, char* buf, size_t len);

#endif