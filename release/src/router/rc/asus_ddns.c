#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <rc.h>
#include <bcmnvram.h>
#include <shared.h>
#include <shutils.h>
#include <curl/curl.h>
#ifdef RTCONFIG_HTTPS
#include <openssl/md5.h>
#endif
#include <json.h>
#include <notify_rc.h>
#include <aae_ipc.h>


#define ASUSDDNS_IP_SERVER          "https://ns1.asuscomm.com"
#define ASUSDDNS_IP_SERVER_CN       "https://ns1.asuscomm.cn"
#define ASUSDDNS_REQ_TOKEN_PATH     "/ddnsv2/acquireToken.php"
#define ASUSDDNS_REQ_TOKEN_RES      "/tmp/asusddns_res"

#define ASUSDDNS_ERR_INIT_STATE             0
#define ASUSDDNS_ERR_SUCCESS                1
#define ASUSDDNS_ERR_NO_DM_REFRESH_TICKET   -1
#define ASUSDDNS_ERR_NO_USER_TICKET         -2
#define ASUSDDNS_ERR_NO_DDNS_TOKEN          -3
#define ASUSDDNS_ERR_CURL_ERR               -4
#define ASUSDDNS_ERR_OPEN_FILE_FAIL         -5
#define ASUSDDNS_ERR_JSON_ERR               -6
#define ASUSDDNS_ERR_NO_SID                 -7

#define MD5_DIGEST_BYTES 16

static void hmac_md5( const unsigned char *input, size_t ilen, unsigned char *output)
{
	MD5_CTX ctx;

	MD5_Init(&ctx);
	MD5_Update(&ctx, input, ilen);
	MD5_Final(output, &ctx);
}

int checkTokenExpiration(const char *expireTime)
{
	if (expireTime == NULL || strlen(expireTime) == 0) {
		return -1;
	}

	// Get current UTC time
	time_t current_time;
	time(&current_time);

	// Try parsing as "YYYY-MM-DD HH:MM:SS" format
	struct tm expire_tm;
	if (strptime(expireTime, "%Y-%m-%d %H:%M:%S", &expire_tm) != NULL) {
		// Convert parsed time to time_t format (in UTC)
		time_t expire_time = timegm(&expire_tm);
		// Compare current time and expiry time
		//logmessage("ddns_token", "current_time=%ld", (long)current_time);
		//logmessage("ddns_token", "expire_time =%ld", (long)expire_time);
		if (expire_time == -1) {
			logmessage("ddns_token", "Convert time to time_t format failed");
			return -1;
		}

		return (current_time >= expire_time) ? 1 : 0; // Compare expiry dates
	}

	// Try parsing as a timestamp
	char *endptr;
	time_t timestamp = (time_t)strtoll(expireTime, &endptr, 10);
	if (*endptr == '\0') {
		return (current_time >= timestamp) ? 1 : 0;
	}

	// Parsing failed
	logmessage("ddns_token", "Invalid expireTime format: %s\n", expireTime);
	return -1;
}

