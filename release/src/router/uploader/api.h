#ifndef __API_H
#define __API_H
#include "data.h"

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/HTMLparser.h>
#include <libxml/xmlreader.h>

#include "uploader_config.h"

#define getb(type) (type*)malloc(sizeof(type))
#define NAMESIZE 256

#define UPLOAD 1
#define DOWNLOAD 2
#define UP_EXCEP_FAIL 3

//#define DEBUG 1
#define SYSTEM_LOG 0
#define my_free(x)  if(x) {free(x);x=NULL;}
#define TREE_NODE_ENABLE 0
#define SERVER_TREE      0
#define MUTI_DIR         0
#define WRITE_DOWNLOAD_TEMP_FILE 0
#define MEM_POOL_ENABLE 1



#define DEBUG_LEVEL 4

#define DBG_VERBOSE 5
#define DBG_DEBUG 4
#define DBG_INFO 3
#define DBG_WARN 2
#define DBG_ERROR 1
#define DBG_NO 0

#define MAC_ONE 1
#define MAC_TWO 2
#define MAC_THREE 3
#define MAC_FOUR 4

/*define server error code*/



#define RECORD_ASUS_ALARM_ASUS          0
#define RECORD_SD_ALARM_ASUS            1
#define RECORD_AICLOUD_ALARM_ASUS       2
#define RECORD_GOOGLE_ALARM_ASUS        3


#define DATA_IS_NULL                   -1
#define DEVICE_AUTH_FAIL               -2
#define MD5_PASSWORD_NULL              -5
#define TOKEN_AUTH_FAIL                -6
#define DIR_NEED_CREATE                 0
#define AICAM_ROOT_ID                 -22

#define DEVICE_EXPIRED                 -5
#define SPACE_PLAN_DIFF                -9

#define ASUS_TOKEN_SID                 ((const unsigned char *)"1002")


#define ASUS_FOLDER_NAME              "asus"
#define ROUTER_ROUTER_NAME            "router"
#define LOG_FOLDER_NAME               "log"
#define DRIVE_FILE_ID                 "file_id"

#define DRIVE_FOLDER_TYPE              "1"
#define DRIVE_FILE_TYPE                "2"

//#define DRIVE_FOLDER_TYPE              "1"
//#define DRIVE_FILE_TYPE                "2"

#define HTTP_REQUEST_GET               "1"
#define HTTP_REQUEST_POST              "2"
#define HTTP_REQUEST_POST_JSONTYPE     "3"

#define TCODE_TYPE_ZERO                "00"
#define TCODE_TYPE_ONE                 "01"
#define TCODE_TYPE_TWO                 "02"
#define TCODE_TYPE_THREE               "03"

#define SPACE_2GB_7DAYS                "00"
#define SPACE_10GB_30DAYS              "01"
#define SPACE_50GB_30DAYS              "02"

#define TIME_CONTINOUS_7DAYS           "03"
#define TIME_CONTINOUS_30DAYS          "04"


#define TIME_CONTINOUS_PLAN             0
#define SPACE_SIZE_PLAN                 1


#define STORAGE_PROVISION_PATH          "/tmp/aicam/storageProvision"
#define AICAM_ALARM_PATH                "V2"
#define AICAM_PRODUCT_NAME              "AiCam NightView"
// #define AICAM_PRODUCT_NAME             "AiCam WideView"



#define TCODE_COUNTRY_TW               ((const unsigned char *)"TW")


#define SALES_PLAN_ONE                  1
#define SALES_PLAN_TWO                  2
#define SALES_PLAN_THREE                3
#define SALES_PLAN_FOUR                 4
#define SALES_PLAN_FIVE                 5

#define KEEP_DAY_ONE                    7
#define KEEP_DAY_TWO                    14
#define KEEP_DAY_THREE                  30

#define DIR_NOT_EXIST                   2

#define CONNECT_TIMEOUT               -28
#define CURL_CANNOT_RESOLVE_HOSTNAME   -6
#define CURLE_SSL_CONNECT_ERROR       -35


