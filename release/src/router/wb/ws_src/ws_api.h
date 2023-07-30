#ifndef __WS_API_H__
#define __WS_API_H__

#define TRANSFER_TYPE		"https://"

// #ifdef RTCONFIG_ACCOUNT_BINDING
//- for test
#define SERVER 				"aae-spweb.asuscomm.com"
// #else
// #define SERVER 				"aae-spweb-vx.asuscloud.com"
// #endif

//#define SERVER 				"54.179.149.151"
// #ifdef RTCONFIG_ACCOUNT_BINDING
//- for test
// #define LOGIN_SERVER 		"54.169.254.181"
// #else
#define LOGIN_SERVER 		"aae-sgweb001-1.asuscomm.com"
// #endif
#define GET_SERVICE_AREA	"/aae/getservicearea"
#define LOGIN				"/aae/login"
#define QUERY_FRIEND		"/aae/queryfriend"
#define LIST_PROFILE		"/aae/listprofile"
#define UPDATE_PROFILE		"/aae/updateprofile"
#define LOGOUT				"/aae/logout"
#define GET_WEB_PATH		"/aae/getwebpath"
#define CREATE_PIN			"/aae/createpin"
#define QUERY_PIN			"/aae/querypin"
#define UNREGISTER			"/aae/unregister"
#define UPDATE_ICE_INFO		"/aae/updateiceinfo"
#define KEEP_ALIVE			"/aae/keepalive"
#define PUSH_SENDMSG		"/aae/wlpush_sendmsg"
#define PNS_SENDMSG			"/aae/pns_sendmsg"
#define PNS_SENDMSG_FCM		"/aae/pns_sendmsg_fcm"
#define PSR_SENDMSG			"/aae/psr_sendmsg"
#define GET_USER_TICKET_BY_REFRESH	"/aae/getuserticketbyrefresh"
#define GET_AWS_CERTIFICATE	"/aae/getawscertificate"
#define REMOTELOGIN	"/aae/remotelogin"

#define MAX_STATUS_LEN 	32
#define MAX_URL_LEN		128
#define MAX_TIME_LEN	64
#define MAX_USER_LEVEL_LEN 	64
#define MAX_CUSID_LEN		65
#define MAX_TICKET_LEN		64
#define MAX_REFRESH_TICKET_LEN 512
#define MAX_NICKNAME_LEN	64
#define MAX_DEVICEID_LEN	64
#define MAX_DEVTICKET_LEN	64
#define MAX_RELAY_INFO_LEN	MAX_URL_LEN
#define MAX_RELAY_IP_LEN	MAX_URL_LEN
#define MAX_STUN_INFO_LEN	MAX_URL_LEN
#define MAX_TURN_INFO_LEN	MAX_URL_LEN
#define MAX_DEV_TICKET_EXP_LEN 	64
#define MAX_ID_LEN		64
#define MAX_STATUS_LEN		32
#define MAX_DEVICE_DESC_LEN		1024
#define MAX_DESC_LEN		512
#define MAX_PIN_LEN		64
#define MAX_IP_ADDR_LEN		128 	//32
#define MAX_CA_DATA_LEN		2048

#define DM_API_LEVEL 22

typedef struct _AAE_STATUS
{
	int status;
	char *status_text;
} aae_status_t;

typedef struct _GetServiceArea
{
	char 	status[MAX_STATUS_LEN];
	char 	servicearea[MAX_URL_LEN];
	char	time[MAX_TIME_LEN];
	char	srcip[MAX_IP_ADDR_LEN];
	char	retrytime[MAX_TIME_LEN];
}GetServiceArea, *pGetServiceArea;

typedef struct _SrvInfo
{
	struct _SrvInfo* next;
	char	srv_ip[MAX_IP_ADDR_LEN];
} SrvInfo, *pSrvInfo;

