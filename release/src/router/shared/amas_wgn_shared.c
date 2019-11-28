/*
** amas_wgn_shared.c
**
**
**
*/
#include "amas_wgn_shared.h"

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

wgn_subnet_rule* wgn_subnet_list_get_from_content(
	char *content, 
	wgn_subnet_rule *list, 
	size_t max_list_size, 
	size_t *ret_list_size)
{
	char *nv = NULL, *nvp = NULL, *b;
	char *ipaddr;
	char *ipmask;
	char *dhcp_enable;
	char *dhcp_start; 
	char *dhcp_end;
	char *dhcp_lease;
	char *domain_name;
	char *dns;
	char *wins;
	char *ema;
	char *macipbinding;

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
		fields = vstrsep(b, ">", 
			&ipaddr, &ipmask, &dhcp_enable, &dhcp_start, &dhcp_end, 
			&dhcp_lease, &domain_name, &dns, &wins, &ema, &macipbinding);

		if (fields != WGN_SUBNET_RULE_MAX_FIELDS)
			continue;

		if (list_size > max_list_size)
			break;

		// ipaddr
		if (ipaddr && strlen(ipaddr) > 0 && strlen(ipaddr) <= WGN_SUBNET_RULE_IP_STRING_SIZE) strlcpy(list[list_size].ipaddr, ipaddr, sizeof(list[list_size].ipaddr));
		// ipmask
		if (ipmask && strlen(ipmask) > 0 && strlen(ipmask) <= WGN_SUBNET_RULE_IP_STRING_SIZE) strlcpy(list[list_size].ipmask, ipmask, sizeof(list[list_size].ipmask));
		// dhcp_enable
		if (dhcp_enable && strlen(dhcp_enable) > 0) list[list_size].dhcp_enable = (atoi(dhcp_enable) == 0) ? 0 : 1;
		// dhcp_start
		if (dhcp_start && strlen(dhcp_start) > 0 && strlen(dhcp_start) <= WGN_SUBNET_RULE_IP_STRING_SIZE) strlcpy(list[list_size].dhcp_start, dhcp_start, sizeof(list[list_size].dhcp_start));
		// dhcp_end
		if (dhcp_end && strlen(dhcp_end) > 0 && strlen(dhcp_end) <= WGN_SUBNET_RULE_IP_STRING_SIZE) strlcpy(list[list_size].dhcp_end, dhcp_end, sizeof(list[list_size].dhcp_end));
		// dhcp_lease
		if (dhcp_lease && strlen(dhcp_lease) > 0) list[list_size].dhcp_lease = atoi(dhcp_lease);
		// domain_name
		if (domain_name && strlen(domain_name) > 0 && strlen(domain_name) <= WGN_SUBNET_RULE_DOMIN_MAIN_SIZE) strlcpy(list[list_size].domain_name, domain_name, sizeof(list[list_size].domain_name));
		// dns
		if (dns && strlen(dns) > 0 && strlen(dns) <= WGN_SUBNET_RULE_DNS_SIZE) strlcpy(list[list_size].dns, dns, sizeof(list[list_size].dns));
		// wins
		if (wins && strlen(wins) > 0 && strlen(wins) <= WGN_SUBNET_RULE_DNS_SIZE) strlcpy(list[list_size].wins, wins, sizeof(list[list_size].wins));
		// ema
		if (ema && strlen(ema) > 0) list[list_size].ema = atoi(ema);
		// macipbinding
		if (macipbinding && strlen(macipbinding) > 0 && strlen(macipbinding) <= WGN_SUBNET_RULE_MACIP_BINDING_SIZE) strlcpy(list[list_size].macipbinding, macipbinding, sizeof(list[list_size].macipbinding));
		list_size++;			
	}

	if (ret_list_size)
		*ret_list_size = list_size;

	free(nv);
	return list;
}

