#include <stdio.h>
#include <string.h>
#include "json.h"
#include "uploader_config.h"

#include "log.h"

#include "api.h"

#define UC_DBG 1


int process_google_config(struct google_conf * google_d_conf) {


  FILE *pFile;
  char buffer[1024];
  memset(buffer,0,sizeof(buffer));

  //pFile = fopen("aicloud.conf", "r");
  pFile = fopen("/tmp/router/googleToken", "r");
  if ( NULL == pFile ){
    Cdbg(UC_DBG, "Open googleToken file failure");
    return 1;
  }else{
    Cdbg(UC_DBG, "Open googleToken file success");

    fread( buffer, 1024, 1, pFile );

    //fread(buffer, 1, 1024, pFile);
    //buffer[(sizeof buffer)-1] = 0;
    //printf( "%s", buffer );
  }

  fclose(pFile);


  Cdbg(UC_DBG, "googleToken content : %s", buffer);
  json_object * jobj = json_tokener_parse(buffer);
  google_config_parse(jobj, google_d_conf);


  return 0;
}



void google_config_parse(json_object * jobj ,struct google_conf * google_d_conf) {

  enum json_type type;

  int orig_count = 0;
    
  json_object_object_foreach(jobj, key, val) {


    type = json_object_get_type(val);
    switch (type) {
      case json_type_string: 
        //Cdbg(APP_DBG, "type: json_type_string, ");
        //Cdbg(APP_DBG, "value: %s \n", json_object_get_string(val));
      break;
    }

    orig_count++;


    //Cdbg(APP_DBG, "Key at index %d is [%s]", orig_count, key);
    //Cdbg(APP_DBG, "value: %s \n", json_object_get_string(val));


    if (strcmp(key, "client_id") == 0) {

      strcpy(&google_d_conf->client_id[0], json_object_get_string(val));
      continue;
    } 

    if (strcmp(key, "client_secret") == 0) {

      strcpy(&google_d_conf->client_secret[0], json_object_get_string(val));
      continue;
    } 

    if (strcmp(key, "access_token") == 0) {

      strcpy(&google_d_conf->access_token[0], json_object_get_string(val));
      continue;
    } 

    if (strcmp(key, "refresh_token") == 0) {

      strcpy(&google_d_conf->refresh_token[0], json_object_get_string(val));

      continue;
    } 

  }
}



int process_google_refresh_token(char * filename, struct google_conf * google_d_conf) {


  int status = 0;

  FILE *pFile;

  //pFile = fopen("aicloud.conf", "r");
  pFile = fopen(filename, "r");
  if ( NULL == pFile ){
    Cdbg(UC_DBG, "Open %s file failure", filename);
    return -1;
  }else{
    Cdbg(UC_DBG, "Open %s file success", filename);


    //fread(buffer, 1, 1024, pFile);
    //buffer[(sizeof buffer)-1] = 0;
    //printf( "%s", buffer );
  }

  fseek(pFile, 0, SEEK_END);
  long file_len = ftell(pFile);
  fseek(pFile, 0, SEEK_SET);


  // Cdbg(UC_DBG, "googleToken file_len : %lu", file_len);

  char buffer[file_len+1];
  memset(buffer, 0, sizeof(buffer));

  //fread( buffer, file_len, 1, pFile );
  fread(buffer, 1, file_len, pFile);


  fclose(pFile);


  Cdbg(UC_DBG, "googleToken content : %s", buffer);

  json_object * jobj = json_tokener_parse(buffer);
  status = google_refresh_token_parse(jobj, google_d_conf);


  return status;
}


int google_refresh_token_parse(json_object * jobj ,struct google_conf * google_d_conf) {

  enum json_type type;

  int orig_count = 0;
    
  json_object_object_foreach(jobj, key, val) {


    type = json_object_get_type(val);
    switch (type) {
      case json_type_string: 
        //Cdbg(APP_DBG, "type: json_type_string, ");
        //Cdbg(APP_DBG, "value: %s \n", json_object_get_string(val));
      break;
    }

    orig_count++;


    // Cdbg(APP_DBG, "Key at index %d is [%s]", orig_count, key);
    // Cdbg(APP_DBG, "value: %s \n", json_object_get_string(val));



    if (strcmp(key, "access_token") == 0) {

      strcpy(&google_d_conf->access_token[0], json_object_get_string(val));
      continue;
    } 

    if (strcmp(key, "error") == 0) {

      return -1;
    } 

  }

  return 0;
}




