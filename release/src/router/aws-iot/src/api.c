
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

// #include <shared.h>

#include <sys/time.h>

#include "awsiot_config.h"

#include "api.h"
#include "awsiot.h"
#include "log.h"
#include "json.h"



#define API_DBG 1

extern char receivedSessionData[AWS_IOT_SESSION_BUF_LEN];

// func from libshared.so
extern int file_lock(const char *tag);
extern void file_unlock(int lockfd);
extern int check_if_file_exist(const char *file);


int get_session_data(const char* json_data, char * out_data) {

  struct json_object *received = NULL, *session = NULL;

  received = json_tokener_parse(json_data);
  
  if(is_error(received)) {
    LogInfo( ( "json_tokener_parse failure" ) );
    return -1;
  }

  json_object_object_get_ex(received, "session", &session);
  snprintf(out_data, NETWORK_BUFFER_SIZE, "%s", json_object_get_string(session));

  json_object_put(received);

  return 0;
}


int parse_json_format(const char* json_data) {

  struct json_object *received = NULL;

  received = json_tokener_parse(json_data);
  
  if(is_error(received)) {
    LogInfo( ( "json_tokener_parse failure" ) );
    return -1;
  }

  json_object_put(received);

  return 0;
}



int write_file(char *filename, char* json_data) {

    FILE *pFile;

    pFile = fopen( filename, "w" );

    if( NULL == pFile ){

        Cdbg(API_DBG, "%s -> open failure", filename);

        return -1;

    }else{
        fprintf(pFile, "%s", json_data);
    }

    fclose(pFile);

    // Cdbg(API_DBG, "%s -> write success", filename);

    return 0;
}




int parse_download_update(char* publish_data, char* awsiot_clientid, char *sn_hash) {


  int i, lock;
  int session_array_len = 0;

  struct json_object *create_root = NULL;

  struct json_object *session_array = NULL;
  struct json_object *session_array_obj;
  struct json_object *session_array_topic, *session_array_id, *session_array_state, *session_array_msg;



  json_object *create_session_array = NULL;
  json_object *create_session_array_obj = NULL;

  create_root = json_object_new_object();
  create_session_array = json_object_new_array();


  if(check_if_file_exist(TENCENT_SESSION_FILE)){

    lock = file_lock(TENCENT_SESSION_FILE);
    session_array = json_object_from_file(TENCENT_SESSION_FILE);
    file_unlock(lock);

    if(session_array) {
      session_array_len = json_object_array_length(session_array);
    } else {
      LogInfo( ( "parse_download_update, session_array read error" ) );
      // Cdbg(API_DBG, "json session_array read error");
      return -1;
    }

  } else {
    LogInfo( ( "parse_download_update, file not exist -> TENCENT_SESSION_FILE" ) );
    // Cdbg(API_DBG, "file not exist -> TENCENT_SESSION_FILE");
    return -1;
  }

  // session_array = json_object_from_file(TENCENT_SESSION_FILE);

  // int session_array_len = 0;

  // if(session_array) {
  //   session_array_len = json_object_array_length(session_array);
  // } else {
  //   IOT_INFO("parse_download_update json_tokener_parse failure, filename = %s", TENCENT_SESSION_FILE);
  //   return -1;
  // }


  for (i = 0; i < session_array_len; i++) {

    // get the i-th object in session_array
    session_array_obj = json_object_array_get_idx(session_array, i);


    json_object_object_get_ex(session_array_obj, "id", &session_array_id);
    json_object_object_get_ex(session_array_obj, "state", &session_array_state);



    // int session_error_code = 0;

    char session_state[32];
    char session_msg[128];
    char session_id[64];

    snprintf(session_id, 64, "%s", json_object_get_string(session_array_id));
    snprintf(session_msg, 128, "start");


    LogInfo( ( "parse_download_update, before , session_id -> %s, state -> %s", session_id, json_object_get_string(session_array_state) ) );

    int status = tencentgame_session_id_cmp(session_id, NULL, session_msg);

    if(status != 0) {
      // json_object_object_get_ex(session_array_obj, "state", &session_array_state);
      // snprintf(session_state, 64, "%s", json_object_get_string(session_array_state));
    }


    LogInfo( ( "parse_download_update, after , session_id -> %s, status = %d, session_msg -> %s\n", session_id, status, session_msg ) );


    if((strcmp(session_msg, "start")  == 0) || (strcmp(session_msg, "(null)") == 0)) {
      snprintf(session_msg, 128, "");
    }


    // json_object_array_put_idx(session_array_obj, "client_id", json_object_new_string(awsiot_clientid));
    // json_object_array_put_idx(session_array_obj, "sn", json_object_new_string(sn_hash));
    // json_object_array_put_idx(session_array_obj, "msg", json_object_new_string(session_msg));

    // IOT_INFO("\n\n\nsession_array_objsession_array_objsession_array_objsession_array_obj     -> %s", json_object_get_string(session_array_obj));

    create_session_array_obj = json_object_new_object();

    json_object_object_add(create_session_array_obj, "id", session_array_id);
    json_object_object_add(create_session_array_obj, "client_id", json_object_new_string(awsiot_clientid));
    json_object_object_add(create_session_array_obj, "state", session_array_state);
    json_object_object_add(create_session_array_obj, "sn", json_object_new_string(sn_hash));
    json_object_object_add(create_session_array_obj, "msg", json_object_new_string(session_msg));
    // json_object_object_add(create_session_array_obj, "error_code", json_object_new_int(session_error_code));



    json_object_array_add(create_session_array, create_session_array_obj);
  }

  //add array to the base object
  json_object_object_add(create_root, "session", create_session_array);


  snprintf(publish_data, JSON_CONTENT_SIZE, "%s", json_object_get_string(create_root));


  json_object_to_file(TENCENT_SESSION_PUBLISH_FILE, create_root);

  LogInfo( ( "parse_download_update, json_object_put(create_root)" ) );

  json_object_put(create_root);

  // IOT_INFO("parse_download_update, json_object_put(tencent_report_compare)");
  // json_object_put(tencent_report_compare);

  LogInfo( ( "parse_download_update, json_object_put(session_array)" ) );
  json_object_put(session_array);

  // IOT_INFO("remove(TENCENT_SESSION_JSON_TMP)");
  // remove(TENCENT_SESSION_JSON_TMP);

  LogInfo( ( "parse_download_update, end" ) );


  return 0;
}


