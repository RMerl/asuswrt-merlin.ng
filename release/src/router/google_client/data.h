#include <stdint.h>
#include <pthread.h>
#include <signal.h>
#include "list.h"
#include "function.h"
#include "cJSON.h"
#include <sys/statvfs.h>
#include <utime.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdarg.h>

#include "mem_pool.h"

#define min(a,b) ( ((a)>(b)) ? (b):(a) )

#define __DEBUG__
//#undef __DEBUG__

#ifdef __DEBUG__
//#define DEBUG(format, ...) printf("FILE:"__FILE__",LINE:%05d:"format"\n",__LINE__,##__VA_ARGS__)
#define DEBUG(format, ...) printf(format,##__VA_ARGS__)
#else
#define DEBUG(format, ...)
#endif

#ifdef __DEBUG__
//#define wd_DEBUG(format,...) printf("File: "__FILE__", Line: %05d: "format"/n", __LINE__, ##__VA_ARGS__)
#define wd_DEBUG(format,...) printf(format, ##__VA_ARGS__)
#define CURL_DEBUG
//#define CURL_DEBUG curl_easy_setopt(curl,CURLOPT_VERBOSE,1)
#else
#define wd_DEBUG(format,...)
#define CURL_DEBUG
#endif



#ifdef NVRAM_
    #ifndef USE_TCAPI
        #include <bcmnvram.h>
    #else
        #define WANDUCK "Wanduck_Common"
        #define AICLOUD "AiCloud_Entry"
        #include "libtcapi.h"
        #include "tcapi.h"
    #endif
#include <shutils.h>
#define SHELL_FILE  "/tmp/smartsync/script/write_nvram"
#else
    #define TMPCONFIG "/tmp/smartsync/google/config/google_tmpconfig"
    #define Google_Get_Nvram "/tmp/smartsync/google/script/google_get_nvram"
    #define TMP_NVRAM_VL "/tmp/smartsync/google/temp/google_tmp_nvram_vl"
    #define Google_Get_Nvram_Link "/tmp/smartsync/google/script/google_get_nvram_link"
#endif

#define HAVE_LOCAL_SOCKET               909
#define NOTIFY_PATH "/tmp/notify/usb"

//#define TEST 1
#define RENAME_F
#define TOKENFILE 1
#define MEM_POOL_ENABLE 1
#define CA_INFO_FILE "/tmp/smartsync/google/cert/GeoTrustGlobalCA.crt"


pthread_t newthid1,newthid2,newthid3;
pthread_cond_t cond,cond_log;
pthread_mutex_t mutex,mutex_receve_socket;
extern pthread_mutex_t mutex_log;
extern pthread_cond_t cond_socket;
extern pthread_mutex_t mutex_socket;
/*
 mutex=>server pthread
 mutex_socket=>socket save pthread
 mutex_receve_socket=>socket parse pthread
*/

/* log struct */
char log_path[MAX_LENGTH];
extern char general_log[];
char trans_excep_file[MAX_LENGTH];


extern Local_Str LOCAL_FILE;

#define S_INITIAL		70
#define S_SYNC			71
#define S_DOWNUP		72
#define S_UPLOAD		73
#define S_DOWNLOAD		74
#define S_STOP			75
#define S_ERROR			76
//#define LOG_SIZE                sizeof(struct LOG_STRUCT)


//define temporary file
#define TMP_R "/tmp/smartsync/google/temp/"
#define Con(TMP_R,b) TMP_R#b

/*#####server space######*/
extern long long int server_shared;
extern long long int server_quota;
extern long long int server_normal;
/*######Global var*/

int local_sync;
int server_sync;
int finished_initial;

extern Upload_chunked *upload_chunk;


queue_entry_t SocketActionTmp;


extern Auth *auth;

typedef struct Account_info
{
    char usr[256];
    char pwd[256];
}Info;

Info *info;


#ifdef PC
#define CONFIG_PATH "/WorkSpace_pc/google-test/google.conf"
#else
#define CONFIG_PATH "/tmp/smartsync/google/config/google.conf"
#endif


struct asus_config asus_cfg_stop;
extern struct asus_config asus_cfg;


extern sync_list **g_pSyncList;