int process_google_create_folder(char * filename, struct google_conf * google_d_conf, char * foldername, char * data_type) {


  FILE *pFile;

  //pFile = fopen("aicloud.conf", "r");
  pFile = fopen(filename, "r");
  if ( NULL == pFile ){
    Cdbg(UC_DBG, "Open %s file failure", filename);
    return -1;
  }else{
    Cdbg(UC_DBG, "Open %s file success", filename);


    //fread(buffer, 1, 1024, pFile);
    //buffer[(sizeof buffer)-1] = 0;
    //printf( "%s", buffer );
  }

  fseek(pFile, 0, SEEK_END);
  long file_len = ftell(pFile);
  fseek(pFile, 0, SEEK_SET);


  // Cdbg(UC_DBG, "googleToken file_len : %lu", file_len);

  char buffer[file_len];
  memset(buffer, 0, sizeof(buffer));

  //fread( buffer, file_len, 1, pFile );
  fread(buffer, 1, file_len, pFile);


  fclose(pFile);


  // Cdbg(UC_DBG, "googleToken content : %s", buffer);

  json_object * jobj = json_tokener_parse(buffer);
  google_create_folder_parse(jobj, google_d_conf, foldername, data_type);


  return 0;
}


void google_create_folder_parse(json_object * jobj ,struct google_conf * google_d_conf, char * foldername, char * data_type) {

  enum json_type type;

  int orig_count = 0;
    
  json_object_object_foreach(jobj, key, val) {


    type = json_object_get_type(val);
    switch (type) {
      case json_type_string: 
        //Cdbg(APP_DBG, "type: json_type_string, ");
        //Cdbg(APP_DBG, "value: %s \n", json_object_get_string(val));
      break;
    }

    orig_count++;


    // Cdbg(APP_DBG, "Key at index %d is [%s]", orig_count, key);
    // Cdbg(APP_DBG, "value: %s \n", json_object_get_string(val));



    if (strcmp(key, "id") == 0) {


      if(strcmp(data_type, DRIVE_FOLDER_TYPE) == 0) {

        if (strcmp(foldername, ASUS_FOLDER_NAME) == 0) {

          strcpy(&google_d_conf->asus_folder_id[0], json_object_get_string(val));

        } else if (strcmp(foldername, ROUTER_ROUTER_NAME) == 0) {

          strcpy(&google_d_conf->router_folder_id[0], json_object_get_string(val));

        } else if (strcmp(foldername, LOG_FOLDER_NAME) == 0) {

          strcpy(&google_d_conf->log_folder_id[0], json_object_get_string(val));

        } else if (strlen(foldername) == 8) {

          strcpy(&google_d_conf->date_folder_id[0], json_object_get_string(val));

        }

      } else if(strcmp(data_type, DRIVE_FILE_TYPE) == 0) {

        if (strcmp(foldername, DRIVE_FILE_ID) == 0) {
            strcpy(&google_d_conf->file_id[0], json_object_get_string(val));
        }
      }

      continue;
    } 





  }
}


int process_google_drive_data(char * filename, struct google_conf * google_d_conf, char * foldername, char * data_type) {



  FILE *pFile;

  //pFile = fopen("aicloud.conf", "r");
  pFile = fopen(filename, "r");
  if ( NULL == pFile ){
    Cdbg(UC_DBG, "Open %s file failure", filename);
    return -1;
  }else{
    Cdbg(UC_DBG, "Open %s file success", filename);
  }

  fclose(pFile);

  struct json_object *files_obj, *files_array, *files_array_obj, *files_array_obj_name;

  int arraylen, i;

  files_obj = json_object_from_file(filename);
  files_array = json_object_object_get(files_obj, "files");

  // files_array is an array of objects
  arraylen = json_object_array_length(files_array);

  Cdbg(UC_DBG, "access_token_json parse files len : %d\n", arraylen);


  for (i = 0; i < arraylen; i++) {
    // get the i-th object in files_array
    files_array_obj = json_object_array_get_idx(files_array, i);
    // get the name attribute in the i-th object
    files_array_obj_name = json_object_object_get(files_array_obj, "id");
    // print out the name attribute
    Cdbg(UC_DBG, "file id=%s\n", json_object_get_string(files_array_obj_name));

    if(strcmp(data_type, DRIVE_FOLDER_TYPE) == 0) {

      if (strcmp(foldername, ASUS_FOLDER_NAME) == 0) {

        strcpy(&google_d_conf->asus_folder_id[0], json_object_get_string(files_array_obj_name));

      } else if (strcmp(foldername, ROUTER_ROUTER_NAME) == 0) {

        strcpy(&google_d_conf->router_folder_id[0], json_object_get_string(files_array_obj_name));

      } else if (strcmp(foldername, LOG_FOLDER_NAME) == 0) {

        strcpy(&google_d_conf->log_folder_id[0], json_object_get_string(files_array_obj_name));

      } else if (strlen(foldername) == 8) {

        strcpy(&google_d_conf->date_folder_id[0], json_object_get_string(files_array_obj_name));

      }
      
    } else if(strcmp(data_type, DRIVE_FILE_TYPE) == 0) {

      if (strcmp(foldername, DRIVE_FILE_ID) == 0) {

        strcpy(&google_d_conf->file_id[0], json_object_get_string(files_array_obj_name));

      }
    }

    return 0;
  }


  return -1;

}


