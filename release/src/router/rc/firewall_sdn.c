#include <rc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <shared.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <net/if.h>
#ifdef RTCONFIG_MULTIWAN_IF
#include "multi_wan.h"
#endif

extern const char URL_FILTER_INPUT_CHAIN[];
extern const char URL_FILTER_FORWARD_CHAIN[];
extern const char NW_SERVICE_FILTER_FORWARD_CHAIN[];

/**************************************************************************************************************************************************
 *  Common functions
 **************************************************************************************************************************************************/
extern int addr_type_parse(const char *src, char *dst, int size);
extern char *protoflag_conv(char *proto, char *buf, int isFlag);
extern char *iprange_ex_conv(char *ip, char *buf);
extern int ipt_addr_compact(const char *s, int af, int strict);
extern char *str2time(char *str, char *buf, int buf_size, int flag);

const char filter_header[] = "*filter\n";
const char nat_header[] = "*nat\n";
const char mangle_header[] = "*mangle\n";
const char commit_str[] = "COMMIT\n";

#define STOP_TIME 0
#define START_TIME 1

struct datetime
{
	char start[7];	 // start time
	char stop[7];	 // stop time
	char tmpstop[7]; // cross-night stop time
} __attribute__((packed));

typedef struct _tagllist
{
	void *data;
	struct _tagllist *prev;
	struct _tagllist *next;
} llist;

int append_llist(llist **lst, void *data)
{
	llist *end, *tmp;

	if (!data || !lst)
		return -1;

	tmp = calloc(1, sizeof(llist));
	if (!tmp)
		return -1;
	tmp->data = data;

	if (!(*lst))
	{
		(*lst) = tmp;
	}
	else
	{
		end = (*lst);
		while (end->next)
			end = end->next;
		end->next = tmp;
		tmp->prev = end;
	}
	return 0;
}

typedef void (*func_free_data)(void **);

void free_llist(llist **lst, func_free_data callback)
{
	llist *tmp;

	if (!lst || !callback)
		return;

	while (*lst)
	{
		if ((*lst)->data)
		{
			callback(&((*lst)->data));
		}
		tmp = *lst;
		*lst = tmp->next;
		SAFE_FREE(tmp);
	}
}

int get_drop_accept(char *logdrop, const size_t logdrop_len, char *logaccept, const size_t logaccept_len)
{
	if (!logdrop || !logaccept)
		return -1;

	/* Determine the log type */
	if (nvram_match("fw_log_x", "accept") || nvram_match("fw_log_x", "both"))
		strlcpy(logaccept, "logaccept", logaccept_len);
	else
		strlcpy(logaccept, "ACCEPT", logaccept_len);

	if (nvram_match("fw_log_x", "drop") || nvram_match("fw_log_x", "both"))
		strlcpy(logdrop, "logdrop", logdrop_len);
	else
		strlcpy(logdrop, "DROP", logdrop_len);
	return 0;
}

int create_iptables_file(const int type, const int sdn_idx, FILE **fp, char *file_path, const size_t path_len, const int isv6)
{
	if (!fp || !file_path)
		return -1;

	switch (type)
	{
	case IPTABLE_TYPE_FILTER:
	{
		if (isv6)
			snprintf(file_path, path_len, "%s/filterv6_sdn%d", sdn_dir, sdn_idx);
		else
			snprintf(file_path, path_len, "%s/filter_sdn%d", sdn_dir, sdn_idx);

		*fp = fopen(file_path, "w");
		if (*fp)
		{
			fprintf(*fp, filter_header);
			return 0;
		}
	}
	break;
	case IPTABLE_TYPE_NAT:
	{
		if (isv6)
			snprintf(file_path, path_len, "%s/natv6_sdn%d", sdn_dir, sdn_idx);
		else
			snprintf(file_path, path_len, "%s/nat_sdn%d", sdn_dir, sdn_idx);

		*fp = fopen(file_path, "w");
		if (*fp)
		{
			fprintf(*fp, nat_header);
			return 0;
		}
	}
	break;
	case IPTABLE_TYPE_MANGLE:
	{
		if (isv6)
			snprintf(file_path, path_len, "%s/manglev6_sdn%d", sdn_dir, sdn_idx);
		else
			snprintf(file_path, path_len, "%s/mangle_sdn%d", sdn_dir, sdn_idx);

		*fp = fopen(file_path, "w");
		if (*fp)
		{
			fprintf(*fp, mangle_header);
			return 0;
		}
	}
	break;
	default:
	{
		_dprintf("[%s]Unsupport type(%d)\n", __FUNCTION__, type);
	}
	break;
	}
	return -1;
}

int close_n_restore_iptables_file(const int type, FILE **fp, const char *file_path, const int isv6)
{
	if (!fp || !(*fp) || !file_path)
		return -1;

	fprintf(*fp, commit_str);
	fclose(*fp);
	*fp = NULL;

	if ((type == IPTABLE_TYPE_FILTER && (f_size(file_path) > strlen(filter_header) + strlen(commit_str))) ||
		(type == IPTABLE_TYPE_NAT && (f_size(file_path) > strlen(nat_header) + strlen(commit_str))) ||
		(type == IPTABLE_TYPE_MANGLE && (f_size(file_path) > strlen(mangle_header) + strlen(commit_str))))
	{
		if (isv6)
			eval("ip6tables-restore", "--noflush", (char *)file_path);
		else
			eval("iptables-restore", "--noflush", (char *)file_path);
	}
	return 0;
}

static int _remove_iptables_chain(const char *chain, const int v6)
{
	char cmd[32], buf[512], tmp[128], *ptr;
	FILE *fp;

	if (!chain)
		return -1;

	if (v6)
		strlcpy(cmd, "ip6tables", sizeof(cmd));
	else
		strlcpy(cmd, "iptables", sizeof(cmd));

	// remove all jump rules
	doSystem("%s-save | grep %s > /tmp/.sdn/rm_chain", cmd, chain);
	fp = fopen("/tmp/.sdn/rm_chain", "r");
	if (fp)
	{
		snprintf(tmp, sizeof(tmp), "-j %s", chain);
		while (fgets(buf, sizeof(buf), fp))
		{
			if (strstr(buf, tmp))
			{
				ptr = strstr(buf, "-A");
				if (!ptr)
					ptr = strstr(buf, "-I");

				if (ptr)
				{
					*(ptr + 1) = 'D';
					doSystem("%s %s", cmd, buf);
				}
			}
		}
		fclose(fp);
	}
	// unlink("/tmp/.sdn/rm_chain");

	// clean chain
	doSystem("%s -F %s 2>/dev/null", cmd, chain);

	// delete chain
	doSystem("%s -X %s 2>/dev/null", cmd, chain);

	return 0;
}

int reset_sdn_firewall(const unsigned long features, const int sdn_idx)
{
	char sdn_chain[32];

	if (features & SDN_FEATURE_URL_FILTER)
	{
		snprintf(sdn_chain, sizeof(sdn_chain), "%s_%d", URL_FILTER_INPUT_CHAIN, sdn_idx);
		_remove_iptables_chain(sdn_chain, 0);
#ifdef RTCONFIG_IPV6
		_remove_iptables_chain(sdn_chain, 1);
#endif
		snprintf(sdn_chain, sizeof(sdn_chain), "%s_%d", URL_FILTER_FORWARD_CHAIN, sdn_idx);
		_remove_iptables_chain(sdn_chain, 0);
#ifdef RTCONFIG_IPV6
		_remove_iptables_chain(sdn_chain, 1);
#endif
	}
	if (features & SDN_FEATURE_NWS_FILTER)
	{
		snprintf(sdn_chain, sizeof(sdn_chain), "%s_%d", NW_SERVICE_FILTER_FORWARD_CHAIN, sdn_idx);
		_remove_iptables_chain(sdn_chain, 0);
#ifdef RTCONFIG_IPV6
		_remove_iptables_chain(sdn_chain, 1);
#endif
	}
	return 0;
}

extern const char SDN_FILTER_INPUT_CHAIN[];
extern const char SDN_FILTER_FORWARD_CHAIN[];
extern const char SDN_NAT_POSTROUTING[];

static void _add_sdn_iptables_wan_rules(FILE* fp, const char* wan_ifname, const MTLAN_T *pmtl, const char *logdrop, const char *logaccept)
{
	// SDN to WAN
	fprintf(fp, "-I %s -i %s -o %s -j %s\n", SDN_FILTER_FORWARD_CHAIN, pmtl->nw_t.ifname, wan_ifname, logaccept);

	// Passthrough
	if (nvram_match("fw_pt_pptp", "0")) {
		fprintf(fp, "-I %s -i %s -o %s -p tcp --dport %d -j %s\n", SDN_FILTER_FORWARD_CHAIN, pmtl->nw_t.ifname, wan_ifname, 1723, logdrop);
		fprintf(fp, "-I %s -i %s -o %s -p 47 -j %s\n", SDN_FILTER_FORWARD_CHAIN, pmtl->nw_t.ifname, wan_ifname, logdrop);
	}
	if (nvram_match("fw_pt_l2tp", "0"))
		fprintf(fp, "-I %s -i %s -o %s -p udp --dport %d -j %s\n", SDN_FILTER_FORWARD_CHAIN, pmtl->nw_t.ifname, wan_ifname, 1701, logdrop);
	if (nvram_match("fw_pt_ipsec", "0")) {
		fprintf(fp, "-I %s -i %s -o %s -p udp --dport %d -j %s\n", SDN_FILTER_FORWARD_CHAIN, pmtl->nw_t.ifname, wan_ifname, 500, logdrop);
		fprintf(fp, "-I %s -i %s -o %s -p udp --dport %d -j %s\n", SDN_FILTER_FORWARD_CHAIN, pmtl->nw_t.ifname, wan_ifname, 4500, logdrop);
		fprintf(fp, "-I %s -i %s -o %s -p 50 -j %s\n", SDN_FILTER_FORWARD_CHAIN, pmtl->nw_t.ifname, wan_ifname, logdrop);
		fprintf(fp, "-I %s -i %s -o %s -p 51 -j %s\n", SDN_FILTER_FORWARD_CHAIN, pmtl->nw_t.ifname, wan_ifname, logdrop);
	}
}

