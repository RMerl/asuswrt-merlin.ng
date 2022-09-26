#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <bcmnvram.h>
#include <shared.h>
#include <shutils.h>
#include <curl/curl.h>
#include <openssl/md5.h>
#include <json.h>
#include "tc_utils.h"
#ifdef RTCONFIG_USB
#include <usb_info.h>
#include <disk_io_tools.h>
#include <disk_initial.h>
#include <disk_share.h>
#endif

int md5_filename = 1;
int dst_full_path = 0;

static int tc_restart_flag = 0;
char sessionFile[256] = {0};
struct myprogress {
	double lastruntime;
	CURL *curl;
};

size_t tc_write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
	size_t written = fwrite(ptr, size, nmemb, stream);
	return written;
}

static void replace_char(char *str, char find, char replace) {
	char *p;

	for(p = str; *p != '\0'; p++)
		if(*p == find) *p = replace;
}

void restart_download(int sig){
	dbg("!!!!!receive restart_download signal!!!!!\n");
	TENCENT_DBG("!!!!!receive restart_download signal!!!!!\n");
	tc_restart_flag = 1;
}

int check_process_exist(char *pid_file){
	FILE *fp;
	int result = -1;
	char proc_pid_dir[32] = {0};

	fp= fopen(pid_file, "r");
	if (!fp) {
		return 0;
	}

	fscanf(fp,"%d",&result);
	fclose(fp);

	snprintf(proc_pid_dir, sizeof(proc_pid_dir), "/proc/%d", result);
	if(!check_if_dir_exist(proc_pid_dir)){
		unlink(pid_file);
		return 0;
	}
	else
		return 1;
}

int check_md5_match(char *filePath, int filePath_len, char *md5str){
	int md5match = 1;
	char *buf = NULL;
	FILE *fp = NULL;
	int len = 0, line_len = 0;

	if(md5str != NULL){
		len = filePath_len + 20;
		buf = malloc(len);
		if(buf){
			snprintf(buf, len, "md5sum %s | cut -b-32", filePath);
			TENCENT_DBG("execute md5sum: buf = %s\n", buf);
			fp = popen(buf, "r");
			if(fp){
				memset(buf, 0, len);
				line_len = fread(buf, 1, len, fp);
				pclose(fp);
				buf[line_len-1] = '\0';
				TENCENT_DBG("md5str of downloaded file = %s\n", buf);
				if(strcasecmp(md5str, buf)){
					TENCENT_DBG("md5 mismatch\n");
					md5match = 0;
				}
			}
			else{
				TENCENT_DBG("Fail to calculate md5 of downloaded file %s\n", filePath);
				md5match = 0;
			}
			free(buf);
		}
	}
	else
		TENCENT_DBG("Can't get md5sum from session file.\n");

	return md5match;
}

int str_to_md5(char *str, int str_len, char *md5buf, int buf_len){
	char *buf = NULL;
	FILE *fp = NULL;
	int len = 0, line_len = 0;

	len = str_len + 30;
	buf = malloc(len);
	if(buf){
		snprintf(buf, len, "echo -n %s | md5sum | cut -b-32", str);
		fp = popen(buf, "r");
		if(fp){
			memset(buf, 0, len);
			line_len = fread(buf, 1, len, fp);
			pclose(fp);
			buf[line_len-1] = '\0';
			snprintf(md5buf, buf_len, "%s", buf);
		}
		else{
			dbg("Fail to calculate md5 of string %s\n", str);
		}
		free(buf);
	}

	return 0;
}

int create_full_path(char *path, int path_len){
	char *buf, *buf2;
	char *token;
	char delim[] = "/";
	int depth = 0, buf2_len = 0;

	buf = malloc(path_len);
	snprintf(buf, path_len, "%s", path);
	buf2_len = strlen(buf) + 1;
	buf2 = malloc(buf2_len);
	*buf2 = '\0';

	dbg("create_full_path: folder path = %s(%d)\n", buf, strlen(buf));
	while((token = strsep(&buf, delim)) != NULL){
		if(strlen(token) == 0)
			continue;

		strncat(buf2, "/", buf2_len);
		strncat(buf2, token, buf2_len);
		depth++;
		dbg("buf2 = %s(%d)\n", buf2, depth);
		if(!check_if_dir_exist(buf2))
			mkdir(buf2, 0755);
	}

	free(buf);
	free(buf2);
	return 0;
}

