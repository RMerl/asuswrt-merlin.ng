#include "mtlan_utils.h"

#define OVPN_SERVER_BASE 20

/*******************************
* IP Calculate
*******************************/
static unsigned defv4mask(char *ip)
{
	struct in_addr net;

	if (inet_pton(AF_INET, ip, &net) <= 0) {
		return 24;
	}
	unsigned byte1 = (ntohl(net.s_addr) >> 24) & 0xff;

	if (byte1 < 128)
		return 8;
	if (byte1 >= 128 && byte1 < 192)
		return 16;
	if (byte1 >= 192 && byte1 < 224)
		return 24;

	return 24;
}

static int bit_count(uint32_t i)
{
	int c = 0;
	unsigned int seen_one = 0;

	while (i > 0) {
		if (i & 1) {
			seen_one = 1;
			c++;
		} else {
			if (seen_one) {
				return -1;
			}
		}
		i >>= 1;
	}

	return c;
}

/*!
  \fn uint32_t prefix2mask(int bits)
  \brief creates a netmask from a specified number of bits
  This function converts a prefix length to a netmask.  As CIDR (classless
  internet domain internet domain routing) has taken off, more an more IP
  addresses are being specified in the format address/prefix
  (i.e. 192.168.2.3/24, with a corresponding netmask 255.255.255.0).  If you
  need to see what netmask corresponds to the prefix part of the address, this
  is the function.  See also \ref mask2prefix.
  \param prefix is the number of bits to create a mask for.
  \return a network mask, in network byte order.
*/
uint32_t prefix2mask(int prefix)
{
	struct in_addr mask;
	memset(&mask, 0, sizeof(mask));
	if (prefix) {
		return htonl(~((1 << (32 - prefix)) - 1));
	} else {
		return htonl(0);
	}
}

/*!
  \fn int mask2prefix(struct in_addr mask)
  \brief calculates the number of bits masked off by a netmask.
  This function calculates the significant bits in an IP address as specified by
  a netmask.  See also \ref prefix2mask.
  \param mask is the netmask, specified as an struct in_addr in network byte order.
  \return the number of significant bits.  */
static int mask2prefix(struct in_addr mask)
{
	return bit_count(ntohl(mask.s_addr));
}

static int v4mask2int(const char *prefix)
{
	int ret;
	struct in_addr in;

	ret = inet_pton(AF_INET, prefix, &in);
	if (ret == 0)
		return -1;

	return mask2prefix(in);
}

static struct in_addr calc_network(struct in_addr addr, int prefix)
{
	struct in_addr mask;
	struct in_addr network;

	mask.s_addr = prefix2mask(prefix);

	memset(&network, 0, sizeof(network));
	network.s_addr = addr.s_addr & mask.s_addr;
	return network;
}

static char *calc_subnet(char *addr, unsigned int prefix, char *ret, size_t retsz)
{
        struct in_addr ip, network;
        char subnet[INET_ADDRSTRLEN];

	if (!ret || !addr)
		return NULL;
	if (prefix > 32)
		prefix = defv4mask(addr);

	inet_pton(AF_INET, addr, &ip);
	network = calc_network(ip, prefix);
	inet_ntop(AF_INET, &network, subnet, INET_ADDRSTRLEN);
	strlcpy(ret, subnet, retsz);

	return ret;
}

/*******************************
* Get Nvram to Sturct
*******************************/
void *INIT_MTLAN(const unsigned int sz)
{
	void *p = calloc(MTLAN_MAXINUM, sz);
	if (!p)
		return NULL;
	return p;
}
void FREE_MTLAN(void *p)
{
	if(p) free(p);
}

VLAN_T *get_mtvlan(VLAN_T *vlst, size_t *sz, const int is_rm)
{
	char *nv = NULL, *nvp = NULL, *b;

	/* basic necessary parameters */
	char *idx, *vid;
	/* continue to add parameters */
	char *port_isolation = NULL;

	size_t cnt = 0;

	if (sz)
		*sz = 0;

	if (!(nv = nvp = strdup((!is_rm) ? nvram_safe_get("vlan_rl") : nvram_safe_get("vlan_rl_x"))))
		return NULL;

	while ((b = strsep(&nvp, "<")) != NULL) {
		if (vstrsep(b, ">", &idx, &vid, &port_isolation) < VLAN_LIST_BASIC_PARAM)
			continue;

		if (cnt >= MTLAN_MAXINUM)
			break;

		if (idx && *idx)
			vlst[cnt].idx = strtol(idx, NULL, 10);
		if (vid && *vid)
			vlst[cnt].vid = strtol(vid, NULL, 10);
		if (port_isolation && *port_isolation)
			vlst[cnt].port_isolation = strtol(port_isolation, NULL, 10);

		cnt++;
	}

	*sz = cnt;

	free(nv);
	return vlst;
}