#define S_PASS                          0
#define S_VER_NOT_SUPPORT               1
#define S_AUTH_FAIL                     2
#define S_RAYLOAD_NOT_VALIDATE          4
#define S_ACCOUNT_AUTH_FAIL             5
#define S_READ_ONLY                     200
#define S_NAME_GER_ERR                  210
#define S_NAME_BLANK                    211
#define S_NAME_ILLEGAL                  212
#define S_NAME_LEN_TOO_LONG             213
#define S_NAME_REPEAT                   214
#define S_MOVE_PARENTID_IS_SAME         215
#define S_MOVE_PARENTID_NOT_EXIST       216
#define S_NEED_ADMIN_HANDLE             217
#define S_DIR_NOT_EXIST                 218
#define S_FILE_NOT_EXIST                219
#define S_FILE_GER_ERR                  220
#define S_FILE_TOO_LARGE                221
#define S_DAY_FLOW_OVER                 222
#define S_PARENT_DIR_NOT_EXIST          223
#define S_USER_NOSPACE                  224
#define S_PARAM_OUT_RANGE               225
#define S_ACCOUNT_FROZEN                226
#define S_FILE_FORMAT_NOT_SUPP          227
#define S_USER_CANCLE_FILE_TRANS        228
#define S_ACCESS_FILE_PWD_ERR           229
#define S_SHARE_GER_ERR                 230
#define S_SHARE_GROUP_OUT_LIMIT         231
#define S_SHARE_NUMOFGROUP_OUT_LIMIT    232
#define S_SRCDIR_DETDIR_NOT_ON_LEVEL    233
#define S_PARENT_NOT_ACCORD             234
#define S_ILLEGAL_OPERATION             235
#define EXCEED_GROUPAWARE_SPACE         242
#define S_FILE_CHECKSUM_NOT_MATCH       250
#define S_TRANS_ID_NOT_EXIST            251
#define S_TRANSID_NOTMATCH_FILEID       252
#define S_ILLEGAL_STATE                 501
#define S_UNAUTH_REMOTE_IP              502
#define S_PREDICATE_NOT_ACCORD          503
#define S_OTP_AUTH_FAIL                 504
#define S_OTP_ID_LOCKED                 505
#define S_OTP_ID_OUT_LIMIT              506
#define S_ILLEGAL_ID                    507
#define S_CAPTCHA_FAIL                  508
#define S_GER_ERR                       999
#define S_LOCAL_SPACE_FULL              2001          // local disk full
#define S_SERVER_SPACE_FULL             2002
#define S_MEMORY_FAIL                   4001
#define S_MKDIR_FAIL                    4002
#define S_UPDATE_ATTR_FAIL              4003
#define S_OPENDIR_FAIL                  4004
#define S_REMOVE_LOCAL_FAIL             4005
#define S_GET_PARENTID_FAIL             4006
#define S_UPLOADFILE_FAIL               4007
#define S_DOWNLOADFILE_FAIL             4008
#define S_CREATEFOLDER_FAIL             4009
#define S_HTTP_ERROR                    4010
#define S_MD5_FAIL                      4011
#define S_ENCODE_BASE64_FAIL            4012
#define S_DECODE_BASE64_FAIL            4013
#define S_URL_ESCAPE_FAIL               4014
#define S_SHA1_FAIL                     4015
#define S_RENAME_FAIL                   4016
#define S_UPLOAD_DELETED                4017
#define S_NETWORK_FAIL                  4018
#define S_ACCOUNT_CLOSE                 4019
#define S_PARSE_XML_FAIL                4020

/* define account */
#define IsAutoAccountBit      (1 << 0)  //0X01
#define NORMAL                (1 << 1)  //0X02
#define COUNTING_DOWN         (1 << 2)  //0X04
#define FROZEN                (1 << 3)  //0X08
#define EXTENSION             (1 << 4)  //0X10
#define CLOSE                 (1 << 5)  //0X20

