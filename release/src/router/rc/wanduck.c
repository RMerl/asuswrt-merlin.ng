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
 */

#define _GNU_SOURCE

#include <rc.h>
#include <wanduck.h>

#if defined(RTCONFIG_HIDDEN_BACKHAUL)
#include <qca.h>
#endif
#include <limits.h>		//PATH_MAX, LONG_MIN, LONG_MAX

//#define DETECT_INTERNET_MORE
#define NO_IOS_DETECT_INTERNET

static int wdbg = 0;

#define _wdbg(fmt, args...) do { if (wdbg) { dbg(fmt, ## args); }; } while (0)

#if defined(RTCONFIG_WANRED_LED)
#if defined(RTCONFIG_WANLEDX2)
static int update_wan_led_and_wanred_led(int wan_unit)
{
	/* e.g. BRT-AC828: WAN WHITE/RED LED x 2, RTCONFIG_DUALWAN must be enabled. */
	int mode = sw_mode, l = link_wan[wan_unit], state;
	int wan_led, wanred_led, other_wan_led, other_wanred_led;
	char s[] = "wanX_state_tXXX";

	if (wan_unit < 0 || wan_unit >= WAN_UNIT_MAX)
		return -1;

	if (mode < SW_MODE_ROUTER || mode > SW_MODE_HOTSPOT)
		mode = sw_mode();
	/* Turn on/off WAN WHITE/RED LED in accordance with wan status. */
	switch (mode) {
	case SW_MODE_ROUTER:
		switch (wan_unit) {
		case 0:
			wan_led = LED_WAN;
			wanred_led = LED_WAN_RED;
			other_wan_led = LED_WAN2;
			other_wanred_led = LED_WAN2_RED;
			break;
		case 1:
			wan_led = LED_WAN2;
			wanred_led = LED_WAN2_RED;
			other_wan_led = LED_WAN;
			other_wanred_led = LED_WAN_RED;
			break;
		default:
			return 0;
		}

		if (strcmp(dualwan_mode, "lb")) {
			if (wan_primary_ifunit() != wan_unit) {
				led_control(wanred_led, LED_OFF);
				led_control(wan_led, LED_OFF);
				return 0;
			} else {
				/* Turn off the other WAN LED. */
				led_control(other_wanred_led, LED_OFF);
				led_control(other_wan_led, LED_OFF);
			}
		}

		snprintf(s, sizeof(s), "wan%d_state_t", wan_unit);
		state = nvram_get_int(s);
		l = get_wanports_status(wan_unit);
		if (!inhibit_led_on()) {
			if (l == CONNED && state == WAN_STATE_CONNECTED) {
				led_control(wanred_led, LED_OFF);
				led_control(wan_led, LED_ON);
			} else {
				led_control(wanred_led, LED_ON);
				led_control(wan_led, LED_OFF);
			}
		} else {
			led_control(wanred_led, LED_OFF);
			led_control(wan_led, LED_OFF);
		}
		break;
	case SW_MODE_REPEATER:	/* fallthrough */
		wan_red_led_control(LED_OFF);
		wan2_red_led_control(LED_OFF);
		break;
	case SW_MODE_AP:
		/* Update WAN1 LED and WAN2 LED in AP mode. */
		for (wan_unit = WAN_UNIT_FIRST; wan_unit < WAN_UNIT_MAX; ++wan_unit) {
			switch (wan_unit) {
			case WAN_UNIT_FIRST:
				wan_led = LED_WAN;
				wan_red_led_control(LED_OFF);
				break;
			case WAN_UNIT_SECOND:
				wan_led = LED_WAN2;
				wan2_red_led_control(LED_OFF);
				break;
			default:
				continue;
			}
			l = wanport_status(wan_unit);
			led_control(wan_led, (!inhibit_led_on() && l == 1)? LED_ON : LED_OFF);
		}
		break;
	}

	return 0;
}
#else	/* !RTCONFIG_WANLEDX2 */
static int update_wan_led_and_wanred_led(int wan_unit)
{
	/* e.g. RT-AC55U: WAN BLUE/RED LED */
	int mode = sw_mode, l = link_wan[wan_unit], state;
	char s[] = "wanX_state_tXXX";

	if (wan_unit < 0 || wan_unit >= WAN_UNIT_MAX)
		return -1;

	if (mode < SW_MODE_ROUTER || mode > SW_MODE_HOTSPOT)
		mode = sw_mode();
	/* Turn on/off WAN BLUE/RED LED in accordance with wan status. */
	switch (mode) {
	case SW_MODE_ROUTER:
#if defined(RTCONFIG_DUALWAN)
		if (!strcmp(dualwan_mode, "lb")) {
			int u, onoff = 0;

			/* Turn on WAN BLUE LED if any WAN unit is connected in load-balanced mode. */
			for (u = WAN_UNIT_FIRST; !onoff && u < WAN_UNIT_MAX; ++u) {
				snprintf(s, sizeof(s), "wan%d_state_t", u);
				state = nvram_get_int(s);
				l = link_wan[u];
				if (dualwan_unit__nonusbif(u))
					l = get_wanports_status(u);

				if (l == CONNED && state == WAN_STATE_CONNECTED)
					onoff++;
			}

			if (!inhibit_led_on()) {
				if (onoff) {
					wan_red_led_control(LED_OFF);
					led_control(LED_WAN, LED_ON);
				} else {
					wan_red_led_control(LED_ON);
					led_control(LED_WAN, LED_OFF);
				}
			} else {
				wan_red_led_control(LED_OFF);
				led_control(LED_WAN, LED_OFF);
			}
		} else
#endif
		{
			if (wan_primary_ifunit() != wan_unit)
				return 0;

			snprintf(s, sizeof(s), "wan%d_state_t", wan_unit);
			state = nvram_get_int(s);
			if (dualwan_unit__nonusbif(wan_unit))
				l = get_wanports_status(wan_unit);

			if (!inhibit_led_on()) {
				if (l == CONNED && state == WAN_STATE_CONNECTED) {
					wan_red_led_control(LED_OFF);
					led_control(LED_WAN, LED_ON);
				} else {
					wan_red_led_control(LED_ON);
					led_control(LED_WAN, LED_OFF);
				}
			} else {
				wan_red_led_control(LED_OFF);
				led_control(LED_WAN, LED_OFF);
			}
		}
		break;
	case SW_MODE_REPEATER:	/* fallthrough */
	case SW_MODE_AP:
		wan_red_led_control(LED_OFF);
		break;
	}

	return 0;
}
#endif
#else	/* !RTCONFIG_WANRED_LED */
static inline int update_wan_led_and_wanred_led(int wan_unit) { return 0; }
#endif	/* RTCONFIG_WANRED_LED */

void set_link_internet(int wan_unit, int link_internet){
#if !defined(RTCONFIG_WANLEDX2)
	if(nvram_get_int("link_internet") == link_internet){
		//_dprintf("%s: skip the set link_internet.\n", __func__);
		return;
	}
#endif

	nvram_set_int("link_internet", link_internet);

#if defined(RTCONFIG_HND_ROUTER_AX) || defined(RTCONFIG_LANWAN_LED) || defined(RTCONFIG_WANRED_LED) || defined(RTCONFIG_FAILOVER_LED)
	update_wan_leds(wan_unit, link_wan[wan_unit]);
#endif
#ifdef DSL_AC68U
	if (nvram_match("AllLED", "1")) {
		led_control(LED_WAN, (link_internet == 2) ? LED_ON : LED_OFF);
	}
#endif
}

#ifndef CONFIG_BCMWL5
#if defined(RTCONFIG_LANWAN_LED) || defined(RTCONFIG_WANRED_LED)
#if defined(RTCONFIG_FAILOVER_LED)
int update_failover_led(void)
{
	enum led_fan_mode_id v = LED_OFF;

	/* If dual-wan is not enabled or is not fail-over mode,
	 * always turn off fail-over led.
	 */
	if (get_dualwan_by_unit(1) == WANS_DUALWAN_IF_NONE
#ifdef RTCONFIG_DUALWAN
			|| (strcmp(dualwan_mode, "fo") && strcmp(dualwan_mode, "fb"))
#endif
			){
		failover_led_control(LED_OFF);
		return 0;
	}

	if (wan_primary_ifunit() == 1 && !inhibit_led_on())
		v = LED_ON;

	failover_led_control(v);

	return 0;
}
#endif

int update_wan_leds(int wan_unit, int link_wan_unit)
{
#if defined(RTCONFIG_WANRED_LED)
	if (!inhibit_led_on())
		update_wan_led_and_wanred_led(wan_unit);

#else	/* !RTCONFIG_WANRED_LED */
	int link_internet = nvram_get_int("link_internet");

	/* Turn on/off WAN LED in accordance with link status of WAN port */
	if (link_wan_unit && !inhibit_led_on()) {
		led_control(LED_WAN, LED_ON);
	} else {
		if(link_internet != 2)
			led_control(LED_WAN, LED_OFF);
	}
#endif	/* RTCONFIG_WANRED_LED */

#if defined(RTCONFIG_FAILOVER_LED)
	update_failover_led();
#endif

	return 0;
}
#endif	/* RTCONFIG_LANWAN_LED */

#ifdef RTCONFIG_QCA
void sw_led_ctrl(void)
{
#if defined(GTAXY16000) || defined(RTAX89U)
	if (is_aqr_phy_exist())
		r10g_led_control((!inhibit_led_on() && ethtool_glink("eth5") > 0)? LED_ON : LED_OFF);
	sfpp_led_control((!inhibit_led_on() && ethtool_glink("eth4") > 0)? LED_ON : LED_OFF);
#endif
}
#endif
#endif // CONFIG_BCMWL5

/* 67u,87u,3200: have each led on every port.
 * 88u,3100,5300: have one led to hint wan port but this led is the union of all ports
 * force led_on on usb modem case */
void enable_wan_led()
{
	int usb_wan = get_dualwan_by_unit(wan_primary_ifunit()) == WANS_DUALWAN_IF_USB ? 1:0;

#ifdef RTCONFIG_HND_ROUTER_AX
	return;
#endif

	if(usb_wan) {
		switch (get_model()) {
#ifdef RTAC68U
			case MODEL_RTAC68U:
				if (!is_ac66u_v2_series() && !is_ac68u_v3_series())
					break;
#endif
			case MODEL_RTAC3200:
			case MODEL_RTAC87U:
				eval("et", "-i", "eth0", "robowr", "0", "0x18", "0x01ff");
				eval("et", "-i", "eth0", "robowr", "0", "0x1a", "0x01fe");
				break;

			case MODEL_RTAC5300:
			case MODEL_GTAC5300:
			case MODEL_RTAC88U:
			case MODEL_RTAC86U:
			case MODEL_RTAC3100:
#ifdef HND_ROUTER
#ifdef GTAC2900
				eval("sw", "0x800c00a0", "0");	// disable event on tx/rx activity
#else
				led_control(LED_WAN_NORMAL, LED_ON);
#endif
#else
				eval("et", "-i", "eth0", "robowr", "0", "0x18", "0x01ff");
				eval("et", "-i", "eth0", "robowr", "0", "0x1a", "0");
#endif
				break;
		}
	}

	led_control(LED_WAN, LED_OFF);
#ifdef HND_ROUTER
#ifdef GTAC2900
	eval("sw", "0x800c00a0", "0");				// disable event on tx/rx activity
#else
	led_control(LED_WAN_NORMAL, LED_ON);
#endif
#else
	eval("et", "-i", "eth0", "robowr", "0", "0x18", "0x01ff");
	eval("et", "-i", "eth0", "robowr", "0", "0x1a", "0x01ff");
#endif
}

void disable_wan_led()
{
#ifdef RTCONFIG_HND_ROUTER_AX
	return;
#elif defined(HND_ROUTER)
	led_control(LED_WAN_NORMAL, LED_OFF);
#else
	eval("et", "-i", "eth0", "robowr", "0", "0x18", "0x01fe");
	eval("et", "-i", "eth0", "robowr", "0", "0x1a", "0x01fe");
#endif
}

static void wan_led_control(int sig) {
#ifdef RTCONFIG_HND_ROUTER_AX
	int unit;

	for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit){
		if(get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_NONE)
			continue;

		if(unit != wan_primary_ifunit()
#ifdef RTCONFIG_DUALWAN
				&& strcmp(dualwan_mode, "lb")
#endif
				)
			continue;

		update_wan_leds(unit, !rule_setup);
	}
#elif defined(RTAC68U) ||  defined(RTAC87U) || defined(RTAC3200) || defined(RTCONFIG_BCM_7114) || defined(DSL_AC68U) || defined(HND_ROUTER)
	if(nvram_match("AllLED", "1")
#ifdef RTAC68U
		&& (is_ac66u_v2_series() || is_ac68u_v3_series())
#endif
	) {
#if defined(RTAC68U) ||  defined(RTAC87U) || defined(RTAC3200) || defined(RTCONFIG_BCM_7114) || defined(HND_ROUTER)
		if (rule_setup) {
			led_control(LED_WAN, LED_ON);
			disable_wan_led();
		} else
			enable_wan_led();
#elif defined(DSL_AC68U)
		if (rule_setup) {
			led_control(LED_WAN, LED_OFF);
		} else {
			led_control(LED_WAN, LED_ON);
		}
#endif
	}
#endif // RTCONFIG_HND_ROUTER_AX

#if defined(RTCONFIG_QCA) && (defined(RTCONFIG_LED_BTN) || defined(RTCONFIG_TURBO_BTN))
	if (!inhibit_led_on()) {
		int unit;

		for (unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
			if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_NONE)
				continue;

#if defined(RTCONFIG_LANWAN_LED)
			update_wan_leds(unit, link_wan[unit]);
#endif
		}
	}
#endif
}

static void safe_leave(int signo){
	int i, ret;
	char tmp[100] = "";

	_dprintf("\n## wanduck.safeexit ##\n");

	for(i = WAN_UNIT_FIRST; i < WAN_UNIT_MAX; ++i){
		link_wan_nvname(i, tmp, sizeof(tmp));
		nvram_unset(tmp);
	}
#if defined(HND_ROUTER) && defined(RTCONFIG_BONDING_WAN)
	nvram_unset("link_wan_bond");
#endif

	signal(SIGTERM, SIG_IGN);
	signal(SIGUSR1, SIG_IGN);
	signal(SIGUSR2, SIG_IGN);
	signal(SIGINT, SIG_IGN);

	FD_ZERO(&allset);
	close(http_sock);
	close(dns_sock);

	for(i = 0; i < MAX_USER && client[i].sfd < maxfd; ++i){
		if(client[i].sfd != -1){
			ret = close(client[i].sfd);
			_dprintf("## close %d: result=%d.\n", client[i].sfd, ret);
		}
	}

#ifndef RTCONFIG_BCMARM
	sleep(1);
#endif

#ifdef RTCONFIG_WIRELESSREPEATER
	if(sw_mode == SW_MODE_REPEATER){
		eval("ebtables", "-t", "broute", "-F");
		eval("ebtables", "-t", "filter", "-F");
		f_write_string("/proc/net/dnsmqctrl", "", 0, 0);
	}
#endif

	_dprintf("\n# Disable direct rule(exit wanduck)\n");

	rule_setup = 0;
	conn_changed_state[current_wan_unit] = CONNED; // for cleaning the redirect rules.

	nvram_set_int("nat_state", NAT_STATE_INITIALIZING);
	nat_state = start_nat_rules();

	remove(WANDUCK_PID_FILE);

	_dprintf("\n# return(exit wanduck)\n");
	exit(0);
}

void get_related_nvram(){
	int unit;

	sw_mode = sw_mode();

	boot_end = nvram_get_int("success_start_service");

#if defined(RTCONFIG_FW_JUMP)
	isFirstUse = 0;
#else
#if defined(RTAC58U) || defined(RTAC59U) || defined(RTAX58U) || defined(RTAX56U)
	if (!strncmp(nvram_safe_get("territory_code"), "CX/01", 5)
	 || !strncmp(nvram_safe_get("territory_code"), "CX/05", 5))
		isFirstUse = 0;
	else
#endif
	if(nvram_match("x_Setting", "1"))
		isFirstUse = 0;
	else
		isFirstUse = 1;
#endif

	nat_redirect_enable = nvram_get_int("nat_redirect_enable");

#ifdef RTCONFIG_WIRELESSREPEATER
	if(strlen(nvram_safe_get("wlc_ssid")) > 0)
		setAP = 1;
	else
		setAP = 0;
#endif

#ifdef RTCONFIG_DUALWAN
	snprintf(dualwan_mode, sizeof(dualwan_mode), "%s", nvram_safe_get("wans_mode"));
	snprintf(dualwan_wans, sizeof(dualwan_wans), "%s", nvram_safe_get("wans_dualwan"));

	if(is_router_mode()){
		if(!strcmp(dualwan_mode, "lb")){
			wandog_enable = 0;
			dnsprobe_enable = 0;
		}
		else{
			wandog_enable = nvram_get_int("wandog_enable");
			dnsprobe_enable = nvram_get_int("dns_probe");
		}

		scan_interval = nvram_get_int("wandog_interval");
		for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit)
			max_disconn_count[unit] = nvram_get_int("wandog_maxfail");
		wandog_delay = nvram_get_int("wandog_delay");

		if(!strcmp(dualwan_mode, "fb")){
			max_fb_count = nvram_get_int("wandog_fb_count");
			max_fb_wait_time = scan_interval*max_fb_count;
		}
	}
	else{
		wandog_enable = 0;
		dnsprobe_enable = 0;
		scan_interval = DEFAULT_SCAN_INTERVAL;
		for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit)
			max_disconn_count[unit] = DEFAULT_MAX_DISCONN_COUNT;
		wandog_delay = -1;
	}
#else
	wandog_enable = 0;
	dnsprobe_enable = 0;
	scan_interval = DEFAULT_SCAN_INTERVAL;
	for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit)
		max_disconn_count[unit] = DEFAULT_MAX_DISCONN_COUNT;
#endif
	for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit)
		max_wait_time[unit] = scan_interval*max_disconn_count[unit];

#ifdef RTCONFIG_USB_MODEM
	modem_pdp = nvram_get_int("modem_pdp");
#endif
}

void get_lan_nvram(){
	char nvram_name[16];

	current_lan_unit = nvram_get_int("lan_unit");

	if(current_lan_unit < 0)
		snprintf(prefix_lan, sizeof(prefix_lan), "lan_");
	else
		snprintf(prefix_lan, sizeof(prefix_lan), "lan%d_", current_lan_unit);

	snprintf(current_lan_ifname, sizeof(current_lan_ifname), "%s", nvram_safe_get(strcat_r(prefix_lan, "ifname", nvram_name)));
	snprintf(current_lan_proto, sizeof(current_lan_proto), "%s", nvram_safe_get(strcat_r(prefix_lan, "proto", nvram_name)));
	snprintf(current_lan_ipaddr, sizeof(current_lan_ipaddr), "%s", nvram_safe_get(strcat_r(prefix_lan, "ipaddr", nvram_name)));
	snprintf(current_lan_netmask, sizeof(current_lan_netmask), "%s", nvram_safe_get(strcat_r(prefix_lan, "netmask", nvram_name)));
	snprintf(current_lan_gateway, sizeof(current_lan_gateway), "%s", nvram_safe_get(strcat_r(prefix_lan, "gateway", nvram_name)));
	snprintf(current_lan_dns, sizeof(current_lan_dns), "%s", nvram_safe_get(strcat_r(prefix_lan, "dns", nvram_name)));
	snprintf(current_lan_subnet, sizeof(current_lan_subnet), "%s", nvram_safe_get(strcat_r(prefix_lan, "subnet", nvram_name)));

#ifdef RTCONFIG_WIRELESSREPEATER
#ifdef RTCONFIG_REALTEK
/* [MUST]: Need to disscuss to add new mode for Media Bridge */
	if(repeater_mode() || mediabridge_mode())
#else
	if(sw_mode == SW_MODE_REPEATER)
#endif
	{
		wlc_state = nvram_get_int("wlc_state");
		got_notify = 1;
	}
#endif

	_dprintf("# wanduck: Got LAN(%d) information:\n", current_lan_unit);
	if(test_log){
#ifdef RTCONFIG_WIRELESSREPEATER
#ifdef RTCONFIG_REALTEK
/* [MUST]: Need to disscuss to add new mode for Media Bridge */
		if(repeater_mode() || mediabridge_mode())
#else
		if(sw_mode == SW_MODE_REPEATER)
#endif
		{
			_dprintf("# wanduck:   ipaddr=%s.\n", current_lan_ipaddr);
			_dprintf("# wanduck:wlc_state=%d.\n", wlc_state);
		}
		else
#endif
		{
			_dprintf("# wanduck:   ifname=%s.\n", current_lan_ifname);
			_dprintf("# wanduck:    proto=%s.\n", current_lan_proto);
			_dprintf("# wanduck:   ipaddr=%s.\n", current_lan_ipaddr);
			_dprintf("# wanduck:  netmask=%s.\n", current_lan_netmask);
			_dprintf("# wanduck:  gateway=%s.\n", current_lan_gateway);
			_dprintf("# wanduck:      dns=%s.\n", current_lan_dns);
			_dprintf("# wanduck:   subnet=%s.\n", current_lan_subnet);
		}
	}
}

static void get_network_nvram(int signo){
#ifdef RTCONFIG_REALTEK
/* [MUST]: Need to disscuss to add new mode for Media Bridge */
	if(access_point_mode() 
#ifdef RTCONFIG_AMAS
			|| re_mode()
#endif
			)
#else
	if(sw_mode == SW_MODE_AP)
#endif
		get_lan_nvram();

#ifdef RTCONFIG_WIRELESSREPEATER
#ifdef RTCONFIG_REALTEK
/* [MUST]: Need to disscuss to add new mode for Media Bridge */
	if(repeater_mode() || mediabridge_mode())
#else
	if(sw_mode == SW_MODE_REPEATER)
#endif
		get_lan_nvram();
	else if(sw_mode == SW_MODE_HOTSPOT)
		wlc_state = nvram_get_int("wlc_state");
#endif
}

/* Return values:
    -1: ping target is disabled
     0: ping target has failed
     1: ping target ok
*/
int do_ping_detect(int wan_unit, const char *target)
{
	FILE *fp;
	char cmd[512];
	int count, ret = -1;
	int debug = nvram_get_int("ping_debug");

	/* can be default target, if necesary *//*
	if (!target)
		target = nvram_safe_get("ping_target");
	*/

	/* Check for valid domain to avoid shell escaping */
	if (!is_valid_domainname(target)
#if defined(RTCONFIG_IPV6) && defined(RTCONFIG_USB_MODEM)
		&& !(dualwan_unit__usbif(wan_unit) && modem_pdp == 2)
#endif
			)
		return -1;

	if (debug)
		_dprintf("%s: %s %s\n", __FUNCTION__, "check", target);

	snprintf(cmd, sizeof(cmd), "ping -c1 -w2 -s32 %s -Mdont '%s' 2>&1",
		 nvram_get_int("ttl_spoof_enable") ? "" : "-t128", target);
	if ((fp = popen(cmd, "r")) != NULL) {
		while (fgets(cmd, sizeof(cmd), fp) != NULL) {
			if (sscanf(cmd, "%*s %*s transmitted, %d %*s received", &count) == 1) {
				ret = (count > 0);
				if (debug)
					_dprintf("%s: %s %s\n", __FUNCTION__, target, ret ? "ok" : "fail");
				break;
			}
			else
				ret = 0;
		}
		pclose(fp);
	}

	return ret;
}

#if defined(DETECT_INTERNET_MORE) || defined(RTCONFIG_DUALWAN)
static int wanduck_ping_detect(int wan_unit)
{
#ifdef RTCONFIG_DUALWAN
	snprintf(wandog_target, sizeof(wandog_target), "%s", nvram_safe_get("wandog_target"));

	return do_ping_detect(wan_unit, wandog_target);
#else /* RTCONFIG_DUALWAN */
	return -1;
#endif
}
#endif

#ifdef DETECT_INTERNET_MORE
int get_packets_of_net_dev(const char *net_dev, unsigned long *rx_packets, unsigned long *tx_packets){
	FILE *fp;
	char buf[256];
	char *ifname;
	char *ptr;
	int i, got_packets = 0;

	if((fp = fopen(PROC_NET_DEV, "r")) == NULL){
		_dprintf("%s: Can't open the file: %s.\n", __FUNCTION__, PROC_NET_DEV);
		return got_packets;
	}

	fcntl(fileno(fp), F_SETFL, fcntl(fileno(fp), F_GETFL) | O_NONBLOCK);

	// headers.
	for(i = 0; i < 2; ++i){
		if(fgets(buf, sizeof(buf), fp) == NULL){
			fclose(fp);
			logmessage("wanduck", "%s: Can't read out the headers of %s.\n", __FUNCTION__, PROC_NET_DEV);
			if(errno == EAGAIN || errno == EWOULDBLOCK)
				got_packets = 1;

			return got_packets;
		}
	}

	while(fgets(buf, sizeof(buf), fp) != NULL){
		if((ptr = strchr(buf, ':')) == NULL)
			continue;

		*ptr = 0;
		if((ifname = strrchr(buf, ' ')) == NULL)
			ifname = buf;
		else
			++ifname;

		if(strcmp(ifname, net_dev))
			continue;

		// <rx bytes, packets, errors, dropped, fifo errors, frame errors, compressed, multicast><tx ...>
		if(sscanf(ptr+1, "%*u%lu%*u%*u%*u%*u%*u%*u%*u%lu", rx_packets, tx_packets) != 2){
			fclose(fp);
			logmessage("wanduck", "%s: Can't read the packet's number in %s.\n", __FUNCTION__, PROC_NET_DEV);
			if(errno == EAGAIN || errno == EWOULDBLOCK)
				got_packets = 1;

			return got_packets;
		}

		got_packets = 1;
		break;
	}
	fclose(fp);

	return got_packets;
}

