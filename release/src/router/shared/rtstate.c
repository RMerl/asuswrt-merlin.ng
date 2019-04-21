#define _GNU_SOURCE

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <bcmnvram.h>
#include <bcmdevs.h>
#include <shutils.h>
#include <shared.h>


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

	return nvram_get_int(strcat_r(prefix, "state_t", tmp));
}

int get_wan_sbstate(int unit){
	char tmp[100], prefix[16];

	snprintf(prefix, 16, "wan%d_", unit);

	return nvram_get_int(strcat_r(prefix, "sbstate_t", tmp));
}

int get_wan_auxstate(int unit){
	char tmp[100], prefix[16];

	snprintf(prefix, 16, "wan%d_", unit);

	return nvram_get_int(strcat_r(prefix, "auxstate_t", tmp));
}

char *link_wan_nvname(int unit, char *buf, int size){
	if(buf == NULL)
		return NULL;

	if(unit == WAN_UNIT_FIRST)
		snprintf(buf, size, "link_wan");
	else
		snprintf(buf, size, "link_wan%d", unit);

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
			return 0;
	}
	else
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
			if(strcmp(nvram_safe_get(strcat_r(prefix, "pppoe_ifname", tmp)), ifname)==0) {
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
	char tmp[100], prefix[32]="wanXXXXXX_";
	int unit = 0;
	int model = get_model();

	if(ifname == NULL)
		return -1;

	for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit){
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);

		if(!strncmp(ifname, "ppp", 3) ){

			if(nvram_match(strcat_r(prefix, "pppoe_ifname", tmp), ifname)) {
				if (model ==  MODEL_RTN65U) {
					if(!nvram_match(strcat_r(prefix, "proto", tmp), "pppoe") || nvram_match(strcat_r(prefix, "is_usb_modem_ready", tmp), "1"))						
						return unit;
				}	
				else if (nvram_match(strcat_r(prefix, "state_t", tmp), "2") && nvram_match(strcat_r(prefix, "auxstate_t", tmp), "0") && nvram_match(strcat_r(prefix, "gw_ifname", tmp), ifname)) 
					return unit;				
			}

				
		}
		else if(nvram_match(strcat_r(prefix, "ifname", tmp), ifname)) {

			if (model == MODEL_RTN65U && !nvram_match(strcat_r(prefix, "proto", tmp), "l2tp") && !nvram_match(strcat_r(prefix, "proto", tmp), "pptp"))
					return unit;
			
			if (!nvram_match(strcat_r(prefix, "proto", tmp), "pppoe") && !nvram_match(strcat_r(prefix, "proto", tmp), "l2tp") && !nvram_match(strcat_r(prefix, "proto", tmp), "pptp") && nvram_match(strcat_r(prefix, "gw_ifname", tmp), ifname))
					return unit;						
		}
		else if (model == MODEL_BLUECAVE){
			if (nvram_get_int("switch_stb_x") > 0 && !strcmp(ifname, "eth1"))
				return unit;
		}
	}

	return -1;
}
#endif

// Get physical wan ifname of working connection
char *get_wanx_ifname(int unit)
{
	char *wan_ifname;
	char tmp[100], prefix[sizeof("wanXXXXXXXXXX_")];
	
	snprintf(prefix, sizeof(prefix), "wan%d_", unit);
	wan_ifname = nvram_safe_get(strcat_r(prefix, "ifname", tmp));

	return wan_ifname;
}

// Get wan ifname of working connection
char *get_wan_ifname(int unit)
{
	char *wan_proto, *wan_ifname;
	char tmp[100], prefix[sizeof("wanXXXXXXXXXX_")];

	snprintf(prefix, sizeof(prefix), "wan%d_", unit);
	wan_proto = nvram_safe_get(strcat_r(prefix, "proto", tmp));

#ifdef RTCONFIG_USB_MODEM
	if (dualwan_unit__usbif(unit)) {
		if (strcmp(wan_proto, "dhcp") == 0)
			wan_ifname = nvram_safe_get(strcat_r(prefix, "ifname", tmp));
		else
			wan_ifname = nvram_safe_get(strcat_r(prefix, "pppoe_ifname", tmp));
	} else
#endif
	if (strcmp(wan_proto, "pppoe") == 0 ||
	    strcmp(wan_proto, "pptp") == 0 ||
	    strcmp(wan_proto, "l2tp") == 0) {
		wan_ifname = nvram_safe_get(strcat_r(prefix, "pppoe_ifname", tmp));
	} else
		wan_ifname = nvram_safe_get(strcat_r(prefix, "ifname", tmp));

	return wan_ifname;
}

