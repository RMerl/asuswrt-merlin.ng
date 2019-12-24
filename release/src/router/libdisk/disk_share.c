/*
 *  * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/stat.h>	// for mkdir()
#include <sys/types.h>	// for mkdir()
#include <unistd.h>	// for sleep()
#include <bcmnvram.h>
#include <shutils.h>
#include <shared.h>

#include "usb_info.h"
#include "disk_initial.h"
#include "disk_share.h"

#ifdef RTCONFIG_PERMISSION_MANAGEMENT
#include <PMS_DBAPIs.h>
// PMS_GetAccountInfo(PMS_ACTION_GET_FULL, &account_list, &group_list, &acc_num, &group_num);
// account_list/group_list's name were ascii type.
#else
// get_account_list's name were char type.
#endif


extern void set_file_integrity(const char *const file_name){
	unsigned long file_size;
	char test_file[PATH_MAX], test_file_name[PATH_MAX];
	FILE *fp;
	char target_dir[PATH_MAX], *ptr;
	int len;
	DIR *opened_dir;
	struct dirent *dp;

	if((ptr = strrchr(file_name, '/')) == NULL){
		usb_dbg("Fail to get the target_dir of the file.\n");
		return;
	}
	len = strlen(file_name)-strlen(ptr);
	memset(target_dir, 0, sizeof(target_dir));
	strncpy(target_dir, file_name, len);

	if((file_size = f_size(file_name)) == -1){
		usb_dbg("Fail to get the size of the file.\n");
		return;
	}

	snprintf(test_file, sizeof(test_file), "%s.%lu", file_name, file_size);
	if((fp = fopen(test_file, "w")) != NULL)
		fclose(fp);

	++ptr;
	snprintf(test_file_name, sizeof(test_file_name), "%s.%lu", ptr, file_size);

	if((opened_dir = opendir(target_dir)) == NULL){
		usb_dbg("Can't opendir \"%s\".\n", target_dir);
		return;
	}

	len = strlen(ptr);
	while((dp = readdir(opened_dir)) != NULL){
		char test_path[PATH_MAX];

		if(!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
			continue;

		if(strncmp(dp->d_name, ptr, len) || !strcmp(dp->d_name, ptr) || !strcmp(dp->d_name, test_file_name))
			continue;

		snprintf(test_path, sizeof(test_path), "%s/%s", target_dir, dp->d_name);
		usb_dbg("delete %s.\n", test_path);
		delete_file_or_dir(test_path);
	}
	closedir(opened_dir);
}

extern int check_file_integrity(const char *const file_name){
	unsigned long file_size;
	char test_file[PATH_MAX];

	if((file_size = f_size(file_name)) == -1){
		usb_dbg("Fail to get the size of the file.\n");
		return 0;
	}

	snprintf(test_file, sizeof(test_file), "%s.%lu", file_name, file_size);
	if(!check_if_file_exist(test_file)){
		usb_dbg("Fail to check the folder list.\n");
		return 0;
	}

	return 1;
}

extern int get_folder_list(const char *const mount_path, int *sh_num, char ***folder_list){
	char **tmp_folder_list, target[16];
	int len, i;
	char *list_file, *list_info;
	char *follow_info, *follow_info_end, backup;
	
	// 1. get list file
	len = strlen(mount_path)+strlen("/.__folder_list.txt");
	list_file = (char *)malloc(sizeof(char)*(len+1));
	if(list_file == NULL){
		usb_dbg("Can't malloc \"list_file\".\n");
		return -1;
	}
	snprintf(list_file, (len+1), "%s/.__folder_list.txt", mount_path);
	list_file[len] = 0;
	
	// 2. check the file integrity.
	if(!check_file_integrity(list_file)){
		usb_dbg("Fail to check the folder list.\n");
		if(initial_folder_list(mount_path) != 0){
			usb_dbg("Can't initial the folder list.\n");
			free(list_file);
			return -1;
		}
	}
	
	// 3. read if the list file is existed
	if(!check_if_file_exist(list_file)){
#if 1
		initial_folder_list(mount_path);
#else
		usb_dbg("No file: %s.\n", list_file);
		free(list_file);
		return -1;
#endif
	}
	
	list_info = read_whole_file(list_file);
	if(list_info == NULL){
		usb_dbg("No content in %s.\n", list_file);
		free(list_file);
		return -1;
	}
	
	// 4. find sh_num
	follow_info = strstr(list_info, "sh_num=");
	if(follow_info == NULL){
		usb_dbg("No sh_num in %s is wrong.\n", list_file);
		free(list_file);
		free(list_info);
		return -1;
	}
	
	follow_info += strlen("sh_num=");
	follow_info_end = follow_info;
	while(*follow_info_end != 0 && *follow_info_end != '\n')
		++follow_info_end;
	if(*follow_info_end == 0){
		usb_dbg("The content in %s is wrong.\n", list_file);
		free(list_file);
		free(list_info);
		return -1;
	}
	backup = *follow_info_end;
	*follow_info_end = 0;
	
	*sh_num = atoi(follow_info);
	*follow_info_end = backup;
	
	free(list_file);

	if(*sh_num <= 0){
		usb_dbg("There is no folder in %s.\n", mount_path);
		free(list_info);
		return 0;
	}
	
	// 5. get folder list from the folder list file
	tmp_folder_list = (char **)malloc(sizeof(char *)*((*sh_num)+1));
	if(tmp_folder_list == NULL){
		usb_dbg("Can't malloc \"tmp_folder_list\".\n");
		free(list_info);
		return -1;
	}
	
	for(i = 0; i < *sh_num; ++i){
		// 6. get folder name
		snprintf(target, sizeof(target), "\nsh_name%d=", i);
		follow_info = strstr(list_info, target);
		if(follow_info == NULL){
			usb_dbg("The list content in %s is wrong.\n", mount_path);
			free(list_info);
			free_2_dimension_list(sh_num, &tmp_folder_list);
			return -1;
		}
		
		follow_info += strlen(target);
		follow_info_end = follow_info;
		while(*follow_info_end != 0 && *follow_info_end != '\n')
			++follow_info_end;
		if(*follow_info_end == 0){
			usb_dbg("The list content in %s is wrong.\n", mount_path);
			free(list_info);
			free_2_dimension_list(sh_num, &tmp_folder_list);
			return -1;
		}
		backup = *follow_info_end;
		*follow_info_end = 0;
		
		len = strlen(follow_info);
		tmp_folder_list[i] = (char *)malloc(sizeof(char)*(len+1));
		if(tmp_folder_list == NULL){
			usb_dbg("Can't malloc \"tmp_folder_list\".\n");
			*follow_info_end = backup;
			free(list_info);
			free_2_dimension_list(sh_num, &tmp_folder_list);
			return -1;
		}
		strcpy(tmp_folder_list[i], follow_info);
		tmp_folder_list[i][len] = 0;
		
		*follow_info_end = backup;
	}
	free(list_info);

	*folder_list = tmp_folder_list;
	
	return *sh_num;
}

extern int get_all_folder(const char *const mount_path, int *sh_num, char ***folder_list){
	DIR *pool_to_open;
	struct dirent *dp;
	char *testdir;
	char **tmp_folder_list = NULL, **tmp_folder;
	int len, i;
	
	pool_to_open = opendir(mount_path);
	if(pool_to_open == NULL){
		usb_dbg("Can't opendir \"%s\".\n", mount_path);
		return -1;
	}
	
	*sh_num = 0;
	while((dp = readdir(pool_to_open)) != NULL){
		if(dp->d_name[0] == '.')
			continue;
		
		if(test_if_System_folder(dp->d_name) == 1)
			continue;
		
		len = strlen(mount_path)+strlen("/")+strlen(dp->d_name);
		testdir = (char *)malloc(sizeof(char)*(len+1));
		if(testdir == NULL){
			closedir(pool_to_open);
			return -1;
		}
		snprintf(testdir, (len+1), "%s/%s", mount_path, dp->d_name);
		testdir[len] = 0;
		if(!check_if_dir_exist(testdir)){
			free(testdir);
			continue;
		}
		free(testdir);
		
		tmp_folder = (char **)malloc(sizeof(char *)*(*sh_num+1));
		if(tmp_folder == NULL){
			usb_dbg("Can't malloc \"tmp_folder\".\n");
			closedir(pool_to_open);
			return -1;
		}
		
		len = strlen(dp->d_name);
		tmp_folder[*sh_num] = (char *)malloc(sizeof(char)*(len+1));
		if(tmp_folder[*sh_num] == NULL){
			usb_dbg("Can't malloc \"tmp_folder[%d]\".\n", *sh_num);
			closedir(pool_to_open);
			free(tmp_folder);
			return -1;
		}
		strcpy(tmp_folder[*sh_num], dp->d_name);
		if(*sh_num != 0){
			for(i = 0; i < *sh_num; ++i)
				tmp_folder[i] = tmp_folder_list[i];

			free(tmp_folder_list);
			tmp_folder_list = tmp_folder;
		}
		else
			tmp_folder_list = tmp_folder;
		
		++(*sh_num);
	}
	closedir(pool_to_open);
	
	*folder_list = tmp_folder_list;
	
	return 0;
}

extern int get_var_file_name(const char *const account, const char *const path, char **file_name, const int is_group){
	int len;
	char *var_file;
	char ascii_user[64];

	if(path == NULL)
		return -1;

	len = strlen(path)+strlen("/.___var.txt");
#if defined(RTCONFIG_PERMISSION_MANAGEMENT)
	if(is_group)
		len += 2;
#endif

	if(account != NULL){
		memset(ascii_user, 0, sizeof(ascii_user));
		char_to_ascii_safe(ascii_user, account, sizeof(ascii_user));

		len += strlen(ascii_user);
	}
	*file_name = (char *)malloc(sizeof(char)*(len+1));
	if(*file_name == NULL)
		return -1;

	var_file = *file_name;
	if(account != NULL){
#if defined(RTCONFIG_PERMISSION_MANAGEMENT)
		if(is_group)
			snprintf(var_file, (len+1), "%s/.__G_%s_var.txt", path, ascii_user);
		else
#endif
			snprintf(var_file, (len+1), "%s/.__%s_var.txt", path, ascii_user);
	}
	else
		snprintf(var_file, (len+1), "%s/.___var.txt", path);
	var_file[len] = 0;

	return 0;
}

extern void free_2_dimension_list(int *num, char ***list){
	int i;
	char **target = *list;
	
	if(*num <= 0 || target == NULL){
		*num = 0;
		return;
	}
	
	for(i = 0; i < *num; ++i)
		if(target[i] != NULL)
			free(target[i]);
	
	if(target != NULL)
		free(target);
	
	*num = 0;
}

extern int initial_folder_list(const char *const mount_path){
	int sh_num;
	char **folder_list;
	FILE *fp;
	char *list_file;
	int result, len, i;
	
	if(mount_path == NULL || strlen(mount_path) <= 0){
		usb_dbg("No input, mount_path\n");
		return -1;
	}
	
	// 1. get the list_file
	len = strlen(mount_path)+strlen("/.__folder_list.txt");
	list_file = (char *)malloc(sizeof(char)*(len+1));
	if(list_file == NULL){
		usb_dbg("Can't malloc \"list_file\".\n");
		return -1;
	}
	snprintf(list_file, (len+1), "%s/.__folder_list.txt", mount_path);
	list_file[len] = 0;
	
	// 2. get the folder number and folder_list
	result = get_all_folder(mount_path, &sh_num, &folder_list);
	if(result != 0){
		usb_dbg("Can't get the folder list in \"%s\".\n", mount_path);
		free_2_dimension_list(&sh_num, &folder_list);
		free(list_file);
		return -1;
	}
	
	// 3. write the folder info
	fp = fopen(list_file, "w");
	if(fp == NULL){
		usb_dbg("Can't create folder_list, \"%s\".\n", list_file);
		free_2_dimension_list(&sh_num, &folder_list);
		free(list_file);
		return -1;
	}
	
	fprintf(fp, "sh_num=%d\n", sh_num);
	for(i = 0; i < sh_num; ++i)
		fprintf(fp, "sh_name%d=%s\n", i, folder_list[i]);
	fclose(fp);
	free_2_dimension_list(&sh_num, &folder_list);
	
	// 4. set the check target of file.
	set_file_integrity(list_file);
	free(list_file);
	
	return 0;
}

extern int initial_var_file(const char *const account, const char *const mount_path, const int is_group) {
	FILE *fp;
	char *var_file;
	int i;
	int sh_num;
	char **folder_list;
	int samba_right, ftp_right, dms_right;
#ifdef RTCONFIG_WEBDAV_OLD
	int webdav_right;
#endif

	if (mount_path == NULL || strlen(mount_path) <= 0) {
		usb_dbg("No input, mount_path\n");
		return -1;
	}

	// 1. get the folder number and folder_list
	//get_folder_list(mount_path, &sh_num, &folder_list);
	get_all_folder(mount_path, &sh_num, &folder_list);

	// 2. get the var file
	if(get_var_file_name(account, mount_path, &var_file, 0)){
		usb_dbg("Can't malloc \"var_file\".\n");
		free_2_dimension_list(&sh_num, &folder_list);
		return -1;
	}

	// 3. get the default permission of all protocol.
	if(account == NULL // share mode.
			|| !strcmp(account, nvram_safe_get("http_username"))){
		samba_right = DEFAULT_SAMBA_RIGHT;
		ftp_right = DEFAULT_FTP_RIGHT;
		dms_right = DEFAULT_DMS_RIGHT;
#ifdef RTCONFIG_WEBDAV_OLD
		webdav_right = DEFAULT_WEBDAV_RIGHT;
#endif
	}
#if defined(RTCONFIG_PERMISSION_MANAGEMENT)
	else if(is_group){
		samba_right = DEFAULT_SAMBA_RIGHT;
		ftp_right = DEFAULT_FTP_RIGHT;
		dms_right = DEFAULT_DMS_RIGHT;
#ifdef RTCONFIG_WEBDAV_OLD
		webdav_right = DEFAULT_WEBDAV_RIGHT;
#endif
	}
#endif
	else{
		samba_right = 0;
		ftp_right = 0;
		dms_right = 0;
#ifdef RTCONFIG_WEBDAV_OLD
		webdav_right = 0;
#endif
	}

	// 4. write the default content in the var file
	if ((fp = fopen(var_file, "w")) == NULL) {
		usb_dbg("Can't create the var file, \"%s\".\n", var_file);
		free_2_dimension_list(&sh_num, &folder_list);
		free(var_file);
		return -1;
	}

	for (i = -1; i < sh_num; ++i) {
		fprintf(fp, "*");
		
		if(i != -1)
			fprintf(fp, "%s", folder_list[i]);
#ifdef RTCONFIG_WEBDAV_OLD
		fprintf(fp, "=%d%d%d%d\n", samba_right, ftp_right, dms_right, webdav_right);
#else
		fprintf(fp, "=%d%d%d\n", samba_right, ftp_right, dms_right);
#endif
	}

	fclose(fp);
	free_2_dimension_list(&sh_num, &folder_list);

	// 5. set the check target of file.
	set_file_integrity(var_file);
	free(var_file);

	return 0;
}

#if defined(RTCONFIG_PERMISSION_MANAGEMENT)
int initial_all_var_file(const char *const mount_path)
{
	int result;
	int acc_num;
	PMS_ACCOUNT_INFO_T *account_list, *follow_account;
	int group_num;
	PMS_ACCOUNT_GROUP_INFO_T *group_list, *follow_group;
	char char_user[64];
	DIR *opened_dir;
	struct dirent *dp;

	if (mount_path == NULL || strlen(mount_path) <= 0) {
		usb_dbg("No input, mount_path\n");
		return -1;
	}
	// 1. delete all var files
	if ((opened_dir = opendir(mount_path)) == NULL) {
		usb_dbg("Can't opendir \"%s\".\n", mount_path);
		return -1;
	}

	while ((dp = readdir(opened_dir)) != NULL) {
		char test_path[PATH_MAX];

		if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
			continue;

		if (strncmp(dp->d_name, ".__", 3))
			continue;

		snprintf(test_path, sizeof(test_path), "%s/%s", mount_path, dp->d_name);
		usb_dbg("delete %s.\n", test_path);
		delete_file_or_dir(test_path);
	}
	closedir(opened_dir);

	// 2. initial the var file
	if (initial_var_file(NULL, mount_path, 0) != 0)	// share mode.
		usb_dbg("Can't initial the var file for the share mode.\n");

	// 3. get the account number and account_list
	PMS_GetAccountInfo(PMS_ACTION_GET_FULL, &account_list, &group_list,
			   &acc_num, &group_num);

	for (follow_group = group_list; follow_group != NULL;
	     follow_group = follow_group->next) {
		memset(char_user, 0, sizeof(char_user));
		ascii_to_char_safe(char_user, follow_group->name,
				   sizeof(char_user));

		if (initial_var_file(char_user, mount_path, 1) != 0)
			usb_dbg("Can't initial \"%s\"'s file in %s.\n",
				follow_group->name, mount_path);
	}

	for (follow_account = account_list; follow_account != NULL;
	     follow_account = follow_account->next) {
		memset(char_user, 0, sizeof(char_user));
		ascii_to_char_safe(char_user, follow_account->name,
				   sizeof(char_user));

		if (initial_var_file(char_user, mount_path, 0) != 0)
			usb_dbg("Can't initial \"%s\"'s file in %s.\n",
				follow_account->name, mount_path);
	}

	PMS_FreeAccInfo(&account_list, &group_list);

	// 4. initial the folder list
	result = initial_folder_list(mount_path);
	if (result != 0)
		usb_dbg("Can't initial the folder list.\n");

	return 0;
}
#else	/* !RTCONFIG_PERMISSION_MANAGEMENT */
extern int initial_all_var_file(const char *const mount_path) {
	int i, result;
	int acc_num;
	char **account_list;
	DIR *opened_dir;
	struct dirent *dp;

	if (mount_path == NULL || strlen(mount_path) <= 0) {
		usb_dbg("No input, mount_path\n");
		return -1;
	}

	// 1. get the account number and account_list
	result = get_account_list(&acc_num, &account_list);

	// 2. delete all var files
	if((opened_dir = opendir(mount_path)) == NULL){
		usb_dbg("Can't opendir \"%s\".\n", mount_path);
		return -1;
	}

	while((dp = readdir(opened_dir)) != NULL){
		char test_path[PATH_MAX];

		if(!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
			continue;

		if(strncmp(dp->d_name, ".__", 3))
			continue;

		snprintf(test_path, sizeof(test_path), "%s/%s", mount_path, dp->d_name);
		usb_dbg("delete %s.\n", test_path);
		delete_file_or_dir(test_path);
	}
	closedir(opened_dir);

	// 3. initial the var file
	if(initial_var_file(NULL, mount_path, 0) != 0) // share mode.
		usb_dbg("Can't initial the var file for the share mode.\n");

	for(i = 0; i < acc_num; ++i){
		if(initial_var_file(account_list[i], mount_path, 0) != 0)
			usb_dbg("Can't initial \"%s\"'s file in %s.\n", account_list[i], mount_path);
	}
	free_2_dimension_list(&acc_num, &account_list);

	// 4. initial the folder list
	result = initial_folder_list(mount_path);
	if(result != 0)
		usb_dbg("Can't initial the folder list.\n");

	return 0;
}
#endif

extern int test_of_var_files(const char *const mount_path){
	create_if_no_var_files(mount_path);	// According to the old folder_list, add the new folder.
	initial_folder_list(mount_path);	// get the new folder_list.
	create_if_no_var_files(mount_path);	// According to the new folder_list, add the new var file.
	
	return 0;
}

#if defined(RTCONFIG_PERMISSION_MANAGEMENT)
int create_if_no_var_files(const char *const mount_path)
{
	int result;
	char *var_file;
	int acc_num;
	PMS_ACCOUNT_INFO_T *account_list, *follow_account;
	int group_num;
	PMS_ACCOUNT_GROUP_INFO_T *group_list, *follow_group;
	char char_user[64];

	// 1. get the var_file for the share mode.
	if (get_var_file_name(NULL, mount_path, &var_file, 0)) {	// share mode.
		usb_dbg("Can't malloc \"var_file\".\n");
		return -1;
	}
	// 2. test if the var_file is existed and check the file integrity.
	if (!check_if_file_exist(var_file)) {
		// 3.1. create the var_file when it's not existed
		if (initial_var_file(NULL, mount_path, 0) != 0) {
			usb_dbg
			    ("Can't initial the var file for the share mode.\n");
			free(var_file);
			return -1;
		}
	} else if (!check_file_integrity(var_file)) {
		// 3.2. check the file integrity.
		usb_dbg("Fail to check the file: %s.\n", var_file);
		if (initial_var_file(NULL, mount_path, 0) != 0) {
			usb_dbg
			    ("Can't initial the var file for the share mode.\n");
			free(var_file);
			return -1;
		}
	} else {
		// 3.3. add the new folder into the var file
		result = modify_if_exist_new_folder(NULL, mount_path, 0);
		if (result != 0)
			usb_dbg("Can't check if there's new folder in %s.\n",
				mount_path);
	}
	free(var_file);

	// 4. get the account number and account_list
	PMS_GetAccountInfo(PMS_ACTION_GET_FULL, &account_list, &group_list,
			   &acc_num, &group_num);

	for (follow_group = group_list; follow_group != NULL;
	     follow_group = follow_group->next) {
		memset(char_user, 0, sizeof(char_user));
		ascii_to_char_safe(char_user, follow_group->name,
				   sizeof(char_user));

		if (get_var_file_name(char_user, mount_path, &var_file, 1)) {
			usb_dbg("Can't malloc \"var_file\".\n");
			PMS_FreeAccInfo(&account_list, &group_list);
			return -1;
		}

		if (!check_if_file_exist(var_file)) {
			if (initial_var_file(char_user, mount_path, 1) != 0) {
				usb_dbg("Can't initial \"%s\"'s file in %s.\n",
					char_user, mount_path);
				PMS_FreeAccInfo(&account_list, &group_list);
				free(var_file);
				return -1;
			}
		} else if (!check_file_integrity(var_file)) {
			// 3.2. check the file integrity.
			usb_dbg("Fail to check the file: %s.\n", var_file);
			if (initial_var_file(char_user, mount_path, 1) != 0) {
				usb_dbg("Can't initial \"%s\"'s file in %s.\n",
					char_user, mount_path);
				PMS_FreeAccInfo(&account_list, &group_list);
				free(var_file);
				return -1;
			}
		} else {
			result =
			    modify_if_exist_new_folder(char_user, mount_path,
						       1);
			if (result != 0)
				usb_dbg
				    ("Can't check if there's new folder in %s.\n",
				     mount_path);
		}
		free(var_file);
	}

	for (follow_account = account_list; follow_account != NULL;
	     follow_account = follow_account->next) {
		memset(char_user, 0, sizeof(char_user));
		ascii_to_char_safe(char_user, follow_account->name,
				   sizeof(char_user));

		if (get_var_file_name(char_user, mount_path, &var_file, 0)) {
			usb_dbg("Can't malloc \"var_file\".\n");
			PMS_FreeAccInfo(&account_list, &group_list);
			return -1;
		}

		if (!check_if_file_exist(var_file)) {
			if (initial_var_file(char_user, mount_path, 0) != 0) {
				usb_dbg("Can't initial \"%s\"'s file in %s.\n",
					char_user, mount_path);
				PMS_FreeAccInfo(&account_list, &group_list);
				free(var_file);
				return -1;
			}
		} else if (!check_file_integrity(var_file)) {
			// 3.2. check the file integrity.
			usb_dbg("Fail to check the file: %s.\n", var_file);
			if (initial_var_file(char_user, mount_path, 0) != 0) {
				usb_dbg("Can't initial \"%s\"'s file in %s.\n",
					char_user, mount_path);
				PMS_FreeAccInfo(&account_list, &group_list);
				free(var_file);
				return -1;
			}
		} else {
			result =
			    modify_if_exist_new_folder(char_user, mount_path,
						       0);
			if (result != 0)
				usb_dbg
				    ("Can't check if there's new folder in %s.\n",
				     mount_path);
		}
		free(var_file);
	}

	PMS_FreeAccInfo(&account_list, &group_list);

	return 0;
}
#else	/* !RTCONFIG_PERMISSION_MANAGEMENT */
extern int create_if_no_var_files(const char *const mount_path) {
	int acc_num;
	char **account_list;
	int result, i;
	char *var_file;
	
	// 1. get the var_file for the share mode.
	if(get_var_file_name(NULL, mount_path, &var_file, 0)){ // share mode.
		usb_dbg("Can't malloc \"var_file\".\n");
		return -1;
	}
	
	// 2. test if the var_file is existed and check the file integrity.
	if(!check_if_file_exist(var_file)){
		// 3.1. create the var_file when it's not existed
		if (initial_var_file(NULL, mount_path, 0) != 0)
			usb_dbg("Can't initial the var file for the share mode.\n");
	}
	else if(!check_file_integrity(var_file)){
		// 3.2. check the file integrity.
		usb_dbg("Fail to check the file: %s.\n", var_file);
		if(initial_var_file(NULL, mount_path, 0) != 0){
			usb_dbg("Can't initial the var file for the share mode.\n");
			free(var_file);
			return -1;
		}
	}
	else{
		// 3.3. add the new folder into the var file
		result = modify_if_exist_new_folder(NULL, mount_path, 0);
		if (result != 0)
			usb_dbg("Can't check if there's new folder in %s.\n", mount_path);
	}
	free(var_file);
	
	// 4. get the account number and account_list
	result = get_account_list(&acc_num, &account_list);
	
	// 5. get the var_file of all accounts.
	for(i = 0; i < acc_num; ++i){
		if(get_var_file_name(account_list[i], mount_path, &var_file, 0)){
			usb_dbg("Can't malloc \"var_file\".\n");
			free_2_dimension_list(&acc_num, &account_list);
			return -1;
		}
		
		if(!check_if_file_exist(var_file)){
			if (initial_var_file(account_list[i], mount_path, 0) != 0)
				usb_dbg("Can't initial \"%s\"'s file in %s.\n", account_list[i], mount_path);
		}
		else if(!check_file_integrity(var_file)){
			// 3.2. check the file integrity.
			usb_dbg("Fail to check the file: %s.\n", var_file);
			if (initial_var_file(account_list[i], mount_path, 0) != 0){
				usb_dbg("Can't initial \"%s\"'s file in %s.\n", account_list[i], mount_path);
				free_2_dimension_list(&acc_num, &account_list);
				free(var_file);
				return -1;
			}
		}
		else{
			result = modify_if_exist_new_folder(account_list[i], mount_path, 0);
			if (result != 0)
				usb_dbg("Can't check if there's new folder in %s.\n", mount_path);
		}
		free(var_file);
	}
	free_2_dimension_list(&acc_num, &account_list);
	
	return 0;
}
#endif

extern int modify_if_exist_new_folder(const char *const account, const char *const mount_path, const int is_group) {
	int sh_num;
	char **folder_list, *target;
	int result, i, len;
	char *var_file;
	FILE *fp;
	int samba_right, ftp_right, dms_right;
#ifdef RTCONFIG_WEBDAV_OLD
	int webdav_right;
#endif
	
	// 1. get the var file
	if(get_var_file_name(account, mount_path, &var_file, 0)){
		usb_dbg("Can't malloc \"var_file\".\n");
		return -1;
	}
	
	// 2. check the file integrity.
	if(!check_file_integrity(var_file)){
		usb_dbg("Fail to check the file: %s.\n", var_file);
		if(initial_var_file(account, mount_path, 0) != 0)
			usb_dbg("Can't initial \"%s\"'s file in %s.\n", account, mount_path);

		free(var_file);
		return -1;
	}
	
	// 3. get all folder in mount_path
	result = get_all_folder(mount_path, &sh_num, &folder_list);
	if (result != 0) {
		usb_dbg("Can't get the folder list in \"%s\".\n", mount_path);
		free_2_dimension_list(&sh_num, &folder_list);
		free(var_file);
		return -1;
	}
	
	for (i = 0; i < sh_num; ++i) {
		result = test_if_exist_share(mount_path, folder_list[i]);
		if(result)
			continue;
		
		// 4. get the target
		len = strlen("*")+strlen(folder_list[i])+strlen("=");
		target = (char *)malloc(sizeof(char)*(len+1));
		if (target == NULL) {
			usb_dbg("Can't allocate \"target\".\n");
			free_2_dimension_list(&sh_num, &folder_list);
			free(var_file);
			return -1;
		}
		snprintf(target, (len+1), "*%s=", folder_list[i]);
		target[len] = 0;
		
		// 5. get the default permission of all protocol.
		if(account == NULL // share mode.
				|| !strcmp(account, nvram_safe_get("http_username"))){
			samba_right = DEFAULT_SAMBA_RIGHT;
			ftp_right = DEFAULT_FTP_RIGHT;
			dms_right = DEFAULT_DMS_RIGHT;
#ifdef RTCONFIG_WEBDAV_OLD
			webdav_right = DEFAULT_WEBDAV_RIGHT;
#endif
		}
#if defined(RTCONFIG_PERMISSION_MANAGEMENT)
		else if (is_group) {
			samba_right = DEFAULT_SAMBA_RIGHT;
			ftp_right = DEFAULT_FTP_RIGHT;
			dms_right = DEFAULT_DMS_RIGHT;
#ifdef RTCONFIG_WEBDAV_OLD
			webdav_right = DEFAULT_WEBDAV_RIGHT;
#endif
		}
#endif
		else{
			samba_right = 0;
			ftp_right = 0;
			dms_right = 0;
#ifdef RTCONFIG_WEBDAV_OLD
			webdav_right = 0;
#endif
		}
		
		// 6. add the information of the new folder
		fp = fopen(var_file, "a+");
		if (fp == NULL) {
			usb_dbg("Can't write \"%s\".\n", var_file);
			free_2_dimension_list(&sh_num, &folder_list);
			free(var_file);
			free(target);
			return -1;
		}
		
#ifdef RTCONFIG_WEBDAV_OLD
		fprintf(fp, "%s%d%d%d%d\n", target, samba_right, ftp_right, dms_right, webdav_right);
#else
		fprintf(fp, "%s%d%d%d\n", target, samba_right, ftp_right, dms_right);
#endif

		free(target);
		fclose(fp);
	}
	free_2_dimension_list(&sh_num, &folder_list);
	
	// 7. set the check target of file.
	set_file_integrity(var_file);
	free(var_file);
	
	return 0;
}

#if defined(RTCONFIG_PERMISSION_MANAGEMENT)
int get_permission(const char *const account,
		   const char *const mount_path,
		   const char *const folder,
		   const char *const protocol, const int is_group)
{
	char *var_file, *var_info;
	char *target, *follow_info;
	int len, result;
	char *f = (char *)folder;

	// 1. get the var file
	if (get_var_file_name(account, mount_path, &var_file, is_group)) {
		usb_dbg("Can't malloc \"var_file\".\n");
		return -1;
	}
	// 2. check the file integrity.
	if (!check_file_integrity(var_file)) {
		usb_dbg("Fail to check the file: %s.\n", var_file);
		if (initial_var_file(account, mount_path, is_group) != 0) {
			usb_dbg("Can't initial \"%s\"'s file in %s.\n", account,
				mount_path);
			free(var_file);
			return -1;
		}
	}
	// 3. get the content of the var_file of the account
	var_info = read_whole_file(var_file);
	if (var_info == NULL) {
		usb_dbg
		    ("get_permission: \"%s\" isn't existed or there's no content.\n",
		     var_file);
		free(var_file);
		return -1;
	}
	free(var_file);

	// 4. get the target in the content
      retry_get_permission:
	if (f == NULL)
		len = strlen("*=");
	else
		len = strlen("*") + strlen(f) + strlen("=");
	target = (char *)malloc(sizeof(char)*(len+1));
	if (target == NULL) {
		usb_dbg("Can't allocate \"target\".\n");
		free(var_info);
		return -1;
	}
	snprintf(target, (len+1), "*%s=", (f != NULL)?f:"");
	target[len] = 0;

	follow_info = strcasestr(var_info, target);
	free(target);
	if (follow_info == NULL) {
		if (account == NULL)
			usb_dbg("No right about \"%s\" with the share mode.\n",
				f ? f : "Pool");
		else
			usb_dbg("No right about \"%s\" with \"%s\".\n",
				f ? f : "Pool", account);

		if (f == NULL) {
			free(var_info);
			return -1;
		} else {
			f = NULL;
			goto retry_get_permission;
		}
	}

	follow_info += len;

	if (follow_info[MAX_PROTOCOL_NUM] != '\n') {
		if (account == NULL)
			usb_dbg
			    ("The var info is incorrect.\nPlease reset the var file of the share mode.\n");
		else
			usb_dbg
			    ("The var info is incorrect.\nPlease reset the var file of \"%s\".\n",
			     account);

		free(var_info);
		return -1;
	}
	// 5. get the right of folder
	if (!strcmp(protocol, PROTOCOL_CIFS))
		result = follow_info[0] - '0';
	else if (!strcmp(protocol, PROTOCOL_FTP))
		result = follow_info[1] - '0';
	else if (!strcmp(protocol, PROTOCOL_MEDIASERVER))
		result = follow_info[2] - '0';
#ifdef RTCONFIG_WEBDAV_OLD
	else if (!strcmp(protocol, PROTOCOL_WEBDAV))
		result = follow_info[3] - '0';
#endif
	else {
		usb_dbg("The protocol, \"%s\", is incorrect.\n", protocol);
		free(var_info);
		return -1;
	}
	free(var_info);

	if (result < 0 || result > 3) {
		if (account == NULL)
			usb_dbg
			    ("The var info is incorrect.\nPlease reset the var file of the share mode.\n");
		else
			usb_dbg
			    ("The var info is incorrect.\nPlease reset the var file of \"%s\".\n",
			     account);
		return -1;
	}

	return result;
}
#else	/* !RTCONFIG_PERMISSION_MANAGEMENT */
extern int get_permission(const char *const account,
						  const char *const mount_path,
						  const char *const folder,
						  const char *const protocol,
						  const int is_group) {
	char *var_file, *var_info;
	char *target, *follow_info;
	int len, result;
	char *f = (char*) folder;
	
	// 1. get the var file
	if(get_var_file_name(account, mount_path, &var_file, 0)){
		usb_dbg("Can't malloc \"var_file\".\n");
		return -1;
	}
	
	// 2. check the file integrity.
	if(!check_file_integrity(var_file)){
		usb_dbg("Fail to check the file: %s.\n", var_file);
		if(initial_var_file(account, mount_path, 0) != 0){
			usb_dbg("Can't initial \"%s\"'s file in %s.\n", account, mount_path);
			free(var_file);
			return -1;
		}
	}
	
	// 3. get the content of the var_file of the account
	var_info = read_whole_file(var_file);
	if (var_info == NULL) {
		usb_dbg("get_permission: \"%s\" isn't existed or there's no content.\n", var_file);
		free(var_file);
		return -1;
	}
	free(var_file);
	
	// 4. get the target in the content
retry_get_permission:
	if(f == NULL)
		len = strlen("*=");
	else
		len = strlen("*")+strlen(f)+strlen("=");
	target = (char *)malloc(sizeof(char)*(len+1));
	if (target == NULL) {
		usb_dbg("Can't allocate \"target\".\n");
		free(var_info);
		return -1;
	}
	snprintf(target, (len+1), "*%s=", (f != NULL)?f:"");
	target[len] = 0;
	
	follow_info = strcasestr(var_info, target);
	free(target);
	if (follow_info == NULL) {
		if(account == NULL)
			usb_dbg("No right about \"%s\" with the share mode.\n", f? f:"Pool");
		else
			usb_dbg("No right about \"%s\" with \"%s\".\n", f? f:"Pool", account);

		if (f == NULL) {
			free(var_info);
			return -1;
		} else {
			f = NULL;
			goto retry_get_permission;
		}
	}
	
	follow_info += len;

	if (follow_info[MAX_PROTOCOL_NUM] != '\n') {
		if(account == NULL)
			usb_dbg("The var info is incorrect.\nPlease reset the var file of the share mode.\n");
		else
			usb_dbg("The var info is incorrect.\nPlease reset the var file of \"%s\".\n", account);

		free(var_info);
		return -1;
	}
	
	// 5. get the right of folder
	if (!strcmp(protocol, PROTOCOL_CIFS))
		result = follow_info[0]-'0';
	else if (!strcmp(protocol, PROTOCOL_FTP))
		result = follow_info[1]-'0';
	else if (!strcmp(protocol, PROTOCOL_MEDIASERVER))
		result = follow_info[2]-'0';
#ifdef RTCONFIG_WEBDAV_OLD
	else if (!strcmp(protocol, PROTOCOL_WEBDAV))
		result = follow_info[3]-'0';
#endif
	else{
		usb_dbg("The protocol, \"%s\", is incorrect.\n", protocol);
		free(var_info);
		return -1;
	}
	free(var_info);
	
	if (result < 0 || result > 3) {
		if(account == NULL)
			usb_dbg("The var info is incorrect.\nPlease reset the var file of the share mode.\n");
		else
			usb_dbg("The var info is incorrect.\nPlease reset the var file of \"%s\".\n", account);
		return -1;
	}
	
	return result;
}
#endif

#if defined(RTCONFIG_PERMISSION_MANAGEMENT)
int set_permission(const char *const account,
		   const char *const mount_path,
		   const char *const folder,
		   const char *const protocol,
		   const int flag, const int is_group)
{
	FILE *fp;
	char *var_file, *var_info;
	char *target, *follow_info;
	int len;

	if (flag < 0 || flag > 3) {
		usb_dbg("correct Rights is 0, 1, 2, 3.\n");
		return -1;
	}
	// 1. get the var file
	if (get_var_file_name(account, mount_path, &var_file, is_group)) {
		usb_dbg("Can't malloc \"var_file\".\n");
		return -1;
	}
	// 2. check the file integrity.
	if (!check_file_integrity(var_file)) {
		usb_dbg("Fail to check the file: %s.\n", var_file);
		if (initial_var_file(account, mount_path, is_group) != 0) {
			usb_dbg("Can't initial \"%s\"'s file in %s.\n", account,
				mount_path);
			free(var_file);
			return -1;
		}
	}
	// 3. get the content of the var_file of the account
	var_info = read_whole_file(var_file);
	if (var_info == NULL) {
		initial_var_file(account, mount_path, is_group);
		sleep(1);
		var_info = read_whole_file(var_file);
		if (var_info == NULL) {
			usb_dbg
			    ("set_permission: \"%s\" isn't existed or there's no content.\n",
			     var_file);
			free(var_file);
			return -1;
		}
	}
	// 4. get the target in the content
	if (folder == NULL)
		len = strlen("*=");
	else
		len = strlen("*") + strlen(folder) + strlen("=");
	target = (char *)malloc(sizeof(char)*(len+1));
	if (target == NULL) {
		usb_dbg("Can't allocate \"target\".\n");
		free(var_file);
		free(var_info);
		return -1;
	}
	snprintf(target, (len+1), "*%s=", (folder != NULL)?folder:"");
	target[len] = 0;

	// 5. judge if the target is in the var file.
	follow_info = strcasestr(var_info, target);
	if (follow_info == NULL) {
		if (account == NULL)
			usb_dbg("No right about \"%s\" with the share mode.\n",
				(folder == NULL ? "Pool" : folder));
		else
			usb_dbg("No right about \"%s\" with \"%s\".\n",
				(folder == NULL ? "Pool" : folder), account);
		free(var_info);

		fp = fopen(var_file, "a+");
		if (fp == NULL) {
			usb_dbg("1. Can't rewrite the file, \"%s\".\n",
				var_file);
			free(var_file);
			free(target);
			return -1;
		}
		free(var_file);

		fprintf(fp, "%s", target);
		free(target);

		// 6.1 change the right of folder
#ifdef RTCONFIG_WEBDAV_OLD
		if (!strcmp(protocol, PROTOCOL_CIFS))
			fprintf(fp, "%d%d%d%d\n", flag, 0, DEFAULT_DMS_RIGHT,
				DEFAULT_WEBDAV_RIGHT);
		else if (!strcmp(protocol, PROTOCOL_FTP))
			fprintf(fp, "%d%d%d%d\n", 0, flag, DEFAULT_DMS_RIGHT,
				DEFAULT_WEBDAV_RIGHT);
		else if (!strcmp(protocol, PROTOCOL_MEDIASERVER))
			fprintf(fp, "%d%d%d%d\n", 0, 0, flag,
				DEFAULT_WEBDAV_RIGHT);
		else if (!strcmp(protocol, PROTOCOL_WEBDAV))
			fprintf(fp, "%d%d%d%d\n", 0, 0, DEFAULT_DMS_RIGHT,
				flag);
#else
		if (!strcmp(protocol, PROTOCOL_CIFS))
			fprintf(fp, "%d%d%d\n", flag, 0, DEFAULT_DMS_RIGHT);
		else if (!strcmp(protocol, PROTOCOL_FTP))
			fprintf(fp, "%d%d%d\n", 0, flag, DEFAULT_DMS_RIGHT);
		else if (!strcmp(protocol, PROTOCOL_MEDIASERVER))
			fprintf(fp, "%d%d%d\n", 0, 0, flag);
#endif
		else {
			usb_dbg("The protocol, \"%s\", is incorrect.\n",
				protocol);

			fclose(fp);
			return -1;
		}

		fclose(fp);
		return 0;
	}
	free(target);

	follow_info += len;
	if (follow_info[MAX_PROTOCOL_NUM] != '\n') {
		if (account == NULL)
			usb_dbg
			    ("The var info is incorrect.\nPlease reset the var file of the share mode.\n");
		else
			usb_dbg
			    ("The var info is incorrect.\nPlease reset the var file of \"%s\".\n",
			     account);
		free(var_file);
		free(var_info);
		return -1;
	}
	// 6.2. change the right of folder
	if (!strcmp(protocol, PROTOCOL_CIFS))
		follow_info += PROTOCOL_CIFS_BIT;
	else if (!strcmp(protocol, PROTOCOL_FTP))
		follow_info += PROTOCOL_FTP_BIT;
	else if (!strcmp(protocol, PROTOCOL_MEDIASERVER))
		follow_info += PROTOCOL_MEDIASERVER_BIT;
#ifdef RTCONFIG_WEBDAV_OLD
	else if (!strcmp(protocol, PROTOCOL_WEBDAV))
		follow_info += PROTOCOL_WEBDAV_BIT;
#endif
	else {
		usb_dbg("The protocol, \"%s\", is incorrect.\n", protocol);
		free(var_file);
		free(var_info);
		return -1;
	}

	if (follow_info[0] == '0' + flag) {
		usb_dbg("The %s right of \"%s\" is the same.\n", protocol,
			folder);
		free(var_file);
		free(var_info);
		return 0;
	}

	follow_info[0] = '0' + flag;

	// 7. rewrite the var file.
	fp = fopen(var_file, "w");
	if (fp == NULL) {
		usb_dbg("2. Can't rewrite the file, \"%s\".\n", var_file);
		free(var_file);
		free(var_info);
		return -1;
	}
	fprintf(fp, "%s", var_info);
	fclose(fp);
	free(var_file);
	free(var_info);

	return 0;
}
#else	/* !RTCONFIG_PERMISSION_MANAGEMENT */
extern int set_permission(const char *const account,
						  const char *const mount_path,
						  const char *const folder,
						  const char *const protocol,
						  const int flag,
						  const int is_group) {
	FILE *fp;
	char *var_file, *var_info;
	char *target, *follow_info;
	int len;
	
	if (flag < 0 || flag > 3) {
		usb_dbg("correct Rights is 0, 1, 2, 3.\n");
		return -1;
	}
	
	// 1. get the var file
	if(get_var_file_name(account, mount_path, &var_file, 0)){
		usb_dbg("Can't malloc \"var_file\".\n");
		return -1;
	}
	
	// 2. check the file integrity.
	if(!check_file_integrity(var_file)){
		usb_dbg("Fail to check the file: %s.\n", var_file);
		if(initial_var_file(account, mount_path, 0) != 0){
			usb_dbg("Can't initial \"%s\"'s file in %s.\n", account, mount_path);
			free(var_file);
			return -1;
		}
	}
	
	// 3. get the content of the var_file of the account
	var_info = read_whole_file(var_file);
	if (var_info == NULL) {
		initial_var_file(account, mount_path, 0);
		sleep(1);
		var_info = read_whole_file(var_file);
		if (var_info == NULL) {
			usb_dbg("set_permission: \"%s\" isn't existed or there's no content.\n", var_file);
			free(var_file);
			return -1;
		}
	}
	
	// 4. get the target in the content
	if(folder == NULL)
		len = strlen("*=");
	else
		len = strlen("*")+strlen(folder)+strlen("=");
	target = (char *)malloc(sizeof(char)*(len+1));
	if (target == NULL) {
		usb_dbg("Can't allocate \"target\".\n");
		free(var_file);
		free(var_info);
		return -1;
	}
	snprintf(target, (len+1), "*%s=", (folder != NULL)?folder:"");
	target[len] = 0;
	
	// 5. judge if the target is in the var file.
	follow_info = strcasestr(var_info, target);
	if (follow_info == NULL) {
		if(account == NULL)
			usb_dbg("No right about \"%s\" with the share mode.\n", (folder == NULL?"Pool":folder));
		else
			usb_dbg("No right about \"%s\" with \"%s\".\n", (folder == NULL?"Pool":folder), account);
		free(var_info);
		
		fp = fopen(var_file, "a+");
		if (fp == NULL) {
			usb_dbg("1. Can't rewrite the file, \"%s\".\n", var_file);
			free(var_file);
			free(target);
			return -1;
		}
		free(var_file);
		
		fprintf(fp, "%s", target);
		free(target);
		
		// 6.1 change the right of folder
#ifdef RTCONFIG_WEBDAV_OLD
		if (!strcmp(protocol, PROTOCOL_CIFS))
			fprintf(fp, "%d%d%d%d\n", flag, 0, DEFAULT_DMS_RIGHT, DEFAULT_WEBDAV_RIGHT);
		else if (!strcmp(protocol, PROTOCOL_FTP))
			fprintf(fp, "%d%d%d%d\n", 0, flag, DEFAULT_DMS_RIGHT, DEFAULT_WEBDAV_RIGHT);
		else if (!strcmp(protocol, PROTOCOL_MEDIASERVER))
			fprintf(fp, "%d%d%d%d\n", 0, 0, flag, DEFAULT_WEBDAV_RIGHT);
		else if (!strcmp(protocol, PROTOCOL_WEBDAV))
			fprintf(fp, "%d%d%d%d\n", 0, 0, DEFAULT_DMS_RIGHT, flag);
#else
		if (!strcmp(protocol, PROTOCOL_CIFS))
			fprintf(fp, "%d%d%d\n", flag, 0, DEFAULT_DMS_RIGHT);
		else if (!strcmp(protocol, PROTOCOL_FTP))
			fprintf(fp, "%d%d%d\n", 0, flag, DEFAULT_DMS_RIGHT);
		else if (!strcmp(protocol, PROTOCOL_MEDIASERVER))
			fprintf(fp, "%d%d%d\n", 0, 0, flag);
#endif
		else{
			usb_dbg("The protocol, \"%s\", is incorrect.\n", protocol);
			
			fclose(fp);
			return -1;
		}
		
		fclose(fp);
		return 0;
	}
	free(target);
	
	follow_info += len;
	if (follow_info[MAX_PROTOCOL_NUM] != '\n') {
		if(account == NULL)
			usb_dbg("The var info is incorrect.\nPlease reset the var file of the share mode.\n");
		else
			usb_dbg("The var info is incorrect.\nPlease reset the var file of \"%s\".\n", account);
		free(var_file);
		free(var_info);
		return -1;
	}
	
	// 6.2. change the right of folder
	if(!strcmp(protocol, PROTOCOL_CIFS))
		follow_info += PROTOCOL_CIFS_BIT;
	else if(!strcmp(protocol, PROTOCOL_FTP))
		follow_info += PROTOCOL_FTP_BIT;
	else if(!strcmp(protocol, PROTOCOL_MEDIASERVER))
		follow_info += PROTOCOL_MEDIASERVER_BIT;
#ifdef RTCONFIG_WEBDAV_OLD
	else if(!strcmp(protocol, PROTOCOL_WEBDAV))
		follow_info += PROTOCOL_WEBDAV_BIT;
#endif
	else{
		usb_dbg("The protocol, \"%s\", is incorrect.\n", protocol);
		free(var_file);
		free(var_info);
		return -1;
	}
	
	if(follow_info[0] == '0'+flag){
		usb_dbg("The %s right of \"%s\" is the same.\n", protocol, folder);
		free(var_file);
		free(var_info);
		return 0;
	}
	
	follow_info[0] = '0'+flag;
	
	// 7. rewrite the var file.
	fp = fopen(var_file, "w");
	if (fp == NULL) {
		usb_dbg("2. Can't rewrite the file, \"%s\".\n", var_file);
		free(var_file);
		free(var_info);
		return -1;
	}
	fprintf(fp, "%s", var_info);
	fclose(fp);
	free(var_file);
	free(var_info);
	
	return 0;
}
#endif

int add_folder_at_var_file(const char *const account,
			   const char *const owned_account,
			   const char *const mount_path,
			   const char *const folder, const int is_group)
{
	int result, len;
	char *var_file, *var_info;
	char *target;
	FILE *fp;
	int samba_right, ftp_right, dms_right;
#ifdef RTCONFIG_WEBDAV_OLD
	int webdav_right;
#endif

	result = get_var_file_name(account, mount_path, &var_file, is_group);
	if (result) {
		usb_dbg("%s: Can't malloc \"var_file\".\n", __FUNCTION__);
		return -1;
	}

	if (!check_file_integrity(var_file)) {
		usb_dbg("%s: Fail to check the file: %s.\n", __FUNCTION__,
			var_file);
		result = initial_var_file(account, mount_path, is_group);
		if (result != 0)
			usb_dbg("%s: Can't initial the file in %s.\n",
				__FUNCTION__, mount_path);

		free(var_file);
		return -1;
	}

	len = strlen("*") + strlen(folder) + strlen("=");
	target = (char *)malloc(sizeof(char)*(len+1));
	if (target == NULL) {
		usb_dbg("add_folder: Can't allocate \"target\".\n");
		free(var_file);
		return -1;
	}
	snprintf(target, (len+1), "*%s=", folder);
	target[len] = 0;

	var_info = read_whole_file(var_file);
	if (var_info == NULL) {
		usb_dbg("%s: \"%s\" isn't existed or there's no content.\n",
			__FUNCTION__, var_file);
		free(var_file);
		free(target);
		return -1;
	} else if (strcasestr(var_info, target) != NULL) {
		free(var_file);
		free(target);
		free(var_info);
		return 0;
	}
	free(var_info);

	fp = fopen(var_file, "a+");
	if (fp == NULL) {
		usb_dbg("%s: Can't write \"%s\".\n", __FUNCTION__, var_file);
		free(var_file);
		free(target);
		return -1;
	}

	if (account == NULL	// share mode.
	    || !strcmp(account, nvram_safe_get("http_username"))
	    ) {
		samba_right = DEFAULT_SAMBA_RIGHT;
		ftp_right = DEFAULT_FTP_RIGHT;
		dms_right = DEFAULT_DMS_RIGHT;
#ifdef RTCONFIG_WEBDAV_OLD
		webdav_right = DEFAULT_WEBDAV_RIGHT;
#endif
	} else if (owned_account != NULL && !strcmp(account, owned_account)) {
		samba_right = DEFAULT_SAMBA_RIGHT;
		ftp_right = DEFAULT_FTP_RIGHT;
		dms_right = DEFAULT_DMS_RIGHT;
#ifdef RTCONFIG_WEBDAV_OLD
		webdav_right = DEFAULT_WEBDAV_RIGHT;
#endif
	}
	else if (is_group) {
		samba_right = DEFAULT_SAMBA_RIGHT;
		ftp_right = DEFAULT_FTP_RIGHT;
		dms_right = DEFAULT_DMS_RIGHT;
#ifdef RTCONFIG_WEBDAV_OLD
		webdav_right = DEFAULT_WEBDAV_RIGHT;
#endif
	}
	else {
		samba_right = 0;
		ftp_right = 0;
		dms_right = 0;
#ifdef RTCONFIG_WEBDAV_OLD
		webdav_right = 0;
#endif
	}

#ifdef RTCONFIG_WEBDAV_OLD
	fprintf(fp, "%s%d%d%d%d\n", target, samba_right, ftp_right, dms_right,
		webdav_right);
#else
	fprintf(fp, "%s%d%d%d\n", target, samba_right, ftp_right, dms_right);
#endif
	fclose(fp);

	set_file_integrity(var_file);
	free(var_file);
	free(target);

	return 0;
}

extern int add_folder(const char *const account, const char *const mount_path, const char *const folder){
	int result, len;
	int acc_num;
#ifdef RTCONFIG_PERMISSION_MANAGEMENT
	PMS_ACCOUNT_INFO_T *account_list, *follow_account;
	int group_num;
	PMS_ACCOUNT_GROUP_INFO_T *group_list, *follow_group;
	char char_user[64];
#else
	int i;
	char **account_list;
#endif
	char *full_path;

	if(mount_path == NULL || strlen(mount_path) <= 0){
		usb_dbg("add_folder: No input, \"mount_path\".\n");
		return -1;
	}
	if(folder == NULL || strlen(folder) <= 0){
		usb_dbg("add_folder: No input, \"folder\".\n");
		return -1;
	}

	// 1. test if creatting the folder
	result = test_if_exist_share(mount_path, folder);
	if(result != 0){
		usb_dbg("add_folder: \"%s\" is already created in %s.\n", folder, mount_path);
		return -1;
	}

	// 2. create the folder
	len = strlen(mount_path)+strlen("/")+strlen(folder);
	full_path = (char *)malloc(sizeof(char)*(len+1));
	if(full_path == NULL){
		usb_dbg("add_folder: Can't malloc \"full_path\".\n");
		return -1;
	}
	snprintf(full_path, (len+1), "%s/%s", mount_path, folder);
	full_path[len] = 0;

	umask(0000);
	result = mkdir(full_path, 0777);
	free(full_path);
	if(result != 0){
		usb_dbg("add_folder: To create \"%s\" is failed!\n", folder);
		return -1;
	}

	// 3. add folder's right to every var file
	// share mode.
	add_folder_at_var_file(NULL, NULL, mount_path, folder, 0);

#ifdef RTCONFIG_PERMISSION_MANAGEMENT
	if(PMS_GetAccountInfo(PMS_ACTION_GET_FULL, &account_list, &group_list, &acc_num, &group_num) < 0){
		usb_dbg("Can't get the account list\n");
		PMS_FreeAccInfo(&account_list, &group_list);
		return -1;
	}

	for(follow_group = group_list; follow_group != NULL; follow_group = follow_group->next){
		memset(char_user, 0, sizeof(char_user));
		ascii_to_char_safe(char_user, follow_group->name, sizeof(char_user));

		// 4. get the var file
		add_folder_at_var_file(char_user, NULL, mount_path, folder, 1);
	}

	for(follow_account = account_list; follow_account != NULL; follow_account = follow_account->next){
		memset(char_user, 0, sizeof(char_user));
		ascii_to_char_safe(char_user, follow_account->name, sizeof(char_user));

		// 4. get the var file
		add_folder_at_var_file(char_user, account, mount_path, folder, 0);
	}

	PMS_FreeAccInfo(&account_list, &group_list);
#else
	result = get_account_list(&acc_num, &account_list);
	if(result < 0){
		usb_dbg("Can't get the account list\n");
		free_2_dimension_list(&acc_num, &account_list);
		return -1;
	}

	for(i = 0; i < acc_num; ++i){
		// 4. get the var file
		add_folder_at_var_file(account_list[i], account, mount_path, folder, 0);
	}
	free_2_dimension_list(&acc_num, &account_list);
#endif

	// 5. add the folder's info in the folder list
	initial_folder_list(mount_path);

	return 0;
}

int del_folder_at_var_file(const char *const account,
			   const char *const mount_path,
			   const char *const folder, const int is_group)
{
	int result, len;
	char *var_file, *var_info;
	char *follow_info, backup;
	char *target;
	FILE *fp;

	result = get_var_file_name(account, mount_path, &var_file, is_group);
	if (result) {
		usb_dbg("%s: Can't malloc \"var_file\".\n", __FUNCTION__);
		return -1;
	}

	if (!check_file_integrity(var_file)) {
		usb_dbg("%s: Fail to check the file: %s.\n", __FUNCTION__,
			var_file);
		result = initial_var_file(account, mount_path, is_group);
		if (result != 0)
			usb_dbg("%s: Can't initial the file in %s.\n",
				__FUNCTION__, mount_path);

		free(var_file);
		return -1;
	}

	len = strlen("*") + strlen(folder) + strlen("=");
	target = (char *)malloc(sizeof(char)*(len+1));
	if (target == NULL) {
		usb_dbg("%s: Can't allocate \"target\".\n", __FUNCTION__);
		free(var_file);
		return -1;
	}
	snprintf(target, (len+1), "*%s=", folder);
	target[len] = 0;

	var_info = read_whole_file(var_file);
	if (var_info == NULL) {
		usb_dbg("%s: \"%s\" isn't existed or there's no content.\n",
			__FUNCTION__, var_file);
		free(var_file);
		free(target);
		return -1;
	}

	follow_info = strcasestr(var_info, target);
	free(target);
	if (follow_info == NULL) {
		free(var_file);
		free(var_info);
		return -1;
	}
	backup = *follow_info;
	*follow_info = 0;

	fp = fopen(var_file, "w");
	if (fp == NULL) {
		usb_dbg("%s: Can't write \"%s\".\n", __FUNCTION__, var_file);
		*follow_info = backup;
		free(var_file);
		free(var_info);
		return -1;
	}
	fprintf(fp, "%s", var_info);

	*follow_info = backup;
	while (*follow_info != 0 && *follow_info != '\n')
		++follow_info;
	if (*follow_info != 0 && *(follow_info + 1) != 0) {
		++follow_info;
		fprintf(fp, "%s", follow_info);
	}
	fclose(fp);
	free(var_info);

	set_file_integrity(var_file);
	free(var_file);

	return 0;
}

extern int del_folder(const char *const mount_path, const char *const folder){
	int result, len;
	int acc_num;
#ifdef RTCONFIG_PERMISSION_MANAGEMENT
	PMS_ACCOUNT_INFO_T *account_list, *follow_account;
	int group_num;
	PMS_ACCOUNT_GROUP_INFO_T *group_list, *follow_group;
	char char_user[64];
#else
	int i;
	char **account_list;
#endif
	char *full_path;

	if(mount_path == NULL || strlen(mount_path) <= 0){
		usb_dbg("No input, \"mount_path\".\n");
		return -1;
	}
	if(folder == NULL || strlen(folder) <= 0){
		usb_dbg("No input, \"folder\".\n");
		return -1;
	}

	// 1. test if deleting the folder
	len = strlen(mount_path)+strlen("/")+strlen(folder);
	full_path = (char *)malloc(sizeof(char)*(len+1));
	if(full_path == NULL){
		usb_dbg("Can't malloc \"full_path\".\n");
		return -1;
	}
	snprintf(full_path, (len+1), "%s/%s", mount_path, folder);
	full_path[len] = 0;

	result = test_if_exist_share(mount_path, folder);
	if(result == 0){
		result = check_if_dir_exist(full_path);
		if(result != 1){
			usb_dbg("\"%s\" isn't already existed in %s.\n", folder, mount_path);
			free(full_path);
			return -1;
		}
	}

	// 2. delete the folder
	result = delete_file_or_dir(full_path);
	free(full_path);
	if(result != 0){
		usb_dbg("To delete \"%s\" is failed!\n", folder);
		return -1;
	}

	// 3. del folder's right to every var file
	// share mode.
	del_folder_at_var_file(NULL, mount_path, folder, 0);

#ifdef RTCONFIG_PERMISSION_MANAGEMENT
	if(PMS_GetAccountInfo(PMS_ACTION_GET_FULL, &account_list, &group_list, &acc_num, &group_num) < 0){
		usb_dbg("Can't get the account list\n");
		PMS_FreeAccInfo(&account_list, &group_list);
		return -1;
	}

	for(follow_group = group_list; follow_group != NULL; follow_group = follow_group->next){
		memset(char_user, 0, sizeof(char_user));
		ascii_to_char_safe(char_user, follow_group->name, sizeof(char_user));

		// 4. get the var file
		del_folder_at_var_file(char_user, mount_path, folder, 1);
	}

	for(follow_account = account_list; follow_account != NULL; follow_account = follow_account->next){
		memset(char_user, 0, sizeof(char_user));
		ascii_to_char_safe(char_user, follow_account->name, sizeof(char_user));

		// 4. get the var file
		del_folder_at_var_file(char_user, mount_path, folder, 0);
	}

	PMS_FreeAccInfo(&account_list, &group_list);
#else
	result = get_account_list(&acc_num, &account_list);
	if(result < 0){
		usb_dbg("Can't get the account list\n");
		free_2_dimension_list(&acc_num, &account_list);
		return -1;
	}

	for(i = 0; i < acc_num; ++i){
		// 4. get the var file
		del_folder_at_var_file(account_list[i], mount_path, folder, 0);
	}
	free_2_dimension_list(&acc_num, &account_list);
#endif

	// 5. modify the folder's info in the folder list
	initial_folder_list(mount_path);

	return 0;
}

int mod_folder_at_var_file(const char *const account,
			   const char *const mount_path,
			   const char *const folder,
			   const char *const new_folder, const int is_group)
{
	int result, len;
	char *var_file, *var_info;
	char *target, *new_target;
	FILE *fp;
	char *follow_info, backup;

	result = get_var_file_name(account, mount_path, &var_file, is_group);
	if (result) {
		usb_dbg("%s: Can't malloc \"var_file\".\n", __FUNCTION__);
		return -1;
	}

	if (!check_file_integrity(var_file)) {
		usb_dbg("%s: Fail to check the file: %s.\n", __FUNCTION__,
			var_file);
		result = initial_var_file(account, mount_path, is_group);
		if (result != 0)
			usb_dbg("%s: Can't initial the file in %s.\n",
				__FUNCTION__, mount_path);

		free(var_file);
		return -1;
	}

	len = strlen("*") + strlen(folder) + strlen("=");
	target = (char *)malloc(sizeof(char)*(len+1));
	if (target == NULL) {
		usb_dbg("%s: Can't allocate \"target\".\n", __FUNCTION__);
		return -1;
	}
	snprintf(target, (len+1), "*%s=", folder);
	target[len] = 0;

	len = strlen("*") + strlen(new_folder) + strlen("=");
	new_target = (char *)malloc(sizeof(char)*(len+1));
	if (new_target == NULL) {
		usb_dbg("%s: Can't allocate \"new_target\".\n", __FUNCTION__);
		free(target);
		return -1;
	}
	snprintf(new_target, (len+1), "*%s=", new_folder);
	new_target[len] = 0;

	var_info = read_whole_file(var_file);
	if (var_info == NULL) {
		usb_dbg("%s: \"%s\" isn't existed or there's no content.\n",
			__FUNCTION__, var_file);
		free(var_file);
		free(target);
		free(new_target);
		return -1;
	}

	if ((follow_info = strcasestr(var_info, target)) == NULL) {
		usb_dbg("%s: No \"%s\" in \"%s\"..\n", __FUNCTION__, folder,
			var_file);
		free(var_file);
		free(target);
		free(new_target);
		free(var_info);
		return -1;
	}

	fp = fopen(var_file, "w");
	if (fp == NULL) {
		usb_dbg("%s: Can't write \"%s\".\n", __FUNCTION__, var_file);
		free(var_file);
		free(target);
		free(new_target);
		free(var_info);
		return -1;
	}
	// write the info before target
	backup = *follow_info;
	*follow_info = 0;
	fprintf(fp, "%s", var_info);
	*follow_info = backup;

	// write the info before new_target
	fprintf(fp, "%s", new_target);

	// write the info after target
	follow_info += strlen(target);
	fprintf(fp, "%s", follow_info);

	fclose(fp);
	free(var_info);

	set_file_integrity(var_file);
	free(var_file);

	return 0;
}

extern int mod_folder(const char *const mount_path, const char *const folder, const char *const new_folder){
	int result, len;
	int acc_num;
#ifdef RTCONFIG_PERMISSION_MANAGEMENT
	PMS_ACCOUNT_INFO_T *account_list, *follow_account;
	int group_num;
	PMS_ACCOUNT_GROUP_INFO_T *group_list, *follow_group;
	char char_user[64];
#else
	int i;
	char **account_list;
#endif
	char *full_path, *new_full_path;

	if(mount_path == NULL || strlen(mount_path) <= 0){
		usb_dbg("No input, \"mount_path\".\n");
		return -1;
	}
	if(folder == NULL || strlen(folder) <= 0){
		usb_dbg("No input, \"folder\".\n");
		return -1;
	}
	if(new_folder == NULL || strlen(new_folder) <= 0){
		usb_dbg("No input, \"new_folder\".\n");
		return -1;
	}

	// 1. test if modifying the folder
	len = strlen(mount_path)+strlen("/")+strlen(folder);
	full_path = (char *)malloc(sizeof(char)*(len+1));
	if(full_path == NULL){
		usb_dbg("Can't malloc \"full_path\".\n");
		return -1;
	}
	snprintf(full_path, (len+1), "%s/%s", mount_path, folder);
	full_path[len] = 0;

	len = strlen(mount_path)+strlen("/")+strlen(new_folder);
	new_full_path = (char *)malloc(sizeof(char)*(len+1));
	if(new_full_path == NULL){
		usb_dbg("Can't malloc \"new_full_path\".\n");
		free(full_path);
		return -1;
	}
	snprintf(new_full_path, (len+1), "%s/%s", mount_path, new_folder);
	new_full_path[len] = 0;

	result = test_if_exist_share(mount_path, folder);
	if(result == 0){
		result = check_if_dir_exist(full_path);

		if(result != 1){
			usb_dbg("\"%s\" isn't already existed in %s.\n", folder, mount_path);
			free(full_path);
			free(new_full_path);
			return -1;
		}

		// the folder is existed but not in .__folder_list.txt
		add_folder(NULL, mount_path, folder);
	}

	// 2. modify the folder
	result = rename(full_path, new_full_path);
	free(full_path);
	free(new_full_path);
	if(result != 0){
		usb_dbg("To delete \"%s\" is failed!\n", folder);
		return -1;
	}

	// 3. add folder's right to every var file
	// share mode.
	mod_folder_at_var_file(NULL, mount_path, folder, new_folder, 0);

#ifdef RTCONFIG_PERMISSION_MANAGEMENT
	if(PMS_GetAccountInfo(PMS_ACTION_GET_FULL, &account_list, &group_list, &acc_num, &group_num) < 0){
		usb_dbg("Can't get the account list\n");
		PMS_FreeAccInfo(&account_list, &group_list);
		return -1;
	}

	for(follow_group = group_list; follow_group != NULL; follow_group = follow_group->next){
		memset(char_user, 0, sizeof(char_user));
		ascii_to_char_safe(char_user, follow_group->name, sizeof(char_user));

		// 4. get the var file
		mod_folder_at_var_file(char_user, mount_path, folder, new_folder, 1);
	}

	for(follow_account = account_list; follow_account != NULL; follow_account = follow_account->next){
		memset(char_user, 0, sizeof(char_user));
		ascii_to_char_safe(char_user, follow_account->name, sizeof(char_user));

		// 4. get the var file
		mod_folder_at_var_file(char_user, mount_path, folder, new_folder, 0);
	}

	PMS_FreeAccInfo(&account_list, &group_list);
#else
	result = get_account_list(&acc_num, &account_list);
	if(result < 0){
		usb_dbg("Can't get the account list\n");
		free_2_dimension_list(&acc_num, &account_list);
		return -1;
	}

	for(i = 0; i < acc_num; ++i){
		// 4. get the var file
		mod_folder_at_var_file(account_list[i], mount_path, folder, new_folder, 0);
	}
	free_2_dimension_list(&acc_num, &account_list);
#endif

	// 5. modify the folder's info in the folder list
	initial_folder_list(mount_path);

	return 0;
}

extern int test_if_exist_share(const char *const mount_path, const char *const folder){
	int sh_num;
	char **folder_list;
	int result, i;
	
	result = get_folder_list(mount_path, &sh_num, &folder_list);
	if(result < 0){
		usb_dbg("Can't read the folder list in %s.\n", mount_path);
		free_2_dimension_list(&sh_num, &folder_list);
		return 0;
	}
	
	result = 0;
	for(i = 0; i < sh_num; ++i)
		if(strcasecmp(folder, folder_list[i]) == 0){
			result = 1;
			break;
		}
	free_2_dimension_list(&sh_num, &folder_list);
	
	return result;
}

// for FTP: root dir is POOL_MOUNT_ROOT.
extern int how_many_layer(const char *basedir, char **mount_path, char **share){
	char *follow_info, *follow_info_end;
	int layer = 0, len = 0;
	
	*mount_path = NULL;
	*share = NULL;
	
	if(!strcmp(basedir, "/"))
		return layer;
	
	len = strlen(basedir);
	if(len > 1)
		layer = 1;
	
	//if(basedir[len-1] == '/')
	//	--layer;
	
	follow_info = (char *)basedir;
	while(*follow_info != 0 && (follow_info = index(follow_info+1, '/')) != NULL)
		++layer;
	
	if(layer >= MOUNT_LAYER){
		follow_info = (char *)(basedir+strlen(POOL_MOUNT_ROOT));
		follow_info = index(follow_info+1, '/');
		
		if(mount_path != NULL){
			if(follow_info == NULL)
				len = strlen(basedir);
			else
				len = strlen(basedir)-strlen(follow_info);
			*mount_path = (char *)malloc(sizeof(char)*(len+1));
			if(*mount_path == NULL)
				return -1;
			strncpy(*mount_path, basedir, len);
			(*mount_path)[len] = 0;
		}
	}
	
	if(layer >= SHARE_LAYER && share != NULL){
		++follow_info;
		follow_info_end = index(follow_info, '/');
		if(follow_info_end == NULL)
			len = strlen(follow_info);
		else
			len = strlen(follow_info)-strlen(follow_info_end);
		*share = (char *)malloc(sizeof(char)*(len+1));
		if(*share == NULL)
			return -1;
		strncpy(*share, follow_info, len);
		(*share)[len] = 0;
	}
	
	return layer;
}

#if defined(RTCONFIG_PERMISSION_MANAGEMENT)
int add_account(const char *const account, const char *const password)
{
	disk_info_t *disk_list, *follow_disk;
	partition_info_t *follow_partition;
	char ascii_user[64], ascii_passwd[64];
	int acc_num;
	PMS_ACCOUNT_INFO_T *account_list, *follow_account;
	int group_num;
	PMS_ACCOUNT_GROUP_INFO_T *group_list;

	if (account == NULL || strlen(account) <= 0) {
		usb_dbg("No input, \"account\".\n");
		return -1;
	}
	if (password == NULL || strlen(password) <= 0) {
		usb_dbg("No input, \"password\".\n");
		return -1;
	}
	if (test_if_exist_account(account)) {
		usb_dbg("\"%s\" is already created.\n", account);
		return -1;
	}
	if (PMS_GetAccountInfo
	    (PMS_ACTION_GET_FULL, &account_list, &group_list, &acc_num,
	     &group_num) < 0) {
		usb_dbg("Can't read the account list.\n");
		PMS_FreeAccInfo(&account_list, &group_list);
		return -1;
	}

	PMS_FreeAccInfo(&account_list, &group_list);

	if (acc_num < 0)
		acc_num = 0;
	if (acc_num >= MAX_ACCOUNT_NUM) {
		usb_dbg("Too many accounts are created.\n");
		return -1;
	}

	memset(ascii_user, 0, sizeof(ascii_user));
	char_to_ascii_safe(ascii_user, account, sizeof(ascii_user));
	memset(ascii_passwd, 0, sizeof(ascii_passwd));
	char_to_ascii_safe(ascii_passwd, password, sizeof(ascii_passwd));

	if ((follow_account =
	     PMS_list_Account_new(5, "1", ascii_user, ascii_passwd, "",
				  "")) == NULL) {
		usb_dbg("Fail to creat the account: %s.\n", account);
		return -1;
	}
	PMS_ActionAccountInfo(PMS_ACTION_UPDATE, (void *)follow_account, 0);
	PMS_list_ACCOUNT_free(follow_account);

	disk_list = read_disk_data();
	if (disk_list == NULL) {
		usb_dbg("Couldn't read the disk list out.\n");
		return 0;
	}

	for (follow_disk = disk_list; follow_disk != NULL;
	     follow_disk = follow_disk->next) {
		for (follow_partition = follow_disk->partitions;
		     follow_partition != NULL;
		     follow_partition = follow_partition->next) {
			if (follow_partition->mount_point == NULL)
				continue;

			if (initial_var_file
			    (account, follow_partition->mount_point, 0) != 0)
				usb_dbg("Can't initial \"%s\"'s file in %s.\n",
					account, follow_partition->mount_point);
		}
	}
	free_disk_data(&disk_list);

	return 0;
}
#else	/* !RTCONFIG_PERMISSION_MANAGEMENT */
extern int add_account(const char *const account, const char *const password){
	disk_info_t *disk_list, *follow_disk;
	partition_info_t *follow_partition;
	int acc_num;
	int len;
	char nvram_value[PATH_MAX], *ptr;
	char ascii_user[64], ascii_passwd[64];
	int lock;

	if(account == NULL || strlen(account) <= 0){
		usb_dbg("No input, \"account\".\n");
		return -1;
	}
	if(password == NULL || strlen(password) <= 0){
		usb_dbg("No input, \"password\".\n");
		return -1;
	}
	if(test_if_exist_account(account)){
		usb_dbg("\"%s\" is already created.\n", account);
		return -1;
	}

	memset(ascii_user, 0, sizeof(ascii_user));
	char_to_ascii_safe(ascii_user, account, sizeof(ascii_user));

	memset(ascii_passwd, 0, sizeof(ascii_passwd));
	char_to_ascii_safe(ascii_passwd, password, sizeof(ascii_passwd));

#ifdef RTCONFIG_NVRAM_ENCRYPT
	int enclen = pw_enc_blen(ascii_passwd);
	char enc_passwd[enclen];
	char passwdbuf[NVRAM_ENC_MAXLEN];

	if(!pw_dec(password, passwdbuf)){
		pw_enc(ascii_passwd, enc_passwd);
		strlcpy(ascii_passwd, enc_passwd, sizeof(ascii_passwd));
	}else{
		strlcpy(ascii_passwd, password, sizeof(ascii_passwd));
	}
#endif

	acc_num = nvram_get_int("acc_num");
	if(acc_num < 0)
		acc_num = 0;
	if(acc_num >= MAX_ACCOUNT_NUM){
		usb_dbg("Too many accounts are created.\n");
		return -1;
	}

	snprintf(nvram_value, sizeof(nvram_value), "%s", nvram_safe_get("acc_list"));
	len = strlen(nvram_value);
	if(len > 0){
		ptr = nvram_value+len;
		sprintf(ptr, "<%s>%s", ascii_user, ascii_passwd);
		nvram_set("acc_list", nvram_value);

		snprintf(nvram_value, sizeof(nvram_value), "%d", acc_num+1);
		nvram_set("acc_num", nvram_value);
	}
	else{
		snprintf(nvram_value, sizeof(nvram_value), "%s>%s", ascii_user, ascii_passwd);
		nvram_set("acc_list", nvram_value);

		nvram_set("acc_num", "1");
	}

	lock = file_lock("nvramcommit");
	nvram_commit();
	file_unlock(lock);

	disk_list = read_disk_data();
	if(disk_list == NULL){
		usb_dbg("Couldn't read the disk list out.\n");
		return 0;
	}

	for(follow_disk = disk_list; follow_disk != NULL; follow_disk = follow_disk->next){			
		for(follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next){
			if(follow_partition->mount_point == NULL)
				continue;
			
			if(initial_var_file(account, follow_partition->mount_point, 0) != 0)
				usb_dbg("Can't initial \"%s\"'s file in %s.\n", account, follow_partition->mount_point);
		}
	}
	free_disk_data(&disk_list);

	return 0;
}
#endif

#if defined(RTCONFIG_PERMISSION_MANAGEMENT)
int del_account(const char *const account)
{
	disk_info_t *disk_list, *follow_disk;
	partition_info_t *follow_partition;
	char *var_file;
	DIR *opened_dir;
	struct dirent *dp;
	int i, need_commit = 0;
	int lock;
	char *ptr;
	int len;
	int acc_num;
	PMS_ACCOUNT_INFO_T *account_list, *follow_account;
	int group_num;
	PMS_ACCOUNT_GROUP_INFO_T *group_list;
	char ascii_user[64];

	if (account == NULL || strlen(account) <= 0) {
		usb_dbg("No input, \"account\".\n");
		return -1;
	}
	if (PMS_GetAccountInfo
	    (PMS_ACTION_GET_FULL, &account_list, &group_list, &acc_num,
	     &group_num) < 0) {
		usb_dbg("Can't read the account list.\n");
		PMS_FreeAccInfo(&account_list, &group_list);
		return -1;
	}

	PMS_FreeAccInfo(&account_list, &group_list);

	if (acc_num <= 0) {
		return 0;
	}

	memset(ascii_user, 0, sizeof(ascii_user));
	char_to_ascii_safe(ascii_user, account, sizeof(ascii_user));

	if ((follow_account = PMS_list_Account_new(2, "1", ascii_user)) == NULL) {
		usb_dbg("Fail to delete the account: %s.\n", account);
		return -1;
	}
	PMS_ActionAccountInfo(PMS_ACTION_DELETE, (void *)follow_account, 0);
	PMS_list_ACCOUNT_free(follow_account);

	i = acc_num - 1;

	if (i <= 0) {
		nvram_set("st_samba_mode", "1");
		nvram_set("st_samba_force_mode", "1");
		nvram_set("st_ftp_mode", "1");
#ifdef RTCONFIG_WEBDAV_OLD
		nvram_set("st_webdav_mode", "1");
#endif
		need_commit = 1;
	}

	if (need_commit) {
		lock = file_lock("nvramcommit");
		nvram_commit();
		file_unlock(lock);
	}

	disk_list = read_disk_data();
	if (disk_list == NULL) {
		usb_dbg("Couldn't read the disk list out.\n");
		return 0;
	}

	for (follow_disk = disk_list; follow_disk != NULL;
	     follow_disk = follow_disk->next) {
		for (follow_partition = follow_disk->partitions;
		     follow_partition != NULL;
		     follow_partition = follow_partition->next) {
			if (follow_partition->mount_point == NULL)
				continue;

			if (get_var_file_name
			    (account, follow_partition->mount_point, &var_file,
			     0)) {
				usb_dbg("Can't malloc \"var_file\".\n");
				free_disk_data(&disk_list);
				return -1;
			}

			if ((ptr = strrchr(var_file, '/')) == NULL) {
				usb_dbg("Fail to get the %s of the file.\n",
					follow_partition->mount_point);
				free_disk_data(&disk_list);
				free(var_file);
				return -1;
			}

			if ((opened_dir =
			     opendir(follow_partition->mount_point)) == NULL) {
				usb_dbg("Can't opendir \"%s\".\n",
					follow_partition->mount_point);
				free_disk_data(&disk_list);
				free(var_file);
				return -1;
			}

			++ptr;
			len = strlen(ptr);
			while ((dp = readdir(opened_dir)) != NULL) {
				char test_path[PATH_MAX];

				if (!strcmp(dp->d_name, ".")
				    || !strcmp(dp->d_name, ".."))
					continue;

				if (strncmp(dp->d_name, ptr, len))
					continue;

				snprintf(test_path, sizeof(test_path), "%s/%s",
					follow_partition->mount_point,
					dp->d_name);
				usb_dbg("delete %s.\n", test_path);
				delete_file_or_dir(test_path);
			}
			closedir(opened_dir);

			free(var_file);
		}
	}
	free_disk_data(&disk_list);

	return 0;
}
#else	/* !RTCONFIG_PERMISSION_MANAGEMENT */
extern int del_account(const char *const account){
	disk_info_t *disk_list, *follow_disk;
	partition_info_t *follow_partition;
	int acc_num, i, len;
	char nvram_value[PATH_MAX], *ptr;
	char *var_file;
	char *nv, *nvp, *b;
	char *tmp_ascii_user, *tmp_ascii_passwd, char_user[64];
	DIR *opened_dir;
	struct dirent *dp;
	int lock;

	if(account == NULL || strlen(account) <= 0){
		usb_dbg("No input, \"account\".\n");
		return -1;
	}

	acc_num = nvram_get_int("acc_num");
	if(acc_num <= 0)
		return 0;

	memset(nvram_value, 0, sizeof(nvram_value));
	nv = nvp = strdup(nvram_safe_get("acc_list"));
	i = 0;
	if(nv && strlen(nv) > 0){
		while((b = strsep(&nvp, "<")) != NULL){
			if(vstrsep(b, ">", &tmp_ascii_user, &tmp_ascii_passwd) != 2)
				continue;

			memset(char_user, 0, sizeof(char_user));
			ascii_to_char_safe(char_user, tmp_ascii_user, sizeof(char_user));

			if(!strcmp(account, char_user)){
				if(--acc_num == 0){
					nvram_set("acc_num", "0");
					nvram_set("acc_list", "");

					free(nv);
					return 0;
				}

				continue;
			}

			len = strlen(nvram_value);
			if(len > 0){
				ptr = nvram_value+len;
				sprintf(ptr, "<%s>%s", tmp_ascii_user, tmp_ascii_passwd);
			}
			else
				snprintf(nvram_value, sizeof(nvram_value), "%s>%s", tmp_ascii_user, tmp_ascii_passwd);

			if(++i >= acc_num)
				break;
		}
	}
	if(nv)
		free(nv);
	nvram_set("acc_list", nvram_value);

	snprintf(nvram_value, sizeof(nvram_value), "%d", i);
	nvram_set("acc_num", nvram_value);

	if(i <= 0){
		nvram_set("st_samba_mode", "1");
		nvram_set("st_ftp_mode", "1");
#ifdef RTCONFIG_WEBDAV_OLD
		nvram_set("st_webdav_mode", "1");
#endif
	}

	lock = file_lock("nvramcommit");
	nvram_commit();
	file_unlock(lock);

	disk_list = read_disk_data();
	if(disk_list == NULL){
		usb_dbg("Couldn't read the disk list out.\n");
		return 0;
	}

	for(follow_disk = disk_list; follow_disk != NULL; follow_disk = follow_disk->next){			
		for(follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next){
			if(follow_partition->mount_point == NULL)
				continue;

			if(get_var_file_name(account, follow_partition->mount_point, &var_file, 0)){
				usb_dbg("Can't malloc \"var_file\".\n");
				free_disk_data(&disk_list);
				return -1;
			}

			if((ptr = strrchr(var_file, '/')) == NULL){
				usb_dbg("Fail to get the %s of the file.\n", follow_partition->mount_point);
				free_disk_data(&disk_list);
				free(var_file);
				return -1;
			}

			if((opened_dir = opendir(follow_partition->mount_point)) == NULL){
				usb_dbg("Can't opendir \"%s\".\n", follow_partition->mount_point);
				free_disk_data(&disk_list);
				free(var_file);
				return -1;
			}

			++ptr;
			len = strlen(ptr);
			while((dp = readdir(opened_dir)) != NULL){
				char test_path[PATH_MAX];

				if(!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
					continue;

				if(strncmp(dp->d_name, ptr, len))
					continue;

				snprintf(test_path, sizeof(test_path), "%s/%s", follow_partition->mount_point, dp->d_name);
				usb_dbg("delete %s.\n", test_path);
				delete_file_or_dir(test_path);
			}
			closedir(opened_dir);

			free(var_file);
		}
	}
	free_disk_data(&disk_list);

	return 0;
}
#endif

#ifdef RTCONFIG_PERMISSION_MANAGEMENT
// "new_account" can be the same with "account" and only change the password!
extern int mod_account(const char *const account, const char *const new_account, const char *const new_password){
	disk_info_t *disk_list, *follow_disk;
	partition_info_t *follow_partition;
	char *var_file, *new_var_file;
	unsigned long file_size;
	char file1[PATH_MAX], file2[PATH_MAX];
	char ascii_user[64], ascii_newuser[64], ascii_passwd[64];
	int acc_num;
	PMS_ACCOUNT_INFO_T *account_list, *follow_account;
	int group_num;
	PMS_ACCOUNT_GROUP_INFO_T *group_list;
	char *changed_user, *changed_pass;

	if(account == NULL || strlen(account) <= 0){
		usb_dbg("No input, \"account\".\n");
		return -1;
	}
	/*if(new_password == NULL || strlen(new_password) <= 0){
		usb_dbg("No input, \"new_password\".\n");
		return -1;
	}//*/
	if(!test_if_exist_account(account)){
		usb_dbg("\"%s\" is not existed.\n", account);
		return -1;
	}
	if(new_account != NULL && strcmp(account, new_account) && test_if_exist_account(new_account)){
		usb_dbg("\"%s\" is already created.\n", new_account);
		return -1;
	}

	if(PMS_GetAccountInfo(PMS_ACTION_GET_FULL, &account_list, &group_list, &acc_num, &group_num) < 0){
		usb_dbg("Can't read the account list.\n");
		PMS_FreeAccInfo(&account_list, &group_list);
		return -1;
	}

	PMS_FreeAccInfo(&account_list, &group_list);

	if(acc_num <= 0){
		return 0;
	}

	memset(ascii_user, 0, sizeof(ascii_user));
	char_to_ascii_safe(ascii_user, account, sizeof(ascii_user));
	if(new_account != NULL && strlen(new_account) > 0){
		memset(ascii_newuser, 0, sizeof(ascii_newuser));
		char_to_ascii_safe(ascii_newuser, new_account, sizeof(ascii_newuser));
		changed_user = ascii_newuser;
	}
	else
		changed_user = NULL;
	if(new_password != NULL && strlen(new_password) > 0){
		memset(ascii_passwd, 0, sizeof(ascii_passwd));
		char_to_ascii_safe(ascii_passwd, new_password, sizeof(ascii_passwd));
		changed_pass = ascii_passwd;
	}
	else
		changed_pass = NULL;

	if((follow_account = PMS_list_Account_new(5, NULL, changed_user, changed_pass, NULL, NULL)) == NULL){
		usb_dbg("Fail to get the new account: %s-%s.\n", account, new_account);
		return -1;
	}
	PMS_ActionAccountInfo(PMS_ACTION_MODIFY, (void *)follow_account, 0, ascii_user);
	PMS_list_ACCOUNT_free(follow_account);

	if(new_account == NULL || strlen(new_account) <= 0 || !strcmp(new_account, account))
		return 0;

	disk_list = read_disk_data();
	if(disk_list == NULL){
		usb_dbg("Couldn't read the disk list out.\n");
		return 0;
	}

	for(follow_disk = disk_list; follow_disk != NULL; follow_disk = follow_disk->next){
		for(follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next){
			if(follow_partition->mount_point == NULL)
				continue;

			if(get_var_file_name(account, follow_partition->mount_point, &var_file, 0)){
				usb_dbg("Can't malloc \"var_file\".\n");
				free_disk_data(&disk_list);
				return -1;
			}

			if(get_var_file_name(new_account, follow_partition->mount_point, &new_var_file, 0)){
				usb_dbg("Can't malloc \"new_var_file\".\n");
				free_disk_data(&disk_list);
				free(var_file);
				return -1;
			}

			if(!check_file_integrity(var_file)){
				usb_dbg("Fail to check the file: %s.\n", var_file);
				if(initial_var_file(new_account, follow_partition->mount_point, 0) != 0){
					usb_dbg("Can't initial \"%s\"'s file in %s.\n", new_account, follow_partition->mount_point);
					free_disk_data(&disk_list);
					free(var_file);
					free(new_var_file);
					return -1;
				}
			}
			else{
				delete_file_or_dir(new_var_file);
				rename(var_file, new_var_file);

				if((file_size = f_size(new_var_file)) == -1){
					usb_dbg("Fail to get the size of the file.\n");
					free_disk_data(&disk_list);
					free(var_file);
					free(new_var_file);
					return -1;
				}

				snprintf(file1, sizeof(file1), "%s.%lu", var_file, file_size);
				snprintf(file2, sizeof(file2), "%s.%lu", new_var_file, file_size);

				delete_file_or_dir(file2);
				rename(file1, file2);
			}

			free(var_file);
			free(new_var_file);
		}
	}
	free_disk_data(&disk_list);

	return 0;
}
#else	/* !RTCONFIG_PERMISSION_MANAGEMENT */
extern int mod_account(const char *const account, const char *const new_account, const char *const new_password){
	disk_info_t *disk_list, *follow_disk;
	partition_info_t *follow_partition;
	int acc_num;
	int i, len;
	char nvram_value[PATH_MAX], *ptr;
	char *var_file, *new_var_file;
	char *nv, *nvp, *b;
	char *tmp_ascii_user, *tmp_ascii_passwd, char_user[64];
	unsigned long file_size;
	char file1[PATH_MAX], file2[PATH_MAX];
	int lock;

	if(account == NULL || strlen(account) <= 0){
		usb_dbg("No input, \"account\".\n");
		return -1;
	}
	/*if(new_password == NULL || strlen(new_password) <= 0){
		usb_dbg("No input, \"new_password\".\n");
		return -1;
	}//*/
	if(!test_if_exist_account(account)){
		usb_dbg("\"%s\" is not existed.\n", account);
		return -1;
	}
	if(new_account != NULL && strcmp(account, new_account) && test_if_exist_account(new_account)){
		usb_dbg("\"%s\" is already created.\n", new_account);
		return -1;
	}

	acc_num = nvram_get_int("acc_num");
	if(acc_num <= 0)
		return 0;

	memset(nvram_value, 0, sizeof(nvram_value));
	nv = nvp = strdup(nvram_safe_get("acc_list"));
	i = 0;
	if(nv && strlen(nv) > 0){
		while((b = strsep(&nvp, "<")) != NULL){
			if(vstrsep(b, ">", &tmp_ascii_user, &tmp_ascii_passwd) != 2)
				continue;

			char *set_passwd;
			char ascii_user[64], ascii_passwd[64];

			memset(ascii_user, 0, sizeof(ascii_user));
			if(new_account != NULL && strlen(new_account) > 0)
				char_to_ascii_safe(ascii_user, new_account, sizeof(ascii_user));
			else
				char_to_ascii_safe(ascii_user, account, sizeof(ascii_user));

			if(new_password != NULL && strlen(new_password) > 0){
				memset(ascii_passwd, 0, sizeof(ascii_passwd));
				char_to_ascii_safe(ascii_passwd, new_password, sizeof(ascii_passwd));

#ifdef RTCONFIG_NVRAM_ENCRYPT
				char enc_passwd[64];

				memset(enc_passwd, 0, sizeof(enc_passwd));
				pw_enc(ascii_passwd, enc_passwd);
				set_passwd = enc_passwd;
#else
				set_passwd = ascii_passwd;
#endif
			}
			else
				set_passwd = tmp_ascii_passwd;

			len = strlen(nvram_value);
			memset(char_user, 0, sizeof(char_user));
			ascii_to_char_safe(char_user, tmp_ascii_user, sizeof(char_user));
			if(!strcmp(account, char_user)){
				if(len == 0)
					snprintf(nvram_value, sizeof(nvram_value), "%s>%s", ascii_user, set_passwd);
				else{
					ptr = nvram_value+len;
					sprintf(ptr, "<%s>%s", ascii_user, set_passwd);
				}
			}
			else{
				if(len == 0)
					snprintf(nvram_value, sizeof(nvram_value), "%s>%s", tmp_ascii_user, tmp_ascii_passwd);
				else{
					ptr = nvram_value+len;
					sprintf(ptr, "<%s>%s", tmp_ascii_user, tmp_ascii_passwd);
				}
			}

			if(++i >= acc_num)
				break;
		}
	}
	if(nv)
		free(nv);
	nvram_set("acc_list", nvram_value);

	lock = file_lock("nvramcommit");
	nvram_commit();
	file_unlock(lock);

	if(new_account == NULL || strlen(new_account) <= 0 || !strcmp(new_account, account))
		return 0;

	disk_list = read_disk_data();
	if(disk_list == NULL){
		usb_dbg("Couldn't read the disk list out.\n");
		return 0;
	}

	for(follow_disk = disk_list; follow_disk != NULL; follow_disk = follow_disk->next){			
		for(follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next){
			if(follow_partition->mount_point == NULL)
				continue;

			if(get_var_file_name(account, follow_partition->mount_point, &var_file, 0)){
				usb_dbg("Can't malloc \"var_file\".\n");
				free_disk_data(&disk_list);
				return -1;
			}

			if(get_var_file_name(new_account, follow_partition->mount_point, &new_var_file, 0)){
				usb_dbg("Can't malloc \"new_var_file\".\n");
				free(var_file);
				free_disk_data(&disk_list);
				return -1;
			}

			if(!check_file_integrity(var_file)){
				usb_dbg("Fail to check the file: %s.\n", var_file);
				if(initial_var_file(new_account, follow_partition->mount_point, 0) != 0){
					usb_dbg("Can't initial \"%s\"'s file in %s.\n", new_account, follow_partition->mount_point);
					free_disk_data(&disk_list);
					free(var_file);
					free(new_var_file);
					return -1;
				}
			}
			else{
				delete_file_or_dir(new_var_file);
				rename(var_file, new_var_file);

				if((file_size = f_size(new_var_file)) == -1){
					usb_dbg("Fail to get the size of the file.\n");
					free_disk_data(&disk_list);
					free(var_file);
					free(new_var_file);
					return -1;
				}

				snprintf(file1, sizeof(file1), "%s.%lu", var_file, file_size);
				snprintf(file2, sizeof(file2), "%s.%lu", new_var_file, file_size);

				delete_file_or_dir(file2);
				rename(file1, file2);
			}

			free(var_file);
			free(new_var_file);
		}
	}

	free_disk_data(&disk_list);

	return 0;
}
#endif

extern int test_if_exist_account(const char *const account){
	int acc_num;
#ifdef RTCONFIG_PERMISSION_MANAGEMENT
	PMS_ACCOUNT_INFO_T *account_list, *follow_account;
	int group_num;
	PMS_ACCOUNT_GROUP_INFO_T *group_list;
	char char_user[64];
#else
	int i;
	char **account_list;
#endif
	int result;
	
	if(account == NULL)
		return -1;
	
#ifdef RTCONFIG_PERMISSION_MANAGEMENT
	if(PMS_GetAccountInfo(PMS_ACTION_GET_FULL, &account_list, &group_list, &acc_num, &group_num) < 0){
		usb_dbg("Can't get the account list.\n");
		PMS_FreeAccInfo(&account_list, &group_list);
		return -1;
	}

	result = 0;
	for(follow_account = account_list; follow_account != NULL; follow_account = follow_account->next){
		memset(char_user, 0, sizeof(char_user));
		ascii_to_char_safe(char_user, follow_account->name, sizeof(char_user));

		if(!strcmp(char_user, account)){
			result = 1;
			break;
		}
	}
	PMS_FreeAccInfo(&account_list, &group_list);
#else
	result = get_account_list(&acc_num, &account_list);
	if(result < 0){
		usb_dbg("Can't get the account list.\n");
		free_2_dimension_list(&acc_num, &account_list);
		return -1;
	}

	result = 0;
	for(i = 0; i < acc_num; ++i)
		if(!strcmp(account_list[i], account)){
			result = 1;
			break;
		}
	free_2_dimension_list(&acc_num, &account_list);
#endif

	return result;
}

#ifdef RTCONFIG_PERMISSION_MANAGEMENT
extern int add_group(const char *const group){
	disk_info_t *disk_list, *follow_disk;
	partition_info_t *follow_partition;
	char ascii_user[64];
	int acc_num;
	PMS_ACCOUNT_INFO_T *account_list;
	int group_num;
	PMS_ACCOUNT_GROUP_INFO_T *group_list, *follow_group;

	if(group == NULL || strlen(group) <= 0){
		usb_dbg("No input, \"group\".\n");
		return -1;
	}
	if(test_if_exist_group(group)){
		usb_dbg("\"%s\" is already created.\n", group);
		return -1;
	}

	if(PMS_GetAccountInfo(PMS_ACTION_GET_FULL, &account_list, &group_list, &acc_num, &group_num) < 0){
		usb_dbg("Can't read the group list.\n");
		PMS_FreeAccInfo(&account_list, &group_list);
		return -1;
	}

	PMS_FreeAccInfo(&account_list, &group_list);

	if(group_num < 0)
		group_num = 0;
	if(group_num >= MAX_GROUP_NUM){
		usb_dbg("Too many groups are created.\n");
		return -1;
	}

	memset(ascii_user, 0, sizeof(ascii_user));
	char_to_ascii_safe(ascii_user, group, sizeof(ascii_user));

	if((follow_group = PMS_list_AccountGroup_new(3, "1", ascii_user, "")) == NULL){
		usb_dbg("Fail to creat the group: %s.\n", group);
		return -1;
	}
	PMS_ActionAccountInfo(PMS_ACTION_UPDATE, (void *)follow_group, 1);
	PMS_list_ACCOUNT_GROUP_free(follow_group);

	disk_list = read_disk_data();
	if(disk_list == NULL){
		usb_dbg("Couldn't read the disk list out.\n");
		return 0;
	}

	for(follow_disk = disk_list; follow_disk != NULL; follow_disk = follow_disk->next){
		for(follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next){
			if(follow_partition->mount_point == NULL)
				continue;
			
			if(initial_var_file(group, follow_partition->mount_point, 1) != 0)
				usb_dbg("Can't initial \"%s\"'s file in %s.\n", group, follow_partition->mount_point);
		}
	}
	free_disk_data(&disk_list);

	return 0;
}

extern int del_group(const char *const group){
	disk_info_t *disk_list, *follow_disk;
	partition_info_t *follow_partition;
	char *var_file;
	DIR *opened_dir;
	struct dirent *dp;
	int i, need_commit = 0;
	int lock;
	int len;
	char *ptr;
	char ascii_user[64];
	int acc_num;
	PMS_ACCOUNT_INFO_T *account_list;
	int group_num;
	PMS_ACCOUNT_GROUP_INFO_T *group_list, *follow_group;

	if(group == NULL || strlen(group) <= 0){
		usb_dbg("No input, \"group\".\n");
		return -1;
	}

	if(PMS_GetAccountInfo(PMS_ACTION_GET_FULL, &account_list, &group_list, &acc_num, &group_num) < 0){
		usb_dbg("Can't read the group list.\n");
		PMS_FreeAccInfo(&account_list, &group_list);
		return -1;
	}

	PMS_FreeAccInfo(&account_list, &group_list);

	if(group_num <= 0){
		return 0;
	}

	memset(ascii_user, 0, sizeof(ascii_user));
	char_to_ascii_safe(ascii_user, group, sizeof(ascii_user));

	if((follow_group = PMS_list_AccountGroup_new(2, "1", ascii_user)) == NULL){
		usb_dbg("Fail to get the group: %s.\n", group);
		return -1;
	}
	PMS_ActionAccountInfo(PMS_ACTION_DELETE, (void *)follow_group, 1);
	PMS_list_ACCOUNT_GROUP_free(follow_group);

	i = group_num-1;

	if(i <= 0){
		nvram_set("st_samba_mode", "1");
		nvram_set("st_samba_force_mode", "1");
		nvram_set("st_ftp_mode", "1");
#ifdef RTCONFIG_WEBDAV_OLD
		nvram_set("st_webdav_mode", "1");
#endif
		need_commit = 1;
	}

	if(need_commit){
		lock = file_lock("nvramcommit");
		nvram_commit();
		file_unlock(lock);
	}

	disk_list = read_disk_data();
	if(disk_list == NULL){
		usb_dbg("Couldn't read the disk list out.\n");
		return 0;
	}

	for(follow_disk = disk_list; follow_disk != NULL; follow_disk = follow_disk->next){
		for(follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next){
			if(follow_partition->mount_point == NULL)
				continue;

			if(get_var_file_name(group, follow_partition->mount_point, &var_file, 1)){
				usb_dbg("Can't malloc \"var_file\".\n");
				free_disk_data(&disk_list);
				return -1;
			}

			if((ptr = strrchr(var_file, '/')) == NULL){
				usb_dbg("Fail to get the %s of the file.\n", follow_partition->mount_point);
				free_disk_data(&disk_list);
				free(var_file);
				return -1;
			}

			if((opened_dir = opendir(follow_partition->mount_point)) == NULL){
				usb_dbg("Can't opendir \"%s\".\n", follow_partition->mount_point);
				free_disk_data(&disk_list);
				free(var_file);
				return -1;
			}

			++ptr;
			len = strlen(ptr);
			while((dp = readdir(opened_dir)) != NULL){
				char test_path[PATH_MAX];

				if(!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
					continue;

				if(strncmp(dp->d_name, ptr, len))
					continue;

				snprintf(test_path, sizeof(test_path), "%s/%s", follow_partition->mount_point, dp->d_name);
				usb_dbg("delete %s.\n", test_path);
				delete_file_or_dir(test_path);
			}
			closedir(opened_dir);

			free(var_file);
		}
	}
	free_disk_data(&disk_list);

	return 0;
}

extern int mod_group(const char *const group, const char *const new_group){
	disk_info_t *disk_list, *follow_disk;
	partition_info_t *follow_partition;
	char *var_file, *new_var_file;
	unsigned long file_size;
	char file1[PATH_MAX], file2[PATH_MAX];
	char ascii_user[64], ascii_newuser[64];
	int acc_num;
	PMS_ACCOUNT_INFO_T *account_list;
	int group_num;
	PMS_ACCOUNT_GROUP_INFO_T *group_list, *follow_group;

	if(group == NULL || strlen(group) <= 0){
		usb_dbg("No input, \"group\".\n");
		return -1;
	}
	if(new_group == NULL || strlen(new_group) <= 0 || !strcmp(group, new_group))
		return 0;

	if(!test_if_exist_group(group)){
		usb_dbg("\"%s\" is not existed.\n", group);
		return -1;
	}
	if(test_if_exist_group(new_group)){
		usb_dbg("\"%s\" is already created.\n", new_group);
		return -1;
	}

	if(PMS_GetAccountInfo(PMS_ACTION_GET_FULL, &account_list, &group_list, &acc_num, &group_num) < 0){
		usb_dbg("Can't read the group list.\n");
		PMS_FreeAccInfo(&account_list, &group_list);
		return -1;
	}

	PMS_FreeAccInfo(&account_list, &group_list);

	if(group_num <= 0){
		return 0;
	}

	memset(ascii_user, 0, sizeof(ascii_user));
	char_to_ascii_safe(ascii_user, group, sizeof(ascii_user));
	memset(ascii_newuser, 0, sizeof(ascii_newuser));
	char_to_ascii_safe(ascii_newuser, new_group, sizeof(ascii_newuser));

	if((follow_group = PMS_list_AccountGroup_new(2, NULL, ascii_newuser)) == NULL){
		usb_dbg("Fail to get the group: %s.\n", group);
		return -1;
	}
	PMS_ActionAccountInfo(PMS_ACTION_MODIFY, (void *)follow_group, 1, ascii_user);
	PMS_list_ACCOUNT_GROUP_free(follow_group);

	disk_list = read_disk_data();
	if(disk_list == NULL){
		usb_dbg("Couldn't read the disk list out.\n");
		return 0;
	}

	for(follow_disk = disk_list; follow_disk != NULL; follow_disk = follow_disk->next){
		for(follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next){
			if(follow_partition->mount_point == NULL)
				continue;

			if(get_var_file_name(group, follow_partition->mount_point, &var_file, 1)){
				usb_dbg("Can't malloc \"var_file\".\n");
				free_disk_data(&disk_list);
				return -1;
			}

			if(get_var_file_name(new_group, follow_partition->mount_point, &new_var_file, 1)){
				usb_dbg("Can't malloc \"new_var_file\".\n");
				free_disk_data(&disk_list);
				free(var_file);
				return -1;
			}

			if(!check_file_integrity(var_file)){
				usb_dbg("Fail to check the file: %s.\n", var_file);
				if(initial_var_file(new_group, follow_partition->mount_point, 1) != 0){
					usb_dbg("Can't initial \"%s\"'s file in %s.\n", new_group, follow_partition->mount_point);
					free_disk_data(&disk_list);
					free(var_file);
					free(new_var_file);
					return -1;
				}
			}
			else{
				delete_file_or_dir(new_var_file);
				rename(var_file, new_var_file);

				if((file_size = f_size(new_var_file)) == -1){
					usb_dbg("Fail to get the size of the file.\n");
					free_disk_data(&disk_list);
					free(var_file);
					free(new_var_file);
					return -1;
				}

				snprintf(file1, sizeof(file1), "%s.%lu", var_file, file_size);
				snprintf(file2, sizeof(file2), "%s.%lu", new_var_file, file_size);

				delete_file_or_dir(file2);
				rename(file1, file2);
			}

			free(var_file);
			free(new_var_file);
		}
	}
	free_disk_data(&disk_list);

	return 0;
}

extern int test_if_exist_group(const char *const group){
	int acc_num;
	PMS_ACCOUNT_INFO_T *account_list;
	int group_num;
	PMS_ACCOUNT_GROUP_INFO_T *group_list, *follow_group;
	int result;
	char char_user[64];
	
	if(group == NULL)
		return -1;
	
	if(PMS_GetAccountInfo(PMS_ACTION_GET_FULL, &account_list, &group_list, &acc_num, &group_num) < 0){
		usb_dbg("Can't get the group list.\n");
		PMS_FreeAccInfo(&account_list, &group_list);
		return -1;
	}

	result = 0;
	for(follow_group = group_list; follow_group != NULL; follow_group = follow_group->next){
		memset(char_user, 0, sizeof(char_user));
		ascii_to_char_safe(char_user, follow_group->name, sizeof(char_user));

		if(!strcmp(char_user, group)){
			result = 1;
			break;
		}
	}
	PMS_FreeAccInfo(&account_list, &group_list);

	return result;
}
#else
// Success: return value is account number. Fail: return value is -1.
extern int get_account_list(int *acc_num, char ***account_list)
{
	char **tmp_account_list = NULL, **tmp_account;
	int len, i, j;
	char *nv, *nvp, *b;
	char *tmp_ascii_user, *tmp_ascii_passwd;
	char char_user[64];

	*acc_num = nvram_get_int("acc_num");
	if(*acc_num <= 0)
		return 0;

	nv = nvp = strdup(nvram_safe_get("acc_list"));
	i = 0;
	if(nv && strlen(nv) > 0){
		while((b = strsep(&nvp, "<")) != NULL){
			if(vstrsep(b, ">", &tmp_ascii_user, &tmp_ascii_passwd) != 2)
				continue;

			tmp_account = (char **)malloc(sizeof(char *)*(i+1));
			if(tmp_account == NULL){
				usb_dbg("Can't malloc \"tmp_account\".\n");
				free(nv);
				return -1;
			}

			memset(char_user, 0, sizeof(char_user));
			ascii_to_char_safe(char_user, tmp_ascii_user, sizeof(char_user));

			len = strlen(char_user);
			tmp_account[i] = (char *)malloc(sizeof(char)*(len+1));
			if(tmp_account[i] == NULL){
				usb_dbg("Can't malloc \"tmp_account[%d]\".\n", i);
				free(tmp_account);
				free(nv);
				return -1;
			}
			strcpy(tmp_account[i], char_user);
			tmp_account[i][len] = 0;

			if(i != 0){
				for(j = 0; j < i; ++j)
					tmp_account[j] = tmp_account_list[j];

				free(tmp_account_list);
				tmp_account_list = tmp_account;
			}
			else
				tmp_account_list = tmp_account;

			if(++i >= *acc_num)
				break;
		}
		free(nv);

		*account_list = tmp_account_list;
		*acc_num = i;

		return *acc_num;
	}

	if(nv)
		free(nv);

	return 0;
}
#endif
