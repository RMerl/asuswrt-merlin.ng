#include <rc.h>
#include <dlfcn.h>
#include <futest.h>
#include <sys/socket.h>
#include <sys/un.h>

#if defined(RTCONFIG_TUNNEL)
#include <aae_ipc.h>
#endif

#if defined(RTCONFIG_UPLOADER)
#include <uploader_ipc.h>
#endif

static void free_test_result_data(char* test_result_data) {
	if (test_result_data) {
		free(test_result_data);
		test_result_data = NULL;
	}
}

static char* do_make_str(const char *fmt,	...) {
	/* Guess	we need	no more	than 100 bytes.	*/
	int n, size = 100;
	char	*p,	*np;
	va_list ap;

	if ((p =	(char*)malloc(size)) == NULL)
		return NULL;

	memset(p, 0, size);
	while (1) {
		/* Try to print in the allocated	space. */
		va_start(ap,	fmt);
		n = vsnprintf(p,	size, fmt, ap);
		va_end(ap);
		/* If that worked, return the string. */
		if (n > -1 && n < size)
			return p;
		/* Else try again with more space. */
		if (n > -1)	  /* glibc 2.1 */
			size	= n+1; /* precisely	what is	needed */
		else			  /* glibc 2.0 */
			size	*= 2;  /* twice	the	old	size */
		if ((np = (char*)realloc (p, size))	== NULL) {
			free(p);
			return NULL;
		}
		else {
			p = np;
		}
	}
}

static char* read_file_to_string(const char* file) {
	
	FILE* fp;
	long fileSize;

	fp = fopen(file, "r");
	if (fp == NULL) {
		return -1;
    }

    fseek(fp, 0, SEEK_END);
    fileSize = ftell(fp);
    rewind(fp);

    char* content = (char*)malloc((fileSize + 1) * sizeof(char));
    if (content == NULL) {
    	fclose(fp);
    	return -1;
    }

	if (fread(content, sizeof(char), fileSize, fp) != fileSize) {
		free(content);
        fclose(fp);
        return -1;
    }

    content[fileSize] = '\0';
    
    fclose(fp);

	return content;
}

static int check_file_existed(const char* file) {
	FILE* fp;
    
    fp = fopen(file, "r");
    if (fp == NULL) {
        return -1;
    }
    
    fclose(fp);

    return 0;
}

static void output_test_result(const char* output_file, const char* result_data) {
	
	if (result_data==NULL) {
		return;
	}

	if (output_file==NULL) {
		fprintf(stdout, "No output file specified and print result data [%s]\n", result_data);
		return;
	}
	
	fprintf(stdout, "%s\n", result_data);
	
	FILE *file = fopen(output_file, "w");
	if (file == NULL) {
        printf("Fail to open output file\n");
        return;
    }

    fputs(result_data, file);
    fclose(file);
}

static int lib_unload(void* handle) {
	return dlclose(handle);
}

static int lib_load(void** handle, const char* lib_path) {
	*handle = NULL;
	*handle = dlopen(lib_path, RTLD_LAZY);
	if(!*handle){
		// Cdbg(NATAPI_DBG, "dll get functions error =%s", dlerror());
		return -1;	
	}
	else
		return 0;
}

static int lib_get_func(void* handle, const char* func_name, void** func_sym) {
	if(!handle) return -1;
	*func_sym = dlsym(handle, func_name);
	if(!*func_sym) {
		// Cdbg(NATAPI_DBG, "dll get functions error =%s", dlerror());
		return -1;
	}
	return 0;
}

static void lib_run_func(TESTCASE* handler, void* func_sym, const char* input_params, const char* output_file) {

	if (handler==NULL) {
		return;
	}

	int res = -1;
	char* result_data = NULL;

	switch (handler->id) {

    	case TC_GET_MEM_INFO: {

    		int mem_available = ((GET_MEM_INFO)func_sym)("MemAvailable")*1024;
			res = (mem_available>0) ? 0 : -1;
			result_data = get_test_result_data(test_int_result_template, handler->id, handler->name, res, mem_available);
			break;
    	}

    	default: 
			break;
	}

    if (result_data!=NULL) {
    	output_test_result(output_file, result_data);
		free_test_result_data(result_data);
	}
}

