#ifndef UPLOAD_API_H_
#define UPLOAD_API_H_
 


#include "json.h"

#define AICAMCLOUD_URL 							"https://aicamcloud.asuscomm.com/"
#define API_ROUTER_CHECK_TOKEN 					"router_check_token"
#define API_ROUTER_GET_UPLOAD_FILE_URL 			"router_get_upload_file_url"
#define API_ROUTER_UPLOAD_FILE 					"router_upload_file"
#define API_ROUTER_GET_FILE						"router_get_file"
#define API_ROUTER_LIST_FILE					"router_list_file"
#define API_ROUTER_DEL_FILE 					"router_del_file"

#define DEBUG_TOKEN_URL							"https://outage.asuscomm.com/getS3Cert.php"
#define API_EX_DB_BACKUP						"ex_db_backup"

#define RESPONSE_DATA_FILE 						"/tmp/api_response_data.json"
#define S3_RESPONSE_FILE 						"/tmp/s3_response_data"
#define UPLOADER_FOLDER 						"/tmp/uploader/"
#define LS_LAST_TIME	 						"/tmp/uploader/ls_last_time"


#define UPLOAD_FILE_TO_S3 			 	"upload_file"
#define DEL_S3_FILE 			   		"delete_file"
#define GET_LIST_FILE 			 		"get_list_file"
#define GET_FILE_INFO 			 		"get_file_info"

#define EX_BACKUP_DB_PATH 			 		"/jffs/.diag/"

#define EX_BACKUP_LIST 				 		"ex_backup_list"

#define AMASCNTRL_BACKUP_PATH				"/jffs/.sys/cfg_mnt/"


#define TYPE_NUMBER											 6

#define SETTING_BACKUP_TYPE									 "setting"
#define OPENVPN_BACKUP_TYPE								 	 "openvpn"
#define IPSEC_BACKUP_TYPE									 "ipsec"
#define USERICON_BACKUP_TYPE								 "usericon"
#define UI_SUPORT_BACKUP_TYPE								 "ui_support"
#define AMASCNTRL_BACKUP_TYPE								 "amascntrl"


#define SETTING_FILE_LIMIT									 3
#define OPENVPN_FILE_LIMIT									 3
#define IPSEC_FILE_LIMIT									 3
#define USERICON_FILE_LIMIT									 3
#define UI_SUPORT_FILE_LIMIT								 1
#define AMASCNTRL_FILE_LIMIT								 3
    
#define HTTP_REQUEST_GET               			"1"
#define HTTP_REQUEST_POST             			"2"
#define HTTP_REQUEST_DELETE             		"3"


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

#define URLSIZE 				256
#define NORMALSIZE 				512
#define MAXSIZE 				2048

#define SID 					"1004"

#define EXIT_UPLOADER   		-999

#define CURL_RETRY_TIME  		60

#define UPLOAD_WAITING_TIME					30



#define FILENAME_LEN 64
#define FILEPATH_LEN 128
#define TYPE_LEN 32

#define FILENAME_NUMBER 6

struct api_response_data {

	int status;
	char access_token[384];

  char url[192];
	char key[192];
	char x_amz_algorithm[32];
	char x_amz_credential[128];
	char x_amz_date[32];
	char policy[576];
	char x_amz_signature[128];
};



struct cloud_files_l_list_bak {

	char file_type[TYPE_LEN];
	int file_len;
	char filename[FILENAME_NUMBER][64];
};


typedef struct _backup_files backup_files;

struct _backup_files {
	char filename[FILENAME_LEN];
	backup_files *next;
};


typedef struct _cloud_files_list cloud_files_list;

struct _cloud_files_list {
	int file_len;
	int file_len_limit;
	char file_type[TYPE_LEN];
	backup_files *bf;
	cloud_files_list *next;
};



int upload_file(const char * cusid, const char * user_ticket, const char * devicehashmac, 
                const char *file_path, const char *file_name);

void reorganization_cloud_files();

backup_files *delete_backup_file(backup_files *files, backup_files *delete_file);

int get_cloud_file_list();

int cloud_file_process(const char * api, const char * file_path, const char * file_name);

int call_cloud_api(const char * api_name , const char * cusid, 
                            const char * user_ticket, const char * devicehashmac, 
                            const char *upload_path, const char *upload_name);

int http_request_process(char *response_data_file, char *url, char *postdata, 
                        char *cookie, char *header, char * request_type);

int send_request(char *filename, char *url, char *postdata, 
                 char *cookie,  char *header, char * request_type);

size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stream);

int upload_file_to_s3(char *resposne_file, char *upload_file, char *header);

int backup_file_to_s3(char *resposne_file, char *upload_file);

void handle_error(int code, char *type);

int parse_responae_json(const char *json_file, const char *api_name);

int parse_json(json_object *jobj, const char *api_name);

int string_compare(const void *arg1, const void *arg2);

int upload_backup_file_to_cloud(const char *filename);


void backup_file_plus(char *type, char *filename);

void cloud_files_init(char *filename);

backup_files *insert_backup_file(backup_files *backup_file, const int file_len, const char *filename);

cloud_files_list *backup_file_init();


void free_files_list(cloud_files_list *first);

void free_backup_files(backup_files *first);

void print_cloud_files_list(cloud_files_list *);

void print_backup_files_list(backup_files *first);

void ex_db_backup();

int get_ts_today_at_midnight(void);

void backup_db_process(char *dir);

int compare_backup_list(char * backup_db_path, char * backup_db_name);

int write_backup_db(char * backup_db_path, char * backup_db_name);

int reconstruct_ex_backup_db_files(const char * backup_db_path, const int backup_db_number, const int upload_time);

char *get_extension(char *fileName);

int update_ui_support_db(char *fileName);

#endif /* UPLOAD_API_H_ */