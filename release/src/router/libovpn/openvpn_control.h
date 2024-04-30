#ifndef _OPENVPN_CONTROL_H
#define _OPENVPN_CONTROL_H

#define VPNROUTING_LOCK "vpnrouting-dns"

extern int ovpn_skip_dnsmasq();
extern int check_ovpn_server_enabled(int unit);
extern int check_ovpn_client_enabled(int unit);
extern void ovpn_run_fw_scripts();
extern void ovpn_run_fw_nat_scripts();
extern int ovpn_run_instance(ovpn_type_t type, int unit);
extern void update_ovpn_status(ovpn_type_t type, int unit, ovpn_status_t status_type, ovpn_errno_t err_no);
extern ovpn_status_t get_ovpn_status(ovpn_type_t type, int unit);
extern ovpn_errno_t get_ovpn_errno(ovpn_type_t type, int unit);
extern void ovpn_server_up_handler(int unit);
extern void ovpn_server_down_handler(int unit);
extern void ovpn_client_route_pre_down_handler();
extern void ovpn_client_route_up_handler();
extern void ovpn_client_down_handler(int unit);
extern void ovpn_client_up_handler(int unit);
char *_safe_getenv(const char* name);
void _ovpn_run_event_script();
extern void ovpn_process_eas(int start);
extern void ovpn_start_client(int unit);
extern void ovpn_start_server(int unit);
extern void ovpn_stop_client(int unit);
extern void ovpn_stop_server(int unit);
extern void ovpn_set_killswitch(int unit);
extern int ovpn_need_dnsmasq_restart();

extern void start_ovpn_serverall();
extern void stop_ovpn_serverall();

#ifdef RTCONFIG_MULTILAN_CFG
extern void _update_ovpn_by_sdn(MTLAN_T *pmtl, size_t mtl_sz, int restart_all_sdn, wg_type_t client);
extern void _update_ovpn_by_sdn_remove(MTLAN_T *pmtl, size_t mtl_sz, wg_type_t client);
extern void _ovpn_client_nf_bind_sdn(FILE* fp, const char* ovpn_ifname, const char* sdn_ifname);
//extern void _ovpn_server_nf_bind_wan(const char* ifname, int add);
extern void _ovpn_server_nf_bind_sdn(FILE* fp, const char* ovpn_ifname, const char* sdn_ifname);
#endif

#endif
