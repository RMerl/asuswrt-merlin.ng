
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>

#include <curl/curl.h>
#include <sys/stat.h>


#include "upload_api.h"
#include "log.h"
#include "json.h"

#include <shared.h>

#define API_DBG 1


struct api_response_data api_response_data;



extern int backup_file_limit[TYPE_NUMBER]; 

extern char aae_webstorageinfo[128];
extern char devicemd5mac[33];
extern char mac_no_symbol[32];


char backup_file_type[TYPE_NUMBER][TYPE_LEN] = {"ipsec", "openvpn", "setting", "usericon", "ui_support", "amascntrl"};
int backup_file_limit[TYPE_NUMBER] = {3, 3, 3, 3, 1, 3};  // files limit -> "setting", "openvpn", "ipsec", "usericon", "ui_support", "amascntrl"

int backup_db_number_tmp = 0;

extern cloud_files_list *files_list_frist;

int upload_file(const char * cusid, const char * user_ticket, const char * devicehashmac, 
                const char *file_path, const char *file_name) 
{

    int status = call_cloud_api(API_ROUTER_CHECK_TOKEN , cusid, 
                                     user_ticket, devicehashmac, NULL, NULL);

    if(status != 0) {
        Cdbg(API_DBG, "Post [router_check_token] API message error, status = %d", status);
        return status;
    } else if(api_response_data.status != 0) {
        Cdbg(API_DBG, "[check_token] get data error, status = %d", api_response_data.status);
        return api_response_data.status;
    }


    status = call_cloud_api(API_ROUTER_GET_UPLOAD_FILE_URL , NULL, NULL, 
                                     devicehashmac, file_path, file_name);


    if(status != 0) {
        Cdbg(API_DBG, "Post [router_get_upload_file_url] API message error, status = %d", status);
        return status;
    } else if(api_response_data.status != 0) {
        Cdbg(API_DBG, "[router_get_upload_file_url] get data error, status = %d", api_response_data.status);
        return api_response_data.status;
    }

    // Cdbg(API_DBG, "status = %d, policy = %s", api_response_data.status, api_response_data.policy);
    // Cdbg(API_DBG, "url = %s", api_response_data.url);
    // Cdbg(API_DBG, "key = %s", api_response_data.key);
    // Cdbg(API_DBG, "x_amz_algorithm = %s", api_response_data.x_amz_algorithm);
    // Cdbg(API_DBG, "x_amz_credential = %s", api_response_data.x_amz_credential);
    // Cdbg(API_DBG, "x_amz_date = %s", api_response_data.x_amz_date);
    // Cdbg(API_DBG, "policy = %s", api_response_data.policy);
    // Cdbg(API_DBG, "x_amz_signature = %s", api_response_data.x_amz_signature);


    char upload_file[FILEPATH_LEN];
    snprintf(upload_file, sizeof(upload_file) ,"%s%s", file_path, file_name);


    status = upload_file_to_s3(S3_RESPONSE_FILE, upload_file, NULL);

    Cdbg(API_DBG, "S3_RESPONSE_FILE = %s, status = %d", S3_RESPONSE_FILE, status);

    if(status != 204) {
        Cdbg(API_DBG, "Post [upload_file_to_s3] API message error, status = %d", status);
        return status;
    } 


    status = call_cloud_api(API_ROUTER_UPLOAD_FILE , NULL, NULL, 
                                 devicehashmac, file_path, file_name);

    if(status != 0) {
        Cdbg(API_DBG, "Post [router_upload_file] API message error, status = %d", status);
        return status;
    } else if(api_response_data.status != 0) {
        Cdbg(API_DBG, "[router_upload_file] get data error, status = %d", api_response_data.status);
        return api_response_data.status;
    }

}


int delete_cloud_file(char *type, char *filename) {


  if (!strstr(filename,  mac_no_symbol) ) {
    Cdbg(API_DBG, "mac diff , filename = %s,  mac_no_symbol = %s", filename,  mac_no_symbol);
    return -1;
  }

  int status = call_cloud_api(API_ROUTER_DEL_FILE , NULL, NULL, 
                                   devicemd5mac, NULL, filename);

  if(status == 0) {  
    Cdbg(API_DBG, "Del file SUCCESS, file type = %s, filename = %s", type, filename);
  } else {
    Cdbg(API_DBG, "Del file fail, status = %d", status);
  }

  return status;
}



void reorganization_cloud_files(){

  cloud_files_list *files_list = files_list_frist;

  while(files_list != NULL){

    // Cdbg(API_DBG, "reorganization_cloud_files file type = %s, file_len = %d, file_len_limit = %d ", files_list->file_type, files_list->file_len, files_list->file_len_limit);

    while ( files_list->file_len > files_list->file_len_limit) {

      if(delete_cloud_file(files_list->file_type, files_list->bf->filename) == 0) {
        
        // print_backup_files_list(files_list->bf);
        files_list->bf = delete_backup_file(files_list->bf, files_list->bf);
        files_list->file_len--;
        // print_backup_files_list(files_list->bf);
      }
    }

    files_list = files_list->next;
  }
}



backup_files *delete_backup_file(backup_files *files, backup_files *delete_file) {

  backup_files* bf = files;

  if(files == NULL){ 
    Cdbg(API_DBG, "backup file nothing to delete ");
    return NULL;
  }

  if(delete_file == files){  // If the first node is deleted
    Cdbg(API_DBG, "delete_file frist");
    files = files->next;//把backup_files指向下一個節點(NULL)
  }else{
    Cdbg(API_DBG, "delete_file next");
    while(bf->next != delete_file){ //找到要刪除之節點的前一個節點
        bf = bf->next;
    }
    bf->next = delete_file->next; //重新設定bf的next成員
  }

  free(delete_file);
  return files;
}


