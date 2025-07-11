#define _GNU_SOURCE

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <bcmnvram.h>
#include <bcmdevs.h>
#include <shutils.h>
#include <shared.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef RTCONFIG_BROOP
#include <linux/netlink.h>
#include <rtstate.h>
#endif
#include <limits.h>	//PATH_MAX

static char *wantype_str[] = {
	[WANS_DUALWAN_IF_LAN] = "lan",
	[WANS_DUALWAN_IF_2G] = "2g",
	[WANS_DUALWAN_IF_5G] = "5g",
	[WANS_DUALWAN_IF_USB] = "usb",
	[WANS_DUALWAN_IF_DSL] = "dsl",
	[WANS_DUALWAN_IF_WAN] = "wan",
	[WANS_DUALWAN_IF_WAN2] = "wan2",
	[WANS_DUALWAN_IF_USB2] = "usb2",
	[WANS_DUALWAN_IF_SFPP] = "sfp+",
};

/* keyword for rc_support 	*/
/* ipv6 mssid update parental 	*/

void add_rc_support(char *feature)
{
	char *rcsupport, *features;

	if (!(feature && *feature))
		return;

	rcsupport = nvram_safe_get("rc_support");
	if (*rcsupport) {
		if (asprintf(&features, "%s %s", rcsupport, feature) < 0 || !features) {
			_dprintf("add_rc_support fail\n");
			return;
		}
		nvram_set("rc_support", features);
		free(features);
	} else
		nvram_set("rc_support", feature);
}

void del_rc_support(char *feature)
{
	char *rcsupport, *features;
	char word[256], *next;

	if (!(feature && *feature))
		return;

	rcsupport = nvram_safe_get("rc_support");
	if (*rcsupport) {
		features = strdup(rcsupport);
		if (!features) {
			_dprintf("del_rc_support fail\n");
			return;
		}
		foreach(word, feature, next)
			remove_from_list(word, features, strlen(features) + 1);
		if (strcmp(rcsupport, features) != 0)
			nvram_set("rc_support", features);
		free(features);
	}
}

#if defined(RTCONFIG_DUALWAN)
/**
 * is_nat_enabled() for dual/multiple WAN.
 * In Single WAN mode or Dual WAN Fail-Over/Fail-Back mode, check primary WAN.
 * In Dual WAN load-balance mode, check all WAN.
 */
int is_nat_enabled(void)
{
	int i, nr_nat = 0, sw_mode = nvram_get_int("sw_mode");
	char prefix[sizeof("wanX_XXXXXX")];

	if (sw_mode != SW_MODE_ROUTER && sw_mode != SW_MODE_HOTSPOT)
		return 0;

	if (get_nr_wan_unit() >= 2 && nvram_match("wans_mode", "lb")) {
		/* Dual WAN LB, check all WAN unit. */
		for (i = WAN_UNIT_FIRST; i < WAN_UNIT_MAX; ++i) {
			snprintf(prefix, sizeof(prefix), "wan%d_", i);
			if (nvram_pf_get_int(prefix, "nat_x") == 1)
				nr_nat++;
		}
	} else {
		/* Single WAN/Dual WAN FO/FB, check primary WAN unit only. */
		snprintf(prefix, sizeof(prefix), "wan%d_", wan_primary_ifunit());
		nr_nat = nvram_pf_get_int(prefix, "nat_x");
	}

	return (nr_nat > 0)? 1 : 0;
}
#endif

int get_wan_state(int unit){
	char tmp[100], prefix[16];

	snprintf(prefix, 16, "wan%d_", unit);

	return nvram_get_int(strlcat_r(prefix, "state_t", tmp, sizeof(tmp)));
}

int get_wan_sbstate(int unit){
	char tmp[100], prefix[16];

	snprintf(prefix, 16, "wan%d_", unit);

	return nvram_get_int(strlcat_r(prefix, "sbstate_t", tmp, sizeof(tmp)));
}

int get_wan_auxstate(int unit){
	char tmp[100], prefix[16];

	snprintf(prefix, 16, "wan%d_", unit);

	return nvram_get_int(strlcat_r(prefix, "auxstate_t", tmp, sizeof(tmp)));
}

char *link_wan_nvname(int unit, char *buf, int size){
	if(buf == NULL)
		return NULL;

	if(unit == WAN_UNIT_FIRST)
		snprintf(buf, size, "link_wan");
	else
		snprintf(buf, size, "link_wan%d", unit);

#ifdef RTCONFIG_MULTISERVICE_WAN
	if (unit > WAN_UNIT_MULTISRV_BASE)
	{
		int base_unit = get_ms_base_unit(unit);
		if (base_unit == WAN_UNIT_FIRST)
			snprintf(buf, size, "link_wan");
		else
			snprintf(buf, size, "link_wan%d", base_unit);
	}
#endif

	return buf;
}

int is_internet_connect(int unit){
	int link_internet, wan_auxstate;

	if(!is_wan_connect(unit))
		return 0;

	if(nvram_match("wans_mode", "lb")){
		link_internet = nvram_get_int("link_internet");
		if(link_internet == 2)
			return 1;
	}
	else{
		wan_auxstate = get_wan_auxstate(unit);
		if(wan_auxstate == WAN_AUXSTATE_NONE)
			return 1;
	}

	return 0;
}

int is_wan_connect(int unit){
	int wan_state, wan_sbstate, wan_auxstate;

	if(!is_phy_connect(unit))
		return 0;

	wan_state = get_wan_state(unit);
	wan_sbstate = get_wan_sbstate(unit);
	wan_auxstate = get_wan_auxstate(unit);

	if(wan_state == WAN_STATE_CONNECTED && wan_sbstate == WAN_STOPPED_REASON_NONE &&
			(wan_auxstate == WAN_AUXSTATE_NONE || wan_auxstate == WAN_AUXSTATE_NO_INTERNET_ACTIVITY)
			)
		return 1;
	else
		return 0;
}

// auxstate will be reset by update_wan_state(), but wanduck cannot set it soon sometimes.
// only link_wan will be safe.
int is_phy_connect(int unit){
	char prefix[sizeof("link_wanXXXXXX")], *ptr;
	int link_wan;

	link_wan_nvname(unit, prefix, sizeof(prefix));

	ptr = nvram_safe_get(prefix);
	if(strlen(ptr) > 0){
		link_wan = atoi(ptr);

		if(link_wan)
			return 1;
		else
		{
#ifdef RTCONFIG_DSL
			int wan_type = get_dualwan_by_unit(unit);
			if (wan_type == WANS_DUALWAN_IF_WAN || wan_type == WANS_DUALWAN_IF_LAN)
				return get_wanports_status(unit);
#endif
			return 0;
		}
	}
	else
#ifdef RTCONFIG_USB_MODEM
	if(dualwan_unit__usbif(unit))
		return 1;
	else
#endif
		return get_wanports_status(unit);
}

int is_phy_connect2(int unit){
#ifdef RTCONFIG_USB_MODEM
	if(dualwan_unit__usbif(unit))
		return 1;
	else
#endif
		return get_wanports_status(unit);
}

int is_ip_conflict(int unit){
	int wan_state, wan_sbstate;

	wan_state = get_wan_state(unit);
	wan_sbstate = get_wan_sbstate(unit);

	if(wan_state == WAN_STATE_STOPPED && wan_sbstate == WAN_STOPPED_REASON_INVALID_IPADDR)
		return 1;
	else
		return 0;
}

