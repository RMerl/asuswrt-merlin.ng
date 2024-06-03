

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>

#include <curl/curl.h>
#include <sys/stat.h>


#include "upload_api.h"
#include "log.h"
#include "json.h"
#include "api.h"

#include "upload_util.h"

#include "curl_api.h"

#include <shared.h>

#include <aae_ipc.h>

#include "webapi.h"

#define API_DBG 1

extern char g_formated_router_mac[32];

backup_file_types BACK_FILE_TYPES[] = {
    { TYPE_SETTING, SETTING_BACKUP_TYPE, NULL, 0, CLOUD_FILE_PREFIX_NAME_SETTING, CLOUD_FILE_EXT_CFG, 3 },
    { TYPE_IPSEC, IPSEC_BACKUP_TYPE, NULL, 0, CLOUD_FILE_PREFIX_NAME_IPSEC, CLOUD_FILE_EXT_COMPRESS, 3 },
    { TYPE_OPENVPN, OPENVPN_BACKUP_TYPE, NULL, 0, CLOUD_FILE_PREFIX_NAME_OPENVPN, CLOUD_FILE_EXT_COMPRESS, 3, },
    { TYPE_USERICON, USERICON_BACKUP_TYPE, NULL, 0, CLOUD_FILE_PREFIX_NAME_USERICON, CLOUD_FILE_EXT_COMPRESS, 3 },
    { TYPE_UISUPORT, UISUPORT_BACKUP_TYPE, NULL, 0, CLOUD_FILE_PREFIX_NAME_UISUPORT, CLOUD_FILE_EXT_JSON, 1 },
    { TYPE_AMASCNTRL, AMASCNTRL_BACKUP_TYPE, NULL, 0, CLOUD_FILE_PREFIX_NAME_AMASCNTRL, CLOUD_FILE_EXT_COMPRESS, 3 },
    { -1, "", NULL, 0, "", "", 0 }
};

int g_backup_db_number_tmp = 0;
////////////////////////////////////////////////////////////////////////////////

size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userp) {

	// ptr must be filled data fully , and sent to curl socket buffer
  	struct input_info* pooh = (struct input_info*)userp;

  	if(size*nmemb < 1)
		return 0;

  	if(pooh->sizeleft) {
		*(char *)ptr = pooh->readptr[0]; /* copy one single byte */ 
		pooh->readptr++;         /* advance pointer */
		pooh->sizeleft--;        /* less data left */
		return 1;            /* we return 1 byte at a time! */
  	}

  	return 0;              /* no more data left to deliver */
}

/* curl write callback, write data from curl socket to libxml2 text buffer  */ 
size_t write_cb(char *in, size_t size, size_t nmemb, void* cb_data) {

	size_t      r = size * nmemb;  
  	RWCB* rwcb = (RWCB*)cb_data;
  	char* buff = (char*)rwcb->write_data;
  	if (buff) {
		size_t cb_org_size = strlen(buff);
		size_t total_size = cb_org_size +r+1;
		
		char* new_cb_data = (char*)malloc(total_size);
		
		memset(new_cb_data, 0, total_size);
		
		strncpy(new_cb_data, buff, cb_org_size);
		
		strncat(new_cb_data, in, r);
		
		if(buff) free(buff);
		buff = new_cb_data;
  	}
  	else {
		buff = (char*)malloc(r+1);
		memset(buff, 0, r+1);
		strncpy(buff, in, r);
  	}

  	rwcb->write_data = buff;

  	return(r);
}
////////////////////////////////////////////////////////////////////////////////

int is_router_cloud_file_format(const char* filename) {
	if (strstr(filename, g_formated_router_mac)==NULL) {
		return -1;
	}

	return 0;
}

int is_allowed_cloud_file_format(const char* filename) {

	if (is_router_cloud_file_format(filename)!=0) {
		return -1;
	}

	int count = 0;
	char* token;
	char* copy = strdup(filename);
	if (copy!=NULL) {
		token = strtok(copy, "_");
	    while (token != NULL) {
	        token = strtok(NULL, "_");
	        if (token!=NULL) {
	        	count++;
	    	}
	    }

	    free(copy);
	}

	return (count==3) ? 0 : -1;
}

int is_oldest_cloud_file_format(const char *filename) {

	if (is_router_cloud_file_format(filename)!=0) {
		return -1;
	}

	//- special case
	if (strncmp(filename, "ui_support", 10)==0) {
		return 0;
	}

	int count = 0;
	char* token;
	char* copy = strdup(filename);
	if (copy!=NULL) {
		token = strtok(copy, "_");
	    while (token != NULL) {
	        token = strtok(NULL, "_");
	        if (token!=NULL) {
	        	count++;
	    	}
	    }

	    free(copy);
	}

	return (count==2) ? 0 : -1;
}

int generate_backup_file(backup_file_types *handler, char *filename, int update_time) {
	
	int ret = -1;

	if (handler==NULL) {
		return -1;
	}

	int uploader_reset_time = nvram_get_int("uploader_reset_time");
	if (uploader_reset_time<=0) {
		return -1;
	}

	if (update_time<uploader_reset_time) {
		return -1;
	}
	
	int offset_time = update_time - uploader_reset_time;

	//- format: {type}_{formated router mac}_{uploader reset time}_{upload offset time}.{file_ext}
	snprintf(filename, MAX_FILENAME_LEN, "%s_%s_%d_%d.%s", handler->bf_name, g_formated_router_mac, uploader_reset_time, offset_time, handler->bf_ext);

	DECLARE_CLEAR_MEM(char, filepath, MAX_FILEPATH_LEN);
	snprintf(filepath, sizeof(filepath), "%s%s", UPLOADER_FOLDER, filename);

	DECLARE_CLEAR_MEM(char, cmd, 128);

	switch (handler->id) {

		case TYPE_SETTING:
			snprintf(cmd, sizeof(cmd), "nvram save %s", filepath);
			ret = system(cmd);
			break;

		case TYPE_IPSEC:
			//- tar file
			if ((ret = gen_server_ipsec_file()) == HTTP_OK){
      			snprintf(cmd, sizeof(cmd), "mv %s %s", IPSEC_EXPORT_FILE, filepath);
      			ret = system(cmd);
    		}
			else {
				Cdbg(API_DBG, "Fail gen_server_ovpn_file, IPSEC_EXPORT_FILE=%s", IPSEC_EXPORT_FILE);
				return -1;
			}

			break;

		case TYPE_OPENVPN:
			//- tar file
			if ((ret = gen_server_ovpn_file()) == HTTP_OK){
      			snprintf(cmd, sizeof(cmd), "mv %s %s", OPENVPN_EXPORT_FILE, filepath);
      			ret = system(cmd);
    		}
			else {
				Cdbg(API_DBG, "Fail gen_server_ovpn_file, OPENVPN_EXPORT_FILE=%s", OPENVPN_EXPORT_FILE);
				return -1;
			}

			break;

		case TYPE_USERICON:
    		if ((ret = gen_jffs_backup_profile("usericon", filepath)) == HTTP_OK) {
    			ret = 0;
    		}
    		else {
    			Cdbg(API_DBG, "Fail gen_jffs_backup_profile", filepath);
				return -1;
			}

			break;

		case TYPE_UISUPORT: {
			struct json_object *ui_support_obj = json_object_new_object();

			ret = get_ui_support_info(ui_support_obj);
			if (ret==0) {
				json_object_to_file(filepath, ui_support_obj);
			}

			json_object_put(ui_support_obj);

			break;
		}

#ifdef RTCONFIG_AMAS_CENTRAL_CONTROL
		case TYPE_AMASCNTRL:
			if ((ret = gen_jffs_backup_profile("amascntrl", filepath)) == HTTP_OK) {
      			ret = 0;
    		}
    		else {
				return -1;
			}

			break;
#endif

		default:
			return -1;

	}

	return ret;
}