typedef struct _Login{
	char 	status[MAX_STATUS_LEN];
	char 	apilevel_status[MAX_STATUS_LEN];
	char 	apilevel[MAX_STATUS_LEN];
	char	usersvclevel[MAX_USER_LEVEL_LEN];
	char	cusid[MAX_CUSID_LEN];
	char	userticket[MAX_TICKET_LEN]; 
	char    userrefreshticket[MAX_REFRESH_TICKET_LEN]; 
	char	usernickname[MAX_NICKNAME_LEN];
	char	ssoflag[2];
	char	deviceid[MAX_DEVICEID_LEN];
	char	deviceticket[MAX_DEVTICKET_LEN];
//	char	relayinfo[MAX_RELAY_INFO_LEN];
//	char	relayip[MAX_RELAY_IP_LEN];
//	char	stuninfo[MAX_STUN_INFO_LEN];
//	char	turninfo[MAX_TURN_INFO_LEN];
	pSrvInfo	relayinfoList;
	pSrvInfo	stuninfoList;
	pSrvInfo	turninfoList;
	pSrvInfo	pnsinfoList;
	pSrvInfo	psrinfoList;
	pSrvInfo	webstorageinfoList;
	pSrvInfo	ddnsinfoList;
	char	deviceticketexpiretime[MAX_DEV_TICKET_EXP_LEN];
	char 	time[MAX_TIME_LEN];
} Login, *pLogin;

typedef struct _Friends{
	struct _Friends* 	next;
	char 		userid[MAX_ID_LEN];
	char 		cusid[MAX_ID_LEN];
	char		nickname[MAX_ID_LEN];
} Friends, *pFriends;

typedef struct _QueryFriend{
	char 		status[MAX_STATUS_LEN];
	char		time[MAX_TIME_LEN];
	pFriends	FriendList;
}QueryFriend, *p_QueryFriend;

typedef struct _Profile{
	struct _Profile*	next;
	char	deviceid[MAX_DEVICEID_LEN];
	char	devicestatus[MAX_STATUS_LEN];
	char	devicename[MAX_DEVICEID_LEN];
	char	deviceservice[MAX_DEVICEID_LEN];
	char	devicenat[MAX_DEVICEID_LEN];
	char	devicedesc[MAX_DEVICE_DESC_LEN];
} Profile, *pProfile;

typedef struct _ListProfile{
	char 		status[MAX_STATUS_LEN];
	char		time[MAX_TIME_LEN];
	pProfile	pProfileList;
}ListProfile, *pListProfile;

typedef struct _UpdateProfile
{
	char	status[MAX_STATUS_LEN];
	char	deviceticketexpiretime[MAX_TIME_LEN];
	char	time[MAX_TIME_LEN];
}UpdateProfile, *pUpdateProfile;

typedef struct _Logout{
	char	status[MAX_STATUS_LEN];
	char	time[MAX_TIME_LEN];
}Logout, *pLogout;

typedef struct _Getwebpath{
	char	status[MAX_STATUS_LEN];
	char	url[MAX_URL_LEN];
	char	time[MAX_TIME_LEN];
}Getwebpath, *pGetwebpath;

typedef struct _CreatePin{
	char	status[MAX_STATUS_LEN];
	char	pin[MAX_PIN_LEN];
	char	deviceticketexpiretime[MAX_TIME_LEN];
	char	time[MAX_TIME_LEN];	
}CreatePin, *pCreatePin;

typedef struct _QueryPin{
	char	status[MAX_STATUS_LEN];
	char	deviceid[MAX_DEVICEID_LEN];
	char	devicestatus[MAX_STATUS_LEN];
	char	devicename[MAX_ID_LEN];
	char	deviceservice[MAX_ID_LEN];
	char	deviceticketexpiretime[MAX_DEV_TICKET_EXP_LEN];
	char	time[MAX_TIME_LEN];
}QueryPin, *pQueryPin;

typedef struct _UnregisterDevice{
	char	status[MAX_STATUS_LEN];
	char	time[MAX_TIME_LEN];
}UnregisterDevice, *pUnregisterDevice;

