/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __LIBKNETLINK_H__
#define __LIBKNETLINK_H__

#include <linux/net.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

//#define TC_NL_DEBUG_ENABLE
#if defined(TC_NL_DEBUG_ENABLE)
#define DBG_PRINT(fmt, arg...) \
        printk(fmt, ##arg); \

#define TC_NL_ERROR(rc, fmt, arg...) DBG_PRINT(fmt " (rc %d)", ##arg, rc)
#define TC_NL_PRINT(fmt, arg...) DBG_PRINT(fmt, ##arg)
#else
#define TC_NL_ERROR(rc, fmt, arg...)
#define TC_NL_PRINT(fmt, arg...)
#endif

#define MAX_MSG 512

struct rtnl_handle {
    struct socket *sock;
    struct sockaddr_nl  local;
    time64_t seq;
};

int rtnl_open(struct rtnl_handle *rth, unsigned int subscriptions)
    __attribute__((warn_unused_result));

int rtnl_open_byproto(struct rtnl_handle *rth, unsigned int subscriptions,
                 int protocol)
    __attribute__((warn_unused_result));
void rtnl_close(struct rtnl_handle *rth);

int rtnl_talk(struct rtnl_handle *rtnl, struct nlmsghdr *n)
    __attribute__((warn_unused_result));

int addattr8(struct nlmsghdr *n, int maxlen, int type, __u8 data);
int addattr16(struct nlmsghdr *n, int maxlen, int type, __u16 data);
int addattr32(struct nlmsghdr *n, int maxlen, int type, __u32 data);
int addattr_l(struct nlmsghdr *n, int maxlen, int type,
          const void *data, int alen);
struct rtattr *addattr_nest(struct nlmsghdr *n, int maxlen, int type);
int addattr_nest_end(struct nlmsghdr *n, struct rtattr *nest);

#define NLMSG_TAIL(nmsg) \
    ((struct rtattr *) (((void *) (nmsg)) + NLMSG_ALIGN((nmsg)->nlmsg_len)))

#endif /* __LIBKNETLINK_H__ */