int update_SDN_iptables(const MTLAN_T *pmtl, const char *logdrop, const char *logaccept)
{
	FILE *fp = NULL;
	char path[64];
	char sdn_subnet_class[INET_ADDRSTRLEN + 1];
	char wan_ifname[IFNAMSIZ] = {0};
#ifdef	RTCONFIG_MULTIWAN_IF
#ifdef RTCONFIG_MULTIWAN_PROFILE
	char prefix[16] = {0};
	int group;
	char mt_group[32] = {0};
	char word[4] = {0}, *next = NULL;
	int i;
#endif
#else
	int i;
#if defined(RTCONFIG_DUALWAN) || defined(RTCONFIG_MULTICAST_IPTV)
	int wan_max_unit = WAN_UNIT_MAX;
#endif
#endif
#ifdef RTCONFIG_IPV6
#ifdef RTCONFIG_MULTIWAN_IF
	int wan6_idx = mtwan_get_mapped_unit(pmtl->sdn_t.wan6_idx) ? : wan_primary_ifunit_ipv6();
#else
	int wan6_idx = (pmtl->sdn_t.wan6_idx) ? : wan_primary_ifunit_ipv6();
#endif
	int v6_enable = pmtl->nw_t.v6_enable && ipv6x_enabled(wan6_idx);
#endif

	if (!pmtl || !logdrop || !logaccept)
		return -1;

#if !defined(RTCONFIG_MULTIWAN_IF) && defined(RTCONFIG_DUALWAN) && defined(RTCONFIG_MULTICAST_IPTV)
	if (nvram_get_int("switch_stb_x") > 6)
		wan_max_unit = WAN_UNIT_MULTICAST_IPTV_MAX;
#endif

	snprintf(path, sizeof(path), "%s/sdn-%d-filter", sdn_dir, pmtl->sdn_t.sdn_idx);
	// remove old rule first
	if (!access(path, F_OK))
	{
		eval("sed", "-i", "s/-I/-D/g", path);
		eval("sed", "-i", "s/-A/-D/g", path);
		eval("iptables-restore", "--noflush", path);
	}

	fp = fopen(path, "w");
	if (fp)
	{
		fprintf(fp, filter_header);

		if (pmtl->enable)
		{
			if (pmtl->nw_t.idx)	//ignore br0
			{
				// DNS and DHCP ports
				if (!pmtl->nw_t.dhcp_enable)
				{
					fprintf(fp, "-A %s -i %s -p udp --dport 53 -j %s\n", SDN_FILTER_INPUT_CHAIN, pmtl->nw_t.ifname, logaccept);
				}
				else
				{
					fprintf(fp, "-A %s -i %s -p udp -m multiport --dports 53,67,68 -j %s\n", SDN_FILTER_INPUT_CHAIN, pmtl->nw_t.ifname, logaccept);
				}

				// access host service
				if (!pmtl->sdn_t.ahs_sw)
				{
					fprintf(fp, "-A %s -i %s -d %s -j %s\n", SDN_FILTER_INPUT_CHAIN, pmtl->nw_t.ifname, nvram_safe_get("lan_ipaddr"), logdrop);
				}

				// accept state NEW
				fprintf(fp, "-A %s -i %s -m state --state NEW -j %s\n", SDN_FILTER_INPUT_CHAIN, pmtl->nw_t.ifname, logaccept);
			}

			// WAN rules
			if (!(pmtl->sdn_t.killsw_sw))
			{
#ifdef RTCONFIG_MULTIWAN_IF
				if( pmtl->sdn_t.mtwan_idx)
				{
#ifdef RTCONFIG_MULTIWAN_PROFILE
					snprintf(prefix, sizeof(prefix), "mtwan%d_", pmtl->sdn_t.mtwan_idx);
					if (nvram_pf_get_int(prefix, "enable"))
					{
						group = nvram_pf_get_int(prefix, "group");
						strlcpy(mt_group, nvram_pf_safe_get(prefix, "mt_group"), sizeof(mt_group));
						i = 0;
						foreach(word, mt_group, next)
						{
							if (group == atoi(word))
							{
								strlcpy(wan_ifname, get_wan_ifname(mtwan_get_mapped_unit(i + MULTI_WAN_START_IDX)), sizeof(wan_ifname));
								_add_sdn_iptables_wan_rules(fp, wan_ifname, pmtl, logdrop, logaccept);
							}
							i++;
						}
					}
					else
					{
						mtwan_get_ifname((pmtl->sdn_t.wan_idx)?:mtwan_get_default_wan(), wan_ifname, sizeof(wan_ifname));
						if (!wan_ifname[0])
							strlcpy(wan_ifname, get_wan_ifname(wan_primary_ifunit()), sizeof(wan_ifname));
						_add_sdn_iptables_wan_rules(fp, wan_ifname, pmtl, logdrop, logaccept);
					}
				}
				else if (pmtl->sdn_t.wan_idx == 0 && nvram_get_int("mtwan1_enable"))	//follow default Multi-WAN profile
				{
					group = nvram_get_int("mtwan1_group");
					strlcpy(mt_group, nvram_safe_get("mtwan1_mt_group"), sizeof(mt_group));
					i = 0;
					foreach(word, mt_group, next)
					{
						if (group == atoi(word))
						{
							strlcpy(wan_ifname, get_wan_ifname(mtwan_get_mapped_unit(i + MULTI_WAN_START_IDX)), sizeof(wan_ifname));
							_add_sdn_iptables_wan_rules(fp, wan_ifname, pmtl, logdrop, logaccept);
						}
						i++;
					}
#endif
				}
				else
				{
					mtwan_get_ifname((pmtl->sdn_t.wan_idx)?:mtwan_get_default_wan(), wan_ifname, sizeof(wan_ifname));
					if (!wan_ifname[0])
						strlcpy(wan_ifname, get_wan_ifname(wan_primary_ifunit()), sizeof(wan_ifname));
					_add_sdn_iptables_wan_rules(fp, wan_ifname, pmtl, logdrop, logaccept);
				}
#else
				if (0
#if defined(RTCONFIG_DUALWAN)
				 || (!strstr(nvram_safe_get("wans_dualwan"), "none") && nvram_match("wans_mode", "lb"))
#endif
#if defined(RTCONFIG_MULTICAST_IPTV)
				 || (nvram_get_int("switch_stb_x") > 6)
#endif
				) {
					for (i = WAN_UNIT_FIRST; i < wan_max_unit; ++i)
					{
						if (!is_wan_connect(i))
							continue;

						strlcpy(wan_ifname, get_wan_ifname(i), sizeof(wan_ifname));
						_add_sdn_iptables_wan_rules(fp, wan_ifname, pmtl, logdrop, logaccept);
					}
				}
				else
				{
					strlcpy(wan_ifname, get_wan_ifname(wan_primary_ifunit()), sizeof(wan_ifname));
					_add_sdn_iptables_wan_rules(fp, wan_ifname, pmtl, logdrop, logaccept);
				}
#endif
#ifdef RTCONFIG_MULTI_PPP
				if(pmtl->sdn_t.wan_idx == 0)
				{
					char wan_prefix[16];
					int base_unit = wan_primary_ifunit();
					int ppp_conn;
					int i;
					snprintf(wan_prefix, sizeof(wan_prefix), "wan%d_", base_unit);
					if (get_wan_proto(wan_prefix) == WAN_PPPOE && (ppp_conn = nvram_pf_get_int(wan_prefix, "ppp_conn")) > 1)
					{
						for (i = 1; i <= ppp_conn; i++)
						{
							fprintf(fp, "-I %s -i %s -o ppp%d -j %s\n", SDN_FILTER_FORWARD_CHAIN
								, pmtl->nw_t.ifname, get_mtppp_wan_unit(base_unit, i), logaccept);
						}
					}
				}
#endif
			}
		}
		fprintf(fp, commit_str);
		fclose(fp);
		eval("iptables-restore", "--noflush", path);
	}

	snprintf(path, sizeof(path), "%s/sdn-%d-nat", sdn_dir, pmtl->sdn_t.sdn_idx);
	// remove old rule first
	if (!access(path, F_OK))
	{
		eval("sed", "-i", "s/-A/-D/g", path);
		eval("iptables-restore", "--noflush", path);
	}

	fp = fopen(path, "w");
	if (fp)
	{
		fprintf(fp, nat_header);

		if (pmtl->enable)
		{
#ifdef RTCONFIG_DNSPRIVACY
			if (pmtl->nw_t.idx)	//ignore br0
			{
				// DoT
				if (pmtl->nw_t.dot_enable)
				{
					fprintf(fp, "-A PREROUTING --src %s/%d -p udp -m udp --dport 53 -j DNAT --to %s:53%02d\n", pmtl->nw_t.subnet, pmtl->nw_t.prefixlen, pmtl->nw_t.addr, pmtl->nw_t.idx);
				}
			}
#endif
			ip2class((char *)(pmtl->nw_t.addr), (char *)(pmtl->nw_t.netmask), sdn_subnet_class);
			// NAT lookback
			fprintf(fp, "-A POSTROUTING -s %s -d %s -o %s -j MASQUERADE\n", sdn_subnet_class, sdn_subnet_class, pmtl->nw_t.ifname);
		}
		fprintf(fp, commit_str);
		fclose(fp);
		eval("iptables-restore", "--noflush", path);
	}

#ifdef RTCONFIG_IPV6
	char wan6face[IFNAMSIZ] = "";
	char addr6[INET6_ADDRSTRLEN] = "";
	strlcpy(addr6, nvram_safe_get(ipv6_nvname("ipv6_rtr_addr")), sizeof(addr6));	//br0 ip6 addr
	strlcpy(wan6face, get_wan6_ifname(wan6_idx), sizeof(wan6face));
	snprintf(path, sizeof(path), "%s/sdn-%d-filter-v6", sdn_dir, pmtl->sdn_t.sdn_idx);

	// remove old rule first
	if (!access(path, F_OK))
	{
		eval("sed", "-i", "s/-I/-D/g", path);
		eval("sed", "-i", "s/-A/-D/g", path);
		eval("ip6tables-restore", "--noflush", path);
	}

	if (v6_enable)
	{
		fp = fopen(path, "w");
		if (fp)
		{
			fprintf(fp, filter_header);
			if (pmtl->enable)
			{
				if (pmtl->nw_t.idx)	//ignore br0
				{
					// DNS
					if (!pmtl->nw_t.dhcp_enable)
					{
						fprintf(fp, "-A %s -i %s -p udp --dport 53 -j %s\n", SDN_FILTER_INPUT_CHAIN, pmtl->nw_t.ifname, logaccept);
					}
					else
					{
						fprintf(fp, "-A %s -i %s -p udp -m multiport --dports 53,547 -j %s\n", SDN_FILTER_INPUT_CHAIN, pmtl->nw_t.ifname, logaccept);
					}

					// access host service
					if (!pmtl->sdn_t.ahs_sw && addr6[0] != '\0')
					{
						fprintf(fp, "-A %s -i %s -d %s -j %s\n", SDN_FILTER_INPUT_CHAIN, pmtl->nw_t.ifname, addr6, logdrop);
					}

					// accept state NEW
					fprintf(fp, "-A %s -i %s -m state --state NEW -j %s\n", SDN_FILTER_INPUT_CHAIN, pmtl->nw_t.ifname, logaccept);
				}

				// kill switch
				if (!(pmtl->sdn_t.killsw_sw))
				{
					fprintf(fp, "-I %s -i %s -o %s -j %s\n", SDN_FILTER_FORWARD_CHAIN, pmtl->nw_t.ifname, wan6face, logaccept);
				}
			}
			fprintf(fp, commit_str);
			fclose(fp);
			eval("ip6tables-restore", "--noflush", path);
		}

		// TODO: If there is IPv6 NAT. NAT lookback might be needed.
	}
#endif
	return 0;
}

