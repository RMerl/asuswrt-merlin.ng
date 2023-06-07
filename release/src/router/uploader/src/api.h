
#include "json.h"

int write_file(char *filename, char *json_data);

int str2md5(const char *input, char *output);


uint64_t get_timestamp();

void getTimeInMillis(char* timestamp_str);

int upload_conn_diag_db(char* filename);

int local_backup_file_output(char *type, char *filename, int filesize, const int backup_file_timestamp);

#ifdef RTCONFIG_AMAS_CENTRAL_CONTROL
void backup_cfg_mnt();
#endif

int mac_toupper(char * mac, char * output);

int delete_sub_str(const char *str, const char *sub_str, char *result_str);
