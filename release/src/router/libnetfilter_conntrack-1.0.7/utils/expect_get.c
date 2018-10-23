#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>

#include <libnetfilter_conntrack/libnetfilter_conntrack.h>

static int cb(enum nf_conntrack_msg_type type,
	      struct nf_expect *exp,
	      void *data)
{
	char buf[1024];

	nfexp_snprintf(buf, 1024, exp, NFCT_T_UNKNOWN, NFCT_O_DEFAULT, 0);
	printf("%s\n", buf);

	return NFCT_CB_CONTINUE;
}

int main(void)
{
	int ret;
	struct nfct_handle *h;
	struct nf_conntrack *master;
	struct nf_expect *exp;

	master = nfct_new();
	if (!master) {
		perror("nfct_new");
		exit(EXIT_FAILURE);
	}

	nfct_set_attr_u8(master, ATTR_L3PROTO, AF_INET);
	nfct_set_attr_u32(master, ATTR_IPV4_SRC, inet_addr("1.1.1.1"));
	nfct_set_attr_u32(master, ATTR_IPV4_DST, inet_addr("2.2.2.2"));

	nfct_set_attr_u8(master, ATTR_L4PROTO, IPPROTO_TCP);
	nfct_set_attr_u16(master, ATTR_PORT_SRC, htons(10240));
	nfct_set_attr_u16(master, ATTR_PORT_DST, htons(10241));

	exp = nfexp_new();
	if (!exp) {
		perror("nfexp_new");
		nfct_destroy(master);
		exit(EXIT_FAILURE);
	}

	nfexp_set_attr(exp, ATTR_EXP_MASTER, master);

	h = nfct_open(EXPECT, 0);
	if (!h) {
		perror("nfct_open");
		nfct_destroy(master);
		return -1;
	}

	nfexp_callback_register(h, NFCT_T_ALL, cb, NULL);
	ret = nfexp_query(h, NFCT_Q_GET, exp);

	printf("TEST: get expectation ");
	if (ret == -1)
		printf("(%d)(%s)\n", ret, strerror(errno));
	else
		printf("(OK)\n");

	nfct_close(h);

	nfct_destroy(master);

	ret == -1 ? exit(EXIT_FAILURE) : exit(EXIT_SUCCESS);
}
