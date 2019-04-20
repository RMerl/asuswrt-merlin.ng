 /*
 * Copyright 2018, ASUSTeK Inc.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. ASUS
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */

/*
	amas_lib is for mac / ip transfer in AiMesh environment
*/

#include "amas_lib.h"

#ifdef RTCONFIG_SW_HW_AUTH
#include <auth_common.h>
#define APP_ID    "33716237"
#define APP_KEY   "g2hkhuig238789ajkhc"
#endif

/* global variables */
#define MAX_DEV_LIST 256
#define AMAS_ENTRY_FILE_LOCK   "amas_entry_list"
#define AMAS_NODE_FILE_LOCK    "amas_node_list"
struct list *amas_entry_list = NULL;
unsigned int amas_count = 0;
int is_static_scan = 0;
struct list *amas_node_list = NULL;
unsigned int amas_node_count = 0;

/* struct for RE node : amas_node_list */
typedef struct __amas_node_list__t_
{
	char mac[18];
} AMAS_NODE_LIST;

#if defined(RTCONFIG_BWDPI)
static void mesh_set_user_common(char *mac, char *ip, int action1, int action2)
{
	char *str1 = NULL, *p = NULL;
	char *str2 = NULL, *q = NULL;

	if (action1 == ACT_NA && action2 == ACT_NA) AMASLIB_DBG("[NA] existed or do nothing, mac=%s\n", mac);

	if (action1 != ACT_NA) {
		if (action1 == ACT_ADD) AMASLIB_DBG("[ADD] mac=%s, ip=%s\n", mac, ip);
		if (action1 == ACT_UPDATE) AMASLIB_DBG("[UPDATE] mac=%s, ip=%s\n", mac, ip);
		if (action1 == ACT_DELETE) AMASLIB_DBG("[DELETE] mac=%s, ip=%s\n", mac, ip);

		str1 = p = strdup(mac);
		str2 = q = strdup(ip);
		mesh_set_user(str1, str2, action1);

		if (p) free(p);
		if (q) free(q);
	}

	if (action2 != ACT_NA) {
		if (action2 == ACT_ADD) AMASLIB_DBG("[ADD] mac=%s, ip=%s\n", mac, ip);
		if (action2 == ACT_UPDATE) AMASLIB_DBG("[UPDATE] mac=%s, ip=%s\n", mac, ip);
		if (action2 == ACT_DELETE) AMASLIB_DBG("[DELETE] mac=%s, ip=%s\n", mac, ip);

		str1 = p = strdup(mac);
		str2 = q = strdup(ip);
		mesh_set_user(str1, str2, action2);

		if (p) free(p);
		if (q) free(q);
	}
}

static void setup_re_device_into_tdts(char *mac)
{
	AMASLIB_DHCP_T *mylist = NULL;
	struct listnode *ln = NULL;
	char macbuf[18] = {0};
	char ipbuf[16] = {0};
	int is_new = 1;
	int is_update = 0;

	int ret = 0;
	int lock;
	unsigned int buf_pos, buf_used_len;
	int i = 0;
	int action1 = ACT_NA;
	int action2 = ACT_NA;
	char *buf = NULL;
	mesh_user_ioc_list_t *tbl = NULL;
	mesh_user_ioc_entry_t *entry = NULL;

	lock = file_lock(AMAS_ENTRY_FILE_LOCK);
	LIST_LOOP(amas_entry_list, mylist, ln)
	{
		if (strcasecmp(mylist->mac, mac) != 0) continue;

		if (ret = get_fw_mesh_user((void **) &buf, &buf_used_len))
		{
			printf("Fail to get_fw_meash_user\n");
			goto __ret;
		}

		if (!buf || !buf_used_len)
		{
			AMASLIB_DBG(" There are 0 mesh user in SHN control\n");
			goto __ret;
		}

		buf_pos = 0;
		tbl = (mesh_user_ioc_list_t *)buf;

		if (!IOC_SHIFT_LEN_SAFE(buf_pos, sizeof(mesh_user_ioc_list_t), buf_used_len))
		{
			goto __ret;
		}

		for (i = 0; i < tbl->entry_cnt; i++)
		{
			entry = (mesh_user_ioc_entry_t *)(buf + buf_pos);
			if (!IOC_SHIFT_LEN_SAFE(buf_pos, sizeof(mesh_user_ioc_entry_t), buf_used_len))
			{
				goto __ret;
			}
			memset(macbuf, 0, sizeof(macbuf));
			memset(ipbuf, 0, sizeof(ipbuf));
			snprintf(macbuf, sizeof(macbuf), ""MAC_OCTET_FMT"", MAC_OCTET_EXPAND(entry->mac));
			snprintf(ipbuf, sizeof(ipbuf), ""IPV4_OCTET_FMT"", IPV4_OCTET_EXPAND(entry->ipv4));

			// tdts module exists this mac
			if (strcasecmp(macbuf, mylist->mac) == 0) {
				if (strcasecmp(ipbuf, mylist->ip) == 0) {
					is_new = 0;
					is_update = 0;
					break;
				}
				else {
					is_new = 0;
					is_update = 1;
					break;
				}
			}
		}

		// check status
		if (is_new == 0 && is_update == 1) {
			action1 = ACT_UPDATE;
			action2 = ACT_NA;
		}
		else if (is_new == 1 && is_update == 0) {
			action1 = ACT_UPDATE;
			action2 = ACT_ADD;
		}
		else {
			action1 = ACT_NA;
			action2 = ACT_NA;
		}

		// use ioctol to setup
		mesh_set_user_common(mylist->mac, mylist->ip, action1, action2);

		// reset flag
		is_new = 1;
		is_update = 0;
		ret = 0;
	}

__ret:
	if (buf) free(buf);
	file_unlock(lock);
}

