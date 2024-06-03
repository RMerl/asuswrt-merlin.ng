/*
 * Copyright 2022, ASUSTeK Inc.
 * All Rights Reserved.
 *
 */

#include "rc.h"

#define GRE_DIR_CONF    "/etc/gre"

typedef enum {
	GRE_TYPE_L2,
	GRE_TYPE_L3,
	GRE_TYPE_L2_V6,
	GRE_TYPE_L3_V6,
	GRE_TYPE_UNKOWN
} gre_type_t;

static int _gre_if_exist(const char* ifname)
{
	return if_nametoindex(ifname) ? 1 : 0;
}

static void _gre_tunnel_create(gre_type_t type, char* ifname, char* remote, char* local)
{
	char gre_type[16] = {0};

	switch (type) {
		case GRE_TYPE_L2:
			strlcpy(gre_type, "gretap", sizeof(gre_type));
			break;
		case GRE_TYPE_L3:
			strlcpy(gre_type, "gre", sizeof(gre_type));
			break;
		case GRE_TYPE_L2_V6:
			strlcpy(gre_type, "ip6gretap", sizeof(gre_type));
			break;
		case GRE_TYPE_L3_V6:
			strlcpy(gre_type, "ip6gre", sizeof(gre_type));
			break;
		default:
			_dprintf("Unknown GRE type");
			return;
	}

	eval("ip", "link", "add", "name", ifname, "type", gre_type, "remote", remote, "local", local);
	eval("ip", "link", "set", "up", "dev", ifname);
}

static void _gre_tunnel_destroy(char* ifname)
{
	eval("ip", "link", "del", ifname);
}

void start_l2gre(int unit)
{
	char prefix[16] = {0};
	char ifname[16] = {0};
	char vifname[16] = {0};
	char remote[64] = {0};
	char local[64] = {0};
	int v6 = 0;
	int vlanid = 0;
	char br_ifname[16] = {0};
	char *gre_argv[] = {"grekad", "-i", ifname, "-p", NULL, "-r", NULL, "-n", NULL};
#ifdef RTCONFIG_MULTILAN_CFG
	int gre_idx = get_gre_idx_by_proto_unit(VPN_PROTO_L2GRE, unit);
	MTLAN_T *pmtl = (MTLAN_T *)INIT_MTLAN(sizeof(MTLAN_T));
	size_t  mtl_sz = 0;
#endif

	snprintf(prefix, sizeof(prefix), "%s%d_", L2GRE_NVRAM_PREFIX, unit);
	snprintf(ifname, sizeof(ifname), "%s%d", L2GRE_IF_PREFIX, unit);

	if (!nvram_pf_get_int(prefix, "enable"))
		return;

	// tunnel
	strlcpy(remote, nvram_pf_safe_get(prefix, "remote"), sizeof(remote));
	v6 = is_valid_ip6(remote);
	if (v6) {
		strlcpy(local, getifaddr(get_wan6face(), AF_INET6, GIF_PREFIXLEN) ? : nvram_safe_get(ipv6_nvname("ipv6_wan_ipaddr")), sizeof(local));
		_gre_tunnel_create(GRE_TYPE_L2_V6, ifname, remote, local);
	}
	else {
		strlcpy(local, get_wanip(), sizeof(local));
		_gre_tunnel_create(GRE_TYPE_L2, ifname, remote, local);
	}

#ifdef RTCONFIG_MULTILAN_CFG
	get_mtlan_by_idx(SDNFT_TYPE_GRE, gre_idx, pmtl, &mtl_sz);
	if (mtl_sz)
		strlcpy(br_ifname, pmtl[0].nw_t.br_ifname, sizeof(br_ifname));
	else
#endif
	strlcpy(br_ifname, nvram_safe_get("lan_ifname"), sizeof(br_ifname));


	vlanid = nvram_pf_get_int(prefix, "vlanid");
	if (vlanid) {
		snprintf(vifname, sizeof(vifname), "%s%d.%d", L2GRE_IF_PREFIX, unit, vlanid);
		eval("ip", "link", "add", "link", ifname, "name", vifname, "type", "vlan", "id", nvram_pf_safe_get(prefix, "vlanid"));
		eval("ip", "link", "set", vifname, "up");
		eval("brctl", "addif", br_ifname, vifname);
	}
	else {
		eval("brctl", "addif", br_ifname, ifname);
	}

	// keepalive
	nvram_pf_set(prefix, "state", "0");
	if (nvram_pf_get_int(prefix, "ka_enable")) {
		gre_argv[4] = nvram_pf_safe_get(prefix, "ka_period");
		gre_argv[6] = nvram_pf_safe_get(prefix, "ka_retries");
		_eval(gre_argv, ">/dev/console", 0, NULL);
	}
}

