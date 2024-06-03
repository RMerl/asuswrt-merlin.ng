#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include <rc.h>
#include <shared.h>
#include <llist.h>
#include <multi_wan.h>
#include <det_dns.h>

#define DEFAULT_INTERVAL	5

extern int stop;

void mtwan_set_detect_routing_rule(const char *dest, const char *ifname, int add);
static int _dns_probe(const int wan_unit, const char *host, const int timeout, const char *content, char *ip, const size_t ip_sz);

typedef struct _check_wan
{
	int wan_unit;
	long last_check_tm;
	int fail_cnt;
	int success_cnt;
}CHECK_WAN;

LList *chk_wan_lst = NULL;

static int _chk_wan_list_search_func(void* data, void* compare_data)
{
	int *wan_unit;
	CHECK_WAN *chk_wan;

	if(data && compare_data)
	{
		chk_wan = data;
		wan_unit = compare_data;
		if(chk_wan->wan_unit == *wan_unit)
			return 0;
	}
	return -1;
}

static void _chk_wan_list_free_func(void *data)
{
	CHECK_WAN *chk_wan;

	if(data)
	{
		chk_wan = data;
		SAFE_FREE(chk_wan);
	}
}

static void _chk_wan_dump_data_func(const int index, void *data)
{
	CHECK_WAN *chk_wan;

	if(data)
	{
		chk_wan = data;
		_dprintf("index:%d  wan_unit:%d  last_check_tm:%d  fail_cnt:%d  success_cnt:%d\n", 	\
			index, chk_wan->wan_unit, chk_wan->last_check_tm, chk_wan->fail_cnt, chk_wan->success_cnt);
	}
}

static int _ping_check(const int wan_unit)
{
	char target[128] = {0};
	char wan_prefix[8] = {0};
	FILE *fp;
	char cmd[512];
	int count = 0, timeout;
	int ret = -1;
	char tmp[256];
	char ifname[IF_NAMESIZE], ip[32];

	snprintf(wan_prefix, sizeof(wan_prefix), "wan%d_", wan_unit);
	strlcpy(target, nvram_safe_get(strlcat_r(wan_prefix, "wandog_target", tmp, sizeof(tmp))), sizeof(target));
	mtwan_get_ifname(wan_unit, ifname, sizeof(ifname));
	timeout = nvram_pf_get_int(wan_prefix, "wandog_interval");

	//check FQDN or IP
	if(!validate_ip(target))
	{
		if(_dns_probe(wan_unit, target, timeout, NULL, ip, sizeof(ip)) == -1)
		{
			_dprintf("[%s]Wrong ping target.\n", __FUNCTION__);
			return -1;
		}
	}
	else
		strlcpy(ip, target, sizeof(ip));

	mtwan_set_detect_routing_rule(ip, ifname, 1);
	//ping
	snprintf(cmd, sizeof(cmd), "ping -c1 -w2 -s32 -Mdont '%s' 2>&1", ip);
	if ((fp = popen(cmd, "r")) != NULL)
	{
		while (fgets(cmd, sizeof(cmd), fp) != NULL)
		{
			if (sscanf(cmd, "%*s %*s transmitted, %d %*s received", &count) == 1)
			{
				ret = (count > 0)? 0: -1;
				break;
			}
			else
				ret = -1;
		}
		pclose(fp);
	}
	mtwan_set_detect_routing_rule(ip, ifname, 0);

	return ret;
}

