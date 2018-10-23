#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>

#include <libnetfilter_conntrack/libnetfilter_conntrack.h>
#include <libnetfilter_conntrack/libnetfilter_conntrack_tcp.h>

int main(void)
{
	int ret;
	struct nfct_handle *h;
	struct nf_conntrack *ct;

	ct = nfct_new();
	if (!ct) {
		perror("nfct_new");
		return 0;
	}

	struct nfct_attr_grp_ipv4 grp_ipv4 = {
		.src = inet_addr("1.1.1.1"),
		.dst = inet_addr("2.2.2.2")
	};
	nfct_set_attr_grp(ct, ATTR_GRP_ORIG_IPV4, &grp_ipv4);

	struct nfct_attr_grp_port grp_port = {
		.sport = htons(20),
		.dport = htons(10)
	};
	nfct_set_attr_grp(ct, ATTR_GRP_ORIG_PORT, &grp_port);
	nfct_set_attr_u8(ct, ATTR_L4PROTO, IPPROTO_TCP);

	nfct_setobjopt(ct, NFCT_SOPT_SETUP_REPLY);

	nfct_set_attr_u8(ct, ATTR_TCP_STATE, TCP_CONNTRACK_SYN_SENT);
	nfct_set_attr_u32(ct, ATTR_TIMEOUT, 100);
	nfct_set_attr(ct, ATTR_HELPER_NAME, "ftp");

	h = nfct_open(CONNTRACK, 0);
	if (!h) {
		perror("nfct_open");
		nfct_destroy(ct);
		return -1;
	}

	ret = nfct_query(h, NFCT_Q_CREATE, ct);

	printf("TEST: create conntrack ");
	if (ret == -1) 
		printf("(%d)(%s)\n", ret, strerror(errno));
	else
		printf("(OK)\n");

	nfct_close(h);

	nfct_destroy(ct);

	ret == -1 ? exit(EXIT_FAILURE) : exit(EXIT_SUCCESS);
}
