#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <linux/if_packet.h>
#include <syslog.h>
#include "shared.h"

#define MAX_BUF_SIZE 2048
#define PID_FILE "/var/run/mod_snooper.pid"

#if defined(RTCONFIG_NETDEV_PER_PORT) || defined(GTBN98_PRO) || defined(GTBN96X) || defined(GTBN98) || defined(GTBN96)
#define IS_NETDEV_MACRO 1
#else
#define IS_NETDEV_MACRO 0
#endif

#if defined(GTBE98) || defined(GTBE98_PRO) || defined(GTBE96) || defined(RTBE58U) || defined(TUFBE3600) || defined(RTBE58U_V2) || defined(TUFBE3600_V2) || defined(RTBE55) || defined(GTBE19000) || defined(RTBE92U) || defined(RTBE95U) || defined(RTBE82U) || defined(TUFBE82) || defined(RTBE58U_PRO) || defined(GTBE19000AI) || defined(GTBE96_AI)
#define IS_RTK_MACRO 1
#else
#define IS_RTK_MACRO 0
#endif

#ifdef RTCONFIG_MXL_826XX
#define IS_MXL_MACRO 1
#else
#define IS_MXL_MACRO 0
#endif

int m_raw_sock;
int m_exit = 0;
char m_vendor[] = "CHT";

char isolated_mac[18] = "";
char isolated_if[IFNAMSIZ] = "";
int isolated_phy_port = -1;
#if IS_MXL_MACRO
int g_mxls_inited = 0;
int first_detected_phy_port = -1;
#endif

#if !IS_NETDEV_MACRO
#if IS_RTK_MACRO
extern int rtkswitch_find_mac_port(const char *mac_str);
extern int rtkswitch_phy_port_link_status(int phy_port);
#endif
#if IS_MXL_MACRO
extern int mxl_find_mac_port(const char *mac_str);
extern int mxl_phy_port_link_status(int phy_port);
#endif

extern int __setup_vlan(int vid, int prio, unsigned int mask);
#endif

int switch_phy_port_link_status(int phy_port)
{
#if IS_RTK_MACRO
	return rtkswitch_phy_port_link_status(phy_port);
#endif
#if IS_MXL_MACRO
	return mxl_phy_port_link_status(phy_port);
#endif
	return -1;
}

int switch_find_mac_port(const char *mac_str)
{
#if IS_RTK_MACRO
	return rtkswitch_find_mac_port(mac_str);
#endif
#if IS_MXL_MACRO
	return mxl_find_mac_port(mac_str);
#endif
	return -1;
}

#if IS_MXL_MACRO
int mod_mxls_reinit()
{

	int i;

	nvram_set("mxl_init", "-1");
	g_mxls_inited = 0;

	for (i=0; i<4; ++i) {
		if (mxls_init() == 0) { g_mxls_inited = 1; break; }
		sleep(1);
	}
	syslog(LOG_NOTICE, "mod_snooper:: performing mxl port init(%d) ..., ret = %d.", i, g_mxls_inited);
	_dprintf("mod_snooper:: performing mxl port init(%d) ..., ret = %d.", i, g_mxls_inited);

	return g_mxls_inited;
/*
	notify_rc("restart_mxls_init");
	int wait = nvram_get_int("mxls_init_wait") ? : 10;
	syslog(LOG_NOTICE, "mod_snooper: mxls_init wait %d\n", wait);
	sleep(wait);
	return 0;
*/
}
#endif

void mod_setup_vlan(int v0_prio, int v1_prio, unsigned int v0_mask, unsigned int v1_mask)
{
#if IS_RTK_MACRO
	__setup_vlan(1, v1_prio, v1_mask);
	__setup_vlan(0, v0_prio, v0_mask);
#endif
#if IS_MXL_MACRO
	mod_mxls_reinit();

	__setup_vlan(0, v0_prio, v0_mask);
	__setup_vlan(1, v1_prio, v1_mask);

	char nv_val[32];
	snprintf(nv_val, sizeof(nv_val), "0x%x", v0_mask);
	nvram_set("hinet_mesh_vlan0_mask_new", nv_val);
	snprintf(nv_val, sizeof(nv_val), "0x%x", v1_mask);
	nvram_set("hinet_mesh_vlan1_mask_new", nv_val);
/*
	notify_rc("restart_setup_vlan");
*/
#endif
}