char *organize_tcpcheck_cmd(char *dns_list, char *cmd, int size){
	char buf[256], *next;
	int len;

	if(cmd == NULL || size <= 0)
		return NULL;

	len = snprintf(cmd, size, "/sbin/tcpcheck %d", TCPCHECK_TIMEOUT);

	foreach(buf, dns_list, next){
		len += snprintf(cmd+len, size-len, " %s:53", buf);
	}

	len += snprintf(cmd+len, size-len, " >>%s", DETECT_FILE);

	return cmd;
}

int do_tcp_dns_detect(int wan_unit){
	FILE *fp = NULL;
	char line[80], cmd[PATH_MAX];
	char prefix_wan[8], nvram_name[16], wan_dns[256];

	if(remove(DETECT_FILE) < 0)
	{
		logmessage("%s: cannot remove DETECT_FILE.\n", __FUNCTION__);
		return 0;
	}

	snprintf(prefix_wan, sizeof(prefix_wan), "wan%d_", wan_unit);

#ifdef RTCONFIG_DNSPRIVACY
	if (nvram_get_int("dnspriv_enable"))
		strcpy(wan_dns, "127.0.1.1");
	else
#endif
	snprintf(wan_dns, sizeof(wan_dns), "%s", nvram_safe_get(strcat_r(prefix_wan, "dns", nvram_name)));

	if(organize_tcpcheck_cmd(wan_dns, cmd, PATH_MAX) == NULL){
		_dprintf("wanduck: No tcpcheck cmd.\n");
		return 0;
	}
	system(cmd);

	if((fp = fopen(DETECT_FILE, "r")) == NULL){
		_dprintf("wanduck: No file: %s.\n", DETECT_FILE);
		return 0;
	}

	fcntl(fileno(fp), F_SETFL, fcntl(fileno(fp), F_GETFL) | O_NONBLOCK);

	while(fgets(line, sizeof(line), fp) != NULL){
		if(strstr(line, "alive")){
			fclose(fp);
			return 1;
		}
	}
	fclose(fp);

	return 0;
}
#endif // DETECT_INTERNET_MORE

#include <sys/wait.h>
#include <resolv.h>
#include <setjmp.h>

static sigjmp_buf getaddrinfo_jmpbuf;
static volatile int getaddrinfo_jmpset;

static void getaddrinfo_alarm(int sig)
{
	if (getaddrinfo_jmpset) {
		getaddrinfo_jmpset = 0;
		siglongjmp(getaddrinfo_jmpbuf, 1);
	}
}

/* Return values:
    -1: error or disable
     0: dns probe has failed
     1: dns probe ok
*/
int do_dns_detect(int wan_unit)
{
	struct addrinfo hints, *res, *ai;
	union {
		struct in_addr in;
		struct in6_addr in6;
	} *addr, target;
	sigset_t set;
	char status, word[64], *next, host[PATH_MAX], content[PATH_MAX];
	int timeout, size, ret, pipefd[2];
	int debug = nvram_get_int("dns_probe_debug");
#ifdef RTCONFIG_DUALWAN
	int backup_det = (wan_unit != wan_primary_ifunit() && wan_unit != WAN_UNIT_NONE) ? 1 : 0;
#endif

#if defined(RTCONFIG_IPV6) && defined(RTCONFIG_USB_MODEM)
	if(dualwan_unit__usbif(wan_unit) && modem_pdp == 2)
		return 1;
#endif

	snprintf(host, sizeof(host), "%s", nvram_safe_get("dns_probe_host"));
	// the dns probe is disabled
	if(*host == '\0')
		return 1;

	snprintf(content, sizeof(content), "%s", nvram_safe_get("dns_probe_content"));
	if (*content == '\0' || *host == '\0') {
		snprintf(content, sizeof(content), "%s", nvram_default_get("dns_probe_content"));
		snprintf(host, sizeof(host), "%s", nvram_default_get("dns_probe_host"));
	}
	if (debug)
		_dprintf("%s: %s %s %s\n", __FUNCTION__, "check", host, content);

	/* Check for valid domain to avoid shell escaping */
	if (!is_valid_domainname(host) || *content == '\0')
		return -1;

	memset(&hints, 0, sizeof(hints));
#ifdef RTCONFIG_IPV6
	hints.ai_family = ipv6_enabled() ? AF_UNSPEC : AF_INET;
#else
	hints.ai_family = AF_INET;
#endif
	hints.ai_socktype = SOCK_STREAM;
	timeout = nvram_get_int("dns_probe_timeout") ? : scan_interval-2;

	ret = -1;
	if (pipe(pipefd) < 0)
		goto error;

	switch (fork()) {
	case -1:
		close(pipefd[1]);
		goto error;
	case 0:
		/* child */
		close(pipefd[0]);
		break;
	default:
		/* parent */
		close(pipefd[1]);
		do {
			ret = read(pipefd[0], &status, sizeof(status));
		} while (ret < 0 && errno == EINTR);

		/* ret > 0: status has been read, return real status
		 * ret = 0: child timeout or dead w/o status, return 0
		 * ret < 0: child read error, return -1 */
		if (ret >= sizeof(status))
			ret = status;
		else if (ret != 0)
			ret = -1;
	error:
		close(pipefd[0]);

		if (debug)
			_dprintf("%s: %s ret %d\n", __FUNCTION__, host, ret);

		return ret;
	}

	/* Unblock signals */
	sigemptyset(&set);
	sigaddset(&set, SIGTERM);
	sigaddset(&set, SIGPIPE);
	sigaddset(&set, SIGALRM);
	sigprocmask(SIG_UNBLOCK, &set, NULL);

	/* Restore signals */
	signal(SIGTERM, SIG_DFL);
	signal(SIGPIPE, SIG_DFL);
	signal(SIGALRM, getaddrinfo_alarm);

	status = 0;

	/* keep using libc's resolver state
	res_init();
	*/

	if (sigsetjmp(getaddrinfo_jmpbuf, 1))
		goto dns_timeout;

	getaddrinfo_jmpset = 1;
	alarm(timeout);

#ifdef RTCONFIG_DUALWAN
	if (backup_det) {
		// use first dns server of wanX_dns
		struct in_addr dns_server;
		struct sockaddr_in dns_server_sock;
		char nv[16], wan_dns[16];
		char *p;
		snprintf(nv, sizeof(nv), "wan%d_dns", wan_unit);
		nvram_safe_get_r(nv, wan_dns, sizeof(wan_dns));
		p = strchr(wan_dns, ' ');
		if (p) *p = '\0';
		if (inet_pton(AF_INET, wan_dns, &dns_server) != 1) {
			_dprintf("dns server %s is error\n", wan_dns);
			return -1;
		}
		dns_server_sock.sin_family = AF_INET;
		dns_server_sock.sin_port = htons(53);
		dns_server_sock.sin_addr = dns_server;
		if (!(_res.options & RES_INIT))
			res_init();
		_res.nscount = 1;
		_res.nsaddr_list[0] = dns_server_sock;
	}
#endif

	ret = getaddrinfo(host, NULL, &hints, &res);
	getaddrinfo_jmpset = 0;

	if (ret == 0) {
		if (*content == '\0' && res)
			status = 1;
		for (ai = res; !status && ai; ai = ai->ai_next) {
			if (ai->ai_family == AF_INET) {
				addr = (void *)&((struct sockaddr_in *)ai->ai_addr)->sin_addr;
				size = sizeof(addr->in);
			}
#ifdef RTCONFIG_IPV6
			else if (ai->ai_family == AF_INET6) {
				addr = (void *)&((struct sockaddr_in6 *)ai->ai_addr)->sin6_addr;
				size = sizeof(addr->in6);
			}
#endif
			else
				continue;

			if (debug) {
				inet_ntop(ai->ai_family, addr, word, sizeof(word));
				_dprintf("%s: %s %s %s\n", __FUNCTION__, "resolve", host, word);
			}

			foreach(word, content, next) {
				if ((strcmp(word, "*") == 0 && inet_pton(ai->ai_family, "10.0.0.1", &target) > 0 && memcmp(addr, &target, size) != 0) ||
					(inet_pton(ai->ai_family, word, &target) > 0 && memcmp(addr, &target, size) == 0)) {
					status = 1;
					break;
				}
			}
		}
		freeaddrinfo(res);
	}

dns_timeout:
	do {
		ret = write(pipefd[1], &status, sizeof(status));
	} while (ret < 0 && errno == EINTR);

	close(pipefd[1]);

	_exit(ret != sizeof(status));
}

int delay_dns_response(int wan_unit)
{
	static int last = -1;
	static int fail = 0;
	static int skip = 0;
	int delay_round = nvram_get_int("dns_delay_round");
	int debug = nvram_get_int("dns_probe_debug");
	int ret;

	if (last > 0 && skip > 0) {
		skip--;
		return last;
	}

	ret = do_dns_detect(wan_unit);
	if (ret > 0) {
		fail = 0;
		skip = delay_round;
	} else if (ret < 0) {
		return ret;
	} else if (fail++ < delay_round && last >= 0) {
		return last;
	}

	if (debug && ret != last)
		logmessage("WAN Connection", "DNS probe %s", ret ? "succeeded" : "failed");
	last = ret;

	return ret;
}

int if_wan_ppp(int wan_unit, int pppoe){
	char tmp[100], prefix[16];
	char wan_proto[16];
	int wan_ppp;

	snprintf(prefix, sizeof(prefix), "wan%d_", wan_unit);
	snprintf(wan_proto, sizeof(wan_proto), "%s", nvram_safe_get(strcat_r(prefix, "proto", tmp)));

	wan_ppp = dualwan_unit__nonusbif(wan_unit) &&
			((pppoe && !strcmp(wan_proto, "pppoe"))
					|| !strcmp(wan_proto, "pptp")
					|| !strcmp(wan_proto, "l2tp"));

	return wan_ppp;
}

int op_resolve(char *name, struct addrinfo **res) {
	struct addrinfo hints;
	int err;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_RAW;

	if ((err = getaddrinfo(name, NULL, &hints, res)) != 0) {
		fprintf(stderr, "%s\n", strerror(err));
		return -1;
	}
	return 0;
}

#ifdef RTCONFIG_DUALWAN
int do_backup_ping_detect(int wan_unit)
{
	char target[128] = {0};
	char wan_prefix[8] = {0};
	char wan_ifname[16] = {0};
	char wan_gateway[16] = {0};
	struct addrinfo *res, *p;
	char addr[64] = {0};
	const char* paddr = NULL;
	FILE *fp;
	char cmd[128];
	int route = 0;
	int count = 0;
	int ret = 0;

	strlcpy(target, nvram_safe_get("wandog_target"), sizeof(target));
	snprintf(wan_prefix, sizeof(wan_prefix), "wan%d_", wan_unit);

	if (op_resolve(target, &res) < 0)
		return -1;
	for(p = res; p != NULL; p = p->ai_next) {
		// if (p->ai_family == AF_INET6)
			// paddr = inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)(p->ai_addr))->sin6_addr), addr, INET6_ADDRSTRLEN);
		// else
			paddr = inet_ntop(AF_INET, &(((struct sockaddr_in *)(p->ai_addr))->sin_addr), addr, INET_ADDRSTRLEN);
		if (paddr)
			break;
	}

	if (paddr) {
		// add route
		strlcpy(wan_ifname, get_wan_ifname(wan_unit), sizeof(wan_ifname));
		strlcpy(wan_gateway, nvram_pf_safe_get(wan_prefix, "gateway"), sizeof(wan_gateway));
		route = route_exist(wan_ifname, 0, addr, wan_gateway, "255.255.255.255");
		if (route == 0)
			route_add(wan_ifname, 0, addr, wan_gateway, "255.255.255.255");
		//ping
		snprintf(cmd, sizeof(cmd), "ping -c1 -w2 -s32 -Mdont -I %s '%s' 2>&1", wan_ifname, addr);
		if ((fp = popen(cmd, "r")) != NULL) {
			while (fgets(cmd, sizeof(cmd), fp) != NULL) {
				if (sscanf(cmd, "%*s %*s transmitted, %d %*s received", &count) == 1) {
					ret = (count > 0);
					break;
				}
				else
					ret = 0;
			}
			pclose(fp);
		}
		// del route
		if (route == 0)
			route_del(wan_ifname, 0, addr, wan_gateway, "255.255.255.255");
		return (ret);
	}
	return 0;
}

int detect_backup_internet(int wan_unit)
{
	int dns_ret, ping_ret;

	dns_ret = (dnsprobe_enable) ? do_dns_detect(wan_unit) : -1;
	ping_ret = (wandog_enable) ? do_backup_ping_detect(wan_unit) : -1;
	if ((dnsprobe_enable && dns_ret > 0 && !wandog_enable)
	 || (wandog_enable && ping_ret > 0 && !dnsprobe_enable)
	 || (dnsprobe_enable && dns_ret > 0 && wandog_enable && ping_ret > 0)
	) {
		_dprintf("Detect WAN(%d) internet success.\n", wan_unit);
		return CONNED;
	}
	else {
		_dprintf("Detect WAN(%d) internet failed.\n", wan_unit);
		return DISCONN;
	}
}

int switch_backup_line(int wan_unit, int restart_other)
{
	char wan_ifname[16] = {0};
	int unit = 0;
	char buf[32] = {0};

	strlcpy(wan_ifname, get_wan_ifname(wan_unit), sizeof(wan_ifname));

	_dprintf("%s: switch to WAN %d\n", __FUNCTION__, wan_unit);

	set_wan_primary_ifunit(wan_unit);
	wan_up(wan_ifname);
#ifdef DSL_AX82U
	if (is_custom_i2()) {
		update_iptv_ifname(wan_unit);
		start_igmpproxy(wan_ifname);
	}
#endif

	if (restart_other) {
		for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit){
			if(unit == wan_unit)
				continue;

			_dprintf("wanduck: restart_wan_if %d.\n", unit);
			snprintf(buf, sizeof(buf), "restart_wan_if %d", unit);
			notify_rc(buf);
		}
	}

	return 0;
}
#endif

int detect_internet(int wan_unit)
{
#ifdef DETECT_INTERNET_MORE
	char wan_ifname[16];
	unsigned long rx_packets, tx_packets;
#endif
	int link_internet;
	int wan_ppp, is_ppp_demand, ppp_echo_dns;
	int dns_ret, ping_ret;
	char tmp[100], prefix[16];

#ifdef DETECT_INTERNET_MORE
	snprintf(wan_ifname, sizeof(wan_ifname), "%s", get_wan_ifname(wan_unit));
#endif


	snprintf(prefix, sizeof(prefix), "wan%d_", wan_unit);

	wan_ppp = if_wan_ppp(wan_unit, 1);

	/* Don't trigger demand PPP connections with DNS probes & ping */
	is_ppp_demand = (wan_ppp && nvram_get_int(strcat_r(prefix, "pppoe_demand", tmp)));
#if defined(RTCONFIG_DUALWAN)
	ppp_echo_dns = (wan_ppp && nvram_get_int(strcat_r(prefix, "ppp_echo", tmp)) == 2 && strcmp(dualwan_mode, "lb"));
#else
	ppp_echo_dns = (wan_ppp && nvram_get_int(strcat_r(prefix, "ppp_echo", tmp)) == 2);
#endif

	if(isFirstUse)
		dns_ret = delay_dns_response(wan_unit);
	else if(dnsprobe_enable || ppp_echo_dns)
		dns_ret = is_ppp_demand ? -1 : delay_dns_response(wan_unit);
	else
		dns_ret = -1;

#if defined(RTCONFIG_IPV6) && defined(RTCONFIG_INTERNAL_GOBI)
	if(dualwan_unit__usbif(wan_unit) && modem_pdp == 2)
		ping_ret = do_ping_detect(wan_unit, "2001:4860:4860::8888");
	else
#endif
#if defined(RTCONFIG_DUALWAN)
	if(wandog_enable)
		ping_ret = is_ppp_demand ? -1 : wanduck_ping_detect(wan_unit);
	else
#endif
		ping_ret = -1;

	if(
#ifdef RTCONFIG_DUALWAN
			strcmp(dualwan_mode, "lb") &&
#endif
			!found_default_route(wan_unit))
		link_internet = DISCONN;
#ifdef DETECT_INTERNET_MORE
	else if(!get_packets_of_net_dev(wan_ifname, &rx_packets, &tx_packets) || rx_packets <= RX_THRESHOLD)
		link_internet = DISCONN;
	else if(!isFirstUse && (!dns_ret && !do_tcp_dns_detect(wan_unit) && !wanduck_ping_detect(wan_unit)))
		link_internet = DISCONN;
#endif
#if defined(RTCONFIG_IPV6) && defined(RTCONFIG_INTERNAL_GOBI)
	else if(dualwan_unit__usbif(wan_unit) && modem_pdp == 2 && !ping_ret)
		link_internet = DISCONN;
#endif
#if 0
	else if((!strcmp(dualwan_mode, "fo") || !strcmp(dualwan_mode, "fb"))
			&& wandog_enable == 1 && !isFirstUse && !wanduck_ping_detect(wan_unit)){
		link_internet = DISCONN;

		// avoid the nat rules had be applied by wan_up before wanduck.
		if(nvram_get_int("nat_state") == NAT_STATE_NORMAL)
			nat_state = stop_nat_rules();
	}
#else
	else if((!ping_ret && !dnsprobe_enable)
			|| (!dns_ret && !wandog_enable)
			|| (!ping_ret && !dns_ret)
			){
		link_internet = DISCONN;

		// avoid the nat rules had be applied by wan_up before wanduck.
		if(nvram_get_int("nat_state") == NAT_STATE_NORMAL)
			nat_state = stop_nat_rules();
	}
#endif
	else if(ppp_echo_dns /* PPP connections with DNS detection */
			&& !dns_ret)
		link_internet = DISCONN;
	else
		link_internet = CONNED;

	/* Set no DNS state even if connected for WEB UI */
	if(link_internet == DISCONN){
		if(nvram_get_int("web_redirect") & WEBREDIRECT_FLAG_NOINTERNET)
			set_link_internet(wan_unit, 1);
		else{
			set_link_internet(wan_unit, 2);
			link_internet = CONNED;
		}
		record_wan_state_nvram(wan_unit, -1, -1, WAN_AUXSTATE_NO_INTERNET_ACTIVITY);
	}
	else{
		set_link_internet(wan_unit, 2);
		record_wan_state_nvram(wan_unit, -1, -1, WAN_AUXSTATE_NONE);
	}

	return link_internet;
}

int passivesock(char *service, int protocol_num, int qlen){
	//struct servent *pse;
	struct sockaddr_in sin;
	int s, type, on;

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;

	// map service name to port number
	if((sin.sin_port = htons((u_short)atoi(service))) == 0){
		perror("cannot get service entry");

		return -1;
	}

	if(protocol_num == IPPROTO_UDP)
		type = SOCK_DGRAM;
	else
		type = SOCK_STREAM;

	s = socket(PF_INET, type, protocol_num);
	if(s < 0){
		perror("cannot create socket");

		return -1;
	}

	on = 1;
	if(setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on)) < 0){
		perror("cannot set socket's option: SO_REUSEADDR");
		close(s);

		return -1;
	}

	if(bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0){
		perror("cannot bind port");
		close(s);

		return -1;
	}

	if(type == SOCK_STREAM && listen(s, qlen) < 0){
		perror("cannot listen to port");
		close(s);

		return -1;
	}

	return s;
}

#if 0
int check_ppp_exist(){
	DIR *dir;
	struct dirent *dent;
	char task_file[64], cmdline[64];
	int pid, fd;

	if((dir = opendir("/proc")) == NULL){
		perror("open proc");
		return 0;
	}

	while((dent = readdir(dir)) != NULL){
		if((pid = atoi(dent->d_name)) > 1){
			snprintf(task_file, sizeof(task_file), "/proc/%d/cmdline", pid);
			if((fd = open(task_file, O_RDONLY | O_NONBLOCK)) > 0){
				memset(cmdline, 0, 64);
				read(fd, cmdline, 64);
				close(fd);

				if(strstr(cmdline, "pppd")
						|| strstr(cmdline, "l2tpd")
						|| (errno==EAGAIN || errno==EWOULDBLOCK)
						){
					closedir(dir);
					return 1;
				}
			}
			else
				printf("cannot open %s\n", task_file);
		}
	}
	closedir(dir);

	return 0;
}
#endif

unsigned long long get_wan_flow(int wan_unit){
	unsigned long long rx, tx, total;

	if(dualwan_unit__usbif(wan_unit)){
		rx = strtoull(nvram_safe_get("modem_bytes_rx"), NULL, 10);
		tx = strtoull(nvram_safe_get("modem_bytes_tx"), NULL, 10);

		total = rx+tx;
	}
	else{
		// TODO: Data limit for the ethernet connection.
#ifdef RTCONFIG_TRAFFIC_LIMITER
		total = (unsigned long long)traffic_limiter_get_realtime(wan_unit) * 1024 * 1024 * 1024;
		if(test_log) _dprintf("[TRAFFIC LIMITER] /tmp/tl%d_realtime = %lld\n", wan_unit, total);
#else
		total = 0;
#endif
	}

	return total;
}