backup_files *insert_backup_file(backup_files *backup_file, const int file_len, const char *filename) {

	if (is_allowed_cloud_file_format(filename)!=0) {
		Cdbg(API_DBG, "fail is_allowed_cloud_file_format[%s]", filename); 
		return NULL;
	}

	int reset_time = 0;
	int upload_time = 0;

	DECLARE_CLEAR_MEM(char, s_type, MAX_TYPE_LEN);
	DECLARE_CLEAR_MEM(char, s_mac, MAX_MAC_LEN);
	DECLARE_CLEAR_MEM(char, s_reset_time, MAX_TIMESTAMP_LEN);
	DECLARE_CLEAR_MEM(char, s_upload_time, MAX_TIMESTAMP_LEN);
	DECLARE_CLEAR_MEM(char, s_file_ext, MAX_FILE_EXT_LEN);

	if (sscanf(filename, "%[^_]_%[^_]_%[^_]_%[^.].%s", s_type, s_mac, s_reset_time, s_upload_time, s_file_ext)==5) {
		//- e.g. setting_0492266A7060_1680862400_1680862430.cfg
    	// Cdbg(API_DBG, "filename=%s, type=%s, mac=%s, reset_time=%s, upload_time=%s, file_ext=%s", 
    	// 	filename, s_type, s_mac, s_reset_time, s_upload_time, s_file_ext);
    }
    else {
    	return NULL;
    }

    if (strlen(s_reset_time)>0) {
    	reset_time = atoi(s_reset_time);
	}

	if (strlen(s_upload_time)>0) {
		upload_time = atoi(s_upload_time);
	}

	// Cdbg(API_DBG, "reset_time=%d, upload_time=%d", reset_time, upload_time);

	if (backup_file==NULL) {
		backup_file = malloc(sizeof(backup_files));
		snprintf(backup_file->filename, sizeof(backup_file->filename), "%s", filename);
		backup_file->reset_time = reset_time;
		backup_file->upload_time = upload_time;
		backup_file->next = NULL;
	}
	else {
		backup_files *new_file = backup_file;
		
		while (new_file->next!=NULL) {
	  		new_file = new_file->next;
		}

		new_file->next = malloc(sizeof(backup_files));
		new_file = new_file->next;
		snprintf(new_file->filename, sizeof(new_file->filename), "%s", filename);
		new_file->reset_time = reset_time;
		new_file->upload_time = upload_time;
		new_file->next = NULL;
	}

	// Cdbg(API_DBG, "Success to insert backup file[%s].", filename);

	return backup_file;
}

backup_files *delete_backup_file(backup_files *files, backup_files *delete_file) {

	backup_files* bf = files;

  	if (files == NULL) { 
		// Cdbg(API_DBG, "backup file nothing to delete ");
		return NULL;
  	}

  	if (delete_file == files) {  // If the first node is deleted
		// Cdbg(API_DBG, "delete_file frist");
		files = files->next;//把backup_files指向下一個節點(NULL)
  	}
  	else {
		// Cdbg(API_DBG, "delete_file next");
		while (bf->next != delete_file) { //找到要刪除之節點的前一個節點
			bf = bf->next;
		}

		bf->next = delete_file->next; //重新設定bf的next成員
  	}

  	free(delete_file);

  	return files;
}

int clear_backup_file(backup_files *files) {
	if (files==NULL) {
		return -1;
	}

	backup_files *tmp = NULL;
	backup_files *curr_file = files;
	while (curr_file!=NULL) {
		tmp = curr_file;
		curr_file = curr_file->next;

		if (tmp) {
			free(tmp);
			tmp = NULL;
		}
	}

	return 0;
}
////////////////////////////////////////////////////////////////////////////////

int update_userticket() {

    int update_status;
    char event[AAE_MAX_IPC_PACKET_SIZE];
    char out[AAE_MAX_IPC_PACKET_SIZE];
    
    if(strlen(nvram_safe_get("oauth_dm_refresh_ticket")) == 0) {
        return -1;
    }

    // Cdbg(APP_DBG, "Update userticket!");
    snprintf(event, sizeof(event), AAE_DDNS_GENERIC_MSG, AAE_EID_DDNS_REFRESH_TOKEN);
    
    aae_sendIpcMsgAndWaitResp(MASTIFF_IPC_SOCKET_PATH, event, strlen(event), out, sizeof(out), 10);
    
    // Cdbg(APP_DBG, "Update userticket  out -> %s", out);

    json_object *root = NULL;
    json_object *ddnsObj = NULL;
    json_object *eidObj = NULL;
    json_object *stsObj = NULL;
    root = json_tokener_parse((char *)out);
    json_object_object_get_ex(root, AAE_DDNS_PREFIX, &ddnsObj);
    json_object_object_get_ex(ddnsObj, AAE_IPC_EVENT_ID, &eidObj);
    json_object_object_get_ex(ddnsObj, AAE_IPC_STATUS, &stsObj);
    if (!ddnsObj || !eidObj || !stsObj) {
        // Cdbg(APP_DBG, "Failed to aae_refresh_ticket\n");
    }
    else {
        int eid = json_object_get_int(eidObj);
        const char *status = json_object_get_string(stsObj);
        if ((eid == AAE_EID_DDNS_REFRESH_TOKEN) && (!strcmp(status, "0"))) {
            // Cdbg(APP_DBG, "Success to aae_refresh_ticket\n");
            update_status = 0;
        }
        else {
            // Cdbg(APP_DBG, "Failed to aae_refresh_ticket\n");
            update_status = -1;
        }
    }

    json_object_put(root);

    // if(update_status == 0) {
        // snprintf(oauth_dm_cusid, sizeof(oauth_dm_cusid), "%s", nvram_safe_get("oauth_dm_cusid"));
        // snprintf(oauth_dm_user_ticket, sizeof(oauth_dm_user_ticket), "%s", nvram_safe_get("oauth_dm_user_ticket"));
        // Cdbg(APP_DBG, "oauth_dm_cusid = %s", oauth_dm_cusid);
        // Cdbg(APP_DBG, "oauth_dm_user_ticket = %s", oauth_dm_user_ticket);
    // }

    return update_status;    
}

int get_cloud_access_token(char* access_token) {

	if (access_token==NULL) {
		return -1;
	}

	int status, retry_count = 0;
	const char* server = nvram_safe_get("aae_webstorageinfo");
  	const char* cusid = nvram_safe_get("oauth_dm_cusid");
  	const char* user_tiket = nvram_safe_get("oauth_dm_user_ticket");

  	RouterCheckToken rt;
  	memset(&rt, 0, sizeof(RouterCheckToken));
  
  	// check token
 	do {
		status = send_router_check_token_req(server, cusid, user_tiket, &rt);

	  	if (status!=0) {
	  		Cdbg(API_DBG, "Fail to get cloud access token, status=%d", status);
		  	return -1;
	  	} 
	  	else if (strncmp(rt.status, "0", 1) != 0) {
			Cdbg(API_DBG, "Fail to get cloud access token, rt.status=%s", rt.status);

		  	update_userticket();

		  	sleep(10);
		  
		  	retry_count++;
		  	if (retry_count>5) {
				Cdbg(API_DBG, "Maximum number of errors exceeded, retry_count=%d", retry_count);
				return -1;
		  	}

		  	continue;
	  	}

	  	break;

	} while (1);

	strncpy(access_token, rt.access_token, MAX_DESC_LEN);

	return 0;
}

void print_cloud_file() {

#if 0
	backup_file_types *handler = NULL;
    for(handler = &BACK_FILE_TYPES[0]; handler->id>0; handler++){
    	Cdbg(API_DBG, "%s, %d(%d)/%d", handler->bf_name, handler->bf_len, count_backup_file(handler->bf), handler->max_file_limit);
    }
    Cdbg(API_DBG, "*******");
#endif
}