int process_asuswebstorage_config(struct asuswebstorage_conf * asus_conf) {

  FILE *pFile;
  char buffer[1024];
  memset(buffer,0,sizeof(buffer));

  pFile = fopen("/tmp/router/uploader.conf", "r");
  if ( NULL == pFile ){
    Cdbg(UC_DBG, "Open uploader.conf file failure");
    return 1;
  }else{
    Cdbg(UC_DBG, "Open uploader.conf file success");
    fread( buffer, 1024, 1, pFile );
  }

  fclose(pFile);

  Cdbg(UC_DBG, "uploader.conf content : %s", buffer);

  json_object * jobj = json_tokener_parse(buffer);
  asuswebstorage_config_parse(jobj, asus_conf);

  my_mkdir(asus_conf->uploader_path); 
}







void asuswebstorage_config_parse(json_object * jobj ,struct asuswebstorage_conf * asus_conf) {

  enum json_type type;

  int orig_count = 0;
    
  json_object_object_foreach(jobj, key, val) {


    type = json_object_get_type(val);
    switch (type) {
      case json_type_string: 
        //Cdbg(APP_DBG, "type: json_type_string, ");
        //Cdbg(APP_DBG, "value: %s \n", json_object_get_string(val));
      break;
    }

    orig_count++;


    //Cdbg(APP_DBG, "Key at index %d is [%s]", orig_count, key);
    //Cdbg(APP_DBG, "value: %s \n", json_object_get_string(val));

    if (strcmp(key, "servicegateway") == 0) {

      strcpy(&asus_conf->servicegateway[0], json_object_get_string(val));
      continue;
    } 

    if (strcmp(key, "serviceid") == 0) {

      strcpy(&asus_conf->serviceid[0], json_object_get_string(val));

      continue;
    } 

    if (strcmp(key, "key") == 0) {

      strcpy(&asus_conf->key[0], json_object_get_string(val));

      continue;
    } 

/*
    if (strcmp(key, "otainfourl") == 0) {

      strcpy(&asus_conf->otainfourl[0], json_object_get_string(val));

      continue;
    } 

    if (strcmp(key, "otafileurl") == 0) {

      strcpy(&asus_conf->otafileurl[0], json_object_get_string(val));

      continue;
    } 
*/

    if (strcmp(key, "username") == 0) {

      strcpy(&asus_conf->username[0], json_object_get_string(val));

      continue;
    } 

    if (strcmp(key, "password") == 0) {
      strcpy(&asus_conf->password[0], json_object_get_string(val));
      continue;
    } 

/*
    if (strcmp(key, "sid") == 0) {
      strcpy(&asus_conf->sid[0], json_object_get_string(val));
      continue;
    } 

    if (strcmp(key, "progKey") == 0) {
      strcpy(&asus_conf->progKey[0], json_object_get_string(val));
      continue;
    } 
*/

    if (strcmp(key, "uploader_path") == 0) {
      strcpy(&asus_conf->uploader_path[0], json_object_get_string(val));
      continue;
    }

    if (strcmp(key, "device_id") == 0) {
      strcpy(&asus_conf->device_id[0], json_object_get_string(val));
      continue;
    } 

    if (strcmp(key, "device_name") == 0) {
      strcpy(&asus_conf->device_name[0], json_object_get_string(val));
      continue;
    } 


    if (strcmp(key, "merchandise_type") == 0) {
      strcpy(&asus_conf->merchandise_type[0], json_object_get_string(val));
      continue;
    } 

    if (strcmp(key, "client_set") == 0) {
      strcpy(&asus_conf->client_set[0], json_object_get_string(val));
      continue;
    } 
    

    if (strcmp(key, "device_manager_host") == 0) {
      strcpy(&asus_conf->device_manager_host[0], json_object_get_string(val));
      continue;
    } 

    if (strcmp(key, "client_type") == 0) {
      strcpy(&asus_conf->client_type[0], json_object_get_string(val));
      continue;
    } 

    if (strcmp(key, "client_version") == 0) {
      strcpy(&asus_conf->client_version[0], json_object_get_string(val));
      continue;
    } 

    if (strcmp(key, "manufacturer") == 0) {
      strcpy(&asus_conf->manufacturer[0], json_object_get_string(val));
      continue;
    } 

    if (strcmp(key, "product_name") == 0) {
      strcpy(&asus_conf->product_name[0], json_object_get_string(val));
      continue;
    } 

    if (strcmp(key, "machine_id") == 0) {
      strcpy(&asus_conf->machine_id[0], json_object_get_string(val));
      continue;
    } 

    if (strcmp(key, "uuid") == 0) {
      strcpy(&asus_conf->uuid[0], json_object_get_string(val));
      continue;
    } 

    if (strcmp(key, "mac") == 0) {

      strcpy(&asus_conf->mac[0], json_object_get_string(val));


      char macSix[8];

      memset(macSix,0,sizeof(macSix));
      strncpy(macSix, &asus_conf->mac[0] + 6,  6);

      strcpy(&asus_conf->mac_name[0], "AiCAM_");
      strcat(&asus_conf->mac_name[0], macSix);


      continue;
    } 

  }
}



