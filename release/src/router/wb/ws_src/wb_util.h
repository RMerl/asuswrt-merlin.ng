#ifndef __WB_UTIL_H__
#define __WB_UTIL_H__

int 	get_web_path_len(char* trans_type, char* server, char* append);
char* 	get_webpath(const char* trans_type, const char* server, const char* append);
void 	free_webpath(char* webpath);
void 	free_append_data(char* append_data);

char* make_str(const char *fmt,	...);

const char* getservicearea_template = 
"<getservicearea>\r\n"
"<userid>%s</userid>\r\n"
"<passwd>%s</passwd>\r\n"
"</getservicearea>\r\n\r\n"
;
const char* login_template = 
"<login>\r\n"
"<userid>%s</userid>\r\n"
"<passwd>%s</passwd>\r\n"
"<cusid>%s</cusid>\r\n"
"<userticket>%s</userticket>\r\n"
"<devicemd5mac>%s</devicemd5mac>\r\n"
"<devicename>%s</devicename>\r\n"
"<deviceservice>%s</deviceservice>\r\n"
"<devicetype>%s</devicetype>\r\n"
"<permission>%s</permission>\r\n"
"<devicedesc>%s</devicedesc>\r\n"
"</login>\r\n\r\n";

const char* loginbyticket_template = 
"<loginbyticket>\r\n"
"<userticket>%s</userticket>\r\n"
"<devicemd5mac>%s</devicemd5mac>\r\n"
"<devicename>%s</devicename>\r\n"
"<deviceservice>%s</deviceservice>\r\n"
"<devicetype>%s</devicetype>\r\n"
"<permission>%s</permission>\r\n"
"</loginbyticket>\r\n\r\n";


const char* queryfriend_template = 
"<queryfriend>\r\n"
"<userticket>%s</userticket>\r\n"
"</queryfriend>\r\n\r\n";

const char* listprofile_template =
"<listprofile>\r\n"
      "<cusid>%s</cusid>\r\n"
      "<userticket>%s</userticket>\r\n"
      "<deviceticket>%s</deviceticket>\r\n"
      "<friendid>%s</friendid>\r\n"
      "<deviceid>%s</deviceid>\r\n"
"</listprofile>\r\n\r\n";

const char* updateprofile_template =
"<updateprofile>\r\n"
"<cusid>%s</cusid>\r\n"
"<deviceid>%s</deviceid>\r\n"
"<deviceticket>%s</deviceticket>\r\n"
"<profile>\r\n"
"<devicename>%s</devicename>\r\n"
"<deviceservice>%s</deviceservice>\r\n"
"<devicestatus>%s</devicestatus>\r\n"
"<permission>%s</permission>\r\n"
"<devicenat>%s</devicenat>\r\n"
"<devicedesc>%s</devicedesc>\r\n"
"</profile>\r\n"
"</updateprofile>\r\n\r\n";

const char* logout_template = 
"<logout>\r\n"
"<cusid>%s</cusid>\r\n"
"<deviceid>%s</deviceid>\r\n"
"<deviceticket>%s</deviceticket>\r\n"
"</logout>\r\n\r\n";

const char* getwebpath_template = 
"<getwebpath>\r\n"
"<userticket>%s</userticket>\r\n"
"<url>%s</url>\r\n"
"</getwebpath>\r\n\r\n";

const char* createpin_template = 
"<createpin>\r\n"																																																																																	
"<cusid>%s</cusid>\r\n"
"<deviceid>%s</deviceid>\r\n"
"<deviceticket>%s</deviceticket>\r\n"
"</createpin>\r\n\r\n";

const char* querypin_template = 
"<querypin>\r\n"
"<cusid>%s</cusid>\r\n"
"<deviceticket>%s</deviceticket>\r\n"
"<pin>%s</pin>\r\n"
"</querypin>\r\n\r\n";
const char* unregister_template =
"<unregister>\r\n"
"<cusid>%s</cusid>\r\n"
"<updateprofile_template>%s</updateprofile_template>\r\n"
"<deviceticket>%s</deviceticket>\r\n"
"</unregister>\r\n\r\n";

const char* updateiceinfo_template = 
"<updateiceinfo>\r\n"
"<cusid>%s</cusid>\r\n"
"<deviceid>%s</deviceid>\r\n"
"<deviceticket>%s</deviceticket>\r\n"
"<iceinfo>%s</iceinfo>\r\n"
"</updateiceinfo>\r\n\r\n";

const char* keepalive_template = 
"<keepalive>\r\n"
"<cusid>%s</cusid>\r\n"
"<deviceid>%s</deviceid>\r\n"
"<deviceticket>%s</deviceticket>\r\n"
"</keepalive>\r\n";

const char* push_msg_template = 
"<wlpush_sendmsg>\r\n"
"<router_mac>%s</router_mac>\r\n"
"<app_token>%s</app_token>\r\n"
"<message>%s</message>\r\n"
"</wlpush_sendmsg>\r\n\r\n"
;

const char* pns_sendmsg_template = 
"<pns_sendmsg>\r\n"
"<cusid>%s</cusid>\r\n"
"<deviceid>%s</deviceid>\r\n"
"<deviceticket>%s</deviceticket>\r\n"
"<todeviceid>%s</todeviceid>\r\n"
"<message>%s</message>\r\n"
"</pns_sendmsg>\r\n\r\n"
;

const char* getuserticketbyrefresh_template = 
"<getuserticketbyrefresh>\r\n"
"<cusid>%s</cusid>\r\n"
"<devicemd5mac>%s</devicemd5mac>\r\n"
"<userrefreshticket>%s</userrefreshticket>\r\n"
"</getuserticketbyrefresh>\r\n\r\n"
;

const char* getawscertificate_template = 
"<getawscertificate>\r\n"
"<cusid>%s</cusid>\r\n"
"<userticket>%s</userticket>\r\n"
"<deviceid>%s</deviceid>\r\n"
"<deviceticket>%s</deviceticket>\r\n"
"</getawscertificate>\r\n\r\n"
;

const char* psr_sendmsg_template = 
"<psr_sendmsg>\r\n"
"<cusid>%s</cusid>\r\n"
"<deviceid>%s</deviceid>\r\n"
"<deviceticket>%s</deviceticket>\r\n"
"<psr_payload>%s</psr_payload>\r\n"
"</psr_sendmsg>\r\n\r\n"
;

const char* pns_sendmsg_fcm_template = 
"<pns_sendmsg_fcm>\r\n"
"<cusid>%s</cusid>\r\n"
"<deviceid>%s</deviceid>\r\n"
"<deviceticket>%s</deviceticket>\r\n"
"<message>%s</message>\r\n"
"</pns_sendmsg_fcm>\r\n\r\n"
;

const char* remotelogin_template = 
"<remotelogin>\r\n"
"<oauth_dm_cusid>%s</oauth_dm_cusid>\r\n"
"<mobile_deviceid>%s</mobile_deviceid>\r\n"
"<dm_ticket>%s</dm_ticket>\r\n"
"</remotelogin>\r\n\r\n"
;

const char* ifttt_notification_template = 
"%s"
;

const char* wb_custom_header_templ = 
"Set-Cookie:ONE_VER=1_0; path=/; sid=%s; devicetype=%s; fwver=%s; dmapilevel=%d; apilevel=%d; modelname=%s";

const char* wb_custom_header_templ2 = 
"Set-Cookie:ONE_VER=1_0; path=/; sid=%s; devicetype=%s; fwver=%s; dmapilevel=%d; apilevel=%s; modelname=%s";

const char* ifttt_notification_header_templ = 
"Content-Type: application/json";
#endif