int split_received_tencent_session(const char *topic_url) {

  int i;
  int ret = 0;


  struct json_object *session_array = NULL;

  struct json_object *session_array_obj;
  struct json_object *session_array_topic;


  session_array = json_object_from_file(TENCENT_SESSION_FILE);

  int session_array_len = 0;

  if(session_array) {
    session_array_len = json_object_array_length(session_array);
  } else {
    LogInfo( ( "parse_tencent_session_file -> json_tokener_parse failure" ) );
    Cdbg(API_DBG, "json_tokener_parse failure");
    return -1;
  }



  json_object *topic_session_array = NULL;
  json_object *other_session_array = NULL;

  //new a array
  // topic_session_array = json_object_new_array();
  other_session_array = json_object_new_array();


  for (i = 0; i < session_array_len; i++) {

    // get the i-th object in session_array
    session_array_obj = json_object_array_get_idx(session_array, i);
    json_object_object_get_ex(session_array_obj, "topic", &session_array_topic);


    char session_topic[MAX_TOPIC_LENGTH];
    snprintf(session_topic, MAX_TOPIC_LENGTH, "%s", json_object_get_string(session_array_topic));

    LogInfo( ( "session_topic=%s", session_topic) );

    if( strcmp(session_topic, "(null)") == 0 ) {
      LogInfo( ( "session_topic (null) " ) );
      continue;
    }




    if( strcmp(topic_url, session_topic) == 0 ) {

      // IOT_INFO("topic_url  data\n");
      // json_object_array_add(topic_session_array, session_array_obj);

    } else {

      LogInfo( ( "other_url  data\n" ) );
      json_object_array_add(other_session_array, session_array_obj);
    }
  }

  // json_object_to_file(TENCENT_SESSION_TOPIC, topic_session_array);
  json_object_to_file(TENCENT_SESSION_OTHER, other_session_array);

  // IOT_INFO("split_received_tencent_session release space : topic_session_array");
  // json_object_put(topic_session_array);

  LogInfo( ( "split_received_tencent_session release space : other_session_array" ) );
  json_object_put(other_session_array);
  LogInfo( ( "split_received_tencent_session release space : session_array" ) );
  json_object_put(session_array);
  LogInfo( ( "split_received_tencent_session parse_tencent_session_file end\n" ) );

  return 0;
}


int parse_receive_remote_connection(const char* json_data, const char* subscribe_topic) {

  struct json_object *received = NULL, *enable = NULL;

  received = json_tokener_parse(json_data);
  
  if(is_error(received)) {
    LogInfo( ( "json_tokener_parse failure" ) );
    return -1;
  }

  json_object_object_get_ex(received, "tunnel_enable", &enable);

  int enable_status = json_object_get_int(enable);

  // IOT_INFO("enable=%s\n", json_object_get_int(enable));

  json_object_put(received);

  return enable_status;
}


int parse_receive_httpd(const char* json_data, char *request_data, int request_len, 
      char *api_name, int api_name_len, char *session_id, int session_id_len) {


  // char api_name[128];
  // char session_id[128];
  char epid[128];
  char epusr[128];
  char epts[128];

  struct json_object *received = NULL, *api_obj = NULL, *session_obj = NULL;
  struct json_object *param_obj = NULL, *epid_obj = NULL, *epusr_obj = NULL;
  struct json_object *epts_obj = NULL;

  received = json_tokener_parse(json_data);
  
  if(is_error(received)) {
    LogInfo( ( "json_tokener_parse failure" ) );
    return -1;
  }

  json_object_object_get_ex(received, "api_name", &api_obj);
  json_object_object_get_ex(received, "session_id", &session_obj);
  json_object_object_get_ex(received, "param", &param_obj);


  snprintf(api_name, api_name_len, "%s", json_object_get_string(api_obj));
  snprintf(session_id, session_id_len, "%s", json_object_get_string(session_obj));


  LogInfo( ( "api_name = %s", api_name) );
  LogInfo( ( "session_id = %s", session_id) );

  if(param_obj) {

    json_object_object_get_ex(param_obj, "epid", &epid_obj);
    json_object_object_get_ex(param_obj, "epusr", &epusr_obj);
    json_object_object_get_ex(param_obj, "epts", &epts_obj);


    snprintf(epid, sizeof(epid), "%s", json_object_get_string(epid_obj));
    snprintf(epusr, sizeof(epusr), "%s", json_object_get_string(epusr_obj));
    snprintf(epts, sizeof(epts), "%s", json_object_get_string(epts_obj));


    LogInfo( ( "epid_obj = %s", json_object_get_string(epid_obj)) );
    LogInfo( ( "epusr_obj = %s", json_object_get_string(epusr_obj)) );
    LogInfo( ( "epts_obj = %s", json_object_get_string(epts_obj)) );


    snprintf(request_data, request_len, "{\"epid\":\"%s\",\"epusr\":\"%s\", \"epts\":\"%s\"}", 
                                      epid, epusr, epts);

    LogInfo( ( "request_data 0 = %s", request_data ) );

  } else {

    LogInfo( ( "json param failure" ) );
    json_object_put(received);
    return -1;
  }


  json_object_put(received);

  LogInfo( ( "request_data 1 = %s", request_data ) );

  return 0;
}