void stop_l2gre(int unit)
{
	char ifname[16] = {0};
	char path[128] = {0};

	snprintf(ifname, sizeof(ifname), "%s%d", L2GRE_IF_PREFIX, unit);
	_gre_tunnel_destroy(ifname);

	snprintf(path, sizeof(path), "/var/run/grekad-%s.pid", ifname);
	kill_pidfile_tk(path);
}

static void _gre_nf_bind_sdn(FILE* fp, const char* gre_ifname, const char* sdn_ifname)
{
	if (fp) {
		if (sdn_ifname) {
			fprintf(fp, "iptables -I GREF -i %s -o %s -j ACCEPT\n", gre_ifname, sdn_ifname);
			fprintf(fp, "iptables -I GREF -o %s -i %s -j ACCEPT\n", gre_ifname, sdn_ifname);
			fprintf(fp, "ip6tables -I GREF -i %s -o %s -j ACCEPT\n", gre_ifname, sdn_ifname);
			fprintf(fp, "ip6tables -I GREF -o %s -i %s -j ACCEPT\n", gre_ifname, sdn_ifname);
		}
		else {
			fprintf(fp, "iptables -I GREF -i %s -j ACCEPT\n", gre_ifname);
			fprintf(fp, "iptables -I GREF -o %s -j ACCEPT\n", gre_ifname);
			fprintf(fp, "ip6tables -I GREF -i %s -j ACCEPT\n", gre_ifname);
			fprintf(fp, "ip6tables -I GREF -o %s -j ACCEPT\n", gre_ifname);
		}
	}
}

