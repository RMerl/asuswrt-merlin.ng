/* uploader.c: display a message on the screen */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <dirent.h>
#include <signal.h>
#include <sys/stat.h>
#include <ftw.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <setjmp.h>
#include <curl/curl.h>



//#define IPKG 0

//#ifdef DEBUG
//#include <mcheck.h>
//#endif

//#ifndef IPKG
#include <bcmnvram.h>
//#include <shutils.h>
//#endif
#include <sys/time.h>
#include "api.h"
#include "function.h"
#include "data.h"
#include "list.h"
#include "mem_pool.h"
#include "log.h"

#define APP_DBG 1


#define MAXDATASIZE 1024
#define MYPORT 3567
#define INOTIFY_PORT 3678
#define BACKLOG 100 /* max listen num*/
//#define CONFIG_PATH "/home/gauss/Cloud/Cloud.conf"

#define SEM 0

#define UP_FAIL   10
#define DOWN_FAIL 11
#define SOCKET_FAIL 12
#define RETRY_MAX_TIME 1
#define PASSWORD_SIZE 128


int stop_sync_server;
int pre_seq = -10;
Servicegateway sergate;
Aaa aaa;
//int sync_up;
//int sync_down;
int stop_up = 0;
int stop_down = 0 ;
int local_sync;
int server_sync;
int server_modify;
int MySyncFolder;
int MyRecycleID;
char username[256];
char password[256];
//char otp_key[8];
//char captcha[8];
char base_path[256];


char device_id[48], aicam_model[12];
char ac_tsdb_type[32] , type_value1[32], type_value2[32], type_value3[32];

char dbg_msg[256];


sync_item_t from_server_sync_head = NULL;
sync_item_t down_head = NULL;
sync_item_t up_head = NULL;
sync_item_t up_excep_fail = NULL;
sync_item_t download_only_socket_head = NULL;
sync_item_t download_only_modify_head = NULL;
sync_item_t copy_file_list_head = NULL;
int exit_proc = 0;
//pthread_t newthid1,newthid2,newthid3;
pthread_t server_thread,local_thread,socket_thread,download_socket_thread;
int no_config;
int restart_run;
int stop_progress = 0;
int exit_loop = 0;
//int uploading = 0;
//int downloading = 0;
int retry_fail_time = 0;
my_mutex_t my_mutex = {PTHREAD_MUTEX_INITIALIZER,PTHREAD_COND_INITIALIZER,0};
//my_mutex_t wait_sleep_mutex = {PTHREAD_MUTEX_INITIALIZER,PTHREAD_COND_INITIALIZER,0};
my_mutex_t wait_server_mutex = {PTHREAD_MUTEX_INITIALIZER,PTHREAD_COND_INITIALIZER,0};
my_mutex_t wait_socket_mutex = {PTHREAD_MUTEX_INITIALIZER,PTHREAD_COND_INITIALIZER,0};
pthread_mutex_t mutex_socket = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_receve_socket = PTHREAD_MUTEX_INITIALIZER;
int upload_only = 0;
int download_only = 0;
s_tree *s_link = NULL;
Hb_TreeNode *DirRootNode = NULL;
Server_TreeNode  *SRootNode = NULL;
Hb_SubNode *SyncNode;
//char uploadfile_path[NORMALSIZE];
//char downloadfile_path[NORMALSIZE];
int server_space_full = 0;
int local_space_full = 0 ;
//int file_iscopying = 0 ;
int has_del_fiel = 0;
int has_socket = 0;
int max_upload_filesize;
struct asus_config cfg;
int no_local_root = 0; /* handle rm sync root dir*/
int mysync;
int copying_file_number =0;
int has_local_socket = 0 ;
int is_server_running = 0;
int IsAccountFrozen = 0;
int IsSyncError = 0;
api_count_s api_count;
//int IsSleep;
int loop_max;
int usleep_time;
int IsNetworkUnlink = 0;
queue_t SocketActionList;

char mount_path[NAMESIZE];
char cloud_path[NAMESIZE];
char asus_path[NAMESIZE];
char log_path[NAMESIZE];
char xml_path[NAMESIZE];
char sync_path[NAMESIZE];
char system_log[NAMESIZE];
char general_log[NAMESIZE];
char confilicted_log[NAMESIZE];
char up_item_file[NAMESIZE];
char down_item_file[NAMESIZE];

#if WRITE_DOWNLOAD_TEMP_FILE
char down_item_temp_file[NAMESIZE];
#endif

char up_excep_fail_file[NAMESIZE];
char gateway_xml[NAMESIZE];
char get_user_state_xml[NAMESIZE];
char token_xml[NAMESIZE];
char get_info_xml[NAMESIZE];
char get_sync_folder_xml[NAMESIZE];

char get_init_attachable_device_xml[NAMESIZE]; // add by markcool
char get_browse_attachable_device_xml[NAMESIZE]; // add by markcool
char get_send_aws_email_xml[NAMESIZE]; // add by markcool

char get_personal_system_folder_xml[NAMESIZE];
char browse_folder_xml[NAMESIZE];

char sliding_browse_file_xml[NAMESIZE]; // add by markcool

char propfind_xml[NAMESIZE];
char create_folder_xml[NAMESIZE];
char rename_xml[NAMESIZE];
char move_xml[NAMESIZE];
char remove_xml[NAMESIZE];
char update_xml[NAMESIZE];
char get_entry_info_xml[NAMESIZE];
char get_change_seq_xml[NAMESIZE];
char access_token_json[NAMESIZE];
char search_drive_file_json[NAMESIZE];
char create_folder_json[NAMESIZE];

char init_upload_xml[NAMESIZE];
char resume_upload_xml[NAMESIZE];
char finish_upload_xml[NAMESIZE];
char trans_excep_file[128];
char system_token[256];
int receve_socket;
int is_check_token;
int no_completed;

//int SocketNumber =1;
//int IsNetworkLinkDown;
// token file var
#ifdef IPKG
int disk_change;
int sync_disk_removed;
char disk_info_filename[128];
char record_token_file[256];
char token_filename[256];
#endif

void *SyncServer();
void *SyncLocal();
void sig_handler (int signum);
int cmd_parser(char *command);
//int download_only_cmd_parser(char *command,Socket_cmd *socket_cmd);
int send_action(int type,char *content);
void init_global_var();
void clean_global_var();
//void init_sync_folder();
void clean_up();
//void run();
int aiCamRun();
int oauth_token;
int read_config();
void handle_fail_item();
void* sigmgr_thread();

//int update_mount_path_to_nvram();
char *get_mount_path(char *path, int n);
void create_parent_folder_r(char *path);
//int update_mount_path_to_nvram(char *new_sync_path);

// void checkAaewsRun();

int tsdbProcess(int status, char * msg);
void initTcodeParam();
int delOverSpaceFile(int del_day);
long long current_time_with_ms (char * folder_date);

#ifdef IPKG
int write_notify_file(char *path, int signal_num);
#endif

//static int trim_sync_path(char *in,char *out);

// start : modify by markcool 
#include "uploader_config.h"
#include "des.h"


#define DEBUG 1
#define AICAM 1

int upload_type = 0;    // 0 : webstorage , 1 : SD, 2 : aiCloud
// int tcode_channel = 0;
int space_plan = 0;

int space_plan_tmp = -9;

int system_debug_status = 0;

char tcode_channel[3];
char tcode_country[5];

struct google_conf google_d_conf;


struct asuswebstorage_conf asus_conf;
struct aicloud_conf ai_conf;
struct storage_provision_conf sp_conf;
struct aaews_provision_conf ap_conf;
struct sysinfo_conf si_conf;
struct deploy_conf dp_conf;
struct basic_command_conf bc_conf;
struct system_conf s_conf;

struct aaews_event_conf ae_conf;
// end : modify by markcool 

RouterInfo router_info;

void spacePlanDir();

void timePlanDir();


long long aiCamID;

long long aiCamV2ID = 0;


long long aiCamDeviceID;


long long aiCamMacAddressOneID = 0;
long long aiCamMacAddressTwoID = 0;
long long aiCamMacAddressThreeID = 0;
long long aiCamMacAddressFourID = 0;

long long aiCamMacAddressOneAlarmID = 0;
long long aiCamMacAddressTwoAlarmID = 0;
long long aiCamMacAddressThreeAlarmID = 0;
long long aiCamMacAddressFourAlarmID = 0;

long long aiCamMacAddressOneAlarmDateID = 0;
long long aiCamMacAddressTwoAlarmDateID = 0;
long long aiCamMacAddressThreeAlarmDateID = 0;
long long aiCamMacAddressFourAlarmDateID = 0;


long long aiCamMacAddressOneRecordID = 0;
long long aiCamMacAddressTwoRecordID = 0;
long long aiCamMacAddressThreeRecordID = 0;
long long aiCamMacAddressFourRecordID = 0;

long long aiCamMacAddressOneRecordDateID = 0;
long long aiCamMacAddressTwoRecordDateID = 0;
long long aiCamMacAddressThreeRecordDateID = 0;
long long aiCamMacAddressFourRecordDateID = 0;


char aiCamMacAddressOne[32]; 
char aiCamMacAddressTwo[32]; 
char aiCamMacAddressThree[32]; 
char aiCamMacAddressFour[32]; 


char macVideoModeOne[3]; 
char macVideoModeTwo[3]; 
char macVideoModeThree[3]; 
char macVideoModeFour[3]; 


long long aiCamDeviceAlarmID;
long long aiCamDeviceAlarmDateID;

long long aiCamDeviceRecordID;
long long aiCamDeviceRecordDateID;

long long aiCamDeviceVideoID;
long long aiCamDeviceTmpID;

char time_now[12]; // markcool add
char aicam_device_dir[20]; 

void search_video(char * dirType, char * mac_name, int mac_type);
//int toolCurl(int api_id,char* api_url, char* authorization, char* cookie, char* payload, char* action, char* service, char* sid, char* token);
void initAiCamConfig();
void initRouterConfig();
void delExpiredFile();
void search_video_dir();
void search_mac_dir();
void search_mac_config();

long long createWebStorageMacDir(char * mac_name, int mac_type);
long long createWebStorageMacAlarmDir(char * mac_name, int mac_type);
long long createWebStorageMacRecordDir(char * mac_name, int mac_type);
void getMacAlarmDateID(char * mac_name, int mac_type);
void getMacRecordDateID(char * mac_name, int mac_type);
int getAiCamTempID(int mac_type, int process_type);

void dirFileUpload(char *filename, char * dirType, int file_type, char * mac_name, int mac_type);


void datecmp(char * filename, char * dirType, char * mac_name, int mac_type);

void driveDatecmp();

int get_correct_id(int macDirNumber);

char config_upload_path[128];
char config_download_path[128];


#define NVRAM_LINK_INTERNET "link_internet"
#define NVRAM_GOOGLE_DRIVE_ACCESS_TOKEN "oauth_google_drive_access_token"
#define NVRAM_GOOGLE_DRIVE_REFRESH_TOKEN "oauth_google_drive_refresh_token"
#define OAUTH_GOOGLE_CLIENT_ID "103584452676-437qj6gd8o9tuncit9h8h7cendd2eg58.apps.googleusercontent.com"
#define OAUTH_GOOGLE_CLIENT_SECRET "xivDhVGSSHZ3LJMx228wdcDf"


int main(int argc, char *argv[]) {

    // Open Log
    CF_OPEN(APP_LOG_PATH, SYSLOG_TYPE | FILE_TYPE | CONSOLE_TYPE);


    if(APP_DBG) {

        debugFileWrite();

    }
    // int i;
    // for(i = 0; i < argc; ++i) {
    //     Cdbg(APP_DBG, "[%d] %s\n", i, argv[i]);      
    // }

    // waiting internet connect
    if((strlen(nvram_safe_get(NVRAM_LINK_INTERNET)) != 0)) {
            
        if( strcmp(nvram_safe_get(NVRAM_LINK_INTERNET),"2") == 0) {

            Cdbg(APP_DBG, "LINK_INTERNET = 2, success");

        } else {
            sleep(10);
        }

    }

    if(argc > 1) {

        if(argc == 3) {
            int sleep_time = atoi(argv[2]);
            Cdbg(APP_DBG, "uploader sleep : %d s", sleep_time);
            sleep(sleep_time);
        }

        int status = uploader_config_process(argv[1]);

        if(status != 0) {

            Cdbg(APP_DBG, "uploader config error , exit");
            exit(0);

        }
    } else {

        Cdbg(APP_DBG, "config file read error, exit");
        exit(0);
    }


    while(1) {





        if((strlen(nvram_safe_get(NVRAM_GOOGLE_DRIVE_ACCESS_TOKEN)) != 0)  && 
           (strlen(nvram_safe_get(NVRAM_GOOGLE_DRIVE_REFRESH_TOKEN)) != 0)) {
      
            Cdbg(APP_DBG, "NVRAM_GOOGLE_DRIVE_ACCESS_TOKEN : %s", nvram_safe_get(NVRAM_GOOGLE_DRIVE_ACCESS_TOKEN));
            Cdbg(APP_DBG, "NVRAM_GOOGLE_DRIVE_REFRESH_TOKEN : %s", nvram_safe_get(NVRAM_GOOGLE_DRIVE_REFRESH_TOKEN));

            strcpy(google_d_conf.client_id, OAUTH_GOOGLE_CLIENT_ID);
            strcpy(google_d_conf.client_secret, OAUTH_GOOGLE_CLIENT_SECRET);
            strcpy(google_d_conf.access_token, nvram_safe_get(NVRAM_GOOGLE_DRIVE_ACCESS_TOKEN));
            strcpy(google_d_conf.refresh_token, nvram_safe_get(NVRAM_GOOGLE_DRIVE_REFRESH_TOKEN));

            break;

        } else {

            Cdbg(APP_DBG, "GOOGLE_DRIVE_REFRESH_TOKEN is null, sleep 60s, drive oauth waiting ...");

            sleep(60);
            //exit(-1);
        }

    }


    // read aicam dir param, frist
    //initGoogleToken(&google_d_conf);

    initRouterConfig();


    routerRun();
    // aiCamRun();


    // log close
    close_log();

    return 0;
}

void *SyncServer()
{
    Cdbg(APP_DBG, "sync server Thread start\n");

    pre_seq = -10 ;
    int cur_seq = 0;
    int sync_fail = 0;

    while(!exit_loop)
    {
        while(local_sync && !exit_loop)
            enter_sleep_time(1000*10,NULL);
        server_sync = 1;
        //write_system_log("SyncServer","start");

        if(local_sync == 0 && SocketActionList->head == NULL)
        {
#ifdef IPKG
            if(disk_change)
            {
                pthread_mutex_lock(&mutex_socket);
                disk_change = 0;
                pthread_mutex_unlock(&mutex_socket);
                check_disk_change();
            }
#endif

            Changeseq *cs;
            int status;
            cs = getChangeSeq(MySyncFolder);
            if(NULL == cs)
            {
                enter_sleep_time(1000*100,NULL);
                check_network_state();
                Cdbg(APP_DBG, "########server get seqnumber fail########\n");
                continue;
            }
            if(cs->status == 2)
            {
#if SYSTEM_LOG
               write_system_log("GetChageSeq","Auth Fail");
#endif
                my_free(cs);

                status = getToken(username,password,"",0);

                if(status != 0)
                {
                    if( status != S_NETWORK_FAIL)
                        exit_loop = 1;
                    continue;
                }

            }
            else if(cs->status == 0)
            {
                cur_seq = cs->changeseq;
                if(local_sync == 0)
                    handle_fail_item();

                if( cur_seq != pre_seq || sync_fail == 1 || pre_seq == -10)
                {
                    Cdbg(APP_DBG, "pre seq_number is %d,cur seq number is %d\n",pre_seq,cur_seq);
//#ifdef DEBUG
                    Cdbg(APP_DBG, "#### sync server start#####\n");
//#endif

#if SYSTEM_LOG
                    write_system_log("sync server","start");
#endif
                    is_server_running = 1;
          if(pre_seq == -10)
                        SyncNode = create_node(MySyncFolder,-10);
                    status = syncServerAllItem(username,MySyncFolder,sync_path);
                    is_server_running = 0 ;                  
                    write_finish_log();

//#ifdef DEBUG
                    print_all_nodes(SyncNode);
                    Cdbg(APP_DBG, "#### sync server end#####\n");
//#endif

#if SYSTEM_LOG
                    write_system_log("sync server","end");
#endif
                    if(status == -1)
                    {
                        if(pre_seq == -10)
                            free_node(SyncNode);
                        sync_fail = 1;
                        enter_sleep_time(1000*100,NULL);
                    }
                    else
                    {
                        sync_fail = 0;
                    }

                    print_count_call_api();
                }

                if(sync_fail != 1)
                {
                    pre_seq = cur_seq;
                }

                //Cdbg(APP_DBG, "#### sync server end final#####\n");

            }
            else
            {
                handle_error(cs->status,"get seqnumber");
            }
            my_free(cs);

        }
        server_sync = 0;
        //write_system_log("SyncServer","end");
        enter_sleep_time(30,&wait_server_mutex);
    }

    Cdbg(APP_DBG, "stop asuswebstorage server sync\n");

    stop_up = 1;

    return NULL;
}

int download_only_add_socket_item(Socket_cmd *socket_cmd)
{
    char fullname[512];
    char old_fullname[512];
    char cmd_name[32];
    char action[32];

    memset(cmd_name,0,sizeof(cmd_name));
    memset(fullname,0,sizeof(fullname));
    memset(old_fullname,0,sizeof(old_fullname));
    memset(action,0,sizeof(action));

    strcpy(cmd_name,socket_cmd->cmd_name);

    if( !strncmp(cmd_name,"copyfile",strlen("copyfile")) )
    {
        sprintf(fullname,"%s/%s",socket_cmd->path,socket_cmd->filename);
        add_sync_item("copyfile",fullname,copy_file_list_head);
        return 0;
    }
    else if( !strncmp(cmd_name,"rename",strlen("rename")) )
    {
        snprintf(fullname,512,"%s/%s",socket_cmd->path,socket_cmd->newname);
        snprintf(old_fullname,512,"%s/%s",socket_cmd->path,socket_cmd->oldname);
    }
    else if( !strncmp(cmd_name,"move",strlen("move")) )
    {
        snprintf(fullname,512,"%s/%s",socket_cmd->path,socket_cmd->oldname);
        snprintf(old_fullname,512,"%s/%s",socket_cmd->oldpath,socket_cmd->oldname);
    }
    else
    {
        snprintf(fullname,512,"%s/%s",socket_cmd->path,socket_cmd->filename);
    }

    if( strcmp(cmd_name, "createfile") == 0 )
    {
        strcpy(action,"createfile");
        struct sync_item* item;

        item = get_sync_item("copyfile",fullname,copy_file_list_head);

        if(item != NULL)
        {
            del_sync_item("copyfile",fullname,copy_file_list_head);
        }
    }
    else if( strcmp(cmd_name, "remove") == 0  || strcmp(cmd_name, "delete") == 0)
    {
        strcpy(action,"remove");
        del_download_only_sync_item(action,fullname,download_only_socket_head);
        del_download_only_sync_item(action,fullname,download_only_modify_head);
    }
    else if( strcmp(cmd_name, "createfolder") == 0 )
    {
        strcpy(action,"createfolder");
    }
    else if( strcmp(cmd_name, "rename") == 0 )
    {
        strcpy(action,"rename");
        del_download_only_sync_item(action,old_fullname,download_only_socket_head);
        del_download_only_sync_item(action,fullname,download_only_modify_head);
    }
    else if( strcmp(cmd_name, "move") == 0 )
    {
        strcpy(action,"move");
        del_download_only_sync_item(action,old_fullname,download_only_socket_head);
        del_download_only_sync_item(action,fullname,download_only_modify_head);
    }
    else if( strcmp(cmd_name, "modify") == 0 )
    {
        strcpy(action,"modify");
    }

    if(from_server_sync_head->next != NULL)
    {
        sync_item_t item = get_sync_item(action,fullname,from_server_sync_head);

        if(item != NULL)
        {
            del_sync_item(action,fullname,from_server_sync_head);
            return 0;
        }

    }

    if(strcmp(cmd_name, "rename") == 0 || strcmp(cmd_name, "move") == 0)
    {
        if(test_if_dir(fullname))
        {
            add_all_download_only_socket_list(cmd_name,fullname);
        }
        else
        {
            add_sync_item(cmd_name,fullname,download_only_socket_head);
        }
    }
    else if(strcmp(cmd_name, "createfolder") == 0 || strcmp(cmd_name, "dragfolder") == 0)
    {
        add_sync_item(cmd_name,fullname,download_only_socket_head);
        if(!strcmp(cmd_name, "dragfolder"))
            add_all_download_only_dragfolder_socket_list(fullname);
    }
    else if( strcmp(cmd_name, "createfile") == 0  || strcmp(cmd_name, "dragfile") == 0 || strcmp(cmd_name, "modify") == 0)
    {
        add_sync_item(cmd_name,fullname,download_only_socket_head);
    }
    else if( strcmp(cmd_name, "modify") == 0)
    {
        sync_item_t item = get_sync_item(action,fullname,download_only_socket_head);

        if(item == NULL)
        {
            add_sync_item(cmd_name,fullname,download_only_modify_head);
        }
    }

    return 0;
}

int trim_sync_path(char *in,char *out)
{
    int len = 0;
    char *p = NULL;
    char prefix[1024] = {0};
    char new_buf[1024] = {0};
    char tail[1024] = {0};

    len = strlen(sync_path);
    p = strstr(in,sync_path);
    if(p != NULL)
    {
        strncpy(prefix,in,strlen(in)-strlen(p));
        strcpy(tail,p+len);

        if(p[len] == '\n')
            sprintf(new_buf,"%s/%s",prefix,tail);
        else
            sprintf(new_buf,"%s%s",prefix,tail);

        strcpy(out,new_buf);

        //Cdbg(APP_DBG, "new_buf=%s\n",out);
    }
    else
        strcpy(out,in);

    return 0;
}

int add_socket_item(char *in)
{
    //Cdbg(APP_DBG, "@@@@@@@@@@@ add socket=%s\n",in);
    int len;
    char buf[1024] = {0};
    char move_buf[1024] = {0};

    trim_sync_path(in,buf); // trim sync path

    if(!strncmp(in,"move",4))
    {
       strcpy(move_buf,buf);
       memset(buf,0,1024);
       trim_sync_path(move_buf,buf);
    }

    pthread_mutex_lock(&mutex_socket);
    queue_entry_t SocketActionTmp;

#if MEM_POOL_ENABLE
    SocketActionTmp = mem_alloc(8);
#else
    SocketActionTmp = malloc (sizeof (struct queue_entry));
#endif
    if(SocketActionTmp == NULL)
    {
        Cdbg(APP_DBG, "SocketActionTmp momery not enough\n");
        return -1;
    }
    memset(SocketActionTmp,0,sizeof(struct queue_entry));
    len = strlen(buf)+1;

#if MEM_POOL_ENABLE
    SocketActionTmp->cmd_name = mem_alloc(len);
#else
    SocketActionTmp->cmd_name = (char *)calloc(len,sizeof(char));
#endif
    if(SocketActionTmp->cmd_name == NULL)
    {
        Cdbg(APP_DBG, "momery not enough\n");
        return -1;
    }
    memset(SocketActionTmp->cmd_name,0,len);
    sprintf(SocketActionTmp->cmd_name,"%s",buf);


    queue_enqueue(SocketActionTmp,SocketActionList);
    pthread_mutex_unlock(&mutex_socket);

    return 0;
}

void *SyncLocal()
{
    Cdbg(APP_DBG, "sync local Thread start\n");

    int sockfd;
    int new_fd;
    int numbytes;
    char buf[MAXDATASIZE];
    int yes = 1;

    struct sockaddr_in my_addr; /* my address information */
    struct sockaddr_in their_addr; /* connector's address information */
    socklen_t sin_size;

    fd_set read_fds;
    fd_set master;
    int fdmax;
    struct timeval timeout;
    int ret;

    FD_ZERO(&read_fds);
    FD_ZERO(&master);

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        return NULL;
    }

    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
        perror("Server-setsockopt() error lol!");
        return NULL;
    }

    my_addr.sin_family = AF_INET; /* host byte order */
    my_addr.sin_port = htons(MYPORT); /* short, network byte order */
    my_addr.sin_addr.s_addr = INADDR_ANY; /* auto-fill with my IP */
    bzero(&(my_addr.sin_zero), sizeof(my_addr.sin_zero)); /* zero the rest of the struct */

    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct
                                                         sockaddr))== -1) {
        perror("bind");
        return NULL;
    }
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        return NULL;
    }
    sin_size = sizeof(struct sockaddr_in);    //add by alan

    FD_SET(sockfd,&master);
    fdmax = sockfd;

    while(!exit_loop)
    { /* main accept() loop */
        timeout.tv_sec = 0;
        timeout.tv_usec = 1000*100;

        read_fds = master;

        ret = select(fdmax+1,&read_fds,NULL,NULL,&timeout);
        switch (ret)
        {
        case 0:
            continue;
            break;
        case -1:
            perror("select");
            continue;
            break;
        default:
            if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, \
                                 &sin_size)) == -1) {
                perror("accept");
                continue;
            }

            memset(buf, 0, sizeof(buf));

            if ((numbytes=recv(new_fd, buf, MAXDATASIZE, 0)) == -1) {
                perror("recv");
                continue;
            }

            if(buf[strlen(buf)] == '\n')
            {
                buf[strlen(buf)] = '\0';
            }

            close(new_fd);  //add by alan

            pthread_mutex_lock(&mutex_receve_socket);
            receve_socket = 1;
            pthread_mutex_unlock(&mutex_receve_socket);
           
            add_socket_item(buf);

        }
    }

    Cdbg(APP_DBG, "stop asuswebstorage local sync\n");

    close(sockfd);

    stop_down = 1;
    return NULL;
}

void *Socket_Parser(){
    Cdbg(APP_DBG, "*******Socket_Parser start********\n");
    queue_entry_t socket_execute;
    int status = 0;
    mysync = 1;
    char buf[1024] = {0};

    while(!exit_loop)
    {
        while(server_sync && !exit_loop)
            enter_sleep_time(1000*100,NULL);
        local_sync = 1;

        //write_system_log("Socket_Parser","start");
        if(no_local_root == 1)
        {
            my_mkdir(sync_path);
            send_action(1,sync_path);
            if(upload_only)
                mysync = 1;
            else
                pre_seq = -10;
            no_local_root = 0;
            continue;
        }

#ifdef IPKG
        if(disk_change)
        {
            pthread_mutex_lock(&mutex_socket);
            disk_change = 0;
            pthread_mutex_unlock(&mutex_socket);
            check_disk_change();
        }
#endif
        if(upload_only == 1 && mysync == 1)
        {
                Cdbg(APP_DBG, "upload only mysync start\n");
                status = syncServerAllItem(username,MySyncFolder,sync_path);
                Cdbg(APP_DBG, "upload only mysync end,status is %d\n",status);
                write_log(S_SYNC,"","");
                if(status == 0)
                    mysync = 0;
        }

        while(!exit_loop) //if file is copying ,don't exit local sync
        {
            if(upload_only)
            {
                handle_fail_item();
            }

            while(!exit_loop)
            {
                has_socket = 1;
                pthread_mutex_lock(&mutex_socket);
                if(SocketActionList->head == NULL)
                {
                    pthread_mutex_unlock(&mutex_socket);
                     break;
                }
                socket_execute = SocketActionList->head;
                memset(buf,0,sizeof(buf));
                strncpy(buf,socket_execute->cmd_name,1024);
                pthread_mutex_unlock(&mutex_socket);

                if(cmd_parser(buf) == 0)
                {
                    pthread_mutex_lock(&mutex_socket);
                    socket_execute = queue_dequeue(SocketActionList);

#if MEM_POOL_ENABLE
                    mem_free(socket_execute->cmd_name);
                    mem_free(socket_execute);
#else
                    my_free(socket_execute->cmd_name);
                    my_free(socket_execute);
#endif
                    pthread_mutex_unlock(&mutex_socket);
                }
                else
                   Cdbg(APP_DBG, "######## socket item fail########\n");              
                enter_sleep_time(1000*100,NULL);
            }

            if(copy_file_list_head->next == NULL || exit_loop)
            {
#ifdef DEBUG
                //Cdbg(APP_DBG, "copy_file_list is blank\n");
#endif
                break;
            }
            enter_sleep_time(1000*100,NULL);
        }

        if(has_socket == 1)
        {
            write_finish_log();
            has_socket = 0;
        }


       if(!is_server_running)
           has_local_socket = 0;

        pthread_mutex_lock(&mutex_receve_socket);
        receve_socket = 0;
        pthread_mutex_unlock(&mutex_receve_socket);

        local_sync = 0;
        //write_system_log("Socket_Parser","end");
        enter_sleep_time(5,&wait_socket_mutex);       
    }


    Cdbg(APP_DBG, "stop asuswebstorage Socket_Parser\n");
    stop_down = 1;
    return NULL;
}

void sig_handler (int signum)
{
    switch (signum)
    {
    case SIGTERM:
        Cdbg(APP_DBG, "signal is SIGTERM\n");
        stop_progress = 1;
        exit_loop = 1;

        pthread_cond_signal(&(wait_server_mutex.cond));
        pthread_cond_signal(&(wait_socket_mutex.cond));
        pthread_cond_signal(&(my_mutex.cond));
#ifdef IPKG
        sync_disk_removed = 0;
#endif
        break;
    case SIGUSR1:  // add user
        Cdbg(APP_DBG, "signal is SIGUSER1\n");
        exit_loop = 1;
        read_config();
        break;
    case SIGUSR2:  // modify user
        Cdbg(APP_DBG, "signal is SIGUSR2\n");
#ifdef IPKG
        pthread_mutex_lock(&mutex_socket);
        disk_change = 1;
        pthread_mutex_unlock(&mutex_socket);
#endif
        break;
#if 1
    case SIGPIPE:  // delete user
        Cdbg(APP_DBG, "signal is SIGPIPE\n");
        signal(SIGPIPE, SIG_IGN);
        break;
#endif
    default:
        Cdbg(APP_DBG, "signal is %d\n",signum);
        signal(signum,SIG_IGN);
        break;

    }
}

unsigned long stat_file(char *filename)
{
    unsigned long size;
    struct stat filestat;
    if( stat(filename,&filestat) == -1)
    {
        perror("stat:");
        Cdbg(APP_DBG, "stat file stat error:%s file not exist\n",filename);
        return 0;
    }
    return  filestat.st_size;

    return size;

}

#if 1
int split_socket_cmd(char *cmd_in,Socket_cmd *scmd)
{
#ifdef DEBUG
    Cdbg(APP_DBG, "socket cmd is %s \n",cmd_in);
#endif

    char cmd[1024];
    char *p;
    const char *split = "\n";
    int i=0;

    memset(cmd,0,sizeof(cmd));
    strcpy(cmd,cmd_in);

    if(!strncmp(cmd,"rmroot",6))
    {
        no_local_root = 1;
        return 0;
    }

    if(strncmp(cmd,"exit",4) == 0)
    {
        Cdbg(APP_DBG, "exit socket\n");
        return -1;
    }

    if(strstr(cmd,".asus.td"))
    {
        Cdbg(APP_DBG, "ignore download temp file\n");
        return -1;
    }

    if(!strchr(cmd,'\n'))
    {
        Cdbg(APP_DBG, "socket cmd is not math cmd\\npath\\nname format\n");
        return -1;
    }

    p=strtok(cmd,split);

    while(p!=NULL)
    {
        switch (i)
        {
        case 0 :
            if(!strncmp(p,"rename",6) || !strncmp(p,"move",4))
                strncpy(scmd->cmd_name,p,strlen(p)-1);
            else
                strcpy(scmd->cmd_name,p);
            break;
        case 1:
            if(!strcmp(p,"/"))
                strcpy(scmd->path,sync_path);
            else
                sprintf(scmd->path,"%s%s",sync_path,p);
            break;
        case 2:
            if(!strcmp(scmd->cmd_name,"rename"))
            {
                strcpy(scmd->oldname,p);
            }
            else if(!strcmp(scmd->cmd_name,"move"))
            {
                if(!strcmp(p,"/"))
                    strcpy(scmd->oldpath,sync_path);
                else
                    sprintf(scmd->oldpath,"%s%s",sync_path,p);
            }
            else
                strcpy(scmd->filename,p);
            break;
        case 3:
            if(!strcmp(scmd->cmd_name,"rename"))
                strcpy(scmd->newname,p);
            else if(!strcmp(scmd->cmd_name,"move"))
                strcpy(scmd->oldname,p);
            break;
        default:
            break;
        }
        i++;
        p=strtok(NULL,split);
    }


    if(scmd->cmd_name[0] == '\0' || scmd->path[0] == '\0')
    {
        Cdbg(APP_DBG, "socket cmd is not math cmd\\npath\\nname format\n");
        return -1;
    }

    return 0;
}
#endif

