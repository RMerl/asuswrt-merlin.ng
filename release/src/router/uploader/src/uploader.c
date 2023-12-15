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


char device_model[32];
char aae_deviceid[64];
char oauth_dm_cusid[128];
char oauth_dm_user_ticket[64];
char router_mac[32];
char mac_no_symbol[32];

char devicemd5mac[33] = {0};
char aae_webstorageinfo[128] = {0};


char process_api[32];
char file_name[64];
char file_path[256];
char server_api[32];


int sleep_stop = 0;
 
extern struct api_response_data api_response_data;
extern struct cloud_files_l_list_bak cloud_files_l_list_bak[TYPE_NUMBER];


extern char backup_file_type[TYPE_NUMBER][TYPE_LEN];
extern int backup_file_limit[TYPE_NUMBER]; 

cloud_files_list *files_list_frist, *files_list_current, *files_list_previous;


void init_basic_data() 
{

    int ready_count = 0;

    while(1) {

        ready_count++;

        int ntp_ready = nvram_get_int("ntp_ready");

        int link_internet = nvram_get_int("link_internet"); // 2 -> connected

        int oauth_auth_status = nvram_get_int("oauth_auth_status"); // 2 -> account_binding

#ifdef RTCONFIG_ACCOUNT_BINDING
        int account_bound = is_account_bound();
#else
        int account_bound = 0;
#endif
        
        if((ntp_ready == 1) && (link_internet == 2) && account_bound) {
            Cdbg(APP_DBG, "ntp_ready -> %d, link_internet -> %d, oauth_auth_status -> %d", ntp_ready, link_internet, oauth_auth_status);
            break;
        } else {
            if(ready_count < 3) {
                // Cdbg(APP_DBG, "waiting link_internet -> %d", link_internet);
            }
        }

        ex_db_backup();

        // if( (ready_count % 10) == 3) {
        //     ex_db_backup();
        //     if(ready_count > 10000)
        //         ready_count = 3;
        // }

        sleep(60);
    }

    ready_count = 0;

    while(1) {

        ready_count++;

        snprintf(aae_deviceid, sizeof(aae_deviceid), "%s", nvram_safe_get("aae_deviceid"));
        snprintf(router_mac, sizeof(router_mac), "%s", nvram_safe_get("lan_hwaddr"));
        snprintf(device_model, sizeof(device_model), "%s", nvram_safe_get("odmpid"));
        snprintf(oauth_dm_cusid, sizeof(oauth_dm_cusid), "%s", nvram_safe_get("oauth_dm_cusid"));
        snprintf(oauth_dm_user_ticket, sizeof(oauth_dm_user_ticket), "%s", nvram_safe_get("oauth_dm_user_ticket"));

        mac_toupper(router_mac, mac_no_symbol);

        str2md5(router_mac, devicemd5mac);

        if((strlen(device_model) < 1)) {
            snprintf(device_model, sizeof(device_model), "%s", nvram_safe_get("productid"));
        }

        if( (strlen(aae_deviceid) > 1) && (strlen(device_model) > 1) && (strlen(oauth_dm_cusid) > 1) && (strlen(oauth_dm_user_ticket) > 1) && (strlen(router_mac) > 1) ) {
            Cdbg(APP_DBG, "Get oauth_dm_cusid -> %s, aae_deviceid -> %s, device_model -> %s, oauth_dm_user_ticket -> %s, router_mac -> %s", oauth_dm_cusid, aae_deviceid, device_model, oauth_dm_user_ticket, router_mac);
            break;
        } else {
            if(ready_count < 2) {
                Cdbg(APP_DBG, "error data, oauth_dm_cusid -> %s, aae_deviceid -> %s, device_model -> %s, oauth_dm_user_ticket -> %s, router_mac -> %s", oauth_dm_cusid, aae_deviceid, device_model, oauth_dm_user_ticket, router_mac);
            }
        }
        sleep(30);
    }

    if(!check_if_dir_exist(UPLOADER_FOLDER)) {

      Cdbg(APP_DBG, "create temp folder for uploader (%s)", UPLOADER_FOLDER);
      mkdir(UPLOADER_FOLDER, 0755);
    }

    snprintf(aae_webstorageinfo, sizeof(aae_webstorageinfo), "https://%s/", nvram_safe_get("aae_webstorageinfo"));

    // check url :  aae_webstorageinfo  
    if(strlen(aae_webstorageinfo) < 15) {
        snprintf(aae_webstorageinfo, sizeof(aae_webstorageinfo), "%s", AICAMCLOUD_URL);
    }
}




