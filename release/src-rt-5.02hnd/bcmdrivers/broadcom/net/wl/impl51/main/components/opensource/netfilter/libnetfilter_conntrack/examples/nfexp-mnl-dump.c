#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>

#include <libmnl/libmnl.h>
#include <libnetfilter_conntrack/libnetfilter_conntrack.h>

static int data_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nf_expect *exp;
	uint32_t type = NFCT_T_UNKNOWN;
	char buf[4096];

	exp = nfexp_new();
	if (exp == NULL)
		return MNL_CB_OK;

	if (nfexp_nlmsg_parse(nlh, exp) < 0) {
		perror("failed to parse message from kernel");
		return MNL_CB_ERROR;
	}

	nfexp_snprintf(buf, sizeof(buf), exp, type, NFCT_O_DEFAULT, 0);
	printf("%s\n", buf);

	nfexp_destroy(exp);

	return MNL_CB_OK;
}

int main(void)
{
	struct mnl_socket *nl;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	struct nfgenmsg *nfh;
	uint32_t seq, portid;
	int ret;

	nl = mnl_socket_open(NETLINK_NETFILTER);
	if (nl == NULL) {
		perror("mnl_socket_open");
		exit(EXIT_FAILURE);
	}

	if (mnl_socket_bind(nl, 0, MNL_SOCKET_AUTOPID) < 0) {
		perror("mnl_socket_bind");
		exit(EXIT_FAILURE);
	}

	portid = mnl_socket_get_portid(nl);

	nlh = mnl_nlmsg_put_header(buf);
	nlh->nlmsg_type = (NFNL_SUBSYS_CTNETLINK_EXP << 8) | IPCTNL_MSG_EXP_GET;
	nlh->nlmsg_flags = NLM_F_REQUEST|NLM_F_DUMP|NLM_F_ACK;
	nlh->nlmsg_seq = seq = time(NULL);

	nfh = mnl_nlmsg_put_extra_header(nlh, sizeof(struct nfgenmsg));
	nfh->nfgen_family = AF_INET;
	nfh->version = NFNETLINK_V0;
	nfh->res_id = 0;

	ret = mnl_socket_sendto(nl, nlh, nlh->nlmsg_len);
	if (ret == -1) {
		perror("mnl_socket_recvfrom");
		exit(EXIT_FAILURE);
	}

	ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	while (1) {
		ret = mnl_cb_run(buf, ret, seq, portid, data_cb, NULL);
		if (ret <= MNL_CB_STOP)
			break;

		ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	}
	if (ret == -1) {
		perror("mnl_socket_recvfrom");
		exit(EXIT_FAILURE);
	}

	mnl_socket_close(nl);

	return 0;
}
