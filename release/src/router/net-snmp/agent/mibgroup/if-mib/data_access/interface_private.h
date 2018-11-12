struct prefix_info;
struct netsnmp_container_s;
struct netsnmp_interface_entry_s;

extern struct prefix_info *prefix_head_list;

void netsnmp_arch_interface_init(void);
int netsnmp_arch_interface_container_load(struct netsnmp_container_s* container,
                                          u_int load_flags);
oid netsnmp_arch_interface_index_find(const char *name);
int netsnmp_arch_set_admin_status(struct netsnmp_interface_entry_s * entry,
                                  int ifAdminStatus_val);
