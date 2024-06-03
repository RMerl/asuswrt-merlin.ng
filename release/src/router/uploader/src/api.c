#include <ctype.h>
#include <limits.h>

#include <errno.h>

#include <time.h>
#include <curl/curl.h>

#include <sys/stat.h>

#include <dirent.h>


#include <shared.h>

#include "api.h"
#include "log.h"

#define API_DBG 1

#include "upload_api.h"
#include "uploader.h"

#include "webapi.h"

int write_file(char *filename, char* json_data) {

    FILE *pFile;

    pFile = fopen( filename, "w" );

    if( NULL == pFile ){
        Cdbg(API_DBG, "%s -> open failure", filename);
        return -1;
    }
    else {
        fprintf(pFile, "%s", json_data);
    }

    fclose(pFile);

    return 0;
}

void get_md5_string(const char* hwaddr, char* out_md5string) {

    //unsigned char* hwaddr = "00:0c:29:62:72:68";  
    DECLARE_CLEAR_MEM(unsigned char, md, MD_LEN);
    int i =0;
    MD5((const unsigned char*)hwaddr, strlen((const char*)hwaddr), md);

    memset(out_md5string, 0 , MD_STR_LEN+1);

    char tmp[3];
    for(i =0; i<MD_LEN; i++){
        memset(tmp,0,3);
        sprintf(tmp,"%02x", md[i]);
        strcat(out_md5string, tmp);
    }
    //printf(">>>>>md5stirng =%s", out_md5string);  
}

uint64_t get_timestamp() {

    struct timeval t_val;
    gettimeofday(&t_val, NULL);
    return t_val.tv_sec;
}

void getTimeInMillis(char* timestamp_str) {

    struct timeval t_val;
    gettimeofday(&t_val, NULL);
    
    sprintf(timestamp_str,"%ld", t_val.tv_sec);
}

#ifdef RTCONFIG_AMAS_CENTRAL_CONTROL
void backup_cfg_mnt() {

    FILE *fp = NULL;

    char cmd[128] = {0}, stat_file[128] = {0};
    char line_buf_s[300], filename[MAX_FILENAME_LEN], file_path[MAX_FILEPATH_LEN];
    char *line_buf = NULL;

    struct stat st = {0};
    int file_update_time = 0;

    snprintf(cmd, sizeof(cmd), "ls -RAlet %s |  head -n 2 > %s", AMASCNTRL_BACKUP_PATH, LS_LAST_TIME);
    system(cmd);

    //Cdbg(API_DBG, "cmd = %s", cmd);

    int i = 0;
    if ((fp = fopen(LS_LAST_TIME, "r"))) {

        while ( line_buf = fgets(line_buf_s, sizeof(line_buf_s), fp) ) {
            i++;
            // Cdbg(API_DBG, "i = %d, line_buf = %s", i, line_buf);
            if(i == 1) {
                sscanf(line_buf, "%[^:]", file_path);
            } 
            else if(i == 2) {
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

int format_mac(const char *mac, char *output) {

    char *remove_str = ":";  
    delete_sub_str(mac, remove_str, output);

    int i = 0;

    for(i = 0; i < sizeof(output); i++) {
        output[i] = toupper(output[i]);
    }

    return 0;
}

int delete_sub_str(const char *str, const char *sub_str, char *result_str) {  

    int count = 0;  
    int str_len = strlen(str);  
    int sub_str_len = strlen(sub_str);  
  
    if (str == NULL)  {  
        result_str = NULL;  
        return 0;  
    }  
  
    if (str_len < sub_str_len || sub_str == NULL) {  
        while (*str != '\0') {  
            *result_str = *str;  
            str++;  
            result_str++;  
        }  
  
        return 0;  
    }  
  
    while (*str != '\0') {  
        while (*str != *sub_str && *str != '\0') {  
            *result_str = *str;  
            str++;  
            result_str++;  
        }  
  
        if (strncmp(str, sub_str, sub_str_len) != 0) {  
            *result_str = *str;  
            str++;  
            result_str++;  
            continue;  
        }  
        else {  
            count++;  
            str += sub_str_len;  
        }  
    }  
  
    *result_str = '\0';  
  
    return count;  
}