int perform_socket_cmd(Socket_cmd *scmd)
{
    char cmd_name[64];
    char cmd_param[512];
    char path[512];
    char temp[512];
    char filename[NORMALSIZE] ={0};
    char fullname[NORMALSIZE];
    char oldname[NORMALSIZE],newname[NORMALSIZE],oldpath[NORMALSIZE];
    char action[64];
    char cmp_name[512];
    int parent_ID = -10;
    int entry_ID;
    Propfind *find = NULL;
    Createfolder *createfolder = NULL;
    Operateentry *oe = NULL;
    Moveentry *me = NULL;
    struct sync_item *item = NULL;
    int status = -10;
    int res_value = -10;
    int isdelete = 0 ;
    char type[64] = {0};

    memset(cmd_name, 0, sizeof(cmd_name));
    memset(cmd_param, 0, sizeof(cmd_param));
    memset(path,0,sizeof(path));
    memset(temp,0,sizeof(temp));
    memset(oldname,0,sizeof(oldname));
    memset(newname,0,sizeof(newname));
    memset(oldpath,0,sizeof(oldpath));
    memset(fullname,0,sizeof(fullname));
    memset(action,0,sizeof(action));
    memset(cmp_name,0,sizeof(cmp_name));

    strcpy(cmd_name,scmd->cmd_name);
    strcpy(path,scmd->path);

    if(!strcmp(cmd_name,"rename"))
    {
        strcpy(oldname,scmd->oldname);
        strcpy(newname,scmd->newname);
    }
    else if(!strcmp(cmd_name,"move"))
    {
        strcpy(oldpath,scmd->oldpath);
        strcpy(oldname,scmd->oldname);
    }
    else
    {
        strcpy(filename,scmd->filename);
    }

    //add by alan
    if( strcmp(cmd_name, "copyfile") == 0 )
    {
        //  2013/7/25 change copying way
        sprintf(fullname,"%s/%s",path,filename);
        add_sync_item("copyfile",fullname,copy_file_list_head);

        return 0;
    }

#if 1

    if( !strcmp(cmd_name,"rename") )
        snprintf(cmp_name,512,"%s/%s",path,newname);
    else
        snprintf(cmp_name,512,"%s/%s",path,filename);

    if( strcmp(cmd_name, "createfile") == 0 )
    {
        strcpy(action,"createfile");
        //file_iscopying = 0;
        //copying_file_number--;
        struct sync_item* item;

        item = get_sync_item("copyfile",cmp_name,copy_file_list_head);

        if(item != NULL)
        {
            del_sync_item("copyfile",cmp_name,copy_file_list_head);
        }
    }
    else if( strcmp(cmd_name, "remove") == 0  || strcmp(cmd_name, "delete") == 0)
    {
        strcpy(action,"remove");
        isdelete = 1;
    }
    else if( strcmp(cmd_name, "createfolder") == 0 )
    {
        strcpy(action,"createfolder");
    }
    else if( strcmp(cmd_name, "rename") == 0 )
    {
        strcpy(action,"rename");
    }

    struct timespec timeout;
    timeout.tv_sec = time(NULL) + 3;
    timeout.tv_nsec = 0;

    if(from_server_sync_head->next != NULL)
    {
        pthread_mutex_lock(&my_mutex.mutex);

        item = get_sync_item(action,cmp_name,from_server_sync_head);

        if(item != NULL)
        {
#ifdef DEBUG
            Cdbg(APP_DBG, "##### %s %s by asuswebstorage self ######\n",action,cmp_name);
#endif
            del_sync_item(action,cmp_name,from_server_sync_head);
            pthread_mutex_unlock(&my_mutex.mutex);

            return 0;
        }

        pthread_mutex_unlock(&my_mutex.mutex);
        item = NULL;
    }
    has_local_socket = 1;

    /* first make sure that the token is expired */
    if(is_check_token)
    {
        Getmysyncfolder *gf;
        gf = getMySyncFolder(username);
        //Cdbg(APP_DBG, "551\n");
        if(NULL == gf)
        {
            check_network_state();
            return -1;
        }
        if(gf->status == S_AUTH_FAIL)
        {
            status = getToken(username,password,"",0);

            if(status != 0)
            {
                if( status != S_NETWORK_FAIL)
                    exit_loop = 1;
                my_free(gf);
                return -1;
            }
        }
        my_free(gf);
    }

    parent_ID = getParentID(path);

    if(parent_ID == -1)
    {
        return -1;
    }
    if(parent_ID == -2) //parent dir has del from server
    {
        if(isdelete)
            return 0;

        parent_ID = create_server_folder_r(path);
        if(parent_ID == -1)
            return -1;

    }

#ifdef DEBUG
    Cdbg(APP_DBG, "###### %s is start ######\n",cmd_name);
    //write_system_log(cmd_name,"start");
#endif

    if( strcmp(cmd_name, "createfile") == 0 || strcmp(cmd_name, "dragfile") == 0 )
    {
        snprintf(fullname,NORMALSIZE,"%s/%s",path,filename);
#if TEST
        sleep(30);
        Cdbg(APP_DBG, "real upload start\n");
#endif
        status = uploadFile(fullname,parent_ID,NULL,0);

#if TREE_NODE_ENABLE
        modify_tree_node(fullname,DirRootNode,ADD_TREE_NODE);
#endif

        if(status != 0)
        {
#ifdef DEBUG
            Cdbg(APP_DBG, "upload %s failed\n",fullname);
            //write_system_log("error","uploadfile fail");
#endif
            res_value = handle_upload_fail_code(status,parent_ID,fullname,path);

            return res_value;
        }
    }
    else if( strcmp(cmd_name, "delete") == 0  || strcmp(cmd_name, "remove") == 0 || strcmp(cmd_name, "modify") == 0 ||
             strcmp(cmd_name, "rename") == 0 )
    {
        int isfolder;
        int ischildonly;

        if( strcmp(cmd_name, "rename") == 0 )
            find = checkEntryExisted(username,parent_ID,oldname,"system.unknown");
        else
            find = checkEntryExisted(username,parent_ID,filename,"system.unknown");

        if(NULL == find)
        {
            Cdbg(APP_DBG, "find prop failed\n");
            return -1;
        }

        if( find->status != 0 )
        {
            handle_error(find->status,"propfind");
            my_free(find);
            return -1;
        }

        entry_ID = find->id;
        strcpy(type,find->type);
        my_free(find);

        //Cdbg(APP_DBG, "entry id is %d\n",entry_ID);

        if(strcmp(type,"system.folder") == 0)
        {
            isfolder = 1;
            ischildonly = 0;
        }
        else if(strcmp(type,"system.file") == 0)
        {
            isfolder = 0;
            ischildonly = 0;
        }

        if( strcmp(cmd_name, "rename") == 0 )
        {
            if(strcmp(type,"system.notfound") == 0)
            {
                snprintf(fullname,NORMALSIZE,"%s/%s",path,newname);
                res_value = upload_entry(fullname,parent_ID,path);
                return res_value;
            }
            else
            {
#if TEST
                Cdbg(APP_DBG, "renameEntry() start\n");
                Cdbg(APP_DBG, "wait 40 start\n");
                sleep(30);
                Cdbg(APP_DBG, "rename start\n");
#endif
                oe = renameEntry(username,entry_ID,0,newname,isfolder) ;
                //Cdbg(APP_DBG, "renameEntry() end\n");

#if TREE_NODE_ENABLE
                rename_update_tree(oldname,newname);
#endif

                if(NULL == oe)
                {
                    Cdbg(APP_DBG, "operate rename failed\n");
                    return -1;
                }

                if( oe->status != 0 )
                {
                    handle_error(oe->status,cmd_name);
                    snprintf(fullname,NORMALSIZE,"%s/%s",path,newname);
                    res_value = handle_rename_fail_code(oe->status,parent_ID,fullname,path,isfolder);
                    my_free(oe);
                    return res_value;
                }

                my_free(oe);

            }

        }
        else if( strcmp(cmd_name, "modify") == 0 )
        {

            if(!strcmp(type,"system.notfound"))
            {
                Cdbg(APP_DBG, "del item has not exist server\n");
                entry_ID = 0 ;
            }

            snprintf(fullname,NORMALSIZE,"%s/%s",path,filename);
            status = uploadFile(fullname,parent_ID,NULL,entry_ID);

#if TREE_NODE_ENABLE
            modify_tree_node(fullname,DirRootNode,ADD_TREE_NODE);
#endif

            if(status != 0)
            {
                Cdbg(APP_DBG, "upload %s failed\n",fullname);
                res_value = handle_upload_fail_code(status,parent_ID,fullname,path);
                return res_value;
            }
        }
        else
        {
#if TEST
            Cdbg(APP_DBG, "wait 40 start\n");
            sleep(30);
            Cdbg(APP_DBG, "del start\n");
#endif
            if(!strcmp(type,"system.notfound"))
            {
                Cdbg(APP_DBG, "del item has not exist server\n");
                return 0;
            }

            oe = removeEntry(username,entry_ID,ischildonly,isfolder,parent_ID);

#if TREE_NODE_ENABLE
            snprintf(fullname,NORMALSIZE,"%s/%s",path,filename);
            modify_tree_node(fullname,DirRootNode,DEL_TREE_NODE);
#endif

            if(NULL == oe)
            {
                Cdbg(APP_DBG, "operate rename failed\n");
                return -1;
            }

            //Cdbg(APP_DBG, "remove status is %d \n",oe->status);

            if( oe->status != 0 )
            {
                handle_error(oe->status,cmd_name);
                res_value = handle_delete_fail_code(oe->status);
                my_free(oe);
                return res_value;
            }
            my_free(oe);
        }

    }
    else if(strcmp(cmd_name, "dragfolder") == 0 || strcmp(cmd_name, "createfolder") == 0 )
    {
        snprintf(fullname,NORMALSIZE,"%s/%s",path,filename);

#if TREE_NODE_ENABLE
        modify_tree_node(fullname,DirRootNode,ADD_TREE_NODE);
#endif

#if TEST
        Cdbg(APP_DBG, "wait 40 start\n");
        sleep(30);
        Cdbg(APP_DBG, "create folder start\n");
#endif
        createfolder = createFolder(username,parent_ID,0,fullname);


        if(NULL == createfolder)
            return -1;
        else if( createfolder->status != 0 )
        {
            handle_error(createfolder->status,"createfolder");
            res_value = handle_createfolder_fail_code(createfolder->status,parent_ID,path,fullname);
            my_free(createfolder);
            return res_value;
        }
        else
        {
            entry_ID = createfolder->id;
            my_free(createfolder);
            if(!strcmp(cmd_name,"dragfolder"))
            {
                res_value = sync_all_item(fullname,entry_ID);
                if(res_value == S_UPLOAD_DELETED)
                    return 0;
                else
                    return res_value;
            }

        }
    }
    else if( strcmp(cmd_name, "move") == 0 )
    {
#if TREE_NODE_ENABLE
        char del_name[NORMALSIZE];
        char insert_name[NORMALSIZE];

        memset(del_name,0,sizeof(del_name));
        memset(insert_name,0,sizeof(insert_name));

        snprintf(del_name,NORMALSIZE,"%s/%s",oldpath,oldname);
        snprintf(insert_name,NORMALSIZE,"%s/%s",path,oldname);

        modify_tree_node(del_name,DirRootNode,TREE_DEL_NODE);
        modify_tree_node(insert_name,DirRootNode,ADD_TREE_NODE);

#endif
        int pre_parent_ID;
        int isfolder;

        pre_parent_ID = getParentID(oldpath);
        if(pre_parent_ID == -1)
            return -1;
        //Cdbg(APP_DBG, "pre parent id is %d\n",pre_parent_ID);

        find = checkEntryExisted(username,pre_parent_ID,oldname,"system.unknown");

        if(NULL == find)
            return -1;

        if( find->status != 0 )
        {
            handle_error(find->status,"propfind");
            my_free(find);
            return -1;
        }
        else
        {
            if(!strcmp(find->type,"system.notfound"))
            {
                snprintf(fullname,NORMALSIZE,"%s/%s",path,oldname);
                res_value = upload_entry(fullname,parent_ID,path);
                my_free(find);
                return res_value;
            }
            else
            {
                entry_ID = find->id;
                //Cdbg(APP_DBG, "entry id is %d\n",entry_ID);
                if(strcmp(find->type,"system.folder") == 0)
                {
                    isfolder = 1;
                }
                else if(strcmp(find->type,"system.file") == 0)
                {
                    isfolder = 0;
                }

                my_free(find);

#if TEST
                Cdbg(APP_DBG, "wait 40 start\n");
                sleep(30);
                Cdbg(APP_DBG, "move start\n");
#endif
                me = moveEntry(username,entry_ID,oldname,parent_ID,isfolder,pre_parent_ID);

                if(NULL == me)
                {
                    Cdbg(APP_DBG, "operate move failed\n");
                    return -1;
                }
                else if( me->status != 0 )
                {
                    handle_error(me->status,cmd_name);
                    snprintf(fullname,NORMALSIZE,"%s/%s",path,oldname);
                    res_value = handle_move_fail_code(me->status,path,fullname,parent_ID,oldpath,entry_ID,isfolder);
                    my_free(me);
                    return res_value;
                }

                my_free(me);
            }
        }
    }


#ifdef DEBUG
    Cdbg(APP_DBG, "###### %s is ending ######\n",cmd_name);
#endif

#if SYSTEM_LOG
    write_system_log(cmd_name,filename);
#endif

    return 0;

#endif
}

int cmd_parser(char *command)
{
    Socket_cmd scmd;
    int res;

    memset(&scmd,0,sizeof(scmd));

    res = split_socket_cmd(command,&scmd);

    if(res == -1)
        return 0;

    if(download_only == 1 )
        res = download_only_add_socket_item(&scmd);
    else
    {
        res = perform_socket_cmd(&scmd);
        if(res != 0)
            is_check_token = 1;
        else
            is_check_token = 0;
    }

    return res;
}

int send_action(int type, char *content)
{
    int sockfd;
    char str[1024] = {0};
    int port;

    if(type == 1)
        port = 3678;
    else if(type == 2)
        port = MYPORT;

    struct sockaddr_in their_addr; /* connector's address information */


    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        return -1;
    }

    bzero(&(their_addr), sizeof(their_addr)); /* zero the rest of the struct */
    their_addr.sin_family = AF_INET; /* host byte order */
    their_addr.sin_port = htons(port); /* short, network byte order */
    their_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bzero(&(their_addr.sin_zero), sizeof(their_addr.sin_zero)); /* zero the rest of the struct */
    if (connect(sockfd, (struct sockaddr *)&their_addr,sizeof(struct
                                                              sockaddr)) == -1) {
        perror("connect");
        return -1;
    }

    sprintf(str,"0@%s",content);
    if (send(sockfd, str, strlen(str), 0) == -1) {
        perror("send");
        return -1;
    }
    else
    {
        Cdbg(APP_DBG, "send content is %s\n",str);
    }

    close(sockfd);
    return 0;
}



void clean_global_var()
{
    memset(mount_path,0,sizeof(mount_path));
    memset(base_path, 0, sizeof(base_path));
    memset(cloud_path, 0, sizeof(cloud_path));
    //memset(temp_path, 0, sizeof(temp_path));
    memset(asus_path, 0, sizeof(asus_path));
    memset(log_path, 0, sizeof(log_path));
    memset(xml_path, 0, sizeof(xml_path));
    memset(system_log, 0, sizeof(system_log));
    memset(general_log,0,sizeof(general_log));
    memset(confilicted_log,0,sizeof(confilicted_log));
//    memset(temp_file, 0, sizeof(temp_file));

    memset(gateway_xml, 0, sizeof(gateway_xml));
    memset(token_xml, 0, sizeof(token_xml));
    memset(get_info_xml, 0, sizeof(get_info_xml));
    memset(get_sync_folder_xml, 0, sizeof(get_sync_folder_xml));
        
    memset(get_init_attachable_device_xml, 0, sizeof(get_init_attachable_device_xml)); // add by markcool
    memset(get_browse_attachable_device_xml, 0, sizeof(get_browse_attachable_device_xml)); // add by markcool    
    memset(get_send_aws_email_xml, 0, sizeof(get_send_aws_email_xml)); // add by markcool
        
    memset(get_personal_system_folder_xml,0,sizeof(get_personal_system_folder_xml));
    memset(browse_folder_xml, 0, sizeof(browse_folder_xml));
    memset(sliding_browse_file_xml, 0, sizeof(sliding_browse_file_xml));
    
    memset(propfind_xml, 0, sizeof(propfind_xml));
    memset(create_folder_xml, 0, sizeof(create_folder_xml));
    memset(rename_xml, 0, sizeof(rename_xml));
    memset(move_xml, 0, sizeof(move_xml));
    memset(remove_xml, 0, sizeof(remove_xml));
    memset(update_xml, 0, sizeof(update_xml));
    memset(get_entry_info_xml, 0, sizeof(get_entry_info_xml));
//    memset(set_mark_xml, 0, sizeof(set_mark_xml));
//    memset(get_change_files_xml, 0, sizeof(get_change_files_xml));
//    memset(get_uploads_xml, 0, sizeof(get_uploads_xml));
//    memset(get_share_code_xml, 0, sizeof(get_share_code_xml));
//    memset(del_share_code_xml, 0, sizeof(del_share_code_xml));
//    memset(get_share_entry_xml, 0, sizeof(get_share_entry_xml));
//    memset(check_pwd_xml, 0, sizeof(check_pwd_xml));
//    memset(cmp_pwd_xml, 0, sizeof(cmp_pwd_xml));
    memset(get_change_seq_xml, 0, sizeof(get_change_seq_xml));
    
    memset(access_token_json, 0, sizeof(access_token_json));
    memset(search_drive_file_json, 0, sizeof(search_drive_file_json));
    memset(create_folder_json, 0, sizeof(create_folder_json));
    
    memset(init_upload_xml, 0, sizeof(init_upload_xml));
    memset(resume_upload_xml, 0, sizeof(resume_upload_xml));
    memset(finish_upload_xml, 0, sizeof(finish_upload_xml));
//    memset(get_resize_photo_xml, 0, sizeof(get_resize_photo_xml));
//    memset(get_full_txt_xml, 0, sizeof(get_full_txt_xml));
//    memset(get_video_snapshot_xml, 0, sizeof(get_video_snapshot_xml));
    memset(trans_excep_file,0,sizeof(trans_excep_file));
    memset(system_token,0,sizeof(system_token));
}


void init_global_var()
{
    char config_path[256] = {0};
    char script_path[256] = {0};
    char temp_path[256] = {0};


    snprintf(cloud_path,NAMESIZE,"%s/%s",base_path,"smartsync");

    Cdbg(APP_DBG, "cloud_path is %s",cloud_path);

    snprintf(log_path,NAMESIZE,"%s/%s",cloud_path,".logs");

    //snprintf(temp_path,NAMESIZE,"%s/%s",cloud_path,"temp");
    snprintf(asus_path,NAMESIZE,"%s/%s",cloud_path,"asuswebstorage");
    snprintf(config_path,256,"%s/config",asus_path);
    snprintf(script_path,256,"%s/script",asus_path);
    snprintf(temp_path,256,"%s/temp",asus_path);
    snprintf(xml_path,NAMESIZE,"%s/xml",temp_path);

    Cdbg(APP_DBG, "log_path : %s",log_path);
    Cdbg(APP_DBG, "asus_path : %s",asus_path);
    Cdbg(APP_DBG, "config_path : %s",config_path);
    Cdbg(APP_DBG, "script_path : %s",script_path);
    Cdbg(APP_DBG, "temp_path : %s",temp_path);
    Cdbg(APP_DBG, "xml_path : %s",xml_path);


    // printf("log_path is %s\n",log_path);

    
    // printf("log_path is %s\n",log_path);
    // printf("asus_path is %s\n",asus_path);
    // printf("config_path is %s\n",config_path);
    // printf("script_path is %s\n",script_path);
    // printf("temp_path is %s\n",temp_path);
    // printf("xml_path is %s\n",xml_path);

    snprintf(general_log,NAMESIZE,"%s/asuswebstorage",log_path);
    snprintf(system_log,NAMESIZE,"%s/system.log",log_path);
//    snprintf(temp_file,NAMESIZE,"%s/temp",log_path);
    snprintf(gateway_xml,NAMESIZE,"%s/%s",xml_path,"gateway.xml");
    snprintf(get_user_state_xml,NAMESIZE,"%s/%s",xml_path,"get_user_state.xml");
    snprintf(token_xml,NAMESIZE,"%s/%s",xml_path,"token.xml");
    snprintf(get_info_xml,NAMESIZE,"%s/%s",xml_path,"get_info.xml");
    snprintf(get_sync_folder_xml,NAMESIZE,"%s/%s",xml_path,"get_sync_folder.xml");
        
    snprintf(get_init_attachable_device_xml,NAMESIZE,"%s/%s",xml_path,"get_init_attachable_device_xml.xml"); // add by markcool
    snprintf(get_browse_attachable_device_xml,NAMESIZE,"%s/%s",xml_path,"get_browse_attachable_device_xml.xml"); // add by markcool
    snprintf(get_send_aws_email_xml,NAMESIZE,"%s/%s",xml_path,"get_send_aws_email_xml.xml"); // add by markcool
        
    snprintf(get_personal_system_folder_xml,NAMESIZE,"%s/%s",xml_path,"get_personal_system_folder.xml");
    snprintf(browse_folder_xml,NAMESIZE,"%s/%s",xml_path,"browse_folder.xml");
    snprintf(sliding_browse_file_xml,NAMESIZE,"%s/%s",xml_path,"sliding_browse_file.xml");
    
    snprintf(propfind_xml,NAMESIZE,"%s/%s",xml_path,"propfind.xml");
    snprintf(create_folder_xml,NAMESIZE,"%s/%s",xml_path,"create_folder.xml");
    snprintf(rename_xml,NAMESIZE,"%s/%s",xml_path,"rename.xml");
    snprintf(move_xml,NAMESIZE,"%s/%s",xml_path,"move.xml");
    snprintf(remove_xml,NAMESIZE,"%s/%s",xml_path,"remove.xml");
    snprintf(update_xml,NAMESIZE,"%s/%s",xml_path,"update.xml");
    snprintf(get_entry_info_xml,NAMESIZE,"%s/%s",xml_path,"get_entry_info.xml");
//    snprintf(set_mark_xml,NAMESIZE,"%s/%s",xml_path,"set_mark.xml");
//    snprintf(get_change_files_xml,NAMESIZE,"%s/%s",xml_path,"get_change_files.xml");
//    snprintf(get_uploads_xml,NAMESIZE,"%s/%s",xml_path,"get_uploads.xml");
//    snprintf(get_share_code_xml,NAMESIZE,"%s/%s",xml_path,"get_share_code.xml");
//    snprintf(del_share_code_xml,NAMESIZE,"%s/%s",xml_path,"del_share_code.xml");
//    snprintf(get_share_entry_xml,NAMESIZE,"%s/%s",xml_path,"get_share_entry.xml");
//    snprintf(check_pwd_xml,NAMESIZE,"%s/%s",xml_path,"check_pwd.xml");
//    snprintf(cmp_pwd_xml,NAMESIZE,"%s/%s",xml_path,"cmp_pwd.xml");
    snprintf(get_change_seq_xml,NAMESIZE,"%s/%s",xml_path,"get_change_seq.xml");
    snprintf(access_token_json,NAMESIZE,"%s/%s",xml_path,"access_token.json");
    snprintf(search_drive_file_json,NAMESIZE,"%s/%s",xml_path,"search_drive_file.json");
    snprintf(create_folder_json,NAMESIZE,"%s/%s",xml_path,"create_folder.json");
    
    snprintf(init_upload_xml,NAMESIZE,"%s/%s",xml_path,"init_upload.xml");
    
    snprintf(resume_upload_xml,NAMESIZE,"%s/%s",xml_path,"resume_upload.xml");
    snprintf(finish_upload_xml,NAMESIZE,"%s/%s",xml_path,"finish_upload.xml");
//    snprintf(get_resize_photo_xml,NAMESIZE,"%s/%s",xml_path,"get_resize_photo.xml");
//    snprintf(get_full_txt_xml,NAMESIZE,"%s/%s",xml_path,"get_full_txt.xml");
//    snprintf(get_video_snapshot_xml,NAMESIZE,"%s/%s",xml_path,"get_video_snapshot.xml");
    snprintf(trans_excep_file,128,"%s/%s",log_path,"asuswebstorage_errlog");

    my_mkdir(cloud_path);
    //my_mkdir(temp_path);
    my_mkdir(log_path);
    my_mkdir(asus_path);
    my_mkdir(config_path);
    my_mkdir(script_path);
    my_mkdir(temp_path);
    my_mkdir(xml_path);
    //my_mkdir_r(xml_path);
    //my_mkdir("/tmp/Cloud");

    //remove(general_log);
    remove(trans_excep_file);
}

void clean_up()
{
    Cdbg(APP_DBG, "enter clean up\n");

    remove("/tmp/smartsync/.logs/asuswebstorage");

    check_accout_status();
#if 0
    if( !item_empty(up_head))
    {
        Cdbg(APP_DBG, "write uncomplete upload item to file\n");
        print_sync_item(up_head,UPLOAD);
    }

    if( !item_empty(down_head))
    {
#if WRITE_DOWN_TEMP_FILE
        remove(down_item_temp_file);
#endif
        Cdbg(APP_DBG, "write uncomplete download item to file\n");
        print_sync_item(down_head,DOWNLOAD);
    }

    if( !item_empty(up_excep_fail))
    {
        Cdbg(APP_DBG, "write up excep fail item to file\n");
        print_sync_item(up_excep_fail,UP_EXCEP_FAIL);
    }
#endif

    free_sync_item(up_head);
    free_sync_item(down_head);
    free_sync_item(from_server_sync_head);
    free_sync_item(up_excep_fail);
    queue_destroy(SocketActionList);
    free_sync_item(copy_file_list_head);
    if(download_only == 1)
        free_sync_item(download_only_socket_head);

#if TREE_NODE_ENABLE
    write_tree_to_file(tree_log,DirRootNode);
    free_tree_node(DirRootNode);
#endif

#if MEM_POOL_ENABLE
    mem_pool_destroy();
#endif

    Cdbg(APP_DBG, "clean up end !!!\n");
}

void str_to_lower(char *word)
{
    int i;
    int len=strlen(word);
    for(i=0;i<len;i++)
    {
        if(word[i] >='A' && word[i] <= 'Z')
        {
            word[i] += 32;

        }
    }
}

void apiErrorAddTsDb(long long api_response_status, char * api_name) {

    char api_response_code[32];
    memset(api_response_code,0, sizeof(api_response_code));

    sprintf(api_response_code, "%lli", api_response_status);

    time_t t = time(0);
    char time_new[26];
    memset(time_new,0, sizeof(time_new));

    //strftime( tmp, sizeof(tmp), "%Y%m%d %X %A :%A:%s本年第%j天 %z",localtime(&t) );
    strftime(time_new, sizeof(time_new), "%Y-%m-%d %X",localtime(&t) );

    Cdbg(APP_DBG, "apiErrorAddTsDb : TsDB time_new : %s\n", time_new);

    int return_value = 0;
    return_value = apiTsdbInputNoId(&asus_conf, time_new, "api", asus_conf.device_id, "AiCAM_2.0.1", api_name, "0" , api_response_code);

    if(return_value == 0) {
        Cdbg(APP_DBG, "apiErrorAddTsDb : tsdb status upload success\n");
    } else {
        Cdbg(APP_DBG, "apiErrorAddTsDb : tsdb status apiTsdbInputNoId fail: %d\n", return_value);
    }

}

long long getMacParentID(char * mac_name, int mac_type);

long long getMacParentID(char * mac_name, int mac_type) {

    if(mac_type == 1) {

        if(strstr(macVideoModeOne, SPACE_2GB_7DAYS)) {

            Cdbg(APP_DBG, "parentID aiCamV2ID = %lli, check -22/AiCAM/%s/%s DIR", aiCamV2ID, AICAM_ALARM_PATH, mac_name);

            return aiCamV2ID;

        } else {

            Cdbg(APP_DBG, "parentID aiCamID = %lli, check -22/AiCAM/%s DIR", aiCamID, mac_name);

            return aiCamID;
        }

    } else if(mac_type == 2) {

        if(strstr(macVideoModeTwo, SPACE_2GB_7DAYS)) {

            Cdbg(APP_DBG, "parentID aiCamV2ID = %lli, check -22/AiCAM/%s/%s DIR", aiCamV2ID, AICAM_ALARM_PATH, mac_name);

            return aiCamV2ID;

        } else {

            Cdbg(APP_DBG, "parentIDparentID aiCamID = %lli, check -22/AiCAM/%s DIR \n", aiCamID, mac_name);


            return aiCamID;
        }

    } else if(mac_type == 3) {

        if(strstr(macVideoModeThree, SPACE_2GB_7DAYS)) {


            // Cdbg(APP_DBG, "parentIDparentID aiCamID = %lli, check -22/AiCAM/%s DIR \n", aiCamID, mac_name);
            // snprintf(dbg_msg, sizeof(dbg_msg), "parentID aiCamV2ID = %lli, check -22/AiCAM/%s/%s DIR \n", aiCamV2ID, AICAM_ALARM_PATH, mac_name);
            // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
            Cdbg(APP_DBG, "parentID aiCamV2ID = %lli, check -22/AiCAM/%s/%s DIR \n", aiCamV2ID, AICAM_ALARM_PATH, mac_name);

            return aiCamV2ID;

        } else {

            // snprintf(dbg_msg, sizeof(dbg_msg), "parentID aiCamID = %lli, check -22/AiCAM/%s DIR \n", aiCamID, mac_name);
            // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);

            Cdbg(APP_DBG, "parentIDparentID aiCamID = %lli, check -22/AiCAM/%s DIR \n", aiCamID, mac_name);

            return aiCamID;
        }

    } else if(mac_type == 4) {

        if(strstr(macVideoModeFour, SPACE_2GB_7DAYS)) {

            // snprintf(dbg_msg, sizeof(dbg_msg), "parentID aiCamV2ID = %lli, check -22/AiCAM/%s/%s DIR \n", aiCamV2ID, AICAM_ALARM_PATH, mac_name);
            // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);

            Cdbg(APP_DBG, "parentIDparentID aiCamID = %lli, check -22/AiCAM/%s DIR \n", aiCamID, mac_name);

            return aiCamV2ID;

        } else {

            // snprintf(dbg_msg, sizeof(dbg_msg), "parentID aiCamID = %lli, check -22/AiCAM/%s DIR \n", aiCamID, mac_name);
            // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
            Cdbg(APP_DBG, "parentIDparentID aiCamID = %lli, check -22/AiCAM/%s DIR \n", aiCamID, mac_name);

            return aiCamID;
        }

    }



}


long long createWebStorageMacDir(char * mac_name, int mac_type) {


    long long tempMacDirID;
    long long tempParentID;


    //  : process AiCAM(Dir) -> V2 ID(Dir) -> Mac(Dir) (ex: -22/AiCAM/V2/mac001~mac004/)
    char dirname[64];

    memset(dirname,0,sizeof(dirname));
    strcat(dirname, "/tmp/");
    strcpy(dirname, mac_name);

    // get MacParentID - > diffirent  vidoe type 
    tempParentID = getMacParentID(mac_name, mac_type);

    while(1) {

        if(mac_type == 1) {

            aiCamMacAddressOneID = checkDirId(tempParentID, dirname);

            tempMacDirID = aiCamMacAddressOneID;

            if( tempMacDirID >= 1000 ) {

                memset(aiCamMacAddressOne,0,sizeof(aiCamMacAddressOne));
                strcpy(aiCamMacAddressOne, mac_name);

            }


            snprintf(dbg_msg, sizeof(dbg_msg), "Get aiCamMacAddressOne : %s, Get aiCamMacAddressOneID : %lli", aiCamMacAddressOne, aiCamMacAddressOneID);

        } else if(mac_type == 2) {

            aiCamMacAddressTwoID = checkDirId(tempParentID, dirname);
            
            tempMacDirID = aiCamMacAddressTwoID;

            if( tempMacDirID >= 1000 ) {
                memset(aiCamMacAddressTwo,0,sizeof(aiCamMacAddressTwo));
                strcpy(aiCamMacAddressTwo, mac_name);
            }

            snprintf(dbg_msg, sizeof(dbg_msg), "Get aiCamMacAddressTwo : %s, Get aiCamMacAddressTwoID : %lli", aiCamMacAddressTwo, aiCamMacAddressTwoID);

        } else if(mac_type == 3) {


            aiCamMacAddressThreeID = checkDirId(tempParentID, dirname);            

            tempMacDirID = aiCamMacAddressThreeID;

            if( tempMacDirID >= 1000 ) {
                memset(aiCamMacAddressThree,0,sizeof(aiCamMacAddressThree));
                strcpy(aiCamMacAddressThree, mac_name);
            }

            snprintf(dbg_msg, sizeof(dbg_msg), "Get aiCamMacAddressThree : %s, Get aiCamMacAddressThreeID : %lli", aiCamMacAddressThree, aiCamMacAddressThreeID);

        } else if(mac_type == 4) {    

            aiCamMacAddressFourID = checkDirId(tempParentID, dirname);
            
            tempMacDirID = aiCamMacAddressFourID;

            if( tempMacDirID >= 1000 ) {
                memset(aiCamMacAddressFour,0,sizeof(aiCamMacAddressFour));
                strcpy(aiCamMacAddressFour, mac_name);
            }

            snprintf(dbg_msg, sizeof(dbg_msg), "Get aiCamMacAddressFour : %s, Get aiCamMacAddressFourID : %lli", aiCamMacAddressFour, aiCamMacAddressFourID);
        }

        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(APP_DBG, "%s", dbg_msg);
        



        // ID >= 1000 : get id sucess
        if((tempMacDirID == DIR_NEED_CREATE) || (tempMacDirID >= 1000)) {
            break;
        } else {

            int tsdb_status = tsdbProcess(tempMacDirID, "propfind:macDir fail");

            snprintf(dbg_msg, sizeof(dbg_msg), "aiCamDeviceMacID get data fail, waiting 1s, continue");
            // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
            Cdbg(APP_DBG, "%s", dbg_msg);

            sleep(1);
            continue;
        }

    }


    // WebStorage not -22/AiCAM/V2/AiCAM_215C34/mac_xxx DIR
    if(tempMacDirID == DIR_NEED_CREATE) {

        memset(dirname,0,sizeof(dirname));

        strcpy(dirname, "-22");
        strcat(dirname, "/AiCAM/");
        strcat(dirname, mac_name);
        
        snprintf(dbg_msg, sizeof(dbg_msg), "not %s DIR, parentID aiCamV2ID is %lli , Prepare Create AiCAM Device/%s dir", mac_name, aiCamV2ID, mac_name);
        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(APP_DBG, "%s", dbg_msg);


        while(1) {

            if(mac_type == 1) {

                aiCamMacAddressOneID = createDirId(tempParentID, dirname);
                tempMacDirID = aiCamMacAddressOneID;

                if( tempMacDirID >= 1000 ) {
                    memset(aiCamMacAddressOne,0,sizeof(aiCamMacAddressOne));
                    strcpy(aiCamMacAddressOne, mac_name);
                }

                snprintf(dbg_msg, sizeof(dbg_msg), "Create aiCamMacAddressOne : %s, Get aiCamMacAddressOneID : %lli", aiCamMacAddressOne, aiCamMacAddressOneID);


            } else if(mac_type == 2) {

                aiCamMacAddressTwoID = createDirId(tempParentID, dirname);
                tempMacDirID = aiCamMacAddressTwoID;

                if( tempMacDirID >= 1000 ) {
                    memset(aiCamMacAddressTwo,0,sizeof(aiCamMacAddressTwo));
                    strcpy(aiCamMacAddressTwo, mac_name);
                }

                snprintf(dbg_msg, sizeof(dbg_msg), "Create aiCamMacAddressTwo : %s, Get aiCamMacAddressTwoID : %lli", aiCamMacAddressTwo, aiCamMacAddressTwoID);

            } else if(mac_type == 3) {
                
                aiCamMacAddressThreeID = createDirId(tempParentID, dirname);
                tempMacDirID = aiCamMacAddressThreeID;

                if( tempMacDirID >= 1000 ) {
                    memset(aiCamMacAddressThree,0,sizeof(aiCamMacAddressThree));
                    strcpy(aiCamMacAddressThree, mac_name);
                }

                snprintf(dbg_msg, sizeof(dbg_msg), "Create aiCamMacAddressThree : %s, Get aiCamMacAddressThreeID : %lli", aiCamMacAddressThree, aiCamMacAddressThreeID);

            } else if(mac_type == 4) {    

                aiCamMacAddressFourID = createDirId(tempParentID, dirname);
                tempMacDirID = aiCamMacAddressFourID;


                if( tempMacDirID >= 1000 ) {
                    memset(aiCamMacAddressFour,0,sizeof(aiCamMacAddressFour));
                    strcpy(aiCamMacAddressFour, mac_name);
                }

                snprintf(dbg_msg, sizeof(dbg_msg), "Create aiCamMacAddressFour : %s, Get aiCamMacAddressFourID : %lli", aiCamMacAddressFour, aiCamMacAddressFourID);

            }

            // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
            Cdbg(APP_DBG, "%s", dbg_msg);


            // ID >= 1000 : get id sucess
            if(tempMacDirID >= 1000) {
            //if((aiCamDeviceAlarmID == DIR_NEED_CREATE) || (aiCamDeviceAlarmID >= 1000)) {
                break;
            } else {

                int tsdb_status = tsdbProcess(tempMacDirID, "mac folder create fail");

                //apiErrorAddTsDb(aiCamDeviceAlarmID, "folder_create");

                snprintf(dbg_msg, sizeof(dbg_msg), "Creat aiCamDeviceMacID Floder fail, waiting 1s, continue");
                // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
                Cdbg(APP_DBG, "%s", dbg_msg);

                sleep(1);
                continue;
            }

        }

    }

    return tempMacDirID;

    // snprintf(dbg_msg, sizeof(dbg_msg), "Finish createWebStorageMacDir [-22/AiCAM/V2/%s/%s] , Dir ID is %lli\n", aicam_device_dir, mac_name, tempMacDirID);
    // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);

}