int chk_proto(int wan_unit){
	int wan_sbstate = nvram_get_int(nvram_sbstate[wan_unit]);
	char prefix_wan[8], nvram_name[16], wan_proto[16];
	pid_t pid;
#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2) || defined(RTCONFIG_UBIFS)
	char buff[128];
#ifdef RTCONFIG_USB_MODEM
	unsigned long long total = get_wan_flow(wan_unit);
#endif
#endif
#ifdef RTCONFIG_TRAFFIC_LIMITER
	char cmd[32];
#endif

	snprintf(prefix_wan, sizeof(prefix_wan), "wan%d_", wan_unit);

	snprintf(wan_proto, sizeof(wan_proto), "%s", nvram_safe_get(strcat_r(prefix_wan, "proto", nvram_name)));

#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2) || defined(RTCONFIG_UBIFS)
	// had detected the DATA limit before.
	if(wan_sbstate == WAN_STOPPED_REASON_DATALIMIT){
		if(nvram_get_int("modem_sms_limit") == 1 && nvram_get_int("modem_sms_limit_send") == 0){
			snprintf(buff, sizeof(buff), "start_sendSMS limit");
			nvram_set("freeze_duck", "5");
			notify_rc(buff);
			nvram_set("modem_sms_limit_send", "1");
			// if send the limit SMS, the alert SMS is not needed.
			nvram_set("modem_sms_alert_send", "1");
		}

		disconn_case[wan_unit] = CASE_DATALIMIT;
		return DISCONN;
	}
	else
#endif
#ifdef RTCONFIG_WIRELESSREPEATER
	if(sw_mode == SW_MODE_HOTSPOT){
		if(current_state[wan_unit] == WAN_STATE_STOPPED) {
			if(wan_sbstate == WAN_STOPPED_REASON_INVALID_IPADDR)
				disconn_case[wan_unit] = CASE_THESAMESUBNET;
			else disconn_case[wan_unit] = CASE_DHCPFAIL;
			return DISCONN;
		}
		else if(current_state[wan_unit] == WAN_STATE_INITIALIZING){
			disconn_case[wan_unit] = CASE_DHCPFAIL;
			return DISCONN;
		}
		else if(current_state[wan_unit] == WAN_STATE_CONNECTING){
			disconn_case[wan_unit] = CASE_DHCPFAIL;
			return DISCONN;
		}
		else if(current_state[wan_unit] == WAN_STATE_DISCONNECTED){
			disconn_case[wan_unit] = CASE_DHCPFAIL;
			return DISCONN;
		}
	}
	else
#endif
	// Start chk_proto() in SW_MODE_ROUTER.
#ifdef RTCONFIG_USB_MODEM
	if (dualwan_unit__usbif(wan_unit)) {
		int case_fail = (strcmp(wan_proto, "dhcp") == 0) ? CASE_DHCPFAIL : CASE_PPPFAIL;
#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2) || defined(RTCONFIG_UBIFS)
		unsigned long long alert, limit;

		eval("/usr/sbin/modem_status.sh", "bytes");

		alert = strtoull(nvram_safe_get("modem_bytes_data_warning"), NULL, 10);
		limit = strtoull(nvram_safe_get("modem_bytes_data_limit"), NULL, 10);

		if(limit > 0 && total >= limit){
			if(current_state[wan_unit] != WAN_STATE_STOPPED || wan_sbstate != WAN_STOPPED_REASON_DATALIMIT){
				_dprintf("wanduck(%d): chk_proto() detect the data limit.\n", wan_unit);
				record_wan_state_nvram(wan_unit, WAN_STATE_STOPPED, WAN_STOPPED_REASON_DATALIMIT, -1);
				current_state[wan_unit] = WAN_STATE_STOPPED;
				wan_sbstate = WAN_STOPPED_REASON_DATALIMIT;
			}

			if(nvram_get_int("modem_sms_limit") == 1 && nvram_get_int("modem_sms_limit_send") == 0){
				snprintf(buff, sizeof(buff), "start_sendSMS limit");
				nvram_set("freeze_duck", "5");
				notify_rc(buff);
				nvram_set("modem_sms_limit_send", "1");
				// if send the limit SMS, the alert SMS is not needed.
				nvram_set("modem_sms_alert_send", "1");
			}
		}

		if(alert > 0 && total >= alert){
			if(nvram_get_int("modem_sms_limit") == 1 && nvram_get_int("modem_sms_alert_send") == 0){
				snprintf(buff, sizeof(buff), "start_sendSMS alert");
				nvram_set("freeze_duck", "5");
				notify_rc(buff);
				nvram_set("modem_sms_alert_send", "1");
			}
		}
#endif
		if(current_state[wan_unit] == WAN_STATE_INITIALIZING){
			disconn_case[wan_unit] = case_fail;
			return DISCONN;
		}
		else if(current_state[wan_unit] == WAN_STATE_CONNECTING){
			ppp_fail_state = pppstatus(wan_unit);

			if(ppp_fail_state != WAN_STOPPED_REASON_NONE)
				record_wan_state_nvram(wan_unit, -1, ppp_fail_state, -1);

			disconn_case[wan_unit] = case_fail;
			return DISCONN;
		}
		else if(current_state[wan_unit] == WAN_STATE_DISCONNECTED){
			disconn_case[wan_unit] = case_fail;
			return DISCONN;
		}
		else if(current_state[wan_unit] == WAN_STATE_STOPPED){
#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2) || defined(RTCONFIG_UBIFS)
			if(wan_sbstate == WAN_STOPPED_REASON_DATALIMIT)
				disconn_case[wan_unit] = CASE_DATALIMIT;
			else
#endif
			if(wan_sbstate == WAN_STOPPED_REASON_INVALID_IPADDR)
				disconn_case[wan_unit] = CASE_THESAMESUBNET;
			else
				disconn_case[wan_unit] = case_fail;

			return DISCONN;
		}

		return CONNED;
	}
#endif // RTCONFIG_USB_MODEM

	// PPPoE detect
	if(isFirstUse){
		char *autodet_argv[] = {"autodet", NULL};

		_eval(autodet_argv, NULL, 0, &pid);
#ifdef RTCONFIG_SOFTWIRE46
		char *auto46det_argv[] = {"auto46det", NULL};

		_eval(auto46det_argv, NULL, 0, &pid);
#endif
	}

	if(!if_wan_ppp(wan_unit, 1)){
#ifdef RTCONFIG_TRAFFIC_LIMITER
		traffic_limiter_limitdata_check();

		// TODO: Data limit for the ethernet connection.
		if(traffic_limiter_wanduck_check(wan_unit)){
			if(current_state[wan_unit] != WAN_STATE_STOPPED || wan_sbstate != WAN_STOPPED_REASON_DATALIMIT){
				_dprintf("wanduck(%d): chk_proto() detect the data limit - dhcp / static.\n", wan_unit);
				record_wan_state_nvram(wan_unit, WAN_STATE_STOPPED, WAN_STOPPED_REASON_DATALIMIT, -1);
				current_state[wan_unit] = WAN_STATE_STOPPED;
				wan_sbstate = WAN_STOPPED_REASON_DATALIMIT;

				/* stop_wan_if() */
				snprintf(cmd, sizeof(cmd), "stop_wan_if %d", wan_unit);
				notify_rc(cmd);
			}
		}
#endif

		if(current_state[wan_unit] == WAN_STATE_INITIALIZING){
			disconn_case[wan_unit] = CASE_DHCPFAIL;
			return DISCONN;
		}
		else if(current_state[wan_unit] == WAN_STATE_CONNECTING){
			disconn_case[wan_unit] = CASE_DHCPFAIL;
			return DISCONN;
		}
		else if(current_state[wan_unit] == WAN_STATE_DISCONNECTED){
			disconn_case[wan_unit] = CASE_DHCPFAIL;
			return DISCONN;
		}
		else if(current_state[wan_unit] == WAN_STATE_STOPPED) {
#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2) || defined(RTCONFIG_UBIFS)
			if(wan_sbstate == WAN_STOPPED_REASON_DATALIMIT)
				disconn_case[wan_unit] = CASE_DATALIMIT;
			else
#endif
			if(wan_sbstate == WAN_STOPPED_REASON_INVALID_IPADDR)
				disconn_case[wan_unit] = CASE_THESAMESUBNET;
			else
				disconn_case[wan_unit] = CASE_DHCPFAIL;

			return DISCONN;
		}
	}
	else{
		ppp_fail_state = pppstatus(wan_unit);
#ifdef RTCONFIG_TRAFFIC_LIMITER
		traffic_limiter_limitdata_check();

		// TODO: Data limit for the ethernet connection.
		if(traffic_limiter_wanduck_check(wan_unit)){
			if(current_state[wan_unit] != WAN_STATE_STOPPED || wan_sbstate != WAN_STOPPED_REASON_DATALIMIT){
				_dprintf("wanduck(%d): chk_proto() detect the data limit - pppoe / pptp / l2tp.\n", wan_unit);
				record_wan_state_nvram(wan_unit, WAN_STATE_STOPPED, WAN_STOPPED_REASON_DATALIMIT, -1);
				current_state[wan_unit] = WAN_STATE_STOPPED;
				wan_sbstate = WAN_STOPPED_REASON_DATALIMIT;

				/* stop_wan_if() */
				snprintf(cmd, sizeof(cmd), "stop_wan_if %d", wan_unit);
				notify_rc(cmd);
			}
		}
#endif

		if(current_state[wan_unit] != WAN_STATE_CONNECTED
				&& ppp_fail_state == WAN_STOPPED_REASON_PPP_LACK_ACTIVITY){
			// PPP is into the idle mode.
			if(current_state[wan_unit] == WAN_STATE_STOPPED) // Sometimes ip_down() didn't set it.
				record_wan_state_nvram(wan_unit, -1, WAN_STOPPED_REASON_PPP_LACK_ACTIVITY, -1);
		}
		else if(current_state[wan_unit] == WAN_STATE_INITIALIZING){
			disconn_case[wan_unit] = CASE_PPPFAIL;
			return DISCONN;
		}
		else if(current_state[wan_unit] == WAN_STATE_CONNECTING){
			if(ppp_fail_state != WAN_STOPPED_REASON_NONE)
				record_wan_state_nvram(wan_unit, -1, ppp_fail_state, -1);

			disconn_case[wan_unit] = CASE_PPPFAIL;
			return DISCONN;
		}
		else if(current_state[wan_unit] == WAN_STATE_DISCONNECTED){
			disconn_case[wan_unit] = CASE_PPPFAIL;
			return DISCONN;
		}
		else if(current_state[wan_unit] == WAN_STATE_STOPPED){
#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2) || defined(RTCONFIG_UBIFS)
			if(wan_sbstate == WAN_STOPPED_REASON_DATALIMIT)
				disconn_case[wan_unit] = CASE_DATALIMIT;
			else
#endif
				disconn_case[wan_unit] = CASE_PPPFAIL;
			return DISCONN;
		}
	}

	return CONNED;
}

#ifdef RTCONFIG_ALPINE
int if_10G_phyconnected(){ // 1: 10G Eth, 2: 10G SFP+.
	int debug = nvram_get_int("debug_10g");
	const char *intf_10g[] = {"eth0", "eth2", NULL};
	char path[128], str[8], name[32];
	int i = 0;

	for(i = 0; intf_10g[i] != NULL; ++i){
		snprintf(name, sizeof(name), "link_10G%d", i+1);
		snprintf(path, sizeof(path), "/sys/class/net/%s/operstate", intf_10g[i]);
		f_read_string(path, str, sizeof(str));
		if(debug) _dprintf("name=%s, path=%s, str=%s\n", name, path, str);

		if(!strncmp(str, "up", 2))
			nvram_set(name, "1");
		else
			nvram_set(name, "0");
	}

	return 0;
}
#endif

int if_wan_phyconnected(int wan_unit){
	char *ptr, wired_link_nvram[32];
#ifdef RTCONFIG_WIRELESSREPEATER
	int link_ap = 0;
#endif
	char tmp[100], prefix[32];
	char wan_proto[16];
#ifdef RTCONFIG_USB_MODEM
	int wan_state = 0;
	int sim_state = 0;
	int modem_unit;
	char tmp2[100], prefix2[32];
	char env_unit[32];
	char modem_type[32];
#ifdef RTCONFIG_INTERNAL_GOBI
	char usb_if[16];
	static int got_modem_info = 0;
	char buf[32];
	char act_ip[32];
#endif

#ifdef RTCONFIG_ALPINE
	if(!wan_unit) // only be ecexute once
		if_10G_phyconnected();
#endif

#ifdef RTCONFIG_DYN_MODEM
#ifdef RTCONFIG_DUALWAN
	if(wan_unit == WAN_UNIT_FIRST && dualwan_unit__nonusbif(wan_unit) && get_dualwan_by_unit(other_wan_unit) == WANS_DUALWAN_IF_NONE){
		snprintf(prefix, sizeof(prefix), "wan%d_", other_wan_unit);

		wan_state = nvram_get_int(nvram_state[other_wan_unit]);
		modem_unit = get_modemunit_by_type(get_dualwan_by_unit(other_wan_unit));
if(test_log)
_dprintf("%s: DYN_MODEM: modem_unit=%d!\n", __FUNCTION__, modem_unit);

		if(modem_unit == MODEM_UNIT_NONE){
			_dprintf("%s 1: cannot get the modem unit!\n", __FUNCTION__);
			return conn_state_old[other_wan_unit];
		}

		usb_modem_prefix(modem_unit, prefix2, sizeof(prefix2));

		snprintf(env_unit, sizeof(env_unit), "unit=%d", modem_unit);
		putenv(env_unit);

#ifdef RTCONFIG_INTERNAL_GOBI
		if(strlen(usb_if) <= 0 && nvram_get_int("usb_gobi") && !strcmp(nvram_safe_get(strcat_r(prefix2, "act_type", tmp2)), "gobi")){
			snprintf(usb_if, sizeof(usb_if), "%s", nvram_safe_get(strcat_r(prefix, "ifname", tmp)));
_dprintf("wanduck: modem_unit=%d, try to get usb_if=%s.\n", modem_unit, usb_if);
		}

		if(strlen(usb_if) > 0){
			if(strlen(nvram_safe_get(strcat_r(prefix2, "act_imei", tmp2))) <= 0)
				eval("/usr/sbin/modem_status.sh", "imei");
			if(got_modem_info < 3 && strlen(nvram_safe_get(strcat_r(prefix2, "act_hwver", tmp2))) <= 0){
				++got_modem_info;
				eval("/usr/sbin/modem_status.sh", "hwver");
			}
			if(strlen(nvram_safe_get(strcat_r(prefix2, "act_swver", tmp2))) <= 0)
				eval("/usr/sbin/modem_status.sh", "swver");
		}
#endif

		link_wan[other_wan_unit] = is_usb_modem_ready(get_dualwan_by_unit(other_wan_unit));
		if(link_wan[other_wan_unit]){
			snprintf(modem_type, sizeof(modem_type), "%s", nvram_safe_get(strcat_r(prefix2, "act_type", tmp2)));
			if(strlen(modem_type) <= 0){
				eval("/usr/sbin/find_modem_type.sh");
				snprintf(modem_type, sizeof(modem_type), "%s", nvram_safe_get(strcat_r(prefix2, "act_type", tmp2)));
			}
		}

		if(!nvram_get(strcat_r(prefix2, "act_sim", tmp2)))
			sim_state = 100; // 100: didn't detect the SIM status yet.
		else
			sim_state = nvram_get_int(strcat_r(prefix2, "act_sim", tmp2));

		if(!strcmp(modem_type, "tty") || !strcmp(modem_type, "mbim") || !strcmp(modem_type, "qmi") || !strcmp(modem_type, "gobi")){
			if(link_wan[other_wan_unit] == 1){
				if(!strcmp(nvram_safe_get(strcat_r(prefix2, "act_int", tmp2)), "")){
					if(!strcmp(modem_type, "qmi")){	// e.q. Huawei E398.
						_dprintf("wanduck(%d): Sleep 3 seconds to wait modem nodes.\n", other_wan_unit);
						sleep(3);
					}
#if 0
					else{
						_dprintf("wanduck(%d): Sleep 2 seconds to wait modem nodes.\n", other_wan_unit);
						sleep(2);
					}
#endif

					wan_state = nvram_get_int(nvram_state[other_wan_unit]); // after sleep(), wan_state is changed.
				}

#if 0
				if(!strcmp(modem_type, "tty") && !strcmp(nvram_safe_get(strcat_r(prefix2, "act_vid", tmp2)), "6610")){ // e.q. ZTE MF637U
					if(wan_state == WAN_STATE_INITIALIZING && sim_state <= 0)
						eval("/usr/sbin/modem_status.sh", "sim");
				}
				else
#endif
				if(wan_state != WAN_STATE_CONNECTING || sim_state == 100){
					if(!strcmp(modem_type, "gobi"))
						eval("/usr/sbin/modem_status.sh", "sim");
					else if(sim_state == 100 || sim_state == -2) // QMI
						eval("/usr/sbin/modem_status.sh", "sim");
				}

				sim_state = nvram_get_int(strcat_r(prefix2, "act_sim", tmp2));
				if(sim_state == 2 || sim_state == 3){
					sim_lock = 1;

					if(sim_state == 3 || !strcmp(nvram_safe_get("modem_pincode"), "") || nvram_get_int(nvram_auxstate[other_wan_unit]) == WAN_STOPPED_REASON_PINCODE_ERR)
						link_wan[other_wan_unit] = 3;
				}
				else if(sim_state != 1)
					link_wan[other_wan_unit] = 0;

#ifdef RTCONFIG_INTERNAL_GOBI
				if(sim_state > 1 && sim_state <= 6 && !strcmp(nvram_safe_get(strcat_r(prefix2, "act_auth", tmp2)), ""))
					eval("/usr/sbin/modem_status.sh", "simauth");
#endif
			}
		}

		unsetenv("unit");

		nvram_set_int(strcat_r(prefix, "is_usb_modem_ready", tmp), link_wan[other_wan_unit]);
if(test_log)
_dprintf("# wanduck: if_wan_phyconnected: x_Setting=%d, link_modem=%d, sim_state=%d.\n", !isFirstUse, link_wan[other_wan_unit], sim_state);

		link_wan_nvname(other_wan_unit, wired_link_nvram, sizeof(wired_link_nvram));
		if((ptr = nvram_get(wired_link_nvram)) == NULL || strlen(ptr) <= 0 || link_wan[other_wan_unit] != atoi(ptr)){
			nvram_set_int(wired_link_nvram, link_wan[other_wan_unit]);
			if(link_wan[other_wan_unit] != 0)
				record_wan_state_nvram(other_wan_unit, -1, -1, WAN_AUXSTATE_NONE);
		}

		disconn_case[other_wan_unit] = CASE_DYN_MODEM;
	}
#endif	/* RTCONFIG_DUALWAN */
#endif	/* RTCONFIG_DYN_MODEM */

	if(dualwan_unit__usbif(wan_unit)){
		snprintf(prefix, sizeof(prefix), "wan%d_", wan_unit);

		wan_state = nvram_get_int(nvram_state[wan_unit]);
		modem_unit = get_modemunit_by_type(get_dualwan_by_unit(wan_unit));

		if(modem_unit == MODEM_UNIT_NONE){
			_dprintf("%s 2: cannot get the modem unit!\n", __FUNCTION__);
			return conn_state_old[wan_unit];
		}

		usb_modem_prefix(modem_unit, prefix2, sizeof(prefix2));

		snprintf(env_unit, sizeof(env_unit), "unit=%d", modem_unit);
		putenv(env_unit);

		// need to check before detecting SIM. If not, the detect of wanduck will be blocked.
		if(nvram_get_int(strcat_r(prefix2, "act_scanning", tmp2)) != 0){
			_dprintf("wanduck(%d): detect the USB scan.\n", wan_unit);
			link_wan[wan_unit] = 4;
			sim_lock = 1;
			modem_type[0] = '\0';
		}
		else{
			modem_act_reset = nvram_get_int(strcat_r(prefix2, "act_reset", tmp2));
			if(modem_act_reset == 1 || modem_act_reset == 2)
				return CONNED;

			// need to see the link status of modem anyway.
			link_wan[wan_unit] = is_usb_modem_ready(get_dualwan_by_unit(wan_unit));

			if(wan_state == WAN_STATE_CONNECTING)
				return CONNED;

			if(link_wan[wan_unit]){
				snprintf(modem_type, sizeof(modem_type), "%s", nvram_safe_get(strcat_r(prefix2, "act_type", tmp2)));
				if(strlen(modem_type) <= 0){
					eval("/usr/sbin/find_modem_type.sh");
					snprintf(modem_type, sizeof(modem_type), "%s", nvram_safe_get(strcat_r(prefix2, "act_type", tmp2)));
				}
			}
		}

#ifdef RTCONFIG_INTERNAL_GOBI
		if(link_wan[wan_unit] && !strcmp(modem_type, "gobi") && nvram_get_int("usb_gobi"))
			snprintf(usb_if, sizeof(usb_if), "%s", nvram_safe_get(strcat_r(prefix, "ifname", tmp)));
		else
			usb_if[0] = '\0';

		if(strlen(usb_if) > 0){
			if(strlen(nvram_safe_get(strcat_r(prefix2, "act_imei", tmp2))) <= 0)
				eval("/usr/sbin/modem_status.sh", "imei");
			if(got_modem_info < 3 && strlen(nvram_safe_get(strcat_r(prefix2, "act_hwver", tmp2))) <= 0){
				++got_modem_info;
				eval("/usr/sbin/modem_status.sh", "hwver");
			}
			if(strlen(nvram_safe_get(strcat_r(prefix2, "act_swver", tmp2))) <= 0)
				eval("/usr/sbin/modem_status.sh", "swver");
#ifdef RTCONFIG_USB_SMS_MODEM
			if(strlen(nvram_safe_get(strcat_r(prefix2, "act_smsc", tmp2))) <= 0)
				eval("/usr/sbin/modem_status.sh", "smsc");
#endif
		}
#endif // RTCONFIG_INTERNAL_GOBI

		if(!nvram_get(strcat_r(prefix2, "act_sim", tmp2)))
			sim_state = 100; // 100: didn't detect the SIM status yet.
		else
			sim_state = nvram_get_int(strcat_r(prefix2, "act_sim", tmp2));

		if(!strcmp(modem_type, "tty") || !strcmp(modem_type, "mbim") || !strcmp(modem_type, "qmi") || !strcmp(modem_type, "gobi")){
			if(link_wan[wan_unit] == 1){
				if(!strcmp(nvram_safe_get(strcat_r(prefix2, "act_int", tmp2)), "")){
					if(!strcmp(modem_type, "qmi")){	// e.q. Huawei E398.
						_dprintf("wanduck(%d): Sleep 3 seconds to wait modem nodes.\n", wan_unit);
						sleep(3);
					}
#if 0
					else{
						_dprintf("wanduck(%d): Sleep 2 seconds to wait modem nodes.\n", wan_unit);
						sleep(2);
					}
#endif

					wan_state = nvram_get_int(nvram_state[wan_unit]); // after sleep(), wan_state is changed.
				}

#if 0
				if(!strcmp(modem_type, "tty") && !strcmp(nvram_safe_get(strcat_r(prefix2, "act_vid", tmp2)), "6610")){ // e.q. ZTE MF637U
					if(wan_state == WAN_STATE_INITIALIZING && sim_state <= 0)
						eval("/usr/sbin/modem_status.sh", "sim");
				}
				else
#endif
				if(wan_state != WAN_STATE_CONNECTING || sim_state == 100){
					if(!strcmp(modem_type, "gobi"))
						eval("/usr/sbin/modem_status.sh", "sim");
					else if(sim_state == 100 || sim_state == -2) // QMI
						eval("/usr/sbin/modem_status.sh", "sim");
				}

				sim_state = nvram_get_int(strcat_r(prefix2, "act_sim", tmp2));
				if(sim_state == 2 || sim_state == 3){
					sim_lock = 1;

					if(sim_state == 3 || !strcmp(nvram_safe_get("modem_pincode"), "") || nvram_get_int(nvram_auxstate[wan_unit]) == WAN_STOPPED_REASON_PINCODE_ERR)
						link_wan[wan_unit] = 3;
				}
				else if(sim_state != 1)
					link_wan[wan_unit] = 0;

#ifdef RTCONFIG_INTERNAL_GOBI
				if(sim_state > 1 && sim_state <= 6 && !strcmp(nvram_safe_get(strcat_r(prefix2, "act_auth", tmp2)), ""))
					eval("/usr/sbin/modem_status.sh", "simauth");

				if(sim_state == 1){
					eval("/usr/sbin/modem_status.sh", "ip");

					if(modem_pdp != 2){
						snprintf(act_ip, sizeof(act_ip), "%s", nvram_safe_get(strcat_r(prefix2, "act_ip", tmp2)));

						snprintf(buf, sizeof(buf), "%s", nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)));
						if(strcmp(buf, "") && strcmp(buf, "0.0.0.0")
								&& strcmp(act_ip, "") && strcmp(act_ip, "0.0.0.0")
								&& strcmp(buf, act_ip)){
							_dprintf("wanduck: renew IP...(%s)\n", act_ip);
							logmessage("wanduck", "renew IP...(%s)\n", act_ip);
							nvram_set(tmp, "0.0.0.0");
							snprintf(tmp, sizeof(tmp), "/var/run/udhcpc%d.pid", wan_unit);
							kill_pidfile_s(tmp, SIGUSR1);
						}
					}

					if(is_wan_connect(wan_unit))
#if 1 // +CGCELLI seems to cause the Input/Output errors of ttyACM.
						eval("/usr/sbin/modem_status.sh", "fullsignal");
#else
						eval("/usr/sbin/modem_status.sh", "signal");
#endif
#if 0
					// from the newest GUI, the signal don't be shown without SIM.
					else
						eval("/usr/sbin/modem_status.sh", "signal");
#endif
				}
#endif
			}
		}

		unsetenv("unit");

		nvram_set_int(strcat_r(prefix, "is_usb_modem_ready", tmp), link_wan[wan_unit]);
if(test_log)
_dprintf("# wanduck(%d): if_wan_phyconnected: x_Setting=%d, link_modem=%d, sim_state=%d.\n", wan_unit, !isFirstUse, link_wan[wan_unit], sim_state);

		link_wan_nvname(wan_unit, wired_link_nvram, sizeof(wired_link_nvram));
		if((ptr = nvram_get(wired_link_nvram)) == NULL || strlen(ptr) <= 0 || link_wan[wan_unit] != atoi(ptr)){
			nvram_set_int(wired_link_nvram, link_wan[wan_unit]);
			if(link_wan[wan_unit] != 0)
				record_wan_state_nvram(wan_unit, -1, -1, WAN_AUXSTATE_NONE);

			if(link_wan[wan_unit] == 3){
				if(sim_state == 3)
					logmessage("wanduck", "The modem need the PUK code to reset PIN.");
				else
					logmessage("wanduck", "The modem need the PIN code to unlock.");
			}
			else if(link_wan[wan_unit] == 4){
				logmessage("wanduck", "The modem is scanning the stations.");
			}
			//else if(strcmp(modem_type, "ncm"))
			else
				link_changed[wan_unit] = 1;
		}
		else
			link_changed[wan_unit] = 0;

#ifdef RTCONFIG_NOTIFICATION_CENTER
		if(wan_unit == wan_primary_ifunit()
#ifdef RTCONFIG_DUALWAN
				&& strcmp(dualwan_mode, "lb")
#endif
				&& link_wan[wan_unit] == 0)
			sent_unpublic = 0;
#endif

		if(link_wan[wan_unit] == 3)
			return SET_PIN;
		else if(link_wan[wan_unit] == 4)
			return SET_USBSCAN;
	}
	else
#endif // RTCONFIG_USB_MODEM
	{
		snprintf(prefix, sizeof(prefix), "wan%d_", wan_unit);
		snprintf(wan_proto, sizeof(wan_proto), "%s", nvram_safe_get(strcat_r(prefix, "proto", tmp)));

		// check wan port.
		link_wan[wan_unit] = get_wanports_status(wan_unit);

#if defined(RTCONFIG_BONDING_WAN)
#if defined(RTCONFIG_HND_ROUTER_AX_6710) || defined(BCM4912) || defined(BCM6756)
		if(is_router_mode() && nvram_match("bond_wan", "1")){
			char word[64], *next, *ptr;
			int bond_unit = 0, link, speed;

			foreach(word, nvram_safe_get("bond_wan_ifnames"), next) {
				link_wan_nvname(bond_unit, wired_link_nvram, sizeof(wired_link_nvram));
				link = hnd_get_phy_status(word);
				if(nvram_get_int(wired_link_nvram) != link)
					nvram_set_int(wired_link_nvram, link);

				snprintf(wired_link_nvram, sizeof(wired_link_nvram), "bond_wan_ifname%d", bond_unit);
				if(!(ptr = nvram_get(wired_link_nvram)) || strcmp(ptr, word))
					nvram_set(wired_link_nvram, word);

				snprintf(wired_link_nvram, sizeof(wired_link_nvram), "bond_wan_speed%d", bond_unit);
				speed = hnd_get_phy_speed(word);
				if(nvram_get_int(wired_link_nvram) != speed)
					nvram_set_int(wired_link_nvram, speed);

				++bond_unit;
			}

			strlcpy(wired_link_nvram, "link_wan_bond", sizeof(wired_link_nvram));
		}
		else
#endif
#endif
			link_wan_nvname(wan_unit, wired_link_nvram, sizeof(wired_link_nvram));

		if((ptr = nvram_get(wired_link_nvram)) == NULL || strlen(ptr) <= 0 || link_wan[wan_unit] != atoi(ptr)){
			if(link_wan[wan_unit]){
//_dprintf("# wanduck(%d): set %s=%d.\n", wan_unit, wired_link_nvram, CONNED);
				nvram_set_int(wired_link_nvram, CONNED);

				record_wan_state_nvram(wan_unit, -1, -1, WAN_AUXSTATE_NONE);

				if(nvram_get_int(strcat_r(prefix, "dhcpenable_x", tmp)) == 0 && !found_default_route(wan_unit))
					add_multi_routes(0, -1);
			}
			else{
//_dprintf("# wanduck(%d): set %s=%d.\n", wan_unit, wired_link_nvram, DISCONN);
				nvram_set_int(wired_link_nvram, DISCONN);

				record_wan_state_nvram(wan_unit, WAN_STATE_DISCONNECTED, -1, WAN_AUXSTATE_NOPHY);
			}

			if(link_wan[wan_unit] == 1 && get_wan_state(wan_unit) == 2){
				link_changed[wan_unit] = 0;
				link_setup[wan_unit] = 0;
			}
			else
				link_changed[wan_unit] = 1;
		}
		else
			link_changed[wan_unit] = 0;

		// after update_wan_state(), auxstate will be cleaned.
		if(!link_wan[wan_unit]){
			record_wan_state_nvram(wan_unit, -1, -1, WAN_AUXSTATE_NOPHY);

#ifdef RTCONFIG_NOTIFICATION_CENTER
			if(wan_unit == wan_primary_ifunit()
#ifdef RTCONFIG_DUALWAN
					&& strcmp(dualwan_mode, "lb")
#endif
					)
				sent_unpublic = 0;
#endif
		}
	}

#if defined(RTCONFIG_LANWAN_LED) || defined(RTCONFIG_LAN4WAN_LED)
	LanWanLedCtrl();