static int _dns_probe(const int wan_unit, const char *host, const int timeout, const char *content, char *ip, const size_t ip_sz)
{
	char wan_prefix[] = "wanXXXX_";
	char ifname[IF_NAMESIZE];
	char dns[2][16];
	int dns_cnt, i, ret = -1, flag;

	if(!host || !ip )
		return ret;

	snprintf(wan_prefix, sizeof(wan_prefix), "wan%d_", wan_unit);
	dns_cnt = mtwan_get_dns(wan_unit, dns[0], sizeof(dns[0]), dns[1], sizeof(dns[1]));
	mtwan_get_ifname(wan_unit, ifname, sizeof(ifname));

	for(i = 0; i < dns_cnt; ++i)
	{
		mtwan_set_detect_routing_rule(dns[i], ifname, 1);
		flag = get_host_by_name((unsigned char*)host, T_A, ifname, dns[i], timeout > 0? timeout: DEFAULT_INTERVAL, ip, ip_sz);
		mtwan_set_detect_routing_rule(dns[i], ifname, 0);
		if(!flag)
		{
			if(!content || content[0] == '*' || !strcmp(ip, content))
			{
				ret = 0;
				break;
			}
		}
	}
	return ret;
}

static int _dns_check(const int wan_unit)
{
	char wan_prefix[8] = {0}, tmp[256];
	char host[PATH_MAX], content[PATH_MAX];
	char ip[16];
	int ret = 0, timeout;

	snprintf(wan_prefix, sizeof(wan_prefix), "wan%d_", wan_unit);
	snprintf(host, sizeof(host), "%s", nvram_safe_get(strlcat_r(wan_prefix, "dns_probe_host", tmp, sizeof(tmp))));
	snprintf(content, sizeof(content), "%s", nvram_safe_get(strlcat_r(wan_prefix, "dns_probe_content", tmp, sizeof(tmp))));

	if (*host == '\0')
	{
		snprintf(host, sizeof(host), "%s", nvram_default_get("dns_probe_host"));
		snprintf(content, sizeof(content), "%s", nvram_default_get("dns_probe_content"));
	}
	if (*content == '\0')
	{
		strlcpy(content, "*", sizeof(content));
	}

	/* Check for valid domain to avoid shell escaping */
	if (!is_valid_domainname(host) || *content == '\0')
	{
		_dprintf("[%s] Invalid hostname\n", __FUNCTION__);
		return -1;
	}

	timeout = atoi(nvram_safe_get(strlcat_r(wan_prefix, "dns_probe_timeout", tmp, sizeof(tmp))));
	if(!timeout)
		timeout = DEFAULT_INTERVAL;

	ret = _dns_probe(wan_unit, host, timeout, content, ip, sizeof(ip));

	return ret;
}

static int _set_link_internet(const int wan_unit, const int link_internet, int *success_cnt, int *fail_cnt)
{
	char name[128];

#ifdef RTCONFIG_MULTIWAN_PROFILE
	if (wan_unit != WAN_UNIT_NONE)
#else
	if(is_mtwan_unit(wan_unit))
#endif
	{
		snprintf(name, sizeof(name), "wan%d_link_internet", wan_unit);
		nvram_set_int(name, link_internet);
		if(link_internet == 2)
		{
			if(success_cnt)
				(*success_cnt) ++;
			if(fail_cnt)
				(*fail_cnt) = 0;
		}
		else
		{
			if(success_cnt)
				(*success_cnt) =0;
			if(fail_cnt)
				(*fail_cnt) ++;
		}
		return 0;
	}
	return -1;
}

