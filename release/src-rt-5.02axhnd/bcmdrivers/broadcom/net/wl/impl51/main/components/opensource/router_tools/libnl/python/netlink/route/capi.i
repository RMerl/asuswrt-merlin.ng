%module capi
%{
#include <netlink/route/rtnl.h>
#include <netlink/route/link.h>
#include <netlink/route/link/vlan.h>
#include <netlink/route/link/macvlan.h>
#include <netlink/route/link/vxlan.h>
#include <netlink/route/link/bridge.h>
#include <netlink/route/link/inet.h>

#include <netlink/route/tc.h>
#include <netlink/route/qdisc.h>
#include <netlink/route/class.h>
#include <netlink/route/classifier.h>

#include <netlink/route/qdisc/htb.h>

#include <netlink/route/addr.h>
%}

%include <stdint.i>
%include <cstring.i>

%inline %{
        struct nl_object *link2obj(struct rtnl_link *link)
        {
                return OBJ_CAST(link);
        }

        struct rtnl_link *obj2link(struct nl_object *obj)
        {
                return (struct rtnl_link *) obj;
        }

        struct rtnl_link *get_from_kernel(struct nl_sock *sk, int ifindex, const char *name)
        {
                struct rtnl_link *link;
                if (rtnl_link_get_kernel(sk, ifindex, name, &link) < 0)
                        return NULL;
                return link;
        }

        uint32_t inet_get_conf(struct rtnl_link *link, const unsigned int id)
        {
                uint32_t result;

                if (rtnl_link_inet_get_conf(link, id, &result) < 0)
                        return 0;

                return result;
        }
%};

extern struct nl_object *link2obj(struct rtnl_link *);
extern struct rtnl_link *obj2link(struct nl_object *);

/* <netlink/route/rtnl.h> */

%cstring_output_maxsize(char *buf, size_t len)
extern char *		rtnl_scope2str(int, char *buf, size_t len);
extern int		rtnl_str2scope(const char *);

/* <netlink/route/link.h> */

extern struct rtnl_link *rtnl_link_alloc(void);

extern struct rtnl_link *rtnl_link_get(struct nl_cache *, int);
extern struct rtnl_link *rtnl_link_get_by_name(struct nl_cache *, const char *);

extern int rtnl_link_build_add_request(struct rtnl_link *, int, struct nl_msg **);
extern int rtnl_link_add(struct nl_sock *, struct rtnl_link *, int);
extern int rtnl_link_build_change_request(struct rtnl_link *, struct rtnl_link *, int, struct nl_msg **);
extern int rtnl_link_change(struct nl_sock *, struct rtnl_link *, struct rtnl_link *, int);

extern int rtnl_link_build_delete_request(const struct rtnl_link *, struct nl_msg **);
extern int rtnl_link_delete(struct nl_sock *, const struct rtnl_link *);
extern int rtnl_link_build_get_request(int, const char *, struct nl_msg **);

extern char *rtnl_link_stat2str(int, char *, size_t);
extern int rtnl_link_str2stat(const char *);

%cstring_output_maxsize(char *buf, size_t len)
extern char *rtnl_link_flags2str(int, char *buf, size_t len);
extern int rtnl_link_str2flags(const char *);

%cstring_output_maxsize(char *buf, size_t len)
extern char *rtnl_link_operstate2str(uint8_t, char *buf, size_t len);
extern int rtnl_link_str2operstate(const char *);

%cstring_output_maxsize(char *buf, size_t len)
extern char *rtnl_link_mode2str(uint8_t, char *buf, size_t len);
extern int rtnl_link_str2mode(const char *);

extern void rtnl_link_set_qdisc(struct rtnl_link *, const char *);
extern char *rtnl_link_get_qdisc(struct rtnl_link *);

extern void rtnl_link_set_name(struct rtnl_link *, const char *);
extern char *rtnl_link_get_name(struct rtnl_link *);

extern void rtnl_link_set_flags(struct rtnl_link *, unsigned int);
extern void rtnl_link_unset_flags(struct rtnl_link *, unsigned int);
extern unsigned int rtnl_link_get_flags(struct rtnl_link *);