static void check_tdts_amaslib_setting()
{
	char *enable = NULL, *mac = NULL;
	char *p = NULL, *g = NULL, *t = NULL;
	char *a = NULL, *b = NULL, *c = NULL, *d = NULL;

	g = t = strdup(nvram_safe_get("wrs_rulelist"));
	while (g) {
		if ((p = strsep(&g, "<")) == NULL) break;
		if ((vstrsep(p, ">", &enable, &mac, &a, &b, &c, &d)) != 6) continue;
		if (!strcmp(enable, "0")) continue;
		setup_re_device_into_tdts(mac);
	}
	if (t) free(t);
}

/*
	return value
	0 : this device doesn't exist in NODE
	1 : this device exists in NODE
	2 : wrong format
*/
static int check_re_node_exist_in_tdts(char *mac)
{
	int ret = 0;
	int val = 0;
	char macbuf[18] = {0};
	unsigned int buf_pos, buf_used_len;
	int i;
	char *buf = NULL;
	mesh_ext_ioc_list_t *tbl = NULL;
	mesh_ext_ioc_entry_t *entry = NULL;

	if (mac == NULL || !strcmp(mac, "")) return 2;

	if (ret = get_fw_mesh_extender((void **) &buf, &buf_used_len))
	{
		printf("Fail to get_fw_meash_extender\n");
		goto __ret;
	}

	if (!buf || !buf_used_len)
	{
		AMASLIB_DBG(" There are 0 mesh node in SHN control\n");
		goto __ret;
	}

	buf_pos = 0;
	tbl = (mesh_ext_ioc_list_t *)buf;

	if (!IOC_SHIFT_LEN_SAFE(buf_pos, sizeof(mesh_ext_ioc_list_t), buf_used_len))
	{
		goto __ret;
	}

	for (i = 0; i < tbl->entry_cnt; i++)
	{
		entry = (mesh_ext_ioc_entry_t *)(buf + buf_pos);
		if (!IOC_SHIFT_LEN_SAFE(buf_pos, sizeof(mesh_ext_ioc_entry_t), buf_used_len))
		{
			goto __ret;
		}
		memset(macbuf, 0, sizeof(macbuf));
		snprintf(macbuf, sizeof(macbuf), ""MAC_OCTET_FMT"", MAC_OCTET_EXPAND(entry->mac));

		// mesh node exists in the table
		if (strcasecmp(macbuf, mac) == 0) {
			val = 1;
			break;
		}
	}

	ret = 0;
__ret:
	if (buf) free(buf);
	return val;
}

static void setup_re_node_into_tdts(char *mac)
{
	int action = ACT_NA;
	char *str = NULL, *p = NULL;

	if (check_re_node_exist_in_tdts(mac) == 0) {
		AMASLIB_DBG("[ADD RE] mac=%s\n", mac);
		action = ACT_ADD;

		// use new pointer to write ioctl
		str = p = strdup(mac);
		mesh_set_extender(str, action); 
		if (p) free(p);
	}
}
#endif

static void clear_amas_entry_list_flag()
{
	int lock;
	AMASLIB_DHCP_T *mylist = NULL;
	struct listnode *ln = NULL;

	lock = file_lock(AMAS_ENTRY_FILE_LOCK);
	LIST_LOOP(amas_entry_list, mylist, ln) {
		AMASLIB_DBG(" amas_entry_list, mac=%s, ip=%s, flag=%d.%s\n", 
			mylist->mac, mylist->ip, mylist->flag, ((mylist->flag > 0) ? " <<< need to be cleared" : ""));
		mylist->flag = AMASLIB_DHCP_FLAG_NO_CHANGED;
	}

	file_unlock(lock);
}

static int check_time_sched_amaslib_setting()
{
	char *buf;
	char addr[4096], *next_word;
	int lock;
	AMASLIB_DHCP_T *mylist = NULL;
	struct listnode *ln = NULL;
	int is_checked = 0;

	buf = nvram_safe_get("MULTIFILTER_MAC");

	lock = file_lock(AMAS_ENTRY_FILE_LOCK);
	foreach_62(addr, buf, next_word){
		LIST_LOOP(amas_entry_list, mylist, ln)
		{
			if (strcasecmp(mylist->mac, addr) == 0 && mylist->flag > 0)
			{
				is_checked = 1;
				goto CHECK_DONE;
			}
		}
	}
CHECK_DONE:
	file_unlock(lock);
	return is_checked;
}

