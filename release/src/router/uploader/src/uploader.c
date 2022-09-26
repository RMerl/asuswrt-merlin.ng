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


#include <assert.h>
// #include <time.h>
// #include <getopt.h>


#include "log.h"
#include "api.h"
#include "uploader.h"

#include "upload_api.h"

#include "uploader_ipc.h"
#include "uploader_mnt.h"


#define APP_DBG 1


char device_model[32];
char aae_deviceid[64];
char oauth_dm_cusid[128];
char oauth_dm_user_ticket[64];
char router_mac[32];
char mac_no_symbol[32];

char devicemd5mac[33] = {0};


char aae_webstorageinfo[128] = {0};


 
extern struct api_response_data api_response_data;


void init_basic_data() 
{

    int ready_count = 0;

    while(1) {

        ready_count++;

        int ntp_ready = nvram_get_int("ntp_ready");

        int svc_ready = nvram_get_int("svc_ready");

        int link_internet = nvram_get_int("link_internet"); // 2 -> connected

        int oauth_auth_status = nvram_get_int("oauth_auth_status"); // 2 -> account_binding

        if((svc_ready == 1) && (ntp_ready == 1) && (link_internet == 2) && (oauth_auth_status == 2)) {
            Cdbg(APP_DBG, "svc_ready -> %d, ntp_ready -> %d, link_internet -> %d, oauth_auth_status -> %d", svc_ready, ntp_ready, link_internet, oauth_auth_status);
            break;
        } else {
            if(ready_count < 3) {
                // Cdbg(APP_DBG, "waiting svc_ready -> %d, link_internet -> %d, tencent_download_enable -> %d", svc_ready, link_internet, tencent_download_enable);
            }
        }
        sleep(30);
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



char process_api[32];
char file_name[64];
char file_path[256];
char server_api[32];


int db_file_process(const char * api, const char * file_path, const char * file_name) 
{

    int status;
    int retry_count = 0;

    do {

        status = get_aicamcloud_api_info(API_ROUTER_CHECK_TOKEN , oauth_dm_cusid, 
                                         oauth_dm_user_ticket, devicemd5mac, NULL, NULL);

        if(status != 0) {
            Cdbg(APP_DBG, "Post [router_check_token] API message error, status = %d", status);
            return -1;
        } else if(api_response_data.status != 0) {
            Cdbg(APP_DBG, "[check_token] get data error, status = %d", api_response_data.status);
            update_userticket();            
            sleep(10);
            retry_count++;
            if(retry_count > 5) {
                Cdbg(APP_DBG, "[check_token] error, update_userticket error, retry_count = %d", retry_count);
                return -1;
            }
            continue;
        }
        break;

    } while (1);

    Cdbg(APP_DBG, "API_ROUTER_CHECK_TOKEN status = %d, access_token = %s", status, api_response_data.access_token);


    if( strcmp(api, UPLOAD_FILE_TO_S3) == 0) {

        snprintf(server_api, sizeof(server_api) ,"%s", API_ROUTER_GET_UPLOAD_FILE_URL);


        status = get_aicamcloud_api_info(API_ROUTER_GET_UPLOAD_FILE_URL , NULL, NULL, 
                                         devicemd5mac, file_path, file_name);


        if(status != 0) {
            Cdbg(APP_DBG, "Post [%s] API message error, status = %d", server_api, status);
            return -1;
        } else if(api_response_data.status != 0) {
            Cdbg(APP_DBG, "[%s] get data error, status = %d", server_api, api_response_data.status);
            return -1;
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
        snprintf(upload_file, sizeof(upload_file) ,"%s/%s", file_path, file_name);


        status = upload_file_to_s3(S3_RESPONSE_FILE, upload_file, NULL);

        Cdbg(APP_DBG, "S3_RESPONSE_FILE = %s, status = %d", S3_RESPONSE_FILE, status);


        if(status != 204) {
            Cdbg(APP_DBG, "Post [upload_file_to_s3] API message error, status = %d", status);
            return -1;
        } 


        snprintf(server_api, sizeof(server_api) ,"%s", API_ROUTER_UPLOAD_FILE);


        status = get_aicamcloud_api_info(API_ROUTER_UPLOAD_FILE , NULL, NULL, 
                                     devicemd5mac, file_path, file_name);

        // char del_file[64];
        // snprintf(del_file, sizeof(del_file), "%s%s", file_path, filename);

        // remove(del_file);

        // Cdbg(APP_DBG, "remove file = %s", del_file);


    } else if( strcmp(api, DEL_S3_FILE) == 0) {


        snprintf(server_api, sizeof(server_api) ,"%s", API_ROUTER_DEL_FILE);

        status = get_aicamcloud_api_info(API_ROUTER_DEL_FILE , NULL, NULL, 
                                         devicemd5mac, file_path, file_name);


    } else if( strcmp(api, GET_LIST_FILE) == 0) {



        snprintf(server_api, sizeof(server_api) ,"%s", API_ROUTER_LIST_FILE);

        status = get_aicamcloud_api_info(API_ROUTER_LIST_FILE , NULL, NULL, 
                                         devicemd5mac, file_path, file_name);


    } else if( strcmp(api, GET_FILE_INFO) == 0) {


        snprintf(server_api, sizeof(server_api) ,"%s", API_ROUTER_GET_FILE);

        status = get_aicamcloud_api_info(API_ROUTER_GET_FILE , NULL, NULL, 
                                         devicemd5mac, file_path, file_name);
    }


    if(status != 0) {
        Cdbg(APP_DBG, "Post [%s] API message error, status = %d", server_api, status);
        return -1;
    } else if(api_response_data.status != 0) {
        Cdbg(APP_DBG, "[%s] get data error, status = %d", server_api, api_response_data.status);
        return -1;
    }



    return 0;
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



    status = get_aicamcloud_api_info(API_ROUTER_CHECK_TOKEN , oauth_dm_cusid, 
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


        status = get_aicamcloud_api_info(API_ROUTER_GET_UPLOAD_FILE_URL , NULL, NULL, 
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


        status = get_aicamcloud_api_info(API_ROUTER_UPLOAD_FILE , NULL, NULL, 
                                     devicemd5mac, file_path, file_name);




    } else if( strcmp(process_api, DEL_S3_FILE) == 0) {


        snprintf(server_api, sizeof(server_api) ,"%s", API_ROUTER_DEL_FILE);

        status = get_aicamcloud_api_info(API_ROUTER_DEL_FILE , NULL, NULL, 
                                         devicemd5mac, file_path, file_name);


    } else if( strcmp(process_api, GET_LIST_FILE) == 0) {



        snprintf(server_api, sizeof(server_api) ,"%s", API_ROUTER_LIST_FILE);

        status = get_aicamcloud_api_info(API_ROUTER_LIST_FILE , NULL, NULL, 
                                         devicemd5mac, file_path, file_name);


    } else if( strcmp(process_api, GET_FILE_INFO) == 0) {


        snprintf(server_api, sizeof(server_api) ,"%s", API_ROUTER_GET_FILE);

        status = get_aicamcloud_api_info(API_ROUTER_GET_FILE , NULL, NULL, 
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

int main( int argc, char ** argv )
{

    system("touch /tmp/UPLOADER_DEBUG_SYSLOG");

   // Open Debug Log
    CF_OPEN(APP_LOG_PATH,  FILE_TYPE | CONSOLE_TYPE | SYSLOG_TYPE);
    // CF_OPEN(UL_DEBUG_TO_FILE, FILE_TYPE );
    CF_OPEN(UL_DEBUG_TO_CONSOLE,  CONSOLE_TYPE | SYSLOG_TYPE | FILE_TYPE);

    if(APP_DBG) {
        write_file(UL_DEBUG_TO_FILE, " ");
    }


    init_basic_data();

        // ipc : waiting call 
    uploader_ipc_start();


    char setting_update_time_tmp[16] = {0};
    getTimeInMillis(setting_update_time_tmp);
    nvram_set("setting_update_time", setting_update_time_tmp);


    int status = 0;

    do {

        status = search_local_db_file();

        if(status == 0) {

            status = check_file_list(oauth_dm_cusid, oauth_dm_user_ticket, 
                             devicemd5mac, NULL, NULL);
        }


        if(strcmp(nvram_safe_get("setting_update_time"), setting_update_time_tmp) != 0) {

            Cdbg(APP_DBG, "setting_update_time value change %s > %s", setting_update_time_tmp, nvram_safe_get("setting_update_time"));

            snprintf(setting_update_time_tmp, sizeof(setting_update_time_tmp), "%s", nvram_safe_get("setting_update_time"));
            setting_file_output();
        }


        sleep(15);


    } while (1);



    // do {

    //     char file_path[16] = "/tmp/";
    //     char filename[32] = {0};


    //     char timestamp_str[16] = {0};
    //     getTimeInMillis(timestamp_str);

    //     snprintf(filename, sizeof(filename), "settings_%s", timestamp_str);

    //     Cdbg(APP_DBG, "filename = %s", filename);


    //     char settings_cmd[128] = {0};
    //     snprintf(settings_cmd, sizeof(settings_cmd), "nvram save %s%s", file_path, filename);


    //     Cdbg(APP_DBG, "settings_cmd = %s", settings_cmd);
    //     // remove(settings_date);

    //     system(settings_cmd);

    //     status = upload_file(oauth_dm_cusid, oauth_dm_user_ticket, 
    //                          devicemd5mac, file_path, filename);


    //     if(status == 0) {

    //         status = check_file_list(oauth_dm_cusid, oauth_dm_user_ticket, 
    //                          devicemd5mac, file_path, filename);


    //         }


    //     char del_file[64];
    //     snprintf(del_file, sizeof(del_file), "%s%s", file_path, filename);

    //     remove(del_file);

    //     Cdbg(APP_DBG, "remove file = %s", del_file);

    //     sleep(60);

    // } while (1);




    Cdbg(APP_DBG, "exit uploader, waiting restart");
    Cdbg2(APP_DBG, 1, "exit uploader, waiting restart");

    return 0;
}