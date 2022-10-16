
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>

#include <dirent.h>
#include <errno.h>

// #ifdef __cplusplus  
// extern "C" {  
// #endif
// #include <bcmnvram.h>
// #ifdef __cplusplus  
// }
// #endif

#include <shared.h>


#include <curl/curl.h>
#include <sys/stat.h>


#include "awsiot_config.h"

#include "httpd_api.h"
#include "awsiot.h"
#include "log.h"
#include "json.h"



#define API_DBG 1


struct api_response_data api_response_data;


int get_eptoken_data(char * postdata) {

  char aae_deviceid[64];
  char oauth_dm_refresh_ticket[384];
  char oauth_user_email[384];
  char oauth_dm_cusid[96];


  char timestamp_str[16] = {0};
  getTimeInMillis(timestamp_str);

  snprintf(aae_deviceid, sizeof(aae_deviceid), "%s", nvram_safe_get("aae_deviceid"));
  snprintf(oauth_user_email, sizeof(oauth_user_email), "%s", nvram_safe_get("oauth_user_email"));
  snprintf(oauth_dm_refresh_ticket, sizeof(oauth_dm_refresh_ticket), "%s", nvram_safe_get("oauth_dm_refresh_ticket"));
  snprintf(oauth_dm_cusid, sizeof(oauth_dm_cusid), "%s", nvram_safe_get("oauth_dm_cusid"));


  LogInfo( ( "timestamp_str : %s", timestamp_str) );
  LogInfo( ( "aae_deviceid : %s", aae_deviceid) );
  LogInfo( ( "oauth_dm_cusid : %s", oauth_dm_cusid) );
  LogInfo( ( "oauth_user_email : %s", oauth_user_email) );
  LogInfo( ( "oauth_dm_refresh_ticket : %s", oauth_dm_refresh_ticket) );

  char deviceid_md5[33] = {0};
  char user_email_md5[33] = {0};
  char refresh_ticket_md5[33] = {0};

  snprintf(deviceid_md5, sizeof(deviceid_md5), "%s", str2md5(aae_deviceid, strlen(aae_deviceid)));
  snprintf(user_email_md5, sizeof(user_email_md5), "%s", str2md5(oauth_user_email, strlen(oauth_user_email)));
  snprintf(refresh_ticket_md5, sizeof(refresh_ticket_md5), "%s", str2md5(oauth_dm_refresh_ticket, strlen(oauth_dm_refresh_ticket)));


  toUpperCase(deviceid_md5);
  toUpperCase(user_email_md5);
  toUpperCase(refresh_ticket_md5);

  LogInfo( ( "deviceid_md5 : %s", deviceid_md5) );
  LogInfo( ( "user_email_md5 : %s", user_email_md5) );
  LogInfo( ( "refresh_ticket_md5 : %s", refresh_ticket_md5) );

  int outLen=0;

  unsigned char epid[256] = {0};
  unsigned char epusr[256] = {0};
  unsigned char epts[256] = {0};

  snprintf(epid, sizeof(epid), "%s", AES_BASE64_Encrypt( (unsigned char *) deviceid_md5, strlen(deviceid_md5), (unsigned char *) refresh_ticket_md5, strlen(refresh_ticket_md5), &outLen));
  snprintf(epusr, sizeof(epusr), "%s", AES_BASE64_Encrypt( (unsigned char *) user_email_md5, strlen(user_email_md5), (unsigned char *) refresh_ticket_md5, strlen(refresh_ticket_md5), &outLen));
  snprintf(epts, sizeof(epts), "%s", AES_BASE64_Encrypt( (unsigned char *) timestamp_str, strlen(timestamp_str), (unsigned char *) refresh_ticket_md5, strlen(refresh_ticket_md5), &outLen));


  snprintf(postdata, MAXSIZE, "{\"epid\":\"%s\",\"epusr\":\"%s\", \"epts\":\"%s\"}", 
                            epid, epusr, epts);

  LogInfo( ( "postdata : %s", postdata) );

  return 0;
}