int start_session_download(char *session_file){
	struct json_object *session_info = NULL;
	struct json_object *target= NULL;
	struct json_object *target_state = NULL, *target_id = NULL;
	struct json_object *game_id = NULL, *game_ver = NULL;
	struct json_object *filelist = NULL;
	struct json_object *file = NULL, *src = NULL, *dst = NULL, *md5 = NULL, *size = NULL;
	int lock, target_num = 0, idx = 0, filenum = 0, idx2 = 0;
	char *download_device = nvram_safe_get("tencent_download_device");
	char *tencent_download_path = nvram_safe_get("tencent_download_path");
	char *folderPath1 = NULL, *folderPath2 = NULL, *filePath = NULL, *fileName = NULL;
	size_t len = 0, filePath_len = 0;
	char *dst_string = NULL , *md5str = NULL;
	int file_download_state = 0, target_download_state = 1; //0: failed  1:completed
	int disk_check = 0;
	int64_t file_size = 0;
	TENCENT_DBG("start start_session_download\n");
	tc_restart_flag = 0;

	lock = file_lock(session_file);
	session_info = json_object_from_file(session_file);
	file_unlock(lock);

	if(session_info){
		clean_disk(SESSION_RECORD, session_file);
		target_num =  json_object_array_length(session_info);
		TENCENT_DBG("session_info exists. target_num = %d\n", target_num);
		for(idx = 0; idx < target_num; idx++){
			target_download_state = 1;
			target = json_object_array_get_idx(session_info, idx);
			json_object_object_get_ex(target, "id", &target_id);
			json_object_object_get_ex(target, "state", &target_state);
			json_object_object_get_ex(target, "gameid", &game_id);
			json_object_object_get_ex(target, "ver", &game_ver);
			if(!target_id || !target_state || !game_id){
				TENCENT_DBG("Target information is not enough!\n");
				json_object_put(session_info);
				return -1;
			}

			if(!check_if_dir_exist(tencent_download_path)){
				TENCENT_DBG("create %s\n", tencent_download_path);
				mkdir(tencent_download_path, 0755);
			}

			if(!strcmp(json_object_get_string(target_state), "received") ||
				!strcmp(json_object_get_string(target_state), "going")){
				json_object_object_add(target, "state", json_object_new_string(TC_STATE_GOING));
				update_session_record(SESSION_RECORD, target);
				killall("awsiot", SIGUSR1);

				len = strlen(tencent_download_path) + strlen(json_object_get_string(target_id)) + 2;
				folderPath1 = malloc(len);
				snprintf(folderPath1, len, "%s/%s", tencent_download_path, json_object_get_string(target_id));
				TENCENT_DBG("folderPath1 (%d) = %s\n", len, folderPath1);
				if(!check_if_dir_exist(folderPath1))
					mkdir(folderPath1, 0755);

				len = strlen(folderPath1) + strlen(json_object_get_string(game_id)) + strlen(json_object_get_string(game_ver)) + 3;
				folderPath2 = malloc(len);
				snprintf(folderPath2, len, "%s/%s_%s", folderPath1, json_object_get_string(game_id), json_object_get_string(game_ver));
				TENCENT_DBG("folderPath2 (%d) = %s\n", len, folderPath2);
				if(!check_if_dir_exist(folderPath2))
					mkdir(folderPath2, 0755);

				json_object_object_get_ex(target, "filelist", &filelist);
				disk_check = check_available_space(filelist, download_device);
				if(disk_check == DISK_SPACE_ENOUGH){
					if(!check_if_dir_exist(tencent_download_path)){
						TENCENT_DBG("create %s\n", tencent_download_path);
						mkdir(tencent_download_path, 0755);
					}
					filenum = json_object_array_length(filelist);
					TENCENT_DBG("target id = %s filenum = %d\n", json_object_get_string(target_id), filenum);
					for(idx2 = 0; idx2 < filenum; idx2++){
						file = json_object_array_get_idx(filelist, idx2);
						json_object_object_get_ex(file, "src", &src);
						json_object_object_get_ex(file, "dst", &dst);
						json_object_object_get_ex(file, "md5", &md5);
						json_object_object_get_ex(file, "size", &size);
						dst_string = json_object_get_string(dst);
						if(md5)
							md5str = json_object_get_string(md5);
						TENCENT_DBG("md5 of %s gotten form tencent_session.json = %s\n", dst_string, md5str);
						file_size = json_object_get_int64(size);
						TENCENT_DBG("file size of %s gotten form tencent_session.json = %lld\n", dst_string, file_size);

						if(md5_filename){
						//if(nvram_get_int("md5_filename")){
							char md5_name[64];
							str_to_md5(dst_string, strlen(dst_string), md5_name, sizeof(md5_name));
							fileName = malloc(strlen(md5_name) + 1);
							snprintf(fileName, len, "%s", md5_name);
							json_object_object_add(file, "md5_name", json_object_new_string(fileName));
						}
						else{
							len = strlen(strrchr(dst_string, '/')+1) + 1;
							fileName = malloc(len);
							snprintf(fileName, len, "%s", strrchr(dst_string, '/')+1);
						}
						TENCENT_DBG("fileName (%d) = %s\n", len, fileName);

						if(dst_full_path){
						//if(nvram_get_int("dst_full_path")){
							filePath_len = strlen(folderPath2) + strlen(dst_string) + strlen(fileName) + 3;
							filePath = malloc(filePath_len);
							snprintf(filePath, filePath_len, "%s%s", folderPath2, dst_string);
							*strrchr(filePath, '/') = '\0';
							create_full_path(filePath, filePath_len);
							strncat(filePath, "/", filePath_len);
							strncat(filePath, fileName, filePath_len);
						}
						else{
							filePath_len = strlen(folderPath2) + strlen(fileName) + 2;
							filePath = malloc(filePath_len);
							snprintf(filePath, filePath_len, "%s/%s", folderPath2, fileName);
						}

						free(fileName);
						TENCENT_DBG("filePath (%d) = %s\n", filePath_len, filePath);

						if(!check_if_file_exist(filePath) || (get_file_size(filePath) != file_size) || !check_md5_match(filePath, filePath_len, md5str)){
							file_download_state = tc_curl_download_file(json_object_get_string(src), filePath, 3, 0);
							if(file_download_state == CURLE_OK){
								if(check_md5_match(filePath, filePath_len, md5str)){
									json_object_object_add(file, "rdst", json_object_new_string(filePath));
								}
								else{
									target_download_state = 0;
									json_object_object_add(target, "error_code", json_object_new_int(TC_MD5_MISMATCH));
									json_object_object_add(target, "msg", json_object_new_string("md5 mismatch"));
									unlink(filePath);
								}
							}
							else{
								TENCENT_DBG("Fail to download: %s\n", filePath);
								json_object_object_add(target, "error_code", json_object_new_int(TC_CURL_ERROR));
								json_object_object_add(target, "msg", json_object_new_string(curl_easy_strerror(file_download_state)));
								target_download_state = 0;
							}
						}
						else{
							json_object_object_add(file, "rdst", json_object_new_string(filePath));
						}

						TENCENT_DBG("tc_restart_flag = %d\n", tc_restart_flag);
						free(filePath);
						if(tc_restart_flag){
							TENCENT_DBG("*****break file download for loop *****\n");
							dbg("*****break file download for loop *****\n");
							break;
						}
					}
				}
				else{
					target_download_state = 0;
					if(disk_check == DISK_SPACE_NOT_ENOUGH){
						json_object_object_add(target, "error_code", json_object_new_int(TC_SPACE_NOT_ENOUGH));
						json_object_object_add(target, "msg", json_object_new_string("Free disk space is not enough."));
					}
					else if(disk_check == DISK_READ_INFO_FAIL){
						json_object_object_add(target, "error_code", json_object_new_int(TC_READ_DISKINFO_FAIL));
						json_object_object_add(target, "msg", json_object_new_string("Can't read the information of the disk."));
					}
					else if(disk_check == DISK_PARTITION_NOT_FOUND){
						json_object_object_add(target, "error_code", json_object_new_int(TC_PARTITION_NOT_FOUND));
						json_object_object_add(target, "msg", json_object_new_string("Download device can't be found."));
					}
				}

				if(tc_restart_flag){
					TENCENT_DBG("Stop downloading and restart tc_download.\n");
				}
				else if(!target_download_state){
					json_object_object_add(target, "state", json_object_new_string(TC_STATE_FAILED));
					TENCENT_DBG("target %s download failed\n", json_object_get_string(target_id));
				}
				else
					json_object_object_add(target, "state", json_object_new_string(TC_STATE_COMPLETED));

				free(folderPath1);
				free(folderPath2);
				update_session_record(SESSION_RECORD, target);
				if(!tc_restart_flag){
					killall("awsiot", SIGUSR1);
				}

				if(tc_restart_flag){
					TENCENT_DBG("*****break session for loop *****\n");
					dbg("*****break session for loop *****\n");
					break;
				}
			}
		}

		json_object_put(session_info);
		if(tc_restart_flag){
			TENCENT_DBG("*****receive restart signal to restart download!!*****\n");
			dbg("*****receive restart signal to restart download!!*****\n");
			start_session_download(sessionFile);
		}
	}
	else{
		TENCENT_DBG("Session file '%s' doesn't contain valid JSON object.\n", sessionFile);
		return -1;
	}
}

