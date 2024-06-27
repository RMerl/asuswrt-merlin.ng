#ifndef __NETWORK_H__
#define __NETWORK_H__


extern int convert_subnet_mask_to_cidr(const char *mask);

extern char *convert_cidr_to_subnet_mask(const unsigned long cidr, char *mask, const int mask_len);

extern int get_network_addr_by_ip_prefix(const char *ip, const char *netmask, char *full_addr, const int len);

extern int is_valid_ip(const char* addr);
extern int is_valid_ip4(const char* addr);
extern int is_valid_ip6(const char* addr);
extern int is_ip4_in_use(const char* addr);

extern int resolv_addr4(const char *dn, char *buf, size_t len);
extern int resolv_addr4_all(const char *dn, char *buf, size_t len);
extern int resolv_addr6(const char *dn, char *buf, size_t len);
extern int resolv_addr6_all(const char *dn, char *buf, size_t len);
extern int resolv_addr_all(const char *dn, char *buf, size_t len);

extern int validate_ip(char *ip);

extern int is_same_subnet(const char *ip1, const char *ip2, const char *netmask);
#endif
