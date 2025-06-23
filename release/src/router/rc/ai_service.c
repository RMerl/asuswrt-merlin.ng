/*
	Services support for AI Board
*/

#include <rc.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <shared.h>
#include <json.h>
#include <openssl/md5.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "ai_service.h"

#if defined(RTCONFIG_AI_SERVICE)
static ai_log_level_t g_ai_log_level = AI_LOG_DEFAULT;
static ai_image_t images[IMAGE_NUM] = {
	{"homeassistant.tar", "asus", "2025.2.5", NULL, NULL},
	{"frigate.tar", "asus", "0.16.0", NULL, NULL}
};

static char *ai_level_to_str(ai_log_level_t level){
	switch(level){
		case(AI_LOG_ERROR): return "ERROR";	
		case(AI_LOG_WARN): return "WARN";	
		case(AI_LOG_INFO): return "INFO";	
		case(AI_LOG_DEBUG): return "DEBUG";	
		default: return "DEFAULT";
	}
}

void aiprint(ai_log_level_t log_level, const char * logpath, const char * format, ...)
{
	FILE *f;
	int nfd;
	va_list args, args_f, args_s, args_c;
	struct stat st;
	char newname[128] = {0};
	time_t rawtime;
        struct tm *timeinfo;
        char timestr[64];
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", timeinfo);

	if(!logpath)
	{
		return;
	}

	if(stat(logpath, &st) != -1)
	{
		if(st.st_size > 1024*1024)
		{
			snprintf(newname, sizeof(newname), "%s.1", logpath);
			rename(logpath, newname);
		}
	}
	// for file, syslog and (console or stderr)
	va_start(args, format);
	va_copy(args_f, args);
	va_copy(args_s, args);
	va_copy(args_c, args);

	if(log_level < g_ai_log_level){
		if(((nfd = open(logpath, O_WRONLY | O_NONBLOCK | O_CREAT, S_IWUSR | S_IRUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)) > 0) &&
		    (f = fdopen(nfd, "a")))
		{
			fprintf(f, "[%s]", timestr);
			vfprintf(f, format, args_f);
			fprintf(f, "\n");
			fclose(f);
			//The fclose() function performs a close() on the file descriptor that is associated with the stream pointed to by 'f'.
			//Thus no need to close(nfd) again.
			nfd = -1;
		}
		if(nfd != -1)
		{
			close(nfd);
		}
		vsyslog(0, format, args_s);
	}
	if(((nfd = open("/dev/console", O_WRONLY | O_NONBLOCK)) > 0) && (f = fdopen(nfd, "w")))
	{
		fprintf(f, "[%s]", timestr);
		vfprintf(f, format, args_c);
		fprintf(f, "\n");
		fclose(f);
		nfd = -1;
	}
	else
	{
		fprintf(stderr, "[%s]", timestr);
		vfprintf(stderr, format, args_c);
		fprintf(stderr, "\n");
	}

	if(nfd != -1)
	{
		close(nfd);
	}
	// close for these copy
	va_end(args);
	va_end(args_f);
	va_end(args_s);
	va_end(args_c);
	
}

// ##### COMMUNICATION KEY #####

/*
 * Function: update_ai_key
 * Params: public_key_filename, tftp_filename
 * Description: Update router's RSA public key to tftp server.
 * Return: 0 if success, else ...
 */
int update_ai_key(char *public_key_filename, char *tftp_filename) {
	int ret = -1;
	char cmd[256] = {0};
	// copy public key to tftp server
	memset(cmd, 0, sizeof(cmd));
	snprintf(cmd, sizeof(cmd), "cp %s/%s %s/%s", ROUTER_KEY_BASE_PATH, public_key_filename, BASE_PATH, tftp_filename);
	ret = system(cmd);
	if(ret) AI_LOG(AI_LOG_ERROR, AI_LOG_PATH, "Fail to update router's public key %s/%s", BASE_PATH, tftp_filename);
	return ret;
}

/*
 * Function: verify_ai_key
 * Params: NULL
 * Description: Verify router's RSA private/public key.
 * 		Encrypt/decrypt a test file and compare the md5 of original file and decrypted file.
 * Return: 0 if success, else ...
 */
int verify_ai_key(char *private_key_filename, char *public_key_filename) {
	int ret = -1;
	char cmd[512] = {0}, orig_tf_path[64] = {0}, enc_tf_path[64] = {0}, dec_tf_path[64] = {0}, orig_md5_s[64] = {0}, dec_md5_s[64] = {0};
	unsigned char computed_orig_md5[MD5_DIGEST_LENGTH], computed_dec_md5[MD5_DIGEST_LENGTH];
	
	snprintf(orig_tf_path, sizeof(orig_tf_path), "%s/%s", ROUTER_KEY_BASE_PATH, AI_TEST_FILE);
	snprintf(enc_tf_path, sizeof(enc_tf_path), "%s/%s", ROUTER_KEY_BASE_PATH, AI_ENC_TEST_FILE);
	snprintf(dec_tf_path, sizeof(dec_tf_path), "%s/%s", ROUTER_KEY_BASE_PATH, AI_DEC_TEST_FILE);

	// generate test file 
	memset(cmd, 0, sizeof(cmd));
	//snprintf(cmd, sizeof(cmd), "echo \"testfile\" > %s/%s", ROUTER_KEY_BASE_PATH, AI_TEST_FILE);
	snprintf(cmd, sizeof(cmd), "echo \"testfile\" > %s", orig_tf_path);
	ret = system(cmd);
	if(ret){
		AI_LOG(AI_LOG_ERROR, AI_LOG_PATH, "Fail to generate test file");
		goto VERIFY_END;
	}
	// encrypt test file
	memset(cmd, 0, sizeof(cmd));
	snprintf(cmd, sizeof(cmd), "openssl pkeyutl -encrypt -inkey %s/%s -pubin -in %s -out %s", ROUTER_KEY_BASE_PATH, public_key_filename, orig_tf_path, enc_tf_path);
	ret = system(cmd);
	if(ret){
		AI_LOG(AI_LOG_ERROR, AI_LOG_PATH, "Fail to generate encrypted test file");
		goto VERIFY_END;
	}
	// decrypt test file
	memset(cmd, 0, sizeof(cmd));
	snprintf(cmd, sizeof(cmd), "openssl pkeyutl -decrypt -inkey %s/%s -in %s -out %s", ROUTER_KEY_BASE_PATH, private_key_filename, enc_tf_path, dec_tf_path); 
	ret = system(cmd);
	if(ret){
		AI_LOG(AI_LOG_ERROR, AI_LOG_PATH, "Fail to decrypted test file");
		goto VERIFY_END;
	}

	// compare original test file md5 and decrypted test file md5 to ensure enc/dec works correctly
	ret = compute_md5(orig_tf_path, computed_orig_md5);
	if(ret){
		AI_LOG(AI_LOG_ERROR, AI_LOG_PATH, "Fail to compute original testfile's md5");
		goto VERIFY_END;
	}
	ret = compute_md5(dec_tf_path, computed_dec_md5);
	if(ret){
		AI_LOG(AI_LOG_ERROR, AI_LOG_PATH, "Fail to compute decrypted testfile's md5");
		goto VERIFY_END;
	}
	md5_to_string(computed_orig_md5, orig_md5_s);
	md5_to_string(computed_dec_md5, dec_md5_s);
	if(strcmp(orig_md5_s, dec_md5_s)){
		ret = -1;
		AI_LOG(AI_LOG_ERROR, AI_LOG_PATH, "Orig md5 [%s] not equals to decrypt md5 [%s]", orig_md5_s, dec_md5_s);
	}
	else AI_LOG(AI_LOG_INFO, AI_LOG_PATH, "Key verify successfully");
VERIFY_END:
	return ret;
}
/*
 * Function: gen_ai_key
 * Params: NULL
 * Description: Generate router's RSA private/public key pair.
 * Return: 0 if success, else ...
 */
