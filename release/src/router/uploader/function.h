#ifndef FUNCTION_H
#define FUNCTION_H
//#include "api.h"
#include <sys/types.h>
#include "data.h"

#define LOCAL_FILE_QUEUE 1
#define LOCAL_FOLDER_QUEUE 2

extern int receve_socket;

/* download queue*/
struct queue_entry;

struct queue_entry
{
  struct queue_entry * next_ptr;   /* Pointer to next entry */
  char *cmd_name;
};

typedef struct queue_entry * queue_entry_t;

/*struct queue*/
struct queue_struct
{
  struct queue_entry * head;
  struct queue_entry * tail;
};
typedef struct queue_struct *queue_t;

/*add by alan*/

int queue_empty (queue_t q);
queue_t queue_create ();
void queue_destroy (queue_t q);
void queue_enqueue (queue_entry_t d, queue_t q);
queue_entry_t queue_dequeue (queue_t q);


/* sync server item struct*/


struct sync_item
{
    char *action;
    char *name;
    struct sync_item *next;
};

typedef struct sync_item *sync_item_t;

//typedef struct sync_item *sync_item_q;

int item_empty(sync_item_t head);
sync_item_t create_head();
int add_sync_item(const char *action,const char *name,sync_item_t head);
void del_sync_item(char *action,char *name, sync_item_t phead);
void del_download_only_sync_item(char *action,char *name, struct sync_item *phead);
sync_item_t get_sync_item(char *action,char *name,sync_item_t head);
void free_sync_item(sync_item_t head);
void print_sync_item(sync_item_t head,int type);
void print_all_sync_item(struct sync_item *head);
//void update_all_item_path(struct sync_item *head,char *new_sync_path);

/*basic function */
char *oauth_encode_base64(int size, const unsigned char *src);
int oauth_decode_base64(unsigned char *dest, const char *src);
char *oauth_url_escape(const char *string);
char *oauth_url_unescape(const char *string, size_t *olen);
char *oauth_sign_hmac_sha1 (const char *m, const char *k);
char *oauth_gen_nonce();

char* MD5_string(char *string);

int get_all_folder_in_mount_path(const char *const mount_path, int *sh_num, char ***folder_list);

int syncServerAllItem(char *username,int parentid,char *localpath);
//int mySync(char *username,int parentid,char *localpath,NodeStack **head);

int get_all_folders(const char *dirname,Folders *allfolderlist);
int initMyLocalFolder(char *username,int parentid,char *localpath,char *xmlfilename);

long long int check_disk_space(char *path);

int test_if_dir(const char *dir);

void print_all_local_item(char *dir,char *sync_item_filename,int init);

int parse_trans_item(char *path,int type);
Transitem *parse_trans_item_from_buffer(char *buffer,int type);
char *parse_name_from_path(const char *path);
void init_up_excep_fail(char *path);
void add_server_action_list(char *action,char *fullname,struct sync_item *head);

/*define local item strct*/
typedef struct L_FOLDER
{
    unsigned int id;
    char name[256];
    char parentfolder[256];
    struct L_FOLDER *next;
}L_folder;

typedef struct L_FILE
{
    unsigned int id;
    char name[256];
    char parentfolder[256];
    struct L_FILE *next;
}L_file;

/* other function*/
int test_if_dir_empty(char *path);
int test_if_file_exist(char *filename);
int test_if_file_up_excep_fail(char *name);
void free_local_fileslist(Local *local);
void free_local_folderslist(Local *local);
void free_local_list(Local *local);
void free_server_fileslist(Browse *browse);
void free_server_folderslist(Browse *browse);
void free_server_list(Browse *browse);
int is_copying_finished(char *filename);

int my_mkdir(char *path);
int my_mkdir_r(char *path);
char *get_confilicted_name(const char *filename,int isfolder);

int parse_nvram(const char *value,struct asus_config *config);
int test_if_download_temp_file(char *filename);
void enter_sleep_time(int num,my_mutex_t *mutex);
int wait_handle_socket();

#ifdef IPKG
int check_disk_change();

struct mounts_info_tag
{
    int num;
    char **paths;
};

int get_mounts_info(struct mounts_info_tag *info);
int check_token_file(struct asus_config *cfg);
int check_record_token_file(char *record_filename,char *token_filename);
int write_token_file(char *path,char *token_filename);
int record_token_to_file(char *filename,char *token_filename);
//int get_token_filename(char *filename,char *sync_path);
//int rewrite_config(char *config_path,char *new_sync_path);

int detect_process_file();
void create_start_file();
#endif

#ifdef IPKG
int write_get_nvram_script(char *nvram_name,char *nvram_path,char *script_path);
int create_asuswebstorage_conf_file(char *path);
#else
int convert_nvram_to_file(char *file);
//int create_shell_file();
//int write_to_nvram(char *contents,char *nv_name);
//int del_old_token_file(char *mount_path);
//int check_nvram_token_file(char *token_filename);
#endif


int parse_config_new(char *path,struct asus_config *cfg);
int  parse_sync_path(struct asus_config *cfg,char *path);
int check_network_state();

int clean_download_temp_file(struct sync_item *head);
int check_accout_status();
int parse_config_onexit(char *path,struct asus_config *cfg);
char *get_confilicted_name_case(const char *fullname,const char *path,const char *pre_name, const char *raw_name);

/*create statk*/
typedef struct FolderNode_t
{
    char *path;
    //char *name;
    int id;
    int seq;
}FolderNode;

typedef struct NodeStack_t
{
    FolderNode *point;
    struct NodeStack_t *next;
}NodeStack;

void push_node(FolderNode *node,NodeStack **head);
FolderNode *pop_node(NodeStack **head);

long long FileSize(const char* szFilename);
int LoadFileIntoBuffer(const char* szFileName, char** pBuffer, int* pBufferLength);

/*save cloud all folders id and seqnum*/
struct SubNode
{
    //int pid;
    int id;
    int seq;
    struct SubNode *Child;
    struct SubNode *NextBrother;
};

typedef struct SubNode Hb_SubNode;

Hb_SubNode *create_node(int id,int seq);
Hb_SubNode *get_parent_node(int pid,Hb_SubNode *node);
Hb_SubNode *find_node(Hb_SubNode *pnode,int id);
int update_seq(int id,int seq,Hb_SubNode *node);
int del_node(int pid,int id);
int remove_node(Hb_SubNode *node,int id);
int add_node(int id,int seq,Hb_SubNode *node);
int insert_node(int pid,int id,int seq);
int move_node(int move_from_pid,int id,int move_to_pid);
void free_node(Hb_SubNode *node);
void print_all_nodes(Hb_SubNode *node);

typedef struct _api_count_s
{
    int requestservicegateway;
    int acquiretoken;
    int getinfo;
    int getmysyncfolder;
    int getpersonalsystemfolder;
    int getuserstate;
    int browse;
    int propfind;
    int create;
    int rename;
    int move;
    int remove;
    int getentryinfo;
    int getchangeseq;
    int initbinaryupload;
    int resumebinaryupload;
    int finishbinaryupload;
    int directdownload;
} api_count_s;

#endif