extern void rtnl_link_set_mtu(struct rtnl_link *, unsigned int);
extern unsigned int rtnl_link_get_mtu(struct rtnl_link *);

extern void rtnl_link_set_txqlen(struct rtnl_link *, unsigned int);
extern unsigned int rtnl_link_get_txqlen(struct rtnl_link *);

extern void rtnl_link_set_ifindex(struct rtnl_link *, int);
extern int rtnl_link_get_ifindex(struct rtnl_link *);

extern void rtnl_link_set_family(struct rtnl_link *, int);
extern int rtnl_link_get_family(struct rtnl_link *);

extern void rtnl_link_set_arptype(struct rtnl_link *, unsigned int);
extern unsigned int rtnl_link_get_arptype(struct rtnl_link *);

extern void rtnl_link_set_addr(struct rtnl_link *, struct nl_addr *);
extern struct nl_addr *rtnl_link_get_addr(struct rtnl_link *);

extern void rtnl_link_set_broadcast(struct rtnl_link *, struct nl_addr *);
extern struct nl_addr *rtnl_link_get_broadcast(struct rtnl_link *);

extern void rtnl_link_set_link(struct rtnl_link *, int);
extern int rtnl_link_get_link(struct rtnl_link *);

extern void rtnl_link_set_master(struct rtnl_link *, int);
extern int rtnl_link_get_master(struct rtnl_link *);

extern void rtnl_link_set_operstate(struct rtnl_link *, uint8_t);
extern uint8_t rtnl_link_get_operstate(struct rtnl_link *);

extern void rtnl_link_set_linkmode(struct rtnl_link *, uint8_t);
extern uint8_t rtnl_link_get_linkmode(struct rtnl_link *);

extern const char *rtnl_link_get_ifalias(struct rtnl_link *);
extern void rtnl_link_set_ifalias(struct rtnl_link *, const char *);

extern int rtnl_link_get_num_vf(struct rtnl_link *, uint32_t *);

extern uint64_t rtnl_link_get_stat(struct rtnl_link *, int);
extern int rtnl_link_set_stat(struct rtnl_link *, const unsigned int, const uint64_t);

extern int rtnl_link_set_type(struct rtnl_link *, const char *);
extern char *rtnl_link_get_type(struct rtnl_link *);

extern int rtnl_link_enslave(struct nl_sock * sock, struct rtnl_link * master, struct rtnl_link * slave);
extern int rtnl_link_release(struct nl_sock * sock, struct rtnl_link * slave);

/* <netlink/route/link/vlan.h> */

struct vlan_map
{
	uint32_t		vm_from;
	uint32_t		vm_to;
};

#define VLAN_PRIO_MAX 7

%cstring_output_maxsize(char *buf, size_t len)
extern char *rtnl_link_vlan_flags2str(int, char *buf, size_t len);
extern int rtnl_link_vlan_str2flags(const char *);

extern int rtnl_link_vlan_set_id(struct rtnl_link *, int);
extern int rtnl_link_vlan_get_id(struct rtnl_link *);

extern int rtnl_link_vlan_set_flags(struct rtnl_link *, unsigned int);
extern int rtnl_link_vlan_unset_flags(struct rtnl_link *, unsigned int);
extern unsigned int rtnl_link_vlan_get_flags(struct rtnl_link *);

extern int rtnl_link_vlan_set_ingress_map(struct rtnl_link *, int, uint32_t);
extern uint32_t *rtnl_link_vlan_get_ingress_map(struct rtnl_link *);

extern int rtnl_link_vlan_set_egress_map(struct rtnl_link *, uint32_t, int);
extern struct vlan_map *rtnl_link_vlan_get_egress_map(struct rtnl_link *, int *);

/* <netlink/route/link/macvlan.h> */

