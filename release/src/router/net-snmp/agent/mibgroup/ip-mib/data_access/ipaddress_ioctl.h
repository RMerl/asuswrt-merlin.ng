#ifdef __cplusplus
extern          "C" {
#endif

struct netsnmp_ipaddress_s;

/*
 * struct for netlink extras
 */
struct address_flag_info {
    int bcastflg;
    int anycastflg;
    in_addr_t addr;
};
 
#define IS_APIPA(a)  (((in_addr_t)(a << 16)) == 0xFEA90000)

int
_netsnmp_ioctl_ipaddress_container_load_v4(netsnmp_container *container,
                                                  int idx_offset);
int
_netsnmp_ioctl_ipaddress_set_v4(struct netsnmp_ipaddress_s * entry);
int
_netsnmp_ioctl_ipaddress_remove_v4(struct netsnmp_ipaddress_s * entry);

int
_netsnmp_ioctl_ipaddress_set_v6(struct netsnmp_ipaddress_s * entry);
int
_netsnmp_ioctl_ipaddress_remove_v6(struct netsnmp_ipaddress_s * entry);

int
netsnmp_access_ipaddress_ioctl_get_interface_count(int sd, struct ifconf * ifc);

struct address_flag_info
netsnmp_access_other_info_get(int index, int family);

/*
 * struct ioctl for arch_data
 */
typedef struct _ioctl_extras {
   u_int            flags;
   u_char           name[IFNAMSIZ];
} _ioctl_extras;



_ioctl_extras *
netsnmp_ioctl_ipaddress_entry_init(struct netsnmp_ipaddress_s *entry);
void
netsnmp_ioctl_ipaddress_entry_cleanup(struct netsnmp_ipaddress_s *entry);
int
netsnmp_ioctl_ipaddress_entry_copy(struct netsnmp_ipaddress_s *lhs,
                                   struct netsnmp_ipaddress_s *rhs);

_ioctl_extras *
netsnmp_ioctl_ipaddress_extras_get(struct netsnmp_ipaddress_s *entry);

int
_netsnmp_ioctl_ipaddress_delete_v4(struct netsnmp_ipaddress_s * entry);
int
_netsnmp_ioctl_ipaddress_delete_v6(struct netsnmp_ipaddress_s * entry);

#ifdef __cplusplus
}
#endif