// get wan_unit from device ifname or hw device ifname
#if 0
int get_wan_unit(char *ifname)
{
	char word[256], tmp[100], *next;
	char prefix[32]="wanXXXXXX_";
	int unit, found = 0;

	unit = WAN_UNIT_FIRST;

	foreach (word, nvram_safe_get("wan_ifnames"), next) {
		if(strncmp(ifname, "ppp", 3)==0) {
			snprintf(prefix, sizeof(prefix), "wan%d_", unit);
			if(strcmp(nvram_safe_get(strlcat_r(prefix, "pppoe_ifname", tmp, sizeof(tmp))), ifname)==0) {
				found = 1;
			}	
		}
		else if(strcmp(ifname, word)==0) {
			found = 1;
		}
		if(found) break;
		unit ++;
	}

	if(!found) unit = WAN_UNIT_FIRST;
	return unit;
}
#else
int get_wan_unit(char *ifname)
{
	char tmp[100], prefix[sizeof("wanXXXXXXXXXX_")];
	int unit, wan_proto;

	if (ifname == NULL || *ifname == '\0')
		return -1;

#ifdef BLUECAVE
	/* TODO: unclear, why this workaround is required? */
	int bluecave = (get_model() == MODEL_BLUECAVE &&
			strcmp(ifname, "eth1") == 0 && nvram_get_int("switch_stb_x") > 0);
#endif

	for (unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; unit++) {
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);
		wan_proto = get_wan_proto(prefix);

		switch (wan_proto) {
		case WAN_PPPOE:
		case WAN_PPTP:
		case WAN_L2TP:
#ifdef RTCONFIG_SOFTWIRE46
		case WAN_LW4O6:
		case WAN_MAPE:
		case WAN_V6PLUS:
		case WAN_OCNVC:
		case WAN_V6OPTION:
		case WAN_DSLITE:
#endif
			if (nvram_match(strlcat_r(prefix, "pppoe_ifname", tmp, sizeof(tmp)), ifname))
				return unit;
#ifdef RTCONFIG_USB_MODEM
			if (dualwan_unit__usbif(unit))
				break;
#endif
			/* fall through */
		default:
			if (nvram_match(strlcat_r(prefix, "ifname", tmp, sizeof(tmp)), ifname))
				return unit;
#ifdef BLUECAVE
			if (bluecave)
				return unit;
#endif
			break;
		}
	}

#ifdef RTCONFIG_MULTIWAN_IF
	for(unit = MULTI_WAN_START_IDX; unit < MULTI_WAN_START_IDX + MAX_MULTI_WAN_NUM; unit++) {
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);
		wan_proto = get_wan_proto(prefix);

		switch (wan_proto) {
		case WAN_PPPOE:
		case WAN_PPTP:
		case WAN_L2TP:
#ifdef RTCONFIG_SOFTWIRE46
		case WAN_LW4O6:
		case WAN_MAPE:
		case WAN_V6PLUS:
#endif
			if (nvram_match(strlcat_r(prefix, "pppoe_ifname", tmp, sizeof(tmp)), ifname))
				return unit;
#ifdef RTCONFIG_USB_MODEM
			if (dualwan_unit__usbif(unit))
				break;
#endif
			/* fall through */
		default:
			if (nvram_match(strlcat_r(prefix, "ifname", tmp, sizeof(tmp)), ifname))
				return unit;
#ifdef BLUECAVE
			if (bluecave)
				return unit;
#endif
			break;
		}
	}
#endif

	return -1;
}
#endif

// Get physical wan ifname of working connection
char *get_wanx_ifname(int unit)
{
	char *wan_ifname;
	char tmp[100], prefix[sizeof("wanXXXXXXXXXX_")];
	
	snprintf(prefix, sizeof(prefix), "wan%d_", unit);
	wan_ifname = nvram_safe_get(strlcat_r(prefix, "ifname", tmp, sizeof(tmp)));

	return wan_ifname;
}

// Get wan ifname of working connection
char *get_wan_ifname(int unit)
{
	char tmp[100], prefix[sizeof("wanXXXXXXXXXX_")];
	char *wan_ifname;
	int wan_proto;

	snprintf(prefix, sizeof(prefix), "wan%d_", unit);
	wan_proto = get_wan_proto(prefix);

#ifdef RTCONFIG_USB_MODEM
	if (dualwan_unit__usbif(unit)) {
		wan_ifname = (wan_proto == WAN_DHCP) ?
			nvram_safe_get(strlcat_r(prefix, "ifname", tmp, sizeof(tmp))) :
			nvram_safe_get(strlcat_r(prefix, "pppoe_ifname", tmp, sizeof(tmp)));
	} else
#endif
#if defined(RTAX82_XD6) || defined(RTAX82_XD6S) || defined(XD6_V2) || defined(ET12)
	if (!strncmp(nvram_safe_get("territory_code"), "CH", 2) &&
		nvram_match(ipv6_nvname("ipv6_only"), "1"))
		return nvram_safe_get(strlcat_r(prefix, "ifname", tmp, sizeof(tmp)));
	else
#endif
	switch (wan_proto) {
	case WAN_PPPOE:
	case WAN_PPTP:
	case WAN_L2TP:
#ifdef RTCONFIG_SOFTWIRE46
	case WAN_LW4O6:
	case WAN_MAPE:
#endif
		wan_ifname = nvram_safe_get(strlcat_r(prefix, "pppoe_ifname", tmp, sizeof(tmp)));
		break;
#ifdef RTCONFIG_SOFTWIRE46
	case WAN_V6PLUS:
	case WAN_OCNVC:
	case WAN_V6OPTION:
	case WAN_DSLITE:
		if (nvram_pf_get_int(prefix, "s46_hgw_case") >= S46_CASE_MAP_HGW_OFF) {
			wan_ifname = nvram_safe_get(strlcat_r(prefix, "pppoe_ifname", tmp, sizeof(tmp)));
			break;
		}
#endif
	default:
		wan_ifname = nvram_safe_get(strlcat_r(prefix, "ifname", tmp, sizeof(tmp)));
		break;
	}

	return wan_ifname;
}

// Get wan ipv6 ifname of working connection
#ifdef RTCONFIG_IPV6
char *get_wan6_ifname(int unit)
{
	char tmp[100], prefix[sizeof("wanXXXXXXXXXX_")];
	char *wan_ifname;
	int wan_proto;
	static char tunnel_ifname[IFNAMSIZ] = {0};

	switch (get_ipv6_service_by_unit(unit)) {
	case IPV6_NATIVE_DHCP:
	case IPV6_MANUAL:
#ifdef RTCONFIG_6RELAYD
	case IPV6_PASSTHROUGH:
#endif
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);
		wan_proto = get_wan_proto(prefix);

#ifdef RTCONFIG_USB_MODEM
		if (dualwan_unit__usbif(unit)) {
			wan_ifname = (wan_proto == WAN_DHCP) ?
				nvram_safe_get(strlcat_r(prefix, "ifname", tmp, sizeof(tmp))) :
				nvram_safe_get(strlcat_r(prefix, "pppoe_ifname", tmp, sizeof(tmp)));
		} else
#endif
		switch (wan_proto) {
		case WAN_PPPOE:
		case WAN_PPTP:
		case WAN_L2TP:
			if (nvram_match(ipv6_nvname_by_unit("ipv6_ifdev", unit), "ppp")) {
				wan_ifname = nvram_safe_get(strlcat_r(prefix, "pppoe_ifname", tmp, sizeof(tmp)));
				break;
			}
			/* fall through */
		default:
			wan_ifname = nvram_safe_get(strlcat_r(prefix, "ifname", tmp, sizeof(tmp)));
			break;
		}
		break;
	case IPV6_6TO4:
	case IPV6_6IN4:
	case IPV6_6RD:
		snprintf(tunnel_ifname, sizeof(tunnel_ifname), "v6tun%d", unit);
		wan_ifname = tunnel_ifname;
		break;
	default:
		return "";
	}

	return wan_ifname;
}

int get_wan6_unit(char* ifname)
{
	if (!ifname)
		return 0;

	if (!strncmp(ifname, "v6tun", 5)) {
		return (strlen(ifname) > 5) ? atoi(ifname+5) : 0;
	}
	else {
		return get_wan_unit(ifname);
	}
}
#endif

int get_ports_status(unsigned int port_mask)
{
#if defined(RTCONFIG_QCA) && defined(RTCONFIG_DETWAN)
	return rtkswitch_Port_phyStatus(port_mask);
#else
	return -1;
#endif
}

// OR all lan port status
int get_lanports_status(void)
{
	return lanport_status();
}

