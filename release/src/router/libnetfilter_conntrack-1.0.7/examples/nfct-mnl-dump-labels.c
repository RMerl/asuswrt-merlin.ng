#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <libmnl/libmnl.h>
#include <libnetfilter_conntrack/libnetfilter_conntrack.h>

static void print_label(struct nf_conntrack *ct, struct nfct_labelmap *map)
{
	unsigned int i, max;
	const struct nfct_bitmask *b = nfct_get_attr(ct, ATTR_CONNLABELS);
	if (!b)
		return;

	puts("labels:");
	max = nfct_bitmask_maxbit(b);
	for (i = 0; i <= max; i++) {
		if (nfct_bitmask_test_bit(b, i))
			printf("\t'%s' (%d)\n", map ? nfct_labelmap_get_name(map, i) : "", i);
	}
}

static int data_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nf_conntrack *ct;
	char buf[4096];

	ct = nfct_new();
	if (ct == NULL)
		return MNL_CB_OK;

	nfct_nlmsg_parse(nlh, ct);

	nfct_snprintf(buf, sizeof(buf), ct, NFCT_T_UNKNOWN, NFCT_O_DEFAULT, 0);
	printf("%s\n", buf);

	print_label(ct, data);

	nfct_destroy(ct);

	return MNL_CB_OK;
}

int main(void)
{
	struct mnl_socket *nl;
	struct nlmsghdr *nlh;
	struct nfgenmsg *nfh;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	unsigned int seq, portid;
	int ret;
	struct nfct_labelmap *l = nfct_labelmap_new(NULL);

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
	nlh->nlmsg_type = (NFNL_SUBSYS_CTNETLINK << 8) | IPCTNL_MSG_CT_GET;
	nlh->nlmsg_flags = NLM_F_REQUEST|NLM_F_DUMP;
	nlh->nlmsg_seq = seq = time(NULL);

	nfh = mnl_nlmsg_put_extra_header(nlh, sizeof(struct nfgenmsg));
	nfh->nfgen_family = AF_UNSPEC;
	nfh->version = NFNETLINK_V0;
	nfh->res_id = 0;


	ret = mnl_socket_sendto(nl, nlh, nlh->nlmsg_len);
	if (ret == -1) {
		perror("mnl_socket_sendto");
		exit(EXIT_FAILURE);
	}

	ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));


	while (ret > 0) {
		ret = mnl_cb_run(buf, ret, seq, portid, data_cb, l);
		if (ret <= MNL_CB_STOP)
			break;
		ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	}
	if (ret == -1) {
		perror("mnl_socket_recvfrom");
		exit(EXIT_FAILURE);
	}

	if (l)
		nfct_labelmap_destroy(l);

	mnl_socket_close(nl);

	return 0;
}