// if mac dir exist , check alarm id 
long long createWebStorageMacAlarmDir(char * mac_name, int mac_type) {

    long long tempMacAlarmDirID;

    //  : process AiCAM(Dir) -> V2(Dir) -> Device ID(Dir) -> Uuid(Dir) -> alarm(Dir) (ex: -22/AiCAM/V2/AiCAM_215C34/mac001~mac004/alarm/)
    char dirname[64];

    memset(dirname,0,sizeof(dirname));
    strcpy(dirname, "alarm");

    snprintf(dbg_msg, sizeof(dbg_msg), "check ../%s/alarm DIR", mac_name);
    // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
    Cdbg(APP_DBG, "%s", dbg_msg);

    while(1) {

        if(mac_type == 1) {

   
            aiCamMacAddressOneAlarmID = checkDirId(aiCamMacAddressOneID, dirname);
            tempMacAlarmDirID = aiCamMacAddressOneAlarmID;

            snprintf(dbg_msg, sizeof(dbg_msg), "Get aiCamMacAddressOneAlarmID : %lli", aiCamMacAddressOneAlarmID);



        } else if(mac_type == 2) {


            aiCamMacAddressTwoAlarmID = checkDirId(aiCamMacAddressTwoID, dirname);
            tempMacAlarmDirID = aiCamMacAddressTwoAlarmID;

            snprintf(dbg_msg, sizeof(dbg_msg), "Get aiCamMacAddressTwoAlarmID : %lli", aiCamMacAddressTwoAlarmID);


        } else if(mac_type == 3) {

            aiCamMacAddressThreeAlarmID = checkDirId(aiCamMacAddressThreeID, dirname);
            tempMacAlarmDirID = aiCamMacAddressThreeAlarmID;

            snprintf(dbg_msg, sizeof(dbg_msg), "Get aiCamMacAddressThreeAlarmID : %lli", aiCamMacAddressThreeAlarmID);


        } else if(mac_type == 4) {

            aiCamMacAddressFourAlarmID = checkDirId(aiCamMacAddressFourID, dirname);
            tempMacAlarmDirID = aiCamMacAddressFourAlarmID;

            snprintf(dbg_msg, sizeof(dbg_msg), "Get aiCamMacAddressFourAlarmID : %lli", aiCamMacAddressFourAlarmID);

        }

        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(APP_DBG, "%s", dbg_msg);



        // ID >= 1000 : get id sucess
        if((tempMacAlarmDirID == DIR_NEED_CREATE) || (tempMacAlarmDirID >= 1000)) {
            break;
        } else {

            int tsdb_status = tsdbProcess(tempMacAlarmDirID, "find macAlarmDir fail");

            snprintf(dbg_msg, sizeof(dbg_msg), "DeviceUuidAlarmID get data fail, waiting 1s, continue");
            // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
            Cdbg(APP_DBG, "%s", dbg_msg);

            sleep(1);
            continue;
        }

    }

    // not -22/AiCAM/V2/AiCAM_215C34/macxxxxxxxx/alarm DIR
    if(tempMacAlarmDirID == DIR_NEED_CREATE) {

        memset(dirname,0,sizeof(dirname));

        strcpy(dirname, "/../");
        strcat(dirname, mac_name);
        strcat(dirname, "/alarm");
        
        snprintf(dbg_msg, sizeof(dbg_msg), "not ../%s/alarm DIR, Prepare Create AiCAM Device/Mac/Alarm dir : %s\n", mac_name, dirname);
        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(APP_DBG, "%s", dbg_msg);


        while(1) {

            if(mac_type == 1) {

                aiCamMacAddressOneAlarmID = createDirId(aiCamMacAddressOneID, dirname);
                tempMacAlarmDirID = aiCamMacAddressOneAlarmID;

                snprintf(dbg_msg, sizeof(dbg_msg), "Create aiCamMacAddressOneAlarmID : %lli", aiCamMacAddressOneAlarmID);
            
            } else if(mac_type == 2) {

                aiCamMacAddressTwoAlarmID = createDirId(aiCamMacAddressTwoID, dirname);
                tempMacAlarmDirID = aiCamMacAddressTwoAlarmID;

                snprintf(dbg_msg, sizeof(dbg_msg), "Create aiCamMacAddressTwoAlarmID : %lli", aiCamMacAddressTwoAlarmID);


            } else if(mac_type == 3) {
                
                aiCamMacAddressThreeAlarmID = createDirId(aiCamMacAddressThreeID, dirname);
                tempMacAlarmDirID = aiCamMacAddressThreeAlarmID;

                snprintf(dbg_msg, sizeof(dbg_msg), "Create aiCamMacAddressThreeAlarmID : %lli", aiCamMacAddressThreeAlarmID);


            } else if(mac_type == 4) {    

                aiCamMacAddressFourAlarmID = createDirId(aiCamMacAddressFourID, dirname);
                tempMacAlarmDirID = aiCamMacAddressFourAlarmID;

                snprintf(dbg_msg, sizeof(dbg_msg), "Create aiCamMacAddressFourAlarmID : %lli", aiCamMacAddressFourAlarmID);


            }

            // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
            Cdbg(APP_DBG, "%s", dbg_msg);


            // ID >= 1000 : get id sucess
            if(tempMacAlarmDirID >= 1000) {
            //if((aiCamDeviceAlarmID == DIR_NEED_CREATE) || (aiCamDeviceAlarmID >= 1000)) {
                break;
            } else {

                int tsdb_status = tsdbProcess(tempMacAlarmDirID, "mac alarm dir create fail");

                //apiErrorAddTsDb(aiCamDeviceAlarmID, "folder_create");

                snprintf(dbg_msg, sizeof(dbg_msg), "Creat aiCamDeviceUuidAlarmID Floder fail, waiting 1s, continue");
                // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
                Cdbg(APP_DBG, "%s", dbg_msg);

                sleep(1);
                continue;
            }

        }

    }

    return tempMacAlarmDirID;
    // snprintf(dbg_msg, sizeof(dbg_msg), "Finish createWebStorageMacAlarmDir [-22/AiCAM/V2/%s/%s/alarm] , Dir ID is %lli\n", aicam_device_dir, mac_name, tempMacAlarmDirID);
    // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);

}


// if mac dir exist , check Record id 
long long createWebStorageMacRecordDir(char * mac_name, int mac_type) {


    long long tempMacRecordDirID;

    //  : process AiCAM(Dir) -> V2(Dir) -> Device ID(Dir) -> Uuid(Dir) -> Record(Dir) (ex: -22/AiCAM/AiCAM_215C34/mac001~mac004/Record/)
    char dirname[64];

    memset(dirname,0,sizeof(dirname));
    strcpy(dirname, "record");

    snprintf(dbg_msg, sizeof(dbg_msg), "check -22/AiCAM/%s/record DIR \n", mac_name);
    // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
    Cdbg(APP_DBG, "%s", dbg_msg);

    while(1) {

        if(mac_type == 1) {

   
            aiCamMacAddressOneRecordID = checkDirId(aiCamMacAddressOneID, dirname);
            tempMacRecordDirID = aiCamMacAddressOneRecordID;

            snprintf(dbg_msg, sizeof(dbg_msg), "Get aiCamMacAddressOneRecordID : %lli", aiCamMacAddressOneRecordID);



        } else if(mac_type == 2) {


            aiCamMacAddressTwoRecordID = checkDirId(aiCamMacAddressTwoID, dirname);
            tempMacRecordDirID = aiCamMacAddressTwoRecordID;

            snprintf(dbg_msg, sizeof(dbg_msg), "Get aiCamMacAddressTwoRecordID : %lli", aiCamMacAddressTwoRecordID);


        } else if(mac_type == 3) {

            aiCamMacAddressThreeRecordID = checkDirId(aiCamMacAddressThreeID, dirname);
            tempMacRecordDirID = aiCamMacAddressThreeRecordID;

            snprintf(dbg_msg, sizeof(dbg_msg), "Get aiCamMacAddressThreeRecordID : %lli", aiCamMacAddressThreeRecordID);


        } else if(mac_type == 4) {

            aiCamMacAddressFourRecordID = checkDirId(aiCamMacAddressFourID, dirname);
            tempMacRecordDirID = aiCamMacAddressFourRecordID;

            snprintf(dbg_msg, sizeof(dbg_msg), "Get aiCamMacAddressFourRecordID : %lli", aiCamMacAddressFourRecordID);

        }

        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(APP_DBG, "%s", dbg_msg);


        // ID >= 1000 : get id sucess
        if((tempMacRecordDirID == DIR_NEED_CREATE) || (tempMacRecordDirID >= 1000)) {
            break;
        } else {

            int tsdb_status = tsdbProcess(tempMacRecordDirID, "find macRecordDir fail");

            snprintf(dbg_msg, sizeof(dbg_msg), "Get MacRecordID fail, waiting 1s, continue");
            // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
            Cdbg(APP_DBG, "%s", dbg_msg);

            sleep(1);
            continue;
        }

    }

    // not -22/AiCAM/AiCAM_215C34/macxxxxxxxx/Record DIR
    if(tempMacRecordDirID == DIR_NEED_CREATE) {

        memset(dirname,0,sizeof(dirname));

        strcpy(dirname, "-22");
        strcat(dirname, "/AiCAM/");
        strcat(dirname, mac_name);
        strcat(dirname, "/record");
        
        snprintf(dbg_msg, sizeof(dbg_msg), "not -22/AiCAM/%s/record DIR, Prepare Create AiCAM Device/Mac/Record dir : %s\n", mac_name, dirname);
        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(APP_DBG, "%s", dbg_msg);


        while(1) {

            if(mac_type == 1) {

                aiCamMacAddressOneRecordID = createDirId(aiCamMacAddressOneID, dirname);
                tempMacRecordDirID = aiCamMacAddressOneRecordID;

                snprintf(dbg_msg, sizeof(dbg_msg), "Create aiCamMacAddressOneRecordID : %lli", aiCamMacAddressOneRecordID);
            
            } else if(mac_type == 2) {

                aiCamMacAddressTwoRecordID = createDirId(aiCamMacAddressTwoID, dirname);
                tempMacRecordDirID = aiCamMacAddressTwoRecordID;

                snprintf(dbg_msg, sizeof(dbg_msg), "Create aiCamMacAddressTwoRecordID : %lli", aiCamMacAddressTwoRecordID);


            } else if(mac_type == 3) {
                
                aiCamMacAddressThreeRecordID = createDirId(aiCamMacAddressThreeID, dirname);
                tempMacRecordDirID = aiCamMacAddressThreeRecordID;

                snprintf(dbg_msg, sizeof(dbg_msg), "Create aiCamMacAddressThreeRecordID : %lli", aiCamMacAddressThreeRecordID);


            } else if(mac_type == 4) {    

                aiCamMacAddressFourRecordID = createDirId(aiCamMacAddressFourID, dirname);
                tempMacRecordDirID = aiCamMacAddressFourRecordID;

                snprintf(dbg_msg, sizeof(dbg_msg), "Create aiCamMacAddressFourRecordID : %lli", aiCamMacAddressFourRecordID);


            }

            // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
            Cdbg(APP_DBG, "%s", dbg_msg);


            // ID >= 1000 : get id sucess
            if(tempMacRecordDirID >= 1000) {
            //if((aiCamDeviceRecordID == DIR_NEED_CREATE) || (aiCamDeviceRecordID >= 1000)) {
                break;
            } else {

                int tsdb_status = tsdbProcess(tempMacRecordDirID, "mac record dir create fail");

                //apiErrorAddTsDb(aiCamDeviceRecordID, "folder_create");

                snprintf(dbg_msg, sizeof(dbg_msg), "Creat aiCamDeviceMacRecordID Floder fail, waiting 1s, continue");
                // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
                Cdbg(APP_DBG, "%s", dbg_msg);

                sleep(1);
                continue;
            }

        }

    }

    return tempMacRecordDirID;
    // snprintf(dbg_msg, sizeof(dbg_msg), "Finish createWebStorageMacRecordDir [-22/AiCAM/%s/Record] , Dir ID is %lli\n", mac_name, tempMacRecordDirID);
    // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);

}

// if mac && alarm folder exist , check date id 
void createWebStorageMacAlarmDateDir(char * mac_name, int mac_type) {


    long long tempMacAlarmDateDirID;

    //  : process AiCAM(Dir) -> V2(Dir) -> Device ID(Dir) -> Uuid(Dir) -> alarm(Dir) -> date(Dir) (ex: -22/AiCAM/V2/AiCAM_215C34/mac001~mac004/alarm/20180227/)
    char dirname[64];

    memset(dirname,0,sizeof(dirname));
    strcpy(dirname, time_now);

    snprintf(dbg_msg, sizeof(dbg_msg), "check ../../%s/alarm/%s DIR \n", mac_name, time_now);
    // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
    Cdbg(APP_DBG, "%s", dbg_msg);

    while(1) {

        if(mac_type == 1) {

   
            aiCamMacAddressOneAlarmDateID = checkDirId(aiCamMacAddressOneAlarmID, dirname);
            tempMacAlarmDateDirID = aiCamMacAddressOneAlarmDateID;

            snprintf(dbg_msg, sizeof(dbg_msg), "Get aiCamMacAddressOneAlarmDateID : %lli", aiCamMacAddressOneAlarmDateID);



        } else if(mac_type == 2) {


            aiCamMacAddressTwoAlarmDateID = checkDirId(aiCamMacAddressTwoAlarmID, dirname);
            tempMacAlarmDateDirID = aiCamMacAddressTwoAlarmDateID;

            snprintf(dbg_msg, sizeof(dbg_msg), "Get aiCamMacAddressTwoAlarmDateID : %lli", aiCamMacAddressTwoAlarmDateID);


        } else if(mac_type == 3) {

            aiCamMacAddressThreeAlarmDateID = checkDirId(aiCamMacAddressThreeAlarmID, dirname);
            tempMacAlarmDateDirID = aiCamMacAddressThreeAlarmDateID;

            snprintf(dbg_msg, sizeof(dbg_msg), "Get aiCamMacAddressThreeAlarmDateID : %lli", aiCamMacAddressThreeAlarmDateID);


        } else if(mac_type == 4) {

            aiCamMacAddressFourAlarmDateID = checkDirId(aiCamMacAddressFourAlarmID, dirname);
            tempMacAlarmDateDirID = aiCamMacAddressFourAlarmDateID;

            snprintf(dbg_msg, sizeof(dbg_msg), "Get aiCamMacAddressFourAlarmDateID : %lli", aiCamMacAddressFourAlarmDateID);

        }

        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(APP_DBG, "%s", dbg_msg);


        // ID >= 1000 : get id sucess
        if((tempMacAlarmDateDirID == DIR_NEED_CREATE) || (tempMacAlarmDateDirID >= 1000)) {
            break;
        } else {

            int tsdb_status = tsdbProcess(tempMacAlarmDateDirID, "find macAlarmDateDir fail");

            snprintf(dbg_msg, sizeof(dbg_msg), "DeviceUuidAlarmDateID get data fail, waiting 1s, continue");
            // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
            Cdbg(APP_DBG, "%s", dbg_msg);

            sleep(1);
            continue;
        }

    }

    // not -22/AiCAM/V2/AiCAM_215C34/macxxxxxxxx/alarm DIR
    if(tempMacAlarmDateDirID == DIR_NEED_CREATE) {

        memset(dirname,0,sizeof(dirname));

        strcpy(dirname, "/../");
        strcat(dirname, mac_name);
        strcat(dirname, "/alarm/");
        strcat(dirname, time_now);
        
        snprintf(dbg_msg, sizeof(dbg_msg), "not ../../%s/alarm/%s DIR, Prepare Create AiCAM Device/Uuid/Alarm/Date dir : %s\n", mac_name, time_now, dirname);
        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(APP_DBG, "%s", dbg_msg);


        while(1) {

            if(mac_type == 1) {

                aiCamMacAddressOneAlarmDateID = createDirId(aiCamMacAddressOneAlarmID, dirname);
                tempMacAlarmDateDirID = aiCamMacAddressOneAlarmDateID;

                snprintf(dbg_msg, sizeof(dbg_msg), "Create aiCamMacAddressOneAlarmDateID : %lli", aiCamMacAddressOneAlarmDateID);
            
            } else if(mac_type == 2) {

                aiCamMacAddressTwoAlarmDateID = createDirId(aiCamMacAddressTwoAlarmID, dirname);
                tempMacAlarmDateDirID = aiCamMacAddressTwoAlarmDateID;

                snprintf(dbg_msg, sizeof(dbg_msg), "Create aiCamMacAddressTwoAlarmDateID : %lli", aiCamMacAddressTwoAlarmDateID);


            } else if(mac_type == 3) {
                
                aiCamMacAddressThreeAlarmDateID = createDirId(aiCamMacAddressThreeAlarmID, dirname);
                tempMacAlarmDateDirID = aiCamMacAddressThreeAlarmDateID;

                snprintf(dbg_msg, sizeof(dbg_msg), "Create aiCamMacAddressThreeAlarmDateID : %lli", aiCamMacAddressThreeAlarmDateID);


            } else if(mac_type == 4) {    

                aiCamMacAddressFourAlarmDateID = createDirId(aiCamMacAddressFourAlarmID, dirname);
                tempMacAlarmDateDirID = aiCamMacAddressFourAlarmDateID;

                snprintf(dbg_msg, sizeof(dbg_msg), "Create aiCamMacAddressFourAlarmDateID : %lli", aiCamMacAddressFourAlarmDateID);


            }

            // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
            Cdbg(APP_DBG, "%s", dbg_msg);


            // ID >= 1000 : get id sucess
            if(tempMacAlarmDateDirID >= 1000) {
            //if((aiCamDeviceAlarmID == DIR_NEED_CREATE) || (aiCamDeviceAlarmID >= 1000)) {
                break;
            } else {

                int tsdb_status = tsdbProcess(tempMacAlarmDateDirID, "macAlarmDateDir create fail");

                //apiErrorAddTsDb(aiCamDeviceAlarmID, "folder_create");

                snprintf(dbg_msg, sizeof(dbg_msg), "Creat aiCamDeviceUuidAlarmDateID Floder fail, waiting 1s, continue");
                // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
                Cdbg(APP_DBG, "%s", dbg_msg);

                sleep(1);
                continue;
            }

        }

    }

    // snprintf(dbg_msg, sizeof(dbg_msg), "Finish createWebStorageMacAlarmDir [-22/AiCAM/V2/%s/%s/alarm] , Dir ID is %lli\n", AICAM_ALARM_PATH, mac_name, tempMacAlarmDateDirID);
    // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);

}


// if mac && Record folder exist , check date id 
void createWebStorageMacRecordDateDir(char * mac_name, int mac_type) {


    long long tempMacRecordDateDirID;

    //  : process AiCAM(Dir) ->Mac(Dir) -> Record(Dir) -> date(Dir) (ex: -22/AiCAM/AiCAM_215C34/mac001~mac004/Record/20180227/)
    char dirname[64];

    memset(dirname,0,sizeof(dirname));
    strcpy(dirname, time_now);

    snprintf(dbg_msg, sizeof(dbg_msg), "check ../../%s/record/%s DIR \n", mac_name, time_now);
    // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
    Cdbg(APP_DBG, "%s", dbg_msg);

    while(1) {

        if(mac_type == 1) {

   
            aiCamMacAddressOneRecordDateID = checkDirId(aiCamMacAddressOneRecordID, dirname);
            tempMacRecordDateDirID = aiCamMacAddressOneRecordDateID;

            snprintf(dbg_msg, sizeof(dbg_msg), "Get aiCamMacAddressOneRecordDateID : %lli", aiCamMacAddressOneRecordDateID);



        } else if(mac_type == 2) {


            aiCamMacAddressTwoRecordDateID = checkDirId(aiCamMacAddressTwoRecordID, dirname);
            tempMacRecordDateDirID = aiCamMacAddressTwoRecordDateID;

            snprintf(dbg_msg, sizeof(dbg_msg), "Get aiCamMacAddressTwoRecordDateID : %lli", aiCamMacAddressTwoRecordDateID);


        } else if(mac_type == 3) {

            aiCamMacAddressThreeRecordDateID = checkDirId(aiCamMacAddressThreeRecordID, dirname);
            tempMacRecordDateDirID = aiCamMacAddressThreeRecordDateID;

            snprintf(dbg_msg, sizeof(dbg_msg), "Get aiCamMacAddressThreeRecordDateID : %lli", aiCamMacAddressThreeRecordDateID);


        } else if(mac_type == 4) {

            aiCamMacAddressFourRecordDateID = checkDirId(aiCamMacAddressFourRecordID, dirname);
            tempMacRecordDateDirID = aiCamMacAddressFourRecordDateID;

            snprintf(dbg_msg, sizeof(dbg_msg), "Get aiCamMacAddressFourRecordDateID : %lli", aiCamMacAddressFourRecordDateID);

        }

        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(APP_DBG, "%s", dbg_msg);


        // ID >= 1000 : get id sucess
        if((tempMacRecordDateDirID == DIR_NEED_CREATE) || (tempMacRecordDateDirID >= 1000)) {
            break;
        } else {

            int tsdb_status = tsdbProcess(tempMacRecordDateDirID, "find macRecordDateDir fail");

            snprintf(dbg_msg, sizeof(dbg_msg), "DeviceUuidRecordDateID get data fail, waiting 1s, continue");
            // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
            Cdbg(APP_DBG, "%s", dbg_msg);

            sleep(1);
            continue;
        }

    }

    // not -22/AiCAM/V2/AiCAM_215C34/macxxxxxxxx/Record DIR
    if(tempMacRecordDateDirID == DIR_NEED_CREATE) {

        memset(dirname,0,sizeof(dirname));

        strcpy(dirname, "/../");
        strcat(dirname, mac_name);
        strcat(dirname, "/record/");
        strcat(dirname, time_now);
        
        snprintf(dbg_msg, sizeof(dbg_msg), "not ../../%s/record/%s DIR, Prepare Create AiCAM Device/Uuid/Record/Date dir : %s\n", mac_name, time_now, dirname);
        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(APP_DBG, "%s", dbg_msg);


        while(1) {

            if(mac_type == 1) {

                aiCamMacAddressOneRecordDateID = createDirId(aiCamMacAddressOneRecordID, dirname);
                tempMacRecordDateDirID = aiCamMacAddressOneRecordDateID;

                snprintf(dbg_msg, sizeof(dbg_msg), "Create aiCamMacAddressOneRecordDateID : %lli", aiCamMacAddressOneRecordDateID);
            
            } else if(mac_type == 2) {

                aiCamMacAddressTwoRecordDateID = createDirId(aiCamMacAddressTwoRecordID, dirname);
                tempMacRecordDateDirID = aiCamMacAddressTwoRecordDateID;

                snprintf(dbg_msg, sizeof(dbg_msg), "Create aiCamMacAddressTwoRecordDateID : %lli", aiCamMacAddressTwoRecordDateID);


            } else if(mac_type == 3) {
                
                aiCamMacAddressThreeRecordDateID = createDirId(aiCamMacAddressThreeRecordID, dirname);
                tempMacRecordDateDirID = aiCamMacAddressThreeRecordDateID;

                snprintf(dbg_msg, sizeof(dbg_msg), "Create aiCamMacAddressThreeRecordDateID : %lli", aiCamMacAddressThreeRecordDateID);


            } else if(mac_type == 4) {    

                aiCamMacAddressFourRecordDateID = createDirId(aiCamMacAddressFourRecordID, dirname);
                tempMacRecordDateDirID = aiCamMacAddressFourRecordDateID;

                snprintf(dbg_msg, sizeof(dbg_msg), "Create aiCamMacAddressFourRecordDateID : %lli", aiCamMacAddressFourRecordDateID);


            }

            // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
            Cdbg(APP_DBG, "%s", dbg_msg);


            // ID >= 1000 : get id sucess
            if(tempMacRecordDateDirID >= 1000) {
            //if((aiCamDeviceRecordID == DIR_NEED_CREATE) || (aiCamDeviceRecordID >= 1000)) {
                break;
            } else {

                int tsdb_status = tsdbProcess(tempMacRecordDateDirID, "macRecordDateDir create fail");

                //apiErrorAddTsDb(aiCamDeviceRecordID, "folder_create");

                snprintf(dbg_msg, sizeof(dbg_msg), "Creat aiCamDeviceUuidRecordDateID Floder fail, waiting 1s, continue");
                // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
                Cdbg(APP_DBG, "%s", dbg_msg);

                sleep(1);
                continue;
            }

        }

    }

    // snprintf(dbg_msg, sizeof(dbg_msg), "Finish createWebStorageMacRecordDir [-22/AiCAM/%s/%s/record] , Dir ID is %lli\n", mac_name, tempMacRecordDateDirID);
    // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);

}
void createWebStorageRecordDateDir() {

    char dirname[64];

    memset(dirname,0,sizeof(dirname));
    strcpy(dirname, time_now);

    Cdbg(APP_DBG, "aiCamDeviceRecordID : %lli", aiCamDeviceRecordID);
    // Cdbg(APP_DBG, "check 22/AiCAM/%s/record/%s DIR \n", asus_conf.mac_name, dirname);
    Cdbg(APP_DBG, "check 22/AiCAM/%s/record/%s DIR \n", aicam_device_dir, dirname);
    
    while(1) {

        aiCamDeviceRecordDateID = checkDirId(aiCamDeviceRecordID, dirname);

        Cdbg(APP_DBG, "aiCamDeviceRecordDateID = %lli\n", aiCamDeviceRecordDateID);

        // ID >= 1000 : get id sucess
        if((aiCamDeviceRecordDateID == DIR_NEED_CREATE) || (aiCamDeviceRecordDateID >= 1000)) {
            break;
        } else {

            int tsdb_status = tsdbProcess(aiCamDeviceRecordDateID, "propfind:RecordDate find Fail");

            //apiErrorAddTsDb(aiCamDeviceRecordDateID, "propfind");

            Cdbg(APP_DBG, "aiCamDeviceRecordDateID get data fail, waiting 1s, continue \n");
            sleep(1);
            continue;
        }

    }

    Cdbg(APP_DBG, "Get aiCamDeviceRecordDateID ID is %lli\n", aiCamDeviceRecordDateID);
    
    // not -22/AiCAM/AiCAM_215C34/tmp_xxxxxx DIR
    if(aiCamDeviceRecordDateID == DIR_NEED_CREATE) {

        memset(dirname,0,sizeof(dirname));
        //strcpy(dirname, "/tmp/AiCAM/AiCAM_215C34/record/");

        strcpy(dirname, asus_conf.uploader_path);
        strcat(dirname, "/AiCAM/");
        // strcat(dirname, asus_conf.mac_name);
        strcat(dirname, aicam_device_dir);
        strcat(dirname, "/record/");
        strcat (dirname,time_now);
        
        Cdbg(APP_DBG, "not -22/AiCAM/%s/record/date DIR, aiCamDeviceRecordID is %lli , Prepare Create AiCAM Device Record date dir : %s\n", aicam_device_dir, aiCamDeviceRecordID, time_now);

        while(1) {

            aiCamDeviceRecordDateID = createDirId(aiCamDeviceRecordID, dirname);

            Cdbg(APP_DBG, "aiCamDeviceRecordDateID = %lli\n", aiCamDeviceRecordDateID);

            // ID >= 1000 : get id sucess
            if(aiCamDeviceRecordDateID >= 1000) {
            //if((aiCamDeviceRecordDateID == DIR_NEED_CREATE) || (aiCamDeviceRecordDateID >= 1000)) {
                break;
            } else {

                int tsdb_status = tsdbProcess(aiCamDeviceRecordDateID, "RecordDate folder create fail");

                //apiErrorAddTsDb(aiCamDeviceRecordDateID, "folder_create");

                Cdbg(APP_DBG, "Creat aiCamDeviceRecordDate Floder fail, waiting 1s, continue \n");
                sleep(1);
                continue;
            }
        }
        
    }
    
    Cdbg(APP_DBG, "Check aiCamDeviceRecordDateID is %lli , check dirname : %s\n", aiCamDeviceRecordDateID, dirname);


}



void createWebStorageAlarmDateDir() {

    char dirname[64];

    memset(dirname,0,sizeof(dirname));
    strcpy(dirname, time_now);


    // Cdbg(APP_DBG, "aiCamDeviceAlarmID : %lli, check 22/AiCAM/%s/alarm/%s DIR \n", aiCamDeviceAlarmID, asus_conf.mac_name, dirname);
    Cdbg(APP_DBG, "aiCamDeviceAlarmID : %lli, check -22/AiCAM/%s/alarm/%s DIR \n", aiCamDeviceAlarmID, aicam_device_dir, dirname);
    
    
    while(1) {

        aiCamDeviceAlarmDateID = checkDirId(aiCamDeviceAlarmID, dirname);

        Cdbg(APP_DBG, "aiCamDeviceAlarmDateID = %lli\n", aiCamDeviceAlarmDateID);

        // ID >= 1000 : get id sucess
        if((aiCamDeviceAlarmDateID == DIR_NEED_CREATE) || (aiCamDeviceAlarmDateID >= 1000)) {
            break;
        } else {

            int tsdb_status = tsdbProcess(aiCamDeviceAlarmDateID, "propfind:AlarmDate find fail");

            //apiErrorAddTsDb(aiCamDeviceAlarmDateID, "propfind");

            Cdbg(APP_DBG, "aiCamDeviceAlarmDateID get data fail, waiting 1s, continue \n");
            sleep(1);
            continue;
        }

    }
    

    Cdbg(APP_DBG, "Get aiCamDeviceAlarmDateID ID is %lli\n", aiCamDeviceAlarmDateID);
    
    // not -22/AiCAM/AiCAM_215C34/tmp_xxxxxx DIR
    if(aiCamDeviceAlarmDateID == 0) {

        memset(dirname,0,sizeof(dirname));
        //strcpy(dirname, "/tmp/AiCAM/AiCAM_215C34/record/");

        strcpy(dirname, asus_conf.uploader_path);
        strcat(dirname, "/AiCAM/");
        // strcat(dirname, asus_conf.mac_name);
        strcat(dirname, aicam_device_dir);
        strcat(dirname, "/alarm/");
        strcat(dirname, time_now);
        
        Cdbg(APP_DBG, "not -22/AiCAM/%s/alarm/%s DIR, aiCamDeviceAlarmID is %lli , Prepare Create AiCAM Device Alarm date dir : %s\n", aicam_device_dir, time_now, aiCamDeviceAlarmID, dirname);

        while(1) {

            aiCamDeviceAlarmDateID = createDirId(aiCamDeviceAlarmID, dirname);

            Cdbg(APP_DBG, "aiCamDeviceAlarmDateID = %lli\n", aiCamDeviceAlarmDateID);

            // ID >= 1000 : get id sucess
            if(aiCamDeviceAlarmDateID >= 1000) {
            //if((aiCamDeviceAlarmDateID == DIR_NEED_CREATE) || (aiCamDeviceAlarmDateID >= 1000)) {
                break;
            } else {

                int tsdb_status = tsdbProcess(aiCamDeviceAlarmDateID, "AlarmDate folder create fail");

                //apiErrorAddTsDb(aiCamDeviceAlarmDateID, "folder_create");

                Cdbg(APP_DBG, "Creat aiCamDeviceAlarmDateID Floder fail, waiting 1s, continue \n");
                sleep(1);
                continue;
            }

        }
        
    }
    
    Cdbg(APP_DBG, "Check aiCamDeviceRecordDateID is %lli , check dirname : %s\n", aiCamDeviceAlarmDateID, dirname);
}




void createAlarmDateDir() {

    char dirname[64];

    memset(dirname,0,sizeof(dirname));
    strcpy(dirname, time_now);


    // Cdbg(APP_DBG, "aiCamDeviceAlarmID : %lli, check 22/AiCAM/%s/alarm/%s DIR \n", aiCamDeviceAlarmID, asus_conf.mac_name, dirname);
    Cdbg(APP_DBG, "aiCamDeviceAlarmID : %lli, check 22/AiCAM/V2/%s/alarm/%s DIR \n", aiCamDeviceAlarmID, asus_conf.mac_name, dirname);
    
    
    while(1) {

        aiCamDeviceAlarmDateID = checkDirId(aiCamDeviceAlarmID, dirname);

        Cdbg(APP_DBG, "aiCamDeviceAlarmDateID = %lli\n", aiCamDeviceAlarmDateID);

        // ID >= 1000 : get id sucess
        if((aiCamDeviceAlarmDateID == DIR_NEED_CREATE) || (aiCamDeviceAlarmDateID >= 1000)) {
            break;
        } else {

            int tsdb_status = tsdbProcess(aiCamDeviceAlarmDateID, "propfind:AlarmDate find fail");

            //apiErrorAddTsDb(aiCamDeviceAlarmDateID, "propfind");

            Cdbg(APP_DBG, "aiCamDeviceAlarmDateID get data fail, waiting 1s, continue \n");
            sleep(1);
            continue;
        }

    }
    

    Cdbg(APP_DBG, "Get aiCamDeviceAlarmDateID ID is %lli\n", aiCamDeviceAlarmDateID);
    
    // not -22/AiCAM/V2/AiCAM_215C34/tmp_xxxxxx DIR
    if(aiCamDeviceAlarmDateID == 0) {

        memset(dirname,0,sizeof(dirname));
        //strcpy(dirname, "/tmp/AiCAM/AiCAM_215C34/record/");

        strcpy(dirname, asus_conf.uploader_path);
        strcat(dirname, "/AiCAM/");
        strcat(dirname, "V2/");
        strcat(dirname, asus_conf.mac_name);
        strcat(dirname, "/alarm/");
        strcat(dirname, time_now);
        
        Cdbg(APP_DBG, "not -22/AiCAM/V2/%s/alarm/%s DIR, aiCamDeviceAlarmID is %lli , Prepare Create AiCAM Device Alarm date dir : %s\n", asus_conf.mac_name, time_now, aiCamDeviceAlarmID, dirname);

        while(1) {

            aiCamDeviceAlarmDateID = createDirId(aiCamDeviceAlarmID, dirname);

            Cdbg(APP_DBG, "aiCamDeviceAlarmDateID = %lli\n", aiCamDeviceAlarmDateID);

            // ID >= 1000 : get id sucess
            if(aiCamDeviceAlarmDateID >= 1000) {
            //if((aiCamDeviceAlarmDateID == DIR_NEED_CREATE) || (aiCamDeviceAlarmDateID >= 1000)) {
                break;
            } else {

                int tsdb_status = tsdbProcess(aiCamDeviceAlarmDateID, "AlarmDate folder create fail");

                //apiErrorAddTsDb(aiCamDeviceAlarmDateID, "folder_create");

                Cdbg(APP_DBG, "Creat aiCamDeviceAlarmDateID Floder fail, waiting 1s, continue \n");
                sleep(1);
                continue;
            }

        }
        
    }
    
    Cdbg(APP_DBG, "Check aiCamDeviceRecordDateID is %lli , check dirname : %s\n", aiCamDeviceAlarmDateID, dirname);
}

long long createWebStorageDir(char * create_dir, char * aicam_all_dir, long long parentID) {


    long long getAiCamDirID = -1;

    while(1) {

        Cdbg(APP_DBG, "Check parentID : %lli, DIR = %s", parentID, create_dir);

        getAiCamDirID = checkDirId(parentID, create_dir);

        Cdbg(APP_DBG, "getAiCamDirID = %lli\n", getAiCamDirID);

        // DIR_NEED_CREATE = 0, need create dir
        // ID >= 1000 : get id sucess
        if((getAiCamDirID == DIR_NEED_CREATE) || (getAiCamDirID >= 1000)) {
            break;
        } else {

            int tsdb_status = tsdbProcess(getAiCamDirID, "propfind:AiCamDir find fail");

            //apiErrorAddTsDb(getAiCamDirID, "propfind");

            Cdbg(APP_DBG, "getAiCamDirID get data fail, waiting 1s, continue \n");
            sleep(1);
            continue;
        }
   }

    Cdbg(APP_DBG, "Get getAiCamDirID ID is %lli\n", getAiCamDirID);
    

    // not -22/AiCAM/AiCAM_215C34/tmp_xxxxxx DIR
    if(getAiCamDirID == DIR_NEED_CREATE) {
        
        Cdbg(APP_DBG, "not DIR -> %s, Prepare Create AiCAM dir : %s, parentID = %lli", aicam_all_dir, create_dir, parentID);

        while(1) {

            getAiCamDirID = createDirId(parentID, aicam_all_dir);

            Cdbg(APP_DBG, "getAiCamDirID = %lli\n", getAiCamDirID);

            // ID >= 1000 : get id sucess
            if(getAiCamDirID >= 1000) {
            //if((getAiCamDirID == DIR_NEED_CREATE) || (getAiCamDirID >= 1000)) {
                break;
            } else {

                int tsdb_status = tsdbProcess(getAiCamDirID, "AiCamDir folder create fail");

                //apiErrorAddTsDb(getAiCamDirID, "folder_create");

                Cdbg(APP_DBG, "Creat getAiCamDirID Floder fail, waiting 1s, continue \n");
                sleep(1);
                continue;
            }


        }
        
    }

    return getAiCamDirID;

}



