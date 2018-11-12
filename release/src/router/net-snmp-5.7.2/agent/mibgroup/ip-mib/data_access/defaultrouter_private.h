struct netsnmp_defaultrouter_s;
struct netsnmp_container_s;

int netsnmp_arch_defaultrouter_entry_init(struct netsnmp_defaultrouter_s *entry);
int netsnmp_arch_defaultrouter_container_load(struct netsnmp_container_s *container,
                                              u_int load_flags);
