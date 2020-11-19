#include <termios.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <time.h>
#include <errno.h>
#include <paths.h>
#include <sys/wait.h>
#include <sys/reboot.h>
#include <sys/klog.h>
#include <arpa/inet.h>
#include <string.h>
#include <bcmnvram.h>
#include <rc.h>
#include <shutils.h>
#ifdef LINUX26
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#endif
#include <shared.h>

extern void get_country_code_from_rc(char* country_code);
extern struct nvram_tuple router_defaults[];

// from in.h
#define IN_CLASSB_NET           0xffff0000
#define IN_CLASSB_HOST          (0xffffffff & ~IN_CLASSB_NET)
// from net.h
#define LINKLOCAL_ADDR	0xa9fe0000

#ifdef DSL_AC68U
#define ETH_WAN_BASE_IFNAME     "eth0"
#define ETH_WAN_IFNAME_PREFIX   "vlan"
#elif defined(DSL_N55U) || defined(DSL_N55U_B)
#define ETH_WAN_BASE_IFNAME     "eth2.1"
#define ETH_WAN_IFNAME_PREFIX   "eth2.1."
#endif

// from zcip.c and revised
// Pick a random link local IP address on 169.254/16, except that
// the first and last 256 addresses are reserved.
void pick_a_random_ipv4(char* buf_ip, size_t len)
{
	unsigned tmp;
	unsigned int tmp_ip;

	do {
		f_read("/dev/urandom", &tmp, sizeof(unsigned));
		tmp &= IN_CLASSB_HOST;
	} while (tmp > (IN_CLASSB_HOST - 0x0200));
	tmp_ip = (LINKLOCAL_ADDR + 0x0100) + tmp;
	snprintf(buf_ip, len, "%d.%d.%d.%d",tmp_ip>>24,(tmp_ip>>16)&0xff,(tmp_ip>>8)&0xff,tmp_ip&0xff);
}


//find the wan setting from WAN List and convert to 
//wan_xxx for original rc flow.

static void convert_dsl_config_num()
{
	int config_num = 0;
	if(nvram_match("dslx_transmode", "atm")) {
		if (nvram_match("dsl0_enable","1")) config_num++;
		if (nvram_match("dsl1_enable","1")) config_num++;
		if (nvram_match("dsl2_enable","1")) config_num++;
		if (nvram_match("dsl3_enable","1")) config_num++;
		if (nvram_match("dsl4_enable","1")) config_num++;
		if (nvram_match("dsl5_enable","1")) config_num++;
		if (nvram_match("dsl6_enable","1")) config_num++;
		if (nvram_match("dsl7_enable","1")) config_num++;
	}
	else {
		if (nvram_match("dsl8_enable","1")) config_num++;
		if (nvram_match("dsl8.1_enable","1")) config_num++;
		if (nvram_match("dsl8.2_enable","1")) config_num++;
		if (nvram_match("dsl8.3_enable","1")) config_num++;
		if (nvram_match("dsl8.4_enable","1")) config_num++;
		if (nvram_match("dsl8.5_enable","1")) config_num++;
		if (nvram_match("dsl8.6_enable","1")) config_num++;
		if (nvram_match("dsl8.7_enable","1")) config_num++;
		//if (nvram_match("dsl9_enable","1")) config_num++;
	}
	nvram_set_int("dslx_config_num", config_num);
}

#if defined(RTCONFIG_MULTISERVICE_WAN)
static void _convert_old_dslx()
{
	char *dsl_prefix = nvram_match("dslx_transmode", "ptm") ? "dsl8_" : "dsl0_";
	char *dslx_prefix = "dslx_";
	char tmp[64] = {0};
	char *attr[] = {
		"enable", "nat", "upnp_enable", "link_enable",
		"DHCPClient", "ipaddr", "netmask", "gateway",
		"dnsenable", "dns1", "dns2",
		"pppoe_username", "pppoe_passwd", "pppoe_auth", "pppoe_idletime",
		"pppoe_mtu", "pppoe_mru", "pppoe_service", "pppoe_ac",
		"pppoe_hostuniq", "pppoe_options",
		"dhcp_clientid_type", "dhcp_clientid", "dhcp_vendorid", "dhcp_hostname",
		"hwaddr"
		, NULL };
	int i = 0;

	if (nvram_get_int("x_Setting") == 0)
		return;

	//tmp flag till new QIS implemented.
	if (nvram_get_int("dslx_converted"))
		return;

	while(attr[i])
	{
		strlcpy(tmp, nvram_pf_safe_get(dslx_prefix, attr[i]), sizeof(tmp));
		if (strlen(tmp))
			nvram_pf_set(dsl_prefix, attr[i], tmp);
		nvram_unset(strcat_r(dslx_prefix, attr[i], tmp));
		i++;
	}

	nvram_set("dslx_converted", "1");
}

