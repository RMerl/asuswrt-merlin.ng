/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * libnetlink.c RTnetlink service routines.
 *
 * Authors: Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru>
 */

#include "libnetlink.h"
#include <net/sock.h>

#ifndef __aligned
#define __aligned(x)        __attribute__((aligned(x)))
#endif

#ifndef SOL_NETLINK
#define SOL_NETLINK 270
#endif

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

typedef uint32_t socklen_t;

void rtnl_close(struct rtnl_handle *rth)
{
    if (rth->sock)
    {
        kernel_sock_shutdown(rth->sock, SHUT_RDWR);
        rth->sock = NULL;
    }
}

static int setsockopt(struct socket *sock, int level, int op,
            void *val, unsigned int len)
{
    sockptr_t optval = KERNEL_SOCKPTR(val);
    return sock_setsockopt(sock, level, op, optval, len);
}

int rtnl_open_byproto(struct rtnl_handle *rth, unsigned int subscriptions, int protocol)
{
    int rc;
    socklen_t addr_len;
    int sndbuf = 1024 * 2;
    int rcvbuf = 1024 * 2;
    int one = 1;

    memset(rth, 0, sizeof(*rth));

    rc = sock_create(AF_NETLINK, SOCK_RAW, protocol, &rth->sock);
    if(rc)
    {
        TC_NL_ERROR(rc, "Cannot open netlink socket");
        return -1;
    }

    rc = setsockopt(rth->sock, SOL_SOCKET, SO_SNDBUF, &sndbuf, sizeof(sndbuf));
    if (rc)
    {
        TC_NL_ERROR(rc, "SO_SNDBUF");
        goto err;
    }

    rc = setsockopt(rth->sock, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf));
    if (rc)
    {
        TC_NL_ERROR(rc, "SO_RCVBUF");
        goto err;
    }

    setsockopt(rth->sock, SOL_NETLINK, NETLINK_EXT_ACK, &one, sizeof(one));

    memset(&rth->local, 0, sizeof(rth->local));
    rth->local.nl_family = AF_NETLINK;
    rth->local.nl_groups = subscriptions;

    rc = kernel_bind(rth->sock, (struct sockaddr *)&rth->local, sizeof(rth->local));
    if (rc)
    {
        TC_NL_ERROR(rc, "Cannot bind netlink socket");
        goto err;
    }

    addr_len = kernel_getsockname(rth->sock, (struct sockaddr *)&rth->local);

    if (addr_len != sizeof(rth->local))
    {
        TC_NL_PRINT("Wrong address length %d\n", addr_len);
        goto err;
    }
    if (rth->local.nl_family != AF_NETLINK)
    {
        TC_NL_PRINT("Wrong address family %d\n", rth->local.nl_family);
        goto err;
    }
    rth->seq = ktime_get_seconds();
    return 0;
err:
    rtnl_close(rth);
    return rc;
}

int rtnl_open(struct rtnl_handle *rth, unsigned int subscriptions)
{
    return rtnl_open_byproto(rth, subscriptions, NETLINK_ROUTE);
}

