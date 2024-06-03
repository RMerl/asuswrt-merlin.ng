
#ifndef API_H_
#define API_H_

#include <openssl/md5.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "json.h"

#define MAC_MAX_LEN 18
#define MD_STR_LEN 33
#define MD_LEN 16
#define DECLARE_CLEAR_MEM(type, var, len) \
    type var[len]; \
    memset(var, 0, len );

#define CLEAR_MEM(var, len) \
    memset(var, 0, len );

int write_file(char *filename, char *json_data);

void get_md5_string(const char* hwaddr, char* out_md5string);

uint64_t get_timestamp();

void getTimeInMillis(char* timestamp_str);

int upload_conn_diag_db(char* filename);

#ifdef RTCONFIG_AMAS_CENTRAL_CONTROL
void backup_cfg_mnt();
#endif

int format_mac(const char * mac, char * output);

int delete_sub_str(const char *str, const char *sub_str, char *result_str);

#endif /* API_H_ */
