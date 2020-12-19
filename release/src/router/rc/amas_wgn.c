/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 * Copyright 2004, ASUSTeK Inc.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */
#include "rc.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <syslog.h>														
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <wlutils.h>
#include <wlif_utils.h>
#include <rtstate.h>
#include <stdlib.h>
#include <linux/sockios.h>
#include "amas_wgn_shared.h"

#define DBG_ODS(...) (_dprintf("%s(LINE:%d):%s\n", __FILE__, __LINE__, __VA_ARGS__));
#define DBG_PRINTF(...) do {\
	char MSG_BUF[2049];\
	memset(MSG_BUF, 0, sizeof(MSG_BUF));\
	DBG_ODS(args2str(MSG_BUF,sizeof(MSG_BUF)-1,__VA_ARGS__));\
}while(0)

#define DBG_TRACE_LINE do {\
	char MSG_BUF[33];\
	memset(MSG_BUF, 0, sizeof(MSG_BUF));\
	DBG_ODS(args2str(MSG_BUF,sizeof(MSG_BUF)-1,"TRACE LINE !!"));\
}while(0)

#define IS_ZERO_MAC(MACADDR)	( (memcmp(MACADDR, "\x00\x00\x00\x00\x00\x00", 6) == 0) )
#define IS_CAP() 			( (sw_mode() == SW_MODE_ROUTER || access_point_mode()) )
#define IS_RE()  			( (nvram_get_int("re_mode") == 1) )

#define WGN_WLIFU_MAX_NO_BRIDGE	WLIFU_MAX_NO_BRIDGE
#define WGN_ETH_IFNAME			WAN_IF_ETH
#define WGN_WAN_IFNAME			"wan0_ifname"
#define WGN_IFNAMES 			"wgn_ifnames"
#define WGN_ENABLED             "wgn_enabled"
#define WGN_VLAN_IFNAMES 		"wgn_vlan_ifnames"

#if defined(HND_ROUTER) && !(defined(RTCONFIG_HND_ROUTER_AX_675X) && !defined(RTCONFIG_HND_ROUTER_AX_6710))
#define WGN_HAVE_VLAN0
#endif

#if 1
#define exec_cmd(cmd, args ...) do {\
	if (eval(cmd, args) == -1)\
		DBG_PRINTF("*** WARNING, eval() return -1 ***");\
}while(0)
#else
#define CMDBUFF_SIZE	1024
#define exec_cmd(cmd, args ...) do {\
	int cmd_ret = 0;\
	int cmd_argc = 0;\
	int cmd_buff_offset = 0;\
	char * const cmd_argv[] = { cmd, ## args, NULL };\
	char cmd_buff[CMDBUFF_SIZE];\
	memset(cmd_buff, 0, sizeof(cmd_buff));\
	for (cmd_argc=0, cmd_buff_offset=0; cmd_argv[cmd_argc] != NULL && cmd_buff_offset < sizeof(cmd_buff); cmd_argc++)\
	{\
		strlcat(cmd_buff, cmd_argv[cmd_argc], sizeof(cmd_buff));\
		cmd_buff_offset+=strlen(cmd_argv[cmd_argc]);\
		strlcat(cmd_buff, " ", sizeof(cmd_buff));\
		cmd_buff_offset++;\
	}\
	if ((cmd_ret = doSystem("%s", cmd_buff)) != 0)\
	{\
		DBG_PRINTF("doSystem() fail ..., ret : %d", cmd_ret);\
	}\
}while(0)
#endif	/* USE_EVAL */

static char *args2str(char *buf, int buf_size, char *s, ...)
{
 	va_list args;
    va_start(args, s);
    vsnprintf(buf, buf_size, s, args);
    va_end(args);
    return buf;
}

int subnetcmp(
	unsigned long ip1, 
	unsigned long ipmask, 
	unsigned long ip2)
{
	return ( (ip1 & ipmask) == (ip2 & ipmask) ) ? 1 : 0;
}


int ipmask_to_int(
	unsigned long ipmask)
{
	int i;
	int ret = 0;

	for (i=0; i<32; i++)
	{
		if ((ipmask & (0x80000000 >> i)) == 0x00)
			break;
	}

	if (i < 31)
		ret = i;

	return ret;
}

int iface_is_up(
	char *ifname)
{
	int sock;
	struct ifreq ifr;

	if (!ifname)
		return 0;

	if ((sock = socket(PF_INET6, SOCK_DGRAM, IPPROTO_IP)) <= 0)
		return 0;

	memset(&ifr, 0, sizeof(ifr));
	strlcpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
	if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0)
		return 0;
	close(sock);
	
	return ((ifr.ifr_flags & IFF_UP) == IFF_UP) ? 1 : 0;
}

int find_lan_ifnames_by_ifname(
    char *ifname)
{
    int ret = 0;    
    char word[64];
    char *next = NULL;
                    
    if (!ifname)
        return 0;

    foreach(word, nvram_safe_get("lan_ifnames"), next)
        if ((ret = !strncmp(word, ifname, strlen(ifname))))
            break;
    
    return ret;
}

int find_br0_ifnames_by_ifname(
	char *ifname)
{
	int ret = 0;
	char word[64];
	char *next = NULL;

	if (!ifname)
		return 0;

	foreach(word, nvram_safe_get("br0_ifnames"), next)
		if ((ret = !strncmp(word, ifname, strlen(ifname))))
			break;

	return ret;
}

int iface_is_exists(
	char *ifname)
{
	int sock;
	struct ifreq ifr;
	int ret = 0;

	if (!ifname)
		return 0;

	if ((sock = socket(PF_INET6, SOCK_DGRAM, IPPROTO_IP)) <= 0)
		return 0;

	memset(&ifr, 0, sizeof(ifr));
	strlcpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
	ret = (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0) ? 0 : 1;
	close(sock);
	
	return ret;
}


int iface_get_mac(
	char *ifname, 
	unsigned char *ret_mac)
{
	int sock;
	int ret = 0;
	struct ifreq ifr;

	if (!ifname)
		return 0;

	if (!ret_mac)
		return 0;

	if ((sock = socket(PF_INET6, SOCK_DGRAM, IPPROTO_IP)) <= 0)
		return 0;

	memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_addr.sa_family = AF_INET;
	strlcpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
	ret = (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0) ? 0 : 1;
	if (ret == 1) memcpy((unsigned char *)&ret_mac[0], (unsigned char *)&ifr.ifr_hwaddr.sa_data[0], 6);
	close(sock);
	return ret;
}

// ioctl for bridge
enum { ARG_ADDBR = 0, ARG_DELBR, ARG_ADDIF, ARG_DELIF};
int ioctl_bridge(
    int action, 
    char *br, 
    char *brif)
{
    int ret = -1;
    int fd = 0;
    unsigned request;
    struct ifreq ifr;

    if (br == NULL) {
        _dprintf("[%s:%s] Unknow lan_ifname, can't add interface to bridge.\n", __FILE__, __FUNCTION__);
        return -1;
    }    

    memset(&ifr, 0x00, sizeof(struct ifreq));
    strlcpy(ifr.ifr_name, br, IFNAMSIZ);
    ifr.ifr_ifindex = if_nametoindex(brif);
             
    if (!ifr.ifr_ifindex) {
        _dprintf("Can't get index for %s\n", brif);
        return -1;
    }

    if (action == ARG_ADDIF)
        request = SIOCBRADDIF;
    else if (action == ARG_DELIF)
        request = SIOCBRDELIF;
    else {
        _dprintf("Only support addif/delif from bridge interface.\n");
        return -1;
    }

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1;

    if ((ret = ioctl(fd, request, &ifr)) < 0)
        return errno;

    close(fd);
    return 0;    
}

struct brif_rule_t 
{
#define WGN_BRIF_RULE_NVRAM			"wgn_brif_rulelist"	
#define WGN_BRIF_RULE_MAX_FIELDS	2

	char br_name[IFNAMSIZ+1];
	char subnet_name[WGN_VLAN_RULE_SUBNET_NAME_SIZE+1];
};

struct brif_rule_t* brif_list_get_from_content(
	char *content,
	struct brif_rule_t *list,
	size_t max_list_size,
	size_t *ret_list_size)
{
	char *nv, *nvp, *b;
	char *br_name;
	char *subnet_name;	

	int fields = 0;
	size_t list_size = 0;

	if (ret_list_size)
		*ret_list_size = 0;

	if (!content || !list || max_list_size <= 0)
		return NULL;

	nv = nvp = strdup(content);
	if (!nv)
		return NULL;

	while ((b = strsep(&nvp, "<")) != NULL)
	{
		fields = vstrsep(b, ">", &br_name, &subnet_name);
		if (fields != WGN_BRIF_RULE_MAX_FIELDS)
			continue;

		if (list_size > max_list_size)
			break;

		// br_name
		if (br_name && strlen(br_name) > 0 && strlen(br_name) <= IFNAMSIZ) strlcpy(list[list_size].br_name, br_name, sizeof(list[list_size].br_name));
		// subnet_name
		if (subnet_name && strlen(subnet_name) > 0 && strlen(subnet_name) <= WGN_VLAN_RULE_SUBNET_NAME_SIZE) strlcpy(list[list_size].subnet_name, subnet_name, sizeof(list[list_size].subnet_name));
		list_size++;
	}

	if (ret_list_size)
		*ret_list_size = list_size;

	free(nv);
	return list;
}

struct brif_rule_t* brif_list_get_from_nvram(
	struct brif_rule_t *list,
	size_t max_list_size,
	size_t *ret_list_size) 
{
	char *s = NULL;
	char *b = NULL;
	struct brif_rule_t *p = NULL;

	if (ret_list_size)
		*ret_list_size = 0;

	if (!list || max_list_size <= 0)
		return NULL;

	if (!(s = nvram_get(WGN_BRIF_RULE_NVRAM)))
		return NULL;

	if (!(b = strdup(s)))
		return NULL;

	p = brif_list_get_from_content(b, list, max_list_size, ret_list_size);
	free(b);

	return p;
}

struct brif_rule_t* brif_list_find(
	struct brif_rule_t *list,
	size_t list_size,
	char *br_ifname)
{
	struct brif_rule_t *p = NULL;
	size_t index = 0;

	if (!list || list_size <= 0 || !br_ifname)
		return NULL;

	for (index=0; index<list_size; index++)
	{
		if (strncmp(list[index].br_name, br_ifname, strlen(br_ifname)) == 0)
		{
			p = &list[index];
			break;
		}
	}

	return p;
}

#define SET_SUBNET_RULE(VLAN_RULE, SUBNET_RULE, IP, IPMASK) do {\
	struct in_addr ADDR;\
	ADDR.s_addr = htonl(IP);\
	memset(SUBNET_RULE->ipaddr, 0, sizeof(SUBNET_RULE->ipaddr));\
	snprintf(SUBNET_RULE->ipaddr, WGN_SUBNET_RULE_IP_STRING_SIZE, "%s", inet_ntoa(ADDR));\
	\
	ADDR.s_addr = htonl(IPMASK);\
	memset(SUBNET_RULE->ipmask, 0, sizeof(SUBNET_RULE->ipmask));\
	snprintf(SUBNET_RULE->ipmask, WGN_SUBNET_RULE_IP_STRING_SIZE, "%s", inet_ntoa(ADDR));\
	\
	ADDR.s_addr = htonl(IP+1);\
	memset(SUBNET_RULE->dhcp_start, 0, sizeof(SUBNET_RULE->dhcp_start));\
	snprintf(SUBNET_RULE->dhcp_start, WGN_SUBNET_RULE_IP_STRING_SIZE, "%s", inet_ntoa(ADDR));\
	\
	ADDR.s_addr = htonl((IP | ~IPMASK) & 0xfffffffe);\
	memset(SUBNET_RULE->dhcp_end, 0, sizeof(SUBNET_RULE->dhcp_end));\
	snprintf(SUBNET_RULE->dhcp_end, WGN_SUBNET_RULE_IP_STRING_SIZE, "%s", inet_ntoa(ADDR));\
	\
	memset(VLAN_RULE->subnet_name, 0, sizeof(VLAN_RULE->subnet_name));\
	snprintf(VLAN_RULE->subnet_name, WGN_VLAN_RULE_SUBNET_NAME_SIZE, "%s/%d", SUBNET_RULE->ipaddr, ipmask_to_int((IPMASK)));\
}while(0)