SUBNET_T *get_mtsubnet(SUBNET_T *sublst, size_t *lst_sz, const int is_rm)
{
	char *nv = NULL, *nvp = NULL, *b;

	/* basic necessary parameters */
	char *idx, *ifname, *addr, *netmask;
	char *dhcp_enable, *dhcp_min, *dhcp_max, *dhcp_lease;
	char *domain_name, *dns, *wins, *dhcp_res, *dhcp_res_idx;

	/* continue to add parameters */
	char *v6_enable = NULL, *v6_autoconf = NULL, *addr6 = NULL;
	char *dhcp6_min = NULL, *dhcp6_max = NULL, *dns6 = NULL;
	char *dot_enable = NULL, *dot_tls = NULL;

	char subnet[INET_ADDRSTRLEN];

	int dnscnt = 0;
	char *duptr, *subptr, *dnstr;

	size_t cnt = 0;

	if (lst_sz)
		*lst_sz = 0;

	if (!(nv = nvp = strdup((!is_rm) ? nvram_safe_get("subnet_rl") : nvram_safe_get("subnet_rl_x"))))
		return NULL;

	while ((b = strsep(&nvp, "<")) != NULL) {
		/* init */
		v6_enable = v6_autoconf = addr6 = NULL;
		dhcp6_min = dhcp6_max = dns6 = NULL;

		if (vstrsep(b, ">", &idx, &ifname, &addr, &netmask, &dhcp_enable, &dhcp_min, &dhcp_max,
			            &dhcp_lease, &domain_name, &dns, &wins, &dhcp_res, &dhcp_res_idx,
			            &v6_enable, &v6_autoconf, &addr6, &dhcp6_min, &dhcp6_max, &dns6,
				    &dot_enable, &dot_tls) < SUBNET_LIST_BASIC_PARAM)
			continue;

		if (cnt >= MTLAN_MAXINUM)
			break;

		if (idx && *idx)
			sublst[cnt].idx = strtol(idx, NULL, 10);
		if (ifname && *ifname) {
			strlcpy(sublst[cnt].ifname, ifname, sizeof(sublst[cnt].ifname));
			strlcpy(sublst[cnt].br_ifname, ifname, sizeof(sublst[cnt].br_ifname));
		}
		if (addr && *addr)
			strlcpy(sublst[cnt].addr, addr, sizeof(sublst[cnt].addr));
		if (netmask && *netmask)
			strlcpy(sublst[cnt].netmask, netmask, sizeof(sublst[cnt].netmask));
		if (dhcp_enable && *dhcp_enable)
			sublst[cnt].dhcp_enable = (strtol(dhcp_enable, NULL, 10) == 0) ? 0 : 1;
		if (dhcp_min && *dhcp_min)
			strlcpy(sublst[cnt].dhcp_min, dhcp_min, sizeof(sublst[cnt].dhcp_min));
		if (dhcp_max && *dhcp_max)
			strlcpy(sublst[cnt].dhcp_max, dhcp_max, sizeof(sublst[cnt].dhcp_max));
		if (dhcp_lease && *dhcp_lease)
			sublst[cnt].dhcp_lease = strtol(dhcp_lease, NULL, 10);
		if (domain_name && *domain_name)
			strlcpy(sublst[cnt].domain_name, domain_name, sizeof(sublst[cnt].domain_name));
		if (dns && *dns) {
			dnscnt = 0;
			duptr = subptr = strdup(dns);
			if (duptr) {
				while ((dnstr = strsep(&subptr, ",")) != NULL) {
					if (dnscnt > 1)
						break;
					strlcpy(sublst[cnt].dns[dnscnt], dnstr, sizeof(sublst[cnt].dns[dnscnt]));
					dnscnt++;
				}
				free(duptr);
			}
		}
		if (wins && *wins)
			strlcpy(sublst[cnt].wins, wins, sizeof(sublst[cnt].wins));
		if (dhcp_res && *dhcp_res)
			sublst[cnt].dhcp_res = strtol(dhcp_res, NULL, 10);
		if (dhcp_res && *dhcp_res_idx)
			sublst[cnt].dhcp_res_idx = strtol(dhcp_res_idx, NULL, 10);

		strlcpy(sublst[cnt].subnet, calc_subnet(addr, v4mask2int(netmask), subnet, sizeof(subnet)), sizeof(sublst[cnt].subnet));
		sublst[cnt].prefixlen = v4mask2int(netmask);

		if (dot_enable && *dot_enable)
			sublst[cnt].dot_enable = (strtol(dot_enable, NULL, 10) == 0) ? 0 : 1;
		if (dot_tls && *dot_tls)
			sublst[cnt].dot_tls = (strtol(dot_tls, NULL, 10) == 0) ? 0 : 1;

		if (v6_enable && *v6_enable)
			sublst[cnt].v6_enable = strtol(v6_enable, NULL, 10);
		if (v6_autoconf && *v6_autoconf)
			sublst[cnt].v6_autoconf = strtol(v6_autoconf, NULL, 10);
		if (addr6 && *addr6)
			strlcpy(sublst[cnt].addr6, addr6, sizeof(sublst[cnt].addr6));
		if (dhcp6_min && *dhcp6_min)
			strlcpy(sublst[cnt].dhcp6_min, dhcp6_min, sizeof(sublst[cnt].dhcp6_min));
		if (dhcp6_max && *dhcp6_max)
			strlcpy(sublst[cnt].dhcp6_max, dhcp6_max, sizeof(sublst[cnt].dhcp6_max));
		if (dns6 && *dns6) {
			dnscnt = 0;
			duptr = subptr = strdup(dns6);
			if (duptr) {
				while ((dnstr = strsep(&subptr, ",")) != NULL) {
					if (dnscnt > 2)
						break;
					strlcpy(sublst[cnt].dns6[dnscnt], dnstr, sizeof(sublst[cnt].dns6[dnscnt]));
					dnscnt++;
				}
				free(duptr);
			}
		}

		cnt++;
	}

	*lst_sz = cnt;

	free(nv);
	return sublst;
}

static SUBNET_T *get_default_lan(SUBNET_T *ptr)
{
	char subnet[INET_ADDRSTRLEN];

	if (!ptr)
		return NULL;

	memset(ptr, 0, sizeof(SUBNET_T));

	/* IPv4 */
	strlcpy(ptr->ifname, nvram_safe_get("lan_ifname"), sizeof(ptr->ifname));
	strlcpy(ptr->br_ifname, ptr->ifname, sizeof(ptr->br_ifname));
	strlcpy(ptr->addr, nvram_safe_get("lan_ipaddr"), sizeof(ptr->addr));
	strlcpy(ptr->netmask, nvram_safe_get("lan_netmask"), sizeof(ptr->netmask));
	strlcpy(ptr->subnet, calc_subnet(ptr->addr, v4mask2int(ptr->netmask), subnet, sizeof(subnet)), sizeof(ptr->subnet));
	ptr->prefixlen = v4mask2int(nvram_safe_get("lan_netmask"));
	ptr->dhcp_enable = nvram_get_int("dhcp_enable_x");
	strlcpy(ptr->dhcp_min, nvram_safe_get("dhcp_start"), sizeof(ptr->dhcp_min));
	strlcpy(ptr->dhcp_max, nvram_safe_get("dhcp_end"), sizeof(ptr->dhcp_max));
	ptr->dhcp_lease = nvram_get_int("dhcp_lease");
	strlcpy(ptr->domain_name, nvram_safe_get("lan_domain"), sizeof(ptr->domain_name));
	strlcpy(ptr->dns[0], nvram_safe_get("dhcp_dns1_x"), sizeof(ptr->dns[0]));
	strlcpy(ptr->dns[1], nvram_safe_get("dhcp_dns2_x"), sizeof(ptr->dns[1]));
	strlcpy(ptr->wins, nvram_safe_get("dhcp_wins_x"), sizeof(ptr->wins));
	ptr->dhcp_res = nvram_get_int("dhcp_static_x");
	ptr->dhcp_res_idx = 0; /* no reference */
	ptr->dot_enable = nvram_get_int("dnspriv_enable");
	ptr->dot_tls = nvram_get_int("dnspriv_profile");

	/* IPv6 */
	ptr->v6_enable = nvram_invmatch(ipv6_nvname("ipv6_service"), "disabled");
	ptr->v6_autoconf = nvram_get_int("ipv6_autoconf_type");

	return ptr;
}

static SUBNET_T *get_subnet_by_idx(int idx, SUBNET_T *lst, size_t lst_sz)
{
	int i;
	SUBNET_T *ptr = NULL;

	if (!lst)
		return NULL;
	for (i = 0; i < lst_sz; i++) {
		if (lst[i].idx == idx) {
			ptr = &lst[i];
			return ptr;
		}
	}
	return NULL;
}

static VLAN_T *get_vlan_by_idx(int idx, VLAN_T *lst, size_t lst_sz)
{
	int i;
	VLAN_T *ptr = NULL;

	if (!lst)
		return NULL;
	for (i = 0; i < lst_sz; i++) {
		if (lst[i].idx == idx) {
			ptr = &lst[i];
			return ptr;
		}
	}
	return NULL;
}

