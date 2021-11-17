#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
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

#define ASUSDDNS_DBG(fmt,args...) \
        if(1) { \
                char info[1024]; \
                snprintf(info, sizeof(info), "echo \"[ASUSDDNS][%s:(%d)]"fmt"\" >> /tmp/asusddns.log", __FUNCTION__, __LINE__, ##args); \
                system(info); \
        }

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

static int _update_userticket()
{
	char event[AAE_MAX_IPC_PACKET_SIZE];
	char out[AAE_MAX_IPC_PACKET_SIZE];
	
	if(strlen(nvram_safe_get("oauth_dm_refresh_ticket")) == 0)	//APP not registered.
	{
		return ASUSDDNS_ERR_NO_DM_REFRESH_TICKET;
	}

	ASUSDDNS_DBG("Update userticket!\n");
	snprintf(event, sizeof(event), DDNS_GENERIC_MSG, EID_DDNS_REFRESH_TOKEN);
	
	aae_sendIpcMsgAndWaitResp(MASTIFF_IPC_SOCKET_PATH, event, strlen(event), out, sizeof(out), 5);
	
	json_object *root = NULL;
	json_object *ddnsObj = NULL;
	json_object *eidObj = NULL;
	json_object *stsObj = NULL;
	root = json_tokener_parse((char *)out);
	json_object_object_get_ex(root, DDNS_PREFIX, &ddnsObj);
	json_object_object_get_ex(ddnsObj, AAE_IPC_EVENT_ID, &eidObj);
	json_object_object_get_ex(ddnsObj, AAE_IPC_STATUS, &stsObj);
	if (!ddnsObj || !eidObj || !stsObj)
	{
		ASUSDDNS_DBG("Failed to aae_refresh_ticket\n");
	}
	else
	{
		int eid = json_object_get_int(eidObj);
		char *status = json_object_get_string(stsObj);
		if ((eid == EID_DDNS_REFRESH_TOKEN) && (!strcmp(status, "0")))
		{
			ASUSDDNS_DBG("Success to aae_refresh_ticket\n");
		}
		else
		{
			ASUSDDNS_DBG("Failed to aae_refresh_ticket\n");
		}
	}
	json_object_put(root);

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

static int _acquire_token(const char *res_path, const int check_CA)
{
	CURL *curl;
	CURLcode res;
	char *cusid = NULL, *userticket = NULL, *auth_status = NULL;
	char ddns_url[256];
	json_object *obj = NULL, *cusid_obj = NULL, *userticket_obj = NULL, *devicemd5mac_obj = NULL, *sid_obj = NULL;
	unsigned char digest[MD5_DIGEST_BYTES]={0};
	char md_label_mac[MD5_DIGEST_BYTES * 2 + 1]={0};
	char *label_mac_str=NULL;
	char *key = NULL;
	const char *auth_string = NULL;
	int ret, i;
	FILE *fp_res = NULL;

	if(!res_path)
	{
		ASUSDDNS_DBG("No response file path.\n");
		return ASUSDDNS_ERR_OPEN_FILE_FAIL;
	}

	fp_res = fopen(res_path, "wb");	
		
	if(!fp_res)
	{
		ASUSDDNS_DBG("Cannot open file for response data.\n");
		return ASUSDDNS_ERR_OPEN_FILE_FAIL;
	}
	
	cusid = nvram_safe_get("oauth_dm_cusid");
	userticket = nvram_safe_get("oauth_dm_user_ticket");
	auth_status = nvram_safe_get("oauth_auth_status");

	if(cusid[0] == '\0' || userticket[0] == '\0')
	{
		ASUSDDNS_DBG("No userticket.\n");
		fclose(fp_res);
		return ASUSDDNS_ERR_NO_USER_TICKET;
	}
	if (auth_status[0] == '\0' || (strncmp(auth_status, "0", 1) != 0 && strncmp(auth_status, "2", 1) != 0))
	{
		ASUSDDNS_DBG("No sid (service id).\n");
		fclose(fp_res);
		return ASUSDDNS_ERR_NO_SID;
	}


	obj = json_object_new_object();
	if(!obj)
	{
		ASUSDDNS_DBG("Cannot create json object!\n");
		ret = ASUSDDNS_ERR_JSON_ERR;
		goto Err;
	}

	cusid_obj = json_object_new_string(cusid);
	if(cusid_obj)
		json_object_object_add(obj, "cusid", cusid_obj);
	else
	{
		ASUSDDNS_DBG("Cannot add json object, cusid!\n");
		ret = ASUSDDNS_ERR_JSON_ERR;
		goto Err;
	}

	userticket_obj = json_object_new_string(userticket);
	if(userticket_obj)
		json_object_object_add(obj, "userticket", userticket_obj);
	else
	{
		ASUSDDNS_DBG("Cannot add json object, userticket!\n");
		ret = ASUSDDNS_ERR_JSON_ERR;
		goto Err;
	}

	label_mac_str = nvram_safe_get("label_mac");
	hmac_md5(label_mac_str, strlen(label_mac_str), digest);

	for (i = 0; i < MD5_DIGEST_BYTES; i++)
	{
		sprintf(&md_label_mac[i*2], "%02x", (unsigned int)digest[i]);
	}
	
	devicemd5mac_obj = json_object_new_string(md_label_mac);
	if(devicemd5mac_obj)
		json_object_object_add(obj, "devicemd5mac", devicemd5mac_obj);
	else
	{
		ASUSDDNS_DBG("Cannot add json object, devicemd5mac!\n");
		ret = ASUSDDNS_ERR_JSON_ERR;
		goto Err;
	}

	if (strncmp(auth_status, "0", 1) == 0) {
		sid_obj = json_object_new_string("1001");
	} else if (strncmp(auth_status, "2", 1) == 0) {
		sid_obj = json_object_new_string("1004");
	}
	if(sid_obj)
		json_object_object_add(obj, "sid", sid_obj);
	else
	{
		ASUSDDNS_DBG("Cannot add json object, sid!\n");
		ret = ASUSDDNS_ERR_JSON_ERR;
		goto Err;
	}

	auth_string = json_object_to_json_string(obj);
	//ASUSDDNS_DBG("auth_string=%s\n", auth_string);
	curl = curl_easy_init();

	if(curl)
	{
#ifdef RTCONFIG_ASUSDDNS_ACCOUNT_BASE
		if (nvram_get_int("oauth_auth_status") == 2) {
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
	
		curl_easy_cleanup(curl);
		if(res == CURLE_OK)
		{
			ret = ASUSDDNS_ERR_SUCCESS;
		}
		else
		{
			ret = ASUSDDNS_ERR_CURL_ERR;
			ASUSDDNS_DBG("CURL perform fail\n");
		}
	}
	else
	{
		ASUSDDNS_DBG("Cannot init CURL\n");
		ret = ASUSDDNS_ERR_CURL_ERR;
		goto Err;
	}

Err:
	if(cusid_obj)
		json_object_put(cusid_obj);
	if(userticket_obj)
		json_object_put(userticket_obj);
	if(devicemd5mac_obj)
		json_object_put(devicemd5mac_obj);
	if(obj)
		json_object_put(obj);
	fclose(fp_res);
	return ret;
}

static int _check_response(const char *res_path)
{
	json_object *res_obj = NULL, *status_obj = NULL, *token_obj = NULL;
	int ret = ASUSDDNS_ERR_NO_DDNS_TOKEN;
	
	if(!res_path)
	{
		ASUSDDNS_DBG("No response file path.\n");
		return ASUSDDNS_ERR_OPEN_FILE_FAIL;
	}

	res_obj = json_object_from_file(res_path);
	if(!res_obj)
	{
		ASUSDDNS_DBG("Cannot open response file.\n");
		return ASUSDDNS_ERR_OPEN_FILE_FAIL;
	}
	if(json_object_object_get_ex(res_obj, "status", &status_obj))
	{
		if(!json_object_get_int(status_obj))
		{
			json_object_object_get_ex(res_obj, "ddnstoken", &token_obj);			
			nvram_set("asusddns_token", json_object_get_string(token_obj));
			ret = ASUSDDNS_ERR_SUCCESS;
		}
		else
		{
			ASUSDDNS_DBG("No status\n");
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
	int ret;
	
	nvram_set_int("asusddns_token_state", ASUSDDNS_ERR_INIT_STATE);
	
	if(_update_userticket() == ASUSDDNS_ERR_NO_DM_REFRESH_TICKET)
	{
		ASUSDDNS_DBG("User does not login the account yet.\n");
		nvram_set_int("asusddns_token_state", ASUSDDNS_ERR_NO_DM_REFRESH_TICKET);
		return ASUSDDNS_ERR_NO_DM_REFRESH_TICKET;
	}

	ret = _acquire_token(ASUSDDNS_REQ_TOKEN_RES, 0);
	if(ret == ASUSDDNS_ERR_SUCCESS)
	{
		nvram_set_int("asusddns_token_state", _check_response(ASUSDDNS_REQ_TOKEN_RES));
	}
	else
	{
		nvram_set_int("asusddns_token_state", ret);
	}
	ASUSDDNS_DBG("Done. ret=%d\n", ret);
	return ret;
}

int update_asus_ddns_token_main(int argc, char *argv[])
{
	return update_asus_ddns_token();
}


