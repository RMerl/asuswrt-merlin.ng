/*
 * Test for the filter API
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>

#include <libnetfilter_conntrack/libnetfilter_conntrack.h>

static int event_cb(enum nf_conntrack_msg_type type,
		    struct nf_conntrack *ct,
		    void *data)
{
	static int n = 0;
	char buf[1024];

	nfct_snprintf(buf, sizeof(buf), ct, type, NFCT_O_PLAIN, NFCT_OF_TIME);
	printf("%s\n", buf);

	if (++n == 10)
		return NFCT_CB_STOP;

	return NFCT_CB_CONTINUE;
}

int main(void)
{
	int i, ret;
	struct nfct_handle *h;
	struct nfct_filter *filter;

	h = nfct_open(CONNTRACK, NF_NETLINK_CONNTRACK_NEW |
				 NF_NETLINK_CONNTRACK_UPDATE);
	if (!h) {
		perror("nfct_open");
		return 0;
	}

	filter = nfct_filter_create();
	if (!filter) {
		perror("nfct_create_filter");
		return 0;
	}

	if (nfct_filter_attach(nfct_fd(h), filter) == -1) {
		perror("nfct_filter_attach");
		return 0;
	}

	/* protocol 255 is skipped since we support up to 255 protocols max */
	for (i=0; i<IPPROTO_MAX; i++)
		nfct_filter_add_attr_u32(filter,NFCT_FILTER_L4PROTO,i);

	/* up to 127 IP addresses, above that adding is noop */
	for (i=0; i<128; i++) {
		/* BSF always wants data in host-byte order */
		struct nfct_filter_ipv4 fltr_ipv4 = {
			.addr = ntohl(inet_addr("127.0.0.1")) + i,
			.mask = 0xffffffff,
		};
		nfct_filter_add_attr(filter, NFCT_FILTER_SRC_IPV4, &fltr_ipv4);
	};

	if (nfct_filter_attach(nfct_fd(h), filter) == -1) {
		perror("nfct_filter_attach");
		return 0;
	}

	nfct_filter_destroy(filter);

	nfct_callback_register(h, NFCT_T_ALL, event_cb, NULL);

	ret = nfct_catch(h);
	printf("test ret=%d (%s)\n", ret, strerror(errno));
	return EXIT_SUCCESS;
}