int httpd_get_eptoken_api(const char * api_name)
{




  int status;

  status = httpd_api(API_GET_EPTOKEN, "");

  if(status == PROCESS_SUCCESS) {

    // status = httpd_api(API_GET_EPTOKEN);

    // if(status == PROCESS_SUCCESS) {



    // } else {

    // }

  } else {

  }


  return PROCESS_SUCCESS;
}



int httpd_api(const char * api_name, const char * request_data)
{

  char url[URLSIZE];
  char header[NORMALSIZE];
  char cookie[NORMALSIZE];
  char postdata[MAXSIZE] = {0};


  snprintf(header,NORMALSIZE,"content-type: application/json\nUser-Agent:asusrouter-Windows-DUTUtil-1.0");

  if(strcmp(api_name, API_LOGIN) == 0) {

    char login_authorization[MIDSIZE];
    // http_passwd_default(login_authorization);
    // snprintf(url,URLSIZE, "%s%s?login_authorization=%s", HTTPD_URL, api_name, login_authorization); 
    snprintf(url,URLSIZE, "%s%s?login_authorization=%s", HTTPD_URL, api_name, "YWRtaW46MTExMTExMTE="); 

  } else if(strcmp(api_name, API_GET_EPTOKEN) == 0) {

    snprintf(url,URLSIZE, "%s%s", HTTPD_URL, API_GET_EPTOKEN); 
    // snprintf(cookie,NORMALSIZE,"asus_token==%s", "YWRtaW46MTExMTExMTE=");
    // snprintf(cookie,NORMALSIZE,"asus_token=%s", api_response_data.asus_token);
    // get_eptoken_data(postdata);
    snprintf(postdata,URLSIZE, "%s", request_data); 

  }

  
  LogInfo( ( "cookie : %s", cookie) );
  LogInfo( ( "httpd_api postdata : %s", postdata) );
  LogInfo( ( "Get url : %s", url) );


  Cdbg(API_DBG, "url : %s", url);
  // Cdbg(API_DBG, "postdata strlen : %d, content : \n%s", strlen(postdata), postdata);

  int status = -1;
  status = http_request_process(HTTPD_RESPONSE_FILE, url, postdata, 
                                  cookie, header, HTTP_REQUEST_GET);

  // process : http response status 
  if(status == 200) {

    if(parse_responae_json(HTTPD_RESPONSE_FILE, api_name) == FILE_PARSE_ERROR)
    {

      Cdbg(API_DBG, "%s response_json parse error : %d", api_name, status);

      // error : json content
      return FILE_PARSE_ERROR;

    } else {

      Cdbg(API_DBG, "%s response_json parse success", api_name);
      // remove(HTTPD_RESPONSE_FILE);

      // response_data_process(API_ROUTER_CHECK_TOKEN);

    }
  }

  return PROCESS_SUCCESS;
}


int http_request_process(char *response_data_file, char *url, char *postdata, 
                         char *cookie, char *header, char * request_type) 
{

  int curl_status = -1;
  int curl_retry_count = 0;

  do {

    
    curl_status = send_request(response_data_file, url, postdata, cookie, header, request_type);
    
    // curl error code :  <= 100
    if( curl_status <= 100) {

      curl_retry_count++;

      handle_error(curl_status,"curl");

      Cdbg(API_DBG, "CURL error -> curl_status =  %d", curl_status);


      sleep(CURL_RETRY_TIME * curl_retry_count);

      if(curl_retry_count > 5) {

        Cdbg(API_DBG, "CURL retry upper limit, exit awsiot");

        return EXIT_PROGRAM;
      }

      continue;

    }

  } while ( curl_status <= 100);

  return curl_status;
}