int clear_cloud_file() {

	backup_file_types *handler = NULL;
    for (handler = &BACK_FILE_TYPES[0]; handler->id>0; handler++){
    	if (clear_backup_file(handler->bf)==0) {
    		handler->bf = NULL;
    		handler->bf_len = 0;
    	}
    }

    print_cloud_file();
    
    return 0;
}

int list_cloud_file(const char* access_token) {

	int status = 0;

	int uploader_reset_time = nvram_get_int("uploader_reset_time");
  	const char* server = nvram_safe_get("aae_webstorageinfo");

  	RouterListFile lf;
  	memset(&lf, 0, sizeof(RouterListFile));
  	status = send_router_list_file_req(server, access_token, &lf);

  	if (status != 0) {
  		Cdbg(API_DBG, "Fail to list cloud file, status=%d", status);
	  	return status;
  	} 
	else if (strncmp(lf.status, "0", 1) != 0) {
		Cdbg(API_DBG, "Fail to list cloud file, status=%s", lf.status);
	  	return -1;
  	}

  	Cdbg(API_DBG, "lf.files=%s", lf.files);

	json_object *files_obj = json_tokener_parse(lf.files);
	if (files_obj) {
		int i, files_len = 0;
		files_len = json_object_array_length(files_obj);

		char file_list[files_len][MAX_FILENAME_LEN];

		for (i = 0; i < files_len; i++) {
			json_object *file_obj = json_object_array_get_idx(files_obj, i);
			if (file_obj) {
				const char* file_name = json_object_get_string(file_obj);

				if (is_router_cloud_file_format(file_name)!=0) {
					continue;
				}

				//- Special case: Delete oldest format cloud backup files
				if (is_oldest_cloud_file_format(file_name)==0) {
					delete_cloud_file(access_token, file_name);
					continue;
				}

				backup_file_types *handler = NULL;
				for(handler = &BACK_FILE_TYPES[0]; handler->id>0; handler++){
					if (strstr(file_name, handler->bf_name)!=NULL) {
						backup_files* bf = insert_backup_file(handler->bf, handler->bf_len, file_name);
			    		if (bf) {
							handler->bf = bf;
				  			handler->bf_len++;
				  		}
				  	}
				}
			}
		}

		json_object_put(files_obj);
	}

	print_cloud_file();
	
  	return status;
}

int count_backup_file(backup_files* list) {

	if (list==NULL) {
		return 0;
	}

	int count = 0;

	backup_files* curr_file = list;
	while (curr_file) {
		count++;
		curr_file = curr_file->next;
	}

	return count;
}

int count_backup_file_with_same_reset_time(backup_files* list, int uploader_reset_time) {

	if (list==NULL) {
		return 0;
	}

	int count = 0;

	backup_files* curr_file = list;
	while (curr_file) {

		if (curr_file->reset_time == uploader_reset_time) {
			count++;
		}

		curr_file = curr_file->next;
	}

	return count;
}

int count_backup_file_with_different_reset_time(backup_files* list, int uploader_reset_time) {

	if (list==NULL) {
		return 0;
	}

	int count = 0;

	backup_files* curr_file = list;
	while (curr_file) {

		if (curr_file->reset_time != uploader_reset_time) {
			count++;
		}

		curr_file = curr_file->next;
	}

	return count;
}

backup_files* get_backup_file_with_minimum_upload_time(backup_files* list, int uploader_reset_time) {

	if (list==NULL) {
		return NULL;
	}

	float minimum_upload_time = 9999999999;

	backup_files* find_file = NULL;
	backup_files* curr_file = list;
	while (curr_file) {

		if (curr_file->reset_time != uploader_reset_time) {
			curr_file = curr_file->next;
			continue;
		}
		
		if (curr_file->upload_time < minimum_upload_time) {
			minimum_upload_time = curr_file->upload_time;
			find_file = curr_file;
		}

		curr_file = curr_file->next;
	}

	return find_file;
}

backup_files* get_backup_file_with_minimum_reset_time(backup_files* list, int uploader_reset_time) {

	if (list==NULL) {
		return NULL;
	}

	float minimum_reset_time = 9999999999;

	backup_files* find_file = NULL;
	backup_files* curr_file = list;
	while (curr_file) {

		if (curr_file->reset_time == uploader_reset_time) {
			curr_file = curr_file->next;
			continue;
		}
		
		if (curr_file->reset_time < minimum_reset_time) {
			minimum_reset_time = curr_file->upload_time;
			find_file = curr_file;
		}

		curr_file = curr_file->next;
	}

	return find_file;
}

void reorganize_cloud_file(const char* access_token) {

	int uploader_reset_time = nvram_get_int("uploader_reset_time");

	backup_file_types *handler = NULL;

    for (handler = &BACK_FILE_TYPES[0]; handler->id>0; handler++) {

    	if (handler->bf_len<=0) {
    		continue;
    	}

    	backup_files* del_file = NULL;
    	backup_files* curr_file = handler->bf;
    	while (curr_file) {

    		int max_count = 1;
    		int backup_file_reset_time = curr_file->reset_time;

    		if (backup_file_reset_time == uploader_reset_time) {
    			max_count = handler->max_file_limit;
    		}

    		int count = count_backup_file_with_same_reset_time(handler->bf, backup_file_reset_time);
			if (count>max_count) {
				del_file = get_backup_file_with_minimum_upload_time(handler->bf, backup_file_reset_time);
				if (del_file && delete_cloud_file(access_token, del_file->filename)==0) {
					handler->bf = delete_backup_file(handler->bf, del_file);
					handler->bf_len--;
					curr_file = handler->bf;

					//- continue to delete another backup file
					continue;
			  	}
			}

    		curr_file = curr_file->next;
    	}
		//////////////////////////////////////////////////////////////////////
    	
    	//- Delete the locked files(different with current uploader reset time) exceeding the maximum number(MAX_RESET_TIME_BACKUP_NUM).
    	int count2 = count_backup_file_with_different_reset_time(handler->bf, uploader_reset_time);
    	if (count2>MAX_RESET_TIME_BACKUP_NUM) {
    		Cdbg(API_DBG, "%s: Delete the locked files(%d) exceeding the maximum number(%d)", handler->bf_name, count2, MAX_RESET_TIME_BACKUP_NUM);
    		backup_files* del_file2 = get_backup_file_with_minimum_reset_time(handler->bf, uploader_reset_time);
    		if (del_file2 && delete_cloud_file(access_token, del_file2->filename)==0) {
				handler->bf = delete_backup_file(handler->bf, del_file2);
				handler->bf_len--;
		  	}
    	}
    	//////////////////////////////////////////////////////////////////////
    }

    print_cloud_file();
}

int delete_cloud_file(const char* access_token, const char *file_name) {

	if (is_router_cloud_file_format(file_name)!=0) {
		Cdbg(API_DBG, "Not allowed file=%s", file_name);
		return -1;
  	}

  	const char* server = nvram_safe_get("aae_webstorageinfo");

  	RouterDelFile df;
  	memset(&df, 0, sizeof(RouterDelFile));
  	int status = send_router_del_file_req(server, access_token, file_name, &df);

  	if (status==0) {  
		Cdbg(API_DBG, "Success to delete cloud file[%s]", file_name);
  	}
  	else {
		Cdbg(API_DBG, "Fail to delete cloud file[%s], status = %d", file_name, status);
  	}

  	return status;
}