// OR all wan port status
int get_wanports_status(int wan_unit)
{
// 1. PHY type, 2. factory owner, 3. model.
#ifdef RTCONFIG_DSL
#ifdef RTCONFIG_DUALWAN
	if(get_dualwan_by_unit(wan_unit) == WANS_DUALWAN_IF_DSL)
#endif
	{
		if (nvram_match("dsltmp_adslsyncsts","up")) return 1;
		return 0;
	}
#ifdef RTCONFIG_DUALWAN
	if( get_dualwan_by_unit(wan_unit) == WANS_DUALWAN_IF_LAN
	 || get_dualwan_by_unit(wan_unit) == WANS_DUALWAN_IF_WAN
	)
	{
	#ifdef RTCONFIG_RALINK
		return rtkswitch_wanPort_phyStatus(wan_unit); //Paul modify 2012/12/4
	#else
		return wanport_status(wan_unit) ? 1 : 0;
	#endif
	}
#endif
	// TO CHENI:
	// HOW TO HANDLE USB?	
#else // RJ-45
#if defined(RTCONFIG_RALINK) || defined(RTCONFIG_QCA)
	return rtkswitch_wanPort_phyStatus(wan_unit);
#else
	return wanport_status(wan_unit);
#endif
#endif
}

int get_usb_modem_state(){
	if(!strcmp(nvram_safe_get("modem_running"), "1"))
		return 1;
	else
		return 0;
}

int set_usb_modem_state(const int flag){
	if(flag != 1 && flag != 0)
		return 0;

	if(flag){
		nvram_set("modem_running", "1");
		return 1;
	}
	else{
		nvram_set("modem_running", "0");
		return 0;
	}
}

int
set_wan_primary_ifunit(const int unit)
{
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	int i;

	if (unit < WAN_UNIT_FIRST || unit >= WAN_UNIT_MAX)
		return -1;

	nvram_set_int("wan_primary", unit);
	for (i = WAN_UNIT_FIRST; i < WAN_UNIT_MAX; i++) {
		snprintf(prefix, sizeof(prefix), "wan%d_", i);
		nvram_set_int(strlcat_r(prefix, "primary", tmp, sizeof(tmp)), (i == unit) ? 1 : 0);
	}

	return 0;
}

int
wan_primary_ifunit(void)
{
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	int unit;

	/* TODO: Why not just nvram_get_int("wan_primary")? */
	for (unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; unit ++) {
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);
		if (nvram_match(strlcat_r(prefix, "primary", tmp, sizeof(tmp)), "1"))
			return unit;
	}

#ifdef RTCONFIG_MULTIWAN_PROFILE
	for (unit = MULTI_WAN_START_IDX; unit < MULTI_WAN_START_IDX + MAX_MULTI_WAN_NUM; unit++) {
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);
		if (nvram_match(strlcat_r(prefix, "primary", tmp, sizeof(tmp)), "1"))
			return unit;
	}
#endif

	return 0;
}

#ifdef RTCONFIG_REALTEK
/* The fuction is avoiding watchdog segfault on RP-AC68U.
 * This is a workaround solution.
 * */
int
rtk_wan_primary_ifunit(void)
{
	return wan_primary_ifunit();
}
#endif

int
wan_primary_ifunit_ipv6(void)
{
#ifdef RTCONFIG_DUALWAN
#if defined(RTCONFIG_MULTIWAN_CFG)
	int unit = wan_primary_ifunit();

	if (!strstr(nvram_safe_get("wans_dualwan"), "none")
#ifdef RTCONFIG_IPV6
	    && get_ipv6_service_by_unit(unit) == IPV6_DISABLED
#endif
	)
		return (1 - unit);

	return unit;
#else
	return 0;
#endif
#else
	return wan_primary_ifunit();
#endif
}

#ifdef RTCONFIG_MEDIA_SERVER
void
set_invoke_later(int flag)
{
	nvram_set_int("invoke_later", nvram_get_int("invoke_later")|flag);
}

int
get_invoke_later()
{
	return(nvram_get_int("invoke_later"));
}
#endif	/* RTCONFIG_MEDIA_SERVER */

#ifdef RTCONFIG_USB
#if defined(RTCONFIG_ALPINE) || defined(RTCONFIG_LANTIQ)
char *get_usb_xhci_port(int port){
	if(port == 2)
		return USB_XHCI_PORT_2;
	else
		return USB_XHCI_PORT_1;
}

char *get_usb_ehci_port(int port){
	if(port == 3)
		return USB_EHCI_PORT_3;
	else if(port == 2)
		return USB_EHCI_PORT_2;
	else
		return USB_EHCI_PORT_1;
}

char *get_usb_ohci_port(int port){
	if(port == 3)
		return USB_OHCI_PORT_3;
	else if(port == 2)
		return USB_OHCI_PORT_2;
	else
		return USB_OHCI_PORT_1;
}

int get_usb_port_number(const char *usb_port){
	int i;

	for(i = 1; i <= 2; ++i){
		if(!strcmp(usb_port, get_usb_xhci_port(i))){
			return i;
		}
	}

	for(i = 1; i <= 3; ++i){
		if(!strcmp(usb_port, get_usb_ehci_port(i))){
			return i;
		}
	}

	for(i = 1; i <= 3; ++i){
		if(!strcmp(usb_port, get_usb_ohci_port(i))){
			return i;
		}
	}

	return 0;
}

int get_usb_port_host(const char *usb_port){
	int i;

	for(i = 1; i <= 2; ++i){
		if(!strcmp(usb_port, get_usb_xhci_port(i))){
			return USB_HOST_XHCI;
		}
	}

	for(i = 1; i <= 3; ++i){
		if(!strcmp(usb_port, get_usb_ehci_port(i))){
			return USB_HOST_EHCI;
		}
	}

	for(i = 1; i <= 3; ++i){
		if(!strcmp(usb_port, get_usb_ohci_port(i))){
			return USB_HOST_OHCI;
		}
	}

	return USB_HOST_NONE;
}
#else
char xhci_string[32];
char ehci_string[32];
char ohci_string[32];

char *get_usb_xhci_port(int port)
{
	char word[100], *next;
	int i=0;

	strlcpy(xhci_string, "xxxxxxxx", sizeof(xhci_string));

	foreach(word, nvram_safe_get("xhci_ports"), next){
		if(i == port){
			strlcpy(xhci_string, word, sizeof(xhci_string));
			break;
		}
		i++;
	}
	return xhci_string;
}

char *get_usb_ehci_port(int port)
{
	char word[100], *next;
	int i=0;

	strlcpy(ehci_string, "xxxxxxxx", sizeof(ehci_string));

	foreach(word, nvram_safe_get("ehci_ports"), next) {
		if(i==port) {
			strlcpy(ehci_string, word, sizeof(ehci_string));
			break;
		}		
		i++;
	}
	return ehci_string;
}

char *get_usb_ohci_port(int port)
{
	char word[100], *next;
	int i=0;

	strlcpy(ohci_string, "xxxxxxxx", sizeof(ohci_string));

	foreach(word, nvram_safe_get("ohci_ports"), next) {
		if(i==port) {
			strlcpy(ohci_string, word, sizeof(ohci_string));
			break;
		}		
		i++;
	}
	return ohci_string;
}

int get_usb_port_number(const char *usb_port)
{
	char word[100], *next;
	int i;

	i = 0;
	foreach(word, nvram_safe_get("xhci_ports"), next){
		++i;
		if(!strcmp(usb_port, word)){
			return i;
		}
	}

	i = 0;
	foreach(word, nvram_safe_get("ehci_ports"), next){
		++i;
		if(!strcmp(usb_port, word)){
			return i;
		}
	}

	i = 0;
	foreach(word, nvram_safe_get("ohci_ports"), next){
		++i;
		if(!strcmp(usb_port, word)){
			return i;
		}
	}

	return 0;
}

int get_usb_port_host(const char *usb_port)
{
	char word[100], *next;
	int i;

	i = 0;
	foreach(word, nvram_safe_get("xhci_ports"), next){
		++i;
		if(!strcmp(usb_port, word)){
			return USB_HOST_XHCI;
		}
	}

	i = 0;
	foreach(word, nvram_safe_get("ehci_ports"), next){
		++i;
		if(!strcmp(usb_port, word)){
			return USB_HOST_EHCI;
		}
	}

	i = 0;
	foreach(word, nvram_safe_get("ohci_ports"), next){
		++i;
		if(!strcmp(usb_port, word)){
			return USB_HOST_OHCI;
		}
	}

	return USB_HOST_NONE;
}
#endif
#endif // RTCONFIG_USB