int get_cloud_file_list()
{
  int status, retry_count = 0;

  // check token
  do {

      status = call_cloud_api(API_ROUTER_CHECK_TOKEN , nvram_safe_get("oauth_dm_cusid"), 
                                       nvram_safe_get("oauth_dm_user_ticket"), devicemd5mac, NULL, NULL);

      if(status != 0) {
          Cdbg(API_DBG, "Post [router_check_token] API message error, status = %d", status);
          return -1;
      } else if(api_response_data.status != 0) {
          Cdbg(API_DBG, "[check_token] get data error, status = %d", api_response_data.status);
          update_userticket();            
          sleep(10);
          retry_count++;
          if(retry_count > 5) {
              Cdbg(API_DBG, "[check_token] error, update_userticket error, retry_count = %d", retry_count);
              return -1;
          }
          continue;
      }
      break;

  } while (1);

  Cdbg(API_DBG, "API_ROUTER_CHECK_TOKEN status = %d, access_token = %s", status, api_response_data.access_token);


  status = call_cloud_api(API_ROUTER_LIST_FILE , NULL, NULL, 
                                   devicemd5mac, NULL, NULL);

  if(status != 0) {
      Cdbg(API_DBG, "Post [router_get_upload_file_url] API message error, status = %d", status);
      return status;
  } else if(api_response_data.status != 0) {
      Cdbg(API_DBG, "[router_get_upload_file_url] get data error, status = %d", api_response_data.status);
      return api_response_data.status;
  }

  return status;
}



int cloud_file_process(const char * api, const char * file_path, const char * file_name) 
{

    int status;
    int retry_count = 0;
    char server_api[32];

    // check token
    do {

        status = call_cloud_api(API_ROUTER_CHECK_TOKEN , nvram_safe_get("oauth_dm_cusid"), 
                                         nvram_safe_get("oauth_dm_user_ticket"), devicemd5mac, NULL, NULL);

        if(status != 0) {
            Cdbg(API_DBG, "Post [router_check_token] API message error, status = %d", status);
            return -1;
        } else if(api_response_data.status != 0) {
            Cdbg(API_DBG, "[check_token] get data error, status = %d", api_response_data.status);
            update_userticket();            
            sleep(10);
            retry_count++;
            if(retry_count > 5) {
                Cdbg(API_DBG, "[check_token] error, update_userticket error, retry_count = %d", retry_count);
                return -1;
            }
            continue;
        }
        break;

    } while (1);

    Cdbg(API_DBG, "API_ROUTER_CHECK_TOKEN status = %d, access_token = %s", status, api_response_data.access_token);


    // upload file to S3
    if( strcmp(api, UPLOAD_FILE_TO_S3) == 0) {

        snprintf(server_api, sizeof(server_api) ,"%s", API_ROUTER_GET_UPLOAD_FILE_URL);


        status = call_cloud_api(API_ROUTER_GET_UPLOAD_FILE_URL , NULL, NULL, 
                                         devicemd5mac, NULL, file_name);


        if(status != 0) {
            Cdbg(API_DBG, "Post [%s] API message error, status = %d", server_api, status);
            return -1;
        } else if(api_response_data.status != 0) {
            Cdbg(API_DBG, "[%s] get data error, status = %d", server_api, api_response_data.status);
            return -1;
        }

        Cdbg(API_DBG, "status = %d, policy = %s", status, api_response_data.policy);
        Cdbg(API_DBG, "url = %s", api_response_data.url);
        Cdbg(API_DBG, "key = %s", api_response_data.key);
        Cdbg(API_DBG, "x_amz_algorithm = %s", api_response_data.x_amz_algorithm);
        Cdbg(API_DBG, "x_amz_credential = %s", api_response_data.x_amz_credential);
        Cdbg(API_DBG, "x_amz_date = %s", api_response_data.x_amz_date);
        Cdbg(API_DBG, "policy = %s", api_response_data.policy);
        Cdbg(API_DBG, "x_amz_signature = %s", api_response_data.x_amz_signature);

        char upload_file[256];
        snprintf(upload_file, sizeof(upload_file) ,"%s/%s", file_path, file_name);


        status = upload_file_to_s3(S3_RESPONSE_FILE, upload_file, NULL);

        Cdbg(API_DBG, "S3_RESPONSE_FILE = %s, status = %d", S3_RESPONSE_FILE, status);


        if(status != 204) {
            Cdbg(API_DBG, "Post [upload_file_to_s3] API message error, status = %d", status);
            return -1;
        } 


        snprintf(server_api, sizeof(server_api) ,"%s", API_ROUTER_UPLOAD_FILE);


        status = call_cloud_api(API_ROUTER_UPLOAD_FILE , NULL, NULL, 
                                     devicemd5mac, file_path, file_name);


    } else if( strcmp(api, DEL_S3_FILE) == 0) {


        snprintf(server_api, sizeof(server_api) ,"%s", API_ROUTER_DEL_FILE);

        status = call_cloud_api(API_ROUTER_DEL_FILE , NULL, NULL, 
                                         devicemd5mac, file_path, file_name);


    } else if( strcmp(api, GET_LIST_FILE) == 0) {



        snprintf(server_api, sizeof(server_api) ,"%s", API_ROUTER_LIST_FILE);

        status = call_cloud_api(API_ROUTER_LIST_FILE , NULL, NULL, 
                                         devicemd5mac, file_path, file_name);


    } else if( strcmp(api, GET_FILE_INFO) == 0) {


        snprintf(server_api, sizeof(server_api) ,"%s", API_ROUTER_GET_FILE);

        status = call_cloud_api(API_ROUTER_GET_FILE , NULL, NULL, 
                                         devicemd5mac, file_path, file_name);
    }


    if(status != 0) {
        Cdbg(API_DBG, "Post [%s] API,  message error, status = %d", server_api, status);
        return -1;
    } else if(api_response_data.status != 0) {
        Cdbg(API_DBG, "Post [%s] API,  get data error, status = %d", server_api, api_response_data.status);
        return -1;
    }

    return 0;
}