static int check_TQoS_amaslib_setting()
{
	char *buf, *g, *p;
	char *desc, *addr, *port, *prio, *transferred, *proto;
	int lock;
	AMASLIB_DHCP_T *mylist = NULL;
	struct listnode *ln = NULL;
	int is_checked = 0;

	g = buf = strdup(nvram_safe_get("qos_rulelist"));

	lock = file_lock(AMAS_ENTRY_FILE_LOCK);
	while (g) {
		if ((p = strsep(&g, "<")) == NULL) break;
		if ((vstrsep(p, ">", &desc, &addr, &port, &proto, &transferred, &prio)) != 6) continue;
		LIST_LOOP(amas_entry_list, mylist, ln)
		{
			if (strcasecmp(mylist->mac, addr) == 0 && mylist->flag > 0)
			{
				is_checked = 1;
				goto CHECK_DONE;
			}
		}
	}
CHECK_DONE:
	file_unlock(lock);
	return is_checked;
}

static int check_BW_limiter_amaslib_setting()
{
	char *buf, *g, *p;
	char *enable, *addr, *dlc, *upc, *prio;
	int lock;
	AMASLIB_DHCP_T *mylist = NULL;
	struct listnode *ln = NULL;
	int is_checked = 0;

	g = buf = strdup(nvram_safe_get("qos_bw_rulelist"));
	lock = file_lock(AMAS_ENTRY_FILE_LOCK);
	while (g) {
		if ((p = strsep(&g, "<")) == NULL) break;
		if ((vstrsep(p, ">", &enable, &addr, &dlc, &upc, &prio)) != 5) continue;
		if (!strcmp(enable, "0")) continue;
		LIST_LOOP(amas_entry_list, mylist, ln)
		{
			if (strcasecmp(mylist->mac, addr) == 0 && mylist->flag > 0)
			{
				is_checked = 1;
				goto CHECK_DONE;
			}
		}
	}
CHECK_DONE:
	file_unlock(lock);
	return is_checked;
}

static void setup_amaslib_related_fun()
{
	#define FLAG_RESTART_QOS 1
	#define FLAG_RESTART_FIREWALL 2

	int restart_flag = 0;
	int f_qo = 0;
	int f_fw = 0;

#if defined(RTCONFIG_BWDPI)
	// wrs & apps filters
	if (nvram_get_int("wrs_enable") && nvram_get_int("wrs_app_enable")) {
		check_tdts_amaslib_setting();
	}
#endif

	// time-scheduling
	if (nvram_get_int("MULTIFILTER_ALL")) {
		if (check_time_sched_amaslib_setting()) {
			restart_flag |= FLAG_RESTART_FIREWALL;
		}
	}

	// T.QoS
	if (nvram_get_int("qos_enable") && nvram_get_int("qos_type") == 0) {
		if (check_TQoS_amaslib_setting()) {
			restart_flag |= FLAG_RESTART_QOS;
		}
	}

	// bandwidth limiter
	if (nvram_get_int("qos_enable") && nvram_get_int("qos_type") == 2) {
		if (check_BW_limiter_amaslib_setting()) {
			restart_flag |= FLAG_RESTART_QOS;
		}
	}

	// get rc_service flag
	f_qo = nvram_get_int("restart_qo");
	f_fw = nvram_get_int("restart_fwl");
	AMASLIB_DBG(" restart_flag=%d, f_qo=%d, f_fw=%d\n", restart_flag, f_qo, f_fw);

	if ((restart_flag & FLAG_RESTART_QOS) && f_qo == 0) {
		notify_rc("restart_qos;restart_firewall");
	} else if ((restart_flag & FLAG_RESTART_FIREWALL) && f_fw == 0) {
		notify_rc("restart_firewall");
	}

	// clear all flag in amas_entry_list
	clear_amas_entry_list_flag();
}

/*
	return value
	0 : don't exists in amas etry list
	1 : exists in amas etry list
*/
static int get_ip_from_amas_entry_list(char *mac, char *ip)
{
	int ret = 0;
	int lock;
	AMASLIB_DHCP_T *mylist = NULL;
	struct listnode *ln = NULL;

	lock = file_lock(AMAS_ENTRY_FILE_LOCK);
	LIST_LOOP(amas_entry_list, mylist, ln)
	{
		if (strcasecmp(mylist->mac, mac) == 0) {
			ret = 1;
			memcpy(ip, mylist->ip, sizeof(mylist->ip));
			break;
		}
	}
	file_unlock(lock);

	AMASLIB_DBG(" mac=%s, ip=%s, ret=%d\n", mac, ip, ret);
	return ret;
}

/*
	return value
	0 : don't exists in amas etry list
	1 : already exists in amas etry list
	2 : already exists in amas etry list and IP changed, update IP automatically
*/
static int check_amas_entry_list(char *mac, char *ip)
{
	int ret = 0;
	int lock;
	AMASLIB_DHCP_T *mylist = NULL;
	struct listnode *ln = NULL;

	lock = file_lock(AMAS_ENTRY_FILE_LOCK);
	LIST_LOOP(amas_entry_list, mylist, ln)
	{
		if (strcasecmp(mylist->mac, mac) == 0) {
			ret = 1;
			if (strcmp(mylist->ip, ip) != 0) {
				// update IP automatically
				memcpy(mylist->ip, ip, sizeof(mylist->ip));
				mylist->flag = AMASLIB_DHCP_FLAG_IP_CHANGED;
				ret = 2;
			}
			AMASLIB_DBG(" mymac=%s, myip=%s, mac=%s, ip=%s, ret=%d\n", mylist->mac, mylist->ip, mac, ip, ret);
			break;
		}
	}
	file_unlock(lock);

	//AMASLIB_DBG(" mac=%s, ip=%s, ret=%d\n", mac, ip, ret);
	return ret;
}