int tc_download_main(int argc, char *argv[])
{
	FILE *fp = NULL;
	struct json_object *target= NULL;
	struct json_object *target_state = NULL, *target_id = NULL;
	struct json_object *game_id = NULL, *game_ver = NULL;
	struct json_object *filelist = NULL;
	struct json_object *file = NULL, *src = NULL, *dst = NULL, *md5 = NULL;
	int lock, target_num = 0, idx = 0, filenum = 0, idx2 = 0;
	char *tencent_download_path = nvram_safe_get("tencent_download_path");
	char *folderPath1 = NULL, *folderPath2 = NULL, *filePath = NULL, *fileName = NULL;
	size_t len = 0, filePath_len = 0;
	char *dst_string = NULL , *md5str = NULL;
	int file_download_state = 0, target_download_state = 1; //0: failed  1:completed

	if(!check_process_exist(TC_DOWNLOAD_PID_FILE))
	{
		/* Write pid */
		char pid_t[8] = {0};
		snprintf(pid_t, sizeof(pid_t), "%d", getpid());
		f_write_string(TC_DOWNLOAD_PID_FILE, pid_t, 0, 0);
		TENCENT_DBG("start tc_download\n");
	}
	else
	{
		TENCENT_DBG("tc_download is already running.\n");
		return 0;
	}

	TENCENT_DBG("argc=%d, argv[1]=%s\n", argc, argv[1]);
	if(argc == 1){
		snprintf(sessionFile, sizeof(sessionFile), TC_SESSION_FILE);
	}
	else{
		snprintf(sessionFile, sizeof(sessionFile), argv[1]);
	}

	signal(SIGTERM, restart_download);

	TENCENT_DBG("check if %s exists: %d\n", sessionFile, check_if_file_exist(sessionFile));
	if(check_if_file_exist(sessionFile)){
		start_session_download(sessionFile);
	}
	else{
		TENCENT_DBG("'%s' doesn't exist.\n", sessionFile);
		return -1;
	}

	unlink(TC_DOWNLOAD_PID_FILE);
	return 0;
}