#endif

#ifdef RTCONFIG_QCA
	sw_led_ctrl();
#endif

#ifdef RTCONFIG_DSL
	if (get_dualwan_by_unit(wan_unit) == WANS_DUALWAN_IF_DSL) {
		if (link_changed[wan_unit] && link_wan[wan_unit]) {
			_dprintf("\n=====\nwan[%d] down->up\n=====\n", wan_unit);
#ifdef RTCONFIG_DSL_BCM
			if (nvram_get_int("dsltmp_config_xtm")) {
				static int first_up = 1;
				nvram_set("dsltmp_config_xtm", "0");
				dsl_wan_config(2);
				if (first_up) {
					handle_wan_line(wan_unit, 0);
					first_up = 0;
				}
			}
			else
#endif
			if (nvram_get_int("dsltmp_syncup_short")
#ifdef RTCONFIG_DUALWAN
			  && (strstr(dualwan_wans, "none") || strcmp(dualwan_mode, "lb")) //not loadbalance
#endif
			) {
				// to skip restart wan if
				_dprintf("set connected\n");
				nvram_set("dsltmp_syncup_short", "0");
				link_setup[wan_unit] = 0;
				record_wan_state_nvram(wan_unit, WAN_STATE_CONNECTED, WAN_STOPPED_REASON_NONE, WAN_AUXSTATE_NONE);
				return CONNED;
			}
		}
	}
#endif

#ifdef RTCONFIG_WIRELESSREPEATER
	// check if set AP.
#ifdef RTCONFIG_REALTEK
	/* [MUST]: Need to disscuss to add new mode for Media Bridge */
	if(repeater_mode() || sw_mode == SW_MODE_HOTSPOT || mediabridge_mode())
#else
	if(sw_mode == SW_MODE_REPEATER || sw_mode == SW_MODE_HOTSPOT)
#endif
	{
		link_ap = (wlc_state == WLC_STATE_CONNECTED);
		if(link_ap != nvram_get_int("link_ap"))
			nvram_set_int("link_ap", link_ap);
	}
#endif

	if(is_router_mode()){
		// this means D2C because of reconnecting the WAN port.
		if(link_changed[wan_unit]){
#ifdef RTCONFIG_USB_MODEM
			if(dualwan_unit__usbif(wan_unit)){
				if(link_wan[wan_unit]){
					if(sim_lock){
						sim_lock = 0;
						record_wan_state_nvram(wan_unit, WAN_STATE_INITIALIZING, WAN_STOPPED_REASON_NONE, WAN_AUXSTATE_NONE);
					}

					_dprintf("wanduck(%d): modem PHY_RECONN.\n", wan_unit);
					return PHY_RECONN;
				}
				else{
					//record_wan_state_nvram(wan_unit, -1, -1, WAN_AUXSTATE_NOPHY);
					record_wan_state_nvram(wan_unit, WAN_STATE_INITIALIZING, WAN_STOPPED_REASON_NONE, WAN_AUXSTATE_NOPHY);
					_dprintf("wanduck(%d): SIM or modem is pulled off.\n", wan_unit);

#if defined(RTCONFIG_NOTIFICATION_CENTER)
					_dprintf("wanduck(%d): NC send SYS_WAN_USB_MODEM_UNREADY_EVENT.\n", wan_unit);
					snprintf(tmp, sizeof(tmp), "0x%x", SYS_WAN_USB_MODEM_UNREADY_EVENT);
					eval("Notify_Event2NC", tmp, "");
#endif
					return DISCONN;
				}
			}
			else
#endif
			// WAN port was disconnected, arm reconnect
			if(!link_setup[wan_unit] && !link_wan[wan_unit]){
				link_setup[wan_unit] = 1;
			}
			// WAN port was connected, fire reconnect if armed
			else if(link_setup[wan_unit]){
				link_setup[wan_unit] = 0;

				if(!strcmp(wan_proto, "static")){
					disconn_case[wan_unit] = CASE_OTHERS;
					_dprintf("wanduck(%d): static PHY_RECONN.\n", wan_unit);
					return PHY_RECONN;
				}
				else if(!strcmp(wan_proto, "dhcp")){
					disconn_case[wan_unit] = CASE_DHCPFAIL;
					_dprintf("wanduck(%d): dhcp PHY_RECONN.\n", wan_unit);
					return PHY_RECONN;
				}
				else{
					// PPPoE, PPTP, L2TP
					disconn_case[wan_unit] = CASE_PPPFAIL;
					_dprintf("wanduck(%d): PPP PHY_RECONN.\n", wan_unit);
					return PHY_RECONN;
				}
			}
		}
		else if(!link_wan[wan_unit]){
			if(wan_unit == wan_primary_ifunit()
#ifdef RTCONFIG_DUALWAN
					&& strcmp(dualwan_mode, "lb")
#endif
					)
				set_link_internet(wan_unit, 1);

			if((nvram_get_int("web_redirect")&WEBREDIRECT_FLAG_NOLINK)){
				disconn_case[wan_unit] = CASE_DISWAN;
			}

#ifdef RTCONFIG_DSL
			if (get_dualwan_by_unit(wan_unit) != WANS_DUALWAN_IF_DSL)
#endif
			max_disconn_count[wan_unit] = max_disconn_count[wan_unit]/2;
			if(max_disconn_count[wan_unit] < 1)
				max_disconn_count[wan_unit] = 1;
			max_wait_time[wan_unit] = scan_interval*max_disconn_count[wan_unit];

			return DISCONN;
		}
	}
#ifdef RTCONFIG_REALTEK
/* [MUST]: Need to disscuss to add new mode for Media Bridge */
	else if(access_point_mode()
#ifdef RTCONFIG_AMAS
			|| re_mode()
#endif
)
#else
	else if(sw_mode == SW_MODE_AP)
#endif
	{
#if 0
		if(!link_wan[wan_unit]){
			// ?: type error?
			nvram_set_int("link_internet", 1);

			record_wan_state_nvram(wan_unit, -1, -1, WAN_AUXSTATE_NOPHY);

			if((nvram_get_int("web_redirect")&WEBREDIRECT_FLAG_NOLINK)){
				disconn_case[wan_unit] = CASE_DISWAN;
				return DISCONN;
			}
		}
#else
		set_link_internet(wan_unit, 2);

#ifdef RTCONFIG_DHCP_OVERRIDE
		if (nvram_match("dnsqmode", "2")) {
			disconn_case[wan_unit] = CASE_DISWAN;
			return DISCONN;
		}
		else
#endif

		return CONNED;
#endif
	}
#ifdef RTCONFIG_WIRELESSREPEATER
	else{ // sw_mode == SW_MODE_REPEATER, SW_MODE_MediaBridge...etc
		if(!link_ap){
			set_link_internet(wan_unit, 1);

			if(sw_mode == SW_MODE_HOTSPOT)
				record_wan_state_nvram(wan_unit, -1, -1, WAN_AUXSTATE_NOPHY);

			disconn_case[wan_unit] = CASE_AP_DISCONN;
			return DISCONN;
		}
		else
		{
			if(nvram_match("lan_proto", "dhcp") && nvram_get_int("lan_state_t") != LAN_STATE_CONNECTED){
				set_link_internet(wan_unit, 1);
				return DISCONN;
			}
			else{
				set_link_internet(wan_unit, 2);
				return CONNED;
			}
		}
	}
#endif

	return CONNED;
}

#ifdef RTCONFIG_NOTIFICATION_CENTER
void check_unpublic_event(const int wan_unit, const int link_internet){
	int needed = 0;
	char tmp[100];
#ifdef RTCONFIG_GETREALIP
	char prefix[16];
	char wanip[32], realip[32];

	if(wan_unit != wan_primary_ifunit()
#ifdef RTCONFIG_DUALWAN
			|| !strcmp(dualwan_mode, "lb")
#endif
			)
		return;

	snprintf(prefix, sizeof(prefix), "wan%d_", wan_unit);
#endif

	if(link_internet == CONNED && !sent_unpublic){
		if(dualwan_unit__usbif(wan_unit))
			needed = 1;
#ifdef RTCONFIG_GETREALIP
		else if(nvram_get_int(strcat_r(prefix, "realip_state", tmp)) == 2){
			snprintf(wanip, sizeof(wanip), "%s", nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)));
			snprintf(realip, sizeof(realip), "%s", nvram_safe_get(strcat_r(prefix, "realip_ip", tmp)));

			if(strcmp(wanip, realip))
				needed = 1;
		}
#endif

		if(needed && !sent_unpublic){
			_dprintf("wanduck(%d): NC send SYS_WAN_UNPUBLIC_IP_EVENT.\n", wan_unit);
			snprintf(tmp, sizeof(tmp), "0x%x", SYS_WAN_UNPUBLIC_IP_EVENT);
			eval("Notify_Event2NC", tmp, "");

			sent_unpublic = 1;
		}
	}
}
#endif

int if_wan_connected(int wan_unit){
	int link_internet;

	if(chk_proto(wan_unit) != CONNED) {
#ifdef RTCONFIG_WIFI_SON
		if (((nvram_get_int("link_internet") == 2) && !isFirstUse) && nvram_match("wifison_ready", "1")) {
			logmessage("WAN Connection", "WAN stopped...");
			set_link_internet(wan_unit, 1);
		}
#endif
		return DISCONN;
	}
	else if(wan_unit == wan_primary_ifunit()
#ifdef RTCONFIG_DUALWAN
			|| !strcmp(dualwan_mode, "lb")
#endif
			){
		link_internet = detect_internet(wan_unit);

#ifdef RTCONFIG_NOTIFICATION_CENTER
		check_unpublic_event(wan_unit, link_internet);
#endif

		return link_internet;
	}

#ifdef RTCONFIG_NOTIFICATION_CENTER
	check_unpublic_event(wan_unit, CONNED);
#endif

	return CONNED;
}

void handle_wan_line(int wan_unit, int action){
	char cmd[32];
	char prefix_wan[8], nvram_name[16], wan_proto[16];

	// Redirect rules.
	if(action){
//_dprintf("nat_rule: stop_nat_rules 3.\n");
		nat_state = stop_nat_rules();
	}
	/*
	 * When C2C and remove the redirect rules,
	 * it means dissolve the default state.
	 */
	else if(conn_changed_state[wan_unit] == D2C || conn_changed_state[wan_unit] == CONNED){
#if defined(RTCONFIG_LANWAN_LED) || defined(RTCONFIG_WANRED_LED) || defined(RTCONFIG_FAILOVER_LED)
		update_wan_leds(wan_unit, link_wan[wan_unit]);
#endif
		// When DUT got IP during the detect interval, the DNS setting might be missed by no PHY.
		update_resolvconf();

_dprintf("nat_rule: start_nat_rules 3.\n");
		nat_state = start_nat_rules();

		snprintf(prefix_wan, sizeof(prefix_wan), "wan%d_", wan_unit);

		snprintf(wan_proto, sizeof(wan_proto), "%s", nvram_safe_get(strcat_r(prefix_wan, "proto", nvram_name)));

		if(!strcmp(wan_proto, "static")){
			/* Sync time */
			refresh_ntpc();
		}

#if defined(RTCONFIG_APP_PREINSTALLED) || defined(RTCONFIG_APP_NETINSTALLED)
		if(check_if_dir_exist("/opt/lib/ipkg")){
			_dprintf("wanduck: update the APP's lists...\n");
			notify_rc("start_apps_update");
		}
#endif
	}
	else{ // conn_changed_state[wan_unit] == PHY_RECONN
		nat_state = stop_nat_rules();

#ifdef RT4GAC86U
		if(get_dualwan_by_unit(wan_unit) == WANS_DUALWAN_IF_USB)
			system("/usr/sbin/modem_status.sh init_sms");
#endif

		_dprintf("\n# wanduck(%d): Try to restart_wan_if.\n", wan_unit);
		snprintf(cmd, sizeof(cmd), "restart_wan_if %d", wan_unit);
		notify_rc_and_wait(cmd);
#ifdef RTCONFIG_MULTICAST_IPTV
		if (nvram_get_int("switch_stb_x") > 6) {
			int unit;
			for (unit = WAN_UNIT_IPTV; unit < WAN_UNIT_MULTICAST_IPTV_MAX; unit++) {
				snprintf(cmd, sizeof(cmd), "restart_wan_if %d", unit);
				notify_rc_and_wait(cmd);
			}
		}
#endif
	}
}

void close_socket(int sockfd, char type){
	close(sockfd);
	FD_CLR(sockfd, &allset);
	client[fd_i].sfd = -1;
	client[fd_i].type = 0;
}

int build_socket(char *http_port, char *dns_port, int *hd, int *dd){
	if((*hd = passivesock(http_port, IPPROTO_TCP, 10)) < 0){
		_dprintf("Fail to socket for httpd port: %s.\n", http_port);
		return -1;
	}

	if((*dd = passivesock(dns_port, IPPROTO_UDP, 10)) < 0){
		_dprintf("Fail to socket for DNS port: %s.\n", dns_port);
		return -1;
	}

	return 0;
}

const char *wl_device_handler[] = {
		"www.apple.com/library/test/success.html",	//macOS 10.9 and iOS 7 and older
		"captive.apple.com/hotspot-detect.html", //macOS 10.10 and iOS 8 and newer
		"connectivitycheck.gstatic.com/generate_204",	//Android 5 and newer, Google Chrome browser
		"play.googleapis.com/generate_204",	//Android 7 and newer
		"clients3.google.com/generate_204",	//Android 4 and older
		"www.google.com/gen_204",	//Android
		//"fedoraproject.org/static/hotspot.txt",	//Fedora Linux
		//"network-test.debian.org/nm",	//Debian Linux
		//"www.archlinux.org/check_network_status.txt",	//Arch Linux
		//"nmcheck.gnome.org/check_network_status.txt",	//GNOME Desktop
		//"detectportal.firefox.com/success.txt",	//Firefox browser
		//"spectrum.s3.amazonaws.com/kindle-wifi/wifistub.html",	//Kindle
		NULL
	};

void send_page(int wan_unit, int sfd, char *file_dest, char *url){
	char buf[2*MAXLINE];
	time_t now;
	char timebuf[100];
	char dut_addr[64];
	char dut_proto[16];
	char dut_port[5];
	char redirection[100];
	char indexpage[128];
	int i=0, wl_url_hit=0;

	memset(buf, 0, sizeof(buf));
	now = time(NULL);
	strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&now));

#ifdef NO_IOS_DETECT_INTERNET
	if(nvram_match("disiosdet", "1")){
		if (!strcmp(url, "www.msftncsi.com/ncsi.txt")){	//Windows 8.1 and older
			snprintf(buf, sizeof(buf), "%s%s%s", "HTTP/1.1 200 OK\r\nContent-Length: 14\r\nDate: ", timebuf, "\r\nConnection: keep-alive\r\nContent-Type: text/plain\r\nCache-Control: max-age=30, must-revalidate\r\n\r\nMicrosoft NCSI");
			wl_url_hit = 1;
		}else if (!strcmp(url, "www.msftconnecttest.com/connecttest.txt")){	//Windows 10 and Xbox
			snprintf(buf, sizeof(buf), "%s%s%s", "HTTP/1.1 200 OK\r\nContent-Length: 22\r\nContent-Type: text/plain\r\nContent-MD5: BMP8SohYjuR9M9BmkgrEEA==\r\nLast-Modified: Fri, 04 Mar 2016 06:55:03 GMT\r\nETag: \"0x8D343F9E96C9DAC\"\r\nServer: Microsoft-IIS/7.5\r\nx-ms-request-id: 18b95cc2-001e-0049-0f92-b2bac4000000\r\nx-ms-version: 2009-09-19\r\nx-ms-meta-CbModifiedTime: Tue, 01 Mar 2016 21:41:22 GMT\r\nx-ms-lease-status: unlocked\r\nx-ms-blob-type: BlockBlob\r\nX-ECN-P: RD0003FF835356\r\nAccess-Control-Expose-Headers: X-MSEdge-Ref\r\nX-CID: 7\r\nDate: ", timebuf, "\r\n\r\nMicrosoft Connect Test");
			wl_url_hit = 1;
		}else{
			for(i=0; wl_device_handler[i]; i++){
				if (!strcmp(wl_device_handler[i], url)){
					wl_url_hit = 1;
					break;
				}
			}
		}
	}
#endif

	// avoid redirect loops
	if(nvram_get_int("http_enable") == 2){
		if(strstr(url, "generate_204")){
			snprintf(buf, sizeof(buf), "%s", "HTTP/1.1 204\r\n");
			wl_url_hit = 1;
		}
	}

	if(!wl_url_hit){
	snprintf(buf, sizeof(buf), "%s%s%s%s%s", "HTTP/1.0 302 Moved Temporarily\r\n", "Server: wanduck\r\n", "Date: ", timebuf, "\r\n");
#ifdef RTCONFIG_WIRELESSREPEATER
	if(sw_mode == SW_MODE_REPEATER || sw_mode == SW_MODE_HOTSPOT)
		snprintf(dut_addr, sizeof(dut_addr), "%s", DUT_DOMAIN_NAME);
	else
#endif

	snprintf(dut_addr, sizeof(dut_addr), "%s", DUT_DOMAIN_NAME);

#ifdef RTCONFIG_HTTPS
	if (nvram_get_int("http_enable") == 1) {
		snprintf(dut_proto, sizeof(dut_proto), "https://");
		snprintf(dut_port, sizeof(dut_port), "%d", nvram_get_int("https_lanport") ? : 443);
	} else
#endif
	{
		snprintf(dut_proto, sizeof(dut_proto), "http://");
		snprintf(dut_port, sizeof(dut_port), "%d", nvram_get_int("http_lanport") ? : 80);
	}

	if(strstr(url, "hotspot-detect") || strstr(url, "generate_204")){
		snprintf(redirection, sizeof(redirection), "%s%s%s", "Location:", dut_proto, dut_addr);
	}
	else{
		//snprintf(redirection, sizeof(redirection), "%s%s%s%s%s", "refresh:1.0001; url=", dut_proto, dut_addr, ":", dut_port);
		snprintf(redirection, sizeof(redirection), "%s%s%s%s%s", "Location:", dut_proto, dut_addr, ":", dut_port);
	}

	// TODO: Only send pages for the wan(0)'s state.
	if(isFirstUse){
#ifdef RTCONFIG_WIRELESSREPEATER
		if(sw_mode == SW_MODE_REPEATER || sw_mode == SW_MODE_HOTSPOT)
			snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "%s%s%s%s", "Connection: close\r\n", redirection, "/QIS_default.cgi?flag=sitesurvey", "\r\nContent-Type: text/html\r\n");
		else
#endif
#ifdef RTCONFIG_TMOBILE
			snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "%s%s%s%s", "Connection: close\r\n", redirection, "/MobileQIS_Login.asp", "\r\nContent-Type: text/html\r\n");
#else
			snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "%s%s%s%s", "Connection: close\r\n", redirection, "/QIS_default.cgi?flag=welcome", "\r\nContent-Type: text/html\r\n");
#endif
	}
	else if(conn_changed_state[wan_unit] == C2D || conn_changed_state[wan_unit] == DISCONN){
#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2) || defined(RTCONFIG_UBIFS)
		if(disconn_case[wan_unit] == CASE_DATALIMIT)
			snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "%s%s%s%d%s", "Connection: close\r\n", redirection, "/blocking.asp?flag=", disconn_case[wan_unit], "\r\nContent-Type: text/html\r\n");
		else
#endif
#ifdef RTCONFIG_USB_MODEM
#ifdef RTCONFIG_DYN_MODEM
		if(wan_unit == WAN_UNIT_FIRST && dualwan_unit__nonusbif(wan_unit) && get_dualwan_by_unit(other_wan_unit) == WANS_DUALWAN_IF_NONE && link_wan[other_wan_unit])
			snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "%s%s%s%d%s", "Connection: close\r\n", redirection, "/error_page.htm?flag=", disconn_case[other_wan_unit], "\r\nContent-Type: text/html\r\n");
		else
#endif
#endif
		if(disconn_case[wan_unit] == CASE_THESAMESUBNET)
			snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "%s%s%s%d%s", "Connection: close\r\n", redirection, "/error_page.htm?flag=", disconn_case[wan_unit], "\r\nContent-Type: text/html\r\n");
		else
			snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "%s%s%s%d%s", "Connection: close\r\n", redirection, "/error_page.htm?flag=", disconn_case[wan_unit], "\r\nContent-Type: text/html\r\n");
	}
#ifdef RTCONFIG_WIRELESSREPEATER
	else{
		get_index_page(indexpage, sizeof(indexpage));
		snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "%s%s%s/%s", "Connection: close\r\n", redirection, indexpage, "\r\nContent-Type: text/html\r\n");
	}
#endif

	}

	write(sfd, buf, strlen(buf));
	close_socket(sfd, T_HTTP);
}

void parse_dst_url(char *page_src){
	int i, j;
	char dest[PATHLEN], host[64];
	char *hp;

	j = 0;
	memset(dest, 0, sizeof(dest));
	memset(host, 0, sizeof(host));

	for(i = 0; i < strlen(page_src); ++i){
		if(i >= PATHLEN)
			break;

		if(page_src[i] == ' ' || page_src[i] == '?'){
			dest[j] = '\0';
			break;
		}

		dest[j++] = page_src[i];
	}

	if((hp = strstr(page_src, "Host:")) != NULL){
		hp += 6;
		j = 0;
		for(i = 0; i < strlen(hp); ++i){
			if(i >= 64)
				break;

			if(hp[i] == '\r' || hp[i] == '\n'){
				host[j] = '\0';
				break;
			}

			host[j++] = hp[i];
		}
	}

	snprintf(dst_url, sizeof(dst_url), "%s/%s", host, dest);
}

void handle_http_req(int sfd, char *line){
	int len;

	if(!strncmp(line, "GET /", 5)){
		parse_dst_url(line+5);

		len = strlen(dst_url);
		if((dst_url[len-4] == '.') &&
				(dst_url[len-3] == 'i') &&
				(dst_url[len-2] == 'c') &&
				(dst_url[len-1] == 'o')){
			close_socket(sfd, T_HTTP);

			return;
		}

		send_page(current_wan_unit, sfd, NULL, dst_url);
	}
	else
		close_socket(sfd, T_HTTP);
}

void handle_dns_req(int sfd, unsigned char *request, int maxlen, struct sockaddr *pcliaddr, int clen){
#if !defined(RTCONFIG_FINDASUS)
	const static unsigned char auth_name_srv[] = {
		0x00, 0x00, 0x06, 0x00, 0x01, 0x00,
		0x00, 0x2a, 0x30, 0x00, 0x40, 0x01, 0x61, 0x0c,
		0x72, 0x6f, 0x6f, 0x74, 0x2d, 0x73, 0x65, 0x72,
		0x76, 0x65, 0x72, 0x73, 0x03, 0x6e, 0x65, 0x74,
		0x00, 0x05, 0x6e, 0x73, 0x74, 0x6c, 0x64, 0x0c,
		0x76, 0x65, 0x72, 0x69, 0x73, 0x69, 0x67, 0x6e,
		0x2d, 0x67, 0x72, 0x73, 0x03, 0x63, 0x6f, 0x6d,
		0x00, 0x78, 0x1b, 0x1a, 0xfd, 0x00, 0x00, 0x07,
		0x08, 0x00, 0x00, 0x03, 0x84, 0x00, 0x09, 0x3a,
		0x80, 0x00, 0x01, 0x51, 0x80
	};
#endif
	unsigned char reply_content[MAXLINE], *ptr, *end;
	char *nptr, *nend;
	dns_header *d_req, *d_reply;
	dns_queries queries;
	dns_answer answer;
	uint16_t opcode;
	int is_answer_ready = 0;

	/* validation */
	d_req = (dns_header *)request;
	if (maxlen <= sizeof(dns_header) ||			/* incomplete header */
	    d_req->flag_set.flag_num & htons(0x8000) ||		/* not query */
	    d_req->questions == 0)				/* no questions */
		return;
	opcode = d_req->flag_set.flag_num & htons(0x7800);
	ptr = request + sizeof(dns_header);
	end = request + maxlen;

	/* query, only first so far */
	memset(&queries, 0, sizeof(queries));
	nptr = queries.name;
	nend = queries.name + sizeof(queries.name) - 1;
	while (ptr < end) {
		size_t len = *ptr++;
		if (len > 63 || end - ptr < (len ? : 4))
			return;
		if (len == 0) {
			memcpy(&queries.type, ptr, 2);
			memcpy(&queries.ip_class, ptr + 2, 2);
			ptr += 4;
			break;
		}
		if (nptr < nend && *queries.name)
			*nptr++ = '.';
		if (nptr < nend)
			nptr = stpncpy(nptr, (char *)ptr, min(len, nend - nptr));
		ptr += len;
	}
	if (queries.type == 0 || queries.ip_class == 0 || strlen(queries.name) > 1025)
		return;
	maxlen = ptr - request;

	/* reply */
	if (maxlen > sizeof(reply_content))
		return;
	ptr = memcpy(reply_content, request, maxlen) + maxlen;
	end = reply_content + sizeof(reply_content);

	/* header */
	d_reply = (dns_header *)reply_content;
	d_reply->flag_set.flag_num = htons(0x8180);
	d_reply->questions = htons(1);
	d_reply->answer_rrs = htons(0);
	d_reply->auth_rrs = htons(0);
	d_reply->additional_rss = htons(0);

	/* answer */
	memset(&answer, 0, sizeof(answer));
	answer.name = htons(0xc00c);
	answer.type = queries.type;
	answer.ip_class = queries.ip_class;
	answer.ttl = htonl(0);
	answer.data_len = htons(4);

	if (opcode != 0) {
		/* not implemented, non-Query op */
		d_reply->flag_set.flag_num = htons(0x8184) | opcode;
	} else if (queries.ip_class == htons(1) && queries.type == htons(1)) {
		/* class IN type A */
		if (strcasecmp(queries.name, router_name) == 0
#ifdef RTCONFIG_FINDASUS
		 || strcasecmp(queries.name, "findasus.local") == 0
#endif
		) {
			/* no error, authoritative */
			d_reply->flag_set.flag_num = htons(0x8580);
			d_reply->answer_rrs = htons(1);
			if (client_mode() &&
			    nvram_match("lan_proto", "dhcp") && nvram_get_int("lan_state_t") != LAN_STATE_CONNECTED)
				answer.addr = inet_addr_(nvram_default_get("lan_ipaddr"));
			else
				answer.addr = inet_addr_(nvram_safe_get("lan_ipaddr"));
#if !defined(RTCONFIG_FINDASUS)
		} else if (strcasecmp(queries.name, "findasus.local") == 0) {
			/* non existent domain */
			d_reply->flag_set.flag_num = htons(0x8183);
			d_reply->auth_rrs = htons(1);
#endif
#ifdef RTCONFIG_DSL
		} else if (strcasecmp(queries.name, "ntp01.mvp.tivibu.com.tr") == 0) {
			d_reply->answer_rrs = htons(1);
			answer.addr = htonl(0xc0a87946);
		} else if (strcasecmp(queries.name, "ntp02.mvp.tivibu.com.tr") == 0) {
			d_reply->answer_rrs = htons(1);
			answer.addr = htonl(0xc0a87947);
#endif
#ifdef RTCONFIG_ISP_OPTUS
		} else if (op_is_whitelist_url(queries.name)) { // Optus comstomization
			struct addrinfo *res;
			struct addrinfo *p;
			//char command[255];
			if (op_resolve(queries.name, &res) == 0) {
				/* no error */
				int rrs = 0;
				for(p = res; p != NULL; p = p->ai_next) {
					if (p->ai_family == AF_INET) {
						//snprintf(command, sizeof(command), IPTABLES_CMD, inet_ntoa(((struct sockaddr_in*)(p->ai_addr))->sin_addr));
						answer.addr = ((struct sockaddr_in*)(p->ai_addr))->sin_addr.s_addr;
						//system(command);
						if (end - ptr < sizeof(answer))
							break;
						ptr = memcpy(ptr, &answer, sizeof(answer)) + sizeof(answer);
						rrs++;
					}
				}
				d_reply->answer_rrs = htons(rrs);
				is_answer_ready = 1;
				op_check_and_add_rules((void *)res);
				freeaddrinfo(res);
			} else {
				/* non existent domain */
				d_reply->flag_set.flag_num = htons(0x8183);
				d_reply->auth_rrs = htons(1);
			}
#endif /* RTCONFIG_ISP_OPTUS */
		} else if (*queries.name) {
			/* no error */
			d_reply->answer_rrs = htons(1);
			answer.addr = htonl(0x0a000001);	// 10.0.0.1
		}
	} else {
		/* not implemented */
		d_reply->flag_set.flag_num = htons(0x8184);
	}

	if (!is_answer_ready && d_reply->answer_rrs) {
		if (end - ptr < sizeof(answer))
			return;
		ptr = memcpy(ptr, &answer, sizeof(answer)) + sizeof(answer);
	}
#if !defined(RTCONFIG_FINDASUS)
	if (d_reply->auth_rrs) {
		if (end - ptr < sizeof(auth_name_srv))
			return;
		ptr = memcpy(ptr, auth_name_srv, sizeof(auth_name_srv)) + sizeof(auth_name_srv);
	}
#endif

	sendto(sfd, reply_content, ptr - reply_content, 0, pcliaddr, clen);
}

