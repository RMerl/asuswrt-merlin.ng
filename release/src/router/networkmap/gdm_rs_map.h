#ifndef GDM_RS_MAP_H
#define GDM_RS_MAP_H

#include <json.h>

#define HOSTNAME_LEN 32
#define IP_LEN 16
#define TYPE_LEN 32
#define OS_LEN 32
#define VENDOR_LEN 128

typedef struct {
	char hostname[HOSTNAME_LEN];
	char ip[IP_LEN];
	char type[TYPE_LEN];
	char os[OS_LEN];
	char vendor[VENDOR_LEN];
} gdm_client_info;

int gdm_rs_map_hs_ns(json_object* root, const char* client_mac, gdm_client_info* gdm_info);

#endif