static void init_cleanup()
{
#if IS_NETDEV_MACRO
	syslog(LOG_NOTICE, "mod_snooper:: Performing startup port cleanup (NETDEV)...");

	DIR *dir = opendir("/sys/class/net/br101/brif");
	if (!dir) return;

	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_name[0] == '.') continue;
		
		if ((strncmp(entry->d_name, "eth", 3) == 0 && entry->d_name[3] >= '1' && entry->d_name[3] <= '9') ||
		    (strncmp(entry->d_name, "lan", 3) == 0 && entry->d_name[3] >= '1' && entry->d_name[3] <= '9')) {
			
			syslog(LOG_NOTICE, "mod_snooper:: Found stranded port %s in br101, restoring to br0", entry->d_name);
			safe_do_system("brctl delif br101 %s", entry->d_name);
			safe_do_system("brctl addif br0 %s", entry->d_name);
		}
	}
	closedir(dir);
#else
	isolated_phy_port = -1;
#if IS_MXL_MACRO
	//mod_mxls_reinit();
#endif
#endif
}

#if IS_NETDEV_MACRO
static char* get_ifname_by_mac(const char *mac, char *out_ifname)
{
	FILE *fp = popen("brctl showmacs br0", "r");
	if (!fp) return NULL;

	char line[256];
	int port_no = -1;
	
	while (fgets(line, sizeof(line), fp)) {
		if (strcasestr(line, mac)) {
			sscanf(line, "%d", &port_no);
			break;
		}
	}
	pclose(fp);

	if (port_no == -1) return NULL;

	DIR *dir = opendir("/sys/class/net/br0/brif");
	if (!dir) return NULL;

	struct dirent *entry;
	out_ifname[0] = '\0';
	
	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_name[0] == '.') continue;
		
		char path[256];
		snprintf(path, sizeof(path), "/sys/class/net/br0/brif/%s/port_no", entry->d_name);
		FILE *fport = fopen(path, "r");
		if (fport) {
			int p;
			if (fscanf(fport, "0x%x", &p) == 1 && p == port_no) {
				if (strncmp(entry->d_name, "eth", 3) == 0 || strncmp(entry->d_name, "lan", 3) == 0) {
					strncpy(out_ifname, entry->d_name, IFNAMSIZ - 1);
					out_ifname[IFNAMSIZ - 1] = '\0';
				}
				fclose(fport);
				break;
			}
			fclose(fport);
		}
	}
	closedir(dir);

	return out_ifname[0] ? out_ifname : NULL;
}
#endif

#if IS_MXL_MACRO
static void flush_mxl_fdb() {
	syslog(LOG_NOTICE, "mod_snooper:: Flushing MXL FDB table to clear stale MAC entries.");
	_dprintf("mod_snooper:: Flushing MXL FDB table to clear stale MAC entries.");
	safe_do_system("fapi-GSW-MAC-TableClear");

	safe_do_system("brctl setageing br0 0");
	safe_do_system("brctl setageing br0 300");

	/* safe_do_system() has no shell, so ">/dev/null 2>&1" would be rejected as
	 * a forbidden unquoted meta-character and the whole command would not run.
	 * Redirect output explicitly via safe_do_system_to_file() instead. */
	safe_do_system_to_file("/dev/null", 0, "ethswbox fapi-GSW-MAC-TableClear");
	safe_do_system_to_file("/dev/null", 0, "ethswbox MAC_TableClear");
}
#endif

static void restore_mod_port()
{
	isolated_phy_port = -1;
	syslog(LOG_NOTICE, "mod_snooper:: don't restore ports...");
	return;

#if IS_NETDEV_MACRO
	if (isolated_if[0] == '\0') return;

	syslog(LOG_NOTICE, "mod_snooper:: Restoring port %s back to LAN (br0)...", isolated_if);

	safe_do_system("brctl delif br101 %s", isolated_if);
	safe_do_system("brctl addif br0 %s", isolated_if);

	isolated_if[0] = '\0';
#else
#if IS_MXL_MACRO
	isolated_phy_port = -1;
	//return flush_mxl_fdb();
	syslog(LOG_NOTICE, "mod_snooper:: Restoring port (fake)...");
	return;
#endif
	if (isolated_phy_port == -1) return;
	syslog(LOG_NOTICE, "mod_snooper:: Restoring PHY port index %d back to LAN", isolated_phy_port);
	_dprintf("mod_snooper:: Restoring PHY port index %d back to LAN", isolated_phy_port);
      
	unsigned int v0_mask = strtoul(nvram_safe_get("hinet_mesh_vlan0_mask"), NULL, 16);
	unsigned int v1_mask = strtoul(nvram_safe_get("hinet_mesh_vlan1_mask"), NULL, 16);
	int v0_prio = nvram_get_int("hinet_mesh_vlan0_prio");
	int v1_prio = nvram_get_int("hinet_mesh_vlan1_prio");

	mod_setup_vlan(v0_prio, v1_prio, v0_mask, v1_mask);
	isolated_phy_port = -1;
#endif
	isolated_mac[0] = '\0';
	syslog(LOG_NOTICE, "mod_snooper:: Restoration complete.");
	_dprintf("mod_snooper:: Restoration complete.");
}

