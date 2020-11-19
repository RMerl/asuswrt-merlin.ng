/*
** amas_wgn_shared.h
**
**
**
*/
#ifndef __WGN_SHAREDH__
#define __WGN_SHAREDH__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bcmnvram.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "shutils.h"

#define SYNC_NODE_ROUTER_ONLY	0
#define SYNC_NODE_RE_ALL		1

#define WGN_MAX_NO_GUEST_NETWORK			8
#define WGN_MAXINUM_SUBNET_RULELIST			256
#define WGN_MAXINUM_VLAN_RULELIST			256
#define WGN_VLAN_RULE_MAX_BUFFER_SIZE		1024 * WGN_MAXINUM_VLAN_RULELIST
#define WGN_SUBNET_RULE_MAX_BUFFER_SIZE		1024 * WGN_MAXINUM_SUBNET_RULELIST

#define WGN_GET_CFG_TYPE_ALL				0
#define WGN_GET_CFG_TYPE_WGN_ONLY			1
#define WGN_GET_CFG_TYPE_NOT_WGN			2
#define WGN_RULE_LIST_TYPE					1

typedef struct wgn_subnet_rule_t 
{
#define WGN_SUBNET_RULE_NVRAM 				"subnet_rulelist"	
#define WGN_SUBNET_RULE_MAX_FIELDS			12
#define WGN_SUBNET_RULE_IP_STRING_SIZE		16
#define WGN_SUBNET_RULE_DOMIN_MAIN_SIZE		132
#define WGN_SUBNET_RULE_MACIP_BINDING_SIZE	1024
#define WGN_SUBNET_RULE_DNS_SIZE			132
	
	char ipaddr[WGN_SUBNET_RULE_IP_STRING_SIZE+1];
	char ipmask[WGN_SUBNET_RULE_IP_STRING_SIZE+1];
	unsigned int dhcp_enable;
	char dhcp_start[WGN_SUBNET_RULE_IP_STRING_SIZE+1];
	char dhcp_end[WGN_SUBNET_RULE_IP_STRING_SIZE+1];
	unsigned int dhcp_lease;
	char domain_name[WGN_SUBNET_RULE_DOMIN_MAIN_SIZE+1];
	char dns[WGN_SUBNET_RULE_DNS_SIZE+1];
	char wins[WGN_SUBNET_RULE_DNS_SIZE+1];
	unsigned int ema;
	char macipbinding[WGN_SUBNET_RULE_MACIP_BINDING_SIZE+1];
	unsigned int type;	// 1: AiMesh guest network
} wgn_subnet_rule;

extern wgn_subnet_rule* wgn_subnet_list_get_from_content(char *content, wgn_subnet_rule *list, size_t max_list_size, size_t *ret_list_size, int cfg_type);
extern char* 			wgn_subnet_list_set_to_buffer(wgn_subnet_rule *list, size_t list_size, char *buffer, size_t max_buffer_size);
extern wgn_subnet_rule* wgn_subnet_list_get_from_nvram(wgn_subnet_rule *list, size_t max_list_size, size_t *ret_list_size);
extern wgn_subnet_rule* wgn_subnet_list_get_all_from_nvram(wgn_subnet_rule *list, size_t max_list_size, size_t *ret_list_size);
extern wgn_subnet_rule* wgn_subnet_list_get_other_from_nvram(wgn_subnet_rule *list, size_t max_list_size, size_t *ret_list_size);
extern void 			wgn_subnet_list_set_to_nvram(wgn_subnet_rule *list, size_t list_size);
extern wgn_subnet_rule* wgn_subnet_list_find(wgn_subnet_rule *list, size_t list_size, char *subnet_name);

typedef struct wgn_vlan_rule_t 
{
#define WGN_VLAN_RULE_NVRAM				"vlan_rulelist"	
#define WGN_VLAN_RULE_MAX_FIELDS		11	
#define WGN_VLAN_RULE_PORTSET_SIZE		8
#define WGN_VLAN_RULE_WLANSET_SIZE		12
#define WGN_VLAN_RULE_SUBNET_NAME_SIZE	32

	unsigned int enable;
	unsigned int vid;
	char prio;
	char wanportset[WGN_VLAN_RULE_PORTSET_SIZE+1];
	char lanportset[WGN_VLAN_RULE_PORTSET_SIZE+1];
	char wl2gset[WGN_VLAN_RULE_WLANSET_SIZE+1];
	char wl5gset[WGN_VLAN_RULE_WLANSET_SIZE+1];
	char subnet_name[WGN_VLAN_RULE_SUBNET_NAME_SIZE+1];
	unsigned int internet;
	unsigned int public_vlan;
	unsigned int type; 	// 1: AiMesh guest network
} wgn_vlan_rule;

extern wgn_vlan_rule* 	wgn_vlan_list_get_from_content(char *content, wgn_vlan_rule *list, size_t max_list_size, size_t *ret_list_size, int cfg_type);
extern wgn_vlan_rule* 	wgn_vlan_list_get_from_nvram(wgn_vlan_rule *list, size_t max_list_size, size_t *ret_list_size);
extern wgn_vlan_rule* 	wgn_vlan_list_get_all_from_nvram(wgn_vlan_rule *list, size_t max_list_size, size_t *ret_list_size);
extern wgn_vlan_rule* 	wgn_vlan_list_get_other_from_nvram(wgn_vlan_rule *list, size_t max_list_size, size_t *ret_list_size);
extern char*			wgn_vlan_list_set_to_buffer(wgn_vlan_rule *list, size_t list_size, char *buffer, size_t max_buffer_size);
extern void 			wgn_vlan_list_set_to_nvram(wgn_vlan_rule *list, size_t list_size);
extern wgn_vlan_rule* 	wgn_vlan_list_find(wgn_vlan_rule *list, size_t list_size, char *subnet_name);
extern wgn_vlan_rule* 	wgn_vlan_list_find_unit(wgn_vlan_rule *list, size_t list_size, int unit, int subunit);
extern int 				wgn_vlan_list_check_subunit_conflict(wgn_vlan_rule *list, size_t list_size, int unit, int subunit);

extern int 				wgn_get_wl_band_unit(int *result_unit, int *result_subunit, char *wl2gset, char *wl5gset);
extern void 			wgn_get_wl_unit(int *result_unit, int *result_subunit, wgn_vlan_rule *vlan_rule);
extern void 			wgn_set_wl_unit(int unit, int subunit, wgn_vlan_rule *vlan_rule);
extern int 				wgn_guest_ifcount(int band);
extern char*			wgn_guest_ifnames(int band, int total, char *ret_ifnames, size_t buffer_size);
extern char*			wgn_guest_all_ifnames(char *ret_ifnames, size_t ifnames_bsize);
extern char*			wgn_guest_vlans(char *ifnames, char *ret_vlans, size_t vlans_bsize);
extern void 			wgn_check_settings(void);
extern int 				wgn_guest_is_enabled(void);
#endif 	/* !__WGN_SHAREDH__ */