void backup_cloud_file(const char* access_token) {

	backup_file_types *handler = NULL;

    for(handler = &BACK_FILE_TYPES[0]; handler->id>0; handler++){
    	
    	DECLARE_CLEAR_MEM(char, s_update_name, MAX_FILENAME_LEN);
    	DECLARE_CLEAR_MEM(char, s_upload_name, MAX_FILENAME_LEN);

    	snprintf(s_update_name, sizeof(s_update_name), "%s_update_time", handler->name);
        snprintf(s_upload_name, sizeof(s_upload_name), "%s_upload_time", handler->name);
        
        int update_time = nvram_get_int(s_update_name);
        int upload_time = nvram_get_int(s_upload_name);

        // Cdbg(API_DBG, "handler->name=%s, update_time=%d, upload_time=%d", handler->name, update_time, upload_time);

        DECLARE_CLEAR_MEM(char, filename, MAX_FILENAME_LEN);

        if (update_time>upload_time) {

        	// Cdbg(API_DBG, "update_time %d> upload_time %d", update_time, upload_time);

            if (generate_backup_file(handler, filename, update_time) == PROCESS_SUCCESS) {
            	
            	// backup file upload
                if (upload_cloud_file(access_token, filename)==0) {
                    backup_files* bf = insert_backup_file(handler->bf, handler->bf_len, filename);
		    		if (bf) {
						handler->bf = bf;
			  			handler->bf_len++;
			  		}
                }

                DECLARE_CLEAR_MEM(char, filepath, MAX_FILEPATH_LEN);
                snprintf(filepath, sizeof(filepath), "%s%s", UPLOADER_FOLDER, filename);

                if(!remove(filepath)) {
                    // Cdbg(API_DBG, "Success to delete backup file[%s].", filepath);
                    print_cloud_file();
                }
                else {
                	Cdbg(API_DBG, "Fail to delete backup file[%s].", filepath);	
                }
            }

            nvram_set_int(s_upload_name, update_time);
        }
    }

}

int upload_cloud_file(const char* access_token, const char *file_name) {

	int status = -1;

  	const char* server = nvram_safe_get("aae_webstorageinfo");
 
  	RouterGetUploadFileUrl gul;
  	memset(&gul, 0, sizeof(RouterGetUploadFileUrl));
  	status = send_router_get_upload_file_url_req(server, access_token, file_name, &gul);
  	if (status!=0 || strncmp(gul.status, "0", 1)!=0) {
  		Cdbg(API_DBG, "Fail to get uploade file url, status=%d, gul.status=%s", status, gul.status);
  		return -1;
  	}

  	DECLARE_CLEAR_MEM(char, upload_file, MAX_FILEPATH_LEN);
	snprintf(upload_file, sizeof(upload_file) ,"%s/%s", UPLOADER_FOLDER, file_name);
	
	status = upload_file_to_s3(gul.url, 
							   NULL,
							   NULL,
							   gul.key,
							   gul.x_amz_algorithm,
							   gul.x_amz_credential,
							   gul.x_amz_date,
							   gul.policy,
							   gul.x_amz_signature,
							   NULL,
							   upload_file);
	
	if (status != 204) {
		Cdbg(API_DBG, "Fail to upload file[%s] to S3, status=%d", file_name, status);
		return -1;
	} 
	
	RouterUploadFile uf;
  	memset(&uf, 0, sizeof(RouterUploadFile));
	status = send_router_upload_file_req(server, access_token, file_name, &uf);
	if (status!=0 || strncmp(uf.status, "0", 1)!=0) {
  		return -1;
  	}

  	Cdbg(API_DBG, "Success to upload backup file[%s].", file_name);	

	return status;
}
////////////////////////////////////////////////////////////////////////////////

int proc_router_check_token_json(void* data_struct, struct json_object* json_obj){
  
  	if (!data_struct) {
		Cdbg(API_DBG, "data_struct is NULL.");
		return -1;
	}

	if (!json_obj) {
		Cdbg(API_DBG, "josn_obj is NULL.");
		return -1;
	}

  	RouterCheckToken *this_data = (RouterCheckToken*)data_struct;

  	GET_JSON_STRING_FIELD_TO_ARRARY(json_obj, "status", this_data->status);
  	GET_JSON_STRING_FIELD_TO_ARRARY(json_obj, "access_token", this_data->access_token);

  	return 0;
}

int proc_router_list_file_json(void* data_struct, struct json_object* json_obj){

	if (!data_struct) {
		Cdbg(API_DBG, "data_struct is NULL.");
		return -1;
	}

	if (!json_obj) {
		Cdbg(API_DBG, "josn_obj is NULL.");
		return -1;
	}

  	RouterListFile *this_data = (RouterListFile*)data_struct;

  	GET_JSON_STRING_FIELD_TO_ARRARY(json_obj, "status", this_data->status);
  	GET_JSON_STRING_FIELD_TO_ARRARY(json_obj, "files", this_data->files);

  	return 0;
}

int proc_router_del_file_json(void* data_struct, struct json_object* json_obj){

	if (!data_struct) {
		Cdbg(API_DBG, "data_struct is NULL.");
		return -1;
	}

	if (!json_obj) {
		Cdbg(API_DBG, "josn_obj is NULL.");
		return -1;
	}

	RouterListFile *this_data = (RouterListFile*)data_struct;

  	GET_JSON_STRING_FIELD_TO_ARRARY(json_obj, "status", this_data->status);

  	return 0;
}

int proc_router_get_upload_file_url_json(void* data_struct, struct json_object* json_obj){

	if (!data_struct) {
		Cdbg(API_DBG, "data_struct is NULL.");
		return -1;
	}

	if (!json_obj) {
		Cdbg(API_DBG, "josn_obj is NULL.");
		return -1;
	}

	RouterGetUploadFileUrl *this_data = (RouterGetUploadFileUrl*)data_struct;
	
  	GET_JSON_STRING_FIELD_TO_ARRARY(json_obj, "status", this_data->status);

  	struct json_object *presigned_url_obj = NULL;
  	struct json_object *fields_obj = NULL;

  	json_object_object_get_ex(json_obj, "presigned_url", &presigned_url_obj);

  	if (presigned_url_obj) {

  		GET_JSON_STRING_FIELD_TO_ARRARY(presigned_url_obj, "url", this_data->url);

  		json_object_object_get_ex(presigned_url_obj, "fields", &fields_obj);
  		if (fields_obj) {
  			GET_JSON_STRING_FIELD_TO_ARRARY(fields_obj, "key", this_data->key);
			GET_JSON_STRING_FIELD_TO_ARRARY(fields_obj, "x-amz-algorithm", this_data->x_amz_algorithm);
			GET_JSON_STRING_FIELD_TO_ARRARY(fields_obj, "x-amz-credential", this_data->x_amz_credential);
			GET_JSON_STRING_FIELD_TO_ARRARY(fields_obj, "x-amz-date", this_data->x_amz_date);
			GET_JSON_STRING_FIELD_TO_ARRARY(fields_obj, "policy", this_data->policy);
			GET_JSON_STRING_FIELD_TO_ARRARY(fields_obj, "x-amz-signature", this_data->x_amz_signature);
  		}
  	}

  	return 0;
}

int proc_router_upload_file_json(void* data_struct, struct json_object* json_obj){

	if (!data_struct) {
		Cdbg(API_DBG, "data_struct is NULL.");
		return -1;
	}

	if (!json_obj) {
		Cdbg(API_DBG, "josn_obj is NULL.");
		return -1;
	}

	RouterUploadFile *this_data = (RouterUploadFile*)data_struct;

  	if (!this_data) {
		return -1;
  	}

  	if (!json_obj) {
		return -1;
  	}

  	GET_JSON_STRING_FIELD_TO_ARRARY(json_obj, "status", this_data->status);

  	return 0;
}