int gen_ai_key(char *private_key_filename, char *public_key_filename) {
	int ret = -1;
	char cmd[512] = {0};

	// generate rsa private key
	memset(cmd, 0, sizeof(cmd));
	snprintf(cmd, sizeof(cmd), "openssl genpkey -algorithm RSA -out %s/%s -pkeyopt rsa_keygen_bits:2048", ROUTER_KEY_BASE_PATH, private_key_filename);
	ret = system(cmd);
	if(ret){
		AI_LOG(AI_LOG_ERROR, AI_LOG_PATH, "Fail to generate router's RSA private key");
		_dprintf("%s Fail to generate router's RSA private key\n", AISRV);
		return ret;
	}

	// extract rsa public from private key
	memset(cmd, 0, sizeof(cmd));
	snprintf(cmd, sizeof(cmd), "openssl rsa -pubout -in %s/%s -out %s/%s", ROUTER_KEY_BASE_PATH, private_key_filename, ROUTER_KEY_BASE_PATH, public_key_filename);
	ret = system(cmd);
	if(ret) AI_LOG(AI_LOG_ERROR, AI_LOG_PATH, "Fail to extract router's RSA public key");
	return ret;
}

/*
 * Function: renew_ai_key
 * Params: private_key_filename: private key filename
 * 	   public_key_filename: public key filename
 * 	   tftp_filename: the public key's final filename on tftp server
 * Description: Renew router's RSA private/public key pair.
 * 		AI board will use public key to encryt AES key.
 * 		Key pair will be generated when boot or fail to decrypt response.
 * Return: 0 if success, else ...
 */
int renew_ai_key(char *private_key_filename, char *public_key_filename, char *tftp_filename) {
	int ret = -1, retry = 3;
	while(retry > 0){
		ret = gen_ai_key(private_key_filename, public_key_filename);
		if(ret) goto RETRY;
		ret = verify_ai_key(private_key_filename, public_key_filename);
		if(ret) goto RETRY;
		ret = update_ai_key(public_key_filename, tftp_filename);
		if(ret) goto RETRY;
RETRY:
		retry--;
	}

	return ret;
}

// ##### COMMUNICATION KEY #####


/*
 * Function: start_ai_upgrade
 * Params: NULL
 * Description: execute AI Board firmware upgrade process
 * webs_state_ai_error => 4(aiboard fetch firmware failed), 5(aiboard upgrade failed)
 * webs_state_ai_upgrade => 1(Download ai board firmware from server success), 2(ai board fetch firmware from router success), 3(ai board upgrade success)
 * It's for auto firmware upgrade.
 * Return: 0
 */
int start_ai_upgrade(char *filename) {
	int ret = -1, upgrade_stage = 0, error_state = 0;
	int ai_tx_timeout = 90;
        int ai_upg_timeout = 300;
        time_t ai_tx_start_time, ai_upg_start_time;
	
	error_state = nvram_get_int("webs_state_ai_error");

	if(!error_state)
	{
		AI_LOG(AI_LOG_INFO, AI_LOG_PATH, "Init AI FW upgrade");
		upgrade_stage = nvram_get_int("webs_state_ai_upgrade");
		nvram_set("ai_fw_path", filename);
		
		ret = send_ai_request(AI_UPD_REQ);
		if(ret){
			nvram_set("webs_state_ai_error", "5");
			AI_LOG(AI_LOG_ERROR, AI_LOG_PATH, "Failed to send ai upgrade request");
			goto END;	
		}
		// succeed in notify ai board update request
		nvram_set("webs_state_ai_upgrade", "1");	
		// upgrade_stage 1 -> 2, router success to download firmware from server and ai board start to fetch it
		ai_tx_start_time = time(NULL);
		while(upgrade_stage == 1)
		{
			if(difftime(time(NULL), ai_tx_start_time) >= ai_tx_timeout)
			{
				nvram_set("webs_state_ai_error", "6");
				break;
			}
			upgrade_stage = nvram_get_int("webs_state_ai_upgrade");
		}
		error_state = nvram_get_int("webs_state_ai_error");
		if(error_state)
		{
			AI_LOG(AI_LOG_ERROR, AI_LOG_PATH, "Transmit FW timeout [%d]", error_state);
			goto END;
		}

		// upgrade_stage 2 -> 3, ai board success to fetch firmware and start to upgrade
		ai_upg_start_time = time(NULL);
		while(upgrade_stage == 2)
		{
			if(difftime(time(NULL), ai_upg_start_time) >= ai_upg_timeout)
			{
				nvram_set("webs_state_ai_error", "7");
				break;
			}
			upgrade_stage = nvram_get_int("webs_state_ai_upgrade");
		}
		error_state = nvram_get_int("webs_state_ai_error");
		if(error_state)
		{
			AI_LOG(AI_LOG_ERROR, AI_LOG_PATH, "Upgrade FW timeout [%d]", error_state);
			goto END;
		}
	}
END:
	nvram_set("webs_state_ai_upgrade", "0");
	nvram_set("webs_state_ai_error", "0");
	return 0;
}

/* Function: response_parser
 * Params: Decrypted `filepath`(JSON file) which put from ai board
 * Description: Parse the content of RSP/FWUP file
 * Return: 0
 */
