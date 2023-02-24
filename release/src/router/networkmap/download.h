#ifndef __DOWNLOAD_H__
#define __DOWNLOAD_H__

// #define NMP_DEBUG    1

int cloud_db_process();
int download_cloud_version(const int check_sig, const int file_enc);
int get_nmp_db_from_local_version_file(const int check_sig, const int file_enc, char *version,
	const size_t ver_len, char *checksum, const size_t checksum_len);
int parse_nmp_db(const char *parse_db_path, struct json_object *create_nmp_db_array);

int download_nmp_db(const char *cur_nmp_db_type, const char *cur_nmp_db_ver, const char *cur_nmp_db_checksum, const int check_sig);

int keep_local_nmp_db(const char * db_type, const char * keep_local_db_path,  const char * cloud_db_path);
int merge_local_db_and_cloud_db(const char * cur_nmp_db_type, const char * cloud_db_path);

int nmp_db_cmp(const char* cloud_db_path, char* local_db_keyword, char* local_db_type, char* local_db_os_type);


#endif
