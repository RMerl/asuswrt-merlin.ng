#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "vpndirector_utils.h"

static int _vpndir_valid_prefix_length(const char *prefix, unsigned int maximum)
{
	const char *p;
	char *end = NULL;
	unsigned long value;

	if (!prefix || !*prefix)
		return 0;

	for (p = prefix; *p; ++p) {
		if (*p < '0' || *p > '9')
			return 0;
	}

	errno = 0;
	value = strtoul(prefix, &end, 10);
	return errno == 0 && end && *end == '\0' && value <= maximum;
}

vpndir_addr_family_t vpndir_address_family(const char *address)
{
	char host[INET6_ADDRSTRLEN];
	const char *slash;
	size_t host_len;
	struct in_addr addr4;
	struct in6_addr addr6;

	if (!address || !*address || !strcmp(address, "0.0.0.0"))
		return VPNDIR_ADDR_ANY;

	slash = strchr(address, '/');
	if (slash) {
		if (strchr(slash + 1, '/'))
			return VPNDIR_ADDR_INVALID;
		host_len = (size_t)(slash - address);
	} else {
		host_len = strlen(address);
	}

	if (!host_len || host_len >= sizeof(host))
		return VPNDIR_ADDR_INVALID;

	memcpy(host, address, host_len);
	host[host_len] = '\0';

	if (inet_pton(AF_INET, host, &addr4) == 1) {
		if (slash && !_vpndir_valid_prefix_length(slash + 1, 32))
			return VPNDIR_ADDR_INVALID;
		return VPNDIR_ADDR_IPV4;
	}

	if (inet_pton(AF_INET6, host, &addr6) == 1) {
		if (slash && !_vpndir_valid_prefix_length(slash + 1, 128))
			return VPNDIR_ADDR_INVALID;
		return VPNDIR_ADDR_IPV6;
	}

	return VPNDIR_ADDR_INVALID;
}

vpndir_addr_family_t vpndir_rule_address_family(const char *source, const char *destination)
{
	vpndir_addr_family_t source_family = vpndir_address_family(source);
	vpndir_addr_family_t destination_family = vpndir_address_family(destination);

	if (source_family == VPNDIR_ADDR_INVALID || destination_family == VPNDIR_ADDR_INVALID)
		return VPNDIR_ADDR_INVALID;

	if (source_family != VPNDIR_ADDR_ANY && destination_family != VPNDIR_ADDR_ANY &&
	    source_family != destination_family)
		return VPNDIR_ADDR_INVALID;

	return source_family != VPNDIR_ADDR_ANY ? source_family : destination_family;
}