int reset_sdn_iptables(const int sdn_idx)
{
	char path[256], sdn_chain[32];

	snprintf(path, sizeof(path), "%s/sdn-%d-filter", sdn_dir, sdn_idx);
	// remove old rule first
	if (!access(path, F_OK))
	{
		eval("sed", "-i", "s/-I/-D/g", path);
		eval("sed", "-i", "s/-A/-D/g", path);
		eval("iptables-restore", "--noflush", path);
	}
	snprintf(path, sizeof(path), "%s/sdn-%d-nat", sdn_dir, sdn_idx);
	// remove old rule first
	if (!access(path, F_OK))
	{
		eval("sed", "-i", "s/-A/-D/g", path);
		eval("iptables-restore", "--noflush", path);
	}

	snprintf(sdn_chain, sizeof(sdn_chain), "%s_%d", URL_FILTER_INPUT_CHAIN, sdn_idx);
	_remove_iptables_chain(sdn_chain, 0);

	snprintf(sdn_chain, sizeof(sdn_chain), "%s_%d", URL_FILTER_FORWARD_CHAIN, sdn_idx);
	_remove_iptables_chain(sdn_chain, 0);

	snprintf(sdn_chain, sizeof(sdn_chain), "%s_%d", NW_SERVICE_FILTER_FORWARD_CHAIN, sdn_idx);
	_remove_iptables_chain(sdn_chain, 0);

#ifdef RTCONFIG_IPV6
	snprintf(path, sizeof(path), "%s/sdn-%d-filter-v6", sdn_dir, sdn_idx);
	// remove old rule first
	if (!access(path, F_OK))
	{
		eval("sed", "-i", "s/-I/-D/g", path);
		eval("sed", "-i", "s/-A/-D/g", path);
		eval("ip6tables-restore", "--noflush", path);
	}

	snprintf(sdn_chain, sizeof(sdn_chain), "%s_%d", URL_FILTER_INPUT_CHAIN, sdn_idx);
	_remove_iptables_chain(sdn_chain, 1);

	snprintf(sdn_chain, sizeof(sdn_chain), "%s_%d", URL_FILTER_FORWARD_CHAIN, sdn_idx);
	_remove_iptables_chain(sdn_chain, 1);

	snprintf(sdn_chain, sizeof(sdn_chain), "%s_%d", NW_SERVICE_FILTER_FORWARD_CHAIN, sdn_idx);
	_remove_iptables_chain(sdn_chain, 1);
#endif
	return 0;
}

/**************************************************************************************************************************************************
 *  URL FILTER
 **************************************************************************************************************************************************/
#define TYPE_IP 0
#define TYPE_MAC 1
#define TYPE_IPRANGE 2

typedef struct _tagURLF_RULE
{
	int enable;
	char *addr;
	char *url;
} URLF_RULE;

typedef struct _tagURLF_PF
{
	int enable;
	int mode;
	size_t rl_sz;
	llist *rl;
} URLF_PF;

void _free_urlf_rl(void **data)
{
	URLF_RULE *rule;

	if (data && *data)
	{
		rule = (URLF_RULE *)(*data);
		SAFE_FREE(rule->addr);
		SAFE_FREE(rule->url);
		SAFE_FREE(rule);
	}
}

int _check_urlf_pf_enable(const int urlf_idx)
{
	char name[64];
	char *nv, *nvp;
	char *enable, *desc, *mode;
	int ret = 0;

	// read options in control  nvram
	snprintf(name, sizeof(name), "urlf%d_ctrl", urlf_idx);
	nv = nvp = strdup(nvram_safe_get(name));
	if (nv)
	{
		if (vstrsep(nvp, ">", &desc, &enable, &mode) != 3)
		{
			SAFE_FREE(nv);
			return ret;
		}
		else
		{
			ret = atoi(enable);
		}
		SAFE_FREE(nv);
	}
	else
	{
		return ret;
	}
	return ret;
}

int _get_urlf_pf(const int urlf_idx, URLF_PF *pf)
{
	char name[64];
	char *nv, *nvp, *b;
	char *enable, *addr, *url, *desc, *mode;
	llist *lst;
	URLF_RULE *rule;

	if (!urlf_idx || !pf)
		return -1;

	// read options in control  nvram
	snprintf(name, sizeof(name), "urlf%d_ctrl", urlf_idx);
	nv = nvp = strdup(nvram_safe_get(name));
	if (nvp)
	{
		if (vstrsep(nvp, ">", &desc, &enable, &mode) != 3)
		{
			SAFE_FREE(nv);
			return -1;
		}
		else
		{
			pf->enable = atoi(enable);
			pf->mode = atoi(mode);
		}
	}
	else
	{
		return -1;
	}
	//_dprintf("[%s, %d]<%s, %s, %s>\n", __FUNCTION__, __LINE__, desc, enable, mode);
	SAFE_FREE(nv);

	// read rule list in nvram
	snprintf(name, sizeof(name), "urlf%d_rl", urlf_idx);
	nv = nvp = strdup(nvram_safe_get(name));

	pf->rl_sz = 0;
	while (nvp)
	{
		if ((b = strsep(&nvp, "<")) == NULL)
			break;
		if ((vstrsep(b, ">", &enable, &addr, &url)) != 3)
			continue;

		rule = calloc(1, sizeof(URLF_RULE));
		if (rule)
		{
			if (enable)
			{
				rule->enable = atoi(enable);
			}
			if (addr && *addr)
			{
				rule->addr = strdup(addr);
			}
			if (url && *url)
			{
				rule->url = strdup(url);
			}

			lst = calloc(1, sizeof(llist));
			if (lst)
			{
				lst->data = rule;
				append_llist(&(pf->rl), rule);
				++pf->rl_sz;
			}
			else
			{
				SAFE_FREE(rule);
			}
		}
	}
	SAFE_FREE(nv);
	return 0;
}

static void _dump_urlf_prof(URLF_PF *pf)
{
	llist *lst;
	URLF_RULE *rule;
	int cnt = 0;

	if (pf)
	{
		_dprintf("[%s, %d]enable=%d, mode=%d, rule_sz=%d, rule:\n", __FUNCTION__, __LINE__, pf->enable, pf->mode, pf->rl_sz);
		lst = pf->rl;
		while (lst)
		{
			rule = (URLF_RULE *)lst->data;
			_dprintf("\t[%d;%s;%s]", rule->enable, rule->addr ? rule->addr : "", rule->url ? rule->url : "");
			++cnt;
			if (cnt % 3 == 0)
				_dprintf("\n");
			lst = lst->next;
		}
		_dprintf("\n");
	}
}

static int _convert_url_to_hex(const char *url, char *output, const size_t output_len)
{
	char tmp[256], tmp2[256];
	char *ptr, *p, *pvalue;
	int first;

	if (!url || !output)
		return -1;

	if (strstr(url, "."))
	{
		memset(tmp, 0, sizeof(tmp));
		memset(tmp2, 0, sizeof(tmp2));

		ptr = pvalue = strdup(url);
		first = 1;
		while (pvalue && (p = strsep(&pvalue, ".")) != NULL)
		{
			if (!strlen(p))
			{
				pvalue++;
				continue;
			}
			if (first)
			{
				first = 0;
				if (!strncasecmp(p, "www", 3))
					continue;
			}
			snprintf(tmp2, sizeof(tmp2), "%s|%02x|%s", tmp, (unsigned int)strlen(p), p);
			strlcpy(tmp, tmp2, sizeof(tmp));
		}
		strlcpy(output, tmp, output_len);
		SAFE_FREE(ptr);
		return 0;
	}
	return -1;
}