static int response_parser(const char *filepath) {
	int ret = -1, i = 0;
	size_t cmd_len = 0, dns_len = 0, res_len = 0;
	
	// json root and common info
	struct json_object *root=NULL, *action=NULL, *commands=NULL, *next=NULL;
	char *command[MAX_RSP][MAX_CMD_LEN] = {{0}};
	char *tmp = NULL;
	
	// *_key used to be the key in nvram
	// *_s used to be the value in nvram
	// json (query)system response
	struct json_object *system=NULL, *sys_status=NULL, *sys_data=NULL, *sys_os_version=NULL, *sys_kernel_version=NULL, *sys_cpu_usage_percentage=NULL, *sys_total_memory_kb=NULL, *sys_free_memory_kb=NULL, *sys_hw_version=NULL, *sys_firmware_version=NULL, *sys_firmware_commit_number=NULL, *sys_firmware_commit_hash=NULL, *sys_sdk_version=NULL, *sys_sdk_commit_hash=NULL, *sys_ethernet=NULL, *sys_ip_address=NULL, *sys_netmask=NULL, *sys_gateway=NULL, *sys_dns=NULL;
	char sys_status_s[16] = {0}, os_version[128] = {0}, kernel_version[32] = {0}, hw_version[16] = {0}, fw_version[16] = {0}, fw_commit_number[16] = {0}, fw_commit_hash[16] = {0}, sdk_version[16] = {0}, sdk_commit_hash[16] = {0}, ip_address[32] = {0}, netmask[32] = {0}, gateway[32] = {0}, dns[32] = {0}, total_memory_kb[16] = {0}, free_memory_kb[16] = {0}, cpu_usage_percentage[16] = {0};
	
	// json (apply)save response
	struct json_object *save=NULL, *sav_status=NULL,*sav_results=NULL, *sav_result=NULL, *sav_result_command=NULL, *sav_result_status=NULL, *sav_result_error_code=NULL, *sav_result_reason=NULL;
	char sav_status_s[8] = {0}, sav_res_command_s[32] = {0}, sav_res_status_s[8] = {0}, sav_res_error_code_s[8] = {0}, sav_res_reason_s[64] = {0};
	char sav_res_command_key[64] = {0}, sav_res_status_key[64] = {0}, sav_res_error_code_key[64] = {0}, sav_res_reason_key[64] = {0};

	// json (reset)default response
	struct json_object *def_default=NULL, *def_status=NULL, *def_error_code=NULL, *def_reason=NULL;
	char def_status_s[8] = {0}, def_error_code_s[8] = {0}, def_reason_s[64] = {0};

	// json (update)progress response
	struct json_object *progress=NULL, *prog_status=NULL, *prog_error_code=NULL, *prog_reason=NULL, *prog_total_steps=NULL, *prog_current_step=NULL, *prog_current_percent=NULL, *prog_current_image=NULL;
	char prog_status_s[16] = {0}, prog_error_code_s[8] = {0}, prog_reason_s[64] = {0}, prog_total_steps_s[8] = {0}, prog_current_step_s[8] = {0}, prog_current_percent_s[8] = {0}, prog_current_image_s[32] = {0};
	
	// json (feedback)feedback default response
	struct json_object *feedback=NULL, *fb_status=NULL;
	char fb_status_s[8] = {0};

	// json (communication error) key/file enc/dec failed response
	struct json_object *comm_err=NULL, *comm_reason=NULL, *comm_code=NULL;
	char comm_reason_s[128] = {0}, comm_code_s[8] = {0};
	
	root = json_object_from_file(filepath);
	if (!root)
	{
		AI_LOG(AI_LOG_ERROR, AI_LOG_PATH, "Fail to open [%s]", filepath);
		return ret;
	}
	AI_LOG(AI_LOG_DEBUG, AI_LOG_PATH, "Start to parse %s", filepath);
    	// parsing
	if (json_object_object_get_ex(root, "action", &action)) {
		if (json_object_object_get_ex(action, "command", &commands)) {
			AI_LOG(AI_LOG_DEBUG, AI_LOG_PATH, "Commands: %s", json_object_to_json_string(commands));
			if (json_object_get_type(commands) == json_type_array) {
				cmd_len = json_object_array_length(commands);
				for (i = 0; i < cmd_len && i < MAX_RSP; i++) {
					next = json_object_array_get_idx(commands, i);
					tmp = json_object_get_string(next);
					strncpy(command[i], tmp, MAX_CMD_LEN - 1);
					command[i][MAX_CMD_LEN - 1] = '\0';
				}
			}
		}
	}
	// apply -> save, update -> progress, query -> system, reset -> default	
	if (json_object_object_get_ex(root, "system", &system)) {
		if (json_object_object_get_ex(system, "status", &sys_status)) {
			strlcpy(sys_status_s, json_object_get_string(sys_status), sizeof(sys_status_s));
			nvram_set("ai_sys_status", sys_status_s);
			AI_LOG(AI_LOG_DEBUG, AI_LOG_PATH, "System Status: %s", sys_status_s);
		}
		if (json_object_object_get_ex(system, "data", &sys_data)) {
			if (json_object_object_get_ex(sys_data, "os_version", &sys_os_version)) {
				strlcpy(os_version, json_object_get_string(sys_os_version), sizeof(os_version));
				nvram_set("ai_sys_os_version", os_version);
				AI_LOG(AI_LOG_DEBUG, AI_LOG_PATH, "OS Version: %s", os_version);
			}
			if (json_object_object_get_ex(sys_data, "kernel_version", &sys_kernel_version)) {
				strlcpy(kernel_version, json_object_get_string(sys_kernel_version), sizeof(kernel_version));
				nvram_set("ai_sys_kernel_version", kernel_version);
				AI_LOG(AI_LOG_DEBUG, AI_LOG_PATH, "Kernel Version: %s", kernel_version);
			}
			if (json_object_object_get_ex(sys_data, "cpu_usage_percentage", &sys_cpu_usage_percentage)) {
				strlcpy(cpu_usage_percentage, json_object_get_string(sys_cpu_usage_percentage), sizeof(cpu_usage_percentage));
				nvram_set("ai_sys_cpu_usage_percentage", cpu_usage_percentage);
				AI_LOG(AI_LOG_DEBUG, AI_LOG_PATH, "CPU Usage Percentage: %s", json_object_get_string(sys_cpu_usage_percentage));
			}
			if (json_object_object_get_ex(sys_data, "total_memory_kb", &sys_total_memory_kb)) {
				strlcpy(total_memory_kb, json_object_get_string(sys_total_memory_kb), sizeof(total_memory_kb));
				nvram_set("ai_sys_total_memory_kb", total_memory_kb);
				AI_LOG(AI_LOG_DEBUG, AI_LOG_PATH, "Total Memory KB: %s", json_object_get_string(sys_total_memory_kb));
			}
			if (json_object_object_get_ex(sys_data, "free_memory_kb", &sys_free_memory_kb)) {
				strlcpy(free_memory_kb, json_object_get_string(sys_free_memory_kb), sizeof(free_memory_kb));
				nvram_set("ai_sys_free_memory_kb", free_memory_kb);
				AI_LOG(AI_LOG_DEBUG, AI_LOG_PATH, "Free Memory KB: %s", json_object_get_string(sys_free_memory_kb));
			}
			if (json_object_object_get_ex(sys_data, "hw_version", &sys_hw_version)) {
				strlcpy(hw_version, json_object_get_string(sys_hw_version), sizeof(hw_version));
				nvram_set("ai_sys_hw_version", hw_version);
				AI_LOG(AI_LOG_DEBUG, AI_LOG_PATH, "HW Version: %s", hw_version);
			}
			if (json_object_object_get_ex(sys_data, "firmware_version", &sys_firmware_version)) {
				strlcpy(fw_version, json_object_get_string(sys_firmware_version), sizeof(fw_version));
				nvram_set("ai_sys_fw_version", fw_version);
				AI_LOG(AI_LOG_DEBUG, AI_LOG_PATH, "Firmware version: %s", fw_version);
			}
			if (json_object_object_get_ex(sys_data, "firmware_commit_number", &sys_firmware_commit_number)) {
				strlcpy(fw_commit_number, json_object_get_string(sys_firmware_commit_number), sizeof(fw_commit_number));
				nvram_set("ai_sys_fw_commit_number", fw_commit_number);
				AI_LOG(AI_LOG_DEBUG, AI_LOG_PATH, "Firmware commit number: %s", fw_commit_number);
			}
			if (json_object_object_get_ex(sys_data, "firmware_commit_hash", &sys_firmware_commit_hash)) {
				strlcpy(fw_commit_hash, json_object_get_string(sys_firmware_commit_hash), sizeof(fw_commit_hash));
				nvram_set("ai_sys_fw_commit_hash", fw_commit_hash);
				AI_LOG(AI_LOG_DEBUG, AI_LOG_PATH, "Firmware commit hash: %s", fw_commit_hash);
			}
			if (json_object_object_get_ex(sys_data, "sdk_version", &sys_sdk_version)) {
				strlcpy(sdk_version, json_object_get_string(sys_sdk_version), sizeof(sdk_version));
				nvram_set("ai_sys_sdk_version", sdk_version);
				AI_LOG(AI_LOG_DEBUG, AI_LOG_PATH, "sdk version: %s", sdk_version);
			}
			if (json_object_object_get_ex(sys_data, "sdk_commit_hash", &sys_sdk_commit_hash)) {
				strlcpy(sdk_commit_hash, json_object_get_string(sys_sdk_commit_hash), sizeof(sdk_commit_hash));
				nvram_set("ai_sys_sdk_commit_hash", sdk_commit_hash);
				AI_LOG(AI_LOG_DEBUG, AI_LOG_PATH, "sdk commit hash: %s", sdk_commit_hash);
			}
			if (json_object_object_get_ex(sys_data, "ethernet", &sys_ethernet)) {
				if (json_object_object_get_ex(sys_ethernet, "ip_address", &sys_ip_address)) {
					strlcpy(ip_address, json_object_get_string(sys_ip_address), sizeof(ip_address));
					nvram_set("ai_sys_ip_address", ip_address);
					AI_LOG(AI_LOG_DEBUG, AI_LOG_PATH, "Ethernet IP Address: %s", ip_address);
				}
				if (json_object_object_get_ex(sys_ethernet, "netmask", &sys_netmask)) {
					strlcpy(netmask, json_object_get_string(sys_netmask), sizeof(netmask));
					nvram_set("ai_sys_netmask", netmask);
					AI_LOG(AI_LOG_DEBUG, AI_LOG_PATH, "Ethernet Netmask: %s", sys_netmask);
				}
				if (json_object_object_get_ex(sys_ethernet, "gateway", &sys_gateway)) {
					strlcpy(gateway, json_object_get_string(sys_gateway), sizeof(gateway));
					nvram_set("ai_sys_gateway", gateway);
					AI_LOG(AI_LOG_DEBUG, AI_LOG_PATH, "Ethernet Gateway: %s", gateway);
				}
				if (json_object_object_get_ex(sys_ethernet, "dns", &sys_dns)) {
					// concate the dns str and store in nvram
					if (json_object_get_type(sys_dns) == json_type_array) {
						dns_len = json_object_array_length(sys_dns);
						for (i = 0; i < dns_len && i < MAX_DNS_NUM; i++) {
							next = json_object_array_get_idx(sys_dns, i);
							tmp = json_object_get_string(next);
							if(i > 0){
								strlcat(dns, ">", sizeof(dns));
							}
							strlcat(dns, tmp, sizeof(dns));
						}
						AI_LOG(AI_LOG_DEBUG, AI_LOG_PATH, "Ethernet DNS: %s", dns);
						nvram_set("ai_sys_dns", dns);
					}
				}
			}
		}
	} 

	if (json_object_object_get_ex(root, "save", &save)) {
		if (json_object_object_get_ex(save, "status", &sav_status)) {
			strlcpy(sav_status_s, json_object_get_string(sav_status), sizeof(sav_status_s));
			nvram_set("ai_sav_status", sav_status_s);
			AI_LOG(AI_LOG_DEBUG, AI_LOG_PATH, "Save Status: %s", sav_status_s);
		}
		if (json_object_object_get_ex(save, "results", &sav_results)) {
			if (json_object_get_type(sav_results) == json_type_array) {
				res_len = json_object_array_length(sav_results);
				// cause there's several return status, etc
				// so we iter for each and store the result seperately.
				for (i = 0; i < res_len; i++) {
					sav_result = json_object_array_get_idx(sav_results, i);
					if (json_object_object_get_ex(sav_result, "command", &sav_result_command)) {
						// clean up key and str's buffer
						memset(sav_res_command_key, 0, sizeof(sav_res_command_key));
						memset(sav_res_command_s, 0, sizeof(sav_res_command_s));
						
						// copy current command and mark in the nvram key
						strlcpy(sav_res_command_s, json_object_get_string(sav_result_command), sizeof(sav_res_command_s));
						// set nvram key based on command
						snprintf(sav_res_command_key, sizeof(sav_res_command_key), "ai_sav_res_%s_command", sav_res_command_s);
						nvram_set(sav_res_command_key, sav_res_command_s);
						AI_LOG(AI_LOG_DEBUG, AI_LOG_PATH, "Result Command %s: %s", sav_res_command_key, sav_res_command_s);
					}
					if (json_object_object_get_ex(sav_result, "status", &sav_result_status)) {
						memset(sav_res_status_key, 0, sizeof(sav_res_status_key));
						memset(sav_res_status_s, 0, sizeof(sav_res_status_s));

						strlcpy(sav_res_status_s, json_object_get_string(sav_result_status), sizeof(sav_res_status_s));
						snprintf(sav_res_status_key, sizeof(sav_res_status_key), "ai_sav_res_%s_status", sav_res_command_s);
						
						nvram_set(sav_res_status_key, sav_res_status_s);
						AI_LOG(AI_LOG_DEBUG, AI_LOG_PATH, "Result Status %s: %s", sav_res_status_key, sav_res_status_s);
					}
					if (json_object_object_get_ex(sav_result, "error_code", &sav_result_error_code)) {
						memset(sav_res_error_code_key, 0, sizeof(sav_res_error_code_key));
						memset(sav_res_error_code_s, 0, sizeof(sav_res_error_code_s));
						
						strlcpy(sav_res_error_code_s, json_object_get_string(sav_result_error_code), sizeof(sav_res_error_code_s));
						snprintf(sav_res_error_code_key, sizeof(sav_res_error_code_key), "ai_sav_res_%s_error_code", sav_res_command_s);
						
						nvram_set(sav_res_error_code_key, sav_res_error_code_s);
						AI_LOG(AI_LOG_DEBUG, AI_LOG_PATH, "Result Error Code %s: %s", sav_res_error_code_key, sav_res_error_code_s);
					}
					if (json_object_object_get_ex(sav_result, "reason", &sav_result_reason)) {
						memset(sav_res_reason_key, 0, sizeof(sav_res_reason_key));
						memset(sav_res_reason_s, 0, sizeof(sav_res_reason_s));
						
						strlcpy(sav_res_reason_s, json_object_get_string(sav_result_reason), sizeof(sav_res_reason_s));
						snprintf(sav_res_reason_key, sizeof(sav_res_reason_key), "ai_sav_res_%s_reason", sav_res_command_s);
						
						nvram_set(sav_res_reason_key, sav_res_reason_s);
						AI_LOG(AI_LOG_DEBUG, AI_LOG_PATH, "Result Reason %s: %s", sav_res_reason_key, sav_res_reason_s);
					}
				}
			}
		}
	}

	if (json_object_object_get_ex(root, "default", &def_default)) {
		if (json_object_object_get_ex(def_default, "status", &def_status)) {
			strlcpy(def_status_s, json_object_get_string(def_status), sizeof(def_status_s));
			nvram_set("ai_def_status", def_status_s);
			AI_LOG(AI_LOG_DEBUG, AI_LOG_PATH, "Default Status: %s", def_status_s);
		}
		if (json_object_object_get_ex(def_default, "error_code", &def_error_code)) {
			strlcpy(def_error_code_s, json_object_get_string(def_error_code), sizeof(def_error_code_s));
			nvram_set("ai_def_error_code", def_error_code_s);
			AI_LOG(AI_LOG_DEBUG, AI_LOG_PATH, "Default Error Code: %s", def_error_code_s);
		}
		if (json_object_object_get_ex(def_default, "reason", &def_reason)) {
			strlcpy(def_reason_s, json_object_get_string(def_reason), sizeof(def_reason_s));
			nvram_set("ai_def_reason", def_reason_s);
			AI_LOG(AI_LOG_DEBUG, AI_LOG_PATH, "Default Reason: %s", def_reason_s);
		}
	}

	if (json_object_object_get_ex(root, "progress", &progress)) {
		if (json_object_object_get_ex(progress, "status", &prog_status)) {
			strlcpy(prog_status_s, json_object_get_string(prog_status), sizeof(prog_status_s));
			nvram_set("ai_prog_status", prog_status_s);
			AI_LOG(AI_LOG_DEBUG, AI_LOG_PATH, "Progress Status: %s", prog_status_s);
		}
		if (json_object_object_get_ex(progress, "error_code", &prog_error_code)) {
			strlcpy(prog_error_code_s, json_object_get_string(prog_error_code), sizeof(prog_error_code_s));
			nvram_set("ai_prog_error_code", prog_error_code_s);
			AI_LOG(AI_LOG_DEBUG, AI_LOG_PATH, "Progress Error Code: %s", prog_error_code_s);
		}
		if (json_object_object_get_ex(progress, "reason", &prog_reason)) {
			strlcpy(prog_reason_s, json_object_get_string(prog_reason), sizeof(prog_reason_s));
			nvram_set("ai_prog_reason", prog_reason_s);
			AI_LOG(AI_LOG_DEBUG, AI_LOG_PATH, "Progress Reason: %s", prog_reason_s);
		}
		if (json_object_object_get_ex(progress, "total_steps", &prog_total_steps)) {
			strlcpy(prog_total_steps_s, json_object_get_string(prog_total_steps), sizeof(prog_total_steps_s));
			nvram_set("ai_prog_total_steps", prog_total_steps_s);
			AI_LOG(AI_LOG_DEBUG, AI_LOG_PATH, "Progress Total Steps: %s", prog_total_steps_s);
		}
		if (json_object_object_get_ex(progress, "current_step", &prog_current_step)) {
			strlcpy(prog_current_step_s, json_object_get_string(prog_current_step), sizeof(prog_current_step_s));
			nvram_set("ai_prog_current_step", prog_current_step_s);
			AI_LOG(AI_LOG_DEBUG, AI_LOG_PATH, "Progress Current Step: %s", prog_current_step_s);
		}
		if (json_object_object_get_ex(progress, "current_percent", &prog_current_percent)) {
			strlcpy(prog_current_percent_s, json_object_get_string(prog_current_percent), sizeof(prog_current_percent_s));
			nvram_set("ai_prog_current_percent", prog_current_percent_s);
			AI_LOG(AI_LOG_DEBUG, AI_LOG_PATH, "Progress Current Percent: %s", prog_current_percent_s);
		}
		if (json_object_object_get_ex(progress, "current_image", &prog_current_image)) {
			strlcpy(prog_current_image_s, json_object_get_string(prog_current_image), sizeof(prog_current_image_s));
			nvram_set("ai_prog_current_image", prog_current_image_s);
			AI_LOG(AI_LOG_DEBUG, AI_LOG_PATH, "Progress Current Image: %s", prog_current_image_s);
		}
		
		// webs_state_ai_upgrade : 2 is success fetch fw from router, 3 is success upgrade
		// First, check fw download succ or not
		if(!strcmp(prog_status_s, AI_PROG_DNLD) && !strcmp(prog_error_code_s, "0") && !strcmp(prog_current_percent_s, "100"))
		{
			AI_LOG(AI_LOG_INFO, AI_LOG_PATH, "Download fw succ !!!");
			nvram_set("webs_state_ai_upgrade", "2");
		}
		// firmware update last step is postinstall.sh. Currently, there's two state DONE and SUCCESS seems same?
		else if((!strcmp(prog_status_s, AI_PROG_DONE) || !strcmp(prog_status_s, AI_PROG_SUCC)) && !strcmp(prog_current_image_s, "postinstall.sh"))
		{
			AI_LOG(AI_LOG_INFO, AI_LOG_PATH, "Upgrade fw succ !!!");
			nvram_set("webs_state_ai_upgrade", "3");
		}
	}
	
	if (json_object_object_get_ex(root, "feedback", &feedback)) {
		if (json_object_object_get_ex(feedback, "status", &fb_status)) {
			strlcpy(fb_status_s, json_object_get_string(fb_status), sizeof(fb_status_s));
			nvram_set("ai_fb_status", fb_status_s);
			AI_LOG(AI_LOG_DEBUG, AI_LOG_PATH, "Feedback Status: %s", fb_status_s);
		}
	}

	if (json_object_object_get_ex(root, "error", &comm_err)) {
		if (json_object_object_get_ex(comm_err, "reason", &comm_reason)) {
			strlcpy(comm_reason_s, json_object_get_string(comm_reason), sizeof(comm_reason_s));
			// nvram_set("ai_comm_reason", fb_status_s);
			AI_LOG(AI_LOG_ERROR, AI_LOG_PATH, "AI Communication error reason: %s", comm_reason_s);
		}
		if (json_object_object_get_ex(comm_err, "error_code", &comm_code)) {
			strlcpy(comm_code_s, json_object_get_string(comm_code), sizeof(comm_code_s));
			nvram_set("ai_comm_code", comm_code_s);
			AI_LOG(AI_LOG_ERROR, AI_LOG_PATH, "AI Communication error code: %s", comm_code_s);
		}
	}

	json_object_put(root);

	return ret;
}
/* Function: ai_responses_check_main
 * Params: None
 * Description: A daemon process used to detect and handle RSP/FWUP file
 * - Monitor by watchdog 
 * Return: 0
 */