typedef void (*processing)(double current, double total);


extern char mount_path[NAMESIZE];
extern char cloud_path[NAMESIZE];
//char temp_path[NAMESIZE];
extern char asus_path[NAMESIZE];
extern char log_path[NAMESIZE];
extern char xml_path[NAMESIZE];
//char mount_path[NAMESIZE];
extern char sync_path[NAMESIZE];
extern char system_log[NAMESIZE];
extern char general_log[NAMESIZE];
//char tree_log[NAMESIZE];
extern char confilicted_log[NAMESIZE];
//char temp_file[NAMESIZE];
extern char up_item_file[NAMESIZE];
extern char down_item_file[NAMESIZE];

#if WRITE_DOWNLOAD_TEMP_FILE
char down_item_temp_file[NAMESIZE];
#endif

//char all_local_item_file[NAMESIZE];
extern char up_excep_fail_file[NAMESIZE];
//char up_limit_file[NAMESIZE];
extern char gateway_xml[NAMESIZE];
extern char get_user_state_xml[NAMESIZE];
extern char token_xml[NAMESIZE];
extern char get_info_xml[NAMESIZE];
extern char get_sync_folder_xml[NAMESIZE];

extern char get_init_attachable_device_xml[NAMESIZE]; // by markcool
extern char get_browse_attachable_device_xml[NAMESIZE]; // by markcool
extern char get_send_aws_email_xml[NAMESIZE]; // by markcool

extern char get_personal_system_folder_xml[NAMESIZE];
extern char browse_folder_xml[NAMESIZE];
extern char sliding_browse_file_xml[NAMESIZE];

extern char propfind_xml[NAMESIZE];
extern char create_folder_xml[NAMESIZE];
extern char rename_xml[NAMESIZE];
extern char move_xml[NAMESIZE];
extern char remove_xml[NAMESIZE];
extern char update_xml[NAMESIZE];
extern char get_entry_info_xml[NAMESIZE];
//char set_mark_xml[NAMESIZE];
//char get_change_files_xml[NAMESIZE];
//char get_uploads_xml[NAMESIZE];
//char get_share_code_xml[NAMESIZE];
//char del_share_code_xml[NAMESIZE];
//char get_share_entry_xml[NAMESIZE];
//char check_pwd_xml[NAMESIZE];
//char cmp_pwd_xml[NAMESIZE];
extern char get_change_seq_xml[NAMESIZE];
extern char init_upload_xml[NAMESIZE];

extern char search_drive_file_json[NAMESIZE];
extern char create_folder_json[NAMESIZE];
extern char access_token_json[NAMESIZE];
extern char resume_upload_xml[NAMESIZE];
extern char finish_upload_xml[NAMESIZE];
//char get_resize_photo_xml[NAMESIZE];
//char get_full_txt_xml[NAMESIZE];
//char get_video_snapshot_xml[NAMESIZE];
extern char trans_excep_file[128];
extern char system_token[256];
extern char dbg_msg[256];

int if_file_exist(char *filename);
int myParseHTML(char *name);
int parseDoc1(char *docname,void *obj);
void parseDoc(char *docname);
void parseMemory(char *buffer,void *obj);
void streamMyFile(const char *filename);

int getServiceGateway(char *username, char *password,Servicegateway *sg);
int obtainGateway(char *user,char *pwd,Servicegateway *sg);
int getToken(char *username, char *password,char *key,int first);
int obtainToken(char *user,char *pwd,struct asus_config *cfg,int first);

Getinfo *getInfo(char *username,char *server);
Getmysyncfolder *getMySyncFolder(char *username);
int obtainSyncRootID(char *user);

GetInitAttachableDevice *getInitAttachableDevice(char *username, char *base64EncodeOutput, struct asuswebstorage_conf *asus_conf);
int initAttachableDevice(char *user, char *base64EncodeOutput, struct asuswebstorage_conf *asus_conf);