void check_vlan_rulelist_subnet_conflict(
	void)
{
	size_t i, j;
	int cfg_changed = 0;
	int conflict = 0;

	struct wgn_vlan_rule_t *p_vlan_list = NULL;
	struct wgn_vlan_rule_t tagged_base_vlan_list[WGN_MAXINUM_VLAN_RULELIST];
	size_t tagged_base_vlan_list_size = 0;
	struct wgn_vlan_rule_t vlan_list[WGN_MAXINUM_VLAN_RULELIST];
	size_t vlan_list_size = 0;
	struct wgn_subnet_rule_t *p1 = NULL, *p2 = NULL;
	struct wgn_subnet_rule_t subnet_list[WGN_MAXINUM_SUBNET_RULELIST];
	size_t subnet_list_size = 0;
	struct wgn_subnet_rule_t tagged_base_subnet_list[WGN_MAXINUM_SUBNET_RULELIST];
	size_t tagged_base_subnet_list_size = 0;
	struct in_addr lan_addr;

	if (IS_RE())
		return;

	if (!IS_CAP())
		return;

	memset(tagged_base_vlan_list, 0, sizeof(struct wgn_vlan_rule_t) * WGN_MAXINUM_VLAN_RULELIST);
	if (!wgn_vlan_list_get_other_from_nvram(tagged_base_vlan_list, WGN_MAXINUM_VLAN_RULELIST, &tagged_base_vlan_list_size))
		return;

	if (tagged_base_vlan_list_size <= 0)
		return;

	memset(tagged_base_subnet_list, 0, sizeof(struct wgn_subnet_rule_t) * WGN_MAXINUM_SUBNET_RULELIST);
	if (!wgn_subnet_list_get_other_from_nvram(tagged_base_subnet_list, WGN_MAXINUM_SUBNET_RULELIST, &tagged_base_subnet_list_size))
		return;

	if (tagged_base_subnet_list_size <= 0)
		return;

	memset(vlan_list, 0, sizeof(struct wgn_vlan_rule_t) * WGN_MAXINUM_VLAN_RULELIST);
	if (!wgn_vlan_list_get_from_nvram(vlan_list, WGN_MAXINUM_VLAN_RULELIST, &vlan_list_size))
		return;

	if (vlan_list_size <= 0)
		return;

	memset(subnet_list, 0, sizeof(struct wgn_subnet_rule_t) * WGN_MAXINUM_SUBNET_RULELIST);
	if (!wgn_subnet_list_get_from_nvram(subnet_list, WGN_MAXINUM_SUBNET_RULELIST, &subnet_list_size))
		return;

	if (subnet_list_size <= 0)
		return;

	for (i=0, p_vlan_list=&vlan_list[0]; i<vlan_list_size; )
	{
		conflict = 0;
		if (!(p1 = wgn_subnet_list_find(subnet_list, subnet_list_size, p_vlan_list->subnet_name)))
		{
			i++;
			p_vlan_list++;
			continue;
		}

		for (j=0; j<tagged_base_vlan_list_size; j++)
		{
			if (!(p2 = wgn_subnet_list_find(tagged_base_subnet_list, tagged_base_subnet_list_size, tagged_base_vlan_list[j].subnet_name)))
				continue;

			if ((conflict = inet_deconflict(p1->ipaddr, p1->ipmask, p2->ipaddr, p2->ipmask, &lan_addr)))
			{
				cfg_changed = 1;
				SET_SUBNET_RULE(p_vlan_list, p1, htonl(lan_addr.s_addr), 0xffffff00);
				if (!(p1 = wgn_subnet_list_find(subnet_list, subnet_list_size, p_vlan_list->subnet_name)))
					break;
			}
		}

		if (!p1)
		{
			i++;
			p_vlan_list++;
			continue;
		}

		for (j=0; j<vlan_list_size; j++)
		{
			if (i==j)
				continue;

			if (!(p2 = wgn_subnet_list_find(subnet_list, subnet_list_size, vlan_list[j].subnet_name)))
				continue;

			if ((conflict = inet_deconflict(p1->ipaddr, p1->ipmask, p2->ipaddr, p2->ipmask, &lan_addr)))
			{
				cfg_changed = 1;
				SET_SUBNET_RULE(p_vlan_list, p1, ntohl(lan_addr.s_addr), 0xffffff00);
				if (!(p1 = wgn_subnet_list_find(subnet_list, subnet_list_size, p_vlan_list->subnet_name)))
					break;				
			}
		}

		if (conflict == 1)
			continue; 

		i++;
		p_vlan_list++;
	}

   	if (cfg_changed)
 	{
 		wgn_vlan_list_set_to_nvram(vlan_list, vlan_list_size);
 		wgn_subnet_list_set_to_nvram(subnet_list, subnet_list_size);
 	}

 	return;
}

void check_ip_subnet_conflict(
	void)
{
	size_t i, j;
	int cfg_changed = 0;
	int conflict = 0;

	struct wgn_vlan_rule_t *p_vlan_list = NULL;
	struct wgn_vlan_rule_t vlan_list[WGN_MAXINUM_VLAN_RULELIST];
	size_t vlan_total = 0;
	struct wgn_subnet_rule_t *p1 = NULL, *p2 = NULL;
	struct wgn_subnet_rule_t subnet_list[WGN_MAXINUM_SUBNET_RULELIST];
	size_t subnet_total = 0;

	struct in_addr lan_addr;

	char *wan_ipaddr;
	char *wan_netmask;
	char *lan_ipaddr;
	char *lan_netmask;

	if (IS_RE())
		return;

	if (!IS_CAP())
		return;

	wan_ipaddr = wan_netmask = NULL;
	if (nvram_get_int("wan0_state_t") == WAN_STATE_CONNECTED)
	{
		wan_ipaddr = nvram_safe_get("wan0_ipaddr");
		if (wan_ipaddr[0] == '\0') wan_ipaddr = NULL;
		wan_netmask = nvram_safe_get("wan0_netmask");
		if (wan_netmask[0] == '\0') wan_netmask = NULL;
	}

	lan_ipaddr = nvram_safe_get("lan_ipaddr");
	if (lan_ipaddr[0] == '\0') lan_ipaddr = NULL;
	lan_netmask = nvram_safe_get("lan_netmask");
	if (lan_netmask[0] == '\0') lan_netmask = NULL;

	if ((!wan_ipaddr || !wan_netmask) && (!lan_ipaddr || !lan_netmask))
		return;

	memset(vlan_list, 0, sizeof(struct wgn_vlan_rule_t) * WGN_MAXINUM_VLAN_RULELIST);
	if (!wgn_vlan_list_get_from_nvram(vlan_list, WGN_MAXINUM_VLAN_RULELIST, &vlan_total))
		return;

	if (vlan_total <= 0)
		return;

	memset(subnet_list, 0, sizeof(struct wgn_subnet_rule_t) * WGN_MAXINUM_SUBNET_RULELIST);
	if (!wgn_subnet_list_get_from_nvram(subnet_list, WGN_MAXINUM_SUBNET_RULELIST, &subnet_total))
		return;

	if (subnet_total <= 0)
		return;

 	for (i=0, p_vlan_list=&vlan_list[0]; i<vlan_total; )
 	{
 		conflict = 0;
 		if (!(p1 = wgn_subnet_list_find(subnet_list, subnet_total, p_vlan_list->subnet_name)))
 		{
 			i++;
 			p_vlan_list++;
 			continue;
 		}

 		if (wan_ipaddr && wan_netmask)
 		{
 			if (inet_deconflict(p1->ipaddr, p1->ipmask, wan_ipaddr, wan_netmask, &lan_addr))
 			{
				SET_SUBNET_RULE(p_vlan_list, p1, ntohl(lan_addr.s_addr), 0xffffff00);
				cfg_changed = 1;
				continue;
			}
 		}

 		if (lan_ipaddr && lan_netmask)
 		{
 			if (inet_deconflict(p1->ipaddr, p1->ipmask, lan_ipaddr, lan_netmask, &lan_addr))
 			{
				SET_SUBNET_RULE(p_vlan_list, p1, ntohl(lan_addr.s_addr), 0xffffff00);
				cfg_changed = 1;
				continue;
			}
 		}

 		for (j=0; j<vlan_total; j++)
 		{
 			if (i==j)
 				continue;

 			if (!(p2 = wgn_subnet_list_find(subnet_list, subnet_total, vlan_list[j].subnet_name)))
 				continue;

 			if ((conflict = inet_deconflict(p1->ipaddr, p1->ipmask, p2->ipaddr, p2->ipmask, &lan_addr)))
 				break;
 		}

 		if (conflict)
 		{
 			SET_SUBNET_RULE(p_vlan_list, p1, ntohl(lan_addr.s_addr), 0xffffff00);
 			cfg_changed = 1;
 			continue;
 		}

 		i++;
 		p_vlan_list++;
 	}

 	if (cfg_changed)
 	{
 		wgn_subnet_list_set_to_nvram(subnet_list, subnet_total);
 		wgn_vlan_list_set_to_nvram(vlan_list, vlan_total);
 	}

 	return;		
}

void check_vlan_id_conflict(
	void)
{
#define WGN_VID_ALL_RANGE_START 	50
#define WGN_VID_ALL_RANGE_END 		4095
#define WGN_VID_RANGE_START 		510
#define WGN_VID_RANGE_END			1510			

	int iptv_vids[3]={0}, x = 0, vid = 0, conflict = 0, find_all_range = 0, cfg_changed = 0;
	unsigned int wan_allow_list = 0x00000000;//wan0,wan1
	unsigned int lan_allow_list = 0x00FF00FF;//lan1~8
	size_t i, j;

	struct wgn_vlan_rule_t tagged_base_vlan_list[WGN_MAXINUM_VLAN_RULELIST];
	size_t tagged_base_vlan_list_size = 0;
	struct wgn_vlan_rule_t vlan_list[WGN_MAXINUM_VLAN_RULELIST];
	size_t vlan_list_size = 0;

	memset(tagged_base_vlan_list, 0, sizeof(struct wgn_vlan_rule_t) * WGN_MAXINUM_VLAN_RULELIST);
	if (!wgn_vlan_list_get_other_from_nvram(tagged_base_vlan_list, WGN_MAXINUM_VLAN_RULELIST, &tagged_base_vlan_list_size))
		return;

	memset(vlan_list, 0, sizeof(struct wgn_vlan_rule_t) * WGN_MAXINUM_VLAN_RULELIST);
	if (!wgn_vlan_list_get_from_nvram(vlan_list, WGN_MAXINUM_VLAN_RULELIST, &vlan_list_size))
		return;

	get_iptv_and_dualwan_info(iptv_vids,3,&wan_allow_list,&lan_allow_list);
	for (i=0, find_all_range=0, vid=0; i<vlan_list_size; )
	{
		if (conflict == 1)
		{
			conflict = 0;
			cfg_changed = 1;
			if (vid == 0)
				vid = WGN_VID_RANGE_START;
			else if (vid == -1)
			{
				vlan_list[i].vid = -1;
				i++;
				continue;
			}
			else
			{
				if (!find_all_range)
				{
					if (++vid > WGN_VID_RANGE_END)
					{
						vid = WGN_VID_ALL_RANGE_START;
						find_all_range = 1;
					}
				}
				else 
				{
					if (++vid > WGN_VID_ALL_RANGE_END)
						vid = -1;
				}				
			}

			if (vid == -1)
			{
				vlan_list[i].vid = -1;
				i++;
				continue;				
			}

			vlan_list[i].vid = vid;
			DBG_PRINTF("solve conflict: vlan_list[%d].vid: %d\n", i, vlan_list[i].vid);
		}

		if (vlan_list[i].vid == -1)
		{
			conflict = 1;
			continue;
		}

		// check iptv
		for (x=0; x<3; x++)
		{
			if (vlan_list[i].vid == iptv_vids[x])
			{
				DBG_PRINTF("IPTV conflict: vlan_list[%d].vid: %d\n", i, vlan_list[i].vid);
				conflict = 1;
				break;
			}
		}

		if (conflict == 1)
			continue;

		for (j=0; j<tagged_base_vlan_list_size; j++)
		{
			if (vlan_list[i].vid == tagged_base_vlan_list[j].vid)
			{
				DBG_PRINTF("TAGGED_BASE conflict: vlan_list[%d].vid: %d\n", i, vlan_list[i].vid);
				conflict = 1;
				break;
			}						
		}

		if (conflict == 1)
			continue;

		// self
		for (j=0; j<vlan_list_size; j++)
		{
			if (i==j)
				continue;

			if (vlan_list[i].vid == vlan_list[j].vid)
			{
				DBG_PRINTF("WGN conflict: vlan_list[%d].vid: %d\n", i, vlan_list[i].vid);
				conflict = 1;
				break;				
			}
		}

		if (conflict == 1)
			continue;

		i++;
	}

 	if (cfg_changed)
 	{
 		wgn_vlan_list_set_to_nvram(vlan_list, vlan_list_size);
 	}

	return;
}