// webstorage aicam dir check add && add
int create_aicam_dir() {

    
    long long parentID = AICAM_ROOT_ID; // -22;
    //char *dirname = "AiCam";
    char dirname[64];
    
    memset(dirname,0,sizeof(dirname));
    strcpy(dirname,"AiCAM");
    
    Cdbg(APP_DBG, "aicam_dir_check -> AICAM_ROOT_ID = %lli , check aicam dir : %s\n", parentID, dirname);
    
    // step 1 : process AiCam(Dir) (ex: -22/AiCam ) [-22 is aicam ParentID]
    // sure aiCamId get success
    while(1) {

        aiCamID = checkDirId(parentID, dirname);

        Cdbg(APP_DBG, "Get aiCamID = %lli\n", aiCamID);

        // DIR_NEED_CREATE = 0, need create dir
        // ID >= 1000 : get id sucess
        if((aiCamID == DIR_NEED_CREATE) || (aiCamID >= 1000)) {
            break;
        } else {

            int tsdb_status = tsdbProcess(aiCamID, "Get aiCamID fail");
            //apiErrorAddTsDb(aiCamID, "propfind");

            Cdbg(APP_DBG, "Get aiCamID fail, waiting 1s, continue \n");
            sleep(1);

            continue;
        }

    }

    // not -22/AiCam DIR, need create aiCam dir
    if(aiCamID == DIR_NEED_CREATE) {

        memset(dirname,0,sizeof(dirname));
        //strcpy(dirname, "/tmp/AiCAM");
        strcpy(dirname, asus_conf.uploader_path);
        strcat(dirname, "/AiCAM");
        
        Cdbg(APP_DBG, "Not -22/AiCAM DIR, parentID = %lli , Prepare Create AiCAM dir : %s \n", parentID, dirname);

        // must get aiCamID
        while(1) {

            aiCamID = createDirId(parentID, dirname);

            Cdbg(APP_DBG, "Get aiCamID = %lli\n", aiCamID);

            // aiCamID >= 1000 : get aiCamID success
            //if((aiCamID == DIR_NEED_CREATE) || (aiCamID >= 1000)) {
            if(aiCamID >= 1000) {
                break;
            } else {

                int tsdb_status = tsdbProcess(aiCamID, "Create aiCamID folder fail");
                //apiErrorAddTsDb(aiCamID, "folder_create");

                Cdbg(APP_DBG, "Creat aiCamID Floder fail, waiting 1s, continue \n");
                sleep(1);
                continue;
            }

        }
    }
    
    Cdbg(APP_DBG, "WebStorage Dir ->  [-22/AiCAM], Get Dir ID (aiCamID) =  %lli\n", aiCamID);




    // process AiCAM(Dir) -> V2 ID(Dir) (ex: -22/AiCAM/V2)
    memset(dirname,0,sizeof(dirname));
    strcpy(dirname, "V2");
    //strcpy(dirname, aicam_device_dir);


    while(1) {

        aiCamV2ID = checkDirId(aiCamID, dirname);

        Cdbg(APP_DBG, "Get aiCamV2ID = %lli\n", aiCamV2ID);

        // ID >= 1000 : get id sucess
        if((aiCamV2ID == DIR_NEED_CREATE) || (aiCamV2ID >= 1000)) {
            break;
        } else {

            int tsdb_status = tsdbProcess(aiCamV2ID, "Get aiCamV2ID fail");
            //apiErrorAddTsDb(aiCamDeviceID, "propfind");

            Cdbg(APP_DBG, "Get aiCamV2ID fail, waiting 1s, continue \n");
            sleep(1);
            continue;
        }


    }

    // not -22/AiCAM/V2 DIR
    if(aiCamV2ID == DIR_NEED_CREATE) {

        memset(dirname,0,sizeof(dirname));
        //strcpy(dirname, "/tmp/AiCAM/AiCAM_215C34");

        strcpy(dirname, asus_conf.uploader_path);
        strcat(dirname, "AiCAM/");
        // strcat(dirname, asus_conf.mac_name);
        strcat(dirname, "V2");
        
        Cdbg(APP_DBG, "not -22/AiCAM/V2 DIR, aiCamV2ID is %lli , Prepare Create AiCAM/V2 dir : %s\n", aiCamID, dirname);

        while(1) {

            aiCamV2ID = createDirId(aiCamID, dirname);

            Cdbg(APP_DBG, "Get aiCamV2ID = %lli\n", aiCamV2ID);

            // ID >= 1000 : get id sucess
            if(aiCamV2ID >= 1000) {
            //if((aiCamV2ID == DIR_NEED_CREATE) || (aiCamV2ID >= 1000)) {
                break;
            } else {

                int tsdb_status = tsdbProcess(aiCamV2ID, "V2 folder create fail");

                //apiErrorAddTsDb(aiCamDeviceID, "folder_create");

                Cdbg(APP_DBG, "Creat V2 Floder fail, waiting 1s, continue \n");
                sleep(1);
                continue;
            }
        }

    }
    
    Cdbg(APP_DBG, "-22/AiCAM/V2 DIR ID is %lli\n", aiCamV2ID);

    
    return 0;
    
}


// webstorage aicam dir check add && add
int aicam_dir_check() {

    
    long long parentID = AICAM_ROOT_ID; // -22;
    //char *dirname = "AiCam";
    char dirname[64];
    
    memset(dirname,0,sizeof(dirname));
    strcpy(dirname,"AiCAM");
    
    Cdbg(APP_DBG, "aicam_dir_check -> AICAM_ROOT_ID = %lli , check aicam dir : %s\n", parentID, dirname);
    
    // step 1 : process AiCam(Dir) (ex: -22/AiCam ) [-22 is aicam ParentID]
    // sure aiCamId get success
    while(1) {

        aiCamID = checkDirId(parentID, dirname);

        Cdbg(APP_DBG, "aiCamID = %lli\n", aiCamID);

        // DIR_NEED_CREATE = 0, need create dir
        // ID >= 1000 : get id sucess
        if((aiCamID == DIR_NEED_CREATE) || (aiCamID >= 1000)) {
            break;
        } else {

            int tsdb_status = tsdbProcess(aiCamID, "propfind:-22 dir find fail");
            //apiErrorAddTsDb(aiCamID, "propfind");

            Cdbg(APP_DBG, "aiCamID get data fail, waiting 1s, continue \n");
            sleep(1);

            continue;
        }

    }

    Cdbg(APP_DBG, "Get conf.uploader_path ID is %s\n",asus_conf.uploader_path);
    Cdbg(APP_DBG, "Get AiCAM ID is %lli\n", aiCamID);


    // not -22/AiCam DIR, need create aiCam dir
    if(aiCamID == DIR_NEED_CREATE) {

        memset(dirname,0,sizeof(dirname));
        //strcpy(dirname, "/tmp/AiCAM");
        strcpy(dirname, asus_conf.uploader_path);
        strcat(dirname, "/AiCAM");
        
        Cdbg(APP_DBG, "Not -22/AiCAM DIR, parentID = %lli , Prepare Create AiCAM dir : %s \n", parentID, dirname);

        // must get aiCamID
        while(1) {

            aiCamID = createDirId(parentID, dirname);

            Cdbg(APP_DBG, "aiCamID = %lli\n", aiCamID);

            // aiCamID >= 1000 : get aiCamID success
            //if((aiCamID == DIR_NEED_CREATE) || (aiCamID >= 1000)) {
            if(aiCamID >= 1000) {
                break;
            } else {

                int tsdb_status = tsdbProcess(aiCamID, "-22 parent folder create fail");
                //apiErrorAddTsDb(aiCamID, "folder_create");

                Cdbg(APP_DBG, "Creat aiCamID Floder fail, waiting 1s, continue \n");
                sleep(1);
                continue;
            }

        }
    }
    
    Cdbg(APP_DBG, "WebStorage Dir ->  [-22/AiCAM], get Dir ID =  %lli\n", aiCamID);

    // process time plan
    if( space_plan == SPACE_SIZE_PLAN) {

        spacePlanDir();

    // process space plan
    } else {

        timePlanDir();

    }

    //timePlanDir();
    
    // record asus, create record date dir
    if(upload_type == RECORD_ASUS_ALARM_ASUS) {

        if( space_plan == TIME_CONTINOUS_PLAN) {

            createWebStorageRecordDateDir();
        }


    }


    if((aiCamDeviceAlarmDateID == 0) || (aiCamDeviceAlarmID == 0)) {
        // add error log

        return -1;
    }
    
    
    return 0;
    
}

void timePlanDir() {


    char dirname[64];

    // step 2 : process AiCAM(Dir) -> Device ID(Dir) (ex: -22/AiCAM/AICAM_000C29)
    memset(dirname,0,sizeof(dirname));
    // strcpy(dirname,"AiCAM_215C34");
    // strcpy(dirname, asus_conf.mac_name);
    strcpy(dirname, aicam_device_dir);
    Cdbg(APP_DBG, "check -22/AiCAM/%s DIR \n", dirname);

    
    while(1) {

        aiCamDeviceID = checkDirId(aiCamID, dirname);

        Cdbg(APP_DBG, "aiCamDeviceID = %lli\n", aiCamDeviceID);

        // ID >= 1000 : get id sucess
        if((aiCamDeviceID == DIR_NEED_CREATE) || (aiCamDeviceID >= 1000)) {
            break;
        } else {

            int tsdb_status = tsdbProcess(aiCamDeviceID, "propfind:DeviceDir find fail");
            //apiErrorAddTsDb(aiCamDeviceID, "propfind");

            Cdbg(APP_DBG, "aiCamDeviceID get data fail, waiting 1s, continue \n");
            sleep(1);
            continue;
        }


    }

    Cdbg(APP_DBG, "Get aiCamDeviceID ID is %lli\n", aiCamDeviceID);


    // not -22/AiCAM/AiCAM_215C34 DIR
    if(aiCamDeviceID == DIR_NEED_CREATE) {

        memset(dirname,0,sizeof(dirname));
        //strcpy(dirname, "/tmp/AiCAM/AiCAM_215C34");

        strcpy(dirname, asus_conf.uploader_path);
        strcat(dirname, "/AiCAM/");
        // strcat(dirname, asus_conf.mac_name);
        strcat(dirname, aicam_device_dir);
        
        Cdbg(APP_DBG, "not -22/AiCAM/%s DIR, aiCamID is %lli , Prepare Create AiCAM Device dir : %s\n", aicam_device_dir, aiCamID, dirname);


        while(1) {

            aiCamDeviceID = createDirId(aiCamID, dirname);

            Cdbg(APP_DBG, "aiCamDeviceID = %lli\n", aiCamDeviceID);

            // ID >= 1000 : get id sucess
            if(aiCamDeviceID >= 1000) {
            //if((aiCamDeviceID == DIR_NEED_CREATE) || (aiCamDeviceID >= 1000)) {
                break;
            } else {

                int tsdb_status = tsdbProcess(aiCamDeviceID, "Device folder create fail");

                //apiErrorAddTsDb(aiCamDeviceID, "folder_create");

                Cdbg(APP_DBG, "Creat aiCamDeviceID Floder fail, waiting 1s, continue \n");
                sleep(1);
                continue;
            }
        }

    }
    
    //Cdbg(APP_DBG, "-22/AiCAM/%s/ DIR ID is %lli\n", asus_conf.mac_name, aiCamDeviceID);
    Cdbg(APP_DBG, "-22/AiCAM/%s/ DIR ID is %lli\n", aicam_device_dir, aiCamDeviceID);


    // step 3-1 : process AiCAM(Dir) -> Device ID(Dir) -> Alarm(Dir) (ex: -22/AiCAM/AiCAM_215C34/Alarm)
    memset(dirname,0,sizeof(dirname));
    strcpy(dirname,"alarm");
    
    
    //Cdbg(APP_DBG, "aiCamDeviceID id : %lli, check 22/AiCAM/%s/%s DIR \n", aiCamDeviceID, asus_conf.mac_name, dirname);
    Cdbg(APP_DBG, "aiCamDeviceID id : %lli, check 22/AiCAM/%s/%s DIR \n", aiCamDeviceID, aicam_device_dir, dirname);

    
    while(1) {


        aiCamDeviceAlarmID = checkDirId(aiCamDeviceID, dirname);

        Cdbg(APP_DBG, "aiCamDeviceAlarmID = %lli\n", aiCamDeviceAlarmID);

        // ID >= 1000 : get id sucess
        if((aiCamDeviceAlarmID == DIR_NEED_CREATE) || (aiCamDeviceAlarmID >= 1000)) {
            break;
        } else {

            int tsdb_status = tsdbProcess(aiCamDeviceAlarmID, "propfind:AlarmDir find fail");

            //apiErrorAddTsDb(aiCamDeviceAlarmID, "propfind");

            Cdbg(APP_DBG, "aiCamDeviceAlarmID get data fail, waiting 1s, continue \n");
            sleep(1);
            continue;
        }

    }
    Cdbg(APP_DBG, "Get aiCamDeviceAlarmID ID is %lli\n", aiCamDeviceAlarmID);


    // not -22/AiCAM/AiCAM_215C34/Alarm DIR
    if(aiCamDeviceAlarmID == DIR_NEED_CREATE) {

        memset(dirname,0,sizeof(dirname));
        //strcpy(dirname, "/tmp/AiCAM/AiCAM_215C34/alarm");

        strcpy(dirname, asus_conf.uploader_path);
        strcat(dirname, "/AiCAM/");
        // strcat(dirname, asus_conf.mac_name);
        strcat(dirname, aicam_device_dir);
        strcat(dirname, "/alarm");
        
        Cdbg(APP_DBG, "not -22/AiCAM/%s/alarm DIR, aiCamDeviceID is %lli , Prepare Create AiCAM Device Alarm dir : %s\n", aicam_device_dir, aiCamDeviceID, dirname);


        while(1) {

            aiCamDeviceAlarmID = createDirId(aiCamDeviceID, dirname);

            Cdbg(APP_DBG, "aiCamDeviceAlarmID = %lli\n", aiCamDeviceAlarmID);

            // ID >= 1000 : get id sucess
            if(aiCamDeviceAlarmID >= 1000) {
            //if((aiCamDeviceAlarmID == DIR_NEED_CREATE) || (aiCamDeviceAlarmID >= 1000)) {
                break;
            } else {

                int tsdb_status = tsdbProcess(aiCamDeviceAlarmID, "Alarm folder create fail");

                //apiErrorAddTsDb(aiCamDeviceAlarmID, "folder_create");

                Cdbg(APP_DBG, "Creat aiCamDeviceAlarmID Floder fail, waiting 1s, continue \n");
                sleep(1);
                continue;
            }

        }



    }

    //Cdbg(APP_DBG, "-22/AiCAM/%s/alarm DIR ID is %lli\n", asus_conf.mac_name, aiCamDeviceAlarmID);
    Cdbg(APP_DBG, "-22/AiCAM/%s/alarm DIR ID is %lli\n", aicam_device_dir, aiCamDeviceAlarmID);


    time_t t = time(0);

    //strftime( tmp, sizeof(tmp), "%Y-%m-%d %X:%A:%S 本年第%j天 %z",localtime(&t) );

    memset(time_now,0, sizeof(time_now));

    strftime( time_now, sizeof(time_now), "%Y%m%d",localtime(&t) );
    Cdbg(APP_DBG, "time_now : %s\n", time_now);

    memset(dirname,0,sizeof(dirname));
    strcpy(dirname, time_now);


    createWebStorageAlarmDateDir();


    // step 3-4 : process AiCAM(Dir) -> Device ID(Dir) -> aiCamDeviceRecordID(Dir) (ex: -22/AiCAM/AiCAM_215C34/record)

    // createAiCamDeviceRecordDir
    memset(dirname,0,sizeof(dirname));
    //strcpy(dirname, "/tmp/AiCAM/AiCAM_215C34/record");
    strcpy(dirname, asus_conf.uploader_path);
    strcat(dirname, "/AiCAM/");
    // strcat(dirname, asus_conf.mac_name);
    strcat(dirname, aicam_device_dir);
    strcat(dirname, "/record");
        
    aiCamDeviceRecordID = createWebStorageDir("record", dirname, aiCamDeviceID);


   /*
    while(1) {


        aiCamDeviceRecordID = checkDirId(aiCamDeviceID, dirname);

        Cdbg(APP_DBG, "aiCamDeviceRecordID = %lli\n", aiCamDeviceRecordID);

        // DIR_NEED_CREATE = 0, need create dir
        // ID >= 1000 : get id sucess
        if((aiCamDeviceRecordID == DIR_NEED_CREATE) || (aiCamDeviceRecordID >= 1000)) {
            break;
        } else {

            int tsdb_status = tsdbProcess(aiCamDeviceRecordID, "propfind");

            //apiErrorAddTsDb(aiCamDeviceRecordID, "propfind");

            Cdbg(APP_DBG, "aiCamDeviceRecordID get data fail, waiting 1s, continue \n");
            sleep(1);
            continue;
        }
   }

    Cdbg(APP_DBG, "Get aiCamDeviceRecordID ID is %lli\n", aiCamDeviceRecordID);
    

    // not -22/AiCAM/AiCAM_215C34/tmp_xxxxxx DIR
    if(aiCamDeviceRecordID == DIR_NEED_CREATE) {

        memset(dirname,0,sizeof(dirname));
        //strcpy(dirname, "/tmp/AiCAM/AiCAM_215C34/record");

        strcpy(dirname, asus_conf.uploader_path);
        strcat(dirname, "/AiCAM/");
        strcat(dirname, asus_conf.mac_name);
        strcat(dirname, "/record");
        
        Cdbg(APP_DBG, "not -22/AiCAM/AiCAM_XXXXXX/record DIR, aiCamDeviceID is %lli , Prepare Create AiCAM Device Record dir : %s\n", aiCamDeviceID, dirname);

        while(1) {

            aiCamDeviceRecordID = createDirId(aiCamDeviceID, dirname);

            Cdbg(APP_DBG, "aiCamDeviceRecordID = %lli\n", aiCamDeviceRecordID);

            // ID >= 1000 : get id sucess
            if(aiCamDeviceRecordID >= 1000) {
            //if((aiCamDeviceRecordID == DIR_NEED_CREATE) || (aiCamDeviceRecordID >= 1000)) {
                break;
            } else {

                int tsdb_status = tsdbProcess(aiCamDeviceRecordID, "folder_create");

                //apiErrorAddTsDb(aiCamDeviceRecordID, "folder_create");

                Cdbg(APP_DBG, "Creat aiCamDeviceRecordID Floder fail, waiting 1s, continue \n");
                sleep(1);
                continue;
            }


        }
        
    }
    */

}

void spacePlanDir() {

    char dirname[64];

    // step 2 : process AiCAM(Dir) -> V2 ID(Dir) (ex: -22/AiCAM/V2)
    memset(dirname,0,sizeof(dirname));
    strcpy(dirname, "V2");
    //strcpy(dirname, aicam_device_dir);
    Cdbg(APP_DBG, "check -22/AiCAM/V2 DIR \n");

    while(1) {

        aiCamV2ID = checkDirId(aiCamID, dirname);

        Cdbg(APP_DBG, "aiCamV2ID = %lli\n", aiCamV2ID);

        // ID >= 1000 : get id sucess
        if((aiCamV2ID == DIR_NEED_CREATE) || (aiCamV2ID >= 1000)) {
            break;
        } else {

            int tsdb_status = tsdbProcess(aiCamV2ID, "propfind:DeviceDir find fail");
            //apiErrorAddTsDb(aiCamDeviceID, "propfind");

            Cdbg(APP_DBG, "aiCamV2ID get data fail, waiting 1s, continue \n");
            sleep(1);
            continue;
        }


    }

    Cdbg(APP_DBG, "Get aiCamV2ID ID is %lli\n", aiCamV2ID);


    // not -22/AiCAM/V2 DIR
    if(aiCamV2ID == DIR_NEED_CREATE) {

        memset(dirname,0,sizeof(dirname));
        //strcpy(dirname, "/tmp/AiCAM/AiCAM_215C34");

        strcpy(dirname, asus_conf.uploader_path);
        strcat(dirname, "/AiCAM/");
        // strcat(dirname, asus_conf.mac_name);
        strcat(dirname, "V2");
        
        Cdbg(APP_DBG, "not -22/AiCAM/%s DIR, aiCamV2ID is %lli , Prepare Create AiCAM Device dir : %s\n", "V2", aiCamID, dirname);


        while(1) {

            aiCamV2ID = createDirId(aiCamID, dirname);

            Cdbg(APP_DBG, "aiCamV2ID = %lli\n", aiCamV2ID);

            // ID >= 1000 : get id sucess
            if(aiCamV2ID >= 1000) {
            //if((aiCamV2ID == DIR_NEED_CREATE) || (aiCamV2ID >= 1000)) {
                break;
            } else {

                int tsdb_status = tsdbProcess(aiCamV2ID, "V2 folder create fail");

                //apiErrorAddTsDb(aiCamDeviceID, "folder_create");

                Cdbg(APP_DBG, "Creat V2 Floder fail, waiting 1s, continue \n");
                sleep(1);
                continue;
            }
        }

    }
    
    Cdbg(APP_DBG, "-22/AiCAM/%s/ DIR ID is %lli\n", "V2", aiCamV2ID);




    // step 2-2: process AiCAM(Dir) -> V2 -> Device ID(Dir)(ex:-22/AiCAM/V2/AICAM_000C29)
    memset(dirname,0,sizeof(dirname));
    strcpy(dirname, asus_conf.mac_name);
    //strcpy(dirname, aicam_device_dir);
    Cdbg(APP_DBG, "check -22/AiCAM/V2/AiCAM_XXXXXX DIR \n");


    while(1) {

        aiCamDeviceID = checkDirId(aiCamV2ID, dirname);

        Cdbg(APP_DBG, "aiCamDeviceID = %lli\n", aiCamDeviceID);

        // ID >= 1000 : get id sucess
        if((aiCamDeviceID == DIR_NEED_CREATE) || (aiCamDeviceID >= 1000)) {
            break;
        } else {

            int tsdb_status = tsdbProcess(aiCamDeviceID, "propfind:DeviceDir find fail");
            //apiErrorAddTsDb(aiCamDeviceID, "propfind");

            Cdbg(APP_DBG, "aiCamDeviceID get data fail, waiting 1s, continue \n");
            sleep(1);
            continue;
        }


    }

    Cdbg(APP_DBG, "Get aiCamDeviceID ID is %lli\n", aiCamDeviceID);


    // not -22/AiCAM/AiCAM_215C34 DIR
    if(aiCamDeviceID == DIR_NEED_CREATE) {

        memset(dirname,0,sizeof(dirname));
        //strcpy(dirname, "/tmp/AiCAM/AiCAM_215C34");

        strcpy(dirname, asus_conf.uploader_path);
        strcat(dirname, "/AiCAM/");
        strcat(dirname, "V2/");
        strcat(dirname, asus_conf.mac_name);
        
        Cdbg(APP_DBG, "not -22/AiCAM/V2/%s DIR, aiCamV2ID is %lli , Prepare Create AiCAM Device dir : %s\n", asus_conf.mac_name, aiCamV2ID, dirname);


        while(1) {

            aiCamDeviceID = createDirId(aiCamV2ID, dirname);

            Cdbg(APP_DBG, "aiCamDeviceID = %lli\n", aiCamDeviceID);

            // ID >= 1000 : get id sucess
            if(aiCamDeviceID >= 1000) {
            //if((aiCamDeviceID == DIR_NEED_CREATE) || (aiCamDeviceID >= 1000)) {
                break;
            } else {

                int tsdb_status = tsdbProcess(aiCamDeviceID, "Device folder create fail");

                //apiErrorAddTsDb(aiCamDeviceID, "folder_create");

                Cdbg(APP_DBG, "Creat aiCamDeviceID Floder fail, waiting 1s, continue \n");
                sleep(1);
                continue;
            }
        }

    }
    
    Cdbg(APP_DBG, "-22/AiCAM/V2/%s/ DIR ID is %lli\n", asus_conf.mac_name, aiCamDeviceID);








    // step 3-1 : process AiCAM(Dir) -> Device ID(Dir) -> V2 -> Alarm(Dir) (ex: -22/AiCAM/AiCAM_215C34/Alarm)
    memset(dirname,0,sizeof(dirname));
    strcpy(dirname,"alarm");
    

    Cdbg(APP_DBG, "aiCamDeviceID id : %lli, check 22/AiCAM/V2/%s/%s DIR \n", aiCamDeviceID, asus_conf.mac_name, dirname);

    
    while(1) {


        aiCamDeviceAlarmID = checkDirId(aiCamDeviceID, dirname);

        Cdbg(APP_DBG, "aiCamDeviceAlarmID = %lli\n", aiCamDeviceAlarmID);

        // ID >= 1000 : get id sucess
        if((aiCamDeviceAlarmID == DIR_NEED_CREATE) || (aiCamDeviceAlarmID >= 1000)) {
            break;
        } else {

            int tsdb_status = tsdbProcess(aiCamDeviceAlarmID, "propfind:AlarmDir find fail");

            //apiErrorAddTsDb(aiCamDeviceAlarmID, "propfind");

            Cdbg(APP_DBG, "aiCamDeviceAlarmID get data fail, waiting 1s, continue \n");
            sleep(1);
            continue;
        }

    }
    Cdbg(APP_DBG, "Get aiCamDeviceAlarmID ID is %lli\n", aiCamDeviceAlarmID);


    // not -22/AiCAM/V2/AiCAM_215C34/Alarm DIR
    if(aiCamDeviceAlarmID == DIR_NEED_CREATE) {

        memset(dirname,0,sizeof(dirname));

        strcpy(dirname, asus_conf.uploader_path);
        strcat(dirname, "/AiCAM/");
        strcat(dirname, "V2/");
        strcat(dirname, asus_conf.mac_name);
        strcat(dirname, "/alarm");
        
        Cdbg(APP_DBG, "not -22/AiCAM/V2/%s/alarm DIR, aiCamDeviceID is %lli , Prepare Create AiCAM Device Alarm dir : %s\n", asus_conf.mac_name, aiCamDeviceID, dirname);


        while(1) {

            aiCamDeviceAlarmID = createDirId(aiCamDeviceID, dirname);

            Cdbg(APP_DBG, "aiCamDeviceAlarmID = %lli\n", aiCamDeviceAlarmID);

            // ID >= 1000 : get id sucess
            if(aiCamDeviceAlarmID >= 1000) {
            //if((aiCamDeviceAlarmID == DIR_NEED_CREATE) || (aiCamDeviceAlarmID >= 1000)) {
                break;
            } else {

                int tsdb_status = tsdbProcess(aiCamDeviceAlarmID, "Alarm folder create fail");

                //apiErrorAddTsDb(aiCamDeviceAlarmID, "folder_create");

                Cdbg(APP_DBG, "Creat aiCamDeviceAlarmID Floder fail, waiting 1s, continue \n");
                sleep(1);
                continue;
            }

        }



    }

    //Cdbg(APP_DBG, "-22/AiCAM/%s/alarm DIR ID is %lli\n", asus_conf.mac_name, aiCamDeviceAlarmID);
    Cdbg(APP_DBG, "-22/AiCAM/V2/%s/alarm DIR ID is %lli\n", asus_conf.mac_name, aiCamDeviceAlarmID);


    time_t t = time(0);

    //strftime( tmp, sizeof(tmp), "%Y-%m-%d %X:%A:%S 本年第%j天 %z",localtime(&t) );

    memset(time_now,0, sizeof(time_now));

    strftime( time_now, sizeof(time_now), "%Y%m%d",localtime(&t) );
    Cdbg(APP_DBG, "time_now : %s\n", time_now);

    memset(dirname,0,sizeof(dirname));
    strcpy(dirname, time_now);


    createAlarmDateDir();


    // step 3-4 : process AiCAM(Dir) -> Device ID(Dir) -> aiCamDeviceRecordID(Dir) (ex: -22/AiCAM/AiCAM_215C34/record)

    // createAiCamDeviceRecordDir
    memset(dirname,0,sizeof(dirname));
    //strcpy(dirname, "/tmp/AiCAM/V2/AiCAM_215C34/record");
    strcpy(dirname, asus_conf.uploader_path);
    strcat(dirname, "/AiCAM/");
    strcat(dirname, "V2/");
    strcat(dirname, asus_conf.mac_name);
    strcat(dirname, "/record");
        
    aiCamDeviceRecordID = createWebStorageDir("record", dirname, aiCamDeviceID);

}



//int initSendEmail();

// int initSendEmail() {

//     Cdbg(APP_DBG, "initSendAwsEmail Init\n");

//     int InitSendAwsEmail = initSendAwsEmail(username, &asus_conf);

//     Cdbg(APP_DBG, "InitSendAwsEmail is %d\n",InitSendAwsEmail);

//     return InitSendAwsEmail;
// }
    


// process NewRegisterUser API DESede -> Ex: <provision>09876ZXCVBNM23456FGHJKLRTGH</provision
int initDevice() {

    // encrytp data
    unsigned char dataout[512];
    //memset(dataout, 0, sizeof(dataout));

    int encrytpLen;
    //char datain[] = "<clienttype>AiCAM</clienttype><clientversion>1.0.1.120</clientversion><manufacturer>ASUSTeK Computer INC.</manufacturer><productname>AiCAM</productname><machineid>AiCAM_V03_112288</machineid><uuid>00E018112288</uuid><mac>00E018112288</mac>"; /* 明文 */
    char datain[300];
    
    snprintf(datain,sizeof(datain), "<clienttype>%s</clienttype><clientversion>%s</clientversion><manufacturer>%s</manufacturer><productname>%s</productname><machineid>%s</machineid><uuid>%s</uuid><mac>%s</mac><country>%s</country><channel>%s</channel>", asus_conf.client_type, asus_conf.client_version, asus_conf.manufacturer, asus_conf.product_name, asus_conf.machine_id, asus_conf.uuid, asus_conf.mac, tcode_country, tcode_channel);


    char key[25];
    memset(key, 0, 25);
    strncpy(key, asus_conf.key, 24);


    Cdbg(APP_DBG, "key :%s\n", key);
    Cdbg(APP_DBG, "datain :%s\n", datain);

    des_encrypt(datain, dataout, &encrytpLen, key, strlen(key));

    //unsigned char encytp_dataout[encrytpLen];
    unsigned char* encytp_dataout = (char*)malloc(sizeof(char)*encrytpLen + 1);
    memset(encytp_dataout, 0, sizeof(char)*encrytpLen + 1);



    //memset(encytp_dataout, 0, sizeof(encytp_dataout)+1);
    memcpy(encytp_dataout, dataout, encrytpLen);

    int i;
    if(APP_DBG) {
        printf("\n initDevice -> encrytpLen : %d\n", encrytpLen);
        for (i = 0; i < encrytpLen; i++) {
            printf("%d, ", *(dataout + i));
        }
        printf("\n");
    }
 

    //Encode To Base64
    char* base64EncodeOutput;

    Base64Encode(encytp_dataout, encrytpLen, &base64EncodeOutput);

    Cdbg(APP_DBG, "\nOutput (base64): %d, %s\n",  (int) strlen(base64EncodeOutput), base64EncodeOutput);
    
    Cdbg(APP_DBG, "initDevice ->  initAttachableDevice init start\n");

    // asus webstorage api connect function
    int initAttachableDeviceStatus = initAttachableDevice(username, base64EncodeOutput, &asus_conf);

    Cdbg(APP_DBG, "initDevice ->  initAttachableDeviceStatus = %d\n",initAttachableDeviceStatus);

     
    free(encytp_dataout);
    free(base64EncodeOutput);

    return initAttachableDeviceStatus;
    
}

void browseAttachableDeviceCheck() {

    // browseAttachableDevice API , return type 0 : status , 1: space plan diff
    int browseAttachableDeviceSpacePlanStatus = browseAttachableDevice(username, &ap_conf, 1);

    Cdbg(APP_DBG, "browseAttachableDeviceSpacePlanStatus = %d", browseAttachableDeviceSpacePlanStatus);

    if(browseAttachableDeviceSpacePlanStatus == SPACE_PLAN_DIFF) {

        Cdbg(APP_DBG, "browse Attachable Device Space Plan Diff, device_id : [ %s ]", asus_conf.device_id);
        Cdbg(APP_DBG, "uploader end");
        exit(0);

    } else if(browseAttachableDeviceSpacePlanStatus == DEVICE_EXPIRED) {

        Cdbg(APP_DBG, "device_id : [ %s ] EXPIRED", asus_conf.device_id);
        Cdbg(APP_DBG, "uploader end");
        exit(0);

    } else if((browseAttachableDeviceSpacePlanStatus == S_AUTH_FAIL) || 
              (browseAttachableDeviceSpacePlanStatus == S_ACCOUNT_AUTH_FAIL) )  {

        Cdbg(APP_DBG, "device_id : [ %s ] S_AUTH_FAIL", asus_conf.device_id);
        Cdbg(APP_DBG, "uploader end");
        exit(0);
    }
}

// Check if the directory exists
int checkDirId(long long parentID, char *dirname) {

    Propfind *pfind;

    pfind = checkEntryIdExisted(username, parentID, dirname, "system.folder");

    if(NULL == pfind) {
        Cdbg(APP_DBG, "%s parentID find is null\n", dirname);
        my_free(pfind);
        return DIR_NEED_CREATE;
    }

    if(pfind->status != 0) {

        int status = pfind->status;

        my_free(pfind);

        Cdbg(APP_DBG, "checkDirId : pfind->status find status is %d\n", status);
        if(status == CONNECT_TIMEOUT) {
            Cdbg(APP_DBG, "checkDirId : connect time out\n");
        } else if(status == CURL_CANNOT_RESOLVE_HOSTNAME) {
            Cdbg(APP_DBG, "checkDirId : curl can't resolve host name\n");
        } else if(status == CURLE_SSL_CONNECT_ERROR) {
            Cdbg(APP_DBG, "checkDirId : CURLE_SSL_CONNECT_ERROR\n");
        } else if(status == S_AUTH_FAIL) {

            Cdbg(APP_DBG, "Account auth fail\n");

            int token_status = GetToken("checkDirId OAuth");
            if(token_status == 225) {
                Cdbg(APP_DBG, "status : 225 = access token fail, uploader Over");
                exit(1);
            }

            // browse Attachable Device Check, if space plan change?
            browseAttachableDeviceCheck();

        }


        return status;
    }

    if(strcmp(pfind->type,"system.notfound") == 0) {
        Cdbg(APP_DBG, "dirname = [%s], pfind->type = %s\n", dirname, pfind->type);
        my_free(pfind);
        return 0;
    }      

    parentID = pfind->id;
    my_free(pfind);
    
    return parentID;
    
}


// start : add by markcool (Dir dir check)
int createDirId(long long parentID, char *dirname) {
    
    Createfolder *cf;

    //cf = createFolder(username,parentID,0, "/tmp/AiCAM");

    cf = createFolderId(username, parentID, 0, dirname);


    if(NULL == cf) {

        Cdbg(APP_DBG, "createDirId -> create [ %s ]folder fail\n", dirname);
        
        my_free(cf);
        return -1;

    }

    Cdbg(APP_DBG, "createDirId -> create [ %s ]folder , status = %d,  id = %lli\n", dirname, cf->status, cf->id);

    if(cf->status != 0) {

        int status = cf->status;

        my_free(cf);

        Cdbg(APP_DBG, "createDirId : cf->status find status is %d\n", status);
        if(status == CONNECT_TIMEOUT) {
            Cdbg(APP_DBG, "createDirId : connect time out\n");
        } else if(status == CURL_CANNOT_RESOLVE_HOSTNAME) {
            Cdbg(APP_DBG, "createDirId : curl can't resolve host name\n");
        } else if(status == CURLE_SSL_CONNECT_ERROR) {
            Cdbg(APP_DBG, "createDirId : CURLE_SSL_CONNECT_ERROR\n");
        } else if(status == S_AUTH_FAIL) {
            Cdbg(APP_DBG, "Account auth fail\n");
            int token_status = GetToken("createDirId OAuth");
            if(token_status == 225) {
                Cdbg(APP_DBG, "status : 225 = access token fail, uploader Over");
                exit(1);
            }

            // browse Attachable Device Check, if space plan change?
            browseAttachableDeviceCheck();

        }

        return status;
    }


    long long folderId = cf->id;
    
    my_free(cf);
    
    return folderId;
}