%cstring_output_maxsize(char *buf, size_t len)
extern struct rtnl_link *rtnl_link_macvlan_alloc(void);
extern int		rtnl_link_is_macvlan(struct rtnl_link *);
extern char *		rtnl_link_macvlan_mode2str(int, char *, size_t);
extern int		rtnl_link_macvlan_str2mode(const char *);
extern char *		rtnl_link_macvlan_flags2str(int, char *, size_t);
extern int		rtnl_link_macvlan_str2flags(const char *);
extern int		rtnl_link_macvlan_set_mode(struct rtnl_link *, uint32_t);
extern uint32_t		rtnl_link_macvlan_get_mode(struct rtnl_link *);
extern int		rtnl_link_macvlan_set_flags(struct rtnl_link *, uint16_t);
extern int		rtnl_link_macvlan_unset_flags(struct rtnl_link *, uint16_t);
extern uint16_t		rtnl_link_macvlan_get_flags(struct rtnl_link *);

/* <netlink/route/link/vxlan.h> */

#define VXLAN_ID_MAX 16777215

extern struct rtnl_link *rtnl_link_vxlan_alloc(void);

extern int rtnl_link_is_vxlan(struct rtnl_link *);

extern int rtnl_link_vxlan_set_id(struct rtnl_link *, uint32_t);
extern int rtnl_link_vxlan_get_id(struct rtnl_link *, uint32_t *);

extern int rtnl_link_vxlan_set_group(struct rtnl_link *, struct nl_addr *);
extern int rtnl_link_vxlan_get_group(struct rtnl_link *, struct nl_addr **);

extern int rtnl_link_vxlan_set_link(struct rtnl_link *, uint32_t);
extern int rtnl_link_vxlan_get_link(struct rtnl_link *, uint32_t *);

extern int rtnl_link_vxlan_set_local(struct rtnl_link *, struct nl_addr *);
extern int rtnl_link_vxlan_get_local(struct rtnl_link *, struct nl_addr **);

extern int rtnl_link_vxlan_set_ttl(struct rtnl_link *, uint8_t);
extern int rtnl_link_vxlan_get_ttl(struct rtnl_link *);

extern int rtnl_link_vxlan_set_tos(struct rtnl_link *, uint8_t);
extern int rtnl_link_vxlan_get_tos(struct rtnl_link *);

extern int rtnl_link_vxlan_set_learning(struct rtnl_link *, uint8_t);
extern int rtnl_link_vxlan_get_learning(struct rtnl_link *);
extern int rtnl_link_vxlan_enable_learning(struct rtnl_link *);
extern int rtnl_link_vxlan_disable_learning(struct rtnl_link *);

extern int rtnl_link_vxlan_set_ageing(struct rtnl_link *, uint32_t);
extern int rtnl_link_vxlan_get_ageing(struct rtnl_link *, uint32_t *);

extern int rtnl_link_vxlan_set_limit(struct rtnl_link *, uint32_t);
extern int rtnl_link_vxlan_get_limit(struct rtnl_link *, uint32_t *);

extern int rtnl_link_vxlan_set_port_range(struct rtnl_link *,
										  struct ifla_vxlan_port_range *);
extern int rtnl_link_vxlan_get_port_range(struct rtnl_link *,
										  struct ifla_vxlan_port_range *);

extern int rtnl_link_vxlan_set_proxy(struct rtnl_link *, uint8_t);
extern int rtnl_link_vxlan_get_proxy(struct rtnl_link *);
extern int rtnl_link_vxlan_enable_proxy(struct rtnl_link *);
extern int rtnl_link_vxlan_disable_proxy(struct rtnl_link *);

extern int rtnl_link_vxlan_set_rsc(struct rtnl_link *, uint8_t);
extern int rtnl_link_vxlan_get_rsc(struct rtnl_link *);
extern int rtnl_link_vxlan_enable_rsc(struct rtnl_link *);
extern int rtnl_link_vxlan_disable_rsc(struct rtnl_link *);

extern int rtnl_link_vxlan_set_l2miss(struct rtnl_link *, uint8_t);
extern int rtnl_link_vxlan_get_l2miss(struct rtnl_link *);
extern int rtnl_link_vxlan_enable_l2miss(struct rtnl_link *);
extern int rtnl_link_vxlan_disable_l2miss(struct rtnl_link *);

extern int rtnl_link_vxlan_set_l3miss(struct rtnl_link *, uint8_t);
extern int rtnl_link_vxlan_get_l3miss(struct rtnl_link *);
extern int rtnl_link_vxlan_enable_l3miss(struct rtnl_link *);
extern int rtnl_link_vxlan_disable_l3miss(struct rtnl_link *);

