#ifndef _NW_UTIL_H
#define _NW_UTIL_H

#ifndef RTCONFIG_GETREALIP
#define DEFAULT_STUN_PORT 3478
#define GOOGLE_STUN_PORT 19302

struct stun_server
{
	char *url;
	int port;
};
#endif //#ifndef RTCONFIG_GETREALIP

#define MASTIFF_DEF_PORT 61689
int get_mac(unsigned char* mac_address);
int check_wan_ip_change(void);
int is_private_ip(int caller);
#endif
