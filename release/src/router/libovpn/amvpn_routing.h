#ifndef _AMVPN_ROUTING_H
#define _AMVPN_ROUTING_H

typedef enum {
	VPNDIR_PROTO_NONE,
	VPNDIR_PROTO_OPENVPN,
	VPNDIR_PROTO_WIREGUARD
} vpndir_proto_t;


void _flush_routing_cache();
void _write_routing_rules(int unit, char *rules, int verb, vpndir_proto_t proto);
extern void amvpn_set_routing_rules(int unit, vpndir_proto_t proto);
void amvpn_clear_routing_rules(int unit, vpndir_proto_t proto);
extern void update_client_routes(char *server_iface, int addroute);
void _add_server_routes(char *server_iface, int client_unit, vpndir_proto_t proto);
void _del_server_routes(char *server_iface, int client_unit, vpndir_proto_t proto);
extern char *amvpn_get_policy_rules(int unit, char *buffer, int bufferlen, vpndir_proto_t proto);
extern int amvpn_set_policy_rules(char *buffer);

extern void amvpn_clear_exclusive_dns(int unit, vpndir_proto_t proto);
extern void amvpn_update_exclusive_dns_rules();
extern void ovpn_set_exclusive_dns(int unit);
#ifdef RTCONFIG_WIREGUARD
extern void wgc_set_exclusive_dns(int unit);
#if defined(RTCONFIG_HND_ROUTER_AX_6756) || defined(RTCONFIG_BCM_502L07P2) || defined(RTCONFIG_HND_ROUTER_AX_675X) || defined(RTCONFIG_HND_ROUTER_BE_4916)
extern void amvpn_clear_wg_bypass_rules();
void _amvpn_apply_wg_bypass(int unit, int add);
extern void amvpn_refresh_wg_bypass_rules();
#endif
#endif

extern void amvpn_set_wan_routing_rules();
#endif