char* wgn_subnet_list_set_to_buffer(
	wgn_subnet_rule *list, 
	size_t list_size, 
	char *buffer, 
	size_t max_buffer_size)
{
	char s[32];
	size_t i;

	if (!list || list_size <= 0 || !buffer || max_buffer_size <= 0)
		return NULL;

	if (max_buffer_size > WGN_SUBNET_RULE_MAX_BUFFER_SIZE)
		return NULL;

	for (i=0; i<list_size; i++)
	{
		// subnet_rule
		strlcat(buffer, "<", max_buffer_size);
		// subnet_rule : ipaddr
		memset(s, 0, sizeof(s));
		snprintf(s, sizeof(s), "%s>", list[i].ipaddr);
		strlcat(buffer, s, max_buffer_size);
		// subnet_rule : ipmask
		memset(s, 0, sizeof(s));
		snprintf(s, sizeof(s), "%s>", list[i].ipmask);
		strlcat(buffer, s, max_buffer_size);
		// subnet_rule : dhcp_enable
		memset(s, 0, sizeof(s));
		snprintf(s, sizeof(s), "%d>", list[i].dhcp_enable);
		strlcat(buffer, s, max_buffer_size);
		// subnet_rule : dhcp_start
		memset(s, 0, sizeof(s));
		snprintf(s, sizeof(s), "%s>", list[i].dhcp_start);
		strlcat(buffer, s, max_buffer_size);
		// subnet_rule : dhcp_end
		memset(s, 0, sizeof(s));
		snprintf(s, sizeof(s), "%s>", list[i].dhcp_end);
		strlcat(buffer, s, max_buffer_size);
		// subnet_rule : dhcp_lease
		memset(s, 0, sizeof(s));
		snprintf(s, sizeof(s), "%d>", list[i].dhcp_lease);
		strlcat(buffer, s, max_buffer_size);
		// subnet_rule : domain_name
		memset(s, 0, sizeof(s));
		snprintf(s, sizeof(s), "%s>", list[i].domain_name);
		strlcat(buffer, s, max_buffer_size);
		// subnet_rule : dns
		memset(s, 0, sizeof(s));
		snprintf(s, sizeof(s), "%s>", list[i].dns);
		strlcat(buffer, s, max_buffer_size);
		// subnet_rule : wins
		memset(s, 0, sizeof(s));
		snprintf(s, sizeof(s), "%s>", list[i].wins);
		strlcat(buffer, s, max_buffer_size);
		// subnet_rule : ema
		memset(s, 0, sizeof(s));
		snprintf(s, sizeof(s), "%d>", list[i].ema);
		strlcat(buffer, s, max_buffer_size);
		// subnet_rule : macipbinding
		memset(s, 0, sizeof(s));
		snprintf(s, sizeof(s), "%s", list[i].macipbinding);
		strlcat(buffer, s, max_buffer_size);
	}

	return buffer;
}

wgn_subnet_rule* wgn_subnet_list_get_from_nvram(
	wgn_subnet_rule *list, 
	size_t max_list_size, 
	size_t *ret_list_size)
{
	char *b = NULL;
	wgn_subnet_rule *p = NULL;

	if (ret_list_size)
		*ret_list_size = 0;

	if (!list || max_list_size <= 0)
		return NULL;

	if (!(b = strdup(nvram_safe_get(WGN_SUBNET_RULE_NVRAM))))
		return NULL;

	p = wgn_subnet_list_get_from_content(b, list, max_list_size, ret_list_size);
	free(b);

	return p;
}

void wgn_subnet_list_set_to_nvram(
	wgn_subnet_rule *list, 
	size_t list_size)
{
	char b[WGN_SUBNET_RULE_MAX_BUFFER_SIZE];

	if (!list || list_size <= 0)
		return;

	memset(b, 0, sizeof(b));
	if (wgn_subnet_list_set_to_buffer(list, list_size, b, sizeof(b)))
	{
		nvram_set(WGN_SUBNET_RULE_NVRAM, b);
		nvram_commit();
	}

	return;
}