int process_aicloud_config(struct aicloud_conf * ai_conf) {


  FILE *pFile;
  char buffer[1024];
  memset(buffer,0,sizeof(buffer));



  //pFile = fopen("aicloud.conf", "r");
  pFile = fopen("/tmp/router/aicloud", "r");
  if ( NULL == pFile ){
    Cdbg(UC_DBG, "Open aicloud file failure");
    return 1;
  }else{
    Cdbg(UC_DBG, "Open aicloud file success");

    fread( buffer, 1024, 1, pFile );

    //fread(buffer, 1, 1024, pFile);
    //buffer[(sizeof buffer)-1] = 0;
    //printf( "%s", buffer );
  }

  fclose(pFile);


  Cdbg(UC_DBG, "aicloud content : %s", buffer);
  json_object * jobj = json_tokener_parse(buffer);
  aicloud_config_parse(jobj, ai_conf);

  my_mkdir(ai_conf->uploader_path); 


  return 0;
}


void aicloud_config_parse(json_object * jobj, struct aicloud_conf * ai_conf) {


  enum json_type type;

  int orig_count = 0;

  //Cdbg(APP_DBG, "Key at index %d \n", orig_count);
    
  json_object_object_foreach(jobj, key, val) {


    type = json_object_get_type(val);
    switch (type) {
      case json_type_string: 
        //Cdbg(APP_DBG, "aicloud_config type: json_type_string, ");
        //Cdbg(APP_DBG, "aicloud_config value: %s \n", json_object_get_string(val));
      break;
    }

    orig_count++;

    //Cdbg(APP_DBG, "aicloud config Key at index %d is [%s]", orig_count, key);
    //Cdbg(APP_DBG, "aicloud config value: %s \n", json_object_get_string(val));

    if (strcmp(key, "username") == 0) {

      strcpy(&ai_conf->username[0], json_object_get_string(val));

      continue;
    } 

    if (strcmp(key, "password") == 0) {
      strcpy(&ai_conf->password[0], json_object_get_string(val));
      continue;
    } 


    if (strcmp(key, "url") == 0) {
      strcpy(&ai_conf->url[0], json_object_get_string(val));
      continue;
    } 

    if (strcmp(key, "privateip") == 0) {
      strcpy(&ai_conf->privateip[0], json_object_get_string(val));
      continue;
    } 

    if (strcmp(key, "quota") == 0) {
      strcpy(&ai_conf->quota[0], json_object_get_string(val));
      continue;
    } 


    if (strcmp(key, "uploader_path") == 0) {
      strcpy(&ai_conf->uploader_path[0], json_object_get_string(val));
      continue;
    }

  }

}



int process_storage_provision_config(struct storage_provision_conf * sp_conf, char * file_path) {

  FILE *pFile;
  char buffer[1024];
  memset(buffer,0,sizeof(buffer));

  //pFile = fopen("storage_provision.conf", "r");

  pFile = fopen(file_path, "r");  
  if ( NULL == pFile ){
    Cdbg(UC_DBG, "Open storageProvision file failure");
    return 1;
  }else{
    Cdbg(UC_DBG, "Open storageProvision file success");
    fread( buffer, 1024, 1, pFile );
  }

  fclose(pFile);


  Cdbg(UC_DBG, "storageProvision content : %s", buffer);
  json_object * jobj = json_tokener_parse(buffer);
  storage_provision_config_parse(jobj, sp_conf);


  return 0;
}