int copy_tencent_update_tmp() {

  char cp_command[128] = {0};
  snprintf(cp_command, 128, "cp %s %s ", TENCENT_SESSION_REPORT_FILE, TENCENT_SESSION_JSON_TMP);

  int cp_count = 0;
  int system_code;
  do {

    cp_count++;
    system_code = system(cp_command);

    LogInfo( ( "system_code = %d, cp_command = %s", system_code, cp_command) );
    if((system_code == 0) || (cp_count >= 2)) 
      break;

  } while(1);
}


int compare_tencent_session() {

  int i;
  int ret = 0;

  struct json_object *session_array = NULL;

  struct json_object *session_array_obj;
  struct json_object *session_array_topic, *session_array_id, *session_array_gameid;
  struct json_object *session_array_ver, *session_array_expire, *session_array_state;



  struct json_object *filelist_obj, *filelist_array, *filelist_array_src;
  struct json_object *filelist_array_md5, *filelist_array_size, *filelist_array_dst;
  struct json_object *filelist_array_platform, *filelist_array_meta;


  session_array = json_object_from_file(TENCENT_SESSION_FILE);

  int session_array_len = 0;

  if(session_array) {
    session_array_len = json_object_array_length(session_array);
  } else {
    LogInfo( ( "json_tokener_parse failure, filename = %s", TENCENT_SESSION_FILE) );
    Cdbg(API_DBG, "json_tokener_parse failure , filename = %s", TENCENT_SESSION_FILE);
    return -1;
  }


  json_object *create_filelist_obj = NULL;
  json_object *create_session_array = NULL;

  //new a array
  create_session_array = json_object_new_array();
  if (!create_session_array)
  {
    printf("Cannot create array object\n");
    ret = -1;
  }

  json_object *create_session_array_obj = NULL;
  json_object *create_filelist_array_obj = NULL;

  for (i = 0; i < session_array_len; i++) {

    // get the i-th object in session_array
    session_array_obj = json_object_array_get_idx(session_array, i);
    json_object_object_get_ex(session_array_obj, "topic", &session_array_topic);
    json_object_object_get_ex(session_array_obj, "id", &session_array_id);
    json_object_object_get_ex(session_array_obj, "gameid", &session_array_gameid);
    json_object_object_get_ex(session_array_obj, "ver", &session_array_ver);
    json_object_object_get_ex(session_array_obj, "expire", &session_array_expire);
    json_object_object_get_ex(session_array_obj, "state", &session_array_state);


    create_session_array_obj = json_object_new_object();
    json_object_object_add(create_session_array_obj, "topic", session_array_topic);
    json_object_object_add(create_session_array_obj, "id", session_array_id);
    json_object_object_add(create_session_array_obj, "gameid", session_array_gameid);
    json_object_object_add(create_session_array_obj, "ver", session_array_ver);
    json_object_object_add(create_session_array_obj, "expire", session_array_expire);


    char session_state[32];
    char session_id[64];

    snprintf(session_id, 64, "%s", json_object_get_string(session_array_id));
    snprintf(session_state, 64, "%s", json_object_get_string(session_array_state));

    LogInfo( ( "compare_tencent_session, cmp before, session_id -> %s, session_state -> %s", session_id, session_state) );

    int status = tencentgame_session_id_cmp(session_id, session_state, NULL);

    if(status != 0) {
      // snprintf(session_state, 64, "%s", json_object_get_string(session_array_state));
    }

    LogInfo( ( "compare_tencent_session, cmp after,  session_id -> %s, state -> %s\n", session_id, session_state) );
    Cdbg(API_DBG, "session_id -> %s, state -> %s", session_id, session_state);

    json_object_object_add(create_session_array_obj, "state", json_object_new_string(session_state));


    if( json_object_object_get_ex(session_array_obj, "filelist", &filelist_array) ) {

      create_filelist_array_obj = json_object_new_array();

      int filelist_lie, j; 
      // session_array is an array of objects
      filelist_lie = json_object_array_length(filelist_array);

      // printf("filelist_lie=%d\n", filelist_lie);

      for (j = 0; j < filelist_lie; j++) {

        // get the i-th object in session_array
        filelist_obj = json_object_array_get_idx(filelist_array, j);

        json_object_object_get_ex(filelist_obj, "src", &filelist_array_src);
        json_object_object_get_ex(filelist_obj, "md5", &filelist_array_md5);
        json_object_object_get_ex(filelist_obj, "size", &filelist_array_size);
        json_object_object_get_ex(filelist_obj, "dst", &filelist_array_dst);
        json_object_object_get_ex(filelist_obj, "platform", &filelist_array_platform);
        json_object_object_get_ex(filelist_obj, "meta", &filelist_array_meta);

        // printf("src=%s\n", json_object_get_string(filelist_array_src));
        // printf("md5=%s\n", json_object_get_string(filelist_array_md5));
        // printf("size=%s\n", json_object_get_string(filelist_array_size));
        // printf("dst=%s\n", json_object_get_string(filelist_array_dst));


        create_filelist_obj = json_object_new_object();
        // //new a null
        json_object_object_add(create_filelist_obj, "src", filelist_array_src);
        json_object_object_add(create_filelist_obj, "md5", filelist_array_md5);
        json_object_object_add(create_filelist_obj, "size", filelist_array_size);
        json_object_object_add(create_filelist_obj, "dst", filelist_array_dst);
        json_object_object_add(create_filelist_obj, "platform", filelist_array_platform);
        json_object_object_add(create_filelist_obj, "meta", filelist_array_meta);

        json_object_array_add(create_filelist_array_obj, create_filelist_obj);

        // json_object_array_add(create_filelist_array_obj, filelist_obj);

      }


      json_object_object_add(create_session_array_obj, "filelist", create_filelist_array_obj);

      json_object_array_add(create_session_array, create_session_array_obj);

    }
      
  }

  
  // char create_data[JSON_CONTENT_SIZE];
  // snprintf(create_data, JSON_CONTENT_SIZE, "%s", json_object_get_string(create_session_array));
  // write_file(TENCENT_SESSION_FILE, create_data);

  json_object_to_file(TENCENT_SESSION_FILE, create_session_array);

  // json_object_put(create_filelist_array_obj);
  // json_object_put(create_session_array_obj);
  // json_object_put(create_filelist_obj);

  LogInfo( ( "compare_tencent_session json_object_put(create_session_array)" ) );

  json_object_put(create_session_array);
  LogInfo( ( "compare_tencent_session json_object_put(session_array)" ) );
  json_object_put(session_array);

  // IOT_INFO("json_object_put(tencent_report_compare)");
  // json_object_put(tencent_report_compare);

  // IOT_INFO("compare_tencent_session remove(TENCENT_SESSION_JSON_TMP)");

  // remove(TENCENT_SESSION_JSON_TMP);

  LogInfo( ( "compare_tencent_session end" ) );

  // IOT_INFO("create_data = %s\n", create_data);
  // exit(0);

  return 0;
}



