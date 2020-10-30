#ifndef _WS_CALLER_H
#define _WS_CALLER_H
#include <ws_api.h>					// include the header of web service 

typedef int (*cb_DeviceDesc)(char *buf, int *buf_len, int flag);

int st_KeepAlive(GetServiceArea* gsa, Login* lg);
int st_UpdateProfile(const char* update_status, int nat_type, char* mac_addr, GetServiceArea* gsa, Login* lg);
int st_UpdateProfile2(const char* update_status, int nat_type, char* mac_addr, GetServiceArea* gsa, Login* lg, char *dev_desc);
int st_ListProfile( GetServiceArea* gsa, Login* lg, ListProfile* lp, pProfile *pP );
int st_Logout(GetServiceArea* gsa, Login* lg);
int st_Login(GetServiceArea* gsa, Login* lg, char* vip_id, char* vip_pwd, char* dev_desc);
int st_KeepAlive_loop(int sec, int* is_terminate);
int st_KeepAlive_threading(int sec, int* is_terminate);
int st_KeepAlive_thread_exit();
int st_Unregister(GetServiceArea* gsa, Login* lg);
int st_PnsSendMsg(GetServiceArea* gsa, Login* lg, char *token, char *serviceid, char *msg);
int st_IftttNotification(char *server, char *api, char *msg);
int st_GetUserTicketByRefresh(char *server, GetUserTicketByRefresh* ut, char* cusid, char* refresh_ticket);
#endif