BrowseAttachableDevice *getBrowseAttachableDevice(char *username, struct aaews_provision_conf * ap_conf);
int browseAttachableDevice(char *user, struct aaews_provision_conf * ap_conf, int return_type);

BrowseAttachableDevice *getBrowseAttachableDeviceList(char *username, struct aaews_provision_conf * ap_conf);


int checkBrowseAttachableDeviceXml(char *docname,char *device_id);

int parseAttachableDevice(xmlDocPtr doc, xmlNodePtr cur, BrowseAttachableDevice *bad);

SendAwsEmail *sendAwsEmail(char *username, struct asuswebstorage_conf *asus_conf);
int initSendAwsEmail(char *user, struct asuswebstorage_conf *asus_conf);

int initAsusWebstorageConfig(struct asuswebstorage_conf *asus_conf);

int initAsusWebstorageCfg(struct asuswebstorage_conf *asus_conf, 
	struct aaews_provision_conf * ap_conf, struct deploy_conf * dp_conf,
	struct sysinfo_conf * si_conf, struct basic_command_conf * bc_conf,
	struct system_conf * s_conf);

int initAiCloudConfig(struct aicloud_conf *ai_conf);

int initStorageProvisionConfig(struct storage_provision_conf * sp_conf, char * file_path);

int initAaewsProvisionConfig(struct aaews_provision_conf * ap_conf);

int initSysinfoConfig(struct sysinfo_conf * si_conf);

int initDeployConfig(struct deploy_conf * dp_conf);

int initBasicCommandConfig(struct basic_command_conf * bc_conf);

int initSystemConfig(struct system_conf * s_conf, char * file_path);


int writeAlarmFile();

int debugFileWrite();

Getpersonalsystemfolder *getPersonalSystemFolder(char *username,char *filename);
int GetMyRecycleID(char *username,char *filename);
Getuserstate *getUserState(char *user,char *server);
int CheckUserState(char *user,char *server);
int get_max_upload_filesize(char *username);

Browse *browseFolder(char *username,long long id,int issibiling,int pageno);
int my_parse_browse_xml(const char *filename,Browse *br);

SlidingBrowse *slidingBrowseFile(char *username,long long folderid,long long timestamp);

Operateentry *removeFiles(char *username,char * files_id);

//Browse *GetServerList(char *username,int id,int issibiling);
//int browseFolder(char *username,int id,int issibiling,Browse *br);
Propfind *checkEntryExisted(char *username,int parentID,char *filename,char *type);

Propfind *checkEntryIdExisted(char *userid,long long parentID,char *filename,char *type);


Createfolder *createFolder(char *username,int parentID,int isencrpted,char *name);
Createfolder *createFolderId(char *username,long long parentID,int isencrpted,char *name);

Operateentry *renameEntry(char *username,int id,int isencrpted,char *newname,int isfolder);
Moveentry *moveEntry(char *username,int id,char *name,int parentID,int isfolder,int pre_pid);
Operateentry *removeEntry(char *username,int id,int ischildonly,int isfolder,int pid);
int updateEntryAttribute(char *username,int id,int parentID,int isencrpted,int isfolder);

Getentryinfo *getEntryInfo(int isfolder,int entryid);
//int setEntryMark(int isfolder,int entryid,int markid);
//int getLatestChangeFiles(char *username,int top,int targetroot,int sortdirection);
//int getLatestUploads(char *username,int top,int targetroot,int sortdirection);

/*shared files API*/
//int getShareCode(char *username,int entryType,int entryID,char *password,int actionType);
//int deleteShareCode(char *username,int entryType,int entryID,char *password);
//int getSharedEntries(char *username,int kind,int pagesize,int sortby,int sortdirection,char *firstentrybound);
//int checkPassword(char *username,char *suri);
//int comparePassword(char *username,int isfolder,int ffid,char *password);
Changeseq *getChangeSeq(int folderid);