typedef struct _Updateiceinfo{
	char	status[MAX_STATUS_LEN];
	char	deviceticketexpiretime[MAX_DEV_TICKET_EXP_LEN];
	char	time[MAX_TIME_LEN];
}Updateiceinfo, *pUpdateiceinfo;

typedef struct _Keepalive{
	char	status[MAX_STATUS_LEN];
	char	deviceticketexpiretime[MAX_DEV_TICKET_EXP_LEN];
	char	time[MAX_TIME_LEN];
}Keepalive, *pKeepalive;

typedef struct _Push_Msg
{
	char 	status[MAX_STATUS_LEN];
	char	time[MAX_TIME_LEN];
} Push_Msg;

typedef struct _PnsSendMsg
{
	char 	status[MAX_STATUS_LEN];
	char	time[MAX_TIME_LEN];
} PnsSendMsg, *pPnsSendMsg;

typedef struct _IftttNotification
{
	char 	status[MAX_STATUS_LEN];
	char	message[MAX_DEVICEID_LEN];
} IftttNotification, *pIftttNotification;

typedef struct _GetUserTicketByRefresh
{
	char 	status[MAX_STATUS_LEN];
	char	userticket[MAX_DEVTICKET_LEN];
	char    userrefreshticket[MAX_REFRESH_TICKET_LEN];
	char	time[MAX_TIME_LEN];
}GetUserTicketByRefresh, *pGetUserTicketByRefresh;

typedef struct _GetAWSCertificate
{
	char 	status[MAX_STATUS_LEN];
	char	awsiotendpoint[MAX_DEVTICKET_LEN];
	char	rootca[MAX_CA_DATA_LEN];
	char	certificate[MAX_CA_DATA_LEN];
	char	privatekey[MAX_CA_DATA_LEN];
	char    message[MAX_DESC_LEN];
}GetAWSCertificate, *pGetAWSCertificate;

typedef struct _PsrSendMsg
{
	char 	status[MAX_STATUS_LEN];
	char 	result_httpcode[MAX_STATUS_LEN];
	char 	result_content[MAX_DESC_LEN];
	char	time[MAX_TIME_LEN];
} PsrSendMsg, *pPsrSendMsg;

typedef struct _PnsSendMsgFcm
{
	char 	status[MAX_STATUS_LEN];
	char	time[MAX_TIME_LEN];
} PnsSendMsgFcm, *pPnsSendMsgFcm;

typedef struct _RemoteLogin
{
	char 	status[MAX_STATUS_LEN];
	char	time[MAX_TIME_LEN];
} RemoteLogin, *pRemoteLogin;

typedef enum _ws_status_code
{
	Success,
	Authentication_Fail,
	Invalid_Service_ID,
	No_Device_Exist,
	No_Right_to_Access,
	No_User_Exist,
	Service_error,
	Invalid_xml_document
}ws_status_code;

int send_loginbyticket_req(
	const char* server,
//	const char* userid, 
	const char* userticket,
//	const char* passwd,
	const char* devicemd5mac,
	const char*	devicename,
	const char*	deviceservice,
	const char* devicetype,
	const char*	permission,
	const char* devicedesc,
	Login*		pLogin
);

int send_getservicearea_req(
	const char* server, 
	const char* serviceid, 
	const char* userid, 
	const char* passwd,
	const char* devicetype, 
	const char* fwver, 
	const int apilevel,
	const char* modelname,
	GetServiceArea* pGSA//out put
	);

int send_login_req(
	const char* server,
	const char* userid, 
	const char* passwd,
	const char*	cusid,
	const char*	userticket,
	const char* devicemd5mac,
	const char*	devicename,
	const char*	deviceservice,
	const char* devicetype,
	const char*	permission,
	const char* devicedesc,
	const char* fwver, 
	const int apilevel,
	const char* modelname,
	Login*		pLogin
	);

int send_query_friend_req(
	const char* server,
	const char* user_ticket,
	QueryFriend*	fd_list
	);