extern
void wgn_check_subnet_conflict(
	void)
{
	if (!IS_CAP() && !IS_RE())
		return;

	// check_ip_subnet_conflict
	check_ip_subnet_conflict();
	// check_vlan_rulelist_subnet_conflict
	check_vlan_rulelist_subnet_conflict();
	// check_vlan_id_conflict
	check_vlan_id_conflict();
	return;
}

extern 
void wgn_check_avalible_brif(
	void)
{
#define WGN_BRIF_INDEX_START    1

	size_t i = 0, j = 0, vlan_total = 0;	
	char br_ifnames[32 * WGN_MAXINUM_VLAN_RULELIST];
	char brif_rule[256 * WGN_MAXINUM_VLAN_RULELIST];
	char s[81], brx[81];
	struct wgn_vlan_rule_t vlan_list[WGN_MAXINUM_VLAN_RULELIST];

	if (!IS_CAP() && !IS_RE())
		return;

	nvram_unset(WGN_BRIF_RULE_NVRAM);
	memset(vlan_list, 0, sizeof(struct wgn_vlan_rule_t) * WGN_MAXINUM_VLAN_RULELIST);
	if (!wgn_vlan_list_get_from_nvram(vlan_list, WGN_MAXINUM_VLAN_RULELIST, &vlan_total))
		return;

	memset(br_ifnames, 0, sizeof(br_ifnames));
	if (!get_unused_brif(vlan_total, br_ifnames, sizeof(br_ifnames)))
		return;

	memset(brif_rule, 0, sizeof(brif_rule));
	for (i=0, j=WGN_BRIF_INDEX_START; i<vlan_total && j<WGN_MAXINUM_VLAN_RULELIST; i++, j++)
	{
        memset(brx, 0, sizeof(brx));
        snprintf(brx, sizeof(brx)-1, "br%d", j);
        if (strstr(nvram_safe_get("wgn_ifnames"), brx) || strstr(br_ifnames, brx))
        {
            // brif_rules
            strlcat(brif_rule, "<", sizeof(brif_rule));

            // brif_rules : brif name
            memset(s, 0, sizeof(s));
            snprintf(s, sizeof(s)-1, "%s>", brx);
            strlcat(brif_rule, s, sizeof(brif_rule));

            // brif_rules : subnet name
            memset(s, 0, sizeof(s));
            snprintf(s, sizeof(s)-1, "%s>", vlan_list[i].subnet_name);
            strlcat(brif_rule, s, sizeof(brif_rule));
        }
	}

	if (strlen(brif_rule) > 0)
		nvram_set(WGN_BRIF_RULE_NVRAM, brif_rule);

	return;
}

extern
void wgn_init(
	void)
{
	return;
}

int get_wl_dwds(
	int unit, 
	int subunit)
{
	char s[81];
	memset(s, 0, sizeof(s));
	if (subunit <= 0)
		snprintf(s, sizeof(s)-1, "wl%d_dwds", unit);
	else	
		snprintf(s, sizeof(s)-1, "wl%d.%d_dwds", unit, subunit);
	return nvram_get_int(s);
}

int get_wl_lanaccess(
	int unit, 
	int subunit)
{
	char s[81];

	if (IS_CAP()) {
		if (sw_mode() != SW_MODE_ROUTER)
			return 1;
	}

	if (IS_RE()) {
		if (nvram_get_int("wgn_without_vlan") != 0)
			return 1;
	}
	
	memset(s, 0, sizeof(s));
	if (subunit <= 0)
		snprintf(s, sizeof(s)-1, "wl%d_lanaccess", unit);
	else
		snprintf(s, sizeof(s)-1, "wl%d.%d_lanaccess", unit, subunit);
	return (nvram_match(s, "on")) ? 1 : 0;
}


int get_wl_bss_enabled(
	int unit, 
	int subunit)
{
	char s[81];

	if (unit < 0)
		return 0;

	if (IS_RE() && subunit == 1)
		return 0;

	memset(s, 0, sizeof(s));
	if (subunit <= 0)
		snprintf(s, sizeof(s)-1, "wl%d_bss_enabled", unit);
	else
		snprintf(s, sizeof(s)-1,  "wl%d.%d_bss_enabled", unit, subunit);
	
	return nvram_get_int(s);
}

char* get_wl_ifname(
	int unit, 
	int subunit, 
	char *buffer, 
	size_t buffer_size)
{
	char s[81], *ss = NULL, *ret = NULL;

	if (!buffer || buffer_size <= 0 || unit < 0)
		return NULL;

	memset(s, 0, sizeof(s));
	if (subunit <= 0)
		snprintf(s, sizeof(s), "wl%d_ifname", unit);
	else
		snprintf(s, sizeof(s)-1, "wl%d.%d_ifname", unit, subunit);

	ss = nvram_get(s);
	if (ss && strlen(ss) > 0)
	{
		memcpy(buffer, ss, (strlen(ss) > buffer_size) ? buffer_size : strlen(ss));
		ret = &buffer[0];
	}

	return ret;
}


int get_wl_unit_by_ifname(
	char *wl_ifname,
	int *unit, 
	int *subunit)
{	
	int found = 0;
	int index = 0, subindex = 0;
	char s[81], *ifname = NULL;

	if (!ifname)
		return 0;

	if (!unit)
		*unit = -1;

	if (!subunit)
		*subunit = -1;

	for (index = 0; index < num_of_wl_if(); index++)
	{
		memset(s, 0, sizeof(s));
		snprintf(s, sizeof(s), "wl%d_ifname", index);
		ifname = nvram_safe_get(s);
		if (!strncmp(ifname, wl_ifname, strlen(ifname)))
		{
			found = 1;
			subindex = -1;
			break;
		}

		for (subindex = 1; subindex < 32; subindex++)
		{
			memset(s, 0, sizeof(s));
			snprintf(s, sizeof(s), "wl%d.%d_ifname", index, subindex);
			ifname = nvram_safe_get(s);
			if (!strncmp(ifname, wl_ifname, strlen(ifname)))
			{
				found = 1;
				break;
			}			
		}

		if (found)
			break;
	}

	if (found)
	{
		if (!unit)
			*unit = index;
		if (!subunit && subindex > -1)
			*subunit = subindex;
	}

	return found;
}


int is_wlif(
	char *ifname)
{
	int ret = 0;
	char word[33];
	char *next = NULL;

	if (!ifname)
		return ret;

	foreach(word, nvram_safe_get("wl_ifnames"), next)
		if ((ret = !strncmp(word, ifname, strlen(ifname))))
			break;

	return ret;
}

extern 
void wgn_filter_forward(
	FILE *fp)
{	
	char word[64];
	char *next = NULL;
	char *wan_ifname = NULL;
    char *pppoe_ifname = NULL;
	char *wgn_ifnames = NULL;

	struct brif_rule_t *p_brif_rule = NULL;
	struct brif_rule_t brif_list[WGN_MAXINUM_VLAN_RULELIST];
	size_t brif_listsize = 0;
	wgn_vlan_rule *p_vlan_rule = NULL;
	wgn_vlan_rule vlan_list[WGN_MAXINUM_VLAN_RULELIST];
	size_t vlan_listsize = 0;

	if (IS_RE())
		return;

	if (!IS_CAP())
		return;

	if (!fp)
		return;

    wan_ifname = nvram_safe_get(WGN_WAN_IFNAME);
	if (wan_ifname[0] == '\0') 
		return;

	wgn_ifnames = nvram_safe_get(WGN_IFNAMES);
	if (wgn_ifnames[0] == '\0')
		return;

    if (nvram_invmatch("wan0_pppoe_ifname", ""))
        pppoe_ifname = nvram_get("wan0_pppoe_ifname");

	memset(brif_list, 0, sizeof(struct brif_rule_t) * WGN_MAXINUM_VLAN_RULELIST);
	if (!brif_list_get_from_nvram(brif_list, WGN_MAXINUM_VLAN_RULELIST, &brif_listsize))
		return;

	memset(vlan_list, 0, sizeof(wgn_vlan_rule) * WGN_MAXINUM_VLAN_RULELIST);
	if (!wgn_vlan_list_get_from_nvram(vlan_list, WGN_MAXINUM_VLAN_RULELIST, &vlan_listsize))
		return;

	foreach (word, wgn_ifnames, next)
	{
		if (!(p_brif_rule = brif_list_find(brif_list, brif_listsize, word)))
			continue;

		if (!(p_vlan_rule = wgn_vlan_list_find(vlan_list, vlan_listsize, p_brif_rule->subnet_name)))
			continue;

		if (!p_vlan_rule->internet)
			continue;

		//iptables -A  FORWARD  -i brX -o eth0 -j ACCEPT
		fprintf(fp, "-A FORWARD -i %s -o %s -j ACCEPT\n", word, wan_ifname);

        if (pppoe_ifname)
            fprintf(fp, "-A FORWARD -i %s -o %s -j ACCEPT\n", word, pppoe_ifname);
	}

	return;
}

void wgn_filter_input(
	FILE *fp)
{
	char word[64];
	char *next = NULL;
	char *wgn_ifnames = NULL;

	struct brif_rule_t *p_brif_rule = NULL;
	struct brif_rule_t brif_list[WGN_MAXINUM_VLAN_RULELIST];
	size_t brif_listsize = 0;
	wgn_vlan_rule *p_vlan_rule = NULL;
	wgn_vlan_rule vlan_list[WGN_MAXINUM_VLAN_RULELIST];
	size_t vlan_listsize = 0;

	int unit = 0;
	int subunit = 0; 

	if (IS_RE())
		return;

	if (!IS_CAP())
		return;

	if (!fp)
		return;

	wgn_ifnames = nvram_safe_get(WGN_IFNAMES);
	if (wgn_ifnames[0] == '\0')
		return;

	memset(brif_list, 0, sizeof(struct brif_rule_t) * WGN_MAXINUM_VLAN_RULELIST);
	if (!brif_list_get_from_nvram(brif_list, WGN_MAXINUM_VLAN_RULELIST, &brif_listsize))
		return;

	memset(vlan_list, 0, sizeof(wgn_vlan_rule) * WGN_MAXINUM_VLAN_RULELIST);
	if (!wgn_vlan_list_get_from_nvram(vlan_list, WGN_MAXINUM_VLAN_RULELIST, &vlan_listsize))
		return;
	foreach(word, wgn_ifnames, next)
	{
		if (!(p_brif_rule = brif_list_find(brif_list, brif_listsize, word)))
			continue;

		if (!(p_vlan_rule = wgn_vlan_list_find(vlan_list, vlan_listsize, p_brif_rule->subnet_name)))
			continue;

		if (!p_vlan_rule->internet)
			continue;

		unit = subunit = -1;
		wgn_get_wl_unit(&unit, &subunit, p_vlan_rule);
		if (unit < 0 || subunit <= 0)
			continue;

		if (!get_wl_bss_enabled(unit, subunit))
			continue;

		if (get_wl_lanaccess(unit, subunit))
			continue;

		// iptables -A INPUT -i brX -p udp --dport 53 -j ACCEPT
		fprintf(fp, "-A INPUT -i %s -p udp --dport 53 -j ACCEPT\n", word);
		// iptables -A INPUT -i brX -p udp --dport 67 -j ACCEPT
		fprintf(fp, "-A INPUT -i %s -p udp --dport 67 -j ACCEPT\n", word);
		// iptables -A INPUT -i brX -p udp --dport 68 -j ACCEPT
		fprintf(fp, "-A INPUT -i %s -p udp --dport 68 -j ACCEPT\n", word);
		// iptables -A INPUT -i brX -j DROP
		fprintf(fp, "-A INPUT -i %s -j DROP\n", word);
	}

	return;
}

