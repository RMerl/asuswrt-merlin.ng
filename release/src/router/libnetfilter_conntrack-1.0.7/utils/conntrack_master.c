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
	struct nf_conntrack *ct, *expected;

	/* create master conntrack */
	ct = nfct_new();
	if (!ct) {
		perror("nfct_new");
		return 0;
	}

	nfct_set_attr_u8(ct, ATTR_L3PROTO, AF_INET);
	nfct_set_attr_u32(ct, ATTR_IPV4_SRC, inet_addr("1.1.1.1"));
	nfct_set_attr_u32(ct, ATTR_IPV4_DST, inet_addr("2.2.2.2"));
	
	nfct_set_attr_u8(ct, ATTR_L4PROTO, IPPROTO_TCP);
	nfct_set_attr_u16(ct, ATTR_PORT_SRC, htons(20));
	nfct_set_attr_u16(ct, ATTR_PORT_DST, htons(10));

	nfct_setobjopt(ct, NFCT_SOPT_SETUP_REPLY);

	nfct_set_attr_u8(ct, ATTR_TCP_STATE, TCP_CONNTRACK_SYN_SENT);
	nfct_set_attr_u32(ct, ATTR_TIMEOUT, 100);

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

	nfct_destroy(ct);

	if (ret == -1)
		exit(EXIT_FAILURE);

	/* setup confirmed conntrack */

	expected = nfct_new();
	if (!expected) {
		perror("nfct_new");
		return 0;
	}

	nfct_set_attr_u8(ct, ATTR_L3PROTO, AF_INET);
	nfct_set_attr_u32(ct, ATTR_IPV4_SRC, inet_addr("1.1.1.1"));
	nfct_set_attr_u32(ct, ATTR_IPV4_DST, inet_addr("2.2.2.2"));
	
	nfct_set_attr_u8(ct, ATTR_L4PROTO, IPPROTO_TCP);
	nfct_set_attr_u16(ct, ATTR_PORT_SRC, htons(1024));
	nfct_set_attr_u16(ct, ATTR_PORT_DST, htons(1025));

	nfct_setobjopt(ct, NFCT_SOPT_SETUP_REPLY);

	nfct_set_attr_u8(ct, ATTR_TCP_STATE, TCP_CONNTRACK_SYN_SENT);
	nfct_set_attr_u32(ct, ATTR_TIMEOUT, 100);

	/* my conntrack master is ... */

	nfct_set_attr_u8(ct, ATTR_MASTER_L3PROTO, AF_INET);
	nfct_set_attr_u32(ct, ATTR_MASTER_IPV4_SRC, inet_addr("1.1.1.1"));
	nfct_set_attr_u32(ct, ATTR_MASTER_IPV4_DST, inet_addr("2.2.2.2"));
	
	nfct_set_attr_u8(ct, ATTR_MASTER_L4PROTO, IPPROTO_TCP);
	nfct_set_attr_u16(ct, ATTR_MASTER_PORT_SRC, htons(20));
	nfct_set_attr_u16(ct, ATTR_MASTER_PORT_DST, htons(10));

	ret = nfct_query(h, NFCT_Q_CREATE, ct);

	printf("TEST: create confirmed conntrack ");
	if (ret == -1)
		printf("(%d)(%s)\n", ret, strerror(errno));
	else
		printf("(OK)\n");

	nfct_close(h);

	nfct_destroy(expected);

	ret == -1 ? exit(EXIT_FAILURE) : exit(EXIT_SUCCESS);
}