int ai_response_check_main(int argc, char *argv[]) {
	int ifd = -1, wd = -1;
	char buffer[BUF_LEN] = {0};
	int length = -1;
	// keep checking for new response md5 file exist or not
	ifd = inotify_init();
	if (ifd < 0)
	{
		AI_LOG(AI_LOG_ERROR, AI_LOG_PATH, "inotify_init failed");
		exit(EXIT_FAILURE);
	}
	wd = inotify_add_watch(ifd, BASE_PATH, IN_CLOSE_WRITE);
	while(1)
	{
		// The read() function will block, meaning it will wait until an event occurs.
		length = read(ifd, buffer, BUF_LEN);
		if (length < 0)
		{
			AI_LOG(AI_LOG_ERROR, AI_LOG_PATH, "Failed to read inotify fd");
			exit(EXIT_FAILURE);
		}

		for (int i = 0; i < length;)
		{
			struct inotify_event *event = (struct inotify_event *)&buffer[i];
			if((event) && (event->len))
			{
				if((strstr(ENC_RSP_MD5, event->name) != 0) || (strstr(ENC_FWUP_MD5, event->name) != 0))
				{
					if(event->mask & IN_CLOSE_WRITE)
					{
						if(!strcmp(event->name, MONITOR_RSP_FILE)) recv_ai_response(ENC_RSP_MD5, ENC_RSP_MSG, ENC_RSP_AES, DEC_RSP_AES, DEC_RSP_JSON);
						else if(!strcmp(event->name, MONITOR_FWUP_FILE)) recv_ai_response(ENC_FWUP_MD5, ENC_FWUP_MSG, ENC_FWUP_AES, DEC_FWUP_AES, DEC_FWUP_JSON);
						else AI_LOG(AI_LOG_WARN, AI_LOG_PATH, "Name: [%s]", event->name);
						
					}
				}
				i += EVENT_SIZE + event->len;
			}
		}
	}

	inotify_rm_watch(ifd, wd);
	close(ifd);

	return 0;
}