int call_cloud_api(const char * api_name , const char * cusid, 
                            const char * user_ticket, const char * devicehashmac, 
                            const char *upload_path, const char *upload_name)
{

  char url[URLSIZE];
  char header[NORMALSIZE];
  char cookie[NORMALSIZE];
  char postdata[MAXSIZE];

  snprintf(url,URLSIZE, "%s%s", aae_webstorageinfo, api_name);    // AICAMCLOUD_URL
  snprintf(header,NORMALSIZE,"content-type: application/json");
  snprintf(cookie,NORMALSIZE,"OMNISTORE_VER=1_0; path=/;sid=%s",SID);
  
  if(strcmp(api_name, API_ROUTER_CHECK_TOKEN) == 0) {

    snprintf(postdata,MAXSIZE, "{\"cusid\":\"%s\",\"user_ticket\":\"%s\",\"devicehashmac\":\"%s\", \"sid\": \"%s\"}", 
      cusid, user_ticket, devicehashmac, SID);
  
  } else if(strcmp(api_name, API_ROUTER_GET_UPLOAD_FILE_URL) == 0) {
    snprintf(postdata,MAXSIZE, "{\"devicehashmac\":\"%s\",\"access_token\":\"%s\",\"file_name\":\"%s\"}", 
      devicehashmac, api_response_data.access_token, upload_name); 
  } else if(strcmp(api_name, API_ROUTER_UPLOAD_FILE) == 0) {
    snprintf(postdata,MAXSIZE, "{\"devicehashmac\":\"%s\",\"access_token\":\"%s\",\"file_name\":\"%s\"}", 
      devicehashmac, api_response_data.access_token, upload_name); 

  } else if(strcmp(api_name, API_ROUTER_GET_FILE) == 0) {
    snprintf(postdata,MAXSIZE, "{\"devicehashmac\":\"%s\",\"access_token\":\"%s\",\"file_name\":\"%s\"}", 
      devicehashmac, api_response_data.access_token, upload_name); 

  } else if(strcmp(api_name, API_ROUTER_LIST_FILE) == 0) {
    snprintf(postdata,MAXSIZE, "{\"devicehashmac\":\"%s\",\"access_token\":\"%s\"}", 
      devicehashmac, api_response_data.access_token); 

  } else if(strcmp(api_name, API_ROUTER_DEL_FILE) == 0) {
    snprintf(postdata,MAXSIZE, "{\"devicehashmac\":\"%s\",\"access_token\":\"%s\",\"file_name\":\"%s\"}", 
      devicehashmac, api_response_data.access_token, upload_name); 
  }

  Cdbg(API_DBG, "url : %s", url);
  Cdbg(API_DBG, "postdata strlen : %d, content : \n%s", strlen(postdata), postdata);

  int status = -1;
  if(strcmp(api_name, API_ROUTER_DEL_FILE) == 0) {
    status = http_request_process(RESPONSE_DATA_FILE, url, postdata, 
                                  cookie, header, HTTP_REQUEST_DELETE);
  } else {

    status = http_request_process(RESPONSE_DATA_FILE, url, postdata, 
                                  cookie, header, HTTP_REQUEST_POST);
  } 

  // process : http response status 
  if(status == 200) {

    if(parse_responae_json(RESPONSE_DATA_FILE, api_name) == FILE_PARSE_ERROR)
    {

      Cdbg(API_DBG, "%s response_json parse error : %d", api_name, status);

      // error : json content
      return FILE_PARSE_ERROR;

    } else {

      Cdbg(API_DBG, "%s response_json parse success", api_name);
      // remove(RESPONSE_DATA_FILE);

      // response_data_process(API_ROUTER_CHECK_TOKEN);

    }

  } else {

      Cdbg(API_DBG, "%s http_request_process error status : %d", api_name, status);

  }

  return PROCESS_SUCCESS;
}



