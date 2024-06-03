#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>

#include <pthread.h>

#include <signal.h>

#ifdef __cplusplus  
extern "C" {  
#endif
#include <bcmnvram.h>
#ifdef __cplusplus  
}
#endif

#include <shared.h>
#include <aae_ipc.h>

#include <assert.h>
// #include <time.h>
// #include <getopt.h>


#include "log.h"
#include "api.h"
#include "uploader.h"

#include "upload_api.h"

#include "uploader_ipc.h"
#include "uploader_mnt.h"

#include "webapi.h"

#define APP_DBG 1

char g_formated_router_mac[32] = {0};
int g_sleep_stop = 0;

void init_basic_data() {

    int account_bound = 0;

    const char* router_mac = nvram_get("lan_hwaddr");

    while(1) {

#ifdef RTCONFIG_ACCOUNT_BINDING
        account_bound = is_account_bound();
#endif
        
        if (nvram_get_int("ntp_ready")==1 &&
            nvram_get_int("link_internet")==2/* 2 -> connected*/ &&
            nvram_get_int("uploader_enable")==1 &&
			account_bound) {
            
            Cdbg(APP_DBG, "System is ready and start uploader.");

            format_mac(router_mac, g_formated_router_mac);

            break;
        }

        // ex_db_backup();

        sleep(60);
    }

    if(!check_if_dir_exist(UPLOADER_FOLDER)) {
        Cdbg(APP_DBG, "Create folder for uploader (%s)", UPLOADER_FOLDER);
        mkdir(UPLOADER_FOLDER, 0755);
    }
}

void sleep_time(int sleep_time) {

    g_sleep_stop = 0;

    int i;
    for(i = 0; i < sleep_time; i++) {
        sleep(1);
        // Cdbg(APP_DBG, "sleep count = %d", i);
        if(g_sleep_stop) {
            break;
        }
    }

}

#define UPLOADER_SIG_ACTION SIGUSR1

enum {
    UPLOADER_ACTION_SIGUSR1,
};

static void uploader_sig_handler(int sig) {

    Cdbg(APP_DBG, "Get uploader sig = %d", sig);

    switch (sig) {

        case UPLOADER_SIG_ACTION: {
            g_sleep_stop = 1;

            Cdbg(APP_DBG, "reset sleep time");
        }

        break;
    }
}

int main( int argc, char ** argv ) {
    
    // Open Debug Log
    CF_OPEN(APP_LOG_PATH,  FILE_TYPE | CONSOLE_TYPE | SYSLOG_TYPE);
    // CF_OPEN(UL_DEBUG_TO_FILE, FILE_TYPE );
    // CF_OPEN(UL_DEBUG_TO_CONSOLE,  CONSOLE_TYPE | SYSLOG_TYPE | FILE_TYPE);

    init_basic_data();
    
    // receive uploader sig = SIGUSR1 = 10
    signal(UPLOADER_SIG_ACTION, uploader_sig_handler);

    // ipc : waiting call 
    uploader_ipc_start();

    int while_count = 0;

    if (nvram_get_int("uploader_reset_time")<=0) {
        nvram_set_int("uploader_reset_time", (unsigned )time(NULL));
    }

    // uploader start:  backup [ui_support] config
    nvram_set_int("ui_support_update_time", (unsigned )time(NULL));
    
    DECLARE_CLEAR_MEM(char, access_token, MAX_DESC_LEN);
    if (get_cloud_access_token(access_token)!=0) {
        Cdbg(APP_DBG, "Fail to get access token");
        return 0;
    }
    
    list_cloud_file(access_token);
    
    do {
        while_count++;
        
        reorganize_cloud_file(access_token);

#ifdef RTCONFIG_AMAS_CENTRAL_CONTROL
        backup_cfg_mnt();
#endif

        backup_cloud_file(access_token);
        
        // External Backup File Upload (Scan db file every 30 seconds)
        backup_db_file();

        sleep_time(UPLOAD_WAITING_TIME);
        
    } while (1);
    
    clear_cloud_file();

    Cdbg(APP_DBG, "exit uploader, waiting restart");

    return 0;
}