static void md5_to_string(const unsigned char *md5_hash, char *md5_string) {
    for (int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
        sprintf(&md5_string[i * 2], "%02x", (unsigned int)md5_hash[i]);
    }
    md5_string[MD5_DIGEST_LENGTH * 2] = '\0';
}

static int compute_md5(const char *filename, unsigned char *md5_hash) {
	FILE *fp = NULL;
	size_t bytes_read;
	unsigned char buffer[1024];


	if(!(fp = fopen(filename, "rb"))) {
		AI_LOG(AI_LOG_ERROR, AI_LOG_PATH, "Failed to read file %s", filename);
		return -1;
	}
	MD5_CTX md5_ctx;
	MD5_Init(&md5_ctx);

	while ((bytes_read = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
		MD5_Update(&md5_ctx, buffer, bytes_read);
	}

	MD5_Final(md5_hash, &md5_ctx);
	fclose(fp);
	return 0;
}

static int read_md5(const char *filename, char *md5_buf) {
	FILE *file = fopen(filename, "r");
	if (!file) {
		AI_LOG(AI_LOG_ERROR, AI_LOG_PATH, "Failed open md5 file %s", filename);
		return -1;
	}

	if (fgets(md5_buf, MD5_HASH_SIZE + 1, file) == NULL) {
		AI_LOG(AI_LOG_ERROR, AI_LOG_PATH, "Failed to read md5 %s", filename);
		fclose(file);
		return -1;
	}

	// Remove newline character if it's present
	size_t len = strlen(md5_buf);
	if (len > 0 && md5_buf[len - 1] == '\n') {
		md5_buf[len - 1] = '\0';
	}
	fclose(file);
	return 0;
}

static int compare_md5(unsigned char *calculated_md5, const char *expected_md5) {
	char calculated_md5_hex[MD5_HASH_SIZE + 1];
	for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
		sprintf(&calculated_md5_hex[i * 2], "%02x", calculated_md5[i]);
	}
	calculated_md5_hex[MD5_HASH_SIZE] = '\0';
	AI_LOG(AI_LOG_DEBUG, AI_LOG_PATH, "calc md5: [%s] | expected md5: [%s]", calculated_md5_hex, expected_md5);
	return strcmp(calculated_md5_hex, expected_md5);
}

/* Function: gen_docker_images_list
 * Params: None
 * Description: Generate docker_images_list json file for reset to default
 * Return: 0 if success, else -1
 */
