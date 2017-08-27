#ifndef __LIBNETLINK_H__
#define __LIBNETLINK_H__ 1

#include <stdio.h>
#include <string.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/if_link.h>
#include <linux/if_addr.h>
#include <linux/neighbour.h>
#include <linux/netconf.h>

struct rtnl_handle
{
	int			fd;
	struct sockaddr_nl	local;
	struct sockaddr_nl	peer;
	__u32			seq;
	__u32			dump;
	int			proto;
	FILE		       *dump_fp;
#define RTNL_HANDLE_F_LISTEN_ALL_NSID		0x01
	int			flags;
};

extern int rcvbuf;

int rtnl_open(struct rtnl_handle *rth, unsigned subscriptions)
	__attribute__((warn_unused_result));

int rtnl_open_byproto(struct rtnl_handle *rth, unsigned subscriptions,
			     int protocol)
	__attribute__((warn_unused_result));

void rtnl_close(struct rtnl_handle *rth);
int rtnl_wilddump_request(struct rtnl_handle *rth, int fam, int type)
	__attribute__((warn_unused_result));
int rtnl_wilddump_req_filter(struct rtnl_handle *rth, int fam, int type,
				    __u32 filt_mask)
	__attribute__((warn_unused_result));
int rtnl_dump_request(struct rtnl_handle *rth, int type, void *req,
			     int len)
	__attribute__((warn_unused_result));

struct rtnl_ctrl_data {
	int	nsid;
};

typedef int (*rtnl_filter_t)(const struct sockaddr_nl *,
			     struct nlmsghdr *n, void *);

typedef int (*rtnl_listen_filter_t)(const struct sockaddr_nl *,
				    struct rtnl_ctrl_data *,
				    struct nlmsghdr *n, void *);

struct rtnl_dump_filter_arg
{
	rtnl_filter_t filter;
	void *arg1;
};

int rtnl_dump_filter_l(struct rtnl_handle *rth,
			      const struct rtnl_dump_filter_arg *arg);
int rtnl_dump_filter(struct rtnl_handle *rth, rtnl_filter_t filter, void *arg);
int rtnl_talk(struct rtnl_handle *rtnl, struct nlmsghdr *n,
	      struct nlmsghdr *answer, size_t len)
	__attribute__((warn_unused_result));
int rtnl_send(struct rtnl_handle *rth, const void *buf, int)
	__attribute__((warn_unused_result));
int rtnl_send_check(struct rtnl_handle *rth, const void *buf, int)
	__attribute__((warn_unused_result));

int addattr(struct nlmsghdr *n, int maxlen, int type);
int addattr8(struct nlmsghdr *n, int maxlen, int type, __u8 data);
int addattr16(struct nlmsghdr *n, int maxlen, int type, __u16 data);
int addattr32(struct nlmsghdr *n, int maxlen, int type, __u32 data);
int addattr64(struct nlmsghdr *n, int maxlen, int type, __u64 data);
int addattrstrz(struct nlmsghdr *n, int maxlen, int type, const char *data);

int addattr_l(struct nlmsghdr *n, int maxlen, int type,
	      const void *data, int alen);
int addraw_l(struct nlmsghdr *n, int maxlen, const void *data, int len);
struct rtattr *addattr_nest(struct nlmsghdr *n, int maxlen, int type);
int addattr_nest_end(struct nlmsghdr *n, struct rtattr *nest);
struct rtattr *addattr_nest_compat(struct nlmsghdr *n, int maxlen, int type,
				   const void *data, int len);
int addattr_nest_compat_end(struct nlmsghdr *n, struct rtattr *nest);
int rta_addattr8(struct rtattr *rta, int maxlen, int type, __u8 data);
int rta_addattr16(struct rtattr *rta, int maxlen, int type, __u16 data);
int rta_addattr32(struct rtattr *rta, int maxlen, int type, __u32 data);
int rta_addattr64(struct rtattr *rta, int maxlen, int type, __u64 data);
int rta_addattr_l(struct rtattr *rta, int maxlen, int type,
		  const void *data, int alen);

int parse_rtattr(struct rtattr *tb[], int max, struct rtattr *rta, int len);
int parse_rtattr_flags(struct rtattr *tb[], int max, struct rtattr *rta,
			      int len, unsigned short flags);
int parse_rtattr_byindex(struct rtattr *tb[], int max,
			 struct rtattr *rta, int len);