void storage_provision_config_parse(json_object * jobj, struct storage_provision_conf * sp_conf) {


  enum json_type type;

  int orig_count = 0;

  //Cdbg(APP_DBG, "Key at index %d \n", orig_count);
    
  json_object_object_foreach(jobj, key, val) {


    type = json_object_get_type(val);
    switch (type) {
      case json_type_string: 
        //Cdbg(APP_DBG, "storage_provision_config type: json_type_string, ");
        //Cdbg(APP_DBG, "storage_provision_config value: %s \n", json_object_get_string(val));
      break;
    }

    orig_count++;

    //Cdbg(APP_DBG, "storage_provision config Key at index %d is [%s]", orig_count, key);
    //Cdbg(APP_DBG, "storage_provision config value: %s \n", json_object_get_string(val));

    if (strcmp(key, "timeStamp") == 0) {

      strcpy(&sp_conf->timeStamp[0], json_object_get_string(val));

      continue;
    } 

    if (strcmp(key, "alarmAccount") == 0) {
      strcpy(&sp_conf->alarmAccount[0], json_object_get_string(val));
      continue;
    } 


    if (strcmp(key, "alarmToken") == 0) {
      strcpy(&sp_conf->alarmToken[0], json_object_get_string(val));
      continue;
    } 

    if (strcmp(key, "alarmStorageType") == 0) {
      strcpy(&sp_conf->alarmStorageType[0], json_object_get_string(val));
      continue;
    } 

    if (strcmp(key, "alarmStorageQuota") == 0) {
      strcpy(&sp_conf->alarmStorageQuota[0], json_object_get_string(val));
      continue;
    } 


    if (strcmp(key, "alarmAppNotification") == 0) {
      strcpy(&sp_conf->alarmAppNotification[0], json_object_get_string(val));
      continue;
    }

    if (strcmp(key, "alarmEmailNotificaiton") == 0) {
      strcpy(&sp_conf->alarmEmailNotificaiton[0], json_object_get_string(val));
      continue;
    }

    if (strcmp(key, "recordAccount") == 0) {
      strcpy(&sp_conf->recordAccount[0], json_object_get_string(val));
      continue;
    }

    if (strcmp(key, "recordToken") == 0) {
      strcpy(&sp_conf->recordToken[0], json_object_get_string(val));
      continue;
    }

    if (strcmp(key, "recordStorageType") == 0) {
      strcpy(&sp_conf->recordStorageType[0], json_object_get_string(val));
      continue;
    }

    if (strcmp(key, "recordStorageURL") == 0) {
      strcpy(&sp_conf->recordStorageURL[0], json_object_get_string(val));
      continue;
    }


    if (strcmp(key, "recordStorageQuota") == 0) {
      strcpy(&sp_conf->recordStorageURL[0], json_object_get_string(val));
      continue;
    }

    if (strcmp(key, "recordStoragePath") == 0) {
      strcpy(&sp_conf->recordStorageURL[0], json_object_get_string(val));
      continue;
    }

    if (strcmp(key, "recordStoragePrivateIP") == 0) {
      strcpy(&sp_conf->recordStoragePrivateIP[0], json_object_get_string(val));
      continue;
    }

  }

}

int process_aaews_even_conf(char * filename, struct aaews_event_conf * ae_conf) {

  FILE *pFile;
  char buffer[1024];
  memset(buffer,0,sizeof(buffer));

  pFile = fopen(filename, "r");
  if ( NULL == pFile ){
    Cdbg(UC_DBG, "Open %s file failure", filename);
    return 1;
  }else{
    Cdbg(UC_DBG, "Open %s file success", filename);
    fread( buffer, 1024, 1, pFile );
    //printf( "%s", buffer );
  }

  fclose(pFile);

  Cdbg(UC_DBG, "%s content : %s", filename, buffer);
  json_object * jobj = json_tokener_parse(buffer);

  aaews_event_conf_parse(jobj, ae_conf);

  return 0;
}


void aaews_event_conf_parse(json_object * jobj, struct aaews_event_conf * ae_conf) {


  enum json_type type;

  int orig_count = 0;

  //Cdbg(APP_DBG, "Key at index %d \n", orig_count);
    
  json_object_object_foreach(jobj, key, val) {



    type = json_object_get_type(val);


    switch (type) {
      case json_type_string: 
        // Cdbg(APP_DBG, "type: json_type_string, ");
        // Cdbg(APP_DBG, "value: %s \n", json_object_get_string(val));
      break;
    }

    orig_count++;

    //Cdbg(APP_DBG, "Key at index %d is [%s]", orig_count, key);
    //Cdbg(APP_DBG, "value: %s \n", json_object_get_string(val));

    if (strcmp(key, "type") == 0) {

      strcpy(&ae_conf->type[0], json_object_get_string(val));

      continue;
    } 

    if (strcmp(key, "start_time") == 0) {

      strcpy(&ae_conf->start_time[0], json_object_get_string(val));

      continue;
    } 

    if (strcmp(key, "device_id") == 0) {

      strcpy(&ae_conf->device_id[0], json_object_get_string(val));

      continue;
    } 

    if (strcmp(key, "model") == 0) {

      strcpy(&ae_conf->model[0], json_object_get_string(val));

      continue;
    } 

    if (strcmp(key, "value1") == 0) {

      strcpy(&ae_conf->value1[0], json_object_get_string(val));

      continue;
    } 

    if (strcmp(key, "value2") == 0) {

      strcpy(&ae_conf->value2[0], json_object_get_string(val));

      continue;
    } 

    if (strcmp(key, "value3") == 0) {

      strcpy(&ae_conf->value3[0], json_object_get_string(val));

      continue;
    } 


  }
}


