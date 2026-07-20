#ifndef _VPNDIRECTOR_UTILS_H
#define _VPNDIRECTOR_UTILS_H

typedef enum {
	VPNDIR_ADDR_INVALID = -1,
	VPNDIR_ADDR_ANY = 0,
	VPNDIR_ADDR_IPV4 = 4,
	VPNDIR_ADDR_IPV6 = 6
} vpndir_addr_family_t;

vpndir_addr_family_t vpndir_address_family(const char *address);
vpndir_addr_family_t vpndir_rule_address_family(const char *source, const char *destination);

#endif
