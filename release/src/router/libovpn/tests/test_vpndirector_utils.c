#include <assert.h>
#include <stdio.h>

#include "../vpndirector_utils.h"

static void test_address_family_detection(void)
{
	assert(vpndir_address_family("") == VPNDIR_ADDR_ANY);
	assert(vpndir_address_family("0.0.0.0") == VPNDIR_ADDR_ANY);
	assert(vpndir_address_family("::") == VPNDIR_ADDR_IPV6);
	assert(vpndir_address_family("::/0") == VPNDIR_ADDR_IPV6);
	assert(vpndir_address_family("192.0.2.20") == VPNDIR_ADDR_IPV4);
	assert(vpndir_address_family("192.0.2.0/24") == VPNDIR_ADDR_IPV4);
	assert(vpndir_address_family("2001:db8:50::204") == VPNDIR_ADDR_IPV6);
	assert(vpndir_address_family("2001:db8:50::/64") == VPNDIR_ADDR_IPV6);
}

static void test_invalid_addresses_are_rejected(void)
{
	assert(vpndir_address_family("192.0.2.1/33") == VPNDIR_ADDR_INVALID);
	assert(vpndir_address_family("192.0.2.1/+1") == VPNDIR_ADDR_INVALID);
	assert(vpndir_address_family("2001:db8::/129") == VPNDIR_ADDR_INVALID);
	assert(vpndir_address_family("2001:db8::/-0") == VPNDIR_ADDR_INVALID);
	assert(vpndir_address_family("not-an-address") == VPNDIR_ADDR_INVALID);
}

static void test_rule_family_rejects_mixed_ip_versions(void)
{
	assert(vpndir_rule_address_family("2001:db8:50::/64", "") == VPNDIR_ADDR_IPV6);
	assert(vpndir_rule_address_family("", "2001:db8:60::/64") == VPNDIR_ADDR_IPV6);
	assert(vpndir_rule_address_family("192.0.2.0/24", "") == VPNDIR_ADDR_IPV4);
	assert(vpndir_rule_address_family("", "") == VPNDIR_ADDR_ANY);
	assert(vpndir_rule_address_family("192.0.2.0/24", "2001:db8:60::/64") == VPNDIR_ADDR_INVALID);
}

int main(void)
{
	test_address_family_detection();
	test_invalid_addresses_are_rejected();
	test_rule_family_rejects_mixed_ip_versions();
	puts("vpndirector utility tests passed");
	return 0;
}