/*
	Compare current session file(tencent_session.json) and session record(tencent_session_update.json)
	to remove files which are not needed anymore by target id.
*/
void clean_disk(char *record_file, char *current_session_file){
	struct json_object *record_target_list = NULL, *cur_target_list = NULL;
	struct json_object *record_target = NULL, *cur_target = NULL;
	struct json_object *record_target_id = NULL, *cur_target_id = NULL;
	const char *record_id = NULL, *cur_id = NULL;
	int lock, record_target_num = 0, cur_target_num = 0;
	int idx = 0, idx2 = 0, record_id_found = 0;
	TENCENT_DBG("start to clean disk: record_file: %s current_session_file: %s\n", record_file, current_session_file);
	if(check_if_file_exist(record_file)){
		lock = file_lock(record_file);
		record_target_list = json_object_from_file(record_file);
		file_unlock(lock);
		if(record_target_list)
			record_target_num = json_object_array_length(record_target_list);
		TENCENT_DBG("record_target_num: %d\n", record_target_num);
	}

	if(check_if_file_exist(current_session_file)){
		lock = file_lock(current_session_file);
		cur_target_list = json_object_from_file(current_session_file);
		file_unlock(lock);
		if(cur_target_list)
			cur_target_num = json_object_array_length(cur_target_list);
		TENCENT_DBG("cur_target_num: %d\n", cur_target_num);
	}
	else{
		TENCENT_DBG("session file: %s doesn't exist.\n", current_session_file);
	}

	if(record_target_list && record_target_num > 0 && cur_target_list){
		for(idx = 0; idx < record_target_num; idx++){
			record_id_found = 0;
			record_target = json_object_array_get_idx(record_target_list, idx);
			if(record_target){
				json_object_object_get_ex(record_target, "id", &record_target_id);
				if(record_target_id){
					record_id = json_object_get_string(record_target_id);
					for(idx2 = 0; idx2 < cur_target_num; idx2++){
						cur_target = json_object_array_get_idx(cur_target_list, idx2);
						if(cur_target){
							json_object_object_get_ex(cur_target, "id", &cur_target_id);
							if(cur_target_id){
								cur_id = json_object_get_string(cur_target_id);
								if(!strcmp(record_id, cur_id)){
									record_id_found	= 1;
									break;
								}
							}
						}
					}

					if(!record_id_found && strlen(record_id) > 0){
						char *folderPath;
						size_t len = strlen(nvram_safe_get("tencent_download_path")) + strlen(record_id) + 2;
						folderPath = malloc(len);
						snprintf(folderPath, len, "%s/%s", nvram_safe_get("tencent_download_path"), record_id);
						TENCENT_DBG("remove folder: %s\n", folderPath);
						if(check_if_dir_exist(folderPath)){
							remove_dir(folderPath);
							free(folderPath);
							TENCENT_DBG("Succeed to remove folder.\n");
						}
						else
							TENCENT_DBG("folder doesn't exist!\n");

						/* update record file */
						json_object_object_add(record_target, "state", json_object_new_string(TC_STATE_DELETE));
						update_session_record(SESSION_RECORD, record_target);
					}
				}
			}
		}
	}
	TENCENT_DBG("clean_disk finish\n");
}