int proc_router_get_debug_token_json(void* data_struct, struct json_object* json_obj){

	if (!data_struct) {
		Cdbg(API_DBG, "data_struct is NULL.");
		return -1;
	}

	if (!json_obj) {
		Cdbg(API_DBG, "josn_obj is NULL.");
		return -1;
	}

	RouterGetDebugToken *this_data = (RouterGetDebugToken*)data_struct;

  	if (!this_data) {
		return -1;
  	}

  	if (!json_obj) {
		return -1;
  	}

  	GET_JSON_STRING_FIELD_TO_ARRARY(json_obj, "status", this_data->status);
  	GET_JSON_STRING_FIELD_TO_ARRARY(json_obj, "credential", this_data->x_amz_credential);
  	GET_JSON_STRING_FIELD_TO_ARRARY(json_obj, "date", this_data->x_amz_date);
  	GET_JSON_STRING_FIELD_TO_ARRARY(json_obj, "hashpolicy", this_data->policy);
  	GET_JSON_STRING_FIELD_TO_ARRARY(json_obj, "signature", this_data->x_amz_signature);

  	return 0;
}

PROC_JSON_DATA get_proc_json_fn(WS_ID wi) {

	if (wi == e_router_check_token)  return proc_router_check_token_json;
  	if (wi == e_router_list_file)  return proc_router_list_file_json;
  	if (wi == e_router_del_file)  return proc_router_del_file_json;
  	if (wi == e_router_get_upload_file_url)  return proc_router_get_upload_file_url_json;
  	if (wi == e_router_upload_file)  return proc_router_upload_file_json;
  	if (wi == e_router_get_debug_token)  return proc_router_get_debug_token_json;
}

void get_wm(WS_MANAGER* wsM, WS_ID wsID, void* pSrvType, size_t SrvSize) {

	memset(wsM, 0 , sizeof(wsM));
  	wsM->ws_id                = wsID;
  	wsM->ws_storage           = pSrvType;
  	wsM->ws_storage_size      = SrvSize;
}

int process_json(char* json_buff, WS_MANAGER*  wm) {

	PROC_JSON_DATA proc = NULL;
	struct json_object *json_obj = NULL;
	proc = get_proc_json_fn(wm->ws_id);

	// Cdbg(API_DBG, "json_buff : %s", json_buff);

	if (json_buff) {
		json_obj = json_tokener_parse(json_buff);
	} 
	else {
		json_obj = json_tokener_parse("{}");
	}

	if(proc) {
		(proc)(wm->ws_storage, json_obj);
	}

	if (json_obj!=NULL) {
		json_object_put(json_obj);
	}

	return 0;
}

int send_router_check_token_req(
	const char *server,
  	const char *cusid, 
  	const char *user_ticket,
  	RouterCheckToken *pData
) { 
  
  	char *json_outbuf = NULL;
  	const char* router_mac = nvram_get("lan_hwaddr");

  	DECLARE_CLEAR_MEM(char, header, MAX_HEADER_LEN);
	snprintf(header, sizeof(header), s3_req_header_templ);

	DECLARE_CLEAR_MEM(char, cookie, MAX_COOKIE_LEN);
	snprintf(cookie, sizeof(cookie), s3_req_custom_cookie_templ, SID);

  	DECLARE_CLEAR_MEM(char, devicehashmac, MD_STR_LEN);
  	get_md5_string(router_mac, devicehashmac);

  	char* url = get_webpath(TRANSFER_TYPE, server, REQ_ROUTER_CHECK_TOKEN);
  	char* append_data = get_append_data(router_check_token_template, cusid, user_ticket, devicehashmac, SID);
  
  	int status = http_request_process(url, append_data, header, cookie, HTTP_REQUEST_POST, &json_outbuf);
  
  	WS_MANAGER ws_manager;
  	get_wm(&ws_manager, e_router_check_token, pData, sizeof(RouterCheckToken));
  	process_json(json_outbuf, &ws_manager);
  
  	if (append_data) free_append_data(append_data);
  	if (json_outbuf) free(json_outbuf);
  	if (url) free_webpath(url);

  	return status == 200 ? 0 : status;
}

int send_router_list_file_req(
	const char *server,
	const char *access_token,
	RouterListFile *pData
) { 
  
	char *json_outbuf = NULL;
	const char* router_mac = nvram_get("lan_hwaddr");

	DECLARE_CLEAR_MEM(char, header, MAX_HEADER_LEN);
	snprintf(header, sizeof(header), s3_req_header_templ);

	DECLARE_CLEAR_MEM(char, cookie, MAX_COOKIE_LEN);
	snprintf(cookie, sizeof(cookie), s3_req_custom_cookie_templ, SID);

	DECLARE_CLEAR_MEM(char, devicehashmac, MD_STR_LEN);
	get_md5_string(router_mac, devicehashmac);

	char* url = get_webpath(TRANSFER_TYPE, server, REQ_ROUTER_LIST_FILE);
	char* append_data = get_append_data(router_list_file_template, devicehashmac, access_token);

	int status = http_request_process(url, append_data, header, cookie, HTTP_REQUEST_POST, &json_outbuf);

	WS_MANAGER ws_manager;
	get_wm(&ws_manager, e_router_list_file, pData, sizeof(RouterListFile));
	process_json(json_outbuf, &ws_manager);

	if (append_data) free_append_data(append_data);
	if (json_outbuf) free(json_outbuf);
	if (url) free_webpath(url);

	return status == 200 ? 0 : status;
}

int send_router_del_file_req(
	const char *server,
	const char *access_token,
	const char *file_name,
	RouterDelFile *pData
) { 
  
	char *json_outbuf = NULL;
	const char* router_mac = nvram_get("lan_hwaddr");

	DECLARE_CLEAR_MEM(char, header, MAX_HEADER_LEN);
	snprintf(header, sizeof(header), s3_req_header_templ);

	DECLARE_CLEAR_MEM(char, cookie, MAX_COOKIE_LEN);
	snprintf(cookie, sizeof(cookie), s3_req_custom_cookie_templ, SID);

	DECLARE_CLEAR_MEM(char, devicehashmac, MD_STR_LEN);
	get_md5_string(router_mac, devicehashmac);

	char* url = get_webpath(TRANSFER_TYPE, server, REQ_ROUTER_DEL_FILE);
	char* append_data = get_append_data(router_file_operation_template, devicehashmac, access_token, file_name);

	int status = http_request_process(url, append_data, header, cookie, HTTP_REQUEST_DELETE, &json_outbuf);

	WS_MANAGER ws_manager;
	get_wm(&ws_manager, e_router_del_file, pData, sizeof(RouterDelFile));
	process_json(json_outbuf, &ws_manager);

	if (append_data) free_append_data(append_data);
	if (json_outbuf) free(json_outbuf);
	if (url) free_webpath(url);

	return status == 200 ? 0 : status;
}

int send_router_get_upload_file_url_req(
	const char *server,
	const char *access_token,
	const char *file_name,
	RouterGetUploadFileUrl *pData
) { 
  
	char *json_outbuf = NULL;
	const char* router_mac = nvram_get("lan_hwaddr");

	DECLARE_CLEAR_MEM(char, header, MAX_HEADER_LEN);
	snprintf(header, sizeof(header), s3_req_header_templ);

	DECLARE_CLEAR_MEM(char, cookie, MAX_COOKIE_LEN);
	snprintf(cookie, sizeof(cookie), s3_req_custom_cookie_templ, SID);

	DECLARE_CLEAR_MEM(char, devicehashmac, MD_STR_LEN);
	get_md5_string(router_mac, devicehashmac);

	char* url = get_webpath(TRANSFER_TYPE, server, REQ_ROUTER_GET_UPLOAD_FILE_URL);
	char* append_data = get_append_data(router_file_operation_template, devicehashmac, access_token, file_name);

	int status = http_request_process(url, append_data, header, cookie, HTTP_REQUEST_POST, &json_outbuf);

	WS_MANAGER ws_manager;
	get_wm(&ws_manager, e_router_get_upload_file_url, pData, sizeof(RouterGetUploadFileUrl));
	process_json(json_outbuf, &ws_manager);

	if (append_data) free_append_data(append_data);
	if (json_outbuf) free(json_outbuf);
	if (url) free_webpath(url);

	return status == 200 ? 0 : status;
}