static void _gre_nf_add(int unit, char* prefix, char* ifname)
{
	FILE* fp;
	char path[128] = {0};
#ifdef RTCONFIG_MULTILAN_CFG
	int gre_idx = get_gre_idx_by_proto_unit(VPN_PROTO_L3GRE, unit);
	MTLAN_T *pmtl = (MTLAN_T *)INIT_MTLAN(sizeof(MTLAN_T));
	size_t  mtl_sz = 0;
	int i;
#endif

	if (!d_exists(GRE_DIR_CONF))
		mkdir(GRE_DIR_CONF, 0700);

	snprintf(path, sizeof(path), "%s/fw_%s.sh", GRE_DIR_CONF, ifname);
	fp = fopen(path, "w");
	if (fp)
	{
		fprintf(fp, "#!/bin/sh\n\n");

		fprintf(fp, "iptables -I GREI -i %s -j ACCEPT\n", ifname);
		fprintf(fp, "ip6tables -I GREI -i %s -j ACCEPT\n", ifname);
#if !defined(RTCONFIG_MULTILAN_CFG)
		fprintf(fp, "iptables -I GREF -i %s -j ACCEPT\n", ifname);
		fprintf(fp, "iptables -I GREF -o %s -j ACCEPT\n", ifname);
		fprintf(fp, "ip6tables -I GREF -i %s -j ACCEPT\n", ifname);
		fprintf(fp, "ip6tables -I GREF -o %s -j ACCEPT\n", ifname);
#endif

		fclose(fp);
		chmod(path, S_IRUSR|S_IWUSR|S_IXUSR);
		eval(path);
	}

#ifdef RTCONFIG_MULTILAN_CFG
	get_mtlan_by_idx(SDNFT_TYPE_GRE, gre_idx, pmtl, &mtl_sz);
	if (mtl_sz) {
		for (i = 0; i < mtl_sz; i++) {
			snprintf(path, sizeof(path), "%s/fw_%s_sdn%d.sh", GRE_DIR_CONF, ifname, pmtl[i].nw_t.idx);
			fp = fopen(path, "w");
			if(fp) {
				fprintf(fp, "#!/bin/sh\n\n");
				_gre_nf_bind_sdn(fp, ifname, pmtl[i].nw_t.ifname);
				fclose(fp);
				chmod(path, S_IRUSR|S_IWUSR|S_IXUSR);
				eval(path);
			}
		}
	}
	else {
		snprintf(path, sizeof(path), "%s/fw_%s_none.sh", GRE_DIR_CONF, ifname);
		fp = fopen(path, "w");
		if(fp) {
			fprintf(fp, "#!/bin/sh\n\n");
			_gre_nf_bind_sdn(fp, ifname, NULL);
			fclose(fp);
			chmod(path, S_IRUSR|S_IWUSR|S_IXUSR);
			eval(path);
		}
	}
	FREE_MTLAN((void *)pmtl);
#endif
}

static void _gre_nf_del(const char* ifname)
{
	char path[128] = {0};
#ifdef RTCONFIG_MULTILAN_CFG
	int i;
#endif

	snprintf(path, sizeof(path), "%s/fw_%s.sh", GRE_DIR_CONF, ifname);
	if(f_exists(path)) {
		eval("sed", "-i", "s/-I/-D/", path);
		eval(path);
		unlink(path);
	}

#ifdef RTCONFIG_MULTILAN_CFG
	for (i = 0; i <=MTLAN_MAXINUM; i++) {
		snprintf(path, sizeof(path), "%s/fw_%s_sdn%d.sh", GRE_DIR_CONF, ifname, i);
		if(f_exists(path)) {
			eval("sed", "-i", "s/-I/-D/", path);
			eval(path);
			unlink(path);
		}
	}
	snprintf(path, sizeof(path), "%s/fw_%s_none.sh", GRE_DIR_CONF, ifname);
	if(f_exists(path)) {
		eval("sed", "-i", "s/-I/-D/", path);
		eval(path);
		unlink(path);
	}
#endif
}