void update_session_record(char *record_file, struct json_object *update_target){
	struct json_object *record_target_info = NULL;
	struct json_object *update_target_id = NULL, *update_target_state = NULL;
	struct json_object *target_tmp= NULL, *target_id_tmp = NULL;
	struct json_object *updated_record_info = json_object_new_array();
	struct json_object *new_target= json_tokener_parse(json_object_to_json_string(update_target));
	int lock, target_num = 0, idx = 0;
	const char *update_id_str = NULL;

	json_object_object_get_ex(update_target, "id", &update_target_id);
	json_object_object_get_ex(update_target, "state", &update_target_state);
	update_id_str = json_object_get_string(update_target_id);
	TENCENT_DBG("update record file [%s] target id = %s state = %s\n", record_file, update_id_str, json_object_get_string(update_target_state));

	if(check_if_file_exist(record_file)){
		lock = file_lock(record_file);
		record_target_info = json_object_from_file(record_file);
		file_unlock(lock);
		if(record_target_info)
			target_num = json_object_array_length(record_target_info);
	}

	if(record_target_info && target_num > 0){
		for(idx = 0; idx < target_num; idx++){
			target_tmp = json_object_array_get_idx(record_target_info, idx);
			json_object_object_get_ex(target_tmp, "id", &target_id_tmp);
			if(strcmp(json_object_get_string(target_id_tmp), update_id_str)){
				json_object_array_add(updated_record_info, target_tmp);
			}
		}
	}

	if(strcmp(json_object_get_string(update_target_state), "delete"))
		json_object_array_add(updated_record_info, new_target);

	json_object_to_file(record_file, updated_record_info);
	json_object_put(record_target_info);
	json_object_put(updated_record_info);
}