static int gen_docker_images_list(void){
	int i = 0, ret = 0;
	char *json_str = NULL;
	unsigned char computed_md5[MD5_DIGEST_LENGTH];
	char cmd[512] = {0}, md5_string[64] = {0}, tmp_value[64] = {0};
	FILE *fp = NULL;
	time_t t = time(NULL);
	struct tm *tm_info = localtime(&t);
	json_object *root = NULL;

	// initialize json content
	root = json_object_new_object();
	
	// write action section
	json_object *metadata = json_object_new_object();
	
	// version : current aisom fw version, date : current datetime
	memset(tmp_value, 0, sizeof(tmp_value));
	strlcpy(tmp_value, nvram_safe_get("ai_sys_fw_version"), sizeof(tmp_value));
	json_object_object_add(metadata, "version", json_object_new_string(tmp_value));
	memset(tmp_value, 0, sizeof(tmp_value));
	strftime(tmp_value, sizeof(tmp_value), "%Y-%m-%d", tm_info);
	json_object_object_add(metadata, "date", json_object_new_string(tmp_value));
	json_object_object_add(root, "metadata", metadata);
	
	json_object *ai_images = json_object_new_array();
	for(i = 0; i < IMAGE_NUM; i++){
		json_object *image = json_object_new_object();
		json_object_object_add(image, "filename", json_object_new_string(images[i].filename));
		json_object_object_add(image, "repository", json_object_new_string(images[i].repository));
		json_object_object_add(image, "tag", json_object_new_string(images[i].tag));
		
		memset(tmp_value, 0, sizeof(tmp_value));
		memset(computed_md5, 0, sizeof(computed_md5));
		memset(md5_string, 0, sizeof(md5_string));

		snprintf(tmp_value, sizeof(tmp_value), "%s/%s", IMAGE_BASE_PATH, images[i].filename);
		if(!f_exists(tmp_value)){
			ret = -1;
			goto END;
		}
		ret = compute_md5(tmp_value, computed_md5);
		if (ret) goto END;

		md5_to_string(computed_md5, md5_string);
		json_object_object_add(image, "md5sum", json_object_new_string(md5_string));
		json_object_object_add(image, "extra_command", json_object_new_string(""));
		json_object_array_add(ai_images, image);
	}
	json_object_object_add(root, "images", ai_images);
	
	// casting from json obj to char *
	json_str = json_object_to_json_string_ext(root, JSON_C_TO_STRING_PRETTY);
	 
	// write json file
	if ((fp = fopen(DEC_IMG_JSON, "w")) != NULL) {
		// write action section
		if (fprintf(fp, "%s", json_str) < 0){
			AI_LOG(AI_LOG_ERROR, AI_LOG_PATH, "Failed to write image list content: %s", DEC_IMG_JSON);
			ret = -1;
		}
		fclose(fp);
		if(ret) goto END;
	}
	else {
		AI_LOG(AI_LOG_ERROR, AI_LOG_PATH, "Failed to create request content file %s", DEC_IMG_JSON);
		ret = -1;
		goto END;
	}
	AI_LOG(AI_LOG_DEBUG, AI_LOG_PATH, "Gen IMAGE list [%s]", json_str);
	
	// remove the json complement '\'
	memset(cmd, 0, sizeof(cmd));
	snprintf(cmd, sizeof(cmd), "sed -i 's/\\\\//g' %s", DEC_IMG_JSON);
	ret = system(cmd);
	if(ret) AI_LOG(AI_LOG_ERROR, AI_LOG_PATH, "Failed to clean up \\ in %s - [%d]", DEC_IMG_JSON, ret);
END:
	json_object_put(root);
	return ret;
}

/*
 * Function: send_ai_request
 * Description: pull GPIO 21 to notify aiboard fetching files
 * 1. write json file
 * 2. Generate aes key
 * 3. encrypt json file with aes key
 * 4. encrypt aes key with rsa pub key
 * 5. pull GPIO 21 to low to notify ai board fetching CTRL_MSG.enc and KEY.enc
 * 6. remove aes key and json file
 * Return: 0 if success, else ... 
 */