static MTLAN_T *__get_mtlan(MTLAN_T *nwlst, size_t *lst_sz, int is_rm)
{
	char *nv = NULL, *nvp = NULL, *b;

	/* basic necessary parameters */
	char *sdn_idx, *sdn_name, *sdn_enable;
	char *vlan_idx, *subnet_idx, *apg_idx;

	/* continue to add parameters */
	char *vpnc_idx = NULL, *vpns_idx = NULL, *dnsf_idx = NULL, *urlf_idx = NULL, *nwf_idx = NULL;
	char *cp_idx = NULL, *gre_idx = NULL, *fw_idx = NULL, *killsw_sw = NULL, *ahs_sw = NULL;
	char *wan_idx = NULL, *ppprelay_sw = NULL, *wan6_idx = NULL, *createby= NULL, *mtwan_idx = NULL;
	char *mswan_idx = NULL;

	char idx[4], *next;

	size_t vlan_sz = 0, subnet_sz = 0;
	size_t cnt = 0;

	int __cnt = 0;

	if (lst_sz)
		*lst_sz = 0;

	if (!nwlst)
		return NULL;

	VLAN_T   *pvlan_rl = (VLAN_T *)INIT_MTLAN(sizeof(VLAN_T));
	SUBNET_T *psubnet_rl = (SUBNET_T *)INIT_MTLAN(sizeof(SUBNET_T));

	if (!pvlan_rl || !psubnet_rl) {
		nwlst = NULL;
		goto MTEND;
	}

	if (!get_mtvlan(pvlan_rl, &vlan_sz, is_rm)) {
		nwlst = NULL;
		goto MTEND;
	}
	if (!get_mtsubnet(psubnet_rl, &subnet_sz, is_rm)) {
		nwlst = NULL;
		goto MTEND;
	}
	if (!(nvp = nv = strdup((!is_rm) ? nvram_safe_get("sdn_rl") : nvram_safe_get("sdn_rl_x")))) {
		nwlst = NULL;
		goto MTEND;
	}


	while ((b = strsep(&nvp, "<")) != NULL) {

		/* init */
		vpnc_idx = vpns_idx = dnsf_idx = urlf_idx = nwf_idx = NULL;
		cp_idx = gre_idx = fw_idx = killsw_sw = ahs_sw = NULL;
		wan_idx = ppprelay_sw = wan6_idx = createby = mtwan_idx = NULL;
		mswan_idx = NULL;

		if (vstrsep(b, ">", &sdn_idx, &sdn_name, &sdn_enable, &vlan_idx, &subnet_idx,
				    &apg_idx, &vpnc_idx, &vpns_idx, &dnsf_idx,
				    &urlf_idx, &nwf_idx, &cp_idx, &gre_idx, &fw_idx,
				    &killsw_sw, &ahs_sw, &wan_idx, &ppprelay_sw, &wan6_idx,
				    &createby, &mtwan_idx, &mswan_idx) < SDN_LIST_BASIC_PARAM)
			continue;

		if (cnt >= MTLAN_MAXINUM)
			break;

		if (!((vlan_idx && *vlan_idx) && (subnet_idx && *subnet_idx)))
			continue;

		if (strtol(vlan_idx, NULL, 10) > 0 && strtol(subnet_idx, NULL, 10) > 0) {

			VLAN_T   *pvlan_rl_idx = get_vlan_by_idx(strtol(vlan_idx, NULL, 10), pvlan_rl, vlan_sz);
			SUBNET_T *psubnet_rl_idx = get_subnet_by_idx(strtol(subnet_idx, NULL, 10), psubnet_rl, subnet_sz);

			if (!pvlan_rl_idx || !psubnet_rl_idx)
				continue;

			memcpy(&nwlst[cnt].vid, &pvlan_rl_idx->vid, sizeof(nwlst[cnt].vid));
			memcpy(&nwlst[cnt].port_isolation, &pvlan_rl_idx->port_isolation, sizeof(nwlst[cnt].port_isolation));
			memcpy(&nwlst[cnt].nw_t, psubnet_rl_idx, sizeof(nwlst[cnt].nw_t));

		} else { /* Get Default LAN Info */

			if (!get_default_lan(&nwlst[cnt].nw_t))
				continue;
			nwlst[cnt].vid = 0;
			nwlst[cnt].port_isolation = 0;

			if (sdn_idx && *sdn_idx && strtol(sdn_idx, NULL, 10))	// Special case. IoT use br0. Follow LAN IPv6 flow.
				nwlst[cnt].nw_t.v6_enable = 0;
		}

		if (sdn_enable && *sdn_enable)
			nwlst[cnt].enable = strtol(sdn_enable, NULL, 10);
		if (sdn_name && *sdn_name)
			strlcpy(nwlst[cnt].name, sdn_name, sizeof(nwlst[cnt].name));
		if (createby && *createby)
			strlcpy(nwlst[cnt].createby, createby, sizeof(nwlst[cnt].createby));
		if (sdn_idx && *sdn_idx)
			nwlst[cnt].sdn_t.sdn_idx = strtol(sdn_idx, NULL, 10);
		if (apg_idx && *apg_idx)
			nwlst[cnt].sdn_t.apg_idx = strtol(apg_idx, NULL, 10);
		if (vpnc_idx && *vpnc_idx)
			nwlst[cnt].sdn_t.vpnc_idx = strtol(vpnc_idx, NULL, 10);
		if (vpns_idx && *vpns_idx) {
			__cnt = 0;
			__foreach (idx, vpns_idx, next, ",") {
				if (__cnt < MTLAN_VPNS_MAXINUM) {
					nwlst[cnt].sdn_t.vpns_idx_rl[__cnt] = strtol(idx, NULL, 10);
					__cnt++;
				}
			}
		}
		if (dnsf_idx && *dnsf_idx)
			nwlst[cnt].sdn_t.dnsf_idx = strtol(dnsf_idx, NULL, 10);
		if (urlf_idx && *urlf_idx)
			nwlst[cnt].sdn_t.urlf_idx = strtol(urlf_idx, NULL, 10);
		if (nwf_idx && *nwf_idx)
			nwlst[cnt].sdn_t.nwf_idx = strtol(nwf_idx, NULL, 10);
		if (cp_idx && *cp_idx) {
			nwlst[cnt].sdn_t.cp_idx = strtol(cp_idx, NULL, 10);
			// update chilli interface
			switch (nwlst[cnt].sdn_t.cp_idx) {
				case 2:
				case 4:
					strlcpy(nwlst[cnt].nw_t.ifname, "tun22", sizeof(nwlst[cnt].nw_t.ifname));
					break;
				case 1:
					strlcpy(nwlst[cnt].nw_t.ifname, "tun23", sizeof(nwlst[cnt].nw_t.ifname));
					break;
			}
		}
		if (gre_idx && *gre_idx) {
			__cnt = 0;
			__foreach (idx, gre_idx, next, ",") {
				if (__cnt < MTLAN_GRE_MAXINUM) {
					nwlst[cnt].sdn_t.gre_idx_rl[__cnt] = strtol(idx, NULL, 10);
					__cnt++;
				}
			}
		}
		if (fw_idx && *fw_idx)
			nwlst[cnt].sdn_t.fw_idx = strtol(fw_idx, NULL, 10);
		if (killsw_sw && *killsw_sw)
			nwlst[cnt].sdn_t.killsw_sw = strtol(killsw_sw, NULL, 10);
		if (ahs_sw && *ahs_sw)
			nwlst[cnt].sdn_t.ahs_sw = strtol(ahs_sw, NULL, 10);
		if (wan_idx && *wan_idx)
			nwlst[cnt].sdn_t.wan_idx = strtol(wan_idx, NULL, 10);
		if (ppprelay_sw && *ppprelay_sw)
			nwlst[cnt].sdn_t.ppprelay_sw = strtol(ppprelay_sw, NULL, 10);
		if (wan6_idx && *wan6_idx)
			nwlst[cnt].sdn_t.wan6_idx = strtol(wan6_idx, NULL, 10);
		if (mtwan_idx && *mtwan_idx)
			nwlst[cnt].sdn_t.mtwan_idx = strtol(mtwan_idx, NULL, 10);
		if (mswan_idx && *mswan_idx)
			nwlst[cnt].sdn_t.mswan_idx = strtol(mswan_idx, NULL, 10);
		cnt++;
	}

	*lst_sz = cnt;

	free(nv);

MTEND:
	FREE_MTLAN((void *)pvlan_rl);
	FREE_MTLAN((void *)psubnet_rl);
	return nwlst;
}