static void convert_dsl_wan()
{
	char wan_prefix[16] = {0};
	char dsl_prefix[16] = {0};
	char dsl_proto[16] = {0};
	int wan_base_unit = WAN_UNIT_FIRST;
	int i = 0;
	int isPTM = nvram_match("dslx_transmode","ptm") ? 1 : 0;

#ifdef RTCONFIG_DUALWAN
	if (get_dualwan_primary() == WANS_DUALWAN_IF_DSL)
		wan_base_unit = WAN_UNIT_FIRST;
	else if (get_dualwan_secondary() == WANS_DUALWAN_IF_DSL)
		wan_base_unit = WAN_UNIT_SECOND;
	else
		return;
#endif

	// for old config
	_convert_old_dslx();

	for(i = 0; i < MAX_PVC; i++)
	{
		if(i)
		{
			snprintf(wan_prefix, sizeof(wan_prefix), "wan%d_",
				get_ms_wan_unit(wan_base_unit, i));
			if (isPTM)
				snprintf(dsl_prefix, sizeof(dsl_prefix), "dsl8.%d_", i);
			else
				snprintf(dsl_prefix, sizeof(dsl_prefix), "dsl%d_", i);
		}
		else
		{
			snprintf(wan_prefix, sizeof(wan_prefix), "wan%d_", wan_base_unit);
			if (isPTM)
				snprintf(dsl_prefix, sizeof(dsl_prefix), "dsl8_");
			else
				snprintf(dsl_prefix, sizeof(dsl_prefix), "dsl0_");
		}

		if (nvram_pf_get_int(dsl_prefix, "enable") == 0)
		{
			nvram_pf_set(wan_prefix, "enable", "0");
			//unset to reduce nvram usage
			nvram_pf_unset(wan_prefix, "nat_x");
			nvram_pf_unset(wan_prefix, "upnp_enable");
			nvram_pf_unset(wan_prefix, "dhcpenable_x");
			nvram_pf_unset(wan_prefix, "ipaddr_x");
			nvram_pf_unset(wan_prefix, "netmask_x");
			nvram_pf_unset(wan_prefix, "gateway_x");
			nvram_pf_unset(wan_prefix, "dnsenable_x");
			nvram_pf_unset(wan_prefix, "dns1_x");
			nvram_pf_unset(wan_prefix, "dns2_x");
			nvram_pf_unset(wan_prefix, "pppoe_username");
			nvram_pf_unset(wan_prefix, "pppoe_passwd");
			nvram_pf_unset(wan_prefix, "pppoe_auth");
			nvram_pf_unset(wan_prefix, "pppoe_idletime");
			nvram_pf_unset(wan_prefix, "pppoe_mtu");
			nvram_pf_unset(wan_prefix, "pppoe_mru");
			nvram_pf_unset(wan_prefix, "pppoe_service");
			nvram_pf_unset(wan_prefix, "pppoe_ac");
			nvram_pf_unset(wan_prefix, "pppoe_hostuniq");
			nvram_pf_unset(wan_prefix, "pppoe_options_x");
			nvram_pf_unset(wan_prefix, "hwaddr_x");
			nvram_pf_unset(wan_prefix, "mtu");
			nvram_pf_unset(wan_prefix, "dhcp_qry");
			nvram_pf_unset(wan_prefix, "clientid_type");
			nvram_pf_unset(wan_prefix, "clientid");
			nvram_pf_unset(wan_prefix, "vendorid");
			nvram_pf_unset(wan_prefix, "hostname");
			continue;
		}

		nvram_pf_set(wan_prefix, "enable", "1");
		nvram_pf_set(wan_prefix, "nat_x", nvram_pf_safe_get(dsl_prefix, "nat"));
		nvram_pf_set(wan_prefix, "upnp_enable", nvram_pf_safe_get(dsl_prefix, "upnp_enable"));
		nvram_pf_set(wan_prefix, "dhcpenable_x", nvram_pf_safe_get(dsl_prefix, "DHCPClient"));
		nvram_pf_set(wan_prefix, "ipaddr_x", nvram_pf_safe_get(dsl_prefix, "ipaddr"));
		nvram_pf_set(wan_prefix, "netmask_x", nvram_pf_safe_get(dsl_prefix, "netmask"));
		nvram_pf_set(wan_prefix, "gateway_x", nvram_pf_safe_get(dsl_prefix, "gateway"));
		nvram_pf_set(wan_prefix, "dnsenable_x", nvram_pf_safe_get(dsl_prefix, "dnsenable"));
		nvram_pf_set(wan_prefix, "dns1_x", nvram_pf_safe_get(dsl_prefix, "dns1"));
		nvram_pf_set(wan_prefix, "dns2_x", nvram_pf_safe_get(dsl_prefix, "dns2"));
		nvram_pf_set(wan_prefix, "pppoe_username", nvram_pf_safe_get(dsl_prefix, "pppoe_username"));
		nvram_pf_set(wan_prefix, "pppoe_passwd", nvram_pf_safe_get(dsl_prefix, "pppoe_passwd"));
		nvram_pf_set(wan_prefix, "pppoe_auth", nvram_pf_safe_get(dsl_prefix, "pppoe_auth"));
		nvram_pf_set(wan_prefix, "pppoe_idletime", nvram_pf_safe_get(dsl_prefix, "pppoe_idletime"));
		nvram_pf_set(wan_prefix, "pppoe_mtu", nvram_pf_safe_get(dsl_prefix, "pppoe_mtu"));
		nvram_pf_set(wan_prefix, "pppoe_mru", nvram_pf_safe_get(dsl_prefix, "pppoe_mru"));
		nvram_pf_set(wan_prefix, "pppoe_service", nvram_pf_safe_get(dsl_prefix, "pppoe_service"));
		nvram_pf_set(wan_prefix, "pppoe_ac", nvram_pf_safe_get(dsl_prefix, "pppoe_ac"));
		nvram_pf_set(wan_prefix, "pppoe_hostuniq", nvram_pf_safe_get(dsl_prefix, "pppoe_hostuniq"));
		nvram_pf_set(wan_prefix, "pppoe_options_x", nvram_pf_safe_get(dsl_prefix, "pppoe_options"));
		nvram_pf_set(wan_prefix, "hwaddr_x", nvram_pf_safe_get(dsl_prefix, "hwaddr"));
		nvram_pf_set(wan_prefix, "mtu", nvram_pf_safe_get(dsl_prefix, "mtu"));
		nvram_pf_set(wan_prefix, "dhcp_qry", nvram_pf_safe_get(dsl_prefix, "dhcp_qry"));
		nvram_pf_set(wan_prefix, "clientid_type", nvram_pf_safe_get(dsl_prefix, "dhcp_clientid_type"));
		nvram_pf_set(wan_prefix, "clientid", nvram_pf_safe_get(dsl_prefix, "dhcp_clientid"));
		nvram_pf_set(wan_prefix, "vendorid", nvram_pf_safe_get(dsl_prefix, "dhcp_vendorid"));
		nvram_pf_set(wan_prefix, "hostname", nvram_pf_safe_get(dsl_prefix, "dhcp_hostname"));

		snprintf(dsl_proto, sizeof(dsl_proto), "%s", nvram_pf_safe_get(dsl_prefix, "proto"));
		if (!strcmp(dsl_proto, "pppoe") || !strcmp(dsl_proto, "pppoa"))
		{
			nvram_pf_set(wan_prefix, "dhcpenable_x", "1");
			nvram_pf_set(wan_prefix, "vpndhcp", "0");
			nvram_pf_set(wan_prefix, "proto", "pppoe");
		}
		else if (!strcmp(dsl_proto, "bridge"))
		{
			nvram_pf_set(wan_prefix, "nat_x", "0");
			nvram_pf_set(wan_prefix, "proto", "bridge");
		}
		else if (!strcmp(dsl_proto, "mer"))
		{
			if (nvram_pf_match(dsl_prefix, "DHCPClient","1"))
				nvram_pf_set(wan_prefix, "proto", "dhcp");
			else
				nvram_pf_set(wan_prefix, "proto", "static");
		}
		else if (!strcmp(dsl_proto, "ipoa"))
		{
			nvram_pf_set(wan_prefix, "proto", "static");
		}
		else
		{
			nvram_pf_set(wan_prefix, "proto", dsl_proto);
		}
	}
	nvram_commit();
}

