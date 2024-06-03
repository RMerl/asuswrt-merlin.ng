#ifndef __AAE_IPC_SHARED_H__
#define __AAE_IPC_SHARED_H__

#define AWSIOT_IPC_SOCKET_PATH  "/var/run/awsiot_ipc_socket"
#define MASTIFF_IPC_SOCKET_PATH  "/var/run/mastiff_ipc_socket"
#define MASTIFF_IPC_MAX_CONNECTION       10

#define AAE_MAX_IPC_PACKET_SIZE   2048

/* for ipc packet handler */
struct aaeIpcArgStruct {
  unsigned char data[AAE_MAX_IPC_PACKET_SIZE];
  size_t dataLen;
  char waitResp;
  int sock;
};

#define AAE_IPC_EVENT_ID "eid"
#define AAE_IPC_STATUS "status"

//#ifdef RTCONFIG_AWSIOT
#define AAE_AWSIOT_PREFIX "awsiot"
#define AAE_AWSIOT_CALLEE_ID_PREFIX "callee_id"
enum awsiotEventType {
	EID_AWSIOT_NONE = 0,
	EID_AWSIOT_TUNNEL_ENABLE = 1,
	EID_AWSIOT_TUNNEL_TEST = 2,
	EID_AWSIOT_GET_SHADOW = 3,
	EID_AWSIOT_UPDATE_DB_WIREGUARD = 4,
	EID_AWSIOT_UPDATE_DB_TUNNELTEST = 5,
	EID_AWSIOT_MAX
};
#define AAE_AWSIOT_GENERIC_MSG	 "{\""AAE_AWSIOT_PREFIX"\":{\""AAE_IPC_EVENT_ID"\":%d}}"
#define AAE_AWSIOT_GENERIC_RESP_MSG	 "{\""AAE_AWSIOT_PREFIX"\":{\""AAE_IPC_EVENT_ID"\":%d, \""AAE_IPC_STATUS"\":\"%s\"}}"
#define AAE_AWSIOT_TNL_TEST_MSG	 "{\""AAE_AWSIOT_PREFIX"\":{\""AAE_IPC_EVENT_ID"\":%d, \""AAE_AWSIOT_CALLEE_ID_PREFIX"\":\"%s\"}}"
#define AAE_AWSIOT_GET_SHADOW_MSG	 "{\""AAE_AWSIOT_PREFIX"\":{\""AAE_IPC_EVENT_ID"\":%d}}"
#define AAE_AWSIOT_GET_SHADOW_MSG_RESP_MSG	 "{\""AAE_AWSIOT_PREFIX"\":{\""AAE_IPC_EVENT_ID"\":%d, \""AAE_IPC_STATUS"\":\"%s\", \""SHADOW_REPORTED"\":%s}}"
#define AAE_AWSIOT_UPDATE_DB_WIREGUARD_MSG	 "{\""AAE_AWSIOT_PREFIX"\":{\""AAE_IPC_EVENT_ID"\":%d, \"group_name\":\"%s\", \"role\":\"%s\", \"content\":%s}}"
#define AAE_AWSIOT_UPDATE_DB_TUNNELTEST_MSG	 "{\""AAE_AWSIOT_PREFIX"\":{\""AAE_IPC_EVENT_ID"\":%d, \"target_deviceid\":\"%s\", \"type\":\"%s\", \"error\":%d}}"
//#endif

//#ifdef RTCONFIG_ACCOUNT_BINDING
#define AAE_DDNS_PREFIX "ddns"
enum aaeDdnsEventType {
	AAE_EID_DDNS_NONE = 0,
	AAE_EID_DDNS_REFRESH_TOKEN = 1,
	AAE_EID_DDNS_MAX
};
#define AAE_DDNS_GENERIC_MSG	 "{\""AAE_DDNS_PREFIX"\":{\""AAE_IPC_EVENT_ID"\":%d}}"
#define AAE_DDNS_GENERIC_RESP_MSG	 "{\""AAE_DDNS_PREFIX"\":{\""AAE_IPC_EVENT_ID"\":%d, \""AAE_IPC_STATUS"\":\"%s\"}}"

#define AAE_NTC_PREFIX "nt_center"
enum aaeNtcEventType {
	AAE_EID_NTC_NONE = 0,
	AAE_EID_NTC_REFRESH_DEVICE_TICKET = 1,
	AAE_EID_NTC_MAX
};
#define AAE_NTC_GENERIC_MSG	 "{\""AAE_NTC_PREFIX"\":{\""AAE_IPC_EVENT_ID"\":%d}}"
#define AAE_NTC_GENERIC_RESP_MSG	 "{\""AAE_NTC_PREFIX"\":{\""AAE_IPC_EVENT_ID"\":%d, \""AAE_IPC_STATUS"\":\"%s\"}}"