void set_wanscap_support(char *feature)
{
	nvram_set("wans_cap", feature);
}

#if defined(RTCONFIG_DUALWAN)
void add_wanscap_support(char *feature)
{
	char features[128];
	int len;

	strlcpy(features, nvram_safe_get("wans_cap"), sizeof(features));

	if((len = strlen(features))==0)
		nvram_set("wans_cap", feature);
	else {
		len += sprintf(features + len, " %s", feature);
		nvram_set("wans_cap", features);
	}
}

int get_wans_cap(void)
{
	char wans_cap[64] = {0};
	char word[8] = {0};
	char *next = NULL;
	int caps = 0;

	strlcpy(wans_cap, nvram_safe_get("wans_cap"), sizeof(wans_cap));
	foreach(word, wans_cap, next) {
		if (!strcmp(word,"lan")) caps |= WANSCAP_LAN;
		if (!strcmp(word,"2g")) caps |= WANSCAP_2G;
		if (!strcmp(word,"5g")) caps |= WANSCAP_5G;
		if (!strcmp(word,"usb")) caps |= WANSCAP_USB;
		if (!strcmp(word,"dsl")) caps |= WANSCAP_DSL;
		if (!strcmp(word,"wan")) caps |= WANSCAP_WAN;
		if (!strcmp(word,"wan2")) caps |= WANSCAP_WAN2;
		if (!strcmp(word,"sfp+")) caps |= WANSCAP_SFPP;
		if (!strcmp(word,"6g")) caps |= WANSCAP_6G;
	}

	return caps;
}

int get_wans_dualwan_str(char *wancaps, int size){
	snprintf(wancaps, size, "%s", nvram_get("wans_dualwan") ? : (nvram_default_get("wans_dualwan") ? :
#ifdef RTCONFIG_DSL
				"dsl"
#elif defined(RTCONFIG_INTERNAL_GOBI) && defined(RTCONFIG_NO_WANPORT)
				"usb"
#else
				"wan"
#endif
				" "DEF_SECOND_WANIF
				)
			);

	return 0;
}

int get_wans_dualwan(void) 
{
	int caps=0;
	char word[80], *next;
	char *wancaps = nvram_get("wans_dualwan");

	if(wancaps == NULL)
	{
#ifdef RTCONFIG_DSL
		caps =  WANSCAP_DSL;
#elif defined(RTCONFIG_INTERNAL_GOBI) && defined(RTCONFIG_NO_WANPORT)
		caps = WANSCAP_USB;
#else
		caps = WANSCAP_WAN;
#endif
		wancaps = DEF_SECOND_WANIF;
	}

	foreach(word, wancaps, next) {
		if (!strcmp(word,"lan")) caps |= WANSCAP_LAN;
		if (!strcmp(word,"2g")) caps |= WANSCAP_2G;
		if (!strcmp(word,"5g")) caps |= WANSCAP_5G;
		if (!strcmp(word,"usb")) caps |= WANSCAP_USB;
		if (!strcmp(word,"dsl")) caps |= WANSCAP_DSL;
		if (!strcmp(word,"wan")) caps |= WANSCAP_WAN;
		if (!strcmp(word,"wan2")) caps |= WANSCAP_WAN2;
		if (!strcmp(word,"sfp+")) caps |= WANSCAP_SFPP;
		if (!strcmp(word,"6g")) caps |= WANSCAP_6G;
	}

	return caps;
}

static int _convert_wan_type(char *type)
{
	if(!type)
		return WANS_DUALWAN_IF_NONE;
	if (!strcmp(type,"lan"))
		return WANS_DUALWAN_IF_LAN;
	if (!strcmp(type,"2g"))
		return WANS_DUALWAN_IF_2G;
	if (!strcmp(type,"5g"))
		return WANS_DUALWAN_IF_5G;
	if (!strcmp(type,"usb"))
		return WANS_DUALWAN_IF_USB;
	if (!strcmp(type,"dsl"))
		return WANS_DUALWAN_IF_DSL;
	if (!strcmp(type,"wan"))
		return WANS_DUALWAN_IF_WAN;
	if (!strcmp(type,"wan2"))
		return WANS_DUALWAN_IF_WAN2;
#ifdef RTCONFIG_USB_MULTIMODEM
	if (!strcmp(type,"usb2"))
		return WANS_DUALWAN_IF_USB2;
#endif
	if (!strcmp(type,"sfp+"))
		return WANS_DUALWAN_IF_SFPP;
	return WANS_DUALWAN_IF_NONE;
}

int get_dualwan_by_unit(int unit) 
{
	int i;
	char word[80], *next;
	char *wans_dualwan = nvram_get("wans_dualwan");

	if(wans_dualwan == NULL)	//default value
	{
		wans_dualwan = nvram_default_get("wans_dualwan");
	}

#ifdef RTCONFIG_MULTICAST_IPTV
	if(unit == WAN_UNIT_IPTV)
		return WAN_UNIT_IPTV;
	if(unit == WAN_UNIT_VOIP)
		return WAN_UNIT_VOIP;
#endif

#ifdef RTCONFIG_MULTIWAN_IF
	if(unit >= MULTI_WAN_START_IDX && unit < MULTI_WAN_START_IDX + MAX_MULTI_WAN_NUM)
	{
		snprintf(word, sizeof(word), "wan%d_type", unit);
		return _convert_wan_type(nvram_safe_get(word));
	}
#ifdef RTCONFIG_MULTISERVICE_WAN
	int base_unit = get_ms_base_unit(unit);
	if(base_unit >= MULTI_WAN_START_IDX && base_unit < MULTI_WAN_START_IDX + MAX_MULTI_WAN_NUM)
	{
		snprintf(word, sizeof(word), "wan%d_type", base_unit);
		return _convert_wan_type(nvram_safe_get(word));
	}
#endif
#endif

	i = 0;
	foreach(word, wans_dualwan, next) {
		if(i==unit
#ifdef RTCONFIG_MULTISERVICE_WAN
			|| i == get_ms_base_unit(unit)
#endif
		) {
			return _convert_wan_type(word);
		}
		i++;
	}

	return WANS_DUALWAN_IF_NONE;
}

int get_wanunit_by_type(int wan_type){
	int unit;

	for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit){
		if(get_dualwan_by_unit(unit) == wan_type){
			return unit;
		}
	}

#ifdef RTCONFIG_MULTIWAN_IF
	char prefix[16];
	for(unit = MULTI_WAN_START_IDX; unit < MULTI_WAN_START_IDX + MAX_MULTI_WAN_NUM; ++unit) {
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);
		if (wan_type == _convert_wan_type(nvram_pf_safe_get(prefix, "type")))
			return unit;
	}
#endif

	return WAN_UNIT_NONE;
}

/* Return wan type string of @unit wan_unit.
 * @return:	pointer to a string.
 *  NULL:	invalid parameter or unknown wan type.
 */
char *get_wantype_str_by_unit(int unit)
{
	int type = get_dualwan_by_unit(unit);

	if (unit < 0 || unit > ARRAY_SIZE(wantype_str))
		return NULL;

	return wantype_str[type];
}

// imply: unit 0: primary, unit 1: secondary
int get_dualwan_primary(void)
{
	return get_dualwan_by_unit(0);
}

int get_dualwan_secondary(void) 
{
	return get_dualwan_by_unit(1);
}

/**
 * Return total number of WAN unit.
 * @return:
 */
int get_nr_wan_unit(void)
{
	int i, c = 0;

	for (i = WAN_UNIT_FIRST; i < WAN_UNIT_MAX; ++i) {
		if (get_dualwan_by_unit(i) != WANS_DUALWAN_IF_NONE)
			c++;
	}

	return c;
}
#endif	/* RTCONFIG_DUALWAN */