/*
	return value
	0 : can't find the device in dnsmasq.leases
	1 : can find the device in dnsmasq.leases
*/
static int check_dhcp_lease_table(char *mac, char *ip)
{
	int ret = 0;
	FILE *fp = NULL;
	char buf[256] = {0};
	char macbuf[18] = {0};
	char ipbuf[16] = {0};

	if ((fp = fopen(DHCP_TABLE, "r")) != NULL)
	{
		while (fgets(buf, sizeof(buf), fp))
		{
			memset(macbuf, 0, sizeof(macbuf));
			memset(ipbuf, 0, sizeof(ipbuf));
			if (sscanf(buf, "%*s %s %s %*s %*s", macbuf, ipbuf) != 2) continue;
			if (strcasecmp(mac, macbuf) == 0 && strcasecmp(ip, ipbuf) == 0) {
				ret = 1;
				break;
			}
		}
		if (fp) fclose(fp);
	}
	else {
		printf("FAIL to open %s\n", DHCP_TABLE);
	}

	AMASLIB_DBG(" mac=%s, ip=%s, macbuf=%s, ipbuf=%s, ret=%d\n", mac, ip, macbuf, ipbuf, ret);
	return ret;
}

/*
	return value
	0 : can't find the device in arp table
	1 : can find the device in arp table, and the mac is correct
	2 : can find the device in arp table, but the mac is incorrect
*/
static int check_arp_entry_table(char *mac, char *ip)
{
	int ret = 0;
	FILE *fp = NULL;
	char buf[256] = {0};
	char macbuf[18] = {0};
	char ipbuf[16] = {0};
	int n = 0;

OPEN_ARP_FILE:

	if ((fp = fopen(ARP_TABLE, "r")) != NULL)
	{
		n++;

		// skip first row
		fgets(buf, sizeof(buf), fp);

		// while loop
		while (fgets(buf, sizeof(buf), fp))
		{
			memset(macbuf, 0, sizeof(macbuf));
			memset(ipbuf, 0, sizeof(ipbuf));
			if (sscanf(buf, "%15s %*s %*s %s %*s %*s", ipbuf, macbuf) != 2) continue;
			if (!strcmp(ipbuf, ip)) {
				if (!strcasecmp(macbuf, mac)) {
					ret = 1;
				}
				else if (strcasecmp(macbuf, mac) && strcasecmp(macbuf, "00:00:00:00:00:00")) {
					ret = 2;
				} else if(!strcasecmp(macbuf, "00:00:00:00:00:00")) { // if mac is empty, retry it.
					if (fp) fclose(fp);
					sleep(1);
					AMASLIB_DBG(" IP matched but mac is empty, retry to read arp table!!\n");
					if (n > 9)
						goto END;
					else
						goto OPEN_ARP_FILE;
				}
				break;
			}
		}

		if (fp) fclose(fp);
	}
	else {
		printf("FAIL to open %s\n", ARP_TABLE);
	}
END:
	AMASLIB_DBG(" mac=%s, ip=%s, macbuf=%s, ipbuf=%s, ret=%d\n", mac, ip, macbuf, ipbuf, ret);
	return ret;
}

static void add_amas_entry_list(char *mac, char *ip)
{
	int lock = file_lock(AMAS_ENTRY_FILE_LOCK);
	// check the length of linklist
	if ((amas_count > MAX_DEV_LIST) && amas_entry_list)
	{
		list_delete_all_node(amas_entry_list);
		amas_count = 0;
		AMASLIB_DBG(" The linklist is over 256 entries, free all\n");
	}

	// add new node into linklist
	AMASLIB_DHCP_T *mylist = NULL;
	mylist = (AMASLIB_DHCP_T *)malloc(sizeof(AMASLIB_DHCP_T));
	memcpy(mylist->mac, mac, sizeof(mylist->mac));
	memcpy(mylist->ip, ip, sizeof(mylist->ip));
	mylist->flag = AMASLIB_DHCP_FLAG_NEW_DATA;
	listnode_add(amas_entry_list, (void*)mylist);
	amas_count++;

	AMASLIB_DBG(" mac=%s, ip=%s, amas_count=%d\n", mac, ip, amas_count);
	file_unlock(lock);
}

static void is_re_device_dhcp(char *mac, char *ip)
{
	int is_check = 0;

	// check whether this device exists in amas_entry_list
	is_check = check_amas_entry_list(mac, ip);
	if (is_check == 1 || is_check == 2) return;

	// check dhcp and arp table, and then add into amas_entry_list
	if ((check_dhcp_lease_table(mac, ip) == 1) && (check_arp_entry_table(mac, ip) == 2)) {
		add_amas_entry_list(mac, ip);
	}
}