void start_l3gre(int unit)
{
	char prefix[16] = {0};
	char ifname[16] = {0};
	char remote[64] = {0};
	char local[64] = {0};
	int v6 = 0;
	char addr[64] = {0};
	char buf[64] = {0};
	char *next;
	char routes[1024] = {0};
	char *gre_argv[] = {"grekad", "-i", ifname, "-p", NULL, "-r", NULL, "-n", NULL};

	snprintf(prefix, sizeof(prefix), "%s%d_", L3GRE_NVRAM_PREFIX, unit);
	snprintf(ifname, sizeof(ifname), "%s%d", L3GRE_IF_PREFIX, unit);

	if (!nvram_pf_get_int(prefix, "enable"))
		return;

	// tunnel
	strlcpy(remote, nvram_pf_safe_get(prefix, "remote"), sizeof(remote));
	v6 = is_valid_ip6(remote);
	if (v6) {
		strlcpy(local, getifaddr(get_wan6face(), AF_INET6, GIF_PREFIXLEN) ? : nvram_safe_get(ipv6_nvname("ipv6_wan_ipaddr")), sizeof(local));
		_gre_tunnel_create(GRE_TYPE_L3_V6, ifname, remote, local);
	}
	else {
		strlcpy(local, get_wanip(), sizeof(local));
		_gre_tunnel_create(GRE_TYPE_L3, ifname, remote, local);
	}
	snprintf(addr, sizeof(addr), "%s", nvram_pf_safe_get(prefix, "addr"));
	foreach_44(buf, addr, next) {
		eval("ip", "address", "add", buf, "dev", ifname);
	}

	// routes
	strlcpy(routes, nvram_pf_safe_get(prefix, "routes"), sizeof(routes));
	foreach_44(buf, routes, next) {
		if (strchr(buf, '/') == NULL)
			continue;
		eval("ip", "route", "add", buf, "dev", ifname);
	}

	// netfilter
	_gre_nf_add(unit, prefix, ifname);

	// keepalive
	nvram_pf_set(prefix, "state", "0");
	if (nvram_pf_get_int(prefix, "ka_enable")) {
		gre_argv[4] = nvram_pf_safe_get(prefix, "ka_period");
		gre_argv[6] = nvram_pf_safe_get(prefix, "ka_retries");
		_eval(gre_argv, ">/dev/console", 0, NULL);
	}
	if (v6 == 0) {
		snprintf(buf, sizeof(buf), "/proc/sys/net/ipv4/conf/%s/accept_local", ifname);
		f_write_string(buf, "1", 0, 0);
	}
}

void stop_l3gre(int unit)
{
	char ifname[16] = {0};
	char path[128] = {0};

	snprintf(ifname, sizeof(ifname), "%s%d", L3GRE_IF_PREFIX, unit);
	_gre_tunnel_destroy(ifname);
	_gre_nf_del(ifname);

	snprintf(path, sizeof(path), "/var/run/grekad-%s.pid", ifname);
	kill_pidfile_tk(path);
}

void restart_gre(int v6)
{
	int unit;
	char prefix[16] = {0};
	char ifname[16] = {0};
	int ret;
	for (unit = 1; unit <= L2GRE_MAX; unit++) {
		snprintf(prefix, sizeof(prefix), "%s%d_", L2GRE_NVRAM_PREFIX, unit);
		snprintf(ifname, sizeof(ifname), "%s%d", L2GRE_IF_PREFIX, unit);
		ret = is_valid_ip(nvram_pf_safe_get(prefix, "remote"));
		if ((v6 && ret == 2) || (!v6 && ret == 1)) {
			if (_gre_if_exist(ifname))
				stop_l2gre(unit);
			if (nvram_pf_get_int(prefix, "enable"))
				start_l2gre(unit);
		}
	}
	for (unit = 1; unit <= L3GRE_MAX; unit++) {
		snprintf(prefix, sizeof(prefix), "%s%d_", L3GRE_NVRAM_PREFIX, unit);
		snprintf(ifname, sizeof(ifname), "%s%d", L3GRE_IF_PREFIX, unit);
		ret = is_valid_ip(nvram_pf_safe_get(prefix, "remote"));
		if ((v6 && ret == 2) || (!v6 && ret == 1)) {
			if (_gre_if_exist(ifname))
				stop_l3gre(unit);
			if (nvram_pf_get_int(prefix, "enable"))
				start_l3gre(unit);
		}
	}
}

void run_gre_fw_scripts()
{
	int unit;
	char path[128] = {0};
	for (unit = 1; unit <= L3GRE_MAX; unit++) {
		snprintf(path, sizeof(path), "%s/fw_%s%d.sh", GRE_DIR_CONF, L3GRE_IF_PREFIX, unit);
		if (f_exists(path))
			eval(path);
	}
}