/**
 * Return number of enabled guest network of one/all band.
 * @band:
 *  >= 0:	calculate number of enabled guest network of specified band.
 *  <  0:	calculate number of enabled guest network of all band.
 * @return:	number of enabled guest network of one/all band.
 */
int get_nr_guest_network(int band)
{
	int i, j, c = 0, mode = get_model();
	char prefix[16];

	if (__repeater_mode(mode) || __mediabridge_mode(mode))
		return 0;

	/* 0:	2G
	 * 1:	5G
	 * 2:	5G-2, may not exist.
	 * 3:	Wigig=11ad, may not exist.
	 */
	for (i = 0; i < 4; ++i) {
		if (band >= 0 && band != i)
			continue;

		for (j = 1; j < MAX_NO_MSSID; ++j) {
			snprintf(prefix, sizeof(prefix), "wl%d.%d_", i, j);
			if (nvram_pf_match(prefix, "bss_enabled", "1"))
				c++;
		}
	}

	return c;
}

int get_gate_num(void)
{
	char prefix[] = "wanXXXXXXXXXX_", link_wan[sizeof("link_wanXXXXXX")];
	char wan_ip[32], wan_gate[32];
	int unit;
	int gate_num = 0;
	for (unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit){ // Multipath
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);
		strncpy(wan_ip, nvram_pf_safe_get(prefix, "ipaddr"), 32);
		strncpy(wan_gate, nvram_pf_safe_get(prefix, "gateway"), 32);

		// when wan_down().
#if defined(RTCONFIG_HND_ROUTER_BE_4916)
		if(!is_phy_connect2(unit))
			continue;
#else
		if(!is_wan_connect(unit))
			continue;

		/* We need to check link_wanX instead of wanX_state_t if this WAN unit is static IP. */
		if (nvram_pf_match(prefix, "proto", "static") && dualwan_unit__nonusbif(unit)) {
			if (unit == WAN_UNIT_FIRST)
				strlcpy(link_wan, "link_wan", sizeof(link_wan));
			else
				snprintf(link_wan, sizeof(link_wan), "link_wan%d", unit);

			if (!nvram_match(link_wan, "1"))
				continue;
		}
#endif

		if(strlen(wan_gate) <= 0 || !strcmp(wan_gate, "0.0.0.0"))
			continue;

		if(strlen(wan_ip) <= 0 || !strcmp(wan_ip, "0.0.0.0"))
			continue;

		++gate_num;
#ifndef	RTCONFIG_DUALWAN
		break;
#endif	/* RTCONFIG_DUALWAN */
	}
	return gate_num;
}

// no more to use
/*
void set_dualwan_type(char *type)
{
	nvram_set("wans_dualwan", type);
}

void add_dualwan_type(char *type)
{
	char types[128];
	int len;

	strcpy(types, nvram_safe_get("wans_dualwan"));

	if((len = strlen(types))==0) nvram_set("wans_dualwan", type);
	else {
		sprintf(types+len, " %s", type);
		nvram_set("wans_dualwan", types);
	}
}
*/

void set_lan_phy(char *phy)
{
	nvram_set("lan_ifnames", phy);
}

void add_lan_phy(char *phy)
{
	char phys[512], *ifnames;

	if (phy == NULL || *phy == '\0')
		return;

	ifnames = nvram_safe_get("lan_ifnames");
	if(find_word(ifnames, phy) != NULL)
		return;	/* exist */

	snprintf(phys, sizeof(phys), "%s%s%s", ifnames,
		(*ifnames && *phy) ? " " : "", phy);
	nvram_set("lan_ifnames", phys);
}

void del_lan_phy(const char *phy)
{
	char phys[512] = {0};
	const char *ifnames, *p;

	if (phy == NULL || *phy == '\0')
		return;

	ifnames = nvram_safe_get("lan_ifnames");
	if(!(p = find_word(ifnames, phy)))
		return;	// not exist
	strncpy(phys, ifnames, p - ifnames);
	strlcat(phys, p + strlen(phy) + 1, sizeof(phys));
	nvram_set("lan_ifnames", phys);
}

void set_wan_phy(char *phy)
{
	nvram_set("wan_ifnames", phy);
}

void add_wan_phy(char *phy)
{
	char phys[128], *ifnames;

	if (phy == NULL || *phy == '\0')
		return;

	ifnames = nvram_safe_get("wan_ifnames");
	if(find_word(ifnames, phy) != NULL)
		return;	/* exist */

	snprintf(phys, sizeof(phys), "%s%s%s", ifnames,
		(*ifnames && *phy) ? " " : "", phy);
	nvram_set("wan_ifnames", phys);
}

char *usb_modem_prefix(int modem_unit, char *prefix, int size)
{
	if (prefix == NULL)
		return NULL;

	if (modem_unit == MODEM_UNIT_FIRST)
		snprintf(prefix, size, "usb_modem_");
	else
		snprintf(prefix, size, "usb_modem%d_", modem_unit);

	return prefix;
}

#ifdef RTCONFIG_USB_MULTIMODEM
int get_modemunit_by_dev(const char *dev){
	int modem_unit;
	char tmp[100], prefix[32];

	for(modem_unit = MODEM_UNIT_FIRST; modem_unit < MODEM_UNIT_MAX; ++modem_unit){
		usb_modem_prefix(modem_unit, prefix, sizeof(prefix));

		if(!strcmp(dev, nvram_safe_get(strlcat_r(prefix, "act_dev", tmp, sizeof(tmp)))))
			return modem_unit;
	}

	return MODEM_UNIT_NONE;
}

int get_modemunit_by_node(const char *usb_node){
	int modem_unit;
	char tmp[100], prefix[32];

	for(modem_unit = MODEM_UNIT_FIRST; modem_unit < MODEM_UNIT_MAX; ++modem_unit){
		usb_modem_prefix(modem_unit, prefix, sizeof(prefix));

		if(!strcmp(usb_node, nvram_safe_get(strlcat_r(prefix, "act_path", tmp, sizeof(tmp)))))
			return modem_unit;
	}

	return MODEM_UNIT_NONE;
}
#else
inline int get_modemunit_by_dev(const char *dev){
	return MODEM_UNIT_FIRST;
}
inline int get_modemunit_by_node(const char *usb_node){
	return MODEM_UNIT_FIRST;
}
#endif

int get_modemunit_by_type(int wan_type){
	// Simple way
#ifdef RTCONFIG_USB_MULTIMODEM
	if(wan_type == WANS_DUALWAN_IF_USB2)
		return MODEM_UNIT_SECOND;
	else
#endif
	if(wan_type == WANS_DUALWAN_IF_USB)
		return MODEM_UNIT_FIRST;
	else
		return MODEM_UNIT_NONE;
}

int get_wantype_by_modemunit(int modem_unit){
	// Simple way
#ifdef RTCONFIG_USB_MULTIMODEM
	if(modem_unit == MODEM_UNIT_SECOND)
		return WANS_DUALWAN_IF_USB2;
	else
#endif
	if(modem_unit == MODEM_UNIT_FIRST)
		return WANS_DUALWAN_IF_USB;
	else
		return WANS_DUALWAN_IF_NONE;
}

char ssid[64] = { 0 };
char ssid2[64] = { 0 };

/**
 * Get default ssid.
 * @unit:	wireless unit
 * @subunit:	wireless subunit
 * @return:	pointer to a char array which contains result, default ssid.
 */
