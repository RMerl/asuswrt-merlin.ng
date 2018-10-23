#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <libmnl/libmnl.h>
#include <libnetfilter_conntrack/libnetfilter_conntrack.h>

struct callback_args {
	struct mnl_socket *nl;
	unsigned int seq;
	int bit;
};

static void set_label(struct nf_conntrack *ct, struct callback_args *cbargs)
{
	struct nfct_bitmask *b = (void *) nfct_get_attr(ct, ATTR_CONNLABELS);
	int bit = cbargs->bit;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	struct nfgenmsg *nfh;

	if (b) {
		if (bit < 0)
			b = nfct_bitmask_new(0);
		else if (nfct_bitmask_test_bit(b, bit))
			return;
	} else {
		b = nfct_bitmask_new(0);
	}

	if (!b)
		return;
	if (bit >= 0)
		nfct_bitmask_set_bit(b, bit);
	nfct_set_attr(ct, ATTR_CONNLABELS, b);

	if (bit >= 0) {
		b = nfct_bitmask_new(bit);
		if (b) {
			nfct_bitmask_set_bit(b, bit);
			nfct_set_attr(ct, ATTR_CONNLABELS_MASK, b);
		}
	}

	cbargs->seq++;

	nlh = mnl_nlmsg_put_header(buf);
	nlh->nlmsg_type = (NFNL_SUBSYS_CTNETLINK << 8) | IPCTNL_MSG_CT_NEW;
	nlh->nlmsg_flags = NLM_F_REQUEST|NLM_F_CREATE;
	nlh->nlmsg_seq = cbargs->seq;

	nfh = mnl_nlmsg_put_extra_header(nlh, sizeof(struct nfgenmsg));
	nfh->nfgen_family = nfct_get_attr_u8(ct, ATTR_L3PROTO);
	nfh->version = NFNETLINK_V0;
	nfh->res_id = 0;

	nfct_nlmsg_build(nlh, ct);

	if (mnl_socket_sendto(cbargs->nl, nlh, nlh->nlmsg_len) < 0)
		perror("mnl_socket_sendto");
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

	set_label(ct, data);

	nfct_destroy(ct);

	return MNL_CB_OK;
}

static void show_labels(struct nfct_labelmap *l)
{
	unsigned int i = 0;
	const char *name;

	if (l) {
		fputs("usage: program label, configured labels are:\n", stderr);
		while ((name = nfct_labelmap_get_name(l, i))) {
			if (*name)
				fprintf(stderr, "%s -> bit %d\n", name, i);
			i++;
		}
	} else {
		fputs("no labels configured, usage: program bit\n", stderr);
	}
	exit(1);
}

static struct mnl_socket *sock_nl_create(void)
{
	struct mnl_socket *nl;

	nl = mnl_socket_open(NETLINK_NETFILTER);
	if (nl == NULL) {
		perror("mnl_socket_open");
		exit(EXIT_FAILURE);
	}

	if (mnl_socket_bind(nl, 0, MNL_SOCKET_AUTOPID) < 0) {
		perror("mnl_socket_bind");
		exit(EXIT_FAILURE);
	}

	return nl;
}

int main(int argc, char *argv[])
{
	struct mnl_socket *nl;
	struct nlmsghdr *nlh;
	struct nfgenmsg *nfh;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	unsigned int seq, portid;
	struct callback_args cbargs;
	int ret;
	struct nfct_labelmap *l = nfct_labelmap_new(NULL);

	if (argc < 2)
		show_labels(l);

	cbargs.bit = l ? nfct_labelmap_get_bit(l, argv[1]) : -1;

	if (cbargs.bit < 0) {
		cbargs.bit = atoi(argv[1]);
		if (cbargs.bit == 0 && argv[1][0] != '0')
			show_labels(l);
	}

	if (cbargs.bit < 0)
		puts("will clear all labels");
	else
		printf("will set label bit %d\n", cbargs.bit);

	nl = sock_nl_create();
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


	cbargs.nl = sock_nl_create();
	cbargs.seq = seq;

	while (ret > 0) {
		ret = mnl_cb_run(buf, ret, seq, portid, data_cb, &cbargs);
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