int process_input(int argc, char ** argv)
{

    int status = 0;

    int o;

    while(-1 != (o = getopt(argc, argv, "f:a:p:"))) {

        Cdbg(APP_DBG, "[%d] %s\n", o, optarg);      

        switch(o) 
        {
            case 'p':

                Cdbg(APP_DBG, "-p [%d] %s\n", o, optarg);   
                snprintf(file_path, sizeof(file_path) ,"%s", optarg);
                break;
            case 'a':
                Cdbg(APP_DBG, "-a [%d] %s\n", o, optarg);
                snprintf(process_api, sizeof(process_api) ,"%s", optarg);
                break;
            case 'f':
                Cdbg(APP_DBG, "-f [%d] %s\n", o, optarg);
                snprintf(file_name, sizeof(file_name) ,"%s", optarg);
            break;
        }
    }


    if( (strlen(file_name) < 3) && (strlen(file_path) < 3) && (strlen(process_api) < 10) ) {
        Cdbg(APP_DBG, "file file_path [%s] error OR file name [%s] error OR api name [%s] error", file_path, file_name, process_api);
        exit(0);
    }



    status = call_cloud_api(API_ROUTER_CHECK_TOKEN , oauth_dm_cusid, 
                                     oauth_dm_user_ticket, devicemd5mac, NULL, NULL);

    if(status != 0) {
        Cdbg(APP_DBG, "Post [router_check_token] API message error, status = %d", status);
        exit(0);
    } else if(api_response_data.status != 0) {
        Cdbg(APP_DBG, "[check_token] get data error, status = %d", api_response_data.status);
        exit(0);
    }


    Cdbg(APP_DBG, "API_ROUTER_CHECK_TOKEN status = %d, access_token = %s", status, api_response_data.access_token);


    if( strcmp(process_api, UPLOAD_FILE_TO_S3) == 0) {


        snprintf(server_api, sizeof(server_api) ,"%s", API_ROUTER_GET_UPLOAD_FILE_URL);


        status = call_cloud_api(API_ROUTER_GET_UPLOAD_FILE_URL , NULL, NULL, 
                                         devicemd5mac, file_path, file_name);


        if(status != 0) {
            Cdbg(APP_DBG, "Post [%s] API message error, status = %d", server_api, status);
            exit(0);
        } else if(api_response_data.status != 0) {
            Cdbg(APP_DBG, "[%s] get data error, status = %d", server_api, api_response_data.status);
            exit(0);
        }

        Cdbg(APP_DBG, "status = %d, policy = %s", status, api_response_data.policy);
        Cdbg(APP_DBG, "url = %s", api_response_data.url);
        Cdbg(APP_DBG, "key = %s", api_response_data.key);
        Cdbg(APP_DBG, "x_amz_algorithm = %s", api_response_data.x_amz_algorithm);
        Cdbg(APP_DBG, "x_amz_credential = %s", api_response_data.x_amz_credential);
        Cdbg(APP_DBG, "x_amz_date = %s", api_response_data.x_amz_date);
        Cdbg(APP_DBG, "policy = %s", api_response_data.policy);
        Cdbg(APP_DBG, "x_amz_signature = %s", api_response_data.x_amz_signature);

        char upload_file[256];
        snprintf(upload_file, sizeof(upload_file) ,"%s%s", file_path, file_name);


        status = upload_file_to_s3(S3_RESPONSE_FILE, upload_file, NULL);

        Cdbg(APP_DBG, "S3_RESPONSE_FILE = %s, status = %d", S3_RESPONSE_FILE, status);


        if(status != 204) {
            Cdbg(APP_DBG, "Post [upload_file_to_s3] API message error, status = %d", status);
            exit(0);
        } 


        snprintf(server_api, sizeof(server_api) ,"%s", API_ROUTER_UPLOAD_FILE);


        status = call_cloud_api(API_ROUTER_UPLOAD_FILE , NULL, NULL, 
                                     devicemd5mac, file_path, file_name);




    } else if( strcmp(process_api, DEL_S3_FILE) == 0) {


        snprintf(server_api, sizeof(server_api) ,"%s", API_ROUTER_DEL_FILE);

        status = call_cloud_api(API_ROUTER_DEL_FILE , NULL, NULL, 
                                         devicemd5mac, file_path, file_name);


    } else if( strcmp(process_api, GET_LIST_FILE) == 0) {



        snprintf(server_api, sizeof(server_api) ,"%s", API_ROUTER_LIST_FILE);

        status = call_cloud_api(API_ROUTER_LIST_FILE , NULL, NULL, 
                                         devicemd5mac, file_path, file_name);


    } else if( strcmp(process_api, GET_FILE_INFO) == 0) {


        snprintf(server_api, sizeof(server_api) ,"%s", API_ROUTER_GET_FILE);

        status = call_cloud_api(API_ROUTER_GET_FILE , NULL, NULL, 
                                         devicemd5mac, file_path, file_name);
    }


    if(status != 0) {
        Cdbg(APP_DBG, "Post [%s] API message error, status = %d", server_api, status);
        exit(0);
    } else if(api_response_data.status != 0) {
        Cdbg(APP_DBG, "[%s] get data error, status = %d", server_api, api_response_data.status);
        exit(0);
    }



    return 0;
}

 
void sleep_time(int sleep_time) {

    sleep_stop = 0;
    int i;
    for(i = 0; i < sleep_time; i++) {
        sleep(1);
        // Cdbg(APP_DBG, "sleep count = %d", i);
        if(sleep_stop) {
            break;
        }
    }

}

