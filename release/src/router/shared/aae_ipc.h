#ifndef __AAE_IPC_SHARED_H__
#define __AAE_IPC_SHARED_H__


#define AWSIOT_IPC_SOCKET_PATH  "/var/run/awsiot_ipc_socket"
#define MASTIFF_IPC_SOCKET_PATH  "/var/run/mastiff_ipc_socket"
#define MASTIFF_IPC_MAX_CONNECTION       10


#define AAE_MAX_IPC_PACKET_SIZE   1024

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
#define AWSIOT_PREFIX "awsiot"
enum awsiotEventType {
	EID_AWSIOT_NONE = 0,
	EID_AWSIOT_TUNNEL_ENABLE = 1,
	EID_AWSIOT_MAX
};
#define AWSIOT_GENERIC_MSG	 "{\""AWSIOT_PREFIX"\":{\""AAE_IPC_EVENT_ID"\":%d}}"
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
enum aaeHttpdEventType {
	AAE_EID_HTTPD_NONE = 0,
	AAE_EID_HTTPD_PAYLOAD2 = 1,
	AAE_EID_HTTPD_MAX
};
#define AAE_HTTPD_PAYLOAD2_MSG	 "{\""AAE_HTTPD_PREFIX"\":{\""AAE_IPC_EVENT_ID"\":%d, \""AAE_HTTPD_PAYLOAD2_PREFIX"\":%s}}"
#define AAE_HTTPD_PAYLOAD2_RESP_MSG	 "{\""AAE_HTTPD_PREFIX"\":{\""AAE_IPC_EVENT_ID"\":%d, \""AAE_IPC_STATUS"\":\"%s\"}}"
//#endif

int aae_sendIpcMsgAndWaitResp(char *ipcPath, char *data, int dataLen, char *out, int outLen, int timeout_sec);

#endif  // #ifndef __AAE_IPC_SHARED_H__