int process_aaews_provision_config(struct aaews_provision_conf * ap_conf) {

  FILE *pFile;
  char buffer[1024];
  memset(buffer,0,sizeof(buffer));

  pFile = fopen("/tmp/router/aaewsProvision", "r");  
  if ( NULL == pFile ){
    Cdbg(UC_DBG, "Open aaewsProvision file failure");
    return 1;
  }else{
    Cdbg(UC_DBG, "Open aaewsProvision file success");
    fread( buffer, 1024, 1, pFile );
    //printf( "%s", buffer );
  }

  fclose(pFile);

  Cdbg(UC_DBG, "aaewsProvision content : %s", buffer);
  json_object * jobj = json_tokener_parse(buffer);
  aaews_provision_config_parse(jobj, ap_conf);


  return 0;
}



void aaews_provision_config_parse(json_object * jobj, struct aaews_provision_conf * ap_conf) {


  enum json_type type;

  int orig_count = 0;

  //Cdbg(APP_DBG, "Key at index %d \n", orig_count);
    
  json_object_object_foreach(jobj, key, val) {


    type = json_object_get_type(val);
    switch (type) {
      case json_type_string: 
        //Cdbg(APP_DBG, "aaews_provision_config type: json_type_string, ");
        //Cdbg(APP_DBG, "aaews_provision_config value: %s \n", json_object_get_string(val));
      break;
    }

    orig_count++;

    //Cdbg(APP_DBG, "aaews_provision config Key at index %d is [%s]", orig_count, key);
    //Cdbg(APP_DBG, "aaews_provision config value: %s \n", json_object_get_string(val));


    if (strcmp(key, "timeStamp") == 0) {
      strcpy(&ap_conf->timeStamp[0], json_object_get_string(val));
      continue;
    } 

    if (strcmp(key, "account") == 0) {
      strcpy(&ap_conf->account[0], json_object_get_string(val));
      continue;
    } 

    if (strcmp(key, "password") == 0) {
      strcpy(&ap_conf->password[0], json_object_get_string(val));
      continue;
    } 

    if (strcmp(key, "token") == 0) {
      strcpy(&ap_conf->token[0], json_object_get_string(val));
      continue;
    }


    if (strcmp(key, "tokenExpireTime") == 0) {
      strcpy(&ap_conf->tokenExpireTime[0], json_object_get_string(val));
      continue;
    } 

    if (strcmp(key, "refreshToken") == 0) {
      strcpy(&ap_conf->refreshToken[0], json_object_get_string(val));
      continue;
    } 


    if (strcmp(key, "deviceName") == 0) {
      strcpy(&ap_conf->deviceName[0], json_object_get_string(val));
      continue;
    }

    if (strcmp(key, "timeZone") == 0) {
      strcpy(&ap_conf->timeZone[0], json_object_get_string(val));
      continue;
    }

  }

}



int process_sysinfo_config(struct sysinfo_conf * si_conf) {

  FILE *pFile;
  char buffer[1024];
  memset(buffer,0,sizeof(buffer));

  pFile = fopen("/tmp/router/sysinfo.txt", "r");  
  if ( NULL == pFile ){
    Cdbg(UC_DBG, "Open sysinfo.txt file failure");
    return 1;
  }else{
    Cdbg(UC_DBG, "Open sysinfo.txt file success");
    fread( buffer, 1024, 1, pFile );
  }

  fclose(pFile);

  Cdbg(UC_DBG, "sysinfo.txt content : %s", buffer);
  json_object * jobj = json_tokener_parse(buffer);
  sysinfo_config_parse(jobj, si_conf);


  return 0;
}



void sysinfo_config_parse(json_object * jobj, struct sysinfo_conf * si_conf) {


  enum json_type type;

  int orig_count = 0;

  //Cdbg(APP_DBG, "Key at index %d \n", orig_count);
    
  json_object_object_foreach(jobj, key, val) {


    type = json_object_get_type(val);
    switch (type) {
      case json_type_string: 
        //Cdbg(APP_DBG, "sysinfo_config type: json_type_string, ");
        //Cdbg(APP_DBG, "sysinfo_config value: %s \n", json_object_get_string(val));
      break;
    }

    orig_count++;

    //Cdbg(APP_DBG, "sysinfo config Key at index %d is [%s]", orig_count, key);
    //Cdbg(APP_DBG, "sysinfo config value: %s \n", json_object_get_string(val));


    if (strcmp(key, "deviceid") == 0) {
      strcpy(&si_conf->deviceid[0], json_object_get_string(val));
      continue;
    } 

    // if (strcmp(key, "macaddr") == 0) {
    //   strcpy(&si_conf->macaddr[0], json_object_get_string(val));
    //   continue;
    // }


  }

}





