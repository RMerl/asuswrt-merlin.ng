#ifndef _XTABLES_MULTI_H
#define _XTABLES_MULTI_H 1

extern int iptables_xml_main(int, char **);
#ifdef ENABLE_NFTABLES
extern int xtables_ip4_main(int, char **);
extern int xtables_ip4_save_main(int, char **);
extern int xtables_ip4_restore_main(int, char **);
extern int xtables_ip6_main(int, char **);
extern int xtables_ip6_save_main(int, char **);
extern int xtables_ip6_restore_main(int, char **);
extern int xtables_arp_xlate_main(int, char **);
extern int xtables_ip4_xlate_main(int, char **);
extern int xtables_ip6_xlate_main(int, char **);
extern int xtables_eb_xlate_main(int, char **);
extern int xtables_ip4_xlate_restore_main(int, char **);
extern int xtables_ip6_xlate_restore_main(int, char **);
extern int xtables_arp_main(int, char **);
extern int xtables_arp_restore_main(int, char **);
extern int xtables_arp_save_main(int, char **);
extern int xtables_eb_main(int, char **);
extern int xtables_eb_restore_main(int, char **);
extern int xtables_eb_save_main(int, char **);
extern int xtables_monitor_main(int, char **);

extern struct xtables_globals arptables_globals;
extern struct xtables_globals ebtables_globals;
#endif

#endif /* _XTABLES_MULTI_H */