/* <netlink/route/link/bridge.h> */

enum rtnl_link_bridge_flags {
	RTNL_BRIDGE_HAIRPIN_MODE	= 0x0001,
	RTNL_BRIDGE_BPDU_GUARD		= 0x0002,
	RTNL_BRIDGE_ROOT_BLOCK		= 0x0004,
	RTNL_BRIDGE_FAST_LEAVE		= 0x0008,
};

extern int	rtnl_link_is_bridge(struct rtnl_link *);
extern int	rtnl_link_bridge_has_ext_info(struct rtnl_link *);

extern int	rtnl_link_bridge_set_port_state(struct rtnl_link *, uint8_t );
extern int	rtnl_link_bridge_get_port_state(struct rtnl_link *);

extern int	rtnl_link_bridge_set_priority(struct rtnl_link *, uint16_t);
extern int	rtnl_link_bridge_get_priority(struct rtnl_link *);

extern int	rtnl_link_bridge_set_cost(struct rtnl_link *, uint32_t);
extern int	rtnl_link_bridge_get_cost(struct rtnl_link *, uint32_t *);

extern int	rtnl_link_bridge_unset_flags(struct rtnl_link *, unsigned int);
extern int	rtnl_link_bridge_set_flags(struct rtnl_link *, unsigned int);
extern int	rtnl_link_bridge_get_flags(struct rtnl_link *);

extern char * rtnl_link_bridge_flags2str(int, char *, size_t);
extern int	rtnl_link_bridge_str2flags(const char *);

/* <netlink/route/link/inet.h> */
%cstring_output_maxsize(char *buf, size_t len)
extern const char *rtnl_link_inet_devconf2str(int, char *buf, size_t len);
extern unsigned int rtnl_link_inet_str2devconf(const char *);

extern int rtnl_link_inet_set_conf(struct rtnl_link *, const unsigned int, uint32_t);

/* <netlink/route/tc.h> */

%inline %{
        uint32_t tc_str2handle(const char *name)
        {
                uint32_t result;

                if (rtnl_tc_str2handle(name, &result) < 0)
                        return 0;

                return result;
        }
%};

extern void		rtnl_tc_set_ifindex(struct rtnl_tc *, int);
extern int		rtnl_tc_get_ifindex(struct rtnl_tc *);
extern void		rtnl_tc_set_link(struct rtnl_tc *, struct rtnl_link *);
extern struct rtnl_link *rtnl_tc_get_link(struct rtnl_tc *);
extern void		rtnl_tc_set_mtu(struct rtnl_tc *, uint32_t);
extern uint32_t		rtnl_tc_get_mtu(struct rtnl_tc *);
extern void		rtnl_tc_set_mpu(struct rtnl_tc *, uint32_t);
extern uint32_t		rtnl_tc_get_mpu(struct rtnl_tc *);
extern void		rtnl_tc_set_overhead(struct rtnl_tc *, uint32_t);
extern uint32_t		rtnl_tc_get_overhead(struct rtnl_tc *);
extern void		rtnl_tc_set_linktype(struct rtnl_tc *, uint32_t);
extern uint32_t		rtnl_tc_get_linktype(struct rtnl_tc *);
extern void		rtnl_tc_set_handle(struct rtnl_tc *, uint32_t);
extern uint32_t		rtnl_tc_get_handle(struct rtnl_tc *);
extern void		rtnl_tc_set_parent(struct rtnl_tc *, uint32_t);
extern uint32_t		rtnl_tc_get_parent(struct rtnl_tc *);
extern int		rtnl_tc_set_kind(struct rtnl_tc *, const char *);
extern char *		rtnl_tc_get_kind(struct rtnl_tc *);
extern uint64_t		rtnl_tc_get_stat(struct rtnl_tc *, enum rtnl_tc_stat);

extern int		rtnl_tc_calc_txtime(int, int);
extern int		rtnl_tc_calc_bufsize(int, int);
extern int		rtnl_tc_calc_cell_log(int);