int process_deploy_config(struct deploy_conf * dp_conf) {

  FILE *pFile;
  char buffer[1024];
  memset(buffer,0,sizeof(buffer));

  pFile = fopen("/tmp/router/deploy", "r");  
  if ( NULL == pFile ){
    Cdbg(UC_DBG, "Open deploy file failure");
    return 1;
  }else{
    Cdbg(UC_DBG, "Open deploy file success");
    fread( buffer, 1024, 1, pFile );
  }

  fclose(pFile);

  Cdbg(UC_DBG, "deploy content : %s", buffer);
  json_object * jobj = json_tokener_parse(buffer);
  deploy_config_parse(jobj, dp_conf);


  return 0;
}



void deploy_config_parse(json_object * jobj, struct deploy_conf * dp_conf) {


  enum json_type type;

  int orig_count = 0;

  //Cdbg(APP_DBG, "Key at index %d \n", orig_count);

  json_object_object_foreach(jobj, key, val) {


    type = json_object_get_type(val);
    switch (type) {
      case json_type_string: 
        //Cdbg(APP_DBG, "sysinfo_config type: json_type_string, ");
        //Cdbg(APP_DBG, "sysinfo_config value: %s \n", json_object_get_string(val));
      break;
    }

    orig_count++;

    //Cdbg(APP_DBG, "sysinfo config Key at index %d is [%s]", orig_count, key);
    //Cdbg(APP_DBG, "sysinfo config value: %s \n", json_object_get_string(val));

    if (strcmp(key, "timeStamp") == 0) {
      strcpy(&dp_conf->timeStamp[0], json_object_get_string(val));
      continue;
    } 

    if (strcmp(key, "aaeAppId") == 0) {
      strcpy(&dp_conf->aaeAppId[0], json_object_get_string(val));
      continue;
    }

    if (strcmp(key, "aaeAppKey") == 0) {
      strcpy(&dp_conf->aaeAppKey[0], json_object_get_string(val));
      continue;
    }

    if (strcmp(key, "aaeAppPortal") == 0) {
      strcpy(&dp_conf->aaeAppPortal[0], json_object_get_string(val));
      continue;
    }

    if (strcmp(key, "aaeSid") == 0) {
      strcpy(&dp_conf->aaeSid[0], json_object_get_string(val));
      continue;
    }

    if (strcmp(key, "aaeOAuthPortal") == 0) {
      strcpy(&dp_conf->aaeOAuthPortal[0], json_object_get_string(val));
      continue;
    }

    if (strcmp(key, "awsAppId") == 0) {
      strcpy(&dp_conf->awsAppId[0], json_object_get_string(val));
      continue;
    }

    if (strcmp(key, "awsAppKey") == 0) {
      strcpy(&dp_conf->awsAppKey[0], json_object_get_string(val));
      continue;
    }

    if (strcmp(key, "awsAppGateway") == 0) {
      strcpy(&dp_conf->awsAppGateway[0], json_object_get_string(val));
      continue;
    }

    if (strcmp(key, "awsAppPortal") == 0) {
      strcpy(&dp_conf->awsAppPortal[0], json_object_get_string(val));
      continue;
    }

    if (strcmp(key, "awsSid") == 0) {
      strcpy(&dp_conf->awsSid[0], json_object_get_string(val));
      continue;
    }


    if (strcmp(key, "awsOAuthPortal") == 0) {
      strcpy(&dp_conf->awsOAuthPortal[0], json_object_get_string(val));
      continue;
    }

    if (strcmp(key, "otaInfoUrl") == 0) {
      strcpy(&dp_conf->otaInfoUrl[0], json_object_get_string(val));
      continue;
    }

    if (strcmp(key, "otaFileUrl") == 0) {
      strcpy(&dp_conf->otaFileUrl[0], json_object_get_string(val));
      continue;
    }

    if (strcmp(key, "fbRtmpUrl") == 0) {
      strcpy(&dp_conf->fbRtmpUrl[0], json_object_get_string(val));
      continue;
    }

    

  }

}





int process_basic_command_config(struct basic_command_conf * bc_conf) {

  FILE *pFile;
  char buffer[1024];
  memset(buffer,0,sizeof(buffer));

  pFile = fopen("/tmp/router/basicCommand", "r");  
  if ( NULL == pFile ){
    Cdbg(UC_DBG, "Open basicCommand file failure");
    return 1;
  }else{
    Cdbg(UC_DBG, "Open basicCommand file success");
    fread( buffer, 1024, 1, pFile );
  }

  fclose(pFile);

  Cdbg(UC_DBG, "basicCommand content : %s", buffer);
  json_object * jobj = json_tokener_parse(buffer);
  basic_command_config_parse(jobj, bc_conf);





  return 0;
}