static void sysdep_run_func(TESTCASE* handler, const char* input_params, const char* output_file) {

	if (handler==NULL) {
		return;
	}

	int res = -1;
	char* result_data = NULL;

	switch (handler->id) {

		case TC_GET_HW_ACCELERATION: {

			DECLARE_CLEAR_MEM(unsigned char, acceleration, 8);
			res = get_hw_acceleration(acceleration, sizeof(acceleration));

			result_data = get_test_result_data(test_result_template, handler->id, handler->name, res, acceleration);

			break;
		}

		case TC_GET_SYS_CLK: {
			
			DECLARE_CLEAR_MEM(unsigned char, clk, 8);
			res = get_sys_clk(clk, sizeof(clk));
			
			result_data = get_test_result_data(test_result_template, handler->id, handler->name, res, clk);

			break;
		}

		case TC_GET_SYS_TEMP: {

			int temp = 0;
			res = get_sys_temp(&temp);

			result_data = get_test_result_data(test_int_result_template, handler->id, handler->name, res, temp);

			break;
		}

		case TC_GET_WIFI_CHIP: {

			if (input_params==NULL) {
				printf("input params needed!\n");
				break;
			}

			DECLARE_CLEAR_MEM(unsigned char, buf, 32);

			DECLARE_CLEAR_MEM(unsigned char, ifname, 5);
			strncpy(ifname, input_params, sizeof(ifname));

			res = get_wifi_chip(ifname, buf, sizeof(buf));

			result_data = get_test_result_data(test_result_template, handler->id, handler->name, res, buf);

			break;
		}

		case TC_GET_WIFI_TEMP: {
			
			int temp = 0;

			if (input_params==NULL) {
				printf("input params needed!\n");
				break;
			}

			DECLARE_CLEAR_MEM(unsigned char, ifname, 5);
			strncpy(ifname, input_params, sizeof(ifname));
			
			res = get_wifi_temp(ifname, &temp);
			
			result_data = get_test_result_data(test_int_result_template, handler->id, handler->name, res, temp);

			break;
		}

		case TC_GET_WIFI_COUNTRY: {

			if (input_params==NULL) {
				printf("input params needed!\n");
				break;
			}

			DECLARE_CLEAR_MEM(unsigned char, buf, 32);

			DECLARE_CLEAR_MEM(unsigned char, ifname, 5);
			strncpy(ifname, input_params, sizeof(ifname));

			res = get_wifi_country(ifname, buf, sizeof(buf));

			result_data = get_test_result_data(test_result_template, handler->id, handler->name, res, buf);

			break;
		}

		case TC_GET_WIFI_NOISE: {

			if (input_params==NULL) {
				printf("input params needed!\n");
				break;
			}

			DECLARE_CLEAR_MEM(unsigned char, buf, 32);

			DECLARE_CLEAR_MEM(unsigned char, ifname, 5);
			strncpy(ifname, input_params, sizeof(ifname));

			res = get_wifi_noise(ifname, buf, sizeof(buf));

			result_data = get_test_result_data(test_result_template, handler->id, handler->name, res, buf);

			break;
		}

		case TC_GET_WIFI_MCS: {

			if (input_params==NULL) {
				printf("input params needed!\n");
				break;
			}

			DECLARE_CLEAR_MEM(unsigned char, buf, 32);

			DECLARE_CLEAR_MEM(unsigned char, ifname, 5);
			strncpy(ifname, input_params, sizeof(ifname));

			res = get_wifi_mcs(ifname, buf, sizeof(buf));

			result_data = get_test_result_data(test_result_template, handler->id, handler->name, res, buf);

			break;
		}

		case TC_GET_WIFI_STATUS: {

			if (input_params==NULL) {
				printf("input params needed!\n");
				break;
			}

			DECLARE_CLEAR_MEM(char, buf, 2048);

			DECLARE_CLEAR_MEM(unsigned char, ifname, 5);
			strncpy(ifname, input_params, sizeof(ifname));

			res = get_wifi_status(ifname, buf, sizeof(buf));

			result_data = get_test_result_data(test_result_template, handler->id, handler->name, res, buf);

			break;
		}

		case TC_GET_PHY_INFO: {

			phy_info_list phy_list = {0};
			int i;

			GetPhyStatus(1, &phy_list);

			phy_port_mapping port_mapping;
			get_phy_port_mapping(&port_mapping);

#ifdef RTCONFIG_NEW_PHYMAP
#ifdef RTCONFIG_USB
			get_usb_modem_status(&phy_list);
#endif
#endif

			struct json_object *json_arr = json_object_new_array();

			for(i=0;i<phy_list.count;i++) {

				struct json_object *json_obj = json_object_new_object();
    			json_object_object_add(json_obj, "label_name", json_object_new_string(phy_list.phy_info[i].label_name));
    			json_object_object_add(json_obj, "phy_port_id", json_object_new_int(phy_list.phy_info[i].phy_port_id));
    			json_object_object_add(json_obj, "cap", json_object_new_int(phy_list.phy_info[i].cap));
    			json_object_object_add(json_obj, "cap_name", json_object_new_string(phy_list.phy_info[i].cap_name));
    			json_object_object_add(json_obj, "state", json_object_new_string(phy_list.phy_info[i].state));
    			json_object_object_add(json_obj, "link_rate", json_object_new_int(phy_list.phy_info[i].link_rate));
    			json_object_object_add(json_obj, "max_rate", json_object_new_int(port_mapping.port[i].max_rate));

    			json_object_array_add(json_arr, json_obj);
			}

			res = (phy_list.count>0) ? 0 : -1;

			result_data = get_test_result_data(test_json_result_template, handler->id, handler->name, res, json_object_to_json_string(json_arr));

			json_object_put(json_arr);

			break;
		}

		case TC_GET_PORT_STATUS: {

			phy_info_list phy_list = {0};
			int i;

			phy_port_mapping port_mapping;
			get_phy_port_mapping(&port_mapping);

			phy_list.count = port_mapping.count;
			for(i=0;i<port_mapping.count;i++) {
				phy_list.phy_info[i].phy_port_id = port_mapping.port[i].phy_port_id;
				snprintf(phy_list.phy_info[i].label_name, sizeof(phy_list.phy_info[i].label_name), port_mapping.port[i].label_name);
				phy_list.phy_info[i].cap = port_mapping.port[i].cap;
				//phy_list.phy_info[i].max_rate = port_mapping.port[i].max_rate;
			}

			GetPhyStatus(0, &phy_list);

#ifdef RTCONFIG_USB
			get_usb_modem_status(&phy_list);
#endif

			struct json_object *json_arr = json_object_new_array();

			for(i=0;i<phy_list.count;i++) {

				struct json_object *json_obj = json_object_new_object();
    			json_object_object_add(json_obj, "label_name", json_object_new_string(phy_list.phy_info[i].label_name));
    			json_object_object_add(json_obj, "phy_port_id", json_object_new_int(phy_list.phy_info[i].phy_port_id));
    			json_object_object_add(json_obj, "cap", json_object_new_int(phy_list.phy_info[i].cap));
    			json_object_object_add(json_obj, "cap_name", json_object_new_string(phy_list.phy_info[i].cap_name));
    			json_object_object_add(json_obj, "state", json_object_new_string(phy_list.phy_info[i].state));
    			json_object_object_add(json_obj, "link_rate", json_object_new_int(phy_list.phy_info[i].link_rate));
    			json_object_object_add(json_obj, "max_rate", json_object_new_int(port_mapping.port[i].max_rate));

    			json_object_array_add(json_arr, json_obj);


#ifdef RTCONFIG_USB
				if ((phy_list.phy_info[i].cap & PHY_PORT_CAP_USB) > 0) {

					fprintf(stdout, "usb.....\n");

					#if 0
					int max_hub_port = sizeof(phy_list.phy_info[i].usb_devices)/sizeof(usb_device_info_t);
					get_usb_devices_by_usb_port(phy_list.phy_info[i].usb_devices, max_hub_port, usb_port++);
					for (j=0; j<max_hub_port; j++) {
						if (strlen(phy_list.phy_info[i].usb_devices[j].type) == 0)
							continue;
						fprintf(stderr, "usb_path=%s, node=%s, type=%s, manufacturer=%s, product=%s, serial=%s, device_name=%s, speed=%d, storage_size=%" PRIu64 ", storage_used=%" PRIu64 "\n",
							phy_list.phy_info[i].usb_devices[j].usb_path,
							phy_list.phy_info[i].usb_devices[j].node,
							phy_list.phy_info[i].usb_devices[j].type,
							phy_list.phy_info[i].usb_devices[j].manufacturer,
							phy_list.phy_info[i].usb_devices[j].product,
							phy_list.phy_info[i].usb_devices[j].serial,
							phy_list.phy_info[i].usb_devices[j].device_name,
							phy_list.phy_info[i].usb_devices[j].speed,
							phy_list.phy_info[i].usb_devices[j].storage_size_in_kilobytes,
							phy_list.phy_info[i].usb_devices[j].storage_used_in_kilobytes);
					}
					#endif

				}
#endif


			}

			res = (phy_list.count>0) ? 0 : -1;

			result_data = get_test_result_data(test_json_result_template, handler->id, handler->name, res, json_object_to_json_string(json_arr));

			json_object_put(json_arr);

			break;
		}

		default: 
			break;
	}

	if (result_data!=NULL) {
		output_test_result(output_file, result_data);
		free_test_result_data(result_data);
	}
}