extern int		rtnl_tc_read_classid_file(void);
%cstring_output_maxsize(char *buf, size_t len)
extern char *		rtnl_tc_handle2str(uint32_t, char *buf, size_t len);
extern int		rtnl_classid_generate(const char *, uint32_t *, uint32_t);

/* <netlink/route/qdisc.h> */

%inline %{
        struct nl_object *qdisc2obj(struct rtnl_qdisc *qdisc)
        {
                return OBJ_CAST(qdisc);
        }

        struct rtnl_qdisc *obj2qdisc(struct nl_object *obj)
        {
                return (struct rtnl_qdisc *) obj;
        }

        struct nl_object *class2obj(struct rtnl_class *cl)
        {
                return OBJ_CAST(cl);
        }

        struct rtnl_class *obj2class(struct nl_object *obj)
        {
                return (struct rtnl_class *) obj;
        }

        struct nl_object *cls2obj(struct rtnl_cls *cls)
        {
                return OBJ_CAST(cls);
        }

        struct rtnl_cls *obj2cls(struct nl_object *obj)
        {
                return (struct rtnl_cls *) obj;
        }

        struct rtnl_tc *obj2tc(struct nl_object *obj)
        {
                return TC_CAST(obj);
        }
%};
extern struct rtnl_qdisc *
		rtnl_qdisc_alloc(void);

extern struct rtnl_qdisc *
		rtnl_qdisc_get(struct nl_cache *, int, uint32_t);

extern struct rtnl_qdisc *
		rtnl_qdisc_get_by_parent(struct nl_cache *, int, uint32_t);

extern int	rtnl_qdisc_build_add_request(struct rtnl_qdisc *, int,
					     struct nl_msg **);
extern int	rtnl_qdisc_add(struct nl_sock *, struct rtnl_qdisc *, int);

extern int	rtnl_qdisc_build_update_request(struct rtnl_qdisc *,
						struct rtnl_qdisc *,
						int, struct nl_msg **);

extern int	rtnl_qdisc_update(struct nl_sock *, struct rtnl_qdisc *,
				  struct rtnl_qdisc *, int);

extern int	rtnl_qdisc_build_delete_request(struct rtnl_qdisc *,
						struct nl_msg **);
extern int	rtnl_qdisc_delete(struct nl_sock *, struct rtnl_qdisc *);

/* <netlink/route/classifier.h> */

extern struct rtnl_cls *rtnl_cls_alloc(void);
extern void		rtnl_cls_put(struct rtnl_cls *);

extern int		rtnl_cls_add(struct nl_sock *, struct rtnl_cls *, int);

extern int		rtnl_cls_delete(struct nl_sock *, struct rtnl_cls *,
					int);

extern void		rtnl_cls_set_prio(struct rtnl_cls *, uint16_t);
extern uint16_t		rtnl_cls_get_prio(struct rtnl_cls *);

extern void		rtnl_cls_set_protocol(struct rtnl_cls *, uint16_t);
extern uint16_t		rtnl_cls_get_protocol(struct rtnl_cls *);

/* <netlink/route/qdisc/htb.h> */

extern uint32_t	rtnl_htb_get_rate2quantum(struct rtnl_qdisc *);
extern int	rtnl_htb_set_rate2quantum(struct rtnl_qdisc *, uint32_t);
extern uint32_t	rtnl_htb_get_defcls(struct rtnl_qdisc *);
extern int	rtnl_htb_set_defcls(struct rtnl_qdisc *, uint32_t);

extern uint32_t	rtnl_htb_get_prio(struct rtnl_class *);
extern int	rtnl_htb_set_prio(struct rtnl_class *, uint32_t);
extern uint32_t	rtnl_htb_get_rate(struct rtnl_class *);
extern int	rtnl_htb_set_rate(struct rtnl_class *, uint32_t);
extern uint32_t	rtnl_htb_get_ceil(struct rtnl_class *);
extern int	rtnl_htb_set_ceil(struct rtnl_class *, uint32_t);
extern uint32_t	rtnl_htb_get_rbuffer(struct rtnl_class *);
extern int	rtnl_htb_set_rbuffer(struct rtnl_class *, uint32_t);
extern uint32_t	rtnl_htb_get_cbuffer(struct rtnl_class *);
extern int	rtnl_htb_set_cbuffer(struct rtnl_class *, uint32_t);
extern uint32_t	rtnl_htb_get_quantum(struct rtnl_class *);
extern int	rtnl_htb_set_quantum(struct rtnl_class *, uint32_t);
extern int	rtnl_htb_get_level(struct rtnl_class *);