int check_available_space(struct json_object *filelist, char *download_device){
	int file_num = json_object_array_length(filelist);
	struct json_object *file = NULL, *dst = NULL, *size = NULL;
	int idx = 0, partition_found = 0;
	int64_t file_size = 0, total_size = 0, size_inkbytes = 0, free_size_inkbytes = 0;
	disk_info_t *disks_info, *follow_disk;
	partition_info_t *follow_partition;

	/* calculate required disk size */
	for(idx = 0; idx < file_num; idx++){
		file = json_object_array_get_idx(filelist, idx);
		json_object_object_get_ex(file, "size", &size);
		json_object_object_get_ex(file, "dst", &dst);
		file_size = json_object_get_int64(size);
		total_size = total_size + file_size;
	}
	size_inkbytes = (total_size / 1024) + 1;
	TENCENT_DBG("total size of files = %lld size_inkbytes = %lld\n", total_size, size_inkbytes);

	disks_info = read_disk_data();
	if (disks_info != NULL){
		for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next){
			for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next) {
				if (follow_partition->mount_point == NULL || strlen(follow_partition->mount_point) <= 0)
					continue;
				TENCENT_DBG("follow_partition->mount_point = %s\n", follow_partition->mount_point);
				if(!strcmp(follow_partition->mount_point+9, download_device)){
					TENCENT_DBG("total disk size: %lld Kbytes  used disk size = %lld Kbytes\n", follow_partition->size_in_kilobytes, follow_partition->used_kilobytes);
					free_size_inkbytes = follow_partition->size_in_kilobytes - follow_partition->used_kilobytes;
					free_size_inkbytes = free_size_inkbytes * 0.9;
					partition_found = 1;
					break;
				}
			}

			if(partition_found)
				break;
		}
		free_disk_data(&disks_info);
	}
	else{
		TENCENT_DBG("Can't read the information of the disk.\n");
		return DISK_READ_INFO_FAIL;
	}

	if(partition_found){
		if(size_inkbytes < free_size_inkbytes){
			return DISK_SPACE_ENOUGH;
		}
		else{
			TENCENT_DBG("Free disk space is not enough.\n");
			return DISK_SPACE_NOT_ENOUGH;
		}
	}
	else
		return DISK_PARTITION_NOT_FOUND;


}

int remove_dir(char *dirPath){
	DIR *dir = opendir(dirPath);
	size_t path_len = strlen(dirPath);
	int r = -1;

	TENCENT_DBG("remove directory: %s\n", dirPath);
	if(dir){
		struct dirent *ptr;

		r = 0;
		while (!r && (ptr = readdir(dir)) != NULL) {
			int r2 = -1;
			char *buf;
			size_t len;

			if (!strcmp(ptr->d_name, ".") || !strcmp(ptr->d_name, ".."))
				continue;

			len = path_len + strlen(ptr->d_name) + 2;
			buf = malloc(len);

			if (buf) {
				struct stat statbuf;

				snprintf(buf, len, "%s/%s", dirPath, ptr->d_name);
				if (!stat(buf, &statbuf)) {
					if (S_ISDIR(statbuf.st_mode))
						r2 = remove_dir(buf);
					else
						r2 = unlink(buf);
				}
				free(buf);
			}
			r = r2;
		}
		closedir(dir);

		if (!r)
			r = rmdir(dirPath);
	}
	TENCENT_DBG("r = %d\n", r);
	return r;
}