static int _update_userticket()
{
	char event[AAE_MAX_IPC_PACKET_SIZE];
	char out[AAE_MAX_IPC_PACKET_SIZE];
	char* oauth_dm_user_ticket_expiretime = nvram_safe_get("oauth_dm_user_ticket_expiretime");
	logmessage("ddns_token", "Old userticket expiretime=%s", oauth_dm_user_ticket_expiretime);
	/*** Debug
	char *endptr;
	time_t oauth_dm_user_ticket_expiretime_t = (time_t)strtoll(oauth_dm_user_ticket_expiretime, &endptr, 10);
	struct tm *tm_utc = gmtime(&oauth_dm_user_ticket_expiretime_t);
	char userticket_expiretime[20];
	strftime(userticket_expiretime, sizeof(userticket_expiretime), "%Y-%m-%d %H:%M:%S", tm_utc);
	logmessage("ddns_token", "Old userticket expiretime =%s(UTC)", userticket_expiretime);
	**********/
	if (checkTokenExpiration(oauth_dm_user_ticket_expiretime) != 0)
	{
		if(strlen(nvram_safe_get("oauth_dm_refresh_ticket")) == 0)	//APP not registered.
		{
			return ASUSDDNS_ERR_NO_DM_REFRESH_TICKET;
		}

		logmessage("ddns_token", "Update userticket!");
		snprintf(event, sizeof(event), AAE_DDNS_GENERIC_MSG, AAE_EID_DDNS_REFRESH_TOKEN);
		
		aae_sendIpcMsgAndWaitResp(MASTIFF_IPC_SOCKET_PATH, event, strlen(event), out, sizeof(out), 5);
		
		json_object *root = NULL;
		json_object *ddnsObj = NULL;
		json_object *eidObj = NULL;
		json_object *stsObj = NULL;
		root = json_tokener_parse((char *)out);
		json_object_object_get_ex(root, AAE_DDNS_PREFIX, &ddnsObj);
		json_object_object_get_ex(ddnsObj, AAE_IPC_EVENT_ID, &eidObj);
		json_object_object_get_ex(ddnsObj, AAE_IPC_STATUS, &stsObj);
		if (!ddnsObj || !eidObj || !stsObj)
		{
			logmessage("ddns_token", "Failed to aae_refresh_ticket 1");
		}
		else
		{
			int eid = json_object_get_int(eidObj);
			const char *status = json_object_get_string(stsObj);
			if ((eid == AAE_EID_DDNS_REFRESH_TOKEN) && (!strcmp(status, "0")))
			{
				logmessage("ddns_token", "Success to aae_refresh_ticket");
			}
			else
			{
				logmessage("ddns_token", "Failed to aae_refresh_ticket 2");
			}
		}
		json_object_put(root);
	}
	else {
		logmessage("ddns_token", "userticket hasn't expired yet");
	}

/*	stop_mastiff();
	start_mastiff();
	sleep(1);*/
	
	return ASUSDDNS_ERR_SUCCESS;	
}

static size_t _write_data(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	size_t written = fwrite(ptr, size, nmemb, stream);
	return written;
}

void delete_char(char str[], char ch)
{
	int i, j;
	for(i = j = 0; str[i] != '\0'; i++)
		if(str[i] != ch)
			str[j++] = str[i];

	str[j] = '\0';
}

