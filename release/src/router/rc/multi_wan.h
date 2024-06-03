#ifndef __MULTI_WAN_H
#define __MULTI_WAN_H

#include <rc.h>
#include <shared.h>
#include <shutils.h>

extern char wanX_resolv_path[];

enum {
	MTWAN_MODE_ANY = 0,
	MTWAN_MODE_ALL,
	MTWAN_MODE_TIME,
};

typedef struct __mtwan_profile {
	// nvram
	int enable;
	int mt_group[MAX_MULTI_WAN_NUM];
	int mt_weight[MAX_MULTI_WAN_NUM];
	int mode;
	int fb;
	int order[MAX_MULTI_WAN_NUM];
	char sched[16];
	int group;
	// aux
	double prob[MAX_MULTI_WAN_NUM];
	int wan_units[MAX_MULTI_WAN_NUM];
	int lb_wan_units[MAX_MULTI_WAN_NUM];
} MTWAN_PROF_T;

int is_mtwan_ifname(const char *ifname);
int is_mtwan_unit(const int unit);
int mtwan_get_unit_by_dualwan_unit(const int dualwan_unit);
int mtwan_get_real_wan(const int unit, char *prefix, const size_t len);
int mtwan_ifunit(const char *wan_ifname);
int mtwan_start_multi_wan();
int mtwan_stop_multi_wan6();
int mtwan_handle_if_updown(const int unit, const char *ifname, const int up);
int mtwan_handle_ip_rule(const int unit);
int mtwan_handle_ip_route(const int unit);
int mtwan_get_route_table_id(const int unit, char *table, const size_t table_len);
int mtwan_get_route_rule_pref(const int unit) ;
int mtwan_get_default_wan();
int mtwan_get_ifname(const int unit, char *ifname, const size_t len);
int mtwan_update_nat_firewall(FILE *fp_nat);
int mtwan_init_nvram();
int mtwan_get_mapped_unit (int mtwan_unit);
int mtwan_get_dns(const int mtwan_unit, char *dns1, const size_t dns1_sz, char *dns2, const size_t dns2_sz);
int mtwan_get_wans_type(void);
char *mtwan_get_lan_ifname(int port, char *buf, size_t len);
char *mtwan_get_wan_ifname(char *buf, size_t len);
#ifdef RTCONFIG_MULTIWAN_PROFILE
int is_mtwan_enable();
int is_mtwan_lb_in_profile(int mtwan_idx, int wan_unit);
int is_mtwan_lb(int unit);
int is_mtwan_group_lb(int mtwan_idx, int mtwan_group);
int is_mtwan_group_in_profile(int mtwan_idx, int mtwan_group);
int is_mtwan_unit_in_profile(int mtwan_idx, int wan_unit);
int is_mtwan_primary(int wan_unit);
int is_mtwan_primary_group(int wan_unit);
int mtwan_get_num_of_wan_by_group(int mtwan_idx, int mtwan_group);
int mtwan_get_lb_route_table_id(int mtwan_idx, int mtwan_group, char *table, size_t table_len);
int mtwan_get_lb_rule_pref(int mtwan_idx, char *pref, size_t pref_len);
int mtwan_get_mark_rule_pref(int unit);
int mtwan_get_first_wan_unit_by_group(int mtwan_idx, int mtwan_group);
int mtwan_group_compare(int mtwan_idx, int mtwan_group_1, int mtwan_group_2);
int mtwan_get_wan_group(int mtwan_idx, int wan_unit);
int mtwan_get_fo_group(int mtwan_idx, int check_conn);
int mtwan_get_first_group(int mtwan_idx);
int mtwan_get_second_group(int mtwan_idx);
void mtwan_handle_group_change(int mtwan_idx, int to_group);
void mtwan_handle_wan_conn(int wan_unit, int connected);
void mtwan_init_profile();
void mtwan_init_mtwan_group();
void mtwan_append_group_main_resolvconf(int wan_unit);
void mtwan_update_lb_route(int unit, int up);
void mtwan_update_lb_prob(int unit, int up);
void mtwan_update_lb_iptables(int unit, int up);
void mtwan_update_main_default_route(int unit, int up);
void mtwan_update_profile_lb_route(int mtwan_idx, int mtwan_group, int unit, int up);
void mtwan_update_profile();
void mtwan_print_profile(int mtwan_idx);
#endif
#ifdef RTCONFIG_SOFTWIRE46
void mtwan_append_s46_resolvconf(void);
#endif
#endif