#ifdef RTCONFIG_MULTILAN_CFG
static void _gre_set_to_bridge(char* gre_ifname, char* br_ifname)
{
	char fpath[128] = {0};
	DIR *dirp;
	struct dirent *direntp;
	int inbr = 0;
	int i;
	char old_br_ifname[8] = {0};

	// check gre_ifname already in br_ifname or not.
	snprintf(fpath, sizeof(fpath), "/sys/class/net/%s/brif", br_ifname);
	if ((dirp = opendir(fpath)) != NULL) {
		while((direntp = readdir(dirp)) != NULL) {
			if (direntp->d_name[0] == '.'
			 && (direntp->d_name[1] == '\0'
			 || (direntp->d_name[1] == '.' && direntp->d_name[2] == '\0')))
				continue;

			if (!strcmp(gre_ifname, direntp->d_name)) {
				inbr = 1;
				break;
			}
		}
		closedir(dirp);
	}

	if (inbr)
		return;

	// find gre_ifname in which bridge, remove it from this bridge
	inbr = 0;
	for (i = 0; i < MTLAN_MAXINUM; i++) {
		snprintf(old_br_ifname, sizeof(old_br_ifname), "br%d", i ? MTLAN_IFUNIT_BASE + i : 0);
		snprintf(fpath, sizeof(fpath), "/sys/class/net/%s/brif", old_br_ifname);
		if ((dirp = opendir(fpath)) != NULL) {
			while((direntp = readdir(dirp)) != NULL) {
				if (direntp->d_name[0] == '.'
				 && (direntp->d_name[1] == '\0'
				 || (direntp->d_name[1] == '.' && direntp->d_name[2] == '\0')))
					continue;

				if (!strcmp(gre_ifname, direntp->d_name)) {
					inbr = 1;
					break;
				}
			}
			closedir(dirp);
		}
		if (inbr)
			break;
	}
	if (inbr) {
		eval("brctl", "delif", old_br_ifname, gre_ifname);
	}

	// add to new bridge.
	eval("brctl", "addif", br_ifname, gre_ifname);
}

