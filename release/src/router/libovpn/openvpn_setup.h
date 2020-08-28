#ifndef _OPENVPN_SETUP_H
#define _OPENVPN_SETUP_H

extern int ovpn_write_server_config(ovpn_sconf_t *sconf, int unit);
extern int ovpn_write_client_config(ovpn_cconf_t *cconf, int unit);
extern void ovpn_write_client_keys(ovpn_cconf_t *cconf, int unit);
extern void ovpn_write_server_keys(ovpn_sconf_t *sconf, int unit);
extern void ovpn_setup_client_fw(ovpn_cconf_t *cconf, int unit);
extern void ovpn_setup_server_fw(ovpn_sconf_t *sconf, int unit);
extern void ovpn_setup_server_watchdog(ovpn_sconf_t *sconf, int unit);
extern void write_ovpn_resolv_dnsmasq(FILE* dnsmasq_conf);
extern void write_ovpn_dnsmasq_config(FILE* dnsmasq_conf);
extern char *get_ovpn_remote_address(char *buf, int len);
extern void update_ovpn_profie_remote(void);
extern char *ovpn_get_runtime_filename(ovpn_type_t type, int unit, ovpn_key_t key_type, char *buffer, int len);
extern int ovpn_setup_iface(char *iface, ovpn_if_t iface_type, int bridge);
extern void ovpn_remove_iface(ovpn_type_t type, int unit);
extern void ovpn_setup_dirs(ovpn_type_t type, int unit);
extern void ovpn_cleanup_dirs(ovpn_type_t type, int unit);
extern void ovpn_write_dh(ovpn_sconf_t *sconf, int unit);
extern int ovpn_is_clientcert_valid(int unit);
#endif

