#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>

#include <libnetfilter_conntrack/libnetfilter_conntrack.h>
#include <libnetfilter_conntrack/libnetfilter_conntrack_tcp.h>

/*
 * WARNING: This test file creates an expectation for the FTP helper.
 * Therefore, make sure you have load nf_conntrack_ftp before executing it. 
 */

int main(void)
{
	int ret;
	struct nfct_handle *h;
	struct nf_conntrack *master, *expected, *mask;
	struct nf_expect *exp;

	/*
	 * Step 1: Setup master conntrack
	 */

	master = nfct_new();
	if (!master) {
		perror("nfct_new");
		exit(EXIT_FAILURE);
	}

	nfct_set_attr_u8(master, ATTR_L3PROTO, AF_INET);
	nfct_set_attr_u32(master, ATTR_IPV4_SRC, inet_addr("1.1.1.1"));
	nfct_set_attr_u32(master, ATTR_IPV4_DST, inet_addr("2.2.2.2"));

	nfct_set_attr_u8(master, ATTR_L4PROTO, IPPROTO_TCP);
	nfct_set_attr_u16(master, ATTR_PORT_SRC, htons(1025));
	nfct_set_attr_u16(master, ATTR_PORT_DST, htons(21));

	nfct_setobjopt(master, NFCT_SOPT_SETUP_REPLY);

	nfct_set_attr_u8(master, ATTR_TCP_STATE, TCP_CONNTRACK_ESTABLISHED);
	nfct_set_attr_u32(master, ATTR_TIMEOUT, 200);
	nfct_set_attr(master, ATTR_HELPER_NAME, "ftp");

	h = nfct_open(CONNTRACK, 0);
	if (!h) {
		perror("nfct_open");
		nfct_destroy(master);
		return -1;
	}

	ret = nfct_query(h, NFCT_Q_CREATE, master);

	printf("TEST: add master conntrack ");
	if (ret == -1)
		printf("(%d)(%s)\n", ret, strerror(errno));
	else
		printf("(OK)\n");

	nfct_close(h);

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

	mask = nfct_new();
	if (!mask) {
		perror("nfct_new");
		nfct_destroy(master);
		nfct_destroy(expected);
		exit(EXIT_FAILURE);
	}

	nfct_set_attr_u8(mask, ATTR_L3PROTO, AF_INET);
	nfct_set_attr_u32(mask, ATTR_IPV4_SRC, 0xffffffff);
	nfct_set_attr_u32(mask, ATTR_IPV4_DST, 0xffffffff);

	nfct_set_attr_u8(mask, ATTR_L4PROTO, IPPROTO_TCP);
	nfct_set_attr_u16(mask, ATTR_PORT_SRC, 0x0000);
	nfct_set_attr_u16(mask, ATTR_PORT_DST, 0xffff);

	/*
	 * Step 2: Setup expectation
	 */
	
	exp = nfexp_new();
	if (!exp) {
		perror("nfexp_new");
		nfct_destroy(master);
		nfct_destroy(expected);
		nfct_destroy(mask);
		exit(EXIT_FAILURE);
	}

	nfexp_set_attr(exp, ATTR_EXP_MASTER, master);
	nfexp_set_attr(exp, ATTR_EXP_EXPECTED, expected);
	nfexp_set_attr(exp, ATTR_EXP_MASK, mask);
	nfexp_set_attr_u32(exp, ATTR_EXP_TIMEOUT, 200);

	nfct_destroy(master);
	nfct_destroy(expected);
	nfct_destroy(mask);

	h = nfct_open(EXPECT, 0);
	if (!h) {
		perror("nfct_open");
		return -1;
	}

	ret = nfexp_query(h, NFCT_Q_CREATE, exp);

	printf("TEST: create expectation ");
	if (ret == -1)
		printf("(%d)(%s)\n", ret, strerror(errno));
	else
		printf("(OK)\n");

	nfct_close(h);

	ret == -1 ? exit(EXIT_FAILURE) : exit(EXIT_SUCCESS);
}
