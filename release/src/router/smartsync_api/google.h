//#include <stddef.h>
#include <stdint.h>
#include<time.h>
#include <dirent.h>
#include <stdio.h>
#include "cJSON.h"
#include <pthread.h>
#include <sys/types.h>

//data.h****************************
#define max(a,b) ( ((a)>(b)) ? (a):(b) )


/*#####server space######*/
long long int server_shared;
long long int server_quota;
long long int server_normal;
/*######Global var*/
typedef struct Action_Item
{
    char *action;
    char *path;
    struct Action_Item *next;
} action_item;

typedef struct LOG_STRUCT{
    uint8_t  status;
    char  error[512];
    float  progress;
    char path[512];
}Log_struc;
#define LOG_SIZE                sizeof(struct LOG_STRUCT)

pthread_mutex_t mutex_log;
pthread_cond_t cond_socket;
/*********************list.h***********************/
#define MAX_LENGTH 128
#define MIN_LENGTH 64
#define MAX_CONTENT 256
#define MAXDATASIZE 1024
#define MINSIZE 64

typedef struct node{
    char *href;
    long long int size;
    int isFolder;
    char tmptime[MIN_LENGTH];
    char *name;
    struct node *next;
    time_t mtime;
    char *mimeType;
    char *id;
    char *parents_id;
    char *exportLinks_document;
    int isRoot;
    int isTrash;
}CloudFile;

CloudFile *FolderCurrent;
CloudFile *FolderTmp;
CloudFile *FolderTmp_One;
CloudFile *FileList_one;
CloudFile *FileTail_one;
CloudFile *FileTmp_one;
CloudFile *TreeFolderList;
CloudFile *TreeFileList;
CloudFile *TreeFolderTail;
CloudFile *TreeFileTail;

typedef struct BROWSE
{
    int foldernumber;
    int filenumber;
    CloudFile *folderlist;
    CloudFile *filelist;
}Browse;

struct ServerTreeNode
{
    int level;
    char *parenthref;
    char *folderid;
    Browse *browse;
    struct ServerTreeNode *Child;
    struct ServerTreeNode *NextBrother;
};

typedef struct ServerTreeNode Server_TreeNode;

/*******************data.h*****************/
char general_log[MAX_CONTENT];
#define LOCAL_SPACE_NOT_ENOUGH          900
#define SERVER_SPACE_NOT_ENOUGH         901
#define LOCAL_FILE_LOST                 902
#define SERVER_FILE_DELETED             903
#define COULD_NOT_CONNECNT_TO_SERVER    904
#define CONNECNTION_TIMED_OUT           905
#define INVALID_ARGUMENT                906
#define PARSE_XML_FAILED                907
#define COULD_NOT_READ_RESPONSE_BODY    908
#define HAVE_LOCAL_SOCKET               909
#define SERVER_ROOT_DELETED             910

#define CMD_SPLIT "\n"

#ifdef OAuth1
/*####auth*/
typedef struct Authentication
{
    char tmp_oauth_token_secret[32];
    char tmp_oauth_token[32];
    char oauth_token_secret[32];
    char oauth_token[32];
    int uid;
}Auth;

Auth *auth;
#else
typedef struct Authentication
{
    char *oauth_token;
}Auth;
Auth *auth;
#endif
/*#####socket list*/
struct queue_entry
{
    struct queue_entry * next_ptr;   /* Pointer to next entry */
    char *cmd_name;
    char *re_cmd;
    int is_first;
};
typedef struct queue_entry * queue_entry_t;

/*struct queue*/
struct queue_struct
{
    struct queue_entry *head;
    struct queue_entry *tail;
};
typedef struct queue_struct *queue_t;
typedef struct SYNC_LIST{

    char conflict_file[256];
    char temp_path[MAX_LENGTH];

    int sync_disk_exist;
    int have_local_socket;
    int no_local_root;
    int init_completed;
    int receve_socket;
    int first_sync;
    action_item *download_only_socket_head;
    action_item *server_action_list;    //Server变化导致的Socket
    action_item *copy_file_list;         //The copy files
    action_item *unfinished_list;
    action_item *up_space_not_enough_list;
    action_item *access_failed_list;
    queue_t SocketActionList;
    Server_TreeNode *ServerRootNode;
    Server_TreeNode *OldServerRootNode;
    Server_TreeNode *VeryOldServerRootNode;
}sync_list;
sync_list **g_pSyncList;

typedef cJSON *(*proc_pt)(char *filename);

/*stat local file*/
typedef struct Local_Struct
{
    char *path;
    int index;
}Local_Str;
Local_Str LOCAL_FILE;

typedef struct Upload_chunk
{
    char *filename;
    char *upload_id;
    unsigned long chunk;
    unsigned long offset;
    time_t expires;
} Upload_chunked;
Upload_chunked *upload_chunk;

pthread_mutex_t mutex_socket;

/*muti dir read config*/
struct asus_rule
{
    int rule;
    char path[512];//fullname name
    char rooturl[512]; //sync folder
    char base_path[512];     //base_path is the mount path
    int base_path_len;
    int rooturl_len;
};

struct asus_config
{
    int type;
    char user[256];
    char pwd[256];
    int  enable;
    int ismuti;
    int dir_number;
    struct asus_rule **prule;
};
struct asus_config asus_cfg;

int add_action_item(const char *action,const char *path,action_item *head);
size_t my_write_func(void *ptr, size_t size, size_t nmemb, FILE *stream);
int my_progress_func(char *clientfp,double t,double d,double ultotal,double ulnow);
int api_upload_put( char *filename,char *serverpath,int flag,int index);

char *get_server_exist(char *temp_name,int index);
char *makeAuthorize(int flag);
size_t my_read_func(void *ptr, size_t size, size_t nmemb, FILE *stream);
char *search_newpath(char *href,int index);
Browse *browseFolder(char *parentref, char *URL, int index);
cJSON *dofile(char *filename);
int g_delete(char *herf,int index, char *delete_ids);
int g_create_folder(char *localpath,char *foldername,char *newfolderid, int index, int i);
time_t ne_rfc1123_parse2(const char *date);
char *my_str_malloc(size_t len);
time_t ne_rfc1123_parse(const char *date);
char *parse_name_from_path(const char *path);
int g_download(CloudFile *filetmp, char *fullname,char *filename,int index);
int g_upload_file(char *filename,char *serverpath,int flag,int index);
int g_browse_to_tree(char *parenthref, char *id, Server_TreeNode *node, int index);
int g_move(char *oldname,char *newname,int index,int is_changed_time,char *newname_r);