static void is_re_device_full(char *mac, char *ip)
{
	int is_check = 0;

	// check whether this device exists in amas_entry_list
	is_check = check_amas_entry_list(mac, ip);
	if (is_check == 1 || is_check == 2) return;

	add_amas_entry_list(mac, ip);
}

static void fullscan_amas_re_device()
{
	int lock;
	json_object *clietListObj = NULL;
	json_object *brMacObj = NULL;
	json_object *clientObj = NULL;
	json_object *infoObj = NULL;
	char mac[18] = {0};
	char ip[16] = {0};

	lock = file_lock(CLIENTLIST_FILE_LOCK);
	clietListObj = json_object_from_file(CLIENT_LIST_JSON_PATH);
	if (clietListObj) {
		json_object_object_foreach(clietListObj, key, val) {
			brMacObj = val;
			//AMASLIB_DBG(" 1. clientListObj key=%s\n", key);
			json_object_object_foreach(brMacObj, key, val) {
				clientObj = val;
				//AMASLIB_DBG(" 2. brMacObj key=%s\n", key);
				json_object_object_foreach(clientObj, key, val) {
					infoObj = val;
					//AMASLIB_DBG(" 3. clientObj key=%s\n", key);
					memset(mac, 0, sizeof(mac));
					snprintf(mac, sizeof(mac), "%s", key);
					json_object_object_foreach(infoObj, key, val) {
						if (!strcmp(key, "ip")) {
							memset(ip, 0, sizeof(ip));
							snprintf(ip, sizeof(ip), "%s", json_object_get_string(val));
							if (strcmp(ip, "")) {
								AMASLIB_DBG(" mac=%s, ip=%s\n", mac, ip);
								is_re_device_full(mac, ip);
							}
						}
					}
				}
			}
		}
	}
	json_object_put(clietListObj);
	file_unlock(lock);
}

static void trigger_full_scan(int type)
{
	if (is_static_scan == 1 && type == 0) {
		AMASLIB_DBG(" static_full_scan is running!\n");
		return;
	}
	else if (is_static_scan == 0 && type == 1) {
		AMASLIB_DBG(" no such case\n");
		return;
	}

	/* full scan amas /tmp/clientlist.json to find re device */
	fullscan_amas_re_device();

	/* setup IP / mac transfer logic for amas */
	setup_amaslib_related_fun();
}

static void amaslib_acitve_mode(int type)
{
	if (type == 0) {
		trigger_full_scan(0);
	}
	else if (type == 1) {
		// delay 30 sec to trigger full scan
		if (getpid() == pthread_self()) // Check whether it's main thread.
			alarm(30);
		else {
			sleep(30);
			kill(getpid(), SIGALRM);
		}
	}
}

static void amaslib_passive_mode(char *mac, char *ip)
{
	is_re_device_dhcp(mac, ip);
}

static int check_amaslib_RE_node(char *mac)
{
	AMAS_NODE_LIST *mylist = NULL;
	struct listnode *ln = NULL;
	int ret = 0;
	int lock = file_lock(AMAS_NODE_FILE_LOCK);

	LIST_LOOP(amas_node_list, mylist, ln)
	{
		if (strcasecmp(mylist->mac, mac) == 0) {
			ret = 1;
			break;
		}
	}
	file_unlock(lock);

	return ret;
}

static void amaslib_get_RE_mode()
{
	AMAS_NODE_LIST *mylist = NULL;
	struct listnode *ln = NULL;
	int lock = file_lock(AMAS_NODE_FILE_LOCK);

	LIST_LOOP(amas_node_list, mylist, ln)
	{
		// debug only
		if (nvram_get_int("amaslib_re_debug")) {
			AMASLIB_DBG(" mylist->mac=%s\n", mylist->mac);
		}

#if defined(RTCONFIG_BWDPI)
		/* if no dpi engine, stop to setup_re_node_into_tdts */
		if (check_bwdpi_nvram_setting() == 0) {
			file_unlock(lock);
			return;
		}

		setup_re_node_into_tdts(mylist->mac);
#endif
	}
	file_unlock(lock);
}

static void restore_amaslib_RE_node(char *mac)
{
	// check whether RE node exists in linklist
	if (check_amaslib_RE_node(mac) == 1) return;
	
	int lock = file_lock(AMAS_NODE_FILE_LOCK);

	// check the length of linklist
	if ((amas_node_count > MAX_DEV_LIST) && amas_node_list)
	{
		list_delete_all_node(amas_node_list);
		amas_node_count = 0;
		AMASLIB_DBG(" The linklist is over 256 entries, free all\n");
	}

	// add new node into linklist
	AMAS_NODE_LIST *mylist = NULL;
	mylist = (AMAS_NODE_LIST *)malloc(sizeof(AMAS_NODE_LIST));
	memcpy(mylist->mac, mac, sizeof(mylist->mac));
	listnode_add(amas_node_list, (void*)mylist);
	amas_node_count++;

	AMASLIB_DBG(" mac=%s, amas_node_count=%d\n", mac, amas_node_count);
	file_unlock(lock);
}