int tencentgame_session_id_cmp(const char* cmp_session_id, char* get_session_state, char *get_session_msg) {

  int i, lock;
  int session_array_len = 0;

  struct json_object *session_array;
  struct json_object *session_array_obj;
  struct json_object *session_array_id, *session_array_state, *session_array_msg;

  if(check_if_file_exist(TENCENT_SESSION_JSON_TMP)){
    lock = file_lock(TENCENT_SESSION_JSON_TMP);
    session_array = json_object_from_file(TENCENT_SESSION_JSON_TMP);
    file_unlock(lock);

    if(session_array) {

      int json_len = strlen(json_object_get_string(session_array));
      if(json_len > 2) {
        session_array_len = json_object_array_length(session_array);
      } else {
        session_array_len = 0;
      }
      
    } else {
      LogInfo( ( "tencentgame_session_id_cmp, session_array read error" ) );
      // Cdbg(API_DBG, "json session_array read error");
      return -1;
    }

  } else {
    LogInfo( ( "tencentgame_session_id_cmp, file not exist -> TENCENT_SESSION_JSON_TMP" ) );
    // Cdbg(API_DBG, "file not exist -> TENCENT_SESSION_JSON_TMP");
    return -1;
  }

  LogInfo( ( "tencentgame_session_id_cmp, session_array_len -> %d", session_array_len) );


  for (i = 0; i < session_array_len; i++) {

    // get the i-th object in session_array
    session_array_obj = json_object_array_get_idx(session_array, i);

    json_object_object_get_ex(session_array_obj, "id", &session_array_id);


    char session_id[64];
    snprintf(session_id, 64, "%s", json_object_get_string(session_array_id));

    // const char *session_id = json_object_get_string(session_array_id);

    LogInfo( ( "tencentgame_session_id_cmp, session_id -> %s, cmp_session_id -> %s, state -> [%s] , msg -> [%s] ", cmp_session_id, session_id, get_session_state, get_session_msg) );

    if(strcmp(session_id, cmp_session_id) == 0) {
      
      if(get_session_state != NULL) {
        json_object_object_get_ex(session_array_obj, "state", &session_array_state);
        snprintf(get_session_state, 32, "%s", json_object_get_string(session_array_state));
        LogInfo( ( "tencentgame_session_id_cmp, get [%s], session_state -> %s ", TENCENT_SESSION_REPORT_FILE, get_session_state) );
      }

      if(get_session_msg != NULL) {
        json_object_object_get_ex(session_array_obj, "msg", &session_array_msg);
        if(session_array_msg) {
          snprintf(get_session_msg, 128, "%s", json_object_get_string(session_array_msg));
          LogInfo( ( "tencentgame_session_id_cmp, get [%s], get_session_msg -> %s ", TENCENT_SESSION_REPORT_FILE, get_session_msg) );
        } else {
          snprintf(get_session_msg, 128, "");
        }

      }

      json_object_put(session_array);

      return 0;
    }

  }

  json_object_put(session_array);

  return -1;
}