int http_request_process(char *response_data_file, char *url, char *postdata, 
                         char *cookie, char *header, char * request_type) 
{

  int curl_status = 0;
  int curl_retry_count = 0;

  do {

    curl_status = send_request(response_data_file, url, postdata, cookie, header, request_type);
    
    // curl error code :  <= 100
    if( curl_status <= 100) {

      curl_retry_count++;

      handle_error(curl_status,"curl");

      sleep(CURL_RETRY_TIME * curl_retry_count);

      if(curl_retry_count > 5) {

        Cdbg(API_DBG, "CURL retry upper limit, exit uploader");

        return EXIT_UPLOADER;
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

    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);
    // curl_easy_setopt(curl,CURLOPT_ERRORBUFFER,err_message);
    // curl_easy_setopt(curl,CURLOPT_FAILONERROR,1);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1);
    //curl_easy_setopt(curl,CURLOPT_SSL_VERIFYHOST,0);
    //curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);

    // curl_easy_setopt(curl,CURLOPT_TIMEOUT,90); // 90 -> 60
    curl_easy_setopt(curl,CURLOPT_CONNECTTIMEOUT, 90); // 2017/07/31 add

    // abort if slower than 30 bytes/sec during 60 seconds 
    // curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 60L);
    // curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 60L);

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


    curl_easy_setopt(curl,CURLOPT_WRITEDATA,fd);

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



size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
  size_t retcode;

  /* in real-world cases, this would probably get this data differently
     as this fread() stuff is exactly what the library already would do
     by default internally */
  retcode = fread(ptr, size, nmemb, stream);


  // curl_off_t nread;

  curl_off_t nread = (curl_off_t)retcode;

  // fprintf(stderr, "*** We read %" CURL_FORMAT_CURL_OFF_T " bytes from file\n", nread);

  

  return retcode;
}


int upload_file_to_s3(char *resposne_file, char *upload_file, char *header) 
{


  CURL *curl;
  CURLcode res;

  FILE * fd_src;

  struct stat file_info;

  /* get the file size of the local file */
  stat(upload_file, &file_info);

  fd_src = fopen(upload_file, "rb");

  if(NULL == fd_src)
  {
    Cdbg(API_DBG, "open %s file fail\n", upload_file);
    return -1;
  }


  long response_code;


  /* In windows, this will init the winsock stuff */
  curl_global_init(CURL_GLOBAL_ALL);

  /* get a curl handle */
  curl = curl_easy_init();

  if(curl) {

    struct curl_slist *headers = NULL;

    if(header != NULL)
    {

      headers = curl_slist_append(headers, header);

      curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    } 

    // curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(curl, CURLOPT_URL, api_response_data.url);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");


    /* get verbose debug output please */
    curl_easy_setopt(curl, CURLOPT_VERBOSE,   1L);

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L); 
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1L);

    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 90); // 2017/07/31 add

    /* abort if slower than 30 bytes/sec during 60 seconds */
    // curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 60L);
    // curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 60L);


    /* we want to use our own read function */
    /* now specify which file to upload */
    // curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
    // curl_easy_setopt(curl, CURLOPT_READDATA, fd_src);
    // curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)file_info.st_size);


    /* open the file */ 
    FILE *wfd = fopen(resposne_file, "wb");

    if(wfd) {
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, wfd);
    }


    curl_mime *mime;
    curl_mimepart *part;
    mime = curl_mime_init(curl);
    part = curl_mime_addpart(mime);
    curl_mime_name(part, "key");
    curl_mime_data(part, api_response_data.key, CURL_ZERO_TERMINATED);
    part = curl_mime_addpart(mime);
    curl_mime_name(part, "x-amz-algorithm");
    curl_mime_data(part, "AWS4-HMAC-SHA256", CURL_ZERO_TERMINATED);
    part = curl_mime_addpart(mime);
    curl_mime_name(part, "x-amz-credential");
    curl_mime_data(part, api_response_data.x_amz_credential, CURL_ZERO_TERMINATED);
    part = curl_mime_addpart(mime);
    curl_mime_name(part, "x-amz-date");
    curl_mime_data(part, api_response_data.x_amz_date, CURL_ZERO_TERMINATED);
    part = curl_mime_addpart(mime);
    curl_mime_name(part, "policy");
    curl_mime_data(part, api_response_data.policy, CURL_ZERO_TERMINATED);
    part = curl_mime_addpart(mime);
    curl_mime_name(part, "x-amz-signature");
    curl_mime_data(part, api_response_data.x_amz_signature , CURL_ZERO_TERMINATED);
    part = curl_mime_addpart(mime);
    curl_mime_name(part, "file");
    curl_mime_filedata(part, upload_file);

    curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);


    /* Now run off and do what you've been told! */
    res = curl_easy_perform(curl);


    if( res != CURLE_OK )
    {
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

      Cdbg(API_DBG, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

      response_code = res;

    } else {

      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    }

    /* close the header file */ 
    fclose(wfd);


    if(header)
        curl_slist_free_all(headers);

    /* always cleanup */
    curl_easy_cleanup(curl);

  }

  fclose(fd_src); /* close the local file */

  curl_global_cleanup();

  // free(url);
  return response_code;

}


int get_debug_token()
{

  char url[URLSIZE];
  char header[NORMALSIZE];
  char cookie[NORMALSIZE];
  char postdata[MAXSIZE];

  snprintf(url,URLSIZE, "%s", DEBUG_TOKEN_URL);    // DEBUG_TOKEN_URL
  snprintf(header,NORMALSIZE,"content-type: application/json");
  snprintf(cookie,NORMALSIZE,"OMNISTORE_VER=1_0; path=/;sid=%s","1004");


  char ex_router_mac[33] = {0};
  char ex_mac_md5[33] = {0};

  snprintf(ex_router_mac, sizeof(ex_router_mac), "%s+Markcool", nvram_safe_get("lan_hwaddr"));
  str2md5(ex_router_mac, ex_mac_md5);
  snprintf(ex_router_mac, sizeof(ex_router_mac), "%s", nvram_safe_get("lan_hwaddr"));


  snprintf(postdata,MAXSIZE, "{\"mac\":\"%s\", \"token\": \"%s\"}", 
      ex_router_mac, ex_mac_md5);

  Cdbg(API_DBG, "url : %s", url);
  Cdbg(API_DBG, "postdata strlen : %d, content : \n%s", strlen(postdata), postdata);

  int status = -1;
  status = http_request_process(RESPONSE_DATA_FILE, url, postdata, 
                                cookie, header, HTTP_REQUEST_POST);

  // process : http response status 
  if(status == 200) {

    if(parse_responae_json(RESPONSE_DATA_FILE, API_EX_DB_BACKUP) == FILE_PARSE_ERROR)
    {

      Cdbg(API_DBG, "%s response_json parse error : %d", API_EX_DB_BACKUP, status);

      // error : json content
      return FILE_PARSE_ERROR;

    } else {

      Cdbg(API_DBG, "%s response_json parse success", API_EX_DB_BACKUP);
      // remove(RESPONSE_DATA_FILE);

      // response_data_process(API_ROUTER_CHECK_TOKEN);

    }

  } else {

      Cdbg(API_DBG, "%s http_request_process error status : %d", API_EX_DB_BACKUP, status);

  }

  return PROCESS_SUCCESS;
}