// markcool add 



void processTsDBUpload(char * filename, char * tsdb_type, int return_status) {

/*
    Finishbinaryupload *fbu = NULL;

    fbu = getb(Finishbinaryupload);

    if( NULL == fbu ) {
        Cdbg(APP_DBG, "create memery error\n");
    }

    memset(fbu,0,sizeof(Finishbinaryupload));

    if(parseDoc1(finish_upload_xml,fbu) == -1) {
        my_free(fbu);
        return;
    }


    char s_fileid[32], s_status[32];

    sprintf(s_fileid,"%lli",fbu->fileid);
    sprintf(s_status,"%d",fbu->status);

    Cdbg(APP_DBG, "ibu->fileid : %s\n", s_fileid);
    Cdbg(APP_DBG, "ibu->status : %s\n", s_status);
*/

    char s_value2[32], s_status[32];

    sprintf(s_value2,"%d", 0);
    sprintf(s_status,"%d",return_status);

    memset(device_id,0, sizeof(device_id));
    strcpy(device_id, asus_conf.device_id);


    time_t t = time(0);
    char time_new[26];
    memset(time_new,0, sizeof(time_new));

    //strftime( tmp, sizeof(tmp), "%Y%m%d %X %A :%A:%s本年第%j天 %z",localtime(&t) );
    strftime( time_new, sizeof(time_new), "%Y-%m-%d %X",localtime(&t) );



    memset(ac_tsdb_type,0, sizeof(ac_tsdb_type));
    strcpy(ac_tsdb_type, tsdb_type);

    memset(type_value1,0, sizeof(type_value1));
    strcpy(type_value1, filename);  // filename or even type

    memset(type_value2,0, sizeof(type_value2));
    strcpy(type_value2, s_value2); // RecordAndAlarm file id

    memset(type_value3,0, sizeof(type_value3));
    strcpy(type_value3, s_status); // return code of direct upload api


    int return_value = 0;
    return_value = apiTsdbInputNoId(&asus_conf, time_new, &ac_tsdb_type, device_id, "AiCAM_2.0.1", &type_value1, &type_value2, &type_value3);

    if(return_value == 0) {
        snprintf(dbg_msg, sizeof(dbg_msg), "tsdb status upload success, time_new : %s\n", time_new);
        // debugLogLevel(DEBUG_LEVEL, DBG_INFO, dbg_msg);
        Cdbg(APP_DBG, "%s", dbg_msg);

    } else {
        snprintf(dbg_msg, sizeof(dbg_msg), "tsdb status fail return vlaue = %d, time_new : %s\n", return_value, time_new);
        // debugLogLevel(DEBUG_LEVEL, DBG_INFO, dbg_msg);
        Cdbg(APP_DBG, "%s", dbg_msg);
    }

 //   my_free(fbu);
/*
    if( remove(finish_upload_xml) != 0 ) {
        Cdbg(APP_DBG, "Error deleting file : %s\n",finish_upload_xml);
    } else {
        Cdbg(APP_DBG, "File success deleted : %s\n",finish_upload_xml);
    }
*/
}


void dirFileUpload(char *filename, char * dirType, int file_type, char * mac_name, int mac_type) {

    // modify by markcool : start
    char path_file_name[128];
    int status;

    memset(path_file_name, 0, sizeof(path_file_name));

    strcpy(path_file_name, asus_conf.uploader_path);
    strcat (path_file_name, "mac/");
    strcat (path_file_name, mac_name);
    strcat (path_file_name, "/");
    strcat (path_file_name, dirType);
    strcat (path_file_name, "/");
    strcat (path_file_name, filename);

    snprintf(dbg_msg, sizeof(dbg_msg), "dirFileUpload -> mac_type -> %d, path_file_name =  %s", mac_type, path_file_name);
    // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
    Cdbg(APP_DBG, "%s", dbg_msg);

    // upload config file,  (to webstorage date)
    //status = uploadFile(path_file_name,MySyncFolder,NULL,0);
    //status = uploadFile(path_file_name,aiCamDeviceVideoID,NULL,0);

    // need mark
    //status = uploadFile(path_file_name,aiCamDeviceID,NULL,0);


    if(strcmp(dirType, "record") == 0) {
        


        if(mac_type == MAC_ONE) {

            status = upload_file(path_file_name, aiCamMacAddressOneRecordDateID,NULL,0);
            

        } else if(mac_type == MAC_TWO) {

            status = upload_file(path_file_name, aiCamMacAddressTwoRecordDateID,NULL,0);

        } else if(mac_type == MAC_THREE) {

            status = upload_file(path_file_name, aiCamMacAddressThreeRecordDateID,NULL,0);

        } else if(mac_type == MAC_FOUR) {

            status = upload_file(path_file_name, aiCamMacAddressFourRecordDateID,NULL,0);

        }

        // // 0 : WebStorage
        // if(upload_type == RECORD_ASUS_ALARM_ASUS) {

        //     // Cdbg(APP_DBG, "dirFileUpload ->  WebStorage aiCamDeviceRecordDateID = %lli\n",aiCamDeviceRecordDateID);
        //     //status = uploadFile(path_file_name,aiCamDeviceRecordDateID,NULL,0);

        //     status = upload_file(path_file_name,aiCamDeviceRecordDateID,NULL,0);

        // // 1 : sd mode
        // } else if(upload_type == RECORD_SD_ALARM_ASUS) {

        //     Cdbg(APP_DBG, "SD Mode : not upload mp4 file.\n");            

        //     status = 0;

        // // 2 : upload file to [aiCloud]
        // } else if(upload_type == RECORD_AICLOUD_ALARM_ASUS) {

        //     Cdbg(APP_DBG, "aiCloud upload: mp4 file start.\n");

        //     status = uploadFileToAiCloud(path_file_name, filename);

        // }


    } else if(strcmp(dirType, "alarm") == 0) {
        

        if(mac_type == MAC_ONE) {

            status = upload_file(path_file_name, aiCamMacAddressOneAlarmDateID,NULL,0);

        } else if(mac_type == MAC_TWO) {

            status = upload_file(path_file_name, aiCamMacAddressTwoAlarmDateID,NULL,0);

        } else if(mac_type == MAC_THREE) {

            status = upload_file(path_file_name, aiCamMacAddressThreeAlarmDateID,NULL,0);

        } else if(mac_type == MAC_FOUR) {

            status = upload_file(path_file_name, aiCamMacAddressFourAlarmDateID,NULL,0);

        }
        


    } else {
        snprintf(dbg_msg, sizeof(dbg_msg), "uploadFile type Error : %s\n", dirType);
        // debugLogLevel(DEBUG_LEVEL, DBG_ERROR, dbg_msg);
        Cdbg(APP_DBG, "%s", dbg_msg);
    }
    
    snprintf(dbg_msg, sizeof(dbg_msg), "FileUpload status is %d, upload_type_status is %d, type is %s\n",status, upload_type, dirType);
    // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
    Cdbg(APP_DBG, "%s", dbg_msg);

    // 0 : upload success
    if(status == 0) {

        if( remove(path_file_name) != 0 ) {
            snprintf(dbg_msg, sizeof(dbg_msg), "file [%s] upload success, Error deleting file\n",path_file_name);
            // debugLogLevel(DEBUG_LEVEL, DBG_INFO, dbg_msg);
            Cdbg(APP_DBG, "%s", dbg_msg);
        } else {
            snprintf(dbg_msg, sizeof(dbg_msg), "file [%s] upload success, File success deleted\n",path_file_name);
            // debugLogLevel(DEBUG_LEVEL, DBG_INFO, dbg_msg);
            Cdbg(APP_DBG, "%s", dbg_msg);
        }

        // debugOffOn : on -> upload to tsdb
        // if(system_debug_status) {

        //     char tsdb_msg[32];
        //     memset(tsdb_msg, 0, sizeof(tsdb_msg));

        //     //Cdbg(APP_DBG, "%s upload finish, status : %d", dirType, status);
        //     if(file_type == 1) {  // 1 = mp4 , 2 = od
        //         sprintf(tsdb_msg,"%s mp4 upload OK,status:%d", dirType, status);
        //     } else {
        //         sprintf(tsdb_msg,"%s od upload OK,status:%d", dirType, status);    
        //     }
            

        //     processTsDBUpload(tsdb_msg, "api", status);
        // }


    } else {

        if( remove(path_file_name) != 0 ) {

            // Cdbg(APP_DBG, "upload fail: %s, Error deleting file\n",path_file_name);
            
            snprintf(dbg_msg, sizeof(dbg_msg), "upload fail: %s, Error deleting file\n",path_file_name);
            // debugLogLevel(DEBUG_LEVEL, DBG_ERROR, dbg_msg);
            Cdbg(APP_DBG, "%s", dbg_msg);

        } else {

            // Cdbg(APP_DBG, "upload fail: %s, File success deleted\n",path_file_name);

            snprintf(dbg_msg, sizeof(dbg_msg), "upload fail: %s, File success deleted\n",path_file_name);
            // debugLogLevel(DEBUG_LEVEL, DBG_ERROR, dbg_msg);
            Cdbg(APP_DBG, "%s", dbg_msg);

        }

        if((status == CURLE_SSL_CONNECT_ERROR) || 
           (status == CURL_CANNOT_RESOLVE_HOSTNAME) || 
           (status == CONNECT_TIMEOUT)) {

            apiErrorAddTsDb(status, "network connect fail, upload_file_fail");


        } else if(status == S_AUTH_FAIL) {

            apiErrorAddTsDb(status, "Account auth fail");

            int token_status = GetToken("dirFileUpload OAuth");

            if(token_status == 225) {

                Cdbg(APP_DBG, "status : 225 = access token fail, uploader Over");
                apiErrorAddTsDb(token_status, "Get token fail,uploader exit");
                exit(1);
            } else {
                Cdbg(APP_DBG, "status : 225 = access token fail, uploader Over");
                apiErrorAddTsDb(token_status, "Upload S_AUTH_FAIL, exit");
                exit(1);
            }

            // browse Attachable Device Check, if space plan change?
            browseAttachableDeviceCheck();

        } else {


            char error_msg[32];
            memset(error_msg, 0, sizeof(error_msg));

            if(file_type == 1) {  // 1 = mp4 , 2 = od
                sprintf(error_msg,"mp4 upload fail,status:%d", status);
            } else {
                sprintf(error_msg,"od upload fail,status:%d", status);    
            }


            processTsDBUpload(error_msg, "api", status);

        }

        // webstorage no space, del webstorage space
        if((status == S_USER_NOSPACE) || (status == EXCEED_GROUPAWARE_SPACE)) {

            Cdbg(APP_DBG, "User webstorage no space, start del space");

            // old del function, old structure
            //delExpiredSpaceDate();

            // new del function, new structure
            delSpaceInsufficient();

            Cdbg(APP_DBG, "End webstorage del space");
        }

    }

}

int googleDriveFolderSearch(char * foldername, char * folderid) {

    int status = 0;

    status = google_folder_search(foldername, folderid);

    // 0 : upload success
    if(status == 0) {

    } else if(status == -1) {

    } else {

        if((status == CURLE_SSL_CONNECT_ERROR) || 
           (status == CURL_CANNOT_RESOLVE_HOSTNAME) || 
           (status == CONNECT_TIMEOUT)) {

            //apiErrorAddTsDb(status, "network connect fail, upload_file_fail");


        } else if(status == S_AUTH_FAIL) {


        } else {

        }

        // webstorage no space, del webstorage space
        if((status == S_USER_NOSPACE) || (status == EXCEED_GROUPAWARE_SPACE)) {


        }

    }

    return status;

}



int googleDriveFileSearch(char * filename, char * folderid) {

    int status = 0;

    status = google_file_search(filename, folderid);

    // 0 : upload success
    if(status == 0) {

    } else if(status == -1) {

    } else {

        if((status == CURLE_SSL_CONNECT_ERROR) || 
           (status == CURL_CANNOT_RESOLVE_HOSTNAME) || 
           (status == CONNECT_TIMEOUT)) {

            //apiErrorAddTsDb(status, "network connect fail, upload_file_fail");


        } else if(status == S_AUTH_FAIL) {


        } else {

        }

        // webstorage no space, del webstorage space
        if((status == S_USER_NOSPACE) || (status == EXCEED_GROUPAWARE_SPACE)) {


        }

    }

    return status;

}


void sqlFileUpload(char *filename, int file_type) {

    // modify by markcool : start
    char path_file_name[128];
    int status;

    memset(path_file_name, 0, sizeof(path_file_name));

    // strcpy(path_file_name, "/tmp/router/");
    // strcat (path_file_name, "log/");

    strcpy(path_file_name, config_upload_path);
    strcat (path_file_name, "/");
    strcat (path_file_name, filename);

    Cdbg(APP_DBG, "sqlFileUpload -> path_file_name =  %s", path_file_name);



    do {

        status = google_drive_file_create(path_file_name);

        if(status != 0) {
            sleep(20);
        }

    } while ( status != 0);


    Cdbg(APP_DBG, "google_d_conf.file_id is %s", google_d_conf.file_id);


    
    do {

        status = upload_file_to_google_drive(path_file_name);

        if(status != 0) {
            sleep(20);
        }

    } while ( status != 0);
    
    
    // status = google_drive_upload(path_file_name);

    Cdbg(APP_DBG, "FileUpload status is %d",status);

    // 0 : upload success
    if(status == 0) {

        if( remove(path_file_name) != 0 ) {
            Cdbg(APP_DBG, "file [%s] upload success, Error deleting file\n",path_file_name);
        } else {
            Cdbg(APP_DBG, "file [%s] upload success, File success deleted\n",path_file_name);
        }

        // debugOffOn : on -> upload to tsdb
        // if(system_debug_status) {

        //     char tsdb_msg[32];
        //     memset(tsdb_msg, 0, sizeof(tsdb_msg));

        //     //Cdbg(APP_DBG, "%s upload finish, status : %d", dirType, status);
        //     if(file_type == 1) {  // 1 = mp4 , 2 = od
        //         sprintf(tsdb_msg,"%s mp4 upload OK,status:%d", dirType, status);
        //     } else {
        //         sprintf(tsdb_msg,"%s od upload OK,status:%d", dirType, status);    
        //     }
            

        //     processTsDBUpload(tsdb_msg, "api", status);
        // }


    } else {



        if((status == CURLE_SSL_CONNECT_ERROR) || 
           (status == CURL_CANNOT_RESOLVE_HOSTNAME) || 
           (status == CONNECT_TIMEOUT)) {

             Cdbg(APP_DBG, "network connect fail, upload file fail, Waiting for reupload");

            //apiErrorAddTsDb(status, "network connect fail, upload_file_fail");


        } else if(status == S_AUTH_FAIL) {

            // apiErrorAddTsDb(status, "Account auth fail");

            // int token_status = GetToken("dirFileUpload OAuth");

            // if(token_status == 225) {

            //     Cdbg(APP_DBG, "status : 225 = access token fail, uploader Over");
            //     apiErrorAddTsDb(token_status, "Get token fail,uploader exit");
            //     exit(1);
            // } else {
            //     Cdbg(APP_DBG, "status : 225 = access token fail, uploader Over");
            //     apiErrorAddTsDb(token_status, "Upload S_AUTH_FAIL, exit");
            //     exit(1);
            // }

            // // browse Attachable Device Check, if space plan change?
            // browseAttachableDeviceCheck();

        } else {


            // if( remove(path_file_name) != 0 ) {

            //      Cdbg(APP_DBG, "upload fail: %s, Error deleting file\n",path_file_name);
                
            // } else {

            //     Cdbg(APP_DBG, "upload fail: %s, File success deleted\n",path_file_name);

            // }

            // char error_msg[32];
            // memset(error_msg, 0, sizeof(error_msg));

            // if(file_type == 1) {  // 1 = mp4 , 2 = od
            //     sprintf(error_msg,"mp4 upload fail,status:%d", status);
            // } else {
            //     sprintf(error_msg,"od upload fail,status:%d", status);    
            // }


            // processTsDBUpload(error_msg, "api", status);

        }

        // webstorage no space, del webstorage space
        if((status == S_USER_NOSPACE) || (status == EXCEED_GROUPAWARE_SPACE)) {

            // Cdbg(APP_DBG, "User webstorage no space, start del space");

            // // old del function, old structure
            // //delExpiredSpaceDate();

            // // new del function, new structure
            // delSpaceInsufficient();

            // Cdbg(APP_DBG, "End webstorage del space");

        }

    }

}


int sqlFileDownload(char *filename, int file_type) {

    // modify by markcool : start
    char path_file_name[128];
    int status;

    memset(path_file_name, 0, sizeof(path_file_name));

    // strcpy(path_file_name, "/tmp/router/");
    // strcat (path_file_name, "download/");

    strcpy(path_file_name, config_download_path);
    strcat (path_file_name, "/");
    strcat (path_file_name, filename);

    Cdbg(APP_DBG, "sqlFileDownload -> path_file_name =  %s", path_file_name);


    // del : download file link
    if( remove(path_file_name) != 0 ) {
        Cdbg(APP_DBG, "link download file [%s] , Error deleting file",path_file_name);
    } else {
        Cdbg(APP_DBG, "link download file [%s] , File success deleted",path_file_name);
    }

    // download drive file
    status = google_drive_download(path_file_name);

    Cdbg(APP_DBG, "FileDownload status is %d",status);

    // 0 : upload success
    if(status == 0) {

        Cdbg(APP_DBG, "download file [%s] success\n",path_file_name);

    } else {


        Cdbg(APP_DBG, "download fail: %s, status = %d",path_file_name, status);

        if((status == CURLE_SSL_CONNECT_ERROR) || 
           (status == CURL_CANNOT_RESOLVE_HOSTNAME) || 
           (status == CONNECT_TIMEOUT)) {

            //apiErrorAddTsDb(status, "network connect fail, upload_file_fail");


        } else if(status == S_AUTH_FAIL) {


        } else {


        }

        // webstorage no space, del webstorage space
        if((status == S_USER_NOSPACE) || (status == EXCEED_GROUPAWARE_SPACE)) {


        }

    }

    return status;


}


// webstorage date dir check, if date dir not exist -> create
void timecmp(char * filename, char * dirType) {

    char time_new[10];

    memset(time_new,0, sizeof(time_new));

    substr(time_new, filename, 0, 8);

    //strftime( tmp, sizeof(tmp), "%Y%m%d %X %A 本年第%j天 %z",localtime(&t) );
    // time_t t = time(0);
    // strftime( time_new, sizeof(time_new), "%Y%m%d",localtime(&t) );

//    Cdbg(APP_DBG, "timecmp : time_new : %s\n", time_new);

    // date check
    if(strcmp (time_new,time_now) == 0) {

//        Cdbg(APP_DBG, "timecmp : same date, not create new date.\n");

    } else {

        snprintf(dbg_msg, sizeof(dbg_msg), "time_now = %s ,  : time_new : %s", time_now, time_new);
        // debugLogLevel(DEBUG_LEVEL, DBG_INFO, dbg_msg);
        Cdbg(APP_DBG, "%s", dbg_msg);

        snprintf(dbg_msg, sizeof(dbg_msg), "timecmp : %s Dir different date, create new date.", dirType);
        // debugLogLevel(DEBUG_LEVEL, DBG_INFO, dbg_msg);
        Cdbg(APP_DBG, "%s", dbg_msg);

        memset(time_now,0, sizeof(time_now));

        strcpy(time_now, time_new);


        snprintf(dbg_msg, sizeof(dbg_msg), "space_plan == %d || upload_type == %d", space_plan, upload_type);
        // debugLogLevel(DEBUG_LEVEL, DBG_INFO, dbg_msg);
        Cdbg(APP_DBG, "%s", dbg_msg);

        if( (space_plan == TIME_CONTINOUS_PLAN) || 
            (upload_type == RECORD_AICLOUD_ALARM_ASUS) ) {

            // create new alarm date dir
            createWebStorageAlarmDateDir();

            // create new record date dir
            createWebStorageRecordDateDir();


        } else {

            if(strcmp(dirType, "alarm") == 0) {

                // create new alarm date dir
                createWebStorageAlarmDateDir();


            } else if(strcmp(dirType, "record") == 0) {

                // create new alarm date dir
                createWebStorageAlarmDateDir();

            
               // create new record date dir
               createWebStorageRecordDateDir();

            } else {

                snprintf(dbg_msg, sizeof(dbg_msg), "DirType error: %s", dirType);
                // debugLogLevel(DEBUG_LEVEL, DBG_INFO, dbg_msg);
                Cdbg(APP_DBG, "%s", dbg_msg);

            }

        }

    }
}



// webstorage date dir check, if date dir not exist -> create
void datecmp(char * filename, char * dirType, char * mac_name, int mac_type) {

    char time_new[10];

    memset(time_new,0, sizeof(time_new));

    substr(time_new, filename, 0, 8);

    // date check
    if(strcmp (time_new,time_now) == 0) {

//        Cdbg(APP_DBG, "timecmp : same date, not create new date.\n");

    } else {

        snprintf(dbg_msg, sizeof(dbg_msg), "time_now = %s ,  : time_new : %s", time_now, time_new);
        // debugLogLevel(DEBUG_LEVEL, DBG_INFO, dbg_msg);
        Cdbg(APP_DBG, "%s", dbg_msg);

        snprintf(dbg_msg, sizeof(dbg_msg), "datecmp : %s Dir different date, create new date.", dirType);
        // debugLogLevel(DEBUG_LEVEL, DBG_INFO, dbg_msg);
        Cdbg(APP_DBG, "%s", dbg_msg);

        memset(time_now,0, sizeof(time_now));

        strcpy(time_now, time_new);


        snprintf(dbg_msg, sizeof(dbg_msg), "space_plan == %d || upload_type == %d", space_plan, upload_type);
        // debugLogLevel(DEBUG_LEVEL, DBG_INFO, dbg_msg);
        Cdbg(APP_DBG, "%s", dbg_msg);

        if( (space_plan == TIME_CONTINOUS_PLAN) || 
            (upload_type == RECORD_AICLOUD_ALARM_ASUS) ) {

            // create new alarm date dir
            createWebStorageMacAlarmDateDir(mac_name, mac_type);

            // create new record date dir
            // createWebStorageRecordDateDir();


        } else {

            if(strcmp(dirType, "alarm") == 0) {

                // create new alarm date dir

                createWebStorageMacAlarmDateDir(mac_name, mac_type);



            } else if(strcmp(dirType, "record") == 0) {

                // create new alarm date dir
                // createWebStorageAlarmDateDir();

            
               // create new record date dir
               // createWebStorageRecordDateDir();

            } else {

                // snprintf(dbg_msg, sizeof(dbg_msg), "DirType error: %s", dirType);
                // debugLogLevel(DEBUG_LEVEL, DBG_INFO, dbg_msg);
                Cdbg(APP_DBG, "DirType error: %s", dirType);

            }

        }

    }
}



// webstorage date dir check, if date dir not exist -> create
void driveDatecmp() {

    char time_new[10];

    time_t t = time(0);

    //strftime( tmp, sizeof(tmp), "%Y-%m-%d %X:%A:%S 本年第%j天 %z",localtime(&t) );

    memset(time_new,0, sizeof(time_new));

    strftime( time_new, sizeof(time_new), "%Y%m%d",localtime(&t) );
    

    // date check
    if(strcmp (time_new, time_now) == 0) {

       Cdbg(APP_DBG, "drive date cmp : same date = %s, not create new date.", time_now);

    } else {

        Cdbg(APP_DBG, "old time = %s ,  new time : %s , different date, create new date.", time_now, time_new);

        memset(time_now,0, sizeof(time_now));

        strcpy(time_now, time_new);

        Cdbg(APP_DBG, "Yesterday date_folder_id : %s", google_d_conf.date_folder_id);

        googleDriveFolderSearch(time_now, google_d_conf.log_folder_id);

        Cdbg(APP_DBG, "Today date_folder_id : %s", google_d_conf.date_folder_id);

    }
}


int uploadFileToAiCloud(char * path_file_name, char * filename) {


    memset(&router_info,0,sizeof(RouterInfo));

    getRouterInfo(&router_info, &ai_conf);

    Cdbg(APP_DBG, "router_info.servertime : %s\n", router_info.servertime);
    Cdbg(APP_DBG, "router_info.mac : %s\n", router_info.mac);
    Cdbg(APP_DBG, "router_info.version : %s\n", router_info.version);
    Cdbg(APP_DBG, "router_info.aicloud_version : %s\n", router_info.aicloud_version);
    Cdbg(APP_DBG, "router_info.aicloud_app_type : %s\n", router_info.aicloud_app_type);
    Cdbg(APP_DBG, "router_info.modalname : %s\n", router_info.modalname);
    Cdbg(APP_DBG, "router_info.computername : %s\n", router_info.computername);
    Cdbg(APP_DBG, "router_info.usbdiskname : %s\n", router_info.usbdiskname);
    Cdbg(APP_DBG, "router_info.DiskName : %s\n", router_info.DiskName);
    Cdbg(APP_DBG, "router_info.DiskUsed : %s\n", router_info.DiskUsed);
    Cdbg(APP_DBG, "router_info.DiskAvailable : %s\n", router_info.DiskAvailable);
    Cdbg(APP_DBG, "router_info.DiskUsedPercent : %s\n", router_info.DiskUsedPercent);

    int diskused_len = strlen(router_info.DiskUsed);
    int disk_available_len = strlen(router_info.DiskAvailable);
    int diskused_percent_len = strlen(router_info.DiskUsedPercent);

    Cdbg(APP_DBG, "diskused_len : %d\n", diskused_len);
    Cdbg(APP_DBG, "disk_available_len : %d\n", disk_available_len);
    Cdbg(APP_DBG, "diskused_percent_len : %d\n", diskused_percent_len);


    if((diskused_len > 0) && (disk_available_len > 0) && (diskused_percent_len > 0)) {

        if(disk_available_len > 4) {

            time_t t = time(0);
            char time_new[12];

            memset(time_new,0, sizeof(time_new));

            strftime( time_new, sizeof(time_new), "%Y%m%d",localtime(&t) );

            Cdbg(APP_DBG, "uploadFileToAiCloud time_new : %s\n", time_new);


            char api_url[128]; 

            memset(api_url,0, sizeof(api_url));
            strcpy(api_url, "https://");
            strcat(api_url, ai_conf.privateip);
            strcat(api_url, "/");
            strcat(api_url, router_info.modalname);
            strcat(api_url, "/");
            strcat(api_url, router_info.DiskName);
            strcat(api_url, "/aicam_video/");
            strcat(api_url, asus_conf.device_id);
            strcat(api_url, "/");
            strcat(api_url, time_new);
            strcat(api_url, "/");
            strcat(api_url, filename);

            Cdbg(APP_DBG, "api_url : %s\n", api_url);
            Cdbg(APP_DBG, "path_file_name : %s\n", path_file_name);

            httpPut(api_url, path_file_name, &ai_conf);


        }
        

    } else {

        Cdbg(APP_DBG, "not router_usb \n");

    }
    


    return 0;

}

    
int file_size_check(char * file_path, char * file_name) {


    char file_dir_name[128];;
    memset(file_dir_name,0,sizeof(file_dir_name));

    strcpy(file_dir_name, file_path);
    strcat(file_dir_name, file_name);

    struct stat st;

    stat(file_dir_name, &st);

    int size = st.st_size;

    if(size == 0) {

        if( remove(file_dir_name) != 0 ) {
            // Cdbg(APP_DBG, "file size = 0, Error deleting file : %s\n",file_dir_name);
            
            snprintf(dbg_msg, sizeof(dbg_msg), "file size = 0, Error deleting file : %s\n",file_dir_name);
            // debugLogLevel(DEBUG_LEVEL, DBG_INFO, dbg_msg);
            Cdbg(APP_DBG, "%s", dbg_msg);


        } else {
            // Cdbg(APP_DBG, "file size = 0, File success deleted : %s\n",file_dir_name);

            snprintf(dbg_msg, sizeof(dbg_msg), "file size = 0, File success deleted : %s\n",file_dir_name);
            // debugLogLevel(DEBUG_LEVEL, DBG_INFO, dbg_msg);
            Cdbg(APP_DBG, "%s", dbg_msg);

        }

        return -1;

    } else {

        snprintf(dbg_msg, sizeof(dbg_msg), "[ %s ] size = %d", file_dir_name, size);
        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(APP_DBG, "%s", dbg_msg);

        return 0;
    }
}




// DBG_VERBOSE = 5 ,  DBG_DEBUG = 4; DBG_INFO = 3; DBG_WARN = 2; DBG_ERROR = 1; DBG_NO = 0;

// search mp4, upload mp4,  (type=alarm  *.od)
// void video_upload(char * dirType)
// {

//     int fileNumber = 0;
//     DIR *dp; 
//     struct dirent *dirp;

//     char * pch;
//     char uploadDir[36];

//     memset(uploadDir,0,sizeof(uploadDir));

//     strcpy(uploadDir, asus_conf.uploader_path);
//     strcat(uploadDir, "/aicam/");
//     strcat(uploadDir, dirType);
//     strcat(uploadDir, "/");

//     my_mkdir(uploadDir);

//     // Cdbg(APP_DBG, "Upload type : %s , dir : %s", dirType, uploadDir);    

//     if (dp = opendir(uploadDir)) { 

//         while (dirp = readdir(dp)) {

//             pch = strstr (dirp->d_name, ".mp4");

//             // Cdbg(APP_DBG, "pch od : %s\n", strstr (dirp->d_name, ".od"));
//             // Cdbg(APP_DBG, "pch mp4 : %s\n", strstr (dirp->d_name, ".mp4"));

//             //if (pch != NULL) {

//             if (strstr (dirp->d_name, ".mp4")  || strstr (dirp->d_name, ".od")) {

//                 int file_type = 0;  // 1 = mp4 , 2 = od

//                 if (strstr (dirp->d_name, ".mp4")) {

//                     snprintf(dbg_msg, sizeof(dbg_msg), "process 'mp4' file name : %s\n", dirp->d_name);
//                     debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);

//                     file_type = 1;

//                 } else {

//                     snprintf(dbg_msg, sizeof(dbg_msg), "process 'od' file name : %s\n", dirp->d_name);
//                     debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);

//                     file_type = 2;
//                 }
//                 // Cdbg(APP_DBG, "process 'mp4' or 'od' file name : %s\n", dirp->d_name);


//                 // file size = 0, continue
//                 if(file_size_check(uploadDir, dirp->d_name) == -1) {
//                     continue;
//                 }


//                 //if(strcmp (pch,".mp4") == 0) {
//                 fileNumber++;   

//                 // check date, if different, create new date , upload file to new date
//                 timecmp(dirp->d_name, dirType);

//                 // upload file
//                 dirFileUpload(dirp->d_name, dirType, file_type);


//                 // alarm : need send mail
//                 if(strcmp(dirType, "alarm") == 0) {


//                     if(strcmp(sp_conf.alarmEmailNotificaiton, "on") == 0) {

//                         // send mail 
//                         int init_sendemail_status;

//                         snprintf(dbg_msg, sizeof(dbg_msg), "initSendAwsEmail Init\n");
//                         debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);


//                         init_sendemail_status = initSendAwsEmail(username, &asus_conf);
                    
//                         snprintf(dbg_msg, sizeof(dbg_msg), "InitSendAwsEmail end, status = %d\n",init_sendemail_status);
//                         debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
//                         //init_sendemail_status = initSendEmail();

//                         //Cdbg(APP_DBG, "init_sendemail_status is %d , [1-> succrss]\n", init_sendemail_status);
                        
//                     }

//                 }

//             } else {
//                 //Cdbg(APP_DBG, "ignore mp4 file name : %s\n", dirp->d_name);  
//             }

//             continue;
//             //puts(dirp->d_name);
//             //Cdbg(APP_DBG, "file is : %s\n", dirp->d_name);

//         }
//     } 

//     closedir(dp);

//     //Cdbg(APP_DBG, "Finish upload, Dir /tmp/aicam/%s/ , Upload mp4 file number -> %d", dirType, fileNumber);

// }

// dirType 'alarm' or 'record', mac_type
void search_video(char * dirType, char * mac_name, int mac_type) {

    int fileNumber = 0;
    DIR *dp; 
    struct dirent *dirp;

    char * pch;
    char uploadDir[64];

    memset(uploadDir,0,sizeof(uploadDir));

    strcpy(uploadDir, asus_conf.uploader_path);
    strcat(uploadDir, "mac/");
    strcat(uploadDir, mac_name);
    strcat(uploadDir, "/");
    strcat(uploadDir, dirType);
    strcat(uploadDir, "/");

    my_mkdir(uploadDir);

    Cdbg(APP_DBG, "Upload Video Mode : %s , dir : %s", dirType, uploadDir);    

    if (dp = opendir(uploadDir)) { 

        while (dirp = readdir(dp)) {

            pch = strstr (dirp->d_name, ".mp4");

            // Cdbg(APP_DBG, "pch od : %s\n", strstr (dirp->d_name, ".od"));
            // Cdbg(APP_DBG, "pch mp4 : %s\n", strstr (dirp->d_name, ".mp4"));

            //if (pch != NULL) {

            if (strstr (dirp->d_name, ".ts")  || strstr (dirp->d_name, ".mp4")  || strstr (dirp->d_name, ".od")) {

                int file_type = 0;  // 1 = ts or mp4 , 2 = od

                if (strstr (dirp->d_name, ".ts")  || strstr (dirp->d_name, ".mp4")) {

                    snprintf(dbg_msg, sizeof(dbg_msg), "process 'ts or mp4' file name : %s\n", dirp->d_name);
                    // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
                    Cdbg(APP_DBG, "%s", dbg_msg);

                    file_type = 1;

                } else {

                    snprintf(dbg_msg, sizeof(dbg_msg), "process 'od' file name : %s\n", dirp->d_name);
                    // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
                    Cdbg(APP_DBG, "%s", dbg_msg);

                    file_type = 2;
                }
                // Cdbg(APP_DBG, "process 'mp4' or 'od' file name : %s\n", dirp->d_name);


                // file size = 0, continue
                if(file_size_check(uploadDir, dirp->d_name) == -1) {
                    continue;
                }


                //if(strcmp (pch,".mp4") == 0) {
                fileNumber++;   

                // check date, if different, create new date , upload file to new date
                //datecmp(dirp->d_name, dirType, mac_name, mac_type);


                // upload file
                dirFileUpload(dirp->d_name, dirType, file_type, mac_name, mac_type);


                // alarm : need send mail
                // if(strcmp(dirType, "alarm") == 0) {


                //     if(strcmp(sp_conf.alarmEmailNotificaiton, "on") == 0) {

                //         // send mail 
                //         int init_sendemail_status;

                //         snprintf(dbg_msg, sizeof(dbg_msg), "initSendAwsEmail Init\n");
                //         debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);


                //         init_sendemail_status = initSendAwsEmail(username, &asus_conf);
                    
                //         snprintf(dbg_msg, sizeof(dbg_msg), "InitSendAwsEmail end, status = %d\n",init_sendemail_status);
                //         debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);

                //     }

                // }

            } else {
                //Cdbg(APP_DBG, "ignore mp4 file name : %s\n", dirp->d_name);  
            }

            continue;
            //puts(dirp->d_name);
            //Cdbg(APP_DBG, "file is : %s\n", dirp->d_name);

        }
    } 

    closedir(dp);

    // Cdbg(APP_DBG, "Finish upload, Dir /tmp/aicam/%s/%s/ , Upload mp4 file number -> %d", dirType, fileNumber);

}