static void process_call_func(TESTCASE* handler, const char* input_params, const char* output_file) {

	if (handler==NULL) {
		return;
	}

	int res = -1;
	char* result_data = NULL;

	switch (handler->id) {

#ifdef RTCONFIG_TUNNEL

		case TC_TUNNEL_ENABLE: {

			DECLARE_CLEAR_MEM(char, out, AAE_MAX_IPC_PACKET_SIZE);
			DECLARE_CLEAR_MEM(char, event, AAE_MAX_IPC_PACKET_SIZE);

			snprintf(event, sizeof(event), AAE_AWSIOT_GENERIC_MSG, EID_AWSIOT_TUNNEL_ENABLE);
	   		aae_sendIpcMsgAndWaitResp(MASTIFF_IPC_SOCKET_PATH, event, strlen(event), out, sizeof(out), 10);

			res = 0;

			if (strcmp(out, "")==0) {
				result_data = get_test_result_data(test_result_template, handler->id, handler->name, res, out);
			}
			else {
				result_data = get_test_result_data(test_json_result_template, handler->id, handler->name, res, out);	
			}
       		
			break;
		}

		case TC_TUNNEL_TEST: {

			DECLARE_CLEAR_MEM(char, out, AAE_MAX_IPC_PACKET_SIZE);
			DECLARE_CLEAR_MEM(char, event, AAE_MAX_IPC_PACKET_SIZE);

			DECLARE_CLEAR_MEM(char, callee_id, 33);
			DECLARE_CLEAR_MEM(char, keep_run, 2);
			DECLARE_CLEAR_MEM(char, lport, 10);
			DECLARE_CLEAR_MEM(char, rport, 10);
			int target_channel = 0, i=0;

			const char* delimiters = ",";
			char* tmp = strdup(input_params);
			char *token = strtok(tmp, delimiters);

			while (token != NULL) {
				if (i==0) {
					snprintf(callee_id, sizeof(callee_id), "%s", token);
				}
				else if (i==1) {
					snprintf(keep_run, sizeof(keep_run), "%s", token);
				}
				else if (i==2) {
					snprintf(lport, sizeof(lport), "%s", token);
				}
				else if (i==3) {
					snprintf(rport, sizeof(rport), "%s", token);
				}

				token = strtok(NULL, delimiters);

				i++;		   
			}

			if (tmp) {
				free(tmp);
				tmp = NULL;
			}

			snprintf(event, sizeof(event), AAE_AWSIOT_TNL_TEST_MSG, EID_AWSIOT_TUNNEL_TEST, callee_id, keep_run, lport, rport);
			aae_sendIpcMsgAndWaitResp(MASTIFF_IPC_SOCKET_PATH, event, strlen(event), out, sizeof(out), 90);

			json_object *root = NULL;
			json_object *awsiotObj = NULL;
			json_object *eidObj = NULL;
			json_object *stsObj = NULL;
			root = json_tokener_parse((char *)out);
			json_object_object_get_ex(root, AAE_AWSIOT_PREFIX, &awsiotObj);
			json_object_object_get_ex(awsiotObj, AAE_IPC_EVENT_ID, &eidObj);
			json_object_object_get_ex(awsiotObj, AAE_IPC_STATUS, &stsObj);
			if (!awsiotObj || !eidObj || !stsObj)
				printf("Failed to tunnel test\n");
			else {
				int eid = json_object_get_int(eidObj);
				const char *status = json_object_get_string(stsObj);
				if ((eid == EID_AWSIOT_TUNNEL_TEST) && (!strcmp(status, "0"))) {
					res = 0;
				}
			}
			json_object_put(root);

			if (strcmp(out, "")==0) {
				result_data = get_test_result_data(test_result_template, handler->id, handler->name, res, out);
			}
			else {
				result_data = get_test_result_data(test_json_result_template, handler->id, handler->name, res, out);	
			}

			break;
		}

		case TC_REFRESH_USERTICKET: {

			DECLARE_CLEAR_MEM(char, out, AAE_MAX_IPC_PACKET_SIZE);
			DECLARE_CLEAR_MEM(char, event, AAE_MAX_IPC_PACKET_SIZE);
			
			snprintf(event, sizeof(event), AAE_DDNS_GENERIC_MSG, AAE_EID_DDNS_REFRESH_TOKEN);
			aae_sendIpcMsgAndWaitResp(MASTIFF_IPC_SOCKET_PATH, event, strlen(event), out, sizeof(out), 10);
			
			json_object *root = NULL;
			json_object *ddnsObj = NULL;
			json_object *eidObj = NULL;
			json_object *stsObj = NULL;
			root = json_tokener_parse((char *)out);
			json_object_object_get_ex(root, AAE_DDNS_PREFIX, &ddnsObj);
			json_object_object_get_ex(ddnsObj, AAE_IPC_EVENT_ID, &eidObj);
			json_object_object_get_ex(ddnsObj, AAE_IPC_STATUS, &stsObj);
			if (!ddnsObj || !eidObj || !stsObj)
				printf("Failed to aae_refresh_userticket\n");
			else {
				int eid = json_object_get_int(eidObj);
				const char *status = json_object_get_string(stsObj);
				if ((eid == AAE_EID_DDNS_REFRESH_TOKEN) && (!strcmp(status, "0"))) {
					res = 0;
				}
			}
			json_object_put(root);

			if (strcmp(out, "")==0) {
				result_data = get_test_result_data(test_result_template, handler->id, handler->name, res, out);
			}
			else {
				result_data = get_test_result_data(test_json_result_template, handler->id, handler->name, res, out);	
			}

			break;
		}

		case TC_REFRESH_DEVICETICKET:{

			DECLARE_CLEAR_MEM(char, out, AAE_MAX_IPC_PACKET_SIZE);
			DECLARE_CLEAR_MEM(char, event, AAE_MAX_IPC_PACKET_SIZE);

			snprintf(event, sizeof(event), AAE_HTTPD_REFRESH_DEVICE_TICKET_MSG, AAE_EID_HTTPD_REFRESH_DEVICE_TICKET);
			aae_sendIpcMsgAndWaitResp(MASTIFF_IPC_SOCKET_PATH, event, strlen(event), out, sizeof(out), 60);
			
			json_object *root = NULL;
			json_object *httpdObj = NULL;
			json_object *eidObj = NULL;
			json_object *stsObj = NULL;
			root = json_tokener_parse((char *)out);
			
			json_object_object_get_ex(root, AAE_HTTPD_PREFIX, &httpdObj);
			json_object_object_get_ex(httpdObj, AAE_IPC_EVENT_ID, &eidObj);
			json_object_object_get_ex(httpdObj, AAE_IPC_STATUS, &stsObj);
			if (!httpdObj || !eidObj || !stsObj) {
				// printf("Failed to aae_httpd_refresh_deviceticket1\n");
			}
			else {
				int eid = json_object_get_int(eidObj);
				const char *status = json_object_get_string(stsObj);
				if ((eid == AAE_EID_HTTPD_REFRESH_DEVICE_TICKET) && (!strcmp(status, "0"))) {
					res = 0;
				}
			}
			json_object_put(root);

			if (strcmp(out, "")==0) {
				result_data = get_test_result_data(test_result_template, handler->id, handler->name, -1, "");
			}
			else {
				result_data = get_test_result_data(test_json_result_template, handler->id, handler->name, 0, out);	
			}

			break;
		}
#endif

		case TC_11K_REQ: {

			char result_file[] = "/tmp/11k_result.json";
			char* file_content = NULL;
			int i = 0;
			int valid_count = 0;

			if (check_file_existed(result_file)==0) {
				remove(result_file);
			}

			DECLARE_CLEAR_MEM(char, out, AAE_MAX_IPC_PACKET_SIZE);
			DECLARE_CLEAR_MEM(char, event, AAE_MAX_IPC_PACKET_SIZE);

			DECLARE_CLEAR_MEM(char, sta_mac, 18);
			snprintf(sta_mac, sizeof(sta_mac), "%s", input_params);
			
			snprintf(event, sizeof(event), "{\"CFG\":{\"EID\": %d, \"mac\": \"%s\"}}", EID_RM_11K_REQ, sta_mac);
			aae_sendIpcMsgAndWaitResp(RAST_IPC_SOCKET_PATH, event, strlen(event), out, sizeof(out), 90);

			if (check_file_existed(result_file)==0) {
				file_content = read_file_to_string(result_file);
			 	if (file_content!=NULL) {
			 		
			 		json_object *root = json_tokener_parse((char *)file_content);
					struct array_list* json_array_list = json_object_get_array(root);
                    if (json_array_list!=NULL) {
                        int array_length = json_array_list->length;
                        for (i=0; i<array_length; i++) {
	                        json_object* stsObj = json_object_array_get_idx(root, i);

	                        if (stsObj!=NULL) {
		                        json_object *bssid_obj = NULL;
		                        DECLARE_CLEAR_MEM(char, the_bssid, 18);
		                        if(json_object_object_get_ex(stsObj, "bssid", &bssid_obj))
		                            strlcpy(the_bssid, (char *)json_object_get_string(bssid_obj), sizeof(the_bssid));
		                        
		                        json_object *rssi_obj = NULL;
		                        DECLARE_CLEAR_MEM(char, the_rssi, 5);
		                        if(json_object_object_get_ex(stsObj, "rssi", &rssi_obj))
		                            strlcpy(the_rssi, (char *)json_object_get_string(rssi_obj), sizeof(the_rssi));

		                        if (strncmp(the_bssid, "00:00:00:00:00:00", strlen(the_bssid))!=0) {
		                        	valid_count++;
		                        }

		                        printf("the_bssid=%s, the_rssi=%s\n", the_bssid, the_rssi);
		                    }
	                    }
                    }

					json_object_put(root);
			 	}
			}

			if (valid_count>0) {
				res = 0;
		 		result_data = get_test_result_data(test_json_result_template, handler->id, handler->name, res, file_content);
			}
			else {
				result_data = get_test_result_data(test_result_template, handler->id, handler->name, res, out);
			}

			if (file_content) {
	 			free(file_content);
	 			file_content = NULL;
	 		}

			break;
		}

		case TC_11V_REQ: {
			
			char result_file[] = "/tmp/11v_result.json";
			char* file_content = NULL;
			int i = 0;

			if (check_file_existed(result_file)==0) {
				remove(result_file);
			}

			DECLARE_CLEAR_MEM(char, out, AAE_MAX_IPC_PACKET_SIZE);
			DECLARE_CLEAR_MEM(char, event, AAE_MAX_IPC_PACKET_SIZE);

			DECLARE_CLEAR_MEM(char, sta_mac, 18);
			DECLARE_CLEAR_MEM(char, target_bssid, 18);
			DECLARE_CLEAR_MEM(char, target_channel_tmp, 3);
			int target_channel = 0;

			const char* delimiters = ",";
			char* tmp = strdup(input_params);
			char *token = strtok(tmp, delimiters);

			while (token != NULL) {
				if (i==0) {
					snprintf(sta_mac, sizeof(sta_mac), "%s", token);
				}
				else if (i==1) {
					snprintf(target_bssid, sizeof(target_bssid), "%s", token);
				}
				else if (i==2) {
					snprintf(target_channel_tmp, sizeof(target_channel_tmp), "%s", token);		
					target_channel = atoi(target_channel_tmp);
				}

				token = strtok(NULL, delimiters);

				i++;		   
			}

			if (tmp) {
				free(tmp);
				tmp = NULL;
			}

			snprintf(event, sizeof(event), "{\"CFG\":{\"EID\": %d, \"mac\": \"%s\", \"bssid\": \"%s\", \"channel\": %d}}", EID_RM_11V_REQ, sta_mac, target_bssid, target_channel);
			aae_sendIpcMsgAndWaitResp(RAST_IPC_SOCKET_PATH, event, strlen(event), out, sizeof(out), 90);

			if (check_file_existed(result_file)==0) {
				file_content = read_file_to_string(result_file);
			 	if (file_content!=NULL) {
			 		json_object *root = json_tokener_parse((char *)file_content);

			 		json_object *resultObj = NULL;
			 		DECLARE_CLEAR_MEM(char, the_result, 2);
			 		if(json_object_object_get_ex(root, "result", &resultObj)) {
			 			strlcpy(the_result, (char *)json_object_get_string(resultObj), sizeof(the_result));
			 			if (strncmp(the_result, "0", strlen(the_result))==0) {
		                	res = 0;
		                }
			 		}

					json_object_put(root);
			 	}
			}

			if (res==0) {
		 		result_data = get_test_result_data(test_json_result_template, handler->id, handler->name, res, file_content);
			}
			else {
				result_data = get_test_result_data(test_result_template, handler->id, handler->name, res, out);
			}

			if (file_content) {
	 			free(file_content);
	 			file_content = NULL;
	 		}

			break;
		}

#ifdef RTCONFIG_UPLOADER
		case TC_UPLOADER_LIST_FILES: {
			
			DECLARE_CLEAR_MEM(char, out, AAE_MAX_IPC_PACKET_SIZE);
			DECLARE_CLEAR_MEM(char, event, AAE_MAX_IPC_PACKET_SIZE);

			snprintf(event, sizeof(event), "{\"function_name\": \"list_files\"}");
			aae_sendIpcMsgAndWaitResp(UPLOADER_IPC_SOCKET_PATH, event, strlen(event), out, sizeof(out), 90);

			if (strcmp(out, "")==0) {
				result_data = get_test_result_data(test_result_template, handler->id, handler->name, -1, "");
			}
			else {
				result_data = get_test_result_data(test_json_result_template, handler->id, handler->name, 0, out);	
			}

			break;
		}
#endif
		
		case TC_NETWORKMAP_SCAN: {
			// killall -SIGUSR1 networkmap.

			int index = 0;
			pid_t *pl;
			pid_t *pidList = find_pid_by_name("networkmap");
			int find_networkmap = 0;

			for (pl = pidList; *pl; pl++) {
				kill(pidList[index], SIGUSR1);
				find_networkmap = 1;
				index++;
			}
			
			free(pidList);

			if (find_networkmap==0) {
				result_data = get_test_result_data(test_result_template, handler->id, handler->name, -1, "");
			}
			else {

				const char* ststus = nvram_get("networkmap_scan_status");
				int while_count = 0;

				while(1) {

					sleep(1);

					if (strncmp(ststus, "scan_done", 9)==0) {
						res = 0;
						break;
					}

					while_count++;

					if (while_count>120) {
						//- Greater than 120 sec
						break;
					}
				}

				result_data = get_test_result_data(test_result_template, handler->id, handler->name, res, ststus);	
			}

			break;
		}
		
		case TC_NETWORKMAP_GET_CLIENT_LIST: {
			// killall -SIGCONT networkmap.

			char result_file[] = "/tmp/nmp_client_list";
			char* file_content = NULL;
			int i = 0;

			if (check_file_existed(result_file)==0) {
				remove(result_file);
			}
			
			int index = 0;
			pid_t *pl;
			pid_t *pidList = find_pid_by_name("networkmap");
			int find_networkmap = 0;

			for (pl = pidList; *pl; pl++) {
				kill(pidList[index], SIGCONT);
				find_networkmap = 1;
				index++;
			}
			
			free(pidList);

			if (find_networkmap==0) {
				result_data = get_test_result_data(test_result_template, handler->id, handler->name, -1, "");
			}
			else {
				
				int while_count = 0;

				while(1) {

					sleep(1);

					if (check_file_existed(result_file)==0) {
						file_content = read_file_to_string(result_file);
			 			if (file_content!=NULL) {
							res = 0;
						}

						break;
					}

					while_count++;

					if (while_count>5) {
						//- Greater than 5 sec
						break;
					}
				}

				result_data = get_test_result_data(test_json_result_template, handler->id, handler->name, res, (file_content==NULL) ? "" : file_content);	
			}

			break;
		}

		default: 
			break;
	
	}

	if (result_data!=NULL) {
		output_test_result(output_file, result_data);
		free_test_result_data(result_data);
	}
}

