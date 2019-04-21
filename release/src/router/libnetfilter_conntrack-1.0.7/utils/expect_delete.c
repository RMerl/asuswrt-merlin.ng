#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>

#include <libnetfilter_conntrack/libnetfilter_conntrack.h>

int main(void)
{
	int ret;
	struct nfct_handle *h;
	struct nf_conntrack *expected;
	struct nf_expect *exp;

	expected = nfct_new();
	if (!expected) {
		perror("nfct_new");
		exit(EXIT_FAILURE);
	}

	nfct_set_attr_u8(expected, ATTR_L3PROTO, AF_INET);
	nfct_set_attr_u32(expected, ATTR_IPV4_SRC, inet_addr("1.1.1.1"));
	nfct_set_attr_u32(expected, ATTR_IPV4_DST, inet_addr("2.2.2.2"));

	nfct_set_attr_u8(expected, ATTR_L4PROTO, IPPROTO_TCP);
	nfct_set_attr_u16(expected, ATTR_PORT_SRC, 0);
	nfct_set_attr_u16(expected, ATTR_PORT_DST, htons(10241));

	exp = nfexp_new();
	if (!exp) {
		perror("nfexp_new");
		nfct_destroy(expected);
		exit(EXIT_FAILURE);
	}

	nfexp_set_attr(exp, ATTR_EXP_EXPECTED, expected);

	h = nfct_open(EXPECT, 0);
	if (!h) {
		perror("nfct_open");
		nfct_destroy(expected);
		return -1;
	}

	ret = nfexp_query(h, NFCT_Q_DESTROY, exp);

	printf("TEST: delete expectation ");
	if (ret == -1)
		printf("(%d)(%s)\n", ret, strerror(errno));
	else
		printf("(OK)\n");

	nfct_close(h);

	nfct_destroy(expected);

	ret == -1 ? exit(EXIT_FAILURE) : exit(EXIT_SUCCESS);
}