#define AAE_HTTPD_PREFIX "httpd"
#define AAE_HTTPD_PAYLOAD2_PREFIX "payload2"

#define AAE_HTTPD_ACCOUNT_REG_PREFIX "account_reg"
#define OAUTH_TYPE "oauth_type"
#define OAUTH_USERNAME "username"
#define OAUTH_PASSWORD "password"
#define OAUTH_CUSID "cusid"
#define OAUTH_REFRESH_TICKET "refresh_ticket"
#define OAUTH_USER_TICKET "user_ticket"

#define AAE_HTTPD_GET_PROVISION_PINCODE_PREFIX "get_provision_pincode"
#define PIN_CODE "pincode"

#define AAE_HTTPD_FBWIFI2_REG_PREFIX "fbwifi2_reg"
#define AAE_HTTPD_REFRESH_DEVICE_TICKET_PREFIX "refresh_device_ticket"
enum aaeHttpdEventType {
	AAE_EID_HTTPD_NONE = 0,
	AAE_EID_HTTPD_PAYLOAD2 = 1,
	AAE_EID_HTTPD_FBWIFI2_REG = 2,
	AAE_EID_HTTPD_ACCOUNT_REG = 3,
	AAE_EID_HTTPD_REFRESH_DEVICE_TICKET = 4,
	AAE_EID_HTTPD_GET_PROVISION_PINCODE = 5,
	AAE_EID_HTTPD_MAX
};

#define SHADOW_REPORTED "reported"

#define AAE_HTTPD_PAYLOAD2_MSG	 "{\""AAE_HTTPD_PREFIX"\":{\""AAE_IPC_EVENT_ID"\":%d, \""AAE_HTTPD_PAYLOAD2_PREFIX"\":%s}}"
#define AAE_HTTPD_PAYLOAD2_RESP_MSG	 "{\""AAE_HTTPD_PREFIX"\":{\""AAE_IPC_EVENT_ID"\":%d, \""AAE_IPC_STATUS"\":\"%s\"}}"
#define AAE_HTTPD_ACCOUNT_REG_MSG	 "{\""AAE_HTTPD_PREFIX"\":{\""AAE_IPC_EVENT_ID"\":%d, \""AAE_HTTPD_ACCOUNT_REG_PREFIX"\":%s}}"
#define AAE_HTTPD_ACCOUNT_REG_RESP_MSG	 "{\""AAE_HTTPD_PREFIX"\":{\""AAE_IPC_EVENT_ID"\":%d, \""AAE_IPC_STATUS"\":\"%s\", \""AAE_HTTPD_ACCOUNT_REG_PREFIX"\":%s}}"
#define AAE_HTTPD_ACCOUNT_REG_RESP_RAW	 "{\""OAUTH_CUSID"\":\"%s\", \""OAUTH_REFRESH_TICKET"\":\"%s\", \""OAUTH_USER_TICKET"\":\"%s\"}"
#define AAE_HTTPD_FBWIFI2_REG_MSG	 "{\""AAE_HTTPD_PREFIX"\":{\""AAE_IPC_EVENT_ID"\":%d, \""AAE_HTTPD_FBWIFI2_REG_PREFIX"\":%s}}"
#define AAE_HTTPD_FBWIFI2_REG_RESP_MSG	 "{\""AAE_HTTPD_PREFIX"\":{\""AAE_IPC_EVENT_ID"\":%d, \""AAE_IPC_STATUS"\":\"%s\", \""AAE_HTTPD_FBWIFI2_REG_PREFIX"\":%s}}"
#define AAE_HTTPD_REFRESH_DEVICE_TICKET_MSG	 "{\""AAE_HTTPD_PREFIX"\":{\""AAE_IPC_EVENT_ID"\":%d}}"
#define AAE_HTTPD_REFRESH_DEVICE_TICKET_RESP_MSG	 "{\""AAE_HTTPD_PREFIX"\":{\""AAE_IPC_EVENT_ID"\":%d, \""AAE_IPC_STATUS"\":\"%s\"}}"
#define AAE_HTTPD_GET_PROVISION_PINCODE_MSG	 "{\""AAE_HTTPD_PREFIX"\":{\""AAE_IPC_EVENT_ID"\":%d}}"
#define AAE_HTTPD_GET_PROVISION_PINCODE_RESP_MSG	 "{\""AAE_HTTPD_PREFIX"\":{\""AAE_IPC_EVENT_ID"\":%d, \""AAE_IPC_STATUS"\":\"%s\", \""PIN_CODE"\":\"%s\"}}"
//#endif

int aae_sendIpcMsg(char *ipcPath, char *data, int dataLen);
int aae_sendIpcMsgAndWaitResp(char *ipcPath, char *data, int dataLen, char *out, int outLen, int timeout_sec);

#endif  // #ifndef __AAE_IPC_SHARED_H__