int send_request(char *filename, char *url, char *postdata, 
                 char *cookie,  char *header, char * request_type)  
{


    // curl_response_status = 0;
    FILE *fd;
    CURL *curl;
    CURLcode res = CURLE_FAILED_INIT;
    // char cookies[NORMALSIZE];
    // char err_message[NORMALSIZE] = {0};

    struct curl_slist *headers = NULL;

    fd = fopen(filename,"w");
    if(NULL == fd) {
        Cdbg(API_DBG, "open %s file fail\n",filename);
        return -1;
    }


    curl = curl_easy_init();

    // 1 : GET , 2 : POST && ( header -> Content-Type: application/x-www-form-urlencoded )
    // 3 : POST && ( header -> content-type: application/json )
    if( strcmp(request_type, HTTP_REQUEST_GET) == 0) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
    } else if( strcmp(request_type, HTTP_REQUEST_POST) == 0) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
    } else if( strcmp(request_type, HTTP_REQUEST_DELETE) == 0) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    }
    

    curl_easy_setopt(curl, CURLOPT_URL, url);

    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
    // curl_easy_setopt(curl,CURLOPT_ERRORBUFFER,err_message);
    // curl_easy_setopt(curl,CURLOPT_FAILONERROR,1);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1);
    //curl_easy_setopt(curl,CURLOPT_SSL_VERIFYHOST,0);
    //curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);

    //curl_easy_setopt(curl,CURLOPT_TIMEOUT,30); // 90 -> 60
    curl_easy_setopt(curl,CURLOPT_CONNECTTIMEOUT, 60); // 2017/07/31 add

    // abort if slower than 30 bytes/sec during 60 seconds 
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 60L);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 60L);

    // curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    // curl_easy_setopt(curl,CURLOPT_READDATA,fd);

    curl_easy_setopt(curl,CURLOPT_WRITEDATA,fd);

    // curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)file_info.st_size);
    // curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (curl_off_t)file_info.st_size);


    if(header != NULL)
    {
        headers = curl_slist_append(headers, header);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    } 

    if(cookie != NULL)
    {
        curl_easy_setopt(curl,CURLOPT_COOKIE,cookie);
    }

    if(postdata != NULL)
    {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postdata);
    } 

    res = curl_easy_perform(curl);

    long response_code;

    if( res != CURLE_OK )
    {
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
      response_code = res;

    } else {

      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    }

    if(header)
        curl_slist_free_all(headers);


    curl_easy_cleanup(curl);

    fclose(fd);

    return response_code;
}



int handle_error(int code, char *type)
{

    Cdbg(API_DBG, "code is %d, type is %s", code, type);

    char error_message[NORMALSIZE];
    memset(error_message,0,sizeof(error_message));

    switch (code) {

    case CURL_CANNOT_RESOLVE_HOSTNAME:

        snprintf(error_message, NORMALSIZE, "type is %s, code is %d, Couldn't resolve host. The given remote host was not resolved.", type, code);
        break;

    case CURLE_COULDNT_CONNECT:

        snprintf(error_message, NORMALSIZE, "type is %s, code is %d, Failed to connect() to host or proxy", type, code);
        break;

    case CURL_CONNECT_TIMEOUT:

        snprintf(error_message, NORMALSIZE, "type is %s, code is %d, can't connect to host,please check connection", type, code);
        break;

    case CURLE_SSL_CONNECT_ERROR:
        
        snprintf(error_message, NORMALSIZE, "type is %s, code is %d, ssl connect error, please check connection", type, code);
        break;


    default:
        break;
    }

    Cdbg(API_DBG, "%s", error_message);

    return 0;
}




int parse_responae_json(const char *json_file, const char *api_name) 
{

  int status;
  int ret = 0;

  json_object * jobj = json_object_from_file(json_file);

  if (!jobj) {
    Cdbg(API_DBG, "json file parse failure");
    return -1;
  }

  status = parse_json(jobj, api_name);

  return status;
}