void run_http_serv(int sockfd){
	ssize_t n;
	char line[MAXLINE];

	memset(line, 0, sizeof(line));

	if((n = read(sockfd, line, MAXLINE)) == 0){	// client close
		close_socket(sockfd, T_HTTP);

		return;
	}
	else if(n < 0){
		perror("wanduck serv http");
		return;
	}
	else{
		if(client[fd_i].type == T_HTTP)
			handle_http_req(sockfd, line);
		else
			close_socket(sockfd, T_HTTP);
	}
}

void run_dns_serv(int sockfd){
	unsigned char line[MAXLINE];
	struct sockaddr_in cliaddr;
	int n, clilen = sizeof(cliaddr);

	if((n = recvfrom(sockfd, line, MAXLINE, 0, (struct sockaddr *)&cliaddr, (socklen_t *)&clilen)) == 0)	// client close
		return;
	else if(n < 0){
		perror("wanduck serv dns");
		return;
	}
	else
		handle_dns_req(sockfd, line, n, (struct sockaddr *)&cliaddr, clilen);
}

void record_wan_state_nvram(int wan_unit, int state, int sbstate, int auxstate){
	if(state != -1 && state != nvram_get_int(nvram_state[wan_unit]))
		nvram_set_int(nvram_state[wan_unit], state);

	if(sbstate != -1 && sbstate != nvram_get_int(nvram_sbstate[wan_unit]))
		nvram_set_int(nvram_sbstate[wan_unit], sbstate);

	if(auxstate != -1 && auxstate != nvram_get_int(nvram_auxstate[wan_unit]))
		nvram_set_int(nvram_auxstate[wan_unit], auxstate);
}

void record_conn_status(int wan_unit){
	char prefix_wan[8], nvram_name[16], wan_proto[16];
	char log_title[32];
#if defined(RTCONFIG_NOTIFICATION_CENTER)
	char buff[32];
#endif

#ifdef RTCONFIG_DUALWAN
	if(!strcmp(dualwan_mode, "lb") || !strcmp(dualwan_mode, "fb"))
		snprintf(log_title, sizeof(log_title), "WAN(%d) Connection", wan_unit);
	else
#endif
		snprintf(log_title, sizeof(log_title), "WAN Connection");

	snprintf(prefix_wan, sizeof(prefix_wan), "wan%d_", wan_unit);

	snprintf(wan_proto, sizeof(wan_proto), "%s", nvram_safe_get(strcat_r(prefix_wan, "proto", nvram_name)));

	if(conn_changed_state[wan_unit] == DISCONN || conn_changed_state[wan_unit] == C2D){
#ifdef RTCONFIG_WIRELESSREPEATER
		if(disconn_case[wan_unit] == CASE_AP_DISCONN){
			if(disconn_case_old[wan_unit] == CASE_AP_DISCONN)
				return;
			disconn_case_old[wan_unit] = CASE_AP_DISCONN;

			logmessage(log_title, "Don't connect the AP yet.");

#if defined(RTCONFIG_NOTIFICATION_CENTER)
			_dprintf("wanduck(%d): NC send SYS_WAN_UNABLE_CONNECT_PARENT_AP_EVENT.\n", wan_unit);
			snprintf(buff, sizeof(buff), "0x%x", SYS_WAN_UNABLE_CONNECT_PARENT_AP_EVENT);
			eval("Notify_Event2NC", buff, "");
#endif
		}
		else
#endif
		if(disconn_case[wan_unit] == CASE_DISWAN){
			if(disconn_case_old[wan_unit] == CASE_DISWAN)
				return;
			disconn_case_old[wan_unit] = CASE_DISWAN;

#if defined(RTCONFIG_AMAS) && defined(RTCONFIG_PRELINK)
			if (isFirstUse) {
				nvram_set("amas_bdl_wanstate", "0");
#if defined(RTCONFIG_BT_CONN)
				ble_rename_ssid();
#endif
			}
#endif

			logmessage(log_title, "WAN(%d) link down.", wan_unit);

#ifdef RTCONFIG_SOFTWIRE46
			if (!strncmp(nvram_safe_get("territory_code"), "JP", 2))
				stop_auto46det();
#endif
#if defined(RTCONFIG_NOTIFICATION_CENTER)
			_dprintf("wanduck(%d): NC send SYS_WAN_CABLE_UNPLUGGED_EVENT.\n", wan_unit);
			snprintf(buff, sizeof(buff), "0x%x", SYS_WAN_CABLE_UNPLUGGED_EVENT);
			eval("Notify_Event2NC", buff, "");
#endif
		}
		else if(disconn_case[wan_unit] == CASE_PPPFAIL){
			if(disconn_case_old[wan_unit] == CASE_PPPFAIL)
				return;
			disconn_case_old[wan_unit] = CASE_PPPFAIL;

			if(ppp_fail_state == WAN_STOPPED_REASON_PPP_AUTH_FAIL){
				logmessage(log_title, "Failed to authenticate ourselves to peer.");

#if defined(RTCONFIG_NOTIFICATION_CENTER)
				_dprintf("wanduck(%d): NC send SYS_WAN_PPPOE_AUTH_FAILURE_EVENT.\n", wan_unit);
				snprintf(buff, sizeof(buff), "0x%x", SYS_WAN_PPPOE_AUTH_FAILURE_EVENT);
				eval("Notify_Event2NC", buff, "");
#endif
			}
			else if(ppp_fail_state == WAN_STOPPED_REASON_PPP_LACK_ACTIVITY){
				logmessage(log_title, "Terminating connection due to lack of activity.");

#if defined(RTCONFIG_NOTIFICATION_CENTER)
				_dprintf("wanduck(%d): NC send SYS_WAN_GOT_PROBLEMS_FROM_ISP_EVENT 1.\n", wan_unit);
				snprintf(buff, sizeof(buff), "0x%x", SYS_WAN_GOT_PROBLEMS_FROM_ISP_EVENT);
				eval("Notify_Event2NC", buff, "");
#endif
			}
			else if(ppp_fail_state == WAN_STOPPED_REASON_PPP_NO_RESPONSE){
				logmessage(log_title, "No response from ISP.");

#if defined(RTCONFIG_NOTIFICATION_CENTER)
				_dprintf("wanduck(%d): NC send SYS_WAN_GOT_PROBLEMS_FROM_ISP_EVENT 2.\n", wan_unit);
				snprintf(buff, sizeof(buff), "0x%x", SYS_WAN_GOT_PROBLEMS_FROM_ISP_EVENT);
				eval("Notify_Event2NC", buff, "");
#endif
			}
			else{
				logmessage(log_title, "Fail to connect with some issues.");

#if defined(RTCONFIG_NOTIFICATION_CENTER)
				_dprintf("wanduck(%d): NC send SYS_WAN_DISCONN_EVENT.\n", wan_unit);
				snprintf(buff, sizeof(buff), "0x%x", SYS_WAN_DISCONN_EVENT);
				eval("Notify_Event2NC", buff, "");
#endif
			}
#if defined(RTCONFIG_AMAS) && defined(RTCONFIG_PRELINK)
			if (isFirstUse) {
				nvram_set("amas_bdl_wanstate", "2");
#if defined(RTCONFIG_BT_CONN)
				ble_rename_ssid();
#endif
			}
#endif
		}
		else if(disconn_case[wan_unit] == CASE_DHCPFAIL){
			if(disconn_case_old[wan_unit] == CASE_DHCPFAIL)
				return;
			disconn_case_old[wan_unit] = CASE_DHCPFAIL;

#if 0
			if(!strcmp(wan_proto, "dhcp")
#ifdef RTCONFIG_WIRELESSREPEATER
					|| sw_mode == SW_MODE_HOTSPOT
#endif
					){
				logmessage(log_title, "ISP's DHCP did not function properly.");

#if defined(RTCONFIG_NOTIFICATION_CENTER)
				_dprintf("wanduck(%d): NC send SYS_WAN_MODEM_OFFLINE_EVENT.\n", wan_unit);
				snprintf(buff, sizeof(buff), "0x%x", SYS_WAN_MODEM_OFFLINE_EVENT);
				eval("Notify_Event2NC", buff, "");
#endif
			}
#endif

#if defined(RTCONFIG_AMAS) && defined(RTCONFIG_PRELINK)
			if (isFirstUse) {
				nvram_set("amas_bdl_wanstate", "2");
#if defined(RTCONFIG_BT_CONN)
				ble_rename_ssid();
#endif
			}
#endif
		}
		else if(disconn_case[wan_unit] == CASE_MISROUTE){
			if(disconn_case_old[wan_unit] == CASE_MISROUTE)
				return;
			disconn_case_old[wan_unit] = CASE_MISROUTE;

			logmessage(log_title, "The router's ip was the same as gateway's ip. It led to your packages couldn't dispatch to internet correctly.");
		}
		else if(disconn_case[wan_unit] == CASE_THESAMESUBNET){
			char *str = " ";
#if defined(RTCONFIG_PORT_BASED_VLAN) || defined(RTCONFIG_TAGGED_BASED_VLAN)
			str = " or VLAN's subnet ";
#endif
			if(disconn_case_old[wan_unit] == CASE_THESAMESUBNET)
				return;
			disconn_case_old[wan_unit] = CASE_THESAMESUBNET;

			logmessage(log_title, "The LAN's subnet%smay be the same with the WAN's subnet.", str);

#if defined(RTCONFIG_NOTIFICATION_CENTER)
			_dprintf("wanduck(%d): NC send SYS_WAN_IP_CONFLICT_EVENT.\n", wan_unit);
			snprintf(buff, sizeof(buff), "0x%x", SYS_WAN_IP_CONFLICT_EVENT);
			eval("Notify_Event2NC", buff, "");
#endif

#ifdef RTCONFIG_TMOBILE
			logmessage(log_title, "reset LAN subnet and dhcp pool.");
			if(!strncmp(nvram_safe_get("lan_ipaddr"), "192.168.29.", 11))
			{
				nvram_set("lan_ipaddr"   , "192.168.24.1");
				nvram_set("lan_ipaddr_rt", "192.168.24.1");
				nvram_set("dhcp_start"   , "192.168.24.100");
				nvram_set("dhcp_end"     , "192.168.24.254");
			} else {
				nvram_set("lan_ipaddr"   , "192.168.29.1");
				nvram_set("lan_ipaddr_rt", "192.168.29.1");
				nvram_set("dhcp_start"   , "192.168.29.100");
				nvram_set("dhcp_end"     , "192.168.29.254");
			}
			nvram_commit();
			reboot(RB_AUTOBOOT);
#endif
#if defined(RTCONFIG_AMAS) && defined(RTCONFIG_PRELINK)
			if (isFirstUse) {
				nvram_set("amas_bdl_wanstate", "2");
#if defined(RTCONFIG_BT_CONN)
				ble_rename_ssid();
#endif
			}
#endif
		}
#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2) || defined(RTCONFIG_UBIFS)
		else if(disconn_case[wan_unit] == CASE_DATALIMIT){
			if(disconn_case_old[wan_unit] == CASE_DATALIMIT)
				return;
			disconn_case_old[wan_unit] = CASE_DATALIMIT;

			logmessage(log_title, "The Data is at limit.");
		}
#endif
		else{	// disconn_case[wan_unit] == CASE_OTHERS
			if(disconn_case_old[wan_unit] == CASE_OTHERS)
				return;
			disconn_case_old[wan_unit] = CASE_OTHERS;

			logmessage(log_title, "WAN was exceptionally disconnected.");

#if defined(RTCONFIG_NOTIFICATION_CENTER)
			if(get_wan_state(wan_unit) == WAN_STATE_DISABLED){
				_dprintf("wanduck(%d): NC send SYS_WAN_BLOCK_EVENT.\n", wan_unit);
				snprintf(buff, sizeof(buff), "0x%x", SYS_WAN_BLOCK_EVENT);
				eval("Notify_Event2NC", buff, "");
			}
#endif
#if defined(RTCONFIG_AMAS) && defined(RTCONFIG_PRELINK)
			if (isFirstUse) {
				nvram_set("amas_bdl_wanstate", "2");
#if defined(RTCONFIG_BT_CONN)
				ble_rename_ssid();
#endif
			}
#endif
		}
	}
	else if(conn_changed_state[wan_unit] == D2C){
		if(disconn_case_old[wan_unit] == -1)
			return;
		disconn_case_old[wan_unit] = -1;

#if defined(RTCONFIG_AMAS) && defined(RTCONFIG_PRELINK)
		if (isFirstUse) {
			nvram_set("amas_bdl_wanstate", "2");
#if defined(RTCONFIG_BT_CONN)
			ble_rename_ssid();
#endif
		}
#endif

		logmessage(log_title, "WAN was restored.");
	}
	else if(conn_changed_state[wan_unit] == PHY_RECONN){

#if defined(RTCONFIG_AMAS) && defined(RTCONFIG_PRELINK)
		if (isFirstUse) {
			nvram_set("amas_bdl_wanstate", "2");
#if defined(RTCONFIG_BT_CONN)
			ble_rename_ssid();
#endif
		}
#endif

		logmessage(log_title, "WAN(%d) link up.", wan_unit);
	}
}

int get_disconn_count(int wan_unit){
	return changed_count[wan_unit];
}

void set_disconn_count(int wan_unit, int flag){
	changed_count[wan_unit] = flag;
}

int get_next_unit(int wan_unit){
	int next = (wan_unit+1)%WAN_UNIT_MAX;

	return next;
}

int get_last_unit(int wan_unit){
	int last = wan_unit-1;

	if(last < WAN_UNIT_FIRST)
		last = WAN_UNIT_MAX-1;

	return last;
}

int switch_wan_line(const int wan_unit, const int restart_other){
#if defined(RTCONFIG_DUALWAN) || defined(RTCONFIG_USB_MODEM)
	char tmp[100] = "";
#endif
#ifdef RTCONFIG_USB_MODEM
	int retry, lock;
#endif
	char prefix[] = "wanXXXXXX_", cmd[32];
	int unit;

	for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit)
		if(unit == wan_unit)
			break;
	if(unit == WAN_UNIT_MAX)
		return 0;

	snprintf(prefix, sizeof(prefix), "wan%d_", wan_unit);

	if(wan_primary_ifunit() == wan_unit) // Already have no running modem.
		return 0;
#ifdef RTCONFIG_USB_MODEM
	else if (dualwan_unit__usbif(wan_unit)) {
		if(!link_wan[wan_unit]) {
			nvram_set_int(strcat_r(prefix, "is_usb_modem_ready", tmp), link_wan[wan_unit]);
			return 0; // No modem in USB ports.
		}
	}
#endif

#ifdef RTCONFIG_TR069
	tr_switch_wan_line(wan_unit);
#endif

	_dprintf("%s: wan(%d) Starting...\n", __FUNCTION__, wan_unit);
	// Set the modem to be running.
	set_wan_primary_ifunit(wan_unit);

#ifdef RTCONFIG_USB_MODEM
	if (nvram_invmatch("modem_enable", "4") && dualwan_unit__usbif(wan_unit)) {
		// Wait the PPP config file to be done.
		retry = 0;
		while((lock = file_lock("3g")) == -1 && retry < MAX_WAIT_FILE)
			sleep(1);

		if(lock == -1){
			_dprintf("(%d): No pppd conf file and turn off the state of USB Modem.\n", wan_unit);
			set_wan_primary_ifunit(get_last_unit(wan_unit));
			return 0;
		}
		else
			file_unlock(lock);
	}
#endif

	// TODO: don't know if it's necessary?
	// clean or restart the other line.
	if(restart_other)
	{
		for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit){
			if(unit == wan_unit)
				continue;

			snprintf(prefix, sizeof(prefix), "wan%d_", unit);

#if defined(RTCONFIG_DUALWAN) || defined(RTCONFIG_USB_MODEM)
			if(!nvram_get_int(strcat_r(prefix, "ppp_cross", tmp)) && if_wan_ppp(unit, 0) && !link_wan[unit]){
				// PPTP/L2TP can build the PPP connection on the other WAN
				// Need to avoid it
				_dprintf("wanduck1: stop_wan_if %d.\n", unit);
				snprintf(cmd, sizeof(cmd), "stop_wan_if %d", unit);
			}
			else
#endif
			{
				_dprintf("wanduck1: restart_wan_if %d.\n", unit);
				snprintf(cmd, sizeof(cmd), "restart_wan_if %d", unit);
			}
			notify_rc_and_wait(cmd);
			sleep(1);
		}
	}
#ifdef RTCONFIG_USB_MODEM
	else{
		for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit){
			if(unit == wan_unit)
				continue;

			if(dualwan_unit__nonusbif(unit))
				continue;

			_dprintf("wanduck1: stop_wan_if(usb) %d.\n", unit);
			snprintf(cmd, sizeof(cmd), "stop_wan_if %d", unit);
			notify_rc_and_wait(cmd);
			sleep(1);
		}
	}
#endif

#if defined(RTCONFIG_IPV6) && defined(RTCONFIG_DUALWAN)
	//int ipv6_service = 0;
	/* Start each configured and enabled wan connection and its undelying i/f */
	for (unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit)
		if (get_ipv6_service_by_unit(unit) != IPV6_DISABLED) {
			//ipv6_service = 1;
			break;
		}
#endif

	current_state[wan_unit] = get_wan_state(wan_unit);
	if(current_state[wan_unit] != WAN_STATE_CONNECTING && current_state[wan_unit] != WAN_STATE_CONNECTED && current_state[wan_unit] != WAN_STATE_DISABLED){
		snprintf(cmd, sizeof(cmd), "restart_wan_if %d", wan_unit);
		_dprintf("wanduck2: %s.\n", cmd);
		notify_rc_and_wait(cmd);
	}
	else if(if_wan_ppp(wan_unit, 0)){
		// the connection which built in advance was invalid
		snprintf(cmd, sizeof(cmd), "restart_wan_if %d", wan_unit);
		_dprintf("wanduck2(ppp): %s.\n", cmd);
		notify_rc(cmd);
	}
	else if(strcmp(get_wan_ifname(wan_unit), "")){
		snprintf(cmd, sizeof(cmd), "restart_wan_line %d", wan_unit);
		_dprintf("wanduck2: %s.\n", cmd);
		notify_rc(cmd);
	}

#ifdef RTCONFIG_DUALWAN
	if(is_router_mode()
			&& (!strcmp(dualwan_mode, "fo") || !strcmp(dualwan_mode, "fb"))
			){
		delay_detect = 1;
	}
#endif

	_dprintf("%s: wan(%d) End.\n", __FUNCTION__, wan_unit);
	return 1;
}

