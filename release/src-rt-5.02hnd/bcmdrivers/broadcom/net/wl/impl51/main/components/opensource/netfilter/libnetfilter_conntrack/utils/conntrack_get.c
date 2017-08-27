#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>

#include <libnetfilter_conntrack/libnetfilter_conntrack.h>
#include <libnetfilter_conntrack/libnetfilter_conntrack_tcp.h>

static int cb(enum nf_conntrack_msg_type type,
	      struct nf_conntrack *ct,
	      void *data)
{
	char buf[1024];

	nfct_snprintf(buf, sizeof(buf), ct, NFCT_T_UNKNOWN, NFCT_O_DEFAULT, NFCT_OF_SHOW_LAYER3);
	printf("%s\n", buf);

	return NFCT_CB_CONTINUE;
}

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

	nfct_set_attr_u8(ct, ATTR_L3PROTO, AF_INET);
	nfct_set_attr_u32(ct, ATTR_IPV4_SRC, inet_addr("1.1.1.1"));
	nfct_set_attr_u32(ct, ATTR_IPV4_DST, inet_addr("2.2.2.2"));
	
	nfct_set_attr_u8(ct, ATTR_L4PROTO, IPPROTO_TCP);
	nfct_set_attr_u16(ct, ATTR_PORT_SRC, htons(20));
	nfct_set_attr_u16(ct, ATTR_PORT_DST, htons(10));

	h = nfct_open(CONNTRACK, 0);
	if (!h) {
		perror("nfct_open");
		nfct_destroy(ct);
		return -1;
	}

	nfct_callback_register(h, NFCT_T_ALL, cb, NULL);

	ret = nfct_query(h, NFCT_Q_GET, ct);

	printf("TEST: get conntrack ");
	if (ret == -1)
		printf("(%d)(%s)\n", ret, strerror(errno));
	else
		printf("(OK)\n");

	nfct_close(h);

	nfct_destroy(ct);

	ret == -1 ? exit(EXIT_FAILURE) : exit(EXIT_SUCCESS);
}