wgn_subnet_rule* wgn_subnet_list_find(
	wgn_subnet_rule *list, 
	size_t list_size, 
	char *subnet_name)
{
	char name[WGN_VLAN_RULE_SUBNET_NAME_SIZE+1];
	wgn_subnet_rule *p = NULL;
	size_t i;

	if (!list || list_size <= 0 || !subnet_name)
		return NULL;

	for (i=0; i<list_size; i++)
	{
		memset(name, 0, sizeof(name));
		snprintf(name, sizeof(name), "%s/%d", list[i].ipaddr, ipmask_to_int(inet_network(list[i].ipmask)));
		if (strncmp(name, subnet_name, strlen(subnet_name)) == 0)
		{
			p = &list[i];
			break;
		}
	}

	return p;
}

wgn_vlan_rule* 	wgn_vlan_list_get_from_content(
	char *content, 
	wgn_vlan_rule *list, 
	size_t max_list_size, 
	size_t *ret_list_size)
{
	char *nv, *nvp, *b;
	char *enable;
	char *vid;
	char *prio;
	char *wanportset;
	char *lanportset;
	char *wl2gset;
	char *wl5gset;
	char *subnet_name;
	char *internet;
	char *public_vlan;

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
		fields = vstrsep(b, ">", 
			&enable, &vid, &prio, &wanportset, &lanportset, 
			&wl2gset, &wl5gset, &subnet_name, &internet, &public_vlan);

		if (fields != WGN_VLAN_RULE_MAX_FIELDS)
			continue;

		if (list_size > max_list_size)
			break;

		// enable
		if (enable && strlen(enable) > 0) list[list_size].enable = atoi(enable);
		// vid
		if (vid && strlen(vid) > 0) list[list_size].vid = atoi(vid);
		// prio
		if (prio && strlen(prio) > 0) list[list_size].prio = atoi(prio);
		// wanportset
		if (wanportset && strlen(wanportset) > 0 && strlen(wanportset) <= WGN_VLAN_RULE_PORTSET_SIZE) strlcpy(list[list_size].wanportset, wanportset, sizeof(list[list_size].wanportset));
		// lanportset
		if (lanportset && strlen(lanportset) > 0 && strlen(lanportset) <= WGN_VLAN_RULE_PORTSET_SIZE) strlcpy(list[list_size].lanportset, lanportset, sizeof(list[list_size].lanportset));
		// wl2gset
		if (wl2gset && strlen(wl2gset) > 0 && strlen(wl2gset) <= WGN_VLAN_RULE_WLANSET_SIZE) strlcpy(list[list_size].wl2gset, wl2gset, sizeof(list[list_size].wl2gset));
		// wl5gset
		if (wl5gset && strlen(wl5gset) > 0 && strlen(wl5gset) <= WGN_VLAN_RULE_WLANSET_SIZE) strlcpy(list[list_size].wl5gset, wl5gset, sizeof(list[list_size].wl5gset));
		// subnet_name
		if (subnet_name && strlen(subnet_name) > 0 && strlen(subnet_name) <= WGN_VLAN_RULE_SUBNET_NAME_SIZE) strlcpy(list[list_size].subnet_name, subnet_name, sizeof(list[list_size].subnet_name));
		// internet
		if (internet && strlen(internet) > 0) list[list_size].internet = atoi(internet);
		// public_vlan
		if (public_vlan && strlen(public_vlan) > 0) list[list_size].public_vlan = atoi(public_vlan);
		list_size++;
	}

	if (ret_list_size)
		*ret_list_size = list_size;

	free(nv);		
	return list;
}

