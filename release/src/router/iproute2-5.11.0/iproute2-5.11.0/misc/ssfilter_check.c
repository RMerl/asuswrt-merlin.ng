#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "libnetlink.h"
#include "ssfilter.h"
#include "ss_util.h"

static int dummy_filter(struct nlmsghdr *n, void *arg)
{
	/* just stops rtnl_dump_filter() */
	return -1;
}

static bool cgroup_filter_check(void)
{
	struct sockaddr_nl nladdr = { .nl_family = AF_NETLINK };
	DIAG_REQUEST(req, struct inet_diag_req_v2 r);
	struct instr {
		struct inet_diag_bc_op op;
		__u64 cgroup_id;
	} __attribute__((packed));
	int inslen = sizeof(struct instr);
	struct instr instr = {
		{ INET_DIAG_BC_CGROUP_COND, inslen, inslen + 4 },
		0
	};
	struct rtnl_handle rth;
	struct iovec iov[3];
	struct msghdr msg;
	struct rtattr rta;
	int ret = false;
	int iovlen = 3;

	if (rtnl_open_byproto(&rth, 0, NETLINK_SOCK_DIAG))
		return false;
	rth.dump = MAGIC_SEQ;
	rth.flags = RTNL_HANDLE_F_SUPPRESS_NLERR;

	memset(&req.r, 0, sizeof(req.r));
	req.r.sdiag_family = AF_INET;
	req.r.sdiag_protocol = IPPROTO_TCP;
	req.nlh.nlmsg_len += RTA_LENGTH(inslen);

	rta.rta_type = INET_DIAG_REQ_BYTECODE;
	rta.rta_len = RTA_LENGTH(inslen);

	iov[0] = (struct iovec) { &req, sizeof(req) };
	iov[1] = (struct iovec) { &rta, sizeof(rta) };
	iov[2] = (struct iovec) { &instr, inslen };

	msg = (struct msghdr) {
		.msg_name = (void *)&nladdr,
		.msg_namelen = sizeof(nladdr),
		.msg_iov = iov,
		.msg_iovlen = iovlen,
	};

	if (sendmsg(rth.fd, &msg, 0) < 0)
		goto out;

	if (rtnl_dump_filter(&rth, dummy_filter, NULL) < 0) {
		ret = (errno != EINVAL);
		goto out;
	}

	ret = true;

out:
	rtnl_close(&rth);

	return ret;
}


struct filter_check_t {
	bool (*check)(void);
	int checked:1,
	    supported:1;
};

static struct filter_check_t filter_checks[SSF__MAX] = {
	[SSF_CGROUPCOND] = { cgroup_filter_check, 0 },
};

bool ssfilter_is_supported(int type)
{
	struct filter_check_t f;

	if (type >= SSF__MAX)
		return false;

	f = filter_checks[type];
	if (!f.check)
		return true;

	if (!f.checked) {
		f.supported = f.check();
		f.checked = 1;
	}

	return f.supported;
}