#else
/* Paul modify 2013/1/23 */
static void convert_dsl_wan()
{
	int conv_dsl_to_wan0 = 0;

#ifdef RTCONFIG_DUALWAN
	if (get_dualwan_primary()==WANS_DUALWAN_IF_DSL)
		conv_dsl_to_wan0 = 1;
#else
	conv_dsl_to_wan0 = 1;
#endif

	if (conv_dsl_to_wan0)
	{
		nvram_set("wan_nat_x",nvram_safe_get("dslx_nat"));
		nvram_set("wan_upnp_enable",nvram_safe_get("dslx_upnp_enable"));
		nvram_set("wan_enable",nvram_safe_get("dslx_link_enable"));
		nvram_set("wan_dhcpenable_x",nvram_safe_get("dslx_DHCPClient"));
		nvram_set("wan_ipaddr_x",nvram_safe_get("dslx_ipaddr"));
		nvram_set("wan_upnp_enable",nvram_safe_get("dslx_upnp_enable"));
		nvram_set("wan_netmask_x",nvram_safe_get("dslx_netmask"));
		nvram_set("wan_gateway_x",nvram_safe_get("dslx_gateway"));
		nvram_set("wan_dnsenable_x",nvram_safe_get("dslx_dnsenable"));
		nvram_set("wan_dns1_x",nvram_safe_get("dslx_dns1"));
		nvram_set("wan_dns2_x",nvram_safe_get("dslx_dns2"));
		nvram_set("wan_pppoe_username",nvram_safe_get("dslx_pppoe_username"));
		nvram_set("wan_pppoe_passwd",nvram_safe_get("dslx_pppoe_passwd"));
		nvram_set("wan_pppoe_auth",nvram_safe_get("dslx_pppoe_auth"));
		nvram_set("wan_pppoe_idletime",nvram_safe_get("dslx_pppoe_idletime"));
		nvram_set("wan_pppoe_mtu",nvram_safe_get("dslx_pppoe_mtu"));
		nvram_set("wan_pppoe_mru",nvram_safe_get("dslx_pppoe_mtu"));
		nvram_set("wan_pppoe_service",nvram_safe_get("dslx_pppoe_service"));
		nvram_set("wan_pppoe_ac",nvram_safe_get("dslx_pppoe_ac"));
		nvram_set("wan_pppoe_hostuniq",nvram_safe_get("dslx_pppoe_hostuniq"));
		nvram_set("wan_pppoe_options_x",nvram_safe_get("dslx_pppoe_options"));
		nvram_set("wan_hwaddr_x",nvram_safe_get("dslx_hwaddr"));

		nvram_set("wan0_nat_x",nvram_safe_get("dslx_nat"));
		nvram_set("wan0_upnp_enable",nvram_safe_get("dslx_upnp_enable"));
		nvram_set("wan0_enable",nvram_safe_get("dslx_link_enable"));
		nvram_set("wan0_dhcpenable_x",nvram_safe_get("dslx_DHCPClient"));
		nvram_set("wan0_ipaddr_x",nvram_safe_get("dslx_ipaddr"));
		nvram_set("wan0_upnp_enable",nvram_safe_get("dslx_upnp_enable"));
		nvram_set("wan0_netmask_x",nvram_safe_get("dslx_netmask"));
		nvram_set("wan0_gateway_x",nvram_safe_get("dslx_gateway"));
		nvram_set("wan0_dnsenable_x",nvram_safe_get("dslx_dnsenable"));
		nvram_set("wan0_dns1_x",nvram_safe_get("dslx_dns1"));
		nvram_set("wan0_dns2_x",nvram_safe_get("dslx_dns2"));
		nvram_set("wan0_pppoe_username",nvram_safe_get("dslx_pppoe_username"));
		nvram_set("wan0_pppoe_passwd",nvram_safe_get("dslx_pppoe_passwd"));
		nvram_set("wan0_pppoe_auth",nvram_safe_get("dslx_pppoe_auth"));
		nvram_set("wan0_pppoe_idletime",nvram_safe_get("dslx_pppoe_idletime"));
		nvram_set("wan0_pppoe_mtu",nvram_safe_get("dslx_pppoe_mtu"));
		nvram_set("wan0_pppoe_mru",nvram_safe_get("dslx_pppoe_mtu"));
		nvram_set("wan0_pppoe_service",nvram_safe_get("dslx_pppoe_service"));
		nvram_set("wan0_pppoe_ac",nvram_safe_get("dslx_pppoe_ac"));
		nvram_set("wan0_pppoe_hostuniq",nvram_safe_get("dslx_pppoe_hostuniq"));
		nvram_set("wan0_pppoe_options_x",nvram_safe_get("dslx_pppoe_options"));
		nvram_set("wan0_hwaddr_x",nvram_safe_get("dslx_hwaddr"));					
	}

	if (conv_dsl_to_wan0)
	{
		if (nvram_match("dslx_transmode","ptm")) {
			if (nvram_match("dsl8_proto","pppoe")) {
				nvram_set("wan0_proto", "pppoe");
				/* Turn off DHCP on MAN interface */
				nvram_set("wan0_dhcpenable_x", "1");
				nvram_set("wan0_vpndhcp", "0");
			}
			else if (nvram_match("dsl8_proto","bridge")) {
				nvram_set("wan0_nat_x","0");
				nvram_set("wan0_proto","bridge");
			}
			else if (nvram_match("dsl8_proto","dhcp")) {
				nvram_set("wan0_proto",nvram_safe_get("dsl8_proto"));
			}
			nvram_set("wan_proto",nvram_safe_get("wan0_proto"));

			nvram_set("wan0_mtu",nvram_safe_get("dsl8_mtu"));
		}
		else {
			if (nvram_match("dsl0_proto","pppoe") || nvram_match("dsl0_proto","pppoa")) {
				nvram_set("wan0_proto","pppoe");
				/* Turn off DHCP on MAN interface */
				nvram_set("wan0_dhcpenable_x", "1");
				nvram_set("wan0_vpndhcp", "0");
			}
			else if (nvram_match("dsl0_proto","ipoa")) {
				nvram_set("wan0_proto","static");
			}
			else if (nvram_match("dsl0_proto","bridge")) {
				// disable nat
				nvram_set("wan0_nat_x","0");

				/* Paul add 2012/7/13, for Bridge connection type wan0_proto set to dhcp, and dsl_proto set as bridge */
				nvram_set("wan0_proto","bridge");
				nvram_set("dsl_proto","bridge");
			}
			else if (nvram_match("dsl0_proto","mer")) {
				if (nvram_match("dslx_DHCPClient","1")) {
					nvram_set("wan0_proto","dhcp");
				}
				else {
					nvram_set("wan0_proto","static");
				}
			}
			nvram_set("wan_proto",nvram_safe_get("wan0_proto"));

			nvram_set("wan0_mtu",nvram_safe_get("dsl0_mtu"));
		}
	}
	nvram_commit();
}
#endif

