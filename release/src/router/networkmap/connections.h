#ifndef __CONNECTIONS_H__
#define __CONNECTIONS_H__

#include "networkmap.h"

void find_wireless_device(P_CLIENT_DETAIL_INFO_TABLE p_client_detail_info_tab, int offline);

void rc_diag_stainfo(P_CLIENT_DETAIL_INFO_TABLE p_client_detail_info_tab, int i, char *mlo_mac);

#endif
