#ifndef UPLOAD_API_H_
#define UPLOAD_API_H_

#include <json.h>

#define TRANSFER_TYPE		"https://"

#define DEBUG_TOKEN_URL		"https://outage.asuscomm.com/getS3Cert.php"

#define UPLOADER_FOLDER 	"/tmp/uploader/"
#define LS_LAST_TIME	 	"/tmp/uploader/ls_last_time"

#define EX_BACKUP_DB_PATH 	"/jffs/.diag/"

#define EX_BACKUP_LIST 		"ex_backup_list"

#define AMASCNTRL_BACKUP_PATH "/jffs/.sys/cfg_mnt/"

#define HTTP_REQUEST_GET     "GET"
#define HTTP_REQUEST_POST    "POST"
#define HTTP_REQUEST_DELETE  "DELETE"

// CURL error code
#define CURL_CANNOT_RESOLVE_HOSTNAME    		6
#define CURLE_COULDNT_CONNECT   				7
#define CURL_CONNECT_TIMEOUT           			28
#define CURLE_SSL_CONNECT_ERROR        			35

#define S_AUTH_FAIL                     		2

#define FILE_PARSE_ERROR                    	-1
#define PROCESS_SUCCESS                      	0
#define SUCCESS                 		     	0

#define CLOUD_FILE_NUMBER  						12

#define SID 					"1004"

#define EXIT_UPLOADER   		-999

#define CURL_RETRY_TIME  		60

#define UPLOAD_WAITING_TIME		30

#define MAX_RESET_TIME_BACKUP_NUM 2

#define MAX_TIMESTAMP_LEN       12
#define MAX_MAC_LEN             18
#define MAX_TYPE_LEN            16
#define MAX_URL_LEN 		    64
#define MAX_STATUS_LEN		    32
#define MAX_DESC_LEN		    512
#define MAX_FILENAME_LEN	    64
#define MAX_FILEPATH_LEN	    128
#define MAX_CA_DATA_LEN		    2048
#define MAX_FILE_EXT_LEN	    16
#define MAX_HEADER_LEN	        32
#define MAX_COOKIE_LEN	        32
#define MAX_MESSAGE_LEN	        64
#define MAX_CMD_LEN	            128

#define MAX_KEY_LEN		        128
#define MAX_ALGORITHM_LEN		20
#define MAX_CREDENTIAL_LEN		64
#define MAX_DATE_LEN		    18
#define MAX_POLICY_LEN		    576
#define MAX_SIGNATURE_LEN		128
////////////////////////////////////////////////////////////////////////////////

#define SETTING_BACKUP_TYPE	  "setting"
#define OPENVPN_BACKUP_TYPE	  "openvpn"
#define IPSEC_BACKUP_TYPE	  "ipsec"
#define USERICON_BACKUP_TYPE  "usericon"
#define UISUPORT_BACKUP_TYPE  "ui_support"
#define AMASCNTRL_BACKUP_TYPE "amascntrl"

#define CLOUD_FILE_PREFIX_NAME_SETTING	  "setting"
#define CLOUD_FILE_PREFIX_NAME_OPENVPN	  "openvpn"
#define CLOUD_FILE_PREFIX_NAME_IPSEC	  "ipsec"
#define CLOUD_FILE_PREFIX_NAME_USERICON   "usericon"
#define CLOUD_FILE_PREFIX_NAME_UISUPORT   "uisupport"
#define CLOUD_FILE_PREFIX_NAME_AMASCNTRL  "amascntrl"

#define CLOUD_FILE_EXT_CFG "cfg"
#define CLOUD_FILE_EXT_COMPRESS "compress"
#define CLOUD_FILE_EXT_JSON "json"

enum {
	TYPE_SETTING = 1,
	TYPE_IPSEC,
	TYPE_OPENVPN,
	TYPE_USERICON,
	TYPE_UISUPORT,
	TYPE_AMASCNTRL
};
////////////////////////////////////////////////////////////////////////////////

#define REQ_ROUTER_CHECK_TOKEN			"/router_check_token"
#define REQ_ROUTER_LIST_FILE			"/router_list_file"
#define REQ_ROUTER_DEL_FILE			    "/router_del_file"
#define REQ_ROUTER_GET_UPLOAD_FILE_URL	"/router_get_upload_file_url"
#define REQ_ROUTER_UPLOAD_FILE	        "/router_upload_file"

typedef int (*PROC_JSON_DATA)(void* data_struct, struct json_object * json_obj);

/**
	Retreive a string value to specific buffer. The buffer is a char pointer.
 */
#define GET_JSON_STRING_FIELD_EX(json_obj, name, buffer) { \
			struct json_object *obj = NULL;\
			json_object_object_get_ex(json_obj, name, &obj); \
			if (obj) buffer = (char *)json_object_get_string(obj); \
		}

/**
	Retreive a string value to specific array buffer. The buffer is a char array.
 */
#define GET_JSON_STRING_FIELD_TO_ARRARY(json_obj, name, dest) {\
			char *tmp = NULL; \
			GET_JSON_STRING_FIELD_EX(json_obj, name, tmp); \
			if (tmp) { \
				snprintf(dest, sizeof(dest), "%s", tmp); \
        	} \
		}

