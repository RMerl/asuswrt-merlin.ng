
#include "json.h"

int write_file(char *filename, char *json_data);

int str2md5(const char *input, char *output);

void getTimeInMillis(char* timestamp_str);

int search_local_db_file();

int upload_conn_diag_db(char* filename);

int upload_setting_backup(char* filename);

int setting_file_output();

int mac_toupper(char * mac, char * output);

int delete_sub_str(const char *str, const char *sub_str, char *result_str);