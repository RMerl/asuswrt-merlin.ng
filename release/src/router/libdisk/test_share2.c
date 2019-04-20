/*
 * This program is free software; you can redistribute it and/or
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

#define VERSION 1
//#define PC

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/vfs.h>
#include <limits.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <shared.h>

#include <disk_io_tools.h>
//#include <disk_share2.h>

#ifdef DEBUG_USB
#define usb_dbg(fmt, args...) do{ \
		FILE *fp = fopen("/tmp/usb.log", "a+"); \

		if(fp){ \
			fprintf(fp, "[usb_dbg: %s] ", __FUNCTION__); \
			fprintf(fp, fmt, ## args); \
			fclose(fp); \
		} \
	}while(0)
#else
#define usb_dbg printf
#endif


#define ADMIN_ORDER 0
#define MAX_ACCOUNT_NUM 6

#define SHARE_LAYER MOUNT_LAYER+1

// Support Protocol
#define PROTOCOL_CIFS "cifs"
#define PROTOCOL_FTP "ftp"
#define PROTOCOL_MEDIASERVER "dms"
#ifdef RTCONFIG_WEBDAV_OLD
#define PROTOCOL_WEBDAV "webdav"
#define MAX_PROTOCOL_NUM 4
#else
#define MAX_PROTOCOL_NUM 3
#endif

#define PROTOCOL_CIFS_BIT 0
#define PROTOCOL_FTP_BIT 1
#define PROTOCOL_MEDIASERVER_BIT 2
#ifdef RTCONFIG_WEBDAV_OLD
#define PROTOCOL_WEBDAV_BIT 3
#endif

#define DEFAULT_SAMBA_RIGHT 3
#define DEFAULT_FTP_RIGHT 3
#define DEFAULT_DMS_RIGHT 1
#ifdef RTCONFIG_WEBDAV_OLD
#define DEFAULT_WEBDAV_RIGHT 3
#endif

// fake code begin. {
enum {
	TYPE_ACCOUNT,
	TYPE_GROUP,
	TYPE_NONE
};

typedef struct _pms_account_info_t_ PMS_ACCOUNT_INFO_T;
typedef struct _pms_account_group_info_t_ PMS_ACCOUNT_GROUP_INFO_T;

#pragma pack(1) // let struct be neat by byte.
struct _pms_account_info_t_ {
	u32 index;
	u32 active;
	char *name;
	char *passwd;
	char *desc;
	char *email;

	u32 owned_group_num; // the number of owned_group
	PMS_ACCOUNT_GROUP_INFO_T *owned_group;
	PMS_ACCOUNT_INFO_T *next;
};

struct _pms_account_group_info_t_ {
	u32 index;
	u32 active;
	char *name;
	char *desc;

	u32 owned_account_num; // the number of owned_account
	PMS_ACCOUNT_INFO_T *owned_account;
	PMS_ACCOUNT_GROUP_INFO_T *next;
};
#pragma pack() // End.


void PMS_FreeAccountGroupInfo(PMS_ACCOUNT_GROUP_INFO_T **input);
int PMS_ActionAccountGroupInfo(char *action, PMS_ACCOUNT_GROUP_INFO_T **input, int *input_num);
int create_if_no_var_files(const char *const mount_path);
int modify_if_exist_new_folder(const int type, const char *const name, const char *const mount_path);
int test_if_exist_share(const char *const mount_path, const char *const folder);
int initial_folder_list(const char *const mount_path);


void PMS_FreeAccountInfo(PMS_ACCOUNT_INFO_T **input){
	PMS_ACCOUNT_INFO_T *follow_account, *old_account;

	if(input == NULL)
		return;

	if(*input == NULL)
		return;

	follow_account = *input;
	while(follow_account != NULL){
		follow_account->index = (u32)0;
		follow_account->active = (u32)0;
		if(follow_account->name != NULL)
			free(follow_account->name);

		PMS_FreeAccountGroupInfo(&(follow_account->owned_group));

		old_account = follow_account;
		follow_account = follow_account->next;
		free(old_account);
	}
}

PMS_ACCOUNT_INFO_T *PMS_InitialAccountInfo(PMS_ACCOUNT_INFO_T **input){
	PMS_ACCOUNT_INFO_T *follow_account;

	if(input == NULL)
		return NULL;

	*input = (PMS_ACCOUNT_INFO_T *)malloc(sizeof(PMS_ACCOUNT_INFO_T));
	if(*input == NULL)
		return NULL;

	follow_account = *input;

	follow_account->index = (u32)0;
	follow_account->active = (u32)0;
	follow_account->name = NULL;
	follow_account->passwd = NULL;
	follow_account->desc = NULL;
	follow_account->email = NULL;
	follow_account->owned_group_num = (u32)0;
	follow_account->owned_group = NULL;
	follow_account->next = NULL;

	return follow_account;
}

PMS_ACCOUNT_INFO_T *PMS_CreateAccountInfo(PMS_ACCOUNT_INFO_T **account_list, const int index, const int active, const char *account){
	PMS_ACCOUNT_INFO_T *follow_account;
	int len;
	int ret;

	if(account_list == NULL)
		return NULL;

	*account_list = NULL; // initial value.

	if(PMS_InitialAccountInfo(&follow_account) == NULL){
		printf("No memory!!(follow_account)\n");
		return NULL;
	}

	follow_account->index = index;
	follow_account->active = active;

	len = strlen(account);
	follow_account->name = (char *)malloc(len+1);
	if(follow_account->name == NULL){
		printf("No memory!!(follow_account->name)\n");
		PMS_FreeAccountInfo(&follow_account);
		return NULL;
	}
	strcpy(follow_account->name, account);
	follow_account->name[len] = 0;

	if((ret = PMS_ActionAccountGroupInfo("get", &(follow_account->owned_group), 0)) != 0){
		printf("Could not get the group information successfully!\n");
	}

	*account_list = follow_account;

	return *account_list;
}

void PMS_PrintAccountInfo(const PMS_ACCOUNT_INFO_T *const account_list){
	PMS_ACCOUNT_INFO_T *follow_account;
	PMS_ACCOUNT_GROUP_INFO_T *follow_group;

	if(account_list == NULL)
		return;

	for(follow_account = (PMS_ACCOUNT_INFO_T *)account_list; follow_account != NULL; follow_account = follow_account->next){
		printf("Accounts:\n");
		printf(" Index: %u\n", follow_account->index);
		printf("Active: %u\n", follow_account->active);
		printf("  Name: %s\n", follow_account->name);

		for(follow_group = follow_account->owned_group; follow_group != NULL; follow_group = follow_group->next){
			printf("Groups:\n");
			printf(" Index: %u\n", follow_group->index);
			printf("Active: %u\n", follow_group->active);
			printf("  Name: %s\n", follow_group->name);
		}
	}
}

int PMS_ActionAccountInfo(char *action, PMS_ACCOUNT_INFO_T **input, int *input_num){
	int i;
	char fake_account[32];
	PMS_ACCOUNT_INFO_T **follow_account_end;

	if(!strcmp(action, "get")){
		if(PMS_CreateAccountInfo(input, 0, 0, "Tnuocca") == NULL){
			printf("No memory!!(input)\n");
			return -1;
		}
		follow_account_end = &((*input)->next);

		for(i = 0; i < 5; ++i){
			snprintf(fake_account, 32, "Tnuocca%d", i+1);

			if(PMS_CreateAccountInfo(follow_account_end, i+1, (i+1)%2, fake_account) == NULL){
				printf("No memory!!(&((*input)->next)\n");
				return -1;
			}
			follow_account_end = &((*follow_account_end)->next);
		}
	}

	return 5;
}


void PMS_FreeAccountGroupInfo(PMS_ACCOUNT_GROUP_INFO_T **input){
	PMS_ACCOUNT_GROUP_INFO_T *follow_group, *old_group;

	if(input == NULL)
		return;

	if(*input == NULL)
		return;

	follow_group = *input;
	while(follow_group != NULL){
		follow_group->index = (u32)0;
		follow_group->active = (u32)0;
		if(follow_group->name != NULL)
			free(follow_group->name);

		old_group = follow_group;
		follow_group = follow_group->next;
		free(old_group);
	}
}

PMS_ACCOUNT_GROUP_INFO_T *PMS_InitialAccountGroupInfo(PMS_ACCOUNT_GROUP_INFO_T **input){
	PMS_ACCOUNT_GROUP_INFO_T *follow_group;

	if(input == NULL)
		return NULL;

	*input = (PMS_ACCOUNT_GROUP_INFO_T *)malloc(sizeof(PMS_ACCOUNT_GROUP_INFO_T));
	if(*input == NULL)
		return NULL;

	follow_group = *input;

	follow_group->index = (u32)0;
	follow_group->active = (u32)0;
	follow_group->name = NULL;
	follow_group->desc = NULL;
	follow_group->owned_account_num = (u32)0;
	follow_group->owned_account = NULL;
	follow_group->next = NULL;

	return follow_group;
}

PMS_ACCOUNT_GROUP_INFO_T *PMS_CreateAccountGroupInfo(PMS_ACCOUNT_GROUP_INFO_T **group_list, const int index, const int active, const char *group){
	PMS_ACCOUNT_GROUP_INFO_T *follow_group;
	int len;

	if(group_list == NULL)
		return NULL;

	*group_list = NULL; // initial value.

	if(PMS_InitialAccountGroupInfo(&follow_group) == NULL){
		printf("No memory!!(follow_group)\n");
		return NULL;
	}

	follow_group->index = index;
	follow_group->active = active;

	len = strlen(group);
	follow_group->name = (char *)malloc(len+1);
	if(follow_group->name == NULL){
		printf("No memory!!(follow_group->name)\n");
		PMS_FreeAccountGroupInfo(&follow_group);
		return NULL;
	}
	strcpy(follow_group->name, group);
	follow_group->name[len] = 0;


	*group_list = follow_group;

	return *group_list;
}

void PMS_PrintAccountGroupInfo(const PMS_ACCOUNT_GROUP_INFO_T *const group_list){
	PMS_ACCOUNT_GROUP_INFO_T *follow_group;

	if(group_list == NULL)
		return;

	for(follow_group = (PMS_ACCOUNT_GROUP_INFO_T *)group_list; follow_group != NULL; follow_group = follow_group->next){
		printf("\tAccounts:\n");
		printf("\t Index: %u\n", follow_group->index);
		printf("\tActive: %u\n", follow_group->active);
		printf("\t  Name: %s\n", follow_group->name);
	}
}

int PMS_ActionAccountGroupInfo(char *action, PMS_ACCOUNT_GROUP_INFO_T **input, int *input_num){
	int i;
	char fake_group[32];
	PMS_ACCOUNT_GROUP_INFO_T **follow_group_end;

	if(!strcmp(action, "get")){
		if(PMS_CreateAccountGroupInfo(input, 0, 0, "Puorg") == NULL){
			printf("No memory!!(input)\n");
			return -1;
		}
		follow_group_end = &((*input)->next);

		for(i = 0; i < 2; ++i){
			snprintf(fake_group, 32, "Puorg%d", i+1);

			if(PMS_CreateAccountGroupInfo(follow_group_end, i+1, (i+1)%2, fake_group) == NULL){
				printf("No memory!!(&((*input)->next)\n");
				return -1;
			}
			follow_group_end = &((*follow_group_end)->next);
		}
	}

	return 3;
}
// } fake code end.

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
	memset(target_dir, 0, szieof(target_dir));
	strncpy(target_dir, file_name, len);

	if((file_size = f_size(file_name)) == -1){
		usb_dbg("Fail to get the size of the file.\n");
		return;
	}

	snprintf(test_file, szieof(test_file), "%s.%lu", file_name, file_size);
	if((fp = fopen(test_file, "w")) != NULL)
		fclose(fp);

	++ptr;
	snprintf(test_file_name, szieof(test_file_name), "%s.%lu", ptr, file_size);

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

		snprintf(test_path, szieof(test_path), "%s/%s", target_dir, dp->d_name);
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

	snprintf(test_file, szieof(test_file), "%s.%lu", file_name, file_size);
	if(!check_if_file_exist(test_file)){
		usb_dbg("Fail to check the folder list.\n");
		return 0;
	}

	return 1;
}

extern void free_2_dimension_list(int *num, char ***list){
	int i;
	char **target = *list;
	
	if(*num <= 0 || target == NULL){
		*num = 0;
		return;
	}
	
	for (i = 0; i < *num; ++i)
		if(target[i] != NULL)
			free(target[i]);
	
	if(target != NULL)
		free(target);
	
	*num = 0;
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
		free(list_info);
		return -1;
	}
	
	follow_info += strlen("sh_num=");
	follow_info_end = follow_info;
	while (*follow_info_end != 0 && *follow_info_end != '\n')
		++follow_info_end;
	if(*follow_info_end == 0){
		usb_dbg("The content in %s is wrong.\n", list_file);
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
		return 0;
	}
	
	// 5. get folder list from the folder list file
	tmp_folder_list = (char **)malloc(sizeof(char *)*((*sh_num)+1));
	if(tmp_folder_list == NULL){
		usb_dbg("Can't malloc \"tmp_folder_list\".\n");
		free(list_info);
		return -1;
	}
	
	for (i = 0; i < *sh_num; ++i){
		// 6. get folder name
		snprintf(target, szieof(target), "\nsh_name%d=", i);
		follow_info = strstr(list_info, target);
		if(follow_info == NULL){
			usb_dbg("The list content in %s is wrong.\n", mount_path);
			free(list_info);
			free_2_dimension_list(sh_num, &tmp_folder_list);
			return -1;
		}
		
		follow_info += strlen(target);
		follow_info_end = follow_info;
		while (*follow_info_end != 0 && *follow_info_end != '\n')
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
	while ((dp = readdir(pool_to_open)) != NULL){
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
			return -1;
		}
		
		len = strlen(dp->d_name);
		tmp_folder[*sh_num] = (char *)malloc(sizeof(char)*(len+1));
		if(tmp_folder[*sh_num] == NULL){
			usb_dbg("Can't malloc \"tmp_folder[%d]\".\n", *sh_num);
			free(tmp_folder);
			return -1;
		}
		strcpy(tmp_folder[*sh_num], dp->d_name);
		if(*sh_num != 0){
			for (i = 0; i < *sh_num; ++i)
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

extern int get_var_file_name(const int type, const char *const name, const char *const path, char **file_name){
	int len;
	char *var_file;
	char ascii_user[64];

	if(path == NULL)
		return -1;

	len = strlen(path)+strlen("/.___var.txt");
	if(type == TYPE_GROUP)
		++len;

	memset(ascii_user, 0, szieof(ascii_user));
	if(name != NULL){
		char_to_ascii_safe(ascii_user, name, 64);

		len += strlen(ascii_user);
	}
	*file_name = (char *)malloc(sizeof(char)*(len+1));
	if(*file_name == NULL)
		return -1;

	var_file = *file_name;
	if(name != NULL){
		if(type == TYPE_GROUP)
			snprintf(var_file, (len+1), "%s/.__G%s_var.txt", path, ascii_user);
		else
			snprintf(var_file, (len+1), "%s/.__%s_var.txt", path, ascii_user);
	}
	else
		snprintf(var_file, (len+1), "%s/.___var.txt", path);
	var_file[len] = 0;

	return 0;
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
	for (i = 0; i < sh_num; ++i)
		fprintf(fp, "sh_name%d=%s\n", i, folder_list[i]);
	fclose(fp);
	free_2_dimension_list(&sh_num, &folder_list);
	
	// 4. set the check target of file.
	set_file_integrity(list_file);
	free(list_file);
	
	return 0;
}

extern int initial_var_file(const int type, const char *const name, const char *const mount_path){
	FILE *fp;
	char *var_file;
	int result, i;
	int sh_num;
	char **folder_list;
	int samba_right, ftp_right, dms_right;
#ifdef RTCONFIG_WEBDAV_OLD
	int webdav_right;
#endif

	if(mount_path == NULL || strlen(mount_path) <= 0){
		usb_dbg("No input, mount_path\n");
		return -1;
	}

	// 1. get the folder number and folder_list
	//result = get_folder_list(mount_path, &sh_num, &folder_list);
	result = get_all_folder(mount_path, &sh_num, &folder_list);

	// 2. get the var file
	if(get_var_file_name(type, name, mount_path, &var_file)){
		usb_dbg("Can't malloc \"var_file\".\n");
		free_2_dimension_list(&sh_num, &folder_list);
		return -1;
	}

	// 3. get the default permission of all protocol.
#if 0
	if(name == NULL // share mode.
			|| !strcmp(name, "admin")){
		samba_right = DEFAULT_SAMBA_RIGHT;
		ftp_right = DEFAULT_FTP_RIGHT;
		dms_right = DEFAULT_DMS_RIGHT;
#ifdef RTCONFIG_WEBDAV_OLD
		webdav_right = DEFAULT_WEBDAV_RIGHT;
#endif
	}
	else{
		samba_right = 0;
		ftp_right = 0;
		dms_right = 0;
#ifdef RTCONFIG_WEBDAV_OLD
		webdav_right = 0;
#endif
	}
#else
	samba_right = DEFAULT_SAMBA_RIGHT;
	ftp_right = DEFAULT_FTP_RIGHT;
	dms_right = DEFAULT_DMS_RIGHT;
#ifdef RTCONFIG_WEBDAV_OLD
	webdav_right = DEFAULT_WEBDAV_RIGHT;
#endif
#endif

	// 4. write the default content in the var file
	if((fp = fopen(var_file, "w")) == NULL){
		usb_dbg("Can't create the var file, \"%s\".\n", var_file);
		free_2_dimension_list(&sh_num, &folder_list);
		free(var_file);
		return -1;
	}

	for (i = -1; i < sh_num; ++i){
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

extern int initial_all_var_file(const char *const mount_path){
	int result;
	PMS_ACCOUNT_INFO_T *account_list, *follow_account;
	PMS_ACCOUNT_GROUP_INFO_T *group_list, *follow_group;
	DIR *opened_dir;
	struct dirent *dp;

	if(mount_path == NULL || strlen(mount_path) <= 0){
		usb_dbg("No input, mount_path\n");
		return -1;
	}

	// 1. delete all var files
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

		snprintf(test_path, szieof(test_path), "%s/%s", mount_path, dp->d_name);
		usb_dbg("delete %s.\n", test_path);
		delete_file_or_dir(test_path);
	}
	closedir(opened_dir);

	// 2. initial the var file
	if(initial_var_file(TYPE_NONE, NULL, mount_path) != 0) // share mode.
		usb_dbg("Can't initial the var file for the share mode.\n");

	// 3. get the account list
	if((result = PMS_ActionAccountInfo("get", &account_list, 0)) != 0){
		printf("Could not get the account information successfully!\n");
		PMS_FreeAccountInfo(&account_list);
		return -1;
	}

	for(follow_account = account_list; follow_account != NULL; follow_account = follow_account->next){
		if(initial_var_file(TYPE_ACCOUNT, follow_account->name, mount_path) != 0)
			usb_dbg("Can't initial \"%s\"'s file in %s.\n", follow_account->name, mount_path);
	}
	PMS_FreeAccountInfo(&account_list);

	// 4. get the group list
	if((result = PMS_ActionAccountGroupInfo("get", &group_list, 0)) != 0){
		printf("Could not get the group information successfully!\n");
		PMS_FreeAccountGroupInfo(&group_list);
		return -1;
	}

	for(follow_group = group_list; follow_group != NULL; follow_group = follow_group->next){
		if(initial_var_file(TYPE_GROUP, follow_group->name, mount_path) != 0)
			usb_dbg("Can't initial \"%s\"'s file in %s.\n", follow_group->name, mount_path);
	}
	PMS_FreeAccountGroupInfo(&group_list);

	// 5. initial the folder list
	result = initial_folder_list(mount_path);
	if(result != 0)
		usb_dbg("Can't initial the folder list.\n");

	return 0;
}

extern int test_of_var_files(const char *const mount_path){
	create_if_no_var_files(mount_path);	// According to the old folder_list, add the new folder.
	initial_folder_list(mount_path);	// get the new folder_list.
	create_if_no_var_files(mount_path);	// According to the new folder_list, add the new var file.

	return 0;
}

extern int create_if_no_var_files(const char *const mount_path){
	PMS_ACCOUNT_INFO_T *account_list, *follow_account = NULL;
	PMS_ACCOUNT_GROUP_INFO_T *group_list, *follow_group = NULL;
	int acc_num;
	int result;
	char *var_file;

	// 1. get the var_file for the share mode.
	if(get_var_file_name(TYPE_NONE, NULL, mount_path, &var_file)){ // share mode.
		usb_dbg("Can't malloc \"var_file\".\n");
		return -1;
	}

	// 2. test if the var_file is existed and check the file integrity.
	if(!check_if_file_exist(var_file)){
		// 3.1. create the var_file when it's not existed
		if(initial_var_file(TYPE_NONE, NULL, mount_path) != 0)
			usb_dbg("Can't initial the var file for the share mode.\n");
	}
	else if(!check_file_integrity(var_file)){
		// 3.2. check the file integrity.
		usb_dbg("Fail to check the file: %s.\n", var_file);
		if(initial_var_file(TYPE_NONE, NULL, mount_path) != 0){
			usb_dbg("Can't initial the var file for the share mode.\n");
			free(var_file);
			return -1;
		}
	}
	else{
		// 3.3. add the new folder into the var file
		result = modify_if_exist_new_folder(TYPE_NONE, NULL, mount_path);
		if(result != 0)
			usb_dbg("Can't check if there's new folder in %s.\n", mount_path);
	}
	free(var_file);

	// 4. get the account number and account_list
	if((acc_num = PMS_ActionAccountInfo("get", &account_list, 0)) != 0){
		usb_dbg("Could not get the account information successfully!\n");
		PMS_FreeAccountInfo(&account_list);
		return -1;
	}

	// 5. get the var_file of all accounts.
	for(follow_account = account_list; follow_account != NULL; follow_account = follow_account->next){
		if(get_var_file_name(TYPE_ACCOUNT, follow_account->name, mount_path, &var_file)){
			usb_dbg("Can't malloc \"var_file\".\n");
			PMS_FreeAccountInfo(&account_list);
			return -1;
		}
		
		if(!check_if_file_exist(var_file)){
			if(initial_var_file(TYPE_ACCOUNT, follow_account->name, mount_path) != 0){
				usb_dbg("Can't initial \"%s\"'s file in %s.\n", follow_account->name, mount_path);
				PMS_FreeAccountInfo(&account_list);
				free(var_file);
				return -1;
			}
		}
		else if(!check_file_integrity(var_file)){
			// 3.2. check the file integrity.
			usb_dbg("Fail to check the file: %s.\n", var_file);
			if(initial_var_file(TYPE_ACCOUNT, follow_account->name, mount_path) != 0){
				usb_dbg("Can't initial \"%s\"'s file in %s.\n", follow_account->name, mount_path);
				PMS_FreeAccountInfo(&account_list);
				free(var_file);
				return -1;
			}
		}
		else{
			result = modify_if_exist_new_folder(TYPE_ACCOUNT, follow_account->name, mount_path);
			if(result != 0)
				usb_dbg("Can't check if there's new folder in %s.\n", mount_path);
		}
		free(var_file);
	}
	PMS_FreeAccountInfo(&account_list);

	// 6. get the var_file of all groups.
	if((acc_num = PMS_ActionAccountGroupInfo("get", &group_list, 0)) != 0){
		usb_dbg("Could not get the group information successfully!\n");
		PMS_FreeAccountGroupInfo(&group_list);
		return -1;
	}

	for(follow_group = group_list; follow_group != NULL; follow_group = follow_group->next){
		if(initial_var_file(TYPE_GROUP, follow_group->name, mount_path) != 0)
			usb_dbg("Can't initial \"%s\"'s file in %s.\n", follow_group->name, mount_path);
	}
	PMS_FreeAccountGroupInfo(&group_list);

	for(follow_group = group_list; follow_group != NULL; follow_group = follow_group->next){
		if(get_var_file_name(TYPE_GROUP, follow_group->name, mount_path, &var_file)){
			usb_dbg("Can't malloc \"var_file\".\n");
			PMS_FreeAccountGroupInfo(&group_list);
			return -1;
		}
		
		if(!check_if_file_exist(var_file)){
			if(initial_var_file(TYPE_GROUP, follow_group->name, mount_path) != 0){
				usb_dbg("Can't initial \"%s\"'s file in %s.\n", follow_group->name, mount_path);
				PMS_FreeAccountGroupInfo(&group_list);
				free(var_file);
				return -1;
			}
		}
		else if(!check_file_integrity(var_file)){
			// 3.2. check the file integrity.
			usb_dbg("Fail to check the file: %s.\n", var_file);
			if(initial_var_file(TYPE_GROUP, follow_group->name, mount_path) != 0){
				usb_dbg("Can't initial \"%s\"'s file in %s.\n", follow_group->name, mount_path);
				PMS_FreeAccountGroupInfo(&group_list);
				free(var_file);
				return -1;
			}
		}
		else{
			result = modify_if_exist_new_folder(TYPE_GROUP, follow_group->name, mount_path);
			if(result != 0)
				usb_dbg("Can't check if there's new folder in %s.\n", mount_path);
		}
		free(var_file);
	}
	PMS_FreeAccountGroupInfo(&group_list);

	return 0;
}

extern int modify_if_exist_new_folder(const int type, const char *const name, const char *const mount_path){
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
	if(get_var_file_name(type, name, mount_path, &var_file)){
		usb_dbg("Can't malloc \"var_file\".\n");
		return -1;
	}

	// 2. check the file integrity.
	if(!check_file_integrity(var_file)){
		usb_dbg("Fail to check the file: %s.\n", var_file);
		if(initial_var_file(type, name, mount_path) != 0){
			usb_dbg("Can't initial \"%s\"'s file in %s.\n", name, mount_path);
			free(var_file);
			return -1;
		}
	}

	// 3. get all folder in mount_path
	result = get_all_folder(mount_path, &sh_num, &folder_list);
	if(result != 0){
		usb_dbg("Can't get the folder list in \"%s\".\n", mount_path);
		free_2_dimension_list(&sh_num, &folder_list);
		free(var_file);
		return -1;
	}

	for(i = 0; i < sh_num; ++i){
		result = test_if_exist_share(mount_path, folder_list[i]);
		if(result)
			continue;

		// 4. get the target
		len = strlen("*")+strlen(folder_list[i])+strlen("=");
		target = (char *)malloc(sizeof(char)*(len+1));
		if(target == NULL){
			usb_dbg("Can't allocate \"target\".\n");
			free_2_dimension_list(&sh_num, &folder_list);
			free(var_file);
			return -1;
		}
		snprintf(target, (len+1), "*%s=", folder_list[i]);
		target[len] = 0;

		// 5. get the default permission of all protocol.
#if 0
		if(name == NULL // share mode.
				|| !strcmp(name, "admin")){
			samba_right = DEFAULT_SAMBA_RIGHT;
			ftp_right = DEFAULT_FTP_RIGHT;
			dms_right = DEFAULT_DMS_RIGHT;
#ifdef RTCONFIG_WEBDAV_OLD
			webdav_right = DEFAULT_WEBDAV_RIGHT;
#endif
		}
		else{
			samba_right = 0;
			ftp_right = 0;
			dms_right = 0;
#ifdef RTCONFIG_WEBDAV_OLD
			webdav_right = 0;
#endif
		}
#else
		samba_right = DEFAULT_SAMBA_RIGHT;
		ftp_right = DEFAULT_FTP_RIGHT;
		dms_right = DEFAULT_DMS_RIGHT;
#ifdef RTCONFIG_WEBDAV_OLD
		webdav_right = DEFAULT_WEBDAV_RIGHT;
#endif
#endif

		// 6. add the information of the new folder
		fp = fopen(var_file, "a+");
		if(fp == NULL){
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

extern int set_permission(const int type, const char *const name, const char *const mount_path, const char *const folder, const char *const protocol, const int flag){
	FILE *fp;
	char *var_file, *var_info;
	char *target, *follow_info;
	int len;
	
	if(flag < 0 || flag > 3){
		usb_dbg("correct Rights is 0, 1, 2, 3.\n");
		return -1;
	}
	
	// 1. get the var file
	if(get_var_file_name(type, name, mount_path, &var_file)){
		usb_dbg("Can't malloc \"var_file\".\n");
		return -1;
	}
	
	// 2. check the file integrity.
	if(!check_file_integrity(var_file)){
		usb_dbg("Fail to check the file: %s.\n", var_file);
		if(initial_var_file(type, name, mount_path) != 0){
			usb_dbg("Can't initial \"%s\"'s file in %s.\n", name, mount_path);
			free(var_file);
			return -1;
		}
	}
	
	// 3. get the content of the var_file of the account
	var_info = read_whole_file(var_file);
	if(var_info == NULL){
		initial_var_file(type, name, mount_path);
		sleep(1);
		var_info = read_whole_file(var_file);
		if(var_info == NULL){
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
	if(target == NULL){
		usb_dbg("Can't allocate \"target\".\n");
		free(var_file);
		free(var_info);
		
		return -1;
	}
	snprintf(target, (len+1), "*%s=", (folder != NULL)?folder:"");
	target[len] = 0;
	
	// 5. judge if the target is in the var file.
	follow_info = strcasestr(var_info, target);
	if(follow_info == NULL){
		if(name == NULL)
			usb_dbg("No right about \"%s\" with the share mode.\n", (folder == NULL?"Pool":folder));
		else
			usb_dbg("No right about \"%s\" with \"%s\".\n", (folder == NULL?"Pool":folder), name);
		free(var_info);
		
		fp = fopen(var_file, "a+");
		if(fp == NULL){
			usb_dbg("1. Can't rewrite the file, \"%s\".\n", var_file);
			free(var_file);
			return -1;
		}
		free(var_file);
		
		fprintf(fp, "%s", target);
		free(target);
		
		// 6.1 change the right of folder
#ifdef RTCONFIG_WEBDAV_OLD
		if(!strcmp(protocol, PROTOCOL_CIFS))
			fprintf(fp, "%d%d%d%d\n", flag, 0, DEFAULT_DMS_RIGHT, DEFAULT_WEBDAV_RIGHT);
		else if(!strcmp(protocol, PROTOCOL_FTP))
			fprintf(fp, "%d%d%d%d\n", 0, flag, DEFAULT_DMS_RIGHT, DEFAULT_WEBDAV_RIGHT);
		else if(!strcmp(protocol, PROTOCOL_MEDIASERVER))
			fprintf(fp, "%d%d%d%d\n", 0, 0, flag, DEFAULT_WEBDAV_RIGHT);
		else if(!strcmp(protocol, PROTOCOL_WEBDAV))
			fprintf(fp, "%d%d%d%d\n", 0, 0, DEFAULT_DMS_RIGHT, flag);
#else
		if(!strcmp(protocol, PROTOCOL_CIFS))
			fprintf(fp, "%d%d%d\n", flag, 0, DEFAULT_DMS_RIGHT);
		else if(!strcmp(protocol, PROTOCOL_FTP))
			fprintf(fp, "%d%d%d\n", 0, flag, DEFAULT_DMS_RIGHT);
		else if(!strcmp(protocol, PROTOCOL_MEDIASERVER))
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
	if(follow_info[MAX_PROTOCOL_NUM] != '\n'){
		if(name == NULL)
			usb_dbg("The var info is incorrect.\nPlease reset the var file of the share mode.\n");
		else
			usb_dbg("The var info is incorrect.\nPlease reset the var file of \"%s\".\n", name);
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
	if(fp == NULL){
		usb_dbg("2. Can't rewrite the file, \"%s\".\n", var_file);
		free(var_file);
		free(var_info);
		return -1;
	}
	fprintf(fp, "%s", var_info);
	fclose(fp);
	free(var_info);
	
	return 0;
}

extern int get_permission(const int type, const char *const name, const char *const mount_path, const char *const folder, const char *const protocol){
	char *var_file, *var_info;
	char *target, *follow_info;
	int len, result;
	char *f = (char*) folder;

	// 1. get the var file
	if(get_var_file_name(type, name, mount_path, &var_file)){
		usb_dbg("Can't malloc \"var_file\".\n");
		return -1;
	}

	// 2. check the file integrity.
	if(!check_file_integrity(var_file)){
		usb_dbg("Fail to check the file: %s.\n", var_file);
		if(initial_var_file(type, name, mount_path) != 0){
			usb_dbg("Can't initial \"%s\"'s file in %s.\n", name, mount_path);
			free(var_file);
			return -1;
		}
	}

	// 3. get the content of the var_file of the account
	var_info = read_whole_file(var_file);
	if(var_info == NULL){
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
	if(target == NULL){
		usb_dbg("Can't allocate \"target\".\n");
		free(var_info);
		return -1;
	}
	snprintf(target, (len+1), "*%s=", (f != NULL)?f:"");
	target[len] = 0;

	follow_info = strcasestr(var_info, target);
	free(target);
	if(follow_info == NULL){
		if(name == NULL)
			usb_dbg("No right about \"%s\" with the share mode.\n", f? f:"Pool");
		else
			usb_dbg("No right about \"%s\" with \"%s\".\n", f? f:"Pool", name);

		if(f == NULL){
			free(var_info);
			return -1;
		} else {
			f = NULL;
			goto retry_get_permission;
		}
	}

	follow_info += len;

	if(follow_info[MAX_PROTOCOL_NUM] != '\n'){
		if(name == NULL)
			usb_dbg("The var info is incorrect.\nPlease reset the var file of the share mode.\n");
		else
			usb_dbg("The var info is incorrect.\nPlease reset the var file of \"%s\".\n", name);

		free(var_info);
		return -1;
	}

	// 5. get the right of folder
	if(!strcmp(protocol, PROTOCOL_CIFS))
		result = follow_info[0]-'0';
	else if(!strcmp(protocol, PROTOCOL_FTP))
		result = follow_info[1]-'0';
	else if(!strcmp(protocol, PROTOCOL_MEDIASERVER))
		result = follow_info[2]-'0';
#ifdef RTCONFIG_WEBDAV_OLD
	else if(!strcmp(protocol, PROTOCOL_WEBDAV))
		result = follow_info[3]-'0';
#endif
	else{
		usb_dbg("The protocol, \"%s\", is incorrect.\n", protocol);
		free(var_info);
		return -1;
	}
	free(var_info);

	if(result < 0 || result > 3){
		if(name == NULL)
			usb_dbg("The var info is incorrect.\nPlease reset the var file of the share mode.\n");
		else
			usb_dbg("The var info is incorrect.\nPlease reset the var file of \"%s\".\n", name);
		return -1;
	}

	return result;
}

extern int get_permission_all(const char *const account, const char *const mount_path, const char *const folder, const char *const protocol){
	PMS_ACCOUNT_INFO_T *account_list, *follow_account = NULL;
	PMS_ACCOUNT_GROUP_INFO_T *follow_group = NULL;
	int ret;
	int acc_right, gro_right, total_right;

	if((ret = PMS_ActionAccountInfo("get", &account_list, 0)) != 0){
		printf("Could not get the account information successfully!\n");
		PMS_FreeAccountInfo(&account_list);
		return -1;
	}

	for(follow_account = account_list; follow_account != NULL; follow_account = follow_account->next){
		if(!strcmp(follow_account->name, account)){
			break;
		}
	}
	if(follow_account == NULL){
		printf("Could not get the account in the list!\n");
		PMS_FreeAccountInfo(&account_list);
		return -1;
	}

	if((acc_right = get_permission(TYPE_ACCOUNT, account, mount_path, folder, protocol)) < 0){
		PMS_FreeAccountInfo(&account_list);
		return -1;
	}
	total_right = acc_right;

	for(follow_group = follow_account->owned_group; follow_group != NULL; follow_group = follow_group->next){
		if((gro_right = get_permission(TYPE_GROUP, follow_group->name, mount_path, folder, protocol)) < 0){
			PMS_FreeAccountInfo(&account_list);
			return total_right;
		}

		if(total_right > gro_right)
			total_right = gro_right;

		if(total_right == 0)
			break;
	}
	PMS_FreeAccountInfo(&account_list);

	return total_right;
}

extern int add_folder(const char *const account, const char *const mount_path, const char *const folder){
	PMS_ACCOUNT_INFO_T *account_list, *follow_account = NULL;
	PMS_ACCOUNT_GROUP_INFO_T *group_list, *follow_group = NULL;
	int result, i, len;
	int acc_num;
	char *var_file, *var_info;
	char *target;
	FILE *fp;
	int samba_right, ftp_right, dms_right;
#ifdef RTCONFIG_WEBDAV_OLD
	int webdav_right;
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

	// 1. test if creatting the folder
	result = test_if_exist_share(mount_path, folder);
	if(result != 0){
		usb_dbg("\"%s\" is already created in %s.\n", folder, mount_path);
		return -1;
	}

	// 2. create the folder
	len = strlen(mount_path)+strlen("/")+strlen(folder);
	full_path = (char *)malloc(sizeof(char)*(len+1));
	if(full_path == NULL){
		usb_dbg("Can't malloc \"full_path\".\n");
		return -1;
	}
	snprintf(full_path, (len+1), "%s/%s", mount_path, folder);
	full_path[len] = 0;

	umask(0000);
	result = mkdir(full_path, 0777);
	free(full_path);
	if(result != 0){
		usb_dbg("To create \"%s\" is failed!\n", folder);
		return -1;
	}

	len = strlen("*")+strlen(folder)+strlen("=");
	target = (char *)malloc(sizeof(char)*(len+1));
	if(target == NULL){
		usb_dbg("Can't allocate \"target\".\n");
		return -1;
	}
	snprintf(target, (len+1), "*%s=", folder);
	target[len] = 0;

	// 3. add folder's right to every var file	
	if((acc_num = PMS_ActionAccountInfo("get", &account_list, 0)) != 0){
		usb_dbg("Can't get the account list\n");
		PMS_FreeAccountInfo(&account_list);
		free(target);
		return -1;
	}

	for(i = -1, follow_account = account_list; i < acc_num; ++i, follow_account = follow_account->next){
		// 4. get the var file
		if(i == -1) // share mode.
			result = get_var_file_name(TYPE_NONE, NULL, mount_path, &var_file);
		else
			result = get_var_file_name(TYPE_ACCOUNT, follow_account->name, mount_path, &var_file);
		if(result){
			usb_dbg("Can't malloc \"var_file\".\n");
			PMS_FreeAccountInfo(&account_list);
			free(target);
			return -1;
		}

		// 5. check the file integrity.
		if(!check_file_integrity(var_file)){
			usb_dbg("Fail to check the file: %s.\n", var_file);
			if(i == -1) // share mode.
				result = initial_var_file(TYPE_NONE, NULL, mount_path);
			else
				result = initial_var_file(TYPE_ACCOUNT, follow_account->name, mount_path);
			if(result != 0){
				usb_dbg("Can't initial the file in %s.\n", mount_path);
				PMS_FreeAccountInfo(&account_list);
				free(target);
				free(var_file);
				return -1;
			}
		}

		// 6. check if the created target is exist in the var file
		var_info = read_whole_file(var_file);
		if(var_info == NULL){
			usb_dbg("add_folder: \"%s\" isn't existed or there's no content.\n", var_file);
		}
		else if(strcasestr(var_info, target) != NULL){
			free(var_file);
			free(var_info);
			continue;
		}
		else
			free(var_info);

		// 7. add the folder's info in the var file
		fp = fopen(var_file, "a+");
		if(fp == NULL){
			usb_dbg("Can't write \"%s\".\n", var_file);
			PMS_FreeAccountInfo(&account_list);
			free(target);
			free(var_file);
			return -1;
		}

		// 8. get the default permission of all protocol.
#if 0
		if(i == -1 // share mode.
				|| !strcmp(follow_account->name, "admin")
				){
			samba_right = DEFAULT_SAMBA_RIGHT;
			ftp_right = DEFAULT_FTP_RIGHT;
			dms_right = DEFAULT_DMS_RIGHT;
#ifdef RTCONFIG_WEBDAV_OLD
			webdav_right = DEFAULT_WEBDAV_RIGHT;
#endif
		}
		else if(account != NULL && !strcmp(follow_account->name, account)){
			samba_right = DEFAULT_SAMBA_RIGHT;
			ftp_right = DEFAULT_FTP_RIGHT;
			dms_right = DEFAULT_DMS_RIGHT;
#ifdef RTCONFIG_WEBDAV_OLD
			webdav_right = DEFAULT_WEBDAV_RIGHT;
#endif
		}
		else{
			samba_right = 0;
			ftp_right = 0;
			dms_right = 0;
#ifdef RTCONFIG_WEBDAV_OLD
			webdav_right = 0;
#endif
		}
#else
		samba_right = DEFAULT_SAMBA_RIGHT;
		ftp_right = DEFAULT_FTP_RIGHT;
		dms_right = DEFAULT_DMS_RIGHT;
#ifdef RTCONFIG_WEBDAV_OLD
		webdav_right = DEFAULT_WEBDAV_RIGHT;
#endif
#endif

#ifdef RTCONFIG_WEBDAV_OLD
		fprintf(fp, "%s%d%d%d%d\n", target, samba_right, ftp_right, dms_right, webdav_right);
#else
		fprintf(fp, "%s%d%d%d\n", target, samba_right, ftp_right, dms_right);
#endif
		fclose(fp);

		// 9. set the check target of file.
		set_file_integrity(var_file);
		free(var_file);
	}
	PMS_FreeAccountInfo(&account_list);

	if((acc_num = PMS_ActionAccountGroupInfo("get", &group_list, 0)) != 0){
		printf("Could not get the group information successfully!\n");
		PMS_FreeAccountGroupInfo(&group_list);
		free(target);
		return -1;
	}

	for(follow_group = group_list; follow_group != NULL; follow_group = follow_group->next){
		// 4. get the var file
		result = get_var_file_name(TYPE_GROUP, follow_group->name, mount_path, &var_file);
		if(result){
			usb_dbg("Can't malloc \"var_file\".\n");
			PMS_FreeAccountGroupInfo(&group_list);
			free(target);
			return -1;
		}

		// 5. check the file integrity.
		if(!check_file_integrity(var_file)){
			usb_dbg("Fail to check the file: %s.\n", var_file);
			result = initial_var_file(TYPE_GROUP, follow_group->name, mount_path);
			if(result != 0){
				usb_dbg("Can't initial the file in %s.\n", mount_path);
				PMS_FreeAccountGroupInfo(&group_list);
				free(target);
				free(var_file);
				return -1;
			}
		}

		// 6. check if the created target is exist in the var file
		var_info = read_whole_file(var_file);
		if(var_info == NULL){
			usb_dbg("add_folder: \"%s\" isn't existed or there's no content.\n", var_file);
		}
		else if(strcasestr(var_info, target) != NULL){
			free(var_file);
			free(var_info);
			continue;
		}
		else
			free(var_info);

		// 7. add the folder's info in the var file
		fp = fopen(var_file, "a+");
		if(fp == NULL){
			usb_dbg("Can't write \"%s\".\n", var_file);
			PMS_FreeAccountGroupInfo(&group_list);
			free(target);
			free(var_file);
			return -1;
		}

		// 8. get the default permission of all protocol.
#if 0
		if(i == -1 // share mode.
				|| !strcmp(follow_group->name, "admin")
				){
			samba_right = DEFAULT_SAMBA_RIGHT;
			ftp_right = DEFAULT_FTP_RIGHT;
			dms_right = DEFAULT_DMS_RIGHT;
#ifdef RTCONFIG_WEBDAV_OLD
			webdav_right = DEFAULT_WEBDAV_RIGHT;
#endif
		}
		else if(account != NULL && !strcmp(follow_group->name, account)){
			samba_right = DEFAULT_SAMBA_RIGHT;
			ftp_right = DEFAULT_FTP_RIGHT;
			dms_right = DEFAULT_DMS_RIGHT;
#ifdef RTCONFIG_WEBDAV_OLD
			webdav_right = DEFAULT_WEBDAV_RIGHT;
#endif
		}
		else{
			samba_right = 0;
			ftp_right = 0;
			dms_right = 0;
#ifdef RTCONFIG_WEBDAV_OLD
			webdav_right = 0;
#endif
		}
#else
		samba_right = DEFAULT_SAMBA_RIGHT;
		ftp_right = DEFAULT_FTP_RIGHT;
		dms_right = DEFAULT_DMS_RIGHT;
#ifdef RTCONFIG_WEBDAV_OLD
		webdav_right = DEFAULT_WEBDAV_RIGHT;
#endif
#endif

#ifdef RTCONFIG_WEBDAV_OLD
		fprintf(fp, "%s%d%d%d%d\n", target, samba_right, ftp_right, dms_right, webdav_right);
#else
		fprintf(fp, "%s%d%d%d\n", target, samba_right, ftp_right, dms_right);
#endif
		fclose(fp);

		// 9. set the check target of file.
		set_file_integrity(var_file);
		free(var_file);
	}
	PMS_FreeAccountGroupInfo(&group_list);

	// 10. add the folder's info in the folder list
	initial_folder_list(mount_path);

	free(target);

	return 0;
}

extern int del_folder(const char *const mount_path, const char *const folder){
	PMS_ACCOUNT_INFO_T *account_list, *follow_account = NULL;
	PMS_ACCOUNT_GROUP_INFO_T *group_list, *follow_group = NULL;
	int result, i, len;
	int acc_num;
	char *var_file, *var_info;
	char *follow_info, backup;
	char *target;
	FILE *fp;
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

	// 3. get the target which is deleted in every var file
	len = strlen("*")+strlen(folder)+strlen("=");
	target = (char *)malloc(sizeof(char)*(len+1));
	if(target == NULL){
		usb_dbg("Can't allocate \"target\".\n");
		return -1;
	}
	snprintf(target, (len+1), "*%s=", folder);
	target[len] = 0;

	// 4. del folder's right to every var file
	if((acc_num = PMS_ActionAccountInfo("get", &account_list, 0)) != 0){
		usb_dbg("Can't get the account list\n");
		PMS_FreeAccountInfo(&account_list);
		free(target);
		return -1;
	}

	for(i = -1, follow_account = account_list; i < acc_num; ++i, follow_account = follow_account->next){
		// 5. get the var file
		if(i == -1) // share mode.
			result = get_var_file_name(TYPE_NONE, NULL, mount_path, &var_file);
		else
			result = get_var_file_name(TYPE_ACCOUNT, follow_account->name, mount_path, &var_file);
		if(result){
			usb_dbg("Can't malloc \"var_file\".\n");
			PMS_FreeAccountInfo(&account_list);
			free(target);
			return -1;
		}

		// 6. check the file integrity.
		if(!check_file_integrity(var_file)){
			usb_dbg("Fail to check the file: %s.\n", var_file);
			if(i == -1) // share mode.
				result = initial_var_file(TYPE_NONE, NULL, mount_path);
			else
				result = initial_var_file(TYPE_ACCOUNT, follow_account->name, mount_path);
			if(result != 0){
				usb_dbg("Can't initial the file in %s.\n", mount_path);
				PMS_FreeAccountInfo(&account_list);
				free(target);
				free(var_file);
				return -1;
			}
		}

		// 7. delete the content about the folder
		var_info = read_whole_file(var_file);
		if(var_info == NULL){
			usb_dbg("del_folder: \"%s\" isn't existed or there's no content.\n", var_file);
			free(var_file);
			continue;
		}

		follow_info = strcasestr(var_info, target);
		if(follow_info == NULL){
			if(i == -1)
				usb_dbg("No right about \"%s\" of the share mode.\n", folder);
			else
				usb_dbg("No right about \"%s\" with \"%s\".\n", folder, follow_account->name);
			free(var_file);
			free(var_info);
			continue;
		}
		backup = *follow_info;
		*follow_info = 0;

		fp = fopen(var_file, "w");
		if(fp == NULL){
			usb_dbg("Can't write \"%s\".\n", var_file);
			*follow_info = backup;
			free(var_file);
			free(var_info);
			continue;
		}
		fprintf(fp, "%s", var_info);

		*follow_info = backup;
		while(*follow_info != 0 && *follow_info != '\n')
			++follow_info;
		if(*follow_info != 0 && *(follow_info+1) != 0){
			++follow_info;
			fprintf(fp, "%s", follow_info);
		}
		fclose(fp);
		free(var_info);

		// 8. set the check target of file.
		set_file_integrity(var_file);
		free(var_file);
	}
	PMS_FreeAccountInfo(&account_list);

	if((acc_num = PMS_ActionAccountGroupInfo("get", &group_list, 0)) != 0){
		usb_dbg("Can't get the group list\n");
		PMS_FreeAccountGroupInfo(&group_list);
		free(target);
		return -1;
	}

	for(follow_group = group_list; follow_group != NULL; follow_group = follow_group->next){
		// 5. get the var file
		result = get_var_file_name(TYPE_GROUP, follow_group->name, mount_path, &var_file);
		if(result){
			usb_dbg("Can't malloc \"var_file\".\n");
			PMS_FreeAccountGroupInfo(&group_list);
			free(target);
			return -1;
		}

		// 6. check the file integrity.
		if(!check_file_integrity(var_file)){
			usb_dbg("Fail to check the file: %s.\n", var_file);
			result = initial_var_file(TYPE_GROUP, follow_group->name, mount_path);
			if(result != 0){
				usb_dbg("Can't initial the file in %s.\n", mount_path);
				PMS_FreeAccountGroupInfo(&group_list);
				free(target);
				free(var_file);
				return -1;
			}
		}

		// 7. delete the content about the folder
		var_info = read_whole_file(var_file);
		if(var_info == NULL){
			usb_dbg("del_folder: \"%s\" isn't existed or there's no content.\n", var_file);
			free(var_file);
			continue;
		}

		follow_info = strcasestr(var_info, target);
		if(follow_info == NULL){
			if(i == -1)
				usb_dbg("No right about \"%s\" of the share mode.\n", folder);
			else
				usb_dbg("No right about \"%s\" with \"%s\".\n", folder, follow_group->name);
			free(var_file);
			free(var_info);
			continue;
		}
		backup = *follow_info;
		*follow_info = 0;

		fp = fopen(var_file, "w");
		if(fp == NULL){
			usb_dbg("Can't write \"%s\".\n", var_file);
			*follow_info = backup;
			free(var_file);
			free(var_info);
			continue;
		}
		fprintf(fp, "%s", var_info);

		*follow_info = backup;
		while(*follow_info != 0 && *follow_info != '\n')
			++follow_info;
		if(*follow_info != 0 && *(follow_info+1) != 0){
			++follow_info;
			fprintf(fp, "%s", follow_info);
		}
		fclose(fp);
		free(var_info);

		// 8. set the check target of file.
		set_file_integrity(var_file);
		free(var_file);
	}
	PMS_FreeAccountGroupInfo(&group_list);

	// 9. modify the folder's info in the folder list
	initial_folder_list(mount_path);

	free(target);

	return 0;
}

extern int mod_folder(const char *const mount_path, const char *const folder, const char *const new_folder){
	PMS_ACCOUNT_INFO_T *account_list, *follow_account = NULL;
	PMS_ACCOUNT_GROUP_INFO_T *group_list, *follow_group = NULL;
	int result, i, len;
	int acc_num;
	char *var_file, *var_info;
	char *target, *new_target;
	FILE *fp;
	char *follow_info, backup;
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
	len = strlen("*")+strlen(folder)+strlen("=");
	target = (char *)malloc(sizeof(char)*(len+1));
	if(target == NULL){
		usb_dbg("Can't allocate \"target\".\n");
		return -1;
	}
	snprintf(target, (len+1), "*%s=", folder);
	target[len] = 0;

	len = strlen("*")+strlen(new_folder)+strlen("=");
	new_target = (char *)malloc(sizeof(char)*(len+1));
	if(new_target == NULL){
		usb_dbg("Can't allocate \"new_target\".\n");
		free(target);
		return -1;
	}
	snprintf(new_target, (len+1), "*%s=", new_folder);
	new_target[len] = 0;

	if((acc_num = PMS_ActionAccountInfo("get", &account_list, 0)) != 0){
		usb_dbg("Can't get the account list\n");
		PMS_FreeAccountInfo(&account_list);
		free(target);
		free(new_target);
		return -1;
	}

	for(i = -1, follow_account = account_list; i < acc_num; ++i, follow_account = follow_account->next){
		// 5. get the var file
		if(i == -1) // share mode.
			result = get_var_file_name(TYPE_NONE, NULL, mount_path, &var_file);
		else
			result = get_var_file_name(TYPE_ACCOUNT, follow_account->name, mount_path, &var_file);
		if(result){
			usb_dbg("Can't malloc \"var_file\".\n");
			PMS_FreeAccountInfo(&account_list);
			free(target);
			free(new_target);
			return -1;
		}

		// 6. check the file integrity.
		if(!check_file_integrity(var_file)){
			usb_dbg("Fail to check the file: %s.\n", var_file);
			if(i == -1) // share mode.
				result = initial_var_file(TYPE_NONE, NULL, mount_path);
			else
				result = initial_var_file(TYPE_ACCOUNT, follow_account->name, mount_path);
			if(result != 0){
				usb_dbg("Can't initial the file in %s.\n", mount_path);
				free(var_file);
				PMS_FreeAccountInfo(&account_list);
				free(target);
				free(new_target);
				return -1;
			}
		}

		// 7. check if the created target is exist in the var file
		var_info = read_whole_file(var_file);
		if(var_info == NULL){
			usb_dbg("mod_folder: \"%s\" isn't existed or there's no content.\n", var_file);
			PMS_FreeAccountInfo(&account_list);
			free(target);
			free(new_target);
			free(var_file);
			return -1;
		}

		if((follow_info = strcasestr(var_info, target)) == NULL){
			usb_dbg("1. No \"%s\" in \"%s\"..\n", folder, var_file);
			PMS_FreeAccountInfo(&account_list);
			free(target);
			free(new_target);
			free(var_file);
			free(var_info);
			return -1;
		}

		// 8. modify the folder's info in the var file
		fp = fopen(var_file, "w");
		if(fp == NULL){
			usb_dbg("Can't write \"%s\".\n", var_file);
			PMS_FreeAccountInfo(&account_list);
			free(target);
			free(new_target);
			free(var_file);
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

		// 9. set the check target of file.
		set_file_integrity(var_file);
		free(var_file);
	}
	PMS_FreeAccountInfo(&account_list);

	if((acc_num = PMS_ActionAccountGroupInfo("get", &group_list, 0)) != 0){
		usb_dbg("Can't get the group list\n");
		PMS_FreeAccountGroupInfo(&group_list);
		free(target);
		free(new_target);
		return -1;
	}

	for(follow_group = group_list; follow_group != NULL; follow_group = follow_group->next){
		// 5. get the var file
		result = get_var_file_name(TYPE_GROUP, follow_group->name, mount_path, &var_file);
		if(result){
			usb_dbg("Can't malloc \"var_file\".\n");
			PMS_FreeAccountGroupInfo(&group_list);
			free(target);
			free(new_target);
			return -1;
		}

		// 6. check the file integrity.
		if(!check_file_integrity(var_file)){
			usb_dbg("Fail to check the file: %s.\n", var_file);
			result = initial_var_file(TYPE_GROUP, follow_group->name, mount_path);
			if(result != 0){
				usb_dbg("Can't initial the file in %s.\n", mount_path);
				free(var_file);
				PMS_FreeAccountGroupInfo(&group_list);
				free(target);
				free(new_target);
				return -1;
			}
		}

		// 7. check if the created target is exist in the var file
		var_info = read_whole_file(var_file);
		if(var_info == NULL){
			usb_dbg("mod_folder: \"%s\" isn't existed or there's no content.\n", var_file);
			PMS_FreeAccountGroupInfo(&group_list);
			free(target);
			free(new_target);
			free(var_file);
			return -1;
		}

		if((follow_info = strcasestr(var_info, target)) == NULL){
			usb_dbg("1. No \"%s\" in \"%s\"..\n", folder, var_file);
			PMS_FreeAccountGroupInfo(&group_list);
			free(target);
			free(new_target);
			free(var_file);
			free(var_info);
			return -1;
		}

		// 8. modify the folder's info in the var file
		fp = fopen(var_file, "w");
		if(fp == NULL){
			usb_dbg("Can't write \"%s\".\n", var_file);
			PMS_FreeAccountGroupInfo(&group_list);
			free(target);
			free(new_target);
			free(var_file);
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

		// 9. set the check target of file.
		set_file_integrity(var_file);
		free(var_file);
	}
	PMS_FreeAccountGroupInfo(&group_list);

	// 10. modify the folder's info in the folder list
	initial_folder_list(mount_path);

	free(target);
	free(new_target);

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
	for (i = 0; i < sh_num; ++i)
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
	int i;

	*mount_path = NULL;
	*share = NULL;

	if(!strcmp(basedir, "/"))
		return layer;

	len = strlen(basedir);
	if(len > 1)
		layer = 1;

	i = len-1;
	while(basedir[i] == '/'){
		--layer;
		--i;
	}

	follow_info = (char *)basedir;
	while (*follow_info != 0 && (follow_info = index(follow_info+1, '/')) != NULL)
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

int main(int argc, char *argv[]){
	char *command;
	int ret;
	int right;
	char *mount_path, *share;

	usb_dbg("%d: Using myself to get information:\n", VERSION);

	if((command = rindex(argv[0], '/')) != NULL)
		++command;
	else
		command = argv[0];

	if(!strcmp(command, "initial_all_var_file")){
		if(argc != 2)
			usb_dbg("Usage: initial_all_var_file mount_path\n");
		else if((ret = initial_all_var_file(argv[1])) != 0)
			usb_dbg("Can't initial all permission files in %s.\n", argv[1]);
		else
			usb_dbg("done.\n");
	}
	else if(!strcmp(command, "test_of_var_files")){
		if(argc != 2)
			usb_dbg("Usage: test_of_var_files mount_path\n");
		else if(test_of_var_files(argv[1]) < 0)
			usb_dbg("Can't test_of_var_files in %s.\n", argv[1]);
		else
			usb_dbg("done.\n");
	}else if(!strcmp(command, "set_permission")){
		if(argc != 7)
			usb_dbg("Usage: set_permission type name mount_path folder [cifs|ftp|dms] [0~3]\n");
		else if((right = set_permission(atoi(argv[1]), argv[2], argv[3], argv[4], argv[5], atoi(argv[6]))) != 0)
			usb_dbg("%s can't set %s's %s permission in %s.\n", argv[2], argv[4], argv[5], argv[3]);
		else
			usb_dbg("done.\n");
	}
	else if(!strcmp(command, "get_permission")){
		if(argc != 6)
			usb_dbg("Usage: get_permission type name mount_path folder [cifs|ftp|dms]\n");
		else if((right = get_permission(atoi(argv[1]), argv[2], argv[3], argv[4], argv[5])) < 0)
			usb_dbg("%s can't get %s's %s permission in %s.\n", argv[2], argv[4], argv[5], argv[3]);
		else
			usb_dbg("done: %d.\n", right);
	}
	else if(!strcmp(command, "get_permission_all")){
		if(argc != 5)
			usb_dbg("Usage: get_permission_all name mount_path folder [cifs|ftp|dms]\n");
		else if((right = get_permission_all(argv[1], argv[2], argv[3], argv[4])) < 0)
			usb_dbg("%s can't get %s's %s full permission in %s.\n", argv[2], argv[4], argv[5], argv[3]);
		else
			usb_dbg("done: %d.\n", right);
	}
	else if(!strcmp(command, "print_account")){
		PMS_ACCOUNT_INFO_T *account_list;

		if((ret = PMS_ActionAccountInfo("get", &account_list, 0)) != 0){
			printf("Could not get the account information successfully!\n");
			return -1;
		}

		PMS_PrintAccountInfo(account_list);

		PMS_FreeAccountInfo(&account_list);
	}
	else if(!strcmp(command, "print_group")){
		PMS_ACCOUNT_GROUP_INFO_T *group_list;

		if((ret = PMS_ActionAccountGroupInfo("get", &group_list, 0)) != 0){
			printf("Could not get the group information successfully!\n");
			return -1;
		}

		PMS_PrintAccountGroupInfo(group_list);

		PMS_FreeAccountGroupInfo(&group_list);
	}
	else if(!strcmp(command, "add_folder")){
		if(argc != 4){
			usb_dbg("Usage: add_folder account mount_path folder\n");
			return 0;
		}

		if(!strcmp(argv[1], "NULL"))
			ret = add_folder(NULL, argv[2], argv[3]);
		else
			ret = add_folder(argv[1], argv[2], argv[3]);
		if(ret < 0)
			usb_dbg("Can't add folder(%s) in %s.\n", argv[3], argv[2]);
		else
			usb_dbg("done.\n");
	}
	else if(!strcmp(command, "del_folder")){
		if(argc != 3)
			usb_dbg("Usage: del_folder mount_path folder\n");
		else if(del_folder(argv[1], argv[2]) < 0)
			usb_dbg("Can't del folder(%s) in %s.\n", argv[2], argv[1]);
		else
			usb_dbg("done.\n");
	}
	else if(!strcmp(command, "mod_folder")){
		if(argc != 4)
			usb_dbg("Usage: mod_folder mount_path folder new_folder\n");
		else if(mod_folder(argv[1], argv[2], argv[3]) < 0)
			usb_dbg("Can't mod folder(%s) to (%s) in %s.\n", argv[2], argv[3], argv[1]);
		else
			usb_dbg("done.\n");
	}
	else if(!strcmp(command, "test_if_exist_share")){
		if(argc != 3)
			usb_dbg("Usage: test_if_exist_share mount_path folder\n");
		else if(test_if_exist_share(argv[1], argv[2]))
			usb_dbg("%s is existed in %s.\n", argv[2], argv[1]);
		else
			usb_dbg("%s is NOT existed in %s.\n", argv[2], argv[1]);
	}
	else if(!strcmp(command, "how_many_layer")){
		if(argc != 2)
			usb_dbg("Usage: how_many_layer path\n");
		else if((ret = how_many_layer(argv[1], &mount_path, &share)) < 0)
			usb_dbg("Can't count the layer with %s.\n", argv[1]);
		else
			usb_dbg("done: %d layers, share=%s, mount_path=%s.\n", ret, share, mount_path);
	}

	return 0;
}