int compare_received_session_id(const char* cmp_session_id, char* session_state, char* session_update_state, int *session_error_code, const char* session_update_json) {


  int i, lock;
  int session_array_len = 0;


  struct json_object *session_array;
  struct json_object *session_array_obj;
  struct json_object *session_array_id, *session_array_state;
  struct json_object *session_array_error_code;


  session_array = json_tokener_parse(session_update_json);
  
  if(is_error(session_array)) {
    LogInfo( ( "compare_received_session_id, json_tokener_parse failure" ) );
    Cdbg(API_DBG, "compare_received_session_id, json_tokener_parse failure");
    return -1;
  }

  int session_update_len = strlen(session_update_json);

  if(session_update_len < 2) {
    session_array_len = 0;
  } else {
    session_array_len = json_object_array_length(session_array);
  }


  for (i = 0; i < session_array_len; i++) {

    // get the i-th object in session_array
    session_array_obj = json_object_array_get_idx(session_array, i);

    json_object_object_get_ex(session_array_obj, "id", &session_array_id);


    char session_id[64];
    snprintf(session_id, 64, "%s", json_object_get_string(session_array_id));

    // char *session_id = json_object_get_string(session_array_id);

    LogInfo( ( "compare received session id,  session_id -> %s, cmp_session_id -> %s ", session_id, cmp_session_id) );

    if(strcmp(session_id, cmp_session_id) == 0) {
      
      json_object_object_get_ex(session_array_obj, "state", &session_array_state);
      json_object_object_get_ex(session_array_obj, "error_code", &session_array_error_code);

      if(session_array_error_code) {
        *session_error_code = json_object_get_int(session_array_error_code);
      } else {
        *session_error_code = -1;
      }

      // snprintf(session_error_code, 32, "%d", json_object_get_string(session_array_error_code));


      if(session_array_state) {

        snprintf(session_update_state, 32, "%s", json_object_get_string(session_array_state));

        LogInfo( ( "compare_received_session_id, session_state -> %s, session_update_state -> %s ", session_state, session_update_state) );

        if( (strcmp(session_state, "expired") == 0) && 
            (strcmp(session_update_state, "going") == 0) ) {

          struct json_object *filelist_array;

          if( json_object_object_get_ex(session_array_obj, "filelist", &filelist_array) ) {

             json_object_to_file(TENCENT_SESSION_CHANGE_FILELIST, filelist_array);

          }
        }
      }

      json_object_put(session_array);
      return 0;
    }
      
  }

  json_object_put(session_array);
  return -1;
}




