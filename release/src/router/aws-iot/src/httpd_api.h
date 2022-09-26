#ifndef HTTPD_API_H_
#define HTTPD_API_H_

#include "json.h"

 
#define HTTPD_URL 								"http://127.0.0.1/"
#define API_LOGIN					 			"login.cgi"
#define API_GET_EPTOKEN				 			"get_eptoken.cgi"


#define HTTPD_RESPONSE_FILE 						"/tmp/httpd_response.json"

// CURL error code
#define CURL_CANNOT_RESOLVE_HOSTNAME    		6
#define CURLE_COULDNT_CONNECT   				7
#define CURL_CONNECT_TIMEOUT           			28
#define CURLE_SSL_CONNECT_ERROR        			35

#define HTTP_REQUEST_GET               			"1"
#define HTTP_REQUEST_POST             			"2"
#define HTTP_REQUEST_DELETE             		"3"

#define FILE_PARSE_ERROR                    	-1
#define PROCESS_SUCCESS                      	0

#define CURL_RETRY_TIME  		60
#define EXIT_PROGRAM   			-999


#define USERSIZE 				64
#define MIDSIZE 				128
#define URLSIZE 				256
#define NORMALSIZE 				512
#define MAXSIZE 				2048


struct api_response_data {

	int status;
	char asus_token[256];
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

int get_eptoken_data(char * postdata);


int httpd_get_eptoken_api(const char * api_name);

int  httpd_api(const char * api_name, const char * request_data);

int http_request_process(char *response_data_file, char *url, char *postdata, 
                        char *cookie, char *header, char * request_type);


int send_request(char *filename, char *url, char *postdata, 
                 char *cookie,  char *header, char * request_type);



int handle_error(int code, char *type);

int parse_responae_json(const char *json_file, const char *api_name);

int parse_json(json_object *jobj, const char *api_name);

void http_passwd_default(char *login_authorization);

char * base64enc(const char *p, char *buf, int len);

#endif /* HTTPD_API_H_ */