#ifdef RTCONFIG_DSL_TCLINUX
static int check_if_route_exist(char *iface, char *dest, char *mask)
{
	FILE *f;
	int i, n;
	char buf[256] = {0};
	char get_iface[32] = {0};
	u_int32_t get_dest, get_mask;
	int found = 0;

	if ((f = fopen("/proc/net/route", "r")) != NULL) {
		while (fgets(buf, sizeof(buf), f) != NULL) {
			if (++n == 1 && strncmp(buf, "Iface", 5) == 0)
				continue;

			i = sscanf(buf, "%255s %x %*s %*s %*s %*s %*s %x",
				get_iface, &get_dest, &get_mask);

			if (i != 3)
				break;

			if(!strcmp(iface, get_iface)
				&& (u_int32_t)inet_addr(dest) == get_dest
				&& (u_int32_t)inet_addr(mask) == get_mask
			){
				found = 1;
				break;
			}
		}
		fclose(f);

		if (found)
			return 1;
	}

	return 0;
}

static void check_and_set_comm_if(void)
{
#ifdef RTCONFIG_BCM4708
	const char *ipaddr;
	char buf_ip[32];

	ipaddr = getifaddr("vlan2", AF_INET, GIF_PREFIXLEN);
	//_dprintf("%s: %s\n", __func__, ipaddr);
	if(!ipaddr || (ipaddr && strncmp("169.254", ipaddr, 7))) {
		pick_a_random_ipv4(buf_ip, sizeof(buf_ip));
		ifconfig("vlan2", IFUP, buf_ip, "255.255.0.0");
	}

	if(!check_if_route_exist("vlan2", "169.254.0.1", "255.255.255.255"))
		route_add("vlan2", 0, "169.254.0.1", "0.0.0.0", "255.255.255.255");
#endif
}
#endif

void remove_dsl_autodet(void)
{
#ifdef RTCONFIG_RALINK
	int x;
	char wan_if[9];
#endif

	// not autodet , direct return
	if (nvram_match("dsltmp_autodet_state","")) return;

	// ask auto_det to quit
	nvram_set("dsltmp_adslatequit","1");

#ifdef RTCONFIG_RALINK
	for(x=2; x<=8; x++) {
		sprintf(wan_if, "eth2.1.%d", x);
		eval("ifconfig", wan_if, "down");
	}
#endif

	nvram_set("dsltmp_autodet_state","");
}


void dsl_wan_config(int req)
{
	switch(req)
	{
	case 0:	///default
		convert_dsl_config_num();
		break;

	case 1:	///start_wan (boot up)
		convert_dsl_wan();
		break;

	case 2:	///service dslwan_if
#ifdef RTCONFIG_DSL_TCLINUX
		eval("req_dsl_drv", "rmvlan", nvram_safe_get("dslx_rmvlan"));

		//set debug mode before reloadpvc
		if(strstr(nvram_safe_get("dslx_pppoe_options"), "debug")) {
			eval("req_dsl_drv", "wandebug", "on");
		}
		else {
			eval("req_dsl_drv", "wandebug", "off");
		}
#endif
		convert_dsl_config_num();
		config_xtm(); //pvc interface
		convert_dsl_wan();
		break;

	default:
		_dprintf("%s:%d: req %d undefined\n", __FUNCTION__, __LINE__, req);
		break;
	}
}

