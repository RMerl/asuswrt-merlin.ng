#ifndef __AI_LIB_H__
#define __AI_LIB_H__

/* Docker apps control on AISOM */
/* CMD STATUS definition */
#define TZ_SETTING "TZ_SETTING"
#define TZ_SYNC    "TZ_SYNC"
#define NTP_SETTING "NTP_SETTING"
#define NTP_SYNC   "NTP_SYNC"
#define CHECK      "CHECK"
#define DOWNLOAD   "DOWNLOAD"
#define CREATE     "CREATE"
#define RUN        "RUN"
#define CLEANUP    "CLEANUP"

/* CMD IMAGE definition */
#define APP_TZ         "tz"             // timezone sync
#define APP_CLEAN      "clean"          // clean up all none default data
#define APP_TEST       "test"
#define APP_FRIGATE    "frigate"        // Frigate NVR app
#define APP_PORTAINER  "portainer"      // portainer app
#define APP_DPANEL     "dpanel"
#define APP_HA         "homeassistant"  // Home Assistant app
#define APP_ADGUARD    "adguard"        // Adguard app
#define APP_SLM        "slm"            // Small language model app
#define APP_SLMQA      "slmqa"
#define APP_SLMQAVER   "slmqaver"

#define APP_CMD(cmd, tag) \
    cmd " 2>&1 >/dev/null | systemd-cat -p err -t " tag

#define APP_RESTART_TIMESYNC "systemctl restart systemd-timesyncd"

/* apps cmd struct declaration */
typedef struct {
	char *image;
	char *status;
	char *command;
	int timeout;
} ai_app_cmd_t;

typedef struct {
    char *app_name;
    ai_app_cmd_t *cmds;
    size_t count;
} ai_app_item_t;

extern ai_app_item_t ai_app_map[];
extern const int ai_app_map_size;
#endif