static void amaslib_scan_mode(AMASLIB_EVENT_T event)
{
	AMASLIB_DBG(" sta2g=%s, sta5g=%s, flag=%d\n", event.sta2g, event.sta5g, event.flag);
	if (event.flag == 0) {
		// active scan, trigger from manual configuration
		amaslib_get_RE_mode();
		amaslib_acitve_mode(0);
	}
	else if (event.flag == 1) {
		// passive scan, trigger from cfg_server
		restore_amaslib_RE_node(event.sta2g);
		restore_amaslib_RE_node(event.sta5g);
		amaslib_get_RE_mode();
	}
	else if (event.flag == 2) {
		// get whole RE node
		amaslib_get_RE_mode();

		// passive scan, trigger from dhcp-script (DCHP)
		amaslib_passive_mode(event.sta2g, event.sta5g);

		/* setup IP / mac transfer logic for amas */
		setup_amaslib_related_fun();
	}
	else if (event.flag == 3) {
		// get whole RE node
		amaslib_get_RE_mode();

		// active scan, trigger from arp-script (static)
		amaslib_acitve_mode(1);
	}
}

static void handlesignal(int sig)
{
	if (sig == SIGTERM) {
		AMASLIB_DBG(" receive SIGTERM\n");
		remove(AMASLIB_PID_PATH);

		int lock = file_lock(AMAS_ENTRY_FILE_LOCK);

		/* destroy global linklist */
		if (amas_entry_list) list_delete(amas_entry_list);
		if (amas_node_list)  list_delete(amas_node_list);

		file_unlock(lock);

		exit(0);
	}
	else if (sig == SIGALRM) {
		AMASLIB_DBG(" receive SIGALRM\n");
		is_static_scan = 1;
	}
	else if (sig == SIGUSR1) {
		AMASLIB_DBG(" receive SIGUSR1\n");
		trigger_full_scan(0);
	}
	else if (sig == SIGUSR2) {
		int i = 0;
		int lock;
		AMASLIB_DHCP_T *mylist = NULL;
		struct listnode *ln = NULL;
		AMASLIB_DBG(" receive SIGUSR2\n");
		AMASLIB_DBG(" amas_count=%d\n", amas_count);
		lock = file_lock(AMAS_ENTRY_FILE_LOCK);
		LIST_LOOP(amas_entry_list, mylist, ln)
		{
			AMASLIB_DBG(" amas_entry_list index=%d, mac=%s, ip=%s, flag=%d\n", i, mylist->mac, mylist->ip, mylist->flag);
			i++;
		}
		file_unlock(lock);
		i = 0;
		AMASLIB_DBG(" amas_node_count=%d\n", amas_node_count);
		lock = file_lock(AMAS_NODE_FILE_LOCK);
		LIST_LOOP(amas_node_list, mylist, ln)
		{
			AMASLIB_DBG(" amas_node_list index=%d, mac=%s\n", i, mylist->mac);
			i++;
		}
		file_unlock(lock);
	}
	else {
		AMASLIB_DBG(" Unknown SIGNAL or No defined\n");
	}
}

static void receive_s(int newsockfd)
{
	int n = 0;
	AMASLIB_EVENT_T event;

	memset(&event, 0, sizeof(AMASLIB_EVENT_T));

	n = read(newsockfd, &event, sizeof(AMASLIB_EVENT_T));
	if (n < 0) {
		printf("ERROR reading from socket.\n");
		return;
	}

	switch(event.action) {
		case AMASLIB_ACTION_NODE_MAC_UPDATE:
			AMASLIB_DBG(" Get AMASLIB_ACTION_NODE_MAC_UPDATE\n");
			amaslib_scan_mode(event);
			break;
		case AMASLIB_ACTION_DEVICE_IP_QUERY:
			get_ip_from_amas_entry_list(event.sta2g, event.ip);
			AMASLIB_DBG(" Get AMASLIB_ACTION_DEVICE_IP_QUERY, MAC=%s, IP=%s\n", event.sta2g, event.ip);
			n = write(newsockfd, &event, sizeof(AMASLIB_EVENT_T));
			if (n < 0) {
				printf("ERROR writing to socket.\n");
				return;
			}
			break;
	}
}

void event_handler(void* data)
{
	int newsockfd = *((int *)data);
	
	pthread_detach(pthread_self());
	receive_s(newsockfd);
	close(newsockfd);
	AMASLIB_DBG(" close sockfd ...\n");
}

static int event_handler_threading(int newsockfd)
{
	int status = -1;
	pthread_attr_t attr;
	pthread_t tid;

	pthread_attr_init(&attr);
#ifdef PTHREAD_STACK_SIZE
	pthread_attr_setstacksize(&attr, PTHREAD_STACK_SIZE);
#endif
	if(status = pthread_create(&tid, &attr, (void *)&event_handler, &newsockfd)){
		status =  -1;
		pthread_attr_destroy(&attr);
		return status;
	}

	status = 0;
	pthread_attr_destroy(&attr);
	return status;
}