int compare_received_session(const int subscribe_topic_number, int b_print) {

  int session_len = JSON_CONTENT_SIZE*subscribe_topic_number;
  char session_update_json[session_len];


  read_file_data(TENCENT_SESSION_REPORT_FILE, session_update_json, session_len); 


  int i, lock;
  int session_array_len = 0;

  struct json_object *session_array = NULL;
  struct json_object *session_array_obj = NULL;
  struct json_object *session_array_id = NULL;
  struct json_object *session_array_state = NULL;
  struct json_object *filelist_array = NULL;


  struct json_object *session_array_topic, *session_array_gameid;
  struct json_object *session_array_ver, *session_array_expire;


  struct json_object *filelist_obj, *filelist_array_src;
  struct json_object *filelist_array_md5, *filelist_array_size, *filelist_array_dst;
  struct json_object *filelist_array_platform, *filelist_array_meta;

  struct json_object *create_session_array = NULL;
  struct json_object *create_filelist_obj = NULL;
  struct json_object *create_session_array_obj = NULL;
  struct json_object *create_filelist_array_obj = NULL;


  if(check_if_file_exist(TENCENT_SESSION_FILE)){
    lock = file_lock(TENCENT_SESSION_FILE);
    session_array = json_object_from_file(TENCENT_SESSION_FILE);
    file_unlock(lock);

    if(session_array) {
      int json_len = strlen(json_object_get_string(session_array));
      if(json_len > 2) {
        session_array_len = json_object_array_length(session_array);
      } else {
        session_array_len = 0;
      }
    } else {
      LogInfo( ( "compare_received_session, session_array read error" ) );
      Cdbg(API_DBG, "json session_array read error");
      return -1;
    }

  } else {
    LogInfo( ( "compare_received_session, file not exist -> TENCENT_SESSION_FILE" ) );
    // Cdbg(API_DBG, "file not exist -> TENCENT_SESSION_JSON_TMP");
    return -1;
  }

  create_session_array = json_object_new_array();

  for (i = 0; i < session_array_len; i++) {

    // get the i-th object in session_array
    session_array_obj = json_object_array_get_idx(session_array, i);

    json_object_object_get_ex(session_array_obj, "topic", &session_array_topic);
    json_object_object_get_ex(session_array_obj, "id", &session_array_id);
    json_object_object_get_ex(session_array_obj, "gameid", &session_array_gameid);
    json_object_object_get_ex(session_array_obj, "ver", &session_array_ver);
    json_object_object_get_ex(session_array_obj, "expire", &session_array_expire);
    json_object_object_get_ex(session_array_obj, "state", &session_array_state);


    int change_filelist_status = -1;

    char session_state[32] = {0};
    char session_update_state[32] = {0};
    int session_error_code = -1;

    snprintf(session_state, 32, "%s", json_object_get_string(session_array_state));


    char session_id[64];
    snprintf(session_id, 64, "%s", json_object_get_string(session_array_id));
    // char *session_id = json_object_get_string(session_array_id);

    if(b_print == 1) {
      LogInfo( ( "compare_received_session, receive, cmp before, session_id -> %s, session_state -> %s, session_error_code = %d", session_id, session_state, session_error_code) );
      Cdbg(API_DBG, "receive, cmp before, session_id -> %s, session_state -> %s, session_error_code = %d", session_id, session_state, session_error_code);      
    }



    int status = compare_received_session_id(session_id, session_state, session_update_state, &session_error_code, session_update_json);

    if( (strcmp(session_state, "pushed") == 0) || 
        (strcmp(session_state, "going") == 0)  || 
        (strcmp(session_state, "received") == 0)  || 
        (strcmp(session_state, "completed") == 0)  ) {
 
      // id -> exist
      if(status == 0) {
        if(strcmp(session_update_state, "completed") == 0) {
          snprintf(session_state, 32, "%s", "completed");
        } else if(strcmp(session_update_state, "going") == 0) {
          snprintf(session_state, 32, "%s", "going");
        } else if(strcmp(session_update_state, "failed") == 0) {
          if((session_error_code == 2) || (session_error_code == 3)) {
            snprintf(session_state, 32, "%s", "failed");  
          } else {
            snprintf(session_state, 32, "%s", "going");  
          }
        }
      } else {
        snprintf(session_state, 32, "%s", "received");
      }

    // expired/failed/cancelled
    } else {

      // int status = compare_received_session_id(session_id, session_state, session_update_state, session_update_json);
      
      if(status == 0) {

        if(strcmp(session_update_state, "going") == 0) {
          change_filelist_status = 1;
          snprintf(session_state, 32, "%s", "going");
        } else if(strcmp(session_update_state, "completed") == 0) {
          snprintf(session_state, 32, "%s", "completed");
        } else if(strcmp(session_update_state, "failed") == 0) {
          snprintf(session_state, 32, "%s", "failed");
        }
      } else {
        snprintf(session_state, 32, "%s", "failed");
      }
    }

    if(b_print) {
      Cdbg(API_DBG, "receive, cmp after, session_id -> %s, session_state -> %s, session_update_state -> %s, session_error_code = %d", session_id, session_state, session_update_state, session_error_code);
      LogInfo( ( "compare_received_session, receive, cmp after, session_id -> %s, session_state -> %s, session_update_state -> %s, session_error_code = %d\n", session_id, session_state, session_update_state, session_error_code) );
    }


    create_session_array_obj = json_object_new_object();
    //new a null
    json_object_object_add(create_session_array_obj, "topic", session_array_topic);
    json_object_object_add(create_session_array_obj, "id", session_array_id);
    json_object_object_add(create_session_array_obj, "gameid", session_array_gameid);
    json_object_object_add(create_session_array_obj, "ver", session_array_ver);
    json_object_object_add(create_session_array_obj, "expire", session_array_expire);
    json_object_object_add(create_session_array_obj, "state", json_object_new_string(session_state));


    if( (access(TENCENT_SESSION_CHANGE_FILELIST, 0 ) == 0) && 
        (change_filelist_status == 1)                      ) {

      filelist_array = json_object_from_file(TENCENT_SESSION_CHANGE_FILELIST);
      remove(TENCENT_SESSION_CHANGE_FILELIST);

      LogInfo( ( "compare_received_session, filelist_array  GET File" ) );

    } else {
      json_object_object_get_ex(session_array_obj, "filelist", &filelist_array);
      LogInfo( ( "compare_received_session, filelist_array  GET receive" ) );
    }


    if(filelist_array) {
      
      create_filelist_array_obj = json_object_new_array();

      int filelist_len, j; 
      // session_array is an array of objects
      filelist_len = json_object_array_length(filelist_array);

      for (j = 0; j < filelist_len; j++) {

        // get the i-th object in session_array
        filelist_obj = json_object_array_get_idx(filelist_array, j);

        json_object_object_get_ex(filelist_obj, "src", &filelist_array_src);
        json_object_object_get_ex(filelist_obj, "md5", &filelist_array_md5);
        json_object_object_get_ex(filelist_obj, "size", &filelist_array_size);
        json_object_object_get_ex(filelist_obj, "dst", &filelist_array_dst);
        json_object_object_get_ex(filelist_obj, "platform", &filelist_array_platform);
        json_object_object_get_ex(filelist_obj, "meta", &filelist_array_meta);


        // IOT_INFO("src=%s\n", json_object_get_string(filelist_array_src));
        // IOT_INFO("md5=%s\n", json_object_get_string(filelist_array_md5));
        // IOT_INFO("size=%s\n", json_object_get_string(filelist_array_size));
        // IOT_INFO("dst=%s\n", json_object_get_string(filelist_array_dst));

        create_filelist_obj = json_object_new_object();
        // //new a null
        json_object_object_add(create_filelist_obj, "src", filelist_array_src);
        json_object_object_add(create_filelist_obj, "md5", filelist_array_md5);
        json_object_object_add(create_filelist_obj, "size", filelist_array_size);
        json_object_object_add(create_filelist_obj, "dst", filelist_array_dst);
        json_object_object_add(create_filelist_obj, "platform", filelist_array_platform);
        json_object_object_add(create_filelist_obj, "meta", filelist_array_meta);

        json_object_array_add(create_filelist_array_obj, create_filelist_obj);


        // json_object_array_add(create_filelist_array_obj, filelist_obj);

      }

      json_object_object_add(create_session_array_obj, "filelist", create_filelist_array_obj);
      json_object_array_add(create_session_array, create_session_array_obj);

      // json_object_object_del(session_array_obj, "filelist");
      // IOT_INFO("111111111111111 json_object_put(session_array)\n")
      // json_object_object_add(session_array_obj, "filelist", create_filelist_array_obj);

      // IOT_INFO("111111111111111 json_object_put(session_array)\n")
      // json_object_array_add(create_session_array, session_array_obj);
      // IOT_INFO("322222222222222222222222 json_object_put(session_array)\n")

    }
  }

  json_object_to_file(TENCENT_SESSION_FILE, create_session_array);

  LogInfo( ( "compare_received_session json_object_put(create_session_array)" ) );

  json_object_put(create_session_array);

  LogInfo( ( "compare_received_session json_object_put(session_array)" ) );

  json_object_put(session_array);

  LogInfo( ( "compare_received_session end\n" ) );

  return 0;
}