void basic_command_config_parse(json_object * jobj, struct basic_command_conf * bc_conf) {


  enum json_type type;

  int orig_count = 0;

  //Cdbg(APP_DBG, "Key at index %d \n", orig_count);

  json_object_object_foreach(jobj, key, val) {


    type = json_object_get_type(val);
    switch (type) {
      case json_type_string: 
        //Cdbg(APP_DBG, "sysinfo_config type: json_type_string, ");
        //Cdbg(APP_DBG, "sysinfo_config value: %s \n", json_object_get_string(val));
      break;
    }

    orig_count++;

    //Cdbg(APP_DBG, "sysinfo config Key at index %d is [%s]", orig_count, key);
    //Cdbg(APP_DBG, "sysinfo config value: %s \n", json_object_get_string(val));

    if (strcmp(key, "timeStamp") == 0) {
      strcpy(&bc_conf->timeStamp[0], json_object_get_string(val));
      continue;
    } 

    if (strcmp(key, "cameraOnOff") == 0) {
      strcpy(&bc_conf->cameraOnOff[0], json_object_get_string(val));
      continue;
    }

    if (strcmp(key, "cameraLedOnOff") == 0) {
      strcpy(&bc_conf->cameraLedOnOff[0], json_object_get_string(val));
      continue;
    }

    if (strcmp(key, "deviceName") == 0) {
      strcpy(&bc_conf->deviceName[0], json_object_get_string(val));
      continue;
    }

    if (strcmp(key, "speakerVolume") == 0) {
      strcpy(&bc_conf->speakerVolume[0], json_object_get_string(val));
      continue;
    }

    if (strcmp(key, "motionAlarmOnOff") == 0) {
      strcpy(&bc_conf->motionAlarmOnOff[0], json_object_get_string(val));
      continue;
    }

    if (strcmp(key, "audioAlarmOnOff") == 0) {
      strcpy(&bc_conf->audioAlarmOnOff[0], json_object_get_string(val));
      continue;
    }

    if (strcmp(key, "recordOnOff") == 0) {
      strcpy(&bc_conf->recordOnOff[0], json_object_get_string(val));
      continue;
    }

    if (strcmp(key, "wdrOnOff") == 0) {
      strcpy(&bc_conf->wdrOnOff[0], json_object_get_string(val));
      continue;
    }

    if (strcmp(key, "ldcOnOff") == 0) {
      strcpy(&bc_conf->ldcOnOff[0], json_object_get_string(val));
      continue;
    }

  }

}





int process_system_config(struct system_conf * s_conf, char * file_path) {

  FILE *pFile;
  char buffer[1024];
  memset(buffer,0,sizeof(buffer));

  pFile = fopen(file_path, "r");  
  if ( NULL == pFile ){
    Cdbg(UC_DBG, "Open system file [%s] failure", file_path);
    return 1;
  }else{
    Cdbg(UC_DBG, "Open system file [%s] success", file_path);
    fread( buffer, 1024, 1, pFile );
  }

  fclose(pFile);

  Cdbg(UC_DBG, "system content : %s", buffer);
  json_object * jobj = json_tokener_parse(buffer);
  system_config_parse(jobj, s_conf);

  return 0;
}



void system_config_parse(json_object * jobj, struct system_conf * s_conf) {


  enum json_type type;

  int orig_count = 0;

  //Cdbg(APP_DBG, "Key at index %d \n", orig_count);

  json_object_object_foreach(jobj, key, val) {


    type = json_object_get_type(val);
    switch (type) {
      case json_type_string: 
        //Cdbg(APP_DBG, "sysinfo_config type: json_type_string, ");
        //Cdbg(APP_DBG, "sysinfo_config value: %s \n", json_object_get_string(val));
      break;
    }

    orig_count++;

    //Cdbg(APP_DBG, "sysinfo config Key at index %d is [%s]", orig_count, key);
    //Cdbg(APP_DBG, "sysinfo config value: %s \n", json_object_get_string(val));

    if (strcmp(key, "fwver") == 0) {
      strcpy(&s_conf->fwver[0], json_object_get_string(val));
      continue;
    } 

    if (strcmp(key, "tcode") == 0) {
      strcpy(&s_conf->tcode[0], json_object_get_string(val));
      continue;
    } 

    if (strcmp(key, "sn") == 0) {
      strcpy(&s_conf->sn[0], json_object_get_string(val));
      continue;
    } 

    if (strcmp(key, "apilevel") == 0) {
      strcpy(&s_conf->apilevel[0], json_object_get_string(val));
      continue;
    } 

    if (strcmp(key, "recordplan") == 0) {
      strcpy(&s_conf->recordplan[0], json_object_get_string(val));
      continue;
    } 


    if (strcmp(key, "privateip") == 0) {
      strcpy(&s_conf->privateip[0], json_object_get_string(val));
      continue;
    }

    if (strcmp(key, "macaddr") == 0) {
      strcpy(&s_conf->macaddr[0], json_object_get_string(val));
      continue;
    }

    if (strcmp(key, "videomode") == 0) {
      strcpy(&s_conf->videomode[0], json_object_get_string(val));
      continue;
    }

    if (strcmp(key, "initattachabledevice") == 0) {
      strcpy(&s_conf->initattachabledevice[0], json_object_get_string(val));
      continue;
    }


  }

}
