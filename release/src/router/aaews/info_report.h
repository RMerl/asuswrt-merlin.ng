#ifndef __INFO_REPORT_H
#define __INFO_REPORT_H

#include <rtconfig.h>

#ifdef RTCONFIG_NOTIFICATION_CENTER
void write_login_info(char *login_status, char *server, char *psr_server, char *cusid, char *deviceid, char *deviceticket);
void write_mac_info(char *mac);
#endif

#endif