long long get_file_size(char *file_path){
	struct stat buf;
	long long file_size = 0;

	if(stat(file_path, &buf) == 0)
		file_size = buf.st_size;
	TENCENT_DBG("file_path = %s\n", file_path);
	TENCENT_DBG("file size = %lld\n", file_size);
	return file_size;
}

static int xferinfo(void *p,
					double dltotal, double dlnow,
					double ultotal, double ulnow)
{
	struct myprogress *myp = (struct myprogress *)p;
	CURL *curl = myp->curl;
	double curtime = 0;
	double fractiondownloaded = dlnow / dltotal;
	double percentage = round(fractiondownloaded*100);

	curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &curtime);

	if((curtime - myp->lastruntime) >= MINIMAL_PROGRESS_FUNCTIONALITY_INTERVAL) {
		myp->lastruntime = curtime;
		//dbg("TOTAL TIME: %f \r\n", curtime);
	}

	if(tc_restart_flag){
		TENCENT_DBG("Percentage of downloaded: %f %\n", percentage);
		return 1;
	}

	return 0;
}

int tc_curl_download_file(char *url, char *file_path, int retry, int check_CA)
{
	CURL *curl;
	FILE *fp;
	CURLcode res = CURLE_OK;
	struct curl_httppost *post = NULL;
	struct myprogress prog;

	TENCENT_DBG("url = %s\n", url);
	TENCENT_DBG("file_path = %s\n", file_path);

	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	if (curl) {
		prog.lastruntime = 0;
		prog.curl = curl;
		if((fp = fopen(file_path,"a+b"))
		 != NULL){
			if(post != NULL)
				curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);

			/* enable verbose for easier tracing */
			curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
			curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
			curl_easy_setopt(curl, CURLOPT_URL, url);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, tc_write_data);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
			curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);
			curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
			//curl_easy_setopt(curl, CURLOPT_TIMEOUT, 1800L);
			curl_easy_setopt(curl, CURLOPT_RESUME_FROM_LARGE, get_file_size(file_path));
			curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, xferinfo);
			curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &prog);
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
			res = curl_easy_perform(curl);
			/* always cleanup */
			fclose(fp);
		}

		while(retry > 0 && res != CURLE_OK){
			sleep(1);
			retry--;
			TENCENT_DBG("curl_easy_perform() failed: (%d)%s\n", res, curl_easy_strerror(res));
			res = curl_easy_perform(curl);
		}

		/* Get the curl infomation */
		{
			curl_off_t info;
			CURLcode res_info = CURLE_OK;
			res_info = curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD_T, &info);
			if(!res_info) {
				TENCENT_DBG("Downloaded %" CURL_FORMAT_CURL_OFF_T " bytes\n", info);
				dbg("Downloaded %" CURL_FORMAT_CURL_OFF_T " bytes\n", info);
			}

			res_info = curl_easy_getinfo(curl, CURLINFO_SPEED_DOWNLOAD_T, &info);
			if(!res_info) {
				TENCENT_DBG("Download speed %" CURL_FORMAT_CURL_OFF_T " bytes/sec\n", info);
				dbg("Download speed %" CURL_FORMAT_CURL_OFF_T " bytes/sec\n", info);
			}
		}

		/* always cleanup */
		if(post != NULL)
			curl_formfree(post);

		curl_easy_cleanup(curl);
	}

	curl_global_cleanup();
	TENCENT_DBG("file size = %lld\n", get_file_size(file_path));
	return res;
}