char *get_default_ssid(int unit, int subunit)
{
	int rev3 = 0;
	const int band_num __attribute__((unused)) = num_of_wl_if();
	char ssidbase[16], *macp = NULL;
	unsigned char mac_binary[6];
	const char *post_5g __attribute__((unused)) = "-1", *post_5g2 __attribute__((unused))= "-2", *post_guest = "_Guest";	/* postfix for RTCONFIG_NEWSSID_REV2 case */
#if defined(RTCONFIG_WIFI6E) || defined(RTCONFIG_HAS_6G)
	const char *post_6g __attribute__((unused)) = "6G";
#endif
#if defined(RTCONFIG_NEWSSID_REV2) || defined(RTCONFIG_NEWSSID_REV4) || defined(RTCONFIG_NEWSSID_REV5)
	rev3 = 1;
#endif
	if (unit < 0 || unit >= WL_NR_BANDS || subunit < 0) {
		dbg("%s: invalid parameter. (unit %d, subunit %d)\n",
			__func__, unit, subunit);
	}

	/* Adjust postfix for different conditions. */
#if defined(GTAC5300) || defined(GTAX11000)
	post_5g = "";
	post_5g2 = "_Gaming";
#elif defined(RTCONFIG_NEWSSID_REV4)
	post_5g = "";
	post_5g2 = "";
#if defined(RTCONFIG_WIFI6E) || defined(RTCONFIG_HAS_6G)
	post_6g = "";
#endif
#elif !defined(RTCONFIG_NEWSSID_REV2) && !defined(RTCONFIG_NEWSSID_REV4) && !defined(RTCONFIG_SINGLE_SSID)
	post_5g = "";
#endif

#if defined(RTCONFIG_NEWSSID_REV5)
#if defined(RTAX56_XD4) || defined(XD4PRO) || defined(XC5)
	if (nvram_match("SSIDRULE", "RT-V5")){
		post_5g = "";
	}
#elif defined(ET12) || defined(XT12)
	post_5g = "";
	post_5g2 = "";
#endif
#endif
#if defined(RTCONFIG_SINGLE_SSID) && defined(RTCONFIG_SSID_AMAPS)
	post_guest = "_AMAPS_Guest";
#endif

	memset(ssid, 0x0, sizeof(ssid));

#ifdef RTCONFIG_SINGLE_SSID
	macp = get_2g_hwaddr();
	ether_atoe(macp, mac_binary);
#if defined(RTCONFIG_SSID_AMAPS)
	snprintf((char *)ssidbase, sizeof(ssidbase), "%s_%02X_AMAPS", SSID_PREFIX, mac_binary[5]);
#elif defined(VZWAC1300)
	if (nvram_match("odmpid", "ASUSMESH-AC1300"))
		snprintf((char *)ssidbase, sizeof(ssidbase), "ASUS_%02X_MESH", mac_binary[5]);
	else
		snprintf((char *)ssidbase, sizeof(ssidbase), "%s_%02X", SSID_PREFIX, mac_binary[5]);
#else
	snprintf((char *)ssidbase, sizeof(ssidbase), "%s_%02X", SSID_PREFIX, mac_binary[5]);
#endif /* RTCONFIG_SSID_AMAPS */
#else
	if (rev3
#ifdef RTAC68U
		&& is_ssid_rev3_series()
#endif
	) {
		macp = get_2g_hwaddr();
		ether_atoe(macp, mac_binary);
#if defined(RTAC58U) || defined(RTAC59U)
		if (!strncmp(nvram_safe_get("territory_code"), "SP", 2))
			snprintf((char *)ssidbase, sizeof(ssidbase), "Spirit_%02X", mac_binary[5]);
		else if (!strncmp(nvram_safe_get("territory_code"), "CX/01", 5)
		      || !strncmp(nvram_safe_get("territory_code"), "CX/05", 5))
			snprintf((char *)ssidbase, sizeof(ssidbase), "Stuff-Fibre_%02X", mac_binary[5]);
		else
#endif
#ifdef RTAC68U
		if (is_dpsta_repeater())
			snprintf((char *)ssidbase, sizeof(ssidbase), "%s_RP_%02X", SSID_PREFIX, mac_binary[5]);
		else
#endif
#if defined(DSL_AX82U) && !defined(RTCONFIG_BCM_MFG)
		if (is_ax5400_i1())
			snprintf((char *)ssidbase, sizeof(ssidbase), "OPTUSGR%02X%02X%02X", mac_binary[3], mac_binary[4], mac_binary[5]);
		else
#endif
			snprintf((char *)ssidbase, sizeof(ssidbase), "%s_%02X", SSID_PREFIX, mac_binary[5]);
	} else {
		macp = get_lan_hwaddr();
		ether_atoe(macp, mac_binary);
		snprintf((char *)ssidbase, sizeof(ssidbase), "%s_%02X", get_productid(), mac_binary[5]);
	}
#endif

	strlcpy(ssid, ssidbase, sizeof(ssid));

	/* main ssid */
#ifdef RTCONFIG_NEWSSID_REV4
	if ((!subunit)) {
#if defined(RTAC59U)
		if (strncmp(nvram_safe_get("territory_code"), "CX/01", 5) && strncmp(nvram_safe_get("territory_code"), "CX/05", 5))
#elif defined(RTAX58U) || defined(RTAX56U)
		if (strncmp(nvram_safe_get("territory_code"), "CX", 2))
#elif defined(RTAC59_CD6R) || defined(RTAC59_CD6N)
		strlcat(ssid, "_CD6", sizeof(ssid));
#elif defined(PLAX56_XP4)
		strlcat(ssid, "_XP4", sizeof(ssid));
#elif defined(RTAX82_XD6) || defined(XD6_V2)
		if (strstr(nvram_safe_get("odmpid"), "XD6E"))
			strlcat(ssid, "_XD6E", sizeof(ssid));
		else
			strlcat(ssid, "_XD6", sizeof(ssid));
#elif defined(RTAX82_XD6S)
		if (strstr(nvram_safe_get("odmpid"), "XD6E"))
			strlcat(ssid, "_XD6E", sizeof(ssid));
		else
	                strlcat(ssid, "_XD6S", sizeof(ssid));
#elif defined(DSL_AX82U) && !defined(RTCONFIG_BCM_MFG)
		if (is_ax5400_i1() && unit == WL_5G_BAND)
			strlcat(ssid, "_5G", sizeof(ssid));
#elif defined(RTAX53U)
		if (!strncmp(nvram_safe_get("territory_code"), "JP", 2))
		{
			if(unit == WL_2G_BAND)
				strlcat(ssid, "_2G", sizeof(ssid));
			else
				strlcat(ssid, "_5G", sizeof(ssid));
		}
#endif
#if defined(RTCONFIG_NEWSSID_REV5)
#if defined(RTAX56_XD4)
		if (nvram_match("SSIDRULE", "RT-V5")){
			strlcat(ssid, "_XD4", sizeof(ssid));
		}
#elif defined(XD4PRO)
		if (nvram_match("SSIDRULE", "RT-V5")){
			if(nvram_match("odmpid","ZenWiFi_XD4_Pro")){
				strlcat(ssid, "_XD4_Pro", sizeof(ssid));
			}else{ /* odmpid","ZenWiFi_XD5" */
				strlcat(ssid, "_XD5", sizeof(ssid));
			}
		}
#elif defined(XC5)
		if (nvram_match("SSIDRULE", "RT-V5")){
			strlcat(ssid, "_XC5", sizeof(ssid));
		}
#elif defined(ET12) || defined(XT12)
		strlcat(ssid, "_", sizeof(ssid));
		strlcat(ssid, nvram_safe_get("model"), sizeof(ssid));
#elif defined(XT8PRO)
		strlcat(ssid, "_XT9", sizeof(ssid));
#elif defined(BT12)
		strlcat(ssid, "_BT12", sizeof(ssid));
#elif defined(BT10)
		if(nvram_match("odmpid", "ZenWiFi_BT10")){
			strlcat(ssid, "_BT10", sizeof(ssid));
		}else if(nvram_match("odmpid", "RT-BE18000")){
			strlcat(ssid, "_RT-BE18000", sizeof(ssid));
		}else{
			strlcat(ssid, "_BT10", sizeof(ssid));
		}
#elif defined(BQ16)
		strlcat(ssid, "_BQ16", sizeof(ssid));
#elif defined(BQ16_PRO)
		if(nvram_match("odmpid", "ZenWiFi_BQ16_Pro")){
			strlcat(ssid, "_BQ16_Pro", sizeof(ssid));
		}else{
			strlcat(ssid, "_BE30000", sizeof(ssid));
		}
#elif defined(BM68)
		strlcat(ssid, "_EBM68", sizeof(ssid));
#elif defined(XT8_V2)
		strlcat(ssid, "_XT8", sizeof(ssid));
#elif defined(XD4S)
		if(nvram_match("odmpid","ZenWiFi_XD4_Plus"))
			strlcat(ssid, "_XD4_Plus", sizeof(ssid));
		else
			strlcat(ssid, "_XD4S", sizeof(ssid));
#else
		strlcat(ssid, "_", sizeof(ssid));
		char *pid = get_productid();
		if (strstr(pid, "ExpertWiFi_"))
			pid += strlen("ExpertWiFi_");
		else if (strstr(pid, "ZenWiFi_"))
			pid += strlen("ZenWiFi_");
		strlcat(ssid, pid, sizeof(ssid));
#endif

#endif
		return ssid;
	}
#endif

#if !defined(RTCONFIG_SINGLE_SSID)	/* including RTCONFIG_NEWSSID_REV2 */
	switch (unit) {
	case WL_2G_BAND:
#if defined(RTCONFIG_NEWSSID_REV2) || defined(RTCONFIG_NEWSSID_REV4)
		if ((band_num > 1)
#ifdef RTAC68U
			&& !is_dpsta_repeater()
#endif
		)
#endif
#if !defined(RTCONFIG_NEWSSID_REV4)
			strlcat(ssid, "_2G", sizeof(ssid));
#endif
		break;
	case WL_5G_BAND:
#if !defined(RTCONFIG_NEWSSID_REV4)
		strlcat(ssid, "_5G", sizeof(ssid));
#endif
#ifdef RTCONFIG_HAS_5G_2
		if (band_num > 2 &&
		    nvram_get(wl_nvname("nband", WL_5G_2_BAND, 0)) != NULL)
		{
			strlcat(ssid, post_5g, sizeof(ssid));
		}
#endif
		break;
#ifdef RTCONFIG_HAS_5G_2
	case WL_5G_2_BAND:
#if !defined(RTCONFIG_NEWSSID_REV4)
		strlcat(ssid, "_5G", sizeof(ssid));
#endif
		strlcat(ssid, post_5g2, sizeof(ssid));
		break;
#endif
#if defined(RTCONFIG_WIFI6E) || defined(RTCONFIG_HAS_6G)
	case WL_6G_BAND:
		strlcat(ssid, post_6g, sizeof(ssid));
		break;
#endif
	case WL_60G_BAND:
		strlcat(ssid, "_60G", sizeof(ssid));
		break;
	default:
		dbg("%s: Unknown wl_unit (%d)\n", __func__, unit);
		strlcat(ssid, "_UNKNOWN", sizeof(ssid));
	}
#endif

	/* Handle guest network SSID. */
	if (subunit
#ifdef RTAC68U
		&& !is_dpsta_repeater()
#endif
	) {
#if defined(RTCONFIG_SSID_AMAPS)
		/* RTCONFIG_SSID_AMAPS use the same guest network SSID rule as SINGLE_SSID */
		snprintf(ssid, sizeof(ssid), "%s_AMAPS_Guest", SSID_PREFIX);
#elif defined(VZWAC1300)
	if (nvram_match("odmpid", "ASUSMESH-AC1300"))
		snprintf(ssid, sizeof(ssid), "ASUS_MESH_Guest");
	else
		strlcat(ssid, post_guest, sizeof(ssid));
#else
#if defined(RTCONFIG_NEWSSID_REV4)
	switch (unit) {
		case WL_2G_BAND:
			strlcat(ssid, "_2G", sizeof(ssid));
			break;
		case WL_5G_BAND:
#if defined(RTCONFIG_QUADBAND) || defined(RTCONFIG_HAS_5G_2) && ((!defined(RTCONFIG_WIFI6E) && !defined(RTCONFIG_WIFI7)) || (defined(RTCONFIG_WIFI7) && !defined(RTCONFIG_HAS_6G)))
			strlcat(ssid, "_5G-1", sizeof(ssid));
#else
			strlcat(ssid, "_5G", sizeof(ssid));
#endif
			break;
#if defined(RTCONFIG_HAS_5G_2)
		case WL_5G_2_BAND:
#if defined(RTCONFIG_QUADBAND) || (!defined(RTCONFIG_WIFI6E) && !defined(RTCONFIG_WIFI7)) || (defined(RTCONFIG_WIFI7) && !defined(RTCONFIG_HAS_6G))
			strlcat(ssid, "_5G-2", sizeof(ssid));
#else
#if defined(RTCONFIG_WIFI6E) || defined(RTCONFIG_WIFI7)
			strlcat(ssid, "_6G", sizeof(ssid));
#else
			strlcat(ssid, "_5G", sizeof(ssid));
#endif
#endif
			break;
#endif
#if defined(RTCONFIG_WIFI6E) || defined(RTCONFIG_HAS_6G)
		case WL_6G_BAND:
#if defined(RTCONFIG_HAS_6G_2)
			strlcat(ssid, "_6G-1", sizeof(ssid));
#else
			strlcat(ssid, "_6G", sizeof(ssid));
#endif
			break;
#endif
#if defined(RTCONFIG_QUADBAND) && defined(RTCONFIG_HAS_6G_2) && (defined(RTCONFIG_WIFI6E) || defined(RTCONFIG_WIFI7))
		case WL_6G_2_BAND:
			strlcat(ssid, "_6G-2", sizeof(ssid));
			break;
#endif

	}
#endif
		strlcat(ssid, post_guest, sizeof(ssid));
#endif
		if (subunit > 1) {
			snprintf(ssid2, sizeof(ssid2), "%s%d", ssid, subunit);
			strlcpy(ssid, ssid2, sizeof(ssid));
		}
	}

	return ssid;
}