int update_userticket()
{
    int update_status;
    char event[AAE_MAX_IPC_PACKET_SIZE];
    char out[AAE_MAX_IPC_PACKET_SIZE];
    
    if(strlen(nvram_safe_get("oauth_dm_refresh_ticket")) == 0)  //APP not registered.
    {
        return -1;
    }

    Cdbg(APP_DBG, "Update userticket!");
    snprintf(event, sizeof(event), AAE_DDNS_GENERIC_MSG, AAE_EID_DDNS_REFRESH_TOKEN);
    
    aae_sendIpcMsgAndWaitResp(MASTIFF_IPC_SOCKET_PATH, event, strlen(event), out, sizeof(out), 10);
    
    Cdbg(APP_DBG, "Update userticket  out -> %s", out);

    json_object *root = NULL;
    json_object *ddnsObj = NULL;
    json_object *eidObj = NULL;
    json_object *stsObj = NULL;
    root = json_tokener_parse((char *)out);
    json_object_object_get_ex(root, AAE_DDNS_PREFIX, &ddnsObj);
    json_object_object_get_ex(ddnsObj, AAE_IPC_EVENT_ID, &eidObj);
    json_object_object_get_ex(ddnsObj, AAE_IPC_STATUS, &stsObj);
    if (!ddnsObj || !eidObj || !stsObj)
    {
        Cdbg(APP_DBG, "Failed to aae_refresh_ticket\n");
    }
    else
    {
        int eid = json_object_get_int(eidObj);
        const char *status = json_object_get_string(stsObj);
        if ((eid == AAE_EID_DDNS_REFRESH_TOKEN) && (!strcmp(status, "0")))
        {
            Cdbg(APP_DBG, "Success to aae_refresh_ticket\n");
            update_status = 0;
        }
        else
        {
            Cdbg(APP_DBG, "Failed to aae_refresh_ticket\n");
            update_status = -1;
        }
    }
    json_object_put(root);


    if(update_status == 0) {
        snprintf(oauth_dm_cusid, sizeof(oauth_dm_cusid), "%s", nvram_safe_get("oauth_dm_cusid"));
        snprintf(oauth_dm_user_ticket, sizeof(oauth_dm_user_ticket), "%s", nvram_safe_get("oauth_dm_user_ticket"));
        Cdbg(APP_DBG, "oauth_dm_cusid = %s", oauth_dm_cusid);
        Cdbg(APP_DBG, "oauth_dm_user_ticket = %s", oauth_dm_user_ticket);
    }
/*  stop_mastiff();
    start_mastiff();
    sleep(1);*/
    
    return update_status;    
}

