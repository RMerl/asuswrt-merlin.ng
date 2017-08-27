extern int get_operstate(const char *name);
extern int print_linkinfo(const struct sockaddr_nl *who,
			  struct nlmsghdr *n,
			  void *arg);
extern int print_addrinfo(const struct sockaddr_nl *who,
			  struct nlmsghdr *n,
			  void *arg);
extern int print_addrlabel(const struct sockaddr_nl *who,
			   struct nlmsghdr *n, void *arg);
extern int print_neigh(const struct sockaddr_nl *who,
		       struct nlmsghdr *n, void *arg);
extern int print_ntable(const struct sockaddr_nl *who,
			struct nlmsghdr *n, void *arg);
extern int ipaddr_list(int argc, char **argv);
extern int ipaddr_list_link(int argc, char **argv);
extern int iproute_monitor(int argc, char **argv);
extern void iplink_usage(void) __attribute__((noreturn));
extern void iproute_reset_filter(void);
extern void ipmroute_reset_filter(void);
extern void ipaddr_reset_filter(int);
extern void ipneigh_reset_filter(void);
extern void ipntable_reset_filter(void);
extern int print_route(const struct sockaddr_nl *who,
		       struct nlmsghdr *n, void *arg);
extern int print_mroute(const struct sockaddr_nl *who,
			struct nlmsghdr *n, void *arg);
extern int print_prefix(const struct sockaddr_nl *who,
			struct nlmsghdr *n, void *arg);
extern int print_rule(const struct sockaddr_nl *who,
		      struct nlmsghdr *n, void *arg);
extern int print_netconf(const struct sockaddr_nl *who,
			 struct nlmsghdr *n, void *arg);
extern int do_ipaddr(int argc, char **argv);
extern int do_ipaddrlabel(int argc, char **argv);
extern int do_iproute(int argc, char **argv);
extern int do_iprule(int argc, char **argv);
extern int do_ipneigh(int argc, char **argv);
extern int do_ipntable(int argc, char **argv);
extern int do_iptunnel(int argc, char **argv);
extern int do_ip6tunnel(int argc, char **argv);
extern int do_iptuntap(int argc, char **argv);
extern int do_iplink(int argc, char **argv);
extern int do_ipmonitor(int argc, char **argv);
extern int do_multiaddr(int argc, char **argv);
extern int do_multiroute(int argc, char **argv);
extern int do_multirule(int argc, char **argv);
extern int do_netns(int argc, char **argv);
extern int do_xfrm(int argc, char **argv);
extern int do_ipl2tp(int argc, char **argv);
extern int do_tcp_metrics(int argc, char **argv);
extern int do_ipnetconf(int argc, char **argv);
extern int do_iptoken(int argc, char **argv);

static inline int rtm_get_table(struct rtmsg *r, struct rtattr **tb)
{
	__u32 table = r->rtm_table;
	if (tb[RTA_TABLE])
		table = rta_getattr_u32(tb[RTA_TABLE]);
	return table;
}

extern struct rtnl_handle rth;

#include <stdbool.h>

struct link_util
{
	struct link_util	*next;
	const char		*id;
	int			maxattr;
	int			(*parse_opt)(struct link_util *, int, char **,
					     struct nlmsghdr *);
	void			(*print_opt)(struct link_util *, FILE *,
					     struct rtattr *[]);
	void			(*print_xstats)(struct link_util *, FILE *,
					     struct rtattr *);
	bool			slave;
};

struct link_util *get_link_kind(const char *kind);
struct link_util *get_link_slave_kind(const char *slave_kind);
int get_netns_fd(const char *name);

#ifndef	INFINITY_LIFE_TIME
#define     INFINITY_LIFE_TIME      0xFFFFFFFFU
#endif