static int _acquire_token(const char *res_path, const int check_CA)
{
	CURLcode res = -1;
	char *cusid = NULL, *userticket = NULL, *auth_status = NULL;
	char ddns_url[256], devicemac[64];
	json_object *obj = NULL, *cusid_obj = NULL, *userticket_obj = NULL, *devicemac_obj = NULL, *devicemd5mac_obj = NULL, *sid_obj = NULL;
	unsigned char digest[MD5_DIGEST_BYTES]={0};
	char md_lan_mac[MD5_DIGEST_BYTES * 2 + 1]={0};
	char *lan_mac_str=NULL;
	const char *auth_string = NULL;
	int ret = ASUSDDNS_ERR_INIT_STATE, i;

	if(!res_path)
	{
		logmessage("ddns_token", "No response file path.");
		return ASUSDDNS_ERR_OPEN_FILE_FAIL;
	}
	
	cusid = nvram_safe_get("oauth_dm_cusid");
	userticket = nvram_safe_get("oauth_dm_user_ticket");
	auth_status = nvram_safe_get("oauth_auth_status");

	if(cusid[0] == '\0' || userticket[0] == '\0')
	{
		logmessage("ddns_token", "No userticket.");
		return ASUSDDNS_ERR_NO_USER_TICKET;
	}
	if (auth_status[0] == '\0' || (strncmp(auth_status, "0", 1) != 0 && strncmp(auth_status, "2", 1) != 0))
	{
		logmessage("ddns_token", "No sid (service id).");
		return ASUSDDNS_ERR_NO_SID;
	}


	obj = json_object_new_object();
	if(!obj)
	{
		logmessage("ddns_token", "Cannot create json object!");
		ret = ASUSDDNS_ERR_JSON_ERR;
		goto Err;
	}

	cusid_obj = json_object_new_string(cusid);
	if(cusid_obj)
		json_object_object_add(obj, "cusid", cusid_obj);
	else
	{
		logmessage("ddns_token", "Cannot add json object, cusid!");
		ret = ASUSDDNS_ERR_JSON_ERR;
		goto Err;
	}

	userticket_obj = json_object_new_string(userticket);
	if(userticket_obj)
		json_object_object_add(obj, "userticket", userticket_obj);
	else
	{
		logmessage("ddns_token", "Cannot add json object, userticket!");
		ret = ASUSDDNS_ERR_JSON_ERR;
		goto Err;
	}

	snprintf(devicemac, sizeof(devicemac), "%s", get_ddns_macaddr());
	delete_char(devicemac, ':');
	devicemac_obj = json_object_new_string(devicemac);
	if(devicemac_obj)
		json_object_object_add(obj, "devicemac", devicemac_obj);
	else
	{
		logmessage("ddns_token", "Cannot add json object, devicemac!");
		ret = ASUSDDNS_ERR_JSON_ERR;
		goto Err;
	}

	lan_mac_str = nvram_safe_get("lan_hwaddr");
	hmac_md5((unsigned char *)lan_mac_str, strlen(lan_mac_str), digest);
	for (i = 0; i < MD5_DIGEST_BYTES; i++)
	{
		sprintf(&md_lan_mac[i*2], "%02x", (unsigned int)digest[i]);
	}
	
	devicemd5mac_obj = json_object_new_string(md_lan_mac);
	if(devicemd5mac_obj)
		json_object_object_add(obj, "devicemd5mac", devicemd5mac_obj);
	else
	{
		logmessage("ddns_token", "Cannot add json object, devicemd5mac!");
		ret = ASUSDDNS_ERR_JSON_ERR;
		goto Err;
	}

#ifdef RTCONFIG_ACCOUNT_BINDING
    	int account_bound = is_account_bound();
#else
    	int account_bound = 0;
#endif

	if (account_bound) {
		sid_obj = json_object_new_string("1004");
	}
	else {
		sid_obj = json_object_new_string("1001");
	}
	
	if(sid_obj)
		json_object_object_add(obj, "sid", sid_obj);
	else
	{
		logmessage("ddns_token", "Cannot add json object, sid!");
		ret = ASUSDDNS_ERR_JSON_ERR;
		goto Err;
	}

	auth_string = json_object_to_json_string(obj);
	logmessage("ddns_token", "auth_string=%s", auth_string);

	int retry = 5;
	while(retry > 0 && res != CURLE_OK){
		FILE *fp_res = NULL;
		CURL *curl = NULL;

		fp_res = fopen(res_path, "wb");
		if(!fp_res)
		{
			logmessage("ddns_token", "Cannot open file for response data.");
			return ASUSDDNS_ERR_OPEN_FILE_FAIL;
		}

		curl = curl_easy_init();

		if(curl)
		{
#ifdef RTCONFIG_ACCOUNT_BINDING
			if (account_bound && nvram_match("ddns_replace_status", "1") &&
				((strstr(nvram_safe_get("aae_ddnsinfo"), ".asuscomm.com") && (strstr(nvram_safe_get("ddns_hostname_x"), ".asuscomm.com")))
				|| (strstr(nvram_safe_get("aae_ddnsinfo"), ".asuscomm.cn") && (strstr(nvram_safe_get("ddns_hostname_x"), ".asuscomm.cn"))))) {
				snprintf(ddns_url, sizeof(ddns_url), "https://%s%s",  nvram_safe_get("aae_ddnsinfo"), ASUSDDNS_REQ_TOKEN_PATH);
			} else
#endif
			if(nvram_match("ddns_server_x", "WWW.ASUS.COM.CN")) {
				snprintf(ddns_url, sizeof(ddns_url), "%s%s",  ASUSDDNS_IP_SERVER_CN, ASUSDDNS_REQ_TOKEN_PATH);
			} else {
				snprintf(ddns_url, sizeof(ddns_url), "%s%s",  ASUSDDNS_IP_SERVER, ASUSDDNS_REQ_TOKEN_PATH);
			}

			curl_easy_setopt(curl, CURLOPT_URL, ddns_url);

			curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);

			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, check_CA); /* do not verify subject/hostname */
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, check_CA); /* since most certs will be self-signed, do not verify against CA */

			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, auth_string);
			curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(auth_string));

			/* enable verbose for easier tracing */
			curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
			curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
			curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120L);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _write_data);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp_res);
			curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);

			res = curl_easy_perform(curl);
			fclose(fp_res);

			curl_easy_cleanup(curl);
			if(res == CURLE_OK)
			{
				logmessage("ddns_token", "CURL perform success");
				ret = ASUSDDNS_ERR_SUCCESS;

				FILE *fp_res_read = fopen(res_path, "r");
				if (fp_res_read) {
					char buffer[256];
					while (fgets(buffer, sizeof(buffer), fp_res_read) != NULL) {
						logmessage("ddns_token", "response: %s", buffer);
					}
					fclose(fp_res_read);
				} else {
					logmessage("ddns_token", "Failed to open %s for reading.", res_path);
				}
			}
			else
			{
				logmessage("ddns_token", "CURL perform fail: %s(%d)", curl_easy_strerror(res), res);
				ret = ASUSDDNS_ERR_CURL_ERR;
				unlink(res_path);
				retry--;
				sleep(1);
			}
		}
		else
		{
			logmessage("ddns_token", "Cannot init CURL");
			ret = ASUSDDNS_ERR_CURL_ERR;
			//goto Err;
		}
	}
	curl_global_cleanup();