#if defined(EBG15) || defined(EBG19)
char defpsk[32] = { 0 };
char *get_default_psk(int unit, int subunit)
{
	char pskbase[32], *macp = NULL;
	unsigned char mac_binary[6];

	if (unit < 0 || unit >= WL_NR_BANDS || subunit < 0) {
		dbg("%s: invalid parameter. (unit %d, subunit %d)\n",
			__func__, unit, subunit);
	}

	memset(defpsk, 0x0, sizeof(defpsk));

	macp = get_2g_hwaddr();
	ether_atoe(macp, mac_binary);

	snprintf((char *)pskbase, sizeof(pskbase), "%s_%02X%02X", SSID_PREFIX, mac_binary[4], mac_binary[5]);
	strlcpy(defpsk, pskbase, sizeof(defpsk));

	return defpsk;
}
#endif

/**
 * Get Static IPv4 DNS list separated by spaces.
 * @prefix:	WAN/LAN prefix of dns1_x & dns2_x values
 * @buf:	char buffer for storing the return value
 * @buflen:	char buffer size
 * @return:	pointer to a char array with DNS lisr, space separated
 */
char *get_userdns_r(const char *prefix, char *buf, size_t buflen)
{
	char tmp[32], *value;
	int i;

	if (buf == NULL || buflen <= 0)
		return NULL;

	strlcpy(buf, "", buflen);
	for (i = 1; i <= 2; i++) {
		snprintf(tmp, sizeof(tmp), "%sdns%d_x", prefix, i);
		value = nvram_safe_get_r(tmp, tmp, sizeof(tmp));
		if (*value && inet_addr_(value) != INADDR_ANY) {
			if (*buf)
				strlcat(buf, " ", buflen);
			strlcat(buf, value, buflen);
		}
	}
	return buf;
}

/* brif can be NULL for any bridge */
int is_bridged(const char *brif, const char *ifname)
{
	char path[PATH_MAX];

	if (!ifname)
		return 0;

	if (brif)
		snprintf(path, sizeof(path), "/sys/class/net/%s/brif/%s", brif, ifname);
	else
		snprintf(path, sizeof(path), "/sys/class/net/%s/brport/bridge", ifname);

	return l_exists(path) || d_exists(path) || f_exists(path);
}

int del_from_bridge(const char* ifname)
{
	char path[128] = {0};
	char br_path[PATH_MAX] = {0};
	char *br_ifname;

	snprintf(path, sizeof(path), "/sys/class/net/%s/brport/bridge", ifname);
	if (readlink(path, br_path, sizeof(br_path)-1) <= 0)
		return -1;
	br_ifname = strrchr(br_path, '/');
	br_ifname = (br_ifname) ? br_ifname+1 : br_path;
	return eval("brctl", "delif", br_ifname, (char*)ifname);
}

