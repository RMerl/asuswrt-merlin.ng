#ifndef __FIREWALL_SDN_H__
#define __FIREWALL_SDN_H__

#include <rtconfig.h>

enum
{
	IPTABLE_TYPE_FILTER,
	IPTABLE_TYPE_NAT,
	IPTABLE_TYPE_MANGLE
};

int get_drop_accept(char *logdrop, const size_t logdrop_len, char *logaccept, const size_t logaccept_len);
int create_iptables_file(const int type, const int sdn_idx, FILE **fp, char *file_path, const size_t path_len, const int isv6);
int close_n_restore_iptables_file(const int type, FILE **fp, const char *file_path, const int isv6);

/**************************************************************************************************************************
 * SDN general rules
 * ************************************************************************************************************************/
int update_SDN_iptables(const MTLAN_T *pmtl, const char *logdrop, const char *logaccept);

int handle_SDN_internal_access(const char *logdrop, const char *logaccept);

int reset_sdn_iptables(const int sdn_idx);
int reset_sdn_firewall(const unsigned long features, const int sdn_idx);

/**************************************************************************************************************************
 * URL Filter
 * ************************************************************************************************************************/
#ifdef RTCONFIG_IPV6
int write_URLFilter_SDN(const MTLAN_T *pmtl, const char *logdrop, FILE *fp_filter, FILE *fpv6_filter);
#else
int write_URLFilter_SDN(const MTLAN_T *pmtl, const char *logdrop, FILE *fp_filter);
#endif

int handle_URLFilter_jump_rule(const MTLAN_T *pmtl);
int update_URLFilter(const int urlf_idx);

/**************************************************************************************************************************
 * Network Service  Filter
 * ************************************************************************************************************************/
#ifdef RTCONFIG_IPV6
int write_NwServiceFilter_SDN(const MTLAN_T *pmtl, const char *logdrop, const char *logaccept, FILE *fp_filter, FILE *fpv6_filter);
#else
int write_NwServiceFilter_SDN(const MTLAN_T *pmtl, const char *logdrop, const char *logaccept, FILE *fp_filter);
#endif

int handle_NwServiceFilter_jump_rule(const MTLAN_T *pmtl);
int update_NwServiceFilter(const int nwf_idx);
#endif