Err:
	if(cusid_obj)
		json_object_put(cusid_obj);
	if(userticket_obj)
		json_object_put(userticket_obj);
	if(devicemd5mac_obj)
		json_object_put(devicemd5mac_obj);
	if(obj)
		json_object_put(obj);
	return ret;
}

static int _check_response(const char *res_path)
{
	json_object *res_obj = NULL, *status_obj = NULL, *token_obj = NULL;
	int ret = ASUSDDNS_ERR_NO_DDNS_TOKEN;
	
	if(!res_path)
	{
		logmessage("ddns_token", "No response file path.");
		return ASUSDDNS_ERR_OPEN_FILE_FAIL;
	}

	res_obj = json_object_from_file(res_path);
	if(!res_obj)
	{
		logmessage("ddns_token", "Cannot open response file.");
		return ASUSDDNS_ERR_OPEN_FILE_FAIL;
	}
	if(json_object_object_get_ex(res_obj, "status", &status_obj))
	{
		if(!json_object_get_int(status_obj))
		{
			json_object_object_get_ex(res_obj, "ddnstoken", &token_obj);			
			nvram_set("asusddns_token", json_object_get_string(token_obj));
			json_object_object_get_ex(res_obj, "expiretime", &token_obj);
			const char *expire_time_str = json_object_get_string(token_obj);
			nvram_set("asusddns_token_expiretime", expire_time_str);
			logmessage("ddns_token", "New ddnstoken expiretime=%s(UTC)", expire_time_str);
			ret = ASUSDDNS_ERR_SUCCESS;
		}
		else
		{
			logmessage("ddns_token", "No status");
		}
	}

	if(status_obj)
		json_object_put(status_obj);
	if(token_obj)
		json_object_put(token_obj);
	if(res_obj) 
		json_object_put(res_obj);

	return ret;
}

int update_asus_ddns_token()
{
	int ret = -1;
	char* asusddns_token_expiretime = nvram_safe_get("asusddns_token_expiretime");
	logmessage("ddns_token", "Old ddnstoken expiretime=%s(UTC)", asusddns_token_expiretime);
	
	nvram_set_int("asusddns_token_state", ASUSDDNS_ERR_INIT_STATE);

	if ((checkTokenExpiration(asusddns_token_expiretime) != 0) || !json_object_from_file(ASUSDDNS_REQ_TOKEN_RES))
	{
		if(_update_userticket() == ASUSDDNS_ERR_NO_DM_REFRESH_TICKET)
		{
			logmessage("ddns_token", "User does not login the account yet.");
			nvram_set_int("asusddns_token_state", ASUSDDNS_ERR_NO_DM_REFRESH_TICKET);
			return ASUSDDNS_ERR_NO_DM_REFRESH_TICKET;
		}
		logmessage("ddns_token", "Acquire token!");
		ret = _acquire_token(ASUSDDNS_REQ_TOKEN_RES, 0);
	}
	else {
		logmessage("ddns_token", "ddnstoken hasn't expired yet");
		ret = ASUSDDNS_ERR_SUCCESS;
	}
	
	if(ret == ASUSDDNS_ERR_SUCCESS)
	{
		nvram_set_int("asusddns_token_state", _check_response(ASUSDDNS_REQ_TOKEN_RES));
		//logmessage("ddns_token", "Now ddnstoken expiretime=%s(UTC)", nvram_safe_get("asusddns_token_expiretime"));
	}
	else
	{
		nvram_set_int("asusddns_token_state", ret);
	}

	logmessage("ddns_token", "Done.(%d)", ret);
	return ret;
}

int update_asus_ddns_token_main(int argc, char *argv[])
{
	return update_asus_ddns_token();
}


