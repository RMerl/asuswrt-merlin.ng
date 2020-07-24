#ifndef _OPENVPN_CONTROL_H
#define _OPENVPN_CONTROL_H

extern int check_ovpn_server_enabled(int unit);
extern int check_ovpn_client_enabled(int unit);
extern void ovpn_update_routing(int unit);
extern void ovpn_run_fw_scripts();
extern int ovpn_run_instance(ovpn_type_t type, int unit);
extern void update_ovpn_status(ovpn_type_t type, int unit, ovpn_status_t status_type, ovpn_errno_t err_no);
extern ovpn_status_t get_ovpn_status(ovpn_type_t type, int unit);
extern ovpn_errno_t get_ovpn_errno(ovpn_type_t type, int unit);
#endif
