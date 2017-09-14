#ifndef _OPENVPN_CONTROL_H
#define _OPENVPN_CONTROL_H

extern int check_ovpn_server_enabled(int unit);
extern int check_ovpn_client_enabled(int unit);
extern void update_ovpn_routing(int unit);

#endif