int save_received_session(const char* json_data, const char* subscribe_topic) {

  int i;

  struct json_object *session_array = NULL;

  session_array = json_tokener_parse(json_data);
  
  if(is_error(session_array)) {
  // if (!session_array) {
    LogInfo( ( "json_tokener_parse failure" ) );
    return -1;
  }


  struct json_object *session_array_obj;

  struct json_object *session_array_id, *session_array_gameid, *session_array_ver;
  struct json_object *session_array_expire, *session_array_state;

  // int session_array_len;
  // session_array is an array of objects
  int session_array_len = json_object_array_length(session_array);



  for (i = 0; i < session_array_len; i++) {

    // get the i-th object in session_array
    session_array_obj = json_object_array_get_idx(session_array, i);

    json_object_object_get_ex(session_array_obj, "id", &session_array_id);
    json_object_object_get_ex(session_array_obj, "gameid", &session_array_gameid);
    json_object_object_get_ex(session_array_obj, "ver", &session_array_ver);
    json_object_object_get_ex(session_array_obj, "expire", &session_array_expire);
    json_object_object_get_ex(session_array_obj, "state", &session_array_state);



    Cdbg(API_DBG, "session id = %s, gameid = %s, ver = %s, expire = %s, state = %s",
        json_object_get_string(session_array_id),
        json_object_get_string(session_array_gameid),
        json_object_get_string(session_array_ver),
        json_object_get_string(session_array_expire),
        json_object_get_string(session_array_state)  );


    LogInfo( ( "save_received_session session id = %s, gameid = %s, ver = %s, expire = %s, state = %s",
        json_object_get_string(session_array_id),
        json_object_get_string(session_array_gameid),
        json_object_get_string(session_array_ver),
        json_object_get_string(session_array_expire),
        json_object_get_string(session_array_state)  ) );

    json_object_object_add(session_array_obj, "topic", json_object_new_string(subscribe_topic));


  }

  if(session_array_len == 0) {
    Cdbg(API_DBG, "not session data [ ]");
  }


  LogInfo( ( "json_object_array_add)" ) );
  // IOT_INFO("json_object_to_file (json_object_get_string(session_array)) = %s", json_object_get_string(session_array));

  json_object_to_file(TENCENT_SESSION_TOPIC, session_array);

  LogInfo( ( "json_object_put(session_array)" ) );

  json_object_put(session_array);

  LogInfo( ( "parse_received_session end" ) );


  return 0;
}

int merge_received_session(const int subscribe_topic_number) {


  // int session_len = JSON_CONTENT_SIZE*SUBSCRIBE_TOPIC_MAX_NUMBER;
  int session_len = JSON_CONTENT_SIZE*subscribe_topic_number;
  char session_topic_json[session_len];
  char session_other_json[session_len];

  int topic_status, other_status;
  // copy_tencent_update_tmp();    // json parse bug, add process speed

  topic_status = read_file_data(TENCENT_SESSION_TOPIC, session_topic_json, session_len); 

  other_status = read_file_data(TENCENT_SESSION_OTHER, session_other_json, session_len); 


  int topic_len = strlen(session_topic_json) + 1;
  int other_len = strlen(session_other_json) + 1;

  char new_topic_session[topic_len];
  char new_other_session[other_len];

  memset(new_topic_session, 0, topic_len);
  memset(new_other_session, 0, other_len);

  if(topic_len > 20) {
    strncpy(new_topic_session, session_topic_json + 1, strlen(session_topic_json) - 2);
  }

  if(other_len > 20) {
    strncpy(new_other_session, session_other_json + 1, strlen(session_other_json) - 2);
  }

  int received_len = topic_len + other_len + 4;
  char received_data[received_len];

  LogInfo( ( "topic_len = %d, other_len = %d, received_len %d\n", topic_len, other_len, received_len) );



  if((topic_len < 20) && ( other_len < 20 ) ) {
    snprintf(received_data, received_len, "[]");
  } else if((topic_len > 20) && ( other_len < 20 ) ) {
    snprintf(received_data, received_len, "[%s]", new_topic_session);
  } else if((topic_len < 20) && ( other_len > 20 ) ) {
    snprintf(received_data, received_len, "[%s]", new_other_session);
  } else if((topic_len > 20) && ( other_len > 20 ) ) {
    snprintf(received_data, received_len, "[%s, %s]", 
      new_topic_session, new_other_session);
  } 

  LogInfo( ( "received_data = %s\n", received_data) );


  write_file(TENCENT_SESSION_FILE, received_data);

  return 0;
}