#ifdef RTCONFIG_IPV6
static int _write_UrlFilter(const MTLAN_T *pmtl, const URLF_PF *urlf_pf, const char *chain, const char *ifname, const char *ip, const char *logdrop, FILE *fp, FILE *fp_ipv6)
#else
static int _write_UrlFilter(const MTLAN_T *pmtl, const URLF_PF *urlf_pf, const char *chain, const char *ifname, const char *ip, const char *logdrop, FILE *fp)
#endif
{
	char srcips[64];
	char config_rule[20];
	char timef[256] = {'\0'};
	char sdn_chain[32];
	URLF_RULE *rule;
	llist *lst;
	char url[512];

	if (!chain || !ifname || !ip || !logdrop || !fp)
		return -1;

		//_dprintf("[%s, %d]<%s, %s, %s, %s>\n", __FUNCTION__, __LINE__, chain, ifname, ip, logdrop);

#ifdef RTCONFIG_IPV6
	char config_rulev6[128];
#ifdef RTCONFIG_MULTIWAN_IF
	int wan6_idx = mtwan_get_mapped_unit(pmtl->sdn_t.wan6_idx) ? : wan_primary_ifunit_ipv6();
#else
	int wan6_idx = (pmtl->sdn_t.wan6_idx) ? : wan_primary_ifunit_ipv6();
#endif
	int v6_enable = pmtl->nw_t.v6_enable && ipv6x_enabled(wan6_idx);

	if (v6_enable)
		snprintf(config_rulev6, sizeof(config_rulev6), "-d %s", nvram_safe_get(ipv6_nvname_by_unit("ipv6_rtr_addr", wan6_idx)));

	/* Prevent v6 lan ip is NULL */
	if (strlen(config_rulev6) <= 3)
		strcpy(config_rulev6, "");
#endif
	snprintf(sdn_chain, sizeof(sdn_chain), "%s_%d", chain, pmtl->sdn_t.sdn_idx);

	fprintf(fp, "-N %s\n", sdn_chain);
#ifdef RTCONFIG_IPV6
	if (fp_ipv6)
		fprintf(fp_ipv6, "-N %s\n", sdn_chain);
#endif

	if (urlf_pf)
	{
		snprintf(config_rule, sizeof(config_rule), "-d %s", ip);
		if (urlf_pf->enable)
		{
			// check rule list
			lst = urlf_pf->rl;
			while (lst)
			{
				rule = (URLF_RULE *)lst->data;
				if (rule)
				{
					// Handle source type format
					srcips[0] = '\0';
					if (rule->addr && rule->addr[0] != '\0')
					{
						char srcAddr[32];
						int src_type = addr_type_parse(rule->addr, srcAddr, sizeof(srcAddr));
						if (src_type == TYPE_IP)
							snprintf(srcips, sizeof(srcips), "-s %s", srcAddr);
						else if (src_type == TYPE_MAC)
							snprintf(srcips, sizeof(srcips), "-m mac --mac-source %s", srcAddr);
						else if (src_type == TYPE_IPRANGE)
							snprintf(srcips, sizeof(srcips), "-m iprange --src-range %s", srcAddr);
					}

					if (rule->url)
					{
						if (!strcmp(chain, URL_FILTER_FORWARD_CHAIN) && !urlf_pf->mode)
						{
							fprintf(fp, "-I %s %s -p tcp %s -m webstr --url \"%s\" -j REJECT --reject-with tcp-reset\n", sdn_chain, srcips, timef, rule->url);
						}

						memset(url, 0, sizeof(url));
						if (!_convert_url_to_hex(rule->url, url, sizeof(url)))
						{
							fprintf(fp, "%s %s -i %s %s %s -p udp --dport 53 %s -m string --icase --hex-string \"%s\" --algo bm -j %s\n",
									(strcmp(chain, URL_FILTER_INPUT_CHAIN) == 0) ? "-A" : "-I", sdn_chain, ifname, srcips,
									(strcmp(chain, URL_FILTER_INPUT_CHAIN) == 0 && !urlf_pf->mode) ? config_rule : "", timef, url,
									!urlf_pf->mode ? logdrop : "ACCEPT");
						}
						else
						{
							fprintf(fp, "%s %s -i %s %s %s -p udp --dport 53 %s -m string --icase --string \"%s\" --algo bm -j %s\n",
									(strcmp(chain, URL_FILTER_INPUT_CHAIN) == 0) ? "-A" : "-I", sdn_chain, ifname, srcips,
									(strcmp(chain, URL_FILTER_INPUT_CHAIN) == 0 && !urlf_pf->mode) ? config_rule : "", timef, rule->url,
									!urlf_pf->mode ? logdrop : "ACCEPT");
						}
#ifdef RTCONFIG_IPV6
						if (v6_enable && fp_ipv6)
						{
							if (!strcmp(chain, URL_FILTER_FORWARD_CHAIN) && !urlf_pf->mode)
							{
								if (fp_ipv6)
								{
									fprintf(fp_ipv6, "-I %s -p tcp %s -m webstr --url \"%s\" -j REJECT --reject-with tcp-reset\n", sdn_chain, timef, rule->url);
								}
								else
								{
									doSystem("ip6tables -I %s -p tcp %s -m webstr --url \"%s\" -j REJECT --reject-with tcp-reset\n", sdn_chain, timef, rule->url);
								}
							}

							if (url[0] != '\0')
							{
								fprintf(fp_ipv6, "%s %s -i %s %s -p udp --dport 53 %s -m string --icase --hex-string \"%s\" --algo bm -j %s\n",
										(strcmp(chain, URL_FILTER_INPUT_CHAIN) == 0) ? "-A" : "-I", sdn_chain, ifname,
										(strcmp(chain, URL_FILTER_INPUT_CHAIN) == 0 && !urlf_pf->mode) ? config_rulev6 : "", timef, url,
										!urlf_pf->mode ? logdrop : "ACCEPT");
							}
							else
							{
								fprintf(fp_ipv6, "%s %s -i %s %s -p udp --dport 53 %s -m string --icase --string \"%s\" --algo bm -j %s\n",
										(strcmp(chain, URL_FILTER_INPUT_CHAIN) == 0) ? "-A" : "-I", sdn_chain, ifname,
										(strcmp(chain, URL_FILTER_INPUT_CHAIN) == 0 && !urlf_pf->mode) ? config_rulev6 : "", timef, rule->url,
										!urlf_pf->mode ? logdrop : "ACCEPT");
							}
						} // if (ipv6_enabled())
#endif
					} // if(rule->url)
				}	  // if(rule)
				lst = lst->next;
			} // while(lst)

			if (urlf_pf->mode == 1)
			{
				fprintf(fp, "-A %s -i %s -p udp --dport 53 -m string --icase --hex-string \"|04|asus|03|com|00|\" --algo bm -j ACCEPT\n", sdn_chain, ifname);
				fprintf(fp, "-A %s -i %s -p udp --dport 53 -j DROP\n", sdn_chain, ifname);
#ifdef RTCONFIG_IPV6
				if (v6_enable && fp_ipv6)
				{
					fprintf(fp_ipv6, "-A %s -i %s -p udp --dport 53 -m string --icase --hex-string \"|04|asus|03|com|00|\" --algo bm -j ACCEPT\n", sdn_chain, ifname);
					fprintf(fp_ipv6, "-A %s -i %s -p udp --dport 53 -j DROP\n", sdn_chain, ifname);
				}
#endif
			}
		} // if(urlf_pf->enable)
	}	  // if(urlf_pf)
	return 0;
}

#ifdef RTCONFIG_IPV6
int write_URLFilter_SDN(const MTLAN_T *pmtl, const char *logdrop, FILE *fp_filter, FILE *fpv6_filter)
#else
int write_URLFilter_SDN(const MTLAN_T *pmtl, const char *logdrop, FILE *fp_filter)
#endif
{
	URLF_PF urlf_pf;

	if (!pmtl || !fp_filter)
		return -1;

	memset(&urlf_pf, 0, sizeof(urlf_pf));
	if (pmtl->sdn_t.urlf_idx > 0)
	{
		_get_urlf_pf(pmtl->sdn_t.urlf_idx, &urlf_pf);
		_dump_urlf_prof(&urlf_pf);
	}

#ifdef RTCONFIG_IPV6
	_write_UrlFilter(pmtl, (pmtl->enable && pmtl->sdn_t.urlf_idx) ? &urlf_pf : NULL, URL_FILTER_INPUT_CHAIN, pmtl->nw_t.ifname, pmtl->nw_t.addr, logdrop, fp_filter, fpv6_filter);
	_write_UrlFilter(pmtl, (pmtl->enable && pmtl->sdn_t.urlf_idx) ? &urlf_pf : NULL, URL_FILTER_FORWARD_CHAIN, pmtl->nw_t.ifname, pmtl->nw_t.addr, logdrop, fp_filter, fpv6_filter);
#else
	_write_UrlFilter(pmtl, (pmtl->enable && pmtl->sdn_t.urlf_idx) ? &urlf_pf : NULL, URL_FILTER_INPUT_CHAIN, pmtl->nw_t.ifname, pmtl->nw_t.addr, logdrop, fp_filter);
	_write_UrlFilter(pmtl, (pmtl->enable && pmtl->sdn_t.urlf_idx) ? &urlf_pf : NULL, URL_FILTER_FORWARD_CHAIN, pmtl->nw_t.ifname, pmtl->nw_t.addr, logdrop, fp_filter);
#endif

	if (pmtl->sdn_t.urlf_idx > 0)
		free_llist(&(urlf_pf.rl), _free_urlf_rl);
	return 0;
}

int handle_URLFilter_jump_rule(const MTLAN_T *pmtl)
{
	char sdn_chain[32];

	if (!pmtl)
		return -1;

	snprintf(sdn_chain, sizeof(sdn_chain), "%s_%d", URL_FILTER_INPUT_CHAIN, pmtl->sdn_t.sdn_idx);
	doSystem("iptables -D %s -p udp -m udp --dport 53 -j %s 2>/dev/null", URL_FILTER_INPUT_CHAIN, sdn_chain);

	if (pmtl->enable && _check_urlf_pf_enable(pmtl->sdn_t.urlf_idx))
	{
		doSystem("iptables -A %s -p udp -m udp --dport 53 -j %s", URL_FILTER_INPUT_CHAIN, sdn_chain);
	}

	snprintf(sdn_chain, sizeof(sdn_chain), "%s_%d", URL_FILTER_FORWARD_CHAIN, pmtl->sdn_t.sdn_idx);
	doSystem("iptables -D %s -j %s 2>/dev/null", URL_FILTER_FORWARD_CHAIN, sdn_chain);
	if (pmtl->enable && _check_urlf_pf_enable(pmtl->sdn_t.urlf_idx))
	{
		doSystem("iptables -A %s -j %s", URL_FILTER_FORWARD_CHAIN, sdn_chain);
	}
#ifdef RTCONFIG_IPV6
#ifdef RTCONFIG_MULTIWAN_IF
	int wan6_idx = mtwan_get_mapped_unit(pmtl->sdn_t.wan6_idx) ? : wan_primary_ifunit_ipv6();
#else
	int wan6_idx = (pmtl->sdn_t.wan6_idx) ? : wan_primary_ifunit_ipv6();
#endif
	int v6_enable = pmtl->nw_t.v6_enable && ipv6x_enabled(wan6_idx);
	if (v6_enable)
	{
		snprintf(sdn_chain, sizeof(sdn_chain), "%s_%d", URL_FILTER_INPUT_CHAIN, pmtl->sdn_t.sdn_idx);
		doSystem("ip6tables -D %s -p udp -m udp --dport 53 -j %s 2>/dev/null", URL_FILTER_INPUT_CHAIN, sdn_chain);

		if (pmtl->enable && _check_urlf_pf_enable(pmtl->sdn_t.urlf_idx))
		{
			doSystem("ip6tables -A %s -p udp -m udp --dport 53 -j %s", URL_FILTER_INPUT_CHAIN, sdn_chain);
		}

		snprintf(sdn_chain, sizeof(sdn_chain), "%s_%d", URL_FILTER_FORWARD_CHAIN, pmtl->sdn_t.sdn_idx);
		doSystem("ip6tables -D %s -j %s 2>/dev/null", URL_FILTER_FORWARD_CHAIN, sdn_chain);
		if (pmtl->enable && _check_urlf_pf_enable(pmtl->sdn_t.urlf_idx))
		{
			doSystem("ip6tables -A %s -j %s", URL_FILTER_FORWARD_CHAIN, sdn_chain);
		}
	}
#endif
	return 0;
}