static void isolate_mod_port(const char *mac, const char *ifname, int phy_port)
{
#if IS_NETDEV_MACRO
	syslog(LOG_NOTICE, "mod_snooper:: Moving interface %s to WAN (br101) for MOD MAC %s", ifname, mac);

	safe_do_system("brctl delif br0 %s", ifname);
	safe_do_system("brctl addif br101 %s", ifname);
	strncpy(isolated_if, ifname, sizeof(isolated_if));
#else
	syslog(LOG_NOTICE, "mod_snooper:: Isolating PHY port index %d for MOD MAC %s", phy_port, mac);
	_dprintf("mod_snooper:: Isolating PHY port index %d for MOD MAC %s", phy_port, mac);
       
	unsigned int v0_mask = strtoul(nvram_safe_get("hinet_mesh_vlan0_mask"), NULL, 16);
	unsigned int v1_mask = strtoul(nvram_safe_get("hinet_mesh_vlan1_mask"), NULL, 16);
	int v0_prio = nvram_get_int("hinet_mesh_vlan0_prio");
	int v1_prio = nvram_get_int("hinet_mesh_vlan1_prio");

	unsigned int port_bit = (1 << phy_port);
	unsigned int port_mask = ((port_bit & 0xFFFF) << 16) | (port_bit & 0xFFFF);
       
	v0_mask &= ~port_mask;
	v1_mask |= port_mask;

	mod_setup_vlan(v0_prio, v1_prio, v0_mask, v1_mask);
	isolated_phy_port = phy_port;
#endif
	safe_do_system("nvram set hinet_mod_mac=%s", mac);
	strncpy(isolated_mac, mac, sizeof(isolated_mac));
	syslog(LOG_NOTICE, "mod_snooper:: Isolation complete.");
	_dprintf("mod_snooper:: Isolation complete.");
}

static void check_isolated_port_status()
{
#if IS_NETDEV_MACRO
	if (isolated_if[0] == '\0') return;

	char path[128];
	snprintf(path, sizeof(path), "/sys/class/net/%s/carrier", isolated_if);
	FILE *f = fopen(path, "r");
	
	if (f) {
		int carrier = 0;
		if (fscanf(f, "%d", &carrier) == 1 && carrier == 0) {
			syslog(LOG_NOTICE, "mod_snooper:: Carrier lost (Link Down) on %s.", isolated_if);
			restore_mod_port();
		}
		fclose(f);
	} else {
		syslog(LOG_WARNING, "mod_snooper:: Cannot read %s, assuming port lost.", path);
		restore_mod_port();
	}
#else
	if (isolated_phy_port != -1) {
		if (switch_phy_port_link_status(isolated_phy_port) <= 0) {
			syslog(LOG_NOTICE, "mod_snooper:: Carrier lost on PHY port index %d.", isolated_phy_port);
			_dprintf("mod_snooper:: Carrier lost on PHY port index %d.", isolated_phy_port);
			restore_mod_port();
		}
       }
#endif
}