int send_list_profile_req(
	const char* 	server,
	const char*	cusid,
	const char*	userticket,
	const char*	deviceticket,
	const char*	friendid,
	const char*	deviceid,
	pListProfile	p_ListProfile); //out

int send_update_profile_req(
	const char* server,
	const char* cusid,
	const char* deviceid,
	const char* deviceticket,
	const char* devicename,
	const char* deviceservice,
	const char* devicestatus,
	const char* permission,
	const char* devicenat,
	const char* devicedesc,
	pUpdateProfile pup);

int send_logout_req(
	const char* 	server, 
	const char* 	cusid,
	const char* 	deviceid,
	const char* 	deviceticket,
	Logout*		plgo
);

int send_create_pin_req(
	const char* 	server,
	const char*	cusid,
	const char*	deviceid,
	const char*	deviceticket,
	Getwebpath*	pgw
);

int send_query_pin_req(
	const char*	server,
	const char*	cusid,
	const char*	deviceticket,
	const char*	pin,
	QueryPin*	pqp
);

int send_unregister_device_req(
	const char* 	server,
	const char* 	cusid,
	const char*	deviceid,
	const char*	deviceticket,
	UnregisterDevice*	pud
);

int send_updateiceinfo_req(
	const char*	server,
	const char*	cusid,
	const char*	deviceid,
	const char*	deviceticket,
	const char*	iceinfo,
	Updateiceinfo*	pui
);

int send_keepalive_req(
	const char*	server,
	const char*	cusid,
	const char*	deviceid,
	const char*	deviceticket,
	Keepalive*	pka
);

int send_push_msg_req(
	const char *server,
	const char *mac,
	const char *token,
	const char *msg,
	Push_Msg *pPm
);

int send_pns_sendmsg_req(
	const char *server,
	const char *cusid,
	const char *deviceid,
	const char *deviceticket,
    const char *appids,
	const char *todeviceid,
	const char* devicetype, 
	const char* fwver, 
	const char* apilevel,
	const char* modelname,
	const char *msg,
	PnsSendMsg *pPsm
);

int send_ifttt_notification_req(
	const char *server,
	const char *trigger,
	const char *msg,
	IftttNotification *pIftttnotification
);

int send_getuserticketbyrefresh_req(
	const char* server, 
	const char* serviceid, 
	const char* cusid, 
	const char* devicemd5mac,
	const char* refresh_ticket,
	const char* devicetype, 
	const char* fwver, 
	const int apilevel,
	const char* modelname,
	GetUserTicketByRefresh* pGSA
);

int send_getawscertificate_req(
	const char* server, 
	const char* serviceid, 
	const char* cusid, 
	const char* userticket,
	const char* deviceid,
	const char* deviceticket,
	const char* devicetype, 
	const char* fwver, 
	const int apilevel,
	const char* modelname,
	GetAWSCertificate* pGAC
);

int send_psr_sendmsg_req(
	const char *server,
	const char *cusid,
	const char *deviceid,
	const char *deviceticket,
    const char *appids,
	const char* devicetype, 
	const char* fwver, 
	const char* apilevel,
	const char* modelname,
	const char *msg,
	PsrSendMsg *pPsm
);

int send_pns_sendmsg_fcm_req(
	const char *server,
	const char *cusid,
	const char *deviceid,
	const char *deviceticket,
    const char *appids,
	const char* devicetype, 
	const char* fwver, 
	const char* apilevel,
	const char* modelname,
	const char *msg,
	PnsSendMsgFcm *pPsm
);

int send_remotelogin_req(
	const char *server,
	const char* serviceid, 
	const char *oauth_dm_cusid,
	const char *mobile_deviceid,
	const char *dm_ticket,
	const char* devicetype, 
	const char* fwver, 
	const int apilevel,
	const char* modelname,
	RemoteLogin *pRl
);

char *get_curl_status_string(int error);
char *get_aae_status_string(int error);
#endif