void
dsl_defaults(void)
{
#if !defined(RTCONFIG_DSL_TCLINUX) && defined(RTCONFIG_DSL_REMOTE)
	struct nvram_tuple *t;
	char prefix[]="dslXXXXXX_", tmp[100];
	int unit;

	for(unit=0;unit<8;unit++) {	
		snprintf(prefix, sizeof(prefix), "dsl%d_", unit);

		for (t = router_defaults; t->name; t++) {
			if(strncmp(t->name, "dsl_", 4)!=0) continue;

			if (!nvram_get(strcat_r(prefix, &t->name[4], tmp))) {
				//_dprintf("_set %s = %s\n", tmp, t->value);
				nvram_set(tmp, t->value);
			}
		}
		unit++;
	}

	// dump trx header
	// this is for upgrading check
	// if trx is same, the upgrade procedure will be skiped
	fprintf(stderr, "dump trx header..\n");
	eval("dd", "if=/dev/mtd3", "of=/tmp/trx_hdr.bin", "count=1");
#endif

	dsl_wan_config(0);
}

#if defined(RTCONFIG_DSL_REMOTE)
#ifdef RTCONFIG_RALINK
// a workaround handler, can be removed after bug found
void init_dsl_before_start_wan(void)
{
	// eth2 could not start up in original initialize routine
	// it must put eth2 start-up code here
	dbG("enable eth2 and power up all LAN ports\n");
	eval("ifconfig", "eth2", "up");
	eval("rtkswitch", "14");
	eval("brctl", "addif", "br0", "eth2.2");
}
#endif

void start_dsl()
{
#ifdef RTCONFIG_RALINK
	// todo: is it necessary? 
	init_dsl_before_start_wan();
#endif

	char *argv_tp_init[] = {"tp_init", NULL};
	int pid;
	
	mkdir("/tmp/adsl", S_IRUSR | S_IWUSR | S_IXUSR);

	/* Paul comment 2012/7/25, the "never overcommit" policy would cause Ralink WiFi driver kernel panic when configure DUT through external registrar. *
	 * So let this value be the default which is 0, the kernel will estimate the amount of free memory left when userspace requests more memory. */
	//f_write_string("/proc/sys/vm/overcommit_memory", "2", 0, 0);

	/// host daemon
#ifdef RTCONFIG_DSL_TCLINUX
	check_and_set_comm_if();
#endif

#ifdef RTCONFIG_RALINK
#ifdef RTCONFIG_DUALWAN
	if (get_dualwan_secondary()==WANS_DUALWAN_IF_NONE)
	{
		if (get_dualwan_primary()!=WANS_DUALWAN_IF_DSL)
		{
			// it does not need to start dsl driver when using other modes
			// but it still need to run tp_init to have firmware version info
			printf("get modem info\n");
			xstart("tp_init", "eth_wan_mode_only");
			return;
		}
	}
#endif
#endif

	_eval(argv_tp_init, NULL, 0, &pid);

	/// host interface
	convert_dsl_config_num();
	config_host_interface();
	config_stb_bridge();

#ifdef RTCONFIG_RALINK
	int config_num = nvram_get_int("dslx_config_num");
	// auto detection
	if (nvram_match("x_Setting", "0") && config_num == 0)
	{
		int x;
		char wan_if[9];
		char wan_num[2];		
		char country_value[8];
		char cmd_buf[64];
		char *argv_auto_det[] = {"auto_det", country_value, NULL};		
		int pid;		
		for(x=2; x<=8; x++) {
			sprintf(wan_num, "%d", x);
			sprintf(wan_if, "eth2.1.%d", x);
			eval("vconfig", "add", "eth2.1", wan_num);
			eval("ifconfig", wan_if, "up");
		}
		//
		nvram_set("dsltmp_autodet_state", "Detecting");
		// call auto detection with country code
		get_country_code_from_rc(country_value);
		_eval(argv_auto_det, NULL, 0, &pid);
	}
#endif	//RTCONFIG_RALINK

	/// xDSL diagnostic
#ifdef RTCONFIG_DSL_TCLINUX
	if(nvram_match("dslx_diag_enable", "1") && nvram_match("dslx_diag_state", "1"))
		start_dsl_diag();
#endif
}

void stop_dsl()
{
#ifdef RTCONFIG_DSL_TCLINUX
	eval("req_dsl_drv", "runtcc");
	eval("req_dsl_drv", "dumptcc");
#endif
	eval("adslate", "quitdrv");
}

#elif defined(RTCONFIG_DSL_HOST)

void get_atm_param(XTM_PARAM* p, int idx)
{
	char prefix[8] = {0};

	snprintf(prefix, sizeof(prefix), "dsl%d_", idx);
	p->enable = nvram_pf_get_int(prefix, "enable");
	p->vpi = nvram_pf_get_int(prefix, "vpi");
	p->vci = nvram_pf_get_int(prefix, "vci");
	p->encap = nvram_pf_get_int(prefix, "encap");
	p->svc_cat = nvram_pf_get_int(prefix, "svc_cat");
	p->pcr = nvram_pf_get_int(prefix, "pcr");
	p->scr = nvram_pf_get_int(prefix, "scr");
	p->mbs = nvram_pf_get_int(prefix, "mbs");
	snprintf(p->proto, sizeof(p->proto), nvram_pf_safe_get(prefix, "proto"));
	p->dot1q = nvram_pf_get_int(prefix, "dot1q");
	p->vid = nvram_pf_get_int(prefix, "vid");
	p->dot1p = nvram_pf_get_int(prefix, "dot1p");
	p->total_config = nvram_get_int("dslx_config_num");
}

