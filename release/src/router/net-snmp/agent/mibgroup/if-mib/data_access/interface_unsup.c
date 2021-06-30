#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include "interface_private.h"

void netsnmp_arch_interface_init(void)
{
}

int netsnmp_arch_interface_container_load(struct netsnmp_container_s* container,
                                          u_int load_flags)
{
    return 0;
}

oid netsnmp_arch_interface_index_find(const char *name)
{
    return 0;
}

int netsnmp_arch_set_admin_status(struct netsnmp_interface_entry_s * entry,
                                  int ifAdminStatus_val)
{
    return 0;
}