void handle_raw_packet()
{
	uint8_t recvBuf[MAX_BUF_SIZE] = {0};
	ssize_t data_len = recv(m_raw_sock, recvBuf, MAX_BUF_SIZE, MSG_DONTWAIT);
	if (data_len < 14) return;

	uint16_t raw_ether_type = (recvBuf[12] << 8) | recvBuf[13];
	uint8_t *l3_payload = recvBuf + 14;

	if (raw_ether_type == 0x8100) {
		if (data_len < 18) return;
		raw_ether_type = (recvBuf[16] << 8) | recvBuf[17];
		l3_payload = recvBuf + 18;
	}

	char src_mac[18] = "";
	int matched = 0;

	/* ARP Snooping & lan-conflicts protection */
	if (raw_ether_type == ETH_P_ARP) {
		if (data_len < (l3_payload - recvBuf) + 28) return;
		
		if (l3_payload[0] == 0x00 && l3_payload[1] == 0x01 &&
		    l3_payload[2] == 0x08 && l3_payload[3] == 0x00 &&
		    l3_payload[4] == 0x06 && l3_payload[5] == 0x04 &&
		    l3_payload[6] == 0x00 && l3_payload[7] == 0x01) {
			
			uint8_t *sender_ip = l3_payload + 14;
			if (sender_ip[0] == 10) { /* MOD ip: 10.x.x.x */
				
				int is_lan_conflict = 0;
				
				/* chk dut lan domain */
				const char *str_start = nvram_safe_get("dhcp_start");
				const char *str_end   = nvram_safe_get("dhcp_end");
				
				if (str_start && str_start[0] && str_end && str_end[0]) {
					uint32_t s_ip = (sender_ip[0] << 24) | (sender_ip[1] << 16) | (sender_ip[2] << 8) | sender_ip[3];
					uint32_t d_start = 0, d_end = 0;
					int a, b, c, d;
					
					if (sscanf(str_start, "%d.%d.%d.%d", &a, &b, &c, &d) == 4)
						d_start = (a << 24) | (b << 16) | (c << 8) | d;
					if (sscanf(str_end, "%d.%d.%d.%d", &a, &b, &c, &d) == 4)
						d_end = (a << 24) | (b << 16) | (c << 8) | d;
					
					/* if sender in dhcp_start ~ dhcp_end */
					if (d_start != 0 && d_end != 0 && s_ip >= d_start && s_ip <= d_end) {
						is_lan_conflict = 1;
					}
					
					/* if dhcp_start/mod are in the same class c subnet */
					if (d_start != 0 && sender_ip[0] == (d_start >> 24) &&
					    sender_ip[1] == ((d_start >> 16) & 0xFF) &&
					    sender_ip[2] == ((d_start >> 8) & 0xFF)) {
						is_lan_conflict = 1;
					}
				}
				
				if (is_lan_conflict) {
					syslog(LOG_NOTICE, "mod_snooper:: ARP Sender IP %d.%d.%d.%d matches LAN domain (dhcp_start/end). Ignored.", 
					       sender_ip[0], sender_ip[1], sender_ip[2], sender_ip[3]);
					_dprintf("mod_snooper:: ARP Sender IP %d.%d.%d.%d matches LAN domain (dhcp_start/end). Ignored.", 
					       sender_ip[0], sender_ip[1], sender_ip[2], sender_ip[3]);
				} else {
					/* MOD sure */
					uint8_t *smac = l3_payload + 8;
					snprintf(src_mac, sizeof(src_mac), "%02x:%02x:%02x:%02x:%02x:%02x",
						 smac[0], smac[1], smac[2], smac[3], smac[4], smac[5]);
					syslog(LOG_NOTICE, "mod_snooper:: Detected MOD via ARP (IP: %d.%d.%d.%d, MAC: %s)", 
					       sender_ip[0], sender_ip[1], sender_ip[2], sender_ip[3], src_mac);
					_dprintf("mod_snooper:: Detected MOD via ARP (IP: %d.%d.%d.%d, MAC: %s)", 
					       sender_ip[0], sender_ip[1], sender_ip[2], sender_ip[3], src_mac);
					matched = 1;
				}
			}
		}
	}
	/* DHCP Option 60 Snooping */
	else if (raw_ether_type == ETH_P_IP) {
		unsigned int dhcp_offset = (l3_payload - recvBuf) + 28; // IP(20) + UDP(8)
		if (data_len < dhcp_offset + 240) return;

		uint8_t *p = recvBuf + dhcp_offset + 236;
		if (p[0] == 0x63 && p[1] == 0x82 && p[2] == 0x53 && p[3] == 0x63) { /* Magic Cookie */
			p += 4;
			while ((p - recvBuf) < data_len && *p != 255) {
				uint8_t opcode = *p;
				if (opcode == 0) { p++; continue; }
				uint8_t oplen = *(p + 1);
				
				if (opcode == 60) {
					char value[256] = {0};
					memcpy(value, p + 2, oplen);
					if (strstr(value, m_vendor)) {
						snprintf(src_mac, sizeof(src_mac), "%02x:%02x:%02x:%02x:%02x:%02x",
							 recvBuf[6], recvBuf[7], recvBuf[8], recvBuf[9], recvBuf[10], recvBuf[11]);
						syslog(LOG_NOTICE, "mod_snooper:: Detected MOD via DHCP (Vendor: %s, MAC: %s)", value, src_mac);
						_dprintf("mod_snooper:: Detected MOD via DHCP (Vendor: %s, MAC: %s)", value, src_mac);
						matched = 1;
						break;
					}
				}
				p += 2 + oplen;
			}
		}
	}

	if (matched) {
#if IS_NETDEV_MACRO
		char target_if[IFNAMSIZ];
		if (get_ifname_by_mac(src_mac, target_if)) {
			
			if (isolated_if[0] == '\0' || strcmp(target_if, isolated_if) != 0) {
				
				syslog(LOG_NOTICE, "mod_snooper:: Detected MOD on new port! (MAC: %s, OldPort: %s, NewPort: %s)", 
				       src_mac, isolated_if[0] ? isolated_if : "None", target_if);
				
				if (isolated_if[0] != '\0') {
					restore_mod_port();
				}
				
				isolate_mod_port(src_mac, target_if, -1);
			}
			
		} else {
			syslog(LOG_NOTICE, "mod_snooper:: Target MAC %s found, but physical interface not yet learned by br0.", src_mac);
		}
#else
		int phy_port = -1;
		phy_port = switch_find_mac_port(src_mac);

		if (phy_port >= 0) {
#if IS_MXL_MACRO
			if (first_detected_phy_port == -1) {
				first_detected_phy_port = phy_port;
			} else if (first_detected_phy_port != phy_port) {
				if (nvram_get_int("hinet_reboot") == 1) {
					syslog(LOG_NOTICE, "mod_snooper:: [TEST] MOD port changed (%d -> %d). Rebooting!", first_detected_phy_port, phy_port);
					_dprintf("mod_snooper:: [TEST] MOD port changed (%d -> %d). Rebooting!", first_detected_phy_port, phy_port);
					safe_do_system("reboot");
					sleep(3);
					exit(0);
				}
				first_detected_phy_port = phy_port;
			}
#endif
			if (isolated_phy_port == -1 || phy_port != isolated_phy_port) {
				syslog(LOG_NOTICE, "mod_snooper:: Detected MOD on new PHY port! (MAC: %s, OldPort: %d, NewPort: %d)", src_mac, isolated_phy_port, phy_port);
				_dprintf("mod_snooper:: Detected MOD on new PHY port! (MAC: %s, OldPort: %d, NewPort: %d)", src_mac, isolated_phy_port, phy_port);
				if (isolated_phy_port != -1) 
					restore_mod_port();
				isolate_mod_port(src_mac, NULL, phy_port);
                       }
               }
#endif
	}
}