static MTLAN_T *__get_mtlan_by_vid(const unsigned int vid, MTLAN_T *nwlst, size_t *lst_sz, const int is_rm)
{
	int i;
	size_t r = 0, mtl_sz = 0;

	MTLAN_T *pmtl = (MTLAN_T *)INIT_MTLAN(sizeof(MTLAN_T));

	if (lst_sz)
		*lst_sz = 0;

	if (!is_rm) {
		if (!__get_mtlan(pmtl, &mtl_sz, 0)) {
			FREE_MTLAN((void *)pmtl);
			return NULL;
		}
	} else {
		if (!__get_mtlan(pmtl, &mtl_sz, 1)) {
			FREE_MTLAN((void *)pmtl);
			return NULL;
		}
	}

	for (i = 0; i < mtl_sz; i++) {
		if (pmtl[i].vid == vid) {
			memcpy(&nwlst[r], &pmtl[i], sizeof(MTLAN_T));
			r++;
		}
	}

	*lst_sz = r;
	FREE_MTLAN((void *)pmtl);
	return nwlst;
}

static MTLAN_T *__get_mtlan_by_idx(SDNFT_TYPE type, const unsigned int idx, MTLAN_T *nwlst, size_t *lst_sz, const int is_rm)
{
	int i, j;
	size_t r = 0;
	size_t mtl_sz = 0;

	if (lst_sz)
		*lst_sz = 0;

	if (type >= SDNFT_TYPE_MAX)
		return NULL;

	MTLAN_T *pmtl = (MTLAN_T *)INIT_MTLAN(sizeof(MTLAN_T));

	if (!is_rm) {
		if (!__get_mtlan(pmtl, &mtl_sz, 0)) {
			FREE_MTLAN((void *)pmtl);
			return NULL;
		}
	} else {
		if (!__get_mtlan(pmtl, &mtl_sz, 1)) {
			FREE_MTLAN((void *)pmtl);
			return NULL;
		}
	}

	for (i = 0; i < mtl_sz; i++) {
		if (type == SDNFT_TYPE_SDN) {
			if (pmtl[i].sdn_t.sdn_idx == idx) {
				memcpy(&nwlst[r], &pmtl[i], sizeof(MTLAN_T));
				r++;
			}
		} else if (type == SDNFT_TYPE_APG) {
			if (pmtl[i].sdn_t.apg_idx == idx) {
				memcpy(&nwlst[r], &pmtl[i], sizeof(MTLAN_T));
				r++;
			}
		} else if (type == SDNFT_TYPE_VPNC) {
			if (pmtl[i].sdn_t.vpnc_idx == idx) {
				memcpy(&nwlst[r], &pmtl[i], sizeof(MTLAN_T));
				r++;
			}
		} else if (type == SDNFT_TYPE_VPNS) {
			for (j = 0; j < MTLAN_VPNS_MAXINUM; j++) {
				if (pmtl[i].sdn_t.vpns_idx_rl[j] == idx) {
					memcpy(&nwlst[r], &pmtl[i], sizeof(MTLAN_T));
					r++;
					break;
				}
			}
		} else if (type == SDNFT_TYPE_URLF) {
			if (pmtl[i].sdn_t.urlf_idx == idx) {
				memcpy(&nwlst[r], &pmtl[i], sizeof(MTLAN_T));
				r++;
			}
		} else if (type == SDNFT_TYPE_NWF) {
			if (pmtl[i].sdn_t.nwf_idx == idx) {
				memcpy(&nwlst[r], &pmtl[i], sizeof(MTLAN_T));
				r++;
			}
		} else if (type == SDNFT_TYPE_GRE) {
			for (j = 0; j < MTLAN_GRE_MAXINUM; j++) {
				if (pmtl[i].sdn_t.gre_idx_rl[j] == idx) {
					memcpy(&nwlst[r], &pmtl[i], sizeof(MTLAN_T));
					r++;
					break;
				}
			}
		} else if (type == SDNFT_TYPE_FW) {
			if (pmtl[i].sdn_t.fw_idx == idx) {
				memcpy(&nwlst[r], &pmtl[i], sizeof(MTLAN_T));
				r++;
			}
		} else if (type == SDNFT_TYPE_KILLSW) {
			if (pmtl[i].sdn_t.killsw_sw == idx) {
				memcpy(&nwlst[r], &pmtl[i], sizeof(MTLAN_T));
				r++;
			}
		} else if (type == SDNFT_TYPE_AHS) {
			if (pmtl[i].sdn_t.ahs_sw == idx) {
				memcpy(&nwlst[r], &pmtl[i], sizeof(MTLAN_T));
				r++;
			}
		} else if (type == SDNFT_TYPE_WAN) {
			if (pmtl[i].sdn_t.wan_idx == idx) {
				memcpy(&nwlst[r], &pmtl[i], sizeof(MTLAN_T));
				r++;
			}
		} else if (type == SDNFT_TYPE_PPPRELAY) {
			if (pmtl[i].sdn_t.ppprelay_sw == idx) {
				memcpy(&nwlst[r], &pmtl[i], sizeof(MTLAN_T));
				r++;
			}
		} else if (type == SDNFT_TYPE_WAN6) {
			if (pmtl[i].sdn_t.wan6_idx == idx) {
				memcpy(&nwlst[r], &pmtl[i], sizeof(MTLAN_T));
				r++;
			}
		} else if (type == SDNFT_TYPE_MTWAN) {
			if (pmtl[i].sdn_t.mtwan_idx == idx) {
				memcpy(&nwlst[r], &pmtl[i], sizeof(MTLAN_T));
				r++;
			}
		} else if (type == SDNFT_TYPE_MSWAN) {
			if (pmtl[i].sdn_t.mswan_idx == idx) {
				memcpy(&nwlst[r], &pmtl[i], sizeof(MTLAN_T));
				r++;
			}
		}
	}

	*lst_sz = r;
	FREE_MTLAN((void *)pmtl);

	return nwlst;
}

MTLAN_T *get_mtlan(MTLAN_T *nwlst, size_t *lst_sz)
{
	return __get_mtlan(nwlst, lst_sz, 0);
}

MTLAN_T *get_rm_mtlan(MTLAN_T *nwlst, size_t *lst_sz)
{
	return __get_mtlan(nwlst, lst_sz, 1);
}

MTLAN_T *get_mtlan_by_vid(const unsigned int vid, MTLAN_T *nwlst, size_t *lst_sz)
{
	return __get_mtlan_by_vid(vid, nwlst, lst_sz, 0);
}