void  *mtwan_detect_internet(void *arg)
{
	int i, real_unit, ping_ret, dns_ret, default_route_ret, conn_flag;
	int wan_enable, dns_check, wandog_enable, wandog_interval;
	long cur_tm;
	char prefix[] = "wanXXXX_", tmp[256];
	LList *lst;
	CHECK_WAN *chk_wan;

	_dprintf("[%s]Start!\n", __FUNCTION__);
	while(!stop)
	{
		for(i = 0; i < MAX_MULTI_WAN_NUM; ++i)
		{
			cur_tm = uptime();
			real_unit = mtwan_get_real_wan(i + MULTI_WAN_START_IDX, prefix, sizeof(prefix));
			lst = llist_find(chk_wan_lst, (void*)(&real_unit), _chk_wan_list_search_func);
			wan_enable = nvram_get_int(strlcat_r(prefix, "enable", tmp, sizeof(tmp)));
			dns_check  = nvram_get_int(strlcat_r(prefix, "dns_probe", tmp, sizeof(tmp)));
			wandog_enable = nvram_get_int(strlcat_r(prefix, "wandog_enable", tmp, sizeof(tmp)));
			wandog_interval =  nvram_get_int(strlcat_r(prefix, "wandog_interval", tmp, sizeof(tmp)));
			chk_wan = NULL;				

			if( !wan_enable	//remove disabled chk_wan record
#if !defined(RTCONFIG_MULTIWAN_PROFILE)
			 || !is_mtwan_unit(real_unit)
#endif
			){
				if(lst)
				{
					chk_wan_lst = llist_free_1(chk_wan_lst, lst, _chk_wan_list_free_func);
				}
				continue;
			}

			if(!wandog_interval)
			{
				wandog_interval = DEFAULT_INTERVAL;
			}
			//_dprintf("[%s, %d]unit:%d, prefix:%s, wan_enable:%d, dns_probe:%d, wandog_enable:%d, wandog_interval:%d\n", 
			//	__FUNCTION__, __LINE__, real_unit, prefix, wan_enable, dns_probe, wandog_enable, wandog_interval);

			if(lst)
			{
				chk_wan = lst->data;
			}
			else// if(dns_probe == 1 || wandog_enable == 1)	//add wan which need to detect internet
			{
				chk_wan = calloc(1, sizeof(CHECK_WAN));
				if(chk_wan)
				{
					chk_wan->wan_unit = real_unit;
					chk_wan_lst = llist_append(chk_wan_lst, chk_wan);
				}
			}

			ping_ret = 0;
			dns_ret = 0;

			if(get_wan_state(real_unit) != WAN_STATE_CONNECTED)
			{
				_set_link_internet(real_unit, 1, &(chk_wan->success_cnt), &(chk_wan->fail_cnt));
			}			
			else if (mtwanduck_get_phy_status(i + MULTI_WAN_START_IDX) == 0)
			{
				_set_link_internet(real_unit, 0, &(chk_wan->success_cnt), &(chk_wan->fail_cnt));
			}
			else if(chk_wan && (cur_tm - chk_wan->last_check_tm >= wandog_interval))
			{
				default_route_ret = found_default_route(real_unit);

				if(wandog_enable == 1)
				{
					//_dprintf("[%s, %d]do _ping_check\n", __FUNCTION__, __LINE__);
					ping_ret = _ping_check(real_unit);
				}		
				if(dns_check == 1)
				{
					//_dprintf("[%s, %d]do _dns_check\n", __FUNCTION__, __LINE__);
					dns_ret = _dns_check(real_unit);
				}
				chk_wan->last_check_tm = cur_tm;
				conn_flag = 1;
				if(default_route_ret)
				{
					if(wandog_enable && dns_check && !ping_ret && !dns_ret)
					{
						conn_flag = 2;
					}
					else if(wandog_enable && !ping_ret && !dns_check)
					{
						conn_flag = 2;
					}
					else if(dns_check && !dns_ret && !wandog_enable)
					{
						conn_flag = 2;
					}
					else if(!wandog_enable && !dns_check)
					{
						conn_flag = 2;
					}
				}
				_set_link_internet(real_unit, conn_flag, &(chk_wan->success_cnt), &(chk_wan->fail_cnt));
			}
		}
		//llist_dump_data(chk_wan_lst, _chk_wan_dump_data_func);
		sleep(1);
	}
	llist_free_all(chk_wan_lst, _chk_wan_list_free_func);
	pthread_exit(NULL);
}

void mtwan_set_detect_routing_rule(const char *dest, const char *ifname, int add)
{
	char table[8], cmd[512];

	if(dest && ifname)
	{
		mtwan_get_route_table_id(mtwan_ifunit(ifname), table, sizeof(table));
		snprintf(cmd, sizeof(cmd), "ip rule %s iif lo to %s table %s pref %d", add? "add": "del", dest, table, IP_RULE_PREF_DET_INTERNET);
		system(cmd);
	}
}