static int amas_lib_send_event(AMASLIB_EVENT_T *event, int wait_reuslt)
{
	int fd = -1;
	int length = 0;
	int ret = 0;
	struct sockaddr_un addr;
	int flags;
	int status;
	socklen_t statusLen;
	fd_set writeFds, readFds;
	int selectRet;
	struct timeval timeout = {2, 0};

	if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		printf("ipc socket error!\n");
		goto err;
	}

	/* set NONBLOCK for connect() */
	if ((flags = fcntl(fd, F_GETFL)) < 0) {
		printf("F_GETFL error!\n");
		goto err;
	}

	flags |= O_NONBLOCK;

	if (fcntl(fd, F_SETFL, flags) < 0) {
		printf("F_SETFL error!\n");
		goto err;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, AMASLIB_SOCKET_PATH, sizeof(addr.sun_path)-1);
	if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		if (errno == EINPROGRESS) {
			FD_ZERO(&writeFds);
			FD_SET(fd, &writeFds);

			selectRet = select(fd + 1, NULL, &writeFds, NULL, &timeout);

			//Check return, -1 is error, 0 is timeout
			if (selectRet == -1 || selectRet == 0) {
				printf("ipc connect error\n");
				goto err;
			}
		}
		else
		{
			printf("ipc connect error\n");
			goto err;
		}
	}

	/* check the status of connect() */
	status = 0;
	statusLen = sizeof(status);
	if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &status, &statusLen) == -1) {
		printf("getsockopt(SO_ERROR): %s\n", strerror(errno));
		goto err;
	}

	length = write(fd, (AMASLIB_EVENT_T *)event, sizeof(AMASLIB_EVENT_T));

	if (length < 0) {
		printf("error writing:%s\n", strerror(errno));
		goto err;
	}

	if (wait_reuslt) {
		int retry_cnt = 0;
		while (retry_cnt < 5) {  // retry 5 times
			FD_ZERO(&readFds);
			FD_SET(fd, &readFds);
			timeout.tv_sec = 1;
			timeout.tv_usec = 0;
			selectRet = select(fd + 1, &readFds, NULL, NULL, &timeout);
			//Check return, -1 is error, 0 is timeout
			if (selectRet == -1)  {
				AMASLIB_DBG("ipc read error\n");
				goto err;
			} else if (selectRet == 0) {
				retry_cnt++;
				AMASLIB_DBG("ipc read timeout, retry=%d\n", retry_cnt);
				continue;
			}

			length = read(fd, (AMASLIB_EVENT_T *)event, sizeof(AMASLIB_EVENT_T));

			if (length < 0) {
				AMASLIB_DBG("error read:%s\n", strerror(errno));
				if (errno != EAGAIN)
					goto err;
			} else {
				AMASLIB_DBG("read:length=%d, ip=%s\n", length, event->ip);
			}
			break;
		}
	}

	ret = 1;

err:
	if (fd >= 0)
        close(fd);

	AMASLIB_DBG(" leave, ret=%d\n", ret);
	return ret;
} /* End of amas_lib_send_event */

/*
========================================================================
Routine Description:
	Send event to amas_lib and get result.

Arguments:
        event		- event info.

Return Value:
	0		- fail
	1		- success

========================================================================
*/
int amas_lib_device_ip_query(char *mac, char *ip)
{
	int ret = 0;
	AMASLIB_EVENT_T *event = NULL;

	// Workaround for ipv6 case.
#ifdef RTCONFIG_IPV6
	if(ipv6_enabled())
		return ret;
#endif

#if defined(RTCONFIG_WIFI_SON)
        if (nvram_match("wifison_ready", "1"))
                return ret;
#endif

	if (!mac)
		return ret;

	if ((event = (AMASLIB_EVENT_T *)malloc(sizeof(AMASLIB_EVENT_T))) == NULL) {
		printf("Fail to malloc event structure\n");
		return ret;
	}

	memset(event, 0, sizeof(AMASLIB_EVENT_T));
	event->action = AMASLIB_ACTION_DEVICE_IP_QUERY;

	strlcpy(event->sta2g, mac, sizeof(event->sta2g));

	/* send event to amas lib */
	amas_lib_send_event(event, 1);
	if (ip && strlen(event->ip)) {
		strlcpy(ip, event->ip, sizeof(event->ip));
		AMASLIB_DBG(" ip=%s\n", event->ip);
		ret = 1;
	}

	free(event);

	return ret;
} /* End of amas_lib_device_ip_query */