MTLAN_T *get_rm_mtlan_by_vid(const unsigned int vid, MTLAN_T *nwlst, size_t *lst_sz)
{
	return __get_mtlan_by_vid(vid, nwlst, lst_sz, 1);
}

MTLAN_T *get_mtlan_by_idx(SDNFT_TYPE type, const unsigned int idx, MTLAN_T *nwlst, size_t *lst_sz)
{
	return __get_mtlan_by_idx(type, idx, nwlst, lst_sz, 0);
}

MTLAN_T *get_rm_mtlan_by_idx(SDNFT_TYPE type, const unsigned int idx, MTLAN_T *nwlst, size_t *lst_sz)
{
	return __get_mtlan_by_idx(type, idx, nwlst, lst_sz, 1);
}

size_t get_mtlan_cnt()
{
	char *nv = NULL, *nvp = NULL, *b;
	size_t cnt = 0;
	if (!(nvp = nv = strdup(nvram_safe_get("sdn_rl"))))
		return 0;
	while ((b = strsep(&nvp, "<")) != NULL)
		cnt++;
	free(nv);
	return (cnt - 1);
}

size_t get_rm_mtlan_cnt()
{
	char *nv = NULL, *nvp = NULL, *b;
	size_t cnt = 0;
	if (!(nvp = nv = strdup(nvram_safe_get("sdn_rl_x"))))
		return 0;
	while ((b = strsep(&nvp, "<")) != NULL)
		cnt++;
	free(nv);
	return cnt;
}

int sdn_enable(void)
{
	int i, enable = 0;
	size_t  mtl_sz = 0;
	MTLAN_T *pmtl = (MTLAN_T *)INIT_MTLAN(sizeof(MTLAN_T));

	if (get_mtlan(pmtl, &mtl_sz) && mtl_sz > 0) {
		/* skip DEFAULT */
		for (i = 1; i < mtl_sz; i++) {
			if (pmtl[i].enable) {
				enable = 1;
				break;
			}
		}
	}
	FREE_MTLAN((void *)pmtl);

	return enable;
}

static VPN_PROTO_T _get_real_vpn_proto(const char* proto)
{
	if (!proto)
		return VPN_PROTO_UNKNOWN;
	else if (!strcmp(proto, VPN_PROTO_OVPN_STR))
		return VPN_PROTO_OVPN;
	else if (!strcmp(proto, VPN_PROTO_PPTP_STR))
		return VPN_PROTO_PPTP;
	else if (!strcmp(proto, VPN_PROTO_L2TP_STR))
		return VPN_PROTO_L2TP;
	else if (!strcmp(proto, VPN_PROTO_WG_STR))
		return VPN_PROTO_WG;
	else if (!strcmp(proto, VPN_PROTO_HMA_STR))
		return VPN_PROTO_OVPN;
	else if (!strcmp(proto, VPN_PROTO_NORDVPN_STR))
		return VPN_PROTO_WG;
	else if (!strcmp(proto, VPN_PROTO_IPSEC_STR))
		return VPN_PROTO_IPSEC;
	else if (!strcmp(proto, VPN_PROTO_SURFSHARK_STR))
		return VPN_PROTO_WG;
	else if (!strcmp(proto, VPN_PROTO_CYBERGHOST_STR))
		return VPN_PROTO_OVPN;
	else
		return VPN_PROTO_UNKNOWN;
}

static char* _get_vpn_proto_str(VPN_PROTO_T proto)
{
	switch(proto) {
		case VPN_PROTO_PPTP:
			return VPN_PROTO_PPTP_STR;
		case VPN_PROTO_L2TP:
			return VPN_PROTO_L2TP_STR;
		case VPN_PROTO_OVPN:
			return VPN_PROTO_OVPN_STR;
		case VPN_PROTO_IPSEC:
			return VPN_PROTO_IPSEC_STR;
		case VPN_PROTO_WG:
			return VPN_PROTO_WG_STR;
		case VPN_PROTO_L2GRE:
			return VPN_PROTO_L2GRE_STR;
		case VPN_PROTO_L3GRE:
			return VPN_PROTO_L3GRE_STR;
		default:
			return "UNKNOWN";
	}
}

int get_vpnc_idx_by_proto_unit(VPN_PROTO_T proto, int unit)
{
	char *nv = NULL, *nvp = NULL, *b = NULL;
	char *desc, *nv_proto, *nv_unit, *username, *passwd, *active, *vpnc_idx, *region, *conntype;
	int ret = 0;
	VPN_PROTO_T nv_real_proto;

	//pptp l2tp handle by vpn fusion
	if (proto == VPN_PROTO_PPTP || proto == VPN_PROTO_L2TP)
		return 0;

#if 0	// Asus
	nv = nvp = strdup(nvram_safe_get("vpnc_clientlist"));
	if (nv) {
		while ((b = strsep(&nvp, "<"))) {
			if (vstrsep(b, ">", &desc, &nv_proto, &nv_unit, &username, &passwd, &active, &vpnc_idx, &region, &conntype) < 4)
				continue;
			if (!nv_proto || !nv_unit || !vpnc_idx)
				continue;
			nv_real_proto = _get_real_vpn_proto(nv_proto);
			if (atoi(nv_unit) == unit && nv_real_proto == proto) {
				ret = atoi(vpnc_idx);
				break;
			}
		}
		free(nv);
	}
#else	// Merlin
	if (proto == VPN_PROTO_WG)
		ret = unit;
	else if (proto == VPN_PROTO_OVPN)
		ret = unit + 5;
#endif
	return (ret);
}

int get_vpns_idx_by_proto_unit(VPN_PROTO_T proto, int unit)
{
	char *nv = NULL, *nvp = NULL, *b = NULL;
	char *desc, *nv_proto, *nv_unit, *active, *vpns_idx;
	int ret = 0;
	int nounit = 0;

	// no unit design
	if (proto == VPN_PROTO_PPTP || proto == VPN_PROTO_IPSEC)
		nounit = 1;

	nv = nvp = strdup(nvram_safe_get("vpns_rl"));
	if (nv) {
		while ((b = strsep(&nvp, "<"))) {
			if (vstrsep(b, ">", &desc, &nv_proto, &nv_unit, &active, &vpns_idx) < 2)
				continue;
			if (!nv_proto || !vpns_idx)
				continue;
			if (!strcmp(nv_proto, _get_vpn_proto_str(proto))) {
				if (nounit) {
					ret = atoi(vpns_idx);
					break;
				}
				else if (nv_unit && atoi(nv_unit) == unit) {
					ret = atoi(vpns_idx);
					break;
				}
			}
		}
		free(nv);
	}
	return (ret);
}

int get_gre_idx_by_proto_unit(VPN_PROTO_T proto, int unit)
{
	char *nv = NULL, *nvp = NULL, *b = NULL;
	char *desc, *nv_proto, *nv_unit, *active, *gre_idx;
	int ret = 0;

	nv = nvp = strdup(nvram_safe_get("gre_rl"));
	if (nv) {
		while ((b = strsep(&nvp, "<"))) {
			if (vstrsep(b, ">", &desc, &nv_proto, &nv_unit, &active, &gre_idx) < 3)
				continue;
			if (!nv_proto || !gre_idx)
				continue;
			if (!strcmp(nv_proto, _get_vpn_proto_str(proto))
			 && nv_unit && atoi(nv_unit) == unit
			) {
				ret = atoi(gre_idx);
				break;
			}
		}
		free(nv);
	}
	return (ret);
}