int read_file_data(char *filename, char *data, int data_len) {

  int status = 0;

  FILE *pFile;

  //pFile = fopen("aicloud.conf", "r");
  pFile = fopen(filename, "rb+");
  if ( NULL == pFile ){
    LogInfo( ( "Open %s file failure\n", filename) );
    return -1;
  }else{
    // IOT_INFO("Open %s file success\n", filename);
  }

  fseek(pFile, 0, SEEK_END);
  long file_len = ftell(pFile);
  fseek(pFile, 0, SEEK_SET);


  char buffer[file_len+1];
  memset(buffer, 0, sizeof(buffer));

  //fread( buffer, file_len, 1, pFile );
  fread(buffer, 1, file_len, pFile);

  fclose(pFile);

  snprintf(data, data_len, "%s", buffer);

  return 0;
}



// char *strrpc(char *str,char *oldstr,char *newstr);


// char *strrpc(char *str,char *oldstr,char *newstr){
//   char bstr[strlen(str)];//轉換緩衝區
//   memset(bstr,0,sizeof(bstr));

//   int i = 0;
//   for(i = 0;i < strlen(str);i++){
//     if(!strncmp(str+i,oldstr,strlen(oldstr))){//查詢目標字串
//       strcat(bstr,newstr);
//       i += strlen(oldstr) - 1;
//     }else{
//       strncat(bstr,str + i,1);//儲存一位元組進緩衝區
//     }
//   }

//   strcpy(str,bstr);
//   return str;
// }




// #include "mbedtls/sha256.h" /* SHA-256 only */
// #include "mbedtls/md.h"     /* generic interface */

// int get_sha256(const char *input_str, unsigned char * output_buffer) {  

//   unsigned char sha256_output[32]; /* SHA-256 outputs 32 bytes */

//   mbedtls_sha256(input_str, strlen(input_str), sha256_output, 0);

//   int len = sizeof sha256_output;

//   int i = 0;
//   for(i = 0; i < len; i++) {
//     sprintf(output_buffer + (i * 2), "%02x", sha256_output[i]);
//   }

//   output_buffer[64] = '\0';
//   return 0;
// }



#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <openssl/md5.h>


void get_sha256(const char *input_str, unsigned char * output_buffer)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, input_str, strlen(input_str));
    SHA256_Final(hash, &sha256);
    int i = 0;
    for(i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        sprintf(output_buffer + (i * 2), "%02x", hash[i]);
    }
    output_buffer[64] = '\0';
}


int get_usb_space(unsigned long long int *total_space, unsigned long long int *used_space, unsigned long long int *available_space, char *tencent_download_path) {


  DIR* dir = opendir(tencent_download_path);
  
  if (dir) {
      /* Directory exists. */
      closedir(dir);
  } else if (ENOENT == errno) {
      /* Directory does not exist. */
    Cdbg(API_DBG, "%s > Directory does not exist", tencent_download_path);
    return 0;
  } else {
      /* opendir() failed for some other reason. */
    Cdbg(API_DBG, "%s > opendir() failed for some other reason", tencent_download_path);
    return 0;
  }


  char usb_path[64] = {0};
  int usb_path_len = strlen(tencent_download_path) - 5;   // remove /.INO
  strncpy(usb_path, tencent_download_path, usb_path_len);


  char df_command[128] = {0};
  snprintf(df_command, 128, "df %s > /tmp/df_tmp 2>&1", usb_path);

  system(df_command);

  FILE *fp;
  char file_info[1023];

  if ((fp = fopen("/tmp/df_tmp", "r")) == NULL) {
    Cdbg(API_DBG, "%s open_file_error", "/tmp/df_tmp");
    return -1;
  }

  int i = 0;

  while(fgets(file_info, 1023, fp) != NULL) {

    i++;
    if(i == 1) {
      continue;
    }

    char filesystem[32];

    sscanf(file_info, "%s  %llu %llu %llu", filesystem, total_space, used_space, available_space);

    // if(total_space < 0) {

    //   total_space = 0;
    //   available_space = 0;
    //   total_space = 0;z

    //   Cdbg(API_DBG, "Get Usb data error, total_space = 0, available_space = 0");
    // }

  }

  fclose(fp);

  return 0;
}



void getTimeInMillis(char* timestamp_str)
{


  struct timeval t_val;
  gettimeofday(&t_val, NULL);
  
  // printf("start, now, sec=%ld m_sec=%d \n", t_val.tv_sec, t_val.tv_usec);
  
  sprintf(timestamp_str,"%ld", t_val.tv_sec);
  
}