int amas_lib_main(int argc, char **argv)
{
	FILE *fp = NULL;
	struct sigaction sa;
	struct sockaddr_un addr;
	int sockfd, newsockfd;
	int flags;
	fd_set readFds;
	int selectRet;
	struct timeval timeout = {2, 0};

	AMASLIB_DBG(" amas_lib is starting ...\n");

#if defined(RTCONFIG_WIFI_SON)
        if (nvram_match("wifison_ready", "1"))
                return 0;
#endif

#ifdef RTCONFIG_SW_HW_AUTH
	time_t timestamp = time(NULL);
	char in_buf[48];
	char out_buf[65];
	char hw_out_buf[65];
	char *hw_auth_code = NULL;

	// initial
	memset(in_buf, 0, sizeof(in_buf));
	memset(out_buf, 0, sizeof(out_buf));
	memset(hw_out_buf, 0, sizeof(hw_out_buf));

	// use timestamp + APP_KEY to get auth_code
	snprintf(in_buf, sizeof(in_buf)-1, "%ld|%s", timestamp, APP_KEY);

	hw_auth_code = hw_auth_check(APP_ID, get_auth_code(in_buf, out_buf, sizeof(out_buf)), timestamp, hw_out_buf, sizeof(hw_out_buf));

	// use timestamp + APP_KEY + APP_ID to get auth_code
	snprintf(in_buf, sizeof(in_buf)-1, "%ld|%s|%s", timestamp, APP_KEY, APP_ID);

	// if check fail, return
	if (strcmp(hw_auth_code, get_auth_code(in_buf, out_buf, sizeof(out_buf))) == 0) {
		AMASLIB_DBG(" This is ASUS router\n");
	}
	else {
		AMASLIB_DBG(" This is not ASUS router\n");
		return 0;
	}
#endif

	/* write pid */
	if ((fp = fopen(AMASLIB_PID_PATH, "w")) != NULL) {
		fprintf(fp, "%d", getpid());
		if (fp) fclose(fp);
	}
	else {
		printf("Fail to write amas_lib.pid");
		exit(-1);
	}

	/* Signal */
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = &handlesignal;
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGALRM, &sa, NULL);
	sigaction(SIGUSR1, &sa, NULL); // Trigger full scan, just for test.
	sigaction(SIGUSR2, &sa, NULL); // Print all list, just for test.

	/* create linked list */
	amas_entry_list = list_new();
	amas_node_list = list_new();

	/* start unix socket */
	if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		printf("socket error");
		exit(-1);
	}

	/* set NONBLOCK for connect() */
	if ((flags = fcntl(sockfd, F_GETFL)) < 0) {
		printf("F_GETFL error!\n");
		exit(-1);
	}

	flags |= O_NONBLOCK;

	if (fcntl(sockfd, F_SETFL, flags) < 0) {
		printf("F_SETFL error!\n");
		exit(-1);
	}

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strlcpy(addr.sun_path, AMASLIB_SOCKET_PATH, sizeof(addr.sun_path));
	unlink(AMASLIB_SOCKET_PATH);

	if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		printf("socket bind error");
		exit(-1);
	}
	
	if (listen(sockfd, MAX_AMASLIB_SOCKET_CLIENT) == -1) {
		printf("listen error");
		exit(-1);
	}

	while (1)
	{
		/* handle static scan */
		if (is_static_scan) {
			trigger_full_scan(1);
			is_static_scan = 0;
		}

		FD_ZERO(&readFds);
		FD_SET(sockfd, &readFds);
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		selectRet = select(sockfd + 1, &readFds, NULL, NULL, &timeout);
		//Check return, -1 is error, 0 is timeout
		if (selectRet == -1)  {
			printf("select error %s\n", strerror(errno));
			if (errno == EINTR) continue;
			return 0;
		} else if (selectRet == 0) {
			continue;
		}

		if ((newsockfd = accept(sockfd, NULL, NULL)) == -1) {
			if (errno == EINTR) continue;
			printf("accept error");
			continue;
		}

		/* use thread to receive socket information */
		event_handler_threading(newsockfd);
	}

	return 0;
}

void stop_amas_lib()
{
	kill_pidfile_s(AMASLIB_PID_PATH, SIGTERM);
}

void start_amas_lib()
{
	char *cmd[] = {"amas_lib", NULL};
	pid_t pid;

#if defined(RTCONFIG_WIFI_SON)
        if (nvram_match("wifison_ready", "1"))
                return;
#endif

	if (repeater_mode() || mediabridge_mode())
		return;

#if defined(RTCONFIG_BCMWL6) && defined(RTCONFIG_PROXYSTA)
	if (psr_mode())
		return;
#endif

#if defined(RTCONFIG_AMAS) && defined(RTCONFIG_DPSTA)
	if (dpsta_mode() && !nvram_get_int("re_mode") && nvram_get_int("x_Setting"))
		return;
#endif

	if (!pids("amas_lib")) {
		_eval(cmd, NULL, 0, &pid);
	}
}

int amaslib_lease_main(int argc, char **argv)
{
#if defined(RTCONFIG_WIFI_SON)
        if (nvram_match("wifison_ready", "1"))
                return -1;
#endif

	if (argc < 4)
		return -1;

	_dprintf("%s():: %s, %s, %s, %s\n", __FUNCTION__, argv[1], argv[2], argv[3], argv[4] ? : "No hostname");

	if (!pids("amas_lib")) {
		//_dprintf("fail to find amas_lib\n");
		return -1;
	}

	if (!strcmp(argv[1], "add") || !strcmp(argv[1], "old")) {
		// DCHP
		AMAS_EVENT_TRIGGER(argv[2], argv[3], 2);
	}
	else if (!strcmp(argv[1], "arp-add") || !strcmp(argv[1], "arp-old")) {
		// static
		AMAS_EVENT_TRIGGER(argv[2], argv[3], 3);
	}

	_dprintf("%s():: done\n", __FUNCTION__);
	return 0;
}

void amaslib_check()
{
	start_amas_lib();
}