char* get_vpns_ifname_by_vpns_idx(int vpns_idx, char* buf, size_t len)
{
	char *nv = NULL, *nvp = NULL, *b = NULL;
	char *desc, *nv_proto, *nv_unit, *active, *nv_vpns_idx;
	int ret = 0;

	nv = nvp = strdup(nvram_safe_get("vpns_rl"));
	if (nv) {
		while ((b = strsep(&nvp, "<"))) {
			if (vstrsep(b, ">", &desc, &nv_proto, &nv_unit, &active, &nv_vpns_idx) < 2)
				continue;
			else if (!nv_proto || !nv_vpns_idx)
				continue;
			else if (vpns_idx != atoi(nv_vpns_idx))
				continue;
			else if (!strcmp(nv_proto, VPN_PROTO_WG_STR)) {
				if (nv_unit) {
					snprintf(buf, len, "wgs%d", atoi(nv_unit));
					ret = 1;
				}
			}
			else if (!strcmp(nv_proto, VPN_PROTO_OVPN_STR)) {
				if (nv_unit) {
					snprintf(buf, len, "tun%d", OVPN_SERVER_BASE + atoi(nv_unit));
					ret = 1;
				}
			}
			else if (!strcmp(nv_proto, VPN_PROTO_PPTP_STR)) {
				snprintf(buf, len, "pptp+");
				ret = 1;
			}
		}
		free(nv);
	}
	return (ret) ? buf : NULL;
}

char* get_vpns_iprange_by_vpns_idx(int vpns_idx, char* buf, size_t len)
{
	char *nv = NULL, *nvp = NULL, *b = NULL;
	char *desc, *nv_proto, *nv_unit, *active, *nv_vpns_idx;
	int ret = 0;

	nv = nvp = strdup(nvram_safe_get("vpns_rl"));
	if (nv) {
		while ((b = strsep(&nvp, "<"))) {
			if (vstrsep(b, ">", &desc, &nv_proto, &nv_unit, &active, &nv_vpns_idx) < 2)
				continue;
			else if (!nv_proto || !nv_vpns_idx)
				continue;
			else if (vpns_idx != atoi(nv_vpns_idx))
				continue;
			else if (!strcmp(nv_proto, VPN_PROTO_IPSEC_STR)) {
				//ipsec_profile_1=4>Host-to-Net>null>null>wan>>1>12345678>null>null>null>null>null>1>10.10.10> ...
				int ipsec_vsubnet_idx = 13;
				char ipsec_prof[256] = {0};
				char *p, *p_end;
				nvram_safe_get_r("ipsec_profile_1", ipsec_prof, sizeof(ipsec_prof));
				p = strpbrk(ipsec_prof, ">");
				while (ipsec_vsubnet_idx--)
					p = strpbrk(p+1, ">");
				p_end = strpbrk(p+1, ">");
				*p_end = '\0';
				snprintf(buf, len, "%s.1-%s.254", p+1, p+1);
				ret = 1;
			}
		}
		free(nv);
	}
	return (ret) ? buf : NULL;
}

VPN_VPNX_T* get_vpnx_by_vpnc_idx(VPN_VPNX_T* vpnx, int vpnc_idx)
{
	char *nv = NULL, *nvp = NULL, *b = NULL;
	char *desc, *nv_proto, *nv_unit, *username, *passwd, *active, *nv_vpnc_idx, *region, *conntype;
	int ret = 0;

	if (!vpnx)
		return NULL;

#if 0
	nv = nvp = strdup(nvram_safe_get("vpnc_clientlist"));
	if (nv) {
		while ((b = strsep(&nvp, "<"))) {
			if (vstrsep(b, ">", &desc, &nv_proto, &nv_unit, &username, &passwd, &active, &nv_vpnc_idx, &region, &conntype) < 4)
				continue;
			if (!nv_proto || !nv_vpnc_idx)
				continue;
			else if (vpnc_idx != atoi(nv_vpnc_idx))
				continue;
			vpnx->proto = _get_real_vpn_proto(nv_proto);
			if (nv_unit &&
				(  vpnx->proto == VPN_PROTO_OVPN
				|| vpnx->proto == VPN_PROTO_WG
			)) {
				vpnx->unit = atoi(nv_unit);
			}
			ret = 1;
			break;
		}
		free(nv);
	}
#else
// AMNG does not use vpnc_clientlist, instead OVPN unit is offset by 5 in sdn_rl nvram
	if (vpnc_idx > 5) {
		vpnx->proto = VPN_PROTO_OVPN;
		vpnx->unit = vpnc_idx - 5;
		ret = 1;
	} else {
		vpnx->proto = VPN_PROTO_WG;
		vpnx->unit = vpnc_idx;
		ret = 1;
	}
#endif

	return (ret) ? vpnx : NULL;
}

VPN_VPNX_T* get_vpnx_by_vpns_idx(VPN_VPNX_T* vpnx, int vpns_idx)
{
	char *nv = NULL, *nvp = NULL, *b = NULL;
	char *desc, *nv_proto, *nv_unit, *active, *nv_vpns_idx;
	int ret = 0;

	if (!vpnx)
		return NULL;

	nv = nvp = strdup(nvram_safe_get("vpns_rl"));
	if (nv) {
		while ((b = strsep(&nvp, "<"))) {
			if (vstrsep(b, ">", &desc, &nv_proto, &nv_unit, &active, &nv_vpns_idx) < 2)
				continue;
			else if (!nv_proto || !nv_vpns_idx)
				continue;
			else if (vpns_idx != atoi(nv_vpns_idx))
				continue;
			else if (!strcmp(nv_proto, VPN_PROTO_WG_STR)) {
				vpnx->proto = VPN_PROTO_WG;
				if (nv_unit)
					vpnx->unit = atoi(nv_unit);
				ret = 1;
			}
			else if (!strcmp(nv_proto, VPN_PROTO_OVPN_STR)) {
				vpnx->proto = VPN_PROTO_OVPN;
				if (nv_unit)
					vpnx->unit = atoi(nv_unit);
				ret = 1;
			}
			else if (!strcmp(nv_proto, VPN_PROTO_IPSEC_STR)) {
				vpnx->proto = VPN_PROTO_IPSEC;
				ret = 1;
			}
			else if (!strcmp(nv_proto, VPN_PROTO_PPTP_STR)) {
				vpnx->proto = VPN_PROTO_PPTP;
				ret = 1;
			}
			break;
		}
		free(nv);
	}

	return (ret) ? vpnx : NULL;
}