typedef enum _WS_ID {
	e_router_check_token,
	e_router_list_file,
	e_router_del_file,
	e_router_get_upload_file_url,
	e_router_upload_file,
	e_router_get_debug_token
} WS_ID;

typedef struct _WS_MANAGER{
	WS_ID 	ws_id;
	void*	ws_storage;
	size_t	ws_storage_size;
	char*	response_buff;
	size_t	response_size;
} WS_MANAGER;

typedef struct _RouterCheckToken {
	char 	status[MAX_STATUS_LEN];
	char	access_token[MAX_DESC_LEN];
} RouterCheckToken, *pRouterCheckToken;

typedef struct _RouterListFile {
	char 	status[MAX_STATUS_LEN];
	char	files[MAX_CA_DATA_LEN];
} RouterListFile, *pRouterListFile;

typedef struct _RouterDelFile {
	char 	status[MAX_STATUS_LEN];
} RouterDelFile, *pRouterDelFile;

typedef struct _RouterUploadFile {
	char 	status[MAX_STATUS_LEN];
} RouterUploadFile, *pRouterUploadFile;

typedef struct _RouterGetUploadFileUrl {
	char 	status[MAX_STATUS_LEN];
	char 	url[MAX_URL_LEN];
	char 	key[MAX_KEY_LEN];
	char 	x_amz_algorithm[MAX_ALGORITHM_LEN];
	char 	x_amz_credential[MAX_CREDENTIAL_LEN];
	char 	x_amz_date[MAX_DATE_LEN];
	char 	policy[MAX_POLICY_LEN];
	char 	x_amz_signature[MAX_SIGNATURE_LEN];
} RouterGetUploadFileUrl, *pRouterGetUploadFileUrl;

typedef struct _RouterGetDebugToken {
	char 	status[MAX_STATUS_LEN];
	char 	x_amz_credential[MAX_CREDENTIAL_LEN];
	char 	x_amz_date[MAX_DATE_LEN];
	char 	policy[MAX_POLICY_LEN];
	char 	x_amz_signature[MAX_SIGNATURE_LEN];
} RouterGetDebugToken, *pRouterGetDebugToken;
////////////////////////////////////////////////////////////////////////////////

typedef struct _backup_files {
	char filename[MAX_FILENAME_LEN];
	int reset_time;
	int upload_time;
	struct _backup_files *next;
} backup_files;

typedef struct _backup_file_types {
	int id;
	char name[MAX_TYPE_LEN];
    backup_files *bf;
    int bf_len;
    char bf_name[MAX_TYPE_LEN];
    char bf_ext[10];
    int max_file_limit;
} backup_file_types;
////////////////////////////////////////////////////////////////////////////////

int send_router_check_token_req(
	const char *server,
  	const char *cusid, 
  	const char *user_ticket,
  	RouterCheckToken *pData);

int send_router_list_file_req(
	const char *server,
	const char *access_token,
	RouterListFile *pData);

int send_router_del_file_req(
	const char *server,
	const char *access_token,
	const char *file_name,
	RouterDelFile *pData);

int send_router_get_upload_file_url_req(
	const char *server,
	const char *access_token,
	const char *file_name,
	RouterGetUploadFileUrl *pData);

int send_router_upload_file_req(
	const char *server,
	const char *access_token,
	const char *file_name,
	RouterUploadFile *pData);

int send_router_get_debug_token_req(
	const char *server,
	RouterGetDebugToken *pData);

int send_req(const char* request_type, 
			 const char* url, 
			 char* append_data, 
			 char *wb_custom_hdr, 
			 char* cookie, 
			 char** response_data);

int http_request_process(const char* url, 
						 char* postdata, 
						 char* wb_custom_hdr, 
						 char* cookie, 
						 char* request_type, 
						 char** response_data);
////////////////////////////////////////////////////////////////////////////////

int get_cloud_access_token(char* access_token);
int list_cloud_file(const char* access_token);
void reorganize_cloud_file(const char* access_token);
int delete_cloud_file(const char* access_token, const char *file_name);
void backup_cloud_file(const char* access_token);
int upload_cloud_file(const char *access_token, const char *file_name);
int clear_cloud_file();
////////////////////////////////////////////////////////////////////////////////

int generate_backup_file(backup_file_types *handler, char *filename, int update_time);
backup_files *insert_backup_file(backup_files *backup_file, const int file_len, const char *filename);
backup_files *delete_backup_file(backup_files *files, backup_files *delete_file);
////////////////////////////////////////////////////////////////////////////////

size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stream);

int upload_file_to_s3(const char* url, 
	                  const char* content_type,
	                  const char* meta_uuid,
					  const char* key,
					  const char* algorithm,
					  const char* credential,
					  const char* date,
					  const char* policy,
					  const char* signature,
					  const char* server_side_encryption,
					  const char* upload_file);

void handle_error(int code);

void backup_db_file();

int get_ts_today_at_midnight(void);

void backup_db_process(char *dir);

int compare_backup_list(char * backup_db_path, char * backup_db_name);

int write_backup_db(const char * backup_db_path, const char * backup_db_name);

int reconstruct_ex_backup_db_files(const char * backup_db_path, const int backup_db_number, const int upload_time);

char *get_extension(char *fileName);

#endif /* UPLOAD_API_H_ */