void handle_sigterm(int sig) { m_exit = 1; }

int mod_snooper_main(int argc, char **argv)
{
	pid_t pid = fork();
	if (pid > 0) {
		FILE *fp = fopen(PID_FILE, "w");
		if (fp) { fprintf(fp, "%d", pid); fclose(fp); }
		return 0;
	} else if (pid < 0) {
		return -1;
	}

	syslog(LOG_NOTICE, "mod_snooper:: Daemon started. Initializing...");
	_dprintf("mod_snooper:: Daemon started. Initializing...");

	init_cleanup();

	m_raw_sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	struct sockaddr_ll sll;
	memset(&sll, 0, sizeof(sll));
	sll.sll_family = AF_PACKET;
	sll.sll_protocol = htons(ETH_P_ALL);
	sll.sll_ifindex = if_nametoindex("br0");
	bind(m_raw_sock, (struct sockaddr*)&sll, sizeof(sll));
	fcntl(m_raw_sock, F_SETFL, fcntl(m_raw_sock, F_GETFL, 0) | O_NONBLOCK);

	signal(SIGTERM, handle_sigterm);

	fd_set readfds;
	struct timeval timeout;

	syslog(LOG_NOTICE, "mod_snooper:: Listening for MOD signatures...");
	_dprintf("mod_snooper:: Listening for MOD signatures...");

	while (!m_exit) {
		FD_ZERO(&readfds);
		FD_SET(m_raw_sock, &readfds);
		
		timeout.tv_sec = 2;
		timeout.tv_usec = 0;

		int ret = select(m_raw_sock + 1, &readfds, NULL, NULL, &timeout);
		
		check_isolated_port_status();

		if (ret > 0) {
			if (FD_ISSET(m_raw_sock, &readfds)) {
				handle_raw_packet();
			}
		}
	}

	restore_mod_port();
	close(m_raw_sock);
	unlink(PID_FILE);
	syslog(LOG_NOTICE, "mod_snooper:: Daemon exiting.");
	_dprintf("mod_snooper:: Daemon exiting.");

	return 0;
}