VPN_VPNX_T* get_vpnx_by_gre_idx(VPN_VPNX_T* vpnx, int gre_idx)
{
	char *nv = NULL, *nvp = NULL, *b = NULL;
	char *desc, *nv_proto, *nv_unit, *active, *nv_gre_idx;
	int ret = 0;

	if (!vpnx)
		return NULL;

	nv = nvp = strdup(nvram_safe_get("gre_rl"));
	if (nv) {
		while ((b = strsep(&nvp, "<"))) {
			if (vstrsep(b, ">", &desc, &nv_proto, &nv_unit, &active, &nv_gre_idx) < 2)
				continue;
			else if (!nv_proto || !nv_gre_idx)
				continue;
			else if (gre_idx != atoi(nv_gre_idx))
				continue;
			else if (!strcmp(nv_proto, VPN_PROTO_L2GRE_STR)) {
				vpnx->proto = VPN_PROTO_L2GRE;
				if (nv_unit)
					vpnx->unit = atoi(nv_unit);
				ret = 1;
			}
			else if (!strcmp(nv_proto, VPN_PROTO_L3GRE_STR)) {
				vpnx->proto = VPN_PROTO_L3GRE;
				if (nv_unit)
					vpnx->unit = atoi(nv_unit);
				ret = 1;
			}
			break;
		}
		free(nv);
	}

	return (ret) ? vpnx : NULL;
}

//for freewifi start
CP_TYPE_RL *get_cp_type_byidx(int index, CP_TYPE_RL *lst)
{
	_dprintf("[FUN:%s][LINE:%d]-----index=%d------\n",__FUNCTION__,__LINE__,index);
	int found = 0;
	char *b = NULL;
	char *nvp = NULL;
	char *nv = NULL;

	char *idx = NULL;
	char *ctype = NULL;
	
	if (index < 0 || !lst)
		return NULL;

	nv = nvp = strdup(nvram_safe_get("cp_type_rl"));
	if (!nv) 
		return NULL;

	while ((b = strsep(&nvp, "<")) != NULL) {
		if (vstrsep(b, ">", &idx, &ctype) < 1)
			continue;

		if (!idx || strlen(idx) <= 0)
			continue;

		if (atoi(idx) != index)
			continue;

		found = 1;
		// idx
		lst->cp_idx = (idx && strlen(idx)>0) ? atoi(idx) : 0;
		// type
		lst->cp_type = (ctype && strlen(ctype)>0) ? atoi(ctype) : 0;
		
		break;
	}
 
	free(nv);
	return (found==1) ? lst : NULL;
}

CP_PROFILE *get_cpX_profile_by_cpidx(int index, CP_PROFILE *lst)
{
	_dprintf("[FUN:%s][LINE:%d]-----index=%d------\n",__FUNCTION__,__LINE__,index);
	int found = 0;
	char *nv = NULL, *nvp = NULL, *b;
	char *cp_able = NULL, *cp_au_tp = NULL, *cp_ctimeout = NULL, *cp_idtimeout = NULL, *cp_atimeout = NULL, *cp_rdirecturl = NULL, *cp_enable_tservice = NULL, *cp_blimit_ul = NULL, *cp_blimt_dl = NULL, *cp_nid = NULL, *cp_idx_cusui = NULL;

	char profile_name[32];
	memset(profile_name, 0, sizeof(profile_name));
	snprintf(profile_name, sizeof(profile_name), "cp%d_profile",index);
	char *pf = profile_name;
	_dprintf("[FUN:%s][LINE:%d]-----pf=%s------\n",__FUNCTION__,__LINE__,pf);

	if (index < 0 || !lst)
		return NULL;


	if (!(nv = nvp = strdup(nvram_safe_get(pf))))
		return NULL;

	while ((b = strsep(&nvp, "<")) != NULL) {
		if(vstrsep(b, ">", &cp_able, &cp_au_tp, &cp_ctimeout, &cp_idtimeout, &cp_atimeout, &cp_rdirecturl, &cp_enable_tservice, &cp_blimit_ul, &cp_blimt_dl, &cp_nid, &cp_idx_cusui) < 1)
			continue;
		
			if (!cp_able || strlen(cp_able) <= 0)
				continue;

			found = 1;

			if (cp_able && *cp_able)
				lst->cp_enable = strtol(cp_able, NULL, 10);
			if (cp_au_tp && *cp_au_tp)
				lst->cp_auth_type = strtol(cp_au_tp, NULL, 10);
			if (cp_ctimeout && *cp_ctimeout)
				lst->cp_conntimeout = strtol(cp_ctimeout, NULL, 10);
			if (cp_idtimeout && *cp_idtimeout)
				lst->cp_idletimeout = strtol(cp_idtimeout, NULL, 10);
			if (cp_idtimeout && *cp_idtimeout)
				lst->cp_idletimeout = strtol(cp_idtimeout, NULL, 10);
			if (cp_atimeout && *cp_atimeout)
				lst->cp_authtimeout = strtol(cp_atimeout, NULL, 10);
			if (cp_rdirecturl && *cp_rdirecturl)
				strlcpy(lst->redirecturl , cp_rdirecturl, sizeof(lst->redirecturl));	
			if (cp_enable_tservice && *cp_enable_tservice)
				lst->cp_term_service = strtol(cp_enable_tservice, NULL, 10);		
			if (cp_blimit_ul && *cp_blimit_ul)
				lst->cp_bw_limit_ul = strtol(cp_blimit_ul, NULL, 10);	
			if (cp_blimt_dl && *cp_blimt_dl)
				lst->cp_bw_limit_dl = strtol(cp_blimt_dl, NULL, 10);
			if (cp_nid && *cp_nid)
				strlcpy(lst->cp_nas_id , cp_nid, sizeof(lst->cp_nas_id));	
			if (cp_idx_cusui && *cp_idx_cusui)
				strlcpy(lst->cp_idx_ui_customizes , cp_idx_cusui, sizeof(lst->cp_idx_ui_customizes));
		
			break;
	}
 
	free(nv);
	return (found==1) ? lst : NULL;
}

//type 0:close 1:freewifi 2:cp
CP_LOCAL_AUTH *get_cp_localauth_bytype(CP_LOCAL_AUTH *lst, int index, int type)
{
	_dprintf("[FUN:%s][LINE:%d]-----index=%d------\n",__FUNCTION__,__LINE__,index);
	int found = 0;
	char *passcode;
	char buf_f[128];
	char *dst = NULL;

	char profile_name[32];
	memset(profile_name, 0, sizeof(profile_name));
	snprintf(profile_name, sizeof(profile_name), "cp%d_local_auth_profile",index);
	char *pf = profile_name;

	if (index < 0 || !lst)
		return NULL;

	if((type == 1) || (type == 2)){
		//free wifi 
		memset(buf_f, '\0', sizeof(buf_f));
		strncpy(buf_f, nvram_safe_get(pf), sizeof(buf_f));
		char *temp = strtok(buf_f," < ");
		while(temp)
		{
			printf("%s\n",temp);
			dst = temp;
			found = 1;
			break;
		}

		if (dst && *dst){
			strlcpy(lst->passcode , dst, sizeof(lst->passcode));
		}

	}else
		return NULL;

 
	return (found==1) ? lst : NULL;
}

char *match_vif_from_rulelist(size_t *sz, int v_id)
{
	char *nv = NULL, *nvp = NULL, *b;
	size_t cnt = 0;
	char *vid, *ifname, *portif;
	static char res[512];
	memset(res, 0, sizeof(res));

	if (sz)
		*sz = 0;

	if (!(nv = nvp = strdup(nvram_safe_get("ap_rulelist"))))
		return NULL;

	while ((b = strsep(&nvp, "<")) != NULL) {
		if(vstrsep(b, ">", &vid, &ifname, &portif) < 1)
			continue;
		
		if (cnt >= MTLAN_MAXINUM)
			break;
		
		if((atoi(vid) == v_id) && ifname && *ifname){
			strlcpy(res , ifname, sizeof(res ));	
			return res;
		}

		cnt++;
		
	}

	*sz = cnt;
	free(nv);

	return res;

}
//for freewifi end