extern 
void wgn_dnsmasq_conf(
	FILE *fp)
{
	char *s = NULL;

	size_t i = 0;
	struct brif_rule_t brif_list[WGN_MAXINUM_VLAN_RULELIST];
	size_t brif_listsize = 0;
	struct wgn_subnet_rule_t *p_subnet_rule = NULL;
	struct wgn_subnet_rule_t subnet_list[WGN_MAXINUM_SUBNET_RULELIST];
	size_t subnet_listsize = 0;

	if (IS_RE())
		return;

	if (!IS_CAP())
		return;

	if (fp == NULL)
		return;

	if (!(s = nvram_get(WGN_BRIF_RULE_NVRAM)) || strlen(s) == 0)
		wgn_check_avalible_brif();

	memset(brif_list, 0, sizeof(struct brif_rule_t) * WGN_MAXINUM_VLAN_RULELIST);
	if (!brif_list_get_from_nvram(brif_list, WGN_MAXINUM_VLAN_RULELIST, &brif_listsize))
		return;

	if (brif_listsize <= 0)
		return;

	memset(subnet_list, 0, sizeof(struct wgn_subnet_rule_t) * WGN_MAXINUM_SUBNET_RULELIST);
	if (!wgn_subnet_list_get_from_nvram(subnet_list, WGN_MAXINUM_SUBNET_RULELIST, &subnet_listsize))
		return;

	if (subnet_listsize <= 0)
		return;

	for (i=0; i<brif_listsize; i++)
	{
		if (!(p_subnet_rule = wgn_subnet_list_find(subnet_list, subnet_listsize, brif_list[i].subnet_name)))
			continue;

		if (!p_subnet_rule->dhcp_enable)
			continue;

		fprintf(fp, "interface=%s\n", brif_list[i].br_name);
		fprintf(fp, "dhcp-range=%s,%s,%s,%s,%ds\n", brif_list[i].br_name, p_subnet_rule->dhcp_start, p_subnet_rule->dhcp_end, p_subnet_rule->ipmask, p_subnet_rule->dhcp_lease);
		// Gateway
		fprintf(fp, "dhcp-option=%s,3,%s\n", brif_list[i].br_name, p_subnet_rule->ipaddr);
		// Domain
		if (strlen(p_subnet_rule->domain_name) > 0) fprintf(fp, "dhcp-option=%s,15,%s\n", brif_list[i].br_name, p_subnet_rule->domain_name);
		// DNS server and additional router address
		if (strlen(p_subnet_rule->dns) > 0) fprintf(fp, "dhcp-option=%s,6,%s,0.0.0.0\n", brif_list[i].br_name, p_subnet_rule->dns);
		// WINS server
		if (strlen(p_subnet_rule->wins) > 0) fprintf(fp, "dhcp-option=%s,44,%s\n", brif_list[i].br_name, p_subnet_rule->wins);
	}
	
	return;
}

extern 
int wgn_if_check_used(
	char *ifname)
{
	int found;
	int unit;
	int subunit;
	char wl_ifname[33];
	size_t i;
	size_t total = 0;
	struct wgn_vlan_rule_t vlan_list[WGN_MAXINUM_VLAN_RULELIST];
	
	if (!IS_CAP() && !IS_RE())
		return 0;

	if (!ifname)
		return 0;

	memset(vlan_list, 0, sizeof(struct wgn_vlan_rule_t) * WGN_MAXINUM_VLAN_RULELIST);
	if (!wgn_vlan_list_get_from_nvram(vlan_list, WGN_MAXINUM_VLAN_RULELIST, &total))
		return 0;

	for (i=0, found=0; i<total; i++)
	{
		if (!vlan_list[i].enable)
			continue;

		wgn_get_wl_unit(&unit,&subunit,&vlan_list[i]);
		if (unit < 0 || subunit <= 0)
			continue;

		if (!get_wl_bss_enabled(unit, subunit))
			continue;

		memset(wl_ifname, 0, sizeof(wl_ifname));
		if (!get_wl_ifname(unit, subunit, wl_ifname, sizeof(wl_ifname)-1))
			continue;

		if (strncmp(ifname, wl_ifname, strlen(wl_ifname)) != 0)
			continue;

		if (get_wl_lanaccess(unit, subunit))
			continue;

		found = 1;
		break;
	}

	return (found) ? 1 : 0;
}


void update_br0_ifnames(
	char *guest_ifnames)
{
	int found = 0;

	char *s = NULL;
	char word1[256], *next1 = NULL;
	char word2[256], *next2 = NULL;
	char word3[256], *next3 = NULL;
	char br0_ifnames[2048];

	if (!guest_ifnames)
		return;

	s = nvram_safe_get("br0_ifnames");
	if (s[0] == '\0')
		return;

	memset(br0_ifnames, 0, sizeof(br0_ifnames));
	foreach (word1, nvram_safe_get("br0_ifnames"), next1)
	{

		found = 0;
		SKIP_ABSENT_FAKE_IFACE(word1);
		foreach (word2, guest_ifnames, next2)
			if ((found = (strncmp(word1, word2, strlen(word2)) == 0)))
				break;

		if (!found && IS_RE())
		{
			foreach(word3, nvram_safe_get("wl_ifnames"), next3)
			{
				SKIP_ABSENT_FAKE_IFACE(word3);
				if ((found = (strncmp(word1, word3, strlen(word3)) == 0)))
					break;
			}
		}

		if (!found)
		{
			strlcat(br0_ifnames, word1, sizeof(br0_ifnames));
			strlcat(br0_ifnames, " ", sizeof(br0_ifnames));
		}
	}

	if (strlen(br0_ifnames) > 0)
	{
		br0_ifnames[strlen(br0_ifnames)-1] = '\0';
		nvram_set("br0_ifnames", br0_ifnames);
	}

	return;
}

void update_lan_ifnames(
	char *guest_ifnames)
{
	int found = 0;

	char *s = NULL;
	char word1[256], *next1 = NULL;
	char word2[256], *next2 = NULL;
	char lan_ifnames[2048];

	if (!guest_ifnames)
		return;

	s = nvram_safe_get("lan_ifnames");
	if (s[0] == '\0')
		return;

	memset(lan_ifnames, 0, sizeof(lan_ifnames));
	foreach (word1, nvram_safe_get("lan_ifnames"), next1)
	{
		found = 0;
		SKIP_ABSENT_FAKE_IFACE(word1);
		foreach (word2, guest_ifnames, next2)
			if ((found = strncmp(word1, word2, strlen(word2)) == 0))
				break;

		if (!found)
		{
			strlcat(lan_ifnames, word1, sizeof(lan_ifnames));
			strlcat(lan_ifnames, " ", sizeof(lan_ifnames));
		}
	}

	if (strlen(lan_ifnames) > 0)
	{
		lan_ifnames[strlen(lan_ifnames)-1] = '\0';
		nvram_set("lan_ifnames", lan_ifnames);
	}

	return;
}

extern 
int wgn_is_wds_guest_vlan(
	char *ifname)
{
	int ret = 0, total = 0;
	size_t i = 0;
	int unit1 = 0, unit2 = 0, unit3 = 0, unit4 = 0, unit5 = 0, get_vid = 0;

	struct brif_rule_t brif_list[WGN_MAXINUM_VLAN_RULELIST];
	size_t brif_total = 0;
	struct wgn_vlan_rule_t *p_vlan_rule = NULL;
	struct wgn_vlan_rule_t vlan_list[WGN_MAXINUM_VLAN_RULELIST];
	size_t vlan_total = 0;

	if (!ifname || strlen(ifname) == 0)
		return 0;

	if (strncmp(ifname, "wds", 3) != 0)
		return 0;

	total = sscanf(ifname, "wds%d.%d.%d.%d.%d", &unit1, &unit2, &unit3, &unit4, &unit5);

#if defined(HND_ROUTER)
	if (total == 4)	// wdsX.X.X.X
		get_vid = unit4;
	else
		return 0;
#else
	if (total == 3)	// wdsX.X.X
		get_vid = unit3;
	else
		return 0;
#endif	// HND_ROUTER	

	memset(brif_list, 0, sizeof(struct brif_rule_t) * WGN_MAXINUM_VLAN_RULELIST);
	if (!brif_list_get_from_nvram(brif_list, WGN_MAXINUM_VLAN_RULELIST, &brif_total))
		return 0;

	memset(vlan_list, 0, sizeof(struct wgn_vlan_rule_t) * WGN_MAXINUM_VLAN_RULELIST);
	if (!wgn_vlan_list_get_from_nvram(vlan_list, WGN_MAXINUM_VLAN_RULELIST, &vlan_total))
		return 0;

	for (i=0; i<brif_total; i++)
	{
		if (!(p_vlan_rule = wgn_vlan_list_find(vlan_list, vlan_total, brif_list[i].subnet_name)))
			continue;

		if ((ret = (get_vid == p_vlan_rule->vid)) == 1)
			break;
	}

	return ret;
}