int update_URLFilter(const int urlf_idx)
{
	char logaccept[32], logdrop[32];
	MTLAN_T *pmtl = NULL;
	size_t mtl_sz = 0;
	int i;
	URLF_PF urlf_pf;
	FILE *fp = NULL;
	char file_name[256];
#ifdef RTCONFIG_IPV6
	FILE *fpv6 = NULL;
	char filev6_name[256];
	int wan6_idx, v6_enable;
#endif
	char sdn_chain[32];

	memset(&urlf_pf, 0, sizeof(urlf_pf));

	pmtl = (MTLAN_T *)INIT_MTLAN(sizeof(MTLAN_T));
	if (pmtl)
	{
		if (_get_urlf_pf(urlf_idx, &urlf_pf) == -1)
		{
			FREE_MTLAN((void *)pmtl);
			return -1;
		}
		else
			_dump_urlf_prof(&urlf_pf);

		get_drop_accept(logdrop, sizeof(logdrop), logaccept, sizeof(logaccept));

		if (get_mtlan_by_idx(SDNFT_TYPE_URLF, urlf_idx, pmtl, &mtl_sz) && mtl_sz > 0)
		{
			for (i = 0; i < mtl_sz; ++i)
			{
				snprintf(sdn_chain, sizeof(sdn_chain), "%s_%d", URL_FILTER_INPUT_CHAIN, pmtl[i].sdn_t.sdn_idx);
				_remove_iptables_chain(sdn_chain, 0);
#ifdef RTCONFIG_IPV6
				_remove_iptables_chain(sdn_chain, 1);
#endif
				snprintf(sdn_chain, sizeof(sdn_chain), "%s_%d", URL_FILTER_FORWARD_CHAIN, pmtl[i].sdn_t.sdn_idx);
				_remove_iptables_chain(sdn_chain, 0);
#ifdef RTCONFIG_IPV6
				_remove_iptables_chain(sdn_chain, 1);
#endif

				if (create_iptables_file(IPTABLE_TYPE_FILTER, pmtl[i].sdn_t.sdn_idx, &fp, file_name, sizeof(file_name), 0) == -1)
					_dprintf("[%s] create iptable file v4 fail!\n", __FUNCTION__);
#ifdef RTCONFIG_IPV6
#ifdef RTCONFIG_MULTIWAN_IF
				wan6_idx = mtwan_get_mapped_unit(pmtl->sdn_t.wan6_idx) ? : wan_primary_ifunit_ipv6();
#else
				wan6_idx = (pmtl->sdn_t.wan6_idx) ? : wan_primary_ifunit_ipv6();
#endif
				v6_enable = pmtl->nw_t.v6_enable && ipv6x_enabled(wan6_idx);
				if (v6_enable)
				{
					if (create_iptables_file(IPTABLE_TYPE_FILTER, pmtl[i].sdn_t.sdn_idx, &fpv6, filev6_name, sizeof(filev6_name), 1) == -1)
						_dprintf("[%s] create iptable file v6 fail!\n", __FUNCTION__);
				}
#endif

#ifdef RTCONFIG_IPV6
				_write_UrlFilter(&pmtl[i], &urlf_pf, URL_FILTER_INPUT_CHAIN, pmtl->nw_t.ifname, pmtl->nw_t.addr, logdrop, fp, fpv6);
				_write_UrlFilter(&pmtl[i], &urlf_pf, URL_FILTER_FORWARD_CHAIN, pmtl->nw_t.ifname, pmtl->nw_t.addr, logdrop, fp, fpv6);
#else
				_write_UrlFilter(&pmtl[i], &urlf_pf, URL_FILTER_INPUT_CHAIN, pmtl->nw_t.ifname, pmtl->nw_t.addr, logdrop, fp);
				_write_UrlFilter(&pmtl[i], &urlf_pf, URL_FILTER_FORWARD_CHAIN, pmtl->nw_t.ifname, pmtl->nw_t.addr, logdrop, fp);
#endif

				close_n_restore_iptables_file(IPTABLE_TYPE_FILTER, &fp, file_name, 0);
#ifdef RTCONFIG_IPV6
				if (v6_enable)
					close_n_restore_iptables_file(IPTABLE_TYPE_FILTER, &fpv6, filev6_name, 1);
#endif
				handle_URLFilter_jump_rule(&pmtl[i]);
			}
		}
		FREE_MTLAN((void *)pmtl);
		free_llist(&(urlf_pf.rl), _free_urlf_rl);
	}
	return 0;
}

/**************************************************************************************************************************************************
 *  Network Service FILTER
 **************************************************************************************************************************************************/
typedef struct _tagNWF_RULE
{
	char *src_ip;
	char *src_port;
	char *dst_ip;
	char *dst_port;
	char *proto;
} NWF_RULE;

typedef struct _tagNWF_PF
{
	int enable;
	char *mode;
	char *date;
	char *time1;
	char *time2;
	char *icmp;
	size_t rl_sz;
	llist *rl;
} NWF_PF;

void _free_nwf_rl(void **data)
{
	NWF_RULE *rule;

	if (data && *data)
	{
		rule = (NWF_RULE *)(*data);
		SAFE_FREE(rule->src_port);
		SAFE_FREE(rule->src_port);
		SAFE_FREE(rule->dst_ip);
		SAFE_FREE(rule->dst_port);
		SAFE_FREE(rule->proto);
		SAFE_FREE(rule);
	}
}

void _free_nwf_pf(NWF_PF *nwf_pf)
{
	if (nwf_pf)
	{
		SAFE_FREE(nwf_pf->date);
		SAFE_FREE(nwf_pf->mode);
		SAFE_FREE(nwf_pf->time1);
		SAFE_FREE(nwf_pf->time2);
		SAFE_FREE(nwf_pf->icmp);
		if (nwf_pf->rl)
			free_llist(&(nwf_pf->rl), _free_nwf_rl);
	}
}

int _check_nwf_pf_enable(const int nwf_idx)
{
	char name[64];
	char *nv, *nvp;
	char *enable, *desc, *mode, *date, *time1, *time2, *icmp;
	int ret = 0;

	// read options in control  nvram
	snprintf(name, sizeof(name), "nwf%d_ctrl", nwf_idx);
	nv = nvp = strdup(nvram_safe_get(name));
	if (nv)
	{
		if (vstrsep(nvp, ">", &desc, &enable, &mode, &date, &time1, &time2, &icmp) != 7)
		{
			SAFE_FREE(nv);
			return ret;
		}
		else
		{
			ret = atoi(enable);
		}
		SAFE_FREE(nv);
	}
	else
	{
		return ret;
	}
	return ret;
}

int _get_nwf_pf(const int nwf_idx, NWF_PF *pf)
{
	char name[64];
	char *nv, *nvp, *b;
	char *enable, *desc, *mode, *date, *time1, *time2, *icmp;
	char *src_ip, *src_port, *dst_ip, *dst_port, *proto;
	llist *lst;
	NWF_RULE *rule;

	if (!nwf_idx || !pf)
		return -1;

	// read options in control  nvram
	snprintf(name, sizeof(name), "nwf%d_ctrl", nwf_idx);
	nv = nvp = strdup(nvram_safe_get(name));
	if (nvp)
	{
		if (vstrsep(nvp, ">", &desc, &enable, &mode, &date, &time1, &time2, &icmp) != 7)
		{
			SAFE_FREE(nv);
			return -1;
		}
		else
		{
			pf->enable = atoi(enable);
			pf->mode = strdup(mode);
			pf->date = strdup(date);
			pf->time1 = strdup(time1);
			pf->time2 = strdup(time2);
			pf->icmp = strdup(icmp);
		}
	}
	else
	{
		return -1;
	}
	SAFE_FREE(nv);

	// read rule list in nvram
	snprintf(name, sizeof(name), "nwf%d_rl", nwf_idx);
	nv = nvp = strdup(nvram_safe_get(name));

	pf->rl_sz = 0;
	while (nvp)
	{
		if ((b = strsep(&nvp, "<")) == NULL)
			break;
		if ((vstrsep(b, ">", &src_ip, &src_port, &dst_ip, &dst_port, &proto)) != 5)
			continue;

		rule = calloc(1, sizeof(NWF_RULE));
		if (rule)
		{
			if (src_ip && *src_ip)
			{
				rule->src_ip = strdup(src_ip);
			}
			else
				rule->src_ip = strdup("");

			if (src_port && *src_port)
			{
				rule->src_port = strdup(src_port);
			}
			else
				rule->src_port = strdup("");

			if (dst_ip && *dst_ip)
			{
				rule->dst_ip = strdup(dst_ip);
			}
			else
				rule->dst_ip = strdup("");

			if (dst_port && *dst_port)
			{
				rule->dst_port = strdup(dst_port);
			}
			else
				rule->dst_port = strdup("");

			if (proto && *proto)
			{
				rule->proto = strdup(proto);
			}
			else
				rule->proto = strdup("");

			lst = calloc(1, sizeof(llist));
			if (lst)
			{
				lst->data = rule;
				append_llist(&(pf->rl), rule);
				++pf->rl_sz;
			}
			else
			{
				SAFE_FREE(rule->src_ip);
				SAFE_FREE(rule->src_port);
				SAFE_FREE(rule->dst_ip);
				SAFE_FREE(rule->dst_port);
				SAFE_FREE(rule->proto);
				SAFE_FREE(rule);
			}
		}
	}
	SAFE_FREE(nv);
	return 0;
}

static void _dump_nwf_prof(NWF_PF *pf)
{
	llist *lst;
	NWF_RULE *rule;
	int cnt = 0;

	if (pf)
	{
		_dprintf("[%s, %d]enable=%d, mode=%d, date = %s, time1=%s, time2=%s, icmp=%s, rule_sz=%d, rule:\n", __FUNCTION__, __LINE__,
				 pf->enable, pf->mode, pf->date ? pf->date : "", pf->time1 ? pf->time1 : "", pf->time2 ? pf->time2 : "", pf->icmp ? pf->icmp : "", pf->rl_sz);
		lst = pf->rl;
		while (lst)
		{
			rule = (NWF_RULE *)lst->data;
			_dprintf("\t[%s;%s;%s;%s;%s]", rule->src_ip ? rule->src_ip : "", rule->src_port ? rule->src_port : "",
					 rule->dst_ip ? rule->dst_ip : "", rule->dst_port ? rule->dst_port : "", rule->proto ? rule->proto : "");
			++cnt;
			if (cnt % 3 == 0)
				_dprintf("\n");
			lst = lst->next;
		}
		_dprintf("\n");
	}
}