int wanduck_main(int argc, char *argv[]){
	char *http_servport, *dns_servport;
	int clilen;
	struct sockaddr_in cliaddr;
	struct timeval  tval;
	int nready, maxi, sockfd;
	int wan_unit, wan_sbstate;
	char prefix_wan[8];
	char wired_link_nvram[16];
#if defined(RTCONFIG_WIRELESSREPEATER) && !defined(RTCONFIG_QCA) && !defined(RTCONFIG_CONCURRENTREPEATER)
	char domain_mapping[64];
#endif
#ifdef RTCONFIG_DSL
	int usb_switched_back_dsl = 0;
#endif
	unsigned int now;
#if defined(RTCONFIG_DUALWAN) || defined(RTCONFIG_USB_MODEM)
	char cmd[32];
#endif
#ifdef RTCONFIG_DUALWAN
#if defined(RTAC68U) ||  defined(RTAC87U) || defined(RTAC3200) || defined(RTCONFIG_BCM_7114) || defined(HND_ROUTER) && !defined(RTCONFIG_HND_ROUTER_AX)
	int wanred_led_status = 0;	/* 1 is no internet, 2 is internet ok */
	int u, link_status;
#endif
#endif
#ifdef RTCONFIG_QTN
	time_t qtn_now;
	struct tm *qtn_tm;
	char time_string[sizeof("MMDDhhmmYYYY")];
#endif
#ifdef RTCONFIG_USB_MODEM
	int modem_unit;
	char tmp2[100], prefix2[32];
	char modem_type[32];
#endif

	wdbg = nvram_get_int("wdbg");
	test_log = nvram_get_int("wanduck_debug");
#if defined(RTCONFIG_HIDDEN_BACKHAUL)
	char vif[15];
	if(sw_mode() != SW_MODE_REPEATER)
		sprintf(vif,"%s.%s",WIF_5G_BH,"55");
#endif

	signal(SIGHUP, SIG_IGN);
	signal(SIGTERM, safe_leave);
	signal(SIGCHLD, chld_reap);
	signal(SIGUSR1, get_network_nvram);
	signal(SIGUSR2, wan_led_control);

	if(argc < 3){
		http_servport = DFL_HTTP_SERV_PORT;
		dns_servport = DFL_DNS_SERV_PORT;
	}
	else{
		if(atoi(argv[1]) <= 0)
			http_servport = DFL_HTTP_SERV_PORT;
		else
			http_servport = (char *)argv[1];

		if(atoi(argv[2]) <= 0)
			dns_servport = DFL_DNS_SERV_PORT;
		else
			dns_servport = (char *)argv[2];
	}

	if(build_socket(http_servport, dns_servport, &http_sock, &dns_sock) < 0){
		_dprintf("\n*** Fail to build socket! ***\n");
		exit(0);
	}
	if(fcntl(dns_sock, F_SETFL, fcntl(dns_sock, F_GETFL, 0) | O_NONBLOCK) < 0){
		_dprintf("wanduck set dnssock [%d] nonblock fail !\n", dns_sock);
		exit(0);
	}

	FILE *fp = fopen(WANDUCK_PID_FILE, "w");

	if(fp != NULL){
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}

	maxfd = (http_sock > dns_sock)?http_sock:dns_sock;
	maxi = -1;

	FD_ZERO(&allset);
	FD_SET(http_sock, &allset);
	FD_SET(dns_sock, &allset);

	for(fd_i = 0; fd_i < MAX_USER; ++fd_i){
		client[fd_i].sfd = -1;
		client[fd_i].type = 0;
	}

	rule_setup = 0;
	got_notify = 0;
	modem_act_reset = 0;
	clilen = sizeof(cliaddr);

	snprintf(router_name, sizeof(router_name), "%s", DUT_DOMAIN_NAME);

	nvram_set_int("link_internet", 0);

#ifdef RTCONFIG_WIRELESSREPEATER
	nvram_set_int("link_ap", 2);
#endif

#if defined(RTCONFIG_DUALWAN) || defined(RTCONFIG_USB_MODEM)
	for(wan_unit = WAN_UNIT_FIRST; wan_unit < WAN_UNIT_MAX; ++wan_unit)
#else
	wan_unit = WAN_UNIT_FIRST;
#endif
	{
		link_changed[wan_unit] = 0;
		link_setup[wan_unit] = 0;
		link_wan[wan_unit] = 0;
		link_wan_old = -1;

		changed_count[wan_unit] = S_IDLE;
		disconn_case[wan_unit] = CASE_NONE;

		snprintf(prefix_wan, sizeof(prefix_wan), "wan%d_", wan_unit);

		strcat_r(prefix_wan, "state_t", nvram_state[wan_unit]);
		strcat_r(prefix_wan, "sbstate_t", nvram_sbstate[wan_unit]);
		strcat_r(prefix_wan, "auxstate_t", nvram_auxstate[wan_unit]);

		set_disconn_count(wan_unit, S_IDLE);

		link_wan_nvname(wan_unit, wired_link_nvram, sizeof(wired_link_nvram));
		nvram_set_int(wired_link_nvram, link_wan[wan_unit]);

#ifdef RTCONFIG_USB_MODEM
		nvram_set_int(strcat_r(prefix_wan, "is_usb_modem_ready", tmp2), link_wan[wan_unit]);
#endif
	}

	loop_sec = uptime();

	get_related_nvram(); // initial the System's variables.
	get_lan_nvram(); // initial the LAN's variables.

	nat_redirect_enable_old = nat_redirect_enable;
	nat_state = nvram_get_int("nat_state");

#ifdef RTCONFIG_DUALWAN
#if 1
	WAN_FB_UNIT = WAN_UNIT_FIRST;
#else
	WAN_FB_UNIT = WAN_UNIT_SECOND;
#endif

	// let wanduck's first detect be activated after start_wan().
	int i;
	for(i = 0; i < 5; ++i){
		if(nvram_get_int("wanduck_start_detect") == 1)
			break;

		_dprintf("wanduck: delay %d seconds before the first detect...\n", i+1);
		sleep(1);
	}

#ifdef WEB_REDIRECT
	if(nvram_get_int("freeze_duck"))
	{
		if (test_log)
		_dprintf("\n<*>freeze the duck, %ds left!\n", nvram_get_int("freeze_duck"));	// don't check conn state during inner events period
	}
	else
#endif
	if(is_router_mode() && !strcmp(dualwan_mode, "lb")){
		cross_state = DISCONN;
		for(wan_unit = WAN_UNIT_FIRST; wan_unit < WAN_UNIT_MAX; ++wan_unit){
			if(get_dualwan_by_unit(wan_unit) == WANS_DUALWAN_IF_NONE)
				continue;

			conn_state[wan_unit] = if_wan_phyconnected(wan_unit);
			if(conn_state[wan_unit] == CONNED){
				current_state[wan_unit] = nvram_get_int(nvram_state[wan_unit]);
#ifdef RTCONFIG_USB_MODEM
				if(dualwan_unit__usbif(wan_unit) && current_state[wan_unit] == WAN_STATE_INITIALIZING)
					conn_state[wan_unit] = DISCONN;
				else
#endif
					conn_state[wan_unit] = if_wan_connected(wan_unit);
			}
			else
				conn_state[wan_unit] = DISCONN;

			conn_changed_state[wan_unit] = conn_state[wan_unit];

			if(conn_state[wan_unit] == CONNED && cross_state != CONNED)
				cross_state = CONNED;

			conn_state_old[wan_unit] = conn_state[wan_unit];

			record_conn_status(wan_unit);

			if(cross_state == CONNED)
				set_disconn_count(wan_unit, S_IDLE);
			else
				set_disconn_count(wan_unit, S_COUNT);
		}
	}
	else if(is_router_mode()
			&& (!strcmp(dualwan_mode, "fo") || !strcmp(dualwan_mode, "fb"))
			){
		if(wandog_delay > 0){
			_dprintf("wanduck: delay %d seconds...\n", wandog_delay);
			sleep(wandog_delay);
			delay_detect = 0;
		}

		// To check the phy connection of the standby line.
		for(wan_unit = WAN_UNIT_FIRST; wan_unit < WAN_UNIT_MAX; ++wan_unit){
			if(get_dualwan_by_unit(wan_unit) != WANS_DUALWAN_IF_NONE)
				conn_state[wan_unit] = if_wan_phyconnected(wan_unit);
		}

		current_wan_unit = wan_primary_ifunit();
		other_wan_unit = get_next_unit(current_wan_unit);
if(test_log)
_dprintf("wanduck(%d)(first detect start): state %d, state_old %d, changed %d, wan_state %d.\n"
		, current_wan_unit, conn_state[current_wan_unit], conn_state_old[current_wan_unit], conn_changed_state[current_wan_unit], current_state[current_wan_unit]);

		if(conn_state[current_wan_unit] == CONNED){
			current_state[current_wan_unit] = nvram_get_int(nvram_state[current_wan_unit]);
#ifdef RTCONFIG_USB_MODEM
			if(!(dualwan_unit__usbif(current_wan_unit) && current_state[current_wan_unit] == WAN_STATE_INITIALIZING))
#endif
				conn_state[current_wan_unit] = if_wan_connected(current_wan_unit);

			cross_state = conn_state[current_wan_unit];
		}
		else
			cross_state = DISCONN;

		conn_changed_state[current_wan_unit] = conn_state[current_wan_unit];

		conn_state_old[current_wan_unit] = conn_state[current_wan_unit];

		record_conn_status(current_wan_unit);
	}
	else
#endif // RTCONFIG_DUALWAN
	if(is_router_mode()
#ifdef RTCONFIG_WIRELESSREPEATER
			|| sw_mode == SW_MODE_HOTSPOT
#endif
			){
		current_wan_unit = wan_primary_ifunit();
#ifdef RTCONFIG_USB_MODEM
		other_wan_unit = get_next_unit(current_wan_unit);
#endif

		conn_state[current_wan_unit] = if_wan_phyconnected(current_wan_unit);
		if(conn_state[current_wan_unit] == CONNED){
			current_state[current_wan_unit] = nvram_get_int(nvram_state[current_wan_unit]);
#ifdef RTCONFIG_USB_MODEM
			if(!(dualwan_unit__usbif(current_wan_unit) && current_state[current_wan_unit] == WAN_STATE_INITIALIZING))
#endif
				conn_state[current_wan_unit] = if_wan_connected(current_wan_unit);

			cross_state = conn_state[current_wan_unit];
		}
		else
			cross_state = DISCONN;

		conn_changed_state[current_wan_unit] = conn_state[current_wan_unit];

		conn_state_old[current_wan_unit] = conn_state[current_wan_unit];

		record_conn_status(current_wan_unit);
	}
	else{ // sw_mode == SW_MODE_AP, SW_MODE_REPEATER.
		current_wan_unit = WAN_UNIT_FIRST;

		conn_state[current_wan_unit] = if_wan_phyconnected(current_wan_unit);

		cross_state = conn_state[current_wan_unit];

		conn_state_old[current_wan_unit] = conn_state[current_wan_unit];

#ifdef RTCONFIG_WIRELESSREPEATER
#ifdef RTCONFIG_REALTEK
/* [MUST]: Need to disscuss to add new mode for Media Bridge */
		if(repeater_mode() || mediabridge_mode())
#else
		if(sw_mode == SW_MODE_REPEATER)
#endif
			record_conn_status(current_wan_unit);
#endif

		if(sw_mode == SW_MODE_AP){
#ifdef RTCONFIG_REDIRECT_DNAME
			if (nvram_invmatch("redirect_dname", "0") && access_point_mode()) {

			if(cross_state == DISCONN){
				int evalRet;
				_dprintf("\n# AP mode: Enable direct rule(DISCONN)\n");
				eval("ebtables", "-t", "broute", "-F");
				eval("ebtables", "-t", "filter", "-F");
#if defined(RTCONFIG_CONCURRENTREPEATER) && defined(RTCONFIG_RALINK)
				eval("rmmod", "ebtable_broute");
				eval("rmmod", "ebtable_filter");
				eval("rmmod", "ebtables");
#endif
				redirect_setting();
				evalRet = eval("iptables-restore", REDIRECT_RULES);
				rule_apply_checking("wanduck", __LINE__, REDIRECT_RULES, evalRet);
				// nat_rules = NAT_STATE_REDIRECT;
			}
#ifdef RTCONFIG_WIFI_SON
			else if(!nvram_match("cfg_master", "1") && nvram_match("wifison_ready", "1"))
				; // do nothing.
#endif
			else if(cross_state == CONNED){
				int evalRet;
				_dprintf("\n# AP mode: Disable direct rule(CONNED)\n");
				eval("ebtables", "-t", "broute", "-F");
				eval("ebtables", "-t", "filter", "-F");
				eval("ebtables", "-t", "broute", "-I", "BROUTING", "-p", "ipv4", "--ip-proto", "udp", "--ip-dport", "53", "-j", "redirect", "--redirect-target", "DROP");
#ifdef RTCONFIG_WIFI_SON
				if((sw_mode() == SW_MODE_AP && nvram_match("cfg_master", "1")) && nvram_match("wifison_ready", "1"))
                                {
                                        if(nvram_get_int("wl0.1_bss_enabled"))
                                        {
#if defined(RTCONFIG_HIDDEN_BACKHAUL)
                                                eval("ebtables", "-t", "broute", "-I", "BROUTING","-i",vif ,"-j","ACCEPT");
#else
                                                eval("ebtables", "-t", "broute", "-I", "BROUTING","-i","ath1.55" ,"-j","ACCEPT");
#endif
                                                eval("ebtables", "-t", "broute", "-I", "BROUTING","-i",nvram_safe_get("wl0.1_ifname"),"-j","ACCEPT");
                                        }
                                }
#endif
				redirect_nat_setting();
				evalRet = eval("iptables-restore", NAT_RULES);
				rule_apply_checking("wanduck", __LINE__, NAT_RULES, evalRet);
				// nat_rules = NAT_STATE_NORMAL;
			}

			}
#endif

#ifdef RTCONFIG_RESTRICT_GUI
			if(cross_state == CONNED){
				if(nvram_get_int("fw_restrict_gui")){
					char word[PATH_MAX], *next_word;

					foreach(word, nvram_safe_get("lan_ifnames"), next_word){
						SKIP_ABSENT_FAKE_IFACE(word);
						if(!strncmp(word, "vlan", 4))
							continue;

						eval("ebtables", "-t", "broute", "-I", "BROUTING", "-i", word, "-j", "mark", "--mark-set", BIT_RES_GUI, "--mark-target", "CONTINUE");
					}

					repeater_filter_setting(0);
				}
			}
#endif
		}
	}

	/*
	 * Because start_wanduck() is run early than start_wan()
	 * and the redirect rules is already set before running wanduck,
	 * handle_wan_line() must not be run at the first detect.
	 */
#ifdef WEB_REDIRECT
	if(nvram_get_int("freeze_duck"))
	{
		if (test_log)
		_dprintf("\n<**>freeze the duck, %ds left!\n", nvram_get_int("freeze_duck"));	// don't check conn state during inner events period
	}
	else
#endif
	if(cross_state == DISCONN){
		_dprintf("\n# Enable direct rule\n");
		rule_setup = 1;
	}
	else if(cross_state == CONNED && isFirstUse){
		_dprintf("\n#CONNED : Enable direct rule\n");
		rule_setup = 1;
	}
	else{ // CONNED at the first detect
#if defined(RTCONFIG_WPS_ALLLED_BTN)
		if(nvram_match("AllLED", "1"))
			led_control(LED_WAN, LED_ON);
		else
			led_control(LED_WAN, LED_OFF);
#elif defined(DSL_N55U) || defined(DSL_N55U_B)
		led_control(LED_WAN, LED_ON);
#elif defined(RTAC68U) || defined(RTAC87U) || defined(RTAC3200) || defined(RTCONFIG_BCM_7114) || (defined(HND_ROUTER) && !defined(RTCONFIG_HND_ROUTER_AX))
		if(nvram_match("AllLED", "1")
#ifdef RTAC68U
				&& (is_ac66u_v2_series() || is_ac68u_v3_series())
#endif
				)
		{
			enable_wan_led();
		}
#endif
		_dprintf("\n#CONNED : first detect\n");
		rule_setup = 0;
	}

#if defined(RTCONFIG_HND_ROUTER_AX) || defined(RTCONFIG_LANWAN_LED) || defined(RTCONFIG_WANRED_LED) || defined(RTCONFIG_FAILOVER_LED)
	for(wan_unit = WAN_UNIT_FIRST; wan_unit < WAN_UNIT_MAX; ++wan_unit){
		if(get_dualwan_by_unit(wan_unit) == WANS_DUALWAN_IF_NONE)
			continue;

		if(wan_unit != wan_primary_ifunit()
#ifdef RTCONFIG_DUALWAN
				&& strcmp(dualwan_mode, "lb")
#endif
				)
			continue;

		update_wan_leds(wan_unit, !rule_setup);
	}
#endif

if(test_log)
_dprintf("wanduck(%d)(first detect   end): state %d, state_old %d, changed %d, wan_state %d.\n"
		, current_wan_unit, conn_state[current_wan_unit], conn_state_old[current_wan_unit], conn_changed_state[current_wan_unit], current_state[current_wan_unit]);

	int first_loop = 1;
	unsigned int diff;
	for(;;){
		if(!first_loop){
			now = uptime();
			diff = now-loop_sec;

			if(diff < scan_interval){
				rset = allset;
				tval.tv_sec = scan_interval-diff;
				tval.tv_usec = 0;

				goto WANDUCK_SELECT;
			}

			loop_sec = now;
		}
		else
			first_loop = 0;

		rset = allset;
		tval.tv_sec = scan_interval;
		tval.tv_usec = 0;

		get_related_nvram();		

		// Sam 2014/10/14
		// rule_setup depend on wan status,
		// if nat_redirect_enable changed, rebuild nat rule.
		if(rule_setup) {
			if(!nat_redirect_enable_old && nat_redirect_enable)	//need redirect
			{
_dprintf("nat_rule: stop_nat_rules 4.\n");
				nat_state = stop_nat_rules();
			}
			else if(nat_redirect_enable_old && !nat_redirect_enable)	//don't redirect
			{
_dprintf("nat_rule: start_nat_rules 4.\n");
				nat_state = start_nat_rules();
			}
		}

		if(nat_redirect_enable_old != nat_redirect_enable)
			nat_redirect_enable_old = nat_redirect_enable;
		//_dprintf("rule_setup: %d, nat_state: %s\n", rule_setup, nvram_safe_get("nat_state"));

#ifdef WEB_REDIRECT
		if(nvram_get_int("freeze_duck")){
			if (test_log)
			_dprintf("\n<****>freeze the duck, %ds left!\n", nvram_get_int("freeze_duck"));	// don't check conn state during inner events period
			goto WANDUCK_SELECT;
		}
		else
#endif
#ifdef RTCONFIG_DUALWAN
		if(is_router_mode() && !strcmp(dualwan_mode, "lb")){
			cross_state = DISCONN;
			for(wan_unit = WAN_UNIT_FIRST; wan_unit < WAN_UNIT_MAX; ++wan_unit){
#if 0
#ifdef RTCONFIG_USB_MODEM
				if(dualwan_unit__usbif(wan_unit) && !link_wan[wan_unit]){
					if_wan_phyconnected(wan_unit);
					goto WANDUCK_SELECT;
				}
#endif
#endif
				if(get_dualwan_by_unit(wan_unit) == WANS_DUALWAN_IF_NONE){
					link_wan[wan_unit] = 0;
					link_wan_old = link_wan[wan_unit];
					continue;
				}

				link_wan_old = link_wan[wan_unit];
				conn_state[wan_unit] = if_wan_phyconnected(wan_unit);
if(test_log){
_dprintf("wanduck(%d)(PHY state): PHY_RECONN=%d...\n", wan_unit, PHY_RECONN);
_dprintf("wanduck(%d)(PHY state): %d...\n", wan_unit, conn_state[wan_unit]);
}

				current_state[wan_unit] = nvram_get_int(nvram_state[wan_unit]);

				if(current_state[wan_unit] == WAN_STATE_DISABLED){
					//record_wan_state_nvram(wan_unit, WAN_STATE_STOPPED, WAN_STOPPED_REASON_MANUAL, -1);

					disconn_case[wan_unit] = CASE_OTHERS;
					conn_state[wan_unit] = DISCONN;
				}
#ifdef RTCONFIG_USB_MODEM
				else if(dualwan_unit__usbif(wan_unit)
						&& (modem_act_reset == 1 || modem_act_reset == 2)
						){
_dprintf("wanduck(%d): detect the modem to be reset...\n", wan_unit);
					disconn_case[wan_unit] = CASE_OTHERS;
					conn_state[wan_unit] = DISCONN;
					set_disconn_count(wan_unit, S_IDLE);
				}
#endif
				else{
#if defined(RTCONFIG_HND_ROUTER_AX) || defined(RTCONFIG_LANWAN_LED) || defined(RTCONFIG_WANRED_LED) || defined(RTCONFIG_FAILOVER_LED)
					if(link_wan[wan_unit] != link_wan_old)
						update_wan_leds(wan_unit, link_wan[wan_unit]);
#endif

					if(conn_state[wan_unit] == CONNED){
#ifdef RTCONFIG_USB_MODEM
						if(dualwan_unit__usbif(wan_unit) && current_state[wan_unit] == WAN_STATE_INITIALIZING)
							conn_state[wan_unit] = DISCONN;
						else
#endif
							conn_state[wan_unit] = if_wan_connected(wan_unit);
if(test_log)
_dprintf("wanduck(%d)(conn     ): %d...\n", wan_unit, conn_state[wan_unit]);
					}
				}

				wan_sbstate = nvram_get_int(nvram_sbstate[wan_unit]);
if(test_log)
_dprintf("wanduck(%d)(sbstate  ): %d...\n", wan_unit, wan_sbstate);

#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2) || defined(RTCONFIG_UBIFS)
				if(disconn_case_old[wan_unit] != CASE_DATALIMIT && wan_sbstate == WAN_STOPPED_REASON_DATALIMIT){
					_dprintf("wanduck(%d)(lb): detect the data limit.\n", wan_unit);
					conn_state[wan_unit] = DISCONN;
				}
#endif

				if(conn_state[wan_unit] == CONNED && cross_state != CONNED)
					cross_state = CONNED;
if(test_log)
_dprintf("wanduck(%d)(link_wan ): %d...\n", wan_unit, link_wan[wan_unit]);

#ifdef RTCONFIG_USB_MODEM
				if(wan_sbstate != WAN_STOPPED_REASON_DATALIMIT && dualwan_unit__usbif(wan_unit)){
if(test_log)
_dprintf("wanduck(%d): decide start_wan_if or stop_wan_if...\n", wan_unit);
					if(link_wan[wan_unit] == 1 && current_state[wan_unit] == WAN_STATE_INITIALIZING && boot_end == 1){
						set_disconn_count(wan_unit, S_COUNT); // reset count for the new start_wan_if.
						_dprintf("wanduck: start_wan_if %d.\n", wan_unit);
						snprintf(cmd, sizeof(cmd), "start_wan_if %d", wan_unit);
						notify_rc(cmd);
						goto WANDUCK_SELECT;
					}
					else if(!link_wan[wan_unit] && current_state[wan_unit] != WAN_STATE_INITIALIZING
							//&& current_state[wan_unit] != WAN_STATE_STOPPED
							){
						_dprintf("wanduck: stop_wan_if %d.\n", wan_unit);
						snprintf(cmd, sizeof(cmd), "stop_wan_if %d", wan_unit);
						notify_rc(cmd);
						goto WANDUCK_SELECT;
					}
				}

				if(conn_state[wan_unit] == SET_PIN){
					conn_changed_state[wan_unit] = SET_PIN;
					set_disconn_count(wan_unit, S_IDLE);
				}
				else if(conn_state[wan_unit] == SET_USBSCAN){
					conn_changed_state[wan_unit] = SET_USBSCAN;
					set_disconn_count(wan_unit, S_IDLE);
				}
				else
#endif
				if(conn_state[wan_unit] != conn_state_old[wan_unit]){
					conn_state_old[wan_unit] = conn_state[wan_unit];

					if(conn_state[wan_unit] == PHY_RECONN){
						conn_changed_state[wan_unit] = PHY_RECONN;

						set_disconn_count(wan_unit, max_disconn_count[wan_unit]);
					}
					else if(conn_state[wan_unit] == DISCONN){
						conn_changed_state[wan_unit] = C2D;

#ifdef RTCONFIG_USB_MODEM
						if (dualwan_unit__usbif(wan_unit))
							set_disconn_count(wan_unit, max_disconn_count[wan_unit]);
						else
#endif
#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2) || defined(RTCONFIG_UBIFS)
						if(disconn_case[wan_unit] == CASE_DATALIMIT)
							set_disconn_count(wan_unit, max_disconn_count[wan_unit]);
						else
#endif
#ifdef RTCONFIG_DSL ///TODO: apply to all ?
						if (wan_unit == WAN_UNIT_FIRST && !link_wan[wan_unit])
							set_disconn_count(wan_unit, max_disconn_count[wan_unit]);
						else
#endif
							set_disconn_count(wan_unit, S_COUNT);
					}
					else if(conn_state[wan_unit] == CONNED){
						conn_changed_state[wan_unit] = D2C;

						set_disconn_count(wan_unit, S_IDLE);
					}
					else
						conn_changed_state[wan_unit] = CONNED;

					record_conn_status(wan_unit);
				}

				if(get_disconn_count(wan_unit) != S_IDLE){
					if(get_disconn_count(wan_unit) >= max_disconn_count[wan_unit]){
						set_disconn_count(wan_unit, S_IDLE);

#ifdef RTCONFIG_USB_MODEM
						if(dualwan_unit__usbif(wan_unit)){
							if((modem_unit = get_modemunit_by_type(get_dualwan_by_unit(wan_unit))) == MODEM_UNIT_NONE){
								_dprintf("%s 3: cannot get the modem unit!\n", __FUNCTION__);
								goto WANDUCK_SELECT;
							}

							usb_modem_prefix(modem_unit, prefix2, sizeof(prefix2));
							snprintf(modem_type, sizeof(modem_type), "%s", nvram_safe_get(strcat_r(prefix2, "act_type", tmp2)));
							_dprintf("\n# wanduck(%d): modem_type=%s.\n", wan_unit, modem_type);
						}

						if(dualwan_unit__usbif(wan_unit) &&
								(!strcmp(modem_type, "") || (strcmp(modem_type, "tty") && strcmp(modem_type, "mbim")))
								){
							// connection be activated by hotplug or PHY_RECONN.
							_dprintf("\n# wanduck(%d): skip to run restart_wan_if.\n", wan_unit);
						}
						else
#endif
						{
							// connection be activated by wanduck.
							_dprintf("\n# wanduck(%d): run restart_wan_if.\n", wan_unit);
							snprintf(cmd, sizeof(cmd), "restart_wan_if %d", wan_unit);
							notify_rc(cmd);
						}

						if(is_wan_connect(get_next_unit(wan_unit))){
							_dprintf("\n# wanduck(%d): run restart_wan_line %d.\n", wan_unit, get_next_unit(wan_unit));
							snprintf(cmd, sizeof(cmd), "restart_wan_line %d", get_next_unit(wan_unit));
							notify_rc(cmd);
						}
					}
					else
						set_disconn_count(wan_unit, get_disconn_count(wan_unit)+1);

					_dprintf("%s: wan(%d) disconn count = %d/%d ...\n", __FUNCTION__, wan_unit, get_disconn_count(wan_unit), max_disconn_count[wan_unit]);
				}
			}
if(test_log)
_dprintf("wanduck(%d)(lb change): state %d, state_old %d, changed %d, cross_state=%d, wan_state %d.\n"
		, current_wan_unit, conn_state[current_wan_unit], conn_state_old[current_wan_unit], conn_changed_state[current_wan_unit], cross_state, current_state[current_wan_unit]);
		}
		else if(is_router_mode() && !strcmp(dualwan_mode, "fo")){
			if(delay_detect == 1 && wandog_delay > 0){
				_dprintf("wanduck: delay %d seconds...\n", wandog_delay);
				sleep(wandog_delay);
				delay_detect = 0;
			}

			// To check the phy connection of the standby line.
			for(wan_unit = WAN_UNIT_FIRST; wan_unit < WAN_UNIT_MAX; ++wan_unit){
				if(get_dualwan_by_unit(wan_unit) != WANS_DUALWAN_IF_NONE)
					conn_state[wan_unit] = if_wan_phyconnected(wan_unit);
			}

			current_wan_unit = wan_primary_ifunit();
			other_wan_unit = get_next_unit(current_wan_unit);

			current_state[current_wan_unit] = nvram_get_int(nvram_state[current_wan_unit]);
if(test_log)
_dprintf("wanduck(%d)(fo    phy): state %d, state_old %d, changed %d, wan_state %d.\n"
		, current_wan_unit, conn_state[current_wan_unit], conn_state_old[current_wan_unit], conn_changed_state[current_wan_unit], current_state[current_wan_unit]);

#ifdef RTCONFIG_USB_MODEM
			if(conn_state[current_wan_unit] == CONNED && conn_state_old[current_wan_unit] == PHY_RECONN)
				conn_state[current_wan_unit] = PHY_RECONN;
#endif

			if(current_state[current_wan_unit] == WAN_STATE_DISABLED){
				//record_wan_state_nvram(current_wan_unit, WAN_STATE_STOPPED, WAN_STOPPED_REASON_MANUAL, -1);

				disconn_case[current_wan_unit] = CASE_OTHERS;
				conn_state[current_wan_unit] = DISCONN;
				set_disconn_count(current_wan_unit, S_IDLE);
			}
#ifdef RTCONFIG_USB_MODEM
			else if(dualwan_unit__usbif(current_wan_unit)
					&& (modem_act_reset == 1 || modem_act_reset == 2)
					){
_dprintf("wanduck(%d): detect the modem to be reset...\n", current_wan_unit);
				disconn_case[current_wan_unit] = CASE_OTHERS;
				conn_state[current_wan_unit] = DISCONN;
				set_disconn_count(current_wan_unit, S_IDLE);
			}
#endif
			else{
				if(conn_state[current_wan_unit] == CONNED){
#ifdef RTCONFIG_USB_MODEM
					if(!(dualwan_unit__usbif(current_wan_unit) && current_state[current_wan_unit] == WAN_STATE_INITIALIZING))
#endif
						conn_state[current_wan_unit] = if_wan_connected(current_wan_unit);
if(test_log)
_dprintf("wanduck(%d)(fo   conn): state %d, state_old %d, changed %d, wan_state %d.\n"
		, current_wan_unit, conn_state[current_wan_unit], conn_state_old[current_wan_unit], conn_changed_state[current_wan_unit], current_state[current_wan_unit]);
				}
			}

			wan_sbstate = nvram_get_int(nvram_sbstate[current_wan_unit]);

			if(conn_state[current_wan_unit] == PHY_RECONN){
				conn_changed_state[current_wan_unit] = PHY_RECONN;

				conn_state_old[current_wan_unit] = DISCONN;

				// When the current line is re-plugged and the other line has plugged, the count has to be reset.
				if(get_dualwan_by_unit(other_wan_unit) != WANS_DUALWAN_IF_NONE && link_wan[other_wan_unit]){
					_dprintf("# wanduck: set S_COUNT: PHY_RECONN.\n");
					set_disconn_count(current_wan_unit, S_COUNT);
				}
			}
#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2) || defined(RTCONFIG_UBIFS)
			else if(wan_sbstate == WAN_STOPPED_REASON_DATALIMIT){
				if(conn_state_old[current_wan_unit] == CONNED){
					_dprintf("wanduck(%d)(fo): detect the data limit.\n", current_wan_unit);
					conn_changed_state[current_wan_unit] = C2D;
				}
				else
					conn_changed_state[current_wan_unit] = DISCONN;

				conn_state_old[current_wan_unit] = DISCONN;

				if(get_dualwan_by_unit(other_wan_unit) != WANS_DUALWAN_IF_NONE
						&& link_wan[other_wan_unit]
						&& nvram_get_int(nvram_sbstate[other_wan_unit]) != WAN_STOPPED_REASON_DATALIMIT)
					set_disconn_count(current_wan_unit, max_disconn_count[current_wan_unit]);
				else
					set_disconn_count(current_wan_unit, S_IDLE);
			}
#endif
#ifdef RTCONFIG_USB_MODEM
			else if(conn_state[current_wan_unit] == SET_PIN){
				conn_changed_state[current_wan_unit] = SET_PIN;

				conn_state_old[current_wan_unit] = DISCONN;
				// The USB modem needs the PIN code to unlock.
				set_disconn_count(current_wan_unit, S_IDLE);
			}
			else if(conn_state[current_wan_unit] == SET_USBSCAN){
				conn_changed_state[current_wan_unit] = C2D;

				conn_state_old[current_wan_unit] = DISCONN;
				// The USB modem is scanning the stations.
				set_disconn_count(current_wan_unit, S_IDLE);
			}
#if 0
#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2) || defined(RTCONFIG_UBIFS)
			else if(dualwan_unit__usbif(current_wan_unit) && wan_sbstate == WAN_STOPPED_REASON_DATALIMIT){
				if(conn_state_old[current_wan_unit] == CONNED){
					_dprintf("wanduck(%d)(fo): detect the data limit.\n", current_wan_unit);
					conn_changed_state[current_wan_unit] = C2D;
				}
				else
					conn_changed_state[current_wan_unit] = DISCONN;

				conn_state_old[current_wan_unit] = DISCONN;
				//set_disconn_count(current_wan_unit, max_disconn_count[current_wan_unit]);
				set_disconn_count(current_wan_unit, S_IDLE);
			}
#endif
#endif
#endif
			else if(conn_state[current_wan_unit] == CONNED){
				if(conn_state_old[current_wan_unit] == DISCONN)
					conn_changed_state[current_wan_unit] = D2C;
				else
					conn_changed_state[current_wan_unit] = CONNED;

				conn_state_old[current_wan_unit] = conn_state[current_wan_unit];
				set_disconn_count(current_wan_unit, S_IDLE);
			}
			else if(conn_state[current_wan_unit] == DISCONN){
				if(conn_state_old[current_wan_unit] == CONNED)
					conn_changed_state[current_wan_unit] = C2D;
				else
					conn_changed_state[current_wan_unit] = DISCONN;

				conn_state_old[current_wan_unit] = conn_state[current_wan_unit];

				if(disconn_case[current_wan_unit] == CASE_THESAMESUBNET){
					_dprintf("# wanduck: set S_IDLE: CASE_THESAMESUBNET.\n");
					set_disconn_count(current_wan_unit, S_IDLE);
				}
#ifdef RTCONFIG_USB_MODEM
				// when the other line is modem and not plugged, the current disconnected line would not count.
				else if(!link_wan[other_wan_unit] && dualwan_unit__usbif(other_wan_unit))
					set_disconn_count(current_wan_unit, S_IDLE);
#endif
				else if(current_state[current_wan_unit] != WAN_STATE_DISABLED
						&& get_dualwan_by_unit(other_wan_unit) != WANS_DUALWAN_IF_NONE) {
					if (get_disconn_count(current_wan_unit) == S_IDLE)
						set_disconn_count(current_wan_unit, S_COUNT);
				}
				// when auth failed, the single disconnected line would not count.
				else if(disconn_case[current_wan_unit] == CASE_PPPFAIL && wan_sbstate == WAN_STOPPED_REASON_PPP_AUTH_FAIL)
					set_disconn_count(current_wan_unit, S_IDLE);
			}

			if(get_disconn_count(current_wan_unit) != S_IDLE){
				if(get_disconn_count(current_wan_unit) < max_disconn_count[current_wan_unit]){
					set_disconn_count(current_wan_unit, get_disconn_count(current_wan_unit)+1);
					_dprintf("# wanduck(%d): wait time for switching: %d/%d.\n", current_wan_unit, get_disconn_count(current_wan_unit)*scan_interval, max_wait_time[current_wan_unit]);
				}
				else{
					_dprintf("# wanduck(%d): set S_COUNT: changed_count[] >= max_disconn_count.\n", current_wan_unit);
				}
			}

			record_conn_status(current_wan_unit);
if(test_log)
_dprintf("wanduck(%d)(fo change): state %d, state_old %d, changed %d, wan_state %d.\n"
		, current_wan_unit, conn_state[current_wan_unit], conn_state_old[current_wan_unit], conn_changed_state[current_wan_unit], current_state[current_wan_unit]);
		}
		else if(is_router_mode() && !strcmp(dualwan_mode, "fb")){
			if(delay_detect == 1 && wandog_delay > 0){
				_dprintf("wanduck: delay %d seconds...\n", wandog_delay);
				sleep(wandog_delay);
				delay_detect = 0;
			}

			// To check the phy connection of the standby line.
			for(wan_unit = WAN_UNIT_FIRST; wan_unit < WAN_UNIT_MAX; ++wan_unit){
				if(get_dualwan_by_unit(wan_unit) != WANS_DUALWAN_IF_NONE)
					conn_state[wan_unit] = if_wan_phyconnected(wan_unit);
			}

			current_wan_unit = wan_primary_ifunit();
			other_wan_unit = get_next_unit(current_wan_unit);

			current_state[current_wan_unit] = nvram_get_int(nvram_state[current_wan_unit]);

#ifdef RTCONFIG_USB_MODEM
			if(conn_state[current_wan_unit] == CONNED && conn_state_old[current_wan_unit] == PHY_RECONN)
				conn_state[current_wan_unit] = PHY_RECONN;
#endif

			if(current_state[current_wan_unit] == WAN_STATE_DISABLED){
				//record_wan_state_nvram(current_wan_unit, WAN_STATE_STOPPED, WAN_STOPPED_REASON_MANUAL, -1);

				disconn_case[current_wan_unit] = CASE_OTHERS;
				conn_state[current_wan_unit] = DISCONN;
				set_disconn_count(current_wan_unit, S_IDLE);
			}
#ifdef RTCONFIG_USB_MODEM
			else if(dualwan_unit__usbif(current_wan_unit)
					&& (modem_act_reset == 1 || modem_act_reset == 2)
					){
_dprintf("wanduck(%d): detect the modem to be reset...\n", current_wan_unit);
				disconn_case[current_wan_unit] = CASE_OTHERS;
				conn_state[current_wan_unit] = DISCONN;
				set_disconn_count(current_wan_unit, S_IDLE);
			}
#endif
			else{
				if(conn_state[current_wan_unit] == CONNED){
#ifdef RTCONFIG_USB_MODEM
					if(!(dualwan_unit__usbif(current_wan_unit) && current_state[current_wan_unit] == WAN_STATE_INITIALIZING))
#endif
						conn_state[current_wan_unit] = if_wan_connected(current_wan_unit);
				}

				if(other_wan_unit == WAN_FB_UNIT && conn_state[other_wan_unit] == CONNED){
					current_state[other_wan_unit] = nvram_get_int(nvram_state[other_wan_unit]);
#ifdef RTCONFIG_USB_MODEM
					if(!(dualwan_unit__usbif(other_wan_unit) && current_state[other_wan_unit] == WAN_STATE_INITIALIZING))
#endif
						conn_state[other_wan_unit] = if_wan_connected(other_wan_unit);
					_dprintf("wanduck: detect the fail-back line(%d)...\n", other_wan_unit);
if(test_log)
_dprintf("wanduck(%d) fail-back: state %d, state_old %d, changed %d, wan_state %d.\n"
		, other_wan_unit, conn_state[other_wan_unit], conn_state_old[other_wan_unit], conn_changed_state[other_wan_unit], current_state[other_wan_unit]);
				}
			}

			wan_sbstate = nvram_get_int(nvram_sbstate[current_wan_unit]);

			if(conn_state[current_wan_unit] == PHY_RECONN){
				conn_changed_state[current_wan_unit] = PHY_RECONN;

				conn_state_old[current_wan_unit] = DISCONN;

				// When the current line is re-plugged and the other line has plugged, the count has to be reset.
				if(get_dualwan_by_unit(other_wan_unit) != WANS_DUALWAN_IF_NONE && link_wan[other_wan_unit]){
					_dprintf("# wanduck: set S_COUNT: PHY_RECONN.\n");
					set_disconn_count(current_wan_unit, S_COUNT);
				}
			}
#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2) || defined(RTCONFIG_UBIFS)
			else if(wan_sbstate == WAN_STOPPED_REASON_DATALIMIT){
				if(conn_state_old[current_wan_unit] == CONNED){
					_dprintf("wanduck(%d)(fb): detect the data limit.\n", current_wan_unit);
					conn_changed_state[current_wan_unit] = C2D;
				}
				else
					conn_changed_state[current_wan_unit] = DISCONN;

				conn_state_old[current_wan_unit] = DISCONN;

				if(get_dualwan_by_unit(other_wan_unit) != WANS_DUALWAN_IF_NONE
						&& link_wan[other_wan_unit]
						&& nvram_get_int(nvram_sbstate[other_wan_unit]) != WAN_STOPPED_REASON_DATALIMIT)
					set_disconn_count(current_wan_unit, max_disconn_count[current_wan_unit]);
				else
					set_disconn_count(current_wan_unit, S_IDLE);
			}
#endif
#ifdef RTCONFIG_USB_MODEM
			else if(conn_state[current_wan_unit] == SET_PIN){
				conn_changed_state[current_wan_unit] = SET_PIN;

				conn_state_old[current_wan_unit] = DISCONN;
				// The USB modem needs the PIN code to unlock.
				set_disconn_count(current_wan_unit, S_IDLE);
			}
			else if(conn_state[current_wan_unit] == SET_USBSCAN){
				conn_changed_state[current_wan_unit] = C2D;

				conn_state_old[current_wan_unit] = DISCONN;
				// The USB modem is scanning the stations.
				set_disconn_count(current_wan_unit, S_IDLE);
			}
#if 0
#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2) || defined(RTCONFIG_UBIFS)
			else if(dualwan_unit__usbif(current_wan_unit) && wan_sbstate == WAN_STOPPED_REASON_DATALIMIT){
				if(conn_state_old[current_wan_unit] == CONNED){
					_dprintf("wanduck(%d)(fb): detect the data limit.\n", current_wan_unit);
					conn_changed_state[current_wan_unit] = C2D;
				}
				else
					conn_changed_state[current_wan_unit] = DISCONN;

				conn_state_old[current_wan_unit] = DISCONN;
				//set_disconn_count(current_wan_unit, max_disconn_count[current_wan_unit]);
				set_disconn_count(current_wan_unit, S_IDLE);
			}
#endif
#endif
#endif
			else if(conn_state[current_wan_unit] == CONNED){
				if(conn_state_old[current_wan_unit] == DISCONN)
					conn_changed_state[current_wan_unit] = D2C;
				else
					conn_changed_state[current_wan_unit] = CONNED;

				conn_state_old[current_wan_unit] = conn_state[current_wan_unit];
				set_disconn_count(current_wan_unit, S_IDLE);
			}
			else if(conn_state[current_wan_unit] == DISCONN){
				if(conn_state_old[current_wan_unit] == CONNED)
					conn_changed_state[current_wan_unit] = C2D;
				else
					conn_changed_state[current_wan_unit] = DISCONN;

				conn_state_old[current_wan_unit] = conn_state[current_wan_unit];

				if(disconn_case[current_wan_unit] == CASE_THESAMESUBNET){
					_dprintf("# wanduck: set S_IDLE: CASE_THESAMESUBNET.\n");
					set_disconn_count(current_wan_unit, S_IDLE);
				}
#ifdef RTCONFIG_USB_MODEM
				// when the other line is modem and not plugged, the current disconnected line would not count.
				else if(!link_wan[other_wan_unit] && dualwan_unit__usbif(other_wan_unit))
					set_disconn_count(current_wan_unit, S_IDLE);
#endif
				else if(current_state[current_wan_unit] != WAN_STATE_DISABLED
						&& get_dualwan_by_unit(other_wan_unit) != WANS_DUALWAN_IF_NONE){
					if (get_disconn_count(current_wan_unit) == S_IDLE)
						set_disconn_count(current_wan_unit, S_COUNT);
				}
				// when auth failed, the single disconnected line would not count.
				else if(disconn_case[current_wan_unit] == CASE_PPPFAIL && wan_sbstate == WAN_STOPPED_REASON_PPP_AUTH_FAIL)
					set_disconn_count(current_wan_unit, S_IDLE);
			}

			if(other_wan_unit == WAN_FB_UNIT){
				if(conn_state[other_wan_unit] == CONNED
						&& get_disconn_count(other_wan_unit) == S_IDLE
						)
					set_disconn_count(other_wan_unit, S_COUNT);
				else if(conn_state[other_wan_unit] == DISCONN)
					set_disconn_count(other_wan_unit, S_IDLE);
			}
			else
				set_disconn_count(other_wan_unit, S_IDLE);

			if(get_disconn_count(current_wan_unit) != S_IDLE){
				if(get_disconn_count(current_wan_unit) < max_disconn_count[current_wan_unit]){
					set_disconn_count(current_wan_unit, get_disconn_count(current_wan_unit)+1);
					_dprintf("# wanduck(%d): wait time for switching: %d/%d.\n", current_wan_unit, get_disconn_count(current_wan_unit)*scan_interval, max_wait_time[current_wan_unit]);
				}
				else{
					_dprintf("# wanduck(%d): set S_COUNT: changed_count[] >= max_disconn_count.\n", current_wan_unit);
				}
			}

			if(get_disconn_count(other_wan_unit) != S_IDLE){
				if(get_disconn_count(other_wan_unit) < max_fb_count){
					set_disconn_count(other_wan_unit, get_disconn_count(other_wan_unit)+1);
					_dprintf("# wanduck: wait time for returning: %d/%d.\n", get_disconn_count(other_wan_unit)*scan_interval, max_fb_wait_time);
				}
				else{
					_dprintf("# wanduck: set S_COUNT: changed_count[] >= max_fb_count.\n");
					set_disconn_count(other_wan_unit, S_COUNT);
				}
			}

			record_conn_status(current_wan_unit);
		}
		else
#else // RTCONFIG_DUALWAN
		if(is_router_mode()
#ifdef RTCONFIG_WIRELESSREPEATER
				|| sw_mode == SW_MODE_HOTSPOT
#endif
				){
			// To check the phy connection of the standby line.
#ifdef RTCONFIG_USB_MODEM
			for(wan_unit = WAN_UNIT_FIRST; wan_unit < WAN_UNIT_MAX; ++wan_unit)
#else
			wan_unit = WAN_UNIT_FIRST;
#endif
			{
				if(get_dualwan_by_unit(wan_unit) != WANS_DUALWAN_IF_NONE)
					conn_state[wan_unit] = if_wan_phyconnected(wan_unit);
			}

			current_wan_unit = wan_primary_ifunit();
#ifdef RTCONFIG_USB_MODEM
			other_wan_unit = get_next_unit(current_wan_unit);
#endif

			current_state[current_wan_unit] = nvram_get_int(nvram_state[current_wan_unit]);
if(test_log)
_dprintf("wanduck(%d)(phy): state %d, state_old %d, changed %d, wan_state %d.\n"
		, current_wan_unit, conn_state[current_wan_unit], conn_state_old[current_wan_unit], conn_changed_state[current_wan_unit], current_state[current_wan_unit]);

#ifdef RTCONFIG_USB_MODEM
			if(conn_state[current_wan_unit] == CONNED && conn_state_old[current_wan_unit] == PHY_RECONN)
				conn_state[current_wan_unit] = PHY_RECONN;
#endif

			if(current_state[current_wan_unit] == WAN_STATE_DISABLED){
				disconn_case[current_wan_unit] = CASE_OTHERS;
				conn_state[current_wan_unit] = DISCONN;
				set_disconn_count(current_wan_unit, S_IDLE);
			}
#ifdef RTCONFIG_USB_MODEM
			else if(dualwan_unit__usbif(current_wan_unit)
					&& (modem_act_reset == 1 || modem_act_reset == 2)
					){
_dprintf("wanduck(%d): detect the modem to be reset...\n", current_wan_unit);
				disconn_case[current_wan_unit] = CASE_OTHERS;
				conn_state[current_wan_unit] = DISCONN;
				set_disconn_count(current_wan_unit, S_IDLE);
			}
#endif
			else{
				if(conn_state[current_wan_unit] == CONNED){
#ifdef RTCONFIG_USB_MODEM
					if(!(dualwan_unit__usbif(current_wan_unit) && current_state[current_wan_unit] == WAN_STATE_INITIALIZING))
#endif
						conn_state[current_wan_unit] = if_wan_connected(current_wan_unit);
if(test_log)
_dprintf("wanduck(%d)(conn): state %d, state_old %d, changed %d, wan_state %d.\n"
		, current_wan_unit, conn_state[current_wan_unit], conn_state_old[current_wan_unit], conn_changed_state[current_wan_unit], current_state[current_wan_unit]);
				}
			}

			wan_sbstate = nvram_get_int(nvram_sbstate[current_wan_unit]);

			if(conn_state[current_wan_unit] == PHY_RECONN){
				conn_changed_state[current_wan_unit] = PHY_RECONN;

				conn_state_old[current_wan_unit] = DISCONN;

#ifdef RTCONFIG_USB_MODEM
				// When the current line is re-plugged and the other line has plugged, the count has to be reset.
				if(link_wan[other_wan_unit]){
					_dprintf("# wanduck: set S_COUNT: PHY_RECONN.\n");
					set_disconn_count(current_wan_unit, S_COUNT);
				}
#endif
			}
#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2) || defined(RTCONFIG_UBIFS)
			else if(wan_sbstate == WAN_STOPPED_REASON_DATALIMIT){
				if(conn_state_old[current_wan_unit] == CONNED){
					_dprintf("wanduck(usb): detect the data limit.\n");
					conn_changed_state[current_wan_unit] = C2D;
				}
				else
					conn_changed_state[current_wan_unit] = DISCONN;

				conn_state_old[current_wan_unit] = DISCONN;

#ifdef RTCONFIG_USB_MODEM
				if(link_wan[other_wan_unit]
						&& nvram_get_int(nvram_sbstate[other_wan_unit]) != WAN_STOPPED_REASON_DATALIMIT)
					set_disconn_count(current_wan_unit, max_disconn_count[current_wan_unit]);
				else
#endif
					set_disconn_count(current_wan_unit, S_IDLE);
			}
#endif
#ifdef RTCONFIG_USB_MODEM
			else if(conn_state[current_wan_unit] == SET_PIN){
				conn_changed_state[current_wan_unit] = SET_PIN;

				conn_state_old[current_wan_unit] = DISCONN;
				// The USB modem needs the PIN code to unlock.
				set_disconn_count(current_wan_unit, S_IDLE);
			}
			else if(conn_state[current_wan_unit] == SET_USBSCAN){
				conn_changed_state[current_wan_unit] = C2D;

				conn_state_old[current_wan_unit] = DISCONN;
				// The USB modem is scanning the stations.
				set_disconn_count(current_wan_unit, S_IDLE);
			}
#if 0
#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2) || defined(RTCONFIG_UBIFS)
			else if(dualwan_unit__usbif(current_wan_unit) && wan_sbstate == WAN_STOPPED_REASON_DATALIMIT){
				if(conn_state_old[current_wan_unit] == CONNED){
					_dprintf("wanduck(usb): detect the data limit.\n");
					conn_changed_state[current_wan_unit] = C2D;
				}
				else
					conn_changed_state[current_wan_unit] = DISCONN;

				conn_state_old[current_wan_unit] = DISCONN;
				//set_disconn_count(current_wan_unit, max_disconn_count[current_wan_unit]);
				set_disconn_count(current_wan_unit, S_IDLE);
			}
#endif
#endif
#endif
			else if(conn_state[current_wan_unit] == CONNED){
				if(conn_state_old[current_wan_unit] == DISCONN)
					conn_changed_state[current_wan_unit] = D2C;
				else
					conn_changed_state[current_wan_unit] = CONNED;

				conn_state_old[current_wan_unit] = conn_state[current_wan_unit];

#ifdef RTCONFIG_DSL /* Paul add 2013/7/29, for Non-DualWAN 3G/4G WAN -> DSL WAN, auto Fail-Back feature */
#ifdef RTCONFIG_USB_MODEM
				if (nvram_match("dsltmp_adslsyncsts","up") && current_wan_unit == WAN_UNIT_SECOND){
					_dprintf("\n# wanduck: adslsync up.\n");
					set_disconn_count(current_wan_unit, S_IDLE);
					link_wan[current_wan_unit] = 0;
					conn_state[current_wan_unit] = DISCONN;
					usb_switched_back_dsl = 1;
					max_disconn_count[current_wan_unit] = 1;
				}
				else
					set_disconn_count(current_wan_unit, S_IDLE);
#endif
#else
				set_disconn_count(current_wan_unit, S_IDLE);
#endif
			}
			else if(conn_state[current_wan_unit] == DISCONN){
				if(conn_state_old[current_wan_unit] == CONNED)
					conn_changed_state[current_wan_unit] = C2D;
				else
					conn_changed_state[current_wan_unit] = DISCONN;

				conn_state_old[current_wan_unit] = conn_state[current_wan_unit];

				if(disconn_case[current_wan_unit] == CASE_THESAMESUBNET){
					_dprintf("# wanduck: set S_IDLE: CASE_THESAMESUBNET.\n");
					set_disconn_count(current_wan_unit, S_IDLE);
				}
#ifdef RTCONFIG_USB_MODEM
				// when the other line is modem and not plugged, the current disconnected line would not count.
				else if(!link_wan[other_wan_unit] && dualwan_unit__usbif(other_wan_unit))
					set_disconn_count(current_wan_unit, S_IDLE);
				else if(get_disconn_count(current_wan_unit) == S_IDLE && current_state[current_wan_unit] != WAN_STATE_DISABLED)
					set_disconn_count(current_wan_unit, S_COUNT);
#else
				// when auth failed, the single disconnected line would not count.
				else if(disconn_case[current_wan_unit] == CASE_PPPFAIL && wan_sbstate == WAN_STOPPED_REASON_PPP_AUTH_FAIL)
					set_disconn_count(current_wan_unit, S_IDLE);
#endif
			}

			if(get_disconn_count(current_wan_unit) != S_IDLE){
				if(get_disconn_count(current_wan_unit) < max_disconn_count[current_wan_unit]){
					set_disconn_count(current_wan_unit, get_disconn_count(current_wan_unit)+1);
					_dprintf("# wanduck(%d): wait time for switching: %d/%d.\n", current_wan_unit, get_disconn_count(current_wan_unit)*scan_interval, max_wait_time[current_wan_unit]);
				}
				else{
					_dprintf("# wanduck(%d): set S_COUNT: changed_count[] >= max_disconn_count.\n", current_wan_unit);
				}
			}

			record_conn_status(current_wan_unit);
if(test_log)
_dprintf("wanduck(%d)(change): state %d, state_old %d, changed %d, wan_state %d.\n"
		, current_wan_unit, conn_state[current_wan_unit], conn_state_old[current_wan_unit], conn_changed_state[current_wan_unit], current_state[current_wan_unit]);
		}
		else
#endif // RTCONFIG_DUALWAN
		{ // sw_mode == SW_MODE_AP, SW_MODE_REPEATER.
			current_wan_unit = WAN_UNIT_FIRST;
			conn_state[current_wan_unit] = if_wan_phyconnected(current_wan_unit);

#if defined(RTCONFIG_DSL)
			if(conn_state[current_wan_unit] == CONNED){
				if (delay_dns_response(current_wan_unit) > 0)
					set_link_internet(current_wan_unit, 2);
				else
					set_link_internet(current_wan_unit, 1);
			}
#endif

			if(conn_state[current_wan_unit] == DISCONN){
				if(conn_state_old[current_wan_unit] == CONNED)
					conn_changed_state[current_wan_unit] = C2D;
				else
					conn_changed_state[current_wan_unit] = DISCONN;
			}
			else{
				if(conn_state_old[current_wan_unit] == DISCONN)
					conn_changed_state[current_wan_unit] = D2C;
				else
					conn_changed_state[current_wan_unit] = CONNED;
			}

			conn_state_old[current_wan_unit] = conn_state[current_wan_unit];
		}
if(test_log)
_dprintf("wanduck(%d)(all   end): state %d, state_old %d, changed %d, wan_state %d.\n"
		, current_wan_unit, conn_state[current_wan_unit], conn_state_old[current_wan_unit], conn_changed_state[current_wan_unit], current_state[current_wan_unit]);

#ifdef RTCONFIG_DUALWAN
		if(is_router_mode() && !strcmp(dualwan_mode, "lb")){
#ifdef RTCONFIG_DSL_REMOTE
			int internet_led = 0;
			for(wan_unit = WAN_UNIT_FIRST; wan_unit < WAN_UNIT_MAX; ++wan_unit){
				if(is_wan_connect(wan_unit))	//since not update current_state[wan_unit] in USB modem case
					internet_led = 1;
			}
			if(internet_led && !inhibit_led_on()) {
				led_control(LED_WAN, LED_ON);
			}
			else {
				led_control(LED_WAN, LED_OFF);
			}
#endif // RTCONFIG_DSL

			if(cross_state == DISCONN || isFirstUse){
				if(!rule_setup){
					if(isFirstUse)
						_dprintf("\n# LB: Enable direct rule(isFirstUse)\n");
					else
						_dprintf("\n# LB: Enable direct rule\n");
					rule_setup = 1;
					set_link_internet(wan_unit, 1);
				}
				nat_state = stop_nat_rules();
			}
			//else if(cross_state == CONNED && !isFirstUse){
			else{
				if(rule_setup){
					_dprintf("\n# LB: Disable direct rule\n");
					rule_setup = 0;
					set_link_internet(wan_unit, 2);
				}
				nat_state = start_nat_rules();
			}
		}
		else
#endif // RTCONFIG_DUALWAN
#ifdef RTCONFIG_WIRELESSREPEATER
#ifdef RTCONFIG_REALTEK
/* [MUST]: Need to disscuss to add new mode for Media Bridge */
		if(repeater_mode() || mediabridge_mode())
#else
		if(sw_mode == SW_MODE_REPEATER)
#endif
		{
#ifdef RTCONFIG_RESTRICT_GUI
			char word[PATH_MAX], *next_word;
#endif

			if(!got_notify)
				; // do nothing.
			else if(conn_changed_state[current_wan_unit] == DISCONN || conn_changed_state[current_wan_unit] == C2D || isFirstUse){
				//if(!rule_setup){
					if(conn_changed_state[current_wan_unit] == DISCONN)
						_dprintf("\n# mode(%d): Enable direct rule(DISCONN)\n", sw_mode);
					else if(conn_changed_state[current_wan_unit] == C2D)
						_dprintf("\n# mode(%d): Enable direct rule(C2D)\n", sw_mode);
					else
						_dprintf("\n# mode(%d): Enable direct rule(isFirstUse)\n", sw_mode);
					rule_setup = 1;

					eval("ebtables", "-t", "broute", "-F");
					eval("ebtables", "-t", "filter", "-F");

					// Drop the DHCP server from PAP.
#ifdef RTCONFIG_CONCURRENTREPEATER
					if (nvram_get_int("wlc_express") == 0) {	/* concurrent repeater */
						eval("ebtables", "-t", "filter", "-A", "FORWARD", "-i", nvram_safe_get(wl_nvname("ifname", 0, 1)), "-j", "DROP");
						eval("ebtables", "-t", "filter", "-A", "FORWARD", "-i", nvram_safe_get(wl_nvname("ifname", 1, 1)), "-j", "DROP");
					}
					else
#endif
					eval("ebtables", "-t", "filter", "-A", "FORWARD", "-i", nvram_safe_get(wlc_nvname("ifname")), "-j", "DROP");
					f_write_string("/proc/net/dnsmqctrl", "", 0, 0);

#ifdef RTCONFIG_RESTRICT_GUI
					if(nvram_get_int("fw_restrict_gui")){
						foreach(word, nvram_safe_get("lan_ifnames"), next_word){
							SKIP_ABSENT_FAKE_IFACE(word);
							if(!strncmp(word, "vlan", 4))
								goto WANDUCK_SELECT;

							eval("ebtables", "-t", "broute", "-A", "BROUTING", "-i", word, "-j", "mark", "--mark-set", BIT_RES_GUI, "--mark-target", "ACCEPT");
						}

						repeater_filter_setting(0);
					}
#endif
_dprintf("nat_rule: stop_nat_rules 6.\n");
					nat_state = stop_nat_rules();
				//}
				got_notify = 0;
			}
			else{
				//if(rule_setup && !isFirstUse)
				if(!isFirstUse)
				{
					_dprintf("\n# mode(%d): Disable direct rule(CONNED)\n", sw_mode);
					rule_setup = 0;
					eval("ebtables", "-t", "broute", "-F");
					eval("ebtables", "-t", "filter", "-F");

#ifdef RTCONFIG_RESTRICT_GUI
					if(nvram_get_int("fw_restrict_gui")){
						foreach(word, nvram_safe_get("lan_ifnames"), next_word){
							SKIP_ABSENT_FAKE_IFACE(word);
							if(!strncmp(word, "vlan", 4))
								goto WANDUCK_SELECT;

							eval("ebtables", "-t", "broute", "-A", "BROUTING", "-i", word, "-j", "mark", "--mark-set", BIT_RES_GUI, "--mark-target", "CONTINUE");
						}

						repeater_filter_setting(1);
					}
#endif

#if !defined(RTCONFIG_QCA) && !defined(RTCONFIG_CONCURRENTREPEATER) 
					eval("ebtables", "-t", "broute", "-A", "BROUTING", "-d", "00:E0:11:22:33:44", "-j", "redirect", "--redirect-target", "DROP");
					snprintf(domain_mapping, sizeof(domain_mapping), "%x %s", inet_addr(nvram_safe_get("lan_ipaddr")), DUT_DOMAIN_NAME);
					f_write_string("/proc/net/dnsmqctrl", domain_mapping, 0, 0);
#endif

#ifndef RTCONFIG_LANTIQ
#ifdef RTCONFIG_REDIRECT_DNAME
					if (nvram_invmatch("redirect_dname", "0"))
						eval("ebtables", "-t", "broute", "-A", "BROUTING", "-p", "ipv4", "--ip-proto", "udp", "--ip-dport", "53", "-j", "redirect", "--redirect-target", "DROP");
#endif
#endif

#ifdef RTCONFIG_WIFI_SON
                                	if((sw_mode() == SW_MODE_AP && nvram_match("cfg_master", "1")) && nvram_match("wifison_ready", "1"))
                                	{
                                       		if(nvram_get_int("wl0.1_bss_enabled"))
                                        	{
#if defined(RTCONFIG_HIDDEN_BACKHAUL)
                                                	eval("ebtables", "-t", "broute", "-I", "BROUTING","-i",vif ,"-j","ACCEPT");
#else
                                                	eval("ebtables", "-t", "broute", "-I", "BROUTING","-i","ath1.55" ,"-j","ACCEPT");
#endif                                                	
							eval("ebtables", "-t", "broute", "-I", "BROUTING","-i",nvram_safe_get("wl0.1_ifname"),"-j","ACCEPT");
                                        	}
                                	}
#endif

_dprintf("nat_rule: start_nat_rules 6.\n");
					nat_state = start_nat_rules();
#if defined(RTCONFIG_CONCURRENTREPEATER) && defined(RTCONFIG_RALINK)
					nvram_set_int("lan_ready",1);
#endif						
				}

				got_notify = 0;
			}
		}
		else
#endif // RTCONFIG_WIRELESSREPEATER
		if(sw_mode == SW_MODE_AP){
#ifdef RTCONFIG_REDIRECT_DNAME
			if (nvram_invmatch("redirect_dname", "0") && access_point_mode()) {

			if (conn_changed_state[current_wan_unit] == C2D) {
				int evalRet;
				_dprintf("\n# AP mode: Enable direct rule(C2D)\n");
				eval("ebtables", "-t", "broute", "-F");
				eval("ebtables", "-t", "filter", "-F");
				redirect_setting();
				evalRet = eval("iptables-restore", REDIRECT_RULES);
				rule_apply_checking("wanduck", __LINE__, REDIRECT_RULES, evalRet);
				// nat_rules = NAT_STATE_REDIRECT;
			}
#ifdef RTCONFIG_WIFI_SON
			else if(!nvram_match("cfg_master", "1") && nvram_match("wifison_ready", "1"))
				; // do nothing.
#endif
			else if (conn_changed_state[current_wan_unit] == D2C) {
				int evalRet;
				_dprintf("\n# AP mode: Disable direct rule(D2C)\n");
				eval("ebtables", "-t", "broute", "-F");
				eval("ebtables", "-t", "filter", "-F");
				eval("ebtables", "-t", "broute", "-I", "BROUTING", "-p", "ipv4", "--ip-proto", "udp", "--ip-dport", "53", "-j", "redirect", "--redirect-target", "DROP");
#ifdef RTCONFIG_WIFI_SON
                                if((sw_mode() == SW_MODE_AP && nvram_match("cfg_master", "1"))&& nvram_match("wifison_ready", "1"))
                                {
                                        if(nvram_get_int("wl0.1_bss_enabled"))
                                        {
#if defined(RTCONFIG_HIDDEN_BACKHAUL)
                                                eval("ebtables", "-t", "broute", "-I", "BROUTING","-i",vif ,"-j","ACCEPT");
#else
                                                eval("ebtables", "-t", "broute", "-I", "BROUTING","-i","ath1.55" ,"-j","ACCEPT");
#endif
                                                eval("ebtables", "-t", "broute", "-I", "BROUTING","-i",nvram_safe_get("wl0.1_ifname"),"-j","ACCEPT");
                                        }
                                }
#endif
				redirect_nat_setting();
				evalRet = eval("iptables-restore", NAT_RULES);
				rule_apply_checking("wanduck", __LINE__, NAT_RULES, evalRet);
				// nat_rules = NAT_STATE_NORMAL;
			}

			}
#else
			; // do nothing.
#endif

#ifdef RTCONFIG_RESTRICT_GUI
			if(conn_changed_state[current_wan_unit] == D2C){
				if(nvram_get_int("fw_restrict_gui")){
					char word[PATH_MAX], *next_word;

					foreach(word, nvram_safe_get("lan_ifnames"), next_word){
						SKIP_ABSENT_FAKE_IFACE(word);
						if(!strncmp(word, "vlan", 4))
							goto WANDUCK_SELECT;

						eval("ebtables", "-t", "broute", "-I", "BROUTING", "-i", word, "-j", "mark", "--mark-set", BIT_RES_GUI, "--mark-target", "CONTINUE");
					}

					repeater_filter_setting(0);
				}
			}
#endif
		}
		else if(conn_changed_state[current_wan_unit] == C2D || (conn_changed_state[current_wan_unit] == CONNED && isFirstUse)){
			if(!rule_setup){
				if(conn_changed_state[current_wan_unit] == C2D){
#if defined(RTCONFIG_WPS_ALLLED_BTN) || defined(RTCONFIG_DSL)
					led_control(LED_WAN, LED_OFF);
#elif defined(RTCONFIG_HND_ROUTER_AX) || defined(RTCONFIG_LANWAN_LED) || defined(RTCONFIG_WANRED_LED) || defined(RTCONFIG_FAILOVER_LED)
					update_wan_leds(current_wan_unit, link_wan[current_wan_unit]);
#elif defined(RTAC68U) || defined(RTAC87U) || defined(RTAC3200) || defined(RTCONFIG_BCM_7114) || defined(HND_ROUTER)
					if(
#ifdef RTAC68U
						(is_ac66u_v2_series() || is_ac68u_v3_series())
#else
						1
#endif // RTAC68U
#ifdef RTCONFIG_DUALWAN
						&& (strcmp(dualwan_mode, "fb") == 0 ||
							 strcmp(dualwan_mode, "fo") == 0)
#endif // RTCONFIG_DUALWAN
					){
						logmessage("DualWAN", "skip single wan wan_led_control - WANRED off\n");
						if (nvram_match("AllLED", "1")) {
							led_control(LED_WAN, LED_ON);
							disable_wan_led();
						}
					}
#endif

					_dprintf("\n# Enable direct rule(C2D)......\n");
				}
				else
					_dprintf("\n# Enable direct rule(isFirstUse)\n");
				rule_setup = 1;

				if(conn_changed_state[current_wan_unit] == C2D){
#ifdef RTCONFIG_USB_MODEM
					// the current line is USB and have been plugged off.
					if(!link_wan[current_wan_unit] && dualwan_unit__usbif(current_wan_unit)){
						if((modem_unit = get_modemunit_by_type(get_dualwan_by_unit(current_wan_unit))) == MODEM_UNIT_NONE){
							_dprintf("%s 4: cannot get the modem unit!\n", __FUNCTION__);
							//goto WANDUCK_SELECT;
						}
						else{
							_dprintf("%s: wanduck clean_modem_state(%d)!\n", __FUNCTION__, modem_unit);
							clean_modem_state(modem_unit, 2);
						}

						if(get_dualwan_by_unit(other_wan_unit) != WANS_DUALWAN_IF_NONE){
							_dprintf("\n# wanduck(C2D): Modem was plugged off and try to Switch the other line.\n");
							switch_wan_line(other_wan_unit, 0);
						}
						else if(current_state[current_wan_unit] != WAN_STATE_INITIALIZING){
							_dprintf("wanduck3: stop_wan_if %d.\n", current_wan_unit);
							snprintf(cmd, sizeof(cmd), "stop_wan_if %d", current_wan_unit);
							notify_rc(cmd);
						}

#ifdef RTCONFIG_DSL /* Paul add 2013/7/29, for Non-DualWAN 3G/4G WAN -> DSL WAN, auto Fail-Back feature */
#ifndef RTCONFIG_DUALWAN
						if(nvram_match("dsltmp_adslsyncsts","up") && usb_switched_back_dsl == 1){
							_dprintf("\n# wanduck: usb_switched_back_dsl: 1.\n");
							link_wan[WAN_UNIT_SECOND] = 1;
							max_disconn_count[WAN_UNIT_SECOND] = max_wait_time[WAN_UNIT_SECOND]/scan_interval;
							usb_switched_back_dsl = 0;
						}
#endif
#endif
					}
//					else
#endif // RTCONFIG_USB_MODEM
#if 0
					// C2D: Try to prepare the backup line.
					if(link_wan[other_wan_unit] == 1 && !is_wan_connect(other_wan_unit)){
						_dprintf("\n# wanduck(C2D): Try to prepare the backup line.\n");
						snprintf(cmd, sizeof(cmd), "restart_wan_if %d", other_wan_unit);
						notify_rc(cmd);
					}
#endif
				}
			}

			handle_wan_line(current_wan_unit, rule_setup);
		}
		else if(conn_changed_state[current_wan_unit] == D2C || conn_changed_state[current_wan_unit] == CONNED){
			if(rule_setup && !isFirstUse){
#if (defined(RTCONFIG_WPS_ALLLED_BTN) && !defined(RTCONFIG_HND_ROUTER_AX))|| \
    ((defined(RTCONFIG_LED_BTN) || defined(RTCONFIG_TURBO_BTN)) && defined(RTCONFIG_QCA))
				if (!inhibit_led_on())
					led_control(LED_WAN, LED_ON);
				else
					led_control(LED_WAN, LED_OFF);
#elif defined(DSL_N55U) || defined(DSL_N55U_B)
				led_control(LED_WAN, LED_ON);
#elif defined(RTCONFIG_HND_ROUTER_AX) || defined(RTCONFIG_LANWAN_LED) || defined(RTCONFIG_WANRED_LED) || defined(RTCONFIG_FAILOVER_LED)
				update_wan_leds(current_wan_unit, link_wan[current_wan_unit]);
#elif defined(RTAC68U) || defined(RTAC87U) || defined(RTAC3200) || defined(RTCONFIG_BCM_7114) || defined(HND_ROUTER)
				if(nvram_match("AllLED", "1")
#ifdef RTAC68U
						&& (is_ac66u_v2_series() || is_ac68u_v3_series())
#endif
						)
					enable_wan_led();
#endif

				_dprintf("\n# Disable direct rule(D2C)......\n");
				rule_setup = 0;

				handle_wan_line(current_wan_unit, rule_setup);
			}
		}
		/*
		 * when all lines are plugged in and the currect line is disconnected over max_wait_time seconds,
		 * switch the connect to the other line.
		 */
		else if(conn_changed_state[current_wan_unit] == DISCONN){
#if defined(RTCONFIG_DUALWAN) || defined(RTCONFIG_USB_MODEM)
			if(get_dualwan_by_unit(other_wan_unit) != WANS_DUALWAN_IF_NONE
#ifdef RTCONFIG_USB_MULTIMODEM
					&& (dualwan_unit__nonusbif(other_wan_unit) || link_wan[other_wan_unit])
#endif
					&& (get_disconn_count(current_wan_unit) >= max_disconn_count[current_wan_unit]
#ifdef RTCONFIG_USB_MODEM
							|| (dualwan_unit__usbif(current_wan_unit) && !link_wan[current_wan_unit])
#endif
							)
					)
			{
				_dprintf("# wanduck(%d): Switching the connect to the %d WAN line...\n", current_wan_unit, get_next_unit(current_wan_unit));
				set_disconn_count(current_wan_unit, S_IDLE);;
				if(!link_wan[current_wan_unit] && dualwan_unit__usbif(current_wan_unit))
					switch_wan_line(other_wan_unit, 0);
				else
					switch_wan_line(other_wan_unit, 1);
			}
			else
#endif // RTCONFIG_DUALWAN || RTCONFIG_USB_MODEM
#ifdef RTCONFIG_AUTOCOVER_SIP
			if(disconn_case[current_wan_unit] == CASE_THESAMESUBNET && isFirstUse && nvram_get_int("atcover_sip") == 1){
#if 1
				struct in_addr addr;
				in_addr_t new_addr;

				if (inet_deconflict(current_lan_ipaddr, current_lan_netmask,
						    current_lan_ipaddr, current_lan_netmask, &addr)) {
					nvram_set("lan_ipaddr", inet_ntoa(addr));
					nvram_set("lan_ipaddr_rt", inet_ntoa(addr));

					new_addr = ntohl(addr.s_addr);
					addr.s_addr = htonl(new_addr + 1);
					nvram_set("dhcp_start", inet_ntoa(addr));
					addr.s_addr = htonl((new_addr | ~inet_network(current_lan_netmask)) & 0xfffffffe);
					nvram_set("dhcp_end", inet_ntoa(addr));

					notify_rc_and_wait("restart_net_and_phy");
				}
#else
				/* nb: it does the commit, code above - not */
				notify_rc_and_wait("restart_subnet");
#endif
			}
			else
#endif // RTCONFIG_AUTOCOVER_SIP
			/* recover redirect rules if overwritten by ppp+dhcp/zeroconf/etc */
			//if(disconn_case[current_wan_unit] == CASE_PPPFAIL && nat_state == NAT_STATE_REDIRECT &&
			//	nvram_get_int("nat_state") == NAT_STATE_NORMAL) {
			if(disconn_case[current_wan_unit] == CASE_PPPFAIL){
_dprintf("nat_rule: stop_nat_rules 7.\n");
				nat_state = stop_nat_rules();
			}

#if defined(RTCONFIG_HND_ROUTER_AX) || defined(RTCONFIG_LANWAN_LED) || defined(RTCONFIG_WANRED_LED) || defined(RTCONFIG_FAILOVER_LED)
			update_wan_leds(current_wan_unit, link_wan[current_wan_unit]);
#endif
		}
		// phy connected -> disconnected -> connected
		else if(conn_changed_state[current_wan_unit] == PHY_RECONN){
#ifdef RTCONFIG_USB_MODEM
			if(dualwan_unit__usbif(current_wan_unit)){
				if(current_state[current_wan_unit] == WAN_STATE_INITIALIZING
						|| (nvram_get_int(nvram_state[current_wan_unit]) == WAN_STATE_STOPPED && nvram_get_int(nvram_sbstate[current_wan_unit]) == WAN_STOPPED_REASON_NONE)){
					if((modem_unit = get_modemunit_by_type(get_dualwan_by_unit(current_wan_unit))) == MODEM_UNIT_NONE){
						_dprintf("%s 5: cannot get the modem unit!\n", __FUNCTION__);
						goto WANDUCK_SELECT;
					}

					usb_modem_prefix(modem_unit, prefix2, sizeof(prefix2));
					snprintf(modem_type, sizeof(modem_type), "%s", nvram_safe_get(strcat_r(prefix2, "act_type", tmp2)));

					handle_wan_line(current_wan_unit, 0);
				}
				else
					_dprintf("wanduck(%d): the modem had been run...\n", current_wan_unit);
			}
			else
#endif
				handle_wan_line(current_wan_unit, 0);

			conn_changed_state[current_wan_unit] = conn_state[current_wan_unit];

#if defined(RTCONFIG_HND_ROUTER_AX) || defined(RTCONFIG_LANWAN_LED) || defined(RTCONFIG_WANRED_LED) || defined(RTCONFIG_FAILOVER_LED)
			update_wan_leds(current_wan_unit, link_wan[current_wan_unit]);
#endif
		}

#ifdef RTCONFIG_DUALWAN
		if(!strcmp(dualwan_mode, "fb") && other_wan_unit == WAN_FB_UNIT){
#if 1
			if(conn_state[other_wan_unit] == CONNED) {
				_dprintf("# wanduck: try to detect the fail-back WAN line(%d)...\n", other_wan_unit);
				if (detect_backup_internet(other_wan_unit))
					switch_backup_line(other_wan_unit, nvram_get_int("wandog_fb_restart"));
			}
#else
			if(conn_state[other_wan_unit] == CONNED
					&& get_disconn_count(other_wan_unit) >= max_fb_count
					){
				_dprintf("# wanduck: returning to the primary WAN line(%d)...\n", other_wan_unit);
				rule_setup = 1;

				handle_wan_line(other_wan_unit, rule_setup);
				switch_wan_line(other_wan_unit, 0);
			}
#endif
			else if(conn_state[other_wan_unit] == PHY_RECONN){
				_dprintf("\n# wanduck(fail-back): Try to prepare the backup line.\n");
				snprintf(cmd, sizeof(cmd), "restart_wan_if %d", other_wan_unit);
				notify_rc(cmd);
			}
		}
		// hot-standby: Try to prepare the backup line.
		else if(!strcmp(dualwan_mode, "fo") || !strcmp(dualwan_mode, "fb")){
			if(nvram_get_int("wans_standby") == 1 && link_wan[other_wan_unit] == 1 && get_wan_state(other_wan_unit) == WAN_STATE_INITIALIZING){
				_dprintf("\n# wanduck(hot-standby): Try to prepare the backup line.\n");
				snprintf(cmd, sizeof(cmd), "restart_wan_if %d", other_wan_unit);
				notify_rc(cmd);
			}
		}

#if defined(RTAC68U) || defined(RTAC87U) || defined(RTAC3200) || defined(RTCONFIG_BCM_7114) || (defined(HND_ROUTER) && !defined(RTCONFIG_HND_ROUTER_AX))
		if (strcmp(dualwan_wans, "wan none")) {
			if(nvram_match("AllLED", "1")
#ifdef RTAC68U
				&& (is_ac66u_v2_series() || is_ac68u_v3_series())
#endif
			){
				link_status = 0;
				for (u = WAN_UNIT_FIRST; !link_status && u < WAN_UNIT_MAX; ++u) {
					if(conn_state[u] == CONNED)
						link_status++;
				}

				if(link_status) {
					if(wanred_led_status != 2 ){
						enable_wan_led();
						wanred_led_status = 2;
					}
				}else{
					if(wanred_led_status != 1 ){
						disable_wan_led();
						wanred_led_status = 1;
					}
				}
			}
		}
#endif
#endif // RTCONFIG_DUALWAN

#ifdef RTCONFIG_QTN
		if (nvram_get_int("ntp_ready") == 1 && nvram_get_int("qtn_ready") == 1){
			if (nvram_get_int("qtn_ntp_ready") == 0){
				time(&qtn_now);
				qtn_tm = localtime(&qtn_now);
				snprintf(time_string, sizeof(time_string), "%02d%02d%02d%02d%04d",
						qtn_tm->tm_mon+1, qtn_tm->tm_mday, qtn_tm->tm_hour, qtn_tm->tm_min, qtn_tm->tm_year+1900);
				eval("qcsapi_sockrpc", "run_script", "router_command.sh", "sync_time", time_string);
				nvram_set_int("qtn_ntp_ready", 1);
			}
		}
#endif

		start_demand_ppp(current_wan_unit, 1);

WANDUCK_SELECT:
		if((nready = select(maxfd+1, &rset, NULL, NULL, &tval)) <= 0)
			continue;


		if(FD_ISSET(dns_sock, &rset)){
			run_dns_serv(dns_sock);
			if(--nready <= 0)
				continue;
		}
		else if(FD_ISSET(http_sock, &rset)){
			if((cur_sockfd = accept(http_sock, (struct sockaddr *)&cliaddr, (socklen_t *)&clilen)) <= 0){
				perror("http accept");
				continue;
			}

			for(fd_i = 0; fd_i < MAX_USER; ++fd_i){
				if(client[fd_i].sfd < 0){
					client[fd_i].sfd = cur_sockfd;
					client[fd_i].type = T_HTTP;
					break;
				}
			}

			if(fd_i == MAX_USER){
				_dprintf("# wanduck servs full\n");
				close(cur_sockfd);
				continue;
			}

			FD_SET(cur_sockfd, &allset);
			if(cur_sockfd > maxfd)
				maxfd = cur_sockfd;
			if(fd_i > maxi)
				maxi = fd_i;

			if(--nready <= 0)
				continue;	// no more readable descriptors
		}

		// polling
		for(fd_i = 0; fd_i <= maxi; ++fd_i){
			if((sockfd = client[fd_i].sfd) < 0)
				continue;

			if(FD_ISSET(sockfd, &rset)){
				int nread;
				ioctl(sockfd, FIONREAD, &nread);
				if(nread == 0){
					close_socket(sockfd, T_HTTP);
					continue;
				}
				if(fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0)|O_NONBLOCK) < 0){
					_dprintf("wanduck set http req [%d] nonblock fail !\n", sockfd);
					continue;
				}

				cur_sockfd = sockfd;

				run_http_serv(sockfd);

				if(--nready <= 0)
					break;
			}
		}
	}

	_dprintf("# wanduck exit error\n");
	exit(1);
}