char* wgn_vlan_list_set_to_buffer(
	wgn_vlan_rule *list, 
	size_t list_size, 
	char *buffer, 
	size_t max_buffer_size)
{
	char s[32];
	size_t i;

	if (!list || list_size <= 0 || !buffer || max_buffer_size <= 0)
		return NULL;

	if (max_buffer_size > WGN_VLAN_RULE_MAX_BUFFER_SIZE)
		return NULL;

	for (i=0; i<list_size; i++)
	{
		// vlan_rule
		strlcat(buffer, "<", max_buffer_size);
		// vlan_rule : enable
		memset(s, 0, sizeof(s));
		snprintf(s, sizeof(s), "%d>", list[i].enable);
		strlcat(buffer, s, max_buffer_size);
		// vlan_rule : vid
		memset(s, 0, sizeof(s));
		snprintf(s, sizeof(s), "%d>", list[i].vid);
		strlcat(buffer, s, max_buffer_size);
		// vlan_rule : prio
		memset(s, 0, sizeof(s));
		snprintf(s, sizeof(s), "%d>", list[i].prio);
		strlcat(buffer, s, max_buffer_size);
		// vlan_rule : wanportset
		memset(s, 0, sizeof(s));
		snprintf(s, sizeof(s), "%s>", list[i].wanportset);
		strlcat(buffer, s, max_buffer_size);
		// vlan_rule : lanportset
		memset(s, 0, sizeof(s));
		snprintf(s, sizeof(s), "%s>", list[i].lanportset);
		strlcat(buffer, s, max_buffer_size);
		// vlan_rule : wl2gset
		memset(s, 0, sizeof(s));
		snprintf(s, sizeof(s), "%s>", list[i].wl2gset);
		strlcat(buffer, s, max_buffer_size);
		// vlan_rule : wl5gset
		memset(s, 0, sizeof(s));
		snprintf(s, sizeof(s), "%s>", list[i].wl5gset);
		strlcat(buffer, s, max_buffer_size);
		// vlan_rule : subnet_name
		memset(s, 0, sizeof(s));
		snprintf(s, sizeof(s), "%s>", list[i].subnet_name);
		strlcat(buffer, s, max_buffer_size);
		// vlan_rule : internet
		memset(s, 0, sizeof(s));
		snprintf(s, sizeof(s), "%d>", list[i].internet);
		strlcat(buffer, s, max_buffer_size);
		// vlan_rule : public_vlan
		memset(s, 0, sizeof(s));
		snprintf(s, sizeof(s), "%d>", list[i].public_vlan);
		strlcat(buffer, s, max_buffer_size);
	}

	return buffer;
}

wgn_vlan_rule* wgn_vlan_list_get_from_nvram(
	wgn_vlan_rule *list, 
	size_t max_list_size, 
	size_t *ret_list_size)
{
	char *b = NULL;
	wgn_vlan_rule *p = NULL;

	if (ret_list_size)
		*ret_list_size = 0;

	if (!list || max_list_size <= 0)
		return NULL;

	if (!(b = strdup(nvram_safe_get(WGN_VLAN_RULE_NVRAM))))
		return NULL;

	p = wgn_vlan_list_get_from_content(b, list, max_list_size, ret_list_size);
	free(b);

	return p;	
}

void wgn_vlan_list_set_to_nvram(
	wgn_vlan_rule *list, 
	size_t list_size)
{
	char b[WGN_VLAN_RULE_MAX_BUFFER_SIZE];

	if (!list || list_size <= 0)
		return;

	memset(b, 0, sizeof(b));
	if (wgn_vlan_list_set_to_buffer(list, list_size, b, sizeof(b)))
	{
		nvram_set(WGN_VLAN_RULE_NVRAM, b);
		nvram_commit();
	}

	return;	
}

wgn_vlan_rule* wgn_vlan_list_find(
	wgn_vlan_rule *list, 
	size_t list_size, 
	char *subnet_name)
{
	wgn_vlan_rule *p = NULL;
	size_t i;

	if (!list || list_size <= 0 || !subnet_name)
		return NULL;

	for (i=0; i<list_size; i++)
	{
		if (strncmp(list[i].subnet_name, subnet_name, strlen(subnet_name)) == 0)
		{
			p = &list[i];
			break;
		}		
	}

	return p;
}

wgn_vlan_rule* wgn_vlan_list_find_unit(
	wgn_vlan_rule *list, 
	size_t list_size, 
	int unit, 
	int subunit)
{
	int band, sunit;
	wgn_vlan_rule *p = NULL;
	size_t i;

	if (!list || list_size <= 0 || unit < 0 || subunit < 1)
		return NULL;

	for (i=0; i<list_size; i++)
	{
		band = sunit = -1;
		wgn_get_wl_unit(&band, &sunit, &list[i]);
		if (band == unit && sunit == subunit)
		{
			p = &list[i];
			break;
		}
	}

	return p;
}

int compare (const void * a, const void * b)
{
  return ( *(int*)a - *(int*)b );
}