extern 
void wgn_hotplug_net(
	char *interface, 
	int action /* 0:del, 1:add */)
{
	int total = 0, unit = 0, subunit = 0, subunit1 = 0, subunit2 = 0, subunit3 = 0;
	size_t i;
	struct brif_rule_t brif_list[WGN_MAXINUM_VLAN_RULELIST];
	size_t brif_total = 0;
	struct wgn_vlan_rule_t *p_vlan_rule = NULL;
	struct wgn_vlan_rule_t vlan_list[WGN_MAXINUM_VLAN_RULELIST];
	size_t vlan_total = 0;

	char vid[33], vif[IFNAMSIZ+1];
#if defined(WGN_HAVE_VLAN0)	
	char vif0[64];
#endif	// defined(WGN_USE_VLAN0)

	if (!interface || strlen(interface) == 0)
		return;

	if (action != 0 && action != 1)
		return;

	if (strncmp(interface, "wds", 3) != 0)
		return;

	total = sscanf(interface, "wds%d.%d.%d.%d.%d", &unit, &subunit, &subunit1, &subunit2, &subunit3);
#if defined(HND_ROUTER)
	if (total != 3)	// wdsX.X.X.0
		return;
#else	// HND_ROUTER
	if (total != 2)	// wdsX.X
		return;
#endif	// HND_ROUTER	

	if (!brif_list_get_from_nvram(brif_list, WGN_MAXINUM_VLAN_RULELIST, &brif_total))
		return;

	if (!wgn_vlan_list_get_from_nvram(vlan_list, WGN_MAXINUM_VLAN_RULELIST, &vlan_total))
		return;

	for (i=0; i<brif_total; i++)
	{
		if (!(p_vlan_rule = wgn_vlan_list_find(vlan_list, vlan_total, brif_list[i].subnet_name)))
			continue;

		if (!p_vlan_rule->enable)
			continue;

		if (p_vlan_rule->vid <= 0)
			continue;

		unit = subunit = 0;
		wgn_get_wl_unit(&unit, &subunit, p_vlan_rule);
		if (!get_wl_bss_enabled(unit, subunit))
			continue;

		if (get_wl_lanaccess(unit, subunit))
			continue;

		memset(vif, 0, sizeof(vif));
		snprintf(vif, sizeof(vif)-1, "%s.%d", interface, p_vlan_rule->vid);
		memset(vid, 0, sizeof(vid));
		snprintf(vid, sizeof(vid)-1, "%d", p_vlan_rule->vid);

#if defined(WGN_HAVE_VLAN0)
		memset(vif0, 0, sizeof(vif0));
		snprintf(vif0, sizeof(vif0)-1, "%s.0", interface);
#endif	// defined(WGN_HAVE_VLAN0)

		if (action == 1)	// add
		{
#if defined(HND_ROUTER)
#if defined(WGN_HAVE_VLAN0)			
			//if (find_br0_ifnames_by_ifname(interface)) exec_cmd("brctl", "delif", "br0", interface);	
			exec_cmd("brctl", "delif", "br0", interface);		
			// vlanctl --mcast --if-create-name eth6 eth6.0 --if eth6 --set-if-mode-rg;
			if (!iface_is_exists(vif0)) exec_cmd("vlanctl", "--mcast", "--if-create-name", interface, vif0, "--if", interface, "--set-if-mode-rg");
			// vlanctl --mcast --if-create-name eth6 eth6.55 --if eth6 --set-if-mode-rg;
			exec_cmd("vlanctl", "--mcast", "--if-create-name", interface, vif, "--if", interface, "--set-if-mode-rg");
			// vlanctl --if eth6 --rx --tags 0 --set-rxif eth6.0 --rule-append; 
			exec_cmd("vlanctl", "--if", interface, "--rx", "--tags", "0", "--set-rxif", vif0, "--rule-append");
			// vlanctl --if eth6 --tx --tags 0 --filter-txif eth6.55 --push-tag --set-vid 55 0 --set-pbits 0 0 --rule-append;
			exec_cmd("vlanctl", "--if", interface, "--tx", "--tags", "0", "--filter-txif", vif, "--push-tag", "--set-vid", vid, "0", "--set-pbits", "0", "0", "--rule-append");
			// vlanctl --if eth6 --tx --tags 0 --filter-txif eth6.0 --rule-append; 
			exec_cmd("vlanctl", "--if", interface, "--tx", "--tags", "0", "--filter-txif", vif0, "--rule-append");
			// vlanctl --if eth6 --rx --tags 1 --filter-vid 55 0 --pop-tag --set-rxif eth6.55 --rule-append; 
			exec_cmd("vlanctl", "--if", interface, "--rx", "--tags", "1", "--filter-vid", vid, "0", "--pop-tag", "--set-rxif", vif, "--rule-append");
			// ip link set eth6.0 up;
			exec_cmd("ip", "link", "set", vif0, "up");
			// ifconfig vif0 IFF_MULTICAST
			ifconfig(vif0, IFUP | IFF_MULTICAST, NULL, NULL);
			// ip link set eth6.55 up;
			exec_cmd("ip", "link", "set", vif, "up");
			// ifconfig vif IFF_MULTICAST
			ifconfig(vif, IFUP | IFF_MULTICAST, NULL, NULL);
			// brctl addif br0 eth6.0;			
			exec_cmd("brctl", "delif", "br0", vif);
			exec_cmd("brctl", "addif", "br0", vif0);
			if (strncmp(interface, "wds", 3) == 0)
				exec_cmd("brctl", "addif", "br0", interface);
			//if (find_br0_ifnames_by_ifname(interface)) exec_cmd("brctl", "addif", "br0", interface);
			exec_cmd("brctl", "addif", brif_list[i].br_name, vif);
#else	// defined(WGN_HAVE_VLAN0)
			if(strncmp(interface, "wds", 3) == 0)
				exec_cmd("brctl", "delif", "br0", interface);
			exec_cmd("vconfig", "set_name_type", "DEV_PLUS_VID_NO_PAD");
			// vconfig add interface vid
			exec_cmd("vconfig", "add", interface, vid);
			// ifconfig vifname UP
			ifconfig(vif, IFUP | IFF_MULTICAST, NULL, NULL);
			// brctl addif brX vifname
			if(strncmp(interface, "wds", 3) == 0)
				exec_cmd("brctl", "addif", "br0", interface);
			exec_cmd("brctl", "addif", brif_list[i].br_name, vif);
			exec_cmd("vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");						
#endif	// defined(WGN_HAVE_VLAN0)			
#else	// defined(HND_ROUTER)
			if(strncmp(interface, "wds", 3) == 0)
				exec_cmd("brctl", "delif", "br0", interface);
			exec_cmd("vconfig", "set_name_type", "DEV_PLUS_VID_NO_PAD");
			// vconfig add interface vid
			exec_cmd("vconfig", "add", interface, vid);
			// ifconfig vifname UP
			ifconfig(vif, IFUP | IFF_MULTICAST, NULL, NULL);
			// brctl addif brX vifname
			if(strncmp(interface, "wds", 3) == 0)
				exec_cmd("brctl", "addif", "br0", interface);
			exec_cmd("brctl", "addif", brif_list[i].br_name, vif);
			exec_cmd("vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");			
#endif	// defined(HND_ROUTER)
		}
		else	// del
		{
#if defined(HND_ROUTER)			
#if defined(WGN_HAVE_VLAN0)			
			exec_cmd("vlanctl", "--if-delete", vif);
			exec_cmd("vlanctl", "--if-delete", vif0);
#else	// defined(WGN_HAVE_VLAN0)
			exec_cmd("vconfig", "rem", vif);
#endif	// defined(WGN_HAVE_VLAN0)			
#else	// defined(HND_ROUTER)			
			exec_cmd("vconfig", "rem", vif);
#endif	// defined(HND_ROUTER)
		}
	}

	return;
}

char *get_sta_bh_ifnames(
	char *buffer,
	size_t buffer_size)
{
	char *s = NULL;

	if (!buffer || buffer_size <= 0)
		return NULL;

	memset(buffer, 0, buffer_size);

	if (!IS_RE())
		return NULL;

	if (!(s = nvram_get("sta_phy_ifnames")))
		return NULL;

	if (strlen(s) > 0 && strlen(s) < buffer_size)
		strlcpy(buffer, s, buffer_size);

	return (strlen(buffer) > 0) ? buffer : NULL; 
}

char *get_wl_bh_ifnames(
	char *buffer, 
	size_t buffer_size)
{
	int i;
	char *ptr = NULL;
	char *end = NULL;
	char *next = NULL;
	char word[64];
	char *wl_ifnames = NULL;
	char wlifname[33];
	char *s = NULL;
	size_t size;

	if (!buffer || buffer_size <= 0)
		return NULL;

	memset(buffer, 0, buffer_size);
	if (!IS_RE())
	{
		if (!(wl_ifnames = nvram_get("wl_ifnames")))
			return NULL;		
	}

	ptr = &buffer[0];
	end = ptr + buffer_size;

	if (IS_RE())
	{
		for (i=0, size=0; i<num_of_wl_if() && size<buffer_size; i++)
		{
			memset(wlifname, 0, sizeof(wlifname));
			s = get_wl_ifname(i, 1, wlifname, sizeof(wlifname)-1);
			if (s && strlen(s) > 0 && (size + strlen(s) + 1) < buffer_size)
			{
				ptr += snprintf(ptr, end-ptr, "%s ", s);
				size += strlen(s) + 1;
			}
		}
	}
	else
	{
		size = 0;
		foreach (word, wl_ifnames, next)
		{
			if (size >= buffer_size || (size + strlen(word) + 1) >= buffer_size)
				break;
			ptr += snprintf(ptr, end-ptr, "%s ", word);
			size += strlen(word) + 1;
		}
	}

	if (strlen(buffer) > 0)
	{
		buffer[strlen(buffer)-1] = '\0';
	}

	return (strlen(buffer) > 0) ? buffer : NULL;
}


char *get_eth_bh_ifnames(
	char *buffer,
	size_t buffer_size)
{
	char *ptr = NULL;
	char *end = NULL;
	char *lan_ifnames = NULL;

#if defined(HND_ROUTER)
	char word[64];
	char *next = NULL;
	size_t size = 0;
#endif	/* HND_ROUTER */

	if (!buffer || buffer_size <= 0)
		return NULL;	

	memset(buffer, 0, buffer_size);
	if (!(lan_ifnames = nvram_get("lan_ifnames")))
		return NULL;

	ptr = &buffer[0];
	end = ptr + buffer_size;

#if defined(HND_ROUTER)

	foreach(word, lan_ifnames, next)
	{
		if (is_wlif(word) || guest_wlif(word))
			continue;

		if (size >= buffer_size || (size + strlen(word) + 1) >= buffer_size)
			break;

		ptr += snprintf(ptr, end-ptr, "%s ", word);
		size += strlen(word) + 1;
	}

#else	/* HND_ROUTER */

#if defined(RTCONFIG_QCA)
	if (buffer_size> (strlen(nvram_safe_get("wired_ifnames"))+1))
	{
		if (!IS_RE())
			ptr += snprintf(ptr, end-ptr, "%s ", nvram_safe_get("wired_ifnames"));
		else
			ptr += snprintf(ptr, end-ptr, "%s %s ", nvram_safe_get("wired_ifnames"),nvram_safe_get("eth_ifnames"));
	}
#else
	if (buffer_size > 4)
		ptr += snprintf(ptr, end-ptr, "%s ", WAN_IF_ETH);
#endif

#endif	/* HND_ROUTER */

	if (strlen(buffer) > 0)
		buffer[strlen(buffer)-1] = '\0';	

	return (strlen(buffer) > 0) ? buffer : NULL;
}

void destory_vlan(
	void)
{
	char s[33];
	char word[64];
	char *next = NULL;

	int i;

#if defined(HND_ROUTER)
	char wl_bh_ifnames[2048];
	char sta_bh_ifnames[2048];
	char *bh_ifnames = NULL;
#if defined(WGN_HAVE_VLAN0)	
	char vif0[64];
	char *s1 = NULL;
	char *s2 = NULL;
	char sta_phy_ifnames[2048];
#endif	// defined(WGN_HAVE_VLAN0)
#endif	// HND_ROUTER	

	for (i=0; i<WGN_WLIFU_MAX_NO_BRIDGE; i++)
	{
		memset(s, 0, sizeof(s));
		snprintf(s, sizeof(s), "wgn_br%d_lan_ifnames", i);
		foreach(word, nvram_safe_get(s), next)
			exec_cmd("vconfig", "rem", word);
		nvram_unset(s);

		memset(s, 0, sizeof(s));
		snprintf(s, sizeof(s), "wgn_br%d_wl_ifnames", i);
		foreach(word, nvram_safe_get(s), next)
#if defined(HND_ROUTER)		
#if defined(WGN_HAVE_VLAN0)		
			exec_cmd("vlanctl", "--if-delete", word);
#else	// defined(WGN_HAVE_VLAN0)
			exec_cmd("vconfig", "rem", word);
#endif	// defined(WGN_HAVE_VLAN0)			
#else	// defined(HND_ROUTER)
			exec_cmd("vconfig", "rem", word);
#endif	// defined(HND_ROUTER)
		nvram_unset(s);

		memset(s, 0, sizeof(s));
		snprintf(s, sizeof(s), "wgn_br%d_sta_ifnames", i);
		foreach(word, nvram_safe_get(s), next)
#if defined(HND_ROUTER)
#if defined(WGN_HAVE_VLAN0)	
			exec_cmd("vlanctl", "--if-delete", word);
#else	// defined(WGN_HAVE_VLAN0)
			exec_cmd("vconfig", "rem", word);
#endif	// defined(WGN_HAVE_VLAN0)			
#else 	// defined(HND_ROUTER)			
			exec_cmd("vconfig", "rem", word);
#endif	// defined(HND_ROUTER)			
		nvram_unset(s);
	}

#if defined(WGN_HAVE_VLAN0)
	memset(wl_bh_ifnames, 0, sizeof(wl_bh_ifnames));
	if ((bh_ifnames = get_wl_bh_ifnames(wl_bh_ifnames, sizeof(wl_bh_ifnames))))
	{
		foreach (word, bh_ifnames, next)
		{
			memset(vif0, 0, sizeof(vif0));
			snprintf(vif0, sizeof(vif0)-1, "%s.0", word);
			exec_cmd("vlanctl", "--if-delete", vif0);
		}
	}

	memset(sta_bh_ifnames, 0, sizeof(sta_bh_ifnames));
	if ((bh_ifnames = get_sta_bh_ifnames(sta_bh_ifnames, sizeof(sta_bh_ifnames))))
	{
		memset(sta_phy_ifnames, 0, sizeof(sta_phy_ifnames));
		foreach (word, bh_ifnames, next)
		{
			//memset(vif0, 0, sizeof(vif0));
			//snprintf(vif0, sizeof(vif0)-1, "%s.0", word);
			//exec_cmd("vlanctl", "--if-delete", vif0);
			exec_cmd("vlanctl", "--if-delete", word);
			s1 = &word[0];
			if ((s2 = strstr(word, ".0"))) 
				word[s2 - s1] = '\0';
			strlcat(sta_phy_ifnames, word, sizeof(sta_phy_ifnames));
			strlcat(sta_phy_ifnames, " ", sizeof(sta_phy_ifnames));
		}
		
		if (strlen(sta_phy_ifnames) > 0)
		{
			sta_phy_ifnames[strlen(sta_phy_ifnames)-1] = '\0';
			nvram_set("sta_phy_ifnames", sta_phy_ifnames);
		}
	}
#endif	// defined(WGN_HAVE_VLAN0)
	return;
}