char *search_newpath(char *href,int index);
Browse *browseFolder(char *parentref, char *URL, int index);
#ifdef OAuth1
char *makeAuthorize(int flag);
#else
char *makeAuthorize();
#endif
int get_request_token();
int parse(char *filename,int flag);
char *parse_login_page();
int login(void);
int cgi_init(char *query);
int api_download(CloudFile *filetmp, char *fullname,char *filename,int index);
int sync_local_with_server_init(Server_TreeNode *treenode,Browse *perform_br,Local *perform_lo,int index);
int the_same_name_compare(LocalFile *localfiletmp,CloudFile *filetmp,int index,int is_init);
char *parse_name_from_path(const char *path);
int add_socket_item(char *buf);
char *get_socket_base_path(char *cmd);
void queue_enqueue (queue_entry_t d, queue_t q);
queue_t queue_create ();
void *Socket_Parser();
int cmd_parser(char *cmd,int index,char *re_cmd);
int is_server_exist(char *path, char *temp_name, char *URL, int index);
int get_path_to_index(char *path);
int dragfolder(char *dir,int index);
int dragfolder_rename(char *dir,int index,time_t server_mtime);
queue_entry_t  queue_dequeue (queue_t q);
queue_entry_t  queue_dequeue_t (queue_t q);
int del_action_item(const char *action,const char *path,action_item *head);
action_item *get_action_item(const char *action,const char *path,action_item *head,int index);
int add_action_item(const char *action,const char *path,action_item *head);
void del_download_only_action_item(const char *action,const char *path,action_item *head);
void free_action_item(action_item *head);
action_item *create_action_item_head();
int download_only_add_socket_item(char *cmd,int index);
int add_all_download_only_socket_list(char *cmd,const char *dir,int index);
int wait_handle_socket(int index);
int get_create_threads_state();
void *SyncServer();
int compareServerList(int index);
int isServerChanged(Server_TreeNode *newNode,Server_TreeNode *oldNode);
int Server_sync(int index);
void del_all_items(char *dir,int index);
void cjson_to_space(cJSON *q);
int test_if_download_temp_file(char *filename);
int do_unfinished(int index);
int my_progress_func(char *clientfp,double t,double d,double ultotal,double ulnow);
size_t my_write_func(void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t my_read_func(void *ptr, size_t size, size_t nmemb, FILE *stream);
void clean_up();
void queue_destroy (queue_t q);
int write_log(int status, char *message, char *filename,int index);
char *change_local_same_file(char *oldname,int index);
void replace_char_in_str(char *str,char newchar,char oldchar);
action_item *get_action_item_access(const char *action,const char *path,action_item *head,int index);
action_item *check_action_item(const char *action,const char *oldpath,action_item *head,int index,const char *newpath);
void buffer_free(char *b);
int write_conflict_log(char *prename, char *conflict_name,int index);
int write_trans_excep_log(char *fullname,int type,char *msg);
int sync_initial_again(int index);

int api_metadata_test_dir(char *id, proc_pt cmd_data);
int cJSON_printf_dir(cJSON *json);
char *get_server_exist(char *temp_name,int index);
void updata_socket_list(char *temp_name,char *new_name,int i);

#if TOKENFILE
int disk_change;
int sync_disk_removed;
int sighandler_finished;
#define TOKENFILE_RECORD "/opt/etc/.smartsync/gd_tokenfile"
struct mounts_info_tag
{
    int num;
    char **paths;
};
struct tokenfile_info_tag
{
    char *folder;
    char *url;
    char *mountpath;
    struct tokenfile_info_tag *next;
};
struct tokenfile_info_tag *tokenfile_info,*tokenfile_info_start,*tokenfile_info_tmp;
int get_mounts_info(struct mounts_info_tag *info);
int write_notify_file(char *path,int signal_num);
int get_tokenfile_info();
struct tokenfile_info_tag *initial_tokenfile_info_data(struct tokenfile_info_tag **token);
int check_config_path(int is_read_config);
int write_to_tokenfile(char *mpath);
int write_to_wd_tokenfile(char *contents);
char *write_error_message(char *format,...);
#ifdef NVRAM_
int create_shell_file();
int convert_nvram_to_file_mutidir(char *file,struct asus_config *config);
int detect_process(char * process_name);
#else
int create_webdav_conf_file(struct asus_config *config);
int write_get_nvram_script();
int write_get_nvram_script_va(char *str);
#endif
int rewrite_tokenfile_and_nv();
int delete_tokenfile_info(char *url,char *folder);
char *delete_nvram_contents(char *url,char *folder);
int add_tokenfile_info(char *url,char *folder,char *mpath);
char *add_nvram_contents(char *url,char *folder);
int check_disk_change();
int check_sync_disk_removed();
int free_tokenfile_info(struct tokenfile_info_tag *head);
#endif
char *change_server_same_name(char *fullname, char *URL, int index);

//int ci_trash_list();