int wgn_vlan_list_check_subunit_conflict(
	wgn_vlan_rule *list, 
	size_t list_size,
	int unit,
	int subunit)
{
	int band, sunit, count = 0;
	int v[WGN_MAX_NO_GUEST_NETWORK];
	int i, j;
	int ret = 0;

	if (!list || list_size <= 0 || unit < 0 || subunit < 1)
		return 0;

	memset(v, 0, (sizeof(int) * WGN_MAX_NO_GUEST_NETWORK));
	for (i=0, count=0; i<list_size && count<WGN_MAX_NO_GUEST_NETWORK; i++)
	{
		band = sunit = -1;
		wgn_get_wl_unit(&band, &sunit, &list[i]);
		if (band == unit && sunit > 0)
		{
			v[count] = sunit;
			count++;
		}
	}

	qsort(v, count, sizeof(int), compare);
	for (i=0; i<count; i++)
	{
		if (v[i] == subunit)
		{
			v[i] = v[i] + 1;
			ret = 1;
		}
	}

	if (ret == 1)
	{
		for (i=0; i<count; i++)
		{
			subunit = v[i];
			for (j=i+1; j<count; j++)
			{
				if (v[j] == subunit)
				{
					v[j] = v[j] + 1;
				}
			}
		}

		for (i=0, j=0; i<list_size && j<count; i++)
		{
			band = sunit = -1;
			wgn_get_wl_unit(&band, &sunit, &list[i]);
			if (band == unit && sunit > 0)
			{
				wgn_set_wl_unit(band, v[j], &list[i]);
				j++;
			}
		}
	}

	return ret;
}

void wgn_get_wl_unit(
	int *result_unit,
	int *result_subunit,
	wgn_vlan_rule *vlan_rule)
{
	int i;
	int unit = -1;
	int subunit;
	int wlset = 0;
	struct wgn_vlan_rule_t *p;

	if (result_unit)
		*result_unit = -1;

	if (result_subunit)
		*result_subunit = -1;

	if (!(p = vlan_rule))
		return;

	if (p->wl2gset && strlen(p->wl2gset) == 4 && (wlset = strtoul(p->wl2gset, NULL, 10)) > 0)
		unit = 0;
	else if (p->wl5gset && strlen(p->wl5gset) == 4 && (wlset = strtoul(p->wl5gset,  NULL, 10)) > 0)
		unit = 1;
	else if (p->wl5gset && strlen(p->wl5gset) == 8 && (wlset = strtoul(&p->wl5gset[4], NULL, 10)) > 0)
		unit = 2;
	else
		return;

	for (subunit=0, i=0; i<16; i++, subunit++)
	{
		if (!(wlset & 1 << i))
			continue;
		break;
	}

	if (result_unit)
		*result_unit = unit;

	if (result_subunit)
		*result_subunit = subunit;

	return;
}

void wgn_set_wl_unit(
	int unit, 
	int subunit,
	wgn_vlan_rule *vlan_rule)
{
	struct wgn_vlan_rule_t *p;

	if (!(p = vlan_rule))
		return;

	if (subunit < 1 || subunit > 15)
		return;

	switch (unit)
	{
		case 0:
			memset(p->wl2gset, 0, sizeof(p->wl2gset));
			snprintf(p->wl2gset, sizeof(p->wl2gset), "%04X", (1 << subunit));
			break;
		case 1:
			memset(p->wl5gset, 0, sizeof(p->wl5gset));
			snprintf(p->wl5gset, sizeof(p->wl5gset), "%04X", (1 << subunit));
			break;
		case 2:
			memset(p->wl5gset, 0, sizeof(p->wl5gset));
			snprintf(p->wl5gset, sizeof(p->wl5gset), "%04X%04X", 0, (1 << subunit));
			break;
	}

	return;
}

