#ifndef __AI_LIB_H__
#define __AI_LIB_H__

/* Docker apps control on AISOM */
/* CMD STATUS definition */
#define TZ_SETTING "TZ_SETTING"
#define TZ_SYNC    "TZ_SYNC"
#define CHECK      "CHECK"
#define DOWNLOAD   "DOWNLOAD"
#define CREATE     "CREATE"
#define RUN        "RUN"
#define CLEANUP    "CLEANUP"
/* CMD IMAGE definition */
#define APP_TZ         "tz"
#define APP_TEST       "test"
#define APP_FRIGATE    "frigate"
#define APP_PORTAINER  "portainer"
#define APP_DPANEL     "dpanel"
#define APP_HA         "homeassistant"
#define APP_ADGUARD    "adguard"
#define APP_SLM        "slm"
#define APP_SLMQA      "slmqa"
#define APP_SLMQAVER   "slmqaver"
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