void update_gre_by_sdn(MTLAN_T *pmtl, size_t mtl_sz, int restart_all_sdn)
{
	int unit, i, j;
	char prefix[16] = {0};
	char gre_ifname[16] = {0};
	int vlanid = 0;
	VPN_VPNX_T vpnx;
	int bind = 0, bind_mtl_i = 0;
	char fpath[128] = {0};
	FILE *fp;
	int sdn_rule_exist = 0;

	// L2GRE
	for (unit = 1; unit <= L2GRE_MAX; unit++) {
		snprintf(prefix, sizeof(prefix), "%s%d_", L2GRE_NVRAM_PREFIX, unit);
		vlanid = nvram_pf_get_int(prefix, "vlanid");
		if (vlanid)
			snprintf(gre_ifname, sizeof(gre_ifname), "%s%d.%d", L2GRE_IF_PREFIX, unit, vlanid);
		else
			snprintf(gre_ifname, sizeof(gre_ifname), "%s%d", L2GRE_IF_PREFIX, unit);

		if (!nvram_pf_get_int(prefix, "enable"))
			continue;

		bind = 0;
		bind_mtl_i = 0;
		for (i = 0; i < mtl_sz; i++) {
			if (!pmtl[i].enable)
				continue;
			for (j = 0; j < MTLAN_GRE_MAXINUM; j++) {
				if (pmtl[i].sdn_t.gre_idx_rl[j]
				&& get_vpnx_by_gre_idx(&vpnx, pmtl[i].sdn_t.gre_idx_rl[j])
				&& vpnx.proto == VPN_PROTO_L2GRE
				&& vpnx.unit == unit) {
					bind = 1;
					bind_mtl_i = i;
					break;
				}
			}
			if (bind)
				break;
		}

		if (bind)
			_gre_set_to_bridge(gre_ifname, pmtl[bind_mtl_i].nw_t.br_ifname);
		else {
#if 1		// ui block setting l2gre to 2+ SDN. If setting 2+ SDN, need to restart all sdn to recover
			_gre_set_to_bridge(gre_ifname, nvram_safe_get("lan_ifname"));
#else		// recover from setting l2gre in 2+ SDN
			int gre_idx = get_gre_idx_by_proto_unit(VPN_PROTO_L2GRE, unit);
			MTLAN_T *pmtl_l2gre = (MTLAN_T *)INIT_MTLAN(sizeof(MTLAN_T));
			size_t mtl_sz_l2gre;
			if (get_mtlan_by_idx(SDNFT_TYPE_VPNC, gre_idx, pmtl_l2gre, &mtl_sz_l2gre) && mtl_sz_l2gre)
				_gre_set_to_bridge(gre_ifname, pmtl_l2gre[0].nw_t.br_ifname);
			else
				_gre_set_to_bridge(gre_ifname, nvram_safe_get("lan_ifname"));
			FREE_MTLAN((void *)pmtl_l2gre);
#endif
		}
	}

	// L3GRE
	if (restart_all_sdn)
		eval("iptables", "-F", "GREF");
	for (unit = 1; unit <= L3GRE_MAX; unit++) {
		snprintf(prefix, sizeof(prefix), "%s%d_", L3GRE_NVRAM_PREFIX, unit);
		snprintf(gre_ifname, sizeof(gre_ifname), "%s%d", L3GRE_IF_PREFIX, unit);

		if (!nvram_pf_get_int(prefix, "enable"))
			continue;

		for (i = 0; i < mtl_sz; i++) {
			// delete old rules for specific sdn
			snprintf(fpath, sizeof(fpath), "%s/fw_%s_sdn%d.sh", GRE_DIR_CONF, gre_ifname, pmtl[i].nw_t.idx);
			if(f_exists(fpath)) {
				eval("sed", "-i", "s/-I/-D/", fpath);
				eval(fpath);
				unlink(fpath);
			}

			// add new rules for specific sdn
			if (!pmtl[i].enable)
				continue;
			for (j = 0; j < MTLAN_GRE_MAXINUM; j++) {
				if (pmtl[i].sdn_t.gre_idx_rl[j]
				&& get_vpnx_by_gre_idx(&vpnx, pmtl[i].sdn_t.gre_idx_rl[j])
				&& vpnx.proto == VPN_PROTO_L3GRE
				&& vpnx.unit == unit) {
					snprintf(fpath, sizeof(fpath), "%s/fw_%s_sdn%d.sh", GRE_DIR_CONF, gre_ifname, pmtl[i].nw_t.idx);
					fp = fopen(fpath, "w");
					if (fp) {
						fprintf(fp, "#!/bin/sh\n\n");
						_gre_nf_bind_sdn(fp, gre_ifname, pmtl[i].nw_t.ifname);
						fclose(fp);
						chmod(fpath, S_IRUSR|S_IWUSR|S_IXUSR);
						eval(fpath);
					}
				}
			}
		}

		// if no rule for specific SDN, add rule for all SDN.
		sdn_rule_exist = 0;
		for (i = 0; i < MTLAN_MAXINUM; i++) {
			snprintf(fpath, sizeof(fpath), "%s/fw_%s_sdn%d.sh", GRE_DIR_CONF, gre_ifname, i);
			if (f_exists(fpath))
				sdn_rule_exist = 1;
		}
		if (sdn_rule_exist) {
			snprintf(fpath, sizeof(fpath), "%s/fw_%s_none.sh", GRE_DIR_CONF, gre_ifname);
			if (f_exists(fpath)) {
				eval("sed", "-i", "s/-I/-D/", fpath);
				eval(fpath);
				unlink(fpath);
			}
		}
		else {
			snprintf(fpath, sizeof(fpath), "%s/fw_%s_none.sh", GRE_DIR_CONF, gre_ifname);
			if (!f_exists(fpath)) {
				fp = fopen(fpath, "w");
				if (fp) {
					fprintf(fp, "#!/bin/sh\n\n");
					_gre_nf_bind_sdn(fp, gre_ifname, NULL);
					fclose(fp);
					chmod(fpath, S_IRUSR|S_IWUSR|S_IXUSR);
					eval(fpath);
				}
			}
			else if (restart_all_sdn) {
				eval(fpath);
			}
		}

	}
}

