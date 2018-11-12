struct netsnmp_container_s;
struct netsnmp_route_s;

int netsnmp_access_route_container_arch_load(struct netsnmp_container_s* container,
                                             u_int load_flags);
int netsnmp_arch_route_create(struct netsnmp_route_s *entry);
int netsnmp_arch_route_delete(struct netsnmp_route_s *entry);