int backup_file_to_s3(char *resposne_file, char *upload_file) {

  // get s3_ex_db_backup_cert
  int status = get_debug_token();

  if(status != 0) {
      Cdbg(API_DBG, "Post [get_debug_token] API message error, status = %d", status);
      return -1;
  } else if(api_response_data.status != 0) {
      Cdbg(API_DBG, "[get_debug_token] get data error, api_response_data.status = %d", api_response_data.status);
      return -1;
  }


  FILE * fd_src;

  fd_src = fopen(upload_file, "rb");

  if(NULL == fd_src)
  {
    Cdbg(API_DBG, "open %s file fail, backup_file_to_s3 end\n", upload_file);
    return -1;
  }


  long response_code;
  CURL *curl;
  CURLcode res;
  curl = curl_easy_init();

  if(curl) {
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(curl, CURLOPT_URL, "https://asus-router-config.s3.us-west-2.amazonaws.com");
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");

    struct curl_slist *headers = NULL;
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  

    /* get verbose debug output please */
    // curl_easy_setopt(curl, CURLOPT_VERBOSE,   1L);
    // curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L); 
    // curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1L);

    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 90);




    /* open the file */ 
    FILE *wfd = fopen(resposne_file, "wb");

    if(wfd) {
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, wfd);
    }

    curl_mime *mime;
    curl_mimepart *part;


    mime = curl_mime_init(curl);
    part = curl_mime_addpart(mime);
    curl_mime_name(part, "Content-Type");
    curl_mime_data(part, "image/", CURL_ZERO_TERMINATED);
    part = curl_mime_addpart(mime);
    curl_mime_name(part, "x-amz-meta-uuid");
    curl_mime_data(part, "14365123651274", CURL_ZERO_TERMINATED);
    part = curl_mime_addpart(mime);
    curl_mime_name(part, "key");
    curl_mime_data(part, "router/config/${filename}", CURL_ZERO_TERMINATED);
    part = curl_mime_addpart(mime);
    curl_mime_name(part, "x-amz-credential");
    curl_mime_data(part, api_response_data.x_amz_credential, CURL_ZERO_TERMINATED);
    part = curl_mime_addpart(mime);
    curl_mime_name(part, "x-amz-algorithm");
    curl_mime_data(part, "AWS4-HMAC-SHA256", CURL_ZERO_TERMINATED);
    part = curl_mime_addpart(mime);
    curl_mime_name(part, "x-amz-date");
    curl_mime_data(part, api_response_data.x_amz_date, CURL_ZERO_TERMINATED);
    part = curl_mime_addpart(mime);
    curl_mime_name(part, "X-Amz-Signature");
    curl_mime_data(part, api_response_data.x_amz_signature, CURL_ZERO_TERMINATED);
    part = curl_mime_addpart(mime);
    curl_mime_name(part, "policy");
    curl_mime_data(part, api_response_data.policy, CURL_ZERO_TERMINATED);
    part = curl_mime_addpart(mime);
    curl_mime_name(part, "x-amz-server-side-encryption");
    curl_mime_data(part, "AES256", CURL_ZERO_TERMINATED);
    part = curl_mime_addpart(mime);
    curl_mime_name(part, "file");
    curl_mime_filedata(part, upload_file);
    curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
    res = curl_easy_perform(curl);
    // curl_mime_free(mime);

    // Cdbg(API_DBG, "res = %d", res);

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    if( res != CURLE_OK )
    {
      Cdbg(API_DBG, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
      response_code = res;
    }

    // Cdbg(API_DBG, "res = [%d], response_code = [%d]", res, response_code);

    curl_mime_free(mime);

    /* close the header file */ 
    fclose(wfd);
  }

  /* always cleanup */
  curl_easy_cleanup(curl);

  fclose(fd_src); /* close the local file */


  return response_code;
}