char *create_vlan(
	int vlan_id, 
	int type, 
	char *ret_ifnames,
	size_t ifnames_bsize)
{
// type 0:ethernet, 1:wlan, 2:sta
	char vid[16];
	char vif[64];
	char b[2048];
	char *bh_ifnames = NULL;
	char *ptr = NULL, *end = NULL;
	char word[64];
	char *next = NULL;
	size_t size = 0;
    int ret = -1;

#if defined(HND_ROUTER)
#if defined(WGN_HAVE_VLAN0)	
	char sta_phy_ifnames[2048];
	char vif0[64];
	char *s1 = NULL, *s2 = NULL;
#endif	// defined(WGN_HAVE_VLAN0)		
#endif	// HND_ROUTER

	if (!ret_ifnames || ifnames_bsize <= 0)
		goto create_vlan_failed;	

	memset(ret_ifnames, 0, ifnames_bsize);
	if (vlan_id <= 0)
		goto create_vlan_failed;

	memset(b, 0, sizeof(b));
	switch (type)
	{
		case 0:
			bh_ifnames = get_eth_bh_ifnames(b, sizeof(b));
			break;
		case 1:
			bh_ifnames = get_wl_bh_ifnames(b, sizeof(b));
			break;
		case 2:
			bh_ifnames = get_sta_bh_ifnames(b, sizeof(b));
			break;
		default:
			goto create_vlan_failed;
	}

	if (!bh_ifnames || strlen(bh_ifnames) == 0)
		goto create_vlan_failed;

	ptr = &ret_ifnames[0];
	end = ptr + ifnames_bsize;

	memset(vid, 0, sizeof(vid));
	snprintf(vid, sizeof(vid)-1, "%d", vlan_id);

#if defined(HND_ROUTER)
	if (type != 0)	// wl & sta
	{
#if defined(WGN_HAVE_VLAN0)			
		memset(sta_phy_ifnames, 0, sizeof(sta_phy_ifnames));
		foreach (word, bh_ifnames, next)
		{
			s1 = &word[0];
			if ((s2 = strstr(word, ".0")))
				word[s2 - s1] = '\0';
			memset(vif, 0, sizeof(vif));
			snprintf(vif, sizeof(vif), "%s.%s", word, vid);
			if (size > ifnames_bsize || (size + strlen(vif) + 1) > ifnames_bsize)
				goto create_vlan_failed;
			ptr += snprintf(ptr, end-ptr, "%s ", vif);
			size += strlen(vif) + 1;
			memset(vif, 0, sizeof(vif));
			snprintf(vif, sizeof(vif)-1, "%s.%s", word, vid);
			if (size > ifnames_bsize || (size + strlen(vif) + 1) > ifnames_bsize)
				goto create_vlan_failed;
			ptr += snprintf(ptr, end-ptr, "%s ", vif);
			size += strlen(vif) + 1;		
		}

		foreach (word, bh_ifnames, next)
		{
			// if (find_br0_ifnames_by_ifname(word)) exec_cmd("brctl", "delif", "br0", word);
			s1 = &word[0];
			if ((s2 = strstr(word, ".0")))
				word[s2 - s1] = '\0';
			exec_cmd("brctl", "delif", "br0", word);
			memset(vif0, 0, sizeof(vif0));
			snprintf(vif0, sizeof(vif0)-1, "%s.0", word);
			memset(vif, 0, sizeof(vif));
			snprintf(vif, sizeof(vif)-1, "%s.%s", word, vid);
			//vlanctl --mcast --if-create-name eth6 eth6.0 --if eth6 --set-if-mode-rg;
			if (!iface_is_exists(vif0)) exec_cmd("vlanctl", "--mcast", "--if-create-name", word, vif0, "--if", word, "--set-if-mode-rg");
			//vlanctl --mcast --if-create-name eth6 eth6.55 --if eth6 --set-if-mode-rg;
			exec_cmd("vlanctl", "--mcast", "--if-create-name", word, vif, "--if", word, "--set-if-mode-rg");
			// vlanctl --if eth6 --rx --tags 0 --set-rxif eth6.0 --rule-append; 
			exec_cmd("vlanctl", "--if", word, "--rx", "--tags", "0", "--set-rxif", vif0, "--rule-append");
			// vlanctl --if eth6 --tx --tags 0 --filter-txif eth6.55 --push-tag --set-vid 55 0 --set-pbits 0 0 --rule-append;
			exec_cmd("vlanctl", "--if", word, "--tx", "--tags", "0", "--filter-txif", vif, "--push-tag", "--set-vid", vid, "0", "--set-pbits", "0", "0", "--rule-append");
			// vlanctl --if eth6 --tx --tags 0 --filter-txif eth6.0 --rule-append; 
			exec_cmd("vlanctl", "--if", word, "--tx", "--tags", "0", "--filter-txif", vif0, "--rule-append");
			// vlanctl --if eth6 --rx --tags 1 --filter-vid 55 0 --pop-tag --set-rxif eth6.55 --rule-append; 
			exec_cmd("vlanctl", "--if", word, "--rx", "--tags", "1", "--filter-vid", vid, "0", "--pop-tag", "--set-rxif", vif, "--rule-append");
			// ip link set eth6.0 up;
			exec_cmd("ip", "link", "set", vif0, "up");
			// ifconfig vif0 allmulti
			ifconfig(vif0, IFUP | IFF_ALLMULTI | IFF_MULTICAST, NULL, NULL);
			// ip link set eth6.55 up;
			exec_cmd("ip", "link", "set", vif, "up");
			// ifconfig vif allmulti
			ifconfig(vif, IFUP | IFF_ALLMULTI | IFF_MULTICAST, NULL, NULL);
			// brctl addif br0 eth6.0;
			// if (find_br0_ifnames_by_ifname(word)) exec_cmd("brctl", "addif", "br0", vif0);
			exec_cmd("brctl", "addif", "br0", vif0);
			exec_cmd("brctl", "addif", "br0", word);

			if (type == 2)	// sta
			{
				strlcat(sta_phy_ifnames, vif0, sizeof(sta_phy_ifnames));
				strlcat(sta_phy_ifnames, " ", sizeof(sta_phy_ifnames));
			}
		}

		if (type == 2 && strlen(sta_phy_ifnames) > 0)
		{
			sta_phy_ifnames[strlen(sta_phy_ifnames) - 1] = '\0';
			nvram_set("sta_phy_ifnames", sta_phy_ifnames);
		}
#else	// defined(WGN_HAVE_VLAN0)
		foreach (word, bh_ifnames, next)
		{
			memset(vif, 0, sizeof(vif));
			snprintf(vif, sizeof(vif)-1, "%s.%s", word, vid);
			if (size > ifnames_bsize || (size + strlen(vif) + 1) > ifnames_bsize)
				goto create_vlan_failed;
			ptr += snprintf(ptr, end-ptr, "%s ", vif);
			size += strlen(vif) + 1;
		}

		exec_cmd("vconfig", "set_name_type", "DEV_PLUS_VID_NO_PAD");
		foreach (word, bh_ifnames, next)
		{      
			memset(vif, 0, sizeof(vif));
			snprintf(vif, sizeof(vif)-1, "%s.%s", word, vid);
			// if (find_br0_ifnames_by_ifname(word)) exec_cmd("brctl", "delif", "br0", word);
			//exec_cmd("brctl", "delif", "br0", word);
			ret = ioctl_bridge(ARG_DELIF, "br0", word);
	        exec_cmd("vconfig", "add", word, vid);
			// if (find_br0_ifnames_by_ifname(word)) exec_cmd("brctl", "addif", "br0", word);
			//if (find_lan_ifnames_by_ifname(word)) exec_cmd("brctl", "addif", "br0", word);
		    if (ret == 0) ioctl_bridge(ARG_ADDIF, "br0", word);
	        ifconfig(vif, IFUP | IFF_ALLMULTI | IFF_MULTICAST, NULL, NULL);
		}
		exec_cmd("vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");
#endif	// defined(WGN_HAVE_VLAN0)						
	}
	else
	{
		foreach (word, bh_ifnames, next)
		{
			memset(vif, 0, sizeof(vif));
			snprintf(vif, sizeof(vif)-1, "%s.%s", word, vid);
			if (size > ifnames_bsize || (size + strlen(vif) + 1) > ifnames_bsize)
				goto create_vlan_failed;

			ptr += snprintf(ptr, end-ptr, "%s ", vif);	
			size += strlen(vif) + 1;							
		}

		exec_cmd("vconfig", "set_name_type", "DEV_PLUS_VID_NO_PAD");
		foreach (word, bh_ifnames, next)
		{
			memset(vif, 0, sizeof(vif));
			snprintf(vif, sizeof(vif)-1, "%s.%s", word, vid);
			// if (find_br0_ifnames_by_ifname(word)) exec_cmd("brctl", "delif", "br0", word);
			//exec_cmd("brctl", "delif", "br0", word);
			ret = ioctl_bridge(ARG_DELIF, "br0", word);
            exec_cmd("vconfig", "add", word, vid);
			// if (find_br0_ifnames_by_ifname(word)) exec_cmd("brctl", "addif", "br0", word);
			//if (find_lan_ifnames_by_ifname(word)) exec_cmd("brctl", "addif", "br0", word);
			if (ret == 0) ioctl_bridge(ARG_ADDIF, "br0", word);
            ifconfig(vif, IFUP | IFF_ALLMULTI | IFF_MULTICAST, NULL, NULL);
		}
		exec_cmd("vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");	
	}
//#else	/* defined(HND_ROUTER) && !(defined(RTCONFIG_HND_ROUTER_AX_675X) && !defined(RTCONFIG_HND_ROUTER_AX_6710)) */
#else	// defined(HND_ROUTER)
	foreach (word, bh_ifnames, next)
	{
		memset(vif, 0, sizeof(vif));
		snprintf(vif, sizeof(vif)-1, "%s.%s", word, vid);
		if (size > ifnames_bsize || (size + strlen(vif) + 1) > ifnames_bsize)
			goto create_vlan_failed;
		ptr += snprintf(ptr, end-ptr, "%s ", vif);
		size += strlen(vif) + 1;
	}

	exec_cmd("vconfig", "set_name_type", "DEV_PLUS_VID_NO_PAD");
	foreach (word, bh_ifnames, next)
	{      
		memset(vif, 0, sizeof(vif));
		snprintf(vif, sizeof(vif)-1, "%s.%s", word, vid);
		// if (find_br0_ifnames_by_ifname(word)) exec_cmd("brctl", "delif", "br0", word);
		//exec_cmd("brctl", "delif", "br0", word);
		ret = ioctl_bridge(ARG_DELIF, "br0", word);
        exec_cmd("vconfig", "add", word, vid);
		// if (find_br0_ifnames_by_ifname(word)) exec_cmd("brctl", "addif", "br0", word);
		//if (find_lan_ifnames_by_ifname(word)) exec_cmd("brctl", "addif", "br0", word);
	    if (ret == 0) ioctl_bridge(ARG_ADDIF, "br0", word);
        ifconfig(vif, IFUP | IFF_ALLMULTI | IFF_MULTICAST, NULL, NULL);
	}
	exec_cmd("vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");
// #endif	/* defined(HND_ROUTER) && !(defined(RTCONFIG_HND_ROUTER_AX_675X) && !defined(RTCONFIG_HND_ROUTER_AX_6710)) */
#endif	// defined(HND_ROUTER)	

	if (strlen(ret_ifnames) > 0)
		ret_ifnames[strlen(ret_ifnames)-1] = '\0';

	return (strlen(ret_ifnames) > 0) ? ret_ifnames : NULL;

create_vlan_failed:
	return NULL;
}