static int _timematch_conv2(char *mstr, const size_t mstr_size, const char *date, const char *time, const char *time2)
{
	char *datestr[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
	char timestart[6], timestop[6], timestart2[6], timestop2[6];
	int i;
	int ret = 1;
	int dow = 0; // day of week
	char buf[1024], buf2[1024];
	char mstr2[2048];

	struct datetime datetime[7];

	if (!mstr || !date || !time || !time2)
		return 0;

	if (strlen(date) != 7 || strlen(time) != 8 || strlen(time2) != 8)
	{
		ret = 0;
		goto no_match;
	}

	if (strncmp(date, "0000000", 7) == 0)
	{
		ret = 0;
		goto no_match;
	}

	if (strncmp(date, "1111111", 7) == 0 &&
		strncmp(time, "00002359", 8) == 0 &&
		strncmp(time2, "00002359", 8) == 0)
	{
		goto no_match;
	}
	// schedule day of week
	for (i = 0; i <= 6; i++)
	{
		dow += (date[i] - '0') << (6 - i);
	}
	memset(timestart, 0, sizeof(timestart));
	memset(timestop, 0, sizeof(timestop));
	memset(timestart2, 0, sizeof(timestart2));
	memset(timestop2, 0, sizeof(timestop2));

	// weekdays time
	strncpy(timestart, time, 4);
	strncpy(timestop, time + 4, 4);
	// weekend time
	strncpy(timestart2, time2, 4);
	strncpy(timestop2, time2 + 4, 4);

	// initialize
	memset(datetime, 0, sizeof(datetime));
	// cprintf("%s: dow=%d, timestart=%d, timestop=%d, timestart2=%d, timestop2=%d, sizeof(datetime)=%d\n", __FUNCTION__, dow, atoi(timestart), atoi(timestop), atoi(timestart2), atoi(timestop2), sizeof(datetime)); //tmp test

	// Sunday
	if ((dow & 0x40) != 0)
	{
		if (atoi(timestart2) < atoi(timestop2))
		{
			snprintf(datetime[0].start, sizeof(datetime[0].start), "%s00", timestart2);
			snprintf(datetime[0].stop, sizeof(datetime[0].stop), "%s59", timestop2);
		}
		else
		{
			snprintf(datetime[0].start, sizeof(datetime[0].start), "%s00", timestart2);
			strncpy(datetime[0].stop, "235959", 6);
			snprintf(datetime[1].tmpstop, sizeof(datetime[1].tmpstop), "%s59", timestop2);
		}
	}

	// Monday to Friday
	for (i = 1; i < 6; i++)
	{
		if ((dow & 1 << (6 - i)) != 0)
		{
			if (atoi(timestart) < atoi(timestop))
			{
				snprintf(datetime[i].start, sizeof(datetime[i].start), "%s00", timestart);
				snprintf(datetime[i].stop, sizeof(datetime[i].stop), "%s59", timestop);
			}
			else
			{
				snprintf(datetime[i].start, sizeof(datetime[i].start), "%s00", timestart);
				strncpy(datetime[i].stop, "235959", 6);
				snprintf(datetime[i + 1].tmpstop, sizeof(datetime[i + 1].tmpstop), "%s59", timestop);
			}
		}
	}

	// Saturday
	if ((dow & 0x01) != 0)
	{
		if (atoi(timestart2) < atoi(timestop2))
		{
			snprintf(datetime[6].start, sizeof(datetime[6].start), "%s00", timestart2);
			snprintf(datetime[6].stop, sizeof(datetime[6].stop), "%s59", timestop2);
		}
		else
		{
			snprintf(datetime[6].start, sizeof(datetime[6].start), "%s00", timestart2);
			strncpy(datetime[6].stop, "235959", 6);
			snprintf(datetime[0].tmpstop, sizeof(datetime[0].tmpstop), "%s59", timestop2);
		}
	}

	for (i = 0; i < 7; i++)
	{
		// cprintf("%s: i=%d, start=%s, stop=%s, tmpstop=%s\n", __FUNCTION__, i, datetime[i].start, datetime[i].stop, datetime[i].tmpstop); //tmp test

		// cascade cross-night time
		if ((strcmp(datetime[i].tmpstop, "") != 0) && (strcmp(datetime[i].start, "") != 0) && (strcmp(datetime[i].stop, "") != 0))
		{
			if ((atoi(datetime[i].tmpstop) >= atoi(datetime[i].start)) && (atoi(datetime[i].tmpstop) <= atoi(datetime[i].stop)))
			{
				strncpy(datetime[i].start, "000000", 6);
				strncpy(datetime[i].tmpstop, "", 6);
			}
			else if (atoi(datetime[i].tmpstop) > atoi(datetime[i].stop))
			{
				strncpy(datetime[i].start, "000000", 6);
				strncpy(datetime[i].stop, datetime[i].tmpstop, 6);
				strncpy(datetime[i].tmpstop, "", 6);
			}
		}

		// cprintf("%s: i=%d, start=%s, stop=%s, tmpstop=%s\n", __FUNCTION__, i, datetime[i].start, datetime[i].stop, datetime[i].tmpstop); //tmp test

		char tmp1[9], tmp2[9];
		memset(tmp1, 0, sizeof(tmp1));
		memset(tmp2, 0, sizeof(tmp2));
		memset(buf, 0, sizeof(buf));

		// cross-night period
		if (strcmp(datetime[i].tmpstop, "") != 0)
		{
			snprintf(buf2, sizeof(buf2), "%s-m time --timestop %s" DAYS_PARAM "%s", buf, str2time(datetime[i].tmpstop, tmp2, sizeof(tmp2), STOP_TIME), datestr[i]);

			strlcpy(buf, buf2, sizeof(buf));
		}

		// normal period
		if ((strcmp(datetime[i].start, "") != 0) && (strcmp(datetime[i].stop, "") != 0))
		{
			if (strcmp(buf, "") != 0)
			{
				snprintf(buf2, sizeof(buf2), "%s>", buf); // add ">"
				strlcpy(buf, buf2, sizeof(buf));
			}
			if ((strcmp(datetime[i].start, "000000") == 0) && (strcmp(datetime[i].stop, "235959") == 0)) // whole day
			{
				snprintf(buf2, sizeof(buf2), "%s-m time" DAYS_PARAM "%s", buf, datestr[i]);
				strlcpy(buf, buf2, sizeof(buf));
			}
			else if ((strcmp(datetime[i].start, "000000") != 0) && (strcmp(datetime[i].stop, "235959") == 0)) // start ~ 2359
			{
				snprintf(buf2, sizeof(buf2), "%s-m time --timestart %s" DAYS_PARAM "%s", buf, str2time(datetime[i].start, tmp1, sizeof(tmp1), START_TIME), datestr[i]);

				strlcpy(buf, buf2, sizeof(buf));
			}
			else if ((strcmp(datetime[i].start, "000000") == 0) && (strcmp(datetime[i].stop, "235959") != 0)) // 0 ~ stop
			{
				snprintf(buf2, sizeof(buf2), "%s-m time --timestop %s" DAYS_PARAM "%s", buf, str2time(datetime[i].stop, tmp2, sizeof(tmp2), STOP_TIME), datestr[i]);

				strlcpy(buf, buf2, sizeof(buf));
			}
			else if ((strcmp(datetime[i].start, "000000") != 0) && (strcmp(datetime[i].stop, "235959") != 0)) // start ~ stop
			{
				snprintf(buf2, sizeof(buf2), "%s-m time --timestart %s --timestop %s" DAYS_PARAM "%s", buf, str2time(datetime[i].start, tmp1, sizeof(tmp1), START_TIME), str2time(datetime[i].stop, tmp2, sizeof(tmp2), STOP_TIME), datestr[i]);
				strlcpy(buf, buf2, sizeof(buf));
			}
		}

		if (strcmp(buf, "") != 0)
		{
			if (strcmp(mstr, "") == 0)
				snprintf(mstr, mstr_size, "%s", buf);
			else
			{
				snprintf(mstr2, sizeof(mstr2), "%s>%s", mstr, buf); // add ">"
				strlcpy(mstr, mstr2, mstr_size);
			}
		}

		// cprintf("%s: mstr=%s, len=%d\n", __FUNCTION__, mstr, strlen(mstr)); //tmp test
	}

	return ret;

no_match:
	// sprintf(mstr, "");
	mstr[0] = 0; // oleg patch
	return ret;
}

static char *_filter_conv(const char *proto, const char *flag, const char *srcip, const char *srcport, const char *dstip, const char *dstport, char *buf, const size_t buf_sz)
{
	char newstr[64];

	if (!proto || !flag || !srcip || !srcport || !dstip || !dstport || !buf)
		return NULL;

	_dprintf("filter : %s,%s,%s,%s,%s,%s\n", proto, flag, srcip, srcport, dstip, dstport);

	memset(buf, 0, buf_sz);

	if (strcmp(proto, "") != 0)
	{
		snprintf(newstr, sizeof(newstr), " -p %s", proto);
		strlcat(buf, newstr, buf_sz);
	}

	if (strcmp(flag, "") != 0)
	{
		// snprintf(newstr, sizeof(newstr), " --tcp-flags %s RST", flag);
		snprintf(newstr, sizeof(newstr), " --tcp-flags %s %s", flag, flag);
		strlcat(buf, newstr, buf_sz);
	}

	if (strcmp(srcip, "") != 0)
	{
		if (strchr(srcip, '-'))
			snprintf(newstr, sizeof(newstr), " --src-range %s", srcip);
		else
			snprintf(newstr, sizeof(newstr), " -s %s", srcip);
		strlcat(buf, newstr, buf_sz);
	}

	if (strcmp(srcport, "") != 0)
	{
		snprintf(newstr, sizeof(newstr), " --sport %s", srcport);
		strlcat(buf, newstr, buf_sz);
	}

	if (strcmp(dstip, "") != 0)
	{
		if (strchr(dstip, '-'))
			snprintf(newstr, sizeof(newstr), " --dst-range %s", dstip);
		else
			snprintf(newstr, sizeof(newstr), " -d %s", dstip);
		strlcat(buf, newstr, buf_sz);
	}

	if (strcmp(dstport, "") != 0)
	{
		snprintf(newstr, sizeof(newstr), " --dport %s", dstport);
		strlcat(buf, newstr, buf_sz);
	}
	return buf;
	// printf("str: %s\n", g_buf);
}

#ifdef RTCONFIG_IPV6
int write_NwServiceFilter_SDN(const MTLAN_T *pmtl, const char *logdrop, const char *logaccept, FILE *fp_filter, FILE *fpv6_filter)
#else
int write_NwServiceFilter_SDN(const MTLAN_T *pmtl, const char *logdrop, const char *logaccept, FILE *fp_filter)
#endif
{
	NWF_PF nwf_pf;
	NWF_RULE *rule;
	char lanwan_timematch[2048];
	char lanwan_buf[2048];
	char ptr[32], *icmplist;
	const char *ftype, *dtype;
	char protoptr[16], flagptr[16];
	char srcipbuf[32], dstipbuf[32];
	int apply;
	char *p, *g;
	char setting[1024], chain[64];
	int v4v6_ok = IPT_V4;
	char *wan_if = NULL;
	char name[32], *vpnc_if;
#if !defined(RTCONFIG_MULTIWAN_IF) && (defined(RTCONFIG_DUALWAN) || defined(RTCONFIG_MULTICAST_IPTV))
	int i;
	int wan_max_unit = WAN_UNIT_MAX;
#endif
#ifdef RTCONFIG_IPV6
	char wan6face[IFNAMSIZ];
	int wan6_idx, v6_enable;
#endif
#ifdef RTCONFIG_MULTIWAN_IF
	int wan_unit;
	char wan_ifname[IFNAMSIZ] = {0};
	char wan_prefix[16] = {0};
#endif
	llist *list;

	if (!pmtl || !fp_filter || !logaccept || !logdrop)
		return -1;

#if !defined(RTCONFIG_MULTIWAN_IF) && defined(RTCONFIG_MULTICAST_IPTV)
	if (nvram_get_int("switch_stb_x") > 6)
		wan_max_unit = WAN_UNIT_MULTICAST_IPTV_MAX;
#endif

#ifdef RTCONFIG_MULTIWAN_IF
	if(pmtl->sdn_t.wan_idx)
	{
		if (mtwan_get_ifname(pmtl->sdn_t.wan_idx, wan_ifname, sizeof(wan_ifname)) < 0)
			strlcpy(wan_ifname, get_wan_ifname(wan_primary_ifunit()), sizeof(wan_ifname));
	}
	else
	{
		if (mtwan_get_ifname(mtwan_get_default_wan(), wan_ifname, sizeof(wan_ifname)) < 0)
			strlcpy(wan_ifname, get_wan_ifname(wan_primary_ifunit()), sizeof(wan_ifname));
	}
	wan_if = wan_ifname;
#ifdef RTCONFIG_IPV6
	if(pmtl->sdn_t.wan6_idx && (wan_unit = mtwan_get_real_wan(pmtl->sdn_t.wan6_idx, wan_prefix, sizeof(wan_prefix))) != -1)
		strlcpy(wan6face, get_wan6_ifname(wan_unit), sizeof(wan6face));
	else
	{
		if ((wan_unit = mtwan_get_default_wan()) != -1)
			strlcpy(wan6face, get_wan6_ifname(wan_unit), sizeof(wan6face));
		else
			strlcpy(wan6face, get_wan6face(), sizeof(wan6face));
	}
#endif
#else
#ifdef RTCONFIG_IPV6
	strlcpy(wan6face, get_wan6face(), sizeof(wan6face));
#endif

	wan_if = get_wan_ifname(wan_primary_ifunit());
#endif

	snprintf(chain, sizeof(chain), "%s_%d", NW_SERVICE_FILTER_FORWARD_CHAIN, pmtl->sdn_t.sdn_idx);

	fprintf(fp_filter, "-N %s\n", chain);
#ifdef RTCONFIG_IPV6
	if (fpv6_filter)
	{
		fprintf(fpv6_filter, "-N %s\n", chain);
	}

#ifdef RTCONFIG_MULTIWAN_IF
	wan6_idx = mtwan_get_mapped_unit(pmtl->sdn_t.wan6_idx) ? : wan_primary_ifunit_ipv6();
#else
	wan6_idx = (pmtl->sdn_t.wan6_idx) ? : wan_primary_ifunit_ipv6();
#endif
	v6_enable = pmtl->nw_t.v6_enable && ipv6x_enabled(wan6_idx);
#endif

	memset(&nwf_pf, 0, sizeof(nwf_pf));

	if (pmtl->sdn_t.nwf_idx > 0)
	{
		if (_get_nwf_pf(pmtl->sdn_t.nwf_idx, &nwf_pf))
		{
			_dprintf("[%s] Cannot get nwf profile(%d)\n", __FUNCTION__, pmtl->sdn_t.nwf_idx);
			return -1;
		}
		else
			_dump_nwf_prof(&nwf_pf);

		if (nwf_pf.enable)
		{
			memset(lanwan_timematch, 0, sizeof(lanwan_timematch));
			memset(lanwan_buf, 0, sizeof(lanwan_buf));
			apply = _timematch_conv2(lanwan_timematch, sizeof(lanwan_timematch), nwf_pf.date, nwf_pf.time1, nwf_pf.time2);

			if (!strcmp(nwf_pf.mode, "DROP"))
			{
				dtype = logdrop;
				ftype = logaccept;
			}
			else
			{
				dtype = logaccept;
				ftype = logdrop;
			}

			if (apply)
			{
				v4v6_ok = IPT_V4;
				list = nwf_pf.rl;
				while (list)
				{
					rule = (NWF_RULE *)list->data;
					if (!rule)
					{
						list = list->next;
						continue;
					}

					protoflag_conv(rule->proto, protoptr, 0);
					protoflag_conv(rule->proto, flagptr, 1);
					_filter_conv(protoptr, flagptr, iprange_ex_conv(rule->src_ip, srcipbuf), rule->src_port, iprange_ex_conv(rule->dst_ip, dstipbuf), rule->dst_port, setting, sizeof(setting));

					if (rule->src_ip && rule->src_ip[0] != '\0')
						v4v6_ok = ipt_addr_compact(srcipbuf, v4v6_ok, (v4v6_ok == IPT_V4));
					if (rule->dst_ip && rule->dst_ip[0] != '\0')
						v4v6_ok = ipt_addr_compact(dstipbuf, v4v6_ok, (v4v6_ok == IPT_V4));

					if (pmtl->sdn_t.vpnc_idx > 0)
					{
						snprintf(name, sizeof(name), "vpnc%d_ifname", pmtl->sdn_t.vpnc_idx);
						vpnc_if = nvram_safe_get(name);
						if (vpnc_if && vpnc_if[0] != '\0')
						{
							/* separate lanwan timematch */
							strlcpy(lanwan_buf, lanwan_timematch, sizeof(lanwan_buf));
							p = lanwan_buf;
							while (p)
							{
								if ((g = strsep(&p, ">")) != NULL)
								{
									// cprintf("[timematch] g=%s, p=%s, lanwan=%s, buf=%s\n", g, p, lanwan_timematch, lanwan_buf);
									if (v4v6_ok & IPT_V4)
										fprintf(fp_filter, "-A %s %s -i %s -o %s %s -j %s\n", chain, g, pmtl->nw_t.ifname, vpnc_if, setting, ftype);
#ifdef RTCONFIG_IPV6
										// TODO: VPN client not support IPv6 yet.
#endif
								}
							}
						}
					}
					else
					{
#if !defined(RTCONFIG_MULTIWAN_IF) && (defined(RTCONFIG_DUALWAN) || defined(RTCONFIG_MULTICAST_IPTV))
						for (i = 0; i < wan_max_unit; ++i)
						{
							if (!is_wan_connect(i))
								continue;

							wan_if = get_wan_ifname(i);
#endif
							/* separate lanwan timematch */
							strlcpy(lanwan_buf, lanwan_timematch, sizeof(lanwan_buf));
							p = lanwan_buf;
							while (p)
							{
								if ((g = strsep(&p, ">")) != NULL)
								{
									// cprintf("[timematch] g=%s, p=%s, lanwan=%s, buf=%s\n", g, p, lanwan_timematch, lanwan_buf);
									if (v4v6_ok & IPT_V4)
										fprintf(fp_filter, "-A %s %s -i %s -o %s %s -j %s\n", chain, g, pmtl->nw_t.ifname, wan_if, setting, ftype);
#ifdef RTCONFIG_IPV6
									if (v6_enable && (v4v6_ok & IPT_V6) && *wan6face)
										fprintf(fpv6_filter, "-A %s %s -i %s -o %s %s -j %s\n", chain, g, pmtl->nw_t.ifname, wan6face, setting, ftype);
#endif
								}
							}
#if !defined(RTCONFIG_MULTIWAN_IF) && (defined(RTCONFIG_DUALWAN) || defined(RTCONFIG_MULTICAST_IPTV))
						}
#endif
					}
					list = list->next;
				}
			}

			if (nwf_pf.icmp && nwf_pf.icmp[0] != '\0')
			{
				// ICMP
				foreach (ptr, nwf_pf.icmp, icmplist)
				{
#if !defined(RTCONFIG_MULTIWAN_IF) && (defined(RTCONFIG_DUALWAN) || defined(RTCONFIG_MULTICAST_IPTV))
					for (i = WAN_UNIT_FIRST; i < wan_max_unit; ++i)
					{
						if (!is_wan_connect(i))
							continue;

						wan_if = get_wan_ifname(i);
#endif
						/* separate lanwan timematch */
						strlcpy(lanwan_buf, lanwan_timematch, sizeof(lanwan_buf));
						p = lanwan_buf;
						while (p)
						{
							if ((g = strsep(&p, ">")) != NULL)
							{
								// cprintf("[timematch] g=%s, p=%s, lanwan=%s, buf=%s\n", g, p, lanwan_timematch, lanwan_buf);
								if (v4v6_ok & IPT_V4)
									fprintf(fp_filter, "-A %s %s -i %s -o %s -p icmp --icmp-type %s -j %s\n", chain, g, pmtl->nw_t.ifname, wan_if, ptr, ftype);
							}
						}
#if !defined(RTCONFIG_MULTIWAN_IF) && (defined(RTCONFIG_DUALWAN) || defined(RTCONFIG_MULTICAST_IPTV))
					}
#endif
				}

#ifdef RTCONFIG_IPV6
				/* separate lanwan timematch */
				strlcpy(lanwan_buf, lanwan_timematch, sizeof(lanwan_buf));
				p = lanwan_buf;
				while (p)
				{
					if ((g = strsep(&p, ">")) != NULL)
					{
						// cprintf("[timematch] g=%s, p=%s, lanwan=%s, buf=%s\n", g, p, lanwan_timematch, lanwan_buf);
						if (v6_enable && *wan6face)
							fprintf(fpv6_filter, "-A %s %s -i %s -o %s -p icmp --icmp-type %s -j %s\n", chain, g, pmtl->nw_t.ifname, wan6face, ptr, ftype);
					}
				}
#endif
			}

			// Default
#if !defined(RTCONFIG_MULTIWAN_IF) && (defined(RTCONFIG_DUALWAN) || defined(RTCONFIG_MULTICAST_IPTV))
			for (i = WAN_UNIT_FIRST; i < wan_max_unit; ++i)
			{
				if (!is_wan_connect(i))
					continue;

				wan_if = get_wan_ifname(i);
				fprintf(fp_filter, "-A %s -i %s -o %s -j %s\n", chain, pmtl->nw_t.ifname, wan_if, dtype);
			}
#else
			fprintf(fp_filter, "-A %s -i %s -o %s -j %s\n", chain, pmtl->nw_t.ifname, wan_if, dtype);
#endif

#ifdef RTCONFIG_IPV6
			if (v6_enable && *wan6face)
				fprintf(fpv6_filter, "-A %s -i %s -o %s -j %s\n", chain, pmtl->nw_t.ifname, wan6face, dtype);
#endif
		}
		_free_nwf_pf(&nwf_pf);
	}
	return 0;
}

int handle_NwServiceFilter_jump_rule(const MTLAN_T *pmtl)
{
	char sdn_chain[32];

	if (!pmtl)
		return -1;

	snprintf(sdn_chain, sizeof(sdn_chain), "%s_%d", NW_SERVICE_FILTER_FORWARD_CHAIN, pmtl->sdn_t.sdn_idx);
	doSystem("iptables -D %s -i %s -j %s 2>/dev/null", NW_SERVICE_FILTER_FORWARD_CHAIN, pmtl->nw_t.ifname, sdn_chain);
	if (pmtl->enable && _check_nwf_pf_enable(pmtl->sdn_t.nwf_idx))
	{
		doSystem("iptables -A %s -i %s -j %s", NW_SERVICE_FILTER_FORWARD_CHAIN, pmtl->nw_t.ifname, sdn_chain);
	}
#ifdef RTCONFIG_IPV6
#ifdef RTCONFIG_MULTIWAN_IF
	int wan6_idx = mtwan_get_mapped_unit(pmtl->sdn_t.wan6_idx) ? : wan_primary_ifunit_ipv6();
#else
	int wan6_idx = (pmtl->sdn_t.wan6_idx) ? : wan_primary_ifunit_ipv6();
#endif
	int v6_enable = pmtl->nw_t.v6_enable && ipv6x_enabled(wan6_idx);
	if (v6_enable)
	{
		snprintf(sdn_chain, sizeof(sdn_chain), "%s_%d", NW_SERVICE_FILTER_FORWARD_CHAIN, pmtl->sdn_t.sdn_idx);
		doSystem("ip6tables -D %s -i %s -j %s 2>/dev/null", NW_SERVICE_FILTER_FORWARD_CHAIN, pmtl->nw_t.ifname, sdn_chain);
		if (pmtl->enable && _check_nwf_pf_enable(pmtl->sdn_t.nwf_idx))
		{
			doSystem("ip6tables -A %s -i %s -j %s", NW_SERVICE_FILTER_FORWARD_CHAIN, pmtl->nw_t.ifname, sdn_chain);
		}
	}
#endif
	return 0;
}

int update_NwServiceFilter(const int nwf_idx)
{
	char logaccept[32], logdrop[32];
	MTLAN_T *pmtl = NULL;
	size_t mtl_sz = 0;
	int i;
	NWF_PF nwf_pf;
	FILE *fp = NULL;
	char file_name[256];
#ifdef RTCONFIG_IPV6
	FILE *fpv6 = NULL;
	char filev6_name[256];
	int wan6_idx, v6_enable;
#endif

	memset(&nwf_pf, 0, sizeof(nwf_pf));

	pmtl = (MTLAN_T *)INIT_MTLAN(sizeof(MTLAN_T));
	if (pmtl)
	{
		if (_get_nwf_pf(nwf_idx, &nwf_pf) == -1)
		{
			FREE_MTLAN((void *)pmtl);
			return -1;
		}
		else
			_dump_nwf_prof(&nwf_pf);

		get_drop_accept(logdrop, sizeof(logdrop), logaccept, sizeof(logaccept));

		if (get_mtlan_by_idx(SDNFT_TYPE_NWF, nwf_idx, pmtl, &mtl_sz) && mtl_sz > 0)
		{
			for (i = 0; i < mtl_sz; ++i)
			{
				if (create_iptables_file(IPTABLE_TYPE_FILTER, pmtl[i].sdn_t.sdn_idx, &fp, file_name, sizeof(file_name), 0) == -1)
					_dprintf("[%s] create iptable file v4 fail!\n", __FUNCTION__);
#ifdef RTCONFIG_IPV6
#ifdef RTCONFIG_MULTIWAN_IF
				wan6_idx = mtwan_get_mapped_unit(pmtl->sdn_t.wan6_idx) ? : wan_primary_ifunit_ipv6();
#else
				wan6_idx = (pmtl->sdn_t.wan6_idx) ? : wan_primary_ifunit_ipv6();
#endif
				v6_enable = pmtl->nw_t.v6_enable && ipv6x_enabled(wan6_idx);
				if (v6_enable)
				{
					if (create_iptables_file(IPTABLE_TYPE_FILTER, pmtl[i].sdn_t.sdn_idx, &fpv6, filev6_name, sizeof(filev6_name), 1) == -1)
						_dprintf("[%s] create iptable file v6 fail!\n", __FUNCTION__);
				}
#endif

#ifdef RTCONFIG_IPV6
				write_NwServiceFilter_SDN(&pmtl[i], logdrop, logaccept, fp, fpv6);
#else
				write_NwServiceFilter_SDN(&pmtl[i], logdrop, logaccept, fp);
#endif

				close_n_restore_iptables_file(IPTABLE_TYPE_FILTER, &fp, file_name, 0);
#ifdef RTCONFIG_IPV6
				if (v6_enable)
					close_n_restore_iptables_file(IPTABLE_TYPE_FILTER, &fpv6, filev6_name, 1);
#endif
				handle_NwServiceFilter_jump_rule(&pmtl[i]);
			}
		}
		FREE_MTLAN((void *)pmtl);
		_free_nwf_pf(&nwf_pf);
	}
	return 0;
}

/**************************************************************************************************************************************************
 *  SDN internal access
 **************************************************************************************************************************************************/
extern const char SDN_INTERNAL_ACCESS_CHAIN[];

int handle_SDN_internal_access(const char *logdrop, const char *logaccept)
{
	FILE *fp = NULL;
	char path[64];
	char *nv, *nvp, *b;
	char *idx1, *idx2;
	MTLAN_T *pmtl = NULL;
	size_t mtl_sz = 0;
	char ifname1[IFNAMSIZ], ifname2[IFNAMSIZ];
#ifdef RTCONFIG_IPV6
	FILE *fpv6 = NULL;
	char pathv6[64];
#endif

	if (!logaccept)
		return -1;

	pmtl = (MTLAN_T *)INIT_MTLAN(sizeof(MTLAN_T));
	if (pmtl)
	{
		snprintf(path, sizeof(path), "%s/sdn_internal_access", sdn_dir);

		fp = fopen(path, "w");

#ifdef RTCONFIG_IPV6
		if (ipv6_enabled())
		{
			snprintf(pathv6, sizeof(pathv6), "%s/sdn_internal_access_v6", sdn_dir);
			fpv6 = fopen(pathv6, "w");
		}
#endif
		if (fp)
		{
			fprintf(fp, filter_header);
			fprintf(fp, "-F %s\n", SDN_INTERNAL_ACCESS_CHAIN);
#ifdef RTCONFIG_IPV6
			if (fpv6)
			{
				fprintf(fpv6, filter_header);
				fprintf(fpv6, "-F %s\n", SDN_INTERNAL_ACCESS_CHAIN);
			}
#endif
			nv = nvp = strdup(nvram_safe_get("sdn_access_rl"));

			while (nvp)
			{
				if ((b = strsep(&nvp, "<")) == NULL)
					break;
				if (vstrsep(b, ">", &idx1, &idx2) != 2)
					continue;

				if (get_mtlan_by_idx(SDNFT_TYPE_SDN, atoi(idx1), pmtl, &mtl_sz) && mtl_sz > 0)
				{
					strlcpy(ifname1, pmtl[0].nw_t.ifname, sizeof(ifname1));
				}
				else
				{
					ifname1[0] = '\0';
				}
				if (get_mtlan_by_idx(SDNFT_TYPE_SDN, atoi(idx2), pmtl, &mtl_sz) && mtl_sz > 0)
				{
					strlcpy(ifname2, pmtl[0].nw_t.ifname, sizeof(ifname2));
				}
				else
				{
					ifname2[0] = '\0';
				}

				if (ifname1[0] != '\0' && ifname2[0] != '\0' && strcmp(ifname1, ifname2))
				{
					fprintf(fp, "-A %s -i %s -o %s -j %s\n", SDN_INTERNAL_ACCESS_CHAIN, ifname1, ifname2, logaccept);
					fprintf(fp, "-A %s -i %s -o %s -j %s\n", SDN_INTERNAL_ACCESS_CHAIN, ifname2, ifname1, logaccept);
#ifdef RTCONFIG_IPV6
					if (fpv6)
					{
						fprintf(fpv6, "-A %s -i %s -o %s -j %s\n", SDN_INTERNAL_ACCESS_CHAIN, ifname1, ifname2, logaccept);
						fprintf(fpv6, "-A %s -i %s -o %s -j %s\n", SDN_INTERNAL_ACCESS_CHAIN, ifname2, ifname1, logaccept);
					}
#endif
				}
			}
			SAFE_FREE(nv);
			fprintf(fp, "-A %s -i br+ -o br+ -j %s\n", SDN_INTERNAL_ACCESS_CHAIN, logdrop);
			fprintf(fp, commit_str);
			fclose(fp);
			eval("iptables-restore", "--noflush", path);

#ifdef RTCONFIG_IPV6
			if (fpv6)
			{
				fprintf(fpv6, "-A %s -i br+ -o br+ -j %s\n", SDN_INTERNAL_ACCESS_CHAIN, logdrop);
				fprintf(fpv6, commit_str);
				fclose(fpv6);
				eval("ip6tables-restore", "--noflush", pathv6);
			}
#endif
		}
		FREE_MTLAN((void *)pmtl);
	}
	return 0;
}

/**************************************************************************************************************************************************
 *  Firewall
 **************************************************************************************************************************************************/