int send_router_upload_file_req(
	const char *server,
	const char *access_token,
	const char *file_name,
	RouterUploadFile *pData
) { 
  
	char *json_outbuf = NULL;
	const char* router_mac = nvram_get("lan_hwaddr");

	DECLARE_CLEAR_MEM(char, header, MAX_HEADER_LEN);
	snprintf(header, sizeof(header), s3_req_header_templ);

	DECLARE_CLEAR_MEM(char, cookie, MAX_COOKIE_LEN);
	snprintf(cookie, sizeof(cookie), s3_req_custom_cookie_templ, SID);

	DECLARE_CLEAR_MEM(char, devicehashmac, MD_STR_LEN);
	get_md5_string(router_mac, devicehashmac);

	char* url = get_webpath(TRANSFER_TYPE, server, REQ_ROUTER_UPLOAD_FILE);
	char* append_data = get_append_data(router_file_operation_template, devicehashmac, access_token, file_name);

	int status = http_request_process(url, append_data, header, cookie, HTTP_REQUEST_POST, &json_outbuf);

	WS_MANAGER ws_manager;
	get_wm(&ws_manager, e_router_upload_file, pData, sizeof(RouterUploadFile));
	process_json(json_outbuf, &ws_manager);

	if (append_data) free_append_data(append_data);
	if (json_outbuf) free(json_outbuf);
	if (url) free_webpath(url);

	return status == 200 ? 0 : status;
}

int send_router_get_debug_token_req(
	const char *server,
	RouterGetDebugToken *pData
) { 
  
	char *json_outbuf = NULL;
	const char* router_mac = nvram_get("lan_hwaddr");

	DECLARE_CLEAR_MEM(char, header, MAX_HEADER_LEN);
	snprintf(header, sizeof(header), s3_req_header_templ);

	DECLARE_CLEAR_MEM(char, cookie, MAX_COOKIE_LEN);
	snprintf(cookie, sizeof(cookie), s3_req_custom_cookie_templ, SID);

	DECLARE_CLEAR_MEM(char, ex_router_mac, 33);
  	snprintf(ex_router_mac, sizeof(ex_router_mac), "%s+Markcool", router_mac);

	DECLARE_CLEAR_MEM(char, devicehashmac, MD_STR_LEN);
	get_md5_string(ex_router_mac, devicehashmac);

	char* append_data = get_append_data(router_get_debug_token_template, router_mac, devicehashmac);
	
	int status = http_request_process(server, append_data, header, cookie, HTTP_REQUEST_POST, &json_outbuf);

	WS_MANAGER ws_manager;
	get_wm(&ws_manager, e_router_get_debug_token, pData, sizeof(RouterGetDebugToken));
	process_json(json_outbuf, &ws_manager);

	if (append_data) free_append_data(append_data);
	if (json_outbuf) free(json_outbuf);

	return status == 200 ? 0 : status;
}
////////////////////////////////////////////////////////////////////////////////

int http_request_process(const char* url, 
						 char* postdata, 
						 char* wb_custom_hdr, 
						 char* cookie, 
						 char* request_type, 
						 char** response_data) {

	int curl_status = 0;
  	int curl_retry_count = 0;

  	do {

		curl_status = send_req(request_type, url, postdata, wb_custom_hdr, cookie, response_data);

		// curl error code :  <= 100
		if ( curl_status <= 100) {

	  		curl_retry_count++;

	  		handle_error(curl_status);

	  		sleep(CURL_RETRY_TIME * curl_retry_count);

	  		if (curl_retry_count>5) {

				Cdbg(API_DBG, "CURL retry upper limit, exit uploader");

				return EXIT_UPLOADER;
	  		}

	  		continue;

		}

  	} while ( curl_status <= 100);

  	return curl_status;
}

int send_req(const char* request_type, 
			 const char* url, 
			 char* append_data, 
			 char *wb_custom_hdr, 
			 char* cookie, 
			 char** response_data) {

	int ret = -1;
	if(!url || !append_data) 
		goto SEND_REQ_EXIT;

	// wb_custom header is defined in wb_util.h for "Set-Cookie:ONE_VER=1_0; path=/; sid=appid"
	const char* custom_head[] = {wb_custom_hdr, NULL};

	struct input_info inbuf;
	inbuf.readptr = append_data;
	inbuf.sizeleft= strlen(append_data);

	RWCB rwcb;
	memset(&rwcb, 0, sizeof(rwcb)); 
	rwcb.write_cb   = &write_cb;
	rwcb.read_cb  = &read_callback;
	rwcb.pInput   = &inbuf;

	curl_io(request_type, url, custom_head, cookie, &rwcb);
	ret = rwcb.code;
	if(!rwcb.write_data) {
		goto SEND_REQ_EXIT; 
	}

	//if  rwcb.write_data is allocated in write_cb, it should be freed later
	size_t resp_len = strlen((char*)rwcb.write_data)+1;
	*response_data = (char*)malloc(resp_len);
	memset(*response_data, 0, resp_len);
	strcpy(*response_data, (const char*)rwcb.write_data);

	if(rwcb.write_data) 
		free(rwcb.write_data);

SEND_REQ_EXIT:  
	return ret;
}
////////////////////////////////////////////////////////////////////////////////