// dirType 'alarm' or 'record', mac_type
void search_sqlite_upload() {

    int status = 0;
    int fileNumber = 0;
    DIR *dp; 
    struct dirent *dirp;

    char uploadDir[128];

    memset(uploadDir,0,sizeof(uploadDir));

    strcpy(uploadDir, config_upload_path);
    strcat(uploadDir, "/");

    // strcpy(uploadDir, "/tmp/router/");

    // my_mkdir(uploadDir);

    // strcat(uploadDir, "log/");

    // my_mkdir(uploadDir);

    Cdbg(APP_DBG, "Upload Dir : %s", uploadDir);

    if (dp = opendir(uploadDir)) {

        while (dirp = readdir(dp)) {


            if (strstr (dirp->d_name, ".sql")  || strstr (dirp->d_name, ".db")) {
            // if (strstr (dirp->d_name, ".sql")  || strstr (dirp->d_name, ".ts")  || strstr (dirp->d_name, ".od")) {

                int file_type = 0;  // 1 = ts or mp4 , 2 = od

                if (strstr (dirp->d_name, ".sql")) {

                    snprintf(dbg_msg, sizeof(dbg_msg), "process 'sql or ts' file name : %s\n", dirp->d_name);
                    // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
                    Cdbg(APP_DBG, "%s", dbg_msg);

                    file_type = 1;


                } else {

                    snprintf(dbg_msg, sizeof(dbg_msg), "process 'db' file name : %s\n", dirp->d_name);
                    // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
                    Cdbg(APP_DBG, "%s", dbg_msg);

                    file_type = 2;
                }
                // Cdbg(APP_DBG, "process 'mp4' or 'od' file name : %s\n", dirp->d_name);


                // file size = 0, continue
                if(file_size_check(uploadDir, dirp->d_name) == -1) {
                    continue;
                }


                //if(strcmp (pch,".mp4") == 0) {
                fileNumber++;   

                // get new access token
                status = get_google_token();

                if(status == -1) {
                    Cdbg(APP_DBG, "Refresh token error, exit uploader");
                    exit(0);
                }

                // check date, if different, create new date , upload file to new date
                driveDatecmp();
                
                // upload file
                sqlFileUpload(dirp->d_name, file_type);


                // alarm : need send mail
                // if(strcmp(dirType, "alarm") == 0) {


                //     if(strcmp(sp_conf.alarmEmailNotificaiton, "on") == 0) {

                //         // send mail 
                //         int init_sendemail_status;

                //         snprintf(dbg_msg, sizeof(dbg_msg), "initSendAwsEmail Init\n");
                //         debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);


                //         init_sendemail_status = initSendAwsEmail(username, &asus_conf);
                    
                //         snprintf(dbg_msg, sizeof(dbg_msg), "InitSendAwsEmail end, status = %d\n",init_sendemail_status);
                //         debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);

                //     }

                // }

            } else {
                //Cdbg(APP_DBG, "ignore mp4 file name : %s\n", dirp->d_name);  
            }

            continue;
            //puts(dirp->d_name);
            //Cdbg(APP_DBG, "file is : %s\n", dirp->d_name);

        }
    } 

    closedir(dp);

    // Cdbg(APP_DBG, "Finish upload, Dir /tmp/aicam/%s/%s/ , Upload mp4 file number -> %d", dirType, fileNumber);

}



// dirType 'alarm' or 'record', mac_type
void search_sqlite_download() {

    int status = 0;
    int fileNumber = 0;
    DIR *dp; 
    struct dirent *dirp;

    char downloadDir[128];

    memset(downloadDir,0,sizeof(downloadDir));

    strcpy(downloadDir, config_download_path);
    strcat(downloadDir, "/");

    // strcpy(downloadDir, "/tmp/router/");

    // my_mkdir(downloadDir);

    // strcat(downloadDir, "download/");

    // my_mkdir(downloadDir);
    

    Cdbg(APP_DBG, "download Dir : %s", downloadDir);

    if (dp = opendir(downloadDir)) { 

        while (dirp = readdir(dp)) {


            if (strstr (dirp->d_name, ".sql")  || strstr (dirp->d_name, ".db")) {
            // if (strstr (dirp->d_name, ".sql")  || strstr (dirp->d_name, ".ts")  || strstr (dirp->d_name, ".od")) {

                int file_type = 0;  // 1 = ts or mp4 , 2 = od


                char downloadFilename[128];

                memset(downloadFilename,0,sizeof(downloadFilename));

                strcpy(downloadFilename, downloadDir);
                strcat(downloadFilename, dirp->d_name);

                struct stat buf;
                int x;

                x = lstat (downloadFilename, &buf);

                if(x != 0) {
                    Cdbg(APP_DBG, "download file [%s] reading fail", dirp->d_name);
                    continue;
                }

                if (S_ISLNK(buf.st_mode)) {
                    Cdbg(APP_DBG, "download file [%s] , lstat == link", dirp->d_name);
                } else {
                    Cdbg(APP_DBG, "download file [%s] , lstat != link", dirp->d_name);
                    continue;
                }


                if (strstr (dirp->d_name, ".sql")) {

                    Cdbg(APP_DBG, "process 'sql or ts' file name : %s", dirp->d_name);

                    file_type = 1;


                } else {
                    Cdbg(APP_DBG, "process 'db' file name : %s", dirp->d_name);

                    file_type = 2;
                }
                // Cdbg(APP_DBG, "process 'mp4' or 'od' file name : %s\n", dirp->d_name);


                // file size = 0, continue
                // if(file_size_check(downloadDir, dirp->d_name) == -1) {
                //     continue;
                // }


                //if(strcmp (pch,".mp4") == 0) {
                fileNumber++;   

                // check date, if different, create new date , upload file to new date
                //datecmp(dirp->d_name, dirType, mac_name, mac_type);


                // get new access token
                status = get_google_token();

                if(status == -1) {
                    Cdbg(APP_DBG, "Refresh token error, exit uploader");
                    exit(0);
                }


                status = googleDriveFileSearch(dirp->d_name, NULL);


                // download file exist
                if(status == 0) {

                    do {

                        status = sqlFileDownload(dirp->d_name, file_type);

                        if(status != 0) {
                            Cdbg(APP_DBG, "download file error, db download again after 30 seconds");
                            sleep(30);
                        }

                    } while ( status != 0);

                }



            } else {

                //Cdbg(APP_DBG, "ignore mp4 file name : %s\n", dirp->d_name);  
            }

            continue;
            //puts(dirp->d_name);
            //Cdbg(APP_DBG, "file is : %s\n", dirp->d_name);

        }
    } 

    closedir(dp);

    // Cdbg(APP_DBG, "Finish upload, Dir /tmp/aicam/%s/%s/ , Upload mp4 file number -> %d", dirType, fileNumber);

}


// search mp4 files
// void dir_files_del(char * dirType)
// {

//     int fileNumber = 0;
//     DIR *dp; 
//     struct dirent *dirp;

//     char * pch;
//     char uploadDir[36];;

//     memset(uploadDir,0,sizeof(uploadDir));

//     strcpy(uploadDir, asus_conf.uploader_path);
//     strcat(uploadDir, "/aicam/");
//     strcat(uploadDir, dirType);
//     strcat(uploadDir, "/");

//     my_mkdir(uploadDir);

//     //Cdbg(APP_DBG, "del dir : %s", uploadDir);

//     if (dp = opendir(uploadDir)) { 

//         while (dirp = readdir(dp)) {


//             pch = strstr (dirp->d_name, ".mp4");


//             if (pch != NULL) {

//                 Cdbg(APP_DBG, "process mp4 file name : %s\n", dirp->d_name);


//                 fileNumber++;   

//                 // dels file
//                 char path_file_name[128];
//                 memset(path_file_name, 0, sizeof(path_file_name));

//                 strcpy(path_file_name, uploadDir);
//                 strcat (path_file_name,dirp->d_name);

//                 if( remove(path_file_name) != 0 ) {
//                     Cdbg(APP_DBG, "Error deleting file : %s\n",path_file_name);
//                 } else {
//                     Cdbg(APP_DBG, "File success deleted : %s\n",path_file_name);
//                 }



//             } else {
//                 //Cdbg(APP_DBG, "ignore mp4 file name : %s\n", dirp->d_name);  
//             }

//             continue;
//             //puts(dirp->d_name);
//             //Cdbg(APP_DBG, "file is : %s\n", dirp->d_name);

//         }
//     } 

//     closedir(dp);

//     // Cdbg(APP_DBG, "del Dir /tmp/aicam/%s/ , del mp4 file number -> %d", dirType, fileNumber);

// }

// search aaews txt, wirte tsdb(webstorage)
void process_aaews_event_config(void)
{

    char dirname[50];
    memset(dirname,0,sizeof(dirname));
    // strcpy(dirname, asus_conf.uploader_path);
    strcpy(dirname, "/tmp");
    strcat(dirname, "/aicam/event");


    int fileNumber = 0;
    DIR *dp; 
    struct dirent *dirp;

    char * pch;

    if (dp = opendir(dirname)) { 

        while (dirp = readdir(dp)) {

            //Cdbg(APP_DBG, "Find file name -> %s", dirp->d_name);

            //pch = strpbrk (dirp->d_name, ".txt");
            pch = strstr (dirp->d_name, ".txt");

            if (pch != NULL) {
                /* 也可以直接輸出字串 */
                //Cdbg(APP_DBG, "ignore file : %s\n", pch);

                if(strcmp (pch,".txt") == 0) {

                    fileNumber++;

                    Cdbg(APP_DBG, "txt file name : %s", dirp->d_name);

                    char txt_filename[50];
                    memset(txt_filename,0, sizeof(txt_filename));

                    strcpy(txt_filename, dirname);
                    strcat (txt_filename, "/"); 
                    strcat (txt_filename, dirp->d_name); 


                    Cdbg(APP_DBG, "aaews event file path : %s", txt_filename);


                    process_aaews_even_conf(txt_filename, &ae_conf);

                    Cdbg(APP_DBG, "parse aaews event file, ae_conf.type : %s", ae_conf.type);
                    Cdbg(APP_DBG, "parse aaews event file, ae_conf.start_time : %s", ae_conf.start_time);
                    Cdbg(APP_DBG, "parse aaews event file, ae_conf.device_id : %s", ae_conf.device_id);
                    Cdbg(APP_DBG, "parse aaews event file, ae_conf.model : %s", ae_conf.model);
                    Cdbg(APP_DBG, "parse aaews event file, ae_conf.value1 : %s", ae_conf.value1);
                    Cdbg(APP_DBG, "parse aaews event file, ae_conf.value2 : %s", ae_conf.value2);
                    Cdbg(APP_DBG, "parse aaews event file, ae_conf.value3 : %s", ae_conf.value3);
                    

                    int return_value = 0;
                    return_value = apiTsdbInputNoId(&asus_conf, ae_conf.start_time, ae_conf.type, ae_conf.device_id, ae_conf.model, ae_conf.value1, ae_conf.value2, ae_conf.value3);

                    // 0 : write TSDB success
                    if(return_value == 0) {

                        Cdbg(APP_DBG, "Aaews event message write TSDB success, return value = %d", return_value);

                    } else {
                        Cdbg(APP_DBG, "Aaews event message write TSDB fail, return value = %d", return_value);
                    }

                    // del event file
                    if( remove(txt_filename) != 0 ) {
                        Cdbg(APP_DBG, "Aaews event file remove fail, filename -> %s", txt_filename);
                    } else {
                        Cdbg(APP_DBG, "Aaews event file Remove success, filename -> %s", txt_filename);
                    }
                }

                continue;
            }

        }
    } 

    closedir(dp);

    //Cdbg(APP_DBG, "Aaews event dir [%s], txt file number -> %d\n", dirname, fileNumber);
}

int delAiCamDateID(char * dir_type, char * del_time) {

    long long delAiCamDateID = 0;
    long long parentID = 0;

    if(strstr(dir_type, "record")) {
        
        parentID = aiCamDeviceRecordID;

        delAiCamDateID = checkDirId(parentID, del_time);

    } else if(strstr(dir_type, "alarm")) {

        parentID = aiCamDeviceAlarmID;

        delAiCamDateID = checkDirId(parentID, del_time);

    }

    // ID >= 1000 : get id sucess
    if((delAiCamDateID >= 1000)) {

        Cdbg(APP_DBG, "del date = %s, del type = %s, delAiCamDateID = %lli\n", del_time, dir_type, delAiCamDateID);

        // del del AiCam [alarm] or [record] date
       
        Operateentry *oe;

        oe = removeEntry(username, delAiCamDateID, 0, 1, parentID);

        if(oe == NULL) {
            return -1;
        }
        my_free(oe);

    } else {
        
        Cdbg(APP_DBG, "Not need del data, date = %s, del type = %s, delAiCamDateID = %lli\n", del_time, dir_type, delAiCamDateID);
        return DIR_NOT_EXIST;

    } 

    return 0;
}

void delExpiredFile() {

    // time setting
    time_t t = time(0);

    // if(tcode_channel == SALES_PLAN_ONE) {

    //     t = t - (7 *24 * 3600);

    // } else if(tcode_channel == SALES_PLAN_TWO) {

    //     t = t - (10 *24 * 3600);

    // } else if(tcode_channel == SALES_PLAN_THREE) {

    //     t = t - (30 *24 * 3600);

    // }

    char del_time[12];
    memset(del_time,0, sizeof(del_time));
    strftime( del_time, sizeof(del_time), "%Y%m%d",localtime(&t) );

    Cdbg(APP_DBG, "prepare del_time : %s\n", del_time);


    // del record date 
    delAiCamDateID("alarm", del_time);

    // del alarm date
    // delAiCamDateID("record", del_time);

}

// step 1: browse all date dir
// step 2: del old date
int delSpaceInsufficient() {

    Cdbg(APP_DBG, "Browse aiCamV2ID = %lli", aiCamV2ID);

    int del_status = 1;

    ////////////////////

    Browse *browse = NULL;

    // Get V2 Dir
    while(1) {

        browse = browseFolder(username, aiCamV2ID,0,1);

        if(NULL == browse) {
            Cdbg(APP_DBG, "Get aiCamV2ID : browse is null");
            return DATA_IS_NULL;
        }

        int status = browse->status;

        Cdbg(APP_DBG, "Get aiCamV2ID : browse->status = %d", status);

        if(status == 0) {
            break;
        } else if((status == CONNECT_TIMEOUT) || 
                  (status == CURL_CANNOT_RESOLVE_HOSTNAME) ||
                  (status == CURLE_SSL_CONNECT_ERROR)) {

            my_free(browse);

        } else if(status == S_AUTH_FAIL) {

            Cdbg(APP_DBG, "Get aiCamV2ID : Account auth fail\n");

            my_free(browse);

            int token_status = GetToken("Account Auth fail,OAuth");
            if(token_status == 225) {
                Cdbg(APP_DBG, "status : 225 = access token fail, uploader Over");
                exit(1);
            }

            // browse Attachable Device Check, if space plan change?
            browseAttachableDeviceCheck();

        // other status
        } else {
            my_free(browse);
        }
    }

    Cdbg(APP_DBG, "aiCamV2ID browse Folder, filenum=%d,foldernum=%d",browse->filenumber,browse->foldernumber);

    int foldernumber = browse->foldernumber;

    // del old folder
    if(foldernumber == 1)  {

        Cdbg(APP_DBG, "start del V2/AiCAM_XXXXXXXXXXXX/alarm/date folder");

        delExpiredSpaceDate();

    } else if(foldernumber > 1) {

        Cdbg(APP_DBG, "start del more V2/AiCAM_XXXXXXXXXXXX folder");

        // browse Attachable Device Info

        Cdbg(APP_DBG, "start get browseAttachableDeviceList");

        int browseDeviceStatus = browseAttachableDeviceList(username, &ap_conf);

        Cdbg(APP_DBG, "browseDeviceStatus = %d" , browseDeviceStatus);

        // g : et data success
        if(browseDeviceStatus == 0) {


            Cdbg(APP_DBG, "del more folder : start");

            int i=0;

            for(i=0; i < foldernumber; i++) {

                Cdbg(APP_DBG, "foldernumber = %d, now foldernumber count =%d", foldernumber, i+1);

                Folder *fd = browse->folderlist[i];

                Cdbg(APP_DBG, "Folder id = %lli, dir name(Base64Enecode) = %s", fd->id, fd->display);     

                char* dataBase64Decode;
                size_t date_len;
                Base64Decode(fd->display, &dataBase64Decode, &date_len);

                Cdbg(APP_DBG, "Folder id = %lli, Dir V2/%s, len = %d", fd->id, dataBase64Decode, date_len);

                int checkBrowseDeviceStatus = checkBrowseAttachableDeviceXml(get_browse_attachable_device_xml, dataBase64Decode);

                Cdbg(APP_DBG, "checkBrowseDeviceStatus = %d", checkBrowseDeviceStatus);

                Cdbg(APP_DBG, "V2 id = %lli, Folder id = %lli, Dir V2/%s, len = %d", aiCamV2ID, fd->id, dataBase64Decode, date_len);

                free(dataBase64Decode);

                // 0 : is dir exist ,
                if(checkBrowseDeviceStatus == 0) {
                    
                    Cdbg(APP_DBG, "Del Dir -> V2/AiCAM_XXXXXXXXXXXX/alarm/date , V2 id = %lli, Folder id = %lli, ", aiCamV2ID, fd->id);


                    delV2MacDateDir(fd->id, dataBase64Decode);


                // !=0 dir not exist -> del
                } else {


                    // 
                    Cdbg(APP_DBG, "Start del V2 id = %lli, Folder id = %lli, Dir V2/Not AiCAM Dir/", aiCamV2ID, fd->id);

                    // del del AiCam [V2/XXXXXXX] Dir
       
                    Operateentry *oe;

                    oe = removeEntry(username, fd->id, 0, 1, aiCamV2ID);

                    if(oe == NULL) {
                        del_status = -1;
                        Cdbg(APP_DBG, "del dir fail");
                    } else {

                        del_status = oe->status;

                        if(del_status == 0) {
                            Cdbg(APP_DBG, "del dir success");

                        }
                    }
                    my_free(oe);
                    
                }


                free(fd);
                
            }

        }

    } else {

        Cdbg(APP_DBG, "browse folder number error");

    }


    free(browse);


    return del_status;
}

// step 1: browse all date dir
// step 2: del old date
int delExpiredSpaceDate() {

    Cdbg(APP_DBG, "Browse aiCamDeviceAlarmID = %lli", aiCamDeviceAlarmID);

    int del_status = 1;

    Browse *browse = NULL;

    while(1) {

        browse = browseFolder(username, aiCamDeviceAlarmID,0,1);

        if(NULL == browse) {
            Cdbg(APP_DBG, "browse is null");
            return DATA_IS_NULL;
        }

        int status = browse->status;

        Cdbg(APP_DBG, "browse->status = %d", status);

        if(status == 0) {
            break;
        } else if((status == CONNECT_TIMEOUT) || 
                  (status == CURL_CANNOT_RESOLVE_HOSTNAME) ||
                  (status == CURLE_SSL_CONNECT_ERROR)) {

            my_free(browse);

        } else if(status == S_AUTH_FAIL) {

            Cdbg(APP_DBG, "Account auth fail\n");

            my_free(browse);

            int token_status = GetToken("Account Auth fail,OAuth");
            if(token_status == 225) {
                Cdbg(APP_DBG, "status : 225 = access token fail, uploader Over");
                exit(1);
            }

            // browse Attachable Device Check, if space plan change?
            browseAttachableDeviceCheck();

        // other status
        } else {
            my_free(browse);
        }
    }

    Cdbg(APP_DBG, "browse Folder, filenum=%d,foldernum=%d",browse->filenumber,browse->foldernumber);

    int foldernumber = browse->foldernumber;

    // del old folder
    if(foldernumber > 1) {

        Cdbg(APP_DBG, "del more folder : start");

        Folder *fd = browse->folderlist[0];
        Cdbg(APP_DBG, "del date base64encode = %s", fd->display);        
        //Cdbg(APP_DBG, "i=%d, id=%d, ttttt->display =%s",i+1, ttttt->id, ttttt->display); 

        char* dataBase64Decode;
        size_t date_len;
        Base64Decode(fd->display, &dataBase64Decode, &date_len);

        Cdbg(APP_DBG, "del date = %s, len = %d", dataBase64Decode, date_len);        

        del_status = delAiCamDateID("alarm", dataBase64Decode);

        if(del_status == 0) {
            Cdbg(APP_DBG, "del date = %s -> success", dataBase64Decode);
        } else {
            Cdbg(APP_DBG, "del date = %s -> fail", dataBase64Decode);
        }

        free(dataBase64Decode);

        free(fd);

    // del only one folder -> files(500)
    } else if(foldernumber == 1) {

        Cdbg(APP_DBG, "del more files : start, foldernumber = %d", foldernumber);

        Folder *fd = browse->folderlist[0];

        Cdbg(APP_DBG, "del date id = %lli", fd->id);  

        char* dataBase64Decode;
        size_t date_len;
        Base64Decode(fd->display, &dataBase64Decode, &date_len);

        Cdbg(APP_DBG, "del date = %s, len = %d", dataBase64Decode, date_len);        

    
        long long timestamp = current_time_with_ms(dataBase64Decode);
        // long long timestamp = 1485014400000;
        //del_status = delAiCamDateID("alarm", dataBase64Decode);
        del_status = spaceExpiredFileProcess(fd->id, timestamp);


        if(del_status == 0) {
            Cdbg(APP_DBG, "del date = %s -> success", dataBase64Decode);
        } else {
            Cdbg(APP_DBG, "del date = %s -> fail", dataBase64Decode);
        }

        free(dataBase64Decode);

        free(fd);

    } else {

        Cdbg(APP_DBG, "browse folder number error");

    }


    // int i=0;    
    // for(i=0; i < browse->foldernumber; i++) {

    //     Folder *ttttt = browse->folderlist[i];
    //     Cdbg(APP_DBG, "i=%d, id=%d, ttttt->createdtime =%s",i+1, ttttt->id, ttttt->createdtime);        
    //     Cdbg(APP_DBG, "i=%d, id=%d, ttttt->display =%s",i+1, ttttt->id, ttttt->display);        

    // }

    free(browse);


    return del_status;
}



// step 1: get aiCam->V2->Mac Address -> alarm dir
// step 2: del old date
int delV2MacDateDir(long long aiCamV2MacId, char *aiCamV2MacName) {

    int aiCamV2MaAlarmcId = 0;

    while(1) {

        aiCamV2MaAlarmcId = checkDirId(aiCamV2MacId, "alarm");

        // ID >= 1000 : get id sucess
        if(aiCamV2MaAlarmcId == DIR_NEED_CREATE) {

            Cdbg(APP_DBG, "aiCamV2MaAlarmcId = %lli, not exist \n", aiCamV2MaAlarmcId);

            return 0;

        } else if(aiCamV2MaAlarmcId >= 1000) { 

            Cdbg(APP_DBG, "V2/Mac_Addr/alarm Dir exist, aiCamV2MaAlarmcId = %lli, prepare del [date] dir\n", aiCamV2MaAlarmcId);
            break;
        } else {

            Cdbg(APP_DBG, "aiCamDeviceRecordDateID get data fail, waiting 1s, continue \n");
            sleep(1);
            continue;
        }

    }


    int del_status = 1;

    Browse *browse = NULL;

    while(1) {

        browse = browseFolder(username, aiCamV2MaAlarmcId,0,1);

        if(NULL == browse) {
            Cdbg(APP_DBG, "browse is null");
            return DATA_IS_NULL;
        }

        int status = browse->status;

        Cdbg(APP_DBG, "browse->status = %d", status);

        if(status == 0) {
            break;
        } else if((status == CONNECT_TIMEOUT) || 
                  (status == CURL_CANNOT_RESOLVE_HOSTNAME) ||
                  (status == CURLE_SSL_CONNECT_ERROR)) {

            my_free(browse);

        } else if(status == S_AUTH_FAIL) {

            Cdbg(APP_DBG, "Account auth fail\n");

            my_free(browse);

            int token_status = GetToken("Account Auth fail,OAuth");
            if(token_status == 225) {
                Cdbg(APP_DBG, "status : 225 = access token fail, uploader Over");
                exit(1);
            }

            // browse Attachable Device Check, if space plan change?
            //browseAttachableDeviceCheck();

        // other status
        } else {
            my_free(browse);
        }
    }

    Cdbg(APP_DBG, "browse Folder, filenum=%d,foldernum=%d",browse->filenumber,browse->foldernumber);

    int foldernumber = browse->foldernumber;

    // del old folder
    if(foldernumber > 1) {

        Cdbg(APP_DBG, "del more folder : start");

        Folder *fd = browse->folderlist[0];
        Cdbg(APP_DBG, "del date base64encode = %s", fd->display);        
        //Cdbg(APP_DBG, "i=%d, id=%d, ttttt->display =%s",i+1, ttttt->id, ttttt->display); 

        char* dataBase64Decode;
        size_t date_len;
        Base64Decode(fd->display, &dataBase64Decode, &date_len);

        Cdbg(APP_DBG, "del date = %s, len = %d", dataBase64Decode, date_len);        

        del_status = delAiCamDateID("alarm", dataBase64Decode);

        if(del_status == 0) {
            Cdbg(APP_DBG, "del date = %s -> success", dataBase64Decode);
        } else {
            Cdbg(APP_DBG, "del date = %s -> fail", dataBase64Decode);
        }

        free(dataBase64Decode);

        free(fd);

    // del only one folder -> files(500)
    } else if(foldernumber == 1) {

        Cdbg(APP_DBG, "del more files : start, foldernumber = %d", foldernumber);

        Folder *fd = browse->folderlist[0];

        Cdbg(APP_DBG, "del date id = %lli", fd->id);  

        char* dataBase64Decode;
        size_t date_len;
        Base64Decode(fd->display, &dataBase64Decode, &date_len);

        Cdbg(APP_DBG, "del date = %s, len = %d", dataBase64Decode, date_len);        

    
        long long timestamp = current_time_with_ms(dataBase64Decode);
        // long long timestamp = 1485014400000;
        //del_status = delAiCamDateID("alarm", dataBase64Decode);
        del_status = spaceExpiredFileProcess(fd->id, timestamp);


        if(del_status == 0) {
            Cdbg(APP_DBG, "del date = %s -> success", dataBase64Decode);
        } else {
            Cdbg(APP_DBG, "del date = %s -> fail", dataBase64Decode);
        }

        free(dataBase64Decode);

        free(fd);

    } else {

        Cdbg(APP_DBG, "browse folder number error");

    }

    free(browse);


    return del_status;
}

// del webstorage files
int spaceExpiredFileProcess(long long folderid, long long timestamp) {

    int del_files_status = -1;

    // get expired files
    SlidingBrowse *slidingBrowse = NULL;

    while(1) {

        slidingBrowse = slidingBrowseFile(username, folderid, timestamp);

        if(NULL == slidingBrowse) {
            Cdbg(APP_DBG, "slidingBrowse is null");
            return DATA_IS_NULL;
        }

        int status = slidingBrowse->status;

        Cdbg(APP_DBG, "slidingBrowse->status = %d", status);

        if(status == 0) {
            break;
        } else if((status == CONNECT_TIMEOUT) || 
                  (status == CURL_CANNOT_RESOLVE_HOSTNAME) ||
                  (status == CURLE_SSL_CONNECT_ERROR)) {

            my_free(slidingBrowse);

        } else if(status == S_AUTH_FAIL) {

            Cdbg(APP_DBG, "Account auth fail\n");
            my_free(slidingBrowse);

            int token_status = GetToken("Account Auth fail,OAuth");
            if(token_status == 225) {
                Cdbg(APP_DBG, "status : 225 = access token fail, uploader Over");
                exit(1);
            }

            // browse Attachable Device Check, if space plan change?
            browseAttachableDeviceCheck();

        // other status
        } else {
            my_free(slidingBrowse);
        }
    }


    Cdbg(APP_DBG, "slidingBrowse Files, filenum=%d",slidingBrowse->filenumber);

    int filenumber = slidingBrowse->filenumber;

    if(filenumber > 1) {

        Cdbg(APP_DBG, "del more files : start");

        // setting files len
        int file_char_len = filenumber * 21;

        Cdbg(APP_DBG, "file_char_len = %d", file_char_len);

        char *id_total_str = (char*)malloc(sizeof(char) * file_char_len);  
        memset(id_total_str, 0, sizeof(id_total_str));
        
        int i=0; 

        for(i=0; i < filenumber; i++) {

            Files *fs = slidingBrowse->filelist[i];

            strcat(id_total_str, fs->id);

            if(i == (filenumber -1)) {
                continue;
            } else {
                strcat(id_total_str, ",");
            }

            free(fs);
        }

        Cdbg(APP_DBG, "id_total_str = %s", id_total_str);

        // del files
        Operateentry *rf = NULL;;

        rf = removeFiles(username, id_total_str);

        Cdbg(APP_DBG, "removeFiles->status = %d", rf->status);

        if(rf->status == 0) {

            Cdbg(APP_DBG, "removeFiles success");

        } else {

            Cdbg(APP_DBG, "removeFiles fail");

        }


        free(id_total_str);
        free(rf);
    }

    free(slidingBrowse);

    return 0;
}

int delOverSpaceFile(int del_day) {

    
    Cdbg(APP_DBG, "aiCamDeviceAlarmID = %lli, del_day = %d", aiCamDeviceAlarmID, del_day);

    // del record date 
    int del_status = 1;
    int i;
    for(i = 0; i < del_day; i++) {

        // del time setting
        time_t t = time(0);
        t = t - ((del_day - i) *24 * 3600);

        char del_time[12];
        memset(del_time,0, sizeof(del_time));
        strftime( del_time, sizeof(del_time), "%Y%m%d",localtime(&t) );

        Cdbg(APP_DBG, "i = %d, prepare del_time : %s\n", i, del_time);

        del_status = delAiCamDateID("alarm", del_time);

        if(del_status == 0) {
            break;
        }

    }

    del_day = del_day - i + 1;

    return del_day;
}


long long current_time_with_ms (char * folder_date) {

    char t_year[5];
    memset(t_year,0,sizeof(t_year));
    memcpy( t_year, &folder_date[0], 4 );

    Cdbg(APP_DBG, "t_year = %s", t_year);

    int i_year = atoi(t_year);

    char t_month[3];
    memset(t_month,0,sizeof(t_month));
    memcpy( t_month, &folder_date[4], 2 );

    Cdbg(APP_DBG, "t_month = %s", t_month);

    int i_month = atoi(t_month);

    char t_day[3];
    memset(t_day,0,sizeof(t_day));

    memcpy( t_day, &folder_date[6], 2 );

    Cdbg(APP_DBG, "t_day = %s", t_day);

    int i_day = atoi(t_day);


    struct tm tday;
    time_t t_of_day;

    tday.tm_year = i_year - 1900;
    tday.tm_mon = i_month-1;
    tday.tm_mday = i_day;
    tday.tm_hour = 0;
    tday.tm_min = 0;
    tday.tm_sec = 0;
    tday.tm_isdst=0;

    t_of_day = mktime(&tday) ;


    //printf(ctime(&t_of_day));
    Cdbg(APP_DBG, "ctime(&t_of_day) = %s", ctime(&t_of_day));

    //t_of_day = t_of_day ;

    // printf("time()->localtime()->mktime():%zu\n", t_of_day);
    


    long long timestamp = t_of_day;

    timestamp = timestamp * 1000;

    Cdbg(APP_DBG, "timestamp = %lli", timestamp);

    return timestamp;
}

void mytest() {



    delExpiredSpaceDate();

    exit(1);

    // if( (tcode_channel == SALES_PLAN_ONE) || 
    //     (tcode_channel == SALES_PLAN_TWO) || 
    //     (tcode_channel == SALES_PLAN_THREE) ) {

    //     int keep_day = 0;

    //     if( tcode_channel == SALES_PLAN_ONE) {

    //         keep_day = delOverSpaceFile(KEEP_DAY_ONE);

    //     } else if(tcode_channel == SALES_PLAN_TWO) {

    //         keep_day = delOverSpaceFile(KEEP_DAY_TWO);

    //     } else if( tcode_channel == SALES_PLAN_THREE) {

    //         keep_day = delOverSpaceFile(KEEP_DAY_THREE);
    //     }


    //     Cdbg(APP_DBG, "keep_day = %d", keep_day);

    //     if(keep_day == 1) {

    //         Cdbg(APP_DBG, "process today file");

    //     }
    // }



    exit(1);
}



void dir_search_thread()
{

    Cdbg(APP_DBG, "start : Log Dir search");

    // char time_now[26];

    // time_t t = time(0);

    // memset(time_now,0, sizeof(time_now));
    // strftime(time_now, sizeof(time_now), "%Y-%m-%d %X",localtime(&t) );  //:%A:%S


    while(1) {

        // process aaews txt file context
        // Cdbg(APP_DBG, "start : process event file content");

        // process_aaews_event_config();

        //search_mac_config();

        // create && get webstorage mac folder (macxxxx/alarm/date) 
        //search_mac_dir();

        // search video && uploader video
        // search_video_dir();


        search_sqlite_upload();

        //video_upload("alarm", 1);


        search_sqlite_download();

        sleep(60);
    }




}


// search mac dir , create webstorage mac folder (macxxxx/alarm/date) 
void search_mac_config() {

    int macConfigNumber = 0;
    DIR *dp; 
    struct dirent *dirp;

    char * pch;
    char aicamDir[36];

    memset(aicamDir,0,sizeof(aicamDir));

    strcpy(aicamDir, asus_conf.uploader_path);

    my_mkdir(aicamDir);


    snprintf(dbg_msg, sizeof(dbg_msg), "mac config search dir : %s", aicamDir);
    // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
    Cdbg(APP_DBG, "%s", dbg_msg);

    if (dp = opendir(aicamDir)) { 

        while (dirp = readdir(dp)) {

            // pch = strstr (dirp->d_name, "AiCAM_");

            if (strstr(dirp->d_name, "system_mac_")) {

                snprintf(dbg_msg, sizeof(dbg_msg), "find mac config = %s", dirp->d_name);
                // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
                Cdbg(APP_DBG, "%s", dbg_msg);

                // file or dir check
                char macConfigName[48];

                memset(macConfigName,0,sizeof(macConfigName));

                strcpy(macConfigName, aicamDir);
                strcat(macConfigName, dirp->d_name);

                // search mac dir
                struct stat s;

                if( stat(macConfigName,&s) == 0 ) {

                    if( s.st_mode & S_IFREG ) {
                        //it's a file
                        snprintf(dbg_msg, sizeof(dbg_msg), "%s  file EXISITS!", macConfigName);
                        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
                        Cdbg(APP_DBG, "%s", dbg_msg);

                        // parse mac config data

                        initSystemConfig(&s_conf, macConfigName);

                        // setting asuswebstorage.conf
                        initAsusWebstorageCfg(&asus_conf, &ap_conf, &dp_conf, &si_conf, &bc_conf, &s_conf);

                        snprintf(dbg_msg, sizeof(dbg_msg), "initattachabledevice = %s", s_conf.initattachabledevice);
                        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
                        Cdbg(APP_DBG, "%s", dbg_msg);

                        if( strstr(s_conf.initattachabledevice, "off")) {
                            // tcode_channel -> gobal var
                            initTcodeParam();

                            // process init AiCam Device
                            int init_device_status;
                            init_device_status = initDevice();

                            Cdbg(APP_DBG, "init_device_status = %d", init_device_status);

                            //if((init_device_status == DEVICE_AUTH_FAIL) || (init_device_status == S_GER_ERR)) {
                            if(init_device_status != 0) {
                                tsdbProcess(init_device_status, "init_device:DEVICE AUTH FAIL");
                                // insert tsdb
                                //return DEVICE_AUTH_FAIL;
                            }

                            Cdbg(APP_DBG, "initDevice end");

                            // Step 3 : init device success, aaewsProvision config content -> password = "" ,
                            // Indicates that the device is initialized
                            if(init_device_status == 0) {

                                systemMacAddressEdit(&s_conf, macConfigName);
                                Cdbg(APP_DBG, "write [%s] content -> OK", macConfigName);


                            }
                        }

                    }

                } else {
                    //error
                   // snprintf(dbg_msg, sizeof(dbg_msg), "%s error!", aicamMacdDir);
                   // debugLogLevel(DEBUG_LEVEL, DBG_ERROR, dbg_msg);
                }



            } else {
                snprintf(dbg_msg, sizeof(dbg_msg), "not mac file -> ignore file = %s", dirp->d_name);
                // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
                Cdbg(APP_DBG, "%s", dbg_msg);
            }

            continue;
        }
    } 

    closedir(dp);

    //Cdbg(APP_DBG, "Finish upload, Dir /tmp/aicam/%s/ , Upload mp4 file number -> %d", dirType, aicamDirNumber);

}

