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

#define RESPONSE_DATA_FILE 						"/tmp/api_response_data.json"
#define S3_RESPONSE_FILE 						"/tmp/s3_response_data"

#define UPLOADER_FOLDER 						"/tmp/uploader/"


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

#define CLOUD_FILE_NUMBER  						5

#define URLSIZE 				256
#define NORMALSIZE 				512
#define MAXSIZE 				2048

#define SID 					"1004"

#define EXIT_UPLOADER   		-999

#define CURL_RETRY_TIME  		60


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
	int files_len;
	char files_content[2048];

};

int upload_file(const char * cusid, const char * user_ticket, const char * devicehashmac, 
                const char *file_path, const char *file_name);

int check_file_list(const char * cusid, const char * user_ticket, const char * devicehashmac, 
                    const char *file_path, const char *file_name);

int get_aicamcloud_api_info(const char * api_name , const char * cusid, 
                            const char * user_ticket, const char * devicehashmac, 
                            const char *upload_path, const char *upload_name);

int http_request_process(char *response_data_file, char *url, char *postdata, 
                        char *cookie, char *header, char * request_type);

int send_request(char *filename, char *url, char *postdata, 
                 char *cookie,  char *header, char * request_type);

size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stream);

int upload_file_to_s3(char *resposne_file, char *upload_file, char *header);


int handle_error(int code, char *type);

int parse_responae_json(const char *json_file, const char *api_name);

int parse_json(json_object *jobj, const char *api_name);


#endif /* UPLOAD_API_H_ */