void get_ptm_param(XTM_PARAM* p, int idx)
{
	char prefix[8] = {0};

	if(idx)
		snprintf(prefix, sizeof(prefix), "dsl8.%d_", idx);
	else
		snprintf(prefix, sizeof(prefix), "dsl8_");
	p->enable = nvram_pf_get_int(prefix, "enable");
	snprintf(p->proto, sizeof(p->proto), nvram_pf_safe_get(prefix, "proto"));
	p->dot1q = nvram_pf_get_int(prefix, "dot1q");
	p->vid = nvram_pf_get_int(prefix, "vid");
	p->dot1p = nvram_pf_get_int(prefix, "dot1p");
	p->total_config = nvram_get_int("dslx_config_num");
#ifdef DSL_AX82U
	if (is_ax5400_i1()) {
		p->pbr = nvram_get_int("qos_xobw_dsl");
		p->mbs = p->pbr ? 10000 : 0;
	}
#endif
}

void get_xdsl_param(XDSL_PARAM* p)
{
	p->mod = nvram_get_int("dslx_modulation");
	if (p->mod >= DSL_MOD_MAX)
		p->mod = DSL_MOD_AUTO;
	p->annex = nvram_get_int("dslx_annex");
	if (p->annex >= DSL_ANNEX_MAX)
		p->annex = DSL_ANNEX_ALM;
	p->sra = nvram_get_int("dslx_sra");
	p->bitswap = nvram_get_int("dslx_bitswap");
	p->ginp = nvram_get_int("dslx_ginp");
	p->snrm = nvram_get_int("dslx_snrm_offset");
	p->vdsl_profile = nvram_get_int("dslx_vdsl_profile");
	if (p->vdsl_profile >= VDSL_PROFILE_MAX)
		p->vdsl_profile = VDSL_PROFILE_ALL;
}

void start_dsl()
{
	mkdir("/tmp/adsl", S_IRUSR | S_IWUSR | S_IXUSR);

	/// xDSL driver, settings ..
	config_xdsl();

	/// xDSL interface
	config_xtm();

	/// lan wan bridge for some iptv case
	config_stb_bridge();

	///
	system("dsld&");

	/// TODO: xDSL diagnostic
}

void stop_dsl()
{
}

#endif

int get_dsl_prefix_by_wan_unit(int wan_unit, char *prefix, size_t len)
{
	int ret = -1;
	int isPTM = nvram_match("dslx_transmode","ptm") ? 1 : 0;

	if (wan_unit < WAN_UNIT_MAX)
	{
#ifdef RTCONFIG_DUALWAN
		if (get_dualwan_by_unit(wan_unit) != WANS_DUALWAN_IF_DSL)
			ret = -1;
		else
#endif
		{
			if (isPTM)
				snprintf(prefix, len, "dsl8_");
			else
				snprintf(prefix, len, "dsl0_");
			ret = 0;
		}
	}
#if defined(RTCONFIG_MULTISERVICE_WAN)
	else if(wan_unit > WAN_UNIT_MULTISRV_BASE)
	{
		int srv_unit = get_ms_idx_by_wan_unit(wan_unit);
#ifdef RTCONFIG_DUALWAN
		int dw_unit = get_ms_base_unit(wan_unit);
		if (get_dualwan_by_unit(dw_unit) != WANS_DUALWAN_IF_DSL)
			ret = -1;
		else
#endif
		{
			if (isPTM)
				snprintf(prefix, len, "dsl8.%d_", srv_unit);
			else
				snprintf(prefix, len, "dsl%d_", srv_unit);
			ret = 0;
		}
	}
#endif

	return (ret);
}

void config_xdsl()
{
	if (nvram_get_int("dslx_diag_state") == 1 && nvram_get_int("success_start_service") == 1)
	{
		nvram_set("dsltmp_diag_confxdsl", "1");
		return;
	}

#if defined(RTCONFIG_DSL_REMOTE)
	eval("req_dsl_drv", "dslsetting");

#elif defined(RTCONFIG_DSL_HOST)
#ifdef RTCONFIG_DSL_BCM
	XDSL_PARAM xparam;
	memset(&xparam, 0, sizeof(xparam));
	get_xdsl_param(&xparam);
	set_xdsl_settings(&xparam);
#endif //RTCONFIG_DSL_BCM

	if (nvram_get_int("success_start_service"))
		nvram_set("dsltmp_syncloss_apply", "1");
#endif
}