void data_test() {

    char *arrrr[] = { "usericon_0C9D924F2BC5_01.cfg", "usericon_4CEDFB4C28A0_02.cfg", "usericon_4CEDFB4C28A0_03.cfg", "usericon_4CEDFB4C28A0_04.cfg", "usericon_4CEDFB4C28A0_05.tar.gz", "ipsec_4CEDFB4C28A0_06.tar.gz", "openvpn_4CEDFB4C28A0_07.tar.gz", "setting_4CEDFB4C28A0_08.cfg", "setting_4CEDFB4C28A0_09.cfg", "usericon_4CEDFB4C28A0_10.tar.gz", "usericon_4CEDFB4C28A0_11.tar.gz", "setting_4CEDFB4C28A0_12.cfg" };
    int i;

    for (i = 0; i < 12; i++) {
        cloud_files_init(arrrr[i]);
    }

    print_cloud_files_list(files_list_frist);
}



#define UPLOADER_SIG_ACTION SIGUSR1

enum {
    UPLOADER_ACTION_SIGUSR1,
};


static void uploader_sig_handler(int sig) {

    Cdbg(APP_DBG, "Get uploader sig = %d", sig);

    switch (sig) {

        case UPLOADER_SIG_ACTION: {
            sleep_stop = 1;

            Cdbg(APP_DBG, "reset sleep time");
        }

        break;
    }
}



int main( int argc, char ** argv )
{

   // Open Debug Log
    CF_OPEN(APP_LOG_PATH,  FILE_TYPE | CONSOLE_TYPE | SYSLOG_TYPE);
    // CF_OPEN(UL_DEBUG_TO_FILE, FILE_TYPE );
    // CF_OPEN(UL_DEBUG_TO_CONSOLE,  CONSOLE_TYPE | SYSLOG_TYPE | FILE_TYPE);

    init_basic_data();

    // receive uploader sig = SIGUSR1 = 10
    signal(UPLOADER_SIG_ACTION, uploader_sig_handler);

    // ipc : waiting call 
    uploader_ipc_start();

    int i = 0, while_count = 0;
    int status;

    // uploader start:  backup [ui_support] config
    nvram_set_int("ui_support_update_time", (unsigned )time(NULL));


    files_list_frist = backup_file_init();

    // data_test();

    // mac info : get cloud file list
    get_cloud_file_list();

    // print_cloud_files_list(files_list_frist);


    do {
        while_count++;
        // Handling redundant data
        reorganization_cloud_files();

#ifdef RTCONFIG_AMAS_CENTRAL_CONTROL
        backup_cfg_mnt();
#endif

        for(i = 0; i < TYPE_NUMBER; i++) {

            char type_update_name[FILENAME_LEN], type_upload_name[FILENAME_LEN];
            snprintf(type_update_name, sizeof(type_update_name), "%s_update_time", backup_file_type[i]);
            snprintf(type_upload_name, sizeof(type_upload_name), "%s_upload_time", backup_file_type[i]);
            int backup_file_update_time = nvram_get_int(type_update_name);
            int backup_file_upload_time = nvram_get_int(type_upload_name);

            Cdbg(APP_DBG, "type_update_name = %s, type_upload_name = %s, backup_file_update_time = %d, backup_file_upload_time = %d", 
                type_update_name, type_upload_name, backup_file_update_time, backup_file_upload_time);

            char filename[FILENAME_LEN] = {0};

            // upload [db type] update time (nvram update time > last upload time )
            if( backup_file_update_time > backup_file_upload_time ) {
                // type if// 
                if( local_backup_file_output(backup_file_type[i], filename, FILENAME_LEN, backup_file_update_time) == PROCESS_SUCCESS) {

                    // backup file upload
                    if(upload_backup_file_to_cloud(filename) == 0) {
                        backup_file_plus(backup_file_type[i], filename);
                    }

                    char filepath[128] = {0};
                    snprintf(filepath, sizeof(filepath), "%s%s", UPLOADER_FOLDER, filename);

                    if(!remove(filepath)) {
                      Cdbg(APP_DBG, "remove file = %s", filepath);
                    }
                }

                nvram_set_int(type_upload_name, backup_file_update_time);
            }
        }

        // External Backup File Upload (Scan db file every 30 seconds)
        ex_db_backup();

        sleep_time(UPLOAD_WAITING_TIME);

    } while (1);


    free_files_list(files_list_frist);

    Cdbg(APP_DBG, "exit uploader, waiting restart");

    return 0;
}