int upload_file_to_s3(const char* url,
						 const char* content_type,
						 const char* meta_uuid,
						 const char* key,
						 const char* algorithm,
						 const char* credential,
						 const char* date,
						 const char* policy,
						 const char* signature,
						 const char* server_side_encryption,
						 const char* upload_file) {

	CURL *curl;
  	CURLcode res;

  	FILE * fd_src;

  	struct stat file_info;

  	/* get the file size of the local file */
  	stat(upload_file, &file_info);

  	fd_src = fopen(upload_file, "rb");

  	if (NULL == fd_src) {
		Cdbg(API_DBG, "File[%s] is not existed!", upload_file);
		return -1;
  	}

  	long response_code;

  	/* In windows, this will init the winsock stuff */
  	curl_global_init(CURL_GLOBAL_ALL);

  	/* get a curl handle */
  	curl = curl_easy_init();

  	if(curl) {

		// struct curl_slist *headers = NULL;

		// if(header != NULL) {

	  	// 	headers = curl_slist_append(headers, header);

	  	// 	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

		// }
		
		// curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");


		/* get verbose debug output please */
		curl_easy_setopt(curl, CURLOPT_VERBOSE,   1L);

		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L); 
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1L);

		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 90); // 2017/07/31 add

		/* abort if slower than 30 bytes/sec during 60 seconds */
		// curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 60L);
		// curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 60L);


		/* we want to use our own read function */
		/* now specify which file to upload */
		// curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
		// curl_easy_setopt(curl, CURLOPT_READDATA, fd_src);
		// curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)file_info.st_size);


		/* open the file */ 
		// FILE *wfd = fopen(resposne_file, "wb");

		// if(wfd) {
		// 	curl_easy_setopt(curl, CURLOPT_WRITEDATA, wfd);
		// }

		curl_mime *mime;
		curl_mimepart *part;

		mime = curl_mime_init(curl);
		if (!mime) {
		    Cdbg(API_DBG, "curl_mime_init() failed!");
		    curl_easy_cleanup(curl);
		    curl_global_cleanup();
			return -1;
		 }

		//- Upload db file for debugging use
		if (content_type!=NULL) {
			part = curl_mime_addpart(mime);
			if (!part) {
				Cdbg(API_DBG, "curl_mime_addpart() failed!");
			    curl_mime_free(mime);
			    curl_easy_cleanup(curl);
			    curl_global_cleanup();
			    return -1;
			}

			curl_mime_name(part, "Content-Type");
			curl_mime_data(part, content_type, CURL_ZERO_TERMINATED);
		}

		//- Upload db file for debugging use
		if (meta_uuid!=NULL) {
			part = curl_mime_addpart(mime);
			if (!part) {
				Cdbg(API_DBG, "curl_mime_addpart() failed!");
			    curl_mime_free(mime);
			    curl_easy_cleanup(curl);
			    curl_global_cleanup();
			    return -1;
			}

			curl_mime_name(part, "x-amz-meta-uuid");
			curl_mime_data(part, meta_uuid, CURL_ZERO_TERMINATED);
		}
		
		if (key!=NULL) {
			part = curl_mime_addpart(mime);
			if (!part) {
				Cdbg(API_DBG, "curl_mime_addpart() failed!");
			    curl_mime_free(mime);
			    curl_easy_cleanup(curl);
			    curl_global_cleanup();
			    return -1;
			}

			curl_mime_name(part, "key");
			curl_mime_data(part, key, CURL_ZERO_TERMINATED);
		}

		if (algorithm!=NULL) {
			part = curl_mime_addpart(mime);
			if (!part) {
				Cdbg(API_DBG, "curl_mime_addpart() failed!");
			    curl_mime_free(mime);
			    curl_easy_cleanup(curl);
			    curl_global_cleanup();
			    return -1;
			}

			curl_mime_name(part, "x-amz-algorithm");
			curl_mime_data(part, algorithm, CURL_ZERO_TERMINATED);
		}

		if (credential!=NULL) {
			part = curl_mime_addpart(mime);
			if (!part) {
				Cdbg(API_DBG, "curl_mime_addpart() failed!");
			    curl_mime_free(mime);
			    curl_easy_cleanup(curl);
			    return -1;
			}

			curl_mime_name(part, "x-amz-credential");
			curl_mime_data(part, credential, CURL_ZERO_TERMINATED);
		}

		if (date!=NULL) {
			part = curl_mime_addpart(mime);
			if (!part) {
				Cdbg(API_DBG, "curl_mime_addpart() failed!");
			    curl_mime_free(mime);
			    curl_easy_cleanup(curl);
			    curl_global_cleanup();
			    return -1;
			}

			curl_mime_name(part, "x-amz-date");
			curl_mime_data(part, date, CURL_ZERO_TERMINATED);
		}

		if (policy!=NULL) {
			part = curl_mime_addpart(mime);
			if (!part) {
				Cdbg(API_DBG, "curl_mime_addpart() failed!");
			    curl_mime_free(mime);
			    curl_easy_cleanup(curl);
			    curl_global_cleanup();
			    return -1;
			}

			curl_mime_name(part, "policy");
			curl_mime_data(part, policy, CURL_ZERO_TERMINATED);
		}

		if (signature!=NULL) {
			part = curl_mime_addpart(mime);
			if (!part) {
				Cdbg(API_DBG, "curl_mime_addpart() failed!");
			    curl_mime_free(mime);
			    curl_easy_cleanup(curl);
			    curl_global_cleanup();
			    return -1;
			}

			curl_mime_name(part, "x-amz-signature");
			curl_mime_data(part, signature, CURL_ZERO_TERMINATED);
		}
			
		//- Upload db file for debugging use
		if (server_side_encryption!=NULL) {
			part = curl_mime_addpart(mime);
			if (!part) {
				Cdbg(API_DBG, "curl_mime_addpart() failed!");
			    curl_mime_free(mime);
			    curl_easy_cleanup(curl);
			    curl_global_cleanup();
			    return -1;
			}

			curl_mime_name(part, "x-amz-server-side-encryption");
			curl_mime_data(part, server_side_encryption, CURL_ZERO_TERMINATED);
		}

		part = curl_mime_addpart(mime);
		if (!part) {
			Cdbg(API_DBG, "curl_mime_addpart() failed!");
		    curl_mime_free(mime);
		    curl_easy_cleanup(curl);
		    curl_global_cleanup();
		    return -1;
		}

		curl_mime_name(part, "file");
		curl_mime_filedata(part, upload_file);

		curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

		/* Now run off and do what you've been told! */
		res = curl_easy_perform(curl);

		if( res != CURLE_OK ) {
	  		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

	  		// Cdbg(API_DBG, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

	  		response_code = res;

		} 
		else {
	  		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
		}

		/* close the header file */ 
		// fclose(wfd);

		// if(header)
		// 	curl_slist_free_all(headers);

		/* always cleanup */
		curl_easy_cleanup(curl);

		/* now cleanup the mimepost structure */
		curl_mime_free(mime);
  	}

  	fclose(fd_src); /* close the local file */

  	curl_global_cleanup();

  	// free(url);
  	return response_code;
}

void handle_error(int code) {

	DECLARE_CLEAR_MEM(char, error_message, MAX_MESSAGE_LEN);

	switch (code) {

	case CURL_CANNOT_RESOLVE_HOSTNAME:

		snprintf(error_message, sizeof(error_message), "Couldn't resolve host. The given remote host was not resolved.");
		break;

	case CURLE_COULDNT_CONNECT:

		snprintf(error_message, sizeof(error_message), "Failed to connect() to host or proxy");
		break;

	case CURL_CONNECT_TIMEOUT:

		snprintf(error_message, sizeof(error_message), "Can't connect to host,please check connection");
		break;

	case CURLE_SSL_CONNECT_ERROR:
		
		snprintf(error_message, sizeof(error_message), "ssl connect error, please check connection");
		break;

	default:
		break;
	}

	Cdbg(API_DBG, "code is %d, %s", code, error_message);
}

void backup_db_file() {

	// Cdbg(API_DBG, "nvram -> s3_ex_db_backup_enable = %d, ex_db_backup_path = %s", nvram_get_int("s3_ex_db_backup_enable"), nvram_safe_get("ex_db_backup_path"));

	if (nvram_get_int("s3_ex_db_backup_enable")!=1) {
		return;
	}

	DECLARE_CLEAR_MEM(char, ex_db_backup_path, MAX_FILEPATH_LEN);
	
	if (strcmp(nvram_safe_get("ex_db_backup_path"), "")==0) {
		snprintf(ex_db_backup_path, sizeof(ex_db_backup_path), "%s", EX_BACKUP_DB_PATH);
	} 
	else {
		snprintf(ex_db_backup_path, sizeof(ex_db_backup_path), "%s/.diag/", nvram_safe_get("ex_db_backup_path"));  
	}

	// Cdbg(API_DBG, "set ex_db_backup_path = [%s]", ex_db_backup_path);

	backup_db_process(ex_db_backup_path);
}