/* <netlink/route/addr.h> */

%inline %{
        struct nl_object *addr2obj(struct rtnl_addr *addr)
        {
                return OBJ_CAST(addr);
        }

        struct rtnl_addr *obj2addr(struct nl_object *obj)
        {
                return (struct rtnl_addr *) obj;
        }
%};

extern struct rtnl_addr *rtnl_addr_alloc(void);

extern struct rtnl_addr *
		rtnl_addr_get(struct nl_cache *, int, struct nl_addr *);

extern int	rtnl_addr_build_add_request(struct rtnl_addr *, int,
					    struct nl_msg **);
extern int	rtnl_addr_add(struct nl_sock *, struct rtnl_addr *, int);

extern int	rtnl_addr_build_delete_request(struct rtnl_addr *, int,
					       struct nl_msg **);
extern int	rtnl_addr_delete(struct nl_sock *,
				 struct rtnl_addr *, int);

%cstring_output_maxsize(char *buf, size_t len)
extern char *	rtnl_addr_flags2str(int, char *buf, size_t len);
extern int	rtnl_addr_str2flags(const char *);

extern int	rtnl_addr_set_label(struct rtnl_addr *, const char *);
extern char *	rtnl_addr_get_label(struct rtnl_addr *);

extern void	rtnl_addr_set_ifindex(struct rtnl_addr *, int);
extern int	rtnl_addr_get_ifindex(struct rtnl_addr *);

extern void	rtnl_addr_set_link(struct rtnl_addr *, struct rtnl_link *);
extern struct rtnl_link *
		rtnl_addr_get_link(struct rtnl_addr *);
extern void	rtnl_addr_set_family(struct rtnl_addr *, int);
extern int	rtnl_addr_get_family(struct rtnl_addr *);

extern void	rtnl_addr_set_prefixlen(struct rtnl_addr *, int);
extern int	rtnl_addr_get_prefixlen(struct rtnl_addr *);

extern void	rtnl_addr_set_scope(struct rtnl_addr *, int);
extern int	rtnl_addr_get_scope(struct rtnl_addr *);

extern void	rtnl_addr_set_flags(struct rtnl_addr *, unsigned int);
extern void	rtnl_addr_unset_flags(struct rtnl_addr *, unsigned int);
extern unsigned int rtnl_addr_get_flags(struct rtnl_addr *);

extern int	rtnl_addr_set_local(struct rtnl_addr *,
					    struct nl_addr *);
extern struct nl_addr *rtnl_addr_get_local(struct rtnl_addr *);

extern int	rtnl_addr_set_peer(struct rtnl_addr *, struct nl_addr *);
extern struct nl_addr *rtnl_addr_get_peer(struct rtnl_addr *);

extern int	rtnl_addr_set_broadcast(struct rtnl_addr *, struct nl_addr *);
extern struct nl_addr *rtnl_addr_get_broadcast(struct rtnl_addr *);

extern int	rtnl_addr_set_multicast(struct rtnl_addr *, struct nl_addr *);
extern struct nl_addr *rtnl_addr_get_multicast(struct rtnl_addr *);

extern int	rtnl_addr_set_anycast(struct rtnl_addr *, struct nl_addr *);
extern struct nl_addr *rtnl_addr_get_anycast(struct rtnl_addr *);

extern uint32_t rtnl_addr_get_valid_lifetime(struct rtnl_addr *);
extern void	rtnl_addr_set_valid_lifetime(struct rtnl_addr *, uint32_t);
extern uint32_t rtnl_addr_get_preferred_lifetime(struct rtnl_addr *);
extern void	rtnl_addr_set_preferred_lifetime(struct rtnl_addr *, uint32_t);
extern uint32_t rtnl_addr_get_create_time(struct rtnl_addr *);
extern uint32_t rtnl_addr_get_last_update_time(struct rtnl_addr *);