static int __rtnl_talk(struct rtnl_handle *rtnl, struct nlmsghdr *n)
{
    struct kvec iov[1] = { {
        .iov_base = n,
        .iov_len = n->nlmsg_len
    } };
    size_t iovlen = 1;
    struct sockaddr_nl nladdr = { .nl_family = AF_NETLINK };
    struct msghdr msg = {
        .msg_name = &nladdr,
        .msg_namelen = sizeof(nladdr),
    };
    unsigned int seq = 0;
    struct nlmsghdr *h;
    int i, rc;
    char inbuf[256];
    struct kvec riov[1] = { {
        .iov_base = inbuf,
        .iov_len = sizeof(inbuf)
    } };

    for (i = 0; i < iovlen; i++)
    {
        h = iov[i].iov_base;
        h->nlmsg_seq = seq = ++rtnl->seq;
        h->nlmsg_flags |= NLM_F_ACK;
    }

    rc = kernel_sendmsg(rtnl->sock, &msg, iov, 1, iov[0].iov_len);
    if (rc != iov[0].iov_len)
    {
        TC_NL_ERROR(rc, "Cannot talk to rtnetlink");
        return -1;
    }

    i = 0;
    while (1)
    {
next:
        rc = kernel_recvmsg(rtnl->sock, &msg, riov, 1, riov[0].iov_len, MSG_PEEK | MSG_TRUNC);
        ++i;
        if (rc < 0)
            return rc;

        if (msg.msg_namelen != sizeof(nladdr))
        {
            TC_NL_PRINT("sender address length == %d\n", msg.msg_namelen);
            return -1;
        }
        for (h = (struct nlmsghdr *)inbuf; rc >= sizeof(*h); )
        {
            int len = h->nlmsg_len;
            int l = len - sizeof(*h);

            if (l < 0 || len > rc)
            {
                if (msg.msg_flags & MSG_TRUNC)
                {
                    TC_NL_PRINT("Truncated message\n");
                    return -1;
                }
                TC_NL_PRINT("!!!malformed message: len=%d\n", len);
                return -1;
            }

            if (nladdr.nl_pid != 0 ||
                h->nlmsg_pid != rtnl->local.nl_pid ||
                h->nlmsg_seq > seq || h->nlmsg_seq < seq - iovlen)
            {
                /* Don't forget to skip that message. */
                rc -= NLMSG_ALIGN(len);
                h = (struct nlmsghdr *)((char *)h + NLMSG_ALIGN(len));
                continue;
            }

            if (h->nlmsg_type == NLMSG_ERROR)
            {
                struct nlmsgerr *err = (struct nlmsgerr *)NLMSG_DATA(h);
                int error = err->error;

                if (l < sizeof(struct nlmsgerr))
                {
                    TC_NL_PRINT("ERROR truncated\n");
                    return -1;
                }

                if (i < iovlen)
                {
                    goto next;
                }

                if (error)
                    return -i;
                return 0;
            }
        }

        if (rc)
        {
            TC_NL_PRINT("!!!Remnant of size %d\n", rc);
            return -1;
        }
    }
}

int rtnl_talk(struct rtnl_handle *rtnl, struct nlmsghdr *n)
{
    return __rtnl_talk(rtnl, n);
}

int addattr8(struct nlmsghdr *n, int maxlen, int type, __u8 data)
{
    return addattr_l(n, maxlen, type, &data, sizeof(__u8));
}

int addattr16(struct nlmsghdr *n, int maxlen, int type, __u16 data)
{
    return addattr_l(n, maxlen, type, &data, sizeof(__u16));
}

int addattr32(struct nlmsghdr *n, int maxlen, int type, __u32 data)
{
    return addattr_l(n, maxlen, type, &data, sizeof(__u32));
}

int addattr_l(struct nlmsghdr *n, int maxlen, int type, const void *data,
          int alen)
{
    int len = RTA_LENGTH(alen);
    struct rtattr *rta;

    if (NLMSG_ALIGN(n->nlmsg_len) + RTA_ALIGN(len) > maxlen)
    {
        TC_NL_PRINT("addattr_l ERROR: message exceeded bound of %d\n", maxlen);
        return -1;
    }
    rta = NLMSG_TAIL(n);
    rta->rta_type = type;
    rta->rta_len = len;
    if (alen)
        memcpy(RTA_DATA(rta), data, alen);
    n->nlmsg_len = NLMSG_ALIGN(n->nlmsg_len) + RTA_ALIGN(len);
    return 0;
}

struct rtattr *addattr_nest(struct nlmsghdr *n, int maxlen, int type)
{
    struct rtattr *nest = NLMSG_TAIL(n);

    addattr_l(n, maxlen, type, NULL, 0);
    return nest;
}

int addattr_nest_end(struct nlmsghdr *n, struct rtattr *nest)
{
    nest->rta_len = (void *)NLMSG_TAIL(n) - (void *)nest;
    return n->nlmsg_len;
}

