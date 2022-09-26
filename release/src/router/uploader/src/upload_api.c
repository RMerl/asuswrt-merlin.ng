
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


#define API_DBG 1


struct api_response_data api_response_data;

extern char aae_webstorageinfo[128];



int upload_file(const char * cusid, const char * user_ticket, const char * devicehashmac, 
                const char *file_path, const char *file_name) 
{


    int status = get_aicamcloud_api_info(API_ROUTER_CHECK_TOKEN , cusid, 
                                     user_ticket, devicehashmac, NULL, NULL);

    if(status != 0) {
        Cdbg(API_DBG, "Post [router_check_token] API message error, status = %d", status);
        return status;
    } else if(api_response_data.status != 0) {
        Cdbg(API_DBG, "[check_token] get data error, status = %d", api_response_data.status);
        return api_response_data.status;
    }


    status = get_aicamcloud_api_info(API_ROUTER_GET_UPLOAD_FILE_URL , NULL, NULL, 
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


    char upload_file[256];
    snprintf(upload_file, sizeof(upload_file) ,"%s%s", file_path, file_name);


    status = upload_file_to_s3(S3_RESPONSE_FILE, upload_file, NULL);

    Cdbg(API_DBG, "S3_RESPONSE_FILE = %s, status = %d", S3_RESPONSE_FILE, status);

    if(status != 204) {
        Cdbg(API_DBG, "Post [upload_file_to_s3] API message error, status = %d", status);
        return status;
    } 


    status = get_aicamcloud_api_info(API_ROUTER_UPLOAD_FILE , NULL, NULL, 
                                 devicehashmac, file_path, file_name);

    if(status != 0) {
        Cdbg(API_DBG, "Post [router_upload_file] API message error, status = %d", status);
        return status;
    } else if(api_response_data.status != 0) {
        Cdbg(API_DBG, "[router_upload_file] get data error, status = %d", api_response_data.status);
        return api_response_data.status;
    }

}

int check_file_list(const char * cusid, const char * user_ticket, const char * devicehashmac, 
                    const char *file_path, const char *file_name)

{
  int status;


  status = get_aicamcloud_api_info(API_ROUTER_LIST_FILE , NULL, NULL, 
                                   devicehashmac, file_path, file_name);

  if(status != 0) {
      Cdbg(API_DBG, "Post [router_get_upload_file_url] API message error, status = %d", status);
      return status;
  } else if(api_response_data.status != 0) {
      Cdbg(API_DBG, "[router_get_upload_file_url] get data error, status = %d", api_response_data.status);
      return api_response_data.status;
  }

  while(api_response_data.files_len > CLOUD_FILE_NUMBER) {

      char buffer[2048];
      snprintf(buffer, sizeof(buffer), "%s", api_response_data.files_content);

      json_object * jobj = json_tokener_parse(buffer);
      int arraylen = json_object_array_length(jobj);
      int i;

      for (i = 0; i < arraylen; i++) {
        // get the i-th object in medi_array
        json_object * array_obj = json_object_array_get_idx(jobj, i);

        // json_object * tmp_obj = json_object_object_get(jobj, "");

        char del_file[64];
        snprintf(del_file, sizeof(del_file), "%s", json_object_get_string(array_obj));


        if( (arraylen - i) > CLOUD_FILE_NUMBER ) {


          status = get_aicamcloud_api_info(API_ROUTER_DEL_FILE , NULL, NULL, 
                                           devicehashmac, file_path, del_file);

          if(status == 0) {
              Cdbg(API_DBG, "%s -> del_file SUCCESS ",  del_file);
              api_response_data.files_len--;
              Cdbg(API_DBG, "api_response_data.files_len = %d", api_response_data.files_len);
          }
          
        }  else {
          break;
        }
          
      }

  }

  return PROCESS_SUCCESS;
}



int get_aicamcloud_api_info(const char * api_name , const char * cusid, 
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
  Cdbg(API_DBG, "11postdata strlen : %d, content : \n%s", strlen(postdata), postdata);

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

    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
    // curl_easy_setopt(curl,CURLOPT_ERRORBUFFER,err_message);
    // curl_easy_setopt(curl,CURLOPT_FAILONERROR,1);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1);
    //curl_easy_setopt(curl,CURLOPT_SSL_VERIFYHOST,0);
    //curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);

    //curl_easy_setopt(curl,CURLOPT_TIMEOUT,30); // 90 -> 60
    // curl_easy_setopt(curl,CURLOPT_CONNECTTIMEOUT, 60); // 2017/07/31 add

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

    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 60); // 2017/07/31 add

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

    } else if (strcmp(api_name, API_ROUTER_UPLOAD_FILE) == 0) {


      if (strcmp(key, "status") == 0) {
        api_response_data.status = atoi(json_object_get_string(val));
        continue;
      }

    } else if (strcmp(api_name, API_ROUTER_LIST_FILE) == 0) {


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



// int main( int argc, char ** argv )
// {
//   return 0;
// }