int create_guest_bridge(
	char *br_ifname, 
	char *br_ipaddr,
	char *br_ipmask, 
	char *guest_ifnames, 
	unsigned int vlan_id)
{
	char wl_ifnames[2048];
	char *wl_vifnames = NULL;
	char eth_ifnames[2048];
	char *eth_vifnames = NULL;
	char sta_ifnames[2048];
	char *sta_vifnames = NULL;

	char word[64];
	char *next = NULL;
	char nvram_name[64], nvram_val[1024], s[81];
	int br_idx = 0;

	unsigned char guest_ifmac[6];

	if (!br_ifname)
		goto create_guest_bridge_failed;

	if (sscanf(br_ifname, "br%d", &br_idx) != 1)
		goto create_guest_bridge_failed;

	if (!guest_ifnames)
		goto create_guest_bridge_failed;

	memset(eth_ifnames, 0, sizeof(eth_ifnames));
	eth_vifnames = create_vlan(vlan_id, 0, eth_ifnames, sizeof(eth_ifnames));
	memset(wl_ifnames, 0, sizeof(wl_ifnames));
	wl_vifnames = create_vlan(vlan_id, 1, wl_ifnames, sizeof(wl_ifnames));
	memset(sta_ifnames, 0, sizeof(sta_ifnames));
	sta_vifnames = create_vlan(vlan_id, 2, sta_ifnames, sizeof(sta_ifnames));

	if (!eth_vifnames && !wl_vifnames)
		goto create_guest_bridge_failed;

	if (IS_RE())
		if (!sta_vifnames)
			goto create_guest_bridge_failed;

	memset(nvram_val, 0, sizeof(nvram_val));
	// create bridge interface
	exec_cmd("brctl", "addbr", br_ifname);
	// add guest_ifname to bridge
	memset(guest_ifmac, 0, sizeof(guest_ifmac));
	foreach (word, guest_ifnames, next)
	{
		if (IS_ZERO_MAC(guest_ifmac))
			iface_get_mac(word, guest_ifmac);
		exec_cmd("brctl", "addif", br_ifname, word);
	}
	strlcat(nvram_val, guest_ifnames, sizeof(nvram_val));
	strlcat(nvram_val, " ", sizeof(nvram_val));

	if (!IS_RE())
	{
		// add eth vlan to bridge
		if (eth_vifnames)
		{
			foreach (word, eth_vifnames, next)
				exec_cmd("brctl", "addif", br_ifname, word);
			strlcat(nvram_val, eth_vifnames, sizeof(nvram_val));
			strlcat(nvram_val, " ", sizeof(nvram_val));
		}
		// add wl vlan to bridge
		if (wl_vifnames)
		{
			foreach (word, wl_vifnames, next)
				exec_cmd("brctl", "addif", br_ifname, word);
			strlcat(nvram_val, wl_vifnames, sizeof(nvram_val));
			strlcat(nvram_val, " ", sizeof(nvram_val));
		}
	}
	else
	{
		if (strstr(nvram_safe_get("sta_phy_ifnames"), nvram_safe_get("amas_ifname")))
		{
			// add wl vlan to bridge
			if (sta_vifnames)
			{	
				foreach (word, sta_vifnames, next)
				{
#if defined(RTCONFIG_QCA)
					if (strstr(word,nvram_safe_get("amas_ifname")))
#else
					//if (strstr(nvram_safe_get("amas_ifname"), word))
					if (strstr(word,nvram_safe_get("amas_ifname")))						
#endif
					{			
						exec_cmd("brctl", "addif", br_ifname, word);
						strlcat(nvram_val, word, sizeof(nvram_val));
						strlcat(nvram_val, " ", sizeof(nvram_val));
						break;
					}
				}
			}
		}
		else
		{
			// add eth vlan to bridge 
			if (eth_vifnames)
			{
				foreach (word, eth_vifnames, next)
					exec_cmd("brctl", "addif", br_ifname, word);						
				strlcat(nvram_val, eth_vifnames, sizeof(nvram_val));
				strlcat(nvram_val, " ", sizeof(nvram_val));
			}
		}

		// add wl vlan to bridge
		if (wl_vifnames)
		{
			foreach (word, wl_vifnames, next)
				exec_cmd("brctl", "addif", br_ifname, word);
			strlcat(nvram_val, wl_vifnames, sizeof(nvram_val));
			strlcat(nvram_val, " ", sizeof(nvram_val));
		}				
	}
	// ifconfig brX up
	ifconfig(br_ifname, IFUP | IFF_ALLMULTI | IFF_MULTICAST, br_ipaddr, br_ipmask);
	// assign brX mac address ref guest mac (ifconfig brX hw ether XX:XX:XX:XX:XX:XX)
	if (!IS_ZERO_MAC(guest_ifmac))
	{
		memset(s, 0, sizeof(s));
		snprintf(s, sizeof(s)-1, "%02X:%02X:%02X:%02X:%02X:%02X", 
			guest_ifmac[0], guest_ifmac[1], guest_ifmac[2], guest_ifmac[3], guest_ifmac[4], guest_ifmac[5]);
		exec_cmd("ifconfig", br_ifname, "hw", "ether", s);
	}
	// brctl stp brX on
	exec_cmd("brctl", "stp", br_ifname, "on");

#ifdef RTCONFIG_EMF
	if (nvram_get_int("emf_enable")
#if defined(RTCONFIG_BCMWL6) && !defined(HND_ROUTER)
			|| wl_igs_enabled()
#endif	// defined(RTCONFIG_BCMWL6) && !defined(HND_ROUTER)		
	) {
#if defined(HND_ROUTER) && defined(RTCONFIG_PROXYSTA)
	eval("bcmmcastctl", "mode", "-i",  "br0",  "-p", "1",  "-m", (psta_exist() || psr_exist()) ? "0" : "2");
	eval("bcmmcastctl", "mode", "-i",  "br0",  "-p", "2",  "-m", (psta_exist() || psr_exist()) ? "0" : "2");		
#endif	// defined(HND_ROUTER) && defined(RTCONFIG_PROXYSTA) 		
		exec_cmd("emf", "add", "bridge", br_ifname);
		exec_cmd("igs", "add", "bridge", br_ifname);
	}
#endif	/* RTCONFIG_EMF */

	// set ethernet switch 
	wgn_sysdep_swtich_set(vlan_id);
	// nvram_set wgn_brX_lan_ifnames
	if (eth_vifnames)
	{
		memset(nvram_name, 0, sizeof(nvram_name));
		snprintf(nvram_name, sizeof(nvram_name), "wgn_br%d_lan_ifnames", br_idx);
		nvram_set(nvram_name, eth_vifnames);
	}

	// nvram_set wgn_brX_sta_ifnames
	if (sta_vifnames)
	{
		memset(nvram_name, 0, sizeof(nvram_name));
		snprintf(nvram_name, sizeof(nvram_name), "wgn_br%d_sta_ifnames", br_idx);
		nvram_set(nvram_name, sta_vifnames);
	}

	// nvram_set wgn_brX_wl_ifnames
	if (wl_vifnames)
	{
		memset(nvram_name, 0, sizeof(nvram_name));
		snprintf(nvram_name, sizeof(nvram_name), "wgn_br%d_wl_ifnames", br_idx);
		nvram_set(nvram_name, wl_vifnames);
	}

	// nvram_set lanX_ifname
	memset(nvram_name, 0, sizeof(nvram_name));
	snprintf(nvram_name, sizeof(nvram_name), "lan%d_ifname", br_idx);
	nvram_set(nvram_name, br_ifname);
	// nvram_set lanX_ifnames
	memset(nvram_name, 0, sizeof(nvram_name));
	snprintf(nvram_name, sizeof(nvram_name), "lan%d_ifnames", br_idx);
	nvram_set(nvram_name, nvram_val); 
	// nvram_set brX_ifname
	memset(nvram_name, 0, sizeof(nvram_name));
	snprintf(nvram_name, sizeof(nvram_name), "br%d_ifname", br_idx);
	nvram_set(nvram_name, br_ifname);
	// nvram_set brX_ifnames
	memset(nvram_name, 0, sizeof(nvram_name));
	snprintf(nvram_name, sizeof(nvram_name), "br%d_ifnames", br_idx);
	nvram_set(nvram_name, nvram_val);

	return 1;

create_guest_bridge_failed:
	destory_vlan();
	wgn_sysdep_swtich_unset(vlan_id);

	// remove bridge
	exec_cmd("brctl", "delbr", br_ifname);

	// nvram_unset lanX_ifname
	memset(nvram_name, 0, sizeof(nvram_name));
	snprintf(nvram_name, sizeof(nvram_name), "lan%d_ifname", br_idx);
	nvram_unset(nvram_name);
	// nvram_unset lanX_ifnames
	memset(nvram_name, 0, sizeof(nvram_name));
	snprintf(nvram_name, sizeof(nvram_name), "lan%d_ifnames", br_idx);
	nvram_unset(nvram_name); 
	// nvram_unset brX_ifname
	memset(nvram_name, 0, sizeof(nvram_name));
	snprintf(nvram_name, sizeof(nvram_name), "br%d_ifname", br_idx);
	nvram_unset(nvram_name);
	// nvram_unset brX_ifnames
	memset(nvram_name, 0, sizeof(nvram_name));
	snprintf(nvram_name, sizeof(nvram_name), "br%d_ifnames", br_idx);
	nvram_unset(nvram_name);	
	
	return 0;
}

extern 
void wgn_start(
	void)
{
	char wgn_ifnames[(WGN_MAXINUM_VLAN_RULELIST * IFNAMSIZ) + 1];
	char wl_ifnames[(WGN_MAXINUM_VLAN_RULELIST * IFNAMSIZ) + 1];
	char wl_ifname[33];
	char *ipaddr = NULL;
	char *ipmask = NULL;

	size_t i;
	int unit;
	int subunit;

	struct brif_rule_t brif_list[WGN_MAXINUM_VLAN_RULELIST];
	size_t brif_total = 0;
	struct wgn_vlan_rule_t vlan_list[WGN_MAXINUM_VLAN_RULELIST];
	struct wgn_vlan_rule_t *p_vlan_rule = NULL;
	size_t vlan_total = 0;
	struct wgn_subnet_rule_t subnet_list[WGN_MAXINUM_VLAN_RULELIST];
	struct wgn_subnet_rule_t *p_subnet_rule = NULL;
	size_t subnet_total = 0;

	memset(wgn_ifnames, 0, sizeof(wgn_ifnames));
	memset(wl_ifnames, 0, sizeof(wl_ifnames));

	if (!IS_CAP() && !IS_RE())
		return;
	
	if (IS_RE())
		wgn_check_avalible_brif();

	memset(brif_list, 0, sizeof(struct brif_rule_t) * WGN_MAXINUM_VLAN_RULELIST);
	if (!brif_list_get_from_nvram(brif_list, WGN_MAXINUM_VLAN_RULELIST, &brif_total))
		return;

	if (brif_total <= 0)
		return;

	memset(vlan_list, 0, sizeof(struct wgn_vlan_rule_t) * WGN_MAXINUM_VLAN_RULELIST);
	if (!wgn_vlan_list_get_from_nvram(vlan_list, WGN_MAXINUM_VLAN_RULELIST, &vlan_total))
		return;

	if (vlan_total <= 0)
		return;

	memset(subnet_list, 0, sizeof(struct wgn_subnet_rule_t) * WGN_MAXINUM_VLAN_RULELIST);
	if (!wgn_subnet_list_get_from_nvram(subnet_list, WGN_MAXINUM_VLAN_RULELIST, &subnet_total))
		return;

	if (subnet_total <= 0)
		return;

	for (i=0; i<brif_total; i++)
	{
		if (!(p_vlan_rule = wgn_vlan_list_find(vlan_list, vlan_total, brif_list[i].subnet_name)))
			continue;

		if (p_vlan_rule->vid == -1)
			continue;

		if (!p_vlan_rule->enable)
			continue;

		wgn_get_wl_unit(&unit,&subunit,p_vlan_rule);
		if (unit < 0 || subunit <= 0)
			continue;

		if (!get_wl_bss_enabled(unit, subunit)) 
			continue;

		if (get_wl_lanaccess(unit, subunit))
			continue;

		memset(wl_ifname, 0, sizeof(wl_ifname));
		if (!get_wl_ifname(unit, subunit, wl_ifname, sizeof(wl_ifname)-1))
			continue;
		
		if (p_vlan_rule->vid <= 0)
			continue;

		ipaddr = ipmask = NULL;
		if (!IS_RE())
		{
			if (!(p_subnet_rule = wgn_subnet_list_find(subnet_list, subnet_total, p_vlan_rule->subnet_name)))
				continue;

			if (!(ipaddr = p_subnet_rule->ipaddr))
				continue;

			if (!(ipmask = p_subnet_rule->ipmask))
				continue;

		}

		if (!create_guest_bridge(brif_list[i].br_name, ipaddr, ipmask, wl_ifname, p_vlan_rule->vid))
			continue;

		// add wl_ifname
		strlcat(wl_ifnames, wl_ifname, sizeof(wl_ifnames));
		strlcat(wl_ifnames, " ", sizeof(wl_ifnames));
		// add br_ifname
		strlcat(wgn_ifnames, brif_list[i].br_name, sizeof(wgn_ifnames));
		strlcat(wgn_ifnames, " ", sizeof(wgn_ifnames));
	}

	if (strlen(wl_ifnames) > 0 && strlen(wgn_ifnames) > 0)
	{
		update_br0_ifnames(wl_ifnames);
		update_lan_ifnames(wl_ifnames);
		nvram_set(WGN_IFNAMES, wgn_ifnames);
		nvram_set_int(WGN_ENABLED, 1);		
	}

	return;
}