void handle_error(int code, char *type)
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

    case S_AUTH_FAIL:

        if( strcmp(type,"gateway") == 0)
        {
            strcpy(error_message,"username is error");
        }
        else
        {
            strcpy(error_message,"auth failed ,please check username and password");
        }
        break;

    default:
        break;
    }

    Cdbg(API_DBG, "%s", error_message);

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

    // Cdbg(API_DBG, "api_name = %s, %s , key: \"%s\" at index %d, type of val: %s ", api_name, type_str, key, orig_count, json_object_get_string(val));

    if (strcmp(api_name, API_ROUTER_CHECK_TOKEN) == 0) {

      if (strcmp(key, "status") == 0) {
        api_response_data.status = atoi(json_object_get_string(val));
        continue;
      } else if (strcmp(key, "access_token") == 0) {
        snprintf(api_response_data.access_token, 384, json_object_get_string(val));
      }

    } else if (strcmp(api_name, API_ROUTER_GET_UPLOAD_FILE_URL) == 0) {

      if (strcmp(key, "status") == 0) {
        api_response_data.status = atoi(json_object_get_string(val));
        continue;
      } else if ((strcmp(key, "presigned_url") == 0) || (strcmp(key, "fields") == 0)) {
        parse_json( val, API_ROUTER_GET_UPLOAD_FILE_URL );
      } else if (strcmp(key, "url") == 0) {
        snprintf(api_response_data.url, sizeof(api_response_data.url), json_object_get_string(val));
      } else if (strcmp(key, "key") == 0) {
        snprintf(api_response_data.key, sizeof(api_response_data.key), json_object_get_string(val));
      } else if (strcmp(key, "x-amz-algorithm") == 0) {
        snprintf(api_response_data.x_amz_algorithm, sizeof(api_response_data.x_amz_algorithm), json_object_get_string(val));
      } else if (strcmp(key, "x-amz-credential") == 0) {
        snprintf(api_response_data.x_amz_credential, sizeof(api_response_data.x_amz_credential), json_object_get_string(val));
      } else if (strcmp(key, "x-amz-date") == 0) {
        snprintf(api_response_data.x_amz_date, sizeof(api_response_data.x_amz_date), json_object_get_string(val));
      } else if (strcmp(key, "policy") == 0) {
        snprintf(api_response_data.policy, sizeof(api_response_data.policy), json_object_get_string(val));
      } else if (strcmp(key, "x-amz-signature") == 0) {
        snprintf(api_response_data.x_amz_signature, sizeof(api_response_data.x_amz_signature), json_object_get_string(val));
      }

    } else if (strcmp(api_name, API_EX_DB_BACKUP) == 0) {

      if (strcmp(key, "status") == 0) {
        api_response_data.status = atoi(json_object_get_string(val));
        continue;
      } else if (strcmp(key, "credential") == 0) {
        snprintf(api_response_data.x_amz_credential, sizeof(api_response_data.x_amz_credential), json_object_get_string(val));
      } else if (strcmp(key, "date") == 0) {
        snprintf(api_response_data.x_amz_date, sizeof(api_response_data.x_amz_date), json_object_get_string(val));
      } else if (strcmp(key, "hashpolicy") == 0) {
        snprintf(api_response_data.policy, sizeof(api_response_data.policy), json_object_get_string(val));
      } else if (strcmp(key, "signature") == 0) {
        snprintf(api_response_data.x_amz_signature, sizeof(api_response_data.x_amz_signature), json_object_get_string(val));
      }

    } else if (strcmp(api_name, API_ROUTER_UPLOAD_FILE) == 0) {


      if (strcmp(key, "status") == 0) {
        api_response_data.status = atoi(json_object_get_string(val));
        continue;
      }

    } else if (strcmp(api_name, API_ROUTER_LIST_FILE) == 0) {


      if (strcmp(key, "files") == 0) {


        int i;
        struct json_object *files_array;

        // files_array = json_object_object_get(jobj, "files");

        json_object_object_get_ex(jobj, "files", &files_array);

        // files_array is an array of objects
        int arraylen = json_object_array_length(files_array);


        Cdbg(API_DBG, "Get files len =  %d ", arraylen);
        Cdbg(API_DBG, "Get files =  %s ", json_object_get_string(files_array));

        char file_list[arraylen][FILENAME_LEN];

        for (i = 0; i < arraylen; i++) {
            json_object * array_obj = json_object_array_get_idx(files_array, i);
            snprintf(file_list[i], FILENAME_LEN, "%s", json_object_get_string(array_obj));
        }

        qsort(file_list, arraylen, sizeof(file_list[0]), string_compare);


        for (i = 0; i < arraylen; i++) {
          // struct set : cloud list info
          cloud_files_init(file_list[i]);

        }

      }

    }

  }
  return 0;
}


void cloud_files_init(char *filename) {

  // Cdbg(API_DBG, "init filename = %s",  filename);

  cloud_files_list *files_list = files_list_frist;

  while(files_list != NULL){

    Cdbg(API_DBG, "check -> file_type = %s, filename = %s, mac = %s, file_len = %d", files_list->file_type, filename, mac_no_symbol, files_list->file_len);

    if ( strstr(filename,  files_list->file_type) && strstr(filename,  mac_no_symbol)) {

      files_list->bf = insert_backup_file(files_list->bf, files_list->file_len, filename);
      files_list->file_len++;
      
      Cdbg(API_DBG, "insert -> file_type = %s, filename = %s, file_len = %d", files_list->file_type, filename, files_list->file_len);


      break;
    } else {
      files_list = files_list->next; 
    }
  }
}

backup_files *insert_backup_file(backup_files *backup_file, const int file_len, const char *filename) {

  backup_files *new_file;
  new_file = backup_file;

  if(new_file != NULL)      
  {
    // Cdbg(API_DBG, "new_file > next");

    while(new_file->next != NULL)
      new_file = new_file->next;

    new_file->next = malloc(sizeof(backup_files));
    new_file = new_file->next;
    snprintf(new_file->filename, sizeof(new_file->filename), "%s", filename);
    new_file->next = NULL;


  }else{

    // Cdbg(API_DBG, "new_file > head");

    backup_file = malloc(sizeof(backup_files));
    snprintf(backup_file->filename, sizeof(backup_file->filename), "%s", filename);
    backup_file->next = NULL;
  }

  return backup_file;

}




int string_compare(const void *arg1, const void *arg2) {
    char *a = (char*)arg1;
    char *b = (char*)arg2;
    int result = strcmp(a, b);
    // int result = strcmp(b, a);
    if (result > 0) {
        return 1;
    }
    else if (result < 0) {
        return -1;
    }
    else {
        return 0;
    }
}

int upload_backup_file_to_cloud(const char *filename) {

  int status = -1;

  status = cloud_file_process(UPLOAD_FILE_TO_S3, UPLOADER_FOLDER, filename);

  if(status != 0) {
    Cdbg(API_DBG, "upload_file_process error, status = %d", status);
  }

  return status;
}