int futest_main(int argc, char *argv[]){

	int opt;
	char *testcase_name = NULL;
    char *input_params = NULL;
    char *output_file = NULL;
    // int verbose = 0;
    
	while ((opt = getopt(argc, argv, "t:i:o:v")) != -1) {

        switch (opt) {

        	case 't':
                testcase_name = optarg;
                break;

            case 'i':
                input_params = optarg;
                break;

            case 'o':
                output_file = optarg;
                break;

            // case 'v':
            //     verbose = 1;
            //     break;

            default:
                fprintf(stderr, "Usage: %s [-t testcase_name] [-i input_params] [-o output_file] [-v]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (testcase_name==NULL) {
    	fprintf(stderr, "Usage: %s [-t testcase_name] [-i input_params] [-o output_file] [-v]\n", argv[0]);
    	exit(EXIT_FAILURE);
    }

    // printf("Testcase name: %s\n", testcase_name);
    // printf("Input params: %s\n", input_params);
    // printf("Output file: %s\n", output_file);
    // printf("Verbose mode: %s\n", verbose ? "on" : "off");

	TESTCASE *handler = NULL;

    for (handler = &CHK_TESTCASE[0]; handler->type >= 0; handler++) {

    	if (strncmp(handler->name, testcase_name, strlen(handler->name))!=0) {
    		continue;
    	}

    	switch (handler->type) {

    		case TYPE_CONNDIAG_LIB: {

    			void* lib_handle = NULL;
    			void* func_sym = NULL;

    			if (lib_load(&lib_handle, "libconn_diag.so")!=0) {
    				printf("Fail to load library\n");
    				break;
    			}

    			if (lib_get_func(lib_handle, handler->name, &func_sym)!=0) {
    				printf("Fail to get function from library %s\n", handler->name);
    				lib_unload(lib_handle);
    				break;
    			}

    			lib_run_func(handler, func_sym, input_params, output_file);

    			lib_unload(lib_handle);

    			break;
			}

			case TYPE_SYSDEP_FUNC: {

				sysdep_run_func(handler, input_params, output_file);
				break;
			}

			case TYPE_PROCESS: {

				process_call_func(handler, input_params, output_file);
				break;
			}

    		default:
    			break;
    	}
    }

#if 0
	printf("Hello world!!!");

	char *error;

	void* handle = dlopen("libconn_diag.so", RTLD_LAZY);
	if(!handle){
		printf("dll get functions error =%s\n", dlerror());
		return -1;	
	}

	unsigned long (*get_mem_info)(char *name);

	get_mem_info = (unsigned long (*)(char *name)) dlsym(handle, "get_mem_info");
	if ((error = dlerror()) != NULL)  {
	        printf("dll get functions error =%s\n", error);
	        dlclose(handle);
	        return 1;
	}

	unsigned long ts1, ts2;

	int mem_available = get_mem_info("MemAvailable")*1024;

	fprintf(stderr, "open success!! mem_available=%d\n", mem_available);

	dlclose(handle);
#endif

	return 0;
}