/* upload file*/
int sha512(char *filename,char *checksum);
Initbinaryupload  *initBinaryUpload(char *filename,long long parentID,char *transid,int fileID);
Resumebinaryupload *resumeBinaryUpload(char *filename, Initbinaryupload *ibu);
Finishbinaryupload *finishBinaryUpload(Initbinaryupload *ibu);
int uploadFile(char *filename,int parentID,char *transid,int fileID);
//int check_exist_on_server(char *username,char *filename,int parentID);

/*download file*/
int update_local_file_attr(Fileattribute *attr,char *filename);
int downloadFile(int fileID,char *filename,long long int size,int ismodify,Fileattribute *attr);

int getResizedPhoto(int photoFileID,int sizeType,int preview);
int getFullTextCompanion(int fileID,int preview,int key);
int getVideoSnapshot(int filmID,int preview);

//int syncFolder(char *parentfolder,Browse *browse,Local *local);

int handle_error(int code,char *type);
int getParentID(char *path);
long int check_server_space(char *username);
//int write_log(Browse *bs);
int write_log(int status,char *message,char *filename);
int write_finish_log();
int write_system_log(char *action,char *name);
int write_confilicted_log(char *prename,char *confilicted_name);
int write_trans_excep_log(char *fullname,int type,char *msg);
int sync_all_item(char *dir,int parentID);
int sync_all_item_uploadonly(char *dir,int parentID);
int add_all_download_only_socket_list(char *cmd,const char *dir);
int add_all_download_only_dragfolder_socket_list(const char *dir);
int handle_rename(int parentID,char *fullname,int type,char *prepath,int is_case_conflict,char *pre_name);
int handle_createfolder_fail_code(int status,int parent_ID,char *path,char* fullname);
int handle_upload_fail_code(int status,int parent_ID,char* fullname,const char *path);
int handle_delete_fail_code(int status);
int handle_rename_fail_code(int status,int parentID,char *fullname,char *path,int isfolder);
int handle_move_fail_code(int status,char *path,char *fullname,int parentID,char *prepath,int entryID,int isfolder);
int create_server_folder_r(const char *path);
int upload_entry(char *fullname,int parent_ID,char *path);

int StrToHex(char *src,int len);

int IsEntryDeletedFromServer(int fileID,int isfolder);
int obtain_token_from_file(const char *filename,Aaa *aaa);

int count_call_api(const char *url);
void print_count_call_api();

int getRouterInfo(RouterInfo *ri, struct aicloud_conf *ai_conf);
int parseRouterInfoDoc(char *docname,void *obj);
void parseRouterInfo(xmlDocPtr doc, xmlNodePtr cur,RouterInfo *ri);
void parseDiskSpace(xmlDocPtr doc, xmlNodePtr cur, RouterInfo *ri);
void parseDiskSpaceItem(xmlDocPtr doc, xmlNodePtr cur, RouterInfo *ri);

int getSgToken(Aaa *aaa, struct aaews_provision_conf *ap_conf);
int parseSgTokenDoc(char *docname,void *obj);
void parseSgToken(xmlDocPtr doc, xmlNodePtr cur,Aaa *aaa);

int aaewsProvisionEdit(struct aaews_provision_conf *ap_conf);

int httpPut(char* api_url,char* filename, struct aicloud_conf *ai_conf);

char *str_replace (char *source, char *find,  char *rep);

void checkAaewsRun();

char * accountEncodeBase64(struct aicloud_conf *ai_conf);

int writeAaewsRefreshToken();

int checkAaewsRefreshTokenOk();

void refreshTokenDel();

void substr(char *dest, const char* src, unsigned int start, unsigned int cnt);

int ipcamConfigXmlParse();

int fbRtmpRun();

int base_exec(char *) ;

void fbRtmpThread();

void debugLogLevel(int debug_level, int debug_type, char * log_value);

int systemMacAddressEdit(struct system_conf *s_conf, char * file_name);

//int fbRtmp(void);

#endif