void backup_file_plus(char *type, char *filename) {

  cloud_files_list *files_list = files_list_frist;

  while(files_list != NULL){

    if( strcmp(files_list->file_type, type) == 0 ) {


      // print_backup_files_list(files_list->bf);

      files_list->bf = insert_backup_file(files_list->bf, files_list->file_len, filename);
      files_list->file_len++;

      // print_backup_files_list(files_list->bf);

      Cdbg(API_DBG, "backup file -> filename = %s, type = %s, file len = %d",  type, filename, files_list->file_len);

      break;

    } else {
      files_list = files_list->next; 
    }
  }

}


cloud_files_list *backup_file_init() {

  int i;

  cloud_files_list *first, *current, *previous;

  for(i = 0; i < TYPE_NUMBER; i++) {

    current = (cloud_files_list *) malloc(sizeof(cloud_files_list));
    current->file_len = 0;
    current->file_len_limit = backup_file_limit[i];
    current->bf = NULL;

    snprintf(current->file_type, TYPE_LEN, "%s", backup_file_type[i]);

    if(i == 0){
      first = current;
    }else{
      previous->next = current;
    }
    current->next = NULL;
    previous = current;
  }
  // return previous;
  return first;
}



void print_cloud_files_list(cloud_files_list *first)
{
  cloud_files_list *node = first; //將node指向第一個節點
  if(first == NULL){
    Cdbg(API_DBG, "print_cloud_files_list List is empty");
  } else {
    while(node != NULL){

      Cdbg(API_DBG, "node->file_len = %d", node->file_len);
      Cdbg(API_DBG, "node->file_type = %s", node->file_type);

      if(node->file_len > 0)
        print_backup_files_list(node->bf);

      node = node->next;
    }
  }
}

void print_backup_files_list(backup_files *first)
{
  backup_files *node = first;

  if(first == NULL){
    Cdbg(API_DBG, "backup_files List is empty!");
  }else{
    while(node != NULL){
      Cdbg(API_DBG, "backup_files filename = %s", node->filename);
      node = node->next;
    }
  }
}



void free_files_list(cloud_files_list *first){
    cloud_files_list *current, *tmp;
    current = first;
    while(current!=NULL){

        free_backup_files(current->bf);

        tmp = current;
        current = current->next;
        free(tmp);
    }
}

void free_backup_files(backup_files *first){
    backup_files *current, *tmp;
    current = first;
    while(current!=NULL){
        tmp = current;
        current = current->next;
        free(tmp);
    }
}

void ex_db_backup() {

  // Cdbg(API_DBG, "nvram -> s3_ex_db_backup_enable = %d, ex_db_backup_path = %s", nvram_get_int("s3_ex_db_backup_enable"), nvram_safe_get("ex_db_backup_path"));

  if(nvram_get_int("s3_ex_db_backup_enable") == 1) {

    char ex_db_backup_path[128] = {0}; 
    
    if(strcmp(nvram_safe_get("ex_db_backup_path"), "") == 0) {
      snprintf(ex_db_backup_path, sizeof(ex_db_backup_path), "%s", EX_BACKUP_DB_PATH);
    } else {
      snprintf(ex_db_backup_path, sizeof(ex_db_backup_path), "%s/.diag/", nvram_safe_get("ex_db_backup_path"));  
    }

    // Cdbg(API_DBG, "set ex_db_backup_path = [%s]", ex_db_backup_path);

    backup_db_process(ex_db_backup_path);
  }
}

void backup_db_process(char *dir) {
    

  struct dirent* ent = NULL;
  DIR *pDir;
  pDir = opendir(dir);

  if(NULL == pDir)
  {
    Cdbg(API_DBG, "open %s fail \n",dir);
    return;
  }

  int i = 0, backup_db_number = 0;

  int upload_time = get_ts_today_at_midnight();

  // Cdbg(API_DBG, "if [ (backup db timestamp) < (upload_time == %d)], run [backup db to s3 server]", upload_time);

  while (NULL != (ent=readdir(pDir)))
  {
    if(!strcmp(ent->d_name,".") || !strcmp(ent->d_name,"..") || (strcmp(get_extension(ent->d_name), "db") != 0))
        continue;

    // Cdbg(API_DBG, "find db[%d] -> [%s]", i, ent->d_name);

    char backup_db_timestamp[128] = {0};
    sscanf(ent->d_name, "%[^_]", backup_db_timestamp);
    int db_timestamp = atoi(backup_db_timestamp);

    if(db_timestamp < upload_time) {

      backup_db_number++;

      // Cdbg(API_DBG, "need upload db[%d] >> [%s] < upload_time (%d)", backup_db_number, ent->d_name, upload_time);

      // compare : Whether the backup file is uploaded
      if(compare_backup_list(dir, ent->d_name) == SUCCESS) {

        char backup_db_path[NORMALSIZE];
        char upload_backup_path[NORMALSIZE];
        char mac_name[64] = {0};
        char cmd[NORMALSIZE];

        mac_toupper(nvram_safe_get("lan_hwaddr"), mac_name);

        snprintf(upload_backup_path,NORMALSIZE,"%s%s_%s",dir, mac_name, ent->d_name);
        snprintf(backup_db_path,NORMALSIZE,"%s%s",dir,ent->d_name);

        // Cdbg(API_DBG, "backup_db_path = [%s]", backup_db_path);

        snprintf(cmd,NORMALSIZE,"cp %s %s",backup_db_path, upload_backup_path);
        system(cmd);

        if(access(upload_backup_path,F_OK) != -1) {

          Cdbg(API_DBG, "prepare upload tmp db [%s] to S3 server", upload_backup_path);

          int status = backup_file_to_s3(S3_RESPONSE_FILE, upload_backup_path);

          if(status == 204) {
            Cdbg(API_DBG, "[%s] upload success", ent->d_name);
            // write_backup_db(dir, ent->d_name);
          } else {
            Cdbg(API_DBG, "upload_file_to_s3 error, status = %d", status);
          }
        
          // deleted uploaded backup_db
          if(!remove(upload_backup_path)) {
              Cdbg(API_DBG, "del sucess >> tmp db filename > %s", upload_backup_path);
          }

        }
      }
    }

  }

  reconstruct_ex_backup_db_files(dir, backup_db_number, upload_time);  

  closedir(pDir);

}