// search mac dir , create webstorage mac folder (macxxxx/alarm/date) 
void search_mac_dir() {

    int macDirNumber = 0;
    DIR *dp; 
    struct dirent *dirp;

    char * pch;
    char macDir[36];

    memset(macDir,0,sizeof(macDir));

    strcpy(macDir, asus_conf.uploader_path);
    strcat(macDir, "mac/");

    my_mkdir(macDir);


    snprintf(dbg_msg, sizeof(dbg_msg), "mac Dir search dir : %s", macDir);
    // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
    Cdbg(APP_DBG, "%s", dbg_msg);

    if (dp = opendir(macDir)) { 

        while (dirp = readdir(dp)) {

            // pch = strstr (dirp->d_name, "AiCAM_");

            if (strstr(dirp->d_name, "AiCAM_")) {

                snprintf(dbg_msg, sizeof(dbg_msg), "aicam mac Dir = %s", dirp->d_name);
                // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
                Cdbg(APP_DBG, "%s", dbg_msg);


                // file or dir check
                char aicamMacdDir[48];

                memset(aicamMacdDir,0,sizeof(aicamMacdDir));

                strcpy(aicamMacdDir, macDir);
                strcat(aicamMacdDir, dirp->d_name);

                // search mac dir
                struct stat s;

                if( stat(aicamMacdDir,&s) == 0 ) {

                    if( s.st_mode & S_IFDIR ) {
                        //it's a directory
                        snprintf(dbg_msg, sizeof(dbg_msg), "%s  Dir EXISITS!", aicamMacdDir);
                        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
                        Cdbg(APP_DBG, "%s", dbg_msg);


                        macDirNumber++;

                        if (strcmp (dirp->d_name, aiCamMacAddressOne) == 0) {


                            getMacVideoMode(&s_conf, dirp->d_name, 1);

                   
                            getMacAlarmDateID(dirp->d_name, 1);

                            if(strstr(macVideoModeOne, TIME_CONTINOUS_7DAYS)) {
                                getMacRecordDateID(dirp->d_name, 1);
                            }



                        } else if (strcmp (dirp->d_name, aiCamMacAddressTwo) == 0) {


                            getMacVideoMode(&s_conf, dirp->d_name, 2);
                   
                            getMacAlarmDateID(dirp->d_name, 2);

                            if(strstr(macVideoModeTwo, TIME_CONTINOUS_7DAYS)) {
                                getMacRecordDateID(dirp->d_name, 2);
                            }

                        } else if (strcmp (dirp->d_name, aiCamMacAddressThree) == 0) {


                            getMacVideoMode(&s_conf, dirp->d_name, 3);
                   
                            getMacAlarmDateID(dirp->d_name, 3);

                            if(strstr(macVideoModeThree, TIME_CONTINOUS_7DAYS)) {
                                getMacRecordDateID(dirp->d_name, 3);
                            }

                        } else if (strcmp (dirp->d_name, aiCamMacAddressFour) == 0) {

                            getMacVideoMode(&s_conf, dirp->d_name, 4);

                            getMacAlarmDateID(dirp->d_name, 4);

                            if(strstr(macVideoModeFour, TIME_CONTINOUS_7DAYS)) {
                                getMacRecordDateID(dirp->d_name, 4);
                            }

                        } else {

                            //macDirNumber++;

                            // Get the correct ID
                            macDirNumber = get_correct_id(macDirNumber);

                            getMacVideoMode(&s_conf, dirp->d_name, macDirNumber);

                            Cdbg(APP_DBG, "dirp->d_name : %s", dirp->d_name);

                            getMacAlarmDateID(dirp->d_name, macDirNumber);

                            Cdbg(APP_DBG, "------------------------------------------------------------------------------------------");

                            if(strstr(macVideoModeOne, TIME_CONTINOUS_7DAYS) && (macDirNumber == 1)) {
                                Cdbg(APP_DBG, "1111111111111111111111030303030303030303030033");
                                getMacRecordDateID(dirp->d_name, macDirNumber);
                            } else if(strstr(macVideoModeTwo, TIME_CONTINOUS_7DAYS) && (macDirNumber == 2)) {
                                Cdbg(APP_DBG, "132222222222222222030303030303030303030033");
                                getMacRecordDateID(dirp->d_name, macDirNumber);
                            } else if(strstr(macVideoModeThree, TIME_CONTINOUS_7DAYS) && (macDirNumber == 3)) {
                                Cdbg(APP_DBG, "12233333333333333333331030303030303030303030033");
                                getMacRecordDateID(dirp->d_name, macDirNumber);
                            } else if(strstr(macVideoModeFour, TIME_CONTINOUS_7DAYS) && (macDirNumber == 4)) {
                                Cdbg(APP_DBG, "14444444444444444444444444111030303030303030303030033");
                                getMacRecordDateID(dirp->d_name, macDirNumber);
                            }

                        }

                        // debug message
                        if(macDirNumber == 1) {

                            snprintf(dbg_msg, sizeof(dbg_msg), "Get Mac One Name = %s , Mac ID = %lli,  Alarm ID = %lli, Date ID = %lli", dirp->d_name, aiCamMacAddressOneID, aiCamMacAddressOneAlarmID, aiCamMacAddressOneAlarmDateID);
                            
                        } else if(macDirNumber == 2) {

                            snprintf(dbg_msg, sizeof(dbg_msg), "Get Mac Two Name = %s , Mac ID = %lli,  Alarm ID = %lli, Date ID = %lli", dirp->d_name, aiCamMacAddressTwoID, aiCamMacAddressTwoAlarmID, aiCamMacAddressTwoAlarmDateID);

                        } else if(macDirNumber == 3) {

                            snprintf(dbg_msg, sizeof(dbg_msg), "Get Mac Three Name = %s , Mac ID = %lli,  Alarm ID = %lli, Date ID = %lli", dirp->d_name, aiCamMacAddressThreeID, aiCamMacAddressThreeAlarmID, aiCamMacAddressThreeAlarmDateID);

                        } else if(macDirNumber == 4) {

                            snprintf(dbg_msg, sizeof(dbg_msg), "Get Mac Four Name = %s , Mac ID = %lli,  Alarm ID = %lli, Date ID = %lli", dirp->d_name, aiCamMacAddressFourID, aiCamMacAddressFourAlarmID, aiCamMacAddressFourAlarmDateID);

                        } else {

                            snprintf(dbg_msg, sizeof(dbg_msg), "Get Device Mac , error mac number");

                        }

                        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
                        Cdbg(APP_DBG, "%s", dbg_msg);


                    } else if( s.st_mode & S_IFREG ) {
                        //it's a file
                        snprintf(dbg_msg, sizeof(dbg_msg), "%s file EXISITS!", aicamMacdDir);
                        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
                        Cdbg(APP_DBG, "%s", dbg_msg);

                    }  else {
                        //something else
                        snprintf(dbg_msg, sizeof(dbg_msg), "%s file EXISITS!", aicamMacdDir);
                        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
                        Cdbg(APP_DBG, "%s", dbg_msg);

                    }

                } else {
                    //error
                    snprintf(dbg_msg, sizeof(dbg_msg), "%s error!", aicamMacdDir);
                    // debugLogLevel(DEBUG_LEVEL, DBG_ERROR, dbg_msg);
                    Cdbg(APP_DBG, "%s", dbg_msg);
                }




            } else {
                snprintf(dbg_msg, sizeof(dbg_msg), "mac dir -> ignore file name = %s", dirp->d_name);
                // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
                Cdbg(APP_DBG, "%s", dbg_msg);
            }

            continue;
        }
    } 

    closedir(dp);

    //Cdbg(APP_DBG, "Finish upload, Dir /tmp/aicam/%s/ , Upload mp4 file number -> %d", dirType, macDirNumber);


}



int getMacVideoMode(struct system_conf * s_conf, char * mac_name, int mac_type);

int getMacVideoMode(struct system_conf * s_conf, char * mac_name, int mac_type) {


    // get mac address
    char config_mac[20];
    memset(config_mac, 0, sizeof(config_mac));
    strcpy(config_mac, mac_name);



    strcpy(config_mac, strtok(config_mac, "_"));

    //char *config_mac = strtok(mac_name, "_");
    int i = 0;
    while (config_mac != NULL) {
        i++;
        if(i == 2) { break; }

        //config_mac = strtok(NULL, "_");

        strcpy(config_mac, strtok(NULL, "_"));
    }


    char mac_config_path[36];

    memset(mac_config_path, 0, sizeof(mac_config_path));

    strcpy(mac_config_path, asus_conf.uploader_path);
    strcat(mac_config_path, "system_mac_");
    strcat(mac_config_path, config_mac);

    Cdbg(APP_DBG, "Get mac video mode : mac_config_path : %s", mac_config_path);


    process_system_config(s_conf, mac_config_path);

    // Cdbg(API_DBG, "system config parse   ----------");
    // Cdbg(API_DBG, "JSON File parse, s_conf->fwver : %s", s_conf->fwver);
    // Cdbg(API_DBG, "JSON File parse, s_conf->tcode : %s", s_conf->tcode);
    // Cdbg(API_DBG, "JSON File parse, s_conf->sn : %s", s_conf->sn);
    // Cdbg(API_DBG, "JSON File parse, s_conf->recordplan : %s", s_conf->recordplan);
    // Cdbg(API_DBG, "JSON File parse, s_conf->privateip : %s", s_conf->privateip);
    Cdbg(APP_DBG, "JSON File parse, s_conf->macaddr : %s", s_conf->macaddr);
    Cdbg(APP_DBG, "JSON File parse, s_conf->videomode : %s", s_conf->videomode);
    Cdbg(APP_DBG, "JSON File parse, s_conf->initattachabledevice : %s", s_conf->initattachabledevice);


    
    if(mac_type == 1) {
        snprintf(macVideoModeOne, sizeof(macVideoModeOne), "%s", s_conf->videomode);
        Cdbg(APP_DBG, "mac_type = %d , macVideoModeOne : %s", mac_type, macVideoModeOne);
    } else if(mac_type == 2) {
        snprintf(macVideoModeTwo, sizeof(macVideoModeTwo), "%s", s_conf->videomode);
        Cdbg(APP_DBG, "mac_type = %d , macVideoModeTwo : %s", mac_type, macVideoModeTwo);
    } else if(mac_type == 3) {
        snprintf(macVideoModeThree, sizeof(macVideoModeThree), "%s", s_conf->videomode);
        Cdbg(APP_DBG, "mac_type = %d , macVideoModeOne : %s", mac_type, macVideoModeOne);
    } else if(mac_type == 4) {
        snprintf(macVideoModeFour, sizeof(macVideoModeFour), "%s", s_conf->videomode);
        Cdbg(APP_DBG, "mac_type = %d , macVideoModeFour : %s", mac_type, macVideoModeFour);
    }
   // exit(0);

}


int get_correct_id(int macDirNumber) {

    if( (macDirNumber == 1) &&  
        (aiCamMacAddressOneID > 1000) && 
        (aiCamMacAddressOneAlarmID > 1000) &&  
        (aiCamMacAddressOneAlarmDateID > 1000)) {

        macDirNumber++;
    }

    if( (macDirNumber == 2) && 
        (aiCamMacAddressTwoID > 1000) && 
        (aiCamMacAddressTwoAlarmID > 1000) &&  
        (aiCamMacAddressTwoAlarmDateID > 1000)) {

        macDirNumber++;

    }


    if( (macDirNumber == 3) && 
        (aiCamMacAddressThreeID > 1000) && 
        (aiCamMacAddressThreeAlarmID > 1000) &&  
        (aiCamMacAddressThreeAlarmDateID > 1000)) {


        macDirNumber++;

    }

    return macDirNumber;


}




void getMacAlarmDateID(char * mac_name, int mac_type) {

    long long tempAiCamMacID = 0;

    long long tempAiCamMacAlarmID = 0;
    long long tempAiCamMacAlarmDateID = 0;

    if(mac_type == 1) {

        tempAiCamMacID = aiCamMacAddressOneID;
        tempAiCamMacAlarmID = aiCamMacAddressOneAlarmID;
        tempAiCamMacAlarmDateID = aiCamMacAddressOneAlarmDateID;

        snprintf(dbg_msg, sizeof(dbg_msg), "Device Mac One ->  Name = %s , Mac ID = %lli / Alarm ID = %lli / Alarm Date ID = %lli", mac_name, tempAiCamMacID, tempAiCamMacAlarmID, tempAiCamMacAlarmDateID);

    } else if(mac_type == 2) {

        tempAiCamMacID = aiCamMacAddressTwoID;
        tempAiCamMacAlarmID = aiCamMacAddressTwoAlarmID;
        tempAiCamMacAlarmDateID = aiCamMacAddressTwoAlarmDateID;

        snprintf(dbg_msg, sizeof(dbg_msg), "Device Mac Two ->  Name = %s , Mac ID = %lli / Alarm ID = %lli / Alarm Date ID = %lli", mac_name, tempAiCamMacID, tempAiCamMacAlarmID, tempAiCamMacAlarmDateID);

    } else if(mac_type == 3) {

        tempAiCamMacID = aiCamMacAddressThreeID;
        tempAiCamMacAlarmID = aiCamMacAddressThreeAlarmID;
        tempAiCamMacAlarmDateID = aiCamMacAddressThreeAlarmDateID;

        snprintf(dbg_msg, sizeof(dbg_msg), "Device Mac Three ->  Name = %s , Mac ID = %lli / Alarm ID = %lli / Alarm Date ID = %lli", mac_name, tempAiCamMacID, tempAiCamMacAlarmID, tempAiCamMacAlarmDateID);

    } else if(mac_type == 4) {

        tempAiCamMacID = aiCamMacAddressFourID;
        tempAiCamMacAlarmID = aiCamMacAddressFourAlarmID;
        tempAiCamMacAlarmDateID = aiCamMacAddressFourAlarmDateID;

        snprintf(dbg_msg, sizeof(dbg_msg), "Device Mac Four ->  Name = %s , Mac ID = %lli / Alarm ID = %lli / Alarm Date ID = %lli", mac_name, tempAiCamMacID, tempAiCamMacAlarmID, tempAiCamMacAlarmDateID);

    }

    // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
    Cdbg(APP_DBG, "%s", dbg_msg);

    // check -> mac, alarm id, alarm date id
    if( (tempAiCamMacID > 1000) && 
        (tempAiCamMacAlarmID > 1000) && 
        (tempAiCamMacAlarmDateID > 1000)) {

        return;

    // check -> alarm id, alarm date id
    } else if((tempAiCamMacID > 1000) && (tempAiCamMacAlarmID <= 1000)) {


        tempAiCamMacAlarmID = createWebStorageMacAlarmDir(mac_name, mac_type);

        // 2 -> get mac/alarmID type -> number
        //tempAiCamMacAlarmID = getAiCamTempID(mac_type, 2);

        if(tempAiCamMacAlarmID > 1000) {

            createWebStorageMacAlarmDateDir(mac_name, mac_type);
        }

    // check -> alarm date id
    } else if((tempAiCamMacID > 1000) && 
              (tempAiCamMacAlarmID > 1000) && 
              (tempAiCamMacAlarmDateID <= 1000)) {

        createWebStorageMacAlarmDateDir(mac_name, mac_type);

    } else {

        // check(create) mac001~004 dir && aicamDeviceMacId
        // get macID
        tempAiCamMacID = createWebStorageMacDir(mac_name, mac_type);

        // 1 -> get macID type -> number
        //tempAiCamMacID = getAiCamTempID(mac_type, 1);

        if(tempAiCamMacID > 1000) {

            tempAiCamMacAlarmID = createWebStorageMacAlarmDir(mac_name, mac_type);

            // 2 -> get mac/alarmID type -> number
            //tempAiCamMacAlarmID = getAiCamTempID(mac_type, 2);

            if(tempAiCamMacAlarmID > 1000) {
                createWebStorageMacAlarmDateDir(mac_name, mac_type);
            }
        }
    }

}




void getMacRecordDateID(char * mac_name, int mac_type) {

    long long tempAiCamMacID = 0;

    long long tempAiCamMacRecordID = 0;
    long long tempAiCamMacRecordDateID = 0;

    if(mac_type == 1) {

        tempAiCamMacID = aiCamMacAddressOneID;

        tempAiCamMacRecordID = aiCamMacAddressOneRecordID;
        tempAiCamMacRecordDateID = aiCamMacAddressOneRecordDateID;

        snprintf(dbg_msg, sizeof(dbg_msg), "Device Mac One ->  Name = %s , Mac ID = %lli / Record ID = %lli / Record Date ID = %lli", mac_name, tempAiCamMacID, tempAiCamMacRecordID, tempAiCamMacRecordDateID);

    } else if(mac_type == 2) {

        tempAiCamMacID = aiCamMacAddressTwoID;

        tempAiCamMacRecordID = aiCamMacAddressTwoRecordID;
        tempAiCamMacRecordDateID = aiCamMacAddressTwoRecordDateID;

        snprintf(dbg_msg, sizeof(dbg_msg), "Device Mac Two ->  Name = %s , Mac ID = %lli / Record ID = %lli / Record Date ID = %lli", mac_name, tempAiCamMacID, tempAiCamMacRecordID, tempAiCamMacRecordDateID);

    } else if(mac_type == 3) {

        tempAiCamMacID = aiCamMacAddressThreeID;


        tempAiCamMacRecordID = aiCamMacAddressThreeRecordID;
        tempAiCamMacRecordDateID = aiCamMacAddressThreeRecordDateID;

        snprintf(dbg_msg, sizeof(dbg_msg), "Device Mac Three ->  Name = %s , Mac ID = %lli / Record ID = %lli / Record Date ID = %lli", mac_name, tempAiCamMacID, tempAiCamMacRecordID, tempAiCamMacRecordDateID);

    } else if(mac_type == 4) {

        tempAiCamMacID = aiCamMacAddressFourID;

        tempAiCamMacRecordID = aiCamMacAddressFourRecordID;
        tempAiCamMacRecordDateID = aiCamMacAddressFourRecordDateID;

        snprintf(dbg_msg, sizeof(dbg_msg), "Device Mac Four ->  Name = %s , Mac ID = %lli / Record ID = %lli / Record Date ID = %lli", mac_name, tempAiCamMacID, tempAiCamMacRecordID, tempAiCamMacRecordDateID);

    }

    // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
    Cdbg(APP_DBG, "%s", dbg_msg);

    // check -> mac, record id, record date id
    if( (tempAiCamMacID > 1000) && 
        (tempAiCamMacRecordID > 1000) && 
        (tempAiCamMacRecordDateID > 1000)) {

        return;

    // check -> Record id, Record date id
    } else if((tempAiCamMacID > 1000) && (tempAiCamMacRecordID <= 1000)) {


        tempAiCamMacRecordID = createWebStorageMacRecordDir(mac_name, mac_type);

        // 2 -> get mac/RecordID type -> number
        //tempAiCamMacRecordID = getAiCamTempID(mac_type, 2);

        if(tempAiCamMacRecordID > 1000) {

            createWebStorageMacRecordDateDir(mac_name, mac_type);
        }

    // check -> Record date id
    } else if((tempAiCamMacID > 1000) && 
              (tempAiCamMacRecordID > 1000) && 
              (tempAiCamMacRecordDateID <= 1000)) {

        createWebStorageMacRecordDateDir(mac_name, mac_type);

    } else {

        // check(create) mac001~004 dir && aicamDeviceMacId
        tempAiCamMacID = createWebStorageMacDir(mac_name, mac_type);

        // 1 -> get macID type -> number
        // tempAiCamMacID = getAiCamTempID(mac_type, 1);

        if(tempAiCamMacID > 1000) {

            tempAiCamMacRecordID = createWebStorageMacRecordDir(mac_name, mac_type);

            // 2 -> get mac/RecordID type -> number
            // tempAiCamMacRecordID = getAiCamTempID(mac_type, 2);

            if(tempAiCamMacRecordID > 1000) {
                createWebStorageMacRecordDateDir(mac_name, mac_type);
            }
        }
    }

}


// mac type -> 1: mac one, 2: mac two, 3: mac three, 4: mac four
// process type -> 1:mac id, 2: alarm id, 3:date id
int getAiCamTempID(int mac_type, int process_type) {

    long long tempAiCamMacID = 0;
    long long tempAiCamMacAlarmID = 0;
    long long tempAiCamMacAlarmDateID = 0;

    if(mac_type == 1) {


        tempAiCamMacID = aiCamMacAddressOneID;
        tempAiCamMacAlarmID = aiCamMacAddressOneAlarmID;
        tempAiCamMacAlarmDateID = aiCamMacAddressOneAlarmDateID;

    } else if(mac_type == 2) {

        tempAiCamMacID = aiCamMacAddressTwoID;
        tempAiCamMacAlarmID = aiCamMacAddressTwoAlarmID;
        tempAiCamMacAlarmDateID = aiCamMacAddressTwoAlarmDateID;

    } else if(mac_type == 3) {

        tempAiCamMacID = aiCamMacAddressThreeID;
        tempAiCamMacAlarmID = aiCamMacAddressThreeAlarmID;
        tempAiCamMacAlarmDateID = aiCamMacAddressThreeAlarmDateID;

    } else if(mac_type == 4) {

        tempAiCamMacID = aiCamMacAddressFourID;
        tempAiCamMacAlarmID = aiCamMacAddressFourAlarmID;
        tempAiCamMacAlarmDateID = aiCamMacAddressFourAlarmDateID;

    }

    // return mac type || alarm type || date type
    if(process_type == 1) {         return tempAiCamMacID;
    } else if(process_type == 2) {  return tempAiCamMacAlarmID;
    } else if(process_type == 3) {  return tempAiCamMacAlarmDateID;
    } else {
        return -1;
    }

}



void search_video_dir() {

    snprintf(dbg_msg, sizeof(dbg_msg), "111111  aiCamMacAddressOneID : %lli", aiCamMacAddressOneID);
    // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
    Cdbg(APP_DBG, "%s", dbg_msg);

    snprintf(dbg_msg, sizeof(dbg_msg), "111111  aiCamMacAddressOneAlarmID : %lli", aiCamMacAddressOneAlarmID);
    // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
    Cdbg(APP_DBG, "%s", dbg_msg);

    snprintf(dbg_msg, sizeof(dbg_msg), "111111  aiCamMacAddressOneAlarmDateID : %lli", aiCamMacAddressOneAlarmDateID);
    // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
    Cdbg(APP_DBG, "%s", dbg_msg);

    snprintf(dbg_msg, sizeof(dbg_msg), "111111  aiCamMacAddressOne : %s\n", aiCamMacAddressOne);
    // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
    Cdbg(APP_DBG, "%s", dbg_msg);

    // search rule : alarm first , then record
    // Mac One : alarm video search
    if((aiCamMacAddressOneID > 1000) &&  (aiCamMacAddressOneAlarmID > 1000) &&  (aiCamMacAddressOneAlarmDateID > 1000)) {
        if(strstr(aiCamMacAddressOne, "AiCAM_")) {
            search_video("alarm", aiCamMacAddressOne, MAC_ONE);
        }
    }

    // Mac One : record video search
    if(strstr(macVideoModeOne, TIME_CONTINOUS_7DAYS)) {

        if((aiCamMacAddressOneID > 1000) &&  (aiCamMacAddressOneRecordID > 1000) &&  (aiCamMacAddressOneRecordDateID > 1000)) {

            if(strstr(aiCamMacAddressOne, "AiCAM_")) {
                search_video("record", aiCamMacAddressOne, MAC_ONE);
            }
        }
    }



        snprintf(dbg_msg, sizeof(dbg_msg), "222222  aiCamMacAddressTwoID : %lli", aiCamMacAddressTwoID);
        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(APP_DBG, "%s", dbg_msg);

        snprintf(dbg_msg, sizeof(dbg_msg), "222222  aiCamMacAddressTwoAlarmID : %lli", aiCamMacAddressTwoAlarmID);
        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(APP_DBG, "%s", dbg_msg);

        snprintf(dbg_msg, sizeof(dbg_msg), "222222  aiCamMacAddressTwoAlarmDateID : %lli", aiCamMacAddressTwoAlarmDateID);
        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(APP_DBG, "%s", dbg_msg);


        snprintf(dbg_msg, sizeof(dbg_msg), "222222  aiCamMacAddressTwo : %s\n", aiCamMacAddressTwo);
        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(APP_DBG, "%s", dbg_msg);

    // Mac Two : alarm video search
    if((aiCamMacAddressTwoID > 1000) && (aiCamMacAddressTwoAlarmID > 1000) && (aiCamMacAddressTwoAlarmDateID > 1000)) {

        if(strstr(aiCamMacAddressTwo, "AiCAM_")) {
            search_video("alarm", aiCamMacAddressTwo, MAC_TWO);
        }
    }

    // Mac Two : record video search
    if(strstr(macVideoModeTwo, TIME_CONTINOUS_7DAYS)) {

        if((aiCamMacAddressTwoID > 1000) &&  (aiCamMacAddressTwoRecordID > 1000) &&  (aiCamMacAddressTwoRecordDateID > 1000)) {

            snprintf(dbg_msg, sizeof(dbg_msg), "222222  aiCamMacAddressTwoRecordID : %lli", aiCamMacAddressTwoRecordID);
            // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
            Cdbg(APP_DBG, "%s", dbg_msg);

            snprintf(dbg_msg, sizeof(dbg_msg), "222222  aiCamMacAddressTwoRecordDateID : %lli", aiCamMacAddressTwoRecordDateID);
            // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
            Cdbg(APP_DBG, "%s", dbg_msg);

            if(strstr(aiCamMacAddressTwo, "AiCAM_")) {
                search_video("record", aiCamMacAddressTwo, MAC_TWO);
            }
        }
    }

        snprintf(dbg_msg, sizeof(dbg_msg), "333333  aiCamMacAddressThreeID : %lli", aiCamMacAddressThreeID);
        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(APP_DBG, "%s", dbg_msg);

        snprintf(dbg_msg, sizeof(dbg_msg), "333333  aiCamMacAddressThreeAlarmID : %lli", aiCamMacAddressThreeAlarmID);
        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(APP_DBG, "%s", dbg_msg);

        snprintf(dbg_msg, sizeof(dbg_msg), "333333  aiCamMacAddressThreeAlarmDateID : %lli", aiCamMacAddressThreeAlarmDateID);
        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(APP_DBG, "%s", dbg_msg);


        snprintf(dbg_msg, sizeof(dbg_msg), "333333  aiCamMacAddressThree : %s\n", aiCamMacAddressThree);
        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(APP_DBG, "%s", dbg_msg);
    
    // Mac Three : alarm video search
    if((aiCamMacAddressThreeID > 1000) && (aiCamMacAddressThreeAlarmID > 1000) && (aiCamMacAddressThreeAlarmDateID > 1000)) {

        if(strstr(aiCamMacAddressThree, "AiCAM_")) {
           search_video("alarm", aiCamMacAddressThree, MAC_THREE);
        }
    }


    // Mac Three : record video search
    if(strstr(macVideoModeThree, TIME_CONTINOUS_7DAYS)) {

        if((aiCamMacAddressThreeID > 1000) &&  (aiCamMacAddressThreeRecordID > 1000) &&  (aiCamMacAddressThreeRecordDateID > 1000)) {

            snprintf(dbg_msg, sizeof(dbg_msg), "333333  aiCamMacAddressThreeRecordID : %lli", aiCamMacAddressThreeRecordID);
            // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
            Cdbg(APP_DBG, "%s", dbg_msg);

            snprintf(dbg_msg, sizeof(dbg_msg), "333333  aiCamMacAddressThreeRecordDateID : %lli", aiCamMacAddressThreeRecordDateID);
            // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
            Cdbg(APP_DBG, "%s", dbg_msg);

            if(strstr(aiCamMacAddressThree, "AiCAM_")) {
                search_video("record", aiCamMacAddressThree, MAC_THREE);
            }
        }
    }

        snprintf(dbg_msg, sizeof(dbg_msg), "444444  aiCamMacAddressFourID : %lli", aiCamMacAddressFourID);
        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(APP_DBG, "%s", dbg_msg);

        snprintf(dbg_msg, sizeof(dbg_msg), "444444  aiCamMacAddressFourAlarmID : %lli", aiCamMacAddressFourAlarmID);
        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(APP_DBG, "%s", dbg_msg);

        snprintf(dbg_msg, sizeof(dbg_msg), "444444  aiCamMacAddressFourAlarmDateID : %lli", aiCamMacAddressFourAlarmDateID);
        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(APP_DBG, "%s", dbg_msg);


        snprintf(dbg_msg, sizeof(dbg_msg), "444444  aiCamMacAddressFour : %s\n", aiCamMacAddressFour);
        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(APP_DBG, "%s", dbg_msg);
    
    // Mac Four : alarm video search
    if((aiCamMacAddressFourID > 1000) && (aiCamMacAddressFourAlarmID > 1000) && (aiCamMacAddressFourAlarmDateID > 1000)) {

        if(strstr(aiCamMacAddressFour, "AiCAM_")) {
           search_video("alarm", aiCamMacAddressFour, MAC_FOUR);
        }
    }


    // Mac Four : record video search
    if(strstr(macVideoModeFour, TIME_CONTINOUS_7DAYS)) {

        if((aiCamMacAddressFourID > 1000) &&  (aiCamMacAddressFourRecordID > 1000) &&  (aiCamMacAddressFourRecordDateID > 1000)) {


            snprintf(dbg_msg, sizeof(dbg_msg), "444444  aiCamMacAddressFourRecordID : %lli", aiCamMacAddressFourRecordID);
            // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
            Cdbg(APP_DBG, "%s", dbg_msg);

            snprintf(dbg_msg, sizeof(dbg_msg), "444444  aiCamMacAddressFourRecordDateID : %lli", aiCamMacAddressFourRecordDateID);
            // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
            Cdbg(APP_DBG, "%s", dbg_msg);



            if(strstr(aiCamMacAddressFour, "AiCAM_")) {
                search_video("record", aiCamMacAddressFour, MAC_FOUR);
            }
        }
    }



    // snprintf(dbg_msg, sizeof(dbg_msg), "Dir mp4 file search pthread start");
    // debugLogLevel(DEBUG_LEVEL, DBG_INFO, dbg_msg);


    // char time_now[26];

    // time_t t = time(0);

    // memset(time_now,0, sizeof(time_now));
    // strftime( time_now, sizeof(time_now), "%Y-%m-%d %X",localtime(&t) );  //:%A:%S


    // while(1) {


    //     // process aaews txt file context
    //     // Cdbg(APP_DBG, "start : process event file content");

    //     process_aaews_event_config();

    //     //Cdbg(APP_DBG, "end : process event file content");


    //     // Cdbg(APP_DBG, "alarm Dir search mp4 file start time");

    //     video_upload("alarm");

    //     //Cdbg(APP_DBG, "WebStorage : alarm Dir search mp4 file end.\n\n");


    //     // The user has a purchase webstorage service
    //     if( (space_plan == TIME_CONTINOUS_PLAN) || 
    //         (upload_type == RECORD_AICLOUD_ALARM_ASUS) ) {

    //         //Cdbg(APP_DBG, "TIME_CONTINOUS_PLAN : Record Dir search mp4 file start. upload_type = %d", upload_type);

    //         video_upload("record");

    //         //Cdbg(APP_DBG, "TIME_CONTINOUS_PLAN : record Dir search mp4 file end, waiting 5s.");

    //     } else {

    //         //Cdbg(APP_DBG, "SPACE_SIZE_PLAN : tcode_channel = %s, upload_type = %d , (!= record && aiCloud )", tcode_channel, upload_type);

    //         // del record dir file
    //         dir_files_del("record");


    //     }

    //     sleep(5);
    // }

}

int GetToken(char * des) {

    //Cdbg(APP_DBG, "Start OAuth : OAuth data dp_conf.aaeSid : %s", dp_conf.aaeSid);
    Cdbg(APP_DBG, "Start OAuth ");

    sergate.status = 0;

    char* gateway_domain = str_replace(dp_conf.awsAppGateway, "https://","");  

    strcpy(sergate.gateway, gateway_domain);
    strcpy(aaa.service_gateway, gateway_domain);
    strcpy(aaa.gateway, gateway_domain);

    free(gateway_domain);

    Cdbg(APP_DBG, "OAuth -> aaa.service_gateway = %s", aaa.service_gateway);

    // Using OAuth token, through the getsgtoken api to exchange asuswebstorage token
    // sure getSgToken OK
    while(1) {

        Cdbg(APP_DBG, "Start getSgToken API connecting");

        int getSgTokenStatus = getSgToken(&aaa, &ap_conf);

        if(getSgTokenStatus == 0) {
            Cdbg(APP_DBG, "Get SgToken success");
            break;
        } else {
            Cdbg(APP_DBG, "Get SgToken fail, return status = %d", getSgTokenStatus);
        }
    }

    if(aaa.status != 0) {

        Cdbg(APP_DBG, "Get SgToken Api status %d :", aaa.status);
         
        
        char error_msg[32];
        memset(error_msg,0, sizeof(error_msg));
        strcpy(error_msg, "GetToken:");
        strcat(error_msg, des);

        int tsdb_status = tsdbProcess(aaa.status, error_msg);

        return tsdb_status;
    }

    strcpy(aaa.user, username);
    strcpy(aaa.pwd, password);

        /*
        aaa.status = 0;
        //strcpy(sergate.gateway, "sgb02.asuswebstorage.com");
        strcpy(aaa.token, "yy0cjason09b77");
        //strcpy(aaa.token, "8afaibasfjou2137");
        strcpy(aaa.contentrelay, "s02.asusvibe.com");
        strcpy(aaa.filerelay, "127.0.0.1");
        strcpy(aaa.inforelay, "irb02.asuswebstorage.com");
        strcpy(aaa.jobrelay, "jr02.asuswebstorage.com");
        strcpy(aaa.rssrelay, "208.74.76.171");
        strcpy(aaa.searchrelay, "ss02.asuswebstorage.com");
        strcpy(aaa.webrelay, "wrb02.asuswebstorage.com");
        strcpy(aaa.auxpasswordurl, "");
        strcpy(aaa.gateway, "sgb01.asuswebstorage.com");
        strcpy(aaa.user, "deanli.asus@gmail.com");
        strcpy(aaa.pwd, "1c63129ae9db9c60c3e8aa94d3e00495");
        */

    aaa.package.id = 0;
        
    strcpy(aaa.package.display, "");
    aaa.package.capacity = 0;
    aaa.package.uploadbandwidth = 0;


    return aaa.status;
}



int routerRun()
{


    int status = 0;

    // get new access token
    // status = get_google_token();
    do {

        status = get_google_token();

        if(status != 0) {
            Cdbg(APP_DBG, "Refresh token error, sleep 30s");
            sleep(30);
        }

    } while ( status != 0);


    // if(status == -1) {
    //     Cdbg(APP_DBG, "Refresh token error, exit uploader");
    //     exit(-1);
    // }


    do {

        status = googleDriveFolderSearch(ASUS_FOLDER_NAME, NULL);


        if(status != 0) {
            Cdbg(APP_DBG, "Get asus_folder_id error, sleep 20s");
            sleep(20);
        }


    } while ( status != 0);

    Cdbg(APP_DBG, "Get asus_folder_id : %s\n", google_d_conf.asus_folder_id);

    do {

        status = googleDriveFolderSearch(ROUTER_ROUTER_NAME, google_d_conf.asus_folder_id);

        if(status != 0) {
            Cdbg(APP_DBG, "Get router_folder_id error, sleep 20s");
            sleep(20);
        }

    } while ( status != 0);

    Cdbg(APP_DBG, "Get router_folder_id : %s", google_d_conf.router_folder_id);

    do {

        status = googleDriveFolderSearch(LOG_FOLDER_NAME, google_d_conf.router_folder_id);


        if(status != 0) {
            Cdbg(APP_DBG, "Get log_folder_id error, sleep 20s");
            sleep(20);
        }

    } while ( status != 0);

    Cdbg(APP_DBG, "Get log_folder_id : %s", google_d_conf.log_folder_id);

    time_t t = time(0);
    memset(time_now,0, sizeof(time_now));

    strftime( time_now, sizeof(time_now), "%Y%m%d",localtime(&t) );

    Cdbg(APP_DBG, "routerRun time_now : %s", time_now);

    do {

        status = googleDriveFolderSearch(time_now, google_d_conf.log_folder_id);

        if(status != 0) {
            Cdbg(APP_DBG, "Get date_folder_id error, sleep 20s");
            sleep(20);
        }

    } while ( status != 0);


    Cdbg(APP_DBG, "Get date_folder_id : %s", google_d_conf.date_folder_id);


    pthread_t dir_search_id;
    int alarm_dir_ret;
    alarm_dir_ret = pthread_create(&dir_search_id,NULL,(void *) dir_search_thread,NULL);
    if(alarm_dir_ret!=0)
    {
        Cdbg(APP_DBG, "Create dir_search_thread error!\n");
        return;
    }

        
    pthread_join(dir_search_id,NULL); //等待線程（pthread）結束


    return;

}