#ifdef RTCONFIG_DSL_REMOTE
void config_host_interface()
{
	char buf[16] = {0};
	char name[16] = {0};
	int base_wan_unit = WAN_UNIT_FIRST;
	int i;
	int mr_mswan_idx = nvram_get_int("mr_mswan_idx");
	int real_mr_mswan_idx = 0;

#ifdef RTCONFIG_DUALWAN
	if (get_dualwan_primary() == WANS_DUALWAN_IF_DSL)
		base_wan_unit = WAN_UNIT_FIRST;
	else if (get_dualwan_secondary() == WANS_DUALWAN_IF_DSL)
		base_wan_unit = WAN_UNIT_SECOND;
	else
		return;
#else
	base_wan_unit = WAN_UNIT_FIRST;
#endif

#if 0
	//remove all virtual interface
	for (i = 0; i < MAX_PVC; i++)
	{
		snprintf(buf, sizeof(buf), "vlan%d", DSL_WAN_VID + i);
		eval("vconfig", "rem", buf);
		usleep(100000);
	}
#endif

	// add virtual interface
#ifdef DSL_AC68U
	eval("vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");
#elif defined(DSL_N55U) || defined(DSL_N55U_B)
	eval("vconfig", "set_name_type", "DEV_PLUS_VID_NO_PAD");
#endif
	if (nvram_match("dslx_transmode", "atm"))
	{
		for (i = 0; i < MAX_PVC; i++)
		{
			snprintf(buf, sizeof(buf), "dsl%d_enable", i);
			if (nvram_get_int(buf) == 0)
				continue;
			snprintf(buf, sizeof(buf), "%d", DSL_WAN_VID + i);
			eval("vconfig", "add", ETH_WAN_BASE_IFNAME, buf);
			snprintf(buf, sizeof(buf), "%s%d", ETH_WAN_IFNAME_PREFIX, DSL_WAN_VID + i);
			snprintf(name, sizeof(name), "wan%d_ifname", get_ms_wan_unit(base_wan_unit, i));
			nvram_set(name, buf);

			if (i == mr_mswan_idx && base_wan_unit == WAN_UNIT_FIRST)
				real_mr_mswan_idx = mr_mswan_idx;
		}
	}
	else
	{
		for (i = 0; i < MAX_PVC; i++)
		{
			if (i)
				snprintf(buf, sizeof(buf), "dsl8.%d_enable", i);
			else
				snprintf(buf, sizeof(buf), "dsl8_enable");
			if (nvram_get_int(buf) == 0)
				continue;
			snprintf(buf, sizeof(buf), "%d", DSL_WAN_VID + i);
			eval("vconfig", "add", ETH_WAN_BASE_IFNAME, buf);
			snprintf(buf, sizeof(buf), "vlan%d", DSL_WAN_VID + i);
			snprintf(name, sizeof(name), "wan%d_ifname", get_ms_wan_unit(base_wan_unit, i));
			nvram_set(name, buf);

			if (i == mr_mswan_idx && base_wan_unit == WAN_UNIT_FIRST)
				real_mr_mswan_idx = mr_mswan_idx;
		}
	}

	// Only WAN_UNIT_FIRST for igmp/udp proxy by definition, not configurable.
	if (base_wan_unit == WAN_UNIT_FIRST)
	{
		snprintf(buf, sizeof(buf), "wan%d_ifname", get_ms_wan_unit(base_wan_unit, real_mr_mswan_idx));
		nvram_set("iptv_ifname", nvram_safe_get(buf));
	}
}
#endif

void config_xtm()
{
#if defined(RTCONFIG_DSL_REMOTE)
	eval("req_dsl_drv", "reloadpvc");
	config_host_interface();

#elif defined(RTCONFIG_DSL_HOST)
#ifdef RTCONFIG_DSL_BCM
	int i = 0;
	MSWAN_PARAM mparam;
	XTM_PARAM xparam;
	char dsl_prefix[16] = {0};
	int mr_mswan_idx = nvram_get_int("mr_mswan_idx");
	int real_mr_mswan_idx = 0;
	char wan_ifnames[32] = {0};
	char *p = NULL;

	eval("xtmctl", "start");

	set_xtm_intf();

	if(!nvram_match("dsltmp_adslsyncsts", "up"))
		return;

	memset(&mparam, 0, sizeof(mparam));

#ifdef RTCONFIG_DUALWAN
	if (get_dualwan_primary() == WANS_DUALWAN_IF_DSL)
		mparam.base_wan_unit = WAN_UNIT_FIRST;
	else if (get_dualwan_secondary() == WANS_DUALWAN_IF_DSL)
		mparam.base_wan_unit = WAN_UNIT_SECOND;
	else
		return;
#else
	mparam.base_wan_unit = WAN_UNIT_FIRST;
#endif

#ifdef RTCONFIG_BCM_OAM
	if (mparam.base_wan_unit == WAN_UNIT_FIRST)
		stop_oam();
#endif

	clean_mswan_vitf(mparam.base_wan_unit);

	// check mr_mswan_idx is enabled.
	// Only WAN_UNIT_FIRST for igmp proxy by definition, not configurable.
	if (mparam.base_wan_unit == WAN_UNIT_FIRST)
	{
		if (nvram_match("dslx_transmode", "atm"))
		{
			for (i = 0; i < MAX_PVC; i++)
			{
				snprintf(dsl_prefix, sizeof(dsl_prefix), "dsl%d_", i);
				if (nvram_pf_get_int(dsl_prefix, "enable"))
				{
					if (i == mr_mswan_idx)
						real_mr_mswan_idx = mr_mswan_idx;
				}
			}
		}
		else
		{
			for (i = 0; i < MAX_PVC; i++)
			{
				if (i)
					snprintf(dsl_prefix, sizeof(dsl_prefix), "dsl8.%d_", i);
				else
					snprintf(dsl_prefix, sizeof(dsl_prefix), "dsl8_");
				if (nvram_pf_get_int(dsl_prefix, "enable"))
				{
					if (i == mr_mswan_idx)
						real_mr_mswan_idx = mr_mswan_idx;
				}
			}
		}
	}

	if(nvram_match("dslx_transmode", "atm"))
	{
		for(i = 0; i < MAX_PVC; i++)
		{
			memset(&xparam, 0, sizeof(xparam));
			get_atm_param(&xparam, i);
			if(xparam.enable)
			{
				set_atm_tdte(&xparam, i);
				set_atm_conn(&xparam, i);

				if (!strcmp(xparam.proto, "pppoa") || !strcmp(xparam.proto, "ipoa"))
				{
					char wan_prefix[16] = {0};
					snprintf(wan_prefix, sizeof(wan_prefix), "wan%d_", get_ms_wan_unit(mparam.base_wan_unit, i));
					nvram_pf_set(wan_prefix, "ifname", xparam.phy_ifname);
					continue;
				}

				// virtual interface
				mparam.unit = i;
				mparam.enable = xparam.enable;
				strlcpy(mparam.proto, xparam.proto, sizeof(mparam.proto));
				mparam.dot1q = xparam.dot1q;
				mparam.vid = xparam.vid;
				mparam.dot1p = xparam.dot1p;
				mparam.total_config = xparam.total_config;
				strlcpy(mparam.base_ifname, xparam.phy_ifname, sizeof(mparam.base_ifname));
				if (mparam.base_wan_unit == WAN_UNIT_FIRST && i == real_mr_mswan_idx)
					mparam.mcast = 1;
				else
					mparam.mcast = 0;

				set_mswan_vitf(&mparam);
			}
		}

		// update wan_ifnames to atm
		nvram_safe_get_r("wan_ifnames", wan_ifnames, sizeof(wan_ifnames));
		p = strstr(wan_ifnames, DSL_WAN_PTM_IF);
		if (p)
		{
			strlcpy(p, DSL_WAN_ATM_IF, sizeof(wan_ifnames));
			nvram_set("wan_ifnames", wan_ifnames);
		}
	}
	else //PTM
	{
		for(i = 0; i < MAX_PVC; i++)
		{
			memset(&xparam, 0, sizeof(xparam));
			get_ptm_param(&xparam, i);
			if(xparam.enable)
			{
				set_ptm_conn(&xparam, i);

				// virtual interface
				mparam.unit = i;
				mparam.enable = xparam.enable;
				strlcpy(mparam.proto, xparam.proto, sizeof(mparam.proto));
				mparam.dot1q = xparam.dot1q;
				mparam.vid = xparam.vid;
				mparam.dot1p = xparam.dot1p;
				mparam.total_config = xparam.total_config;
				strlcpy(mparam.base_ifname, xparam.phy_ifname, sizeof(mparam.base_ifname));
				if (mparam.base_wan_unit == WAN_UNIT_FIRST && i == real_mr_mswan_idx)
					mparam.mcast = 1;
				else
					mparam.mcast = 0;

				set_mswan_vitf(&mparam);
			}
		}

		// update wan_ifnames to ptm
		nvram_safe_get_r("wan_ifnames", wan_ifnames, sizeof(wan_ifnames));
		p = strstr(wan_ifnames, DSL_WAN_ATM_IF);
		if (p)
		{
			strlcpy(p, DSL_WAN_PTM_IF, sizeof(wan_ifnames));
			nvram_set("wan_ifnames", wan_ifnames);
		}
	}
#ifdef RTCONFIG_BCM_OAM
	if (mparam.base_wan_unit == WAN_UNIT_FIRST)
		start_oam();
#endif
#endif //RTCONFIG_DSL_BCM
#endif
}