struct rtattr *parse_rtattr_one(int type, struct rtattr *rta, int len);
int __parse_rtattr_nested_compat(struct rtattr *tb[], int max, struct rtattr *rta, int len);

struct rtattr *rta_nest(struct rtattr *rta, int maxlen, int type);
int rta_nest_end(struct rtattr *rta, struct rtattr *nest);

#define RTA_TAIL(rta) \
		((struct rtattr *) (((void *) (rta)) + \
				    RTA_ALIGN((rta)->rta_len)))

#define parse_rtattr_nested(tb, max, rta) \
	(parse_rtattr((tb), (max), RTA_DATA(rta), RTA_PAYLOAD(rta)))

#define parse_rtattr_one_nested(type, rta) \
	(parse_rtattr_one(type, RTA_DATA(rta), RTA_PAYLOAD(rta)))

#define parse_rtattr_nested_compat(tb, max, rta, data, len) \
	({ data = RTA_PAYLOAD(rta) >= len ? RTA_DATA(rta) : NULL;	\
		__parse_rtattr_nested_compat(tb, max, rta, len); })

static inline __u8 rta_getattr_u8(const struct rtattr *rta)
{
	return *(__u8 *)RTA_DATA(rta);
}
static inline __u16 rta_getattr_u16(const struct rtattr *rta)
{
	return *(__u16 *)RTA_DATA(rta);
}
static inline __u32 rta_getattr_u32(const struct rtattr *rta)
{
	return *(__u32 *)RTA_DATA(rta);
}
static inline __u64 rta_getattr_u64(const struct rtattr *rta)
{
	__u64 tmp;
	memcpy(&tmp, RTA_DATA(rta), sizeof(__u64));
	return tmp;
}
static inline const char *rta_getattr_str(const struct rtattr *rta)
{
	return (const char *)RTA_DATA(rta);
}

int rtnl_listen_all_nsid(struct rtnl_handle *);
int rtnl_listen(struct rtnl_handle *, rtnl_listen_filter_t handler,
		void *jarg);
int rtnl_from_file(FILE *, rtnl_listen_filter_t handler,
		   void *jarg);

#define NLMSG_TAIL(nmsg) \
	((struct rtattr *) (((void *) (nmsg)) + NLMSG_ALIGN((nmsg)->nlmsg_len)))

#ifndef IFA_RTA
#define IFA_RTA(r) \
	((struct rtattr*)(((char*)(r)) + NLMSG_ALIGN(sizeof(struct ifaddrmsg))))
#endif
#ifndef IFA_PAYLOAD
#define IFA_PAYLOAD(n)	NLMSG_PAYLOAD(n,sizeof(struct ifaddrmsg))
#endif

#ifndef IFLA_RTA
#define IFLA_RTA(r) \
	((struct rtattr*)(((char*)(r)) + NLMSG_ALIGN(sizeof(struct ifinfomsg))))
#endif
#ifndef IFLA_PAYLOAD
#define IFLA_PAYLOAD(n)	NLMSG_PAYLOAD(n,sizeof(struct ifinfomsg))
#endif

#ifndef NDA_RTA
#define NDA_RTA(r) \
	((struct rtattr*)(((char*)(r)) + NLMSG_ALIGN(sizeof(struct ndmsg))))
#endif
#ifndef NDA_PAYLOAD
#define NDA_PAYLOAD(n)	NLMSG_PAYLOAD(n,sizeof(struct ndmsg))
#endif

#ifndef NDTA_RTA
#define NDTA_RTA(r) \
	((struct rtattr*)(((char*)(r)) + NLMSG_ALIGN(sizeof(struct ndtmsg))))
#endif
#ifndef NDTA_PAYLOAD
#define NDTA_PAYLOAD(n) NLMSG_PAYLOAD(n,sizeof(struct ndtmsg))
#endif

#ifndef NETNS_RTA
#define NETNS_RTA(r) \
	((struct rtattr*)(((char*)(r)) + NLMSG_ALIGN(sizeof(struct rtgenmsg))))
#endif
#ifndef NETNS_PAYLOAD
#define NETNS_PAYLOAD(n)	NLMSG_PAYLOAD(n,sizeof(struct rtgenmsg))
#endif

/* User defined nlmsg_type which is used mostly for logging netlink
 * messages from dump file */
#define NLMSG_TSTAMP	15

#endif /* __LIBNETLINK_H__ */

