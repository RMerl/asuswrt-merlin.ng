
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>

#include <errno.h>

#include <time.h>
#include <curl/curl.h>
#include <stdio.h>

#include <sys/stat.h>

#include <openssl/md5.h>

#include <dirent.h>


#include "api.h"
#include "log.h"


#define API_DBG 1


#include "upload_api.h"
#include "uploader.h"

extern char mac_no_symbol[32];

int write_file(char *filename, char* json_data) 
{

    FILE *pFile;

    pFile = fopen( filename, "w" );

    if( NULL == pFile ){

        Cdbg(API_DBG, "%s -> open failure", filename);

        return -1;

    }else{
        fprintf(pFile, "%s", json_data);
    }

    fclose(pFile);

    return 0;
}


int str2md5(const char *input, char *output) 
{

  unsigned char digest[16];
  MD5_CTX context;
  MD5Init(&context);
  MD5Update(&context, input, strlen(input));
  MD5Final(digest, &context);


  int n;
  for (n = 0; n < 16; ++n) {
      snprintf(&(output[n*2]), 16*2, "%02x", (unsigned int)digest[n]);
  }

}

void getTimeInMillis(char* timestamp_str)
{


  struct timeval t_val;
  gettimeofday(&t_val, NULL);
  
  // printf("start, now, sec=%ld m_sec=%d \n", t_val.tv_sec, t_val.tv_usec);
  
  sprintf(timestamp_str,"%ld", t_val.tv_sec);
  
  // Cdbg(API_DBG, "timestamp_str = %s", timestamp_str);

  
  // long sec = t_val.tv_sec;
  // time_t t_sec = (time_t)sec;
  // printf("date:%s", ctime(&t_sec));

  //ToDo: bypass to use const value
  
  // char *ptr;
  // unsigned long long lval = 0;
  // struct timeval tp;  
  // gettimeofday(&tp, NULL);
  // //lval = (unsigned long long)((tp.tv_sec * 1000 * 1000) + (tp.tv_usec)) / 1000; 

  // lval = (unsigned long long)(tp.tv_sec * 1000 * 1000 ) + (tp.tv_usec); 
  // sprintf(timestamp_str,"%llu",lval);
 
  // Cdbg(API_DBG, "timestamp_str = %s", timestamp_str);

}



int search_local_db_file() {

  int count = 0;
  int status = -1;

  DIR *dp; 
  struct dirent *dirp;


  if (dp = opendir(UPLOADER_FOLDER)) {

    while (dirp = readdir(dp)) {

      // Cdbg(API_DBG, "upload file count = %d, filename = %s", count, dirp->d_name);

      if (strstr (dirp->d_name, ".cfg") ) {

        count++;

        // if(upload_conn_diag_db(dirp->d_name) == 0) {
        if(upload_setting_backup(dirp->d_name) == 0) {
          
          status = 0;

          char del_file[128];
          snprintf(del_file, sizeof(del_file), "%s%s", UPLOADER_FOLDER, dirp->d_name);

          if(!remove(del_file)) {
            Cdbg(API_DBG, "remove file = %s", del_file);
          }

        }

      }

    }

    closedir(dp);
  } 


  return status;

}


// parse json data
int upload_conn_diag_db(char* filename) {

  int status = -1;
  char upload_file[64] = {0};

  snprintf(upload_file, sizeof(upload_file), "%s%s", UPLOADER_FOLDER, filename);


  json_object *root = NULL;
  json_object *o_api = NULL, *o_file_path = NULL, *o_file_name = NULL;

  root = json_object_from_file(upload_file);

  if (root) {

    json_object_object_get_ex(root, "api", &o_api);
    json_object_object_get_ex(root, "file_path", &o_file_path);
    json_object_object_get_ex(root, "file_name", &o_file_name);

    const char *api = json_object_get_string(o_api);
    const char *file_path = json_object_get_string(o_file_path);
    const char *file_name = json_object_get_string(o_file_name);

    Cdbg(API_DBG, "upload_conn_diag_db api = %s", api);
    Cdbg(API_DBG, "upload_conn_diag_db file_path = %s", file_path);
    Cdbg(API_DBG, "upload_conn_diag_db file_name = %s", file_name);

    status = db_file_process(api, file_path, file_name);


    json_object_put(root);


  } else {
    Cdbg(API_DBG, "root json is invalid");
  }


  return status;
}

// setting file
int upload_setting_backup(char* filename) {

  int status = -1;
  char upload_file[64] = {0};


  status = db_file_process(UPLOAD_FILE_TO_S3, UPLOADER_FOLDER, filename);


  return status;
}

int setting_file_output() {

  int status = -1;

  char filename[64] = {0};

  char timestamp_str[16] = {0};
  getTimeInMillis(timestamp_str);

  snprintf(filename, sizeof(filename), "router_%s_%s.cfg", mac_no_symbol, timestamp_str);

  Cdbg(API_DBG, "setting_file_output -> %s%s", UPLOADER_FOLDER, filename);


  char settings_cmd[128] = {0};
  snprintf(settings_cmd, sizeof(settings_cmd), "nvram save %s%s", UPLOADER_FOLDER, filename);

  // remove(settings_date);

  status = system(settings_cmd);

  Cdbg(API_DBG, "system  settings_cmd status = %d", status);

  return 0;
}


int mac_toupper(char * mac, char * output) {

  char *remove_str = ":";  
  delete_sub_str(mac, remove_str, output);

  int i = 0;

  for(i = 0; i < sizeof(output); i++) {
    output[i] = toupper(output[i]);
  }

  return 0;

}

int delete_sub_str(const char *str, const char *sub_str, char *result_str)  
{  
    int count = 0;  
    int str_len = strlen(str);  
    int sub_str_len = strlen(sub_str);  
  
    if (str == NULL)  
    {  
        result_str = NULL;  
        return 0;  
    }  
  
    if (str_len < sub_str_len || sub_str == NULL)  
    {  
        while (*str != '\0')  
        {  
            *result_str = *str;  
            str++;  
            result_str++;  
        }  
  
        return 0;  
    }  
  
    while (*str != '\0')  
    {  
        while (*str != *sub_str && *str != '\0')  
        {  
            *result_str = *str;  
            str++;  
            result_str++;  
        }  
  
        if (strncmp(str, sub_str, sub_str_len) != 0)  
        {  
            *result_str = *str;  
            str++;  
            result_str++;  
            continue;  
        }  
        else  
        {  
            count++;  
            str += sub_str_len;  
        }  
    }  
  
    *result_str = '\0';  
  
    return count;  
} 

// int main( int argc, char ** argv )
// {
//   return 0;
// }