void config_stb_bridge()
{
	int s;
	struct ifreq ifr;
	char br_ifname[16] = {STB_BR_IF};
	char lan_ifname[16] = {0};
	int stbport = atoi(nvram_safe_get("switch_stb_x"));

	snprintf(lan_ifname, sizeof(lan_ifname), "%s", nvram_safe_get("lan_ifname"));

	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
		return;

	// check bridge exist
	strncpy(ifr.ifr_name, br_ifname, IFNAMSIZ);
	if (ioctl(s, SIOCGIFFLAGS, &ifr))
	{
		char buf_ip[16] = {0};

		eval("brctl", "addbr", br_ifname);
		pick_a_random_ipv4(buf_ip, sizeof(buf_ip));
		ifconfig(br_ifname, IFUP, buf_ip, "255.255.0.0");
#ifdef RTCONFIG_DSL_BCM
		eval("bcmmcastctl", "mode", "-i", br_ifname, "-p" , "1", "-m", "0");
		eval("bcmmcastctl", "mode", "-i", br_ifname, "-p" , "2", "-m", "0");
#endif
	}
	close(s);

	// move stb port to target bridge
	switch(get_model())
	{
		case MODEL_DSLAX82U:
		{
			eval("brctl", "delif", br_ifname, "eth0");
			eval("brctl", "delif", br_ifname, "eth1");
			eval("brctl", "delif", br_ifname, "eth2");
			eval("brctl", "delif", br_ifname, "eth3");
			eval("brctl", "addif", lan_ifname, "eth0");
			eval("brctl", "addif", lan_ifname, "eth1");
			eval("brctl", "addif", lan_ifname, "eth2");
			eval("brctl", "addif", lan_ifname, "eth3");
			switch(stbport)
			{
			case 1:
				eval("brctl", "delif", lan_ifname, "eth3");
				eval("brctl", "addif", br_ifname, "eth3");
				break;
			case 2:
				eval("brctl", "delif", lan_ifname, "eth2");
				eval("brctl", "addif", br_ifname, "eth2");
				break;
			case 3:
				eval("brctl", "delif", lan_ifname, "eth1");
				eval("brctl", "addif", br_ifname, "eth1");
				break;
			case 4:
				eval("brctl", "delif", lan_ifname, "eth0");
				eval("brctl", "addif", br_ifname, "eth0");
				break;
			case 5:
				eval("brctl", "delif", lan_ifname, "eth3");
				eval("brctl", "addif", br_ifname, "eth3");
				eval("brctl", "delif", lan_ifname, "eth2");
				eval("brctl", "addif", br_ifname, "eth2");
				break;
			case 6:
				eval("brctl", "delif", lan_ifname, "eth1");
				eval("brctl", "addif", br_ifname, "eth1");
				eval("brctl", "delif", lan_ifname, "eth0");
				eval("brctl", "addif", br_ifname, "eth0");
				break;
			}
			break;
		}
		case MODEL_DSLAC68U:
		{
			eval("brctl", "addif", br_ifname, "vlan3");
			break;
		}
		case MODEL_DSLN55U:
		{
			eval("brctl", "addif", br_ifname, "eth2.3");
			break;
		}
	}

}

void config_wan_bridge(char *br_ifname, char *wan_ifname, int add)
{
	if (br_ifname && wan_ifname)
	{
		if (add)
			eval("brctl", "addif", br_ifname, wan_ifname);
		else
			eval("brctl", "delif", br_ifname, wan_ifname);
	}

}