int send_ai_request(int req_flag){
	int ret = 0, retry = 3, rescue = 0;
	char *json_str = NULL, *dns_token = NULL;
	const char delim[2] = ">";
	unsigned char computed_md5[MD5_DIGEST_LENGTH];
	char cmd[512] = {0}, fw_md5_string[64] = {0}, fw_abs_path[128] = {0}, tmp_value[64] = {0}, aisom_key_path[64] = {0};
	FILE *fp = NULL;
	json_object *root = NULL;

REQ_RETRY:
	// Initialize reboot or not (If it is reset to default/rescue update, don't reboot)
	if(req_flag & AI_RSC_REQ) rescue = 1;
	// initialize json content
	root = json_object_new_object();
	
	// write action section
	json_object *action = json_object_new_object();
	
	//supported values -- update: firmware upgrade, apply: apply settings, reset: restore default settings, query: Query the system information
	json_object *command = json_object_new_array();
	if(req_flag & AI_STA_REQ) json_object_array_add(command, json_object_new_string(AI_REQ_QUERY));
 	if(req_flag & AI_UPD_REQ) json_object_array_add(command, json_object_new_string(AI_REQ_UPDATE));
 	if(req_flag & AI_SET_REQ) json_object_array_add(command, json_object_new_string(AI_REQ_APPLY));
 	if(req_flag & AI_RST_REQ) json_object_array_add(command, json_object_new_string(AI_REQ_RESET));
	if(req_flag & AI_FBK_REQ) json_object_array_add(command, json_object_new_string(AI_REQ_FEEDBACK));
	
	json_object_object_add(action, "command", command);
	json_object_object_add(action, "__comment", json_object_new_string(""));
	json_object_object_add(root, "action", action);
	
	// write setting section
	if(req_flag & AI_SET_REQ)
	{
		json_object *setting = json_object_new_object();
		if(strlen(nvram_safe_get("ai_ssh_enable"))) {
 			memset(tmp_value, 0, sizeof(tmp_value));
 			strlcpy(tmp_value, nvram_safe_get("ai_ssh_enable"), sizeof(tmp_value));
			json_object_object_add(setting, "ssh_enable", json_object_new_string(tmp_value));
			//nvram_set("ai_ssh_enable", "");
		}
		if(strlen(nvram_safe_get("ai_ssh_acc"))) {
 			memset(tmp_value, 0, sizeof(tmp_value));
 			strlcpy(tmp_value, nvram_safe_get("ai_ssh_acc"), sizeof(tmp_value));
			json_object_object_add(setting, "ssh_acc", json_object_new_string(tmp_value));
			//nvram_set("ai_ssh_acc", "");
		}
		if(strlen(nvram_safe_get("ai_ssh_pass"))) {
 			memset(tmp_value, 0, sizeof(tmp_value));
 			strlcpy(tmp_value, nvram_safe_get("ai_ssh_pass"), sizeof(tmp_value));
			json_object_object_add(setting, "ssh_pass", json_object_new_string(tmp_value));
			//nvram_set("ai_ssh_pass", "");
		}
		if(strlen(nvram_safe_get("ai_lan_proto"))) {
 			memset(tmp_value, 0, sizeof(tmp_value));
 			strlcpy(tmp_value, nvram_safe_get("ai_lan_proto"), sizeof(tmp_value));
			json_object_object_add(setting, "lan_proto", json_object_new_string(tmp_value));
			//nvram_set("ai_lan_proto", "");
		}
		if(strlen(nvram_safe_get("ai_lan_ip"))) {
 			memset(tmp_value, 0, sizeof(tmp_value));
 			strlcpy(tmp_value, nvram_safe_get("ai_lan_ip"), sizeof(tmp_value));
			json_object_object_add(setting, "lan_ip", json_object_new_string(tmp_value));
			//nvram_set("ai_lan_ip", "");
		}
		if(strlen(nvram_safe_get("ai_lan_netmask"))) {
 			memset(tmp_value, 0, sizeof(tmp_value));
 			strlcpy(tmp_value, nvram_safe_get("ai_lan_netmask"), sizeof(tmp_value));
			json_object_object_add(setting, "lan_netmask", json_object_new_string(tmp_value));
			//nvram_set("ai_lan_netmask", "");
		}
		if(strlen(nvram_safe_get("ai_lan_gateway"))) {
 			memset(tmp_value, 0, sizeof(tmp_value));
 			strlcpy(tmp_value, nvram_safe_get("ai_lan_gateway"), sizeof(tmp_value));
			json_object_object_add(setting, "lan_gateway", json_object_new_string(tmp_value));
			//nvram_set("ai_lan_gateway", "");
		}
		if(strlen(nvram_safe_get("ai_dns")))
		{
			json_object *lan_dns = json_object_new_array();
 			memset(tmp_value, 0, sizeof(tmp_value));
 			strlcpy(tmp_value, nvram_safe_get("ai_dns"), sizeof(tmp_value));
			dns_token = strtok(tmp_value, delim);
			while(dns_token != NULL)
			{
				AI_LOG(AI_LOG_DEBUG, AI_LOG_PATH,"dns_token: %s", dns_token);
				json_object_array_add(lan_dns, json_object_new_string(dns_token));
				dns_token = strtok(NULL, delim);
			}
			json_object_object_add(setting, "lan_dns", lan_dns);
		}
		json_object_object_add(root, "setting", setting);
		
		// firmware path settings
 		json_object *fw_setting = json_object_new_object();
 		if(strlen(nvram_safe_get("ai_fw_path"))) {
 			memset(tmp_value, 0, sizeof(tmp_value));
 			strlcpy(tmp_value, nvram_safe_get("ai_fw_path"),sizeof(tmp_value));
 			json_object_object_add(fw_setting, "path", json_object_new_string(tmp_value));
 			//nvram_set("ai_lan_netmask", "");
 		}
 		json_object_object_add(root, "firmware", fw_setting);
	}
	if(req_flag & AI_UPD_REQ)
	{
		// write firmware section
		json_object *firmware = json_object_new_object();
 		memset(tmp_value, 0, sizeof(tmp_value));

		if(strlen(nvram_safe_get("ai_fw_path"))) strlcpy(tmp_value, nvram_safe_get("ai_fw_path"),sizeof(tmp_value));
		else strlcpy(tmp_value, DEFAULT_FW_PATH, sizeof(tmp_value));
		
		json_object_object_add(firmware, "path", json_object_new_string(tmp_value));
		// compute fw md5
		strlcpy(fw_abs_path, "/ai/", sizeof(fw_abs_path));
		strlcat(fw_abs_path, tmp_value, sizeof(fw_abs_path));
		if (compute_md5(fw_abs_path, computed_md5) != 0) {
			return -1;
		}
		md5_to_string(computed_md5, fw_md5_string);
		AI_LOG(AI_LOG_DEBUG, AI_LOG_PATH,"md5 string %s", fw_md5_string);

		json_object_object_add(firmware, "md5sum", json_object_new_string(fw_md5_string));
		json_object_object_add(root, "firmware", firmware);
	}
	
	// casting from json obj to char *
	json_str = json_object_to_json_string_ext(root, JSON_C_TO_STRING_PRETTY);
	 
	// write json file
	if ((fp = fopen(DEC_REQ_JSON, "w")) != NULL) {
		if (fprintf(fp, "%s", json_str) < 0){
			AI_LOG(AI_LOG_ERROR, AI_LOG_PATH, "Failed to write request content: %s", DEC_REQ_JSON);
			ret = -1;
		}
		fclose(fp);
		if(ret) goto REQ_END;
	}
	else {
		AI_LOG(AI_LOG_ERROR, AI_LOG_PATH, "Failed to create request content file: %s", DEC_REQ_JSON);
		goto REQ_END;
	}
	
	// remove the json complement '\'
	memset(cmd, 0, sizeof(cmd));
	snprintf(cmd, sizeof(cmd), "sed -i 's/\\\\//g' %s", DEC_REQ_JSON);
	ret = system(cmd);
	if(ret) {
		AI_LOG(AI_LOG_ERROR, AI_LOG_PATH, "Failed to clean up \\ in %s - [%d]", DEC_REQ_JSON, ret);
		goto REQ_END;
	}
	
	// check whether AISOM's public key exists
	// If it's not exist, generate a dummy key pair and send to aisom to trigger aisom put its public onto tftp server
	snprintf(aisom_key_path, sizeof(aisom_key_path), "%s/%s", BASE_PATH, AISOM_PUBLIC_KEY);
	if(!f_exists(aisom_key_path)){ 
		ret = renew_ai_key(AISOM_DUMMY_PRIVATE_KEY, AISOM_DUMMY_PUBLIC_KEY, AISOM_PUBLIC_KEY);
		if(ret) goto REQ_END;
	}
	ret = gen_msg_key(DEC_REQ_AES);
	if(ret) goto REQ_END;
	ret = encrypt_json(DEC_REQ_JSON, ENC_REQ_MSG);
	if(ret) goto REQ_END;
	ret = compute_enc_json_md5(ENC_REQ_MSG, ENC_REQ_MD5);
	if(ret) goto REQ_END;
	ret = encrypt_aes_key(DEC_REQ_AES, ENC_REQ_AES);
	if(ret) goto REQ_END;
	
	// reset to default/rescue need to generate an additional docker_image_list json file
	// It also use Update command, but with reboot flag
	if((req_flag & AI_UPD_REQ) && rescue){
		ret = gen_docker_images_list();
		if(ret) goto REQ_END;
		ret = gen_msg_key(DEC_IMG_AES);
		if(ret) goto REQ_END;
		ret = encrypt_json(DEC_IMG_JSON, ENC_IMG_MSG);
		if(ret) goto REQ_END;
		ret = compute_enc_json_md5(ENC_IMG_MSG, ENC_IMG_MD5);
		if(ret) goto REQ_END;
		ret = encrypt_aes_key(DEC_IMG_AES, ENC_IMG_AES);
		if(ret) goto REQ_END;
	}

	// Always verify router's private/public key pair to ensure that it can encrypt/decrypt correctly
	ret = verify_ai_key(ROUTER_PRIVATE_KEY, ROUTER_PUBLIC_KEY);
	if(ret) ret = renew_ai_key(ROUTER_PRIVATE_KEY, ROUTER_PUBLIC_KEY, ROUTER_PUBLIC_KEY);
	else ret = update_ai_key(ROUTER_PUBLIC_KEY, ROUTER_PUBLIC_KEY);

	if(ret) goto REQ_END;

	if(rescue){
		AI_LOG(AI_LOG_INFO, AI_LOG_PATH, "It's a reset to default/rescue REQ. Reboot to rescue mode");
		goto REQ_END;
	}
	ret = trigger_aisom(AISOM_GPIO_LOW, AISOM_MSG);
	usleep(500000);
	ret = trigger_aisom(AISOM_GPIO_HIGH, AISOM_MSG);
	AI_LOG(AI_LOG_DEBUG, AI_LOG_PATH, "Send REQ [%s] to ai board", json_str);
REQ_END:
	// release json obj
	json_object_put(root);
	if(ret && retry > 0){
		retry--;
		AI_LOG(AI_LOG_ERROR, AI_LOG_PATH, "Send REQ [%d] to ai board was failed, retry[%d]", req_flag, retry);
		goto REQ_RETRY;
	}
	else AI_LOG(AI_LOG_INFO, AI_LOG_PATH, "Succeed in sending REQ[%d] to ai board", req_flag);
	return ret;
}

static int gen_msg_key(char *dec_aes_key)
{
	int ret = -1;
	char cmd[512] = {0};
	// generate key
	memset(cmd, 0, sizeof(cmd));
	snprintf(cmd, sizeof(cmd), "openssl rand -hex -out %s 32", BASE_KEY);
	ret = system(cmd);
	if(ret) {
		AI_LOG(AI_LOG_ERROR, AI_LOG_PATH, "Failed to generate key [%s] - [%d]", BASE_KEY, ret);
		goto END;
	}
	// generate iv
	memset(cmd, 0, sizeof(cmd));
	snprintf(cmd, sizeof(cmd), "openssl rand -hex -out %s 16", BASE_IV);
	ret = system(cmd);
	if(ret) {
		AI_LOG(AI_LOG_ERROR, AI_LOG_PATH, "Failed to generate iv [%s] - [%d]", BASE_IV, ret);
		goto END;
	}
	// generate aes key
	memset(cmd, 0, sizeof(cmd));
	snprintf(cmd, sizeof(cmd), "cat %s %s > %s", BASE_KEY, BASE_IV, dec_aes_key);
	ret = system(cmd);
	if(ret){
		AI_LOG(AI_LOG_ERROR, AI_LOG_PATH, "Failed to generate aes key [%s]- [%d]", dec_aes_key, ret);
	}
END:
	return ret;
}
static int encrypt_json(char *dec_json, char *enc_json)
{
	int ret = -1;
	char cmd[512] = {0};
	// encrypt json file with aes key
	memset(cmd, 0, sizeof(cmd));
	snprintf(cmd, sizeof(cmd), "openssl enc -aes-256-cbc -in %s -out %s -K $(cat %s) -iv $(cat %s)", dec_json, enc_json, BASE_KEY, BASE_IV);
	ret = system(cmd);
	if(ret) AI_LOG(AI_LOG_ERROR, AI_LOG_PATH, "Failed to encrypt [%s] json file", dec_json);
	return ret;
}