int wgn_guest_ifcount(
	int band)
{
	int count = 0, unit = -1, subunit = -1;
	size_t i = 0;

	struct wgn_vlan_rule_t vlan_list[WGN_MAX_NO_GUEST_NETWORK];
	size_t vlan_total = 0;

	memset(vlan_list, 0, sizeof(struct wgn_vlan_rule_t) * WGN_MAX_NO_GUEST_NETWORK);
	if (!wgn_vlan_list_get_from_nvram(vlan_list, WGN_MAX_NO_GUEST_NETWORK, &vlan_total))
		return count;
	
	for (i=0, count=0; i<vlan_total; i++)
	{
		unit = subunit = -1;
		wgn_get_wl_unit(&unit,&subunit,&vlan_list[i]);
		if (unit < 0 || subunit <= 0)
			continue;
		if (unit == band)
			count++;		
	}

	return count;
}

char *wgn_guest_ifnames(
	int band,
	int total,
	char *ret_ifnames, 
	size_t ifnames_bsize)
{
	int unit = -1, subunit = -1, count = 0;
	size_t i = 0, offset = 0;

	struct wgn_vlan_rule_t vlan_list[WGN_MAX_NO_GUEST_NETWORK];
	size_t vlan_total = 0;

	char s[33];

	if (!ret_ifnames || ifnames_bsize <= 0)
		return NULL;

	memset(ret_ifnames, 0, ifnames_bsize);
	memset(vlan_list, 0, sizeof(struct wgn_vlan_rule_t) * WGN_MAX_NO_GUEST_NETWORK);
	if (!wgn_vlan_list_get_from_nvram(vlan_list, WGN_MAX_NO_GUEST_NETWORK, &vlan_total))
		return NULL;

	if (total <= 0 || total > vlan_total)
		total = vlan_total; 

	for (i=0, offset=0, count=0; i<vlan_total && offset<ifnames_bsize && count<total; i++)
	{
		unit = subunit = -1;
		wgn_get_wl_unit(&unit, &subunit, &vlan_list[i]);
		if (unit == band && subunit > 0)
		{
			memset(s, 0, sizeof(s));
			snprintf(s, sizeof(s), "wl%d.%d ", band, subunit);
			strlcat(ret_ifnames, s, ifnames_bsize);
			offset += strlen(s);
			count++;
		}
	}

	if (strlen(ret_ifnames) > 0)
		ret_ifnames[strlen(ret_ifnames) - 1] = '\0';

	return (offset > 0 && offset <= ifnames_bsize) ? ret_ifnames : NULL;
}

char *wgn_guest_vlans(
	char *ifnames, 
	char *ret_vlans,
	size_t vlans_bsize)
{
	int unit = -1, subunit = -1, unit2 = -1, subunit2 = -1;
	char word[64], *next = NULL;
	size_t i = 0, offset = 0;

	struct wgn_vlan_rule_t vlan_list[WGN_MAX_NO_GUEST_NETWORK];
	size_t vlan_total = 0;

	char s[33];

	if (!ifnames || !ret_vlans || vlans_bsize <= 0)
		return NULL;

	memset(ret_vlans, 0, vlans_bsize);
	memset(vlan_list, 0, sizeof(struct wgn_vlan_rule_t) * WGN_MAX_NO_GUEST_NETWORK);
	if (!wgn_vlan_list_get_from_nvram(vlan_list, WGN_MAX_NO_GUEST_NETWORK, &vlan_total))
		return NULL;

	if (vlan_total > 0)
	{
		foreach(word, ifnames, next)
		{
			unit = subunit = -1;
			sscanf(word, "wl%d.%d_%*s", &unit, &subunit);
			if (unit < 0 || subunit <= 0)
				continue;
			for (i=0, offset=0; i<vlan_total && offset<vlans_bsize; i++)
			{
				unit2 = subunit2 = -1;
				wgn_get_wl_unit(&unit2,&subunit2,&vlan_list[i]);
				if (unit == unit2 && subunit == subunit2)
				{
					memset(s, 0, sizeof(s));
					snprintf(s, sizeof(s), "%d ", vlan_list[i].vid);
					strlcat(ret_vlans, s, vlans_bsize);
					offset += strlen(s);
					break;					
				}
			}
		}
	}

	if (strlen(ret_vlans) > 0)
		ret_vlans[strlen(ret_vlans) - 1] = '\0';

	return (offset > 0 && offset <= vlans_bsize) ? ret_vlans : NULL;
}

