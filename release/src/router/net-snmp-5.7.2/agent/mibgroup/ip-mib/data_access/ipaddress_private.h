int netsnmp_arch_ipaddress_container_load(netsnmp_container *container,
                                          u_int load_flags);
int netsnmp_arch_ipaddress_entry_init(netsnmp_ipaddress_entry *entry);
void netsnmp_arch_ipaddress_entry_cleanup(netsnmp_ipaddress_entry *entry);
int netsnmp_arch_ipaddress_entry_copy(netsnmp_ipaddress_entry *lhs,
                                  netsnmp_ipaddress_entry *rhs);
int netsnmp_arch_ipaddress_create(netsnmp_ipaddress_entry *entry);
int netsnmp_arch_ipaddress_delete(netsnmp_ipaddress_entry *entry);
int netsnmp_arch_ipaddress_container_load(netsnmp_container *container,
                                      u_int load_flags);