static int compute_enc_json_md5(char *enc_json, char *enc_json_md5)
{
	int ret = -1;
	char cmd[512] = {0};
	// md5 the encrypted json file
	memset(cmd, 0, sizeof(cmd));
	snprintf(cmd, sizeof(cmd), "md5sum %s | cut -d ' ' -f 1 > %s", enc_json, enc_json_md5);
	ret = system(cmd);
	if(ret) AI_LOG(AI_LOG_ERROR, AI_LOG_PATH, "Failed to generate encrypt json file [%s] to md5 [%s]", enc_json, enc_json_md5);
	return ret;
}
static int encrypt_aes_key(char *dec_aes_key, char *enc_aes_key)
{
	int ret = -1;
	char cmd[512] = {0};
	
	// encrypt aes key
	memset(cmd, 0, sizeof(cmd));
	snprintf(cmd, sizeof(cmd), "openssl pkeyutl -encrypt -pubin -inkey %s/%s -in %s -out %s", BASE_PATH, AISOM_PUBLIC_KEY, dec_aes_key, enc_aes_key);
	ret = system(cmd);
	// If failed, need to generate a dummy public key to enforce send a Bad REQ to AISOM, trigger AISOM key updating mechanism
	// If aisom_pub.pem is replacing by hacker, it will also trigger AISOM key updating mechanism
	if(ret) {
		AI_LOG(AI_LOG_ERROR, AI_LOG_PATH, "Failed to encrypt aes key [%s] to [%s]", dec_aes_key, enc_aes_key);
		renew_ai_key(AISOM_DUMMY_PRIVATE_KEY, AISOM_DUMMY_PUBLIC_KEY, AISOM_PUBLIC_KEY);
	}
	return ret;
}

/*
 * Function: trigger_aisom 
 * params: val(specific value), pin(GPIO pin)
 * Description: Pull and reset GPIO pin for 
 * Return: 0 if success, else ...
 */
static int trigger_aisom(ai_volt_t val, ai_action_t pin)
{
	int ret = 0;
	char cmd[512] = {0};
	// pull GPIO pin to notify aiboard fetch the encoded json and key file
	memset(cmd, 0, sizeof(cmd));
	snprintf(cmd, sizeof(cmd), "echo %d > /sys/class/leds/led_gpio_%d/brightness", val, pin);
	ret = system(cmd);
	if(ret) AI_LOG(AI_LOG_ERROR, AI_LOG_PATH, "Fail to pull %d to %d", pin, val);
	else AI_LOG(AI_LOG_INFO, AI_LOG_PATH, "Succeed in pulling %d to %d", pin, val);
	return ret;
}

/*
 * Function: recv_ai_response
 * params: enc_*_md5, enc_*_msg, enc_*_aes, dec_*_aes, dec_*_json
 * Description: Decrypt RSA/AES/JSON (RSP/FWUP) file
 * Return: 0 if success, else -1
 */
static int recv_ai_response(char *enc_f_md5, char *enc_f_msg, char *enc_f_aes, char *dec_f_aes, char *dec_f_json){
	char cmd[256] = {0};
	int ret = 0;

	unsigned char computed_md5[MD5_DIGEST_LENGTH];
	char expected_md5[MD5_HASH_SIZE + 1];

    	// Step 1: Compute MD5 of the file to check
	if (compute_md5(enc_f_msg, computed_md5) != 0) {
		AI_LOG(AI_LOG_ERROR, AI_LOG_PATH, "compute %s and %s MD5", enc_f_msg, computed_md5);
		ret = -1;
		goto RECV_END;
	}

    	// Step 2: Read expected MD5 checksum from file
	if (read_md5(enc_f_md5, expected_md5) != 0) {
		AI_LOG(AI_LOG_ERROR, AI_LOG_PATH, "read %s and %s MD5!", enc_f_msg, expected_md5);
		ret = -1;
		goto RECV_END;
	}

    	// Step 3: Compare the computed MD5 with the expected MD5
	if (compare_md5(computed_md5, expected_md5) == 0) {
		AI_LOG(AI_LOG_DEBUG, AI_LOG_PATH, "%s and %s MD5 checksum match!", enc_f_msg, enc_f_md5);
	} else {
		AI_LOG(AI_LOG_ERROR, AI_LOG_PATH, "%s and %s MD5 checksum do not match.", enc_f_msg, enc_f_md5);
		ret = -1;
		goto RECV_END;
	}

	// decrypt aes key
	memset(cmd, 0, sizeof(cmd));
	snprintf(cmd, sizeof(cmd), "openssl pkeyutl -decrypt -inkey %s/%s -in %s -out %s", ROUTER_KEY_BASE_PATH, ROUTER_PRIVATE_KEY, enc_f_aes, dec_f_aes);
	ret = system(cmd);
	if(ret) {
		AI_LOG(AI_LOG_ERROR, AI_LOG_PATH, "Failed to decrypt aes key [%s] to [%s]", enc_f_aes, dec_f_aes);
		
		// When failing to decrypt the aes key, it means private/public key was broken.
		// Need to verify original key pair via enc/dec.
		// If it's also failed, renew a key pair, else only update the public key to tftp server.
		ret = verify_ai_key(ROUTER_PRIVATE_KEY, ROUTER_PUBLIC_KEY);
		if(ret) renew_ai_key(ROUTER_PRIVATE_KEY, ROUTER_PUBLIC_KEY, ROUTER_PUBLIC_KEY);
		else update_ai_key(ROUTER_PUBLIC_KEY, ROUTER_PUBLIC_KEY);
		
		goto RECV_END;
	}

	// decrypt json file
	memset(cmd, 0, sizeof(cmd));
	snprintf(cmd, sizeof(cmd), "openssl enc -d -aes-256-cbc -in %s -out %s -K $(head -n 1 %s) -iv $(tail -n 1 %s)", enc_f_msg, dec_f_json, dec_f_aes, dec_f_aes);
	ret = system(cmd);
	if(ret) {
		AI_LOG(AI_LOG_ERROR, AI_LOG_PATH, "Failed to decrypt json file [%s] to [%s]", enc_f_msg, dec_f_json);
		goto RECV_END;
	}

	// only parse the RSP/FWUP json if decrypt successfully
	response_parser(dec_f_json);
RECV_END:
	return ret;
}

int start_ai_reboot(void){
	int ret = -1;
	ret = trigger_aisom(AISOM_GPIO_LOW, AISOM_REBOOT);
	usleep(500000);
	ret = trigger_aisom(AISOM_GPIO_HIGH, AISOM_REBOOT);
	return ret;
}

void start_ai_tftpd(void) {
	pid_t pid;
	char *ip = "169.254.0.1";
	char *tftpdir = "/ai";
	char username[256] = {0};
	memset(username, 0, sizeof(username));
	strlcpy(username, nvram_safe_get("http_username"), sizeof(username));
	char *tftpd_argv[] = { "in.tftpd", "-4lcvs", "-u", username, "--address", ip, tftpdir, NULL };
	
	if (getpid() != 1) {
		notify_rc_after_wait("start_ai_tftpd");
		return;
	}

	killall_tk("in.tftpd");
	if (!pids("in.tftpd")){
		AI_LOG(AI_LOG_WARN, AI_LOG_PATH, "AI tftpd Off");
		_eval(tftpd_argv, NULL, 0, &pid);
	}
	if (pids("in.tftpd")){
		AI_LOG(AI_LOG_INFO, AI_LOG_PATH, "AI tftpd On");
	}
}

void stop_ai_tftpd(int force)
{
	if(!force && getpid() != 1){
		notify_rc_after_wait("stop_ai_tftpd");
		return;
	}

	killall_tk("in.tftpd");
	AI_LOG(AI_LOG_WARN, AI_LOG_PATH,"Stop AI tftpd");
}

void stop_ai_response_check(void)
{
        killall_tk("ai_response_check");
	AI_LOG(AI_LOG_WARN, AI_LOG_PATH,"Stop AI response check daemon");
}

int start_ai_response_check(void)
{
        char *ai_response_argv[] = {"ai_response_check", NULL};
        pid_t aircpid;
	AI_LOG(AI_LOG_INFO, AI_LOG_PATH,"Start AI response check daemon");
        return _eval(ai_response_argv, NULL, 0, &aircpid);
}

void stop_ai_service(void){
	stop_ai_response_check();
	stop_ai_tftpd(0);
	return;
}

void start_ai_service(void){
	start_ai_response_check();
	start_ai_tftpd();
	return;
}
#endif	/* RTCONFIG_AI_SERVICE */