int get_ts_today_at_midnight(void)
{
  time_t now = time(NULL);
  struct tm utctm;
  utctm = *gmtime(&now);
  utctm.tm_isdst = -1;

  time_t utctt = mktime(&utctm);
  // diff is the offset in seconds
  long diff = now - utctt;

  utctm.tm_hour = 0;
  utctm.tm_min = 0;
  utctm.tm_sec = 0;

  time_t night_ts = mktime(&utctm);

  int midnight = (int) (night_ts + diff);

  Cdbg(API_DBG, "get now time = %ld, utc_time = %ld, time diff = %ld, gmt midnight = %d", now, utctt, diff, midnight);

  return midnight;
}

int compare_backup_list(char * backup_db_path, char * backup_db_name) {


  char backup_list_path[NORMALSIZE];
  snprintf(backup_list_path,NORMALSIZE, "%s%s", backup_db_path, EX_BACKUP_LIST);


  // create backup_list_path new file
  if(access(backup_list_path, 0) == -1) {
    eval("touch", backup_list_path);
  } 

  FILE *fp;

  char file_info[1023];

  if ((fp = fopen(backup_list_path, "r")) == NULL) {
    Cdbg(API_DBG, "%s open_file_error", backup_list_path);
    return -1;
  }

  // int i = 0;

  while(fgets(file_info, 1023, fp) != NULL)
  {
    // i++;
    // Cdbg(API_DBG, "i = %d, %s", i, file_info);

    if(strstr(file_info, backup_db_name) != NULL) {
      // Cdbg(API_DBG, "backup_db uploaded >> [%s]", backup_db_name);
      fclose(fp);
      return -1;
    }
  }

  fclose(fp);

  return SUCCESS;
}




int write_backup_db(char * backup_db_path, char * backup_db_name) {

  char write_db_path[NORMALSIZE];
  snprintf(write_db_path,NORMALSIZE,"%s%s", backup_db_path, EX_BACKUP_LIST);
  

  FILE *fp = fopen(write_db_path, "a+");

  if( NULL == fp ){
    Cdbg(API_DBG, "write [%s] file error -> open failure", write_db_path);
    return -1;
  } else {
    fprintf(fp, "%s\n", backup_db_name);    
    fclose(fp);
    Cdbg(API_DBG, "write [%s] to [%s] success", backup_db_name, write_db_path);
  }

  return 0;
}

int reconstruct_ex_backup_db_files(const char * backup_db_path, const int backup_db_number, const int upload_time) {

  // The number of files has not changed
  if( backup_db_number_tmp == backup_db_number )  {
    return 0;
  }

  Cdbg(API_DBG, "backup_db_number_tmp[%d] != backup_db_number[%d], backup_db_path = %s, upload_time = %d, prepare write new ex backup db file", backup_db_number_tmp, backup_db_number, backup_db_path, upload_time);


  struct dirent* ent = NULL;
  DIR *pDir;
  pDir = opendir(backup_db_path);

  if(NULL == pDir)
  {
    Cdbg(API_DBG, "open %s fail \n",backup_db_path);
    return -1;
  }


  // delete old ex backup db file
  char write_db_path[NORMALSIZE];
  snprintf(write_db_path,NORMALSIZE,"%s%s", backup_db_path, EX_BACKUP_LIST);
  unlink(write_db_path);


  // write new ex backup db files
  while (NULL != (ent=readdir(pDir)))
  {
    if(!strcmp(ent->d_name,".") || !strcmp(ent->d_name,"..") || (strcmp(get_extension(ent->d_name), "db") != 0))
        continue;

    char backup_db_timestamp[128] = {0};
    sscanf(ent->d_name, "%[^_]", backup_db_timestamp);

    int db_timestamp = atoi(backup_db_timestamp);

    if(db_timestamp < upload_time) {
      Cdbg(API_DBG, "wirte db name = %s, db_timestamp[%d] <= upload_time[%d]", ent->d_name, db_timestamp, upload_time);
      write_backup_db(backup_db_path, ent->d_name);
    }
  }

  // save backup_db_number
  backup_db_number_tmp = backup_db_number;


  return 0;
}

char *get_extension(char *fileName) {  
  int len = strlen(fileName);  
  int i = len;  
  while( fileName[i] != '.' && i > 0 ){ i--; }  

  if(fileName[i] == '.') {  
    return &fileName[i+1];  
  }else{  
    return &fileName[len];  
  }  
}  

int update_ui_support_db(char *fileName) {


  char ui_support_path[128] = {0};
  char timestamp_str[16] = {0};

  getTimeInMillis(timestamp_str);
  snprintf(ui_support_path, sizeof(ui_support_path), "%s%s", UPLOADER_FOLDER, fileName);

  struct json_object *ui_support_obj = json_object_new_object();

  get_ui_support_info(ui_support_obj);

  json_object_to_file(ui_support_path, ui_support_obj);

  if(ui_support_obj)
      json_object_put(ui_support_obj);


  return 0;
}