#ifdef RTCONFIG_BROOP

//int broop_state = 0;

#define NETLINK_BROOP	26
#define MAX_PAYLOAD	16

typedef struct broop_state {
	char ctrl;
	int val;
} broop_st;

enum {
        GET,
        SET
};

int netlink_broop(char ctrl, int val)
{
	struct sockaddr_nl src_addr, dest_addr;
	struct nlmsghdr *nlh = NULL;
	struct iovec iov;
	int sock_fd;
	struct msghdr msg;
	broop_st bst;
	int brs;
	
	sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_BROOP);
	if(sock_fd < 0) {
		printf("invalid sockfd\n");
		return 1;
	}
	memset(&src_addr, 0, sizeof(src_addr));
	src_addr.nl_family = AF_NETLINK;
	src_addr.nl_pid = getpid(); 
	src_addr.nl_groups = 0;  /* not in mcast groups */
	bind(sock_fd, (struct sockaddr*)&src_addr,sizeof(src_addr));

	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.nl_family = AF_NETLINK;
	dest_addr.nl_pid = 0;   /* For Linux Kernel */
	dest_addr.nl_groups = 0; /* unicast */

	nlh=(struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
	nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
	nlh->nlmsg_pid = getpid(); 
	nlh->nlmsg_flags = 0;

	bst.ctrl = ctrl;
	bst.val = val;
	memcpy(NLMSG_DATA(nlh), &bst, sizeof(broop_st));

	iov.iov_base = (void *)nlh;
	iov.iov_len = nlh->nlmsg_len;
	memset(&msg, 0, sizeof(msg));
	msg.msg_name = (void *)&dest_addr;
	msg.msg_namelen = sizeof(dest_addr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	if(sendmsg(sock_fd, &msg, 0) < 0)
		perror("sendmsg error");
	else
		printf("sendmsg ok\n");
	
 	if(recvmsg(sock_fd, &msg, 0) < 0) {
		perror("recvmsg fail:");
		goto end;
	}
 	
	memcpy(&brs, NLMSG_DATA(msg.msg_iov->iov_base), 4);
end:
	close(sock_fd);
	return brs;
}

int detect_broop()
{
	int fd, ret = 0;
/*
	switch (broop_state) {
	case BROOP_IDLE:
		netlink_broop(SET, 0);
		broop_state = BROOP_DETECT;
		
		break;
	case BROOP_DETECT:
		ret = netlink_broop(GET, 0);
		if (ret == 1)
			broop_state = BROOP_IDLE;		

		break;
	default:
		_dprintf("Illegal broop state.\n");
		break;
	}
*/
	ret = netlink_broop(GET, 0);

	return ret;
}


#endif

#ifdef RTCONFIG_MULTI_PPP
int is_mtppp_base_unit(int wan_unit)
{
	if (wan_unit < WAN_UNIT_MAX)
		return 1;
	else
		return 0;
}

int is_mtppp_unit(int wan_unit)
{
	if (wan_unit > WAN_UNIT_FIRST_MULTIPPP_BASE && wan_unit < WAN_UNIT_MULTIPPP_MAX)
		return 1;
	else
		return 0;
}

int get_mtppp_base_unit(int wan_unit)
{
	if (wan_unit >= WAN_UNIT_FIRST_MULTIPPP_START && wan_unit <= WAN_UNIT_FIRST_MULTIPPP_END)
		return WAN_UNIT_FIRST;
#ifdef RTCONFIG_DUALWAN
	else if (wan_unit >= WAN_UNIT_SECOND_MULTIPPP_START && wan_unit <= WAN_UNIT_SECOND_MULTIPPP_END)
		return WAN_UNIT_SECOND;
#endif
	else
		return wan_unit;
}

int get_mtppp_wan_unit(int base_wan_unit, int idx)
{
	if (!is_mtppp_base_unit(base_wan_unit))
		return WAN_UNIT_NONE;

	if (idx) {
		if (base_wan_unit == WAN_UNIT_FIRST)
			return WAN_UNIT_FIRST_MULTIPPP_BASE + idx;
#ifdef RTCONFIG_DUALWAN
		else if (base_wan_unit == WAN_UNIT_SECOND)
			return WAN_UNIT_SECOND_MULTIPPP_BASE + idx;
#endif
		else
			return WAN_UNIT_NONE;
	}
	else
		return base_wan_unit;
}

int get_mtppp_idx_by_wan_unit(int wan_unit)
{
	if (is_mtppp_base_unit(wan_unit))
		return 0;
	else if (wan_unit >= WAN_UNIT_FIRST_MULTIPPP_START && wan_unit <= WAN_UNIT_FIRST_MULTIPPP_END)
		return (wan_unit - WAN_UNIT_FIRST_MULTIPPP_BASE);
#ifdef RTCONFIG_DUALWAN
	else if (wan_unit >= WAN_UNIT_SECOND_MULTIPPP_START && wan_unit <= WAN_UNIT_SECOND_MULTIPPP_END)
		return (wan_unit - WAN_UNIT_SECOND_MULTIPPP_BASE);
#endif
	else
		return -1;
}
#endif // RTCONFIG_MULTI_PPP

#ifdef RTCONFIG_MULTISERVICE_WAN
int is_ms_base_unit(int wan_unit)
{
	if (wan_unit < WAN_UNIT_MAX)
		return 1;
#ifdef RTCONFIG_MULTIWAN_IF
	else if (wan_unit >= MULTI_WAN_START_IDX && wan_unit < MULTI_WAN_START_IDX + MAX_MULTI_WAN_NUM)
		return 1;
#endif
	else
		return 0;
}

int is_ms_wan_unit(int wan_unit)
{
#ifdef RTCONFIG_MULTIWAN_IF
	if (wan_unit > WAN_UNIT_MTWAN0_MS_BASE && wan_unit < WAN_UNIT_MTWAN_MS_MAX)
		return 1;
#endif
	if (wan_unit > WAN_UNIT_FIRST_MULTISRV_BASE && wan_unit < WAN_UNIT_MULTISRV_MAX)
		return 1;
	return 0;
}

int get_ms_base_unit(int wan_unit)
{
#ifdef RTCONFIG_MULTIWAN_IF
	if (wan_unit > WAN_UNIT_MTWAN0_MS_BASE)
		return ((wan_unit - WAN_UNIT_MTWAN_MS_BASE) / 10);
	else
#endif
	return (wan_unit > WAN_UNIT_MULTISRV_BASE) ? ((wan_unit - WAN_UNIT_MULTISRV_BASE) / 10) : wan_unit;
}

/*
 * base_wan_unit: WAN_UNIT_FIRST, WAN_UNIT_SECOND
 * idx: 0~9
 */
int get_ms_wan_unit(int base_wan_unit, int idx)
{
	if (!is_ms_base_unit(base_wan_unit) || idx >= WAN_MULTISRV_MAX)
		return WAN_UNIT_NONE;

	if (idx)
	{
		if (base_wan_unit == WAN_UNIT_FIRST)
			return WAN_UNIT_FIRST_MULTISRV_BASE + idx;
#if defined(RTCONFIG_DUALWAN) || defined(RTCONFIG_USB_MODEM)
		else if (base_wan_unit == WAN_UNIT_SECOND)
			return WAN_UNIT_SECOND_MULTISRV_BASE + idx;
#endif
#ifdef RTCONFIG_MULTIWAN_IF
		else if (base_wan_unit >= MULTI_WAN_START_IDX && base_wan_unit < MULTI_WAN_START_IDX + MAX_MULTI_WAN_NUM)
			return (WAN_UNIT_MTWAN_MS_BASE + base_wan_unit * 10 + idx);
#endif
		else
			return WAN_UNIT_NONE;
	}
	else
		return base_wan_unit;
}

int get_ms_idx_by_wan_unit(int wan_unit)
{
	if (is_ms_base_unit(wan_unit))
		return 0;
	else if(wan_unit > WAN_UNIT_MULTISRV_BASE)
		return wan_unit % 10;
	else
		return -1;
}
#endif //RTCONFIG_MULTISERVICE_WAN