extern 
int wgn_is_enabled(
	void)
{
	return (int) nvram_get_int(WGN_ENABLED);
}

extern
char* wgn_guest_lan_ipaddr(
	const char *guest_wlif, 
	char *result, 
	size_t result_bsize)
{
	char s[81];
	char *wgn_ifnames = NULL, *lan_ifnames = NULL;
	char word[64], *next = NULL;
	char word1[64], *next1 = NULL;

	int if_idx = 0, found = 0;
	struct brif_rule_t brif_list[WGN_MAXINUM_VLAN_RULELIST];
	struct brif_rule_t *p_brif_rule = NULL;
	size_t brif_total = 0;
	struct wgn_subnet_rule_t subnet_list[WGN_MAXINUM_SUBNET_RULELIST];
	struct wgn_subnet_rule_t *p_subnet_rule = NULL;
	size_t subnet_total = 0;

	if (!IS_CAP() && !IS_RE())
		return NULL;

	if (!result || result_bsize <= 0)
		return NULL;

	if (!(wgn_ifnames = nvram_get(WGN_IFNAMES)))
		return NULL;

	memset(brif_list, 0, sizeof(struct brif_rule_t) * WGN_MAXINUM_VLAN_RULELIST);
	if (!brif_list_get_from_nvram(brif_list, WGN_MAXINUM_VLAN_RULELIST, &brif_total))
		return NULL;

	if (brif_total <= 0)
		return NULL;

	memset(subnet_list, 0, sizeof(struct wgn_subnet_rule_t) * WGN_MAXINUM_SUBNET_RULELIST);
	if (!wgn_subnet_list_get_from_nvram(subnet_list, WGN_MAXINUM_SUBNET_RULELIST, &subnet_total))
		return NULL;

	if (subnet_total <= 0)
		return NULL;

	foreach (word, wgn_ifnames, next)
	{
		if (sscanf(word, "br%d", &if_idx) != 1)
			continue;

		memset(s, 0, sizeof(s));
		snprintf(s, sizeof(s)-1, "lan%d_ifnames", if_idx);
		if (!(lan_ifnames = nvram_get(s)))
			continue;

		foreach(word1, lan_ifnames, next1)
		{
			if ((found = !strncmp(word1, guest_wlif, strlen(guest_wlif))))
				break;
		}

		if (found)
			break;
	}

	if (found && (p_brif_rule = brif_list_find(brif_list, brif_total, word)))
	{
		found = 0;
		if ((p_subnet_rule = wgn_subnet_list_find(subnet_list, subnet_total, p_brif_rule->subnet_name)))
		{
			found = 1;
			strlcpy(result, p_subnet_rule->ipaddr, result_bsize);
		}
	}

	return (found) ? result : NULL;
}

extern
char* wgn_guest_lan_netmask(
	const char *guest_wlif, 
	char *result, 
	size_t result_bsize)
{
	char s[81];
	char *wgn_ifnames = NULL, *lan_ifnames = NULL;
	char word[64], *next = NULL;
	char word1[64], *next1 = NULL;

	int if_idx = 0, found = 0;
	struct brif_rule_t brif_list[WGN_MAXINUM_VLAN_RULELIST];
	struct brif_rule_t *p_brif_rule = NULL;
	size_t brif_total = 0;
	struct wgn_subnet_rule_t subnet_list[WGN_MAXINUM_SUBNET_RULELIST];
	struct wgn_subnet_rule_t *p_subnet_rule = NULL;
	size_t subnet_total = 0;

	if (!IS_CAP() && !IS_RE())
		return NULL;

	if (!result || result_bsize <= 0)
		return NULL;

	if (!(wgn_ifnames = nvram_get(WGN_IFNAMES)))
		return NULL;

	memset(brif_list, 0, sizeof(struct brif_rule_t) * WGN_MAXINUM_VLAN_RULELIST);
	if (!brif_list_get_from_nvram(brif_list, WGN_MAXINUM_VLAN_RULELIST, &brif_total))
		return NULL;

	if (brif_total <= 0)
		return NULL;

	memset(subnet_list, 0, sizeof(struct wgn_subnet_rule_t) * WGN_MAXINUM_SUBNET_RULELIST);
	if (!wgn_subnet_list_get_from_nvram(subnet_list, WGN_MAXINUM_SUBNET_RULELIST, &subnet_total))
		return NULL;

	if (subnet_total <= 0)
		return NULL;

	foreach (word, wgn_ifnames, next)
	{
		if (sscanf(word, "br%d", &if_idx) != 1)
			continue;

		memset(s, 0, sizeof(s));
		snprintf(s, sizeof(s)-1, "lan%d_ifnames", if_idx);
		if (!(lan_ifnames = nvram_get(s)))
			continue;

		foreach(word1, lan_ifnames, next1)
		{
			if ((found = !strncmp(word1, guest_wlif, strlen(guest_wlif))))
				break;
		}

		if (found)
			break;
	}

	if (found && (p_brif_rule = brif_list_find(brif_list, brif_total, word)))
	{
		found = 0;
		if ((p_subnet_rule = wgn_subnet_list_find(subnet_list, subnet_total, p_brif_rule->subnet_name)))
		{
			found = 1;
			strlcpy(result, p_subnet_rule->ipmask, result_bsize);
		}
	}

	return (found) ? result : NULL;
}

extern 
char* wgn_guest_lan_ifnames(
	char *ret_ifnames, 
	size_t ret_ifnames_bsize)
{
	char s[81], *ss = NULL;
	char *wgn_ifnames = NULL;
	char word[64], *next = NULL;
	int ifidx = 0;

	if (!IS_CAP() && !IS_RE())
		return NULL;

	if (!ret_ifnames || ret_ifnames_bsize <= 0)
		return NULL;

	memset(ret_ifnames, 0, ret_ifnames_bsize);
	if (!(wgn_ifnames = nvram_get(WGN_IFNAMES)))
		return NULL;

	foreach (word, wgn_ifnames, next)
	{
		if (sscanf(word, "br%d", &ifidx) != 1)
			continue;

		memset(s, 0, sizeof(s));
		snprintf(s, sizeof(s), "lan%d_ifnames", ifidx);
		ss = nvram_get(s);
		if (ss && strlen(ss) > 0)
		{
			strlcat(ret_ifnames, ss, ret_ifnames_bsize);
			if (ss[strlen(ss)-1] != ' ') strlcat(ret_ifnames, " ", ret_ifnames_bsize);
		}
	}

	return (strlen(ret_ifnames) > 0) ? ret_ifnames : NULL;	
}

extern
char* wgn_all_lan_ifnames(
	void)
{
	char s[81], *ss = NULL;
	char *b = NULL;
	char *wgn_ifnames = NULL;
	char word[64], *next = NULL;
	int if_idx = 0;
	size_t alloc_size = 256 * WLIFU_MAX_NO_BRIDGE;

	if (!(b = (char *)malloc(alloc_size)))
		return NULL;

	memset(b, 0, alloc_size);
	ss = nvram_get("lan_ifnames");
	if (ss && strlen(ss) > 0)
	{
		strlcat(b, ss, alloc_size);
		if (ss[strlen(ss)-1] != ' ') strlcat(b, " ", alloc_size);
	}

	if (IS_CAP())
	{
		if ((wgn_ifnames = nvram_get(WGN_IFNAMES)) && strlen(wgn_ifnames) > 0)
		{
			foreach (word, wgn_ifnames, next)
			{
				if (sscanf(word, "br%d", &if_idx) != 1)
					continue;

				memset(s, 0, sizeof(s));
				snprintf(s, sizeof(s)-1, "lan%d_ifnames", if_idx);
				if ((ss = nvram_get(s)) && strlen(ss) > 0)
				{
					strlcat(b, ss, alloc_size);
					if (ss[strlen(ss)-1] != ' ') strlcat(b, " ", alloc_size);
				}
			}
		}
	}

	if (strlen(b) > 0)
	{
		b[strlen(b)-1] = '\0';
	}
	else
	{
		free(b);
		b = NULL;
	}

	return b;
}

extern
void wgn_stop(
	void)
{
	char s[81];
	char *wgn_ifnames = NULL;
	char word[64], *next = NULL;
	struct brif_rule_t *p_brif_rule = NULL;
	struct brif_rule_t brif_list[WGN_MAXINUM_VLAN_RULELIST];
	size_t brif_total = 0;
	struct wgn_vlan_rule_t *p_vlan_rule = NULL;
	struct wgn_vlan_rule_t vlan_list[WGN_MAXINUM_VLAN_RULELIST];
	size_t vlan_total = 0;
	int ifidx = 0;
	
	if (!(wgn_ifnames = nvram_get(WGN_IFNAMES)))
		return;

	memset(brif_list, 0, sizeof(struct brif_rule_t) * WGN_MAXINUM_VLAN_RULELIST);
	if (!brif_list_get_from_nvram(brif_list, WGN_MAXINUM_VLAN_RULELIST, &brif_total))
		return;

	if (brif_total <= 0)
		return;

	memset(vlan_list, 0, sizeof(struct wgn_vlan_rule_t) * WGN_MAXINUM_VLAN_RULELIST);
	if (!wgn_vlan_list_get_from_nvram(vlan_list, WGN_MAXINUM_VLAN_RULELIST, &vlan_total))
		return;

	if (vlan_total <= 0)
		return;

	destory_vlan();
	foreach (word, wgn_ifnames, next)
	{
		if (!(p_brif_rule = brif_list_find(brif_list, brif_total, word)))
			continue;

		if (!(p_vlan_rule = wgn_vlan_list_find(vlan_list, vlan_total, p_brif_rule->subnet_name)))
			continue;

		if (sscanf(word, "br%d", &ifidx) != 1)
			continue;


#ifdef RTCONFIG_EMF
#if defined(HND_ROUTER) && defined(RTCONFIG_PROXYSTA)

	eval("bcmmcastctl", "mode", "-i",  word,  "-p", "1",  "-m", "0");
	eval("bcmmcastctl", "mode", "-i",  word,  "-p", "2",  "-m", "0");

#endif

	/* Stop the EMF for this LAN */
	eval("emf", "stop", word);
	/* Remove Bridge from igs */
	eval("igs", "del", "bridge", word);
	eval("emf", "del", "bridge", word);

#endif	/* RTCONFIG_EMF */

		// unset switch port
		wgn_sysdep_swtich_unset(p_vlan_rule->vid);
		// ifconfig down brX
		ifconfig(word, 0, NULL, NULL);
		// brctl delbr brX
		exec_cmd("brctl", "delbr", word);

		// nvram_unset wgn_lanX_ifname
		memset(s, 0, sizeof(s));
		snprintf(s, sizeof(s)-1, "lan%d_ifname", ifidx);
		nvram_unset(s);
		// nvram_unset wgn_lanX_ifnames
		memset(s, 0, sizeof(s));
		snprintf(s, sizeof(s)-1, "lan%d_ifnames", ifidx);
		nvram_unset(s);
		// nvram_unset wgn_brX_ifname
		memset(s, 0, sizeof(s));
		snprintf(s, sizeof(s)-1, "br%d_ifname", ifidx);
		nvram_unset(s);
		// nvram_unset wgn_brX_ifnames
		memset(s, 0, sizeof(s));
		snprintf(s, sizeof(s)-1, "br%d_ifnames", ifidx);
		nvram_unset(s);		
	}

	nvram_unset(WGN_IFNAMES);
	nvram_set_int(WGN_ENABLED, 0);
	return;
}

