
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


#include <shared.h>

#include "api.h"
#include "log.h"


#define API_DBG 1


#include "upload_api.h"
#include "uploader.h"

#include "webapi.h"

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



uint64_t get_timestamp()
{

  struct timeval t_val;
  gettimeofday(&t_val, NULL);
    
  return t_val.tv_sec;

}

void getTimeInMillis(char* timestamp_str)
{

  struct timeval t_val;
  gettimeofday(&t_val, NULL);
    
  sprintf(timestamp_str,"%ld", t_val.tv_sec);
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

    status = cloud_file_process(api, file_path, file_name);


    json_object_put(root);


  } else {
    Cdbg(API_DBG, "root json is invalid");
  }


  return status;
}

int local_backup_file_output(char *type, char *filename, int filesize, const int backup_file_timestamp) {

  int ret = -1;
  char cmd[128] = {0};

  if( strcmp(type, SETTING_BACKUP_TYPE) == 0 ) {

    snprintf(filename, filesize, "setting_%s_%d.cfg", mac_no_symbol, backup_file_timestamp);
    snprintf(cmd, sizeof(cmd), "nvram save %s%s", UPLOADER_FOLDER, filename);

  } else if( strcmp(type, OPENVPN_BACKUP_TYPE) == 0 ) {

    snprintf(filename, filesize, "openvpn_%s_%d.tar.gz", mac_no_symbol, backup_file_timestamp);

    if((ret = gen_server_ovpn_file()) == HTTP_OK){
      snprintf(cmd, sizeof(cmd), "mv %s %s%s ", OPENVPN_EXPORT_FILE, UPLOADER_FOLDER, filename);
    }

  } else if( strcmp(type, IPSEC_BACKUP_TYPE) == 0 ) {

    snprintf(filename, filesize, "ipsec_%s_%d.tar.gz", mac_no_symbol, backup_file_timestamp);

    if((ret = gen_server_ipsec_file()) == HTTP_OK){
      snprintf(cmd, sizeof(cmd), "mv %s %s%s ", IPSEC_EXPORT_FILE, UPLOADER_FOLDER, filename);
    }

  } else if( strcmp(type, USERICON_BACKUP_TYPE) == 0 ) {

    char usericon_export_file[128] = {0};

    snprintf(filename, filesize, "usericon_%s_%d.tar.gz", mac_no_symbol, backup_file_timestamp);
    snprintf(usericon_export_file, sizeof(usericon_export_file), "%s%s", UPLOADER_FOLDER, filename);

    if((ret = gen_jffs_backup_profile("usericon", usericon_export_file)) == HTTP_OK) {
      return PROCESS_SUCCESS;
    }

#ifdef RTCONFIG_AMAS_CENTRAL_CONTROL
  } else if( strcmp(type, AMASCNTRL_BACKUP_TYPE) == 0 ) {

    char amascntrl_export_file[128] = {0};

    snprintf(filename, filesize, "amascntrl_%s_%d.tar.gz", mac_no_symbol, backup_file_timestamp);
    snprintf(amascntrl_export_file, sizeof(amascntrl_export_file), "%s%s", UPLOADER_FOLDER, filename);

    if((ret = gen_jffs_backup_profile("amascntrl", amascntrl_export_file)) == HTTP_OK) {
      return PROCESS_SUCCESS;
    }
#endif  // RTCONFIG_AMAS_CENTRAL_CONTROL

  } else if( strcmp(type, UI_SUPORT_BACKUP_TYPE) == 0 ) {

    snprintf(filename, filesize, "ui_support_%s_%d.json", mac_no_symbol, backup_file_timestamp);
    if(update_ui_support_db(filename) == 0) {
      return PROCESS_SUCCESS;
    }

  } else {
    return -1;
  }

  ret = system(cmd);

  Cdbg(API_DBG, "cmd = %s, ret = %d, type = %s, file_output -> %s%s", cmd, ret, type, UPLOADER_FOLDER, filename);

  return PROCESS_SUCCESS;
}

#ifdef RTCONFIG_AMAS_CENTRAL_CONTROL
void backup_cfg_mnt() {

  FILE *fp = NULL;

  char cmd[128] = {0}, stat_file[128] = {0};
  char line_buf_s[300], filename[64], file_path[128];
  char *line_buf = NULL;

  struct stat st = {0};
  int file_update_time = 0;
  char amascntrl_update_name[FILENAME_LEN];

  snprintf(cmd, sizeof(cmd), "ls -RAlet %s |  head -n 2 > %s", AMASCNTRL_BACKUP_PATH, LS_LAST_TIME);
  system(cmd);

  Cdbg(API_DBG, "cmd = %s", cmd);

  int i = 0;
  if ((fp = fopen(LS_LAST_TIME, "r"))) {

    while ( line_buf = fgets(line_buf_s, sizeof(line_buf_s), fp) ) {
      i++;
      // Cdbg(API_DBG, "i = %d, line_buf = %s", i, line_buf);
      if(i == 1) {

        sscanf(line_buf, "%[^:]", file_path);

      } else if(i == 2) {

        sscanf(line_buf, "%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s%s", filename);
        snprintf(stat_file, sizeof(stat_file), "%s/%s",  file_path,  filename);

        if (lstat(stat_file, &st) == 0) {

          file_update_time = st.st_mtim.tv_sec;

          if(file_update_time > nvram_get_int("amascntrl_update_time")) {
            Cdbg(API_DBG, "stat_file = %s: file_update_time = %d  > amascntrl_update_time = %d\n", stat_file, file_update_time, nvram_get_int("amascntrl_update_time"));
            nvram_set_int("amascntrl_update_time", file_update_time);
          }
        }
      }
    }
    fclose(fp);
  }
}
#endif  // RTCONFIG_AMAS_CENTRAL_CONTROL


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