// Get wan ipv6 ifname of working connection
#ifdef RTCONFIG_IPV6
char *get_wan6_ifname(int unit)
{
	char *wan_proto, *wan_ifname;
	char tmp[100], prefix[sizeof("wanXXXXXXXXXX_")];

	switch (get_ipv6_service_by_unit(unit)) {
	case IPV6_NATIVE_DHCP:
	case IPV6_MANUAL:
#ifdef RTCONFIG_6RELAYD
	case IPV6_PASSTHROUGH:
#endif
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);
		wan_proto = nvram_safe_get(strcat_r(prefix, "proto", tmp));

#ifdef RTCONFIG_USB_MODEM
		if (dualwan_unit__usbif(unit)) {
			if (strcmp(wan_proto, "dhcp") == 0)
				wan_ifname = nvram_safe_get(strcat_r(prefix, "ifname", tmp));
			else
				wan_ifname = nvram_safe_get(strcat_r(prefix, "pppoe_ifname", tmp));
		} else
#endif
		if (strcmp(wan_proto, "dhcp") != 0 && strcmp(wan_proto, "static") != 0 &&
		    nvram_match(ipv6_nvname_by_unit("ipv6_ifdev", unit), "ppp")) {
			wan_ifname = nvram_safe_get(strcat_r(prefix, "pppoe_ifname", tmp));
		} else
			wan_ifname = nvram_safe_get(strcat_r(prefix, "ifname", tmp));
		break;
	case IPV6_6TO4:
	case IPV6_6IN4:
	case IPV6_6RD:
		/* no ipv6 multiwan tunnel support so far */
		wan_ifname = "v6tun0";
		break;
	default:
		return "";
	}

	return wan_ifname;
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
	if(get_dualwan_by_unit(wan_unit) == WANS_DUALWAN_IF_LAN)
	{
	#ifdef RTCONFIG_RALINK
		return rtkswitch_wanPort_phyStatus(wan_unit); //Paul modify 2012/12/4
	#else
		return wanport_status(wan_unit);
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
		nvram_set_int(strcat_r(prefix, "primary", tmp), (i == unit) ? 1 : 0);
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
		if (nvram_match(strcat_r(prefix, "primary", tmp), "1"))
			return unit;
	}

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
	    && !strcmp(nvram_safe_get("wans_mode"), "lb")
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

	strcpy(xhci_string, "xxxxxxxx");

	foreach(word, nvram_safe_get("xhci_ports"), next){
		if(i == port){
			strcpy(xhci_string, word);
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

	strcpy(ehci_string, "xxxxxxxx");

	foreach(word, nvram_safe_get("ehci_ports"), next) {
		if(i==port) {
			strcpy(ehci_string, word);
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

	strcpy(ohci_string, "xxxxxxxx");

	foreach(word, nvram_safe_get("ohci_ports"), next) {
		if(i==port) {
			strcpy(ohci_string, word);
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

#if defined(RTCONFIG_DUALWAN)
void set_wanscap_support(char *feature)
{
	nvram_set("wans_cap", feature);
}

void add_wanscap_support(char *feature)
{
	char features[128];

	strcpy(features, nvram_safe_get("wans_cap"));

	if(strlen(features)==0) nvram_set("wans_cap", feature);
	else {
		sprintf(features, "%s %s", features, feature);
		nvram_set("wans_cap", features);
	}
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
	}

	return caps;
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

	i = 0;
	foreach(word, wans_dualwan, next) {
		if(i==unit) {
			if (!strcmp(word,"lan")) return WANS_DUALWAN_IF_LAN;
			if (!strcmp(word,"2g")) return WANS_DUALWAN_IF_2G;
			if (!strcmp(word,"5g")) return WANS_DUALWAN_IF_5G;
			if (!strcmp(word,"usb")) return WANS_DUALWAN_IF_USB;
			if (!strcmp(word,"dsl")) return WANS_DUALWAN_IF_DSL;
			if (!strcmp(word,"wan")) return WANS_DUALWAN_IF_WAN;
			if (!strcmp(word,"wan2")) return WANS_DUALWAN_IF_WAN2;
#ifdef RTCONFIG_USB_MULTIMODEM
			if (!strcmp(word,"usb2")) return WANS_DUALWAN_IF_USB2;
#endif
			return WANS_DUALWAN_IF_NONE;
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

	return WAN_UNIT_NONE;
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

	strcpy(types, nvram_safe_get("wans_dualwan"));

	if(strlen(types)==0) nvram_set("wans_dualwan", type);
	else {
		sprintf(types, "%s %s", types, type);
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
	char phys[128], *ifnames;

	if (phy == NULL || *phy == '\0')
		return;

	ifnames = nvram_safe_get("lan_ifnames");
	if(find_word(ifnames, phy) != NULL)
		return;	/* exist */

	snprintf(phys, sizeof(phys), "%s%s%s", ifnames,
		(*ifnames && *phy) ? " " : "", phy);
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

		if(!strcmp(dev, nvram_safe_get(strcat_r(prefix, "act_dev", tmp))))
			return modem_unit;
	}

	return MODEM_UNIT_NONE;
}

int get_modemunit_by_node(const char *usb_node){
	int modem_unit;
	char tmp[100], prefix[32];

	for(modem_unit = MODEM_UNIT_FIRST; modem_unit < MODEM_UNIT_MAX; ++modem_unit){
		usb_modem_prefix(modem_unit, prefix, sizeof(prefix));

		if(!strcmp(usb_node, nvram_safe_get(strcat_r(prefix, "act_path", tmp))))
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
	const int band_num = num_of_wl_if();
	char ssidbase[16], *macp = NULL;
	unsigned char mac_binary[6];
	const char *post_5g = "-1", *post_5g2 = "-2", *post_guest = "_Guest";	/* postfix for RTCONFIG_NEWSSID_REV2 case */

#ifdef RTCONFIG_NEWSSID_REV2
	rev3 = 1;
#endif

	if (unit < 0 || unit >= WL_NR_BANDS || subunit < 0) {
		dbg("%s: invalid parameter. (unit %d, subunit %d)\n",
			__func__, unit, subunit);
	}

	/* Adjust postfix for different conditions. */
#ifdef GTAC5300
	post_5g = "";
	post_5g2 = "_Gaming";
#elif !defined(RTCONFIG_NEWSSID_REV2) && !defined(RTCONFIG_SINGLE_SSID)
	post_5g = "";
#endif

#if defined(RTCONFIG_SINGLE_SSID) && defined(RTCONFIG_SSID_AMAPS)
	post_guest = "_AMAPS_Guest";
#endif


	memset(ssid, 0x0, sizeof(ssid));

#ifdef RTCONFIG_SINGLE_SSID
	macp = get_2g_hwaddr();
	ether_atoe(macp, mac_binary);
#if defined(RTCONFIG_SSID_AMAPS)
	sprintf((char *)ssidbase, "%s_%02X_AMAPS", SSID_PREFIX, mac_binary[5]);
#elif defined(VZWAC1300)
	if (nvram_match("odmpid", "ASUSMESH-AC1300"))
		sprintf((char *)ssidbase, "ASUS_%02X_MESH", mac_binary[5]);
	else
		sprintf((char *)ssidbase, "%s_%02X", SSID_PREFIX, mac_binary[5]);
#else
	sprintf((char *)ssidbase, "%s_%02X", SSID_PREFIX, mac_binary[5]);
#endif /* RTCONFIG_SSID_AMAPS */
#else
	if (rev3
#ifdef RTAC68U
		&& is_ssid_rev3_series()
#endif
	) {
		macp = get_2g_hwaddr();
		ether_atoe(macp, mac_binary);
#if defined(RTAC58U)
		if (!strncmp(nvram_safe_get("territory_code"), "SP", 2))
			sprintf((char *)ssidbase, "Spirit_%02X", mac_binary[5]);
		else if (!strncmp(nvram_safe_get("territory_code"), "CX", 2))
			sprintf((char *)ssidbase, "Stuff-Fibre_%02X", mac_binary[5]);
		else
#endif
#ifdef RTAC68U
		if (is_dpsta_repeater())
			sprintf((char *)ssidbase, "%s_RP_%02X", SSID_PREFIX, mac_binary[5]);
		else
#endif
			sprintf((char *)ssidbase, "%s_%02X", SSID_PREFIX, mac_binary[5]);
	} else {
		macp = get_lan_hwaddr();
		ether_atoe(macp, mac_binary);
		sprintf((char *)ssidbase, "%s_%02X", get_productid(), mac_binary[5]);
	}
#endif

	strlcpy(ssid, ssidbase, sizeof(ssid));
#if !defined(RTCONFIG_SINGLE_SSID)	/* including RTCONFIG_NEWSSID_REV2 */
	switch (unit) {
	case WL_2G_BAND:
#if defined(RTCONFIG_NEWSSID_REV2)
		if ((band_num > 1)
#ifdef RTAC68U
			&& !is_dpsta_repeater()
#endif
		)
#endif
			strlcat(ssid, "_2G", sizeof(ssid));
		break;
	case WL_5G_BAND:
		strlcat(ssid, "_5G", sizeof(ssid));
		if (band_num > 2 &&
		    nvram_get(wl_nvname("nband", WL_5G_2_BAND, 0)) != NULL)
		{
			strlcat(ssid, post_5g, sizeof(ssid));
		}
		break;
	case WL_5G_2_BAND:
		strlcat(ssid, "_5G", sizeof(ssid));
		strlcat(ssid, post_5g2, sizeof(ssid));
		break;
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
		strlcat(ssid, post_guest, sizeof(ssid));
#endif
		if (subunit > 1) {
			sprintf(ssid2, "%s%d", ssid, subunit);
			strlcpy(ssid, ssid2, sizeof(ssid));
		}
	}
	return ssid;
}

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