int parse_json(json_object *jobj, const char *api_name)
{

  int i;
  int ret = 0;

  if (!jobj) {
    Cdbg(API_DBG, "json file parse failure");
    return -1;
  }

  char type_str[64];;
  memset(type_str,0,sizeof(type_str));

  enum json_type type;

  int orig_count = 0;
    
  json_object_object_foreach(jobj, key, val) {

    type = json_object_get_type(val);

    switch (type) {
      case json_type_string: 
        snprintf(type_str, 64, "type : json_type_string");
        break;
      case json_type_null:
        snprintf(type_str, 64, "type : json_type_null");
        break;
      case json_type_boolean:
        snprintf(type_str, 64, "type : json_type_boolean");
        break;
      case json_type_double: 
        snprintf(type_str, 64, "type : json_type_double");
        break;
      case json_type_int:
        snprintf(type_str, 64, "type : json_type_int");
        break;
      case json_type_object:
        snprintf(type_str, 64, "type : json_type_object");
        break;
      case json_type_array:
        snprintf(type_str, 64, "type : json_type_array");
        break;
    }

    orig_count++;

    Cdbg(API_DBG, "api_name = %s, %s , key: \"%s\" at index %d, type of val: %s ", api_name, type_str, key, orig_count, json_object_get_string(val));

    if (strcmp(api_name, API_LOGIN) == 0) {

      if (strcmp(key, "status") == 0) {
        api_response_data.status = atoi(json_object_get_string(val));
        continue;
      } else if (strcmp(key, "asus_token") == 0) {
        snprintf(api_response_data.asus_token, sizeof(api_response_data.asus_token), json_object_get_string(val));
      }


    } else if (strcmp(api_name, API_GET_EPTOKEN) == 0) {


      if (strcmp(key, "files") == 0) {


        struct json_object *files_array;

        // files_array = json_object_object_get(jobj, "files");

        json_object_object_get_ex(jobj, "files", &files_array);

        // files_array is an array of objects
        int arraylen = json_object_array_length(files_array);



        Cdbg(API_DBG, "arraylen =  %d ", arraylen);

        api_response_data.files_len = arraylen;
        // snprintf(api_response_data.files_content, sizeof(api_response_data.files_content), "%s", json_object_get_string(val));
        snprintf(api_response_data.files_content, sizeof(api_response_data.files_content), json_object_get_string(val));
      }

    }

  }
  return 0;
}


void http_passwd_default(char *login_authorization) {

  char http_username[64];
  char http_passwd[64];
  char user_pwd[USERSIZE];

  char * http_passwd_t = nvram_safe_get("http_passwd");


  LogInfo( ( "Get login_authorization = %s", login_authorization) );
  LogInfo( ( "Get http_passwd_t = %s", http_passwd_t) );

  pw_dec(http_passwd_t, http_passwd, 64);


  LogInfo( ( "Get http_passwd = %s", http_passwd) );


  snprintf(http_username, USERSIZE, "%s", nvram_safe_get("http_username"));
  snprintf(user_pwd, MIDSIZE, "%s:%s", http_username, http_passwd); 


  LogInfo( ( "Get http_username = %s, http_passwd_t = %s, dec_passwd = %s, user_pwd = %s", http_username, http_passwd_t, http_passwd, user_pwd) );


  base64enc(user_pwd, login_authorization, MIDSIZE);

  LogInfo( ( "Get login_authorization = %s", login_authorization) );

}


char * base64enc(const char *p, char *buf, int len)
{
  char al[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
    "0123456789+/";
  char *s = buf;

  while (*p) {
    if (s >= buf+len-4)
      break;
    *(s++) = al[(*p >> 2) & 0x3F];
    *(s++) = al[((*p << 4) & 0x30) | ((*(p+1) >> 4) & 0x0F)];
    *s = *(s+1) = '=';
    *(s+2) = 0;
    if (! *(++p)) break;
    *(s++) = al[((*p << 2) & 0x3C) | ((*(p+1) >> 6) & 0x03)];
    if (! *(++p)) break;
    *(s++) = al[*(p++) & 0x3F];
  }

  return buf;
}



