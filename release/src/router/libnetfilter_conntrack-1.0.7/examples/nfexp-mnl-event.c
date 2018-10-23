#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <libmnl/libmnl.h>
#include <libnetfilter_conntrack/libnetfilter_conntrack.h>

static int data_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nf_expect *exp;
	uint32_t type = NFCT_T_UNKNOWN;
	char buf[4096];

	switch(nlh->nlmsg_type & 0xFF) {
	case IPCTNL_MSG_EXP_NEW:
		if (nlh->nlmsg_flags & (NLM_F_CREATE|NLM_F_EXCL))
			type = NFCT_T_NEW;
		else
			type = NFCT_T_UPDATE;
		break;
	case IPCTNL_MSG_EXP_DELETE:
		type = NFCT_T_DESTROY;
		break;
	}

	exp = nfexp_new();
	if (exp == NULL)
		return MNL_CB_OK;

	nfexp_nlmsg_parse(nlh, exp);

	nfexp_snprintf(buf, sizeof(buf), exp,
			type, NFCT_O_DEFAULT, 0);
	printf("%s\n", buf);

	nfexp_destroy(exp);

	return MNL_CB_OK;
}

int main(void)
{
	struct mnl_socket *nl;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	int ret;

	nl = mnl_socket_open(NETLINK_NETFILTER);
	if (nl == NULL) {
		perror("mnl_socket_open");
		exit(EXIT_FAILURE);
	}

	if (mnl_socket_bind(nl, NF_NETLINK_CONNTRACK_EXP_NEW |
				NF_NETLINK_CONNTRACK_EXP_UPDATE |
				NF_NETLINK_CONNTRACK_EXP_DESTROY,
				MNL_SOCKET_AUTOPID) < 0) {
		perror("mnl_socket_bind");
		exit(EXIT_FAILURE);
	}

	while (1) {
		ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
		if (ret == -1) {
			perror("mnl_socket_recvfrom");
			exit(EXIT_FAILURE);
		}

		ret = mnl_cb_run(buf, ret, 0, 0, data_cb, NULL);
		if (ret == -1) {
			perror("mnl_cb_run");
			exit(EXIT_FAILURE);
		}
	}

	mnl_socket_close(nl);

	return 0;
}