int mtlan_extend_prefix_by_subnet_idx(const char* prefix, int prefix_length, uint8_t subnet_idx, int subnet_length, char* buf, size_t len)
{
	uint8_t addr[16] = {0};
	uint8_t subnet[16] = {0};
	int n = 0;
	int i;

	if (inet_pton(AF_INET6, prefix, addr) < 0)
		return 0;
	if (subnet_length != 8 && subnet_length != 16)
		return 0;
	if (prefix_length + subnet_length > 104)
		return 0;

	n = prefix_length / 8;
	n += (prefix_length % 8) ? 1:0;
	n += (subnet_length > 8) ? 1:0;
	subnet[n] = subnet_idx;

	for (i = 0; i < 16; i++) {
		addr[i] |= subnet[i];
	}

	if (inet_ntop(AF_INET6, addr, buf, len) < 0)
		return 0;

	return ((n+1)*8);
}

CP_RADIUS_PROFILE *get_cpX_radius_profile(CP_RADIUS_PROFILE *cpX_radius_profile, int idx)
{
	char *nv = NULL, *nvp = NULL, *b;
	char *radius_idx = NULL;
	int found = 0;

	char profile_name[32];
	memset(profile_name, 0, sizeof(profile_name));
	snprintf(profile_name, sizeof(profile_name), "cp%d_radius_profile",idx);
	char *pf = profile_name;
		_dprintf("[FUN:%s][LINE:%d]----------pf=%s------------\n",__FUNCTION__,__LINE__,pf);

	if (idx < 0 || !cpX_radius_profile)
		return NULL;

	if (!(nv = nvp = strdup(nvram_safe_get(pf))))
		return NULL;

	while ((b = strsep(&nvp, "<")) != NULL) {
		if(vstrsep(b, ">", &radius_idx) < 1)
			continue;

		if (!radius_idx || strlen(radius_idx) <= 0)
			continue;

		if (atoi(radius_idx) != idx)
			continue;

		found = 1;

		if (radius_idx && *radius_idx)
			cpX_radius_profile->radius_idx = strtol(radius_idx, NULL, 10);
	_dprintf("[FUN:%s][LINE:%d]----------radius_idx=%s---------------\n",__FUNCTION__,__LINE__,radius_idx);
		
		break;

	}

	free(nv);

	return (found==1) ? cpX_radius_profile : NULL;
}

CP_RADIUS_LIST *match_radius_rl_by_cpidx(CP_RADIUS_LIST *cp_radius_rl, int idx)
{

	_dprintf("[FUN:%s][LINE:%d]------------------------------\n",__FUNCTION__,__LINE__);
	int found = 0;
	char *nv = NULL, *nvp = NULL, *b;
	char *cprl_idx = NULL, *ipaddr = NULL, *port = NULL, *key = NULL, *acct_ipaddr = NULL, *acct_port = NULL, *acct_key = NULL, *ipaddr2 = NULL, *port2 = NULL, *key2 = NULL, *acct_ipaddr2 = NULL, *acct_port2 = NULL, *acct_key2 = NULL;


	if (idx < 0 || !cp_radius_rl)
		return NULL;

	if (!(nv = nvp = strdup(nvram_safe_get("radius_list"))))
		return NULL;

	while ((b = strsep(&nvp, "<")) != NULL) {
		if(vstrsep(b, ">", &cprl_idx, &ipaddr, &port, &key, &acct_ipaddr, &acct_port, &acct_key, &ipaddr2, &port2, &key2, &acct_ipaddr2, &acct_port2, &acct_key2) < 12)
			continue;
		
		
		if (!cprl_idx || strlen(cprl_idx) <= 0)
			continue;

		if (atoi(cprl_idx) != idx)
			continue;
		
		found = 1;
		
		if (cprl_idx && *cprl_idx)
			cp_radius_rl->idx = strtol(cprl_idx, NULL, 10);
	_dprintf("[FUN:%s][LINE:%d]-----------cprl_idx=%s---------idx=%d----------\n",__FUNCTION__,__LINE__,cprl_idx,idx);

		//strlcpy(lst->redirecturl , cp_rdirecturl, sizeof(lst->redirecturl));	
		//lst->cp_authtimeout = strtol(cp_atimeout, NULL, 10);
		if (ipaddr && *ipaddr)
			strlcpy(cp_radius_rl->authipaddr1 , ipaddr, sizeof(cp_radius_rl->authipaddr1));
		if (port && *port)
			cp_radius_rl->authport1 = strtol(port, NULL, 10);
		if (key && *key)
			strlcpy(cp_radius_rl->authkey1 , key, sizeof(cp_radius_rl->authkey1));

	_dprintf("[FUN:%s][LINE:%d]-----------ipaddr=%s---------port=%s---key=%s------\n",__FUNCTION__,__LINE__,ipaddr,port,key);

		if (acct_ipaddr && *acct_ipaddr)
			strlcpy(cp_radius_rl->acct_ipaddr1, acct_ipaddr, sizeof(cp_radius_rl->acct_ipaddr1));
		if (acct_port && *acct_port)
			cp_radius_rl->acct_port1 = strtol(acct_port, NULL, 10);
		if (acct_key && *acct_key)
			strlcpy(cp_radius_rl->acct_key1, acct_key, sizeof(cp_radius_rl->acct_key1));
	_dprintf("[FUN:%s][LINE:%d]-----acct_ipaddr=%s------acct_port=%s----key=%s--\n",__FUNCTION__,__LINE__,acct_ipaddr,acct_port,acct_key);


		if (ipaddr2 && *ipaddr2)
			strlcpy(cp_radius_rl->authipaddr2, ipaddr2, sizeof(cp_radius_rl->authipaddr2));
		if (port2 && *port2)
			cp_radius_rl->authport2 = strtol(port2, NULL, 10);
		if (key2 && *key2)
			strlcpy(cp_radius_rl->authkey2 , key2, sizeof(cp_radius_rl->authkey2));
	_dprintf("[FUN:%s][LINE:%d]-----ipaddr2=%s------port2=%s----key2=%s--\n",__FUNCTION__,__LINE__,ipaddr2,port2,key2);


		if (acct_ipaddr2 && *acct_ipaddr2)
			strlcpy(cp_radius_rl->acct_ipaddr2, acct_ipaddr2, sizeof(cp_radius_rl->acct_ipaddr2));
		if (acct_port2 && *acct_port2)
			cp_radius_rl->acct_port2 = strtol(acct_port2, NULL, 10);
		if (acct_key2 && *acct_key2)
			strlcpy(cp_radius_rl->acct_key2, acct_key2, sizeof(cp_radius_rl->acct_key2));
	_dprintf("[FUN:%s][LINE:%d]-----acct_ipaddr2=%s------acct_port2=%s----acct_key2=%s--\n",__FUNCTION__,__LINE__,acct_ipaddr2,acct_port2,acct_key2);

		break;
	}


	_dprintf("[FUN:%s][LINE:%d]--------------end----------------\n",__FUNCTION__,__LINE__);
	free(nv);
	return (found==1) ? cp_radius_rl : NULL;
}