int aiCamRun()
{


    initAiCamConfig();

    Cdbg(APP_DBG, "System token file tmp path -> %s", system_token);

    restart_run = 0;
    int status;
    int auth_ok = 0;

    strcpy(username,ap_conf.account);
    strcpy(password,ap_conf.password);

    char *pwd = NULL;
    char pword[PASSWORD_SIZE];

    strcpy(pword,password);
    str_to_lower(pword);

    pwd = MD5_string(pword);

    if(pwd == NULL) {
        return MD5_PASSWORD_NULL;
    }

    memset(password,0,sizeof(password));
    snprintf(password, PASSWORD_SIZE, "%s",pwd);
    my_free(pwd);


    // if(strcmp(password, "") != 0) {

    //     Cdbg(APP_DBG, "Password not null, aicam first start, start initDevice start");

    //     char *pwd = NULL;
    //     char pword[PASSWORD_SIZE];

    //     strcpy(pword,password);
    //     str_to_lower(pword);

    //     pwd = MD5_string(pword);

    //     if(pwd == NULL) {
    //         return MD5_PASSWORD_NULL;
    //     }

    //     memset(password,0,sizeof(password));
    //     snprintf(password, PASSWORD_SIZE, "%s",pwd);
    //     my_free(pwd);


    //     // Step 2 : process init AiCam Device
    //     int init_device_status;
    //     init_device_status = initDevice();

    //     Cdbg(APP_DBG, "init_device_status = %d", init_device_status);

    //     //if((init_device_status == DEVICE_AUTH_FAIL) || (init_device_status == S_GER_ERR)) {
    //     if(init_device_status != 0) {
    //         tsdbProcess(init_device_status, "init_device:DEVICE AUTH FAIL");
    //         return DEVICE_AUTH_FAIL;
    //     }

    //     Cdbg(APP_DBG, "initDevice end");

    //     // Step 3 : init device success, aaewsProvision config content -> password = "" ,
    //     // Indicates that the device is initialized
    //     if(init_device_status == 0) {
    //         aaewsProvisionEdit(&ap_conf);
    //         Cdbg(APP_DBG, "write aaewsProvision content -> password = ''");
    //     }
        

    // } else {

    //     Cdbg(APP_DBG, "password is null, start OAuth");

    //     if( (strcmp(dp_conf.aaeSid, ASUS_TOKEN_SID) == 0)) {

    //         strcpy(cfg.pwd, ap_conf.token);
    //         strcpy(asus_conf.password, ap_conf.token);

    //         char *pwd = NULL;
    //         char pword[PASSWORD_SIZE];

    //         strcpy(pword, ap_conf.token);
    //         str_to_lower(pword);

    //         pwd = MD5_string(pword);
    //         if(pwd == NULL) {
    //             return MD5_PASSWORD_NULL;
    //         }

    //         memset(password,0,sizeof(password));
    //         snprintf(password, PASSWORD_SIZE, "%s",pwd);
    //         my_free(pwd);

    //     }

    // }

    // // is oauth token
    // if( (strcmp(dp_conf.aaeSid, ASUS_TOKEN_SID) == 0)) {
    //     oauth_token = 0;
    // } else {
    //     oauth_token = 1;
    // }

    // Cdbg(APP_DBG, "username is %s,pwd is %s",username,password);
    // Cdbg(APP_DBG, "sync path is %s", sync_path);



    // // browseAttachableDevice API , return type 0 : status , 1: space plan diff
    // int browseAttachableDeviceStatus = browseAttachableDevice(username, &ap_conf, 0);

    // Cdbg(APP_DBG, "browseAttachableDevice = %d\n", browseAttachableDeviceStatus);

    // if(browseAttachableDeviceStatus == S_AUTH_FAIL) {

    //     return S_AUTH_FAIL;

    // // state = 2
    // } else if(browseAttachableDeviceStatus == DEVICE_EXPIRED) {

    //     Cdbg(APP_DBG, "device_id : [ %s ] EXPIRED", asus_conf.device_id);
    //     Cdbg(APP_DBG, "uploader end");
    //     exit(0);
    //     //return DEVICE_EXPIRED;
    // }


    // if(space_plan == TIME_CONTINOUS_PLAN) {

    //     Cdbg(APP_DBG, "space_plan = %d, TIME_CONTINOUS_PLAN", space_plan);

    //     // webstorage dir , record = mac , alarm = V2
    //     strcpy(aicam_device_dir, asus_conf.mac_name);

    // } else {
    //     Cdbg(APP_DBG, "space_plan = %d, SPACE_SIZE_PLAN", space_plan);
    // }
    
    // // space_plan_tmp save
    // space_plan_tmp = space_plan;


    memset(&sergate,0,sizeof(Servicegateway));
    memset(&aaa,0,sizeof(Aaa));


    strcpy(dp_conf.aaeSid, "1002");
    // // not OAuth data
    if( (strcmp(dp_conf.aaeSid, ASUS_TOKEN_SID) == 0)) {


        Cdbg(APP_DBG, "Not OAuth data -> dp_conf.aaeSid : %s", ASUS_TOKEN_SID);
        Cdbg(APP_DBG, "check webstorage token files : %s", system_token);

        if(obtain_token_from_file(system_token,&aaa) == -1)
        {
            Cdbg(APP_DBG, "webstorage token file not exist");

            if(obtainGateway(username,password,&sergate) == -1) {
                Cdbg(APP_DBG, "obtainGateway fail, status -1");
                return S_AUTH_FAIL;
            }

            Cdbg(APP_DBG, "getServiceGateway ok");

            if(obtainToken(username,password,&cfg,1) == -1) {
                Cdbg(APP_DBG, "obtainToken status -1, token auth fail");
                return TOKEN_AUTH_FAIL;
            }

            Cdbg(APP_DBG, "GetToken ok");
        } else { 
            Cdbg(APP_DBG, "webstorage token file exist");
        }

    } else {

        int toke_status = GetToken("OAuth Start");

    }

    //int toke_status = GetToken("OAuth Start");

    Cdbg(APP_DBG, "sergate.status   : %d", sergate.status);

    Cdbg(APP_DBG, "sergate.gateway   : %s", sergate.gateway);
    Cdbg(APP_DBG, "aaa.status   : %d", aaa.status);
    
    if(aaa.status  == S_PARAM_OUT_RANGE) {
        Cdbg(APP_DBG, "param out rage");
        return S_PARAM_OUT_RANGE;
    }

    Cdbg(APP_DBG, "aaa.token   : %s", aaa.token);
    Cdbg(APP_DBG, "aaa.contentrelay   : %s", aaa.contentrelay);
    Cdbg(APP_DBG, "aaa.filerelay   : %s", aaa.filerelay);
    Cdbg(APP_DBG, "aaa.inforelay   : %s", aaa.inforelay);
    Cdbg(APP_DBG, "aaa.jobrelay   : %s", aaa.jobrelay);
    Cdbg(APP_DBG, "aaa.rssrelay   : %s", aaa.rssrelay);
    Cdbg(APP_DBG, "aaa.searchrelay   : %s", aaa.searchrelay);
    Cdbg(APP_DBG, "aaa.webrelay   : %s", aaa.webrelay);
    Cdbg(APP_DBG, "aaa.auxpasswordurl   : %s", aaa.auxpasswordurl);
    Cdbg(APP_DBG, "aaa.gateway   : %s", aaa.gateway);
    Cdbg(APP_DBG, "aaa.user   : %s", aaa.user);
    Cdbg(APP_DBG, "aaa.pwd   : %s", aaa.pwd);
    Cdbg(APP_DBG, "aaa.package.id   : %d", aaa.package.id);
    Cdbg(APP_DBG, "aaa.package.display   : %s", aaa.package.display);
    Cdbg(APP_DBG, "aaa.package.capacity   : %d", aaa.package.capacity);
    Cdbg(APP_DBG, "aaa.package.uploadbandwidth   : %d", aaa.package.uploadbandwidth);

    auth_ok = 1;

     
/*
    Propfind *pfind;

    pfind = checkEntryExisted(username, -22, "AiCAM","system.folder");
    //pfind = checkEntryExisted(username, -22, "AiCAM","system.unknown");
*/
    // Browse *browse = NULL;;

    // browse = browseFolder(username, 336717617,0,1);
    // // browse = browseFolder(username,-22,0,1);

    // return -1;
/*
    // AiCAM
    browse = browseFolder(username,128625106,0,1);

    // AiCAM/AiCAM_E6C752/
    browse = browseFolder(username,147819171,0,1);

    // AiCAM/AiCAM_E6C752/alert
    // AiCAM/AiCAM_E6C752/video
    // AiCAM/AiCAM_E6C752/tmp_E6C752
    browse = browseFolder(username,147819177,0,1);


    
    
    Operateentry *orename = NULL;

    orename = renameEntry(username, 1618018276, 0, "20160501155722VO#duration_120.596000.mp4", 0);
    orename = renameEntry(username, 1618081170, 0, "20160501155922VO#duration_120.596000.mp4", 0);
    //orename = renameEntry(username, 147819177, 0, "video", 1);  //isfolder

    return;



    //MySyncFolder = obtainSyncRootID(username);

    //Cdbg(APP_DBG, "getMySyncFolder : %s \n", MySyncFolder);
return;
*/


/*
    // markcool : if uploader AiCam , not need obtain obtainSyncRootID
   MySyncFolder = obtainSyncRootID(username);
   if(MySyncFolder == -1)
       return ;

   Cdbg(APP_DBG, "MySyncFolder is %d\n",MySyncFolder);

#ifdef DEBUG
    Cdbg(APP_DBG, "getMySyncFolder ok\n");
#endif
*/

    /*  
    max_upload_filesize = get_max_upload_filesize(username);
    if(max_upload_filesize == -1)
        return ;
    */

    max_upload_filesize = 1000;

/*
    status = CheckUserState(username,sergate.gateway);
    if(status == -1 || status == S_ACCOUNT_CLOSE)
           return;

    if(status == S_ACCOUNT_FROZEN)
         IsAccountFrozen = 1;

    MyRecycleID = GetMyRecycleID(username,"MyRecycleBin");
    if(MyRecycleID == -1)
        return;
*/

    //Cdbg(APP_DBG, "##### max upload filesize is %dMB ####\n",max_upload_filesize);


    if(auth_ok == 1)
    {


        // process fb stmp (get video)
        /*
        pthread_t rtmp_id;
        int rtmp_ret;
        rtmp_ret = pthread_create(&rtmp_id,NULL,(void *) fbRtmpThread,NULL);
        if(rtmp_ret != 0)
        {
            printf ("fbRtmpThread : Create pthread error!\n");
        }
        */


        int aicam_dir_status;

        // aicam_dir_status = aicam_dir_check();
        aicam_dir_status = create_aicam_dir(); //aicam_dir_check();
         
        if(aicam_dir_status == -1) {
            Cdbg(APP_DBG, "Get aiCam Dir error");
            return;
        }

/*
#if SERVER_TREE
    Cdbg(APP_DBG, "######enable SERVER_TREE ######\n");
        SRootNode = create_server_treeroot(gf->id);
        browse_to_tree(username,gf->id,browse_folder_xml,SRootNode);
        SearchServerTree(SRootNode);
#endif
*/
        //create mem pool
#if MEM_POOL_ENABLE
        mem_pool_init();
        Cdbg(APP_DBG, "######enable mem  pool######\n");
#endif

        from_server_sync_head = create_head();
        up_head = create_head();
        down_head =  create_head();
        up_excep_fail = create_head();
        copy_file_list_head = create_head();
        if(download_only == 1)
        {
            download_only_socket_head = create_head();
            download_only_modify_head = create_head();
        }

        if(NULL == from_server_sync_head || NULL == up_head ||
           NULL == up_excep_fail)
            return;

        if(download_only == 1)
        {
            if(NULL == download_only_socket_head || NULL == download_only_modify_head)
                return;
        }


        // modify by markcool : start

        pthread_t dir_search_id;
        int alarm_dir_ret;
        alarm_dir_ret = pthread_create(&dir_search_id,NULL,(void *) dir_search_thread,NULL);
        if(alarm_dir_ret!=0)
        {
            Cdbg(APP_DBG, "Create dir_search_thread error!\n");
            return;
        }

        
        // pthread_join(rtmp_id,NULL); //waiting pthread end

        pthread_join(dir_search_id,NULL); //等待線程（pthread）結束


        return;
    }

}


char  *get_mount_path(char *path , int n)
{
    int i;
    char *m_path = NULL;
    m_path = (char *)malloc(sizeof(char)*NORMALSIZE);
    memset(m_path,0,NORMALSIZE);

    char *new_path = NULL;
    new_path = path;

    for(i= 0;i< n ;i++)
    {
        new_path = strchr(new_path,'/');
        if(new_path == NULL)
            break;
        new_path++;
    }

    if( i > 3)
        strncpy(m_path,path,strlen(path)-strlen(new_path)-1);
    else
        strcpy(m_path,path);

    Cdbg(APP_DBG, "mount path is [%s]\n",m_path);

    return m_path;
}



int read_config()
{
    
    char resume_path[NORMALSIZE];
    upload_only = 0;
    download_only = 0;
    memset(resume_path,0,sizeof(resume_path));

    Cdbg(APP_DBG, "CONFIG_PATH -> %s", CONFIG_PATH);



// markcool modify

/*
#ifdef IPKG 

    if(create_asuswebstorage_conf_file(CONFIG_PATH) == -1)
    {
        Cdbg(APP_DBG, "create_asuswebstorage_conf_file fail\n");
        return;
    }
// start : modify by markcool
#elif defined AICAM
 
        Cdbg(APP_DBG, "run AiCam config file\n");
// end : modify by markcool
#else
    if(convert_nvram_to_file(CONFIG_PATH) == -1)
    {
        Cdbg(APP_DBG, "convert_nvram_to_file fail\n");
        return -1;
    }
#endif

*/
    memset(up_item_file, 0, sizeof(up_item_file));
    memset(down_item_file, 0, sizeof(down_item_file));
#if WRITE_DOWNLOAD_TEMP_FILE
    memset(down_item_temp_file, 0, sizeof(down_item_temp_file));
#endif
    memset(up_excep_fail_file,0,sizeof(up_excep_fail_file));
    memset(confilicted_log,0,sizeof(confilicted_log));
    memset(&cfg,0,sizeof(struct asus_config));

// start : modify by markcool
#if AICAM   

    memset(username, 0, sizeof(username));
    memset(password, 0, sizeof(password));
    memset(sync_path, 0, sizeof(sync_path));

    parse_config_copy(&asus_conf,&cfg);

    Cdbg(APP_DBG, "cfg.type -> %d", cfg.type);

    Cdbg(APP_DBG, "cfg.user -> %s", cfg.user);
    Cdbg(APP_DBG, "cfg.pwd -> %s", cfg.pwd);
    Cdbg(APP_DBG, "cfg.url -> %s", cfg.url);
    Cdbg(APP_DBG, "cfg.rule -> %d", cfg.rule);
    Cdbg(APP_DBG, "cfg.sync_path -> %s", cfg.sync_path);
    Cdbg(APP_DBG, "cfg.enable -> %d", cfg.enable);


#else

    parse_config_new(CONFIG_PATH,&cfg);

#endif
// end : modify by markcool

    if( strlen(username) == 0 )
    {
#ifdef DEBUG
        Cdbg(APP_DBG, "username is blank ,please input your username and passwrod\n");
#endif

#if SYSTEM_LOG
        write_system_log("error","username is blank ,please input your username and passwrod");
#endif
        no_config = 1;
    }

    if(no_config)
        return -1;

    if(!no_config)
    {
        
        parse_sync_path(&cfg,sync_path);
        
        
#ifdef IPKG
        memset(record_token_file,0,sizeof(record_token_file));
        memset(token_filename,0,sizeof(token_filename));
        sprintf(token_filename,".__smartsync_0_%s_%s",username,cfg.dir_name);
        sprintf(record_token_file,"/opt/etc/.smartsync/asuswebstorage_%s",cfg.user);

        int have_log = 0;

        
        sync_disk_removed = check_token_file(&cfg);

        Cdbg(APP_DBG, "disk status is %d \n",sync_disk_removed);
        while(sync_disk_removed == 2) //sync disk unmount
        {
            if(!have_log)
            {
                write_log(S_ERROR,"sync disk unmount","");
                Cdbg(APP_DBG, "write log end\n");
                have_log = 1;
            }
            if(disk_change)
            {
                Cdbg(APP_DBG, "enter check tokenf file\n");
                pthread_mutex_lock(&mutex_socket);
                disk_change = 0;
                pthread_mutex_unlock(&mutex_socket);
                sync_disk_removed = check_token_file(&cfg);
            }
            else
               //enter_sleep_time(1);
                //usleep(1000*300);
                enter_sleep_time(1000*300,NULL);
        }
#endif
            if(my_mkdir_r(sync_path) == -1)
            {
               Cdbg(APP_DBG, "create sync_path=%s fail,asuswebstorage exit\n",sync_path);
               write_log(S_ERROR,"Create sync path fail,please check whether the hard disk is read only mode","");
               exit(-1);
            }

        
            DIR *dir = opendir(mount_path);
            if(NULL == dir)
            {
                Cdbg(APP_DBG, "open %s fail,asuswebstorage exit\n",mount_path);
                exit(-1);
            }
            closedir(dir);

            snprintf(resume_path,NORMALSIZE,"%s/.smartsync/asuswebstorage",mount_path);


            Cdbg(APP_DBG, "resume_path -> %s", resume_path);

            my_mkdir_r(resume_path);

            snprintf(up_item_file,NAMESIZE,"%s/%s_up_item",resume_path,username);
            snprintf(down_item_file,NAMESIZE,"%s/%s_down_item",resume_path,username);
    #if WRITE_DOWNLOAD_TEMP_FILE
            sprintf(down_item_temp_file,"%s/%s_down_temp_item",resume_path,username);
    #endif
            snprintf(up_excep_fail_file,NAMESIZE,"%s/%s_up_excep_fail",resume_path,username);
            snprintf(confilicted_log,NAMESIZE,"%s/confilicted.log",resume_path);
            snprintf(system_token,256,"%s/sys_token",resume_path);

        no_config = 0 ;
        exit_loop = 0;
    }

    return 0;
}


int retry_all_fail_item(int type)
{
    if(upload_only)
        has_socket = 1;
    struct sync_item *point,*p2;
    Transitem *item;
    char content[NORMALSIZE];
    int retry_time = 1;
    int status;
    char path[NORMALSIZE];
    int ok;

    memset(content,0,sizeof(content));
    memset(path,0,sizeof(path));

    if(type == UP_FAIL)
        point = up_head->next;

    while(point != NULL)
    {
        p2 = point;
        point = point->next;

        switch (type)
        {
        case UP_FAIL:
            snprintf(content,NORMALSIZE,"%s,%s",p2->action,p2->name);
            item = parse_trans_item_from_buffer(content,UPLOAD);
            struct stat buf;

            if(NULL == item)
            {
                return -1;
            }

            if(item->id > 0)
            {
                if( stat(item->name,&buf) == -1)
                {
                    Cdbg(APP_DBG, " %s file does not exist\n",item->name);
                    memset(content,0,sizeof(content));
                    strncpy(content,p2->name,NORMALSIZE);
                    del_sync_item(p2->action,p2->name,up_head); 
                    del_sync_item("up_excep_fail",content,up_excep_fail);
                    my_free(item);
                    return -1;
                }
#ifdef DEBUG
                Cdbg(APP_DBG, "##### handle upload fail file is %s,parentid id %d,transid is %s#####\n",item->name,item->id,item->transid);
#endif
                while(retry_time)
                {
                    //2015/1/9 fix file can't upload when transe id is not exist,set trans_id=NULL
                    status = uploadFile(item->name,item->id,NULL,0);
                    if(status != 0)
                    {
                        char *p = strrchr(item->name,'/');
                        if(p)
                        {
                            strncpy(path,item->name,strlen(item->name)-strlen(p));
                            status = handle_upload_fail_code(status,item->id,item->name,path);
                        }
                    }
                    if(status == 0)
                    {
                        ok = 1;
                        //del_sync_item(p2->action,p2->name,up_head);
                        break;
                    }
                    else
                    {
                        retry_time--;
                    }
                }
            }
            else
            {
                del_sync_item(p2->action,p2->name,up_head);
            }
            my_free(item);
            break;
#if 0
        case DOWN_FAIL:
            snprintf(content,NORMALSIZE,"%s,%s",p2->action,p2->name);
            item = parse_trans_item_from_buffer(content,DOWNLOAD);

            if(item->id > 0)
            {
#ifdef DEBUG
                //Cdbg(APP_DBG, "##### download fail file is %s#####\n",item->name);
                //Cdbg(APP_DBG, "##### download arg id=%d,name=%s,size=%lld #####\n",item->id,item->name,item->size);
#endif

                filename = parse_name_from_path(item->name);

                if(NULL == filename)
                {
                    Cdbg(APP_DBG, "parse %s filenam fail\n",item->name);
                    my_free(item);
                    return -1;
                }

                //Cdbg(APP_DBG, "download filenam is %s\n",filename);

                strncpy(path,item->name,strlen(item->name)-strlen(filename)-1);


                parentID = getParentID(path);

                if(parentID <= 0)
                {
                    Cdbg(APP_DBG, "obtain %s parent ID is fai\n",item->name);
                    my_free(filename);
                    my_free(item);
                    return -1;
                }

                find = checkEntryExisted(username,parentID,filename,"system.file");

                if(NULL == find)
                {
                    Cdbg(APP_DBG, "find prop failed\n");
                    my_free(filename);
                    my_free(item);
                    return -1;
                }
                else if(find->status != 0)
                {
                    handle_error(find->status,"profind");
                    my_free(filename);
                    my_free(item);
                    my_free(find);
                    return -1;
                }
                else if(!strcmp(find->type,"system.notfound"))
                {
                    del_sync_item(p2->action,p2->name,down_head);
                    my_free(filename);
                    my_free(item);
                    my_free(find);
                    return 0;
                }
                else if(!strcmp(find->type,"system.file"))
                {
                    if( find->status != 0 )
                    {
                        handle_error(find->status,"propfind");
                        my_free(find);
                        my_free(item);
                        return -1;
                    }

                    //Cdbg(APP_DBG, "prppfind status=%d,type=%s\n",find->status,find->type);

                    if(!strncmp(find->type,"system.file",sizeof("system.file")))
                    {
                        while(retry_time)
                        {
                            status = downloadFile(item->id,item->name,item->size,0);
                            if(status == 0)
                            {
                                ok = 1;
                                break;
                            }
                            else
                            {
                                Cdbg(APP_DBG, "##### download %s fail #####\n",item->name);
                                retry_time--;
                            }
                        }
                    }
                }

                my_free(filename);
                my_free(item);
                my_free(find);
            }
            break;
#endif
              case SOCKET_FAIL:
            break;
              default:
            break;
        }
    }
    return 0;
}

void handle_fail_item()
{
#if 0
    if(!item_empty())
    {
#ifdef DEBUG
        Cdbg(APP_DBG, "######## handle downoad fail########\n");
#endif
        retry_all_fail_item(DOWN_FAIL);
    }
#endif

    if(!item_empty(up_head))
    {
#ifdef DEBUG
        Cdbg(APP_DBG, "######## handle up item fail########\n");
#endif
        retry_all_fail_item(UP_FAIL);
    }
}

void* sigmgr_thread(){
    sigset_t   waitset;
    int        sig;
    int        rc;
    pthread_t  ppid = pthread_self();

    pthread_detach(ppid);


    sigemptyset(&waitset);
    sigaddset(&waitset,SIGUSR1);
    sigaddset(&waitset,SIGUSR2);
    sigaddset(&waitset,SIGTERM);
    sigaddset(&waitset,SIGPIPE);

    Cdbg(APP_DBG, "SIGUSR1 - %d\n", SIGUSR1);
    Cdbg(APP_DBG, "SIGUSR2 - %d\n", SIGUSR2);
    Cdbg(APP_DBG, "SIGTERM - %d\n", SIGTERM);
    Cdbg(APP_DBG, "SIGPIPE - %d\n", SIGPIPE);



    while (1)  {
        rc = sigwait(&waitset, &sig);
        Cdbg(APP_DBG, "rc - %d\n", rc);
        if (rc != -1) {
            Cdbg(APP_DBG, "sigwait() fetch the signal - %d\n", sig);
            sig_handler(sig);
        } else {
            Cdbg(APP_DBG, "sigwaitinfo() returned err: %d; %s\n", errno, strerror(errno));
        }
    }
}

#ifdef IPKG
int write_notify_file(char *path,int signal_num)
{
    FILE *fp;
    char fullname[64];
    memset(fullname,0,sizeof(fullname));

    //my_mkdir_r(path);
    my_mkdir("/tmp/notify");
    my_mkdir("/tmp/notify/usb");
    sprintf(fullname,"%s/asuswebstorage",path);
    fp = fopen(fullname,"w");
    if(NULL == fp)
    {
        Cdbg(APP_DBG, "open notify %s file fail\n",fullname);
        return -1;
    }
    fprintf(fp,"%d",signal_num);
    fclose(fp);
    return 0;
}
#endif



void initRouterConfig() {

    memset(time_now,0, sizeof(time_now));

    time_t t = time(0);
    strftime( time_now, sizeof(time_now), "%Y%m%d",localtime(&t) );

    Cdbg(APP_DBG, "time_now : %s\n", time_now);


    curl_global_init(CURL_GLOBAL_DEFAULT);


    char error_message[NORMALSIZE];

    memset(error_message,0,sizeof(error_message));

    clean_global_var();


    // setting base path

    strcpy(base_path,"/tmp");

    Cdbg(APP_DBG, "base_path : %s", base_path);

    DIR *dp = opendir(base_path);

    if(NULL == dp)
    {
        Cdbg(APP_DBG, "Open %s dir fail, aicam exit",base_path);
        exit(-1);
    } else {
        Cdbg(APP_DBG, "Open %s dir success",base_path);
    }

    closedir(dp);

    init_global_var();

}

void initAiCamConfig() {


    memset(time_now,0, sizeof(time_now));

    time_t t = time(0);
    strftime( time_now, sizeof(time_now), "%Y%m%d",localtime(&t) );

    Cdbg(APP_DBG, "time_now : %s\n", time_now);

    // read aicam dir param, frist
    initGoogleToken(&google_d_conf);

    initStorageProvisionConfig(&sp_conf, STORAGE_PROVISION_PATH);

    initAaewsProvisionConfig(&ap_conf);

    initSysinfoConfig(&si_conf);

    initDeployConfig(&dp_conf);

    initBasicCommandConfig(&bc_conf);


    //initSystemConfig(&s_conf);



    if( (strstr(sp_conf.alarmStorageType, "asus")) 
         && (strstr(sp_conf.recordStorageType, "asus")) ) {

        upload_type = RECORD_ASUS_ALARM_ASUS;

        Cdbg(APP_DBG, "File upload type : alarm && record  upload asus webstorage");

    } else if( (strstr(sp_conf.alarmStorageType, "asus")) 
         && (strstr(sp_conf.recordStorageType, "aicloud")) ) {

        upload_type = RECORD_AICLOUD_ALARM_ASUS;

        Cdbg(APP_DBG, "File upload type : alarm upload asus webstorage, record upload aiCloud");

    } else if( (strstr(sp_conf.alarmStorageType, "asus")) 
         && (strstr(sp_conf.recordStorageType, "google")) ) {

        upload_type = RECORD_GOOGLE_ALARM_ASUS;

        Cdbg(APP_DBG, "File upload type : alarm upload asus webstorage, record upload google drive, but now function not work");

    } else if( (strstr(sp_conf.alarmStorageType, "asus")) 
         && (strstr(sp_conf.recordStorageType, "sd")) ) {

        upload_type = RECORD_SD_ALARM_ASUS;

        Cdbg(APP_DBG, "File upload type : alarm upload asus webstorage, record save SD");

    }

    // setting asuswebstorage.conf
    initAsusWebstorageCfg(&asus_conf, &ap_conf, &dp_conf, &si_conf, &bc_conf, &s_conf);


    curl_global_init(CURL_GLOBAL_DEFAULT);


    char error_message[NORMALSIZE];

    memset(error_message,0,sizeof(error_message));

    clean_global_var();


    //strcpy(base_path,"/tmp");
    // setting base path

/*
    char *getBasePath = strtok(asus_conf.uploader_path, "/");

    int iw = 0;

    Cdbg(APP_DBG, "asus_conf.uploader_path = %s\n", asus_conf.uploader_path);


    while (getBasePath != NULL) {
        iw++;
        if(iw == 1) {
            strcat(base_path, "/");
            strcat(base_path, getBasePath);

        }
        getBasePath = strtok(NULL, " ");
    }

*/

    // setting base path

    strcpy(base_path,"/tmp");

    Cdbg(APP_DBG, "base_path : %s", base_path);

    DIR *dp = opendir(base_path);

    if(NULL == dp)
    {
        Cdbg(APP_DBG, "Open %s dir fail, aicam exit",base_path);
        exit(-1);
    } else {
        Cdbg(APP_DBG, "Open %s dir success",base_path);
    }

    closedir(dp);

    init_global_var();


    //write_log(S_INITIAL,"","");


    //Cdbg(APP_DBG, "######asuswebstorage##### start \n");
   // write_system_log("######asuswebstorage#####","start");

/*
#ifdef IPKG
    Cdbg(APP_DBG, "######asuswebstorage##### start \n");
    write_get_nvram_script("cloud_sync",NVRAM_PATH_1,GET_NVRAM_SCRIPT_1);
    write_get_nvram_script("link_internet",NVRAM_PATH_2,GET_NVRAM_SCRIPT_2);
    system(SH_GET_NVRAM_SCRIPT_1);
    enter_sleep_time(1000*2000,NULL);
#endif


#ifdef IPKG
    write_notify_file(NOTIFY_PATH,SIGUSR2);
#endif
*/

    read_config();
}

int tsdbProcess(int status, char * msg) {

    Cdbg(APP_DBG, "tsdbProcess -> status %d , %s CONNECT_TIMEOUT\n", status, msg);
    // 28 -> connect timeout
    // 6 -> This error mean that curl can't resolve host name
    // 35 -> CURLE_SSL_CONNECT_ERROR

    if((status == 28) || (status == -28)) {
        Cdbg(APP_DBG, "tsdbProcess -> %s CONNECT_TIMEOUT\n", msg);
    } else if((status == 6) || (status == -6)) {
        Cdbg(APP_DBG, "tsdbProcess -> %s  CURL_CANNOT_RESOLVE_HOSTNAME\n", msg);
    } else if((status == 35) || (status == -35)) {
        Cdbg(APP_DBG, "tsdbProcess -> %s  CURLE_SSL_CONNECT_ERROR\n", msg);
    } else {
        apiErrorAddTsDb(status, msg);
    }

    return -1;

}


void initTcodeParam() {

    // init aicam_device_dir
    memset(aicam_device_dir,0,sizeof(aicam_device_dir));
    memset(tcode_country,0,sizeof(tcode_country));
    memset(tcode_channel,0,sizeof(tcode_channel));

    Cdbg(APP_DBG, "Tcode = %s", s_conf.tcode);

    char tcode_tmp[10];
    memset(tcode_tmp,0,sizeof(tcode_tmp));
    strcpy(tcode_tmp, s_conf.tcode);
    
    // get country code    
    strcpy(tcode_country, strtok(tcode_tmp, "/"));
    strcpy(tcode_channel, s_conf.videomode);

    if( strstr(tcode_channel, "00")) {
    
        Cdbg(APP_DBG, "tcode country =  %s ,tcode_channel = %s, Alarm Video", tcode_country, tcode_channel);

    } else if( strstr(tcode_channel, "03")) {

        Cdbg(APP_DBG, "tcode country =  %s ,tcode_channel = %s, Record Video", tcode_country, tcode_channel);

    } else {

        Cdbg(APP_DBG, "tcode country =  %s ,tcode_channel = %s, Video Type unknown", tcode_country, tcode_channel);

    }


    // if( strstr(s_conf.videomode, "alarm")) {


    //     strcpy(tcode_channel, SPACE_2GB_7DAYS);

    //     Cdbg(APP_DBG, "tcode country =  %s ,tcode_channel = %s, Alarm Video", tcode_country, tcode_channel);

    // } else if( strstr(s_conf.videomode, "record")) {

    //     strcpy(tcode_channel, TIME_CONTINOUS_7DAYS);

    //    Cdbg(APP_DBG, "tcode country =  %s ,tcode_channel = %s, Record Video", tcode_country, tcode_channel);

    // } else {


    //     strcpy(tcode_channel, SPACE_2GB_7DAYS);

    //     Cdbg(APP_DBG, "tcode country =  %s ,tcode_channel = %s, Alarm Video", tcode_country, tcode_channel);


    // }

    // Cdbg(APP_DBG, "init space_plan = %d , SPACE_SIZE_PLAN", space_plan);

    // if( strcmp(s_conf.tcode, "") == 0 ) {

    //     strcpy(tcode_channel, TIME_CONTINOUS_7DAYS);
    //     strcpy(tcode_country, TCODE_COUNTRY_TW);

    //     strcpy(aicam_device_dir, asus_conf.mac_name);

    //     space_plan = TIME_CONTINOUS_PLAN;

    //     Cdbg(APP_DBG, "init tcode is \"\", setting : tcode_channel = %s, TIME_CONTINOUS_7DAYS", tcode_channel);
    //     Cdbg(APP_DBG, "init space_plan = %d , TIME_CONTINOUS_PLAN", space_plan);

    // // country -> TW / 01
    // } else if( strstr(s_conf.tcode, TCODE_COUNTRY_TW)  && strstr(s_conf.tcode, "01") ) {        

    //     strcpy(tcode_country, TCODE_COUNTRY_TW);

    //     strcpy(tcode_channel, TIME_CONTINOUS_7DAYS);
    //     strcpy(aicam_device_dir, asus_conf.mac_name);   // setting : record dir = mac

    //     space_plan = TIME_CONTINOUS_PLAN;

    //     Cdbg(APP_DBG, "init tcode country =  %s , setting : tcode_channel = %s, TIME_CONTINOUS_7DAYS", tcode_country, tcode_channel);
    //     Cdbg(APP_DBG, "init space_plan = %d , TIME_CONTINOUS_PLAN", space_plan);

    // // country -> other, aicam_device_dir = V2
    // } else {

    //     strcpy(tcode_country, strtok(tcode_tmp, "/"));

    //     strcpy(tcode_channel, SPACE_2GB_7DAYS);
    //     strcpy(aicam_device_dir, "V2");           // setting : alarm dir = V2

    //     space_plan = SPACE_SIZE_PLAN;

    //     Cdbg(APP_DBG, "init tcode country =  %s , setting : tcode_channel = %s, SPACE_2GB_7DAYS", tcode_country, tcode_channel);
    //     Cdbg(APP_DBG, "init space_plan = %d , SPACE_SIZE_PLAN", space_plan);

    // }

    // Cdbg(APP_DBG, "aicam device upload dir = %s", aicam_device_dir);
}