void backup_db_process(char *db_path) {
	
	struct dirent* ent = NULL;
	DIR *pDir = opendir(db_path);

  	if (NULL == pDir) {
		Cdbg(API_DBG, "Fail to open %s", db_path);
		return;
  	}

  	int i = 0, backup_db_number = 0;

  	int upload_time = get_ts_today_at_midnight();

  	// Cdbg(API_DBG, "if [ (backup db timestamp) < (upload_time == %d)], run [backup db to s3 server]", upload_time);

  	while (NULL != (ent=readdir(pDir))) {

		if(!strcmp(ent->d_name,".") || !strcmp(ent->d_name,"..") || (strcmp(get_extension(ent->d_name), "db") != 0))
			continue;

		// Cdbg(API_DBG, "find db[%d] -> [%s]", i, ent->d_name);

		DECLARE_CLEAR_MEM(char, backup_db_timestamp, MAX_TIMESTAMP_LEN);
		sscanf(ent->d_name, "%[^_]", backup_db_timestamp);
		int db_timestamp = atoi(backup_db_timestamp);

		if (db_timestamp < upload_time) {

	  		backup_db_number++;

	  		// Cdbg(API_DBG, "need upload db[%d] >> [%s] < upload_time (%d)", backup_db_number, ent->d_name, upload_time);

	  		// compare : Whether the backup file is uploaded
	  		if (compare_backup_list(db_path, ent->d_name) == SUCCESS) {
				
				DECLARE_CLEAR_MEM(char, backup_db_path, MAX_FILEPATH_LEN);
				DECLARE_CLEAR_MEM(char, upload_backup_path, MAX_FILEPATH_LEN);
				DECLARE_CLEAR_MEM(char, cmd, MAX_CMD_LEN);

				snprintf(upload_backup_path, sizeof(upload_backup_path), "%s%s_%s", db_path, g_formated_router_mac, ent->d_name);
				snprintf(backup_db_path, sizeof(backup_db_path), "%s%s", db_path, ent->d_name);

				// Cdbg(API_DBG, "backup_db_path = [%s]", backup_db_path);

				snprintf(cmd, sizeof(cmd), "cp %s %s", backup_db_path, upload_backup_path);
				system(cmd);

				if (access(upload_backup_path, F_OK) != -1) {

		  			// Cdbg(API_DBG, "prepare upload tmp db [%s] to S3 server", upload_backup_path);

		  			RouterGetDebugToken rt;
				  	memset(&rt, 0, sizeof(RouterGetDebugToken));
					
					int status = send_router_get_debug_token_req(DEBUG_TOKEN_URL, &rt);
					
					if (status == 0 && strncmp(rt.status, "0", 1) == 0) {
						
						status = upload_file_to_s3("https://asus-router-config.s3.us-west-2.amazonaws.com", 
												   "image/",
												   "14365123651274",
												   "router/config/${filename}",
												   "AWS4-HMAC-SHA256",
												   rt.x_amz_credential,
												   rt.x_amz_date,
												   rt.policy,
												   rt.x_amz_signature,
												   "AES256",
												   upload_backup_path);
						
						if (status == 204) {
							Cdbg(API_DBG, "Success to upload file[%s] to S3", upload_backup_path);
							// write_backup_db(dir, ent->d_name);
			  			} 
			  			else {
							Cdbg(API_DBG, "Fail to upload file[%s] to S3", upload_backup_path);
			  			}
			
			  			// deleted uploaded backup_db
			  			if (!remove(upload_backup_path)) {
			  				// Cdbg(API_DBG, "Success to delete tmp db file[%s]", upload_backup_path);
			  			}
			  			else {
			  				Cdbg(API_DBG, "Fail to delete tmp db file[%s]", upload_backup_path);
			  			}
					}
		  			else {
						Cdbg(API_DBG, "Fail to get debug token, status = %d", status);
			  		}
			  		
		  			
				}
	  		}
		}

  	}

  	reconstruct_ex_backup_db_files(db_path, backup_db_number, upload_time);  

  	closedir(pDir);

}

int get_ts_today_at_midnight(void) {

	time_t now = time(NULL);
	struct tm utctm;
	utctm = *gmtime(&now);
	utctm.tm_isdst = -1;

	time_t utctt = mktime(&utctm);
	// diff is the offset in seconds
	long diff = now - utctt;

	utctm.tm_hour = 0;
	utctm.tm_min = 0;
	utctm.tm_sec = 0;

	time_t night_ts = mktime(&utctm);

	int midnight = (int) (night_ts + diff);

	// Cdbg(API_DBG, "get now time = %ld, utc_time = %ld, time diff = %ld, gmt midnight = %d", now, utctt, diff, midnight);

	return midnight;
}

int compare_backup_list(char * backup_db_path, char * backup_db_name) {

	DECLARE_CLEAR_MEM(char, backup_list_path, MAX_FILEPATH_LEN);
  	snprintf(backup_list_path, sizeof(backup_list_path), "%s%s", backup_db_path, EX_BACKUP_LIST);

  	// create backup_list_path new file
  	if (access(backup_list_path, 0)==-1) {
		eval("touch", backup_list_path);
  	} 

  	FILE *fp;

  	DECLARE_CLEAR_MEM(char, file_info, 1023);

  	if ((fp = fopen(backup_list_path, "r")) == NULL) {
		Cdbg(API_DBG, "Fail to open %s.", backup_list_path);
		return -1;
  	}

  	// int i = 0;

  	while(fgets(file_info, 1023, fp) != NULL) {
		// i++;
		// Cdbg(API_DBG, "i = %d, %s", i, file_info);

		if(strstr(file_info, backup_db_name) != NULL) {
	  		// Cdbg(API_DBG, "backup_db uploaded >> [%s]", backup_db_name);
	  		fclose(fp);
	  		return -1;
		}
  	}

  	fclose(fp);

  	return SUCCESS;
}

int write_backup_db(const char * backup_db_path, const char * backup_db_name) {

	DECLARE_CLEAR_MEM(char, write_db_path, MAX_FILEPATH_LEN);

  	snprintf(write_db_path, sizeof(write_db_path), "%s%s", backup_db_path, EX_BACKUP_LIST);
  
  	FILE *fp = fopen(write_db_path, "a+");

  	if (NULL==fp){
		Cdbg(API_DBG, "write [%s] file error -> open failure", write_db_path);
		return -1;
  	} 
  	else {
		fprintf(fp, "%s\n", backup_db_name);    
		fclose(fp);
		// Cdbg(API_DBG, "write [%s] to [%s] success", backup_db_name, write_db_path);
  	}

  	return 0;
}

int reconstruct_ex_backup_db_files(const char * backup_db_path, const int backup_db_number, const int upload_time) {

	// The number of files has not changed
  	if( g_backup_db_number_tmp == backup_db_number )  {
		return 0;
  	}

  	// Cdbg(API_DBG, "g_backup_db_number_tmp[%d] != backup_db_number[%d], backup_db_path = %s, upload_time = %d, prepare write new ex backup db file", g_backup_db_number_tmp, backup_db_number, backup_db_path, upload_time);

  	struct dirent* ent = NULL;
  	DIR *pDir = opendir(backup_db_path);

  	if (NULL == pDir) {
		Cdbg(API_DBG, "Fail to open %s", backup_db_path);
		return -1;
  	}

  	// delete old ex backup db file
  	DECLARE_CLEAR_MEM(char, write_db_path, MAX_FILEPATH_LEN);
  	snprintf(write_db_path, sizeof(write_db_path), "%s%s", backup_db_path, EX_BACKUP_LIST);
  	unlink(write_db_path);

  	// write new ex backup db files
	while (NULL != (ent=readdir(pDir))) {

		if(!strcmp(ent->d_name,".") || !strcmp(ent->d_name,"..") || (strcmp(get_extension(ent->d_name), "db") != 0))
			continue;

		DECLARE_CLEAR_MEM(char, backup_db_timestamp, 128);
		sscanf(ent->d_name, "%[^_]", backup_db_timestamp);

		int db_timestamp = atoi(backup_db_timestamp);

		if (db_timestamp<upload_time) {
	  		// Cdbg(API_DBG, "wirte db name = %s, db_timestamp[%d] <= upload_time[%d]", ent->d_name, db_timestamp, upload_time);
	  		write_backup_db(backup_db_path, ent->d_name);
		}
  	}

  	// save backup_db_number
  	g_backup_db_number_tmp = backup_db_number;

	return 0;
}

char *get_extension(char *fileName) {  
	
	int len = strlen(fileName);  
	int i = len;  
	while ( fileName[i] != '.' && i > 0 ){ 
		i--; 
	}  

	if (fileName[i] == '.') {  
		return &fileName[i+1];  
	}
	else {  
		return &fileName[len];
	}  
}