void update_gre_by_sdn_remove(MTLAN_T *pmtl, size_t mtl_sz)
{
	int unit, i, j;
	char prefix[16] = {0};
	char gre_ifname[IFNAMSIZ] = {0};
	int vlanid = 0;
	VPN_VPNX_T vpnx;
	int binded = 0;
	char fpath[128] = {0};
	FILE *fp;
	int sdn_rule_exist = 0;

	// L2GRE
	for (unit = 1; unit <= L2GRE_MAX; unit++) {
		snprintf(prefix, sizeof(prefix), "%s%d_", L2GRE_NVRAM_PREFIX, unit);
		vlanid = nvram_pf_get_int(prefix, "vlanid");
		if (vlanid)
			snprintf(gre_ifname, sizeof(gre_ifname), "%s%d.%d", L2GRE_IF_PREFIX, unit, vlanid);
		else
			snprintf(gre_ifname, sizeof(gre_ifname), "%s%d", L2GRE_IF_PREFIX, unit);

		if (!nvram_pf_get_int(prefix, "enable"))
			continue;

		binded = 0;
		for (i = 0; i < mtl_sz; i++) {
			if (!pmtl[i].enable)
				continue;
			for (j = 0; j < MTLAN_GRE_MAXINUM; j++) {
				if (pmtl[i].sdn_t.gre_idx_rl[j]
				&& get_vpnx_by_gre_idx(&vpnx, pmtl[i].sdn_t.gre_idx_rl[j])
				&& vpnx.proto == VPN_PROTO_L2GRE
				&& vpnx.unit == unit) {
					binded = 1;
					break;
				}
			}
			if (binded)
				break;
		}

		if (binded)
			_gre_set_to_bridge(gre_ifname, nvram_safe_get("lan_ifname"));
	}

	// L3GRE
	for (unit = 1; unit <= L3GRE_MAX; unit++) {
		snprintf(prefix, sizeof(prefix), "%s%d_", L3GRE_NVRAM_PREFIX, unit);
		snprintf(gre_ifname, sizeof(gre_ifname), "%s%d", L3GRE_IF_PREFIX, unit);

		if (!nvram_pf_get_int(prefix, "enable"))
			continue;

		/// remove rule if binded with the removed SDN.
		for (i = 0; i < mtl_sz; i++) {
			snprintf(fpath, sizeof(fpath), "%s/fw_%s_sdn%d.sh", GRE_DIR_CONF, gre_ifname, pmtl[i].nw_t.idx);
			if(f_exists(fpath)) {
				eval("sed", "-i", "s/-I/-D/", fpath);
				eval(fpath);
				unlink(fpath);
			}
		}

		/// if not bind with other SDN, add rule for all SDN.
		sdn_rule_exist = 0;
		for (i = 0; i < MTLAN_MAXINUM; i++) {
			snprintf(fpath, sizeof(fpath), "%s/fw_%s_sdn%d.sh", GRE_DIR_CONF, gre_ifname, i);
			if (f_exists(fpath))
				sdn_rule_exist = 1;
		}
		if (sdn_rule_exist == 0) {
			snprintf(fpath, sizeof(fpath), "%s/fw_%s_none.sh", GRE_DIR_CONF, gre_ifname);
			if (!f_exists(fpath)) {	//bind -> none
				fp = fopen(fpath, "w");
				if (fp) {
					fprintf(fp, "#!/bin/sh\n\n");
					_gre_nf_bind_sdn(fp, gre_ifname, NULL);
					fclose(fp);
					chmod(fpath, S_IRUSR|S_IWUSR|S_IXUSR);
					eval(fpath);
				}
			}
		}
	}
}
#endif
